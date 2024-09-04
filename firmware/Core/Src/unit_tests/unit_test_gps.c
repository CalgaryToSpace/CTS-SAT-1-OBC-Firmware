#include "unit_tests/unit_test_gps.h"
#include "unit_tests/unit_test_helpers.h"
#include "transforms/number_comparisons.h"
#include "gps/gps.h"
#include "gps/gps_types.h"

#include <string.h>


uint8_t TEST_EXEC__GPS_assign_gps_time_status() {
    GPS_reference_time_status_t status;

    // Test with recognized status strings
    TEST_ASSERT_TRUE(assign_gps_time_status("UNKNOWN", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_UNKNOWN);
    TEST_ASSERT_TRUE(assign_gps_time_status("APPROXIMATE", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_APPROXIMATE);
    TEST_ASSERT_TRUE(assign_gps_time_status("COARSEADJUSTING", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_COARSEADJUSTING);
    TEST_ASSERT_TRUE(assign_gps_time_status("COARSE", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_COARSE);
    TEST_ASSERT_TRUE(assign_gps_time_status("COARSESTEERING", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_COARSESTEERING);
    TEST_ASSERT_TRUE(assign_gps_time_status("FREEWHEELING", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_FREEWHEELING);
    TEST_ASSERT_TRUE(assign_gps_time_status("FINEADJUSTING", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_FINEADJUSTING);
    TEST_ASSERT_TRUE(assign_gps_time_status("FINE", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_FINE);
    TEST_ASSERT_TRUE(assign_gps_time_status("FINEBACKUPSTEERING", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_FINEBACKUPSTEERING);
    TEST_ASSERT_TRUE(assign_gps_time_status("FINESTEERING", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_FINESTEERING);
    TEST_ASSERT_TRUE(assign_gps_time_status("SATTIME", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_SATTIME);

    // Test with an unrecognized status string
    TEST_ASSERT_TRUE(assign_gps_time_status("INVALID_STATUS", &status) == 1);

    // Test with an empty string
    TEST_ASSERT_TRUE(assign_gps_time_status("", &status) == 1);

    // Test with a NULL string
    TEST_ASSERT_TRUE(assign_gps_time_status(NULL, &status) == 1);

    return 0;

}

uint8_t TEST_EXEC__GPS_assign_gps_solution_status() {
    GPS_solution_status_enum_t status;

    // Test with recognized status strings
    TEST_ASSERT_TRUE(assign_gps_solution_status("SOL_COMPUTED", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_SOL_COMPUTED);
    TEST_ASSERT_TRUE(assign_gps_solution_status("INSUFFICIENT_OBS", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_INSUFFICIENT_OBS);
    TEST_ASSERT_TRUE(assign_gps_solution_status("NO_CONVERGENCE", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_NO_CONVERGENCE);
    TEST_ASSERT_TRUE(assign_gps_solution_status("SINGULARITY", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_SINGULARITY);
    TEST_ASSERT_TRUE(assign_gps_solution_status("COV_TRACE", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_COV_TRACE);
    TEST_ASSERT_TRUE(assign_gps_solution_status("TEST_DIST", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_TEST_DIST);
    TEST_ASSERT_TRUE(assign_gps_solution_status("COLD_START", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_COLD_START);
    TEST_ASSERT_TRUE(assign_gps_solution_status("V_H_LIMIT", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_V_H_LIMIT);
    TEST_ASSERT_TRUE(assign_gps_solution_status("VARIANCE", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_VARIANCE);
    TEST_ASSERT_TRUE(assign_gps_solution_status("RESIDUALS", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_RESIDUALS);
    TEST_ASSERT_TRUE(assign_gps_solution_status("INTEGRITY_WARNING", &status) == 0);
    TEST_ASSERT_TRUE(status == GPS_INTEGRITY_WARNING);

    // Test with an unrecognized status string
    TEST_ASSERT_TRUE(assign_gps_solution_status("UNKNOWN_STATUS", &status) == 1);

    // Test with an empty string
    TEST_ASSERT_TRUE(assign_gps_solution_status("", &status) == 1);

    // Test with a NULL string
    TEST_ASSERT_TRUE(assign_gps_solution_status(NULL, &status) == 1);

    return 0;
}


uint8_t TEST_EXEC__GPS_position_velocity_type() {
    GPS_position_type_enum_t type;

    // Test with recognized type strings
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("NONE", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_NONE);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("FIXEDPOS", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_FIXEDPOS);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("FIXEDHEIGHT", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_FIXEDHEIGHT);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("DOPPLER_VELOCITY", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_DOPPLER_VELOCITY);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("SINGLE", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_SINGLE);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("PSDIFF", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_PSDIFF);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("WAAS", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_WAAS);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("PROPAGATED", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_PROPAGATED);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("L1_FLOAT", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_L1_FLOAT);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("NARROW_FLOAT", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_NARROW_FLOAT);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("L1_INT", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_L1_INT);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("WIDE_INT", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_WIDE_INT);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("NARROW_INT", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_NARROW_INT);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("RTK_DIRECT_INS", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_RTK_DIRECT_INS);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("INS_SBAS", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_INS_SBAS);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("INS_PSRSP", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_INS_PSRSP);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("INS_PSRDIFF", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_INS_PSRDIFF);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("INS_RTKFLOAT", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_INS_RTKFLOAT);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("INS_RTKFIXED", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_INS_RTKFIXED);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("PPP_CONVERGING", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_PPP_CONVERGING);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("PPP", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_PPP);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("OPERATIONAL", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_OPERATIONAL);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("WARNING", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_WARNING);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("OUT_OF_BOUNDS", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_OUT_OF_BOUNDS);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("INS_PPP_CONVERGING", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_INS_PPP_CONVERGING);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("INS_PPP", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_INS_PPP);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("PPP_BASIC_CONVERGING", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_PPP_BASIC_CONVERGING);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("PPP_BASIC", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_PPP_BASIC);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("INS_PPP_BASIC_CONVERGING", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_INS_PPP_BASIC_CONVERGING);
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("INS_PPP_BASIC", &type) == 0);
    TEST_ASSERT_TRUE(type == GPS_TYPE_INS_PPP_BASIC);

    // Test with an unrecognized type string
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("INVALID_TYPE", &type) == 1);

    // Test with an empty string
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type("", &type) == 1);

    // Test with a NULL string
    TEST_ASSERT_TRUE(assign_gps_position_velocity_type(NULL, &type) == 1);

    return 0;
}

uint8_t TEST_EXEC__GPS_Parse_header(){
    // TODO: Try testing with another header string
    
    // Testing with a Valid GPS Header
    char gps_data[512] = "#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;"
                  "SOL_COMPUTED,NARROW_INT,-1634531.5683,-3664618.0326,4942496.3270,0.0099,"
                  "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                  "0.0230,\"AAAA\",0.250,1.000,0.000,12,11,11,11,0,01,0,33*e9eafeca";

    gps_response_header gps_header_result;

    uint8_t result = parse_gps_header(gps_data, &gps_header_result);
    TEST_ASSERT_TRUE(result == 0);
    TEST_ASSERT_TRUE(strcmp(gps_header_result.log_name, "BESTXYZA") == 0);
    TEST_ASSERT_TRUE(gps_header_result.time_status == GPS_FINESTEERING);

    // Testing with a different Valid GPS Header
    strcpy(gps_data,
    "#TIMEA,COM1,0,86.5,FINESTEERING,1930,428348.000,02000020,9924,32768;VALID,1.667187222e-10,9.641617960e-10,-18.00000000000,2017,1,5,22,58,50000,VALID*2a066e78")
    ;
    result = parse_gps_header(gps_data, &gps_header_result);
    TEST_ASSERT_TRUE(result == 0);

    // Testing an empty string
    strcpy(gps_data,"");
    result = parse_gps_header(gps_data, &gps_header_result);
    TEST_ASSERT_TRUE(result == 1);

    // Testing Missing Sync Character
    strcpy(gps_data,"BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;");
    result = parse_gps_header(gps_data, &gps_header_result);
    TEST_ASSERT_TRUE(result == 2);

    // Testing missing Terminating character
    strcpy(gps_data,"#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724");
    result = parse_gps_header(gps_data, &gps_header_result);
    TEST_ASSERT_TRUE(result == 2);

    // Testing missing both Sync and Terminating Character
    strcpy(gps_data,"BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724");
    result = parse_gps_header(gps_data, &gps_header_result);
    TEST_ASSERT_TRUE(result == 2);

    // Testing missing both Sync and Terminating Character
    strcpy(gps_data,"BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724");
    result = parse_gps_header(gps_data, &gps_header_result);
    TEST_ASSERT_TRUE(result == 2);

    // Testing when the ; charcacter comes before the # character
    strcpy(gps_data,
                  "02000040,d821,2724;SOL_COMPUTED,NARROW_INT,-1634531.5683,-3664618.0326,4942496.3270,"
                  "0.0099,0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                  "0.0230,\"AAAA\",0.250,1.000,0.000,12,11,11,11,0,01,0,33*e9eafeca"
                  "#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;" );
    result = parse_gps_header(gps_data, &gps_header_result);
    TEST_ASSERT_TRUE(result == 3);

    // Testing when the header buffer is exceeded
    strcpy(gps_data,"#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724"
                    "BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724"
                    "BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724"
                    "BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;");
    result = parse_gps_header(gps_data, &gps_header_result);
    TEST_ASSERT_TRUE(result == 4);

    return 0;
}

uint8_t TEST_EXEC__GPS_Parse_bestxyza(){
    char gps_data[512] = "#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;"
                  "SOL_COMPUTED,NARROW_INT,-1634531.5683,-3664618.0326,-3664618.0326,0.0099,"
                  "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                  "0.0230,\"AAAA\",0.250,1.000,0.000,12,11,11,11,0,01,0,33*e9eafeca";

    gps_bestxyza_response result;

    // Call bestxyza parser
    uint8_t parse_result = parse_bestxyza_data(gps_data, &result);
    TEST_ASSERT_TRUE(parse_result == 0);
    TEST_ASSERT_TRUE(result.position_solution_status == GPS_SOL_COMPUTED);
    TEST_ASSERT_TRUE(result.position_type == GPS_TYPE_NARROW_INT);
    TEST_ASSERT_TRUE(result.position_x_mm == -1634531568);
    TEST_ASSERT_TRUE(result.position_y_mm == -3664618032);
    TEST_ASSERT_TRUE(result.position_z_mm == -3664618032);
    TEST_ASSERT_TRUE(result.position_x_std_mm == 9);
    TEST_ASSERT_TRUE(result.position_y_std_mm == 21);
    TEST_ASSERT_TRUE(result.position_z_std_mm == 11);
    TEST_ASSERT_TRUE(result.differential_age_ms == 1000);
    TEST_ASSERT_TRUE(result.solution_age_ms == 0000);
    TEST_ASSERT_TRUE(result.crc == 0xe9eafeca);

    return 0;
}


uint8_t TEST_EXEC__GPS_Parse_timea(){
    char gps_data[512] = "#TIMEA,COM1,0,86.5,FINESTEERING,1930,428348.000,02000020,9924,32768;VALID,1.667187222e-10,9.641617960e-10,-18.00000000000,2017,1,5,22,58,50000,VALID*2a066e78";

    gps_timea_response result;

    // Call timea parser
    uint8_t parse_result = parse_timea_data(gps_data, &result);
    TEST_ASSERT_TRUE(parse_result == 0);
    TEST_ASSERT_TRUE(result.clock_status == GPS_CLOCK_VALID);

    TEST_ASSERT_TRUE(result.utc_offset == -18.00000000000);
    TEST_ASSERT_TRUE(result.utc_status == GPS_UTC_VALID);
    TEST_ASSERT_TRUE(result.crc == 0x2a066e78);

    return 0;
}