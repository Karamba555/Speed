/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_ubus.h
 *
 *  Authors:  Tomer Eliyahu
 *
 *  Description: mgui interface to ubus
 *
 *  HISTORY:
 *   Nov 23, 2014 - Initial Version
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef MGUI_UBUS_H
#define MGUI_UBUS_H

#include <libubox/ustream.h>
#include <libubus.h>

struct ubus_context *mgui_ubus_init(void);
int mgui_ubus_exit(struct ubus_context *);
int mgui_ubus_register_subscriber(struct ubus_context *ctx, struct ubus_subscriber *s);
int mgui_ubus_unregister_subscriber(struct ubus_context *ctx, struct ubus_subscriber *s);
int mgui_ubus_subscribe_event(struct ubus_context *ctx,
			   struct ubus_subscriber *s,
			   const char *e);
int mgui_ubus_unsubscribe_event(struct ubus_context *ctx,
			   struct ubus_subscriber *s,
			   const char *e);
int mgui_ubus_invoke(struct ubus_context *u, struct blob_buf *b,
		  uint32_t id, const char *method, ubus_data_handler_t cb,
		  void *priv, unsigned int timeout);
int mgui_ubus_invoke_async(struct ubus_context *u, struct blob_buf *b,
			uint32_t id, const char *method,
			ubus_data_handler_t data_cb,
			ubus_complete_handler_t complete_cb, void *priv);


static inline int mgui_ubus_lookup_id(struct ubus_context *ctx,
				       const char *name, uint32_t *id)
{
	return ubus_lookup_id(ctx, name, id);
}

static inline int mgui_ubus_subscribe(struct ubus_context *ctx,
				       struct ubus_subscriber *s, uint32_t id)
{
	return ubus_subscribe(ctx, s, id);
}

static inline int mgui_ubus_unsubscribe(struct ubus_context *ctx,
					 struct ubus_subscriber *s, uint32_t id)
{
	return ubus_unsubscribe(ctx, s, id);
}


static inline void mgui_ubus_uloop_run(void)
{
	uloop_run();
}

static inline int mgui_ubus_fd_add(struct uloop_fd *fd)
{
	return uloop_fd_add(fd, ULOOP_READ);
}

#endif	//MBIM_UBUS_H

