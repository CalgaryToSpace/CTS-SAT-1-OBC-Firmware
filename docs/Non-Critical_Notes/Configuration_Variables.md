# Configuration Variables List

## 📚 **Table of Contents**

- [Configuration Variables](#configuration-variables)
- [Configuration Variable Arrays](#configuration-variable-arrays)
- [Helper Functions](#helper-functions)

---

## Configuration Variables

**1) `TASK_heartbeat_period_ms`**: The frequency, in ms, at which the `TASK_DEBUG_print_heartbeat` telecommand is logging messages.

**2) `CONFIG_freertos_min_remaining_stack_percent`**:  
 The percentage of the stack space that should remain free. If the free space falls below this percentage, a warning will be logged.

**3) `CONFIG_int_config_variables_count`**:  
 The number of integer configuration variables in the `CONFIG_int_config_variables[]` array.

**4) `CONFIG_str_config_variables_count`**:
 The number of string configuration variables in the `CONFIG_str_config_variables[]` array.

**5) `TCMD_require_unique_tssent`**:
A boolean value set to either 0 or 1. A value of 1 indicates that timestamps (usage of `@tssent`) are required, and a value of 0 indicates that timestamps are not required.

**6) `toggleable_channels`**:  
 32 bit number with each bit representing a channel (0-31). 1 indicates that the eps power managment can toggle the channel off and 0 indicating it can't.

**7) `watchdog_timer`**:  
 The time in ms in which the system will reset the watchdog timer.

**8) `EPS_monitor_timer`**:  
 The time in ms in which the system will check the channels connected to the EPS and toggle them off in case of errors. 

---

## Configuration Variable Arrays

**1) `CONFIG_int_config_variables[]`**:  
 An array of integer configuration variables, where each config var consists of:

- The variable name (const char \*)
- int value (uint32_t \*)

**2) `CONFIG_str_config_variables[]`**:  
 An array of string configuration variables, where each config var consists of:

- The variable name (const char \*)
- char value (char \*)
- The maximum allowed string length (const uint8_t)

---

## Helper Functions

### 1. **Get Index of a Variable**

```c
// Finds an int config variable in `CONFIG_int_config_variables` and returns its index.
int16_t CONFIG_get_int_var_index(const char *search_name)

// Finds a string config variable in `CONFIG_str_config_variables` and returns its index.
int16_t CONFIG_get_str_var_index(const char *search_name)
```

### 2. **Set the value of a Variable**

```c
// Assigns a new value to an integer configuration variable
uint8_t CONFIG_set_int_variable(const char *var_name, const uint64_t new_value)

// Assigns a new value to a string configuration variable
uint8_t CONFIG_set_str_variable(const char *var_name, const char *new_value)
```

### 3. **Convert value to JSON String**

```c
// Converts an integer configuration variable to a JSON string
uint16_t CONFIG_int_var_to_json(const char *var_name, char *json_str, const uint16_t json_str_max_len)

// Converts a string configuration variable to a JSON string
uint16_t CONFIG_str_var_to_json(const char *var_name, char *json_str, const uint16_t json_str_max_len)
```
