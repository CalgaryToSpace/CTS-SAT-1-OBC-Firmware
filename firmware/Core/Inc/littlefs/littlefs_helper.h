/**
 * littlefs_helper.h
 *
 * Created on: May 2, 2024
 *     Author: Saksham Puri
 */

#ifndef INCLUDE_GUARD__LITTLEFS_HELPER_H__
#define INCLUDE_GUARD__LITTLEFS_HELPER_H__

/*-----------------------------INCLUDES-----------------------------*/

#include <stdint.h>

#include "littlefs/flash_driver.h"
#include "littlefs/lfs.h"

extern lfs_t LFS_filesystem; // LittleFS filesystem object; traditionally called `lfs`
extern struct lfs_config LFS_cfg;
extern struct lfs_file_config LFS_file_cfg;
extern uint8_t LFS_is_lfs_mounted;

/// @brief The maximum path length supported by CTS-SAT-1, including the null terminator.
/// @note LittleFS supports up to 255 bytes by default, but we'll restrict a bit under.
#define LFS_MAX_PATH_LENGTH 250

/*---------------------------FUNCTIONS---------------------------*/
int8_t LFS_format();
int8_t LFS_mount();
int8_t LFS_unmount();
int8_t LFS_ensure_mounted();
int8_t LFS_ensure_unmounted();
int8_t LFS_list_directory(const char root_directory[], uint16_t offset, int16_t count);
int8_t LFS_make_directory(const char dir_name[]);
int8_t LFS_delete_file(const char file_name[]);
int8_t LFS_recursively_delete_directory(const char directory_path[]);
int8_t LFS_write_file(const char file_name[], uint8_t *write_buffer, uint32_t write_buffer_len);
int8_t LFS_write_file_with_offset(const char file_name[], lfs_soff_t offset, uint8_t *write_buffer, uint32_t write_buffer_len);
int8_t LFS_append_file(const char file_name[], uint8_t *write_buffer, uint32_t write_buffer_len);
lfs_ssize_t LFS_read_file(const char file_name[], lfs_soff_t offset, uint8_t *read_buffer, uint32_t read_buffer_size);
lfs_ssize_t LFS_file_size(const char file_name[]);

#endif /* INCLUDE_GUARD__LITTLEFS_HELPER_H__ */
