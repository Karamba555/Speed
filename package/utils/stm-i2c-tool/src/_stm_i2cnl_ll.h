/*
 * _stm_i2cnl_ll.h
 *
 *  Created on: 24 Oct 2024
 *      Author: Sergey Frolov
 */

#ifndef __STM_I2CNL_LL_H_
#define __STM_I2CNL_LL_H_


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>
#include <stdint.h>

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <semaphore.h>


#define FW_FILENAME             "app.bin"
#define FW_MAX_SIZE             0x4000


#define I2C_CHIP_ADDR           0x33

#define I2C_PACKET_LEN          64
#define I2C_HEADER_LEN          8
#define I2C_PAYLOAD_LEN         (I2C_PACKET_LEN - I2C_HEADER_LEN)
#define I2C_RTC_PACKET_LEN      (24 + I2C_HEADER_LEN)

enum
{
    STMI2C_OK,
    STMI2C_ERROR,
    STMI2C_RESET,
    STMI2C_ERASE,
    STMI2C_WRITE,
    STMI2C_BUSY,
    STMI2C_CMPT,
    STMI2C_TIMEOUT
};


enum upgrade_stat {
	UPGRADE_TRIGGER = 1,
	UPGRADE_ING,
	UPGRADE_CHECKSUM,
	UPGRADE_FINISH,
	UPGRADE_CKM_FLAG,
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

enum event_type {
	EVENT_NULL = 0x0,
	EVENT_VERSION = 0x01,
	EVENT_WDG = 0x02,
	EVENT_RTC_GET = 0x03,
	EVENT_RTC_SET = 0x04,
	EVENT_UPGRADE = 0x12,
	EVENT_VST_SET = 0x13,
	EVENT_STDBY_CD = 0x14,
	EVENT_COIN_BATT_VLT = 0x15,
	EVENT_SECURITY = 0x20,
	EVENT_ACCELER = 0x23,
	EVENT_CYC_POWER_CPU = 0x32,
	EVENT_MAX,
};

enum subevent
{
    I2C_RTC_GET_EMPTY,
    I2C_RTC_GET_TIME,
    I2C_RTC_GET_DATE,
};

enum
{
    I2C_THREAD_RST,
    I2C_THREAD_UPD
};


typedef struct
{
    uint8_t h_event;
    uint8_t h_state;
    uint8_t h_ckm;
    uint8_t h_txlen;
    uint8_t h_nohigh;
    uint8_t h_nolow;
}STMI2C_UpdHeader_TD;

typedef struct
{
    uint8_t h_event;
    uint8_t h_state;
    uint8_t h_ckm;
    uint8_t h_txlen;
    uint8_t h_nohigh;
    uint8_t h_nolow;
    uint8_t h_body;
    uint8_t h_rxlen;
}STMI2C_TxHeader_TD;


typedef struct
{
    STMI2C_UpdHeader_TD         updheader;
    uint8_t                     payload[I2C_PAYLOAD_LEN];
}STMI2C_UpdateBuffer_TD;

typedef struct
{
    STMI2C_TxHeader_TD          txheader;
    uint8_t                     payload[I2C_PAYLOAD_LEN];
}STMI2C_TxBuffer_TD;

typedef struct
{
    uint8_t                     payload[I2C_PACKET_LEN];
}STMI2C_RxBuffer_TD;

typedef struct
{
    uint8_t                     Hours;
    uint8_t                     Minutes;
    uint8_t                     Seconds;
    uint8_t                     TimeFormat;
    uint32_t                    SubSeconds;
    uint32_t                    SecondFraction;
    uint32_t                    DayLightSaving;
    uint32_t                    StoreOperation;
    uint8_t                     WeekDay;
    uint8_t                     Month;
    uint8_t                     Date;
    uint8_t                     Year;
}STMI2C_RTC_TD;



typedef struct
{
    FILE                        *file;
    uint32_t                    fsize;
    uint32_t                    cursor_pos;
    uint32_t                    bytes_left;
    uint32_t                    packet_num;
    uint8_t                     progress;
    uint8_t                     state;
    uint8_t                     error_cnt;
}STMI2C_File_TD;


typedef struct
{
    int                         i2c_h;
    STMI2C_File_TD              fwfile;
    STMI2C_UpdateBuffer_TD      upddata;
    STMI2C_TxBuffer_TD          txdata;
    STMI2C_RxBuffer_TD          rxdata;
    STMI2C_RTC_TD               rtc;
    uint32_t                    vst;
    uint32_t                    stdbycd;
    pthread_t                   wpthread;
    int                         status;
    int                         fopsid;
    int                         nlready;
}STMI2C_Tool_TD;




typedef struct
{
    int (*GetVersion)(STMI2C_Tool_TD *stmi2c);
    int (*GetRTC)(STMI2C_Tool_TD *stmi2c, int subevent);
    int (*SetRTC)(STMI2C_Tool_TD *stmi2c, char *hour, char *min, char *sec,
                                           char *date, char *month, char *year);
    int (*Upgrade)(STMI2C_Tool_TD *stmi2c);
    int (*SetVST)(STMI2C_Tool_TD *stmi2c, char *vst);
    int (*GetSTBCD)(STMI2C_Tool_TD *stmi2c);
    int (*ResetCPU)(STMI2C_Tool_TD *stmi2c);
    int (*ResetMCU)(STMI2C_Tool_TD *stmi2c);
    int (*StartFW)(STMI2C_Tool_TD *stmi2c);
}STMI2C_Func_TD;



int _stm_i2cnl_open(STMI2C_Tool_TD *stmi2c);
int _stm_i2cnl_write(STMI2C_Tool_TD *stmi2c, uint8_t *data, uint32_t len);
int _stm_i2cnl_read(STMI2C_Tool_TD *stmi2c, uint8_t *data, uint32_t len);


#endif /* __STM_I2CNL_LL_H_ */