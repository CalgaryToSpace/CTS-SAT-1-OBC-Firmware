
#include "main.h"
#include "littlefs/littlefs_benchmark.h"
#include "littlefs/littlefs_helper.h"

#include <string.h>
#include <stdio.h>

/// @brief Benchmarks the write/read operations on the LittleFS file system.
/// @details This function will write test data to a static filename, read it back, and verify the read data.
/// @param write_chunk_size Number of bytes to write in each chunk.
/// @param write_chunk_count Number of chunks to write.
/// @param response_str 
/// @param response_str_len
/// @param mode Check to see if we are writing to a new file or the same file.
/// @return 0 on success. >0 if there was an error.
uint8_t LFS_benchmark_write_read(uint16_t write_chunk_size, uint16_t write_chunk_count, char* response_str, uint16_t response_str_len, LFS_benchmark_mode_enum_t mode) {
    char file_name[100];
    
    if(mode == LFS_NEW_FILE) {
        const char dir_name[] = "benchmark_write_read";
        // FIXME: check if we care about return value
        lfs_mkdir(&LFS_filesystem, dir_name);
        snprintf(
            file_name,
            sizeof(file_name),
            "%s/benchmark_test_%lu.txt",
            dir_name,
            HAL_GetTick()
        );
    } else {
        // Default to single file mode
        snprintf(
            file_name,
            sizeof(file_name),
            "benchmark_test.txt"
        );
    }

    uint8_t expected_checksum = 0;

    // Prep some data to write
    uint8_t write_buffer[write_chunk_size];
    for (uint32_t i = 0; i < write_chunk_size; i++) {
        const uint8_t b = (i + 42) % 256;
        write_buffer[i] = b;
    }

    // Open file for writing
    const uint32_t write_open_start_time = HAL_GetTick();
    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC, &LFS_file_cfg);
	if (open_result != 0)
	{
		snprintf(
            &response_str[strlen(response_str)],
            response_str_len - strlen(response_str),
            "Open failed. LFS_file_open return: %d\n", open_result);
		return 1;
	}
    const uint32_t open_end_time = HAL_GetTick();
    snprintf(
        &response_str[strlen(response_str)],
        response_str_len - strlen(response_str),
        "Write open: %lu ms\n", open_end_time - write_open_start_time);

    // Write
    const uint32_t write_send_start_time = HAL_GetTick();
    for (uint32_t chunk_num = 0; chunk_num < write_chunk_count; chunk_num++) {
        const ssize_t write_result = lfs_file_write(&LFS_filesystem, &file, write_buffer, write_chunk_size);
        for (uint32_t i = 0; i < write_chunk_size; i++) {
            // Weak checksum, but good enough
            expected_checksum ^= write_buffer[i];
        }
        if (write_result < 0) {
            snprintf(
                &response_str[strlen(response_str)],
                response_str_len - strlen(response_str),
                "Write failed. LFS_write_file return: %d\n", write_result);
            return 1;
        }
    }
    const uint32_t write_send_end_time = HAL_GetTick();
    snprintf(
        &response_str[strlen(response_str)],
        response_str_len - strlen(response_str),
        "Write: %lu ms\n",
        write_send_end_time - write_send_start_time);

    // Close the file
    const uint32_t close_start_time = HAL_GetTick();
    const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0) {
        snprintf(
            &response_str[strlen(response_str)],
            response_str_len - strlen(response_str),
            "Close failed. LFS_file_close return: %d\n", close_result);
        return 1;
    }
    const uint32_t close_end_time = HAL_GetTick();
    snprintf(
        &response_str[strlen(response_str)],
        response_str_len - strlen(response_str),
        "Write close: %lu ms\n", close_end_time - close_start_time);

    // Re-open the file for reading
    const uint32_t read_open_start_time = HAL_GetTick();
    const int8_t read_open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_RDONLY, &LFS_file_cfg);
    if (read_open_result < 0) {
        snprintf(
            &response_str[strlen(response_str)],
            response_str_len - strlen(response_str),
            "Read open failed. LFS_file_open return: %d\n", read_open_result);
        return 1;
    }
    const uint32_t read_open_end_time = HAL_GetTick();
    snprintf(
        &response_str[strlen(response_str)],
        response_str_len - strlen(response_str),
        "Read open: %lu ms\n", read_open_end_time - read_open_start_time);

    // Read
    const uint32_t read_start_time = HAL_GetTick();
    uint8_t read_buffer[write_chunk_size];
    uint8_t read_checksum = 0;
    for (uint32_t chunk_num = 0; chunk_num < write_chunk_count; chunk_num++) {
        const ssize_t read_result = lfs_file_read(&LFS_filesystem, &file, read_buffer, write_chunk_size);
        if (read_result < 0) {
            snprintf(
                &response_str[strlen(response_str)],
                response_str_len - strlen(response_str),
                "Read failed on chunk %lu. LFS_read_file return: %d\n",
                chunk_num, read_result);
            return 2;
        }
        for (uint32_t i = 0; i < write_chunk_size; i++) {
            read_checksum ^= read_buffer[i];
        }
    }
    const uint32_t read_end_time = HAL_GetTick();
    snprintf(
        &response_str[strlen(response_str)],
        response_str_len - strlen(response_str),
        "Read: %lu ms\n", read_end_time - read_start_time);

    // Close the file
    const uint32_t read_close_start_time = HAL_GetTick();
    const int8_t read_close_result = lfs_file_close(&LFS_filesystem, &file);
    if (read_close_result < 0) {
        snprintf(
            &response_str[strlen(response_str)],
            response_str_len - strlen(response_str),
            "Read close failed. LFS_file_close return: %d\n", read_close_result);
        return 1;
    }
    const uint32_t read_close_end_time = HAL_GetTick();
    snprintf(
        &response_str[strlen(response_str)],
        response_str_len - strlen(response_str),
        "Read close: %lu ms\n\n", read_close_end_time - read_close_start_time);
    
    // Verify checksum
    if (read_checksum != expected_checksum) {
        snprintf(
            &response_str[strlen(response_str)],
            response_str_len - strlen(response_str),
            "Checksum mismatch: expected %u, got %u\n", expected_checksum, read_checksum);
        return 3;
    }

    return 0;
}

/// @brief Benchmarks the write/read operations on the LittleFS file system for both cases: writing to a new file and writing to an existing file.
/// @details This function will write test data to a static filename, read it back, and verify the read data for both cases.
/// @param write_chunk_size Number of bytes to write in each chunk.
/// @param write_chunk_count Number of chunks to write.
/// @param response_str 
/// @param response_str_len 
/// @return 0 on success. >0 if there was an error.
uint8_t LFS_benchmark_write_read_single_and_new(uint16_t write_chunk_size, uint16_t write_chunk_count, char* response_str, uint16_t response_str_len) {
    response_str[0] = '\0';

    snprintf(
        &response_str[strlen(response_str)],
        response_str_len - strlen(response_str),
        "Benchmark writing to the same file: \n");
    uint8_t benchmark_same_file_result = LFS_benchmark_write_read(write_chunk_size, write_chunk_count, response_str, response_str_len, LFS_SINGLE_FILE);
    if(benchmark_same_file_result != 0) {
        return benchmark_same_file_result;
    }

    snprintf(
       &response_str[strlen(response_str)],
        response_str_len - strlen(response_str),
        "Benchmark writing to a new file: \n");
    uint8_t benchmark_new_file_result = LFS_benchmark_write_read(write_chunk_size, write_chunk_count, response_str, response_str_len, LFS_NEW_FILE);
    if(benchmark_new_file_result != 0) {
        return benchmark_same_file_result;
    }

    return 0;

}