# Non-Critical STM32 Notes

* Lower NVIC priority number means higher priority.
* For UART, FIFO Mode must be enabled.
* When starting a new FreeRTOS project, you must manually increase the total heap size. Hard 
faults may mean a thread is running out of stack memory.
