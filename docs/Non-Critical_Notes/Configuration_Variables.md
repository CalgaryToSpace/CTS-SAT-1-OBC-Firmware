# Configuration Variables List

## Variables

### Integer Configuration

**1) `TASK_heartbeat_period_ms`**: The frequency, in ms, at which the `TASK_DEBUG_print_heartbeat` telecommand is logging messages.

**2) `CONFIG_freertos_min_remaining_stack_percent`**:  
 The percentage of the stack space that should remain free. If the free space falls below this percentage, a warning will be logged.

**3) `CONFIG_int_config_variables[]`**:  
 An array of objects, where each object contains:

- The variable name (const char \*)
- int value (uint32_t \*)

**4) `CONFIG_int_config_variables_count`**:  
 The number of entries in the `CONFIG_int_config_variables[]` array.

---

### String Configuration

**5) `CONFIG_str_config_variables[]`**:  
 An array of objects, where each object contains:

- The variable name (const char \*)
- char value (char \*)
- The maximum allowed string length (const uint8_t)

**6) `CONFIG_str_config_variables_count`**:  
 The number of entries in the `CONFIG_str_config_variables[]` array.

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

#
