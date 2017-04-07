/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2012-? PAX Technology, Inc. All rights reserved.
 * Description: Bluetooth.c - D200/D210 Demo bluetooth driver Code.
 * Revision History:
 * Date                  Author                 Action
 *
 *  Author:zhaorh Email:zhaorh@paxsz.com
 * ============================================================================
 */

#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H
#include <posapi.h>

#define TIME_BT_0 3
#define TIME_BT_1 4
#define STRING_LEN 64
#define BT_MAC_LEN 18
#define BT_NAME_MAXLEN 64
#define BT_PIN_MAXLEN 16
#define BT_ROLE_MASTER	1
#define BT_ROLE_SLAVE	0
#define COM_BT	7

#define BT_RET_OK					0
#define BT_RET_ERROR_DEFAULT		-1
#define BT_RET_ERROR_NOTOPEN		-2
#define BT_RET_ERROR_NOTCLOSE		-3
#define BT_RET_ERROR_BAUDCHECKERROR -4
#define BT_RET_ERROR_PORTERROR		-5
#define BT_RET_ERROR_DATAMODE		-6
#define BT_RET_ERROR_NULL			-7
#define BT_RET_ERROR_PARAMERROR		-8
#define BT_RET_ERROR_ROLEERROR		-9
#define BT_RET_ERROR_EXPLAINERROR	-10

typedef struct
{
	int role;
	long baud;
	uchar name[64];
	uchar pin[16];
	uchar mac[6];
}ST_BT_CONFIG;
typedef struct  
{
	uchar name[STRING_LEN];
	uchar addr[STRING_LEN];
}ST_BT_SLAVE;

int BtOpen(unsigned char COM, const unsigned char *pParam);
int BtClose(void);
int BtSetName(const uchar *Name);
int BtSetRole(int role);
int BtSetPin(const uchar *Pin);
int BtGetConfig(ST_BT_CONFIG *ConfigInfo);
int BtSendCmd(const uchar * ATstrIn,
               uchar *RspOut,
               ushort Rsplen,
               ushort TimeOut, 
               ushort Mode,
			   ushort ExpectCount);

int BtScan(ST_BT_SLAVE *outSlave, int *SlaveCount, int TimeOutMs);  //Master
int BtConnect(ST_BT_SLAVE Slave); //Master
int BtDiscon(void); //Master
int BtPause(void); //Master
int BtReConnect(int slave); //Master
int BtCheckList(int slave); //Master
#endif
