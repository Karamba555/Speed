include $(TOPDIR)/rules.mk

PKG_NAME:=libatel
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
#PKG_KCONFIG:=RALINK_MT7620 RALINK_MT7621 RALINK_MT7628
#PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/$(PKG_NAME)
  SECTION:=Atel Properties
  CATEGORY:=Atel Properties
  TITLE:= lib atel 
  DEPENDS:=+libopenssl +libnvram
endef

define Package/$(PKG_NAME)/description
  lib atel
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(call Build/Compile/Default)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/include/libatel $(1)/usr/include/libatel/upgrade
	$(CP) $(PKG_BUILD_DIR)/encrypt_api.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/utils.h $(1)/usr/include/libatel
	$(CP) $(PKG_BUILD_DIR)/upgrade/*.h $(1)/usr/include/libatel/upgrade
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/libatel.so $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libatel.so $(1)/usr/lib
endef


$(eval $(call BuildPackage,$(PKG_NAME)))

