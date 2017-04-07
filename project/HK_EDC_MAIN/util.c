
#include "global.h"

/********************** Internal macros declaration ************************/
#define TIMER_TEMPORARY     4
#define EPS_MULTIAPP_FILE       "MAPPEVENTS"
#define SYS_FILE                "SYSPARAM.DAT"
/********************** Internal structure declaration *********************/
void DispAmountASCII(uchar ucLine, uchar *pszAmount);//2014-5-9
/********************** Internal functions declaration *********************/
static void DispBox2(void);
static int  MatchCardTableForInstalment(uchar acq_index);
static int  SelectAcqForCard(uchar *psAcqMatchFlag);
static void GetHolderNameFromTrack1(uchar *pszHolderName);
static void ConvertHolderName(uchar *pszOrgName, uchar *pszNormalName);
static int  GetEmvTrackData(void);
static int  GetPanFromTrack(uchar *pszPAN, uchar *pszExpDate);
static int  MatchTrack2AndPan(uchar *pszTrack2, uchar *pszPan);
static int  MatchCardBin(uchar *pszPAN);
static int  GetSecurityCode(void);
static int  DetectCardEvent(uchar ucMode);
static void DispFallBackPrompt(void);
static int  VerifyManualPan(void);
static int  GetTipAmount(void);
static int  ExtractPAN(uchar *pszPAN, uchar *pszOutPan);
static void DispWelcomeOnPED(void);
static int  GetExpiry(void);
static void DispOkSub(uchar ucLogoNo);
static int CLSSGetCard(void);//2016-4-25 ttt

/********************** Internal variables declaration *********************/

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/


// 初始化交易参数
void InitTransInfo(void)
{
    uchar ucEcrBak;
#ifdef AMT_PROC_DEBUG
	uchar glAmtBuff[80][12+4+1],szAmtBuff[80][12+4+1];//2014-9-19 ttt save previous received amts
	int iCnt=0, iCntBuff;
#endif
    //ECR 交易时，要保存上一次的ECR状态
//  if (glProcInfo.ucEcrRFFlg == 1)
//  {
//
//      ucEcrBak = glProcInfo.ucEcrCtrl;
//  }
//  else
//  {
//      ucEcrBak = ECR_NONE;
//  }
    ucEcrBak = glProcInfo.ucEcrCtrl;//the next code will clear the glProcInfo,save before clear,Squall 2013.2.19
#ifdef AMT_PROC_DEBUG
	//2014-9-19 save previously received amts
	iCntBuff = 0;
	if(glProcInfo.stTranLog.cntAmt != 0)
	{
		iCntBuff = glProcInfo.stTranLog.cntAmt;
		for(iCnt;iCnt<glProcInfo.stTranLog.cntAmt;iCnt++)
		{
			sprintf((char*)glAmtBuff[iCnt],"%.17s", glProcInfo.stTranLog.glAmt[iCnt]); 
			sprintf((char*)szAmtBuff[iCnt],"%.17s", glProcInfo.stTranLog.szAmt[iCnt]); 
		}
	}
#endif
    memset(&glProcInfo, 0, sizeof(SYS_PROC_INFO));
#ifdef AMT_PROC_DEBUG
	//2014-9-19 save previously received amts
	if(iCntBuff)
	{
		glProcInfo.stTranLog.cntAmt = iCntBuff;
		for(iCnt=0;iCnt<iCntBuff;iCnt++)
		{
			sprintf((char*)glProcInfo.stTranLog.glAmt[iCnt],"%.17s", glAmtBuff[iCnt]); 
			sprintf((char*)glProcInfo.stTranLog.szAmt[iCnt],"%.17s", szAmtBuff[iCnt]); 
		}
	}
#endif
    glProcInfo.uiRecNo     = 0xFFFF;
    glProcInfo.bIsFirstGAC = TRUE;
    sprintf((char *)glProcInfo.stTranLog.szTipAmount, "%012ld", 0L);

    // set initial transaction currency to local currency at first
    glProcInfo.stTranLog.stTranCurrency = glSysParam.stEdcInfo.stLocalCurrency;     // initial currency
    glProcInfo.stTranLog.uiEntryMode = MODE_NO_INPUT;
    glProcInfo.stTranLog.bPanSeqOK   = FALSE;

    if (ucEcrBak == ECR_BEGIN)
    {
        glProcInfo.ucEcrCtrl = ucEcrBak;
    }
    else
    {
        glProcInfo.ucEcrCtrl             = ECR_NONE;
    }
//  glProcInfo.ucEcrCtrl             = ucEcrBak;

    glProcInfo.stTranLog.ulInvoiceNo = glSysCtrl.ulInvoiceNo;
    PubGetDateTime(glProcInfo.stTranLog.szDateTime);    // set default txn time

    memcpy(&glCommCfg, &glSysParam.stTxnCommCfg, sizeof(COMM_CONFIG));
}

// 获得新的流水号
ulong GetNewTraceNo(void)
{
    glSysCtrl.ulSTAN++;
    if( !(glSysCtrl.ulSTAN>0 && glSysCtrl.ulSTAN<=999999L) )
    {
        glSysCtrl.ulSTAN = 1L;
    }
    SaveSysCtrlBase();

    return (glSysCtrl.ulSTAN);
}

// calculate new batch number
ulong GetNewBatchNo(ulong ulCurBatchNo)
{
    ulCurBatchNo++;
    if( !(ulCurBatchNo>0 && ulCurBatchNo<=999999L) )
    {
        return 1L;
    }

    return ulCurBatchNo;
}

// 获得新的票据号
ulong GetNewInvoiceNo(void)
{
    glSysCtrl.ulInvoiceNo++;
    if( !(glSysCtrl.ulInvoiceNo>0 && glSysCtrl.ulInvoiceNo<=999999L) )
    {
        glSysCtrl.ulInvoiceNo = 1L;
    }
    SaveSysCtrlBase();

    return (glSysCtrl.ulInvoiceNo);
}

// 提示拔出IC卡
void PromptRemoveICC(void)
{
    if( !ChkIfEmvEnable() )
    {
        return;
    }

    IccClose(ICC_USER);
    if( IccDetect(ICC_USER)!=0 )    // 如果IC卡已拔出，直接返回。
    {
        return;
    }

    // 显示并等待IC卡拔出
    ScrCls();
    DispTransName();
    PubDispString(_T("PLS REMOVE CARD"), 4|DISP_LINE_LEFT);
    while( IccDetect(ICC_USER)==0 )
    {
        Beep();
        DelayMs(500);
    }
}
#ifdef ENABLE_EMV
// 删除过期CAPK
// erase expired CAPK
void EraseExpireCAPK(void)
{
    int         iRet, iCnt;
    EMV_CAPK    stCAPK;

    for(iCnt=0; iCnt<MAX_KEY_NUM; iCnt++)
    {
        memset(&stCAPK, 0, sizeof(EMV_CAPK));
        iRet = EMVCheckCAPK(&stCAPK.KeyID, stCAPK.RID);
        if( iRet==EMV_OK )
        {
            break;
        }
        EMVDelCAPK(stCAPK.KeyID, stCAPK.RID);
    }
}
#endif

static int DispMainLogo(void)
{
    return -1;
}

static void DispIndicator(void)
{
#ifdef APP_DEBUG
    ScrPrint(0, 2, ASCII, "DEBUG");
#elif defined(EMV_TEST_VERSION) && defined(ENABLE_EMV)
    ScrPrint(0, 2, ASCII, "EMVTEST");
#endif

#ifdef DEMO_HK
    ScrPrint(0, 3, ASCII, "HK DEMO");
#else
    if (ChkIfTrainMode())
    {
        ScrPrint(0, 3, ASCII, "DEMO");
    }
#endif
}

// 显示刷卡/插卡界面
void DispSwipeCard(uchar bShowLogo)
{
    char *pszStr;

    if (bShowLogo)
    {
        if (DispMainLogo()!=0)
        {
            bShowLogo = FALSE;
        }
    }

    DispIndicator();


    //----------------------------//
    if( ChkIfEmvEnable() )
    {
        pszStr = _T_NOOP("SWIPE/INSERT ...");
    }

    else
    {
        pszStr = _T_NOOP("PLS SWIPE CARD");
    }
    //----------------------------------//

    if (bShowLogo)
    {
        ScrPrint(0, 7, ASCII, pszStr);
    }
    else
    {
        PubDispString(_T(pszStr), 4|DISP_LINE_LEFT);
    }
}
void DispKeyInOnly(void)
{
    ScrClrLine(2, 7);
    PubDispString(_T("ENTER ACC. NO."), 4|DISP_LINE_LEFT);
}
void DispTapOnly(void)
{
    PubDispString(_T("PLS TAP CARD"), 4|DISP_LINE_LEFT);
}
void DispSwipeOnly(void)
{
    PubDispString(_T("PLS SWIPE CARD"), 4|DISP_LINE_LEFT);
}

void DispInsertOnly(void)
{
    ScrClrLine(2, 7);
    PubDispString(_T("PLS INSERT CARD"), 4|DISP_LINE_LEFT);
}

void DispFallBackSwipeCard(void)
{
    PubDispString(_T("PLS SWIPE CARD"), 4|DISP_LINE_LEFT);
    ScrPrint(0, 7, ASCII|REVER, _T("FALL BACK"));
}

void DispBlockFunc(void)
{
    ScrClrLine(2, 7);
    PubDispString(_T("TRANS NOT ALLOW"), 4|DISP_LINE_LEFT);
    PubBeepErr();
    PubWaitKey(3);
}

void DispProcess(void)
{
    ScrClrLine(2, 7);
    PubDispString(_T("PROCESSING...."), 4|DISP_LINE_LEFT);
//  DispBox2();
}

void DispWait(void)
{
    ScrClrLine(2, 7);
    PubDispString(_T("PLEASE WAIT..."), 4|DISP_LINE_LEFT);
}

void DispDial(void)
{
    ScrClrLine(2, 7);
    PubDispString(_T("DIALING...."), 4|DISP_LINE_LEFT);
}

void DispSend(void)
{
    ScrClrLine(2, 7);
    PubDispString(_T("SENDING...."), 4|DISP_LINE_LEFT);
}

void DispReceive(void)
{
    ScrClrLine(2, 7);
    PubDispString(_T("RECEIVING..."), 4|DISP_LINE_LEFT);
}

void DispPrinting(void)
{
    ScrClrLine(2, 7);
    PubDispString(_T("PRINTING..."), 4|DISP_LINE_LEFT);
}

void DispClearOk(void)
{
    DispOperOk(_T("CLEARED     "));
}

void DispAdjustTitle(void)
{
    PubShowTitle(TRUE, (uchar *)_T("ADJUST          "));
}

void DispVersion(void)
{
    uchar   sBuff[32];

    // Show public version
    PubShowTitle(TRUE, (uchar *)_T("VERSION"));
    sprintf(sBuff, "%s %s", APP_NAME, EDC_VER_PUB);
    if (strlen(sBuff)>16)
    {
        ScrPrint(0, 2, ASCII, "%s", APP_NAME);
        ScrPrint(0, 3, ASCII, "%s", EDC_VER_PUB);
    }
    else
    {
        ScrPrint(0, 2, CFONT, sBuff);
    }

#ifdef ENABLE_EMV
    // Show EMV lib version (EMV lib updates sometimes)
    memset(sBuff, 0, sizeof(sBuff));
    if (EMVReadVerInfo(sBuff)==EMV_OK)
    {
        ScrPrint(0, 5, ASCII, "EMV:%.20s", sBuff);
    }


#endif

    // Free memory on flash
    ScrPrint(0, 6, ASCII, "FREE:%-8ld", freesize());
    // Downloaded terminal ID
    ScrPrint(0, 7, ASCII, "TERM ID:%.8s", glSysParam.stEdcInfo.szDownLoadTID);
    if (PubWaitKey(USER_OPER_TIMEOUT)!=KEYFN)
    {
        return;
    }

    //--------------------------------------------------
    // Internal version
    ScrClrLine(2, 7);
    ScrPrint(0, 2, ASCII, "DETAIL VERSION");
    ScrPrint(0, 3, ASCII, "%s", EDC_VER_INTERN);
    if (glSysParam.ucTermStatus!=INIT_MODE)
    {
        ScrPrint(0, 4, ASCII, "PARA INIT DATE");
        ScrPrint(0, 5, ASCII, "%.14s", glSysParam.stEdcInfo.szInitTime);
    }
    if (PubWaitKey(USER_OPER_TIMEOUT)!=KEYFN)
    {
        return;
    }
	//Gillian 2016-8-19
     #ifdef PP_SSL    
   // if(stComm.ucEnableSSL == TRUE && stComm.ucEnableSSL_URL == TRUE) //notice
	if(!ChkIfAmex() && !ChkIfDiners())
	{
		if(glSysParam.stTxnCommCfg.ucEnableSSL_URL)
		{
			ScrCls();
			ScrPrint(0, 0, ASCII, "SSL URL");

			ScrPrint(0,2, ASCII, "URL: %s", glSysParam.stTxnCommCfg.szSSL_URL);
      
			ScrPrint(0,5, ASCII, "URLIP:%s\n", glSysParam.stTxnCommCfg.szSSL_BackupIP);
      
			ScrPrint(0,6, ASCII, "SSL Port: %s", glSysParam.stTxnCommCfg.szSSL_Port);
		}
	}
	 if (PubWaitKey(USER_OPER_TIMEOUT)!=KEYFN)
    {
        return;
    }
	
      #endif // end HASE_SSL
}

// 接收等待回调函数
void DispWaitRspStatus(ushort uiLeftTime)
{
    ScrPrint(87, 3, CFONT, "%-3d", uiLeftTime);
}

void DispBox2(void)
{
    ShowBoxNoShadow(2, 0, 8, 21);
}

void ShowBoxNoShadow(uchar y1, uchar x1, uchar y2, uchar x2)
{
    uchar   i, wn, m, n;

    wn = x2 * 6;
    m = y1 * 8 + 4;
    n = y2 * 8 - 4;
    for(i = x1 * 6 + 1; i < wn; i++)
    {
        ScrPlot(i, m, 1);
        ScrPlot(i, n, 1);
    }

    wn = y2 * 8 - 4;
    m = x1 * 6 + 1;
    n = x2 * 6 - 1;
    for(i = y1 * 8 + 5; i < wn; i++)
    {
        ScrPlot(m, i, 1);
        ScrPlot(n, i, 1);
    }
}

// 输入功能号码
int FunctionInput(void)
{
    uchar szFuncNo[2+1];

    ScrClrLine(2,7);
    PubDispString(_T("FUNCTION ?"), 2|DISP_LINE_LEFT);
    memset(szFuncNo, 0, sizeof(szFuncNo));
    if( PubGetString(NUM_IN, 1, 2, szFuncNo, USER_OPER_TIMEOUT)!=0 )
    {
        return -1;
    }

    return  atoi((char *)szFuncNo);
}

void SysHalt(void)
{
    ScrCls();
    PubDispString(_T("HALT FOR SAFETY "), DISP_LINE_REVER|DISP_LINE_LEFT);
    PubDispString(_T("PLS RESTART POS "), 4|DISP_LINE_LEFT);
    while(1);
}

void SysHaltInfo(void *pszDispInfo, ...)
{
    uchar       szBuff[1024+1];
    va_list     pVaList;

    if( pszDispInfo==NULL || *(uchar *)pszDispInfo==0 )
    {
        return;
    }

    va_start(pVaList, pszDispInfo);
    vsprintf((char*)szBuff, (char*)pszDispInfo, pVaList);
    va_end(pVaList);

    ScrCls();
    PubDispString(_T("HALT FOR SAFETY "), DISP_LINE_REVER|DISP_LINE_LEFT);
    PubDispString(_T(szBuff),            3|DISP_LINE_LEFT);
    while(1);
}

void DispMagReadErr(void)
{
    PubDispString(_T("READ CARD ERR."), 4|DISP_LINE_LEFT);
}

void DispTransName(void)
{
    uchar   szTitle[16+1];

    if( glProcInfo.stTranLog.ucTranType==LOGON ) //add by richard 20170223
    {
        return;
    }
    
    if( glProcInfo.stTranLog.ucTranType==SALE_OR_AUTH )
    {
        GetEngTime(szTitle);
        ScrPrint(0, 0, CFONT|REVER, "%-16.16s", szTitle);
    }
    else
    {
        ScrPrint(0, 0, CFONT|REVER, "%-16.16s", _T(glTranConfig[glProcInfo.stTranLog.ucTranType].szLabel));
    }
}

void DispAccepted(void)
{
    ScrClrLine(2, 7);
    PubDispString(_T("TXN ACCEPTED"), 4|DISP_LINE_LEFT);
}

void DispErrMsg(char *pFirstMsg, char *pSecondMsg, short sTimeOutSec, ushort usOption)
{
    // DERR_BEEP     : error beep

    PubASSERT(pFirstMsg!=NULL);

    ScrClrLine(2, 7);
    if (pSecondMsg==NULL)
    {
        PubShowMsg(4, pFirstMsg);
    }
    else
    {
        PubShowMsg(3, pFirstMsg);
        PubShowMsg(5, pSecondMsg);
    }

    if (usOption & DERR_BEEP)
    {
        PubBeepErr();
    }

    PubWaitKey(sTimeOutSec);
}

void PrintOne(void)
{
    static uchar sPrinterLogo[137] =
    {
        0x04,
        0x00,0x20,
        0x0,0xf8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,
        0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0xf8,

        0x00,0x20,
        0x0,0xff,0x0,0x0,0x0,0x0,0x80,0x40,0x20,0x70,0x4c,0x44,0x54,0x54,0x54,0x54,
        0x54,0x54,0x44,0x44,0x44,0x74,0x2c,0x20,0xa0,0xe0,0xe0,0x0,0x0,0x0,0x0,0xff,

        0x00,0x20,
        0x0,0xff,0x0,0x0,0x0,0x3f,0x21,0xe1,0x21,0x21,0x21,0x21,0x21,0x21,0x21,0x21,
        0x21,0x21,0x21,0x25,0x25,0xe1,0xe1,0x7f,0x1f,0xf,0x7,0x0,0x0,0x0,0x0,0xff,

        0x00,0x20,
        0x0,0xf,0x8,0x8,0x8,0x8,0x8,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,
        0x9,0x9,0x9,0x9,0x9,0x9,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0xf
    };

    ScrGotoxy(86, 3);
    ScrDrLogo(sPrinterLogo);
}

void apiBeef(uchar mode,ushort DlyTime)
{
#if defined(_P78_) || defined(_P80_) || defined(_P90_) || defined(_S_SERIES_) ||  defined(_SP30_) || defined(_P58_)
    Beef(mode, DlyTime);
#else
    kbbeef(mode, DlyTime);
#endif
}

int CHangeTransCurrency(void)
{
    // TODO: implement currency selection
    // let  glProcInfo.stTranLog.stTranCurrency = selected currency
    // ...

#ifdef ENABLE_EMV
    EMVGetParameter(&glEmvParam);
    memcpy(glEmvParam.TransCurrCode, glProcInfo.stTranLog.stTranCurrency.sCurrencyCode, 2);
    glEmvParam.TransCurrExp = glProcInfo.stTranLog.stTranCurrency.ucDecimal;
    EMVSetParameter(&glEmvParam);
    // Only in this trasaction, so DON'T back up
#endif

    return 0;
}

// 根据Issuer Key设置glCurIssuer
// set current issuer by key
void FindIssuer(uchar ucIssuerKey)
{
    uchar   ucIndex;

    for(ucIndex=0; ucIndex<glSysParam.ucIssuerNum; ucIndex++)
    {
        if( glSysParam.stIssuerList[ucIndex].ucKey==ucIssuerKey )
        {
            SetCurIssuer(ucIndex);
            glProcInfo.stTranLog.ucIssuerKey = ucIssuerKey;
            break;
        }
    }
}

// 根据Key设置glCurAcq
// set current acquirer by key.
void FindAcq(uchar ucAcqKey)
{
    uchar   ucIndex;

    for(ucIndex=0; ucIndex<glSysParam.ucAcqNum; ucIndex++)
    {
        if( glSysParam.stAcqList[ucIndex].ucKey==ucAcqKey )
        {
            SetCurAcq(ucIndex);
            glProcInfo.stTranLog.ucAcqKey = ucAcqKey;
            break;
        }
    }
}

// 根据pszAcqName查找Acq
// Find acquirer by name. Return index if found, else return MAX_ACQ
uchar FindAcqIdxByName(char *pszAcqName, uchar bFullMatch)
{
    uchar   ucIndex;

    for(ucIndex=0; ucIndex<glSysParam.ucAcqNum; ucIndex++)
    {
        if (bFullMatch)
        {
            //PubDebugOutput(pszAcqName, glSysParam.stAcqList[ucIndex].szName, strlen(glSysParam.stAcqList[ucIndex].szName), DEVICE_SCR, ASC_MODE);
            if( strcmp(glSysParam.stAcqList[ucIndex].szName, pszAcqName)==0 )
            {
                return ucIndex;
            }
        }
        else
        {
            if( strstr(glSysParam.stAcqList[ucIndex].szName, pszAcqName)!=NULL )
            {
                return ucIndex;
            }
        }
    }

    return MAX_ACQ;
}

uchar FindInstPlanIndex(uchar ucInstalMonth)
{
    uchar   ucCnt;

    for (ucCnt=0; ucCnt<glSysParam.ucPlanNum; ucCnt++)
    {
        if (glSysParam.stPlanList[ucCnt].ucMonths==ucInstalMonth)
        {
            break;
        }
    }
    return ucCnt;
}


// 设置当前收单行信息
// set current acquirer by index
void SetCurAcq(uchar ucAcqIndex)
{
    memcpy(&glCurAcq, &glSysParam.stAcqList[ucAcqIndex], sizeof(ACQUIRER));
    glCurAcq.ucIndex = ucAcqIndex;
}

// 设置当前发卡行信息
// set current issuer by index
void SetCurIssuer(uchar ucIssuerIndex)
{
    memcpy(&glCurIssuer, &glSysParam.stIssuerList[ucIssuerIndex], sizeof(ISSUER));
}

// 读取磁卡磁道及PAN
int ReadMagCardInfo(void)
{
    int     iRet;

    if( glEdcMsgPtr->MsgType==MAGCARD_MSG )
    {
        glProcInfo.stTranLog.uiEntryMode = MODE_SWIPE_INPUT;
        sprintf((char *)glProcInfo.szTrack1, "%.*s", LEN_TRACK1, glEdcMsgPtr->MagMsg.track1);
        sprintf((char *)glProcInfo.szTrack2, "%.*s", LEN_TRACK2, glEdcMsgPtr->MagMsg.track2);
        sprintf((char *)glProcInfo.szTrack3, "%.*s", LEN_TRACK3, glEdcMsgPtr->MagMsg.track3);
    }
    else
    {
        glProcInfo.stTranLog.uiEntryMode = MODE_SWIPE_INPUT;
        MagRead(glProcInfo.szTrack1, glProcInfo.szTrack2, glProcInfo.szTrack3);
    }

    iRet = GetPanFromTrack(glProcInfo.stTranLog.szPan, glProcInfo.stTranLog.szExpDate);
    if( iRet!=0 )
    {
        DispMagReadErr();
        PubBeepErr();
        PubWaitKey(3);
        return ERR_NO_DISP;
    }
    Beep();

    return 0;
}

// 从磁道信息分析出卡号(PAN)
int GetPanFromTrack(uchar *pszPAN, uchar *pszExpDate)
{
    int     iPanLen;
    char    *p, *pszTemp;

    // 从2磁道开始到'＝'
    if( strlen((char *)glProcInfo.szTrack2)>0 )
    {
        pszTemp = (char *)glProcInfo.szTrack2;
    }
    else if( strlen((char *)glProcInfo.szTrack3)>0 )
    {
        pszTemp = (char *)&glProcInfo.szTrack3[2];
    }
    else
    {// 2、3磁道都没有
        return ERR_SWIPECARD;
    }

    p = strchr((char *)pszTemp, '=');
    if( p==NULL )
    {
        return ERR_SWIPECARD;
    }
    iPanLen = p - pszTemp;
    if( iPanLen<13 || iPanLen>19 )
    {
        return ERR_SWIPECARD;
    }

    sprintf((char *)pszPAN, "%.*s", iPanLen, pszTemp);
    if( pszTemp==(char *)glProcInfo.szTrack2 )
    {
        sprintf((char *)pszExpDate, "%.4s", p+1);
    }
    else
    {
        sprintf((char *)pszExpDate, "0000");
    }

    return 0;
}

// 检测磁道信息是否为IC卡磁道信息
// Check service code in track 2, whther it is 2 or 6
uchar IsChipCardSvcCode(uchar *pszTrack2)
{
    char    *pszSeperator;

    if( *pszTrack2==0 )
    {
        return FALSE;
    }

    pszSeperator = strchr((char *)pszTrack2, '=');
    if( pszSeperator==NULL )
    {
        return FALSE;
    }
    if( (pszSeperator[5]=='2') || (pszSeperator[5]=='6') )
    {
        return TRUE;
    }

    return FALSE;
}

// 校验卡号
int ValidPanNo(uchar *pszPanNo)
{
    uchar   bFlag, ucTemp, ucResult;
    uchar   *pszTemp;

    // 是否检查卡号
    if( !ChkIssuerOption(ISSUER_CHKPAN_MOD10) )
    {
        return 0;
    }

    // (2121算法)
    bFlag    = FALSE;
    pszTemp  = &pszPanNo[strlen((char *)pszPanNo)-1];
    ucResult = 0;
    while( pszTemp>=pszPanNo )
    {
        ucTemp = (*pszTemp--) & 0x0F;
        if( bFlag )    ucTemp *= 2;
        if( ucTemp>9 ) ucTemp -= 9;
        ucResult = (ucTemp + ucResult) % 10;
        bFlag = !bFlag;
    }

    if( ucResult!=0 )
    {
        PubDispString(_T("INVALID CARD"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(3);
        return ERR_NO_DISP;
    }

    return 0;
}

// 检查卡的有效期(YYMM)
int ValidCardExpiry(void)
{
    uchar   szDateTime[14+1];
    ulong   ulCardYear, ulCardMonth;
    ulong   ulCurYear, ulCurMonth;
    uchar   ucInvalidFormat;

    glProcInfo.bExpiryError = FALSE;
    ucInvalidFormat = FALSE;

    ulCardYear  = PubAsc2Long(glProcInfo.stTranLog.szExpDate, 2, NULL);
    ulCardYear += (ulCardYear>80) ? 1900 : 2000;
    ulCardMonth = PubAsc2Long(glProcInfo.stTranLog.szExpDate+2, 2, NULL);

    PubGetDateTime(szDateTime);
    ulCurYear  = PubAsc2Long(szDateTime, 4, NULL);
    ulCurMonth = PubAsc2Long(szDateTime+4, 2, NULL);

    if( ulCardMonth>12 || ulCardMonth<1 )
    {
        ucInvalidFormat = TRUE;
        glProcInfo.bExpiryError = TRUE;
    }
    if (//ulCardYear>ulCurYear+20 ||    // 是否需要判断有效期太长的卡?
        ulCardYear<ulCurYear ||
        (ulCurYear==ulCardYear && ulCurMonth>ulCardMonth) )
    {
        glProcInfo.bExpiryError = TRUE;
    }

    if (glProcInfo.bExpiryError)
    {
        if( ChkIssuerOption(ISSUER_EN_EXPIRY) && ChkIssuerOption(ISSUER_CHECK_EXPIRY) )
        {
            ScrClrLine(2,7);
            if (ucInvalidFormat)
            {
                PubDispString(_T("ERR EXP. FORMAT"), 4|DISP_LINE_LEFT);
            }
            else
            {
                PubDispString(_T("CARD EXPIRED"), 4|DISP_LINE_LEFT);
            }
            if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
            {// if EMV expired card, let core to continue process(based upon TACs/IACs)
                DelayMs(1500);
                return 0;
            }
            else
            {
                PubBeepErr();
                PubWaitKey(3);
                return ERR_NO_DISP;
            }
        }
    }

    return 0;
}

// 获取终端当前时间,格式:YYYYMMDDhhmmss
// format : YYYYMMDDhhmmss
void PubGetDateTime(uchar *pszDateTime)
{
    uchar   sCurTime[7];

    GetTime(sCurTime);
    sprintf((char *)pszDateTime, "%02X%02X%02X%02X%02X%02X%02X",
            (sCurTime[0]>0x80 ? 0x19 : 0x20), sCurTime[0], sCurTime[1],
            sCurTime[2], sCurTime[3], sCurTime[4], sCurTime[5]);
}

int UpdateLocalTime(uchar *pszNewYear, uchar *pszNewDate, uchar *pszNewTime)
{
    uchar   szLocalTime[14+1], sBuffer[16];

    if ((pszNewDate!=0) && (pszNewTime!=0))
    {
        if (pszNewYear==NULL)
        {
            memset(szLocalTime, 0, sizeof(szLocalTime));
            PubGetDateTime(szLocalTime);

            if ((memcmp(szLocalTime+4, "12", 2)==0) &&      // local month is DECEMBER
                (memcmp(pszNewDate, "01", 2)==0))           // received month is JANUARY. local clock slower
            {
                PubAscInc(szLocalTime, 4);      // increase local year
            }
            if ((memcmp(szLocalTime+4, "01", 2)==0) &&      // local month is JANUARY
                (memcmp(pszNewDate, "12", 2)==0))           // received month is DECEMBER. local clock faster
            {
                PubAscDec(szLocalTime, 4);      // increase local year
            }
        }
        else
        {
            memcpy(szLocalTime, pszNewYear, 4);
        }

        memcpy(szLocalTime+4, pszNewDate, 4);   // MMDD
        memcpy(szLocalTime+8, pszNewTime, 6);   // hhmmss

        memset(sBuffer, 0, sizeof(sBuffer));
        PubAsc2Bcd(szLocalTime+2, 12, sBuffer);
        return SetTime(sBuffer);
    }

    return -1;
}

// 英文习惯的日期时间(16 bytes, eg: "OCT07,2006 11:22")
void GetEngTime(uchar *pszCurTime)
{
    uchar   Month[12][4] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    uchar   sCurTime[7], ucMonth;

    GetTime(sCurTime);
    ucMonth = (sCurTime[1]>>4) * 10 + (sCurTime[1] & 0x0F) - 1;
    sprintf((char *)pszCurTime, "%.3s%02X,%02X%02X %02X:%02X", Month[ucMonth],
            sCurTime[2], (sCurTime[0]>0x80 ? 0x19 : 0x20),
            sCurTime[0], sCurTime[3], sCurTime[4]);
//  sprintf((char *)pszCurTime, "%.3s %02X,%02X  %02X:%02X", Month[ucMonth],
//      sCurTime[2], sCurTime[0], sCurTime[3], sCurTime[4]);

}

// 转换YYYYMMDDhhmmss 到 OCT 07, 2006  11:22
// Convert from ... to ...
void Conv2EngTime(uchar *pszDateTime, uchar *pszEngTime)
{
    uchar   Month[12][4] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    uchar   ucMonth;

    ucMonth = (uchar)((PubAsc2Long(&pszDateTime[4], 2, NULL)-1) % 12);
    sprintf((char *)pszEngTime, "%3.3s %2.2s, %4.4s  %2.2s:%2.2s", Month[ucMonth],
            pszDateTime+6, pszDateTime, pszDateTime+8, pszDateTime+10);
}

// 检查卡号,并确定收单行/发卡行(必须在读出卡号后调用)
// Check PAN, and determine Issuer/Acquirer.
int ValidCard(void)
{
    int     iRet;

    iRet = MatchCardTable(glProcInfo.stTranLog.szPan,
                          glProcInfo.stTranLog.ucTranType,
                          0);
    if( iRet!=0 )
    {
        ScrClrLine(2, 7);
        PubDispString(_T("UNSUPPORT CARD"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(3);
        return ERR_NO_DISP;
    }

    if( !ChkAcqOption(ACQ_EMV_FEATURE) )
    {
        // 非EMV Acquirer不允许Fallback及插卡
        if( (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) ||
            (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
            (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
        {
            PubDispString(_T("NOT EMV ENABLE"), 4|DISP_LINE_LEFT);
            PubBeepErr();
            PubWaitKey(3);
            return ERR_NO_DISP;
        }
    }

    iRet = ValidPanNo(glProcInfo.stTranLog.szPan);
    if( iRet!=0 )
    {
        return iRet;
    }

    iRet = ValidCardExpiry();
    if( iRet!=0 )
    {
        return iRet;
    }

    CheckCapture();

    if( glProcInfo.stTranLog.ucTranType==INSTALMENT )
    {
        if( glProcInfo.bIsFallBack || glEdcMsgPtr->MsgType==ICCARD_MSG )
        {
            PubDispString(_T("PLS SWIPE AGAIN"), 4|DISP_LINE_LEFT);
            PubBeepErr();
            PubWaitKey(5);
            return ERR_NEED_SWIPE;
        }
    }

    if (PPDCC_ChkIfDccAcq())
    {
        if (glProcInfo.stTranLog.ucDccType==0)
        {
            // If not set to MCP yet, then set to DCC now
            glProcInfo.stTranLog.ucDccType = PPTXN_DCC;
        }
    }

    GetCardHolderName(glProcInfo.stTranLog.szHolderName);
    iRet = ConfirmPanInfo();
    if( iRet!=0 )
    {
        CommOnHook(FALSE);
        return iRet;
    }

    iRet = GetSecurityCode();
    if( iRet!=0 )
    {
        return iRet;
    }

    return 0;
}

// 获得持卡人姓名(已经转换为打印格式)
// Read and convert holder name to printable format.
void GetCardHolderName(uchar *pszHolderName)
{
#ifdef ENABLE_EMV
    int     iRet, iTagLen;
    uchar   szBuff[40];
#endif
    uchar   szTempName[40];

    *pszHolderName = 0;

#ifdef ENABLE_EMV
    if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
    {
        memset(szBuff, 0, sizeof(szBuff));
        iRet = EMVGetTLVData(0x5F20, szBuff, &iTagLen);
        if( iRet!=EMV_OK )
        {
            return;
        }
        ConvertHolderName(szBuff, szTempName);
    }
    else    // other entry mode, just get it from track 1 data
#endif
    {
        GetHolderNameFromTrack1(szTempName);
    }

    sprintf((char *)pszHolderName, "%.26s", szTempName);
}

// get holder name form track 1, which is identified by '^'
void GetHolderNameFromTrack1(uchar *pszHolderName)
{
    char    *p, *q;
    uchar   szOrgName[50];
    int     iLen;

    *pszHolderName = 0;
    if( glProcInfo.szTrack1[0]==0 )
    {
        return;
    }

    p = strchr((char *)glProcInfo.szTrack1, '^');
    if( p==NULL )
    {
        return;
    }
    p++;
    iLen = strlen(p);

    q = strchr(p, '^');
    if( q!=NULL )
    {
        iLen = MIN(q-p, iLen);
    }

    sprintf((char *)szOrgName, "%.*s", (int)MIN(sizeof(szOrgName)-1, iLen), p);
    ConvertHolderName(szOrgName, pszHolderName);
}

// 转换ISO7813格式人名为打印格式
// "Amex/F D.Mr" --> "Mr F D Amex"
void ConvertHolderName(uchar *pszOrgName, uchar *pszNormalName)
{
    char    *pszTitle, *pszMidName, *pszTemp, szBuff[50];

    sprintf((char *)pszNormalName, "%s", pszOrgName);
    if( *pszOrgName==0 )
    {
        return;
    }
    pszTemp = (char *)pszNormalName;

    pszMidName = strchr((char *)pszOrgName, '/');
    if( pszMidName==NULL )
    {
        return;
    }

    pszTitle = strrchr((char *)pszOrgName, '.');
    if( pszTitle!=NULL )
    {
        sprintf(szBuff, "%s ", pszTitle+1);
        PubTrimStr((uchar *)szBuff);
        pszTemp += sprintf(pszTemp, "%s ", szBuff);

        sprintf(szBuff, "%.*s ", (int)(pszTitle-pszMidName-1), pszMidName+1);
        PubTrimStr((uchar *)szBuff);
        pszTemp += sprintf(pszTemp, "%s ", szBuff);
    }
    else
    {
        sprintf(szBuff, "%s", pszMidName+1);
        PubTrimStr((uchar *)szBuff);
        pszTemp += sprintf(pszTemp, "%s ", szBuff);
    }
    sprintf(pszTemp, "%.*s", (int)(pszMidName-(char *)pszOrgName), pszOrgName);
}

static uchar IsInstalAcq(void)
{
    int ii;

    for (ii=0; ii<glSysParam.ucPlanNum; ii++)
    {
        if (glSysParam.stPlanList[ii].ucAcqIndex==glCurAcq.ucIndex)
        {
            return TRUE;
        }
    }
    return FALSE;
}

// 根据卡号匹配卡表,并最终确定收单行(glCurAca)和发卡行(glCurIssuer)
// determine glCurAcq and glCurIssuer, due to ACQ-ISS-CARD matching table.
// ucMode :
//    0 -- normal mode
//    1 -- for PPDCC, select non-DCC acquirer; for BOCDCC, select DCC sale acquirer after rate lookup
//    2 -- for BOCDCC, select non-DCC acquirer after rate lookup
int MatchCardTable(uchar *pszPAN, uchar ucTranType, uchar ucMode)
{
    int         iRet;
    uchar       ucCnt, ucPanLen, ucAcqNum;
    uchar       sPanHeader[5], sAcqMatchFlag[MAX_ACQ], sCardTableMatchFlag[MAX_CARD];
    CARD_TABLE  *pstCardTbl;
    uchar       ucPPDccCardIndex, ucScbLoyaltyCardIndex, ucIsVisaMcJcb;

    memset(sAcqMatchFlag, 0, sizeof(sAcqMatchFlag));
    memset(sCardTableMatchFlag, 0, sizeof(sCardTableMatchFlag));

    // 建立匹配收单行列表，判断卡属性
    // create a list of matched card table ------------------------------------------
    ucPanLen = strlen((char *)pszPAN);
    PubAsc2Bcd(pszPAN, 10, sPanHeader);
    for(ucCnt=0; ucCnt<glSysParam.ucCardNum; ucCnt++)
    {
        pstCardTbl = &glSysParam.stCardTable[ucCnt];
        if( (memcmp(pstCardTbl->sPanRangeLow,  sPanHeader, 5)<=0  &&
             memcmp(pstCardTbl->sPanRangeHigh, sPanHeader, 5)>=0) &&
            (pstCardTbl->ucPanLength==0 || pstCardTbl->ucPanLength==ucPanLen) )
        {
            sCardTableMatchFlag[ucCnt] = 1;
        }
    }

    // 过滤
    // Filtering issuer/acquirer ----------------------------------------------------
    ucPPDccCardIndex      = MAX_CARD;
    ucScbLoyaltyCardIndex = MAX_CARD;
    ucIsVisaMcJcb         = FALSE;
    ucAcqNum              = 0;
    for (ucCnt=0; ucCnt<glSysParam.ucCardNum; ucCnt++)
    {
        if (sCardTableMatchFlag[ucCnt]==0)// not bingo
        {
            continue;
        }
        FindIssuer(glSysParam.stCardTable[ucCnt].ucIssuerKey);
        FindAcq(glSysParam.stCardTable[ucCnt].ucAcqKey);
        if( glSysCtrl.sAcqStatus[glCurAcq.ucIndex]!=S_USE  &&
            glSysCtrl.sAcqStatus[glCurAcq.ucIndex]!=S_PENDING )// Abnormal acquirer status
        {
            sCardTableMatchFlag[ucCnt] = 0;// unmask this card range
            continue;
        }

        if (ChkIfIssuerVMJ())// for casion environment
        {
            ucIsVisaMcJcb = TRUE;
        }
        if (IsInstalAcq())// for instalment, use MatchCardTableForInstalment()
        {
            if (ucTranType ==INSTALMENT)
            {
                break;//20121203
            }
            sCardTableMatchFlag[ucCnt] = 0;// unmask this card range
            continue;
        }

        if ((glProcInfo.stTranLog.ucTranType==REFUND) && ChkIssuerOption(ISSUER_NO_REFUND))
        {
            sCardTableMatchFlag[ucCnt] = 0; //1 Feb 2012 build88Q: bug fix for foreign card refund
            continue;
        }

        if (PPDCC_ChkIfDccAcq())
        {
            if (ucMode!=0)
            {
                sCardTableMatchFlag[ucCnt] = 0;// unmask this card range
                continue;
            }

            if (PPDCC_LoadBinFile()!=0)
            {
                PPDCC_PromptDownloadBin();// prompt operator to download bin
                ucPPDccCardIndex = ucCnt;// If bin is not yet download, default goes to DCC
            }
            else if (!PPDCC_ChkIfLocalCard(pszPAN))
            {
                ucPPDccCardIndex = ucCnt;
            }
            else
            {
                sCardTableMatchFlag[ucCnt] = 0; // local card shouldn't go DCC. unmask this card range and acquirer
                continue;
            }
        }

        // TODO: Add more acquirer/issuer/card checking here
        // if ()...continue

        // all OK. count into matched acquirer
        if (sAcqMatchFlag[glCurAcq.ucIndex]==0)
        {
            sAcqMatchFlag[glCurAcq.ucIndex] = 1;
            ucAcqNum++;
        }
    }

    // Various special restrictions -------------------------------------------------
    if (ChkIfCasinoMode())
    {
        if (!ucIsVisaMcJcb)
        {
            return ERR_UNSUPPORT_CARD;
        }
    }
    else if (ucScbLoyaltyCardIndex<MAX_CARD)
    {
    }
    else if (ucPPDccCardIndex<MAX_CARD)
    {
        FindIssuer(glSysParam.stCardTable[ucPPDccCardIndex].ucIssuerKey);
        FindAcq(glSysParam.stCardTable[ucPPDccCardIndex].ucAcqKey);
        return 0;
    }
    // more "else if" here. be careful of logic priority.

    // Two or more acquirers match, and if allow user selection ---------------------
    if ((ucAcqNum>1) && ChkEdcOption(EDC_SELECT_ACQ_FOR_CARD) && (ucTranType!=INSTALMENT))
    {
        // User manual selection
        iRet = SelectAcqForCard(sAcqMatchFlag);
        if ( (iRet<0) || (iRet>=glSysParam.ucAcqNum) )
        {
            return iRet;
        }

        FindIssuer(glSysParam.stCardTable[iRet].ucIssuerKey);
        FindAcq(glSysParam.stCardTable[iRet].ucAcqKey);
        return 0;
    }

    // Else, auto select.
    for (ucCnt=0; ucCnt<glSysParam.ucCardNum; ucCnt++)
    {
        if (sCardTableMatchFlag[ucCnt]!=0)
        {
            FindIssuer(glSysParam.stCardTable[ucCnt].ucIssuerKey);
            FindAcq(glSysParam.stCardTable[ucCnt].ucAcqKey);
            if (ChkIfCitiTaxi())
            {
                if (glCurAcq.ucIndex == glSysCtrl.ucCurrAcqIdx)
                {
                    return 0;
                }
            }
            else
                return 0;
        }
    }

    return ERR_UNSUPPORT_CARD;
}

int  ChkIfMC(uchar *pszPAN) //2016-2-15 check if master card! add cardbin "222100272099"
{
	uchar		sPanHeader[6];

	PubAsc2Bcd(pszPAN, 12, sPanHeader);
////2015-11-16 ttt
//	ScrCls();
//	ScrPrint(0,0,0, "%.6s",pszPAN);
//	ScrPrint(0,2,0, "%d",memcmp("541736",  pszPAN, 6));
//	ScrPrint(0,4,0, "%d",memcmp("541738",  pszPAN, 6));
//	getkey();	
	if( (memcmp("222100",  pszPAN/*sPanHeader*/, 6)<=0  &&
		memcmp("272099", pszPAN/*sPanHeader*/, 6)>=0) )
	{
		return TRUE;
	}
	else if(pszPAN[0]=='5')
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

int MatchCardTableForInstalment(uchar ucIndex)
{
    uchar       ucCnt, ucPanLen, ucAcqNum;
    uchar       sPanHeader[5], sCardIndex[MAX_ACQ], sAcqMatchFlag[MAX_ACQ];
    CARD_TABLE  *pstCardTbl;

    memset(sCardIndex,    0, sizeof(sCardIndex));
    memset(sAcqMatchFlag, 0, sizeof(sAcqMatchFlag));

    // 建立收单行列表
    // create a list of matched acquirer.
    ucPanLen = strlen((char *)glProcInfo.stTranLog.szPan);
    PubAsc2Bcd(glProcInfo.stTranLog.szPan, 10, sPanHeader);

    for(ucAcqNum=ucCnt=0; ucCnt<glSysParam.ucCardNum; ucCnt++)
    {
        pstCardTbl = &glSysParam.stCardTable[ucCnt];
        if( (memcmp(pstCardTbl->sPanRangeLow,  sPanHeader, 5)<=0  &&
             memcmp(pstCardTbl->sPanRangeHigh, sPanHeader, 5)>=0) &&
            (pstCardTbl->ucPanLength==0 || pstCardTbl->ucPanLength==ucPanLen) )
        {
            FindIssuer(pstCardTbl->ucIssuerKey);
            FindAcq(pstCardTbl->ucAcqKey);

            if (glCurAcq.ucIndex != ucIndex)
            {
                continue;
            }

            if( glSysCtrl.sAcqStatus[glCurAcq.ucIndex]==S_USE  ||
                glSysCtrl.sAcqStatus[glCurAcq.ucIndex]==S_PENDING )
            {
                return 0;
            }
        }
    }

    return ERR_UNSUPPORT_CARD;
}

// 根据卡号匹配的收单行信息,提示用户选择收单行
// Select acquirer for card. only select in already matched acquirers.
int SelectAcqForCard(uchar *psAcqMatchFlag)
{
    int         iMenuNo;
    uchar       ucCnt, ucAcqNum, szPrompt[16+1];
    MenuItem    stAcqMenu[MAX_ACQ+1];
    uchar       sAcqIndexForMenuItem[MAX_ACQ+1];

    // build menu from candidate acquirer's list
    memset(&stAcqMenu[0], 0, sizeof(stAcqMenu));
    memset(sAcqIndexForMenuItem, 0, sizeof(sAcqIndexForMenuItem));
    for(ucAcqNum=ucCnt=0; ucCnt<glSysParam.ucAcqNum; ucCnt++)
    {
        if( psAcqMatchFlag[ucCnt]==0 )
        {
            continue;
        }
        stAcqMenu[ucAcqNum].bVisible = TRUE;
        sprintf((char *)stAcqMenu[ucAcqNum].szMenuName, "%.10s", glSysParam.stAcqList[ucCnt].szName);
        sAcqIndexForMenuItem[ucAcqNum] = ucCnt;
        ucAcqNum++;
    }

    // prompt use select acquirer
    if( ucAcqNum<=9 )
    {
        sprintf((char *)szPrompt, _T("SELECT  HOST"));
    }
    else
    {
        sprintf((char *)szPrompt, _T("SELECT:"));
    }
    iMenuNo = PubGetMenu(szPrompt, stAcqMenu, MENU_AUTOSNO, USER_OPER_TIMEOUT);
    if( iMenuNo<0 )
    {
        return (iMenuNo==-3) ? ERR_USERCANCEL : ERR_SEL_ACQ;
    }

    return sAcqIndexForMenuItem[iMenuNo];
}

/************************************************************************
* 刷卡事件处理函数
* bCheckICC:    TRUE  检查2磁道的service code(对EMV终端有效)
*               FALSE 不检查
************************************************************************/
int SwipeCardProc(uchar bCheckICC)
{
    int     iRet;

    iRet = ReadMagCardInfo();
    if( iRet!=0 )
    {
        return iRet;
    }

    //2011-06-27 此处增加MatchCardTable为了判断是否该卡片的收单行支持EMV，若不支持则下面不需检查是否需要插卡
    iRet = MatchCardTable(glProcInfo.stTranLog.szPan,
                          glProcInfo.stTranLog.ucTranType,
                          0);
    if( iRet!=0 )
    {
        ScrClrLine(2, 7);
        PubDispString(_T("UNSUPPORT CARD"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(3);
        return ERR_NO_DISP;
    }
    //对某些不支持EMV收单行，将bCheckICC赋为FALSE
    if( !ChkAcqOption(ACQ_EMV_FEATURE) )
    {
        bCheckICC = FALSE;
    }
    // 交易不要求判断卡片类型或者为非EMV终端,直接返回
    if( !bCheckICC || !ChkIfEmvEnable() )
    {
        return 0;
    }


    // EMV终端,继续检查
    if( glProcInfo.bIsFallBack==TRUE )
    {
        if( IsChipCardSvcCode(glProcInfo.szTrack2) )
        {// fallback并且是IC卡,则返回成功
            glProcInfo.stTranLog.uiEntryMode = MODE_FALLBACK_SWIPE;
            return 0;
        }
        else
        {
            DispTransName();
            PubDispString(_T("NON EMV,RE-SWIPE"), 4|DISP_LINE_LEFT);
            PubBeepErr();
            PubWaitKey(3);
            return ERR_NO_DISP;
        }
    }
    else if( IsChipCardSvcCode(glProcInfo.szTrack2) )
    {
        if( glProcInfo.stTranLog.ucTranType!=OFF_SALE &&
            glProcInfo.stTranLog.ucTranType!=REFUND   &&
            glProcInfo.stTranLog.ucTranType!=INSTALMENT )
        {
            DispTransName();
            DispInsertOnly();
            PubBeepErr();
            PubWaitKey(3);
            return ERR_NEED_INSERT;
        }
    }

    return 0;
}

#ifdef ENABLE_EMV

void SetEMVAmountTLV(uchar *pszAmount)
{
    uchar   sTemp[6];

    PubAsc2Bcd(pszAmount, 12, sTemp);
    EMVSetTLVData(0x9F02, sTemp, 6);
    PubLong2Char((ulong)atol((char *)pszAmount), 4, sTemp);
    EMVSetTLVData(0x81, sTemp, 4);
}

// ICC插卡事件处理函数
// Process insertion event.
int InsertCardProc(void)
{
    int     iRet;
    uchar   szTotalAmt[12+1], sTemp[6];

    // 如果已经FALLBACK,忽略IC卡插卡操作
    if( glProcInfo.bIsFallBack==TRUE )
    {
        return ERR_NEED_FALLBACK;
    }

    //!!!! deleted: it is fixed and not allowed to modify after GPO.
    //ModifyTermCapForPIN();

    glProcInfo.stTranLog.uiEntryMode = MODE_CHIP_INPUT;

    DispTransName();
    DispProcess();

    InitTransEMVCfg();

    // 应用选择
    // EMV application selection. This is EMV kernel API
    iRet = EMVAppSelect(ICC_USER, glSysCtrl.ulSTAN);
    DispTransName();
    if( iRet==EMV_DATA_ERR || iRet==ICC_RESET_ERR || iRet==EMV_NO_APP ||
        iRet==ICC_CMD_ERR  || iRet==EMV_RSP_ERR )
    {
        glProcInfo.bIsFallBack = TRUE;
        glProcInfo.iFallbackErrCode = iRet;
        return ERR_NEED_FALLBACK;
    }
    if( iRet==EMV_TIME_OUT || iRet==EMV_USER_CANCEL )
    {
        return ERR_USERCANCEL;
    }
    if( iRet!=EMV_OK )
    {
        return ERR_TRAN_FAIL;
    }

    DispTransName();

    // Clear log to avoid amount accumulation for floor limit checking
    iRet = EMVClearTransLog();

    // Read Track 2 and/or Pan
    iRet = GetEmvTrackData();

    if( iRet!=0 )
    {
        return iRet;
    }
	//Gillian 20161114
	/*=======BEGIN: Jason 2015.04.10  14:20 modify===========*/
    //amex diners do not support online PIN
    if((memcmp(glProcInfo.stTranLog.sAID,"\xA0\x00\x00\x00\x25",5) == 0) ||(memcmp(glProcInfo.stTranLog.sAID,"\xA0\x00\x00\x01\x52",5) == 0) )
    {
        EMVGetParameter(&glEmvParam);
        memcpy(glEmvParam.Capability, EMV_AECAPABILITY, 3);
        EMVSetTLVData(0x9F33, EMV_AECAPABILITY, 3);//Jason 2015.06.19 10:47
        EMVSetParameter(&glEmvParam);
    }
    /*====================== END======================== */

    // Display app prefer name
    if (strlen(glProcInfo.stTranLog.szAppPreferName)!=0)
    {
        PubDispString(glProcInfo.stTranLog.szAppPreferName, 4|DISP_LINE_LEFT);
        PubWaitKey(1);
    }
    else if (strlen(glProcInfo.stTranLog.szAppLabel)!=0)
    {
        PubDispString(glProcInfo.stTranLog.szAppLabel, 4|DISP_LINE_LEFT);
        PubWaitKey(1);
    }
    ////2014-5-12 ttt
    //ScrCls();
    //ScrPrint(0,0,0,"tt- insert ");
    //PortClose(0);
    //PubWaitKey(7);
    //PortOpen(3, "9600,8,e,1");

    iRet = ValidCard(); // 校验卡号
    if( iRet!=0 )
    {
        return iRet;
    }
    UpdateEMVTranType();

    // EMVSetMCKParam to set bypass PIN
    if (ChkIfAmex())
    {
        AppSetMckParam(!ChkIssuerOption(ISSUER_FORBID_EMVPIN_BYPASS));
    }
    //end
    // 输入交易金额
    if( glProcInfo.stTranLog.ucTranType!=RATE_BOC &&
        glProcInfo.stTranLog.ucTranType!=RATE_SCB &&
        glProcInfo.stTranLog.ucTranType!=VOID )
    {
        iRet = GetAmount();
        if( iRet!=0 )
        {
            return ERR_USERCANCEL;
        }
        PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
        //PubAddHeadChars(szTotalAmt, 12, '0');  no need: already 12 digits
        SetEMVAmountTLV(szTotalAmt);
    }

    DispTransName();
    DispProcess();

    // 卡片数据认证
    // Card data authentication
    if( glProcInfo.stTranLog.ucTranType==SALE ||
        glProcInfo.stTranLog.ucTranType==AUTH ||
        glProcInfo.stTranLog.ucTranType==PREAUTH )
    {
        iRet = EMVCardAuth();
        if( iRet!=EMV_OK )
        {
            return ERR_TRAN_FAIL;
        }
    }

    return 0;
}
#endif

#ifdef ENABLE_EMV
// 读取IC卡磁道信息/卡号信息等
int GetEmvTrackData(void)
{
    int     iRet, iLength;
    uchar   sTemp[50], szCardNo[20+1];
    int     i, bReadTrack2, bReadPan;
    ST_EVENT_MSG    stEvent;
    // 读取应用数据
    DispProcess();
    iRet = EMVReadAppData();
    if( iRet==EMV_TIME_OUT || iRet==EMV_USER_CANCEL )
    {
        return ERR_USERCANCEL;
    }
    if( iRet!=EMV_OK )
    {
        return ERR_TRAN_FAIL;
    }

    // Read Track 2 Equivalent Data
    bReadTrack2 = FALSE;
    memset(sTemp, 0, sizeof(sTemp));
    iRet = EMVGetTLVData(0x57, sTemp, &iLength);
    if( iRet==EMV_OK )
    {
        bReadTrack2 = TRUE;
        PubBcd2Asc0(sTemp, iLength, glProcInfo.szTrack2);
        PubTrimTailChars(glProcInfo.szTrack2, 'F'); // erase padded 'F' chars
        for(i=0; glProcInfo.szTrack2[i]!='\0'; i++)     // convert 'D' to '='
        {
            if( glProcInfo.szTrack2[i]=='D' )
            {
                glProcInfo.szTrack2[i] = '=';
                break;
            }
        }
    }

    // read PAN
    bReadPan = FALSE;
    memset(sTemp, 0, sizeof(sTemp));
    iRet = EMVGetTLVData(0x5A, sTemp, &iLength);
    if( iRet==EMV_OK )
    {
        PubBcd2Asc0(sTemp, iLength, szCardNo);

#ifdef APP_MANAGER_VER


        if ((!memcmp("34",szCardNo,2)||!memcmp("37",szCardNo,2))&& ChkIfAppEnabled(APP_AMEX)
            &&strlen(szCardNo)==15)

        {

            InitEventMsg(&stEvent, ICCARD_MSG);
            AppDoEvent(glSysParam.astSubApp[APP_AMEX].ucAppNo, &stEvent);

            InitEventMsg(&stEvent, AMEX_RETURN);
            event_main(&stEvent);
            //  AppDoEvent(glSysParam.astSubApp[APP_EDC].ucAppNo, &stEvent);
        }
        //ttttttttttttttttttttttt
#endif
        PubTrimTailChars(szCardNo, 'F');        // erase padded 'F' chars
        if( bReadTrack2 && !MatchTrack2AndPan(glProcInfo.szTrack2, szCardNo) )
        {
            // 如果Track2 & PAN 同时存在,则必须匹配
            // if Track2 & PAN exist at the same time, must match
            PubDispString(_T("CARD ERROR"), 4|DISP_LINE_LEFT);
            PubBeepErr();
            PubWaitKey(3);
            PromptRemoveICC();
            return ERR_NO_DISP;
//          return ERR_TRAN_FAIL;
        }
        sprintf((char *)glProcInfo.stTranLog.szPan, "%.19s", szCardNo);
        bReadPan = TRUE;
    }
    else if ( !bReadTrack2 )    // 如果Track 2 和 PAN 都没有,则交易失败
    {
        return ERR_TRAN_FAIL;
    }
    if( !bReadPan ) // 没有读取PAN，但是有track 2
    {
        iRet = GetPanFromTrack(glProcInfo.stTranLog.szPan, glProcInfo.stTranLog.szExpDate);
        if( iRet!=0 )
        {
            return ERR_TRAN_FAIL;
        }
    }

    // read PAN sequence number
    glProcInfo.stTranLog.bPanSeqOK = FALSE;
    iRet = EMVGetTLVData(0x5F34, &glProcInfo.stTranLog.ucPanSeqNo, &iLength);
    if( iRet==EMV_OK )
    {
        glProcInfo.stTranLog.bPanSeqOK = TRUE;
    }

    // read Application Expiration Date
    if( bReadPan )
    {
        memset(sTemp, 0, sizeof(sTemp));
        iRet = EMVGetTLVData(0x5F24, sTemp, &iLength);
        if( iRet==EMV_OK )
        {
            PubBcd2Asc0(sTemp, 2, glProcInfo.stTranLog.szExpDate);
        }
    }

    // read other data for print slip
    // application label
    EMVGetTLVData(0x50, glProcInfo.stTranLog.szAppLabel, &iLength);



    // Issuer code table
    iRet = EMVGetTLVData(0x9F11, sTemp, &iLength);
    if ((iRet==0) && (sTemp[0]==0x01))
    {
        // Application prefer name
        EMVGetTLVData(0x9F12, glProcInfo.stTranLog.szAppPreferName, &iLength);  // application label
    }
    // Application ID
    iRet = EMVGetTLVData(0x4F, glProcInfo.stTranLog.sAID, &iLength);    // AID
    if( iRet==EMV_OK )
    {
        glProcInfo.stTranLog.ucAidLen = (uchar)iLength;
    }

    // read cardholder name
    memset(sTemp, 0, sizeof(sTemp));
    iRet = EMVGetTLVData(0x5F20, sTemp, &iLength);
    if( iRet==EMV_OK )
    {
        sprintf((char *)glProcInfo.stTranLog.szHolderName, "%.20s", sTemp);
    }

    return 0;
}
#endif

// 比较2磁道信息和PAN是否一致(For ICC)
// Check whether track2 (from ICC) and PAN (from ICC) are same.
int MatchTrack2AndPan(uchar *pszTrack2, uchar *pszPan)
{
    int     i;
    uchar   szTemp[19+1];

    for(i=0; i<19 && pszTrack2[i]!='\0'; i++)
    {
        if( pszTrack2[i]=='=' )
        {
            break;
        }
        szTemp[i] = pszTrack2[i];
    }
    szTemp[i] = 0;

    if( strcmp((char *)szTemp, (char *)pszPan)==0 )
    {
        return TRUE;
    }

    return FALSE;
}

// 确认卡号信息
int ConfirmPanInfo(void)
{
    int     iIndex;
    uchar   ucLine, szIssuerName[10+1];

    if( glProcInfo.stTranLog.ucTranType==SALE_OR_AUTH ||
        glProcInfo.stTranLog.ucTranType==SALE         ||
        glProcInfo.stTranLog.ucTranType==AUTH         ||
        glProcInfo.stTranLog.ucTranType==PREAUTH )
    {
        ScrCls();
        DispTransName();
        DispWait();
        PreDial();
    }

    ScrCls();
    DispTransName();
    ucLine = 3;

    iIndex = MatchCardBin(glProcInfo.stTranLog.szPan);
    if( iIndex>=0 )
    {
        ScrPrint(0, ucLine++, ASCII, "%.21s", glSysParam.stIssuerNameList[iIndex].szEnglishName);
        if( strlen((char *)glSysParam.stIssuerNameList[iIndex].szEnglishName)>21 )
        {
            ScrPrint(0, ucLine++, ASCII, "%.9s", glSysParam.stIssuerNameList[iIndex].szEnglishName+21);
        }
    }
    else
    {
        ConvIssuerName(glCurIssuer.szName, szIssuerName);
        ScrPrint(0, ucLine++, ASCII, "%s", szIssuerName);
    }

    ScrPrint(0, ucLine++, ASCII, "%.21s", glProcInfo.stTranLog.szHolderName);
    ScrPrint(0, ucLine++, ASCII, "%.19s", glProcInfo.stTranLog.szPan);

    if( ChkIssuerOption(ISSUER_EN_EXPIRY) )
    {
        ScrPrint(0, ucLine++, ASCII, "EXP DATE:%2.2s/%2.2s",
                 &glProcInfo.stTranLog.szExpDate[2], &glProcInfo.stTranLog.szExpDate[0]);
    }

    if( PubYesNo(USER_OPER_TIMEOUT) )
    {
        return ERR_USERCANCEL;
    }

#ifdef ENABLE_EMV
    // set EMV library parameters
    if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
    {
        EMVGetParameter(&glEmvParam);
        memcpy(glEmvParam.MerchId, glCurAcq.szMerchantID, 15);
        memcpy(glEmvParam.TermId, glCurAcq.szTermID, 8);
        EMVSetParameter(&glEmvParam);
        // Only in this trasaction, so DON'T back up
    }
#endif

    return 0;
}

// RFU for HK
int MatchCardBin(uchar *pszPAN)
{
    uchar   szStartNo[20+1], szEndNo[20+1];
    ushort  i;

    for(i=0; i<glSysParam.uiCardBinNum; i++)
    {
        PubBcd2Asc(glSysParam.stCardBinTable[i].sStartNo, 10, szStartNo);
        PubBcd2Asc(glSysParam.stCardBinTable[i].sEndNo,   10, szEndNo);
        if( memcmp(pszPAN, szStartNo, glSysParam.stCardBinTable[i].ucMatchLen)>=0 &&
            memcmp(pszPAN, szEndNo,   glSysParam.stCardBinTable[i].ucMatchLen)<=0 )
        {
            return (int)glSysParam.stCardBinTable[i].ucIssuerIndex;
        }
    }

    return -1;
}

void ConvIssuerName(uchar *pszOrgName, uchar *pszOutName)
{
    char    *p;

    sprintf((char *)pszOutName, "%.10s", pszOrgName);
    p = strchr((char *)pszOutName, '_');
    if( p!=NULL )
    {
        *p = 0;
    }
}

// input CVV2 or 4DBC
int GetSecurityCode(void)
{
    uchar   ucLen;

    if( !ChkIfNeedSecurityCode() )
	{
        return 0;
    }

    if( !ChkIfAmex() && ChkAcqOption(ACQ_ASK_CVV2) )
    {
        ScrClrLine(2,7);
        PubDispString("ENTER",          DISP_LINE_LEFT|2);
        PubDispString("SECURITY CODE?", DISP_LINE_LEFT|4);
        PubDispString("Y/N",            DISP_LINE_LEFT|6);
        if( PubYesNo(60) )
        {
            return 0;
        }
    }

    ScrClrLine(2,7);
    ucLen = ChkIfAmex() ? 4 : 3;
    ScrPrint(0, 2, CFONT, "%s", ChkIfAmex() ? _T("ENTER 4DBC") : _T("SECURITY CODE"));
    if( PubGetString(NUM_IN, ucLen, ucLen, glProcInfo.szSecurityCode, USER_OPER_TIMEOUT)!=0 )
    {
        return ERR_USERCANCEL;
    }

    glProcInfo.stTranLog.uiEntryMode |= MODE_SECURITYCODE;
    return 0;
}

// 根据参数进行,刷卡/插卡/输入卡号
// Accept different entry mode due to input:ucMode
// ucMode: bit 8: 1=skipping check track 2 service code, 0=check
//         bit 7: 1=fallback swipe
//         bit 6: 1=skip detect ICC
int GetCard(uchar ucMode)
{
    int     iRet, iEventID;
    uchar   bCheckICC, ucKey;
	//2016-4-25 add =====================================START
	uchar			ucBuff[6+1];
	uchar			szMaxAmt[12+1];
	PubLong2Bcd(glSysParam.stEdcInfo.ulClssMaxLmt,6,ucBuff);
	PubBcd2Asc0(ucBuff,6,szMaxAmt);
	szMaxAmt[13]=0;
	//2016-4-25 add =====================================END
    if( (glProcInfo.stTranLog.uiEntryMode & 0x0F)!=MODE_NO_INPUT )
    {
        return 0;
    }

    if( ucMode & FALLBACK_SWIPE )
    {
        ucMode &= ~(SKIP_CHECK_ICC|CARD_INSERTED);  // clear bit 8, force to check service code
    }

    bCheckICC = !(ucMode & SKIP_CHECK_ICC);
	//2016-4-25 add============================================START
	//2013-8-28 tttttttttttttttttt
	if (glSysParam.stEdcInfo.ucClssFlag == 1 && ucMode & CARD_TAPPED 
		&& ((glSysParam.stEdcInfo.ucClssMode == 0)||(glSysParam.stEdcInfo.ucClssMode == 2))
		&& (glProcInfo.stTranLog.ucTranType == SALE || glProcInfo.stTranLog.ucTranType == REFUND) /*&& glClssRetryFlag != TRUE*/)//2016-3-29 ttt
	{
		kbflush();
		iRet = GetAmount();
		if(iRet == 0)
		{	
			if(memcmp(glProcInfo.stTranLog.szAmount, szMaxAmt, 12) < 0)//2013-9-6 amount limit
			{
				memset(glAmount, 0, sizeof(glAmount));
				memcpy(glAmount, glProcInfo.stTranLog.szAmount, 12);
				iRet = CLSSGetCard();
				CommOnHook(FALSE);
				if(iRet != SWIPED_MAGCARD && iRet != INSERTED_ICCARD)
				{
					return 77;
				}
			}
		}
		else
		{
			CommOnHook(FALSE);
			return iRet;
		}
	}
	//2016-4-25 add============================================END
    while( 1 )
    {
		//2016-5-11 add contactless handling
		if(iRet == SWIPED_MAGCARD)
		{
			iEventID = CARD_SWIPED;
		}
		else if(iRet == INSERTED_ICCARD)
		{
			iEventID = CARD_INSERTED;
		}
		else
		{
			iEventID = DetectCardEvent(ucMode);
		}

        if( iEventID==CARD_KEYIN )
        {
            ucKey = getkey();
            if( ucKey==KEYCANCEL )
            {
                return ERR_USERCANCEL;
            }
            if( (ucMode & CARD_KEYIN) && ucKey>='0' && ucKey<='9' )
            {
                return ManualInputPan(ucKey);
            }
        }
        else if( iEventID==CARD_SWIPED )
        {
            //2014-5-14 enhancement for ECR & Modem connection
#ifdef _S60_
            ClssClose(); //2014-5-16 enhance
#endif
            iRet = SwipeCardProc(bCheckICC);
            if( iRet==0 )
            {
                return ValidCard();
            }
            else if( iRet==ERR_SWIPECARD )
            {
                DispMagReadErr();
                PubBeepErr();
                PubWaitKey(3);
            }
            else if( iRet==ERR_NEED_INSERT )    // 是芯片卡
            {

                if( !(ucMode & CARD_INSERTED) )
                {// 本身交易不允许插卡

                    return iRet;
                }
                ucMode &= ~CARD_SWIPED;         // 去掉刷卡检查
            }
            else
            {
                return iRet;
            }
        }		
#ifdef ENABLE_EMV
        else if( iEventID==CARD_INSERTED )
        {
            //2014-5-14 enhancement for ECR & Modem connection
#ifdef _S60_
            ClssClose(); //2014-5-16 enhance
#endif
            iRet = InsertCardProc();
            if( iRet==0 )
            {
                return 0;
            }
            else if( iRet==ERR_NEED_FALLBACK )
            {
                DispFallBackPrompt();
                PromptRemoveICC();
                ucMode = CARD_SWIPED|FALLBACK_SWIPE;    // Now we don't support fallback to manual-PAN-entry
            }
            else if( iRet==ERR_TRAN_FAIL )
            {
                PubDispString(_T("NOT ACCEPTED"), 4|DISP_LINE_LEFT);
                PubBeepErr();
                PubWaitKey(3);
                PromptRemoveICC();
                return ERR_NO_DISP;
            }
            else
            {
                return iRet;
            }
        }
#endif
    }
}

// 用户输入事件检测(按键/刷卡/插卡)
int DetectCardEvent(uchar ucMode)
{
    /*-------------------- Lois 2014-5-9 ------------------
      add amount display*/
    uchar   szTotalAmt[12+1];
    uchar   szAmount[12+1];             // ユ肂 // 0xFF2B
    uchar   szTipAmount[12+1];          // 禣肂
    /*-------------------- Lois 2014-5-9 ------------------
    add amount display*/

    //磁头上电、打开、清缓冲
    if( ucMode & CARD_SWIPED )
    {
        MagClose();
        MagOpen();
        MagReset();
    }

    if( ucMode & FALLBACK_SWIPE )
    {
        DispFallBackSwipeCard();
    }
    else if( (ucMode & CARD_SWIPED) && (ucMode & CARD_INSERTED) )
    {
        DispSwipeCard(FALSE);
    }
    else if( (ucMode & CARD_INSERTED)  )
    {
        if( !(ucMode & SKIP_DETECT_ICC) )
        {
            DispInsertOnly();
        }
    }
    else
    {
        DispSwipeOnly();
    }

    /*-------------------- Lois 2014-5-9 ------------------
      add amount display*/
    if (glProcInfo.ucEcrCtrl==ECR_BEGIN&&0==EcrGetAmount(szAmount, szTipAmount))
    {
        if (!ChkIfZeroAmt(szAmount) ||
            !ChkIfZeroAmt(szTipAmount))
        {
            PubAscAdd(szAmount, szTipAmount, 12, szTotalAmt);
            DispAmountASCII(7,szTotalAmt);

        }
    }
    /*-------------------- Lois 2014-5-9 ------------------
      add amount display*/
    kbflush();

    while( 1 )
    {
        if( PubKeyPressed() )
        {
            return CARD_KEYIN;      // 有按键事件
        }
        if( (ucMode & CARD_SWIPED) && (MagSwiped()==0) )
        {
            return CARD_SWIPED;     // 有刷卡事件
        }
		// Gillian 20160719
	/*	if( (ucMode & CARD_TAPPED) && ChkIfEmvEnable() )  //Added by Gillian 2016/1/29 //2016-2-5 AMEX
        {
            return CARD_TAPPED;     // 有刷卡事件
        }*/
        if( (ucMode & CARD_INSERTED) && ChkIfEmvEnable() )
        {
            if( ucMode & SKIP_DETECT_ICC )
            {
                return CARD_INSERTED;   // 有插入IC卡事件
            }
            else if( IccDetect(ICC_USER)==0 )
            {
                return CARD_INSERTED;   // 有插入IC卡事件
            }
        }
    }
}

// 显示Fallback提示界面
void DispFallBackPrompt(void)
{
    uint    iCnt;

    PubDispString(_T("PLS SWIPE CARD"), 4|DISP_LINE_LEFT);
    iCnt = 0;
    while( IccDetect(0)==0 )
    {
        iCnt++;
        if( iCnt>4 )
        {
            Beep();
            ScrPrint(16*5, 7, ASCII, "%8s", iCnt%2 ? "PULL OUT" : "");
        }
        DelayMs(500);
    }
}

// 输入金额及小费
// Get amount and tips.
int GetAmount(void)
{
    int     iRet;
    uchar   bTipManualInput;
    uchar   szTotalAmt[12+1];
//new requirement
    uchar   ucBuff[20];
    uchar   szMaxAmt[12+1];
    PubLong2Bcd(glSysParam.stEdcInfo.ulClssMaxLmt,6,ucBuff);
    PubBcd2Asc0(ucBuff,6,szMaxAmt);
    szMaxAmt[13]=0;

    if ((glSysParam.stEdcInfo.ucClssFlag == 1) && (glSysParam.stEdcInfo.ucClssMode == 2))
    {
        memcpy(glProcInfo.stTranLog.szAmount, glSysParam.stEdcInfo.ucClssFixedAmt, 12);
#ifdef AMT_PROC_DEBUG
		//2014-9-18 each time come to glAmount or szAmount
		sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(25)%.13s",glAmount);
		sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(25)%.13s",glProcInfo.stTranLog.szAmount);
		glProcInfo.stTranLog.cntAmt++;
	//	//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 29: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
    }
    if (glSysParam.stEdcInfo.ucClssFlag == 1 && glAmount[0] != 0)
    {
        memcpy(glProcInfo.stTranLog.szAmount, glAmount, 12);
#ifdef AMT_PROC_DEBUG
		//2014-9-18 each time come to glAmount or szAmount
		sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(26)%.13s",glAmount);
		sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(26)%.13s",glProcInfo.stTranLog.szAmount);
		glProcInfo.stTranLog.cntAmt++;
	//	//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 30: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
    }
    if( glProcInfo.stTranLog.szAmount[0]!=0)
    {
#ifdef AMT_PROC_DEBUG
			//2014-9-18 each time come to glAmount or szAmount
			sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(66)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(66)%.13s",glProcInfo.stTranLog.szAmount);
			glProcInfo.stTranLog.cntAmt++;
	//		//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 31: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
        if ((glSysParam.stEdcInfo.ucClssFlag == 1) && (glSysParam.stEdcInfo.ucClssMode == 2))
        {
            if (ConfirmAmount(NULL,glProcInfo.stTranLog.szAmount))
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }

    if (glProcInfo.ucEcrCtrl==ECR_BEGIN)
    {
        if (0==EcrGetAmount(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount))
        {			
#ifdef AMT_PROC_DEBUG
			//2014-9-18 each time come to glAmount or szAmount
			sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(27)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(27)%.13s",glProcInfo.stTranLog.szAmount);
			glProcInfo.stTranLog.cntAmt++;
	//		//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 32: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
            if (!ChkIfZeroAmt(glProcInfo.stTranLog.szAmount) ||
                !ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount))
            {
                memcpy(glProcInfo.stTranLog.szInitialAmount, glProcInfo.stTranLog.szAmount, sizeof(glProcInfo.stTranLog.szInitialAmount));
                PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
                if (!ValidBigAmount(szTotalAmt))
                {
                    return ERR_NO_DISP;
                }
//                if ((memcmp(glProcInfo.stTranLog.szAmount, szMaxAmt, 12) > 0))//2014-9-8 ECR sale limit should not be constrained by CLSS limit
//                {
//#if 0  //Jason 2014.08.21 11:45  
//                    return ERR_NO_DISP;
//#else
//                    return ERR_CLSS_AMT;
//#endif
//                }

//              if (!ConfirmAmount(NULL, szTotalAmt))
//              {
//                  return ERR_USERCANCEL;//build 0122c
//              }
//              if (!AllowDuplicateTran())
//              {
//                  return ERR_USERCANCEL;
//              }
                return 0;
            }
        }
    }

    while( 1 )
    {
        iRet = InputAmount(AMOUNT);
        if( iRet!=0 )
        {
            return iRet;
        }
        bTipManualInput = FALSE;

        if (!PPDCC_ChkIfDccAcq() && ChkIfNeedTip())
        {
            iRet = GetTipAmount();
            if( iRet!=0 )
            {
                return iRet;
            }
            bTipManualInput = TRUE;
        }

        memcpy(glProcInfo.stTranLog.szInitialAmount, glProcInfo.stTranLog.szAmount, sizeof(glProcInfo.stTranLog.szInitialAmount));
#ifdef AMT_PROC_DEBUG
			//2014-9-18 each time come to glAmount or szAmount
			sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(68)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(68)%.13s",glProcInfo.stTranLog.szAmount);
			glProcInfo.stTranLog.cntAmt++;
	//		//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 33: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
        //Lijy
        if(ChkIfCasinoMode())
        {
            memcpy(glProcInfo.stTranLog.szDebitInAmt, glProcInfo.stTranLog.szAmount, sizeof(glProcInfo.stTranLog.szAmount));
            AmtConvtToSettleAmtByExgRate(glProcInfo.stTranLog.szAmount,glSysParam.stEdcInfo.szExchgeRate);
        }
        //Lijy


        PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
        if (!ValidBigAmount(szTotalAmt))
        {
            continue;
        }
        if (bTipManualInput)
        {
            if (ConfirmAmount(NULL, szTotalAmt))
            {
                break;
            }
        }
        else
        {
            break;
        }

    }

    if( !AllowDuplicateTran() )
    {
        return ERR_USERCANCEL;
    }

    return 0;
}

void AmtConvtToSettleAmtByExgRate(uchar *pszAmt, uchar *psExgRate)
{
    uchar szExgRate[12+1];  // 汇率
    uchar szServiceFee[12+1]; //服务费率
    uchar szRateMultiFee[24+1]; //汇率*（1+服务费率）
    uchar szMultiAmt[24+1];   //金额*汇率*（1+服务费率）
    uchar szTotalAmtEx[24+1];  //将12字节金额扩展成24字节
    uchar szRound[12+1];   //折算后的金额四舍五入
    uchar   szIssuerName[10+1];

    uchar *pch;
    int i;
    int iPosPosi;  //汇率小数位数
    int iLen;

    if(pszAmt == NULL || psExgRate == NULL)
    {
        return;
    }

    memset(szExgRate, 0x00, sizeof(szExgRate));
    memset(szServiceFee, 0x00, sizeof(szServiceFee));
    memset(szRateMultiFee, 0x00, sizeof(szRateMultiFee));
    memset(szMultiAmt, 0x00, sizeof(szMultiAmt));
    memset(szTotalAmtEx, 0x00, sizeof(szTotalAmtEx));
    memset(szRound, 0x00, sizeof(szRound));

    pch = psExgRate;

    i=0;
    iPosPosi=0;
    while(*pch != '\0')
    {
        if(*pch != '.')  //找汇率的小数点，去掉小数点
        {
            szExgRate[i++]=*pch;
        }
        else
        {
            iPosPosi=0;
        }
        pch++;
        iPosPosi++;
    }
    AmtConvToBit4Format(szExgRate, 0);

    ConvIssuerName(glCurIssuer.szName, szIssuerName);
    if(!strcmp(szIssuerName, "JCB")) //(1+4%)*100%
    {
        memcpy(szServiceFee, "104", 3);
    }
    else if(!strcmp(szIssuerName, "MASTERCARD") || !strcmp(szIssuerName, "MC")\
            ||!strcmp(szIssuerName, "MASTER") || !strcmp(szIssuerName, "VISA"))
    {
        memcpy(szServiceFee, "103", 3);  //(1+3%)*100%
    }
    else
    {
        memcpy(szServiceFee, "000", 3);
    }
    AmtConvToBit4Format(szServiceFee, 0);
    PubAscMul(szExgRate, szServiceFee, szRateMultiFee);
    AmtConvToBit4Format(szRateMultiFee, 0);
    memmove(&szRateMultiFee[sizeof(szRateMultiFee)-13], szRateMultiFee, 12);
    memset(szRateMultiFee, '0', 12);
    memmove(&szTotalAmtEx[sizeof(szTotalAmtEx)-13], pszAmt, 12);
    memset(szTotalAmtEx, '0', 12);

    PubAscMul(szTotalAmtEx, szRateMultiFee, szMultiAmt);
    iLen = strlen((szMultiAmt));
    if(iLen >= iPosPosi+2)  //2为服务费的小数位数
    {
        if(szMultiAmt[iLen-(iPosPosi-1+2)] >= '5')
        {
            szRound[0] = '1';
        }
        else
        {
            szRound[0] = '0';
        }

        szMultiAmt[iLen-(iPosPosi-1+2)] = 0;
    }
    AmtConvToBit4Format(szRound, 0);
    AmtConvToBit4Format(szMultiAmt, 0);
    PubAscAdd(szMultiAmt, szRound, 12, szMultiAmt);
    //  memcpy(glProcInfo.stTranLog.szAmount, szMultiAmt, 12);
    memcpy(pszAmt, szMultiAmt, 12);
}


// 把不含小数点，不含ignore digit的数字串转换为ISO8583 bit4格式的12位ASCII数字串
void AmtConvToBit4Format(uchar *pszString, uchar ucIgnoreDigit)
{
    uint    uiLen;

    if (pszString == NULL)
    {
        return;
    }

    uiLen = (uint)strlen((char *)pszString);
    if( uiLen>=(uint)(12-ucIgnoreDigit) )
    {
        return;
    }

    // 前补0
    memmove(pszString+12-uiLen-ucIgnoreDigit, pszString, uiLen+1);
    memset(pszString, '0', 12-uiLen-ucIgnoreDigit);

    // 后补ignore digit个0
    for (uiLen=0; uiLen<ucIgnoreDigit; uiLen++)
    {
        strcat((char *)pszString, "0");
    }
}

int GetInstalPlan(void)
{
    int             ii, iRet;
    ulong           ulAmt, ulTemp;
    uchar           ucTemp, sBuff[64];
    MenuItem        stPlanItems[MAX_PLAN+1];
    INSTALMENT_PLAN stPlanList[MAX_PLAN+1], *pstPlan;

TAG_SELECT_PLAN:
    if (glProcInfo.stTranLog.ucTranType!=INSTALMENT)
    {
        return 0;
    }

    if (glProcInfo.ucEcrCtrl==ECR_BEGIN)
    {
        if (0==EcrGetInstPlan(&ucTemp))
        {
            for (ii=0; ii<glSysParam.ucPlanNum; ii++)
            {
                if (glSysParam.stPlanList[ii].ucMonths==ucTemp)
                {
                    glProcInfo.stTranLog.ucInstalment = ucTemp;
                    strcpy((char *)glProcInfo.stTranLog.szInstalProgID,    (char *)stPlanList[ii].szProgID);
                    strcpy((char *)glProcInfo.stTranLog.szInstalProductID, (char *)stPlanList[ii].szProductID);
                    break;
                }
            }
            if (glProcInfo.stTranLog.ucInstalment)  // if found
            {
                ulAmt = atol(glProcInfo.stTranLog.szAmount);
                if (stPlanList[ii].ulBottomAmt > ulAmt)
                {
                    App_ConvAmountLocal(glProcInfo.stTranLog.szAmount, sBuff, 0);
                    DispTransName();
                    PubDispString(_T("LOWER THAN"),   2|DISP_LINE_LEFT);
                    PubDispString(_T("LIMIT AMOUNT"), 4|DISP_LINE_LEFT);
                    PubDispString(sBuff,              6|DISP_LINE_LEFT);
                    PubBeepErr();
                    PubWaitKey(5);
                    return ERR_NO_DISP;
                }

                // MS-SCB
                ulTemp = atol(stPlanList[ii].szMaxAmount);
                if ((ulTemp>0) && (ulAmt>ulTemp))
                {
                    App_ConvAmountLocal(stPlanList[ii].szMaxAmount, sBuff, 0);
                    DispTransName();
                    PubDispString(_T("BIGGER THAN"),  2|DISP_LINE_LEFT);
                    PubDispString(_T("LIMIT AMOUNT"), 4|DISP_LINE_LEFT);
                    PubDispString(sBuff,              6|DISP_LINE_LEFT);
                    PubBeepErr();
                    PubWaitKey(5);
                    return ERR_NO_DISP;
                }

                iRet = MatchCardTableForInstalment(stPlanList[ii].ucAcqIndex);  // 重新决定ACQ
                if (iRet!=0)
                {
                    ScrClrLine(2, 7);
                    PubDispString(_T("UNSUPPORT CARD"), 4|DISP_LINE_LEFT);
                    PubBeepErr();
                    PubWaitKey(3);
                    return ERR_NO_DISP;
                }

                return 0;
            }
        }
    }

    // 生成installment plan菜单 install here
    memset(stPlanItems, 0, sizeof(stPlanItems));
    memset(stPlanList,  0, sizeof(stPlanList));
    for (ii=0; ii<glSysParam.ucPlanNum; ii++)
    {
        stPlanItems[ii].bVisible = TRUE;
        sprintf((char *)stPlanItems[ii].szMenuName, "%.*s - %d mths", NUM_MAXCHARS, (char *)glSysParam.stPlanList[ii].szName,glSysParam.stPlanList[ii].ucMonths);
        stPlanItems[ii].pfMenuFunc = NULL;
        stPlanList[ii] = glSysParam.stPlanList[ii];
    }
    stPlanItems[ii].bVisible = FALSE;
    sprintf((char *)stPlanItems[ii].szMenuName, "");
    stPlanItems[ii].pfMenuFunc = NULL;

    // 菜单选择
    iRet = PubGetMenu((uchar *)_T("SELECT PLAN"), stPlanItems, MENU_AUTOSNO|MENU_ASCII, USER_OPER_TIMEOUT);
    if (iRet<0)
    {
        return ERR_USERCANCEL;
    }
    pstPlan = &stPlanList[iRet];

    ulAmt = atol(glProcInfo.stTranLog.szAmount);
    ulTemp = pstPlan->ulBottomAmt;
    for (ii=0; ii<glSysParam.stEdcInfo.stLocalCurrency.ucDecimal; ii++)
    {
        ulTemp *= 10;   // the "bottom amount" do not contain decimals
    }
    if (ulTemp > ulAmt)
    {
        sprintf(sBuff, "%012lu", ulTemp);
        App_ConvAmountLocal(sBuff, sBuff+20, 0);
        DispTransName();
        PubDispString(_T("LOWER THAN"),   2|DISP_LINE_LEFT);
        PubDispString(_T("LIMIT AMOUNT"), 4|DISP_LINE_LEFT);
        PubDispString(sBuff+20,           6|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(5);
        goto TAG_SELECT_PLAN;
    }

    // MS-SCB
    ulTemp = atol(pstPlan->szMaxAmount);
    if ((ulTemp>0) && (ulAmt>ulTemp))
    {
        App_ConvAmountLocal(pstPlan->szMaxAmount, sBuff, 0);
        DispTransName();
        PubDispString(_T("BIGGER THAN"),  2|DISP_LINE_LEFT);
        PubDispString(_T("LIMIT AMOUNT"), 4|DISP_LINE_LEFT);
        PubDispString(sBuff,              6|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(5);
        goto TAG_SELECT_PLAN;
    }

    glProcInfo.stTranLog.ucInstalment = pstPlan->ucMonths;
    strcpy((char *)glProcInfo.stTranLog.szInstalProgID,    (char *)pstPlan->szProgID);
    strcpy((char *)glProcInfo.stTranLog.szInstalProductID, (char *)pstPlan->szProductID);

    iRet = MatchCardTableForInstalment(pstPlan->ucAcqIndex);    // 重新决定ACQ
    if (iRet!=0)
    {
        ScrClrLine(2, 7);
        PubDispString(_T("UNSUPPORT CARD"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(3);
        return ERR_NO_DISP;
    }

    return 0;
}


// Use local currency to call PubConvAmount
void App_ConvAmountLocal(uchar *pszIn, uchar *pszOut, uchar ucMisc)
{
    uchar   szBuff[12];

    memset(szBuff, 0, sizeof(szBuff));
    strcpy((char *)szBuff, (char *)glSysParam.stEdcInfo.stLocalCurrency.szName);
    if ((glSysParam.stEdcInfo.ucCurrencySymbol!=' ')
        && (glSysParam.stEdcInfo.ucCurrencySymbol!=0) )
    {
        szBuff[strlen(szBuff)] = glSysParam.stEdcInfo.ucCurrencySymbol;
    }

    PubConvAmount(szBuff, pszIn,
                  glSysParam.stEdcInfo.stLocalCurrency.ucDecimal,
                  glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit,
                  pszOut, ucMisc);
}

// Use transaction currency to call PubConvAmount (MAYBE different form local currency)
void App_ConvAmountTran(uchar *pszIn, uchar *pszOut, uchar ucMisc)
{
    uchar   szBuff[12];

    memset(szBuff, 0, sizeof(szBuff));
    if(!ChkIfCasinoMode())
    {
        strcpy((char *)szBuff, (char *)glProcInfo.stTranLog.stTranCurrency.szName);
    }


    PubConvAmount(szBuff, pszIn,
                  glProcInfo.stTranLog.stTranCurrency.ucDecimal,
                  glProcInfo.stTranLog.stTranCurrency.ucIgnoreDigit,
                  pszOut, ucMisc);
}

// 输入交易金额
int InputAmount(uchar ucAmtType)
{
    uchar   ucRet, ucMinLen, ucMaxLen, ucFlag, szCurrName[4+1];
    uchar   *pszAmt;

    PubASSERT( ucAmtType==AMOUNT    || ucAmtType==RFDAMOUNT ||
               ucAmtType==ORGAMOUNT || ucAmtType==TIPAMOUNT);

    ScrClrLine(2, 7);

    if (ChkIfTrainMode()&&glSysParam.stEdcInfo.ucClssFlag == 1)//training mode display
    {
        PubDispString(_T("* DEMO ONLY *"), 4|DISP_LINE_CENTER);
    }
#ifdef APP_MANAGER_VER
    if (glSysParam.stECR.ucMode&&glSysParam.stEdcInfo.ucClssFlag == 1)
    {
        ScrPrint(96, 5, ASCII, "ECR");
    }
#endif

    switch( ucAmtType )
    {
        case AMOUNT:
            if( ChkIfNeedTip() )
            {
                PubDispString(_T("BASE AMOUNT"), 2|DISP_LINE_LEFT);
            }
            else
            {
                PubDispString(_T("ENTER AMOUNT"), 2|DISP_LINE_LEFT);
            }
#ifdef PAYPASS_DEMO
            if (gucIsPayWavePass == PAYPASS)
            {
                ScrPrint(0,4,0,"PayPass Card");
            }
#endif
            break;

        case RFDAMOUNT: // RFU
            PubDispString(_T("REFUND AMOUNT"), 2|DISP_LINE_LEFT);
            break;

        case ORGAMOUNT: // RFU
            PubDispString(_T("ORIGINAL AMOUNT"), 2|DISP_LINE_LEFT);
            break;

        case TIPAMOUNT:
            PubDispString(_T("ENTER TIP"), 2|DISP_LINE_LEFT);
            break;
    }

    sprintf((char *)szCurrName, "%.3s", glSysParam.stEdcInfo.stLocalCurrency.szName);
//  sprintf((char *)szCurrName, "%.3s%1.1s", glSysParam.stEdcInfo.szCurrencyName, &glSysParam.stEdcInfo.ucCurrencySymbol);
    ucMinLen = (ucAmtType==TIPAMOUNT) ? 0 : 1;
    ucMaxLen = MIN(glSysParam.stEdcInfo.ucTranAmtLen, 10);
    ucFlag   = 0;
    if( glProcInfo.stTranLog.ucTranType==REFUND || ucAmtType==RFDAMOUNT )
    {
        ucFlag = GA_NEGATIVE;
    }

    if( ucAmtType==TIPAMOUNT )
    {
        pszAmt = glProcInfo.stTranLog.szTipAmount;
    }
    else
    {
        pszAmt = glProcInfo.stTranLog.szAmount;
    }
    ucRet = PubGetAmount(szCurrName, glProcInfo.stTranLog.stTranCurrency.ucDecimal,
                         ucMinLen, ucMaxLen, pszAmt, USER_OPER_TIMEOUT, ucFlag,0);
    if (glSysParam.stEdcInfo.ucClssFlag == 1 && (glSysParam.stEdcInfo.ucClssMode != 1))
    {
        if( ucRet!=0 )
        {
            return ucRet;
        }
    }
    else
    {
        if( ucRet!=0 )
        {
            return ERR_USERCANCEL;
        }
    }

    // Use transaction currency to do conversion
    AmtConvToBit4Format(pszAmt, glProcInfo.stTranLog.stTranCurrency.ucIgnoreDigit);
    return 0;
}

// 输入TIP金额
int GetTipAmount(void)
{
    int     iRet;
    uchar   szTotalAmt[12+1];
    // ChkIfNeedTip is moved outside


    while( 1 )
    {
        iRet = InputAmount(TIPAMOUNT);
        if( iRet!=0 )
        {
            return iRet;
        }

        PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
        if( ValidAdjustAmount(glProcInfo.stTranLog.szAmount, szTotalAmt) )
        {
            break;
        }
    }

    return 0;
}

// 检查调整金额是否合法(TRUE: 合法, FALSE: 非法)
// 金额必须为12数字
uchar ValidAdjustAmount(uchar *pszBaseAmt, uchar *pszTotalAmt)
{
    uchar   szMaxAmt[15+1], szAdjRate[3+1];

    if( memcmp(pszTotalAmt, pszBaseAmt, 12)<0 )
    {
        DispAdjustTitle();
        PubDispString(_T("AMOUNT ERROR"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(3);
        return FALSE;
    }

    if( glCurIssuer.ucAdjustPercent==0 || memcmp(pszTotalAmt, pszBaseAmt, 12)==0 )//0 for no limit squall 2013.12.09
    {
        return TRUE;
    }

    sprintf((char *)szAdjRate, "%ld", (ulong)(glCurIssuer.ucAdjustPercent+100));
    PubAscMul(pszBaseAmt, szAdjRate, szMaxAmt);
    PubAddHeadChars(szMaxAmt, 15, '0');
    if( memcmp(pszTotalAmt, &szMaxAmt[1], 12)>0 )   // 最后两位为小数,不进行比较
    {
        DispAdjustTitle();
        PubDispString(_T("OVER LIMIT"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(3);
        return FALSE;
    }




    return TRUE;
}

// amount should be less than 4294967296 (max of unsigned long)
uchar ValidBigAmount(uchar *pszAmount)
{
    int iLen;

    iLen = strlen(pszAmount);
    if (iLen<10)
    {
        return TRUE;
    }
    if (iLen>12)
    {
        return FALSE;
    }
    if (PubAsc2Long(pszAmount, iLen-3, NULL)>4294966)
    {
        return FALSE;
    }
    return TRUE;
}

// 确认金额界面处理
uchar ConfirmAmount(char *pszDesignation, uchar *pszAmount)
{
    if ((pszDesignation==NULL) || (pszDesignation[0]==0))
    {
        PubDispString(_T("TOTAL           "), 2|DISP_LINE_LEFT);
    }
    else
    {
        PubDispString(_T(pszDesignation), 2|DISP_LINE_LEFT);
    }

    DispAmount(4, pszAmount);
    PubDispString(_T("CORRECT ? Y/N"), DISP_LINE_LEFT|6);
    return !PubYesNo(USER_OPER_TIMEOUT);
}

uchar ConfirmAmount_PPDCC(uchar *pszDccAmount, CURRENCY_CONFIG *pstDccCurrency,
                          uchar *pszLocalAmount, CURRENCY_CONFIG *pstLocalCurrency)
{
    uchar   szBuff[32];

    ScrPrint(0, 2, ASCII, "TOTAL:");
    PubConvAmount(pstDccCurrency->szName, pszDccAmount, pstDccCurrency->ucDecimal, pstDccCurrency->ucIgnoreDigit, szBuff, 0);
    PubDispString(szBuff, 3|DISP_LINE_RIGHT);

    PubConvAmount(pstLocalCurrency->szName, pszLocalAmount, pstLocalCurrency->ucDecimal, pstLocalCurrency->ucIgnoreDigit, szBuff, 0);
    ScrPrint(0, 5, ASCII, "(%s)", szBuff);
    PubDispString(_T("CORRECT ? Y/N"), DISP_LINE_LEFT|6);
    return !PubYesNo(USER_OPER_TIMEOUT);
}

// 在指定行显示格式化的金额信息。注意金额是bit4格式，即可能含有ignore digit
// display formatted amount in specified line.
void DispAmount_Currency(uchar ucLine, uchar *pszAmount, CURRENCY_CONFIG *pstCurrency)
{
    uchar   ucFlag, ucFont, szOutAmt[30];

    ucFlag = 0;
    if( glProcInfo.stTranLog.ucTranType==VOID || glProcInfo.stTranLog.ucTranType==REFUND ||
        *pszAmount=='D' )
    {
        ucFlag |= GA_NEGATIVE;
        if( *pszAmount=='D' )
        {
            *pszAmount = '0';
        }
    }

    PubConvAmount(pstCurrency->szName, pszAmount, pstCurrency->ucDecimal, pstCurrency->ucIgnoreDigit,
                  szOutAmt, ucFlag);
    ucFont = strlen((char *)szOutAmt)>16 ? ASCII : CFONT;
    ScrPrint(0, ucLine, ucFont, "%*.21s", (ucFont==CFONT ? 16 : 21), szOutAmt);
}

void DispAmount(uchar ucLine, uchar *pszAmount)
{
    DispAmount_Currency(ucLine, pszAmount, &glSysParam.stEdcInfo.stLocalCurrency);
}

// 从UsrMsg取得卡号
// Format: "CARDNO=4333884001356283"
int GetManualPanFromMsg(void *pszUsrMsg)
{
    sprintf(glProcInfo.stTranLog.szPan, "%.19s", (char *)pszUsrMsg);
    if (!IsNumStr(glProcInfo.stTranLog.szPan))
    {
        return ERR_NO_DISP;
    }

    return VerifyManualPan();
}

// 手工输入PAN及其相关信息
int ManualInputPan(uchar ucInitChar)
{
    ScrClrLine(2, 7);
    PubDispString(_T("ENTER ACCOUNT NO"), 2|DISP_LINE_LEFT);

    memset(glProcInfo.stTranLog.szPan, 0, sizeof(glProcInfo.stTranLog.szPan));
    if( ucInitChar>='0' && ucInitChar<='9' )
    {
        glProcInfo.stTranLog.szPan[0] = ucInitChar;
    }
    if( PubGetString(NUM_IN|CARRY_IN, 13, LEN_PAN, glProcInfo.stTranLog.szPan, USER_OPER_TIMEOUT)!=0 )
    {
        return ERR_NO_DISP;
    }
    if (glProcInfo.bIsFallBack==TRUE)
    {
        glProcInfo.stTranLog.uiEntryMode = MODE_FALLBACK_MANUAL;
    }

    return VerifyManualPan();
}

int VerifyManualPan(void)
{
    int     iRet;

    glProcInfo.stTranLog.uiEntryMode = MODE_MANUAL_INPUT;

    DispTransName();
    iRet = MatchCardTable(glProcInfo.stTranLog.szPan,
                          glProcInfo.stTranLog.ucTranType,
                          0);
    if( iRet!=0 )
    {
        ScrClrLine(2, 7);
        PubDispString(_T("UNSUPPORT CARD"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(3);
        return ERR_NO_DISP;
    }

    if( !ChkIssuerOption(ISSUER_EN_MANUAL) )
    {
        PubDispString(_T("NOT ALLOW MANUL"), 4|DISP_LINE_LEFT);
        PubWaitKey(3);
        return ERR_NO_DISP;
    }

    iRet = ValidPanNo(glProcInfo.stTranLog.szPan);
    if( iRet!=0 )
    {
        return iRet;
    }

    iRet = GetExpiry();
    if( iRet!=0 )
    {
        return iRet;
    }
    if( !ChkEdcOption(EDC_NOT_MANUAL_PWD) )
    {
        if( PasswordMerchant()!=0 )
        {
            return ERR_USERCANCEL;
        }
    }
    CheckCapture();

    if (PPDCC_ChkIfDccAcq())
    {
        if (glProcInfo.stTranLog.ucDccType==0)
        {
            // If not set to MCP yet, then set to DCC now
            glProcInfo.stTranLog.ucDccType = PPTXN_DCC;
        }
    }

    //2014-5-14 enhancement for ECR & Modem connection
#ifdef _S60_
    ClssClose(); //2014-5-16 enhance
#endif
    iRet = ConfirmPanInfo();
    if( iRet!=0 )
    {
        CommOnHook(FALSE);
        return iRet;
    }
    iRet = GetSecurityCode();
    if( iRet!=0 )
    {
        return iRet;
    }

    return 0;
}

// 输入有效期
int GetExpiry(void)
{
    int     iRet;
    uchar   szExpDate[4+1];

    if( !ChkIssuerOption(ISSUER_EN_EXPIRY) )
    {
        sprintf((char *)glProcInfo.stTranLog.szExpDate, "1111");
        return 0;
    }

    while( 1 )
    {
        ScrCls();
        DispTransName();
        PubDispString(_T("EXP DATE: MMYY"), 2|DISP_LINE_LEFT);
        if( PubGetString(NUM_IN, 4, 4, szExpDate, USER_OPER_TIMEOUT)!=0 )
        {
            return ERR_USERCANCEL;
        }

        sprintf((char *)glProcInfo.stTranLog.szExpDate, "%.2s%.2s", &szExpDate[2], szExpDate);
        iRet = ValidCardExpiry();
        if( iRet==0 )
        {
            break;
        }
    }

    return 0;
}

// 输入商品描述信息
int GetDescriptor(void)
{
    uchar   ucCnt, ucKey, ucTotal, ucDesc, bInputDesc;

    if( !ChkIssuerOption(ISSUER_EN_DISCRIPTOR) )
    {		
        return 0;
    }

    if( glSysParam.ucDescNum==0 )
    {
        return 0;
    }
    if( glSysParam.ucDescNum==1 )
    {
		glProcInfo.stTranLog.szDescriptor[0] = '0';
        glProcInfo.stTranLog.ucDescTotal     = 1;
        return 0;
    }

    ScrCls();
    DispTransName();
    PubDispString(_T("PRODUCT CODE?"),  DISP_LINE_LEFT|2);

    for(bInputDesc=FALSE,ucTotal=0; ;)
    {
        ucKey = PubWaitKey(USER_OPER_TIMEOUT);
        if( ucKey==KEYCANCEL || ucKey==NOKEY )
        {
            return ERR_USERCANCEL;
        }
        else if( ucKey==KEYENTER && ucTotal>0 )
        {
            glProcInfo.stTranLog.ucDescTotal = ucTotal;
            return 0;
        }
        else if( ucKey>='0' && ucKey<='9' && ucTotal<MAX_GET_DESC )
        {
            ucDesc = (ucKey=='0') ? '9' : ucKey - 1;
            if( strchr((char *)glProcInfo.stTranLog.szDescriptor, ucDesc)==NULL &&
                ucDesc<glSysParam.ucDescNum+'0' )
            {
                glProcInfo.stTranLog.szDescriptor[ucTotal] = ucDesc;
                ucTotal++;
                bInputDesc = TRUE;
            }
        }
        else if ( ucKey==KEYCLEAR && ucTotal>0 )
        {
            glProcInfo.stTranLog.szDescriptor[--ucTotal] = 0;
        }
        if( bInputDesc )
        {
            ScrClrLine(4, 7);
            for(ucCnt=0; ucCnt<ucTotal; ucCnt++)
            {
                ucDesc = glProcInfo.stTranLog.szDescriptor[ucCnt] - '0';
                ScrPrint(0, (uchar)(4+ucCnt), ASCII, "%.21s", glSysParam.stDescList[ucDesc].szText);
            }
        }
    }
}

// 输入附加信息
// Input additional message.
int GetAddlPrompt(void)
{
    uchar   ucRet;

    if( !ChkAcqOption(ACQ_ADDTIONAL_PROMPT) && !ChkAcqOption(ACQ_AIR_TICKET) )
    {
        return 0;
    }

    ScrCls();
    DispTransName();
    PubDispString(glSysParam.stEdcInfo.szAddlPrompt, DISP_LINE_LEFT|4);

    ucRet = PubGetString(ALPHA_IN, 0, 16, glProcInfo.stTranLog.szAddlPrompt, USER_OPER_TIMEOUT);
    if( ucRet!=0 )
    {
        return ERR_USERCANCEL;
    }

    return 0;
}

// 输入PIN, ucFlag: bit 8, ICC online PIN
int GetPIN(uchar ucFlag)
{
    int     iRet;
#ifdef _P60_S1_
    uchar   ucLen;
#endif
    uchar   szPAN[16+1], szTotalAmt[12+1];
    uchar   ucPinKeyID;
    uchar   ucAmtFlag, szBuff[25];

#ifdef DEMO_HK
    // HK Demo 模式下必输入PIN
    if (glProcInfo.stTranLog.uiEntryMode & MODE_PIN_INPUT)
    {
        return 0;
    }
    if( !ChkIssuerOption(ISSUER_EN_PIN) )
    {
        return 0;
    }
#if defined(_S_SERIES_) || defined(_SP30_)
    if (ChkTermPEDMode(PED_INT_PCI))
    {
        SxxWriteKey(0, "1234567887654321", 16, DEF_PIN_KEY_ID, PED_TPK, NULL);
    }
#endif
#else
    SxxWriteKey(0, "1234567887654321", 16, DEF_PIN_KEY_ID, PED_TPK, NULL);
    // 非EMV PIN的模式下，如果是chip则直接返回，不是chip则检查ISSUER
    // in non-EMV-PIN mode, if it is chip transaction then return directly
    if( !(ucFlag & GETPIN_EMV) )
    {
        //2013-10-10  should disable this PIN ENTRY even protims set this bit,because the server does not vertify online pin

        if(1)//!ChkIfIndirectCupAcq()) //add by richard 20161115,  CUP acq need to input PIN
        {
            return 0;
        }
//      if ( (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) )
//      {
//          return 0;
//      }
//      else if( !ChkIssuerOption(ISSUER_EN_PIN) )
//      {
//          return 0;
//      }
    }
#endif

    //ucPinKeyID = DEF_PIN_KEY_ID;    //!!!! : 预留扩展：ACQUIRER可定义自己使用的ID
    ucPinKeyID = DEF_PIN_KEY_ID;   //modified by richard 20161115, use the downloaded TPK 

    iRet = ExtractPAN(glProcInfo.stTranLog.szPan, szPAN);
    if( iRet!=0 )
    {
        return iRet;
    }

    PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);

    ucFlag &= (uchar)(0xFF-GETPIN_EMV);
    ScrClrLine(2, 7);
    DispAmount(2, szTotalAmt);
    if( ucFlag==0 )
    {
        PubDispString(_T("PLS ENTER PIN"), 4|DISP_LINE_CENTER);
    }
    else
    {
        PubDispString(_T("PIN ERR, RETRY"), 4|DISP_LINE_CENTER);
    }

#ifndef _P60_S1_
    // show prompt message on PINPAD
    if (ChkTermPEDMode(PED_EXT_PP))
    {
        ucAmtFlag = 0;
        if( glProcInfo.stTranLog.ucTranType==VOID || glProcInfo.stTranLog.ucTranType==REFUND )
        {
            ucAmtFlag |= GA_NEGATIVE;
        }
        App_ConvAmountTran(szTotalAmt,  szBuff, ucFlag);
        // show amount on PINPAD
        PubDispString("(PLS USE PINPAD)", 6|DISP_LINE_CENTER);
        PPScrCls();
        PPScrPrint(0, 0, szBuff);
    }
#endif

    if (ChkTermPEDMode(PED_INT_PCI) || ChkTermPEDMode(PED_INT_PXX))
    {
        ScrGotoxy(32, 6);
    }

    memset(glProcInfo.sPinBlock, 0, sizeof(glProcInfo.sPinBlock));

    // !!!! to be check
#if defined(_S_SERIES_) || defined(_SP30_)
    //-------------- Internal PCI PED --------------
    if (ChkTermPEDMode(PED_INT_PCI))
    {

        iRet = PedGetPinBlock(ucPinKeyID, "0,4,5,6,7,8", szPAN, glProcInfo.sPinBlock, 0, USER_OPER_TIMEOUT*1000);
        if( iRet==0 )
        {
            glProcInfo.stTranLog.uiEntryMode |= MODE_PIN_INPUT;
            return 0;
        }
        else if (iRet==PED_RET_ERR_NO_PIN_INPUT)    // !!!! PIN bypass
        {
            return 0;
        }
        else if( iRet==PED_RET_ERR_INPUT_CANCEL )   // !!!! cancel input
        {
            return ERR_USERCANCEL;
        }

        DispPciErrMsg(iRet);
        PubBeepErr();
        PubWaitKey(3);
        return ERR_NO_DISP;
    }
#endif

    //-------------- External PCI PED --------------
    if (ChkTermPEDMode(PED_EXT_PCI))
    {
        // !!!! to be implemented
        ScrClrLine(2, 7);
        PubDispString("EXT PCI PINPAD ",  4|DISP_LINE_LEFT);
        PubDispString("NOT IMPLEMENTED.", 6|DISP_LINE_LEFT);
        PubWaitKey(30);
        return ERR_NO_DISP;
    }

    //-------------- PXX PED, and PINPAD --------------
#if defined(_P60_S1_) || defined(_P80_)
    if (ChkTermPEDMode(PED_INT_PXX))
    {
        iRet = PEDGetPwd_3Des(ucPinKeyID, 4, 6, szPAN, &ucLen, glProcInfo.sPinBlock);
    }
#endif

#ifndef _P60_S1_
    if (ChkTermPEDMode(PED_EXT_PP))
    {
        iRet = PPGetPwd_3Des(ucPinKeyID, 0x31, 4, 6, szPAN, glProcInfo.sPinBlock, 0);
        DispWelcomeOnPED();
    }
#endif

    if( iRet==0 )
    {
        glProcInfo.stTranLog.uiEntryMode |= MODE_PIN_INPUT;
        return 0;
    }
    else if( iRet==0x0A )
    {
        return 0;   // PIN by pass
    }
    else if( iRet==0x06 || iRet==0x07 )
    {
        return ERR_USERCANCEL;
    }

    DispPPPedErrMsg((uchar)iRet);

    return ERR_NO_DISP;
}

void DispWelcomeOnPED(void)
{
//  PPScrCls();
//  PPScrPrint(0, 0, (uchar *)"   WELCOME!   ");
//  PPScrPrint(1, 0, (uchar *)"PAX TECHNOLOGY");
}

// get partial pan for PIN entry
int ExtractPAN(uchar *pszPAN, uchar *pszOutPan)
{
    int     iLength;

    iLength = strlen((char*) pszPAN);
    if( iLength<13 || iLength>19 )
    {
        return ERR_SWIPECARD;
    }

    memset(pszOutPan, '0', 16);
    memcpy(pszOutPan+4, pszPAN+iLength-13, 12);
    pszOutPan[16] = 0;

    return 0;
}

// calculate mac
int GetMAC(uchar ucMode, uchar *psData, ushort uiDataLen, uchar ucMacKeyID, uchar *psOutMAC)
{
    uchar   ucRet;
    int     iRet;

    if( !ChkIfNeedMac() )
    {
        return 0;
    }

    // 0: ANSI x9.9, 1: fast arithm
#ifndef _P60_S1_
    if (ChkTermPEDMode(PED_EXT_PP))
    {
        ucRet = PPMac(ucMacKeyID, 0x01, psData, uiDataLen, psOutMAC, 0);    // !!!! mode 改造
        if( ucRet!=0 )
        {
            DispPPPedErrMsg(ucRet);
            return ERR_NO_DISP;
        }
    }
#endif

#if defined(_P60_S1_) || defined(_P80_) || defined(_P90_)
    if (ChkTermPEDMode(PED_INT_PXX))
    {
        ucRet = PEDMac(ucMacKeyID, 0x01, psData, uiDataLen, psOutMAC, 0);   // !!!! mode 改造
        if( ucRet!=0 )
        {
            DispPPPedErrMsg(ucRet);
            return ERR_NO_DISP;
        }
    }
#endif

#if defined(_S_SERIES_) || defined(_SP30_)
    if (ChkTermPEDMode(PED_INT_PCI))
    {
        iRet = PedGetMac(ucMacKeyID, psData, uiDataLen, psOutMAC, ucMode);
        if (iRet!=0)
        {
            DispPciErrMsg(iRet);
            PubBeepErr();
            PubWaitKey(3);
            return ERR_NO_DISP;
        }
        return 0;
    }
#endif

    if (ChkTermPEDMode(PED_EXT_PCI))
    {
        // External PCI
        // !!!! to be implemented
        ScrClrLine(2, 7);
        PubDispString("EXTERNAL PCI",     4|DISP_LINE_LEFT);
        PubDispString("NOT IMPLEMENTED.", 6|DISP_LINE_LEFT);
        PubWaitKey(30);
        return ERR_NO_DISP;
    }

    return 0;
}

// show PED/PINPAD error message
void DispPPPedErrMsg(uchar ucErrCode)
{
    switch( ucErrCode )
    {
        case 0x01:
            PubDispString(_T("INV MAC DATA"), 4|DISP_LINE_LEFT);
            break;

        case 0x02:
        case 0x0B:
            PubDispString(_T("INVALID KEY ID"), 4|DISP_LINE_LEFT);
            break;

        case 0x03:
            PubDispString(_T("INVALID MODE"), 4|DISP_LINE_LEFT);
            break;

        default:
            ScrClrLine(2, 7);
            ScrPrint(3, 4, CFONT, _T("PED ERROR:%02X"), ucErrCode);
            DispBox2();
            break;
    }

    PubBeepErr();
    PubWaitKey(3);
}

#if defined(_S_SERIES_) || defined(_SP30_)
void DispPciErrMsg(int iErrCode)
{
    char    szDispBuff[64];

    // to be implemented
    switch( iErrCode )
    {
        case PED_RET_OK:
            return;

        case PED_RET_ERR_NO_KEY:
            sprintf(szDispBuff, _T("KEY MISSING"));
            break;

        case PED_RET_ERR_KEYIDX_ERR:
            sprintf(szDispBuff, _T("INVALID KEY INDEX"));
            break;

        case PED_RET_ERR_DERIVE_ERR:
            sprintf(szDispBuff, _T("INVALID KEY LEVEL"));
            break;

        case PED_RET_ERR_CHECK_KEY_FAIL:
        case PED_RET_ERR_KCV_CHECK_FAIL:
            sprintf(szDispBuff, _T("CHECK KEY FAIL"));
            break;

        case PED_RET_ERR_NO_PIN_INPUT:
            sprintf(szDispBuff, _T("PIN BYPASS"));
            break;

        case PED_RET_ERR_INPUT_CANCEL:
        case PED_RET_ERR_INPUT_TIMEOUT:
            sprintf(szDispBuff, _T("INPUT CANCELLED"));
            break;

        case PED_RET_ERR_WAIT_INTERVAL:
            sprintf(szDispBuff, _T("PLS TRY AGAIN LATER"));
            break;

        case PED_RET_ERR_CHECK_MODE_ERR:
            sprintf(szDispBuff, _T("INVALID MODE"));
            break;

        case PED_RET_ERR_NO_RIGHT_USE:
        case PED_RET_ERR_NEED_ADMIN:
            sprintf(szDispBuff, _T("PED ACCESS DENIALED"));
            break;

        case PED_RET_ERR_KEY_TYPE_ERR:
        case PED_RET_ERR_SRCKEY_TYPE_ERR:
            sprintf(szDispBuff, _T("INVALID KEY TYPE"));
            break;

        case PED_RET_ERR_EXPLEN_ERR:
            sprintf(szDispBuff, _T("INVALID PARA"));
            break;

        case PED_RET_ERR_DSTKEY_IDX_ERR:
            sprintf(szDispBuff, _T("INVALID DST INDEX"));
            break;

        case PED_RET_ERR_SRCKEY_IDX_ERR:
            sprintf(szDispBuff, _T("INVALID SRC INDEX"));
            break;

        case PED_RET_ERR_KEY_LEN_ERR:
            sprintf(szDispBuff, _T("INVALID KEY LENGTH"));
            break;

        case PED_RET_ERR_NO_ICC:
            sprintf(szDispBuff, _T("CARD NOT READY"));
            break;

        case PED_RET_ERR_ICC_NO_INIT:
            sprintf(szDispBuff, _T("CARD NOT INIT"));
            break;

        case PED_RET_ERR_GROUP_IDX_ERR:
            sprintf(szDispBuff, _T("INVALID GROUP INDEX"));
            break;

        case PED_RET_ERR_PARAM_PTR_NULL:
            sprintf(szDispBuff, _T("INVALID PARA"));
            break;

        case PED_RET_ERR_LOCKED:
            sprintf(szDispBuff, _T("PED LOCKED"));
            break;

        case PED_RET_ERROR:
            sprintf(szDispBuff, _T("PED GENERAL ERR"));
            break;

        case PED_RET_ERR_UNSPT_CMD:
        case PED_RET_ERR_DUKPT_OVERFLOW:
        case PED_RET_ERR_NOMORE_BUF:
        case PED_RET_ERR_COMM_ERR:
        case PED_RET_ERR_NO_UAPUK:
        case PED_RET_ERR_ADMIN_ERR:
        case PED_RET_ERR_DOWNLOAD_INACTIVE:
        default:
            sprintf(szDispBuff, _T("PED ERROR:%d"), iErrCode);
            break;
    }

    ScrClrLine(2, 7);
    PubDispString(szDispBuff, 4|DISP_LINE_LEFT);
    DispBox2();
}
#endif

// 根据参数设置对PAN进行掩码
// Output a mask PAN
void MaskPan(uchar *pszInPan, uchar *pszOutPan)
{
    uchar   szBuff[30];
    int     iCnt, iPanLen, iOption;

    memset(szBuff, 0, sizeof(szBuff));
    iPanLen = strlen((char *)pszInPan);
    if( !ChkOptionExt(glCurIssuer.sPanMask, 0x0080) )   //the first bit of the 24 bits
    {// right align
        for(iCnt=0; iCnt<iPanLen; iCnt++)
        {
            iOption = ((2-iCnt/8)<<8) + (1<<(iCnt%8));
            if( !ChkOptionExt(glCurIssuer.sPanMask, (ushort)iOption) )
            {
                szBuff[iPanLen-iCnt-1] = pszInPan[iPanLen-iCnt-1];
            }
            else
            {
                szBuff[iPanLen-iCnt-1] = '*';
            }
        }
    }
    else
    {// left align
        for(iCnt=0; iCnt<iPanLen; iCnt++)
        {
            iOption = (((iCnt+2)/8)<<8) + (0x80>>((iCnt+2)%8));
            if( !ChkOptionExt(glCurIssuer.sPanMask, (ushort)iOption) )
            {
                szBuff[iCnt] = pszInPan[iCnt];
            }
            else
            {
                szBuff[iCnt] = '*';
            }
        }
    }

    sprintf((char *)pszOutPan, "%.*s", LEN_PAN, szBuff);
}

// 获取8583打包/解包错误信息
void Get8583ErrMsg(uchar bPackErr, int iErrCode, uchar *pszErrMsg)
{
    PubASSERT( iErrCode<0 );
    if( bPackErr )
    {
        if( iErrCode<-2000 )
        {
            sprintf((char *)pszErrMsg, "BIT %d DEF ERR", -iErrCode-2000);
        }
        else if( iErrCode<-1000 )
        {
            sprintf((char *)pszErrMsg, "BIT %d PACK ERR", -iErrCode-1000);//2014-9-15 ttt
        }
        else
        {
            sprintf((char *)pszErrMsg, "PACK HEADER ERR");
        }
    }
    else
    {
        if( iErrCode<-2000 )
        {
            sprintf((char *)pszErrMsg, "BIT %d DEF ERR", -iErrCode-2000);
        }
        else if( iErrCode<-1000 )
        {
            sprintf((char *)pszErrMsg, "UNPACK %d ERR", -iErrCode-1000);
        }
        else if( iErrCode==-1000 )
        {
            sprintf((char *)pszErrMsg, "DATA LENGTH ERR");
        }
        else
        {
            sprintf((char *)pszErrMsg, "UNPACK HEAD ERR");
        }
    }
}


// 获取预授权号码
int GetPreAuthCode(void)
{
    uchar   ucRet, szBuff[LEN_AUTH_CODE+1];
    uchar   ucAuthMinLen, ucAuthMaxLen;

    ScrClrLine(2, 7);
    ucAuthMinLen = 2;
    ucAuthMaxLen = LEN_AUTH_CODE;
    PubDispString(_T("APPV CODE ?"), 2|DISP_LINE_LEFT);
    memset(szBuff, 0, sizeof(szBuff));
    ucRet = PubGetString(ALPHA_IN, ucAuthMinLen, ucAuthMaxLen, szBuff, USER_OPER_TIMEOUT);
    if( ucRet!=0 )
    {
        return ERR_USERCANCEL;
    }

    sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%-*s", LEN_AUTH_CODE, szBuff);

    return 0;
}



//build88S 1.0C:
//**********************************************
//get RRN
//return: 0 - success, otherwise - fail
//**********************************************
int  GetRRN(void)
{
    uchar   ucRet, szBuff[LEN_RRN+1];

    ScrClrLine(2, 7);
    PubDispString(_T("ENTER RRN:"), 2|DISP_LINE_LEFT);
    memset(szBuff, 0, LEN_RRN+1);

    //allow RRN bypass
    ucRet = PubGetString(ALPHA_IN, 0, LEN_RRN, szBuff, USER_OPER_TIMEOUT);

    if(ucRet != 0)
    {
        return ERR_USERCANCEL;
    }

    sprintf((char *)glProcInfo.stTranLog.szRRN, "%-*s", LEN_RRN, szBuff);

    return 0;
}
//end build88S 1.0C



// 获取票据号
int InputInvoiceNo(ulong *pulInvoiceNo)
{
    uchar   ucRet, szBuff[LEN_INVOICE_NO+1];

    ScrClrLine(2, 7);
    PubDispString(_T("TRACE NO ?"), 2|DISP_LINE_LEFT);

    memset(szBuff, 0, sizeof(szBuff));
    ucRet = PubGetString(NUM_IN, 1, LEN_INVOICE_NO, szBuff, USER_OPER_TIMEOUT);
    if( ucRet!=0 )
    {
        return ERR_USERCANCEL;
    }
    *pulInvoiceNo = (ulong)atol((char *)szBuff);

    return 0;
}

// 获取要显示的交易状态信息
void GetStateText(ushort uiStatus, uchar *pszStaText)
{
    *pszStaText = 0;
    if( uiStatus & TS_VOID)
    {
        sprintf((char *)pszStaText,"VOIDED");
    }
    else if( uiStatus & TS_ADJ)
    {
        sprintf((char *)pszStaText, "ADJUSTED");
    }
    else if( uiStatus & TS_NOSEND)
    {
        sprintf((char *)pszStaText, "NOT_SEND");
    }
}

// 显示交易状态信息
void DispStateText(ushort uiStatus)
{
    if( uiStatus & TS_VOID )
    {
        PubDispString(_T("ALREADY VOID"), 4|DISP_LINE_LEFT);
    }
    else if( uiStatus & TS_ADJ )
    {
        PubDispString(_T("ALREADY ADJUST"), 4|DISP_LINE_LEFT);
    }
    else if( uiStatus & TS_NOSEND )
    {
        PubDispString(_T("NOT SEND"), 4|DISP_LINE_LEFT);
    }
}

void DispOkSub(uchar ucLogoNo)
{
    static uchar sLogoOkThree[117] =
    {
        0x03,
        0x00,0x24,
        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc0,0x40,0x0,0x40,0x0,0x40,0x0,0x40,0x0,
        0x40,0x0,0x40,0x0,0x40,0x0,0x80,0xc0,0xc0,0xc0,0xe0,0x60,0x60,0x30,0x30,0x30,
        0x10,0x10,0x10,0x0,

        0x00,0x24,
        0x0,0x4,0x4,0x4,0xc,0x18,0x18,0xba,0x70,0xe0,0xe0,0xc0,0xc0,0x80,0xc0,0xe0,
        0xf0,0xf8,0x7c,0x3e,0x1f,0xf,0x7,0xab,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
        0x0,0x0,0x0,0x0,

        0x00,0x24,
        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6,0x4,0x0,0x5,0x3,0x7,0x3,0x7,0x3,
        0x5,0x0,0x4,0x0,0x4,0x0,0x4,0x6,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
        0x0,0x0,0x0,0x0
    };
    static uchar sLogoOkTwo[117] =
    {
        0x03,
        0x00,0x24,
        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc0,0x40,0x0,0x40,0x0,0x40,0x0,0x40,0x0,
        0x40,0x0,0x40,0x0,0x40,0x0,0x40,0xc0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
        0x0,0x0,0x0,0x0,

        0x00,0x24,
        0x0,0x4,0x4,0x4,0xc,0x18,0x18,0xba,0x70,0xe0,0xe0,0xc0,0xc0,0x80,0xc0,0xe0,
        0xf0,0xf8,0x7c,0x38,0x10,0x0,0x0,0xaa,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
        0x0,0x0,0x0,0x0,

        0x00,0x24,
        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6,0x4,0x0,0x5,0x3,0x7,0x3,0x7,0x3,
        0x5,0x0,0x4,0x0,0x4,0x0,0x4,0x6,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
        0x0,0x0,0x0,0x0
    };
    static uchar sLogoOkOne[117] =
    {
        0x03,
        0x00,0x24,
        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc0,0x40,0x0,0x40,0x0,0x40,0x0,0x40,0x0,
        0x40,0x0,0x40,0x0,0x40,0x0,0x40,0xc0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
        0x0,0x0,0x0,0x0,

        0x00,0x24,
        0x0,0x4,0x4,0x4,0xc,0x18,0x18,0xba,0x70,0xe0,0xe0,0xc0,0xc0,0x80,0x0,0x0,
        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xaa,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
        0x0,0x0,0x0,0x0,

        0x00,0x24,
        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6,0x4,0x0,0x5,0x3,0x7,0x3,0x6,0x0,
        0x4,0x0,0x4,0x0,0x4,0x0,0x4,0x6,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
        0x0,0x0,0x0,0x0
    };

    ScrGotoxy(76, 3);
    if( ucLogoNo==0 )
    {
        ScrDrLogo(sLogoOkOne);
    }
    else if( ucLogoNo==1 )
    {
        ScrDrLogo(sLogoOkTwo);
    }
    else
    {
        ScrDrLogo(sLogoOkThree);
    }
}
//2016-4-25 add ==========================
int CLSSGetCard(void)
{
    int iRet;
    uchar ch;
    int iFlag = 0;

	uchar szCurrAmt[7];
    uchar floorAmt[12];
	
	PubLong2Bcd(glSysParam.stEdcInfo.ulODCVClssFLmt,4,szCurrAmt);
	PubBcd2Asc0(szCurrAmt,4,floorAmt);

    if (memcmp(glProcInfo.stTranLog.szAmount, floorAmt, 12) < 0)
    {
		ClssOpen();
		iRet = ClssProcTxn(glAmount, 1, glProcInfo.stTranLog.ucTranType);  	//Modify by Gillian 2016/1/29//2016-2-5

		if (iRet != 0 && iRet != SWIPED_MAGCARD && iRet != INSERTED_ICCARD)//2013-9-3 ttt
		{
			EcrSendTranFail();
		}
	}	
    return iRet;
}
// 显示操作成功的动画
// Show animation of "done"
void DispOperOk(void *pszMsg)
{
    uchar   ucLogoNo;

    PubDispString(pszMsg, 3|DISP_LINE_LEFT);

    TimerSet(TIMER_TEMPORARY, 2);
    for(ucLogoNo=0; ucLogoNo<3; )
    {
        if( TimerCheck(TIMER_TEMPORARY)==0 )
        {
            DispOkSub(ucLogoNo);
            ucLogoNo++;
            TimerSet(TIMER_TEMPORARY, 2);
        }
    }
    DelayMs(1500);
}

uchar AskYesNo(void)
{
    PubDispString("[ NO ]  [YES ]", DISP_LINE_CENTER|6);

    return !PubYesNo(USER_OPER_TIMEOUT);
}


// 选择收单行(for settle/reprint ....)
int SelectAcqMid(uchar *pszTitle, uchar *pucAcqIndex)
{
    uchar       ucCnt, ucIndex;
    MenuItem    stAcqMenu[MAX_ACQ+1+1];
    int         iSelMenu, iMenuNum;

    if( glSysParam.ucAcqNum==0 )
    {
        ScrCls();
        PubDispString(_T("NO DRIVER"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(3);
        return ERR_NO_DISP;
    }
    if( glSysParam.ucAcqNum==1 )
    {
        SetCurAcq(0);
        if( pucAcqIndex!=NULL )
        {
            *pucAcqIndex = 0;
        }
        return 0;
    }

    // here, glSysParam.ucAcqNum must >= 2
    memset(&stAcqMenu[0], 0, sizeof(stAcqMenu));
    iMenuNum = 0;

    for(ucCnt=0; ucCnt<glSysParam.ucAcqNum; ucCnt++)
    {
        stAcqMenu[iMenuNum].bVisible = TRUE;
        sprintf((char *)stAcqMenu[iMenuNum].szMenuName, "%.15s",
                glSysParam.stAcqList[ucCnt].szMerchantID);
        iMenuNum++;
    }

    iSelMenu = PubGetMenu(pszTitle, &stAcqMenu[0], MENU_CFONT, USER_OPER_TIMEOUT);
    if( iSelMenu<0 )
    {
        return ERR_USERCANCEL;
    }

    ucIndex = (uchar)iSelMenu;

    if( ucIndex!=MAX_ACQ )
    {
        SetCurAcq(ucIndex);
    }
    if( pucAcqIndex!=NULL )
    {
        *pucAcqIndex = ucIndex;
    }

    return 0;
}


// 选择收单行(for settle/reprint ....)
int SelectAcq(uchar bAllowSelAll, uchar *pszTitle, uchar *pucAcqIndex)
{
    uchar       ucCnt, ucIndex;
    MenuItem    stAcqMenu[MAX_ACQ+1+1];
    int         iSelMenu, iMenuNum;

    if( glSysParam.ucAcqNum==0 )
    {
        ScrCls();
        PubDispString(_T("NO ACQUIRER"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(3);
        return ERR_NO_DISP;
    }
    if( glSysParam.ucAcqNum==1 )
    {
        SetCurAcq(0);
        if( pucAcqIndex!=NULL )
        {
            *pucAcqIndex = 0;
        }
        return 0;
    }

    // here, glSysParam.ucAcqNum must >= 2
    memset(&stAcqMenu[0], 0, sizeof(stAcqMenu));
    iMenuNum = 0;
    if( bAllowSelAll )
    {
        stAcqMenu[iMenuNum].bVisible = TRUE;
        sprintf((char *)stAcqMenu[iMenuNum].szMenuName, "SELECT ALL");
        iMenuNum++;
    }

    for(ucCnt=0; ucCnt<glSysParam.ucAcqNum; ucCnt++)
    {
        stAcqMenu[iMenuNum].bVisible = TRUE;
        sprintf((char *)stAcqMenu[iMenuNum].szMenuName, "%3.3s %.10s",
                glSysParam.stAcqList[ucCnt].szNii, glSysParam.stAcqList[ucCnt].szName);
        iMenuNum++;
    }

    iSelMenu = PubGetMenu(pszTitle, &stAcqMenu[0], MENU_AUTOSNO|MENU_ASCII, USER_OPER_TIMEOUT);
    if( iSelMenu<0 )
    {
        return ERR_USERCANCEL;
    }
    if( bAllowSelAll )
    {
        ucIndex = (uchar)((iSelMenu==0) ? MAX_ACQ : iSelMenu-1);
    }
    else
    {
        ucIndex = (uchar)iSelMenu;		
    }

    if( ucIndex!=MAX_ACQ )
    {
        SetCurAcq(ucIndex);
    }
    if( pucAcqIndex!=NULL )
    {
        *pucAcqIndex = ucIndex;
    }

    return 0;
}

// 选择发卡行
int SelectIssuer(uchar *pucIssuerIndex)
{
    uchar       ucCnt, szTitle[16+1];
    int         iMenuNo;
    MenuItem    stMenuIssuer[MAX_ISSUER+1];

    if( glSysParam.ucIssuerNum==0 )
    {
        ScrCls();
        PubDispString(_T("NO ISSUER"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(3);
        return ERR_NO_DISP;
    }
    if( glSysParam.ucIssuerNum==1 )
    {
        SetCurIssuer(0);
        if( pucIssuerIndex!=NULL )
        {
            *pucIssuerIndex = 0;
        }
        return 0;
    }

    memset((char*)&stMenuIssuer, 0, sizeof(stMenuIssuer));
    for(ucCnt=0; ucCnt<glSysParam.ucIssuerNum; ucCnt++)
    {
        stMenuIssuer[ucCnt].bVisible = TRUE;
        sprintf((char *)stMenuIssuer[ucCnt].szMenuName, "%.10s", glSysParam.stIssuerList[ucCnt].szName);
    }

    sprintf((char *)szTitle, "%.16s", glSysParam.ucIssuerNum>9 ? "SELECT ISSUER:" : "SELECT ISSUER");
    iMenuNo = PubGetMenu(szTitle, stMenuIssuer, MENU_ASCII|MENU_AUTOSNO, USER_OPER_TIMEOUT);
    if( iMenuNo<0 )
    {
        return ERR_USERCANCEL;
    }
    SetCurIssuer((uchar)iMenuNo);
    if( pucIssuerIndex!=NULL )
    {
        *pucIssuerIndex = (uchar)iMenuNo;
    }

    return 0;
}

// 清除汇总信息
void ClearTotalInfo(void *pstTotalInfo)
{
    memset(pstTotalInfo, 0, sizeof(TOTAL_INFO));

    memset(((TOTAL_INFO *)pstTotalInfo)->szSaleAmt,       '0', 12);
    memset(((TOTAL_INFO *)pstTotalInfo)->szTipAmt,        '0', 12);
    memset(((TOTAL_INFO *)pstTotalInfo)->szRefundAmt,     '0', 12);
    memset(((TOTAL_INFO *)pstTotalInfo)->szVoidSaleAmt,   '0', 12);
    memset(((TOTAL_INFO *)pstTotalInfo)->szVoidRefundAmt, '0', 12);
}

// 显示交易汇总信息(glTransTotal)
int DispTransTotal(uchar bShowVoidTrans)
{
    uchar   szBuff[25];

    ScrCls();

    sprintf((char *)szBuff, _T("SALE%12d"),    glTransTotal.uiSaleCnt);
    PubDispString(szBuff, 0|DISP_LINE_LEFT);
    DispAmount(2, glTransTotal.szSaleAmt);

    sprintf((char *)szBuff, _T("REFUND%10d"),    glTransTotal.uiRefundCnt);
    PubDispString(szBuff, 4|DISP_LINE_LEFT);

    sprintf((char *)szBuff, "%.12s", glTransTotal.szRefundAmt);
    szBuff[0] = 'D';
    DispAmount(6, szBuff);

    if( PubYesNo(USER_OPER_TIMEOUT) )
    {
        return ERR_USERCANCEL;
    }
    if( !bShowVoidTrans )
    {
        return 0;
    }

    ScrCls();
    sprintf((char *)szBuff, _T("VOIDED SALE%5d"), glTransTotal.uiVoidSaleCnt);
    PubDispString(szBuff, 0|DISP_LINE_LEFT);

    sprintf((char *)szBuff, "%.12s", glTransTotal.szVoidSaleAmt);
    szBuff[0] = 'D';
    DispAmount(2, szBuff);

    sprintf((char *)szBuff, _T("VOIDED REFD%5d"), glTransTotal.uiVoidRefundCnt);
    PubDispString(szBuff, 4|DISP_LINE_LEFT);
    DispAmount(6, glTransTotal.szVoidRefundAmt);

    if( !PubYesNo(USER_OPER_TIMEOUT) )
    {
        return 0;
    }

    return ERR_USERCANCEL;
}

// 显示交易日志
void DispTranLog(ushort uiActRecNo, void *pTranLog)
{
    uchar       szBuff[25], szTotalAmt[12+1];
    TRAN_LOG    *pstLog;

    ScrCls();
    pstLog = (TRAN_LOG *)pTranLog;
    GetStateText(pstLog->uiStatus, szBuff);
    if( szBuff[0]!=0 )
    {
        PubDispString(_T(glTranConfig[pstLog->ucTranType].szLabel), DISP_LINE_RIGHT|0);
        ScrPrint(0,1, ASCII, "%-10.10s", szBuff);
    }
    else
    {
        PubDispString(_T(glTranConfig[pstLog->ucTranType].szLabel), DISP_LINE_CENTER|0);
    }

    sprintf((char *)szBuff, "%03d/%03d  TRACE:%06lu", uiActRecNo, GetTranLogNum(ACQ_ALL),
            pstLog->ulInvoiceNo);

    ScrPrint(0, 2, ASCII, "%.21s", szBuff);

    if( ChkIfDispMaskPan2() )
    {
        MaskPan(pstLog->szPan, szBuff);
        ScrPrint(0, 3, ASCII, "%.21s", szBuff);
    }
    else
    {
        ScrPrint(0, 3, ASCII, "%.21s", pstLog->szPan);
    }

    PubAscAdd(pstLog->szAmount, pstLog->szTipAmount, 12, szTotalAmt);
    App_ConvAmountTran(szTotalAmt,  szBuff, GetTranAmountInfo(pstLog));
    ScrPrint(0, 4, ASCII, "%.21s", szBuff);
    if (PPDCC_ChkIfDccAcq())
    {
        ScrPrint(96, 4, ASCII|REVER, " %.3s ", pstLog->stTranCurrency.szName);
    }
    ScrPrint(0, 5, ASCII, "APPROVAL CODE :%6.6s", pstLog->szAuthCode);
    Conv2EngTime(pstLog->szDateTime, szBuff);
    ScrPrint(0, 6, ASCII, "%.21s", szBuff);
    ScrPrint(0, 7, ASCII, "RRN     :%12.12s", pstLog->szRRN);
}


#ifdef SUPPORT_TABBATCH
void DispAuthLog(int iNow, int iTotal, void *pTranLog)
{
	uchar		szBuff[25], szTranName[21+1];
	TRAN_LOG	*pstLog;

	ScrCls();
	pstLog = (TRAN_LOG *)pTranLog;
    PubDispString(_T(glTranConfig[pstLog->ucTranType].szLabel), DISP_LINE_CENTER|0);

    if (iNow>0 && iTotal>0)
    {
	    sprintf((char *)szBuff, "%03d/%03d  TRACE:%06lu", iNow, iTotal, pstLog->ulInvoiceNo);
	    ScrPrint(0, 2, ASCII, "%.21s", szBuff);
    }

	MaskPan(pstLog->szPan, szBuff);
	ScrPrint(0, 3, ASCII, "%.21s", szBuff);

	App_ConvAmountTran(pstLog->szAmount,	szBuff, 0);
	ScrPrint(0, 4, ASCII, "%.21s", szBuff);
	ScrPrint(0, 5, ASCII, "APPROVAL CODE :%6.6s", pstLog->szAuthCode);
	Conv2EngTime(pstLog->szDateTime, szBuff);
	ScrPrint(0, 6, ASCII, "%.21s", szBuff);
	ScrPrint(0, 7, ASCII, "RRN     :%12.12s", pstLog->szRRN);
}

//// MS-SCB
//void MSSCB_DispTranLogDetail(ushort uiActRecNo, void *pTranLog)
//{
//	char		szBuff[25], szBuff2[25], szSign[2];
//	TRAN_LOG	*pstLog;
//
//	ScrClrLine(4, 7);
//	pstLog = (TRAN_LOG *)pTranLog;
//
//    memset(szSign, 0, sizeof(szSign));
//    if ((pstLog->uiStatus & TS_VOID) || (pstLog->ucTranType==VOID))
//    {
//        strcpy(szSign, "-");
//    }
//
//    strcpy(szBuff, (char *)(pstLog->szBPEarnPts));
//    App_TrimNumStr(szBuff);
//    strcpy(szBuff2, szSign);
//    strcat(szBuff2, szBuff);
//    ScrPrint(0, 4, ASCII, "EARN PTS   %10.10s", szBuff2);
//
//    App_ConvAmountLocal(pstLog->szBPEarnAmt, szBuff, 0);
//    strcpy(szBuff2, szSign);
//    strcat(szBuff2, szBuff);
//    ScrPrint(0, 5, ASCII, "EARN AMT %12.12s", szBuff2);
//
//    strcpy(szBuff, (char *)(pstLog->szBPRedeemPts));
//    App_TrimNumStr(szBuff);
//    strcpy(szBuff2, szSign);
//    strcat(szBuff2, szBuff);
//    ScrPrint(0, 6, ASCII, "REDEEM PTS %10.10s", szBuff2);
//
//    App_ConvAmountLocal(pstLog->szBPRedeemAmt, szBuff, 0);
//    strcpy(szBuff2, szSign);
//    strcat(szBuff2, szBuff);
//    ScrPrint(0, 7, ASCII, "AMT%18.18s", szBuff2);
//}
#endif

/*
// 调节屏幕对比度
void AdjustLcd(void)
{
    uchar   ucKey, szBuff[30];

    while( 1 )
    {
        PubShowTitle(TRUE, (uchar *)"ADJUST CONTRAST");
        sprintf((char *)szBuff, _T("STEP = [%d]"), glSysParam.stEdcInfo.ucScrGray);
        PubDispString(szBuff, DISP_LINE_CENTER|3);
        PubDispString(_T("[CANCEL] - EXIT"), DISP_LINE_CENTER|6);
        ScrGray(glSysParam.stEdcInfo.ucScrGray);
        ucKey = PubWaitKey(USER_OPER_TIMEOUT);
        if( ucKey==KEYCANCEL || ucKey==NOKEY )
        {
            break;
        }
        glSysParam.stEdcInfo.ucScrGray = (glSysParam.stEdcInfo.ucScrGray+1) % 8;
    }
    SaveEdcParam();
}
*/

// 判断是否为数字串
uchar IsNumStr(char *pszStr)
{
    if( pszStr==NULL || *pszStr==0 )
    {
        return FALSE;
    }

    while( *pszStr )
    {
        if( !isdigit(*pszStr++) )
        {
            return FALSE;
        }
    }

    return TRUE;
}

// 获取交易的英文名称
//void GetEngTranLabel(uchar *pszTranTitle, uchar *pszEngLabel)

// 取得金额的符号
uchar GetTranAmountInfo(void *pTranLog)
{
    uchar       ucSignChar;
    TRAN_LOG    *pstLog;

    pstLog = (TRAN_LOG *)pTranLog;
    ucSignChar = 0;
    if( pstLog->ucTranType==REFUND || pstLog->ucTranType==VOID || pstLog->uiStatus &TS_VOID )
    {
        ucSignChar = GA_NEGATIVE;
    }

    if( (pstLog->ucTranType==VOID) && (pstLog->ucOrgTranType==REFUND) )
    {
        ucSignChar = 0;
    }

    if( (pstLog->ucTranType==REFUND) && ( pstLog->uiStatus &TS_VOID))
    {
        ucSignChar = 0;
    }

    return ucSignChar;
}

void DispHostRspMsg(uchar *pszRspCode, HOST_ERR_MSG *pstMsgArray)
{
    int     iCnt;
    char    szDispMsg[64];

    for(iCnt=0; pstMsgArray[iCnt].szRspCode[0]!=0; iCnt++)
    {
        if( memcmp(pszRspCode, pstMsgArray[iCnt].szRspCode, 2)==0 )
        {
            break;
        }
    }

    sprintf(szDispMsg, (char *)(pstMsgArray[iCnt].szMsg));

    if( ChkIfBea() )
    {
        if( memcmp(glProcInfo.stTranLog.szRspCode, "08", 2)==0 ||
            memcmp(glProcInfo.stTranLog.szRspCode, "43", 2)==0 )
        {
            sprintf(szDispMsg, "PLS CALL BANK");
        }
    }

    PubDispString(_T(szDispMsg), 4|DISP_LINE_CENTER);
    ScrPrint(112, 7, ASCII, "%.2s", pszRspCode);
}

void DispResult(int iErrCode)
{
    int     iCnt;

    if( (iErrCode==ERR_NO_DISP) ||
        (iErrCode==ERR_EXIT_APP) )
    {
        return;
    }

    if( iErrCode!=0 )
    {
        ScrCls();
        DispTransName();
    }

    switch( iErrCode )
    {
        case 0:
            switch( glProcInfo.stTranLog.ucTranType )
            {
                case UPLOAD:
                case LOAD_PARA:
                case LOAD_CARD_BIN:
                case LOAD_RATE_REPORT:
                    apiBeef(3, 60);
                    break;

                case OFF_SALE:
                case VOID:
                    ScrCls();
                    DispTransName();
                    DispAccepted();
                    PubBeepOk();
                    PubWaitKey(glSysParam.stEdcInfo.ucAcceptTimeout);
                    break;

                case ECHO_TEST:
                    ScrCls();
                    DispTransName();
                    PubDispString(_T("LINK IS OK"), 4|DISP_LINE_LEFT);
                    PubBeepOk();
                    PubWaitKey(glSysParam.stEdcInfo.ucAcceptTimeout);
                    break;

                case LOGON:
                    PubDispString("LOGON COMPLETE", 4|DISP_LINE_LEFT);
                    PubBeepOk();
                    PubWaitKey(glSysParam.stEdcInfo.ucAcceptTimeout);
                    break;

                default:
                    ScrCls();
                    DispTransName();
					if ( ((memcmp(glProcInfo.stTranLog.szRspCode, "08", 2)==0) || (memcmp(glProcInfo.stTranLog.szRspCode, "88", 2)==0))
                              &&
                              ((glProcInfo.stTranLog.ucTranType==SALE) || (glProcInfo.stTranLog.ucTranType==AUTH) || (glProcInfo.stTranLog.ucTranType==PREAUTH))
                              &&
                              ChkIfAmex())
                    {
                        DispHostRspMsg(glProcInfo.stTranLog.szRspCode, glHostErrMsg_AE_Tran);
                    }
                    else if( glProcInfo.stTranLog.szAuthCode[0]==0 ||
                        memcmp(glProcInfo.stTranLog.szAuthCode, "       ", 6)==0 )
                    {
                        DispAccepted();
                    }                     
                    else
                    {
                        ScrPrint(0, 3, CFONT, _T("  APPV CODE"));
                        ScrPrint(0, 5, CFONT,    "  %.6s", glProcInfo.stTranLog.szAuthCode);
                    }
                    PubBeepOk();
                    PubWaitKey(glSysParam.stEdcInfo.ucAcceptTimeout);
            }
            break;

        case ERR_HOST_REJ:
            if (ChkIfAmex())
            {
                if (glProcInfo.stTranLog.ucTranType==SETTLEMENT)
                {
                    DispHostRspMsg(glProcInfo.stTranLog.szRspCode, glHostErrMsg_AE_Sett);
                }
                else
                {
                    DispHostRspMsg(glProcInfo.stTranLog.szRspCode, glHostErrMsg_AE_Tran);
                    if (memcmp(glProcInfo.stTranLog.szRspCode, "02", 2)==0)
                    {
                        ScrPrint(0, 6, CFONT, "   %.6s", glProcInfo.stTranLog.szAuthCode);
                    }
                }
            }
            else
            {
                DispHostRspMsg(glProcInfo.stTranLog.szRspCode, glHostErrMsg);
            }
            PubBeepErr();
            PubWaitKey(5);
            break;
            /*----------------2014-5-20 IP encryption----------------*/
#ifdef IP_ENCRYPT
        //Jerome
        case ERR_ENCRYPT:
            ScrCls();
            PubDispString("ENCRYPT ERROR", 4|DISP_LINE_LEFT);
            PubBeepErr();
            PubWaitKey(5);
            break;
            //end of Jerome

#endif
        /*----------------2014-5-20 IP encryption----------------*/
        default:
            for(iCnt=0; glTermErrMsg[iCnt].iErrCode!=0; iCnt++)
            {
                if( glTermErrMsg[iCnt].iErrCode==iErrCode )
                {
                    PubDispString(_T(glTermErrMsg[iCnt].szMsg), 4|DISP_LINE_CENTER);
                    break;
                }
            }
            if( glTermErrMsg[iCnt].iErrCode==0 )
            {
                PubDispString(_T("SYSTEM ERROR"), 4|DISP_LINE_LEFT);
                ScrPrint(80, 7, ASCII, "%04x", iErrCode);
            }
            PubBeepErr();
            PubWaitKey(5);
            break;
    }
}

// 计算单据数目
// Get number of receipts to pint out in one transaction.
int NumOfReceipt(void)
{
    int     iNum;

    iNum = 1;
    if( ChkEdcOption(EDC_NUM_PRINT_LOW) )
    {
        iNum++;
    }
    if( ChkEdcOption(EDC_NUM_PRINT_HIGH) )
    {
        iNum += 2;
    }

    return iNum;
}

// if any issuer ticked PIN REQUIRED option, it open the EMV offline PIN
// deleted
//void ModifyTermCapForPIN(void)

#ifdef ENABLE_EMV
// set tag 9C for EMV
void UpdateEMVTranType(void)
{
    // 设置EMV参数
    EMVGetParameter(&glEmvParam);
    switch( glProcInfo.stTranLog.ucTranType )
    {
        case SALE:
            glEmvParam.TransType = EMV_GOODS;
            EMVSetTLVData(0x9C, (uchar *)"\x00", 1);
            break;

        case AUTH:
            EMVSetTLVData(0x9C, (uchar *)"\x00", 1);
            glEmvParam.TransType = EMV_GOODS;
            break;

        case PREAUTH:
            EMVSetTLVData(0x9C, (uchar *)"\x30", 1);
            glEmvParam.TransType = EMV_GOODS;
            break;

        case CASH:  //add by richard 20161222, v1.00.0213P, JCB CASH EMV tag 9C wrong
            EMVSetTLVData(0x9C, (uchar *)"\x01", 1);
            glEmvParam.TransType = EMV_CASH;
            break;   

//  case BALANCE:
//      EMVSetTLVData(0x9C, (uchar *)"\x31", 1);
//      glEmvParam.TransType = EMV_GOODS;
//      break;

        default:
            return;
    }
    EMVSetParameter(&glEmvParam);
    // Only in this trasaction, so DON'T back up
}
#endif

int FindCurrency(uchar *pszCurrencyNameCode, CURRENCY_CONFIG *pstCurrency)
{
    int iCnt;
    uchar   sBCD[8], sBuff[8];

    for (iCnt=0; glCurrency[iCnt].szName[0]!=0; iCnt++)
    {
        if (IsNumStr(pszCurrencyNameCode))
        {
            sprintf((char *)sBuff, "0%.3s", pszCurrencyNameCode);
            PubAsc2Bcd(sBuff, 3, sBCD);
            if (memcmp(sBCD, glCurrency[iCnt].sCurrencyCode, 2)==0)
            {
                memcpy(pstCurrency, &glCurrency[iCnt], sizeof(CURRENCY_CONFIG));
                return 0;
            }
        }
        else
        {
            if (strcmp((char *)pszCurrencyNameCode, (char *)glCurrency[iCnt].szName)==0)
            {
                memcpy(pstCurrency, &glCurrency[iCnt], sizeof(CURRENCY_CONFIG));
                return 0;
            }
        }
    }

    return -1;
}

#ifdef ENABLE_EMV
// 根据EDC参数设定EMV库的国家代码和货币特性
// Setup EMV core parameter due to EDC para
void SyncEmvCurrency(uchar *psCountryCode, uchar *psCurrencyCode, uchar ucDecimal)
{
    EMVGetParameter(&glEmvParam);
    if ((memcmp(psCountryCode,  glEmvParam.CountryCode,   2)!=0) ||
        (memcmp(psCurrencyCode, glEmvParam.TransCurrCode, 2)!=0) ||
        (glEmvParam.TransCurrExp!=ucDecimal) )
    {
        memcpy(glEmvParam.CountryCode,   psCountryCode,  2);
        memcpy(glEmvParam.TransCurrCode, psCurrencyCode, 2);
        memcpy(glEmvParam.ReferCurrCode, psCurrencyCode, 2);
        glEmvParam.TransCurrExp = ucDecimal;
        glEmvParam.ReferCurrExp = ucDecimal;
        EMVSetParameter(&glEmvParam);
    }
}
#endif

// Read monitor config info, by API: GetTermInfo()
// return: 0--No need save; 1--Need save
// 读取monitor保存的系统配置信息
// 返回：0－－不需要保存更新；1－－需要保存
int UpdateTermInfo(void)
{
    int     iRet;
    uchar   ucNeedUpdate, sBuff[sizeof(glSysParam.sTermInfo)];

    ucNeedUpdate = 0;

#if defined(_P70_S_) || defined(_P60_S1_)

#else
    while (1)
    {
        memset(sBuff, 0, sizeof(sBuff));
        iRet = GetTermInfo(sBuff);
        if ( iRet<0 )
        {
#ifdef _WIN32
            ScrClrLine(2, 7);
            PubDispString(_T("CONNECT SIMULTR."), 4|DISP_LINE_LEFT);
            DelayMs(1000);
            continue;
#else
            SysHaltInfo(_T("FAIL GET SYSINFO"));
#endif
        }

        break;
    }

    if (memcmp(sBuff, glSysParam.sTermInfo, sizeof(glSysParam.sTermInfo))!=0)
    {
        memcpy(glSysParam.sTermInfo, sBuff, sizeof(glSysParam.sTermInfo));
        ucNeedUpdate = 1;
    }

#endif

    return ucNeedUpdate;
}

#if defined(_S_SERIES_) || defined(_SP30_) || defined(_P58_)
static void ShowMsgFontMissing(uchar bIsPrnFont, ST_FONT *psingle_code_font, ST_FONT *pmulti_code_font, int iErrCode)
{
    uchar   szBuff1[32], szBuff2[32];
    sprintf((char *)szBuff1, "%02d:%d*%d %s%s",
            psingle_code_font->CharSet,
            psingle_code_font->Width, psingle_code_font->Height,
            (psingle_code_font->Bold ? "B" : ""), (psingle_code_font->Italic ? "I" : ""));
    sprintf((char *)szBuff2, "%02d:%d*%d %s%s %d",
            pmulti_code_font->CharSet,
            pmulti_code_font->Width, pmulti_code_font->Height,
            (pmulti_code_font->Bold ? "B" : ""), (pmulti_code_font->Italic ? "I" : ""), iErrCode);

    ScrClrLine(2, 7);
    PubDispString(szBuff1, 2|DISP_LINE_LEFT);
    PubDispString(szBuff2, 4|DISP_LINE_LEFT);
    if (bIsPrnFont)
    {
        PubDispString("PRN FONT MISS", 6|DISP_LINE_LEFT);
    }
    else
    {
        PubDispString("DISP FONT MISS", 6|DISP_LINE_LEFT);
    }
    PubWaitKey(-1);
}
#endif

// Check whether system has fonts(for print and display) required under selected language.
// 在多语言动态切换环境下，检查系统是否有已选择语言所对应的打印和显示字库
int CheckSysFont(void)
{
#if defined(_S_SERIES_) || defined(_SP30_) || defined(_P58_)
    int     iRet, iRet1, iRet2, ii;

	ST_FONT	stPrnFonts[] = {
		{CHARSET_WEST, 8,  16, 0, 0},	{-1, 16, 16, 0, 0},
		{CHARSET_WEST, 12, 24, 0, 0},	{-1, 24, 24, 0, 0},
	};

	ST_FONT	stDispFonts[] = {
        {CHARSET_WEST, 8, 16, 0, 0}, {-1, 16, 16, 0, 0},
    };

    iRet = 0;
    // 检查是否有打印/显示字库 Check printer/display used fonts
    // for non-S60, display and print share the same fonts
    PrnInit();
    for (ii=0; ii<sizeof(stPrnFonts)/sizeof(ST_FONT); ii+=2)
    {
        if (stPrnFonts[ii+1].CharSet==-1)// 换成系统当前已选择的语言的编码
        {
            stPrnFonts[ii+1].CharSet = glSysParam.stEdcInfo.stLangCfg.ucCharSet;
        }

        iRet2 = PrnSelectFont(&stPrnFonts[ii], &stPrnFonts[ii+1]);
        if (iRet2)
        {
            ShowMsgFontMissing(TRUE, &stPrnFonts[ii], &stPrnFonts[ii+1], iRet2);
            iRet = -1;
        }
    }
    // 如果是S60，还要检查手机显示用字库 Check display-used fonts on handset of S60
    if (ChkTerm(_TERMINAL_S60_))
    {
        for (ii=0; ii<sizeof(stDispFonts)/sizeof(ST_FONT); ii+=2)
        {
            if (stDispFonts[ii+1].CharSet==-1)// 换成系统当前已选择的语言的编码
            {
                stDispFonts[ii+1].CharSet = glSysParam.stEdcInfo.stLangCfg.ucCharSet;
            }

            iRet1 = ScrSelectFont(&stDispFonts[ii], &stDispFonts[ii+1]);
            if (iRet1)
            {
                ShowMsgFontMissing(FALSE, &stDispFonts[ii], &stDispFonts[ii+1], iRet1);
                iRet = -1;
            }
        }
    }

    return iRet;

#else
    return 0;

#endif
}

// Enumerates all supported fonts in POS.
// 列举系统字库
void EnumSysFonts(void)
{
#if defined(_S_SERIES_) || defined(_SP30_) || defined(_P58_)
    int         iRet, ii;
    ST_FONT     stFontsList[16];
    MenuItem    stInfoList[16+1];

    PubShowTitle(TRUE, _T("ENUM SYS FONTS"));

    iRet = EnumFont(stFontsList, 16);
    if (iRet<1)
    {
        PubDispString(_T("ENUM FAILED"), 4|DISP_LINE_LEFT);
        PubWaitKey(USER_OPER_TIMEOUT);
        return;
    }

    memset(stInfoList, 0, sizeof(stInfoList));
    for (ii=0; ii<iRet; ii++)
    {
        stInfoList[ii].bVisible = TRUE;
        stInfoList[ii].pfMenuFunc = NULL;
        sprintf((char *)stInfoList[ii].szMenuName, "%02d. %02d-%02dx%02d,%d,%d",
                ii+1, stFontsList[ii].CharSet, stFontsList[ii].Width, stFontsList[ii].Height,
                stFontsList[ii].Bold, stFontsList[ii].Italic);
    }
    stInfoList[ii].bVisible = FALSE;
    stInfoList[ii].pfMenuFunc = NULL;
    stInfoList[ii].szMenuName[0] = 0;
    PubGetMenu(_T("ENUM FONTS"), stInfoList, MENU_USESNO|MENU_ASCII, USER_OPER_TIMEOUT);

#else
    PubDispString(_T("NOT SUPPORTED"), 4|DISP_LINE_LEFT);
    PubWaitKey(USER_OPER_TIMEOUT);

#endif

    return;
}

#if defined(_S_SERIES_) || defined(_SP30_)
int SxxWriteKey(uchar ucSrcKeyIdx, uchar *psKeyBCD, uchar ucKeyLen, uchar ucDstKeyId, uchar ucDstKeyType, uchar *psKCV)
{
    int         iRet;
    ST_KEY_INFO stKeyInfoIn;
    ST_KCV_INFO stKcvInfoIn;

    memset(&stKeyInfoIn, 0, sizeof(stKeyInfoIn));
    memset(&stKcvInfoIn, 0, sizeof(stKcvInfoIn));

    memcpy(stKeyInfoIn.aucDstKeyValue, psKeyBCD, ucKeyLen);
    stKeyInfoIn.iDstKeyLen   = ucKeyLen;
    stKeyInfoIn.ucDstKeyIdx  = ucDstKeyId;
    stKeyInfoIn.ucDstKeyType = ucDstKeyType;
    stKeyInfoIn.ucSrcKeyIdx  = ucSrcKeyIdx;
    stKeyInfoIn.ucSrcKeyType = PED_TMK;

    if (psKCV==NULL)
    {
        stKcvInfoIn.iCheckMode = 0;
    }
    else
    {
        stKcvInfoIn.iCheckMode = 1;
        stKcvInfoIn.aucCheckBuf[0] = 4;
        memcpy(stKcvInfoIn.aucCheckBuf+1, psKCV, 4);
    }

    iRet = PedWriteKey(&stKeyInfoIn, &stKcvInfoIn);
    return iRet;
}
#endif
// Load event map (for EPS use)
uchar FileReadEventMap(void)
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    int     iRet, ii;
    MULTIAPP_EVENTS MultiAppEventBak;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    iRet = PubFileRead(EPS_MULTIAPP_FILE, 0L, &MultiAppEventBak, sizeof(MULTIAPP_EVENTS));
    if (iRet!=0)
    {
        return 1;
    }

    MultiAppEvent.RePrn = MultiAppEventBak.RePrn;
    for (ii=0; ii<5; ii++)
    {
        if(MultiAppEvent.Timers[ii].AID[0] == 0xff)
        {
            MultiAppEvent.Timers[ii] = MultiAppEventBak.Timers[ii];
        }
    }

    return 0;
}

int ChkIfExistSysParam(void)
{
    return (fexist(SYS_FILE)>=0);
}
uchar ChkOnBase(void)
{
#ifdef _P60_S1_
    return OnBase();

#elif defined(_S_SERIES_)
    if (!ChkTerm(_TERMINAL_S60_))
    {
        return 0;
    }
    return OnBase();

#else
    return 0;

#endif
}

void SetOffBase(unsigned char (*Handle)())
{
#ifdef _P60_S1_
    SetOffBaseProc(Handle);
#elif defined(_S_SERIES_)
    if (!ChkTerm(_TERMINAL_S60_))
    {
        return;
    }
    if (Handle==NULL)
    {
        //ScrPrint(0,0,0," SET NULL ");DelayMs(500);ScrPrint(0,0,0,"          ");
        ScrSetEcho(0);
        return;
    }
    //ScrPrint(0,0,0," SET ECHO ");DelayMs(500);ScrPrint(0,0,0,"          ");
    ScrSetOutput(2);
    (*Handle)();
    ScrSetEcho(2);
    ScrSetOutput(1);
    ScrSetEcho(1);

#else
    return;

#endif
}

uchar OffBaseDisplay(void)
{
#ifdef _P60_S1_
    if( OnBase()==0 )
    {
        return 0;
    }
    ScrRestore(0);
    ScrClrLine(2, 7);
    PubDispString(_T("RETURN TO BASE"), 4|DISP_LINE_LEFT);
    PubBeepErr();

    TimerSet(TIMER_TEMPORARY, 30);
    while( OnBase()!=0 )
    {
        if( TimerCheck(TIMER_TEMPORARY)==0 )
        {
            TimerSet(TIMER_TEMPORARY, 10);
            Beef(6, 200);
        }
    }
    kbflush();
    ScrRestore(1);
    return 0;

#elif defined(_S_SERIES_)
    ScrClrLine(2, 7);
    PubDispString(_T("RETURN TO BASE"), 4|DISP_LINE_LEFT);
    return 0;

#else
    return 0;

#endif
}
#ifdef APP_MANAGER_VER

void ResetSysParam(uchar bResetAll)
{
#if !defined(_P60_S1_) && !defined(_P70_S_)
    int         iRet;
#endif
    SYS_PARAM   stParaBak;

    if (!bResetAll)
    {
        stParaBak = glSysParam;
    }

    memset(&glSysParam, 0, sizeof(glSysParam));

    // Basic parameters
    glSysParam.ucKbSound = TRUE;    // Enable keyboard sound
    glSysParam.ucIdleMinute = 10;   // Count in minutes.

    // Auto select language.
    glSysParam.ucLang = LANG_ENG;

    // Init manager password
//  ResetManagerPassword();

    // Get hardware infomation
#if !defined(_P60_S1_) && !defined(_P70_S_)
    iRet = GetTermInfo(glSysParam.sTermInfo);
#endif

    // Recover if not first time
    if (!bResetAll)
    {
        glSysParam.ucKbSound = stParaBak.ucKbSound;
        glSysParam.ucIdleMinute = stParaBak.ucIdleMinute;
        glSysParam.ucIdleShutdown = stParaBak.ucIdleShutdown;
    }

    SaveSysParam();
}
#endif
uchar FileWriteEventMapDefault(void)
{
    memset(&MultiAppEvent, (uchar)0xFF, sizeof(MULTIAPP_EVENTS));
    return PubFileWrite(EPS_MULTIAPP_FILE, 0L, (uchar *)&MultiAppEvent, sizeof(MULTIAPP_EVENTS));
}
uchar OffBaseCheckPrint(void)
{
#ifdef _P60_S1_
    if( OnBase()==0 )
    {
        return 0;
    }

    ScrRestore(0);
    ScrClrLine(2, 7);
    PubDispString(_T("RETURN TO BASE"), 4|DISP_LINE_LEFT);
    PubBeepErr();

    TimerSet(TIMER_TEMPORARY, 30);
    while( OnBase()!=0 )
    {
        if( PubKeyPressed() )
        {
            if( getkey()==KEYCANCEL )
            {
                PubDispString(_T("  PRINT CANCEL  "), 4|DISP_LINE_LEFT);
                DelayMs(1000);
                kbflush();
                return KEYCANCEL;
            }
        }
        if( TimerCheck(TIMER_TEMPORARY)==0 )
        {
            TimerSet(TIMER_TEMPORARY, 10);
            Beef(6, 200);
        }
    }
    kbflush();
    ScrRestore(1);
    return 0;

#elif defined(_S_SERIES_)
    if( OnBase()==0 )
    {
        return 0;
    }
    return OffBaseDisplay();

#else
    return 0;

#endif
}

uchar OffBaseCheckEcr(void)
{
#ifdef _P60_S1_
    if( OnBase()==0 )
    {
        return 0;
    }

    ScrRestore(0);
    ScrClrLine(2, 7);
    PubDispString(_T("RETURN TO BASE"), 4|DISP_LINE_LEFT);
    PubBeepErr();

    TimerSet(TIMER_TEMPORARY, 30);
    while(OnBase()!=0)
    {
        if( PubKeyPressed() )
        {
            if( getkey()==KEYCANCEL )
            {
                PubDispString(_T(" ECR SEND CANCEL"), 4|DISP_LINE_LEFT);
                DelayMs(1000);
                kbflush();
                return KEYCANCEL;
            }
        }
        if( TimerCheck(TIMER_TEMPORARY)==0 )
        {
            TimerSet(TIMER_TEMPORARY, 10);
            Beef(6, 200);
        }
    }
    kbflush();
    ScrRestore(1);

    return 0;

#elif defined(_S_SERIES_)
    if( OnBase()==0 )
    {
        return 0;
    }
    return OffBaseDisplay();

#else
    return 0;

#endif
}
// ﹚︽陪ボΑて肂獺
void DispAmountASCII(uchar ucLine, uchar *pszAmount)
{
    uchar   ucFlag, szOutAmt[30];

    ucFlag = 0;
    if( glProcInfo.stTranLog.ucTranType==VOID || glProcInfo.stTranLog.ucTranType==REFUND ||
        *pszAmount=='D' )
    {
        ucFlag |= GA_NEGATIVE;
        if( *pszAmount=='D' )
        {
            *pszAmount = '0';
        }
    }

    //PubConvAmount(glSysParam.stEdcInfo.szCurrencyName, pszAmount, 2, szOutAmt, ucFlag);
    PubConvAmount(glSysParam.stEdcInfo.stLocalCurrency.szName, pszAmount, 2, 0,
                  szOutAmt, ucFlag);
    //ucFont = strlen((char *)szOutAmt)>16 ? ASCII : CFONT;
    ScrPrint(0, ucLine, ASCII, "%*.21s", 16, szOutAmt);
}
// end of file

