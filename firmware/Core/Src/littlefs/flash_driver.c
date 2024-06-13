/**
 * flash_driver.c
 *
 * Created on: May 2, 2024
 * Author: Saksham Puri
 */

/*-----------------------------INCLUDES-----------------------------*/
// #include "main.h" // FIXME: Is this needed?
#include "littlefs/flash_driver.h"
#include "debug_tools/debug_uart.h"

/*------------------------------DEFINES------------------------------*/
// TODO: check on timeout, maybe decrease a lot
#define FLASH_TIMEOUT_MS 50

// -----------------------------FLASH DRIVER FUNCTIONS-----------------------------
/**
 * @brief Activates the chip select for the given flash module number.
 * @param chip_number - Index of the chip select line to activate
 * @details Activates the chip select for the given chip number by setting the corresponding GPIO
 *          pins LOW. Deactivates all other chip selects by setting their corresponding GPIO pins HIGH.
 * @retval None - GPIO writes can't fail
 */
void FLASH_activate_chip_select(uint8_t chip_number)
{
    // NOTE: the "reset low" activate action must be AFTER all other pins are "set high"
    FLASH_deactivate_chip_select();

    if (chip_number == 0)
    {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_0_GPIO_Port, PIN_MEM_NCS_FLASH_0_Pin, GPIO_PIN_RESET);
    }
    else if (chip_number == 1)
    {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_1_GPIO_Port, PIN_MEM_NCS_FLASH_1_Pin, GPIO_PIN_RESET);
    }
    else if (chip_number == 2)
    {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_2_GPIO_Port, PIN_MEM_NCS_FLASH_2_Pin, GPIO_PIN_RESET);
    }
    else if (chip_number == 3)
    {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_3_GPIO_Port, PIN_MEM_NCS_FLASH_3_Pin, GPIO_PIN_RESET);
    }
    else if (chip_number == 4)
    {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_4_GPIO_Port, PIN_MEM_NCS_FLASH_4_Pin, GPIO_PIN_RESET);
    }
    else if (chip_number == 5)
    {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_5_GPIO_Port, PIN_MEM_NCS_FLASH_5_Pin, GPIO_PIN_RESET);
    }
    else if (chip_number == 6)
    {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_6_GPIO_Port, PIN_MEM_NCS_FLASH_6_Pin, GPIO_PIN_RESET);
    }
    else if (chip_number == 7)
    {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_7_GPIO_Port, PIN_MEM_NCS_FLASH_7_Pin, GPIO_PIN_RESET);
    }

    // TODO: check if this is how we want to access the FRAM chips
    else if (chip_number == 8)
    {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FRAM_0_GPIO_Port, PIN_MEM_NCS_FRAM_0_Pin, GPIO_PIN_RESET);
    }
    else if (chip_number == 9)
    {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FRAM_1_GPIO_Port, PIN_MEM_NCS_FRAM_1_Pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief Deactivates the chip select for all lines.
 * @retval None - GPIO writes can't fail
 */
void FLASH_deactivate_chip_select()
{
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_0_GPIO_Port, PIN_MEM_NCS_FLASH_0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_1_GPIO_Port, PIN_MEM_NCS_FLASH_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_2_GPIO_Port, PIN_MEM_NCS_FLASH_2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_3_GPIO_Port, PIN_MEM_NCS_FLASH_3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_4_GPIO_Port, PIN_MEM_NCS_FLASH_4_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_5_GPIO_Port, PIN_MEM_NCS_FLASH_5_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_6_GPIO_Port, PIN_MEM_NCS_FLASH_6_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_7_GPIO_Port, PIN_MEM_NCS_FLASH_7_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(PIN_MEM_NCS_FRAM_0_GPIO_Port, PIN_MEM_NCS_FRAM_0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FRAM_1_GPIO_Port, PIN_MEM_NCS_FRAM_1_Pin, GPIO_PIN_SET);
}

/**
 * @brief Read Status Register and store the values in given buffer
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @param buf - Pointer to a buffer to store SR1 value. Length: 1 byte.
 * @retval 0 on success, != 0 on failure
 */
uint8_t FLASH_read_status_register(SPI_HandleTypeDef *hspi, uint8_t chip_number, uint8_t *buf)
{
    FLASH_activate_chip_select(chip_number);

    const HAL_StatusTypeDef tx_result = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_RDSR1, 1, FLASH_TIMEOUT_MS);
    if (tx_result != HAL_OK)
    {
        FLASH_deactivate_chip_select();
        return 1;
    }

    const HAL_StatusTypeDef rx_result = HAL_SPI_Receive(hspi, (uint8_t *)buf, 1, FLASH_TIMEOUT_MS);
    if (rx_result != HAL_OK)
    {
        FLASH_deactivate_chip_select();
        return 2;
    }

    FLASH_deactivate_chip_select();
    return 0;
}

/**
 * @brief Sends Write Enable Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @retval 0 on success, != 0 on failure
 */
uint8_t FLASH_write_enable(SPI_HandleTypeDef *hspi, uint8_t chip_number)
{
    // Buffer to store status register values in
    uint8_t status_reg_buffer[1] = {0};

    FLASH_activate_chip_select(chip_number);
    const HAL_StatusTypeDef tx_result_1 = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_WREN, 1, HAL_MAX_DELAY);
    FLASH_deactivate_chip_select();
    if (tx_result_1 != HAL_OK)
    {
        return 1;
    }

    // Keep looping as long as device is busy
    // FIXME: Needs a timeout, could stall forever if status register isn't changed 
    uint8_t wip = 1;
    while (wip)
    {
        const uint8_t read_status_result = FLASH_read_status_register(hspi, chip_number, status_reg_buffer);
        if (read_status_result != 0)
        {
            FLASH_deactivate_chip_select();
            return 2;
        }
        wip = status_reg_buffer[0] & 1;

        DEBUG_uart_print_str("DEBUG: status_reg = ");
        DEBUG_uart_print_array_hex(status_reg_buffer, 1);
        DEBUG_uart_print_str("\n");
    }
    return 0;
}

/**
 * @brief Sends Write Disable Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @retval 0 on success, != 0 on failure
 */
uint8_t FLASH_write_disable(SPI_HandleTypeDef *hspi, uint8_t chip_number)
{

    // Buffer to store status register values in
    uint8_t status_reg_buffer[1] = {0};

    FLASH_deactivate_chip_select();
    const HAL_StatusTypeDef tx_status = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_WRDI, 1, FLASH_TIMEOUT_MS);

    if (tx_status != HAL_OK)
    {
        FLASH_deactivate_chip_select();
        return 1;
    }

    // Keep looping until Write Enable latch isn't 0
    // FIXME: Needs a timeout, could stall forever if status register isn't changed 
    uint8_t wel = 1;
    while (wel)
    {
        const uint8_t read_status_result = FLASH_read_status_register(hspi, chip_number, status_reg_buffer);
        if (read_status_result != 0)
        {
            FLASH_deactivate_chip_select();
            return 2;
        }
        wel = status_reg_buffer[0] & 2;
    }
    return 0;
}

/**
 * @brief Sends Sector Erase Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @param addr - address that is to be erased
 * @retval Returns 0 on success, != 0 on failure
 */
uint8_t FLASH_erase(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr)
{
    // Split block into 3 address spaces that the device accepts
    uint8_t addr_bytes[4] = {(addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};

    // Send Write Enable Command
    const uint8_t wren_result = FLASH_write_enable(hspi, chip_number);
    if (wren_result != 0)
    {
        FLASH_deactivate_chip_select();
        return 1;
    }

    // Send Sector Erase Command
    FLASH_activate_chip_select(chip_number);
    const HAL_StatusTypeDef tx_result_1 = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_4SE, 1, FLASH_TIMEOUT_MS);
    if (tx_result_1 != HAL_OK)
    {
        FLASH_deactivate_chip_select();
        return 2;
    }

    // Send the address to erase
    const HAL_StatusTypeDef tx_result_2 = HAL_SPI_Transmit(hspi, (uint8_t *)addr_bytes, 4, FLASH_TIMEOUT_MS);
    if (tx_result_2 != HAL_OK)
    {
        FLASH_deactivate_chip_select();
        return 3;
    }

    FLASH_deactivate_chip_select();

    // Buffer to store status register values in
    uint8_t status_reg_buffer[1] = {0};

    // Keep looping as long as device is busy
    // FIXME: Needs a timeout, could stall forever if status register isn't changed 
    uint8_t wip = 1;
    uint8_t err = 0;
    while (wip)
    {
        const uint8_t status_result = FLASH_read_status_register(hspi, chip_number, status_reg_buffer);
        if (status_result != 0)
        {
            FLASH_deactivate_chip_select();
            return 5;
        }
        wip = status_reg_buffer[0] & 1;
        err = status_reg_buffer[0] & 0b01000000;
        if (err == 1)
        {
            return 6;
        }
    }

    return 0;
}

/**
 * @brief Sends Page Program Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @param addr - block number that is to be written
 * @param packet_buffer - Pointer to buffer containing data to write
 * @param size - integer that idicates the size of the data
 * @retval Returns 0 on success, != 0 on failure
 */
uint8_t FLASH_write(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr, uint8_t *packet_buffer, lfs_size_t size)
{
    // Split block into 3 address spaces that the device accepts
    uint8_t addr_bytes[4] = {(addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};

    // Enable WREN Command, so that we can write to the memory module
    const uint8_t wren_result = FLASH_write_enable(hspi, chip_number);
    if (wren_result != 0)
    {
        FLASH_deactivate_chip_select();
        return 1;
    }

    // Send Page Program Command and the Data required with the command
    FLASH_activate_chip_select(chip_number);
    const uint8_t tx_result_1 = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_4WRITE, 1, FLASH_TIMEOUT_MS);
    if (tx_result_1 != HAL_OK)
    {
        FLASH_deactivate_chip_select();
        return 2;
    }

    // Send the Address for the page program
    const uint8_t tx_result_2 = HAL_SPI_Transmit(hspi, (uint8_t *)addr_bytes, 4, FLASH_TIMEOUT_MS);
    if (tx_result_2 != HAL_OK)
    {
        FLASH_deactivate_chip_select();
        return 3;
    }

    // Send Data to program
    const uint8_t tx_result_3 = HAL_SPI_Transmit(hspi, (uint8_t *)packet_buffer, size, FLASH_TIMEOUT_MS);
    if (tx_result_3 != HAL_OK)
    {
        FLASH_deactivate_chip_select();
        return 4;
    }

    FLASH_deactivate_chip_select();

    // Buffer to store status register values in
    uint8_t status_reg_buffer[1] = {0};

    // Keep looping as long as device is busy
    // FIXME: Needs a timeout, could stall forever if status register isn't changed 
    uint8_t wip = 1;
    uint8_t err = 0;
    while (wip)
    {
        const uint8_t status_result = FLASH_read_status_register(hspi, chip_number, status_reg_buffer);
        if (status_result != 0)
        {
            FLASH_deactivate_chip_select();
            return 5;
        }

        wip = status_reg_buffer[0] & 1;
        err = status_reg_buffer[0] & 0b01000000;
        if (err == 1)
        {
            return 6;
        }
    }

    return 0;
}

/**
 * @brief Sends Page Program Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @param addr - block number that is to be read
 * @param rx_buffer - a to buffer where the read data will be stored
 * @param rx_buffer_len - integer that idicates the size of the data
 * @retval Returns 0 on success, != 0 on failure
 */
uint8_t FLASH_read(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr, uint8_t *rx_buffer, lfs_size_t rx_buffer_len)
{
    // Split block into 3 address spaces that the device accepts
    uint8_t addr_bytes[4] = {(addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};

    // Send Read Command and the data required with it
    FLASH_activate_chip_select(chip_number);
    const HAL_StatusTypeDef tx_result_1 = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_4READ, 1, FLASH_TIMEOUT_MS);
    if (tx_result_1 != HAL_OK)
    {
        FLASH_deactivate_chip_select();
        return 1;
    }
    const HAL_StatusTypeDef tx_result_2 = HAL_SPI_Transmit(hspi, (uint8_t *)addr_bytes, 4, FLASH_TIMEOUT_MS);
    if (tx_result_2 != HAL_OK)
    {
        FLASH_deactivate_chip_select();
        return 2;
    }
    const HAL_StatusTypeDef rx_result_1 = HAL_SPI_Receive(hspi, (uint8_t *)rx_buffer, rx_buffer_len, FLASH_TIMEOUT_MS);
    if (rx_result_1 != HAL_OK)
    {
        FLASH_deactivate_chip_select();
        return 3;
    }
    FLASH_deactivate_chip_select();

    // Haven't yet implemented a way to check any errors while reading data from memory
    return 0;
}

uint8_t FLASH_is_reachable(SPI_HandleTypeDef *hspi, uint8_t chip_number)
{
    // TODO: confirm if this works with the CS2 logical chip on each physical FLASH chip;
    // ^ Seems as though it only works for CS1.
    const uint8_t READ_ID_CMD = 0x9F;

    uint8_t tx_buffer[1] = {READ_ID_CMD};
    uint8_t rx_buffer[5];
    memset(rx_buffer, 0, 5);

    FLASH_activate_chip_select(chip_number);

    // Transmit the READ_ID_CMD
    if (HAL_SPI_Transmit(&hspi1, tx_buffer, 1, FLASH_TIMEOUT_MS) != HAL_OK)
    {
        FLASH_deactivate_chip_select();
        return 1;
    }

    // Receive the response
    if (HAL_SPI_Receive(&hspi1, rx_buffer, 5, FLASH_TIMEOUT_MS) != HAL_OK)
    {
        FLASH_deactivate_chip_select();
        return 2;
    }

    FLASH_deactivate_chip_select();

    // Check the received ID (modify according to the expected ID of your memory module)
    // rx_buffer[0] is the manufacturer ID, rx_buffer[1] is the memory type,
    // and rx_buffer[2] is the memory capacity (not checked, as we have a few different capacities).
    // rx_buffer[2] is 0x20=512 for 512 Mib (mebibits)
    // TODO: maybe check the capacity as well here, esp. in deployment
    uint8_t are_bytes_correct = 0;
    if (rx_buffer[0] == 0x01 && rx_buffer[1] == 0x02)
    {
        DEBUG_uart_print_str("SUCCESS: FLASH_is_reachable received IDs: ");
        are_bytes_correct = 1;
    }
    else
    {
        DEBUG_uart_print_str("ERROR: FLASH_is_reachable received IDs: ");
        are_bytes_correct = 0;
    }

    DEBUG_uart_print_array_hex(rx_buffer, 5);
    DEBUG_uart_print_str("\n");

    if (!are_bytes_correct)
    {
        // error: IDs don't match
        return 3;
    }
    return 0; // success
}