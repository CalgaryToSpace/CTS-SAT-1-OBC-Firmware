#include "main.h"

#include "littlefs/flash_driver.h"
#include "debug_tools/debug_uart.h"

void demo_flash_write() {
    DEBUG_uart_print_str("Starting demo_flash_write()...\n");

    uint8_t chip_num = 0;
    uint32_t flash_addr = 0;
    uint8_t bytes_to_write[100];
    strcpy((char*)bytes_to_write, "1234567890abcdef_XYZ");
    uint32_t num_bytes = strlen((char*)bytes_to_write);

    for (uint8_t i = 0; i < 2; i++) {
        FLASH_error_enum_t result = FLASH_write_data(&hspi1, chip_num, flash_addr, bytes_to_write, num_bytes);
        if (result != 0) {
            DEBUG_uart_print_str("Error in FLASH_write\n");
            return;
        }
    }

    DEBUG_uart_print_str("Wrote bytes in demo_flash_write(). Done.\n");
}


void demo_flash_read() {
    DEBUG_uart_print_str("Starting demo_flash_read()...\n");

    uint8_t chip_num = 0;
    uint32_t flash_addr = 0;
    uint32_t num_bytes = 98;

    uint8_t bytes_store[100];

    FLASH_error_enum_t result = FLASH_read_data(&hspi1, chip_num, flash_addr, bytes_store, num_bytes);

    if (result != 0) {
        DEBUG_uart_print_str("Error in FLASH_read_data\n");
        return;
    }

    DEBUG_uart_print_str("Read bytes: \n");
    DEBUG_uart_print_array_hex(bytes_store, num_bytes);

    DEBUG_uart_print_str("\nRead bytes in demo_flash_read(). Done.\n");
}

// void demo_flash_full() {
//     char spiRxBuffer[512] = {0};
//     char spiRxBuffer2[512] = {0};
//     char spiTxBuffer[512] = {0};

//     strcpy((char *)spiTxBuffer, "Test 1234 String");
//     FLASH_write(&hspi1, (uint8_t *)spiTxBuffer, 17777215, 16);
//     FLASH_read(&hspi1, (uint8_t *)spiRxBuffer, 17777215, 16);
//     FLASH_erase(&hspi1, 17777215);
//     FLASH_read(&hspi1, (uint8_t *)spiRxBuffer2, 17777215, 16);
// }
