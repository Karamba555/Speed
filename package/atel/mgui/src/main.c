/******************************************************************************
*(C) Copyright 2014 Marvell International Ltd.
* All Rights Reserved
******************************************************************************/
/* -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: main.c
 *
 *  Authors: Tomer Eliyahu
 *
 *  Description: mgui (marvell gui) main
 *
 *  HISTORY:
 *
 *   May 26, 2015 - Initial Version
 *
 *  Notes:
 *
 ******************************************************************************/

/******************************************************************************
 *   Include files
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mgui.h"
#include "mgui_onkey.h"
#include "mgui_utils.h"
#include "mgui_config.h"
#include <signal.h>

/******************************************************************************
 *   Code
 ******************************************************************************/
static void recvSignal(int sig)
{
	printf("received signal %d !!!\n",sig);
	exit(0);
}
static void recvWpsSignal(int sig)
{
	struct mgui_context *mgui;
	printf("received signal wps %d pressed\n",sig);
	system("echo \"wps wlan1 pressed\" > /tmp/wpskey_status");
}
static void recvChargerSignal(int sig)
{
	printf("received signal charger %d connect\n",sig);
	system("echo \"Charger connect\" > /tmp/charger_status");

}
int main(int argc, char *argv[])
{
	struct mgui_context *mgui;
	signal(SIGSEGV, recvSignal);
	//signal(SIGUSR1, recvWpsSignal);
	signal(SIGUSR2, recvChargerSignal);
	MGUI_LOG_INIT();
	MGUI_IMSG("mgui version 0.1\n");

	mgui = mgui_init_main_speedway(argc, argv);
	if (!mgui) {
		MGUI_EMSG("mgui init failed\n");
		exit(1);
	}

	mgui_run_speedway(mgui);
	mgui_exit_speedway(mgui);
	return 0;
}
