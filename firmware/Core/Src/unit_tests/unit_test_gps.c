#include "unit_tests/unit_test_gps.h"
#include "gps/gps_types.h"

uint8_t TEST_EXEC__GPS_Pack_to_header(){
    char gps_data[] = "#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;"
                  "SOL_COMPUTED,NARROW_INT,-1634531.5683,-3664618.0326,4942496.3270,0.0099,"
                  "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                  "0.0230,\"AAAA\",0.250,1.000,0.000,12,11,11,11,0,01,0,33*e9eafeca";

    gps_response_header result;

    // Call header parser function

    TEST_ASSERT_TRUE(result.log_name == "BESTXYZA");
    TEST_ASSERT_TRUE(result.port == "COM1");
    TEST_ASSERT_TRUE(result.sequence_no == 0);
    TEST_ASSERT_TRUE(result.idle_time == 55.0);
    // TEST_ASSERT_TRUE(result.time_status == "FINESTEERING");
    TEST_ASSERT_TRUE(result.week == 1419);
    TEST_ASSERT_TRUE(result.seconds == 340033.000);
    TEST_ASSERT_TRUE(result.rx_status == 02000040);
    TEST_ASSERT_TRUE(result.reserved == 0xd821);
    TEST_ASSERT_TRUE(result.rx_sw_version == 2724);

    return 0;
};
