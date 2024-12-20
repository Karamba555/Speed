
/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_charger.h
 *
 *  Authors:  Tomer Eliyahu
 *
 *  Description: mgui interface to charger
 *
 *  HISTORY:
 *   Jun 14, 2015 - Initial Version
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef MGUI_CHARGER_H
#define MGUI_CHARGER_H

/******************************************************************************
 *   Include files
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "mgui_ubus.h"

/******************************************************************************
 *   Macros
 ******************************************************************************/

/******************************************************************************
 *   Structures
 ******************************************************************************/

enum chg_status_stat {
	STATUS_UNKNOWN = 0,
	STATUS_CHARGING,
	STATUS_DISCHARGING,
	STATUS_NOT_CHARGING,
	STATUS_FULL,
};

struct chg_bat_tag {
	unsigned int capacity;
	unsigned int health;
	unsigned int present;
	unsigned int type;
	unsigned int voltage_now;
	unsigned int bat_status;
	unsigned int bat_temp;
	unsigned int bat_technology;
	unsigned int acchg_online;
	unsigned int acchg_type;
	unsigned int acchg_status;
	unsigned int usbchg_online;
	unsigned int usbchg_type;
	unsigned int usbchg_status;
};

struct mgui_charger_context {
	struct mgui_context *mgui;
	uint32_t charger_ubus_id;
	struct ubus_subscriber charger_ubus_subscriber;

	struct chg_bat_tag status; /* charger / battery status database */
};

/******************************************************************************
 *  Function prototypes
 ******************************************************************************/
struct mgui_charger_context *mgui_charger_init(struct mgui_context *mgui);
int mgui_charger_exit(struct mgui_charger_context *ctx);

static inline int get_chg_bat_status(struct mgui_charger_context *ctx)
{
	if (ctx->status.present)
		return ctx->status.bat_status;

	return STATUS_UNKNOWN;
}

static inline int get_chg_bat_capacity(struct mgui_charger_context *ctx)
{
	return ctx->status.capacity;
}

#endif //MGUI_RIL_H

