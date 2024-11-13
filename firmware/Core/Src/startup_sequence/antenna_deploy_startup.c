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

    if(mkdir_result == LFS_ERR_EXIST) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "lifecycle directory already exists. Skipping creation."
        );
    }

    if(mkdir_result == 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "lifecycle directory created."
        );
    }
    // At this point the lifecycle directory exists or has been created

    // Create deploy_antenna_on_boot_enabled.bool file if it doesn't exist
    //
    lfs_file_t file;
    int32_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, "lifecycle/deploy_antenna_on_boot_enabled.bool", LFS_O_RDONLY, &LFS_file_cfg);
    if ( open_result == LFS_ERR_NOENT) {
        // Create file if it doesn't exist
        open_result = lfs_file_opencfg(&LFS_filesystem, &file, "lifecycle/deploy_antenna_on_boot_enabled.bool", LFS_O_RDONLY | LFS_O_CREAT, &LFS_file_cfg);
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
            "Error %d opening/creating deploy_antenna_on_boot_enabled.bool file.",
            open_result
        );
    }
    else if (open_result == 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "deploy_antenna_on_boot_enabled.bool file opened."
        );
    }
    // At this point the depoy_antenna_on_boot_enabled.bool file is open 

    uint8_t deploy_antenna_on_boot_enabled[10];
    size_t num_bytes_read = lfs_file_read(&LFS_filesystem, &file, &deploy_antenna_on_boot_enabled, sizeof(deploy_antenna_on_boot_enabled));

    if(num_bytes_read < 0) { 
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_WARNING, 
            LOG_SINK_ALL, 
            "Error %d reading deploy_antenna_on_boot_enabled.bool file.",
            num_bytes_read
        );
    }
    if(num_bytes_read == 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "deploy_antenna_on_boot_enabled.bool file is empty."
        );
    }
    if(num_bytes_read > 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "read %d bytes from deploy_antenna_on_boot_enabled.bool. Result: %s",
            num_bytes_read,
            deploy_antenna_on_boot_enabled
        );
    }

    //uint8_t deploy_antenna_on_boot_enabled;

    //lfs_file_read(&LFS_filesystem, &file, &deploy_antenna_on_boot_enabled, sizeof(deploy_antenna_on_boot_enabled));

    // lfs_file_close(&LFS_filesystem, &file);

    if(LFS_unmount_on_completion) {
        lfs_unmount(&LFS_filesystem);
    }
    return 0;
}