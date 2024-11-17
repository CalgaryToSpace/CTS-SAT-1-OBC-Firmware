#include "startup_sequence/antenna_deploy_startup.h"
#include "littlefs/littlefs_helper.h"
#include "littlefs/lfs.h"
#include "log/log.h"
#include "antenna_deploy_drivers/ant_commands.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
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
    uint8_t file_did_exist = 1;
    if ( open_result == LFS_ERR_NOENT) {
        // file doesn't exist, create it and open for writing 
        file_did_exist = 0;
        open_result = lfs_file_opencfg(&LFS_filesystem, &file, "lifecycle/deploy_antenna_on_boot_enabled.bool", LFS_O_WRONLY | LFS_O_CREAT, &LFS_file_cfg);
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "file did not exist, created deploy_antenna_on_boot_enabled.bool file."
        );
        if (open_result == 0)
        {
            uint8_t buff = 1;
            int32_t write_result = lfs_file_write(&LFS_filesystem, &file, &buff, sizeof(buff));
            if(write_result < 0) {
                LOG_message(
                    LOG_SYSTEM_LFS, 
                    LOG_SEVERITY_WARNING, 
                    LOG_SINK_ALL, 
                    "Error %d writing to newly created deploy_antenna_on_boot_enabled.bool file.",
                    write_result
                );
                //TODO: what happens if write fails? 
            }
            
            int32_t close_result = lfs_file_close(&LFS_filesystem, &file);
            if(close_result < 0) {
            LOG_message(
                    LOG_SYSTEM_LFS, 
                    LOG_SEVERITY_WARNING, 
                    LOG_SINK_ALL, 
                    "Error %d closing newly created deploy_antenna_on_boot_enabled.bool file.",
                    close_result
                );
                //TODO: what happens if close fails?
            }

            // the fil has been created and 1 (assuming no errors occured) has been written to it, open for reading
            open_result = lfs_file_opencfg(&LFS_filesystem, &file, "lifecycle/deploy_antenna_on_boot_enabled.bool", LFS_O_RDONLY, &LFS_file_cfg);
        }
        
    } 
    //TODO: should this be else if?
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

    uint8_t deploy_antenna_on_boot_enabled;
    int32_t num_bytes_read = lfs_file_read(&LFS_filesystem, &file, &deploy_antenna_on_boot_enabled, sizeof(deploy_antenna_on_boot_enabled));
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
            "read %d bytes from deploy_antenna_on_boot_enabled.bool. Result: %d",
            num_bytes_read,
            deploy_antenna_on_boot_enabled
        );
    }
    //reading the file is done now, unmount fs and close the file
    lfs_file_close(&LFS_filesystem, &file);
    if(LFS_unmount_on_completion) {
        lfs_unmount(&LFS_filesystem);
    }

    if(deploy_antenna_on_boot_enabled == 1) {
        //TODO: which mcu should be armed on the antenna deploy system? Error handling
     ANT_CMD_arm_antenna_system(ANT_I2C_BUS_A_MCU_A);
     ANT_CMD_start_automated_sequential_deployment(ANT_I2C_BUS_A_MCU_A, 7);
    }


    return 0;
}