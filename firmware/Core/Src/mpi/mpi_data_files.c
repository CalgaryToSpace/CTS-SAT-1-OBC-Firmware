#include "mpi/mpi_data_files.h"
#include "log/log.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "littlefs/littlefs_searching.h"


/// @brief Check if the given file path is a valid MPI science data file with a few heuristics (file size, sync word count).
/// @param file_path Path of potential science data file to validate.
/// @return 0 if valid. >0 on invalid.
uint8_t MPI_validate_science_data_file(const char* file_path) {
    // Check the file size: Expecting at least 20_000 bytes.
    lfs_ssize_t file_size = LFS_file_size(file_path, 1);
    if (file_size < 0) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "LFS_file_size() failed for file %s with error code %ld",
            file_path, file_size
        );
        return 10;
    }
    else if (file_size < 20000) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "File size is too small: %ld bytes. Expected at least 20_000 bytes.",
            file_size
        );
        return 20;
    }

    // Check the file contents. Expect to see the sync words (0x0C 0xFF 0xFF 0x0C) repeatedly.
    uint8_t expected_sync_word[] = {0x0C, 0xFF, 0xFF, 0x0C};
    const int32_t sync_count = LFS_search_count_occurrences(
        file_path, expected_sync_word, sizeof(expected_sync_word)
    );
    const int32_t expected_sync_word_count = file_size / 160;
    const uint8_t valid_sync_word_count = (
        (sync_count >= expected_sync_word_count * 0.8)
        && (sync_count <= expected_sync_word_count * 1.2)
    );
    if (!valid_sync_word_count) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Sync word count is invalid: found %ld sync words, expected %ld sync words in %ld bytes (%ld to %ld).",
            sync_count, expected_sync_word_count,
            file_size,
            (int32_t)(expected_sync_word_count * 0.8),
            (int32_t)(expected_sync_word_count * 1.2)
        );
        return 30;
    }

    return 0; // Success.
}
