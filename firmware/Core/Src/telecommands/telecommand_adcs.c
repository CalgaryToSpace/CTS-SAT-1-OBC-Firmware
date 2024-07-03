#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"
#include "transforms/arrays.h"
#include "unit_tests/unit_test_executor.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "adcs_types.h"

uint8_t TCMDEXEC_ADCS_ack(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_TC_Ack_Struct ack;
    ADCS_TC_Ack(&ack);
    return ack.error_flag;
}

uint8_t TCMDEXEC_ADCS_set_wheel_speed(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    // parse arguments: first into uint64_t, then convert to correct form for input
    uint8_t num_args = 3;
    uint64_t arguments[num_args]; 
    uint16_t args_16[num_args];
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_uint64_arg(args_str, strlen((char*)args_str), i, &arguments[i]);
        args_16[i] = (uint16_t) arguments[i];
    }
    
    uint8_t status = ADCS_Set_Wheel_Speed(args_16[0], args_16[1], args_16[2]); 
    return status;
}

uint8_t TCMDEXEC_ADCS_reset(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Reset(); 
    return status;
}                

uint8_t TCMDEXEC_ADCS_identification(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Identification(); 
    return status;
}                            

uint8_t TCMDEXEC_ADCS_program_status(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Program_Status(); 
    return status;
}                            

uint8_t TCMDEXEC_ADCS_communication_status(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                           char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Communication_Status(); 
    return status;
}                                

uint8_t TCMDEXEC_ADCS_deploy_magnetometer(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                          char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t timeout;
    TCMD_extract_uint64_arg(args_str, strlen((char*)args_str), 0, &timeout);
    uint8_t status = ADCS_Deploy_Magnetometer((uint8_t) timeout);
    return status;
}                                

uint8_t TCMDEXEC_ADCS_set_run_mode(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t run_mode;
    TCMD_extract_uint64_arg(args_str, strlen((char*)args_str), 0, &run_mode);
    uint8_t status = ADCS_Set_Run_Mode((ADCS_Run_Mode) run_mode); 
    return status;
}                        

uint8_t TCMDEXEC_ADCS_clear_errors(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Clear_Errors();
    return status;
}                        

uint8_t TCMDEXEC_ADCS_attitude_control_mode(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    // parse arguments into uint64_t
    uint8_t num_args = 2;
    uint64_t arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_uint64_arg(args_str, strlen((char*)args_str), i, &arguments[i]);
    }
    // then convert to correct form for input
    uint8_t status = ADCS_Attitude_Control_Mode((ADCS_Control_Mode) arguments[0], (uint16_t) arguments[1]);
    return status;
}                                                         

uint8_t TCMDEXEC_ADCS_attitude_estimation_mode(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                               char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t estimation_mode;
    TCMD_extract_uint64_arg(args_str, strlen((char*)args_str), 0, &estimation_mode);
    uint8_t status = ADCS_Attitude_Estimation_Mode((ADCS_Estimation_Mode) estimation_mode); 
    return status;
}                                    

uint8_t TCMDEXEC_ADCS_run_once(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                               char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Run_Once();
    return status;
}                    

uint8_t TCMDEXEC_ADCS_set_magnetometer_mode(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t mode;
    TCMD_extract_uint64_arg(args_str, strlen((char*)args_str), 0, &mode);
    uint8_t status = ADCS_Set_Magnetometer_Mode((ADCS_Magnetometer_Mode) mode);
    return status;
}                                

uint8_t TCMDEXEC_ADCS_set_magnetorquer_output(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Set_Magnetorquer_Output();
    return status;
}                                    

uint8_t TCMDEXEC_ADCS_save_config(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                  char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                        

uint8_t TCMDEXEC_ADCS_estimate_angular_rates(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                             char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                    

uint8_t TCMDEXEC_ADCS_get_llh_position(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                       char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

uint8_t TCMDEXEC_ADCS_get_power_control(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

uint8_t TCMDEXEC_ADCS_set_power_control(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

uint8_t TCMDEXEC_ADCS_set_magnetometer_config(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                    

uint8_t TCMDEXEC_ADCS_bootloader_clear_errors(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                    

uint8_t TCMDEXEC_ADCS_set_unix_time_save_mode(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                    

uint8_t TCMDEXEC_ADCS_get_unix_time_save_mode(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                    

uint8_t TCMDEXEC_ADCS_set_sgp4_orbit_params(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                

uint8_t TCMDEXEC_ADCS_get_sgp4_orbit_params(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                

uint8_t TCMDEXEC_ADCS_save_orbit_params(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

uint8_t TCMDEXEC_ADCS_rate_sensor_rates(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

uint8_t TCMDEXEC_ADCS_get_wheel_speed(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

uint8_t TCMDEXEC_ADCS_get_magnetorquer_command(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                               char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                    

uint8_t TCMDEXEC_ADCS_get_raw_magnetometer_values(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                  char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                        

uint8_t TCMDEXEC_ADCS_fine_angular_rates(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                         char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                

uint8_t TCMDEXEC_ADCS_estimate_fine_angular_rates(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                  char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                        

uint8_t TCMDEXEC_ADCS_get_magnetometer_config(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                    

uint8_t TCMDEXEC_ADCS_get_commanded_attitude_angles(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                    char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                        

uint8_t TCMDEXEC_ADCS_set_commanded_attitude_angles(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                    char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                        

uint8_t TCMDEXEC_ADCS_set_estimation_params(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                

uint8_t TCMDEXEC_ADCS_get_estimation_params(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                

uint8_t TCMDEXEC_ADCS_set_asgp4_params(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                       char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

uint8_t TCMDEXEC_ADCS_get_asgp4_params(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                       char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

uint8_t TCMDEXEC_ADCS_set_tracking_controller_target_reference(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                                char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                                    

uint8_t TCMDEXEC_ADCS_get_tracking_controller_target_reference(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                                char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                                    

uint8_t TCMDEXEC_ADCS_set_rate_gyro_config(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                           char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                

uint8_t TCMDEXEC_ADCS_get_rate_gyro_config(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                           char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                

uint8_t TCMDEXEC_ADCS_estimated_attitude_angles(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                    

uint8_t TCMDEXEC_ADCS_magnetic_field_vector(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                

uint8_t TCMDEXEC_ADCS_fine_sun_vector(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

uint8_t TCMDEXEC_ADCS_nadir_vector(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                        

uint8_t TCMDEXEC_ADCS_commanded_wheel_speed(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                

uint8_t TCMDEXEC_ADCS_igrf_magnetic_field_vector(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                 char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                        

uint8_t TCMDEXEC_ADCS_quaternion_error_vector(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                    

uint8_t TCMDEXEC_ADCS_estimated_gyro_bias(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                          char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                

uint8_t TCMDEXEC_ADCS_estimation_innovation_vector(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                        

uint8_t TCMDEXEC_ADCS_raw_cam1_sensor(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

uint8_t TCMDEXEC_ADCS_raw_cam2_sensor(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

uint8_t TCMDEXEC_ADCS_raw_css_1_to_6(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

uint8_t TCMDEXEC_ADCS_raw_css_7_to_10(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

uint8_t TCMDEXEC_ADCS_cubecontrol_current(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                          char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                

uint8_t TCMDEXEC_ADCS_raw_gps_status(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

uint8_t TCMDEXEC_ADCS_raw_gps_time(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                        

uint8_t TCMDEXEC_ADCS_raw_gps_x(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                    

uint8_t TCMDEXEC_ADCS_raw_gps_y(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                    

uint8_t TCMDEXEC_ADCS_raw_gps_z(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                    

uint8_t TCMDEXEC_ADCS_measurements(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                        

// TODO: telecommand definitions