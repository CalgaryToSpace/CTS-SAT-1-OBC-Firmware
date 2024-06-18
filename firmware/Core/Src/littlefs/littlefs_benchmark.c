
#include "main.h"
#include "littlefs/littlefs_benchmark.h"
#include "littlefs/littlefs_helper.h"

#include <string.h>
#include <stdio.h>

uint8_t LFS_benchmark_write_read(uint16_t write_chunk_size, uint16_t write_chunk_count, char* response_str, uint16_t response_str_len) {
    const char file_name[] = "benchmark_test.txt";
    response_str[0] = '\0';

    uint8_t expected_checksum = 0;

    // Write
    uint8_t write_buffer[write_chunk_size];
    for (uint32_t i = 0; i < write_chunk_size; i++) {
        const uint8_t b = (i + 42) % 256;
        write_buffer[i] = b;
    }
    const uint32_t write_send_start_time = HAL_GetTick();
    for (uint32_t chunk_num = 0; chunk_num < write_chunk_count; chunk_num++) {
        const int8_t write_result = LFS_write_file(file_name, write_buffer, write_chunk_size);
        for (uint32_t i = 0; i < write_chunk_size; i++) {
            // bad checksum, but good enough
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

    // Read
    const uint32_t read_start_time = HAL_GetTick();
    uint8_t read_buffer[write_chunk_size];
    uint8_t read_checksum = 0;
    for (uint32_t chunk_num = 0; chunk_num < write_chunk_count; chunk_num++) {
        const int8_t read_result = LFS_read_file(file_name, read_buffer, write_chunk_size);
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
    
    if (read_checksum != expected_checksum) {
        snprintf(
            &response_str[strlen(response_str)],
            response_str_len - strlen(response_str),
            "Checksum mismatch: expected %u, got %u\n", expected_checksum, read_checksum);
        return 3;
    }

    return 0;
}

