# CTS-SAT-1 Safety Features

Satellites are expensive and complex. They're also difficult to debug once in orbit, and are challenging to patch (modify) once on orbit.

This guide explains some of the safety features of CTS-SAT-1.

## Hardware Watchdogs

### OBC's STM32 Hardware Watchdog

The STM32's watchdog prevents latchups from firmware.

* See also: `STM32_pet_watchdog()`
* The watchdog must be petted at least once every 16 seconds, and no more than once every 200ms.
* We minimize the number of sites the STM32 is petted to prevents glitches in infinite loops.

### EPS Watchdog

* The EPS has a watchdog that must be petted every 5 minutes.
* Petted from a low-priority FreeRTOS task.

### AX100 Watchdog

* The AX100 has an inbuilt watchdog that resets if no uplink is received for a long time.
* No interaction with the OBC firmware.

## Software/Logical Watchdogs

These logical features are implemented in the OBC's firmware.

## OBC Uptime

Configuration variable: `STM32_system_reset_interval_sec`. If the system uptime exceeds this value, the system will reset (reboot).

* https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/282

## Duration Since Uplink

Configuration variable: `STM32_system_reset_no_uplink_interval_sec`. If the duration since an AX100 uplink telecommand exceeds this value, the system will reset (reboot).

* https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/547

## UHF Radio Communication

### Dipole Switch Reset

* Configuration Variable: `COMMS_max_duration_without_uplink_before_setting_default_rf_switch_mode_sec`
* If a short duration passes without an uplink, the dipole/antenna switch configuration is reset.
