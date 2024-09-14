#include "adcs_drivers/adcs_types.h"
#include "adcs_drivers/adcs_command_ids.h"
#include "adcs_drivers/adcs_struct_packers.h"
#include "adcs_drivers/adcs_types_to_json.h"
#include "adcs_drivers/adcs_commands.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/// @brief Converts ADCS_CMD_Ack_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_CMD_Ack_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_cmd_ack_struct_TO_json(const ADCS_cmd_ack_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 63) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"last_id\":%u,\"processed\":%u,\"error_flag\":%u,\"error_index\":%u}", 
            data->last_id, data->processed, data->error_flag, data->error_index);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_ID_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_ID_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_id_struct_TO_json(const ADCS_id_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 153) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"node_type\":%u,\"interface_version\":%u,\"major_firmware_version\":%u,\"minor_firmware_version\":%u,\"seconds_since_startup\":%u,\"ms_past_second\":%u}",
            data->node_type, data->interface_version, data->major_firmware_version, data->minor_firmware_version, data->seconds_since_startup, data->ms_past_second);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Boot_Running_Status_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Boot_Running_Status_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_boot_running_status_struct_TO_json(const ADCS_boot_running_status_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 136) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"reset_cause\":%u,\"boot_cause\":%u,\"boot_counter\":%u,\"boot_program_index\":%u,\"major_firmware_version\":%u,\"minor_firmware_version\":%u}",
            data->reset_cause, data->boot_cause, data->boot_counter, data->boot_program_index, data->major_firmware_version, data->minor_firmware_version);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Comms_Status_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Comms_Status_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_comms_status_struct_TO_json(const ADCS_comms_status_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 101) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"cmd_counter\":%u,\"tlm_counter\":%u,\"cmd_buffer_overrun\":%u,\"i2c_tlm_error\":%u,\"i2c_cmd_error\":%u}",
            data->cmd_counter, data->tlm_counter, data->cmd_buffer_overrun, data->i2c_tlm_error, data->i2c_cmd_error);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Angular_Rates_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Angular_Rates_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_angular_rates_struct_TO_json(const ADCS_angular_rates_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 119) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"x_rate_mdeg_per_sec\":%ld,\"y_rate_mdeg_per_sec\":%ld,\"z_rate_mdeg_per_sec\":%ld}", 
            data->x_rate_mdeg_per_sec, data->y_rate_mdeg_per_sec, data->z_rate_mdeg_per_sec);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_LLH_Position_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_LLH_Position_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_llh_position_struct_TO_json(const ADCS_llh_position_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 98) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"latitude_mdeg\":%ld,\"longitude_mdeg\":%ld,\"altitude_meters\":%ld}", 
            data->latitude_mdeg, data->longitude_mdeg, data->altitude_meters);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

// below this line, ballpark estimates are used for the minimum json_output_str_len which should be overestimates by up to about 10 percent

/// @brief Converts ADCS_Power_Control_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Power_Control_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_power_control_struct_TO_json(const ADCS_power_control_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 246) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"cube_control_signal\":%u,\"cube_control_motor\":%u,\"cube_sense1\":%u,\"cube_sense2\":%u,\"cube_star_power\":%u,\"cube_wheel1_power\":%u,\"cube_wheel2_power\":%u,\"cube_wheel3_power\":%u,\"motor_power\":%u,\"gps_power\":%u}",
            data->cube_control_signal, data->cube_control_motor, data->cube_sense1, data->cube_sense2, data->cube_star_power, data->cube_wheel1_power, data->cube_wheel2_power, data->cube_wheel3_power, data->motor_power, data->gps_power);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Set_Unix_Time_Save_Mode_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Set_Unix_Time_Save_Mode_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_set_unix_time_save_mode_struct_TO_json(const ADCS_set_unix_time_save_mode_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 82) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"save_now\":%u,\"save_on_update\":%u,\"save_periodic\":%u,\"period\":%u}",
            data->save_now, data->save_on_update, data->save_periodic, data->period);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Orbit_Params_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Orbit_Params_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_orbit_params_struct_TO_json(const ADCS_orbit_params_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 256) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"inclination_deg\":%.6f,\"eccentricity\":%.6f,\"ascending_node_right_ascension_deg\":%.6f,\"perigee_argument_deg\":%.6f,\"b_star_drag_term\":%.6f,\"mean_motion_orbits_per_day\":%.6f,\"mean_anomaly_deg\":%.6f,\"epoch_year_point_day\":%.6f}",
            data->inclination_deg, data->eccentricity, data->ascending_node_right_ascension_deg, data->perigee_argument_deg, data->b_star_drag_term, data->mean_motion_orbits_per_day, data->mean_anomaly_deg, data->epoch_year_point_day); 
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0; 
}

/// @brief Converts ADCS_Rated_Sensor_Rates_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Rated_Sensor_Rates_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_rated_sensor_rates_struct_TO_json(const ADCS_rated_sensor_rates_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 136) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"x_mdeg_per_sec\":%ld,\"y_mdeg_per_sec\":%ld,\"z_mdeg_per_sec\":%ld}", 
            data->x_mdeg_per_sec, data->y_mdeg_per_sec, data->z_mdeg_per_sec);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Wheel_Speed_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Wheel_Speed_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_wheel_speed_struct_TO_json(const ADCS_wheel_speed_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 37) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"actual(1)_or_commanded(0)\":%d,\"x_rpm\":%d,\"y_rpm\":%d,\"z_rpm\":%d}", 
            data->actual_wheel_speed, data->x_rpm, data->y_rpm, data->z_rpm);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Magnetorquer_Command_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Magnetorquer_Command_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_magnetorquer_command_struct_TO_json(const ADCS_magnetorquer_command_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 91) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"x_ms\":%ld,\"y_ms\":%ld,\"z_ms\":%ld}", 
            data->x_ms, data->y_ms, data->z_ms);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Raw_Magnetometer_Values_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Raw_Magnetometer_Values_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_raw_magnetometer_values_struct_TO_json(const ADCS_raw_magnetometer_values_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 37) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"x\":%d,\"y\":%d,\"z\":%d}", 
            data->x_raw, data->y_raw, data->z_raw);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Fine_Angular_Rates_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Fine_Angular_Rates_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_fine_angular_rates_struct_TO_json(const ADCS_fine_angular_rates_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 88) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"x_mdeg_per_sec\":%d,\"y_mdeg_per_sec\":%d,\"z_mdeg_per_sec\":%d}", 
            data->x_mdeg_per_sec, data->y_mdeg_per_sec, data->z_mdeg_per_sec);

    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Magnetometer_Config_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Magnetometer_Config_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_magnetometer_config_struct_TO_json(const ADCS_magnetometer_config_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 806) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"mounting_transform_alpha_angle_mdeg_per_sec\":%ld,\"mounting_transform_beta_angle_mdeg_per_sec\":%ld,\"mounting_transform_gamma_angle_mdeg_per_sec\":%ld,"
            "\"channel_1_offset_mdeg_per_sec\":%d,\"channel_2_offset_mdeg_per_sec\":%d,\"channel_3_offset_mdeg_per_sec\":%d,"
            "\"sensitivity_matrix_s11_mdeg_per_sec\":%d,\"sensitivity_matrix_s22_mdeg_per_sec\":%d,\"sensitivity_matrix_s33_mdeg_per_sec\":%d,"
            "\"sensitivity_matrix_s12_mdeg_per_sec\":%d,\"sensitivity_matrix_s13_mdeg_per_sec\":%d,\"sensitivity_matrix_s21_mdeg_per_sec\":%d,"
            "\"sensitivity_matrix_s23_mdeg_per_sec\":%d,\"sensitivity_matrix_s31_mdeg_per_sec\":%d,\"sensitivity_matrix_s32_mdeg_per_sec\":%d}",
            data->mounting_transform_alpha_angle_mdeg_per_sec, data->mounting_transform_beta_angle_mdeg_per_sec, data->mounting_transform_gamma_angle_mdeg_per_sec,
            data->channel_1_offset_mdeg_per_sec, data->channel_2_offset_mdeg_per_sec, data->channel_3_offset_mdeg_per_sec,
            data->sensitivity_matrix_s11_mdeg_per_sec, data->sensitivity_matrix_s22_mdeg_per_sec, data->sensitivity_matrix_s33_mdeg_per_sec,
            data->sensitivity_matrix_s12_mdeg_per_sec, data->sensitivity_matrix_s13_mdeg_per_sec, data->sensitivity_matrix_s21_mdeg_per_sec,
            data->sensitivity_matrix_s23_mdeg_per_sec, data->sensitivity_matrix_s31_mdeg_per_sec, data->sensitivity_matrix_s32_mdeg_per_sec);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Commanded_Angles_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Commanded_Angles_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_commanded_angles_struct_TO_json(const ADCS_commanded_angles_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 106) {
        return 1; // Error: invalid input
    }
    
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, "{\"x_mdeg\":%ld,\"y_mdeg\":%ld,\"z_mdeg\":%ld}", 
            data->x_mdeg, data->y_mdeg, data->z_mdeg);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }

    return 0;
}



/// @brief Converts ADCS_Estimation_Params_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Estimation_Params_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_estimation_params_struct_TO_json(const ADCS_estimation_params_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 774) {
        return 1; // Error: invalid input
    }
   // using 7 decimal places for noise covariances, which is the same as on CubeSupport
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
        "{\"magnetometer_rate_filter_system_noise\":%.7f,"
        "\"extended_kalman_filter_system_noise\":%.7f,"
        "\"coarse_sun_sensor_measurement_noise\":%.7f,"
        "\"sun_sensor_measurement_noise\":%.7f,"
        "\"nadir_sensor_measurement_noise\":%.7f,"
        "\"magnetometer_measurement_noise\":%.7f,"
        "\"star_tracker_measurement_noise\":%.7f,"
        "\"use_sun_sensor\":%d,"
        "\"use_nadir_sensor\":%d,"
        "\"use_css\":%d,"
        "\"use_star_tracker\":%d,"
        "\"nadir_sensor_terminator_test\":%d,"
        "\"automatic_magnetometer_recovery\":%d,"
        "\"magnetometer_mode\":%d,"
        "\"magnetometer_selection_for_raw_magnetometer_telemetry\":%d,"
        "\"automatic_estimation_transition_due_to_rate_sensor_errors\":%d,"
        "\"wheel_30s_power_up_delay\":%d,"
        "\"cam1_and_cam2_sampling_period\":%d}", 
        data->magnetometer_rate_filter_system_noise,
        data->extended_kalman_filter_system_noise,
        data->coarse_sun_sensor_measurement_noise,
        data->sun_sensor_measurement_noise,
        data->nadir_sensor_measurement_noise,
        data->magnetometer_measurement_noise,
        data->star_tracker_measurement_noise,
        data->use_sun_sensor,
        data->use_nadir_sensor,
        data->use_css,
        data->use_star_tracker,
        data->nadir_sensor_terminator_test,
        data->automatic_magnetometer_recovery,
        data->magnetometer_mode,
        data->magnetometer_selection_for_raw_magnetometer_telemetry,
        data->automatic_estimation_transition_due_to_rate_sensor_errors,
        data->wheel_30s_power_up_delay,
        data->cam1_and_cam2_sampling_period);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }

    return 0;
}


/// @brief Converts ADCS_Augmented_SGP4_Params_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Augmented_SGP4_Params_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_augmented_sgp4_params_struct_TO_json(const ADCS_augmented_sgp4_params_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 537) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"incl_coefficient_milli\":%d,\"raan_coefficient_milli\":%d,"
                                "\"ecc_coefficient_milli\":%d,\"aop_coefficient_milli\":%d,"
                                "\"time_coefficient_milli\":%d,\"pos_coefficient_milli\":%d,"
                                "\"maximum_position_error_milli\":%ld,\"augmented_sgp4_filter\":%d,"
                                "\"xp_coefficient_nano\":%lld,\"yp_coefficient_nano\":%lld,"
                                "\"gps_roll_over\":%d,\"position_sd_milli\":%ld,"
                                "\"velocity_sd_milli\":%d,\"min_satellites\":%d,"
                                "\"time_gain_milli\":%d,\"max_lag_milli\":%d,\"min_samples\":%d}", 
                                data->incl_coefficient_milli, data->raan_coefficient_milli, 
                                data->ecc_coefficient_milli, data->aop_coefficient_milli, 
                                data->time_coefficient_milli, data->pos_coefficient_milli, 
                                data->maximum_position_error_milli, data->augmented_sgp4_filter, 
                                data->xp_coefficient_nano, data->yp_coefficient_nano, 
                                data->gps_roll_over, data->position_sd_milli, 
                                data->velocity_sd_milli, data->min_satellites, 
                                data->time_gain_milli, data->max_lag_milli, data->min_samples);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Tracking_Controller_Target_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Tracking_Controller_Target_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_tracking_controller_target_struct_TO_json(const ADCS_tracking_controller_target_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 40) {
        return 1; // Error: invalid input
    }
            // 4 decimals in %.4f longitude and latitude gives within roughly 11 m 
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"lon\":%.4f,\"lat\":%.4f,\"alt\":%.4f}", 
                                data->longitude_degrees, data->latitude_degrees, data->altitude_meters);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Rate_Gyro_Config_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Rate_Gyro_Config_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_rate_gyro_config_struct_TO_json(const ADCS_rate_gyro_config_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 201) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"gyro1\":%d,\"gyro2\":%d,\"gyro3\":%d,"
                                "\"x_rate_offset_mdeg_per_sec\":%d,\"y_rate_offset_mdeg_per_sec\":%d,"
                                "\"z_rate_offset_mdeg_per_sec\":%d,\"rate_sensor_mult\":%d}", 
                                data->gyro1, data->gyro2, data->gyro3, 
                                data->x_rate_offset_mdeg_per_sec, data->y_rate_offset_mdeg_per_sec, 
                                data->z_rate_offset_mdeg_per_sec, data->rate_sensor_mult);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Estimated_Attitude_Angles_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Estimated_Attitude_Angles_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_estimated_attitude_angles_struct_TO_json(const ADCS_estimated_attitude_angles_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 165) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"estimated_roll_angle_mdeg\":%ld,\"estimated_pitch_angle_mdeg\":%ld,"
                                "\"estimated_yaw_angle_mdeg\":%ld}", 
                                data->estimated_roll_angle_mdeg, data->estimated_pitch_angle_mdeg, 
                                data->estimated_yaw_angle_mdeg);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Magnetic_Field_Vector_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Magnetic_Field_Vector_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_magnetic_field_vector_struct_TO_json(const ADCS_magnetic_field_vector_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 85) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"x_nT\":%ld,\"y_nT\":%ld,\"z_nT\":%ld}", 
                                data->x_nT, data->y_nT, data->z_nT);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Fine_Sun_Vector_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Fine_Sun_Vector_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_fine_sun_vector_struct_TO_json(const ADCS_fine_sun_vector_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 94) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"x_micro\":%ld,\"y_micro\":%ld,\"z_micro\":%ld}", 
                                data->x_micro, data->y_micro, data->z_micro);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Nadir_Vector_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Nadir_Vector_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_nadir_vector_struct_TO_json(const ADCS_nadir_vector_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 94) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"x_micro\":%ld,\"y_micro\":%ld,\"z_micro\":%ld}", 
                                data->x_micro, data->y_micro, data->z_micro);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Quaternion_Error_Vector_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Quaternion_Error_Vector_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_quaternion_error_vector_struct_TO_json(const ADCS_quaternion_error_vector_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 150) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"quaternion_error_q1_micro\":%ld,\"quaternion_error_q2_micro\":%ld,"
                                "\"quaternion_error_q3_micro\":%ld}", 
                                data->quaternion_error_q1_micro, data->quaternion_error_q2_micro, 
                                data->quaternion_error_q3_micro);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Estimated_Gyro_Bias_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Estimated_Gyro_Bias_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_estimated_gyro_bias_struct_TO_json(const ADCS_estimated_gyro_bias_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 194) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"estimated_x_gyro_bias_mdeg_per_sec\":%ld,"
                                "\"estimated_y_gyro_bias_mdeg_per_sec\":%ld,"
                                "\"estimated_z_gyro_bias_mdeg_per_sec\":%ld}", 
                                data->estimated_x_gyro_bias_mdeg_per_sec, 
                                data->estimated_y_gyro_bias_mdeg_per_sec, 
                                data->estimated_z_gyro_bias_mdeg_per_sec);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Estimation_Innovation_Vector_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Estimation_Innovation_Vector_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_estimation_innovation_vector_struct_TO_json(const ADCS_estimation_innovation_vector_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 150) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"innovation_vector_x_micro\":%ld,\"innovation_vector_y_micro\":%ld,"
                                "\"innovation_vector_z_micro\":%ld}", 
                                data->innovation_vector_x_micro, data->innovation_vector_y_micro, 
                                data->innovation_vector_z_micro);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Raw_Cam_Sensor_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Raw_Cam_Sensor_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_raw_cam_sensor_struct_TO_json(const ADCS_raw_cam_sensor_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 131) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"which_sensor\":%d,\"cam_centroid_x\":%d,\"cam_centroid_y\":%d,"
                                "\"cam_capture_status\":%d,\"cam_detection_result\":%d}", 
                                data->which_sensor, data->cam_centroid_x, data->cam_centroid_y, 
                                data->cam_capture_status, data->cam_detection_result);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Raw_Coarse_Sun_Sensor_1_to_6_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Raw_Coarse_Sun_Sensor_1_to_6_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_raw_coarse_sun_sensor_1_to_6_struct_TO_json(const ADCS_raw_coarse_sun_sensor_1_to_6_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 454) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"coarse_sun_sensor_1\":%d,\"coarse_sun_sensor_2\":%d,\"coarse_sun_sensor_3\":%d,\"coarse_sun_sensor_4\":%d,"
                                "\"coarse_sun_sensor_5\":%d,\"coarse_sun_sensor_6\":%d}", 
                                data->coarse_sun_sensor_1, data->coarse_sun_sensor_2, data->coarse_sun_sensor_3, data->coarse_sun_sensor_4, 
                                data->coarse_sun_sensor_5, data->coarse_sun_sensor_6);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Raw_Coarse_Sun_Sensor_7_to_10_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Raw_Coarse_Sun_Sensor_7_to_10_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_raw_coarse_sun_sensor_7_to_10_struct_TO_json(const ADCS_raw_coarse_sun_sensor_7_to_10_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 306) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"coarse_sun_sensor_7\":%d,\"coarse_sun_sensor_8\":%d,\"coarse_sun_sensor_9\":%d,\"coarse_sun_sensor_10\":%d}", 
                                data->coarse_sun_sensor_7, data->coarse_sun_sensor_8, data->coarse_sun_sensor_9, data->coarse_sun_sensor_10);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_CubeControl_Current_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_CubeControl_Current_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_cubecontrol_current_struct_TO_json(const ADCS_cubecontrol_current_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 117) {
        return 1; // Error: invalid input
    }
                // measured up to 0.1 mA (100 uA); current measured in 0.48828125 mA steps
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"cubecontrol_3v3_current_mA\":%.1f,\"cubecontrol_5v_current_mA\":%.1f,"
                                "\"cubecontrol_vbat_current_mA\":%.1f}", 
                                data->cubecontrol_3v3_current_mA, data->cubecontrol_5v_current_mA, 
                                data->cubecontrol_vbat_current_mA);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Raw_GPS_Status_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Raw_GPS_Status_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_raw_gps_status_struct_TO_json(const ADCS_raw_gps_status_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 181) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"gps_solution_status\":%d,\"num_tracked_satellites\":%d,"
                                "\"num_used_satellites\":%d,\"counter_xyz_log\":%d,"
                                "\"counter_range_log\":%d,\"response_message_gps_log\":%d}", 
                                data->gps_solution_status, data->num_tracked_satellites, 
                                data->num_used_satellites, data->counter_xyz_log, 
                                data->counter_range_log, data->response_message_gps_log);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Raw_GPS_Time_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Raw_GPS_Time_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_raw_gps_time_struct_TO_json(const ADCS_raw_gps_time_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 60) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"gps_reference_week\":%d,\"gps_time\":%ld}", 
                                data->gps_reference_week, data->gps_time_ms);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Raw_GPS_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Raw_GPS_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_raw_gps_struct_TO_json(const ADCS_raw_gps_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 92) {
        return 1; // Error: invalid input
    }
    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"axis\":%d,\"ecef_position_meters\":%ld,\"ecef_velocity_meters_per_sec\":%d}", 
                                data->axis, data->ecef_position_meters, data->ecef_velocity_meters_per_sec);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Measurements_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Measurements_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_measurements_struct_TO_json(const ADCS_measurements_struct_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 1557) {
        return 1; // Error: invalid input
    }

    int16_t snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"magnetic_field_x_nT\":%ld,\"magnetic_field_y_nT\":%ld,"
                                "\"magnetic_field_z_nT\":%ld,\"coarse_sun_x_micro\":%ld,"
                                "\"coarse_sun_y_micro\":%ld,\"coarse_sun_z_micro\":%ld,"
                                "\"sun_x_micro\":%ld,\"sun_y_micro\":%ld,\"sun_z_micro\":%ld,"
                                "\"nadir_x_micro\":%ld,\"nadir_y_micro\":%ld,\"nadir_z_micro\":%ld,"
                                "\"x_angular_rate_mdeg_per_sec\":%ld,\"y_angular_rate_mdeg_per_sec\":%ld,"
                                "\"z_angular_rate_mdeg_per_sec\":%ld,\"x_wheel_speed_rpm\":%d,"
                                "\"y_wheel_speed_rpm\":%d,\"z_wheel_speed_rpm\":%d,"
                                "\"star1_body_x_micro\":%ld,\"star1_body_y_micro\":%ld,"
                                "\"star1_body_z_micro\":%ld,\"star1_orbit_x_micro\":%ld,"
                                "\"star1_orbit_y_micro\":%ld,\"star1_orbit_z_micro\":%ld,"
                                "\"star2_body_x_micro\":%ld,\"star2_body_y_micro\":%ld,"
                                "\"star2_body_z_micro\":%ld,\"star2_orbit_x_micro\":%ld,"
                                "\"star2_orbit_y_micro\":%ld,\"star2_orbit_z_micro\":%ld,"
                                "\"star3_body_x_micro\":%ld,\"star3_body_y_micro\":%ld,"
                                "\"star3_body_z_micro\":%ld,\"star3_orbit_x_micro\":%ld,"
                                "\"star3_orbit_y_micro\":%ld,\"star3_orbit_z_micro\":%ld}", 
                                data->magnetic_field_x_nT, data->magnetic_field_y_nT, data->magnetic_field_z_nT,
                                data->coarse_sun_x_micro, data->coarse_sun_y_micro, data->coarse_sun_z_micro,
                                data->sun_x_micro, data->sun_y_micro, data->sun_z_micro,
                                data->nadir_x_micro, data->nadir_y_micro, data->nadir_z_micro,
                                data->x_angular_rate_mdeg_per_sec, data->y_angular_rate_mdeg_per_sec, 
                                data->z_angular_rate_mdeg_per_sec, data->x_wheel_speed_rpm, 
                                data->y_wheel_speed_rpm, data->z_wheel_speed_rpm,
                                data->star1_body_x_micro, data->star1_body_y_micro, data->star1_body_z_micro,
                                data->star1_orbit_x_micro, data->star1_orbit_y_micro, data->star1_orbit_z_micro,
                                data->star2_body_x_micro, data->star2_body_y_micro, data->star2_body_z_micro,
                                data->star2_orbit_x_micro, data->star2_orbit_y_micro, data->star2_orbit_z_micro,
                                data->star3_body_x_micro, data->star3_body_y_micro, data->star3_body_z_micro,
                                data->star3_orbit_x_micro, data->star3_orbit_y_micro, data->star3_orbit_z_micro);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts a generic array of ADCS telemetry to a JSON string.
/// @param[in] data Pointer to the array.
/// @param[in] data_length Length of the array.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_generic_telemetry_uint8_array_TO_json(const uint8_t *data, const uint16_t data_length, char json_output_str[], uint16_t json_output_str_len) {
    bool remove_spaces = false;

    if (data == NULL || json_output_str == NULL) { 
        return 1; // Error: invalid input
    } else if (json_output_str_len < (data_length * 3)) { // 3 characters per hex byte
        // longest data length is 504, but it looks like json_output_str_len is 1024, so the 504-byte command won't work here.
        remove_spaces = true;
        if (json_output_str_len < (data_length * 2)) {
            return 1; // invalid input again
        }
    }

    int16_t snprintf_ret; 
    int16_t total_written = 0;
    for (uint16_t i = 0; i < data_length; i++) {
        if (!remove_spaces) { // write each data byte in hex, separated by spaces (for easy translation into CubeSupport)
            snprintf_ret = snprintf(&json_output_str[total_written], json_output_str_len, "%x ", data[i]);
        } else { // if the command size is too long, remove the spaces and try again
            snprintf_ret = snprintf(&json_output_str[total_written], json_output_str_len, "%x", data[i]);
        }

        if (snprintf_ret < 0) {
            return 2; // Error: snprintf encoding error
        } else {
            total_written += snprintf_ret;
        }

        if (total_written >= json_output_str_len) {
            return 3; // Error: string buffer too short
        }
    }
    return 0;
}