
#include "global.h"

/********************** Internal macros declaration ************************/

/********************** Internal structure declaration *********************/

/********************** Internal functions declaration *********************/
static void SetSystemParam(void);
static void SetSystemParamSub(uchar ucPermission);
static int  SetCommType(uchar ucMode);
static void SetSysCommParam(uchar ucPermission);
static int  SetCommDetails(uchar *pucCommType);
static int  SetPabx(void);
static int  SetModemParam(void);
static int  SetTcpIpSharedPara(COMM_CONFIG *pstCommCfg);
static void SetAcqParam(uchar ucPermission);
#ifdef SXX_IP_MODULE
static int  SetTcpIpParam_S80(TCPIP_PARA *pstParam);
#endif
static int  GetIPAddress(uchar *pszPrompts, uchar bAllowNull, uchar *pszIPAddress);
//uchar ValidIPAddress(char *pszIPAddr);
static int  GetIPPort(uchar *pszPrompts, uchar bAllowNull, uchar *pszPortNo);
static void SetIssuerParam(uchar ucPermission);
static uchar DispAcqPama(void);
static int  SetAcqTransComm(uchar ucCommType);
static int  SetAcqTransTelNo(void);
static int  SetTel(uchar *pszTelNo, uchar *psDisp);
static void SetEdcParam(uchar ucPermission);
static int  SetTermCurrency(uchar ucPermission);
static int  SetMerchantName(uchar ucPermission);
static int  SetGetSysTraceNo(uchar ucPermission);
static int  SetGetSysInvoiceNo(uchar ucPermission);
static int  SetPEDMode(void);
static int  SetAcceptTimeOut(void);
static int  SetPrinterType(void);
static int  SetPrnStlTotal(void);//2014-11-4
static int  SetNumOfReceipt(void);
static int  SetCallInTime(void);
static uchar IsValidTime(uchar *pszTime);
static int  ModifyOptList(uchar *psOption, uchar ucMode, uchar ucPermission);
static void ChangePassword(void);
static void SetSysTime(void);
static void SetEdcLang(void);
static void SetPowerSave(void);
static void TestMagicCard1(void);
static void TestMagicCard2(void);
static void TestMagicCard3(void);
static void TestMagicCard(int iTrackNum);
static void ToolsViewPreTransMsg(void);
static void ShowExchangePack(void);
static void PrnExchangePack(void);
static void DebugNacTxd(uchar ucPortNo, uchar *psTxdData, ushort uiDataLen);
static int SetCasinoMode(void);
static int SetNoSignLmt(void);
static uchar PasswordOfDemo(void);//2016-3-11
#ifdef PP_SSL
void SetIPMode(void);//2014-9-11 //Gillian 20160914
void SetSSL_URL(void);
#endif

/********************** Internal variables declaration *********************/

static MenuItem sgFuncMenu[] =
{
	{FALSE, _T_NOOP("0.INIT"),			DownLoadTMSPara_Manual},
	{TRUE,  _T_NOOP("1.VIEW RECORD"),	ViewTranList},
	{FALSE, _T_NOOP("2.SETUP"),			SetSystemParam},
	{TRUE,  _T_NOOP("3.LANGUAGE"),		SetEdcLang},
	{TRUE,  _T_NOOP("4.LOCK TERM"),		LockTerm},
	{TRUE,  _T_NOOP("5.VIEW TOTAL"),	ViewTotal},
	{FALSE, _T_NOOP("6.ECHO TEST"),		TransEchoTest},
	{TRUE,  _T_NOOP("7.WAVE POLL"),		ClssPoll},//squall
#ifdef SUPPORT_TABBATCH
	{TRUE,  _T_NOOP("8.VIEW AUTH RECD"),ViewAuthList}, 
#endif
#ifdef ENABLE_EMV
	{FALSE, _T_NOOP("9.LAST TSI TVR"),	ViewTVR_TSI},
#endif
	{TRUE,  _T_NOOP("10.SET TIME"),		SetSysTime},
	{FALSE, _T_NOOP("11.PRINT PARA"),	PrintParam},
	{TRUE,  _T_NOOP("12.SET CLSS"),		SetClssParam},
#ifdef AMT_PROC_DEBUG
	{TRUE,  _T_NOOP("13.PRN AMT PROC"),	PrintAmount},//2014-9-18 ttt: debug printing all amounts in process
	{TRUE,  _T_NOOP("14.PRN ECR AMT"),	PrintEcrAmount},//2014-11-4 
#endif
	{TRUE,  _T_NOOP("21.TXN REVIEW"),	ViewSpecList},
	{FALSE, _T_NOOP("22.ADJUST"),		TransAdjust},
    {FALSE, _T_NOOP("24.DCC OPT OUT"),  PPDCC_TransOptOut}, // PP-DCC
    {FALSE, _T_NOOP("27.TEST FUNC"),    Richard_test}, // add by richard 20161214
#ifdef PP_SSL
	{TRUE,  _T_NOOP("40.SET IP MODE"),  SetIPMode},//2015-7-8 HASE SSL //Gillian 20160914
#endif
	{TRUE,	_T_NOOP("50.APP UPDATE"),	DownLoadTMSPara_Auto},	// hidden, not for public use until confirm.
	{FALSE,	_T_NOOP("60.CHECK FONTS"),	EnumSysFonts},
	{TRUE,  _T_NOOP("70.RATE REPORT"),	PPDCC_TransRateReport}, // PP-DCC
	{TRUE,  _T_NOOP("71.REPRN SETTLE"),	RePrnSettle},
	{TRUE,  _T_NOOP("72.REPRINT LAST"),	PrnLastTrans},
	{TRUE,  _T_NOOP("73.REPRINT BILL"),	RePrnSpecTrans},
	{TRUE,  _T_NOOP("74.PRINT TOTAL"),	PrnTotal},
	{TRUE,  _T_NOOP("75.PRINT LOG"),	PrnAllList},
	{TRUE,  _T_NOOP("81.POWER SAVE"),	SetPowerSave},
//	{FALSE, _T_NOOP("87.TEST TRACK1"),	TestMagicCard1},
//	{FALSE, _T_NOOP("88.TEST TRACK2"),	TestMagicCard2},
//	{FALSE, _T_NOOP("89.TEST TRACK3"),	TestMagicCard3},
	{TRUE,  _T_NOOP("90.MODIFY PWD"),	ChangePassword},
	{TRUE,  _T_NOOP("91.DISP VER"),		DispVersion},
    {TRUE,  _T_NOOP("92.TMK DWONLOAD"), KeyDownload},       //add by richard 20161107, for TMK download 
	{FALSE, _T_NOOP("93.PP CARD BIN"),	PPDCC_CardBinMenu}, // PP-DCC
	{TRUE,  _T_NOOP("94.TERM SIGNON"),  TerminalSignOn},    //add by richard 20161107,  for SignOn 
	{FALSE, _T_NOOP("95.SHOW PACKAGE"),	ToolsViewPreTransMsg},
#ifdef ENABLE_EMV
	{FALSE, _T_NOOP("96.PRINT ERR LOG"),PrintEmvErrLog},
#endif
#ifdef APP_MANAGER_VER
	{TRUE,  _T_NOOP("97.FUNCTIONALITY"),SetManagerService},
	{FALSE, _T_NOOP("98.SUB APP"),		EnableProc},
#endif
	{FALSE, _T_NOOP("99.CLEAR"),		DoClear},
	{TRUE,  "", NULL},
};

static MenuItem sgInitMenu[] =
{
	{FALSE, _T_NOOP("0.INIT"),			DownLoadTMSPara_Manual},
//	{FALSE, _T_NOOP("2.SETUP"),			SetSystemParam},
	{TRUE,  _T_NOOP("3.LANGUAGE"),		SetEdcLang},
	{TRUE,  _T_NOOP("10.SET TIME"),		SetSysTime},
	{FALSE,	_T_NOOP("60.CHECK FONTS"),	EnumSysFonts},
	{FALSE, _T_NOOP("87.TEST TRACK1"),	TestMagicCard1},
	{FALSE, _T_NOOP("88.TEST TRACK2"),	TestMagicCard2},
	{FALSE, _T_NOOP("89.TEST TRACK3"),	TestMagicCard3},
	{TRUE,  _T_NOOP("90.MODIFY PWD"),	ChangePassword},
	{TRUE,  _T_NOOP("91.DISP VER"),		DispVersion},
	{FALSE, _T_NOOP("99.CLEAR"),		DoClear},
	{TRUE,  "", NULL},
};

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/
uchar PasswordOfDemo(void)//2016-3-11
{
    uchar   ucRet, szBuff[20];

    PubShowTitle(TRUE, (uchar *)_T("DEMO MODE"));

    while( 1 )
    {
        ScrClrLine(2, 7);
        PubDispString(_T("PASSWORD"), 2|DISP_LINE_LEFT);
        memset(szBuff, 0, sizeof(szBuff));
        ucRet = PubGetString(ALPHA_IN, 8, 8, szBuff, USER_OPER_TIMEOUT);
        if( ucRet!=0 )
        {
            return 1;
        }
        if( memcmp(szBuff, "DEMO1234", 8)==0 &&
            strlen((char *)szBuff)==8 )
        {
            return 0;
        }
        ScrClrLine(4, 7);
        PubDispString(_T("PWD ERROR!"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(3);
    }
}
void GetAllSupportFunc(char *pszBuff)
{
	int	ii;
	ulong	ulTemp;

	pszBuff[0] = 0;
	for (ii=0; ii<sizeof(sgFuncMenu)/sizeof(sgFuncMenu[0]); ii++)
	{
		if (sgFuncMenu[ii].szMenuName[0]!=0)
		{
			ulTemp = PubAsc2Long(sgFuncMenu[ii].szMenuName, 2, NULL);
			if (ulTemp<100)
			{
				if (strlen(pszBuff)!=0)
				{
					strcat(pszBuff, ",");
				}
				sprintf(pszBuff+strlen(pszBuff), "%lu", ulTemp);
			}
		}
	}
}

// 执行指定功能号的函数
void FunctionExe(uchar bUseInitMenu, int iFuncNo)
{
	int			iCnt, iTempNo;
	MenuItem	*pstMenu;

	pstMenu = bUseInitMenu ? sgInitMenu : sgFuncMenu;
	for(iCnt=0; pstMenu[iCnt].szMenuName[0]!=0; iCnt++)
	{
		if( isdigit(pstMenu[iCnt].szMenuName[1]) )
		{
			iTempNo = (int)PubAsc2Long(pstMenu[iCnt].szMenuName, 2, NULL);
		}
		else
		{
			iTempNo = (int)PubAsc2Long(pstMenu[iCnt].szMenuName, 1, NULL);
		}
		if( iTempNo==iFuncNo )
		{
			if( pstMenu[iCnt].pfMenuFunc==NULL )
			{
				break;
			}
			(*pstMenu[iCnt].pfMenuFunc)();
			return;
		}
	}

	ScrClrLine(2, 7);
	PubDispString(_T("FUNC NUMBER ERR"), 4|DISP_LINE_CENTER);
	PubBeepErr();
	PubWaitKey(3);
}

void FunctionMenu(void)
{
	PubGetMenu((uchar *)_T("FUNCTION:"), sgFuncMenu, MENU_USESNO|MENU_CFONT, USER_OPER_TIMEOUT);
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
void SetSystemParam(void)
{
	uchar ucPermission;

#ifdef FUN2_READ_ONLY
	ucPermission = PM_LOW;		// 低权限
#else
	ucPermission = PM_MEDIUM;	// 中等权限
#endif

	PubShowTitle(TRUE, (uchar *)_T("TERM SETUP"));
	if( PasswordBank()!=0 )
	{
		return;
	}

	SetSystemParamSub(ucPermission);
}
		
void SetSystemParamSub(uchar ucPermission)
{
	uchar	ucKey;
	while( 1 )
	{
		ScrCls();
		PubDispString(_T("1.COMM PARA"),     DISP_LINE_LEFT|0);
		PubDispString(_T("2.VIEW EDC"),      DISP_LINE_LEFT|2);
		PubDispString(_T("3.VIEW ISSUER"),   DISP_LINE_LEFT|4);
		PubDispString(_T("4.VIEW ACQUIRER"), DISP_LINE_LEFT|6);

		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( ucKey==KEYCANCEL || ucKey==NOKEY )
		{
			return;
		}
		else if( ucKey==KEY1 )
		{
			SetSysCommParam(ucPermission);
		}
		else if( ucKey==KEY2 )
		{
			SetEdcParam(ucPermission);
		}
		else if( ucKey==KEY3 )
		{
			SetIssuerParam(ucPermission);
		}
		else if( ucKey==KEY4 )
		{
			SetAcqParam(ucPermission);
		}
	}
}

void SetSystemParamAll(void)
{
	// 最高权限，可以修改所有参数
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
	case CT_CDMA:
		sprintf((char *)pszText, "CDMA");
		return 0;
	case CT_GPRS:
		sprintf((char *)pszText, "GPRS");
		return 0;
	case CT_3G:
		sprintf((char *)pszText, "3G");//2015-11-23
		return 0;
	case CT_WIFI:
		sprintf((char *)pszText, "WIFI");
	    return 0;
	case CT_DEMO:
		sprintf((char *)pszText, "DEMO");
	    return 0;
	default:
		sprintf((char *)pszText, "DISABLED");
	    return -1;
	}
}

// ucForAcq : set custom comm type for ACQ
int SetCommType(uchar ucMode)
{
	int		iRet;
	char	szTitle[32];
	uchar	*pucCommType;
	SMART_MENU	stSmDownMode;
	MENU_ITEM stCommMenu[] =
	{
		{"SELECT COMM",		-1,			NULL},
		{"DISABLE",			CT_NONE,	NULL},
		{"MODEM",			CT_MODEM,	NULL},
		{"TCPIP",			CT_TCPIP,	NULL},
		{"GPRS",			CT_GPRS,	NULL},
		{"3G",			    CT_3G,   	NULL},//2015-11-23
		{"CDMA",			CT_CDMA,	NULL},
		{"WIFI",			CT_WIFI,	NULL},
		{"RS232",			CT_RS232,	NULL},
		{"DEMO ONLY",		CT_DEMO,	NULL},
		{"",				0, NULL},
	};// This menu does not provide translation

	// DEMO 模式需要先选择modem之后，在PABX设置成DEMO

	//--------------------------------------------------
	memset(&stSmDownMode, 0, sizeof(stSmDownMode));
	PubSmInit(&stSmDownMode, stCommMenu);

	if (ucMode==0)
	{
		PubSmMask(&stSmDownMode, "DISABLE",   SM_OFF);
	}
	if (ucMode!=0)
	{
		PubSmMask(&stSmDownMode, "DEMO ONLY", SM_OFF);
	}

	if (ChkTerm(_TERMINAL_P60_S1_) || ChkTerm(_TERMINAL_P70_S_))
	{
		PubSmMask(&stSmDownMode, "TCPIP", SM_OFF);	
		PubSmMask(&stSmDownMode, "GPRS", SM_OFF);
		PubSmMask(&stSmDownMode, "3G", SM_OFF);//2015-11-23
		PubSmMask(&stSmDownMode, "CDMA", SM_OFF);
		PubSmMask(&stSmDownMode, "WIFI", SM_OFF);
	} 
	else
	{
#ifndef DEMO_HK
		if (ChkHardware(HWCFG_MODEM, HW_NONE) ||
			((ucMode!=0) && (glSysParam.stTxnCommCfg.ucCommType==CT_MODEM)) )
		{
			PubSmMask(&stSmDownMode, "MODEM", SM_OFF);
		}
#endif
		if (ChkHardware(HWCFG_LAN, HW_NONE) ||									// If no LAN module
			((ucMode!=0) && (glSysParam.stTxnCommCfg.ucCommType==CT_TCPIP)) )	// and now is selecting 2nd comm && 1st comm already selected LAN
		{
			PubSmMask(&stSmDownMode, "TCPIP", SM_OFF);							// then disable this option
		}
		if ((ChkHardware(HWCFG_GPRS, HW_NONE) && !ChkTerm(_TERMINAL_P90_)) ||	// Sometimes P90 cannot get module info
			((ucMode!=0) && (glSysParam.stTxnCommCfg.ucCommType==CT_GPRS)) )
		{
			PubSmMask(&stSmDownMode, "GPRS", SM_OFF);
		}
		if ((ChkHardware(HWCFG_WCDMA, HW_NONE) && !ChkTerm(_TERMINAL_P90_)) ||	// Sometimes P90 cannot get module info
			((ucMode!=0) && (glSysParam.stTxnCommCfg.ucCommType==CT_3G)) )//2015-11-23 3G
		{
			PubSmMask(&stSmDownMode, "3G", SM_OFF);
		}
		if ((ChkHardware(HWCFG_CDMA, HW_NONE) && !ChkTerm(_TERMINAL_P90_)) ||	// Sometimes P90 cannot get module info
			((ucMode!=0) && (glSysParam.stTxnCommCfg.ucCommType==CT_CDMA)) )
		{
			PubSmMask(&stSmDownMode, "CDMA", SM_OFF);
		}
		if ((ChkHardware(HWCFG_WIFI, HW_NONE) && !ChkTerm(_TERMINAL_S80_)) ||
			((ucMode!=0) && (glSysParam.stTxnCommCfg.ucCommType==CT_WIFI)) )
		{
			PubSmMask(&stSmDownMode, "WIFI", SM_OFF);
		}
		if ((ucMode!=0) && (glSysParam.stTxnCommCfg.ucCommType==CT_RS232))
		{
			PubSmMask(&stSmDownMode, "RS232", SM_OFF);
		}
	}

	memset(szTitle, 0, sizeof(szTitle));
	stCommMenu[0].pszText = szTitle;
	if (ucMode==0)
	{
		pucCommType = &glSysParam.stTxnCommCfg.ucCommType;
		strcpy((char *)stCommMenu[0].pszText, "1st:");
	}
	else
	{
		pucCommType = &glSysParam.stTxnCommCfg.ucCommTypeBak;
		strcpy((char *)stCommMenu[0].pszText, "2nd:");
	}

	GetCommName(*pucCommType, stCommMenu[0].pszText+strlen((char *)stCommMenu[0].pszText));
	iRet = PubSmartMenuEx(&stSmDownMode, SM_1ITEM_PER_LINE, USER_OPER_TIMEOUT);
	if (iRet == CT_DEMO)
	{
		PubShowTitle(TRUE, (uchar *)"SETUP COMM      ");
		if( PasswordOfDemo()!=0 )//2016-3-11 default password
		{
			return ERR_USERCANCEL;
		}
		ScrClrLine(2,7);
		PubDispString(_T("CONFIRM DEMO?"), 2|DISP_LINE_LEFT);
		PubDispString(_T("[ENTER]YES"), 4|DISP_LINE_LEFT);
		PubDispString(_T("[CANCEL]NO"), 6|DISP_LINE_LEFT);
 		if(KEYENTER!=PubWaitKey(USER_OPER_TIMEOUT))
		{
			return ERR_USERCANCEL;
		}			
	}
	if (iRet<0)
	{
		return ERR_USERCANCEL;
	}

	if (!ChkIfBatchEmpty() && (*pucCommType!=(uchar)iRet))
	{
		// Not allow to switch into/off demo mode unless batch empty
		if ((*pucCommType==CT_DEMO) || ((uchar)iRet==CT_DEMO))
		{
			ScrClrLine(2,7);
			PubDispString(_T("PLS SETTLE BATCH"), 4|DISP_LINE_LEFT);
			PubBeepErr();
			PubWaitKey(USER_OPER_TIMEOUT);
			return ERR_USERCANCEL;
		}
	}

	*pucCommType = (uchar)iRet;
	return 0;
}

// 设置通讯参数
void SetSysCommParam(uchar ucPermission)
{
	PubShowTitle(TRUE, (uchar *)"SETUP COMM      ");

	while (1)
	{
		if (SetCommType(0)!=0)
		{
			break;
		}

		if (SetCommDetails(&glSysParam.stTxnCommCfg.ucCommType))
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

		if (SetCommDetails(&glSysParam.stTxnCommCfg.ucCommTypeBak))
		{
			break;
		}

		break;
	}

	SaveSysParam();
}

int SetCommDetails(uchar *pucCommType)
{
	uchar	szDispBuff[32];

	sprintf((char *)szDispBuff, "SETUP ");
	GetCommName(*pucCommType, szDispBuff+strlen((char *)szDispBuff));
	ScrCls();
	PubDispString(szDispBuff, DISP_LINE_REVER);

	switch( *pucCommType )
	{
	case CT_RS232:
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
	case CT_3G://2015-11-23
		SetTcpIpSharedPara(&glSysParam.stTxnCommCfg);
		Set3GParam(&glSysParam._TxnWirlessPara);
		SyncWirelessParam(&glSysParam._TmsWirlessPara, &glSysParam._TxnWirlessPara);
		CommOnHook(TRUE);
		DispWait();
		CommInitModule(&glSysParam.stTxnCommCfg);
		break;
	case CT_GPRS:
	case CT_CDMA:
		SetTcpIpSharedPara(&glSysParam.stTxnCommCfg);
		SetWirelessParam(&glSysParam._TxnWirlessPara);
		SyncWirelessParam(&glSysParam._TmsWirlessPara, &glSysParam._TxnWirlessPara);
		CommOnHook(TRUE);
		DispWait();
		CommInitModule(&glSysParam.stTxnCommCfg);
		break;

//	case CT_WIFI:
//		...
//		CommInitModule(&glSysParam.stTxnCommCfg);
//		break;

	case CT_DEMO:
	default:
	    break;
	}

	return 0;
}

// 输入PABX
int SetPabx(void)
{
	PubDispString("MODIFY PABX", DISP_LINE_LEFT|2);
	if( PubGetString(ALPHA_IN|ECHO_IN, 0, 10, glSysParam.stEdcInfo.szPabx, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}

	return 0;
}

// 修改Modem参数
int SetModemParam(void)
{
	uchar	ucKey, szBuff[20], ucCurBaud, ucCnt, ucTemp;
	static	char	*ppszBaudRate[4] = {"1200", "2400", "9600", "14400"};

	if( SetPabx()!=0 )
	{
		return ERR_USERCANCEL;
	}

	ucTemp = glSysParam.stEdcInfo.bPreDial;
	while( 1 )
	{
		ScrClrLine(2, 7);
		PubDispString("PRE DIAL", DISP_LINE_CENTER|2);
		sprintf((char *)szBuff, "(%c)", ucTemp ? '1' : '0');
		PubDispString(szBuff,       DISP_LINE_CENTER|4);
		PubDispString("1.ON 0.OFF", DISP_LINE_CENTER|6);
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( ucKey==KEYCANCEL || ucKey==NOKEY )
		{
			return ERR_USERCANCEL;
		}
		else if( ucKey==KEYENTER )
		{
			break;
		}
		else if ( ucKey==KEY0 )
		{
			ucTemp = FALSE;
		}
		else if ( ucKey==KEY1 )
		{
			ucTemp = TRUE;
		}
	}
	glSysParam.stEdcInfo.bPreDial = ucTemp;

	ucTemp = glSysParam._TxnModemPara.DP;
	while( 2 )
	{
		ScrClrLine(2, 7);
		sprintf((char *)szBuff, "(%c)",  ucTemp ? '1' : '0');
		PubDispString("DIAL MODE",      DISP_LINE_CENTER|2);
		PubDispString(szBuff,           DISP_LINE_CENTER|4);
		PubDispString("0.DTMF 1.PULSE", DISP_LINE_CENTER|6);

		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( ucKey==KEYCANCEL || ucKey==NOKEY )
		{
			return ERR_USERCANCEL;
		}
		else if( ucKey==KEYENTER )
		{
			break;
		}
		else if ( ucKey==KEY0 )
		{
			 ucTemp = 0;
		}
		else if ( ucKey==KEY1 )
		{
			ucTemp = 1;
		}
	}
	glSysParam._TxnModemPara.DP = ucTemp;

	ucTemp = glSysParam._TxnModemPara.CHDT;
	while( 3 )
	{
		ScrClrLine(2, 7);
		sprintf((char *)szBuff, "(%c)", ucTemp ? '1' : '0');
		PubDispString("DIAL TONE",        DISP_LINE_CENTER|2);
		PubDispString(szBuff,             DISP_LINE_CENTER|4);
		PubDispString("0.DETECT 1IGNORE", DISP_LINE_CENTER|6);
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( ucKey==KEYCANCEL || ucKey==NOKEY )
		{
			return ERR_USERCANCEL;
		}
		else if( ucKey==KEYENTER )
		{
			break;
		}
		else if ( ucKey==KEY0 )
		{
			ucTemp = 0;
		}
		else if ( ucKey==KEY1 )
		{
			ucTemp = 1;
		}
	}
	glSysParam._TxnModemPara.CHDT = ucTemp;

	ScrClrLine(2, 7);
	sprintf((char *)szBuff, "OLD:%u(*100ms)", (uint)glSysParam._TxnModemPara.DT1);
	PubDispString(szBuff,       DISP_LINE_LEFT|2);
	PubDispString("DIAL WAIT:", DISP_LINE_LEFT|4);
	sprintf((char *)szBuff, "%u", (uint)glSysParam._TxnModemPara.DT1);
	if( PubGetString(NUM_IN|ECHO_IN, 0, 2, szBuff, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}
	glSysParam._TxnModemPara.DT1 = (uchar)atoi((char *)szBuff);

	ScrClrLine(2, 7);
	sprintf((char *)szBuff, "OLD:%u(*100ms)", (uint)glSysParam._TxnModemPara.DT2);
	PubDispString(szBuff,        DISP_LINE_LEFT|2);
	PubDispString("PABX PAUSE:", DISP_LINE_LEFT|4);
	sprintf((char *)szBuff,"%u", (uint)glSysParam._TxnModemPara.DT2);
	if( PubGetString(NUM_IN|ECHO_IN, 0, 2, szBuff, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
 	}
	glSysParam._TxnModemPara.DT2 = (uchar)atoi((char *)szBuff);

	ScrClrLine(2, 7);
	sprintf((char *)szBuff, "OLD:%u(ms)", (uint)glSysParam._TxnModemPara.HT);
	PubDispString(szBuff,           DISP_LINE_LEFT|2);
	PubDispString("ONE DTMF HOLD:", DISP_LINE_LEFT|4);
	sprintf((char *)szBuff, "%u", (uint)glSysParam._TxnModemPara.HT);
	if( PubGetString(NUM_IN|ECHO_IN, 0, 3, szBuff, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}
	glSysParam._TxnModemPara.HT = (uchar)atoi((char *)szBuff);

	ScrClrLine(2, 7);
	sprintf((char *)szBuff, "OLD:%u(*10ms)", (uint)glSysParam._TxnModemPara.WT);
	PubDispString(szBuff,             DISP_LINE_LEFT|2);
	PubDispString("DTMF CODE SPACE:", DISP_LINE_LEFT|4);
	sprintf((char *)szBuff, "%u", (uint)glSysParam._TxnModemPara.WT);
	if( PubGetString(NUM_IN|ECHO_IN, 0, 3, szBuff, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}
	glSysParam._TxnModemPara.WT = (uchar)atoi((char *)szBuff);
    
    //---------------------------------------------------
    while (1)
    {
	    ScrClrLine(2, 7);
	    sprintf((char *)szBuff, "OLD:%u(0, 1~15)", (uint)glSysParam._TxnPSTNPara.ucSignalLevel);
	    PubDispString(szBuff,             DISP_LINE_LEFT|2);
	    PubDispString("SIGNAL LEVEL:", DISP_LINE_LEFT|4);
	    sprintf((char *)szBuff, "%u", (uint)glSysParam._TxnPSTNPara.ucSignalLevel);
	    if( PubGetString(NUM_IN|ECHO_IN, 0, 2, szBuff, USER_OPER_TIMEOUT)!=0 )
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

	// set baud rate
	ucCurBaud = (glSysParam._TxnModemPara.SSETUP>>5) & 0x03;
	while( 1 )
	{
		ScrClrLine(2, 7);
		PubDispString("BAUD RATE:", 2|DISP_LINE_LEFT);
		for(ucCnt=0; ucCnt<4; ucCnt++)
		{
			ScrPrint(0, (uchar)(ucCnt+4), ASCII, "   %s%d.%s", (ucCnt==ucCurBaud ? ">" : " "),
					(int)(ucCnt+1), ppszBaudRate[ucCnt]);
		}
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( ucKey==KEYCANCEL || ucKey==NOKEY )
		{
			return ERR_USERCANCEL;
		}
		else if( ucKey==KEYENTER )
		{
			break;
		}
		else if( ucKey==KEYUP )
		{
			ucCurBaud = (ucCurBaud==0) ? 3 : ucCurBaud-1;
		}
		else if( ucKey==KEYDOWN )
		{
			ucCurBaud = (ucCurBaud>=3) ? 0 : ucCurBaud+1;
		}
		else if( ucKey>=KEY1 && ucKey<=KEY4 )
		{
			ucCurBaud = ucKey - KEY1;
			break;
		}
	}
	glSysParam._TxnModemPara.SSETUP &= 0x9F;	// 1001 1111
	glSysParam._TxnModemPara.SSETUP |= (ucCurBaud<<5);

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

int GetRemoteIp(uchar *pszHalfText, uchar bAllowNull, void *pstIPAddr)
{
	int		iRet;
	IP_ADDR	*pstIp;
	uchar	szBuff[32];

	pstIp = (IP_ADDR *)pstIPAddr;


	sprintf((char *)szBuff, "%s IP", pszHalfText);
#ifdef PP_SSL
	//pstIp->szIP = 
#endif
	iRet = GetIPAddress(szBuff, bAllowNull, pstIp->szIP);
	if( iRet!=0 )
	{
		return iRet;
	}

	sprintf((char *)szBuff, "%s PORT", pszHalfText);
	iRet = GetIPPort(szBuff, bAllowNull, pstIp->szPort);
	if( iRet<0 )
	{
		return iRet;
	}

	return 0;
}

int ChkIfValidIp(uchar *pszIP)
{
	return ((pszIP[0]!=0) && (ValidIPAddress(pszIP)));
}

int ChkIfValidPort(uchar *pszPort)
{
	return ((pszPort[0]!=0) &&
			(atol((uchar *)pszPort)>0) &&
			(atol((uchar *)pszPort)<65536));
}

int SetTcpIpSharedPara(COMM_CONFIG *pstCommCfg)
{
	uchar	ucKey;
	uchar	szDispBuff[128];

	while (1)
	{
		ScrClrLine(2, 7);
		sprintf((char *)szDispBuff, "TCP LENGTH: %s", (pstCommCfg->ucTCPClass_BCDHeader ? "BCD" : "HEX"));
	
		ucKey = PubSelectOneTwo((char *)szDispBuff, (uchar *)_T("BCD"), (uchar *)_T("HEXIMAL"));
		if (ucKey==KEYCANCEL)
		{
			return -1;
		}
		if (ucKey==KEYENTER)
		{
			return 0;
		}
		if (ucKey==KEY1)
		{
			pstCommCfg->ucTCPClass_BCDHeader = TRUE;
			return 0;
		}
		if (ucKey==KEY2)
		{
			pstCommCfg->ucTCPClass_BCDHeader = FALSE;
			return 0;
		}
	}
}

// 设置TCP/IP参数
int SetTcpIpParam(void *pstParam)
{
	int		iRet;

	// !!!! 需要应用到开机步骤

#ifdef SXX_IP_MODULE
    iRet = SetTcpIpParam_S80((TCPIP_PARA *)pstParam);
	return iRet;

#else
	iRet = GetIpLocalSettings((TCPIP_PARA *)pstParam);
	if( iRet!=0 )
	{
		return iRet;
	}

	return 0;

#endif
}

void SyncTcpIpParam(void *pstDst, void *pstSrc)
{
	((TCPIP_PARA *)pstDst)->ucDhcp = ((TCPIP_PARA *)pstSrc)->ucDhcp;
	strcpy((char *)(((TCPIP_PARA *)pstDst)->szLocalIP),   (char *)(((TCPIP_PARA *)pstSrc)->szLocalIP));
	strcpy((char *)(((TCPIP_PARA *)pstDst)->szGatewayIP), (char *)(((TCPIP_PARA *)pstSrc)->szGatewayIP));
	strcpy((char *)(((TCPIP_PARA *)pstDst)->szNetMask),   (char *)(((TCPIP_PARA *)pstSrc)->szNetMask));
	strcpy((char *)(((TCPIP_PARA *)pstDst)->szDNSIP),     (char *)(((TCPIP_PARA *)pstSrc)->szDNSIP));
}

#ifdef SXX_IP_MODULE
int SetTcpIpParam_S80(TCPIP_PARA *pstParam)
{
	int		iRet;
	uchar	ucKey;
	uchar	szDispBuff[32];
	long lTcpState;


	while (1)
	{
		ScrClrLine(2, 7);
		iRet = DhcpCheck();
		if (iRet==0)
		{
			sprintf((char *)szDispBuff, "DHCP: OK");
		}
		else
		{
			sprintf((char *)szDispBuff, "DHCP: STOPPED");
		}
	
		ucKey = PubSelectOneTwo((char *)szDispBuff, (uchar *)_T("START"), (uchar *)_T("STOP"));
		if (ucKey==KEYCANCEL)
		{
			return -1;
		}
		if (ucKey==KEYENTER)
		{
			continue;
		}
		if (ucKey==KEY1)
		{
			pstParam->ucDhcp = 1;

			ScrClrLine(2, 7);
			PubDispString("GETTING IP...", 4|DISP_LINE_LEFT);
			if (SxxDhcpStart(FALSE, 30)==0)
			{
				iRet = EthGet(pstParam->szLocalIP, pstParam->szNetMask, pstParam->szGatewayIP, pstParam->szDNSIP, &lTcpState);
				
				ScrClrLine(2, 7);
				PubDispString("LOCAL IP", 2|DISP_LINE_LEFT);
				PubDispString(pstParam->szLocalIP, 6|DISP_LINE_RIGHT);
				getkey();
				//memcpy(gCurComPara.LocalParam.szLocalIP, pstParam->szLocalIP, strlen(pstParam->szLocalIP));
				strcpy(gCurComPara.LocalParam.szLocalIP, pstParam->szLocalIP);

				ScrClrLine(2, 7);
				PubDispString("IP MASK", 2|DISP_LINE_LEFT);
				PubDispString(pstParam->szNetMask, 6|DISP_LINE_RIGHT);
				getkey();
				strcpy(gCurComPara.LocalParam.szNetMask, pstParam->szNetMask);
				ScrClrLine(2, 7);
				PubDispString("GATEWAY IP", 2|DISP_LINE_LEFT);
				PubDispString(pstParam->szGatewayIP, 6|DISP_LINE_RIGHT);
				getkey();
				strcpy(gCurComPara.LocalParam.szGatewayIP, pstParam->szGatewayIP);

				ScrClrLine(2, 7);
				PubDispString("DNS", 2|DISP_LINE_LEFT);
				PubDispString(pstParam->szDNSIP, 6|DISP_LINE_RIGHT);
				getkey();
				strcpy(gCurComPara.LocalParam.szDNSIP, pstParam->szDNSIP);
				return 0;
			}
			else
			{
				DhcpStop();
			}
		}
		if (ucKey==KEY2)
		{
			pstParam->ucDhcp = 0;
			DhcpStop();
			break;
		}
	}

	// Manual setup

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
		ScrClrLine(2, 7);
		PubDispString("SET STATIC IP", 2|DISP_LINE_LEFT);
		PubDispString("FAILED.",       4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(USER_OPER_TIMEOUT);
		return -1;
	}

	return 0;
}
#endif

// 输入IP地址
int GetIPAddress(uchar *pszPrompts, uchar bAllowNull, uchar *pszIPAddress)
{
	uchar	szTemp[20], szDispBuff[20];

	sprintf((char *)szTemp, "%.15s", pszIPAddress);
	while( 1 )
	{
		ScrClrLine(2, 7);
		sprintf((char *)szDispBuff, "%.16s", pszPrompts);
		PubDispString(szDispBuff, 2|DISP_LINE_LEFT);

#ifdef IP_INPUT
    if( PubGetIPString(ALPHA_IN|ECHO_IN, 0, 15, szTemp, USER_OPER_TIMEOUT) != 0 )	
#else
		if( PubGetString(ALPHA_IN|ECHO_IN, 0, 15, szTemp, USER_OPER_TIMEOUT)!=0 )
#endif
		{
			return ERR_USERCANCEL;
		}
		if( bAllowNull && szTemp[0]==0 )
		{
			*pszIPAddress = 0;
			break;
		}
		if( ValidIPAddress((char *)szTemp) )
		{
			sprintf((char *)pszIPAddress, "%.15s", szTemp);
			break;
		}

		ScrClrLine(4, 7);
		PubDispString(_T("INV IP ADDR"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(4);
	}

	return 0;
}

// 检查IP地址
/*uchar ValidIPAddress(char *pszIPAddr)
{
	int		i;
	char	*p, *q, szBuf[5+1];

	PubTrimStr((uchar *)pszIPAddr);
	if( *pszIPAddr==0 )
	{
		return FALSE;
	}

	p = strchr(pszIPAddr, ' ');
	if( p!=NULL )
	{
		return FALSE;
	}
	if( strlen(pszIPAddr)>15 )
	{
		return FALSE;
	}

	// 1st --- 3rd  part
	for(q=pszIPAddr, i=0; i<3; i++)
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
*/

// 输入端口
int GetIPPort(uchar *pszPrompts, uchar bAllowNull, uchar *pszPortNo)
{
	int		iTemp;
	uchar	ucMinLen, szTemp[15], szDispBuff[20];

	ucMinLen = (bAllowNull ? 0 : 1);
	while( 1 )
	{
		sprintf((char *)szTemp, "%.5s", pszPortNo);
		ScrClrLine(2, 7);
		sprintf((char *)szDispBuff, "%.16s", pszPrompts);
		PubDispString(szDispBuff, 2|DISP_LINE_LEFT);
		if( PubGetString(NUM_IN|ECHO_IN, ucMinLen, 5, szTemp, USER_OPER_TIMEOUT)!=0 )
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

		ScrClrLine(4, 7);
		PubDispString(_T("INV PORT #"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(4);
	}

	return 0;
}

// 选择发卡行并修改参数
void SetIssuerParam(uchar ucPermission)
{
	int		iRet;
	uchar	ucIndex, szBuff[32], szBuff2[32];
#ifdef ENABLE_EMV
	int		iCnt;
	ulong	ulTemp;
	EMV_APPLIST	stEmvApp;
#endif

	while (1)
	{
		iRet = SelectIssuer(&ucIndex);
		if( iRet!=0 )
		{
			return;
		}

		sprintf((char *)szBuff, "ISSUER: %-8.8s", (char *)glCurIssuer.szName);
		PubShowTitle(TRUE, szBuff);
		ModifyOptList(glSysParam.stIssuerList[ucIndex].sOption, 'I', ucPermission);

		if (ucPermission==PM_HIGH)
		{
			while (1)
			{
				PubShowTitle(TRUE, szBuff);
				PubDispString(_T("NON-EMV FLOOR"),  DISP_LINE_LEFT|2);
				PubDispString(_T("EXCLUDE DECIMAL"),  DISP_LINE_LEFT|4);
				sprintf((char *)szBuff2, "%lu", glSysParam.stIssuerList[ucIndex].ulFloorLimit);
				iRet = PubGetString(NUM_IN|ECHO_IN, 1, 2, szBuff2, USER_OPER_TIMEOUT);
				if( iRet!=0 )
				{
					break;
				}
				glSysParam.stIssuerList[ucIndex].ulFloorLimit = (ulong)atoi(szBuff2);

				break;
			}

#ifdef ENABLE_EMV
			while (1)
			{
				PubShowTitle(TRUE, szBuff);
				PubDispString(_T("EMV FLOOR LIMIT"),  DISP_LINE_LEFT|2);
				PubDispString(_T("EXCLUDE DECIMAL"),  DISP_LINE_LEFT|4);
				memset(szBuff2, 0, sizeof(szBuff2));
				iRet = PubGetString(NUM_IN|ECHO_IN, 1, 4, szBuff2, USER_OPER_TIMEOUT);
				if( iRet!=0 )
				{
					break;
				}
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

				break;
			}
#endif

			if (ChkEdcOption(EDC_TIP_PROCESS))
			{
				while (1)
				{
					PubShowTitle(TRUE, szBuff);
					PubDispString(_T("ADJUST PERCENT"),  DISP_LINE_LEFT|2);
					if (glSysParam.stIssuerList[ucIndex].ucAdjustPercent == 'd')
					{//2013311
						sprintf((char *)szBuff2, "%d", (int)(glSysParam.stIssuerList[ucIndex].ucAdjustPercent));
					}
					else
					{
						sprintf((char *)szBuff2, "%d", (int)(glSysParam.stIssuerList[ucIndex].ucAdjustPercent%100));
					}
					
					if( PubGetString(NUM_IN|ECHO_IN, 1, 6, szBuff2, USER_OPER_TIMEOUT)!=0 )
					{
						break;
					}
					if (atoi(szBuff2)>50)
					{
						continue;
					}
					
					glSysParam.stIssuerList[ucIndex].ucAdjustPercent = (uchar)atoi(szBuff2);
					break;
				}
			}

			if (ChkEdcOption(EDC_REFERRAL_DIAL))
			{
				while (1)
				{
					PubShowTitle(TRUE, szBuff);
					PubDispString(_T("REFERRAL TEL"),  DISP_LINE_LEFT|2);
					sprintf((char *)szBuff2, "%.12s", (char *)glSysParam.stIssuerList[ucIndex].szRefTelNo);
					if( PubGetString(NUM_IN|ECHO_IN, 0, 12, szBuff2, USER_OPER_TIMEOUT)!=0 )
					{
						break;
					}

					sprintf((char *)glSysParam.stIssuerList[ucIndex].szRefTelNo, "%.12s", (char *)szBuff2);
					break;
				}
			}
		}

		SaveSysParam();
		if( glSysParam.ucIssuerNum<2 )
		{
			break;
		}
	}
}

uchar ChkTAXIAcq(void *pszKeyword, uchar *AcqName)//find CITI_TAXI ACQ//squall20121108 build116
{
	uchar	szBuff[10+1];
	
	sprintf((char *)szBuff, "%.10s", AcqName);
	PubStrUpper(szBuff);
	
		if( memcmp(szBuff, pszKeyword, strlen((char *)pszKeyword))==0 )
		{
			return TRUE;
		}
	
	return FALSE;
}

// 设置收单行参数
void SetAcqParam(uchar ucPermission)
{
	int     iCounter;
	int		iRet;
	uchar	ucKey, szTitle[16+1], szBuff[16+1];
	if (glSysParam.stTxnCommCfg.ucCommType == CT_GPRS ||glSysParam.stTxnCommCfg.ucCommType == CT_CDMA )//squall20121108
	{
		for (iCounter=0;iCounter<glSysParam.ucAcqNum;iCounter++)
		{
			if ( ChkTAXIAcq("CITI_TAXI",glSysParam.stAcqList[iCounter].szName))
			{
				if (!strlen(glSysParam.stAcqList[iCounter].stTxnGPRSInfo[0].szIP))
				{
					strcpy(glSysParam.stAcqList[iCounter].stTxnGPRSInfo[0].szIP,"10.30.19.100");
				}
				if (!strlen(glSysParam.stAcqList[iCounter].stTxnGPRSInfo[0].szPort))
				{
					strcpy(glSysParam.stAcqList[iCounter].stTxnGPRSInfo[0].szPort,"4088");
				}
				if (!strlen(glSysParam.stAcqList[iCounter].stTxnGPRSInfo[1].szIP))
				{
					strcpy(glSysParam.stAcqList[iCounter].stTxnGPRSInfo[1].szIP,"10.30.19.108");
				}
				if (!strlen(glSysParam.stAcqList[iCounter].stTxnGPRSInfo[1].szPort))
				{
					strcpy(glSysParam.stAcqList[iCounter].stTxnGPRSInfo[1].szPort,"4088");
				}
			}
		}
	}
	if (glSysParam.stTxnCommCfg.ucCommType == CT_WIFI||glSysParam.stTxnCommCfg.ucCommType == CT_TCPIP )//squall20121108
	{
		for (iCounter=0;iCounter<glSysParam.ucAcqNum;iCounter++)
		{
			if ( ChkTAXIAcq("CITI_TAXI",glSysParam.stAcqList[iCounter].szName))
			{
				if (!strlen(glSysParam.stAcqList[iCounter].stTxnTCPIPInfo[0].szIP))
				{
					strcpy(glSysParam.stAcqList[iCounter].stTxnTCPIPInfo[0].szIP,"10.30.19.100");
				}
				if (!strlen(glSysParam.stAcqList[iCounter].stTxnTCPIPInfo[0].szPort))
				{
					strcpy(glSysParam.stAcqList[iCounter].stTxnTCPIPInfo[0].szPort,"4088");
				}
				if (!strlen(glSysParam.stAcqList[iCounter].stTxnTCPIPInfo[1].szIP))
				{
					strcpy(glSysParam.stAcqList[iCounter].stTxnTCPIPInfo[1].szIP,"10.30.19.108");
				}
				if (!strlen(glSysParam.stAcqList[iCounter].stTxnTCPIPInfo[1].szPort))
				{
					strcpy(glSysParam.stAcqList[iCounter].stTxnTCPIPInfo[1].szPort,"4088");
				}
			
			}
		}
	}//CITI taxi too many ACQs thus need a default para
	while (1)
	{
		sprintf((char *)szTitle, "%.16s", glSysParam.ucAcqNum>9 ? "SELECT ACQ:" : "SELECT ACQUIRER");
		iRet = SelectAcq(FALSE, szTitle, NULL);
		if( iRet!=0 )
		{
			return;
		}

		sprintf((char *)szTitle, "ACQ: %-10.10s ", (char *)glCurAcq.szName);
		while( 1 )
		{
			ucKey = DispAcqPama();
			if( ucKey==KEYCANCEL || ucKey==NOKEY )
			{
				break;
			}

			// set all bit flag options
			ScrCls();
			PubShowTitle(TRUE, szTitle);
			if( ModifyOptList(glCurAcq.sOption, 'A', ucPermission)!=0 )
			{
				break;
			}

			if (ucPermission>PM_MEDIUM)
			{
				PubShowTitle(TRUE, szTitle);
				PubDispString("ACQUIRER NAME", DISP_LINE_LEFT|2);
				if( PubGetString(NUM_IN|ALPHA_IN|ECHO_IN, 3, 10, glCurAcq.szName, USER_OPER_TIMEOUT)!=0 )
				{
					break;
				}
			}

			if (ucPermission>PM_LOW)
			{
				PubShowTitle(TRUE, szTitle);
				PubDispString("TERMINAL ID", DISP_LINE_LEFT|2);
				if( PubGetString(NUM_IN|ECHO_IN, 8, 8, glCurAcq.szTermID, USER_OPER_TIMEOUT)!=0 )
				{
					break;
				}

				ScrClrLine(2, 7);
				PubDispString("MERCHANT ID", DISP_LINE_LEFT|2);
				if( PubGetString(NUM_IN|ECHO_IN, 8, 15, glCurAcq.szMerchantID, USER_OPER_TIMEOUT)!=0 )
				{
					break;
				}

				ScrClrLine(2, 7);
				PubDispString("NII", DISP_LINE_LEFT|2);
				if( PubGetString(NUM_IN|ECHO_IN, 3, 3, glCurAcq.szNii, USER_OPER_TIMEOUT)!=0 )
				{
					break;
				}

				ScrClrLine(2, 7);
				PubDispString("BATCH NO.", DISP_LINE_LEFT|2);
				sprintf((char *)szBuff, "%lu", glCurAcq.ulCurBatchNo);
				if( PubGetString(NUM_IN|ECHO_IN, 1, 6, szBuff, USER_OPER_TIMEOUT)!=0 )
				{
					break;
				}
				if (glCurAcq.ulCurBatchNo!=(ulong)atoi(szBuff))
				{
					glCurAcq.ulCurBatchNo  = (ulong)atoi(szBuff);
					glCurAcq.ulNextBatchNo = GetNewBatchNo(glCurAcq.ulCurBatchNo);
				}

				if( SetAcqTransComm(glSysParam.stTxnCommCfg.ucCommType)!=0 )//tttttttttttt
				{
					break;
				}

				if( SetAcqTransComm(glSysParam.stTxnCommCfg.ucCommTypeBak)!=0 )
				{
					break;
				}
				

				if (ChkEdcOption(EDC_BEA_IP_ENCRY))  //Gillian 20160923
				{
					//2015-11-17 ttt  Gillian 20160923
					if ( KeyInit(szTitle) != 0 )
					{
						break;
					}
				}
			}
			break;
		}

		memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));

		SaveSysParam();
		if (glSysParam.ucAcqNum<2)
		{
			break;
		}
	}
}

// 显示当前收单行参数信息
uchar DispAcqPama(void)
{
	int		ii;
	uchar	ucShowComm, ucKey;

	for (ii=0; ii<2; ii++)
	{
		ScrCls();
		ScrPrint(0, 0, CFONT, "%-10.10s   %3.3s  %d", glCurAcq.szName, glCurAcq.szNii, glCurAcq.ucIndex); //Gillian debug
		if (ii==0)
		{
			ScrPrint(0, 2, ASCII, "TID:%s", glCurAcq.szTermID);
			ScrPrint(0, 3, ASCII, "MID:%s", glCurAcq.szMerchantID);
		}
		else
		{
			ScrPrint(0, 3, ASCII, "SECONDARY COMM");
		}

		ucShowComm = (ii ? glSysParam.stTxnCommCfg.ucCommTypeBak : glSysParam.stTxnCommCfg.ucCommType);

		if (ucShowComm==CT_NONE)
		{
			continue;
		}
		else if ((ucShowComm==CT_TCPIP) || (ucShowComm==CT_WIFI))
		{
			ScrPrint(0, 4, ASCII, "%.15s %.5s", glCurAcq.TxnTcpIp1, glCurAcq.TxnTcpPort1);
			ScrPrint(0, 5, ASCII, "%.15s %.5s", glCurAcq.TxnTcpIp2, glCurAcq.TxnTcpPort2);
			ScrPrint(0, 6, ASCII, "%.15s %.5s", glCurAcq.StlTcpIp1, glCurAcq.StlTcpPort1);
			ScrPrint(0, 7, ASCII, "%.15s %.5s", glCurAcq.StlTcpIp2, glCurAcq.StlTcpPort2);
		}
		//else if (ucShowComm==CT_PPP)
		// ...
		else if ((ucShowComm==CT_GPRS) || (ucShowComm==CT_CDMA) || (ucShowComm==CT_3G))//2015-11-23
		{
			ScrPrint(0, 4, ASCII, "%.15s %.5s", glCurAcq.TxnGPRSIp1, glCurAcq.TxnGPRSPort1);
			ScrPrint(0, 5, ASCII, "%.15s %.5s", glCurAcq.TxnGPRSIp2, glCurAcq.TxnGPRSPort2);
			ScrPrint(0, 6, ASCII, "%.15s %.5s", glCurAcq.StlGPRSIp1, glCurAcq.StlGPRSPort1);
			ScrPrint(0, 7, ASCII, "%.15s %.5s", glCurAcq.StlGPRSIp2, glCurAcq.StlTcpPort2);
		}
		else
		{
			ScrPrint(0, 4, ASCII, "%.21s", glCurAcq.TxnTelNo1);
			ScrPrint(0, 5, ASCII, "%.21s", glCurAcq.TxnTelNo2);
			ScrPrint(0, 6, ASCII, "%.21s", glCurAcq.StlTelNo1);
			ScrPrint(0, 7, ASCII, "%.21s", glCurAcq.StlTelNo2);
		}

		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if ((ucKey==KEYCANCEL) || (ucKey==NOKEY))
		{
			return ucKey;
		}
	}

	return ucKey;
}

int SetAcqTransComm(uchar ucCommType)
{
	int		iRet, MyRet;
	char Result[32]= {0}, Port_DCC[5] = "9457", Port_DCD[6] = "48081", Port_INST[4] = "443";
	switch(ucCommType)
	{
	case CT_TCPIP:
	case CT_SSL://Jason 2015.01.27 11:31  //Gillian 2016-8-23 
	case CT_WIFI:
#ifdef PP_SSL	
	iRet = LoadSysParam();

	if(!ChkIfAmex() && !ChkIfDiners())
	{
		if(glSysParam.stTxnCommCfg.ucEnableSSL_URL)   //Gillian 2016-8-18
		{
            if(ChkIfFubon()){
                if(DnsResolve(glSysParam.stTxnCommCfg.szSSL_URL, glSysParam.stTxnCommCfg.szSSL_BackupIP, 32))
                {
                    memcpy(glSysParam.stTxnCommCfg.szSSL_BackupIP, glSysParam.stTxnCommCfg.szSSL_URL, strlen(glSysParam.stTxnCommCfg.szSSL_URL));
                }
                strcpy(gCurComPara.LocalParam.szRemoteIP_1, glSysParam.stTxnCommCfg.szSSL_BackupIP);
                strcpy(gCurComPara.LocalParam.szRemoteIP_2, glSysParam.stTxnCommCfg.szSSL_BackupIP);
                strcpy(glCurAcq.stTxnGPRSInfo[0].szIP, gCurComPara.LocalParam.szRemoteIP_1);
				strcpy(glCurAcq.stTxnGPRSInfo[1].szIP, gCurComPara.LocalParam.szRemoteIP_2);

                strcpy(gCurComPara.LocalParam.szRemotePort_1, glSysParam.stTxnCommCfg.szSSL_Port);
                strcpy(gCurComPara.LocalParam.szRemotePort_2, glSysParam.stTxnCommCfg.szSSL_Port);
                strcpy(glCurAcq.stTxnGPRSInfo[0].szPort, gCurComPara.LocalParam.szRemotePort_1);
				strcpy(glCurAcq.stTxnGPRSInfo[1].szPort, gCurComPara.LocalParam.szRemotePort_2);
            }
            else  {
    			memset(Result, 0, sizeof(Result));
    		    memset(glSysParam.stTxnCommCfg.szPP_URL, 0, sizeof(glSysParam.stTxnCommCfg.szPP_URL));
    			memcpy(glSysParam.stTxnCommCfg.szPP_URL, "terminal.planetpayment.net", strlen("terminal.planetpayment.net"));
    			MyRet = DnsResolve(glSysParam.stTxnCommCfg.szPP_URL, Result, 32);
    			strcpy(gCurComPara.LocalParam.szRemoteIP_1, Result);
    			strcpy(gCurComPara.LocalParam.szRemoteIP_2, Result);
    			strcpy(gCurComPara.LocalParam.szRemotePort_1, Port_DCC);
    			strcpy(gCurComPara.LocalParam.szRemotePort_2, Port_DCD);
            }
            
			if (ChkCurAcqName("DCC_BEA", TRUE))
			{
				strcpy(glCurAcq.stTxnTCPIPInfo[0].szIP, gCurComPara.LocalParam.szRemoteIP_1);
				strcpy(glCurAcq.stTxnTCPIPInfo[0].szPort, gCurComPara.LocalParam.szRemotePort_1);
				strcpy(glCurAcq.stTxnTCPIPInfo[1].szIP, gCurComPara.LocalParam.szRemoteIP_1);
				strcpy(glCurAcq.stTxnTCPIPInfo[1].szPort, gCurComPara.LocalParam.szRemotePort_1);
			}
			else if (ChkCurAcqName("BEA", FALSE) || ChkCurAcqName("JCB", FALSE))
			{
				strcpy(glCurAcq.stTxnTCPIPInfo[0].szIP, gCurComPara.LocalParam.szRemoteIP_2);
				strcpy(glCurAcq.stTxnTCPIPInfo[0].szPort, gCurComPara.LocalParam.szRemotePort_2);
				strcpy(glCurAcq.stTxnTCPIPInfo[1].szIP, gCurComPara.LocalParam.szRemoteIP_2);
				strcpy(glCurAcq.stTxnTCPIPInfo[1].szPort, gCurComPara.LocalParam.szRemotePort_2);
			}
		}
	}
	else if (ChkCurAcqName("AMEX_INST", FALSE))
	{
		memset(Result, 0, sizeof(Result));
		memset(glSysParam.stTxnCommCfg.szPP_URL, 0, sizeof(glSysParam.stTxnCommCfg.szPP_URL));
		//Gillian 20161011
		memcpy(glSysParam.stTxnCommCfg.szPP_URL, "qwww318.americanexpress.com", strlen("qwww318.americanexpress.com"));
		MyRet = DnsResolve(glSysParam.stTxnCommCfg.szPP_URL, Result, 32);
		strcpy(glCurAcq.stTxnTCPIPInfo[0].szIP, Result);
		strcpy(glCurAcq.stTxnTCPIPInfo[0].szPort, Port_INST);
		strcpy(glCurAcq.stTxnTCPIPInfo[1].szIP, Result);
		strcpy(glCurAcq.stTxnTCPIPInfo[1].szPort, Port_INST);
	}

#endif
		iRet = GetRemoteIp("TRANS IP 1",  TRUE, &glCurAcq.stTxnTCPIPInfo[0]);
		if (iRet)
		{
			break;
		}
		iRet = GetRemoteIp("TRANS IP 2",  TRUE, &glCurAcq.stTxnTCPIPInfo[1]);
		break;
	case CT_GPRS:
	case CT_CDMA:
	case CT_3G://2015-11-23
		iRet = GetRemoteIp("TRANS IP 1",  TRUE, &glCurAcq.stTxnGPRSInfo[0]);
		if (iRet)
		{
			break;
		}
		iRet = GetRemoteIp("TRANS IP 2",  TRUE, &glCurAcq.stTxnGPRSInfo[1]);
		//iRet = GetRemoteIp("SETTLE 1st", TRUE, &glCurAcq.stStlGPRSInfo[0]);
		//iRet = GetRemoteIp("SETTLE 2nd", TRUE, &glCurAcq.stStlGPRSInfo[1]);
	    break;
		break;
	case CT_MODEM:
		iRet = SetAcqTransTelNo();
	case CT_RS232:
	case CT_NONE:
	default:
		iRet = 0;
	    break;
	}

	return iRet;
}

// 设置当前收单行交易电话号码
int SetAcqTransTelNo(void)
{
	if( SetTel(glCurAcq.TxnTelNo1, (uchar *)_T("TRANS TELNO 1"))!=0 )
	{
		return ERR_USERCANCEL;
	}
	memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));

	if( SetTel(glCurAcq.TxnTelNo2, (uchar *)_T("TRANS TELNO 2"))!=0 )
	{
		return ERR_USERCANCEL;
	}
	memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));

	if( SetTel(glCurAcq.StlTelNo1, (uchar *)_T("SETTTLE TELNO 1"))!=0 )
	{
		return ERR_USERCANCEL;
	}
	memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));

	if( SetTel(glCurAcq.StlTelNo2, (uchar *)_T("SETTTLE TELNO 2"))!=0 )
	{
		return ERR_USERCANCEL;
	}
	memcpy(&glSysParam.stAcqList[glCurAcq.ucIndex], &glCurAcq, sizeof(ACQUIRER));

	return 0;
}

// 设置电话号码
int SetTel(uchar *pszTelNo, uchar *pszPromptInfo)
{
	ScrClrLine(2, 7);
	PubDispString(pszPromptInfo, DISP_LINE_LEFT|2);
	if( PubGetString(ALPHA_IN|ECHO_IN, 0, 24, pszTelNo, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}

	return 0;
}
//2015-11-23
int Set3GParam(WIRELESS_PARAM *pstParam)
{
    /*=======BEGIN: Jason VER20150716 2015.07.16  9:55 modify===========*/
    uchar buf[30] = {0},buf1[100+1] = {0};
    uchar key;
    uchar szURL[100 + 1] = {0};
    uchar szIP[22];
    int i, dnsResult,iRetryTime,iRet;
	int portLen;
	char temp[100], szBuff[100];
    /*====================== END======================== */

	if (pstParam==NULL)
	{
		return ERR_NO_DISP;
	}

	PubShowTitle(TRUE, (uchar *)"SETUP 3G  ");
	ScrClrLine(2, 7);
	PubDispString("APN", 2|DISP_LINE_LEFT);
	if( PubGetString(ALPHA_IN|ECHO_IN, 0, 32, pstParam->szAPN, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}

	ScrClrLine(2, 7);
	PubDispString("LOGIN NAME", 2|DISP_LINE_LEFT);
	if( PubGetString(ALPHA_IN|ECHO_IN, 0, 32, pstParam->szUID, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}

	ScrClrLine(2, 7);
	PubDispString("LOGIN PWD", 2|DISP_LINE_LEFT);
	if( PubGetString(ALPHA_IN|ECHO_IN, 0, 16, pstParam->szPwd, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}

	ScrClrLine(2, 7);
	PubDispString("SIM PIN", 2|DISP_LINE_LEFT);
	if( PubGetString(ALPHA_IN|ECHO_IN, 0, 16, pstParam->szSimPin, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}

	//while(1)
	{
#ifdef _S_SERIES_
		PubDispString(_T("SELECT MODE"), 0|DISP_LINE_CENTER|DISP_LINE_REVER);
		//if (glSysParam.stTxnCommCfg.szSSL_URL == '0')
  //      {
  //          PubDispString(_T("CUR:IP"), 2|DISP_LINE_RIGHT);
  //      }
  //      else if (glb_file.IPorURL == '1')
  //      {
  //          PubDispString(_T("CUR:URL"), 2|DISP_LINE_RIGHT);
  //      }
        PubDispString(_T("1.IP"), 4|DISP_LINE_LEFT);
        PubDispString(_T("2.URL"), 6|DISP_LINE_LEFT);
        do
        {
            key = getkey();
            if (key == KEYCANCEL)
            {
                return ERR_USERCANCEL;
            }
        }
        while((key < KEY1 || key > KEY2));
#else
		key = KEY1;
#endif
		if (KEY1 == key)
        {
            ScrClrLine(2,7);
            PubDispString(_T("SET HOST IP1"), 2|DISP_LINE_LEFT);
            PubGetString(ALPHA_IN|ECHO_IN,7,15,glCommCfg.stWirlessPara.szRemoteIP_1,60);
            //PutEnv("GIP1", glCommCfg.stWirlessPara.szRemoteIP_1);

			ScrClrLine(2,7);
            PubDispString(_T("SET HOST PORT1"), 2|DISP_LINE_LEFT);
			memset(szBuff, 0, sizeof(szBuff));
			if(PubGetString( NUM_IN | ECHO_IN, 1, 6, (uchar *)szBuff, USER_OPER_TIMEOUT) == 0)
			{
				portLen = strlen(szBuff);

				if(portLen != 0  && portLen <=5)
				{
					memset(glCommCfg.stWirlessPara.szRemotePort_1, 0, PORT_MAX_LEN+1);
					memcpy(glCommCfg.stWirlessPara.szRemotePort_1, szBuff, portLen);
				}
			}
			//PutEnv("GPort1",glCommCfg.stWirlessPara.szRemotePort_1);

            ScrClrLine(2,7);
            PubDispString(_T("SET HOST IP2"), 2|DISP_LINE_LEFT);
            PubGetString(ALPHA_IN|ECHO_IN,7,15,glCommCfg.stWirlessPara.szRemoteIP_2,60);
            //PutEnv("GIP2", glCommCfg.stWirlessPara.szRemoteIP_2);

			ScrClrLine(2,7);
            PubDispString(_T("SET HOST PORT2"), 2|DISP_LINE_LEFT);
			memset(szBuff, 0, sizeof(szBuff));
			if(PubGetString( NUM_IN | ECHO_IN, 1, 6, (uchar *)szBuff, USER_OPER_TIMEOUT) == 0)
			{
				portLen = strlen(szBuff);

				if(portLen != 0  && portLen <=5)
				{
					memset(glCommCfg.stWirlessPara.szRemotePort_2, 0, PORT_MAX_LEN+1);
					memcpy(glCommCfg.stWirlessPara.szRemotePort_2, szBuff, portLen);
				}
			}
			//PutEnv("GPort2",glCommCfg.stWirlessPara.szRemotePort_2);
			//sprintf((char *)glSysParam.stTxnCommCfg.szSSL_BackupIP, "%.15s",glCommCfg.stWirlessPara.szRemoteIP_1);
			//sprintf((char *)glSysParam.stTxnCommCfg.szSSL_BackupIP, "%.15s",glCommCfg.stWirlessPara.szRemoteIP_2);
			//sprintf((char *)glSysParam.stTxnCommCfg.szSSL_Port, "%.5s",glCommCfg.stWirlessPara.szRemotePort_1);
			//sprintf((char *)glSysParam.stTxnCommCfg.szSSL_Port, "%.5s",glCommCfg.stWirlessPara.szRemotePort_2);
        }
        else
        {
			//glSysParam.stTxnCommCfg.ucEnableSSL = TRUE;
			//glSysParam.stTxnCommCfg.ucEnableSSL_URL = TRUE;
			//2015-7-23 -----------------------------------START-----------
			iRetryTime = 10;
			while(iRetryTime--)
            {
				iRet = WlPppLogin(pstParam->szAPN,pstParam->szUID,pstParam->szPwd,0xFF, 0, 3600);

                if (iRet != 0)
                {
                    DelayMs(500);
                    continue;
                }

                iRet = WlPppCheck();
                if (iRet == 0)
                {
                    break;
                }
            }
			//2015-7-23 -----------------------------------END-----------
			//SetSSL_URL();
			//sprintf((char *)glCommCfg.stWirlessPara.szRemoteIP_1, "%.15s",glSysParam.stTxnCommCfg.szSSL_BackupIP);
			//sprintf((char *)glCommCfg.stWirlessPara.szRemoteIP_2, "%.15s",glSysParam.stTxnCommCfg.szSSL_BackupIP);
			//sprintf((char *)glCommCfg.stWirlessPara.szRemotePort_1, "%.5s",glSysParam.stTxnCommCfg.szSSL_Port);
			//sprintf((char *)glCommCfg.stWirlessPara.szRemotePort_2, "%.5s",glSysParam.stTxnCommCfg.szSSL_Port);
			//PutEnv("GIP1", glCommCfg.stWirlessPara.szRemoteIP_1);
			//PutEnv("GPort1",glCommCfg.stWirlessPara.szRemotePort_1);
			//PutEnv("GIP2", glCommCfg.stWirlessPara.szRemoteIP_2);
			//PutEnv("GPort2",glCommCfg.stWirlessPara.szRemotePort_2);
			//
        }
        /*====================== END======================== */
	}
	return 0;
}
int SetWirelessParam(WIRELESS_PARAM *pstParam)
{
	if (pstParam==NULL)
	{
		return ERR_NO_DISP;
	}

	PubShowTitle(TRUE, (uchar *)"SETUP WIRELESS  ");

	ScrClrLine(2, 7);
	PubDispString("APN", 2|DISP_LINE_LEFT);
	if( PubGetString(ALPHA_IN|ECHO_IN, 0, 32, pstParam->szAPN, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}

	ScrClrLine(2, 7);
	PubDispString("LOGIN NAME", 2|DISP_LINE_LEFT);
	if( PubGetString(ALPHA_IN|ECHO_IN, 0, 32, pstParam->szUID, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}

	ScrClrLine(2, 7);
	PubDispString("LOGIN PWD", 2|DISP_LINE_LEFT);
	if( PubGetString(ALPHA_IN|ECHO_IN, 0, 16, pstParam->szPwd, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}

	ScrClrLine(2, 7);
	PubDispString("SIM PIN", 2|DISP_LINE_LEFT);
	if( PubGetString(ALPHA_IN|ECHO_IN, 0, 16, pstParam->szSimPin, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}

	return 0;
}

void SyncWirelessParam(WIRELESS_PARAM *pstDst, WIRELESS_PARAM *pstSrc)
{
	strcpy((char *)(pstDst->szAPN),    (char *)(pstSrc->szAPN));
	strcpy((char *)(pstDst->szUID),    (char *)(pstSrc->szUID));
	strcpy((char *)(pstDst->szPwd),    (char *)(pstSrc->szPwd));
	strcpy((char *)(pstDst->szSimPin), (char *)(pstSrc->szSimPin));
	strcpy((char *)(pstDst->szDNS),    (char *)(pstSrc->szDNS));
}

int SetPANMask(void)//2014-11-5 show clearly if PAN MASK is use or no use
{
	uchar ucTemp;
	uchar ucKey;
	uchar szBuff[5];
	
	ucTemp = glSysParam.stEdcInfo.ucPANmaskFlag;
	
	while( 1 )
	{
		ScrClrLine(2, 7);
		PubDispString("PAN MASK", DISP_LINE_CENTER|2);
		sprintf((char *)szBuff, "(%c)", ucTemp ? '1' : '0');
		PubDispString(szBuff,       DISP_LINE_CENTER|4);
		PubDispString("1.USE 0.NO USE", DISP_LINE_CENTER|6);
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( ucKey==KEYCANCEL || ucKey==NOKEY )
		{
			return ERR_USERCANCEL;
		}
		else if( ucKey==KEYENTER )
		{
			break;
		}
		else if ( ucKey==KEY0 )
		{
			ucTemp = 0;//2014-11-11
		}
		else if ( ucKey==KEY1 )
		{
			ucTemp = 1;
		}
	}	
	glSysParam.stEdcInfo.ucPANmaskFlag = ucTemp;
	return 0;

	//ucKey = PubSelectOneTwo("PAN MASK", "USE", "NO USE");//2014-11-5 show if on or off
	//switch(ucKey)
	//{
	//case KEY1:
	//	glSysParam.stEdcInfo.ucPANmaskFlag = 1;
	//	return 0;
	//case KEY2:
	//	glSysParam.stEdcInfo.ucPANmaskFlag = 0;
	//	return 0;
	//case KEYENTER:
	//	return 0;
	//case KEYCANCEL:
	//default:
	//	return ERR_USERCANCEL;
	//};
}
int Setkeyin(void)
{
	uchar ucKey;
	ucKey = PubSelectOneTwo("KEY-IN ONLY", "USE", "NO USE");
	switch(ucKey)
	{
	case KEY1:
		glSysParam.stEdcInfo.ucINPUT_ONLY = 1;
		return 0;
	case KEY2:
		glSysParam.stEdcInfo.ucINPUT_ONLY = 0;
		return 0;
	case KEYENTER:
		return 0;
	case KEYCANCEL:
	default:
		return ERR_USERCANCEL;
	};
}

// 设置EDC参数
void SetEdcParam(uchar ucPermission)
{
	while( 1 )
	{
		PubShowTitle(TRUE, (uchar *)_T("SETUP EDC       "));
		if (SetTermCurrency(ucPermission)!=0)
		{
			break;
		}
		if( SetMerchantName(ucPermission)!=0 )
		{
			break;
		}
		if( SetPEDMode()!=0 )
		{
			break;
		}

		PubShowTitle(TRUE, (uchar *)_T("SETUP EDC       "));
		if( SetAcceptTimeOut()!=0 )
		{
			break;
		}
		if( SetPrinterType()!=0 )
		{
			break;
		}
		if( SetNumOfReceipt()!=0 )
		{
			break;
		}
		//2014-11-4 set print total before settle 
		if( SetPrnStlTotal()!=0 )
		{
			break;
		}
		if( SetGetSysTraceNo(ucPermission)!=0 )
		{
			break;
		}
		if( SetGetSysInvoiceNo(ucPermission)!=0 )
		{
			break;
		}
		if (SetPANMask()!=0)
		{
			break;
		}
		if(Setkeyin()!=0)
		{
			break;
		}
		if( SetCasinoMode()!=0 )  //Lijy 2010-12-30 set casino mode
		{
			break;
		}
		if (SetNoSignLmt()!=0)
		{
			break;
		}
#ifdef SUPPORT_TABBATCH
		PubShowTitle(TRUE, (uchar *)_T("SETUP EDC       "));
		if (SetAuthTabBatch()!=0) //Gillian 2016-7-14 Gillian 2016-7-28 ecr issue
		{
			break;
		}
#endif
		PubShowTitle(TRUE, (uchar *)_T("SETUP EDC       "));
		ModifyOptList(glSysParam.stEdcInfo.sOption, 'E', ucPermission);
		//if (PPDCC_SetCurAcq()==0) //removed by richard 20170120
		{
			ModifyOptList(glSysParam.stEdcInfo.sExtOption, 'e', ucPermission);
		}
		break;
	}

	SaveEdcParam();

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
long ViewGetValue(uchar ucEdit, void *pszFirstMsg, void *pszSecMsg,
				  ulong ulMin, ulong ulMax, long lOrgValue)
{
	uchar	szBuff[32], ucMinDigit, ucMaxDigit, ucTemp;
	ulong	ulTemp;

	PubASSERT(ulMax<2147483648);

	ulTemp = ulMin;
	ucMinDigit = 0;
	do{ucMinDigit++;}while (ulTemp/=10);

	ulTemp = ulMax;
	ucMaxDigit = 0;
	do{ucMaxDigit++;}while (ulTemp/=10);

	memset(szBuff, 0, sizeof(szBuff));
	if (lOrgValue>=0)
	{
		sprintf((char *)szBuff, "%ld", lOrgValue);
	}

	while (1)
	{
		ScrClrLine(2, 7);
		if (pszFirstMsg!=NULL)
		{
			PubDispString(_T(pszFirstMsg), 2|DISP_LINE_LEFT);
		}

		if (pszSecMsg!=NULL)
		{
			PubDispString(_T(pszSecMsg), 4|DISP_LINE_LEFT);
		}

		if (ucEdit)
		{
			// Allow modify 允许修改
			if (PubGetString(NUM_IN|ECHO_IN, ucMinDigit, ucMaxDigit, szBuff, USER_OPER_TIMEOUT)!=0)
			{
				return -2;
			}
			ulTemp = (ulong)atol((char *)szBuff);
			if ((ulTemp<ulMin) || (ulTemp>ulMax))
			{
				ScrClrLine(4, 7);
				PubDispString(_T("INVALID VALUE"), 5|DISP_LINE_LEFT);
				PubBeepErr();
				PubWaitKey(3);
				continue;
			}

			if (ulTemp!=(ulong)lOrgValue)
			{
				return (long)ulTemp;
			}
			return -1;
		}
		else
		{
			// Read only 只读
			PubDispString(szBuff, 6|DISP_LINE_RIGHT);
			ucTemp = PubWaitKey(USER_OPER_TIMEOUT);
			if ((ucTemp==KEYCANCEL) || (ucTemp==NOKEY))
			{
				return -2;
			}
			return -1;
		}
	}
}

uchar ViewGetEppType(void *pszFirstMsg, ulong ulMin, ulong ulMax, uchar lOrgValue)
{
	static unsigned char uEppType[] = "05";
	uchar	ulTemp;

	if (lOrgValue>=0)
	{
		ScrClrLine(2, 7);
		if (pszFirstMsg!=NULL)
		{
			PubDispString((pszFirstMsg), 2|DISP_LINE_LEFT);
		}
		sprintf((char *)uEppType, "%02d", lOrgValue);
		PubDispString(uEppType, 6|DISP_LINE_RIGHT);
	}
	while (1)
	{
		
		if (PubGetString(NUM_IN|ECHO_IN, ulMin, ulMax, uEppType, USER_OPER_TIMEOUT)!=0)
		{
			return -2;
		}
		ulTemp = atoi((char *)uEppType);
		return ulTemp;
	}
}

int SetTermCurrency(uchar ucPermission)
{
	uchar	szBuff[32], ucTemp;
	long	lTemp;
	CURRENCY_CONFIG	stCurrency;

    // Country Code
	while (1)
	{
		ScrClrLine(2, 7);
		PubBcd2Asc0(glSysParam.stEdcInfo.stLocalCurrency.sCountryCode, 2, szBuff);
		PubDispString(_T("AREA CODE"), 2|DISP_LINE_LEFT);
        // Allow modify 允许修改
        if (PubGetString(NUM_IN|ECHO_IN, 3, 3, szBuff+1, USER_OPER_TIMEOUT)!=0)
        {
            return ERR_USERCANCEL;
        }
        
        PubAsc2Bcd(szBuff, 3, glSysParam.stEdcInfo.stLocalCurrency.sCountryCode);
		break;
	}

	// Currency
	while (1)
	{
		ScrClrLine(2, 7);
		PubBcd2Asc0(glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, 2, szBuff);
		memmove(szBuff, szBuff+1, 4);

		PubDispString(_T("CURRENCY CODE"), 2|DISP_LINE_LEFT);

		if (ucPermission<PM_HIGH)
		{
			// Read only 只读
			PubDispString(szBuff, 6|DISP_LINE_RIGHT);
			ucTemp = PubWaitKey(USER_OPER_TIMEOUT);
			if ((ucTemp==KEYCANCEL) || (ucTemp==NOKEY))
			{
				return ERR_USERCANCEL;
			}
		}
		else
		{
			// Allow modify 允许修改
			if (PubGetString(NUM_IN|ECHO_IN, 3, 3, szBuff, USER_OPER_TIMEOUT)!=0)
			{
				return ERR_USERCANCEL;
			}
			if (FindCurrency(szBuff, &stCurrency)!=0)
			{
				ScrClrLine(4, 7);
				PubDispString(_T("INVALID CURRENCY"), 5|DISP_LINE_LEFT);
				PubBeepErr();
				PubWaitKey(3);
				continue;
			}

			ScrClrLine(2, 7);
			sprintf((char *)szBuff, "%.3s %02X%02X",
				stCurrency.szName, stCurrency.sCurrencyCode[0], stCurrency.sCurrencyCode[1]);
			PubDispString(szBuff, 2|DISP_LINE_LEFT);
			if (!AskYesNo())
			{
				continue;
			}
			sprintf((char *)glSysParam.stEdcInfo.stLocalCurrency.szName, "%.3s", stCurrency.szName);
			memcpy(glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, stCurrency.sCurrencyCode, 2);
		}
		break;
	}

	// Input decimal position value, 0<=x<=3
	// for JPY and KRW, x=0; for TWD, x=0 or x=2
	lTemp = ViewGetValue((uchar)(ucPermission>PM_MEDIUM), _T("DECIMAL POSITION"), NULL,
						0, 3, (long)glSysParam.stEdcInfo.stLocalCurrency.ucDecimal);
	if (lTemp==-2)
	{
		return ERR_USERCANCEL;
	}
	if (lTemp>=0)
	{
		glSysParam.stEdcInfo.stLocalCurrency.ucDecimal = (uchar)lTemp;
	}

	// Input ignore digit value, 0<=x<=3
	// for JPY and KRW, x=2; for TWD, when decimal=0, x=2; decimal=2, x=0;
	lTemp = ViewGetValue((uchar)(ucPermission>PM_MEDIUM), _T("IGNORE DIGIT"), NULL,
						0, 3, (long)glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit);
	if (lTemp==-2)
	{
		return ERR_USERCANCEL;
	}
	if (lTemp>=0)
	{
		glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit = (uchar)lTemp;
	}

	//Gillian 20161031
	glSysParam.stEdcInfo.szEppType2 = glSysParam.stEdcInfo.szEppType;
	lTemp = ViewGetEppType(_T("AE EPP DATA TYPE"),2, 2, glSysParam.stEdcInfo.szEppType2);
	if (lTemp==-2)
	{
		return ERR_USERCANCEL;
	}
	if (lTemp>=0)
	{
		glSysParam.stEdcInfo.szEppType = (uchar)lTemp;
	}

	return 0;
}

int SetMerchantName(uchar ucPermission)
{
	uchar	szBuff[46+1];

	if (ucPermission<PM_HIGH)	// Not allow to set
	{
		return 0;
	}

	ScrClrLine(2, 7);
	PubDispString(_T("MERCHANT NAME"), DISP_LINE_LEFT|2);
	sprintf((char *)szBuff, "%.23s", (char *)glSysParam.stEdcInfo.szMerchantName);
	
	if( PubGetString(ALPHA_IN|ECHO_IN, 1, 23, szBuff, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}
	if (strcmp((char *)glSysParam.stEdcInfo.szMerchantName, (char *)szBuff)!=0)
	{
		sprintf((char *)glSysParam.stEdcInfo.szMerchantName, "%.23s", (char *)szBuff);
	}

	ScrClrLine(2, 7);
	PubDispString(_T("MERCHANT ADDR"), DISP_LINE_LEFT|2);
	sprintf((char *)szBuff, "%.46s", (char *)glSysParam.stEdcInfo.szMerchantAddr);

	if( PubGetString(ALPHA_IN|ECHO_IN, 1, 46, szBuff, USER_OPER_TIMEOUT)!=0 )
	{
		return ERR_USERCANCEL;
	}
	if (strcmp((char *)glSysParam.stEdcInfo.szMerchantAddr, (char *)szBuff)!=0)
	{
		sprintf((char *)glSysParam.stEdcInfo.szMerchantAddr, "%.23s", (char *)szBuff);
	}

	return 0;
}

int SetGetSysTraceNo(uchar ucPermission)
{
	uchar	szBuff[6+1];

	ScrClrLine(2, 7);
	PubDispString("S.T.A.N.", DISP_LINE_LEFT|2);
	sprintf((char *)szBuff, "%06ld", glSysCtrl.ulSTAN);

	if (ucPermission>PM_LOW)
	{
		if( PubGetString(NUM_IN|ECHO_IN, 1, 6, szBuff, USER_OPER_TIMEOUT)!=0 )
		{
			return ERR_USERCANCEL;
		}
		glSysCtrl.ulSTAN = (ulong)atol((char *)szBuff);
		SaveSysCtrlBase();
	} 
	else
	{
		PubDispString(szBuff, DISP_LINE_RIGHT|6);
		PubYesNo(USER_OPER_TIMEOUT);
	}

	return 0;
}

int SetGetSysInvoiceNo(uchar ucPermission)
{
	uchar	szBuff[6+1];

	ScrClrLine(2, 7);
	PubDispString("TRACE NO", DISP_LINE_LEFT|2);
	sprintf((char *)szBuff, "%06ld", glSysCtrl.ulInvoiceNo);

	if (ucPermission>PM_LOW)
	{
		if( PubGetString(NUM_IN|ECHO_IN, 1, 6, szBuff, USER_OPER_TIMEOUT)!=0 )
		{
			return ERR_USERCANCEL;
		}
		glSysCtrl.ulInvoiceNo = (ulong)atol((char *)szBuff);
		SaveSysCtrlBase();
	} 
	else
	{
		PubDispString(szBuff, DISP_LINE_RIGHT|6);
		PubYesNo(USER_OPER_TIMEOUT);
	}

	return 0;
}

// Select PED mode used.
int SetPEDMode(void)
{
	int		iCnt;
	char	szBuff[32];
	uchar	ucKey;

#ifdef _P60_S1_
	glSysParam.stEdcInfo.ucPedMode = PED_INT_PXX;
	return 0;

#else

	struct {
		char	szPrompt[16+1];
		uchar	ucPEDMode;
	} stModeList[] = {
		{"PINPAD        ", PED_EXT_PP},
#if defined(_P60_S1_) || defined(_P80_) || defined(_P90_)
		{"PED           ", PED_INT_PXX},
#endif
#ifdef _S_SERIES_
		{"PCI PED       ", PED_INT_PCI},
		{"EXT PCI PINPAD", PED_EXT_PCI},
#endif
		{"", 0}
	};

	sprintf(szBuff, "CUR: NONE");
	for (iCnt=0; iCnt<sizeof(stModeList)/sizeof(stModeList[0]); iCnt++)
	{
		if (glSysParam.stEdcInfo.ucPedMode==stModeList[iCnt].ucPEDMode)
		{
			sprintf(szBuff, "CUR: %.16s", stModeList[iCnt].szPrompt);
			break;
		}
	}

	while (1)
	{
		ScrClrLine(2, 7);
		PubDispString(szBuff, 2|DISP_LINE_LEFT);
		for (iCnt=0; iCnt<sizeof(stModeList)/sizeof(stModeList[0])-1; iCnt++)
		{
			ScrPrint(0, (uchar)(4+iCnt), ASCII, "%d.%.14s", iCnt+1, stModeList[iCnt].szPrompt);
		}

		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if ((ucKey==KEYCANCEL) || (ucKey==NOKEY))
		{
			return ERR_USERCANCEL;
		}
		if (ucKey==KEYENTER)
		{
			return 0;
		}
		if ((ucKey<KEY1) || (ucKey>KEY0+iCnt))
		{
			continue;
		}
	
		glSysParam.stEdcInfo.ucPedMode = stModeList[ucKey-KEY1].ucPEDMode;
		return 0;
	}

#endif
}

// 输入交易成功时确认信息显示时间
int SetAcceptTimeOut(void)
{
	uchar	szBuff[2+1];

	sprintf((char *)szBuff, "%d", glSysParam.stEdcInfo.ucAcceptTimeout);
	while( 1 )
	{
		ScrClrLine(2, 7);
		PubDispString("Confirm Timeout", DISP_LINE_LEFT|2);
		PubDispString("[0..60]",       DISP_LINE_LEFT|4);
		if( PubGetString(NUM_IN|ECHO_IN, 1, 2, szBuff, USER_OPER_TIMEOUT)!=0 )
		{
			return ERR_USERCANCEL;
		}
		if( atoi((char *)szBuff)<=60 )
		{
			break;
		}

		ScrClrLine(4, 7);
		PubDispString(_T("INVALID INPUT"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(3);
	}
	glSysParam.stEdcInfo.ucAcceptTimeout = (uchar)atoi((char *)szBuff);

	return 0;
}

int SetPrinterType(void)
{
	char 	ucKey, szBuff[16+1];

	// 仅适用于分离式打印机
	if (!ChkTerm(_TERMINAL_S60_))
	{
		return 0;
	}

	sprintf(szBuff, "PRINT: %s", (ChkIfThermalPrinter() ? "THERMAL" : "SPROCKET"));
	ucKey = PubSelectOneTwo((uchar *)szBuff, "THERMAL", "SPROCKET");
	switch(ucKey)
	{
	case KEY1:
		glSysParam.stEdcInfo.ucPrinterType = 1;
		return 0;
	case KEY2:
		glSysParam.stEdcInfo.ucPrinterType = 0;
		return 0;
	case KEYENTER:
	    return 0;
	case KEYCANCEL:
	default:
	    return ERR_USERCANCEL;
	};
}
int  SetPrnStlTotal(void)//2014-11-4
{
	uchar ucTemp;
	uchar ucKey;
	uchar szBuff[5];
	
	ucTemp = glSysParam.stEdcInfo.ucPrnStlTotalFlag;
	
	while( 1 )
	{
		ScrClrLine(2, 7);
		PubDispString("TOTAL BEF SETTLE", DISP_LINE_CENTER|2);
		sprintf((char *)szBuff, "(%c)", ucTemp ? '1' : '0');
		PubDispString(szBuff,       DISP_LINE_CENTER|4);
		PubDispString("1.ON 0.OFF", DISP_LINE_CENTER|6);
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( ucKey==KEYCANCEL || ucKey==NOKEY )
		{
			return ERR_USERCANCEL;
		}
		else if( ucKey==KEYENTER )
		{
			break;
		}
		else if ( ucKey==KEY0 )
		{
			ucTemp = FALSE;
		}
		else if ( ucKey==KEY1 )
		{
			ucTemp = TRUE;
		}
	}	
	glSysParam.stEdcInfo.ucPrnStlTotalFlag = ucTemp;
	return 0;
}

// Lijy 2010-12-30 设置赌场模式
int SetCasinoMode(void)
{
	uchar ucTemp;
	uchar ucKey;
	uchar szBuff[5];
	
	ucTemp = glSysParam.stEdcInfo.ucCasinoMode;
	
	while( 1 )
	{
		ScrClrLine(2, 7);
		PubDispString("CASINO MODE", DISP_LINE_CENTER|2);
		sprintf((char *)szBuff, "(%c)", ucTemp ? '1' : '0');
		PubDispString(szBuff,       DISP_LINE_CENTER|4);
		PubDispString("1.ON 0.OFF", DISP_LINE_CENTER|6);
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( ucKey==KEYCANCEL || ucKey==NOKEY )
		{
			return ERR_USERCANCEL;
		}
		else if( ucKey==KEYENTER )
		{
			break;
		}
		else if ( ucKey==KEY0 )
		{
			ucTemp = FALSE;
		}
		else if ( ucKey==KEY1 )
		{
			ucTemp = TRUE;
		}
	}
	
	
	glSysParam.stEdcInfo.ucCasinoMode = ucTemp;
	
	//Lijy 2010-12-30 如果是赌场模式，则设置港币兑葡元汇率,默认汇率为1.032
	if(glSysParam.stEdcInfo.ucCasinoMode == TRUE)
	{
		uchar szTempBuff[10];
		
		memset(szTempBuff, 0x00, sizeof(szTempBuff));
		memcpy(szTempBuff,glSysParam.stEdcInfo.szExchgeRate,10);
		
		ScrClrLine(2, 7);
		PubDispString("EXCHANGE RATE", DISP_LINE_LEFT|2);
		PubDispString("HKD-MOP", DISP_LINE_CENTER|4);
		if( PubGetString(ALPHA_IN|CARRY_IN, 1, 9, szTempBuff, USER_OPER_TIMEOUT)!=0 )
		{
			return ERR_USERCANCEL;
		}
		strcpy(glSysParam.stEdcInfo.szExchgeRate,szTempBuff); 
	}
	return 0;
}

#if 1//2015-6-16 allow revise limit to 0

int SetNoSignLmt(void)
{
	uchar	szBuff[4+1];	
	
	sprintf((char *)szBuff, "%lu", glSysParam.stEdcInfo.ulNoSignLmtVisa/100);
	while( 1 )
	{
		ScrClrLine(2, 7);
		PubDispString("No sign amt VISA", DISP_LINE_LEFT|2);
		PubDispString("[0..1000]",       DISP_LINE_LEFT|4);
		if( PubGetString(NUM_IN|ECHO_IN, 1, 4, szBuff, USER_OPER_TIMEOUT)!=0 )
		{
			return ERR_USERCANCEL;
		}
		if( atoi((char *)szBuff)<=1000 )
		{
			break;
		}
		ScrClrLine(2, 7);
		PubDispString("块礚", DISP_LINE_LEFT|3);
		PubDispString("INVALID INPUT", DISP_LINE_LEFT|5);
		PubBeepErr();
		PubWaitKey(3);
	}
	glSysParam.stEdcInfo.ulNoSignLmtVisa = atoi((char *)szBuff)*100;
	SaveSysParam();

	sprintf((char *)szBuff, "%lu", glSysParam.stEdcInfo.ulNoSignLmtMaster/100);
	while( 1 )
	{
		ScrClrLine(2, 7);
		PubDispString("No sign amt MC", DISP_LINE_LEFT|2);
		PubDispString("[0..1000]",       DISP_LINE_LEFT|4);
		if( PubGetString(NUM_IN|ECHO_IN, 1, 4, szBuff, USER_OPER_TIMEOUT)!=0 )
		{
			return ERR_USERCANCEL;
		}
		if( atoi((char *)szBuff)<=1000 )
		{
			break;
		}
		
		ScrClrLine(2, 7);
		PubDispString("块礚", DISP_LINE_LEFT|3);
		PubDispString("INVALID INPUT", DISP_LINE_LEFT|5);
		PubBeepErr();
		PubWaitKey(3);
	}
	glSysParam.stEdcInfo.ulNoSignLmtMaster = atoi((char *)szBuff)*100;
	SaveSysParam();
		
	sprintf((char *)szBuff, "%lu", glSysParam.stEdcInfo.ulNoSignLmtAmex/100);
	while( 1 )
	{
		ScrClrLine(2, 7);
		PubDispString("No sign amt AMEX", DISP_LINE_LEFT|2);
		PubDispString("[0..1000]",       DISP_LINE_LEFT|4);
		if( PubGetString(NUM_IN|ECHO_IN, 1, 4, szBuff, USER_OPER_TIMEOUT)!=0 )
		{
			return ERR_USERCANCEL;
		}
		if( atoi((char *)szBuff)<=1000 )
		{
			break;
		}
		ScrClrLine(2, 7);
		PubDispString("块礚", DISP_LINE_LEFT|3);
		PubDispString("INVALID INPUT", DISP_LINE_LEFT|5);
		PubBeepErr();
		PubWaitKey(3);
	}
	glSysParam.stEdcInfo.ulNoSignLmtAmex = atoi((char *)szBuff)*100;
	SaveSysParam();

	return 0;
}
#else
int SetNoSignLmt(void)
{

	int		iMenuNo;
	uchar	pszAmt[6], ucRet;
	uchar	szCurrName[4+1], szCurrAmt[6+1],szTem[16+1],szCurrAmt2[16+1],szCurrAmt3[16+1];
	static	MenuItem stTranMenu[5] =
	{
		{TRUE, _T_NOOP("VISA"),		NULL},
		{TRUE, _T_NOOP("MASTER"),		NULL},
		{TRUE, _T_NOOP("AMEX"),		NULL},
		{TRUE, "", NULL},
	};
	
	while( 1 )
	{
		iMenuNo = PubGetMenu((uchar *)_T("CVM LIMIT"), stTranMenu, MENU_AUTOSNO, USER_OPER_TIMEOUT);
		switch( iMenuNo )
		{
		case 0:
			PubShowTitle(TRUE, (uchar *)_T("VISA"));
			ScrClrLine(2,7);
			PubLong2Bcd(glSysParam.stEdcInfo.ulNoSignLmtVisa,4,szCurrAmt);
			PubBcd2Asc0(szCurrAmt,4,szCurrAmt3);
			PubConvAmount(NULL,szCurrAmt3,
				glProcInfo.stTranLog.stTranCurrency.ucDecimal,0,
				szCurrAmt2,0);
			sprintf(szTem, _T("CURRENT: %s"),szCurrAmt2);
			PubDispString(_T(szTem), 2|DISP_LINE_LEFT);
			PubDispString(_T("ENTER AMOUNT"), 4|DISP_LINE_LEFT);
			
			sprintf((char *)szCurrName, "%.3s", glSysParam.stEdcInfo.stLocalCurrency.szName);
			ucRet = PubGetAmount(szCurrName, glProcInfo.stTranLog.stTranCurrency.ucDecimal, 1, 6, pszAmt, USER_OPER_TIMEOUT, 0,0);
			if (ucRet != 0)
			{
				return ERR_NO_DISP;
			}
			if (ConfirmAmount("CVM LIMIT",pszAmt))
			{
				PubAsc2Long(pszAmt,6, &glSysParam.stEdcInfo.ulNoSignLmtVisa);
			}
			else
			{
				break;
			}
			SaveSysParam();
			ScrClrLine(2,7);
			PubDispString(_T("SUCCESS"), 4|DISP_LINE_CENTER);
			PubWaitKey(2);
			break;
		case 1:
			PubShowTitle(TRUE, (uchar *)_T("MASTER"));
			ScrClrLine(2,7);
			PubLong2Bcd(glSysParam.stEdcInfo.ulNoSignLmtMaster,4,szCurrAmt);
			PubBcd2Asc0(szCurrAmt,4,szCurrAmt3);
			PubConvAmount(NULL,szCurrAmt3,
				glProcInfo.stTranLog.stTranCurrency.ucDecimal,0,
				szCurrAmt2,0);
			sprintf(szTem, _T("CURRENT: %s"),szCurrAmt2);
			PubDispString(_T(szTem), 2|DISP_LINE_LEFT);
			PubDispString(_T("ENTER AMOUNT"), 4|DISP_LINE_LEFT);
			
			sprintf((char *)szCurrName, "%.3s", glSysParam.stEdcInfo.stLocalCurrency.szName);
			ucRet = PubGetAmount(szCurrName, glProcInfo.stTranLog.stTranCurrency.ucDecimal, 1, 6, pszAmt, USER_OPER_TIMEOUT, 0,0);
			if (ucRet != 0)
			{
				return ERR_NO_DISP;;
			}
			if (ConfirmAmount("CVM LIMIT",pszAmt))
			{
				PubAsc2Long(pszAmt,6, &glSysParam.stEdcInfo.ulNoSignLmtMaster);
			}
			else
			{
				break;
			}
			SaveSysParam();
			ScrClrLine(2,7);
			PubDispString(_T("SUCCESS"), 4|DISP_LINE_CENTER);
			PubWaitKey(2);
			break;
		case 2:
			PubShowTitle(TRUE, (uchar *)_T("AMEX"));
			ScrClrLine(2,7);
			PubLong2Bcd(glSysParam.stEdcInfo.ulNoSignLmtAmex,4,szCurrAmt);
			PubBcd2Asc0(szCurrAmt,4,szCurrAmt3);
			PubConvAmount(NULL,szCurrAmt3,
				glProcInfo.stTranLog.stTranCurrency.ucDecimal,0,
				szCurrAmt2,0);
			sprintf(szTem, _T("CURRENT: %s"),szCurrAmt2);
			PubDispString(_T(szTem), 2|DISP_LINE_LEFT);
			PubDispString(_T("ENTER AMOUNT"), 4|DISP_LINE_LEFT);
			
			sprintf((char *)szCurrName, "%.3s", glSysParam.stEdcInfo.stLocalCurrency.szName);
			ucRet = PubGetAmount(szCurrName, glProcInfo.stTranLog.stTranCurrency.ucDecimal, 1, 6, pszAmt, USER_OPER_TIMEOUT, 0,0);
			if (ucRet != 0)
			{
				return ERR_NO_DISP;;
			}
			if (ConfirmAmount("CVM LIMIT",pszAmt))
			{
				PubAsc2Long(pszAmt,6, &glSysParam.stEdcInfo.ulNoSignLmtAmex);
			}
			else
			{
				break;
			}
			SaveSysParam();
			ScrClrLine(2,7);
			PubDispString(_T("SUCCESS"), 4|DISP_LINE_CENTER);
			PubWaitKey(2);
			break;
		default:
			return 0;
		}
	}
	return 0;
}
#endif

#ifdef SUPPORT_TABBATCH
int SetAuthTabBatch(void)
{
    uchar   ucRet, szBuff[32];

    //-----------------------------
    while (1)
    {
	    ScrClrLine(2, 7);
        ScrPrint(12, 2, ASCII, "TAB BATCH");
        ScrPrint(12, 3, ASCII, "1.DISABLE");
        ScrPrint(12, 4, ASCII, "2.SETTLE DELETE");
        ScrPrint(12, 5, ASCII, "3.SETTLE ASK DELETE");
        ScrPrint(12, 6, ASCII, "4.EXPIRY DELETE");
        if (glSysParam.stEdcInfo.ucEnableAuthTabBatch==0)   ScrPrint(0, 3, ASCII, ">");
        if (glSysParam.stEdcInfo.ucEnableAuthTabBatch==1)   ScrPrint(0, 4, ASCII, ">");
        if (glSysParam.stEdcInfo.ucEnableAuthTabBatch==2)   ScrPrint(0, 5, ASCII, ">");
        if (glSysParam.stEdcInfo.ucEnableAuthTabBatch==3)   ScrPrint(0, 6, ASCII, ">");
        ucRet = PubWaitKey(USER_OPER_TIMEOUT);
        if ((ucRet==KEYCANCEL) || (ucRet==NOKEY))
        {
            return ERR_USERCANCEL;
        }
        if (ucRet==KEYENTER)
        {
            break;
        }
        if (ucRet>KEY0 && ucRet<KEY5)
        {
            glSysParam.stEdcInfo.ucEnableAuthTabBatch = (uchar)(ucRet-KEY1);
        }
    }

    if (!glSysParam.stEdcInfo.ucEnableAuthTabBatch)
    {
        return 0;
    }
    if (MS_InitTabBatchLogFile())
    {
        glSysParam.stEdcInfo.ucEnableAuthTabBatch = 0;
	    ScrClrLine(2, 7);
	    PubDispString("INIT TAB FAIL", DISP_LINE_LEFT|4);
        PubBeepErr();
        PubWaitKey(5);
        return 0;
    }

    //-----------------------------
    if (glSysParam.stEdcInfo.ucEnableAuthTabBatch==3)
    {
        sprintf((char *)szBuff, "%d", (int)glSysParam.stEdcInfo.ucTabBatchExpDates);
	    ScrClrLine(2, 7);
	    PubDispString("AUTH EXPIRY DATES", DISP_LINE_LEFT|2);
        ucRet = PubGetString(NUM_IN|ECHO_IN, 0, 2, szBuff, USER_OPER_TIMEOUT);
        if (ucRet)
        {
            return ERR_USERCANCEL;
        }
        glSysParam.stEdcInfo.ucTabBatchExpDates = (int)atoi(szBuff);
    }

    //-----------------------------
    sprintf((char *)szBuff, "%d", (int)glSysParam.stEdcInfo.ucPreAuthCompPercent);
	ScrClrLine(2, 7);
	PubDispString("AUTH COMPLETE+%", DISP_LINE_LEFT|2);
    ucRet = PubGetString(NUM_IN|ECHO_IN, 0, 2, szBuff, USER_OPER_TIMEOUT);
    if (ucRet)
    {
        return ERR_USERCANCEL;
    }
    glSysParam.stEdcInfo.ucPreAuthCompPercent = (int)atoi(szBuff);

    return 0;
}
#endif

// 输入热敏打印单据张数
int SetNumOfReceipt(void)
{
	uchar 	ucNum, szBuff[1+1];

	if( !ChkIfThermalPrinter() )
	{
		return 0;
	}

	sprintf((char *)szBuff, "%d", NumOfReceipt());
	while( 1 )
	{
		ScrClrLine(2, 7);
		PubDispString("Receipt pages", DISP_LINE_LEFT|2);
		PubDispString("[1..4]Thermal",   DISP_LINE_LEFT|4);
		if( PubGetString(NUM_IN|ECHO_IN, 1, 1, szBuff, USER_OPER_TIMEOUT)!=0 )
		{
			return ERR_USERCANCEL;
		}

		ucNum = (uchar)atoi((char *)szBuff);
		if( ucNum>=1 && ucNum<=4 )
		{
			ucNum--;
			break;
		}

		ScrClrLine(4, 7);
		PubDispString(_T("INVALID INPUT"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(3);
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

	return 0;
}

// 输入参数自动更新时间
int SetCallInTime(void)
{
	return 0;
}

// TRUE:判断时间是否合法
uchar IsValidTime(uchar *pszTime)
{
	int		i, iHour, iMinute;

	for(i=0; i<4; i++)
	{
		if( pszTime[i]<'0' || pszTime[i]>'9' )
		{
			return FALSE;
		}
	}

	iHour   = (int)PubAsc2Long(pszTime, 2, NULL);
	iMinute = (int)PubAsc2Long(pszTime+2, 2, NULL);
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

// 修改或者查看开关选项
int ModifyOptList(uchar *psOption, uchar ucMode, uchar ucPermission)
{
	// 通过FUN2进入设置时，若定义了FUN2_READ_ONLY，则用户权限为PM_LOW，否则用户权限为PM_MEDIUM
	// 使用无下载方式初始化时，用户权限为PM_HIGH

	// Protims可控的issuer option列表
	static OPTION_INFO stIssuerOptList[] =
	{
// 		{"CAPTURE CASH?",		ALLOW_EXTEND_PAY,			FALSE,	PM_MEDIUM},
		{"CAPTURE TXN",			ISSUER_CAPTURE_TXN,			FALSE,	PM_MEDIUM},
// 		{"ENABLE BALANCE?",		ISSUER_EN_BALANCE,			FALSE,	PM_MEDIUM},
		{"ENABLE ADJUST",		ISSUER_EN_ADJUST,			FALSE,	PM_MEDIUM},
		{"ENABLE OFFLINE",		ISSUER_EN_OFFLINE,			FALSE,	PM_MEDIUM},
		{"ALLOW (PRE)AUTH",		ISSUER_NO_PREAUTH,			TRUE,	PM_MEDIUM},
		{"ALLOW REFUND",		ISSUER_NO_REFUND,			TRUE,	PM_MEDIUM},
		{"ALLOW VOID",			ISSUER_NO_VOID,				TRUE,	PM_MEDIUM},
		{"ENABLE EXPIRY",		ISSUER_EN_EXPIRY,			FALSE,	PM_MEDIUM},
		{"MASK EXPIRY?",	   	ISSUER_MASK_EXPIRY,         FALSE,  PM_MEDIUM},//2016-5-26 AE Express update: mask expiry date in receipt
		{"CHECK EXPIRY",		ISSUER_CHECK_EXPIRY,		FALSE,	PM_MEDIUM},
//		{"CHKEXP OFFLINE",		ISSUER_CHECK_EXPIRY_OFFLINE,FALSE,	PM_MEDIUM},
		{"CHECK PAN",			ISSUER_CHKPAN_MOD10,		FALSE,	PM_MEDIUM},
// 		{"CHECK PAN11",			ISSUER_CHKPAN_MOD11,		FALSE,	PM_MEDIUM},
//		{"EN DISCRIPTOR",		ISSUER_EN_DISCRIPTOR,		FALSE,	PM_MEDIUM},
		{"ENABLE MANUAL",		ISSUER_EN_MANUAL,			FALSE,	PM_MEDIUM},
		{"ENABLE PRINT",		ISSUER_EN_PRINT,			FALSE,	PM_MEDIUM},
		{"VOICE REFERRAL",		ISSUER_EN_VOICE_REFERRAL,	FALSE,	PM_MEDIUM},
		{"PIN REQUIRED",		ISSUER_EN_PIN,				FALSE,	PM_MEDIUM}, //Gillian 20161020
#ifdef ISSUER_FORBID_EMVPIN_BYPASS
		{"NOT EMV BYPASS",		ISSUER_FORBID_EMVPIN_BYPASS,FALSE,	PM_MEDIUM},
#endif
//		{"ACCOUNT SELECT",		ISSUER_EN_ACCOUNT_SELECTION,FALSE,	PM_MEDIUM},
//		{"ROC INPUT REQ",		ISSUER_ROC_INPUT_REQ,		FALSE,	PM_MEDIUM},
//		{"DISP AUTH CODE",		ISSUER_AUTH_CODE,			FALSE,	PM_MEDIUM},
//		{"ADDTIONAL DATA",		ISSUER_ADDTIONAL_DATA,		FALSE,	PM_MEDIUM},
		{"ENABLE TRACK 1 ",		ISSUER_EN_TRACK1,			FALSE,	PM_MEDIUM},
		{"4DBC WHEN SWIPE",		ISSUER_SECURITY_SWIPE,		FALSE,	PM_MEDIUM},
		{"4DBC WHEN MANUL",		ISSUER_SECURITY_MANUL,		FALSE,	PM_MEDIUM},
		{"4DBC WHEN INSERT",	ISSUER_SECURITY_INSERT,		FALSE,	PM_MEDIUM},
		{NULL, 0, FALSE, PM_DISABLE},
	};

	// Protims可控的acquirer option列表
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
		{"EMV ACQUIRER",		ACQ_EMV_FEATURE,			FALSE,	PM_HIGH},
		{NULL, 0, FALSE, PM_DISABLE},
	};

	// Protims不可控的acquirer option列表
	static OPTION_INFO stAcqExtOptList[] =
	{
		// 因为只能在且必须在POS上修改，因此权限设为PM_LOW
		{NULL, 0, FALSE, PM_DISABLE},
	};

	// Protims可控的edc option列表
	static OPTION_INFO stEdcOptList[] =
	{
		{"AUTH PAN MASKING",	EDC_AUTH_PAN_MASKING,	FALSE,	PM_LOW},
//		{"SELECT ACQ_CARD",		EDC_SELECT_ACQ_FOR_CARD,FALSE,	PM_LOW},
		{"ENABLE ECR",			EDC_ECR_ENABLE,			FALSE,	PM_LOW},
		{"FREE PRINT",			EDC_FREE_PRINT,			FALSE,  PM_LOW},
		
		{"EN. INSTALMENT?",		EDC_ENABLE_INSTALMENT,	FALSE,	PM_LOW/*PM_MEDIUM*/},
		{"ENABLE. DCC?",		EDC_ENABLE_DCC,			FALSE,	PM_LOW},
		{"CAPTURE CASH",		EDC_CASH_PROCESS,		FALSE,	PM_MEDIUM},
		{"REFERRAL DIAL",		EDC_REFERRAL_DIAL,		FALSE,	PM_MEDIUM},
		{"WINGLUNG ENCRY",	    EDC_WINGLUNG_ENCRY,		FALSE,	PM_MEDIUM},	//Gillian 2016-8-1	
		
		{"1.AUTH 0.PREAUTH",	EDC_AUTH_PREAUTH,		FALSE,	PM_MEDIUM},
		{"BEA IP ENCRY",	    EDC_BEA_IP_ENCRY,		FALSE,	PM_MEDIUM},	//Gillian 2016-8-1
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
	static OPTION_INFO stEdcExtOptList[] =
	{
		{"TCP ENCRYPT?",		EDC_EXT_TCP_ENC},//2016-2-29 *** //Gillian 2016-7-14
		// 因为只能在且必须在POS上修改，因此权限设为PM_LOW		
		{"INVERSE RATE",	EDC_EX_INVERSE_RATE,	FALSE, PM_LOW},
		{"NO DECIMAL?",		EDC_EXT_DCC_ADJUST_AMT_NO_DECIMAL,FALSE, PM_LOW},
		{NULL, 0, FALSE, PM_DISABLE},
	};

	OPTION_INFO		*pstCurOpt;
	uchar			ucCnt, szBuff[4], ucOptIndex, ucOptBitNo;

	switch(ucMode)
	{
	case 'I':
	case 'i':
		pstCurOpt = (OPTION_INFO *)stIssuerOptList;
		break;
	case 'E':
		pstCurOpt = (OPTION_INFO *)stEdcOptList;
		break;
	case 'e':
		pstCurOpt = (OPTION_INFO *)stEdcExtOptList;
		break;
	case 'd':
		pstCurOpt = (OPTION_INFO *)stEdcExtOptList; // PP-DCC
		break;
	case 'A':
		pstCurOpt = (OPTION_INFO *)stAcqOptList;
		break;
	case 'a':
		pstCurOpt = (OPTION_INFO *)stAcqExtOptList;
		break;
	default:
		break;
	}

	if( pstCurOpt->pText==NULL )
	{
		return 0;
	}

	ucCnt = 0;
	while( 1 )
	{
		ucOptIndex = (uchar)(pstCurOpt[ucCnt].uiOptVal>>8);
		ucOptBitNo = (uchar)(pstCurOpt[ucCnt].uiOptVal & 0xFF);

		ScrClrLine(2, 7);
		PubDispString(pstCurOpt[ucCnt].pText, DISP_LINE_CENTER|2);
		if (pstCurOpt[ucCnt].ucInverseLogic)
		{
			sprintf((char *)szBuff, "(%c)", (psOption[ucOptIndex] & ucOptBitNo) ? '0' : '1');
		}
		else
		{
			sprintf((char *)szBuff, "(%c)", (psOption[ucOptIndex] & ucOptBitNo) ? '1' : '0');
		}
		PubDispString(szBuff,       DISP_LINE_CENTER|4);
		PubDispString("1.ON 0.OFF", DISP_LINE_CENTER|6);
		switch( PubWaitKey(USER_OPER_TIMEOUT) )
		{
		case KEY1:
			if (ucPermission>=pstCurOpt[ucCnt].ucPermissionLevel)
			{
				if (pstCurOpt[ucCnt].ucInverseLogic)
				{
					psOption[ucOptIndex] &= ~ucOptBitNo;
				}
				else
				{
					psOption[ucOptIndex] |= ucOptBitNo;
				}
			} 
			break;

		case KEY0:
			if (ucPermission>=pstCurOpt[ucCnt].ucPermissionLevel)
			{
				if (pstCurOpt[ucCnt].ucInverseLogic)
				{
					psOption[ucOptIndex] |= ucOptBitNo;
				}
				else
				{
					psOption[ucOptIndex] &= ~ucOptBitNo;
				}
			}
			break;

		case KEYUP:
			if( ucCnt>0 )
			{
				ucCnt--;
			}
			break;

		case KEYDOWN:
		case KEYENTER:
			if( pstCurOpt[ucCnt+1].pText==NULL )
			{
				return 0;
			}
			ucCnt++;
			break;

		case NOKEY:
		case KEYCANCEL:
			return ERR_USERCANCEL;
		}
	}
}

// 修改口令
void ChangePassword(void)
{
	static MenuItem stChgPwdMenu[] =
	{
		{TRUE, _T_NOOP("TERMINAL   PWD"),   ModifyPasswordTerm},
		{TRUE, _T_NOOP("BANK       PWD"),   ModifyPasswordBank},
		{TRUE, _T_NOOP("MERCHANT   PWD"),   ModifyPasswordMerchant},
		{TRUE, _T_NOOP("VOID       PWD"),   ModifyPasswordVoid},
		{TRUE, _T_NOOP("REFUND     PWD"),   ModifyPasswordRefund},
		{TRUE, _T_NOOP("ADJUST     PWD"),   ModifyPasswordAdjust},
		{TRUE, _T_NOOP("SETTLE     PWD"),   ModifyPasswordSettle},
		{TRUE, "", NULL},
	};

	PubGetMenu((uchar *)_T("CHANGE PWD"), (MenuItem *)stChgPwdMenu, MENU_AUTOSNO|MENU_ASCII, USER_OPER_TIMEOUT);
}

// 手工设置系统时间
void SetSysTime(void)
{
	uchar	szBuff[14+1], sInputTime[6];

	PubShowTitle(TRUE, (uchar *)_T("SET TIME"));

	while( 1 )
	{
		ScrClrLine(2, 7);
		PubDispString("YYYYMMDDHHMMSS", DISP_LINE_RIGHT|2);
		PubGetDateTime(szBuff);
		PubDispString(szBuff, DISP_LINE_RIGHT|4);

		if( PubGetString(NUM_IN|ECHO_IN, 14, 14, szBuff, USER_OPER_TIMEOUT)!=0 )
		{
			return;
		}
		PubAsc2Bcd(&szBuff[2], 12, sInputTime);
		if( SetTime(sInputTime)==0 )
		{
			return;
		}
		ScrClrLine(4, 7);
		PubDispString(_T("INVALID TIME"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(3);
	}
}

// provide manual select and prompt message when pszLngName==NULL
// mode:
// 0--auto load the first available non-english language (if language file available)
// 1--auto load the last time used language
// 2--provide a menu for selection
void SetSysLang(uchar ucSelectMode)
{
	int	iCnt, iTotal, iRet;
	LANG_CONFIG		stLastCfg;

	MenuItem stLangMenu[32];

	if (ucSelectMode==2)
	{
		stLastCfg = glSysParam.stEdcInfo.stLangCfg;
	}

REDO_SELECT_LANG:
	if (ucSelectMode==0 || ucSelectMode==2)
	{
		// 搜寻已下载的语言文件，准备菜单
		for (iCnt=0, iTotal=0;
			iCnt<sizeof(stLangMenu)/sizeof(stLangMenu[0])-1;
			iCnt++)
		{
			if (glLangList[iCnt].szDispName[0]==0)
			{
				break;
			}
			if ((iCnt==0) || fexist((char *)glLangList[iCnt].szFileName)>=0)
			{
				stLangMenu[iTotal].bVisible = TRUE;
				strcpy(stLangMenu[iTotal].szMenuName, _T((char *)glLangList[iCnt].szDispName));
				stLangMenu[iTotal].pfMenuFunc = NULL;
				iTotal++;
			}
		}
		stLangMenu[iTotal].bVisible = TRUE;
		strcpy(stLangMenu[iTotal].szMenuName, "");
		stLangMenu[iTotal].pfMenuFunc = NULL;

		if (ucSelectMode==0)// 首次加载
		{
			// 如果有一个或多个非英文语言，自动选择第一个；否则选择英语
			iRet = ((iTotal>1) ? 1 : 0);
		}
		else
		{
			// 菜单手动选择
			iRet = PubGetMenu((uchar *)_T("SELECT LANG"), stLangMenu, MENU_CFONT|MENU_AUTOSNO, 60);
			if (iRet<0)
			{
				return;
			}
		}

		for (iCnt=0; glLangList[iCnt].szDispName[0]!=0; iCnt++)
		{
			if (strcmp((char *)glLangList[iCnt].szDispName,(char *)stLangMenu[iRet].szMenuName)==0)
			{
				glSysParam.stEdcInfo.stLangCfg = glLangList[iCnt];
				break;
			}
		}
	}

	// 设为英语
	if (strcmp(glSysParam.stEdcInfo.stLangCfg.szFileName, "")==0)
	{
		iRet = SetLng(NULL);
		glSysParam.stEdcInfo.ucLanguage = 0;
		return;
	}

	iRet = SetLng(glSysParam.stEdcInfo.stLangCfg.szFileName);
	glSysParam.stEdcInfo.ucLanguage = 1;
	if (iRet!=0)
	{
		glSysParam.stEdcInfo.stLangCfg = glLangList[0];
		return;
	}
	
	if ((ucSelectMode==0) || (ucSelectMode==2))
	{
		// 在初次加载或者手动选择模式下，检查字库是否含有此内码
		if (CheckSysFont()!=0)
		{
			ScrCls();
			PubDispString((uchar *)"SELECT LANG",  DISP_LINE_LEFT|DISP_LINE_REVER);
			PubDispString((uchar *)"DISPLAY/PRINT",    2|DISP_LINE_LEFT);
			PubDispString((uchar *)"MAY HAVE PROBLEM", 4|DISP_LINE_LEFT);
			PubDispString((uchar *)"SET ANYWAY ?",     6|DISP_LINE_LEFT);
			if (PubYesNo(USER_OPER_TIMEOUT)==0)
			{
				return;
			}

			iRet = SetLng(NULL);
			glSysParam.stEdcInfo.stLangCfg = glLangList[0];
			ucSelectMode = 2;
			goto REDO_SELECT_LANG;
		}
	}
}

// Set system language
void SetEdcLang(void)
{
	LANG_CONFIG	stLangBak;

	memcpy(&stLangBak, &glSysParam.stEdcInfo.stLangCfg, sizeof(LANG_CONFIG));
	SetSysLang(2);
	if (memcmp(&stLangBak, &glSysParam.stEdcInfo.stLangCfg, sizeof(LANG_CONFIG)) != 0)
	{
		if(glSysParam.stEdcInfo.ucClssFlag == 1)
		{
			ClssOpen();
			SetReaderLan();//2016-3-9 add for set R50 Language
			ClssClose();
		}
		SaveEdcParam();
	}
}
int SetReaderLan(void)//2016-3-9 add for set R50 Language
{
	int	iRet;
	uchar   ucRecvData[256] = {0};//2016-3-9
    ushort  usLenRecv;//2016-3-9

	if(glSysParam.stEdcInfo.ucLanguage == 0)
	{
		//first byte 0xE4 is command  the follow one is language parameter 'E' is english   'C ' is chinese
		iRet = WaveProcOtherProtocol("\xE4\x45",2,ucRecvData,&usLenRecv);
	}
	else
	{
		//first byte 0xE4 is command  the follow one is language parameter 'E' is english   'C ' is chinese
		iRet =WaveProcOtherProtocol("\xE4\x43",2,ucRecvData,&usLenRecv);
	}
	return iRet;
}
#ifndef APP_MANAGER_VER
void SetEdcLangExt(char *pszDispName)
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

void SetPowerSave(void)
{
#ifdef _S_SERIES_
	uchar	ucRet, ucTemp, szBuff[32];

	ScrCls();
	PubDispString(_T("POWERSAVE OPTION"), DISP_LINE_REVER);
	if (!ChkTerm(_TERMINAL_S90_))
	{
		PubDispString(_T("UNSUPPORTED."), 2|DISP_LINE_LEFT);
		PubWaitKey(USER_OPER_TIMEOUT);
		return;
	}
	else
	{
		while (1)
		{
			sprintf((char *)szBuff, _T("IDLE: POWER SAVE"));
			if (glSysParam.stEdcInfo.ucIdleShutdown)
			{
				sprintf((char *)szBuff, _T("IDLE: SHUTDOWN  "));
			}
			ScrClrLine(2, 7);
			ucRet = PubSelectOneTwo(szBuff, _T("POWER SAVE"), _T("SHUTDOWN"));
			if (ucRet==KEY1)
			{
				glSysParam.stEdcInfo.ucIdleShutdown = 0;
				SaveSysParam();
			}
			if (ucRet==KEY2)
			{
				glSysParam.stEdcInfo.ucIdleShutdown = 1;
				if (glSysParam.stEdcInfo.ucIdleMinute<5)
				{
					glSysParam.stEdcInfo.ucIdleMinute = 5;
				}
				SaveSysParam();
			}
			if (ucRet==KEYCANCEL)
			{
				return;
			}
			break;
		}

		ucTemp = glSysParam.stEdcInfo.ucIdleMinute;
		while (1)
		{
			ScrClrLine(2, 7);
			if (glSysParam.stEdcInfo.ucIdleShutdown)
			{
				PubDispString(_T("SHUTDOWN TIMEOUT"), 2|DISP_LINE_LEFT|DISP_LINE_REVER);
				PubDispString("(5-60 minutes)", 4|DISP_LINE_LEFT);
			}
			else
			{
				PubDispString(_T("PWR SAVE TIMEOUT"), 2|DISP_LINE_LEFT|DISP_LINE_REVER);
				PubDispString("(0-60 minutes)", 4|DISP_LINE_LEFT); //modified  by richard 20161222, 0 - not power save
			}
			sprintf((char *)szBuff, "%d", (int)ucTemp);
			ucRet = PubGetString(NUM_IN|ECHO_IN, 1, 2, szBuff, USER_OPER_TIMEOUT);
			if (ucRet!=0)
			{
				return;
			}
			ucTemp = (uchar)atol((char *)szBuff);
			if (ucTemp>60)
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
#endif
}

void TestMagicCard1(void)
{
	TestMagicCard(1);
}

void TestMagicCard2(void)
{
	TestMagicCard(2);
}

void TestMagicCard3(void)
{
	TestMagicCard(3);
}

void TestMagicCard(int iTrackNum)
{
	uchar	ucRet, ucKey;
	uchar	szMagTrack1[79+1], szMagTrack2[40+1], szMagTrack3[104+1];
	uchar	szTitle[16+1];

	MagClose();
	MagOpen();
	MagReset();
	while( 1 )
	{
		sprintf((char *)szTitle, "TRACK %d TEST", iTrackNum);
		PubShowTitle(TRUE, szTitle);
		PubDispString(_T("PLS SWIPE CARD"), 4|DISP_LINE_LEFT);
		while( 2 )
		{
			if( PubKeyPressed() )
			{
				if( getkey()==KEYCANCEL )
				{
					MagClose();
					return;
				}
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
		ScrClrLine(2, 7);
		if( iTrackNum==1 )
		{
			ScrPrint(0, 5, ASCII, "%.21s", szMagTrack1[0]==0 ? (uchar *)"NULL" : szMagTrack1);
			ScrPrint(0, 7, ASCII, "Length=[%d]", strlen((char *)szMagTrack1));
		}
		else if (iTrackNum == 2)
		{
			ScrPrint(0, 5, ASCII, "%.21s", szMagTrack2[0]==0 ? (uchar *)"NULL" : szMagTrack2);
			ScrPrint(0, 7, ASCII, "Length=[%d]", strlen((char *)szMagTrack2));
		}
		else
		{
			ScrPrint(0, 5, ASCII, "%.21s", szMagTrack3[0]==0 ? (uchar *)"NULL" : szMagTrack3);
			ScrPrint(0, 7, ASCII, "Length=[%d]", strlen((char *)szMagTrack3));
		}
		ScrPrint(128-6*6, 7, ASCII, "RET:%02X", ucRet);

		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( ucKey==KEYCANCEL || ucKey==NOKEY )
		{
			return;
		}
	}
}

void ToolsViewPreTransMsg(void)
{
	static MenuItem stViewMsgMenu[] =
	{
		{TRUE, "OUTPUT SEND/RECV",	ShowExchangePack},
		{TRUE, "PRINT SEND/RECV",	PrnExchangePack},
		{TRUE, "",	NULL},
	};

	PubShowTitle(TRUE, (uchar *)_T("VIEW MSG"));
	if( PasswordBank()!=0 )
	{
		return;
	}

	PubGetMenu((uchar *)_T("VIEW MSG"), 
				stViewMsgMenu, MENU_AUTOSNO|MENU_ASCII, USER_OPER_TIMEOUT);
}

// 发送通讯报文到COM1
// send comm package
void ShowExchangePack(void)
{
#if defined(WIN32) && defined(_S_SERIES_)
#define DEBUG_OUT_PORT	PINPAD
#else
#define DEBUG_OUT_PORT	0
#endif
	PubShowTitle(TRUE, (uchar *)_T("VIEW MSG"));
	if (!glSendData.uiLength && !glRecvData.uiLength)
	{
		DispErrMsg((uchar *)_T("NO DATA"), NULL, 5, 0);
		return;
	}
	
	PubDispString(_T("SENDING..."), 4|DISP_LINE_LEFT);

	//2014-9-1 output the whole message
	if( glCommCfg.ucCommType==CT_TCPIP ||
        glCommCfg.ucCommType==CT_GPRS  ||
        glCommCfg.ucCommType==CT_CDMA  )
    {


        PubDebugOutput((uchar *)_T("VIEW MSG"), glSendDataDebug.sContent+2, glSendDataDebug.uiLength,
                       DEVICE_COM2, ISO_MODE);
    }
    else
    {
        PubDebugOutput((uchar *)_T("VIEW MSG"), glSendDataDebug.sContent, glSendDataDebug.uiLength,
                       DEVICE_COM2, ISO_MODE);
    }

    PubDebugOutput((uchar *)_T("VIEW MSG"), glRecvDataDebug.sContent, glRecvDataDebug.uiLength,
                   DEVICE_COM2, ISO_MODE);

	//DebugNacTxd(DEBUG_OUT_PORT, glSendData.sContent, glSendData.uiLength);
	//DelayMs(2000);
	//DebugNacTxd(DEBUG_OUT_PORT, glRecvData.sContent, glRecvData.uiLength);
	PortClose(3);//2014-9-2 com confilct

	PubDispString(_T("SEND OK"), 4|DISP_LINE_LEFT);
	PubWaitKey(2);
}

// 打印通讯报文
// Print comm package
void PrnExchangePack(void)
{
	PubShowTitle(TRUE, (uchar *)_T("VIEW MSG"));
	if (!glSendData.uiLength && !glRecvData.uiLength)
	{
		DispErrMsg((uchar *)_T("NO DATA"), NULL, 5, 0);
		return;
	}

	PubDebugOutput((uchar *)_T("VIEW MSG"), glSendData.sContent, glSendData.uiLength,
					DEVICE_PRN, ISO_MODE);
	PubDebugOutput((uchar *)_T("VIEW MSG"), glRecvData.sContent, glRecvData.uiLength,
					DEVICE_PRN, ISO_MODE);
}

void DebugNacTxd(uchar ucPortNo, uchar *psTxdData, ushort uiDataLen)
{
	uchar	*psTemp, sWorkBuf[LEN_MAX_COMM_DATA+10];
	uchar  ucInit = 0;
	
	if( uiDataLen>LEN_MAX_COMM_DATA )
	{
		PubDispString(_T("INVALID PACK"), 4|DISP_LINE_LEFT);
		PubWaitKey(2);
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
#ifdef APP_MANAGER_VER
static void SetManagerService(void)
{
	uchar	ucKey, szBuff[32];
	
    //-----------------------------------------------------
	sprintf((char *)szBuff, "CUR: %.10s", (glSysParam.ucKbSound ? _T("ENABLE") : _T("MUTE")));
	PubShowTitle(TRUE, (uchar *)_T("KEYBOARD SOUND"));
	ucKey = PubSelectOneTwo(szBuff, (uchar *)_T("ENABLE"), (uchar *)_T("MUTE"));
	if ((ucKey==KEY1) && !glSysParam.ucKbSound)
	{
		glSysParam.ucKbSound = 1;
		SaveSysParam();
	}
	if ((ucKey==KEY2) && glSysParam.ucKbSound)
	{
		glSysParam.ucKbSound = 0;
		SaveSysParam();
	}
    if (ucKey==KEYCANCEL)
    {
        return;
    }
	kbmute(glSysParam.ucKbSound);
	
    //-----------------------------------------------------
	sprintf((char *)szBuff, "CUR: %.10s",
        (glSysParam.stECR.ucMode==ECRMODE_OFF) ? "DISABLE" : (glSysParam.stECR.ucMode==ECRMODE_ENABLED ? "ENABLE" : "ECR ONLY"));
	PubShowTitle(TRUE, (uchar *)"ECR SETUP");
    PubDispString(szBuff, 2|DISP_LINE_LEFT);
    ScrPrint(0, 5, ASCII, "1. DISABLE");
    ScrPrint(0, 6, ASCII, "2. ENABLE");
    ScrPrint(0, 7, ASCII, "3. ECR ONLY");
    while (1)
    {
        ucKey = PubWaitKey(USER_OPER_TIMEOUT);
        if (ucKey==KEYENTER)
        {
            break;
        }
		if ((ucKey==KEY1) || (ucKey==KEY2) || (ucKey==KEY3))
		{
			glSysParam.stECR.ucMode = ucKey - KEY1;
            SaveSysParam();
            break;
		}
        if ((ucKey==KEYCANCEL) || (ucKey==NOKEY))
        {
            return;
        }
    }
	
    //-----------------------------------------------------
    if (glSysParam.stECR.ucMode)
    {
		PubShowTitle(TRUE, (uchar *)"ECR SERIAL PORT");
		sprintf((char *)szBuff, "CUR: %.10s", (glSysParam.stECR.ucPort==0 ? "PORT 1" : "PPAD"));
		ucKey = PubSelectOneTwo(szBuff, (uchar *)"COM 1", (uchar *)"PPAD");
        if (ucKey==KEYCANCEL)
        {
            return;
        }
		if (ucKey==KEY1)
		{
			glSysParam.stECR.ucPort = 0;
		}
		if (ucKey==KEY2)
		{
			glSysParam.stECR.ucPort = PINPAD;
		}
        SaveSysParam();
		
		PubShowTitle(TRUE, (uchar *)"ECR BAUD RATE");
		sprintf((char *)szBuff, "CUR: %.10s", (glSysParam.stECR.ucSpeed==ECRSPEED_9600 ? "9600" : "115200"));
		ucKey = PubSelectOneTwo(szBuff, (uchar *)"9600", (uchar *)"115200");
        if (ucKey==KEYCANCEL)
        {
            return;
        }
		if (ucKey==KEY1)
		{
			glSysParam.stECR.ucSpeed = ECRSPEED_9600;
		}
		if (ucKey==KEY2)
		{
			glSysParam.stECR.ucSpeed = ECRSPEED_115200;
		}
        SaveSysParam();
    }
	
    //-----------------------------------------------------
    // TODO : more settings
}
#endif
// end of file

#ifdef PP_SSL
//*************************************************************************************
//Function:
//Set IP mode manually
//*************************************************************************************
void SetIPMode(void)
{
	uchar szURL[100 + 1];     // shawn 20101206 添加域名解析
    uchar buf1[100 + 1];     // shawn 20101206 添加域名解析
    uchar key, ucKey, szBuff[32];     // shawn 20101206 添加域名解析
	char Result[32]= {0};
	int MyRet=-1, iRet;

	//============================Gillian 20160902===========================

    ScrCls();

	ScrPrint(0, 0, CFONT + REVER, "    IP or SSL   ");
	sprintf((char *)szBuff, "(%s)", (glSysParam.stTxnCommCfg.ucEnableSSL_URL == TRUE ? "2" : "1"));
	PubDispString(szBuff, 2|DISP_LINE_CENTER);
    PubDispString("1. IP", 4|DISP_LINE_LEFT);
 
    PubDispString("2. SSL", 6|DISP_LINE_LEFT);

	while (1)
	{
		ucKey = PubWaitKey(30);
		if ((ucKey == KEYCANCEL) || (ucKey == NOKEY))
		{
			break;
		}
		else if (ucKey == KEY1)
		{
			glSysParam.stTxnCommCfg.ucEnableSSL_URL = FALSE;
			glSysParam.stTxnCommCfg.ucEnableSSL = FALSE;
			glSysParam.stTxnCommCfg.ucCommType = CT_TCPIP;
			break;
		}
		else if (ucKey == KEY2)
		{
			glSysParam.stTxnCommCfg.ucEnableSSL_URL = TRUE;
		//	glSysParam.stTxnCommCfg.ucCommType = CT_SSL;								
			SetSSL_URL();
		    break;
		}
	}
	
	SaveSysParam();//2014-9-24
	ScrClrLine(2, 7);
	//apiDispString("砞竚ЧΘ", 16, DISP_LINE_CENTER + 3);
	//PubDispString("設置完成", 3|DISP_LINE_CENTER);//2015-7-7 to avoid messy code, remark first
	//apiDispString("COMPLETED", 16, DISP_LINE_CENTER + 5);
	PubDispString("COMPLETED", 5|DISP_LINE_CENTER);
	PubWaitKey(3);     
}

void SetSSL_URL(void)
{
	uchar szURL[100 + 1];     // shawn 20101206 添加域名解析
    uchar buf1[100 + 1];     // shawn 20101206 添加域名解析
    uchar key, ucKey;     // shawn 20101206 添加域名解析
	char Result[32]= {0}, Port_DCC[5] = "9457", Port_DCD[6] = "48081";
	int MyRet=-1, iRet;
	
	/*if(!ChkIfAmex() || !ChkIfDiners())
	{*/
		memset(Result, 0, sizeof(Result));
		memset(glSysParam.stTxnCommCfg.szPP_URL, 0, sizeof(glSysParam.stTxnCommCfg.szPP_URL));
		memcpy(glSysParam.stTxnCommCfg.szPP_URL, "terminal.planetpayment.net", strlen("terminal.planetpayment.net"));
		InitSSL();
		ScrCls();
		PubDispString(_T("DISPLAY URL"), 0 | DISP_LINE_CENTER | DISP_LINE_REVER);

		PubDispString(glSysParam.stTxnCommCfg.szPP_URL, 1 | DISP_LINE_RIGHT);
		MyRet = DnsResolve(glSysParam.stTxnCommCfg.szPP_URL, Result, 32);
	
		strcpy(gCurComPara.LocalParam.szRemoteIP_1, Result);
		strcpy(gCurComPara.LocalParam.szRemoteIP_2, Result);
		 
		ScrCls();
		PubDispString(_T("SET LAN COMM"), 0 | DISP_LINE_CENTER | DISP_LINE_REVER);
		PubGetString(ALPHA_IN | ECHO_IN, 1, URL_MAX_LEN, glSysParam.stTxnCommCfg.szPP_URL, 60);

		memset(glSysParam.stTxnCommCfg.szPP_Port_DCC, 0, sizeof(glSysParam.stTxnCommCfg.szPP_Port_DCC));  //Gillian 20160910
		strcpy(glSysParam.stTxnCommCfg.szPP_Port_DCC, Port_DCC);
		ScrCls();
		PubDispString(_T("SET DCC PORT"), 0 | DISP_LINE_CENTER | DISP_LINE_REVER);
		PubGetString(ALPHA_IN | ECHO_IN, 1, 6, glSysParam.stTxnCommCfg.szPP_Port_DCC, 60);

		memset(glSysParam.stTxnCommCfg.szPP_Port_DCD, 0, sizeof(glSysParam.stTxnCommCfg.szPP_Port_DCD));  //Gillian 20160910
		strcpy(glSysParam.stTxnCommCfg.szPP_Port_DCD, Port_DCD);
		ScrCls();
		PubDispString(_T("SET DCD PORT"), 0 | DISP_LINE_CENTER | DISP_LINE_REVER);
		PubGetString(ALPHA_IN | ECHO_IN, 1, 6, glSysParam.stTxnCommCfg.szPP_Port_DCD, 60);

		memcpy(gCurComPara.LocalParam.szRemotePort_1, glSysParam.stTxnCommCfg.szPP_Port_DCC, sizeof(glSysParam.stTxnCommCfg.szPP_Port_DCC));
		memcpy(gCurComPara.LocalParam.szRemotePort_2, glSysParam.stTxnCommCfg.szPP_Port_DCD, sizeof(glSysParam.stTxnCommCfg.szPP_Port_DCD));

        //add by richard 20161209,for fubon S90 ssl url setup
        ScrCls();
        //memset(glSysParam.stTxnCommCfg.szSSL_URL, 0, sizeof(glSysParam.stTxnCommCfg.szSSL_URL));
        memset(glSysParam.stTxnCommCfg.szSSL_BackupIP, 0, sizeof(glSysParam.stTxnCommCfg.szSSL_BackupIP));
        if(0==strlen(glSysParam.stTxnCommCfg.szSSL_URL))
        {
            strcpy(glSysParam.stTxnCommCfg.szSSL_URL, S90_3G_SSL_URL);
        }
		PubDispString(_T("SET FUBON URL"), 0 | DISP_LINE_CENTER | DISP_LINE_REVER);
		PubGetString(ALPHA_IN | ECHO_IN, 1, URL_MAX_LEN, glSysParam.stTxnCommCfg.szSSL_URL, 60);
        if(DnsResolve(glSysParam.stTxnCommCfg.szSSL_URL, glSysParam.stTxnCommCfg.szSSL_BackupIP, 32))
        {
            memcpy(glSysParam.stTxnCommCfg.szSSL_BackupIP, glSysParam.stTxnCommCfg.szSSL_URL, strlen(glSysParam.stTxnCommCfg.szSSL_URL));
        }

		ScrCls();
        //memset(glSysParam.stTxnCommCfg.szSSL_Port, 0, sizeof(glSysParam.stTxnCommCfg.szSSL_Port));
        if(0==strlen(glSysParam.stTxnCommCfg.szSSL_Port))
        {
            strcpy(glSysParam.stTxnCommCfg.szSSL_Port, S90_3G_SSL_PORT);
        }
		PubDispString(_T("SET FUBON PORT"), 0 | DISP_LINE_CENTER | DISP_LINE_REVER);
		PubGetString(ALPHA_IN | ECHO_IN, 1, 6, glSysParam.stTxnCommCfg.szSSL_Port, 60);
	/*}
	else if (ChkCurAcqName("AMEX_INST", FALSE))
	{
		memset(Result, 0, sizeof(Result));
		memset(glSysParam.stTxnCommCfg.szPP_URL, 0, sizeof(glSysParam.stTxnCommCfg.szPP_URL));
		//Gillian 20161011
		memcpy(glSysParam.stTxnCommCfg.szPP_URL, "qwww318.americanexpress.com", strlen("qwww318.americanexpress.com"));
		MyRet = DnsResolve(glSysParam.stTxnCommCfg.szPP_URL, Result, 32);
		strcpy(glCurAcq.stTxnTCPIPInfo[0].szIP, Result);
		strcpy(glCurAcq.stTxnTCPIPInfo[0].szPort, Port_INST);
		strcpy(glCurAcq.stTxnTCPIPInfo[1].szIP, Result);
		strcpy(glCurAcq.stTxnTCPIPInfo[1].szPort, Port_INST);
	}	*/
}

#endif

/*----------------2014-5-20 IP encryption----------------*/
#ifdef IP_ENCRYPT
/************************************************************************/
/* file test function, often used to debug                              */
/************************************************************************/

void FileTest(char* psFileName)
{
	int iOffset;
	int iRet;
	int fid;
	int iFileSize;
	char str[300];
	fid = open(psFileName,O_RDWR);
	iFileSize = filesize(psFileName);
	iRet = read(fid, str, iFileSize);
	close(psFileName);
	iRet = 0;
}
/************************************************************************/
/* ﹍てKIN㎝keyIdxまゅン                                            */
/*蹦ノ繰篈渺Α玂まㄣ砰
/*KIN 2竊 KeyIdx1竊next竊酚KIN逼
/*眖逼
/*next眖0秨﹍程-1,﹍てゲ斗块ぶKIN㎝KeyIdx
/* return 0 OK, -1 wrong;
/************************************************************************/
int InitKINFile(char* pszFileName, uint uiKIN, uchar ucKeyIdx)
{
	int iRet;
	int fid;
	uchar strGetData[4];
	uchar szTem[2];
	if ( pszFileName == NULL)
	{
		return -1;
	}
    
	fid = open(pszFileName, O_CREATE);
	
	if (fid<0)
	{
		return -1;
	}
	close(fid);
	fid = open(pszFileName, O_RDWR);
	if (fid<0)
	{
		return -1;
	}
	
	PubLong2Char(uiKIN, 2, szTem);
	strGetData[0] = szTem[0];
	strGetData[1] = szTem[1];
	PubLong2Char(ucKeyIdx, 2, szTem);
	strGetData[2] = szTem[1];
	PubLong2Char(1, 2, szTem);
	strGetData[3] = szTem[1];
	iRet = write(fid, strGetData, 4); //糶计沮
	
	//	FileTest(pszFileName);
	
	if (iRet == -1)
	{
		close(fid);
		return -1;
	}
	PubLong2Char(255, 2, szTem);
	strGetData[3] = szTem[1];
	iRet = seek(fid, 4, SEEK_SET);
	if (iRet != 0)
	{
		close(fid);
		return -2;
	}	
	iRet = write(fid, strGetData, 4); //糶计沮
	
	
	
	if (iRet == -1)
	{
		close(fid);
		return -1;
	}
	
	close(fid);
	//	FileTest(pszFileName);
	return 0;
}

/************************************************************************/
/* get keyindex from file system by using KIN                           */
/*蹦ノ繰篈渺Α玂まㄣ砰
/*KIN 2竊 KeyIdx1竊next竊酚KIN逼
/*next眖0秨﹍程255
/************************************************************************/
/* return: 0  OK,  -1 NO keyindex, -2 other reason                      */
/************************************************************************/
int FindKeyidx(char* pszFileName, uint uiKIN, uchar* puiKeyIdx)
{
	int iRet;
	int fid;
	uchar strGetData[4];
	//	uchar szTem[2];
	//	int ii;
	int iCurNext;
	int iLastNext;
	uint iKin;
	//	int iFileSize;
	int iMinKIn;
	int iKeyIdx;
	if ( pszFileName == NULL)
	{
		return -2;
	}
	iRet = fexist(pszFileName);
	if (iRet == -1)
	{
		return -2;
	}
	fid = open(pszFileName, O_RDWR);
	if (fid<0)
	{
		return -2;
	}
	
	//﹍て材计沮
	iLastNext = 0;
	iCurNext =0;
	iRet = seek(fid, iCurNext*4, SEEK_SET);
	if (iRet != 0)
	{
		close(fid);
		return -2;
	}
	iRet = read(fid, strGetData, 4);
	if(iRet == -1)
	{
		close(fid);
		return -2;
	}
	PubChar2Long(strGetData, 2, &iMinKIn);
	PubChar2Long(strGetData+3, 1, &iCurNext);
	iLastNext = iCurNext;
	if (iCurNext < 0)
	{
		close(fid);
		return -2;
	}
	//琩高
	while (iCurNext != 255) //рiCurNext㎝iLastNextΘ皐碞
	{
		
		iRet = seek(fid, iCurNext*4, SEEK_SET);
		if (iRet != 0)
		{
			close(fid);
			return -2;
		}
		iRet = read(fid, strGetData, 4);
		if(iRet == -1)
		{
			close(fid);
			return -2;
		}
		PubChar2Long(strGetData, 2, &iKin);
		
		if (uiKIN == iKin) //find the num
		{
			PubChar2Long(strGetData+2, 1, &iKeyIdx);
			(*puiKeyIdx) = (char)iKeyIdx;
			close(fid);
			return 0; //插入成功。
		}
		iLastNext = iCurNext;
		PubChar2Long(strGetData+3, 1, &iCurNext);
	}
	
	close(fid);
	return -1; //⊿ΤэKIN
}

/************************************************************************/
/* write keyindex into file for associating KIN                         */
/*蹦ノ繰篈渺Α玂まㄣ砰
/*KIN 2竊 KeyIdx1竊next竊酚KIN逼
/*next眖0秨﹍程255
/************************************************************************/
/* return                                                               */
/************************************************************************/
int WriteKeyIdx(char* pszFileName, uint uiKIN, uchar ucKeyIdx)
{
	int iRet;
	int fid;
	uchar strGetData[4];
	uchar szTem[2];
	//	int ii;
	int iCurNext;
	int iLastNext;
	uint iKin;
	int iFileSize;
	uint iMinKIn;
	if ( pszFileName == NULL)
	{
		return -1;
	}
	iRet = fexist(pszFileName);
	if (iRet == -1)
	{
		return iRet;
	}
	iFileSize = filesize(pszFileName);
	if (iFileSize == -1)
	{
		return -1;
	}
	fid = open(pszFileName, O_RDWR);
	if (fid<0)
	{
		return fid;
	}
	
	//﹍て材计沮
	iLastNext = 0;
	iCurNext =0;
	iRet = seek(fid, iCurNext*4, SEEK_SET);
	if (iRet != 0)
	{
		close(fid);
		return -1;
	}
	iRet = read(fid, strGetData, 4); //??
	if(iRet == -1)
	{
		close(fid);
		return -1;
	}
	PubChar2Long(strGetData, 2, &iMinKIn);
	PubChar2Long(strGetData+3, 1, &iCurNext);
	//	iLastNext = iCurNext;
	//琩高
	while (iCurNext != 255) //рiCurNext㎝iLastNextΘ皐碞
	{
		
		iRet = seek(fid, iCurNext*4, SEEK_SET);
		if (iRet != 0)
		{
			close(fid);
			return -1;
		}
		iRet = read(fid, strGetData, 4);
		if(iRet == -1)
		{
			close(fid);
			return -1;
		}
		PubChar2Long(strGetData, 2, &iKin);
		
		if (uiKIN <= iKin) //find the num
		{
			iRet = seek(fid, 0, SEEK_END);
			if (iRet != 0)
			{
				close(fid);
				return -1;
			}
			if (freesize() <= 4) //空闲空间的大小
			{
				close(fid);
				return -1;
			}
			PubLong2Char(uiKIN, 2, szTem);
			strGetData[0] = szTem[0];
			strGetData[1] = szTem[1];
			PubLong2Char(ucKeyIdx, 2, szTem);
			strGetData[2] = szTem[1];
			PubLong2Char(iCurNext, 2, szTem);
			strGetData[3] = szTem[1];
			iRet = write(fid, strGetData, 4); //写入数据
			//	FileTest(pszFileName);
			if (iRet == -1)
			{
				close(fid);
				return -1;
			}
			
			iRet = seek(fid, iLastNext*4, SEEK_SET);//修改指针
			if (iRet != 0)
			{
				close(fid);
				return -1;
			}
			iRet = read(fid, strGetData, 4);
			if(iRet == -1)
			{
				close(fid);
				return -1;
			}
			PubLong2Char((iFileSize/4), 2, szTem);
			strGetData[3] = szTem[1];
			//seek(fid, it*4, SEEK_SET);
			seek(fid, iLastNext*4, SEEK_SET);//修改指针
			iRet = write(fid, strGetData, 4); //写入数据
			if (iRet == -1)
			{
				close(fid);
				return -1;
			}
			close(fid);
			return 0; //插入成功。
		}
		iLastNext = iCurNext;
		PubChar2Long(strGetData+3, 1, &iCurNext);
	}
	iRet = seek(fid, 0, SEEK_END);
	if (iRet != 0)
	{
		close(fid);
		return -1;
	}
	if (freesize() <= 4) //空闲空间的大小
	{
		close(fid);
		return -1;
	}
	PubLong2Char(uiKIN, 2, szTem);
	strGetData[0] = szTem[0];
	strGetData[1] = szTem[1];
	PubLong2Char(ucKeyIdx, 2, szTem);
	strGetData[2] = szTem[1];
	PubLong2Char(iCurNext, 2, szTem);
	strGetData[3] = szTem[1];
	iRet = write(fid, strGetData, 4); //写入数据
	//	FileTest(pszFileName);
	if (iRet == -1)
	{
		close(fid);
		return -1;
	}
	
	iRet = seek(fid, iLastNext*4, SEEK_SET);//修改指针
	if (iRet != 0)
	{
		close(fid);
		return -1;
	}
	iRet = read(fid, strGetData, 4);
	if(iRet == -1)
	{
		close(fid);
		return -1;
	}
	PubLong2Char((iFileSize/4), 2, szTem);
	strGetData[3] = szTem[1];
	//seek(fid, it*4, SEEK_SET);
	seek(fid, iLastNext*4, SEEK_SET);//修改指针
	iRet = write(fid, strGetData, 4); //写入数据
	if (iRet == -1)
	{
		close(fid);
		return -1;
	}
	close(fid);
	return -1;
}

/*//2014-7-7 惠璶эKeyInit瑈祘惠璶ノめ匡拒赣Μ虫︽琌蹦ノIP盞块莱KINぃ惠璶盢俱兵key常も笆块
  Inject Key13
*/
int KeyInit(char* szTitle)
{
	int			iRet;
	uchar* psKCV;
	uchar szTmp[16];

#ifdef LOIS_DEFAULT_KEY
	ST_KEY_INFO	stKeyInfoIn;
	ST_KCV_INFO	stKcvInfoIn;
	uchar ucSrcKeyIdx;
	uchar psKeyBCD[24];
	uchar ucKeyLen;
	uchar ucDstKeyId;
	uchar ucDstKeyType;
	uchar strInput[128];
	int iInputLen;
	//uchar* psKCV;
	long lTem;
	uint uKIN;
	uchar ucAcNum;
	//uchar szTmp[16];
	uchar szKey1[16+1];
	uchar szKey2[16+1];
	int ii;
	static int ifHardCodeFlag = 0;//2015-3-10
  long tempValue;
#endif
#ifdef LOIS_DEFAULT_KEY
	//2015-3-10 ttt
    ScrCls();
	//ifHardCodeFlag = 0;
	PubDispString("Hard Code Key",       DISP_LINE_CENTER|2);
	PubDispString("1.YES 0.NO", DISP_LINE_CENTER|4);
	sprintf((char*)szTmp, "(%c)", (ifHardCodeFlag == 1)? '1':'0');
	PubDispString(szTmp,       DISP_LINE_CENTER|6);

	while (1)
	{
		switch (PubWaitKey(180))
		{
		case KEY1:
			ifHardCodeFlag = 1;
			HardCodeFlag = ifHardCodeFlag;
			sprintf((char*)szTmp, "(%c)", '1');
			PubDispString(szTmp,       DISP_LINE_CENTER|6);
			break;
		case KEY0:
			ifHardCodeFlag = 0;
			sprintf((char*)szTmp, "(%c)", '0');
			PubDispString(szTmp,       DISP_LINE_CENTER|6);
			break;
		case NOKEY:		
		case KEYCANCEL:
			return -1;
		case KEYENTER:
			goto IFENTER1;
			break;		
		default:
			ifHardCodeFlag = 0;
			break;
		}
	}
#endif
#if defined(_S_SERIES_) || defined(_SP30_)
IFENTER1://2015-3-10
	psKCV = NULL;
	PubDispString("KEY SET",       DISP_LINE_CENTER|2);
	sprintf((char*)szTmp, "(%c)", (glCurAcq.ucIsSupportKIN == SupportKIN)? '1':'0');
	PubDispString(szTmp,       DISP_LINE_CENTER|4);
	PubDispString("1.ON 0.OFF", DISP_LINE_CENTER|6);
 
	while (1)
	{
		switch (PubWaitKey(180))
		{
		case KEY1:
			sprintf((char*)szTmp, "(%c)", '1');
			glCurAcq.ucIsSupportKIN == SupportKIN;
			PubDispString(szTmp,       DISP_LINE_CENTER|4);
			break;
		case KEY0:
			glCurAcq.ucIsSupportKIN = NotSupportKIN;
			sprintf((char*)szTmp, "(%c)", '0');
			PubDispString(szTmp,       DISP_LINE_CENTER|4);
			break;
		case NOKEY:		
		case KEYCANCEL:
			return -1;
		case KEYENTER:
			if (szTmp[1] == '0')
			{
				return -1;
			}
			else
				goto IFENTER;
			break;			
		default:
			break;
		}
	}

#ifdef LOIS_DEFAULT_KEY
	//2015-3-10 ttt
IFENTER:
	if(ifHardCodeFlag == 1)
	{

		while (1)
		{
			PubShowTitle(TRUE, szTitle);
			PubDispString("SOURCE KEY INDEX", DISP_LINE_CENTER|2);
			memset(strInput, 0 , 128);

	  #ifdef TEST_SET_DEFAULT
			memcpy(strInput, "0", 1);
	  #endif

			iRet = PubGetString(NUM_IN|ECHO_IN, 1, 3, strInput,60); // 60 seconds to input keyindx of source
			iInputLen = strlen(strInput);
			if (iRet == 0)  //enter
			{
				PubAsc2Long(strInput, iInputLen, &tempValue);
				ucSrcKeyIdx = (unsigned char) tempValue;
				if (ucSrcKeyIdx > 100)
				{
					PubShowTitle(TRUE, szTitle);
					ScrPrint(0, 3, ASCII, "  Sorry, the max key ");
					ScrPrint(0, 4, ASCII, "   index is 100!  ");
					ScrPrint(0, 5, ASCII, "  Please try again!");
					DelayMs(1500);
					continue;
				}
				else
					break;//break while(1)
			}
			else //cancel or timeout. 
			{
				return -1;
			}
		}


		while (1)
		{
			PubShowTitle(TRUE, "DST KEY TYPE"); //PED_GMK,PED_TMK,PED_TPK,PED_TAK,PED_TDK
			ScrPrint(0, 2, ASCII, "1.PED_TLK 2.PED_TMK");
			ScrPrint(0, 3, ASCII, "3.PED_TPK 4.PED_TAK");
			ScrPrint(0, 4, ASCII, "5.PED_TDK");
			memset(strInput, 0 , 128);

	  #ifdef TEST_SET_DEFAULT
				memcpy(strInput, "5", 1);
	  #endif

			iRet = PubGetString(NUM_IN|ECHO_IN, 1, 1, strInput,60); // 60 seconds to input keyindx of source
			iInputLen = strlen(strInput);
			if (iRet == 0)  //enter
			{
				PubAsc2Long(strInput, iInputLen, &lTem);
				switch (lTem)
				{
				case 1:
					//ucDstKeyType = PED_TLK;
					PubShowTitle(TRUE, "SORRY");
					ScrPrint(0, 3, ASCII, "   Sorry to say that");
					ScrPrint(0, 4, ASCII, "  we are not support  ");
					ScrPrint(0, 5, ASCII, "   this kind of key   ");	
					DelayMs(1500);
				//	return -1;
					break;
				case 2:
					ucDstKeyType = PED_TMK;
					break;
				case 3:
					ucDstKeyType = PED_TPK;
					break;
				case 4:
					ucDstKeyType = PED_TAK;
					break;
				case 5:
					ucDstKeyType = PED_TDK;
					break;
				default:
					PubShowTitle(TRUE, "DST KEY TYPE"); //PED_GMK,PED_TMK,PED_TPK,PED_TAK,PED_TDK
					ScrPrint(0, 3, ASCII, "     Wrong number ");
					ScrPrint(0, 4, ASCII, "  Please try again!");
					DelayMs(1500);
					break;
				}
				if ((lTem <= 5) && (lTem >= 2))
				{
					break;  //break while(1)
				}
			
			}
			else //cancel or timeout. 
			{
				return -1;
			}
		}

	
		while (1)
		{
			PubShowTitle(TRUE, szTitle);
			PubDispString("DST KEY LENGTH", DISP_LINE_CENTER|2);
			PubDispString("(BYTES)", DISP_LINE_CENTER|4);
			memset(strInput, 0 , 128);

	  #ifdef TEST_SET_DEFAULT
				memcpy(strInput, "16", 2);
	  #endif

			iRet = PubGetString(NUM_IN|ECHO_IN, 1, 2, strInput,60); // 30 seconds to input keyindx of source
			iInputLen = strlen(strInput);
			if (iRet == 0)  //enter
			{
		  PubAsc2Long(strInput, iInputLen, &tempValue);	
		  ucKeyLen = (unsigned char) tempValue;

		  if (!((ucKeyLen == 8) ||(ucKeyLen == 16) || (ucKeyLen == 24)))
				{
					PubShowTitle(TRUE, szTitle);
					ScrPrint(0, 3, ASCII, "  The value must be ");
					ScrPrint(0, 4, ASCII, "  8 or 16 or 24");
					ScrPrint(0, 5, ASCII, "  Please try again!");
					DelayMs(1500);
					continue;
				}
				else
				{
					break; //break while(1)
				}
			}
			else //cancel or timeout. 
			{
				return -1;
			}
		}

		while (1)
		{

			while (1)
			{
				PubShowTitle(TRUE, szTitle);
				PubDispString("DST KEY INDEX", DISP_LINE_CENTER|2);
				memset(strInput, 0 , 128);

	  #ifdef TEST_SET_DEFAULT
				memcpy(strInput, "3", 1);
	  #endif
      
		  iRet = PubGetString(NUM_IN|ECHO_IN, 1, 3, strInput,60); // 60 seconds to input keyindx of source
				iInputLen = strlen(strInput);
				if (iRet == 0)  //enter
				{
					PubAsc2Long(strInput, iInputLen, &tempValue);	
					ucDstKeyId = (unsigned char) tempValue;

			if (ucDstKeyId > 100)
					{
						PubShowTitle(TRUE, szTitle);
						ScrPrint(0, 3, ASCII, "  Sorry, the max key ");
						ScrPrint(0, 4, ASCII, "   index is 100!  ");
						ScrPrint(0, 5, ASCII, "  Please try again!");
						DelayMs(1500);
						continue;
					}
					else
						break;
				}
				else //cancel or timeout. 
				{
					return -1;
				}
			}
		
		
			while (1)
			{
				PubShowTitle(TRUE, szTitle);
				PubDispString("KIN", DISP_LINE_CENTER|2);
				memset(strInput, 0 , 128);

	  #ifdef TEST_SET_DEFAULT
				memcpy(strInput, "3", 1);
	  #endif

				iRet = PubGetString(NUM_IN|ECHO_IN, 1, 4, strInput,60); // 30 seconds to input keyindx of source
				iInputLen = strlen(strInput);
				if (iRet == 0)  //enter
				{
					PubAsc2Long(strInput, iInputLen, &tempValue);
					uKIN = (unsigned int) tempValue;

					if (uKIN > 4095) //4095是按照EFTSec说明来的。
					{
						PubShowTitle(TRUE, szTitle);
						ScrPrint(0, 3, ASCII, "  Sorry, the max ");
						ScrPrint(0, 4, ASCII, "   KIN is 4095!  ");
						ScrPrint(0, 5, ASCII, "  Please try again!");
						DelayMs(1500);
						continue;
					}
					else
						break;
				}
				else //cancel or timeout. 
				{
					return -1;
				}
			}


			while (1)
			{
				PubShowTitle(TRUE, szTitle);
				PubDispString("KEY COMPONENT 1 ", DISP_LINE_CENTER|2);
				memset(strInput, 0 , 128);

	  #ifdef TEST_SET_DEFAULT
		  //!!!ray: set default component for testing 
		  memcpy(strInput, "ABCDEF0123456789EEEEEEEEEEEEEEEE", 32);
		  //end raymond
	  #endif

				iRet = PubGetString(NUM_IN|ALPHA_IN|ECHO_IN, 8, 32, strInput,240); // 120 seconds to input keyindx of source
				iInputLen = strlen(strInput);
				if (iRet == 0)  //enter
				{
					if ( (iInputLen/2) == ucKeyLen)
					{
						PubAsc2Bcd(strInput, iInputLen, szKey1);
						break;
					}
					else
					{
						PubShowTitle(TRUE, szTitle);
						ScrPrint(0, 3, ASCII, " Sorry, the length ");
						ScrPrint(0, 4, ASCII, " of key doesn't match  ");
						ScrPrint(0, 5, ASCII, "  Please try again!");
						DelayMs(1500);
						continue;
					}
				}
				else //cancel or timeout. 
				{
					return -1;
				}
			}
		
			while (1)
			{
				PubShowTitle(TRUE, szTitle);
				PubDispString("KEY COMPONENT 2 ", DISP_LINE_CENTER|2);
				memset(strInput, 0 , 128);

	  #ifdef TEST_SET_DEFAULT
		  //!!!ray: set default component for testing 
		  memcpy(strInput, "FFFFFFFFFFFFFFFF9876543210FEDCBA", 32);
		  //end raymond
	  #endif

				iRet = PubGetString(NUM_IN|ALPHA_IN|ECHO_IN, 8, 32, strInput,240); // 120 seconds to input keyindx of source
				iInputLen = strlen(strInput);
				if (iRet == 0)  //enter
				{
					if ( (iInputLen/2) == ucKeyLen)
					{
						PubAsc2Bcd(strInput, iInputLen, szKey2);
						memset(psKeyBCD, 0 , 24);
						for (ii=0; ii < ucKeyLen; ii++)
						{
							psKeyBCD[ii] = szKey1[ii] ^ szKey2[ii];
						}
						break;
					}
					else
					{
						PubShowTitle(TRUE, szTitle);
						ScrPrint(0, 3, ASCII, " Sorry, the length ");
						ScrPrint(0, 4, ASCII, " of key doesn't match  ");
						ScrPrint(0, 5, ASCII, "  Please try again!");
						DelayMs(1500);
						continue;
					}
				}
				else //cancel or timeout. 
				{
					return -1;
				}
			}

			PubShowTitle(TRUE, szTitle);
			PubDispString("KCV", DISP_LINE_CENTER|2);
			memset(strInput, 0 , 128);
			iRet = PubGetString(NUM_IN|ALPHA_IN|ECHO_IN, 0, 128, strInput,600); // 600 seconds to input keyindx of source
			iInputLen = strlen(strInput);
			if (iRet == 0)  //enter
			{
				if (iInputLen == 0)
				{
					psKCV = NULL;
				}
				else
					psKCV = strInput;
				
			}
			else //cancel or timeout. 
			{
				return -1;
			}
		

			iRet = SxxWriteKey(ucSrcKeyIdx, psKeyBCD, ucKeyLen, ucDstKeyId, ucDstKeyType, psKCV);//2015-3-10 ttt write key! 1)
			if (iRet != 0)
			{
				PubShowTitle(TRUE, szTitle);
				PubDispString("Fail to write!", DISP_LINE_CENTER|2);
				PubDispString("     KEY  ", DISP_LINE_CENTER|4);
				DelayMs(1000);
			}
			else
			{
				glCurAcq.ulKeyIdx = ucDstKeyId;
				glCurAcq.ucIsSupportKIN = SupportKIN;
				glCurAcq.ucIsKeyBlank = KeyNotBlank;
				glCurAcq.ulKIN = uKIN;
			}

			return 0;

		}

		return 0;
	}
	/*else
	{
		//2015-11-17 
		if(glCurAcq.ucIsSupportKIN == SupportKIN && glKeyInjectOK)//already successfully injected, ask if re-inject
		{
			PubShowTitle(TRUE, szTitle);
			ScrPrint(0, 4, ASCII, "  Re-Inject Key?");
			PubDispString("YES/NO", DISP_LINE_CENTER|6);
			while (1)
			{
				switch (PubWaitKey(180))
				{
				case NOKEY:		
				case KEYCANCEL:			
					return 0;		
				case KEYENTER:
					goto IFYES;
				default:
					break;
				}
			
			}

		}*/
	else
	{
//IFYES:
	iRet = KmsMain();
	return iRet;
	}
}
#else

IFENTER:
	//2015-11-17 
	if(glCurAcq.ucIsSupportKIN == SupportKIN && glKeyInjectOK)//already successfully injected, ask if re-inject
	{
		PubShowTitle(TRUE, szTitle);
		ScrPrint(0, 4, ASCII, "  Re-Inject Key?");
		PubDispString("YES/NO", DISP_LINE_CENTER|6);
		while (1)
		{
			switch (PubWaitKey(180))
			{
			case NOKEY:		
			case KEYCANCEL:			
				return 0;		
			case KEYENTER:
				goto IFYES;
			default:
				break;
			}
			
		}

	}

IFYES:
	iRet = KmsMain();
	return iRet;
#endif
#else
	PubShowTitle(TRUE, "KEY INJECTING"); //PED_GMK,PED_TMK,PED_TPK,PED_TAK,PED_TDK
	PubDispString("This Pos don't ", DISP_LINE_CENTER|2);
	PubDispString("support EFTSec!", DISP_LINE_CENTER|4);
	memset(strInput, 0 , 128);
#endif
#endif
/*----------------2014-5-20 IP encryption----------------*/








