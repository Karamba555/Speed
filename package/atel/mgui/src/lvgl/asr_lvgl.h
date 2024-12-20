#ifndef __ASR_LVGL_H__
#define __ASR_LVGL_H__
#include "../Object.h"

#define DBG_MSG_DEBUG 0
#define CONFIG_BUILD_DEBUG 1
#ifdef CONFIG_BUILD_DEBUG
///#define LOGFILE "/dev/console" 
#define LOGFILE "/tmp/lcd_debug"
#else
#undef DBG_MSG_DEBUG
#define LOGFILE "/dev/null" 
#endif


#define MGUI_LOGCAT 1
#define MGUI_DBG  1

#if DBG_MSG_DEBUG
#define DBG_MSG(fmt, arg ... ) { do	\
	{			       \
		FILE *log_fp = fopen(LOGFILE, "a+"); 	\
		fprintf(log_fp,"%s:%d: "fmt, __func__, __LINE__ , ##arg); 			\
		fprintf(log_fp,"\n"); 			\
		fclose(log_fp); 						\
	} while(0); }
#else
#define DBG_MSG(fmt, arg ... )
#endif

void lvgl_obj_del(void *obj);
extern unsigned int lv_task_handler(void);
void* lvgl_create_image(const char *name);
void lvgl_image_show(void *img, int xs, int ys, int w, int h);
void lvgl_image_hide(void *img);

int image_get_size(const char *path, int *w, int *h);
void* lvgl_create_textbox();
int lvgl_textbox_set(void *label, const char *text, OBJECT_COLOR *c, OBJECT_POS *pos);
//int lvgl_textbox_set2(void *label, const char *text, OBJECT_COLOR *c, OBJECT_POS *pos,TEXTALIGN align);

int lvgl_init(void);
int lvgl_init1(void);
int lvgl_init2(void);
int lvgl_init3(void);
void lvgl_update(void);
void lvgl_get_res(int *w, int *h);
void lvgl_set_power(int on);
void lvgl_set_screen_color(int color);

unsigned int custom_tick_get(void);
void lvgl_show_qrcode(char *str);

int lvgl_worktime(void);
int lvgl_update_info(int screen);
int lvgl_next_sreen(void);

#endif
