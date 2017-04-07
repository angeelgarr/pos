﻿/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2006-? PAX Technology, Inc. All rights reserved.
 * Description:     实现EMV交易处理
 * Revision History:
 * Date                  Author                 Action
 * 2006.09.05         shengjx               created
 * ============================================================================
 */

#include "global.h"




#ifdef ENABLE_EMV
uchar	sPinBlock[8];

/********************** Internal macros declaration ************************/
// macros for analyze EMV TLV string
#define TAGMASK_CLASS			0xC0	// tag mask of tag class
#define TAGMASK_CONSTRUCTED		0x20	// tag mask of constructed/primitive data
#define TAGMASK_FIRSTBYTE		0x1F	// tag mask of first byte
#define TAGMASK_NEXTBYTE		0x80	// tag mask of next byte

#define LENMASK_NEXTBYTE		0x80	// length mask
#define LENMASK_LENBYTES		0x7F	// mask of bytes of lenght

#define TAG_NULL_1				0x00	// null tag
#define TAG_NULL_2				0xFF	// null tag

#define DE55_LEN_FIXED		0x01	// for amex
#define DE55_LEN_VAR1		0x02
#define DE55_LEN_VAR2		0x03

#define DE55_MUST_SET		0x10	// 必须存在
#define DE55_OPT_SET		0x20	// 可选择存在
#define DE55_COND_SET		0x30	// 根据条件存在
/********************** Internal structure declaration *********************/
// callback function for GetTLVItem() to save TLV value
typedef void (*SaveTLVData)(uint uiTag, const uchar *psData, int iDataLen);

typedef struct _tagDE55Tag
{
	ushort	uiEmvTag;
	uchar	ucOption;
	uchar	ucLen;		// for process AMEX bit 55, no used for ADVT/TIP
}DE55Tag;

typedef struct _tagScriptInfo
{
	ushort	uiTag;
	int		iIDLen;
	uchar	sScriptID[4];
	int		iCmdLen[20];
	uchar	sScriptCmd[20][300];
}ScriptInfo;

/********************** Internal functions declaration *********************/

static int  SetAmexDE55(const DE55Tag *pstList, uchar *psOutData, int *piOutLen);
static int  SetStdDE55(uchar bForUpLoad, const DE55Tag *pstList, uchar *psOutData, int *piOutLen);
static int  SetStdDE56(const DE55Tag *pstList, uchar *psOutData, int *piOutLen);
static int  GetTLVItem(uchar **ppsTLVString, int iMaxLen, SaveTLVData pfSaveData, uchar bExpandAll);
static int  GetSpecTLVItem(const uchar *psTLVString, int iMaxLen, uint uiSearchTag, uchar *psOutTLV, ushort *puiOutLen);
static int  GetDE55Amex(const uchar *psSendHeader, const uchar *psRecvDE55, int iLen);
static int  IsConstructedTag(uint uiTag);
static void SaveRspICCData(uint uiTag, const uchar *psData, int iDataLen);
static void BuildTLVString(ushort uiEmvTag, const uchar *psData, int iLength, uchar **ppsOutData);
static void SaveEmvData(void);
static void SaveTVRTSI(uchar bBeforeOnline);
static void UpdateEntryModeForOfflinePIN(void);

/********************** Internal variables declaration *********************/
static uchar sAuthData[16];			// authentication data from issuer
static uchar sIssuerScript[300];	// issuer script
// { // for test only
// 	0x71, 0x12+0x0F, 0x9F, 0x18, 0x00, 0x86, 0x0D, 0x84, 0x1E, 0x00, 0x00, 0x08,
// 	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
// 	0x86, 0x0D, 0x84, 0x1E, 0x00, 0x00, 0x08,
// 	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
// 	0x72, 0x12+4, 0x9F, 0x18, 0x04, 0,1,2,3, 0x86, 0x0D, 0x84, 0x1E, 0x00, 0x00, 0x08,
// 	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
// };
//static int iScriptLen=40+15+4;

static int			sgAuthDataLen, sgScriptLen;

// AMEX format field 55
static DE55Tag sgAmexTagList[] =
{
	{0x9F26, DE55_LEN_FIXED, 8},
	{0x9F10, DE55_LEN_VAR1,  32},
	{0x9F37, DE55_LEN_FIXED, 4},
	{0x9F36, DE55_LEN_FIXED, 2},
	{0x95,   DE55_LEN_FIXED, 5},
	{0x9A,   DE55_LEN_FIXED, 3},
	{0x9C,   DE55_LEN_FIXED, 1},
	{0x9F02, DE55_LEN_FIXED, 6},
	{0x5F2A, DE55_LEN_FIXED, 2},
	{0x9F1A, DE55_LEN_FIXED, 2},
	{0x82,   DE55_LEN_FIXED, 2},
	{0x9F03, DE55_LEN_FIXED, 6},
	{0x5F34, DE55_LEN_FIXED, 1},
	{0x9F27, DE55_LEN_FIXED, 1},
	{0x9F06, DE55_LEN_VAR1,  16},
	{0x9F09, DE55_LEN_FIXED, 2},
	{0x9F34, DE55_LEN_FIXED, 3},
	{0x9F0E, DE55_LEN_FIXED, 5},
	{0x9F0F, DE55_LEN_FIXED, 5},
	{0x9F0D, DE55_LEN_FIXED, 5},
	{0},
};

// 消费/(预)授权,55域EMV标签, TLV format
// tags data in field 55 of transaction sale/(pre-)authorization (TLV format)
static DE55Tag sgStdEmvTagList[] =
{
	{0x5F2A, DE55_MUST_SET, 0},
	{0x5F34, DE55_MUST_SET, 1}, // notice it's limited to L=1
	{0x82,   DE55_MUST_SET, 0},
	{0x84,   DE55_MUST_SET, 0},
	{0x95,   DE55_MUST_SET, 0},
	{0x9A,   DE55_MUST_SET, 0},
	{0x9C,   DE55_MUST_SET, 0},
	{0x9F02, DE55_MUST_SET, 0},
	{0x9F03, DE55_MUST_SET, 0},
	{0x9F09, DE55_MUST_SET, 0},
	{0x9F10, DE55_MUST_SET, 0},
	{0x9F1A, DE55_MUST_SET, 0},
	{0x9F1E, DE55_MUST_SET, 0},
	{0x9F26, DE55_MUST_SET, 0},
	{0x9F27, DE55_MUST_SET, 0},
	{0x9F33, DE55_MUST_SET, 0},
	{0x9F34, DE55_MUST_SET, 0},
	{0x9F35, DE55_MUST_SET, 0},
	{0x9F36, DE55_MUST_SET, 0},
	{0x9F37, DE55_MUST_SET, 0},
	{0x9F41, DE55_MUST_SET, 0},
	{0},
};

// 消费/(预)授权,56域EMV标签, TLV format
static DE55Tag sgStdEmvField56TagList[] =
{
	{0x5A,   DE55_MUST_SET, 0},
	{0x95,   DE55_MUST_SET, 0},
	{0x9B,   DE55_MUST_SET, 0},
	{0x9F10, DE55_MUST_SET, 0},
	{0x9F26, DE55_MUST_SET, 0},
	{0x9F27, DE55_MUST_SET, 0},
	{0},
};

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// This is NOT a callback of EMV lib
void AppSetMckParam(uchar bEnablePinBypass)
{
	//在交易处理前调用接口函数  EMVSetMCKParam 设置是否使用批数据采集
	EMV_MCKPARAM	stMckParam;
	EMV_EXTMPARAM	stExtMckParam;

	stMckParam.ucBypassPin = (bEnablePinBypass ? 1 : 0);
	stMckParam.ucBatchCapture = 1;
	memset(&stExtMckParam, 0, sizeof(stExtMckParam));
	stExtMckParam.ucUseTermAIPFlg = 1;
	stExtMckParam.aucTermAIP[0] = 0x08;	// always perform term risk management
	stExtMckParam.ucBypassAllFlg = 1;
	stMckParam.pvoid = &stExtMckParam;
	EMVSetMCKParam(&stMckParam);
}

// Set to default EMV parameter, since it may be changed during last transaction.
void InitTransEMVCfg(void)
{
	//在交易处理前调用接口函数  EMVSetMCKParam 设置是否使用批数据采集
	AppSetMckParam(TRUE);

	EMVGetParameter(&glEmvParam);
	glEmvParam.ForceOnline   = 0;
	memcpy(glEmvParam.CountryCode,   glSysParam.stEdcInfo.stLocalCurrency.sCountryCode, 2);
	memcpy(glEmvParam.TransCurrCode, glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, 2);
	memcpy(glEmvParam.ReferCurrCode, glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, 2);
	glEmvParam.TransCurrExp = glSysParam.stEdcInfo.stLocalCurrency.ucDecimal;
	glEmvParam.ReferCurrExp = glSysParam.stEdcInfo.stLocalCurrency.ucDecimal;
	EMVSetParameter(&glEmvParam);
}

// Modified by Kim_LinHB 2014-5-31
// for displaying a application list to card holder to select
// if there is only one application in the chip, then EMV kernel will not call this callback function
int cEMVWaitAppSel(int TryCnt, EMV_APPLIST List[], int AppNum)
{
	int			iRet, iCnt, iAppCnt;
	GUI_MENU		stAppMenu;
	GUI_MENUITEM	stAppMenuItem[MAX_APP_NUM+1];
	APPLABEL_LIST	stAppList[MAX_APP_NUM];
	int iSelected = 0;

	if( TryCnt!=0 )
	{
		unsigned char szBuff[200];
		sprintf(szBuff, "%s\n%s", _T("NOT ACCEPT"), _T("PLS TRY AGAIN"));
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, szBuff, gl_stCenterAttr, GUI_BUTTON_OK, 3, NULL);
	}

	EMVGetLabelList(stAppList, &iAppCnt);

	PubASSERT( AppNum<=MAX_APP_NUM );
	memset(stAppMenuItem, 0, sizeof(stAppMenuItem));
	for(iCnt=0; iCnt<iAppCnt && iCnt<MAX_APP_NUM; iCnt++)
	{
		stAppMenuItem[iCnt].bVisible = TRUE;	
		stAppMenuItem[iCnt].nValue = iCnt;
		stAppMenuItem[iCnt].vFunc = NULL;
 		sprintf((char *)stAppMenuItem[iCnt].szText, "%d.%.16s", iCnt+1, stAppList[iCnt].aucAppLabel);
	}
	strcpy(stAppMenuItem[iCnt].szText, "");

	Gui_BindMenu(_T("Select App"), gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stAppMenuItem, &stAppMenu);
	Gui_ClearScr();
	iSelected = 0;
	iRet = Gui_ShowMenuList(&stAppMenu, 0, USER_OPER_TIMEOUT, &iSelected);
	if( iRet != GUI_OK)
	{
	    //EPT-19
	    if(iRet == GUI_ERR_TIMEOUT)
	        return EMV_TIME_OUT;
	    return EMV_USER_CANCEL;
	}

	return iSelected;
}

// it is acallback function for EMV kernel, 
// for displaying a amount input box,  
// developer customize
int cEMVInputAmount(ulong *AuthAmt, ulong *CashBackAmt)
{
	uchar	szTotalAmt[20];
	uchar   szBuff[32];

	if( glProcInfo.stTranLog.szAmount[0]!=0 )
	{
		PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
		*AuthAmt = atol((char *)szTotalAmt);
		if( CashBackAmt!=NULL )
		{
			*CashBackAmt = 0L;
		}
	}
	else
	{
		*AuthAmt = 0L;
		if( CashBackAmt!=NULL )
		{
			*CashBackAmt = 0L;
		}
	}
	
	if (glProcInfo.stTranLog.ucTranType ==CASH)
	{
		if( CashBackAmt==NULL )
		{
            if ((EMVReadVerInfo(szBuff)==EMV_OK) && (memcmp(szBuff, "v2", 2)==0))
            {
                // For EMV2x, "v28_7" etc. Not for EMV4xx
			    // Set cash back amount
			    EMVSetTLVData(0x9F03, (uchar *)"\x00\x00\x00\x00\x00\x00", 6);
			    EMVSetTLVData(0x9F04, (uchar *)"\x00\x00\x00\x00", 4);
            }
		}
	}
//	else{
//	    EMVSetTLVData(0x9F03, (uchar *)"\x00\x00\x00\x00\x00\x00", 6);
//	}

	return EMV_OK;
}

// 处理DOL的过程中，EMV库遇到不识别的TAG时会调用该回调函数，要求应用程序处理
// 如果应用程序无法处理，则直接返回-1，提供该函数只为解决一些不符合EMV的特殊
// 应用程序的要求，一般情况下返回-1即可
// Callback function required by EMV core.
// When processing DOL, if there is a tag that EMV core doesn't know about, core will call this function.
// developer should offer processing for proprietary tag.
// if really unable to, just return -1
int cEMVUnknowTLVData(ushort iTag, uchar *psDat, int iDataLen)
{
	uchar sBuff[50];

	switch( iTag )
	{
		/*
		'C' = CASH DESBUS
		'Z' = ATM CASH
		'O' = COLLEGE SCHOOL
		'H' = HOTEL/SHIP
		'X' = TRANSFER
		'A' = AUTO MOBILE/RENTAL
		'F' = RESTAURANT
		'T' = TELEPHONE ORDER PREAUTH
		'U' = UNIQUE TRANS
		'R' = RENTAL/OTHER TRANS
		*/
	case 0x9F53:		// Transaction Category Code (TCC) - Master Card
		*psDat = 'R';	// 0x52;
		break;
	case 0x9F37:
		memset(sBuff, 0, sizeof(sBuff));
		OsGetRandom((uchar*)sBuff, iDataLen);
		memcpy(psDat, sBuff, iDataLen);
		break;
	case 0x9F1E:
		memset(sBuff, 0, sizeof(sBuff));
		ReadSN(sBuff);
		if(0 == strlen(sBuff))
		    memset(sBuff, '0', iDataLen);
		memcpy(psDat, sBuff, iDataLen);
		break;
	case 0x9A:
		memset(sBuff, 0, sizeof(sBuff));
		GetTime(sBuff);
		memcpy(psDat,sBuff,iDataLen);
		break;
	case 0x9F21:
		memset(sBuff, 0, sizeof(sBuff));
		GetTime(sBuff);
		memcpy(psDat,sBuff+3,iDataLen);
		break;
	case 0xFF01:
		break;

	default:
		return -1;
	}

	return EMV_OK;
}

// Callback function required by EMV core.
// Wait holder enter PIN.
// developer customized.
// Modified by Kim_LinHB 2014-6-8 v1.01.0000
int cEMVGetHolderPwd(int iTryFlag, int iRemainCnt, uchar *pszPlainPin)
{
	int		iResult;
	uchar	ucRet, szBuff[30], szAmount[15];

	// online PIN
	if( pszPlainPin==NULL )
	{
		iResult = GetPIN(GETPIN_EMV);
		if( iResult==0 )
		{
			if( glProcInfo.stTranLog.uiEntryMode & MODE_PIN_INPUT )
			{
				return EMV_OK;
			}
			else
			{
				return EMV_NO_PASSWORD;
			}
		}
		else if( iResult==ERR_USERCANCEL )
		{
			return EMV_USER_CANCEL;
		}
		else
		{
			return EMV_NO_PINPAD;
		}
	}

	// Offline plain/enciphered PIN processing below
	Gui_ClearScr();
	if( iRemainCnt==1 )
	{
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr,_T("LAST ENTER PIN"), _ICON_WARNING, gl_stCenterAttr, GUI_BUTTON_NONE, 2, NULL);
	}

	PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szAmount);
	Gui_ClearScr();
	// Modified by Kim_LinHB 2014-8-11 v1.01.0003
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	if( iTryFlag==0 )
	{
	    GetDispAmount(szAmount, szAmount);
		Gui_DrawText(szAmount, gl_stCenterAttr, 0, 25);
	}
	else
	{
		Gui_DrawText(_T("PIN ERR, RETRY"), gl_stCenterAttr, 0, 25);
	}

ENTERPIN:
	Gui_DrawText(_T("PLS ENTER PIN"), gl_stCenterAttr, 0, 50);

	if (ChkTermPEDMode(PED_INT_PCI))
	{
		// Add by lirz v1.02.0000
		if(ChkIssuerOption(ISSUER_EN_EMVPIN_BYPASS) && ChkIfAmex() )
		{
			Gui_ShowMsgBox(NULL, gl_stTitleAttr, _ICON_WARNING, "by-pass not permitted", gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
		}
	   // End add by lirz

		// Offline PIN, done by core itself since EMV core V25_T1. Application only needs to display prompt message.
		// In this mode, cEMVGetHolderPwd() will be called twice. the first time is to display message to user,
		// then back to kernel and wait PIN. afterwards kernel call this again and inform the process result.
		if (pszPlainPin[0]==EMV_PED_TIMEOUT)
		{
			// EMV core has processed PIN entry and it's timeout
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("PED ERROR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
			return EMV_TIME_OUT;
		}
		else if (pszPlainPin[0]==EMV_PED_WAIT)
		{
			// API is called too frequently
			DelayMs(1000);
			Gui_ClearScr();
			// Modified by Kim_LinHB 2014-8-11 v1.01.0003
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

			OsPedSetAsteriskLayout(getPEDAsterisk_X(), getPEDAsterisk_Y(), getPEDAsterisk_H(), BLACK, PED_ASTERISK_ALIGN_CENTER);
			return EMV_OK;
		}
		else if (pszPlainPin[0]==EMV_PED_FAIL)
		{
			// EMV core has processed PIN entry and PED failed.
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("PED ERROR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);

			return EMV_NO_PINPAD;
		}
		else
		{
			// EMV PIN not processed yet. So just display.
			OsPedSetAsteriskLayout(getPEDAsterisk_X(), getPEDAsterisk_Y(), getPEDAsterisk_H(), BLACK, PED_ASTERISK_ALIGN_CENTER);

			return EMV_OK;
		}
	}
	else if (ChkTermPEDMode(PED_EXT_PP))
	{
		Gui_DrawText(_T("PLS USE PINPAD"), gl_stCenterAttr, 0, 75);
		App_ConvAmountTran(szAmount, szBuff, 0);
		// show amount on PINPAD
		ucRet = PPScrCls();
		if( ucRet )
		{
			return EMV_NO_PINPAD;
		}
		PPScrPrint(0, 0, szBuff);
		PPScrClrLine(1);

		memset(sPinBlock, 0, sizeof(sPinBlock));
		ucRet = PPEmvGetPwd(4, 12, sPinBlock);
		switch( ucRet )
		{
		case 0x00:
			return EMV_OK;

		case 0x06:
		case 0x07:
		case 0xC6:
			return EMV_USER_CANCEL;

		case 0x0A:
			if(!ChkIssuerOption(ISSUER_EN_EMVPIN_BYPASS) && ChkIfAmex())
			{
				PPScrCls();
				PPScrPrint(1,0," NOT PERMITTED");
				PPBeep();

				Gui_ClearScr();
				Beef(6, 200);
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_WARNING, _T("NOT PERMITTED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);

				goto ENTERPIN;
			}
			else
			{
				return EMV_NO_PASSWORD;
			}

		default:
			return EMV_NO_PINPAD;
		}
	} 
	else	// PED_EXT_PCI
	{
		// !!!! extern PCI, to be implemented.
		unsigned char szBuff[200];
		sprintf(szBuff, "%s\n%s", _T("EXT PCI PINPAD"), _T("NOT IMPLEMENTED"));
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_WARNING, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, 30, NULL);
		return ERR_NO_DISP;
	}
}

// 语音参考处理
// 如果不支持，如果是卡片发起的参考，
// 则可根据发卡行要求直接返回REFER_DENIAL或REFER_ONLINE,
// 一般情况下不应该直接返回REFER_APPROVE(除非发卡行要求这么做)

// 如果不支持，如果是发卡行(主机)发起的参考，
// 则可根据发卡行要求直接返回REFER_DENIAL
// 一般情况下不应该直接返回REFER_APPROVE(除非发卡行要求这么做)

// 下边是参考的处理代码，供参考
// Callback function required by EMV core.
// Voice referral.
// If not support, return REFER_DENIAL.
int cEMVReferProc(void)
{
	return REFER_DENIAL;
}

// Callback function required by EMV core.
// TC advise after EMV transaction. If not support, immediately return.
void cEMVAdviceProc(void)
{
	//	脱机通知的处理：
	//	通过函数EMVGetTLVData()获得通知数据包需要的数据，存贮到交易日志中，
	//	然后在交易结算时，再联机发送到主机。
	//	需要注意的是：通知数据包的任何数据(比如金额等)不可以用于交易对帐。

	//	联机通知的处理：
	//	(1)拨号连接主机。
	//	(2)通过函数EMVGetTLVData()获得通知数据包需要的数据，再发送到主机。
	//	需要注意的是：联机通知方式在我们的POS机中应该不会使用。
}

//联机处理
/*
	处理步骤：
	(1)拨号连接主机,如果所有交易都要联机，那么可以在插入IC卡时预拨号,
	   如果拨号失败返回ONLINE_FAILED
	(2)通过函数EMVGetTLVData()获得交易数据包需要的数据，并打包。
	(3)保存冲正数据及标志,然后发送交易数据包到主机(冲正处理完全由应用完成)
	(4)接收主机的回应数据包,根据主机的回应，做如下返回：
	   A.如果主机返回批准，则根据返回数据填写RspCode、AuthCode、AuthCodeLen等
		 参数的值，并返回ONLINE_APPROVE
	   B.如果主机拒绝交易,则根据返回数据填写RspCode,如果其他参数也有数据值，
		 同样需要填写，返回ONLINE_DENIAL
	   C.如果主机请求语音参考,则根据返回数据填写RspCode,如果其他参数也有数据值，
		 同样需要填写，返回ONLINE_REFER。需要说明的是：很多情况可能没有参考处理，
		 在这种情况下，应用程序就不需要返回ONLINE_REFER了

	等交易处理成功后，应用程序才可以清除冲正标志。
*/
/* Online processing.
    steps:
	(1) Dial. If dial failed, return ONLINE_FAILED
	(2) Use EMVGetTLVData() to retrieve data from core, pack to ISO8583.
	(3) Save reversal data and flag, then send request to host
	(4) Receive from host, then do accordingly:
	   A. If host approved, copy RspCode,AuthCode,AuthCodeLen or so, and return ONLINE_APPROVE
	   B. If host denial, copy RspCode or so, return ONLINE_DENIAL
	   C. If host require voice referral, copy RspCode or so.,return ONLINE_REFER.
	       Note that if not support, needn't return ONLINE_REFER but directly ONLINE_DENIAL

	Reversal flag can only be cleared after all EMV processing, NOT immediately after online.
*/
int  cEMVOnlineProc(uchar *psRspCode,  uchar *psAuthCode, int *piAuthCodeLen,
					uchar *psAuthData, int *piAuthDataLen,
					uchar *psScript,   int *piScriptLen)
{
	int		iRet, iLength, iRetryPIN;
	ulong	ulICCDataLen;
	uchar	*psICCData, *psTemp;

	// initialize output parameters
	*psRspCode      = 0;
	*piAuthCodeLen  = 0;
	*piAuthDataLen  = 0;
	*piScriptLen    = 0;
	SaveTVRTSI(TRUE);
	glProcInfo.bIsFirstGAC = FALSE;

    //add begin. fixed bug about not pack F52 when use chip card. linzhao 20160510
    if (glProcInfo.stTranLog.uiEntryMode & MODE_PIN_INPUT)
    {
       if(memcmp(glProcInfo.sPinBlock,"\x00\x00\x00\x00\x00\x00\x00\x00",8) !=0)
       {
           PubLong2Char((ulong)LEN_PIN_DATA, 2, glSendPack.sPINData);
           memcpy(&glSendPack.sPINData[2], glProcInfo.sPinBlock, LEN_PIN_DATA);
       }

    }
    //add end.linzhao 20160510

	UpdateEntryModeForOfflinePIN();

	// prepare online DE55 data
	iRet = SetDE55(DE55_SALE, &glSendPack.sICCData[2], &iLength);
	if( iRet!=0 )
	{
		glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
		return ONLINE_FAILED;
	}
	PubASSERT( iLength<LEN_ICC_DATA );
	PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
	memcpy(glProcInfo.stTranLog.sIccData, &glSendPack.sICCData[2], iLength);	// save for batch upload
	glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;

	if( !ChkIfAmex() )
	{
		if( !(ChkIfDah() || ChkIfCiti() || ChkIfScb()) &&
            ChkIfAcqNeedDE56() )
		{
			if ( glProcInfo.stTranLog.ucTranType!=AUTH && glProcInfo.stTranLog.ucTranType!=PREAUTH )
			{
				iLength = glSysCtrl.stField56[glCurAcq.ucIndex].uiLength;
				PubASSERT(iLength<LEN_ICC_DATA);
				if( iLength>0 )
				{
					memcpy(&glSendPack.sICCData2[2], glSysCtrl.stField56[glCurAcq.ucIndex].sData, iLength);
				}
				else
				{
					SetStdEmptyDE56(&glSendPack.sICCData2[2], &iLength);
				}
				PubLong2Char((ulong)iLength, 2, glSendPack.sICCData2);
			}
		}
	}

	// 冲正交易处理 & 离线交易上送
	// 判断上次交易是否需要进行冲正等
	// send reversal here. If required by bank, also send offline here
	iRet = TranReversal();
	if( iRet!=0 )
	{
		glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
		return ONLINE_FAILED;
	}

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
			glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
			return ONLINE_FAILED;
		}

		if( memcmp(glRecvPack.szRspCode, "55", 2)!=0 || ++iRetryPIN>3 || !ChkIfNeedPIN() )
		{
			break;
		}

		// retry EMV online PIN
		iRet = GetPIN(GETPIN_RETRY);
		if( iRet!=0 )
		{
			return ONLINE_DENIAL;
		}
		sprintf((char *)glSendPack.szSTAN, "%06lu", glSysCtrl.ulSTAN);
		memcpy(&glSendPack.sPINData[0], "\x00\x08", 2);
		memcpy(&glSendPack.sPINData[2], glProcInfo.sPinBlock, 8);
	}

	// set response code
	memcpy(psRspCode,  glRecvPack.szRspCode,  LEN_RSP_CODE);
	glProcInfo.ucOnlineStatus = ST_ONLINE_APPV;

	// get response issuer data
	sgAuthDataLen = sgScriptLen = 0;

	ulICCDataLen = PubChar2Long(glRecvPack.sICCData, 2);
	psICCData = &glRecvPack.sICCData[2];
	if (ChkIfAmex())
	{
		iRet = GetDE55Amex(glSendPack.sICCData+2, psICCData, ulICCDataLen);
		if( iRet<0 )
		{
			// if analyze response ICC data failed, return fail
			glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
			return ONLINE_FAILED;
		}
	}
	else
	{
		for(psTemp=psICCData; psTemp<psICCData+ulICCDataLen; )
		{
			// version 1.00.0016 change by Jolie Yang at 2013-08-16
			// due to the application just only get contents of 71\72, and transfer it to EMV kernal.
			iRet = GetTLVItem(&psTemp, psICCData+ulICCDataLen-psTemp, SaveRspICCData, FALSE);
			//iRet = GetTLVItem(&psTemp, psICCData+ulICCDataLen-psTemp, SaveRspICCData, TRUE);
			//if( iRet<0 )
			//{	// if analyze response ICC data failed, return fail
			//	glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
			//	return ONLINE_FAILED;
			//}
		}
	}


	memcpy(psAuthData, sAuthData, sgAuthDataLen);
	*piAuthDataLen = sgAuthDataLen;
	
	// version 1.00.0016 change by Jolie Yang at 2013-08-16
	// due to the application need not extract the sub-tag of 71/72, just get contents of 71\72, and transfer to EMV kernal
	// AdjustIssuerScript();
	memcpy(psScript, sIssuerScript, sgScriptLen);
	*piScriptLen = sgScriptLen;

	if( memcmp(glRecvPack.szRspCode, "00", LEN_RSP_CODE)!=0 )
	{
		return ONLINE_DENIAL;
	}
	// set authorize code only if txn approved
	memcpy(psAuthCode, glRecvPack.szAuthCode, LEN_AUTH_CODE);
	*piAuthCodeLen = strlen((char *)glRecvPack.szAuthCode);

	return ONLINE_APPROVE;
}

// 如果不需要提示密码验证成功，则直接返回就可以了
// Callback function required by EMV core.
// Display "EMV PIN OK" info. (plaintext/enciphered PIN)
// Modified by Kim_LinHB 2014-6-8 v1.01.0000
void cEMVVerifyPINOK(void)
{
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(),  gl_stTitleAttr, _ICON_SUCCESS, _T("PIN VERIFY OK"), gl_stCenterAttr, GUI_BUTTON_OK, 1, NULL);
}

// 持卡人认证例程
// Callback function required by EMV core.
// Don't need to care about this function
int cCertVerify(void)
{
//	AppSetMckParam(!ChkIssuerOption(ISSUER_EN_EMVPIN_BYPASS));
	return -1;
}

// Callback function required by EMV core.
// in EMV ver 2.1+, this function is called before GPO
int cEMVSetParam(void)
{
	return 0;
}

int FinishEmvTran(void)
{
	int		iRet, iLength;


	// 根据需要设置是否强制联机
	// decide whether need force online
	EMVGetParameter(&glEmvParam);
	glEmvParam.ForceOnline = ((glProcInfo.stTranLog.ucTranType!=SALE )? 1 : 0);
	EMVSetParameter(&glEmvParam);
	// Only in this transaction, so DON'T back up

	// clear last EMV status
	memset(&glEmvStatus, 0, sizeof(EMV_STATUS));
	SaveEmvStatus();


	// Process EMV transaction.
	iRet = EMVProcTrans();
	SaveTVRTSI(FALSE);
	UpdateEntryModeForOfflinePIN();
	if( iRet==EMV_TIME_OUT || iRet==EMV_USER_CANCEL )
	{
		return ERR_USERCANCEL;
	}
	if( (glProcInfo.ucOnlineStatus==ST_ONLINE_APPV) && memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)==0 )
	{
		if( (glProcInfo.stTranLog.ucTranType!=AUTH) &&
			(glProcInfo.stTranLog.ucTranType!=PREAUTH) &&
			ChkIfAcqNeedDE56())
		{
			SetDE56(glSysCtrl.stField56[glCurAcq.ucIndex].sData, &iLength);
			glSysCtrl.stField56[glCurAcq.ucIndex].uiLength = (ushort)iLength;
			SaveField56();

			// if online approved, save bit 56 for void/upload etc
			memcpy(glProcInfo.stTranLog.sField56, glSysCtrl.stField56[glCurAcq.ucIndex].sData, iLength);
			glProcInfo.stTranLog.uiField56Len = (ushort)iLength;
		}

		// update for reversal(maybe have script result)
		SetDE55(DE55_SALE, &glSendPack.sICCData[2], &iLength);
		PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
		glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
		SaveRevInfo(TRUE);	// update reversal information
	}
	if( iRet!=EMV_OK )
	{
		SaveEmvErrLog();

		// for sale completion only send 5A not 57 [1/11/2007 Tommy]
		if( !ChkIfAmex() && (glProcInfo.ucOnlineStatus!=ST_OFFLINE) &&
			(memcmp(glProcInfo.stTranLog.szRspCode, "01", 2)==0 ||
			 memcmp(glProcInfo.stTranLog.szRspCode, "02", 2)==0)
			)
		{
			SetDE55(DE55_UPLOAD, glProcInfo.stTranLog.sIccData, &iLength);
			glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
		}

		if( glProcInfo.stTranLog.szRspCode[0]!=0 &&
			memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)!=0 )
		{	// show reject code from host
			return AfterTranProc();
		}
		return ERR_TRAN_FAIL;
	}

	// transaction approved. save EMV data
	SaveEmvData();
	if( glProcInfo.ucOnlineStatus!=ST_ONLINE_APPV )
	{
		return FinishOffLine();
	}
	
	if (ChkIfAcqNeedTC())
	{
		glProcInfo.stTranLog.uiStatus |= TS_NEED_TC;
	}

	return AfterTranProc();
}

// Set bit 55 data for online transaction.
int SetDE55(DE55_TYPE ucType, uchar *psOutData, int *piOutLen )
{
	if( ChkIfAmex() )
	{
		return SetAmexDE55(sgAmexTagList, psOutData, piOutLen);
	}
	else
	{
		return SetStdDE55((uchar)ucType, sgStdEmvTagList, psOutData, piOutLen);
	}
}

// set AMEX bit 55, structure of TLV items
int SetAmexDE55(const DE55Tag *pstList, uchar *psOutData, int *piOutLen)
{
	int		iRet, iCnt, iLength;
	uchar	*psTemp, sBuff[128];

	*piOutLen = 0;
	memcpy(psOutData, "\xC1\xC7\xD5\xE2\x00\x01", 6);	// AMEX header
	psTemp = psOutData+6;

	for(iCnt=0; pstList[iCnt].uiEmvTag!=0; iCnt++)
	{
		iLength = 0;
		memset(sBuff, 0, sizeof(sBuff));
		iRet = EMVGetTLVData(pstList[iCnt].uiEmvTag, sBuff, &iLength);
		if( (iRet!=EMV_OK ) && (iRet!=EMV_NO_DATA))
		{
			return ERR_TRAN_FAIL;
		}

		if(iRet==EMV_NO_DATA)
		{
			iLength = pstList[iCnt].ucLen;
			memset(sBuff, 0, sizeof(sBuff));
		}

		if( pstList[iCnt].ucOption==DE55_LEN_VAR1 )
		{
			*psTemp++ = (uchar)iLength;
		}
		else if( pstList[iCnt].ucOption==DE55_LEN_VAR2 )
		{
			*psTemp++ = (uchar)(iLength>>8);
			*psTemp++ = (uchar)iLength;
		}
		memcpy(psTemp, sBuff, iLength);
		psTemp += iLength;
	}
	*piOutLen = (psTemp-psOutData);

	return 0;
}

// set ADVT/TIP bit 55
int SetStdDE55(uchar bForUpLoad, const DE55Tag *pstList, uchar *psOutData, int *piOutLen)
{
	int		iRet, iCnt, iLength;
	uchar	*psTemp, sBuff[200];
	DE55Tag	astLocalTaglist[64];

	*piOutLen = 0;
	psTemp    = psOutData;

	// ???? MODE_FALLBACK_MANUAL ????
	if ( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE ) ||
		 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
		if( ChkIfBoc() || ChkIfFubon() || ChkIfBea() )
		{
			memcpy(psTemp, "\xDF\xEC\x01\x01", 4);
			psTemp += 4;
			memcpy(psTemp, "\xDF\xED\x01", 3);
			psTemp += 3;

			if( ChkIfBea() )
			{
				*psTemp++ = LastRecordIsFallback() ? 0x02 : 0x01;
			}
			else
			{
				if( glProcInfo.iFallbackErrCode==EMV_NO_APP ||
					glProcInfo.iFallbackErrCode==ICC_RESET_ERR )
				{
					*psTemp++ = 0x02;
				}
				else
				{
					*psTemp++ = 0x01;
				}
			}
		}
		else
		{
			memcpy(psTemp, "\xDF\x5A\x01\x01", 4);
			psTemp += 4;
			memcpy(psTemp, "\xDF\x39\x01\x01", 4);
			psTemp += 4;
		}
	}
	else if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	{
		// Copy from std tag list
		//-----------------------------------------------------------
		memset(astLocalTaglist, 0, sizeof(astLocalTaglist));
		for(iCnt=0; pstList[iCnt].uiEmvTag!=0; iCnt++)
		{
			astLocalTaglist[iCnt] = pstList[iCnt];
		}
		
		//-----------------------------------------------------------
		// Customize tag list according to different acquirer
		//RemoveFromTagList(astLocalTaglist, 0x5F34);
		//AppendStdTagList(astLocalTaglist, 0x5A, DE55_MUST_SET, 0);

		//-----------------------------------------------------------
		// Generate data by tag list
		for(iCnt=0; pstList[iCnt].uiEmvTag!=0; iCnt++)
		{
			memset(sBuff, 0, sizeof(sBuff));
			iRet = EMVGetTLVData(pstList[iCnt].uiEmvTag, sBuff, &iLength);
			if( iRet==EMV_OK )
			{
				if ((pstList[iCnt].ucLen > 0) && (iLength > pstList[iCnt].ucLen))
				{
					iLength = pstList[iCnt].ucLen;
				}
				BuildTLVString(pstList[iCnt].uiEmvTag, sBuff, iLength, &psTemp);
			}
			else if( pstList[iCnt].ucOption==DE55_MUST_SET )
			{
				BuildTLVString(pstList[iCnt].uiEmvTag, NULL, 0, &psTemp);
//				return ERR_TRAN_FAIL;	// 如果必须存在的TAG不存在,交易失败
			}
		}

		//-----------------------------------------------------------
		// Generate custom tag content
		if( glProcInfo.stTranLog.szPan[0]=='5' )
		{	// for master card TCC = "R" -- retail
			BuildTLVString(0x9F53, (uchar *)"R", 1, &psTemp);
		}

		memset(sBuff, 0, sizeof(sBuff));
		iRet = EMVGetScriptResult(sBuff, &iLength);
		if( iRet==EMV_OK )
		{
			BuildTLVString(0xDF5B, sBuff, iLength, &psTemp);
		}
	}
	else
	{
		return 0;
	}

	if( ChkIfBoc() )
	{
		memcpy(psTemp, "\xDF\xEE\x01\x05", 4);
		psTemp += 4;
	}

	*piOutLen = (psTemp-psOutData);

	return 0;
}

int SetTCDE55(void *pstTranLog, uchar *psOutData, int *piOutLen)
{
    char    sBuff[LEN_ICC_DATA];
    ushort  uiLen;
    int     iRet;

    if (ChkIfICBC_MACAU())
    {
        // ICBC-Macau only need 9F26 in TC DE55
        *piOutLen = 0;
        iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0x9F26, sBuff, &uiLen);
        if (iRet==0)
        {
            memcpy(psOutData, sBuff, uiLen);
            psOutData += uiLen;
            *piOutLen += uiLen;
        }
        return 0;
    }
    else if (ChkIfDah() || ChkIfWingHang())
    {
        *piOutLen = 0;
        iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0x9F26, sBuff, &uiLen);
        if (iRet==0)
        {
            memcpy(psOutData, sBuff, uiLen);
            psOutData += uiLen;
            *piOutLen += uiLen;
        }
        iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0x9F27, sBuff, &uiLen);
        if (iRet==0)
        {
            memcpy(psOutData, sBuff, uiLen);
            psOutData += uiLen;
            *piOutLen += uiLen;
        }
        return 0;
    }

    *piOutLen = ((TRAN_LOG *)pstTranLog)->uiIccDataLen;
    memcpy(psOutData, ((TRAN_LOG *)pstTranLog)->sIccData, *piOutLen);
    return 0;
}

//Set 56 field
int SetDE56(uchar *psOutData, int *piOutLen)
{
	*piOutLen = 0;
	if( ChkIfAmex() )
	{
		return 0;
	}

	return SetStdDE56(sgStdEmvField56TagList, psOutData, piOutLen);
}

int SetStdEmptyDE56(uchar *psOutData, int *piOutLen)
{
	if( ChkIfAmex() )
	{
		*piOutLen = 0;
		return 0;
	}

	if( ChkIfBea() )
	{
		memcpy(psOutData, "\xDF\xF0\x0D\x00\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20", 16);
		*piOutLen = 16;
	}
	else if( ChkIfBoc() || ChkIfFubon() )
	{
		memcpy(psOutData, "\xDF\xF0\x07\x00\x20\x20\x20\x20\x20\x20", 10);
		*piOutLen = 10;
	}
	else
	{
		memcpy(psOutData, "\xDF\x5C\x07\x00\x20\x20\x20\x20\x20\x20", 10);
		*piOutLen = 10;
	}

	return 0;
}

int SetStdDE56(const DE55Tag *pstList, uchar *psOutData, int *piOutLen)
{
	int		iRet, iCnt, iLength;
	uchar	*psTemp, sBuff[110];

	// build header of bit 56
	*piOutLen = 0;
	psTemp    = psOutData;
	if( ChkIfBea() )
	{
		memcpy(psTemp, "\xDF\xF0\x0D\x01", 4);
		psTemp += 4;
		PubLong2Bcd(glProcInfo.stTranLog.ulInvoiceNo, 3, psTemp);
		psTemp += 3;
		PubLong2Bcd(glProcInfo.stTranLog.ulSTAN, 3, psTemp);
		psTemp += 3;
		PubAsc2Bcd(glProcInfo.stTranLog.szRRN, 12, psTemp);
		psTemp += 6;
	}
	else
	{
		if( ChkIfBoc() || ChkIfFubon() )
		{
			memcpy(psTemp, "\xDF\xF0\x07\x01", 4);
		}
		else
		{
			memcpy(psTemp, "\xDF\x5C\x07\x01", 4);
		}
		psTemp += 4;

		PubLong2Bcd(glProcInfo.stTranLog.ulInvoiceNo, 3, psTemp);
		psTemp += 3;
		PubLong2Bcd(glProcInfo.stTranLog.ulSTAN, 3, psTemp);
		psTemp += 3;
	}

	// build common EMV core tags for all HK banks
	for(iCnt=0; pstList[iCnt].uiEmvTag!=0; iCnt++)
	{
		memset(sBuff, 0, sizeof(sBuff));
		iRet = EMVGetTLVData(pstList[iCnt].uiEmvTag, sBuff, &iLength);
		if( iRet==EMV_OK )
		{
			BuildTLVString(pstList[iCnt].uiEmvTag, sBuff, iLength, &psTemp);
		}
		else
		{
			BuildTLVString(pstList[iCnt].uiEmvTag, NULL, 0, &psTemp);
		}
	}

	// process special EMC core tags for different banks
	if( ChkIfFubon() )
	{
		memset(sBuff, 0, sizeof(sBuff));
		EMVGetTLVData(0x9A, sBuff, &iLength);
		BuildTLVString(0x9A, sBuff, iLength, &psTemp);
	}
	if( ChkIfHSBC() )
	{
		memset(sBuff, 0, sizeof(sBuff));
		EMVGetTLVData(0x9F36, sBuff, &iLength);
		BuildTLVString(0x9F36, sBuff, iLength, &psTemp);
	}

	memset(sBuff, 0, sizeof(sBuff));
	iRet = EMVGetScriptResult(sBuff, &iLength);
	if( iRet!=EMV_OK )
	{
		*piOutLen = (psTemp-psOutData);
		return 0;
	}

	// continue issuer script result process
	if( ChkIfBoc() || ChkIfFubon() || ChkIfBea() )
	{
		memcpy(psTemp, "\xDF\x91", 2);
	}
	else if( ChkIfDah() || ChkIfScb() || ChkIfCiti() )
	{
		memcpy(psTemp, "\x9F\x5B", 2);
	}
	else
	{
		memcpy(psTemp, "\xDF\x5B", 2);
	}
	psTemp   += 2;
	*psTemp++ = (uchar)iLength;
	memcpy(psTemp, sBuff, iLength);
	psTemp += iLength;

	*piOutLen = (psTemp-psOutData);

	return 0;
}

// bExpandAll:       TRUE: expand constructed item, FALSE: not
int GetTLVItem(uchar **ppsTLVString, int iMaxLen, SaveTLVData pfSaveData, uchar bExpandAll)
{
	int			iRet;
	uchar		*psTag, *psSubTag;
	uint		uiTag, uiLenBytes;
	ulong		lTemp;

	// skip null tags
	for(psTag=*ppsTLVString; psTag<*ppsTLVString+iMaxLen; psTag++)
	{
		if( (*psTag!=TAG_NULL_1) && (*psTag!=TAG_NULL_2) )
		{
			break;
		}
	}
	if( psTag>=*ppsTLVString+iMaxLen )
	{
		*ppsTLVString = psTag;
		return 0;	// no tag available
	}

	// process tag bytes
	uiTag = *psTag++;
	if( (uiTag & TAGMASK_FIRSTBYTE)==TAGMASK_FIRSTBYTE )
	{	// have another byte
		uiTag = (uiTag<<8) + *psTag++;
	}
	if( psTag>=*ppsTLVString+iMaxLen )
	{
		return -1;
	}

	// process length bytes
	if( (*psTag & LENMASK_NEXTBYTE)==LENMASK_NEXTBYTE )
	{
		uiLenBytes = *psTag & LENMASK_LENBYTES;
		lTemp = PubChar2Long(psTag+1, uiLenBytes);
	}
	else
	{
		uiLenBytes = 0;
		lTemp      = *psTag & LENMASK_LENBYTES;
	}
	psTag += uiLenBytes+1;
	if( psTag+lTemp>*ppsTLVString+iMaxLen )
	{
		return -2;
	}
	*ppsTLVString = psTag+lTemp;	// advance pointer of TLV string

	// save data
	(*pfSaveData)(uiTag, psTag, (int)lTemp);
	if( !IsConstructedTag(uiTag) || !bExpandAll )
	{
		return 0;
	}

	// constructed data
	for(psSubTag=psTag; psSubTag<psTag+lTemp; )
	{
		iRet = GetTLVItem(&psSubTag, psTag+lTemp-psSubTag, pfSaveData, TRUE);
		if( iRet<0 )
		{
			return iRet;
		}
	}

	return 0;
}

int GetSpecTLVItem(const uchar *psTLVString, int iMaxLen, uint uiSearchTag, uchar *psOutTLV, ushort *puiOutLen)
{
	uchar		*psTag, *psTagStr;
	uint		uiTag, uiLenBytes;
	ulong		lTemp;

	// skip null tags
    for (psTag=(uchar *)psTLVString; psTag<psTLVString+iMaxLen; psTag++)
    {
        if ((*psTag!=TAG_NULL_1) && (*psTag!=TAG_NULL_2))
        {
            break;
        }
    }
    if ( psTag>=psTLVString+iMaxLen )
    {
        return -1;	// no tag available
    }
    
    while (1)
    {
        psTagStr = psTag;
        // process tag bytes
        uiTag = *psTag++;
        if ((uiTag & TAGMASK_FIRSTBYTE)==TAGMASK_FIRSTBYTE)
        {	// have another byte
            uiTag = (uiTag<<8) + *psTag++;
        }
        if (psTag>=psTLVString+iMaxLen)
        {
            return -2;	// specific tag not found
        }
        
        // process length bytes
        if ((*psTag & LENMASK_NEXTBYTE)==LENMASK_NEXTBYTE)
        {
            uiLenBytes = *psTag & LENMASK_LENBYTES;
            lTemp = PubChar2Long(psTag+1, uiLenBytes);
        }
        else
        {
            uiLenBytes = 0;
            lTemp      = *psTag & LENMASK_LENBYTES;
        }
        psTag += uiLenBytes+1;
        if (psTag+lTemp>psTLVString+iMaxLen)
        {
            return -2;	// specific tag not found also
        }
        
        // Check if tag needed
        if (uiTag==uiSearchTag)
        {
            *puiOutLen = (ushort)(psTag-psTagStr+lTemp);
            memcpy(psOutTLV, psTagStr, *puiOutLen);
            return 0;
        }
        
        if (IsConstructedTag(uiTag))
        {
            if (GetSpecTLVItem(psTag, (int)lTemp, uiSearchTag, psOutTLV, puiOutLen)==0)
            {
                return 0;
            }
        }
        
        psTag += lTemp;	// advance pointer of TLV string
        if (psTag>=psTLVString+iMaxLen)
        {
            return -2;
        }
    }

    return 0;
}


int GetDE55Amex(const uchar *psSendHeader, const uchar *psRecvDE55, int iLen)
{
	uchar	*psTmp;
	uint	uiLenBytes;

	// invalid length
	if (iLen<6)
	{
		return -1;
	}
	// header mismatch
	if (memcmp(psSendHeader, psRecvDE55, 6)!=0)
	{
		return -1;
	}

	psTmp = (uchar *)psRecvDE55+6;

	// Data Sub Field 1 : Issuer Authentication Data EMV (Tag 91)
	uiLenBytes = *psTmp++;
	if (uiLenBytes>16)
	{
		return -2;
	}
	memcpy(sAuthData, psTmp, uiLenBytes);
	sgAuthDataLen = uiLenBytes;
	psTmp += uiLenBytes;
	if (psTmp-psRecvDE55>iLen)
	{
		return -3;
	}
	if (psTmp-psRecvDE55==iLen)	// end up
	{
		return 0;
	}

	// Data Sub Field 2 : Issuer Script Data
	uiLenBytes = *psTmp++;
	if (uiLenBytes>128)
	{
		return -2;
	}
	sgScriptLen = 0;
	memcpy(&sIssuerScript[sgScriptLen], psTmp, uiLenBytes);
	sgScriptLen += uiLenBytes;
	psTmp += uiLenBytes;
	if (psTmp-psRecvDE55>iLen)
	{
		return -3;
	}
	if (psTmp-psRecvDE55==iLen)	// end up
	{
		return 0;
	}

	return 0;
}

int IsConstructedTag(uint uiTag)
{
	int		i;

	for(i=0; (uiTag&0xFF00) && i<2; i++)
	{
		uiTag >>= 8;
	}

	return ((uiTag & TAGMASK_CONSTRUCTED)==TAGMASK_CONSTRUCTED);
}

// Save Iuuser Authentication Data, Issuer Script.
void SaveRspICCData(uint uiTag, const uchar *psData, int iDataLen)
{
	switch( uiTag )
	{
	case 0x91:
		memcpy(sAuthData, psData, MIN(iDataLen, 16));
		sgAuthDataLen = MIN(iDataLen, 16);
		break;

	case 0x71:
	case 0x72:
		sIssuerScript[sgScriptLen++] = (uchar)uiTag;
		if( iDataLen>127 )
		{
			sIssuerScript[sgScriptLen++] = 0x81;
		}
		sIssuerScript[sgScriptLen++] = (uchar)iDataLen;
		memcpy(&sIssuerScript[sgScriptLen], psData, iDataLen);
		sgScriptLen += iDataLen;
		break;

	case 0x9F36:
//		memcpy(sATC, psData, MIN(iDataLen, 2));	// ignore
		break;

	default:
		break;
	}
}

// 只处理基本数据元素Tag,不包括结构/模板类的Tag
// Build basic TLV data, exclude structure/template.
void BuildTLVString(ushort uiEmvTag, const uchar *psData, int iLength, uchar **ppsOutData)
{
	uchar	*psTemp;

	if( iLength<0 )
	{
		return;
	}

	// set tags
	psTemp = *ppsOutData;
	if( uiEmvTag & 0xFF00 )
	{
		*psTemp++ = (uchar)(uiEmvTag >> 8);
	}
	*psTemp++ = (uchar)uiEmvTag;

	// set length
	// for now, lengths of all data are less then 127 bytes, but still extend the rest part based on standard
	if( iLength<=127 )	
	{	
		*psTemp++ = (uchar)iLength;
	}
	else
	{	
		// the upper limit is 255 bytes data defined by EMV standard
		*psTemp++ = 0x81;
		*psTemp++ = (uchar)iLength;
	}

	// set value
	if( iLength>0 )
	{
		memcpy(psTemp, psData, iLength);
		psTemp += iLength;
	}

	*ppsOutData = psTemp;
}

// Retrieve EMV data from core, for saving record or upload use.
void SaveEmvData(void)
{
	int		iLength;

	EMVGetTLVData(0x9F26, glProcInfo.stTranLog.sAppCrypto, &iLength);
	EMVGetTLVData(0x8A,   glProcInfo.stTranLog.szRspCode,  &iLength);
	EMVGetTLVData(0x95,   glProcInfo.stTranLog.sTVR,       &iLength);
	EMVGetTLVData(0x9B,   glProcInfo.stTranLog.sTSI,       &iLength);
	EMVGetTLVData(0x9F36, glProcInfo.stTranLog.sATC,       &iLength);

	// save for upload
	SetDE55(DE55_UPLOAD, glProcInfo.stTranLog.sIccData, &iLength);
	glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;

	if( glProcInfo.ucOnlineStatus!=ST_ONLINE_APPV )
	{	
		// ICC脱机, offline approved
		SaveTVRTSI(TRUE);
		GetNewTraceNo();
//		sprintf((char *)glProcInfo.stTranLog.szRspCode, "00");
//		sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");
		if(ChkIfAmex())
		{
			if(glProcInfo.ucOnlineStatus==ST_ONLINE_FAIL)
			{
				sprintf((char *)glProcInfo.stTranLog.szAuthCode, "Y3");
			} 
			else
			{
				// for AMEX, approval code = Y1 while chip off line apporved.
				sprintf((char *)glProcInfo.stTranLog.szAuthCode, "Y1");	
			}		
		}
		else
		{
			sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%06lu", glSysCtrl.ulSTAN);
		}

		if (ChkIfAcqNeedDE56())
		{
			SetDE56(glProcInfo.stTranLog.sField56, &iLength);
			glProcInfo.stTranLog.uiField56Len = (ushort)iLength;
		}
	}
}

// save EMV status for FUNC 9
void SaveTVRTSI(uchar bBeforeOnline)
{
	int				iRet, iLength, iCnt;
	uchar			sTermAID[16], sBuff[512];
	uchar			*psTLVData;
	EMV_APPLIST		stEmvApp;
	DE55Tag stList[] =
	{
		{0x5A,   DE55_MUST_SET, 0},
		{0x5F2A, DE55_MUST_SET, 0},
		{0x5F34, DE55_MUST_SET, 0},
		{0x82,   DE55_MUST_SET, 0},
		{0x84,   DE55_MUST_SET, 0},
		{0x8A,   DE55_MUST_SET, 0},
		{0x95,   DE55_MUST_SET, 0},
		{0x9A,   DE55_MUST_SET, 0},
		{0x9C,   DE55_MUST_SET, 0},
		{0x9F02, DE55_MUST_SET, 0},
		{0x9F03, DE55_MUST_SET, 0},
		{0x9F09, DE55_MUST_SET, 0},
		{0x9F10, DE55_MUST_SET, 0},
		{0x9F1A, DE55_MUST_SET, 0},
		{0x9F1E, DE55_MUST_SET, 0},
		{0x9F33, DE55_MUST_SET, 0},
		{0x9F34, DE55_MUST_SET, 0},
		{0x9F35, DE55_MUST_SET, 0},
		{0x9F36, DE55_MUST_SET, 0},
		{0x9F37, DE55_MUST_SET, 0},
		{0x9F41, DE55_MUST_SET, 0},
		{0},
	};

	SetStdDE55(FALSE, stList, glEmvStatus.sTLV+2, &iLength);
	glEmvStatus.sTLV[0] = iLength/256;
	glEmvStatus.sTLV[1] = iLength%256;

	if (glProcInfo.bIsFirstGAC)
	{
		psTLVData = glEmvStatus.sAppCryptoFirst+2;

		EMVGetTLVData(0x9F26, sBuff, &iLength);
		BuildTLVString(0x9F26, sBuff, iLength, &psTLVData);

		EMVGetTLVData(0x9F27, sBuff, &iLength);
		BuildTLVString(0x9F27, sBuff, iLength, &psTLVData);

		iLength = psTLVData - glEmvStatus.sAppCryptoFirst - 2;
		glEmvStatus.sAppCryptoFirst[0] = iLength/256;
		glEmvStatus.sAppCryptoFirst[1] = iLength%256;
	}
	else
	{
		psTLVData = glEmvStatus.sAppCryptoSecond+2;

		EMVGetTLVData(0x9F26, sBuff, &iLength);
		BuildTLVString(0x9F26, sBuff, iLength, &psTLVData);

		EMVGetTLVData(0x9F27, sBuff, &iLength);
		BuildTLVString(0x9F27, sBuff, iLength, &psTLVData);

		iLength = psTLVData - glEmvStatus.sAppCryptoSecond - 2;
		glEmvStatus.sAppCryptoSecond[0] = iLength/256;
		glEmvStatus.sAppCryptoSecond[1] = iLength%256;
	}

	if( bBeforeOnline )
	{
		EMVGetTLVData(0x95,   glEmvStatus.sgTVROld,  &iLength);
		EMVGetTLVData(0x9B,   glEmvStatus.sgTSIOld,  &iLength);
		glEmvStatus.sgARQCLenOld = 0;
		EMVGetTLVData(0x9F10, glEmvStatus.sgARQCOld, &glEmvStatus.sgARQCLenOld);

		EMVGetTLVData(0x9F0E, glEmvStatus.sgIACDeinal, &iLength);
		EMVGetTLVData(0x9F0F, glEmvStatus.sgIACOnline, &iLength);
		EMVGetTLVData(0x9F0D, glEmvStatus.sgIACDefault, &iLength);

		// search TAC from terminal parameter
		memset(sTermAID, 0, sizeof(sTermAID));
		EMVGetTLVData(0x9F06, sTermAID, &iLength);
		for(iCnt=0; iCnt<MAX_APP_NUM; iCnt++)
		{
			memset(&stEmvApp, 0, sizeof(EMV_APPLIST));
			iRet = EMVGetApp(iCnt, &stEmvApp);
			if( iRet!=EMV_OK )
			{
				continue;
			}
			if( memcmp(sTermAID, stEmvApp.AID, stEmvApp.AidLen)==0 )
			{
				memcpy(glEmvStatus.sgTACDeinal,  stEmvApp.TACDenial,  5);
				memcpy(glEmvStatus.sgTACOnline,  stEmvApp.TACOnline,  5);
				memcpy(glEmvStatus.sgTACDefault, stEmvApp.TACDefault, 5);
				break;
			}
		}
	}
	else
	{
		EMVGetTLVData(0x95,   glEmvStatus.sgTVRNew,  &iLength);
		EMVGetTLVData(0x9B,   glEmvStatus.sgTSINew,  &iLength);
		OsLog(LOG_ERROR, "%s%d: [TVR]: %02X %02X %02X %02X %02X", __FILE__, __LINE__,
		        glEmvStatus.sgTVRNew[0], glEmvStatus.sgTVRNew[1],
		        glEmvStatus.sgTVRNew[2], glEmvStatus.sgTVRNew[3],
		        glEmvStatus.sgTVRNew[4]);
		OsLog(LOG_ERROR, "%s%d: [TSI]: %02X %02X", __FILE__, __LINE__,
		        glEmvStatus.sgTSINew[0], glEmvStatus.sgTSINew[1]);
	}
	SaveEmvStatus();
}

void UpdateEntryModeForOfflinePIN(void)
{
	int		iRet, iLength;
	uchar	sTemp[64];

	if ( !(glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) )
	{
		return;
	}

	memset(sTemp, 0, sizeof(sTemp));
	iRet = EMVGetTLVData(0x9F34, sTemp, &iLength);
	if( iRet==EMV_OK )
	{
		sTemp[0] &= 0x3F;
		if (sTemp[2]==0x02)		// last CVM succeed
		{
			if (sTemp[0]==0x01 ||	// plaintext PIN
				sTemp[0]==0x03 ||	// plaintext PIN and signature
				sTemp[0]==0x04 ||	// enciphered PIN
				sTemp[0]==0x05)	// enciphered PIN and signature
			{
				glProcInfo.stTranLog.uiEntryMode |= MODE_OFF_PIN;
			}
		}
	}
}

// show last EMV status
// Modified by Kim_LinHB 2014-6-8
int ViewTVR_TSI(void)
{
	int		iTemp;
	GUI_PAGELINE stBuff[100];
	unsigned char szHex[1+1];
	int		nLine = 0;
	GUI_PAGE stPage;

	SetCurrTitle(_T("View TVR TSI")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	if( PasswordBank()!=0 )
	{
		return ERR_NO_DISP;
	}

	LoadEmvStatus();

	memset(stBuff, 0, sizeof(stBuff));

	sprintf(stBuff[nLine].szLine,"Before TSI=%02X %02X", glEmvStatus.sgTSIOld[0], glEmvStatus.sgTSIOld[1]);
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	sprintf(stBuff[nLine].szLine,"TVR=%02X %02X %02X %02X %02X",
								glEmvStatus.sgTVROld[0], glEmvStatus.sgTVROld[1], glEmvStatus.sgTVROld[2],
								glEmvStatus.sgTVROld[3], glEmvStatus.sgTVROld[4]);
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	sprintf(stBuff[nLine].szLine, "IssuAppData=");
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	for(iTemp=0; iTemp<glEmvStatus.sgARQCLenOld; iTemp++)
	{	
		sprintf(szHex, "%02X", glEmvStatus.sgARQCOld[iTemp]);
		strcat(stBuff[nLine].szLine, szHex);
		if(0 == iTemp % 5)
			++nLine;
	}
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	sprintf(stBuff[nLine].szLine, "After TSI=%02X %02X", glEmvStatus.sgTSINew[0], glEmvStatus.sgTSINew[1]);
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	sprintf(stBuff[nLine].szLine, "TVR=%02X %02X %02X %02X %02X",
								glEmvStatus.sgTVRNew[0], glEmvStatus.sgTVRNew[1], glEmvStatus.sgTVRNew[2],
								glEmvStatus.sgTVRNew[3], glEmvStatus.sgTVRNew[4]);
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	strcpy(stBuff[nLine].szLine, "TACDenial =");
	PubBcd2Asc0(glEmvStatus.sgTACDeinal, 5, stBuff[nLine].szLine+strlen("TACDenial ="));
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	strcpy(stBuff[nLine].szLine, "TACOnline =");
	PubBcd2Asc0(glEmvStatus.sgTACOnline, 5, stBuff[nLine].szLine+strlen("TACOnline ="));
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	strcpy(stBuff[nLine].szLine, "IACDenial =");
	PubBcd2Asc0(glEmvStatus.sgIACDeinal, 5, stBuff[nLine].szLine+strlen("IACDenial ="));
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	strcpy(stBuff[nLine].szLine, "IACOnline =");
	PubBcd2Asc0(glEmvStatus.sgIACOnline, 5, stBuff[nLine].szLine+strlen("IACOnline ="));
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	strcpy(stBuff[nLine].szLine, "IACDefault =");
	PubBcd2Asc0(glEmvStatus.sgIACDefault, 5, stBuff[nLine].szLine+strlen("IACDefault ="));
	stBuff[nLine++].stLineAttr = gl_stLeftAttr;

	Gui_CreateInfoPage(GetCurrTitle(), gl_stTitleAttr, stBuff, nLine, &stPage);
	
	Gui_ClearScr();
	Gui_ShowInfoPage(&stPage, FALSE, USER_OPER_TIMEOUT);

    if(glEmvStatus.sAppCryptoFirst[1] > 0 ||
       glEmvStatus.sAppCryptoSecond[1] > 0 ||
       glEmvStatus.sTLV[1] > 0) // Added by Kim 20150116 bug612
    {
        Gui_ClearScr();
        if(GUI_OK != Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, _T("PRINT DETAIL?"), gl_stCenterAttr, GUI_BUTTON_YandN, USER_OPER_TIMEOUT, NULL)){
            return 0;
        }

        PrnInit();
        PrnSetNormal();
        PubDebugOutput("FIRST GAC", glEmvStatus.sAppCryptoFirst+2,
                        glEmvStatus.sAppCryptoFirst[1],
                        DEVICE_PRN, TLV_MODE);
        PubDebugOutput("SECOND GAC", glEmvStatus.sAppCryptoSecond+2,
                        glEmvStatus.sAppCryptoSecond[1],
                        DEVICE_PRN, TLV_MODE);
        PubDebugOutput("TRANS TLV", glEmvStatus.sTLV+2,
                        glEmvStatus.sTLV[1],
                        DEVICE_PRN, TLV_MODE);
	}
    return 0;
}

unsigned char  cEMVPiccIsoCommand(unsigned char cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv)
{
	return 0;
}

#endif		// #ifdef ENABLE_EMV

unsigned char cPiccIsoCommand_Entry(uchar cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv)
{
	return PiccIsoCommand(cid, ApduSend, ApduRecv);
}

unsigned char  cPiccIsoCommand_Pboc (unsigned char cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv)
{
	return PiccIsoCommand(cid, ApduSend, ApduRecv);
}

uchar cPiccIsoCommand_Wave(uchar cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv)
{
	return PiccIsoCommand(cid, ApduSend, ApduRecv);
}

unsigned char cPiccIsoCommand_MC(uchar cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv)
{
	return PiccIsoCommand(cid, ApduSend, ApduRecv);
}

int cClssCheckExceptionFile_Pboc(uchar *pucPAN, int nPANLen, uchar *pucPANSeq)
{
	return EMV_OK;
}

int cClssGetUnknowTLVData_Wave(unsigned short usTag, unsigned char *pucData, int nLen)
{
	return EMV_OK;
}

unsigned char cEMVSM2Verify(unsigned char *paucPubkeyIn,unsigned char *paucMsgIn,int nMsglenIn, unsigned char *paucSignIn, int nSignlenIn)
{
	return EMV_OK;
}

unsigned char cEMVSM3(unsigned char *paucMsgIn, int nMsglenIn,unsigned char *paucResultOut)
{
	return EMV_OK;
}

// end of file

