#include "compression/heatshrink_helpers.h"

#include "littlefs/lfs.h"
#include "compression/heatshrink_lib/heatshrink_encoder.h"
#include "log/log.h"

#define HS_WINDOW_BITS   8   // tune for memory vs compression
#define HS_LOOKAHEAD_BITS 4

#define INPUT_CHUNK_SIZE   64
#define OUTPUT_CHUNK_SIZE  64


/// @brief Compress a file with heatshrink, writing the output to a new file.
/// @param lfs 
/// @param input_path 
/// @param output_path 
/// @return 0 on success. Negative errors for LittleFS. Positive errors for heatshrink.
int8_t LFS_compress_lfs_file_with_heatshrink(
    lfs_t *lfs,
    const char *input_path,
    const char *output_path
) {
    int err;

    lfs_file_t in_file;
    lfs_file_t out_file;

    // Open input file.
    err = lfs_file_open(lfs, &in_file, input_path, LFS_O_RDONLY);
    if (err < 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Compression: Could not input file for reading: %d",
            err
        );
        return err;
    }

    // Open output file.
    err = lfs_file_open(
        lfs, &out_file, output_path,
        LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC
    );
    if (err < 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Compression: Could not output file for writing: %d",
            err
        );

        lfs_file_close(lfs, &in_file);
        return err;
    }

    // Init heatshrink encoder
    heatshrink_encoder hse;
    heatshrink_encoder_reset(&hse);

    uint8_t in_buf[INPUT_CHUNK_SIZE];
    uint8_t out_buf[OUTPUT_CHUNK_SIZE];

    LOG_message(
        LOG_SYSTEM_LFS, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "Compression: Starting compression of file"
    );

    size_t sunk = 0;
    size_t polled = 0;

    while (1) {
        // Read chunk
        lfs_ssize_t read_bytes = lfs_file_read(lfs, &in_file, in_buf, sizeof(in_buf));
        if (read_bytes < 0) {
            err = read_bytes;
            break;
        }

        // End of file
        int input_done = (read_bytes == 0);

        size_t input_offset = 0;

        // Sink input into encoder
        while (input_offset < (size_t)read_bytes) {
            HSE_sink_res sres = heatshrink_encoder_sink(
                &hse,
                &in_buf[input_offset],
                read_bytes - input_offset,
                &sunk
            );
            input_offset += sunk;

            if (sres < 0) {
                err = 1;
                goto cleanup;
            }

            // Poll output after every sink
            do {
                HSE_poll_res pres = heatshrink_encoder_poll(
                    &hse,
                    out_buf,
                    sizeof(out_buf),
                    &polled
                );

                if (pres < 0) {
                    err = 2;
                    goto cleanup;
                }

                if (polled > 0) {
                    const lfs_ssize_t write_result = lfs_file_write(
                        lfs, &out_file, out_buf, polled
                    );
                    if (write_result < 0) {
                        err = write_result;
                        goto cleanup;
                    }
                }

            } while (polled > 0);
        }

        if (input_done) {
            break;
        }
    }

    LOG_message(
        LOG_SYSTEM_LFS, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "Compression: Past polling section"
    );

    // Finish encoding
    heatshrink_encoder_finish(&hse);

    // Flush remaining output
    while (1) {
        HSE_poll_res pres = heatshrink_encoder_poll(
            &hse,
            out_buf,
            sizeof(out_buf),
            &polled
        );

        if (pres < 0) {
            err = 4;
            goto cleanup;
        }

        if (polled > 0) {
            const lfs_ssize_t write_result = lfs_file_write(
                lfs, &out_file, out_buf, polled
            );
            if (write_result < 0) {
                err = write_result;
                goto cleanup;
            }
        }

        if (pres == HSER_POLL_EMPTY) {
            break;
        }
    }

    err = 0;

cleanup:
    lfs_file_close(lfs, &in_file);
    lfs_file_close(lfs, &out_file);
    return err;
}
