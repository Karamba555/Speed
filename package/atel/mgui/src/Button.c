/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/*******************************************************************************
 *
 *  Filename: Button.c
 *
 *  Authors:  Boaz Sommer
 *
 *  Description: A Button class
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
#include 	<string.h>
#include	<pthread.h>
#include	<errno.h>
#include	<signal.h>
#include	<math.h>
#include	"Image.h"
#include	"Button.h"
#include	"TextBox.h"
#include	"mgui_config.h"
#include	"mgui_utils.h"

typedef	struct	S_BUTTON_PRIV
{
	void			*dfb;
	IMAGE				buttonImage;
	TEXTBOX				*text;
	OBJECT_RECT			buttonRect;
	pthread_t			buttonThreadHandle;
	long				eventHandle;
}BUTTON_PRIV;

#if 0
static void	_ButtonEventHandler	(int event, void *params)
{
	ButtonOnClickParams *onClick = (ButtonOnClickParams *)params;

	MGUI_DMSG("Button is clicked\n");

	if (onClick && onClick->cb)
		onClick->cb(event, onClick->cb_data);

}
#endif

void ButtonSetFromArray(BUTTON *pt, int key)
{
	BUTTON_PRIV *button = (BUTTON_PRIV *)pt;

	if (button) {
		ImageSetFromArray(button->buttonImage, key);
		ImageGetGeometry(button->buttonImage, &button->buttonRect);
	}
}

void ButtonSetArray(BUTTON *pt, const IMAGE_ARRAY_ITEM *array, int size)
{
	BUTTON_PRIV *button = (BUTTON_PRIV *)pt;

	if (button) {
		ImageSetArray(button->buttonImage, array, size);
	}
}

BUTTON	*ButtonInit				(void *dfb)
{
	BUTTON_PRIV	*button	=	(BUTTON_PRIV *)calloc(1, sizeof(*button));

	if	(button)
	{
		button->dfb	=	dfb;
		GuiAddObject(button->dfb, button, (ObjectDrawFunc)ButtonDraw);
		button->buttonImage	=	ImageInit(dfb);
	}

	return (BUTTON *)button;
}
void	ButtonDeinit			(BUTTON *pt)
{
	BUTTON_PRIV	*button	=	(BUTTON_PRIV *)pt;

	if	(button)
	{
#if 0
		if	(button->eventHandle)
			DirectFbUnregisterEventHandler(button->dfb, button->eventHandle);
#endif
		if	(button->text)
			TextBoxDeinit(button->text);

		ImageDeinit(button->buttonImage);

		GuiRemoveObject(button->dfb, button);


		free	(button);
	}
}


void ButtonSetup2(BUTTON *pt, OBJECT_RECT rect)
{
	BUTTON_PRIV *button = (BUTTON_PRIV *)pt;
	if (button) {
		button->buttonRect = rect;
		ImageSetup2(button->buttonImage, button->buttonRect);
	}
}

void	ButtonSetup				(BUTTON *pt, int lx, int ly)
{
	BUTTON_PRIV	*button	=	(BUTTON_PRIV *)pt;

	if	(button)
	{
		button->buttonRect.lx 	= lx;
		button->buttonRect.ly 	= ly;
		/*
		button->buttonRect.w	= w;
		button->buttonRect.lx 	= h;
		*/
		MGUI_DMSG("Button rect: (x,y,w,h) = (%d,%d,%d,%d)\n", lx, ly, button->buttonRect.w, button->buttonRect.h);
		ImageSetup(button->buttonImage,
				button->buttonRect.lx,
				button->buttonRect.ly,
				button->buttonRect.w,
				button->buttonRect.h);
	}
}

void	ButtonDraw				(BUTTON *pt)
{
	BUTTON_PRIV	*button	=	(BUTTON_PRIV *)pt;

	if	(button)
	{
		//ImageDraw(button->buttonImage);
		if	(button->text)
			TextBoxDraw(button->text);
	}
	else
		MGUI_DMSG("invalid button\n");
}
void ButtonSetupOnClick(BUTTON *pt, ButtonOnClickParams *onClick)
{
#if 1
	UNUSED(pt);
	UNUSED(onClick);
#else
	BUTTON_PRIV	*button	=	(BUTTON_PRIV *)pt;
	unsigned long mask = EVENT_SET(DIRECTFB_EVT_PRESS) | EVENT_SET(DIRECTFB_EVT_RELEASE);

	if	(button)
	{
		button->eventHandle = DirectFbRegisterEventHandler(button->dfb,
								&button->buttonRect,
								mask ,
								_ButtonEventHandler,
								(void *)onClick);
	}
#endif
}

void	ButtonGetGeometry		(BUTTON pt, OBJECT_RECT *rect)
{
	BUTTON_PRIV	*button	=	(BUTTON_PRIV *)pt;
	if (button)
	{
		ImageGetGeometry(button->buttonImage, rect);
	}
}


int		ButtonGetWidth			(BUTTON *pt)
{
	BUTTON_PRIV	*button	=	(BUTTON_PRIV *)pt;

	if	(button)
		return	(button->buttonRect.w);
	else
		return 0;
}

int		ButtonGetHeight			(BUTTON *pt)
{
	BUTTON_PRIV	*button	=	(BUTTON_PRIV *)pt;

	if	(button)
		return	(button->buttonRect.h);
	else
		return 0;
}

void	ButtonSetText			(BUTTON *pt, const char *text, const char *font_path)
{
	BUTTON_PRIV	*button	=	(BUTTON_PRIV *)pt;
	int height;

	if	(button)
	{
		height = button->buttonRect.h *.45;
		button->text = TextBoxInit(button->dfb, height, -1, font_path);
		TextBoxSetup(button->text, text, button->buttonRect.lx + button->buttonRect.w/2,
				button->buttonRect.ly + (button->buttonRect.h * 1.5 / 2), TA_CENTER, COLOR_BLACK);
	}
}
