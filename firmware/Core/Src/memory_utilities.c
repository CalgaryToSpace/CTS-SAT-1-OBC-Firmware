/**
 * memory_utilities.c
 *
 * Created on: May 2, 2024
 *     Author: Saksham Puri
 */

/*-----------------------------INCLUDES-----------------------------*/
#include "memory_utilities.h"
#include "debug_tools/debug_uart.h"

/*-----------------------------VARIABLES-----------------------------*/
// Variables to track LittleFS on Flash Memory Module
uint8_t LFS_initialized = 0;
uint8_t mounted = 0;

// Variables LittleFS uses for various functions
lfs_t lfs;
lfs_file_t file;
struct lfs_config cfg;

// LittleFS Buffers for reading and writing
uint8_t lfs_read_buf[512];
uint8_t lfs_prog_buf[512];

// Variable to store SPI configuration
SPI_HandleTypeDef *hspi_ptr;

// -----------------------------LITTLEFS CONFIG FUNCTIONS-----------------------------
/**
 * @brief Initialize LittleFS configurations
 * @param None
 * @retval None
 */
void LFS_Config()
{
	// block device operations
	cfg.read = block_device_read;
	cfg.prog = block_device_prog;
	cfg.erase = block_device_erase;
	cfg.sync = block_device_sync;

	// block device configuration
	cfg.read_size = 512;
	cfg.prog_size = 512;
	cfg.block_size = 262144;
	cfg.block_count = 256;
	cfg.block_cycles = 100; // ASK ABOUT THIS (HOW FREQUENT ARE WE USING THE MODULE)
	cfg.cache_size = 512;
	cfg.lookahead_size = 16;
	cfg.compact_thresh = -1; // Defaults to ~88% block_size when zero (lfs.h, line 232)

	cfg.read_buffer = lfs_read_buf;
	cfg.prog_buffer = lfs_prog_buf;
}

/**
 * @brief LittleFS read function
 * @param LittleFS Configurations, Block to write, offset, buffer, buffer size
 * @retval int - any error codes that happened in littlefs
 */
int block_device_read(const struct lfs_config *c, lfs_block_t block,
					  lfs_off_t off, void *buffer, lfs_size_t size)
{

	return READ_LFS(hspi_ptr, (uint8_t *)buffer, (block * c->block_size + off),
					size);
}

/**
 * @brief LittleFS write function
 * @param LittleFS Configurations, Block to read, offset, buffer, buffer size
 * @retval int - any error codes that happened in littlefs
 */
int block_device_prog(const struct lfs_config *c, lfs_block_t block,
					  lfs_off_t off, const void *buffer, lfs_size_t size)
{
	return WRITE_LFS(hspi_ptr, (uint8_t *)buffer, (block * c->block_size + off),
					 size);
}

/**
 * @brief LittleFS erase function
 * @param LittleFS Configurations, Block to erase
 * @retval int - any error codes that happened in littlefs
 */
int block_device_erase(const struct lfs_config *c, lfs_block_t block)
{
	MEM_CLEAR_LFS(hspi_ptr, block * c->block_size);
	//	MEM_CLEAR_LFS(hspi_ptr, block * c->block_size);
	return 0;
}

/**
 * @brief LittleFS sync function
 * @param LittleFS Configurations
 * @retval int - 0 since this function isn't used
 */
int block_device_sync(const struct lfs_config *c)
{
	return 0;
}

// -----------------------------MEMORY FUNCTIONS-----------------------------
/*
 * Chip Select set to LOW
 */
void CS_LOW()
{
	HAL_GPIO_WritePin(PIN_SCI_NCS_FLASH_0_GPIO_Port, PIN_SCI_NCS_FLASH_0_Pin,
					  GPIO_PIN_RESET); // LSB
}

/*
 * Chip Select set to HIGH
 * This is the default state of the CS
 */
void CS_HIGH()
{
	HAL_GPIO_WritePin(PIN_SCI_NCS_FLASH_0_GPIO_Port, PIN_SCI_NCS_FLASH_0_Pin,
					  GPIO_PIN_SET);
}

/**
 * @brief Read Status Register and store the values in given buffer
 * @param SPI_HandleTypeDef - Pointer to the SPI configurations used
 * @param uint8_t - Pointer to a buffer to store SR1 values
 * @retval None
 */
void READ_STATUS_REGISTER(SPI_HandleTypeDef *hspi1, uint8_t *buf)
{
	CS_LOW();
	HAL_SPI_Transmit(hspi1, (uint8_t *)&FLASH_RDSR1, 1, 100);
	HAL_SPI_Receive(hspi1, (uint8_t *)buf, 1, 100);
	CS_HIGH();
}

/**
 * @brief Sends Write Enable Command
 * @param SPI_HandleTypeDef - Pointer to the SPI configurations used
 * @retval None
 */
void ENABLE_WREN(SPI_HandleTypeDef *hspi1)
{
	// Buffer to store status register values in
	uint8_t statusRegBuffer[1] = {0};

	CS_LOW();
	if (HAL_SPI_Transmit(hspi1, (uint8_t *)&FLASH_WREN, 1, HAL_MAX_DELAY) != HAL_OK)
		debug_uart_print_str("Timeout when enabling WREN\n");
	CS_HIGH();

	// Keep looping as long as device is busy
	uint8_t wip = 1;
	while (wip)
	{
		READ_STATUS_REGISTER(hspi1, statusRegBuffer);
		wip = statusRegBuffer[0] & 1;
	}
}

/**
 * @brief Sends Write Disable Command
 * @param SPI_HandleTypeDef - Pointer to the SPI configurations used
 * @retval None
 */
void ENABLE_WRDI(SPI_HandleTypeDef *hspi1)
{
	// Buffer to store status register values in
	uint8_t statusRegBuffer[1] = {0};

	CS_HIGH();
	HAL_SPI_Transmit(hspi1, (uint8_t *)&FLASH_WRDI, 1, 100);

	// Keep looping until Write Enable latch isn't 0
	uint8_t wel = 1;
	while (wel)
	{
		READ_STATUS_REGISTER(hspi1, statusRegBuffer);
		wel = statusRegBuffer[0] & 2;
	}
}

/**
 * @brief Sends Sector Erase Command
 * @param SPI_HandleTypeDef - Pointer to the SPI configurations used
 * @param lfs_block_t - block number that is to be erased
 * @retval Returns -1 if erase failed, else return 0
 */
uint8_t MEM_CLEAR_LFS(SPI_HandleTypeDef *ptr_hspi1, lfs_block_t block)
{
	// Split block into 3 address spaces that the device accepts
	uint8_t addr[3] = {(block >> 16) & 0xFF, (block >> 8) & 0xFF, block & 0xFF};

	// Send Write Enable Command
	ENABLE_WREN(ptr_hspi1);

	// Send Sector Erase Command
	CS_LOW();
	HAL_SPI_Transmit(ptr_hspi1, (uint8_t *)&FLASH_SE, 1, 100);
	HAL_SPI_Transmit(ptr_hspi1, (uint8_t *)addr, 3, 100);
	CS_HIGH();

	// Buffer to store status register values in
	uint8_t statusRegBuffer[1] = {0};

	// Keep looping as long as device is busy
	uint8_t wip = 1;
	uint8_t err = 0;
	while (wip)
	{
		READ_STATUS_REGISTER(ptr_hspi1, statusRegBuffer);
		wip = statusRegBuffer[0] & 1;
		err = statusRegBuffer[0] & 0b01000000;
		if (err == 1)
		{
			debug_uart_print_str("Error during Erasing\n");
			break;
		}
	}
	if (err)
		return -1;
	else
		return 0;
}

/**
 * @brief Sends Page Program Command
 * @param SPI_HandleTypeDef - Pointer to the SPI configurations used
 * @param uint8_t - Pointer to buffer containing data to write
 * @param lfs_block_t - block number that is to be written
 * @param lfs_size_t - integer that idicates the size of the data
 * @retval Returns -1 if write failed, else return 0
 */
uint8_t WRITE_LFS(SPI_HandleTypeDef *hspi1, uint8_t *packetBuffer, lfs_block_t block, lfs_size_t size)
{
	// Split block into 3 address spaces that the device accepts
	uint8_t addr[3] = {(block >> 16) & 0xFF, (block >> 8) & 0xFF, block & 0xFF};

	// Enable WREN Command, so that we can write to the memory module
	ENABLE_WREN(hspi1);

	// Send WREN Command and the Data required with the command
	CS_LOW();
	HAL_SPI_Transmit(hspi1, (uint8_t *)&FLASH_WRITE, 1, 100);
	HAL_SPI_Transmit(hspi1, (uint8_t *)addr, 3, 100);
	HAL_SPI_Transmit(hspi1, (uint8_t *)packetBuffer, size, 100);
	CS_HIGH();

	// Buffer to store status register values in
	uint8_t statusRegBuffer[1] = {0};

	// Keep looping as long as device is busy
	uint8_t wip = 1;
	uint8_t err = 0;
	while (wip)
	{
		READ_STATUS_REGISTER(hspi1, statusRegBuffer);
		wip = statusRegBuffer[0] & 1;
		err = statusRegBuffer[0] & 0b01000000;
		if (err == 1)
		{
			debug_uart_print_str("Error during Writing\n");
			break;
		}
	}
	if (err)
		return -1;
	else
		return 0;
}

/**
 * @brief Sends Page Program Command
 * @param SPI_HandleTypeDef - Pointer to the SPI configurations used
 * @param uint8_t - Pointer to buffer where the read data will be stored
 * @param lfs_block_t - block number that is to be read
 * @param lfs_size_t - integer that idicates the size of the data
 * @retval Returns -1 if read failed, else return 0
 */
uint8_t READ_LFS(SPI_HandleTypeDef *hspi1, uint8_t *spiRxBuffer, lfs_block_t block, lfs_size_t size)
{
	// Split block into 3 address spaces that the device accepts
	uint8_t addr[3] = {(block >> 16) & 0xFF, (block >> 8) & 0xFF, block & 0xFF};

	// Send Read Command and the data required with it
	CS_LOW();
	HAL_SPI_Transmit(hspi1, (uint8_t *)&FLASH_READ, 1, 100);
	HAL_SPI_Transmit(hspi1, (uint8_t *)addr, 3, 100);
	HAL_SPI_Receive(hspi1, (uint8_t *)spiRxBuffer, size, 100);
	CS_HIGH();

	// Haven't yet implemented a way to check any errors while reading data from memory
	return 0;
}

// -----------------------------LITTLEFS FUNCTIONS-----------------------------
/**
 * @brief Initializes all LittleFS values as well as
 * 		  recieves SPI pointer to use for all functions
 * @param SPI_HandleTypeDef - Pointer to the SPI configurations used
 * @retval None
 */
void INITIALIZE(SPI_HandleTypeDef *ptr)
{
	LFS_Config();
	LFS_initialized = 1;
	hspi_ptr = ptr;
}

/**
 * @brief Fromates Memory Module so it can successfully mount LittleFS
 * @param None
 * @retval Returns negative values if format failed, else return 0
 */
int8_t FORMAT()
{
	if (LFS_initialized)
	{
		int8_t result = lfs_format(&lfs, &cfg);
		if (result < 0)
		{
			debug_uart_print_str("Error Formatting!\n");
			return result;
		}
		else
		{
			debug_uart_print_str("Formatting Successful!\n");
			return result;
		}
	}
	debug_uart_print_str("LittleFS not Initialized\n");
	return -1;
}

/**
 * @brief Mounts LittleFS to the Memory Module
 * @param None
 * @retval Returns negative values if mount failed, else return 0
 */
int8_t MOUNT()
{
	// If LittleFS is initialized and not mounted
	if (LFS_initialized && !mounted)
	{
		// Variable to store status of LittleFS mounting
		int8_t result = lfs_mount(&lfs, &cfg);
		if (result < 0)
		{
			debug_uart_print_str("Mounting Unsuccessful\n");
			return result;
		}
		else
		{
			debug_uart_print_str("Mounting Successful\n");
			mounted = 1;
			return 0;
		}
	}
	if (!LFS_initialized) debug_uart_print_str("LittleFS not initialized!\n");
	else debug_uart_print_str("LittleFS already mounted!\n");
	return -1;
}

/**
 * @brief Unmounts LittleFS to the Memory Module
 * @param None
 * @retval Returns negative values if unmount failed, else return 0
 */
int8_t UNMOUNT()
{
	if (LFS_initialized && mounted)
	{
		// Unmount LittleFS to release any resources used by LittleFS
		int8_t result = lfs_unmount(&lfs);
		if (result < 0)
		{
			debug_uart_print_str("Error Un-mounting!\n");
			return result;
		}
		else
		{
			debug_uart_print_str("Successfully Un-mounted LittleFS!\n");
			mounted = 0;
			return result;
		}
	}
	if (!LFS_initialized) debug_uart_print_str("LittleFS not initialized!\n");
	else debug_uart_print_str("LittleFS not mounted!\n");
	return -1;
}

//Will be deprecated soon!!
int8_t FORCE_MOUNT()
{
	if (LFS_initialized && !mounted)
	{
		// Check if LittleFS successfully mounted
		int8_t result = MOUNT();

		// If it doesn't work, try formatting the memory, and mounting again
		if (result < 0)
		{
			result = FORMAT();
			if (result < 0)
				return result;

			result = MOUNT();
			if (result < 0)
				return result;
		}
		return 0;
	}
	debug_uart_print_str("LittleFS not Initialized or already mounted\n");
	return -1;
}

void LIST_DIRECTORY()
{
}

void REMOVE_FILE()
{
}

void MAKE_DIRECTORY()
{
}

void MOVE_DIRECTORY()
{
}

/**
 * @brief Creates / Opens LittleFS File to write to the Memory Module
 * @param char - Pointer to buffer holding the file name to create or open
 * @param void - Pointer to buffer holding the data to write
 * @param uint32_t - Size of the data to write
 * @retval Returns negative values if write or file create / open failed, else return 0
 */
int8_t WRITE_FILE(char *file_name, void *write_buffer, uint32_t size)
{
	//Create or Open a file with Write only flag
	int8_t result = lfs_file_open(&lfs, &file, file_name, LFS_O_WRONLY | LFS_O_CREAT);

	if (result < 0)
	{
		debug_uart_print_str("Error Opening / Creating a File!\n");
		return result;
	}
	else
	{
		debug_uart_print_str("Opened / Created a file named: \n");
		debug_uart_print_str(file_name);

		// Write data to file
		result = lfs_file_write(&lfs, &file, write_buffer, size);
		if (result < 0)
		{
			debug_uart_print_str("Error Writing to File!\n");
			return result;
		}
		else
		{
			debug_uart_print_str("Successfully wrote data to file!\n");

			// Close the File, the storage is not updated until the file is closed successfully
			result = lfs_file_close(&lfs, &file);
			if (result < 0)
			{
				debug_uart_print_str("Error Closing the File!\n");
				return result;
			}
			else
			{
				debug_uart_print_str("Successfully closed the File!\n");
				return result;
			}
		}
	}
}

/**
 * @brief Opens LittleFS File to read from the Memory Module
 * @param char - Pointer to buffer holding the file name to open
 * @param void - Pointer to buffer where the read data will be stored
 * @param uint32_t - Size of the data to read
 * @retval Returns negative values if read or file open failed, else return 0
 */
int8_t READ_FILE(char *file_name, void *read_buffer, uint32_t size)
{
	int8_t result = lfs_file_open(&lfs, &file, file_name, LFS_O_RDONLY | LFS_O_CREAT);
	if (result < 0)
	{
		debug_uart_print_str("Error Opening / Creating a File!\n");
		return result;
	}
	else
	{
		debug_uart_print_str("Opened / Created a file named: \n");
		debug_uart_print_str(file_name);

		// Write defined data to file
		result = lfs_file_read(&lfs, &file, read_buffer, size);
		if (result < 0)
		{
			debug_uart_print_str("Error Reading File!\n");
			return result;
		}
		else
		{
			debug_uart_print_str("Successfully read file!\n");

			// Close the File, the storage is not updated until the file is closed successfully
			result = lfs_file_close(&lfs, &file);
			if (result < 0)
			{
				debug_uart_print_str("Error Closing the File!\n");
				return result;
			}
			else
			{
				debug_uart_print_str("Successfully closed the File!\n");
				return result;
			}
		}
	}
}
