
/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui.h
 *
 *  Authors:  Boaz Sommer Tomer Eliyahu
 *
 *  Description: 
 *
 *  HISTORY:
 *   Nov 23, 2014 - Initial Version
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef MGUI_ICONS_H
#define MGUI_ICONS_H

#include "Image.h"
#include "Button.h"
#include "Object.h"

enum e_mgui_icons {
	MGUI_ICONS_UNKNOWN,
	MGUI_BATTERY_ICON,
	MGUI_CELLULAR_ICON,
	MGUI_WIFI_ICON,
	MGUI_NETWORK_TECH_ICON,
	//MGUI_BLUETOOTH_ICON,
	//MGUI_GPS_ICON,
	//MGUI_WIFI_SSID_ICON,
	//MGUI_WIFI_KEY_ICON,
	MGUI_SMS_ICON,
	//MGUI_WIFI_NUM_CLIENTS_ICON,
	//MGUI_SDCARD_ICON,
	//MGUI_SIM_ICON,
	
	//MGUI_OPERATOR_ICON,
	//MGUI_CLOCK_ICON,
	//MGUI_VERSION_ICON,
	MGUI_ICON_END,
};

enum e_mgui_buttons {
	MGUI_FOTA_BUTTON,
	MGUI_KEEPALIVE_BUTTON,
	MGUI_NODATA_ASSERT_BUTTON,
	MGUI_RESET_BUTTON,
};

#define MAX_TEXT_SIZE	30

enum e_icon_type {
	MGUI_ICON_TYPE_IMAGE,
	MGUI_ICON_TYPE_TEXT,
	MGUI_ICON_TYPE_BUTTON,
	MGUI_ICON_TYPE_END,
};

struct mgui_icon {
	void *mgui; /* mgui context */
	const char *name;
	int id;
	enum e_icon_type type;
	void *h; /* directfb handle */

	/* information used only for image icons */
	struct {
		const IMAGE_ARRAY_ITEM *arr;
		const size_t arr_size;
		int (*val_to_key)(struct mgui_icon *this, void *data);
		int default_key; /* default image key */
		int current_key; /* current image key */
		OBJECT_RECT rect; /* rectangle representing the layout position */
	} image;
	/* information for icons containing text */
	struct {
		const char *font_path;
		const char *default_string;
		const char *(*val_to_string)(struct mgui_icon *this, void *data);
		unsigned int color; /*in argb format */
	} text;
	/* information for button icons */
	struct {
		ButtonOnClickParams click_params;
		void *data; /* private data */
		int status; /* up or down */
	} button;
};

struct mgui_background {
	void *mgui; /* mgui context */
	void *h; /* directfb handle */
	char *path; /* image path */

	/* layout information */
	struct {
		unsigned int icons_height;
		unsigned int operator_x;
		unsigned int operator_y;
		unsigned int wifi_ssid_x;
		unsigned int wifi_ssid_y;
		unsigned int buttons_x;
		unsigned int buttons_y;
		unsigned int buttons_space;
	} info;
};

#endif
