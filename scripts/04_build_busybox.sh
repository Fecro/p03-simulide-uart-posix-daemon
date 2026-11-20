#!/usr/bin/env bash
set -euo pipefail

PROJ_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CROSS_COMPILE="${CROSS_COMPILE:-arm-linux-gnueabihf-}"
JOBS="${JOBS:-$(nproc)}"
BB_VER="1.37.0"

echo "=== [Step 4/7] Building Static BusyBox (${BB_VER}) for ARMv7 ==="
mkdir -p "${PROJ_ROOT}/sources" "${PROJ_ROOT}/rootfs"

if [ ! -d "${PROJ_ROOT}/sources/busybox-${BB_VER}" ]; then
  cd "${PROJ_ROOT}/sources"
  wget -nc "https://busybox.net/downloads/busybox-${BB_VER}.tar.bz2"
  tar -xf "busybox-${BB_VER}.tar.bz2"
fi

if [ ! -f "${PROJ_ROOT}/rootfs/bin/busybox" ]; then
  cd "${PROJ_ROOT}/sources/busybox-${BB_VER}"
  make ARCH=arm CROSS_COMPILE="${CROSS_COMPILE}" defconfig

  sed -i 's/# CONFIG_STATIC is not set/CONFIG_STATIC=y/' .config
  sed -i 's/^CONFIG_TC=y/# CONFIG_TC is not set/' .config
  sed -i 's/^CONFIG_SHA1_HWACCEL=y/# CONFIG_SHA1_HWACCEL is not set/' .config
  sed -i 's/^CONFIG_SHA256_HWACCEL=y/# CONFIG_SHA256_HWACCEL is not set/' .config

  make ARCH=arm CROSS_COMPILE="${CROSS_COMPILE}" -j"${JOBS}"
  make ARCH=arm CROSS_COMPILE="${CROSS_COMPILE}" CONFIG_PREFIX="${PROJ_ROOT}/rootfs" install
fi

cd "${PROJ_ROOT}/rootfs"
mkdir -p dev proc sys etc etc/init.d tmp var run root usr/bin usr/sbin

cat <<'INITTAB' >"${PROJ_ROOT}/rootfs/etc/inittab"
::sysinit:/etc/init.d/rcS
ttyAMA0::respawn:-/bin/sh
::ctrlaltdel:/sbin/reboot
::shutdown:/bin/umount -a -r
INITTAB

cat <<'RCS' >"${PROJ_ROOT}/rootfs/etc/init.d/rcS"
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev
mkdir -p /dev/pts
mount -t devpts none /dev/pts

echo "=================================================================="
echo " Embedded Linux Portfolio - Project 2 (SimulIDE + QEMU Dual-UART) "
echo " Kernel: $(uname -srm)"
echo " Board : $(cat /proc/device-tree/model 2>/dev/null || echo 'vexpress-a9')"
echo "=================================================================="
echo " Run 'uart_collector_d /dev/ttyAMA1' to receive SimulIDE frames."
echo "=================================================================="
RCS
chmod +x "${PROJ_ROOT}/rootfs/etc/init.d/rcS"

echo "[OK] BusyBox RootFS staged at: ${PROJ_ROOT}/rootfs"
