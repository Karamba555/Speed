/******************************************************************************
*(C) Copyright 2016 Notion International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_version.h
 *
 *  Authors:  yueguangkai
 *
 *  Description: mgui interface to version
 *
 *  HISTORY:
 *   Feb 23, 2016 - Initial version
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef MGUI_VERSION_H
#define MGUI_VERSION_H

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

struct mgui_version_info {
	char *sw_version;
	char *hdware_ver;
	char *build_time;
	char *CP_LTG_VER;
	char *CP_LWG_VER;
	char *mac_addr;
};

struct mgui_version_context {
	struct mgui_context *mgui;
	uint32_t version_ubus_id;
	struct mgui_version_info info; /* version info database */
};

/******************************************************************************
 *  Function prototypes
 ******************************************************************************/

struct mgui_version_context *mgui_version_init(struct mgui_context *mgui);
int mgui_version_exit(struct mgui_version_context *ctx);

#endif /* MGUI_VERSION_H */

