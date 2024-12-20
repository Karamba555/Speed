
include $(TOPDIR)/rules.mk

PKG_NAME:=mgui
PKG_RELEASE:=1
CMAKE_INSTALL:=1
PKG_FLAGS:=nonshared

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=Atel Properties
  CATEGORY:=Atel Properties
  TITLE:= atel mgui
  DEPENDS:=+libuci +libubox +libubus +libblobmsg-json
endef

define Package/$(PKG_NAME)/description
  mgui app
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) $(TOPDIR)/user_conf.h $(STAGING_DIR)/usr/include/
endef

#define Build/Configure
#endef

#define Build/Compile
#	$(call Build/Compile/Default)
#endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr $(1)/usr/bin $(1)/usr/share $(1)/usr/share/mgui
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/bin
	$(CP) -rf $(PKG_BUILD_DIR)/data/images $(1)/usr/share/mgui
endef


$(eval $(call BuildPackage,$(PKG_NAME)))

