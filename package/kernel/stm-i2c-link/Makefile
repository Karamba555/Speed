#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=stm-i2c-link
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

SOURCE_DIR:=$(PWD)/package/kernel/stm-i2c-link/src

define KernelPackage/stm-i2c-link
  SECTION:=STM32 Properties
  CATEGORY:=STM32 Properties
  TITLE:=STM32 I2C Link Driver
  SUBMENU:=Drivers
  FILES:=$(PKG_BUILD_DIR)/stm-i2c-link.ko
  AUTOLOAD:=$(call AutoLoad,81,stm-i2c-link)
endef

define KernelPackage/stm-i2c-link/description
  STM I2C Link driver
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp -r $(SOURCE_DIR)/* $(PKG_BUILD_DIR)
endef

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,stm-i2c-link))
