/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2009-? PAX Technology, Inc. All rights reserved.
 * Description:    定义模拟、debug或培训模式所使用的功能
 * Revision History:
 * Date                  Author                 Action
 * 2009.05.07         Gary Ho                      created
 * ============================================================================
 */

#include "global.h"

/* ********************* Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// 检查是否处于练习/演示模式
// check if is in training mode
uchar ChkIfTrainMode(void)
{
	return (glSysParam.stTxnCommCfg.ucCommType==CT_DEMO);
}

// 练习/演示模式下用于模拟后台数据
// generate Dummy data for receiving in training mode
int CreatDummyRecvData(void *pstProcInfo, void *pstRecvPacket)
{
	uchar	szBuff[32];
	STISO8583		*pstRecv;
	SYS_PROC_INFO	*pstProc;

	pstRecv = (STISO8583 *)pstRecvPacket;
	pstProc = (SYS_PROC_INFO *)pstProcInfo;

	GetDateTime(szBuff);
	if ( (strlen(pstProc->stTranLog.szDateTime)==0) || (pstProc->stTranLog.ucTranType==VOID) )
	{
		sprintf((char *)(pstProc->stTranLog.szDateTime), "%.14s", (char *)szBuff);
	}

	// Authorization code
	sprintf(pstProc->stTranLog.szAuthCode, "      ");
	if ((pstProc->stTranLog.ucTranType!=VOID) && (strlen(pstProc->stTranLog.szAuthCode)==0))
	{
		sprintf(pstProc->stTranLog.szAuthCode, "*DEMO*");
	}

	// RRN
	sprintf((char *)(pstProc->stTranLog.szRRN), "*DEMO*%6.6s", (char *)szBuff+8);

	// Response code
	sprintf((char *)(pstRecv->szRspCode), "00");
	if ((pstProc->stTranLog.ucTranType==SETTLEMENT) && (glSendPack.szProcCode[1]=='2'))
	{
		sprintf((char *)(pstRecv->szRspCode), "95");
	}

	sprintf((char *)(pstProc->stTranLog.szRspCode), (char *)(pstRecv->szRspCode));
	return 0;
}

// end of file

