
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
#ifndef MGUI_SMS_H
#define MGUI_SMS_H

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

struct mgui_sms_status {
	char *address;
	char *date;
	char *time;
	char *body;
	int sms_num;
};

struct mgui_sms_context {
	struct mgui_context *mgui;
	uint32_t sms_ubus_id;
	struct mgui_sms_status status;
	pthread_t tid;
};

struct mgui_smskey_context {
	struct mgui_context *mgui;
	pthread_t tid;
};
/******************************************************************************
 *  Function prototypes
 ******************************************************************************/
int mgui_sms_exit(struct mgui_sms_context *ctx);
struct mgui_sms_context *mgui_sms_unread_body_data_get(struct mgui_context *ctx);
void mgui_sms_unread_number(void);
struct mgui_smskey_context *mgui_smskey_init(struct mgui_context *mgui);
int mgui_smskey_exit(struct mgui_smskey_context *ctx);
/*
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
*/
#endif //MGUI_RIL_H

