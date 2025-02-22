#include "unit_tests/test_eps_calculations.h"
#include "unit_tests/unit_test_helpers.h"

#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_calculations.h"


uint8_t TEST_EXEC__EPS_convert_battery_voltage_to_percent()
{
    EPS_battery_pack_datatype_eng_t battery;
    float calc = 0;
    float expected_value = 0;

    // The max tolerance of the floating point calculation
    // should be +/- 0.1 percent of the actual calculation
    const float TOLERANCE = 0.1;

    
    // Success Cases
    // Case 1: Minimum voltage (12V -> 0%)
    battery.cell_voltage_each_cell_mV[0] = 3000;
    battery.cell_voltage_each_cell_mV[1] = 3000;
    battery.cell_voltage_each_cell_mV[2] = 3000;
    battery.cell_voltage_each_cell_mV[3] = 3000;

    expected_value = 0;
    
    calc = EPS_convert_battery_voltage_to_percent(battery);
    TEST_ASSERT(calc >= (expected_value - TOLERANCE));
    TEST_ASSERT(calc <= (expected_value + TOLERANCE));

    // Case 2: Maximum voltage (16V -> 100%)
    battery.cell_voltage_each_cell_mV[0] = 4000;
    battery.cell_voltage_each_cell_mV[1] = 4000;
    battery.cell_voltage_each_cell_mV[2] = 4000;
    battery.cell_voltage_each_cell_mV[3] = 4000;

    expected_value = 100;
    
    calc = EPS_convert_battery_voltage_to_percent(battery);
    TEST_ASSERT(calc >= (expected_value - TOLERANCE));
    TEST_ASSERT(calc <= (expected_value + TOLERANCE));


    // Case 3: Midpoint (14V -> 50%)
    battery.cell_voltage_each_cell_mV[0] = 3500;
    battery.cell_voltage_each_cell_mV[1] = 3500;
    battery.cell_voltage_each_cell_mV[2] = 3500;
    battery.cell_voltage_each_cell_mV[3] = 3500;

    expected_value = 50;
    
    calc = EPS_convert_battery_voltage_to_percent(battery);
    TEST_ASSERT(calc >= (expected_value - TOLERANCE));
    TEST_ASSERT(calc <= (expected_value + TOLERANCE));


    // Case 4: Over 16V (Should be greater than 100%)
    battery.cell_voltage_each_cell_mV[0] = 4500;
    battery.cell_voltage_each_cell_mV[1] = 4500;
    battery.cell_voltage_each_cell_mV[2] = 4500;
    battery.cell_voltage_each_cell_mV[3] = 4500;

    // 16 - 12 = 4V
    // total voltage above = 18 V
    // 18 - 16 = 2
    // 2 / 4 = .5 = 50%, 50% above maximum total voltage
    expected_value = 150;
    
    calc = EPS_convert_battery_voltage_to_percent(battery);
    TEST_ASSERT(calc >= (expected_value - TOLERANCE));
    TEST_ASSERT(calc <= (expected_value + TOLERANCE));

    // Case 5: Under 12V (Should be negative)
    battery.cell_voltage_each_cell_mV[0] = 2500;
    battery.cell_voltage_each_cell_mV[1] = 2500;
    battery.cell_voltage_each_cell_mV[2] = 2500;
    battery.cell_voltage_each_cell_mV[3] = 2500;
    
    // 16 - 12 = 4 V
    // total voltage above is 1000 mV = 1 V
    // 10 - 12 = -2
    // -2 / 4 = -0.5 = -50%, 50% less than minimum
    expected_value = -50;
    
    calc = EPS_convert_battery_voltage_to_percent(battery);
    TEST_ASSERT(calc >= (expected_value - TOLERANCE));
    TEST_ASSERT(calc <= (expected_value + TOLERANCE));
    
    return 0;
}