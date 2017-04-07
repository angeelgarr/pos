/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2012-? PAX Technology, Inc. All rights reserved.
 * Description:
 * Revision History:
 * Date                  Author                 Action
 * 2012-12-4          wenjf                    created
 * ============================================================================
 */

#ifndef MODEM_H_
#define MODEM_H_

#include "structDefine.h"
//#include "osal.h"

//Modem拨号参数结构
typedef struct{
	unsigned char DP;	// 设置音频/脉冲拨号
	unsigned char CHDT;	// 是否检测拨号音
	unsigned char DT1;	// 摘机到拨号的等待时间(单位：100ms)
	unsigned char DT2;	// 拨外线时“,”等待时间(单位：100ms) 
	unsigned char HT;	// 双音拨号单一号码保持时间单位：1ms 
	unsigned char WT;	// 双音拨号两个号码之间的间隔时间10ms 
	unsigned char SSETUP;	// 通讯字节的设置[包括设置同步异步、波特率、线路、应答音超时等] 
	unsigned char DTIMES;	// 重拨次数,必须大于等于1 
	unsigned char TimeOut;	// 通讯超时(若在此时间内没有数据交换,MODEM则挂断,为0时无超时,单位：10秒) 
	unsigned char AsMode;	// 异步通讯方式(仅异步通信时有效) 
}COMM_PARA;

typedef struct{
	char rate[20];
}BAUD_RATE;

typedef struct{
	char format[20];
}MODEM_CONNECT_FORMAT;

typedef struct{
	int prolin_ret;
	uchar monitor_ret;
}MODEM_RESULT;

/***************Modem*********************/
uchar ModemReset();
uchar ModemDial(COMM_PARA *MPara, uchar *TelNo, uchar mode);
uchar ModemCheck(void);
uchar ModemTxd(uchar *SendData, ushort len);
uchar ModemRxd(uchar *RecvData, ushort *len);
uchar ModemAsyncGet(uchar *ch);
uchar OnHook(void);
uchar HangOff(void);
ushort ModemExCommand(uchar *CmdStr, uchar *RespData, ushort *Dlen, ushort Timeout10ms);
int PPPLogin(char *name, char *passwd, long auth , int timeout);
void PPPLogout(void);
int PPPCheck(void);

#endif /* MODEM_H_ */
