#include "startup_sequence/antenna_deploy_startup.h"
#include "littlefs/littlefs_helper.h"
#include "littlefs/lfs.h"
#include "log/log.h"
#include "antenna_deploy_drivers/ant_commands.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
/// @brief Attempts to read to the file "lifecycle/deploy_antenna_on_boot_enabled.bool".
/// If 1 is stored in the file then it attempts to deploy the antenna. If the file did not
/// exist: then it creates the file, stores a 1 in it, and attempts to deploy.
/// @return 0 on success, <0 on failure

//TODO: unmount filesystem if error occurs
static int16_t START_check_file_read_bool(const char* file_path, uint8_t* read_value) {
    uint8_t unmount_on_completion = 0;
    if ( !LFS_is_lfs_mounted ) {
        unmount_on_completion = 1;
        int32_t mount_result = lfs_mount(&LFS_filesystem, &LFS_cfg); 
        if (mount_result != 0) {
            LOG_message(
                LOG_SYSTEM_LFS, 
                LOG_SEVERITY_ERROR, 
                LOG_SINK_ALL, 
                "Error %d mounting LittleFS.",
                mount_result
            )
            return mount_result;
        }
    }
    // lfs is mounted
    
    lfs_file_t file;
    int16_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_path, LFS_O_RDONLY, &LFS_file_cfg);
    if (open_result == LFS_ERR_NOENT) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "File %s does not exist. ",
            file_path
        );
        return LFS_ERR_NOENT;
    }
    
    if (open_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "Error %d opening file %s.",
            open_result,
            file_path
        );
        return open_result;
    }

    int32_t read_result = lfs_file_read(&LFS_filesystem, &file, read_value, sizeof(uint8_t));
    if (read_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "Error %d reading from file %s.",
            read_result,
            file_path
        );
        return read_result;
    }

    int32_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "Error %d closing file %s.",
            close_result,
            file_path
        );
        return close_result;
    }

    if (unmount_on_completion) {
        int16_t unmount_result = lfs_unmount(&LFS_filesystem);
        if (unmount_result != 0) {  
            LOG_message(
                LOG_SYSTEM_LFS, 
                LOG_SEVERITY_ERROR, 
                LOG_SINK_ALL, 
                "Error %d unmounting LittleFS.",
                unmount_result
            )
            return unmount_result;
        }
    }
    return 0;
}

static int16_t START_create_missing_directory_and_file() {
    
    
}

int16_t START_antenna_deploy() {
    //TODO: remove after validation
    LOG_message(
        LOG_SYSTEM_LFS, 
        LOG_SEVERITY_NORMAL, 
        LOG_SINK_ALL, 
        "Starting Antenna Deploy"
    );
    const int16_t mkdir_result = lfs_mkdir(&LFS_filesystem, "lifecycle");
    if(mkdir_result == LFS_ERR_EXIST) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "lifecycle directory already exists. Skipping creation."
        );
    }

    if(mkdir_result != LFS_ERR_EXIST && mkdir_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "Error %d creating lifecycle directory.",
            mkdir_result
        );
        return -2;
    }

    //TODO: remove after validation
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
    lfs_file_t file;
    int32_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, "lifecycle/deploy_antenna_on_boot_enabled.bool", LFS_O_RDONLY, &LFS_file_cfg);
    if ( open_result == LFS_ERR_NOENT) {

        // if file doesn't exist, create it and open for writing and write 1 
        open_result = lfs_file_opencfg(&LFS_filesystem, &file, "lifecycle/deploy_antenna_on_boot_enabled.bool", LFS_O_WRONLY | LFS_O_CREAT, &LFS_file_cfg);
        if(open_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_WARNING, 
            LOG_SINK_ALL, 
            "Error %d opening/creating deploy_antenna_on_boot_enabled.bool file.",
            open_result
        );
        return open_result;
        }
          
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
            return write_result;
        }

        //TODO: remove after validation
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "file did not exist, created deploy_antenna_on_boot_enabled.bool file."
        );
        
        int32_t close_result = lfs_file_close(&LFS_filesystem, &file);
        if(close_result != 0) {
            LOG_message(
                LOG_SYSTEM_LFS, 
                LOG_SEVERITY_WARNING, 
                LOG_SINK_ALL, 
                "Error %d closing newly created deploy_antenna_on_boot_enabled.bool file.",
                close_result
            );
            return close_result;
        }

        // the file has been created and 1 has been written to it, open for reading
        open_result = lfs_file_opencfg(&LFS_filesystem, &file, "lifecycle/deploy_antenna_on_boot_enabled.bool", LFS_O_RDONLY, &LFS_file_cfg);
        if (open_result != 0) {
            LOG_message(
                LOG_SYSTEM_LFS, 
                LOG_SEVERITY_WARNING, 
                LOG_SINK_ALL, 
                "Error %d opening/creating deploy_antenna_on_boot_enabled.bool file.",
                open_result
            );
            return -5 ;
        }
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "new file created, 1 written, opend for reading."
        );
    }
        

    //TODO: remove after validation
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
        return num_bytes_read;
    }
    //TODO: remove after validation
    if(num_bytes_read == 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "deploy_antenna_on_boot_enabled.bool file is empty."
        );
    }
    //TODO: remove after validation
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
    //reading the file is done now,  close the file and unmount lfs if needed
    int close_status = lfs_file_close(&LFS_filesystem, &file);
    if( close_status != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_ERROR, 
            LOG_SINK_ALL, 
            "Error %d closing deploy_antenna_on_boot_enabled.bool file.",
            close_status
        );
        return close_status;
    }

    if(LFS_unmount_on_completion) {
        int32_t unmount_status = lfs_unmount(&LFS_filesystem);
        if (unmount_status != 0) {
            LOG_message(
                LOG_SYSTEM_LFS, 
                LOG_SEVERITY_ERROR, 
                LOG_SINK_ALL, 
                "Error %d unmounting lfs.",
                unmount_status
            );
            return unmount_status;
        }
    }

    if(deploy_antenna_on_boot_enabled == 1) {
        //TODO: which mcu should be armed on the antenna deploy system? Error handling. How long to activate?
        int8_t arm_result = ANT_CMD_arm_antenna_system(ANT_I2C_BUS_A_MCU_A);
        if (arm_result != 0) {
            LOG_message(
                LOG_SYSTEM_ANTENNA_DEPLOY, 
                LOG_SEVERITY_ERROR, 
                LOG_SINK_ALL, 
                "Error %d arming antenna deploy system.",
                arm_result
            );
            return arm_result;
        }

        int8_t deploy_result = ANT_CMD_start_automated_sequential_deployment(ANT_I2C_BUS_A_MCU_A, 7);
        if (deploy_result != 0) {
            LOG_message(
                LOG_SYSTEM_ANTENNA_DEPLOY, 
                LOG_SEVERITY_ERROR, 
                LOG_SINK_ALL, 
                "Error %d deploying antennas.",
                deploy_result
            );
            return deploy_result;
        }
    }

    return 0;
}