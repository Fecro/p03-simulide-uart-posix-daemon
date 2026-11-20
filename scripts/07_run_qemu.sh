#!/usr/bin/env bash
set -euo pipefail

PROJ_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MODE="${1:-uboot}"
UART1_SOCK="${UART1_SOCK:-/tmp/qemu_uart1.sock}"
SIMULIDE_PTY="${SIMULIDE_PTY:-/tmp/ttySIMULIDE}"

rm -f "${UART1_SOCK}" "${SIMULIDE_PTY}"

# Auto-launch socat bridge in background once QEMU creates the UNIX socket
SOCAT_PID=""
cleanup() {
  if [ -n "${SOCAT_PID}" ]; then
    kill "${SOCAT_PID}" 2>/dev/null || true
  fi
  rm -f "${UART1_SOCK}" "${SIMULIDE_PTY}"
}
trap cleanup EXIT INT TERM

(
  for _ in $(seq 1 50); do
    if [ -S "${UART1_SOCK}" ]; then
      exec socat pty,raw,echo=0,link="${SIMULIDE_PTY}" "UNIX-CONNECT:${UART1_SOCK}" >/dev/null 2>&1
    fi
    sleep 0.1
  done
) &
SOCAT_PID=$!

echo "=========================================================================="
echo " Launching QEMU ARM Versatile Express Cortex-A9 (Mode: ${MODE})"
echo " UART0 (/dev/ttyAMA0): Interactive Linux Console (stdio)"
echo " UART1 (/dev/ttyAMA1): UNIX Socket (${UART1_SOCK}) <-> ${SIMULIDE_PTY}"
echo " SimulIDE Ready      : Open circuit/sensor_node.sim1 -> Click Open & Power"
echo " Tip: Press Ctrl+A, release, then press X to exit QEMU cleanly."
echo "=========================================================================="

if [ "${MODE}" = "direct" ]; then
  qemu-system-arm \
    -machine vexpress-a9 \
    -cpu cortex-a9 \
    -smp 4 \
    -m 512M \
    -nographic \
    -audio none \
    -serial mon:stdio \
    -chardev "socket,id=uart1,path=${UART1_SOCK},server=on,wait=off" \
    -serial chardev:uart1 \
    -kernel "${PROJ_ROOT}/images/zImage" \
    -dtb "${PROJ_ROOT}/images/vexpress-v2p-ca9.dtb" \
    -initrd "${PROJ_ROOT}/images/initramfs.cpio.gz" \
    -append "console=ttyAMA0,115200 rdinit=/sbin/init loglevel=7"
else
  qemu-system-arm \
    -machine vexpress-a9 \
    -cpu cortex-a9 \
    -m 512M \
    -nographic \
    -audio none \
    -serial mon:stdio \
    -chardev "socket,id=uart1,path=${UART1_SOCK},server=on,wait=off" \
    -serial chardev:uart1 \
    -kernel "${PROJ_ROOT}/Bootloader/u-boot" \
    -drive if=sd,format=raw,file="${PROJ_ROOT}/images/sdcard.img"
fi
