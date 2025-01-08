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

/// @brief reads the value from the file "lifecycle/deploy_antenna_on_boot_enabled.bool"
/// @param read_value where the read value will be stored upon success 
/// @return 0 on success, LFS_ERR_NOENT if the file does not exist, <0 on failure 
static int16_t read_bool_from_deploy_antenna_on_boot_enabled_file (uint8_t* read_value) {
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
            );
            return mount_result;
        }
    }
    // lfs is mounted
    
    lfs_file_t file;
    int16_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, "lifecycle/deploy_antenna_on_boot_enabled.bool", LFS_O_RDONLY, &LFS_file_cfg);
    if (open_result == LFS_ERR_NOENT) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "File <lifecycle/deploy_antenna_on_boot_enabled.bool> does not exist."
        );
        return LFS_ERR_NOENT;
    }
    
    if (open_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "Error %d opening file <lifecycle/deploy_antenna_on_boot_enabled.bool>.",
            open_result
        );
        return open_result;
    }

    int16_t read_result = lfs_file_read(&LFS_filesystem, &file, read_value, sizeof(uint8_t));
    if (read_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "Error %d reading from file <lifecycle/deploy_antenna_on_boot_enabled.bool>.",
            read_result
        );
        return read_result;
    }

    int16_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL, 
            "Error %d closing file <lifecycle/deploy_antenna_on_boot_enabled.bool>.",
            close_result
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
            );
            return unmount_result;
        }
    }
    return 0;
}

/// @brief creates the file "lifecycle/deploy_antenna_on_boot_enabled.bool".
/// Creates the lifecycle directory if it does not exist.
/// write 1 to the file
/// @return 0 on success, <0 on failure
static int16_t create_deploy_antenna_on_boot_enabled_file_and_write_one() {
    int16_t mkdir_result = lfs_mkdir(&LFS_filesystem, "lifecycle");
    if(mkdir_result != LFS_ERR_EXIST && mkdir_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS,
            LOG_SEVERITY_ERROR,
            LOG_SINK_ALL,
            "Error %d creating lifecycle directory.",
            mkdir_result
        );
        return mkdir_result;
    }

    lfs_file_t file;
    int16_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, "lifecycle/deploy_antenna_on_boot_enabled.bool", LFS_O_WRONLY | LFS_O_CREAT, &LFS_file_cfg);
    if (open_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS,
            LOG_SEVERITY_ERROR,
            LOG_SINK_ALL,
            "Error %d opening file lifecycle/deploy_antenna_on_boot_enabled.bool.",
            open_result
        );
        return open_result;
    }

    uint8_t write_value = 1;
    int32_t write_result = lfs_file_write(&LFS_filesystem, &file, &write_value, sizeof(uint8_t));
    if (write_result < 0) {
        LOG_message(
            LOG_SYSTEM_LFS,
            LOG_SEVERITY_ERROR,
            LOG_SINK_ALL,
            "Error %d writing to file lifecycle/deploy_antenna_on_boot_enabled.bool.",
            write_result
        );
        return write_result;
    }

    int32_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS,
            LOG_SEVERITY_ERROR,
            LOG_SINK_ALL,
            "Error %d closing file lifecycle/deploy_antenna_on_boot_enabled.bool.",
            close_result
        );
        return close_result;
    }
    return 0;    
}    
    
//TODO: Implement this
static uint8_t eps_is_sufficiently_charged() {
    return 1;
}


/// @brief Attempts to arm and deploy the antennas 
/// @return 0 if mcu successfully transmitted the arm and deploy commands, <0 otherwise
/// @note WARNING: Care must be taken with using this function as it does not check if the eps is sufficiently charged!
int16_t START_deploy_antenna() {
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

    int8_t deploy_result = ANT_CMD_start_automated_sequential_deployment(ANT_I2C_BUS_A_MCU_A, 20);
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
    return 0;
}

/// @brief Attempts to arm and deploy the antennas if the EPS is sufficiently charged
/// @return 0 on success, <0 on failure 
int16_t START_deploy_antenna_if_sufficiently_charged() {
    if (!eps_is_sufficiently_charged()) {
        LOG_message(
            LOG_SYSTEM_ANTENNA_DEPLOY, 
            LOG_SEVERITY_WARNING, 
            LOG_SINK_ALL, 
            "EPS is not sufficiently charged. Deploying antennas will not occur."
        );
        return -1;
    }
    return START_deploy_antenna();
}

/// @brief Reads the file "lifecycle/deploy_antenna_on_boot_enabled.bool". If the file contains 1 or does not exist then it attempts to deploy the antennas.
/// If the file contains 0 or eps is not sufficiently then it does not attempt to deploy the antennas.
/// @return 0 on success, <0 on failure
int16_t START_read_config_and_deploy_antenna_accordingly() {

    //TODO: remove after validation
    LOG_message(
        LOG_SYSTEM_LFS, 
        LOG_SEVERITY_NORMAL, 
        LOG_SINK_ALL, 
        "Starting Antenna Deploy"
    );

    uint8_t deploy_antenna_on_boot_enabled;
    int16_t read_result = read_bool_from_deploy_antenna_on_boot_enabled_file(&deploy_antenna_on_boot_enabled);
    if (read_result != 0 && read_result != LFS_ERR_NOENT) {
        LOG_message(
            LOG_SYSTEM_LFS, 
            LOG_SEVERITY_ERROR, 
            LOG_SINK_ALL, 
            "Error %d reading from file <lifecycle/deploy_antenna_on_boot_enabled.bool>.",
            read_result
        );
        return read_result;
    }

    if (read_result == LFS_ERR_NOENT) {
        int16_t create_result = create_deploy_antenna_on_boot_enabled_file_and_write_one();
        if (create_result != 0) {
            LOG_message(
                LOG_SYSTEM_LFS, 
                LOG_SEVERITY_ERROR, 
                LOG_SINK_ALL, 
                "Error %d creating file <lifecycle/deploy_antenna_on_boot_enabled.bool>.",
                create_result
            );
            return create_result;
        }
        deploy_antenna_on_boot_enabled = 1;
    }

    if (deploy_antenna_on_boot_enabled == 0) {
        return 0;
    }
    return START_deploy_antenna_if_sufficiently_charged();
}
