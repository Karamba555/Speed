/*
 * _stm_i2cnl_cmds.c
 *
 *  Created on: 24 Oct 2024
 *      Author: Sergey Frolov
*/

#include "_stm_i2cnl_cmds.h"

/* ----------------------------------------------------------------------------------- */
int _stm_i2cnl_version(STMI2C_Tool_TD *stmi2c)
{
    int stat = STMI2C_ERROR;
    int rxlen = 0;
    stat = _stm_i2cnl_open(stmi2c);
    if(stat) return stat;
    memset(stmi2c->rxdata.payload,0xFF,I2C_PACKET_LEN);
    printf("VERSION: Get version ...\n");
    usleep(10000);
    stmi2c->txdata.txheader.h_event = EVENT_VERSION;
    stmi2c->txdata.txheader.h_txlen = I2C_HEADER_LEN;
    stmi2c->txdata.txheader.h_rxlen = 0x01;
    rxlen = stmi2c->txdata.txheader.h_rxlen;
    stat = _stm_i2cnl_write(stmi2c, (uint8_t*)&stmi2c->txdata, I2C_PACKET_LEN);
    if(stat) return stat;
    stat = _stm_i2cnl_read(stmi2c, (uint8_t*)&stmi2c->rxdata.payload[0], rxlen);
    if(stat) return stat;
    printf("VERSION: %02x.\n", (uint8_t)(stmi2c->rxdata.payload[0]));
    return STMI2C_OK;
}
/* ----------------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */
int _stm_i2cnl_rtc_get(STMI2C_Tool_TD *stmi2c, int subevent)
{
    int stat = STMI2C_ERROR;
    int txlen = 0;
    int rxlen = 0;
    stat = _stm_i2cnl_open(stmi2c);
    if(stat) return stat;
    if(subevent != I2C_RTC_GET_EMPTY)
    {
        printf("RTCGET: Get current RTC ...\n");
    }
    stmi2c->txdata.txheader.h_event = EVENT_RTC_GET;
    stmi2c->txdata.txheader.h_txlen = I2C_HEADER_LEN;
    stmi2c->txdata.txheader.h_rxlen = 24;
    rxlen = stmi2c->txdata.txheader.h_rxlen;
    stat = _stm_i2cnl_write(stmi2c, (uint8_t*)&stmi2c->txdata, I2C_PACKET_LEN);
    if(stat) return stat;
    stat = _stm_i2cnl_read(stmi2c, (uint8_t*)&stmi2c->rxdata.payload[0], rxlen);
    if(stat) return stat;
    memcpy((void*)&stmi2c->rtc, (uint8_t*)&stmi2c->rxdata.payload[0], sizeof(stmi2c->rtc));
    if(subevent == I2C_RTC_GET_TIME)
    {
        printf("RTCGET: Hours:          %02x.\n", stmi2c->rtc.Hours);
        printf("RTCGET: Minutes:        %02x.\n", stmi2c->rtc.Minutes);
        printf("RTCGET: Seconds:        %02x.\n", stmi2c->rtc.Seconds);
    }
    else if(subevent == I2C_RTC_GET_DATE)
    {
        printf("RTCGET: WeekDay:        %02x.\n", stmi2c->rtc.WeekDay);
        printf("RTCGET: Month:          %02x.\n", stmi2c->rtc.Month);
        printf("RTCGET: Date:           %02x.\n", stmi2c->rtc.Date);
        printf("RTCGET: Year:           %02x.\n", stmi2c->rtc.Year);
    }
    return STMI2C_OK;
}
/* ----------------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */
int _stm_i2cnl_rtc_set(STMI2C_Tool_TD *stmi2c, char *hour, char *min, char *sec,
                                         char *date, char *month, char *year)
{
    int stat = STMI2C_ERROR;
    uint8_t vhour = 0x00;
    uint8_t vmin = 0x00;
    uint8_t vsec = 0x00;
    uint8_t vdate = 0x00;
    uint8_t vmonth = 0x00;
    uint8_t vyear = 0x00;
    int txlen = 0;
    int rxlen = 0;
    stat = _stm_i2cnl_open(stmi2c);
    if(stat) return stat;
    printf("RTCSET: Get RTC current value ...\n");
    stat = _stm_i2cnl_rtc_get(stmi2c, I2C_RTC_GET_EMPTY);
    if(stat) return stat;       
    printf("RTCSET: Set RTC ...\n");
    stmi2c->txdata.txheader.h_event = EVENT_RTC_SET;
    stmi2c->txdata.txheader.h_txlen = I2C_RTC_PACKET_LEN;
    stmi2c->txdata.txheader.h_rxlen = 24;
    rxlen = stmi2c->txdata.txheader.h_rxlen;
    memcpy(&stmi2c->txdata.payload[0], (void*)&stmi2c->rtc, sizeof(stmi2c->rtc));
    if(hour != NULL) {sscanf(hour, "%hhd", &vhour); stmi2c->txdata.payload[0] = vhour;}
    if(min != NULL) {sscanf(min, "%hhd", &vmin); stmi2c->txdata.payload[1] = vmin;}
    if(sec != NULL) {sscanf(sec, "%hhd", &vsec); stmi2c->txdata.payload[2] = vsec;}
    if(month != NULL) {sscanf(month, "%hhd", &vmonth); stmi2c->txdata.payload[21] = vmonth;}
    if(date != NULL) {sscanf(date, "%hhd", &vdate); stmi2c->txdata.payload[22] = vdate;}
    if(year != NULL) {sscanf(year, "%hhd", &vyear); stmi2c->txdata.payload[23] = vyear;}
    stat = _stm_i2cnl_write(stmi2c, (uint8_t*)&stmi2c->txdata, I2C_PACKET_LEN);
    if(stat) return stat;
    stat = _stm_i2cnl_read(stmi2c, (uint8_t*)&stmi2c->rxdata.payload[0], rxlen);
    if(stat) return stat;
    //printf("RTCSET: Get RTC updated value ...\n");
    //stat = _stm_i2cnl_rtc_get(stmi2c, I2C_RTC_GET_EMPTY);
    //if(stat) return stat;  
    return STMI2C_OK;
}
/* ----------------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------------------- */
int _stm_i2cnl_vst_set(STMI2C_Tool_TD *stmi2c, char *vst)
{
    int stat = STMI2C_ERROR;
    uint32_t vvst = 0x00;
    stat = _stm_i2cnl_open(stmi2c);
    if(stat) return stat;
    printf("VSTSET: Set VST ...\n");
    stmi2c->txdata.txheader.h_event = EVENT_VST_SET;
    stmi2c->txdata.txheader.h_rxlen = 0xFF;
    sscanf(vst, "%d", &vvst); 
    stmi2c->txdata.payload[0] = (vvst & 0x000000FF);
    stmi2c->txdata.payload[0] = (vvst & 0x0000FF00) >> 8;
    stmi2c->txdata.payload[0] = (vvst & 0x00FF0000) >> 16;
    stmi2c->txdata.payload[0] = (vvst & 0xFF000000) >> 24;
    stat = _stm_i2cnl_write(stmi2c, (uint8_t*)&stmi2c->txdata, I2C_PACKET_LEN);
    if(stat) return stat;  
    stat = _stm_i2cnl_read(stmi2c, (uint8_t*)&stmi2c->rxdata.payload[0], 16);
    if(stat) return stat;
    return STMI2C_OK;    
}
/* ----------------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */
int _stm_i2cnl_stdbycd_get(STMI2C_Tool_TD *stmi2c)
{
    int stat = STMI2C_ERROR;
    stat = _stm_i2cnl_open(stmi2c);
    if(stat) return stat;
    memset(stmi2c->rxdata.payload,0xFF,64);
    printf("STDBYCD: Get STDBY countdown ...\n");
    stmi2c->txdata.txheader.h_event = EVENT_STDBY_CD;
    stmi2c->txdata.txheader.h_rxlen = 0x10;
    stat = _stm_i2cnl_write(stmi2c, (uint8_t*)&stmi2c->txdata, I2C_PACKET_LEN);
    if(stat) return stat;
    stat = _stm_i2cnl_read(stmi2c, (uint8_t*)&stmi2c->rxdata.payload[0], sizeof(stmi2c->rxdata));
    if(stat) return stat;
    stmi2c->stdbycd |= stmi2c->rxdata.payload[0] | stmi2c->rxdata.payload[1] |
                    stmi2c->rxdata.payload[2] | stmi2c->rxdata.payload[3];
    printf("STDBYCD: %02x.\n", stmi2c->stdbycd);
    return STMI2C_OK;
}
/* ----------------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */
int _stm_i2cnl_cpu_reset(STMI2C_Tool_TD *stmi2c)
{
    int stat = STMI2C_ERROR;
    stat = _stm_i2cnl_open(stmi2c);
    if(stat) return stat;
    printf("CPURST: Notify about POR event ...\n");
    stmi2c->txdata.txheader.h_event = EVENT_CYC_POWER_CPU;
    stmi2c->txdata.txheader.h_rxlen = 0xFF;
    stat = _stm_i2cnl_write(stmi2c, (uint8_t*)&stmi2c->txdata, I2C_PACKET_LEN);
    if(stat) return stat;  
    return STMI2C_OK;
}
/* ----------------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */
int _stm_i2cnl_mcu_reset(STMI2C_Tool_TD *stmi2c)
{
    int stat = STMI2C_ERROR;
    int txrxlen = 0x01;
    stat = _stm_i2cnl_open(stmi2c);
    if(stat) return stat;
    printf("MCURST: Try to reset ...\n");
    stmi2c->txdata.txheader.h_event = EVENT_UPGRADE;
    stat = _stm_i2cnl_write(stmi2c, (uint8_t*)&stmi2c->txdata, txrxlen);
    if(stat) return stat;
    //stat = _stm_i2cnl_read(stmi2c, (uint8_t*)&stmi2c->rxdata.payload[0], txrxlen);
    //if(stat) return stat;
    printf("MCURST: Completed.\n");
    usleep(4000000);
    return STMI2C_OK;
}
/* ----------------------------------------------------------------------------------- */