/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2006-? PAX Technology, Inc. All rights reserved.
 * Description:    定义终端参数设置、查询模块
 * Revision History:
 * Date                  Author                 Action
 * 2006.09.12          shengjx                      created
 * ============================================================================
 */

#include "global.h"

// Modified by Kim_LinHB 2014-7-9  Update GUI to new version

/********************** Internal macros declaration ************************/

/********************** Internal structure declaration *********************/

/********************** Internal functions declaration *********************/
static int SetSystemParam(void);
static void SetSystemParamSub(uchar ucPermission);
static int  SetCommType(uchar ucMode);
static void SetSysCommParam(uchar ucPermission);
static int  SetCommDetails(uchar mode, uchar *pucCommType);
static int  SetPabx(void);
static int  SetModemParam(void);
static int  SetTcpIpSharedPara(COMM_CONFIG *pstCommCfg);
static void SetAcqParam(uchar ucPermission);
static int SetNoPrinterPara(ACQUIRER *acq);
static int SetSMS(ACQUIRER *acq);

static int  SetTcpIpParam_S80(TCPIP_PARA *pstParam);

static int  GetHostDNS(const uchar *pszPrompts, uchar bAllowNull, uchar *pszName);
static int  GetIPAddress(const uchar *pszPrompts, uchar bAllowNull, uchar *pszIPAddress);
static uchar IsValidIPAddress(const char *pszIPAddr);
static int  GetIPPort(const uchar *pszPrompts, uchar bAllowNull, uchar *pszPortNo);
static void SetIssuerParam(uchar ucPermission);
static int cDispAcqPama(int index, GUI_OPTIONITEM_DATA *data);
static int  SetAcqTransComm(uchar ucCommType);
static int  SetAcqTransEncrypt(ACQUIRER *acq);
static int  SetAcqTransTelNo(void);
static int SetTel(uchar *pszTelNo, const uchar *pszPromptInfo);
static void SetEdcParam(uchar ucPermission);
static int cSetTermCountry(int index, GUI_OPTIONITEM_DATA *data);
static int cSetTermCurrency(int index, GUI_OPTIONITEM_DATA *data);
static int cSetTermDecimalPosition(int index, GUI_OPTIONITEM_DATA *data);
static int cSetTermIgnoreDigit(int index, GUI_OPTIONITEM_DATA *data);
static int cSetMerchantName(int index, GUI_OPTIONITEM_DATA *data);
static int cSetMerchantAddr(int index, GUI_OPTIONITEM_DATA *data);
static int cSetGetSysTraceNo(int index, GUI_OPTIONITEM_DATA *data);
static int cSetGetSysInvoiceNo(int index, GUI_OPTIONITEM_DATA *data);
static int cSetPEDMode(int index, GUI_OPTIONITEM_DATA *data);
static int cSetAcceptTimeOut(int index, GUI_OPTIONITEM_DATA *data);
static int cSetPrinterType(int index, GUI_OPTIONITEM_DATA *data);
static int cSetNumOfReceipt(int index, GUI_OPTIONITEM_DATA *data);
static int cSetEdcInfoOption(int index, GUI_OPTIONITEM_DATA *data);
static int cSetEdcInfoExtOption(int index, GUI_OPTIONITEM_DATA *data);
static int cSetAcqOption(int index, GUI_OPTIONITEM_DATA *data);
static int cSetAcqName(int index, GUI_OPTIONITEM_DATA *data);
static int cSetTerminalId(int index, GUI_OPTIONITEM_DATA *data);
static int cSetMerchantId(int index, GUI_OPTIONITEM_DATA *data);
static int cSetNII(int index, GUI_OPTIONITEM_DATA *data);
static int cSetBatchNo(int index, GUI_OPTIONITEM_DATA *data);
static int cSetAcqComm(int index, GUI_OPTIONITEM_DATA *data);
static int SetCallInTime(void);
static uchar IsValidTime(const uchar *pszTime);
static int ModifyOptList(uchar *psOption, uchar ucMode, uchar ucPermission);
static int ChangePassword(void);
static int SetSysTime(void);
static int SetEdcLang(void);
static int SetPowerSave(void);
static int TestMagicCard1(void);
static int TestMagicCard2(void);
static int TestMagicCard3(void);
static void TestMagicCard(int iTrackNum);
static int ToolsViewPreTransMsg(void);
static int ShowExchangePack(void);
static int PrnExchangePack(void);
static void DebugNacTxd(uchar ucPortNo, const uchar *psTxdData, ushort uiDataLen);

/********************** Internal variables declaration *********************/

static const GUI_MENUITEM sgFuncMenuItem[] =
{
	{ _T_NOOP("INIT"),	0,FALSE,  DownLoadTMSPara_Manual},
	{ _T_NOOP("VIEW RECORD"),	1,TRUE,  ViewTranList},
	{ _T_NOOP("SETUP"),	2,FALSE,  SetSystemParam},
	{ _T_NOOP("LANGUAGE"),	3,TRUE,  SetEdcLang},
	{ _T_NOOP("LOCK TERM"),	4,TRUE,  LockTerm},
	{ _T_NOOP("VIEW TOTAL"),	5,TRUE,  ViewTotal},
	{ _T_NOOP("ECHO TEST"),	6,FALSE,  TransEchoTest},
#ifdef ENABLE_EMV
	{ _T_NOOP("LAST TSI TVR"),	9,FALSE,  ViewTVR_TSI},
#endif
	{ _T_NOOP("SET TIME"),	10,TRUE,  SetSysTime},
	{ _T_NOOP("PRINT PARA"),	11,FALSE,  PrintParam},
	// { _T_NOOP("VIEW PREAUTH RECORD"),	13,TRUE,  ViewTranList},
	{ _T_NOOP("TXN REVIEW"),	21,TRUE,  ViewSpecList},
	{ _T_NOOP("ADJUST"),	22,FALSE,  TransAdjust},
	{ _T_NOOP("APP UPDATE"),	50,FALSE,  DownLoadTMSPara_Auto},	// hidden, not for public use until confirm.
	{ _T_NOOP("CHECK FONTS"),	60,FALSE,  EnumSysFonts},
	{ _T_NOOP("REPRN SETTLE"),	71,TRUE,  RePrnSettle},
	{ _T_NOOP("REPRINT LAST"),	72,TRUE,  PrnLastTrans},
	{ _T_NOOP("REPRINT BILL"),	73,TRUE,  RePrnSpecTrans},
	{ _T_NOOP("PRINT TOTAL"),	74,TRUE,  PrnTotal},
	{ _T_NOOP("PRINT LOG"),	75,TRUE,  PrnAllList},
	{ _T_NOOP("POWER SAVING"),	81,TRUE,  SetPowerSave},
//	{ _T_NOOP("TEST TRACK1"),	87,FALSE,  TestMagicCard1},
//	{ _T_NOOP("TEST TRACK2"),	88,FALSE,  TestMagicCard2},
//	{ _T_NOOP("TEST TRACK3"),	89,FALSE,  TestMagicCard3},
	{ _T_NOOP("MODIFY PWD"),	90,TRUE,  ChangePassword},
	{ _T_NOOP("DISP VER"),	91,TRUE,  DispVersion},
	{ _T_NOOP("SHOW PACKAGE"), 95,FALSE,  ToolsViewPreTransMsg},
#ifdef ENABLE_EMV
	{ _T_NOOP("PRINT ERR LOG"),	96,FALSE,  PrintEmvErrLog},
#endif
    { _T_NOOP("KEY INJECTION"), 98,FALSE,  DoKeyInjection},
	{ _T_NOOP("CLEAR"),	99,FALSE,  DoClear},
	{ "", -1,FALSE,  NULL},
};

static const GUI_MENUITEM sgInitMenuItem[] =
{
	{ _T_NOOP("INIT"),	0,FALSE,  DownLoadTMSPara_Manual},
//	{ _T_NOOP("SETUP"),	2,FALSE,  SetSystemParam},
	{ _T_NOOP("LANGUAGE"),	3,TRUE,  SetEdcLang},
	{ _T_NOOP("SET TIME"),	10,TRUE,  SetSysTime},
	{ _T_NOOP("CHECK FONTS"),	60,FALSE,  EnumSysFonts},
	{ _T_NOOP("TEST TRACK1"),	87,FALSE,  TestMagicCard1},
	{ _T_NOOP("TEST TRACK2"),	88,FALSE,  TestMagicCard2},
	{ _T_NOOP("TEST TRACK3"),	89,FALSE,  TestMagicCard3},
	{ _T_NOOP("MODIFY PWD"),	90,TRUE,  ChangePassword},
	{ _T_NOOP("DISP VER"),	91,TRUE,  DispVersion},
	{ _T_NOOP("KEY INJECTION"), 98,FALSE,  DoKeyInjection},
	{ _T_NOOP("CLEAR"),	99,FALSE,  DoClear},
	{ "", -1,FALSE,  NULL},
};

const static GUI_MENUITEM sg_stPINPADMenuItem[] = {
    { "1.PCI PED", PED_INT_PCI, TRUE,  NULL},
    { "2.PINPAD", PED_EXT_PP, TRUE,  NULL},
    { "3.EXT PCI PINPAD", PED_EXT_PCI, TRUE,  NULL},
    { "", -1, FALSE,  NULL},
};

/********************** external reference declaration *********************/



/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/
void GetAllSupportFunc(char *pszBuff)
{
	int	ii;

	pszBuff[0] = 0;
	for (ii=0; ii<sizeof(sgFuncMenuItem)/sizeof(sgFuncMenuItem[0]); ii++)
	{
		if (sgFuncMenuItem[ii].szText[0]!=0)
		{
			if (strlen(pszBuff)!=0)
			{
				strcat(pszBuff, ",");
			}
			sprintf(pszBuff+strlen(pszBuff), "%lu", (unsigned long)sgFuncMenuItem[ii].nValue);
		}
	}
}

// 执行指定功能号的函数
// call function with a specific id
void FunctionExe(uchar bUseInitMenu, int iFuncNo)
{
	int			iCnt;
	GUI_MENUITEM	*pstMenu;

	pstMenu = (GUI_MENUITEM *)(bUseInitMenu ? sgInitMenuItem : sgFuncMenuItem);
	for(iCnt=0; pstMenu[iCnt].szText[0]!=0; iCnt++)
	{
	
		if( pstMenu[iCnt].nValue == iFuncNo)
		{
			if( !pstMenu[iCnt].vFunc )
			{
				break;
			}
			pstMenu[iCnt].vFunc();
			return;
		}
	}

	Gui_ClearScr();
	PubBeepErr();
	Gui_ShowMsgBox(NULL, gl_stTitleAttr, _ICON_FAIL, _T("FUNC NUMBER ERR"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL); // Modified by Kim_LinHB 2014-8-6 v1.01.0001 bug493
}

void FunctionMenu(void)
{
	GUI_MENU stMenu;
	GUI_MENUITEM stFuncMenuItems[sizeof(sgFuncMenuItem)/sizeof(sgFuncMenuItem[0])];
	memcpy(stFuncMenuItems, sgFuncMenuItem, sizeof(sgFuncMenuItem));
	AddIndexesForGuiMenu(stFuncMenuItems);
	Gui_BindMenu(_T("FUNCTION:"), gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stFuncMenuItems, &stMenu);
	Gui_ClearScr();
	Gui_ShowMenuList(&stMenu, GUI_MENU_MANUAL_INDEX, USER_OPER_TIMEOUT, NULL);
}

void FunctionInit(void)
{
	int		iFuncNo;
	iFuncNo = FunctionInput();
	if( iFuncNo>=0 )
	{
		FunctionExe(TRUE, iFuncNo);
	}
}

// 设置系统参数
// set system parameters
int SetSystemParam(void)
{
	uchar ucPermission;

#ifdef FUN2_READ_ONLY
	ucPermission = PM_LOW;		// 低权限
#else
	ucPermission = PM_MEDIUM;	// 中等权限
#endif

	SetCurrTitle(_T("TERM SETUP"));
	if( PasswordBank()!=0 )
	{
		return ERR_NO_DISP;
	}

	SetSystemParamSub(ucPermission);
	return 0;
}
		
void SetSystemParamSub(uchar ucPermission)
{
	int iSelected;
	GUI_MENU stMenu;
	GUI_MENUITEM stMenuItem[] = {
		{ _T_NOOP("1.COMM PARA"), 1,TRUE,  NULL},
		{ _T_NOOP("2.VIEW EDC"), 2,TRUE,  NULL},
		{ _T_NOOP("3.VIEW ISSUER"), 3,TRUE,  NULL},
		{ _T_NOOP("4.VIEW ACQUIRER"), 4,TRUE,  NULL},
		{ "", -1,FALSE,  NULL},
	};

    Gui_BindMenu(GetCurrTitle(), gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stMenuItem, &stMenu);
	iSelected = 0;
	while( 1 )
	{
		Gui_ClearScr();
		
		if( GUI_OK != Gui_ShowMenuList(&stMenu, 0, USER_OPER_TIMEOUT, &iSelected))
		{
			return;
		}
		
		if( 1 == iSelected )
		{
			SetSysCommParam(ucPermission);
		}
		else if( 2 == iSelected )
		{
			SetEdcParam(ucPermission);
		}
		else if( 3 == iSelected )
		{
			SetIssuerParam(ucPermission);
		}
		else if( 4 == iSelected )
		{
			SetAcqParam(ucPermission);
		}
	}
}

void SetSystemParamAll(void)
{
	// 最高权限，可以修改所有参数
	// using the highest Permission
	SetSystemParamSub(PM_HIGH);
}

int GetCommName(uchar ucCommType, uchar *pszText)
{
	switch(ucCommType)
	{
	case CT_RS232:
		sprintf((char *)pszText, "COM");
		return 0;
	case CT_MODEM:
		sprintf((char *)pszText, "MODEM");
	    return 0;
	case CT_TCPIP:
		sprintf((char *)pszText, "TCPIP");
	    return 0;
	case CT_WCDMA:
		sprintf((char *)pszText, "WCDMA");
		return 0;
	case CT_CDMA:
		sprintf((char *)pszText, "CDMA");
		return 0;
	case CT_GPRS:
		sprintf((char *)pszText, "GPRS");
		return 0;
	case CT_WIFI:
		sprintf((char *)pszText, "WIFI");
	    return 0;
	case CT_DEMO:
		sprintf((char *)pszText, "DEMO");
	    return 0;	
	case CT_BLTH:
		sprintf((char *)pszText, "BLUETOOTH");
	    return 0;
	default:
		sprintf((char *)pszText, "DISABLED");
	    return -1;
	}
}

// ucForAcq : set custom comm type for ACQ
int SetCommType(uchar ucMode)
{
	int		iRet, iSelected;
	char	szTitle[32];
	uchar	*pucCommType;
	GUI_MENU	stSmDownMode;
	// Modified by Kim_LinHB 2014-8-6 v1.01.0001 bug492  remove static
	GUI_MENUITEM stDefCommMenu[] =
	{
		{ "DISABLE",	CT_NONE,TRUE, 	NULL}, //0
		{ "MODEM",	CT_MODEM,TRUE, 	NULL}, //1
		{ "TCPIP",	CT_TCPIP,TRUE, 	NULL}, //2
		{ "GPRS",	CT_GPRS,TRUE, 	NULL}, //3
		{ "CDMA",	CT_CDMA,TRUE, 	NULL}, //4
		{ "WIFI",	CT_WIFI, TRUE, 	NULL}, //5
		{ "RS232",	CT_RS232,TRUE, 	NULL}, //6
		{ "BLUETOOTH",	CT_BLTH,FALSE, 	NULL}, //7
		{ "WCDMA",	CT_WCDMA,TRUE, 	NULL}, //8
		{ "DEMO ONLY",	CT_DEMO,TRUE, 	NULL}, //9
		{ "", -1,FALSE,  NULL},
	};// This menu does not provide translation
	GUI_MENUITEM stCommMenu[20];
	int iMenuItemNum = 0;

	//--------------------------------------------------
	memset(&stSmDownMode, 0, sizeof(stSmDownMode));

	if (ucMode!=0)
	{
	    memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[0], sizeof(GUI_MENUITEM));
	    sprintf(stCommMenu[iMenuItemNum].szText, "%d.%s", iMenuItemNum+1, stDefCommMenu[0].szText);
	    ++iMenuItemNum;
	}
	if (!(ChkHardware(HWCFG_MODEM, HW_NONE) ||
		(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_MODEM)))
	{
	    if(stDefCommMenu[1].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[1], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%d.%s", iMenuItemNum+1, stDefCommMenu[1].szText);
            ++iMenuItemNum;
        }
	}
	if (!(ChkHardware(HWCFG_LAN, HW_NONE) ||									// If no LAN module
		(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_TCPIP)))	// and now is selecting 2nd comm && 1st comm already selected LAN
	{
	    if(stDefCommMenu[2].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[2], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%d.%s", iMenuItemNum+1, stDefCommMenu[2].szText);
            ++iMenuItemNum;
        }
	}
	
	if (!(ChkHardware(HWCFG_GPRS, HW_NONE) ||
		(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_GPRS)))
	{
	    if(stDefCommMenu[3].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[3], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%d.%s", iMenuItemNum+1, stDefCommMenu[3].szText);
            ++iMenuItemNum;
        }
	}
	if (!(ChkHardware(HWCFG_CDMA, HW_NONE) ||
		(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_CDMA)))
	{
	    if(stDefCommMenu[4].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[4], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%d.%s", iMenuItemNum+1, stDefCommMenu[4].szText);
            ++iMenuItemNum;
        }
	}
	if (!(ChkHardware(HWCFG_WIFI, HW_NONE) ||
		(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_WIFI)))
	{
	    if(stDefCommMenu[5].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[5], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%d.%s", iMenuItemNum+1, stDefCommMenu[5].szText);
            ++iMenuItemNum;
        }
	}
	if (!(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_RS232))
	{
	    if(stDefCommMenu[6].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[6], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%d.%s", iMenuItemNum+1, stDefCommMenu[6].szText);
            ++iMenuItemNum;
        }
	}
	if(!(ChkHardware(HWCFG_BLTH, HW_NONE) ||
		(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_BLTH)))
	{
	    if(stDefCommMenu[7].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[7], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%d.%s", iMenuItemNum+1, stDefCommMenu[7].szText);
            ++iMenuItemNum;
        }
	}
	if (!(ChkHardware(HWCFG_WCDMA, HW_NONE) ||
		(ucMode!=0 && glSysParam.stTxnCommCfg.ucCommType==CT_WCDMA)))
	{
	    if(stDefCommMenu[8].bVisible)
        {
	        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[8], sizeof(GUI_MENUITEM));
            sprintf(stCommMenu[iMenuItemNum].szText, "%d.%s", iMenuItemNum+1, stDefCommMenu[8].szText);
            ++iMenuItemNum;
        }
	}
    if (ucMode==0)
    {
        // Only primary comm type can be "demo"
        memcpy(&stCommMenu[iMenuItemNum], &stDefCommMenu[9], sizeof(GUI_MENUITEM));
        sprintf(stCommMenu[iMenuItemNum].szText, "%d.%s", iMenuItemNum+1, stDefCommMenu[9].szText);
        ++iMenuItemNum;
    }

    stCommMenu[iMenuItemNum].szText[0] = 0;

	memset(szTitle, 0, sizeof(szTitle));
	if (ucMode==0)
	{
		pucCommType = &glSysParam.stTxnCommCfg.ucCommType;
		strcpy(szTitle, "1st:");
	}
	else
	{
		pucCommType = &glSysParam.stTxnCommCfg.ucCommTypeBak;
		strcpy(szTitle, "2nd:");
	}

	GetCommName(*pucCommType, szTitle+strlen(szTitle));

	Gui_BindMenu(szTitle, gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stCommMenu, &stSmDownMode);
	Gui_ClearScr();
	iSelected = 0;
	iRet = Gui_ShowMenuList(&stSmDownMode, 0, USER_OPER_TIMEOUT, &iSelected);
	if (iRet != GUI_OK)
	{
		return ERR_USERCANCEL;
	}

	if (!ChkIfBatchEmpty() && (*pucCommType!=(uchar)iSelected))
	{
		CommOnHook(TRUE); // Added by Kim_LinHB 2014-08-18 v1.01.0004
		// Not allow to switch into/off demo mode unless batch empty
		if ((*pucCommType==CT_DEMO) || ((uchar)iSelected==CT_DEMO))
		{
			Gui_ClearScr();
			PubBeepErr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_WARNING, _T("PLS SETTLE BATCH"), gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL);
			return ERR_USERCANCEL;
		}
	}

	if((*pucCommType!=(uchar)iSelected))
	{
	    UnReg_CommIcon(*pucCommType);
	    Reg_CommIcon(iSelected, ucMode);
	}
	if(pucCommType == &glSysParam.stTxnCommCfg.ucCommType &&
	   iSelected == glSysParam.stTxnCommCfg.ucCommTypeBak)
	{
	    UnReg_CommIcon(glSysParam.stTxnCommCfg.ucCommTypeBak);
	}

	*pucCommType = (uchar)iSelected;
	return 0;
}

// 设置通讯参数
// set communication parameters
void SetSysCommParam(uchar ucPermission)
{
	SetCurrTitle("SETUP COMM");
	Gui_ClearScr();
	while (1)
	{
		if (SetCommType(0)!=0)
		{
			break;
		}

		if (SetCommDetails(0, &glSysParam.stTxnCommCfg.ucCommType))
		{
			break;
		}

		if (ChkIfTrainMode())	// if demo mode, no need to set second one
		{
			break;
		}

		if (SetCommType(1)!=0)
		{
			break;
		}

		if (SetCommDetails(1, &glSysParam.stTxnCommCfg.ucCommTypeBak))
		{
			break;
		}

		break;
	}
	SaveSysParam();
}

int SetCommDetails(uchar mode, uchar *pucCommType)
{
	uchar	szDispBuff[32];
	int		iRet;

	sprintf((char *)szDispBuff, "SETUP ");
	GetCommName(*pucCommType, szDispBuff+strlen((char *)szDispBuff));
	SetCurrTitle(szDispBuff);

	iRet = 0;
	switch( *pucCommType )
	{
	case CT_RS232:
	    iRet = SetRs232Param(&glSysParam._TxnRS232Para);
		break;
		
	 case CT_BLTH:
		iRet = SetBTParam(&glSysParam._TxnBlueToothPara);
		if(iRet != 0)
			break;
		SyncBTParam(&glSysParam._TmsBlueToothPara, &glSysParam._TxnBlueToothPara);
		CommOnHook(TRUE);
		DispWait();
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		break;
		
	case CT_WIFI:
		iRet = SetWiFiApp(&glSysParam._TxnWifiPara);
		if(iRet != 0)
		{
			DispWifiErrorMsg(iRet);
			break;
		}
		DispWait();
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		if(iRet != 0){
			DispWifiErrorMsg(iRet);
			break;
		}
		SetTcpIpSharedPara(&glSysParam.stTxnCommCfg);
		SyncWifiParam(&glSysParam._TmsWifiPara, &glSysParam._TxnWifiPara);
	    break;

	case CT_MODEM:
		SetModemParam();
		break;

	case CT_TCPIP:
		SetTcpIpSharedPara(&glSysParam.stTxnCommCfg);
		SetTcpIpParam(&glSysParam._TxnTcpIpPara);
		SyncTcpIpParam(&glSysParam._TmsTcpIpPara, &glSysParam._TxnTcpIpPara);
		DispWait();
		CommInitModule(&glSysParam.stTxnCommCfg);
	    break;

	case CT_GPRS:
	case CT_CDMA:
	case CT_WCDMA:
		SetTcpIpSharedPara(&glSysParam.stTxnCommCfg);
		SetWirelessParam(&glSysParam._TxnWirlessPara);
		SyncWirelessParam(&glSysParam._TmsWirlessPara, &glSysParam._TxnWirlessPara);
		CommOnHook(TRUE);
		DispWait();
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		break;

	case CT_DEMO:
	default:
	    break;
	}

	return iRet;
}

// 输入PABX
// enter PABX
int SetPabx(void)
{
	GUI_INPUTBOX_ATTR stInputBoxAttr;
	LoadNum_SymKeyBoard();

	memset(&stInputBoxAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputBoxAttr.nMinLen = 0;
	stInputBoxAttr.nMaxLen = 10;
	stInputBoxAttr.bEchoMode = 1;

	Gui_ClearScr();
	if( GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, "MODIFY PABX", gl_stLeftAttr, 
		glSysParam.stEdcInfo.szPabx, gl_stRightAttr, &stInputBoxAttr, USER_OPER_TIMEOUT)){
		return ERR_USERCANCEL;
	}

	return 0;
}

int SetRs232Param(RS232_PARA *rs232)
{
    uchar   ucCurBaud;

    GUI_MENU stBaudRateMenu;
    GUI_MENUITEM stBaudRateMenuItem[] = {
        { "1.9600", 0,TRUE,  NULL},
        { "2.38400", 1,TRUE,  NULL},
        { "3.57600", 2,TRUE,  NULL},
        { "4.115200", 3,TRUE,  NULL},
        { "", -1,FALSE,  NULL},
    };
    int iSelected = 0;

    int i;
     //---------------------------------------------------
    for(i = 0; i < sizeof(stBaudRateMenuItem)/ sizeof(GUI_MENUITEM); ++i)
    {
        if(0 == memcmp(stBaudRateMenuItem[i].szText+2, rs232->szAttr, strlen(stBaudRateMenuItem[i].szText) - 2))
        {
            ucCurBaud = i;
            iSelected = ucCurBaud;
            break;
        }
    }

    Gui_BindMenu("BAUD RATE:", gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stBaudRateMenuItem, &stBaudRateMenu);
    Gui_ClearScr();
    if(GUI_OK == Gui_ShowMenuList(&stBaudRateMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, &iSelected))
    {
        char szAttr[21] = {0};
        sprintf(szAttr, "%s%s", stBaudRateMenuItem[iSelected].szText + 2, strchr(rs232->szAttr, ','));
        strcpy(rs232->szAttr, szAttr);
    }
    else
    {
        return ERR_USERCANCEL;
    }
    return 0;
}

// 修改Modem参数
// set Modem parameters
int SetModemParam(void)
{
	uchar	szPrompt[30], szBuff[50], ucCurBaud, ucTemp;

	GUI_MENU stBaudRateMenu;
	GUI_MENUITEM stBaudRateMenuItem[] = {
		{ "1.1200", 0,TRUE,  NULL},
		{ "2.2400", 1,TRUE,  NULL},
		{ "3.9600", 2,TRUE,  NULL},
		{ "4.14400", 3,TRUE,  NULL},
		{ "", -1,FALSE,  NULL},
	};
	int iSelected = 0;

	GUI_INPUTBOX_ATTR stInputAttr;

	if( SetPabx()!=0 )
	{
		return ERR_USERCANCEL;
	}

	LoadNumKeyBoard();

	int iValue = 0;
    iValue = glSysParam.stEdcInfo.bPreDial;
   //---------------------------------------------------
    Gui_ClearScr();
    Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, "PRE DIAL", gl_stCenterAttr,
            "ON", TRUE, "OFF", FALSE, USER_OPER_TIMEOUT, &iValue);
    glSysParam.stEdcInfo.bPreDial = iValue;

    //---------------------------------------------------
	iValue = glSysParam._TxnModemPara.DP;
    Gui_ClearScr();
    Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, "DIAL MODE", gl_stCenterAttr,
        "DTMF", 0, "PULSE", 1, USER_OPER_TIMEOUT, &iValue);
    glSysParam._TxnModemPara.DP = iValue;

    //---------------------------------------------------
    iValue = glSysParam._TxnModemPara.CHDT;
    Gui_ClearScr();
    Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, "DIAL TONE", gl_stCenterAttr,
        "DETECT", 0, "IGNORE", 1, USER_OPER_TIMEOUT, &iValue);
    glSysParam._TxnModemPara.CHDT = iValue;

    //---------------------------------------------------
	memset(&stInputAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 2;
	stInputAttr.bEchoMode = 1;

	sprintf((char *)szPrompt, "DIAL WAIT:");
	sprintf((char *)szBuff, "OLD:%u(*100ms)", (uint)glSysParam._TxnModemPara.DT1);

	Gui_ClearScr();
	if( GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stLeftAttr,
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}
	glSysParam._TxnModemPara.DT1 = (uchar)atoi((char *)szBuff);

    //---------------------------------------------------
	memset(&stInputAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 2;
	stInputAttr.bEchoMode = 1;

	sprintf((char *)szPrompt, "PABX PAUSE:");
	sprintf((char *)szBuff, "OLD:%u(*100ms)", (uint)glSysParam._TxnModemPara.DT2);

	Gui_ClearScr();
	if( GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stLeftAttr,
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}
	glSysParam._TxnModemPara.DT2 = (uchar)atoi((char *)szBuff);

    //---------------------------------------------------
	memset(&stInputAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 3;
	stInputAttr.bEchoMode = 1;

	sprintf((char *)szPrompt, "ONE DTMF HOLD:");
	sprintf((char *)szBuff, "OLD:%u(*1ms)", (uint)glSysParam._TxnModemPara.HT);

	Gui_ClearScr();
	if( GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stLeftAttr,
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}
	glSysParam._TxnModemPara.HT = (uchar)atoi((char *)szBuff);
  

    //---------------------------------------------------
	memset(&stInputAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 3;
	stInputAttr.bEchoMode = 1;

	sprintf((char *)szPrompt, "DTMF CODE SPACE:");
	sprintf((char *)szBuff, "OLD:%u(*10ms)", (uint)glSysParam._TxnModemPara.WT);

	Gui_ClearScr();
	if( GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stLeftAttr,
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}
	glSysParam._TxnModemPara.WT = (uchar)atoi((char *)szBuff);

    //---------------------------------------------------
	memset(&stInputAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 3;
	stInputAttr.bEchoMode = 1;

	sprintf((char *)szPrompt, "SIGNAL LEVEL:");
	sprintf((char *)szBuff, "OLD:%u(0, 1~15)", (uint)glSysParam._TxnPSTNPara.ucSignalLevel);
    while (1)
    {
		Gui_ClearScr();
		if( GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stLeftAttr,
			szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}
        ucTemp = (uchar)atoi((char *)szBuff);
        if (ucTemp<16)
        {
	        glSysParam._TxnPSTNPara.ucSignalLevel = ucTemp;
            break;
        }
    }

    //---------------------------------------------------
	ucCurBaud = (glSysParam._TxnModemPara.SSETUP>>5) & 0x03;
	iSelected = ucCurBaud;

	Gui_BindMenu("BAUD RATE:", gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stBaudRateMenuItem, &stBaudRateMenu);
	Gui_ClearScr();
	if(GUI_OK == Gui_ShowMenuList(&stBaudRateMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, &iSelected))
	{
		ucCurBaud = (unsigned char)(iSelected % 0xFF);
		glSysParam._TxnModemPara.SSETUP &= 0x9F;	// 1001 1111
		glSysParam._TxnModemPara.SSETUP |= (ucCurBaud<<5);
	}
	else
	{
		return ERR_USERCANCEL;
	}

	return 0;
}

int GetIpLocalSettings(void *pstParam)
{
	int		iRet;
	TCPIP_PARA *pstTcpipPara;

	pstTcpipPara = (TCPIP_PARA *)pstParam;

	iRet = GetIPAddress((uchar *)"LOCAL IP", TRUE, pstTcpipPara->szLocalIP);
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetIPAddress((uchar *)"IP MASK", TRUE, pstTcpipPara->szNetMask);
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetIPAddress((uchar *)"GATEWAY IP", TRUE, pstTcpipPara->szGatewayIP);
	if( iRet!=0 )
	{
		return iRet;
	}
	
	iRet = GetIPAddress((uchar *)"DNS", TRUE, pstTcpipPara->szDNSIP);
	if( iRet!=0 )
	{
		return iRet;
	}

	return 0;
}

int GetRemoteIp(const uchar *pszHalfText, uchar bAllowHostName, uchar bAllowNull, void *pstIPAddr)
{
	int		iRet;
	IP_ADDR	*pstIp;
	uchar	szBuff[51];

	pstIp = (IP_ADDR *)pstIPAddr;

	if(bAllowHostName)
	{
        sprintf((char *)szBuff, "%s Host", pszHalfText);
        iRet = GetHostDNS(szBuff, bAllowNull, pstIp->szIP);
        if( iRet!=0 )
        {
            return iRet;
        }
	}
	else
    {
        sprintf((char *)szBuff, "%s IP", pszHalfText);
        iRet = GetIPAddress(szBuff, bAllowNull, pstIp->szIP);
        if( iRet!=0 )
        {
            return iRet;
        }
    }

	sprintf((char *)szBuff, "%s PORT", pszHalfText);
	iRet = GetIPPort(szBuff, bAllowNull, pstIp->szPort);
	if( iRet<0 )
	{
		return iRet;
	}

	return 0;
}

int ChkIfValidIp(const uchar *pszIP)
{
	return ((pszIP[0]!=0) && (IsValidIPAddress(pszIP)));
}

int ChkIfValidPort(const uchar *pszPort)
{
	return ((pszPort[0]!=0) &&
			(atol((uchar *)pszPort)>0) &&
			(atol((uchar *)pszPort)<65536));
}

int SetTcpIpSharedPara(COMM_CONFIG *pstCommCfg)
{
	int	iSel = pstCommCfg->ucTCPClass_BCDHeader;

	Gui_ClearScr();
	if(GUI_OK != Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, "TCP LENGTH", gl_stCenterAttr, 
		_T("BCD"), 1, _T("HEX"), 0, USER_OPER_TIMEOUT, &iSel))
	{
		return -1;
	}

	if(1 == iSel)
	{
		pstCommCfg->ucTCPClass_BCDHeader = TRUE;
	}
	else
	{
		pstCommCfg->ucTCPClass_BCDHeader = FALSE;
	}
	return 0;
}

// 设置TCP/IP参数
// set TCP/IP parameters
int SetTcpIpParam(void *pstParam)
{
	int		iRet;

	// !!!! 需要应用到开机步骤
    iRet = SetTcpIpParam_S80((TCPIP_PARA *)pstParam);
	return iRet;
}

void SyncTcpIpParam(void *pstDst, const void *pstSrc)
{
	((TCPIP_PARA *)pstDst)->ucDhcp = ((TCPIP_PARA *)pstSrc)->ucDhcp;
	strcpy((char *)(((TCPIP_PARA *)pstDst)->szLocalIP),   (char *)(((TCPIP_PARA *)pstSrc)->szLocalIP));
	strcpy((char *)(((TCPIP_PARA *)pstDst)->szGatewayIP), (char *)(((TCPIP_PARA *)pstSrc)->szGatewayIP));
	strcpy((char *)(((TCPIP_PARA *)pstDst)->szNetMask),   (char *)(((TCPIP_PARA *)pstSrc)->szNetMask));
	strcpy((char *)(((TCPIP_PARA *)pstDst)->szDNSIP),     (char *)(((TCPIP_PARA *)pstSrc)->szDNSIP));
}

// Modified by Kim_LinHB 2014-5-31
// Added by Kim_LinHB 2014-5-31
int SetTcpIpParam_S80(TCPIP_PARA *pstParam)
{
	int		iRet;
	int iSelected = 0;
	uchar	szDispBuff[100];
	long	lTcpState;

	iRet = DhcpCheck();
	if (iRet==0)
	{
		sprintf((char *)szDispBuff, "DHCP: OK");
		iSelected = 1;
	}
	else
	{
		sprintf((char *)szDispBuff, "DHCP: STOPPED");
		iSelected = 0;
	}

	Gui_ClearScr();
	if(GUI_OK == Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, szDispBuff, gl_stCenterAttr,
		_T("START"), 1, _T("STOP"), 0, USER_OPER_TIMEOUT, &iSelected))
	{
		if(1 == iSelected)
		{
			pstParam->ucDhcp = 1;

			Gui_ClearScr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, _T("Getting IP..."), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
			if (SxxDhcpStart(FALSE, 30)==0)
			{
			    GUI_PAGE stPage;
			    GUI_PAGELINE stPageLines[4];
			    int iLines = 0;
				iRet = EthGet(pstParam->szLocalIP, pstParam->szNetMask, pstParam->szGatewayIP, pstParam->szDNSIP, &lTcpState);

				sprintf(stPageLines[iLines].szLine, "%s:%s", "LOCAL IP", pstParam->szLocalIP);
				stPageLines[iLines].stLineAttr.eAlign = GUI_ALIGN_LEFT;
				stPageLines[iLines].stLineAttr.ucFontSize = 24;
				stPageLines[iLines].stLineAttr.eStyle = GUI_FONT_STD;
				++iLines;

				sprintf(stPageLines[iLines].szLine, "%s:%s", "IP MASK", pstParam->szNetMask);
                stPageLines[iLines].stLineAttr.eAlign = GUI_ALIGN_LEFT;
                stPageLines[iLines].stLineAttr.ucFontSize = 24;
                stPageLines[iLines].stLineAttr.eStyle = GUI_FONT_STD;
                ++iLines;

				sprintf(stPageLines[iLines].szLine, "%s:%s", "GATEWAY IP", pstParam->szGatewayIP);
                stPageLines[iLines].stLineAttr.eAlign = GUI_ALIGN_LEFT;
                stPageLines[iLines].stLineAttr.ucFontSize = 24;
                stPageLines[iLines].stLineAttr.eStyle = GUI_FONT_STD;
                ++iLines;

				sprintf(stPageLines[iLines].szLine, "%s:%s", "DNS", pstParam->szDNSIP);
                stPageLines[iLines].stLineAttr.eAlign = GUI_ALIGN_LEFT;
                stPageLines[iLines].stLineAttr.ucFontSize = 24;
                stPageLines[iLines].stLineAttr.eStyle = GUI_FONT_STD;
                ++iLines;

                Gui_CreateInfoPage(GetCurrTitle(), gl_stTitleAttr, stPageLines, iLines, &stPage);
                Gui_ShowInfoPage(&stPage, 0, USER_OPER_TIMEOUT);
				return 0;
			}
		}
		else
		{
			pstParam->ucDhcp = 0;
		}
	}
	else
	{
		return -1;
	}
	

	// Manual setup
	if (iRet == 0)
	{
		DhcpStop();
	}

	if (pstParam->ucDhcp)
	{
		iRet = EthGet(pstParam->szLocalIP, pstParam->szNetMask, pstParam->szGatewayIP, pstParam->szDNSIP, &lTcpState);
	}

	iRet = GetIpLocalSettings(pstParam);
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = EthSet(pstParam->szLocalIP, pstParam->szNetMask, pstParam->szGatewayIP, pstParam->szDNSIP);
	if (iRet < 0)
	{
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, "SET STATIC IP\nFAILED.", gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL);
		return -1;
	}

	return 0;
}

static int  GetHostDNS(const uchar *pszPrompts, uchar bAllowNull, uchar *pszName)
{
    uchar   szTemp[50 + 1];
    GUI_INPUTBOX_ATTR stInputBoxAttr;
    LoadMixKeyBoard();
    memset(&stInputBoxAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
    stInputBoxAttr.bEchoMode = 0;
    if(bAllowNull)
        stInputBoxAttr.nMinLen = 0;
    else
        stInputBoxAttr.nMinLen = 1;
    stInputBoxAttr.nMaxLen = sizeof(szTemp) - 1;

    sprintf((char *)szTemp, "%.50s", pszName);
    Gui_ClearScr();
    if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, pszPrompts, gl_stLeftAttr,
      szTemp, gl_stRightAttr, &stInputBoxAttr, USER_OPER_TIMEOUT))
    {
      return ERR_USERCANCEL;
    }

    if( bAllowNull && szTemp[0]==0 )
    {
      *pszName = 0;
    }
    else{
      sprintf((char *)pszName, "%.50s", szTemp);
    }
    return 0;
}

// 输入IP地址
// get Ip address
int GetIPAddress(const uchar *pszPrompts, uchar bAllowNull, uchar *pszIPAddress)
{
	uchar	szTemp[20];
	GUI_INPUTBOX_ATTR stInputBoxAttr;

	LoadNum_SymKeyBoard();

	memset(&stInputBoxAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
    stInputBoxAttr.nMinLen = (bAllowNull ? 0 : 1);
    stInputBoxAttr.nMaxLen = 15;
	stInputBoxAttr.bEchoMode = 1;

	sprintf((char *)szTemp, "%.15s", pszIPAddress);

	while(1){
        Gui_ClearScr();
        if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, pszPrompts, gl_stLeftAttr,
            szTemp, gl_stRightAttr, &stInputBoxAttr, USER_OPER_TIMEOUT))
        {
            return ERR_USERCANCEL;
        }

        if( bAllowNull && szTemp[0]==0 )
        {
            *pszIPAddress = 0;
        }
        else{
            if(!IsValidIPAddress(szTemp)){
                OsBeep(4, 50);
                continue;
            }
            sprintf((char *)pszIPAddress, "%.15s", szTemp);
        }
        return 0;
	}
	return 0;
}

// 检查IP地址
// verify the format of IP address
uchar IsValidIPAddress(const char *pszIPAddr)
{
	int		i;
	char	*p, *q, szBuf[5+1], szIp[16 + 1];

	sprintf(szIp, "%.*s",sizeof(szIp), pszIPAddr); // Modified by Kim_LinHB 2014-8-11 bug507

	PubTrimStr(szIp);
	if( *szIp==0 )
	{
		return FALSE;
	}

	p = strchr(szIp, ' ');
	if( p!=NULL )
	{
		return FALSE;
	}
	if( strlen(szIp)>15 )
	{
		return FALSE;
	}

	// 1st --- 3rd  part
	for(q=szIp, i=0; i<3; i++)
	{
		p = strchr(q, '.');
		if( p==NULL || p==q || p-q>3 )
		{
			return FALSE;
		}
		sprintf(szBuf, "%.*s", (int)(p-q), q);
		if( !IsNumStr(szBuf) || atoi(szBuf)>255 )
		{
			return FALSE;
		}
		q = p + 1;
	}

	// final part
	p = strchr((char *)q, '.');
	if( p!=NULL || !IsNumStr(q) || strlen(q)==0 || strlen(q)>3 || atoi(q)>255 )
	{
		return FALSE;
	}

	return TRUE;
}

// 输入端口
// get IP port
int GetIPPort(const uchar *pszPrompts, uchar bAllowNull, uchar *pszPortNo)
{
	int		iTemp;
	uchar	szTemp[15];

	GUI_INPUTBOX_ATTR stInputBoxAttr;
	LoadNumKeyBoard();

	memset(&stInputBoxAttr, 0, sizeof(GUI_INPUTBOX_ATTR));
	stInputBoxAttr.nMinLen = (bAllowNull ? 0 : 1);
	stInputBoxAttr.nMaxLen = 5;
	stInputBoxAttr.bEchoMode = 1;

	while( 1 )
	{
		sprintf((char *)szTemp, "%.5s", pszPortNo);
		Gui_ClearScr();
		
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, pszPrompts, gl_stLeftAttr, 
			szTemp, gl_stRightAttr, &stInputBoxAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}

		iTemp = atoi((char *)szTemp);
		if( iTemp>0 && iTemp<65535 )
		{
			sprintf((char *)pszPortNo, "%.5s", szTemp);
			break;
		}
		if (bAllowNull)
		{
			pszPortNo[0] = 0;
			break;
		}

		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("INV PORT #"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 4, NULL);
	}

	return 0;
}

// 选择发卡行并修改参数
// select issuer and set its parameters
void SetIssuerParam(uchar ucPermission)
{
	int		iRet;
	uchar	ucIndex, szBuff[32], szBuff2[32];
#ifdef ENABLE_EMV
	int		iCnt;
	ulong	ulTemp;
	EMV_APPLIST	stEmvApp;
#endif

	GUI_INPUTBOX_ATTR stInputAttr;
	LoadNumKeyBoard();

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.bEchoMode = 1;

	while (1)
	{
		iRet = SelectIssuer(&ucIndex);
		if( iRet!=0 )
		{
			return;
		}

		sprintf((char *)szBuff, "ISSUER: %-8.8s", (char *)glCurIssuer.szName);
		SetCurrTitle(szBuff);

        if (ucPermission==PM_HIGH){
            int iSelected;
            GUI_MENU stMenu;
            GUI_MENUITEM stMenuItem[] = {
                { _T_NOOP("INFO"), 1,TRUE,  NULL},
                { _T_NOOP("NON-EMV FLOOR LIMIT"), 2,TRUE,  NULL},
                { _T_NOOP("EMV FLOOR LIMIT"), 3,TRUE,  NULL},
                { _T_NOOP("ADJUST PERCENT"), 4,TRUE,  NULL},
                { _T_NOOP("REFERRAL TEL"), 5,TRUE,  NULL},
                { "", -1,FALSE,  NULL},
            };

            AddIndexesForGuiMenu(stMenuItem);
            Gui_BindMenu(GetCurrTitle(), gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stMenuItem, &stMenu);
            iSelected = 0;

            while(1){
                Gui_ClearScr();

               if( GUI_OK != Gui_ShowMenuList(&stMenu, 0, USER_OPER_TIMEOUT, &iSelected))
               {
                   break;
               }

               switch(iSelected){
                   case 1: ModifyOptList(glSysParam.stIssuerList[ucIndex].sOption, 'I', ucPermission); break;
                   case 2:
                       stInputAttr.nMinLen = 1;
                       stInputAttr.nMaxLen = 2;
                       sprintf((char *)szBuff2, "%lu", glSysParam.stIssuerList[ucIndex].ulFloorLimit);
                       Gui_ClearScr();
                       iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("NON-EMV FLOOR LIMIT"), gl_stLeftAttr,
                               szBuff2, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
                       if( GUI_OK == iRet )
                       {
                           glSysParam.stIssuerList[ucIndex].ulFloorLimit = (ulong)atoi(szBuff2);
                       }
                       break;
                   case 3:
#ifdef ENABLE_EMV
                        stInputAttr.nMinLen = 1;
                        stInputAttr.nMaxLen = 4;
                        memset(szBuff2, 0, sizeof(szBuff2));
                        Gui_ClearScr();
                        iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("EMV FLOOR LIMIT"), gl_stLeftAttr,
                            szBuff2, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
                        if( GUI_OK == iRet )
                        {
                            ulTemp = (ulong)atoi(szBuff2);
                            for (iCnt=0;
                                iCnt<glSysParam.stEdcInfo.stLocalCurrency.ucDecimal+glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit;
                                iCnt++)
                            {
                                ulTemp *= 10;
                            }

                            for(iCnt=0; iCnt<MAX_APP_NUM; iCnt++)
                            {
                                memset(&stEmvApp, 0, sizeof(EMV_APPLIST));
                                iRet = EMVGetApp(iCnt, &stEmvApp);
                                if( iRet!=EMV_OK )
                                {
                                    continue;
                                }
                                stEmvApp.FloorLimit = ulTemp;
                                iRet = EMVAddApp(&stEmvApp);
                            }
                        }
#else
                        Gui_ClearScr();
                        PubBeepErr();
                        Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _T("UNSUPPORTED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
#endif
                       break;
                   case 4:
                       if (ChkEdcOption(EDC_TIP_PROCESS))
                       {
                           while (1)
                           {
                               stInputAttr.nMinLen = 1;
                               stInputAttr.nMaxLen = 6;
                               sprintf((char *)szBuff2, "%d", (int)(glSysParam.stIssuerList[ucIndex].ucAdjustPercent%100));
                               Gui_ClearScr();
                               iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("ADJUST PERCENT"), gl_stLeftAttr,
                                   szBuff2, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
                               if (atoi(szBuff2)>50)
                               {
                                   continue;
                               }
                               glSysParam.stIssuerList[ucIndex].ucAdjustPercent = (uchar)atoi(szBuff2);
                               break;
                           }
                       }
                       else{
                           Gui_ClearScr();
                           PubBeepErr();
                           Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("UNSUPPORTED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
                       }
                       break;
                   case 5:
                       if (ChkEdcOption(EDC_REFERRAL_DIAL))
                       {
                           stInputAttr.nMinLen = 1;
                           stInputAttr.nMaxLen = 12;
                           sprintf((char *)szBuff2, "%.12s", (char *)glSysParam.stIssuerList[ucIndex].szRefTelNo);
                           Gui_ClearScr();
                           iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("REFERRAL TEL"), gl_stLeftAttr,
                               szBuff2, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
                           if( GUI_OK == iRet )
                           {
                               sprintf((char *)glSysParam.stIssuerList[ucIndex].szRefTelNo, "%.12s", (char *)szBuff2);
                           }
                       }
                       else{
                          Gui_ClearScr();
                          PubBeepErr();
                          Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("UNSUPPORTED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
                       }
                       break;
               }
               SaveSysParam();
            }
        }
        else {
            ModifyOptList(glSysParam.stIssuerList[ucIndex].sOption, 'I', ucPermission);
        }

		if( glSysParam.ucIssuerNum<2 )
		{
			break;
		}
	}
}

// 设置收单行参数
// set acquirer's parameters
void SetAcqParam(uchar ucPermission)
{
	int		iRet;
	uchar	szTitle[16+1];

	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.bEchoMode = 1;

	while (1)
	{
		sprintf((char *)szTitle, "%s", glSysParam.ucAcqNum>9 ? "SELECT ACQ:" : "SELECT ACQUIRER");
		iRet = SelectAcq(FALSE, szTitle, NULL);
		if( iRet!=0 )
		{
			return;
		}

        sprintf((char *)szTitle, "ACQ: %-10.10s ", (char *)glCurAcq.szName);
		SetCurrTitle(szTitle);

		int iCnt = 0;
	    GUI_OPTION stOption;
	    GUI_OPTIONITEM stOptItem[50];

        int index = 1;
        stOptItem[iCnt].data.isChangeable = TRUE;
        stOptItem[iCnt].data.type = OptItemCaller;
	    sprintf(stOptItem[iCnt].data.szText, "%d.%s", index++, "DISP PARAM");
	    strcpy(stOptItem[iCnt].data.valueStr, "...");
	    stOptItem[iCnt].vFunc = cDispAcqPama;
	    ++iCnt;

        stOptItem[iCnt].data.isChangeable = TRUE;
        stOptItem[iCnt].data.type = OptItemCaller;
        sprintf(stOptItem[iCnt].data.szText, "%d.%s", index++, "INFO");
        strcpy(stOptItem[iCnt].data.valueStr, "...");
        stOptItem[iCnt].vFunc = cSetAcqOption;
        ++iCnt;

        stOptItem[iCnt].data.isChangeable = ucPermission>PM_MEDIUM ? TRUE : FALSE;
        stOptItem[iCnt].data.type = OptItemCaller;
        sprintf(stOptItem[iCnt].data.szText, "%d.%s", index++, "ACQUIRER NAME");
        strcpy(stOptItem[iCnt].data.valueStr, glCurAcq.szName);
        stOptItem[iCnt].vFunc = cSetAcqName;
        ++iCnt;

        stOptItem[iCnt].data.isChangeable = ucPermission>PM_LOW ? TRUE : FALSE;
        stOptItem[iCnt].data.type = OptItemCaller;
        sprintf(stOptItem[iCnt].data.szText, "%d.%s", index++, "TERMINAL ID");
        strcpy(stOptItem[iCnt].data.valueStr,  glCurAcq.szTermID);
        stOptItem[iCnt].vFunc = cSetTerminalId;
        ++iCnt;

        stOptItem[iCnt].data.isChangeable = ucPermission>PM_LOW ? TRUE : FALSE;
        stOptItem[iCnt].data.type = OptItemCaller;
        sprintf(stOptItem[iCnt].data.szText, "%d.%s", index++, "MERCHANT ID");
        strcpy(stOptItem[iCnt].data.valueStr,  glCurAcq.szMerchantID);
        stOptItem[iCnt].vFunc = cSetMerchantId;
        ++iCnt;

        stOptItem[iCnt].data.isChangeable = ucPermission>PM_LOW ? TRUE : FALSE;
        stOptItem[iCnt].data.type = OptItemCaller;
        sprintf(stOptItem[iCnt].data.szText, "%d.%s", index++, "NII");
        strcpy(stOptItem[iCnt].data.valueStr,  glCurAcq.szNii);
        stOptItem[iCnt].vFunc = cSetNII;
        ++iCnt;

        stOptItem[iCnt].data.isChangeable = ucPermission>PM_LOW ? TRUE : FALSE;
        stOptItem[iCnt].data.type = OptItemCaller;
        sprintf(stOptItem[iCnt].data.szText, "%d.%s", index++, "BATCH NO.");
        sprintf(stOptItem[iCnt].data.valueStr,  "%lu", glCurAcq.ulCurBatchNo);
        stOptItem[iCnt].vFunc = cSetBatchNo;
        ++iCnt;


        if(glSysParam.stTxnCommCfg.ucCommType != CT_DEMO &&
                glSysParam.stTxnCommCfg.ucCommType != CT_RS232 &&
                glSysParam.stTxnCommCfg.ucCommType != CT_NONE){
            stOptItem[iCnt].data.isChangeable = ucPermission>PM_LOW ? TRUE : FALSE;
            stOptItem[iCnt].data.type = OptItemCaller;
            sprintf(stOptItem[iCnt].data.szText, "%d.%s", index++, "COMM. PARAM");
            strcpy(stOptItem[iCnt].data.valueStr,  "...");
            stOptItem[iCnt].vFunc = cSetAcqComm;
            ++iCnt;
        }

        strcpy(stOptItem[iCnt].data.szText, "");

        /*
        sprintf(stMenuItem[iSelected].szText, "%d.%s", index, "PRINTER PARAM");
        //TODO Dock case
        if(!ChkHardware(HWCFG_PRINTER, 0))
            stMenuItem[iSelected].bVisible = FALSE;
        else
        {
            stMenuItem[iSelected].bVisible = TRUE;
            ++index;
        }
        stMenuItem[iSelected].nValue = iSelected;
        stMenuItem[iSelected].vFunc = NULL;
        ++iSelected;
        */

        Gui_BindOption(GetCurrTitle(), gl_stTitleAttr, gl_stLeftAttr, "./res/On.png", "./res/Off.png",
                (GUI_OPTIONITEM *)stOptItem, &stOption);

            Gui_ClearScr();
        if( GUI_OK != Gui_ShowOptionList(&stOption, USER_OPER_TIMEOUT))
        {
            break;
        }

		if (glSysParam.ucAcqNum<2)
		{
			break;
		}
	}
}

static int SetNoPrinterPara(ACQUIRER *acq)
{
    int iRet;
    int iSelected;
    GUI_MENU stMenu;
    GUI_MENUITEM stDefMenuItem[] = {
        { _T_NOOP("Bluetooth Printer"), 1,FALSE,  NULL},
        { _T_NOOP("SMS"), 2,TRUE,  NULL},
        { _T_NOOP("EMAIL"), 3,FALSE,  NULL},
        { "", -1,FALSE,  NULL},
    };

    GUI_MENUITEM stMenuItem[4];
    int iMenuItemNum = 0;

    memset(stMenuItem, 0, sizeof(stMenuItem));

    if(!ChkHardware(HWCFG_BLTH, HW_NONE) && stDefMenuItem[0].bVisible)
    {
        memcpy(&stMenuItem[iMenuItemNum], &stDefMenuItem[0], sizeof(GUI_MENUITEM));
        sprintf(stMenuItem[iMenuItemNum].szText, "%d.%s", iMenuItemNum+1, stDefMenuItem[0].szText);
        ++iMenuItemNum;
    }

    if (!(ChkHardware(HWCFG_GPRS, HW_NONE) &&
            ChkHardware(HWCFG_CDMA, HW_NONE) &&
            ChkHardware(HWCFG_WCDMA, HW_NONE)) &&
            stDefMenuItem[1].bVisible)
    {
        memcpy(&stMenuItem[iMenuItemNum], &stDefMenuItem[1], sizeof(GUI_MENUITEM));
        sprintf(stMenuItem[iMenuItemNum].szText, "%d.%s", iMenuItemNum+1, stDefMenuItem[1].szText);
        ++iMenuItemNum;
    }

    if (!(ChkHardware(HWCFG_GPRS, HW_NONE) &&
         ChkHardware(HWCFG_CDMA, HW_NONE) &&
         ChkHardware(HWCFG_WCDMA, HW_NONE) &&
         ChkHardware(HWCFG_WIFI, HW_NONE)) &&
            stDefMenuItem[2].bVisible)
    {
        memcpy(&stMenuItem[iMenuItemNum], &stDefMenuItem[2], sizeof(GUI_MENUITEM));
        sprintf(stMenuItem[iMenuItemNum].szText, "%d.%s", iMenuItemNum+1, stDefMenuItem[2].szText);
        ++iMenuItemNum;
    }
    stMenuItem[iMenuItemNum].szText[0] = 0;

    if(!stMenuItem[0].bVisible && !stMenuItem[1].bVisible && !stMenuItem[2].bVisible)
        return 0;

    Gui_BindMenu(NULL, gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stMenuItem, &stMenu);
    iSelected = 0;
    while( 1 )
    {
        Gui_ClearScr();

        iRet = Gui_ShowMenuList(&stMenu, 0, USER_OPER_TIMEOUT, &iSelected);
        if(iRet != GUI_OK)
        {
            break;
        }

        switch(iSelected)
        {
            case 1:
                // SetBtPrinter(pair...);
                break;
            case 2:
                iRet = SetSMS(acq);
                break;
            case 3:
                // SetEmail(smtpServer,port,username,password);
                break;
        }
    }
    return iRet;
}

static int SetSMS(ACQUIRER *acq)
{
    int iRet;
    GUI_INPUTBOX_ATTR stInputAttr;
    LoadNumKeyBoard();

    memset(&stInputAttr, 0, sizeof(stInputAttr));
    stInputAttr.bEchoMode = 1;
    stInputAttr.nMinLen = 3;
    stInputAttr.nMaxLen = 6;

    uchar   szCountryCallingCode[120]="";
    GetEnv("TELCODE", szCountryCallingCode);

    while(1){
        Gui_ClearScr();
        iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("COUNTRY CALLING CODES(00*)"), gl_stLeftAttr,
                szCountryCallingCode, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
        if(GUI_ERR_USERCANCELLED == iRet)
            return iRet;
        if(memcmp(szCountryCallingCode, "00", 2))
        {
            OsBeep(4, 50);
            continue;
        }
        PutEnv("TELCODE", szCountryCallingCode);
        return GUI_OK;
    }
    return GUI_OK;
}

// 显示当前收单行参数信息
// display information of the current acquirer
// Modified by Kim_LinHB 2014-08-26 v1.01.0005 bug510
// Modified by Kim_LinHB 2014/9/16 v1.01.0009 bug510
int cDispAcqPama(int index, GUI_OPTIONITEM_DATA *data)
{
	int		ii;
	uchar	ucShowComm;
	GUI_PAGELINE stBuff[20];
	GUI_PAGE stPage;
	unsigned char ucCnt = 0;

	sprintf(stBuff[ucCnt].szLine, "%-10.10s   %3.3s", glCurAcq.szName, glCurAcq.szNii);
	stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;

	sprintf(stBuff[ucCnt].szLine, "TID:%s", glCurAcq.szTermID);
	stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;

	strcpy(stBuff[ucCnt].szLine, "MID:");
	stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
	sprintf(stBuff[ucCnt].szLine, "%s", glCurAcq.szMerchantID);
	stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;

	for (ii=0; ii<2; ii++)
	{
		ucShowComm = (ii ? glSysParam.stTxnCommCfg.ucCommTypeBak : glSysParam.stTxnCommCfg.ucCommType);

		if(1 == ii && ucShowComm != CT_NONE)
		{
			sprintf(stBuff[ucCnt].szLine, "SECONDARY COMM");
			stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
		}

		switch(ucShowComm)
		{
		case CT_TCPIP:
		case CT_WIFI:
			sprintf(stBuff[ucCnt].szLine, "Transaction:");
			stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;

			if(strlen(glCurAcq.stTxnTCPIPInfo[0].szIP) > 0)
			{
				sprintf(stBuff[ucCnt].szLine, "%s:%.5s", glCurAcq.stTxnTCPIPInfo[0].szIP, glCurAcq.stTxnTCPIPInfo[0].szPort);
				stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
			}

			if(strlen(glCurAcq.stTxnTCPIPInfo[1].szIP) > 0)
			{
                sprintf(stBuff[ucCnt].szLine, "%s:%.5s", glCurAcq.stTxnTCPIPInfo[1].szIP, glCurAcq.stTxnTCPIPInfo[1].szPort);
                stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
            }

			if(0 == strlen(glCurAcq.stTxnTCPIPInfo[0].szIP) && 0 == strlen(glCurAcq.stTxnTCPIPInfo[1].szIP))
			{
				sprintf(stBuff[ucCnt].szLine, "(NULL)");
				stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
			}

			sprintf(stBuff[ucCnt].szLine, "Settlement:");
			stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;

			if(strlen(glCurAcq.stStlTCPIPInfo[0].szIP) > 0)
            {
                sprintf(stBuff[ucCnt].szLine, "%s:%.5s", glCurAcq.stStlTCPIPInfo[0].szIP, glCurAcq.stStlTCPIPInfo[0].szPort);
                stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
            }

            if(strlen(glCurAcq.stStlTCPIPInfo[1].szIP) > 0)
            {
                sprintf(stBuff[ucCnt].szLine, "%s:%.5s", glCurAcq.stStlTCPIPInfo[1].szIP, glCurAcq.stStlTCPIPInfo[1].szPort);
                stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
            }

            if(0 == strlen(glCurAcq.stStlTCPIPInfo[0].szIP) && 0 == strlen(glCurAcq.stStlTCPIPInfo[1].szIP))
            {
                sprintf(stBuff[ucCnt].szLine, "(NULL)");
                stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
            }
			break;
		case CT_GPRS:
		case CT_CDMA:
		case CT_WCDMA:
			sprintf(stBuff[ucCnt].szLine, "Transaction:");
			stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;

            if(strlen(glCurAcq.stTxnGPRSInfo[0].szIP) > 0)
            {
                sprintf(stBuff[ucCnt].szLine, "%s:%.5s", glCurAcq.stTxnGPRSInfo[0].szIP, glCurAcq.stTxnGPRSInfo[0].szPort);
                stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
            }

            if(strlen(glCurAcq.stTxnGPRSInfo[1].szIP) > 0)
            {
                sprintf(stBuff[ucCnt].szLine, "%s:%.5s", glCurAcq.stTxnGPRSInfo[1].szIP, glCurAcq.stTxnGPRSInfo[1].szPort);
                stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
            }

            if(0 == strlen(glCurAcq.stTxnGPRSInfo[0].szIP) && 0 == strlen(glCurAcq.stTxnGPRSInfo[1].szIP))
            {
                sprintf(stBuff[ucCnt].szLine, "(NULL)");
                stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
            }

			sprintf(stBuff[ucCnt].szLine, "Settlement:");
			stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;

            if(strlen(glCurAcq.stStlGPRSInfo[0].szIP) > 0)
            {
                sprintf(stBuff[ucCnt].szLine, "%s:%.5s", glCurAcq.stStlGPRSInfo[0].szIP, glCurAcq.stStlGPRSInfo[0].szPort);
                stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
            }

            if(strlen(glCurAcq.stStlGPRSInfo[1].szIP) > 0)
            {
                sprintf(stBuff[ucCnt].szLine, "%s:%.5s", glCurAcq.stStlGPRSInfo[1].szIP, glCurAcq.stStlGPRSInfo[1].szPort);
                stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
            }

            if(0 == strlen(glCurAcq.stStlGPRSInfo[0].szIP) && 0 == strlen(glCurAcq.stStlGPRSInfo[1].szIP))
            {
                sprintf(stBuff[ucCnt].szLine, "(NULL)");
                stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
            }
			break;
		case CT_MODEM:
			sprintf(stBuff[ucCnt].szLine, "Transaction:");
			stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;

			if(strlen(glCurAcq.TxnTelNo1) > 0)
			{
				sprintf(stBuff[ucCnt].szLine, "%.21s", glCurAcq.TxnTelNo1);
				stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
			}
			if(strlen(glCurAcq.TxnTelNo2) > 0)
			{
				sprintf(stBuff[ucCnt].szLine, "%.21s", glCurAcq.TxnTelNo2);
				stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
			}
			if(0 == strlen(glCurAcq.TxnTelNo1) && 0 == strlen(glCurAcq.TxnTelNo2))
			{
				sprintf(stBuff[ucCnt].szLine, "(NULL)");
				stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
			}

			sprintf(stBuff[ucCnt].szLine, "Settlement:");
			stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;

			if(strlen(glCurAcq.StlTelNo1) > 0)
			{
				sprintf(stBuff[ucCnt].szLine, "%.21s", glCurAcq.StlTelNo1);
				stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
			}
			if(strlen(glCurAcq.StlTelNo2) > 0)
			{
				sprintf(stBuff[ucCnt].szLine, "%.21s", glCurAcq.StlTelNo2);
				stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
			}
			if(0 == strlen(glCurAcq.StlTelNo1) && 0 == strlen(glCurAcq.StlTelNo2))
			{
				sprintf(stBuff[ucCnt].szLine, "(NULL)");
				stBuff[ucCnt++].stLineAttr = gl_stLeftAttr;
			}
			break;
		case CT_NONE:
		default:
			break;
		}
	}

	Gui_CreateInfoPage(NULL, gl_stTitleAttr, stBuff, ucCnt, &stPage);

	Gui_ClearScr();

	return Gui_ShowInfoPage(&stPage, FALSE, USER_OPER_TIMEOUT);
}

int SetAcqTransComm(uchar ucCommType)
{
	int		iRet;

	switch(ucCommType)
	{
	case CT_TCPIP:
	case CT_WIFI:
		iRet = GetRemoteIp("TRANS IP 1",  TRUE, TRUE, &glCurAcq.stTxnTCPIPInfo[0]);
		if (iRet)
		{
			break;
		}
		iRet = GetRemoteIp("TRANS IP 2",  TRUE, TRUE, &glCurAcq.stTxnTCPIPInfo[1]);
		//iRet = GetRemoteIp("SETTLE 1st", TRUE, &glCurAcq.stStlTCPIPInfo[0]);
		//iRet = GetRemoteIp("SETTLE 2nd", TRUE, &glCurAcq.stStlTCPIPInfo[1]);
		break;
	case CT_GPRS:
	case CT_CDMA:
	case CT_WCDMA:
		iRet = GetRemoteIp("TRANS IP 1",  TRUE, TRUE, &glCurAcq.stTxnGPRSInfo[0]);
		if (iRet)
		{
			break;
		}
		iRet = GetRemoteIp("TRANS IP 2",  TRUE, TRUE, &glCurAcq.stTxnGPRSInfo[1]);
		//iRet = GetRemoteIp("SETTLE 1st", TRUE, &glCurAcq.stStlGPRSInfo[0]);
		//iRet = GetRemoteIp("SETTLE 2nd", TRUE, &glCurAcq.stStlGPRSInfo[1]);
		break;
	case CT_MODEM:
		iRet = SetAcqTransTelNo();
        break;
	case CT_RS232:
	case CT_NONE:
	default:
		iRet = 0;
	    break;
	}

	return iRet;
}

static int  SetAcqTransEncrypt(ACQUIRER *acq)
{
    uchar szBuff[120] = {0};
    int bSSL = 0;
    uchar szSSL[120];
    if(0 == GetEnv("E_SSL", szSSL))
    {
        bSSL = atoi(szSSL);
    }

    int iRet = Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, _T("SSL"),
            gl_stLeftAttr, _T("NO"), 0, _T("YES"), 1,
            USER_OPER_TIMEOUT, &bSSL);
    if(iRet != GUI_OK)
        return iRet;

   sprintf(szSSL, "%d", bSSL? 1: 0);
   PutEnv("E_SSL", szSSL);

    if(bSSL){
        GUI_INPUTBOX_ATTR stInputAttr;

        memset(&stInputAttr, 0, sizeof(stInputAttr));
        LoadMixKeyBoard();
        stInputAttr.bEchoMode = 1;
        stInputAttr.nMinLen = 1;
        stInputAttr.nMaxLen = 16;

        if(GetEnv("CA_CRT", szBuff) != 0)
        {
            memset(szBuff, 0, sizeof(szBuff));
        }

        iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("CA File"), gl_stLeftAttr, szBuff,gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
        if(iRet != GUI_OK)
            return iRet;
        PutEnv("CA_CRT", szBuff);

        if(GetEnv("CLI_CRT", szBuff) != 0)
        {
            memset(szBuff, 0, sizeof(szBuff));
        }

        iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("Cert File"), gl_stLeftAttr, szBuff,gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
        if(iRet != GUI_OK)
            return iRet;
        PutEnv("CLI_CRT", szBuff);

        if(GetEnv("CLI_KEY", szBuff) != 0)
        {
            memset(szBuff, 0, sizeof(szBuff));
        }
        iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("Private Key"), gl_stLeftAttr, szBuff,gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
        if(iRet != GUI_OK)
            return iRet;
        PutEnv("CLI_KEY", szBuff);
    }
    return 0;
}

// 设置当前收单行交易电话号码
// set the transaction tel NO. of the current acquirer
int SetAcqTransTelNo(void)
{
	if( SetTel(glCurAcq.TxnTelNo1, _T("TRANS TELNO 1"))!=0 )
	{
		return ERR_USERCANCEL;
	}
	memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));

	if( SetTel(glCurAcq.TxnTelNo2, _T("TRANS TELNO 2"))!=0 )
	{
		return ERR_USERCANCEL;
	}
	memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));

	if( SetTel(glCurAcq.StlTelNo1, _T("SETTTLE TELNO 1"))!=0 )
	{
		return ERR_USERCANCEL;
	}
	memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));

	if( SetTel(glCurAcq.StlTelNo2, _T("SETTTLE TELNO 2"))!=0 )
	{
		return ERR_USERCANCEL;
	}
	memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));

	return 0;
}

// 设置电话号码
// set tel NO.
int SetTel(uchar *pszTelNo, const uchar *pszPromptInfo)
{
	GUI_INPUTBOX_ATTR stInputAttr;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	LoadNum_SymKeyBoard();
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 24;
	stInputAttr.bEchoMode = 1;
   
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, pszPromptInfo, gl_stLeftAttr, 
		pszTelNo, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	return 0;
}

int SetWirelessParam(WIRELESS_PARAM *pstParam)
{
	GUI_INPUTBOX_ATTR stInputAttr;

	if (pstParam==NULL)
	{
		return ERR_NO_DISP;
	}

	Gui_ClearScr();
	int iSelect = pstParam->ucUsingSlot;
    if(GUI_OK != Gui_ShowAlternative("SETUP WIRELESS", gl_stTitleAttr, "Select SIM", gl_stCenterAttr,
        "SIM 1", 0, "SIM 2", 1, USER_OPER_TIMEOUT, &iSelect))
    {
        return ERR_USERCANCEL;
    }
    pstParam->ucUsingSlot = iSelect;

	LoadMixKeyBoard();

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.bEchoMode = 1;

	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 32;
    Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox("SETUP WIRELESS", gl_stTitleAttr, "APN", gl_stLeftAttr, pstParam->szAPN, 
		gl_stRightAttr, &stInputAttr,USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 32;
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox("SETUP WIRELESS", gl_stTitleAttr, "LOGIN NAME", gl_stLeftAttr, 
		pstParam->szUID, gl_stRightAttr, &stInputAttr,USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	stInputAttr.bEchoMode = 0;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 16;
	stInputAttr.bSensitive = 1;
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox("SETUP WIRELESS", gl_stTitleAttr, "LOGIN PWD", gl_stLeftAttr, 
		pstParam->szPwd, gl_stRightAttr, &stInputAttr,USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = 16;
	stInputAttr.bSensitive = 1;
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox("SETUP WIRELESS", gl_stTitleAttr, "SIM PIN", gl_stLeftAttr, pstParam->szSimPin, 
		gl_stRightAttr,&stInputAttr,USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	return 0;
}

void SyncWirelessParam(WIRELESS_PARAM *pstDst, const WIRELESS_PARAM *pstSrc)
{
	strcpy((char *)(pstDst->szAPN),    (char *)(pstSrc->szAPN));
	strcpy((char *)(pstDst->szUID),    (char *)(pstSrc->szUID));
	strcpy((char *)(pstDst->szPwd),    (char *)(pstSrc->szPwd));
	strcpy((char *)(pstDst->szSimPin), (char *)(pstSrc->szSimPin));
	strcpy((char *)(pstDst->szDNS),    (char *)(pstSrc->szDNS));
}

//Added by Kim_LinHB 2014-8-16
//TODO Kim for now, ignore master case
int  SetBTParam(BT_PARAM *pstParam)
{
	GUI_INPUTBOX_ATTR stInputAttr;

	if (pstParam==NULL)
	{
		return ERR_NO_DISP;
	}

	LoadMixKeyBoard();

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.bEchoMode = 1;

	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = sizeof(pstParam->stScanResult.Name);
    Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox("SETUP BULETOOTH", gl_stTitleAttr, "Name", gl_stLeftAttr, pstParam->stScanResult.Name,
		gl_stRightAttr, &stInputAttr,USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 0;
	stInputAttr.nMaxLen = sizeof(pstParam->stPaired.LinkKey);
	stInputAttr.bSensitive = 1;
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox("SETUP BULETOOTH", gl_stTitleAttr, "PIN", gl_stLeftAttr, pstParam->stPaired.LinkKey,
		gl_stRightAttr,&stInputAttr,USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}

	return 0;
}
void SyncBTParam(BT_PARAM *pstDst, const BT_PARAM *pstSrc)
{
	pstDst->iHandle = pstSrc->iHandle;
    memcpy(&pstDst->stScanResult, &pstSrc->stScanResult, sizeof(ST_BT_SCAN_RESULT));
    memcpy(&pstDst->stPaired, &pstSrc->stPaired, sizeof(ST_BT_PAIRED));
}
//Add End

// 设置EDC参数
// set EDC parameters
void SetEdcParam(uchar ucPermission)
{
    int iCnt = 0;
    GUI_OPTION stOption;
    GUI_OPTIONITEM stOptItem[20];

    stOptItem[iCnt].data.isChangeable = 1;
    sprintf(stOptItem[iCnt].data.szText, "%d.%s", iCnt + 1, _T("AREA CODE"));
    stOptItem[iCnt].data.type = OptItemCaller;
    PubBcd2Asc0(glSysParam.stEdcInfo.stLocalCurrency.sCountryCode, 2, stOptItem[iCnt].data.valueStr);
    stOptItem[iCnt].vFunc = cSetTermCountry;
    ++iCnt;

    stOptItem[iCnt].data.isChangeable = ucPermission<PM_HIGH ? 0 : 1;
    sprintf(stOptItem[iCnt].data.szText, "%d.%s", iCnt + 1, _T("CURRENCY CODE"));
    stOptItem[iCnt].data.type = OptItemCaller;
    PubBcd2Asc0(glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, 2, stOptItem[iCnt].data.valueStr);
    stOptItem[iCnt].vFunc = cSetTermCurrency;
    ++iCnt;

    stOptItem[iCnt].data.isChangeable = ucPermission>PM_MEDIUM ? 1 : 0;
    sprintf(stOptItem[iCnt].data.szText, "%d.%s", iCnt + 1, _T("DECIMAL POSITION"));
    stOptItem[iCnt].data.type = OptItemCaller;
    sprintf(stOptItem[iCnt].data.valueStr, "%d", glSysParam.stEdcInfo.stLocalCurrency.ucDecimal);
    stOptItem[iCnt].vFunc = cSetTermDecimalPosition;
    ++iCnt;

    stOptItem[iCnt].data.isChangeable = ucPermission>PM_MEDIUM ? 1 : 0;
    sprintf(stOptItem[iCnt].data.szText, "%d.%s", iCnt + 1, _T("IGNORE DIGIT"));
    stOptItem[iCnt].data.type = OptItemCaller;
    sprintf(stOptItem[iCnt].data.valueStr, "%d", glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit);
    stOptItem[iCnt].vFunc = cSetTermIgnoreDigit;
    ++iCnt;

    stOptItem[iCnt].data.isChangeable = ucPermission<PM_HIGH ? 0 : 1;
    sprintf(stOptItem[iCnt].data.szText, "%d.%s", iCnt + 1, _T("MERCHANT NAME"));
    stOptItem[iCnt].data.type = OptItemCaller;
    strcpy(stOptItem[iCnt].data.valueStr, glSysParam.stEdcInfo.szMerchantName);
    stOptItem[iCnt].vFunc = cSetMerchantName;
    ++iCnt;

    stOptItem[iCnt].data.isChangeable = ucPermission<PM_HIGH ? 0 : 1;
    sprintf(stOptItem[iCnt].data.szText, "%d.%s", iCnt + 1, _T("MERCHANT ADDR"));
    stOptItem[iCnt].data.type = OptItemCaller;
    strcpy(stOptItem[iCnt].data.valueStr, glSysParam.stEdcInfo.szMerchantAddr);
    stOptItem[iCnt].vFunc = cSetMerchantAddr;
    ++iCnt;

    stOptItem[iCnt].data.isChangeable = 1;
    sprintf(stOptItem[iCnt].data.szText, "%d.%s", iCnt + 1, _T("PED MODE"));
    stOptItem[iCnt].data.type = OptItemCaller;
    strcpy(stOptItem[iCnt].data.valueStr, sg_stPINPADMenuItem[glSysParam.stEdcInfo.ucPedMode].szText);
    stOptItem[iCnt].vFunc = cSetPEDMode;
    ++iCnt;

    stOptItem[iCnt].data.isChangeable = 1;
    sprintf(stOptItem[iCnt].data.szText, "%d.%s", iCnt + 1, _T("CONFIRM TIME OUT"));
    stOptItem[iCnt].data.type = OptItemCaller;
    sprintf(stOptItem[iCnt].data.valueStr, "%d", glSysParam.stEdcInfo.ucAcceptTimeout);
    stOptItem[iCnt].vFunc = cSetAcceptTimeOut;
    ++iCnt;

    stOptItem[iCnt].data.isChangeable = 0;
    sprintf(stOptItem[iCnt].data.szText, "%d.%s", iCnt + 1, _T("PRINTER TYPE"));
    stOptItem[iCnt].data.type = OptItemCaller;
    strcpy(stOptItem[iCnt].data.valueStr, glSysParam.stEdcInfo.ucPrinterType == 1 ? _T("THERMAL") : _T("SPROCKET"));
    stOptItem[iCnt].vFunc = cSetPrinterType;
    ++iCnt;

    stOptItem[iCnt].data.isChangeable = 1;
    sprintf(stOptItem[iCnt].data.szText, "%d.%s", iCnt + 1, _T("RECEIPT #"));
    stOptItem[iCnt].data.type = OptItemCaller;
    sprintf(stOptItem[iCnt].data.valueStr, "%d", NumOfReceipt());
    stOptItem[iCnt].vFunc = cSetNumOfReceipt;
    ++iCnt;

    stOptItem[iCnt].data.isChangeable = ucPermission>PM_LOW ? 1 : 0;
    sprintf(stOptItem[iCnt].data.szText, "%d.%s", iCnt + 1, _T("TRACE NO"));
    stOptItem[iCnt].data.type = OptItemCaller;
    sprintf(stOptItem[iCnt].data.valueStr, "%06ld", glSysCtrl.ulSTAN);
    stOptItem[iCnt].vFunc = cSetGetSysTraceNo;
    ++iCnt;

    stOptItem[iCnt].data.isChangeable = ucPermission>PM_LOW ? 1 : 0;
    sprintf(stOptItem[iCnt].data.szText, "%d.%s", iCnt + 1, _T("INVOICE NO"));
    stOptItem[iCnt].data.type = OptItemCaller;
    sprintf(stOptItem[iCnt].data.valueStr, "%06ld", glSysCtrl.ulInvoiceNo);
    stOptItem[iCnt].vFunc = cSetGetSysInvoiceNo;
    ++iCnt;

    stOptItem[iCnt].data.isChangeable = ucPermission;
    sprintf(stOptItem[iCnt].data.szText, "%d.%s", iCnt + 1, _T("INFO"));
    stOptItem[iCnt].data.type = OptItemCaller;
    strcpy(stOptItem[iCnt].data.valueStr, "...");
    stOptItem[iCnt].vFunc = cSetEdcInfoOption;
    ++iCnt;

    stOptItem[iCnt].data.isChangeable = ucPermission;
    sprintf(stOptItem[iCnt].data.szText, "%d.%s", iCnt + 1, _T("EXT INFO"));
    stOptItem[iCnt].data.type = OptItemCaller;
    strcpy(stOptItem[iCnt].data.valueStr, "...");
    stOptItem[iCnt].vFunc = cSetEdcInfoExtOption;
    ++iCnt;

    strcpy(stOptItem[iCnt].data.szText, "");


    Gui_BindOption(GetCurrTitle(), gl_stTitleAttr, gl_stLeftAttr, "./res/On.png", "./res/Off.png",
            (GUI_OPTIONITEM *)stOptItem, &stOption);

    Gui_ClearScr();
    if( GUI_OK != Gui_ShowOptionList(&stOption, USER_OPER_TIMEOUT))
    {
        return;
    }

#ifdef ENABLE_EMV
    SyncEmvCurrency(glSysParam.stEdcInfo.stLocalCurrency.sCountryCode,
                    glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode,
                    glSysParam.stEdcInfo.stLocalCurrency.ucDecimal);
#endif
}

// -1 : 值无改变 -2 : 超时或取消
// >=0 : 输入的合法值
// ucEdit      : 是否允许编辑
// pszFirstMsg : 标题下面第一行提示
// pszSecMsg   : 标题下面第二行提示
// ulMin,ulMax : 允许的取值范围
// lOrgValue   : 原值
// -1 : keeping -2 : timeout or cancel
// >=0 : valid input
// ucEdit      : if allowed to edit
// pszFirstMsg : the 1st line of prompt
// pszSecMsg   : the 2nd line of prompt
// ulMin,ulMax : value range
// lOrgValue   : original value
long ViewGetValue(uchar ucEdit, const void *pszFirstMsg, const void *pszSecMsg,
				  ulong ulMin, ulong ulMax, char *valueStr)
{
	uchar	szBuff[32], szPrompt[200] = {0}, ucMinDigit, ucMaxDigit;
	ulong	ulTemp;
	int iRet;

	PubASSERT(ulMax<=0x07FFFFFF); // Modified by Kim_LinHB 2014-8-5 v1.01.0001
	//PubASSERT(ulMax<2147483648);

	ulTemp = ulMin;
	ucMinDigit = 0;
	do{ucMinDigit++;}while (ulTemp/=10);

	ulTemp = ulMax;
	ucMaxDigit = 0;
	do{ucMaxDigit++;}while (ulTemp/=10);

	memset(szBuff, 0, sizeof(szBuff));
	if (valueStr)
	{
		strcpy((char *)szBuff, valueStr);
	}

	if (pszFirstMsg!=NULL)
	{
		strcpy(szPrompt, _T(pszFirstMsg));
	}

	if (pszSecMsg!=NULL)
	{
		if(strlen(szPrompt) > 0){
			strcat(szPrompt, "\n");
		}
		strcat(szPrompt+strlen(szPrompt), _T(pszSecMsg));
	}

	if (ucEdit)
	{
		GUI_INPUTBOX_ATTR stInputAttr;
		LoadNumKeyBoard();

		memset(&stInputAttr, 0, sizeof(stInputAttr));
		stInputAttr.nMinLen = ucMinDigit;
		stInputAttr.nMaxLen = ucMaxDigit;
		stInputAttr.bEchoMode = 1;

		while (1)
		{
			Gui_ClearScr();
			// Allow to modify 
			if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stLeftAttr, szBuff, 
				gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
			{
				return -2;
			}
			ulTemp = (ulong)atol((char *)szBuff);
			if ((ulTemp<ulMin) || (ulTemp>ulMax))
			{
				Gui_ClearScr();
				PubBeepErr();
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("INVALID VALUE"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
				continue;
			}

			if (ulTemp!=(ulong)atol(valueStr))
			{
			    if(valueStr)
			        strcpy(valueStr, szBuff);
				return (long)ulTemp;
			}
			return -1;
		}
	}
	else
	{
		// Read only
		if(strlen(szPrompt) > 0){
			strcat(szPrompt, "\n");
			strcat(szPrompt, szBuff);
		}
		else{
			strcpy(szPrompt, szBuff);
		}
		Gui_ClearScr();
		iRet = Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, szPrompt, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL);
		
		if (iRet != GUI_OK)
		{
			return -2;
		}
		return -1;
	}
	return 0;
}

int cSetTermCountry(int index, GUI_OPTIONITEM_DATA *data)
{
	GUI_INPUTBOX_ATTR stInputAttr;
	LoadNumKeyBoard();

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.bEchoMode = 1;

	stInputAttr.nMinLen = 3;
	stInputAttr.nMaxLen = 3;

    // Allow modify
	Gui_ClearScr();
    if (GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, strchr(data->szText, '.') + 1, gl_stLeftAttr,
		data->valueStr + 1, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
    {
        return ERR_USERCANCEL;
    }
        
    PubAsc2Bcd(data->valueStr, 3, glSysParam.stEdcInfo.stLocalCurrency.sCountryCode);
    SaveEdcParam();
    return 0;
}
int cSetTermCurrency(int index, GUI_OPTIONITEM_DATA *data)
{
    CURRENCY_CONFIG stCurrency;

	if (!data->isChangeable)
	{
		// Modified by Kim_LinHB 2014-08-18 v1.01.0004
		int iRet;
		unsigned char szBuff_Temp[200];
		sprintf(szBuff_Temp, "%s\n%s", strchr(data->szText, '.') + 1, data->valueStr + 1);
		// Read only
		Gui_ClearScr();
		iRet = Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, szBuff_Temp, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL);
		if (iRet != GUI_OK)
		{
			return ERR_USERCANCEL;
		}
	}
	else
	{
	    uchar   szBuff[32];
	    strcpy(szBuff, data->valueStr + 1);

	    GUI_INPUTBOX_ATTR stInputAttr;
	    LoadNumKeyBoard();

	    memset(&stInputAttr, 0, sizeof(stInputAttr));
	    stInputAttr.bEchoMode = 1;

	    stInputAttr.nMinLen = 3;
	    stInputAttr.nMaxLen = 3;
		while(1)
		{
			while (2)
			{
				Gui_ClearScr();
				// Allow modify
				if (GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, strchr(data->szText, '.') + 1,
					gl_stLeftAttr, szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
				{
					return ERR_USERCANCEL;
				}
				if (FindCurrency(szBuff, &stCurrency)!=0)
				{   
					Gui_ClearScr();
					PubBeepErr();
					Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_WARNING, _T("INVALID CURRENCY"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
					continue;
				}
				break;
			}
       
			Gui_ClearScr();
			sprintf((char *)szBuff, "%.3s %02X%02X",
				stCurrency.szName, stCurrency.sCurrencyCode[0], stCurrency.sCurrencyCode[1]);
			if (GUI_ERR_USERCANCELLED == Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, szBuff, gl_stCenterAttr, GUI_BUTTON_YandN, -1, NULL))
			{
				continue;
			}
			break;
		}
		sprintf((char *)glSysParam.stEdcInfo.stLocalCurrency.szName, "%.3s", stCurrency.szName);
		memcpy(glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, stCurrency.sCurrencyCode, 2);
	    SaveEdcParam();

		sprintf(data->valueStr, "%02X%02X", stCurrency.sCurrencyCode[0], stCurrency.sCurrencyCode[1]);
	}
	return 0;
}

int cSetTermDecimalPosition(int index, GUI_OPTIONITEM_DATA *data)
{
	// Input decimal position value, 0<=x<=3
	// for JPY and KRW, x=0; for TWD, x=0 or x=2
    long lTemp = ViewGetValue(data->isChangeable, strchr(data->szText, '.') + 1, NULL, 0, 3, data->valueStr);
	if (lTemp==-2)
	{
		return ERR_USERCANCEL;
	}
	if (lTemp>=0)
	{
		glSysParam.stEdcInfo.stLocalCurrency.ucDecimal = (uchar)lTemp;
	    SaveEdcParam();
	}
	return 0;
}

int cSetTermIgnoreDigit(int index, GUI_OPTIONITEM_DATA *data)
{
	// Input ignore digit value, 0<=x<=3
	// for JPY and KRW, x=2; for TWD, when decimal=0, x=2; decimal=2, x=0;
	long lTemp = ViewGetValue(data->isChangeable, strchr(data->szText, '.') + 1, NULL, 0, 3, data->valueStr);
	if (lTemp==-2)
	{
		return ERR_USERCANCEL;
	}
	if (lTemp>=0)
	{
		glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit = (uchar)lTemp;
		SaveEdcParam();
	}

	return 0;
}

int cSetMerchantName(int index, GUI_OPTIONITEM_DATA *data)
{
	uchar	szBuff[46+1];

	GUI_INPUTBOX_ATTR stInputAttr;

	if (!data->isChangeable)	// Not allow to set
	{
	    uchar szTemp[255];
	    sprintf(szTemp, "%s\n%s", strchr(data->szText, '.') + 1, data->valueStr);
	    Gui_ClearScr();
	    if (GUI_OK != Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, szTemp, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL))
	    {
	        return ERR_USERCANCEL;
        }
	    return 0;
	}

	LoadMixKeyBoard();

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.bEchoMode = 1;
	
	//NAME
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 23;
	sprintf((char *)szBuff, "%.23s", (char *)data->valueStr);
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, strchr(data->szText, '.') + 1, gl_stLeftAttr,
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}
	if (strcmp((char *)data->valueStr, (char *)szBuff)!=0)
	{
		sprintf((char *)data->valueStr, "%.23s", (char *)szBuff);
		SaveEdcParam();
	}
	return 0;
}
int cSetMerchantAddr(int index, GUI_OPTIONITEM_DATA *data)
{
    uchar   szBuff[46+1];

    GUI_INPUTBOX_ATTR stInputAttr;

    if (!data->isChangeable)   // Not allow to set
    {
        uchar szTemp[255];
        sprintf(szTemp, "%s\n%s", strchr(data->szText, '.') + 1, data->valueStr);
        Gui_ClearScr();
        if (GUI_OK != Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, szTemp, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL))
        {
            return ERR_USERCANCEL;
        }
        return 0;
    }

    LoadMixKeyBoard();

    memset(&stInputAttr, 0, sizeof(stInputAttr));
    stInputAttr.bEchoMode = 1;

	//ADDRESS
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 46;
	sprintf((char *)szBuff, "%.46s", data->valueStr);
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, strchr(data->szText, '.') + 1, gl_stLeftAttr,
		szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
	{
		return ERR_USERCANCEL;
	}
	if (strcmp(data->valueStr, (char *)szBuff)!=0)
	{
		sprintf(data->valueStr, "%.46s", (char *)szBuff);
		SaveEdcParam();
	}

	return 0;
}

int cSetGetSysTraceNo(int index, GUI_OPTIONITEM_DATA *data)
{
	uchar	szBuff[20];

	Gui_ClearScr();
	if (data->isChangeable)
	{
		GUI_INPUTBOX_ATTR stInputAttr;
		LoadNumKeyBoard();

		memset(&stInputAttr, 0, sizeof(stInputAttr));
		stInputAttr.bEchoMode = 1;

		//NAME
		stInputAttr.nMinLen = 1;
		stInputAttr.nMaxLen = 6;

		sprintf((char *)szBuff, data->valueStr);
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, "S.T.A.N", gl_stLeftAttr, szBuff,
			gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}

		glSysCtrl.ulSTAN = (ulong)atol((char *)szBuff);
		SaveSysCtrlBase();
		strcpy(data->valueStr, szBuff);
	} 
	else
	{
		sprintf(szBuff, "%s\n%s", strchr(data->szText, '.') + 1, data->valueStr);
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL);
	}

	return 0;
}

int cSetGetSysInvoiceNo(int index, GUI_OPTIONITEM_DATA *data)
{
	uchar	szBuff[20];

	Gui_ClearScr();
	if (data->isChangeable)
	{
		GUI_INPUTBOX_ATTR stInputAttr;
		LoadNumKeyBoard();

		memset(&stInputAttr, 0, sizeof(stInputAttr));
		stInputAttr.bEchoMode = 1;

		//NAME
		stInputAttr.nMinLen = 1;
		stInputAttr.nMaxLen = 6;

		strcpy((char *)szBuff, data->valueStr);
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, "TRACE NO", gl_stLeftAttr, szBuff, 
			gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}

		glSysCtrl.ulInvoiceNo = (ulong)atol((char *)szBuff);
		SaveSysCtrlBase();
	} 
	else
	{
		sprintf(szBuff, "TRACE NO\n%s", data->valueStr);
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL);
	}

	return 0;
}

// Select PED mode used.
int cSetPEDMode(int index, GUI_OPTIONITEM_DATA *data)
{
	int	i, iSel = 0;
	GUI_MENU stPINPADMenu;

	for(i = 0; i < sizeof(sg_stPINPADMenuItem)/sizeof(sg_stPINPADMenuItem[0]); ++i){
	    if(0 == strcmp(sg_stPINPADMenuItem[i].szText,data->valueStr)){
	        iSel = i;
	        break;
	    }
	}
	Gui_BindMenu(GetCurrTitle(), gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)sg_stPINPADMenuItem, &stPINPADMenu);
	
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowMenuList(&stPINPADMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, &iSel))
	{
		return ERR_USERCANCEL;
	}

	glSysParam.stEdcInfo.ucPedMode = (uchar)iSel;
    SaveEdcParam();
    strcpy(data->valueStr, sg_stPINPADMenuItem[iSel].szText);
	return 0;
}

// 输入交易成功时确认信息显示时间
// set the timeout for display "TXN accepted" message
int cSetAcceptTimeOut(int index, GUI_OPTIONITEM_DATA *data)
{
	uchar	szBuff[2+1];

	GUI_TEXT_ATTR stPrompt, stContent;
	GUI_INPUTBOX_ATTR stInputAttr;
	LoadNumKeyBoard();

	stPrompt = stContent = gl_stCenterAttr;
	stPrompt.eAlign = GUI_ALIGN_LEFT;
	stContent.eAlign = GUI_ALIGN_RIGHT;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 2;
	
	while( 1 )
	{
		strcpy((char *)szBuff, data->valueStr);
	   
		Gui_ClearScr();
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, strchr(data->szText, '.') + 1, stPrompt,
			szBuff, stContent, &stInputAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}
		if( atoi((char *)szBuff)<=60 )
		{
			break;
		}

		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("INVALID INPUT"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
	}
	glSysParam.stEdcInfo.ucAcceptTimeout = (uchar)atoi((char *)szBuff);
    SaveEdcParam();
    strcpy(data->valueStr, szBuff);
	return 0;
}

// 仅适用于分离式打印机
int cSetPrinterType(int index, GUI_OPTIONITEM_DATA *data)
{
    uchar szTemp[255];
    sprintf(szTemp, "%s\n%s", strchr(data->szText, '.') + 1, data->valueStr);
    Gui_ClearScr();
    if (GUI_OK != Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, szTemp, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL))
    {
        return ERR_USERCANCEL;
    }
    return 0;

#if 0
	int iSel = glSysParam.stEdcInfo.ucPrinterType;
	Gui_ClearScr();
	if(GUI_OK != Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, "PRINTER TYPE", gl_stCenterAttr, 
		"THERMAL", 1, "SPROCKET", 0, USER_OPER_TIMEOUT, &iSel))
	{
		return ERR_USERCANCEL;
	}

	glSysParam.stEdcInfo.ucPrinterType = iSel;
#endif
	return 0;
}

// 输入热敏打印单据张数
// set receipt numbers, just for thermal terminal
int cSetNumOfReceipt(int index, GUI_OPTIONITEM_DATA *data)
{
	uchar 	ucNum, szBuff[1+1];
	GUI_TEXT_ATTR stPrompt, stContent;
	GUI_INPUTBOX_ATTR stInputAttr;

	if( !ChkIfThermalPrinter() )
	{
		return 0;
	}

	LoadNumKeyBoard();

	stPrompt = stContent = gl_stCenterAttr;
	stPrompt.eAlign = GUI_ALIGN_LEFT;
	stContent.eAlign = GUI_ALIGN_RIGHT;

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.bEchoMode = 1;
	stInputAttr.nMinLen = 1;
	stInputAttr.nMaxLen = 1;

	while( 1 )
	{
		strcpy((char *)szBuff, data->valueStr);
		Gui_ClearScr();
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, strchr(data->szText, '.') + 1, stPrompt,
			szBuff, stContent, &stInputAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}

		ucNum = (uchar)atoi((char *)szBuff);
		if( ucNum>=1 && ucNum<=4 )
		{
			ucNum--;
			break;
		}
		Gui_ClearScr();
		PubBeepErr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("INVALID INPUT"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
	}

	glSysParam.stEdcInfo.sOption[EDC_NUM_PRINT_LOW/0x100]  &= (0xFF^(EDC_NUM_PRINT_LOW%0x100));
	glSysParam.stEdcInfo.sOption[EDC_NUM_PRINT_HIGH/0x100] &= (0xFF^(EDC_NUM_PRINT_HIGH%0x100));
	if( ucNum & 0x01 )
	{
		glSysParam.stEdcInfo.sOption[EDC_NUM_PRINT_LOW/0x100] |= (EDC_NUM_PRINT_LOW%0x100);
	}
	if( ucNum & 0x02 )
	{
		glSysParam.stEdcInfo.sOption[EDC_NUM_PRINT_HIGH/0x100] |= (EDC_NUM_PRINT_HIGH%0x100);
	}
	SaveEdcParam();
	strcpy(data->valueStr, szBuff);

	return 0;
}

static int cSetEdcInfoOption(int index, GUI_OPTIONITEM_DATA *data){
    return ModifyOptList(glSysParam.stEdcInfo.sOption, 'E', data->isChangeable);
}
static int cSetEdcInfoExtOption(int index, GUI_OPTIONITEM_DATA *data){
    return ModifyOptList(glSysParam.stEdcInfo.sExtOption, 'e', data->isChangeable);
}
static int cSetAcqOption(int index, GUI_OPTIONITEM_DATA *data){
    int iRet = ModifyOptList(glSysParam.stAcqList[glCurAcq.ucIndex].sOption, 'A', data->isChangeable);
    SetCurAcq(glCurAcq.ucIndex);
    return iRet;
}
static int cSetAcqName(int index, GUI_OPTIONITEM_DATA *data)
{
    GUI_INPUTBOX_ATTR stInputAttr;

    memset(&stInputAttr, 0, sizeof(stInputAttr));
    stInputAttr.bEchoMode = 1;

    LoadMixKeyBoard();
    stInputAttr.nMinLen = 3;
    stInputAttr.nMaxLen = 10;
    Gui_ClearScr();
    if(GUI_OK == Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, strchr(data->szText, '.') + 1, gl_stLeftAttr,
            glCurAcq.szName, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
    {
        memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));
        SaveSysParam();
        strcpy(data->valueStr, glCurAcq.szName);
    }
    return 0;
}
static int cSetTerminalId(int index, GUI_OPTIONITEM_DATA *data)
{
    GUI_INPUTBOX_ATTR stInputAttr;

    memset(&stInputAttr, 0, sizeof(stInputAttr));
    stInputAttr.bEchoMode = 1;

    LoadNumKeyBoard();
    stInputAttr.nMinLen = 8;
    stInputAttr.nMaxLen = 8;
    Gui_ClearScr();
    if(GUI_OK == Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, strchr(data->szText, '.') + 1, gl_stLeftAttr,
            glCurAcq.szTermID, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
    {
        memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));
        SaveSysParam();
        strcpy(data->valueStr, glCurAcq.szTermID);
    }
    return 0;
}
static int cSetMerchantId(int index, GUI_OPTIONITEM_DATA *data)
{
    GUI_INPUTBOX_ATTR stInputAttr;

    memset(&stInputAttr, 0, sizeof(stInputAttr));
    stInputAttr.bEchoMode = 1;

    LoadNumKeyBoard();
    stInputAttr.nMinLen = 8;
    stInputAttr.nMaxLen = 15;
    Gui_ClearScr();
    if(GUI_OK == Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, strchr(data->szText, '.') + 1, gl_stLeftAttr,
            glCurAcq.szMerchantID, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
    {
        memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));
        SaveSysParam();
        strcpy(data->valueStr, glCurAcq.szMerchantID);
    }
    return 0;
}
static int cSetNII(int index, GUI_OPTIONITEM_DATA *data)
{
    GUI_INPUTBOX_ATTR stInputAttr;

    memset(&stInputAttr, 0, sizeof(stInputAttr));
    stInputAttr.bEchoMode = 1;

    LoadNumKeyBoard();
    stInputAttr.nMinLen = 3;
    stInputAttr.nMaxLen = 3;
    Gui_ClearScr();
    if(GUI_OK == Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, strchr(data->szText, '.') + 1, gl_stLeftAttr, glCurAcq.szNii,
            gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
    {
        memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));
        SaveSysParam();
        strcpy(data->valueStr, glCurAcq.szNii);
    }
    return 0;
}
static int cSetBatchNo(int index, GUI_OPTIONITEM_DATA *data)
{
    char szBuff[16+1];
    GUI_INPUTBOX_ATTR stInputAttr;

    memset(&stInputAttr, 0, sizeof(stInputAttr));
    stInputAttr.bEchoMode = 1;

    LoadNumKeyBoard();
    stInputAttr.nMinLen = 1;
    stInputAttr.nMaxLen = 6;
    strcpy((char *)szBuff, data->valueStr);
    Gui_ClearScr();
    if(GUI_OK == Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, strchr(data->szText, '.') + 1, gl_stLeftAttr, szBuff,
            gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
    {
        if (glCurAcq.ulCurBatchNo!=(ulong)atoi(szBuff))
        {
            glCurAcq.ulCurBatchNo  = (ulong)atoi(szBuff);
            glCurAcq.ulNextBatchNo = GetNewBatchNo(glCurAcq.ulCurBatchNo);
            memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));
            SaveSysParam();
            strcpy(data->valueStr, szBuff);
        }
    }
    return 0;
}
static int cSetAcqComm(int index, GUI_OPTIONITEM_DATA *data)
{
    SetAcqTransComm(glSysParam.stTxnCommCfg.ucCommType);
    SetAcqTransComm(glSysParam.stTxnCommCfg.ucCommTypeBak);

    if(CT_TCPIP == glSysParam.stTxnCommCfg.ucCommType ||
       CT_CDMA == glSysParam.stTxnCommCfg.ucCommType ||
       CT_WCDMA == glSysParam.stTxnCommCfg.ucCommType ||   // Added By Gillian 2015/11/23
       CT_GPRS == glSysParam.stTxnCommCfg.ucCommType ||
       CT_WIFI == glSysParam.stTxnCommCfg.ucCommType ||
       CT_TCPIP == glSysParam.stTxnCommCfg.ucCommTypeBak ||
       CT_CDMA == glSysParam.stTxnCommCfg.ucCommTypeBak ||
       CT_WCDMA == glSysParam.stTxnCommCfg.ucCommTypeBak ||
       CT_GPRS == glSysParam.stTxnCommCfg.ucCommTypeBak ||
       CT_WIFI == glSysParam.stTxnCommCfg.ucCommTypeBak)
        if(SetAcqTransEncrypt(&glCurAcq) != 0) // Added By Kim 20150320
        {
            ;
        }
    memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));
    SaveSysParam();
    return 0;
}

static  OPTION_INFO *sg_pstCurOpt = NULL;
static  uchar *sg_psOption = NULL;
static int cSetAuthMode(int index, GUI_OPTIONITEM_DATA *data){
    char szBuff[255];
    sprintf(szBuff, "%s\n%s", strchr(data->szText, '.') + 1, data->valueStr);
    Gui_ClearScr();
    if(data->isChangeable){
        int iOption = (0 == strcmp(data->valueStr, "AUTH") ? 1 : 0);
        if(GUI_OK == Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, strchr(data->szText, '.') + 1, gl_stCenterAttr,
                "AUTH", 1, "PREAUTH", 0, USER_OPER_TIMEOUT, &iOption)){
            uchar ucOptIndex = (uchar)(sg_pstCurOpt[index].uiOptVal>>8);
            uchar ucOptBitNo = (uchar)(sg_pstCurOpt[index].uiOptVal & 0xFF);
            if(1 == data->valueInt)
            {
                if (sg_pstCurOpt[index].ucInverseLogic)
                {
                    sg_psOption[ucOptIndex] &= ~ucOptBitNo;
                }
                else
                {
                    sg_psOption[ucOptIndex] |= ucOptBitNo;
                }
            }
            else
            {
                if (sg_pstCurOpt[index].ucInverseLogic)
                {
                    sg_psOption[ucOptIndex] |= ucOptBitNo;
                }
                else
                {
                    sg_psOption[ucOptIndex] &= ~ucOptBitNo;
                }
            }
            SaveSysParam();
            strcpy(data->valueStr, 1 == iOption ? "AUTH" : "PREAUTH");
        }
        return 0;
    }
    else{
        return Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL);
    }
}

static int cSaveOption(int index, GUI_OPTIONITEM_DATA *data){
    uchar ucOptIndex = (uchar)(sg_pstCurOpt[index].uiOptVal>>8);
    uchar ucOptBitNo = (uchar)(sg_pstCurOpt[index].uiOptVal & 0xFF);
    if(1 == data->valueInt)
    {
        if (sg_pstCurOpt[index].ucInverseLogic)
        {
            sg_psOption[ucOptIndex] &= ~ucOptBitNo;
        }
        else
        {
            sg_psOption[ucOptIndex] |= ucOptBitNo;
        }
    }
    else
    {
        if (sg_pstCurOpt[index].ucInverseLogic)
        {
            sg_psOption[ucOptIndex] |= ucOptBitNo;
        }
        else
        {
            sg_psOption[ucOptIndex] &= ~ucOptBitNo;
        }
    }
    SaveSysParam();
    return 0;
}

// 修改或者查看开关选项
// modify or view options
// Modified by Kim_LinHB 2014-08-18 v1.01.0004
int ModifyOptList(uchar *psOption, uchar ucMode, uchar ucPermission)
{
	// 通过FUN2进入设置时，若定义了FUN2_READ_ONLY，则用户权限为PM_LOW，否则用户权限为PM_MEDIUM
	// 使用无下载方式初始化时，用户权限为PM_HIGH
	// when setting in Function #2, if activate the macro FUN2_READ_ONLY, then user permission is PM_LOW, otherwise it is PM_MEDIUM
	// if initiated with "load default", then user permission is PM_HIGH

	// Protims可控的issuer option列表
	// available issuer options list in Protims
	static OPTION_INFO stIssuerOptList[] =
	{
// 		{"CAPTURE CASH?",		ALLOW_EXTEND_PAY,			FALSE,	PM_MEDIUM},
		{"CAPTURE TXN",			ISSUER_CAPTURE_TXN,			FALSE,	PM_MEDIUM},
		{"ENABLE BALANCE?",		ISSUER_EN_BALANCE,			FALSE,	PM_MEDIUM},
		{"ENABLE ADJUST",		ISSUER_EN_ADJUST,			FALSE,	PM_MEDIUM},
		{"ENABLE OFFLINE",		ISSUER_EN_OFFLINE,			FALSE,	PM_MEDIUM},
		{"ALLOW (PRE)AUTH",		ISSUER_NO_PREAUTH,			TRUE,	PM_MEDIUM},
		{"ALLOW REFUND",		ISSUER_NO_REFUND,			TRUE,	PM_MEDIUM},
		{"ALLOW VOID",			ISSUER_NO_VOID,				TRUE,	PM_MEDIUM},
		{"ENABLE EXPIRY",		ISSUER_EN_EXPIRY,			FALSE,	PM_MEDIUM},
		{"CHECK EXPIRY",		ISSUER_CHECK_EXPIRY,		FALSE,	PM_MEDIUM},
//		{"CHKEXP OFFLINE",		ISSUER_CHECK_EXPIRY_OFFLINE,FALSE,	PM_MEDIUM},
		{"CHECK PAN",			ISSUER_CHKPAN_MOD10,		FALSE,	PM_MEDIUM},
// 		{"CHECK PAN11",			ISSUER_CHKPAN_MOD11,		FALSE,	PM_MEDIUM},
//		{"EN DISCRIPTOR",		ISSUER_EN_DISCRIPTOR,		FALSE,	PM_MEDIUM},
		{"ENABLE MANUAL",		ISSUER_EN_MANUAL,			FALSE,	PM_MEDIUM},
		{"ENABLE PRINT",		ISSUER_EN_PRINT,			FALSE,	PM_MEDIUM},
		{"VOICE REFERRAL",		ISSUER_EN_VOICE_REFERRAL,	FALSE,	PM_MEDIUM},
		{"PIN REQUIRED",		ISSUER_EN_PIN,				FALSE,	PM_HIGH},
#ifdef ISSUER_EN_EMVPIN_BYPASS
		{"EMV PIN BYPASS",		ISSUER_EN_EMVPIN_BYPASS,	FALSE,	PM_MEDIUM},
#endif
//		{"ACCOUNT SELECT",		ISSUER_EN_ACCOUNT_SELECTION,FALSE,	PM_MEDIUM},
//		{"ROC INPUT REQ",		ISSUER_ROC_INPUT_REQ,		FALSE,	PM_MEDIUM},
//		{"DISP AUTH CODE",		ISSUER_AUTH_CODE,			FALSE,	PM_MEDIUM},
//		{"ADDTIONAL DATA",		ISSUER_ADDTIONAL_DATA,		FALSE,	PM_MEDIUM},
		{"4DBC WHEN SWIPE",		ISSUER_SECURITY_SWIPE,		FALSE,	PM_MEDIUM},
		{"4DBC WHEN MANUL",		ISSUER_SECURITY_MANUL,		FALSE,	PM_MEDIUM},
		{NULL, 0, FALSE, PM_DISABLE},
	};

	// Protims可控的acquirer option列表
	// available acquirer options list in Protims
	static OPTION_INFO stAcqOptList[] =
	{
		{"ONLINE VOID",			ACQ_ONLINE_VOID,			FALSE,	PM_MEDIUM},
		{"ONLINE REFUND",		ACQ_ONLINE_REFUND,			FALSE,	PM_MEDIUM},
		{"EN. TRICK FEED",		ACQ_DISABLE_TRICK_FEED,		TRUE,	PM_MEDIUM},
//		{"ADDTION PROMPT",		ACQ_ADDTIONAL_PROMPT,		FALSE,	PM_MEDIUM},
		{"AMEX ACQUIRER",		ACQ_AMEX_SPECIFIC_FEATURE,	FALSE,	PM_HIGH},
		{"DBS FEATURE",			ACQ_DBS_FEATURE,			FALSE,	PM_MEDIUM},
		{"BOC INSTALMENT",		ACQ_BOC_INSTALMENT_FEATURE,	FALSE,	PM_MEDIUM},
		{"CITI INSTALMENT",		ACQ_CITYBANK_INSTALMENT_FEATURE,FALSE,	PM_MEDIUM},
#ifdef ENABLE_EMV
		{"EMV ACQUIRER",		ACQ_EMV_FEATURE,			FALSE,	PM_HIGH},
#endif
		{NULL, 0, FALSE, PM_DISABLE},
	};

	// Protims不可控的acquirer option列表
	// invalid in Protims
	static OPTION_INFO stAcqExtOptList[] =
	{
		// 因为只能在且必须在POS上修改，因此权限设为PM_LOW
		// this options list can only be modified on POS, so user permission is set as PM_LOW
		{NULL, 0, FALSE, PM_DISABLE},
	};

	// Protims可控的edc option列表
	// available EDC options list in Protims
	static OPTION_INFO stEdcOptList[] =
	{
//		{"AUTH PAN MASKING",	EDC_AUTH_PAN_MASKING,	FALSE,	PM_LOW},
//		{"SELECT ACQ_CARD",		EDC_SELECT_ACQ_FOR_CARD,FALSE,	PM_LOW},
//		{"ENABLE ECR",			EDC_ECR_ENABLE,			FALSE,	PM_MEDIUM},
		{"FREE PRINT",			EDC_FREE_PRINT,			FALSE,  PM_LOW},
		{"EN. INSTALMENT?",		EDC_ENABLE_INSTALMENT,	FALSE,	PM_MEDIUM},
		{"CAPTURE CASH",		EDC_CASH_PROCESS,		FALSE,	PM_MEDIUM},
		{"REFERRAL DIAL",		EDC_REFERRAL_DIAL,		FALSE,	PM_MEDIUM},
		{"AUTH MODE",			EDC_AUTH_PREAUTH,		FALSE,	PM_MEDIUM},
//		{"PRINT TIME",			EDC_PRINT_TIME,			FALSE,	PM_MEDIUM},
		{"TIP PROCESSING",		EDC_TIP_PROCESS,		FALSE,	PM_MEDIUM},
//		{"USE PRINTER",			EDC_USE_PRINTER,		FALSE,	PM_MEDIUM},
		{"NEED ADJUST PWD",		EDC_NOT_ADJUST_PWD,		TRUE,	PM_HIGH},
		{"NEED SETTLE PWD",		EDC_NOT_SETTLE_PWD,		TRUE,	PM_HIGH},
		{"NEED REFUND PWD",		EDC_NOT_REFUND_PWD,		TRUE,	PM_HIGH},
		{"NEED VOID PWD",		EDC_NOT_VOID_PWD,		TRUE,	PM_HIGH},
		{"NEED MANUAL PWD",		EDC_NOT_MANUAL_PWD,		TRUE,	PM_HIGH},
//		{"LOCKED EDC",			EDC_NOT_KEYBOARD_LOCKED,TRUE,	PM_MEDIUM},
		{NULL, 0, FALSE, PM_DISABLE},
	};

	// Protims不可控的edc option列表
	// invalid in Protims
	static OPTION_INFO stEdcExtOptList[] =
	{
		// 因为只能在且必须在POS上修改，因此权限设为PM_LOW
		// this options list can only be modified on POS, so user permission is set as PM_LOW
		{NULL, 0, FALSE, PM_DISABLE},
	};

	uchar			ucCnt, ucOptIndex, ucOptBitNo;
    GUI_OPTION stOption;
    GUI_OPTIONITEM stOptItem[50];

    sg_psOption = psOption;

	switch(ucMode)
	{
	case 'I':
	case 'i':
		sg_pstCurOpt = (OPTION_INFO *)stIssuerOptList;
		break;
	case 'E':
	    sg_pstCurOpt = (OPTION_INFO *)stEdcOptList;
		break;
	case 'e':
	    sg_pstCurOpt = (OPTION_INFO *)stEdcExtOptList;
		break;
	case 'A':
	    sg_pstCurOpt = (OPTION_INFO *)stAcqOptList;
		break;
	case 'a':
	    sg_pstCurOpt = (OPTION_INFO *)stAcqExtOptList;
		break;
	default:
	    sg_pstCurOpt = NULL;
		break;
	}

	if( !sg_pstCurOpt || sg_pstCurOpt->pText==NULL )
	{
	    Gui_ClearScr();
        PubBeepErr();
        Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("UNSUPPORTED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
		return 0;
	}

	ucCnt = 0;
	while( 1 )
	{
	    stOptItem[ucCnt].data.isChangeable = ucPermission >= sg_pstCurOpt[ucCnt].ucPermissionLevel;
        ucOptIndex = (uchar)(sg_pstCurOpt[ucCnt].uiOptVal>>8);
        ucOptBitNo = (uchar)(sg_pstCurOpt[ucCnt].uiOptVal & 0xFF);
        if (sg_pstCurOpt[ucCnt].ucInverseLogic)
        {
            stOptItem[ucCnt].data.valueInt = (psOption[ucOptIndex] & ucOptBitNo) ? 0 : 1;
        }
        else
        {
            stOptItem[ucCnt].data.valueInt = (psOption[ucOptIndex] & ucOptBitNo) ? 1 : 0;
        }

	    if(EDC_AUTH_PREAUTH != sg_pstCurOpt[ucCnt].uiOptVal){
	        stOptItem[ucCnt].data.type = OptItemSwitcher;
	        stOptItem[ucCnt].vFunc = cSaveOption;
	    }
	    else{
	        stOptItem[ucCnt].data.type = OptItemCaller;
	        stOptItem[ucCnt].vFunc = cSetAuthMode;
	        if(1 == stOptItem[ucCnt].data.valueInt){
	            strcpy(stOptItem[ucCnt].data.valueStr, "AUTH");
	        }
	        else{
	            strcpy(stOptItem[ucCnt].data.valueStr, "PREAUTH");
	        }
	    }

        sprintf((char *)stOptItem[ucCnt].data.szText, "%d.%.16s", ucCnt+1, (char *)sg_pstCurOpt[ucCnt].pText);
        if( sg_pstCurOpt[ucCnt+1].pText==NULL )
        {
            strcpy(stOptItem[ucCnt+1].data.szText, "");
            break;
        }
        ucCnt++;
	}

    Gui_BindOption(GetCurrTitle(), gl_stTitleAttr, gl_stLeftAttr, "./res/On.png", "./res/Off.png",
            (GUI_OPTIONITEM *)stOptItem, &stOption);

    if( GUI_OK != Gui_ShowOptionList(&stOption, USER_OPER_TIMEOUT))
    {
        return ERR_USERCANCEL;
    }

	return 0;
}

// 修改口令
// change passwords
int ChangePassword(void)
{
	GUI_MENU stChgPwdMenu;
	GUI_MENUITEM stDefChgPwdMenuItem[] =
	{
		{ _T_NOOP("TERMINAL   PWD"), 1,TRUE,    ModifyPasswordTerm},
		{ _T_NOOP("BANK       PWD"), 2,TRUE,    ModifyPasswordBank},
		{ _T_NOOP("MERCHANT   PWD"), 3,TRUE,    ModifyPasswordMerchant},
		{ _T_NOOP("VOID       PWD"), 4,TRUE,    ModifyPasswordVoid},
		{ _T_NOOP("REFUND     PWD"), 5,TRUE,    ModifyPasswordRefund},
		{ _T_NOOP("ADJUST     PWD"), 6,TRUE,    ModifyPasswordAdjust},
		{ _T_NOOP("SETTLE     PWD"), 7,TRUE,    ModifyPasswordSettle},
		{ "", -1,FALSE,  NULL},
	};

	GUI_MENUITEM stChgPwdMenuItem[20];
	int iMenuItemNum = 0;
	int i;
	for(i = 0; i < sizeof(stDefChgPwdMenuItem)/sizeof(GUI_MENUITEM); ++i){
	    if(stDefChgPwdMenuItem[i].bVisible)
        {
	        memcpy(&stChgPwdMenuItem[iMenuItemNum], &stDefChgPwdMenuItem[i], sizeof(GUI_MENUITEM));
            sprintf(stChgPwdMenuItem[iMenuItemNum].szText, "%d.%s", iMenuItemNum+1, stDefChgPwdMenuItem[i].szText);
            ++iMenuItemNum;
        }
	}

	stChgPwdMenuItem[iMenuItemNum].szText[0] = 0;


	Gui_BindMenu(_T("CHANGE PWD"), gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stChgPwdMenuItem, &stChgPwdMenu);
	Gui_ClearScr();
	Gui_ShowMenuList(&stChgPwdMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, NULL);
	return 0;
}

// 手工设置系统时间
// set system time manually
int SetSysTime(void)
{
    const uchar *title = _T("SET TIME");
	uchar	szBuff[14+1], sInputTime[6];
	int iRet;

	memset(szBuff,0,sizeof(szBuff));
	strcpy(szBuff + 10, "00"); //ss

	while(1){
        Gui_ClearScr();
        memset(szBuff,0,sizeof(szBuff));
        iRet = Gui_ShowTimeBox(title, gl_stTitleAttr, szBuff, gl_stCenterAttr, 0, USER_OPER_TIMEOUT);

        //EPT-9 Kim
        if(GUI_OK == iRet){
            if(memcmp(szBuff, "2038", 4) >= 0 || memcmp(szBuff, "2000", 4) < 0){
                Gui_ShowMsgBox(title, gl_stTitleAttr, _ICON_FAIL, _T("OVER LIMIT"), gl_stCenterAttr, GUI_BUTTON_NONE, 5, NULL);
                continue;
            }

            Gui_ClearScr();
            iRet = Gui_ShowTimeBox(title, gl_stTitleAttr, szBuff + 8, gl_stCenterAttr, 1, USER_OPER_TIMEOUT); //EPT-9 Kim
        }
        else{
            return ERR_NO_DISP;
        }

        if (GUI_OK == iRet)
        {
            PubAsc2Bcd(szBuff+2, 12, sInputTime); //EPT-9 Kim
            iRet = SetTime(sInputTime);
            if(iRet != 0){
                if(GUI_OK != Gui_ShowMsgBox(title, gl_stTitleAttr, _ICON_FAIL, _T("Failed to set\nTry again?"), gl_stCenterAttr, GUI_BUTTON_YandN, USER_OPER_TIMEOUT, NULL))
                    break;
            }
            else
                break;
        }
	}
	return 0;
}

// 输入参数自动更新时间
// set timer for update parameters automatically
int SetCallInTime(void)
{
    return 0;
}

// TRUE:判断时间是否合法
// TRUE:it is a valid time
uchar IsValidTime(const uchar *pszTime)
{
    int     i, iHour, iMinute;

    for(i=0; i<4; i++)
    {
        if( pszTime[i]<'0' || pszTime[i]>'9' )
        {
            return FALSE;
        }
    }

    iHour   = (int)PubAsc2Long(pszTime, 2);
    iMinute = (int)PubAsc2Long(pszTime+2, 2);
    if( iHour>24 || iMinute>59 )
    {
        return FALSE;
    }
    if( iHour==24 && iMinute!=0 )
    {
        return FALSE;
    }

    return TRUE;
}

// provide manual select and prompt message when pszLngName==NULL
// mode:
// 0--auto load the first available non-english language (if language file available)
// 1--auto load the last time used language
// 2--provide a menu for selection
// Modified by Kim_LinHB 2014-8-7 v1.01.0002
//EPT-13, EPT-14 Kim
static char sg_lastLang[16+1] = {0};
void SetSysLang(uchar ucSelectMode)
{
	int	iCnt, iTotal, iRet, iSel = 0;

	GUI_MENU stLangMenu;
	GUI_MENUITEM stLangMenuItem[32];

	if (ucSelectMode==0 || ucSelectMode==2)
	{
		// 搜寻已下载的语言文件，准备菜单
		// search the existed translation files, and prepare the menu list
		for (iCnt=0, iTotal=0;
			iCnt<sizeof(stLangMenuItem)/sizeof(stLangMenuItem[0])-1;
			iCnt++)
		{
			if (glLangList[iCnt].szDispName[0]==0)
			{
				break;
			}
			if ((iCnt==0) || (fexist((char *)glLangList[iCnt].szFileName)>=0
					&& fexist((char *)glLangList[iCnt].szFilePath)>=0 ))
			{
				sprintf(stLangMenuItem[iTotal].szText, "%d.%s", iTotal + 1, _T((char *)glLangList[iCnt].szDispName));
				stLangMenuItem[iTotal].bVisible = TRUE;
				stLangMenuItem[iTotal].nValue = iCnt; // EPT-7
				stLangMenuItem[iTotal].vFunc = NULL;
				if(0 == strcmp(glLangList[iCnt].szDispName, glSysParam.stEdcInfo.stLangCfg.szDispName))
					iSel = iCnt;	// EPT-7

				iTotal++;
			}
		}

		strcpy(stLangMenuItem[iTotal].szText, "");
		stLangMenuItem[iTotal].bVisible = FALSE;
		stLangMenuItem[iTotal].nValue = -1;
		stLangMenuItem[iTotal].vFunc = NULL;

		if (ucSelectMode==0)
		{
			// 首次加载
			// 如果有一个或多个非英文语言，自动选择第一个；否则选择英语
			// the first time loading 
			// if there are one or more than one translation files, then will select the first non-English language as default,
			// otherwise set English as default

			iSel = ((iTotal>1) ? stLangMenuItem[1].nValue : stLangMenuItem[0].nValue);
		}
		else
		{
			// 菜单手动选择
			// display a language menu list to select manually
			Gui_BindMenu(GetCurrTitle(), gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stLangMenuItem, &stLangMenu);
			Gui_ClearScr();
			iRet = Gui_ShowMenuList(&stLangMenu, GUI_MENU_DIRECT_RETURN, 60, &iSel);
			if (iRet != GUI_OK)
			{
				return;
			}
		}

        if(0 == strcmp(sg_lastLang, glLangList[iSel].szDispName))
            return;
	}
	else{
	    if(0 == strcmp(sg_lastLang, glSysParam.stEdcInfo.stLangCfg.szDispName))
	        return;
	}

	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, _T("Loading Language..."), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

    if (ucSelectMode==0 || ucSelectMode==2){
        glSysParam.stEdcInfo.stLangCfg = glLangList[iSel];
    }

	iRet = SetLng(glSysParam.stEdcInfo.stLangCfg.szFileName);
	if (iSel != 0 && iRet!=0)
	{
	    Gui_ClearScr();
	    Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("Failed to set\nLanguage"), gl_stCenterAttr, GUI_BUTTON_OK, 5, NULL);
		glSysParam.stEdcInfo.stLangCfg = glLangList[0];
	}
	FtFontFree();
    FtFontLoad(glSysParam.stEdcInfo.stLangCfg.szFilePath);
    _GUI_INIT_(glSysParam.stEdcInfo.stLangCfg.szFilePath);
    ChangeLangOrder();
    strcpy(sg_lastLang, glSysParam.stEdcInfo.stLangCfg.szDispName);
}

// Set system language
int SetEdcLang(void)
{
	LANG_CONFIG	stLangBak;

	memcpy(&stLangBak, &glSysParam.stEdcInfo.stLangCfg, sizeof(LANG_CONFIG));

	SetCurrTitle(_T("SELECT LANG")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	SetSysLang(2);

	if (memcmp(&stLangBak, &glSysParam.stEdcInfo.stLangCfg, sizeof(LANG_CONFIG)) != 0)
	{
		SaveEdcParam();
	}
	return 0;
}

#ifndef APP_MANAGER_VER
void SetEdcLangExt(const char *pszDispName)
{
	int	ii;
	for (ii=0; glLangList[ii].szDispName[0]!=0; ii++)
	{
		if (PubStrNoCaseCmp((uchar *)glLangList[ii].szDispName, pszDispName)==0)
		{
			if ((ii==0) || (fexist((char *)glLangList[ii].szFileName)>=0))
			{
				glSysParam.stEdcInfo.stLangCfg = glLangList[ii];
				SetSysLang(1);
			}
		}
	}
}
#endif

int SetPowerSave(void)
{
    int iSel = glSysParam.stEdcInfo.ucIdleShutdown;
	uchar	ucTemp, szPrompt[100], szBuff[100];
	int		iRet;

	SetCurrTitle(_T("POWERSAVE OPTION")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493
	if (ERR_SYS_NOT_SUPPORT  == OsCheckBattery()) //bug 803
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_WARNING, _T("UNSUPPORTED"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
		return ERR_NO_DISP;
	}
	else
	{
		GUI_INPUTBOX_ATTR stInputAttr;
		memset(&stInputAttr, 0, sizeof(stInputAttr));

		sprintf((char *)szBuff, _T("IDLE: SLEEP"));
		if (glSysParam.stEdcInfo.ucIdleShutdown)
		{
			sprintf((char *)szBuff, _T("IDLE: SHUTDOWN  "));
		}
		Gui_ClearScr();
		iRet = Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, szBuff, gl_stCenterAttr, 
			_T("SLEEP"), 0, _T("SHUTDOWN"), 1, USER_OPER_TIMEOUT, &iSel);
		
		if(GUI_OK == iRet){
		    glSysParam.stEdcInfo.ucIdleShutdown = iSel;
			if (1 == glSysParam.stEdcInfo.ucIdleShutdown &&
				glSysParam.stEdcInfo.ucIdleMinute<5)
			{
				glSysParam.stEdcInfo.ucIdleMinute = 5;
			}
			SaveSysParam();
		}
		else{
			return ERR_NO_DISP;
		}

		LoadNumKeyBoard();

		ucTemp = glSysParam.stEdcInfo.ucIdleMinute;

		stInputAttr.nMinLen = 1;
		stInputAttr.nMaxLen = 2;
		stInputAttr.bEchoMode = 1;

		if (glSysParam.stEdcInfo.ucIdleShutdown)
		{
			sprintf(szPrompt, "%s[5-60mins]", _T("SHUTDOWN TIME OUT"));
		}
		else
		{
			sprintf(szPrompt, "%s[1-60mins]", _T("PWR SAVE TIME OUT"));
		}

		while (1)
		{
			Gui_ClearScr();
			sprintf((char *)szBuff, "%d", (int)ucTemp);
			iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, szPrompt, gl_stLeftAttr,
				szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
			if (iRet !=GUI_OK)
			{
				return ERR_NO_DISP;
			}
			ucTemp = (uchar)atol((char *)szBuff);
			if (ucTemp>60 || ucTemp<1)
			{
				continue;
			}
			if (glSysParam.stEdcInfo.ucIdleShutdown && (ucTemp<5))
			{
				continue;
			}

			if (glSysParam.stEdcInfo.ucIdleMinute!=ucTemp)
			{
				glSysParam.stEdcInfo.ucIdleMinute = ucTemp;
				SaveSysParam();
			}
			break;
		}
	}
	return 0;
}

int TestMagicCard1(void)
{
	TestMagicCard(1);
	return 0;
}

int TestMagicCard2(void)
{
	TestMagicCard(2);
	return 0;
}

int TestMagicCard3(void)
{
	TestMagicCard(3);
	return 0;
}

void TestMagicCard(int iTrackNum)
{
	uchar	ucRet;
	uchar	szMagTrack1[79+1], szMagTrack2[40+1], szMagTrack3[104+1];
	uchar	szTitle[16+1], szBuff[200];

	MagClose();
	MagOpen();
	MagReset();
	while( 1 )
	{
		sprintf((char *)szTitle, "TRACK %d TEST", iTrackNum);
		Gui_ClearScr();
		Gui_ShowMsgBox(szTitle, gl_stTitleAttr, NULL, _T("PLS SWIPE CARD"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 0, NULL);
		while( 2 )
		{
			if( 0 == kbhit() && getkey()==KEYCANCEL )
			{
				MagClose();
				return;
			}

			if( MagSwiped()==0 )
			{
				break;
			}
		}

		memset(szMagTrack1, 0, sizeof(szMagTrack1));
		memset(szMagTrack2, 0, sizeof(szMagTrack2));
		memset(szMagTrack3, 0, sizeof(szMagTrack3));
		ucRet = MagRead(szMagTrack1, szMagTrack2, szMagTrack3);
		
		if( iTrackNum==1 )
		{
			sprintf(szBuff, "RET:%02X\n%.21s\n Length=[%d]", ucRet,
				szMagTrack1[0]==0 ? (uchar *)"NULL" : szMagTrack1, strlen((char *)szMagTrack1));
		}
		else if (iTrackNum == 2)
		{
			sprintf(szBuff, "RET:%02X\n%.21s\n Length=[%d]", ucRet,
				szMagTrack2[0]==0 ? (uchar *)"NULL" : szMagTrack2, strlen((char *)szMagTrack2));
		}
		else
		{
			sprintf(szBuff, "RET:%02X\n%.21s\n Length=[%d]", ucRet,
				szMagTrack3[0]==0 ? (uchar *)"NULL" : szMagTrack3, strlen((char *)szMagTrack3));
		}

		Gui_ClearScr();
		if(GUI_OK != Gui_ShowMsgBox(szTitle, gl_stTitleAttr, NULL, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, USER_OPER_TIMEOUT, NULL))
		{
			return;
		}
	}
}

int ToolsViewPreTransMsg(void)
{
	GUI_MENU stViewMsgMenu;
	GUI_MENUITEM stViewMsgMenuItem[] =
	{
		{ "1.OUTPUT SEND/RECV", 1,TRUE,  ShowExchangePack},
		{ "2.PRINT SEND/RECV", 2,TRUE,  PrnExchangePack},
		{ "", -1,FALSE,  NULL},
	};

	SetCurrTitle(_T("VIEW MSG"));
	if( PasswordBank()!=0 )
	{
		return ERR_NO_DISP;
	}

	Gui_BindMenu(GetCurrTitle(), gl_stTitleAttr, gl_stLeftAttr, (GUI_MENUITEM *)stViewMsgMenuItem, &stViewMsgMenu);
	Gui_ClearScr();
	Gui_ShowMenuList(&stViewMsgMenu, 0, USER_OPER_TIMEOUT, NULL);
	return 0;
}

// 发送通讯报文到COM1
// send comm package to COM1
int ShowExchangePack(void)
{
#define DEBUG_OUT_PORT	0
	if (!glSendData.uiLength && !glRecvData.uiLength)
	{
		DispErrMsg(_T("NO DATA"), NULL, 5, 0);
		return ERR_NO_DISP;
	}
	
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, _T("SENDING..."), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	DebugNacTxd(DEBUG_OUT_PORT, glSendData.sContent, glSendData.uiLength);
	DelayMs(2000);
	DebugNacTxd(DEBUG_OUT_PORT, glRecvData.sContent, glRecvData.uiLength);

	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_SUCCESS, _T("SEND OK"), gl_stCenterAttr, GUI_BUTTON_OK, 2, NULL);
	return 0;
}

// 打印通讯报文
// Print comm package
int PrnExchangePack(void)
{
	SetCurrTitle(_T("PRINT MSG"));
	if (!glSendData.uiLength && !glRecvData.uiLength)
	{
		DispErrMsg(_T("NO DATA"), NULL, 5, 0);
		return ERR_NO_DISP;
	}

	
	// Modified by Kim_LinHB 2014-8-11 v1.01.0003
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

//	PubDebugOutput(_T("VIEW MSG"), glSendData.sContent, glSendData.uiLength,
//					DEVICE_PRN, ISO_MODE);
//	PubDebugOutput(_T("VIEW MSG"), glRecvData.sContent, glRecvData.uiLength,
//					DEVICE_PRN, ISO_MODE);
    PubDebugOutput(_T("VIEW MSG[Request]"), glSendData.sContent, glSendData.uiLength,
                    DEVICE_PRN, HEX_MODE);
    PubDebugOutput(_T("VIEW MSG[Response]"), glRecvData.sContent, glRecvData.uiLength,
                    DEVICE_PRN, HEX_MODE);

	Gui_ClearScr();
	return 0;
}

void DebugNacTxd(uchar ucPortNo, const uchar *psTxdData, ushort uiDataLen)
{
	uchar	*psTemp, sWorkBuf[LEN_MAX_COMM_DATA+10];
	uchar  ucInit = 0;
	
	if( uiDataLen>LEN_MAX_COMM_DATA )
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, _T("INVALID PACK"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 2, NULL);
		return;
	}

	sWorkBuf[0] = STX;
	sWorkBuf[1] = (uiDataLen/1000)<<4    | (uiDataLen/100)%10;	// convert to BCD
	sWorkBuf[2] = ((uiDataLen/10)%10)<<4 | uiDataLen%10;
	memcpy(&sWorkBuf[3], psTxdData, uiDataLen);
	sWorkBuf[3+uiDataLen]   = ETX;

	//sWorkBuf[3+uiDataLen+1] = PubCalcLRC(psTxdData, uiDataLen, (uchar)(sWorkBuf[1] ^ sWorkBuf[2] ^ ETX));
	PubCalcLRC(sWorkBuf + 1, (ushort)(uiDataLen+3), &ucInit);
	sWorkBuf[3+uiDataLen+1] = ucInit;
	//end
	uiDataLen += 5;

	PortClose(ucPortNo);
	PortOpen(ucPortNo, (void *)"9600,8,n,1");
	psTemp = sWorkBuf;
	while( uiDataLen-->0 )
	{
		if( PortSend(ucPortNo, *psTemp++)!=0 )
		{
			break;
		}
	}
	PortClose(ucPortNo);
}




int GetIpLocalWifiSettings(void *pstParam)
{
	int 	iRet;
	ST_WIFI_PARAM *pstWifiPara;
	TCPIP_PARA stLocalInfo;

	pstWifiPara = (ST_WIFI_PARAM *)pstParam;
	memset(&stLocalInfo, 0, sizeof(TCPIP_PARA));

	iRet = GetIPAddress((uchar *)"LOCAL IP", TRUE, stLocalInfo.szLocalIP);
	if( iRet!=0 )
	{
		return iRet;
	}
	SplitIpAddress(stLocalInfo.szLocalIP, pstWifiPara->Ip);
	
	iRet = GetIPAddress((uchar *)"IP MASK", TRUE, stLocalInfo.szNetMask);
	if( iRet!=0 )
	{
		return iRet;
	}
	SplitIpAddress(stLocalInfo.szNetMask, pstWifiPara->Mask);
	
	iRet = GetIPAddress((uchar *)"GATEWAY IP", TRUE, stLocalInfo.szGatewayIP);
	if( iRet!=0 )
	{
		return iRet;
	}
	SplitIpAddress(stLocalInfo.szGatewayIP, pstWifiPara->Gate);
	
	iRet = GetIPAddress((uchar *)"DNS", TRUE, stLocalInfo.szDNSIP);
	if( iRet!=0 )
	{
		return iRet;
	}
	SplitIpAddress(stLocalInfo.szDNSIP, pstWifiPara->Dns);
	
	return 0;
}

// Modified by Kim_LinHB 2014-08-19 v1.01.0004
int SetWiFiApp(void* pstParam)
{
	int iRet = -1;
	WIFI_PARA *pstWifiPara = (WIFI_PARA *)pstParam;
	ST_WifiApInfo *stWiFiApp;

	unsigned char szPWD[(2 * KEY_WEP_LEN_MAX ) > KEY_WPA_MAXLEN ? (2 * KEY_WEP_LEN_MAX ) : KEY_WPA_MAXLEN];
	
	int	iMenuNo;
	uchar ucCnt;
	int iSel;

	GUI_MENU	stWiFiAppsMenu;
	GUI_MENUITEM	stWiFiAppsMenuItem[MAX_WiFiApp+1];
	GUI_INPUTBOX_ATTR stInputAttr;

	// Modified by Kim for Prolin2.4.31 OsWifiCheck doesn't return not_conenct if the dev is not open,
	// so don't need to check before open it.
	WifiOpen(); // ignore the failed case

	WifiDisconnect();

	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, "WIFI SCANNING...", gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	iRet = OsWifiScan(&stWiFiApp);

	if(iRet < 0)
	{
		return iRet;
	}

	if(0 == iRet)
	{
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, "NOT FIND APPS", gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return ERR_USERCANCEL;
	}

	//sort
	{
		int i,j;
		ST_WifiApInfo temp;
		for(j=0;j<iRet-1;j++)
			for(i=0;i<iRet-1-j;i++)
			{
				if(stWiFiApp[i].Rssi<stWiFiApp[i+1].Rssi)
				{
					memcpy(&temp, &stWiFiApp[i], sizeof(ST_WifiApInfo));
					memcpy(&stWiFiApp[i], &stWiFiApp[i+1], sizeof(ST_WifiApInfo));
					memcpy(&stWiFiApp[i+1], &temp, sizeof(ST_WifiApInfo));
				}
			}
	}

	memset(stWiFiAppsMenuItem,0,sizeof(stWiFiAppsMenuItem));
	for(ucCnt=0; ucCnt< iRet && ucCnt < MAX_WiFiApp; ucCnt++)
	{
		sprintf((char *)stWiFiAppsMenuItem[ucCnt].szText, "%s lv:%d", stWiFiApp[ucCnt].Essid, (99+stWiFiApp[ucCnt].Rssi)/20);
		stWiFiAppsMenuItem[ucCnt].bVisible = TRUE;
		stWiFiAppsMenuItem[ucCnt].nValue = ucCnt;
		stWiFiAppsMenuItem[ucCnt].vFunc = NULL;
	}

	Gui_BindMenu(GetCurrTitle(), gl_stTitleAttr, gl_stLeftAttr, stWiFiAppsMenuItem, &stWiFiAppsMenu);

	Gui_ClearScr();
	iMenuNo = 0;
	if(GUI_OK != Gui_ShowMenuList(&stWiFiAppsMenu, GUI_MENU_DIRECT_RETURN, USER_OPER_TIMEOUT, &iMenuNo))
	{
		return ERR_USERCANCEL;
	}

	memcpy(&pstWifiPara->stParam.stInfo, &stWiFiApp[iMenuNo], sizeof(ST_WifiApInfo));
	snprintf(pstWifiPara->stLastAP.Essid, 32, pstWifiPara->stParam.stInfo.Essid);
	strcpy(pstWifiPara->stLastAP.Bssid, pstWifiPara->stParam.stInfo.Bssid);
	pstWifiPara->stLastAP.Channel = pstWifiPara->stParam.stInfo.Channel;
	pstWifiPara->stLastAP.Mode = pstWifiPara->stParam.stInfo.Mode;
	pstWifiPara->stLastAP.AuthMode = pstWifiPara->stParam.stInfo.AuthMode;
	pstWifiPara->stLastAP.SecMode = pstWifiPara->stParam.stInfo.SecMode;

	LoadMixKeyBoard();

	memset(&stInputAttr, 0, sizeof(stInputAttr));
	stInputAttr.bEchoMode = 1;
	stInputAttr.bSensitive = 1;

	memset(szPWD, 0, sizeof(szPWD));
	memset(&pstWifiPara->stLastAP.KeyUnion, 0, sizeof(pstWifiPara->stLastAP.KeyUnion));
	if(pstWifiPara->stLastAP.AuthMode == AUTH_NONE_WEP)
	{   
		stInputAttr.nMinLen = 0;
		stInputAttr.nMaxLen = 2 * KEY_WEP_LEN_MAX;

		// Added by Kim_LinHB 2014-11-27
		do{
			int len;
			Gui_ClearScr();
			if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("Enter PassWord:"), gl_stLeftAttr, 
				szPWD, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
			{
				return ERR_USERCANCEL;
			}
			len =strlen(szPWD);

			if(len != 10 && len != 24 && len != 32){
				Gui_ClearScr();
				Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, "Wrong Length", gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
				memset(szPWD, 0 ,sizeof(szPWD));
			}
			else{
				break;
			}
		}while(1);
		PubAsc2Bcd(szPWD, strlen(szPWD), pstWifiPara->stLastAP.KeyUnion.WepKey.Key[0]);
		pstWifiPara->stLastAP.KeyUnion.WepKey.Idx = 0;
		pstWifiPara->stLastAP.KeyUnion.WepKey.KeyLen = strlen(szPWD);
	}

	// WLAN_SEC_WPA_WPA2 =2       WLAN_SEC_WPAPSK_WPA2PSK= 3
	if (pstWifiPara->stLastAP.AuthMode == AUTH_WPA_PSK ||
		pstWifiPara->stLastAP.AuthMode ==  AUTH_WPA_EAP ||
		pstWifiPara->stLastAP.AuthMode == AUTH_WPA_WPA2_PSK ||
		pstWifiPara->stLastAP.AuthMode == AUTH_WPA_WPA2_EAP ||
		pstWifiPara->stLastAP.AuthMode == AUTH_WPA2_PSK ||
		pstWifiPara->stLastAP.AuthMode == AUTH_WPA2_EAP)
	{
		stInputAttr.nMinLen = 0;
		stInputAttr.nMaxLen = KEY_WPA_MAXLEN;
		Gui_ClearScr();
		if(GUI_OK != Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("Enter PassWord:"), gl_stLeftAttr, 
			szPWD, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}
		if(pstWifiPara->stLastAP.AuthMode ==  AUTH_WPA_EAP ||
			pstWifiPara->stLastAP.AuthMode ==  AUTH_WPA_WPA2_EAP ||
			pstWifiPara->stLastAP.AuthMode ==  AUTH_WPA2_EAP)
		{
			memcpy(pstWifiPara->stLastAP.KeyUnion.EapKey.Pwd, szPWD, strlen(szPWD));
			//FIXME Kim if need to set Cert
		}
		else
		{
			memcpy(pstWifiPara->stLastAP.KeyUnion.PskKey.Key, szPWD, strlen(szPWD));
			pstWifiPara->stLastAP.KeyUnion.PskKey.KeyLen = strlen(szPWD);
		}
	}
	
	iSel = pstWifiPara->stParam.DhcpEnable;
	if(iSel > 1)
	    iSel = 1;

	Gui_ClearScr();
	if(GUI_OK != Gui_ShowAlternative(GetCurrTitle(), gl_stTitleAttr, _T("DHCP ENABLE"), gl_stCenterAttr,
		"ON", 1, "OFF", 0, USER_OPER_TIMEOUT, &iSel))
	{
		return ERR_USERCANCEL;
	}

	if(1 == iSel)
	{
		pstWifiPara->stParam.DhcpEnable = 1;
	}
	else
	{
		pstWifiPara->stParam.DhcpEnable = 0;
	}
	
	if(pstWifiPara->stParam.DhcpEnable == 0)
	{
		iRet = GetIpLocalWifiSettings(&pstWifiPara->stParam);
		if( iRet!=0 )
		{
			return iRet;
		}
	}
	return 0;
}

// Modified by Kim_LinHB 2014-08-19 v1.01.0004
void SyncWifiParam(void *pstDst, const void *pstSrc)
{
	memcpy(&((WIFI_PARA *)pstDst)->stHost1,   &((WIFI_PARA *)pstSrc)->stHost1, sizeof(IP_ADDR));
	memcpy(&((WIFI_PARA *)pstDst)->stHost2,   &((WIFI_PARA *)pstSrc)->stHost2, sizeof(IP_ADDR));

	((WIFI_PARA *)pstDst)->stParam.DhcpEnable = ((WIFI_PARA *)pstSrc)->stParam.DhcpEnable;
	strcpy((char *)(((WIFI_PARA *)pstDst)->stParam.Ip),   (char *)(((WIFI_PARA *)pstSrc)->stParam.Ip));
	strcpy((char *)(((WIFI_PARA *)pstDst)->stParam.Mask),   (char *)(((WIFI_PARA *)pstSrc)->stParam.Mask));
	strcpy((char *)(((WIFI_PARA *)pstDst)->stParam.Gate),   (char *)(((WIFI_PARA *)pstSrc)->stParam.Gate));
	strcpy((char *)(((WIFI_PARA *)pstDst)->stParam.Dns),   (char *)(((WIFI_PARA *)pstSrc)->stParam.Dns));

	memcpy((char *)(&((WIFI_PARA *)pstDst)->stParam.stInfo), (char *)(&((WIFI_PARA *)pstSrc)->stParam.stInfo), sizeof(ST_WifiApInfo));
	strcpy((char *)(&((WIFI_PARA *)pstDst)->stLastAP),   (char *)(&((WIFI_PARA *)pstSrc)->stLastAP));
}

void DispWifiErrorMsg( int Ret)
{
	unsigned char szBuff[100];
	sprintf(szBuff, "%d", Ret);
	OsWifiClose();
	Gui_ClearScr();
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
	return ;
}

// end of file

