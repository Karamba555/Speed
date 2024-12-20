ARCH:=aarch64
SUBTARGET:=filogic
BOARDNAME:=Filogic 8x0 (MT798x)
CPU_TYPE:=cortex-a53
FEATURES:=squashfs nand ramdisk
DEFAULT_PACKAGES += kmod-phy-aquantia kmod-crypto-hw-safexcel kmod-mt7915e wpad-basic-mbedtls uboot-envtools
KERNELNAME:=Image dtbs

define Target/Description
	Build firmware images for MediaTek Filogic ARM based boards.
endef
