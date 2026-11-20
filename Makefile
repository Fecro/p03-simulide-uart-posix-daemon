# =============================================================================
# Master Orchestration Makefile - Project 02: SimulIDE + QEMU Dual-UART Daemon
# Usage:
#   make bridge    -> Start socat PTY bridge (/tmp/ttySIMULIDE <-> /tmp/ttyQEMU)
#   make demo      -> Full build + Launch QEMU (U-Boot -> Linux -> Dual-UART)
#   make direct    -> Full build + Launch QEMU Direct Kernel Boot (4-core SMP)
#   make build     -> Run all build & packaging stages without launching QEMU
#   make clean     -> Remove compiled app/firmware binaries and generated images
#   make distclean -> Remove all downloaded upstream sources and build trees
# =============================================================================

SHELL := /usr/bin/env bash

.PHONY: all demo direct bridge build deps uboot kernel busybox apps images run run-direct clean distclean

all: build

demo: deps build run

direct: deps build run-direct

bridge:
	@echo "Starting socat virtual null-modem: /tmp/ttySIMULIDE <-> /tmp/ttyQEMU"
	socat -d -d pty,raw,echo=0,link=/tmp/ttySIMULIDE UNIX-CONNECT:/tmp/qemu_uart1.sock

build: uboot kernel busybox apps images

deps:
	./scripts/01_install_deps.sh

uboot:
	./scripts/02_build_uboot.sh

kernel:
	./scripts/03_build_kernel.sh

busybox:
	./scripts/04_build_busybox.sh

apps:
	./scripts/05_build_apps.sh

images:
	./scripts/06_package_images.sh

run:
	./scripts/07_run_qemu.sh uboot

run-direct:
	./scripts/07_run_qemu.sh direct

clean:
	$(MAKE) -C firmware_avr clean
	$(MAKE) -C user/uart_collector clean
	rm -rf images/

distclean: clean
	rm -rf sources/ Bootloader/ kernel/ rootfs/

test-inject:
	@echo "Injecting noise + Golden Test Vector (ADC=512, 2.502V) into /tmp/qemu_uart1.sock..."
	@printf '\xff\x00\xaa\x55\x01\x00\x00\x02\xa9\x09' | socat - UNIX-CONNECT:/tmp/qemu_uart1.sock
	@echo "[OK] Frame injected. Check QEMU console for [RX #00001]."
