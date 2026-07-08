CROSS   = arm-none-eabi-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
SIZE    = $(CROSS)size

BUILD_DIR = build

CFLAGS  = \
    -mthumb \
    -mcpu=cortex-m4 \
    -mfpu=fpv4-sp-d16 \
    -mfloat-abi=hard \
    -fPIC \
    -ffunction-sections \
    -fdata-sections \
    -nostdlib \
    -Os \
    -Wall \
    -Wextra

CFLAGS += \
    -ffreestanding \
    -fno-builtin \
    -fno-stack-protector \
    -fno-common \
    -fno-jump-tables

LDFLAGS += -Wl,--gc-sections

TARGET = copy_file_blob

ELF = $(BUILD_DIR)/$(TARGET).elf
BIN = $(BUILD_DIR)/$(TARGET).bin

all: $(BIN)

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build, link, and print the size info.
$(ELF): copy_file_blob_main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -T blob.ld $< -o $@ $(LDFLAGS)
	@echo "--- Section sizes ---"
	$(SIZE) -A $@
	@if $(SIZE) -A $@ | awk '/\.(data|bss)/{if ($$2+0>0) exit 1}'; then :; else \
	    echo "FATAL: blob defines non-empty .data/.bss - globals are forbidden"; exit 1; fi

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)
	echo ""
