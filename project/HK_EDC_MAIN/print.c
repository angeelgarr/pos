
#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
enum{
	PRN_6x8=0,
	PRN_8x16,
	PRN_16x16,
	PRN_12x24,
	PRN_24x24,
	PRN_6x12,
	PRN_12x12,
	PRN_NULL=0xFF
};

static int  PrnFontSetNew(uchar ucEnType, uchar ucSQType);
//static void PrnSmallConstStr(uchar *str);

static int  PrintReceipt_FreeFmat(uchar ucPrnFlag);
static int  PrintReceipt_T(uchar ucPrnFlag);
static int  PrnCurAcqTransList_T(void);
void PrnHead(uchar ucFreeFmat, uchar bCompact);
static int  PrnCustomLogo_T(void);
void PrnHead_T(void);
static void PrnAmount(uchar *pszIndent, uchar ucNeedSepLine);
static void PrnAmount_PPDCC(void); // PP-DCC
static void PrnDescriptor(void);
static void PrnAdditionalPrompt(void);
static void PrnStatement(void);
static void PrnTotalInfo(void *pstInfo);
static int  PrnParaAcq(uchar ucAcqIndex);
static void PrnParaIssuer(uchar ucAcqIndex);
static void PrnParaIssuerSub(uchar ucIssuerKey);
static void PrnIssuerOption(uchar *psOption);
static void PrnCardTable(uchar ucIssuerKey);
static int  PrnInstalmentPara(void);
static int  PrnEmvPara(void);
static void PrnHexString(char *pszTitle, uchar *psHexStr, int iLen, uchar bNewLine);
static void DispPrnError(uchar ucErrCode);

static void s_PrnCardNum(void);
static void s_PrnCardTypeAndExpDate(void);
static void s_PrnHolder(char *pszHeader);


//PP disclaimer auto adjust
#define SPACE                 0x20
#define PRN_ROW_CHAR_MAX      40      
#define PRN_ROW_CHAR_SP_MAX   30
#define PRN_ROW_MAX           10

uchar GetPrintLine(uchar* inBuf, int inBufLen, uchar outBuf[PRN_ROW_MAX][PRN_ROW_CHAR_MAX+1], uchar rowMaxChar); //build88S 1.0E
//end PP disclaimer

static  uchar ucLOGO[1024*20];
static	uchar LogoName[] = "LOGO.BMP";

typedef struct tagBITMAPFILEHEADER {
	ushort    bfType;
	ulong	 bfSize;
	ushort    bfReserved1;
	ushort    bfReserved2;
	uint   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	ulong      biSize;
	long       biWidth;
	long       biHeight;
	ushort       biPlanes;
	ushort       biBitCount;
	ulong      biCompression;
	ulong      biSizeImage;
	long       biXPelsPerMeter;
	long       biYPelsPerMeter;
	ulong      biClrUsed;
	ulong      biClrImportant;
} BITMAPINFOHEADER;
/********************** Internal variables declaration *********************/
unsigned char sLogoMcDcc[]={
0x05,
0x00,0xb0, 
   0x00,0x00,0x3e,0x00,0x00,0x00,0x3e,0x08,0x08,0x3e,0x00,0x38,0x14,0x12,0x3c,0x00,
   0x1e,0x20,0x10,0x0e,0x00,0x3e,0x2a,0x2a,0x22,0x00,0x00,0x00,0x1c,0x22,0x22,0x22,
   0x00,0x3e,0x08,0x08,0x3e,0x00,0x1c,0x22,0x22,0x1c,0x00,0x24,0x2a,0x2a,0x12,0x00,
   0x3e,0x2a,0x2a,0x22,0x00,0x3e,0x04,0x08,0x3e,0x00,0x00,0x00,0x00,0x3e,0x04,0x08,
   0x3e,0x00,0x1c,0x22,0x22,0x1c,0x02,0x02,0x3e,0x02,0x02,0x00,0x00,0x02,0x02,0x3e,
   0x02,0x02,0x1c,0x22,0x22,0x1c,0x00,0x00,0x00,0x1e,0x20,0x20,0x1e,0x00,0x24,0x2a,
   0x2a,0x12,0x00,0x3e,0x2a,0x2a,0x22,0x00,0x00,0x00,0x00,0x02,0x02,0x3e,0x02,0x02,
   0x3e,0x08,0x08,0x3e,0x00,0x3e,0x2a,0x2a,0x22,0x00,0x00,0x00,0x00,0x3e,0x04,0x18,
   0x04,0x3e,0x00,0x38,0x14,0x12,0x3c,0x00,0x24,0x2a,0x2a,0x12,0x00,0x02,0x02,0x3e,
   0x02,0x02,0x3e,0x2a,0x2a,0x22,0x00,0x3e,0x0a,0x1a,0x24,0x00,0x1c,0x22,0x22,0x22,
   0x00,0x38,0x14,0x12,0x3c,0x00,0x3e,0x0a,0x1a,0x24,0x00,0x3e,0x22,0x22,0x1c,0x00,
   
0x00,0xb0, 
   0x00,0x8e,0x91,0x91,0x91,0x80,0x8f,0x10,0x10,0x8f,0x00,0x1f,0x05,0x8d,0x12,0x80,
   0x9f,0x85,0x8d,0x92,0x00,0x1f,0x15,0x15,0x11,0x80,0x1f,0x02,0x04,0x1f,0x00,0x8e,
   0x11,0x91,0x10,0x81,0x02,0x9c,0x02,0x81,0x00,0x00,0x00,0x0e,0x91,0x11,0x11,0x00,
   0x0e,0x11,0x11,0x0e,0x00,0x9f,0x02,0x04,0x9f,0x00,0x0f,0x10,0x88,0x07,0x00,0x9f,
   0x15,0x15,0x91,0x00,0x9f,0x85,0x8d,0x92,0x00,0x12,0x15,0x15,0x09,0x00,0x9f,0x00,
   0x0e,0x91,0x11,0x0e,0x80,0x9f,0x02,0x04,0x1f,0x00,0x00,0x00,0x80,0x9f,0x85,0x05,
   0x02,0x80,0x9f,0x85,0x8d,0x12,0x00,0x8e,0x91,0x91,0x0e,0x00,0x8e,0x91,0x11,0x11,
   0x80,0x1f,0x15,0x95,0x11,0x80,0x92,0x95,0x15,0x09,0x00,0x92,0x95,0x95,0x09,0x80,
   0x80,0x80,0x80,0x1c,0x0a,0x09,0x1e,0x00,0x1f,0x02,0x04,0x9f,0x00,0x1f,0x11,0x91,
   0x8e,0x80,0x00,0x00,0x00,0x9c,0x0a,0x09,0x9e,0x00,0x8e,0x11,0x15,0x8d,0x00,0x1f,
   0x85,0x8d,0x92,0x80,0x9f,0x95,0x95,0x91,0x00,0x1f,0x15,0x15,0x11,0x00,0x00,0x00,
   
0x00,0xb0, 
   0x00,0x00,0xc0,0x8f,0x00,0x80,0xcf,0x02,0x02,0x8f,0x40,0x8e,0x05,0x84,0x4f,0x40,
   0x40,0x0f,0x40,0x40,0xc0,0x40,0x40,0xc0,0x40,0x4f,0x40,0x00,0xc0,0x40,0x40,0x87,
   0x08,0x87,0x48,0x47,0x40,0x0f,0x00,0x8f,0x48,0x88,0x08,0xc0,0x4f,0x48,0x88,0x08,
   0xc0,0x40,0x40,0x80,0x00,0x0f,0x02,0x02,0x0f,0x00,0x8e,0x45,0x44,0x8f,0x00,0x87,
   0x48,0x44,0x83,0x00,0xcf,0x8a,0x0a,0xc8,0x00,0x80,0x40,0x40,0x40,0x00,0xcf,0x41,
   0x42,0x4f,0x00,0xc7,0x48,0x48,0x87,0x00,0xc0,0x80,0x00,0xc0,0x0f,0xc2,0x06,0xc9,
   0x80,0x0f,0xca,0x0a,0x88,0x40,0x47,0x48,0x08,0x08,0x00,0x07,0x08,0x08,0x47,0x40,
   0xc7,0x48,0x48,0xc7,0x00,0x0f,0xc2,0x06,0xc9,0x40,0x49,0x4a,0x0a,0x04,0x00,0x0f,
   0x0a,0x0a,0x08,0x80,0x40,0x40,0x40,0x00,0xc0,0x0e,0x05,0xc4,0x0f,0xc0,0x47,0x48,
   0x8a,0x06,0xc0,0x4e,0x45,0x84,0x0f,0xc0,0x4f,0x40,0x4f,0x01,0xc2,0x8f,0x00,0xc9,
   0x0a,0x8a,0x44,0x40,0x00,0x4f,0x80,0x00,0x80,0x40,0x00,0x00,0x00,0x00,0x00,0x00,
   
0x00,0xb0, 
   0x00,0xc0,0x27,0x20,0x23,0x00,0xc7,0x20,0x27,0xc2,0x02,0xe7,0x40,0x84,0xe5,0x05,
   0xe2,0x00,0x00,0xe0,0x07,0xe0,0xa0,0xa7,0x25,0x05,0xe4,0xa0,0xa7,0x41,0x03,0x44,
   0xa0,0xa3,0x24,0x04,0xe4,0x00,0xc7,0x22,0x22,0xc7,0x00,0xe7,0x41,0x83,0xe4,0x00,
   0x07,0x04,0x04,0x03,0x00,0xc0,0x20,0x20,0xc0,0x00,0xe3,0xa4,0xa4,0x42,0x00,0x03,
   0x04,0x04,0x03,0xe0,0x07,0x20,0x21,0xe7,0x20,0x23,0x04,0x44,0xa4,0xa0,0x27,0x05,
   0x05,0x04,0x00,0x07,0x01,0xe3,0x24,0x20,0xc7,0x00,0xe1,0x07,0x40,0xa7,0xa0,0x27,
   0x00,0xc1,0x27,0x20,0x23,0x04,0xe5,0x03,0x00,0x00,0x00,0xc0,0x20,0x20,0xc0,0x00,
   0x47,0xa0,0xa0,0x27,0x01,0xe1,0x07,0x00,0xe7,0x05,0xe5,0xa4,0xa0,0x40,0x00,0xe0,
   0xa0,0xa0,0x20,0x03,0x04,0x04,0x04,0x00,0x03,0x04,0x04,0x03,0x00,0x07,0x01,0x03,
   0x04,0x00,0x07,0x01,0x03,0x04,0x00,0x07,0x05,0x05,0x04,0x00,0x07,0x00,0x01,0x07,
   0x00,0x03,0x04,0x04,0x04,0x00,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   
0x00,0xb0, 
   0x00,0x01,0x02,0x02,0x02,0x00,0x01,0x02,0x02,0x01,0x00,0x03,0x00,0x00,0x03,0x00,
   0x01,0x02,0x01,0x00,0x00,0x03,0x02,0x02,0x02,0x00,0x03,0x00,0x01,0x02,0x00,0x02,
   0x02,0x02,0x01,0x00,0x03,0x00,0x01,0x02,0x02,0x01,0x00,0x03,0x00,0x00,0x03,0x00,
   0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x02,0x01,0x00,0x03,0x00,0x01,0x02,0x00,0x00,
   0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x02,0x02,0x02,0x01,0x00,
   0x00,0x00,0x00,0x00,0x00,0x03,0x02,0x02,0x01,0x00,0x03,0x00,0x02,0x02,0x02,0x01,
   0x00,0x01,0x02,0x02,0x02,0x00,0x03,0x02,0x02,0x02,0x00,0x01,0x02,0x02,0x01,0x00,
   0x02,0x02,0x02,0x01,0x00,0x01,0x02,0x02,0x01,0x00,0x03,0x00,0x01,0x02,0x00,0x03,
   0x02,0x02,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 
   
};

/********************** external reference declaration *********************/
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// For thermal, small={8x16,16x16}
// For sprocket, small=normal={6x8,16x16}
void PrnSetSmall(void)
{
	if (ChkIfThermalPrinter())
	{
#if defined(_S_SERIES_) || defined(_SP30_) || defined(_P58_)
		PrnFontSetNew(PRN_8x16, PRN_16x16);
		PrnSpaceSet(1, 2);
#else
		PrnFontSet(0, 0);
		PrnSpaceSet(1, 2);
#endif
	}
	else
	{
		PrnSetNormal();
	}
}

// For thermal, normal={12x24,24x24}
// For sprocket, normal={6x8,16x16}
void PrnSetNormal(void)
{
	if (ChkIfThermalPrinter())
	{
#if defined(_S_SERIES_) || defined(_SP30_) || defined(_P58_)
		PrnFontSetNew(PRN_12x24, PRN_24x24);
#else
		PrnFontSet(1, 1);
#endif
		PrnSpaceSet(1, 3);
	}
	else
	{
#if defined(_S_SERIES_) || defined(_SP30_) || defined(_P58_)
		PrnFontSetNew(PRN_6x8, PRN_16x16);
#else
		PrnFontSet(0, 0);
#endif
		PrnSpaceSet(0, 2);
	}
}
// For thermal, normal=big={12x24,24x24}
// For sprocket, big={8x16,16x16}
void PrnSetBig(void)
{
	if (ChkIfThermalPrinter())
	{
		PrnSetNormal();
	}
	else
	{
#if defined(_S_SERIES_) || defined(_SP30_) || defined(_P58_)
		PrnFontSetNew(PRN_8x16, PRN_16x16);
#else
		PrnFontSet(1, 1);
#endif
		PrnSpaceSet(0, 2);
	}
}
#ifdef AMT_PROC_DEBUG
void  PrintEcrAmount(void)//2014-11-4
{
	uchar	szBuff[50];
	uchar   ucTmp;
	int i=0, iRet;

	ScrCls();
	PubShowTitle(TRUE, (uchar *)_T("PRN ECR AMT  "));

	PrnInit();
	PrnSetNormal();
	PrnStr("\n\n");
	PrnStr("---- Get Ecr Amt List ----\n");

	for(i;i<glEcrAmtCnt;i++)
	{
		PrnStr("[%d]TxnType:%d   TxnTime:%.6s\n",i, glEcrAMT.glEcrAmtLog[i].szTranType, glEcrAMT.glEcrAmtLog[i].szTime);
		PrnStr("**EcrAmt:%.12s\n",glEcrAMT.glEcrAmtLog[i].szEcrAmt);
	}
	PrnStr("----- END OF LIST -----");

	PrnStr("\f");

	StartPrinter();
}

void  PrintAmount(void)//2014-9-18 ttt: debug printing all amounts in process
{
	uchar	szBuff[50];
	uchar   ucTmp;
	int i=0, iRet;

	ScrCls();
	//2014-9-18 load record first
	PubShowTitle(TRUE, (uchar *)_T("PRN AMT PROC  "));
 
	//2014-9-19 choose to print last record or spec record
	/*PubDispString("1.LAST TRANS", 2|DISP_LINE_LEFT);
	PubDispString("2.SPEC TRANS", 4|DISP_LINE_LEFT);*/
	//while (1)
	//{
	//	ucTmp = PubWaitKey(USER_OPER_TIMEOUT);
	//	if (ucTmp==KEY1)
	//	{
	//		if( glSysCtrl.uiRePrnRecNo >= MAX_TRANLOG )
	//		{
	//			PubDispString(_T("EMPTY BATCH"), 4|DISP_LINE_LEFT);
	//			PubBeepErr();
	//			PubWaitKey(5);
	//			return;
	//		}
	//		InitTransInfo();

	//		iRet = LoadTranLog(&glProcInfo.stTranLog, glSysCtrl.uiRePrnRecNo);
	//		if( iRet!=0 )
	//		{
	//			return;
	//		}

	//	}
	//	else if (ucTmp==KEY2)
	//	{
			if( GetTranLogNum(ACQ_ALL)==0 )
			{
				PubDispString(_T("EMPTY BATCH"), 4|DISP_LINE_LEFT);
				PubBeepErr();
				PubWaitKey(5);
				return;
			}

			InitTransInfo();
			iRet = GetRecord(TS_ALL_LOG, &glProcInfo.stTranLog);
			if( iRet!=0 )
			{
				return;
			}
	/*	}
	}	*/

	PubShowTitle(TRUE, (uchar *)_T("PRN AMT PROC  "));

	PrnInit();
	PrnSetNormal();
	PrnStr("\n\n");
	PrnStr("----Amount Process START----\n");

	PrnStr("Trace No.:%06lu\n", glProcInfo.stTranLog.ulInvoiceNo);//2014-11-4
	////PrnStr("Txn Time:%s\n", glProcInfo.stTranLog.szDateTime);
	Conv2EngTime(glProcInfo.stTranLog.szDateTime, szBuff);  //DATE/TIME
	PrnStr("%22s\n", szBuff);

	for(i;i<glProcInfo.stTranLog.cntAmt;i++)
	{
		PrnStr("glAmt:%.17s\n",glProcInfo.stTranLog.glAmt[i]);
		PrnStr("szAmt:%.17s\n\n",glProcInfo.stTranLog.szAmt[i]);
	}
	PrnStr("-----Amount Process END-----");

	PrnStr("\f");

	StartPrinter();
	glProcInfo.stTranLog.cntAmt = 0;//2014-9-22
}
#endif
int PrintReceipt(uchar ucPrnFlag)
{
	uchar	szBuff[50];
	uchar	szIssuerName[10+1];
	uchar szTranName[16+1];
	uchar	sTVR[6], sTSI[5], sCVR[5] = {0}, sCvmR[5] = {0}; //Gillian debug
	ushort	usLength;

	if( !ChkIssuerOption(ISSUER_EN_PRINT) )
	{
		return 0;
	}
	
	if (ChkIfIrDAPrinter())
	{
		SetOffBase(OffBaseDisplay);
		ChkOnBase();
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

	//2014-7-7 add two more line
	//if (ChkIfDahOrBCM())
	//{
	//	PrnStr("\n\n\n");
	//}
	//else
	{
		PrnStr("\n\n\n");//2014-11-12 delete two lines \n\n
	}

	PrnHead(FALSE, FALSE);

   // PrnStep(15);//build 114

	ConvIssuerName(glCurIssuer.szName, szIssuerName);
	PrnStr("%s\n", szIssuerName); // issuer Name

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
		PrnStr("%s S\n", szBuff);
	}
	else if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	{
		PrnStr("%s C\n", szBuff);
	}
	else if( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
			 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
		PrnStr("%s F\n", szBuff);
	}
	else if( glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS )
	{
		PrnStr("%s T\n", szBuff);
	}
	else
	{
		PrnStr("%s M\n", szBuff);
	}
	PrnStr("%s\n", glProcInfo.stTranLog.szHolderName);

	// print txn name & expiry
	sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel);
	if (glSysParam.stEdcInfo.ucLanguage == 1)
	{
		if( glProcInfo.stTranLog.ucTranType==VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
		{
			sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel);				
			sprintf((char *)szBuff, " %s %s %s %s ", _T("VOID"), _T(szTranName), "VOID", szTranName);	
		}
		else if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
		{
			sprintf((char *)szBuff, "%s(%s) %s %s", _T(szTranName), _T("ADJ"), szTranName, "ADJ");
		}
		else if (glProcInfo.stTranLog.uiEntryMode == MODE_CONTACTLESS 
#ifdef PAYPASS_DEMO
			&& (gucIsPayWavePass == PAYWAVE)
#endif		
            && (glProcInfo.stTranLog.szCVMRslt[0] != CLSS_CVM_CONSUMER_DEVICE) //add by richard 20161230, V1.00.0219.
			)
		{
			if (!glProcInfo.stTranLog.uiStatus & TS_NOSEND)
			{
				if (glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_2
					|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_3
					|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_MSD_20)
				{
					sprintf((char *)szBuff, " %s %s", szTranName,"PAYWAVE");
				}
				else if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MCHIP
					|| glProcInfo.stWaveTransData.ucSchemeId ==SCHEME_MC_MSTR)
				{
					sprintf((char *)szBuff, " %s %s", szTranName,"PAYPASS");
				}					
			}
			else
			{
				if (glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_2
					|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_3
					|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_MSD_20)
				{
					sprintf((char *)szBuff,"%s %s","OFFLINE","PAYWAVE");
				}
				else if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MCHIP
					|| glProcInfo.stWaveTransData.ucSchemeId ==SCHEME_MC_MSTR)
				{
					sprintf((char *)szBuff,"%s %s","OFFLINE","PAYPASS");
				}				
			}		
		}
		else
		{
			sprintf((char *)szBuff, " %s %s", _T(szTranName),szTranName);
		}
	} 
	else
	{
		if( glProcInfo.stTranLog.ucTranType==VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
		{		
			sprintf((char *)szBuff, "%s(%s)", "VOID", glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel);	
		}
		else if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
		{
			sprintf((char *)szBuff, "%s(%s)", szTranName, "ADJ");
		}
		else if (glProcInfo.stTranLog.uiEntryMode == MODE_CONTACTLESS
#ifdef PAYPASS_DEMO
			&& (gucIsPayWavePass == PAYWAVE)
#endif	
            && (glProcInfo.stTranLog.szCVMRslt[0] != CLSS_CVM_CONSUMER_DEVICE) //add by richard 20161230, V1.00.0219.
			)
		{
			if (!glProcInfo.stTranLog.uiStatus & TS_NOSEND)
			{
				if (glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_2
					|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_3
					|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_MSD_20)
				{
					sprintf((char *)szBuff, " %s %s", szTranName,"PAYWAVE");
				}
				else if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MCHIP
					|| glProcInfo.stWaveTransData.ucSchemeId ==SCHEME_MC_MSTR)
				{
					sprintf((char *)szBuff, " %s %s", szTranName,"PAYPASS");
				}					
			}
			else
			{
				if (glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_2
					|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_3
					|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_MSD_20)
				{
					sprintf((char *)szBuff,"%s %s","OFFLINE","PAYWAVE");
				}
				else if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MCHIP
					|| glProcInfo.stWaveTransData.ucSchemeId ==SCHEME_MC_MSTR)
				{
					sprintf((char *)szBuff,"%s %s","OFFLINE","PAYPASS");
				}				
			}	
		}
		else
		{
			sprintf((char *)szBuff, "%s", szTranName);	
		}
	}
	//CLSS transaction didn't save scheme,here need to use PAN. squall 2013.12.11
	if (PRN_REPRINT == ucPrnFlag&&glProcInfo.stTranLog.uiEntryMode == MODE_CONTACTLESS
        && (glProcInfo.stTranLog.szCVMRslt[0] != CLSS_CVM_CONSUMER_DEVICE)) //add by richard 20161230, V1.00.0219.
	{	
		if (!glProcInfo.stTranLog.uiStatus & TS_NOSEND)
		{
			if (glProcInfo.stTranLog.szPan[0]=='4')
			{
				sprintf((char *)szBuff, " %s %s", szTranName,"PAYWAVE");
			}
			else if (/*glProcInfo.stTranLog.szPan[0]=='5'*/ChkIfMC(glProcInfo.stTranLog.szPan) )//2016-2-15
			{
				sprintf((char *)szBuff, " %s %s", szTranName,"PAYPASS");
			}
		}
		else//offline
		{
			if (glProcInfo.stTranLog.szPan[0]=='4')
			{
				sprintf((char *)szBuff,"%s %s","OFFLINE","PAYWAVE");
			}
			else if (/*glProcInfo.stTranLog.szPan[0]=='5'*/ChkIfMC(glProcInfo.stTranLog.szPan) )//2016-2-15
			{
				sprintf((char *)szBuff,"%s %s","OFFLINE","PAYPASS");
			}	
		}	
	}
	if( ChkIssuerOption(ISSUER_EN_EXPIRY) )
	{
		if( ChkIssuerOption(ISSUER_MASK_EXPIRY) )
		{
			PrnSetBig();
			PrnStr(" %-16.16s", szBuff);
			PrnSetNormal();
			PrnStr("**/**\n");
		}
		else
		{
			PrnSetBig();
			PrnStr(" %-16.16s", szBuff);
			PrnSetNormal();
			PrnStr("%2.2s/%2.2s\n", &glProcInfo.stTranLog.szExpDate[2],
					glProcInfo.stTranLog.szExpDate);
		}
	}
	else
	{
		PrnSetBig();
		PrnStr(" %s\n", szBuff);
		PrnSetNormal();
	}

	// Batch NO & invoice #
	PrnStep(6);
	PrnStr("   %06ld%14s%06ld\n", glCurAcq.ulCurBatchNo, "", glProcInfo.stTranLog.ulInvoiceNo);
	Conv2EngTime(glProcInfo.stTranLog.szDateTime, szBuff);  //DATE/TIME
	PrnStr("%22s\n", szBuff);
	PrnStr("%15.12s%8s%-6.6s\n", glProcInfo.stTranLog.szRRN, "", glProcInfo.stTranLog.szAuthCode);
	
	PrnStep(2);
	if( glProcInfo.stTranLog.ucInstalment!=0 )
	{
		PrnStr("   NO. OF INSTALMENT:%02d\n", glProcInfo.stTranLog.ucInstalment);
	}
	
	if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT || glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS)
	{
	
        if (strlen(glProcInfo.stTranLog.szAppPreferName)!=0)
        {
		
		    PrnStr("   APP: %.16s\n", glProcInfo.stTranLog.szAppPreferName);
        } 
        else
        {
		    PrnStr("   APP: %.16s\n", glProcInfo.stTranLog.szAppLabel);
        }
		PubBcd2Asc0(glProcInfo.stTranLog.sAID, glProcInfo.stTranLog.ucAidLen, szBuff);
		PubTrimTailChars(szBuff, 'F');
		PrnStr("   AID: %s\n", szBuff);
		PubBcd2Asc0(glProcInfo.stTranLog.sAppCrypto, 8, szBuff);
		PrnStr("   TC : %s\n", szBuff);
//#if defined(ENABLE_EMV) && defined(EMV_TEST_VERSION)//2016-4-25 AE Cert Test!    


//#endif
		if( glProcInfo.stTranLog.uiEntryMode & MODE_OFF_PIN )
		{
			PrnStr("   PIN VERIFIED\n");
		}
	}

	PrnDescriptor();
	
	// amount
    if (PPDCC_ChkIfDccAcq()) // PP-DCC
    {
        PrnAmount_PPDCC();
    }
    else
    {    	
	PrnAmount((uchar *)"   ", TRUE);
    }

// #ifdef _P60_S1_
// 	if (ChkIfShanghaiCB()) 
// 	{
// 		PrnStr("      上海商业银行\n");
// 		PrnStr("      处处为您着想\n");
// 	}
// #else
// 	if (ChkIfShanghaiCB()) 
// 	{
// 		PrnStr("      坝穨蝗︽\n");
// 		PrnStr("      矪矪眤帝稱\n");
// 	}
// #endif


	PrnAdditionalPrompt();

	PrnStatement();

	/*- 
- ┮ΤDCC ユ
  ?PIN, 虫沮莱ゴ NO SIGNATURE REQUIRED ㎝ PIN VERIFIED⊿Τ???┪铬筁????碞ぃノゴNO SIGNATURE REQUIRED ㎝ PIN VERIFIED

- 獶DCCユ

  >JCB ⊿ΤSTT, ┮Τ贺薄猵, 碞琌Τ?PIN碞 NO SIGNATURE REQUIRED ㎝ PIN VERIFIED
  ⊿Τ???┪铬筁????碞ぃノゴNO SIGNATURE REQUIRED ㎝ PIN VERIFIED

   >VISA, MASTER, AMEX ΤSTT, ┮Τㄢ贺薄猵

      >> 肂ゑ?肂蔼:
Τ????⌒≈ゴNO SIGNATURE REQUIRED ㎝ PIN VERIFIED

⊿Τ???┪铬筁????⌒≈ぃノゴNO SIGNATURE REQUIRED ㎝ PIN VERIFIED

>> 肂筁?肂:
            Τ????⌒≈ゴNO SIGNATURE REQUIRED (STT)㎝ PIN VERIFIED

            ⊿Τ???┪铬筁????⌒≈ゴNO SIGNATURE REQUIRED (STT), ぃゴ PIN VERIFIED

  э戳эㄓэи竒礚猭糶冈灿惠―э笆叫高拜keith┪sandy
	*/
	if( (!ChkIfPrnSignature())|| 
		(glProcInfo.stTranLog.uiEntryMode & MODE_OFF_PIN && 
		(memcmp(glProcInfo.stTranLog.szPan,"34",2)== 0 || memcmp(glProcInfo.stTranLog.szPan,"37",2)==0)))//2014-9-2 ttt
	{
		if (ChkIfNeedTip())
		{
			PrnStr("\n\n");
		}
		else
		{
			PrnStr("\n\n\n\n\n\n");
		}

		PrnSetBig();
		PrnStr("NO SIGNATURE REQUIRED\n");
	}
	//Gillian 2016-8-12
	else if ((glProcInfo.stTranLog.szCVMRslt[0] == CLSS_CVM_CONSUMER_DEVICE)) // Gillian applepay signature issue
	{

		if (ChkIfNeedTip())
		{
			PrnStr("\n\n");
		}
		else
		{
			PrnStr("\n\n\n\n\n\n");
		}
		PrnSetBig();
		PrnStr("NO SIGNATURE REQUIRED\n");
	}
	if( ucPrnFlag==PRN_REPRINT )
	{
		PrnSetBig();
		PrnStr("       REPRINT\n");
	}
	if (ChkIfTrainMode())
	{    
		PrnStr("    ****DEMO****");
	}
	else
	{
#if defined(ENABLE_EMV) && defined(EMV_TEST_VERSION)
		PrnStr("* FOR EMV TEST ONLY *");
#endif
	}
	PrnStr("\f");

	StartPrinter();
	return 0;
}

// 打印明细
// Print the list of all transaction
void PrnAllList(void)//func75
{
	int		iRet;
	uchar	szTitle[16+1], ucIndex;
	uchar prnAllFlag;
	uchar prnEndFlag;//2013-10-7 tttttt



	PubShowTitle(TRUE, (uchar *)_T("PRINT LOG       "));
	sprintf((char *)szTitle, _T("PRINT LOG       "));
	iRet = SelectAcq(TRUE, szTitle, &ucIndex);

	if( iRet!=0 )
	{
		return;
	}

		if( ucIndex!=MAX_ACQ )
	{
		SetCurAcq(ucIndex); 
			PrnInit();//2013-11-11 squall modified cause before start print no init call.
			/*
		if( ChkIfThermalPrinter() )
		{
			PrnHead_T();
		}
		else
		{
		
			PrnHead(FALSE, FALSE);
		}
		*/
	//	StartPrinter();
		PrnCurAcqTransList(FALSE);
		return;
	}

	//2013-10-2 Lois: when select all, check all acquirers for transaction record, unless all empty, POS display "EMPTY BATCH"
	prnAllFlag = FALSE;
	prnEndFlag = -1;
	for(ucIndex=0; ucIndex<glSysParam.ucAcqNum; ucIndex++)
	{
		SetCurAcq(ucIndex);
		if( GetTranLogNum(glCurAcq.ucKey)!=0 )
		{
			prnAllFlag = TRUE;
			prnEndFlag = ucIndex;//2013-10-7 ttttttttt
		}
	}
	for(ucIndex=0; ucIndex<glSysParam.ucAcqNum; ucIndex++)
	{
		SetCurAcq(ucIndex);
// 		PrnInit();
// 		if( ChkIfThermalPrinter() )
// 		{
// 			PrnHead_T();
// 		}
// 		else
// 		{
// 			PrnHead(FALSE, FALSE);
// 		}
// 		StartPrinter();
		if( prnEndFlag == ucIndex)
		{
			prnAllFlag = 2;
		}
		PrnCurAcqTransList(prnAllFlag);
	}	
}

// print list of transaction of current acquirer
int PrnCurAcqTransList(uchar prnAllFlag)//2013-10-2 Lois: when select all, check all acquirers for transaction record, unless all empty, POS display "EMPTY BATCH"
{
	int		iCnt, iNumOfOnePage, iPageNum, iMaxNumOfOnePage;
	uchar	szBuff[30], szIssuerName[10+1];
	uchar   szDateTime[14+1], szTranName[16+1];
	uchar   szCurAcqEndTag;//2013-10-10

	PubShowTitle(TRUE, (uchar *)"PRINT LOG       ");
	if( GetTranLogNum(glCurAcq.ucKey)==0 )
	{
		if( !prnAllFlag )
		{
			PubDispString(_T("EMPTY BATCH"), 4|DISP_LINE_CENTER);
			PubBeepErr();
			PubWaitKey(5);
		}
		return 1;
	}

	DispPrinting();
	if( ChkIfThermalPrinter() )
	{
		return PrnCurAcqTransList_T();
	}
	if( prnAllFlag )//if not print all do not call init here
	{
		PrnInit();
	}

	PrnSetNormal();

	if( ChkIfThermalPrinter() )
	{
		PrnHead_T();
	}
	else
	{
		PrnStr("\n\n\n\n");
		PrnHead(FALSE, FALSE);
	}

//	PrnStep(20);
	PrnStr("      TRANSACTION LIST\n");
	PubGetDateTime(szDateTime);
	Conv2EngTime(szDateTime, szBuff);
	PrnStr("%s\n", szBuff);

	PrnStr("HOST NII: %.3s   %.10s\n", glCurAcq.szNii, glCurAcq.szName);
//	PrnStep(15);
//	PrnStr("%14.8s\n%21.15s\n\n", glCurAcq.szTermID, glCurAcq.szMerchantID);

	iMaxNumOfOnePage = ChkEdcOption(EDC_TIP_PROCESS) ? 4 : 6;
	iNumOfOnePage = 0;
	iPageNum = 1;
	szCurAcqEndTag = FALSE;
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
		PrnStr("ISSUER: %.10s\n", szIssuerName);

		if( ChkIfDispMaskPan2() )
		{
			MaskPan(glProcInfo.stTranLog.szPan, szBuff);
			PrnStr("PAN: %s\n", szBuff);
		}
		else
		{
			PrnStr("PAN: %s\n", glProcInfo.stTranLog.szPan);
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
		PrnStr("%-21.21s%06lu\n", szBuff, glProcInfo.stTranLog.ulInvoiceNo);

		 //Ver1.00.0145 add by Jolie 2013-11-14 should print local currency for FUNC 75 (maybe there are stored the currency with cardholder currency)
		glProcInfo.stTranLog.stTranCurrency = glSysParam.stEdcInfo.stLocalCurrency;
		PrnAmount((uchar *)"", FALSE);
		PrnStr("\n");

		iNumOfOnePage++;
		if( (iPageNum==1 && iNumOfOnePage==3) ||
			(iPageNum!=1 && iNumOfOnePage==iMaxNumOfOnePage) )
		{
			PrnStr("\f");
			if( StartPrinter()!=0 )
			{
				return 1;
			}

			iNumOfOnePage = 0;
			iPageNum++;

			PrnInit();
			PrnSetNormal();

			PrnStep(20);
			szCurAcqEndTag = TRUE;
		}
		else
		{
			szCurAcqEndTag = FALSE;
		}
	}
	if( !prnAllFlag || prnAllFlag == 2)//2013-10-7 if "SELECT ALL", do not print "END OF LIST" until all printed
	{
		PrnStr("      END OF LIST   ");
		PrnStr("\f");		
	}
	else if( !szCurAcqEndTag )
	{
		PrnStr("\f");		
	}
	else if( szCurAcqEndTag )
	{
		PrnStep(-20);
	}

	if( StartPrinter()!=0 )
	{
		return 1;
	}	
}

int PrnCurAcqTransList_T(void)
{
	int		iCnt, iNumOfOnePage;
	uchar	szBuff[30], szIssuerName[10+1];
	uchar   szTranName[16+1];

	PrnInit();
	PrnSetNormal();

	PrnStr("\n");
	PrnStr("     TRANSACTION LIST\n");
	PrnEngTime();

	PrnStr("HOST NII: %.3s   %.10s\n", glCurAcq.szNii, glCurAcq.szName);
	PrnStep(15);
	PrnStr("TID: %s\nMID: %s\n", glCurAcq.szTermID, glCurAcq.szMerchantID);
	PrnStr("============================\n");
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
		PrnStr("ISSUER: %.10s\n", szIssuerName);

		if( ChkIfDispMaskPan2() )
		{
			MaskPan(glProcInfo.stTranLog.szPan, szBuff);
			PrnStr("PAN: %s\n", szBuff);
		}
		else
		{
			PrnStr("PAN: %s\n", glProcInfo.stTranLog.szPan);
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
		PrnStr("TXN: %-21.21s\nTRACE: %06lu\n", szBuff, glProcInfo.stTranLog.ulInvoiceNo);

		 //Ver1.00.0145 add by Jolie 2013-11-14 should print local currency for FUNC 75 (maybe there are stored the currency with cardholder currency)
		glProcInfo.stTranLog.stTranCurrency = glSysParam.stEdcInfo.stLocalCurrency;
		PrnAmount((uchar *)"", FALSE);
		PrnStr("============================");
		PrnStr("\n");
		
		iNumOfOnePage++;
		if( (iNumOfOnePage%5)==0 )
		{
			if( StartPrinter()!=0 )
			{
				return 1;
			}
			iNumOfOnePage = 0;
			PrnInit();
			PrnSetNormal();
			PrnStep(20);
		}
	}
	PrnStr("      END OF LIST   ");
	PrnStr("\n\n\n\n\n\n");
	return StartPrinter();	
}

int PrintReceipt_FreeFmat(uchar ucPrnFlag)
{
	uchar	szBuff[50];
	uchar	szIssuerName[10+1];
	uchar szTranName[16+1];

	PrnInit();
	PrnSetNormal();

	//2014-7-7 add two more line
	PrnStr("\n\n\n");//2014-11-12 remove two lines \n\n
	PrnHead(TRUE, PPDCC_ChkIfDccAcq());

    if (!PPDCC_ChkIfDccAcq())
    {
        PrnStep(15);
    }

    if (PPDCC_ChkIfDccAcq())
    {
        s_PrnHolder("");
        s_PrnCardNum();
        s_PrnCardTypeAndExpDate();
    }
    else
    {
	// issuer Name
	ConvIssuerName(glCurIssuer.szName, szIssuerName);
	PrnStr("CARD TYPE: %-10.10s    ", szIssuerName);

	// Expiry date
	if( ChkIssuerOption(ISSUER_EN_EXPIRY) )
	{
		if( ChkIssuerOption(ISSUER_MASK_EXPIRY) )
		{
			PrnStr("**/**");
		}
		else
		{
			PrnStr("%2.2s/%2.2s", &glProcInfo.stTranLog.szExpDate[2],
					glProcInfo.stTranLog.szExpDate);
		}
	}

	PrnStr("\n");

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
		PrnStr("CARD NO: %s S\n", szBuff);
	}
	else if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	{
		PrnStr("CARD NO: %s C\n", szBuff);
	}
	else if( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
			 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
		PrnStr("CARD NO: %s F\n", szBuff);
	}
	else if( glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS )
	{
		PrnStr("CARD NO: %s T\n", szBuff);
	}
	else
	{
		PrnStr("CARD NO: %s M\n", szBuff);
	}

	// Holder
	PrnStr("HOLDER: %s\n", glProcInfo.stTranLog.szHolderName);
    }
	// print txn name & expiry
	sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel);
		if (glSysParam.stEdcInfo.ucLanguage == 1)
		{
			if( glProcInfo.stTranLog.ucTranType==VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
			{
				sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel);				
				sprintf((char *)szBuff, " %s %s %s %s ", _T("VOID"), _T(szTranName), "VOID", szTranName);	
			}
			else if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
			{
				sprintf((char *)szBuff, "%s(%s) %s %s", _T(szTranName), _T("ADJ"), szTranName, "ADJ");
			}
			else if (glProcInfo.stTranLog.uiEntryMode == MODE_CONTACTLESS
#ifdef PAYPASS_DEMO
				&& (gucIsPayWavePass == PAYWAVE)
#endif	
                && (glProcInfo.stTranLog.szCVMRslt[0] != CLSS_CVM_CONSUMER_DEVICE) //add by richard 20161230, V1.00.0219.
				)
			{
				if (!glProcInfo.stTranLog.uiStatus & TS_NOSEND)
				{
					if (glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_2
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_3
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_MSD_20)
					{
						sprintf((char *)szBuff, " %s %s", szTranName,"PAYWAVE");
					}
					else if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MCHIP
						|| glProcInfo.stWaveTransData.ucSchemeId ==SCHEME_MC_MSTR)
					{
						sprintf((char *)szBuff, " %s %s", szTranName,"PAYPASS");
					}					
				}
				else
				{
					if (glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_2
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_3
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_MSD_20)
					{
						sprintf((char *)szBuff,"%s %s","OFFLINE","PAYWAVE");
					}
					else if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MCHIP
						|| glProcInfo.stWaveTransData.ucSchemeId ==SCHEME_MC_MSTR)
					{
						sprintf((char *)szBuff,"%s %s","OFFLINE","PAYPASS");
					}				
				}			
			}
			else
			{
				sprintf((char *)szBuff, " %s %s", _T(szTranName),szTranName);
			}
		} 
		else
		{
			if( glProcInfo.stTranLog.ucTranType==VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
			{		
				sprintf((char *)szBuff, "%s(%s)", "VOID", glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel);
			}
			else if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
			{
				sprintf((char *)szBuff, "%s(%s)", szTranName, "ADJ");
			}
			else if (glProcInfo.stTranLog.uiEntryMode == MODE_CONTACTLESS
#ifdef PAYPASS_DEMO
				&& (gucIsPayWavePass == PAYWAVE)
#endif	
                && (glProcInfo.stTranLog.szCVMRslt[0] != CLSS_CVM_CONSUMER_DEVICE) //add by richard 20161230, V1.00.0219.
				)
			{
				if (!glProcInfo.stTranLog.uiStatus & TS_NOSEND)
				{
					if (glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_2
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_3
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_MSD_20)
					{
						sprintf((char *)szBuff, " %s %s", szTranName,"PAYWAVE");
					}
					else if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MCHIP
						|| glProcInfo.stWaveTransData.ucSchemeId ==SCHEME_MC_MSTR)
					{
						sprintf((char *)szBuff, " %s %s", szTranName,"PAYPASS");
					}					
				}
				else
				{
					if (glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_2
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_3
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_MSD_20)
					{
						sprintf((char *)szBuff,"%s %s","OFFLINE","PAYWAVE");
					}
					else if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MCHIP
						|| glProcInfo.stWaveTransData.ucSchemeId ==SCHEME_MC_MSTR)
					{
						sprintf((char *)szBuff,"%s %s","OFFLINE","PAYPASS");
					}				
				}	
			}
			else
			{
				sprintf((char *)szBuff, "%s", szTranName);
			}
		}
		//CLSS transaction didn't save scheme,here need to use PAN. squall 2013.12.11
		if (PRN_REPRINT == ucPrnFlag&&glProcInfo.stTranLog.uiEntryMode == MODE_CONTACTLESS
            && (glProcInfo.stTranLog.szCVMRslt[0] != CLSS_CVM_CONSUMER_DEVICE)) //add by richard 20161230, V1.00.0219.
		{	
			if (!glProcInfo.stTranLog.uiStatus & TS_NOSEND)
			{
				if (glProcInfo.stTranLog.szPan[0]=='4')
				{
					sprintf((char *)szBuff, " %s %s", szTranName,"PAYWAVE");
				}
				else if (/*glProcInfo.stTranLog.szPan[0]=='5'*/ChkIfMC(glProcInfo.stTranLog.szPan) )//2016-2-15
				{
					sprintf((char *)szBuff, " %s %s", szTranName,"PAYPASS");
				}
			}
			else//offline
			{
				if (glProcInfo.stTranLog.szPan[0]=='4')
				{
					sprintf((char *)szBuff,"%s %s","OFFLINE","PAYWAVE");
				}
				else if (/*glProcInfo.stTranLog.szPan[0]=='5'*/ChkIfMC(glProcInfo.stTranLog.szPan) )//2016-2-15
				{
					sprintf((char *)szBuff,"%s %s","OFFLINE","PAYPASS");
				}	
			}	
	}
    if (PPDCC_ChkIfDccAcq())
    {
        PrnStr("TRANS: ");
    }
	PrnStr("%s\n", szBuff);

	// Batch NO & invoice #
	PrnStep(6);
	PrnStr("BATCH NO: %06ld TRACE: %06ld\n", glCurAcq.ulCurBatchNo, glProcInfo.stTranLog.ulInvoiceNo);//2014-8-14 revise from "REF" to "TRACE"

    if (PPDCC_ChkIfDccAcq())
    {
	    // RRN
	    PrnStr("RRN: %-12.12s\n", glProcInfo.stTranLog.szRRN);
    } 
    else
    {
	// RRN, AuthCode
	PrnStr("RRN: %-12.12s AUTH: %-6.6s\n", glProcInfo.stTranLog.szRRN, glProcInfo.stTranLog.szAuthCode);
    }

	PrnStep(2);
	if( glProcInfo.stTranLog.ucInstalment!=0 )
	{
		PrnStr("   NO. OF INSTALMENT:%02d\n", glProcInfo.stTranLog.ucInstalment);
	}

	if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT || glProcInfo.stTranLog.szCVMRslt[0]==CLSS_CVM_CONSUMER_DEVICE) //add by richard 20161230, V1.00.0219.
	{
         //squall build126
            if (strlen(glProcInfo.stTranLog.szAppPreferName)!=0)
            {
		    PrnStr("   APP: %.16s\n", glProcInfo.stTranLog.szAppPreferName);
            } 
            else
            {
		    PrnStr("   APP: %.16s\n", glProcInfo.stTranLog.szAppLabel);
            }
       
		PubBcd2Asc0(glProcInfo.stTranLog.sAID, glProcInfo.stTranLog.ucAidLen, szBuff);
		PubTrimTailChars(szBuff, 'F');
		PrnStr("   AID: %s\n", szBuff);
		PubBcd2Asc0(glProcInfo.stTranLog.sAppCrypto, 8, szBuff);
		PrnStr("   TC : %s\n", szBuff);
#if defined(ENABLE_EMV) && defined(EMV_TEST_VERSION)//2016-4-25 AE Cert Test!
    	PubBcd2Asc0(glProcInfo.stTranLog.sTSI, 2, szBuff);
    	PrnStr("   TSI: %s\n", szBuff);
    	PubBcd2Asc0(glProcInfo.stTranLog.sTVR, 5, szBuff);
    	PrnStr("   TVR: %s\n", szBuff);
#endif
		if( glProcInfo.stTranLog.uiEntryMode & MODE_OFF_PIN )
		{
			PrnStr("   PIN VERIFIED\n");
		}
		else
		{
			PrnStr("\n");
		}
	}

    if (PPDCC_ChkIfDccAcq())
    {
        PrnStr("APP CODE:  %-6.6s\n", glProcInfo.stTranLog.szAuthCode);
    }

	PrnDescriptor();

    if (PPDCC_ChkIfDccAcq()) // PP-DCC
    {
        PrnAmount_PPDCC();
    }
    else
    {	    
	PrnAmount((uchar *)"   ", TRUE);
    }

// 	if (ChkIfShanghaiCB())
// 	{
// 		PrnStr("      上海商业银行\n");
// 		PrnStr("      处处为您着想\n");
// 	}
	PrnAdditionalPrompt();

	PrnStatement();


	if ( (!ChkIfPrnSignature())|| 
		(glProcInfo.stTranLog.uiEntryMode & MODE_OFF_PIN && 
		(memcmp(glProcInfo.stTranLog.szPan,"34",2)== 0 || memcmp(glProcInfo.stTranLog.szPan,"37",2)==0)))//2014-9-2 ttt
	{
		if (ChkIfNeedTip())
		{
			PrnStr("\n\n");
		}
		else
		{
			PrnStr("\n\n\n\n\n\n");
		}
		PrnSetBig();
		PrnStr("NO SIGNATURE REQUIRED\n");
	}
	//Gillian 2016-8-12
	else if ((glProcInfo.stTranLog.szCVMRslt[0] == CLSS_CVM_CONSUMER_DEVICE)) // Gillian applepay signature issue
	{

		if (ChkIfNeedTip())
		{
			PrnStr("\n\n");
		}
		else
		{
			PrnStr("\n\n\n\n\n\n");
		}
		PrnSetBig();
		PrnStr("NO SIGNATURE REQUIRED\n");
	}
	if( ucPrnFlag==PRN_REPRINT )
	{
		PrnFontSetNew(PRN_8x16, PRN_16x16);
		PrnStr("       REPRINT\n");
	}
	if (ChkIfTrainMode())
	{    
		PrnStr("    ****DEMO****");
	}
	
	PrnStr("\f");
	
	StartPrinter();
	return 0;
}

int PrintReceipt_T(uchar ucPrnFlag)
{	
	uchar	ucNum, iRet;
	uchar	szBuff[50],szBuf1[50];
	uchar	szIssuerName[10+1], szTranName[16+1];
	uchar	sTVR[6], sTSI[5], sCVR[5] = {0}, sCvmR[5] = {0}; //Gillian debug
	int	iLength, iLengthCVR, iLengthCVMR;

	for(ucNum=0; ucNum<NumOfReceipt(); ucNum++)
	{
		PrnInit();
		PrnSetNormal();

		PrnCustomLogo_T();
		if(ChkEdcOption(EDC_PRINT_LOGO_BMP))//2015-1-7 add switch
		{
			if (ChkIfPrnLogo()==0)
			{
				PrnAcqLogo();
			}
		}
		PrnHead_T();
		
		// issuer Name

		sprintf((char *)szBuff, "%s\n", _T("CARD TYPE:"));

		
		PrnSetSmall();
		PrnStr(szBuff);
		
		ConvIssuerName(glCurIssuer.szName, szIssuerName);
		PrnSetNormal();
		PrnStr("%s\n", szIssuerName);
		
		// PAN
		sprintf((char *)szBuff, "%s\n", _T("CARD NO./EXP. DATE"));
		PrnSetSmall();
		PrnStr(szBuff);
		
		
		//memcpy(szBuff, glProcInfo.stTranLog.szPan, sizeof(glProcInfo.stTranLog.szPan));

		if (ChkIfTransMaskPan(1))//2013.11.7 AMEX 
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
		else if( glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS )
		{
			sprintf(szBuf1, "%s (T)", szBuff);
		}
		else
		{
			sprintf(szBuf1, "%s (M)", szBuff);			
		}
		PrnSetNormal();
		PrnStr("%-23.23s", szBuf1);
		
		// print expiry
		
		if( ChkIssuerOption(ISSUER_EN_EXPIRY) )
		{
			if( ChkIssuerOption(ISSUER_MASK_EXPIRY) )
			{
				PrnStr(" **/**");
			}
			else
			{
				PrnStr(" %2.2s/%2.2s", &glProcInfo.stTranLog.szExpDate[2],
					glProcInfo.stTranLog.szExpDate);
			}
		}
		
		PrnStr("\n");
		
		
		
		sprintf((char *)szBuff, "%s\n", _T("HOLDER"));
		PrnSetSmall();
		PrnStr(szBuff);
		

		// Holder name

		PrnSetNormal();
		PrnStr("%-23.23s\n", glProcInfo.stTranLog.szHolderName);
		
		// Transaction type
		
		sprintf((char *)szBuff, "%s\n", _T("TRANS. TYPE"));
		PrnSetSmall();
		PrnStr(szBuff);
		
		
		sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel);
		if (glSysParam.stEdcInfo.ucLanguage == 1)
		{
			if( glProcInfo.stTranLog.ucTranType==VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
			{
				sprintf(szTranName, "%.16s", glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel);	
				sprintf((char *)szBuff, " %s %s %s %s ", _T("VOID"), _T(szTranName), "VOID", szTranName);										
			}
			else if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
			{
				sprintf((char *)szBuff, "%s(%s) %s %s", _T(szTranName), _T("ADJ"), szTranName, "ADJ");	
			}
			else if (glProcInfo.stTranLog.uiEntryMode == MODE_CONTACTLESS
#ifdef PAYPASS_DEMO
				&& (gucIsPayWavePass == PAYWAVE)
#endif	
                && (glProcInfo.stTranLog.szCVMRslt[0] != CLSS_CVM_CONSUMER_DEVICE) //add by richard 20161230, V1.00.0219.
				)
			{
				if (!glProcInfo.stTranLog.uiStatus & TS_NOSEND)
				{
					if (glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_2
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_3
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_MSD_20)
					{
						sprintf((char *)szBuff, " %s %s", szTranName,"PAYWAVE");
					}
					else if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MCHIP
						|| glProcInfo.stWaveTransData.ucSchemeId ==SCHEME_MC_MSTR)
					{
						sprintf((char *)szBuff, " %s %s", szTranName,"PAYPASS");
					}	
					else//2016-5-3 AMEX Express
					{
						sprintf((char *)szBuff, " %s %s", szTranName,"EXPRESS");
					}
				}
				else
				{
					if (glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_2
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_3
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_MSD_20)
					{
						sprintf((char *)szBuff,"%s %s","OFFLINE","PAYWAVE");
					}
					else if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MCHIP
						|| glProcInfo.stWaveTransData.ucSchemeId ==SCHEME_MC_MSTR)
					{
						sprintf((char *)szBuff,"%s %s","OFFLINE","PAYPASS");
					}	
					else//2016-5-3 AMEX Express
					{
						sprintf((char *)szBuff, " %s %s", "OFFLINE","EXPRESS");
					}
				}			
			}
			else
			{
				sprintf((char *)szBuff, " %s %s", _T(szTranName),szTranName);			
			}
		} 
		else
		{
			if( glProcInfo.stTranLog.ucTranType==VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
			{						
				sprintf((char *)szBuff, "%s(%s)", "VOID", glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szLabel);			
			}
			else if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
			{
				sprintf((char *)szBuff, "%s(%s)", szTranName, "ADJ");
			}
			else if (glProcInfo.stTranLog.uiEntryMode == MODE_CONTACTLESS 
                && (glProcInfo.stTranLog.szCVMRslt[0] != CLSS_CVM_CONSUMER_DEVICE) //add by richard 20161230, V1.00.0219.
                )
			{
				if (!glProcInfo.stTranLog.uiStatus & TS_NOSEND)
				{
					if (glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_2
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_3
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_MSD_20)
					{
						sprintf((char *)szBuff, " %s %s", szTranName,"PAYWAVE");
					}
					else if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MCHIP
						|| glProcInfo.stWaveTransData.ucSchemeId ==SCHEME_MC_MSTR)
					{
						sprintf((char *)szBuff, " %s %s", szTranName,"PAYPASS");
					}		
					else//2016-5-3 AMEX Express
					{
						sprintf((char *)szBuff, " %s %s", szTranName,"EXPRESS");
					}
				}
				else
				{
					if (glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_2
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_3
						|| glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_MSD_20)
					{
						sprintf((char *)szBuff,"%s %s","OFFLINE","PAYWAVE");
					}
					else if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MCHIP
						|| glProcInfo.stWaveTransData.ucSchemeId ==SCHEME_MC_MSTR)
					{
						sprintf((char *)szBuff,"%s %s","OFFLINE","PAYPASS");
					}		
					else//2016-5-3 AMEX Express
					{
						sprintf((char *)szBuff, " %s %s", "OFFLINE","EXPRESS");
					}
				}		
			}
			else
			{
				sprintf((char *)szBuff, "%s", szTranName);	
			}
		}
		//CLSS transaction didn't save scheme,here need to use PAN. squall 2013.12.11
		if (PRN_REPRINT == ucPrnFlag&&glProcInfo.stTranLog.uiEntryMode == MODE_CONTACTLESS
            && (glProcInfo.stTranLog.szCVMRslt[0] != CLSS_CVM_CONSUMER_DEVICE)) //add by richard 20161230, V1.00.0219.
		{	
			if (!glProcInfo.stTranLog.uiStatus & TS_NOSEND)
			{
				if (glProcInfo.stTranLog.szPan[0]=='4')
				{
					sprintf((char *)szBuff, " %s %s", szTranName,"PAYWAVE");
				}
				else if (/*glProcInfo.stTranLog.szPan[0]=='5'*/ChkIfMC(glProcInfo.stTranLog.szPan) )//2016-2-15
				{
					sprintf((char *)szBuff, " %s %s", szTranName,"PAYPASS");
				}
				else//2016-5-3 AMEX Express
				{
					sprintf((char *)szBuff, " %s %s", szTranName,"EXPRESS");
				}
			}
			else//offline
			{
				if (glProcInfo.stTranLog.szPan[0]=='4')
				{
					sprintf((char *)szBuff,"%s %s","OFFLINE","PAYWAVE");
				}
				else if (/*glProcInfo.stTranLog.szPan[0]=='5'*/ChkIfMC(glProcInfo.stTranLog.szPan) )//2016-2-15
				{
					sprintf((char *)szBuff,"%s %s","OFFLINE","PAYPASS");
				}	
				else//2016-5-3 AMEX Express
				{
					sprintf((char *)szBuff, " %s %s", "OFFLINE","EXPRESS");
				}
			}	
		}
		PrnSetNormal();
		PrnStr("%s\n", szBuff);
		
		// Batch, Invoice
		
		sprintf((char *)szBuff, "%s\n", _T("BATCH NO.              TRACE NO."));
		
		PrnSetSmall();
		PrnStr(szBuff);
		
		
		sprintf((char *)szBuff, "%06lu          %06lu\n", glCurAcq.ulCurBatchNo, glProcInfo.stTranLog.ulInvoiceNo);
		PrnSetNormal();
		PrnStr(szBuff);
		
		
		
		// Date, time

		sprintf((char *)szBuff, "%s\n", _T("DATE/TIME                       "));
		
		PrnSetSmall();
		PrnStr(szBuff);
		
		Conv2EngTime(glProcInfo.stTranLog.szDateTime, szBuff);
		
		PrnSetNormal();
		PrnStr("%-22.22s\n", szBuff);
		
		// REF, APPV
		sprintf((char *)szBuff, "%s\n", _T("REF. NO.               APP. CODE"));
		PrnSetSmall();
		PrnStr(szBuff);
		PrnSetNormal();
		PrnStr("%-14.14s  %-12.12s\n", glProcInfo.stTranLog.szRRN, glProcInfo.stTranLog.szAuthCode);
		// ECR
        if ((strlen(glProcInfo.stTranLog.szEcrTxnNo)!=0) || (strlen(glProcInfo.stTranLog.szEcrConseqNo)!=0))
        {
			sprintf((char *)szBuff, "%s\n", _T("TXN. NO.               TXN.CONSEQ.NO"));
			PrnSetSmall();
			PrnStr(szBuff);
			PrnSetNormal();
			PrnStr("%-6.6s          %-4.4s\n", glProcInfo.stTranLog.szEcrTxnNo, glProcInfo.stTranLog.szEcrConseqNo);
        }
		
		PrnStr("\n");
		
		if( glProcInfo.stTranLog.ucInstalment!=0 )
		{
			PrnStr("   NO. OF INSTALMENT:%02d\n\n", glProcInfo.stTranLog.ucInstalment);
		}
		if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT || glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS)
		{
			if (strlen(glProcInfo.stTranLog.szAppPreferName)!=0)
			{
				PrnStr("APP:%.16s\n", glProcInfo.stTranLog.szAppPreferName);
			} 
			else
			{
				PrnStr("APP:%.16s\n", glProcInfo.stTranLog.szAppLabel);
			}
			
			PubBcd2Asc0(glProcInfo.stTranLog.sAID, glProcInfo.stTranLog.ucAidLen, szBuff);
			PubTrimTailChars(szBuff, 'F');
			PrnStr("AID:%.32s\n", szBuff);
			
			PubBcd2Asc0(glProcInfo.stTranLog.sAppCrypto, 8, szBuff);
			PrnStr("TC: %.16s\n", szBuff);
#if defined(ENABLE_EMV) && defined(EMV_TEST_VERSION)//2016-4-25 AE Cert Test!
			PubBcd2Asc0(glProcInfo.stTranLog.sTSI, 2, szBuff);
			PrnStr("   TSI: %s\n", szBuff);
			PubBcd2Asc0(glProcInfo.stTranLog.sTVR, 5, szBuff);
			PrnStr("   TVR: %s\n", szBuff);
#endif
			if( glProcInfo.stTranLog.uiEntryMode & MODE_OFF_PIN )
			{
				PrnStr("   PIN VERIFIED\n");
			}
			else
			{
				PrnStr("\n");
			}
		}
				
		PrnDescriptor();
		
		// amount
        if (PPDCC_ChkIfDccAcq()) // PP-DCC
        {
            PrnAmount_PPDCC();
        } 
        else
        {
		PrnAmount((uchar *)"", TRUE);  //Print amount
        }

// 		if (ChkIfShanghaiCB()) 
// 		{
// 			PrnStr("      坝穨蝗︽\n");
// 			PrnStr("      矪矪眤帝稱\n");
// 		}
// 		
		PrnAdditionalPrompt();
		PrnStatement();
		if( ucPrnFlag==PRN_REPRINT )
		{
			sprintf((char *)szBuff, "%s\n", _T("         * REPRINT *"));
			PrnStr((char *)szBuff);
		}
		
		if( ucNum==0 )
		{
			
			sprintf((char *)szBuff, "%s\n", _T("CARDHOLDER SIGNATURE"));
			PrnSetSmall();
			PrnStr(szBuff);
			PrnSetNormal();

			if( (!ChkIfPrnSignature())|| 
				(glProcInfo.stTranLog.uiEntryMode & MODE_OFF_PIN && 
				(memcmp(glProcInfo.stTranLog.szPan,"34",2)== 0 || memcmp(glProcInfo.stTranLog.szPan,"37",2)==0)))//2014-9-2 ttt
			{
				PrnStr("NO SIGNATURE REQUIRED\n");
			}
			//Gillian 2016-8-12
			else if ((glProcInfo.stTranLog.szCVMRslt[0] == CLSS_CVM_CONSUMER_DEVICE)) // Gillian applepay signature issue
			{
				PrnStr("NO SIGNATURE REQUIRED\n");
			}
			else
			{
				PrnStr("\n\n\n\n");
			}
			PrnStr("-----------------------------\n");
			PrnSetSmall();
			
			//2015-12-15 AMEX INSTAL
			if(ChkIfAmex() && glProcInfo.stTranLog.ucTranType == INSTALMENT)
			{			
				PrnStr("I AGREE TO THE INTEREST-FREE INSTALMENT PROGRAM TERMS AND CONDITIONS, A COPY OF WHICH HAS BEEN PROVIDED TO ME.\n");
			}
			else
			{
				PrnStr("I ACKNOWLEDGE SATISFACTORY RECEIPT OF RELATIVE  GOODS/SERVICE\n");
			}
		}

		if (ChkIfTrainMode())
		{
			sprintf((char *)szBuff, "%s\n", _T("     ***** DEMO *****"));
		//	PrnSetNormal();
			PrnFontSet(6, 6);
			PrnStr((char *)szBuff);
		}
		else
		{
#if defined(ENABLE_EMV) && defined(EMV_TEST_VERSION)
			sprintf((char *)szBuff, "%s\n", _T("  ** FOR EMV TEST ONLY **"));
			PrnStr((char *)szBuff);
#endif
		}

        PrnSetNormal();
		if( ucNum==0 )
		{
			PrnStr(_T("  **** MERCHANT COPY ****  "));

#ifdef EMV_DEBUG
			//if( (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) ||
			//(glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) )
			
				//Gillian emv debug
				memset(glTVR, 0, sizeof(glTVR));
				memset(glTSI, 0, sizeof(glTSI));
				memset(glCVMR, 0, sizeof(glCVMR));
				memset(glCVR, 0, sizeof(glCVR));
				iRet = EMVGetTLVData(0x95, glTVR,  &iLength);
				iRet = EMVGetTLVData(0x9B, glTSI,  &iLength);
				iRet = EMVGetTLVData(0x9F10, glCVR,  &iLengthCVR); //Gillian emv debug
				iRet = EMVGetTLVData(0x9f34, glCVMR,  &iLengthCVMR);
				PrnStr("\nTSI = %02x%02x\n", glTSI[0], glTSI[1]);//, glEmvStatus.glTsiLen, DEVICE_PRN, HEX_MODE);
				PrnStr("TVR = %02x%02x%02x%02x%02x\n", glTVR[0], glTVR[1]
				, glTVR[2], glTVR[3], glTVR[4]);//, glEmvStatus.glTvrLen, DEVICE_PRN, HEX_MODE);
				PrnStr("CVR = %02x%02x%02x%02x%02x\n", glCVR[3], glCVR[4]
				, glCVR[5], glCVR[6], glCVR[7]);
				
				PubDebugOutput(" \nCVR", glCVR, iLengthCVR, DEVICE_COM1, HEX_MODE); //Gillian emv debug
				PubDebugOutput(" \nCVMR", glCVMR, iLengthCVMR,DEVICE_COM1, HEX_MODE);  //Gillian emv debug
#endif
	
		}
		else if( ucNum==1 )
		{
			PrnStr(_T("  **** CUSTOMER COPY ****  "));
		}
		else if( ucNum==2 )
		{
			PrnStr(_T("  **** BANK COPY ****  "));
		}
		PrnStr("\n\n\n\n\n\n\n\n");

		StartPrinter();

		if( ucNum==0 && NumOfReceipt() != 1)
		{
            kbflush();
			ScrClrLine(2,7);
			PubDispString(_T("PRESS ANY KEY"), 4|DISP_LINE_CENTER);
			PubWaitKey(USER_OPER_TIMEOUT);
		}
	}

	return 0;
}

void PrnHead(uchar ucFreeFmat, uchar bCompact)
{
	uchar	szBuff[32];

	if (ChkIfTrainMode())
	{
		//PrnStr("\n");
		PrnStep(5);
		PrnStr(_T("DEMONSTRATE ONLY\n"));
		sprintf(szBuff, "%.30s", _T("NOT FOR PAYMENT PROOF"));
		if( szBuff[0]>=0xA0 )
		{
			PrnStr("%.30s\n", szBuff);
		}
		else
		{
			PrnStr("%.30s\n\n", szBuff);
		}
	} 
	else
	{
	
		PrnStr("%.23s\n", glSysParam.stEdcInfo.szMerchantName);

		if( glSysParam.stEdcInfo.szMerchantAddr[0]>=0xA0 )
		{
			PrnStr("%.23s\n", glSysParam.stEdcInfo.szMerchantAddr);
		}
		else
		{
			PrnStr("%.23s\n%.23s\n", glSysParam.stEdcInfo.szMerchantAddr,
					&glSysParam.stEdcInfo.szMerchantAddr[23]);
		}
	}

    if (!bCompact)
    {
	PrnStep(15);
    }

	if (ucFreeFmat)
	{
		GetEngTime(szBuff);
		PrnStr("MID:%-15.15s      %5.5s\n",   glCurAcq.szMerchantID, szBuff+11);
		PrnStr("TID:%-8.8s        %10.10s\n", glCurAcq.szTermID, szBuff);
	}
	else
	{
		
		PrnStr("%14.8s\n%21.15s\n", glCurAcq.szTermID, glCurAcq.szMerchantID);
	}
	if (!ChkIfCHB())
	{	
		 PrnStep(15);
	}
	else
	{
		PrnStr("\n");//squallbuild119
		PrnStep(5);
	}
	
}


int PrnFontSetNew(uchar ucEnType, uchar ucSQType)
{
#if defined(_S_SERIES_) || defined(_P58_)
	int	iRet;
	ST_FONT font1,font2;

	font1.CharSet = CHARSET_WEST;
	font1.Width   = 8;
	font1.Height  = 16;
	font1.Bold    = 0;
	font1.Italic  = 0;

	font2.CharSet = glSysParam.stEdcInfo.stLangCfg.ucCharSet;
	font2.Width   = 16;
	font2.Height  = 16;
	font2.Bold    = 0;
	font2.Italic  = 0;

	if (ucEnType==PRN_6x8)
	{
		font1.Width   = 6;
		font1.Height  = 8;
	}
	if (ucEnType==PRN_6x12)
	{
		font1.Width   = 6;
		font1.Height  = 12;
	}
	if (ucEnType==PRN_12x24)
	{
		font1.Width   = 12;
		font1.Height  = 24;
	}

	if (ucSQType==PRN_12x12)
	{
		font2.Width   = 12;
		font2.Height  = 12;
	}
	if (ucSQType==PRN_24x24)
	{
		font2.Width   = 24;
		font2.Height  = 24;
	}

	// in WIN32 do not allow NULL
#ifndef WIN32
	if (font1.CharSet==font2.CharSet)
	{
		iRet = PrnSelectFont(&font1,NULL);
	} 
	else
#endif
	{
		iRet = PrnSelectFont(&font1,&font2);
	}
	PrnDoubleWidth(0, 0);
	PrnDoubleHeight(0, 0);

	if ((iRet!=0) && (font1.Width>=12) && (font2.Width>=12))
	{
		font1.Width /= 2;
		font1.Height /= 2;
		font2.Width /= 2;
		font2.Height /= 2;
		iRet = PrnSelectFont(&font1,&font2);
		if (iRet==0)
		{
			PrnDoubleWidth(1, 1);
			PrnDoubleHeight(1, 1);
		}		
	}

	return iRet;

#else
	if (ucEnType==PRN_6x8)
	{
		PrnFontSet(0, 0);
	}
	else
	{
		PrnFontSet(1, 1);
	}

	return 0;
	
#endif
}

// for thermal only
// void PrnSmallConstStr(uchar *str)
// {
// 	PrnSetSmall();
// 	PrnStr(str);
// 	PrnSetNormal();
// }

int PrnCustomLogo_T(void)
{
	return -1;
}

void PrnHead_T(void)
{
	uchar	szBuff[32];

	PrnStep(30);
	PrnSetNormal();

	if (ChkIfTrainMode())
	{
		
		PrnStr(_T("* DEMONSTRATE ONLY *"));
		PrnStr("\n");
		PrnStep(10);
		
		sprintf(szBuff, "%.30s", _T("* NOT FOR PAYMENT PROOF *"));
		if( szBuff[0]>=0xA0 )
		{
			PrnStr("%.30s\n", szBuff);
		}
		else
		{
			PrnStr("%.30s\n\n", szBuff);
		}
		PrnStep(15);
	}
	else
	{
		PrnStr("%.23s\n", glSysParam.stEdcInfo.szMerchantName);//print the string of "MERCHANT NAME", just the string, not the real name.

		PrnStep(10);
		
		if (strlen(glSysParam.stEdcInfo.szMerchantAddr)>23)
		{
			//	PrnSetSmall();//squall 2013.12.11 should print the same size as merchant name
			sprintf(szBuff, "%.23s\n", glSysParam.stEdcInfo.szMerchantAddr);
			PrnStr(szBuff);
			sprintf(szBuff, "%.23s", glSysParam.stEdcInfo.szMerchantAddr+23);
			PrnStr(szBuff);
		}
		else
		{
		//	PrnSetSmall();
			PrnStr(glSysParam.stEdcInfo.szMerchantAddr);
			PrnStr("\n");
		}
		PrnSetNormal();
		PrnStr("\n");
		PrnStep(15);
	}
	
	

	
	PrnSetSmall();
	
	PrnStr(_T("MERCHANT ID.        "));
	PrnSetNormal();
	PrnStr("%15.15s\n", glCurAcq.szMerchantID);
	PrnSetSmall();
	
	PrnStr(_T("TERMINAL ID.        "));
	PrnSetNormal();
	PrnStr("%15.15s\n", glCurAcq.szTermID);
	
	PrnStr("=============================\n");
	PrnStep(15);
}

void PrnAmount(uchar *pszIndent, uchar ucNeedSepLine)
{
	uchar	szBuff[50], szBuff1[50], szTotalAmt[12+1];
	ulong  szMonthlyDue;
	uchar   ucBuff[6+1],szMonthlyAmt[12+1], szBuff2[50];

	szMonthlyDue = atol(glProcInfo.stTranLog.szAmount)/(int)glProcInfo.stTranLog.ucInstalment;//2015-12-16 AMEX EPP: /INST Plan ???
	PubLong2Bcd(szMonthlyDue,6,ucBuff);//2015-11-12
	PubBcd2Asc0(ucBuff,6,szMonthlyAmt);
	////2016-1-11 ttt
	//ScrCls();
	//ScrPrint(0,0,0, "MthDue: %ld",szMonthlyDue);
	//ScrPrint(0,2,0, "MthAmt: %.12s",szMonthlyAmt);
	//getkey();
	
/*build1.0E: VOID slip also print tips
	if( ChkIfNeedTip() &&
		!(glProcInfo.stTranLog.uiStatus &TS_VOID) )
*/
  if( ChkIfNeedTip() )
	{
		//-------------------------------- BASE --------------------------------
		App_ConvAmountTran(glProcInfo.stTranLog.szAmount, szBuff, 0);
		if (ChkIfThermalPrinter())
		{
			PrnStr(_T("%sBASE      %17.17s\n"), pszIndent, szBuff);
		}
		else
		{
			PrnStr("%sBASE      %17.17s\n", pszIndent, szBuff);
		}

		//-------------------------------- TIPS --------------------------------
	
		if( !ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount) )
		{
		
				App_ConvAmountTran(glProcInfo.stTranLog.szTipAmount, szBuff, 0);
		

			if (ChkIfThermalPrinter())
			{
				PrnStr(_T("%sTIPS      %17.17s\n"), pszIndent, szBuff);
			} 
			else
			{
				PrnStr("%sTIPS      %17.17s\n", pszIndent, szBuff);
			}
		}
		else
		{
		
				if (ChkIfThermalPrinter())
				{
					PrnStr(_T("%sTIPS\n"), pszIndent);
				} 
				else
				{
					PrnStr("%sTIPS\n", pszIndent);
				}
			

		}

		//-------------------------------- TOTAL --------------------------------
		if( ucNeedSepLine )
		{
			PrnStr("%s          -----------------\n", pszIndent);
		}
		if( !ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount) )
		{
			PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
			App_ConvAmountTran(szTotalAmt, szBuff, 0);
			
			if (ChkIfThermalPrinter())
			{
				if(ChkIfCasinoMode())
				{
					PrnStr(_T("%sTOTAL     MOP%13.13s\n"), pszIndent, szBuff);
					PrnStr(_T("%sDebit in  HKD%13.13s\n"), pszIndent, szBuff);
				}
				else
				{
					PrnStr(_T("%sTOTAL     %17.17s\n"), pszIndent, szBuff);
				}
			} 
			else
			{
				if(ChkIfCasinoMode())
				{
					PrnStr("%sTOTAL     MOP%13.13s\n", pszIndent, szBuff);
					PrnStr("%sDebit in  HKD%13.13s\n", pszIndent, szBuff);
				}
				else
				{
					PrnStr("%sTOTAL     %17.17s\n", pszIndent, szBuff);
				}
			}
		}
		else
		{	
				if (ChkIfThermalPrinter())
				{
					PrnStr(_T("%sTOTAL\n"), pszIndent);
				} 
				else
				{
					PrnStr("%sTOTAL\n", pszIndent);
				}
			
		}
		//if(ChkIfAmex() && glProcInfo.stTranLog.ucTranType == INSTALMENT)//2015-12-16 AMEX EPP: add Monthly Due
		//{
		//	if (ChkIfThermalPrinter())
		//	{
		//		PrnStr(_T("%sMONTHLY DUE\n"), pszIndent);
		//	} 
		//	else
		//	{
		//		PrnStr("%sMONTHLY DUE\n", pszIndent);
		//	}
		//}
		if( ucNeedSepLine )
		{
			PrnStr("%s          =================\n", pszIndent);
		}
	}
	else
	{	
		//Ajust but not send ,when void should print Adjusted total. squall 2013.12.14
		if ((glProcInfo.stTranLog.uiStatus&TS_VOID)&&!ChkIfZeroAmt(glProcInfo.stTranLog.szAbolishedTip))
		{
			PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szAbolishedTip, 12, szTotalAmt);
			App_ConvAmountTran(szTotalAmt, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
		}
		else
		{
			//VOID 已 ADJUST 的 SALE 打印金額由原金額改為印已ADJUST 的 TOTAL. 
			//keith required in mail 2013.12.13,squall 2013.12.14 modify
			if ( !ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount) && glProcInfo.stTranLog.ucTranType == VOID)
			{
				PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
				App_ConvAmountTran(szTotalAmt, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
			
			}
			else
			{
				App_ConvAmountTran(glProcInfo.stTranLog.szAmount, szBuff, GetTranAmountInfo(&glProcInfo.stTranLog));
			
			}
		}
		App_ConvAmountTran(glProcInfo.stTranLog.szDebitInAmt, szBuff1, 0); //Lijy
		App_ConvAmountTran(szMonthlyAmt, szBuff2,0);//2016-1-11
		if (ChkIfThermalPrinter())
		{
			//Lijy
			if(ChkIfCasinoMode())
			{
				PrnStr(_T("%sTOTAL     MOP%13.13s\n"), pszIndent, szBuff);
				PrnStr(_T("%sDebit in  HKD%13.13s\n"), pszIndent, szBuff1);
			}
			else//Lijy
			{
				if(ChkIfAmex() && glProcInfo.stTranLog.ucTranType == INSTALMENT)//2015-12-16 AMEX EPP: add Monthly Due
				{
					PrnStr(_T("%sTOTAL DUE %17.17s\n"), pszIndent, szBuff);
					PrnStr(_T("%sMONTHLY DUE %15.15s\n"), pszIndent,szBuff2);//2016-1-11
				}
				else
				{
					PrnStr(_T("%sTOTAL     %17.17s\n"), pszIndent, szBuff);
				}
			}
		} 
		else
		{
			//Lijy
			if(ChkIfCasinoMode())
			{
				PrnStr("%sTOTAL     MOP%13.13s\n", pszIndent, szBuff);
				PrnStr("%sDebit in  HKD%13.13s\n", pszIndent, szBuff1);
			}
			else//Lijy
			{
				PrnStr("%sTOTAL     %17.17s\n", pszIndent, szBuff);
			}
		}
		
		if( ucNeedSepLine )
		{
			PrnStr("%s          =================\n", pszIndent);
		}
	}
}

// PP-DCC
static void PrnFXRate(void)
{
    uchar   sBuff1[64], sBuff2[64];

  if (glProcInfo.stTranLog.szInvDccRate[0])
	{
        // " USD/HKD 7.654"
		sprintf(sBuff1, "%3.3s/%3.3s", glProcInfo.stTranLog.stHolderCurrency.szName,
                                       glSysParam.stEdcInfo.stLocalCurrency.szName);
        PPDCC_FormatRate(glProcInfo.stTranLog.szInvDccRate, sBuff2);
        PrnStr("FX RATE*: %s %.14s\n", sBuff1, sBuff2);
	}
	else
	{
        // " HKD/USD 0.123"
		sprintf(sBuff1, "%3.3s/%3.3s", glSysParam.stEdcInfo.stLocalCurrency.szName,
                                       glProcInfo.stTranLog.stHolderCurrency.szName);
        PPDCC_FormatRate(glProcInfo.stTranLog.szDccRate, sBuff2);
        PrnStr("FX RATE*: %s %.14s\n", sBuff1, sBuff2);
	}

    
}

// PP-DCC
static void PrnAmount_PPDCC(void)
{
	uchar   sBuff1[64], sBuff2[64];
	uchar   sBuff3[64], sBuff4[64];   //build88S
	uchar   szTotal[12+1];
	uchar   ucAmtAttr;
    
	ucAmtAttr = GetTranAmountInfo(&glProcInfo.stTranLog);    
	PrnSetNormal();
	
	if ( (glProcInfo.stTranLog.ucTranType==VOID)
		|| !PPDCC_ChkIfAutoOptIn())
	{            
		if (!ChkIfThermalPrinter())        
		{
			if (!ChkIfNeedTip())    //build88S: save space
			{            
				PrnStr("\n");        
			}
		}
		if ((glProcInfo.stTranLog.uiStatus&TS_VOID)&&!ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount))
		{
				PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotal);
				App_ConvAmountLocal(szTotal, sBuff1, ucAmtAttr);//squall 2013.12.19
				//App_ConvAmountTran(szTotal, sBuff1, ucAmtAttr);//???? why here use convAmountTran??
		}
		else
		{
			App_ConvAmountLocal(glProcInfo.stTranLog.szAmount, sBuff1, ucAmtAttr);
		}
		
		PrnStr("LOCAL AMT:   %16.16s\n", sBuff1);
		
		//build88S: print tips
		if (ChkIfNeedTip())
		{
			App_ConvAmountLocal(glProcInfo.stTranLog.szTipAmount, sBuff2, ucAmtAttr);
			PrnStr("TIPS: %23.23s\n", sBuff2);
			
			memset(sBuff1, 0, 13);        
			PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, sBuff1);        
			App_ConvAmountLocal(sBuff1, sBuff2, ucAmtAttr);        
			PrnStr("TOTAL: %22.22s\n", sBuff2);
		}
		//end print tips
        
		PrnFXRate();
        

		if ((glProcInfo.stTranLog.uiStatus&TS_VOID)&&!ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount))
		{
			PubAscAdd(glProcInfo.stTranLog.szFrnAmount, glProcInfo.stTranLog.szFrnTip, 12, szTotal);
			App_ConvAmountTran(szTotal, sBuff1, ucAmtAttr);
		}
		else
		{
			App_ConvAmountTran(glProcInfo.stTranLog.szFrnAmount, sBuff1, ucAmtAttr);
		}
	
		PrnStr("TXN CUR AMT: %16.16s\n", sBuff1);
		
		
		//build88S: print tips
		if (ChkIfNeedTip())
		{
			App_ConvAmountTran(glProcInfo.stTranLog.szFrnTip, sBuff2, ucAmtAttr);
			PrnStr("TIPS: %23.23s\n", sBuff2);
			
			memset(sBuff1, 0, 13);        
			PubAscAdd(glProcInfo.stTranLog.szFrnAmount, glProcInfo.stTranLog.szFrnTip, 12, sBuff1);        
			App_ConvAmountTran(sBuff1, sBuff2, ucAmtAttr);        
			PrnStr("TOTAL: %22.22s\n", sBuff2);
		}
		PrnStr("\n");
		//end print tips 
	}
	else
	{        
		if (ChkIfThermalPrinter())        
		{
            PrnFXRate();
            PrnStr("\n");        
		}        
		else        
		{
            PrnFXRate();        
		}
		
        
		PrnStr("\n");
		
		if( glProcInfo.stTranLog.uiStatus & TS_ADJ )    //build88S
		{
		}
		else
		{
			PrnStr("SELECT[X]TRANSACTION CURRENCY\n");
		}
		
        
		if (ChkIfThermalPrinter())        
		{            
			PrnStr("\n");        
		}
		
		
		//build88S
		if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
		{
			sprintf(sBuff1, "[ ]%3.3s AMOUNT   %3.3s AMOUNT[X]\n",				        
				glSysParam.stEdcInfo.stLocalCurrency.szName, glProcInfo.stTranLog.stHolderCurrency.szName);
		}
		else
			//end build88S
		{
			sprintf(sBuff1, "[ ]%3.3s AMOUNT   %3.3s AMOUNT[ ]\n",
				glSysParam.stEdcInfo.stLocalCurrency.szName, glProcInfo.stTranLog.stHolderCurrency.szName);
		}
		
		PrnStr(sBuff1);
		
		App_ConvAmountLocal(glProcInfo.stTranLog.szAmount,   sBuff1, ucAmtAttr);
        App_ConvAmountTran(glProcInfo.stTranLog.szFrnAmount, sBuff2, ucAmtAttr);
        PrnStr("%-14.14s%15.15s\n", sBuff1, sBuff2);
		
		if (ChkIfNeedTip())
		{            
			if (ChkIfThermalPrinter())            
			{                
				PrnStr("\n");            
			}
			
			//build88S
			if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
			{        
				//build1.0R: show both local tip amt & foreign tip amount
				memset(sBuff2, 0, 13);
				memset(sBuff3, 0, 13);
				
				//local tip amount
				App_ConvAmountLocal(glProcInfo.stTranLog.szTipAmount, sBuff3, ucAmtAttr);
				
				//foreign tip amount
				App_ConvAmountTran(glProcInfo.stTranLog.szFrnTip, sBuff2, ucAmtAttr);
				
				//print to buffer
				PrnStr("             TIP\n");
				PrnStr("%-14.14s%15.15s\n", sBuff3, sBuff2);
				//end build1.0R
			}
			else if (glProcInfo.stTranLog.szPan[0] == '4')   //build1.0f: VISA
			{
				PrnStr("\nTIP IN TXN CUR:______________\n");      
			}
			else
				//end build88S
			{
				PrnStr("\nTIP IN TXN CUR:______________\n");
			}
			
            
			if (ChkIfThermalPrinter())            
			{                
				PrnStr("\n");            
			}
			
			
			//build88S
			if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
			{        
				//build1.0R: show both local total amount & foreign total amount
				memset(sBuff1, 0, 13);  
				memset(sBuff2, 0, 13);
				memset(sBuff3, 0, 13);
				memset(sBuff4, 0, 13);
				
				//calculate foreign total
				PubAscAdd(glProcInfo.stTranLog.szFrnAmount, glProcInfo.stTranLog.szFrnTip, 12, sBuff1);
				App_ConvAmountTran(sBuff1, sBuff2, ucAmtAttr);
				
				//calculate local total
				PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, sBuff3);        
				App_ConvAmountLocal(sBuff3, sBuff4, ucAmtAttr);        
				
				//print to buffer
				PrnStr("            TOTAL\n");
				PrnStr("%-14.14s%15.15s\n", sBuff4, sBuff2);
				//end build1.0R
			}
			else if (glProcInfo.stTranLog.szPan[0] == '4')   //build1.0f: VISA
			{			
				PrnStr("\nTOTAL IN TXN CUR:____________\n");
			}
			else
				//end build88S
			{
				PrnStr("\nTOTAL IN TXN CUR:____________\n");
			}      
			
			if (ChkIfThermalPrinter())
			{
				PrnStr("\n");
			}
		}
		else
		{
			PrnStr("\n");
		}
	}
    
  if (ChkIfThermalPrinter())    
  {        
    PrnStep(12);    
  }
}


// print product descriptor
void PrnDescriptor(void)
{
	uchar	ucCnt, ucMaxNum, ucTemp;

	if( ChkIfDccAcquirer() )
	{
		return;
	}

	ucMaxNum = (uchar)MIN(MAX_GET_DESC, glProcInfo.stTranLog.ucDescTotal);
	for(ucCnt=0; ucCnt<ucMaxNum; ucCnt++)
	{
		ucTemp = glProcInfo.stTranLog.szDescriptor[ucCnt] - '0';
		PubASSERT( ucTemp<MAX_DESCRIPTOR );
		PrnStr("   %-20.20s\n", glSysParam.stDescList[ucTemp].szText);
	}

	if (!ChkIfThermalPrinter())
	{
		if( (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT)||(glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS) || ChkEdcOption(EDC_FREE_PRINT) )
		{
			PrnStr("\n");
		}
		else
		{
			for(; ucCnt<MAX_GET_DESC; ucCnt++)
			{
				PrnStr("\n");
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
	PrnStr("%-14.14s%16.16s\n", glSysParam.stEdcInfo.szAddlPrompt,
			glProcInfo.stTranLog.szAddlPrompt);
}

void PrnStatement(void)
{
//build1.0E
  uchar claimBuf[200], outBuf[PRN_ROW_MAX][PRN_ROW_CHAR_MAX+1];                                                    
  uchar line, i, len;                        
  uchar markupBuf[24];                               
//end build1.0E

    // PP-DCC
    if (PPDCC_ChkIfDccAcq())
    {
        if (glProcInfo.stTranLog.szPan[0]=='4')
        {
            //build1.0E: get markup text and trim the space                    
            if (strlen(glProcInfo.stTranLog.szPPDccMarkupTxt+1))                    
            {                      
              len = strlen(glProcInfo.stTranLog.szPPDccMarkupTxt+1);
                      
              memset(markupBuf, 0, 24);
              memcpy(markupBuf, glProcInfo.stTranLog.szPPDccMarkupTxt+1, len);
                                                                  
              for(i=len-1; i>0; i--)                   
              {              
                if(markupBuf[i] !=0 && markupBuf[i] != ' ')                
                {                
                  if(i != len -1)                  
                  {                  
                    markupBuf[i+1] = 0;                    
                  }
                  
                  break;                  
                }                
              }
            }
            //end build1.0E

            if (ChkIfThermalPrinter())
            {
                PrnSetSmall();

                {              
                    PrnStr("This service is offered by the\n"); 
                    PrnStr("merchant's service provider, with FX\n"); 

                    //build88S 1.0E: align the disclaimer with flexible wholesale rate
                    if (strlen(glProcInfo.stTranLog.szPPDccMarkupTxt+1))
                    {                        
                      memset(claimBuf, 0, 200);        
                      sprintf(claimBuf, "rate at VISA rate plus %s percent. I have a choice of currencies including HKD.", markupBuf);
                                      
                      //format the claimBuf message to fit into the slip print format
                      line = GetPrintLine(claimBuf, 200, outBuf, PRN_ROW_CHAR_MAX);
                      
                      //print the formatted disclaimer
                      for(i=0; i<line; i++)
                      {
                        PrnStr(outBuf[i]);
                        PrnStr("\n");
                      }                      
                    }
                    else                       
                    //end build88S 1.0E
                    {
                      PrnStr("currencies including HKD.\n");
                    }
                }
            }
            else    //Sprocket printer
            {
                // Below can be replaced by logo on sprocket printer.
                PrnSetSmall();

                {               
                    PrnStr("This service is offered by\n");
                    PrnStr("the  merchant's  service\n");
                    PrnStr("provider, with FX rate at VISA\n");        

                    if (strlen(glProcInfo.stTranLog.szPPDccMarkupTxt+1))
                    {
                      //build88S 1.0B: align the disclaimer with flexible wholesale rate
                      memset(claimBuf, 0, 200);        
                      sprintf(claimBuf, "rate plus %s percent. I have a choice of currencies including HKD.", markupBuf);
                       
                      //build1.0E                       
                      //format the claimBuf message to fit into the slip print format
                      line = GetPrintLine(claimBuf, 200, outBuf, PRN_ROW_CHAR_SP_MAX);
                      
                      //print the formatted disclaimer
                      for(i=0; i<line; i++)
                      {
                        PrnStr(outBuf[i]);
                        PrnStr("\n");
                      }
                      //end build1.0E
                    }
                    else
                    {
                        PrnStr("including HKD.");   //build1.0o: update the content of disclaimer for non-Disney version
                    }
                }
            } //end print Sprocket (VISA)        
        } 
        else// MASTERCARD
        {
            if (ChkIfThermalPrinter())
            {
                PrnSetSmall();                                    //
                PrnStr("I have chosen not to use the MasterCard\n");
                PrnStr("currency conversion process and agree that\n");
                PrnStr("I will have no recourse against MasterCard\n");
                PrnStr("concerning the currency conversion or its\n");
                PrnStr("disclosure.\n");
            } 
            else
            {
                  PrnLogo(sLogoMcDcc);
            }
        }
    }
	else if( glProcInfo.stTranLog.ucTranType==INSTALMENT )
	{
		if( ChkIfBea() )
		{
			PrnStr("   I ACCEPT THE T&C OVERLEAF\n");
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
		return -1;
	}
	
	PrnStr("---------------------------\n");
	ConvIssuerName(glSysParam.stIssuerList[ucIndex].szName, szBuff);
	if( ChkIfThermalPrinter() )
	{
		PrnStr("ISSUER  : %-10.10s\n", szBuff);
	}
	else
	{
		PrnStr("  ISSUER  : %-10.10s\n", szBuff);
	}

	PrnTotalInfo(&glPrnTotal);

	return 0;
}

// print total information
void PrnTotalInfo(void *pstInfo)
{
	uchar		szBuff[50], szBaseAmt[20];
	TOTAL_INFO	*pstTotal;

	pstTotal = (TOTAL_INFO *)pstInfo;
	PubAscSub(pstTotal->szSaleAmt, pstTotal->szTipAmt, 12, szBaseAmt);
	App_ConvAmountTran(szBaseAmt, szBuff, 0);
	PrnStr("BASE :%-03d %17s\n", pstTotal->uiSaleCnt, szBuff);

	App_ConvAmountTran(pstTotal->szTipAmt, szBuff, 0);
	PrnStr("TIPS :%-03d %17s\n", pstTotal->uiTipCnt, szBuff);

	App_ConvAmountTran(pstTotal->szSaleAmt, szBuff, 0);
	PrnStr("SALES:%-03d %17s\n", pstTotal->uiSaleCnt, szBuff);

	App_ConvAmountTran(pstTotal->szRefundAmt, szBuff, GA_NEGATIVE);
	PrnStr("REFND:%-03d %17s\n", pstTotal->uiRefundCnt, szBuff);

	App_ConvAmountTran(pstTotal->szVoidSaleAmt, szBuff, GA_NEGATIVE);
	PrnStr("VOIDED SALES :%-03d\n%27s\n", pstTotal->uiVoidSaleCnt, szBuff);

	App_ConvAmountTran(pstTotal->szVoidRefundAmt, szBuff, 0);
	PrnStr("VOIDED REFUND:%-03d\n%27s\n\n", pstTotal->uiVoidRefundCnt, szBuff);
}

int PrnTotalAcq(void)
{
	uchar	ucCnt;
	int		iRet,iResult;
	static int IssuerCount=0;

	PrnInit();
	PrnSetNormal();
	if( ChkIfThermalPrinter() )
	{
		PrnHead_T();
	}
	else
	{
		PrnStr("\n\n\n\n");
		PrnHead(FALSE, TRUE);
	}
	//PrnStep(30);
	PrnStr("  TRANS TOTALS BY CARD\n");
	PrnEngTime();
	PrnStep(5);
	if( ChkIfThermalPrinter() )
	{
	//	PrnStr("TID: %s\nMID: %s\n", glCurAcq.szTermID, glCurAcq.szMerchantID);
	//	PrnStep(15);
		PrnStr("ACQUIRER: %-10.10s\n", glCurAcq.szName);
	}
	else
	{
	//	PrnStr("%14.8s\n%21.15s\n", glCurAcq.szTermID, glCurAcq.szMerchantID);
	//	PrnStep(15);
		PrnStr("  ACQUIRER: %-10.10s\n", glCurAcq.szName);
	}
	for(ucCnt=0; ucCnt<glSysParam.ucIssuerNum; ucCnt++)
	{
		if( glCurAcq.sIssuerKey[ucCnt]!=INV_ISSUER_KEY )
		{
			iResult=PrnTotalIssuer(glCurAcq.sIssuerKey[ucCnt]);
			if (iResult==0)
			{
				PrnStr("   END  OF  TOTAL\n");//2013-10-10 tttttt
				PrnStr("%s", (ChkIfThermalPrinter() ? "\n" : "\f"));//2013-10-3 ttttttttt
				iRet = StartPrinter();
				if( iRet!=0 )
				{
					return iRet;
				}
				PrnInit();
				PrnSetNormal();
		
			}
		}
// 		if( (ucCnt%5)==4 )//printer buffer is not enough to save so many characters 
// 		{	if (IssuerCount>0)
// 			{
// 				//PrnStr("%s", (ChkIfThermalPrinter() ? "\n" : "\f"));//2013-10-3 ttttttttt
// 				iRet = StartPrinter();
// 				if( iRet!=0 )
// 				{
// 					return iRet;
// 				}
// 				PrnInit();
// 				PrnSetNormal();
// 				IssuerCount=0;
// 			}
// 		}
	}
// 	PrnStr("   END  OF  TOTAL\n");
// 	PrnStr("%s", (ChkIfThermalPrinter() ? "\n\n\n\n\n\n" : "\f"));
// 
// 	StartPrinter();
	return 0;
}

//2013-10-3 ttttttttttt
int PrnTotalAll(int flag)
{
	uchar	ucCnt;
	uchar	szBuff[50], szBaseAmt[20];
	int		iRet,iResult;
	static int IssuerCount=0;

	PrnInit();
	PrnSetNormal();
	if( ChkIfThermalPrinter() )
	{
		PrnHead_T();
	}
	else
	{
		PrnStr("\n\n\n\n");
		PrnHead(FALSE, TRUE);
	}
	//PrnStep(30);
	CalcTotal(ACQ_ALL);
	PrnStr("  TRANS TOTALS BY TERMINAL\n");
	PrnEngTime();
	PrnStep(5);	

	PubAscSub(glTransTotal.szSaleAmt, glTransTotal.szTipAmt, 12, szBaseAmt);
	App_ConvAmountTran(szBaseAmt, szBuff, 0);
	PrnStr("BASE :%-03d %17s\n", glTransTotal.uiSaleCnt, szBuff);

	App_ConvAmountTran(glTransTotal.szTipAmt, szBuff, 0);
	PrnStr("TIPS :%-03d %17s\n", glTransTotal.uiTipCnt, szBuff);

	App_ConvAmountTran(glTransTotal.szSaleAmt, szBuff, 0);
	PrnStr("SALES:%-03d %17s\n", glTransTotal.uiSaleCnt, szBuff);

	App_ConvAmountTran(glTransTotal.szRefundAmt, szBuff, GA_NEGATIVE);
	PrnStr("REFND:%-03d %17s\n", glTransTotal.uiRefundCnt, szBuff);

	App_ConvAmountTran(glTransTotal.szVoidSaleAmt, szBuff, GA_NEGATIVE);
	PrnStr("VOIDED SALES :%-03d\n%27s\n", glTransTotal.uiVoidSaleCnt, szBuff);

	App_ConvAmountTran(glTransTotal.szVoidRefundAmt, szBuff, 0);
	PrnStr("VOIDED REFUND:%-03d\n%27s\n\n", glTransTotal.uiVoidRefundCnt, szBuff);

	//PrnEPPTotal();//2016-1-11//2016-2-15 remove AMEX EPP for now
	//2013-10-10
	if (flag == 1)
	{
	  PrnStr("   END OF TERMINAL TOTAL\n");
	}
	PrnStr("%s", (ChkIfThermalPrinter() ? "\n\n\n\n\n\n" : "\f"));
	StartPrinter();
	return 0;
}

int   PrnEPPTotal(void)//2016-1-11 EPP summary printing
{
	uchar   ucAcqIndex, i, szBuff[50], iEPPCnt, iEPPSaleAmt[12+1], iAmtBuff[13];

	memset(iAmtBuff,'0',13);
	memset(iEPPSaleAmt,'0',13);

	CalcTotal(ACQ_ALL);

	iEPPCnt = 0;
	for(ucAcqIndex=0; ucAcqIndex<glSysParam.ucAcqNum; ucAcqIndex++)
	{
		for(i=0; i<glSysParam.ucPlanNum; i++)
		{
			if(glSysParam.stPlanList[i].ucAcqIndex ==ucAcqIndex)
			{
				iEPPCnt += glAcqTotal[ucAcqIndex].uiSaleCnt;
				PubAscAdd(iAmtBuff, glAcqTotal[ucAcqIndex].szSaleAmt, 12, iEPPSaleAmt);
				memcpy(iAmtBuff,iEPPSaleAmt,13);
			}
		}
	}
	if(iEPPCnt == 0)
	{
		return 0;
	}
	//PrnInit();
	//PrnSetNormal();
	PrnStr("\n\n**** AMEX TOTALS BY PLAN ****\n");
	PrnStr("TENURE    TXNS         AMOUNT\n");//max 29
	PrnStr("-----------------------------\n");		
	for(ucAcqIndex=0; ucAcqIndex<glSysParam.ucAcqNum; ucAcqIndex++)
	{
		for(i=0; i<glSysParam.ucPlanNum; i++)
		{
			if(glSysParam.stPlanList[i].ucAcqIndex ==ucAcqIndex)
			{
				App_ConvAmountTran(glAcqTotal[ucAcqIndex].szSaleAmt, szBuff, 0);
				PrnStr("%-03d       %03d %15s\n", glSysParam.stPlanList[i].ucMonths, glAcqTotal[ucAcqIndex].uiSaleCnt,szBuff);
			}
		}
	}
	PrnStr("-----------------------------\n");
	App_ConvAmountTran(iEPPSaleAmt, szBuff, 0);
	PrnStr("EPP TOTAL %03d %15s\n\n\n",iEPPCnt,szBuff);
	//StartPrinter();
	return 0;
}

int PrintSettle(uchar ucPrnFlag)
{
	uchar   ucCnt;
    int     iRet,iResult;
	ulong	ulInvoice;
	static int IssuerCount=0;
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
	PrnSetNormal();
	if( ChkIfThermalPrinter() )
	{
		PrnHead_T();
	}
	else
	{
		PrnStr("\n\n\n\n");
		PrnHead(FALSE, FALSE);
	}
	PrnStr("************************\n");
	PrnStr("BUSINESS DATE - TIME\n");
	PrnEngTime();
	PrnStep(7);
	PrnStr(_T("   SETTLEMENT\n\n"));
	PrnStr("***SUMMARY OF CHARGE***\n");
	PrnStr("*CLOSE BATCH CONFIRMED*\n\n");
	ulInvoice = (ucPrnFlag==PRN_NORMAL) ? glSysCtrl.ulInvoiceNo : glSysCtrl.stRePrnStlInfo.ulSOC[glCurAcq.ucIndex];
	if( ChkIfAmex() )
	{
		PrnStr("S.O.C. NO.: %06lu\n", ulInvoice);
	}
	else
	{
		PrnStr("TRACE NO. : %06lu\n", ulInvoice);
	}
	PrnStr("BATCH NO. : %06lu\n", glCurAcq.ulCurBatchNo);
	PrnStr("ACQUIRER  : %s\n", glCurAcq.szName);
	PrnStr("%s\n", glProcInfo.szSettleMsg);

	PrnTotalInfo(&glTransTotal);
	PrnStr("%s", ChkIfThermalPrinter() ? "\n\n\n\n\n\n\n\n" : "\f");
	PrnStr("\n\n\n\n");//new page head
	for(ucCnt=0; ucCnt<glSysParam.ucIssuerNum; ucCnt++)
	{
		if( glCurAcq.sIssuerKey[ucCnt]!=INV_ISSUER_KEY )
		{
				iResult=PrnTotalIssuer(glCurAcq.sIssuerKey[ucCnt]);
			if (iResult==0)
			{
				IssuerCount++;
			}
		}
		if (IssuerCount>2)//2013.10.22
			{
// 				if (ucCnt == (glSysParam.ucIssuerNum-1))
// 				{
// 					PrnStr(" TRANS TOTAL BY TERMINAL");
// 				}
				PrnStr("%s", ChkIfThermalPrinter() ? "\n\n\n\n\n\n\n\n" : "\f");//change page
				iRet = StartPrinter();
				if( iRet!=0 )
				{
					return iRet;
				}
				PrnInit();
				PrnSetNormal();
				
				IssuerCount=0;
				
			}
	}

	if (IssuerCount!=0)
	{	
 		if (glLastPageOfSettle)
		{
			PrnStr("     <END OF TOTAL>");
		}
		if (ChkIfThermalPrinter())
		{
			PrnStr("\n\n\n\n\n\n\n\n");
		}
		else
		{
			PrnStr("\f");
		}
		IssuerCount = 0;
	}

	return StartPrinter();
}

void PrnEngTime(void)
{
	uchar	szDateTime[14+1], szBuff[30];

	PubGetDateTime(szDateTime);
	Conv2EngTime(szDateTime, szBuff);
	PrnStr("%s\n", szBuff);
}

uchar PrnRateReport(uchar *pPrnData, int iDataLen, uchar ucLastPage)
{
	uchar *pStr, *pLineBegin;
	uchar ucLineLen;

	PrnInit();
	PrnSetNormal();
	PrnStr("\n\n\n");

	pStr = pPrnData;
	pLineBegin = pStr;
	ucLineLen = 0;

	for (; (pStr-pPrnData)<=iDataLen; pStr++)
	{	
		if (*pStr == 0x0a)
		{
			ucLineLen = (uchar)(pStr - pLineBegin);
			if (ucLineLen >= 30)
			{
				ucLineLen = 30;
			}
			
			if (ucLineLen != 0)
			{
				if ((*pLineBegin == 0x1b)||(*(pLineBegin+1) == 0x1b))   //1b 41 0a will cause to do not pring("\f")
				{
					*pLineBegin = 0x1b;
				}
				else
				{
					PrnStr("%.*s", ucLineLen, pLineBegin);
				}
			}
			pLineBegin = pStr;
			ucLineLen = 0;
		}
	}

	if (ChkIfThermalPrinter())
	{
		if (ucLastPage)
		{
			PrnStr("\n\n\n\n\n\n");
		}
	}
	else
	{
		PrnStr("\f");
	}

	return StartPrinter();
}

// 打印终端参数
// Print parameter
void PrintParam(void)
{
	uchar	ucCnt;

	PubShowTitle(TRUE, (uchar *)_T("PRINT PARAMETER"));
	if( PasswordTerm()!=0 )
	{
		return;
	}

#if defined(_P60_S1_) || defined(_S_SERIES_)
	SetOffBase(OffBaseCheckPrint);
#endif

	PubShowTitle(TRUE, (uchar *)_T("PRINT PARAMETER"));

	PrnInit();
	PrnSetNormal();

	PrnStr("\n\n%.23s\n", glSysParam.stEdcInfo.szMerchantName);
	PrnStr("%.23s\n",     glSysParam.stEdcInfo.szMerchantAddr);
	PrnStr("%.23s\n",     glSysParam.stEdcInfo.szMerchantAddr+23);
	PrnStr("INIT TID: %.8s\n", glSysParam.stEdcInfo.szDownLoadTID);
	PrnHexString("EDC OPTION:", glSysParam.stEdcInfo.sOption, 5, TRUE);

	PrnEngTime();
	PrnStr("APP VERSION: %s\n", EDC_VER_PUB);

	if (glSysParam.ucDescNum)
	{
		PrnStr("DESCRIPTION:\n");
		for(ucCnt=0; ucCnt<glSysParam.ucDescNum; ucCnt++)
		{
			PrnStr("   %.2s:%.20s\n", glSysParam.stDescList[ucCnt].szCode,
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
				return;
			}
			PrnInit();
			PrnSetNormal();
			PrnStr("\n\n");
		}
	}
	if( StartPrinter()!=0 )
	{
		return;
	}

	if (!ChkIfEmvEnable())
	{
		return;
	}

#ifdef ENABLE_EMV
	PubShowTitle(TRUE, (uchar *)_T("PRINT PARAMETER"));
	PubDispString(_T("PRN EMV PARA ?"), 4|DISP_LINE_LEFT);
	if (PubYesNo(USER_OPER_TIMEOUT))
	{
		return;
	}

	PrnEmvPara();
#endif
}

int PrnParaAcq(uchar ucAcqIndex)
{
	PubASSERT(glSysParam.stAcqList[ucAcqIndex].ucKey!=INV_ACQ_KEY);
	if( glSysParam.stAcqList[ucAcqIndex].ucKey==INV_ACQ_KEY )
	{
		return 1;
	}

	PrnStr("\n========================\n");
	PrnStr("%.3s  %.10s\n", glSysParam.stAcqList[ucAcqIndex].szNii,
			glSysParam.stAcqList[ucAcqIndex].szName);
	PrnStr("TID: %.8s\n",   glSysParam.stAcqList[ucAcqIndex].szTermID);
	PrnStr("MID: %.15s\n",  glSysParam.stAcqList[ucAcqIndex].szMerchantID);
	PrnHexString("ACQ OPTION:", glSysParam.stAcqList[ucAcqIndex].sOption, 4, TRUE);
	PrnStr("TXN TEL1 %.24s\n", glSysParam.stAcqList[ucAcqIndex].TxnTelNo1);
	PrnStr("TXN TEL2 %.24s\n", glSysParam.stAcqList[ucAcqIndex].TxnTelNo2);
	PrnStr("SET TEL1 %.24s\n", glSysParam.stAcqList[ucAcqIndex].StlTelNo1);
	PrnStr("SET TEL2 %.24s\n", glSysParam.stAcqList[ucAcqIndex].StlTelNo2);

	//tcp/ip
	if (strlen(glSysParam.stAcqList[ucAcqIndex].stTxnTCPIPInfo[0].szIP))
	{
		PrnStr("TCP/IP PARAM:\n");
		PrnStr("TXN 1: %.15s:%.5s\n",	glSysParam.stAcqList[ucAcqIndex].stTxnTCPIPInfo[0].szIP,
										glSysParam.stAcqList[ucAcqIndex].stTxnTCPIPInfo[0].szPort);
	}
	if (strlen(glSysParam.stAcqList[ucAcqIndex].stTxnTCPIPInfo[1].szIP))
	{
		PrnStr("TXN 2: %.15s:%.5s\n",	glSysParam.stAcqList[ucAcqIndex].stTxnTCPIPInfo[1].szIP,
										glSysParam.stAcqList[ucAcqIndex].stTxnTCPIPInfo[1].szPort);
	}

	if (strlen(glSysParam.stAcqList[ucAcqIndex].stTxnGPRSInfo[0].szIP))
	{
		PrnStr("GPRS PARAM:\n");
		PrnStr("TXN 1: %.15s:%.5s\n",	glSysParam.stAcqList[ucAcqIndex].stTxnGPRSInfo[0].szIP,
										glSysParam.stAcqList[ucAcqIndex].stTxnGPRSInfo[0].szPort);
	}
	if (strlen(glSysParam.stAcqList[ucAcqIndex].stTxnGPRSInfo[1].szIP))
	{
		PrnStr("TXN 2: %.15s:%.5s\n",	glSysParam.stAcqList[ucAcqIndex].stTxnGPRSInfo[1].szIP,
										glSysParam.stAcqList[ucAcqIndex].stTxnGPRSInfo[1].szPort);
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
	uchar	ucCnt, szBuff[20];

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
	PubASSERT( ucCnt<glSysParam.ucIssuerNum );
	if( ucCnt>=glSysParam.ucIssuerNum )
	{
		return;
	}

	PrnStr("---ISSUER: %-10.10s---\n", glSysParam.stIssuerList[ucCnt].szName);
	PrnHexString("ISSUER OPTION:", glSysParam.stIssuerList[ucCnt].sOption, 4, TRUE);
	PrnStr("PAN MASK: %02x %02x %02x\n",
			(uchar)(0xFF^glSysParam.stIssuerList[ucCnt].sPanMask[0]),
			(uchar)(0xFF^glSysParam.stIssuerList[ucCnt].sPanMask[1]),
			(uchar)(0xFF^glSysParam.stIssuerList[ucCnt].sPanMask[2]));

	sprintf((char *)szBuff, "%lu", glSysParam.stIssuerList[ucCnt].ulFloorLimit);
	PrnStr("FLOOR LIMIT: %s\n", szBuff);
	PrnStr("ADJUSTMENT PERCENT: %d%%\n", glSysParam.stIssuerList[ucCnt].ucAdjustPercent);

	PrnIssuerOption(glSysParam.stIssuerList[ucCnt].sOption);
	PrnCardTable(ucIssuerKey);
}

void PrnIssuerOption(uchar *psOption)
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
		if( (!stIssuerOptList[ucCnt].ucInverseLogic && ChkOptionExt(psOption, stIssuerOptList[ucCnt].uiOptVal)) ||
			(stIssuerOptList[ucCnt].ucInverseLogic && !ChkOptionExt(psOption, stIssuerOptList[ucCnt].uiOptVal)) )
		{
			PrnStr("    %-16.16s[ on]\n", stIssuerOptList[ucCnt].pText);
		}
		else
		{
			PrnStr("    %-16.16s[off]\n", stIssuerOptList[ucCnt].pText);
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
			PubBcd2Asc0(glSysParam.stCardTable[ucCnt].sPanRangeLow, 5, szBuff);
			szBuff[10] = '~';
			PubBcd2Asc0(glSysParam.stCardTable[ucCnt].sPanRangeHigh, 5, &szBuff[11]);
			PrnStr("%s\n", szBuff);
		}
	}
}

int PrnInstalmentPara(void)
{
	uchar	ucCnt, szBuff[20], szBuff1[50], ucAcqIndex;
	ulong		 ulTemp;
	int				ii;
	if( !ChkIfInstalmentPara() )
	{
		return 1;
	}

	PrnStr("\n========INSTALMENT=======\n");
	for(ucCnt=0; ucCnt<glSysParam.ucPlanNum; ucCnt++)
	{
		PubASSERT( glSysParam.stPlanList[ucCnt].ucIndex!=0xFF );
		PrnStr("%02d.%-7.7s - %dmths\n", ucCnt+1, glSysParam.stPlanList[ucCnt].szName,
				glSysParam.stPlanList[ucCnt].ucMonths);
		ulTemp = glSysParam.stPlanList[ucCnt].ulBottomAmt;
		for (ii=0; ii<glSysParam.stEdcInfo.stLocalCurrency.ucDecimal; ii++)
		{
			ulTemp *= 10;   // the "bottom amount" do not contain decimals
		}
		sprintf((char *)szBuff, "%lu", ulTemp);
		App_ConvAmountLocal(szBuff, szBuff1, 0);
		ucAcqIndex = glSysParam.stPlanList[ucCnt].ucAcqIndex;
		PrnStr("%-10.10s  %s\n", glSysParam.stAcqList[ucAcqIndex].szName, szBuff1);
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

	PrnStr("\n=========EMV PARAMETER=======\n");
	EMVGetParameter(&glEmvParam);
	PrnStr("TERMINAL TYPE: %02X\n", glEmvParam.TerminalType);
	PrnHexString("TERMINAL CAPA:",  glEmvParam.Capability, 3, TRUE);
	PrnHexString("TERM EX-CAPA :",  glEmvParam.ExCapability, 5, TRUE);
	PrnStr("TXN CURR EXP : %02X\n", glEmvParam.TransCurrExp);
	PrnStr("REF CURR EXP : %02X\n", glEmvParam.ReferCurrExp);
	PrnHexString("REF CURR CODE:", glEmvParam.ReferCurrCode, 2, TRUE);
	PrnHexString("COUNTRY CODE :", glEmvParam.CountryCode, 2, TRUE);
	PrnHexString("TXN CURR CODE:", glEmvParam.TransCurrCode, 2, TRUE);
	PrnStr("REF CURR CON : %ld\n", glEmvParam.ReferCurrCon);
	PrnStr("SELECT PSE   : %s\n",  glEmvParam.SurportPSESel ? "YES" : "NO");

	PrnStr("\n\n\n========EMV APP LIST=========\n");
	for(iCnt=0; iCnt<MAX_APP_NUM; iCnt++)
	{
		memset(&stEmvApp, 0, sizeof(EMV_APPLIST));
		iRet = EMVGetApp(iCnt, &stEmvApp);
		if( iRet!=EMV_OK )
		{
			continue;
		}
		PrnHexString("AID:",  stEmvApp.AID, (int)stEmvApp.AidLen, TRUE);
		PrnHexString("VERSION:",  stEmvApp.Version, 2, TRUE);
		PrnStr("SELECT FLAG   : %s MATCH\n", stEmvApp.SelFlag==FULL_MATCH ? "FULL" : "PARTIAL");
		PrnStr("PRIORITY      : %d\n", stEmvApp.Priority);
		PrnStr("TARGET PER    : %d\n", stEmvApp.TargetPer);
		PrnStr("MAX TARGET PER: %d\n", stEmvApp.MaxTargetPer);
		PrnStr("CHECK FLOOR   : %s\n", stEmvApp.FloorLimitCheck ? "YES" : "NO");
		PrnStr("RANDOM SELECT : %s\n", stEmvApp.RandTransSel    ? "YES" : "NO");
		PrnStr("CHECK VELOCITY: %s\n", stEmvApp.VelocityCheck   ? "YES" : "NO");
		PrnStr("FLOOR LIMIT   : %lu\n", stEmvApp.FloorLimit);
		PrnStr("THRESHOLD     : %lu\n", stEmvApp.Threshold);
		PrnHexString("TAC DENIAL :",  stEmvApp.TACDenial,  5, TRUE);
		PrnHexString("TAC ONLINE :",  stEmvApp.TACOnline,  5, TRUE);
		PrnHexString("TAC DEFAULT:",  stEmvApp.TACDefault, 5, TRUE);
		PrnStr("-----------------------------\n");
		if( (iCnt%5)==0 )
		{
			if( StartPrinter()!=0 )
			{
				return 1;
			}
			PrnInit();
			PrnSetNormal();
		}
	}

	PrnStr("\n\n=========EMV CAPK LIST========\n");
	for(iCnt=0; iCnt<MAX_KEY_NUM; iCnt++)
	{
		memset(&stEmvCapk, 0, sizeof(EMV_CAPK));
		iRet = EMVGetCAPK(iCnt, &stEmvCapk);
		if( iRet!=EMV_OK )
		{
			continue;
		}
		PrnHexString("RID:",  stEmvCapk.RID, 5, FALSE);
		PrnStr(" ID: %02X\n",  stEmvCapk.KeyID);
// 		PrnStr("HASH   : %02X\n",  stEmvCapk.HashInd);
// 		PrnStr("ARITH  : %02X\n",  stEmvCapk.ArithInd);
		PrnHexString("EXP DATE:",  stEmvCapk.ExpDate, 3, TRUE);
		PrnStr("MOD LEN: %d ",  (int)(8 * stEmvCapk.ModulLen));
		PrnHexString("EXPONENT:",  stEmvCapk.Exponent, (int)stEmvCapk.ExponentLen, TRUE);
		PrnStr("-----------------------------\n");
		if( (iCnt%5)==0 )
		{
// 			PrnStr("\f");
			if( StartPrinter()!=0 )
			{
				return 1;
			}
			PrnInit();
			PrnSetNormal();
		}
	}
	PrnStr("\f");
	return StartPrinter();
}
#endif


void PrnHexString(char *pszTitle, uchar *psHexStr, int iLen, uchar bNewLine)
{
	int		iCnt;

	PrnStr("%s", pszTitle);
	for(iCnt=0; iCnt<iLen; iCnt++)
	{
		PrnStr(" %02X", psHexStr[iCnt]);
	}
	if (bNewLine)
	{
		PrnStr("\n");
	}
}

// 打印错误提示
// Start-up printer, and show error if any.
int StartPrinter(void)
{
	uchar	ucRet, ucKey;

	if (!ChkIfIrDAPrinter())
	{
		while( 1 )
		{
			DispPrinting();
			PrintOne();
			ucRet = PrnStart();
			if( ucRet==PRN_OK )
			{
				return 0;	// print success!
			}
			ScrClrLine(2, 7);
			DispPrnError(ucRet);
			if( ucRet!=PRN_NO_PAPER )
			{
				PubWaitKey(3);
				break;
			}

			ucKey = PubWaitKey(USER_OPER_TIMEOUT);
			if( ucKey==KEYCANCEL || ucKey==NOKEY )
			{
				ScrClrLine(4, 7);
				PubDispString(_T("PLEASE REPRINT"), 4|DISP_LINE_CENTER);
				PubWaitKey(2);
				break;
			}
		}
	}
	else
	{
#if defined(_P60_S1_) || defined(_S_SERIES_)
		SetOffBase(OffBaseCheckPrint);	//????
#endif
		DispPrinting();
		PrnStart();
		PrintOne();
		while( 1 )
		{
			ucRet = PrnStatus();
			if( ucRet==PRN_OK || ucRet==PRN_CANCEL )
			{
				return (ucRet==PRN_OK ? 0 : 1);
			}
			else if( ucRet==PRN_BUSY )
			{
				DelayMs(500);
				continue;
			}
			ScrClrLine(2, 7);
			DispPrnError(ucRet);
			if( ucRet!=PRN_NO_PAPER )
			{
				PubWaitKey(3);
				break;
			}

			ucKey = PubWaitKey(USER_OPER_TIMEOUT);
			if( ucKey==KEYCANCEL || ucKey==NOKEY )
			{
				break;
			}
			DispPrinting();
			PrnStart();
			PrintOne();
		}
	}
	return ERR_NO_DISP;
}

void DispPrnError(uchar ucErrCode)
{
	switch( ucErrCode )
	{
	case PRN_BUSY:
		PubDispString(_T("PRINTER BUSY"), 4|DISP_LINE_CENTER);
		break;

	case PRN_NO_PAPER:
		PubDispString(_T("OUT OF PAPER"), 4|DISP_LINE_CENTER);
		break;

	case PRN_DATA_ERR:
		PubDispString(_T("PRN DATA ERROR"), 4|DISP_LINE_CENTER);
		break;

	case PRN_ERR:
		PubDispString(_T("PRINTER ERROR"), 4|DISP_LINE_CENTER);
		break;

	case PRN_NO_DOT:
		PubDispString(_T("FONT MISSING"), 4|DISP_LINE_CENTER);
		break;

	case PRN_DATA_BIG:
		PubDispString(_T("PRN OVERFLOW"), 4|DISP_LINE_CENTER);
		break;

	default:
		PubDispString(_T("PRINT FAILED"), 4|DISP_LINE_CENTER);
		break;
	}
	PubBeepErr();
}

/*********
BMP picture maximum :192*255 (width*height)
BMP picture name: LOGO.BMP
************/
int ChkIfPrnLogo(void)
{
	if (fexist(LogoName) && filesize(LogoName)>0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int PrnAcqLogo(void)
{
	uchar bLogo,bTemp, bData,ss;
	uchar sBuf[100],pMem[1024*200];
	int fh,iLcdLines,i,j,iPos,nMaxByte,m;
	BITMAPFILEHEADER tBmFileHeader;
	BITMAPINFOHEADER tBmInfoHeader;
	ulong DataSizePerLine;	
	iPos=0;

	memset(&tBmFileHeader,0,sizeof(BITMAPFILEHEADER));
	memset(&tBmInfoHeader,0,sizeof(BITMAPINFOHEADER));

	fh = open(LogoName, O_RDWR);
	if (fh == -1)
	{
		return 1;
	}

	if (read(fh, (char *)&tBmFileHeader.bfType, 2) == 1)
	{
		close(fh);
		return 1;
	}
	if (read(fh, (char *)&tBmFileHeader.bfSize, 4) == 1)
	{
		close(fh);
		return 1;
	}
	if (read(fh, (char *)&tBmFileHeader.bfReserved1, 2) == 1)
	{
		close(fh);
		return 1;
	}
	if (read(fh, (char *)&tBmFileHeader.bfReserved2, 2) == 1)
	{
		close(fh);
		return 1;
	}
	if (read(fh, (char *)&tBmFileHeader.bfOffBits, 4) == 1)
	{
		close(fh);
		return 1;
	}
	seek(fh,14,SEEK_SET);
	if (read(fh,(uchar *) &tBmInfoHeader, sizeof(BITMAPINFOHEADER)) == 1)
	{
		close(fh);
		return 1;
	}
	if (tBmInfoHeader.biBitCount!= 1)
	{
		close(fh);
		return 1;
	}
	
	if (tBmInfoHeader.biWidth > 576)
	{
		close(fh);
		return 1;
	}

	DataSizePerLine= (tBmInfoHeader.biWidth*tBmInfoHeader.biBitCount+31)/8; //一个扫描行所占的字节数
	DataSizePerLine= DataSizePerLine/4*4; 
	iLcdLines = tBmInfoHeader.biHeight;
	ucLOGO[iPos++] = (uchar)iLcdLines;
	memset(pMem, 0xff, iLcdLines*tBmInfoHeader.biWidth);
	
	for(i=tBmInfoHeader.biHeight-1,j=0; i>=0; i--,j++)
	{
		if (seek(fh, tBmFileHeader.bfOffBits+i*DataSizePerLine, SEEK_SET) == -1)
		{
			close(fh);
			return 1;
		}
		
		if (read(fh, sBuf, DataSizePerLine) == -1)
		{
			close(fh);
			return 1;
		}
		memcpy(pMem+j*DataSizePerLine, sBuf, DataSizePerLine);
	}
	close(fh);

	for(i=0; i<iLcdLines; i++)
	{
		nMaxByte =72;
		ucLOGO[iPos++]=nMaxByte/256;
		ucLOGO[iPos++]=nMaxByte%256;
		for(j=0; j<(int)DataSizePerLine; j++)
		{
			if (j >= 72)
				break;

			bLogo = 0x00;
			bData= *(pMem+i*DataSizePerLine+j);
			for(m=7; m>=0; m--)
			{
				bTemp = (bData>>m)&0x01;
				if (bTemp == 0x00)
					bLogo |= 0x01<<m;
			}

			if (j ==tBmInfoHeader.biWidth/8)
			{
				ss =0x00;
				for(m=0; m<tBmInfoHeader.biWidth%8; m++)
				{
					ss |=1<<(7-m);
				}
				bLogo &=ss;
			}
			ucLOGO[iPos++]=bLogo;
			if (j ==tBmInfoHeader.biWidth/8)
			{
				j ++;
				break;
			}
		}
		if (DataSizePerLine < 72)
		{
			memset(sBuf, 0, sizeof(sBuf));
			for(m=0; j<72; m++,j++)
			{
				ucLOGO[iPos++]=sBuf[m];
			}
		}
	}
	PrnInit();
	//PrnLeftIndent(100);
	PrnLogo(ucLOGO);
	PrnStart();
	PrnInit();
	PrnLeftIndent(0);
	return 0;
}

/*
int PrnTest(void)
{
	PrnInit();
	PrnSetNormal();

	//...

	StartPrinter();
	return 0;
}
*/

// end of file


//build88S 1.0E
//*************************************************************************************************************
//Function: 
//format the input message to fit into the slip print format
//
//Parameter:
//inBuf: the input message to be formatted
//inBufLen: length of the input buffer
//outBuf: formatted message, ready for printing 
//rowMaxChar: max character per print row.
//
//return: total rows
//
//*************************************************************************************************************
uchar GetPrintLine(uchar* inBuf, int inBufLen, uchar outBuf[PRN_ROW_MAX][PRN_ROW_CHAR_MAX+1], uchar rowMaxChar)
{
  int i, spaceIdx, start;
  uchar lineCnt, charCnt;

  memset(outBuf, 0, PRN_ROW_MAX * (PRN_ROW_CHAR_MAX+1));

  charCnt = 0;
  spaceIdx = 0;    
  lineCnt = 0;

  start = 0;

  for(i=0; i < inBufLen; i++)
  {
    if (inBuf[i] == SPACE)
    {
      spaceIdx = i;
    }
    
    charCnt++;

    if(charCnt == rowMaxChar && inBuf[i] != 0 && inBuf[i+1] != 0)
    {      
      if(inBuf[i] != SPACE && inBuf[i+1] != SPACE) //last character != SPACE, next to last character != SPACE, message length < 30
      {
        memcpy(outBuf[lineCnt], inBuf + start, spaceIdx - start + 1);
        charCnt = i - spaceIdx;
        start = spaceIdx + 1;
      }
      else if(inBuf[i] != SPACE && inBuf[i+1] == SPACE) //last character belongs to a word, next to last character == SPACE, message length = 30
      {
        memcpy(outBuf[lineCnt], inBuf + start, i - start + 1); 
        charCnt = 0;
        start = i + 2;      //ignore the space of next prn message
      }
      else
      {
        memcpy(outBuf[lineCnt], inBuf + start, i - start + 1);
        charCnt = 0;
        start = i + 1;        
      }
      
      lineCnt++;
    }
    	
    
    //end of buffer
    if(inBuf[i] == 0)
    {
      if(start != i)
      {
        memcpy(outBuf[lineCnt], inBuf + start, i - start + 1);
        lineCnt++;
      }

      break;
    }
  }

  return lineCnt;
}

static void s_PrnCardNum(void)
{
	uchar	szBuff[64];

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
		PrnStr("CARD NO: %s S\n", szBuff);
	}
	else if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	{
		PrnStr("CARD NO: %s C\n", szBuff);
	}
	else if( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
			 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
		PrnStr("CARD NO: %s F\n", szBuff);
	}
	else if( glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS )
	{
		PrnStr("CARD NO: %s T\n", szBuff);
	}
	else
	{
		PrnStr("CARD NO: %s M\n", szBuff);
	}
}

static void s_PrnCardTypeAndExpDate(void)
{
	uchar	szIssuerName[10+1];

	ConvIssuerName(glCurIssuer.szName, szIssuerName);
	PrnStr("CARD TYPE: %-10.10s    ", szIssuerName);

	if( ChkIssuerOption(ISSUER_EN_EXPIRY) )
	{
		if( ChkIssuerOption(ISSUER_MASK_EXPIRY) )
		{
			PrnStr("**/**");
		}
		else
		{
			PrnStr("%2.2s/%2.2s", &glProcInfo.stTranLog.szExpDate[2],
					glProcInfo.stTranLog.szExpDate);
		}
	}

    PrnStr("\n");
}

static void s_PrnHolder(char *pszHeader)
{
    PrnStr("%s%s\n", pszHeader, glProcInfo.stTranLog.szHolderName);
}
