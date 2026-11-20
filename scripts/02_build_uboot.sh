#!/usr/bin/env bash
set -euo pipefail

PROJ_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CROSS_COMPILE="${CROSS_COMPILE:-arm-linux-gnueabihf-}"
JOBS="${JOBS:-$(nproc)}"
UBOOT_VER="v2025.10"

echo "=== [Step 2/7] Building Das U-Boot (${UBOOT_VER}) for vexpress_ca9x4 ==="
mkdir -p "${PROJ_ROOT}/sources" "${PROJ_ROOT}/Bootloader"

if [ ! -d "${PROJ_ROOT}/sources/u-boot" ]; then
  git clone --depth=1 --branch "${UBOOT_VER}" \
    https://source.denx.de/u-boot/u-boot.git "${PROJ_ROOT}/sources/u-boot"
fi

if [ ! -f "${PROJ_ROOT}/Bootloader/u-boot" ]; then
  cd "${PROJ_ROOT}/sources/u-boot"
  make O="${PROJ_ROOT}/Bootloader" ARCH=arm CROSS_COMPILE="${CROSS_COMPILE}" \
    vexpress_ca9x4_defconfig

  ./scripts/config --file "${PROJ_ROOT}/Bootloader/.config" \
    --disable CONFIG_ENV_IS_IN_FLASH \
    --enable CONFIG_ENV_IS_IN_FAT \
    --set-str CONFIG_ENV_FAT_INTERFACE "mmc" \
    --set-str CONFIG_ENV_FAT_DEVICE_AND_PART "0:1" \
    --set-str CONFIG_ENV_FAT_FILE "uboot.env" \
    --enable CONFIG_USE_BOOTCOMMAND \
    --set-str CONFIG_BOOTCOMMAND "fatload mmc 0:1 0x60000000 boot.scr; source 0x60000000"

  make O="${PROJ_ROOT}/Bootloader" ARCH=arm CROSS_COMPILE="${CROSS_COMPILE}" olddefconfig
  make O="${PROJ_ROOT}/Bootloader" ARCH=arm CROSS_COMPILE="${CROSS_COMPILE}" -j"${JOBS}"
fi

echo "[OK] U-Boot ready at: ${PROJ_ROOT}/Bootloader/u-boot"
