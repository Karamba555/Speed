/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_hawk.h
 *
 *  Authors:  Tomer Eliyahu
 *
 *  Description: mgui interface to hawk
 *
 *  HISTORY:
 *   Jun 14, 2015 - Initial Version
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef MGUI_HAWK_H
#define MGUI_HAWK_H

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

struct mgui_hawk_context {
	struct mgui_context *mgui;
	uint32_t hawk_ubus_id;
	struct ubus_subscriber ubus_subscriber;
};

enum hawk_req {
	HAWK_KEEP_ALIVE,
	HAWK_RESET,
	HAWK_FOTA,
	HAWK_PING,
};

/******************************************************************************
 *  Function prototypes
 ******************************************************************************/

struct mgui_hawk_context *mgui_hawk_init(struct mgui_context *mgui);
int mgui_hawk_exit(struct mgui_hawk_context *ctx);
int mgui_hawk_request(struct mgui_hawk_context *ctx, enum hawk_req req);

#endif /* MGUI_HAWK_H */

