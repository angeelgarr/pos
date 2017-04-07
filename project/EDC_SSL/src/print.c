/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2006-? PAX Technology, Inc. All rights reserved.
 * Description:    定义打印模块
 * Revision History:
 * Date                  Author                 Action
 * 2006.09.12        shengjx                      created
 * ============================================================================
 */

#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
enum{
    PRN_SIZE_SMALL  = 20,
	PRN_SIZE_NORMAL = 27,
	PRN_SIZE_LARGE  = 40,
};
static char bOpenPrinter = FALSE;
static char sg_SMS_Buffer[2048] = {0};

extern int WlPortOpen(const char *Attr);
extern int WlPortRecv(void *RecvBuf, int RecvLen, int TimeoutMs);
extern void WlPortReset();
extern int WlPortSend(const void *SendBuf, int SendLen);
extern int PrnBmp(unsigned char *filename,int mode,char alignment, unsigned char *gMallocBuffer);

int PrnInit()
{
    if(!bOpenPrinter)
    {
        if(!ChkHardware(HWCFG_PRINTER, 0))
        {
            int ret = OsPrnOpen(PRN_REAL, NULL);
            if(ret != RET_OK)
                return ret;
        }
        bOpenPrinter = TRUE;
    }
    OsPrnReset();
    FtFontStyle(0,0);
    FtFontAlign(FT_ALIGN_LEFT);
    PrnSetNormal();
    return RET_OK;
}

static int  PrintReceipt_FreeFmat(uchar ucPrnFlag);
static int  PrintReceipt_SMS(uchar ucPrnFlag);
static int  PrintReceipt_T(uchar ucPrnFlag);
static int  PrnCurAcqTransList_T(void);
static void PrnHead(uchar ucFreeFmat);
static int  PrnCustomLogo_T(void);
static void PrnHead_T(void);
static void PrnAmount(const uchar *pszIndent, uchar ucNeedSepLine);
static void PrnDescriptor(void);
static void PrnAdditionalPrompt(void);
static void PrnStatement(void);
static void PrnTotalInfo(const void *pstInfo);
static int  PrnParaAcq(uchar ucAcqIndex);
static void PrnParaIssuer(uchar ucAcqIndex);
static void PrnParaIssuerSub(uchar ucIssuerKey);
static void PrnIssuerOption(const uchar *psOption);
static void PrnCardTable(uchar ucIssuerKey);
static int  PrnInstalmentPara(void);
static int  PrnEmvPara(void);
static void PrnHexString(const char *pszTitle, const uchar *psHexStr, int iLen, uchar bNewLine);
static int  DispPrnError(int iErrCode);

static void  PrnEngTime(void);

/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// For thermal, small={8x16,16x16}
// Modified by Kim_LinHB 2014-6-8
void PrnSetSmall(void)
{
	FtFontSize(PRN_SIZE_SMALL);
}

// For thermal,  normal = {12x24,24x24}
// For sprocket, normal = {6x8,16x16}
void PrnSetNormal(void)
{
    FtFontSize(PRN_SIZE_NORMAL);
}

// For thermal, normal=big={12x24,24x24}
// For sprocket, big={8x16,16x16}
void PrnSetBig(void)
{
    FtFontSize(PRN_SIZE_LARGE);
}

int PrintReceipt(uchar ucPrnFlag)
{
	uchar	szBuff[100];
	uchar	szIssuerName[10+1];

	if( !ChkIssuerOption(ISSUER_EN_PRINT) )
	{
		return 0;
	}

	if (ChkIfIrDAPrinter())
	{
		SetOffBase(OffBaseDisplay);
		ChkOnBase();
	}

	if(ChkHardware(HWCFG_PRINTER, 0))
    {
	    //TODO if()  acquirer printer type for now, just SMS cuz there are so many BT printer brands, and the current architecture is unable to add those codes agilely
	    int iRet;
	    uchar   szBuff[30] = {0};
	    GUI_INPUTBOX_ATTR stInputAttr;
	    LoadNum_SymKeyBoard();

	    memset(&stInputAttr, 0, sizeof(stInputAttr));
	    stInputAttr.bEchoMode = 1;
	    stInputAttr.nMinLen = 1;
	    stInputAttr.nMaxLen = 25;

	    Gui_ClearScr();
	    iRet = Gui_ShowInputBox(GetCurrTitle(), gl_stTitleAttr, _T("PHONE NO"), gl_stLeftAttr,
	        szBuff, gl_stRightAttr, &stInputAttr, USER_OPER_TIMEOUT);
	    if(iRet != GUI_OK)
	        return 0;
	    if(memcmp(szBuff, "00", 2) && memcmp(szBuff, "+", 1))
	    {
	        uchar   szCountryCallingCode[120];
	        GetEnv("TELCODE", szCountryCallingCode);
	        sprintf(glProcInfo.stTranLog.szLastCustomerPhoneNo, "%s%s", szCountryCallingCode, szBuff);
	    }
	    else
	        strcpy(glProcInfo.stTranLog.szLastCustomerPhoneNo, szBuff);
	    DispSend();
        return PrintReceipt_SMS(ucPrnFlag);
    }

	DispPrinting();
	if( ChkIfThermalPrinter() )
	{
		return PrintReceipt_T(ucPrnFlag);
	}
	
	if( ChkEdcOption(EDC_FREE_PRINT) )	// Free format print
	{
		return PrintReceipt_FreeFmat(ucPrnFlag);
	}

	PrnInit();
	PrnSetNormal();

	PrnStep(30);
	PrnHead(FALSE);

	ConvIssuerName(glCurIssuer.szName, szIssuerName);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n", szIssuerName); // issuer Name

	//	memcpy(szBuff, glProcInfo.stTranLog.szPan, sizeof(glProcInfo.stTranLog.szPan));
	if (ChkIfTransMaskPan(1))
	{
		MaskPan(glProcInfo.stTranLog.szPan, szBuff);
	}
	else
	{
		strcpy(szBuff, glProcInfo.stTranLog.szPan);
	}

	if( glProcInfo.stTranLog.uiEntryMode & MODE_SWIPE_INPUT )
	{
	    strcat(szBuff, " S\n");
	}
	else if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	{
	    strcat(szBuff, " C\n");
	}
	else if( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
			 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
	    strcat(szBuff, " F\n");
	}
    else if(glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS)//bug806
    {
        strcat(szBuff, " T\n");
    }
	else
	{
	    strcat(szBuff, " M\n");
	}
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, szBuff);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n", glProcInfo.stTranLog.szHolderName);

	// print txn name & expiry
	if( glProcInfo.stTranLog.ucTranType==VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
	{		
		sprintf((char *)szBuff, "%s(%s)", _T("VOID"), _T(glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel));
	}
	else if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
	{
		sprintf((char *)szBuff, "%s(%s)", _T(glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel), _T("ADJ"));
	}
	else
	{
		sprintf((char *)szBuff, "%s", _T(glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel));
	}
	if( ChkIssuerOption(ISSUER_EN_EXPIRY) )
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_LARGE, " %-16.16s", szBuff);
		if( ChkIssuerOption(ISSUER_MASK_EXPIRY) )
		{
		    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "**/**\n");
		}
		else
		{
		    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%2.2s/%2.2s\n", &glProcInfo.stTranLog.szExpDate[2],
					glProcInfo.stTranLog.szExpDate);
		}
	}
	else
	{
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_LARGE, " %s\n", szBuff);
	}

	// Batch NO & invoice #
	PrnStep(6);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%06ld", glCurAcq.ulCurBatchNo);
	MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%06ld\n", glProcInfo.stTranLog.ulInvoiceNo);

	Conv2EngTime(glProcInfo.stTranLog.szDateTime, szBuff);  //DATE/TIME
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n", szBuff);

	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%12s", glProcInfo.stTranLog.szRRN);
    MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%06s\n", glProcInfo.stTranLog.szAuthCode);

    PrnStep(2);
	if( glProcInfo.stTranLog.ucInstalment!=0 )
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s:%02d\n", _T("NO. OF INSTALMENT"), glProcInfo.stTranLog.ucInstalment);
	}

	if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	{
        if (strlen(glProcInfo.stTranLog.szAppPreferName)!=0)
        {
            MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s: %.16s\n", _T("APP"), glProcInfo.stTranLog.szAppPreferName);
        } 
        else
        {
            MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s: %.16s\n", _T("APP"), glProcInfo.stTranLog.szAppLabel);
        }
		PubBcd2Asc0(glProcInfo.stTranLog.sAID, glProcInfo.stTranLog.ucAidLen, szBuff);
		PubTrimTailChars(szBuff, 'F');
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s: %s\n", _T("AID"), szBuff);
		PubBcd2Asc0(glProcInfo.stTranLog.sAppCrypto, 8, szBuff);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s : %s\n", _T("TC"), szBuff);
#ifdef ENABLE_EMV
#ifdef EMV_TEST_VERSION
// 		PubBcd2Asc0(glProcInfo.stTranLog.sTSI, 2, szBuff);
// 		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s: %s\n", _T("TSI"), szBuff);
// 		PubBcd2Asc0(glProcInfo.stTranLog.sTVR, 5, szBuff);
// 		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s: %s\n", _T("TVR"), szBuff);
#endif
#endif
		if( glProcInfo.stTranLog.uiEntryMode & MODE_OFF_PIN )
		{
		    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s\n", _T("PIN VERIFIED"));
		}
		else
		{
		    FtNextLine(); //MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n");
		}
	}

	PrnDescriptor();

	// amount
	PrnAmount((uchar *)"   ", TRUE);

	PrnAdditionalPrompt();

	PrnStatement();

	if( ucPrnFlag==PRN_REPRINT )
	{
		MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_LARGE, "%s\n", _T("REPRINT"));
	}
	if (ChkIfTrainMode())
	{
	    MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "**** %s ****\n", _T("DEMO"));
	}
	else
	{
#ifdef ENABLE_EMV
#ifdef EMV_TEST_VERSION
	    MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "* %s *\n", _T("FOR EMV TEST ONLY"));
#endif
#endif
	}

    if(!ChkHardware(HWCFG_PRINTER, 0))
        MultiLngPrnStr(FT_ALIGN_LEFT,   PRN_SIZE_NORMAL, "\n\n\n\n");

	StartPrinter();
	return 0;
}

// 打印明细
// Print the list of all transaction
int PrnAllList(void)
{
	int		iRet;
	uchar	ucIndex;

	SetCurrTitle(_T("PRINT LOG")); // Added by Kim_LinHB 2014/9/16 v1.01.0009 bug493

	iRet = SelectAcq(FALSE, GetCurrTitle(), &ucIndex);
	if( iRet!=0 )
	{
		return ERR_NO_DISP;
	}

	if( ucIndex!=ACQ_ALL )
	{
//		SetCurAcq(ucIndex);  //ALEX ADD
		PrnCurAcqTransList();
		return ERR_NO_DISP;
	}

	for(ucIndex=0; ucIndex<glSysParam.ucAcqNum; ucIndex++)
	{
		SetCurAcq(ucIndex);
		PrnCurAcqTransList();
	}
	return 0;
}

// print list of transaction of current acquirer
int PrnCurAcqTransList(void)
{
	int		iCnt, iNumOfOnePage, iPageNum, iMaxNumOfOnePage;
	uchar	szBuff[30], szIssuerName[10+1];
	uchar   szDateTime[14+1], szTranName[16+1];

	SetCurrTitle(_T("PRINT LOG"));

	if( GetTranLogNum(glCurAcq.ucKey)==0 )
	{
		PubBeepErr();
		Gui_ClearScr();
		Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_WARNING, _T("EMPTY BATCH"), gl_stCenterAttr, GUI_BUTTON_CANCEL, 5, NULL);
		return 1;
	}

	DispPrinting();
	if( ChkIfThermalPrinter() )
	{
		return PrnCurAcqTransList_T();
	}

	PrnInit();

	PrnSetNormal();

	PrnStep(30);
	MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "%s\n", _T("TRANSACTION LIST"));

	GetDateTime(szDateTime);
	Conv2EngTime(szDateTime, szBuff);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n", szBuff);

	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %.3s", _T("HOST NII"), glCurAcq.szNii);
	MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%.10s\n", glCurAcq.szName);

	PrnStep(15);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%14.8s\n%21.15s\n\n", glCurAcq.szTermID, glCurAcq.szMerchantID);

	iMaxNumOfOnePage = ChkEdcOption(EDC_TIP_PROCESS) ? 4 : 6;
	iNumOfOnePage = 0;
	iPageNum = 1;
	for(iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
	{
		if( glSysCtrl.sAcqKeyList[iCnt]!=glCurAcq.ucKey )
		{
			continue;
		}

		memset(&glProcInfo.stTranLog, 0, sizeof(TRAN_LOG));
		LoadTranLog(&glProcInfo.stTranLog, (ushort)iCnt);
		FindIssuer(glProcInfo.stTranLog.ucIssuerKey);  //Alex add
		ConvIssuerName(glCurIssuer.szName, szIssuerName);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %.10s\n", _T("ISSUER"), szIssuerName);

		if( ChkIfDispMaskPan2() )
		{
			MaskPan(glProcInfo.stTranLog.szPan, szBuff);
			MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s\n", _T("PAN"), szBuff);
		}
		else
		{
		    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s\n", _T("PAN"), glProcInfo.stTranLog.szPan);
		}

		sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel);
		if( glProcInfo.stTranLog.uiStatus & TS_VOID )
		{
			sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel);
			sprintf((char *)szBuff, "%s(%s)", _T(szTranName), _T("VOID"));
		}
		else if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
		{
		    sprintf((char *)szBuff, "%s(%s)", _T(szTranName), _T("ADJ"));
		}
		else
		{
		    sprintf((char *)szBuff, "%s", _T(szTranName));
		}
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%21.21s", szBuff);
	    MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%06lu\n", glProcInfo.stTranLog.ulInvoiceNo);
		PrnAmount((uchar *)"", FALSE);
		FtNextLine(); //MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n");

		iNumOfOnePage++;
		if( (iPageNum==1 && iNumOfOnePage==3) ||
			(iPageNum!=1 && iNumOfOnePage==iMaxNumOfOnePage) )
		{

		    if(!ChkHardware(HWCFG_PRINTER, 0))
		        MultiLngPrnStr(FT_ALIGN_LEFT,   PRN_SIZE_NORMAL, "\n\n\n\n");

			if( StartPrinter()!=0 )
			{
				return 1;
			}

			iNumOfOnePage = 0;
			iPageNum++;

			PrnInit();
			PrnStep(20);
		}
	}
	MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "%s", _T("END OF LIST"));

    if(!ChkHardware(HWCFG_PRINTER, 0))
        MultiLngPrnStr(FT_ALIGN_LEFT,   PRN_SIZE_NORMAL, "\n\n\n\n");

	return StartPrinter();
}

int PrnCurAcqTransList_T(void)
{
	int		iCnt, iNumOfOnePage;
	uchar	szBuff[30], szIssuerName[10+1];
	uchar   szTranName[16+1];

	PrnInit();

	PrnStep(30);

	MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "%s\n", _T("TRANSACTION LIST"));

	PrnEngTime();

	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %.3s", _T("HOST NII"), glCurAcq.szNii);
	MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%.10s\n", glCurAcq.szName);

	PrnStep(15);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s\n%s: %s\n", _T("TID"), glCurAcq.szTermID, _T("MID"), glCurAcq.szMerchantID);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "============================\n");

	iNumOfOnePage = 0;
	for(iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
	{
		if( glSysCtrl.sAcqKeyList[iCnt]!=glCurAcq.ucKey )
		{
			continue;
		}

		memset(&glProcInfo.stTranLog, 0, sizeof(TRAN_LOG));
		LoadTranLog(&glProcInfo.stTranLog, (ushort)iCnt);
		FindIssuer(glProcInfo.stTranLog.ucIssuerKey);  //Alex add
		ConvIssuerName(glCurIssuer.szName, szIssuerName);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %.10s\n", _T("ISSUER"), szIssuerName);

		if( ChkIfDispMaskPan2() )
		{
			MaskPan(glProcInfo.stTranLog.szPan, szBuff);
			MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s\n", _T("PAN"), szBuff);
		}
		else
		{
		    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s\n", _T("PAN"), glProcInfo.stTranLog.szPan);
		}

		sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel);
		if( glProcInfo.stTranLog.uiStatus & TS_VOID )
		{
			sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel);
			sprintf((char *)szBuff, "%s(%s)", _T(szTranName), _T("VOID"));
		}
		else if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
		{
			sprintf((char *)szBuff, "%s(%s)", _T(szTranName), _T("ADJ"));
		}
		else
		{
			sprintf((char *)szBuff, "%s", _T(szTranName));
		}
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %-21.21s\n%s: %06lu\n", _T("TXN"), szBuff, _T("TRACE"), glProcInfo.stTranLog.ulInvoiceNo);
		PrnAmount((uchar *)"", FALSE);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "============================\n");
		
		iNumOfOnePage++;
		if( (iNumOfOnePage%5)==0 )
		{
			if( StartPrinter()!=0 )
			{
				return 1;
			}
			iNumOfOnePage = 0;
			PrnInit();
			PrnStep(30);
		}
	}

	MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "%s\n\n\n\n\n", _T("END OF LIST"));

    if(!ChkHardware(HWCFG_PRINTER, 0))
        MultiLngPrnStr(FT_ALIGN_LEFT,   PRN_SIZE_NORMAL, "\n\n\n\n");

	return StartPrinter();	
}

int PrintReceipt_FreeFmat(uchar ucPrnFlag)
{
	uchar	szBuff[50];
	uchar	szIssuerName[10+1];

	PrnInit();

    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n\n\n");
	PrnHead(TRUE);

	// issuer Name
	ConvIssuerName(glCurIssuer.szName, szIssuerName);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %-10.10s", _T("CARD TYPE"), szIssuerName);

	// Expiry date
	if( ChkIssuerOption(ISSUER_EN_EXPIRY) )
	{
		if( ChkIssuerOption(ISSUER_MASK_EXPIRY) )
		{
		    MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "**/**");
		}
		else
		{
		    MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%2.2s/%2.2s", &glProcInfo.stTranLog.szExpDate[2],
					glProcInfo.stTranLog.szExpDate);
		}
	}

	FtNextLine(); //MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n");

	//	PAN
	if (ChkIfTransMaskPan(1))
	{
		MaskPan(glProcInfo.stTranLog.szPan, szBuff);
	}
	else
	{
		strcpy(szBuff, glProcInfo.stTranLog.szPan);
	}
	if( glProcInfo.stTranLog.uiEntryMode & MODE_SWIPE_INPUT )
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s S\n", _T("CARD NO"), szBuff);
	}
	else if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s C\n", _T("CARD NO"), szBuff);
	}
	else if( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
			 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s F\n", _T("CARD NO"), szBuff);
	}
	else
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s M\n", _T("CARD NO"), szBuff);
	}

	// Holder
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s\n", _T("HOLDER"), glProcInfo.stTranLog.szHolderName);

	// print txn name & expiry
	if( glProcInfo.stTranLog.ucTranType==VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
	{		
		sprintf((char *)szBuff, "%s(%s)", _T("VOID"), _T(glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel));
	}
	else if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
	{
		sprintf((char *)szBuff, "%s(%s)", _T(glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel), _T("ADJ"));
	}
	else
	{
		sprintf((char *)szBuff, "%s", _T(glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel));
	}
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n", szBuff);

	// Batch NO & invoice #
	PrnStep(6);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %06ld", _T("BATCH NO"), glCurAcq.ulCurBatchNo);
	MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%s: %06ld\n", _T("REF"), glProcInfo.stTranLog.ulInvoiceNo);

	// RRN, AuthCode
    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %-12.12s", _T("RRN"), glProcInfo.stTranLog.szRRN);
    MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%s: %-6.6s\n", _T("AUTH"), glProcInfo.stTranLog.szAuthCode);

	PrnStep(2);
	if( glProcInfo.stTranLog.ucInstalment!=0 )
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s:%02d\n", _T("NO. OF INSTALMENT"), glProcInfo.stTranLog.ucInstalment);
	}

	if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	{
        if (strlen(glProcInfo.stTranLog.szAppPreferName)!=0)
        {
            MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s: %.16s\n", _T("APP"), glProcInfo.stTranLog.szAppPreferName);
        } 
        else
        {
            MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s: %.16s\n", _T("APP"), glProcInfo.stTranLog.szAppLabel);
        }
		PubBcd2Asc0(glProcInfo.stTranLog.sAID, glProcInfo.stTranLog.ucAidLen, szBuff);
		PubTrimTailChars(szBuff, 'F');
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s: %s\n", _T("AID"), szBuff);
		PubBcd2Asc0(glProcInfo.stTranLog.sAppCrypto, 8, szBuff);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s: %s\n", _T("T C"), szBuff);
#ifdef ENABLE_EMV
#ifdef EMV_TEST_VERSION
// 		PubBcd2Asc0(glProcInfo.stTranLog.sTSI, 2, szBuff);
// 		PrnStr("   TSI: %s\n", szBuff);
// 		PubBcd2Asc0(glProcInfo.stTranLog.sTVR, 5, szBuff);
// 		PrnStr("   TVR: %s\n", szBuff);
#endif
#endif
		if( glProcInfo.stTranLog.uiEntryMode & MODE_OFF_PIN )
		{
		    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s\n", _T("PIN VERIFIED"));
		}
		else
		{
		    FtNextLine(); //MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n");
		}
	}

	PrnDescriptor();

	PrnAmount((uchar *)"   ", TRUE);

	PrnAdditionalPrompt();

	PrnStatement();

	if( ucPrnFlag==PRN_REPRINT )
	{
        MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_LARGE, "%s\n", _T("REPRINT"));
	}
	if (ChkIfTrainMode())
	{
	    MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "**** %s ****\n", _T("DEMO"));
	}

    if(!ChkHardware(HWCFG_PRINTER, 0))
        MultiLngPrnStr(FT_ALIGN_LEFT,   PRN_SIZE_NORMAL, "\n\n\n\n");

	StartPrinter();
	return 0;
}

static int  PrintReceipt_SMS(uchar ucPrnFlag)
{
    uchar   ucNum;
    uchar   szTotalAmt[12+1];
    uchar   szBuff[50];
    uchar   szTranName[16+1], szOrgTranName[16+1];

    for(ucNum=0; ucNum<1/*NumOfReceipt()*/; ucNum++)
    {
        PrnInit();

        // Transaction type, Expiry date
        sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel);
        sprintf(szOrgTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel);
        if (glProcInfo.stTranLog.ucTranType == VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID))
        {
            sprintf((char *) szBuff, "%s %s", _T(szTranName), _T(szOrgTranName));
        }
        else if (glProcInfo.stTranLog.uiStatus & TS_ADJ)
        {
            sprintf((char *) szBuff, "%s(%s)", _T(szTranName), _T("ADJ"));
        }
        else
        {
            sprintf((char *) szBuff, "%s", szTranName);
        }

        MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_LARGE, "You have a %s Trx on your card:", szBuff);

        // PAN(CARD NUMBER)
        if (ChkIfTransMaskPan(ucNum))
        {
            MaskPan(glProcInfo.stTranLog.szPan, szBuff);
        }
        else
        {
            strcpy((char *)szBuff, (char *)glProcInfo.stTranLog.szPan);
        }

        MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_LARGE, "%s.", szBuff);

        if (!ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount))
        {
            PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
        }
        else
        {
            memcpy(szTotalAmt, glProcInfo.stTranLog.szAmount, sizeof(szTotalAmt));
        }
        App_ConvAmountTran(szTotalAmt, szBuff, 0);

       MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_LARGE, " TOTAL: %s", szBuff);


        // Date, Time
        Conv2EngTime(glProcInfo.stTranLog.szDateTime, szBuff);
        MultiLngPrnStr(FT_ALIGN_LEFT,  PRN_SIZE_NORMAL, " %s%-*.*s", _T("DATE: "), strlen(szBuff) - 5, strlen(szBuff) - 5, szBuff);
        MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, " %s%5.5s",  _T("TIME: "), szBuff+strlen(szBuff) - 5);

        if(!ChkHardware(HWCFG_PRINTER, 0))
            MultiLngPrnStr(FT_ALIGN_LEFT,   PRN_SIZE_NORMAL, "\n\n\n\n");
        StartPrinter();

        if( ucNum==0 && NumOfReceipt() != 1)
        {
            kbflush();

            Gui_ClearScr();
            Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, _T("PRESS ANY KEY"), gl_stCenterAttr, GUI_BUTTON_NONE, USER_OPER_TIMEOUT, NULL);
        }
    }

    return 0;
}


int PrintReceipt_T(uchar ucPrnFlag)
{	
	uchar	ucNum;
	uchar	szBuff[50],szBuf1[50];
	uchar	szIssuerName[10+1], szTranName[16+1];

	for(ucNum=0; ucNum<NumOfReceipt(); ucNum++)
	{
		PrnInit();
		PrnCustomLogo_T();

		PrnHead_T();

		// issuer Name
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_SMALL, "%s:\n", _T("CARD TYPE"));

		ConvIssuerName(glCurIssuer.szName, szIssuerName);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n", szIssuerName);

		// PAN
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_SMALL, "%s\n", _T("CARD NO./EXP. DATE"));
		//memcpy(szBuff, glProcInfo.stTranLog.szPan, sizeof(glProcInfo.stTranLog.szPan));

		if (ChkIfTransMaskPan(ucNum))
		{
			MaskPan(glProcInfo.stTranLog.szPan, szBuff);
		}
		else
		{
			strcpy((char *)szBuff, (char *)glProcInfo.stTranLog.szPan);
		}

		if( glProcInfo.stTranLog.uiEntryMode & MODE_SWIPE_INPUT )
		{
			sprintf(szBuf1, "%s (S)", szBuff);
		}
		else if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
		{
			sprintf(szBuf1, "%s (C)", szBuff);
		}
		else if( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
				 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
		{
			sprintf(szBuf1, "%s (F)", szBuff);
		}
		else if(glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS)//bug806
		{
		    sprintf(szBuf1, "%s (T)", szBuff);
		}
		else
		{
			sprintf(szBuf1, "%s (M)", szBuff);
		}

		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%-23.23s", szBuf1);

		// print expiry
		if( ChkIssuerOption(ISSUER_EN_EXPIRY) )
		{
			if( ChkIssuerOption(ISSUER_MASK_EXPIRY) )
			{
			    MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "**/**");
			}
			else
			{
				MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%2.2s/%2.2s", &glProcInfo.stTranLog.szExpDate[2],
						glProcInfo.stTranLog.szExpDate);
			}
		}

		FtNextLine(); //MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n");

		// Holder name
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_SMALL, "%s\n", _T("HOLDER"));
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%-23.23s\n", glProcInfo.stTranLog.szHolderName);

		// Transaction type
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_SMALL, "%s\n", _T("TRANS. TYPE"));

		sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel);
		if( glProcInfo.stTranLog.ucTranType==VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
		{
			sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel);
			sprintf((char *)szBuff, "%s(%s)", _T(szTranName), _T("VOID"));
		}
		else if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
		{
			sprintf((char *)szBuff, "%s(%s)", _T(szTranName), _T("ADJ"));
		}
		else
		{
			sprintf((char *)szBuff, "%s", szTranName);
		}
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n", szBuff);

		// Batch, Invoice
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_SMALL, "%s", _T("BATCH NO."));
		MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_SMALL, "%s\n", _T("TRACE NO."));
        MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%06lu", glCurAcq.ulCurBatchNo);
        MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%06lu\n", glProcInfo.stTranLog.ulInvoiceNo);

		// Date, time
        MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_SMALL, "%s\n", _T("DATE/TIME"));

		Conv2EngTime(glProcInfo.stTranLog.szDateTime, szBuff);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%-s\n", szBuff);

		// REF, APPV
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_SMALL, "%s", _T("REF. NO."));
        MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_SMALL, "%s\n", _T("APP. CODE"));
        MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%-14.14s", glProcInfo.stTranLog.szRRN);
        MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%12.12s\n\n", glProcInfo.stTranLog.szAuthCode);

		if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
		{
			MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s:%.16s\n", _T("APP"), glProcInfo.stTranLog.szAppLabel);

			PubBcd2Asc0(glProcInfo.stTranLog.sAID, glProcInfo.stTranLog.ucAidLen, szBuff);
			PubTrimTailChars(szBuff, 'F');
			MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s:%.32s\n", _T("AID"), szBuff);

			PubBcd2Asc0(glProcInfo.stTranLog.sAppCrypto, 8, szBuff);
			MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %.16s\n", _T("T C"), szBuff);

#ifdef ENABLE_EMV
//#ifdef APP_DEBUG
            PubBcd2Asc0(glProcInfo.stTranLog.sTSI, 2, szBuff);
            MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   TSI: %s\n", szBuff);
            PubBcd2Asc0(glProcInfo.stTranLog.sTVR, 5, szBuff);
            MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   TVR: %s\n", szBuff);
//#endif
#endif
			if( glProcInfo.stTranLog.uiEntryMode & MODE_OFF_PIN )
			{
			    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %s\n", _T("PIN VERIFIED"));
			}
			else
			{
			    FtNextLine(); //MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n");
			}
		}

		PrnDescriptor();

		// amount
		PrnAmount((uchar *)"", TRUE);  //Print amount

		PrnAdditionalPrompt();

		PrnStatement();

		if( ucPrnFlag==PRN_REPRINT )
		{
		    MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "* %s *\n", _T("REPRINT"));
		}

		if( ucNum==0)
		{
		    if(HasE_Signature()){
		        unsigned char gSigToPrn[20000];
		        PrnBmp(glProcInfo.stTranLog.szSignPath, 0, 0, gSigToPrn);
		    }
		    else {
		        MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_SMALL, "%s\n", _T("CARDHOLDER SIGNATURE"));
                MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n\n\n\n");
		    }
		    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "-----------------------------\n");
		    MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_SMALL- 5, "%s\n", _T("I ACKNOWLEDGE SATISFACTORY RECEIPT OF RELATIVE  GOODS/SERVICE"));
		}

		if (ChkIfTrainMode())
		{
		    MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "***** %s *****\n", _T("DEMO"));
		}
		else
		{
#ifdef ENABLE_EMV
#ifdef EMV_TEST_VERSION
		    MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "** %s **\n", _T("FOR EMV TEST ONLY"));
#endif
#endif
		}

        PrnSetNormal();
		if( ucNum==0 )
		{
		    MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "**** %s ****\n", _T("MERCHANT COPY"));
		}
		else if( ucNum==1 )
		{
		    MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "**** %s ****\n", _T("CUSTOMER COPY"));
		}
		else if( ucNum==2 )
		{
		    MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "**** %s ****\n", _T("BANK COPY"));
		}
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n\n\n\n\n\n\n\n\n");

		StartPrinter();

		if( ucNum==0 && NumOfReceipt() != 1)
		{
            kbflush();

			Gui_ClearScr();
			Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, _T("PRESS ANY KEY"), gl_stCenterAttr, GUI_BUTTON_NONE, USER_OPER_TIMEOUT, NULL);
		}
	}

	return 0;
}

void PrnHead(uchar ucFreeFmat)
{
	uchar	szBuff[32];

	if (ChkIfTrainMode())
	{
	    MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "%s\n", _T("DEMONSTRATE ONLY\nNOT FOR PAYMENT PROOF"));
	} 
	else
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n", glSysParam.stEdcInfo.szMerchantName);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n", glSysParam.stEdcInfo.szMerchantAddr);
	}

	PrnStep(15);

	if (ucFreeFmat)
	{
		GetEngTime(szBuff);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s:%-15.15s", _T("MID"), glCurAcq.szMerchantID);
		MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%5.5s\n", szBuff+11);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s:%-8.8s", _T("TID"), glCurAcq.szTermID);
		MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%10.10s\n", szBuff);
	}
	else
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%14.8s\n%21.15s\n", glCurAcq.szTermID, glCurAcq.szMerchantID);
	}
	PrnStep(15);
}

int PrnCustomLogo_T(void)
{
	uchar	*psLogoData;

	// TODO even by email?
	if(ChkHardware(HWCFG_PRINTER, 0))
	    return 0;

	psLogoData = NULL;
	GetNowPrnLogo(&psLogoData);
	if (psLogoData!=NULL)
	{
		PrnLogo(psLogoData);
		PrnLeftIndent(0);
		PrnStep(PRN_SIZE_SMALL);
		return 0;
	}

	return -1;
}

void PrnHead_T(void)
{
	PrnStep(20);

	if (ChkIfTrainMode())
	{
	    MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "* %s *\n", _T("DEMONSTRATE ONLY"));
		PrnStep(10);
		
		MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "* %s *\n", _T("NOT FOR PAYMENT PROOF"));
		PrnStep(15);
	}
	else
	{
		MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "%s\n", glSysParam.stEdcInfo.szMerchantName);
		PrnStep(10);

		MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_SMALL, "%s\n", glSysParam.stEdcInfo.szMerchantAddr);

		FtNextLine(); //MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n");
		PrnStep(15);
	}

	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_SMALL, "%s", _T("MERCHANT ID."));
	MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%15.15s\n", glCurAcq.szMerchantID);

	MultiLngPrnStr(FT_ALIGN_LEFT,  PRN_SIZE_NORMAL, "%s", _T("TERMINAL ID.:"));
	MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%15.15s\n", glCurAcq.szTermID);

	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "=============================\n");
	PrnStep(15);
}

void PrnAmount(const uchar *pszIndent, uchar ucNeedSepLine)
{
	uchar	szBuff[50], szTotalAmt[12+1];
	uchar   szTempBuff[100];

    if (ChkIfNeedTip() && !(glProcInfo.stTranLog.uiStatus & TS_VOID))
	{
		//-------------------------------- BASE --------------------------------
		memset(szTempBuff, 0, sizeof(szTempBuff));
		App_ConvAmountTran(glProcInfo.stTranLog.szAmount, szBuff, 0);
		if (ChkIfThermalPrinter())
		{
		    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s%s", pszIndent, _T("BASE"));
		}
		else
		{
		    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s%s", pszIndent, "BASE");
		}
		MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%s\n", szBuff);

		//-------------------------------- TIPS --------------------------------
		memset(szTempBuff, 0, sizeof(szTempBuff));
		if (ChkIfThermalPrinter())
        {
            MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s%s", pszIndent, _T("TIPS"));
        }
        else
        {
            MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s%s", pszIndent, "TIPS");
        }
		if( !ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount) )
		{
			App_ConvAmountTran(glProcInfo.stTranLog.szTipAmount, szBuff, 0);
			MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%s\n", szBuff);
		}
		else
		{
		    FtNextLine(); //MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n");
		}

		//-------------------------------- TOTAL --------------------------------
		memset(szTempBuff, 0, sizeof(szTempBuff));
		if( ucNeedSepLine )
		{
		    MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "-----------------\n");
		}

		memset(szTempBuff, 0, sizeof(szTempBuff));
        if (ChkIfThermalPrinter())
        {
            MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s%s", pszIndent, _T("TOTAL"));
        }
        else
        {
            MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s%s", pszIndent , "TOTAL");
        }
		if( !ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount) )
		{
			PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
			App_ConvAmountTran(szTotalAmt, szBuff, 0);
		}
		else
		{
		    App_ConvAmountTran(glProcInfo.stTranLog.szAmount, szBuff, 0);
		}
		MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%s\n", szBuff);

		if( ucNeedSepLine )
		{
			MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "=================\n");
		}
	}
	else
	{
		App_ConvAmountTran(glProcInfo.stTranLog.szAmount, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));

		memset(szTempBuff, 0, sizeof(szTempBuff));
		if (ChkIfThermalPrinter())
		{
		    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s%s", pszIndent, _T("TOTAL"));
		}
		else
		{
		    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s%s", pszIndent, "TOTAL");
		}
		MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%s\n", szBuff);

		if( ucNeedSepLine )
		{
			MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "=================\n");
		}
	}
}

// print product descriptor
void PrnDescriptor(void)
{
	uchar	ucCnt, ucMaxNum, ucTemp;
	uchar	szBuf[50];

	if( ChkIfDccAcquirer() )
	{
		return;
	}

	ucMaxNum = (uchar)MIN(MAX_GET_DESC, glProcInfo.stTranLog.ucDescTotal);
	for(ucCnt=0; ucCnt<ucMaxNum; ucCnt++)
	{
		ucTemp = glProcInfo.stTranLog.szDescriptor[ucCnt] - '0';
		PubASSERT( ucTemp<MAX_DESCRIPTOR );
		memset(szBuf, 0, sizeof(szBuf));
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %-20.20s\n", glSysParam.stDescList[ucTemp].szText);
	}

	if (!ChkIfThermalPrinter())
	{
		if( (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) || ChkEdcOption(EDC_FREE_PRINT) )
		{
		    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n");
		}
		else
		{
			for(; ucCnt<MAX_GET_DESC; ucCnt++)
			{
			    FtNextLine(); //MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n");
			}
		}
	}
}

void PrnAdditionalPrompt(void)
{
	if( !ChkAcqOption(ACQ_ADDTIONAL_PROMPT) && !ChkAcqOption(ACQ_AIR_TICKET) )
	{
		return;
	}

	MultiLngPrnStr(FT_ALIGN_LEFT,  PRN_SIZE_NORMAL, "%-14.14s",  glSysParam.stEdcInfo.szAddlPrompt);
	MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%16.16s\n", glProcInfo.stTranLog.szAddlPrompt);
}

void PrnStatement(void)
{
	if( glProcInfo.stTranLog.ucTranType==INSTALMENT )
	{
		if( ChkIfBea() )
		{
		    MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "%s\n", _T("I ACCEPT THE T&C OVERLEAF"));
		}
	}
}

// print total information of ucIssuerKey
int PrnTotalIssuer(uchar ucIssuerKey)
{
	uchar	ucIndex, szBuff[20];

	for(ucIndex=0; ucIndex<glSysParam.ucIssuerNum; ucIndex++)
	{
		if( glSysParam.stIssuerList[ucIndex].ucKey==ucIssuerKey )
		{
			break;
		}
	}
	memcpy(&glPrnTotal, &glIssuerTotal[ucIndex], sizeof(TOTAL_INFO));

	if( ChkIfZeroTotal(&glPrnTotal) )
	{
		return 0;
	}
	
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "---------------------------\n");
	ConvIssuerName(glSysParam.stIssuerList[ucIndex].szName, szBuff);
	if( ChkIfThermalPrinter() )
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s  : %-10.10s\n", _T("ISSUER"), szBuff);
	}
	else
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "  %s  : %-10.10s\n", _T("ISSUER"), szBuff);
	}

	PrnTotalInfo(&glPrnTotal);

	return 0;
}

// print total information
void PrnTotalInfo(const void *pstInfo)
{
	uchar		szBuff[50], szBaseAmt[20];
	TOTAL_INFO	*pstTotal;

	pstTotal = (TOTAL_INFO *)pstInfo;
	PubAscSub(pstTotal->szSaleAmt, pstTotal->szTipAmt, 12, szBaseAmt);

	App_ConvAmountTran(szBaseAmt, szBuff, 0);
	MultiLngPrnStr(FT_ALIGN_LEFT,  PRN_SIZE_NORMAL, "%s :%-03d", _T("BASE"), pstTotal->uiSaleCnt);
	MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%s\n", szBuff);

	App_ConvAmountTran(pstTotal->szTipAmt, szBuff, 0);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s :%-03d", _T("TIPS"), pstTotal->uiTipCnt);
	MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%s\n", szBuff);

	App_ConvAmountTran(pstTotal->szSaleAmt, szBuff, 0);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s :%-03d", _T("SALES"), pstTotal->uiSaleCnt);
	MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%s\n", szBuff);

	App_ConvAmountTran(pstTotal->szRefundAmt, szBuff, GA_NEGATIVE);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s :%-03d", _T("REFUND"), pstTotal->uiRefundCnt);
	MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%s\n", szBuff);

	App_ConvAmountTran(pstTotal->szVoidSaleAmt, szBuff, GA_NEGATIVE);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s :%-03d", _T("VOID SALES"), pstTotal->uiVoidSaleCnt);
	MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%s\n", szBuff);

	App_ConvAmountTran(pstTotal->szVoidRefundAmt, szBuff, 0);
	MultiLngPrnStr(FT_ALIGN_LEFT,  PRN_SIZE_NORMAL, "%s :%-03d", _T("VOID REFUND"), pstTotal->uiVoidRefundCnt);
	MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%s\n\n", szBuff);
}

int PrnTotalAcq(void)
{
	uchar	ucCnt;
	int		iRet;

	PrnInit();

	PrnStep(30);
	MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "%s\n",  _T("TRANS TOTALS BY CARD"));
	PrnEngTime();
	PrnStep(20);
	if( ChkIfThermalPrinter() )
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s\n%s: %s\n", _T("TID"), glCurAcq.szTermID, _T("MID"), glCurAcq.szMerchantID);
		PrnStep(15);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %-10.10s\n", _T("ACQUIRER"), glCurAcq.szName);
	}
	else
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%14.8s\n%21.15s\n", glCurAcq.szTermID, glCurAcq.szMerchantID);
		PrnStep(15);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "  %s: %-10.10s\n", _T("ACQUIRER"), glCurAcq.szName);
	}

	for(ucCnt=0; ucCnt<glSysParam.ucIssuerNum; ucCnt++)
	{
		if( glCurAcq.sIssuerKey[ucCnt]!=INV_ISSUER_KEY )
		{
			PrnTotalIssuer(glCurAcq.sIssuerKey[ucCnt]);
		}
		if( (ucCnt%5)==4 )
		{
			iRet = StartPrinter();
			if( iRet!=0 )
			{
				return iRet;
			}
			PrnInit();
			PrnSetNormal();
		}
	}
	MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "%s\n\n\n\n\n", _T("END  OF  TOTAL"));

    if(!ChkHardware(HWCFG_PRINTER, 0))
        MultiLngPrnStr(FT_ALIGN_LEFT,   PRN_SIZE_NORMAL, "\n\n\n\n");

	StartPrinter();
	return 0;
}

int PrintSettle(uchar ucPrnFlag)
{
	ulong	ulInvoice;

	if( ucPrnFlag==PRN_NORMAL )
	{
		// save settle information for reprint
		glSysCtrl.stRePrnStlInfo.bValid[glCurAcq.ucIndex]    = TRUE;
		glSysCtrl.stRePrnStlInfo.ulSOC[glCurAcq.ucIndex]     = glSysCtrl.ulInvoiceNo;
		glSysCtrl.stRePrnStlInfo.ulBatchNo[glCurAcq.ucIndex] = glCurAcq.ulCurBatchNo;
		sprintf((char *)glSysCtrl.stRePrnStlInfo.szSettleMsg[glCurAcq.ucIndex], "%s", glProcInfo.szSettleMsg);
		memcpy(&glSysCtrl.stRePrnStlInfo.stAcqTotal[glCurAcq.ucIndex], &glAcqTotal[glCurAcq.ucIndex], sizeof(TOTAL_INFO));
		memcpy(glSysCtrl.stRePrnStlInfo.stIssTotal[glCurAcq.ucIndex], glIssuerTotal, sizeof(glIssuerTotal));
		SaveRePrnStlInfo();
	}

	PrnInit();

	if( ChkIfThermalPrinter() )
	{
		PrnHead_T();
	}
	else
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n\n\n\n");
		PrnHead(FALSE);
	}
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "************************\n");
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n", _T("BUSINESS DATE - TIME"));
	PrnEngTime();
	PrnStep(7);
	MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "%s\n\n", _T("SETTLEMENT"));
	MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "*** %s ***\n", _T("SUMMARY OF CHARGE"));
	MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "* %s *\n\n", _T("CLOSE BATCH CONFIRMED"));
	ulInvoice = (ucPrnFlag==PRN_NORMAL) ? glSysCtrl.ulInvoiceNo : glSysCtrl.stRePrnStlInfo.ulSOC[glCurAcq.ucIndex];
	if( ChkIfAmex() )
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %06lu\n", _T("S.O.C. NO."), ulInvoice);
	}
	else
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %06lu\n", _T("TRACE  NO."), ulInvoice);
	}
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %06lu\n", _T("BATCH  NO."), glCurAcq.ulCurBatchNo);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n", glProcInfo.szSettleMsg);

	PrnTotalInfo(&glTransTotal);

	if( ucPrnFlag==PRN_REPRINT )
	{
	    MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "\n%s", _T("REPRINT"));
	}

    if(!ChkHardware(HWCFG_PRINTER, 0))
        MultiLngPrnStr(FT_ALIGN_LEFT,   PRN_SIZE_NORMAL, "\n\n\n\n");

	return StartPrinter();
}

void PrnEngTime(void)
{
	uchar	szDateTime[14+1], szBuff[30];

	GetDateTime(szDateTime);
	Conv2EngTime(szDateTime, szBuff);
	MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "%s\n", szBuff);
}

// 打印终端参数
// Print parameter
int PrintParam(void)
{
	uchar	ucCnt;

	SetCurrTitle(_T("PRINT PARAMETER"));
	if( PasswordTerm()!=0 )
	{
		return ERR_NO_DISP;
	}

	SetOffBase(OffBaseCheckPrint);

	Gui_ClearScr();
	// Modified by Kim_LinHB 2014-8-11 v1.01.0003
	Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

	PrnInit();
	PrnSetNormal();

	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n\n%s\n", glSysParam.stEdcInfo.szMerchantName);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n",     glSysParam.stEdcInfo.szMerchantAddr);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %.8s\n", _T("INIT TID"), glSysParam.stEdcInfo.szDownLoadTID);
	PrnHexString("EDC OPTION:", glSysParam.stEdcInfo.sOption, 5, TRUE);

	PrnEngTime();
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s\n", _T("APP VERSION"), EDC_VER_PUB);

	if (glSysParam.ucDescNum)
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s:\n", _T("DESCRIPTION"));
		for(ucCnt=0; ucCnt<glSysParam.ucDescNum; ucCnt++)
		{
		    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "   %.2s:%.20s\n", glSysParam.stDescList[ucCnt].szCode,
					glSysParam.stDescList[ucCnt].szText);
		}
	}

	PrnInstalmentPara();

	for(ucCnt=0; ucCnt<glSysParam.ucAcqNum; ucCnt++)
	{
		if( PrnParaAcq(ucCnt)==0 )
		{
			if( StartPrinter()!=0 )
			{
				return ERR_NO_DISP;
			}
			PrnInit();
			//PrnStep(20);
		}
	}

    if(!ChkHardware(HWCFG_PRINTER, 0))
        MultiLngPrnStr(FT_ALIGN_LEFT,   PRN_SIZE_NORMAL, "\n\n\n\n");

	if( StartPrinter()!=0 )
	{
		return ERR_NO_DISP;
	}

	if (!ChkIfEmvEnable())
	{
		return 0;
	}

#ifdef ENABLE_EMV
	Gui_ClearScr();
	if(GUI_OK == Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, _T("PRN EMV PARA ?"), gl_stCenterAttr, GUI_BUTTON_YandN, USER_OPER_TIMEOUT, NULL)){
		PrnEmvPara();
	}
#endif
	return 0;
}

int PrnParaAcq(uchar ucAcqIndex)
{
	PubASSERT(glSysParam.stAcqList[ucAcqIndex].ucKey!=INV_ACQ_KEY);
	if( glSysParam.stAcqList[ucAcqIndex].ucKey==INV_ACQ_KEY )
	{
		return 1;
	}

	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n========================\n");
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%.3s  %.10s\n", glSysParam.stAcqList[ucAcqIndex].szNii,
			glSysParam.stAcqList[ucAcqIndex].szName);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %.8s\n", _T("TID"), glSysParam.stAcqList[ucAcqIndex].szTermID);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %.15s\n", _T("MID"), glSysParam.stAcqList[ucAcqIndex].szMerchantID);
	PrnHexString("ACQ OPTION:", glSysParam.stAcqList[ucAcqIndex].sOption, 4, TRUE);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s %.24s\n", _T("TXN TEL1"), glSysParam.stAcqList[ucAcqIndex].TxnTelNo1);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s %.24s\n", _T("TXN TEL2"), glSysParam.stAcqList[ucAcqIndex].TxnTelNo2);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s %.24s\n", _T("SET TEL1"), glSysParam.stAcqList[ucAcqIndex].StlTelNo1);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s %.24s\n", _T("SET TEL2"), glSysParam.stAcqList[ucAcqIndex].StlTelNo2);

	//tcp/ip
	if (strlen(glSysParam.stAcqList[ucAcqIndex].stTxnTCPIPInfo[0].szIP) )
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s:\n", _T("TCP/IP PARAM"));
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s:%.5s\n",  _T("TXN 1"), glSysParam.stAcqList[ucAcqIndex].stTxnTCPIPInfo[0].szIP,
	                                                glSysParam.stAcqList[ucAcqIndex].stTxnTCPIPInfo[0].szPort);
	}
    if (strlen(glSysParam.stAcqList[ucAcqIndex].stTxnTCPIPInfo[1].szIP) )
	{
        MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s:%.5s\n",  _T("TXN 2"), glSysParam.stAcqList[ucAcqIndex].stTxnTCPIPInfo[1].szIP,
                                                glSysParam.stAcqList[ucAcqIndex].stTxnTCPIPInfo[1].szPort);
	}

    if (strlen(glSysParam.stAcqList[ucAcqIndex].stTxnGPRSInfo[0].szIP) )
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s:\n", _T("GPRS PARAM"));
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s:%.5s\n",  _T("TXN 1"), glSysParam.stAcqList[ucAcqIndex].stTxnGPRSInfo[0].szIP,
                                                       glSysParam.stAcqList[ucAcqIndex].stTxnGPRSInfo[0].szPort);
	}
    if (strlen(glSysParam.stAcqList[ucAcqIndex].stTxnGPRSInfo[1].szIP) )
	{
        MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s:%.5s\n",  _T("TXN 2"), glSysParam.stAcqList[ucAcqIndex].stTxnGPRSInfo[1].szIP,
                                                       glSysParam.stAcqList[ucAcqIndex].stTxnGPRSInfo[0].szPort);
	}

	PrnParaIssuer(ucAcqIndex);

	return 0;
}

void PrnParaIssuer(uchar ucAcqIndex)
{
	uchar	ucCnt;

	for(ucCnt=0; ucCnt<glSysParam.ucIssuerNum; ucCnt++)
	{
		if( glSysParam.stAcqList[ucAcqIndex].sIssuerKey[ucCnt]!=INV_ISSUER_KEY )
		{
			PrnParaIssuerSub(glSysParam.stAcqList[ucAcqIndex].sIssuerKey[ucCnt]);
		}
	}
}

void PrnParaIssuerSub(uchar ucIssuerKey)
{
	uchar	ucCnt;

	if( ucIssuerKey==INV_ISSUER_KEY )
	{
		return;
	}

	for(ucCnt=0; ucCnt<glSysParam.ucIssuerNum; ucCnt++)
	{
		if( glSysParam.stIssuerList[ucCnt].ucKey==ucIssuerKey )
		{
			break;
		}
	}
	PubASSERT( ucCnt<=glSysParam.ucIssuerNum ); //bug 611
	if( ucCnt>glSysParam.ucIssuerNum )
	{
		return;
	}

	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\n---%s: %-10.10s---\n", _T("ISSUER"), glSysParam.stIssuerList[ucCnt].szName);
	PrnHexString("ISSUER OPTION:", glSysParam.stIssuerList[ucCnt].sOption, 4, TRUE);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %02x %02x %02x\n", _T("PAN MASK"),
			(uchar)(0xFF^glSysParam.stIssuerList[ucCnt].sPanMask[0]),
			(uchar)(0xFF^glSysParam.stIssuerList[ucCnt].sPanMask[1]),
			(uchar)(0xFF^glSysParam.stIssuerList[ucCnt].sPanMask[2]));

	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %lu\n", _T("FLOOR LIMIT"), glSysParam.stIssuerList[ucCnt].ulFloorLimit);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %d%%\n", _T("ADJUSTMENT PERCENT"), glSysParam.stIssuerList[ucCnt].ucAdjustPercent);

	PrnIssuerOption(glSysParam.stIssuerList[ucCnt].sOption);
	PrnCardTable(ucIssuerKey);
}

void PrnIssuerOption(const uchar *psOption)
{
	static	OPTION_INFO	stIssuerOptList[] =
	{
// 		{"ENABLE BALANCE?",		ISSUER_EN_BALANCE},
		{"ENABLE ADJUST",		ISSUER_EN_ADJUST,			FALSE,	PM_MEDIUM},
		{"ENABLE OFFLINE",		ISSUER_EN_OFFLINE,			FALSE,	PM_MEDIUM},
		{"EN. (PRE)AUTH",		ISSUER_NO_PREAUTH,			TRUE,	PM_MEDIUM},
		{"EN. REFUND",			ISSUER_NO_REFUND,			TRUE,	PM_MEDIUM},
		{"EN. VOID",			ISSUER_NO_VOID,				TRUE,	PM_MEDIUM},
// 		{"ENABLE EXPIRY",		ISSUER_EN_EXPIRY,			FALSE,	PM_MEDIUM},
// 		{"CHECK EXPIRY",		ISSUER_CHECK_EXPIRY,		FALSE,	PM_MEDIUM},
// 		{"CHKEXP OFFLINE",		ISSUER_CHECK_EXPIRY_OFFLINE,FALSE,	PM_MEDIUM},
// 		{"CHECK PAN",			ISSUER_CHKPAN_MOD10,		FALSE,	PM_MEDIUM},
// 		{"EN DISCRIPTOR",		ISSUER_EN_DISCRIPTOR,		FALSE,	PM_MEDIUM},
		{"ENABLE MANUAL",		ISSUER_EN_MANUAL,			FALSE,	PM_MEDIUM},
// 		{"ENABLE PRINT",		ISSUER_EN_PRINT,			FALSE,	PM_MEDIUM},
		{"VOICE REFERRAL",		ISSUER_EN_VOICE_REFERRAL,	FALSE,	PM_MEDIUM},
// 		{"PIN REQUIRED",		ISSUER_EN_PIN,				FALSE,	PM_MEDIUM},
// 		{"ACCOUNT SELECT",		ISSUER_EN_ACCOUNT_SELECTION,FALSE,	PM_MEDIUM},
// 		{"ROC INPUT REQ",		ISSUER_ROC_INPUT_REQ,		FALSE,	PM_MEDIUM},
// 		{"DISP AUTH CODE",		ISSUER_AUTH_CODE,			FALSE,	PM_MEDIUM},
// 		{"ADDTIONAL DATA",		ISSUER_ADDTIONAL_DATA,		FALSE,	PM_MEDIUM},
		{"SECURITY CODE",		ISSUER_SECURITY_SWIPE,		FALSE,	PM_MEDIUM},
		{"SECU. CODE MANUL",	ISSUER_SECURITY_MANUL,		FALSE,	PM_MEDIUM},
		{NULL, 0, FALSE, PM_MEDIUM},
	};
	uchar	ucCnt;

	for(ucCnt=0; stIssuerOptList[ucCnt].pText!=NULL; ucCnt++)
	{
	    MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "    %s", stIssuerOptList[ucCnt].pText);
		if( (!stIssuerOptList[ucCnt].ucInverseLogic && ChkOptionExt(psOption, stIssuerOptList[ucCnt].uiOptVal)) ||
			(stIssuerOptList[ucCnt].ucInverseLogic && !ChkOptionExt(psOption, stIssuerOptList[ucCnt].uiOptVal)) )
		{
		    MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "[ on]\n");
		}
		else
		{
		    MultiLngPrnStr(FT_ALIGN_RIGHT, PRN_SIZE_NORMAL, "[off]\n");
		}
	}
}

void PrnCardTable(uchar ucIssuerKey)
{
	uchar	ucCnt, szBuff[30];

	if( ucIssuerKey==INV_ISSUER_KEY )
	{
		return;
	}

	for(ucCnt=0; ucCnt<glSysParam.ucCardNum; ucCnt++)
	{
		if( glSysParam.stCardTable[ucCnt].ucIssuerKey==ucIssuerKey )
		{
			PubBcd2Asc0(glSysParam.stCardTable[ucCnt].stCardRange[0].sPanRangeLow, 5, szBuff);
			szBuff[10] = '~';
			PubBcd2Asc0(glSysParam.stCardTable[ucCnt].stCardRange[0].sPanRangeHigh, 5, &szBuff[11]);
			MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n", szBuff);
			if ( strlen(glSysParam.stCardTable[ucCnt].stCardRange[1].sPanRangeLow) != 0 )
			{
				memset(szBuff, 0, sizeof(szBuff));
				PubBcd2Asc0(glSysParam.stCardTable[ucCnt].stCardRange[1].sPanRangeLow, 5, szBuff);
				szBuff[10] = '~';
				PubBcd2Asc0(glSysParam.stCardTable[ucCnt].stCardRange[1].sPanRangeHigh, 5, &szBuff[11]);
				MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n", szBuff);
			}
		}
	}
}

int PrnInstalmentPara(void)
{
	uchar	ucCnt, szBuff[20], szBuff1[50], ucAcqIndex;

	if( !ChkIfInstalmentPara() )
	{
		return 1;
	}

	MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "\n======== %s =======\n", _T("INSTALMENT"));
	for(ucCnt=0; ucCnt<glSysParam.ucPlanNum; ucCnt++)
	{
		PubASSERT( glSysParam.stPlanList[ucCnt].ucIndex!=0xFF );
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%02d.%-7.7s - %dmths\n", ucCnt+1, glSysParam.stPlanList[ucCnt].szName,
				glSysParam.stPlanList[ucCnt].ucMonths);

		sprintf((char *)szBuff, "%lu", glSysParam.stPlanList[ucCnt].ulBottomAmt);
		App_ConvAmountLocal(szBuff, szBuff1, 0);
		ucAcqIndex = glSysParam.stPlanList[ucCnt].ucAcqIndex;
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%-10.10s  %s\n", glSysParam.stAcqList[ucAcqIndex].szName, szBuff1);
	}

	return 0;
}

#ifdef ENABLE_EMV
// 打印EMV参数
// Print EMV parameter
int PrnEmvPara(void)
{
	int			iRet, iCnt;
	EMV_APPLIST	stEmvApp;
	EMV_CAPK	stEmvCapk;

	PrnInit();
	PrnSetNormal();

	MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "\n========= %s =======\n", _T("EMV PARAMETER"));
	EMVGetParameter(&glEmvParam);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %02X\n", _T("TERMINAL TYPE"), glEmvParam.TerminalType);
	PrnHexString("TERMINAL CAPA:",  glEmvParam.Capability, 3, TRUE);
	PrnHexString("TERM EX-CAPA :",  glEmvParam.ExCapability, 5, TRUE);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %02X\n", _T("TXN CURR EXP "), glEmvParam.TransCurrExp);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %02X\n", _T("REF CURR EXP "), glEmvParam.ReferCurrExp);
	PrnHexString("REF CURR CODE:", glEmvParam.ReferCurrCode, 2, TRUE);
	PrnHexString("COUNTRY CODE :", glEmvParam.CountryCode, 2, TRUE);
	PrnHexString("TXN CURR CODE:", glEmvParam.TransCurrCode, 2, TRUE);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %ld\n", _T("REF CURR CON "), glEmvParam.ReferCurrCon);
	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s\n",  _T("SELECT PSE"), glEmvParam.SurportPSESel ? _T("YES") : _T("NO")); //bug 611
	MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "\n\n\n======== %s =========\n", _T("EMV APP LIST"));

	for(iCnt=0; iCnt<MAX_APP_NUM; iCnt++)
	{
		memset(&stEmvApp, 0, sizeof(EMV_APPLIST));
		iRet = ReadAIDFile(iCnt, &stEmvApp);
		if( iRet!=EMV_OK )
		{
			continue;
		}
		PrnHexString("AID:",  stEmvApp.AID, (int)stEmvApp.AidLen, TRUE);
		PrnHexString("VERSION:",  stEmvApp.Version, 2, TRUE);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s MATCH\n", _T("PARTIAL"));
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %d\n", _T("PRIORITY      "), stEmvApp.Priority);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %d\n", _T("TARGET PER    "), stEmvApp.TargetPer);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %d\n", _T("MAX TARGET PER"), stEmvApp.MaxTargetPer);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s\n", _T("NO"));
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s\n", _T("NO"));
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %s\n", _T("NO"));
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %lu\n", _T("FLOOR LIMIT   "), stEmvApp.FloorLimit);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %lu\n", _T("THRESHOLD     "), stEmvApp.Threshold);
		PrnHexString("TAC DENIAL :",  stEmvApp.TACDenial,  5, TRUE);
		PrnHexString("TAC ONLINE :",  stEmvApp.TACOnline,  5, TRUE);
		PrnHexString("TAC DEFAULT:",  stEmvApp.TACDefault, 5, TRUE);
		MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "-----------------------------\n");
		if( (iCnt%5)==0 )
		{
			if( StartPrinter()!=0 )
			{
				return 1;
			}
			PrnInit();
		}
	}

	MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "\n\n========= %s ========\n", _T("EMV CAPK LIST"));
	for(iCnt=0; iCnt<MAX_KEY_NUM; iCnt++)
	{
		memset(&stEmvCapk, 0, sizeof(EMV_CAPK));
		iRet = ReadCAPKFile(iCnt, &stEmvCapk);
		if( iRet!=EMV_OK )
		{
			continue;
		}
		PrnHexString("RID:",  stEmvCapk.RID, 5, TRUE);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, " %s: %02X\n", _T("ID"), stEmvCapk.KeyID);
// 		PrnStr("HASH   : %02X\n",  stEmvCapk.HashInd);
// 		PrnStr("ARITH  : %02X\n",  stEmvCapk.ArithInd);
		PrnHexString("EXP DATE:",  stEmvCapk.ExpDate, 3, TRUE);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s: %d\n",  _T("MOD LEN"), (int)(8 * stEmvCapk.ModulLen));
		PrnHexString("EXPONENT:",  stEmvCapk.Exponent, (int)stEmvCapk.ExponentLen, TRUE);
		MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "-----------------------------\n");
		if( (iCnt%5)==0 )
		{
// 			PrnStr("\f");
			if( StartPrinter()!=0 )
			{
				return 1;
			}
			PrnInit();
		}
	}

    if(!ChkHardware(HWCFG_PRINTER, 0))
        MultiLngPrnStr(FT_ALIGN_LEFT,   PRN_SIZE_NORMAL, "\n\n\n\n");

	return StartPrinter();
}

void PrintEmvErrLogSub(void)
{
	ushort			uiCnt, uiActNum, uiTemp;
	uchar			szBuff[50];
	EMV_ERR_LOG		stErrLog;

	PrnInit();
	PrnSetNormal();
	MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "%s\n\n", _T("EMV ERROR LOG"));

	for(uiActNum=uiCnt=0; uiCnt<MAX_ERR_LOG; uiCnt++)
	{
		memset(&stErrLog, 0, sizeof(EMV_ERR_LOG));
		LoadErrLog(uiCnt, &stErrLog);
		if( !stErrLog.bValid )
		{
			continue;
		}

		uiActNum++;
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "\nSTAN: %06lu\n", stErrLog.ulSTAN);
		PubBcd2Asc0(stErrLog.sAID, stErrLog.ucAidLen, szBuff);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "AID: %s\n", szBuff);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "PAN: %s\n", stErrLog.szPAN);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "PAN SEQ #: %02X\n", stErrLog.ucPANSeqNo);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "AMT: %.12s\n", stErrLog.szAmount);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "TIP: %.12s\n", stErrLog.szTipAmt);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "RSP: %.2s\n",  stErrLog.szRspCode);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "RRN: %.12s\n", stErrLog.szRRN);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "AUT: %.6s\n",  stErrLog.szAuthCode);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "TVR: %02X %02X %02X %02X %02X\n", stErrLog.sTVR[0], stErrLog.sTVR[1],
					   stErrLog.sTVR[2], stErrLog.sTVR[3], stErrLog.sTVR[4]);
		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "TSI: %02X %02X\n", stErrLog.sTSI[0], stErrLog.sTSI[1]);

		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "REQ BIT 55:\n");
		for(uiTemp=0; uiTemp<stErrLog.uiReqICCDataLen; uiTemp++)
		{
			MultiLngPrnStr(FT_ALIGN_NONE, PRN_SIZE_NORMAL, "%02X %s", stErrLog.sReqICCData[uiTemp], (uiTemp%8)==7 ? "\n" : "");
		}
		if(uiTemp>0)
		{
			MultiLngPrnStr(FT_ALIGN_NONE, PRN_SIZE_NORMAL, "\n");
		}

		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "REQ BIT 56:\n");
		for(uiTemp=0; uiTemp<stErrLog.uiReqField56Len; uiTemp++)
		{
			MultiLngPrnStr(FT_ALIGN_NONE, PRN_SIZE_NORMAL, "%02X %s", stErrLog.sReqField56[uiTemp], (uiTemp%8)==7 ? "\n" : "");
		}
		if(uiTemp>0)
		{
			MultiLngPrnStr(FT_ALIGN_NONE, PRN_SIZE_NORMAL, "\n");
		}

		MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "RSP BIT 55:\n");
		for(uiTemp=0; uiTemp<stErrLog.uiRspICCDataLen; uiTemp++)
		{
			MultiLngPrnStr(FT_ALIGN_NONE, PRN_SIZE_NORMAL, "%02X %s", stErrLog.sRspICCData[uiTemp], (uiTemp%8)==7 ? "\n" : "");
		}
		if(uiTemp>0)
		{
			MultiLngPrnStr(FT_ALIGN_NONE, PRN_SIZE_NORMAL, "\n");
		}

		if( (uiActNum%5)==4 )
		{
			if( StartPrinter()!=0 )
			{
				return;
			}

			PrnInit();
			PrnSetNormal();
		}
	}

	if (uiActNum>0)
	{
		MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "%s", (ChkIfThermalPrinter() ? "\n" : "\f"));
	}
	else
	{
		MultiLngPrnStr(FT_ALIGN_CENTER, PRN_SIZE_NORMAL, "\n  ( NO RECORD )");
	}

    if(!ChkHardware(HWCFG_PRINTER, 0))
        MultiLngPrnStr(FT_ALIGN_LEFT,   PRN_SIZE_NORMAL, "\n\n\n\n");

	StartPrinter();
}
#endif


void PrnHexString(const char *pszTitle, const uchar *psHexStr, int iLen, uchar bNewLine)
{
	int		iCnt;

	MultiLngPrnStr(FT_ALIGN_LEFT, PRN_SIZE_NORMAL, "%s\n", pszTitle);

	FtFontSize(PRN_SIZE_NORMAL);
	for(iCnt=0; iCnt<iLen; iCnt++)
	{
	    if(iCnt % 8 == 0){
	        FtFontAlign(FT_ALIGN_LEFT);
	    }
	    else{
	        FtFontAlign(FT_ALIGN_NONE);
	    }
		FtPrnStr(" %02X", psHexStr[iCnt]);
		if(iCnt % 8 == 7){
		    FtNextLine();
		}
	}
	if (bNewLine)
	{
	    FtNextLine();
	}
}

// 打印错误提示
// Start-up printer, and show error if any.
// Modified by Kim_LinHB 2014-6-8
int StartPrinter(void)
{
	int	iRet, iRet2;

    while (1)
    {
        Gui_SB_ManuallyRefresh(GUI_SB_ICON_PRINTER, OPENICON_PRINTER);
        DispPrinting();
        PrintOne();

        if(ChkHardware(HWCFG_PRINTER, 0))
        {
            iRet = RET_OK;
            break;
        }
        else
        {
            iRet = OsPrnStart();
            if (iRet == RET_OK)
            {
                break;
            }
        }

        iRet2 = DispPrnError(iRet);
        if (iRet != ERR_PRN_PAPEROUT) //bug 807
        {
            break;
        }

        if (GUI_ERR_USERCANCELLED == iRet2 || GUI_ERR_TIMEOUT == iRet2)
        {
            Gui_ClearScr();
            Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, NULL, _T("PLEASE REPRINT"), gl_stCenterAttr, GUI_BUTTON_OK, 2, NULL );
            break;
        }
    }
    Gui_SB_ManuallyRefresh(GUI_SB_ICON_PRINTER, CLOSEICON_PRINTER);
    if(ChkHardware(HWCFG_PRINTER, 0))
    {
//        static char sg_recvBuff[512];
//        int iRest = strlen(sg_SMS_Buffer);
//        int iCnt = 0;
//        while(1){
//            int ret = WlPortOpen("115200,8,n,1");
//            printf("OsWlPortOpen, ret=%d\n", ret);
//            if(ret < 0)
//                return -1;
//
//            WlPortReset();
//
//            WlPortSend("AT\r", 3);
//            memset(sg_recvBuff, 0x00, sizeof(sg_recvBuff));
//            ret = WlPortRecv(sg_recvBuff, sizeof(sg_recvBuff), 500);
//            OsLog(LOG_ERROR, "recv(%d):\n", ret);
//            OsLog(LOG_ERROR, "\n%s", sg_recvBuff);
//
//            WlPortSend("AT+CMGF=1\r", 10);
//            memset(sg_recvBuff, 0x00, sizeof(sg_recvBuff));
//            ret = WlPortRecv(sg_recvBuff, sizeof(sg_recvBuff), 500);
//            OsLog(LOG_ERROR, "recv(%d):\n", ret);
//            OsLog(LOG_ERROR, "\n%s", sg_recvBuff);
//
//            memset(sg_recvBuff, 0, sizeof(sg_recvBuff));
//            sprintf(sg_recvBuff, "AT+CMGS=%s\r", glCurAcq.szLastCustomerPhoneNo);
//            OsLog(LOG_ERROR, "%s-%d size%d", __FILE__, __LINE__, strlen(sg_recvBuff));
//            WlPortSend(sg_recvBuff, strlen(sg_recvBuff) + 1);
//            memset(sg_recvBuff, 0x00, sizeof(sg_recvBuff));
//            ret = WlPortRecv(sg_recvBuff, sizeof(sg_recvBuff), 500);
//            OsLog(LOG_ERROR, "recv(%d):\n", ret);
//            OsLog(LOG_ERROR, "\n%s", sg_recvBuff);
//            OsSleep(500);
//            memset(sg_recvBuff, 0x00, sizeof(sg_recvBuff));
//
//            if(iRest > 120)
//            {
//                OsLog(LOG_ERROR, "%s-%d %120s", __FILE__, __LINE__, sg_SMS_Buffer+iCnt * 120);
//
//                WlPortSend(sg_SMS_Buffer+iCnt * 120, 120);
//                WlPortSend("\x1A", 1);
//                OsSleep(500);
//                WlClosePort();
//                ++iCnt;
//                iRest -= 120;
//            }
//            else{
//                OsLog(LOG_ERROR, "%s-%d %s", __FILE__, __LINE__, sg_SMS_Buffer+iCnt * 120);
//
//                WlPortSend(sg_SMS_Buffer+iCnt * 120, iRest);
//                WlPortSend("\x1A", 1);
//                OsSleep(500);
//                WlClosePort();
//                bOpenPrinter = FALSE;
//                memset(sg_SMS_Buffer, 0, sizeof(sg_SMS_Buffer));
//                break;
//            }
//        }

//        XuiImg *img = XuiImgLoadFromFile("./data/lastreceipt.bmp");
//        XuiImgSaveToFile(img, "./data/lastreceipt.png");
//        XuiImgFree(img);
    }
    if(iRet == RET_OK)
        return 0;
	return ERR_NO_DISP;
}

// Modified by Kim_LinHB 2014-6-8
int DispPrnError(int iErrCode)
{
	unsigned char szBuff[100];
	Gui_ClearScr();
	PubBeepErr();
	switch( iErrCode )
	{
	case ERR_PRN_BUSY:
		strcpy(szBuff, _T("PRINTER BUSY"));
		break;

	case ERR_PRN_PAPEROUT:
		strcpy(szBuff, _T("OUT OF PAPER"));
		return Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_WARNING, szBuff, gl_stCenterAttr, GUI_BUTTON_YandN, USER_OPER_TIMEOUT, NULL);
		break;

	case ERR_PRN_WRONG_PACKAGE:
		strcpy(szBuff, _T("PRN DATA ERROR"));
		break;

	case ERR_PRN_OVERHEAT:
		strcpy(szBuff, _T("PRINTER OVERHEAT"));
		break;

	case ERR_PRN_OUTOFMEMORY:
		strcpy(szBuff, _T("PRN OVERFLOW"));
		break;

	default:
		strcpy(szBuff, _T("PRINT FAILED"));
		break;
	}
	return Gui_ShowMsgBox(GetCurrTitle(), gl_stTitleAttr, _ICON_FAIL, szBuff, gl_stCenterAttr, GUI_BUTTON_CANCEL, 3, NULL);
}

//EPT-16 Kim
int Calc_Callback(const char *str, int len, int fontSize)
{
    char *spTempBuffer = NULL;
    spTempBuffer = (char *)malloc((len + 1) * sizeof(char));
    int w = 0,h = 0;
    sprintf(spTempBuffer, "%.*s", len, str);
    FtGetStrArea(spTempBuffer, &w, &h);
    free(spTempBuffer);
    return w;
}

void MultiLngPrnStr(uchar alignment, uchar size, const char *str, ...)
{
	int w = 0,h = 0;
    static char sbuffer[2048];
    static char *pStr = NULL;
    va_list pVaList;
    va_start(pVaList, str);
    vsnprintf(sbuffer, sizeof(sbuffer), (char*)str, pVaList);
    va_end(pVaList);

    //TODO for now just for sms
    if(ChkHardware(HWCFG_PRINTER, 0))
    {
        if(sg_SMS_Buffer[0] == 0)
            strcpy(sg_SMS_Buffer, sbuffer);
        else
            strcat(sg_SMS_Buffer, sbuffer);
        return;
    }

    if(strcmp(LANGCONFIG, "Arabia") == 0)
    {
        if (alignment == GUI_ALIGN_LEFT)
        {
            FtFontAlign(GUI_ALIGN_RIGHT);
        }
        else if (alignment == GUI_ALIGN_RIGHT)
        {
            FtFontAlign(GUI_ALIGN_LEFT);
        }
        else if (alignment == GUI_ALIGN_CENTER)
        {
            FtFontAlign(GUI_ALIGN_CENTER);
        }
        char *pAr = XuiBidiStrdup(sbuffer);
        if(pAr){
            strcpy(sbuffer, pAr);
            free(pAr);
        }
    }
    else
    {
        FtFontAlign(alignment);
    }

    FtFontSize(size);
    pStr = AutoWrap(sbuffer, 384, size, Calc_Callback);
    if(pStr) {
        FtPrnStr(pStr);
        free(pStr);
        pStr = NULL;
    }
}

// end of file

