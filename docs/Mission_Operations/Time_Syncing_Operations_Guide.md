# Time Syncing Operations Guide

## Syncing Satellite Time (from any source)

### Procedure

1. Temporarily disable the "set OBC time based on EPS" periodic operation with `CTS1+config_set_int_var(EPS_time_sync_period_sec,0)!`.
2. Update the OBC's time based on the GNSS or Ground Station time.
    1. Enable the GNSS EPS channel if using the GNSS for the time sync.
    2. Run the main time sync command (e.g., `set_obc_time_based_on_gnss_time`, `correct_system_time`, `set_system_time`).
    3. Optionally, run the `set_obc_time_based_on_gnss_pps` to bump the OBC time based on the GNSS PPS feature (makes small adjustments of <=500ms).
    4. Optionally, set the EPS time based on the OBC time (`set_eps_time_based_on_obc_time`).
3. Restore the default configuration: `CTS1+config_set_int_var(EPS_time_sync_period_sec,600)!` (where 600 sec = 10 minutes).
