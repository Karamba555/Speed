/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_sms.c
 *
 *  Authors:  Tomer Eliyahu
 *
 *  Description: MGUI sms / sms interface
 *
 *  HISTORY:
 *
 *   Jun 14, 2015 - Initial Version
 *
 *  Notes:
 *
 ******************************************************************************/

/******************************************************************************
 *   Include files
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#include "mgui_utils.h"
#include "mgui_ubus.h"
#include "mgui_icons.h"
#include "mgui_sms.h"
#include "mgui.h"

extern char g_sms_unread[];
extern char g_sms_tip[];
extern char g_sms_time[];
extern char g_sms_date[];
extern char g_sms_transfer[];
extern char g_sms_info[];
/******************************************************************************
 *   Structures
 ******************************************************************************/
enum {
	ATTR_SMS,
	ATTR_NODE_LIST,
};
enum {
	ATTR_S1,
	ATTR_ID,
	ATTR_ADDRESS,
	ATTR_CONTACT_ID,
	ATTR_DATE,
	ATTR_PROTOCOL,
	ATTR_TYPE,
	ATTR_READ,
	ATTR_STATUS,
	ATTR_LOCATION,
	ATTR_BODY,
};
/******************************************************************************
 *   Definitions
 ******************************************************************************/

#define SMS_UBUS_ID	"sms"
#define SMS_UBUS_REQUEST_UNREAD_SMS_BODY	"read_unread_sms"
#define SMSKEY_DEV "/tmp/mgui_sms_action"

extern unsigned int nowTime,lastTime,interval;
//#define to_ubus_subscriber(obj) container_of(obj, struct ubus_subscriber, obj)
//#define to_sms_context(obj) container_of(to_ubus_subscriber(obj), struct mgui_sms_context, sms_ubus_subscriber)

/******************************************************************************
 *   Globals
 ******************************************************************************/
static const struct blobmsg_policy sms_info_attr_policy[] = {
	[ATTR_SMS] = {.name = "sms", .type = BLOBMSG_TYPE_TABLE},
	[ATTR_NODE_LIST] = {.name = "node_list", .type = BLOBMSG_TYPE_TABLE},
};

static const struct blobmsg_policy sms_s1_attr_policy[] = {
	[ATTR_S1] = {.name = "s1", .type = BLOBMSG_TYPE_TABLE},
	[ATTR_ID] = {.name = "id", .type = BLOBMSG_TYPE_STRING},
	[ATTR_ADDRESS] = {.name = "address", .type = BLOBMSG_TYPE_STRING},
	[ATTR_CONTACT_ID] = {.name = "contact_id", .type = BLOBMSG_TYPE_STRING},
	[ATTR_DATE] = {.name = "date", .type = BLOBMSG_TYPE_STRING},
	[ATTR_PROTOCOL] = {.name = "protocol", .type = BLOBMSG_TYPE_STRING},
	[ATTR_TYPE] = {.name = "type", .type = BLOBMSG_TYPE_STRING},
	[ATTR_READ] = {.name = "read", .type = BLOBMSG_TYPE_STRING},
	[ATTR_STATUS] = {.name = "status", .type = BLOBMSG_TYPE_STRING},
	[ATTR_LOCATION] = {.name = "location", .type = BLOBMSG_TYPE_STRING},
	[ATTR_BODY] = {.name = "body", .type = BLOBMSG_TYPE_STRING},
};
static	struct blob_buf b;

static void sms_unread_body_complete_cb(struct ubus_request *req, int ret)
{
	DBG_MSG("sms_unread_body_complete_cb finish\n");
}
char* UniDecode(const char* str) {
    int len = strlen(str);
    char* decodedStr = (char*)malloc((len / 4) + 1);
    int i, j;
    for (i = 0, j = 0; i < len; i += 4, j++) {
        char hex[5];
        strncpy(hex, str + i, 4);
        hex[4] = '\0';
		if(!strcmp(hex,"000a"))
		{
			strcpy(hex,"0020");
		}
        int decimal = strtol(hex, NULL, 16);
        decodedStr[j] = (char)decimal;
		
    }
    decodedStr[j] = '\0';
    return decodedStr;
}

static void sms_unread_body_data_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct mgui_sms_context *sms = (struct mgui_sms_context *)req->priv;
	struct blob_attr *tb[ARRAY_SIZE(sms_info_attr_policy)];
	struct blob_attr *tb2g[ARRAY_SIZE(sms_s1_attr_policy)];
	int ret;
	char* token;
	char* delimiter = ",";
    char* rest = (char*)malloc(100 * sizeof(char));
    char* date = (char*)malloc(100 * sizeof(char));
    char* time = (char*)malloc(100 * sizeof(char));
	char* body = (char*)malloc(1024 * sizeof(char));
	int i=0;
	char transfer_buf[64],date_buf[64],time_buf[64],info_buf[4096];

	MASSERT(sms);

	ret =  blobmsg_parse(sms_info_attr_policy, ARRAY_SIZE(sms_info_attr_policy),
			     tb, blob_data(msg), blob_len(msg));

	if (ret){
		DBG_MSG("%s L%d ret=%d,\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}
	if (!tb[ATTR_SMS])
	{
		DBG_MSG("%s L%d ret=%d,\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}

	ret =  blobmsg_parse(sms_info_attr_policy, ARRAY_SIZE(sms_info_attr_policy), tb,
			     blobmsg_data(tb[ATTR_SMS]),
			     blobmsg_data_len(tb[ATTR_SMS]));

	if (ret){
		DBG_MSG("%s L%d ret=%d,\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}

	if (!tb[ATTR_NODE_LIST])
	{
		DBG_MSG("%s L%d ret=%d,\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}

	ret =  blobmsg_parse(sms_s1_attr_policy, ARRAY_SIZE(sms_s1_attr_policy), tb2g,
			     blobmsg_data(tb[ATTR_NODE_LIST]),
			     blobmsg_data_len(tb[ATTR_NODE_LIST]));

	if (ret) {
		DBG_MSG("%s L%d ret=%d,\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}

	if (!tb2g[ATTR_S1])
	{
		DBG_MSG("%s L%d ret=%d,\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}

	if (tb2g[ATTR_S1]) {
		ret = blobmsg_parse(sms_s1_attr_policy,
		            ARRAY_SIZE(sms_s1_attr_policy), tb2g,
		            blobmsg_data(tb2g[ATTR_S1]),
		            blobmsg_data_len(tb2g[ATTR_S1]));
		sms->status.address = tb2g[ATTR_ADDRESS] ? strdup(blobmsg_get_string(tb2g[ATTR_ADDRESS])) : NULL;

		memset(rest, 0, 100);
		memset(time, 0, 100);
		memset(date, 0, 100);
		rest = strdup(blobmsg_get_string(tb2g[ATTR_DATE]));
		token = strtok(rest, delimiter);
		while (token != NULL) {
			if(i>0 && i<3){
				strcat(date, token);
				strcat(date, "-");
			}
			if(i>=3 && i < 6){
				strcat(time, token);
				strcat(time, ":");
			}
			token = strtok(NULL, delimiter);
			i++;
		}
		token = strtok(rest, delimiter);
		strcat(date, token);
		time[strlen(time) - 1] = '\0';
		memset(body, 0, 1024);
		body = UniDecode(strdup(blobmsg_get_string(tb2g[ATTR_BODY])));

		sms->status.date = tb2g[ATTR_DATE] ? strdup(date) : NULL;
		sms->status.time = tb2g[ATTR_DATE] ? strdup(time) : NULL;
        sms->status.body = tb2g[ATTR_BODY] ? strdup(body) : NULL;
        DBG_MSG(" blobmsg address =%s, date =%s, time =%s, body =%s,\n",
                    sms->status.address,
                    sms->status.date,
                    sms->status.time,
                    sms->status.body);

        memset(transfer_buf, 0, sizeof(transfer_buf));
        memset(date_buf, 0, sizeof(date_buf));
        memset(time_buf, 0, sizeof(time_buf));
        memset(info_buf, 0, sizeof(info_buf));
        sprintf(transfer_buf, "From:%s",sms->status.address);
        strcpy(g_sms_transfer, transfer_buf);
        sprintf(date_buf, "Date:%s",sms->status.date);
        strcpy(g_sms_date, date_buf);
        sprintf(time_buf, "Time:%s",sms->status.time);
        strcpy(g_sms_time, time_buf);
        if(sms->status.body != NULL)
        {
                    sprintf(info_buf, "%s",sms->status.body);
                    strcpy(g_sms_info, info_buf);
        }

	}

    free(body);
	free(rest);
	free(date);
	free(time);

    mgui_sms_unread_number();
	DBG_MSG("%s L%d*******g_sms_time=[%s] g_sms_date=[%s] g_sms_transfer=[%s]\n",__FUNCTION__,__LINE__,g_sms_time,g_sms_date,g_sms_transfer);
	return 0;
}



struct mgui_sms_context *mgui_sms_unread_body_data_get(struct mgui_context *ctx)
{
	struct mgui_sms_context *sms;
	int ret;
	MASSERT(ctx);
	sms = malloc(sizeof(struct mgui_sms_context));
	if (!sms) {
		DBG_MSG("memory allocation failed\n");
		return NULL;
	}
	memset(sms, 0, sizeof(*sms));
	ret = mgui_ubus_lookup_id(ctx, SMS_UBUS_ID,
				  &sms->sms_ubus_id);
	if (ret) {
		DBG_MSG("mubus_lookup_id failed (ret=%d)\n", ret);
		goto out_error;
	}
	//ret = mgui_ubus_invoke_async(ctx,NULL,sms->sms_ubus_id,SMS_UBUS_REQUEST_UNREAD_SMS_BODY,sms_unread_body_data_cb,sms_unread_body_complete_cb,sms);
	ret = mgui_ubus_invoke(ctx,NULL,sms->sms_ubus_id,SMS_UBUS_REQUEST_UNREAD_SMS_BODY,sms_unread_body_data_cb,sms,0);
	if (ret) {
		DBG_MSG("initial request failed\n");
		goto out_error;
	}
	DBG_MSG("sms body update done\n");
	return sms;
out_error:
	free(sms);
	return NULL;
}
void mgui_sms_unread_number(void)
{
	char buf[128]={'\0'},tmpBuf[128]={'\0'};
	FILE *fp = fopen("/tmp/sms_new_number", "r");
    if (fp != NULL)
    {
        if(fgets(buf, sizeof(buf), fp) != NULL)
        {
            buf[strlen(buf)-1]='\0';
            strcpy(g_sms_unread,buf);
        }
        fclose(fp);
    }
    if(atoi(g_sms_unread)>0)
    {	
        strcpy( g_sms_tip, "NEW SMS" );
    }
    else
    {
        strcpy( g_sms_tip, "NO SMS" );
    }

}
static void *mgui_smskey_handler(void *arg)
{
    struct mgui_smskey_context *smskey = (struct mgui_smskey_context *)arg;
    struct mgui_context *mgui = smskey->mgui;
    struct input_event data;
    unsigned int press = 0;
    int fd = -1;
    char buf[128],tmpBuf[128]="",outBuf[128]="";
    int ret;
    struct mgui_event e = {.id = MGUI_SMSKEY_EVENT,};
    DBG_MSG("enter mgui_smskey_handler\n");
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    do {
        fd = open(SMSKEY_DEV, O_RDONLY);
        nowTime = read_poweron_time();
        interval = nowTime - lastTime;
        if (fd > 0) {
            DBG_MSG("smskey pressed interval[%u]\n",interval);
            e.id = MGUI_SMSKEY_EVENT;
            if(interval > 100)
            {
                write(mgui->pipes_fd[1], &e, sizeof(struct mgui_event));
                lastTime = nowTime;
            }
            system("rm -rf /tmp/mgui_sms_action 2>/dev/null");
        }
    } while (1);
    close(fd);
    return arg;
}
struct mgui_smskey_context *mgui_smskey_init(struct mgui_context *ctx)
{
	struct mgui_smskey_context *smskey;
	int ret;
	DBG_MSG("enter mgui_smskey_init\n");
	smskey = malloc(sizeof(*smskey));
	if (!smskey) {
		MGUI_EMSG("memory allocation failed\n");
		return NULL;
	}
	smskey->mgui = ctx;
	ret = pthread_create(&smskey->tid, NULL, mgui_smskey_handler, smskey);
	if (ret != 0) {
		MGUI_EMSG("pthread create error %d\n", ret);
		goto out_pthread_create;
	}
	DBG_MSG("smskey init done\n");
	return smskey;
out_pthread_create:
	free(smskey);
	return NULL;
}
