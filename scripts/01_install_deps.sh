#!/usr/bin/env bash
set -euo pipefail
export PATH="/usr/sbin:/sbin:${PATH}"

echo "=== [Step 1/7] Checking Host, AVR & ARM Cross-Compilation Dependencies ==="

REQUIRED_CMDS=(
  arm-linux-gnueabihf-gcc
  avr-gcc
  qemu-system-arm
  mkimage
  mkfs.vfat
  mcopy
  sfdisk
  socat
)

MISSING=0
for cmd in "${REQUIRED_CMDS[@]}"; do
  if ! command -v "${cmd}" >/dev/null 2>&1; then
    echo "[INFO] Missing tool: ${cmd}"
    MISSING=1
  fi
done

if [ "${MISSING}" -eq 1 ]; then
  sudo apt-get update
  sudo apt-get install -y \
    build-essential git wget curl bc bison flex libssl-dev \
    libncurses-dev libelf-dev dwarves device-tree-compiler \
    u-boot-tools cpio gzip bzip2 xz-utils fdisk dosfstools mtools \
    qemu-system-arm gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf \
    gcc-avr avr-libc binutils-avr socat gdb-multiarch valgrind cppcheck
else
  echo "[OK] All required host tools are already installed (skipping apt)."
fi
