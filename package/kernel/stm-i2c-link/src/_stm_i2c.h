/*
 * stm_i2c.h
 *
 *  Created on: 24 Oct 2024
 *      Author: Sergey Frolov
 */

#ifndef __STM_I2C_H_
#define __STM_I2C_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/types.h>


#define STM_I2C_BUS                      0
#define STM_I2C_ADDR                     0x33
#define STM_I2C_PING                     0x80


#define I2C_PACKET_LEN					 64
#define I2C_HEADER_LEN					 8

enum
{
    STMI2C_OK,
    STMI2C_ERROR,
	STMI2C_BUSY,
	STMI2C_CMPT,
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


int _stm_i2c_ping(void);
int _stm_i2c_transfer(uint8_t *txdata, uint8_t *rxdata, int txlen, int rxlen);

#endif