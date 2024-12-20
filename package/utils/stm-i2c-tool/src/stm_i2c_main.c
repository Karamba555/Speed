/*
 * stm_i2c_main.c
 *
 *  Created on: 24 Oct 2024
 *      Author: Sergey Frolov
*/

#include "stm_i2c_main.h"

extern STMI2C_Tool_TD               stmi2c_o;
STMI2C_Func_TD                      fops;

/***************************************************************************************/
void print_help(void)
{
    printf("Available options:\n");
    printf("    --help: Print available options.\n");
    printf("    --version: Get version of main firmware\n");
    printf("    --rtc-get-time: Get RTC Time value\n");
    printf("    --rtc-get-date: Get RTC Date value\n");
    printf("    --rtc-set-time <hour> <min> <sec>: Set RTC Time value\n");
    printf("    --rtc-set-date <date> <month> <year>: Set RTC Date value\n");
    printf("    --upgrade: Upgrade MCU with default binary file.\n");
    printf("    --vst-set <value>: Set vehicle ignition sense shutdown value.\n");
    printf("    --stdby-cd: Get the countdown to vehicle shutdown.\n");
    printf("    --cpu-reset: Power cycle on MT**** CPU will be reset.\n");
    printf("    --startmain: Start main firmware.\n");;
}
/* ----------------------------------------------------------------------------------- */
/**
  * @brief  Links needed functions depending on type of communication:
  * 1) direct I2C interface (-i option)
  * 2) Netlink bridge (stm-i2c-link driver should be started) (-n option)
  * @param  STMI2C_Tool_TD *stmi2c
  * @retval Status: STMI2C_OK or STMI2C_EROR
  */
int link_functions(STMI2C_Tool_TD *stmi2c)
{
    int stat = STMI2C_ERROR;
    stmi2c->nlready = 0;
    fops.GetVersion = _stm_i2cnl_version;
    fops.GetRTC = _stm_i2cnl_rtc_get;
    fops.SetRTC = _stm_i2cnl_rtc_set;
    fops.Upgrade = _stm_i2cnl_upgrade;
    fops.SetVST = _stm_i2cnl_vst_set;
    fops.GetSTBCD = _stm_i2cnl_stdbycd_get;
    fops.ResetCPU = _stm_i2cnl_cpu_reset;
    fops.ResetMCU = _stm_i2cnl_mcu_reset;
    stat = STMI2C_OK;
    return stat;
}
/* ----------------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------------- */
/**
  * @brief  Main function. Parsing needed params.
  * @param  int argc, char **argv
  * @param  char **argv (char array arguments)
  * @retval Status: STMI2C_OK or STMI2C_EROR
  */
int main(int argc, char **argv)
{
    int stat = STMI2C_ERROR;
    if(argc == 1)
    {
        printf("Not enough parameters. Add --help to see more.\n");
        return STMI2C_OK;
    }
    else if(argc >= 2)
    {
        if(link_functions(&stmi2c_o))
        {
            printf("ERROR: Couldn't link application.\n");
            return STMI2C_OK;            
        }
        /* Print help */
        if(strcmp(argv[1], "--help") == 0)
        {
            print_help();
            return STMI2C_OK;
        }
        /* Get current version */
        else if(strcmp(argv[1], "--version") == 0)
        {
            fops.GetVersion(&stmi2c_o);
        }
        /* Get RTC value Time */
        else if(strcmp(argv[1], "--rtc-get-time") == 0)
        {
            fops.GetRTC(&stmi2c_o, I2C_RTC_GET_TIME);
        }
        /* Get RTC value Date */
        else if(strcmp(argv[1], "--rtc-get-date") == 0)
        {
            fops.GetRTC(&stmi2c_o, I2C_RTC_GET_DATE);
        }
        /* Set RTC value Time */
        else if(strcmp(argv[1], "--rtc-set-time") == 0)
        {
            if(argc != 5)
            {
                printf("Invalid number of arguments. Add --help to see more.\n");
                return STMI2C_OK;                
            }
            fops.SetRTC(&stmi2c_o, argv[3], argv[4], argv[5], NULL, NULL, NULL);
        }
        /* Set RTC value Date */
        else if(strcmp(argv[1], "--rtc-set-date") == 0)
        {
            if(argc != 5)
            {
                printf("Invalid number of arguments. Add --help to see more.\n");
                return STMI2C_OK;                
            }
            fops.SetRTC(&stmi2c_o, NULL, NULL, NULL, argv[3], argv[4], argv[5]);
        }
        /* Upgrade main firmware */
        else if(strcmp(argv[1], "--upgrade") == 0)
        {
            fops.Upgrade(&stmi2c_o);
        }
        /* Set VST value */
        else if(strcmp(argv[1], "--vst-set") == 0)
        {
            if(argc != 3)
            {
                printf("Invalid number of arguments. Add --help to see more.\n");
                return STMI2C_OK;                
            }
            fops.SetVST(&stmi2c_o, argv[3]);
        }
        /* Set STDBY countdown value */
        else if(strcmp(argv[2], "--stdby-cd") == 0)
        {
            fops.GetSTBCD(&stmi2c_o);
        }
        /* Reset main CPU on power */
        else if(strcmp(argv[1], "--cpu-reset") == 0)
        {
            fops.ResetCPU(&stmi2c_o);
        }
        /* Start main firmware if external Reset event was called */
        else if(strcmp(argv[1], "--startmain") == 0)
        {
            //stat = _stm_i2c_startmain(&i2ct_obj);
        }
    }
    return stat;
}
/* ----------------------------------------------------------------------------------- */
/***************************************************************************************/