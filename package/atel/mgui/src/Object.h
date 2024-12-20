/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/*******************************************************************************
 *
 *  Filename: CDirectFB.h
 *
 *  Authors:  Boaz Sommer
 *
 *  Description: A DirectFB class
 *
 *  HISTORY:
 *
 *
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef __OBJECT_H__
#define	__OBJECT_H__

typedef	void	(*DirectFbEventHandlerCb)(int, void *);
typedef	void	(*ObjectDrawFunc)(void *);

#define	COLOR_GET_RED_FROM_MAP(argb)		(((argb) & 0xFF0000) >> 16)
#define	COLOR_GET_GREEN_FROM_MAP(argb)		(((argb) & 0xFF00) >> 8)
#define	COLOR_GET_BLUE_FROM_MAP(argb)		(((argb) & 0xFF) >> 0)
#define	COLOR_GET_ALPHA_FROM_MAP(argb)		(((argb) & 0xFF000000) >> 24)

#define	COLOR_BLACK	0x00000000
#define	COLOR_RED	0x00FF0000
#define	COLOR_GREEN	0x0000FF00
#define	COLOR_BLUE	0x000000FF
#define	COLOR_WHITE	0xFFFFFFFF

typedef	enum	E_TEXTALIGN
{
	TA_NONE		= 0,
	TA_LEFT 	= 0,
	TA_CENTER 	= 1,
	TA_RIGHT	= 2
}TEXTALIGN;

typedef	struct	_OBJECT_COLOR
{
	unsigned	char	red;
	unsigned	char	green;
	unsigned	char	blue;
	unsigned	char	alpha;
}OBJECT_COLOR;

typedef	struct	_DIRECTFB_REGION
{
	unsigned	int	lx;
	unsigned	int	ly;
	unsigned	int	rx;
	unsigned	int	ry;
}DIRECTFB_REGION;

typedef	struct	_OBJECT_RECT
{
	unsigned	int	lx;
	unsigned	int	ly;
	unsigned	int	w;
	unsigned	int	h;
}OBJECT_RECT;

typedef	struct	_OBJECT_SCREEN
{
	int	width;
	int	height;
}OBJECT_SCREEN;

typedef	struct	_OBJECT_POS
{
	int			x;
	int			y;
	TEXTALIGN	align;
}OBJECT_POS;


typedef	enum	_DIRECTFB_EVENTS
{
	DIRECTFB_EVT_PRESS, /* press */
	DIRECTFB_EVT_RELEASE, /* press */
	DIRECTFB_EVT_RELEASE_OUT_OF_BOUNDS,
	DIRECTFB_EVT_TOUCH, /* press / press and move */
}DIRECTFB_EVENTS;

#define EVENT_SET(x) (1<<x)

#ifdef  __cplusplus
    extern  "C" {
#endif
void		*GuiInit(void);
void		*GuiInit1(void);
void		*GuiInit2(void);
void		*GuiInit3(void);
void		GuiDeinit(void *pt);
void		GuiRefreshScreenObjects(void *pt);
void		GuiGetScreenDim(void *pt, OBJECT_SCREEN *screen);
void		GuiDrawResizeImageFromSurface(void *pt, void *surface, void *blitting_flags, OBJECT_RECT *rect);
void		GuiGetImageGeometry(void *pt, char *path, OBJECT_RECT *rect);
void		GuiGetImageGeometry2(void *pt, char *path, OBJECT_RECT *rect);
void		GuiCreateImageSurface(void* pt, char *path, void **surface, unsigned int *flags);
void		GuiReleaseImageSurface(void *pt, void *surface);
void		GuiDrawText(void *pt, void *h, char *text, OBJECT_COLOR *color, OBJECT_POS *pos);
void		GuiScreenSetPower(void *pt, int on);
void		GuiScreenClear(void *pt, unsigned int color);
void		GuiAddObject(void *pt, void *obj, ObjectDrawFunc obj_draw_func);
void		GuiRemoveObject(void *pt, void *obj);
int			GuiGetTextWidth(void *pt, char *text);
void Guiclearscreen (void);
static inline OBJECT_COLOR to_object_color(unsigned int argb)
{
	OBJECT_COLOR color;

	color.red	=	COLOR_GET_RED_FROM_MAP(argb);
	color.green	=	COLOR_GET_GREEN_FROM_MAP(argb);
	color.blue	=	COLOR_GET_BLUE_FROM_MAP(argb);
	color.alpha	=	COLOR_GET_ALPHA_FROM_MAP(argb);

	return color;
}

#ifdef  __cplusplus
	}
#endif

#endif	//__CDIRECTFB_H__

