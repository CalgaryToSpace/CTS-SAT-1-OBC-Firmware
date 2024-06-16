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

/*---------------------------FUNCTIONS---------------------------*/
int8_t LFS_format() {return 0;}
int8_t LFS_mount() {return 0;}
int8_t LFS_unmount() {return 0;}
int8_t LFS_list_directory(char *root_directory) {return 0;}
int8_t LFS_delete_file(char *file_name) {return 0;}
int8_t LFS_make_directory(char *dir_name) {return 0;}
int8_t LFS_write_file(char *file_name, uint8_t *write_buffer, uint32_t write_buffer_len) {return 0;}
int8_t LFS_read_file(char *file_name, uint8_t *read_buffer, uint32_t read_buffer_len) {return 0;}

#endif /* __INCLUDE_GUARD__LITTLEFS_HELPER_H__ */
