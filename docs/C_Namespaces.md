# C Namespaces

Every non-local C function, enum, typedef, etc. in this codebase shall be prefixed with one of the following uppercase identifiers to indicate which subsystem/
region of code it belongs to.

## General Code

* `TCMD_`: related to the telecommand parsing system
* `TCMDEXEC_`: is a telecommand which can be executed
	* These telecommands will all be documented in the operations manual.
* `TASK_`: functions which are FreeRTOS threads/tasks
* `TASK_HELP_`: functions, etc. which are directly related to FreeRTOS threads, which represent logic factored out of individual threads
	* For example, a function which runs at the boot-up of every thread.
* `DEBUG_UART_`: related to debugging via UART, and debugging UART communications
* `DEBUG_I2C_`: related to I2C
* `LFS_`: related to LittleFS, the satellite's filesystem
	* Note that functions implemented in the LittleFS library will be prefixed with `lfs_` (lowercase).
	* The uppercase `LFS_` prefix indicates that the function is implemented by the CTS team.
* `UART_`: related to low-level UART drivers, written by the CTS team
* `TEST_`: unit testing and integration testing infrastructure
* `TEST_EXEC_`: unit test and integration test functions
* `GEN_`: general-purpose functions which don't fit into any other category
	* For example, byte manipulation functions.

## Satellite Subsystems

* `ADCS_`: duh
* `EPS_`: duh
* `GPS_`: duh
* `COMMS_ANT_`: related to the I2C communications for the deployable communications antenna
* `AX100_`: related to the AX100 communication module, at a low level
* `COMMS_`: related to higher-level functions for command and data handling, which calls into the `AX100_` prefix
	* Note: we may want to discard either `COMMS_` or `AX100_` as we begin implementation
* `BOOM_`: related to the burn wire driver for the boom subsystem
* `CAM_`: related to the UART camera
* `LORA_`: related to the UART LoRaWAN communications systems, if integrated
* `ENVIRO_`: related to monitoring the environment (e.g., the on-OBC temperature sensor)
* `FLASH_`: drive functions for the SPI flash system, which the `lfs_` implementation can call into
	* This may be removed.


## Libraries, External, etc.

* `HAL_`: STM32 HAL (Hardware Abstraction Layer)
* `lfs_`: LittleFS library functions
