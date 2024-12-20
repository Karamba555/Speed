
-include $(INCLUDE_DIR)/version.mk
-include $(INCLUDE_DIR)/kernel.mk
-include $(LINUX_DIR)/.config

curdir:=build

BASE_FILES:=$(TOPDIR)/build/base-files

$(curdir)/install: $(TMP_DIR)/.build
	if [[ -d $(BASE_FILES) ]]; then \
		echo "Overwrite product rootfs."; \
		date > $(TARGET_DIR)/etc_ro/build_time; \
		cp -fpR $(BASE_FILES)/. $(TARGET_DIR); \
		/usr/bin/openssl enc -aes-256-cbc -e -a -in $(BASE_FILES)/etc_ro/nvram_2860.dft -out $(TARGET_DIR)/etc_ro/nvram_2860.dft -K 6638626565333461316363333864386337653361376266343165386665613832 -iv 32386561666332333033636263306363; \
		/usr/bin/openssl enc -aes-256-cbc -e -a -in $(BASE_FILES)/etc_ro/nvram_rtdev.dft -out $(TARGET_DIR)/etc_ro/nvram_rtdev.dft -K 6638626565333461316363333864386337653361376266343165386665613832 -iv 32386561666332333033636263306363; \
	fi
	#if [[ -f $(TOPDIR)/Version ]]; then \
	#	cp -fpR $(TOPDIR)/Version $(TARGET_DIR)/etc_ro/lighttpd/www/idu/; \
	#fi
	
	$(TOPDIR)/build/nextivity-files/nextivity.sh

	#if [ "$(CONFIG_MTK_CHIP_MT7981)" == "y" ]; then #wpeng move eeprom to package/mtk/driver/bin/
		#mkdir -p $(TARGET_DIR)/lib/firmware/;
		#cp -f $(BASE_FILES)/lib/firmware/MT7981_iPAiLNA_EEPROM.bin $(TARGET_DIR)/lib/firmware/;
	#fi


ifeq ($(CONFIG_SECURITY_APPARMOR_WHILE_LIST),y)
$(curdir)/modify: $(TMP_DIR)/.build
	touch $(TOPDIR)/tmp/.md5flag
	$(TOPDIR)/build/md5_generate.sh $(TARGET_DIR) $(TARGET_DIR_ORIG) $(LINUX_DIR)/security/apparmor/lsm.c
else
$(curdir)/modify: $(TMP_DIR)/.build
endif

$(eval $(call stampfile,$(curdir),build,install,$(TMP_DIR)/.build))
$(eval $(call stampfile,$(curdir),build,modify,$(TMP_DIR)/.build))
