/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_utils.h
 *
 *  Authors:  Tomer Eliyahu
 *
 *  Description: mgui interface to ubus
 *
 *  HISTORY:
 *   Nov 23, 2014 - Initial Version
 *
 *  Notes:
 *
 ******************************************************************************/

#ifndef	__GUI_UTILS_H__
#define	__GUI_UTILS_H__

#include "mgui_config.h"
#include "user_conf.h"
#include <time.h>



#include "lvgl/asr_lvgl.h"
#include <stdio.h>

#define MGUI_LOG_TAG	"MGUI"
#define LOG_BUF_SIZE	1024

#define TRUE		(1)
#define FALSE		(0)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_A) (sizeof(_A) / sizeof((_A)[0]))
#endif /* ARRAY_SIZE */

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

#define UNUSED(x)	((void)(x))

#if 0 //def MGUI_LOGCAT
#include <log.h>
#define MGUI_DMSG(fmt, args...)	RDBGMSG("%s:%d: "fmt, __func__, __LINE__ , ##args);DBG_MSG("%s:%d: "fmt, __func__, __LINE__ , ##args)
#define MGUI_EMSG(fmt, args...)	RERRMSG("%s:%d: "fmt, __func__,__LINE__ , ##args);DBG_MSG("%s:%d: "fmt, __func__, __LINE__ , ##args)
#define MGUI_WMSG(fmt, args...)	RWARNMSG("%s:%d: "fmt, __func__,__LINE__,  ##args);DBG_MSG("%s:%d: "fmt, __func__, __LINE__ , ##args)
#define MGUI_IMSG(fmt, args...)	RDPRINTF("%s:%d: "fmt, __func__,__LINE__,  ##args);DBG_MSG("%s:%d: "fmt, __func__, __LINE__ , ##args)

#define MASSERT(cond, args...) \
	do {\
		if (!(cond)) { \
			RERRMSG("%s:%d: ASSERT !!!\n", __FILE__,__LINE__, ##args);\
			fprintf(stderr, "%s:%d: ASSERT !!!\n", __FILE__,__LINE__);\
			raise(SIGUSR1);\
		}\
	} while(0);

#define MGUI_LOG_INIT()		set_service_log_tag(MGUI_LOG_TAG);
//#define MGUI_DEBUG			RDBGMSG("%s:%d: HERE\n", __func__, __LINE__);
#else
//#define MGUI_DEBUG			__mgui_log_print("%s:%d: HERE\n", __func__, __LINE__);
#define MGUI_DMSG(fmt, args...)	__mgui_log_print("%s:%d: DEBUG: "fmt, __func__, __LINE__, ##args)
#define MGUI_EMSG(fmt, args...)	__mgui_log_print("%s:%d: ERROR: "fmt, __func__, __LINE__, ##args)
#define MGUI_WMSG(fmt, args...)	__mgui_log_print("%s:%d: WARNING: "fmt, __func__, __LINE__, ##args)
#define MGUI_IMSG(fmt, args...)	__mgui_log_print("%s:%d: INFO: "fmt, __func__, __LINE__, ##args)
#define MASSERT(cond, args...)\
	do { \
		if (!(cond)) { \
			__mgui_log_print("%s:%d: ASSERT !!!\n", __FILE__,__LINE__, ##args); \
			exit(1); \
		} \
	} while (0);
#define MGUI_LOG_INIT()
#endif

int __mgui_log_print(const char *fmt, ...);


static inline void set_time_string(char *time_str, const char *format, size_t size)
{
	time_t current_time;
	struct tm *time_info;

	time(&current_time);
	time_info = localtime(&current_time);
	strftime(time_str, size, format, time_info);
}

#endif	//__GUI_UTILS_H__
