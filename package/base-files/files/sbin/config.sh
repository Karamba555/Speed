CONFIG_MODULES=y
CONFIG_HAVE_DOT_CONFIG=y
CONFIG_DEFAULT_TARGET_mediatek_mt7986=y
CONFIG_TARGET_mediatek=y
CONFIG_TARGET_mediatek_mt7981=y
CONFIG_TARGET_mediatek_mt7981_DEVICE_ATEL_HS600=y
CONFIG_HAS_SUBTARGETS=y
CONFIG_HAS_DEVICES=y
CONFIG_TARGET_BOARD="mediatek"
CONFIG_TARGET_SUBTARGET="mt7981"
CONFIG_TARGET_PROFILE="DEVICE_ATEL_HS600"
CONFIG_TARGET_ARCH_PACKAGES="aarch64_cortex_a53"
CONFIG_DEFAULT_TARGET_OPTIMIZATION="_Os _pipe _mcpu=cortex_a53"
CONFIG_CPU_TYPE="cortex_a53"
CONFIG_LINUX_5_4=y
CONFIG_DEFAULT_base_files=y
CONFIG_DEFAULT_busybox=y
CONFIG_DEFAULT_ca_bundle=y
CONFIG_DEFAULT_dnsmasq=y
CONFIG_DEFAULT_dropbear=y
CONFIG_DEFAULT_fstools=y
CONFIG_DEFAULT_kmod_gpio_button_hotplug=y
CONFIG_DEFAULT_kmod_ipt_offload=y
CONFIG_DEFAULT_kmod_leds_gpio=y
CONFIG_DEFAULT_libc=y
CONFIG_DEFAULT_libgcc=y
CONFIG_DEFAULT_libustream_wolfssl=y
CONFIG_DEFAULT_logd=y
CONFIG_DEFAULT_mtd=y
CONFIG_DEFAULT_netifd=y
CONFIG_DEFAULT_odhcp6c=y
CONFIG_DEFAULT_odhcpd_ipv6only=y
CONFIG_DEFAULT_opkg=y
CONFIG_DEFAULT_ppp=y
CONFIG_DEFAULT_ppp_mod_pppoe=y
CONFIG_DEFAULT_procd=y
CONFIG_DEFAULT_uci=y
CONFIG_DEFAULT_uclient_fetch=y
CONFIG_DEFAULT_urandom_seed=y
CONFIG_DEFAULT_urngd=y
CONFIG_AUDIO_SUPPORT=y
CONFIG_GPIO_SUPPORT=y
CONFIG_PCI_SUPPORT=y
CONFIG_USB_SUPPORT=y
CONFIG_RTC_SUPPORT=y
CONFIG_USES_DEVICETREE=y
CONFIG_USES_INITRAMFS=y
CONFIG_USES_SQUASHFS=y
CONFIG_NAND_SUPPORT=y
CONFIG_ARCH_64BIT=y
CONFIG_aarch64=y
CONFIG_ARCH="aarch64"

CONFIG_TARGET_ROOTFS_INITRAMFS=y
CONFIG_TARGET_INITRAMFS_COMPRESSION_NONE=y
CONFIG_EXTERNAL_CPIO=""


CONFIG_TARGET_ROOTFS_SQUASHFS=y
CONFIG_TARGET_SQUASHFS_BLOCK_SIZE=256
CONFIG_TARGET_UBIFS_FREE_SPACE_FIXUP=y
CONFIG_TARGET_UBIFS_JOURNAL_SIZE=""



CONFIG_SIGNED_PACKAGES=y
CONFIG_SIGNATURE_CHECK=y
CONFIG_DOWNLOAD_CHECK_CERTIFICATE=y

CONFIG_SHADOW_PASSWORDS=y

CONFIG_KERNEL_BUILD_USER=""
CONFIG_KERNEL_BUILD_DOMAIN=""
CONFIG_KERNEL_PRINTK=y
CONFIG_KERNEL_CRASHLOG=y
CONFIG_KERNEL_SWAP=y
CONFIG_KERNEL_DEBUG_FS=y
CONFIG_KERNEL_KALLSYMS=y
CONFIG_KERNEL_DEBUG_KERNEL=y
CONFIG_KERNEL_DEBUG_INFO=y
CONFIG_KERNEL_AIO=y
CONFIG_KERNEL_IO_URING=y
CONFIG_KERNEL_FHANDLE=y
CONFIG_KERNEL_FANOTIFY=y
CONFIG_KERNEL_MAGIC_SYSRQ=y
CONFIG_KERNEL_COREDUMP=y
CONFIG_KERNEL_ELF_CORE=y
CONFIG_KERNEL_PRINTK_TIME=y
CONFIG_KERNEL_KEYS=y
CONFIG_KERNEL_CGROUPS=y
CONFIG_KERNEL_FREEZER=y
CONFIG_KERNEL_CGROUP_PIDS=y
CONFIG_KERNEL_CGROUP_RDMA=y
CONFIG_KERNEL_CGROUP_BPF=y
CONFIG_KERNEL_CPUSETS=y
CONFIG_KERNEL_CGROUP_CPUACCT=y
CONFIG_KERNEL_RESOURCE_COUNTERS=y
CONFIG_KERNEL_MM_OWNER=y
CONFIG_KERNEL_MEMCG=y
CONFIG_KERNEL_MEMCG_SWAP=y
CONFIG_KERNEL_MEMCG_KMEM=y
CONFIG_KERNEL_CGROUP_SCHED=y
CONFIG_KERNEL_FAIR_GROUP_SCHED=y
CONFIG_KERNEL_CFS_BANDWIDTH=y
CONFIG_KERNEL_RT_GROUP_SCHED=y
CONFIG_KERNEL_BLK_CGROUP=y
CONFIG_KERNEL_BLK_DEV_THROTTLING=y
CONFIG_KERNEL_NAMESPACES=y
CONFIG_KERNEL_UTS_NS=y
CONFIG_KERNEL_IPC_NS=y
CONFIG_KERNEL_USER_NS=y
CONFIG_KERNEL_PID_NS=y
CONFIG_KERNEL_NET_NS=y
CONFIG_KERNEL_DEVPTS_MULTIPLE_INSTANCES=y
CONFIG_KERNEL_POSIX_MQUEUE=y
CONFIG_KERNEL_SECCOMP_FILTER=y
CONFIG_KERNEL_SECCOMP=y
CONFIG_KERNEL_IP_MROUTE=y
CONFIG_KERNEL_IPV6=y
CONFIG_KERNEL_IPV6_MULTIPLE_TABLES=y
CONFIG_KERNEL_IPV6_SUBTREES=y
CONFIG_KERNEL_IPV6_MROUTE=y
CONFIG_KERNEL_IPV6_SEG6_LWTUNNEL=y


CONFIG_KERNEL_DEVMEM=y
CONFIG_KERNEL_SQUASHFS_FRAGMENT_CACHE_SIZE=3
CONFIG_KERNEL_CC_OPTIMIZE_FOR_PERFORMANCE=y

CONFIG_IPV6=y

CONFIG_USE_SSTRIP=y
CONFIG_SSTRIP_ARGS="_z"

CONFIG_PKG_CHECK_FORMAT_SECURITY=y
CONFIG_PKG_ASLR_PIE_REGULAR=y
CONFIG_PKG_CC_STACKPROTECTOR_REGULAR=y
CONFIG_KERNEL_CC_STACKPROTECTOR_REGULAR=y
CONFIG_KERNEL_STACKPROTECTOR=y
CONFIG_PKG_FORTIFY_SOURCE_1=y
CONFIG_PKG_RELRO_FULL=y

CONFIG_BINARY_FOLDER=""
CONFIG_DOWNLOAD_FOLDER=""
CONFIG_LOCALMIRROR=""
CONFIG_AUTOREBUILD=y
CONFIG_BUILD_SUFFIX=""
CONFIG_TARGET_ROOTFS_DIR=""
CONFIG_CCACHE_DIR=""
CONFIG_KERNEL_CFLAGS=""
CONFIG_EXTERNAL_KERNEL_TREE=""
CONFIG_KERNEL_GIT_CLONE_URI=""
CONFIG_BUILD_LOG_DIR=""
CONFIG_EXTRA_OPTIMIZATION="_fno_caller_saves _fno_plt"
CONFIG_TARGET_OPTIMIZATION="_Os _pipe _mcpu=cortex_a53"
CONFIG_EXTRA_BINUTILS_CONFIG_OPTIONS=""
CONFIG_EXTRA_GCC_CONFIG_OPTIONS=""
CONFIG_GDB=y
CONFIG_USE_MUSL=y
CONFIG_SSP_SUPPORT=y
CONFIG_BINUTILS_VERSION_2_34=y
CONFIG_BINUTILS_VERSION="2_34"
CONFIG_GCC_VERSION="8_4_0"
CONFIG_LIBC="musl"
CONFIG_TARGET_SUFFIX="musl"
CONFIG_TARGET_PREINIT_SUPPRESS_STDERR=y
CONFIG_TARGET_PREINIT_TIMEOUT=2
CONFIG_TARGET_PREINIT_IFNAME=""
CONFIG_TARGET_PREINIT_IP="192_168_1_1"
CONFIG_TARGET_PREINIT_NETMASK="255_255_255_0"
CONFIG_TARGET_PREINIT_BROADCAST="192_168_1_255"
CONFIG_TARGET_INIT_PATH="/usr/sbin:/usr/bin:/sbin:/bin"
CONFIG_TARGET_INIT_ENV=""
CONFIG_TARGET_INIT_CMD="/sbin/init"
CONFIG_TARGET_INIT_SUPPRESS_STDERR=y
CONFIG_PER_FEED_REPO=y
CONFIG_FEED_packages=y
CONFIG_FEED_luci=y
CONFIG_FEED_routing=y
CONFIG_FEED_telephony=y
CONFIG_FEED_mtk_openwrt_feed=y

CONFIG_PACKAGE_base_files=y
CONFIG_PACKAGE_busybox=y
CONFIG_BUSYBOX_DEFAULT_HAVE_DOT_CONFIG=y
CONFIG_BUSYBOX_DEFAULT_INCLUDE_SUSv2=y
CONFIG_BUSYBOX_DEFAULT_LONG_OPTS=y
CONFIG_BUSYBOX_DEFAULT_SHOW_USAGE=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_VERBOSE_USAGE=y
CONFIG_BUSYBOX_DEFAULT_LFS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_DEVPTS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_PIDFILE=y
CONFIG_BUSYBOX_DEFAULT_PID_FILE_PATH="/var/run"
CONFIG_BUSYBOX_DEFAULT_FEATURE_PREFER_APPLETS=y
CONFIG_BUSYBOX_DEFAULT_BUSYBOX_EXEC_PATH="/proc/self/exe"
CONFIG_BUSYBOX_DEFAULT_FEATURE_SYSLOG=y
CONFIG_BUSYBOX_DEFAULT_CROSS_COMPILER_PREFIX=""
CONFIG_BUSYBOX_DEFAULT_SYSROOT=""
CONFIG_BUSYBOX_DEFAULT_EXTRA_CFLAGS=""
CONFIG_BUSYBOX_DEFAULT_EXTRA_LDFLAGS=""
CONFIG_BUSYBOX_DEFAULT_EXTRA_LDLIBS=""
CONFIG_BUSYBOX_DEFAULT_INSTALL_APPLET_SYMLINKS=y
CONFIG_BUSYBOX_DEFAULT_PREFIX="_/_install"
CONFIG_BUSYBOX_DEFAULT_NO_DEBUG_LIB=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_BUFFERS_GO_ON_STACK=y
CONFIG_BUSYBOX_DEFAULT_PASSWORD_MINLEN=6
CONFIG_BUSYBOX_DEFAULT_MD5_SMALL=1
CONFIG_BUSYBOX_DEFAULT_SHA3_SMALL=1
CONFIG_BUSYBOX_DEFAULT_FEATURE_FAST_TOP=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_EDITING=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_EDITING_MAX_LEN=512
CONFIG_BUSYBOX_DEFAULT_FEATURE_EDITING_HISTORY=256
CONFIG_BUSYBOX_DEFAULT_FEATURE_TAB_COMPLETION=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_EDITING_FANCY_PROMPT=y
CONFIG_BUSYBOX_DEFAULT_SUBST_WCHAR=0
CONFIG_BUSYBOX_DEFAULT_LAST_SUPPORTED_WCHAR=0
CONFIG_BUSYBOX_DEFAULT_FEATURE_NON_POSIX_CP=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_USE_SENDFILE=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_COPYBUF_KB=4
CONFIG_BUSYBOX_DEFAULT_MONOTONIC_SYSCALL=y
CONFIG_BUSYBOX_DEFAULT_IOCTL_HEX2STR_ERROR=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_SEAMLESS_GZ=y
CONFIG_BUSYBOX_DEFAULT_GUNZIP=y
CONFIG_BUSYBOX_DEFAULT_ZCAT=y
CONFIG_BUSYBOX_DEFAULT_BUNZIP2=y
CONFIG_BUSYBOX_DEFAULT_BZCAT=y
CONFIG_BUSYBOX_DEFAULT_BZIP2_SMALL=0
CONFIG_BUSYBOX_DEFAULT_FEATURE_BZIP2_DECOMPRESS=y
CONFIG_BUSYBOX_DEFAULT_GZIP=y
CONFIG_BUSYBOX_DEFAULT_GZIP_FAST=0
CONFIG_BUSYBOX_DEFAULT_FEATURE_GZIP_DECOMPRESS=y
CONFIG_BUSYBOX_DEFAULT_TAR=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_TAR_CREATE=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_TAR_FROM=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_TAR_GNU_EXTENSIONS=y
CONFIG_BUSYBOX_DEFAULT_BASENAME=y
CONFIG_BUSYBOX_DEFAULT_CAT=y
CONFIG_BUSYBOX_DEFAULT_CHGRP=y
CONFIG_BUSYBOX_DEFAULT_CHMOD=y
CONFIG_BUSYBOX_DEFAULT_CHOWN=y
CONFIG_BUSYBOX_DEFAULT_CHROOT=y
CONFIG_BUSYBOX_DEFAULT_CP=y
CONFIG_BUSYBOX_DEFAULT_CUT=y
CONFIG_BUSYBOX_DEFAULT_DATE=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_DATE_ISOFMT=y
CONFIG_BUSYBOX_DEFAULT_DD=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_DD_SIGNAL_HANDLING=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_DD_IBS_OBS=y
CONFIG_BUSYBOX_DEFAULT_DF=y
CONFIG_BUSYBOX_DEFAULT_DIRNAME=y
CONFIG_BUSYBOX_DEFAULT_DU=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_DU_DEFAULT_BLOCKSIZE_1K=y
CONFIG_BUSYBOX_DEFAULT_ECHO=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FANCY_ECHO=y
CONFIG_BUSYBOX_DEFAULT_ENV=y
CONFIG_BUSYBOX_DEFAULT_EXPR=y
CONFIG_BUSYBOX_DEFAULT_EXPR_MATH_SUPPORT_64=y
CONFIG_BUSYBOX_DEFAULT_FALSE=y
CONFIG_BUSYBOX_DEFAULT_HEAD=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FANCY_HEAD=y
CONFIG_BUSYBOX_DEFAULT_ID=y
CONFIG_BUSYBOX_DEFAULT_LN=y
CONFIG_BUSYBOX_DEFAULT_LS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_LS_FILETYPES=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_LS_FOLLOWLINKS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_LS_RECURSIVE=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_LS_WIDTH=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_LS_SORTFILES=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_LS_TIMESTAMPS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_LS_USERNAME=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_LS_COLOR=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_LS_COLOR_IS_DEFAULT=y
CONFIG_BUSYBOX_DEFAULT_MD5SUM=y
CONFIG_BUSYBOX_DEFAULT_SHA256SUM=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_MD5_SHA1_SUM_CHECK=y
CONFIG_BUSYBOX_DEFAULT_MKDIR=y
CONFIG_BUSYBOX_DEFAULT_MKFIFO=y
CONFIG_BUSYBOX_DEFAULT_MKNOD=y
CONFIG_BUSYBOX_DEFAULT_MKTEMP=y
CONFIG_BUSYBOX_DEFAULT_MV=y
CONFIG_BUSYBOX_DEFAULT_NICE=y
CONFIG_BUSYBOX_DEFAULT_PRINTF=y
CONFIG_BUSYBOX_DEFAULT_PWD=y
CONFIG_BUSYBOX_DEFAULT_READLINK=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_READLINK_FOLLOW=y
CONFIG_BUSYBOX_DEFAULT_RM=y
CONFIG_BUSYBOX_DEFAULT_RMDIR=y
CONFIG_BUSYBOX_DEFAULT_SEQ=y
CONFIG_BUSYBOX_DEFAULT_SLEEP=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FANCY_SLEEP=y
CONFIG_BUSYBOX_DEFAULT_SORT=y
CONFIG_BUSYBOX_DEFAULT_SYNC=y
CONFIG_BUSYBOX_DEFAULT_FSYNC=y
CONFIG_BUSYBOX_DEFAULT_TAIL=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FANCY_TAIL=y
CONFIG_BUSYBOX_DEFAULT_TEE=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_TEE_USE_BLOCK_IO=y
CONFIG_BUSYBOX_DEFAULT_TEST=y
CONFIG_BUSYBOX_DEFAULT_TEST1=y
CONFIG_BUSYBOX_DEFAULT_TEST2=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_TEST_64=y
CONFIG_BUSYBOX_DEFAULT_TOUCH=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_TOUCH_SUSV3=y
CONFIG_BUSYBOX_DEFAULT_TR=y
CONFIG_BUSYBOX_DEFAULT_TRUE=y
CONFIG_BUSYBOX_DEFAULT_UNAME=y
CONFIG_BUSYBOX_DEFAULT_UNAME_OSNAME="GNU/Linux"
CONFIG_BUSYBOX_DEFAULT_UNIQ=y
CONFIG_BUSYBOX_DEFAULT_WC=y
CONFIG_BUSYBOX_DEFAULT_YES=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_PRESERVE_HARDLINKS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_HUMAN_READABLE=y
CONFIG_BUSYBOX_DEFAULT_CLEAR=y
CONFIG_BUSYBOX_DEFAULT_DEFAULT_SETFONT_DIR=""
CONFIG_BUSYBOX_DEFAULT_RESET=y
CONFIG_BUSYBOX_DEFAULT_START_STOP_DAEMON=y
CONFIG_BUSYBOX_DEFAULT_WHICH=y
CONFIG_BUSYBOX_DEFAULT_AWK=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_AWK_LIBM=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_AWK_GNU_EXTENSIONS=y
CONFIG_BUSYBOX_DEFAULT_CMP=y
CONFIG_BUSYBOX_DEFAULT_SED=y
CONFIG_BUSYBOX_DEFAULT_VI=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_VI_MAX_LEN=1024
CONFIG_BUSYBOX_DEFAULT_FEATURE_VI_COLON=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_VI_YANKMARK=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_VI_SEARCH=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_VI_USE_SIGNALS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_VI_DOT_CMD=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_VI_READONLY=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_VI_SETOPTS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_VI_SET=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_VI_WIN_RESIZE=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_VI_ASK_TERMINAL=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_VI_UNDO_QUEUE_MAX=0
CONFIG_BUSYBOX_DEFAULT_FEATURE_ALLOW_EXEC=y
CONFIG_BUSYBOX_DEFAULT_FIND=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_PRINT0=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_MTIME=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_MMIN=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_PERM=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_TYPE=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_XDEV=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_MAXDEPTH=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_NEWER=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_EXEC=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_USER=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_GROUP=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_NOT=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_DEPTH=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_PAREN=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_SIZE=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_PRUNE=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_PATH=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FIND_REGEX=y
CONFIG_BUSYBOX_DEFAULT_GREP=y
CONFIG_BUSYBOX_DEFAULT_EGREP=y
CONFIG_BUSYBOX_DEFAULT_FGREP=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_GREP_CONTEXT=y
CONFIG_BUSYBOX_DEFAULT_XARGS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_XARGS_SUPPORT_CONFIRMATION=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_XARGS_SUPPORT_QUOTES=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_XARGS_SUPPORT_TERMOPT=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_XARGS_SUPPORT_ZERO_TERM=y
CONFIG_BUSYBOX_DEFAULT_HALT=y
CONFIG_BUSYBOX_DEFAULT_POWEROFF=y
CONFIG_BUSYBOX_DEFAULT_REBOOT=y
CONFIG_BUSYBOX_DEFAULT_TELINIT_PATH=""
CONFIG_BUSYBOX_DEFAULT_FEATURE_KILL_DELAY=0
CONFIG_BUSYBOX_DEFAULT_INIT_TERMINAL_TYPE=""
CONFIG_BUSYBOX_DEFAULT_FEATURE_SHADOWPASSWDS=y
CONFIG_BUSYBOX_DEFAULT_LAST_ID=0
CONFIG_BUSYBOX_DEFAULT_FIRST_SYSTEM_ID=0
CONFIG_BUSYBOX_DEFAULT_LAST_SYSTEM_ID=0
CONFIG_BUSYBOX_DEFAULT_FEATURE_DEFAULT_PASSWD_ALGO="md5"
CONFIG_BUSYBOX_DEFAULT_LOGIN=y
CONFIG_BUSYBOX_DEFAULT_LOGIN_SESSION_AS_CHILD=y
CONFIG_BUSYBOX_DEFAULT_PASSWD=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_PASSWD_WEAK_CHECK=y
CONFIG_BUSYBOX_DEFAULT_DEFAULT_MODULES_DIR=""
CONFIG_BUSYBOX_DEFAULT_DEFAULT_DEPMOD_FILE=""
CONFIG_BUSYBOX_DEFAULT_DMESG=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_DMESG_PRETTY=y
CONFIG_BUSYBOX_DEFAULT_FLOCK=y
CONFIG_BUSYBOX_DEFAULT_HEXDUMP=y
CONFIG_BUSYBOX_DEFAULT_HWCLOCK=y
CONFIG_BUSYBOX_DEFAULT_MKSWAP=y
CONFIG_BUSYBOX_DEFAULT_MOUNT=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_MOUNT_HELPERS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_MOUNT_CIFS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_MOUNT_FLAGS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_MOUNT_FSTAB=y
CONFIG_BUSYBOX_DEFAULT_PIVOT_ROOT=y
CONFIG_BUSYBOX_DEFAULT_SWAPON=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_SWAPON_DISCARD=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_SWAPON_PRI=y
CONFIG_BUSYBOX_DEFAULT_SWAPOFF=y
CONFIG_BUSYBOX_DEFAULT_SWITCH_ROOT=y
CONFIG_BUSYBOX_DEFAULT_UMOUNT=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_UMOUNT_ALL=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_MOUNT_LOOP=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_BEEP_FREQ=0
CONFIG_BUSYBOX_DEFAULT_FEATURE_BEEP_LENGTH_MS=0
CONFIG_BUSYBOX_DEFAULT_CROND=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_CROND_DIR="/etc"
CONFIG_BUSYBOX_DEFAULT_CRONTAB=y
CONFIG_BUSYBOX_DEFAULT_LESS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_LESS_MAXLINES=9999999
CONFIG_BUSYBOX_DEFAULT_LOCK=y
CONFIG_BUSYBOX_DEFAULT_STRINGS=y
CONFIG_BUSYBOX_DEFAULT_TIME=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_IPV6=y
CONFIG_BUSYBOX_DEFAULT_VERBOSE_RESOLUTION_ERRORS=y
CONFIG_BUSYBOX_DEFAULT_BRCTL=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_BRCTL_FANCY=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_BRCTL_SHOW=y
CONFIG_BUSYBOX_DEFAULT_IFCONFIG=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_IFCONFIG_STATUS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_IFCONFIG_HW=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_IFCONFIG_BROADCAST_PLUS=y
CONFIG_BUSYBOX_DEFAULT_IFUPDOWN_IFSTATE_PATH=""
CONFIG_BUSYBOX_DEFAULT_IP=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_IP_ADDRESS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_IP_LINK=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_IP_ROUTE=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_IP_ROUTE_DIR="/etc/iproute2"
CONFIG_BUSYBOX_DEFAULT_FEATURE_IP_RULE=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_IP_NEIGH=y
CONFIG_BUSYBOX_DEFAULT_NC=y
CONFIG_BUSYBOX_DEFAULT_NETMSG=y
CONFIG_BUSYBOX_DEFAULT_NETSTAT=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_NETSTAT_WIDE=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_NETSTAT_PRG=y
CONFIG_BUSYBOX_DEFAULT_NSLOOKUP_OPENWRT=y
CONFIG_BUSYBOX_DEFAULT_NTPD=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_NTPD_SERVER=y
CONFIG_BUSYBOX_DEFAULT_PING=y
CONFIG_BUSYBOX_DEFAULT_PING6=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_FANCY_PING=y
CONFIG_BUSYBOX_DEFAULT_ROUTE=y
CONFIG_BUSYBOX_DEFAULT_TRACEROUTE=y
CONFIG_BUSYBOX_DEFAULT_TRACEROUTE6=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_TRACEROUTE_VERBOSE=y
CONFIG_BUSYBOX_DEFAULT_DHCPD_LEASES_FILE=""
CONFIG_BUSYBOX_DEFAULT_UDHCPC=y
CONFIG_BUSYBOX_DEFAULT_UDHCPC_DEFAULT_SCRIPT="/usr/share/udhcpc/default_script"
CONFIG_BUSYBOX_DEFAULT_UDHCP_DEBUG=0
CONFIG_BUSYBOX_DEFAULT_UDHCPC_SLACK_FOR_BUGGY_SERVERS=80
CONFIG_BUSYBOX_DEFAULT_FEATURE_UDHCP_RFC3397=y
CONFIG_BUSYBOX_DEFAULT_IFUPDOWN_UDHCPC_CMD_OPTIONS=""
CONFIG_BUSYBOX_DEFAULT_FEATURE_MIME_CHARSET=""
CONFIG_BUSYBOX_DEFAULT_FREE=y
CONFIG_BUSYBOX_DEFAULT_KILL=y
CONFIG_BUSYBOX_DEFAULT_KILLALL=y
CONFIG_BUSYBOX_DEFAULT_PGREP=y
CONFIG_BUSYBOX_DEFAULT_PIDOF=y
CONFIG_BUSYBOX_DEFAULT_PS=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_PS_WIDE=y
CONFIG_BUSYBOX_DEFAULT_BB_SYSCTL=y
CONFIG_BUSYBOX_DEFAULT_TOP=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_TOP_CPU_USAGE_PERCENTAGE=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_TOP_CPU_GLOBAL_PERCENTS=y
CONFIG_BUSYBOX_DEFAULT_UPTIME=y
CONFIG_BUSYBOX_DEFAULT_SV_DEFAULT_SERVICE_DIR=""
CONFIG_BUSYBOX_DEFAULT_SH_IS_ASH=y
CONFIG_BUSYBOX_DEFAULT_BASH_IS_NONE=y
CONFIG_BUSYBOX_DEFAULT_SHELL_ASH=y
CONFIG_BUSYBOX_DEFAULT_ASH=y
CONFIG_BUSYBOX_DEFAULT_ASH_INTERNAL_GLOB=y
CONFIG_BUSYBOX_DEFAULT_ASH_BASH_COMPAT=y
CONFIG_BUSYBOX_DEFAULT_ASH_JOB_CONTROL=y
CONFIG_BUSYBOX_DEFAULT_ASH_ALIAS=y
CONFIG_BUSYBOX_DEFAULT_ASH_EXPAND_PRMT=y
CONFIG_BUSYBOX_DEFAULT_ASH_ECHO=y
CONFIG_BUSYBOX_DEFAULT_ASH_PRINTF=y
CONFIG_BUSYBOX_DEFAULT_ASH_TEST=y
CONFIG_BUSYBOX_DEFAULT_ASH_GETOPTS=y
CONFIG_BUSYBOX_DEFAULT_ASH_CMDCMD=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_SH_MATH=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_SH_MATH_64=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_SH_NOFORK=y
CONFIG_BUSYBOX_DEFAULT_LOGGER=y
CONFIG_BUSYBOX_DEFAULT_FEATURE_SYSLOGD_READ_BUFFER_SIZE=0
CONFIG_BUSYBOX_DEFAULT_FEATURE_IPC_SYSLOG_BUFFER_SIZE=0
CONFIG_PACKAGE_dnsmasq=y
CONFIG_PACKAGE_fstools=y
CONFIG_FSTOOLS_UBIFS_EXTROOT=y
CONFIG_PACKAGE_fwtool=y
CONFIG_PACKAGE_getrandom=y
CONFIG_PACKAGE_jsonfilter=y
CONFIG_PACKAGE_libc=y
CONFIG_PACKAGE_libgcc=y
CONFIG_PACKAGE_libpthread=y
CONFIG_PACKAGE_librt=y
CONFIG_PACKAGE_logd=y
CONFIG_PACKAGE_mtd=y
CONFIG_PACKAGE_netifd=y
CONFIG_PACKAGE_openwrt_keyring=y
CONFIG_PACKAGE_opkg=y
CONFIG_PACKAGE_procd=y


CONFIG_PACKAGE_swconfig=y
CONFIG_PACKAGE_ubox=y
CONFIG_PACKAGE_ubus=y
CONFIG_PACKAGE_ubusd=y
CONFIG_PACKAGE_uci=y
CONFIG_PACKAGE_urandom_seed=y
CONFIG_PACKAGE_urngd=y
CONFIG_PACKAGE_usign=y
CONFIG_PACKAGE_wireless_tools=y



CONFIG_ZABBIX_POSTGRESQL=y





CONFIG_PACKAGE_atelIDURM=y
CONFIG_PACKAGE_atel_api=y
CONFIG_PACKAGE_autoreboot_mng=y
CONFIG_PACKAGE_cgi=y
CONFIG_PACKAGE_cgiapi=y
CONFIG_PACKAGE_diag_transfer=y
CONFIG_PACKAGE_factory_auto=y
CONFIG_PACKAGE_factory_dhcp=y
CONFIG_PACKAGE_gpio_control=y
CONFIG_PACKAGE_libatel=y
CONFIG_PACKAGE_libnvram=y
CONFIG_PACKAGE_libusercrypto=y
CONFIG_PACKAGE_ltemanage=y
CONFIG_PACKAGE_nvram_atel=y
CONFIG_PACKAGE_qlog=y
CONFIG_PACKAGE_quectel_cm=y
CONFIG_PACKAGE_remoteUpgrade=y
CONFIG_PACKAGE_remoteUpgradefactory=y
CONFIG_PACKAGE_serial_test=y





CONFIG_PACKAGE_libiwinfo_data=y







CONFIG_PACKAGE_kmod_ralink_nvram=y










CONFIG_PACKAGE_kmod_leds_gpio=y

CONFIG_PACKAGE_kmod_lib_crc_ccitt=y

CONFIG_PACKAGE_kmod_nls_base=y

CONFIG_PACKAGE_kmod_ip6tables=y
CONFIG_PACKAGE_kmod_ipt_conntrack=y
CONFIG_PACKAGE_kmod_ipt_core=y
CONFIG_PACKAGE_kmod_ipt_nat=y
CONFIG_PACKAGE_kmod_ipt_offload=y
CONFIG_PACKAGE_kmod_nf_conntrack=y
CONFIG_PACKAGE_kmod_nf_conntrack6=y
CONFIG_PACKAGE_kmod_nf_flow=y
CONFIG_PACKAGE_kmod_nf_ipt=y
CONFIG_PACKAGE_kmod_nf_ipt6=y
CONFIG_PACKAGE_kmod_nf_nat=y
CONFIG_PACKAGE_kmod_nf_reject=y
CONFIG_PACKAGE_kmod_nf_reject6=y

CONFIG_PACKAGE_kmod_mediatek_hnat=y
CONFIG_PACKAGE_kmod_mii=y

CONFIG_PACKAGE_kmod_ppp=y
CONFIG_PACKAGE_kmod_pppoe=y
CONFIG_PACKAGE_kmod_pppox=y
CONFIG_PACKAGE_kmod_slhc=y

CONFIG_PACKAGE_kmod_gpio_button_hotplug=y




CONFIG_PACKAGE_kmod_usb_acm=y
CONFIG_PACKAGE_kmod_usb_core=y
CONFIG_PACKAGE_kmod_usb_net=y
CONFIG_PACKAGE_kmod_usb_net_cdc_ether=y
CONFIG_PACKAGE_kmod_usb_net_qmi_wwan=y
CONFIG_PACKAGE_kmod_usb_net_rndis=y
CONFIG_PACKAGE_kmod_usb_serial=y
CONFIG_PACKAGE_kmod_usb_serial_option=y
CONFIG_PACKAGE_kmod_usb_serial_wwan=y
CONFIG_PACKAGE_kmod_usb_wdm=y
CONFIG_PACKAGE_kmod_usb3=y










CONFIG_GOLANG_EXTERNAL_BOOTSTRAP_ROOT=""
CONFIG_GOLANG_BUILD_CACHE_DIR=""


CONFIG_PACKAGE_lua=y















CONFIG_PACKAGE_libip4tc=y
CONFIG_PACKAGE_libip6tc=y
CONFIG_PACKAGE_libxtables=y






CONFIG_PACKAGE_libopenssl=y

CONFIG_OPENSSL_WITH_ASM=y
CONFIG_OPENSSL_WITH_DEPRECATED=y
CONFIG_OPENSSL_WITH_ERROR_MESSAGES=y

CONFIG_OPENSSL_WITH_TLS13=y
CONFIG_OPENSSL_WITH_SRP=y
CONFIG_OPENSSL_WITH_CMS=y

CONFIG_OPENSSL_WITH_CHACHA_POLY1305=y
CONFIG_OPENSSL_WITH_PSK=y


CONFIG_OPENSSL_ENGINE=y
CONFIG_PACKAGE_libwolfssl=y
CONFIG_WOLFSSL_HAS_AES_CCM=y
CONFIG_WOLFSSL_HAS_CHACHA_POLY=y
CONFIG_WOLFSSL_HAS_DH=y
CONFIG_WOLFSSL_HAS_ARC4=y
CONFIG_WOLFSSL_HAS_CERTGEN=y
CONFIG_WOLFSSL_HAS_TLSV10=y
CONFIG_WOLFSSL_HAS_TLSV13=y
CONFIG_WOLFSSL_HAS_SESSION_TICKET=y
CONFIG_WOLFSSL_HAS_OCSP=y
CONFIG_WOLFSSL_HAS_WPAS=y
CONFIG_WOLFSSL_HAS_ECC25519=y
CONFIG_WOLFSSL_ALT_NAMES=y
CONFIG_WOLFSSL_HAS_NO_HW=y




CONFIG_PACKAGE_libblkid=y
CONFIG_PACKAGE_libblobmsg_json=y
CONFIG_PACKAGE_libcap_ng=y
CONFIG_PACKAGE_libfstools_bootparam=y
CONFIG_PACKAGE_libjson_c=y
CONFIG_PACKAGE_libkvcutil=y
CONFIG_PACKAGE_liblua=y
CONFIG_PACKAGE_libmount=y
CONFIG_PACKAGE_libnl_tiny=y
CONFIG_PACKAGE_libpcap=y


CONFIG_PACKAGE_libpcre=y
CONFIG_PACKAGE_libsmartcols=y
CONFIG_PACKAGE_libubox=y
CONFIG_PACKAGE_libubus=y
CONFIG_PACKAGE_libuci=y
CONFIG_PACKAGE_libuclient=y
CONFIG_PACKAGE_libusb_1_0=y
CONFIG_PACKAGE_libustream_wolfssl=y
CONFIG_PACKAGE_libuuid=y
CONFIG_PACKAGE_zlib=y




CONFIG_LUCI_JSMIN=y
CONFIG_LUCI_CSSTIDY=y






CONFIG_PACKAGE_luci_lib_nixio_notls=y


CONFIG_POSTFIX_TLS=y
CONFIG_POSTFIX_SASL=y
CONFIG_POSTFIX_LDAP=y
CONFIG_POSTFIX_CDB=y
CONFIG_POSTFIX_SQLITE=y
CONFIG_POSTFIX_PCRE=y



CONFIG_PACKAGE_8021xd=y
CONFIG_PACKAGE_ated_ext=y
CONFIG_PACKAGE_mii_mgr=y
CONFIG_PACKAGE_regs=y

CONFIG_PACKAGE_kmod_conninfra=y
CONFIG_MTK_CONNINFRA_APSOC=y
CONFIG_MTK_CONNINFRA_APSOC_MT7981=y
CONFIG_CONNINFRA_EMI_SUPPORT=y
CONFIG_CONNINFRA_AUTO_UP=y
CONFIG_PACKAGE_kmod_mapfilter=y
CONFIG_MTK_SUPPORT_OPENWRT=y
CONFIG_MTK_WIFI_DRIVER=y
CONFIG_MTK_FIRST_IF_MT7981=y
CONFIG_MTK_SECOND_IF_NONE=y
CONFIG_MTK_THIRD_IF_NONE=y
CONFIG_MTK_RT_FIRST_IF_RF_OFFSET=0xc0000
CONFIG_MTK_MT_WIFI=m
CONFIG_MTK_MT_WIFI_PATH="mt_wifi"

CONFIG_MTK_FIRST_IF_EEPROM_FLASH=y
CONFIG_MTK_RT_FIRST_CARD_EEPROM="flash"
CONFIG_MTK_WIFI_BASIC_FUNC=y
CONFIG_MTK_DOT11_N_SUPPORT=y
CONFIG_MTK_DOT11_VHT_AC=y
CONFIG_MTK_DOT11_HE_AX=y
CONFIG_MTK_CFG_SUPPORT_FALCON_MURU=y
CONFIG_MTK_CFG_SUPPORT_FALCON_TXCMD_DBG=y
CONFIG_MTK_CFG_SUPPORT_FALCON_SR=y
CONFIG_MTK_CFG_SUPPORT_FALCON_PP=y
CONFIG_MTK_WIFI_TWT_SUPPORT=y
CONFIG_MTK_G_BAND_256QAM_SUPPORT=y
CONFIG_MTK_TPC_SUPPORT=y
CONFIG_MTK_ICAP_SUPPORT=y
CONFIG_MTK_SPECTRUM_SUPPORT=y
CONFIG_MTK_PHY_ICS_SUPPORT=y
CONFIG_MTK_BACKGROUND_SCAN_SUPPORT=y
CONFIG_MTK_SMART_CARRIER_SENSE_SUPPORT=y
CONFIG_MTK_SCS_FW_OFFLOAD=y
CONFIG_MTK_MT_DFS_SUPPORT=y
CONFIG_MTK_OFFCHANNEL_SCAN_FEATURE=y
CONFIG_MTK_HDR_TRANS_TX_SUPPORT=y
CONFIG_MTK_HDR_TRANS_RX_SUPPORT=y
CONFIG_MTK_DBDC_MODE=y
CONFIG_MTK_MULTI_PROFILE_SUPPORT=y
CONFIG_MTK_WSC_INCLUDED=y
CONFIG_MTK_WSC_V2_SUPPORT=y
CONFIG_MTK_DOT11W_PMF_SUPPORT=y
CONFIG_MTK_TXBF_SUPPORT=y
CONFIG_MTK_FAST_NAT_SUPPORT=y
CONFIG_MTK_IGMP_SNOOP_SUPPORT=y
CONFIG_MTK_RTMP_FLASH_SUPPORT=y
CONFIG_MTK_CAL_BIN_FILE_SUPPORT=y
CONFIG_MTK_ATE_SUPPORT=y
CONFIG_MTK_WLAN_SERVICE=y
CONFIG_MTK_MBO_SUPPORT=y
CONFIG_MTK_OCE_SUPPORT=y
CONFIG_MTK_MAP_SUPPORT=y
CONFIG_MTK_MAP_R2_VER_SUPPORT=y
CONFIG_MTK_MAP_R3_VER_SUPPORT=y
CONFIG_MTK_UAPSD=y
CONFIG_MTK_RED_SUPPORT=y
CONFIG_MTK_FIRST_IF_IPAILNA=y
CONFIG_MTK_WIFI_SKU_TYPE="AX6000"
CONFIG_MTK_WIFI_ADIE_TYPE="mt7975"

CONFIG_MTK_WIFI_MODE_AP=m
CONFIG_MTK_MT_AP_SUPPORT=m
CONFIG_MTK_WDS_SUPPORT=y
CONFIG_MTK_MBSS_SUPPORT=y
CONFIG_MTK_APCLI_SUPPORT=y
CONFIG_MTK_APCLI_CERT_SUPPORT=y
CONFIG_MTK_MAC_REPEATER_SUPPORT=y
CONFIG_MTK_MWDS=y
CONFIG_MTK_MUMIMO_SUPPORT=y
CONFIG_MTK_MU_RA_SUPPORT=y
CONFIG_MTK_DOT11R_FT_SUPPORT=y
CONFIG_MTK_DOT11K_RRM_SUPPORT=y
CONFIG_MTK_MLME_MULTI_QUEUE_SUPPORT=y
CONFIG_MTK_WIFI_EAP_FEATURE=y
CONFIG_MTK_VLAN_SUPPORT=y
CONFIG_MTK_SNIFFER_RADIOTAP_SUPPORT=y
CONFIG_MTK_ANTENNA_CONTROL_SUPPORT=y
CONFIG_MTK_MGMT_TXPWR_CTRL=y
CONFIG_MTK_RA_PHY_RATE_SUPPORT=y
CONFIG_MTK_AMPDU_CONF_SUPPORT=y
CONFIG_MTK_ACK_CTS_TIMEOUT_SUPPORT=y
CONFIG_MTK_MBSS_DTIM_SUPPORT=y
CONFIG_MTK_QOS_R1_SUPPORT=y
CONFIG_MTK_DSCP_PRI_SUPPORT=y
CONFIG_MTK_CON_WPS_SUPPORT=y
CONFIG_MTK_MCAST_RATE_SPECIFIC=y
CONFIG_MTK_VOW_SUPPORT=y
CONFIG_MTK_BAND_STEERING=y
CONFIG_MTK_LED_CONTROL_SUPPORT=y
CONFIG_MTK_WLAN_HOOK=y
CONFIG_MTK_GREENAP_SUPPORT=y
CONFIG_MTK_AIR_MONITOR=y
CONFIG_MTK_WNM_SUPPORT=y
CONFIG_MTK_INTERWORKING=y
CONFIG_MTK_WPA3_SUPPORT=y
CONFIG_MTK_OWE_SUPPORT=y

CONFIG_MTK_WIFI_MT_MAC=y
CONFIG_MTK_MT_MAC=y
CONFIG_MTK_CHIP_MT7981=y
CONFIG_PACKAGE_kmod_mtfwd=y
CONFIG_PACKAGE_kmod_warp=y
CONFIG_WARP_VERSION=2
CONFIG_WARP_DBG_SUPPORT=y
CONFIG_WED_HW_RRO_SUPPORT=y
CONFIG_WARP_MEMORY_LEAK_DBG=y
CONFIG_WARP_CHIPSET="mt7981"
CONFIG_PACKAGE_wifi_profile=y
CONFIG_first_card=y
CONFIG_first_card_name="MT7981"
CONFIG_first_card_profile_path="/etc/wireless/mediatek/mt7981_ax6000_dbdc_b0_dat;/etc/wireless/mediatek/mt7981_ax6000_dbdc_b1_dat"
CONFIG_first_card_init_script="/lib/wifi/mtwifi_lua;/lib/wifi/mtwifi_lua"
CONFIG_first_card_init_compatible="mtwifi;mtwifi"
CONFIG_first_card_EEPROM_offset="0x0"
CONFIG_first_card_EEPROM_size="0xA0000"
CONFIG_first_card_EEPROM_name="e2p"
CONFIG_first_card_main_ifname="ra0;rai0"
CONFIG_first_card_ext_ifname="ra;rai"
CONFIG_first_card_wds_ifname="wds;wdsx"
CONFIG_first_card_apcli_ifname="apcli;apclix"
CONFIG_first_card_mesh_ifname="mesh;meshx"
CONFIG_first_card_nvram_zone="dev1;dev2"
CONFIG_first_card_single_sku_path="/etc/wireless/mediatek/mt7981_sku_dat"
CONFIG_first_card_bf_sku_path="/etc/wireless/mediatek/mt7981_sku_bf_dat"
CONFIG_WIFI_QUICK_SETTING=y

CONFIG_PACKAGE_libmapd=y

CONFIG_PACKAGE_mtk_base_files=y
CONFIG_PACKAGE_mtk_factory_rw=y












CONFIG_PACKAGE_ip6tables=y
CONFIG_PACKAGE_iptables=y


CONFIG_PACKAGE_miniupnpd=y













CONFIG_OPENLDAP_DEBUG=y


CONFIG_PACKAGE_ip_bridge=y










CONFIG_PACKAGE_lighttpd=y
CONFIG_LIGHTTPD_SSL=y
CONFIG_PACKAGE_lighttpd_mod_cgi=y
CONFIG_PACKAGE_lighttpd_mod_setenv=y
CONFIG_PACKAGE_lighttpd_mod_ssi=y



CONFIG_PACKAGE_arp_scan=y

CONFIG_PACKAGE_odhcp6c=y
CONFIG_PACKAGE_odhcp6c_ext_cer_id=0
CONFIG_PACKAGE_odhcpd_ipv6only=y

CONFIG_PACKAGE_odhcpd_ipv6only_ext_cer_id=0

CONFIG_PACKAGE_ppp=y
CONFIG_PACKAGE_ppp_mod_pppoe=y
CONFIG_PACKAGE_uclient_fetch=y





















CONFIG_PACKAGE_jshn=y
CONFIG_PACKAGE_kvcedit=y
CONFIG_PACKAGE_libjson_script=y
CONFIG_STRACE_NONE=y
CONFIG_PACKAGE_ubi_utils=y



CONFIG_OVERRIDE_PKGS="inadyn mwan3 ntpclient switch"

CONFIG_ATEL_PCB_HS606=y
CONFIG_RT2880_DRAM_128M=y
CONFIG_RALINK_RAM_SIZE=128
CONFIG_RT2880_DRAM_128M=y
CONFIG_RALINK_RAM_SIZE=128
CONFIG_MANUFACTURE_NULL=y
CONFIG_CUSTOMER_NULL=y
CONFIG_DIAL_NULL=y
CONFIG_PACKAGE_atel_api=y
CONFIG_DUAL_IMAGE=y
CONFIG_DUAL_IAMGE_AND_CONFIG=y
CONFIG_KERNEL_PRINTK=y
CONFIG_BUILD_DEBUG=y
CONFIG_BUSYBOX_CONFIG_NETSTAT=y
CONFIG_BUSYBOX_CONFIG_TELNETD=y
CONFIG_BUSYBOX_CONFIG_FEATURE_TELNETD_STANDALONE=y
CONFIG_USER_WAN_ETH_LTE=y
CONFIG_PACKAGE_remoteUpgrade=y
CONFIG_PACKAGE_remoteUpgradefactory=y
CONFIG_PACKAGE_factory_auto=y
CONFIG_PACKAGE_factory_dhcp=y
CONFIG_BUSYBOX_CONFIG_ADDGROUP=y
CONFIG_BUSYBOX_CONFIG_ADDUSER=y
CONFIG_BUSYBOX_CONFIG_LAST_ID=0
CONFIG_BUSYBOX_CONFIG_FIRST_SYSTEM_ID=0
CONFIG_BUSYBOX_CONFIG_LAST_SYSTEM_ID=0
CONFIG_BUSYBOX_CONFIG_SU=y
CONFIG_PACKAGE_dnsmasq=y
CONFIG_SECURITY_AT_ENCRYPT=y
CONFIG_PACKAGE_arp_scan=y
CONFIG_BUSYBOX_CONFIG_TFTP=y
CONFIG_BUSYBOX_CONFIG_FEATURE_TFTP_GET=y
CONFIG_BUSYBOX_CONFIG_FEATURE_TFTP_PUT=y
CONFIG_BUSYBOX_CONFIG_TFTPD=y
CONFIG_PACKAGE_autoreboot_mng=y
CONFIG_USER_AUTOREBOOT_MNG=y
CONFIG_KERNEL_NVRAM=y
CONFIG_PACKAGE_kmod_ralink_nvram=y
CONFIG_PACKAGE_gpio_control=y
CONFIG_PACKAGE_kmod_leds_gpio=y
CONFIG_PACKAGE_kmod_gpio_button_hotplug=y
CONFIG_USER_SMS=y
CONFIG_SUPPORT_SMS=y
CONFIG_USER_SMS=y
CONFIG_SUPPORT_SMS=y
CONFIG_PACKAGE_cgiapi=y
CONFIG_PACKAGE_libmount=y
CONFIG_PACKAGE_libblkid=y
CONFIG_PACKAGE_libsmartcols=y
CONFIG_PACKAGE_libmicroxml=y
CONFIG_PACKAGE_diag_transfer=y
CONFIG_PACKAGE_kmod_usb3=y
CONFIG_PACKAGE_libusb_1_0=y
CONFIG_PACKAGE_kmod_usb_acm=y
CONFIG_PACKAGE_kmod_usb_core=y
CONFIG_PACKAGE_kmod_usb_net=y
CONFIG_PACKAGE_kmod_usb_net_cdc_ether=y
CONFIG_PACKAGE_kmod_usb_net_rndis=y
CONFIG_PACKAGE_serial_test=y
CONFIG_PACKAGE_swconfig=y
CONFIG_PACKAGE_switch=y
CONFIG_USER_RAETHER_SDK=y
