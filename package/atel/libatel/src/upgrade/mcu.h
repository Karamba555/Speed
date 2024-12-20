#ifndef __MCU_H__
#define __MCU_H__

#include "user_conf.h"

#if 1
#define LOG_FILE	"/dev/console"
#define cprintf(fmt, arg...)    do {    FILE *log_fp = fopen(LOG_FILE, "w+"); \
	fprintf(log_fp, "--> [%s %d] ", __func__, __LINE__);	\
    fprintf(log_fp,fmt, ##arg); \
    fclose(log_fp); \
} while (0)

#define dbg(fmt, arg...)    do {    FILE *log_fp = fopen(LOG_FILE, "w+"); \
    fprintf(log_fp,fmt, ##arg); \
    fclose(log_fp); \
} while (0)
#else
#define cprintf(fmt, arg...)
#define dbg(fmt, arg...)
#endif

#if defined(CONFIG_ATEL_PCB_SW12)
#define MCU_RESET() do {	\
	system("gpio w 483");	\
	system("gpio c 483");	\
	usleep(50000);			\
} while (0)

#define HOST_POWER_HOLD_ON() do {	\
	system("gpio w 466");	\
} while (0)

#define HOST_POWER_HOLD_OFF() do {	\
	system("gpio c 466");	\
} while (0)
#else
/* other pcb product */
#endif

//#define SUPPORT_RETRANSLATE

#define FILE_SIZE 			0xC000
#define BLOCK_SIZE 			56
#define BLOCK_NUM 			(FILE_SIZE/BLOCK_SIZE)
#define PER_SENT_SIZE 32

#define I2C_BUS				0
#define I2C_ADDR			0x33	/* slave address is 0x66, 0x66 = 0x33 >> 1, 7-bits + 1bit(read/write) */

enum mcu_event_type {
	EVENT_UPGRADE = 0x12,
	EVENT_NO_MAX,
};

enum upgrade_header_idx {
	UPGRADE_HEADER_EVENT,
	UPGRADE_HEADER_STATE,
	UPGRADE_HEADER_CKM,
	UPGRADE_HEADER_LEN,
	UPGRADE_HEADER_NO_HIGH,
	UPGRADE_HEADER_NO_LOW,
	UPGRADE_HEADER_BODY,
};

enum upgrade_stat {
	UPGRADE_TRIGGER = 1,
	UPGRADEING,
	UPGRADE_CHECKSUM,
	UPGRADE_FINISH,
};

int ckm_idx = 0;
int check_sum_total[480] = {0};

int mcu_upgrade(char* path, int offset);

#endif
