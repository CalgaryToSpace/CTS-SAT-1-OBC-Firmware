CROSS   = arm-none-eabi-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy

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
    -Wall

TARGET = blob

all: $(TARGET).bin

$(TARGET).elf: blob_main.c
	$(CC) $(CFLAGS) -T blob.ld $< -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f $(TARGET).elf $(TARGET).bin
