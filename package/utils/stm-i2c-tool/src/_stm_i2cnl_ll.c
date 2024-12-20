/*
 * _stm_i2c_nl_ll.c
 *
 *  Created on: 24 Oct 2024
 *      Author: Sergey Frolov
*/

#include "_stm_i2cnl_ll.h"

//same customized protocol as in kernel module
#define NETLINK_USER    30
//maximum payload size
#define MAX_PAYLOAD     1024

STMI2C_Tool_TD          stmi2c_o;

struct sockaddr_nl      src_addr;
struct sockaddr_nl      dest_addr;
struct nlmsghdr         *nlh = NULL;
struct nlmsghdr         *nlh2 = NULL;
struct msghdr           msg;
struct msghdr           resp;
struct iovec            iov;
struct iovec            iov2;
int sock_fd;



/***************************************************************************************/
/* LOW LEVEL I2C/NETLINK INIT STATIC SECTION */
/* ----------------------------------------------------------------------------------- */
/** Returns true on success, or false if there was an error */
static int _stm_i2cnl_set_socket_blocking(int fd, int blocking)
{
   if (fd < 0) return 0;
   int flags = fcntl(fd, F_GETFL, 0);
   if (flags == -1) return 0;
   flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0);
}
/* ----------------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */
static int _stm_i2cnl_netlink_init(STMI2C_Tool_TD *stmi2c)
{
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if(sock_fd < 0)
    {
        printf("Socket open failed.\n");
        return STMI2C_ERROR;
    }

    _stm_i2cnl_set_socket_blocking(sock_fd, 1);

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();

    if(bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)))
    {
        perror("bind() error\n");
        close(sock_fd);
        return STMI2C_ERROR;
    }
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    // For Linux Kernel
    dest_addr.nl_pid = 0;
    // unicast
    dest_addr.nl_groups = 0;

    //nlh: contains Tx msg
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    //self pid
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    //nlh2: contains Rx msg
    nlh2 = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh2, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh2->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    //self pid
    nlh2->nlmsg_pid = getpid();
    nlh2->nlmsg_flags = 0;

    //iov -> nlh
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    //msg_name is Socket name: dest
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    //msg -> iov
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    //iov -> nlh2
    iov2.iov_base = (void *)nlh2;
    iov2.iov_len = nlh2->nlmsg_len;
    //msg_name is Socket name: dest
    resp.msg_name = (void *)&dest_addr;
    resp.msg_namelen = sizeof(dest_addr);
    //resp -> iov
    resp.msg_iov = &iov2;
    resp.msg_iovlen = 1;
    stmi2c->nlready = 1;
    return STMI2C_OK;
}
/* ----------------------------------------------------------------------------------- */
/***************************************************************************************/



/***************************************************************************************/
/* LOW LEVEL I2C/NETLINK OPEN/WRITE/READ  SECTION */
/* ----------------------------------------------------------------------------------- */
int _stm_i2cnl_open(STMI2C_Tool_TD *stmi2c)
{
    int stat = STMI2C_ERROR;
    if(stmi2c->nlready != 0) return STMI2C_OK;
    stat = _stm_i2cnl_netlink_init(stmi2c);
    return stat;
}
/* ----------------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------------- */
int _stm_i2cnl_write(STMI2C_Tool_TD *stmi2c, uint8_t *data, uint32_t len)
{
    int stat = STMI2C_ERROR;
    if(len == 0) return STMI2C_OK;
    nlh->nlmsg_len = len;
    memcpy((uint8_t*)NLMSG_DATA(nlh), (uint8_t*)&data[0], len);
    stat = sendmsg(sock_fd, &msg, 0);
    if(stat == -1)
    {
        printf("I2CNL: Error write to Netlink. %s\n", strerror(errno));
        return STMI2C_ERROR;
    }
    return STMI2C_OK;
}
/* ----------------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */
int _stm_i2cnl_read(STMI2C_Tool_TD *stmi2c, uint8_t *data, uint32_t len)
{
    int stat = STMI2C_ERROR;
    if(len == 0) return STMI2C_OK;
    stat = recvmsg(sock_fd, &resp, 0);
    if(stat == -1)
    {
        printf("I2CNL: Error read Netlink. %s\n", strerror(errno));
        return STMI2C_ERROR;  
    }
    memcpy((uint8_t*)data, (uint8_t*)NLMSG_DATA(nlh2), len);
    return STMI2C_OK;
}
/* ----------------------------------------------------------------------------------- */
/***************************************************************************************/