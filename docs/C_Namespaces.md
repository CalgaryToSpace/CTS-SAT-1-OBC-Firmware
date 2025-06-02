# C Namespaces

Every non-local C function, enum, typedef, etc. in this codebase shall be prefixed with one of the
following uppercase identifiers to indicate which subsystem/region of code it belongs to.

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
	* Note that functions implemented in the LittleFS library are prefixed with `lfs_` (lowercase).
	* The uppercase `LFS_` prefix indicates that the function is implemented by the CTS team.
* `UART_`: related to low-level UART drivers, written by the CTS team
* `TEST_`: unit testing and integration testing infrastructure
* `TEST_EXEC_`: unit test and integration test functions
* `GEN_`: general-purpose functions which don't fit into any other category
	* For example, byte manipulation functions.
* `FREERTOS_`: related to FreeRTOS tasks/threads/metadata

## Satellite Subsystems

* `ADCS_`: Attitude Determination and Control System
* `EPS_`: related to the Electrical Power System
* `EPS_CMD_`: functions which trigger a command to the EPS subsystem
* `GNSS_`: GNSS/GPS receiver
* `COMMS_ANT_`: related to the I2C communications for the deployable communications antenna
* `AX100_`: related to the AX100 communication module, at a low level
* `COMMS_`: related to higher-level functions for command and data handling, which calls into the `AX100_` prefix
* `BOOM_`: related to the burn wire driver for the deployable composite latice boom (DCLB) subsystem
* `CAM_`: related to the UART camera
* `LORA_`: related to the UART LoRaWAN communications systems, if integrated
* `ENVIRO_`: related to monitoring the environment (e.g., the on-OBC temperature sensor)
* `FLASH_`: driver functions for the SPI flash system, which the `lfs_` implementation can call into
* `STM32_`: driver functions for features within the STM32, including internal flash, boot metadata, etc.
* `CTS1_`: related to CTS-SAT-1/FrontierSat as a whole

## Other Categories
* `TIME_`: related to the timer peripheral, or time-keeping in general

## Libraries, External, etc.

* `HAL_`: STM32 HAL (Hardware Abstraction Layer)
* `lfs_`: LittleFS library functions

## Other Conventions

* `_CFG_`: Variables can contain `_CFG_` to indicate that they are configuration variables of 
another subsystem/namespace. For example, `EPS_CFG_` denotes an EPS subsystem configuration variable.
* `_CMD_`: Functions can contain `_CMD_` to indicate that they trigger a command to that subsystem.
For example, `EPS_CMD_` denotes a function which triggers a command to the EPS subsystem (and gets the EPS's response).
