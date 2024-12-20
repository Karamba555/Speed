/*
 * _stm_i2c_netlink.h
 *
 *  Created on: 24 Oct 2024
 *      Author: Sergey Frolov
 */

#ifndef __STM_I2C_NETLINK_H_
#define __STM_I2C_NETLINK_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/arm-smccc.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/hw_random.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <net/sock.h>

#include "_stm_i2c.h"


int _stm_i2c_netlink_init(void);
void _stm_i2c_netlink_deinit(void);

#endif /* __STM_I2C_NETLINK_H_ */