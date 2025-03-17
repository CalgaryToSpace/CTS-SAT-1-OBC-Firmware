#include "eps_drivers/eps_power_management.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "rtos_tasks/rtos_eps_tasks.h"
#include "eps_drivers/eps_types_to_json.h"
#include "eps_drivers/eps_commands.h"
#include "log/log.h"


uint16_t current_mA_threshhold[32]= 
    {   
        1000,   //EPS_CHANNEL_VBATT_STACK = 0, // CH0
        1000,   //EPS_CHANNEL_5V_STACK = 1, // CH1
        1000,   //EPS_CHANNEL_5V_CH2_UNUSED = 2, // CH2
        1000,   //EPS_CHANNEL_5V_CH3_UNUSED = 3, // CH3
        1000,   //EPS_CHANNEL_5V_MPI = 4, // CH4
        1000,   //EPS_CHANNEL_3V3_STACK = 5, // CH5
        1000,   //EPS_CHANNEL_3V3_CAMERA = 6, // CH6
        1000,   //EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY = 7, // CH7
        1000,   //EPS_CHANNEL_3V3_LORA_MODULE = 8, // CH8
        1000,   //EPS_CHANNEL_VBATT_CH9_UNUSED = 9, // CH9
        1000,   //EPS_CHANNEL_VBATT_CH10_UNUSED = 10, // CH10
        1000,   //EPS_CHANNEL_VBATT_CH11_UNUSED = 11, // CH11
        1000,   //EPS_CHANNEL_12V_MPI = 12, // CH12
        1000,   //EPS_CHANNEL_12V_BOOM = 13, // CH13
        1000,   //EPS_CHANNEL_5V_CH14_UNUSED = 14, // CH14
        1000,   //EPS_CHANNEL_5V_CH15_UNUSED = 15, // CH15
        1000,   //EPS_CHANNEL_28V6_CH16_UNUSED = 16, // CH16
        0,      //EPS_CHANNEL_UNKNOWN = 255
        0,      //EPS_CHANNEL_UNKNOWN = 255
        0,      //EPS_CHANNEL_UNKNOWN = 255
        0,      //EPS_CHANNEL_UNKNOWN = 255
        0,      //EPS_CHANNEL_UNKNOWN = 255
        0,      //EPS_CHANNEL_UNKNOWN = 255
        0,      //EPS_CHANNEL_UNKNOWN = 255
        0,      //EPS_CHANNEL_UNKNOWN = 255
        0,      //EPS_CHANNEL_UNKNOWN = 255
        0,      //EPS_CHANNEL_UNKNOWN = 255
        0,      //EPS_CHANNEL_UNKNOWN = 255
        0,      //EPS_CHANNEL_UNKNOWN = 255
        0,      //EPS_CHANNEL_UNKNOWN = 255
        0,      //EPS_CHANNEL_UNKNOWN = 255
        0       //EPS_CHANNEL_UNKNOWN = 255
    };

/// @brief Monitors the power consumption of each channel and logs the data in JSON format.
///
/// @details This function will obtain the PDU housekeeping data and log the data in JSON format.
///          If this is the first time the function is called, it will save the data for future comparisons.
///          It will then check if the power consumption has increased or decreased by more than a certain threshold.
///          If it has, it will disable the channel and log an error message.
///          The function will log an error message if the channel is disabled due to a power issue.
/// 
/// @return 0 if the function was successful, 1 if there was an error.
uint8_t EPS_monitor_and_disable_overcurrent_channels() {

    EPS_struct_pdu_housekeeping_data_eng_t EPS_pdu_housekeeping_data_eng;

    //Obtain the PDU data
    uint8_t pdu_status = EPS_CMD_get_pdu_housekeeping_data_eng(&EPS_pdu_housekeeping_data_eng);
    if (pdu_status != 0) {
        LOG_message(
            LOG_SYSTEM_EPS,
            LOG_SEVERITY_ERROR,
            LOG_SINK_ALL,
            "EPS_CMD_get_pdu_housekeeping_data_eng() -> Error: %d",
            pdu_status
        );
        return 2;
    }

    uint8_t logging_status = EPS_log_pdu_json(&EPS_pdu_housekeeping_data_eng);

    EPS_channel_management(&EPS_pdu_housekeeping_data_eng);

    return logging_status;
}

/// @brief Logs the PDU housekeeping data in JSON format
/// @param[in] EPS_pdu_housekeeping_data_eng PDU housekeeping data to be logged
/// @note This function will log the PDU housekeeping data in JSON format to the error log.   
///     If the conversion to JSON fails, it will log an error with the error status.    
///     Otherwise, it will log the JSON string to the error log.
uint8_t EPS_log_pdu_json(const EPS_struct_pdu_housekeeping_data_eng_t *EPS_pdu_housekeeping_data_eng) {

    char json_str[800];

    //Power Logging                                                        
    int8_t pdu_TO_JSON_status = EPS_struct_pdu_housekeeping_data_eng_TO_json(EPS_pdu_housekeeping_data_eng, json_str, 1000);

    if (pdu_TO_JSON_status != 0) {
        LOG_message(
            LOG_SYSTEM_EPS,
            LOG_SEVERITY_ERROR,
            LOG_SINK_ALL,
            "EPS_struct_pdu_housekeeping_data_eng_TO_json() -> Error: %d",
            pdu_TO_JSON_status
        );
        return 1;
    }
    else {
        LOG_message(
            LOG_SYSTEM_EPS,
            LOG_SEVERITY_NORMAL,
            LOG_SINK_ALL,
            "EPS PDU housekeeping data:\n %s",
            json_str
        );
    }
    return 0;
}

/// @brief Monitor the power consumption of each channel and disable any channels that exceed a certain threshold
/// @param[in] EPS_pdu_housekeeping_data_eng PDU housekeeping data with the current power consumption of each channel
/// @param[in] prev_EPS_pdu_housekeeping_data_eng Previous PDU housekeeping data with the previous power consumption of each chan///
/// @note This function will iterate over each channel and check if the power consumption has increased or decreased by more than a certain threshold.
///       If the power consumption has increased or decreased by more than the threshold, the channel will be disabled.
///       The function will log an error message if the channel is disabled due to a power issue.
void EPS_channel_management(const EPS_struct_pdu_housekeeping_data_eng_t *EPS_pdu_housekeeping_data_eng) {

    EPS_vpid_eng_t vpid_eng[32];
    memcpy(vpid_eng, EPS_pdu_housekeeping_data_eng->vip_each_channel, sizeof(EPS_vpid_eng_t) * 32);

    uint32_t ch_bitfield = (EPS_pdu_housekeeping_data_eng->stat_ch_ext_on_bitfield << 16) & EPS_pdu_housekeeping_data_eng->stat_ch_on_bitfield; 

    //Power Monitoring
    for (int channel = 0; channel < 32; channel++) {

        if ((ch_bitfield & 1) && (current_mA_threshhold[channel]) //Check if channel is enabled
            && ((uint16_t) vpid_eng[channel].current_mA > current_mA_threshhold[channel])) {

            uint8_t disable_result = EPS_CMD_output_bus_channel_off(channel);

            if (disable_result != 0) {
                LOG_message(
                    LOG_SYSTEM_EPS,
                    LOG_SEVERITY_ERROR,
                    LOG_SINK_ALL,
                    "EPS_CMD_output_bus_channel_off(%d) -> Error: %d", channel, disable_result
                );
            } 
            else {
                LOG_message(
                    LOG_SYSTEM_EPS,
                    LOG_SEVERITY_ERROR,
                    LOG_SINK_ALL,
                    "Channel %d was turned off. Due to a overcurrent oveflow.", channel
                );
            }       
        }
        ch_bitfield = ch_bitfield >> 1;

    }
}

void EPS_CMD_power_management_set_current_threshold(uint8_t channel, uint16_t threshold) {
    current_mA_threshhold[channel] = threshold;
}