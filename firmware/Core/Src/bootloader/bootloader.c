#include "bootloader/bootloader.h"
#include "stm32_internal_flash_drivers/stm32_internal_flash_drivers.h"

#include "main.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"


#include "FreeRTOS.h"
#include "task.h"

#include "debug_uart.h"

void Deinitialize_RTOS(void) 
{
    // start backwards so that we eliminate the  default task handle last
    for (int8_t task_idx = FREERTOS_task_handles_array_size - 1; task_idx >= 0; task_idx--) {
        FREERTOS_task_info_struct_t task = FREERTOS_task_handles_array[task_idx];
    
        if (task.task_handle != NULL && *task.task_handle != NULL) {
            vTaskDelete(*task.task_handle);
            task.task_handle = NULL;
            task.lowest_stack_bytes_remaining = UINT32_MAX;
        }
    }

    // osKernelLock();
    // osKernelSuspend(); // Stop the FreeRTOS scheduler
    // osKernelDeinitialize(); // Deinitialize the kernel
}


void BOOTLOADER_deinitialize_peripherals()
{
    

    

    // // Reset all peripherals
    



    HAL_TIM_Base_DeInit(&htim16);

    HAL_CRC_DeInit(&hcrc);

    HAL_SPI_DeInit(&hspi1);


    HAL_UART_DeInit(&hlpuart1);
    HAL_UART_DeInit(&huart4);
    HAL_UART_DeInit(&huart5);
    HAL_UART_DeInit(&huart1);
    HAL_UART_DeInit(&huart2);
    HAL_UART_DeInit(&huart3);

    HAL_I2C_DeInit(&hi2c1);
    HAL_I2C_DeInit(&hi2c2);
    HAL_I2C_DeInit(&hi2c3);
    HAL_I2C_DeInit(&hi2c4);


    HAL_DMA_DeInit(&hdma_uart4_rx);  
    HAL_DMA_DeInit(&hdma_usart1_rx);  

    

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_All);

    // Reset the system tick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // Deinitialize the HAL
    HAL_RCC_DeInit();
    HAL_DeInit();


__HAL_RCC_GPIOA_FORCE_RESET();
    __HAL_RCC_GPIOB_FORCE_RESET();
    __HAL_RCC_GPIOC_FORCE_RESET();
    __HAL_RCC_GPIOD_FORCE_RESET();
    __HAL_RCC_GPIOE_FORCE_RESET();
    __HAL_RCC_GPIOF_FORCE_RESET();
    __HAL_RCC_GPIOG_FORCE_RESET();
    __HAL_RCC_GPIOH_FORCE_RESET();
    __HAL_RCC_DMA1_FORCE_RESET();
    __HAL_RCC_DMA2_FORCE_RESET();
    __HAL_RCC_I2C1_FORCE_RESET();
    __HAL_RCC_I2C2_FORCE_RESET();
    __HAL_RCC_I2C3_FORCE_RESET();
    __HAL_RCC_I2C4_FORCE_RESET();
    __HAL_RCC_LPUART1_FORCE_RESET();
    __HAL_RCC_UART4_FORCE_RESET();
    __HAL_RCC_UART5_FORCE_RESET();
    __HAL_RCC_USART1_FORCE_RESET();
    __HAL_RCC_USART2_FORCE_RESET();
    __HAL_RCC_USART3_FORCE_RESET();
    __HAL_RCC_SPI1_FORCE_RESET();
    __HAL_RCC_CRC_FORCE_RESET();
    __HAL_RCC_TIM16_FORCE_RESET();

    // Release peripheral resets
    __HAL_RCC_GPIOA_RELEASE_RESET();
    __HAL_RCC_GPIOB_RELEASE_RESET();
    __HAL_RCC_GPIOC_RELEASE_RESET();
    __HAL_RCC_GPIOD_RELEASE_RESET();
    __HAL_RCC_GPIOE_RELEASE_RESET();
    __HAL_RCC_GPIOF_RELEASE_RESET();
    __HAL_RCC_GPIOG_RELEASE_RESET();
    __HAL_RCC_GPIOH_RELEASE_RESET();
    __HAL_RCC_DMA1_RELEASE_RESET();
    __HAL_RCC_DMA2_RELEASE_RESET();
    __HAL_RCC_I2C1_RELEASE_RESET();
    __HAL_RCC_I2C2_RELEASE_RESET();
    __HAL_RCC_I2C3_RELEASE_RESET();
    __HAL_RCC_I2C4_RELEASE_RESET();
    __HAL_RCC_LPUART1_RELEASE_RESET();
    __HAL_RCC_UART4_RELEASE_RESET();
    __HAL_RCC_UART5_RELEASE_RESET();
    __HAL_RCC_USART1_RELEASE_RESET();
    __HAL_RCC_USART2_RELEASE_RESET();
    __HAL_RCC_USART3_RELEASE_RESET();
    __HAL_RCC_SPI1_RELEASE_RESET();
    __HAL_RCC_CRC_RELEASE_RESET();
    __HAL_RCC_TIM16_RELEASE_RESET();


}

void DeinitializePeripherals(void) {
    /* Deinitialize UART peripherals */
    HAL_UART_DeInit(&hlpuart1);
    HAL_UART_DeInit(&huart4);
    HAL_UART_DeInit(&huart5);
    HAL_UART_DeInit(&huart1);
    HAL_UART_DeInit(&huart2);
    HAL_UART_DeInit(&huart3);

    /* Deinitialize DMA channels */
    HAL_DMA_DeInit(&hdma_uart4_rx);
    HAL_DMA_DeInit(&hdma_usart1_rx);

    /* Disable DMA controller clocks */
    __HAL_RCC_DMA1_CLK_DISABLE();
    __HAL_RCC_DMA2_CLK_DISABLE();

    /* Deinitialize I2C peripherals */
    HAL_I2C_DeInit(&hi2c1);
    HAL_I2C_DeInit(&hi2c2);
    HAL_I2C_DeInit(&hi2c3);
    HAL_I2C_DeInit(&hi2c4);

    /* Deinitialize SPI peripheral */
    HAL_SPI_DeInit(&hspi1);

    /* Deinitialize CRC peripheral */
    HAL_CRC_DeInit(&hcrc);

    /* Deinitialize Timer peripheral */
    HAL_TIM_Base_DeInit(&htim16);

    /* Deinitialize GPIO pins */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_All);

    /* Disable peripheral clocks */
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();
    __HAL_RCC_GPIOD_CLK_DISABLE();
    __HAL_RCC_GPIOE_CLK_DISABLE();

    /* Disable UART clocks */
    __HAL_RCC_LPUART1_CLK_DISABLE();
    __HAL_RCC_UART4_CLK_DISABLE();
    __HAL_RCC_UART5_CLK_DISABLE();
    __HAL_RCC_USART1_CLK_DISABLE();
    __HAL_RCC_USART2_CLK_DISABLE();
    __HAL_RCC_USART3_CLK_DISABLE();

    /* Disable I2C clocks */
    __HAL_RCC_I2C1_CLK_DISABLE();
    __HAL_RCC_I2C2_CLK_DISABLE();
    __HAL_RCC_I2C3_CLK_DISABLE();
    __HAL_RCC_I2C4_CLK_DISABLE();

    /* Disable SPI clocks */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /* Disable CRC clock */
    __HAL_RCC_CRC_CLK_DISABLE();

    /* Disable Timer clock */
    __HAL_RCC_TIM16_CLK_DISABLE();

    /* Reset peripherals */
    __HAL_RCC_UART4_FORCE_RESET();
    __HAL_RCC_UART4_RELEASE_RESET();
    __HAL_RCC_UART5_FORCE_RESET();
    __HAL_RCC_UART5_RELEASE_RESET();
    __HAL_RCC_USART1_FORCE_RESET();
    __HAL_RCC_USART1_RELEASE_RESET();
    __HAL_RCC_USART2_FORCE_RESET();
    __HAL_RCC_USART2_RELEASE_RESET();
    __HAL_RCC_USART3_FORCE_RESET();
    __HAL_RCC_USART3_RELEASE_RESET();
    __HAL_RCC_SPI1_FORCE_RESET();
    __HAL_RCC_SPI1_RELEASE_RESET();
    __HAL_RCC_I2C1_FORCE_RESET();
    __HAL_RCC_I2C1_RELEASE_RESET();
    __HAL_RCC_I2C2_FORCE_RESET();
    __HAL_RCC_I2C2_RELEASE_RESET();
    __HAL_RCC_I2C3_FORCE_RESET();
    __HAL_RCC_I2C3_RELEASE_RESET();
    __HAL_RCC_I2C4_FORCE_RESET();
    __HAL_RCC_I2C4_RELEASE_RESET();
    __HAL_RCC_CRC_FORCE_RESET();
    __HAL_RCC_CRC_RELEASE_RESET();
    __HAL_RCC_TIM16_FORCE_RESET();
    __HAL_RCC_TIM16_RELEASE_RESET();

    /* Optional: Disable SysTick if no longer needed */
    HAL_SuspendTick();
}


void BOOTLOADER_Jump_To_Application(uint32_t address)
{
    // pFunction JumpToApp;    

    // // Backup firmware's vector table address
    // uint32_t new_stack_pointer = *(__IO uint32_t *)(address);
    // uint32_t new_reset_handler = *(__IO uint32_t *)(address + 4);

    // // Validate the backup firmware
    // if (new_stack_pointer == 0xFFFFFFFF || new_reset_handler == 0xFFFFFFFF) {
    //     // Invalid firmware, do nothing
    //     return;
    // }

    // // Disable all interrupts
    // __disable_irq();

    // Deinitialize_RTOS();
    // // StopFreeRTOSKernel();
    // // BOOTLOADER_deinitialize_peripherals();
    // DeinitializePeripherals();
    // // Set vector table to backup firmware
    // SCB->VTOR = address;

    // // Set the Main Stack Pointer
    // __set_MSP(new_stack_pointer);

    // // Jump to the backup reset handler
    // JumpToApp = (pFunction)new_reset_handler;
    // JumpToApp();

     typedef void (*pFunction)(void);
    pFunction JumpToApp;    

    // Backup firmware's vector table address
    uint32_t new_stack_pointer = *(__IO uint32_t *)(address);
    uint32_t new_reset_handler = *(__IO uint32_t *)(address + 4);

    // Validate the application firmware
    if (new_stack_pointer == 0xFFFFFFFF ||  // Check if stack pointer is in SRAM range
        new_reset_handler == 0xFFFFFFFF) {                 // Check if reset handler is valid
        DEBUG_uart_print_str("Invalid vals\n");
        // Invalid firmware, do nothing
        return;
    }

    DeinitializePeripherals();
    // Disable all interrupts
    __disable_irq();

    // Deinitialize FreeRTOS and peripherals
    Deinitialize_RTOS();

    // Clear pending interrupts and disable SysTick
    // SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk | SCB_ICSR_PENDSTCLR_Msk;
    SysTick->CTRL = 0;

    // Disable all IRQs (set all NVIC IRQ enable registers to 0)
    // for (uint32_t i = 0; i < 8; i++) {
    //     NVIC->ICER[i] = 0xFFFFFFFF;
    //     NVIC->ICPR[i] = 0xFFFFFFFF;
    // }

    // Set vector table to the application firmware
    SCB->VTOR = address;

    // Set the Main Stack Pointer
    __set_MSP(new_stack_pointer);

    // Jump to the application reset handler
    JumpToApp = (pFunction)new_reset_handler;
    JumpToApp();


}

void BOOTLOADER_Jump_To_Golden_Copy()
{
    BOOTLOADER_Jump_To_Application(STM32_INTERNAL_FLASH_MEMORY_REGION_GOLDEN_COPY_ADDRESS);
    return;
}