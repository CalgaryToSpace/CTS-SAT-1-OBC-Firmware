// Source: https://stackoverflow.com/questions/34196663/

#include "stm32/stm32_reboot_reason.h"

#include "main.h"
#include <stdio.h>
#include <string.h>

/// @brief      Obtain the STM32 system reset cause.
/// @param      None
/// @return     The system reset cause, as an enum
STM32_reset_cause_t STM32_get_reset_cause(void)
{
    // Note: static variables persist across calls to this function.
    static STM32_reset_cause_t reset_cause = STM32_RESET_CAUSE_UNKNOWN;
    static uint8_t STM32_has_reset_cause_been_obtained = 0;

    // Fetching the reset cause using the HAL Flag if-statements is only accurate once, so if it's
    // already loaded, then we just return the value from the first time this function was called.
    if (STM32_has_reset_cause_been_obtained)
    {
        return reset_cause;
    }

    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))
    {
        reset_cause = STM32_RESET_CAUSE_LOW_POWER_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
    {
        reset_cause = STM32_RESET_CAUSE_WINDOW_WATCHDOG_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        reset_cause = STM32_RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
    {
        // This reset is induced by calling the ARM CMSIS
        // `NVIC_SystemReset()` function!
        reset_cause = STM32_RESET_CAUSE_SOFTWARE_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST))
    {
        reset_cause = STM32_RESET_CAUSE_BROWNOUT_RESET; // POR/PDR or BOR reset
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
    {
        reset_cause = STM32_RESET_CAUSE_EXTERNAL_RESET_PIN_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST))
    {
        reset_cause = STM32_RESET_CAUSE_OPTION_BYTE_LOADER_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_FWRST))
    {
        reset_cause = STM32_RESET_CAUSE_FIREWALL_RESET;
    }
    else
    {
        reset_cause = STM32_RESET_CAUSE_UNKNOWN;
    }

    // NOTE: If the STM resets, the flags that were turned on from the previous reset do not automatically clear themselves.
    // The only time that a STM will clear it's own reset flags is when the system is physically powered on and off.
    // Therefore there is a need to call __HAL_RCC_CLEAR_RESET_FLAGS(); to clear the flag after gathering the information.
    __HAL_RCC_CLEAR_RESET_FLAGS();

    // Set the flag that indicates that the reset cause has been obtained for future calls to this function.
    STM32_has_reset_cause_been_obtained = 1;

    return reset_cause;
}

/// @brief      Convert a reset cause enum to a ASCII-printable name string.
/// @param      reset_cause A reset cause enum.
/// @return     A pointer to a statically-allocated null-terminated ASCII name string describing the system
///             reset cause
char *STM32_reset_cause_enum_to_str(STM32_reset_cause_t reset_cause)
{
    switch (reset_cause)
    {
    case STM32_RESET_CAUSE_UNKNOWN:
        return "UNKNOWN";
    case STM32_RESET_CAUSE_LOW_POWER_RESET:
        return "LOW_POWER_RESET";
    case STM32_RESET_CAUSE_WINDOW_WATCHDOG_RESET:
        return "WINDOW_WATCHDOG_RESET";
    case STM32_RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET:
        return "INDEPENDENT_WATCHDOG";
    case STM32_RESET_CAUSE_SOFTWARE_RESET:
        return "SOFTWARE_RESET";
    case STM32_RESET_CAUSE_EXTERNAL_RESET_PIN_RESET:
        return "EXTERNAL_RESET_PIN";
    case STM32_RESET_CAUSE_BROWNOUT_RESET:
        return "BROWNOUT_RESET";
    case STM32_RESET_CAUSE_OPTION_BYTE_LOADER_RESET:
        return "OPTION_BYTE_LOADER_RESET";
    case STM32_RESET_CAUSE_FIREWALL_RESET:
        return "FIREWALL_RESET";
    }

    // Default case.
    return "INVALID";
}
