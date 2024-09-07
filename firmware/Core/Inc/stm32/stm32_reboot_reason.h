
#ifndef INCLUDE_GUARD__STM32_REBOOT_REASON_H
#define INCLUDE_GUARD__STM32_REBOOT_REASON_H

/// @brief  Possible STM32 system reset causes
typedef enum STM_reset_cause_e
{
    STM_RESET_CAUSE_UNKNOWN = 0,
    STM_RESET_CAUSE_LOW_POWER_RESET,
    STM_RESET_CAUSE_WINDOW_WATCHDOG_RESET,
    STM_RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET,
    STM_RESET_CAUSE_SOFTWARE_RESET,
    STM_RESET_CAUSE_EXTERNAL_RESET_PIN_RESET,
    STM_RESET_CAUSE_BROWNOUT_RESET,
    STM_RESET_CAUSE_OPTION_BYTE_LOADER_RESET,
    STM_RESET_CAUSE_FIREWALL_RESET,
} STM_reset_cause_t;

extern char * STM_reset_cause_name;

STM_reset_cause_t STM_get_reset_cause(void);
char * STM_reset_cause_enum_to_str(STM_reset_cause_t reset_cause, char * previous_case);

#endif // INCLUDE_GUARD__STM32_REBOOT_REASON_H
