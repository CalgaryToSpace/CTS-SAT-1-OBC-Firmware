#include "unit_tests/unit_test_helpers.h"
#include "unit_tests/unit_test_inventory.h"

#include "unit_tests/test_assorted_prototypes.h"
#include "unit_tests/test_byte_transforms.h"
#include "unit_tests/test_telecommand_parser.h"
#include "unit_tests/test_tests.h"
#include "unit_tests/test_adcs.h"


// extern
const TEST_Definition_t TEST_definitions[] = {

    {
        .test_func = TEST_EXEC__test_asserts,
        .test_file = "unit_tests/test_tests",
        .test_func_name = "test_asserts"
    },
    
    {
        .test_func = TEST_EXEC__this_test_always_fails,
        .test_file = "unit_tests/test_tests",
        .test_func_name = "this_test_always_fails"
    },

    {
        .test_func = TEST_EXEC__TCMD_is_char_alphanumeric,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_is_char_alphanumeric"
    },
    
    {
        .test_func = TEST_EXEC__GEN_get_index_of_substring_in_array,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "GEN_get_index_of_substring_in_array"
    },
    
    {
        .test_func = TEST_EXEC__TCMD_check_starts_with_device_id,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_check_starts_with_device_id"
    },
    
    {
        .test_func = TEST_EXEC__GEN_convert_big_endian_four_byte_array_to_uint32,
        .test_file = "transforms/byte_transforms",
        .test_func_name = "GEN_convert_big_endian_four_byte_array_to_uint32"
    },
    
    /* TODO: ADCS unit tests
    {
		.test_func = TEST_EXEC__ADCS_function_name,
		.test_file = "unit_tests/test_adcs",
		.test_func_name = "ADCS_function_name"
	},
    */

    {
		.test_func = TEST_EXEC__ADCS_Pack_to_Ack,
		.test_file = "unit_tests/test_adcs",
		.test_func_name = "ADCS_Pack_to_Ack"
	},

    {
		.test_func = TEST_EXEC__ADCS_Pack_to_Identification,
		.test_file = "unit_tests/test_adcs",
		.test_func_name = "ADCS_Pack_to_Identification"
	},

    {
		.test_func = TEST_EXEC__ADCS_Pack_to_Program_Status,
		.test_file = "unit_tests/test_adcs",
		.test_func_name = "ADCS_Pack_to_Program_Status"
	},

    {
		.test_func = TEST_EXEC__ADCS_Pack_to_Comms_Status,
		.test_file = "unit_tests/test_adcs",
		.test_func_name = "ADCS_Pack_to_Comms_Status"
	},

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Power_Control,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Power_Control"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Angular_Rates,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Angular_Rates"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_LLH_Position,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_LLH_Position"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Unix_Time_Save_Mode,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Unix_Time_Save_Mode"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Orbit_Params,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Orbit_Params"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Rated_Sensor_Rates,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Rated_Sensor_Rates"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Wheel_Speed,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Wheel_Speed"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Magnetorquer_Command,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Magnetorquer_Command"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Raw_Magnetometer_Values,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_Magnetometer_Values"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Fine_Angular_Rates,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Fine_Angular_Rates"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Magnetometer_Config,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Magnetometer_Config"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Commanded_Attitude_Angles,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Commanded_Attitude_Angles"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Estimation_Params,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Estimation_Params"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_ASGP4_Params,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_ASGP4_Params"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Tracking_Controller_Target_Reference,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Tracking_Controller_Target_Reference"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Rate_Gyro_Config,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Rate_Gyro_Config"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Estimated_Attitude_Angles,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Estimated_Attitude_Angles"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Magnetic_Field_Vector,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Magnetic_Field_Vector"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Fine_Sun_Vector,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Fine_Sun_Vector"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Nadir_Vector,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Nadir_Vector"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Commanded_Wheel_Speed,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Commanded_Wheel_Speed"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_Pack_to_IGRF_Magnetic_Field_Vector,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_IGRF_Magnetic_Field_Vector"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Quaternion_Error_Vector,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Quaternion_Error_Vector"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Estimated_Gyro_Bias,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Estimated_Gyro_Bias"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Estimation_Innovation_Vector,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Estimation_Innovation_Vector"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Raw_Cam1_Sensor,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_Cam1_Sensor"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Raw_Cam2_Sensor,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_Cam2_Sensor"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Raw_CSS_1_to_6,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_CSS_1_to_6"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Raw_CSS_7_to_10,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_CSS_7_to_10"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_CubeControl_Current,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_CubeControl_Current"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Raw_GPS_Status,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_GPS_Status"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Raw_GPS_Time,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_GPS_Time"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Raw_GPS,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_GPS"
    },

    {
        .test_func = TEST_EXEC__ADCS_Pack_to_Measurements,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Measurements"
    },

};

// extern
const int16_t TEST_definitions_count = sizeof(TEST_definitions) / sizeof(TEST_Definition_t);
