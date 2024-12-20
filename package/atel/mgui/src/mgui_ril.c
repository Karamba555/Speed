/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: mgui_ril.c
 *
 *  Authors:  Tomer Eliyahu
 *
 *  Description: MGUI RIL interface
 *
 *  HISTORY:
 *
 *   Nov 23, 2014 - Initial Version
 *
 *  Notes:
 *
 ******************************************************************************/

/******************************************************************************
 *   Include files
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "mgui_utils.h"
#include "mgui_ril.h"
#include "mgui_ubus.h"
#include "mgui_icons.h"
#include "mgui.h"

extern struct mgui_icon *mgui_icons[];
extern void set_icons(struct mgui_context *ctx, struct mgui_icon **icons, int index);

/******************************************************************************
 *   Structures
 ******************************************************************************/

struct mgui_ril_request {
	void (*cb)(struct mgui_ril_request *);
	struct mgui_ril_context *ril;

	/* Request */
	unsigned int id; /* request id */
	int sync; /* async or sync request? */

	/* Respone */
	unsigned int valid;
	unsigned int resp_id;
	unsigned int errcode;

	/* both */
	void *data;	/* data received / sent */
	int data_len;	/* data length */
};

/******************************************************************************
 *   Definitions
 ******************************************************************************/
#define RIL_UBUS_ID		"ril"
#define RIL_UBUS_REQ		"ril_request"

#define to_ubus_subscriber(obj) container_of(obj, struct ubus_subscriber, obj)
#define to_ril_context(obj) container_of(to_ubus_subscriber(obj), struct mgui_ril_context, ril_ubus_subscriber)

/******************************************************************************
 *   Globals
 ******************************************************************************/
/* RILD unsolicited indications subscriptions */
static const char *ril_unsol_events[] = {
	//"ril.unsol.cc",
	//"ril.unsol.dev",
	"ril.unsol.mm",
	//"ril.unsol.msg",
	//"ril.unsol.ps",
	#ifdef ENABLE_UBUS_SIM_STATUS
	"ril.unsol.sim",
	#endif
	//"ril.unsol.ss",
};

static	struct blob_buf b;
extern char g_operator[];
/******************************************************************************
 *  Private Functions Prototypes
 ******************************************************************************/
static int mril_send_initial_requests(struct mgui_ril_context *ril, int sync);
/******************************************************************************
 *  Code
 ******************************************************************************/

static void mril_reset(struct mgui_ril_context *ril)
{
	MASSERT(ril);

	MGUI_DMSG("mgui ril reset icons\n");

	memset(&ril->data_registration, 0, sizeof(ril->data_registration));
	#ifdef ENABLE_USBU_VOICE
	memset(&ril->voice_registration, 0, sizeof(ril->voice_registration));
	#endif
	#ifdef ENABLE_USBU_OPERATOR
	if (strcmp(ril->Operator, "CP ASSERT") != 0) {
		strncpy(ril->Operator, "NoService",
			MIN(MAX_OPERATOR_NAME_SIZE-1, strlen("NoService")));
	}
	#endif
	ril->rssi = 0;
	ril->simcard_state = RIL_CARDSTATE_ABSENT;
#ifdef ENABLE_USBU_OPERATOR
	/* update all ril icons */
	//mgui_update_icon(ril->mgui, MGUI_SIM_ICON, NULL);
	if (strcmp(ril->Operator, "CP ASSERT") != 0) {
		//mgui_update_icon(ril->mgui, MGUI_OPERATOR_ICON, NULL);
	}
#endif
	mgui_update_icon(ril->mgui, MGUI_NETWORK_TECH_ICON, NULL);
	//mgui_update_icon(ril->mgui, MGUI_CELLULAR_ICON, NULL);
	//if(ril->mgui->state == MGUI_STATE_FIRST)
	//set_icons(ril->mgui, mgui_icons, MGUI_CELLULAR_ICON);

}

static void ril_complete_cb(struct ubus_request *req, int ret)
{
	struct mgui_ril_request *ril_req;
	struct mgui_ril_context *ril;

	ril_req = (struct mgui_ril_request *)req->priv;
	ril = ril_req->ril;

	MGUI_DMSG("ril_req = %08X, ubus_request = %08X ril->mgui->state=%d data_len=%d", ril_req, req,ril->mgui->state,ril_req->data_len);
	pthread_mutex_lock(&ril->mgui->lock);
	/* trigger screen refresh since this is a result of an async request */
	if(ril->mgui->state == MGUI_STATE_FIRST && ril_req->data_len > 10)
	{
		MGUI_DMSG("mgui_screen_refresh\n");
		mgui_screen_refresh(ril->mgui);
	}
	pthread_mutex_unlock(&ril->mgui->lock);
	free(ril_req);
	free(req);
}

static void ril_data_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	unsigned int requestid;
	unsigned int rilerrno = 0;
	void *data = NULL;
	int datalen = 0;
	int ret = 0;
	struct mgui_ril_request *ril_req;


	if (req == NULL) {
		MGUI_EMSG("req is NULL !!!");
		return;
	}

	/* Pickup RIL request structure */
	ril_req = (struct mgui_ril_request *)req->priv;
#ifdef CONFIG_TARGET_mmp ///this ril related functions come from ASR, need to change 
	ret = rilutil_parseResponse(msg, &requestid, &rilerrno, &data, &datalen);
#else
	
#endif
	MGUI_DMSG("req = %08X, ril_req = %08X, parseResponse returned %d", req, ril_req, ret);

	if (ril_req) {
		if(ret || rilerrno) {
			if (ril_req->cb) {
				/*
				 * Mark this response as invalid, so that callback will free MGUI
				 * memory
				 */
				ril_req->valid = 0;
				ril_req->cb(ril_req);
			}
#ifdef CONFIG_TARGET_mmp ///this ril related functions come from ASR, need to change			
			if (data)
				rilutil_freeResponseData(requestid, data, datalen);
#else

#endif	
			return;
		}

		ril_req->valid		= 1;
		ril_req->resp_id	= requestid;
		ril_req->errcode	= rilerrno;
		ril_req->data		= data;
		ril_req->data_len	= datalen;

		MGUI_DMSG("reqId = %d, ril_err = %d, data = %08X, data_len = %d", 
			   requestid, rilerrno, data, datalen);

		if (ril_req->cb)
			ril_req->cb(ril_req);
	} else {
		MGUI_EMSG("rilReq is NULL !!!");
	}
#ifdef CONFIG_TARGET_mmp ///this ril related functions come from ASR, need to change
	if (data)
		rilutil_freeResponseData(requestid, data, datalen);
#else

#endif	
}

static int send_to_ril(struct mgui_ril_context *ril,
			struct mgui_ril_request *req)
{
	int ret;
#ifdef CONFIG_TARGET_mmp ///this ril related functions come from ASR, need to change
	ret = rilutil_makeRequestBlob(&b, req->id, req->data, req->data_len);
	if (ret) {
		MGUI_EMSG("rilutil_makeRequestBlob failed\n");
		return ret;
	}
#else
	
#endif
	if (req->sync) {
		ret = mgui_ubus_invoke(ril->mgui->ubus, &b, ril->ril_ubus_id,
				   RIL_UBUS_REQ, ril_data_cb, req, 0);
		free(req);
		if (ret) {
			MGUI_EMSG("mubus_invoke failed\n");
			goto out;
		}
	} else {
		ret = mgui_ubus_invoke_async(ril->mgui->ubus, &b, ril->ril_ubus_id,
					 RIL_UBUS_REQ, ril_data_cb,
					 ril_complete_cb, req);
		if (ret) {
			MGUI_EMSG("mubus_invoke_async failed\n");
			goto out;
		}
	}

	MGUI_DMSG("send to ril done\n");
out:
	return ret;
}

static struct mgui_ril_request *alloc_request(struct mgui_ril_context *ril)
{
	struct mgui_ril_request *req;

	req = malloc(sizeof(struct mgui_ril_request));
	if (!req) {
		MGUI_EMSG("memory allocation failed\n");
		return NULL;
	}
	memset(req, 0, sizeof(*req));

	req->ril = ril;

	return req;
}
#ifdef ENABLE_USBU_OPERATOR
/**
 * operator_cb
 * 
 * ((const char **)response)[0] is long alpha ONS or EONS
 *                                  or NULL if unregistered
 * 
 * ((const char **)response)[1] is short alpha ONS or EONS
 *                                  or NULL if unregistered
 * ((const char **)response)[2] is 5 or 6 digit numeric code (MCC + MNC)
 *                                  or NULL if unregistered
 * 
 * @param req    ril request
 */
static void operator_cb(struct mgui_ril_request *req)
{
	
	struct mgui_ril_context *ril = (struct mgui_ril_context *)req->ril;
	rilutilstrings *resp = (rilutilstrings *)req->data;
	
	MASSERT(ril);
	MASSERT(req);

	if (strcmp(ril->Operator, "CP ASSERT") == 0) {
		return;
	}

	if (req->errcode != 0) {
		MGUI_EMSG("RIL ERROR: valid=%d, errcode=%d, data=%p\n",
			  req->valid, req->errcode, req->data);
		return;
	}

	if (!req->valid || !resp || resp->num != 3) {
		MGUI_IMSG("No operator found\n");
		strncpy(ril->Operator, "NoService",
			MIN(MAX_OPERATOR_NAME_SIZE-1, strlen("NoService")));
	} else {
		/* operator present */
		/*
		MGUI_DMSG("resp[0] = %s, resp[1] = %s, resp[2] = %s",
			  resp->str[0], resp->str[1], resp->str[2]);
		strncpy(ril->Operator, resp->str[0],
			MIN(MAX_OPERATOR_NAME_SIZE-1, strlen(resp->str[0])));
		*/

		if (resp->str[1] && strlen(resp->str[1]) > 0) {
			MGUI_EMSG("resp[1] = %s", resp->str[1]);
			strncpy(ril->Operator, resp->str[1],
				MIN(MAX_OPERATOR_NAME_SIZE-1, strlen("NoService")));
		} else {
			MGUI_EMSG("resp->str[1] is null");
			strncpy(ril->Operator, "NoService",
				MIN(MAX_OPERATOR_NAME_SIZE-1, strlen("NoService")));
		}
	}
	//strcpy(g_operator,ril->Operator);
	DBG_MSG("Operator: %s\n", ril->Operator);

	//mgui_update_icon(ril->mgui, MGUI_OPERATOR_ICON, (void *)ril);
}
#endif
/**
 * sim callback
 * 
 * @param req    ril request
 */
static void sim_cb(struct mgui_ril_request *req)
{
	struct mgui_ril_context *ril = (struct mgui_ril_context *)req->ril;

	MASSERT(ril);
	MASSERT(req);

	if (!req->valid || req->errcode != 0 || !req->data) {
		MGUI_EMSG("RIL ERROR: valid=%d, errcode=%d\n",
			  req->valid, req->errcode);
		return;
	}

	ril->simcard_state = ((RIL_CardStatus_v6 *)req->data)->card_state;

	MGUI_IMSG("ril->simcard_state = %d", ril->simcard_state);

	//mgui_update_icon(ril->mgui, MGUI_SIM_ICON, (void *)ril);
}

/**
 * registration callback
 * 
 * "response" is a "char **"
 * ((const char **)response)[0] is registration state 0-5 from TS 27.007 10.1.20 AT+CGREG
 * ((const char **)response)[1] is LAC if registered or NULL if not
 * ((const char **)response)[2] is CID if registered or NULL if not
 * ((const char **)response)[3] indicates the available data radio technology,
 *                              valid values as defined by RIL_RadioTechnology.
 * ((const char **)response)[4] if registration state is 3 (Registration
 *                               denied) this is an enumerated reason why
 *                               registration was denied.  See 3GPP TS 24.008,
 *                               Annex G.6 "Additonal cause codes for GMM".
 *      7 == GPRS services not allowed
 *      8 == GPRS services and non-GPRS services not allowed
 *      9 == MS identity cannot be derived by the network
 *      10 == Implicitly detached
 *      14 == GPRS services not allowed in this PLMN
 *      16 == MSC temporarily not reachable
 *      40 == No PDP context activated
 * ((const char **)response)[5] The maximum number of simultaneous Data Calls that can be
 *                              established using RIL_REQUEST_SETUP_DATA_CALL.
 * 
 * @param req    ril request
 */
static void registration_cb(struct mgui_ril_request *req)
{
	struct mgui_ril_context *ril = (struct mgui_ril_context *)req->ril;
	rilutilstrings *resp;
	struct mgui_ril_reg_info *reg_info;

	MASSERT(ril);
	MASSERT(req);

	if (!req->valid || req->errcode != 0 || !req->data) {
		MGUI_EMSG("RIL ERROR: valid=%d, errcode=%d\n",
			  req->valid, req->errcode);
		return;
	}

	resp = (rilutilstrings *)req->data;
	if (resp->num < 2) {
		MGUI_EMSG("unexpected data length\n");
		return;
	}

	reg_info = (req->id == RIL_REQUEST_VOICE_REGISTRATION_STATE) ?
		    &ril->voice_registration :
		    &ril->data_registration;

	sscanf(resp->str[0], "%d", &reg_info->reg_state);
	if (resp->num >= 5)
		/* str[3] may be null when no sim card attached */
		if (resp->str[3])
			sscanf(resp->str[3], "%d", (int *)&reg_info->radio_tech);

	MGUI_IMSG("reg_state[%s]=%d, radio_tech=%d\n",
		  req->id == RIL_REQUEST_VOICE_REGISTRATION_STATE ? "voice" : "data",
		  reg_info->reg_state, reg_info->radio_tech);

	if (!REGISTERED_VOICE(ril)) {
		MGUI_IMSG("Not registered!\n");
		if(ril->mgui->state == MGUI_STATE_FIRST){
			//set_icons(ril->mgui, mgui_icons, MGUI_CELLULAR_ICON);
		}
		#ifdef ENABLE_USBU_OPERATOR
		if (strcmp(ril->Operator, "CP ASSERT") != 0) {
			//mgui_update_icon(ril->mgui, MGUI_OPERATOR_ICON, NULL);
		}
		#endif
	} else {
		/* registered - if this is a result of indication */
		MGUI_IMSG("Registered!\n");
	}

	mgui_update_icon(ril->mgui, MGUI_NETWORK_TECH_ICON, (void *)ril);
}

/******************************************************************************
 *  RILD queries
 ******************************************************************************/
#ifdef ENABLE_USBU_OPERATOR
static inline int mril_request_operator(struct mgui_ril_context *ril, int sync)
{
	struct mgui_ril_request *req = alloc_request(ril);

	if (!req) {
		MGUI_EMSG("memory allocation failed\n");
		return -1;
	}

	req->id		= RIL_REQUEST_OPERATOR;
	req->cb		= operator_cb;
	req->sync	= sync;	

	MGUI_IMSG("sending request operator\n");

	return send_to_ril(ril, req);
}
#endif
#ifdef ENABLE_UBUS_SIM_STATUS
static inline int mril_request_sim(struct mgui_ril_context *ril, int sync)
{
	struct mgui_ril_request *req = alloc_request(ril);

	if (!req) {
		MGUI_EMSG("memory allocation failed\n");
		return -1;
	}

	req->id		= RIL_REQUEST_GET_SIM_STATUS;
	req->cb		= sim_cb;
	req->sync	= sync;	

	MGUI_IMSG("sending request sim status\n");

	return send_to_ril(ril, req);
}
#endif
/**
 * request registration state from ril
 * 
 * @param ind    called from indication
 * @param sync   sync / async request
 * 
 * @return 0 for success, error code otherwise
 */
static inline int __mril_request_registration(struct mgui_ril_context *ril, int sync, unsigned int id)
{
	struct mgui_ril_request *req = alloc_request(ril);

	if (!req) {
		MGUI_EMSG("memory allocation failed\n");
		return -1;
	}

	req->id		= id;
	req->cb		= registration_cb;
	req->sync	= sync;	

	MGUI_IMSG("sending request registration state (id=%d)\n", req->id);

	return send_to_ril(ril, req);
}

static inline int mril_request_voice_registration(struct mgui_ril_context *ril, int sync)
{
	return __mril_request_registration(ril,sync,RIL_REQUEST_VOICE_REGISTRATION_STATE);
}

static inline int mril_request_data_registration(struct mgui_ril_context *ril, int sync)
{
	return __mril_request_registration(ril,sync,RIL_REQUEST_DATA_REGISTRATION_STATE);
}

/**
 * request screen state change from ril
 * 
 * request type - sync / async, no callback (can be added in the future for confirmation)
 * 
 * @param ril    ril context
 * @param sync   1 for sync request, 0 for async (needs uloop_run())
 * @param on     1 to enable screen state, 0 to disable
 * 
 * @return 0 for success, error code otherwise
 */
int mril_request_screen(struct mgui_ril_context *ril, int sync, int on)
{
	struct mgui_ril_request *req = alloc_request(ril);
	int *data = (int *)malloc(sizeof(int));

	if (!req || !data) {
		MGUI_EMSG("memory allocation failed\n");
		return -1;
	}

	*data = on;

	req->id		= RIL_REQUEST_SCREEN_STATE;
	req->sync	= sync;	
	req->data	= data;
	req->data_len	= 1;

	MGUI_IMSG("sending request screen change\n");
	
	return send_to_ril(ril, req);
}
#ifdef ENABLE_UBUS_SIM_STATUS
/******************************************************************************
 *  RILD Indications
 ******************************************************************************/
static int mril_indication_sim(struct mgui_ril_context *ril)
{
	MASSERT(ril);

	MGUI_IMSG("indication received, trigger sim status req\n");
	mril_request_sim(ril, 0);

	return 0;
}
#endif
static int mril_indication_radio(struct mgui_ril_context *ril,
				   void *data, int len)
{
	RIL_RadioState radio_state;

	MASSERT(ril);
	MASSERT(data);
	
	radio_state = *(RIL_RadioState *)data;

	MGUI_IMSG("indication received, radio_state=%d\n", radio_state);
	/* whatever - send initial requests again */
	mril_send_initial_requests(ril, 0);

	return 0;
}

static int mril_indication_voice_network(struct mgui_ril_context *ril)
{
	MASSERT(ril);

	MGUI_IMSG("voice network indication received\n");
	/* something changed - send initial requests again */
	mril_send_initial_requests(ril, 0);

	return 0;
}

static int mril_indication_voice_radiotech(struct mgui_ril_context *ril,
					     void *data, int len)
{
	MASSERT(ril);

	MGUI_IMSG("voice radiotech indication received\n");
	/* something changed - send initial requests again */
	mril_send_initial_requests(ril, 0);

	return 0;
}

static inline int convertDbmToRssi(int d)
{
	if(d == INT_MAX)
		return 99;
	else if(d < -113)
		return 0;
	else if(d > -51)
		return 31;
	else
		return (113 + d ) / 2;
}

static int mril_indication_rssi(struct mgui_ril_context *ril, void *data,
				  int len)
{
	RIL_SignalStrength_v6 *rssi = (RIL_SignalStrength_v6 *)data;


	MASSERT(ril);
	MASSERT(rssi);

	MGUI_IMSG("indication received\n");
	MGUI_EMSG("indication received errorRate = %d, signalStrength = %d, ril->mgui->state = %d\n", 
		rssi->GW_SignalStrength.bitErrorRate,
		rssi->GW_SignalStrength.signalStrength,
		ril->mgui->state);

	if ((rssi->GW_SignalStrength.bitErrorRate == 89) && 
		(rssi->GW_SignalStrength.signalStrength == 67)) {
		if (ril->mgui->state == MGUI_STATE_OFF) {
			mgui_cp_assert_wakeup(ril->mgui);
		}
		#ifdef ENABLE_USBU_OPERATOR
		memset(ril->Operator, 0, sizeof(ril->Operator));
		strncpy(ril->Operator, "CP ASSERT",
			MIN(MAX_OPERATOR_NAME_SIZE-1, strlen("CP ASSERT")));
		//return mgui_update_icon(ril->mgui, MGUI_OPERATOR_ICON, (void *)ril);
		#endif
	}

	if (ril->data_registration.radio_tech == RADIO_TECH_LTE || RADIO_TECH_LTEP == ril->data_registration.radio_tech) {
		MGUI_IMSG("in LTE, using rsrp == %d as rssi value\n",
			  rssi->LTE_SignalStrength.rsrp);
		ril->rssi = convertDbmToRssi(rssi->LTE_SignalStrength.rsrp);
	} else {
		MGUI_IMSG("Not LTE, rssi = %d\n",
			   rssi->GW_SignalStrength.signalStrength);
		ril->rssi = rssi->GW_SignalStrength.signalStrength;
	}

	MGUI_DMSG("rssi value changed to %d\n", ril->rssi);

	//if(ril->mgui->state == MGUI_STATE_FIRST)
		 //set_icons(ril->mgui, mgui_icons, MGUI_CELLULAR_ICON);
	return 0;
}

enum {
	REQID,
	ERRNO,
	DATA,
	_MAX
};

static int mril_parse_blobmsg(struct blob_attr *attr, struct blob_attr **tb)
{
	struct blob_attr *pos = NULL;
	struct blobmsg_hdr *hdr = NULL;
	char *policy[] = { "rilid", "resperrno" };
	int rem = 0;
	int i = 0;

	blob_for_each_attr(pos, attr, rem) {
		if (i > _MAX)
			return 0;

		hdr = blob_data(pos);
		if (i == 0) {
			if (strcmp(policy[i], (char *)hdr->name) != 0) {
				MGUI_EMSG("format is error\n");
				return -1;
			}
		}

		tb[i] = pos;
		i++;
	}

	return 0;
}

/* ril indications dispatcher */
static int indication_cb(struct ubus_context *ctx, struct ubus_object *obj,
			  struct ubus_request_data *req, const char *method,
			  struct blob_attr *msg)
{
	unsigned int id = 0;
	unsigned int errcode = 0;
	void *data = NULL;
	int len = 0;
	int ret = 0;
	struct mgui_ril_context *ril = to_ril_context(obj);
	struct blob_attr *tb[_MAX] = { 0 };

	pthread_mutex_lock(&ril->lock);
	if (ril->sleep) {
		pthread_mutex_unlock(&ril->lock);
		MGUI_EMSG("received indication although indications disabled, ignoring\n");
		return -1;
	}
	pthread_mutex_unlock(&ril->lock);

	ret = mril_parse_blobmsg(msg, tb);
	if (ret < 0) {
		MGUI_EMSG("parse msg error\n");
		return -1;
	}

	if (tb[REQID])
		id = blobmsg_get_u32(tb[REQID]);
	if (tb[ERRNO]) {
		errcode = blobmsg_get_u32(tb[ERRNO]);
		if (errcode) {
			MGUI_EMSG("unsolicited id %d blobmsg err %s\n",
				  id, errcode);
			return -1;
		}
	}
	if (tb[DATA]) {
		data = blob_data(tb[DATA]);
		len = blobmsg_data_len(tb[DATA]);
	}

	

	switch(id)
	{
		#ifdef ENABLE_UBUS_SIM_STATUS
		case RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED:
			MGUI_DMSG("SIM_STATUS_CHANGED id is %d, data = %p, len = %d\n", id, data, len);
			ret = mril_indication_sim(ril);
			break;
		#endif
			#ifdef ENABLE_UBUS_RADIO_TECH
		case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED:
			MGUI_DMSG("RADIO_STATE_CHANGED id is %d, data = %p, len = %d\n", id, data, len);
			ret = mril_indication_radio(ril, data, len);
			break;
			#endif
			#ifdef ENABLE_USBU_VOICE
		case RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED:
			MGUI_DMSG("VOICE_NETWORK_STATE_CHANGED id is %d, data = %p, len = %d\n", id, data, len);
			ret = mril_indication_voice_network(ril);
			break;
		case RIL_UNSOL_VOICE_RADIO_TECH_CHANGED:
			MGUI_DMSG("VOICE_RADIO_TECH_CHANGED id is %d, data = %p, len = %d\n", id, data, len);
			ret = mril_indication_voice_radiotech(ril, data, len);
			break;
			#endif
		case RIL_UNSOL_SIGNAL_STRENGTH:
			MGUI_DMSG("RIL_UNSOL_SIGNAL_STRENGTH id is %d, data = %p, len = %d\n", id, data, len);
			ret = mril_indication_rssi(ril, data, len);
			break;
		default:
			//MGUI_EMSG("id of %d is not supported\n",id);
			ret = 0;
	}

	/* refresh screen if required */
	if (ret && ril->mgui->state == MGUI_STATE_FIRST)
	{
		pthread_mutex_lock(&ril->mgui->lock);
		MGUI_EMSG("indication_cb mgui_screen_refresh\n");
		mgui_screen_refresh(ril->mgui);
		pthread_mutex_unlock(&ril->mgui->lock);
	}

	return 0;
}

/* disable all indications (see ril_unsol_events) */
static int mril_disable_indications(struct mgui_ril_context *ril)
{
	int i;

	MASSERT(ril);
	MASSERT(ril->mgui);
	MASSERT(ril->mgui->ubus);

	for (i = 0; i < ARRAY_SIZE(ril_unsol_events); i++)
		mgui_ubus_unsubscribe_event(ril->mgui->ubus,
					&ril->ril_ubus_subscriber,
					ril_unsol_events[i]);

	MGUI_IMSG("unsubscribe ril.unsol events done\n");

	return 0;
}

/* enable all indications (see ril_unsol_events) */
static int mril_enable_indications(struct mgui_ril_context *ril)
{
	int ret, i;

	MASSERT(ril);
	MASSERT(ril->mgui->ubus);

	for (i = 0; i < ARRAY_SIZE(ril_unsol_events); i++) {
		ret = mgui_ubus_subscribe_event(ril->mgui->ubus,
					    &ril->ril_ubus_subscriber,
					    ril_unsol_events[i]);
		if (ret) {
			MGUI_EMSG("event %s subscribe failed\n",
				  ril_unsol_events[i]);
			goto unsubscribe;
		}
	}

	MGUI_IMSG("subscribe ril.unsol events success\n");

	return 0;

unsubscribe:
	for (i = i-1 ; i > 0; i--)
		mgui_ubus_unsubscribe_event(ril->mgui->ubus,
					&ril->ril_ubus_subscriber,
					ril_unsol_events[i]);
	return ret;
}


/**
 * invoke mgui initial requests (sync)
 * 
 * @param ril    mgui ril context
 * 
 * @return 0 for success, return code otherwise
 */
static int mril_send_initial_requests(struct mgui_ril_context *ril, int sync)
{
	int ret;

	#ifdef ENABLE_UBUS_SIM_STATUS
	ret = mril_request_sim(ril, sync);
	if (ret)
		return ret;
	#endif
	#ifdef ENABLE_USBU_OPERATOR
	ret = mril_request_operator(ril, sync);
	if (ret)
		return ret;
	#endif

	#ifdef ENABLE_USBU_VOICE
	ret = mril_request_voice_registration(ril,sync);
	if (ret)
		return ret;
	#endif

	#ifdef ENABLE_UBUS_RADIO_TECH
	ret = mril_request_data_registration(ril, sync);
	if (ret)
		return ret;
	#endif

	MGUI_IMSG("init requests sent\n");

	return 0;
}

void mgui_ril_wakeup(struct mgui_ril_context *ril)
{
	if(!ril) {
		MGUI_IMSG("ril module not running\n");
		return;
	}

	MGUI_IMSG("exit ril sleep\n");

	pthread_mutex_lock(&ril->lock);
	ril->sleep = 0;
	pthread_mutex_unlock(&ril->lock);

	mril_send_initial_requests(ril, 0);
	mril_enable_indications(ril);
}

void mgui_ril_sleep(struct mgui_ril_context *ril)
{
	if(!ril) {
		MGUI_IMSG("ril module not running\n");
		return;
	}

	MGUI_IMSG("enter ril sleep\n");

	pthread_mutex_lock(&ril->lock);
	ril->sleep = 1;
	pthread_mutex_unlock(&ril->lock);

	mril_reset(ril);
	mril_request_screen(ril, 0, 0);
	mril_disable_indications(ril);
}

/**
 * mgui_ril_exit
 * De-initialize RIL interface
 * 
 * @param ril    pointer to previously allocated ril context
 * 
 * @return 0 for success, error code otherwise
 */
int mgui_ril_exit(struct mgui_ril_context *ril)
{
	if(!ril) {
		MGUI_IMSG("ril module not running\n");
		return 0;
	}

	mril_disable_indications(ril);
	mgui_ubus_unregister_subscriber(ril->mgui->ubus, &ril->ril_ubus_subscriber);
	pthread_mutex_destroy(&ril->lock);
	free(ril);

	MGUI_IMSG("ril exit done\n");

	return 0;
}

/**
 * mgui_ril_init
 * Initialize mgui rild interface
 * 
 * Parameters:     mgui - mgui main database
 * Returns:        pointer to mgui_ril_context
 * 
 * @param mgui   mgui context
 * 
 * @return pointer to mgui_ril_context
 */
struct mgui_ril_context *mgui_ril_init(struct mgui_context *mgui)
{
	struct mgui_ril_context *ril;
	int ret;

	MASSERT(mgui);
	MASSERT(mgui->ubus);

	ril = malloc(sizeof(struct mgui_ril_context));
	if (!ril) {
		MGUI_EMSG("memory allocation failed\n");
		return NULL;
	}

	memset(ril, 0, sizeof(*ril));

	ril->mgui = mgui;

	if (pthread_mutex_init(&ril->lock, NULL) != 0) {
		printf("\n mutex init failed\n");
		free(ril);
		return NULL;
	}

	ret = mgui_ubus_lookup_id(ril->mgui->ubus, RIL_UBUS_ID,
				  &ril->ril_ubus_id);
	if (ret) {
		MGUI_EMSG("mubus_lookup_id failed (ret=%d)\n", ret);
		goto out_error;
	}

	ret = mgui_ubus_register_subscriber(ril->mgui->ubus,
					    &ril->ril_ubus_subscriber);
	if (ret) {
		MGUI_EMSG("mubus_register_subscriber failed\n");
		goto out_error;
	}
	ril->ril_ubus_subscriber.cb = indication_cb;

	/* run initial queries to get mgui initial state */
	ret = mril_send_initial_requests(ril, 0);
	if (ret) {
		MGUI_EMSG("failed to run rild initial queries\n");
		goto unregister_subscriber;
	}

	ret = mril_enable_indications(ril);
	if (ret) {
		MGUI_EMSG("Indications enable failed\n");
		goto out_indications;
	}

	MGUI_IMSG("ril init done\n");

	return ril;

out_indications:
unregister_subscriber:
	mgui_ubus_unregister_subscriber(ril->mgui->ubus, &ril->ril_ubus_subscriber);
out_error:
	pthread_mutex_destroy(&ril->lock);
	free(ril);
	return NULL;
}
