/******************************************************************************
  @file    qmap_bridge_mode.c
  @brief   Connectivity bridge manager.

  DESCRIPTION
  Connectivity Management Tool for USB network adapter of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include "QMIThread.h"

static size_t ql_fread(const char *filename, void *buf, size_t size) {
    FILE *fp = fopen(filename , "r");
    int n = 0;

    memset(buf, 0x00, size);

    if (fp) {
        n = fread(buf, 1, size, fp);
        if (n <= 0 || n == (int)size) {
            dbg_time("warnning: fail to fread(%s), fread=%d, buf_size=%zu: (%s)", filename, n, size, strerror(errno));
        }
        fclose(fp);
    }

    return n > 0 ? n : 0;
}

static size_t ql_fwrite(const char *filename, const void *buf, size_t size) {
    FILE *fp = fopen(filename , "w");
    int n = 0;

    if (fp) {
        n = fwrite(buf, 1, size, fp);
        if (n != (int)size) {
            dbg_time("warnning: fail to fwrite(%s), fwrite=%d, buf_size=%zu: (%s)", filename, n, size, strerror(errno));
        }
        fclose(fp);
    }

    return n > 0 ? n : 0;
}

int ql_bridge_mode_detect(PROFILE_T *profile) {
    const char *ifname = profile->qmapnet_adapter ? profile->qmapnet_adapter : profile->usbnet_adapter;
    const char *driver;
    char bridge_mode[128];
    char bridge_ipv4[128];
    char ipv4_h[128];
	char ipv4_d[128];
    char buf[64];
    size_t n;
    int in_bridge = 0;

    driver = profile->driver_name;
    snprintf(bridge_mode, sizeof(bridge_mode), "/sys/class/net/%s/qmi/bridge_mode", ifname);
    snprintf(bridge_ipv4, sizeof(bridge_ipv4), "/sys/class/net/%s/qmi/bridge_ipv4", ifname);

    if (access(bridge_ipv4, R_OK)) {
        if (errno != ENOENT) {
            dbg_time("fail to access %s, errno: %d (%s)", bridge_mode, errno, strerror(errno));
            return 0;
        }

        snprintf(bridge_mode, sizeof(bridge_mode), "/sys/module/%s/parameters/bridge_mode", driver);
        snprintf(bridge_ipv4, sizeof(bridge_ipv4), "/sys/module/%s/parameters/bridge_ipv4", driver);

        if (access(bridge_mode, R_OK)) {
            if (errno != ENOENT) {
                dbg_time("fail to access %s, errno: %d (%s)", bridge_mode, errno, strerror(errno));
            }
            return 0;
        }
    }

    n = ql_fread(bridge_mode, buf, sizeof(buf));
    if (n > 0) {
        in_bridge = (buf[0] != '0');
    }
    if (!in_bridge)
        return 0;
   
    memset(ipv4_h, 0, sizeof(ipv4_h));

	snprintf(ipv4_d, sizeof(ipv4_d), "%u.%u.%u.%u",
			(profile->ipv4.Address >> 24) % 256,
		    (profile->ipv4.Address >> 16) % 256,
		    (profile->ipv4.Address >> 8) %  256,
		     profile->ipv4.Address %  256);
	
	dbg_time("Setting qmi driver bridge IP address to: %s (0x%08x)",
			 ipv4_d, profile->ipv4.Address);
	
    if (strstr(bridge_ipv4, "/sys/class/net/") || profile->qmap_mode == 0 || profile->qmap_mode == 1) {
        snprintf(ipv4_h, sizeof(ipv4_h), "0x%x", profile->ipv4.Address);
        dbg_time("echo '%s' > %s", ipv4_h, bridge_ipv4);
        ql_fwrite(bridge_ipv4, ipv4_h, strlen(ipv4_h));
    }
    else {
        snprintf(ipv4_h, sizeof(ipv4_h), "0x%x:%d", profile->ipv4.Address, profile->muxid);
        dbg_time("echo '%s' > %s", ipv4_h, bridge_ipv4);
        ql_fwrite(bridge_ipv4, ipv4_h, strlen(ipv4_h));
    }
    
    return in_bridge;
}

#if 0
int ql_enable_qmi_wwan_rawip_mode(PROFILE_T *profile) {
    char filename[256];
    char buf[4];
    size_t n;
    FILE *fp;

    if (!qmidev_is_qmiwwan(profile->qmichannel))
        return 0;

    snprintf(filename, sizeof(filename), "/sys/class/net/%s/qmi/raw_ip", profile->usbnet_adapter);
    n = ql_fread(filename, buf, sizeof(buf));

    if (n == 0)
        return 0;

    if (buf[0] == '1' || buf[0] == 'Y')
        return 0;

    fp = fopen(filename , "w");
    if (fp == NULL) {
        dbg_time("Fail to fopen(%s, \"w\"), errno: %d (%s)", filename, errno, strerror(errno));
        return 1;
    }

    buf[0] = 'Y';
    n = fwrite(buf, 1, 1, fp);
    if (n != 1) {
        dbg_time("Fail to fwrite(%s), errno: %d (%s)", filename, errno, strerror(errno));
        fclose(fp);
        return 1;
    }
    fclose(fp);

    return 0;
}
#endif

#if 0
int ql_driver_type_detect(PROFILE_T *profile) {
    if (qmidev_is_gobinet(profile->qmichannel)) {
        profile->qmi_ops = &gobi_qmidev_ops;
    }
    else {
        profile->qmi_ops = &qmiwwan_qmidev_ops;
    }
    qmidev_send = profile->qmi_ops->send;

    return 0;
}
#endif

int ql_qmap_mode_detect(PROFILE_T *profile) {
    char buf[128];
    int n;
    char qmap_netcard[128];
    struct {
        char filename[255 * 2];
        char linkname[255 * 2];
    } *pl;
    
    pl = (typeof(pl)) malloc(sizeof(*pl));

    if (profile->qmapnet_adapter) {
        free(profile->qmapnet_adapter);;
        profile->qmapnet_adapter = NULL;
    }
    
    snprintf(pl->linkname, sizeof(pl->linkname), "/sys/class/net/%s/device/driver", profile->usbnet_adapter);
    n = readlink(pl->linkname, pl->filename, sizeof(pl->filename));
    pl->filename[n] = '\0';
    while (pl->filename[n] != '/')
        n--;
    strset(profile->driver_name, &pl->filename[n+1]);

    snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/%s/qmi/qmap_mode", profile->usbnet_adapter);
    if (access(pl->filename, R_OK)) {
        if (errno != ENOENT) {
            dbg_time("fail to access %s, errno: %d (%s)", pl->filename, errno, strerror(errno));
            goto _out;
        }
        snprintf(pl->filename, sizeof(pl->filename), "/sys/module/%s/parameters/qmap_mode", profile->driver_name);
        if (access(pl->filename, R_OK)) {
            if (errno != ENOENT) {
                dbg_time("fail to access %s, errno: %d (%s)", pl->filename, errno, strerror(errno));
                goto _out;
            }
        }
    }

    if (!access(pl->filename, R_OK)) {
        n = ql_fread(pl->filename, buf, sizeof(buf));
        if (n > 0) {
            profile->qmap_mode = atoi(buf);
            
            if (profile->qmap_mode > 1) {
                profile->muxid = profile->pdp + 0x80; //muxis is 0x8X for PDN-X
                sprintf(qmap_netcard, "%s.%d", profile->usbnet_adapter, profile->pdp);
                profile->qmapnet_adapter = strdup(qmap_netcard);
           } if (profile->qmap_mode == 1) {
                profile->muxid = 0x81;
                profile->qmapnet_adapter = strdup(profile->usbnet_adapter);
           }
        }
    }
    else if (qmidev_is_qmiwwan(profile->qmichannel)) {
        snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/qmimux%d", profile->pdp - 1);
        if (access(pl->filename, R_OK)) {
            if (errno != ENOENT) {
                dbg_time("fail to access %s, errno: %d (%s)", pl->filename, errno, strerror(errno));
            }
            goto _out;
        }

        //upstream Kernel Style QMAP qmi_wwan.c
        snprintf(pl->filename, sizeof(pl->filename), "/sys/class/net/%s/qmi/add_mux", profile->usbnet_adapter);
        n = ql_fread(pl->filename, buf, sizeof(buf));
        if (n >= 5) {
            dbg_time("If use QMAP by /sys/class/net/%s/qmi/add_mux", profile->usbnet_adapter);
            dbg_time("File:%s Line:%d Please make sure add next patch to qmi_wwan.c", __func__, __LINE__);
            /*
            diff --git a/drivers/net/usb/qmi_wwan.c b/drivers/net/usb/qmi_wwan.c
            index 74bebbd..db8a777 100644
            --- a/drivers/net/usb/qmi_wwan.c
            +++ b/drivers/net/usb/qmi_wwan.c
            @@ -379,6 +379,24 @@ static ssize_t add_mux_store(struct device *d,  struct device_attribute *attr, c
                if (!ret) {
                        info->flags |= QMI_WWAN_FLAG_MUX;
                        ret = len;
            +#if 1 //Add by Quectel
            +               if (le16_to_cpu(dev->udev->descriptor.idVendor) == 0x2c7c) {
            +                       int idProduct = le16_to_cpu(dev->udev->descriptor.idProduct);
            +
            +                       if (idProduct == 0x0121 || idProduct == 0x0125 || idProduct == 0x0435) //MDM9x07
            +                               dev->rx_urb_size = 4*1024;
            +                       else if (idProduct == 0x0306) //MDM9x40
            +                               dev->rx_urb_size = 16*1024;
            +                       else if (idProduct == 0x0512) //SDX20
            +                               dev->rx_urb_size = 32*1024;
            +                       else if (idProduct == 0x0620) //SDX24
            +                               dev->rx_urb_size = 32*1024;
            +                       else if (idProduct == 0x0800) //SDX55
            +                               dev->rx_urb_size = 32*1024;
            +                       else
            +                               dev->rx_urb_size = 32*1024;
            +               }
            +#endif
                }
            err:
                rtnl_unlock();
            */
            profile->qmap_mode = n/5; //0x11\n0x22\n0x33\n
            if (profile->qmap_mode > 1) {
                //PDN-X map to qmimux-X
                profile->muxid = (buf[5*(profile->pdp - 1) + 2] - '0')*16 + (buf[5*(profile->pdp - 1) + 3] - '0');
                sprintf(qmap_netcard, "qmimux%d", profile->pdp - 1);
                profile->qmapnet_adapter = strdup(qmap_netcard);
            } else if (profile->qmap_mode == 1) {
                profile->muxid = (buf[5*0 + 2] - '0')*16 + (buf[5*0 + 3] - '0');
                sprintf(qmap_netcard, "qmimux%d", 0);
                profile->qmapnet_adapter = strdup(qmap_netcard);
            }
        }
    } 

_out:
    if (profile->qmap_mode) {
        dbg_time("qmap_mode = %d, muxid = 0x%02x, qmap_netcard = %s",
            profile->qmap_mode, profile->muxid, profile->qmapnet_adapter);
    }

    free(pl);

    return 0;
}

