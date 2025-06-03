#include "adcs_drivers/adcs_types_enum_to_str.h"

char* ADCS_file_type_enum_to_str(ADCS_file_type_enum_t file_type) {
    switch (file_type) {
        case ADCS_FILE_TYPE_TELEMETRY_LOG:
            return "TELEMETRY_LOG";
        case ADCS_FILE_TYPE_JPG_IMAGE:
            return "JPG_IMG";
        case ADCS_FILE_TYPE_BMP_IMAGE:
            return "BMP_IMG";
        case ADCS_FILE_TYPE_INDEX:
            return "INDEX_FILE";
        default:
            return "UNKNOWN";
    }
}
