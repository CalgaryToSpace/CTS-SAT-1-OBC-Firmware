#include "adcs_drivers/adcs_commands.h"
#include "telecommands/telecommand_adcs.h"
#include "comms_drivers/comms_drivers.h"
#include "adcs_drivers/adcs_internal_drivers.h"
#include "adcs_drivers/adcs_types.h"
#include "main.h"

#include <stdint.h>

/*
 * Starts a 60 minute timer after deployment
 */
uint32_t START_deployment_timer(void)
{
    HAL_Delay(3600000); // 60 minutes in milliseconds
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
* Checks if satellite is in tumbling trajectory, returns 0 if not tumbling, 1 if tumbling, 2 if error occurs
*/
uint32_t DETECT_tumble()
{
    uint8_t tumble_detected = 0;
    float angular_velocity[3] = {0.0f, 0.0f, 0.0f};

    // Get angular velocity from ADCS sensors
    ADCS_angular_rates_struct_t vel_out;
    if (ADCS_get_estimate_angular_rates(&vel_out) == 0)
    {
        angular_velocity[0] = (float)vel_out.x_rate_mdeg_per_sec / 1000.0f;
        angular_velocity[1] = (float)vel_out.y_rate_mdeg_per_sec / 1000.0f;
        angular_velocity[2] = (float)vel_out.z_rate_mdeg_per_sec / 1000.0f;

        // Check if angular velocity exceeds threshold for tumbling
        float MIN_THRESHOLD_X = -0.2f;
        float MAX_THRESHOLD_X = 0.2f;
        float MAX_THRESHOLD_Y = -1; // Might be either -1 or -2 with +- tolerance instead of range
        float MIN_THRESHOLD_Y = -2;
        float MIN_THRESHOLD_Z = -0.2f;
        float MAX_THRESHOLD_Z = 0.2f;

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
        tumble_detected = 2; // Default to not tumbling if error occurs
    }
    return tumble_detected;
}
