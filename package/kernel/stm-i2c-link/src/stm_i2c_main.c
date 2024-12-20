/*
 * stm_i2c_main.c
 *
 *  Created on: 24 Oct 2024
 *      Author: Sergey Frolov
*/

#include <linux/kobject.h>
#include <linux/sysfs.h>

#include "stm_i2c_main.h"

#define DRIVER_NAME 						"stm-i2c-dev"

struct i2c_adapter* i2c_adapter;
struct i2c_client*  i2c_client;
int drvready = DRV_NO;


static int stm_i2c_probe(struct i2c_client *client,
						const struct i2c_device_id *dev_id);
static int stm_i2c_remove(struct i2c_client *client);

/****************************************************************************/
static struct i2c_board_info __initdata board_info[] =  {
	{
		I2C_BOARD_INFO("stm_i2c", STM_I2C_ADDR),
	}
};
/****************************************************************************/

/****************************************************************************/
static const struct i2c_device_id stm_i2c_id[] = {
    {"stm_i2c", STM_I2C_ADDR},
	{ }
};
MODULE_DEVICE_TABLE(i2c, stm_i2c_id);
/****************************************************************************/

/****************************************************************************/
/*
 *  Driver handler
 */
static struct i2c_driver stm_i2c_driver = {
	.driver = {
		.name = "stm_i2c",
		.owner  = THIS_MODULE,
	},
	.probe 		= stm_i2c_probe,
	.remove     = stm_i2c_remove,
	.id_table	= stm_i2c_id,
};
/****************************************************************************/


/****************************************************************************/
/*
 *  Probe Driver
 */
int stm_i2c_probe(struct i2c_client *client,
				const struct i2c_device_id *dev_id)
{
	/* Driver not ready flag */
    /* Initialize Netlink bridge */
    _stm_i2c_netlink_init();
	//printk("STMI2C: End driver register");
	/* Driver is Ready set flag */
   	return 0;
}
/****************************************************************************/

/****************************************************************************/
/*
 *  Remove Driver
 */
int stm_i2c_remove(struct i2c_client *client)
{
	//Do something
	return 0;
}
/****************************************************************************/

/****************************************************************************/
/*
 *  Initializes Kernel module
 */
static int __init stm_i2c_link_init_driver(void)
{
	int stat = 0;
	//printk(KERN_INFO "STMI2C: Get I2C adapter.");
	i2c_adapter = i2c_get_adapter(STM_I2C_BUS);
    if(i2c_adapter != NULL )
    {
		printk(KERN_INFO "STMI2C: Get I2C client.\n");
        i2c_client = i2c_new_client_device(i2c_adapter, &board_info[0]);
        if(i2c_client != NULL)
        {
			printk(KERN_INFO "STMI2C: Adding I2C driver.\n");
            stat = i2c_add_driver(&stm_i2c_driver);
            stat = 0;
        }
		printk(KERN_INFO "STMI2C: Put I2C adapter.\n");
        i2c_put_adapter(i2c_adapter);
    }
	return stat;
}
/****************************************************************************/

/****************************************************************************/
/*
 *  Exit from Kernel module
 */
static void __exit stm_i2c_link_exit_driver(void)
{
	_stm_i2c_netlink_deinit();
	i2c_unregister_device(i2c_client);
    i2c_del_driver(&stm_i2c_driver);
	return;
}
/****************************************************************************/


module_init(stm_i2c_link_init_driver);
module_exit(stm_i2c_link_exit_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sergey Frolov");
MODULE_DESCRIPTION("Driver for ETH");
MODULE_VERSION("0.1");