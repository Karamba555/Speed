/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/*******************************************************************************
 *
 *  Filename: TextBox.c
 *
 *  Authors:  Boaz Sommer
 *
 *  Description: A Text Box class
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
#include	<string.h>
#include	"TextBox.h"
#include	"String.h"
#include	"mgui_utils.h"

typedef	struct	S_TEXTBOX_PRIV
{
	void			*dfb;
	STRING				*string;
	OBJECT_COLOR		color;
	OBJECT_POS		pos;
	int					size;
	void *h;
}TEXTBOX_PRIV;

TEXTBOX	*TextBoxInit	(void *dfb, int height, int width, const char *font_path)
{
	TEXTBOX_PRIV	*textbox	=	calloc(1, sizeof(*textbox));
	OBJECT_SCREEN	dim;

	textbox->dfb	=	dfb;
	GuiAddObject(dfb, textbox, (ObjectDrawFunc)TextBoxDraw);
	textbox->string	=	StringInit(0);

	dim.height	=	height;
	dim.width	=	width;
	textbox->h = lvgl_create_textbox();
	return (TEXTBOX *)textbox;
}
void	TextBoxDeinit	(TEXTBOX *pt)
{
	TEXTBOX_PRIV	*textbox	=	(TEXTBOX_PRIV *)pt;
	if	(pt)
	{
		StringDeinit(textbox->string);
		if(textbox->dfb)
			GuiRemoveObject(textbox->dfb, textbox);
		free	(pt);
		pt	=	NULL;
	}
}

char *TextBoxGetText(TEXTBOX *pt)
{
	TEXTBOX_PRIV *textbox = (TEXTBOX_PRIV *)pt;

	if (textbox)
		return StringGet(textbox->string);

	return NULL;
}

void	TextBoxSetText	(TEXTBOX *pt, const char *text)
{
	TEXTBOX_PRIV	*textbox	=	(TEXTBOX_PRIV *)pt;


	if	(textbox)
	{
		if	(text)
		{
			StringSet(textbox->string, text);
		}
	}
}

void TextBoxSetup(TEXTBOX *pt, const char *text, int x, int y, TEXTALIGN align, unsigned int argb)
{
	TEXTBOX_PRIV	*textbox	=	(TEXTBOX_PRIV *)pt;


	if	(textbox)
	{
		if	(text)
		{
			StringSet(textbox->string, text);
		}

		textbox->pos.x = x;
		textbox->pos.y = y;
		textbox->pos.align = align;
		textbox->color = to_object_color(argb);
	}
}
void TextBoxSetup2(TEXTBOX *pt, const char *text, int x, int y, TEXTALIGN align, unsigned int argb)
{
	TEXTBOX_PRIV	*textbox	=	(TEXTBOX_PRIV *)pt;


	if	(textbox)
	{
		if	(text)
		{
			StringSet(textbox->string, text);
		}

		textbox->pos.x = x;
		textbox->pos.y = y;
		textbox->pos.align = align;
		textbox->color = to_object_color(argb);
		printf("%s:%s:%d,x:%d,y:%d,text:%s !!!\n",__FILE__,__FUNCTION__,__LINE__,x,y,text);

	}
}
void	TextBoxDraw2	(TEXTBOX *pt)
{
	TEXTBOX_PRIV	*textbox	=	(TEXTBOX_PRIV *)pt;
	char			*text;

	if	(textbox)
	{
		text	=	StringGet(textbox->string);
		if	(text)
			GuiDrawText2(textbox->dfb, textbox->h, text, &textbox->color, &textbox->pos,textbox->pos.align);
	}
}
void	TextBoxSetColor	(TEXTBOX *pt, unsigned int argb)
{
	TEXTBOX_PRIV	*textbox	=	(TEXTBOX_PRIV *)pt;
	if	(textbox)
	{
		textbox->color = to_object_color(argb);
	}
}

void	TextBoxDraw	(TEXTBOX *pt)
{
	TEXTBOX_PRIV	*textbox	=	(TEXTBOX_PRIV *)pt;
	char			*text;

	if	(textbox)
	{
		text	=	StringGet(textbox->string);
		if	(text)
			GuiDrawText(textbox->dfb, textbox->h, text, &textbox->color, &textbox->pos);
	}
}

int		TextBoxGetWidth	(TEXTBOX *pt)
{
	TEXTBOX_PRIV	*textbox	=	(TEXTBOX_PRIV *)pt;
	int				width = 0;

	if	(textbox)
	{
		width = GuiGetTextWidth(textbox->dfb, StringGet(textbox->string));
	}

	return width;
}

