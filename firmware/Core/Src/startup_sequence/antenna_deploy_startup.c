#include "startup_sequence/antenna_deploy_startup.h"
#include "littlefs/littlefs_helper.h"
#include "littlefs/lfs.h"
#include "log/log.h"
uint8_t START_antenna_deploy() {
    LOG_message(
        LOG_SYSTEM_LFS, 
        LOG_SEVERITY_NORMAL, 
        LOG_SINK_ALL, 
        "Starting Antenna Deploy"
    );
    uint8_t LFS_unmount_on_completion = 0;
    if (!LFS_is_lfs_mounted) {
        lfs_mount(&LFS_filesystem, &LFS_cfg);
        LFS_unmount_on_completion = 1;
    }

    // Create lifecycle directory if it doesn't exist
    const int32_t mkdir_result = lfs_mkdir(&LFS_filesystem, "lifecycle");
    if(mkdir_result != LFS_ERR_EXIST && mkdir_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "Error %d creating lifecycle directory.",
            mkdir_result
        );
        //TODO: what happens if directory creation fails?
    }

    lfs_file_t file;
    int32_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, "lifecycle/deploy_antenna_on_boot_enabled.bool", LFS_O_RDONLY, &LFS_file_cfg);
    if ( open_result == LFS_ERR_NOENT) {
        // Create file if it doesn't exist
        open_result = lfs_file_opencfg(&LFS_filesystem, &file, "lifecycle/deploy_antenna_on_boot_enabled.bool", LFS_O_RDONLY, &LFS_file_cfg);
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "file did not exist, created deploy_antenna_on_boot_enabled.bool file."
        );
    } 
    else if(open_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_WARNING, 
            LOG_SINK_ALL, 
            "Error %d opening deploy_antenna_on_boot_enabled.bool file.",
            open_result
        );
    }

    lfs_file_rewind(&LFS_filesystem, &file);

    uint8_t deploy_antenna_on_boot_enabled;

    lfs_file_read(&LFS_filesystem, &file, &deploy_antenna_on_boot_enabled, sizeof(deploy_antenna_on_boot_enabled));

    lfs_file_close(&LFS_filesystem, &file);

    if(LFS_unmount_on_completion) {
        lfs_unmount(&LFS_filesystem);
    }
    return 0;
}