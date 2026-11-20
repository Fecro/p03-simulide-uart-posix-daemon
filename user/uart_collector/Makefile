# Makefile for uart_collector_d (Host Sanitizer, Unit Tests, and ARMv7 Static Cross-Build)

CROSS_COMPILE ?=
CC            := $(CROSS_COMPILE)gcc
STRIP         := $(CROSS_COMPILE)strip

CSTD_FLAG     := $(shell $(CC) -std=c23 -x c -E /dev/null >/dev/null 2>&1 && echo "-std=c23" || echo "-std=c2x")

CFLAGS        := $(CSTD_FLAG) -O2 -Iinclude -Wall -Wextra -Werror -pedantic \
                 -Wformat=2 -Wconversion -Wsign-conversion \
                 -Wnull-dereference -Wshadow -Wstrict-prototypes \
                 -fstack-protector-strong -D_FORTIFY_SOURCE=2
LDFLAGS       :=

ifeq ($(STATIC),1)
    LDFLAGS += -static
endif

BUILD_DIR     := build
TARGET        := $(BUILD_DIR)/uart_collector_d
SRCS          := src/main.c src/serial_port.c src/frame_parser.c src/ring_buffer.c
OBJS          := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))
HDRS          := include/c23_compat.h include/ring_buffer.h \
                 include/frame_parser.h include/serial_port.h

.PHONY: all test sanitize install-rootfs clean

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: src/%.c $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

test: | $(BUILD_DIR)
	$(CC) $(CSTD_FLAG) -O1 -g3 -Iinclude -Wall -Wextra -Werror -pedantic \
	      -Wconversion -Wsign-conversion -Wnull-dereference \
	      -fsanitize=address,undefined -fno-omit-frame-pointer \
	      src/ring_buffer.c tests/test_ring_buffer.c -o $(BUILD_DIR)/test_ring_buffer
	./$(BUILD_DIR)/test_ring_buffer
	$(CC) $(CSTD_FLAG) -O1 -g3 -Iinclude -Wall -Wextra -Werror -pedantic \
	      -Wconversion -Wsign-conversion -Wnull-dereference \
	      -fsanitize=address,undefined -fno-omit-frame-pointer \
	      src/ring_buffer.c src/frame_parser.c tests/test_parser.c -o $(BUILD_DIR)/test_parser
	./$(BUILD_DIR)/test_parser

sanitize: clean | $(BUILD_DIR)
	$(CC) $(CSTD_FLAG) -O1 -g3 -Iinclude -Wall -Wextra -Werror -pedantic \
	      -Wformat=2 -Wconversion -Wsign-conversion -Wnull-dereference \
	      -fsanitize=address,undefined -fno-omit-frame-pointer \
	      $(SRCS) -o $(BUILD_DIR)/uart_collector_d_san

install-rootfs: clean
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) STATIC=1 all
	$(STRIP) $(TARGET)
	install -m 0755 $(TARGET) $(PROJ_ROOT)/rootfs/usr/bin/uart_collector_d

clean:
	rm -rf $(BUILD_DIR)
