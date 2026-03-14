# Start-of-Pass Mission Operations

## Queries

At the start of a pass over the ground station, the following telecommands are most helpful to get an overall sense of what's happening in the satellite.

* Top Priority:
    * `core_system_stats`
    * `comms_get_rf_switch_info`
    * `eps_get_current_battery_percent`
* Second Priority:
    * `uart_get_last_rx_times_json`
    * `uart_get_errors_json`
    * ADCS commands TBD
* Third Priority:
    * Count errors in logs

## Actions

1. Recommended: Enable logging over the radio.
    * Assuming the default rebooted state, run `CTS1+log_set_sink_enabled_state(1,1)!`
