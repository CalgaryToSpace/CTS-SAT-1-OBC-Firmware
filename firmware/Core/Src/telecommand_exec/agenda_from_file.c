#include "telecommand_exec/agenda_from_file.h"

#include "telecommand_exec/telecommand_parser.h"
#include "telecommand_exec/telecommand_executor.h"
#include "littlefs/littlefs_helper.h"
#include "log/log.h"

static char *TCMD_active_agenda_filename_disabled_sentinel = "DISABLED";

/// @brief The file path of the agenda file to load upcoming telecommands from.
/// @warning While a default agenda is set, it is critical to understand that you SHOULD NOT
///     fill this agenda file unless absolutely necessary. If you create and fill an agenda
///     with this default name, it will run on every boot, thus defeating the purpose of
///     reboots being a stable, fail-safe startup mode.
/// @note If system stability ends up being super bad, you can put commands in the default to run at
///     the earth poles on subsequent reboots. That's probably the only time you should ever
///     create this file though!
/// @note Nominally, put the agenda in another random file, then update this config string to point
///     to that other file. That way, on reboot, the agenda stops running (as a fail-safe)!
/// @note Set this to "DISABLED" (case-sensitive) to disable this feature entirely.
char TCMD_active_agenda_filename[LFS_MAX_PATH_LENGTH] = "default_tcmd_agenda.txt";


/// @brief Parses a file of telecommands and enqueues them into the agenda.
/// @param file_path Path to file with one telecommand per line.
/// @param min_tsexec_inclusive Filter to only telecommands with `tsexec` greater than or equal to this value.
/// @param max_tsexec_inclusive Filter to only telecommands with `tsexec` less than or equal to this value.
/// @param max_enqueue_count Maximum number of telecommands to enqueue. Stop after this many successes. Mostly for safety.
/// @return 0 on success.
/// @details The file should have one telecommand per line, like so: `CTS1+xxx(...)!\nCTS1+yyy(...)!\n`
/// @note Obeys all the rules about enqueuing duplicate tssent telecommands. Recommend having unique @tssent values
///     for each telecommand, and enabling the `TCMD_require_unique_tssent` config option.
uint8_t TCMD_parse_tcmds_from_file_and_enqueue(
    const char *file_path,
    uint64_t min_tsexec_inclusive, uint64_t max_tsexec_inclusive,
    uint16_t max_enqueue_count
) {
    if (strcmp(file_path, TCMD_active_agenda_filename_disabled_sentinel) == 0) {
        return 0; // Success.
    }

    // Open the file.
    lfs_file_t agenda_file;
    const int8_t open_result = lfs_file_open(
        &LFS_filesystem, &agenda_file,
        file_path, LFS_O_RDONLY
    );
    if (open_result < 0) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error opening agenda file: %d", open_result
        );
        return 5;
    }

    char file_chunk[TCMD_MAX_FULL_LENGTH];
    char line_buf[TCMD_MAX_FULL_LENGTH * 2]; // allow spillover across chunks
    size_t line_len = 0;

    uint32_t tcmd_count_success_enqueued = 0;
    uint32_t tcmd_count_success_but_filtered = 0;
    uint32_t tcmd_count_failed_parsing = 0;

    // Parse the file.
    while (1) {
        const lfs_ssize_t read_result = lfs_file_read(
            &LFS_filesystem, &agenda_file,
            file_chunk, sizeof(file_chunk)
        );
        if (read_result < 0) {
            LOG_message(
                LOG_SYSTEM_GNSS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                "Error reading agenda file: %ld", read_result
            );
            return 6;
        }
        if (read_result == 0) {
            break; // End of file
        }

        // Parse the chunk, looking for a delimiter.
        // Append chunk to buffer (with overflow protection).
        if (line_len + read_result >= sizeof(line_buf)) {
            LOG_message(
                LOG_SYSTEM_GNSS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                "TCMD buffer overflow while parsing agenda file (maybe line too long)."
            );
            return 7;
        }

        memcpy(&line_buf[line_len], file_chunk, read_result);
        line_len += read_result;

        // Scan for complete telecommands ending in "!\n".
        size_t start_idx = 0;

        for (size_t i = 0; i + 1 < line_len; i++) {
            // Skip over any positions that aren't "!\n". Specifically looking to locate the full end.
            if (!(line_buf[i] == '!' && line_buf[i + 1] == '\n')) {
                continue;
            }

            // Now, `i` points to the end of the telecommand. Deal with that chunk!
        
            const size_t tcmd_len = i - start_idx + 1; // include '!'

            // Copy into null-terminated string.
            char tcmd_str[TCMD_MAX_FULL_LENGTH];
            if (tcmd_len >= sizeof(tcmd_str)) {
                LOG_message(
                    LOG_SYSTEM_GNSS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "TCMD too long in agenda file."
                );
                return 8;
            }

            memcpy(tcmd_str, &line_buf[start_idx], tcmd_len);
            tcmd_str[tcmd_len] = '\0';

            // Parse telecommand
            TCMD_parsed_tcmd_to_execute_t parsed;
            const uint8_t parse_result = TCMD_parse_full_telecommand(
                tcmd_str, &parsed
            );

            if (parse_result == 0) {
                // Apply tsexec filter.
                if (parsed.timestamp_to_execute >= min_tsexec_inclusive &&
                    parsed.timestamp_to_execute <= max_tsexec_inclusive
                ) {
                    // Enqueue the command.
                    TCMD_add_tcmd_to_agenda(&parsed);

                    tcmd_count_success_enqueued++;
                }
                else {
                    tcmd_count_success_but_filtered++;
                }
            } else {
                LOG_message(
                    LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "Failed to parse TCMD: %s (err=%u)", tcmd_str, parse_result
                );
                tcmd_count_failed_parsing++;
            }

            // Move start past this command.
            start_idx = i + 2;
            i++; // skip '\n'
        }

        // Shift remaining partial data to front.
        if (start_idx > 0) {
            memmove(line_buf, &line_buf[start_idx], line_len - start_idx);
            line_len -= start_idx;
        }

        // Check if we've hit the limit.
        if (tcmd_count_success_enqueued >= max_enqueue_count) {
            break;
        }
    }

    if (tcmd_count_failed_parsing > 0) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Agenda File: Failed to parse %lu/%lu telecommands from agenda file.",
            tcmd_count_failed_parsing,
            tcmd_count_success_enqueued + tcmd_count_success_but_filtered + tcmd_count_failed_parsing
        );
    }

    if (tcmd_count_failed_parsing > 0) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Agenda File: Parsed %lu telecommands and enqueued %lu. Failed to parse %lu/%lu telecommands.",
            tcmd_count_success_enqueued + tcmd_count_success_but_filtered,
            tcmd_count_success_enqueued,
            tcmd_count_failed_parsing,
            tcmd_count_success_enqueued + tcmd_count_success_but_filtered + tcmd_count_failed_parsing
        );
    }

    return 0; // Success.
}
