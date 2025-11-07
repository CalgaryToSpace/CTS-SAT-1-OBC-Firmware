#include "littlefs/flash_driver.h"
#include "log/log.h"

// Static functions are defined at the bottom of this file.
static FLASH_error_enum_t FLASH_disable_block_lock(uint8_t chip_number);
static FLASH_error_enum_t FLASH_write_enable(uint8_t chip_number);
static FLASH_error_enum_t FLASH_write_disable(uint8_t chip_number);
static FLASH_error_enum_t FLASH_wait_until_ready(uint8_t chip_number);

FLASH_error_enum_t FLASH_init(uint8_t chip_number) {
    return FLASH_disable_block_lock(chip_number);
}


FLASH_error_enum_t FLASH_erase_block(uint8_t chip_number, FLASH_Physical_Address_t address) {
    FLASH_write_enable(chip_number);

    // Send erase command along with the address of the block.
    uint32_t block_address = address.row_address; // Address to the start of the block.
    uint8_t cmd_buff[] = {FLASH_CMD_BLOCK_ERASE, ((block_address >> 16) & 0xFF), ((block_address >> 8) & 0xFF), (block_address & 0xFF)};
    FLASH_SPI_Data_t cmd = {.data = cmd_buff, .len = sizeof(cmd_buff)};

    FLASH_error_enum_t result = FLASH_SPI_send_command(&cmd, chip_number);
    if (result != FLASH_ERR_OK) { 
        goto cleanup; 
    }

    result = FLASH_wait_until_ready(chip_number);

cleanup:
    FLASH_write_disable(chip_number);
    return result;
}





FLASH_error_enum_t FLASH_program_page(uint8_t chip_number, FLASH_Physical_Address_t address, uint8_t *data, uint32_t data_len) {

    FLASH_write_enable(chip_number);
    
    // TODO: not sure if I'm doing the memory mapping correctly. maybe should be offset as the address?
    // Send the program load command along with the address of where in the page to start writing the data.  (always 0 since we always write a full page).
    
    const uint32_t col_address = address.col_address;
    const uint8_t program_load_command_bytes[] = {FLASH_CMD_PROGRAM_LOAD, (col_address >> 8) & 0xFF, (col_address & 0xFF)}; 

    const FLASH_SPI_Data_t load_command = {.data = program_load_command_bytes, .len = sizeof(program_load_command_bytes)};
    
    FLASH_SPI_Data_t data_to_load = {.data = data, .len = data_len};
    FLASH_error_enum_t result = FLASH_SPI_send_command_with_data(&load_command, &data_to_load, chip_number);
    if (result != FLASH_ERR_OK) { 
        goto cleanup;
    }

    // Now we write the data in the cache to the flash array at the specified address.
    const uint64_t row_address = address.row_address;
    const uint8_t program_execute_command_bytes[] = {
        FLASH_CMD_PROGRAM_EXECUTE, ((row_address >> 16) & 0xFF), ((row_address >> 8) & 0xFF), (row_address & 0xFF)
    };
    const FLASH_SPI_Data_t program_execute_cmd = {.data = program_execute_command_bytes, .len = sizeof(program_execute_command_bytes)};

    result = FLASH_SPI_send_command(&program_execute_cmd, chip_number);
    if (result != FLASH_ERR_OK) { 
        goto cleanup;
    }

    result = FLASH_wait_until_ready(chip_number);

cleanup:
    FLASH_write_disable(chip_number);
    return result;
}





FLASH_error_enum_t FLASH_read_page(
    uint8_t chip_number, FLASH_Physical_Address_t address, uint8_t *rx_buffer, uint32_t rx_buffer_size
) {
    const uint64_t row_address = address.row_address;

    const uint8_t cmd_buff[] = {FLASH_CMD_PAGE_READ, ((row_address >> 16) & 0xFF), ((row_address >> 8) & 0xFF), (row_address & 0xFF)};
    const FLASH_SPI_Data_t read_cmd = {.data = cmd_buff, .len = sizeof(cmd_buff)};

    FLASH_error_enum_t result = FLASH_SPI_send_command(&read_cmd, chip_number);
    if (result != FLASH_ERR_OK) {
        return result;
    }

    // Wait until the page is read into the cache from the main array.
    result = FLASH_wait_until_ready(chip_number);
    if (result != FLASH_ERR_OK) {
        return result;
    }


    // Read the data from the cache into the buffer.
    const uint32_t col_address = address.col_address;

    const uint8_t read_from_cache_cmd_bytes[] = {
        // Send col address along with one dummy byte (see pg. 18 of the datasheet).
        FLASH_CMD_READ_FROM_CACHE, (col_address >> 8) & 0xFF, (col_address & 0xFF), 0x00
    };
    const FLASH_SPI_Data_t read_from_cache_cmd = {.data = read_from_cache_cmd_bytes, .len = sizeof(read_from_cache_cmd_bytes)};

    return FLASH_SPI_send_command_receive_response(&read_from_cache_cmd, rx_buffer, rx_buffer_size, chip_number);
}





FLASH_error_enum_t FLASH_is_reachable(uint8_t chip_number) {
    const uint8_t read_id_cmd_bytes[] = {FLASH_CMD_READ_ID, 0x00}; // send one dummy byte (see pg. 25 of the datasheet).
    const FLASH_SPI_Data_t read_id_cmd = {.data = read_id_cmd_bytes, .len = sizeof(read_id_cmd_bytes)};

    uint8_t response[] = {0x00, 0x00}; // expecting a 2 byte response.
    const FLASH_error_enum_t result = FLASH_SPI_send_command_receive_response(
        &read_id_cmd, response, sizeof(response), chip_number
    );
    
    // TODO: something like this is probably worth having.
    // See pg. 25 of the datasheet for the expected response.
    // if (response[0] == 0x2C && response[1] == 0x14) {
    //     return FLASH_ERR_UNKNOWN;
    // } else {
    //     return result;
    // }

    return result;
}





FLASH_error_enum_t FLASH_reset(uint8_t chip_number) {
    const uint8_t reset_cmd_bytes[] = {FLASH_CMD_RESET};
    const FLASH_SPI_Data_t reset_cmd = {.data = reset_cmd_bytes, .len = sizeof(reset_cmd_bytes)};

    return FLASH_SPI_send_command(&reset_cmd, chip_number);
}





FLASH_error_enum_t FLASH_read_status_register(uint8_t chip_number, uint8_t *response) {
    const uint8_t cmd_buff[] = {FLASH_CMD_GET_FEATURES, FLASH_FEAT_STATUS};
    const FLASH_SPI_Data_t cmd = {.data = cmd_buff, .len = sizeof(cmd_buff)};

    return FLASH_SPI_send_command_receive_response(&cmd, response, 1, chip_number);
}





/// @brief Here for testing purposes.
/// @param chip_number the chip select line to enable.
void FLASH_enable_then_disable_chip_select(uint8_t chip_number) {
    FLASH_SPI_enable_then_disable_chip_select(chip_number);
}



/// @brief Wait until the flash chip is ready for the next command.
/// @param chip_number the chip select line to enable.
/// @return FLASH_ERR_OK on success, or an error code on failure.
static FLASH_error_enum_t FLASH_wait_until_ready(uint8_t chip_number) {
    // TODO: this will need to be changed if we change the clock speed.
    const uint8_t max_attempts = 20; //TODO: Decide on what this should be. 10 was too low, 20 seems to work well.

    for (uint16_t attempts = 0; attempts < max_attempts; attempts++) {
        uint8_t status_register;
        const FLASH_error_enum_t result = FLASH_read_status_register(chip_number, &status_register);
        if (result != FLASH_ERR_OK) {
            return result;
        }
            
        const uint8_t flash_is_busy = (status_register & FLASH_OP_IN_PROGRESS_MASK);
        if (!flash_is_busy) {
            return FLASH_ERR_OK;
        }

        // Note: Should not delay here. Leads to slower execution.
    }

    return FLASH_ERR_DEVICE_BUSY_TIMEOUT;
}




/// @brief Disable the block lock feature of the flash chip, which is enabled by default after a power cycle.
static FLASH_error_enum_t FLASH_disable_block_lock(uint8_t chip_number) {
    const uint8_t cmd_buff[] = {FLASH_CMD_SET_FEATURES, FLASH_FEAT_BLOCK_LOCK, FLASH_FEAT_STATE_DISABLED};
    const FLASH_SPI_Data_t cmd = {.data = cmd_buff, .len = sizeof(cmd_buff)};

    return FLASH_SPI_send_command(&cmd, chip_number);
}


static FLASH_error_enum_t FLASH_write_enable(uint8_t chip_number) {
    const uint8_t cmd_buff[] = {FLASH_CMD_WRITE_ENABLE};
    const FLASH_SPI_Data_t cmd = {.data = cmd_buff, .len = sizeof(cmd_buff)};

    return FLASH_SPI_send_command(&cmd, chip_number);
}





static FLASH_error_enum_t FLASH_write_disable(uint8_t chip_number) {
    const uint8_t cmd_buff[] = {FLASH_CMD_WRITE_DISABLE};
    const FLASH_SPI_Data_t cmd = {.data = cmd_buff, .len = sizeof(cmd_buff)};

    return FLASH_SPI_send_command(&cmd, chip_number);
}
