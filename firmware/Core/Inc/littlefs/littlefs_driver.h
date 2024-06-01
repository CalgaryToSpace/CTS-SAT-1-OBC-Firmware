
#ifndef __INCLUDE_GUARD__LITTLEFS_DRIVER_H__
#define __INCLUDE_GUARD__LITTLEFS_DRIVER_H__

#include "lfs.h"


/*---------------------------SETUP FUNCTIONS---------------------------*/
int LFS_block_device_read(const struct lfs_config *, lfs_block_t, lfs_off_t, void *, lfs_size_t);
int LFS_block_device_prog(const struct lfs_config *, lfs_block_t, lfs_off_t, const void *, lfs_size_t);
int LFS_block_device_erase(const struct lfs_config *, lfs_block_t);
int LFS_block_device_sync(const struct lfs_config *);


#endif /* __INCLUDE_GUARD__LITTLEFS_DRIVER_H__ */
