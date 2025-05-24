#ifndef INCLUDE_GUARD__GNSS_TYPES_H
#define INCLUDE_GUARD__GNSS_TYPES_H

#include <stdint.h>

// ****************** SECTION: GNSS_header_structs ******************

/// @brief  This refers to the status that indicates how well a time is known
/// All reported receiver times are subject to a qualifying time status
/// Refer to page 51-53 of the OEM7 Commands and Logs Reference Manual
typedef enum {
    GNSS_REF_TIME_UNKNOWN = 20,
    GNSS_REF_TIME_APPROXIMATE = 60,
    GNSS_REF_TIME_COARSEADJUSTING = 80,
    GNSS_REF_TIME_COARSE = 100,
    GNSS_REF_TIME_COARSESTEERING = 120,
    GNSS_REF_TIME_FREEWHEELING = 130,
    GNSS_REF_TIME_FINEADJUSTING = 140,
    GNSS_REF_TIME_FINE = 160,
    GNSS_REF_TIME_FINEBACKUPSTEERING = 170,
    GNSS_REF_TIME_FINESTEERING = 180,
    GNSS_REF_TIME_SATTIME = 200
} GNSS_reference_time_status_t;


/// @brief This is the first part of the response that every command response from the GNSS receiver has.
/// Refer to page 34-35 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    /// @brief The ASCII name of the log or command e.g., "BESTXYZA" or "TIMEA"
    char log_name[128];

    /// @brief Indicates the quality of the GNSS reference time
    GNSS_reference_time_status_t time_status;

} GNSS_header_response_t;

// ****************** END SECTION: GNSS_header_structs ******************


// ****************** SECTION: GNSS_data_structs ******************

/// @brief  This refers to the solution status
/// Refer to table 90 page 500-501 of the OEM7 Commands and Logs Reference Manual
typedef enum {
    GNSS_SOL_STATUS_SOL_COMPUTED = 0,
    GNSS_SOL_STATUS_INSUFFICIENT_OBS = 1,
    GNSS_SOL_STATUS_NO_CONVERGENCE = 2,
    GNSS_SOL_STATUS_SINGULARITY = 3,
    GNSS_SOL_STATUS_COV_TRACE = 4,
    GNSS_SOL_STATUS_TEST_DIST = 5,
    GNSS_SOL_STATUS_COLD_START = 6,
    GNSS_SOL_STATUS_V_H_LIMIT = 7,
    GNSS_SOL_STATUS_VARIANCE = 8,
    GNSS_SOL_STATUS_RESIDUALS = 9,
    GNSS_SOL_STATUS_RESERVED_10_12 = 10,  // Since 10-12 are reserved
    GNSS_SOL_STATUS_INTEGRITY_WARNING = 13
    // 14-17 are reserved
} GNSS_solution_status_enum_t;


/// @brief  This refers to the position or velocity type
/// Refer to table 91 page 501-503 of the OEM7 Commands and Logs Reference Manual
typedef enum {
    GNSS_TYPE_NONE = 0,
    GNSS_TYPE_FIXEDPOS = 1,
    GNSS_TYPE_FIXEDHEIGHT = 2,
    GNSS_TYPE_RESERVED_3_7 = 3,  // Values from 3 to 7 are reserved
    GNSS_TYPE_DOPPLER_VELOCITY = 8,
    GNSS_TYPE_RESERVED_9_15 = 9, // Values from 9 to 15 are reserved
    GNSS_TYPE_SINGLE = 16,
    GNSS_TYPE_PSDIFF = 17,
    GNSS_TYPE_WAAS = 18,
    GNSS_TYPE_PROPAGATED = 19,
    GNSS_TYPE_RESERVED_20_31 = 20, // Values from 20 to 31 are reserved
    GNSS_TYPE_L1_FLOAT = 32,
    GNSS_TYPE_RESERVED_33 = 33,
    GNSS_TYPE_NARROW_FLOAT = 34,
    GNSS_TYPE_RESERVED_35_47 = 35, // Values from 35 to 47 are reserved
    GNSS_TYPE_L1_INT = 48,
    GNSS_TYPE_WIDE_INT = 49,
    GNSS_TYPE_NARROW_INT = 50,
    GNSS_TYPE_RTK_DIRECT_INS = 51,
    GNSS_TYPE_INS_SBAS = 52,
    GNSS_TYPE_INS_PSRSP = 53,
    GNSS_TYPE_INS_PSRDIFF = 54,
    GNSS_TYPE_INS_RTKFLOAT = 55,
    GNSS_TYPE_INS_RTKFIXED = 56,
    GNSS_TYPE_RESERVED_57_67 = 57, // Values from 57 to 67 are reserved
    GNSS_TYPE_PPP_CONVERGING = 68,
    GNSS_TYPE_PPP = 69,
    GNSS_TYPE_OPERATIONAL = 70,
    GNSS_TYPE_WARNING = 71,
    GNSS_TYPE_OUT_OF_BOUNDS = 72,
    GNSS_TYPE_INS_PPP_CONVERGING = 73,
    GNSS_TYPE_INS_PPP = 74,
    GNSS_TYPE_PPP_BASIC_CONVERGING = 77,
    GNSS_TYPE_PPP_BASIC = 78,
    GNSS_TYPE_INS_PPP_BASIC_CONVERGING = 79,
    GNSS_TYPE_INS_PPP_BASIC = 80
} GNSS_position_type_enum_t;


/// @brief This is the struct for the BESTXYZA Command response.
/// Refer to page 515-517 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    /// @brief Position solution status
    GNSS_solution_status_enum_t position_solution_status;

    /// @brief Position type.
    GNSS_position_type_enum_t position_type;

    /// @brief Documentation says the point coordinates come as a double. 
    /// We store as an int64 because the earth's radius(6.10^6 m) and we are storing the values as millimeters which fits within an int64.
    /// Position coordinates in millimeters
    int64_t position_x_mm;
    int64_t position_y_mm;
    int64_t position_z_mm;

    /// @brief Documentation says the standard deviation of the position coordinates come as a float. 
    /// The standard deviation of the position coordinates in millimeters
    uint32_t position_x_std_mm;
    uint32_t position_y_std_mm;
    uint32_t position_z_std_mm;

    /// @brief Differential age in seconds. Storing as a uint64 so as to capture the millisecond accuracy
    uint64_t differential_age_ms;

    /// @brief Solution age in seconds. Storing as a uint64 so as to capture the millisecond accuracy 
    uint64_t solution_age_ms;

    /// @brief 32 bit CRC (ASCII and Binary only). 
    uint32_t crc;
    
} GNSS_bestxyza_response_t;


/// @brief  This refers to the Clock Model Status
/// Refer to table 105 page 526 of the OEM7 Commands and Logs Reference Manual
typedef enum {
    GNSS_CLOCK_VALID = 0,
    GNSS_CLOCK_CONVERGING = 1,
    GNSS_CLOCK_ITERATING = 2,
    GNSS_CLOCK_INVALID = 3,
} GNSS_clock_model_status_enum_t;

typedef enum {
    GNSS_UTC_INVALID = 0,
    GNSS_UTC_VALID = 1,
    GNSS_UTC_WARNING = 2
} GNSS_utc_status_enum_t;

/// @brief This is the struct for the TIMEA Command response.
/// Refer to page 941-943 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    /// @brief Clock model status
    GNSS_clock_model_status_enum_t clock_status;

    /// @brief /// The offset of GNSS system time from UTC time
    /// Documentation says the utc offset comes as a double. 
    int64_t utc_offset;

    /// @brief UTC status 
    /// Refer to page 943 of the OEM7 Commands and Logs Reference Manual
    GNSS_utc_status_enum_t utc_status;

    /// @brief 32 bit CRC (ASCII and Binary only). 
    uint32_t crc;
    
} GNSS_timea_response_t;

// ****************** END SECTION: GNSS_data_structs ******************

// ****************** SECTION: GNSS helper functions ******************

uint8_t GNSS_reference_time_status_str_to_enum(const char *status_str, GNSS_reference_time_status_t *status);
const char* GNSS_reference_time_status_enum_to_str(GNSS_reference_time_status_t status);
uint8_t GNSS_solution_status_str_to_enum(const char *status_str, GNSS_solution_status_enum_t *status);
uint8_t GNSS_position_type_str_to_enum(const char *type_str, GNSS_position_type_enum_t *type);
const char* GNSS_solution_status_enum_to_str(GNSS_solution_status_enum_t status);
const char* GNSS_position_type_enum_to_string(GNSS_position_type_enum_t type);
uint8_t GNSS_clock_model_status_str_to_enum(const char *status_str, GNSS_clock_model_status_enum_t *status);
uint8_t GNSS_utc_status_str_to_enum(const char *status_str, GNSS_utc_status_enum_t *status);
const char* GNSS_clock_model_status_enum_to_string(GNSS_clock_model_status_enum_t status);
const char* GNSS_utc_status_enum_to_string(GNSS_utc_status_enum_t status);

// ****************** END SECTION: GNSS_header_structs ******************

#endif // INCLUDE_GUARD__GNSS_TYPES_H