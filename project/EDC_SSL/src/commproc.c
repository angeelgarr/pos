﻿/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2006-? PAX Technology, Inc. All rights reserved.
 * Description:     实现通讯控制
 * Revision History:
 * Date                  Author                 Action
 * 2006.09.14         shengjx               created
 * ============================================================================
 */

#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static int  AskModemReDial(int iRet);
static int  GenSendPacket(void);
static int  ProcRecvPacket(void);
static void SaveRecvPackData(void);
static int  AdjustCommParam(void);
static int DispCommErrMsg(int iErrCode);

/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// 交易处理
// process transaction
int TranProcess(void)
{
	int		iRet, iRetryPIN;

#ifdef ENABLE_EMV
	// transaction processing for EMV card
	if( (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) &&
		(glProcInfo.stTranLog.ucTranType==SALE || glProcInfo.stTranLog.ucTranType==CASH ||
		 glProcInfo.stTranLog.ucTranType==AUTH || glProcInfo.stTranLog.ucTranType==PREAUTH ||
		 glProcInfo.stTranLog.ucTranType==INSTALMENT) )
	{
		return FinishEmvTran();
	}
#endif

	// check if need to reverse the last transaction 
	iRet = TranReversal();
	if( iRet!=0 )
	{
		return iRet;
	}

	// Some banks may want TC to be sent prior to sale
	iRetryPIN = 0;
	while( 1 )
	{
		if (ChkIfAmex() || ChkCurAcqName("AMEX", FALSE))
		{
			GetNewInvoiceNo();
		}

		iRet = SendRecvPacket();
		if( iRet!=0 )
		{
			return iRet;
		}

		if( memcmp(glRecvPack.szRspCode, "55", 2)!=0 || ++iRetryPIN>3 || !ChkIfNeedPIN() )
		{
			break;
		}

		// Re-enter PIN
		iRet = GetPIN(GETPIN_RETRY);
		if( iRet!=0 )
		{
			return iRet;
		}
		sprintf((char *)glSendPack.szSTAN, "%06lu", glSysCtrl.ulSTAN);
		memcpy(&glSendPack.sPINData[0], "\x00\x08", 2);
		memcpy(&glSendPack.sPINData[2], glProcInfo.sPinBlock, 8);
	}

	return AfterTranProc();
}

// 交换通讯报文
// exchange package(send request & wait response)
int SendRecvPacket(void)
{
	int		iRet;
	
    iRet = SendPacket();
    if( iRet!=0 )
    {
        return iRet;
    }

	// 保存结算状态/冲正数据
	if( glProcInfo.stTranLog.ucTranType==SETTLEMENT )
	{
		glSysCtrl.sAcqStatus[glCurAcq.ucIndex] = S_PENDING;
		SaveSysCtrlBase();
	}

	SaveRevInfo(TRUE);

	iRet = RecvPacket();
	if( iRet!=0 )
	{
		return iRet;
	}

	return 0;
}

// 生成发送报文
// generate request package
int GenSendPacket(void)
{
	int		iRet;
	uint	uiLength;
	uchar	szBuff[20], sMacOut[LEN_MAC];

	// prepare TPDU header
	memset(&glSendData, 0, sizeof(COMM_DATA));
	sprintf((char *)szBuff, "600%3.3s0000", glCurAcq.szNii);
	PubAsc2Bcd(szBuff, LEN_MSG_HEADER * 2, glSendData.sContent);

	// generate iso8583 data
	if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
	{
		iRet = PubPack8583(glTMSDataDef, &glTMSSend, &glSendData.sContent[LEN_MSG_HEADER], &uiLength);
	}
	else
	{
		iRet = PubPack8583(glEdcDataDef, &glSendPack, &glSendData.sContent[LEN_MSG_HEADER], &uiLength);
	}
	if( iRet<0 )
	{
		// Modified by Kim_LinHB 2014-5-31
		Get8583ErrMsg(TRUE, iRet, szBuff);

		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_NO_DISP;
	}
	glSendData.uiLength = (ushort)(uiLength + LEN_MSG_HEADER);

	// 如果DEBIT不发MAC
	// If don't need MAC
	if( !ChkIfNeedMac() )
	{
		return 0;
	}

	// fill mac data
	memset(sMacOut, 0, sizeof(sMacOut));
	// !!!! the MAC key ID (1) should be per acquirer specified.
	// !!!! the algorithm should be per acquirer specified.
	iRet = GetMAC(MAC_ANSIX919, &glSendData.sContent[LEN_MSG_HEADER], (ushort)(uiLength-LEN_MAC), 1, sMacOut);
	if( iRet!=0 )
	{
		return iRet;
	}
	memcpy(&glSendData.sContent[glSendData.uiLength-LEN_MAC], sMacOut, LEN_MAC);

	return 0;
}

// 预拨号
// pre-connect to host
int PreDial(void)
{
	int		iRet;

	if( !glSysParam.stEdcInfo.bPreDial )
	{
		return 0;
	}

	iRet = AdjustCommParam();
	if( iRet!=0 )
	{
		return iRet;
	}

	return CommDial(DM_PREDIAL);
}

// 连接主机
// connect to host
int ConnectHost(void)
{
	int		iRet;

	while( 1 )
	{
		// 设置通信参数（从ACQ取得IP，电话号码）
		// set communication parameters(get IP address, tel NO. of acquirer, etc.)
		iRet = AdjustCommParam();
		if (iRet!=0)
		{
			if ((glCommCfg.ucCommTypeBak!=CT_NONE) && 
				(glCommCfg.ucCommType!=glCommCfg.ucCommTypeBak))		// switch to next connection
			{
				// 如果第一套通信的参数不存在，则切换到备用通信方式
				// switch to the backup communication type if the premier comm type is not existed
				glCommCfg.ucCommType = glSysParam.stTxnCommCfg.ucCommTypeBak;
				CommSwitchType(glSysParam.stTxnCommCfg.ucCommTypeBak);
				continue;
			}

			if( iRet!=ERR_NO_TELNO )
			{
				DispCommErrMsg(iRet);
				return ERR_NO_DISP;
			}
			return iRet;
		}

        SetOffBase(OffBaseDisplay);

		kbflush();
		DispDial();
		iRet = CommDial(DM_DIAL);
		if (iRet==0)
		{
			return 0;
		}

		// 是否按过取消
		// if pressed cancel key
		if ((kbhit()==0) && (getkey()==KEYCANCEL))
		{
			return ERR_USERCANCEL;
		}

		// 如果第一套通信方式连接失败，则切换到备用通信方式
		// switch to the backup communication type if the premier comm type was failed
		if ((glCommCfg.ucCommTypeBak!=CT_NONE) && (glCommCfg.ucCommType!=glCommCfg.ucCommTypeBak))
		{
			glCommCfg.ucCommType = glCommCfg.ucCommTypeBak;
			CommSwitchType(glCommCfg.ucCommTypeBak);
			continue;
		}

		// 非Modem 错误，直接返回
		// return directly if it's not a Modem type error
		if( (iRet & MASK_COMM_TYPE)!=ERR_COMM_MODEM_BASE )
		{
			DispCommErrMsg(iRet);
			return ERR_NO_DISP;
		}

		// Modem 错误，询问是否重拨
		// it's a Modem type error, and ask to re-dial
		if (AskModemReDial(iRet))
		{
			return ERR_USERCANCEL;
		}
	}

	return 0;
}

// Modified by Kim_LinHB 2014-5-31
int AskModemReDial(int iRet)
{
	int	iResult;

	if( iRet==ERR_COMM_MODEM_OCCUPIED || 
		iRet==ERR_COMM_MODEM_NO_LINE ||
		iRet==ERR_COMM_MODEM_NO_LINE_2 )
	{
		iResult = DispCommErrMsg(iRet);
	}
	else{
		if( iRet==ERR_COMM_MODEM_LINE || 
			iRet==ERR_COMM_MODEM_NO_ACK ||
			iRet==ERR_COMM_MODEM_LINE_BUSY )
		{
			DispResult(iRet);

			Gui_ClearScr();
			iResult = (unsigned char)Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, _T("RE DIAL ?"), gl_stCenterAttr, GUI_BUTTON_YandN, 5, NULL);
		}
		else
		{
			unsigned char szBuff[255];
			PubBeepErr();

			sprintf(szBuff, "%s\n   %02X", _T("DIAL FAIL,RETRY?"), (uchar)(iRet & MASK_ERR_CODE));
			
			Gui_ClearScr();
			iResult = (unsigned char)Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_WARNING, szBuff, gl_stCenterAttr, GUI_BUTTON_YandN, 5, NULL);
		}
	}

	//if( iResult == GUI_ERR_TIMEOUT || iResult == GUI_ERR_USERCANCELLED )
	if(iResult != GUI_OK)
	{
	    return ConvertGuiErr(iResult); //EPT-15
	}

	return 0;
}

// 发送报文
// send request package
int SendPacket(void)
{
	int		iRet;

	iRet = GenSendPacket();
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = ConnectHost();
	if( iRet!=0 )
	{
		return iRet;
	}

	DispSend();
	if( glCommCfg.ucCommType==CT_TCPIP ||
		glCommCfg.ucCommType==CT_WIFI  ||
		glCommCfg.ucCommType==CT_BLTH  || // Added by Kim_LinHB 2014-08-18 v1.01.0004
		glCommCfg.ucCommType==CT_GPRS  ||
		glCommCfg.ucCommType==CT_CDMA  ||
		glCommCfg.ucCommType==CT_WCDMA)   // modify Gillian 2015/11/23
	{
		memmove(&glSendData.sContent[LEN_TCP_PACKAGE], glSendData.sContent, glSendData.uiLength);
		if (glCommCfg.ucTCPClass_BCDHeader)
		{
			// BCD 格式的长度字节
			// a BDC TCP header
			PubLong2Bcd((ulong)glSendData.uiLength,  LEN_TCP_PACKAGE, glSendData.sContent);
		} 
		else
		{
			PubLong2Char((ulong)glSendData.uiLength, LEN_TCP_PACKAGE, glSendData.sContent);
		}
		glSendData.uiLength += LEN_TCP_PACKAGE;
	}

#ifdef APP_DEBUG
	//PubISODefine(glEdcDataDef);
	PubDebugOutput("ISO REQ:", glSendData.sContent, glSendData.uiLength, DEVICE_COM1, HEX_MODE);
#endif

	iRet = CommTxd(glSendData.sContent, glSendData.uiLength, USER_OPER_TIMEOUT);	// "no timeout" is forbidden
	if( iRet!=0 )
	{
		DispCommErrMsg(iRet);
		return ERR_NO_DISP;
	}
	GetNewTraceNo();

	return 0;
}

// Modified by Kim_LinHB 2014-5-31
// Added by Kim_LinHB 2014-5-31
// 接收报文
// receive response packet
int RecvPacket(void)
{
	int		iRet;
	ushort	uiTimeOut;
	ulong	ulTemp;

	DispReceive();
	//Added by Kim_LinHB 2014-6-6 v1.01.0000
	switch(glCommCfg.ucCommType)
	{
	case CT_WIFI:
	case CT_TCPIP:
		uiTimeOut = glCurAcq.ucTcpTimeOut;
		break;
	case CT_GPRS:
	case CT_CDMA:
	case CT_WCDMA:
		uiTimeOut = glCurAcq.ucGprsTimeOut;
		break;
	case CT_RS232:
	case CT_MODEM:
	case CT_BLTH:
	default:
		uiTimeOut = glCurAcq.ucPhoneTimeOut;
		break;
	}
	
	memset(&glRecvData, 0, sizeof(COMM_DATA));
	iRet = CommRxd(glRecvData.sContent, LEN_MAX_COMM_DATA, uiTimeOut, &glRecvData.uiLength);
	if( iRet!=0 )
	{
		DispCommErrMsg(iRet);
		return ERR_NO_DISP;
	}

#ifdef APP_DEBUG
    //PubISODefine(glEdcDataDef);
    PubDebugOutput("ISO RESP:", glRecvData.sContent, glRecvData.uiLength, DEVICE_COM1, HEX_MODE);
#endif

	if( glCommCfg.ucCommType==CT_TCPIP ||
		glCommCfg.ucCommType==CT_WIFI  ||
		glCommCfg.ucCommType==CT_BLTH  || // Added by Kim_LinHB 2014-08-18 v1.01.0004
		glCommCfg.ucCommType==CT_GPRS  ||
		glCommCfg.ucCommType==CT_CDMA  ||
		glCommCfg.ucCommType==CT_WCDMA)
	{
		if (glCommCfg.ucTCPClass_BCDHeader)
		{
			// BCD 格式的长度字节
			// a BCD TCP header
			ulTemp = PubBcd2Long(glRecvData.sContent,  LEN_TCP_PACKAGE);
		}
		else
		{
			ulTemp = PubChar2Long(glRecvData.sContent, LEN_TCP_PACKAGE);
		}

		if( ulTemp+LEN_TCP_PACKAGE != (ulong)glRecvData.uiLength )
		{
			SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
			// Modified by Kim_LinHB 2014-5-31
			Gui_ClearScr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("RECV DATA ERR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 2, NULL);
			return ERR_NO_DISP;
		}
		memmove(glRecvData.sContent, &glRecvData.sContent[LEN_TCP_PACKAGE], (uint)ulTemp);
		glRecvData.uiLength = (ushort)ulTemp;
	}

	if (glCommCfg.ucCommType==CT_DEMO)
	{
		iRet = CreatDummyRecvData(&glProcInfo, &glRecvPack);
		return iRet;
	}

	iRet = ProcRecvPacket();
	if( iRet!=0 )
	{
		return iRet;
	}

	return 0;
}

// Modified by Kim_LinHB 2014-5-31
// 处理接收包，并检查公共数据要素
// Process receive packet content and check public data element
int ProcRecvPacket(void)
{
	int		iRet;
	long	lTemp1, lTemp2;
	uchar	bFailFlag, szBuff[20];

	DispProcess();
	// 检查接收到的数据长度(TPDU+MTI+Bitmap+stan+tid)
	// Check received length (TPDU+MTI+Bitmap+stan+tid)
	if( glRecvData.uiLength<LEN_MSG_HEADER+2+8+8+3 )
	{
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("RECV DATA ERR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_NO_DISP;
	}
	bFailFlag = FALSE;

	// 处理TPDU(5)
	// Processing TPDU
	if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
	{
		memset(&glTMSRecv, 0, sizeof(STTMS8583));
		iRet = PubUnPack8583(glTMSDataDef, &glRecvData.sContent[LEN_MSG_HEADER], glRecvData.uiLength-LEN_MSG_HEADER, &glTMSRecv);
	}
	else
	{
		memset(&glRecvPack, 0, sizeof(STISO8583));
		iRet = PubUnPack8583(glEdcDataDef, &glRecvData.sContent[LEN_MSG_HEADER], glRecvData.uiLength-LEN_MSG_HEADER, &glRecvPack);
	}
	if( iRet<0 )
	{
		Get8583ErrMsg(FALSE, iRet, szBuff);
		PubBeepErr();
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, szBuff, gl_stLeftAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_NO_DISP;
	}

	// 检查响应数据包的要素并与请求包匹配
	// Check message type
	bFailFlag = FALSE;
	if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
	{
		lTemp1 = atol((char *)glTMSSend.szMsgCode);
		lTemp2 = atol((char *)glTMSRecv.szMsgCode);
	}
	else
	{
		lTemp1 = atol((char *)glSendPack.szMsgCode);
		lTemp2 = atol((char *)glRecvPack.szMsgCode);
	}
	// 检查消息码
	// check the message id
	if( lTemp2!=(lTemp1+10) )
	{
		PubBeepErr();
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("MSG ID MISMATCH"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_NO_DISP;
	}

    // 保存请求/响应数据
    // Save received response packet.
    SaveRecvPackData();
    
	// 检查处理码
	// Check processing code
    bFailFlag = FALSE;
	if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
	{
        if( (glTMSSend.szProcCode[0]!=0) &&
            (memcmp(glTMSSend.szProcCode, glTMSRecv.szProcCode, LEN_PROC_CODE-1)!=0) )
		{
			bFailFlag = TRUE;
		}
	}
	else
	{
        if( (glSendPack.szProcCode[0]!=0) &&
            (memcmp(glSendPack.szProcCode, glRecvPack.szProcCode, LEN_PROC_CODE-1)!=0) )
		{
			bFailFlag = TRUE;
		}
	}
	if( bFailFlag )
	{
		PubBeepErr();
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("BIT 3 MISMATCH"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_NO_DISP;
	}

	// 检查S.T.A.N.
	// Check S.T.A.N. (Sys Trace Audit Number, field 11)
	bFailFlag = FALSE;
	if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
	{
		if( glTMSSend.szSTAN[0]!=0 &&
			memcmp(glTMSSend.szSTAN, glTMSRecv.szSTAN, LEN_STAN)!=0 )
		{
			bFailFlag = TRUE;
		}
	}
	else
	{
		if( glSendPack.szSTAN[0]!=0 &&
			memcmp(glSendPack.szSTAN, glRecvPack.szSTAN, LEN_STAN)!=0 )
		{
			bFailFlag = TRUE;
		}
	}
	if( bFailFlag )
	{
		PubBeepErr();
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("STAN MISMATCH"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_NO_DISP;
	}

	// 检查终端号/商户号
	// Check TID, MID
	bFailFlag = FALSE;
	if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
	{
		if( memcmp(glTMSSend.szTermID, glTMSRecv.szTermID, LEN_TERM_ID)!=0 )
		{
			bFailFlag = TRUE;
		}
	}
	else
	{
		if( memcmp(glSendPack.szTermID, glRecvPack.szTermID, LEN_TERM_ID)!=0 )
		{
			bFailFlag = TRUE;
		}
	}
	if( bFailFlag )
	{
		PubBeepErr();
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("TID MISMATCH"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_NO_DISP;
	}

	// More basic checks can be placed here.

	return 0;
}

// 保存接收报文要素到交易信息结构
// save some data element from receive packet to transaction data.
void SaveRecvPackData(void)
{
	uchar	szLocalTime[14+1];

	if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
	{
		sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", glTMSRecv.szRspCode);
		return;
	}

	if( (glProcInfo.stTranLog.ucTranType!=OFFLINE_SEND) &&
		(glProcInfo.stTranLog.ucTranType!=TC_SEND) )
	{
		sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", glRecvPack.szRspCode);
	}
	if( glProcInfo.stTranLog.ucTranType==SETTLEMENT ||
		glProcInfo.stTranLog.ucTranType==UPLOAD     ||
		glProcInfo.stTranLog.ucTranType==REVERSAL )
	{
		return;
	}

	UpdateLocalTime(NULL, glRecvPack.szLocalDate, glRecvPack.szLocalTime);

	GetDateTime(szLocalTime);
	sprintf((char *)glProcInfo.stTranLog.szDateTime, "%.14s", szLocalTime);

	if( glRecvPack.szAuthCode[0]!=0 )
	{
		sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%.6s", glRecvPack.szAuthCode);
	}
	if( glProcInfo.stTranLog.szRRN[0]==0 )
	{
		sprintf((char *)glProcInfo.stTranLog.szRRN, "%.12s", glRecvPack.szRRN);
	}
	sprintf((char *)glProcInfo.stTranLog.szCondCode,  "%.2s",  glSendPack.szCondCode);
	sprintf((char *)glProcInfo.stTranLog.szFrnAmount, "%.12s", glRecvPack.szFrnAmt);

	FindCurrency(glRecvPack.szHolderCurcyCode, &glProcInfo.stTranLog.stHolderCurrency);
}

int AfterTranProc(void)
{
	int		iRet;
	int		iTransResult;

	if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
	{
		if( memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)!=0 )
		{
			DispResult(ERR_HOST_REJ);
			return ERR_NO_DISP;
		}

		DispResult(0);
		return 0;
	}

	iTransResult = ERR_HOST_REJ;
	if ( memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)==0 )
	{
		iTransResult = 0;
	}
	if ( (memcmp(glProcInfo.stTranLog.szRspCode, "08", 2)==0) ||
		 (memcmp(glProcInfo.stTranLog.szRspCode, "88", 2)==0) )
	{
		if (ChkIfAmex())
		{
			if ((glProcInfo.stTranLog.ucTranType==SALE) ||
				(glProcInfo.stTranLog.ucTranType==AUTH) ||
				(glProcInfo.stTranLog.ucTranType==PREAUTH) )
			{
				iTransResult = 0;
			}
		}
	}

	// 交易失败处理
	// when transaction is failed
	if (iTransResult)
	{
		CommOnHook(FALSE);
		SaveRevInfo(FALSE);

		DispResult(ERR_HOST_REJ);
		if( memcmp(glProcInfo.stTranLog.szRspCode, "01", 2)==0 ||
			memcmp(glProcInfo.stTranLog.szRspCode, "02", 2)==0 ||
			(glProcInfo.stTranLog.ucInstalment!=0 && ChkAcqOption(ACQ_BOC_INSTALMENT_FEATURE)) )
		{
			// BOC 银行instalment交易时假如交易结果不是"00",都进入referrel交易
			// BOC will start referrel transaction if installment transaction did not respond "00"
			iRet = TranSaleComplete();
			if( iRet==0 )
			{
				return 0;
			}
		}
		return ERR_NO_DISP;
	}

	DoE_Signature();

	if( glProcInfo.stTranLog.ucTranType==VOID )
	{
		glProcInfo.stTranLog.uiStatus |= TS_VOID;
		glProcInfo.stTranLog.uiStatus &= ~(TS_ADJ|TS_NOSEND|TS_OFFLINE_SEND|TS_NEED_TC);
		UpdateTranLog(&glProcInfo.stTranLog, glProcInfo.uiRecNo);
	}
	else if( ChkIfSaveLog() )
	{
		SaveTranLog(&glProcInfo.stTranLog);
	}
	SaveRevInfo(FALSE);

#ifdef ENABLE_EMV
	if( !ChkIfBoc() && ChkAcqOption(ACQ_EMV_FEATURE) )
	{
		if ((glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
			(glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
		{
			if( glProcInfo.stTranLog.ucTranType==SALE || glProcInfo.stTranLog.ucTranType==CASH ||
				glProcInfo.stTranLog.ucTranType==INSTALMENT )
			{
				glSysCtrl.stField56[glCurAcq.ucIndex].uiLength = 0;	// erase bit 56
				SaveField56();
			}
		}
	}
#endif

	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, "", gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
	if( glProcInfo.stTranLog.ucTranType!=RATE_BOC &&
		glProcInfo.stTranLog.ucTranType!=RATE_SCB )
	{
	    //EPT-11
	    uchar ucOrgTranType = glProcInfo.stTranLog.ucTranType;
		OfflineSend(OFFSEND_TC | OFFSEND_TRAN);
		TransInit(ucOrgTranType);
	}
    
	CommOnHook(FALSE);
    
	// Modified by Kim_LinHB 2014-08-14 v1.01.0003
	if(ChkIfPrnReceipt())
	{
		if( glProcInfo.stTranLog.ucTranType!=VOID )
		{
			GetNewInvoiceNo();
		}
		PrintReceipt(PRN_NORMAL);
	}

	DispResult(0);
	return 0;
}

// Modified by Kim_LinHB 2014-6-8 v1.01.0000
// voice referral dialing
int ReferralDial(const uchar *pszPhoneNo)
{
	uchar	ucRet, szTelNo[50];
	
	if( pszPhoneNo==NULL || *pszPhoneNo==0 )
	{
		PubBeepErr();
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("TEL# ERROR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_NO_DISP;
	}

	sprintf((char*)szTelNo, "%s%s.", glSysParam.stEdcInfo.szPabx, pszPhoneNo);
	
	Gui_ClearScr();
	while( 1 )
	{
		OnHook();
#if !defined(WIN32)
		ModemExCommand("AT-STE=0",NULL,NULL,0);
#endif
		ucRet = ModemDial(NULL, szTelNo, 1);
		if( ucRet==0x83 )
		{	// 旁置电话、并线电话均空闲(仅用于发号转人工接听方式)
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_WARNING, _T("PLS USE PHONE"), gl_stCenterAttr, GUI_BUTTON_NONE, 0,NULL);
		}
		DelayMs(1000);
		if( ucRet==0x06 || ucRet==0x00 || PubWaitKey(0)==KEYENTER )
		{
			return 0;
		}
		if( ucRet==0x0D )
		{
		
			OnHook();
			Gui_ClearScr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("LINE BUSY"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
			return ERR_NO_DISP;
		}
		PubWaitKey(1);
	}
	return 0;
}

// 调整通讯参数(只是设置Modem电话号码)
// get communication parameter from appropriate source, and set into glCommCfg
int AdjustCommParam(void)
{
	uchar	szNewTelNo[100+1];

	// 构造拨号电话号码
	// set format of tel NO. for dialing
	if (glCommCfg.ucCommType==CT_MODEM)
	{
		if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
		{
			sprintf((char *)szNewTelNo, "%s.", glSysParam.stEdcInfo.szDownTelNo);
		}
		else if( glProcInfo.stTranLog.ucTranType==SETTLEMENT )
		{
			if( (glCurAcq.stStlPhoneInfo[0].szTelNo[0]!=0) &&
				(glCurAcq.stStlPhoneInfo[1].szTelNo[0]!=0) )
			{
				sprintf((char *)szNewTelNo, "%s%s;%s%s.",
						glSysParam.stEdcInfo.szPabx, glCurAcq.stStlPhoneInfo[0].szTelNo,
						glSysParam.stEdcInfo.szPabx, glCurAcq.stStlPhoneInfo[1].szTelNo);
			}
			else if( glCurAcq.stStlPhoneInfo[0].szTelNo[0]!=0 )
			{
				sprintf((char *)szNewTelNo, "%s%s.",
						glSysParam.stEdcInfo.szPabx, glCurAcq.stStlPhoneInfo[0].szTelNo);
			}
			else if( glCurAcq.stStlPhoneInfo[1].szTelNo[0]!=0 )
			{
				sprintf((char *)szNewTelNo, "%s%s.",
						glSysParam.stEdcInfo.szPabx, glCurAcq.stStlPhoneInfo[1].szTelNo);
			}
			else
			{
				return ERR_NO_TELNO;
			}
		}
		else
		{
			if( (glCurAcq.stTxnPhoneInfo[0].szTelNo[0]!=0) &&
				(glCurAcq.stTxnPhoneInfo[1].szTelNo[0]!=0) )
			{
				sprintf((char *)szNewTelNo, "%s%s;%s%s.",
						glSysParam.stEdcInfo.szPabx, glCurAcq.stTxnPhoneInfo[0].szTelNo,
						glSysParam.stEdcInfo.szPabx, glCurAcq.stTxnPhoneInfo[1].szTelNo);
			}
			else if( glCurAcq.stTxnPhoneInfo[0].szTelNo[0]!=0 )
			{
				sprintf((char *)szNewTelNo, "%s%s.",
						glSysParam.stEdcInfo.szPabx, glCurAcq.stTxnPhoneInfo[0].szTelNo);
			}
			else if( glCurAcq.stTxnPhoneInfo[1].szTelNo[0]!=0 )
			{
				sprintf((char *)szNewTelNo, "%s%s.",
						glSysParam.stEdcInfo.szPabx, glCurAcq.stTxnPhoneInfo[1].szTelNo);
			}
			else
			{
				return ERR_NO_TELNO;
			}
		}
		sprintf((char *)glCommCfg.stPSTNPara.szTelNo, "%s", szNewTelNo);

		if( !CommChkIfSameTelNo(szNewTelNo) )
		{
			CommOnHook(FALSE);
		}
	}

	// get IP address from the current acquirer
	if (glCommCfg.ucCommType == CT_TCPIP)
	{
		if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
		{
			memcpy(&glCommCfg.stTcpIpPara.stHost1, &glSysParam.stEdcInfo.stDownIpAddr, sizeof(IP_ADDR));
		}
		else
		{
			// using transaction IP address, not settlement IP address
		    memcpy(&glCommCfg.stTcpIpPara.stHost1, &glCurAcq.stTxnTCPIPInfo[0], sizeof(IP_ADDR));
		    memcpy(&glCommCfg.stTcpIpPara.stHost2, &glCurAcq.stTxnTCPIPInfo[1], sizeof(IP_ADDR));
		}
	}
	if (glCommCfg.ucCommType == CT_WIFI) //it is using different parameters for wifi and tcp
	{
		if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
		{
		    memcpy(&glCommCfg.stWifiPara.stHost1, &glSysParam.stEdcInfo.stDownIpAddr, sizeof(IP_ADDR));
		}
		else
		{
			// using transaction IP address, not settlement IP address
            memcpy(&glCommCfg.stWifiPara.stHost1, &glCurAcq.stTxnTCPIPInfo[0], sizeof(IP_ADDR));
            memcpy(&glCommCfg.stWifiPara.stHost2, &glCurAcq.stTxnTCPIPInfo[1], sizeof(IP_ADDR));
		}
	}

	// get IP address from the current acquirer
	if ((glCommCfg.ucCommType==CT_GPRS) || (glCommCfg.ucCommType==CT_CDMA) ||
			(glCommCfg.ucCommType==CT_WCDMA))
	{
		if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
		{
		    memcpy(&glCommCfg.stWirlessPara.stHost1, &glSysParam.stEdcInfo.stDownIpAddr, sizeof(IP_ADDR));
		}
		else
		{
			// using transaction IP address, not settlement IP address
		    memcpy(&glCommCfg.stWirlessPara.stHost1, &glCurAcq.stTxnGPRSInfo[0], sizeof(IP_ADDR));
		    memcpy(&glCommCfg.stWirlessPara.stHost2, &glCurAcq.stTxnGPRSInfo[1], sizeof(IP_ADDR));
		}
	}
	

	return CommSetCfgParam(&glCommCfg);
}

// Modified by Kim_LinHB 2014-6-8 v1.01.0000
int DispCommErrMsg(int iErrCode)
{
	COMM_ERR_MSG	stCommErrMsg;

	unsigned char szBuff[200];

	SetClssLightStatus(CLSSLIGHTSTATUS_ERROR);
	CommGetErrMsg(iErrCode, &stCommErrMsg);

	if (iErrCode<0)
	{
		sprintf(szBuff, "%s\n%d", _T(stCommErrMsg.szMsg), iErrCode);
	}
	else
	{
		sprintf(szBuff, "%s\n%02X", _T(stCommErrMsg.szMsg), (iErrCode & MASK_ERR_CODE));
	}

	Gui_ClearScr();
	PubBeepErr();
	return Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, szBuff, gl_stCenterAttr, GUI_BUTTON_NONE, 5, NULL);
}

// end of file

