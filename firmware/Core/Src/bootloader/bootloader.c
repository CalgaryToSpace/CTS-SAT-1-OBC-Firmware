#include "bootloader/bootloader.h"
#include "stm32_internal_flash_drivers/stm32_internal_flash_drivers.h"

#include "main.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

typedef void (*pFunction)(void);

void BOOTLOADER_deinitialize_peripherals()
{

    // Disable all interrupts
    __disable_irq();

    // Deinitialize all peripherals
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_All);

    HAL_DMA_DeInit(&hdma_uart4_rx);  // Replace with your DMA handlers
    HAL_DMA_DeInit(&hdma_usart1_rx);  // Replace with your DMA handlers
    
    HAL_I2C_DeInit(&hi2c1);
    HAL_I2C_DeInit(&hi2c2);
    HAL_I2C_DeInit(&hi2c3);
    HAL_I2C_DeInit(&hi2c4);
    HAL_UART_DeInit(&hlpuart1);
    HAL_UART_DeInit(&huart4);
    HAL_UART_DeInit(&huart5);
    HAL_UART_DeInit(&huart1);
    HAL_UART_DeInit(&huart2);
    HAL_UART_DeInit(&huart3);
    HAL_SPI_DeInit(&hspi1);
    HAL_CRC_DeInit(&hcrc);
    HAL_TIM_Base_DeInit(&htim16);

    // Reset all peripherals
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

    // Reset the system tick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // Deinitialize the HAL
    HAL_RCC_DeInit();
    HAL_DeInit();

}

void BOOTLOADER_Jump_To_Application(uint32_t address)
{
    pFunction JumpToApp;    

    // Backup firmware's vector table address
    uint32_t BackupSP = *(__IO uint32_t *)(address);
    uint32_t BackupResetHandler = *(__IO uint32_t *)(address + 4);

    // Validate the backup firmware
    if (BackupSP == 0xFFFFFFFF || BackupResetHandler == 0xFFFFFFFF) {
        // Invalid firmware, do nothing
        return;
    }

    // Set vector table to backup firmware
    SCB->VTOR = address;

    // Set the Main Stack Pointer
    __set_MSP(BackupSP);

    // Jump to the backup reset handler
    JumpToApp = (pFunction)BackupResetHandler;
    JumpToApp();

}

void BOOTLOADER_Jump_To_Golden_Copy()
{
    BOOTLOADER_Jump_To_Application(STM32_INTERNAL_FLASH_MEMORY_REGION_GOLDEN_COPY_ADDRESS);
    return;
}