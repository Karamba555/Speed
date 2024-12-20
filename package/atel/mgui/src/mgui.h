/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui.h
 *
 *  Authors:  Tomer Eliyahu
 *
 *  Description: 
 *
 *  HISTORY:
 *   Nov 23, 2014 - Initial Version
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef MGUI_H
#define MGUI_H

#include <signal.h>
#include <sys/time.h>
#include "Object.h"
#include "String.h"
#include "Image.h"
#include "TextBox.h"
#include "Button.h"
#include "mgui_icons.h"
#include "mgui_ubus.h"
#include "user_conf.h"
#if defined	CONFIG_USE_MGUI_READ_SMS
#include "mgui_sms.h"
#endif
/******************************************************************************
 *   Definitions
 ******************************************************************************/
#define NOTION_L20_1D44_LCD
//#define MARVELL_ORIGIN_LCD
#define BACKGROUND_COLOR	0x3661AB	///wpeng comment BLUE RGB(36,61,171)

/* top bar definitions */
#define TOP_BAR_ICON_WIDTH	25
#define TOP_BAR_ICON_HEIGHT	25
#define TOP_BAR_LINE_HEIGHT	TOP_BAR_ICON_HEIGHT + 2
#define BOTTOM_BAR_LINE_HEIGHT	30

#define TIME_FORMAT			"%H:%M"
#define TIME_FORMAT_SIZE		sizeof("HH:MM\0")
#define CLOCK_REFRESH_PER_USEC		30*1000000

#define CLOCK_TIMER_FREQ_NANOSECS	(1000000000LL * 3) /* 10 seconds */
#define IDLE_TIMER_FREQ_NANOSECS	(1000000000LL * 20) /* 20 seconds */
#define MODULES_TIMER_FREQ_NANOSECS	(1000000000LL * 5) /* 5 seconds */
#define SMS_TIMER_FREQ_NANOSECS	(1000000000LL * 10)
//#define ENABLE_UBUS_CHARGER 1
//#define ENABLE_UBUS_WIFI 1
//#define ENABLE_UBUS_VERSION 1
//#define ENABLE_USBU_OPERATOR 1
//#define ENABLE_MODULES_TIMER 1
#define ENABLE_IDLE_TIMER 1
//#define ENABLE_USBU_VOICE 1
//#define ENABLE_UBUS_RADIO_TECH 1
//#define ENABLE_UBUS_SIM_STATUS 1
/******************************************************************************
 *   Structures
 ******************************************************************************/

struct mgui_ril_context;

enum e_mgui_state {
	MGUI_STATE_OFF,		/* on key press toggle - enter mgui sleep */
	MGUI_STATE_FIRST,		/* on key press toggle - exit mgui sleep */
	MGUI_STATE_SECOND,
	MGUI_STATE_THIRD,
	MGUI_STATE_ALERT_ONKEY_EVENT,
	MGUI_STATE_FORTH,
	MGUI_STATE_SMS_NUMBER,
	MGUI_STATE_SMS_FIRST,
	MGUI_STATE_SMS_SECOND,
	MGUI_STATE_SMS_THIRD,
	MGUI_STATE_SMS_END,
	MGUI_STATE_WPS_PROCESSING,
	MGUI_STATE_WPS_FAIL,
};

enum e_mgui_event {
	MGUI_EXIT_EVENT,
	MGUI_ONKEY_EVENT,
	MGUI_ALERT_ONKEY_EVENT,
	MGUI_IDLE_TIMEOUT_EVENT,
	MGUI_MODULES_TIMEOUT_EVENT,
	MGUI_BUTTON_EVENT,
	MGUI_WPS_EVENT,
	#if defined	CONFIG_USE_MGUI_READ_SMS
	MGUI_SMSKEY_EVENT,
	#endif
	MGUI_CHARGER_EVENT,
	/* Add more events here*/

	MGUI_EVENT_END,
};

struct mgui_event {
	enum e_mgui_event id;
	unsigned long data; /* for future use */
};

struct mgui_context {
	pthread_mutex_t lock;

	struct ubus_context *ubus;
	struct mgui_ril_context *ril;
	struct mgui_charger_context *charger;
	struct mgui_onkey_context *onkey;
	struct mgui_wifi_context *wifi;
	struct mgui_hawk_context *hawk;
	struct mgui_version_context *ver;
	#if defined	CONFIG_USE_MGUI_READ_SMS
	struct mgui_sms_context *sms;
	struct mgui_smskey_context *smskey;
	#endif
	struct mgui_wpskey_context *wpskey;
	struct mgui_alert_onkey_context *alert;
	/* mgui state machine */
	enum e_mgui_state state;

	/* pipe for IPC */
	int pipes_fd[2];
	struct uloop_fd ubus_fd;

	/* timers */
	timer_t clock_timer;
	timer_t idle_timer;
	timer_t modules_timer;
	timer_t work_timer;
	timer_t update_info_timer;
	timer_t next_screen_timer;
	
	/* dfb interface*/
	void *dfb;
#ifdef MARVELL_ORIGIN_LCD
	long dfb_touch;
#endif
	OBJECT_SCREEN screen;

	char version[120];
};

struct mgui_context *mgui_init_main(int argc, char *argv[]);
int mgui_exit(struct mgui_context *ctx);
int mgui_run(struct mgui_context *ctx);
int wps_test(struct mgui_context *ctx);
void mgui_reset_all_icons(struct mgui_context *ctx);
int mgui_update_icon(struct mgui_context *ctx, enum e_mgui_icons i, void *data);
void mgui_screen_refresh(struct mgui_context *ctx);
void mgui_screen_onkeyevent_handle(struct mgui_context *ctx);
void mgui_screen_wpskeyevent_handle(struct mgui_context *ctx);
void mgui_screen_smskeyevent_handle(struct mgui_context *ctx);
void mgui_screen_chargerevent_handle(struct mgui_context *ctx);
void mgui_cp_assert_wakeup(struct mgui_context *ctx);
struct mgui_context *mgui_init_main_speedway(int argc, char *argv[]);
int mgui_run_speedway(struct mgui_context *ctx);

#endif
