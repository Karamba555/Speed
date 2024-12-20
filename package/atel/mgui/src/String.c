/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/*******************************************************************************
 *
 *  Filename: String.c
 *
 *  Authors:  Boaz Sommer
 *
 *  Description: A String class - holds a string
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
#include	<stdio.h>

#include	"String.h"
#include	"mgui_utils.h"

typedef	struct	S_STRING_PRIV
{
	char	*text;
	int		alloc_length;
}STRING_PRIV;


static	void	StringSetLen	(STRING_PRIV *ptp, int len)
{
	char	*tmp_str;
	if	(ptp) {
		if	(ptp->text)
		{
			if	(len > 0)
			{
				tmp_str = strdup(ptp->text);
				free(ptp->text);
				ptp->text = (char *)malloc(len + 1);
				ptp->alloc_length = len;

				//strcpy(ptp->text, tmp_str);
			}
			else
			{
				free(ptp->text);
				ptp->text = NULL;
				ptp->alloc_length = 0;
			}
		}
		else if	(len > 0)
		{
			ptp->text = (char *)malloc(len + 1);
			ptp->alloc_length = len;
		}
	}			
}




STRING	*StringInit	(STRING *pt)
{
	STRING_PRIV	*ptp	=	(STRING_PRIV *)pt;

	ptp = (STRING_PRIV *)calloc(1, sizeof(*ptp));
	ptp->text = NULL;
	ptp->alloc_length	=	0;

	return (STRING *)ptp;
}

void	StringDeinit	(STRING *pt)
{
	STRING_PRIV	*ptp	=	(STRING_PRIV *)pt;
	if (ptp)
	{
		if(ptp->text)
		{
			free(ptp->text);
			ptp->text = NULL;
		}
		if (ptp)
			free(ptp);
		ptp = NULL;
	}
}

void	StringSet		(STRING *pt, const char *s)
{
	STRING_PRIV	*ptp	=	(STRING_PRIV *)pt;
	int	src_len;

	if (ptp)
		if	(s)
		{
			src_len = strlen(s);
			if	(src_len > ptp->alloc_length + 1)
				StringSetLen(ptp, src_len);

			strncpy(ptp->text, s, src_len);
			ptp->text[src_len] = 0;
		}

}
char	*StringGet		(STRING *pt)
{
	STRING_PRIV	*ptp	=	(STRING_PRIV *)pt;

	if (ptp)
		return ptp->text;
	else
		return NULL;
}
