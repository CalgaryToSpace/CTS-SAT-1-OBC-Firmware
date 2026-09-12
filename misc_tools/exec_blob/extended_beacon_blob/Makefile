CROSS   = arm-none-eabi-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
SIZE    = $(CROSS)size

BUILD_DIR = build

# Reference firmware ELF: resolves `extern` firmware functions/globals used by
# the blob source to their real addresses at link time (see --just-symbols below),
# instead of hardcoding numeric addresses in the C source.
FW_ELF = ../CTS-SAT-1_FW_rc3.elf

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
    -Wextra \
    -I../../../firmware/Core/Inc/

CFLAGS += \
    -ffreestanding \
    -fno-builtin \
    -fno-stack-protector \
    -fno-common \
    -fno-jump-tables

LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--just-symbols=$(FW_ELF)

TARGET = extended_beacon_blob

# Name the blob is uplinked as (see docs/Mission_Operations/Blobs_List.md).
BLOB_NAME = extended_beacon_v4

ELF = $(BUILD_DIR)/$(TARGET).elf
BIN = $(BUILD_DIR)/$(TARGET).bin
BLOB = $(BUILD_DIR)/$(BLOB_NAME).blob

all: $(BIN) $(BLOB)

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build, link, and print the size info.
$(ELF): extended_beacon_blob_main.c $(FW_ELF) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -T blob.ld $< -o $@ $(LDFLAGS)
	@echo "--- Section sizes ---"
	$(SIZE) -A $@
	@if $(SIZE) -A $@ | awk '/\.(data|bss)/{if ($$2+0>0) exit 1}'; then :; else \
	    echo "FATAL: blob defines non-empty .data/.bss - globals are forbidden"; exit 1; fi

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

# Same bytes as the .bin, named as it should be uplinked.
$(BLOB): $(BIN)
	cp $< $@

clean:
	rm -rf $(BUILD_DIR)
	echo ""
