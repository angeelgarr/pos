#include "global.h"




#ifdef ENABLE_EMV




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
typedef void (*SaveTLVData)(uint uiTag, uchar *psData, int iDataLen);

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
static int  SetAmexDE55(DE55Tag *pstList, uchar *psOutData, int *piOutLen);
static int  AppendStdTagList(DE55Tag *pstList, ushort uiTag, uchar ucOption, uchar ucMaxLen);
static int  RemoveFromTagList(DE55Tag *pstList, ushort uiTag);
static int  SetStdDE55(uchar bForUpLoad, DE55Tag *pstList, uchar *psOutData, int *piOutLen);
static int  SetStdDE56(DE55Tag *pstList, uchar *psOutData, int *piOutLen);
static int  GetTLVItem(uchar **ppsTLVString, int iMaxLen, SaveTLVData pfSaveData, uchar bExpandAll);
static int  GetSpecTLVItem(uchar *psTLVString, int iMaxLen, uint uiSearchTag, uchar *psOutTLV, ushort *puiOutLen);
static int  IsConstructedTag(uint uiTag);
static void SaveRspICCData(uint uiTag, uchar *psData, int iDataLen);
static void BuildTLVString(ushort uiEmvTag, uchar *psData, int iLength, uchar **ppsOutData);
static void SaveEmvData(void);
static void AdjustIssuerScript(void);
static void SaveScriptData(uint uiTag, uchar *psData, int iDataLen);
static void PackTLVData(uint uiTag, uchar *psData, uint uiDataLen, uchar *psOutData, int *piOutLen);
static void PackTLVHead(uint uiTag, uint uiDataLen, uchar *psOutData, int *piOutLen);
static int  CalcTLVTotalLen(uint uiTag, uint uiDataLen);
static void PackScriptData(void);
static void SaveTVRTSI(uchar bBeforeOnline);
static int  GetAmexDE55(uchar *psAuthData,int *pulAuthDatalen,uchar *psScript,int *pulScriptLen);
/********************** Internal variables declaration *********************/
static uchar sAuthData[16];			// 发卡方认证数据
static uchar sIssuerScript[300];	// 发卡方脚本
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
static ScriptInfo	sgScriptInfo;
static uchar		sgScriptBak[300];
static int			sgCurScript, bHaveScript, sgScriptBakLen;


// AMEX format bit 55
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
static DE55Tag sgStdEmvTagList[] =
{
	{0x5F2A, DE55_MUST_SET, 0},
	{0x5F34, DE55_OPT_SET, 1}, // notice it's limited to L=1
	{0x82,   DE55_MUST_SET, 0},
	{0x84,   DE55_MUST_SET, 0},
	{0x95,   DE55_MUST_SET, 0},
	{0x9A,   DE55_MUST_SET, 0},
	{0x9C,   DE55_MUST_SET, 0},
	{0x9F02, DE55_MUST_SET, 0},
//	{0x9F03, DE55_MUST_SET, 0},  //Build 1.00.0108a
	{0x9F09, DE55_MUST_SET, 0},
	{0x9F10, DE55_MUST_SET, 0},
	{0x9F1A, DE55_MUST_SET, 0},
	{0x9F1E, DE55_MUST_SET, 0},
	{0x9F26, DE55_MUST_SET, 0},
	{0x9F27, DE55_MUST_SET, 0},
	{0x9F33, DE55_MUST_SET, 0},
	{0x9F34, DE55_OPT_SET, 0},
	{0x9F35, DE55_MUST_SET, 0},
	{0x9F36, DE55_MUST_SET, 0},
	{0x9F37, DE55_MUST_SET, 0},
	{0x9F41, DE55_MUST_SET, 0},

	//{0x57,   DE55_OPT_SET, 0},
	//{0x9F15, DE55_OPT_SET, 0},
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
	stExtMckParam.aucTermAIP[0] = 0x08;
	stExtMckParam.ucBypassAllFlg = 1;
	stMckParam.pvoid = &stExtMckParam;
	EMVSetMCKParam(&stMckParam);
}

// Set to default EMV parameter, since it may be changed during last transaction.
void InitTransEMVCfg(void)
{
	AppSetMckParam(TRUE);	//在交易处理前调用接口函数  EMVSetMCKParam 设置是否使用批数据采集

	EMVGetParameter(&glEmvParam);
	glEmvParam.ForceOnline   = 0;
	memcpy(glEmvParam.CountryCode,   glSysParam.stEdcInfo.stLocalCurrency.sCountryCode, 2);
	memcpy(glEmvParam.TransCurrCode, glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, 2);
	memcpy(glEmvParam.ReferCurrCode, glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, 2);
	glEmvParam.TransCurrExp = glSysParam.stEdcInfo.stLocalCurrency.ucDecimal;
	glEmvParam.ReferCurrExp = glSysParam.stEdcInfo.stLocalCurrency.ucDecimal;
	EMVSetParameter(&glEmvParam);
}

//显示应用列表，等待持卡人选择
//如果只有一个应用，则EMV内核不会调用该函数
int cEMVWaitAppSel(int TryCnt, EMV_APPLIST List[], int AppNum)
{
	int			iRet, iCnt;
	MenuItem	stAppMenu[MAX_APP_NUM+1];

	if( TryCnt!=0 )
	{
		ScrClrLine(2, 7);
		PubDispString(_T("NOT ACCEPT"),    3|DISP_LINE_LEFT);
		PubDispString(_T("PLS TRY AGAIN"), 5|DISP_LINE_LEFT);
		PubWaitKey(3);
	}

	PubASSERT( AppNum<=MAX_APP_NUM );
	memset(&stAppMenu[0], 0, sizeof(stAppMenu));
	for(iCnt=0; iCnt<AppNum && iCnt<MAX_APP_NUM; iCnt++)
	{
		stAppMenu[iCnt].bVisible = TRUE;
		sprintf((char *)stAppMenu[iCnt].szMenuName, "%.16s", List[iCnt].AppName);
	}

	iRet = PubGetMenu((uchar *)_T("Select App"), &stAppMenu[0], MENU_CFONT, USER_OPER_TIMEOUT);
	if( iRet<0 )
	{
		return EMV_USER_CANCEL;
	}

	return iRet;
}

//交易金额输入函数
//应用应该根据实际情况修改
// Callback function required by EMV core.
// developer customize
int cEMVInputAmount(ulong *AuthAmt, ulong *CashBackAmt)
{
	uchar	szTotalAmt[20];
	uchar   szBuff[32];

#ifdef APP_DEBUG_RICHARD
	PubDebugTx("FILE_%s,func:%s,LineNo:%d,%s",__FILE__,__FUNCTION__, __LINE__,"jeff");
#endif
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

	return EMV_OK;
}

//处理DOL的过程中，EMV库遇到不识别的TAG时会调用该回调函数，要求应用程序处理
//如果应用程序无法处理，则直接返回-1，提供该函数只为解决一些不符合EMV的特殊
//应用程序的要求，一般情况下返回-1即可
// Callback function required by EMV core.
// When processing DOL, if there is a tag that EMV core doesn't know about, core will call this function.
// developer should offer processing for proprietary tag.
// if really unable to, just return -1
int cEMVUnknowTLVData(ushort iTag, uchar *psDat, int iDataLen)
{
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

	default:
		return -1;
	}

	return EMV_OK;
}

#if defined(_P60_S1_) || defined(_P80_)
// Generates random hex string
static void GetRandHexStr(uchar *psOut, uchar ucOutLen)
{
    int ii;
    uchar   sBuff[16];

    GetTime(sBuff);
    ii = sBuff[3]*65536 + sBuff[4]*256 + sBuff[5];
    srand(ii);

    for (ii=0; ii<ucOutLen; ii++)
    {
        psOut[ii] = (uchar)rand();
    }
}
#endif

//等待用户输入卡密码
//下边是处理代码，应用应该根据实际情况修改
// Callback function required by EMV core.
// Wait holder enter PIN.
// developer customized.
int cEMVGetHolderPwd(int iTryFlag, int iRemainCnt, uchar *pszPlainPin)
{
	int		iResult;
	uchar	ucRet, szBuff[30], szAmount[15];
	uchar	sPinBlock[8];
#if defined(_P60_S1_) || defined(_P80_)
    int     iRealLen, iByteLen, ii; 
    uchar	sTmpMKey[8], sTmpEncWKey[8], sTmpPlainWKey[8];
#endif

#ifdef APP_DEBUG_RICHARD
	PubDebugTx("FILE_%s,func:%s,LineNo:%d,%s",__FILE__,__FUNCTION__, __LINE__,"jeff");
#endif
	// 联机PIN处理
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

	// 脱机pin处理(明文/密文)
	// Offline plain/enciphered PIN processing below

	ScrClrLine(2, 7);
	if( iRemainCnt==1 )
	{
		PubDispString(_T("LAST ENTER PIN"), 4|DISP_LINE_LEFT);
		DelayMs(1500);
	}

	PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szAmount);
	ScrClrLine(2, 7);
	if( iTryFlag==0 )
	{
		DispAmount(2, szAmount);
	}
	else
	{
		PubDispString(_T("PIN ERR, RETRY"), 2|DISP_LINE_LEFT);
	}

ENTERPIN:
	PubDispString(_T("PLS ENTER PIN"), 4|DISP_LINE_LEFT);

	if (ChkTermPEDMode(PED_INT_PCI))
	{

		if(ChkIssuerOption(ISSUER_FORBID_EMVPIN_BYPASS) && ChkIfAmex())	// bypass not permitted
		{
			ScrPrint(0,7,ASCII|REVER,(char *)"by-pass not permitted");
		}

		// Offline PIN, done by core itself since EMV core V25_T1. Application only needs to display prompt message.
        // In this mode, cEMVGetHolderPwd() will be called twice. the first time is to display message to user,
        // then back to kernel and wait PIN. afterwards kernel call this again and inform the process result.
        if (pszPlainPin[0]==EMV_PED_TIMEOUT)
        {
            // EMV core has processed PIN entry and it's timeout
        	ScrClrLine(2, 7);
            PubDispString(_T("TIME OUT"), 6|DISP_LINE_LEFT);
            PubBeepErr();
            PubWaitKey(3);
            return EMV_TIME_OUT;
        }
		else if (pszPlainPin[0]==EMV_PED_WAIT)
        {
            // API is called too frequently
            DelayMs(1000);
			ScrClrLine(2, 7);
            ScrGotoxy(32, 6);
			return EMV_OK;
        }
        else if (pszPlainPin[0]==EMV_PED_FAIL)
        {
            // EMV core has processed PIN entry and PED failed.
			ScrClrLine(2, 7);
            PubDispString(_T("PED ERROR"), 6|DISP_LINE_LEFT);
            PubBeepErr();
            PubWaitKey(3);
            return EMV_NO_PINPAD;
        }
        else
        {
            // EMV PIN not processed yet. So just display.
            ScrGotoxy(32, 6);
			return EMV_OK;
        }
	}

	else if (ChkTermPEDMode(PED_INT_PXX))
	{
#if defined(_P60_S1_) || defined(_P80_)
        // Generate random master key
        memset(sTmpMKey, 0, sizeof(sTmpMKey));
        GetRandHexStr(sTmpMKey, 8);
        if (PEDWriteMKey(GENERAL_MKEY_ID, 0x01, sTmpMKey)!=0)
        {
            return EMV_NO_PINPAD;
        }
        // Generate random work key
        memset(sTmpEncWKey, 0, sizeof(sTmpEncWKey));
        GetRandHexStr(sTmpEncWKey, 8);
	    if (PEDWriteWKey(GENERAL_MKEY_ID, GENERAL_WKEY_ID, 0x81, sTmpEncWKey)!=0)
        {
            return EMV_NO_PINPAD;
        }

        // input
		memset(szBuff, 0, sizeof(szBuff));
		ScrGotoxy(40, 6);
        ucRet = PEDGetPwd(GENERAL_WKEY_ID, 4, 6, "1234567890123456", szBuff, 0);   // Use virtual PAN "1234567890123456"
	    if ((ucRet==0x0A))
	    {
            return EMV_NO_PASSWORD; // Bypass
	    }
        else if ((ucRet==0x06) || (ucRet==0xC6) || (ucRet==0x07))
		{
			return EMV_USER_CANCEL; // Cancel or timeout
		}
        else if (ucRet!=0)
        {
		    return EMV_NO_PINPAD;   // PED error
        }

        // decrypt to plain text
        // get plaintext PIN
	    des(sTmpEncWKey, sTmpPlainWKey, sTmpMKey, 0);
	    des(szBuff, szBuff+15, sTmpPlainWKey, 0);
        // virtual PAN block
	    memcpy(szBuff, "\x00\x00\x56\x78\x90\x12\x34\x56", 8);
        // Get plaintext PINBlock
        PubXor(szBuff, szBuff+15, 8, szBuff);
	    iRealLen= szBuff[0] & 0x0F;
        iByteLen = iRealLen/2;
	    if (iRealLen % 2)
        {
            iByteLen += 1;
        }
		// Get pure PIN string
        PubBcd2Asc0(szBuff+1, iByteLen, pszPlainPin);
        PubTrimTailChars(pszPlainPin, 'F');
		return EMV_OK;
#else
		return EMV_USER_CANCEL;
#endif
	}
	else if (ChkTermPEDMode(PED_EXT_PP))
	{
#ifndef _P60_S1_
		PubDispString(_T("PLS USE PINPAD"), 6|DISP_LINE_LEFT);
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
		ucRet = PPEmvGetPwd(4, 12, sPinBlock);	// ???? 这是什么回事
		switch( ucRet )
		{
		case 0x00:
			// PinBlock Format: C L P P P P P/F P/F P/F P/F P/F P/F P/F P/F F F
			// C = 0x02, L = length of PIN, P = PIN digits, F = 0x0F
			PubBcd2Asc0(sPinBlock, 8, szBuff);
			PubTrimTailChars(szBuff, 'F');
			sprintf((char *)pszPlainPin, "%.12s", &szBuff[2]);
			return EMV_OK;
			
		case 0x06:
		case 0x07:
		case 0xC6:
			return EMV_USER_CANCEL;
			
		case 0x0A:
			if(ChkIssuerOption(ISSUER_FORBID_EMVPIN_BYPASS) && ChkIfAmex())
			{
				PPScrCls();
				PPScrPrint(1,0," NOT PERMITTED");
				PPBeep();
				
				ScrClrLine(2, 7);
				PubDispString(_T("NOT PERMITTED"), 4|DISP_LINE_CENTER);
				Beef(6, 200);
				PubWaitKey(5);
				goto ENTERPIN;
			}
			else
			{
				return EMV_NO_PASSWORD;
			}
			
		default:
			return EMV_NO_PINPAD;
		}
#else
		return EMV_USER_CANCEL;
#endif
	} 
	else	// PED_EXT_PCI
	{
		// !!!! extern PCI, to be implemented.
		ScrClrLine(2, 7);
		PubDispString("EXT PCI PINPAD  ", 4|DISP_LINE_LEFT);
		PubDispString("NOT IMPLEMENTED.", 6|DISP_LINE_LEFT);
		PubWaitKey(30);
		return ERR_NO_DISP;
	}
}

/*语音参考处理
  如果不支持，如果是卡片发起的参考，
  则可根据发卡行要求直接返回REFER_DENIAL或REFER_ONLINE,
  一般情况下不应该直接返回REFER_APPROVE(除非发卡行要求这么做)

  如果不支持，如果是发卡行(主机)发起的参考，
  则可根据发卡行要求直接返回REFER_DENIAL
  一般情况下不应该直接返回REFER_APPROVE(除非发卡行要求这么做)

  下边是参考的处理代码，供参考
 */
// Callback function required by EMV core.
// Voice refferal.
// If not support, return REFER_DENIAL.
int cEMVReferProc(void)
{
	return REFER_DENIAL;
}

//通知处理(如果不支持，直接返回就可以了)
// Callback function required by EMV core.
// TC advise after EMV transaction. If not support, immediately return.
void cEMVAdviceProc(void)
{
/*
	脱机通知的处理：
	通过函数EMVGetTLVData()获得通知数据包需要的数据，存贮到交易日志中，
	然后在交易结算时，再联机发送到主机。
	需要注意的是：通知数据包的任何数据(比如金额等)不可以用于交易对帐。

	联机通知的处理：
	(1)拨号连接主机。
	(2)通过函数EMVGetTLVData()获得通知数据包需要的数据，再发送到主机。
	需要注意的是：联机通知方式在我们的POS机中应该不会使用。
*/
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
	uchar   sBuffer[32];  //Gillian debug
    int ii;
	int nRet;
	// initialize output parameters
	*psRspCode      = 0;
	*piAuthCodeLen  = 0;
	*piAuthDataLen  = 0;
	*piScriptLen    = 0;
	SaveTVRTSI(TRUE);
	glProcInfo.bIsFirstGAC = FALSE;

#ifdef APP_DEBUG_RICHARD  //added by jeff_xiehuan 20170322 for test
	PubDebugTx("FILE_%s,func:%s,LineNo:%d,%s",__FILE__,__FUNCTION__, __LINE__,"jeff");
#endif
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
		if( !(ChkIfDahOrBCM() || ChkIfCiti() || ChkIfScb()) && ChkIfAcqNeedDE56() )
		{
			if ( glProcInfo.stTranLog.ucTranType!=AUTH && glProcInfo.stTranLog.ucTranType!=PREAUTH)
			{
				iLength = glSysCtrl.stField56[glCurAcq.ucIndex].uiLength;
				PubASSERT(iLength<LEN_ICC_DATA);
				if( iLength>0 )
				{
					memcpy(&glSendPack.sICCData2[2], glSysCtrl.stField56[glCurAcq.ucIndex].sData, iLength);
				}
				else if (ChkIfFubon())
				{
					SetDE56(&glSendPack.sICCData2[2], &iLength);
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

// 	if(ChkIfCiti())
// 	{
// 		iRet=OfflineSend(OFFSEND_TC);
// 		if(iRet!=0)
// 		{
// 			glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
// 			return ONLINE_FAILED;
// 		}
// 		//脱机上送后，流水号会增加，所以当前交易的流水号需用增加后的流水号 build 1.00.0108b
// 		sprintf((char *)glSendPack.szSTAN, "%06lu", glSysCtrl.ulSTAN);  //
// 		glProcInfo.stTranLog.ulSTAN = glSysCtrl.ulSTAN;
// 	}

	iRetryPIN = 0;
	while( 1 )
	{
		//if (ChkIfAmex() || ChkCurAcqName("AMEX", FALSE))//2015-1-23 remove: this cause Amex trace number not consistent
		//{
		//	GetNewInvoiceNo();
		//}

		iRet = SendRecvPacket();
#ifdef APP_DEBUG_RICHARD  //added by jeff_xiehuan 20170322 for test
	PubDebugTx("FILE_%s,func:%s,LineNo:%d,iRet=%d",__FILE__,__FUNCTION__, __LINE__,iRet);
#endif
		if( iRet!=0 )
		{
			glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;
			return ONLINE_FAILED;
		}
		if( memcmp(glRecvPack.szRspCode, "55", 2)!=0 || ++iRetryPIN>3 || !ChkIfNeedPIN() )
		{
			break;
		}

		// 重新输入PIN
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

// 	if(!ChkIfAmex()&&memcmp(glRecvPack.szRspCode,"08",LEN_RSP_CODE)==0)//squall build126,m-tips new case08 have to approve
// 	{
// 		memcpy(glRecvPack.szRspCode,"00",LEN_RSP_CODE);
// 	}
	// set response code
	memcpy(psRspCode,  glRecvPack.szRspCode,  LEN_RSP_CODE);
	glProcInfo.ucOnlineStatus = ST_ONLINE_APPV;

	// get response issuer data
	sgAuthDataLen = sgScriptLen = 0;


	if (ChkIfAmex())
	{  // amex field55 疭矪瞶
		memset(sAuthData,0,sizeof(sAuthData));
		memset(sIssuerScript,0,sizeof(sIssuerScript));
		iRet = GetAmexDE55(sAuthData, &sgAuthDataLen,sIssuerScript, &sgScriptLen);
		////2015-1-8 ttt
		//ScrCls();
		///*ScrPrint(0,0,0, "[%d]AuthData: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",iRet, sAuthData[0],sAuthData[1]
		//,sAuthData[2],sAuthData[3],sAuthData[4],sAuthData[5],sAuthData[6],sAuthData[7],sAuthData[8],sAuthData[9],sAuthData[10],sAuthData[11],sAuthData[12]
		//,sAuthData[13],sAuthData[14],sAuthData[15]);*/
		//ScrPrint(0,0,0, "AuthData: %02x %02x",sAuthData[8],sAuthData[9]);
		//getkey();
		if (iRet!=0)
		{
			return iRet;
		}		
		memcpy(psRspCode,sAuthData+8,2);//2015-1-8 AMEX request to use tag 91 data as ARC (Authorisation Response Code)

		memcpy(psAuthData, sAuthData, sgAuthDataLen);
		*piAuthDataLen = sgAuthDataLen;
		memcpy(psScript, sIssuerScript, sgScriptLen);
		*piScriptLen = sgScriptLen;
	}
	else
	{    	
		PubChar2Long(glRecvPack.sICCData, 2, &ulICCDataLen);
		psICCData = &glRecvPack.sICCData[2];
		for(psTemp=psICCData; psTemp<psICCData+ulICCDataLen; )
		{
			iRet = GetTLVItem(&psTemp, psICCData+ulICCDataLen-psTemp, SaveRspICCData, TRUE);
			if( iRet<0 )
			{	// if analyze response ICC data failed, return fail
				glProcInfo.ucOnlineStatus = ST_ONLINE_FAIL;

				return ONLINE_FAILED;
			}
		}
		memcpy(psAuthData, sAuthData, sgAuthDataLen);//2014-12-2
		*piAuthDataLen = sgAuthDataLen;

		AdjustIssuerScript();
		memcpy(psScript, sIssuerScript, sgScriptLen);
		*piScriptLen = sgScriptLen;
	}

	

	if( memcmp(glRecvPack.szRspCode, "00", LEN_RSP_CODE)!=0)//squall,build126
	{
		return ONLINE_DENIAL;
	}

	// set authorize code only if txn approved
	memcpy(psAuthCode, glRecvPack.szAuthCode, LEN_AUTH_CODE);
	*piAuthCodeLen = strlen((char *)glRecvPack.szAuthCode);

	return ONLINE_APPROVE;
}

//如果不需要提示密码验证成功，则直接返回就可以了
// Callback function required by EMV core.
// Display "EMV PIN OK" info. (plaintext/enciphered PIN)
void cEMVVerifyPINOK(void)
{
	glProcInfo.stTranLog.uiEntryMode |= MODE_OFF_PIN;
	ScrClrLine(2, 7);
	PubDispString(_T("PIN VERIFY OK"), 4|DISP_LINE_LEFT);
	PubWaitKey(1);
}

//持卡人认证例程
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
	uchar	ucSW1, ucSW2;
	uchar	sBuffer[32];

	// 根据需要设置是否强制联机
	// decide whether need force online
	EMVGetParameter(&glEmvParam);
	glEmvParam.ForceOnline = (glProcInfo.stTranLog.ucTranType!=SALE ? 1 : 0);
	EMVSetParameter(&glEmvParam);

	// clear last EMV status
	memset(&glEmvStatus, 0, sizeof(EMV_STATUS));
	SaveEmvStatus();

#if defined(_S_SERIES_) || defined(_SP30_)
	if (ChkTermPEDMode(PED_INT_PCI))
	{
		iRet = EMVSetPCIModeParam(1, (uchar *)"0,4,5,6,7,8", 120000);
	}
#endif

	// 处理EMV交易
	// Process EMV transaction.
	iRet = EMVProcTrans();
	SaveTVRTSI(FALSE);
	if( iRet==EMV_TIME_OUT || iRet==EMV_USER_CANCEL )
	{
		return ERR_USERCANCEL;
	}
	if( (glProcInfo.ucOnlineStatus==ST_ONLINE_APPV) && memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)==0)
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
		EMVGetICCStatus(&ucSW1, &ucSW2);
		if( glProcInfo.bIsFirstGAC && ucSW1==0x69 && ucSW2==0x85 &&
			/*glProcInfo.stTranLog.szPan[0]=='5'*/ChkIfMC(glProcInfo.stTranLog.szPan) )//2016-2-15
		{	// for TIP fallback when 1st GAC return 6985
			return ERR_NEED_FALLBACK;
		}

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

	if (ChkIfAmex())
	{
		iRet = EMVGetTLVData(0x9B, sBuffer, &iLength);
		if ( (iRet==EMV_OK) && (iLength==2) )
		{
			if ( sBuffer[0] & 0x08 )	// terminal risk management has performed
			{
				iRet = EMVGetTLVData(0x95, sBuffer, &iLength);
				if ( (iRet==EMV_OK) && (iLength==5) )	// TVR valid
				{
					if ( (glProcInfo.ucOnlineStatus==ST_OFFLINE) && !(sBuffer[3] & 0x80) )	// not online, not exceed floor limit
					{
						glProcInfo.stTranLog.uiStatus |= TS_FLOOR_LIMIT;
					}
				}
			}
		}
	}

	// 交易成功, 保存EMV交易数据
	// transaction approved. save EMV data
	if (ChkIfWLB())
	{
		iWLBfd = open("WLBBit63.dat", O_RDWR);
		write(iWLBfd, "11", 2);
		ucWLBBuf[0] = 1;
		ucWLBBuf[1] = 1;
		close(iWLBfd);
	}

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
int SetAmexDE55(DE55Tag *pstList, uchar *psOutData, int *piOutLen)
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
		if( iRet!=EMV_OK&&iRet!=EMV_NO_DATA)
		{
			return ERR_TRAN_FAIL;
		}
		if (iRet == EMV_NO_DATA)//squall add here for empty TAG Value 
		{
			iLength = pstList[iCnt].ucLen;
			memset(sBuff,0,iLength);
		}
		////2014-11-6 ttt
		//if(pstList[iCnt].uiEmvTag == 0x9F06)
		//{
		//ScrCls();
		//ScrPrint(0,0,0,"1_uiEmvTag: %0x",pstList[iCnt].uiEmvTag);
		//ScrPrint(0,2,0,"2_value: %02x%02x%02x%02x%02x%02x%02x%02x%02x",sBuff[0],sBuff[1],sBuff[2],sBuff[3],sBuff[4],sBuff[5],sBuff[6],sBuff[7],sBuff[8]);
		//getkey();
		//}
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

#ifdef WIN32
static void WIN32_ShowAllTags(DE55Tag *pstList)
{
    int ii = 0;
    uchar szBuff[256];

    ii = 0;
    memset(szBuff, 0, sizeof(szBuff));
    while (pstList[ii].uiEmvTag!=0)
    {
        sprintf(szBuff+strlen(szBuff), "%02X%02X\r\n", (uchar)(pstList[ii].uiEmvTag/256), (uchar)(pstList[ii].uiEmvTag % 256));
        ii++;
    }
    DelayMs(1);
}
#endif

// this function will not check the overflow risk of array pointed by pstList.
int AppendStdTagList(DE55Tag *pstList, ushort uiTag, uchar ucOption, uchar ucMaxLen)
{
	int	iCnt;
    DE55Tag stTag;

    // Append to the end of list
    /*
	iCnt = 0;
	while (pstList[iCnt].uiEmvTag!=0)
	{
		iCnt++;
	}
	pstList[iCnt].uiEmvTag = uiTag;
	pstList[iCnt].ucOption = ucOption;
	pstList[iCnt].ucLen    = ucMaxLen;
	pstList[iCnt+1].uiEmvTag = 0;
	pstList[iCnt+1].ucOption = 0;
	pstList[iCnt+1].ucLen    = 0;
	return 0;
    */

    // insert by value
    stTag.uiEmvTag = uiTag;
    stTag.ucLen    = ucMaxLen;
    stTag.ucOption = ucOption;

    // go to the end of list
    iCnt = 0;
    while (pstList[iCnt].uiEmvTag!=0)
    {
        iCnt++;
    }

    pstList[iCnt+1].uiEmvTag = 0;
    pstList[iCnt+1].ucLen    = 0;
    pstList[iCnt+1].ucOption = 0;

    while (iCnt>0)
    {
        if (stTag.uiEmvTag<pstList[iCnt-1].uiEmvTag)
        {
            pstList[iCnt] = pstList[iCnt-1];
            iCnt--;
        }
        else
        {
            break;
        }
    }
    pstList[iCnt] = stTag;
#ifdef WIN32
    WIN32_ShowAllTags(pstList);
#endif
    return 0;
}

int RemoveFromTagList(DE55Tag *pstList, ushort uiTag)
{
	int	iCnt;

	for (iCnt=0; pstList[iCnt].uiEmvTag!=0; iCnt++)
	{
		if (pstList[iCnt].uiEmvTag==uiTag)
		{
			break;
		}
	}
	if (pstList[iCnt].uiEmvTag==0)
	{
		return -1;
	}

	for (; pstList[iCnt].uiEmvTag!=0; iCnt++)
	{
		pstList[iCnt] = pstList[iCnt+1];
	}

	return 0;
}

// set ADVT/TIP bit 55
int SetStdDE55(uchar bForUpLoad, DE55Tag *pstList, uchar *psOutData, int *piOutLen)
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
        else if (PPDCC_ChkIfDccAcq())
        {
        }
		else
		{
			memcpy(psTemp, "\xDF\x5A\x01\x01", 4);
			psTemp += 4;
			if (LastRecordIsFallback())
			{
				memcpy(psTemp, "\xDF\x39\x01\x02", 4);
			}
			else
			{
				memcpy(psTemp, "\xDF\x39\x01\x01", 4);
			}
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
			if (ChkIfDahOrBCM() || ChkIfShanghaiCB()||ChkIfICBC_MACAU() || ChkIfBnu() || ChkIfWLB())
			{
				astLocalTaglist[iCnt].ucOption=DE55_OPT_SET;
			}
		}
		
		//-----------------------------------------------------------
		// Customize tag list according to different acquirer
		if (ChkIfCiti())
		{
			RemoveFromTagList(astLocalTaglist, 0x5F34);
			//squall 2013.12.10 synchronized with P60 CITI bank
			AppendStdTagList(astLocalTaglist, 0x57, DE55_OPT_SET, 0);
			
			//  Build 1.00.0108 remove 0x9A for CITI
			//	RemoveFromTagList(astLocalTaglist, 0x9A);
		}
        if (ChkIfWingHang())
        {
			RemoveFromTagList(astLocalTaglist, 0x5F34);
            AppendStdTagList(astLocalTaglist, 0x5F34, DE55_OPT_SET, 1);
        }
		if (ChkIfDCC_CITI())//build130 squall CITI DCC need tsi
		{
			AppendStdTagList(astLocalTaglist, 0x9B, DE55_MUST_SET, 0);
		}

		if ( !ChkIfDahOrBCM() && !ChkIfScb() && !PPDCC_ChkIfDccAcq() )
		{
			memset(sBuff, 0, sizeof(sBuff));
			//Build 1.00.0108 remove 0x57 and 0x5A for CITI
			if(!ChkIfShanghaiCB() && !ChkIfWLB() && !ChkIfCiti())
			{
				if( bForUpLoad )
				{
					AppendStdTagList(astLocalTaglist, 0x5A, DE55_MUST_SET, 0);
				}
				else
				{
					AppendStdTagList(astLocalTaglist, 0x57, DE55_MUST_SET, 0);
				}
			}
			AppendStdTagList(astLocalTaglist, 0x9B, DE55_MUST_SET, 0);
		}

		if( ChkIfBoc() )
		{	// master does not need 9F08
			if( /*glProcInfo.stTranLog.szPan[0]!='5'*/!ChkIfMC(glProcInfo.stTranLog.szPan) )//2016-2-15
			{
				AppendStdTagList(astLocalTaglist, 0x9F08, DE55_MUST_SET, 0);
			}
		}
		else if( !ChkIfScb() && !ChkIfICBC_MACAU() && !PPDCC_ChkIfDccAcq())
		{
			AppendStdTagList(astLocalTaglist, 0x9F08, DE55_MUST_SET, 0);
		}
		//Build 1.00.0106
		//if (ChkIfICBC() || (ChkIfWLB() && (glProcInfo.stTranLog.szPan[0]=='5')))
		//Build 1.00.0117
		if (ChkIfICBC())
		{
			AppendStdTagList(astLocalTaglist, 0x9F53, DE55_MUST_SET, 0);
		}
		if (ChkIfICBC() || ChkIfWLB())
		{
			if (glProcInfo.stTranLog.ucTranType == REVERSAL)
			{
				AppendStdTagList(astLocalTaglist, 0x9F5B, DE55_MUST_SET, 0);
			}
		}
		if (glProcInfo.stTranLog.ucTranType ==CASH)
		{
			AppendStdTagList(astLocalTaglist,0x9F03,DE55_MUST_SET,0);
		}        // PP-DCC

        if (PPDCC_ChkIfDccAcq())
        {
            RemoveFromTagList(astLocalTaglist, 0x5F34);
            AppendStdTagList(astLocalTaglist, 0x5F34, DE55_OPT_SET, 0);
            AppendStdTagList(astLocalTaglist, 0x5F28, DE55_MUST_SET, 0);
            AppendStdTagList(astLocalTaglist, 0x9F42, DE55_MUST_SET, 0);
        }
		//build 133 add for FUBON TLV
		if (ChkIfFubon())
		{
			AppendStdTagList(astLocalTaglist, 0x5F25, DE55_OPT_SET, 0);
           AppendStdTagList(astLocalTaglist, 0x5F28, DE55_OPT_SET, 0);
            AppendStdTagList(astLocalTaglist, 0x9F1F, DE55_OPT_SET, 0);
 		
		}
		//build 137 add for dashing
		if (ChkIfDahOrBCM()&&glProcInfo.stTranLog.ucTranType == CASH)
		{
			AppendStdTagList(astLocalTaglist, 0x9F08, DE55_OPT_SET, 0);
			AppendStdTagList(astLocalTaglist, 0x9F53, DE55_OPT_SET, 0);
		}
		//-----------------------------------------------------------
		// Generate data by tag list
		for(iCnt=0; astLocalTaglist[iCnt].uiEmvTag!=0; iCnt++)
		{
			memset(sBuff, 0, sizeof(sBuff));
			iRet = EMVGetTLVData(astLocalTaglist[iCnt].uiEmvTag, sBuff, &iLength);
			if( iRet==EMV_OK )
			{
				if ((astLocalTaglist[iCnt].ucLen > 0) && (iLength > astLocalTaglist[iCnt].ucLen))
				{
					iLength = astLocalTaglist[iCnt].ucLen;
				}
				BuildTLVString(astLocalTaglist[iCnt].uiEmvTag, sBuff, iLength, &psTemp);
			}
			else if(astLocalTaglist[iCnt].ucOption==DE55_MUST_SET )
			{
				if (!ChkIfBeaHalf())
				{
				    BuildTLVString(astLocalTaglist[iCnt].uiEmvTag, NULL, 0, &psTemp);//bea not accept length = 0
				}

                //add by richard 20161222, JCB need 9F03 v1.00.0213P
                if((strstr(glCurIssuer.szName, "JCB")!=NULL) && astLocalTaglist[iCnt].uiEmvTag==0x9F03) 
                {
                    BuildTLVString(astLocalTaglist[iCnt].uiEmvTag, NULL, 6, &psTemp);
                }
                //end JCB need 9F03 v1.00.0213P
//				return ERR_TRAN_FAIL;	// 如果必须存在的TAG不存在,交易失败
			}
		}

		//-----------------------------------------------------------
		// Generate custom tag content
		//Gillian 20161024

		if( /*glProcInfo.stTranLog.szPan[0]=='5'*/ChkIfMC(glProcInfo.stTranLog.szPan) || ( ChkCurAcqName("AMEX_MACAU", TRUE) ) )//2016-2-15 //Gillian 20161024
		{	// for master card TCC = "R" -- retail
			BuildTLVString(0x9F53, (uchar *)"R", 1, &psTemp);
		}

		memset(sBuff, 0, sizeof(sBuff));
		iRet = EMVGetScriptResult(sBuff, &iLength);
		if( iRet==EMV_OK )
		{
			if (ChkIfDahOrBCM()||ChkIfShanghaiCB()||ChkIfICBC() || ChkIfWLB())// ChkIfICBC added by Arthur
            {
                BuildTLVString(0x9F5B, sBuff, iLength, &psTemp);
            } 
            else
            {
		    	BuildTLVString(0xDF5B, sBuff, iLength, &psTemp);
            }
		}
		if (glProcInfo.stTranLog.ucTranType != CASH)
		{
			memcpy(psTemp, "\x9F\x03\x06\x00\x00\x00\x00\x00\x00",  9);
			psTemp += 9;
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
    else if (ChkIfDahOrBCM() || ChkIfWingHang())
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
	else if(ChkIfBnu())
	{
        *piOutLen = 0;
        iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0x5A, sBuff, &uiLen);
        if (iRet==0)
        {
            memcpy(psOutData, sBuff, uiLen);
            psOutData += uiLen;
            *piOutLen += uiLen;
        }
        iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0x95, sBuff, &uiLen);
        if (iRet==0)
        {
            memcpy(psOutData, sBuff, uiLen);
            psOutData += uiLen;
            *piOutLen += uiLen;
        }
		
        iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0x9B, sBuff, &uiLen);
        if (iRet==0)
        {
            memcpy(psOutData, sBuff, uiLen);
            psOutData += uiLen;
            *piOutLen += uiLen;
        }
		
        iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0x9F10, sBuff, &uiLen);
        if (iRet==0)
        {
            memcpy(psOutData, sBuff, uiLen);
            psOutData += uiLen;
            *piOutLen += uiLen;
        }
		
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
		
        iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0xDFF0, sBuff, &uiLen);
        if (iRet==0)
        {
            memcpy(psOutData, sBuff, uiLen);
            psOutData += uiLen;
            *piOutLen += uiLen;
        }
        iRet = GetSpecTLVItem(((TRAN_LOG *)pstTranLog)->sIccData, ((TRAN_LOG *)pstTranLog)->uiIccDataLen, 0xDF91, sBuff, &uiLen);
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

	if(ChkIfBnu())
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
	else if(!ChkIfShanghaiCB())
	{
		memcpy(psOutData, "\xDF\x5C\x07\x00\x20\x20\x20\x20\x20\x20", 10);
		*piOutLen = 10;
	}
	else
		*piOutLen = 0;
	return 0;
}

int SetStdDE56(DE55Tag *pstList, uchar *psOutData, int *piOutLen)
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
	else if(ChkIfShanghaiCB()||ChkIfICBC() || ChkIfWLB())
	{
		memcpy(psTemp,"\xDF\xF0\x0D",3);
		psTemp +=3;
		if (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT)
		{
			memcpy(psTemp,"\x01",1);
		}
		else
		{
			memcpy(psTemp,"\x00",1);
		}
		psTemp +=1;
		PubLong2Bcd(glProcInfo.stTranLog.ulInvoiceNo, 3,psTemp);
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
	if( ChkIfShanghaiCB()||ChkIfICBC() || ChkIfWLB())
	{
		memset(sBuff, 0, sizeof(sBuff));
		EMVGetTLVData(0x9F02, sBuff, &iLength);
		BuildTLVString(0x9F02, sBuff, iLength, &psTemp);
		memset(sBuff, 0, sizeof(sBuff));
		EMVGetTLVData(0x9F37, sBuff, &iLength);
		BuildTLVString(0x9F37, sBuff, iLength, &psTemp);
	}

	memset(sBuff, 0, sizeof(sBuff));
	iRet = EMVGetScriptResult(sBuff, &iLength);
	if (iRet == EMV_NO_DATA)
	{
		memcpy(sBuff,"\x00\x00\x00\x00\x00",5);
		iLength = 5;
	}
	else if( iRet!=EMV_OK )
	{
		*piOutLen = (psTemp-psOutData);
		return 0;
	}

	// continue issuer script result process
	if( ChkIfBoc() || ChkIfFubon() || ChkIfBea() || ChkIfICBC() || ChkIfWLB() )
	{
		memcpy(psTemp, "\xDF\x91", 2);
	}
	else if( ChkIfDahOrBCM() || ChkIfScb() || ChkIfCiti()||ChkIfShanghaiCB() )
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
		PubChar2Long(psTag+1, uiLenBytes, &lTemp);
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

int GetSpecTLVItem(uchar *psTLVString, int iMaxLen, uint uiSearchTag, uchar *psOutTLV, ushort *puiOutLen)
{
	uchar		*psTag, *psTagStr;
	uint		uiTag, uiLenBytes;
	ulong		lTemp;

	// skip null tags
    for (psTag=psTLVString; psTag<psTLVString+iMaxLen; psTag++)
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
            PubChar2Long(psTag+1, uiLenBytes, &lTemp);
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
void SaveRspICCData(uint uiTag, uchar *psData, int iDataLen)
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
void BuildTLVString(ushort uiEmvTag, uchar *psData, int iLength, uchar **ppsOutData)
{
	uchar	*psTemp;

	if( iLength<0 )
	{
		return;
	}

	// 设置TAG
	// write tag
	psTemp = *ppsOutData;
	if( uiEmvTag & 0xFF00 )
	{
		*psTemp++ = (uchar)(uiEmvTag >> 8);
	}
	*psTemp++ = (uchar)uiEmvTag;

	// 设置Length
	// write length
	if( iLength<=127 )	// 目前数据长度均小余127字节,但仍按标准进行处理
	{
		*psTemp++ = (uchar)iLength;
	}
	else
	{	// EMV规定最多255字节的数据
		*psTemp++ = 0x81;
		*psTemp++ = (uchar)iLength;
	}

	// 设置Value
	// write value
	if( iLength>0 )
	{
		if (psData==NULL)
        {
            memset(psTemp, 0, iLength);
        }
        else
        {
            memcpy(psTemp, psData, iLength);
        }
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
	SetDE55(TRUE, glProcInfo.stTranLog.sIccData, &iLength);//2014-12-2 for AMEX reversal issue
    glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;

    if (PPDCC_ChkIfDccAcq())// PP-DCC

    {
	    SetDE55(DE55_SALE, glProcInfo.stTranLog.sIccData, &iLength);
    }
    else
    {
	SetDE55(DE55_UPLOAD, glProcInfo.stTranLog.sIccData, &iLength);
    }
	glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;

	if( glProcInfo.ucOnlineStatus!=ST_ONLINE_APPV )
	{	// ICC脱机, offline approved
		SaveTVRTSI(TRUE);
		GetNewTraceNo();
//		sprintf((char *)glProcInfo.stTranLog.szRspCode, "00");
//		sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");
		if(ChkIfAmex())
		{
			if (glProcInfo.ucOnlineStatus==ST_ONLINE_FAIL)
			{
				sprintf((char *)glProcInfo.stTranLog.szAuthCode, "Y3");
			} 
			else
			{
				sprintf((char *)glProcInfo.stTranLog.szAuthCode, "Y1");	// for AMEX, approval code = Y1 while chip off line apporved.
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

// core cannot process correctly if length of 9F18 is zero
// eg, 71 12 9F 18 00 86 0D 84 1E 00 00 08 11 22 33 44 55 66 77 88
void AdjustIssuerScript(void)
{
	int		iRet;
	uchar	*psTemp;

	memset(sgScriptBak, 0, sizeof(sgScriptBak));
	memset(&sgScriptInfo, 0, sizeof(sgScriptInfo));
	sgCurScript = sgScriptBakLen = 0;
	bHaveScript  = FALSE;
	for(psTemp=sIssuerScript; psTemp<sIssuerScript+sgScriptLen; )
	{
		iRet = GetTLVItem(&psTemp, sIssuerScript+sgScriptLen-psTemp, SaveScriptData, TRUE);
		if( iRet<0 )
		{
			return;
		}
	}
	if( bHaveScript && sgCurScript>0 )
	{
		PackScriptData();
	}

	memcpy(sIssuerScript, sgScriptBak, sgScriptBakLen);
	sgScriptLen = sgScriptBakLen;
}

// callback function for process issuer script
void  SaveScriptData(uint uiTag, uchar *psData, int iDataLen)
{
	switch( uiTag )
	{
	case 0x71:
	case 0x72:
		if( bHaveScript && sgCurScript>0 )
		{
			PackScriptData();
		}
		sgScriptInfo.uiTag = uiTag;
		bHaveScript = TRUE;
		break;

	case 0x9F18:
		sgScriptInfo.iIDLen = MIN(4, iDataLen);
		memcpy(sgScriptInfo.sScriptID, psData, MIN(4, iDataLen));
		break;

	case 0x86:
		sgScriptInfo.iCmdLen[sgCurScript] = iDataLen;
		memcpy(sgScriptInfo.sScriptCmd[sgCurScript], psData, iDataLen);
		sgCurScript++;
		break;

	default:
		break;
	}
}

void PackTLVData(uint uiTag, uchar *psData, uint uiDataLen, uchar *psOutData, int *piOutLen)
{
	int		iHeadLen;

	PackTLVHead(uiTag, uiDataLen, psOutData, &iHeadLen);
	memcpy(psOutData+iHeadLen, psData, uiDataLen);
	*piOutLen = (uiDataLen+iHeadLen);
}

void PackTLVHead(uint uiTag, uint uiDataLen, uchar *psOutData, int *piOutLen)
{
	uchar	*psTemp;

	// pack tag bytes
	psTemp = psOutData;
	if( uiTag & 0xFF00 )
	{
		*psTemp++ = uiTag>>8;
	}
	*psTemp++ = uiTag;

	// pack length bytes
	if( uiDataLen<=127 )
	{
		*psTemp++ = (uchar)uiDataLen;
	}
	else
	{
		*psTemp++ = LENMASK_NEXTBYTE|0x01;	// one byte length
		*psTemp++ = (uchar)uiDataLen;
	}

	*piOutLen = (psTemp-psOutData);
}

int CalcTLVTotalLen(uint uiTag, uint uiDataLen)
{
	int		iLen;

	// get length of TLV tag bytes
	iLen = 1;
	if( uiTag & 0xFF00 )
	{
		iLen++;
	}

	// get length of TLV length bytes
	iLen++;
	if( uiDataLen>127 )
	{
		iLen++;
	}

	return (iLen+uiDataLen);
}

// re-generate issuer script(remove issuer script ID, if the length is zero)
void PackScriptData(void)
{
	int		iCnt, iTotalLen, iTempLen;

	iTotalLen = 0;
	if( sgScriptInfo.iIDLen>0 )
	{
		iTotalLen += CalcTLVTotalLen(0x9F18, 4);
	}
	for(iCnt=0; iCnt<sgCurScript; iCnt++)
	{
		iTotalLen += CalcTLVTotalLen(0x86, sgScriptInfo.iCmdLen[iCnt]);
	}
	PackTLVHead(sgScriptInfo.uiTag, iTotalLen, &sgScriptBak[sgScriptBakLen], &iTempLen);
	sgScriptBakLen += iTempLen;

	if( sgScriptInfo.iIDLen>0 )
	{
		PackTLVData(0x9F18, sgScriptInfo.sScriptID, 4, &sgScriptBak[sgScriptBakLen], &iTempLen);
		sgScriptBakLen += iTempLen;
	}
	for(iCnt=0; iCnt<sgCurScript; iCnt++)
	{
		PackTLVData(0x86, sgScriptInfo.sScriptCmd[iCnt], sgScriptInfo.iCmdLen[iCnt], &sgScriptBak[sgScriptBakLen], &iTempLen);
		sgScriptBakLen += iTempLen;
	}

	memset(&sgScriptInfo, 0, sizeof(sgScriptInfo));
	sgCurScript = 0;
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
        {0x9B,   DE55_MUST_SET, 0},//2014-12-2 for AMEX reversal issue
        {0x9C,   DE55_MUST_SET, 0},
        {0x5F2A, DE55_MUST_SET, 0},//2014-12-2 for AMEX reversal issue
        {0x5F34, DE55_MUST_SET, 0},//2014-12-2 for AMEX reversal issue
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
		EMVGetTLVData(0x95,   glEmvStatus.sgTVROld,  &glEmvStatus.glTvrLen);
		EMVGetTLVData(0x9B,   glEmvStatus.sgTSIOld,  &glEmvStatus.glTsiLen);
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
	/*	memcpy(glTSI, glEmvStatus.sgTSINew, sizeof(glEmvStatus.sgTSINew)); //Gillian 20161020
		memcpy(glTVR, glEmvStatus.sgTVRNew, sizeof(glEmvStatus.sgTVRNew));
		ScrCls();
		ScrPrint(0, 2, ASCII, "After TSI2=%02X %02X", glTSI[0], glTSI[1]);
		ScrPrint(0, 1, ASCII, "TVR2=%02X %02X %02X %02X %02X",
			glTVR[0], glTVR[1], glTVR[2],
			glTVR[3], glTVR[4]);
    getkey();*/

		iRet = EMVGetTLVData(0x8E,   glEmvStatus.sgCVRNew,  &glEmvStatus.glCvrLen); //Gillian emv debug
		iRet = EMVGetTLVData(0x9f34,   glEmvStatus.sgCVMRNew,  &glEmvStatus.glCvmRLen);
				
	}
	SaveEmvStatus();
}

// show last EMV status
void ViewTVR_TSI(void)
{
	int		iTemp, iRet;
	uchar	szBuff[20];
	uchar	sCVR[5] = {0}, sCvmR[5] = {0}; //Gillian debug
	ushort	usLength;

	if( PasswordBank()!=0 )
	{
		return;
	}

	LoadEmvStatus();

	ScrCls();
	ScrPrint(0, 0, ASCII, "Before TSI=%02X %02X", glEmvStatus.sgTSIOld[0], glEmvStatus.sgTSIOld[1]);
	ScrPrint(0, 1, ASCII, "TVR=%02X %02X %02X %02X %02X",
			glEmvStatus.sgTVROld[0], glEmvStatus.sgTVROld[1], glEmvStatus.sgTVROld[2],
			glEmvStatus.sgTVROld[3], glEmvStatus.sgTVROld[4]);


	ScrPrint(0, 2, ASCII, "IssuAppData=");
	ScrGotoxy(0, 3);
	ScrFontSet(ASCII);
	for(iTemp=0; iTemp<glEmvStatus.sgARQCLenOld; iTemp++)
	{
#if defined(_P60_S1_) || defined(_P70_S_)
		printf("%02X", glEmvStatus.sgARQCOld[iTemp]);
#else	
		Lcdprintf("%02X", glEmvStatus.sgARQCOld[iTemp]);
#endif
	}
	PubWaitKey(USER_OPER_TIMEOUT);

	ScrCls();
	ScrPrint(0, 0, ASCII, "After TSI=%02X %02X", glEmvStatus.sgTSINew[0], glEmvStatus.sgTSINew[1]);
	ScrPrint(0, 1, ASCII, "TVR=%02X %02X %02X %02X %02X",
			glEmvStatus.sgTVRNew[0], glEmvStatus.sgTVRNew[1], glEmvStatus.sgTVRNew[2],
			glEmvStatus.sgTVRNew[3], glEmvStatus.sgTVRNew[4]);

	//PubDebugOutput(" CVR", glEmvStatus.sgCVRNew, glEmvStatus.glCvrLen, DEVICE_PRN, HEX_MODE); //Gillian emv debug
	//PubDebugOutput(" CVMR", glEmvStatus.sgCVMRNew, glEmvStatus.glCvmRLen,DEVICE_PRN, HEX_MODE);  //Gillian emv debug


	PubBcd2Asc0(glEmvStatus.sgTACDeinal, 5, szBuff);
	ScrPrint(0, 2, ASCII, "TACDenial =%10.10s", szBuff);

	PubBcd2Asc0(glEmvStatus.sgTACOnline, 5, szBuff);
	ScrPrint(0, 3, ASCII, "TACOnline =%10.10s", szBuff);

	PubBcd2Asc0(glEmvStatus.sgTACDefault, 5, szBuff);
	ScrPrint(0, 4, ASCII, "TACDefault=%10.10s", szBuff);

	PubBcd2Asc0(glEmvStatus.sgIACDeinal, 5, szBuff);
	ScrPrint(0, 5, ASCII, "IACDenial =%10.10s", szBuff);

	PubBcd2Asc0(glEmvStatus.sgIACOnline, 5, szBuff);
	ScrPrint(0, 6, ASCII, "IACOnline =%10.10s", szBuff);

	PubBcd2Asc0(glEmvStatus.sgIACDefault, 5, szBuff);
	ScrPrint(0, 7, ASCII, "IACDefault=%10.10s", szBuff);

	PubWaitKey(USER_OPER_TIMEOUT);

	ScrCls();
	PubDispString(_T("PRINT DETAIL?"), DISP_LINE_LEFT+4);
	if( PubYesNo(USER_OPER_TIMEOUT) )
	{
		return;
	}

	PrnInit();
	PrnSetNormal();
	PubDebugOutput("BEFORE TSI", glEmvStatus.sgTSINew,2,DEVICE_PRN, HEX_MODE);
	PubDebugOutput("AFTER TSI", glEmvStatus.sgTSINew,2,DEVICE_PRN, HEX_MODE);
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

int  GetAmexDE55(uchar *psAuthData,int *pulAuthDatalen,uchar *psScript,int *pulScriptLen)
{
	uchar  ucICCDataLen;
	uchar *psICCData;

	psICCData = glRecvPack.sICCData+2;//2 bytes for field lenth//2014-12-2
	psICCData += 4;  // Amex  header sub1 (4bytes)
	psICCData += 2;  // Amex  header sub2 (2bytes)

	ucICCDataLen = *psICCData;
	psICCData += 1;  // len

	if (ucICCDataLen<=16)
	{
		*pulAuthDatalen = (int) ucICCDataLen;
		memcpy(psAuthData,psICCData,ucICCDataLen);
		psICCData += ucICCDataLen;
	}
	else
	{
		return ONLINE_FAILED;
	}

	ucICCDataLen = *psICCData;
	psICCData += 1;
	if (ucICCDataLen<=129)
	{
		*pulScriptLen = (int) ucICCDataLen;
		memcpy(psScript,psICCData,ucICCDataLen);
	}

	return 0;
}


#endif		// #ifdef ENABLE_EMV
//Gillian debug
unsigned char cEMVSM2Verify(unsigned char *paucPubkeyIn,unsigned char *paucMsgIn,int nMsglenIn, unsigned char *paucSignIn, int nSignlenIn)
{
    return 0;
}

unsigned char cEMVSM3(unsigned char *paucMsgIn, int nMsglenIn,unsigned char *paucResultOut)
{
    return 0;
}
/*void cEMVPiccIsoCommand(void)
{
	
}*/
unsigned char cEMVPiccIsoCommand(unsigned char cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv)
{
	return 0;
}
// end of file






