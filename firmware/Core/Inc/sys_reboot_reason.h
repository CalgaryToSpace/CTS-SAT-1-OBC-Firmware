/// @brief  Possible STM32 system reset causes
typedef enum reset_cause_e
{
    RESET_CAUSE_UNKNOWN = 0,
    RESET_CAUSE_LOW_POWER_RESET,
    RESET_CAUSE_WINDOW_WATCHDOG_RESET,
    RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET,
    RESET_CAUSE_SOFTWARE_RESET,
    RESET_CAUSE_EXTERNAL_RESET_PIN_RESET,
    RESET_CAUSE_BROWNOUT_RESET,
    RESET_CAUSE_OPTION_BYTE_LOADER_RESET,
    RESET_CAUSE_FIREWALL_RESET,
} reset_cause_t;

reset_cause_t reset_cause_get(void);
const char * reset_cause_get_name(reset_cause_t reset_cause);