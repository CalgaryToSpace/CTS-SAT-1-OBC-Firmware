// Source: https://stackoverflow.com/questions/34196663/

#include "sys_reboot_reason.h"

#include "main.h"
#include <stdio.h>
#include <string.h>

char * STM_reset_cause_name = "UNKNOWN";

/// @brief      Obtain the STM32 system reset cause
/// @param      None
/// @return     The system reset cause
STM_reset_cause_t STM_get_reset_cause(void)
{
    STM_reset_cause_t reset_cause;

    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))
    {
        reset_cause = STM_RESET_CAUSE_LOW_POWER_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
    {
        reset_cause = STM_RESET_CAUSE_WINDOW_WATCHDOG_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        reset_cause = STM_RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
    {
        // This reset is induced by calling the ARM CMSIS 
        // `NVIC_SystemReset()` function!
        reset_cause = STM_RESET_CAUSE_SOFTWARE_RESET; 
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST))
    {
        reset_cause = STM_RESET_CAUSE_BROWNOUT_RESET; // POR/PDR or BOR reset
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
    {
        reset_cause = STM_RESET_CAUSE_EXTERNAL_RESET_PIN_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST))
    {
        reset_cause = STM_RESET_CAUSE_OPTION_BYTE_LOADER_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_FWRST))
    {
        reset_cause = STM_RESET_CAUSE_FIREWALL_RESET;
    }
    else
    {
        reset_cause = STM_RESET_CAUSE_UNKNOWN;
    }

	// NOTE*: If the STM resets, the flags that were turned on from the previous reset do not automatically clear themselves. The only time that a STM will clear it's own reset flags
	// is when the system is physically powered on and off. Therefore there is a need to call __HAL_RCC_CLEAR_RESET_FLAGS(); to clear the flag after gathering the information.
	__HAL_RCC_CLEAR_RESET_FLAGS();


    return reset_cause; 
}

/// @brief      Obtain the system reset cause as an ASCII-printable name string 
///             from a reset cause type
/// @param      reset_cause     The previously-obtained system reset cause flag
/// @param      previous_cause  The reset flag from calling the function TCMDEXEC_core_system_stats
/// @return     A null-terminated ASCII name string describing the system 
///             reset cause
char * STM_reset_cause_enum_to_str(STM_reset_cause_t reset_cause, char * previous_cause)
{
	char * reset_cause_name = "TBD";

	// previous_cause is utilized to keep track of the reset flag so that you can clear the flags and the information will remain for system core stats
	// if statement checks if core system stats is being called again and returns the previous case since the reset flags have already been reset
    if (strcmp(previous_cause,"UNKOWN") == 0 || reset_cause!=STM_RESET_CAUSE_UNKNOWN){
        switch (reset_cause)
    	{
            case STM_RESET_CAUSE_UNKNOWN:
                reset_cause_name = "UNKNOWN";
                break;
            case STM_RESET_CAUSE_LOW_POWER_RESET:
                reset_cause_name = "LOW_POWER_RESET";
                break;
            case STM_RESET_CAUSE_WINDOW_WATCHDOG_RESET:
                reset_cause_name = "WINDOW_WATCHDOG_RESET";
                break;
            case STM_RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET:
                reset_cause_name = "INDEPENDENT_WATCHDOG";
                break;
            case STM_RESET_CAUSE_SOFTWARE_RESET:
                reset_cause_name = "SOFTWARE_RESET";
                break;
            case STM_RESET_CAUSE_EXTERNAL_RESET_PIN_RESET:
                reset_cause_name = "EXTERNAL_RESET_PIN";
                break;
            case STM_RESET_CAUSE_BROWNOUT_RESET:
                reset_cause_name = "POWER_CYCLE_OR_BROWNOUT_RESET";
                break;
            case STM_RESET_CAUSE_OPTION_BYTE_LOADER_RESET:
                reset_cause_name = "OPTION_BYTE_LOADER_RESET";
                break;
            case STM_RESET_CAUSE_FIREWALL_RESET:
                reset_cause_name = "FIREWALL_RESET";
                break;
        }
    }
    else 
    {
        return previous_cause;
    }
	return reset_cause_name;
}