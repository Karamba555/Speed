#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include "asr_lvgl.h"
#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_lib_png/lv_png.h"
#include "lvgl/src/lv_core/lv_style.h"
#include "lv_lib_qrcode/lv_qrcode.h"
#include "user_conf.h"

#define Modemtype_MAX_LENGTH 16
#define DISP_BUF_SIZE (240 * 320)
static int gQrcodeShow = 1;

struct lv_qrcode {
	lv_style_t style;
	void *base;
	void *qrcode;
	void *label;
};

static struct lv_qrcode gQrCode;

struct wifi_data_t {
	char ssid24g[64];
	char ssid5g[64];
	char psk24g[64];
	char psk5g[64];
	int state24g;
	int state5g; 
	bool encrypt24g;
};

static struct wifi_data_t wifi_data;

struct lcd_settings_t {
	int orientation;	/* 0 - Portrait, 1 - Landscape */ 
	int details;		/* 0 - Minimal, 1 - Full */
	int worktime;		/* display stays on for worktime sec */
	int next_screen;	/* switch to a next screen after n sec */
};

struct lcd_settings_t lcd_settings;

static void wifi_status (int state24g, int state5g);

int image_get_size(const char *path, int *w, int *h)
{
	lv_img_header_t header;
    lv_img_decoder_get_info(path, &header);
	*w = header.w;
	*h = header.h;
	return 0;
}

void* lvgl_create_image(const char *name)
{
	lv_obj_t * img = lv_img_create(lv_scr_act(), NULL);
	lv_img_set_src(img, name);
	lv_obj_set_hidden(img, true);
	lv_img_set_auto_size(img, false);
	return img;
}

void lvgl_obj_del(void *obj)
{
	if (obj)
		lv_obj_del(obj);
}

void lvgl_image_show(void *img, int xs, int ys, int w, int h)
{
	if (img) {
		lv_img_ext_t * ext = lv_obj_get_ext_attr(img);
		int zoom = 256 + 256 * (w - ext->w) / ext->w;
		lv_img_set_pivot(img, 0, 0);
		lv_img_set_zoom(img, zoom);
		lv_obj_set_hidden(img, false);
		lv_obj_set_pos(img, xs, ys);
	}
}

void lvgl_image_hide(void *img)
{
	if (img)
		lv_obj_set_hidden(img, true);
}

void* lvgl_create_textbox()
{
	lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);
	return label;
}

int lvgl_textbox_set(void *label, const char *text, OBJECT_COLOR *c, OBJECT_POS *pos)
{
	char buf[256];
	int x, y;
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%s%06x %s%s", LV_TXT_COLOR_CMD, (c->red << 16) | (c->green) << 8 | c->blue, text, LV_TXT_COLOR_CMD);
	lv_label_set_recolor(label, true);
	
	//lv_img_ext_t * ext = lv_obj_get_ext_attr(label);
	//int zoom = 256;
	//lv_img_set_pivot(label, 0, 0);
	//lv_label_set_angle(label,60);
	//lv_img_set_zoom(label, zoom);
	lv_label_set_text(label, buf);
	//lv_obj_set_hidden(label, false);
    //lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	//lv_label_set_align(label, LV_LABEL_ALIGN_AUTO);
	lv_label_set_long_mode(label, LV_LABEL_LONG_EXPAND);

	x = pos->x;
	y = pos->y;

	if (x)
		x = (128 - lv_obj_get_width(label))/2;
	lv_obj_set_pos(label, x, y - 15);
	DBG_MSG("%s:%s:%d,x:%d,y:%d,text:%s !!!\n",__FILE__,__FUNCTION__,__LINE__,x,y,text);
	return 0;
}
int lvgl_textbox_set_longtext(const char *text,int str_prefix,int pos)
{
	char buf[512];
	memset(buf, 0, sizeof(buf));
	DBG_MSG("str_prefix:%d,pos:%d,text=%s\n",str_prefix,pos,text);
	if(str_prefix==1)
	{
		sprintf(buf, "SSID:%s",text);
	}
	else if(str_prefix==2)
	{
		sprintf(buf, "PW:%s",text);
	}
	else if(str_prefix==3)
	{
		sprintf(buf, "FW  Ver:%s", text);
	}
	else
	{
		sprintf(buf, "%s", text);
	}
	int len = strlen(buf);
	DBG_MSG("len=%d\n",len);
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_align(label, LV_LABEL_ALIGN_AUTO);
	lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
	lv_obj_set_width(label, 150);
	if(len > 27)
	{
		if(str_prefix==3 || str_prefix==4)
		{
			static lv_style_t font_style;
			lv_style_init(&font_style);
			lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_12);
			lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
			lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
		}
		else
		{
			static lv_style_t font_style;
			lv_style_init(&font_style);
			lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_10);
			lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
			lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);

		}
	}
	else if(len > 15 && len <= 27) 
	{
		static lv_style_t font_style;
		lv_style_init(&font_style);
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_12);
		lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
		lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
	}
	else if(len <= 15 )
	{
		static lv_style_t font_style;
		lv_style_init(&font_style);
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);
		lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
		lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
	}
	lv_label_set_text(label, buf);
	lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 2, pos);
	return 0;
}
int lvgl_textbox_set_shorttext(const char *text,int str_prefix,int pos)
{
	char buf[512];
	memset(buf, 0, sizeof(buf));
	DBG_MSG("str_prefix:%d,pos:%d,text=%s\n",str_prefix,pos,text);

	if(text==NULL)
	{
		if(str_prefix ==4 || str_prefix ==7)
		{
			sprintf(buf, "0");
		}
		else if(str_prefix==5)
		{
			sprintf(buf, "NO SIM");
		}
		else if(str_prefix ==6 || str_prefix ==8)
		{
			sprintf(buf, " ");
		}
	}
	else
	{
        if(strlen(text) == 0)
        {
            sprintf(buf, " ");
        }
        else{
            sprintf(buf, "%s", text);
        }
		
	}
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_align(label, LV_LABEL_ALIGN_AUTO);
	if(str_prefix ==4)
	{
		static lv_style_t font_style;
		lv_style_init(&font_style);
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_10);
		lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
		lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
		
	}
	else if(str_prefix==5 || str_prefix ==7 || str_prefix ==8 || str_prefix ==9)
	{
		static lv_style_t font_style;
		lv_style_init(&font_style);
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);
		lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
		lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
	}
	else if(str_prefix==6)
	{
		static lv_style_t font_style;
		lv_style_init(&font_style);
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_12);
		lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
		lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
	}
	
	lv_label_set_text(label, buf);
	if(str_prefix ==4)
	{
		lv_obj_align(label, NULL, LV_ALIGN_CENTER, 2, pos);
	}
	else if(str_prefix==5)
	{
		lv_obj_align(label, NULL, LV_ALIGN_CENTER, -10, pos);
	}
	else if(str_prefix==6)
	{
		lv_obj_align(label, NULL, LV_ALIGN_CENTER, -22, pos);
	}
	else if(str_prefix==7)
	{
		lv_obj_align(label, NULL, LV_ALIGN_CENTER, -20, pos);
	}
	else if(str_prefix==8)
	{
		lv_obj_align(label, NULL, LV_ALIGN_CENTER, 30, pos);
	}
	else if(str_prefix==9)
	{
		lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 2, pos);
	}
	return 0;

}
int lvgl_textbox_set3(const char *text,int str_prefix,int pos)
{
	char buf0[512],buf[512],buf1[128],buf2[64],buf3[64],buf4[64];
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	static lv_style_t font_style;
	lv_style_init(&font_style);
	lv_label_set_align(label, LV_LABEL_ALIGN_AUTO);
	memset(buf, 0, sizeof(buf));
	memset(buf0, 0, sizeof(buf0));
	memset(buf1, 0, sizeof(buf1));
	memset(buf2, 0, sizeof(buf2));
	memset(buf3, 0, sizeof(buf3));
	memset(buf4, 0, sizeof(buf4));
	DBG_MSG("str_prefix:%d,pos:%d,text=%s\n",str_prefix,pos,text);
	if(str_prefix<=3)
	{
		if(str_prefix==1)
		{
			sprintf(buf1, "Wi-Fi SSID:%s",text);
		}
		else if(str_prefix==2)
		{
			sprintf(buf1, "Password:%s",text);
		}
		else if(str_prefix==3)
		{
			sprintf(buf1, "FW  Ver:%s", text);
		}
		int len = strlen(buf1);
		DBG_MSG("len=%d\n",len);
		if(len > 44 )
		{
			
			strncpy(buf2, buf1, 22);
			strncpy(buf3, buf1+22, 22);
			strncpy(buf4, buf1+44, len-44);
			DBG_MSG("buf2=%s  buf3=%s  buf4=%s\n",buf2,buf3,buf4);
			sprintf(buf, "%s\n%s\n%s",buf2,buf3,buf4);
			static lv_style_t font_style;
			lv_style_init(&font_style);
			lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
			lv_obj_set_width(label, 160);
			lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_10);
			lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
			lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
			lv_label_set_text(label, buf);
			lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 2, pos);
			return 0;
		}
		else if(len > 20 && len <= 44)
			{
			strncpy(buf2, buf1, 20);
			strncpy(buf3, buf1+20, len-20);
			DBG_MSG("buf2=%s  buf3=%s\n",buf2,buf3);
			sprintf(buf, "%s\n%s",buf2,buf3);
			static lv_style_t font_style;
			lv_style_init(&font_style);
			lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
			lv_obj_set_width(label, 160);
			lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_12);
			lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
			lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
			lv_label_set_text(label, buf);
			lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 2, pos);
			return 0;
			}
		else if(len <= 20 )
		{
			sprintf(buf, "%s",buf1);
			static lv_style_t font_style;
			lv_style_init(&font_style);
			lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);
			lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
			lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
			lv_label_set_text(label, buf);
			lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 2, pos);
			return 0;
		}
	}
	else if(str_prefix==4)
	{
		if(text==NULL)
		{
		sprintf(buf, "0");
		}
		else
		sprintf(buf, "%s", text);
		static lv_style_t font_style;
		lv_style_init(&font_style);
		lv_label_set_align(label, LV_LABEL_ALIGN_AUTO);
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_10);
		lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
		lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
		lv_label_set_text(label, buf);
		lv_obj_align(label, NULL, LV_ALIGN_CENTER, 2, pos);
		return 0;
	}
	else if(str_prefix==5)
	{
		lv_label_set_recolor(label, true);
		if(text==NULL)
		{
			///sprintf(buf, "#ffffff NO SIM#");
		}
		else
		sprintf(buf, "#ffffff %s#", text);
		static lv_style_t font_style;
		lv_style_init(&font_style);
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);
		lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
		lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
		lv_label_set_text(label, buf);
		lv_obj_align(label, NULL, LV_ALIGN_CENTER, -10, pos);
		return 0;
	}
	else if(str_prefix==6)
	{
		if(text==NULL)
		{
		sprintf(buf, "");
		}
		else
		sprintf(buf, "%s", text);
		static lv_style_t font_style;
		lv_style_init(&font_style);
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_12);
		lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
		lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
		lv_label_set_text(label, buf);
		lv_obj_align(label, NULL, LV_ALIGN_CENTER, -22, pos);
		return 0;
	}
	else if(str_prefix==7)
	{
		if(text==NULL)
		{
		sprintf(buf, "0");
		}
		else
		sprintf(buf, "%s", text);
		static lv_style_t font_style;
		lv_style_init(&font_style);
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);
		lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
		lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
		lv_label_set_text(label, buf);
		lv_obj_align(label, NULL, LV_ALIGN_CENTER, -20, pos);
		return 0;
	}
	else if(str_prefix==8)
	{
		if(text==NULL)
		{
		sprintf(buf, "");
		}
		else
		sprintf(buf, "%s", text);
		static lv_style_t font_style;
		lv_style_init(&font_style);
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);
		lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
		lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
		lv_label_set_text(label, buf);
		lv_obj_align(label, NULL, LV_ALIGN_CENTER, 30, pos);
		return 0;
	}
	else if(str_prefix==9)
	{
		sprintf(buf, "%s", text);
		static lv_style_t font_style;
		lv_style_init(&font_style);
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);
		lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
		lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
		lv_label_set_text(label, buf);
		lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 2, pos);
		return 0;
	}
    else 
	{
		sprintf(buf, "N/A");
	}
	lv_label_set_align(label, LV_LABEL_ALIGN_AUTO);
	lv_label_set_text(label, buf);
	lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, pos);
	return 0;
}

int lvgl_textbox_simpletext(int textnumber,int pos)
{
	char buf[256];
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	memset(buf, 0, sizeof(buf));
	if(textnumber == 1)
	{
		//sprintf(buf, "Wi-Fi SSID:");
		sprintf(buf, "SSID:");
	}
	else if(textnumber == 2)
	{
		//sprintf(buf, "Password:");
		sprintf(buf, "PW:");
	}
	else if(textnumber == 3)
	{
		sprintf(buf, "WPS is Processing");
	}
	else if(textnumber == 4)
	{
		sprintf(buf, "WPS is failed");
	}
	else if(textnumber == 5)
	{
		sprintf(buf, "Scan to");
	}
	else if(textnumber == 6)
	{
		sprintf(buf, "Connect");
	}
	else if(textnumber == 7)
	{
		sprintf(buf, "NEW SMS");
	}
	else if(textnumber == 8)
	{
		sprintf(buf, "SMS End");
	}
	else if(textnumber == 9 )
	{
		sprintf(buf, "P:Skip W:Read");
	}
	static lv_style_t font_style;
	lv_style_init(&font_style);
	lv_label_set_align(label, LV_LABEL_ALIGN_AUTO);

	if(textnumber == 5 || textnumber == 6 || textnumber == 7)
	{
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);
	}
	else if (textnumber == 8)
	{
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_18);
	}
	else
	{
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_16);
	}
	lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
	lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
	lv_label_set_text(label, buf);
	if(textnumber == 5 || textnumber == 6)
	{
		lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 88, pos);
	}
	else if (textnumber == 8 )
	{
		lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 50, pos);
	}
	else if (textnumber == 3 )
	{
		lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 2, pos);
	}
	else
	{
		lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 20, pos);
	}
	return 0;
}
#if 1
int lvgl_textbox_set_sms(const char *text,int pos)
{
	char buf[512];
	memset(buf, 0, sizeof(buf));
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	static lv_style_t font_style;
	lv_style_init(&font_style);
	lv_label_set_align(label, LV_LABEL_ALIGN_AUTO);
	sprintf(buf, "%s",text);
	int len = strlen(buf);
	DBG_MSG("len=%d\n",len);
	if(len > 0 && len <= 10)
	{
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_18);
	}
	else if (len > 10 && len <= 100)
	{
		lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
		lv_obj_set_width(label, 150);
		if(len <= 40)
		{
			lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_16);
		}
		else if (len > 40 && len <= 60)
		{
			lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);
		}
		else
		{
			lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_12);
		}
	}
	lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
	lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
	lv_label_set_text(label, buf);
	lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 2, pos);
	return 0;
}
#else
int lvgl_textbox_set_sms(const char *text,int pos)
{
	char buf[512],buf1[128],buf2[128],buf3[128],buf4[128],buf5[128],buf6[128];
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	static lv_style_t font_style;
	lv_style_init(&font_style);
	lv_label_set_align(label, LV_LABEL_ALIGN_AUTO);
	memset(buf, 0, sizeof(buf));
	memset(buf1, 0, sizeof(buf1));
	memset(buf2, 0, sizeof(buf2));
	memset(buf3, 0, sizeof(buf3));
	memset(buf4, 0, sizeof(buf4));
	memset(buf5, 0, sizeof(buf5));
	memset(buf6, 0, sizeof(buf6));
	sprintf(buf1, "%s",text);
	int len = strlen(buf1);
	DBG_MSG("len=%d\n",len);
	if(len > 60 && len <= 100 )
	{
		strncpy(buf2, buf1, 20);
		strncpy(buf3, buf1+20, 20);
		strncpy(buf4, buf1+40, 20);
		if(len > 60 && len <= 80)
		{
			strncpy(buf5, buf1+60, len-60);
			DBG_MSG("buf2=%s  buf3=%s buf4=%s buf5=%s\n",buf2,buf3,buf4,buf5);
			sprintf(buf, "%s\n%s\n%s\n%s",buf2,buf3,buf4,buf5);
		}
		else
		{
			strncpy(buf5, buf1+60, 20);
			strncpy(buf6, buf1+80, len-80);
			DBG_MSG("buf2=%s  buf3=%s buf4=%s buf5=%s buf6=%s\n",buf2,buf3,buf4,buf5,buf6);
			sprintf(buf, "%s\n%s\n%s\n%s\n%s",buf2,buf3,buf4,buf5,buf6);
		}
		lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
		lv_obj_set_width(label, 160);
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_12);
		lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
		lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
		lv_label_set_text(label, buf);
		lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 2, pos);
		return 0;
	}
	else if(len > 10 && len <= 60)
	{
		if (len > 10 && len <= 15)
		{
			sprintf(buf, "%s",buf1);
			lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_16);
			lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
			lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
			lv_label_set_text(label, buf);
			lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 2, pos);
		return 0;
		}
		if (len > 15 && len <= 30)
		{
			strncpy(buf2, buf1, 15);
			strncpy(buf3, buf1+15, len-15);
			DBG_MSG("buf2=%s  buf3=%s\n",buf2,buf3);
			sprintf(buf, "%s\n%s",buf2,buf3);

		}
		else if (len > 30 && len <= 45)
		{
			strncpy(buf2, buf1, 15);
			strncpy(buf3, buf1+15, 15);
			strncpy(buf4, buf1+30, len-30);
			DBG_MSG("buf2=%s  buf3=%s buf4=%s\n",buf2,buf3,buf4);
			sprintf(buf, "%s\n%s\n%s",buf2,buf3,buf4);
		}
		else 
		{
			strncpy(buf2, buf1, 15);
			strncpy(buf3, buf1+15, 15);
			strncpy(buf4, buf1+30, 15);
			strncpy(buf5, buf1+45, len-45);
			DBG_MSG("buf2=%s  buf3=%s buf4=%s buf5=%s\n",buf2,buf3,buf4,buf5);
			sprintf(buf, "%s\n%s\n%s\n%s",buf2,buf3,buf4,buf5);
		}
		lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
		lv_obj_set_width(label, 160);
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_16);
		lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
		lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
		lv_label_set_text(label, buf);
		lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 2, pos);
		return 0;
	}
	else if(len > 0 && len <= 10 )
	{	
		sprintf(buf, "%s",buf1);
		lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_18);
		lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_WHITE);
		lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
		lv_label_set_text(label, buf);
		lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 2, pos);
		return 0;
	}
}
#endif
int lvgl_textbox_set_test(int pos)
{
	char buf[256];
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "test font color");
	static lv_style_t font_style;
	lv_style_init(&font_style);
	lv_label_set_align(label, LV_LABEL_ALIGN_AUTO);
	lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_10);
	lv_style_set_text_color(&font_style,LV_STATE_DEFAULT,LV_COLOR_RED);
	lv_obj_add_style(label,LV_LABEL_PART_MAIN, &font_style);
	lv_label_set_text(label, buf);
	lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 0, pos);
	return 0;
}
static void qrcode_create(char *str)
{
	int xres = fbdev_get_xres();
	int yres = fbdev_get_yres();
	gQrCode.base = lv_obj_create(lv_scr_act(), NULL);
	//lv_obj_set_size(gQrCode.base, xres, yres);
	lv_obj_set_size(gQrCode.base, 80, 80);

	lv_style_init(&gQrCode.style);
    lv_style_set_radius(&gQrCode.style, LV_STATE_DEFAULT, 0);
	
	lv_obj_add_style(gQrCode.base, LV_OBJ_PART_MAIN, &gQrCode.style);
	gQrCode.qrcode = lv_qrcode_create(lv_scr_act(), 80, LV_COLOR_BLACK, LV_COLOR_WHITE);
	lv_qrcode_update(gQrCode.qrcode, str, strlen(str));
	lv_obj_set_pos(gQrCode.qrcode, 0, 26);
	gQrCode.label = lvgl_create_textbox();

	OBJECT_COLOR c;
	OBJECT_POS pos;
	c.red = 0;
	c.green = 0;
	c.blue = 0;
	pos.x = 0;
	pos.y = yres;
	lvgl_textbox_set((void *)gQrCode.label, "Scan to connect", &c, &pos);
}

static void qrcode_del(void)
{
	if (gQrCode.qrcode)
		lv_qrcode_delete(gQrCode.qrcode);
	if (gQrCode.label)
		lv_obj_del(gQrCode.label);
	if (gQrCode.base)
		lv_obj_del(gQrCode.base);
	memset(&gQrCode, 0, sizeof(gQrCode));
}

void lvgl_show_qrcode(char *str)
{
	//gQrcodeShow = !gQrcodeShow;
	if (gQrcodeShow) {
		qrcode_create(str);
	} else {
		qrcode_del();
	}
	lv_task_handler();
}

static char* read_modemtype_from_file(const char* filename, char* modem_buf, size_t buf_size) {
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		modem_buf[0] = '\0';
		return NULL;
	}
	char line[256];
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "Modem Type:", 11) == 0) {
			char* modem_start = line + 11;
			char* modem_end = strchr(modem_start, ';');
			if (modem_end != NULL && (size_t)(modem_end - modem_start) < buf_size - 1) {
				strncpy(modem_buf, modem_start, modem_end - modem_start);
				modem_buf[modem_end - modem_start] = '\0';
				break;
			}
		}
	}
	
	fclose(fp);
	return modem_buf;
}

static char* read_connecttype_from_file(const char* filename,char* connect_buf, size_t buf_size) {
	FILE* fp = fopen(filename,"r");
	if (fp == NULL) {
		connect_buf[0] = '\0';
		return NULL;
	}
	char line[256];
	while (fgets(line,sizeof(line),fp)) {
		if(strncmp(line,"Connection Status:",18) ==0) {
			char* connect_start = line + 18;
			char* connect_end = strchr(connect_start, ';');
			if (connect_end != NULL && (size_t)(connect_end - connect_start) < buf_size - 1) {
				strncpy(connect_buf, connect_start, connect_end - connect_start);
				connect_buf[connect_end - connect_start] = '\0';
				break;

			}
		}
	}

	fclose(fp);
	return connect_buf;
}


static char* read_Signal_from_file(const char* filename, char* signal_buf, size_t buf_size) {
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		signal_buf[0] = '\0';
		return NULL;
	}
	
	char line[256];
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "Signal Level:", 13) == 0) {
			char* signal_start = line + 13;
			char* signal_end = strchr(signal_start, ';');
			if (signal_end != NULL && (size_t)(signal_end - signal_start) < buf_size - 1) {
				strncpy(signal_buf, signal_start, signal_end - signal_start);
				signal_buf[signal_end - signal_start] = '\0';
				break;
			}
		}
	}
	
	fclose(fp);
	return signal_buf;
}

static char* read_RSRP_from_file(const char* filename, char* rsrp_buf, size_t buf_size) {
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		rsrp_buf[0] = '\0';
		return NULL;
	}
	
	char line[256];
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "Signal RSRP:", 12) == 0) {
			char* rsrp_start = line + 12;
			char* rsrp_end = strchr(rsrp_start, ';');
			if (rsrp_end != NULL && (size_t)(rsrp_end - rsrp_start) < buf_size - 1) {
				strncpy(rsrp_buf, rsrp_start, rsrp_end - rsrp_start);
				rsrp_buf[rsrp_end - rsrp_start] = '\0';
				break;
			}
		}
	}
	
	fclose(fp);
	return rsrp_buf;
}


char* read_ssid_from_file(const char* filename) {
	static char ssid[256];
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		ssid[0] = '\0';
		return ssid;
	}
	
	char line[256];
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "SSID1=", 6) == 0) {
			strncpy(ssid, line + 6, sizeof(ssid) - 1);
			ssid[strcspn(ssid, "\n")] = 0;
			break;
		}
	}
	
	fclose(fp);
	return ssid;
}

char* read_psk_from_file(const char* filename) {
	static char psk[256];
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		psk[0] = '\0';
		return psk;
	}
	char line[256];
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "WPAPSK1=", 8) == 0) {
			strncpy(psk, line + 8, sizeof(psk) - 1);
			psk[strcspn(psk, "\n")] = 0;
			break;
		}
	}
	fclose(fp);
	return psk;
}

char* read_5gssid_from_file(const char* filename) {
	static char ssid[256];
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		ssid[0] = '\0';
		return ssid;
	}
	
	char line[256];
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "SSID1=", 6) == 0) {
			strncpy(ssid, line + 6, sizeof(ssid) - 1);
			ssid[strcspn(ssid, "\n")] = 0;
			break;
		}
	}
	
	fclose(fp);
	return ssid;
}

bool read_wireless_info_from_file(const char* filename) {
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		return false;
	}
	char ssid[2][64];
	int ssid_count = 0;
	char pass[2][64];
	int pass_count = 0;
	char disabled[2][10];
	int disabled_count = 0;
	char encrypt[2][10];
	int encr_count = 0;

	char line[256];
	char* ptr;
	while (fgets(line, sizeof(line), fp)) {
		ptr = strstr(line, "option ssid ");
		if (ptr != 0) {
			if (sscanf(ptr, "%*[^\']\'%63[^\']\'", ssid[ssid_count]) == 1) {
				ssid_count = !ssid_count;
			}
		}
		ptr = strstr(line, "option key ");
		if (ptr != 0) {
			if (sscanf(ptr, "%*[^\']\'%63[^\']\'", pass[pass_count]) == 1) {
				pass_count = !pass_count;
			}
		}
		ptr = strstr(line, "option disabled ");
		if (ptr != 0) {
			if (sscanf(ptr, "%*[^\']\'%63[^\']\'", disabled[disabled_count]) == 1) {
				disabled_count = !disabled_count;
			}
		}
		ptr = strstr(line, "option encryption ");
		if (ptr != 0) {
			if (sscanf(ptr, "%*[^\']\'%63[^\']\'", encrypt[encr_count]) == 1) {
				encr_count = !encr_count;
			}
		}
	}

	snprintf(wifi_data.ssid24g, sizeof(ssid[0]), "%s", ssid[0]);
	snprintf(wifi_data.ssid5g, sizeof(ssid[1]), "%s", ssid[1]);
	snprintf(wifi_data.psk24g, sizeof(pass[0]), "%s", pass[0]);
	snprintf(wifi_data.psk5g, sizeof(pass[1]), "%s", pass[1]);
	wifi_data.state24g = atoi(disabled[0]);
	wifi_data.state5g = atoi(disabled[1]);
	if (strstr(encrypt[0], "none") != 0) {
		wifi_data.encrypt24g = false;
	} else {
		wifi_data.encrypt24g = true;
	}
	fclose(fp);
	return true;
}

// reads one line from file
char* read_info_from_file(const char* filename) {
	static char info[256];
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		info[0] = '\0';
		return info;
	}

	char line[256];
	if (fgets(line, sizeof(line), fp) != NULL) {
		strncpy(info, line, sizeof(info) - 1);
		info[strcspn(info, "\n")] = 0;
	} else {
		info[0] = '\0';
	}

	fclose(fp);
	return info;
}

// reads one line from file
bool read_onoff_from_file(const char* filename) {
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		return false;
	}

	char line[256];
	if (fgets(line, sizeof(line), fp) != NULL) {
		if (strstr(line, "ON") != NULL) {
    	    fclose(fp);
            return true;
        }
	}
    fclose(fp);
    return false;
}

void signal_init(void)
{
	lv_obj_t * img_signal = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_size(img_signal, 80, 60);
	if (lcd_settings.orientation) {
		lv_obj_align(img_signal, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 30);
	} else {
		lv_obj_align(img_signal, NULL, LV_ALIGN_IN_TOP_LEFT, 21, 5);
	}
	char signal_buf[256];
	if (read_Signal_from_file("/tmp/wan_status", signal_buf, sizeof(signal_buf)))
	{
		const char *signal_image_path;
		if (strstr(signal_buf, "No Signal") != NULL) {
			signal_image_path = "/usr/share/mgui/images/nosignal.png";
		}	else if (strstr(signal_buf, "Low Signal") != NULL) {
				signal_image_path = "/usr/share/mgui/images/lowsignal.png";
		}	else if (strstr(signal_buf, "Bad Signal") != NULL) {
				signal_image_path = "/usr/share/mgui/images/badsignal.png";
	    }	else if (strstr(signal_buf, "Good Signal") != NULL) {
				signal_image_path = "/usr/share/mgui/images/goodsignal.png";
		}	else if (strstr(signal_buf, "Excellent Signal") != NULL) {
				char *pos = strstr(signal_buf, "Excellent Signal");
				if (pos) {
				pos = strchr(pos, ' ');
					if (pos) {
					*pos = '\n';
					}
				}
				signal_image_path = "/usr/share/mgui/images/excellentsignal.png";
		} else {
			signal_image_path = "/usr/share/mgui/images/waitsignal.png";
		}
		lv_img_set_src(img_signal, signal_image_path);
	}
	else {
		lv_img_set_src(img_signal, "/usr/share/mgui/images/waitsignal.png");
	}
}

void network_init (void)
{
	lv_obj_t * img_network = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_size(img_network, 80, 60);
	if (lcd_settings.orientation) {
		lv_obj_align(img_network, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 150);
	} else {
		lv_obj_align(img_network, NULL, LV_ALIGN_IN_TOP_LEFT, 141, 5);
	}
	lv_obj_t *label_network = lv_label_create(lv_scr_act(), NULL);
	char modem_buf[256];
	if (read_modemtype_from_file("/tmp/wan_status", modem_buf, sizeof(modem_buf)))
	{
		const char *network_image_path;
		if (strstr(modem_buf, "4GLTE") != NULL) {
			network_image_path = "/usr/share/mgui/images/4GLTE.png";
		} else if (strstr(modem_buf, "4G") != NULL) {
			network_image_path = "/usr/share/mgui/images/4G.png";
	    }	else if (strstr(modem_buf, "5GPlus") != NULL) {
			network_image_path = "/usr/share/mgui/images/5GPlus.png";
	    }	else if (strstr(modem_buf, "5G") != NULL) {
			network_image_path = "/usr/share/mgui/images/5G.png";
	    }	else if (strstr(modem_buf, "NSA") != NULL) {
			network_image_path = "/usr/share/mgui/images/NSA.png";
	    }	else {
			network_image_path = "/usr/share/mgui/images/waittao.png";
		}
		lv_img_set_src(img_network, network_image_path);
	}
	else
	{
		lv_img_set_src(img_network, "/usr/share/mgui/images/waittao.png");
	}

}

void wifi_update_data(void)
{
	if (read_wireless_info_from_file("/etc/config/wireless") == false) {
		sprintf(wifi_data.ssid24g, "");
		sprintf(wifi_data.ssid5g, "");
		sprintf(wifi_data.psk24g, "");
		sprintf(wifi_data.psk5g, "");
	}
	wifi_status(wifi_data.state24g, wifi_data.state5g);
}

void wifi_ssid_display(void)
{
	static lv_style_t tao_label_style;
	lv_style_init(&tao_label_style);
	lv_style_set_text_color(&tao_label_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_style_set_text_font(&tao_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);
	
	if (lcd_settings.details) {
		lv_obj_t * scr = lv_scr_act();
		lv_obj_t * label_header = lv_label_create(scr, NULL);
		char header_label_text[256];

		lv_obj_t * scr_info = lv_scr_act();
		lv_obj_t * label_info = lv_label_create(scr_info, NULL);
		char info_label_text[256];

		const char* header_24g = "SSID 2.4G:\n";
		const char* header_passw0 = "Password:\n";
		const char* header_5g = "SSID 5G:\n";
		const char* header_passw1 = "Password:\n";

		if (lcd_settings.orientation) {
			snprintf(header_label_text, sizeof(header_label_text), "%s\n%s\n%s\n%s\n", header_24g, header_passw0, header_5g, header_passw1);
			lv_label_set_text(label_header, header_label_text);
			lv_obj_add_style(label_header, LV_OBJ_PART_MAIN, &tao_label_style);
			lv_obj_align(label_header, NULL, LV_ALIGN_IN_TOP_LEFT, 155, 5);
			snprintf(info_label_text, sizeof(info_label_text), "%s\n\n%s\n\n%s\n\n%s\n\n", wifi_data.ssid24g, wifi_data.psk24g, wifi_data.ssid5g, wifi_data.psk5g);
			lv_label_set_text(label_info, info_label_text);
			lv_obj_add_style(label_info, LV_OBJ_PART_MAIN, &tao_label_style);
			lv_obj_align(label_info, NULL, LV_ALIGN_IN_TOP_LEFT, 155, 20);
		} else {
			snprintf(header_label_text, sizeof(header_label_text), "%s%s%s%s", header_24g, header_passw0, header_5g, header_passw1);
			lv_label_set_text(label_header, header_label_text);
			lv_obj_add_style(label_header, LV_OBJ_PART_MAIN, &tao_label_style);
			lv_obj_align(label_header, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 190);
			snprintf(info_label_text, sizeof(info_label_text), "%s\n%s\n%s\n%s\n", wifi_data.ssid24g, wifi_data.psk24g, wifi_data.ssid5g, wifi_data.psk5g);
			lv_label_set_text(label_info, info_label_text);
			lv_obj_add_style(label_info, LV_OBJ_PART_MAIN, &tao_label_style);
			lv_obj_align(label_info, NULL, LV_ALIGN_IN_TOP_LEFT, 89, 190);
		}
	} else {
		lv_obj_t * scr = lv_scr_act();
		lv_obj_t * label_header = lv_label_create(scr, NULL);
		char header_label_text[256];
		sprintf(header_label_text, "Go to Settings on\nMission Control to see\nmore details here");
		lv_label_set_text(label_header, header_label_text);
		lv_obj_add_style(label_header, LV_OBJ_PART_MAIN, &tao_label_style);
		if (lcd_settings.orientation) {
			lv_obj_align(label_header, NULL, LV_ALIGN_IN_TOP_LEFT, 155, 94);
		} else {
			lv_label_set_align(label_header, LV_LABEL_ALIGN_CENTER);
			lv_obj_align(label_header, NULL, LV_ALIGN_IN_TOP_MID, 0, 220);
		}
	}
}

void sys_info_display (void)
{
	if (lcd_settings.details) {
		lv_obj_t * scr = lv_scr_act();
		lv_obj_t * label_header = lv_label_create(scr, NULL);
		char header_label_text[256];
		const char* header_imei = "IMEI:\n";
		const char* header_sn = "SN:\n";
		const char* header_mac = "MAC:\n";
		static lv_style_t tao_label_style;
		lv_style_init(&tao_label_style);
		lv_style_set_text_color(&tao_label_style,LV_STATE_DEFAULT,LV_COLOR_BLACK);
		lv_style_set_text_font(&tao_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);
		char imei[256];
		strncpy(imei, read_info_from_file("/tmp/nextivity/imei"), sizeof(imei) - 1);
		char sn[256];
		strncpy(sn, read_info_from_file("/tmp/nextivity/serialNumber"), sizeof(sn) - 1); // the password should be here in the future builds
		char mac[256];
		strncpy(mac, read_info_from_file("/sys/devices/virtual/net/br-lan/address"), sizeof(mac) - 1); // or use /sys/devices/virtual/net/ra0/address instead 
		lv_obj_t * scr_info = lv_scr_act();
		lv_obj_t * label_info = lv_label_create(scr_info, NULL);
		char info_label_text[256];
		
		if (lcd_settings.orientation) {
			snprintf(header_label_text, sizeof(header_label_text), "%s\n%s\n%s\n", header_imei, header_sn, header_mac);
			lv_label_set_text(label_header, header_label_text);
			lv_obj_add_style(label_header, LV_OBJ_PART_MAIN, &tao_label_style);
			lv_obj_align(label_header, NULL, LV_ALIGN_IN_TOP_LEFT, 155, 140);
			
			snprintf(info_label_text, sizeof(info_label_text), "%s\n\n%s\n\n%s\n\n", imei, sn, mac);
			lv_label_set_text(label_info, info_label_text);
			lv_obj_add_style(label_info, LV_OBJ_PART_MAIN, &tao_label_style);
			lv_obj_align(label_info, NULL, LV_ALIGN_IN_TOP_LEFT, 155, 155);
		} else {
			snprintf(header_label_text, sizeof(header_label_text), "%s%s%s", header_imei, header_sn, header_mac);
			lv_label_set_text(label_header, header_label_text);
			lv_obj_add_style(label_header, LV_OBJ_PART_MAIN, &tao_label_style);
			lv_obj_align(label_header, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 260);

			snprintf(info_label_text, sizeof(info_label_text), "%s\n%s\n%s\n", imei, sn, mac);
			lv_label_set_text(label_info, info_label_text);
			lv_obj_add_style(label_info, LV_OBJ_PART_MAIN, &tao_label_style);
			lv_obj_align(label_info, NULL, LV_ALIGN_IN_TOP_LEFT, 53, 260);
		}
	}
}

void read_lcd_settings(struct lcd_settings_t *lcd) 
{
	FILE* fp = fopen("/etc/config/lcd", "r");
	if (fp == NULL) {
		return false;
	}

	char orientation_c[32];
	char details_c[32];
	char worktime_c[32];
	char screen_switch_c[32];

	char line[256];
	char* ptr;
	while (fgets(line, sizeof(line), fp)) {
		ptr = strstr(line, "option orientation ");
		if (ptr != 0) {
			if (sscanf(ptr, "%*[^\']\'%63[^\']\'", orientation_c) == 1) {
				if (strcmp(orientation_c, "landscape") == 0) {
					lcd->orientation = 1;
				} else {
					lcd->orientation = 0;
				}
			}
		}
		ptr = strstr(line, "option details ");
		if (ptr != 0) {
			if (sscanf(ptr, "%*[^\']\'%63[^\']\'", details_c) == 1) {
				if (strcmp(details_c, "full") == 0) {
					lcd->details = 1;
				} else {
					lcd->details = 0;
				}
			}
		}
		ptr = strstr(line, "option worktime ");
		if (ptr != 0) {
			if (sscanf(ptr, "%*[^\']\'%63[^\']\'", worktime_c) == 1) {
				lcd->worktime = atoi(worktime_c);
			}
		}
		ptr = strstr(line, "option screen_switch ");
		if (ptr != 0) {
			if (sscanf(ptr, "%*[^\']\'%63[^\']\'", screen_switch_c) == 1) {
				lcd->next_screen = atoi(screen_switch_c);
			} else {
				lcd->next_screen = 10;
			}
		}
	}

	LV_LOG_INFO("screen orientation: %s, %d", orientation_c, lcd->orientation);
	LV_LOG_INFO("screen details: %s, %d", details_c, lcd->details);
	LV_LOG_INFO("screen worktime: %s, %d", worktime_c, lcd->worktime);
	LV_LOG_INFO("screen switch: %s, %d", screen_switch_c, lcd->next_screen);
	fclose(fp);
	return;
}

void home_network_display(void)
{
	char home_network[256];
	char home_network_label_text[256];
	strncpy(home_network, read_info_from_file("/tmp/nextivity/home_network"), sizeof(home_network) - 1);
	snprintf(home_network_label_text, sizeof(home_network_label_text), "%s", home_network);
	//label style
	static lv_style_t network_label_style;
	lv_style_init(&network_label_style);
	lv_style_set_text_color(&network_label_style,LV_STATE_DEFAULT,LV_COLOR_BLUE);
	if (strlen(home_network_label_text) <= 10) {
		lv_style_set_text_font(&network_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_20);
	} else {
		lv_style_set_text_font(&network_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_10);
	}

	lv_obj_t * scr_network = lv_scr_act();
	lv_obj_clean_style_list(scr_network, LV_OBJ_PART_MAIN);
	lv_obj_t * label_network = lv_label_create(scr_network, NULL);
	lv_label_set_text(label_network, home_network_label_text);
	lv_obj_add_style(label_network, LV_OBJ_PART_MAIN, &network_label_style);
	lv_obj_align(label_network, NULL, LV_ALIGN_IN_TOP_MID, 60, 45);
}

static void wifi_status (int state24g, int state5g)
{	
	lv_obj_t * img = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_size(img, 40, 40);
	static lv_style_t tao_label_style;
	lv_style_init(&tao_label_style);
	lv_style_set_text_color(&tao_label_style,LV_STATE_DEFAULT,LV_COLOR_BLACK);
	lv_style_set_text_font(&tao_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);

	lv_obj_t * scr_wifi = lv_scr_act();
	lv_obj_clean_style_list(scr_wifi, LV_OBJ_PART_MAIN);
	lv_obj_t * label_wifi = lv_label_create(scr_wifi, NULL);
	lv_label_set_text(label_wifi, "Wi-Fi ");

	// When both 2.4g wifi and 5g wifi are disabled, the Lcd displays OFF
	int text_align_corr = 0;
	if((state5g == 1) && (state5g == 1)) {
		lv_img_set_src(img, "/usr/share/mgui/images/placeholder.png");
		if (lcd_settings.orientation) {
			lv_label_set_text(label_wifi, "No\nWi-Fi ");
			text_align_corr = 30;
		} else {
			lv_label_set_text(label_wifi, "No Wi-Fi");
			text_align_corr = 45;
		}
	} else {	
		lv_img_set_src(img, "/usr/share/mgui/images/wifi.png");
	}
	lv_obj_add_style(label_wifi, LV_OBJ_PART_MAIN, &tao_label_style);

	if (lcd_settings.orientation) {
		lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_LEFT, 95, 185);
		lv_label_set_align(label_wifi, LV_LABEL_ALIGN_CENTER);
		lv_obj_align(label_wifi, NULL, LV_ALIGN_IN_TOP_MID, -45, (223 - text_align_corr));
	} else {
		lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_LEFT, 130, 130);
		lv_obj_align(label_wifi, NULL, LV_ALIGN_IN_TOP_LEFT, (175 - text_align_corr), 144);
	}
}

void connect_type_display (void)
{
	lv_obj_t * img_inter = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_size(img_inter, 40,40 );

	static lv_style_t tao_label_style;
	lv_style_init(&tao_label_style);
	lv_style_set_text_color(&tao_label_style,LV_STATE_DEFAULT,LV_COLOR_BLACK);
	lv_style_set_text_font(&tao_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);

	lv_obj_t * scr_inter = lv_scr_act();
	lv_obj_clean_style_list(scr_inter, LV_OBJ_PART_MAIN);
	lv_obj_t * label_inter = lv_label_create(scr_inter, NULL);
	lv_label_set_text(label_inter, "Internet");

	char connect_buf[256];
	int text_align_corr = 0;
	if (read_connecttype_from_file("/tmp/wan_status", connect_buf, sizeof(connect_buf)))
	{
		if (strstr(connect_buf, "Connected") != NULL) {
			lv_img_set_src(img_inter, "/usr/share/mgui/images/inter.png");
		} else if (strstr(connect_buf, "Disconnected") != NULL) {
			lv_img_set_src(img_inter, "/usr/share/mgui/images/placeholder.png");
			if (lcd_settings.orientation) {
				lv_label_set_text(label_inter, "No\nInternet");
				text_align_corr = 30;
			} else {
				lv_label_set_text(label_inter, "No Internet");
				text_align_corr = 45;
			}
		}
	} else {
		lv_img_set_src(img_inter, "/usr/share/mgui/images/placeholder.png");
		if (lcd_settings.orientation) {
			lv_label_set_text(label_inter, "No\nInternet");
			text_align_corr = 30;
		} else {
			lv_label_set_text(label_inter, "No Internet");
			text_align_corr = 45;
		}
	}	

	lv_obj_add_style(label_inter, LV_OBJ_PART_MAIN, &tao_label_style);

	if (lcd_settings.orientation) {
		lv_obj_align(img_inter, NULL, LV_ALIGN_IN_TOP_LEFT, 95, 65);
		lv_label_set_align(label_inter, LV_LABEL_ALIGN_CENTER);
		lv_obj_align(label_inter, NULL, LV_ALIGN_IN_TOP_MID, -45, (106 - text_align_corr));
	} else {
		lv_obj_align(img_inter, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 130);
		lv_obj_align(label_inter, NULL, LV_ALIGN_IN_TOP_LEFT, (55 - text_align_corr), 144);
	}
}

void hupe_display(void)
{
	//Hupe png
	lv_obj_t * img_hupe = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_size(img_hupe, 40,40 );
	

	static lv_style_t tao_label_style;
	lv_style_init(&tao_label_style);
	lv_style_set_text_color(&tao_label_style,LV_STATE_DEFAULT,LV_COLOR_BLACK);
	lv_style_set_text_font(&tao_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);

	lv_obj_t * scr_hupe = lv_scr_act();
	lv_obj_clean_style_list(scr_hupe, LV_OBJ_PART_MAIN);
	lv_obj_t * label_hupe = lv_label_create(scr_hupe, NULL);
	lv_label_set_text(label_hupe, "HPUE");
	lv_obj_add_style(label_hupe, LV_OBJ_PART_MAIN, &tao_label_style);

	int text_align_corr = 0;
    if (read_onoff_from_file("/tmp/nextivity/hpue_onoff")) {
		lv_img_set_src(img_hupe, "/usr/share/mgui/images/hupe.png");
	} else {	
		lv_img_set_src(img_hupe, "/usr/share/mgui/images/placeholder.png");
		if (lcd_settings.orientation) {
			lv_label_set_text(label_hupe, "No\nHPUE ");
			text_align_corr = 30;
		} else {
			lv_label_set_text(label_hupe, "No HPUE");
			text_align_corr = 45;
		}
	}

	if (lcd_settings.orientation) {
		lv_obj_align(img_hupe, NULL, LV_ALIGN_IN_TOP_LEFT, 95, 5);
		lv_label_set_align(label_hupe, LV_LABEL_ALIGN_CENTER);
		lv_obj_align(label_hupe, NULL, LV_ALIGN_IN_TOP_MID, -45 , (46 - text_align_corr));
	} else {
		lv_obj_align(img_hupe, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 82);
		lv_obj_align(label_hupe, NULL, LV_ALIGN_IN_TOP_LEFT, (55 - text_align_corr), 100);
	}
}

void gnss_display(void)
{
	//gnss png
	lv_obj_t * img_gnss = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_size(img_gnss, 40,40 );

	static lv_style_t tao_label_style;
	lv_style_init(&tao_label_style);
	lv_style_set_text_color(&tao_label_style,LV_STATE_DEFAULT,LV_COLOR_BLACK);
	lv_style_set_text_font(&tao_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);

	lv_obj_t * scr_gnss = lv_scr_act();
	lv_obj_clean_style_list(scr_gnss, LV_OBJ_PART_MAIN);
	lv_obj_t * label_gnss = lv_label_create(scr_gnss, NULL);
	lv_label_set_text(label_gnss, "GNSS");
	lv_obj_add_style(label_gnss, LV_OBJ_PART_MAIN, &tao_label_style);

	int text_align_corr = 0;
    if (read_onoff_from_file("/tmp/nextivity/gps_onoff")) {
		lv_img_set_src(img_gnss, "/usr/share/mgui/images/gnss.png");
	} else {	
		lv_img_set_src(img_gnss, "/usr/share/mgui/images/placeholder.png");
		if (lcd_settings.orientation) {
			lv_label_set_text(label_gnss, "No\nGNSS ");
			text_align_corr = 30;
		} else {
			lv_label_set_text(label_gnss, "No GNSS");
			text_align_corr = 45;
		}
	}

	if (lcd_settings.orientation) {
		lv_obj_align(img_gnss, NULL, LV_ALIGN_IN_TOP_LEFT, 95, 125);
		lv_label_set_align(label_gnss, LV_LABEL_ALIGN_CENTER);
		lv_obj_align(label_gnss, NULL, LV_ALIGN_IN_TOP_MID, -45, (166 - text_align_corr));
	} else {	
		lv_obj_align(img_gnss, NULL, LV_ALIGN_IN_TOP_LEFT, 130, 82);
		lv_obj_align(label_gnss, NULL, LV_ALIGN_IN_TOP_LEFT, (175 - text_align_corr), 100);
	}
}

void lines_display()
{
	//Screen split line
	if (lcd_settings.orientation) {
		lv_obj_t * img_line = lv_img_create(lv_scr_act(), NULL);
		lv_img_set_src(img_line, "/usr/share/mgui/images/linetao.png");
		lv_obj_set_size(img_line, 10, 240);
		lv_obj_align(img_line, NULL, LV_ALIGN_IN_TOP_LEFT, 75, 0);

		lv_obj_t * img_line2 = lv_img_create(lv_scr_act(), NULL);
		lv_img_set_src(img_line2, "/usr/share/mgui/images/linetao.png");
		lv_obj_set_size(img_line2, 10, 240);
		lv_obj_align(img_line2, NULL, LV_ALIGN_IN_TOP_LEFT, 145, 0);

		lv_obj_t * img_line3 = lv_img_create(lv_scr_act(), NULL);
		lv_img_set_src(img_line3, "/usr/share/mgui/images/linetest.png");
		lv_obj_set_size(img_line3, 79, 10);
		lv_obj_align(img_line3, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 115);
	} else {	
		lv_obj_t * img_line = lv_img_create(lv_scr_act(), NULL);
		lv_img_set_src(img_line, "/usr/share/mgui/images/linetest.png");
		lv_obj_set_size(img_line, 250,10 );
		lv_obj_align(img_line, NULL, LV_ALIGN_IN_LEFT_MID, 0, -90);

		lv_obj_t * img_line2 = lv_img_create(lv_scr_act(), NULL);
		lv_img_set_src(img_line2, "/usr/share/mgui/images/linetest.png");
		lv_obj_set_size(img_line2, 250,10 );
		lv_obj_align(img_line2, NULL, LV_ALIGN_IN_LEFT_MID, 0, 20);

		lv_obj_t * img_line3 = lv_img_create(lv_scr_act(), NULL);
		lv_img_set_src(img_line3, "/usr/share/mgui/images/linetao.png");
		lv_obj_set_size(img_line3, 10,69 );
		lv_obj_align(img_line3, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	}
}

void qr_display(void)
{
	const char * data[64];
	if (wifi_data.encrypt24g) {
		sprintf(data, "WIFI:T:WPA;S:%s;P:%s;;", wifi_data.ssid24g, wifi_data.psk24g);
	} else {
		sprintf(data, "WIFI:S:%s;;", wifi_data.ssid24g);
	}
	lv_coord_t qr_size = 160;
	if (!lcd_settings.orientation) {
		qr_size = 130;
	}
	
	int xres = fbdev_get_xres();
	int yres = fbdev_get_yres();
	gQrCode.base = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_size(gQrCode.base, qr_size, qr_size);

	lv_style_init(&gQrCode.style);
    lv_style_set_radius(&gQrCode.style, LV_STATE_DEFAULT, 0);
	
	lv_obj_add_style(gQrCode.base, LV_OBJ_PART_MAIN, &gQrCode.style);
	gQrCode.qrcode = lv_qrcode_create(lv_scr_act(), qr_size, LV_COLOR_BLACK, LV_COLOR_WHITE);
	lv_qrcode_update(gQrCode.qrcode, data, strlen(data));
	gQrCode.label = lvgl_create_textbox();

	static lv_style_t tao_label_style;
	lv_style_init(&tao_label_style);
	lv_style_set_text_color(&tao_label_style,LV_STATE_DEFAULT,LV_COLOR_BLACK);
	lv_style_set_text_font(&tao_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_12);

	lv_obj_t * scr_qr = lv_scr_act();
	lv_obj_clean_style_list(scr_qr, LV_OBJ_PART_MAIN);
	lv_obj_t * label_qr = lv_label_create(scr_qr, NULL);
	lv_obj_add_style(label_qr, LV_OBJ_PART_MAIN, &tao_label_style);

	if (lcd_settings.orientation) {
		lv_obj_set_pos(gQrCode.base, 155, 40);
		lv_obj_set_pos(gQrCode.qrcode, 155, 40);
		lv_label_set_text(label_qr, "Scan to connect");
		lv_label_set_align(label_qr, LV_LABEL_ALIGN_CENTER);
		lv_obj_align(label_qr, NULL, LV_ALIGN_IN_TOP_MID, 75, 220);
	} else {	
		lv_obj_set_pos(gQrCode.base, 55, 185);
		lv_obj_set_pos(gQrCode.qrcode, 55, 185);
		lv_label_set_text(label_qr, "Scan\nto\nconnect");
		lv_label_set_align(label_qr, LV_LABEL_ALIGN_CENTER);
		lv_obj_align(label_qr, NULL, LV_ALIGN_IN_TOP_MID, -90, 230);
	}

}

int lvgl_init(void)
{
	/*Linux frame buffer device init*/
	fbdev_init();

	read_lcd_settings(&lcd_settings);

	if (lcd_settings.worktime >= 0) {
		fbdev_power(1);
	}

	/*LittlevGL init*/
	lv_init();
	/*A small buffer for LittlevGL to draw the screen's content*/
	static lv_color_t buf[DISP_BUF_SIZE];
	/*Initialize a descriptor for the buffer*/
	static lv_disp_buf_t disp_buf;
	lv_disp_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

	/*Initialize and register a display driver*/
	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.sw_rotate = 1;
	disp_drv.rotated = lcd_settings.orientation ? LV_DISP_ROT_270 : LV_DISP_ROT_NONE;
	disp_drv.buffer   = &disp_buf;
	disp_drv.flush_cb = fbdev_flush;
	lv_disp_drv_register(&disp_drv);

	lv_png_init();
	lines_display();
	signal_init();
	network_init();
	wifi_update_data();
	wifi_ssid_display();
	sys_info_display();
	connect_type_display();
	// home_network_display(); <-- Removed in the new design
	hupe_display();
	gnss_display();

	static lv_style_t tao_label_style;
	lv_style_init(&tao_label_style);
	lv_style_set_text_color(&tao_label_style,LV_STATE_DEFAULT,LV_COLOR_BLACK);
	lv_style_set_text_font(&tao_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);

	lv_obj_t * scr = lv_scr_act();
	lv_obj_clean_style_list(scr, LV_OBJ_PART_MAIN);
	lv_obj_t * label = lv_label_create(scr, NULL);
	lv_obj_add_style(label, LV_OBJ_PART_MAIN, &tao_label_style);
	lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 150, 90);
	lv_obj_set_style_local_bg_color(scr,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_WHITE);
	return 0;
}

int lvgl_update_info(int screen)
{
	lv_obj_clean(lv_scr_act());
	lines_display();
	signal_init();
	network_init();
	wifi_update_data();
	if (screen == 0) {
		wifi_ssid_display();
		sys_info_display();
	} else {
		qr_display();
	}
	connect_type_display();
	// home_network_display(); <-- Removed in the new design
	hupe_display();
	gnss_display();


	static lv_style_t tao_label_style;
	lv_style_init(&tao_label_style);
	lv_style_set_text_color(&tao_label_style,LV_STATE_DEFAULT,LV_COLOR_BLACK);
	lv_style_set_text_font(&tao_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_14);

	lv_obj_t * scr = lv_scr_act();
	lv_obj_clean_style_list(scr, LV_OBJ_PART_MAIN);
	lv_obj_t * label = lv_label_create(scr, NULL);
	lv_obj_add_style(label, LV_OBJ_PART_MAIN, &tao_label_style);
	lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 150, 90);
	lv_obj_set_style_local_bg_color(scr,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_WHITE);
	return 0;
}


int lvgl_init1(void)
{
    /*LittlevGL init*/
    //lv_init();
	
    /*Linux frame buffer device init*/
    //fbdev_init();
	// fbdev_set_rotated(1); //wpeng add to set LCD rotated,1:Vertical 
    /*A small buffer for LittlevGL to draw the screen's content*/
    #if 0
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    //wpeng add to clear screen
   // lvgl_set_screen_color(0);//black
    /*Initialize and register a display driver*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.rotated= 1;
    
    disp_drv.buffer   = &disp_buf;
    //disp_drv.flush_cb = fbdev_flush;
    lv_disp_drv_register(&disp_drv);
    #endif
	lv_obj_t * scr = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_clean_style_list(scr, LV_OBJ_PART_MAIN);
	lv_obj_set_size(scr, 87, 167);  
  
	lv_obj_align(scr, NULL, LV_ALIGN_IN_TOP_LEFT, 23, -3); 
	#if defined CONFIG_USER_HKM
	lv_obj_set_style_local_bg_color(scr,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_BLACK);
	#else
	lv_obj_set_style_local_bg_color(scr,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_SKYBLUE);
	#endif
    return 0;
}
int lvgl_init2(void)
{
    /*LittlevGL init*/
    //lv_init();

    /*Linux frame buffer device init*/
    ///fbdev_init();
	// fbdev_set_rotated(0); //wpeng add to set LCD rotated
    /*A small buffer for LittlevGL to draw the screen's content*/
    #if 0
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    //wpeng add to clear screen
    //lvgl_set_screen_color(0);//black
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.rotated=0;//1 , 0:Landscape， 1: Vertical 
    
    disp_drv.buffer   = &disp_buf;
    //disp_drv.flush_cb = fbdev_flush;	
    lv_disp_drv_register(&disp_drv);
    #endif
	lv_obj_t * scr = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_size(scr, 200, 200); // 设置对象的大小
	lv_obj_align(scr, NULL, LV_ALIGN_IN_LEFT_MID, -3, 0); 
	#if defined CONFIG_USER_HKM
	lv_obj_set_style_local_bg_color(scr,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_BLACK);
	#else
	lv_obj_set_style_local_bg_color(scr,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_SKYBLUE);
	#endif
    return 0;
}

int lvgl_init3(void)
{
    //wpeng add to clear screen
   // lvgl_set_screen_color(0);//black
	/*static lv_color_t buf[DISP_BUF_SIZE];
	static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);
	lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.rotated=0;//1 , 0:Landscape， 1: Vertical 
    fbdev_set_rotated(disp_drv.rotated); //wpeng add to set LCD rotated
    disp_drv.buffer   = &disp_buf;
	lv_disp_drv_register(&disp_drv);*/
	lv_obj_t * scr2 = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_size(scr2, 200, 200); // 设置对象的大小
	lv_obj_align(scr2, NULL, LV_ALIGN_IN_LEFT_MID, -3, 0); 
	#if defined CONFIG_USER_HKM
	lv_obj_set_style_local_bg_color(scr2,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_BLACK);
	#else
	lv_obj_set_style_local_bg_color(scr2,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_SKYBLUE);
	#endif
    return 0;
}

void lvgl_clearscreen(void)
{
	lv_obj_t * scr = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_clean_style_list(scr, LV_OBJ_PART_MAIN);
	lv_obj_set_size(scr, 87, 167);  
	lv_obj_align(scr, NULL, LV_ALIGN_IN_TOP_LEFT, 23, -3); 
	lv_obj_set_style_local_bg_color(scr,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_BLACK);
}
void lvgl_update(void)
{
	lv_task_handler();
}

void lvgl_get_res(int *w, int *h)
{
	*w = fbdev_get_xres();
	*h = fbdev_get_yres();
}

void lvgl_set_power(int on)
{
	fbdev_power(on);
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
unsigned int custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}

void lvgl_set_screen_color(int color)
{
	fbdev_set_screen(color);
}

int lvgl_worktime(void)
{
	return lcd_settings.worktime;
}

int lvgl_next_sreen(void)
{
	return lcd_settings.next_screen;
}
