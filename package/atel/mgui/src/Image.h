/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/*******************************************************************************
 *
 *  Filename: Image.h
 *
 *  Authors:  Boaz Sommer
 *
 *  Description: An Image class
 *
 *  HISTORY:
 *
 *
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef __IMAGE_H__
#define	__IMAGE_H__

#include	"Object.h"
#include "mgui_utils.h"

typedef	void *	IMAGE;

typedef	struct	_IMAGE_ARRAY_ITEM
{
	int	key;
	char	*path;
	void	*image_surface;
	unsigned int image_blitting_flags;
}IMAGE_ARRAY_ITEM;

#ifdef  __cplusplus
    extern  "C" {
#endif
IMAGE	ImageInit			(void *dfb);
void	ImageDeinit			(IMAGE pt);
void	ImageSetup			(IMAGE pt, int lx, int ly, int w, int h);
void	ImageSetup2			(IMAGE pt, OBJECT_RECT rect);
void	ImageDraw			(IMAGE pt);
void	ImageSetArray			(IMAGE pt, const IMAGE_ARRAY_ITEM *array, int size);
void	ImageSetFromArray		(IMAGE pt, int key);
void	ImageSetFromPath		(IMAGE pt, char *resPath);
void	ImageSetFromPath2		(IMAGE pt, char *resPath);
int	ImageGetGeometry		(IMAGE pt, OBJECT_RECT *rect);
#ifdef  __cplusplus
	}
#endif

#endif	//__IMAGE_H__

