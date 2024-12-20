/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/*******************************************************************************
 *
 *  Filename: Point.c
 *
 *  Authors:  Boaz Sommer
 *
 *  Description: A Point class - holds two dimentional point (x,y)
 *
 *  HISTORY:
 *
 *
 *
 *  Notes:
 *
 ******************************************************************************/
#include	<stdarg.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<pthread.h>
#include	<errno.h>
#include	<libubox/list.h>
#include	"Object.h"
#include	"mgui_config.h"
#include	"mgui_utils.h"

typedef	struct	S_EVENT_HANDLERS_LIST
{
	struct	S_EVENT_HANDLERS_LIST 	*next;
	struct	S_EVENT_HANDLERS_LIST 	*prev;
	OBJECT_RECT			area;
	unsigned long			mask; /* events mask */
	DirectFbEventHandlerCb		handler;
	void				*param;
}EVENT_HANDLERS_LIST;


typedef struct	S_OBJECTS_LIST
{
	struct	list_head	list;
	void			*obj;
	ObjectDrawFunc		draw_method;
}OBJECTS_LIST;

typedef struct	S_GUI_PRIV
{
	OBJECT_SCREEN 		screen;
	struct	list_head		obj_list_head;
} GUI_PRIV;

void GuiRefreshScreenObjects(void *pt)
{
	OBJECTS_LIST *entry;
	GUI_PRIV *p = (GUI_PRIV *)pt;
	struct	list_head *ptr;
	if (!p) {
		DBG_MSG("pt is null!!!\n");
		return NULL;
	}
	list_for_each(ptr, &p->obj_list_head) {
		if(ptr)
			entry = list_entry(ptr, OBJECTS_LIST, list);
		//MGUI_DMSG("calling draw method of object %08X\n", entry->obj);
		if (entry && entry->draw_method && entry->obj)
			entry->draw_method(entry->obj);
	}
	DBG_MSG("LCD: width:%d,height:%d!!!\n",p->screen.width, p->screen.height);
	lvgl_update();
}

void *GuiInit (void)
{
	GUI_PRIV	*p;
	p = (GUI_PRIV *)calloc(1, sizeof(*p));
	
	if (!p) {
		MGUI_EMSG("Out of memory!!!\n");
		return NULL;
	}

	INIT_LIST_HEAD(&p->obj_list_head);

	lvgl_init();
	lvgl_get_res(&p->screen.width, &p->screen.height);
	DBG_MSG("LCD: width:%d,height:%d!!!\n",p->screen.width, p->screen.height);
	return (void *)p;
}
void *GuiInit1 (void)
{
	GUI_PRIV	*p;
	p = (GUI_PRIV *)calloc(1, sizeof(*p));
	if (!p) {
		MGUI_EMSG("Out of memory!!!\n");
		return NULL;
	}
	INIT_LIST_HEAD(&p->obj_list_head);
	lvgl_init1();
	lvgl_get_res(&p->screen.width, &p->screen.height);
	return (void *)p;
}
void *GuiInit2 (void)
{
	GUI_PRIV	*p;
	p = (GUI_PRIV *)calloc(1, sizeof(*p));

	if (!p) {
		MGUI_EMSG("Out of memory!!!\n");
		return NULL;
	}

	INIT_LIST_HEAD(&p->obj_list_head);

	lvgl_init2();
	lvgl_get_res(&p->screen.width, &p->screen.height);
	return (void *)p;
}
void *GuiInit3 (void)
{
	GUI_PRIV	*p;
	p = (GUI_PRIV *)calloc(1, sizeof(*p));

	if (!p) {
		MGUI_EMSG("Out of memory!!!\n");
		return NULL;
	}
	INIT_LIST_HEAD(&p->obj_list_head);
	lvgl_init3();
	lvgl_get_res(&p->screen.width, &p->screen.height);
	return (void *)p;
}
void GuiDeinit(void *pt)
{
	GUI_PRIV *p = (GUI_PRIV *)pt;

	if (p) {
		// TODO
		free(p);
	}
}
void Guiclearscreen (void)
{
	lvgl_clearscreen();
}
void GuiGetScreenDim (void *pt, OBJECT_SCREEN *screen)
{
	GUI_PRIV *p = (GUI_PRIV *)pt;

	if (p) {
		screen->width = p->screen.width;
		screen->height = p->screen.height;
	}
}

void GuiDrawResizeImageFromSurface(void *pt, void *surface,
					 void *blitting_flags,
					 OBJECT_RECT *rect)
{
	GUI_PRIV *p = (GUI_PRIV *)pt;
#if 0
	if	(image->image_array.image_index_to_display >= 0) {
				int i;
				for (i = 0; i < image->image_array.num_items; i++)
					lvgl_image_hide(image->image_array.image_array_items[i].image_surface);
			}
			lvgl_image_draw(image_array_item->image_surface, image->lx, image->ly, image->w, image->h);
#endif
	UNUSED(blitting_flags);
	if (p)
		lvgl_image_show(surface, rect->lx, rect->ly, rect->w, rect->h);
}

void GuiReleaseImageSurface (void *pt, void *surface)
{
	UNUSED(pt);
	if(surface)
	lvgl_obj_del(surface);
}

void GuiCreateImageSurface(void *pt, char *path, void **surface, unsigned int *flags)
{
	GUI_PRIV	*p	=	(GUI_PRIV *)pt;
	UNUSED(flags);

	if	(p)
		*surface = lvgl_create_image(path);
}

void GuiGetImageGeometry(void *pt, char *path, OBJECT_RECT *rect)
{
	GUI_PRIV *p = (GUI_PRIV *)pt;
	if (p) {
		int w = 0, h = 0;
		image_get_size(path, &w, &h);
		//DBG_MSG("%s L%d****w=%d>>>h=%d>>>>>>>>>>>**\n",__FUNCTION__,__LINE__,w,h);
		rect->lx = 26;
		rect->ly = 0;
		rect->w = w;
		rect->h = h;
	}
}
void GuiGetImageGeometry2(void *pt, char *path, OBJECT_RECT *rect)
{
	GUI_PRIV *p = (GUI_PRIV *)pt;
	if (p) {
		int w = 0, h = 0;
		image_get_size(path, &w, &h);
		DBG_MSG("%s L%d****w=%d>>>h=%d>>>>>>>>>>>**\n",__FUNCTION__,__LINE__,w,h);
		rect->lx = 0;
		rect->ly = 26;
		rect->w = w;
		rect->h = h;
	}
}

void GuiDrawText(void *pt, void *h, char *text, OBJECT_COLOR *color, OBJECT_POS *pos)
{
	GUI_PRIV *p = (GUI_PRIV *)pt;
	if (p)
		lvgl_textbox_set(h, text, color, pos);
}
void GuiDrawText2(void *pt, void *h, char *text, OBJECT_COLOR *color, OBJECT_POS *pos, TEXTALIGN align)
{
	GUI_PRIV *p = (GUI_PRIV *)pt;
	if (p)
		lvgl_textbox_simpletext(h, text, color, pos,align);
}
int GuiGetTextWidth(void *pt, char *text)
{
	UNUSED(pt);
	UNUSED(text);
	return 0;
}

void GuiScreenClear(void *pt, unsigned int argb)
{
	GUI_PRIV *p = (GUI_PRIV *)pt;
	OBJECT_COLOR color = to_object_color(argb);
	UNUSED(p);
	UNUSED(color);
}

void GuiScreenSetPower(void *pt, int on)
{
	GUI_PRIV *p = (GUI_PRIV *)pt;
	if (p)
		lvgl_set_power(on);
}

void GuiAddObject(void *pt, void *obj, ObjectDrawFunc obj_draw_func)
{
	OBJECTS_LIST *new_object;
	GUI_PRIV *p = (GUI_PRIV *)pt;

	if (p) {
		new_object = (OBJECTS_LIST *)calloc(1, sizeof(*new_object));
		new_object->obj	= obj;
		new_object->draw_method	= obj_draw_func;
		list_add_tail(&new_object->list, &p->obj_list_head);
	}
}

void GuiRemoveObject(void *pt, void *obj)
{
	OBJECTS_LIST *entry;
	struct list_head *ptr;
	GUI_PRIV *p = (GUI_PRIV *)pt;

	if (p) {
		list_for_each(ptr, &p->obj_list_head) {
			entry = list_entry(ptr, OBJECTS_LIST, list);
			if (entry && entry->obj == obj) {
				list_del(&entry->list);
				free(entry);
				return;
			}
		}
	}
}
