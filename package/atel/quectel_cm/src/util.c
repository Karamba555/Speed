/******************************************************************************
  @file    util.c
  @brief   some utils for this QCM tool.

  DESCRIPTION
  Connectivity Management Tool for USB network adapter of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2018 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <sys/time.h>
#if defined(__STDC__)
#include <stdarg.h>
#define __V(x)	x
#else
#include <varargs.h>
#define __V(x)	(va_alist) va_dcl
#define const
#define volatile
#endif

#include <syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



#include "QMIThread.h"
/*
 * Use clock_gettime instead of gettimeofday
 * -- Warnning:
 * There is a risk that, the system may sync time at anytime.
 * if gettimeofday may returns just before the sync time point,
 * the funtion pthread_cond_timedwait will failed.
 * -- use clock_gettime to get relative time will avoid the risk.
 */

void setTimespecRelative(struct timespec *p_ts, long long msec)
{
    struct timespec ts;

    /* 
       CLOCK_REALTIME              system time from 1970-1-1
       CLOCK_MONOTONIC             system start time, cannot be changed
       CLOCK_PROCESS_CPUTIME_ID    the process running time
       CLOCK_THREAD_CPUTIME_ID     the thread running time
     */

    clock_gettime(CLOCK_MONOTONIC, &ts);
    p_ts->tv_sec = ts.tv_sec + (msec / 1000);
    p_ts->tv_nsec = ts.tv_nsec + (msec % 1000) * 1000L * 1000L;
}

int pthread_cond_timeout_np(pthread_cond_t *cond, pthread_mutex_t * mutex, unsigned msecs) {
    if (msecs != 0) {
        struct timespec ts;
        setTimespecRelative(&ts, msecs);
        return pthread_cond_timedwait(cond, mutex, &ts);
    } else {
        return pthread_cond_wait(cond, mutex);
    }
}

void cond_setclock_attr(pthread_cond_t *cond, clockid_t clock)
{
    /* set relative time, for pthread_cond_timedwait */
    pthread_condattr_t attr;
    pthread_condattr_init (&attr);
    pthread_condattr_setclock(&attr, clock);
    pthread_cond_init(cond, &attr);
    pthread_condattr_destroy (&attr);
}

const char * get_time(void) {
    static char time_buf[50];
    struct timeval  tv;
    time_t time;
    suseconds_t millitm;
    struct tm *ti;

    gettimeofday (&tv, NULL);

    time= tv.tv_sec;
    millitm = (tv.tv_usec + 500) / 1000;

    if (millitm == 1000) {
        ++time;
        millitm = 0;
    }

    ti = localtime(&time);
    sprintf(time_buf, "%02d-%02d_%02d:%02d:%02d:%03d", ti->tm_mon+1, ti->tm_mday, ti->tm_hour, ti->tm_min, ti->tm_sec, (int)millitm);
    return time_buf;
}

unsigned long clock_msec(void)
{
	struct timespec tm;
	clock_gettime( CLOCK_MONOTONIC, &tm);
	return (unsigned long)(tm.tv_sec*1000 + (tm.tv_nsec/1000000));
}

FILE *logfilefp = NULL;

const int isBigEndian = 1;
#define is_bigendian() ( (*(char*)&isBigEndian) == 0 )

USHORT le16_to_cpu(USHORT v16) {
    USHORT tmp = v16;
    if (is_bigendian()) {
        unsigned char *s = (unsigned char *)(&v16);
        unsigned char *d = (unsigned char *)(&tmp);
        d[0] = s[1];
        d[1] = s[0];
    }
    return tmp;
}

UINT  le32_to_cpu (UINT v32) {
    UINT tmp = v32;
    if (is_bigendian()) {
        unsigned char *s = (unsigned char *)(&v32);
        unsigned char *d = (unsigned char *)(&tmp);
        d[0] = s[3];
        d[1] = s[2];
        d[2] = s[1];
        d[3] = s[0];
    }
    return tmp;
}

#if 0
UINT ql_swap32(UINT v32) {
    UINT tmp = v32;
    {
        unsigned char *s = (unsigned char *)(&v32);
        unsigned char *d = (unsigned char *)(&tmp);
        d[0] = s[3];
        d[1] = s[2];
        d[2] = s[1];
        d[3] = s[0];
    }
    return tmp;
}
#endif

USHORT cpu_to_le16(USHORT v16) {
    USHORT tmp = v16;
    if (is_bigendian()) {
        unsigned char *s = (unsigned char *)(&v16);
        unsigned char *d = (unsigned char *)(&tmp);
        d[0] = s[1];
        d[1] = s[0];
    }
    return tmp;
}

UINT cpu_to_le32 (UINT v32) {
    UINT tmp = v32;
    if (is_bigendian()) {
        unsigned char *s = (unsigned char *)(&v32);
        unsigned char *d = (unsigned char *)(&tmp);
        d[0] = s[3];
        d[1] = s[2];
        d[2] = s[1];
        d[3] = s[0];
    }
    return tmp;
}

void update_resolv_conf(int iptype, const char *ifname, const char *dns1, const char *dns2) {
    const char *dns_file = "/etc/resolv.conf";
    FILE *dns_fp;
    
    #define MAX_DNS 16
    char *dns_info[MAX_DNS];
    char dns_tag[64];
    int dns_match = 0;
    int i;

    snprintf(dns_tag, sizeof(dns_tag), "# IPV%d %s", iptype, ifname);

    for (i = 0; i < MAX_DNS; i++)
        dns_info[i] = NULL;
    
    dns_fp = fopen(dns_file, "r");
    if (dns_fp) {
        char dns_line[256];
        
        i = 0;    
        dns_line[sizeof(dns_line)-1] = '\0';
        
        while((fgets(dns_line, sizeof(dns_line)-1, dns_fp)) != NULL) {
            if ((strlen(dns_line) > 1) && (dns_line[strlen(dns_line) - 1] == '\n'))
                dns_line[strlen(dns_line) - 1] = '\0';
            //dbg_time("%s", dns_line);
            if (strstr(dns_line, dns_tag)) {
                dns_match++;
                continue;
            }
            dns_info[i++] = strdup(dns_line);
            if (i == MAX_DNS)
                break;
        }

        fclose(dns_fp);
    }
    else if (errno != ENOENT) {
        dbg_time("fopen %s fail, errno:%d (%s)", dns_file, errno, strerror(errno));
        return;
    }
    
    if (dns1 == NULL && dns_match == 0)
        return;

    dns_fp = fopen(dns_file, "w");
    if (dns_fp) {
        if (dns1)
            fprintf(dns_fp, "nameserver %s %s\n", dns1, dns_tag);
        if (dns2)
            fprintf(dns_fp, "nameserver %s %s\n", dns2, dns_tag);
        
        for (i = 0; i < MAX_DNS && dns_info[i]; i++)
            fprintf(dns_fp, "%s\n", dns_info[i]);
        fclose(dns_fp);
    }
    else {
        dbg_time("fopen %s fail, errno:%d (%s)", dns_file, errno, strerror(errno));
    }

    for (i = 0; i < MAX_DNS && dns_info[i]; i++)
        free(dns_info[i]);
}


void write_wan_status(const struct __PROFILE *profile, const struct wan_status *node)
{
//    dbg_time("write_wan_status");
    
	FILE *fp = NULL;
#if 0
	char tmpBuf[128] = {0};

	if ( (fp = fopen("/tmp/wan_status", "w+")) != NULL ) {
		sprintf(tmpBuf, "SIMStatus:%s;\n", node.simstatus);
		fwrite(tmpBuf, strlen(tmpBuf), 1, fp );
		sprintf(tmpBuf, "DataCap:%s;\n", node.datacap);
		fwrite(tmpBuf, strlen(tmpBuf), 1, fp );
		sprintf(tmpBuf, "Udhcpc:%s;\n", node.udhcpc);
		fwrite(tmpBuf, strlen(tmpBuf), 1, fp );
		fclose(fp);
	}
#else	
    char *ip = NULL;
    
    if (profile && profile->ipv4.Address) {
      struct sockaddr_in sa;
      sa.sin_addr.s_addr = htonl(profile->ipv4.Address);
    
      ip = inet_ntoa(sa.sin_addr);
    }
    
    struct {
      const char *name; 
      const char *value;
    } map_s[] = {
        { "ICCID_s", node->iccid },
        { "IMEI_s", node->imei },
        { "Apn_s",   profile->apn },
        { "PdpIpAddr_s", ip },
        { "HnName_s", node->home },
        { "Sn_s", node->serialNumber },
        //{ "", node->linuxVersion },
        { "SwVersion_s", node->softwareVersion },
        { "LanIpAddr_s", node->lanip },
        { "MCLBV_s",     node->lteband },
        { NULL, NULL }
    };
    
    struct {
      const char *name; 
      int value;
    } map_i[] = {
        { "ScMcc_i",  node->mcc }, 
        { "ScMnc_i",  node->mnc }, 
        { "ScRssi_i", node->rssi },
        { "ScRsrq_i", node->rsrq },
        { "ScRsrp_i", node->rsrp },
        { "ScFb_i",   node->band },
        { "ScId_i",   node->scid },
        { "ScPId_i",  node->scpid },
        { NULL, -1 }
    };
    
    struct {
      const char *name;
      float value;
    } map_f[] = {
      { "ScSinr_f", node->sinr },
      { "QmiRxC0RxPwr_f", node->c0rxpower },
      { "QmiRxC0Ecio_f",  node->c0ecio    },
      { "QmiRxC0Rsrp_f",  node->c0rsrp    },   
      { "QmiRxC0Phase_f", node->c0phase   },   
      { "QmiRxC1RxPwr_f", node->c1rxpower },
      { "QmiRxC1Ecio_f",  node->c1ecio    },
      { "QmiRxC1Rsrp_f",  node->c1rsrp    },   
      { "QmiRxC1Phase_f", node->c1phase   },
      { "QmiTxPwr_f",     node->txpower   },
      { NULL, NAN }
    };
    
	
	if ((fp = fopen("/tmp/nextivity/modemstatus.json", "w+")) != NULL ) {
      int now = time(NULL); 
      size_t count = 0;
      size_t items = 0;
        
	  fprintf(fp, "{\"time\": %d,\"status\":[", now);

      while (map_s[count].name) {
        if (map_s[count].value && strlen(map_s[count].value)) {
          fprintf(fp, "%s{\"name\":\"%s\",\"value\":\"%s\"}", 
            items ? "," : "",
            map_s[count].name, 
            map_s[count].value);
            items++;
        }
        
        count++;
      }
      
      count = 0;
      
      while (map_i[count].name) {
        if (map_i[count].value != -1) {
          fprintf(fp, "%s{\"name\":\"%s\",\"value\":%d}", 
            items ? "," : "",
            map_i[count].name, 
            map_i[count].value);
          items++;
        }
        
        count++;
      }
      
      count = 0;
      
      while (map_f[count].name) {
        if (map_f[count].value == map_f[count].value) { // NaN
          fprintf(fp, "%s{\"name\":\"%s\",\"value\":%0.1f}", 
            items ? "," : "",
            map_f[count].name, 
            map_f[count].value);
          items++;
        }
        
        count++;
      }
      
      
      fprintf(fp, "]}");
      
      fclose(fp);
    }
#endif
}
