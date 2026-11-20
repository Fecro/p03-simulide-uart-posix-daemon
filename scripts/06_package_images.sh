#!/usr/bin/env bash
set -euo pipefail
export PATH="/usr/sbin:/sbin:${PATH}"

PROJ_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "=== [Step 6/7] Packaging Initramfs, U-Boot boot.scr & MBR sdcard.img ==="
mkdir -p "${PROJ_ROOT}/images"

# 1. Generate initramfs.cpio.gz with root:root ownership (zero sudo required)
cd "${PROJ_ROOT}/rootfs"
find . -print0 | cpio --null -ov --format=newc --owner=+0:+0 | gzip -9 \
  >"${PROJ_ROOT}/images/initramfs.cpio.gz"

mkimage -A arm -O linux -T ramdisk -C gzip -d \
  "${PROJ_ROOT}/images/initramfs.cpio.gz" \
  "${PROJ_ROOT}/images/uInitramfs"

# 2. Generate U-Boot automatic boot script (boot.cmd -> boot.scr)
cat <<'BOOTCMD' >"${PROJ_ROOT}/images/boot.cmd"
echo "=== [U-Boot Project 2] Loading Kernel, DTB & uInitramfs from SD (mmc 0:1) ==="
setenv bootargs "console=ttyAMA0,115200 rdinit=/sbin/init loglevel=7"
setenv kernel_addr_r 0x60100000
setenv fdt_addr_r 0x62000000
setenv ramdisk_addr_r 0x63000000

fatload mmc 0:1 ${kernel_addr_r} zImage
fatload mmc 0:1 ${fdt_addr_r} vexpress-v2p-ca9.dtb
fatload mmc 0:1 ${ramdisk_addr_r} uInitramfs

bootz ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr_r}
BOOTCMD

mkimage -A arm -O linux -T script -C none -a 0 -e 0 \
  -n "Vexpress-A9 Dual-UART Boot Script" \
  -d "${PROJ_ROOT}/images/boot.cmd" \
  "${PROJ_ROOT}/images/boot.scr"

# 3. Build 64 MiB MBR-partitioned SD card image (mmc 0:1) rootlessly
cd "${PROJ_ROOT}/images"
rm -f part.img sdcard.img

dd if=/dev/zero of=part.img bs=1M count=63
mkfs.vfat -F 32 -n "BOOT" part.img

mcopy -i part.img zImage ::/zImage
mcopy -i part.img vexpress-v2p-ca9.dtb ::/vexpress-v2p-ca9.dtb
mcopy -i part.img uInitramfs ::/uInitramfs
mcopy -i part.img boot.scr ::/boot.scr

dd if=/dev/zero of=sdcard.img bs=1M count=1
cat part.img >>sdcard.img
rm -f part.img

printf "label: dos\n2048,,c,*\n" | sfdisk sdcard.img

echo "-> Verifying MBR partition 1 contents (offset 1MiB):"
mdir -i sdcard.img@@1M ::/
echo "[OK] MBR-partitioned SD image ready at: ${PROJ_ROOT}/images/sdcard.img"
