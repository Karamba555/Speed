

#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>


#include <linux/init.h>
#include <linux/types.h>
#include <linux/arm-smccc.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/hw_random.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/soc/mediatek/mtk_sip_svc.h>



/* 
 * cannot be larger than 31, otherwise we shall get "
 * insmod: ERROR: could not insert module 
 * mtk_smc.ko: No child processes"
 */ 
#define MY_NETLINK 30


struct sock *nl_sk = NULL;

static void myNetLink_recv_msg(struct sk_buff *skb)
{
	struct arm_smccc_res smc_res;
	unsigned long fid = 0;
	unsigned long a1 = 0;
	unsigned long a2 = 0;
	unsigned long a3 = 0;
	unsigned long a4 = 0;
    uint8_t *s1;

    struct nlmsghdr *nlhead;
    //struct sk_buff *skb_out;
    //int pid, res;
    int msg_size = 0;
    char *msg = "SMC: Received command.";

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
    msg_size = strlen(msg);

	//nlhead message comes from skb's data... (sk_buff: unsigned char *data)
    nlhead = (struct nlmsghdr*)skb->data;

	s1 = (uint8_t*)nlmsg_data(nlhead);
	memcpy((uint8_t*)&fid,(uint8_t*)&s1[0],8);
	memcpy((uint8_t*)&a1,(uint8_t*)&s1[8],8);
	memcpy((uint8_t*)&a2,(uint8_t*)&s1[16],8);
	memcpy((uint8_t*)&a3,(uint8_t*)&s1[24],8);
	memcpy((uint8_t*)&a4,(uint8_t*)&s1[32],8);
	//int i = 0;
	//for(i==0;i<8;i++)
	//{
	//	printk("%2x\n", *(s1+i));
	//}
    printk("SMC: FID: %lx\n", fid);
    printk("SMC: A1: %lx\n", a1);
    printk("SMC: A2: %lx\n", a2);
    printk("SMC: A3: %lx\n", a3);
    printk("SMC: A4: %lx\n", a4);
	printk("------------------------\n");
	//printk(KERN_INFO "SMC has received1: %s\n", *s1);
    //printk(KERN_INFO "SMC RX: %s\n",(char*)nlmsg_data(nlhead));
	//printk(KERN_INFO "SMC has received3: %s\n",(uint8_t*)nlmsg_data(nlhead));
	//fid = (unsigned long*)nlmsg_data(nlhead);

	printk("SMC: Send...\n");
	arm_smccc_smc(fid, a1, a2, a3, a4, 0, 0, 0, &smc_res);
	printk(KERN_INFO "R0: %lx\n", smc_res.a0);
	printk(KERN_INFO "R1: %lx\n", smc_res.a1);
	printk(KERN_INFO "R2: %lx\n", smc_res.a2);
	printk(KERN_INFO "R3: %lx\n", smc_res.a3);

	/*
	// Sending process port ID, will send new message back to the 'user space sender'
    pid = nlhead->nlmsg_pid; 

	//nlmsg_new - Allocate a new netlink message: skb_out
    skb_out = nlmsg_new(msg_size, 0);

    if(!skb_out)
    {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }

	// Add a new netlink message to an skb
    nlhead = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);

    NETLINK_CB(skb_out).dst_group = 0;                  

	//char *strncpy(char *dest, const char *src, size_t count)
    strncpy(nlmsg_data(nlhead), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid); 

    if(res < 0)
        printk(KERN_INFO "Error while sending back to user\n");
	*/
}

static int __init myNetLink_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .input = myNetLink_recv_msg,
    };

    /*netlink_kernel_create() returns a pointer, should be checked with == NULL */
    nl_sk = netlink_kernel_create(&init_net, MY_NETLINK, &cfg);
    printk("Entering: %s, protocol family = %d \n",__FUNCTION__, MY_NETLINK);
    if(!nl_sk)
    {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    printk("MyNetLink Init OK!\n");
    return 0;
}

static void __exit myNetLink_exit(void)
{
    printk(KERN_INFO "exiting myNetLink module\n");
    netlink_kernel_release(nl_sk);
}

module_init(myNetLink_init);
module_exit(myNetLink_exit);
MODULE_LICENSE("GPL");
