/*
 * stm_i2c_main.h
 *
 *  Created on: 24 Oct 2024
 *      Author: Sergey Frolov
 */

#ifndef __STM_I2C_MAIN_H_
#define __STM_I2C_MAIN_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/syscalls.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/of_irq.h>
#include <linux/regmap.h>
#include <linux/platform_device.h>

#include "_stm_i2c_netlink.h"


enum
{
    DRV_NO,
    DRV_OK
};

#endif /* __STM_I2C_MAIN_H_ */