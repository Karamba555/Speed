
/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_ubus.c
 *
 *  Authors:  Tomer Eliyahu
 *
 *  Description: MGUI - uBus interface implementation
 *
 *  HISTORY:
 *
 *   Nov 23, 2014 - Initial Version
 *
 *  Notes:
 *
 ******************************************************************************/

/******************************************************************************
 *   Include files
 ******************************************************************************/
#include <stdio.h>

#include "mgui_utils.h"
#include "mgui_ubus.h"

/******************************************************************************
 *  Code
 ******************************************************************************/

int mgui_ubus_unsubscribe_event(struct ubus_context *ctx,
				 struct ubus_subscriber *s,
				 const char *e)
{
	uint32_t id;
	int ret;

	MASSERT(ctx);
	MASSERT(e);
	MASSERT(s);

	if (ubus_lookup_id(ctx, e, &id)) {
		MGUI_EMSG("event %s lookup failed\n", e);
		return -1;
	}

	ret = ubus_unsubscribe(ctx, s, id);
	if (ret) {
		MGUI_EMSG("event %s unsubscribe failed (err: %s)\n",
			  e, ubus_strerror(ret));
		return ret;
	}

	MGUI_IMSG("unsubscribe event %s success\n", e);

	return 0;
}

int mgui_ubus_subscribe_event(struct ubus_context *ctx,
			   struct ubus_subscriber *s,
			   const char *e)
{
	uint32_t id;
	int ret;

	MASSERT(ctx);
	MASSERT(e);
	MASSERT(s);

	if (ubus_lookup_id(ctx, e, &id)) {
		MGUI_EMSG("event %s lookup failed\n", e);
		return -1;
	}

	ret = ubus_subscribe(ctx, s, id);
	if (ret) {
		MGUI_EMSG("event %s subscribe failed (err: %s)\n",
			  e, ubus_strerror(ret));
		return ret;
	}

	MGUI_IMSG("subscribe event %s success\n", e);

	return 0;
}

int mgui_ubus_invoke(struct ubus_context *ctx, struct blob_buf *b,
		  uint32_t id, const char *method, ubus_data_handler_t cb,
		  void *priv, unsigned int timeout)
{
	int ret = 0;

	ret = ubus_invoke(ctx, id, method, b? b->head : NULL , cb, priv, timeout);
	if (ret) {
		MGUI_EMSG("ubus_invoke failed [id=%u, method=%s, err=%s]\n",
			  id, method, ubus_strerror(ret));
		return ret;
	}
	
	MGUI_DMSG("ubus_invoke done [id=%u, method=%s]\n",
		  id, method, ubus_strerror(ret));

	return 0;
}

int mgui_ubus_invoke_async(struct ubus_context *ctx, struct blob_buf *b,
			uint32_t id, const char *method,
			ubus_data_handler_t data_cb,
			ubus_complete_handler_t complete_cb, void *priv)
{
	struct ubus_request *req;
	int ret = 0;

	req = (struct ubus_request *)malloc(sizeof(*req));
	if (!req) {
		MGUI_EMSG("memory allocation failed\n");
		return -1;
	}

	ret = ubus_invoke_async(ctx, id, method, b ? b->head : NULL, req);
	if (ret) {
		MGUI_EMSG("ubus_invoke failed [id=%u, method=%s, err=%s]\n",
			  id, method, ubus_strerror(ret));
		free(req);
		return ret;
	}
	MGUI_DMSG("ubus_invoke_async done [id=%u, method=%s]\n", id, method);

	req->data_cb = data_cb;
	req->complete_cb = complete_cb;
	req->priv = priv;
	ubus_complete_request_async(ctx, req);

	MGUI_DMSG("ubus_complete_request_async done\n");

	return 0;
}

int mgui_ubus_register_subscriber(struct ubus_context *ctx, struct ubus_subscriber *s)
{
	return ubus_register_subscriber(ctx, s);
}

int mgui_ubus_unregister_subscriber(struct ubus_context *ctx, struct ubus_subscriber *s)
{
	return ubus_unregister_subscriber(ctx, s);
}

int mgui_ubus_exit(struct ubus_context *ctx)
{
	MASSERT(ctx);

	ubus_free(ctx);
	uloop_done();

	MGUI_IMSG("ubus exit done");

	return 0;
}

struct ubus_context *mgui_ubus_init(void)
{
	struct ubus_context *ctx;

	uloop_init();

	ctx = ubus_connect(NULL);
	if (!ctx) {
		MGUI_EMSG("Failed to connect to ubus\n");
		return NULL;
	}

	ubus_add_uloop(ctx);

	MGUI_DMSG("done\n");

	return ctx;
}
