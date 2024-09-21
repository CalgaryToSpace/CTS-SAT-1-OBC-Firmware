#include "unit_tests/unit_test_helpers.h"
#include "unit_tests/test_assorted_prototypes.h"
#include<antenna_deploy_drivers/ant_commands.h>

// derivations for the unit tests can be found at
//https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/146#issuecomment-2362673403
uint8_t TEST_EXEC__ANT_convert_raw_temp_to_cCelsius() {
    int16_t result = ANT_convert_raw_temp_to_cCelsius(647);
    
    TEST_ASSERT((100 < result) && (result  < 200) );

    result = ANT_convert_raw_temp_to_cCelsius(396);
    TEST_ASSERT((7500 < result) && (result < 7600));

    result = ANT_convert_raw_temp_to_cCelsius(797);
    TEST_ASSERT((-4480 < result) && (result < 4520));
    return 0;

    uint8_t raw_bytes[2] = {0x3D, 0x02};
    uint16_t measurment = (raw_bytes[1] << 8) | raw_bytes[0];
    int deg_c = ANT_convert_raw_temp_to_cCelsius(measurment);
    TEST_ASSERT((deg_c > 2300) && (deg_c < 2400));
}