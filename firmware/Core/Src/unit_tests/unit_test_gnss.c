#include "unit_tests/unit_test_gnss.h"
#include "unit_tests/unit_test_helpers.h"
#include "transforms/number_comparisons.h"
#include "gnss_receiver/gnss_ascii_parsers.h"
#include "gnss_receiver/gnss_types.h"

#include <string.h>


uint8_t TEST_EXEC__GNSS_reference_time_status_str_to_enum() {
    GNSS_reference_time_status_t status;

    // Test with recognized status strings
    TEST_ASSERT_TRUE(GNSS_reference_time_status_str_to_enum("UNKNOWN", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_REF_TIME_UNKNOWN);
    TEST_ASSERT_TRUE(GNSS_reference_time_status_str_to_enum("APPROXIMATE", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_REF_TIME_APPROXIMATE);
    TEST_ASSERT_TRUE(GNSS_reference_time_status_str_to_enum("COARSEADJUSTING", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_REF_TIME_COARSEADJUSTING);
    TEST_ASSERT_TRUE(GNSS_reference_time_status_str_to_enum("COARSE", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_REF_TIME_COARSE);
    TEST_ASSERT_TRUE(GNSS_reference_time_status_str_to_enum("COARSESTEERING", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_REF_TIME_COARSESTEERING);
    TEST_ASSERT_TRUE(GNSS_reference_time_status_str_to_enum("FREEWHEELING", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_REF_TIME_FREEWHEELING);
    TEST_ASSERT_TRUE(GNSS_reference_time_status_str_to_enum("FINEADJUSTING", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_REF_TIME_FINEADJUSTING);
    TEST_ASSERT_TRUE(GNSS_reference_time_status_str_to_enum("FINE", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_REF_TIME_FINE);
    TEST_ASSERT_TRUE(GNSS_reference_time_status_str_to_enum("FINEBACKUPSTEERING", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_REF_TIME_FINEBACKUPSTEERING);
    TEST_ASSERT_TRUE(GNSS_reference_time_status_str_to_enum("FINESTEERING", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_REF_TIME_FINESTEERING);
    TEST_ASSERT_TRUE(GNSS_reference_time_status_str_to_enum("SATTIME", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_REF_TIME_SATTIME);

    // Test with an unrecognized status string
    TEST_ASSERT_TRUE(GNSS_reference_time_status_str_to_enum("INVALID_STATUS", &status) == 1);

    // Test with an empty string
    TEST_ASSERT_TRUE(GNSS_reference_time_status_str_to_enum("", &status) == 1);

    // Test with a NULL string
    TEST_ASSERT_TRUE(GNSS_reference_time_status_str_to_enum(NULL, &status) == 1);

    return 0;

}

uint8_t TEST_EXEC__GNSS_solution_status_str_to_enum() {
    GNSS_solution_status_enum_t status;

    // Test with recognized status strings
    TEST_ASSERT_TRUE(GNSS_solution_status_str_to_enum("SOL_COMPUTED", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_SOL_STATUS_SOL_COMPUTED);
    TEST_ASSERT_TRUE(GNSS_solution_status_str_to_enum("INSUFFICIENT_OBS", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_SOL_STATUS_INSUFFICIENT_OBS);
    TEST_ASSERT_TRUE(GNSS_solution_status_str_to_enum("NO_CONVERGENCE", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_SOL_STATUS_NO_CONVERGENCE);
    TEST_ASSERT_TRUE(GNSS_solution_status_str_to_enum("SINGULARITY", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_SOL_STATUS_SINGULARITY);
    TEST_ASSERT_TRUE(GNSS_solution_status_str_to_enum("COV_TRACE", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_SOL_STATUS_COV_TRACE);
    TEST_ASSERT_TRUE(GNSS_solution_status_str_to_enum("TEST_DIST", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_SOL_STATUS_TEST_DIST);
    TEST_ASSERT_TRUE(GNSS_solution_status_str_to_enum("COLD_START", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_SOL_STATUS_COLD_START);
    TEST_ASSERT_TRUE(GNSS_solution_status_str_to_enum("V_H_LIMIT", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_SOL_STATUS_V_H_LIMIT);
    TEST_ASSERT_TRUE(GNSS_solution_status_str_to_enum("VARIANCE", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_SOL_STATUS_VARIANCE);
    TEST_ASSERT_TRUE(GNSS_solution_status_str_to_enum("RESIDUALS", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_SOL_STATUS_RESIDUALS);
    TEST_ASSERT_TRUE(GNSS_solution_status_str_to_enum("INTEGRITY_WARNING", &status) == 0);
    TEST_ASSERT_TRUE(status == GNSS_SOL_STATUS_INTEGRITY_WARNING);

    // Test with an unrecognized status string
    TEST_ASSERT_TRUE(GNSS_solution_status_str_to_enum("UNKNOWN_STATUS", &status) == 1);

    // Test with an empty string
    TEST_ASSERT_TRUE(GNSS_solution_status_str_to_enum("", &status) == 1);

    // Test with a NULL string
    TEST_ASSERT_TRUE(GNSS_solution_status_str_to_enum(NULL, &status) == 1);

    return 0;
}


uint8_t TEST_EXEC__GNSS_position_type_str_to_enum() {
    GNSS_position_type_enum_t type;

    // Test with recognized type strings
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("NONE", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_NONE);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("FIXEDPOS", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_FIXEDPOS);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("FIXEDHEIGHT", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_FIXEDHEIGHT);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("DOPPLER_VELOCITY", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_DOPPLER_VELOCITY);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("SINGLE", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_SINGLE);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("PSDIFF", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_PSDIFF);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("WAAS", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_WAAS);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("PROPAGATED", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_PROPAGATED);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("L1_FLOAT", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_L1_FLOAT);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("NARROW_FLOAT", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_NARROW_FLOAT);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("L1_INT", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_L1_INT);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("WIDE_INT", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_WIDE_INT);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("NARROW_INT", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_NARROW_INT);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("RTK_DIRECT_INS", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_RTK_DIRECT_INS);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("INS_SBAS", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_INS_SBAS);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("INS_PSRSP", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_INS_PSRSP);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("INS_PSRDIFF", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_INS_PSRDIFF);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("INS_RTKFLOAT", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_INS_RTKFLOAT);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("INS_RTKFIXED", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_INS_RTKFIXED);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("PPP_CONVERGING", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_PPP_CONVERGING);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("PPP", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_PPP);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("OPERATIONAL", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_OPERATIONAL);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("WARNING", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_WARNING);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("OUT_OF_BOUNDS", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_OUT_OF_BOUNDS);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("INS_PPP_CONVERGING", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_INS_PPP_CONVERGING);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("INS_PPP", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_INS_PPP);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("PPP_BASIC_CONVERGING", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_PPP_BASIC_CONVERGING);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("PPP_BASIC", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_PPP_BASIC);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("INS_PPP_BASIC_CONVERGING", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_INS_PPP_BASIC_CONVERGING);
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("INS_PPP_BASIC", &type) == 0);
    TEST_ASSERT_TRUE(type == GNSS_TYPE_INS_PPP_BASIC);

    // Test with an unrecognized type string
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("INVALID_TYPE", &type) == 1);

    // Test with an empty string
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum("", &type) == 1);

    // Test with a NULL string
    TEST_ASSERT_TRUE(GNSS_position_type_str_to_enum(NULL, &type) == 1);

    return 0;
}

uint8_t TEST_EXEC__GNSS_header_response_parser(){
    // TODO: Try testing with another header string
    
    // Testing with a Valid GNSS Header
    char gnss_data[600] = "#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;"
                "SOL_COMPUTED,NARROW_INT,-1634531.5683,-3664618.0326,4942496.3270,0.0099,"
                "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                "0.0230,\"AAAA\",0.250,1.000,0.000,12,11,11,11,0,01,0,33*e9eafeca";

    GNSS_header_response_t gnss_header_result;

    uint8_t result = GNSS_header_response_parser(gnss_data, &gnss_header_result);
    TEST_ASSERT_TRUE(result == 0);
    TEST_ASSERT_TRUE(strcmp(gnss_header_result.log_name, "BESTXYZA") == 0);
    TEST_ASSERT_TRUE(gnss_header_result.time_status == GNSS_REF_TIME_FINESTEERING);

    // Testing with a different Valid GNSS Header
    strcpy(gnss_data,
    "#TIMEA,COM1,0,86.5,FINESTEERING,1930,428348.000,02000020,"
    "9924,32768;VALID,1.667187222e-10,9.641617960e-10,-18.00000000000,"
    "2017,1,5,22,58,50000,VALID*2a066e78");
    
    result = GNSS_header_response_parser(gnss_data, &gnss_header_result);
    TEST_ASSERT_TRUE(result == 0);

    // Testing an empty string
    strcpy(gnss_data,"");
    result = GNSS_header_response_parser(gnss_data, &gnss_header_result);
    TEST_ASSERT_TRUE(result == 1);

    // Testing Missing Sync Character
    strcpy(gnss_data,"BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;");
    result = GNSS_header_response_parser(gnss_data, &gnss_header_result);
    TEST_ASSERT_TRUE(result == 2);

    // Testing missing Terminating character
    strcpy(gnss_data,"#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724");
    result = GNSS_header_response_parser(gnss_data, &gnss_header_result);
    TEST_ASSERT_TRUE(result == 2);

    // Testing missing both Sync and Terminating Character
    strcpy(gnss_data,"BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724");
    result = GNSS_header_response_parser(gnss_data, &gnss_header_result);
    TEST_ASSERT_TRUE(result == 2);

    // Testing missing both Sync and Terminating Character
    strcpy(gnss_data,"BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724");
    result = GNSS_header_response_parser(gnss_data, &gnss_header_result);
    TEST_ASSERT_TRUE(result == 2);

    // Testing when the ; charcacter comes before the # character
    strcpy(gnss_data,
                "02000040,d821,2724;SOL_COMPUTED,NARROW_INT,-1634531.5683,-3664618.0326,4942496.3270,"
                "0.0099,0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                "0.0230,\"AAAA\",0.250,1.000,0.000,12,11,11,11,0,01,0,33*e9eafeca"
                "#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;" );
    result = GNSS_header_response_parser(gnss_data, &gnss_header_result);
    TEST_ASSERT_TRUE(result == 3);

    // Testing when the header buffer is exceeded
    strcpy(gnss_data,"#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724"
                    "BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724"
                    "BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724"
                    "BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;");
    result = GNSS_header_response_parser(gnss_data, &gnss_header_result);
    TEST_ASSERT_TRUE(result == 4);

    return 0;
}

uint8_t TEST_EXEC__GNSS_bestxyza_data_parser(){
    char gnss_data[600] = "#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;"
                "SOL_COMPUTED,NARROW_INT,-1634531.5683,-3664618.0326,-3664618.0326,0.0099,"
                "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                "0.0230,\"AAAA\",0.250,1.000,0.000,12,11,11,11,0,01,0,33*e9eafeca";

    GNSS_bestxyza_response_t result;

    // Call bestxyza parser
    uint8_t parse_result = GNSS_bestxyza_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 0);
    TEST_ASSERT_TRUE(result.position_solution_status == GNSS_SOL_STATUS_SOL_COMPUTED);
    TEST_ASSERT_TRUE(result.position_type == GNSS_TYPE_NARROW_INT);
    TEST_ASSERT_TRUE(result.position_x_mm == -1634531568);
    TEST_ASSERT_TRUE(result.position_y_mm == -3664618032);
    TEST_ASSERT_TRUE(result.position_z_mm == -3664618032);
    TEST_ASSERT_TRUE(result.position_x_std_mm == 9);
    TEST_ASSERT_TRUE(result.position_y_std_mm == 21);
    TEST_ASSERT_TRUE(result.position_z_std_mm == 11);
    TEST_ASSERT_TRUE(result.differential_age_ms == 1000);
    TEST_ASSERT_TRUE(result.solution_age_ms == 0000);
    TEST_ASSERT_TRUE(result.crc == 0xe9eafeca);

    

    // Testing with an empty string
    strcpy(gnss_data, "");
    parse_result = GNSS_bestxyza_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 1);

    // Introducing an error within the header of the GNSS response: Missing delimiting character
    strcpy(gnss_data, "#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724"
                "SOL_COMPUTED,NARROW_INT,-1634531.5683,-3664618.0326,-3664618.0326,0.0099,"
                "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                "0.0230,\"AAAA\",0.250,1.000,0.000,12,11,11,11,0,01,0,33*e9eafeca");
    parse_result = GNSS_bestxyza_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 2);

    // Testing with a different GNSS Header
    strcpy(gnss_data,
    "#TIMEA,COM1,0,86.5,FINESTEERING,1930,428348.000,02000020,9924,32768;"
    "VALID,1.667187222e-10,9.641617960e-10,-18.00000000000,2017,1,5,22,58,50000,VALID*2a066e78"
    );
    parse_result = GNSS_bestxyza_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 3);

    // Missing bestxyza data response
    strcpy(gnss_data,"#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;");
    parse_result = GNSS_bestxyza_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 4);

    // Missing delimiting character * which denotes the end of the data response
    strcpy(gnss_data, "#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;"
                    "SOL_COMPUTED,NARROW_INT,-1634531.5683,-3664618.0326,-3664618.0326,0.0099,"
                    "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                    "0.0230,\"AAAA\",0.250,1.000,0.000,12,11,11,11,0,01,0,33");
    parse_result = GNSS_bestxyza_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 5);

    // Data Response is larger than the buffer
    strcpy(gnss_data, "#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;"
                    "SOL_COMPUTED,NARROW_INT,-1634531.5683,-3664618.0326,-3664618.0326,0.0099,"
                    "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                    "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                    "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                    "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                    "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                    "0.0219,0.0115,0.11,"
                    "0.0230,\"AAAA\",0.250,1.000,0.000,12,11,11,11,0,01,0,33*2a066e78");
    parse_result = GNSS_bestxyza_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 6);

    // Error within the integer section of the string ie postion_x_mm has an invalid number
    strcpy(gnss_data, "#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;"
                    "SOL_COMPUTED,NARROW_INT,abcd123.456,-3664618.0326,-3664618.0326,0.0099,"
                    "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                    "0.0230,\"AAAA\",0.250,1.000,0.000,12,11,11,11,0,01,0,33*2a066e78");
    parse_result = GNSS_bestxyza_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 7);

    return 0;
}


uint8_t TEST_EXEC__GNSS_timea_data_parser(){
    char gnss_data[600] = "#TIMEA,COM1,0,86.5,FINESTEERING,1930,428348.000,02000020,9924,32768;VALID,"
                        "1.667187222e-10,9.641617960e-10,-18.00000000000,2017,1,5,22,58,50000,VALID*2a066e78";

    GNSS_timea_response_t result;

    // Call timea parser
    uint8_t parse_result = GNSS_timea_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 0);
    TEST_ASSERT_TRUE(result.clock_status == GNSS_CLOCK_VALID);
    TEST_ASSERT_TRUE(result.utc_offset == -18.00000000000);
    TEST_ASSERT_TRUE(result.utc_status == GNSS_UTC_VALID);
    TEST_ASSERT_TRUE(result.crc == 0x2a066e78);

    // Testing with an empty string
    strcpy(gnss_data, "");
    parse_result = GNSS_timea_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 1);

    // Introducing an error within the header of the GNSS response: Missing delimiting character
    strcpy(gnss_data, "#TIMEA,COM1,0,86.5,FINESTEERING,1930,428348.000,02000020,9924,32768,VALID,"
                    "1.667187222e-10,9.641617960e-10,-18.00000000000,2017,1,5,22,58,50000,VALID*2a066e78");
    parse_result = GNSS_timea_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 2);

    // Testing with a different GNSS Header
    strcpy(gnss_data,
    "#RAWEPHEMA,COM1,0,55.5,SATTIME,2072,133140.000,02000000,58ba,15761;32,2072,"
    "136800,8b00602b57a606100004389101eefa4e0eeed24e012f216600007608cd27,"
    "8b00602b58282f02373454d33b986d01bd01a76ba710a2a10d008e21667f,"
    "8b00602b58ae003384abe701001226ff6c6c1c9999f3c99fffa77c2f05c8*d3806ea3"
    );
    parse_result = GNSS_timea_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 3);

    // Missing timea data response after the header
    strcpy(gnss_data, "#TIMEA,COM1,0,86.5,FINESTEERING,1930,428348.000,02000020,9924,32768;");
    parse_result = GNSS_timea_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 4);

    // Missing delimiting character * ie no CRC
    strcpy(gnss_data, "#TIMEA,COM1,0,86.5,FINESTEERING,1930,428348.000,02000020,9924,32768;VALID,"
                    "1.667187222e-10,9.641617960e-10,-18.00000000000,2017,1,5,22,58,50000,VALID");
    parse_result = GNSS_timea_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 5);

    // Data Response Exceeding Buffer Size
    strcpy(gnss_data, "#TIMEA,COM1,0,86.5,FINESTEERING,1930,428348.000,02000020,9924,32768;VALID,"
                        "1.667187222e-10,9.641617960e-10,-18.00000000000,2017,1,5,22,58,50000,VALID,"
                        "1.667187222e-10,9.641617960e-10,-18.00000000000,2017,1,5,22,58,50000,VALID,"
                        "1.667187222e-10,9.641617960e-10,-18.00000000000,2017,1,5,22,58,50000,VALID,"
                        "1.667187222e-10,9.641617960e-10,-18.00000000000,2017,1,5,22,58,50000,VALID,"
                        "1.667187222e-10,9.641617960e-10,-18.00000000000,2017,1,5,22,58,50000,VALID,"
                        "1.667187222e-10,9.641617960e-10,-18.00000000000,2017,1,5,22,58,"
                        "1.667187222e-10,9.641617960e-10,-18.00000000000,2017,1,5,22,58,50000,VALID*d3806ea3");
    parse_result = GNSS_timea_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 6);

    // Error within the integer section of the string ie utc_offset has an invalid number
    strcpy(gnss_data, "#TIMEA,COM1,0,86.5,FINESTEERING,1930,428348.000,02000020,9924,32768;VALID,"
                    "1.667187222e-10,9.641617960e-10,usdhnd18.00000000000,2017,1,5,22,58,50000,VALID*2a066e78");
    parse_result = GNSS_timea_data_parser(gnss_data, &result);
    TEST_ASSERT_TRUE(parse_result == 7);
    

    return 0;
}