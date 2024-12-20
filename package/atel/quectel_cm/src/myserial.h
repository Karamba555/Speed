;/*
 * Copyright (C) 2005 Asiatelco. All rights reserved.
 * 
 * filename: myserial.h
 *
 * Version: 1.0
 * Author: chenglei
 */


#ifndef	__MYSERIAL__H__
#define	__MYSERIAL__H__

#include	<stdio.h>      /*标准输入输出定义*/
#include	<stdlib.h>     /*标准函数库定义*/
#include	<unistd.h>     /*Unix 标准函数定义*/
#include	<sys/types.h>  
#include	<sys/stat.h>   
#include	<fcntl.h>      /*文件控制定义*/
#include	<termios.h>    /*PPSIX 终端控制定义*/
#include	<errno.h>      /*错误号定义*/

typedef int STATUS;
#define STATUS_ERR				-1
#define STATUS_OK				0
#define STATUS_BAD_ARGUMENTS	1
#define STATUS_TIME_OUT			2
#define STATUS_BAD_AT_CMD		3
#define MAX_BUF_SIZE	4096

int OpenDev(int* devfd, char *devport);
int SetParity(int fd,int databits,int stopbits,int parity);
int SetSpeed(int fd, int speed);

STATUS at_handle( int fd, const char* at_commands, char* at_returns_ret, unsigned int wait_time );

#endif
