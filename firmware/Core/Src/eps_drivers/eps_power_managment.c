
#include "eps_drivers/eps_power_management.h"

EPS_struct_pdu_housekeeping_data_eng_t prev_EPS_pdu_housekeeping_data_eng;
uint16_t power_cW_threshhold = 1000;
uint16_t voltage_mV_threshhold = 1000;
uint16_t current_mA_threshhold[32] = {1000, 1000, 1000, 1000, 1000, 1000,           //TODO: Set PDU thresh hold
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};
uint32_t disableable_channels = 0x0001ffff;

/**
 * @brief Monitors the power consumption of each channel and logs the data in JSON format.
 *
 * @details This function will obtain the PDU housekeeping data and log the data in JSON format.
 *          If this is the first time the function is called, it will save the data for future comparisons.
 *          It will then check if the power consumption has increased or decreased by more than a certain threshold.
 *          If it has, it will disable the channel and log an error message.
 *          The function will log an error message if the channel is disabled due to a power issue.
 *
 * @return 0 if the function was successful, 1 if there was an error.
 */
uint8_t EPS_power_monitoring() {
    
    static uint8_t saved_pdu = 0;

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

    //Save the first PDU data
    if (!saved_pdu) {
        prev_EPS_pdu_housekeeping_data_eng = EPS_pdu_housekeeping_data_eng;
        
        saved_pdu = 1;
        return 0;
    }

    uint8_t logging_status = EPS_log_pdu_json(&EPS_pdu_housekeeping_data_eng);

    EPS_channel_managment(&EPS_pdu_housekeeping_data_eng, &prev_EPS_pdu_housekeeping_data_eng);

    prev_EPS_pdu_housekeeping_data_eng = EPS_pdu_housekeeping_data_eng;
    
    return logging_status;
}


/**
 * @brief Logs the PDU housekeeping data in JSON format
 *
 * @param[in] EPS_pdu_housekeeping_data_eng PDU housekeeping data to be logged
 *
 * @note This function will log the PDU housekeeping data in JSON format to the error log.
 *       If the conversion to JSON fails, it will log an error with the error status.
 *       Otherwise, it will log the JSON string to the error log.
 */
uint8_t EPS_log_pdu_json(const EPS_struct_pdu_housekeeping_data_eng_t *EPS_pdu_housekeeping_data_eng) {

    char json_str[1000];

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

/**
 * @brief Monitor the power consumption of each channel and disable any channels that exceed a certain threshold
 *
 * @param[in] EPS_pdu_housekeeping_data_eng PDU housekeeping data with the current power consumption of each channel
 * @param[in] prev_EPS_pdu_housekeeping_data_eng Previous PDU housekeeping data with the previous power consumption of each channel
 *
 * @note This function will iterate over each channel and check if the power consumption has increased or decreased by more than a certain threshold.
 *       If the power consumption has increased or decreased by more than the threshold, the channel will be disabled.
 *       The function will log an error message if the channel is disabled due to a power issue.
 */
void EPS_channel_managment(const EPS_struct_pdu_housekeeping_data_eng_t *EPS_pdu_housekeeping_data_eng, const EPS_struct_pdu_housekeeping_data_eng_t *prev_EPS_pdu_housekeeping_data_eng) {

    EPS_vpid_eng_t vpid_eng[32];
    memcpy(vpid_eng, EPS_pdu_housekeeping_data_eng->vip_each_channel, sizeof(EPS_vpid_eng_t) * 32);

    EPS_vpid_eng_t prev_vpid_eng[32];
    memcpy(prev_vpid_eng, prev_EPS_pdu_housekeeping_data_eng->vip_each_channel, sizeof(EPS_vpid_eng_t) * 32);

    uint32_t ch_bitfield = (EPS_pdu_housekeeping_data_eng->stat_ch_ext_on_bitfield << 16) & EPS_pdu_housekeeping_data_eng->stat_ch_on_bitfield;

    uint32_t prev_ch_bitfield = (prev_EPS_pdu_housekeeping_data_eng->stat_ch_ext_on_bitfield << 16) & prev_EPS_pdu_housekeeping_data_eng->stat_ch_on_bitfield; 

    uint32_t modifiable_disableable_bitfield = disableable_channels;
    //Power Monitoring
    for (int channel = 0; channel < 32; channel++) {

        if ((ch_bitfield & 1) && (prev_ch_bitfield & 1) && (modifiable_disableable_bitfield & 1)//Check if channel is enabled
            && (vpid_eng[channel].power_cW - prev_vpid_eng[channel].power_cW > power_cW_threshhold             //TODO: Set PDU thresh hold
            || vpid_eng[channel].voltage_mV - prev_vpid_eng[channel].voltage_mV > voltage_mV_threshhold
            || vpid_eng[channel].power_cW - prev_vpid_eng[channel].power_cW < -power_cW_threshhold
            || vpid_eng[channel].voltage_mV - prev_vpid_eng[channel].voltage_mV < -voltage_mV_threshhold
            || vpid_eng[channel].current_mA > current_mA_threshhold[channel])) {

            uint8_t disable_result = EPS_CMD_output_bus_channel_off(channel);

            char log_msg[50];
            (disable_result != 0) ? sprintf(log_msg, "EPS_CMD_output_bus_channel_off(%d) -> Error: %d", channel, disable_result) 
                : sprintf(log_msg, "Channel %d was turned off. Due to a power issue.", channel);

            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL,
                log_msg
            );
        }
        ch_bitfield = ch_bitfield >> 1;
        prev_ch_bitfield = prev_ch_bitfield >> 1;
        modifiable_disableable_bitfield = modifiable_disableable_bitfield >> 1;
    }
}


/**
 * @brief Sets the power and voltage thresholds for power monitoring.
 *
 * @param[in] power_cW New power threshold in centiwatts.
 * @param[in] voltage_mV New voltage threshold in millivolts.
 *
 * @details This function will set the power and voltage thresholds for power monitoring.
 *          The power monitoring is done by the function EPS_power_monitoring.
 *          The thresholds can be set by the user to a specific value.
 */
void EPS_power_managment_thresholds(uint16_t power_cW, uint16_t voltage_mV) {
    power_cW_threshhold = power_cW;
    voltage_mV_threshhold = voltage_mV;
} 

/**
 * @brief Set the current threshold for a specific channel.
 *
 * @param[in] current_mA New current threshold in milliamps.
 * @param[in] channel The channel for which the current threshold is being set.
 *
 * @details This function updates the current threshold for the specified channel.
 *          The threshold is used to monitor and manage the power consumption of the channel.
 */

void EPS_power_managment_thresholds_current(uint16_t current_mA, EPS_CHANNEL_enum_t channel) {
    current_mA_threshhold[channel] = current_mA;
}

/**
 * @brief Change the disableable state of a channel.
 *
 * @param[in] channel The channel to change the disableable state of.
 * @param[in] current_mA New threshold for current in milliamps.
 * @param[in] channel The channel for which the current threshold is to be set.
 *
 * @details This function assigns a new current threshold to a specific channel,
 *          which will be used to monitor and manage the power consumption for that channel.
 */
void EPS_change_disableable_channels(EPS_CHANNEL_enum_t channel) {
    if (disableable_channels & (1 << channel)) {
        disableable_channels &= ~(1 << channel);
    }
    else {
        disableable_channels |= (1 << channel);
    }
}

/**
 * @brief Sets the timer for power monitoring
 *
 * @param[in] timer New timer value in milliseconds
 *
 * @details This function will set the timer for power monitoring.

 *          The power monitoring is done by the function EPS_power_monitoring.
 *          The timer can be set by the user to a specific value.
 */
void set_eps_monitoring_timer(uint64_t timer) {
    EPS_monitor_timer = timer;
}


/**
 * @brief Set the timer for the watchdog
 *
 * @param[in] timer New timer value in milliseconds
 *
 * @details This function will set the timer for the watchdog.
 *          The watchdog is serviced by the function TASK_service_eps_watchdog.
 *          The timer can be set by the user to a specific value.
 */
uint8_t set_watchdog_timer(uint64_t timer) {
    if (timer >= 75000) {
        return 1;
    }
    watchdog_timer = timer;
    return 0;
}