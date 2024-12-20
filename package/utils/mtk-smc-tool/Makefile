#
# Copyright (C) 2021 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtk-smc-tool
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/mtk-smc-tool
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=MTK SMC tool
  SUBMENU:=Applications
  DEPENDS:=+libnl-tiny +kmod-mtk-smc-drv
endef

define Package/mtk-smc-tool/description
  MTK SMC tool
endef

define Build/Configure
endef

TARGET_CFLAGS += \
	-D_GNU_SOURCE \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(PKG_BUILD_DIR)

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS) -lnl-tiny -lm"
endef

define Package/mtk-smc-tool/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mtk-smc-tool $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,mtk-smc-tool))
