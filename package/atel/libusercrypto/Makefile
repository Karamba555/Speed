include $(TOPDIR)/rules.mk

PKG_NAME:=libusercrypto
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
#PKG_KCONFIG:=RALINK_MT7620 RALINK_MT7621 RALINK_MT7628
#PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/$(PKG_NAME)
  SECTION:=Atel Properties
  CATEGORY:=Atel Properties
  TITLE:= lib usercrypto 
  DEPENDS:=+libnvram
endef

define Package/$(PKG_NAME)/description
  lib usercrypto
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
	$(INSTALL_DIR) $(1)/usr/include/usercrypto
	$(CP) $(PKG_BUILD_DIR)/aes.h $(1)/usr/include/usercrypto/
	$(CP) $(PKG_BUILD_DIR)/debug.h $(1)/usr/include/usercrypto/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/libusercrypto.so $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libusercrypto.so $(1)/usr/lib
endef


$(eval $(call BuildPackage,$(PKG_NAME)))

