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


uint8_t TEST_EXEC__GPS_Pack_to_bestxyza(){
    char gps_data[] = "#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724;"
                  "SOL_COMPUTED,NARROW_INT,-1634531.5683,-3664618.0326,-3664618.0326,0.0099,"
                  "0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,"
                  "0.0230,\"AAAA\",0.250,1.000,0.000,12,11,11,11,0,01,0,33*e9eafeca";

    bestxyza_response result;

    // Call bestxyza parser

    TEST_ASSERT_TRUE(result.position_solution_status == "SOL_COMPUTED");
    TEST_ASSERT_TRUE(result.position_type == "NARROW_INT");

    TEST_ASSERT_TRUE(result.position_x_m == -1634531.5683);
    TEST_ASSERT_TRUE(result.position_y_m == -3664618.0326);
    TEST_ASSERT_TRUE(result.position_z_m == -3664618.0326);

    TEST_ASSERT_TRUE(result.position_x_std_m == 0.0099);
    TEST_ASSERT_TRUE(result.position_y_std_m == 0.0219);
    TEST_ASSERT_TRUE(result.position_z_std_m == 0.0115);

    TEST_ASSERT_TRUE(result.velocity_solution_status == "SOL_COMPUTED");
    TEST_ASSERT_TRUE(result.velocity_type == "NARROW_INT");

    TEST_ASSERT_TRUE(result.velocity_x_m_per_s == 0.0011);
    TEST_ASSERT_TRUE(result.velocity_y_m_per_s == -0.0049);
    TEST_ASSERT_TRUE(result.velocity_z_m_per_s == -0.0001);

    TEST_ASSERT_TRUE(result.velocity_x_std_m_per_s == 0.0199);
    TEST_ASSERT_TRUE(result.velocity_y_std_m_per_s == 0.0439);
    TEST_ASSERT_TRUE(result.velocity_z_std_m_per_s == 0.0230);

    TEST_ASSERT_TRUE(result.stn_id == '"AAAA"');
    TEST_ASSERT_TRUE(result.velocity_latency == 0.250);
    TEST_ASSERT_TRUE(result.differential_age_sec == 1.000);
    TEST_ASSERT_TRUE(result.solution_age_sec == 0.000);
    TEST_ASSERT_TRUE(result.satellite_no_tracked == 12);

    TEST_ASSERT_TRUE(result.satellite_no_used_in_solution == 11);
    TEST_ASSERT_TRUE(result.satellite_no_l1_e1_b1 == 11);
    TEST_ASSERT_TRUE(result.satellite_no_sol_multifreq == 11);
    TEST_ASSERT_TRUE(result.reserved == 0);

    // Need to confirm the representation of these values
    TEST_ASSERT_TRUE(result.extended_solution_status == 01);
    TEST_ASSERT_TRUE(result.galileo_beiDou_sig_mask == 0);
    TEST_ASSERT_TRUE(result.gps_glonass_sig_mask == 0x33);
    TEST_ASSERT_TRUE(result.crc == 0xe9eafeca);

    return 0;
    
}
