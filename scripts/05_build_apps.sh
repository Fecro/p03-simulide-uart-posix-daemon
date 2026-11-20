#!/usr/bin/env bash
set -euo pipefail

PROJ_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CROSS_COMPILE="${CROSS_COMPILE:-arm-linux-gnueabihf-}"

echo "=== [Step 5/7] Building AVR Firmware & C23 POSIX Collector ==="

echo "-> Building bare-metal ATmega328P firmware (sensor_node.hex)..."
make -C "${PROJ_ROOT}/firmware_avr" clean all

echo "-> Running Host unit tests (ASan + UBSan) for uart_collector_d..."
mkdir -p "${PROJ_ROOT}/rootfs/usr/bin"
make -C "${PROJ_ROOT}/user/uart_collector" test

echo "-> Cross-compiling static ARMv7 uart_collector_d into RootFS..."
make -C "${PROJ_ROOT}/user/uart_collector" \
  CROSS_COMPILE="${CROSS_COMPILE}" \
  PROJ_ROOT="${PROJ_ROOT}" \
  install-rootfs

file "${PROJ_ROOT}/rootfs/usr/bin/uart_collector_d"
echo "[OK] Firmware HEX & ARMv7 uart_collector_d ready."
