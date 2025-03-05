#ifndef INCLUDE_GUARD__GPS_TYPES_H
#define INCLUDE_GUARD__GPS_TYPES_H

#include <stdint.h>

// ****************** SECTION: GPS_header_structs ******************

/// @brief  This refers to the status that indicates how well a time is known
/// All reported receiver times are subject to a qualifying time status
/// Refer to page 51-53 of the OEM7 Commands and Logs Reference Manual
typedef enum {
    GPS_REF_TIME_UNKNOWN = 20,
    GPS_REF_TIME_APPROXIMATE = 60,
    GPS_REF_TIME_COARSEADJUSTING = 80,
    GPS_REF_TIME_COARSE = 100,
    GPS_REF_TIME_COARSESTEERING = 120,
    GPS_REF_TIME_FREEWHEELING = 130,
    GPS_REF_TIME_FINEADJUSTING = 140,
    GPS_REF_TIME_FINE = 160,
    GPS_REF_TIME_FINEBACKUPSTEERING = 170,
    GPS_REF_TIME_FINESTEERING = 180,
    GPS_REF_TIME_SATTIME = 200
} GPS_reference_time_status_t;


/// @brief This is the first part of the response that every command response from the GNSS receiver has.
/// Refer to page 34-35 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    /// @brief The ASCII name of the log or command e.g., "BESTXYZA" or "TIMEA"
    char log_name[128];

    /// @brief Indicates the quality of the GPS reference time
    GPS_reference_time_status_t time_status;

} GPS_header_response_t;

// ****************** END SECTION: GPS_header_structs ******************


// ****************** SECTION: GPS_data_structs ******************

/// @brief  This refers to the solution status
/// Refer to table 90 page 500-501 of the OEM7 Commands and Logs Reference Manual
typedef enum {
    GPS_SOL_STATUS_SOL_COMPUTED = 0,
    GPS_SOL_STATUS_INSUFFICIENT_OBS = 1,
    GPS_SOL_STATUS_NO_CONVERGENCE = 2,
    GPS_SOL_STATUS_SINGULARITY = 3,
    GPS_SOL_STATUS_COV_TRACE = 4,
    GPS_SOL_STATUS_TEST_DIST = 5,
    GPS_SOL_STATUS_COLD_START = 6,
    GPS_SOL_STATUS_V_H_LIMIT = 7,
    GPS_SOL_STATUS_VARIANCE = 8,
    GPS_SOL_STATUS_RESIDUALS = 9,
    GPS_SOL_STATUS_RESERVED_10_12 = 10,  // Since 10-12 are reserved
    GPS_SOL_STATUS_INTEGRITY_WARNING = 13
    // 14-17 are reserved
} GPS_solution_status_enum_t;


/// @brief  This refers to the position or velocity type
/// Refer to table 91 page 501-503 of the OEM7 Commands and Logs Reference Manual
typedef enum {
    GPS_TYPE_NONE = 0,
    GPS_TYPE_FIXEDPOS = 1,
    GPS_TYPE_FIXEDHEIGHT = 2,
    GPS_TYPE_RESERVED_3_7 = 3,  // Values from 3 to 7 are reserved
    GPS_TYPE_DOPPLER_VELOCITY = 8,
    GPS_TYPE_RESERVED_9_15 = 9, // Values from 9 to 15 are reserved
    GPS_TYPE_SINGLE = 16,
    GPS_TYPE_PSDIFF = 17,
    GPS_TYPE_WAAS = 18,
    GPS_TYPE_PROPAGATED = 19,
    GPS_TYPE_RESERVED_20_31 = 20, // Values from 20 to 31 are reserved
    GPS_TYPE_L1_FLOAT = 32,
    GPS_TYPE_RESERVED_33 = 33,
    GPS_TYPE_NARROW_FLOAT = 34,
    GPS_TYPE_RESERVED_35_47 = 35, // Values from 35 to 47 are reserved
    GPS_TYPE_L1_INT = 48,
    GPS_TYPE_WIDE_INT = 49,
    GPS_TYPE_NARROW_INT = 50,
    GPS_TYPE_RTK_DIRECT_INS = 51,
    GPS_TYPE_INS_SBAS = 52,
    GPS_TYPE_INS_PSRSP = 53,
    GPS_TYPE_INS_PSRDIFF = 54,
    GPS_TYPE_INS_RTKFLOAT = 55,
    GPS_TYPE_INS_RTKFIXED = 56,
    GPS_TYPE_RESERVED_57_67 = 57, // Values from 57 to 67 are reserved
    GPS_TYPE_PPP_CONVERGING = 68,
    GPS_TYPE_PPP = 69,
    GPS_TYPE_OPERATIONAL = 70,
    GPS_TYPE_WARNING = 71,
    GPS_TYPE_OUT_OF_BOUNDS = 72,
    GPS_TYPE_INS_PPP_CONVERGING = 73,
    GPS_TYPE_INS_PPP = 74,
    GPS_TYPE_PPP_BASIC_CONVERGING = 77,
    GPS_TYPE_PPP_BASIC = 78,
    GPS_TYPE_INS_PPP_BASIC_CONVERGING = 79,
    GPS_TYPE_INS_PPP_BASIC = 80
} GPS_position_type_enum_t;


/// @brief This is the struct for the BESTXYZA Command response.
/// Refer to page 515-517 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    /// @brief Position solution status
    GPS_solution_status_enum_t position_solution_status;

    /// @brief Position type.
    GPS_position_type_enum_t position_type;

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
    
} GPS_bestxyza_response_t;


/// @brief  This refers to the Clock Model Status
/// Refer to table 105 page 526 of the OEM7 Commands and Logs Reference Manual
typedef enum {
    GPS_CLOCK_VALID = 0,
    GPS_CLOCK_CONVERGING = 1,
    GPS_CLOCK_ITERATING = 2,
    GPS_CLOCK_INVALID = 3,
} GPS_clock_model_status_enum_t;

typedef enum {
    GPS_UTC_INVALID = 0,
    GPS_UTC_VALID = 1,
    GPS_UTC_WARNING = 2
} GPS_utc_status_enum_t;

/// @brief This is the struct for the TIMEA Command response.
/// Refer to page 941-943 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    /// @brief Clock model status
    GPS_clock_model_status_enum_t clock_status;

    /// @brief /// The offset of GPS system time from UTC time
    /// Documentation says the utc offset comes as a double. 
    int64_t utc_offset;

    /// @brief UTC status 
    /// Refer to page 943 of the OEM7 Commands and Logs Reference Manual
    GPS_utc_status_enum_t utc_status;

    /// @brief 32 bit CRC (ASCII and Binary only). 
    uint32_t crc;
    
} GPS_timea_response_t;

// ****************** END SECTION: GPS_data_structs ******************

// ****************** SECTION: GPS helper functions ******************

uint8_t GPS_reference_time_status_str_to_enum(const char *status_str, GPS_reference_time_status_t *status);
const char* GPS_reference_time_status_enum_to_str(GPS_reference_time_status_t status);
uint8_t GPS_solution_status_str_to_enum(const char *status_str, GPS_solution_status_enum_t *status);
uint8_t GPS_position_type_str_to_enum(const char *type_str, GPS_position_type_enum_t *type);
const char* GPS_solution_status_enum_to_str(GPS_solution_status_enum_t status);
const char* GPS_position_type_enum_to_string(GPS_position_type_enum_t type);
uint8_t GPS_clock_model_status_str_to_enum(const char *status_str, GPS_clock_model_status_enum_t *status);
uint8_t GPS_utc_status_str_to_enum(const char *status_str, GPS_utc_status_enum_t *status);
const char* GPS_clock_model_status_enum_to_string(GPS_clock_model_status_enum_t status);
const char* GPS_utc_status_enum_to_string(GPS_utc_status_enum_t status);

// ****************** END SECTION: GPS_header_structs ******************

#endif // INCLUDE_GUARD__GPS_TYPES_H
