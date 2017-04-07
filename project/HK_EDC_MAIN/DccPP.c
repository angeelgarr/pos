/************************************************************************
NAME
    DccPP.c - Definitions of the PlanetPayment DCC public functions

DESCRIPTION
	PlanetPayment DCC is used by several banks. it has 2 basic modes: single-batch / dual-batch
	For dual-batch mode, a DCC acquirer is followed by a non-DCC-acquirer.

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    HeJiajiu(Gary Ho)     2009.11.04      - draft
************************************************************************/
#include "global.h"

/********************** Internal macros declaration ************************/
#define FILE_PP_LOCALBIN	"CARD_PLANETDCC"    // DCC card-bin file name
#define FILE_DCC_STAT       "DCC_STAT"          // DCC
#define MAX_DCCLOCALBIN		2000                // Max entries
#define SIZE_PDSDATA		800

/********************** Internal structure declaration *********************/
// structure of PDS sub-field
typedef struct _tagDCC_PDS 
{
	int		iLen;
	char	szTag[2+1];
	uchar	sBuff[SIZE_PDSDATA];
}DCC_PDS;

// for next struct use only
typedef struct _tagDCC_CARDBIN_BLOCK
{ 
	uchar sBinLow[5];		// n-10
	uchar sBinHigh[5];		// n-10
	uchar sCurrCode[2];		// n-3
}DCC_CARDBIN_BLOCK;

// structure for storing whole local BIN data. This is the structure stored in the file
typedef struct _tagDCC_CARDBIN_STORAGE
{
	uchar	ucPPlen[4];
	int		iTotalBlock;		// Total number of blocks
	uchar	sLastDate[4];		// YYYYmmdd
	DCC_CARDBIN_BLOCK stCards[MAX_DCCLOCALBIN];
}DCC_CARDBIN_STORAGE;

typedef struct {
    uchar   sCurrencyCode[2];
    int     iOptInCount;
    uchar   szOptInAmount[12+1];
    int     iOptOutCount;
    uchar   szOptOutAmount[12+1];
}STAT_UNIT;

typedef struct _tagDCC_STAT 
{
    STAT_UNIT   astCurrency[100];
    ushort      usReverCnt;
}DCC_STAT;

/********************** Internal constants declaration *********************/
static char	sPPDccAcqNameList[][10+1] = {
	"DCC_SCB",
	"DCC_HASE",
	"DCC_BEA",
	"DCC_HSBC",
	"ZDCC",
	"DCC_HSBC",
	"DCC_GP",
	"DCC_CITI",
	"DCC_ICBC"//2015-8-24 for ICBC DCC txn
};

/********************** Internal functions declaration *********************/
static int PackDccPDS(char * pszTag, uchar *psSrc, uchar ucSrcLen, uchar *psDest);
static int PackDccField62or63(char *pszTag, uchar *psDest);
static int GetDccPDS(uchar *psIn, DCC_PDS *pstPDS);

static void DownloadBinTbl_Manual(void);
static void VerifyLocalCard(void);
static void SetupBinAutoDownload(void);
static void RemoveLocalBinFile(void);
static void AutoLoadBinGenNextRemindDate(void);
static int  TransDownloadBin(uchar bAutoDownload);
static int  UnpackCardBin(uchar *pFieldIn, DCC_CARDBIN_STORAGE *pstCardBinStore);


/********************** Internal variables declaration *********************/
static DCC_CARDBIN_STORAGE	sg_stCardBin;
static DCC_STAT sg_DccStat;

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

void PPDCC_SelfTest(void)
{
#if 0
    {
        uchar sBuff[64], ucPort;

        PPDCC_CalcLocalAmount("000000100000", &glCurrency[5], "51428571", &glCurrency[0], sBuff);

        ucPort = glSysParam._TxnRS232Para.ucPortNo;
        GetTermInfo(sBuff);
        if (sBuff[HWCFG_MODEL]==_TERMINAL_S80_)
        {
	        glSysParam._TxnRS232Para.ucPortNo = PINPAD;
        }
        else
        {
            glSysParam._TxnRS232Para.ucPortNo = 1;
        }
        if (ucPort!=glSysParam._TxnRS232Para.ucPortNo)
        {
            SaveSysParam();
        }
    }
#endif
}

// Check if current is Planet Payment DCC acquirer. Lookup in the DCC-acquirer-name list
uchar PPDCC_ChkIfDccAcq(void)
{
	int	ii;

	for (ii=0; ii<sizeof(sPPDccAcqNameList)/sizeof(sPPDccAcqNameList[0]); ii++)
	{
		if (ChkCurAcqName(&sPPDccAcqNameList[ii][0], FALSE))
		{
			return TRUE;
		}
	}

	return FALSE;
}

// Check whether have any PP DCC acquirer in EDC
uchar PPDCC_ChkIfHaveDccAcq(void)
{
    int ii;
    ACQUIRER    stBakAcq = glCurAcq;

    for (ii=0; ii<glSysParam.ucAcqNum; ii++)
    {
        SetCurAcq((uchar)ii);
        if (PPDCC_ChkIfDccAcq())
        {
            glCurAcq = stBakAcq;
            return TRUE;
        }
    }

    glCurAcq = stBakAcq;
    return FALSE;
}

// if there's a PP DCC acquirer, set to it. return 0 when succeed.
uchar PPDCC_SetCurAcq(void)
{
	int	ii;
	ACQUIRER	stTempAcq;

	memcpy(&stTempAcq, &glCurAcq, sizeof(ACQUIRER));
	for (ii=0; ii<MAX_ACQ; ii++)
	{
	 	if (glSysParam.stAcqList[ii].ucKey!=INV_ACQ_KEY)
		{
			SetCurAcq((uchar)ii);
			if (PPDCC_ChkIfDccAcq())
			{
				return 0;
			}
		}
	}

	memcpy(&glCurAcq, &stTempAcq, sizeof(ACQUIRER));
	return -1;
}

// This is called after parameter download finished.
uchar PPDCC_SetDccDefaults(void)
{    
  SetOptionExt(glSysParam.stEdcInfo.sOption, EDC_ENABLE_DCC, OPT_SET);
	SetOptionExt(glSysParam.stEdcInfo.sOption, EDC_FREE_PRINT, OPT_SET);
    
  glSysParam.stEdcInfo.uiDccAutoLoadBinPeriod = 999;    
  AutoLoadBinGenNextRemindDate();
    
  return 0;
}

char *GetCurrencyName(uchar *pCurrencyCode)
{
	int     ii;
	uchar   buffer[5];
	
	if (pCurrencyCode[0]>='0' && pCurrencyCode[0]<='9')
	{
		PubAsc2Bcd(pCurrencyCode, 3, buffer);
	}
	else
	{
		memcpy(buffer, pCurrencyCode, 2);
	}

	for (ii=0; glCurrency[ii].szName[0]; ii++)
	{
		if (memcmp(buffer, glCurrency[ii].sCurrencyCode, 2)==0)
		{
			break;
		}
	}

	return (char *)(glCurrency[ii].szName);
}


/************************************************************************/
/*                                                                      */
/************************************************************************/


// return the packed length
static int PackDccPDS(char * pszTag, uchar *psSrc, uchar ucSrcLen, uchar *psDest)
{
	PubASSERT(ucSrcLen!=0 && pszTag!=NULL && psSrc!=NULL && psDest!=NULL);
	PubLong2Bcd(ucSrcLen+2, 2, psDest);
	memcpy(psDest+2, pszTag, 2);
	memcpy(psDest+4, psSrc, ucSrcLen);
	return ucSrcLen+4;
}

// Pack PP's DCC PDS field
// If psSrc==NULL, then let the function itself to determine the pack content
static int PackDccField62or63(char *pszTag, uchar *psDest)
{
	ulong	ulTag;
	uchar	sBuff[128];

	PubASSERT(pszTag!=NULL && psDest!=NULL);

	ulTag = atol(pszTag);
	switch(ulTag)
	{
	case 12:
		if (glProcInfo.stTranLog.ucDccType==PPTXN_DCC)
		{
			return PackDccPDS("12", "D", 1, psDest);
		}
		else if (glProcInfo.stTranLog.ucDccType==PPTXN_MCP)
		{
			return PackDccPDS("12", "M", 1, psDest);
		}
		else
		{
			return PackDccPDS("12", "X", 1, psDest);
		}
	case 13:
		if (glProcInfo.stTranLog.ucDccType==PPTXN_OPTOUT)
		{
			return PackDccPDS("13", "Y", 1, psDest);
		}
		else
		{
			return PackDccPDS("13", "N", 1, psDest);
		}
	case 16:
		memcpy(sBuff,  "10",2);
		memset(sBuff+2,' ', 4);
		memcpy(sBuff+2, glProcInfo.szSecurityCode, MIN(3, strlen(glProcInfo.szSecurityCode)));
		return PackDccPDS("16", sBuff, 6, psDest);
	case 27:
		return PackDccPDS("27", "R", 1, psDest);
	case 54:
		memset(sBuff,' ',29);
		return PackDccPDS("54", sBuff, 29, psDest);
	}

	return 0;
}

// Get PDS data from DCC private field 63
// The buffer size of psData is fixed to 128
static int GetDccPDS(uchar *psIn, DCC_PDS *pstPDS)
{
	int		iPDSLen;

	PubASSERT(psIn!=NULL && pstPDS!=NULL);

	memset(pstPDS, 0, sizeof(DCC_PDS));

	iPDSLen = (int)PubBcd2Long(psIn, 2, NULL);
	if ((iPDSLen>sizeof(pstPDS->sBuff)) || (iPDSLen<2))
	{
		return -1;
	}

	sprintf(pstPDS->szTag, "%.2s", (char *)psIn+2);
	memcpy(pstPDS->sBuff, psIn+4, iPDSLen-2);
	pstPDS->iLen = iPDSLen-2;
	return 0;
}

int PPDCC_PackField62(uchar ucTranType, void *psSendPack)
{
    // PYC participation report
    uchar   *ptr;
    int     ii, iLen;
    STISO8583 *pstPack = (STISO8583 *)psSendPack;

    PPDCC_GetAllTransStatistic();

    ptr = pstPack->sField62 + 2;

    memcpy(ptr, "V02", 3);
    ptr += 3;
    memcpy(ptr, sg_DccStat.astCurrency[0].sCurrencyCode, 2);
    ptr += 2;
    PubLong2Bcd(sg_DccStat.astCurrency[0].iOptOutCount, 2, ptr);
    ptr += 2;
    PubAsc2Bcd(sg_DccStat.astCurrency[0].szOptOutAmount, 12, ptr);
    ptr += 6;

    for (ii=1; glCurrency[ii].szName[0] && ii<30; ii++)  // length = 13 + 18*n
    {
        if (memcmp(sg_DccStat.astCurrency[ii].sCurrencyCode, "\x00\x00", 2)==0)
        {
            break;
        }
        if ((sg_DccStat.astCurrency[ii].iOptInCount==0) &&
            (sg_DccStat.astCurrency[ii].iOptOutCount==0))
        {
            continue;
        }
        memcpy(ptr, sg_DccStat.astCurrency[ii].sCurrencyCode, 2);
        ptr += 2;
        PubLong2Bcd(sg_DccStat.astCurrency[ii].iOptOutCount, 2, ptr);
        ptr += 2;
        PubAsc2Bcd(sg_DccStat.astCurrency[ii].szOptOutAmount, 12, ptr);
        ptr += 6;
        PubLong2Bcd(sg_DccStat.astCurrency[ii].iOptInCount, 2, ptr);
        ptr += 2;
        PubAsc2Bcd(sg_DccStat.astCurrency[ii].szOptInAmount, 12, ptr);
        ptr += 6;
    }

    iLen = ptr - pstPack->sField62 - 2;
    PubLong2Char(iLen, 2, pstPack->sField62);
	return iLen;
}
static uchar DccPackBit63Tag(uchar *ptr, uchar ucTag)
{
	uchar   ucValueLen;
	uchar   sValueBuff[128];

	if (ptr==NULL)
	{
		return 0;
	}

	ucValueLen = 0;
	memset(sValueBuff, 0, sizeof(sValueBuff));
	switch(ucTag)
	{
	case 12:
		ucValueLen = 1;
		if (PPDCC_ChkIfDccAcq())
		{
			sValueBuff[0] = 'D';
		}
		else
		{
			sValueBuff[0] = 'X';
		}
		break;
	case 13:	// 8.02.16 Build204 : 根据杨迅建议增加tag 13
		ucValueLen = 1;
		if (glProcInfo.stTranLog.ucDccType==PPTXN_OPTOUT)
		{
			sValueBuff[0] = 'Y';
		} 
		else
		{
			sValueBuff[0] = 'N';
		}
		break;
	case 16:
		ucValueLen = 6;
		if (glSendPack.sField48[0]!=0)
		{
			memcpy(sValueBuff,   "10", 2);
			memset(sValueBuff+2, ' ',  4);
			memcpy(sValueBuff+2, glSendPack.sField48, 3);
		}
		else
		{
			memcpy(sValueBuff,  "00", 2);
			memset(sValueBuff+2, ' ', 4);
		}
		break;
	case 27:
		ucValueLen = 1;
		sValueBuff[0] = 'R';
		break;
    case 33:
        ucValueLen = 1;
    //    sValueBuff[0] = 0x07;   // 0x01--PYC rate markup; 0x02--inverse rate; 0x04--PYC rate markup text
		 sValueBuff[0] = 0x17;
		//0x08--Partial Approvals Supported
		//0x10--Tansaction iD
//         if (ChkEdcExtOption(EDC_EX_INVERSE_RATE))
//         {
//             sValueBuff[0] |= 0x02;
//         }
        break;
	case 39:
		ucValueLen = 15;
		memcpy(sValueBuff,glProcInfo.stTranLog.szPPDccTxnID, 15);
		break;
	case 54:
		ucValueLen = 29;
		memset(sValueBuff,' ',29);
		break;
	default:
		break;
	}

	PubLong2Bcd((ucValueLen+2), 2, ptr);
	sprintf((char *)ptr+2, "%02d", (int)ucTag);
	memcpy(ptr+4, sValueBuff, ucValueLen);

	if (ucValueLen)
	{
		return ucValueLen+4;
	}

	return 0;
}

int PPDCC_PackField63(uchar ucTranType, void *psSendPack)
{
    uchar   *ptr;
    int     iLen;
    STISO8583 *pstPack = (STISO8583 *)psSendPack;

    ptr = pstPack->sField63 + 2;

	switch(ucTranType)
	{
	case RATE_SCB:
		ptr += DccPackBit63Tag(ptr, 12);
		ptr += DccPackBit63Tag(ptr, 27);
		ptr += DccPackBit63Tag(ptr, 33);
        break;
	case SALE:
    case AUTH:
    case PREAUTH:
		ptr += DccPackBit63Tag(ptr, 12);
		ptr += DccPackBit63Tag(ptr, 13);
		if (pstPack->sField48[0]!=0)
		{
			ptr += DccPackBit63Tag(ptr, 16);
		}
        ptr += DccPackBit63Tag(ptr, 33);
		break;
	case REFUND:
	case VOID:
		ptr += DccPackBit63Tag(ptr, 12);
		ptr += DccPackBit63Tag(ptr, 13);
        ptr += DccPackBit63Tag(ptr, 33);
		break;
	case OFFLINE_SEND:
	case UPLOAD:
		ptr += DccPackBit63Tag(ptr, 12);
		ptr += DccPackBit63Tag(ptr, 13);
		//build 1.00.0125
		/*
	//Build 1.00.0113
		if (glProcInfo.stTranLog.ucDccType==PPTXN_OPTOUT)
		{
			ptr += DccPackBit63Tag(ptr, 39);
		}
		*/
		break;
	case REVERSAL:
		ptr += DccPackBit63Tag(ptr, 12);
		ptr += DccPackBit63Tag(ptr, 13);
		break;
	default:
		break;
	}

    iLen = ptr - pstPack->sField63 - 2;
    PubLong2Char(iLen, 2, pstPack->sField63);
	return iLen;
}

int PPDCC_RetriveAllPDS(uchar *psField63, void *pProcInfo)
{
    uchar   *pData = psField63+2;
    int     iLen = psField63[0]*256 + psField63[1];
    DCC_PDS stPDS;
    SYS_PROC_INFO   *pstProc = (SYS_PROC_INFO *)pProcInfo;

    if (iLen>LEN_FIELD63)
    {
        return -1;
    }

    while (iLen>0)
    {
        if (GetDccPDS(pData, &stPDS))
        {
            return -2;
        }
        if (strcmp(stPDS.szTag, "07")==0)
        {
            PubBcd2Asc0(stPDS.sBuff, 4, (char *)(pstProc->stTranLog.szInvDccRate));
        }
        if (strcmp(stPDS.szTag, "34")==0)
        {
            sprintf((char *)(pstProc->stTranLog.sPPDccMarkupRate), "%.3s", stPDS.sBuff);
        }
        if (strcmp(stPDS.szTag, "37")==0)
        {
            sprintf((char *)(pstProc->stTranLog.szPPDccMarkupTxt), "%.22s", stPDS.sBuff);
        }
		//Build 1.00.0113
		if ((strcmp(stPDS.szTag, "39")==0) && (ChkIfBea() || ChkifBEADCC()))
		{
			sprintf((char *)(pstProc->stTranLog.szPPDccTxnID), "%.15s", stPDS.sBuff);
		}
        iLen  -= (4+stPDS.iLen);
        pData += (4+stPDS.iLen);
    }

    if (iLen<0)
    {
        return -3;
    }
    return 0;
}

void PPDCC_CreateDummyRspData(void *pstProcInfo)
{
	int	            iRet, iRand;
	uchar	        szBuff[32];
	SYS_PROC_INFO	*pstProc;

    GetTime(szBuff);
    iRand = (int)PubBcd2Long(szBuff+4, 2, NULL);

	pstProc = (SYS_PROC_INFO *)pstProcInfo;

	if ((pstProc->stTranLog.ucTranType==ENQUIRE_BOC_RATE) ||
		(pstProc->stTranLog.ucTranType==RATE_SCB))
	{
        // Random select currency
        if (memcmp(pstProc->stTranLog.szRspCode, "00", 2)==0)
        {
#ifdef WIN32
            ScrCls();
            DispTransName();
            ScrPrint(0, 2, ASCII, "WIN32 SIMULATION");
            ScrPrint(0, 3, ASCII, "SELECT CUR:");
            ScrPrint(0, 4, ASCII, "1.JPY  2.IDR");
            ScrPrint(0, 5, ASCII, "3.KWD  4.KRW");
            ScrPrint(0, 6, ASCII, "DEFAULT. USD");
            iRand = getkey();
            if (iRand>KEY0 && iRand<KEY5)
            {
                iRand -= KEY1;
            }
            else
            {
                iRand = 4;
            }
            ScrCls();
#endif

          //  iRand = 4;  //default the foreign currency as USD

            switch(iRand%5)
            {
            case 0:
	            sprintf(pstProc->stTranLog.szDccRate,    "51000000");
	            sprintf(pstProc->stTranLog.szInvDccRate, "60100000");
                iRet = FindCurrency("392", &glProcInfo.stTranLog.stHolderCurrency); // JPY
                break;
            case 1:
	            sprintf(pstProc->stTranLog.szDccRate,    "42857143");
	            sprintf(pstProc->stTranLog.szInvDccRate, "70035000");
                iRet = FindCurrency("360", &glProcInfo.stTranLog.stHolderCurrency); // IDR
                break;
            case 2:
	            sprintf(pstProc->stTranLog.szDccRate,    "75714289");
	            sprintf(pstProc->stTranLog.szInvDccRate, "61749999");
                iRet = FindCurrency("414", &glProcInfo.stTranLog.stHolderCurrency); // KWD
                break;
			case 3:
				sprintf(pstProc->stTranLog.szDccRate,    "51000000");
				sprintf(pstProc->stTranLog.szInvDccRate, "60100000");
                iRet = FindCurrency("410", &glProcInfo.stTranLog.stHolderCurrency); // KRW
                break;
            default:
	            sprintf(pstProc->stTranLog.szDccRate,    "71428571");
	            sprintf(pstProc->stTranLog.szInvDccRate, "67000002");
                iRet = FindCurrency("840", &glProcInfo.stTranLog.stHolderCurrency); // USD
                break;
            }
            PPDCC_CalcForeignAmount(&glSysParam.stEdcInfo.stLocalCurrency, glProcInfo.stTranLog.szAmount,
                                    glProcInfo.stTranLog.szDccRate,
                                    &glProcInfo.stTranLog.stHolderCurrency, glProcInfo.stTranLog.szFrnAmount);

            if (glProcInfo.stTranLog.szPan[0]=='4')
            {
                memcpy(glProcInfo.stTranLog.sPPDccMarkupRate, "+\x03\x46", 3);
                strcpy(glProcInfo.stTranLog.szPPDccMarkupTxt, "+four pt. four six");
            }
        }
	}
}


/************************************************************************/
/*                                                                      */
/************************************************************************/


void PPDCC_PromptDownloadBin(void)
{
    ScrCls();
	PubShowMsg(3, _T("PLS DOWNLOAD BIN"));
	PubShowMsg(5, _T("(USE FUNC 93)"));
	PubWaitKey(5);
}

// If load ok, return 0
uchar PPDCC_LoadBinFile(void)
{
	memset(&sg_stCardBin, 0, sizeof(sg_stCardBin));
	return PubFileRead(FILE_PP_LOCALBIN, 0, &sg_stCardBin, sizeof(DCC_CARDBIN_STORAGE));
}

// Check whether it is local card. Ensure the BIN file has loaded before.
uchar PPDCC_ChkIfLocalCard(char *pszPAN)
{
	uchar	sBuff[32];
	int		ii;

	if (sg_stCardBin.iTotalBlock==0)	// record not loaded, or record num=0
	{
		return 0;
	}

	PubAsc2Bcd(pszPAN, 10, sBuff);
	for (ii=0; ii<sg_stCardBin.iTotalBlock; ii++)
	{
		if ((memcmp(sBuff, sg_stCardBin.stCards[ii].sBinLow, 5)>=0) &&
			(memcmp(sBuff, sg_stCardBin.stCards[ii].sBinHigh, 5)<=0))
		{
            if (memcmp(sg_stCardBin.stCards[ii].sCurrCode, "\x00\x00", 2)==0)
            {
                return 2;
            }
            else
            {
			    return 1;
            }
		}
	}

	return 0;
}

void PPDCC_CardBinMenu(void)
{
	int		iMenu;
	static MenuItem stPPDccBinMenu[] =
	{
		{TRUE, _T_NOOP("DOWNLOAD BIN"),		DownloadBinTbl_Manual},	// 手动下载卡表
		{TRUE, _T_NOOP("VERIFY CARD"),		VerifyLocalCard},		// 检查卡片是否在表内
		{TRUE, _T_NOOP("SETUP AUTOLOAD"),	SetupBinAutoDownload},	// 设置自动下载参数
		{TRUE, _T_NOOP("CLEAR BIN"),		RemoveLocalBinFile},	// 清除已下载的卡表
		{TRUE, "", NULL},
	};

	//if( PasswordBank()!=0 )
	if( PasswordMerchant()!=0 )
	{
		return;
	}

	if ( PPDCC_SetCurAcq()!=0 )
	{
		PubShowMsg(4, _T("HAVE NOT DCC"));	// "无DCC参数设置"
		PubBeepErr();
		PubWaitKey(3);
		return;
	}

	while (1)
	{
		iMenu = PubGetMenu((uchar *)_T("DCC BIN MANAGE"),
							stPPDccBinMenu, MENU_AUTOSNO|MENU_CFONT, USER_OPER_TIMEOUT);
		if (iMenu<0)
		{
			break;
		}
	}
}

// Manually download local card BIN
static void DownloadBinTbl_Manual(void)
{
	DispResult(TransDownloadBin(FALSE));
}

static void DispCardBin(void)
{
#define LINES_PER_PAGE	6
	uchar	ucKey, ucAttr;
	int		iTotalPage, iNowPage, ii;

	PubShowTitle(TRUE, _T("DISPLAY BIN"));
	if (PPDCC_LoadBinFile()!=0)
	{
		PubShowMsg(4, _T("PLS DOWNLOAD BIN"));
        PubBeepErr();
        PubWaitKey(3);
		return;
	}

	iTotalPage = (sg_stCardBin.iTotalBlock+LINES_PER_PAGE-1)/LINES_PER_PAGE;
	if (iTotalPage==0)
	{
		PubShowTitle(TRUE, _T("DISPLAY BIN"));
		PubShowMsg(4, _T("NO RECORD"));
        PubBeepErr();
        PubWaitKey(3);
		return;
	}

	iNowPage = 1;
	while (1)
	{
		if (iNowPage>iTotalPage)
		{
			iNowPage = 1;
		}
		if (iNowPage<1)
		{
			iNowPage = iTotalPage;
		}

		ScrCls();
		ScrPrint(0, 0, ASCII, "DATE: %02X%02X.%02X.%02X",
				sg_stCardBin.sLastDate[0], sg_stCardBin.sLastDate[1],
				sg_stCardBin.sLastDate[2], sg_stCardBin.sLastDate[3]);
		ScrPrint(0, 1, ASCII, "%d/%d TOTAL: %d", iNowPage, iTotalPage, sg_stCardBin.iTotalBlock);
		for (ii=(iNowPage-1)*LINES_PER_PAGE; ii<iNowPage*LINES_PER_PAGE && ii<sg_stCardBin.iTotalBlock; ii++)
		{
            ucAttr = (memcmp(sg_stCardBin.stCards[ii].sCurrCode, "\x00\x00", 2) ? 0 : REVER);
			ScrPrint(0, (uchar)(2+ii%LINES_PER_PAGE), (uchar)(ucAttr|ASCII),
                    "%02X%02X%02X%02X%02X-%02X%02X%02X%02X%02X",
					sg_stCardBin.stCards[ii].sBinLow[0],sg_stCardBin.stCards[ii].sBinLow[1],
					sg_stCardBin.stCards[ii].sBinLow[2],sg_stCardBin.stCards[ii].sBinLow[3],
					sg_stCardBin.stCards[ii].sBinLow[4],
					sg_stCardBin.stCards[ii].sBinHigh[0],sg_stCardBin.stCards[ii].sBinHigh[1],
					sg_stCardBin.stCards[ii].sBinHigh[2],sg_stCardBin.stCards[ii].sBinHigh[3],
					sg_stCardBin.stCards[ii].sBinHigh[4]);
		}

		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if ((ucKey==KEYCANCEL) || (ucKey==NOKEY))
		{
			return;
		}
		if (ucKey==KEYUP)
		{
			iNowPage--;
		}
		if ((ucKey==KEYDOWN) || (ucKey==KEYENTER))
		{
			iNowPage++;
		}
	}
}

static void VerifyInCardBin(void)
{
	uchar	sBuff[32], ucRet;

	PubShowTitle(TRUE, _T("INPUT & CHECK"));
	if (PPDCC_LoadBinFile()!=0)
	{
		PubShowMsg(4, _T("PLS DOWNLOAD BIN"));
        PubBeepErr();
        PubWaitKey(3);
		return;
	}

	if (sg_stCardBin.iTotalBlock==0)
	{
		PubShowMsg(4, _T("NO RECORD"));
        PubBeepErr();
        PubWaitKey(3);
		return;
	}

	while(1)
	{
		PubShowTitle(TRUE, "INPUT & CHECK");
		memset(sBuff, 0, sizeof(sBuff));
		PubDispString("CARD NUM (10)", 2|DISP_LINE_LEFT);
		if (PubGetString(NUM_IN, 10, 10, sBuff, USER_OPER_TIMEOUT)!=0)
		{
			break;
		}

        ucRet = PPDCC_ChkIfLocalCard(sBuff);
		if (ucRet)
		{
            if (ucRet==2)
            {
			    PubShowMsg(4, "LOCAL CARD.(000)");
            }
            else
            {
			    PubShowMsg(4, "LOCAL CARD.");
            }
            PubBeepErr();
            PubWaitKey(3);
		}
		else
		{
			PubShowMsg(4, "NON-LOCAL CARD.");
            PubBeepErr();
            PubWaitKey(3);
		}
	}
}

// Manually enter a card to check whether in the local BIN range.
static void VerifyLocalCard(void)
{
	static MenuItem stVerfMenu[] =
	{
		{TRUE, _T_NOOP("DISPLAY BIN"),	 DispCardBin},
		{TRUE, _T_NOOP("INPUT & CHECK"), VerifyInCardBin},
		{TRUE, "", NULL},
	};

	PubGetMenu((uchar *)_T("VERIFY CARD"), stVerfMenu,
				MENU_AUTOSNO|MENU_PROMPT, USER_OPER_TIMEOUT);
}

// Setup the parameter for periodically download local BIN
static void SetupBinAutoDownload(void)
{
}

static void RemoveLocalBinFile(void)
{
    ScrCls();
	PubShowTitle(TRUE, (uchar *)"REMOVE CARDBIN  ");
    PubDispString("CONFIRM REMOVE ?", 4|DISP_LINE_CENTER);
    if (AskYesNo())
    {
	    remove(FILE_PP_LOCALBIN);
	    ScrCls();
	    DispOperOk("CLEARED.");
    }
}

static void AutoLoadBinGenNextRemindDate(void)
{
    // ????
}

// DO NOT support incremental download
static int TransDownloadBin(uchar bAutoDownload)
{
	int		iRet;
	long	lPDSLen;
	uchar	sBuff[256], ucGotData;
	DCC_CARDBIN_STORAGE	stCardBinStore;

	iRet = TransInit(LOAD_CARD_BIN);
	if( iRet!=0 )
	{
		return iRet;
	}

	if( !ChkIfTranAllow(glProcInfo.stTranLog.ucTranType) )
	{
		return ERR_NO_DISP;
	}

	ScrCls();
	DispTransName();
	//PP问题后台，只能以传下来的长度为准
		memset(sg_stCardBin.ucPPlen,0,4);//init len
	if ( PPDCC_SetCurAcq()!=0 )
	{
		PubShowMsg(4, _T("HAVE NOT DCC"));	// "无DCC参数设置"
		PubBeepErr();
		PubWaitKey(3);
		return 0;
	}

	if (!ChkEdcOption(EDC_ENABLE_DCC))
	{
		PubShowMsg(4, _T("DCC DISABLED."));
		PubBeepErr();
		PubWaitKey(3);
		return ERR_NO_DISP;
	}

	memset(&stCardBinStore, 0, sizeof(stCardBinStore));
	ucGotData = FALSE;

	while (1)
	{
		ScrCls();
		DispTransName();

		SetCommReqField();
		memset(sBuff,   0, 4);	// date, always 0

		//PP问题后台，只能以传下来的长度为准
		memcpy(sBuff+4,sg_stCardBin.ucPPlen,4);
 	//	PubLong2Bcd(stCardBinStore.iTotalBlock, 4, sBuff+4);	// last sequence in BCD
		memcpy(sBuff+8, glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, 2);
		lPDSLen = PackDccPDS("01", sBuff, 10, glSendPack.sField63+2);
		PubLong2Char(lPDSLen, 2, glSendPack.sField63);

		iRet = SendRecvPacket();
		if( iRet!=0 )
		{
			CommOnHook(FALSE);
			return iRet;
		}
		iRet = AfterTranProc();
		if( iRet!=0 )
		{
			CommOnHook(FALSE);
			return iRet;
		}

		iRet = UnpackCardBin(glRecvPack.sField63, &stCardBinStore);
		if (iRet!=0)
		{
			CommOnHook(FALSE);
			return ERR_TRAN_FAIL;
		}

		if (!ucGotData)	// when receive data, remove file at first time.
		{
			remove(FILE_PP_LOCALBIN);
			ucGotData = TRUE;
		}

		if (glRecvPack.szProcCode[5]=='0')	// no more message required.
		{
			break;
		}
	}

	// here must be complete succeed.
	CommOnHook(FALSE);

	ScrCls();
	DispTransName();
	PubShowMsg(4, _T("SAVING CARD BIN"));
	PubFileWrite(FILE_PP_LOCALBIN, 0, &stCardBinStore, sizeof(stCardBinStore));

    PubBeepOk();

	ScrCls();
	DispTransName();
	PubShowMsg(4, _T("LOAD COMPLETED"));	// "卡表下载完毕"
	sprintf(sBuff, "%s %i", _T("BIN TOTAL:"), stCardBinStore.iTotalBlock);
	PubShowMsg(6, sBuff);
	PubWaitKey(USER_OPER_TIMEOUT);

	return 0;
}

static int UnpackCardBin(uchar *pFieldIn, DCC_CARDBIN_STORAGE *pstCardBinStore)
{
	int		iRet, iDataLen;
	uchar	*pUnpack;
	DCC_PDS	stPDS;

	iDataLen = 256*pFieldIn[0] + pFieldIn[1];
	pUnpack = pFieldIn + 2;
	if (iDataLen<2)
	{
		DispTransName();
		PubShowMsg(4, _T("NO CARDBIN DATA"));
        PubBeepErr();
        PubWaitKey(3);
		return -1;
	}

	iRet = GetDccPDS(pUnpack, &stPDS);
	if (memcmp(stPDS.szTag,"02",2)==0)
	{
		//PP问题后台，只能以传下来的长度为准
		memcpy(sg_stCardBin.ucPPlen,stPDS.sBuff+4,4);
	}
	if ((iRet!=0) || (memcmp(stPDS.szTag, "02", 2)!=0))
	{
		DispTransName();
		PubShowMsg(4, _T("INVALID DATA"));
        PubBeepErr();
        PubWaitKey(3);
		return -1;
	}
	memcpy(pstCardBinStore->sLastDate, stPDS.sBuff+2, 2);	// YYYY	
	memcpy(pstCardBinStore->sLastDate+2, stPDS.sBuff, 2);	// MMDD
	pUnpack += (stPDS.iLen+2+2);

	while (pUnpack<pFieldIn+2+iDataLen)
	{
		if (pstCardBinStore->iTotalBlock>=MAX_DCCLOCALBIN)
		{
			// "卡表数量溢出"
			DispTransName();
			PubShowMsg(4, _T("CARDBIN OVERFLOW"));
            PubBeepErr();
            PubWaitKey(3);
			return -2;
		}

		iRet = GetDccPDS(pUnpack, &stPDS);
		if ((iRet!=0) || (memcmp(stPDS.szTag, "03", 2)!=0))
		{
			DispTransName();
			PubShowMsg(4, _T("UNEXPECTED DATA"));
            PubBeepErr();
            PubWaitKey(3);
			return -1;
		}

		memcpy(pstCardBinStore->stCards[pstCardBinStore->iTotalBlock].sBinLow,   stPDS.sBuff,    5);
		memcpy(pstCardBinStore->stCards[pstCardBinStore->iTotalBlock].sBinHigh,  stPDS.sBuff+5,  5);
        memcpy(pstCardBinStore->stCards[pstCardBinStore->iTotalBlock].sCurrCode, stPDS.sBuff+10, 2);
		pstCardBinStore->iTotalBlock++;

		pUnpack += (stPDS.iLen+2+2);
	}

	if (pUnpack!=pFieldIn+2+iDataLen)
	{
		DispTransName();
		PubShowMsg(4, _T("INVALID LENGTH"));
        PubBeepErr();
        PubWaitKey(3);
		return -1;
	}

	return 0;
}

// Divide an ASCII-numeric string with a rate string. When bCalcRate==TRUE, the output will be a rate format
int PPDCC_CalcLocalAmount(char *pszDivisor, CURRENCY_CONFIG *pstFrnCurrency,
                          char *psRateStr,
                          CURRENCY_CONFIG *pstLocalCurrency, char *pszOut)
{
	int		ii, iDecimalPos;// Decimal position 小数点位置
	ulong	ulRate, ulQuotient, ulRemainder;
    char    szBuff[32], chTemp;

	// Prepare rate 转换汇率格式
	iDecimalPos = psRateStr[0] - '0';
	ulRate = PubAsc2Long(psRateStr+1, 7, NULL);

	// Prepare divisor 准备除数
	if (!ValidBigAmount(pszDivisor))
	{
		return -1;
	}
    strcpy(szBuff, pszDivisor);
    PubTrimHeadChars(szBuff, '0');
    if (strlen(szBuff)==0)
    {
        strcpy(szBuff, "0");
    }
    if (pstFrnCurrency->ucIgnoreDigit)
    {
        szBuff[strlen(szBuff)-(pstFrnCurrency->ucIgnoreDigit)] = 0;
    }
	ulRemainder = PubAsc2Long(szBuff, strlen(szBuff), NULL);

    // 12345/0.1428571 == 123450000000.00/1428571
    iDecimalPos += pstLocalCurrency->ucDecimal;
    iDecimalPos -= pstFrnCurrency->ucDecimal;

    memset(szBuff, 0, sizeof(szBuff));
    for (ii=0; ii<iDecimalPos+2; ii++)
    {
		ulQuotient  = ulRemainder / ulRate;
		ulRemainder = ulRemainder % ulRate;
		ulRemainder *= 10;
        if (ulQuotient==0)
        {
            strcat(szBuff, "0");
        } 
        else
        {
		    sprintf(szBuff+strlen(szBuff), "%lu", ulQuotient);
        }
	}

    // 四舍五入
    chTemp = szBuff[strlen(szBuff)-1];
    szBuff[strlen(szBuff)-1] = 0;  // truncate
    if (chTemp>'4')
    {
        PubAscInc(szBuff, strlen(szBuff));
    }

    PubTrimHeadChars(szBuff, '0');
    sprintf(szBuff+strlen(szBuff), "%.*s", (int)(pstLocalCurrency->ucIgnoreDigit), "0000");
    PubAddHeadChars(szBuff, LEN_TRAN_AMT, '0');

	// generate 12-digit amount
	sprintf(pszOut, "%.12s", szBuff);
	return 0;
}

int PPDCC_CalcForeignAmount(CURRENCY_CONFIG *pstLocalCur, char *pszLocalAmt,
                            char *psRateStr,
                            CURRENCY_CONFIG *pstForeignCur, char *pszForeignAmt)
{
    uchar   szLocal[12+1], szRate[8+1], szForeign[32], ucAbandon;
    uchar   ucDecimalPos;
    int     iTrimDigits;

    // e.g. JPY 000001234500 --> 12345
    sprintf(szLocal, "%.12s", pszLocalAmt);
    PubTrimHeadChars(szLocal, '0');
    if (strlen((char *)szLocal)==0)
    {
        strcpy((char *)szLocal, "0");
    }
    if (pstLocalCur->ucIgnoreDigit)
    {
        // trim ignore digits.
        if (strlen(szLocal) > pstLocalCur->ucIgnoreDigit)
        {
            szLocal[strlen(szLocal) - pstLocalCur->ucIgnoreDigit] = 0;
        }
        else
        {
            strcpy(szLocal, "0");
        }
    }

    // Rate string. "71331782" == 0.1331782, decimal position is 7
    sprintf(szRate, "%.7s", psRateStr+1);

    // Decimal after multiply.
    ucDecimalPos = pstLocalCur->ucDecimal + (psRateStr[0]-'0');

    PubAscMul(szLocal, szRate, szForeign);

    // 0.03 is expressed as "3" with decinal=2. Need to pad to "003"
    if (strlen(szForeign)<=ucDecimalPos)
    {
        PubAddHeadChars(szForeign, ucDecimalPos+1, '0');
    }

    // remove extra decimal digits
    iTrimDigits = ucDecimalPos - pstForeignCur->ucDecimal;
    if (iTrimDigits>0)
    {
        ucAbandon = szForeign[strlen(szForeign)-iTrimDigits];
        szForeign[strlen(szForeign)-iTrimDigits] = 0;   // trim tail
        if (ucAbandon > '4')
        {
            PubAscInc(szForeign, strlen(szForeign));
        }
    }
    if (iTrimDigits<0)
    {
        sprintf(szForeign+strlen(szForeign), "%.*s", 0-iTrimDigits, "000000000000"); // add tail '0'
    }
    // add ignore digits
    iTrimDigits = pstForeignCur->ucIgnoreDigit;
    if (iTrimDigits>0)
    {
        sprintf(szForeign+strlen(szForeign), "%.*s", iTrimDigits, "000000000000"); // add tail '0'
    }

    if (strlen(szForeign)>=LEN_FRN_AMT)
    {
        sprintf(pszForeignAmt, "%.*s", LEN_FRN_AMT, szForeign+strlen(szForeign)-LEN_FRN_AMT);
    }
    else
    {
        PubAddHeadChars(szForeign, LEN_FRN_AMT, '0');
        strcpy(pszForeignAmt, szForeign);    
    }
    
    return 0;
}

// Convert the PP-format rate string to a display format
// In PP-format, rate is 8-digit string, whereas first digit indicates the decimal point position
void PPDCC_FormatRate(uchar *psRateStr, char *pszOut)
{
	char	sBuff[16];
	uchar	ucDecimal;
    
	sprintf(sBuff, "%.8s", psRateStr);
    if (strlen(sBuff)!=8)
    {
        return;
    }

	ucDecimal = sBuff[0] - '0';
	if (ucDecimal>=7)
	{
		sprintf(pszOut, "0.");
		while (ucDecimal>7)
		{
			strcat(pszOut, "0");
			ucDecimal--;
		}
		sprintf(pszOut+strlen(pszOut), "%.7s", sBuff+1);
	}
	else
	{
		sprintf(pszOut, "%.*s.%.*s", 7-ucDecimal, sBuff+1, ucDecimal, sBuff+8-ucDecimal);
	}
}

static uchar NeedSwitchComm(ACQUIRER *pstLastAcq)
{
    if (glCommCfg.ucCommType==CT_MODEM)
    {
        if (0 != memcmp(glCurAcq.stTxnPhoneInfo[0].szTelNo,
                    pstLastAcq->stTxnPhoneInfo[0].szTelNo,
                    sizeof(glCurAcq.stTxnPhoneInfo[0].szTelNo)))
        {
            return TRUE;
        }
    }
    if (glCommCfg.ucCommType==CT_TCPIP)
    {
        if (0 != strcmp(glCurAcq.stTxnTCPIPInfo[0].szIP, pstLastAcq->stTxnTCPIPInfo[0].szIP))
        {
            return TRUE;
        }
    }
    if (glCommCfg.ucCommType==CT_GPRS || glCommCfg.ucCommType==CT_3G)//2015-11-23 share para with GPRS
    {
        if (0 != strcmp(glCurAcq.stTxnGPRSInfo[0].szIP, pstLastAcq->stTxnGPRSInfo[0].szIP))
        {
            return TRUE;
        }
    }
    return FALSE;
}

// Swith the transaction data to a domestic one when it's going on.
static int SwitchTranToDomestic(void)
{
	ACQUIRER	stLastAcq;
	int	iRet;

	stLastAcq = glCurAcq;
	// need to re-direct to correct acquirer. 双batch模式下需要重新确定acquirer
	iRet = MatchCardTable(glProcInfo.stTranLog.szPan,
                            glProcInfo.stTranLog.ucTranType,
                            1);
	if (iRet!=0)
	{
		CommOnHook(FALSE);
		DispTransName();
		PubShowMsg(4, _T("DCC PARA ERROR"));
        PubBeepErr();
        PubWaitKey(5);
		return ERR_NO_DISP;
	}

	if (!ChkSettle())
	{
		CommOnHook(FALSE);
		return ERR_NO_DISP;
	}
	// Change tel if necessary. 换号重拨
    if (NeedSwitchComm(&stLastAcq))
    {
		CommOnHook(FALSE);
	}

	return 0;
}

static int SwitchTranToDCC(SYS_PROC_INFO *pstTran)
{
    uchar   sCurrencyCode[2], szAmount[12+1];
    uchar   ucCurrExp, ucIgnoreDigit;

    pstTran->stTranLog.stTranCurrency = pstTran->stTranLog.stHolderCurrency;

#ifdef ENABLE_EMV
    if (pstTran->stTranLog.uiEntryMode & MODE_CHIP_INPUT)
    {
	    memcpy(sCurrencyCode, glProcInfo.stTranLog.stTranCurrency.sCurrencyCode, 2);
        ucCurrExp = glProcInfo.stTranLog.stTranCurrency.ucDecimal;
        strcpy((char *)szAmount, (char *)(pstTran->stTranLog.szFrnAmount));
        ucIgnoreDigit = pstTran->stTranLog.stTranCurrency.ucIgnoreDigit;

        if (memcmp(sCurrencyCode, "\x03\x60", 2)==0) // IDR
        {
            if (pstTran->stTranLog.szPan[0]=='4')
            {
                // for VISA card of "IDR", pad "00"
                memmove(szAmount, szAmount+2, 12-2);
                memcpy(szAmount+12-2, "00", 2);
            }
        }
        else if (pstTran->stTranLog.stTranCurrency.ucIgnoreDigit!=0) // JPY, KRW
        {
            // remove "ignore digit" in amount string
            memmove(szAmount+ucIgnoreDigit, szAmount, 12-ucIgnoreDigit);
            memcpy(szAmount, "0000", ucIgnoreDigit);
        }

        SetEMVAmountTLV(szAmount);

	    EMVGetParameter(&glEmvParam);
	    memcpy(glEmvParam.TransCurrCode, sCurrencyCode, 2);
	    glEmvParam.TransCurrExp = ucCurrExp;
	    EMVSetParameter(&glEmvParam);
        EMVSetTLVData(0x5F2A, sCurrencyCode, 2);
        EMVSetTLVData(0x5F36, &ucCurrExp, 1);
    }
#endif
    
    return 0;
}

uchar PPDCC_ChkIfAutoOptIn(void)
{
	if ((glProcInfo.stTranLog.ucTranType==SALE) || (glProcInfo.stTranLog.ucTranType==OFF_SALE) ||
        (glProcInfo.stTranLog.ucTranType==PREAUTH) || (glProcInfo.stTranLog.ucTranType==AUTH))
	{
		return 1;
	}
	if ( ChkIfNeedTip() )
	{
		return 1;
	}
	return 0;
}

// amount must be like ISO bit4 format
int PPDCC_CalForeignAmount(uchar *pszLocalAmt, uchar *psRate, uchar *pszForeignAmt)
{
	uchar	sBuff1[32], sBuff2[32], sBuff3[32];
	int		iDecimalPos, iLen;

	sprintf(sBuff1, "%.12s", pszLocalAmt);
	sprintf(sBuff2, "%.7s",  psRate+1);
	iDecimalPos = (int)PubAsc2Long(psRate, 1, NULL);
	PubAscMul(sBuff1, sBuff2, sBuff3);
	iLen = strlen(sBuff3);
    
	if (iDecimalPos)
	{
		if (iLen<=iDecimalPos)
		{
			PubAddHeadChars(sBuff3, iDecimalPos+1, '0');
			iLen = strlen(sBuff3);
		}
		sBuff3[iLen-iDecimalPos+1] = 0;	// reserve extra 1 digit
		iLen = iLen-iDecimalPos;
		sprintf(pszForeignAmt, "%012d", 0);
		sprintf(pszForeignAmt+12-iLen, "%.*s", iLen, sBuff3);
		if (sBuff3[iLen]>'4')
		{
			PubAscInc(pszForeignAmt, 12);
		}
	}
	else
	{
		sprintf(pszForeignAmt, "%012d", 0);
		sprintf(pszForeignAmt+12-iLen, "%.*s", iLen, sBuff3);
	}
	return 0;
}

void PPDCC_TransRateReport(void)
{
	int		iRet;
	long	lPDSLen, iDataLen;
	uchar	ucLastPage;
	uchar	sBuff[256], sLastPageNumber[2], *pData;
	DCC_PDS	stPDS;

	iRet = TransInit(LOAD_RATE_REPORT);
	if( iRet!=0 )
	{
		return;
	}

	ScrCls();
	DispTransName();
	//if( PasswordBank()!=0 )
	if( PasswordMerchant()!=0 )
	{
		return;
	}

	if( !ChkIfTranAllow(glProcInfo.stTranLog.ucTranType) )
	{
		return;
	}

	ScrCls();
	DispTransName();

	if ( PPDCC_SetCurAcq()!=0 )
	{
		PubShowMsg(4, _T("HAVE NOT DCC"));	// "无DCC参数设置"
		PubBeepErr();
		PubWaitKey(3);
		return;
	}

	if (!ChkEdcOption(EDC_ENABLE_DCC))
	{
		PubShowMsg(4, _T("DCC DISABLED."));
		PubWaitKey(USER_OPER_TIMEOUT);
		return;
	}

	ucLastPage = FALSE;
	memset(sLastPageNumber, 0, sizeof(sLastPageNumber));
	while (1)
	{
		ScrCls();
		DispTransName();

		SetCommReqField();
		memset(sBuff, 0, sizeof(sBuff));
		sBuff[0] = 0x01;
		memcpy(sBuff+1, sLastPageNumber, 2);
		lPDSLen = PackDccPDS("04", sBuff, 3, glSendPack.sField63+2);
		PubLong2Char(lPDSLen, 2, glSendPack.sField63);

		iRet = SendRecvPacket();
		if( iRet!=0 )
		{
			CommOnHook(FALSE);
			return;
		}
		iRet = AfterTranProc();
		if( iRet!=0 )
		{
			CommOnHook(FALSE);
			return;
		}

		iDataLen = (int)PubChar2Long(glRecvPack.sField63, 2, NULL);
		if ((GetDccPDS(glRecvPack.sField63+2, &stPDS)!=0) || (memcmp(stPDS.szTag, "05", 2)!=0))
		{
			CommOnHook(FALSE);
			PubShowMsg(4, _T("INVALID DATA"));
            PubBeepErr();
            PubWaitKey(3);
            return;
		}
		memcpy(sLastPageNumber, stPDS.sBuff, 2);

		pData = glRecvPack.sField63+2+2+2+stPDS.iLen;
		if ((GetDccPDS(pData, &stPDS)!=0) || (memcmp(stPDS.szTag, "06", 2)!=0))
		{
			CommOnHook(FALSE);
			PubShowMsg(4, _T("INVALID DATA"));
            PubBeepErr();
            PubWaitKey(3);
            return;
		}

		if (glRecvPack.szProcCode[5]=='0')	// no more message required.
		{
			CommOnHook(FALSE);
			ucLastPage = TRUE;
		}

		iRet = PrnRateReport(stPDS.sBuff, stPDS.iLen, ucLastPage);
		if (iRet!=0)
		{
			CommOnHook(FALSE);
			return;
		}

		if (ucLastPage)
		{
			return;
		}
	}
}

int PPDCC_TransRateEnquiry(void)
{
	int	iRet;
	uchar	sBuff2[128], sBuff3[128], szAmtLocal[32], szAmtFrn[32];
	SYS_PROC_INFO	stProcInfoBak, stProcInfoRate;

	// Backup previous data
	memcpy(&glProcInfo.stSendPack, &glSendPack, sizeof(STISO8583));
	memcpy(&stProcInfoBak, &glProcInfo, sizeof(SYS_PROC_INFO));

	// Online rate enquiry ---------------------------------------------------------- 
	if (TransInit(RATE_SCB))
    {
        return ERR_NO_DISP;
    }

	ScrCls();
	DispTransName();
	SetCommReqField();
	iRet = SendRecvPacket();

	// Backup rate-lookup transaction data
	memcpy(&glProcInfo.stSendPack, &glSendPack, sizeof(STISO8583));
	memcpy(&stProcInfoRate, &glProcInfo, sizeof(SYS_PROC_INFO));
	
    // Recover to original transaction
	memcpy(&glProcInfo, &stProcInfoBak, sizeof(SYS_PROC_INFO));
	memcpy(&glSendPack, &glProcInfo.stSendPack, sizeof(STISO8583));

	// Copy rate data to the coming sale transaction
	strcpy(glProcInfo.stTranLog.szFrnAmount,      stProcInfoRate.stTranLog.szFrnAmount);
	strcpy(glProcInfo.stTranLog.szDccRate,        stProcInfoRate.stTranLog.szDccRate);
	strcpy(glProcInfo.stTranLog.szInvDccRate,     stProcInfoRate.stTranLog.szInvDccRate);
    strcpy(glProcInfo.stTranLog.szPPDccMarkupTxt, stProcInfoRate.stTranLog.szPPDccMarkupTxt);
    memcpy(glProcInfo.stTranLog.sPPDccMarkupRate,
            stProcInfoRate.stTranLog.sPPDccMarkupRate,
            sizeof(glProcInfo.stTranLog.sPPDccMarkupRate));
	strcpy(glProcInfo.stTranLog.szDateTime,  stProcInfoRate.stTranLog.szDateTime);
	glProcInfo.stTranLog.stHolderCurrency = stProcInfoRate.stTranLog.stHolderCurrency;
	glProcInfo.stTranLog.ulInvoiceNo = stProcInfoRate.stTranLog.ulInvoiceNo;	// ????
	// Here do not copy the response code from rate-lookup
    // How about RRN ?

	// If cancel in halfway
	if ((iRet==ERR_USERCANCEL))
	{
        CommOnHook(FALSE);
		return iRet;
	}

	// Cut line when offline sale
	if (glProcInfo.stTranLog.ucTranType==OFF_SALE)
	{
		CommOnHook(FALSE);
	}

    Beef(1, 100);

    // Check rate enquiry result ----------------------------------------------------

    // Any error during online enquiry.
	if (iRet)
	{
        glProcInfo.stTranLog.ucDccType = PPTXN_NOT_ELIGIBLE;
		return SwitchTranToDomestic();
	}
    // Not approved
	if (strcmp(stProcInfoRate.stTranLog.szRspCode, "00")!=0)
	{
        glProcInfo.stTranLog.ucDccType = PPTXN_NOT_ELIGIBLE;
		//return SwitchTranToDomestic();
		iRet = SwitchTranToDomestic();
		return iRet;
	}  

    // Check currency
	if (memcmp(glProcInfo.stTranLog.stHolderCurrency.sCurrencyCode, "\x00\x00", 2)==0)
	{
		PubShowMsg(4, "INVALID CURRENCY");
        PubWaitKey(2);
        glProcInfo.stTranLog.ucDccType = PPTXN_NOT_ELIGIBLE;
		return SwitchTranToDomestic();
	}

    // Check rate
    if ((strlen((char *)glProcInfo.stTranLog.szDccRate)!=8) || !IsNumStr((char *)glProcInfo.stTranLog.szDccRate))
    {
		PubShowMsg(4, "INVALID RATE");
        PubWaitKey(2);
        glProcInfo.stTranLog.ucDccType = PPTXN_NOT_ELIGIBLE;
		return SwitchTranToDomestic();
	}

    //-------------------------------------------------------------------------------
	// DCC eligible. Continue for DCC selection, or auto opt-in

    // Get foreign amount
	if ( !ChkIfZeroAmt(glProcInfo.stTranLog.szFrnAmount) )
	{
		sprintf(sBuff2, "%.12s", glProcInfo.stTranLog.szFrnAmount); // If foreign amount is provided
	}
	else
	{
		PPDCC_CalForeignAmount(glProcInfo.stTranLog.szAmount,
                                glProcInfo.stTranLog.szDccRate, sBuff2);// If foreign amount is not provided.
	}
    PubConvAmount(glProcInfo.stTranLog.stHolderCurrency.szName, sBuff2,
                    glProcInfo.stTranLog.stHolderCurrency.ucDecimal,
                    glProcInfo.stTranLog.stHolderCurrency.ucIgnoreDigit,
                    szAmtFrn, (uchar)(glProcInfo.stTranLog.ucTranType==REFUND ? GA_NEGATIVE : 0));

    // Get Rate
    if (strlen(glProcInfo.stTranLog.szInvDccRate))
    {
        memset(sBuff2, 0, sizeof(sBuff2));
        PPDCC_FormatRate(glProcInfo.stTranLog.szInvDccRate, sBuff2);
        sprintf((char *)sBuff3, "%s %3.3s/%3.3s", sBuff2, glSysParam.stEdcInfo.stLocalCurrency.szName,
                                                         glProcInfo.stTranLog.stHolderCurrency.szName);
    }
    else
    {
	    memset(sBuff2, 0, sizeof(sBuff2));
	    PPDCC_FormatRate(glProcInfo.stTranLog.szDccRate, sBuff2);
        sprintf((char *)sBuff3, "%s %3.3s/%3.3s", sBuff2, glProcInfo.stTranLog.stHolderCurrency.szName,
                                                         glSysParam.stEdcInfo.stLocalCurrency.szName);
    }

    // Get local amount
    App_ConvAmountLocal(glProcInfo.stTranLog.szAmount, szAmtLocal,
                        (uchar)(glProcInfo.stTranLog.ucTranType==REFUND ? GA_NEGATIVE : 0));

    // Card holder selection on screen ----------------------------------------------

    /*
	    ScrCls();
	    DispTransName();
	    DispAmount(2, glProcInfo.stTranLog.szAmount);
	    ScrPrint(0, 4, ASCII, "RATE:%s", sBuff2);
	    ScrPrint(0, 5, CFONT, "%16.16s", sBuff1);
    */

    ScrCls();
    PubDispString(szAmtLocal, 0|DISP_LINE_RIGHT);
    if (glProcInfo.stTranLog.szPPDccMarkupTxt[0])
    {
        ScrPrint(0, 2, ASCII, "%21.21s", sBuff3);
        sprintf((char *)sBuff3, "Wholesale + %X.%02X%%",
                                glProcInfo.stTranLog.sPPDccMarkupRate[1],
                                glProcInfo.stTranLog.sPPDccMarkupRate[2]);
        ScrPrint(0, 3, ASCII, "%21.21s", sBuff3);
    }
    else
    {
	    ScrPrint(0, 2, ASCII, "RATE:");
        ScrPrint(0, 3, ASCII, "%21.21s", sBuff3);
    }
    PubDispString(szAmtFrn, 4|DISP_LINE_RIGHT);

    // Auto opt-in
	if (PPDCC_ChkIfAutoOptIn())
	{
		PubDispString("AUTO OPT-IN...", 6|DISP_LINE_RIGHT);
		PubWaitKey(3);
		return SwitchTranToDCC(&glProcInfo);
	}

    if (glProcInfo.stTranLog.ucTranType==REFUND)
    {
        sprintf((char *)sBuff3, "REFUND BY %.3s ?",
                glProcInfo.stTranLog.stHolderCurrency.szName);
    } 
    else
    {
        sprintf((char *)sBuff3, "PAY BY %.3s ?",
                glProcInfo.stTranLog.stHolderCurrency.szName);
    }
    PubDispString(sBuff3, 6|DISP_LINE_REVER|DISP_LINE_RIGHT);
	if (PubYesNo((ushort)(USER_OPER_TIMEOUT*3))==0)
	{
		return SwitchTranToDCC(&glProcInfo);
	}
    else
    {
        glProcInfo.stTranLog.ucDccType = PPTXN_OPTOUT;
		return SwitchTranToDomestic();
    }
}

static int OptoutRecord(SYS_PROC_INFO *pstDccProc, uchar *pszNewTotal, uchar *pszNewTip)
{
    uchar       szTemp[32], szNewTotal[12+1], szNewTip[12+1];
    uchar       szOptInAmount[12+1], szOptOutAmount[12+1];
    int         iRet;
    SYS_PROC_INFO   stDccProc;
	
    stDccProc = *pstDccProc;
    strcpy(szNewTotal, pszNewTotal);
    strcpy(szNewTip,   pszNewTip);

	// append a new offline txn with SCB ------------------------------------
    InitTransInfo();
    glProcInfo.stTranLog.ucTranType = OFF_SALE;
    strcpy((char *)glProcInfo.stTranLog.szPan,           (char *)stDccProc.stTranLog.szPan);
    strcpy((char *)glProcInfo.stTranLog.szExpDate,       (char *)stDccProc.stTranLog.szExpDate);
    strcpy((char *)glProcInfo.stTranLog.szInitialAmount, (char *)szNewTotal);
    PubAscSub(szNewTotal, szNewTip, LEN_TRAN_AMT, glProcInfo.stTranLog.szAmount);
#ifdef AMT_PROC_DEBUG
			//2014-9-18 each time come to glAmount or szAmount
			sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(40)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(40)%.13s",glProcInfo.stTranLog.szAmount);
			glProcInfo.stTranLog.cntAmt++;
	//		//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 10: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
    strcpy((char *)glProcInfo.stTranLog.szTipAmount,     (char *)szNewTip);
	PubGetDateTime(glProcInfo.stTranLog.szDateTime);
  
  /*build88R: bug fix Void DCC opt out transacation 
	sprintf((char *)&glProcInfo.stTranLog.szRRN, "%12.12s", "");
  */
  glProcInfo.stTranLog.szRRN[0] = 0;
  //end build88R

    iRet = MatchCardTable(glProcInfo.stTranLog.szPan,
                            glProcInfo.stTranLog.ucTranType,
                            1);
    if (iRet)
    {
		PubShowTitle(TRUE, (uchar *)_T("DCC OPT-OUT     "));
		PubShowMsg(4, _T("DCC PARA ERROR"));
        PubBeepErr();
        PubWaitKey(5);
		return ERR_NO_DISP;
    }
    glProcInfo.stTranLog.ucDescTotal = stDccProc.stTranLog.ucDescTotal;
    strcpy((char *)glProcInfo.stTranLog.szDescriptor, (char *)stDccProc.stTranLog.szDescriptor);
    strcpy((char *)glProcInfo.stTranLog.szAddlPrompt, (char *)stDccProc.stTranLog.szAddlPrompt);
    strcpy((char *)glProcInfo.stTranLog.szAuthCode,   (char *)stDccProc.stTranLog.szAuthCode);
    glProcInfo.stTranLog.stHolderCurrency = stDccProc.stTranLog.stHolderCurrency;
    glProcInfo.stTranLog.stTranCurrency = glSysParam.stEdcInfo.stLocalCurrency;
    glProcInfo.stTranLog.ucDccType = PPTXN_OPTOUT;

	glProcInfo.stTranLog.uiEntryMode = MODE_MANUAL_INPUT;	
    glProcInfo.stTranLog.ulInvoiceNo = glSysCtrl.ulInvoiceNo;
	GetNewInvoiceNo();
    sprintf((char *)glProcInfo.stTranLog.szRspCode, "00");
	strcpy(glProcInfo.stTranLog.szPPDccTxnID,stDccProc.stTranLog.szPPDccTxnID);//bea OFFLINE61 NEED squall add
	GetNewTraceNo();
	glProcInfo.stTranLog.uiStatus |= TS_NOSEND;
    // ........need any more ?
	iRet = SaveTranLog(&glProcInfo.stTranLog);
#ifdef AMT_PROC_DEBUG
	glProcInfo.stTranLog.cntAmt = 0;//2014-9-19 ack POS to erase amt records
#endif
    // After save log
	//EcrSendTransSucceed();

    strcpy(szOptOutAmount, szNewTotal); // opt-out amount to add


    // Original record : change amount and status ---------------------------
    glProcInfo = stDccProc;
    GetRecordByInvoice(glProcInfo.stTranLog.ulInvoiceNo, TS_ALL_LOG, &glProcInfo.stTranLog);
    FindAcq(glProcInfo.stTranLog.ucAcqKey);
    FindIssuer(glProcInfo.stTranLog.ucIssuerKey);
	if( !(glProcInfo.stTranLog.uiStatus & TS_NOSEND) )
	{
		glProcInfo.stTranLog.uiStatus |= (TS_ADJ|TS_NOSEND);
	}
    else
    {
        glProcInfo.stTranLog.uiStatus |= TS_NOT_UPLOAD;
    }
    PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTemp);
    sprintf((char *)glProcInfo.stTranLog.szOrgAmount, "%.12s", szTemp);
	sprintf((char *)szOptInAmount, "%.12s", szTemp); // opt-in amount to deduce
    strcpy((char *)glProcInfo.stTranLog.szAmount,    "000000000000");
    strcpy((char *)glProcInfo.stTranLog.szTipAmount, "000000000000");
    strcpy((char *)glProcInfo.stTranLog.szFrnAmount, "000000000000");
    strcpy((char *)glProcInfo.stTranLog.szFrnTip,    "000000000000");
    glProcInfo.stTranLog.ucDccType = PPTXN_OPTOUT;

    iRet = UpdateTranLog(&glProcInfo.stTranLog, glProcInfo.uiRecNo);
#ifdef AMT_PROC_DEBUG
	glProcInfo.stTranLog.cntAmt = 0;//2014-9-19 ack POS to erase amt records
#endif

    // Update DCC report file. reduce opt-in and increase opt-out
    PPDCC_UpdateTransStatistic(glProcInfo.stTranLog.stHolderCurrency.sCurrencyCode,
		szOptInAmount, '-',
		szOptOutAmount, '+');

	DispAccepted();
    PubBeepOk();
    // Print new receipt
#if defined(_P60_S1_) || defined(_S_SERIES_)
	SetOffBase(OffBaseDisplay);
#endif
    LoadTranLog(&glProcInfo.stTranLog, glSysCtrl.uiLastRecNo);
    FindAcq(glProcInfo.stTranLog.ucAcqKey);
    FindIssuer(glProcInfo.stTranLog.ucIssuerKey);
	PrintReceipt(PRN_NORMAL);
    return 0;
}

void PPDCC_TransOptOut(void)
{
	int		iRet;
	uchar	ucResult, szBuff[32], szCurrName[4+1], szNewTotalAmt[12+1], szNewTipAmt[12+1];
    SYS_PROC_INFO   stProcBak;

    if (!PPDCC_ChkIfHaveDccAcq())
    {
        return;
    }

	PubShowTitle(TRUE, (uchar *)_T("DCC OPT-OUT     "));
	if( !ChkEdcOption(EDC_NOT_ADJUST_PWD) )
	{
		if( PasswordAdjust()!=0 )
		{
			return ;
		}
	}

	if( GetTranLogNum(ACQ_ALL)==0 )
	{
		DispAdjustTitle();
		PubDispString(_T("EMPTY BATCH"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(5);
		return;
	}

	while( 1 )
	{
		iRet = AdjustInput();
		if( iRet!=0 )
		{
			return;
		}

        if (!PPDCC_ChkIfDccAcq())
        {
		    PubShowMsg(4, _T("NOT DCC TRAN"));
		    PubBeepErr();
		    PubWaitKey(3);
            continue;
        }

		while( 2 )
		{
            // Input new amount in local currency
            memset(szBuff, 0, sizeof(szBuff));
            PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, LEN_TRAN_AMT, szBuff);     

			App_ConvAmountLocal(szBuff, szNewTotalAmt, 0);
			PubShowTitle(TRUE, (uchar *)_T("DCC OPT-OUT     "));
            PubDispString(szNewTotalAmt, 2|DISP_LINE_RIGHT);
			PubDispString(_T("NEW TOTAL"), 4|DISP_LINE_LEFT);
			sprintf((char *)szCurrName, "%.3s", glSysParam.stEdcInfo.stLocalCurrency.szName);
			memset(szNewTotalAmt, 0, sizeof(szNewTotalAmt));
			memset(szNewTipAmt, 0, sizeof(szNewTipAmt));
			ucResult = PubGetAmount(szCurrName, glSysParam.stEdcInfo.stLocalCurrency.ucDecimal, 1, 10, szNewTotalAmt, USER_OPER_TIMEOUT, 0,0);
		
			if( ucResult!=0 )
			{
				return;
			}
            PubAddHeadChars(szNewTotalAmt, LEN_TRAN_AMT, '0');

            // Calculate new tip amount
			PubAscSub(szNewTotalAmt, glProcInfo.stTranLog.szAmount, 12, szNewTipAmt);
            PubAddHeadChars(szNewTipAmt, LEN_TRAN_AMT, '0');

            // Validate and confirm
			AmtConvToBit4Format(szNewTotalAmt, glProcInfo.stTranLog.stTranCurrency.ucIgnoreDigit);
			if( !ValidAdjustAmount(glProcInfo.stTranLog.szAmount, szNewTotalAmt) )
			{
				continue;
			}
			if (!ValidBigAmount(szNewTotalAmt))
			{
				continue;
			}

            PubDispString(_T("TOTAL           "), 2|DISP_LINE_LEFT);
	        DispAmount_Currency(4, szNewTotalAmt, &glSysParam.stEdcInfo.stLocalCurrency);
	        PubDispString(_T("CORRECT ? Y/N"), DISP_LINE_LEFT|6);
			if (PubYesNo(USER_OPER_TIMEOUT))
			{
				break;
			}

            // Backup
            stProcBak = glProcInfo;

            // This will change the original glProcInfo data
            iRet = MatchCardTable(glProcInfo.stTranLog.szPan,
                                    glProcInfo.stTranLog.ucTranType,
                                    1);
	        if (iRet!=0)
	        {
			    PubShowTitle(TRUE, (uchar *)_T("DCC OPT-OUT     "));
		        PubShowMsg(4, _T("DCC PARA ERROR"));
                PubBeepErr();
                PubWaitKey(5);
		        return;
	        }

            if (!ChkSettle())
            {
                return;
            }

            DispProcess();

            OptoutRecord(&stProcBak, szNewTotalAmt, szNewTipAmt);
            return;
		}	// end of while( 2
	}	// end of while( 1
}

uchar PPDCC_ChkIfNeedFinalSelect(void)
{
	if (!PPDCC_ChkIfDccAcq())
	{
		return FALSE;
	}
	if ((glProcInfo.stTranLog.ucTranType!=SALE) &&
		(glProcInfo.stTranLog.ucTranType!=OFF_SALE) )
	{
		return FALSE;
	}
	if (ChkEdcOption(EDC_TIP_PROCESS))
	{
		return FALSE;
	}
	if (!ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount))
	{
		return FALSE;
	}

	return TRUE;
}

// After auto-optin, provide screen for selection
int PPDCC_FinalSelect(void)
{
	uchar   ucKeyVal, ucTimeOut;
    int     iRet;

	if (!ChkSettle())
	{
		return -1;
	}

	if (ChkIfZeroAmt(glProcInfo.stTranLog.szAmount))	//build124
	{
		return -2;
	}

	// Select DCC / OPT-OUT
	// Use [1]/[2], not allow [ENTER]
	ScrCls();
	DispTransName();
	if ((glProcInfo.stTranLog.ucTranType==AUTH) || (glProcInfo.stTranLog.ucTranType==PREAUTH))
	{
		PubDispString(_T("SELECT FOR AUTH:"), 2|DISP_LINE_LEFT);
	}
	else
	{
		PubDispString(_T("SELECT TO PAY:"), 2|DISP_LINE_LEFT);
	}

	ScrPrint(0, 4, CFONT, "1. %3.3s", glSysParam.stEdcInfo.stLocalCurrency.szName);
	ScrPrint(0, 6, CFONT, "2. %3.3s", glProcInfo.stTranLog.stHolderCurrency.szName);

    ucTimeOut = 15;
	TimerSet(0, (ushort)(60*ucTimeOut*10));
	while (1)
	{
    //build103: The card holder choice should be hold forever until cardholder make a choice
    #ifndef WAIT_CARDHOLDER_CHOICE
		    if (TimerCheck(0)==0)
		    {
			    break;
		    }
    #endif

        if (PubKeyPressed())
        {
		    ucKeyVal = getkey();
		    if (ucKeyVal==KEY1)
		    {
			    break;
		    }
		    if (ucKeyVal==KEY2)
		    {
			    return 0;
		    }
        }
	}

	iRet = OptoutRecord(&glProcInfo, glProcInfo.stTranLog.szAmount, "000000000000");
#ifdef AMT_PROC_DEBUG
			//2014-9-18 each time come to glAmount or szAmount
			sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(47)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(47)%.13s",glProcInfo.stTranLog.szAmount);
			glProcInfo.stTranLog.cntAmt++;
	//		//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 13: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
    return iRet;
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
void PPDCC_ResetTransStatistic(void)
{
    int iFile, ii;
    
    memset(&sg_DccStat, 0, sizeof(sg_DccStat));
    for (ii=0; glCurrency[ii].szName[0]; ii++)
    {
        sprintf(sg_DccStat.astCurrency[ii].szOptInAmount,  "%012d", (int)0);
        sprintf(sg_DccStat.astCurrency[ii].szOptOutAmount, "%012d", (int)0);
    }
    memcpy(sg_DccStat.astCurrency[0].sCurrencyCode, glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, 2);

    iFile = open(FILE_DCC_STAT, O_RDWR|O_CREATE);
    if (iFile>=0)
    {
        seek(iFile, 0, SEEK_SET);
        write(iFile, (unsigned char *)&sg_DccStat, sizeof(sg_DccStat));
    }
    close(iFile);
}

int PPDCC_UpdateTransStatistic(uchar *psCurrency,
                               uchar *pszOptInAmt,  char bIsAddOptIn,
                               uchar *pszOptOutAmt, char bIsAddOptOut)
{
    int     ii;
    uchar   ucRet;
    STAT_UNIT   *pStat;

    if (fexist(FILE_DCC_STAT)<0)
    {
        PPDCC_ResetTransStatistic();
    }

    ucRet = PubFileRead(FILE_DCC_STAT, 0, &sg_DccStat, sizeof(sg_DccStat));
    if (ucRet)
    {
        return 2;
    }

    // Update DCC count -------------------------------------------------------------

    if (memcmp(psCurrency, "\x00\x00", 2)==0)
    {
        // Domestic transaction will store in slot 0
        ii = 0;
        pStat = &sg_DccStat.astCurrency[ii];
        pStat->iOptOutCount++;
        memcpy(pStat->sCurrencyCode, glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, 2);
        PubAscAdd(pStat->szOptOutAmount, pszOptOutAmt, 12, pStat->szOptOutAmount);
    }
    else
    {
        // Find the slot to store.
        for (ii=1; glCurrency[ii].szName[0]; ii++)
        {
            if (memcmp(sg_DccStat.astCurrency[ii].sCurrencyCode, "\x00\x00", 2)==0)
            {
                memcpy(sg_DccStat.astCurrency[ii].sCurrencyCode, psCurrency, 2);
                break;
            }
            else if (memcmp(sg_DccStat.astCurrency[ii].sCurrencyCode, psCurrency, 2)==0)
            {
                break;
            }
        }
        if (glCurrency[ii].szName[0]==0)
        {
            return 3;
        }
        pStat = &sg_DccStat.astCurrency[ii];

        if (pszOptInAmt!=NULL)
        {
            // add or remove a record
            if (bIsAddOptIn=='+')
            {
                pStat->iOptInCount++;
            } 
            else if (bIsAddOptIn=='-')
            {
                pStat->iOptInCount--;
            }

            // Increase or reduce amount
            if ((bIsAddOptIn=='+') || (bIsAddOptIn=='>'))
            {
                PubAscAdd(pStat->szOptInAmount, pszOptInAmt, 12, pStat->szOptInAmount);
            } 
            else
            {
                PubAscSub(pStat->szOptInAmount, pszOptInAmt, 12, pStat->szOptInAmount);
            }
        }

        if (pszOptOutAmt!=NULL)
        {
            // add or remove a record
            if (bIsAddOptOut=='+')
            {
                pStat->iOptOutCount++;
            } 
            else if (bIsAddOptOut=='-')
            {
                pStat->iOptOutCount--;
            }

            // Increase or reduce amount
            if ((bIsAddOptOut=='+') || (bIsAddOptOut=='>'))
            {
                PubAscAdd(pStat->szOptOutAmount, pszOptOutAmt, 12, pStat->szOptOutAmount);
            }
            else
            {
                PubAscSub(pStat->szOptOutAmount, pszOptOutAmt, 12, pStat->szOptOutAmount);
            }
        }
    }

    return PubFileWrite(FILE_DCC_STAT, OFFSET(DCC_STAT, astCurrency[ii]), &sg_DccStat.astCurrency[ii], sizeof(STAT_UNIT));
}

int PPDCC_UpdateReverStatistic(void)
{
    uchar   ucRet;

    if (!PPDCC_ChkIfDccAcq())
    {
        return 1;
    }
    if (fexist(FILE_DCC_STAT)<0)
    {
        PPDCC_ResetTransStatistic();
    }
    ucRet = PubFileRead(FILE_DCC_STAT, 0, &sg_DccStat, sizeof(sg_DccStat));
    if (ucRet)
    {
        return 2;
    }
    // Update reversal count
    sg_DccStat.usReverCnt++;
    return PubFileWrite(FILE_DCC_STAT, OFFSET(DCC_STAT, usReverCnt),
                        &sg_DccStat.usReverCnt, sizeof(sg_DccStat.usReverCnt));
}

int PPDCC_GetAllTransStatistic(void)
{
    if (fexist(FILE_DCC_STAT)<0)
    {
        PPDCC_ResetTransStatistic();
    }
    return PubFileRead(FILE_DCC_STAT, 0, &sg_DccStat, sizeof(sg_DccStat));
}

int PPDCC_GetReversalCnt(void)
{
    PPDCC_GetAllTransStatistic();
    return sg_DccStat.usReverCnt;
}

// UI for viewing
void PPDCC_ViewStatistic(void)
{
#ifdef WIN32
    int     ii, iTotal;
    uchar   szBuff[32];

    PPDCC_GetAllTransStatistic();
    for (iTotal=0; glCurrency[iTotal].szName[0]; iTotal++)
    {
        if (memcmp(sg_DccStat.astCurrency[iTotal].sCurrencyCode, "\x00\x00", 2)==0)
        {
            break;
        }
    }

    ScrCls();
    ScrPrint(0, 0, ASCII|REVER, "CUR    OUT/IN  AMOUNT");
    for (ii=0; glCurrency[ii].szName[0]; ii++)
    {
        if (memcmp(sg_DccStat.astCurrency[ii].sCurrencyCode, "\x00\x00", 2)!=0)
        {
            App_ConvAmountLocal(sg_DccStat.astCurrency[ii].szOptOutAmount, szBuff, 0);
            ScrPrint(0, (uchar)((ii%3)*2+1), ASCII, "%.3s%X%02X %02d%12.12s",
                                    GetCurrencyName(sg_DccStat.astCurrency[ii].sCurrencyCode),
                                    sg_DccStat.astCurrency[ii].sCurrencyCode[0], sg_DccStat.astCurrency[ii].sCurrencyCode[1],
                                    sg_DccStat.astCurrency[ii].iOptOutCount, szBuff+4);
            App_ConvAmountLocal(sg_DccStat.astCurrency[ii].szOptInAmount, szBuff, 0);
            ScrPrint(0, (uchar)((ii%3)*2+2), ASCII, "       %02d%12.12s",
                                    sg_DccStat.astCurrency[ii].iOptInCount, szBuff+4);
        }
        else
        {
            kbflush();
            getkey();
            break;
        }

        if ((ii%3)==2)
        {
            kbflush();
            getkey();
            ScrClrLine(1,7);
        }
    }
#endif
}


void PPDCC_DispTranLogDetail(ushort uiActRecNo, void *pTranLog)
{
	char		szBuff1[25], szBuff2[25], szSign[2];
	TRAN_LOG	*pstLog;

	ScrClrLine(5, 7);
	pstLog = (TRAN_LOG *)pTranLog;

    memset(szSign, 0, sizeof(szSign));
    if ((pstLog->uiStatus & TS_VOID) || (pstLog->ucTranType==VOID))
    {
        strcpy(szSign, "-");
    }

    memset(szBuff1, 0, sizeof(szBuff1));
    memset(szBuff2, 0, sizeof(szBuff2));
    PubConvAmount(pstLog->stTranCurrency.szName, pstLog->szFrnAmount,
                  pstLog->stTranCurrency.ucDecimal, pstLog->stTranCurrency.ucIgnoreDigit,
                  szBuff1, 0);
    PubConvAmount(pstLog->stTranCurrency.szName, pstLog->szFrnTip,
                  pstLog->stTranCurrency.ucDecimal, pstLog->stTranCurrency.ucIgnoreDigit,
                  szBuff2, 0);

    //ScrPrint(0, 4, ASCII, "DCC DETAIL");
    ScrPrint(96, 4, ASCII|REVER, " %.3s ", pstLog->stTranCurrency.szName);
    if (ChkEdcOption(EDC_TIP_PROCESS))
    {
        ScrPrint(0, 5, ASCII, "FRN BASE%13.13s", szBuff1);
        ScrPrint(0, 6, ASCII, "FRN TIP %13.13s", szBuff2);
    }
    else
    {
        ScrPrint(0, 6, ASCII, "TOTAL   %13.13s", szBuff1);
    }

    memset(szBuff2, 0, sizeof(szBuff2));
    PPDCC_FormatRate(pstLog->szDccRate, szBuff2);
    ScrPrint(0, 7, ASCII, "%3.3s/%-3.3s    %10.10s",
            pstLog->stHolderCurrency.szName, glSysParam.stEdcInfo.stLocalCurrency.szName, szBuff2);
}


//build88S 1.0D: Switch DCC Trans to Domestic
int DccSwitchDomestic(void)
{        
  return SwitchTranToDomestic();
}
//end build88S 1.0D
// end of file
