#include "adcs_drivers/adcs_commands.h"
#include "telecommands/telecommand_adcs.h"
#include "comms_drivers/comms_drivers.h"
#include "adcs_drivers/adcs_internal_drivers.h"
#include "adcs_drivers/adcs_types.h"
#include "main.h"
#include "start_of_mission/start_of_mission_procedure.h"
#include "eps_drivers/eps_calculations.h"
#include "eps_drivers/eps_struct_packers.h"
#include "eps_drivers/eps_types.h"

#include <stdint.h>

/*
 * Starts a 60 minute timer after deployment
 */
uint32_t START_deployment_timer(void)
{
    osDelay(1800000); // 30 minutes in milliseconds
    return 0;
}

/*
 * Starts beaconing through comms, once on each dipole switch
 */
uint32_t START_beaconing(void)
{
    // Switch to dipole antenna 1
    COMMS_set_dipole_switch_state(1);
    
    //COMMS_send_beacon(); // Beacon undefined as of now
    
    // Switch to dipole antenna 2
    COMMS_set_dipole_switch_state(2);

    //COMMS_send_beacon(); // Beacon undefined as of now
    
    return 0;
}

/*
 * Checks EPS battery level and waits until it reaches 70% if below that threshold
 * Returns 0 if battery level is >= 70%, 1 if battery level is >=30, 2 if error occurs
 */
uint32_t CHECK_battery_level(void) {
    EPS_battery_pack_datatype_eng_t battery;
    float battery_level;

    while (1) {
        battery_level = EPS_convert_battery_voltage_to_percent(battery);
        
        if (battery_level >= 70.0f) {
            return 0;
        }
        if(battery_level >= 30.0f) {
            return 1;
        }
        osDelay(60000); // Wait 1 minute before checking again
    }
}

/*
 * Attempts to deploy communications antenna
 * Returns 0 if successful, 1 if battery too low, 2 if deployment fails
 */
uint32_t DEPLOY_comms_antenna(void) {
    uint8_t max_attempts = 5;
    uint8_t attempt = 0;

    // Check if battery level above 70% to begin deployment
    if (CHECK_battery_level() != 0) {
        return 1;
    }

    while (attempt < max_attempts) {

        if (START_beaconing() == 0) {
            //Battery must be above 30% to continue deployment
            if (CHECK_battery_level() == 2) 
            {
                return 1;
            }
            // If antenna deployment successful, start beaconing
            if (START_beaconing() == 0) {
                return 0;
            }
        }

        attempt++;
        if (attempt < max_attempts) {
            osDelay(300000); // 5 minute delay between attempts
        }
    }
    
    return 2;
}

/*
 * Main start of mission sequence
 * Returns 0 if successful, non-zero if error occurs
 */
uint32_t START_mission_sequence(void) {
    // Wait for deployment timer
    if (START_deployment_timer() != 0) {
        return 1;
    }

    // Deploy antenna and start beaconing
    uint32_t deploy_result = DEPLOY_comms_antenna();
    if (deploy_result != 0) {
        return deploy_result;
    }

    // Check for tumbling
    uint32_t tumble_status = DETECT_tumble();
    return tumble_status;
}

/*
* Checks if satellite is in tumbling trajectory, returns 0 if not tumbling, 1 if tumbling, 2 if error occurs
*/
uint32_t DETECT_tumble()
{
    uint32_t tumble_detected = 0;
    float angular_velocity[3] = {0.0f, 0.0f, 0.0f};

    // Get angular velocity from ADCS sensors
    ADCS_angular_rates_struct_t vel_out;
    if (ADCS_get_estimate_angular_rates(&vel_out) == 0)
    {
        angular_velocity[0] = (float)vel_out.x_rate_mdeg_per_sec / 1000.0f;
        angular_velocity[1] = (float)vel_out.y_rate_mdeg_per_sec / 1000.0f;
        angular_velocity[2] = (float)vel_out.z_rate_mdeg_per_sec / 1000.0f;

        // Check if angular velocity exceeds threshold for tumbling
        const float MIN_THRESHOLD_X = -1.5f;
        const float MAX_THRESHOLD_X = 1.5f;
        const float MAX_THRESHOLD_Y = -1; // Might be either -1 or -2 with +- tolerance instead of range
        const float MIN_THRESHOLD_Y = -2;
        const float MIN_THRESHOLD_Z = -1.5f;
        const float MAX_THRESHOLD_Z = 1.5f;

        if ((angular_velocity[0] < MIN_THRESHOLD_X || angular_velocity[0] > MAX_THRESHOLD_X) || 
            (angular_velocity[1] < MIN_THRESHOLD_Y || angular_velocity[1] > MAX_THRESHOLD_Y) || 
            (angular_velocity[2] < MIN_THRESHOLD_Z || angular_velocity[2] > MAX_THRESHOLD_Z))
        {
            tumble_detected = 1;
        }
        else
        {
            tumble_detected = 0; // Not tumbling
        }
    }
    else
    {
        tumble_detected = 2; // Default if error occurs
    }
    return tumble_detected;
}

uint32_t DETUMBLE_procedure(void)
{
    uint8_t status;

    // STEP 1: Power on CubeControl Signal and Motor (Magnetometer and MEMS rate sensor)
    status = ADCS_set_power_control(
        ADCS_POWER_SELECT_ON,   // CubeControl Signal Power
        ADCS_POWER_SELECT_ON,   // CubeControl Motor Power
        ADCS_POWER_SELECT_OFF,  // CubeSense1
        ADCS_POWER_SELECT_OFF,  // CubeSense2
        ADCS_POWER_SELECT_OFF,  // CubeStar
        ADCS_POWER_SELECT_OFF,  // CubeWheel1
        ADCS_POWER_SELECT_OFF,  // CubeWheel2
        ADCS_POWER_SELECT_OFF,  // CubeWheel3
        ADCS_POWER_SELECT_OFF,  // Motor
        ADCS_POWER_SELECT_OFF   // GPS
    );
    if (status != 0) return 1;

    // STEP 2: Enable ADCS Run Mode
    status = ADCS_set_run_mode(1); // 1 = Enable
    if (status != 0) return 2;

    // STEP 3: Set estimation mode to Magnetometer rate filter (2)
    status = ADCS_attitude_estimation_mode(ADCS_ESTIMATION_MODE_MAGNETOMETER_RATE_FILTER);
    if (status != 0) return 3;

    // STEP 4: Set Attitude Control Mode to Detumbling (1), timeout 600s
    status = ADCS_attitude_control_mode(ADCS_CONTROL_MODE_DETUMBLING, 600);
    if (status != 0) return 4;

    // STEP 5: Set Attitude Control Mode to Y-Thomson (2), timeout 600s
    status = ADCS_attitude_control_mode(ADCS_CONTROL_MODE_Y_THOMSON_SPIN, 600);
    if (status != 0) return 5;

    // STEP 6: Set Attitude Control Mode to Y-Thomson (2), timeout 0s (continuous)
    status = ADCS_attitude_control_mode(ADCS_CONTROL_MODE_Y_THOMSON_SPIN, 0);
    if (status != 0) return 6;

    return 0;
}
