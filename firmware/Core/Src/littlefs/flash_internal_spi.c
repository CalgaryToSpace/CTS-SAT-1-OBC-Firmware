#include "main.h"
#include "littlefs/flash_internal_spi.h"

// defined at the bottom of this file.
static void _chip_select_low(uint8_t chip_number);
static void _chip_select_high();

// TODO: double check what this should be.
//The spi handle used for by the flash driver.
SPI_HandleTypeDef *hspi_flash_ptr = &hspi1;

// TODO: double check what this should be.
const uint16_t FLASH_SPI_TIMEOUT_MS =  25;


FLASH_error_enum_t FLASH_SPI_send_command(FLASH_SPI_Data_t* cmd, uint8_t chip_number) {

    _chip_select_low(chip_number);
    const HAL_StatusTypeDef tx_result = HAL_SPI_Transmit(hspi_flash_ptr, cmd->data, cmd->len, FLASH_SPI_TIMEOUT_MS);
    _chip_select_high();

    if (tx_result == HAL_OK) {
        return FLASH_ERR_OK;
    } else {
        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }
}





FLASH_error_enum_t FLASH_SPI_send_command_with_data(FLASH_SPI_Data_t *cmd, FLASH_SPI_Data_t *data, uint8_t chip_number){

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





FLASH_error_enum_t FLASH_SPI_send_command_receive_response(FLASH_SPI_Data_t *cmd, uint8_t *response, uint16_t response_len, uint8_t chip_number) {

    _chip_select_low(chip_number);
    const HAL_StatusTypeDef tx_result = HAL_SPI_Transmit(hspi_flash_ptr, cmd->data, cmd->len, FLASH_SPI_TIMEOUT_MS);
    const HAL_StatusTypeDef rx_result = HAL_SPI_Receive(hspi_flash_ptr, response, response_len, FLASH_SPI_TIMEOUT_MS);
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





void _chip_select_low(uint8_t chip_number)
{
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
void _chip_select_high() {
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_0_GPIO_Port, PIN_MEM_NCS_FLASH_0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_1_GPIO_Port, PIN_MEM_NCS_FLASH_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_2_GPIO_Port, PIN_MEM_NCS_FLASH_2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_3_GPIO_Port, PIN_MEM_NCS_FLASH_3_Pin, GPIO_PIN_SET);
    
    HAL_GPIO_WritePin(PIN_MEM_NCS_FRAM_0_GPIO_Port, PIN_MEM_NCS_FRAM_0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FRAM_1_GPIO_Port, PIN_MEM_NCS_FRAM_1_Pin, GPIO_PIN_SET);
}