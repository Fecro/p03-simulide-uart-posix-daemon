#!/usr/bin/env bash
set -euo pipefail

PROJ_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CROSS_COMPILE="${CROSS_COMPILE:-arm-linux-gnueabihf-}"
JOBS="${JOBS:-$(nproc)}"
KERNEL_TAG="v6.12.111"

echo "=== [Step 3/7] Building Linux Kernel (${KERNEL_TAG} LTS) & DTB ==="
mkdir -p "${PROJ_ROOT}/sources" "${PROJ_ROOT}/kernel" "${PROJ_ROOT}/images"

if [ ! -d "${PROJ_ROOT}/sources/linux-lts" ]; then
  git clone --depth=1 --branch "${KERNEL_TAG}" \
    git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git \
    "${PROJ_ROOT}/sources/linux-lts"
fi

if [ ! -f "${PROJ_ROOT}/kernel/arch/arm/boot/zImage" ]; then
  cd "${PROJ_ROOT}/sources/linux-lts"
  make O="${PROJ_ROOT}/kernel" ARCH=arm CROSS_COMPILE="${CROSS_COMPILE}" vexpress_defconfig

  ./scripts/config --file "${PROJ_ROOT}/kernel/.config" \
    --enable CONFIG_BLK_DEV_INITRD \
    --enable CONFIG_RD_GZIP \
    --enable CONFIG_DEVTMPFS \
    --enable CONFIG_DEVTMPFS_MOUNT \
    --enable CONFIG_PROC_FS \
    --enable CONFIG_SYSFS \
    --enable CONFIG_EPOLL \
    --enable CONFIG_SERIAL_AMBA_PL011 \
    --enable CONFIG_SERIAL_AMBA_PL011_CONSOLE

  make O="${PROJ_ROOT}/kernel" ARCH=arm CROSS_COMPILE="${CROSS_COMPILE}" olddefconfig
  make O="${PROJ_ROOT}/kernel" ARCH=arm CROSS_COMPILE="${CROSS_COMPILE}" -j"${JOBS}" zImage dtbs
fi

cp -v "${PROJ_ROOT}/kernel/arch/arm/boot/zImage" "${PROJ_ROOT}/images/zImage"
if [ -f "${PROJ_ROOT}/kernel/arch/arm/boot/dts/arm/vexpress-v2p-ca9.dtb" ]; then
  cp -v "${PROJ_ROOT}/kernel/arch/arm/boot/dts/arm/vexpress-v2p-ca9.dtb" \
    "${PROJ_ROOT}/images/vexpress-v2p-ca9.dtb"
else
  cp -v "${PROJ_ROOT}/kernel/arch/arm/boot/dts/vexpress-v2p-ca9.dtb" \
    "${PROJ_ROOT}/images/vexpress-v2p-ca9.dtb"
fi

echo "[OK] Kernel zImage and DTB staged in ${PROJ_ROOT}/images/"
