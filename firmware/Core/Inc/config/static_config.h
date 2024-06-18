
#ifndef __INCLUDE_GUARD__STATIC_CONFIG_H__
#define __INCLUDE_GUARD__STATIC_CONFIG_H__

/// Whether to enable critical-path debug success logging in the FLASH drivers.
/// Default: 0 (disabled)
#define FLASH_ENABLE_UART_DEBUG_PRINT 1

/// Whether to enable critical-path debug success logging in the LittleFS drivers.
/// Default: 0 (disabled)
#define LFS_ENABLE_UART_DEBUG_PRINT 1


// NOTE: All C files that use config variables from here should include a guard like the following:
// #ifndef LFS_ENABLE_UART_DEBUG_PRINT
//     #error "LFS_ENABLE_UART_DEBUG_PRINT not defined"
// #endif


#endif /* __INCLUDE_GUARD__STATIC_CONFIG_H__ */
