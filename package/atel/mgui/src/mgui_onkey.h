/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_onkey.h
 *
 *  Authors:  Tomer Eliyahu
 *
 *  Description: mgui interface to onkey
 *
 *  HISTORY:
 *   Jun 14, 2015 - Initial Version
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef MGUI_ONKEY_H
#define MGUI_ONKEY_H

#include <stdio.h>
#include <stdlib.h>

struct mgui_onkey_context {
	struct mgui_context *mgui;
	pthread_t tid;
};
struct mgui_wpskey_context {
	struct mgui_context *mgui;
	pthread_t tid;
};
struct mgui_chargerinfo_context {
	struct mgui_context *mgui;
	pthread_t tid;
};
struct mgui_alert_onkey_context  {
       struct mgui_context *mgui;
       pthread_t tid;
};

/******************************************************************************
 *  Function prototypes
 ******************************************************************************/
struct mgui_onkey_context *mgui_onkey_init(struct mgui_context *mgui);
struct mgui_wpskey_context *mgui_wpskey_init(struct mgui_context *mgui);
struct mgui_chargerinfo_context *mgui_chargerinfo_init(struct mgui_context *mgui);
int mgui_onkey_exit(struct mgui_onkey_context *ctx);
int mgui_wpskey_exit(struct mgui_wpskey_context *ctx);
int mgui_chargerinfo_exit(struct mgui_chargerinfo_context *ctx);

#endif //MGUI_RIL_H

