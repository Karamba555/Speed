
/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* ----------------------------------------------------------------------------
 *
 *  Filename: mgui_icons.h
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
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Image.h"
#include "mgui_config.h"
#include "mgui_icons.h"
#include "mgui_utils.h"
#include "mgui_ril.h"
#include "mgui_charger.h"
#include "mgui_wifi.h"
#include "mgui.h"
#include "mgui_version.h"
#include "user_conf.h"

#define WAN_STATUS_FILE 	"/tmp/wan_status"
#define BAT_CAPACITY_FILE 	"/sys/class/power_supply/battery/capacity"
#define BAT_PRESENT_FILE 	"/sys/class/power_supply/battery/present"
#define BAT_STATUS_FILE 	"/sys/class/power_supply/battery/status"
#define ACCHG_ONLINE_FILE	"/sys/class/power_supply/ac/online"
extern char g_client_n[64];
/******************************************************************************
 *   Image Icons
 ******************************************************************************/

/*----------------------------------------------------------------------------
 * BATTERY
 *---------------------------------------------------------------------------*/
static const IMAGE_ARRAY_ITEM battery_icon_arr[] = {
	#if defined CONFIG_USER_HKM
	[0] = { .key = -1, .path = IMAGEDIR"/battery/unknown.png", },
	[1] = { .key = 0,  .path = IMAGEDIR"/battery/hkm/discharge_0.png", },
	[2] = { .key = 20,  .path = IMAGEDIR"/battery/hkm/discharge_25.png", },
	[3] = { .key = 50,  .path = IMAGEDIR"/battery/hkm/discharge_50.png", },
	[4] = { .key = 70,  .path = IMAGEDIR"/battery/hkm/discharge_75.png", },
	[5] = { .key = 100,  .path = IMAGEDIR"/battery/hkm/discharge_100.png", },
	[6] = { .key = 1,  .path = IMAGEDIR"/battery/hkm/charging_0.png", },
	[7] = { .key = 21,  .path = IMAGEDIR"/battery/hkm/charging_25.png", },
	[8] = { .key = 51,  .path = IMAGEDIR"/battery/hkm/charging_50.png", },
	[9] = { .key = 71,  .path = IMAGEDIR"/battery/hkm/charging_75.png", },
	[10] = { .key = 101,  .path = IMAGEDIR"/battery/hkm/charging_100.png", },
    #else
    [0] = { .key = 0,   .path = IMAGEDIR"/battery/discharge_low.png", },
    [1] = { .key = 20,  .path = IMAGEDIR"/battery/discharge_20.png", },
    [2] = { .key = 30,  .path = IMAGEDIR"/battery/discharge_30.png", },
    [3] = { .key = 50,  .path = IMAGEDIR"/battery/discharge_50.png", },
    [4] = { .key = 60,  .path = IMAGEDIR"/battery/discharge_60.png", },
    [5] = { .key = 80,  .path = IMAGEDIR"/battery/discharge_80.png", },
    [6] = { .key = 90,  .path = IMAGEDIR"/battery/discharge_90.png", },
    [7] = {	.key = 100, .path = IMAGEDIR"/battery/discharge_100.png", },
    [8] = { .key = 21,  .path = IMAGEDIR"/battery/charging_20.png", },
    [9] = { .key = 31,  .path = IMAGEDIR"/battery/charging_30.png", },
    [10] = { .key = 51, .path = IMAGEDIR"/battery/charging_50.png", },
    [11] = { .key = 61, .path = IMAGEDIR"/battery/charging_60.png", },
    [12] = { .key = 81, .path = IMAGEDIR"/battery/charging_80.png", },
    [13] = { .key = 91, .path = IMAGEDIR"/battery/charging_90.png", },
    [14] = { .key = 101,.path = IMAGEDIR"/battery/charging_100.png", },
	[15] = { .key = -1, .path = IMAGEDIR"/battery/unknown.png", },
    #endif
	
};

static int read_file(char *file_name, char *buf)
{	
	int fd = -1;
	int ret;
	char str_temp[20];

	memset(str_temp, 0, sizeof(str_temp));
	fd = open(file_name, O_RDONLY);
	if (fd < 0){
		DBG_MSG("open %s failed\n", file_name);
		return -1;
	}
	ret = read(fd, str_temp, sizeof(str_temp));
	if(ret < 0) {
		DBG_MSG("[%s] file read error\n", __FUNCTION__);
		close(fd);
		return -1;
	}
	memcpy(buf, str_temp, sizeof(str_temp));
	close(fd);

	return 0;
}
#if defined CONFIG_USER_HKM
static int battery_val2key(struct mgui_icon *this, void *data)
{
    int preset, capacity;	
    int ret;
    char buf[24];
    int charging = 0;

    memset(buf, 0, sizeof(buf));
    DBG_MSG("%s %d\n",__FUNCTION__,__LINE__);
    ret = read_file(BAT_PRESENT_FILE, buf);
    if(ret < 0)
    {
        DBG_MSG("%s %d ret<0\n",__FUNCTION__,__LINE__);
        return -1;
    }

    preset = atoi(buf);
    DBG_MSG("%s %d preset=%d\n",__FUNCTION__,__LINE__,preset);
    if(preset == 0)
    {
        DBG_MSG("%s %d no battery\n",__FUNCTION__,__LINE__);
        return STATUS_UNKNOWN;
    }

    memset(buf, 0, sizeof(buf));
    ret = read_file(BAT_STATUS_FILE, buf);
    DBG_MSG("%s %d buf[%s]\n",__FUNCTION__,__LINE__,buf);
    if(strstr(buf,"Charging") != NULL || strstr(buf,"Not charging") != NULL )
    {
        charging = 1;
    }
    DBG_MSG("%s %d charging=%d\n",__FUNCTION__,__LINE__,charging);
    memset(buf, 0, sizeof(buf));
    ret = read_file(BAT_CAPACITY_FILE, buf);
    if(ret < 0)
    {
        DBG_MSG("%s %d NO battery\n",__FUNCTION__,__LINE__);
        return STATUS_UNKNOWN;
    }
    capacity = atoi(buf);	
    DBG_MSG("%s %d capacity=%d\n",__FUNCTION__,__LINE__,capacity);
    if (capacity >= 75)
    return 100+charging;
    else if (capacity >= 50)
    return 70+charging;
    else if (capacity >= 25)
    return 50+charging;
    else if (capacity > 0)
    return 20+charging;
    else if (capacity == 0)
    return charging;
    else
    return 0;
}
#else
static int battery_val2key(struct mgui_icon *this, void *data)
{
	struct mgui_charger_context *ctx = (struct mgui_charger_context *)data;
	enum chg_status_stat status;
	int capacity, charging = 0;

	if (data == NULL)
		return this->image.default_key;

	status = get_chg_bat_status(ctx);
	if (status == STATUS_UNKNOWN)
		return -1;
	if (status == STATUS_CHARGING)
		charging = 1;
	capacity = get_chg_bat_capacity(ctx);
	if (capacity == 100)
		return 100 + charging;
	if (capacity >= 80)
		return 80 + charging;
	if (capacity >= 60)
		return 60 + charging;
	if (capacity >= 50)
		return 50 + charging;
	if (capacity >= 30)
		return 30 + charging;
	if (capacity >= 20)
		return 20 + charging;
	return 0 + charging;
}
#endif
static struct mgui_icon battery_icon = {
	.name = "battery",
	.type = MGUI_ICON_TYPE_IMAGE,
	.image = {
		.arr = battery_icon_arr,
		.arr_size = ARRAY_SIZE(battery_icon_arr),
		.val_to_key = battery_val2key,
		.default_key = 0, /* unknown battery status */
	},
};
/*----------------------------------------------------------------------------
 * BLUETOOTH
 *---------------------------------------------------------------------------*/
static const IMAGE_ARRAY_ITEM bluetooth_icon_arr[] = {
	[0] = { .key = 0,   .path = IMAGEDIR"/bluetooth/off.png", },
	[1] = { .key = 20,  .path = IMAGEDIR"/bluetooth/on.png", },
	[2] = { .key = 30,  .path = IMAGEDIR"/bluetooth/connected.png", },
};

static int bluetooth_val2key(struct mgui_icon *this, void *data)
{
	return this->image.default_key;
}

static struct mgui_icon bluetooth_icon = {
	.name = "bluetooth",
	.type = MGUI_ICON_TYPE_IMAGE,
	.image = {
		.arr = bluetooth_icon_arr,
		.arr_size = ARRAY_SIZE(bluetooth_icon_arr),
		.val_to_key = bluetooth_val2key,
		.default_key = 0, /* off */
	},
};

/*----------------------------------------------------------------------------
 * RSSI
 *---------------------------------------------------------------------------*/
static const IMAGE_ARRAY_ITEM cellular_icon_arr[] = {
	#if defined CONFIG_USER_HKM
	[0] = { .key = 0, .path = IMAGEDIR"/cellular/hkm/signal_0_bar.png", },
	[1] = {	.key = 1, .path	= IMAGEDIR"/cellular/hkm/signal_1_bar.png", },
	[2] = { .key = 2, .path	= IMAGEDIR"/cellular/hkm/signal_2_bar.png", },
	[3] = { .key = 3, .path	= IMAGEDIR"/cellular/hkm/signal_3_bar.png", },
	#else
	[0] = { .key = 0, .path = IMAGEDIR"/cellular/signal_0_bar.png", },
    [1] = { .key = 1, .path = IMAGEDIR"/cellular/signal_1_bar.png", },
    [2] = { .key = 2, .path = IMAGEDIR"/cellular/signal_2_bar.png", },
    [3] = { .key = 3, .path = IMAGEDIR"/cellular/signal_3_bar.png", },
	#endif
	[4] = { .key = 4, .path	= IMAGEDIR"/cellular/signal_4_bar.png", },
	[5] = { .key = 5, .path	= IMAGEDIR"/cellular/signal_0_bar_no_data.png", },
	[6] = { .key = 6, .path	= IMAGEDIR"/cellular/signal_1_bar_no_data.png", },
	[7] = { .key = 7, .path	= IMAGEDIR"/cellular/signal_2_bar_no_data.png", },
	[8] = { .key = 8, .path	= IMAGEDIR"/cellular/signal_3_bar_no_data.png", },
	[9] = { .key = 9, .path	= IMAGEDIR"/cellular/signal_4_bar_no_data.png", },
	[10] = { .key = 10, .path = IMAGEDIR"/cellular/no_signal.png", },
	[11] = { .key = 11, .path = IMAGEDIR"/cellular/off.png", },
	[12] = { .key = 111, .path = IMAGEDIR"/cellular/default.png", },
};

/*
	   bars   |      dbm        |     rssi     
	------------------------------------------
	     4    |  -77  or higher |  18 or higher
	     3    |  -78  to -86    |  17 to 13    
	     2    |  -87  to -92    |  12 to 10    
	     1    |  -93  to -101   |  9  to 5     
	     0    |  -102 or lower  |  4 or lower  
	     null |       NA        |  99 (unknown)
*/

/* data is ril context */
#if 1
static int cellular_val2key(struct mgui_icon *this,void *data)
{
	int ret;
	char buf[256],tmpBuf[64]="",outBuf[64]="";
	char signalLevel[64] = "";
        FILE *fp = fopen(WAN_STATUS_FILE, "r");
	int sig_status = this->image.default_key;	
        DBG_MSG("%s %d,sig_status:%d\n",__FUNCTION__,__LINE__,sig_status);
        if (fp != NULL)
        {	
                DBG_MSG("%s %d\n",__FUNCTION__,__LINE__);
                while (fgets(buf, sizeof(buf), fp) != NULL) 
                {
                        memset(outBuf,0,sizeof(outBuf));
                        sscanf(buf, "%[^:]:%[^;];", tmpBuf,outBuf);	
                        
                        if(strstr(tmpBuf,"Signal Level") != NULL) {
                                if(outBuf != NULL)
                                {
                                        strcpy(signalLevel,outBuf);
                                        if(strcmp(signalLevel, "Bad Signal")==0)
                                        sig_status = 0;
                                        else if(strcmp(signalLevel, "Low Signal")==0)
                                        sig_status = 1;
                                        else if(strcmp(signalLevel, "Middle Signal")==0)
                                        sig_status = 2;
                                        else if(strcmp(signalLevel, "Good Signal")==0)
                                        sig_status = 3;
                                        else if(strcmp(signalLevel, "Excellent Signal")==0)
                                        #if defined CONFIG_USER_HKM
                                        sig_status = 3;
                                        #else
                                        sig_status = 4;
                                        #endif
                                }
                                DBG_MSG("%s %d sig_status=%d signalLevel=%s\n",__FUNCTION__,__LINE__,sig_status,signalLevel);
                                break;
                        }
                }
                fclose(fp);
        }
	return  sig_status;
}
#else
static int cellular_val2key(struct mgui_icon *this, void *data)
{
	struct mgui_ril_context *ril = (struct mgui_ril_context *)data;
	unsigned int image_key_offset = 0;

	if (data == NULL)
		return this->image.default_key;


	if (ril->simcard_state != RIL_CARDSTATE_PRESENT)
		return 10; /* cellular null icon */

#ifdef ENABLE_USBU_VOICE
	if (!REGISTERED_VOICE(ril))
		return 10; /* cellular null icon*/
#endif

	if (!REGISTERED_DATA(ril))
		image_key_offset = 5; /* bars with no connection mark */

	if (ril->rssi == 99)
		return 12;
	if (ril->rssi >= 18)
		return 4 + image_key_offset;
	if (ril->rssi >= 13)
		return 3 + image_key_offset;
	if (ril->rssi >= 10)
		return 2 + image_key_offset;
	if (ril->rssi >= 5)
		return 1 + image_key_offset;
	return 0 + image_key_offset;
#endif

static struct mgui_icon cellular_icon = {
	.name = "cellular signal",
	.type = MGUI_ICON_TYPE_IMAGE,
	.image = {
		.arr = cellular_icon_arr,
		.arr_size = ARRAY_SIZE(cellular_icon_arr),
		.val_to_key = cellular_val2key,
		.default_key = 10, /* cellular off */
	},
};

/*----------------------------------------------------------------------------
 * GPS
 *---------------------------------------------------------------------------*/
static const IMAGE_ARRAY_ITEM gps_icon_arr[] = {
	[0] = {.key = 0, .path = IMAGEDIR"/gps/off.png", },
	[1] = {.key = 1, .path = IMAGEDIR"/gps/on.png", },
	[2] = {.key = 2, .path = IMAGEDIR"/gps/fixed.png", },
};

static int gps_val2key(struct mgui_icon *this, void *data)
{
	/* TODO */
	return this->image.default_key;
}

static struct mgui_icon gps_icon = {
	.name = "gps",
	.type = MGUI_ICON_TYPE_IMAGE,
	.image = {
		.arr = gps_icon_arr,
		.arr_size = ARRAY_SIZE(gps_icon_arr),
		.val_to_key = gps_val2key,
		.default_key = 0, /* gps off */
	},
};

/*----------------------------------------------------------------------------
 * WIFI
 *---------------------------------------------------------------------------*/
static const IMAGE_ARRAY_ITEM wifi_icons_arr[] = {
	[0] = {.key = 0, .path = IMAGEDIR"/wireless/signal_0_bar.png", },
	[1] = {.key = 1, .path = IMAGEDIR"/wireless/signal_1_bar.png", },
	[2] = {.key = 2, .path = IMAGEDIR"/wireless/signal_2_bar.png", },
	[3] = {.key = 3, .path = IMAGEDIR"/wireless/signal_3_bar.png", },
	#if defined CONFIG_USER_HKM
	[4] = {.key = 4, .path = IMAGEDIR"/wireless/hkm/signal_4_bar.png", },
	#else
	[4] = {.key = 4, .path = IMAGEDIR"/wireless/signal_4_bar.png", },
	#endif
	[5] = {.key = 5, .path = IMAGEDIR"/wireless/signal_1_bar_lock.png", },
	[6] = {.key = 6, .path = IMAGEDIR"/wireless/signal_2_bar_lock.png", },
	[7] = {.key = 7, .path = IMAGEDIR"/wireless/signal_3_bar_lock.png", },
	[8] = {.key = 8, .path = IMAGEDIR"/wireless/signal_4_bar_lock.png", },
	[9] = {.key = 9, .path = IMAGEDIR"/wireless/off.png", },
	[10] = {.key = 10, .path = IMAGEDIR"/wireless/tethering.png", },
	#if defined CONFIG_USER_HKM
	[11] = {.key = 111, .path = IMAGEDIR"/wireless/hkm/default.png", },
	[12] = {.key = 12, .path = IMAGEDIR"/wireless/hkm/signal_5_bar.png", },
	#else
	[11] = {.key = 111, .path = IMAGEDIR"/wireless/default.png", },
	#endif
};

#if 1
static int wifi_val2key(struct mgui_icon *this, void *data)
{
	int wifi_status = 0;
	char buf[64];
	FILE *fp = popen("uci get wireless.AP1_2G.disabled", "r");
	if (fp != NULL)
	{
		while (fgets(buf, sizeof(buf), fp) != NULL) 
		{
			if(atoi(buf) == 0)
			{
				#if defined CONFIG_USER_HKM
				if(strcmp(g_client_n,"0"))
				{
					wifi_status = 12;
					break;
				}
				else
				{
					wifi_status = 12;
					break;
				}
				#else
				wifi_status = 4;
				break;
				#endif
			}
			else
			{
				wifi_status = 9;
				memset(g_client_n,0,sizeof(g_client_n));
				break;
			}
		}
		pclose(fp);
	}
	else
	{
		wifi_status = 111;
	}
	DBG_MSG("%s L%d*****wifi_status=%d**g_client_n=%s\n",__FUNCTION__,__LINE__,wifi_status,g_client_n);
	return wifi_status;

}
#else
static int wifi_val2key(struct mgui_icon *this, void *data)
{
	struct mgui_wifi_context *wifi = (struct mgui_wifi_context *)data;

	if (data == NULL)
		return this->image.default_key;
	

	if (get_wifi_status(wifi))
		return 4; /* wifi is on */
	else
		return 9; /* wifi is off */
}
#endif
static struct mgui_icon wifi_icon = {
	.name = "wifi",
	.type = MGUI_ICON_TYPE_IMAGE,
	.image = {
		.arr = wifi_icons_arr,
		.arr_size = ARRAY_SIZE(wifi_icons_arr),
		.val_to_key = wifi_val2key,
		.default_key = 111, /* wifi off */
	},
};

/*----------------------------------------------------------------------------
 * SMS
 *---------------------------------------------------------------------------*/
static const IMAGE_ARRAY_ITEM sms_icon_arr[] = {
	[0] = {.key = 0, .path = IMAGEDIR"/sms/no_sms.png",},
	[1] = {.key = 1, .path = IMAGEDIR"/sms/hkm/sms.png",},
};

static int sms_val2key(struct mgui_icon *this, void *data)
{
	/* TODO */
	return this->image.default_key;
}

static struct mgui_icon sms_icon = {
	.name = "sms",
	.type = MGUI_ICON_TYPE_IMAGE,
	.image = {
		.arr = sms_icon_arr,
		.arr_size = ARRAY_SIZE(sms_icon_arr),
		.val_to_key = sms_val2key,
		.default_key = 1, /* no sms */
	},
};

/*----------------------------------------------------------------------------
 * SDCARD
 *---------------------------------------------------------------------------*/
static const IMAGE_ARRAY_ITEM sdcard_icon_arr[] = {
	[0] = {.key = 0, .path = IMAGEDIR"/sdcard/missing.png",},
	[1] = {.key = 1, .path = IMAGEDIR"/sdcard/on.png",},
};

static int sdcard_val2key(struct mgui_icon *this, void *data)
{
	if (data) {
		int on = (int)data;
		if (on)
			return 1;
		return 0;
	}
	
	return this->image.default_key;
}

static struct mgui_icon sdcard_icon = {
	.name = "sdcard",
	.type = MGUI_ICON_TYPE_IMAGE,
	.image = {
		.arr = sdcard_icon_arr,
		.arr_size = ARRAY_SIZE(sdcard_icon_arr),
		.val_to_key = sdcard_val2key,
		.default_key = 0, /* sdcard on */
	},
};

/*----------------------------------------------------------------------------
 * SIM CARD
 *---------------------------------------------------------------------------*/

static const IMAGE_ARRAY_ITEM sim_icon_arr[] = {
	[0] = {.key = 0, .path = IMAGEDIR"/simcard/missing.png", },
	[1] = {.key = 1, .path = IMAGEDIR"/simcard/on.png", },
};

static int simcard_val2key(struct mgui_icon *this, void *data)
{
	struct mgui_ril_context *ril = (struct mgui_ril_context *)data;

	if (data == NULL)
		return this->image.default_key;

	return (ril->simcard_state == RIL_CARDSTATE_PRESENT) ? 1 : 0;
}

static struct mgui_icon simcard_icon = {
	.name = "sim card",
	.type = MGUI_ICON_TYPE_IMAGE,
	.image = {
		.arr = sim_icon_arr,
		.arr_size = ARRAY_SIZE(sim_icon_arr),
		.val_to_key = simcard_val2key,
		.default_key = 0, /* simcard alert */
	},
};

/*----------------------------------------------------------------------------
 * NETWORK TECHNOLOGY
 *---------------------------------------------------------------------------*/

static const IMAGE_ARRAY_ITEM network_tech_icon_arr[] = {
	[0] = {.key = 0, .path = IMAGEDIR"/cellular/tech/unknown.png", },
	[1] = {.key = 1, .path = IMAGEDIR"/cellular/tech/2g.png", },
	[2] = {.key = 2, .path = IMAGEDIR"/cellular/tech/3g.png", },
	[3] = {.key = 3, .path = IMAGEDIR"/cellular/tech/4g.png", },
	[4] = {.key = 4, .path = IMAGEDIR"/cellular/tech/e.png", },
	[5] = {.key = 5, .path = IMAGEDIR"/cellular/tech/h.png", },
	[6] = {.key = 6, .path = IMAGEDIR"/cellular/tech/h+.png", },
	[7] = {.key = 7, .path = IMAGEDIR"/cellular/tech/g.png", },
	[8] = {.key = 111, .path = IMAGEDIR"/cellular/tech/default.png", },
};

#if 1
static int network_tech_val2key(struct mgui_icon *this, void *data)
{
	int tech_status = 0;	
	int ret;
	char buf[256],tmpBuf[64]="",outBuf[64]="";
	char net_tech[64] = "";
	FILE *fp = fopen(WAN_STATUS_FILE, "r");
	 if (fp != NULL)
        {	
                while (fgets(buf, sizeof(buf), fp) != NULL) 
                {
                        memset(outBuf,0,sizeof(outBuf));
                        sscanf(buf, "%[^:]:%[^;];", tmpBuf,outBuf);	
                       if(strstr(tmpBuf,"Modem Type") != NULL) {
                               if(outBuf != NULL)
                               {
                                       strcpy(net_tech,outBuf);
                                       if(strcmp(net_tech, "4G")==0)
                                       tech_status = 3;
                                       else if(strcmp(net_tech, "3G")==0)
                                       tech_status = 2;
                               }
                               else 
                               return this->image.default_key;
                               break;
                        }
                }
                fclose(fp);
        }
	return  tech_status;
}
#else
static int network_tech_val2key(struct mgui_icon *this, void *data)
{
	struct mgui_ril_context *ril;

	if (data == NULL)
		return this->image.default_key;

	ril = (struct mgui_ril_context *)data;

	switch (ril->voice_registration.radio_tech) {
	case RADIO_TECH_GPRS:
		return 1; /* 2G */
	case RADIO_TECH_EDGE:
		return 4; /* E */
	case RADIO_TECH_UMTS:
		return 2; /* 3G */
	case RADIO_TECH_HSDPA:
	case RADIO_TECH_HSUPA:
	case RADIO_TECH_HSPAP: /* Not sure... */
		return 5; /* H */
	case RADIO_TECH_HSPA:
		return 6; /* H+ */
	case RADIO_TECH_LTE: 
	case RADIO_TECH_LTEP: /* temporarily set to 4G, infuture we need to set to 4G+ */
		return 3; /* 4G */
	case RADIO_TECH_GSM:
		return 7; /* G */
	default:
		return 0; /* ? */
	}
}
#endif

static struct mgui_icon network_icon = {
	.name = "network tech",
	.type = MGUI_ICON_TYPE_IMAGE,
	.image = {
		.arr = network_tech_icon_arr,
		.arr_size = ARRAY_SIZE(network_tech_icon_arr),
		.val_to_key = network_tech_val2key,
		.default_key = 111, /* unknown */
	},
};

/******************************************************************************
 *   Text Icons
 ******************************************************************************/
#define ARGB_RED		0x00ff0000
#define ARGB_GREEN		0x0000ff00
#define ARGB_BLUE		0x000000ff
#define ARGB_WHITE		0xffffffff

#define TEXT_FONT NULL

static const char *version_val2string(struct mgui_icon *this, void *data)
{
	struct mgui_version_context *ver = (struct mgui_version_context *)data;
	if (!ver) {
		this->text.color = ARGB_WHITE;
		return this->text.default_string;
	}
	this->text.color = ARGB_WHITE;
	return ver->info.sw_version;
}

static struct mgui_icon version_icon = {
	.name = "version",
	.type = MGUI_ICON_TYPE_TEXT,
	.text = {
		.font_path = (const char *)TEXT_FONT,
		.default_string = "Unknown Version",
		.val_to_string = version_val2string,
		.color = ARGB_WHITE,
	},
};

static const char *wifi_ssid_val2string(struct mgui_icon *this, void *data)
{
	struct mgui_wifi_context *wifi = (struct mgui_wifi_context *)data;
	if (!wifi) {
		this->text.color = ARGB_WHITE;
		return this->text.default_string;
	}
	this->text.color = ARGB_WHITE;
	return wifi->status.ssid;
}

static struct mgui_icon wifi_ssid_icon = {
	.name = "wifi ssid",
	.type = MGUI_ICON_TYPE_TEXT,
	.text = {
		.font_path = (const char *)TEXT_FONT,
		.default_string = "N/A",
		.val_to_string = wifi_ssid_val2string,
		.color = 0xffffffff, /* White */
	},
};
static const char *wifi_key_val2string(struct mgui_icon *this, void *data)
{
	struct mgui_wifi_context *wifi = (struct mgui_wifi_context *)data;
	if (!wifi) {
		this->text.color = ARGB_WHITE;
		return this->text.default_string;
	}
	this->text.color = ARGB_WHITE;
	return wifi->status.key;
}

static struct mgui_icon wifi_key_icon = {
	.name = "wifi key",
	.type = MGUI_ICON_TYPE_TEXT,
	.text = {
		.font_path = (const char *)TEXT_FONT,
		.default_string = "N/A",
		.val_to_string = wifi_key_val2string,
		.color = 0xffffffff, /* White */
	},
};
static const char *wifi_num_clients_val2string(struct mgui_icon *this, void *data)
{
	struct mgui_wifi_context *wifi = (struct mgui_wifi_context *)data;
	if (!wifi) {
		this->text.color = ARGB_WHITE;
		return this->text.default_string;
		
	}
	this->text.color = wifi->status.wifi_status ? ARGB_WHITE : ARGB_WHITE;
	return wifi->status.num_clients;
}

static struct mgui_icon wifi_num_clients_icon = {
	.name = "wifi num clients",
	.type = MGUI_ICON_TYPE_TEXT,
	.text = {
		.font_path = (const char *)TEXT_FONT,
		.default_string = NULL,
		.val_to_string = wifi_num_clients_val2string,
		.color = 0xffffffff, /* White */
	},
};
#ifdef ENABLE_USBU_OPERATOR
static const char *operator_val2string(struct mgui_icon *this, void *data)
{
	struct mgui_ril_context *ril = (struct mgui_ril_context *)data;

	if (data == NULL) {
		this->text.color = ARGB_WHITE;
		return this->text.default_string;
	}
	this->text.color = REGISTERED_VOICE(ril) ? ARGB_WHITE : ARGB_WHITE;
	return ril->Operator;
}

static struct mgui_icon operator_icon = {
	.name = "operator",
	.type = MGUI_ICON_TYPE_TEXT,
	.text = {
		.font_path = (const char *)TEXT_FONT,
		.default_string = NULL,
		.val_to_string = operator_val2string,
		.color = 0xffffffff, /* RED */
	},
};
#endif

static char time_str[TIME_FORMAT_SIZE];

static const char *clock_val2string(struct mgui_icon *this, void *data)
{
	set_time_string(time_str, TIME_FORMAT, TIME_FORMAT_SIZE);
	
	return (const char *)time_str;
}

static struct mgui_icon clock_icon = {
	.name = "clock",
	.type = MGUI_ICON_TYPE_TEXT,
	.text = {
		.font_path = (const char *)TEXT_FONT,
		.val_to_string = clock_val2string,
		.color = 0xffffffff, /* WHITE */
	},
};

/* icons database (order matters - see e_mgui_icons) */
struct mgui_icon *mgui_icons[] = {
	NULL,
	&battery_icon,	
	&cellular_icon,
	&wifi_icon,
	&network_icon,
	//&bluetooth_icon,

	//&gps_icon,

	//&wifi_ssid_icon,
	//&wifi_key_icon,
	&sms_icon,
	//&wifi_num_clients_icon,
	//&sdcard_icon,
	//&simcard_icon,
	
	//&operator_icon,
	//&clock_icon,
	//&version_icon,
};

size_t mgui_num_icons = ARRAY_SIZE(mgui_icons);

/*----------------------------------------------------------------------------
 * BUTTONS
 *---------------------------------------------------------------------------*/

#ifdef MARVELL_ORIGIN_LCD
static int button_val2key(struct mgui_icon *this, void *data)
{
	int pushed = (int)data;
	return pushed ? 1: 0;
}

static const IMAGE_ARRAY_ITEM button_fota_icon_arr[] = {
	[0] = { .key = 0, .path = IMAGEDIR"/buttons/fota/up.png", },
	[1] = { .key = 1, .path = IMAGEDIR"/buttons/fota/down.png", },
};

static struct mgui_icon fota_button = {
	.name = "fota_button",
	.type = MGUI_ICON_TYPE_BUTTON,
	.image = {
		.arr = button_fota_icon_arr,
		.arr_size = ARRAY_SIZE(button_fota_icon_arr),
		.val_to_key = button_val2key,
	},
};

static const IMAGE_ARRAY_ITEM button_keepalive_icon_arr[] = {
	[0] = { .key = 0, .path = IMAGEDIR"/buttons/keepalive/up.png", },
	[1] = { .key = 1, .path = IMAGEDIR"/buttons/keepalive/down.png", },
};

static struct mgui_icon keepalive_button = {
	.name = "keepalive_button",
	.type = MGUI_ICON_TYPE_BUTTON,
	.image = {
		.arr = button_keepalive_icon_arr,
		.arr_size = ARRAY_SIZE(button_keepalive_icon_arr),
		.val_to_key = button_val2key,
	},
};

static const IMAGE_ARRAY_ITEM button_nodata_icon_arr[] = {
	[0] = { .key = 0, .path = IMAGEDIR"/buttons/no_data_assert/up.png", },
	[1] = { .key = 1, .path = IMAGEDIR"/buttons/no_data_assert/down.png", },
};

static struct mgui_icon nodata_assert_button = {
	.name = "no data assert",
	.type = MGUI_ICON_TYPE_BUTTON,
	.image = {
		.arr = button_nodata_icon_arr,
		.arr_size = ARRAY_SIZE(button_nodata_icon_arr),
		.val_to_key = button_val2key,
	},
};

static const IMAGE_ARRAY_ITEM button_reset_icon_arr[] = {
	[0] = { .key = 0, .path = IMAGEDIR"/buttons/reset/up.png", },
	[1] = { .key = 1, .path = IMAGEDIR"/buttons/reset/down.png", },
};

static struct mgui_icon reset_button = {
	.name = "reset_button",
	.type = MGUI_ICON_TYPE_BUTTON,
	.image = {
		.arr = button_reset_icon_arr,
		.arr_size = ARRAY_SIZE(button_reset_icon_arr),
		.val_to_key = button_val2key,
	},
};
#endif

struct mgui_icon *mgui_buttons[] = {
#ifdef MARVELL_ORIGIN_LCD
	&fota_button,
	&keepalive_button,
	&nodata_assert_button,
	&reset_button,
#endif
};

size_t mgui_num_buttons = ARRAY_SIZE(mgui_buttons);

#if defined MARVELL_ORIGIN_LCD
struct mgui_background background = {
	.path = IMAGEDIR"/background.png",
	.info = {
		.icons_height = 24,
		.operator_x = 392,
		.operator_y = 85,
		.wifi_ssid_x = 392,
		.wifi_ssid_y = 185,
		.buttons_x = 17,
		.buttons_y = 29,
		.buttons_space = 6,
	},
};
#elif defined NOTION_L20_1D44_LCD
/*
struct mgui_background background = {
	.path = IMAGEDIR"/background_lcd_1d44.png",
	.info = {
		.icons_height = 15,
		.operator_x = 62,
		.operator_y = 49,
		.wifi_ssid_x = 62,
		.wifi_ssid_y = 97,
		.buttons_x = 2,
		.buttons_y = 19,
		.buttons_space = 3,
	},
};*/
struct mgui_background background = {
	 #if defined CONFIG_USER_HKM
	.path = IMAGEDIR"/screenblack.png",
	#else
	.path = IMAGEDIR"/screen1.png",
	#endif
	.info = {
		.icons_height = 32,
		.operator_x = 62,
		.operator_y = 150,
		.wifi_ssid_x = 62,
		.wifi_ssid_y = 97,
		.buttons_x = 100,
		.buttons_y = 30,
		.buttons_space = 3,
	},
};
struct mgui_background background2 = {
	.path = IMAGEDIR"/screen2.png",
	.info = {
		.icons_height = 32,
		.operator_x = 62,
		.operator_y = 150,
		.wifi_ssid_x = 62,
		.wifi_ssid_y = 50,
		.buttons_x = 2,
		.buttons_y = 19,
		.buttons_space = 3,
	},
};
#endif

