#include "main.h"
#include "bootloader/bootloader.h"
#include "stm32_internal_flash_drivers/stm32_internal_flash_drivers.h"
#include "log.h"

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"



/// @brief Deinitialize all peripherals before jumping 
/// @param  
void BOOTLOADER_deinitialize_peripherals(void) {
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

/// @brief Jumps to any point in memory 
/// @param address The Address to jump to
/// @return Only returns if address is invalid, returning 1
uint8_t BOOTLOADER_Jump_To_Application(uint32_t address)
{

    pFunction JumpToApp;    

    uint32_t new_stack_pointer = *(__IO uint32_t *)(address);
    uint32_t new_reset_handler = *(__IO uint32_t *)(address + 4);


    if (new_stack_pointer == 0xFFFFFFFF ||  
        new_reset_handler == 0xFFFFFFFF) {                 
        LOG_message(LOG_SYSTEM_OBC, LOG_SEVERITY_WARNING, LOG_SINK_ALL, "Invalid jump address. Values are not defined.");
        return 1;
    }

    BOOTLOADER_deinitialize_peripherals();

    // Disable all IRQs (set all NVIC IRQ enable registers to 0)
    for (uint32_t i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    // Set vector table to the application firmware
    SCB->VTOR = address;

    // Set the Main Stack Pointer
    __set_MSP(new_stack_pointer);

    // Jump to the application reset handler
    JumpToApp = (pFunction)new_reset_handler;
    JumpToApp();

    // this should never be reached. this is to appease the compiler
    return 0;

}

/// @brief Jumps to golden copy
/// @return Only returns if jump is unsuccessful
uint8_t BOOTLOADER_Jump_To_Golden_Copy()
{

    const uint8_t jump_failed = BOOTLOADER_Jump_To_Application(STM32_INTERNAL_FLASH_MEMORY_REGION_GOLDEN_COPY_ADDRESS);
    
    return jump_failed;
}