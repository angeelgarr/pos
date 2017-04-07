
#include "global.h"

/********************** Internal macros declaration ************************/
#define MAX_ECR_MSGLEN      512
#define MAX_ECR_TAGLEN      32
#define MAX_ECR_VALUELEN    64

#define RET_ECR_OK              0
#define RET_ECR_ERR_SYNTAX      -1
#define RET_ECR_ERR_UNKNOWNTAG  -2

#define TAG_ECR_START   "\"<START>\","
#define TAG_ECR_END     "\"<END>\""
#define SYMBOL_QUOTE    '\"'
#define SYMBOL_EQUAL    '='
#define STR_QUOTE       "\""
#define STR_EQUAL       "="
#define STR_ECR_ACK     "\"<START>\",\"<END>\""

#define TAG_ECR_TXNID               "TXNID"
#define TAG_ECR_CMD                 "CMD"
#define TAG_ECR_AMT                 "AMT"
#define TAG_ECR_DETAIL              "DETAIL"
#define TAG_ECR_TXNNO               "TXNNO"
#define TAG_ECR_TXNCONSEQUENTIALNO  "TXNCONSEQUENTIALNO"
#define TAG_ECR_FUNC                "FUNC"
#define TAG_ECR_PLAN                "PLAN"
#define TAG_ECR_INVOICENO           "INVOICENO"
#define TAG_ECR_ACQNAME             "ACQNAME"
//#define TAG_ECR_PWD                 "PWD"//2014-11-17 //2015-10-15 ECR Update: change "STLPWD" to "PWD" Gillian 20161010
/********************** Internal structure declaration *********************/
typedef struct _tagECR_TRAN_INFO 
{
    uchar   szTxnID[6+1];           // Txn ID
    uchar   ucTranType;
    uchar   bTranInstallment;       // Is installment
    uchar   szAmount[12+1];         // same as ISO format
    uchar   szTxnNo[6+1];           // TXNNO
    uchar   szTxnConseqNo[4+1];     // TXNCONSEQUENTIALNO
    ulong   ulInvoice;              // invoice number
    uchar   bDetailRequired;        // ECR requires POS to return detail info
    uchar   ucInstPlan;             // installment tenure (month)
    uchar   szAcqName[10+1];
//	uchar   szPwd[6+1]; //2014-11-17 add settle Gillian 20161010
}ST_ECR_TRAN_INFO;

/********************** Internal functions declaration *********************/
static void EcrShowDebug(char *pszStr, ...);
static int UnpackEcrData(char *pszTag, char *pszValue);
static int GetAllEcrElements(void *pszEcrMsg);
static void StrCatEcrRsp_TxnTypeDateTime(TRAN_LOG *pstLog, char *pszEcrMsgBuff);

/********************** Internal variables declaration *********************/
static ST_ECR_TRAN_INFO g_stEcr;

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/


enum{VAL_ECRCMD_NONE=0, VAL_ECRCMD_SALE, VAL_ECRCMD_VOID, VAL_ECRCMD_GETTXN, VAL_ECRCMD_GETTOTAL};//, VAL_ECRCMD_SETTLE, VAL_ECRCMD_REFUND, VAL_ECRCMD_AUTH, VAL_ECRCMD_OFFLINE, VAL_ECRCMD_ADJUST, VAL_ECRCMD_DCC_ADJUST, VAL_ECRCMD_DCC_OPTOUT, VAL_ECRCMD_REPRINT}; //2014-11-17 add settle//2015-10-15 ECR Update:add new iwwuw //Gillian 20161010 

//typedef uchar (*FNPTR_EcrSend)(uchar *psData, ushort usLen);
FNPTR_EcrSend   FuncEcrSend;
static void EcrShowDebug(char *pszStr, ...)
{
	uchar		szBuff[1024+1];
	va_list		pVaList;

	if( pszStr==NULL || *(uchar *)pszStr==0 )
	{
		return;
	}

	va_start(pVaList, pszStr);
	vsprintf((char*)szBuff, (char*)pszStr, pVaList);
	va_end(pVaList);

    ScrCls();
	ScrPrint(0,0,0,szBuff);
	kbflush();getkey();
}

static int UnpackEcrData(char *pszTag, char *pszValue)
{
    int ii, iCnt;
    struct {
        char    szCmdText[16+1];
        uchar   ucTranType;
    }astCmdTxnMap[] = 
    {
        "SALE",     VAL_ECRCMD_SALE,
        "VOID",     VAL_ECRCMD_VOID,
        "GETTXN",   VAL_ECRCMD_GETTXN,
        "GETTOTAL", VAL_ECRCMD_GETTOTAL,
    };

    if (strcmp(pszTag, TAG_ECR_TXNID)==0)
    {
        sprintf(g_stEcr.szTxnID, "%.*s", sizeof(g_stEcr.szTxnID)-1, pszValue);
    }
    else if (strcmp(pszTag, TAG_ECR_CMD)==0)
    {
        for (ii=0; ii<sizeof(astCmdTxnMap)/sizeof(astCmdTxnMap[0]); ii++)
        {
            if (strcmp(astCmdTxnMap[ii].szCmdText, pszValue)==0)
            {
                g_stEcr.ucTranType = astCmdTxnMap[ii].ucTranType;
                break;
            }
        }
    }
	//2014-11-17 Gillian 20161010
	/*else if(strcmp(pszTag, TAG_ECR_PWD)==0)
	{
		ScrCls();
		ScrPrint(0,2,0, "pszValue = %s", pszValue);
		getkey();

		sprintf((char*)g_stEcr.szPwd, pszValue);
	}*/
    else if (strcmp(pszTag, TAG_ECR_AMT)==0)
    {
        // save amount in ISO8583 DE4 format
        if (strlen(pszValue)<14)
        {
            g_stEcr.szAmount[12] = 0;
            for (ii=strlen(pszValue)-1, iCnt=11; ii>=0 && iCnt>=0; ii--)
            {
                if (isdigit(pszValue[ii]))
                {
                    g_stEcr.szAmount[iCnt] = pszValue[ii];
                    iCnt--;
                }
            }
            while (iCnt>=0)
            {
                g_stEcr.szAmount[iCnt] = '0';
                iCnt--;
            }
        }
    }
    else if (strcmp(pszTag, TAG_ECR_DETAIL)==0)
    {
        g_stEcr.bDetailRequired = (strcmp(pszValue, "Y")==0);
    }
    else if (strcmp(pszTag, TAG_ECR_TXNNO)==0)
    {
        sprintf(g_stEcr.szTxnNo, "%.*s", sizeof(g_stEcr.szTxnNo)-1, pszValue);
    }
    else if (strcmp(pszTag, TAG_ECR_TXNCONSEQUENTIALNO)==0)
    {
        sprintf(g_stEcr.szTxnConseqNo, "%.*s", sizeof(g_stEcr.szTxnConseqNo)-1, pszValue);
    }
    else if (strcmp(pszTag, TAG_ECR_FUNC)==0)
    {
        if (strcmp(pszValue, "INST")==0)
        {
            g_stEcr.bTranInstallment = TRUE;
        }
    }
    else if (strcmp(pszTag, TAG_ECR_PLAN)==0)
    {
        g_stEcr.ucInstPlan = (uchar)atoi(pszValue);
    }
    else if (strcmp(pszTag, TAG_ECR_INVOICENO)==0)
    {
           g_stEcr.ulInvoice = (ulong)atol(pszValue);
    }
    else if (strcmp(pszTag, TAG_ECR_ACQNAME)==0)
    {
        sprintf(g_stEcr.szAcqName, "%.*s", sizeof(g_stEcr.szAcqName)-1, pszValue);
    }
    else
    {
        return -1;
    }

    return 0;
}

static int GetAllEcrElements(void *pszEcrMsg)
{
    char    szBuff[MAX_ECR_MSGLEN+1], szTag[MAX_ECR_TAGLEN+1], szValue[MAX_ECR_VALUELEN+1];
    char    *pStart, *pEnd, *pEqual, *pNow;
    int     iLen, iRet;

    iLen = strlen(pszEcrMsg);    // Check length
    if (iLen>MAX_ECR_MSGLEN)
    {
        return RET_ECR_ERR_SYNTAX;
    }

    pStart = strstr(pszEcrMsg, TAG_ECR_START);    // Locate start
    if (pStart==NULL)
    {
        return RET_ECR_ERR_SYNTAX;
    }
    pStart += strlen(TAG_ECR_START);

    pEnd = strstr(pStart, TAG_ECR_END);    // Locate end
    if (pEnd==NULL)
    {
        return RET_ECR_ERR_SYNTAX;
    }

    memset(szBuff, 0, sizeof(szBuff));    // Get msg body
    memcpy(szBuff, pStart, pEnd-pStart-1);// ignore last ',' in string

    pNow = szBuff;
    while (strlen(pNow)!=0)
    {
        pStart = strstr(pNow, STR_QUOTE);
        if (pStart==NULL)
        {
            EcrShowDebug("ERR START QUOTE");
            return RET_ECR_ERR_SYNTAX;
        }
        pEnd = strstr(pNow+1, STR_QUOTE);
        if (pEnd==NULL)
        {
            EcrShowDebug("ERR END QUOTE");
            return RET_ECR_ERR_SYNTAX;
        }
        pEqual = strstr(pNow+1, STR_EQUAL);
        if ((pEqual==NULL) || (pEqual>pEnd) || (pEqual<pStart))
        {
            EcrShowDebug("ERR EQUAL");
            return RET_ECR_ERR_SYNTAX;
        }

        iLen = pEqual-pStart-1;
        if ((iLen==0) || (iLen>MAX_ECR_TAGLEN))
        {
            EcrShowDebug("ERR TAGLEN");
            return RET_ECR_ERR_SYNTAX;
        }

        iLen = pEnd-pEqual-1;
        if ((iLen==0) || (iLen>MAX_ECR_VALUELEN))
        {
            EcrShowDebug("ERR VALUELEN");
            return RET_ECR_ERR_SYNTAX;
        }

        sprintf(szTag,   "%.*s", pEqual-pStart-1, pStart+1);
        sprintf(szValue, "%.*s", pEnd-pEqual-1,   pEqual+1);

        iRet = UnpackEcrData(szTag, szValue);

        pNow = pEnd+1;
        if (*pNow==',')
        {
            pNow++;
        }
    }

    return 0;
}

int EcrTrans(void *pszEcrMsg)
{
    int iRet;
	uchar   sCurTime[7];//2014-11-4
    memset(&g_stEcr, 0, sizeof(g_stEcr));
    iRet = GetAllEcrElements(pszEcrMsg);
    if (iRet!=0)
    {
        PubShowTitle(TRUE, _T("ECR MESSAGE"));
        PubDispString(_T("INVALID ECR MESSAGE"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(5);
        return ERR_NO_DISP;
    }
#ifdef AMT_PROC_DEBUG//2014-11-4 record time txntype and card number
	//time
	GetTime(sCurTime);
	sprintf((char *)glEcrAMT.glEcrAmtLog[glEcrAmtCnt].szTime, "%02X%02X%02X",sCurTime[3], sCurTime[4], sCurTime[5]);
	//txntype
	glEcrAMT.glEcrAmtLog[glEcrAmtCnt].szTranType = 0;
	EcrGetAmount(glEcrAMT.glEcrAmtLog[glEcrAmtCnt].szEcrAmt, glProcInfo.stTranLog.szTipAmount);//tip ??
	
	////2014-11-4 ttt
	//		ScrCls();
	//		ScrPrint(0,0,0, "2_time: %s",glEcrAMT.glEcrAmtLog[glEcrAmtCnt].szTime);
	//		ScrPrint(0,2,0, "2_type: %d",glEcrAMT.glEcrAmtLog[glEcrAmtCnt].szTranType);
	//		ScrPrint(0,4,0, "2_amt : %s",glEcrAMT.glEcrAmtLog[glEcrAmtCnt].szEcrAmt);
	//		getkey();

	if(glEcrAmtCnt++ > 99)
	{
		glEcrAmtCnt = 0;
	}
#endif
    // If it is a CHECK LINE STATUS message
    if ((strlen(g_stEcr.szTxnID)!=0) && (atoi(g_stEcr.szTxnID)==0)
        && (g_stEcr.ucTranType==VAL_ECRCMD_NONE))
    {
		  //iRet = EcrSendData_Sub((uchar *)STR_ECR_ACK, (ushort)strlen(STR_ECR_ACK), FALSE);
      //  iRet = EcrSendAckStr();
        //if (iRet==0)
      //  {
            PubBeepOk();
      //  }
        return iRet;
    }

    sprintf(glProcInfo.stTranLog.szEcrTxnNo,    "%.6s", g_stEcr.szTxnNo);
    sprintf(glProcInfo.stTranLog.szEcrConseqNo, "%.4s", g_stEcr.szTxnConseqNo);
    glProcInfo.ucEcrCtrl = ECR_BEGIN;


	if (glSysParam.ucManagerOn == 1)
	{
		if (glSysParam.stEdcInfo.ucClssFlag==1 && !ChkOnBase())
		{
#if defined(_P60_S1_) || defined(_S_SERIES_)
			SetOffBase(OffBaseDisplay);
#endif
		
			PubShowTitle(TRUE, (uchar *)_T("      POLL      "));
			DispWait();
			iRet = ClssInit();
			if (iRet)
			{
				ProcError_Wave(iRet);
				return iRet;
			}
		}
	}//2013130

    // TODO : process different transactions below
    if (g_stEcr.ucTranType==VAL_ECRCMD_SALE)
    {	
        iRet = TransSale(g_stEcr.bTranInstallment);
        if (iRet!=0)
        {
#ifdef AMT_PROC_DEBUG//2014-9-22
	glProcInfo.stTranLog.cntAmt = 0;
#endif
            EcrSendTranFail();
        }
        CommOnHook(FALSE);
		PromptRemoveICC();
        return iRet;
    }
    else if (g_stEcr.ucTranType==VAL_ECRCMD_VOID)
    {
        iRet = TransVoid();
        if (iRet!=0)
        {
#ifdef AMT_PROC_DEBUG//2014-9-22
	glProcInfo.stTranLog.cntAmt = 0;
#endif
            EcrSendTranFail();
        }
        CommOnHook(FALSE);
		PromptRemoveICC();
        return iRet;
    }
#ifdef REAL_ECR
	else if (g_stEcr.ucTranType == VAL_ECRCMD_REFUND)//2014-11-18 add refund
	{
		memcpy(glPwd, g_stEcr.szPwd, strlen((char*)g_stEcr.szPwd));//2015-10-28 ECR Update:
		iRet = TransRefund();
		if(iRet != 0)
		{
			EcrSendTranFail();
		}
		memset(glPwd,0,7);//2014-11-17 
	}

#endif
    else if (g_stEcr.ucTranType==VAL_ECRCMD_GETTXN)
    {
        g_stEcr.bDetailRequired = TRUE;
        iRet = GetRecordByInvoice(g_stEcr.ulInvoice, TS_ALL_LOG, &glProcInfo.stTranLog);
		if( iRet!=0 )
		{
#ifdef AMT_PROC_DEBUG//2014-9-22
	glProcInfo.stTranLog.cntAmt = 0;
#endif
            EcrSendTranFail();
			return ERR_NO_DISP;
		}
	    FindAcq(glProcInfo.stTranLog.ucAcqKey);
	    FindIssuer(glProcInfo.stTranLog.ucIssuerKey);
        return EcrSendTransSucceed();
    }
    else if (g_stEcr.ucTranType==VAL_ECRCMD_GETTOTAL)
    {
        iRet = FindAcqIdxByName(g_stEcr.szAcqName, TRUE);
        if (iRet>=MAX_ACQ)
        {
#ifdef AMT_PROC_DEBUG//2014-9-22
	glProcInfo.stTranLog.cntAmt = 0;
#endif
            EcrSendTranFail();
            PubBeepErr();
            return ERR_NO_DISP;
        }
        SetCurAcq((uchar)iRet);
        CalcTotal(glCurAcq.ucKey);
        return EcrSendAcqTotal();
    }

    return 0;
}

int EcrGetAmount(uchar *pszAmount, uchar *pszTip)
{
    if (strlen(g_stEcr.szAmount)!=0)
    {
        sprintf(pszAmount, "%.12s", g_stEcr.szAmount);
        sprintf(pszTip, "000000000000");
        return 0;
    }
    return -1;
}

ulong EcrGetTxnID(void)
{
    return (ulong)atol(g_stEcr.szTxnID);
}

ulong EcrGetInvoice(void)
{
    return g_stEcr.ulInvoice;
}

int EcrGetInstPlan(uchar *pucMonth)
{
    if (g_stEcr.ucInstPlan)
    {
        *pucMonth = g_stEcr.ucInstPlan;
        return 0;
    }
    return -1;
}

int EcrSendAckStr(void)
{
 
	int  iCnt, iRet;
    uchar   ucRet,sBuff[512];
	memset(sBuff,0,sizeof(sBuff));
	
	strcpy(sBuff,(uchar *)STR_ECR_ACK);
	for (iCnt=0; iCnt<(ushort)strlen(STR_ECR_ACK); iCnt++)
	{
		ucRet = PortSend(glSysParam.stECR.ucPort, sBuff[iCnt]);
		
		if (ucRet!=0)
		{
			return ucRet;
		}
	}

    return 0;
}

static void StrCatEcrRsp_TxnTypeDateTime(TRAN_LOG *pstLog, char *pszEcrMsgBuff)
{
    uchar   szBuff1[32], szBuff2[32];

    if (pstLog->ucTranType==SALE)
    {
        strcat(pszEcrMsgBuff, "\"TXNTYPE=SALE\",");
    }
    else if (pstLog->ucTranType==VOID)
    {
        strcat(pszEcrMsgBuff, "\"TXNTYPE=VOID\",");
    }

    PubGetDateTime(szBuff1);
    sprintf(szBuff2, "\"TXNDATE=%.8s\",", pstLog->szDateTime);
    strcat(pszEcrMsgBuff, szBuff2);
    sprintf(szBuff2, "\"TXNTIME=%.4s\",", pstLog->szDateTime+8);
    strcat(pszEcrMsgBuff, szBuff2);
}

int EcrSendTransSucceed(void)
{
    char    szEcrMsgBuff[MAX_ECR_MSGLEN+1], szBuff[192];
//    FNPTR_EcrSend   FuncEcrSend;


    if (glProcInfo.ucEcrCtrl==ECR_BEGIN)
    {	 
 
        sprintf(szEcrMsgBuff, TAG_ECR_START);

        strcat(szEcrMsgBuff, "\"STATUS=A\",");
		//2014-6-19 to make the ECR return message more standard accroding to ECR spec
		sprintf(szBuff+32, "%06lu", glProcInfo.stTranLog.ulInvoiceNo);
		sprintf(szBuff, "\"TXNID=%s\",", szBuff+32);
		strcat(szEcrMsgBuff, szBuff);

        StrCatEcrRsp_TxnTypeDateTime(&glProcInfo.stTranLog, szEcrMsgBuff);

        sprintf(szBuff, "\"APPCODE=%s\",", glProcInfo.stTranLog.szAuthCode);
        strcat(szEcrMsgBuff, szBuff);
		if (glSysParam.stEdcInfo.ucPANmaskFlag)
		{
			 MaskPan(glProcInfo.stTranLog.szPan, szBuff+32);//build 0122c
		}
		else
		{
			strcpy(szBuff+32,glProcInfo.stTranLog.szPan);//squall 20130122  no need panmask
		}
        sprintf(szBuff, "\"PAN=%s\",", szBuff+32);
        strcat(szEcrMsgBuff, szBuff);

        sprintf(szBuff, "\"EXPDATE=%.2s%.2s\",", glProcInfo.stTranLog.szExpDate+2, glProcInfo.stTranLog.szExpDate);
        strcat(szEcrMsgBuff, szBuff);

        PubConvAmount(NULL, glProcInfo.stTranLog.szAmount, glSysParam.stEdcInfo.stLocalCurrency.ucDecimal,
                        glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit, szBuff+32, 0);
        sprintf(szBuff, "\"AMT=%s\",", szBuff+32);
        strcat(szEcrMsgBuff, szBuff);

        // When "DETAIL=Y"
        if (g_stEcr.bDetailRequired)
        {
	        if( glProcInfo.stTranLog.uiEntryMode & MODE_SWIPE_INPUT )
	        {
		        strcat(szEcrMsgBuff, "\"ENTRYMODE=S\",");
	        }
	        else if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	        {
		        strcat(szEcrMsgBuff, "\"ENTRYMODE=C\",");
	        }
	        else if( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
			         (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	        {
		        strcat(szEcrMsgBuff, "\"ENTRYMODE=F\",");
	        }
			else if (glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS )
			{
				strcat(szEcrMsgBuff, "\"ENTRYMODE=T\",");
			}
	        else
	        {
		        strcat(szEcrMsgBuff, "\"ENTRYMODE=M\",");
	        }

            sprintf(szBuff+32, "%06lu", glProcInfo.stTranLog.ulInvoiceNo);
            sprintf(szBuff, "\"INVOICENO=%s\",", szBuff+32);
            strcat(szEcrMsgBuff, szBuff);

            sprintf(szBuff, "\"TERMINALID=%s\",", glCurAcq.szTermID);
            strcat(szEcrMsgBuff, szBuff);                

            sprintf(szBuff, "\"MERCHANTID=%s\",", glCurAcq.szMerchantID);
            strcat(szEcrMsgBuff, szBuff);
                
            ConvIssuerName(glCurIssuer.szName, szBuff+32);
            sprintf(szBuff, "\"CARDTYPE=%.10s\",", szBuff+32);
            strcat(szEcrMsgBuff, szBuff);

            if (strlen(glProcInfo.stTranLog.szHolderName)!=0)
            {
                sprintf(szBuff, "\"CARDHOLDERNAME=%s\",", glProcInfo.stTranLog.szHolderName);
                strcat(szEcrMsgBuff, szBuff);
            }

            sprintf(szBuff+32, "%06lu", glCurAcq.ulCurBatchNo);
            sprintf(szBuff, "\"BATCHNO=%s\",", szBuff+32);
            strcat(szEcrMsgBuff, szBuff);

            sprintf(szBuff, "\"REFERENCENO=%s\",", glProcInfo.stTranLog.szRRN);
            strcat(szEcrMsgBuff, szBuff);

            if (strlen(g_stEcr.szTxnNo)!=0)
            {
                sprintf(szBuff, "\"TXNNO=%s\",", g_stEcr.szTxnNo);
                strcat(szEcrMsgBuff, szBuff);
            }

            if (strlen(g_stEcr.szTxnConseqNo)!=0)
            {
                sprintf(szBuff, "\"TXNCONSEQUENTIALNO=%s\",", g_stEcr.szTxnConseqNo);
                strcat(szEcrMsgBuff, szBuff);
            }
            
	        if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
	        {
                if (strlen(glProcInfo.stTranLog.szAppPreferName)!=0)
                {
		            sprintf(szBuff, "\"APP=%.16s\",", glProcInfo.stTranLog.szAppPreferName);
                } 
                else
                {
		            sprintf(szBuff, "\"APP=%.16s\",", glProcInfo.stTranLog.szAppLabel);
                }
                strcat(szEcrMsgBuff, szBuff);

		        PubBcd2Asc0(glProcInfo.stTranLog.sAID, glProcInfo.stTranLog.ucAidLen, szBuff+32);
		        PubTrimTailChars(szBuff+32, 'F');
		        sprintf(szBuff, "\"AID=%s\",", szBuff+32);
                strcat(szEcrMsgBuff, szBuff);

		        PubBcd2Asc0(glProcInfo.stTranLog.sAppCrypto, 8, szBuff+32);
		        sprintf(szBuff, "\"TC=%s\",", szBuff+32);
                strcat(szEcrMsgBuff, szBuff);
            }
        }

        strcat(szEcrMsgBuff, TAG_ECR_END);
 
        EcrSendData((uchar *)szEcrMsgBuff, (ushort)strlen(szEcrMsgBuff));
        glProcInfo.ucEcrCtrl = ECR_SUCCEED;
   }
    return 0;
}

int EcrSendTranFail(void)
{
    char    szEcrMsgBuff[MAX_ECR_MSGLEN+1];
//    FNPTR_EcrSend   FuncEcrSend;

    if (glProcInfo.ucEcrCtrl==ECR_BEGIN)
    {
        sprintf(szEcrMsgBuff, TAG_ECR_START);
        strcat(szEcrMsgBuff, "\"STATUS=R\",");
        StrCatEcrRsp_TxnTypeDateTime(&glProcInfo.stTranLog, szEcrMsgBuff);
        strcat(szEcrMsgBuff, TAG_ECR_END);

        // send response
	//	PubDebugOutput("Ecr Msg", &glEdcMsgPtr->UserMsg, 4, DEVICE_SCR, HEX_MODE);

		EcrSendData((uchar *)szEcrMsgBuff, (ushort)strlen(szEcrMsgBuff));// send

        glProcInfo.ucEcrCtrl = ECR_REJECT;
    }
    return 0;
}

int EcrSendAcqTotal(void)
{
    char    szEcrMsgBuff[MAX_ECR_MSGLEN+1], szBuff[192];
///    FNPTR_EcrSend   FuncEcrSend;

    if (glProcInfo.ucEcrCtrl==ECR_BEGIN)
    {
        sprintf(szEcrMsgBuff, TAG_ECR_START);

        sprintf(szBuff, "\"ACQNAME=%s\",", g_stEcr.szAcqName);
        strcat(szEcrMsgBuff, szBuff);

        sprintf(szBuff, "\"SALENUM=%d\",", (int)glTransTotal.uiSaleCnt);
        strcat(szEcrMsgBuff, szBuff);
        PubConvAmount(NULL, glTransTotal.szSaleAmt, glSysParam.stEdcInfo.stLocalCurrency.ucDecimal,
                        glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit, szBuff+32, 0);
        sprintf(szBuff, "\"SALEAMT=%s\",", szBuff+32);
        strcat(szEcrMsgBuff, szBuff);

        sprintf(szBuff, "\"VOIDNUM=%d\",", (int)glTransTotal.uiVoidSaleCnt);
        strcat(szEcrMsgBuff, szBuff);
        PubConvAmount(NULL, glTransTotal.szVoidSaleAmt, glSysParam.stEdcInfo.stLocalCurrency.ucDecimal,
                        glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit, szBuff+32, 0);
        sprintf(szBuff, "\"VOIDAMT=%s\",", szBuff+32);
        strcat(szEcrMsgBuff, szBuff);

        sprintf(szBuff, "\"REFUNDNUM=%d\",", (int)glTransTotal.uiRefundCnt);
        strcat(szEcrMsgBuff, szBuff);
        PubConvAmount(NULL, glTransTotal.szRefundAmt, glSysParam.stEdcInfo.stLocalCurrency.ucDecimal,
                        glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit, szBuff+32, 0);
        sprintf(szBuff, "\"REFUNDAMT=%s\",", szBuff+32);
        strcat(szEcrMsgBuff, szBuff);

        sprintf(szBuff, "\"VRFNUM=%d\",", (int)glTransTotal.uiVoidRefundCnt);
        strcat(szEcrMsgBuff, szBuff);
        PubConvAmount(NULL, glTransTotal.szVoidRefundAmt, glSysParam.stEdcInfo.stLocalCurrency.ucDecimal,
                        glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit, szBuff+32, 0);
        sprintf(szBuff, "\"VRFAMT=%s\",", szBuff+32);
        strcat(szEcrMsgBuff, szBuff);

        strcat(szEcrMsgBuff, TAG_ECR_END);

//        FuncEcrSend = (FNPTR_EcrSend)(glEdcMsgPtr->UserMsg);
        EcrSendData((uchar *)szEcrMsgBuff, (ushort)strlen(szEcrMsgBuff));
        glProcInfo.ucEcrCtrl = ECR_SUCCEED;
    }
    return 0;
}

// end of file
