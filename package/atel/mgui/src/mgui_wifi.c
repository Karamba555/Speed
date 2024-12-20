/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_wifi.c
 *
 *  Authors:  Tomer Eliyahu
 *
 *  Description: MGUI wifi / battery interface
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
#include <inttypes.h>
#include "mgui_utils.h"
#include "mgui_ubus.h"
#include "mgui_icons.h"
#include "mgui_wifi.h"
#include "mgui.h"

/******************************************************************************
 *   Structures
 ******************************************************************************/

/******************************************************************************
 *   Definitions
 ******************************************************************************/
//#define WIFI_DEBUG

#define WIFI_UBUS_ID			"wireless"
#define WIFI_UBUS_REQUEST		"wifi_get_detail"

#define STATISTICS_UBUS_ID		"statistics"
#define STATISTICS_UBUS_REQUEST	"get_wifi_status"
//#define ACTIVE_CLIENTS_NUM_REQUEST "get_active_clients_num"
#define ACTIVE_CLIENTS_NUM_REQUEST "stat_get_common_data"

#define to_ubus_subscriber(obj) container_of(obj, struct ubus_subscriber, obj)
#define to_wifi_context(obj) container_of(to_ubus_subscriber(obj), struct mgui_wifi_context, ubus_subscriber)

/******************************************************************************
 *   Globals
 ******************************************************************************/

/* wireless get_basic_info */
enum {
	ATTR_WIRELESS = 0,
	ATTR_AP1,
};

enum {
	ATTR_WIFI_IF_2G,
	ATTR_DEVICE,
	ATTR_SWITCH,
	ATTR_CHANNEL,
	ATTR_SSID0,
	ATTR_SSID,
	ATTR_ENCRYPTION,
	ATTR_KEY,
};

enum {
	ATTR2_WIFI_IF_5G,
	ATTR2_DEVICE,
	ATTR2_SWITCH,
	ATTR2_CHANNEL,
	ATTR2_SSID0,
	ATTR2_SSID,
	ATTR2_ENCRYPTION,
	ATTR2_KEY,
};


static const struct blobmsg_policy wifi_info_attr_policy[] = {
	[ATTR_WIRELESS] = {.name = "wireless", .type = BLOBMSG_TYPE_TABLE},
	[ATTR_AP1] = {.name = "AP1", .type = BLOBMSG_TYPE_TABLE},
};

static const struct blobmsg_policy wifi_2G_attr_policy[] = {
	[ATTR_WIFI_IF_2G] = {.name = "wifi_if_24G", .type = BLOBMSG_TYPE_TABLE},
	[ATTR_DEVICE] = {.name = "device", .type = BLOBMSG_TYPE_STRING},
	[ATTR_SWITCH] = {.name = "switch", .type = BLOBMSG_TYPE_STRING},
	[ATTR_CHANNEL] = {.name = "channel", .type = BLOBMSG_TYPE_STRING},
	[ATTR_SSID0] = {.name = "ssid0", .type = BLOBMSG_TYPE_TABLE},
	[ATTR_SSID] = {.name = "ssid", .type = BLOBMSG_TYPE_STRING},
	[ATTR_ENCRYPTION] = {.name = "encryption", .type = BLOBMSG_TYPE_STRING},
	[ATTR_KEY] = {.name = "key", .type = BLOBMSG_TYPE_STRING},
};

static const struct blobmsg_policy wifi_5G_attr_policy[] = {
	[ATTR2_WIFI_IF_5G] = {.name = "wifi_if_5G", .type = BLOBMSG_TYPE_TABLE},
	[ATTR2_DEVICE] = {.name = "device", .type = BLOBMSG_TYPE_STRING},
	[ATTR2_SWITCH] = {.name = "switch", .type = BLOBMSG_TYPE_STRING},
	[ATTR2_CHANNEL] = {.name = "channel", .type = BLOBMSG_TYPE_STRING},
	[ATTR2_SSID0] = {.name = "ssid0", .type = BLOBMSG_TYPE_TABLE},
	[ATTR2_SSID] = {.name = "ssid", .type = BLOBMSG_TYPE_STRING},
	[ATTR2_ENCRYPTION] = {.name = "encryption", .type = BLOBMSG_TYPE_STRING},
	[ATTR2_KEY] = {.name = "key", .type = BLOBMSG_TYPE_STRING},
};
#if 1
/* statistics get_active_clients_num*/
enum {
	ATTR_CLIENTS_INFO,
	ATTR_ACTIVE_CLIENTS_NUM,
	ATTR_USB_STATE,
};
#else
/* statistics stat_get_common_data*/
enum {
	ATTR_STATISTICS,
	ATTR_ACTIVE_CLIENTS_NUM,
};
#endif

static const struct blobmsg_policy active_clients_num_attr_policy[] = {
	[ATTR_CLIENTS_INFO] = {.name = "clients_info", .type = BLOBMSG_TYPE_TABLE},
	[ATTR_ACTIVE_CLIENTS_NUM] = {.name = "active_clients_num", .type = BLOBMSG_TYPE_STRING},
	[ATTR_USB_STATE] = {.name = "usb_state", .type = BLOBMSG_TYPE_STRING},
};


/* statistics get_wifi_status */
enum {
	ATTR_STATISTICS,
	ATTR_WIFI_STATUS,
};

static const struct blobmsg_policy wifi_status_attr_policy[] = {
	[ATTR_STATISTICS] = {.name = "statistics", .type = BLOBMSG_TYPE_TABLE},
	[ATTR_WIFI_STATUS] = {.name = "wifi_status", .type = BLOBMSG_TYPE_STRING},
};

static	struct blob_buf b;
extern char g_ssid[];
extern char g_key[];
extern char g_client_n[];
/******************************************************************************
 *  Code
 ******************************************************************************/

#ifdef WIFI_DEBUG
static const char *indent_str = "\t\t\t\t\t\t\t\t\t\t\t\t\t";

#define indent_printf(indent, ...) do { \
	if (indent > 0) \
		fwrite(indent_str, indent, 1, stderr); \
	fprintf(stderr, __VA_ARGS__); \
} while(0)

static void dump_attr_data(struct blob_attr *data, int indent, int next_indent);

static void
dump_table(struct blob_attr *head, int len, int indent, bool array)
{
	struct blob_attr *attr;
	struct blobmsg_hdr *hdr;

	indent_printf(indent, "{\n");
	__blob_for_each_attr(attr, head, len) {
		hdr = blob_data(attr);
		if (!array)
			indent_printf(indent + 1, "%s : ", hdr->name);
		dump_attr_data(attr, 0, indent + 1);
	}
	indent_printf(indent, "}\n");
}

static void dump_attr_data(struct blob_attr *data, int indent, int next_indent)
{
	int type = blobmsg_type(data);
	switch(type) {
	case BLOBMSG_TYPE_STRING:
		indent_printf(indent, "%s\n", blobmsg_get_string(data));
		break;
	case BLOBMSG_TYPE_INT8:
		indent_printf(indent, "%d\n", blobmsg_get_u8(data));
		break;
	case BLOBMSG_TYPE_INT16:
		indent_printf(indent, "%d\n", blobmsg_get_u16(data));
		break;
	case BLOBMSG_TYPE_INT32:
		indent_printf(indent, "%d\n", blobmsg_get_u32(data));
		break;
	case BLOBMSG_TYPE_INT64:
		indent_printf(indent, "%"PRIu64"\n", blobmsg_get_u64(data));
		break;
	case BLOBMSG_TYPE_TABLE:
	case BLOBMSG_TYPE_ARRAY:
		if (!indent)
			indent_printf(indent, "\n");
		dump_table(blobmsg_data(data), blobmsg_data_len(data),
			   next_indent, type == BLOBMSG_TYPE_ARRAY);
		break;
	}
}
#endif /* WIFI_DEBUG */

static int wifi_info_parse_blobmsg(struct mgui_wifi_context *ctx, struct blob_attr *msg)
{
	struct blob_attr *tb[ARRAY_SIZE(wifi_info_attr_policy)];
	struct blob_attr *tb2g[ARRAY_SIZE(wifi_2G_attr_policy)];
	struct blob_attr *tb5g[ARRAY_SIZE(wifi_5G_attr_policy)];
	int ret;

	MASSERT(ctx);

	ret =  blobmsg_parse(wifi_info_attr_policy, ARRAY_SIZE(wifi_info_attr_policy),
			     tb, blob_data(msg), blob_len(msg));
	if (ret){
		DBG_MSG("%s L%d ret=%d*******\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}


	if (!tb[ATTR_WIRELESS])
	{
		DBG_MSG("%s L%d ret=%d*******\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}
		


	ret =  blobmsg_parse(wifi_info_attr_policy, ARRAY_SIZE(wifi_info_attr_policy), tb,
			     blobmsg_data(tb[ATTR_WIRELESS]),
			     blobmsg_data_len(tb[ATTR_WIRELESS]));
	if (ret){
		DBG_MSG("%s L%d ret=%d*******\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}

	if (!tb[ATTR_AP1])
	{
		DBG_MSG("%s L%d ret=%d*******\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}

	ret =  blobmsg_parse(wifi_2G_attr_policy, ARRAY_SIZE(wifi_2G_attr_policy), tb2g,
			     blobmsg_data(tb[ATTR_AP1]),
			     blobmsg_data_len(tb[ATTR_AP1]));
	if (ret) {
		DBG_MSG("%s L%d ret=%d*******\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}

	if (!tb2g[ATTR_WIFI_IF_2G]) {
		DBG_MSG("%s L%d ret=%d*******\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}

	if (tb2g[ATTR_WIFI_IF_2G]) {
		ret = blobmsg_parse(wifi_2G_attr_policy,
		            ARRAY_SIZE(wifi_2G_attr_policy), tb2g,
		            blobmsg_data(tb2g[ATTR_WIFI_IF_2G]),
		            blobmsg_data_len(tb2g[ATTR_WIFI_IF_2G]));
		ctx->status.on = tb2g[ATTR_SWITCH] ?
		    strdup(blobmsg_get_string(tb2g[ATTR_SWITCH])) : NULL;
		if ((strcmp(ctx->status.on, "on") == 0)) {
		    ctx->status.device = tb2g[ATTR_DEVICE] ?
		        strdup(blobmsg_get_string(tb2g[ATTR_DEVICE])) : NULL;
		    ctx->status.channel = tb2g[ATTR_CHANNEL] ?
		        strdup(blobmsg_get_string(tb2g[ATTR_CHANNEL])) : NULL;

		    if (tb2g[ATTR_SSID0]) {
		        ret =  blobmsg_parse(wifi_2G_attr_policy,
		                     ARRAY_SIZE(wifi_2G_attr_policy), tb2g,
		                     blobmsg_data(tb2g[ATTR_SSID0]),
		                     blobmsg_data_len(tb2g[ATTR_SSID0]));
                sprintf(ctx->status.ssid, "%s", blobmsg_get_string(tb2g[ATTR_SSID]));
				if (ctx->status.num_clients > 0 && ctx->status.usb_status == 1) {
					snprintf(ctx->status.ui_ssid, sizeof(ctx->status.ui_ssid), "%s (%d)", ctx->status.ssid, (ctx->status.num_clients - 1));
				} else if (ctx->status.num_clients <= 0) {
					snprintf(ctx->status.ui_ssid, sizeof(ctx->status.ui_ssid), "%s (%d)", ctx->status.ssid, 0);
				} else {
					snprintf(ctx->status.ui_ssid, sizeof(ctx->status.ui_ssid), "%s (%d)", ctx->status.ssid, ctx->status.num_clients);
				}
		        ctx->status.key = tb2g[ATTR_KEY] ?
		            strdup(blobmsg_get_string(tb2g[ATTR_KEY])) : NULL;
		        ctx->status.encryption = tb2g[ATTR_ENCRYPTION] ?
		            strdup(blobmsg_get_string(tb2g[ATTR_ENCRYPTION])) : NULL;
		    }
		}
	}
/*
	if ((strcmp(ctx->status.on, "on") != 0)) {
		ret = blobmsg_parse(wifi_5G_attr_policy, ARRAY_SIZE(wifi_5G_attr_policy), tb5g,
		            blobmsg_data(tb[ATTR_AP0]),
		            blobmsg_data_len(tb[ATTR_AP0]));
		if (ret) {
		    DBG_MSG("%s L%d ret=%d*******\n",__FUNCTION__,__LINE__,ret);
		    return -1;
		}

		if (!tb5g[ATTR2_WIFI_IF_5G]) {
		    DBG_MSG("%s L%d ret=%d*******\n",__FUNCTION__,__LINE__,ret);
		    return -1;
		}

		if (tb5g[ATTR2_WIFI_IF_5G]) {
		    ret =  blobmsg_parse(wifi_5G_attr_policy,
		                ARRAY_SIZE(wifi_5G_attr_policy), tb5g,
		                blobmsg_data(tb5g[ATTR2_WIFI_IF_5G]),
		                blobmsg_data_len(tb5g[ATTR2_WIFI_IF_5G]));
		    ctx->status.on = tb5g[ATTR2_SWITCH] ?
		        strdup(blobmsg_get_string(tb5g[ATTR2_SWITCH])) : NULL;
		    if ((strcmp(ctx->status.on, "on") == 0)) {
		        ctx->status.device = tb5g[ATTR2_DEVICE] ?
		            strdup(blobmsg_get_string(tb5g[ATTR2_DEVICE])) : NULL;
		        ctx->status.channel = tb5g[ATTR2_CHANNEL] ?
			        strdup(blobmsg_get_string(tb5g[ATTR2_CHANNEL])) : NULL;
		        if (tb5g[ATTR2_SSID0]) {
		            ret =  blobmsg_parse(wifi_5G_attr_policy,
		                        ARRAY_SIZE(wifi_5G_attr_policy), tb5g,
		                        blobmsg_data(tb5g[ATTR2_SSID0]),
		                        blobmsg_data_len(tb5g[ATTR2_SSID0]));
					sprintf(ctx->status.ssid, "%s", blobmsg_get_string(tb5g[ATTR2_SSID]));
					if (ctx->status.num_clients > 0 && ctx->status.usb_status == 1) {
						snprintf(ctx->status.ui_ssid, sizeof(ctx->status.ui_ssid), "%s (%d)", ctx->status.ssid, (ctx->status.num_clients - 1));
					} else if (ctx->status.num_clients <= 0) {
						snprintf(ctx->status.ui_ssid, sizeof(ctx->status.ui_ssid), "%s (%d)", ctx->status.ssid, 0);
					} else {
						snprintf(ctx->status.ui_ssid, sizeof(ctx->status.ui_ssid), "%s (%d)", ctx->status.ssid, ctx->status.num_clients);
					}
		            ctx->status.key = tb5g[ATTR2_KEY] ?
		                strdup(blobmsg_get_string(tb5g[ATTR2_KEY])) : NULL;
		            ctx->status.encryption = tb5g[ATTR2_ENCRYPTION] ?
		                strdup(blobmsg_get_string(tb5g[ATTR2_ENCRYPTION])) : NULL;
		        }
		    } else {
		        ctx->status.device = NULL;
		        ctx->status.channel = NULL;
		        ctx->status.key = NULL;
		        ctx->status.encryption = NULL;
		    }
		}
	}
	if (ret){
		DBG_MSG("parsing blobmsg failed %d\n", ret);
		return -1;
	}
*/
	if(strlen(ctx->status.ssid)>3)
		//strcpy(g_ssid,ctx->status.ssid);
	if(strlen(ctx->status.key)>3)
		//strcpy(g_key,ctx->status.key);
	//strcpy(g_client_n,ctx->status.num_clients);
	sprintf(g_client_n,"%d",ctx->status.num_clients);
    DBG_MSG(" blobmsg device = %s, switch = %s, channel = %s, ssid = %s, num_clients = %d, key = %s, encryption = %s\n", 
		ctx->status.device, 
		ctx->status.on,
		ctx->status.channel,
		ctx->status.ssid,
		ctx->status.num_clients,
		ctx->status.key,
		ctx->status.encryption);
	return 0;
}

static int active_clients_num_parse_blobmsg(struct mgui_wifi_context *ctx, struct blob_attr *msg)
{
	struct blob_attr *tb[ARRAY_SIZE(active_clients_num_attr_policy)];
	int ret;

	MASSERT(ctx);

	ret =  blobmsg_parse(active_clients_num_attr_policy, ARRAY_SIZE(active_clients_num_attr_policy),
			     tb, blob_data(msg), blob_len(msg));
	if (ret){
		DBG_MSG("%s L%d*parsing clients_num blobmsg failed %d******\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}

	if (!tb[ATTR_CLIENTS_INFO])
	{
		DBG_MSG("%s L%d*parsing clients_num blobmsg failed %d******\n",__FUNCTION__,__LINE__,ret);
			return -1;
	}
		
	ret =  blobmsg_parse(active_clients_num_attr_policy, ARRAY_SIZE(active_clients_num_attr_policy), tb,
			     blobmsg_data(tb[ATTR_CLIENTS_INFO]),
			     blobmsg_data_len(tb[ATTR_CLIENTS_INFO]));
	if (ret){
		DBG_MSG("%s L%d*parsing clients_num blobmsg failed %d******\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}

	if (!tb[ATTR_ACTIVE_CLIENTS_NUM]) {
		DBG_MSG("%s L%d*parsing clients_num blobmsg failed %d******\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}
	ctx->status.num_clients = atoi(blobmsg_get_string(tb[ATTR_ACTIVE_CLIENTS_NUM]));
	DBG_MSG("parsing clients_num blobmsg num_clients = %d\n", 
		ctx->status.num_clients);

	if (ctx->status.num_clients > 0) {
		snprintf(ctx->status.ui_ssid, sizeof(ctx->status.ui_ssid), "%s (%d)", ctx->status.ssid, (ctx->status.num_clients - 1));
	} else if (ctx->status.num_clients <= 0) {
		snprintf(ctx->status.ui_ssid, sizeof(ctx->status.ui_ssid), "%s (%d)", ctx->status.ssid, 0);
	} else {
		snprintf(ctx->status.ui_ssid, sizeof(ctx->status.ui_ssid), "%s (%d)", ctx->status.ssid, ctx->status.num_clients);
	}

	DBG_MSG("active_clients_num_parse_blobmsg  ctx->status.ui_ssid%s \n", ctx->status.ui_ssid);
	return 0;
}

static int wifi_status_parse_blobmsg(struct mgui_wifi_context *ctx, struct blob_attr *msg)
{
	struct blob_attr *tb[ARRAY_SIZE(wifi_status_attr_policy)];
	int ret;

	MASSERT(ctx);

	ret =  blobmsg_parse(wifi_status_attr_policy, ARRAY_SIZE(wifi_status_attr_policy),
			     tb, blob_data(msg), blob_len(msg));
	if (ret){
		DBG_MSG("parsing blobmsg failed %d\n", ret);
		return -1;
	}

	if (!tb[ATTR_STATISTICS])
		return -1;
#ifdef WIFI_DEBUG
	fprintf(stderr, "statistics table: ");
	dump_table(blobmsg_data(tb[ATTR_WIRELESS]),
		   blobmsg_data_len(tb[ATTR_WIRELESS]), 0, false);
#endif
	ret =  blobmsg_parse(wifi_status_attr_policy, ARRAY_SIZE(wifi_status_attr_policy), tb,
			     blobmsg_data(tb[ATTR_STATISTICS]),
			     blobmsg_data_len(tb[ATTR_STATISTICS]));
	if (ret){
		DBG_MSG("parsing blobmsg failed %d\n", ret);
		return -1;
	}

	if (!tb[ATTR_WIFI_STATUS]) {
		DBG_MSG("parsing blobmsg failed %d\n", ret);
		return -1;
	}

	ctx->status.wifi_status = 0;
	if (tb[ATTR_WIFI_STATUS]) {
		char *str = blobmsg_get_string(tb[ATTR_WIFI_STATUS]);
		if (!strcmp(str, "ready"))
			ctx->status.wifi_status = 1;
	}

	return 0;
}

static void wifi_info_data_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct mgui_wifi_context *wifi = (struct mgui_wifi_context *)req->priv;
	int ret;

	MASSERT(wifi);

	DBG_MSG("%s L%d  wifi data received*******\n",__FUNCTION__,__LINE__);
	
	ret = wifi_info_parse_blobmsg(wifi, msg);
	DBG_MSG("%s L%d  ret=%d*******\n",__FUNCTION__,__LINE__,ret);
	if (ret){
		DBG_MSG("parsing blobmsg failed %d\n", ret);
		return;
	}

	//mgui_update_icon(wifi->mgui, MGUI_WIFI_SSID_ICON, (void *)wifi);
}

static void active_clients_num_data_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct mgui_wifi_context *wifi = (struct mgui_wifi_context *)req->priv;
	int ret;

	MASSERT(wifi);

	DBG_MSG("%s L%d  wifi data received*******\n",__FUNCTION__,__LINE__);
	
	ret = active_clients_num_parse_blobmsg(wifi, msg);
	if (ret){
		DBG_MSG("parsing blobmsg failed %d\n", ret);
		return;
	}

	//mgui_update_icon(wifi->mgui, MGUI_WIFI_SSID_ICON, (void *)wifi);
}

static void wifi_complete_cb(struct ubus_request *req, int ret)
{
	struct mgui_wifi_context *wifi = (struct mgui_wifi_context *)req->priv;
	
	DBG_MSG("wifi complete received\n");
	/* trigger screen refresh since this is a result of an async request */
	mgui_screen_refresh(wifi->mgui);
}

static void active_clients_num_complete_cb(struct ubus_request *req, int ret)
{
	struct mgui_wifi_context *wifi = (struct mgui_wifi_context *)req->priv;
	
	DBG_MSG("wifi complete received\n");
	DBG_MSG("mgui_screen_refresh\n");
	mgui_screen_refresh(wifi->mgui);
}

static void wifi_status_data_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct mgui_wifi_context *wifi = (struct mgui_wifi_context *)req->priv;
	int ret;

	MASSERT(wifi);

	DBG_MSG("%s L%d  wifi data received*******\n",__FUNCTION__,__LINE__);
	
	ret = wifi_status_parse_blobmsg(wifi, msg);
	if (ret){
		DBG_MSG("parsing blobmsg failed %d\n", ret);
		return;
	}

	mgui_update_icon(wifi->mgui, MGUI_WIFI_ICON, (void *)wifi);
}

static inline int mgui_wifi_info_request(struct mgui_wifi_context *ctx)
{
	DBG_MSG("enter mgui_wifi_info_request>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	return mgui_ubus_invoke_async(ctx->mgui->ubus, &b, ctx->wifi_ubus_id,
			    WIFI_UBUS_REQUEST, wifi_info_data_cb, wifi_complete_cb, ctx);
}

static inline int mgui_active_clients_num_request(struct mgui_wifi_context *ctx)
{
	return mgui_ubus_invoke_async(ctx->mgui->ubus, &b, ctx->statistics_ubus_id,
			    ACTIVE_CLIENTS_NUM_REQUEST, active_clients_num_data_cb, active_clients_num_complete_cb, ctx);
}

static inline int mgui_wifi_status_request(struct mgui_wifi_context *ctx)
{
	return mgui_ubus_invoke_async(ctx->mgui->ubus, &b, ctx->statistics_ubus_id,
			    STATISTICS_UBUS_REQUEST, wifi_status_data_cb, wifi_complete_cb, ctx);
}

enum {
	WIFI_CLIENTS_NOTIFY_ID,
	WIFI_STATUS_NOTIFY_ID,
};

enum {
	REQID,
	DATA,
	_MAX
};

static int wifi_indication_parse_blobmsg(struct blob_attr *attr, struct blob_attr **tb)
{
	struct blob_attr *pos = NULL;
	struct blobmsg_hdr *hdr = NULL;
	char *policy[] = {"id"};
	int rem = 0;
	int i = 0;

	blob_for_each_attr(pos, attr, rem) {
		if (i > _MAX)
			return 0;

		hdr = blob_data(pos);
		if (i == 0) {
			if (strcmp(policy[i], (char *)hdr->name) != 0) {
				DBG_MSG("format is error\n");
				return -1;
			}
		}

		tb[i] = pos;
		i++;
	}

	return 0;
}

static int wifi_indication_clients(struct mgui_wifi_context *wifi, struct blob_attr *attr)
{
	char *str = blobmsg_get_string(attr);
	int num_clients;

	MASSERT(wifi);

	DBG_MSG("clients number changed to %s\n", str);

	num_clients = atoi(str);

	if (num_clients != wifi->status.num_clients) {
		DBG_MSG("wifi status changed to %d\n", num_clients);
		wifi->status.num_clients = num_clients;
		mgui_active_clients_num_request(wifi);
	}
	
	return 0;
}

static int wifi_indication_status(struct mgui_wifi_context *wifi, struct blob_attr *attr)
{
	char *str = blobmsg_get_string(attr);
	int status;

	MASSERT(wifi);

	DBG_MSG("status changed to %s\n", str);

	status = !strcmp(str, "ready");
	DBG_MSG("status=%d\n", status);
	if (status != wifi->status.wifi_status) {
		DBG_MSG("wifi status changed to %s\n", str);
		wifi->status.wifi_status = status;
		mgui_wifi_info_request(wifi);
		mgui_update_icon(wifi->mgui, MGUI_WIFI_ICON, (void *)wifi);
	}
	
	return 0;
}

static int wifi_indication_cb(struct ubus_context *ctx,
			       struct ubus_object *obj,
			       struct ubus_request_data *req,
			       const char *method, struct blob_attr *msg)
{
	struct mgui_wifi_context *wifi = to_wifi_context(obj);
	struct blob_attr *tb[_MAX] = { 0 };
	unsigned int id;
	struct blob_attr *attr;
	int ret = 0;

	ret = wifi_indication_parse_blobmsg(msg, tb);
	if (ret < 0) {
		DBG_MSG("parse msg error\n");
		return -1;
	}


	if (!tb[REQID] || !tb[DATA]) {
		DBG_MSG("Error: id or data not found\n");
		return -1;
	}

	id = blobmsg_get_u32(tb[REQID]);
	attr = tb[DATA];

	DBG_MSG("id is %d, attr = %p\n", id, attr);

	if (strcmp(method, "clients_change") == 0) {
		ret = wifi_indication_clients(wifi, attr);
	} else if (strcmp(method, "wifi_status_change") == 0) {
		ret = wifi_indication_status(wifi, attr);
	} else {
		DBG_MSG("id of %d is not supported\n",id);
	}

	/*
	switch (id) {
	case WIFI_CLIENTS_NOTIFY_ID:
		ret = wifi_indication_clients(wifi, attr);
		break;
	case WIFI_STATUS_NOTIFY_ID:
		ret = wifi_indication_status(wifi, attr);
		break;
	default:
		DBG_MSG("id of %d is not supported\n",id);
	}
	*/

	
	/* refresh screen if required */
	if (ret){
		DBG_MSG("mgui_screen_refresh\n");
		mgui_screen_refresh(wifi->mgui);
	}

	return 0;
}

/**
 * mgui_wifi_exit
 * De-initialize wifi interface
 * 
 * @param wifi pointer to previously allocated wifi context
 * 
 * @return 0 for success, error code otherwise
 */
int mgui_wifi_exit(struct mgui_wifi_context *wifi)
{
	if(!wifi) {
		DBG_MSG("wifi module not running\n");
		return 0;
	}
	/*
	mgui_ubus_unsubscribe(wifi->mgui->ubus,
			      &wifi->ubus_subscriber,
			      wifi->wifi_ubus_id);
	*/
	mgui_ubus_unsubscribe(wifi->mgui->ubus,
			      &wifi->ubus_subscriber,
			      wifi->statistics_ubus_id);
	mgui_ubus_unregister_subscriber(wifi->mgui->ubus,
				    &wifi->ubus_subscriber);

	if (wifi->status.channel)
		free(wifi->status.channel);
	if (wifi->status.device)
		free(wifi->status.device);
	if (wifi->status.on)
		free(wifi->status.on);
	if (wifi->status.key)
		free(wifi->status.key);
	if (wifi->status.encryption)
		free(wifi->status.encryption);
	free(wifi);

	DBG_MSG("wifi exit done\n");

	return 0;
}

/**
 * mgui_wifi_init
 * Initialize mgui wifi interface
 * 
 * NOTE - must be called BEFORE uloop_run()!
 * 
 * @param mgui   mgui context
 * 
 * @return pointer to mgui_wifi_context
 */
struct mgui_wifi_context *mgui_wifi_init(struct mgui_context *mgui)
{
	struct mgui_wifi_context *wifi;
	int ret;

	MASSERT(mgui);
	MASSERT(mgui->ubus);

	wifi = malloc(sizeof(struct mgui_wifi_context));
	if (!wifi) {
		DBG_MSG("memory allocation failed\n");
		return NULL;
	}

	memset(wifi, 0, sizeof(*wifi));

	wifi->mgui = mgui;

	ret = mgui_ubus_lookup_id(wifi->mgui->ubus, WIFI_UBUS_ID,
				  &wifi->wifi_ubus_id);
	if (ret) {
		DBG_MSG("mubus_lookup_id failed (ret=%d)\n", ret);
		goto out_error;
	}

	ret = mgui_ubus_lookup_id(wifi->mgui->ubus, STATISTICS_UBUS_ID,
				  &wifi->statistics_ubus_id);
	if (ret) {
		DBG_MSG("mubus_lookup_id failed (ret=%d)\n", ret);
		goto out_error;
	}

	ret = mgui_wifi_status_request(wifi);
	if (ret) {
		DBG_MSG("initial request failed\n");
		goto out_error;
	}

	ret = mgui_active_clients_num_request(wifi);
	if (ret) {
		DBG_MSG("initial request failed\n");
		goto out_error;
	}


	ret = mgui_wifi_info_request(wifi);
	if (ret) {
		DBG_MSG("initial request failed\n");
		goto out_error;
	}

	wifi->ubus_subscriber.cb = wifi_indication_cb;
	ret = mgui_ubus_register_subscriber(wifi->mgui->ubus,
					    &wifi->ubus_subscriber);
	if (ret) {
		DBG_MSG("mubus_register_subscriber failed\n");
		goto out_error;
	}

/*
	ret = mgui_ubus_subscribe(wifi->mgui->ubus,
				  &wifi->ubus_subscriber,
				  wifi->wifi_ubus_id);
	if (ret) {
		DBG_MSG("ubus subscribe wireless failed\n");
		goto unregister_subscriber;
	}
*/

	ret = mgui_ubus_subscribe(wifi->mgui->ubus,
				  &wifi->ubus_subscriber,
				  wifi->statistics_ubus_id);
	if (ret) {
		DBG_MSG("ubus subscribe statistics failed\n");
		goto unregister_subscriber;
	}

	DBG_MSG("wifi init done\n");

	return wifi;

/*
unsubscribe_wireless:
	mgui_ubus_unsubscribe(wifi->mgui->ubus,
			      &wifi->ubus_subscriber,
			      wifi->wifi_ubus_id);
*/
unregister_subscriber:
	mgui_ubus_unregister_subscriber(wifi->mgui->ubus,
					&wifi->ubus_subscriber);
out_error:
	free(wifi);
	return NULL;
}
