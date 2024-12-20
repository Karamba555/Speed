/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/*******************************************************************************
 *
 *  Filename: String.h
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
#ifndef __STRING_H__
#define	__STRING_H__


typedef	void *	STRING;

#ifdef  __cplusplus
    extern  "C" {
#endif
STRING	*StringInit		(STRING *pt);
void	StringDeinit	(STRING *pt);
void	StringSet		(STRING *pt, const char *s);
char	*StringGet		(STRING *pt);

#ifdef  __cplusplus
	}
#endif

#endif	//__STRING_H__

