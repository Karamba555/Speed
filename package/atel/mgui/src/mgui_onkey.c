
/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_onkey.c
 *
 *  Authors:  Tomer Eliyahu
 *
 *  Description: MGUI onkey interface
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
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#include "mgui.h"
#include "mgui_utils.h"
#include "mgui_onkey.h"

#define ONKEY_DEV "/dev/input/event0"
#define WPSKEY_DEV "/tmp/mgui_wps_action"
#define CHARGER_DEV "/tmp/charger_status"
#define ALERTKEY_DEV "/tmp/mgui_alert_action"

extern char g_FOTA[];
extern char g_Reset[];
extern int dspover,olddspover;
extern unsigned int nowTime,lastTime,interval;
extern void pm_set_screen(struct mgui_context *ctx, int on);
/*
unsigned int read_poweron_time(){
        FILE *fp = NULL;
        char buf[64] = {0};
        double tmppwon,tmpidle;
        unsigned int pwon = -1;
        unsigned int idle = -1 ; 

        fp = fopen("/proc/uptime" , "r");
        if ( fp != NULL ) {
                if ( fgets( buf, sizeof(buf), fp ) != NULL ) {
                        sscanf(buf,"%lf %lf",&tmppwon,&tmpidle);
                        pwon = (unsigned int)tmppwon;
                        idle = (unsigned int)tmpidle;
						DBG_MSG("%s L%d*******pwon=%d idle=%d\n",__FUNCTION__,__LINE__,pwon,idle);
                }
                fclose(fp);
        }
        return pwon;
}
*/
static void *mgui_onkey_handler(void *arg)
{
	struct mgui_onkey_context *onkey = (struct mgui_onkey_context *)arg;
	struct mgui_context *mgui = onkey->mgui;
	struct input_event data;
	unsigned int press = 0;
	int fd = -1;
	char buf[128],tmpBuf[128]="",outBuf[128]="";
	int ret;
	struct mgui_event e = {.id = MGUI_ONKEY_EVENT,};

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	fd = open(ONKEY_DEV, O_RDONLY);
	if (fd < 0) {
		MGUI_EMSG("Error open %s\n", ONKEY_DEV);
		e.id = MGUI_EXIT_EVENT;
		write(mgui->pipes_fd[1], &e, sizeof(struct mgui_event));
		return arg;
	}

	do {
		read(fd, &data, sizeof(data));

		if ((data.type != EV_KEY) || (data.code != KEY_POWER) || (!!data.value == press))
			continue;
		if (!!data.value) {
			/* key pressed */
			press = 1;
		} else {
			/* key released, dispatch event */
			press = 0;
			DBG_MSG("onkey pressed\n");
			// TODO - we might need a mutex here

            if(mgui->state == MGUI_STATE_OFF)
            {
                pm_set_screen( mgui, 1 );
            }
			nowTime = read_poweron_time();
			interval = nowTime - lastTime;


			
			DBG_MSG("%s %d*******interval=%u mgui->state=%d dspover=%d\n",__FUNCTION__,__LINE__,interval,mgui->state,dspover);

			if(mgui->state == MGUI_STATE_FIRST || mgui->state == MGUI_STATE_SECOND || mgui->state == MGUI_STATE_THIRD || mgui->state == MGUI_STATE_FORTH)
			{
				if(mgui->state != dspover)
				{
					DBG_MSG("screen no finish, skip\n");
					continue;
				}
			}


			if ((mgui->state == MGUI_STATE_FIRST || mgui->state == MGUI_STATE_OFF) && interval < 120)
			{
				DBG_MSG("screen too fast, skip\n");
				continue;
			}
			else if (mgui->state == MGUI_STATE_SECOND && interval < 70 )
			{
				DBG_MSG(" screen too fast, skip\n");
				continue;
			}
			else if (mgui->state == MGUI_STATE_THIRD && interval < 68 )
			{
				DBG_MSG(" screen too fast, skip\n");
				continue;
			}
			else if (mgui->state == MGUI_STATE_FORTH && interval < 68 )
			{
				DBG_MSG(" screen too fast, skip\n");
				continue;
			}
			else if (mgui->state > MGUI_STATE_FORTH && interval < 68 )
			{
				DBG_MSG(" screen too fast, skip\n");
				continue;
			}
			else
			{
					DBG_MSG("%s L%d*******mgui event start\n",__FUNCTION__,__LINE__);
					lastTime = nowTime;
					write(mgui->pipes_fd[1], &e, sizeof(struct mgui_event));
					//olddspover = dspover;
					DBG_MSG("%s L%d*******mgui event end\n",__FUNCTION__,__LINE__);
			}
			/*if(dspover != olddspover)
			{
				write(mgui->pipes_fd[1], &e, sizeof(struct mgui_event));
				olddspover = dspover;
			}
			*/
		}
	} while (1);

	close(fd);
	return arg;
}

static void *mgui_alert_onkey_handler(void *arg)
{
	struct mgui_onkey_context *alert_onkey = (struct mgui_onkey_context *)arg;
	struct mgui_context *mgui = alert_onkey->mgui;
	// struct input_event data;
	unsigned int press = 0;
	int fd = -1;
	char buf[128];
	char buf1[128];
	struct mgui_event e = {.id = MGUI_ONKEY_EVENT,};

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	MGUI_EMSG("mgui_alert_onkey_handler\n");
	do {
		fd = open(ALERTKEY_DEV, O_RDONLY);
		if (fd > 0) {
			memset(buf, 0, sizeof(buf));

			read(fd, buf, sizeof(buf) - 1);
			strtok(buf, "\n");
			MGUI_EMSG("[buf=%s]\n", buf);
			if(strcmp("FOTA",buf)==0){
				strcpy(g_FOTA,"1");
			}
			else if(strcmp("Reset",buf)==0){
				strcpy(g_Reset,"1");
			}
			e.id = MGUI_ALERT_ONKEY_EVENT;
			write(mgui->pipes_fd[1], &e, sizeof(struct mgui_event));
			system("rm -rf /tmp/mgui_alert_action");
		}
	} while (1);

	close(fd);
	return arg;
}

static void *mgui_wpskey_handler(void *arg)
{
	struct mgui_onkey_context *wpskey = (struct mgui_onkey_context *)arg;
	struct mgui_context *mgui = wpskey->mgui;
	struct input_event data;
	unsigned int press = 0;
	int fd = -1;
	char buf[128],tmpBuf[128]="",outBuf[128]="";
	int ret;
	struct mgui_event e = {.id = MGUI_WPS_EVENT,};
	DBG_MSG("enter mgui_wpskey_handler\n");
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	do {
	fd = open(WPSKEY_DEV, O_RDONLY);
	if (fd > 0) {
		DBG_MSG("wpskey pressed\n");
		e.id = MGUI_WPS_EVENT;
		write(mgui->pipes_fd[1], &e, sizeof(struct mgui_event));
		system("rm -rf /tmp/mgui_wps_action");
	}
	} while (1);
	close(fd);
	return arg;
}
static void *mgui_chargerinfo_handler(void *arg)
{
	struct mgui_chargerinfo_context *chargerinfo = (struct mgui_chargerinfo_context *)arg;
	struct mgui_context *mgui = chargerinfo->mgui;
	struct input_event data;
	unsigned int press = 0;
	int fd = -1;
	char buf[128],tmpBuf[128]="",outBuf[128]="";
	int ret;
	struct mgui_event e = {.id = MGUI_CHARGER_EVENT,};
	DBG_MSG("enter mgui_chargerinfo_handler\n");
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	do {
	fd = open(CHARGER_DEV, O_RDONLY);
	if (fd > 0) {
		DBG_MSG("charger connect\n");
		e.id = MGUI_CHARGER_EVENT;
		write(mgui->pipes_fd[1], &e, sizeof(struct mgui_event));
		system("rm -rf /tmp/charger_status");
	}
	} while (1);
	close(fd);
	return arg;
}
int mgui_onkey_exit(struct mgui_onkey_context *onkey)
{
	int ret = 0;
	void *res;

	ret = pthread_cancel(onkey->tid);
	if (ret) {
		MGUI_EMSG("pthread cancle error %d\n", ret);
		goto out;
	}

	ret = pthread_join(onkey->tid, &res);
	if (ret) {
		MGUI_EMSG("pthread join error %d\n", ret);
		goto out;
	}
	if (res != PTHREAD_CANCELED) {
		MGUI_EMSG("thread not cancelled error %d\n", (int)res);
		goto out;
	}

	MGUI_IMSG("exit done\n");
out:
	free(onkey);
	return ret;
}

struct mgui_onkey_context *mgui_onkey_init(struct mgui_context *ctx)
{
	struct mgui_onkey_context *onkey;
	int ret;

	onkey = malloc(sizeof(*onkey));
	if (!onkey) {
		MGUI_EMSG("memory allocation failed\n");
		return NULL;
	}

	onkey->mgui = ctx;

	ret = pthread_create(&onkey->tid, NULL, mgui_onkey_handler, onkey);
	if (ret != 0) {
		MGUI_EMSG("pthread create error %d\n", ret);
		goto out_pthread_create;
	}

	MGUI_IMSG("init done\n");
	return onkey;

out_pthread_create:
	free(onkey);
	return NULL;
}
struct mgui_wpskey_context *mgui_wpskey_init(struct mgui_context *ctx)
{
	struct mgui_onkey_context *wpskey;
	int ret;
	DBG_MSG("enter mgui_wpskey_init\n");
	wpskey = malloc(sizeof(*wpskey));
	if (!wpskey) {
		MGUI_EMSG("memory allocation failed\n");
		return NULL;
	}

	wpskey->mgui = ctx;

	ret = pthread_create(&wpskey->tid, NULL, mgui_wpskey_handler, wpskey);
	if (ret != 0) {
		MGUI_EMSG("pthread create error %d\n", ret);
		goto out_pthread_create;
	}

	DBG_MSG("init done\n");
	return wpskey;

out_pthread_create:
	free(wpskey);
	return NULL;
}
struct mgui_chargerinfo_context *mgui_chargerinfo_init(struct mgui_context *ctx)
{
	struct mgui_chargerinfo_context *chargerinfo;
	int ret;
	DBG_MSG("enter mgui_chargerinfo_init\n");
	chargerinfo = malloc(sizeof(*chargerinfo));
	if (!chargerinfo) {
		MGUI_EMSG("memory allocation failed\n");
		return NULL;
	}
	chargerinfo->mgui = ctx;
	ret = pthread_create(&chargerinfo->tid, NULL, mgui_chargerinfo_handler, chargerinfo);
	if (ret != 0) {
		MGUI_EMSG("pthread create error %d\n", ret);
		goto out_pthread_create;
	}
	DBG_MSG("init done\n");
	return chargerinfo;
out_pthread_create:
	free(chargerinfo);
	return NULL;
}

struct mgui_alert_onkey_context *mgui_alert_onkey_init(struct mgui_context *ctx)
{
	struct mgui_alert_onkey_context *alert_onkey;
	int ret;

	alert_onkey = malloc(sizeof(*alert_onkey));
	if (!alert_onkey) {
		MGUI_EMSG("memory allocation failed\n");
		return NULL;
	}

	alert_onkey->mgui = ctx;

	ret = pthread_create(&alert_onkey->tid, NULL, mgui_alert_onkey_handler, alert_onkey);
	if (ret != 0) {
		MGUI_EMSG("pthread create error %d\n", ret);
		goto out_pthread_create;
	}

	MGUI_IMSG("init done\n");
	return alert_onkey;

out_pthread_create:
	free(alert_onkey);
	return NULL;
}
