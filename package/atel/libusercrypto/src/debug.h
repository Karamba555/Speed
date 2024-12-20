#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "user_conf.h"
#define DEBUG_FLAG 0

#ifdef CONFIG_BUILD_RELEASE 
#define LOGFILE "/dev/null" 
#else
#define LOGFILE "/dev/console" 
#endif
#if DEBUG_FLAG
#define LIBDBG_MSG(fmt, arg...)	do {	FILE *log_fp = fopen(LOGFILE, "w+"); \
    fprintf(log_fp,fmt, ##arg); \
    fclose(log_fp); \
} while(0)
#else

#define LIBDBG_MSG(fmt, arg...)  //

#endif

