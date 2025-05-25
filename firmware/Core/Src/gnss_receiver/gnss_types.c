#include "gnss_receiver/gnss_types.h"
 
#include <string.h>

/// @brief Assigns a GNSS time status based on the provided string.
/// @param status_str The status string to parse.
/// @param status Pointer to GNSS_reference_time_status_t where the status will be stored.
/// @return Returns 0 on success, 1 if the status string is unrecognized.
uint8_t GNSS_reference_time_status_str_to_enum(const char *status_str, GNSS_reference_time_status_t *status) {
    if (strcmp(status_str, "UNKNOWN") == 0) {
        *status = GNSS_REF_TIME_UNKNOWN;
    } else if (strcmp(status_str, "APPROXIMATE") == 0) {
        *status = GNSS_REF_TIME_APPROXIMATE;
    } else if (strcmp(status_str, "COARSEADJUSTING") == 0) {
        *status = GNSS_REF_TIME_COARSEADJUSTING;
    } else if (strcmp(status_str, "COARSE") == 0) {
        *status = GNSS_REF_TIME_COARSE;
    } else if (strcmp(status_str, "COARSESTEERING") == 0) {
        *status = GNSS_REF_TIME_COARSESTEERING;
    } else if (strcmp(status_str, "FREEWHEELING") == 0) {
        *status = GNSS_REF_TIME_FREEWHEELING;
    } else if (strcmp(status_str, "FINEADJUSTING") == 0) {
        *status = GNSS_REF_TIME_FINEADJUSTING;
    } else if (strcmp(status_str, "FINE") == 0) {
        *status = GNSS_REF_TIME_FINE;
    } else if (strcmp(status_str, "FINEBACKUPSTEERING") == 0) {
        *status = GNSS_REF_TIME_FINEBACKUPSTEERING;
    } else if (strcmp(status_str, "FINESTEERING") == 0) {
        *status = GNSS_REF_TIME_FINESTEERING;
    } else if (strcmp(status_str, "SATTIME") == 0) {
        *status = GNSS_REF_TIME_SATTIME;
    } else {
        return 1;  // Unrecognized status string
    }
    return 0;  // Success
}


/// @brief Assigns a string value based on the provided GNSS time status.
/// @param status GNSS_reference_time_status_t value.
/// @return Returns the assigned string value for the GNSS time status.
const char* GNSS_reference_time_status_enum_to_str(GNSS_reference_time_status_t status) {
    switch (status) {
        case GNSS_REF_TIME_UNKNOWN:
            return "UNKNOWN";
        case GNSS_REF_TIME_APPROXIMATE:
            return "APPROXIMATE";
        case GNSS_REF_TIME_COARSEADJUSTING:
            return "COARSEADJUSTING";
        case GNSS_REF_TIME_COARSE:
            return "COARSE";
        case GNSS_REF_TIME_COARSESTEERING:
            return "COARSESTEERING";
        case GNSS_REF_TIME_FREEWHEELING:
            return "FREEWHEELING";
        case GNSS_REF_TIME_FINEADJUSTING:
            return "FINEADJUSTING";
        case GNSS_REF_TIME_FINE:
            return "FINE";
        case GNSS_REF_TIME_FINEBACKUPSTEERING:
            return "FINEBACKUPSTEERING";
        case GNSS_REF_TIME_FINESTEERING:
            return "FINESTEERING";
        case GNSS_REF_TIME_SATTIME:
            return "SATTIME";
        default:
            return "UNKNOWN STATUS";  // If status is unrecognized
    }
}


/// @brief Assigns a GNSS solution status based on the provided string.
/// @param status_str The status string to parse.
/// @param status Pointer to GNSS_solution_status_enum_t where the status will be stored.
/// @return Returns 0 on success, 1 if the status string is unrecognized.
uint8_t GNSS_solution_status_str_to_enum(const char *status_str, GNSS_solution_status_enum_t *status) {
    if (strcmp(status_str, "SOL_COMPUTED") == 0) {
        *status = GNSS_SOL_STATUS_SOL_COMPUTED;
    } else if (strcmp(status_str, "INSUFFICIENT_OBS") == 0) {
        *status = GNSS_SOL_STATUS_INSUFFICIENT_OBS;
    } else if (strcmp(status_str, "NO_CONVERGENCE") == 0) {
        *status = GNSS_SOL_STATUS_NO_CONVERGENCE;
    } else if (strcmp(status_str, "SINGULARITY") == 0) {
        *status = GNSS_SOL_STATUS_SINGULARITY;
    } else if (strcmp(status_str, "COV_TRACE") == 0) {
        *status = GNSS_SOL_STATUS_COV_TRACE;
    } else if (strcmp(status_str, "TEST_DIST") == 0) {
        *status = GNSS_SOL_STATUS_TEST_DIST;
    } else if (strcmp(status_str, "COLD_START") == 0) {
        *status = GNSS_SOL_STATUS_COLD_START;
    } else if (strcmp(status_str, "V_H_LIMIT") == 0) {
        *status = GNSS_SOL_STATUS_V_H_LIMIT;
    } else if (strcmp(status_str, "VARIANCE") == 0) {
        *status = GNSS_SOL_STATUS_VARIANCE;
    } else if (strcmp(status_str, "RESIDUALS") == 0) {
        *status = GNSS_SOL_STATUS_RESIDUALS;
    } else if (strcmp(status_str, "INTEGRITY_WARNING") == 0) {
        *status = GNSS_SOL_STATUS_INTEGRITY_WARNING;
    } else {
        return 1;  // Unrecognized status string
    }
    return 0;  // Success
}


/// @brief Assigns a GNSS position or velocity type based on the provided string.
/// @param type_str The type string to parse.
/// @param type Pointer to GNSS_position_velocity_type_enum_t where the type will be stored.
/// @return Returns 0 on success, 1 if the type string is unrecognized.
uint8_t GNSS_position_type_str_to_enum(const char *type_str, GNSS_position_type_enum_t *type) {
    if (strcmp(type_str, "NONE") == 0) {
        *type = GNSS_TYPE_NONE;
    } else if (strcmp(type_str, "FIXEDPOS") == 0) {
        *type = GNSS_TYPE_FIXEDPOS;
    } else if (strcmp(type_str, "FIXEDHEIGHT") == 0) {
        *type = GNSS_TYPE_FIXEDHEIGHT;
    } else if (strcmp(type_str, "DOPPLER_VELOCITY") == 0) {
        *type = GNSS_TYPE_DOPPLER_VELOCITY;
    } else if (strcmp(type_str, "SINGLE") == 0) {
        *type = GNSS_TYPE_SINGLE;
    } else if (strcmp(type_str, "PSDIFF") == 0) {
        *type = GNSS_TYPE_PSDIFF;
    } else if (strcmp(type_str, "WAAS") == 0) {
        *type = GNSS_TYPE_WAAS;
    } else if (strcmp(type_str, "PROPAGATED") == 0) {
        *type = GNSS_TYPE_PROPAGATED;
    } else if (strcmp(type_str, "L1_FLOAT") == 0) {
        *type = GNSS_TYPE_L1_FLOAT;
    } else if (strcmp(type_str, "NARROW_FLOAT") == 0) {
        *type = GNSS_TYPE_NARROW_FLOAT;
    } else if (strcmp(type_str, "L1_INT") == 0) {
        *type = GNSS_TYPE_L1_INT;
    } else if (strcmp(type_str, "WIDE_INT") == 0) {
        *type = GNSS_TYPE_WIDE_INT;
    } else if (strcmp(type_str, "NARROW_INT") == 0) {
        *type = GNSS_TYPE_NARROW_INT;
    } else if (strcmp(type_str, "RTK_DIRECT_INS") == 0) {
        *type = GNSS_TYPE_RTK_DIRECT_INS;
    } else if (strcmp(type_str, "INS_SBAS") == 0) {
        *type = GNSS_TYPE_INS_SBAS;
    } else if (strcmp(type_str, "INS_PSRSP") == 0) {
        *type = GNSS_TYPE_INS_PSRSP;
    } else if (strcmp(type_str, "INS_PSRDIFF") == 0) {
        *type = GNSS_TYPE_INS_PSRDIFF;
    } else if (strcmp(type_str, "INS_RTKFLOAT") == 0) {
        *type = GNSS_TYPE_INS_RTKFLOAT;
    } else if (strcmp(type_str, "INS_RTKFIXED") == 0) {
        *type = GNSS_TYPE_INS_RTKFIXED;
    } else if (strcmp(type_str, "PPP_CONVERGING") == 0) {
        *type = GNSS_TYPE_PPP_CONVERGING;
    } else if (strcmp(type_str, "PPP") == 0) {
        *type = GNSS_TYPE_PPP;
    } else if (strcmp(type_str, "OPERATIONAL") == 0) {
        *type = GNSS_TYPE_OPERATIONAL;
    } else if (strcmp(type_str, "WARNING") == 0) {
        *type = GNSS_TYPE_WARNING;
    } else if (strcmp(type_str, "OUT_OF_BOUNDS") == 0) {
        *type = GNSS_TYPE_OUT_OF_BOUNDS;
    } else if (strcmp(type_str, "INS_PPP_CONVERGING") == 0) {
        *type = GNSS_TYPE_INS_PPP_CONVERGING;
    } else if (strcmp(type_str, "INS_PPP") == 0) {
        *type = GNSS_TYPE_INS_PPP;
    } else if (strcmp(type_str, "PPP_BASIC_CONVERGING") == 0) {
        *type = GNSS_TYPE_PPP_BASIC_CONVERGING;
    } else if (strcmp(type_str, "PPP_BASIC") == 0) {
        *type = GNSS_TYPE_PPP_BASIC;
    } else if (strcmp(type_str, "INS_PPP_BASIC_CONVERGING") == 0) {
        *type = GNSS_TYPE_INS_PPP_BASIC_CONVERGING;
    } else if (strcmp(type_str, "INS_PPP_BASIC") == 0) {
        *type = GNSS_TYPE_INS_PPP_BASIC;
    } else {
        return 1;  // Unrecognized type string
    }
    return 0;  // Success
}

/// @brief Assigns a string value based on the provided GNSS solution status.
/// @param status GNSS_solution_status_enum_t value.
/// @return Returns the assigned string value for the GNSS_solution_status_enum_t.
const char* GNSS_solution_status_enum_to_str(GNSS_solution_status_enum_t status) {
    switch (status) {
        case GNSS_SOL_STATUS_SOL_COMPUTED:
            return "SOL_COMPUTED";
        case GNSS_SOL_STATUS_INSUFFICIENT_OBS:
            return "INSUFFICIENT_OBS";
        case GNSS_SOL_STATUS_NO_CONVERGENCE:
            return "NO_CONVERGENCE";
        case GNSS_SOL_STATUS_SINGULARITY:
            return "SINGULARITY";
        case GNSS_SOL_STATUS_COV_TRACE:
            return "COV_TRACE";
        case GNSS_SOL_STATUS_TEST_DIST:
            return "TEST_DIST";
        case GNSS_SOL_STATUS_COLD_START:
            return "COLD_START";
        case GNSS_SOL_STATUS_V_H_LIMIT:
            return "V_H_LIMIT";
        case GNSS_SOL_STATUS_VARIANCE:
            return "VARIANCE";
        case GNSS_SOL_STATUS_RESIDUALS:
            return "RESIDUALS";
        case GNSS_SOL_STATUS_INTEGRITY_WARNING:
            return "INTEGRITY_WARNING";
        default:
            return "UNKNOWN STATUS";  // For unrecognized enum values
    }
}

/// @brief Assigns a string value based on the provided GNSS position type.
/// @param type GNSS_position_type_enum_t value.
/// @return Returns the assigned string value for the GNSS_position_type_enum_t.
const char* GNSS_position_type_enum_to_string(GNSS_position_type_enum_t type) {
    switch (type) {
        case GNSS_TYPE_NONE:
            return "NONE";
        case GNSS_TYPE_FIXEDPOS:
            return "FIXEDPOS";
        case GNSS_TYPE_FIXEDHEIGHT:
            return "FIXEDHEIGHT";
        case GNSS_TYPE_DOPPLER_VELOCITY:
            return "DOPPLER_VELOCITY";
        case GNSS_TYPE_SINGLE:
            return "SINGLE";
        case GNSS_TYPE_PSDIFF:
            return "PSDIFF";
        case GNSS_TYPE_WAAS:
            return "WAAS";
        case GNSS_TYPE_PROPAGATED:
            return "PROPAGATED";
        case GNSS_TYPE_L1_FLOAT:
            return "L1_FLOAT";
        case GNSS_TYPE_NARROW_FLOAT:
            return "NARROW_FLOAT";
        case GNSS_TYPE_L1_INT:
            return "L1_INT";
        case GNSS_TYPE_WIDE_INT:
            return "WIDE_INT";
        case GNSS_TYPE_NARROW_INT:
            return "NARROW_INT";
        case GNSS_TYPE_RTK_DIRECT_INS:
            return "RTK_DIRECT_INS";
        case GNSS_TYPE_INS_SBAS:
            return "INS_SBAS";
        case GNSS_TYPE_INS_PSRSP:
            return "INS_PSRSP";
        case GNSS_TYPE_INS_PSRDIFF:
            return "INS_PSRDIFF";
        case GNSS_TYPE_INS_RTKFLOAT:
            return "INS_RTKFLOAT";
        case GNSS_TYPE_INS_RTKFIXED:
            return "INS_RTKFIXED";
        case GNSS_TYPE_PPP_CONVERGING:
            return "PPP_CONVERGING";
        case GNSS_TYPE_PPP:
            return "PPP";
        case GNSS_TYPE_OPERATIONAL:
            return "OPERATIONAL";
        case GNSS_TYPE_WARNING:
            return "WARNING";
        case GNSS_TYPE_OUT_OF_BOUNDS:
            return "OUT_OF_BOUNDS";
        case GNSS_TYPE_INS_PPP_CONVERGING:
            return "INS_PPP_CONVERGING";
        case GNSS_TYPE_INS_PPP:
            return "INS_PPP";
        case GNSS_TYPE_PPP_BASIC_CONVERGING:
            return "PPP_BASIC_CONVERGING";
        case GNSS_TYPE_PPP_BASIC:
            return "PPP_BASIC";
        case GNSS_TYPE_INS_PPP_BASIC_CONVERGING:
            return "INS_PPP_BASIC_CONVERGING";
        case GNSS_TYPE_INS_PPP_BASIC:
            return "INS_PPP_BASIC";
        default:
            return "UNKNOWN TYPE";  // If type is unrecognized
    }
}


/// @brief Assigns a GNSS Clock Model status based on the provided string.
/// @param status_str The status string to parse.
/// @param status Pointer to GNSS_clock_model_status_enum_t where the status will be stored.
/// @return Returns 0 on success, 1 if the status string is unrecognized.
uint8_t GNSS_clock_model_status_str_to_enum(const char *status_str, GNSS_clock_model_status_enum_t *status) {
    if (strcmp(status_str, "VALID") == 0) {
        *status = GNSS_CLOCK_VALID;
    } else if (strcmp(status_str, "CONVERGING") == 0) {
        *status = GNSS_CLOCK_CONVERGING;
    } else if (strcmp(status_str, "ITERATING") == 0) {
        *status = GNSS_CLOCK_ITERATING;
    } else if (strcmp(status_str, "INVALID") == 0) {
        *status = GNSS_CLOCK_INVALID;
    } else {
        return 1;  // Unrecognized status string
    }
    return 0;  // Success
}

/// @brief Assigns a GNSS UTC status based on the provided string.
/// @param status_str The status string to parse.
/// @param status Pointer to GNSS_utc_status_enum_t where the status will be stored.
/// @return Returns 0 on success, 1 if the status string is unrecognized.
uint8_t GNSS_utc_status_str_to_enum(const char *status_str, GNSS_utc_status_enum_t *status) {
    if (strcmp(status_str, "INVALID") == 0) {
        *status = GNSS_UTC_INVALID;
    } else if (strcmp(status_str, "VALID") == 0) {
        *status = GNSS_UTC_VALID;
    } else if (strcmp(status_str, "WARNING") == 0) {
        *status = GNSS_UTC_WARNING;
    } else {
        return 1;  // Unrecognized status string
    }
    return 0;  // Success
}

/// @brief Assigns a string value based on the provided GNSS clock model status.
/// @param status GNSS_clock_model_status_enum_t value.
/// @return Returns the assigned string value for the GNSS_clock_model_status_enum_t.
const char* GNSS_clock_model_status_enum_to_string(GNSS_clock_model_status_enum_t status) {
    switch (status) {
        case GNSS_CLOCK_VALID:
            return "VALID";
        case GNSS_CLOCK_CONVERGING:
            return "CONVERGING";
        case GNSS_CLOCK_ITERATING:
            return "ITERATING";
        case GNSS_CLOCK_INVALID:
            return "INVALID";
        default:
            return "UNKNOWN STATUS";  // For unrecognized enum values
    }
}

/// @brief Assigns a string value based on the provided GNSS UTC status.
/// @param status GNSS_utc_status_enum_t value.
/// @return Returns the assigned string value for the GNSS_utc_status_enum_t.
const char* GNSS_utc_status_enum_to_string(GNSS_utc_status_enum_t status) {
    switch (status) {
        case GNSS_UTC_INVALID:
            return "INVALID";
        case GNSS_UTC_VALID:
            return "VALID";
        case GNSS_UTC_WARNING:
            return "WARNING";
        default:
            return "UNKNOWN STATUS";  // For unrecognized enum values
    }
}