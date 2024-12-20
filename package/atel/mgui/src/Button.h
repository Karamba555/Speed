/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/*******************************************************************************
 *
 *  Filename: Button.h
 *
 *  Authors:  Boaz Sommer
 *
 *  Description: An Button class
 *
 *  HISTORY:
 *
 *
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef __BUTTON_H__
#define	__BUTTON_H__

#include	"Object.h"

typedef	void *	BUTTON;
typedef	void	(*ButtonOnClickHandler)(int event, void *data);
typedef struct _ButtonOnClickParams
{
	ButtonOnClickHandler cb;
	void *cb_data;
	int event;
} ButtonOnClickParams;

#ifdef  __cplusplus
    extern  "C" {
#endif
BUTTON	*ButtonInit			(void *dfb);
void	ButtonDeinit			(BUTTON *pt);
void	ButtonSetArray			(BUTTON *pt, const IMAGE_ARRAY_ITEM *array, int size);
void	ButtonSetFromArray		(BUTTON *pt, int key);
void	ButtonSetup			(BUTTON *pt, int lx, int ly);
void	ButtonSetup2			(BUTTON *pt, OBJECT_RECT rect);
void	ButtonDraw			(BUTTON *pt);
void	ButtonSetupOnClick		(BUTTON *pt, ButtonOnClickParams *onClick);
void	ButtonGetGeometry		(BUTTON pt, OBJECT_RECT *rect);
int	ButtonGetWidth			(BUTTON *pt);
int	ButtonGetHeight			(BUTTON *pt);
void	ButtonSetText			(BUTTON *pt, const char *text, const char *font_path);
#ifdef  __cplusplus
	}
#endif

#endif	//__BUTTON_H__

