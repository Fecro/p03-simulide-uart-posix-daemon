# Makefile for ATmega328P bare-metal sensor node firmware (SimulIDE)

MCU      := atmega328p
F_CPU    := 16000000UL

AVR_CC   := avr-gcc
OBJCOPY  := avr-objcopy
AVR_SIZE := avr-size

AVR_CSTD := $(shell $(AVR_CC) -std=c23 -x c -E /dev/null >/dev/null 2>&1 && echo "-std=c23" || \
              ($(AVR_CC) -std=c2x -x c -E /dev/null >/dev/null 2>&1 && echo "-std=c2x" || echo "-std=c11"))

CFLAGS   := $(AVR_CSTD) -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Iinclude \
            -Wall -Wextra -Werror -pedantic -Wconversion -Wsign-conversion

BUILD_DIR := build
TARGET    := sensor_node
SRCS      := src/main.c src/uart.c src/adc.c src/crc16.c
OBJS      := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean test-host

all: $(TARGET).hex

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(AVR_CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(AVR_CC) $(CFLAGS) $(OBJS) -o $@
	$(AVR_SIZE) --format=avr --mcu=$(MCU) $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

test-host: | $(BUILD_DIR)
	gcc -std=c2x -Iinclude -Wall -Wextra -Werror -pedantic -Wconversion -Wsign-conversion -O2 \
	    src/crc16.c tests/test_crc16.c -o $(BUILD_DIR)/test_crc16
	./$(BUILD_DIR)/test_crc16

clean:
	rm -rf $(BUILD_DIR) $(TARGET).elf $(TARGET).hex
