include $(TOPDIR)/rules.mk

PKG_NAME:=libnvram
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
#PKG_KCONFIG:=RALINK_MT7620 RALINK_MT7621 RALINK_MT7628
#PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/$(PKG_NAME)
  SECTION:=Atel Properties
  CATEGORY:=Atel Properties
  TITLE:= lib nvram
  DEPENDS:=+zlib +libopenssl
endef

define Package/$(PKG_NAME)/description
  lib nvram
endef

define Build/Prepare
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include
	$(CP) $(TOPDIR)/user_conf.h $(STAGING_DIR)/usr/include/
	$(CP) $(TOPDIR)/linux_conf.h $(STAGING_DIR)/usr/include/
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(call Build/Compile/Default)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/nvram.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/libnvram-0.9.28.so $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/libcrypto.so.1.1 $(1)/usr/lib
	ln -s ./libnvram-0.9.28.so $(1)/usr/lib/libnvram.so
	ln -s ./libnvram-0.9.28.so $(1)/usr/lib/libnvram.so.0
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libnvram-0.9.28.so $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libcrypto.so.1.1 $(1)/usr/lib
	ln -s ./libnvram-0.9.28.so $(1)/usr/lib/libnvram.so
	ln -s ./libnvram-0.9.28.so $(1)/usr/lib/libnvram.so.0
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

