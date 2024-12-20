/*
 * _stm_i2cnl_cmds.h
 *
 *  Created on: 24 Oct 2024
 *      Author: Sergey Frolov
 */

#ifndef __STM_I2CNL_CMDS_H_
#define __STM_I2CNL_CMDS_H_


#include "_stm_i2cnl_ll.h"



int _stm_i2cnl_version(STMI2C_Tool_TD *stmi2c);
int _stm_i2cnl_rtc_get(STMI2C_Tool_TD *stmi2c, int subevent);
int _stm_i2cnl_rtc_set(STMI2C_Tool_TD *stmi2c, char *hour, char *min, char *sec,
                                         char *date, char *month, char *year);
int _stm_i2cnl_upgrade(STMI2C_Tool_TD *stmi2c);
int _stm_i2cnl_vst_set(STMI2C_Tool_TD *stmi2c, char *vst);
int _stm_i2cnl_stdbycd_get(STMI2C_Tool_TD *stmi2c);
int _stm_i2cnl_cpu_reset(STMI2C_Tool_TD *stmi2c);
int _stm_i2cnl_mcu_reset(STMI2C_Tool_TD *stmi2c);

#endif /* __STM_I2CNL_CMDS_H_ */