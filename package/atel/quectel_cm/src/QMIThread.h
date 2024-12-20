#ifndef __QMI_THREAD_H__
#define __QMI_THREAD_H__

#define CONFIG_GOBINET
#define CONFIG_QMIWWAN
#define CONFIG_SIM
#define CONFIG_APN
#define CONFIG_VERSION
#define CONFIG_DEFAULT_PDP 1
#define CONFIG_IMSI_ICCID

#include <stdbool.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stddef.h>

#include "MPQMI.h"
#include "MPQCTL.h"
#include "MPQMUX.h"
#include "util.h"

#define DEVICE_CLASS_UNKNOWN           0
#define DEVICE_CLASS_CDMA              1
#define DEVICE_CLASS_GSM               2

#define WWAN_DATA_CLASS_NONE            0x00000000
#define WWAN_DATA_CLASS_GPRS            0x00000001
#define WWAN_DATA_CLASS_EDGE            0x00000002 /* EGPRS */
#define WWAN_DATA_CLASS_UMTS            0x00000004
#define WWAN_DATA_CLASS_HSDPA           0x00000008
#define WWAN_DATA_CLASS_HSUPA           0x00000010
#define WWAN_DATA_CLASS_LTE             0x00000020
#define WWAN_DATA_CLASS_1XRTT           0x00010000
#define WWAN_DATA_CLASS_1XEVDO          0x00020000
#define WWAN_DATA_CLASS_1XEVDO_REVA     0x00040000
#define WWAN_DATA_CLASS_1XEVDV          0x00080000
#define WWAN_DATA_CLASS_3XRTT           0x00100000
#define WWAN_DATA_CLASS_1XEVDO_REVB     0x00200000 /* for future use */
#define WWAN_DATA_CLASS_UMB             0x00400000
#define WWAN_DATA_CLASS_CUSTOM          0x80000000

struct wwan_data_class_str {
    ULONG class;
    CHAR *str;
};

#pragma pack(push, 1)

typedef struct _QCQMIMSG {
    QCQMI_HDR QMIHdr;
    union {
        QMICTL_MSG CTLMsg;
        QMUX_MSG MUXMsg;
    };
} __attribute__ ((packed)) QCQMIMSG, *PQCQMIMSG;

typedef struct __IPV4 {
    uint32_t Address;
    uint32_t Gateway;
    uint32_t SubnetMask;
    uint32_t DnsPrimary;
    uint32_t DnsSecondary;
    uint32_t Mtu;
} IPV4_T;

typedef struct __IPV6 {
    UCHAR Address[16];
    UCHAR Gateway[16];
    UCHAR SubnetMask[16];
    UCHAR DnsPrimary[16];
    UCHAR DnsSecondary[16];
    UCHAR PrefixLengthIPAddr;
    UCHAR PrefixLengthGateway;
    ULONG Mtu;
} IPV6_T;

#define IpFamilyV4 (0x04)
#define IpFamilyV6 (0x06)

struct __PROFILE;
struct qmi_device_ops {
	int (*init)(struct __PROFILE *profile);
	int (*deinit)(void);
	int (*send)(PQCQMIMSG pRequest);
	void* (*read)(void *pData);

    
    // int (*thread_read)(struct __PROFILE *profile);
    // int (*init)(struct __PROFILE *profile);
    // int (*open)(struct __PROFILE *profile);
    // int (*close)(struct __PROFILE *profile);
    // int (*reopen)(struct __PROFILE *profile);
    // int (*start_network)(struct __PROFILE *profile);
    // int (*stop_network)(struct __PROFILE *profile);
    // int (*query_network)(struct __PROFILE *profile);
};
extern int (*qmidev_send)(PQCQMIMSG pRequest);

typedef struct __PROFILE {
    char *qmichannel;
    char *usbnet_adapter;
    char *qmapnet_adapter;
    char *driver_name;
    int qmap_mode;
    int qmap_version;
    const char *apn;
    const char *user;
    const char *password;
    const char *pincode;
    int auth;
    int pdp;
    int curIpFamily;
    int rawIP;
    int muxid;
    IPV4_T ipv4;
    IPV6_T ipv6;
    int enable_ipv4;
    int enable_ipv6;
    int apntype;
    const struct qmi_device_ops *qmi_ops;
} PROFILE_T;

typedef enum {
    SIM_ABSENT = 0,
    SIM_NOT_READY = 1,
    SIM_READY = 2, /* SIM_READY means the radio state is RADIO_STATE_SIM_READY */
    SIM_PIN = 3,
    SIM_PUK = 4,
    SIM_NETWORK_PERSONALIZATION = 5,
    SIM_BAD = 6,
    SIM_INITIALIZING = 99 // Fake state for start up
} SIM_Status;

#pragma pack(pop)

#define WDM_DEFAULT_BUFSIZE	256
#define RIL_REQUEST_QUIT    0x1000
#define RIL_INDICATE_DEVICE_CONNECTED    0x1002
#define RIL_INDICATE_DEVICE_DISCONNECTED    0x1003
#define RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED    0x1004
#define RIL_UNSOL_DATA_CALL_LIST_CHANGED    0x1005
#define MODEM_REPORT_RESET_EVENT 0x1006

int pthread_cond_timeout_np(pthread_cond_t *cond, pthread_mutex_t * mutex, unsigned msecs);
int QmiThreadSendQMI(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse);
int QmiThreadSendQMITimeout(PQCQMIMSG pRequest, PQCQMIMSG *ppResponse, unsigned msecs);
void QmiThreadRecvQMI(PQCQMIMSG pResponse);
void udhcpc_start(PROFILE_T *profile);
void udhcpc_stop(PROFILE_T *profile);
void dump_qmi(void *dataBuffer, int dataLen);
void qmidevice_send_event_to_main(int triger_event);
int requestSetEthMode(PROFILE_T *profile);
int requestGetSIMStatus(SIM_Status *pSIMStatus);
int requestEnterSimPin(const CHAR *pPinCode);
int requestGetICCID(void);
int requestGetIMSI(void);
int requestGetSerialNumbers(void);

// Dynamic values
void write_to_file(char *file_name, float value);
void write_string_to_file(char *file_name, char *value);
int read_value_from_file(char *file_name); 
int is_file_exist(char *file_name);
int requestGetSignalStrength(bool debug);
int requestGetTxRxInfo(bool debug);
int requestGetRFBandInfo(bool debug);
int requestGetCellLocationInfo(bool debug);

int requestGetSystemSelectionPreference(void);

int requestRegistrationState(UCHAR *pPSAttachedState);
int requestQueryDataCall(UCHAR  *pConnectionStatus, int curIpFamily);
int requestSetupDataCall(PROFILE_T *profile, int curIpFamily);
int requestDeactivateDefaultPDP(PROFILE_T *profile, int curIpFamily, int force);
int requestSetProfile(PROFILE_T *profile);
int requestGetProfile(PROFILE_T *profile);
int requestBaseBandVersion(const char **pp_reversion);
int requestGetIPAddress(PROFILE_T *profile, int curIpFamily);
int requestSetOperatingMode(UCHAR OperatingMode);

void cond_setclock_attr(pthread_cond_t *cond, clockid_t clock);
int mbim_main(PROFILE_T *profile);
int verify_driver(PROFILE_T *profile);
BOOL qmidevice_detect(char *qmichannel, char *usbnet_adapter, unsigned bufsize);
int ql_bridge_mode_detect(PROFILE_T *profile);
int ql_enable_qmi_wwan_rawip_mode(PROFILE_T *profile);
int ql_driver_type_detect(PROFILE_T *profile);
int ql_qmap_mode_detect(PROFILE_T *profile);
extern const struct qmi_device_ops gobi_qmidev_ops;
extern const struct qmi_device_ops qmiwwan_qmidev_ops;

#define qmidev_is_gobinet(_qmichannel) (strncmp(_qmichannel, "/dev/qcqmi", strlen("/dev/qcqmi")) == 0)
#define qmidev_is_qmiwwan(_qmichannel) (strncmp(_qmichannel, "/dev/cdc-wdm", strlen("/dev/cdc-wdm")) == 0)
#define qmidev_is_pciemhi(_qmichannel) (strncmp(_qmichannel, "/dev/mhi_", strlen("/dev/mhi_")) == 0)

#define driver_is_qmi(_drv_name) (strncasecmp(_drv_name, "qmi_wwan", strlen("qmi_wwan")) == 0)
#define driver_is_mbim(_drv_name) (strncasecmp(_drv_name, "cdc_mbim", strlen("cdc_mbim")) == 0)

extern FILE *logfilefp;
extern bool do_syslog;
extern int debug_qmi;
extern int qmidevice_control_fd[2];
extern int qmiclientId[QMUX_TYPE_WDS_ADMIN + 1];
extern USHORT le16_to_cpu(USHORT v16);
extern UINT  le32_to_cpu (UINT v32);
extern UINT  ql_swap32(UINT v32);
extern USHORT cpu_to_le16(USHORT v16);
extern UINT cpu_to_le32(UINT v32);
extern void update_resolv_conf(int iptype, const char *ifname, const char *dns1, const char *dns2);

enum
{
    SIG_EVENT_START,
    SIG_EVENT_CHECK,
    SIG_EVENT_STOP,
};

#define CM_MAX_BUFF 256
#define strset(k, v) {if (k) free(k); k = strdup(v);}
#define mfree(v) {if (v) {free(v); v = NULL;}

#ifdef CM_DEBUG
#define dbg_time(fmt, args...) do { \
    fprintf(stdout, "[%15s-%04d: %s] " fmt "\n", __FILE__, __LINE__, get_time(), ##args); \
    if (logfilefp) fprintf(logfilefp, "[%s-%04d: %s] " fmt "\n", __FILE__, __LINE__, get_time(), ##args); \
    if (do_syslog) syslog(LOG_DEBUG, fmt, ##args); \
} while(0);
#else
#define dbg_time(fmt, args...) do { \
    fprintf(stdout, "[%s] " fmt "\n", get_time(), ##args); \
    if (logfilefp) fprintf(logfilefp, "[%s] " fmt "\n", get_time(), ##args); \
    if (do_syslog) syslog(LOG_DEBUG, fmt, ##args); \
} while(0);
#endif
#endif
