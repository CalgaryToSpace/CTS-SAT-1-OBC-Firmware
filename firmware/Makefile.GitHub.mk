##########################################################################################################################
# File automatically-generated by tool: [projectgenerator] version: [4.4.0-B60] date: [Mon Oct 21 18:49:21 MDT 2024] 
##########################################################################################################################

# CTS: This file is based on the `Makefile`, generated by CubeMX.

# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET = CTS-SAT-1_FW


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES = $(shell find Core Drivers Middlewares -name "*.c")

# ASM sources
ASM_SOURCES = $(shell find Core Drivers Middlewares -name "*.s")

# ASM sources
ASMM_SOURCES = $(shell find Core Drivers Middlewares -name "*.S")

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32L4R5xx


# AS includes
AS_INCLUDES =  \
-ICore/Inc

# C includes
C_INCLUDES =  \
-ICore/Inc \
-IDrivers/STM32L4xx_HAL_Driver/Inc \
-IDrivers/STM32L4xx_HAL_Driver/Inc/Legacy \
-IMiddlewares/Third_Party/FreeRTOS/Source/include \
-IMiddlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 \
-IMiddlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
-IDrivers/CMSIS/Device/ST/STM32L4xx/Include \
-IDrivers/CMSIS/Include \
-IX-CUBE-SAFEA1/Target \
-IMiddlewares/ST/ST_Cryptolib/Inc/ \
-IMiddlewares/ST/ST_Cryptolib/Inc \
-IMiddlewares/ST/ST_Cryptolib/Inc/AES \
-IMiddlewares/ST/ST_Cryptolib/Inc/AES/CBC \
-IMiddlewares/ST/ST_Cryptolib/Inc/AES/CCM \
-IMiddlewares/ST/ST_Cryptolib/Inc/AES/CFB \
-IMiddlewares/ST/ST_Cryptolib/Inc/AES/CMAC \
-IMiddlewares/ST/ST_Cryptolib/Inc/AES/Common \
-IMiddlewares/ST/ST_Cryptolib/Inc/AES/CTR \
-IMiddlewares/ST/ST_Cryptolib/Inc/AES/ECB \
-IMiddlewares/ST/ST_Cryptolib/Inc/AES/GCM \
-IMiddlewares/ST/ST_Cryptolib/Inc/AES/KEYWRAP \
-IMiddlewares/ST/ST_Cryptolib/Inc/AES/OFB \
-IMiddlewares/ST/ST_Cryptolib/Inc/AES/XTS \
-IMiddlewares/ST/ST_Cryptolib/Inc/ARC4 \
-IMiddlewares/ST/ST_Cryptolib/Inc/C25519 \
-IMiddlewares/ST/ST_Cryptolib/Inc/CHACHA \
-IMiddlewares/ST/ST_Cryptolib/Inc/CHACHA20-POLY1305 \
-IMiddlewares/ST/ST_Cryptolib/Inc/Common \
-IMiddlewares/ST/ST_Cryptolib/Inc/Common_chacha20_poly1305 \
-IMiddlewares/ST/ST_Cryptolib/Inc/Common_ecc_rsa/MATH \
-IMiddlewares/ST/ST_Cryptolib/Inc/Common_ecc_rsa/MATH/BN \
-IMiddlewares/ST/ST_Cryptolib/Inc/Common_ed25519_c25519/internals \
-IMiddlewares/ST/ST_Cryptolib/Inc/DES \
-IMiddlewares/ST/ST_Cryptolib/Inc/DES/CBC \
-IMiddlewares/ST/ST_Cryptolib/Inc/DES/Common \
-IMiddlewares/ST/ST_Cryptolib/Inc/DES/ECB \
-IMiddlewares/ST/ST_Cryptolib/Inc/ECC \
-IMiddlewares/ST/ST_Cryptolib/Inc/ECC/LowLevel \
-IMiddlewares/ST/ST_Cryptolib/Inc/ED25519 \
-IMiddlewares/ST/ST_Cryptolib/Inc/ED25519/internals \
-IMiddlewares/ST/ST_Cryptolib/Inc/HASH \
-IMiddlewares/ST/ST_Cryptolib/Inc/HASH/Common \
-IMiddlewares/ST/ST_Cryptolib/Inc/HASH/MD5 \
-IMiddlewares/ST/ST_Cryptolib/Inc/HASH/SHA1 \
-IMiddlewares/ST/ST_Cryptolib/Inc/HASH/SHA224 \
-IMiddlewares/ST/ST_Cryptolib/Inc/HASH/SHA256 \
-IMiddlewares/ST/ST_Cryptolib/Inc/HASH/SHA384 \
-IMiddlewares/ST/ST_Cryptolib/Inc/HASH/SHA512 \
-IMiddlewares/ST/ST_Cryptolib/Inc/POLY1305 \
-IMiddlewares/ST/ST_Cryptolib/Inc/RNG \
-IMiddlewares/ST/ST_Cryptolib/Inc/RNG/DRBG_AES128 \
-IMiddlewares/ST/ST_Cryptolib/Inc/RSA \
-IMiddlewares/ST/ST_Cryptolib/Inc/RSA/LowLevel \
-IMiddlewares/ST/ST_Cryptolib/Inc/RSA/PKCS\#1v15 \
-IMiddlewares/ST/ST_Cryptolib/Inc/TDES \
-IMiddlewares/ST/ST_Cryptolib/Inc/TDES/CBC \
-IMiddlewares/ST/ST_Cryptolib/Inc/TDES/Common \
-IMiddlewares/ST/ST_Cryptolib/Inc/TDES/ECB \
-IMiddlewares/ST/ST_Cryptolib/X509/Inc


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

# CTS: Enable extra warnings for safety
CFLAGS += -Wextra -Wpedantic -Wmissing-field-initializers -Wno-unused-parameter -Werror

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32L4R5ZITx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = \


# CTS: Additional LD Flags from config file
ADDITIONALLDFLAGS = -Wl,--print-memory-usage -fdiagnostics-color=always -specs=nano.specs 

LDFLAGS = $(MCU) $(ADDITIONALLDFLAGS) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASMM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASMM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@
$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
