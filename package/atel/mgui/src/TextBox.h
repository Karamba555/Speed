/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/*******************************************************************************
 *
 *  Filename: TextBox.h
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
#ifndef __TEXTBOX_H__
#define	__TEXTBOX_H__

#include	"Object.h"

typedef	void *	TEXTBOX;

#ifdef  __cplusplus
    extern  "C" {
#endif
TEXTBOX	*TextBoxInit	(void *dfb, int height, int width, const char *font_path);
void	TextBoxDeinit	(TEXTBOX *pt);
void TextBoxSetup(TEXTBOX *pt, const char *text, int x, int y, TEXTALIGN align, unsigned int color);
void TextBoxSetup2(TEXTBOX *pt, const char *text, int x, int y, TEXTALIGN align, unsigned int color);
void	TextBoxSetText	(TEXTBOX *pt, const char *text);
char	*TextBoxGetText(TEXTBOX *pt);
void	TextBoxDraw		(TEXTBOX *pt);
void	TextBoxDraw2		(TEXTBOX *pt);
void	TextBoxSetColor	(TEXTBOX *pt, unsigned int color);
int		TextBoxGetWidth	(TEXTBOX *pt);
#ifdef USE_UGUI
int		TextBoxGetHeight	(TEXTBOX *pt);
#endif

#ifdef  __cplusplus
	}
#endif

#endif	//__TEXTBOX_H__

