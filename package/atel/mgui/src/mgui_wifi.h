
/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_wifi.h
 *
 *  Authors:  Tomer Eliyahu
 *
 *  Description: mgui interface to wifi
 *
 *  HISTORY:
 *   Jun 14, 2015 - Initial Version
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef MGUI_WIFI_H
#define MGUI_WIFI_H

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

struct mgui_wifi_status {
	char *device;
	char *on;
	char *channel;
	char ssid[128];
	char *key;
	char *encryption;
	int wifi_status;
	int num_clients;
	int usb_status;
	char ui_ssid[128];
};

struct mgui_wifi_context {
	struct mgui_context *mgui;
	uint32_t wifi_ubus_id;
	uint32_t statistics_ubus_id;
	struct ubus_subscriber ubus_subscriber;

	struct mgui_wifi_status status; /* wifi status database */
};

/******************************************************************************
 *  Function prototypes
 ******************************************************************************/
struct mgui_wifi_context *mgui_wifi_init(struct mgui_context *mgui);
int mgui_wifi_exit(struct mgui_wifi_context *ctx);

static inline int get_wifi_status(struct mgui_wifi_context *ctx)
{
	return ctx->status.wifi_status;
}

static inline const char *get_wifi_channel(struct mgui_wifi_context *wifi)
{
	return wifi->status.channel;
}

static inline const char *get_wifi_ssid(struct mgui_wifi_context *wifi)
{
	return wifi->status.ssid;
}
static inline int get_active_clients_num(struct mgui_wifi_context *wifi)
{
	return wifi->status.num_clients;
}

#endif /* MGUI_WIFI_H */

