/*
 * _stm_i2c_netlink.c
 *
 *  Created on: 24 Oct 2024
 *      Author: Sergey Frolov
*/

#include "_stm_i2c_netlink.h"

/* 
 * cannot be larger than 31, otherwise we shall get "
 * insmod: ERROR: could not insert module 
 * mtk_smc.ko: No child processes"
 */ 
#define MY_NETLINK          30

struct sock *nl_sk = NULL;
static struct task_struct *nlink_th;
static int endthread = 0;
static void _stm_i2c_netlink_recv_msg(struct sk_buff *skb);
static void _stm_i2c_netlink_send_msg(uint8_t *data, int len);

/* ----------------------------------------------------------------------------------- */
static void _stm_i2c_netlink_recv_msg(struct sk_buff *skb)
{
    int stat = STMI2C_ERROR;
    uint8_t *nldata;
    uint8_t i2crxdata[64];
    uint8_t nllen = 0;
    int txlen = 0;
    int rxlen = 0;
    int pid = 0;
    int res = 0;
    struct sk_buff *skb_out;
    struct nlmsghdr *nlhead;
    nlhead = (struct nlmsghdr*)skb->data;
    nldata = (uint8_t*)nlmsg_data(nlhead);
    nllen = nlhead->nlmsg_len;
    //i2crxdata = kmalloc(sizeof(uint8_t)*I2C_PACKET_LEN, GFP_KERNEL);
    if(nllen == I2C_PACKET_LEN)
    {
        switch(*(nldata))
        {
            case EVENT_UPGRADE:
            {
                txlen = *(nldata+3);
                rxlen = 0x00;
                stat = _stm_i2c_transfer(nldata, &i2crxdata[0], txlen, rxlen);
            }
            break;
            case EVENT_VERSION:
            {
                txlen = *(nldata+3);
                rxlen = *(nldata+7);
                stat = _stm_i2c_transfer(nldata, &i2crxdata[0], txlen, rxlen);
            }
            break;
            case EVENT_RTC_GET:
            {
                txlen = *(nldata+3);
                rxlen = *(nldata+7);
                stat = _stm_i2c_transfer(nldata, &i2crxdata[0], txlen, rxlen);
            }
            break;
            case EVENT_RTC_SET:
            {
                txlen = *(nldata+3);
                rxlen = *(nldata+7);
                stat = _stm_i2c_transfer(nldata, &i2crxdata[0], txlen, rxlen);
            }
            break;
            case EVENT_VST_SET:
            {
                txlen = *(nldata+3);
                rxlen = *(nldata+7);
                stat = _stm_i2c_transfer(nldata, &i2crxdata[0], txlen, rxlen);
            }
            break;
            case EVENT_STDBY_CD:
            {
                txlen = *(nldata+3);
                rxlen = *(nldata+7);
                stat = _stm_i2c_transfer(nldata, &i2crxdata[0], txlen, rxlen);
            }
            break;
        }
        pid = nlhead->nlmsg_pid;
        if(rxlen <= 0) rxlen = 16;
        //nlmsg_new - Allocate a new netlink message: skb_out
        skb_out = nlmsg_new(rxlen, 0);
        if(!skb_out)
        {
            printk(KERN_ERR "Failed to allocate new skb.\n");
            return;
        }
        // Add a new netlink message to an skb
        nlhead = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, rxlen, 0);
        NETLINK_CB(skb_out).dst_group = 0;
        memcpy(nlmsg_data(nlhead), &i2crxdata[0], rxlen);
        res = nlmsg_unicast(nl_sk, skb_out, pid);
        if(res < 0)
        {
            printk(KERN_INFO "Error while sending back to user");
        }
    }
}
/* ----------------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------------------- */
int _stm_i2c_netlink_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .input = _stm_i2c_netlink_recv_msg,
    };
    //netlink_kernel_create() returns a pointer, should be checked with == NULL
    nl_sk = netlink_kernel_create(&init_net, MY_NETLINK, &cfg);
    if(!nl_sk)
    {
        printk(KERN_INFO "Error creating socket.\n");
        return -10;
    }
    printk(KERN_INFO "STMI2C: NetLink Init OK!\n");
    return 0;
}
/* ----------------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------------- */
void _stm_i2c_netlink_deinit(void)
{
    printk(KERN_INFO "STMI2C: Exiting NetLink module\n");
    netlink_kernel_release(nl_sk);
}
/* ----------------------------------------------------------------------------------- */