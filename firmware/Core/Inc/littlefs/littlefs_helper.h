/**
 * littlefs_helper.h
 *
 * Created on: May 2, 2024
 *     Author: Saksham Puri
 */

#ifndef INC_LITTELFS_HELPER_H_
#define INC_LITTELFS_HELPER_H_

/*-----------------------------INCLUDES-----------------------------*/
#include "flash_driver.h"

/*---------------------------SETUP FUNCTIONS---------------------------*/
void LFS_config();
int LFS_block_device_read(const struct lfs_config *, lfs_block_t, lfs_off_t, void *, lfs_size_t);
int LFS_block_device_prog(const struct lfs_config *, lfs_block_t, lfs_off_t, const void *, lfs_size_t);
int LFS_block_device_erase(const struct lfs_config *, lfs_block_t);
int LFS_block_device_sync(const struct lfs_config *);

/*---------------------------FUNCTIONS---------------------------*/
void LFS_INITIALIZE();
int8_t LFS_FORMAT();
int8_t LFS_MOUNT();
int8_t LFS_UNMOUNT();
int8_t LFS_WRITE_FILE(char *, void *, uint32_t);
int8_t LFS_READ_FILE(char *, void *, uint32_t);

#endif /* INC_LITTELFS_HELPER_H_ */
