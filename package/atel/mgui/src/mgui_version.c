/******************************************************************************
*(C) Copyright 2016 notion International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_version.c
 *
 *  Authors:  yueguangkai
 *
 *  Description: MGUI version interface
 *
 *  HISTORY:
 *
 *   Feb 23, 2016 - Initial Version
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
#include "mgui_version.h"
#include "mgui.h"

/******************************************************************************
 *   Structures
 ******************************************************************************/

/******************************************************************************
 *   Definitions
 ******************************************************************************/
//#define VERSION_DEBUG

#define VERSION_UBUS_ID			"version"
#define VERSION_UBUS_REQUEST		"get_version"

/******************************************************************************
 *   Globals
 ******************************************************************************/

/* version get_version */
enum {
	ATTR_VERSION_INFO = 0,
	ATTR_SW_VERSION,
	ATTR_HDWARE_VER,
	ATTR_BUILD_TIME,
	ATTR_CP_LTG_VER,
	ATTR_CP_LWG_VER,
	ATTR_MAC_ADDR,
};

static const struct blobmsg_policy version_info_attr_policy[] = {
	[ATTR_VERSION_INFO] = {.name = "version_info", .type = BLOBMSG_TYPE_TABLE},
	[ATTR_SW_VERSION] = {.name = "sw_version", .type = BLOBMSG_TYPE_STRING},
	[ATTR_HDWARE_VER] = {.name = "hdware_ver", .type = BLOBMSG_TYPE_STRING},
	[ATTR_BUILD_TIME] = {.name = "build_time", .type = BLOBMSG_TYPE_STRING},
	[ATTR_CP_LTG_VER] = {.name = "CP_LTG_VER", .type = BLOBMSG_TYPE_STRING},
	[ATTR_CP_LWG_VER] = {.name = "CP_LWG_VER", .type = BLOBMSG_TYPE_STRING},
	[ATTR_MAC_ADDR] = {.name = "mac_addr", .type = BLOBMSG_TYPE_STRING},
};

static	struct blob_buf b;
extern char g_swVersion[];
/******************************************************************************
 *  Code
 ******************************************************************************/

#ifdef VERSION_DEBUG
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
#endif /* VERSION_DEBUG */

static int version_info_parse_blobmsg(struct mgui_version_context *ctx, struct blob_attr *msg)
{
	struct blob_attr *tb[ARRAY_SIZE(version_info_attr_policy)];
	int ret;

	MASSERT(ctx);

	ret =  blobmsg_parse(version_info_attr_policy, ARRAY_SIZE(version_info_attr_policy),
			     tb, blob_data(msg), blob_len(msg));
	if (ret){
		MGUI_EMSG("parsing version blobmsg failed %d\n", ret);
		return -1;
	}

	if (!tb[ATTR_VERSION_INFO])
		return -1;
	ret =  blobmsg_parse(version_info_attr_policy, ARRAY_SIZE(version_info_attr_policy), tb,
			     blobmsg_data(tb[ATTR_VERSION_INFO]),
			     blobmsg_data_len(tb[ATTR_VERSION_INFO]));
	if (ret){
		MGUI_EMSG("parsing version blobmsg failed %d\n", ret);
		return -1;
	}

	ctx->info.sw_version= blobmsg_get_string(tb[ATTR_SW_VERSION]);
	ctx->info.hdware_ver= blobmsg_get_string(tb[ATTR_HDWARE_VER]);
	ctx->info.build_time= blobmsg_get_string(tb[ATTR_BUILD_TIME]);
	ctx->info.CP_LTG_VER= blobmsg_get_string(tb[ATTR_CP_LTG_VER]);
	ctx->info.CP_LWG_VER= blobmsg_get_string(tb[ATTR_CP_LWG_VER]);
	ctx->info.mac_addr= blobmsg_get_string(tb[ATTR_MAC_ADDR]);

	if(strlen(ctx->info.sw_version)>3)
		//strcpy(g_swVersion,ctx->info.sw_version);
	
	MGUI_IMSG("parsing version blobmsg sw_version = %s, hdware_ver = %s, build_time = %s, CP_LTG_VER = %s,CP_LWG_VER = %s,mac_addr = %s\n", 
		ctx->info.sw_version,
		ctx->info.hdware_ver,
		ctx->info.build_time,
		ctx->info.CP_LTG_VER,
		ctx->info.CP_LWG_VER,
		ctx->info.mac_addr);

	return 0;
}

static void version_info_data_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct mgui_version_context *version = (struct mgui_version_context *)req->priv;
	int ret;

	MASSERT(version);

	MGUI_DMSG("version data received\n");
	
	ret = version_info_parse_blobmsg(version, msg);
	if (ret){
		MGUI_EMSG("parsing blobmsg failed %d\n", ret);
		return;
	}

	//mgui_update_icon(version->mgui, MGUI_VERSION_ICON, version);
}

static void version_complete_cb(struct ubus_request *req, int ret)
{
	struct mgui_version_context *version = (struct mgui_version_context *)req->priv;
	
	MGUI_DMSG("version complete received\n");
	DBG_MSG("mgui_screen_refresh\n");
	/* trigger screen refresh since this is a result of an async request */
	mgui_screen_refresh(version->mgui);
}


static inline int mgui_version_info_request(struct mgui_version_context *ctx)
{
	return mgui_ubus_invoke_async(ctx->mgui->ubus, &b, ctx->version_ubus_id,
			    VERSION_UBUS_REQUEST, version_info_data_cb, version_complete_cb, ctx);
}

/**
 * mgui_version_exit
 * De-initialize version interface
 * 
 * @param version pointer to previously allocated version context
 * 
 * @return 0 for success, error code otherwise
 */
int mgui_version_exit(struct mgui_version_context *version)
{
	if(!version) {
		MGUI_IMSG("version module not running\n");
		return 0;
	}

	if (version->info.build_time)
		free(version->info.build_time);
	if (version->info.CP_LTG_VER)
		free(version->info.CP_LTG_VER);
	if (version->info.CP_LWG_VER)
		free(version->info.CP_LWG_VER);
	if (version->info.hdware_ver)
		free(version->info.hdware_ver);
	if (version->info.mac_addr)
		free(version->info.mac_addr);
	if (version->info.sw_version)
		free(version->info.sw_version);
	free(version);

	MGUI_IMSG("version exit done\n");

	return 0;
}

/**
 * mgui_version_init
 * Initialize mgui version interface
 * 
 * NOTE - must be called BEFORE uloop_run()!
 * 
 * @param mgui   mgui context
 * 
 * @return pointer to mgui_version_context
 */
struct mgui_version_context *mgui_version_init(struct mgui_context *mgui)
{
	struct mgui_version_context *ver;
	int ret;

	MASSERT(mgui);
	MASSERT(mgui->ubus);

	ver = malloc(sizeof(struct mgui_version_context));
	if (!ver) {
		MGUI_EMSG("memory allocation failed\n");
		return NULL;
	}

	memset(ver, 0, sizeof(*ver));

	ver->mgui = mgui;

	ret = mgui_ubus_lookup_id(ver->mgui->ubus, VERSION_UBUS_ID,
				  &ver->version_ubus_id);
	if (ret) {
		MGUI_EMSG("mubus_lookup_id failed (ret=%d)\n", ret);
		goto out_error;
	}

	ret = mgui_version_info_request(ver);
	if (ret) {
		MGUI_EMSG("initial request failed\n");
		goto out_error;
	}

	MGUI_IMSG("version init done\n");

	return ver;

out_error:
	free(ver);
	return NULL;
}
