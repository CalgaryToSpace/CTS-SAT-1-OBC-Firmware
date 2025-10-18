#include "main.h"
#include "littlefs/flash_internal_spi.h"

// Static functions are defined at the bottom of this file.
static void _chip_select_low(uint8_t chip_number);
static void _chip_select_high();
static HAL_StatusTypeDef _SPI_fast_receive(SPI_HandleTypeDef *hspi, uint8_t *response, uint16_t response_len);

// The spi handle used by the flash driver.
SPI_HandleTypeDef *hspi_flash_ptr = &hspi1;
const uint16_t FLASH_SPI_TIMEOUT_MS = 25;

// Used by SPI1 RxCpltCallback to indicate that the transfer is complete.
volatile uint8_t SPI_DMA_receive_complete = 0;

FLASH_error_enum_t FLASH_SPI_send_command(const FLASH_SPI_Data_t cmd[], uint8_t chip_number) {
    _chip_select_low(chip_number);
    const HAL_StatusTypeDef tx_result = HAL_SPI_Transmit(hspi_flash_ptr, cmd->data, cmd->len, FLASH_SPI_TIMEOUT_MS);
    _chip_select_high();

    if (tx_result == HAL_OK) {
        return FLASH_ERR_OK;
    } else {
        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }
}





FLASH_error_enum_t FLASH_SPI_send_command_with_data(
    const FLASH_SPI_Data_t cmd[], FLASH_SPI_Data_t *data, uint8_t chip_number
) {
    _chip_select_low(chip_number);
    HAL_StatusTypeDef tx_result = HAL_SPI_Transmit(hspi_flash_ptr, cmd->data, cmd->len, FLASH_SPI_TIMEOUT_MS);
    if (tx_result == HAL_OK) {
        tx_result = HAL_SPI_Transmit(hspi_flash_ptr, data->data, data->len, FLASH_SPI_TIMEOUT_MS);
    }
    _chip_select_high();

    if (tx_result == HAL_OK) {
        return FLASH_ERR_OK;
    } else {
        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }
}





FLASH_error_enum_t FLASH_SPI_send_command_receive_response(
    const FLASH_SPI_Data_t cmd[], uint8_t *response, uint16_t response_len, uint8_t chip_number
) {
    HAL_StatusTypeDef  rx_result;

    _chip_select_low(chip_number);

    const HAL_StatusTypeDef tx_result = HAL_SPI_Transmit(hspi_flash_ptr, cmd->data, cmd->len, FLASH_SPI_TIMEOUT_MS);
    if (tx_result == HAL_OK) {
        rx_result = _SPI_fast_receive(hspi_flash_ptr, response, response_len);
    }

    _chip_select_high();

    if (tx_result != HAL_OK) {
        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    } else if (rx_result != HAL_OK) {
        return FLASH_ERR_SPI_RECEIVE_FAILED;
    } else {
        return FLASH_ERR_OK;
    }
}





void FLASH_SPI_enable_then_disable_chip_select(uint8_t chip_number) {

    _chip_select_high();

    _chip_select_low(chip_number);
    HAL_Delay(1000);
    _chip_select_high();
}

/// @brief Use DMA to speed up receiving. At least 3x faster than HAL_SPI_Receive when transferring 2048 bytes.
static HAL_StatusTypeDef _SPI_fast_receive(SPI_HandleTypeDef *hspi, uint8_t *response, uint16_t response_len) {
    const uint32_t start_time = HAL_GetTick();
    uint32_t time_elapsed_ms = 0;

    // Set the receive complete flag to false.
    SPI_DMA_receive_complete = 0;

    // Start the receiving with DMA.
    const HAL_StatusTypeDef rx_result = HAL_SPI_Receive_DMA(hspi_flash_ptr, response, response_len);


    // Be careful here, we are writing into stack memory with DMA. This function should not return until the transfer is complete.
    // TODO: We could use a static buffer then memcpy to the response buffer, but that adds 1ms overhead. Probably not necessary.
    // Wait for DMA transfer to complete.
    while ((rx_result == HAL_OK) && !SPI_DMA_receive_complete && time_elapsed_ms < 10) {
        time_elapsed_ms = HAL_GetTick() - start_time;
    }

    SPI_DMA_receive_complete = 0;

    if (time_elapsed_ms >= 10) {
        HAL_SPI_Abort(hspi);
        return HAL_TIMEOUT;
    } else if (rx_result != HAL_OK) {
        return rx_result;
    }  
    return HAL_OK;
}


static inline void _chip_select_low(uint8_t chip_number) {
    _chip_select_high();
    // NOTE: the "reset low" activate action must be AFTER all other pins are "set high"
    // TODO: Should be a switch statement but I'm lazy right now.
    if (chip_number == 0) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_0_GPIO_Port, PIN_MEM_NCS_FLASH_0_Pin, GPIO_PIN_RESET);
    } else if (chip_number == 1) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_1_GPIO_Port, PIN_MEM_NCS_FLASH_1_Pin, GPIO_PIN_RESET);
    } else if (chip_number == 2) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_2_GPIO_Port, PIN_MEM_NCS_FLASH_2_Pin, GPIO_PIN_RESET);
    } else if (chip_number == 3) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_3_GPIO_Port, PIN_MEM_NCS_FLASH_3_Pin, GPIO_PIN_RESET);
    }
    
    // TODO: check if this is how we want to access the FRAM chips
    else if (chip_number == 8) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FRAM_0_GPIO_Port, PIN_MEM_NCS_FRAM_0_Pin, GPIO_PIN_RESET);
    } else if (chip_number == 9) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FRAM_1_GPIO_Port, PIN_MEM_NCS_FRAM_1_Pin, GPIO_PIN_RESET);
    }
}





/// @brief Deactivates the chip select for all lines.
static inline void _chip_select_high() {
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_0_GPIO_Port, PIN_MEM_NCS_FLASH_0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_1_GPIO_Port, PIN_MEM_NCS_FLASH_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_2_GPIO_Port, PIN_MEM_NCS_FLASH_2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_3_GPIO_Port, PIN_MEM_NCS_FLASH_3_Pin, GPIO_PIN_SET);
    
    HAL_GPIO_WritePin(PIN_MEM_NCS_FRAM_0_GPIO_Port, PIN_MEM_NCS_FRAM_0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FRAM_1_GPIO_Port, PIN_MEM_NCS_FRAM_1_Pin, GPIO_PIN_SET);
}

// Callback is used when using DMA to receive data.
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi == hspi_flash_ptr) {
        SPI_DMA_receive_complete = 1;
    }
}