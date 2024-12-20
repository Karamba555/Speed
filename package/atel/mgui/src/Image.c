/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/*******************************************************************************
 *
 *  Filename: Line.c
 *
 *  Authors:  Boaz Sommer
 *
 *  Description: A Line class
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

#include	"mgui_utils.h"
#include	"Image.h"

typedef	struct	_IMAGE_ARRAY
{
	int					num_items;
	int					image_index_to_display;
	IMAGE_ARRAY_ITEM	*image_array_items;
}IMAGE_ARRAY;

typedef	struct	S_IMAGE_PRIV
{
	void			*dfb;
	IMAGE_ARRAY			image_array;
	IMAGE_ARRAY_ITEM	single_image_path;
	OBJECT_RECT		region;
}IMAGE_PRIV;

static	char	*_ImageGetPath(IMAGE_PRIV *image)
{
	char	*path = NULL;

	if	(image->single_image_path.path)
		path	=	image->single_image_path.path;
	else if	(image->image_array.image_index_to_display != -1)
		path	=	image->image_array.image_array_items[image->image_array.image_index_to_display].path;

	return path;
}

static	void	_ImageFreeArray	(IMAGE_PRIV *image)
{
	int	i, num = image->image_array.num_items;

	for (i = 0; i < num; i++)
	{
		if	(image->image_array.image_array_items[i].path != NULL)
		{
			free(image->image_array.image_array_items[i].path);
			GuiReleaseImageSurface(image->dfb, image->image_array.image_array_items[i].image_surface);
		}
	}

	free(image->image_array.image_array_items);
	image->image_array.image_array_items = NULL;
	image->image_array.num_items	=	0;
	image->image_array.image_index_to_display	=	-1;
}


IMAGE	ImageInit	(void *dfb)
{
	IMAGE_PRIV	*image	=	calloc(1, sizeof(*image));

	image->dfb	=	dfb;
	GuiAddObject(image->dfb, image, (ObjectDrawFunc)ImageDraw);
	image->image_array.image_index_to_display	=	-1;
	image->single_image_path.path = NULL;
	image->image_array.num_items	=	0;

	return (IMAGE)image;
}

void	ImageDeinit	(IMAGE pt)
{
	IMAGE_PRIV	*image	=	(IMAGE_PRIV *)pt;

	if	(image)
	{
		_ImageFreeArray(image);
		if	(image->single_image_path.path)
		{
				GuiReleaseImageSurface(image->dfb, image->single_image_path.image_surface);
			free	(image->single_image_path.path);
		}
			GuiRemoveObject(image->dfb, image);
		free	(image);
		image	=	NULL;
	}
}


void	ImageDraw	(IMAGE pt)
{
	IMAGE_PRIV	*image	=	(IMAGE_PRIV *)pt;
	IMAGE_ARRAY_ITEM	*image_array_item = NULL;
	if	(image)
	{
		//MGUI_DMSG("image_index_to_display = %d\n", image->image_array.image_index_to_display);
		if	(image->image_array.image_index_to_display >= 0)
			image_array_item	= &image->image_array.image_array_items[image->image_array.image_index_to_display];
		else if	(image->single_image_path.path)
			image_array_item	= &image->single_image_path;
		//MGUI_DMSG("image_array_item = %04X\n", (unsigned long)image_array_item);
		if	(image_array_item) {
			//DirectFbDrawImageFromSurface	(image->dfb, image_array_item->image_surface, image->region.lx, image->region.ly);
			/*MGUI_DMSG("ptr=%p, key=%d, path=%s, surface=%p, flags=0x%08x, region=[h:%d w:%d:, lx:%d, ly:%d,\n",
				image_array_item,
				image_array_item->key, image_array_item->path,
				image_array_item->image_surface,
				image_array_item->image_blitting_flags,
				image->region.h, image->region.w, image->region.lx, image->region.ly);*/
			GuiDrawResizeImageFromSurface(image->dfb,
					image_array_item->image_surface,
					&image_array_item->image_blitting_flags,
					&image->region);
		}
	}

#if 0
	IMAGE_PRIV	*image	=	(IMAGE_PRIV *)pt;
	char		*path = NULL;
	if	(image)
	{
		path	=	_ImageGetPath(image);
		if	(path)
		{
			MGUI_DMSG("path = %s\n", path);
			DirectFbDrawImage(image->dfb, path, &image->region);
		}
	}
#endif
}


void ImageSetup2(IMAGE pt, OBJECT_RECT rect)
{
	IMAGE_PRIV *image = (IMAGE_PRIV *)pt;
	if (image) {
		//lv_img_set_angle(image, 45);
		image->region = rect;
	}
}

void	ImageSetup				(IMAGE pt, int lx, int ly, int w, int h)
{
	IMAGE_PRIV	*image	=	(IMAGE_PRIV *)pt;
	if	(image)
	{
		image->region.lx	=	lx;
		image->region.ly	=	ly;
		image->region.w		=	w;
		image->region.h		=	h;
	}
}

void	ImageSetFromPath	(IMAGE pt, char *resPath)
{
	IMAGE_PRIV	*image	=	(IMAGE_PRIV *)pt;

	if	(image && resPath)
	{
		MGUI_DMSG("image path = %s\n", resPath);
		image->single_image_path.path	=	strdup(resPath);
		GuiCreateImageSurface(image->dfb, resPath,
				&image->single_image_path.image_surface,
				&image->single_image_path.image_blitting_flags);

		GuiGetImageGeometry(image->dfb, resPath, &image->region);
		MGUI_DMSG("image Width x Height = %d x %d\n", image->region.w, image->region.h);
	}
}
void	ImageSetFromPath2	(IMAGE pt, char *resPath)
{
	IMAGE_PRIV	*image	=	(IMAGE_PRIV *)pt;

	if	(image && resPath)
	{
		MGUI_DMSG("image path = %s\n", resPath);
		image->single_image_path.path	=	strdup(resPath);
		GuiCreateImageSurface(image->dfb, resPath,
				&image->single_image_path.image_surface,
				&image->single_image_path.image_blitting_flags);

		GuiGetImageGeometry2(image->dfb, resPath, &image->region);
		MGUI_DMSG("image Width x Height = %d x %d\n", image->region.w, image->region.h);
	}
}

int	ImageGetGeometry		(IMAGE pt, OBJECT_RECT *rect)
{
	IMAGE_PRIV	*image	=	(IMAGE_PRIV *)pt;
	int bRet = 0;
	char	*path;
	if	(image)
	{
		path	=	_ImageGetPath(image);
		DBG_MSG("%s L%d****path=%s***\n",__FUNCTION__,__LINE__,path);
		if	(path)
		{
			GuiGetImageGeometry(image->dfb, path, rect);
		}
		else{
			bRet = -1;
		}
	}
	return bRet;
}

void	ImageSetFromArray		(IMAGE pt, int key)
{
	IMAGE_PRIV	*image	=	(IMAGE_PRIV *)pt;
	int			i;
	if	(image)
	{
		for	(i = 0; i < image->image_array.num_items; i++)
		{
			if	(key == image->image_array.image_array_items[i].key)
			{
				image->image_array.image_index_to_display = i;
				break;
			}
		}
	}
}

void	ImageSetArray	(IMAGE pt, const IMAGE_ARRAY_ITEM *array, int size)
{
	IMAGE_PRIV	*image	=	(IMAGE_PRIV *)pt;
	int			i;

	//MGUI_DMSG("image = %08X\n", (unsigned long )image);

	if	(image)
	{
		if	(image->image_array.image_array_items != NULL)
		{
			MGUI_DMSG("Freeing image array\n");
			_ImageFreeArray(image);
		}

		image->image_array.image_array_items	=	calloc(size, sizeof(IMAGE_ARRAY_ITEM));
		//MGUI_DMSG("image_array_items allocated, address = %08X, size = %d\n", (unsigned long)image->image_array.image_array_items,size);
		for	(i = 0; i < size; i++)
		{
			image->image_array.image_array_items[i].key = array[i].key;
			image->image_array.image_array_items[i].path = strdup(array[i].path);
			GuiCreateImageSurface(image->dfb,
						image->image_array.image_array_items[i].path,
						&image->image_array.image_array_items[i].image_surface,
						&image->image_array.image_array_items[i].image_blitting_flags);
		}
		image->image_array.num_items	=	size;
	}

	//MGUI_DMSG("Done.\n");
}

