/*
 * stm_i2c.c
 *
 *  Created on: 12 June 2024
 *      Author: Sergey Frolov
*/

#include "_stm_i2c.h"

extern struct i2c_adapter* i2c_adapter;
extern struct i2c_client*  i2c_client;

/*
int _stm_i2c_ping(void)
{
    int stat = STMI2C_ERROR;
    int value = 0;
	i2c_smbus_write_byte_data(i2c_client, EVENT_NULL, STM_I2C_PING);
    value = i2c_smbus_read_byte_data(i2c_client, STM_I2C_PING);
    if(value == STM_I2C_PING) stat = STMI2C_OK;
	return stat;
}
*/

/* ----------------------------------------------------------------------------------- */
int _stm_i2c_transfer(uint8_t *txdata, uint8_t *rxdata, int txlen, int rxlen)
{
    int stat = STMI2C_ERROR;
    int i = 0;
    stat = i2c_master_send(i2c_client, (char*)&txdata[0], txlen);
    if(stat == -1) return STMI2C_ERROR;
    if(rxlen == 0x00) return STMI2C_OK;
    //printk("Receive I2C: TX: %d RX: %d \n", txlen, rxlen);
    stat = i2c_master_recv(i2c_client, (char*)&rxdata[0], rxlen);
    if(stat == -1) return STMI2C_ERROR;
    //for(i=0;i<rxlen;i++)
    //{
    //    printk("%d: %02x\n", i, rxdata[i]);
    //}
	return STMI2C_OK;
}
/* ----------------------------------------------------------------------------------- */