# Non-Critical STM32 Notes

* Lower NVIC priority number means higher priority.
* For UART, FIFO Mode must be enabled.
* When starting a new FreeRTOS project, you must manually increase the total heap size. Hard 
faults may mean a thread is running out of stack memory.
* With UART DMA, calling `HAL_UART_Receive_DMA()` takes a long time (maybe, 1ms). Do not call it inside an Interrupt Service Routine (ISR). Call it only once to start the DMA receive.
