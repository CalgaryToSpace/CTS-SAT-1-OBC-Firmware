
#include "eps_drivers/eps_commands.h"
#include "eps_drivers/eps_time.h"
#include "rtos_tasks/rtos_eps_tasks.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "log/log.h"
#include "timekeeping/timekeeping.h"
#include "eps_drivers/eps_types_to_json.h"

#include "cmsis_os.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>



/// @brief periodically sends a command to the eps to reset the watchdog timer
/// @note The eps has two watchdog timers: The "watchdog reset" and the "peripheral reset".
/// The "watchdog reset" resets the entire eps system when TTC_WDG_TIMEOUT (default 300s, 
/// see ISIS.EPS2.ICD.SW.IVID.7 pg. 85) seconds pass without a command being received. 
/// The "peripheral reset" resets the communication peripheral on the eps's mcu 
/// after 0.65 * TTC_WDG_TIMEOUT seconds. The "peripheral reset" allows for more graceful 
/// resets if the eps's communication peripheral was responsible for loss of connection.
void TASK_service_eps_watchdog(void *argument) {
    TASK_HELP_start_of_task();
    // This task should sleep for 0.25*TTC_WDG_TIMEOUT = 75s to avoid the peripheral reset
    // going off. Timing also ensures that in the case the peripheral reset restores
    // communication, the watch dog timer will reset before "watchdog reset" causes a full 
    // system reset (See ISIS.EPS2.ICD.SW.IVID.7 pg.9 for further explanation).
    // To avoid resets, we sleep for much shorter than that.
    const uint32_t sleep_duration_ms = 20000;

    // Sleep 10s at the start so that more important tasks work first.
    // Important to service the watchdog near the start, though.
    osDelay(10000);

    while(1) {
        const uint8_t result = EPS_CMD_watchdog();

        if (result != 0) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL,
                "EPS_CMD_watchdog() -> Error: %d", result
            );
        }
        else {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_NORMAL, 
                LOG_SINK_ALL,
                "EPS watchdog serviced successfully." 
            );
        }
        
        osDelay(sleep_duration_ms);
    }
}

/*
* @brief on boot this thread sets the obc time to the eps time. After that it periodically
* checks the obc time against the eps time and logs when they diverge by more than 10 seconds.
*/
void TASK_time_sync(void *argument) {
    TASK_HELP_start_of_task();

    // Run on boot after 5000ms, but only run every 45 seconds in normal operation.
    uint32_t sleep_duration_ms = 5000;
    while(1) {
        // osDelay must be at the top of the while(1) loop so that `continue;` doesn't skip it.
        osDelay(sleep_duration_ms);


        // First, try to sync EPS-to-OBC for the initial boot (or in case stuff gets funky later).
        if (TIM_last_synchronization_source == TIM_SOURCE_NONE) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_NORMAL,
                LOG_SINK_ALL,
                "Setting OBC time based on EPS time because last_source == TIM_SOURCE_NONE"
            );

            if (EPS_set_obc_time_based_on_eps_time() != 0) {
                LOG_message(
                    LOG_SYSTEM_EPS,
                    LOG_SEVERITY_ERROR,
                    LOG_SINK_ALL,
                    "EPS_set_obc_time_based_on_eps_time() -> Error"
                );
            }
            continue;
        }
        // If the OBC's time is ever somehow less than 2010-01-01T00:00:00Z, then sync to EPS time.
        if (TIM_get_current_unix_epoch_time_ms() < 1262329200000) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_NORMAL, 
                LOG_SINK_ALL,
                "Setting OBC time based on EPS time because current time < 2010-01-01"
            );
            if (EPS_set_obc_time_based_on_eps_time() != 0) {
                LOG_message(
                    LOG_SYSTEM_EPS,
                    LOG_SEVERITY_ERROR,
                    LOG_SINK_ALL,
                    "EPS_set_obc_time_based_on_eps_time() -> Error"
                );
            }
            continue;
        }

        // For all subsequent runs, sleep for 10 mins.
        sleep_duration_ms = 600000;
      
        EPS_struct_system_status_t status;
        const uint8_t result_status = EPS_CMD_get_system_status(&status);
        if (result_status != 0) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL,
                "EPS_CMD_get_system_status() -> Error: %d",
                result_status
            );
            continue;
        }

        // Use uint32_t for these values in seconds, as int32_t won't overflow until 2038.
        const uint32_t eps_time_sec = status.unix_time_sec;
        const uint32_t obc_time_sec = TIM_get_current_unix_epoch_time_ms() / 1000;
        const int32_t delta_seconds = ((int32_t)obc_time_sec) - ((int32_t)eps_time_sec);
        if (abs(delta_seconds) > 10) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_WARNING,
                LOG_SINK_ALL,
                "Warning: EPS time and OBC time differ by more than 10 seconds. \n EPS time seconds: %" PRIi32 ",\n OBC time seconds: %" PRIi32,
                eps_time_sec, obc_time_sec
            );
        }
    } // End of task while loop
}

void TASK_EPS_power_monitoring(void *argument) {
    
    //Delay for other tasks to start
    TASK_HELP_start_of_task();
    osDelay(30000);

    uint8_t prev_pdu_status = 1;
    EPS_struct_pdu_housekeeping_data_eng_t prev_EPS_pdu_housekeeping_data_eng;

    //This needs to happen. Will loop until it gets PDU data
    while (prev_pdu_status) {

        osDelay(10000);

        prev_pdu_status = EPS_CMD_get_pdu_housekeeping_data_eng(&prev_EPS_pdu_housekeeping_data_eng);
        if (prev_pdu_status != 0) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL,
                "EPS_CMD_get_pdu_housekeeping_data_eng() -> Error: %d",
                prev_pdu_status
            );
        }
    }

    EPS_vpid_eng_t prev_vpid_eng[32];
    memcpy(prev_vpid_eng, prev_EPS_pdu_housekeeping_data_eng.vip_each_channel, sizeof(EPS_vpid_eng_t) * 32);

    uint16_t sleep_duration_ms = 1000;

    //Superloop
    while (1) {
        osDelay(sleep_duration_ms);
        
        char json_str[1000];
        
        //Get PDU data
        EPS_struct_pdu_housekeeping_data_eng_t EPS_pdu_housekeeping_data_eng;
        uint8_t curr_pdu_status = EPS_CMD_get_pdu_housekeeping_data_eng(&EPS_pdu_housekeeping_data_eng);

        if (curr_pdu_status != 0) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL,
                "EPS_CMD_get_pdu_housekeeping_data_eng() -> Error: %d",
                curr_pdu_status
            );
            continue;
        }

        EPS_vpid_eng_t vpid_eng[32];
        memcpy(vpid_eng, EPS_pdu_housekeeping_data_eng.vip_each_channel, sizeof(EPS_vpid_eng_t) * 32);

        //Power Logging
        uint8_t pdu_TO_JSON_status = EPS_struct_pdu_housekeeping_data_eng_TO_json(&EPS_pdu_housekeeping_data_eng, json_str, 1000);

        if (pdu_TO_JSON_status != 0) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL,
                "EPS_struct_pdu_housekeeping_data_eng_TO_json() -> Error: %d",
                pdu_TO_JSON_status
            );
            continue;
        }

        LOG_message(
            LOG_SYSTEM_EPS,
            LOG_SEVERITY_NORMAL,
            LOG_SINK_ALL,
            "EPS PDU housekeeping data:\n %s",
            json_str
        );

        uint32_t ch_bitfield = (EPS_pdu_housekeeping_data_eng.stat_ch_ext_on_bitfield << 16) & EPS_pdu_housekeeping_data_eng.stat_ch_on_bitfield;

        //Power Monitoring
        for (int channel = 0; channel < 32; channel++) {

            if (ch_bitfield & 1 //Check if channel is enabled
                && (vpid_eng[channel].power_cW - prev_vpid_eng[channel].power_cW > 1000             //TODO: Set PDU thresh hold
                || vpid_eng[channel].voltage_mV - prev_vpid_eng[channel].voltage_mV > 1000
                || vpid_eng[channel].current_mA - prev_vpid_eng[channel].current_mA > 1000
                || vpid_eng[channel].power_cW - prev_vpid_eng[channel].power_cW < -1000
                || vpid_eng[channel].voltage_mV - prev_vpid_eng[channel].voltage_mV < -1000
                || vpid_eng[channel].current_mA - prev_vpid_eng[channel].current_mA < -1000)) {

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
        }

        memcpy(prev_vpid_eng, vpid_eng, sizeof(prev_vpid_eng));
        prev_EPS_pdu_housekeeping_data_eng = EPS_pdu_housekeeping_data_eng;
    }
}
