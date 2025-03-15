#include "main.h"
#include "littlefs/littlefs_benchmark.h"
#include "littlefs/littlefs_helper.h"
#include "flash_driver.h"
#include "log/log.h"

#include <string.h>
#include <stdio.h>

/// @brief Benchmarks the write/read operations on the LittleFS file system.
/// @details This function will write test data to a file, read it back, and verify the read data.
/// @param write_size Number of bytes to write during each write operation.
/// @param write_count Number of times to write the bytes.
/// @param response_str 
/// @param response_str_len
/// @param write_mode Check to see if we are writing to a new file or the same file.
/// @param mount_mode Check to see if we are mounting once, or before each iteration.
/// @return 0 on success. >0 if there was an error.
uint8_t LFS_benchmark_write_read(uint16_t write_size, uint16_t write_count, char* response_str, 
    uint16_t response_str_len, LFS_benchmark_write_mode_enum_t write_mode, LFS_benchmark_mount_mode_enum_t mount_mode) {
    const char dir_name[] = "LFS_benchmark_write_read";
    char file_name[100];

    // If LFS is not mounted, mount it
    if (!LFS_is_lfs_mounted) {
        const int8_t mount_result = LFS_mount();
        if (mount_result != 0) {
            snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str), 
            "LFS Mount Failed. LFS_mount return: %d\n", mount_result);
            return 1;
        }
    }

    // If Append File mode, create a directory and set the file name
    if(write_mode == LFS_NEW_FILE) {
        const int8_t make_dir_result = LFS_make_directory(dir_name);
        if (make_dir_result != 0) {
            snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str), 
            "Make Directory Failed. LFS_make_directory return: %d\n", make_dir_result);
            return 1;
        }
    } else {

        // Else just set the file name
        snprintf(file_name, sizeof(file_name), "benchmark_test.txt");
    }

    // Unmount before starting benchmarking
    int8_t unmount_result = LFS_unmount();
    if (unmount_result != 0) {
        snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str), 
        "LFS Unmount Failed. LFS_unmount return: %d\n", unmount_result);
        return 1;
    }

    // Iterate through the number of writes needed
    for (uint16_t i = 0; i < write_count; i++) {
        uint8_t expected_checksum = 0;
        uint8_t read_checksum = 0;

        // Prep some data to write
        uint8_t write_buffer[write_size];
        for (uint32_t i = 0; i < write_size; i++) {
            const uint8_t b = (i + 42) % 256;
            write_buffer[i] = b;
        }
        uint8_t read_buffer[write_size];

        // Calculate the checksum for write_buffer
        for (uint32_t i = 0; i < write_size; i++) {
            expected_checksum ^= write_buffer[i]; // Weak checksum, but good enough
        }

        //Create a new filename if we do single file writes
        if (write_mode == LFS_NEW_FILE) {
            snprintf(file_name, sizeof(file_name), "%s/benchmark_test_%lu.txt", dir_name, HAL_GetTick());
        }

        // Mount if necessary
        if (!LFS_is_lfs_mounted) {
            const uint32_t mount_start_time = HAL_GetTick();
            const int8_t mount_result = LFS_mount();
            if (mount_result != 0) {
                snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str), 
                "LFS Mount Failed. LFS_mount return: %d\n", mount_result);
                return 1;
            }
            LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "LFS Mount: %lu", HAL_GetTick() - mount_start_time);
        }

        lfs_file_t file;
        
        // Open file for writing
        const uint32_t write_open_start_time = HAL_GetTick();
        const int8_t write_open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC, &LFS_file_cfg);
        if (write_open_result < 0) {
            snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str),
                "Open Failed (Write). lfs_file_open return: %d\n", write_open_result);
            return 1;
	    }
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "File Open (Write): %lu", HAL_GetTick() - write_open_start_time);

        // Seek to the end of file if in append mode
        if (write_mode == LFS_APPEND_FILE) {
            const uint32_t write_seek_start_time = HAL_GetTick();
            const lfs_soff_t write_seek_result = lfs_file_seek(&LFS_filesystem, &file, 0, LFS_SEEK_END);
            if (write_seek_result < 0) {
                snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str),
                    "Seek Failed (Write). lfs_file_seek return: %ld\n", write_seek_result);
                return 1;
            }
            LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "File Seek (Write): %lu", HAL_GetTick() - write_seek_start_time);
        }

        // Write to file
        const uint32_t write_start_time = HAL_GetTick();
        const lfs_ssize_t write_result = lfs_file_write(&LFS_filesystem, &file, write_buffer, write_size);
        if (write_result < 0) {
            snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str),
                "Write Failed. lfs_file_write return: %ld\n", write_result);
            return 1;
	    }
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "File Write: %lu", HAL_GetTick() - write_start_time);

        // Close file
        const uint32_t write_close_start_time = HAL_GetTick();
        const int8_t write_close_result = lfs_file_close(&LFS_filesystem, &file);
        if (write_close_result < 0) {
            snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str),
                "Close Failed (Write). lfs_file_close return: %d\n", write_close_result);
            return 1;
	    }
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "File Close (Write): %lu", HAL_GetTick() - write_close_start_time);


        // Open file for reading
        const uint32_t read_open_start_time = HAL_GetTick();
        const int8_t read_open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_RDONLY, &LFS_file_cfg);
        if (read_open_result < 0) {
            snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str),
                "Open Failed (Write). lfs_file_open return: %d\n", read_open_result);
            return 1;
	    }
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "File Open (Read): %lu", HAL_GetTick() - read_open_start_time);

        // // Seek to the end of file if in append mode
        // if (write_mode == LFS_APPEND_FILE) {
        //     const uint32_t read_seek_start_time = HAL_GetTick();
        //     const lfs_soff_t read_seek_result = lfs_file_seek(&LFS_filesystem, &file, 0, LFS_SEEK_END);
        //     if (read_seek_result < 0) {
        //         snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str),
        //             "Seek Failed (Read). lfs_file_seek return: %ld\n", read_seek_result);
        //         return 1;
        //     }
        //     snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str),
        //             "File Seek (Read): %lu", HAL_GetTick() - read_seek_start_time);
        // }

        // Read file
        const uint32_t read_start_time = HAL_GetTick();
        const lfs_ssize_t read_result = lfs_file_read(&LFS_filesystem, &file, read_buffer, write_size);
        if (read_result < 0) {
            snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str),
                "Read Failed. lfs_file_read return: %ld\n", read_result);
            return 1;
	    }
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "File Read: %lu", HAL_GetTick() - read_start_time);

        // Calculate read_buffer checksum
        for (uint32_t i = 0; i < write_size; i++) {
            expected_checksum ^= read_buffer[i];
        }

        // Close file
        const uint32_t read_close_start_time = HAL_GetTick();
        const int8_t read_close_result = lfs_file_close(&LFS_filesystem, &file);
        if (read_close_result < 0) {
            snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str),
                "Close Failed (Read). lfs_file_close return: %d\n", read_close_result);
            return 1;
	    }
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "File Close (Read): %lu", HAL_GetTick() - read_close_start_time);

        // Verify checksum
        if (read_checksum != expected_checksum) {
            snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str),
                "Checksum mismatch: expected %u, got %u\n", expected_checksum, read_checksum);
            return 3;
        }

        // Unmount
        if (mount_mode == LFS_MOUNT_EACH) {
            const uint32_t unmount_start_time = HAL_GetTick();
            const int8_t unmount_result = LFS_unmount();
            if (unmount_result != 0) {
                snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str), 
                "LFS Unmount Failed. LFS_unmount return: %d\n", unmount_result);
                return 1;
            }
            LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "LFS Unmount: %lu", HAL_GetTick() - unmount_start_time);
        }

        print_calls();
    }

    // Unmount once done with all operations
    if (LFS_is_lfs_mounted) {
        const uint32_t unmount_start_time = HAL_GetTick();
        unmount_result = LFS_unmount();
        if (unmount_result != 0) {
            snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str), 
            "LFS Unmount Failed. LFS_unmount return: %d\n", unmount_result);
            return 1;
        }
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "LFS Unmount: %lu", HAL_GetTick() - unmount_start_time);
    }

    return 0;
}

/// @brief Benchmarks the write/read operations on the LittleFS file system for both cases: writing to a new file and writing to an existing file.
/// @details This function will write test data to a static filename, read it back, and verify the read data for both cases.
/// @param write_size Number of bytes to write in each file per iteration.
/// @param write_count Number of times to write to the file.
/// @param response_str 
/// @param response_str_len 
/// @return 0 on success. >0 if there was an error.
uint8_t LFS_benchmark_write_read_single_and_new(uint16_t write_chunk_size, uint16_t write_chunk_count, char* response_str, uint16_t response_str_len) {
    response_str[0] = '\0';

    FLASH_BENCHMARK_ENABLED = 1;

    int8_t format_result = LFS_format();
    if (format_result != 0) {
        snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str), 
        "LFS Format Failed. LFS_format return: %d\n", format_result);
        return 1;
    }
    uint32_t bench_start_time = HAL_GetTick();
    LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Benchmark appending file (Single Mount): \n");
    uint8_t benchmark_same_file_result = LFS_benchmark_write_read(write_chunk_size, write_chunk_count, response_str, response_str_len, LFS_APPEND_FILE, LFS_MOUNT_ONCE);
    if(benchmark_same_file_result != 0) {
        return benchmark_same_file_result;
    }
    LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "!! Total Time: %lu", HAL_GetTick() - bench_start_time);


    format_result = LFS_format();
    if (format_result != 0) {
        snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str), 
        "LFS Format Failed. LFS_format return: %d\n", format_result);
        return 1;
    }
    bench_start_time = HAL_GetTick();
    LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Benchmark writing to new files (Single Mount): \n");
    uint8_t benchmark_new_file_result = LFS_benchmark_write_read(write_chunk_size, write_chunk_count, response_str, response_str_len, LFS_NEW_FILE, LFS_MOUNT_ONCE);
    if(benchmark_new_file_result != 0) {
        return benchmark_same_file_result;
    }
    LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "!! Total Time: %lu", HAL_GetTick() - bench_start_time);


    // format_result = LFS_format();
    // if (format_result != 0) {
    //     snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str), 
    //     "LFS Format Failed. LFS_format return: %d\n", format_result);
    //     return 1;
    // }
    // bench_start_time = HAL_GetTick();
    // LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Benchmark appending file (Multiple Mount): \n");
    // benchmark_same_file_result = LFS_benchmark_write_read(write_chunk_size, write_chunk_count, response_str, response_str_len, LFS_APPEND_FILE, LFS_MOUNT_EACH);
    // if(benchmark_same_file_result != 0) {
    //     return benchmark_same_file_result;
    // }
    // LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "!! Total Time: %lu", HAL_GetTick() - bench_start_time);


    // format_result = LFS_format();
    // if (format_result != 0) {
    //     snprintf(&response_str[strlen(response_str)], response_str_len - strlen(response_str), 
    //     "LFS Format Failed. LFS_format return: %d\n", format_result);
    //     return 1;
    // }
    // bench_start_time = HAL_GetTick();
    // LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Benchmark writing to new files (Multiple Mount): \n");
    // benchmark_new_file_result = LFS_benchmark_write_read(write_chunk_size, write_chunk_count, response_str, response_str_len, LFS_NEW_FILE, LFS_MOUNT_EACH);
    // if(benchmark_new_file_result != 0) {
    //     return benchmark_same_file_result;
    // }
    // LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "!! Total Time: %lu", HAL_GetTick() - bench_start_time);
    // LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), );
    
    FLASH_BENCHMARK_ENABLED = 0;

    return 0;
}