#include "littlefs/flash_driver.h"
#include "log/log.h"

// static functions which are defined at the bottom of this file.
static FLASH_error_enum_t FLASH_disable_block_lock(uint8_t chip_number);
static FLASH_error_enum_t FLASH_read_status_register(uint8_t chip_number, uint8_t *response);
static FLASH_error_enum_t FLASH_write_enable(uint8_t chip_number);
// static FLASH_error_enum_t FLASH_read_block_lock_register(uint8_t chip_number, uint8_t *response);
static FLASH_error_enum_t FLASH_write_disable(uint8_t chip_number);
static FLASH_error_enum_t FLASH_wait_until_ready(uint8_t chip_number);

//TODO: this is not compelete.
FLASH_error_enum_t FLASH_init(uint8_t chip_number) {
    return FLASH_disable_block_lock(chip_number);
}


FLASH_error_enum_t FLASH_erase_block(uint8_t chip_number, FLASH_Physical_Address_t address) {

    FLASH_write_enable(chip_number);

    //TODO: not sure if I'm doing the memory mapping correctly. 
    // Send erase command along with the address of the block.
    uint16_t block_address = address.block_address;
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





FLASH_error_enum_t FLASH_program_page(uint8_t chip_number, FLASH_Physical_Address_t address, uint8_t *data, lfs_size_t data_len) {

    // FLASH_disable_block_lock(chip_number); // TODO: why do we need to disable block lock? it's never turned back on?
    FLASH_write_enable(chip_number);
    
    // TODO: not sure if I'm doing the memory mapping correctly. maybe should be offset as the address?
    // Send the program load command along with the address of where in the page to start writing the data.  (always 0 since we always write a full page).
    
    uint16_t col_address = address.col_address;
    uint8_t program_load_command_bytes[] = {FLASH_CMD_PROGRAM_LOAD, (col_address >> 8) & 0xFF, (col_address & 0xFF)}; 

    FLASH_SPI_Data_t load_command = {.data = program_load_command_bytes, .len = sizeof(program_load_command_bytes)};
    FLASH_SPI_Data_t data_to_load = {.data = data, .len = data_len};
    
    FLASH_error_enum_t result = FLASH_SPI_send_command_with_data(&load_command, &data_to_load, chip_number);
    if (result != FLASH_ERR_OK) { 
        goto cleanup;
    }

    // Now we write the data in the cache to the flash array at the specified address.
    //TODO: not sure if I'm doing the memory mapping correctly.
    uint64_t row_address =  address.row_address;;//(block << 6) + (offset / FLASH_CHIP_PAGE_SIZE_BYTES);  // left shift 6 to multiply by 64 since each block has 64 pages.
    uint8_t program_execute_command_bytes[] = {FLASH_CMD_PROGRAM_EXECUTE, ((row_address >> 16) & 0xFF), ((row_address >> 8) & 0xFF), (row_address & 0xFF)};
    FLASH_SPI_Data_t program_execute_cmd = {.data = program_execute_command_bytes, .len = sizeof(program_execute_command_bytes)};

    result = FLASH_SPI_send_command(&program_execute_cmd, chip_number);
    if (result != FLASH_ERR_OK) { 
        goto cleanup;
    }

    result = FLASH_wait_until_ready(chip_number);

cleanup:
    FLASH_write_disable(chip_number);
    return result;
}





FLASH_error_enum_t FLASH_read_page(uint8_t chip_number, FLASH_Physical_Address_t address, uint8_t *rx_buffer, lfs_size_t rx_buffer_len) {

    uint64_t row_address = address.row_address;

    uint8_t cmd_buff[] = {FLASH_CMD_PAGE_READ, ((row_address >> 16) & 0xFF), ((row_address >> 8) & 0xFF), (row_address & 0xFF)};
    FLASH_SPI_Data_t read_cmd = {.data = cmd_buff, .len = sizeof(cmd_buff)};

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
    uint16_t col_address = address.col_address;

    uint8_t read_from_cache_cmd_bytes[] = {FLASH_CMD_READ_FROM_CACHE, (col_address >> 8) & 0xFF, (col_address & 0xFF), 0x00}; // send col address along with one dummy byte (see pg. 18 of the datasheet).
    FLASH_SPI_Data_t read_from_cache_cmd = {.data = read_from_cache_cmd_bytes, .len = sizeof(read_from_cache_cmd_bytes)};

    return FLASH_SPI_send_command_receive_response(&read_from_cache_cmd, rx_buffer, rx_buffer_len, chip_number);
}





FLASH_error_enum_t FLASH_is_reachable(uint8_t chip_number) {

    uint8_t read_id_cmd_bytes[] = {FLASH_CMD_READ_ID, 0x00}; // send one dummy byte (see pg. 25 of the datasheet).
    FLASH_SPI_Data_t read_id_cmd = {.data = read_id_cmd_bytes, .len = sizeof(read_id_cmd_bytes)};

    uint8_t response[] = {0x00, 0x00}; // expecting a 2 byte response.
    FLASH_error_enum_t result = FLASH_SPI_send_command_receive_response(&read_id_cmd, response, sizeof(response), chip_number);
    
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

    uint8_t reset_cmd_bytes[] = {FLASH_CMD_RESET};
    FLASH_SPI_Data_t reset_cmd = {.data = reset_cmd_bytes, .len = sizeof(reset_cmd_bytes)};

    return FLASH_SPI_send_command(&reset_cmd, chip_number);
}





static FLASH_error_enum_t FLASH_wait_until_ready(uint8_t chip_number){
    const uint8_t max_attempts = 10; //TODO: Decide on what this should be.
    uint8_t attempts = 0;
    

    uint8_t status_register;
    FLASH_error_enum_t result = FLASH_read_status_register(chip_number, &status_register);
    if (result != FLASH_ERR_OK) {
        return result;
    }

    uint8_t flash_is_busy = (status_register & FLASH_OP_IN_PROGRESS_MASK);
    while (flash_is_busy && attempts < max_attempts) {

        result = FLASH_read_status_register(chip_number, &status_register);
        if (result != FLASH_ERR_OK) { return result; }
            
        flash_is_busy = (status_register & FLASH_OP_IN_PROGRESS_MASK);
        attempts++;
    }

    if (flash_is_busy) {
        return FLASH_ERR_DEVICE_BUSY_TIMEOUT;
    } else {
        return FLASH_ERR_OK;
    }
}




/// @brief Disable the block lock feature of the flash chip, which is enabled by default after a power cycle.
static FLASH_error_enum_t FLASH_disable_block_lock(uint8_t chip_number) {

    uint8_t cmd_buff[] = {FLASH_CMD_SET_FEATURES, FLASH_FEAT_BLOCK_LOCK, FLASH_FEAT_STATE_DISABLED};
    FLASH_SPI_Data_t cmd = {.data = cmd_buff, .len = sizeof(cmd_buff)};

    return FLASH_SPI_send_command(&cmd, chip_number);
}  





static FLASH_error_enum_t FLASH_read_status_register(uint8_t chip_number, uint8_t *response) {

    uint8_t cmd_buff[] = {FLASH_CMD_GET_FEATURES, FLASH_FEAT_STATUS};
    FLASH_SPI_Data_t cmd = {.data = cmd_buff, .len = sizeof(cmd_buff)};

    return FLASH_SPI_send_command_receive_response(&cmd, response, sizeof(uint8_t), chip_number);
}





//FIXME: do we need this?
// static FLASH_error_enum_t FLASH_read_block_lock_register(uint8_t chip_number, uint8_t *response) {

//     uint8_t cmd_buff[] = {FLASH_CMD_GET_FEATURES, FLASH_FEAT_BLOCK_LOCK};
//     FLASH_SPI_Data_t cmd = {.data = cmd_buff, .len = sizeof(cmd_buff)};

//     return FLASH_SPI_send_command_receive_response(&cmd, response, sizeof(uint8_t), chip_number);
// }





static FLASH_error_enum_t FLASH_write_enable(uint8_t chip_number) {

    uint8_t cmd_buff[] = {FLASH_CMD_WRITE_ENABLE};
    FLASH_SPI_Data_t cmd = {.data = cmd_buff, .len = sizeof(cmd_buff)};

    return FLASH_SPI_send_command(&cmd, chip_number);
}





static FLASH_error_enum_t FLASH_write_disable(uint8_t chip_number) {

    uint8_t cmd_buff[] = {FLASH_CMD_WRITE_DISABLE};
    FLASH_SPI_Data_t cmd = {.data = cmd_buff, .len = sizeof(cmd_buff)};

    return FLASH_SPI_send_command(&cmd, chip_number);
}



