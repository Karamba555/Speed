#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>
#include <stdint.h>

#define NETLINK_USER 30 // same customized protocol as in my kernel module
#define MAX_PAYLOAD 1024 // maximum payload size

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct nlmsghdr *nlh2 = NULL;
// famous struct msghdr, it includes "struct iovec *   msg_iov;"
struct msghdr msg, resp;
struct iovec iov, iov2;
int sock_fd;

int main(int args, char *argv[])
{
    int ret = -1;
    uint8_t *smc_args = (uint8_t*)malloc(sizeof(uint8_t*)*40);
    unsigned long smc_fid;
    unsigned long a1;
    unsigned long a2;
    unsigned long a3;
    unsigned long a4;
    if(args != 6)
    {
        printf("Not enough arguments.\n");
        return -1;
    }

    sscanf(argv[1], "%lx",&smc_fid);
    sscanf(argv[2], "%lx",&a1);
    sscanf(argv[3], "%lx",&a2);
    sscanf(argv[4], "%lx",&a3);
    sscanf(argv[5], "%lx",&a4);
    printf("FID: %lx\n", smc_fid);
    printf("A1: %lx\n", a1);
    printf("A2: %lx\n", a2);
    printf("A3: %lx\n", a3);
    printf("A4: %lx\n", a4);

    memcpy((uint8_t*)&smc_args[0],(uint8_t*)&smc_fid,sizeof(unsigned long));
    memcpy((uint8_t*)&smc_args[8],(uint8_t*)&a1,sizeof(unsigned long));
    memcpy((uint8_t*)&smc_args[16],(uint8_t*)&a2,sizeof(unsigned long));
    memcpy((uint8_t*)&smc_args[24],(uint8_t*)&a3,sizeof(unsigned long));
    memcpy((uint8_t*)&smc_args[32],(uint8_t*)&a4,sizeof(unsigned long));

    usleep(50000);

    //----------------------------------------------------------------------
    //int socket(int domain, int type, int protocol);
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER); //NETLINK_KOBJECT_UEVENT  

    if(sock_fd < 0)
    {
        printf("Socket open failed.\n");
        return -1;
    }
    //printf("Socket opened OK.\n");    

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */

    //int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    if(bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)))
    {
        perror("bind() error\n");
        close(sock_fd);
        printf("Unable bind socket.\n");
        return -1;
    }
    //printf("Socket binded OK.\n");

    //----------------------------------------------------------------------
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;       /* For Linux Kernel */
    dest_addr.nl_groups = 0;    /* unicast */

    
    //nlh: contains "Hello" msg
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();  //self pid
    nlh->nlmsg_flags = 0;


    //nlh2: contains received msg
    /*
    nlh2 = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh2, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh2->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh2->nlmsg_pid = getpid();  //self pid
    nlh2->nlmsg_flags = 0;
    */


    iov.iov_base = (void *)nlh;         //iov -> nlh
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;  //msg_name is Socket name: dest
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;                 //msg -> iov
    msg.msg_iovlen = 1;

    /*
    iov2.iov_base = (void *)nlh2;         //iov -> nlh2
    iov2.iov_len = nlh2->nlmsg_len;
    resp.msg_name = (void *)&dest_addr;  //msg_name is Socket name: dest
    resp.msg_namelen = sizeof(dest_addr);
    resp.msg_iov = &iov2;                 //resp -> iov
    resp.msg_iovlen = 1;
    */

    //----------------------------------------------------------------------

    

    //put "Hello" msg into nlh
    //strcpy(NLMSG_DATA(nlh), "Hello this is a msg from userspace");
    //memcpy((char*)NLMSG_DATA(nlh),(char*)umes,(strlen(umes)+0));
    memcpy((uint8_t*)NLMSG_DATA(nlh),(uint8_t*)smc_args,(sizeof(uint8_t*)*40));
  
    //printf("Sending message to kernel\n");

    //int ret = sendmsg(sock_fd, &msg, 0);   
   // printf("send ret: %d\n", ret); 

    //printf("Waiting for message from kernel\n");

    /* Read message from kernel */
    //recvmsg(sock_fd, &resp, 0);  //msg is also receiver for read

    //printf("Received message payload: %s\n", (char *) NLMSG_DATA(nlh2)); 

    //printf("Apply data\n");
    //memcpy(NLMSG_DATA(nlh),&smc_fid,sizeof(unsigned long));
    //printf("Send msg\n");
    ret = sendmsg(sock_fd, &msg, 0);
    //printf("Send result %d\n", ret);



    char usermsg[MAX_PAYLOAD];
    while (1)
    {
        //printf("Input your msg for sending to kernel: ");
        //scanf("%s", usermsg);
        scanf("%s", usermsg);

        //put "Hello" msg into nlh
        strcpy(NLMSG_DATA(nlh), usermsg);


        //printf("Sending message \" %s \" to kernel\n", usermsg);

        ret = sendmsg(sock_fd, &msg, 0);   
        //printf("send ret: %d\n", ret); 

        //printf("Waiting for message from kernel\n");

        /* Read message from kernel */
        //msg is also receiver for read
        //recvmsg(sock_fd, &resp, 0);
        //printf("Received message payload: %s\n", (char *)NLMSG_DATA(nlh2));   

    }
    close(sock_fd);

    return 0;
}
