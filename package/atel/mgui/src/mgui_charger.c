/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_charger.c
 *
 *  Authors:  Tomer Eliyahu
 *
 *  Description: MGUI charger / battery interface
 *
 *  HISTORY:
 *
 *   Jun 14, 2015 - Initial Version
 *
 *  Notes:
 *
 ******************************************************************************/

/******************************************************************************
 *   Include files
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mgui_utils.h"
#include "mgui_ubus.h"
#include "mgui_icons.h"
#include "mgui_charger.h"
#include "mgui.h"

/******************************************************************************
 *   Structures
 ******************************************************************************/
enum {
	ATTR_CAPACITY = 0,
	ATTR_TYPE,
	ATTR_PRESENT,
	ATTR_HEALTH,
	ATTR_VOLTAGE,
	ATTR_TEMP,
	ATTR_STATUS,
	ATTR_TECH,
	ATTR_AC_ONLINE,
	ATTR_AC_TYPE,
	ATTR_AC_STATUS,
	ATTR_USB_ONLINE,
	ATTR_USB_TYPE,
	ATTR_USB_STATUS,
	NR_ATTR,
};

/******************************************************************************
 *   Definitions
 ******************************************************************************/

#define CHARGER_UBUS_ID	"aoc"
#define CHARGER_UBUS_REQUEST	"get_chg_info"

#define to_ubus_subscriber(obj) container_of(obj, struct ubus_subscriber, obj)
#define to_charger_context(obj) container_of(to_ubus_subscriber(obj), struct mgui_charger_context, charger_ubus_subscriber)

/******************************************************************************
 *   Globals
 ******************************************************************************/
static const struct blobmsg_policy charger_attr_policy[] = {
	[ATTR_CAPACITY] = {.name = "capacity_value", .type = BLOBMSG_TYPE_INT32},
	[ATTR_TYPE] = {.name = "type", .type = BLOBMSG_TYPE_INT32},
	[ATTR_PRESENT] = {.name = "present", .type = BLOBMSG_TYPE_INT32},
	[ATTR_HEALTH] = {.name = "health", .type = BLOBMSG_TYPE_INT32},
	[ATTR_VOLTAGE] = {.name = "voltage_now", .type = BLOBMSG_TYPE_INT32},
	[ATTR_TEMP] = {.name = "bat_temp", .type = BLOBMSG_TYPE_INT32},
	[ATTR_STATUS] = {.name = "bat_status", .type = BLOBMSG_TYPE_INT32},
	[ATTR_TECH] = {.name = "bat_technology", .type = BLOBMSG_TYPE_INT32},
	[ATTR_AC_ONLINE] = {.name = "acchg_online", .type = BLOBMSG_TYPE_INT32},
	[ATTR_AC_TYPE] = {.name = "acchg_type", .type = BLOBMSG_TYPE_INT32},
	[ATTR_AC_STATUS] = {.name = "acchg_status", .type = BLOBMSG_TYPE_INT32},
	[ATTR_USB_ONLINE] = {.name = "usbchg_online", .type = BLOBMSG_TYPE_INT32},
	[ATTR_USB_TYPE] = {.name = "usbchg_type", .type = BLOBMSG_TYPE_INT32},
	[ATTR_USB_STATUS] = {.name = "usbchg_status", .type = BLOBMSG_TYPE_INT32},
};

static	struct blob_buf b;

/******************************************************************************
 *  Code
 ******************************************************************************/

static int charger_parseattrs_tag(struct mgui_charger_context *ctx, struct blob_attr *msg)
{
	struct blob_attr *tb[NR_ATTR];
	int ret;

	MASSERT(ctx);

	ret =  blobmsg_parse(charger_attr_policy, NR_ATTR, tb, blob_data(msg), blob_len(msg));
	if (ret){
		MGUI_EMSG("parsing blobmsg failed %d\n", ret);
		return -1;
	}

	ctx->status.capacity = tb[ATTR_CAPACITY] ?
		blobmsg_get_u32(tb[ATTR_CAPACITY]) : 0;
	ctx->status.type = tb[ATTR_TYPE] ?
		blobmsg_get_u32(tb[ATTR_TYPE]) : 0;
	ctx->status.present = tb[ATTR_PRESENT] ?
		blobmsg_get_u32(tb[ATTR_PRESENT]) : 0;
	ctx->status.health = tb[ATTR_HEALTH] ?
		blobmsg_get_u32(tb[ATTR_HEALTH]) : 0;
	ctx->status.voltage_now = tb[ATTR_VOLTAGE] ?
		blobmsg_get_u32(tb[ATTR_VOLTAGE]) : 0;
	ctx->status.bat_temp = tb[ATTR_TEMP] ?
		blobmsg_get_u32(tb[ATTR_TEMP]) : 0;
	ctx->status.bat_status = tb[ATTR_STATUS] ?
		blobmsg_get_u32(tb[ATTR_STATUS]) : 0;
	ctx->status.bat_technology = tb[ATTR_TECH] ?
		blobmsg_get_u32(tb[ATTR_TECH]) : 0;
	ctx->status.acchg_online = tb[ATTR_AC_ONLINE] ?
		blobmsg_get_u32(tb[ATTR_AC_ONLINE]) : 0;
	ctx->status.acchg_type = tb[ATTR_AC_TYPE] ?
		blobmsg_get_u32(tb[ATTR_AC_TYPE]) : 0;
	ctx->status.acchg_status = tb[ATTR_AC_STATUS] ?
		blobmsg_get_u32(tb[ATTR_AC_STATUS]) : 0;
	ctx->status.usbchg_online = tb[ATTR_USB_ONLINE] ?
		blobmsg_get_u32(tb[ATTR_USB_ONLINE]) : 0;
	ctx->status.usbchg_type = tb[ATTR_USB_TYPE] ?
		blobmsg_get_u32(tb[ATTR_USB_TYPE]) : 0;
	ctx->status.usbchg_status = tb[ATTR_USB_STATUS] ?
		blobmsg_get_u32(tb[ATTR_USB_STATUS]) : 0;

	return 0;
}
//#define MGUI_DEBUG
#ifdef MGUI_DEBUG
static void dump_chg_bat_tag(struct mgui_charger_context *ctx)
{
	MASSERT(ctx);

	printf("\nBattery capacity: %d/100\n",ctx->status.capacity);
	printf("    Battery type: %d\n",ctx->status.type);
	printf(" Battery present: %d\n",ctx->status.present);
	printf("  Battery health: %d\n",ctx->status.health);
	printf(" Battery voltage: %d\n",ctx->status.voltage_now);
	printf("    Battery temp: %d\n",ctx->status.bat_temp);
	printf("  Battery status: %d\n",ctx->status.bat_status);
	printf("    Battery tech: %d\n",ctx->status.bat_technology);
	printf("    Acchg online: %d\n",ctx->status.acchg_online);
	printf("      Acchg type: %d\n",ctx->status.acchg_type);
	printf("    Acchg status: %d\n",ctx->status.acchg_status);
	printf("   USBchg online: %d\n",ctx->status.usbchg_online);
	printf("     USBchg type: %d\n",ctx->status.usbchg_type);
	printf("   USBchg status: %d\n",ctx->status.usbchg_status);	
}
#endif

static int charger_indication_cb(struct ubus_context *ctx,
				  struct ubus_object *obj,
				  struct ubus_request_data *req,
				  const char *method, struct blob_attr *msg)
{
	struct mgui_charger_context *charger = to_charger_context(obj);
	int ret = 0;
	
	MGUI_DMSG("charger / battery indication received\n");
	
	ret = charger_parseattrs_tag(charger, msg);
	if (ret){
		MGUI_EMSG("parsing blobmsg failed %d\n", ret);
		return -1;
	}

#ifdef MGUI_DEBUG
	dump_chg_bat_tag(charger);
#endif
/*
	if (mgui_update_icon(charger->mgui, MGUI_BATTERY_ICON, (void *)charger))
		mgui_screen_refresh(charger->mgui);

	return 0;
*/
}


static void charger_complete_cb(struct ubus_request *req, int ret)
{
	struct mgui_charger_context *charger = (struct mgui_charger_context *)req->priv;
	
	MGUI_DMSG("charger complete received\n");
	DBG_MSG("mgui_screen_refresh\n");
	/* trigger screen refresh since this is a result of an async request */
	mgui_screen_refresh(charger->mgui);
}

static void charger_data_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct mgui_charger_context *charger = (struct mgui_charger_context *)req->priv;
	int ret;

	MASSERT(charger);

	MGUI_DMSG("charger / battery data received\n");
	
	ret = charger_parseattrs_tag(charger, msg);
	if (ret){
		MGUI_EMSG("parsing blobmsg failed %d\n", ret);
		return;
	}
#ifdef MGUI_DEBUG
	dump_chg_bat_tag(charger);
#endif
	mgui_update_icon(charger->mgui, MGUI_BATTERY_ICON, (void *)charger);
}

static inline int mgui_charger_request(struct mgui_charger_context *ctx)
{
	return mgui_ubus_invoke_async(ctx->mgui->ubus, &b, ctx->charger_ubus_id,
			    CHARGER_UBUS_REQUEST, charger_data_cb, charger_complete_cb, ctx);
}

/**
 * mgui_charger_exit
 * De-initialize charger interface
 * 
 * @param charger pointer to previously allocated charger context
 * 
 * @return 0 for success, error code otherwise
 */
int mgui_charger_exit(struct mgui_charger_context *charger)
{
	if(!charger) {
		MGUI_IMSG("charger module not running\n");
		return 0;
	}

	mgui_ubus_unsubscribe(charger->mgui->ubus,
			      &charger->charger_ubus_subscriber,
			      charger->charger_ubus_id);
	mgui_ubus_unregister_subscriber(charger->mgui->ubus,
				    &charger->charger_ubus_subscriber);

	free(charger);

	MGUI_IMSG("charger exit done\n");

	return 0;
}

/**
 * mgui_charger_init
 * Initialize mgui charger interface
 * 
 * NOTE - must be called BEFORE uloop_run()!
 * 
 * @param mgui   mgui context
 * 
 * @return pointer to mgui_charger_context
 */
struct mgui_charger_context *mgui_charger_init(struct mgui_context *mgui)
{
	struct mgui_charger_context *charger;
	int ret;

	MASSERT(mgui);
	MASSERT(mgui->ubus);

	charger = malloc(sizeof(struct mgui_charger_context));
	if (!charger) {
		MGUI_EMSG("memory allocation failed\n");
		return NULL;
	}

	memset(charger, 0, sizeof(*charger));

	charger->mgui = mgui;

	ret = mgui_ubus_lookup_id(charger->mgui->ubus, CHARGER_UBUS_ID,
				  &charger->charger_ubus_id);
	if (ret) {
		MGUI_EMSG("mubus_lookup_id failed (ret=%d)\n", ret);
		goto out_error;
	}

	/* get initial charger / battery status */
	ret = mgui_charger_request(charger);
	if (ret) {
		MGUI_EMSG("initial request failed\n");
		goto out_error;
	}

	charger->charger_ubus_subscriber.cb = charger_indication_cb;
	ret = mgui_ubus_register_subscriber(charger->mgui->ubus,
					    &charger->charger_ubus_subscriber);
	if (ret) {
		MGUI_EMSG("mubus_register_subscriber failed\n");
		goto out_error;
	}

	ret = mgui_ubus_subscribe(charger->mgui->ubus,
				  &charger->charger_ubus_subscriber,
				  charger->charger_ubus_id);
	if (ret) {
		MGUI_EMSG("ubus subscribe failed\n");
		goto unsubscribe;
	}

	MGUI_IMSG("charger init done\n");

	return charger;

unsubscribe:
	mgui_ubus_unsubscribe(charger->mgui->ubus,
			      &charger->charger_ubus_subscriber,
			      charger->charger_ubus_id);
out_error:
	free(charger);
	return NULL;
}
