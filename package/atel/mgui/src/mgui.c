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
 *  Description: mgui
 *
 *  HISTORY:
 *   Nov 23, 2014 - Initial Version
 *
 *  Notes:
 *
 ******************************************************************************/
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <uci.h>
#include <uci_blob.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include "mgui_icons.h"
#include "mgui_utils.h"
#include "mgui_config.h"
#include "mgui_ubus.h"
#include "mgui_ril.h"
#include "mgui_charger.h"
#include "mgui_onkey.h"
#include "mgui_wifi.h"
#include "mgui_sms.h"
#include "mgui_hawk.h"
#include "mgui_version.h"
#include "mgui.h"
#include <poll.h>
#include "gpio.h"
#include "user_conf.h"

#define to_mgui_context(u) container_of(u, struct mgui_context, ubus_fd)

#define FB_DEV	"/dev/fb0"

/******************************************************************************
 *   Global variables
 ******************************************************************************/
extern struct mgui_background background;
extern struct mgui_background background2;
extern struct mgui_icon *mgui_icons[];
extern size_t mgui_num_icons;
extern struct mgui_icon *mgui_buttons[];
extern size_t mgui_num_buttons;
struct mgui_context *g_ctx;
char g_FOTA[16],g_FOTA_text[512],g_Reset[16],g_Reset_text[512];
char g_operator[64],g_client_n[64],g_ssid[64],g_key[128],g_swVersion[64],g_encryption[64],g_net_tech[64],g_sms_unread[64],g_sms_tip[64],g_sms_transfer[64],g_sms_time[64],g_sms_date[64],g_sms_info[4096];
int usimready,sms_unread =0,sms_long=0; wps_lock=0;
bool screen_update_info = false;
bool enter_sleep = false;
bool next_screen = false;

static void init_icons_index(struct mgui_context *ctx, struct mgui_icon **icons, int index, int text_height);
static void mgui_init1(int init,struct mgui_context *ctx, int poweron);
/******************************************************************************
 *   Code
 ******************************************************************************/

#ifdef MARVELL_ORIGIN_LCD
static void touch_handler(int event, void *params)
{
	struct mgui_context *ctx = (struct mgui_context *)params;
	struct itimerspec its;

	pthread_mutex_lock(&ctx->lock);
	timer_gettime(ctx->idle_timer, &its);
	pthread_mutex_unlock(&ctx->lock);
	if (its.it_value.tv_sec || its.it_value.tv_nsec) {
		/* timer is active, reschedule */
		its.it_value.tv_nsec = its.it_interval.tv_nsec;
		its.it_value.tv_sec = its.it_interval.tv_sec;
		timer_settime(ctx->idle_timer, 0, &its, NULL);
		MGUI_DMSG("touch timer reschedulred\n");
	}
}
#endif

/* top icons layout:
 *	----------------------------------------------------------
 *	| clk                           sd sms wifi gps rssi bat |
 *	|---------------------------------------------------------
 *	|			Cellular Operator
 *	|			   WIFI SSID		
 *	|
 *	...
 *	...
 *	|		 _______________________
 *	|		|			|
 *	|		|	EXIT BUTTON	|
 *	|		|_______________________|
 *	|			
 *	----------------------------------------------------------
 */

static int mgui_layout_top_first[] = { MGUI_WIFI_ICON, };
static int mgui_layout_top_second[] = { MGUI_CELLULAR_ICON, };
static int mgui_layout_top_third[] = { MGUI_BATTERY_ICON,};
static int mgui_layout_sms[] = { MGUI_SMS_ICON,};
//static int mgui_layout_top_fourth[] = { MGUI_NETWORK_TECH_ICON,};//instaed of text
/*
static int mgui_layout_top_right[] = { MGUI_BATTERY_ICON, MGUI_CELLULAR_ICON,
				       MGUI_GPS_ICON, MGUI_WIFI_ICON,
				       MGUI_SDCARD_ICON, MGUI_SIM_ICON,
				       MGUI_SMS_ICON };
*/
int dspover=MGUI_STATE_OFF,olddspover=MGUI_STATE_OFF;
unsigned int nowTime,lastTime,interval;
unsigned int sms_lastTime;
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
						pwon = tmppwon*100;
                        idle = tmpidle;
                }
                fclose(fp);
        }
        return pwon;
}
enum e_layout_dir {
	DIR_LEFT_TO_RIGHT,
	DIR_RIGHT_TO_LEFT,
};

#define GET_SCALED_WIDTH(orig_w, orig_h, new_h) \
	((orig_w) * (new_h)) / (orig_h)

#define GET_SCALED_HEIGHT(orig_w, orig_h, new_w) \
	((orig_h) * (new_w)) / (orig_w)

static inline OBJECT_RECT layout_icons(struct mgui_context *ctx,
					  int *layout_table,
					  int layout_table_size,
					  int x, int y, int height,
					  enum e_layout_dir dir)
{
	struct mgui_icon *icon;
	OBJECT_RECT rect = {};
	int i, width;
	rect.lx = x;
	rect.ly = y;
	int bRet = 0;
	for (i = 0; i < layout_table_size; i++) {
		icon = mgui_icons[layout_table[i]];
		if (icon->type == MGUI_ICON_TYPE_TEXT) {
			TextBoxSetup(icon->h, NULL, x, height, TA_LEFT, icon->text.color);
			width = TextBoxGetWidth(icon->h) + 10; /* 10 pixel space beteen text icons */

		} else if (icon->type == MGUI_ICON_TYPE_IMAGE) {
			bRet = ImageGetGeometry(icon->h, &rect);
			if(bRet != -1)
			{
				width = GET_SCALED_WIDTH(rect.w, rect.h, height);
				icon->image.rect.h = height;
				icon->image.rect.w = width;
				icon->image.rect.lx = x - width;
				icon->image.rect.ly = y;
				ImageSetup2(icon->h, icon->image.rect);
			}else
			{
				MGUI_DMSG("layout_icons bRet=%d\n", bRet);
				continue;
			}
		} else {
			MGUI_DMSG("unsupported icon type %d in icons layout!\n", icon->type);
			continue;
		}
		x = (dir == DIR_RIGHT_TO_LEFT) ? x - width : x + width;
		rect.h = MAX(rect.h, height);
		rect.w = MAX(rect.w, width);
	}
	return rect;
}

static inline enum hawk_req to_hawk_request(int id)
{
	switch (id) {
	case MGUI_FOTA_BUTTON: return HAWK_FOTA;
	case MGUI_NODATA_ASSERT_BUTTON: return HAWK_PING;
	case MGUI_RESET_BUTTON: return HAWK_RESET;
	default:
		MGUI_DMSG("id mismatch");
	case MGUI_KEEPALIVE_BUTTON: return HAWK_KEEP_ALIVE;
	}
}

static __attribute__((unused)) void button_handler(int event, void *data)
{
	struct mgui_icon *icon = (struct mgui_icon *)data;
	struct mgui_context *ctx;
	struct mgui_event mevent;
	int send_event = 0;

	MASSERT(icon);
	ctx = icon->mgui;
	MASSERT(ctx);

	switch (event) {
	case DIRECTFB_EVT_PRESS:
		MASSERT(!icon->button.status);
		icon->button.status = 1;
		break;
	case DIRECTFB_EVT_RELEASE:
		send_event = 1;
	case DIRECTFB_EVT_RELEASE_OUT_OF_BOUNDS:
		MASSERT(icon->button.status);
		icon->button.status = 0;
		break;
	}

	ButtonSetFromArray(icon->h, icon->button.status);
	ButtonSetup2(icon->h, icon->image.rect);
	//MGUI_DMSG("event=%d, icon=%s, status=%d\n", event, icon->name, icon->button.status);
	//MGUI_DMSG("BEFORE REFRESH\n");
	//mgui_screen_refresh(ctx); /* TODO: change to only refresh button! */
	//MGUI_DMSG("AFTER REFRESH\n");
#if 1
	if (send_event) {
		mevent.id = MGUI_BUTTON_EVENT;
		mevent.data = icon->id;
		write(ctx->pipes_fd[1], &mevent, sizeof(struct mgui_event));
	}
#endif
}

static inline void setup_buttons(struct mgui_context *ctx)
{
	struct mgui_icon *icon;
	OBJECT_RECT rect = {};
	int i, lx, ly, dy;

	MASSERT(ctx);

	lx = background.info.buttons_x;
	ly = background.info.buttons_y;
	dy = background.info.buttons_space;

	for (i = 0; i < mgui_num_buttons; i++) {
		icon = mgui_buttons[i];
		icon->button.click_params.cb = button_handler;
		icon->button.click_params.cb_data = icon;
		ButtonGetGeometry(icon->h, &rect);
		icon->image.rect.lx = lx;
		icon->image.rect.ly = ly;
		icon->image.rect.h = rect.h;
		icon->image.rect.w = rect.w;
		ButtonSetup2(icon->h, icon->image.rect);
		ButtonSetupOnClick(icon->h, &icon->button.click_params);
		ly += rect.h + dy;
	}
}

static inline int init_layout(struct mgui_context *ctx)
{
	OBJECT_RECT rect = {};

	MASSERT(ctx);

	/* background Image */
	//ImageGetGeometry(background.h, &rect);
	//ImageSetup2(background.h, rect);
	
	/* buttons */
	//setup_buttons(ctx);

#ifdef MARVELL_ORIGIN_LCD
	/* touch handler */
	rect.h = ctx->screen.height;
	rect.w = ctx->screen.width;
	rect.lx = rect.ly = 0;
	ctx->dfb_touch = DirectFbRegisterEventHandler(ctx->dfb, &rect,
						      DIRECTFB_EVT_TOUCH,
						      touch_handler, ctx);
	MASSERT(ctx->dfb_touch);
#endif
	layout_icons(ctx, mgui_layout_top_first,
		      ARRAY_SIZE(mgui_layout_top_first),
		      100, 7,
		      background.info.icons_height, DIR_LEFT_TO_RIGHT);
			  
	layout_icons(ctx, mgui_layout_top_second,
		      ARRAY_SIZE(mgui_layout_top_second),
		      105, 48,
		      background.info.icons_height, DIR_LEFT_TO_RIGHT);
	#if 0
	#if defined CONFIG_USER_HKM
	layout_icons(ctx, mgui_layout_top_fourth,
		      ARRAY_SIZE(mgui_layout_top_fourth),
		      78, 65,
		      30, DIR_LEFT_TO_RIGHT);
	#else
	layout_icons(ctx, mgui_layout_top_fourth,
		      ARRAY_SIZE(mgui_layout_top_fourth),
		      75, 50,
              20, DIR_LEFT_TO_RIGHT);
	#endif
	#endif
	layout_icons(ctx, mgui_layout_top_third,
		      ARRAY_SIZE(mgui_layout_top_third),
		      100, 87,
		      background.info.icons_height, DIR_LEFT_TO_RIGHT);	
	return 0;
}
static inline int init_layout_sms(struct mgui_context *ctx)
{
	OBJECT_RECT rect = {};
	MASSERT(ctx);
	layout_icons(ctx, mgui_layout_sms,
		      ARRAY_SIZE(mgui_layout_sms),
		      66, 38,
		      background.info.icons_height, DIR_LEFT_TO_RIGHT);	
	return 0;
}
void mgui_reset_all_icons(struct mgui_context *ctx)
{
	struct mgui_icon *ic;
	int i;

	MASSERT(ctx);

	MGUI_DMSG("reset all icons\n");

	/* reset all icons to default */
	for (i = 0; i < mgui_num_icons; i++) {
		ic = mgui_icons[i];
		if(!ic)
			continue;
		switch (ic->type) {
		case MGUI_ICON_TYPE_IMAGE:
			ic->image.current_key = ic->image.default_key;
			ImageSetFromArray(ic->h, ic->image.current_key);
			break;
		case MGUI_ICON_TYPE_TEXT:
			TextBoxSetText(ic->h, ic->text.default_string);
			TextBoxSetColor(ic->h, ic->text.color);
			break;
		default: /* do nothing */
			break;
		}
	}
}
#if 1
 void set_icons(struct mgui_context *ctx, struct mgui_icon **icons, int index)
{
	struct mgui_icon *ic;
	int i;
	i = index;
	ic = icons[i];
	ic->id = i;
	ic->mgui = ctx;
	if (ic->type == MGUI_ICON_TYPE_IMAGE) {
		ic->h = ImageInit(ctx->dfb);
		ic->image.current_key = ic->image.val_to_key(ic,NULL);
                DBG_MSG("%s %d ic->name=%s ic->image.current_key=%d\n",__FUNCTION__,__LINE__,ic->name,ic->image.current_key );
		ImageSetArray(ic->h, ic->image.arr, ic->image.arr_size);
		ImageSetFromArray(ic->h, ic->image.current_key);
	}

}
#else
 void set_icons(struct mgui_context *ctx, struct mgui_icon **icons, int index,int key)
{
	struct mgui_icon *ic;
	int i;
	MGUI_DMSG("%s L%d*******\n",__FUNCTION__,__LINE__);
	i = index;
	ic = icons[i];
	ic->id = i;
	ic->mgui = ctx;
	if (ic->type == MGUI_ICON_TYPE_IMAGE) {
		MGUI_DMSG("%s L%d*******\n",__FUNCTION__,__LINE__);
		ic->h = ImageInit(ctx->dfb);
		ic->image.current_key = key;
		ImageSetArray(ic->h, ic->image.arr, ic->image.arr_size);
		ImageSetFromArray(ic->h, ic->image.current_key);
		MGUI_DMSG("%s L%d*******\n",__FUNCTION__,__LINE__);
	}

}
#endif
/**
 * updates a given icon using it's val2key function.
 * 
 * @param ctx    mgui context
 * @param i      icon enum
 * @param data   opaque data used by the val2key function
 * 
 * @return 0 if no change was necessary, 1 otherwise (indicates that screen refresh is needed)
 */
#if 1
int mgui_update_icon(struct mgui_context *ctx, enum e_mgui_icons i, void *data)
{
#if 0
	struct mgui_icon *ic;
	const char *text;
	int key;
	MASSERT(ctx);
	if(g_ctx==NULL)
		g_ctx=ctx;
	MGUI_DMSG("mgui_update_icon():ctx->state=%d,i:%d\n", ctx->state,i);
	if(ctx->state == MGUI_STATE_SECOND || ctx->state == MGUI_STATE_THIRD || g_ctx->state == MGUI_STATE_SECOND || g_ctx->state == MGUI_STATE_THIRD)///wpeng don't refresh screen after screen2/3/4
		return 0;
	ic = mgui_icons[i];
	if(ic==NULL)
		return 0;
	switch (ic->type) {
	case MGUI_ICON_TYPE_IMAGE:
		/*key = ic->image.val_to_key(ic);
		MGUI_DMSG("icon %s: current_key %d, new key=%d\n", ic->name, ic->image.current_key, key);
		if (ic->image.current_key != key) {
				ic->image.current_key = key;
				ImageSetFromArray(ic->h, key);
				MGUI_DMSG("update icon %s \n", ic->name);
				return 1;
		
		}*/
		break;
	case MGUI_ICON_TYPE_TEXT:
	/*
		text = ic->text.val_to_string(ic, data);
		MGUI_DMSG("icon %s: current text %s, new text %s\n", ic->name, TextBoxGetText(ic->h), text);
		//if (TextBoxGetText(ic->h) && strcmp(TextBoxGetText(ic->h), text)) {
			
				TextBoxSetText(ic->h, text);
				TextBoxSetColor(ic->h, ic->text.color);
				MGUI_DMSG("update icon %s data=%s color=ARGB=%08x\n", ic->name, text, ic->text.color);
				return 1;
			
		//}
		*/
		break;
	default:
		MGUI_DMSG("unsupported icon type\n");
		break;
	}
	return 0; /* no icon was changed */
#endif
}
#else
int mgui_update_icon(struct mgui_context *ctx, enum e_mgui_icons i, void *data)
{
	struct mgui_icon *ic;
	const char *text;
	int key;

	MASSERT(ctx);
	if(g_ctx==NULL)
		g_ctx=ctx;
	MGUI_DMSG("mgui_update_icon():ctx->state=%d,i:%d\n", ctx->state,i);
	if(ctx->state == MGUI_STATE_SECOND || ctx->state == MGUI_STATE_THIRD || g_ctx->state == MGUI_STATE_SECOND || g_ctx->state == MGUI_STATE_THIRD)///wpeng don't refresh screen after screen2/3/4
		return 0;
		
	ic = mgui_icons[i];
	if(ic==NULL)
		return 0;
	
	switch (ic->type) {
	case MGUI_ICON_TYPE_IMAGE:
		key = ic->image.val_to_key(ic, data);
		MGUI_DMSG("icon %s: current_key %d, new key=%d\n", ic->name, ic->image.current_key, key);
		if (ic->image.current_key != key) {
				ic->image.current_key = key;
				ImageSetFromArray(ic->h, key);
				MGUI_DMSG("update icon %s data=%d\n", ic->name, data);
				return 1;
		
		}
		break;
	case MGUI_ICON_TYPE_TEXT:
		text = ic->text.val_to_string(ic, data);
		MGUI_DMSG("icon %s: current text %s, new text %s\n", ic->name, TextBoxGetText(ic->h), text);
		//if (TextBoxGetText(ic->h) && strcmp(TextBoxGetText(ic->h), text)) {
			
				TextBoxSetText(ic->h, text);
				TextBoxSetColor(ic->h, ic->text.color);
				MGUI_DMSG("update icon %s data=%s color=ARGB=%08x\n", ic->name, text, ic->text.color);
				return 1;
			
		//}
		break;
	default:
		MGUI_DMSG("unsupported icon type\n");
		break;
	}

	return 0; /* no icon was changed */
}
#endif

void pm_set_screen(struct mgui_context *ctx, int on)
{
    // char cmd[64]={'\0'};
	// mril_request_screen(ctx->ril, 0, on);
	// GuiScreenSetPower(ctx->dfb, on);
    // sprintf(cmd,"echo %d > /tmp/ScreenPower",on);
    // system(cmd);
}

#define TOUCH_RUNTIME_PM_PATH "/sys/bus/i2c/drivers/cyttsp-i2c/0-0024/power/control"
#define VERSION_PATH "/etc/mversion"

static inline int pm_set_touch(struct mgui_context *ctx, int on)
{
	int fd;
	int ret = 0;
	const char *mode[] = {"auto", "on"};

	fd = open(TOUCH_RUNTIME_PM_PATH, O_RDWR);
	if (fd < 0) {
		MGUI_DMSG("Can't open touchscreen runtime control (%s)\n", TOUCH_RUNTIME_PM_PATH);
		return -1;
	}

	ret = write(fd, mode[!!on], strlen(mode[!!on]));
	if (ret < 0) {
		MGUI_DMSG("%s runtime suspend failed\n", on ? "disable" : "enable");
		goto out;
	}
	MGUI_DMSG("%s runtime suspend success\n", on ? "disable" : "enable");
out:
	close(fd);
	return ret;
}


/**
 * refresh the screen with new data
 * 
 * Might sleep, do not hold mutex when calling this function!
 * 
 * @param ctx    mgui context
 */
void mgui_screen_refresh(struct mgui_context *ctx)
{
	MASSERT(ctx);

	//if(ctx->state == MGUI_STATE_SECOND || ctx->state == MGUI_STATE_THIRD || ctx->state == MGUI_STATE_FORTH)///wpeng don't refresh screen after screen2/3/4
	//return;
	
	DBG_MSG("ctx->state[%d]\n",ctx->state);
	if (ctx->state != MGUI_STATE_OFF) {
		DBG_MSG("refreshing screen\n");
		GuiRefreshScreenObjects(ctx->dfb);
	}
	
}

static void clock_timer_handler(sigval_t sigval)
{
	struct mgui_context *ctx = (struct mgui_context *)sigval.sival_ptr;

	//mgui_update_icon(ctx, MGUI_CLOCK_ICON, NULL);
	/* TODO: change to only refresh clock! */
	if (ctx->state == MGUI_STATE_FIRST) {
                MGUI_DMSG("ctx->state[%d]\n",ctx->state);
				MGUI_DMSG("update mgui clock\n");
                prepare_status_first();
                pthread_mutex_lock(&ctx->lock);
                mgui_init1(0,ctx,-2);
                pthread_mutex_unlock(&ctx->lock);
	}
}



static void idle_timer_handler(sigval_t sigval)
{
	struct mgui_context *ctx = (struct mgui_context *)sigval.sival_ptr;
	struct mgui_event e = {
		.id = MGUI_IDLE_TIMEOUT_EVENT,
	};

	MGUI_DMSG("idle timeout reached\n");
	write(ctx->pipes_fd[1], &e, sizeof(struct mgui_event));
}

#ifdef ENABLE_MODULES_TIMER
static void modules_timer_handler(sigval_t sigval)
{
	struct mgui_context *ctx = (struct mgui_context *)sigval.sival_ptr;
	struct mgui_event e = {.id = MGUI_MODULES_TIMEOUT_EVENT,};

	MGUI_DMSG("modules timeout reached\n");
	write(ctx->pipes_fd[1], &e, sizeof(struct mgui_event));
}
#endif
static int mgui_timer_start(struct mgui_context *ctx,
			    timer_t timerid,
			    long long freq_nanosecs)
{
	struct itimerspec its;

	its.it_value.tv_sec = freq_nanosecs / 1000000000;
	its.it_value.tv_nsec = freq_nanosecs % 1000000000;
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;

	pthread_mutex_lock(&ctx->lock);
	if (timer_settime(timerid, 0, &its, NULL) == -1) {
		pthread_mutex_unlock(&ctx->lock);
		return -1;
	}
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}
static inline int mgui_timer_stop(struct mgui_context *ctx, timer_t timerid)
{
	return mgui_timer_start(ctx, timerid, 0);
}
static void mgui_timer_all_start(struct mgui_context *ctx)
{
        mgui_timer_start( ctx, ctx->clock_timer, CLOCK_TIMER_FREQ_NANOSECS );
        #ifdef ENABLE_IDLE_TIMER
        mgui_timer_stop(ctx, ctx->idle_timer);
		if( ctx->state == MGUI_STATE_WPS_PROCESSING)
		{
			mgui_timer_start( ctx, ctx->idle_timer, MODULES_TIMER_FREQ_NANOSECS );
		}
		else
		{
			mgui_timer_start( ctx, ctx->idle_timer, IDLE_TIMER_FREQ_NANOSECS );
		}
        #endif
        #ifdef ENABLE_MODULES_TIMER
        mgui_timer_start( ctx, ctx->modules_timer, MODULES_TIMER_FREQ_NANOSECS );
        #endif
}
static void mgui_timer_all_stop(struct mgui_context *ctx)
{
        mgui_timer_stop(ctx, ctx->clock_timer);
        #ifdef ENABLE_IDLE_TIMER
        mgui_timer_stop(ctx, ctx->idle_timer);
        #endif
        #ifdef ENABLE_MODULES_TIMER
        mgui_timer_stop(ctx, ctx->modules_timer);
        #endif

}
static int mgui_timer_init(struct mgui_context *ctx,
			    timer_t *timerid,
			    void (*handler)(sigval_t))
{
	struct sigevent sev;
	/* must clear, otherwise may cause timer_create crash */
	memset(&sev, 0, sizeof(sev));
	MASSERT(ctx);

	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = handler;
	sev.sigev_value.sival_ptr = ctx;
	if (timer_create(CLOCK_REALTIME, &sev, timerid) == -1)
		return -1;
	return 0;
}
static struct uci_package * config_try_load(struct uci_context *ctx, char *path)
{
	char *file = basename(path);
	char *dir = dirname(path);
	char *err;
	struct uci_package *pkg;

	uci_set_confdir(ctx, dir);
	MGUI_DMSG("attempting to load %s/%s\n", dir, file);

	if (uci_load(ctx, file, &pkg)) {
		uci_get_errorstr(ctx, &err, file);
		MGUI_DMSG("unable to load configuration (%s)\n", err);

		free(err);
		return NULL;
	}

	return pkg;
}

enum {
	MOUNT_UUID,
	MOUNT_LABEL,
	MOUNT_ENABLE,
	MOUNT_TARGET,
	MOUNT_DEVICE,
	MOUNT_OPTIONS,
	__MOUNT_MAX
};

static const struct blobmsg_policy mount_policy[__MOUNT_MAX] = {
	[MOUNT_UUID] = { .name = "uuid", .type = BLOBMSG_TYPE_STRING },
	[MOUNT_LABEL] = { .name = "label", .type = BLOBMSG_TYPE_STRING },
	[MOUNT_DEVICE] = { .name = "device", .type = BLOBMSG_TYPE_STRING },
	[MOUNT_TARGET] = { .name = "target", .type = BLOBMSG_TYPE_STRING },
	[MOUNT_OPTIONS] = { .name = "options", .type = BLOBMSG_TYPE_STRING },
	[MOUNT_ENABLE] = { .name = "enabled", .type = BLOBMSG_TYPE_INT32 },
};

static const struct uci_blob_param_list mount_attr_list = {
	.n_params = __MOUNT_MAX,
	.params = mount_policy,
};

static struct blob_buf b;

static int mgui_detect_sdcard(char *cfg)
{
	struct uci_context *ctx = uci_alloc_context();
	struct uci_package *pkg = NULL;
	struct uci_element *e;
	char path[64];

        if (!ctx)return -1;
	if (cfg) {
		snprintf(path, sizeof(path), "%s/upper/etc/config/fstab", cfg);
		pkg = config_try_load(ctx, path);

		if (!pkg) {
			snprintf(path, sizeof(path), "%s/etc/config/fstab", cfg);
			pkg = config_try_load(ctx, path);
		}
	}

	if (!pkg) {
		snprintf(path, sizeof(path), "/etc/config/fstab");
		pkg = config_try_load(ctx, path);
	}

	if (!pkg) {
		MGUI_DMSG("no usable configuration\n");
		return -1;
	}

	uci_foreach_element(&pkg->sections, e) {
		struct uci_section *s = uci_to_section(e);

		if (!strcmp(s->type, "mount")) {
			struct blob_attr *tb[__MOUNT_MAX] = { 0 };

			blob_buf_init(&b, 0);
			uci_to_blob(&b, s, &mount_attr_list);
			blobmsg_parse(mount_policy, __MOUNT_MAX, tb, blob_data(b.head), blob_len(b.head));
			if (!tb[MOUNT_LABEL] && !tb[MOUNT_UUID] && !tb[MOUNT_DEVICE])
				return -1;

			if (tb[MOUNT_ENABLE] && !blobmsg_get_u32(tb[MOUNT_ENABLE]))
				return -1;
			return 1;
		}
	}
	return -1;
}

static void deinit_icons(struct mgui_context *ctx, struct mgui_icon **icons, size_t num_icons)
{
	int i;

	for (i = 0; i < num_icons; i++) {
		if(!icons || !icons[i] || !icons[i]->h)
			continue;
		if (icons[i]->type == MGUI_ICON_TYPE_IMAGE)
			ImageDeinit(icons[i]->h);
		else if (icons[i]->type == MGUI_ICON_TYPE_TEXT)
			TextBoxDeinit(icons[i]->h);
		else if (icons[i]->type == MGUI_ICON_TYPE_BUTTON)
			ButtonDeinit(icons[i]->h);
	}
}
static void deinit_icons_index(struct mgui_context *ctx, struct mgui_icon **icons, int index)
{
	struct mgui_icon *ic;
	ic = icons[index];
	if (ic->type == MGUI_ICON_TYPE_IMAGE) {
		ImageDeinit(ic->h);
	}
	
}

#define BAT_CAPACITY_FILE 	"/sys/class/power_supply/battery/capacity"
#define BAT_PRESENT_FILE 	"/sys/class/power_supply/battery/present"
#define BAT_STATUS_FILE 	"/sys/class/power_supply/battery/status"
#define WAN_STATUS_FILE 	"/tmp/wan_status"
#define VERSION_INFO_FILE 	"/sbin/Version"

static int read_file(char *file_name, char *buf)
{	
	int fd = -1;
	int ret;
	char str_temp[20];

	memset(str_temp, 0, sizeof(str_temp));
	fd = open(file_name, O_RDONLY);
	if (fd < 0){
		MGUI_DMSG("open %s failed\n", file_name);
		return -1;
	}
	ret = read(fd, str_temp, sizeof(str_temp));
	if(ret < 0) {
		MGUI_DMSG("[%s] file read error\n", __FUNCTION__);
		close(fd);
		return -1;
	}
	memcpy(buf, str_temp, sizeof(str_temp));
	close(fd);

	return 0;
}


int prepare_operator_info( void )
{
	int	op_status	= 0;
	char	buf[256], tmpBuf[64] = "", outBuf[64] = "", Plmn[64] = "";
	FILE	*fp = fopen( WAN_STATUS_FILE, "r" );
	if ( fp != NULL )
	{
		while ( fgets( buf, sizeof(buf), fp ) != NULL )
		{
			memset( outBuf, 0, sizeof(outBuf) );
			memset(g_operator,0,sizeof(g_operator));
			sscanf( buf, "%[^:]:%[^;];", tmpBuf, outBuf );
		//	DBG_MSG( "%s %d tmpBuf[%s] outBuf[%s]\n", __FUNCTION__, __LINE__, tmpBuf, outBuf );
			if ( strstr( tmpBuf, "USIM Status" ) != NULL )
			{
				
				if ( strlen(outBuf) > 0 )
				{
					if ( strcmp( outBuf, "USIM Ready" ) == 0 )
					{
						usimready = 1;
					}
					else if ( strcmp( outBuf, "USIM SIMLOCK" ) == 0 )
					{
						strcpy( g_operator, "NoService" );
						break;
					
					}else if ( strcmp( outBuf, "Not Available" ) == 0 )
					{
						strcpy( g_operator, "No SIM" );
						break;
					}
					else if ( strcmp( outBuf, "MSISDN locked" ) == 0 )
					{
						strcpy( g_operator, "NoService" );
						break;
					}else{
						strcpy( g_operator, outBuf );
						break;
					}
				}else{
					strcpy( g_operator, "Detecting" );
					break;
				}
			}
			else if ( strstr( tmpBuf, "Plmn" ) != NULL )
			{
				if ( outBuf != NULL && strlen(outBuf)>=5)
				{
					strcpy( Plmn, outBuf );
				}
			}
			else if ( strstr( tmpBuf, "Operator Name" ) != NULL )
			{
				
				if ( outBuf != NULL && strlen(outBuf) > 1)
				{
						
					strcpy( g_operator, outBuf );
				}else  {
					
					if ( Plmn != NULL && strlen( Plmn ) >= 5 )
					{
						strcpy( g_operator, Plmn );
					}else  {
						
					}
				}
				break;
			}
		}
		fclose( fp );
	}else{
		strcpy( g_operator, "Detecting" );
	}
	MGUI_DMSG( "%s L%d***g_operator=%s****\n", __FUNCTION__, __LINE__, g_operator );
	return 0;
}
int prepare_net_tech_info(void)
{
	char buf[256], tmpBuf[64] = "", outBuf[64] = "", Plmn[64] = "";
	FILE *fp = fopen(WAN_STATUS_FILE, "r");
	if (fp != NULL)
    {	
		 while (fgets(buf, sizeof(buf), fp) != NULL) 
        {
			// DBG_MSG( "%s %d tmpBuf[%s] outBuf[%s]\n", __FUNCTION__, __LINE__, tmpBuf, outBuf);
			memset(outBuf,0,sizeof(outBuf));
            sscanf(buf, "%[^:]:%[^;];", tmpBuf,outBuf);
			
			if(strstr(tmpBuf,"Modem Type") != NULL) 
			{
				if((outBuf != NULL)&&(usimready ==1))
				{
					memset(g_net_tech,0,sizeof(g_net_tech)); 
					if(strstr(outBuf,"LTE") != NULL)
					{
						strcpy(g_net_tech,"4G");
					}
					else
					{
						strcpy(g_net_tech,outBuf);
					} 
				}
				else
				{
					//strcpy(g_net_tech,"4G");///set 4G by default
				}
				break;
			}
		}
		 fclose(fp);
	}
	DBG_MSG("%s L%d******g_net_tech=%s*\n",__FUNCTION__,__LINE__,g_net_tech);	
	return 0;
}
int prepare_wifi_encryption_info(void)
{
	FILE *fp=NULL;
	char buf[64];
	fp = popen("uci get wireless.AP1_2G_if.encryption 2>/dev/null", "r");
	if (fp != NULL)
	{
		while (fgets(buf, sizeof(buf), fp) != NULL) 
		{
			memset(g_encryption,0,sizeof(g_encryption)); 
			strcpy(g_encryption,buf);
			if (g_encryption && strstr(g_encryption, "psk"))
			{
				strcpy(g_encryption,"WPA");
			}
			else
			{
				strcpy(g_encryption,"nopass");
			}
			break;
		}
		pclose(fp);
	}
	else
	{
		strcpy(g_encryption,"N/A");
	}
	MGUI_DMSG("%s L%d***g_encryption=%s****\n",__FUNCTION__,__LINE__,g_encryption);
	return 0;
}
int prepare_wifi_ssid_info(void)
{
	FILE *fp=NULL;
	char buf[64],tmpbuf[64];
	fp = popen("uci get wireless.AP1_2G_if.ssid 2>/dev/null", "r");
	if (fp != NULL)
	{
		if(fgets(buf, sizeof(buf), fp) != NULL) 
		{
			memset(g_ssid,0,sizeof(g_ssid)); 
			strcpy(tmpbuf,buf);
			strncpy(g_ssid,tmpbuf,strlen(tmpbuf)-1);
		}
		pclose(fp);
	}
	else
	{
		strcpy(g_ssid,"N/A");
	}
	MGUI_DMSG("%s L%d***g_ssid=%s****\n",__FUNCTION__,__LINE__,g_ssid);
	return 0;
}
// int prepare_wifi_password_info(void)
// {
// 	FILE *fp=NULL;
// 	char buf[128],tmpBuf[128]="",outBuf[128]="",tmpbuf[128]="";
// 	fp = popen("cat /proc/wlan1/mib_auth |grep dot11PassPhrase: 2>/dev/null", "r");
    
// 	if (fp != NULL)
// 	{
// 		if(fgets(buf, sizeof(buf), fp) != NULL && strlen(buf) > 28) 
// 		{
// 			DBG_MSG("%s %d buf=[%s] strlen(%d)\n",__FUNCTION__,__LINE__,buf,strlen(buf));
		
// 			memset(g_key,0,sizeof(g_key));
// 			sscanf(buf, "%[^:]: %[^;];", tmpBuf,outBuf);
// 			if(strstr(tmpBuf,"dot11PassPhrase") != NULL)
// 			{
// 				if(strlen(outBuf) > 1 && strlen(outBuf) <= 64)
// 				{
// 					outBuf[strlen(outBuf)-1] ='\0'; //remove '\n'
// 					memset(g_key,0,sizeof(g_key)); //need clean, if not lcd will show overflow strings
// 					strcpy(g_key,outBuf);
// 				}
// 				//else 
// 					//strcpy(g_key,"N/A");
// 			}
// 		}
// 		pclose(fp);
// 		// if(strlen(g_key) == 0 )
// 		// {
//         //                 fp = popen("atel_tool decrypt_wifipassword_out", "r");
//         //                            if (fp != NULL)
//         //                 {
//         //                         memset(buf,0,sizeof(buf));
//         //                         if(fgets(buf, sizeof(buf), fp) != NULL)
//         //                         {
//         //                                 DBG_MSG("%s %d buf[%s]\n",__FUNCTION__,__LINE__,buf);
//         //                                 if(strlen(buf) > 1 && strlen(buf) <= 64)
//         //                                 {
//         //                                         buf[strlen(buf)-1] ='\0'; //remove '\n'
//         //                                         DBG_MSG("%s %d buf[%s]\n",__FUNCTION__,__LINE__,buf);
//         //                                         memset(g_key,0,sizeof(g_key)); //need clean, if not lcd will show overflow strings
//         //                                         strcpy(g_key,buf);
//         //                                         DBG_MSG("%s %d g_key[%s]\n",__FUNCTION__,__LINE__,g_key);
//         //                                 }

//         //                         }
//         //                         pclose(fp);
//         //                 }
//         //         }
                
// 	}
// 	else
// 	{
// 		strcpy(g_key,"N/A");
// 	}
// 	DBG_MSG("%s %d\n",__FUNCTION__,__LINE__);
// 	return 0;
// }
int prepare_wifi_client_num(void)
{
    char line[256];
    char *str1; 
    char buf[16];
    FILE *fp = fopen("/proc/wlan1/sta_info", "r");
    if(fp != NULL)
    {
        while (fgets(line, 256, fp)) {
            if (strstr(line, "STA info table") != NULL)
            {
                str1 = strstr(line,"active:");
                strcpy(buf,str1+strlen("active: "));
                if(buf[2] == ')')
                buf[2]='\0';
                if(buf[1] == ')')
                buf[1]='\0';
                strcpy(g_client_n,buf);
                break;
            }

        }
        fclose(fp);
    }
	MGUI_DMSG("%s L%d***g_client_n=%s****\n",__FUNCTION__,__LINE__,g_client_n);
	return 0;
}
int prepare_version_info(void)
{
	int version_status = 0;
	char buf[64];
	char buf1[64];
	if(g_swVersion != NULL && strlen(g_swVersion) > 10)
	{
		/*there is no need to get again */
	}else
	{
		FILE *fp = fopen(VERSION_INFO_FILE, "r");
		if (fp != NULL)
		{
			while (fgets(buf, sizeof(buf), fp) != NULL) 
			{
                memset(g_swVersion,0,sizeof(g_swVersion));
				strcpy(buf1,buf);
				break;
			}
			fclose(fp);
            strncpy(g_swVersion, buf1, strlen(buf1)-5);
            char *str_src = strchr(g_swVersion,'.');
            if(str_src != NULL)
            {
                str_src++;
                char *str_src2 = strchr(str_src,'.');
                if(str_src2 != NULL)
                {
                    str_src2++;
                    str_src = strchr(str_src2,'.');
                    if(str_src != NULL)
                    {
                        g_swVersion[strlen(g_swVersion)-strlen(str_src)]='\0';
                    }
                }
            }
            MGUI_DMSG("%s %d str_src[%s] g_swVersion[%s]\n",__FUNCTION__,__LINE__,str_src,g_swVersion);
        }
		else
		{
			strcpy(g_swVersion,"N/A");
		}
	}

	return 0;
}
static int prepare_signal_key(void)
{
	int sig_status = 0;	
	int ret;
	char buf[256],tmpBuf[64]="",outBuf[64]="";
	char signalLevel[64] = "";
	FILE *fp = fopen(WAN_STATUS_FILE, "r");
	if (fp != NULL)
    {	
        while (fgets(buf, sizeof(buf), fp) != NULL) 
        {
            memset(outBuf,0,sizeof(outBuf));
            sscanf(buf, "%[^:]:%[^;];", tmpBuf,outBuf);	
            if(strstr(tmpBuf,"Signal Level") != NULL) {
                if(outBuf != NULL)
                {
                    strcpy(signalLevel,outBuf);
                    if(strcmp(signalLevel, "Bad Signal")==0)
                        sig_status = 1;
                    else if(strcmp(signalLevel, "Low Signal")==0)
                        sig_status = 2;
                    else if(strcmp(signalLevel, "Middle Signal")==0)
                        sig_status = 3;
                    else if(strcmp(signalLevel, "Good Signal")==0)
                        sig_status = 4;
                    else if(strcmp(signalLevel, "Excellent Signal")==0)
                        sig_status = 5;
                }
				else 
				    sig_status = -1;
                    break;
            }
        }
        fclose(fp);
    }	
	return  sig_status;
}

static int prepare_charger_key(void)
{
	int preset, capacity;	
	int ret;
	char buf[20];
	int charging = 0;

	memset(buf, 0, sizeof(buf));
	ret = read_file(BAT_PRESENT_FILE, buf);
	if(ret < 0)
		return -1;
	preset = atoi(buf);
	if(preset == 0)
		return STATUS_UNKNOWN;

	memset(buf, 0, sizeof(buf));
	ret = read_file(BAT_STATUS_FILE, buf);
	if(ret < 0)
		return -1;
	if(strcmp(buf, "Charging") == 0)
		charging = 1;

	memset(buf, 0, sizeof(buf));
	ret = read_file(BAT_CAPACITY_FILE, buf);
	if(ret < 0)
		return -1;
	capacity = atoi(buf);

	//memset(cmd_buf, 0, sizeof(cmd_buf));
	//snprintf(cmd_buf, sizeof(cmd_buf), "echo \"preset = %d, charging = %d, cap = %d\" > /tmp/xiehj.log",
	//	preset, charging, capacity);
	//system(cmd_buf);
	
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


void prepare_status_first(void)
{
        prepare_operator_info();
        prepare_wifi_client_num();
        prepare_net_tech_info();
}

void prepare_status_second(void)
{
        prepare_wifi_ssid_info();
        // prepare_wifi_password_info();
}

void prepare_status_third(void)
{
        prepare_wifi_encryption_info();
}
void prepare_status_forth(void)
{
        prepare_version_info();
}
#if defined CONFIG_USE_MGUI_READ_SMS	
static void prepare_unread_body_data(struct mgui_context *ctx)
{
        DBG_MSG("%s %d\n",__FUNCTION__,__LINE__);
        mgui_sms_unread_body_data_get(ctx->ubus);
}
static void prepare_status_sms_number(void)
{
	nowTime = read_poweron_time();
	interval = nowTime - sms_lastTime;
	DBG_MSG("%s L%d******* start interval[%d]\n",__FUNCTION__,__LINE__,interval);
	if(nowTime > 3000 && interval > 1000)
	{
        mgui_sms_unread_number();
		sms_lastTime = read_poweron_time();
	}
	if(strlen(g_sms_unread) == 0)
	{
		strcpy( g_sms_unread, "0" );
		strcpy( g_sms_tip, "NO SMS" );
	}
	DBG_MSG("%s L%d*******end g_sms_unread=[%s] sms_lastTime[%u]\n",__FUNCTION__,__LINE__,g_sms_unread,sms_lastTime);
 
}
void mgui_init_sms_first(struct mgui_context *ctx)
{
    prepare_unread_body_data(ctx);
    ctx->dfb = GuiInit2();
    lvgl_textbox_simpletext(7,-42);
    lvgl_textbox_set_shorttext(g_sms_transfer,9,-24);
    lvgl_textbox_set_shorttext(g_sms_time,9,-8);
    lvgl_textbox_set_shorttext(g_sms_date,9,8);
    ctx->state = MGUI_STATE_SMS_FIRST;
    mgui_screen_refresh(ctx);
}
void prepare_sms_second(void)
{
	char buf[512],buf1[128],buf2[128];
	memset(buf, 0, sizeof(buf));
	memset(buf1, 0, sizeof(buf1));
	memset(buf2, 0, sizeof(buf2));
	DBG_MSG("g_sms_info len=%d\n",strlen(g_sms_info));
	if(strlen(g_sms_info) > 98)
	{
		sprintf(buf, "%s",g_sms_info);
		strncpy(buf1, buf, 98);
		lvgl_textbox_set_sms(buf1,-15);
		sms_long = 1;
	}
	else
	{
		lvgl_textbox_set_sms(g_sms_info,-15);
		sms_long = 0;
	}
}
void prepare_sms_third(void)
{
	char buf[512],buf1[128],buf2[128];
	memset(buf, 0, sizeof(buf));
	memset(buf1, 0, sizeof(buf1));
	memset(buf2, 0, sizeof(buf2));
	sprintf(buf, "%s",g_sms_info);
	strncpy(buf1, buf, 98);
	strncpy(buf2, buf+98, strlen(buf)-98);
	lvgl_textbox_set_sms(buf2,-15);
}
void prepare_sms_end(void)
{
	lvgl_textbox_simpletext(8,-30);
	lvgl_textbox_simpletext(9,4);
}
#endif
static void init_icons(struct mgui_context *ctx, struct mgui_icon **icons, size_t num_icons, int text_height)
{
	struct mgui_icon *ic;
	int i;
	for (i = 0; i < num_icons; i++) {
		ic = icons[i];
		if(!ic)
			continue;
		ic->id = i;
		ic->mgui = ctx;
		if (ic->type == MGUI_ICON_TYPE_IMAGE) {
			/*if (strcmp(ic->name, "bluetooth") == 0)
				continue;*/
			ic->h = ImageInit(ctx->dfb);
			if(strcmp(ic->name, "battery"))
				ic->image.current_key = ic->image.val_to_key(ic, NULL);
			else{
				ic->image.current_key = prepare_charger_key();
			}
			
			//ic->image.current_key = ic->image.val_to_key(ic, NULL);
			ImageSetArray(ic->h, ic->image.arr, ic->image.arr_size);
			ImageSetFromArray(ic->h, ic->image.current_key);
		} else if (ic->type == MGUI_ICON_TYPE_TEXT) {
			ic->h = TextBoxInit(ctx->dfb, text_height, -1,
					    ic->text.font_path);
			TextBoxSetText(ic->h, ic->text.val_to_string(ic, NULL));
			TextBoxSetColor(ic->h, ic->text.color);
		} else if (ic->type == MGUI_ICON_TYPE_BUTTON) {
			ic->h = ButtonInit(ctx->dfb);
			ic->image.current_key = ic->image.val_to_key(ic, NULL);
			ButtonSetArray(ic->h, ic->image.arr, ic->image.arr_size);
			ButtonSetFromArray(ic->h, ic->image.current_key);
		}
	}
}

static void init_icons_index(struct mgui_context *ctx, struct mgui_icon **icons, int index, int text_height)
{
	struct mgui_icon *ic;
	int i;
	for (i = 0; i < mgui_num_icons; i++) {
		ic = icons[i];
		if(!ic)
			continue;
		if(ic->id != index)
			continue;
		ic->id = i;
		ic->mgui = ctx;
		if (ic->type == MGUI_ICON_TYPE_IMAGE) {
			/*if (strcmp(ic->name, "bluetooth") == 0)
				continue;*/
			ic->h = ImageInit(ctx->dfb);
			if(strcmp(ic->name, "battery"))
				ic->image.current_key = ic->image.val_to_key(ic, NULL);
			else{
				ic->image.current_key = prepare_charger_key();
			}
			
			//ic->image.current_key = ic->image.val_to_key(ic, NULL);
			ImageSetArray(ic->h, ic->image.arr, ic->image.arr_size);
			ImageSetFromArray(ic->h, ic->image.current_key);
		} else if (ic->type == MGUI_ICON_TYPE_TEXT) {
			ic->h = TextBoxInit(ctx->dfb, text_height, -1,
					    ic->text.font_path);
			TextBoxSetText(ic->h, ic->text.val_to_string(ic, NULL));
			TextBoxSetColor(ic->h, ic->text.color);
		} else if (ic->type == MGUI_ICON_TYPE_BUTTON) {
			ic->h = ButtonInit(ctx->dfb);
			ic->image.current_key = ic->image.val_to_key(ic, NULL);
			ButtonSetArray(ic->h, ic->image.arr, ic->image.arr_size);
			ButtonSetFromArray(ic->h, ic->image.current_key);
		}
	}
}
/* use F_DL key as qrcode key */
#define QRCODE_KEY_DEV "/dev/input/event1"
static void *mgui_qrcode_key_detect(void *context)
{
	int fd, rd, i;
	struct input_event ev[64];
	struct mgui_context *ctx = (struct mgui_context *)context;
	fd = open(QRCODE_KEY_DEV, O_RDONLY);
	if (fd < 0) {
		MGUI_DMSG("open %s error!!!", QRCODE_KEY_DEV);
		pthread_exit(NULL);
	}

	while (1) {
		rd = read(fd, ev, sizeof(struct input_event) * 64);
		for (i = 0; i < rd / sizeof(struct input_event); i++) {
			if (ev[i].type == EV_KEY && ev[i].code == KEY_MENU /* 139 */) {
				MGUI_DMSG("qrcode key %s\n", ev[i].value ? "pressed" : "released");
				if (ev[i].value) {
					/* pressed */
					char str[128];
					char *encryption = ctx->wifi->status.encryption;
					if (encryption && strstr(encryption, "psk"))
						encryption = "WPA";
					memset(str, 0, sizeof(str));
					sprintf(str, "WIFI:T:%s;S:%s;P:%s;H;",
						encryption,
						ctx->wifi->status.ssid,
						ctx->wifi->status.key);
						lvgl_show_qrcode(str);
				}
			}
		}
	}

	pthread_exit(NULL);
}

static void mgui_gpio_key_init(void *ctx)
{
	pthread_t id;
	pthread_create(&id, NULL, (void *)mgui_qrcode_key_detect, ctx);
}
#if 1
struct mgui_context *mgui_init_main(int argc, char *argv[])
{
	struct mgui_context *ctx;
	int tmp;
	if(access(FB_DEV, F_OK) != 0) {
		MGUI_DMSG("no %s, quit\n", FB_DEV);
		return NULL;
	}
	ctx = malloc(sizeof(*ctx));
	if (!ctx)
		return NULL;

	memset(ctx, 0, sizeof(*ctx));

	ctx->state = MGUI_STATE_OFF;
	
	g_ctx=ctx;
	/* init pipe */
	if (pipe(ctx->pipes_fd)) {
		MGUI_DMSG("pipe failed with error %s\n",
			  strerror(errno));
		goto out_pipe;
	}

	if (pthread_mutex_init(&ctx->lock, NULL) != 0) {
		MGUI_DMSG("mutex init failed\n");
		goto out_mutex;
	}

	if (mgui_timer_init(ctx, &ctx->clock_timer, clock_timer_handler) != 0) {
		MGUI_DMSG("timer init failed\n");
		goto out_clock_timer;
	}
#ifdef ENABLE_IDLE_TIMER
	if (mgui_timer_init(ctx, &ctx->idle_timer, idle_timer_handler) != 0) {
		MGUI_DMSG("timer init failed\n");
		goto out_idle_timer;
	}
#endif
	#ifdef ENABLE_MODULES_TIMER
	if (mgui_timer_init(ctx, &ctx->modules_timer, modules_timer_handler) != 0) {
		MGUI_DMSG("timer init failed\n");
		goto out_modules_timer;
	}
	#endif

        /* init icons and buttons */
	//init_icons(ctx, mgui_icons, mgui_num_icons, background.info.icons_height-3);
#ifdef MARVELL_ORIGIN_LCD
	init_icons(ctx, mgui_buttons, mgui_num_buttons, 0);
#endif
	        /* init gui interface */
        
	/* detect SD card
	tmp = mgui_detect_sdcard(NULL);
	mgui_update_icon(ctx, MGUI_SDCARD_ICON, (void *)tmp);*/
#ifdef CONFIG_USER_MGUI_WPS
	ctx->wpskey = mgui_wpskey_init(ctx);
#endif
#if defined CONFIG_USER_HKM 
	mgui_chargerinfo_init(ctx);
#endif
#if !defined(CONFIG_LCD_ST7789V)
	printf("%s:%d.,will run mgui_onkey_init\n",__FUNCTION__,__LINE__);
	ctx->onkey = mgui_onkey_init(ctx);
	//if (!ctx->onkey)
	//	goto out_onkey_init;
#if defined	CONFIG_USE_MGUI_READ_SMS
	ctx->smskey = mgui_smskey_init(ctx);
#endif

    ctx->alert = mgui_alert_onkey_init(ctx);
    //if (!ctx->alert)
    //    goto out_onkey_init;

	ctx->ubus = mgui_ubus_init();
	//if (!ctx->ubus) 
	//	goto out_ubus_init;

#ifdef ENABLE_UBUS_CHARGER
	ctx->charger = mgui_charger_init(ctx);
	if (!ctx->charger)
		MGUI_DMSG("charger service not ready\n");
#endif

	ctx->ril = mgui_ril_init(ctx);
	if (!ctx->ril)
		MGUI_DMSG("ril service not ready\n");
#endif
#ifdef ENABLE_UBUS_WIFI
	ctx->wifi = mgui_wifi_init(ctx);
	if (!ctx->wifi)
		MGUI_DMSG("wifi service not ready\n");
#endif
#ifdef MARVELL_ORIGIN_LCD
	ctx->hawk = mgui_hawk_init(ctx);
	if (!ctx->hawk)
		MGUI_DMSG("hawk service not ready\n");
#endif

#ifdef ENABLE_UBUS_VERSION
	ctx->ver = mgui_version_init(ctx);
	if (!ctx->ver)
		MGUI_DMSG("version service not ready\n");
#endif


        prepare_status_first();
        /* this is boot on ,it need get the second after lcd link ,after it will hold the data when ubus run*/
        prepare_status_second();

	//mgui_gpio_key_init(ctx);
	pthread_mutex_lock(&ctx->lock);
	ctx->dfb = GuiInit();
	if (!ctx->dfb) {
		MGUI_DMSG("gui init failed\n");
	}
	ctx->state = MGUI_STATE_FIRST;
	//GuiGetScreenDim(ctx->dfb, &ctx->screen);
	/* refresh the screen for the first time */
	mgui_screen_refresh(ctx);
	//background.h = ImageInit(ctx->dfb);
    mgui_init1(1,ctx,1);
	//mgui_screen_refresh(ctx);
    /*end*/
	pthread_mutex_unlock(&ctx->lock);
	dspover = MGUI_STATE_FIRST;
	lastTime = read_poweron_time();

	return ctx;

out_ubus_init:
	mgui_onkey_exit(ctx->onkey);
out_onkey_init:
	deinit_icons(ctx, mgui_icons, mgui_num_icons);
	deinit_icons(ctx, mgui_buttons, mgui_num_buttons);
#ifdef MARVELL_ORIGIN_LCD
	DirectFbUnregisterEventHandler(ctx->dfb, ctx->dfb_touch);
#endif
out_bg_image_init:
	ImageDeinit(background.h);
out_gui_init:
	GuiDeinit(ctx->dfb);
#ifdef ENABLE_MODULES_TIMER
out_modules_timer:
	timer_delete(ctx->modules_timer);
#endif
#ifdef ENABLE_IDLE_TIMER
out_idle_timer:
	timer_delete(ctx->idle_timer);
#endif
out_clock_timer:
	timer_delete(ctx->clock_timer);
out_mutex:
	pthread_mutex_destroy(&ctx->lock);
out_pipe:
	close(ctx->pipes_fd[0]);
	close(ctx->pipes_fd[1]);
	free(ctx);
	ctx=NULL;
	return NULL;
}
#else 
#endif
static void mgui_init_strings_only(struct mgui_context *ctx)
{
	ctx->dfb = GuiInit2();//lcd blank
	if(strcmp(g_FOTA,"1") == 0){
		strcpy(g_FOTA_text,"     Software upgrade in\n     progress. Please DO\n NOT turn-off the device!");
		lvgl_textbox_set_longtext(g_FOTA_text,4,-20);
		strcpy(g_FOTA,"0");
		wps_lock=1;
	}else if(strcmp(g_Reset,"1") == 0){
		strcpy(g_Reset_text,"              Resetting to\n           factory defaults\n");
		lvgl_textbox_set_longtext(g_Reset_text,4,-15);
		strcpy(g_Reset,"0");
		wps_lock=1;
	}
	return ctx;
}
int mgui_exit(struct mgui_context *ctx)
{
	MASSERT(ctx);

#ifdef MARVELL_ORIGIN_LCD
	if (ctx->hawk != NULL)
		mgui_hawk_exit(ctx->hawk);
#endif
    #ifdef ENABLE_UBUS_WIFI
	if (ctx->wifi != NULL)
		mgui_wifi_exit(ctx->wifi);
        #endif
	if (ctx->ril != NULL)
		mgui_ril_exit(ctx->ril);
		#ifdef ENABLE_UBUS_CHARGER
	if (ctx->charger != NULL)
		mgui_charger_exit(ctx->charger);
		#endif
	if (ctx->ubus != NULL)
		mgui_ubus_exit(ctx->ubus);
	if (ctx->onkey != NULL)
		mgui_onkey_exit(ctx->onkey);
	#ifdef ENABLE_UBUS_VERSION
	if (ctx->ver != NULL)
		mgui_version_exit(ctx->ver);
	#endif

	MGUI_DMSG("mgui_init deinit_icons");
	deinit_icons(ctx, mgui_icons, mgui_num_icons);
#ifdef MARVELL_ORIGIN_LCD
	deinit_icons(ctx, mgui_buttons, mgui_num_buttons);
	DirectFbUnregisterEventHandler(ctx->dfb, ctx->dfb_touch);
#endif
	ImageDeinit(background.h);
	GuiDeinit(ctx->dfb);
#ifdef ENABLE_IDLE_TIMER
	timer_delete(ctx->idle_timer);
#endif
	timer_delete(ctx->clock_timer);
	#ifdef ENABLE_MODULES_TIMER
	timer_delete(ctx->modules_timer);
	#endif
	pthread_mutex_destroy(&ctx->lock);

	close(ctx->pipes_fd[0]);
	close(ctx->pipes_fd[1]);	
	free(ctx);
	return 0;
}

static void mgui_init1(int init,struct mgui_context *ctx, int poweron)
{
    if(init == 1)
    {
        ctx->dfb = GuiInit1();//lcd blank
    }
    ctx->state = MGUI_STATE_FIRST;
    background.h = ImageInit( ctx->dfb );
    GuiGetScreenDim( ctx->dfb, &ctx->screen );
    //ImageSetFromPath( background.h, background.path );
	Guiclearscreen();
    set_icons( ctx, mgui_icons, MGUI_BATTERY_ICON );
    set_icons( ctx, mgui_icons, MGUI_WIFI_ICON );
    set_icons( ctx, mgui_icons, MGUI_CELLULAR_ICON );
    init_layout( ctx );
    lvgl_textbox_set_shorttext( g_client_n, 4, -48 );
    lvgl_textbox_set_shorttext( g_operator, 5, 60 );
    lvgl_textbox_set_shorttext( g_net_tech, 6, -13 );
    if(poweron == 1)
    {
        pm_set_screen( ctx, 1 );
    }
    else if(poweron == 0)
    {
        pm_set_screen( ctx, 0 );
    }
    else { /*keep*/ }

    mgui_screen_refresh( ctx );
    if(poweron != -2)
    {
        dspover = MGUI_STATE_FIRST;
    }
    MGUI_DMSG("%s %d OK\n",__FUNCTION__,__LINE__);
    return ctx;

}

static void mgui_init2(struct mgui_context *ctx)
{
    deinit_icons_index(ctx, mgui_icons,MGUI_BATTERY_ICON);
    deinit_icons_index(ctx, mgui_icons,MGUI_WIFI_ICON);
    deinit_icons_index(ctx, mgui_icons,MGUI_CELLULAR_ICON);
    ctx->state = MGUI_STATE_SECOND;
	ctx->dfb = GuiInit2();//lcd blank
	//background.h = ImageInit(ctx->dfb);
	GuiGetScreenDim( ctx->dfb, &ctx->screen );
	//ImageSetFromPath(background.h, background.path);
	//mgui_screen_refresh(ctx);

	lvgl_textbox_set_longtext(g_ssid,1,-34);
	lvgl_textbox_set_longtext(g_key,2,5);
    mgui_screen_refresh( ctx );
    dspover = MGUI_STATE_SECOND;
    MGUI_DMSG("%s %d OK\n",__FUNCTION__,__LINE__);

	return ctx;
}
static void mgui_init3(struct mgui_context *ctx)
{
    ctx->state = MGUI_STATE_THIRD;
	ctx->dfb = GuiInit3();//lcd blank
	//ImageSetFromPath(background.h, background.path);
	//mgui_screen_refresh(ctx);
	#if defined CONFIG_USER_HKM 
	char str[128];
	lvgl_textbox_simpletext(5,-20);
	lvgl_textbox_simpletext(6,-6);
	memset(str, 0, sizeof(str));
	sprintf(str, "WIFI:T:%s;S:%s;P:%s;H:;",g_encryption,g_ssid,g_key);
	lvgl_show_qrcode(str);
	#else
	lvgl_textbox_set_longtext(g_swVersion,3,-20);
	#endif
    mgui_screen_refresh( ctx );
    dspover = MGUI_STATE_THIRD;
    MGUI_DMSG("%s %d OK\n",__FUNCTION__,__LINE__);
	return ctx;

}
static void mgui_init4(struct mgui_context *ctx)
{
    ctx->state = MGUI_STATE_FORTH;
	ctx->dfb = GuiInit3();//lcd blank
	//ImageSetFromPath(background.h, background.path);
	//mgui_screen_refresh(ctx);
	lvgl_textbox_set_longtext(g_swVersion,3,-20);
    dspover = MGUI_STATE_FORTH;
    mgui_screen_refresh( ctx );
    MGUI_DMSG("%s %d OK\n",__FUNCTION__,__LINE__);
	return ctx;

}
static void mgui_init_sms_number(struct mgui_context *ctx)
{
    ctx->state = MGUI_STATE_SMS_NUMBER;
	ctx->dfb = GuiInit2();//lcd blank
	//ImageSetFromPath(background.h, background.path);
	set_icons(ctx, mgui_icons, MGUI_SMS_ICON);
	init_layout_sms(ctx);
	lvgl_textbox_set_shorttext(g_sms_unread,7,-16);
	lvgl_textbox_set_shorttext(g_sms_tip,8,-22);
	lvgl_textbox_simpletext(9,4);
    mgui_screen_refresh(ctx);
    dspover = MGUI_STATE_SMS_NUMBER; 
    MGUI_DMSG("%s %d OK\n",__FUNCTION__,__LINE__);
	return ctx;
}
static void mgui_init_wps(struct mgui_context *ctx)
{
	ctx->state = MGUI_STATE_WPS_PROCESSING;
	ctx->dfb = GuiInit2();//lcd blank
	//ImageSetFromPath(background.h, background.path);
	lvgl_textbox_simpletext(3,-20);
	mgui_screen_refresh(ctx);
	dspover = MGUI_STATE_WPS_PROCESSING; 
    MGUI_DMSG("%s %d OK\n",__FUNCTION__,__LINE__);
	return ctx;
}
void mgui_cp_assert_wakeup(struct mgui_context *ctx)
{
        enum e_mgui_state state;

        pthread_mutex_lock(&ctx->lock);
        state = ctx->state;
        pthread_mutex_unlock(&ctx->lock);
        switch (state) 
		{
			case MGUI_STATE_FIRST:
			MGUI_DMSG("already awake!!!\n");
			break;
			case MGUI_STATE_OFF:
			MGUI_DMSG("state=%d\n",state);
			prepare_status_first();
			pthread_mutex_lock(&ctx->lock);
			ctx->state = MGUI_STATE_FIRST;
			pm_set_screen(ctx, 1);
			mgui_screen_refresh(ctx);
			pthread_mutex_unlock(&ctx->lock);
			//mgui_ril_wakeup(ctx->ril);
			prepare_status_second();
			mgui_timer_all_start(ctx);
			break;
			default:
			MGUI_DMSG("unknown state!!!\n");
			break;
        }
}


static void mgui_sleep(struct mgui_context *ctx)
{
	enum e_mgui_state state;
	mgui_timer_all_stop(ctx);
	state = ctx->state;
	pthread_mutex_lock(&ctx->lock);
    mgui_init1(1,ctx,0);
	ctx->state = MGUI_STATE_OFF;
	pthread_mutex_unlock(&ctx->lock);
	prepare_status_first();
	#if defined	CONFIG_USE_MGUI_READ_SMS
	prepare_status_sms_number();
	#endif
	MGUI_DMSG("mgui ON to sleep\n");
}
#if 0
static void mgui_probe_modules(struct mgui_context *ctx)
{
	int need_resched = 0;

	#ifdef ENABLE_UBUS_CHARGER
	if ((ctx->charger == NULL) &&
	    (ctx->charger = mgui_charger_init(ctx)) == NULL) {
		MGUI_DMSG("charger service still not ready\n");
		need_resched = 1;
	}
	#endif

	if ((ctx->ril == NULL) &&
	    (ctx->ril = mgui_ril_init(ctx)) == NULL) {
		MGUI_DMSG("ril service still not ready\n");
		need_resched = 1;
	}

     #ifdef ENABLE_UBUS_WIFI
	if ((ctx->wifi == NULL) &&
	    (ctx->wifi = mgui_wifi_init(ctx)) == NULL) {
		MGUI_DMSG("wifi service still not ready\n");
		need_resched = 1;
	}
        #endif
	#ifdef ENABLE_UBUS_VERSION
	if ((ctx->ver == NULL) &&
	    (ctx->ver = mgui_version_init(ctx)) == NULL) {
		MGUI_DMSG("version service still not ready\n");
		need_resched = 1;
	}
	#endif
	MGUI_DMSG("service ready need_resched = %d\n", need_resched);
	#ifdef ENABLE_MODULES_TIMER
	if (!need_resched) {
		MGUI_DMSG("service ready stop modules timer\n");
		mgui_timer_stop(ctx, ctx->modules_timer);
	}
	#endif
}
#endif

void mgui_screen_onkeyevent_handle(struct mgui_context *ctx)
{
	MASSERT(ctx);
	enum e_mgui_state state;
	state = ctx->state;
	if (state == MGUI_STATE_FIRST)
	{
		pthread_mutex_lock(&ctx->lock);
		mgui_init2( ctx );
		pthread_mutex_unlock( &ctx->lock );
		prepare_status_third();
	}
	else if (state == MGUI_STATE_SECOND)
	{
		pthread_mutex_lock(&ctx->lock);
		mgui_init3( ctx );
		pthread_mutex_unlock( &ctx->lock );
		prepare_status_forth();
	}
	else if (state == MGUI_STATE_THIRD)
	{
		pthread_mutex_lock(&ctx->lock);
		#if defined CONFIG_USER_HKM
		mgui_init4( ctx );
		pthread_mutex_unlock( &ctx->lock );
		#else
        mgui_init1(1,ctx,-1);
		pthread_mutex_unlock( &ctx->lock );
		prepare_status_second();
		#endif
		prepare_status_first();
	}
	else if (state == MGUI_STATE_FORTH)
	{
		pthread_mutex_lock(&ctx->lock);
		#if !defined CONFIG_USE_MGUI_READ_SMS
        mgui_init1(1,ctx,-1);
		pthread_mutex_unlock(&ctx->lock);
		prepare_status_second();
		#else
		if(atoi(g_sms_unread)>0)
		{
			mgui_init_sms_number(ctx);
			pthread_mutex_unlock(&ctx->lock);
			prepare_status_first();
		}
		else
		{
            mgui_init1(1,ctx,-1);
			pthread_mutex_unlock(&ctx->lock);
			prepare_status_second();
		}
		#endif	
	}
	else if (state == MGUI_STATE_SMS_NUMBER)
	{
		pthread_mutex_lock(&ctx->lock);
        mgui_init1(1,ctx,-1);
		pthread_mutex_unlock( &ctx->lock );
		prepare_status_second();
	}
	else if (state == MGUI_STATE_OFF)
	{
		#if defined	CONFIG_USE_MGUI_READ_SMS
		prepare_status_sms_number();
		if(atoi(g_sms_unread)>0)
		{
			pthread_mutex_lock(&ctx->lock);
			pm_set_screen(ctx, 1);
			mgui_init_sms_number(ctx);
			pthread_mutex_unlock(&ctx->lock);
			prepare_status_first();
		}
		else
		#endif
		{
			prepare_status_first();
			pthread_mutex_lock(&ctx->lock);
            mgui_init1(0,ctx,1);
            pthread_mutex_unlock( &ctx->lock );
			prepare_status_second();
		}
	}
	else if (state == MGUI_STATE_SMS_END || state == MGUI_STATE_WPS_PROCESSING || state == MGUI_STATE_WPS_FAIL)
	{
		pthread_mutex_lock(&ctx->lock);
        mgui_init1(1,ctx,-1);
		pthread_mutex_unlock(&ctx->lock);
		prepare_status_second();
		
	}
	mgui_timer_all_start(ctx);
}

void mgui_screen_wpskeyevent_handle(struct mgui_context *ctx)
{
	MASSERT(ctx);
	enum e_mgui_state state;
	state = ctx->state;
	pthread_mutex_lock(&ctx->lock);
	if(state == MGUI_STATE_OFF)
	{
		pm_set_screen(ctx, 1);
	}
	mgui_init_wps(ctx);
	pthread_mutex_unlock(&ctx->lock);
	prepare_status_first();
	mgui_timer_all_start(ctx);
}
#if defined	CONFIG_USE_MGUI_READ_SMS
void mgui_screen_smskeyevent_handle(struct mgui_context *ctx)
{
	MASSERT(ctx);
	enum e_mgui_state state;
	state = ctx->state;
	pthread_mutex_lock(&ctx->lock);
	if(state == MGUI_STATE_SMS_NUMBER)
	{
		prepare_status_sms_number();
		if(atoi(g_sms_unread)>0)
		{
			mgui_init_sms_first(ctx);
		}
		else
		{
			mgui_init_sms_number(ctx);
		}
		pthread_mutex_unlock(&ctx->lock);
	}
	else if(state == MGUI_STATE_SMS_FIRST)
	{
		ctx->dfb = GuiInit2();
		prepare_sms_second();
		ctx->state = MGUI_STATE_SMS_SECOND;
		mgui_screen_refresh(ctx);
		pthread_mutex_unlock( &ctx->lock );
	}
	else if(state == MGUI_STATE_SMS_SECOND)
	{
		ctx->dfb = GuiInit2();
		if(sms_long)
		{
			prepare_sms_third();
			ctx->state = MGUI_STATE_SMS_THIRD;
		}
		else
		{
			prepare_sms_end();
			ctx->state = MGUI_STATE_SMS_END;
		}
		mgui_screen_refresh(ctx);
		pthread_mutex_unlock(&ctx->lock);
		prepare_status_sms_number();
	}
	else if(state == MGUI_STATE_SMS_THIRD)
	{
		ctx->dfb = GuiInit2();
		prepare_sms_end();
		ctx->state = MGUI_STATE_SMS_END;
		mgui_screen_refresh(ctx);
		pthread_mutex_unlock(&ctx->lock);
	}
	else if(state == MGUI_STATE_SMS_END)
	{
		mgui_init_sms_number(ctx);
		pthread_mutex_unlock(&ctx->lock);
	}
	else
	{	
		prepare_status_sms_number();
		if(state == MGUI_STATE_OFF)
		{
			pm_set_screen(ctx, 1);
		}
		mgui_init_sms_number(ctx);
		pthread_mutex_unlock(&ctx->lock);
		prepare_status_first();
	}
	mgui_timer_all_start(ctx);
}
#endif
void mgui_screen_chargerevent_handle(struct mgui_context *ctx)
{
	MASSERT(ctx);
	enum e_mgui_state state;
	state = ctx->state;
	if(state == MGUI_STATE_OFF || state == MGUI_STATE_FIRST)
	{
		prepare_status_first();
		pthread_mutex_lock( &ctx->lock );
        mgui_init1(0,ctx,1);
	}
	else
	{
		pthread_mutex_lock( &ctx->lock );
        mgui_init1(1,ctx,-1);
	}
	pthread_mutex_unlock( &ctx->lock );
	prepare_status_second();
	mgui_timer_all_start(ctx);

}
static void mgui_event_handle( struct uloop_fd *u, unsigned int events )
{
	struct mgui_context	*ctx = to_mgui_context( u );
	struct mgui_event	e;

	MASSERT( ctx );
	read( u->fd, &e, sizeof(struct mgui_event) );

	switch ( e.id )
	{
	case MGUI_EXIT_EVENT:
		MGUI_DMSG( "Exit event received\n" );
		uloop_end();
		break;
	case MGUI_ONKEY_EVENT:
		MGUI_DMSG( "onkey event received\n" );
		enum e_mgui_state state;
		pthread_mutex_lock( &ctx->lock );
		state = ctx->state;
		pthread_mutex_unlock(&ctx->lock);
		//if(state == MGUI_STATE_OFF) ||
		MGUI_DMSG("onkey event start state=[%d] dspover=[%d] g_key[%s]\n",state,dspover,g_key);
		if(state == MGUI_STATE_FIRST && dspover == MGUI_STATE_FIRST && strlen(g_key)<1)
		{
			/* Don't switch the screen when there is no password after reset default*/
			MGUI_DMSG("Don't switch the screen when there is no password after reset default\n");
			MGUI_DMSG("onkey event end state=[%d] dspover=[%d] \n",state,dspover);
			// prepare_wifi_password_info();
			return;
		}

		switch (state) 
		{
			case MGUI_STATE_FIRST: 
			case MGUI_STATE_SECOND:      
			case MGUI_STATE_THIRD:
			case MGUI_STATE_FORTH:
			#if defined CONFIG_USE_MGUI_READ_SMS
			case MGUI_STATE_SMS_NUMBER:
			#endif
			if (dspover != state)
			{
				MGUI_DMSG("onkey event end state=[%d] dspover=[%d] \n",state,dspover);
				break;
			}
			mgui_screen_onkeyevent_handle(ctx);
			break;
			case MGUI_STATE_OFF:
			case MGUI_STATE_SMS_END:
			case MGUI_STATE_WPS_PROCESSING:
			case MGUI_STATE_WPS_FAIL:
				mgui_screen_onkeyevent_handle(ctx);
				break;
		}
		#if defined	CONFIG_USE_MGUI_READ_SMS
		prepare_status_sms_number();
		#endif
		MGUI_DMSG("onkey event end state=[%d] dspover=[%d] \n",ctx->state,dspover);
		return;
	case MGUI_IDLE_TIMEOUT_EVENT:
		MGUI_DMSG( "idle timeout event received\n" );
		if ( ctx->state != MGUI_STATE_OFF && read_poweron_time() > 3500 )
			mgui_sleep( ctx );
		return;
	#ifdef ENABLE_MODULES_TIMER
	case MGUI_MODULES_TIMEOUT_EVENT:
		/*
		 * MGUI_DMSG("modules timeout event received\n");
		 * mgui_probe_modules(ctx);
		 */
		return;
	#endif
	#if defined CONFIG_USER_MGUI_WPS
	case MGUI_WPS_EVENT:
		/* enum e_mgui_state state; */
		MGUI_DMSG( "MGUI_WPS_EVENT received\n" );
		if(wps_lock == 0)
		mgui_screen_wpskeyevent_handle(ctx);
		#endif
		return;
	#if defined CONFIG_USE_MGUI_READ_SMS	
	case MGUI_SMSKEY_EVENT:
		DBG_MSG("MGUI_SMSKEY_EVENT received\n");
		mgui_screen_smskeyevent_handle(ctx);
        system("rm -rf /tmp/mgui_sms_action 2>/dev/null");
		return;
	#endif
	#if defined CONFIG_USER_HKM
	case MGUI_CHARGER_EVENT:
		/* enum e_mgui_state state; */
		DBG_MSG( "MGUI_CHARGER_EVENT received\n" );
		mgui_screen_chargerevent_handle(ctx);
		
#endif
	case MGUI_ALERT_ONKEY_EVENT:
		MGUI_DMSG( "Entering remote upgrade and reser mode\n" );
		ctx->state = MGUI_STATE_ALERT_ONKEY_EVENT;
		e.id = MGUI_ALERT_ONKEY_EVENT;
		pm_set_screen(ctx, 1);
		mgui_init_strings_only(ctx);
		mgui_screen_refresh(ctx);
		mgui_timer_all_stop(ctx);
		return;
	default:
		MGUI_DMSG( "Unknown event!!!\n" );
		break;
	}
}

int mgui_run(struct mgui_context *ctx)
{
	MASSERT(ctx);
	/* start timers */

    mgui_timer_all_start(ctx);
	/* setup uloop and run main loop to get indications */
	memset(&ctx->ubus_fd, 0, sizeof(ctx->ubus_fd));
	ctx->ubus_fd.cb = mgui_event_handle;
	ctx->ubus_fd.fd = ctx->pipes_fd[0];
	mgui_ubus_fd_add(&ctx->ubus_fd);
	mgui_ubus_uloop_run();

	pm_set_screen(ctx, 0); /* make sure screen is off */
	/* stop timers */
    mgui_timer_all_stop(ctx);
}

static void update_info_timer_handler(sigval_t sigval) 
{
	screen_update_info = true;
}

static void work_timer_handler(sigval_t sigval) 
{
	enter_sleep = true;
}

static void next_screen_timer_handler(sigval_t sigval) 
{
	next_screen = true;
}

struct mgui_context *mgui_init_main_speedway(int argc, char *argv[])
{
	struct mgui_context *ctx;
	// int tmp;
	if(access(FB_DEV, F_OK) != 0) {
		MGUI_DMSG("no %s, quit\n", FB_DEV);
		return NULL;
	}
	ctx = malloc(sizeof(*ctx));
	if (!ctx)
		return NULL;

	memset(ctx, 0, sizeof(*ctx));

	ctx->state = MGUI_STATE_OFF;
	
	// g_ctx=ctx;
	// /* init pipe */
	// if (pipe(ctx->pipes_fd)) {
	// 	MGUI_DMSG("pipe failed with error %s\n",
	// 		  strerror(errno));
	// 	goto out_pipe;
	// }

	if (pthread_mutex_init(&ctx->lock, NULL) != 0) {
		MGUI_DMSG("mutex init failed\n");
		goto out_mutex;
	}

	// if (mgui_timer_init(ctx, &ctx->clock_timer, clock_timer_handler) != 0) {
	// 	MGUI_DMSG("timer init failed\n");
	// 	goto out_clock_timer;
	// }

	if (mgui_timer_init(ctx, &ctx->update_info_timer, update_info_timer_handler) != 0) {
		MGUI_DMSG("timer init failed\n");
		goto out_update_info_timer;
	}

	if (mgui_timer_init(ctx, &ctx->work_timer, work_timer_handler) != 0) {
		MGUI_DMSG("timer init failed\n");
		goto out_work_timer;
	}

	if (mgui_timer_init(ctx, &ctx->next_screen_timer, next_screen_timer_handler) != 0) {
		MGUI_DMSG("timer init failed\n");
		goto out_next_screen_timer;
	}

	// prepare_status_first();
	// /* this is boot on ,it need get the second after lcd link ,after it will hold the data when ubus run*/
	// prepare_status_second();

	pthread_mutex_lock(&ctx->lock);
	ctx->dfb = GuiInit();
	if (!ctx->dfb) {
		MGUI_DMSG("gui init failed\n");
	}
	ctx->state = MGUI_STATE_FIRST;
	mgui_screen_refresh(ctx);
	pthread_mutex_unlock(&ctx->lock);

	return ctx;

out_next_screen_timer:
	timer_delete(ctx->next_screen_timer);
out_work_timer:
	timer_delete(ctx->work_timer);
out_update_info_timer:
	timer_delete(ctx->update_info_timer);
out_mutex:
	pthread_mutex_destroy(&ctx->lock);
out_pipe:
	close(ctx->pipes_fd[0]);
	close(ctx->pipes_fd[1]);
	free(ctx);
	ctx=NULL;
	return NULL;
}

int mgui_run_speedway(struct mgui_context *ctx)
{
	static int screen = 0;
	mgui_timer_start( ctx, ctx->work_timer, ((long long)lvgl_worktime() * 1000000000) ); // timer uses nanoseconds
	mgui_timer_start( ctx, ctx->update_info_timer, (5 * 1000000000) ); // refresh screen each 5 seconds
	mgui_timer_start( ctx, ctx->next_screen_timer, ((long long)lvgl_next_sreen() * 1000000000) );
	while (1) {
		if (enter_sleep) {
			MGUI_IMSG("Enter sleep\n");
			break;
		}
		if (next_screen) {
			++screen;
			if (screen > 1) {
				screen = 0;
			}
		}
		if (screen_update_info || next_screen) {
			screen_update_info = false;
			next_screen = false;
			lvgl_update_info(screen);
			mgui_screen_refresh(ctx);
		}
		sleep(1);
	}
}

int mgui_exit_speedway(struct mgui_context *ctx)
{
	MASSERT(ctx);
	GuiDeinit(ctx->dfb);
	int worktime = lvgl_worktime();
	if (worktime != 0) {
		MGUI_IMSG("Disable screen\n");
		fbdev_power(0);
	}
	pthread_mutex_destroy(&ctx->lock);

	timer_delete(ctx->work_timer);
	timer_delete(ctx->update_info_timer);
	timer_delete(ctx->next_screen_timer);

	free(ctx);
	return 0;
}