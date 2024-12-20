# Build commands that can be called from Device/* templates

IMAGE_KERNEL = $(word 1,$^)
IMAGE_ROOTFS = $(word 2,$^)
ifeq (y,${CONFIG_TARGET_mediatek_filogic_DEVICE_ATEL-SW120})
IMAGE_INITRAMFS = $(KDIR)/tmp/openwrt-mediatek-filogic-ATEL-SW120-squashfs-factory.bin
IMAGE_SYSUPGRADE = $(KDIR)/tmp/openwrt-mediatek-filogic-ATEL-SW120-squashfs-sysupgrade.bin
else
IMAGE_INITRAMFS = $(KDIR)/tmp/$(KERNEL_INITRAMFS_IMAGE)
endif

define Build/atel_nand
	dd if=$(IMAGE_KERNEL) >> $@
	dd if=$(IMAGE_ROOTFS) >> $@
	$(STAGING_DIR_HOST)/bin/padjffs2 $@ $(1) $(if $(BLOCKSIZE),$(BLOCKSIZE:%k=%),4 8 16 64 128 256)
	@rm -rf $(BIN_DIR)/* $(TOPDIR)/image
	@sleep 3
	mkdir -p $(TOPDIR)/image
	$(CP) $(IMAGE_INITRAMFS) $(TOPDIR)/image/Speedway_Factory_$(UIMAGE_NAME).bin
        $(CP) $(IMAGE_SYSUPGRADE) $(TOPDIR)/image/Speedway_Sysupgrade_$(UIMAGE_NAME).bin
	touch $(TOPDIR)/image/Package_info.txt
	echo "Latest Version:$(UIMAGE_NAME);" >> $(TOPDIR)/image/Package_info.txt
	echo "LTE Version:Disable;" >>$(TOPDIR)/image/Package_info.txt
	echo "Allow List:Disable;" >> $(TOPDIR)/image/Package_info.txt
	echo "Allow SingleList:Disable;" >> $(TOPDIR)/image/Package_info.txt
	echo "Start Version:Disable;" >>$(TOPDIR)/image/Package_info.txt
	echo "Start LTE Version:Disable;" >>$(TOPDIR)/image/Package_info.txt
	echo "Check Interval:24;" >>$(TOPDIR)/image/Package_info.txt
	ls -ll $(TOPDIR)/image/$(UIMAGE_NAME) 2>/dev/null | awk '{print int($$5)}' > $(TOPDIR)/image/releaseNote_$(UIMAGE_NAME).txt
endef

ifneq (y,${CONFIG_SECURITY_SIGN_ENCRYPT})

#this is normal build image 
define Build/atel_default
	dd if=$(IMAGE_KERNEL) >> $@
	dd if=$(IMAGE_ROOTFS) >> $@
	$(STAGING_DIR_HOST)/bin/padjffs2 $@ $(1) $(if $(BLOCKSIZE),$(BLOCKSIZE:%k=%),4 8 16 64 128 256)
        ###rm -rf $(TOPDIR)/image/CPEWT* $(IMAGE_INITRAMFS).data*  $(TOPDIR)/image/releaseNote*
        ##@sleep 3
        mkdir -p $(TOPDIR)/image
        $(TOPDIR)/build/Package_info.sh $(TOPDIR) $(UIMAGE_NAME)
        ls -ll $(TOPDIR)/image/$(UIMAGE_NAME) 2>/dev/null | awk '{print int($$5)}' > $(TOPDIR)/image/releaseNote_$(UIMAGE_NAME).txt
        [ ! -e $(TOPDIR)/image/uboot.bin ] || $(CP) $(IMAGE_INITRAMFS) $(TOPDIR)/image/$(UIMAGE_NAME_STRING)_U.bin  
        [ ! -e $(TOPDIR)/image/$(UIMAGE_NAME_STRING)_U.bin ] || dd if=$(TOPDIR)/image/uboot.bin >> $(TOPDIR)/image/$(UIMAGE_NAME_STRING)_U.bin


endef
#end normall build image

else
#this is build image with aes_encrypt and rsa_verify 
#please enable CONFIG_SECURITY_SIGN_ENCRYPT and enable ATEL_SECURITY_SIGN_ENCRYPT from uboot , update uboot to support uboot on
define Build/atel_default
	dd if=$(IMAGE_KERNEL) >> $@
	dd if=$(IMAGE_ROOTFS) >> $@
	$(STAGING_DIR_HOST)/bin/padjffs2 $@ $(1) $(if $(BLOCKSIZE),$(BLOCKSIZE:%k=%),4 8 16 64 128 256)
        ##rm -rf $(BIN_DIR)/* $(TOPDIR)/image/CPEWT* $(IMAGE_INITRAMFS).data*  $(TOPDIR)/image/releaseNote*
        ##sleep 3
        mkdir -p $(TOPDIR)/image
        dd if=$(IMAGE_INITRAMFS) of=$(IMAGE_INITRAMFS).se count=1 bs=64
        dd if=$(IMAGE_INITRAMFS) of=$(IMAGE_INITRAMFS).data bs=1 skip=64 
        cp $(TOPDIR)/package/atel/rsa_verify_vboot/futility $(KDIR)
        cp $(TOPDIR)/package/atel/rsa_verify_vboot/myprivate.vbprik2 $(KDIR)
        $(KDIR)/futility sign --type rwsig --prikey $(KDIR)/myprivate.vbprik2 $(IMAGE_INITRAMFS).data $(IMAGE_INITRAMFS).data.sig

        echo $(UIMAGE_NAME) |cut -c1-26 >> $(IMAGE_INITRAMFS).data
	dd if=$(IMAGE_INITRAMFS).data.sig >> $(IMAGE_INITRAMFS).data
        $(TOPDIR)/package/atel/aes_encrypt/src/aes $(IMAGE_INITRAMFS).data $(IMAGE_INITRAMFS).data.aes

        $(call MkImage,lzma,$(IMAGE_INITRAMFS).data.aes,$(IMAGE_INITRAMFS).data.aes.uimage,$(UIMAGE_NAME))
        $(CP) $(IMAGE_INITRAMFS).data.aes.uimage $(TOPDIR)/image/$(UIMAGE_NAME)

        $(TOPDIR)/build/Package_info.sh $(TOPDIR) $(UIMAGE_NAME)
        ls -ll $(TOPDIR)/image/$(UIMAGE_NAME)  2>/dev/null | awk '{print int($$5)}' > $(TOPDIR)/image/releaseNote_$(UIMAGE_NAME).txt

        [ ! -e $(TOPDIR)/image/uboot.bin ] || $(CP) $(TOPDIR)/image/$(UIMAGE_NAME) $(TOPDIR)/image/$(UIMAGE_NAME_STRING)_U.bin  
        [ ! -e $(TOPDIR)/image/$(UIMAGE_NAME_STRING)_U.bin ] || dd if=$(TOPDIR)/image/uboot.bin >> $(TOPDIR)/image/$(UIMAGE_NAME_STRING)_U.bin

endef
#end build security image


endif
