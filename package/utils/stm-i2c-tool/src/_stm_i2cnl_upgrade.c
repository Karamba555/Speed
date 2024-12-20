/*
 * _stm_i2c_firmware.c
 *
 *  Created on: 24 Oct 2024
 *      Author: Sergey Frolov
*/

#include "_stm_i2cnl_upgrade.h"


extern STMI2C_Tool_TD               stmi2c_o;
sem_t                               *semu;

int _stm_i2c_fw_open_binary(STMI2C_Tool_TD *stmi2c);
int _stm_i2c_fw_read_binary(STMI2C_Tool_TD *stmi2c);


/***************************************************************************************/
/* FILE OPEN/READ SECTION */
/* ----------------------------------------------------------------------------------- */
/**
  * @brief  Open FW binary file
  * @param  STMI2C_Tool_TD *stmi2c
  * @retval STMI2C_ERROR or STMI2C_OK
  */
int _stm_i2cnl_open_binary(STMI2C_Tool_TD *stmi2c)
{
    uint8_t stat = STMI2C_ERROR;
    uint32_t length = 0;
    int fwstrlen = 0;
    char fname[32] = {0};
    strcpy(fname, FW_FILENAME);
    fwstrlen = strlen(fname);
    fname[fwstrlen] = 0x00;
    stmi2c->fwfile.file = fopen(fname,"rb");
    if(stmi2c->fwfile.file == NULL)
    {
        printf("No file found %s\n", fname);
        printf("Try to check that file exists.\n");
        return stat;
    }
    // get length of file:
    fseek(stmi2c->fwfile.file, 0, SEEK_END);
    length = ftell(stmi2c->fwfile.file);
    printf("FILE NAME: %s\n", fname);
    printf("FILE SIZE: %d\n", length);
    stmi2c->fwfile.fsize = length;
    stmi2c->fwfile.cursor_pos = 0;
    stmi2c->fwfile.progress = 0;
    stmi2c->fwfile.state = UPGRADE_TRIGGER;
    stmi2c->fwfile.bytes_left = stmi2c->fwfile.fsize;
    stmi2c->fwfile.packet_num = 0;
    fseek(stmi2c->fwfile.file, 0, SEEK_SET);
    stat = STMI2C_OK;
    return stat;
}
/* ----------------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */
int _stm_i2cnl_read_binary(STMI2C_Tool_TD *stmi2c)
{
    uint8_t stat = STMI2C_ERROR;
    int p = 0;
    int m = 0;
    uint8_t b_byte = 0xFF;
    uint8_t b_buffer[I2C_PAYLOAD_LEN];
    uint32_t bleft = I2C_PAYLOAD_LEN;
    memset((uint8_t*)&stmi2c->upddata, 0xFF, sizeof(stmi2c->upddata));
    if(stmi2c->fwfile.state == UPGRADE_TRIGGER)
    {
        stmi2c->upddata.updheader.h_event = EVENT_UPGRADE;
        stmi2c->upddata.updheader.h_state = UPGRADE_TRIGGER;
        stmi2c->upddata.updheader.h_txlen = I2C_PACKET_LEN;
        stmi2c->fwfile.state = UPGRADE_ING;
        stat = STMI2C_ERASE;
        return stat;
    }

    if(stmi2c->fwfile.bytes_left == 0)
    {
        stat = STMI2C_CMPT;
        stmi2c->upddata.updheader.h_event = EVENT_UPGRADE;
        stmi2c->upddata.updheader.h_state = UPGRADE_FINISH;
        stmi2c->upddata.updheader.h_txlen = I2C_PACKET_LEN;
        stmi2c->fwfile.state = UPGRADE_FINISH;
        stat = STMI2C_CMPT;
        return stat;
    }

    fseek(stmi2c->fwfile.file, stmi2c->fwfile.cursor_pos, SEEK_SET);
    bleft = I2C_PAYLOAD_LEN;
    if(stmi2c->fwfile.bytes_left < I2C_PAYLOAD_LEN)
    {
        bleft = stmi2c->fwfile.bytes_left;
    }
    for(p=0; p<bleft; p++)
    {
        fread(&b_byte, sizeof(uint8_t), 1, stmi2c->fwfile.file);
        b_buffer[p] = b_byte;
        m+=1;
    }
    stmi2c->fwfile.bytes_left -= bleft;
    stmi2c->fwfile.cursor_pos += bleft;
    stmi2c->fwfile.progress = (stmi2c->fwfile.cursor_pos * 100)/stmi2c->fwfile.fsize;
    // printf(" %d \n", fwuo->fwfile.bytes_left);
    printf("\r-- %d", stmi2c->fwfile.progress);
    fflush(stdout);

    stmi2c->fwfile.packet_num += 1;

    stmi2c->upddata.updheader.h_event = EVENT_UPGRADE;
    stmi2c->upddata.updheader.h_state = UPGRADE_ING;
    stmi2c->upddata.updheader.h_txlen = I2C_PACKET_LEN;
    stmi2c->upddata.updheader.h_nohigh = (stmi2c->fwfile.packet_num & 0xFF00)>>8;
    stmi2c->upddata.updheader.h_nolow = (stmi2c->fwfile.packet_num & 0x00FF);
    memcpy(&stmi2c->upddata.payload[0], &b_buffer[0], bleft);
    stat = STMI2C_WRITE;
    return stat;
}
/* ----------------------------------------------------------------------------------- */
/***************************************************************************************/



/***************************************************************************************/
/* FUPGRADE PROCESS SECTION */
/* ----------------------------------------------------------------------------------- */
int _stm_i2cnl_upgrade_process(STMI2C_Tool_TD *stmi2c)
{
    int stat = STMI2C_ERROR;
    //Update process in thread
    stat = _stm_i2cnl_read_binary(stmi2c);
    if(stat == STMI2C_ERASE)
    {
        printf("UPGRADE: Erasing ...\n");
        //usleep(5000);
        stmi2c->upddata.updheader.h_ckm = 0;
        stmi2c->status = STMI2C_ERASE;
        stat = _stm_i2cnl_write(stmi2c, (uint8_t*)&stmi2c->upddata, I2C_PACKET_LEN);
        if(stat) {stmi2c->status = STMI2C_ERROR; return stat;}
        stat = _stm_i2cnl_read(stmi2c, (uint8_t*)&stmi2c->upddata.payload[0], 16);
        if(stat) {stmi2c->status = STMI2C_ERROR; return stat;}
        usleep(4000000);
        printf("UPGRADE: Erasing done.\n");
    }
    else if(stat == STMI2C_WRITE)
    {
        //printf("Writing: %d \n", stmi2c->fwfile.packet_num);
        //usleep(5000);
        stmi2c->upddata.updheader.h_ckm = stmi2c->fwfile.packet_num;
        stmi2c->status = STMI2C_WRITE;
        stat = _stm_i2cnl_write(stmi2c, (uint8_t*)&stmi2c->upddata, I2C_PACKET_LEN);
        if(stat) {stmi2c->status = STMI2C_ERROR; return stat;}
        stat = _stm_i2cnl_read(stmi2c, (uint8_t*)&stmi2c->upddata.payload[0], 16);
        if(stat) {stmi2c->status = STMI2C_ERROR; return stat;}
        //usleep(40000);
    }
    else if(stat == STMI2C_CMPT)
    {
        //printf("Writing: END: %d \n", stmi2c->fwfile.packet_num);
        //usleep(5000);
        stat = _stm_i2cnl_write(stmi2c, (uint8_t*)&stmi2c->upddata, I2C_PACKET_LEN);
        if(stat) {stmi2c->status = STMI2C_ERROR; return stat;}
        stat = _stm_i2cnl_read(stmi2c, (uint8_t*)&stmi2c->upddata.payload[0], 16);
        if(stat) {stmi2c->status = STMI2C_ERROR; return stat;}
        stat = STMI2C_CMPT;
        stmi2c->status = STMI2C_CMPT;
        usleep(1000000);
        printf("\nUPGRADE: Completed.\n");
    }
    return stat;
}
/* ----------------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------- */
void* _stm_i2cnl_threadFunc(void* thread_data)
{
    int stat = STMI2C_ERROR;
    while(1)
    {
        stat = _stm_i2cnl_upgrade_process(&stmi2c_o);
        if(stat == STMI2C_CMPT || stat == STMI2C_ERROR)
        {
            sem_post(semu);
            break;
        }
        usleep(10000);
    }
    pthread_exit(0);
}
/* ----------------------------------------------------------------------------------- */
/***************************************************************************************/


/***************************************************************************************/
/* CALL UPGRADE START */
/* ----------------------------------------------------------------------------------- */
int _stm_i2cnl_upgrade_start(STMI2C_Tool_TD *stmi2c)
{
    int stat = STMI2C_ERROR;
    semu = sem_open("/semu", O_CREAT, 0644, 0);
    if(semu == SEM_FAILED) return stat;
    printf("UPGRADE: Reset MCU ...\n");
    //stat = _stm_i2cnl_mcu_reset(stmi2c);
    //if(stat) return stat;
    printf("UPGRADE: Preparing file ...\n");
    stat = _stm_i2cnl_open_binary(stmi2c);
    if(stat) return stat;
    pthread_create(&stmi2c->wpthread, NULL, _stm_i2cnl_threadFunc, NULL);
	return stat;
}
/* ----------------------------------------------------------------------------------- */
/***************************************************************************************/
















/* ----------------------------------------------------------------------------------- */
int _stm_i2cnl_upgrade(STMI2C_Tool_TD *stmi2c)
{
    int stat = STMI2C_ERROR;
    stat = _stm_i2cnl_open(stmi2c);
    if(stat) return stat;
    stat = _stm_i2cnl_upgrade_start(stmi2c);
    if(stat)
    {
        printf("UPGRADE: Failed %d\n", stat);
        return stat;
    }
    /* Get semaphore */
    sem_wait(semu);
    return 0;
}
/* ----------------------------------------------------------------------------------- */