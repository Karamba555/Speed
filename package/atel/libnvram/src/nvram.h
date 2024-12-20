#ifndef _NVRAM_H
#define _NVRAM_H 	1

//#include <linux/autoconf.h>
#include "linux_conf.h"
#include "user_conf.h"
#include <stddef.h>


#define UBOOT_NVRAM	0
#define RT2860_NVRAM    1
#define RTDEV_NVRAM    	2
#define CERT_NVRAM    	3
#define WAPI_NVRAM    	4

#define FB_2860_BLOCK_NAME "2860"
#define FB_CONFIG2_BLOCK_NAME "config2"

#define MANUFACTURE_LEN	2048

#if defined CONFIG_EXTEND_NVRAM
#define RALINK_NVRAM2_MTDNAME "Config2"
#define EXTEND_BLOCK_NUM 2
#define CONFIG2_NVRAM 		WAPI_NVRAM+1
#if defined CONFIG_CONFIG_SHRINK
#define VOIP_NVRAM			RT2860_NVRAM
#else
#define VOIP_NVRAM			CONFIG2_NVRAM
#endif
#define TR069CERT_NVRAM    	CONFIG2_NVRAM+1
#else
#define VOIP_NVRAM			RT2860__NVRAM
#define EXTEND_BLOCK_NUM 0
#define CONFIG2_NVRAM           RT2860_NVRAM
#endif

#define NV_DEV "/dev/nvram"
#define RALINK_NVRAM_IOCTL_GET		0x01
#define RALINK_NVRAM_IOCTL_SET		0x03
#define RALINK_NVRAM_IOCTL_COMMIT	0x04
#define RALINK_NVRAM_IOCTL_CLEAR	0x05
#define RALINK_BLOCK_IOCTL_GET		0x06

#define RALINK_KEY_IOCTL_GET		0x07
#define RALINK_KEY_IOCTL_SET		0x08
#define RALINK_NVRAM_IOCTL_GETALL	0x09

typedef struct environment_s {
	unsigned long crc;		//CRC32 over data bytes
	char *data;
} env_t;

typedef struct cache_environment_s {
	char *name;
	char *value;
} cache_t;

#define MAX_CACHE_ENTRY 3000
typedef struct block_s {
	char *name;
	env_t env;			//env block
	cache_t	cache[MAX_CACHE_ENTRY];	//env cache entry by entry
	unsigned long flash_offset;
	unsigned long flash_max_len;	//ENV_BLK_SIZE

	char valid;
	char dirty;
} block_t;

#define MAX_NAME_LEN 128
#define MAX_VALUE_LEN (ENV_BLK_SIZE * 5)
typedef struct nvram_ioctl_s {
	int index;
	int ret;
	char *name;
	char *value;
} nvram_ioctl_t;

typedef struct _para_crypto {
	char name[32];
	char hwid[32];
} para_crypto;

enum pad_mode {
	PADDING_ZERO,
	PADDING_PKCS7,
};

typedef struct _para_extend {
	int with_base64;
	enum pad_mode pad;
	unsigned char *user_key;
	unsigned char *user_iv;
} para_extend;

enum standard_crypt_mode {
	AES_CBC_None_S,
	AES_CBC_Encrypt_Common,
	AES_CBC_Decrypt_Common,
	AES_CBC_Encrypt_p12,
	AES_CBC_Decrypt_p12,
};

enum base64_mode {
	WITHOUT_BASE64,
	WITH_BASE64,
};

#ifdef CONFIG_DUAL_IMAGE
#define FLASH_BLOCK_NUM	5
#else
#define FLASH_BLOCK_NUM	4
#endif

#if defined(CONFIG_USER_SECURITY)
extern int user_id;
#define seteuid_to_root(root) do {	\
	user_id = geteuid();			\
	if (user_id) seteuid(root);		\
} while (0)

#define seteuid_to_user(user_id) do {	\
	if (user_id) seteuid(user_id);		\
} while (0)
#else
#define seteuid_to_root(root)
#define seteuid_to_user(user_id)
#endif

#if  defined (CONFIG_USER_RENAME_FLASHMTD) 
char const *keymtd_bufget(void);
#define ATEL_flashkey    keymtd_bufget()
#define ATEL_FLASH       "aatelcmd1"
#define ATEL_MTDWRITE  "/sbin/aatelcmd2"
#else
#define ATEL_flashkey  "-k"
#define ATEL_FLASH  "flash"
#define ATEL_MTDWRITE  "/sbin/mtd"
#define ATEL_SYSUPGRADE  "/sbin/sysupgrade"
#endif

//#define SYS_DEBUG	1
#define SYSTEM_WEB_FILE	"/tmp/allsyslog"
void nvram_init(int index);
void nvram_close(int index);

int nvram_set(int index, char *name, char *value);
const char *nvram_get(int index, char *name);
int nvram_bufset(int index, char *name, char *value);
char const *nvram_bufget(int index, char *name);
char const *pure_nvram_bufget(int index, char *name);
int pure_nvram_bufset(int index, char *name, char *value);
#ifdef CONFIG_CUSTOMER_ORANGE
static char *array_passwords[] = {"DDNSPassword","TR069Password","accesspassword","WPAPSK1","remoteUploadPassword","TR069DownPassword","TR069UploadPassword","VoIPRegisterPassword2","lte_apn_pw","CwmpInformConReqPwd","lte_netlock_code","Tr069UniCertPwd","Password",NULL};
#else
static char *array_passwords[] = {"DDNSPassword","TR069Password","accesspassword","WPAPSK1","remoteUploadPassword","TR069DownPassword","TR069UploadPassword","VoIPRegisterPassword2","lte_apn_pw","CwmpInformConReqPwd","lte_netlock_code",NULL};
#endif

void nvram_buflist(int index);
int nvram_commit(int index);
int nvram_clear(int index);
int nvram_erase(int index);

int getNvramNum(void);
unsigned int getNvramOffset(int index);
unsigned int getNvramBlockSize(int index);
char *getNvramName(int index);
unsigned int getNvramIndex(char *name);
void toggleNvramDebug(void);
#if defined (CONFIG_DUAL_IAMGE_AND_CONFIG) 
int block_get(int *value);
#endif

void atelLog(int level,int severity,const char *msg, ...);
void atellogClean(void);
void atelReadFlashLog();
#endif	//end of _NVRAM_H

#if defined CONFIG_CUSTOMER_POLSAT
    #if defined CONFIG_ATEL_PCB_ALR_PT30
        #define NVRAM_ADVANCE_PASSWORD "I3m@St3r&Yek"
    #else
        #define NVRAM_ADVANCE_PASSWORD "I3m@St3r&Yek"
    #endif
#elif defined CONFIG_CUSTOMER_ORANGE
#define NVRAM_ADVANCE_PASSWORD "Chin3$3#T34!"
#elif defined CONFIG_CUSTOMER_PHILIPPINES
#define NVRAM_ADVANCE_PASSWORD "3V0Lu2n@DvAn6E"
#else
#define NVRAM_ADVANCE_PASSWORD "@PWT1l&mOBI5"
#endif

