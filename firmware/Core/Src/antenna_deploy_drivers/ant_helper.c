#include "main.h"

#include "antenna_deploy_drivers/ant_helper.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/// @brief  @brief Convert a byte array to a hex string
/// @param dest destination to write value to
/// @param dest_len length of destination
/// @param values value to convert to hex
/// @param val_len length of value
/// @return true if successful, false if not
bool to_hex(char* dest, size_t dest_len, const uint8_t* values, size_t val_len) {
    if(dest_len < (val_len*2+1)) /* check that dest is large enough */
        return false;
    *dest = '\0'; /* in case val_len==0 */
    while(val_len--) {
        /* sprintf directly to where dest points */
        sprintf(dest, "%02X", *values);
        dest += 2;
        ++values;
    }
    return true;
}