/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_hawk.c
 *
 *  Authors:  Tomer Eliyahu
 *
 *  Description: MGUI hawk interface
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
#include <unistd.h>
#include <inttypes.h>
#include "mgui_utils.h"
#include "mgui_ubus.h"
#include "mgui_icons.h"
#include "mgui_hawk.h"
#include "mgui.h"
#ifdef CONFIG_TARGET_mmp
#include <cutils/properties.h>
#endif
/******************************************************************************
 *   Structures
 ******************************************************************************/

/******************************************************************************
 *   Definitions
 ******************************************************************************/

#define HAWK_UBUS_ID			"hawk"

static const char *hawk_requests[] = {"keep_alive", "reset", "fota", "ping",};
static inline const char *hawk_ubus_request(enum hawk_req req)
{
	return hawk_requests[req];
}

#define to_ubus_subscriber(obj) container_of(obj, struct ubus_subscriber, obj)
#define to_hawk_context(obj) container_of(to_ubus_subscriber(obj), struct mgui_hawk_context, ubus_subscriber)

/******************************************************************************
 *   Globals
 ******************************************************************************/

static	struct blob_buf b;

/******************************************************************************
 *  Code
 ******************************************************************************/

#if 0
static int hawk_parse_blobmsg(struct mgui_hawk_context *hawk, struct blob_attr *msg)
{
	UNUSED(hawk);
	UNUSED(msg);

	return 0;
}
#endif

static void hawk_get_version(struct mgui_context *ctx)
{
	int len, retries = 20;
#ifdef CONFIG_TARGET_mmp
	len = property_get("persist.hawk.build_version", ctx->version, "Version Unknown");

	while (!len && --retries) {
		MGUI_IMSG("retry get hawk version");
		len = property_get("persist.hawk.build_version", ctx->version, "Version Unknown");
		sleep(1);
	}
#endif
	if (len) {
		MGUI_IMSG("hawk version: %s", ctx->version);
		//mgui_update_icon(ctx, MGUI_VERSION_ICON, (void *)ctx->version);
		//mgui_screen_refresh(ctx);
	} else {
		MGUI_EMSG("Failed to get hawk version");
	}
}

static void hawk_data_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	UNUSED(req);
	UNUSED(type);
	UNUSED(msg);
	MGUI_DMSG("hawk data");

}

static void hawk_complete_cb(struct ubus_request *req, int ret)
{
	struct mgui_hawk_context *hawk = (struct mgui_hawk_context *)req->priv;
	UNUSED(hawk);
	MGUI_DMSG("hawk complete received\n");
}

int mgui_hawk_request(struct mgui_hawk_context *ctx, enum hawk_req req)
{
	return mgui_ubus_invoke_async(ctx->mgui->ubus, &b, ctx->hawk_ubus_id,
				      hawk_ubus_request(req),
				      hawk_data_cb, hawk_complete_cb, ctx);
}

/**
 * mgui_hawk_exit De-initialize hawk interface 
 * 
 * @param hawk pointer to previously allocated hawk context
 * 
 * @return 0 for success, error code otherwise
 */
int mgui_hawk_exit(struct mgui_hawk_context *hawk)
{
	if(!hawk) {
		MGUI_IMSG("hawk module not running\n");
		return 0;
	}

	mgui_ubus_unsubscribe(hawk->mgui->ubus,
			      &hawk->ubus_subscriber,
			      hawk->hawk_ubus_id);
	mgui_ubus_unregister_subscriber(hawk->mgui->ubus,
				    &hawk->ubus_subscriber);

	free(hawk);

	MGUI_IMSG("hawk exit done\n");

	return 0;
}

/**
 * mgui_hawk_init
 * Initialize mgui hawk interface
 * 
 * NOTE - must be called BEFORE uloop_run()!
 * 
 * @param mgui   mgui context
 * 
 * @return pointer to mgui_hawk_context
 */
struct mgui_hawk_context *mgui_hawk_init(struct mgui_context *mgui)
{
	struct mgui_hawk_context *hawk;
	int ret;

	MASSERT(mgui);
	MASSERT(mgui->ubus);

	hawk = malloc(sizeof(struct mgui_hawk_context));
	if (!hawk) {
		MGUI_EMSG("memory allocation failed\n");
		return NULL;
	}

	memset(hawk, 0, sizeof(*hawk));

	hawk->mgui = mgui;

	ret = mgui_ubus_lookup_id(hawk->mgui->ubus, HAWK_UBUS_ID,
				  &hawk->hawk_ubus_id);
	if (ret) {
		MGUI_EMSG("mubus_lookup_id failed (ret=%d)\n", ret);
		goto out_error;
	}

	hawk_get_version(mgui);

	MGUI_IMSG("hawk init done\n");

	return hawk;

out_error:
	free(hawk);
	return NULL;
}
