#include "compression/heatshrink_helpers.h"

#include "littlefs/lfs.h"
#include "compression/heatshrink_lib/heatshrink_encoder.h"

#include "log/log.h"
#include <stdlib.h>
#include <string.h>

/// @brief Compress a file with Heatshrink, similar to the CLI.
/// @param lfs pointer to LittleFS instance
/// @param input_path input file path
/// @param output_path output file path
/// @param window_sz2 log2 sliding window size (like CLI arg -w)
/// @param lookahead_sz2 number of bits for backref length (like CLI arg -l)
/// @return 0 success, negative LittleFS error, positive Heatshrink error.
int8_t LFS_compress_lfs_file_with_heatshrink(
    lfs_t *lfs,
    const char *input_path,
    const char *output_path,
    uint8_t window_sz2,
    uint8_t lookahead_sz2
) {
    int err = 0;
    lfs_file_t in_file, out_file;

    if ((err = lfs_file_open(lfs, &in_file, input_path, LFS_O_RDONLY)) < 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Compression: cannot open input %s: %d", input_path, err
        );
        return err;
    }

    if ((err = lfs_file_open(lfs, &out_file, output_path,
                              LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC)) < 0) {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "Compression: cannot open output %s: %d", output_path, err);
        lfs_file_close(lfs, &in_file);
        return err;
    }

    // Allocate encoder (CLI style)
    size_t window_sz = 1 << window_sz2;
    heatshrink_encoder *hse = heatshrink_encoder_alloc(window_sz2, lookahead_sz2);
    if (!hse) {
        lfs_file_close(lfs, &in_file);
        lfs_file_close(lfs, &out_file);
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "Compression: failed to allocate heatshrink encoder");
        return 1;
    }

    uint8_t *in_buf = pvPortMalloc(window_sz);
    uint8_t *out_buf = pvPortMalloc(window_sz);
    if (!in_buf || !out_buf) {
        heatshrink_encoder_free(hse);
        lfs_file_close(lfs, &in_file);
        lfs_file_close(lfs, &out_file);
        vPortFree(in_buf);
        vPortFree(out_buf);
        return 2;
    }

    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
                "Compression: starting compression of %s", input_path);

    size_t sunk = 0, polled = 0;
    while (1) {
        lfs_ssize_t nread = lfs_file_read(lfs, &in_file, in_buf, window_sz);
        if (nread < 0) { err = nread; break; }

        int input_done = (nread == 0);
        size_t offset = 0;

        while (offset < (size_t)nread) {
            HSE_sink_res sres = heatshrink_encoder_sink(hse, in_buf + offset, nread - offset, &sunk);
            if (sres < 0) {
                err = 3; goto cleanup;
            }
            offset += sunk;

            // Poll output after every sink
            do {
                HSE_poll_res pres = heatshrink_encoder_poll(hse, out_buf, window_sz, &polled);
                if (pres < 0) { err = 4; goto cleanup; }

                if (polled > 0) {
                    lfs_ssize_t nwritten = lfs_file_write(lfs, &out_file, out_buf, polled);
                    if (nwritten < 0) {
                        err = nwritten; goto cleanup;
                    }
                }
            } while (polled > 0);
        }

        if (input_done) {
            break;
        }
    }

    // Finish encoder
    heatshrink_encoder_finish(hse);
    while (1) {
        HSE_poll_res pres = heatshrink_encoder_poll(hse, out_buf, window_sz, &polled);
        if (pres < 0) {
            err = 5; goto cleanup;
        }

        if (polled > 0) {
            lfs_ssize_t nwritten = lfs_file_write(lfs, &out_file, out_buf, polled);
            if (nwritten < 0) {
                err = nwritten;
                goto cleanup;
            }
        }

        if (pres == HSER_POLL_EMPTY) {
            break;
        }
    }

cleanup:
    lfs_file_close(lfs, &in_file);
    lfs_file_close(lfs, &out_file);
    heatshrink_encoder_free(hse);
    vPortFree(in_buf);
    vPortFree(out_buf);

    return err;
}
