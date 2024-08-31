#include "unit_tests/unit_test_gps.h"
#include "unit_tests/unit_test_helpers.h"
#include "transforms/number_comparisons.h"
#include "gps/gps.h"
#include "gps/gps_types.h"

#include <string.h>

uint8_t TEST_EXEC__GPS_Parse_header(){

    // TODO: Try testing with another header string
    
    char gps_data[280] = "#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;"
                  "SOL_COMPUTED,NARROW_INT,-1634531.5683,-3664618.0326,4942496.3270,0.0099,"
                  "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                  "0.0230,\"AAAA\",0.250,1.000,0.000,12,11,11,11,0,01,0,33*e9eafeca";

    gps_response_header gps_header_result;

    // Call header parser function
    uint8_t result = parse_gps_header(gps_data, &gps_header_result);
    TEST_ASSERT_TRUE(result == 0);
    TEST_ASSERT_TRUE(strcmp(gps_header_result.log_name, "BESTXYZA") == 0);
    TEST_ASSERT_TRUE(gps_header_result.time_status == GPS_FINESTEERING);

    return 0;
}

uint8_t TEST_EXEC__GPS_Parse_bestxyza(){
    char gps_data[280] = "#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;"
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
    char gps_data[280] = "#TIMEA,COM1,0,86.5,FINESTEERING,1930,428348.000,02000020,9924,32768;VALID,1.667187222e-10,9.641617960e-10,-18.00000000000,2017,1,5,22,58,50000,VALID*2a066e78";

    gps_timea_response result;

    // Call timea parser
    uint8_t parse_result = parse_timea_data(gps_data, &result);
    // TEST_ASSERT_TRUE(parse_result == 0);
    TEST_ASSERT_TRUE(result.clock_status == GPS_CLOCK_VALID);

    TEST_ASSERT_TRUE(result.utc_offset == -18.00000000000);
    TEST_ASSERT_TRUE(result.utc_status == GPS_UTC_VALID);
    // TEST_ASSERT_TRUE(result.crc == 0x2a066e78);

    return 0;
}