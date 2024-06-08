
// /*-------------------------Memory Module Code Testing Begin-------------------------*/
// // Turn on LED1 to indicate program starting
// HAL_GPIO_WritePin(PIN_LED1_OUT_GPIO_Port, PIN_LED1_OUT_Pin, GPIO_PIN_SET);
// debug_uart_print_str("Memory Module Testing Begins\n");

// char spiRxBuffer[512] = {0};
// char spiRxBuffer2[512] = {0};
// char spiTxBuffer[512] = {0};

// strcpy((char *)spiTxBuffer, "Test 1234 String");
// FLASH_write(&hspi1, (uint8_t *)spiTxBuffer, 17777215, 16);
// FLASH_read(&hspi1, (uint8_t *)spiRxBuffer, 17777215, 16);
// FLASH_erase(&hspi1, 17777215);
// FLASH_read(&hspi1, (uint8_t *)spiRxBuffer2, 17777215, 16);

// //LittleFS testing
// uint8_t boot_count = 0;
// int8_t result;
// char result_read;

// // Initialize LittleFS Values, and pass SPI pointer
// LFS_INITIALIZE(&hspi1);

// result = LFS_format();
// if (result < 0)
// {
//   debug_uart_print_str("Formatting Error: ");
//   debug_uart_print_uint32(result);
//   debug_uart_print_str("\n");
//   return result;
// }

// for (int i = 0; i < 10; i++)
// {
//   result = LFS_mount();
//   if (result < 0)
//   {
//     debug_uart_print_str("Mounting Error: ");
//     debug_uart_print_uint32(result);
//     debug_uart_print_str("\n");
//     return result;
//   }

//   result = LFS_read_file("boot_count.txt", &boot_count, sizeof(boot_count));
//   if (result < 0)
//   {
//     debug_uart_print_str("Reading Error: ");
//     debug_uart_print_uint32(result);
//     debug_uart_print_str("\n");
//     return result;
//   }

//   debug_uart_print_str("Read Value: ");
//   debug_uart_print_uint32(boot_count);
//   debug_uart_print_str("\n");

//   boot_count += 1;

//   result = LFS_write_file("boot_count.txt", &boot_count, sizeof(boot_count));
//   if (result < 0)
//   {
//     debug_uart_print_str("Writing Error: ");
//     debug_uart_print_str(&result_read);
//     debug_uart_print_str("\n");
//     return result;
//   }

//   result = LFS_unmount();
//   if (result < 0)
//   {
//     debug_uart_print_str("Unmounting Error: ");
//     debug_uart_print_uint32(result);
//     debug_uart_print_str("\n");
//     return result;
//   }
// }

// // Turn off LED1 to indicate Program End
// HAL_GPIO_WritePin(PIN_LED1_OUT_GPIO_Port, PIN_LED1_OUT_Pin, GPIO_PIN_RESET);
// debug_uart_print_str("Memory Module Testing Ends\n");
// /*-------------------------Memory Module Code Testing End-------------------------*/
