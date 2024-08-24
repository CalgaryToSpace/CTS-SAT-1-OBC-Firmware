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

// TODO: determine how long each of these are and fix the json_output_str_len < ##NUMBER##

/// @brief Converts ADCS_CMD_Ack_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_CMD_Ack_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_CMD_Ack_Struct_TO_json(const ADCS_CMD_Ack_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 100) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"last_id\":%u,\"processed\":%u,\"error_flag\":%u,\"error_index\":%u}", 
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
uint8_t ADCS_ID_Struct_TO_json(const ADCS_ID_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 200) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"node_type\":%u,\"interface_version\":%u,\"major_firmware_version\":%u,\"minor_firmware_version\":%u,\"seconds_since_startup\":%u,\"ms_past_second\":%u}",
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
uint8_t ADCS_Boot_Running_Status_Struct_TO_json(const ADCS_Boot_Running_Status_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 250) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"reset_cause\":%u,\"boot_cause\":%u,\"boot_counter\":%u,\"boot_program_index\":%u,\"major_firmware_version\":%u,\"minor_firmware_version\":%u}",
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
uint8_t ADCS_Comms_Status_Struct_TO_json(const ADCS_Comms_Status_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 150) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"cmd_counter\":%u,\"tlm_counter\":%u,\"cmd_buffer_overrun\":%u,\"i2c_tlm_error\":%u,\"i2c_cmd_error\":%u}",
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
uint8_t ADCS_Angular_Rates_Struct_TO_json(const ADCS_Angular_Rates_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 200) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"x_rate_milli_deg_per_sec\":%ld,\"y_rate_milli_deg_per_sec\":%ld,\"z_rate_milli_deg_per_sec\":%ld}", 
            data->x_rate_milli_deg_per_sec, data->y_rate_milli_deg_per_sec, data->z_rate_milli_deg_per_sec);
    
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
uint8_t ADCS_LLH_Position_Struct_TO_json(const ADCS_LLH_Position_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 200) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"latitude_milli_deg\":%ld,\"longitude_milli_deg\":%ld,\"altitude_meters\":%ld}", 
            data->latitude_milli_deg, data->longitude_milli_deg, data->altitude_meters);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Power_Control_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Power_Control_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_Power_Control_Struct_TO_json(const ADCS_Power_Control_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 300) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"cube_control_signal\":%u,\"cube_control_motor\":%u,\"cube_sense1\":%u,\"cube_sense2\":%u,\"cube_star_power\":%u,\"cube_wheel1_power\":%u,\"cube_wheel2_power\":%u,\"cube_wheel3_power\":%u,\"motor_power\":%u,\"gps_power\":%u}",
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
uint8_t ADCS_Set_Unix_Time_Save_Mode_Struct_TO_json(const ADCS_Set_Unix_Time_Save_Mode_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 150) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"save_now\":%u,\"save_on_update\":%u,\"save_periodic\":%u,\"period\":%u}",
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
uint8_t ADCS_Orbit_Params_Struct_TO_json(const ADCS_Orbit_Params_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 500) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"inclination\":%.6f,\"eccentricity\":%.6f,\"ascending_node_right_ascension\":%.6f,\"perigee_argument\":%.6f,\"b_star_drag_term\":%.6f,\"mean_motion\":%.6f,\"mean_anomaly\":%.6f,\"epoch\":%.6f}",
            data->inclination, data->eccentricity, data->ascending_node_right_ascension, data->perigee_argument, data->b_star_drag_term, data->mean_motion, data->mean_anomaly, data->epoch); 
    
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
uint8_t ADCS_Rated_Sensor_Rates_Struct_TO_json(const ADCS_Rated_Sensor_Rates_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 100) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"x_milli_deg_per_sec\":%ld,\"y_milli_deg_per_sec\":%ld,\"z_milli_deg_per_sec\":%ld}", 
            data->x_milli_deg_per_sec, data->y_milli_deg_per_sec, data->z_milli_deg_per_sec);
    
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
uint8_t ADCS_Wheel_Speed_Struct_TO_json(const ADCS_Wheel_Speed_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 100) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"x\":%d,\"y\":%d,\"z\":%d}", 
            data->x, data->y, data->z);
    
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
uint8_t ADCS_Magnetorquer_Command_Struct_TO_json(const ADCS_Magnetorquer_Command_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 100) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"x_ms\":%ld,\"y_ms\":%ld,\"z_ms\":%ld}", 
            data->x_ms, data->y_ms, data->z_ms);
    
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Raw_Mag_TLM_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Raw_Mag_TLM_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_Raw_Mag_TLM_Struct_TO_json(const ADCS_Raw_Mag_TLM_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 100) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"x\":%d,\"y\":%d,\"z\":%d}", 
            data->x, data->y, data->z);
    
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
uint8_t ADCS_Fine_Angular_Rates_Struct_TO_json(const ADCS_Fine_Angular_Rates_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 100) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"x_milli_deg_per_sec\":%d,\"y_milli_deg_per_sec\":%d,\"z_milli_deg_per_sec\":%d}", 
            data->x_milli_deg_per_sec, data->y_milli_deg_per_sec, data->z_milli_deg_per_sec);

    
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
uint8_t ADCS_Magnetometer_Config_Struct_TO_json(const ADCS_Magnetometer_Config_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 500) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = sprintf(json_output_str, "{\"mounting_transform_alpha_angle_milli_deg_per_sec\":%ld,\"mounting_transform_beta_angle_milli_deg_per_sec\":%ld,\"mounting_transform_gamma_angle_milli_deg_per_sec\":%ld,"
            "\"channel_1_offset_milli_deg_per_sec\":%d,\"channel_2_offset_milli_deg_per_sec\":%d,\"channel_3_offset_milli_deg_per_sec\":%d,"
            "\"sensitivity_matrix_s11_milli_deg_per_sec\":%d,\"sensitivity_matrix_s22_milli_deg_per_sec\":%d,\"sensitivity_matrix_s33_milli_deg_per_sec\":%d,"
            "\"sensitivity_matrix_s12_milli_deg_per_sec\":%d,\"sensitivity_matrix_s13_milli_deg_per_sec\":%d,\"sensitivity_matrix_s21_milli_deg_per_sec\":%d,"
            "\"sensitivity_matrix_s23_milli_deg_per_sec\":%d,\"sensitivity_matrix_s31_milli_deg_per_sec\":%d,\"sensitivity_matrix_s32_milli_deg_per_sec\":%d}",
            data->mounting_transform_alpha_angle_milli_deg_per_sec, data->mounting_transform_beta_angle_milli_deg_per_sec, data->mounting_transform_gamma_angle_milli_deg_per_sec,
            data->channel_1_offset_milli_deg_per_sec, data->channel_2_offset_milli_deg_per_sec, data->channel_3_offset_milli_deg_per_sec,
            data->sensitivity_matrix_s11_milli_deg_per_sec, data->sensitivity_matrix_s22_milli_deg_per_sec, data->sensitivity_matrix_s33_milli_deg_per_sec,
            data->sensitivity_matrix_s12_milli_deg_per_sec, data->sensitivity_matrix_s13_milli_deg_per_sec, data->sensitivity_matrix_s21_milli_deg_per_sec,
            data->sensitivity_matrix_s23_milli_deg_per_sec, data->sensitivity_matrix_s31_milli_deg_per_sec, data->sensitivity_matrix_s32_milli_deg_per_sec);
    
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
uint8_t ADCS_Commanded_Angles_Struct_TO_json(const ADCS_Commanded_Angles_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 100) {
        return 1; // Error: invalid input
    }
    
    int snprintf_ret = sprintf(json_output_str, "{\"x_milli_deg\":%ld,\"y_milli_deg\":%ld,\"z_milli_deg\":%ld}", 
            data->x_milli_deg, data->y_milli_deg, data->z_milli_deg);

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
uint8_t ADCS_Estimation_Params_Struct_TO_json(const ADCS_Estimation_Params_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 600) {
        return 1; // Error: invalid input
    }
    // TODO: %f needs to have significant digits determined
    int snprintf_ret = sprintf(json_output_str, 
        "{\"magnetometer_rate_filter_system_noise\":%f,"
        "\"ekf_system_noise\":%f,"
        "\"css_measurement_noise\":%f,"
        "\"sun_sensor_measurement_noise\":%f,"
        "\"nadir_sensor_measurement_noise\":%f,"
        "\"magnetometer_measurement_noise\":%f,"
        "\"star_tracker_measurement_noise\":%f,"
        "\"use_sun_sensor\":%d,"
        "\"use_nadir_sensor\":%d,"
        "\"use_css\":%d,"
        "\"use_star_tracker\":%d,"
        "\"nadir_sensor_terminator_test\":%d,"
        "\"automatic_magnetometer_recovery\":%d,"
        "\"magnetometer_mode\":%d,"
        "\"magnetometer_selection_for_raw_mtm_tlm\":%d,"
        "\"automatic_estimation_transition_due_to_rate_sensor_errors\":%d,"
        "\"wheel_30s_power_up_delay\":%d,"
        "\"cam1_and_cam2_sampling_period\":%d}", 
        data->magnetometer_rate_filter_system_noise,
        data->ekf_system_noise,
        data->css_measurement_noise,
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
        data->magnetometer_selection_for_raw_mtm_tlm,
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

/// @brief Converts ADCS_ASGP4_Params_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_ASGP4_Params_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_ASGP4_Params_Struct_TO_json(const ADCS_ASGP4_Params_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 300) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"incl_coefficient_milli\":%d,\"raan_coefficient_milli\":%d,"
                                "\"ecc_coefficient_milli\":%d,\"aop_coefficient_milli\":%d,"
                                "\"time_coefficient_milli\":%d,\"pos_coefficient_milli\":%d,"
                                "\"maximum_position_error_milli\":%ld,\"asgp4_filter\":%d,"
                                "\"xp_coefficient_nano\":%lld,\"yp_coefficient_nano\":%lld,"
                                "\"gps_roll_over\":%d,\"position_sd_milli\":%ld,"
                                "\"velocity_sd_milli\":%d,\"min_satellites\":%d,"
                                "\"time_gain_milli\":%d,\"max_lag_milli\":%d,\"min_samples\":%d}", 
                                data->incl_coefficient_milli, data->raan_coefficient_milli, 
                                data->ecc_coefficient_milli, data->aop_coefficient_milli, 
                                data->time_coefficient_milli, data->pos_coefficient_milli, 
                                data->maximum_position_error_milli, data->asgp4_filter, 
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
uint8_t ADCS_Tracking_Controller_Target_Struct_TO_json(const ADCS_Tracking_Controller_Target_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 100) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"lon\":%f,\"lat\":%f,\"alt\":%f}", 
                                data->lon, data->lat, data->alt);

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
uint8_t ADCS_Rate_Gyro_Config_Struct_TO_json(const ADCS_Rate_Gyro_Config_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 200) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"gyro1\":%d,\"gyro2\":%d,\"gyro3\":%d,"
                                "\"x_rate_offset_milli_deg_per_sec\":%d,\"y_rate_offset_milli_deg_per_sec\":%d,"
                                "\"z_rate_offset_milli_deg_per_sec\":%d,\"rate_sensor_mult\":%d}", 
                                data->gyro1, data->gyro2, data->gyro3, 
                                data->x_rate_offset_milli_deg_per_sec, data->y_rate_offset_milli_deg_per_sec, 
                                data->z_rate_offset_milli_deg_per_sec, data->rate_sensor_mult);

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
uint8_t ADCS_Estimated_Attitude_Angles_Struct_TO_json(const ADCS_Estimated_Attitude_Angles_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 150) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"estimated_roll_angle_milli_deg\":%ld,\"estimated_pitch_angle_milli_deg\":%ld,"
                                "\"estimated_yaw_angle_milli_deg\":%ld}", 
                                data->estimated_roll_angle_milli_deg, data->estimated_pitch_angle_milli_deg, 
                                data->estimated_yaw_angle_milli_deg);

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
uint8_t ADCS_Magnetic_Field_Vector_Struct_TO_json(const ADCS_Magnetic_Field_Vector_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 150) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
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
uint8_t ADCS_Fine_Sun_Vector_Struct_TO_json(const ADCS_Fine_Sun_Vector_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 150) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
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
uint8_t ADCS_Nadir_Vector_Struct_TO_json(const ADCS_Nadir_Vector_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 150) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
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
uint8_t ADCS_Quaternion_Error_Vector_Struct_TO_json(const ADCS_Quaternion_Error_Vector_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 150) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
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
uint8_t ADCS_Estimated_Gyro_Bias_Struct_TO_json(const ADCS_Estimated_Gyro_Bias_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 150) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"estimated_x_gyro_bias_milli_deg_per_sec\":%ld,"
                                "\"estimated_y_gyro_bias_milli_deg_per_sec\":%ld,"
                                "\"estimated_z_gyro_bias_milli_deg_per_sec\":%ld}", 
                                data->estimated_x_gyro_bias_milli_deg_per_sec, 
                                data->estimated_y_gyro_bias_milli_deg_per_sec, 
                                data->estimated_z_gyro_bias_milli_deg_per_sec);

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
uint8_t ADCS_Estimation_Innovation_Vector_Struct_TO_json(const ADCS_Estimation_Innovation_Vector_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 150) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
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
uint8_t ADCS_Raw_Cam_Sensor_Struct_TO_json(const ADCS_Raw_Cam_Sensor_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 200) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
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

/// @brief Converts ADCS_Raw_CSS_1_to_6_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Raw_CSS_1_to_6_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_Raw_CSS_1_to_6_Struct_TO_json(const ADCS_Raw_CSS_1_to_6_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 150) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"css1\":%d,\"css2\":%d,\"css3\":%d,\"css4\":%d,"
                                "\"css5\":%d,\"css6\":%d}", 
                                data->css1, data->css2, data->css3, data->css4, 
                                data->css5, data->css6);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    
    return 0;
}

/// @brief Converts ADCS_Raw_CSS_7_to_10_Struct to a JSON string.
/// @param[in] data Pointer to the ADCS_Raw_CSS_7_to_10_Struct.
/// @param[out] json_output_str Buffer to hold the JSON string.
/// @param[in] json_output_str_len Length of the JSON output buffer.
/// @return 0 if successful, 1 for invalid input, 2 for snprintf encoding error, 3 for too short string buffer
uint8_t ADCS_Raw_CSS_7_to_10_Struct_TO_json(const ADCS_Raw_CSS_7_to_10_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 150) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"css7\":%d,\"css8\":%d,\"css9\":%d,\"css10\":%d}", 
                                data->css7, data->css8, data->css9, data->css10);

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
uint8_t ADCS_CubeControl_Current_Struct_TO_json(const ADCS_CubeControl_Current_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 200) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"cubecontrol_3v3_current_mA\":%f,\"cubecontrol_5v_current_mA\":%f,"
                                "\"cubecontrol_vbat_current_mA\":%f}", 
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
uint8_t ADCS_Raw_GPS_Status_Struct_TO_json(const ADCS_Raw_GPS_Status_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 200) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
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
uint8_t ADCS_Raw_GPS_Time_Struct_TO_json(const ADCS_Raw_GPS_Time_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 150) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"gps_reference_week\":%d,\"gps_time\":%f}", 
                                data->gps_reference_week, data->gps_time);

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
uint8_t ADCS_Raw_GPS_Struct_TO_json(const ADCS_Raw_GPS_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 200) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
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
uint8_t ADCS_Measurements_Struct_TO_json(const ADCS_Measurements_Struct *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 1000) {
        return 1; // Error: invalid input
    }
    int snprintf_ret = snprintf(json_output_str, json_output_str_len, 
                                "{\"magnetic_field_x_nT\":%ld,\"magnetic_field_y_nT\":%ld,"
                                "\"magnetic_field_z_nT\":%ld,\"coarse_sun_x_micro\":%ld,"
                                "\"coarse_sun_y_micro\":%ld,\"coarse_sun_z_micro\":%ld,"
                                "\"sun_x_micro\":%ld,\"sun_y_micro\":%ld,\"sun_z_micro\":%ld,"
                                "\"nadir_x_micro\":%ld,\"nadir_y_micro\":%ld,\"nadir_z_micro\":%ld,"
                                "\"x_angular_rate_milli_deg_per_sec\":%ld,\"y_angular_rate_milli_deg_per_sec\":%ld,"
                                "\"z_angular_rate_milli_deg_per_sec\":%ld,\"x_wheel_speed_rpm\":%d,"
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
                                data->x_angular_rate_milli_deg_per_sec, data->y_angular_rate_milli_deg_per_sec, 
                                data->z_angular_rate_milli_deg_per_sec, data->x_wheel_speed_rpm, 
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
