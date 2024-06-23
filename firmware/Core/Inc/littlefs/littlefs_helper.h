/**
 * littlefs_helper.h
 *
 * Created on: May 2, 2024
 *     Author: Saksham Puri
 */

#ifndef __INCLUDE_GUARD__LITTLEFS_HELPER_H__
#define __INCLUDE_GUARD__LITTLEFS_HELPER_H__

/*-----------------------------INCLUDES-----------------------------*/

#include <stdint.h>

#include "littlefs/flash_driver.h"
#include "littlefs/lfs.h"

/*---------------------------FUNCTIONS---------------------------*/
int8_t LFS_format();
int8_t LFS_mount();
int8_t LFS_unmount();
int8_t LFS_list_directory(char *root_directory);
int8_t LFS_delete_file(char *file_name);
int8_t LFS_make_directory(char *dir_name);
int8_t LFS_write_file(char *file_name, uint8_t *write_buffer, uint32_t write_buffer_len);
int8_t LFS_append_file(char *file_name, uint8_t *write_buffer, uint32_t write_buffer_len);
lfs_ssize_t LFS_read_file(char *file_name, lfs_soff_t offset, uint8_t *read_buffer, uint32_t read_buffer_len);
lfs_ssize_t LFS_file_size(char *file_name);

#endif /* __INCLUDE_GUARD__LITTLEFS_HELPER_H__ */
