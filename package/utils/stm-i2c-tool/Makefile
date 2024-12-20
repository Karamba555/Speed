#
# Copyright (C) 2021 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=stm-i2c-tool
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

SOURCE_DIR:=$(PWD)/package/utils/stm-i2c-tool/src

define Package/stm-i2c-tool
  SECTION:=STM32 Properties
  CATEGORY:=STM32 Properties
  TITLE:=STM32 I2C Tool
  SUBMENU:=Tools
endef

define Package/stm-i2c-tool/description
  STM32 I2C Tool
endef

TARGET_CFLAGS += \
	-D_GNU_SOURCE \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(PKG_BUILD_DIR)

# Package preparation instructions; create the build directory and copy the source code. 
# The last command is necessary to ensure our preparation instructions
# remain compatible with the patching system.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp $(SOURCE_DIR)/* $(PKG_BUILD_DIR)
	$(Build/Patch)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS) -lm"
endef

define Package/stm-i2c-tool/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/stm-i2c-tool $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,stm-i2c-tool))
