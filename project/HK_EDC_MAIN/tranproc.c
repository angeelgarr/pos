
#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static void PackInvoice(STISO8583 *pstSendPack, ulong ulInvoice);
static void ModifyProcessCode(void);
static void SetEntryMode(TRAN_LOG *pstLog);
static void SetIndirectCUPEntryMode(TRAN_LOG *pstLog);
static void SetAmexEntryMode(TRAN_LOG *pstLog);
static void SetStdEntryMode(TRAN_LOG *pstLog);
static void SetCondCode(void);
static void SetInstAndAirTicketData(void);
static void SetEMVTermIndic(TRAN_LOG * pstLog);
static int  TransCashSub(void);
static int  TransSaleSub(void);
static int  TransAuthSub(uchar ucTranType);
static int  GetOfflineTrans(uchar ucTypeFlag);
static int  TransSettleSub(void);
static int  TransUpLoad(void);
static int DAHSING_setInstalmentField63(void *psSendPack,TRAN_LOG *stTranlog);
static int DAHSING_CUPField63(void *psSendPack); //add by richard 20161108
static int DAHSING_SetField61(void *psSendPack);



int OfflineSendAdjust(void);      //Build88S

extern void DispHostRspMsg(uchar *pszRspCode, HOST_ERR_MSG *pstMsgArray);//build88R Patch
/********************** Internal variables declaration *********************/

/********************** external reference declaration *********************/
extern void CLSSPreTrans(void);
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

void PackInvoice(STISO8583 *pstSendPack, ulong ulInvoice)
{
    //bit62 ChkIfWLB()
    memset(pstSendPack->sField62, 0, 2+18+1);
    if (glProcInfo.stTranLog.ucTranType == INSTALMENT && ChkIfWLB())
    {
        memcpy(pstSendPack->sField62, "\x00\x12", 2);
        sprintf((char *)&(pstSendPack->sField62[2]), "%06lu", ulInvoice);
        sprintf((char *)&(pstSendPack->sField62[8]), "%02lu", glProcInfo.stTranLog.ucInstalment);
        memset(&pstSendPack->sField62[10], 0x20, 10);
    }
    else
    {
        memcpy(pstSendPack->sField62, "\x00\x06", 2);
        sprintf((char *)&(pstSendPack->sField62[2]), "%06lu", ulInvoice);
    }
}

// 设置请求包的公共bits
// set bit content of ISO8583.
// Work as the "IsoInitSendMsg()" did in old style.
void SetCommReqField(void)
{
#ifdef ENABLE_EMV
    int iLength;
#endif
    uchar   szTotalAmt[12+1], szBuff[128], szLocalTime[14+1] ,ucMaxNum,ucCnt,ucTemp, psTemp[1];
    int i;
    uchar szAmtBuf[12+1];   //build88R

    if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
    {
        memset(&glTMSSend, 0, sizeof(STTMS8583));
        sprintf((char *)glTMSSend.szMsgCode, "%.*s", LEN_MSG_CODE,
                glTranConfig[glProcInfo.stTranLog.ucTranType].szTxMsgID);
        sprintf((char *)glTMSSend.szProcCode,   "%.*s", LEN_PROC_CODE,
                glTranConfig[glProcInfo.stTranLog.ucTranType].szProcCode);
        sprintf((char *)glTMSSend.szNii,    "%.*s", LEN_NII,     glCurAcq.szNii);
        sprintf((char *)glTMSSend.szTermID, "%.*s", LEN_TERM_ID, glCurAcq.szTermID);
        sprintf((char *)glTMSSend.szSTAN, "%06lu", glSysCtrl.ulSTAN);
        return;
    }

    //ISO8583使用说明
    //Step3: 在这里设置打包时需要的数据，该域赋值则送，为空时则不送(默认的初始值全为空)
    //所有的报文都需要用到的域可在//set common bits下进行统一赋值
    //其它根据交易不同条件不同时可送可不送的分别处理
    //另注意sz型与s型开头成员变量,sz型直接赋所需要上送的值，s型需要在前两位赋值为该域的长度
    //例如: // bit 62, ROC/SOC定义为  sField62
    //则先赋长度值memcpy(glSendPack.sField62, "\x00\x06", 2);
    //再赋数据值sprintf((char *)&glSendPack.sField62[2], "%06lu", glProcInfo.stTranLog.ulInvoiceNo);

    // Usage of ISO8583 module (For step 2, see in st8583.h)
    // Step 3: To set the data need in packing here.
    // If the glSendPack.xxxxx is filled with value, this bit will be sent, and vice versa.

    // Note that there're 2 kinds of member in glSendPack:
    //   glSendPack.szxxxxxx : this type means "string end with zero", the actual length can be determined by strlen()
    //   glSendPack.sxxxxxx  : the first 2 bytes contains length infomation, if length=0x01A0, should be "\x01\xA0"
    //     e.g. : bit62
    //             memcpy(glSendPack.sField62, "\x00\x06", 2);
    //             sprintf((char *)&glSendPack.sField62[2], "%06lu", glProcInfo.stTranLog.ulInvoiceNo);

    // set common bits
    memset(&glSendPack, 0, sizeof(STISO8583));
    sprintf((char *)glSendPack.szMsgCode,    "%.*s", LEN_MSG_CODE,
            glTranConfig[glProcInfo.stTranLog.ucTranType].szTxMsgID);

    sprintf((char *)glSendPack.szProcCode,   "%.*s", LEN_PROC_CODE,
            glTranConfig[glProcInfo.stTranLog.ucTranType].szProcCode);

    if(glProcInfo.stTranLog.ucTranType==PREAUTH && ChkIfIndirectCupAcq())
    {
        memcpy(glSendPack.szProcCode, "000000", LEN_PROC_CODE);
    }
	if(glProcInfo.stTranLog.ucTranType == SETTLEMENT && ChkIfIndirectCupAcq()) //added by jeff_xiehuan20170330
	{
		sprintf((char *)glSendPack.szMsgCode,    "%.*s", LEN_MSG_CODE,"0505");
	}
	if(glProcInfo.stTranLog.ucTranType == UPLOAD && ChkIfIndirectCupAcq())
	{
		sprintf((char *)glSendPack.szMsgCode,    "%.*s", LEN_MSG_CODE,"0325");
	}
    //bit 63

    // modify bit 3, process code
    ModifyProcessCode();

    sprintf((char *)glSendPack.szNii,        "%.*s", LEN_NII,         glCurAcq.szNii);
    sprintf((char *)glSendPack.szTermID,     "%.*s", LEN_TERM_ID,     glCurAcq.szTermID);
    sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, glCurAcq.szMerchantID);

    if ((glProcInfo.stTranLog.ucTranType == TC_SEND || glProcInfo.stTranLog.ucTranType == UPLOAD) && ChkIfDahOrBCM())//dashing need tc share the same stan with original transaction
		//2015-5-22 DahSing already handled the TC issue!!!
    {
        sprintf((char *)glSendPack.szSTAN, "%06lu", glProcInfo.stTranLog.ulSTAN);
        //glProcInfo.stTranLog.ulSTAN = glSysCtrl.ulSTAN;
    }
    else
    {
        sprintf((char *)glSendPack.szSTAN, "%06lu", glSysCtrl.ulSTAN);  //??
        glProcInfo.stTranLog.ulSTAN = glSysCtrl.ulSTAN;
    }

    if( glProcInfo.stTranLog.ucTranType==SETTLEMENT || glProcInfo.stTranLog.ucTranType==UPLOAD ||
        glProcInfo.stTranLog.ucTranType==LOGON || glProcInfo.stTranLog.ucTranType==LOAD_CARD_BIN ||
        glProcInfo.stTranLog.ucTranType==LOAD_RATE_REPORT)
    {
        return;
    }
    // bit 4, transaction amount
    if ( PPDCC_ChkIfDccAcq() &&
         (!ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount) || !ChkIfZeroAmt(glProcInfo.stTranLog.szFrnTip)) ) // HJJ 20111004
    {
        // PPDCC with tips

        PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
        sprintf((char *)glSendPack.szTranAmt,   "%.*s", LEN_TRAN_AMT,   szTotalAmt);
        sprintf((char *)glSendPack.szExtAmount, "%.*s%.*s",
                LEN_EXT_AMOUNT, glProcInfo.stTranLog.szTipAmount,
                LEN_EXT_AMOUNT, glProcInfo.stTranLog.szFrnTip);

    }
    else if( !ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount) )//have tips
    {
        PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTotalAmt);
        sprintf((char *)glSendPack.szTranAmt,   "%.*s", LEN_TRAN_AMT,   szTotalAmt);
        if (glProcInfo.stTranLog.ucTranType!=TC_SEND)
        {
            sprintf((char *)glSendPack.szExtAmount, "%.*s", LEN_EXT_AMOUNT, glProcInfo.stTranLog.szTipAmount);
        }
        if (ChkIfAmex())
        {
            if( glProcInfo.stTranLog.ucTranType==REFUND )
            {
                glSendPack.szExtAmount[0] = 0;
            }
            if( glProcInfo.stTranLog.ucTranType==VOID || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
            {
                sprintf(glSendPack.szTranAmt, "%012lu", 0L);
                glSendPack.szExtAmount[0] = 0;
            }
        }
    }
    else//without tips
    {
        sprintf((char *)glSendPack.szTranAmt, "%.*s", LEN_TRAN_AMT, glProcInfo.stTranLog.szAmount);
#ifdef AMT_PROC_DEBUG
			//2014-9-18 each time come to glAmount or szAmount
			sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(51)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(51)%.13s",glProcInfo.stTranLog.szAmount);
			glProcInfo.stTranLog.cntAmt++;
	//		//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 24: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
        if( ChkIfAmex() )
        {
            if( (glProcInfo.stTranLog.ucTranType==VOID) || (glProcInfo.stTranLog.uiStatus & TS_VOID) )
            {
                sprintf(glSendPack.szTranAmt, "%012lu", 0L);
                glSendPack.szExtAmount[0] = 0;
            }
        }
    }

    //bit 6, bit 10
    // PP-DCC
    if (PPDCC_ChkIfDccAcq())
    {
        if ((glProcInfo.stTranLog.ucTranType==UPLOAD) ||
            (glProcInfo.stTranLog.ucTranType==VOID) ||
            (glProcInfo.stTranLog.ucTranType==OFFLINE_SEND))
        {
            //build88R: bit 6 - cardholder billing should include the foreign tips
            if( !ChkIfZeroAmt(glProcInfo.stTranLog.szFrnTip))
            {
                memset(szAmtBuf, 0, 13);
                PubAscAdd(glProcInfo.stTranLog.szFrnAmount, glProcInfo.stTranLog.szFrnTip, 12, szAmtBuf);
                sprintf((char *)glSendPack.szFrnAmtPP, "%.*s", LEN_FRN_AMT, szAmtBuf);
            }
            else
                //end build88R
            {
                sprintf((char *)glSendPack.szFrnAmtPP, "%.*s", LEN_FRN_AMT, glProcInfo.stTranLog.szFrnAmount);
            }


            sprintf((char *)glSendPack.szDccRatePP, "%.*s", LEN_DCC_RATE, glProcInfo.stTranLog.szDccRate);
        }
    }

    //bit 37, bit38, bit 13, bit 39
    if( (glProcInfo.stTranLog.ucTranType==OFFLINE_SEND) ||
        (glProcInfo.stTranLog.ucTranType==TC_SEND) )
    {
        sprintf((char *)glSendPack.szLocalTime, "%.6s",  &glProcInfo.stTranLog.szDateTime[8]);
        sprintf((char *)glSendPack.szLocalDate, "%.4s",  &glProcInfo.stTranLog.szDateTime[4]);
        sprintf((char *)glSendPack.szRRN,       "%.12s", glProcInfo.stTranLog.szRRN);
        sprintf((char *)glSendPack.szAuthCode,  "%.6s",  glProcInfo.stTranLog.szAuthCode);
        if(ChkIfShanghaiCB())
        {
            sprintf((char *)glSendPack.szRspCode,  "%.2s",  glProcInfo.stTranLog.szRspCode);
        }
        //squall 2013.12.05 synchronize with P60,TC should send response code
        if((ChkIfBnu()||ChkIfCiti()|| ChkCurAcqName("AMEX_MACAU", TRUE)/*Gillian 20161201*/) && (glProcInfo.stTranLog.ucTranType==TC_SEND))
        {
            sprintf((char *)glSendPack.szRspCode,  "%.2s",  glProcInfo.stTranLog.szRspCode);
        }
    }
    else if( glProcInfo.stTranLog.ucTranType==VOID )
    {
        if(ChkIfAmex())
        {
            memset(szLocalTime, 0, sizeof(szLocalTime));
            PubGetDateTime(szLocalTime);    // Send Local Datetime for DE12 & DE13  - Kenny Ting 20100112
            sprintf((char *)glSendPack.szLocalTime, "%.6s",  &szLocalTime[8]);
            sprintf((char *)glSendPack.szLocalDate, "%.4s",  &szLocalTime[4]);
        }
        else if (!ChkIfICBC() && !ChkIfWLB()&!ChkIfIndirectCupAcq()) //add ChkIfIndirectCupAcq by richard 20161128
        {
            sprintf((char *)glSendPack.szLocalTime, "%.6s",  &glProcInfo.stTranLog.szDateTime[8]);
            sprintf((char *)glSendPack.szLocalDate, "%.4s",  &glProcInfo.stTranLog.szDateTime[4]);
        }//modified end
        sprintf((char *)glSendPack.szRRN,       "%.12s", glProcInfo.stTranLog.szRRN);       // jiale 2006.12.12
        if(!ChkIfShanghaiCB() && !ChkIfICBC() && !ChkIfWLB())//ChkIfWLB()
        {
            sprintf((char *)glSendPack.szAuthCode,  "%.6s",  glProcInfo.stTranLog.szAuthCode);  // jiale for void send 37.38field
        }
    }
    //build88S 1.0C:
    else if((glProcInfo.stTranLog.ucTranType == REFUND) && PPDCC_ChkIfDccAcq())
    {
        sprintf((char *)glSendPack.szRRN,       "%.12s", glProcInfo.stTranLog.szRRN);
    }
    //end build88S 1.0C

    if(ChkIfICBC())
    {
        if ((glProcInfo.stTranLog.uiStatus & TS_ADJ)||(glProcInfo.stTranLog.ucTranType == VOID))
        {
            sprintf((char *)glSendPack.szRRN,       "%.12s", glProcInfo.stTranLog.szRRN);
        }
    }

    if(ChkIfIndirectCupAcq()) //add by richard 20161114,  add field 37,38 for pre-auth txn
    {
        if(glProcInfo.stTranLog.ucTranType==PREAUTH_VOID || glProcInfo.stTranLog.ucTranType==PERAUTH_COMP_VOID) 
        {
            sprintf((char *)glSendPack.szRRN,       "%.12s", glProcInfo.stTranLog.szRRN);
        }

        if(glProcInfo.stTranLog.ucTranType==PREAUTH_COMP || glProcInfo.stTranLog.ucTranType==PERAUTH_COMP_VOID)
        {
            sprintf((char *)glSendPack.szAuthCode,  "%.6s",  glProcInfo.stTranLog.szAuthCode);
        }
    }

    // PAN/track 1/2/3/expiry etc
    //bit 2(manual), bit 35, bit 14
    if(ChkIfBnu() || ChkCurAcqName("AMEX_MACAU", TRUE)/*Gillian 20161201*/)
    {
        if((glProcInfo.stTranLog.ucTranType == VOID) || (glProcInfo.stTranLog.ucTranType == UPLOAD))
        {
            sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      glProcInfo.stTranLog.szPan);
        }
    }

    if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT || glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS)
    {
        if(ChkIfBnu())
        {
            sprintf((char *)glSendPack.szTrack1,  "%.*s", LEN_TRACK1,   glProcInfo.szTrack1);
        }

        if( (glProcInfo.szTrack2[0]==0) ||
            (glProcInfo.stTranLog.ucTranType==OFFLINE_SEND) ||
            (glProcInfo.stTranLog.ucTranType==TC_SEND) )
        {

            sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      glProcInfo.stTranLog.szPan);
            sprintf((char *)glSendPack.szExpDate, "%.*s", LEN_EXP_DATE, glProcInfo.stTranLog.szExpDate);
        }
        else
        {
            if (glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS)
            {
                PubBcd2Asc0(glProcInfo.szTrack2, glProcInfo.ucTrack2Len, glSendPack.szTrack2);
                PubTrimTailChars(glSendPack.szTrack2, 'F'); // erase padded 'F' chars
                for(i=0; glSendPack.szTrack2[i]!='\0'; i++)     // convert 'D' to '='
                {
                    if( glSendPack.szTrack2[i]=='D' )
                    {
                        glSendPack.szTrack2[i] = '=';
                        break;
                    }
                }
            }
            else if(!ChkIfIndirectCupAcq()) //modified by richard 20161115
            {
                sprintf((char *)glSendPack.szTrack2,  "%.*s", LEN_TRACK2,   glProcInfo.szTrack2);
            }
        }

        if( ChkIfCiti() || ChkIfDahOrBCM() || ChkIfICBC_MACAU()||ChkIfBnu() //add bnu mandy
            ||ChkIfIndirectCupAcq()) //add indirectCup by richard 20160221
        {
            if( glProcInfo.stTranLog.ucTranType!=SETTLEMENT )
            {
                if( glProcInfo.stTranLog.bPanSeqOK )
                {
                    sprintf((char *)glSendPack.szPanSeqNo, "%0*X", LEN_PAN_SEQ_NO, glProcInfo.stTranLog.ucPanSeqNo);
                }
            }
        }
    }
    else if( (glProcInfo.stTranLog.uiEntryMode & MODE_SWIPE_INPUT) ||
             (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) )
    {
        if( glProcInfo.stTranLog.ucTranType==OFFLINE_SEND || glProcInfo.stTranLog.ucTranType==VOID )
        {
            sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      glProcInfo.stTranLog.szPan);
            sprintf((char *)glSendPack.szExpDate, "%.*s", LEN_EXP_DATE, glProcInfo.stTranLog.szExpDate);
        }
        else
        {
            if ( PPDCC_ChkIfDccAcq() || ChkIfICBC_MACAU()
                 || ChkIfShanghaiCB() || ChkIfICBC() || ChkIfWLB() ||  ChkIfDahOrBCM()) 
            {
                sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, glProcInfo.szTrack2);
            }
            else
            {
                sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, glProcInfo.szTrack2);
                if ( glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE )
                {
                    //Build CITI not need track3
                    if((!ChkIfBnu()) && (!ChkIfCiti())&&(!ChkIfBea())&&(!ChkIfFubon()) //build117 //2014-12-10 add Fubon
                    && !ChkCurAcqName("AMEX_MACAU", TRUE)) //add by richard  20161214
                    {
                        sprintf((char *)glSendPack.szTrack3, "%.*s", LEN_TRACK3, glProcInfo.szTrack3);
                    }
                }

                if(ChkIfIndirectCupAcq()) //add by richard 20161111
                {
                    sprintf((char *)glSendPack.szTrack3, "%.*s", LEN_TRACK3, glProcInfo.szTrack3);
                }
                //  if ( (!ChkIfBoc()) && (!ChkIfBnu()) && (!ChkIfCiti()))//added chkifciti() by laurence
                if ( (!ChkIfBoc()) && (!ChkIfBnu()) && (!ChkIfCiti()) && (!ChkIfBea())  &&(!ChkIfFubon()) ) //squall build117 //2014-12-10 add Fubon
                {
                    if (ChkIfAmex())
                    {
                        if ( ChkIssuerOption(ISSUER_EN_TRACK1))
                        {
                            sprintf((char *)glSendPack.szTrack1, "%.*s", LEN_TRACK1, glProcInfo.szTrack1);
                        }
                    }
                    else
                        sprintf((char *)glSendPack.szTrack1, "%.*s", LEN_TRACK1, glProcInfo.szTrack1);
                }
            }
        }
    }
    else if( (glProcInfo.stTranLog.uiEntryMode & MODE_MANUAL_INPUT) ||
             (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
    {
        sprintf((char *)glSendPack.szPan,     "%.*s", LEN_PAN,      glProcInfo.stTranLog.szPan);
        sprintf((char *)glSendPack.szExpDate, "%.*s", LEN_EXP_DATE, glProcInfo.stTranLog.szExpDate);
    }

    SetEntryMode(&glProcInfo.stTranLog);        // bit 22, entry mode
    SetCondCode();      // bit 25, service condition code

    // bit 48 or 55, CVV2 or 4DBC
    if( ChkIfNeedSecurityCode() && (!(ChkIfAmex() && (glProcInfo.stTranLog.ucTranType==INSTALMENT))) )  
    {
        memcpy(glSendPack.sField48, "\x00\x03", 2);
        sprintf((char *)&glSendPack.sField48[2], "%-3.3s", glProcInfo.szSecurityCode);
	}

	// bit 48  Gillian 20161031
    if(((glProcInfo.stTranLog.ucTranType==INSTALMENT) && ChkCurAcqName("AMEX_INST", FALSE)) || (INSTAL_VOID == 1))//&& (glProcInfo.stTranLog.ucTranType = INSTALMENT))  //Gillian 20161019
	{
		INSTAL_VOID = 0;
		memcpy(glSendPack.sField48, "\x00\x02", 2);
		PubLong2Bcd((unsigned long)glSysParam.stEdcInfo.szEppType, 1, psTemp);
		memcpy(&glSendPack.sField48[2], psTemp, 1);
		memset(psTemp, 0 , sizeof(psTemp));
		PubLong2Bcd((unsigned long)glProcInfo.stTranLog.ucInstalment, 1, psTemp);
		memcpy(&glSendPack.sField48[3], psTemp, 1);
		memcpy(glTemp, psTemp, 1);
	}

	
    if( glProcInfo.stTranLog.uiEntryMode & MODE_PIN_INPUT )
    {
        PubLong2Char((ulong)LEN_PIN_DATA, 2, glSendPack.sPINData);
        memcpy(&glSendPack.sPINData[2], glProcInfo.sPinBlock, LEN_PIN_DATA);
    }

    SetInstAndAirTicketData();  // bit 48 and 63

    // bit 49
    // PP-DCC
    if (PPDCC_ChkIfDccAcq())
    {
        if ((glProcInfo.stTranLog.ucTranType!=LOAD_CARD_BIN) &&
            (glProcInfo.stTranLog.ucTranType!=LOAD_RATE_REPORT) &&
            (glProcInfo.stTranLog.ucTranType!=LOAD_PARA) &&
            (glProcInfo.stTranLog.ucTranType!=SETTLEMENT) &&
            (glProcInfo.stTranLog.ucTranType!=ECHO_TEST))
        {
            PubBcd2Asc0(glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, 2, glSendPack.szTranCurcyCode);
            memmove(glSendPack.szTranCurcyCode, glSendPack.szTranCurcyCode+1, strlen(glSendPack.szTranCurcyCode)+1);
        }
    }

    // bit 51
    // PP-DCC
    if (PPDCC_ChkIfDccAcq())
    {
        if ((glProcInfo.stTranLog.ucTranType==UPLOAD) ||
            (glProcInfo.stTranLog.ucTranType==VOID) ||
            (glProcInfo.stTranLog.ucTranType==OFFLINE_SEND))
        {
            PubBcd2Asc0(glProcInfo.stTranLog.stHolderCurrency.sCurrencyCode, 2, szBuff);
            sprintf((char *)glSendPack.szHolderCurcyCode, "%.*s", LEN_CURCY_CODE, szBuff+1);
        }
    }

    // process bit 55,56
    if( (glProcInfo.stTranLog.ucTranType==AUTH) || (glProcInfo.stTranLog.ucTranType==PREAUTH) ||
        (glProcInfo.stTranLog.ucTranType==SALE) || ((glProcInfo.stTranLog.ucTranType==OFF_SALE) && ChkIfICBC()) || // OFF_SALE added for HK_ICBC by Arthur, 2010.12.03
        (glProcInfo.stTranLog.ucTranType==CASH) || (glProcInfo.stTranLog.ucTranType==INSTALMENT) ||
        (ChkIfBnu() && (glProcInfo.stTranLog.ucTranType==OFFLINE_SEND) && (glProcInfo.stTranLog.ucOrgTranType==SALE)) ||
        (PPDCC_ChkIfDccAcq() && (glProcInfo.stTranLog.ucTranType==OFFLINE_SEND) && (glProcInfo.stTranLog.ucOrgTranType==SALE)) ||
        ((glProcInfo.stTranLog.ucTranType==REVERSAL) && ChkIfICBC()) //REVERSAL added for HK_ICBC by Arthur, 2010.12.03
        || (glProcInfo.stTranLog.ucTranType == OFFLINE_SEND && ChkIfWLB()) ) //bit55 ChkIfWLB()
    {
        if( ChkIfAmex() && ChkIfNeedSecurityCode() && (glProcInfo.szSecurityCode[0]!=0) )
        {
            memcpy(glSendPack.sICCData, "\x00\x04", 2);
            sprintf((char *)&glSendPack.sICCData[2], "%-4.4s", glProcInfo.szSecurityCode);
        }
#ifdef ENABLE_EMV
        else if( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
                 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
        {
            if (ChkIfDahOrBCM() || ChkIfShanghaiCB() || ChkIfICBC() || ChkIfWLB() //whether ChkIfICBC() should be added in fallback or not is still a question, Arthur 20101129
                || ChkCurAcqName("AMEX_MACAU", TRUE)) //add by richard 20161214
            {
				;
            }
            else
            {
                if(!ChkIfBnu() && !ChkIfAmex())//2014-7-10 AMEX also not send DE 55 if 4DBC is off
                {
                    SetDE55(DE55_SALE, &glSendPack.sICCData[2], &iLength);
                    PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
                    memcpy(glProcInfo.stTranLog.sIccData, &glSendPack.sICCData[2], iLength);
                    glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
                }

            }
        }
        else if(ChkIfWLB())//bit55 ChkIfWLB()
        {
            if ( glProcInfo.stTranLog.uiStatus != (TS_ADJ | TS_NOSEND) )
            {
                SetDE55(DE55_SALE, &glSendPack.sICCData[2], &iLength);
                PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
                memcpy(glProcInfo.stTranLog.sIccData, &glSendPack.sICCData[2], iLength);
                glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
            }
        }
        if (ChkIfICBC())
        {
            if(!(glProcInfo.stTranLog.uiStatus & TS_ADJ))
            {
                SetDE55(DE55_SALE, &glSendPack.sICCData[2], &iLength);
                PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
                memcpy(glProcInfo.stTranLog.sIccData, &glSendPack.sICCData[2], iLength);
                glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
            }
            if( (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) &&
                ChkIfAcqNeedDE56() )
            {
                SetDE56(glProcInfo.stTranLog.sField56, &iLength);
                glProcInfo.stTranLog.uiField56Len = (ushort)iLength;
            }
        }
        //added end
        if (PPDCC_ChkIfDccAcq())
        {
            if (glProcInfo.stTranLog.ucTranType==OFFLINE_SEND)
            {
                PubLong2Char((ulong)glProcInfo.stTranLog.uiIccDataLen, 2, glSendPack.sICCData);
                memcpy(&glSendPack.sICCData[2], glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen);
            }
        }

#endif
    }
    else if (glProcInfo.stTranLog.ucTranType==VOID)
    {
        if (PPDCC_ChkIfDccAcq())
        {
            PubLong2Char((ulong)glProcInfo.stTranLog.uiIccDataLen, 2, glSendPack.sICCData);
            memcpy(&glSendPack.sICCData[2], glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen);
        }
    }

	//2015-9-22 DE55 for offline contactless upload
	if((glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS) && glProcInfo.stTranLog.ucTranType==OFFLINE_SEND
		&& glProcInfo.stTranLog.uiStatus & TS_NOSEND)
	{
		PubLong2Char((ulong)glProcInfo.stTranLog.uiIccDataLen, 2, glSendPack.sICCData);
		memcpy(&glSendPack.sICCData[2], glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen);
	}
#ifdef ENABLE_EMV
    if(ChkIfIndirectCupAcq() && (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT)) //add by richard 20161111, set field 56 for DSB
    {
        SetDE56(glProcInfo.stTranLog.sField56, &iLength);
        glProcInfo.stTranLog.uiField56Len = (ushort)iLength;
        PubLong2Char((ulong)glProcInfo.stTranLog.uiField56Len, 2, glSendPack.sICCData2);
        memcpy(&glSendPack.sICCData2[2], glProcInfo.stTranLog.sField56, glProcInfo.stTranLog.uiField56Len);
    }
    if( glProcInfo.stTranLog.ucTranType==VOID && ChkIfFubon() )
    {
        PubLong2Char((ulong)glProcInfo.stTranLog.uiIccDataLen, 2, glSendPack.sICCData);
        memcpy(&glSendPack.sICCData[2], glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen);
        PubLong2Char((ulong)glProcInfo.stTranLog.uiField56Len, 2, glSendPack.sICCData2);
        memcpy(&glSendPack.sICCData2[2], glProcInfo.stTranLog.sField56, glProcInfo.stTranLog.uiField56Len);
    }
    if( glProcInfo.stTranLog.ucTranType==VOID && ChkIfHSBC() &&
        (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) )
    {
        PubLong2Char((ulong)glProcInfo.stTranLog.uiField56Len, 2, glSendPack.sICCData2);
        memcpy(&glSendPack.sICCData2[2], glProcInfo.stTranLog.sField56, glProcInfo.stTranLog.uiField56Len);
    }
    if( ChkIfBea() || ChkIfScb() )
    {
        if( (glProcInfo.stTranLog.ucTranType==VOID) ||
            (glProcInfo.stTranLog.ucTranType==OFFLINE_SEND) ||
            (glProcInfo.stTranLog.ucTranType==TC_SEND) )
        {
            PubLong2Char((ulong)glProcInfo.stTranLog.uiIccDataLen, 2, glSendPack.sICCData);
            memcpy(&glSendPack.sICCData[2], glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen);
        }
    }
    if (ChkIfShanghaiCB() && !(glProcInfo.stTranLog.uiStatus & (TS_ADJ|TS_NOSEND)))
    {
        if ((glProcInfo.stTranLog.ucTranType==OFFLINE_SEND)||
            (glProcInfo.stTranLog.ucTranType==UPLOAD)||
            (glProcInfo.stTranLog.ucTranType==REVERSAL))
        {
            PubLong2Char((ulong)glProcInfo.stTranLog.uiIccDataLen, 2, glSendPack.sICCData);
            memcpy(&glSendPack.sICCData[2], glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen);
        }
        if (glProcInfo.stTranLog.ucTranType==VOID)
        {
            sprintf((char *)glSendPack.szField60, "%.12s", glProcInfo.stTranLog.szOrgAmount);
        }
    }

#endif

    //bit 60, Be used for TC Upload Message  wjr
    if((ChkIfBnu()|| ChkCurAcqName("AMEX_MACAU", TRUE)/*Gillian 20161201*/) && (glProcInfo.stTranLog.ucTranType == TC_SEND))
    {
        sprintf((char *)glSendPack.szField60,    "%.*s%06lu", LEN_MSG_CODE,
                glTranConfig[glProcInfo.stTranLog.ucOrgTranType].szTxMsgID, glProcInfo.stTranLog.ulOrgSTAN);
    }
    if(ChkIfWLB() && glProcInfo.stTranLog.ucTranType == VOID && !ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount))//bit60 ChkIfWLB()
    {
        sprintf((char *)glSendPack.szField60, "%.12s", szTotalAmt);
    }

    if(ChkIfWLB() && glProcInfo.stTranLog.ucTranType == INSTALMENT && !ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount))//bit60 ChkIfWLB()
    {
        sprintf((char *)glSendPack.szField60, "%.12s", szTotalAmt);
    }
    if (ChkIfAmex()||ChkIfDiners())
    {

        ucMaxNum = (uchar)MIN(MAX_GET_DESC, glProcInfo.stTranLog.ucDescTotal);
        if (ucMaxNum>0&&ucMaxNum<=4)
        {
            memset(glSendPack.szField61,' ',8);
            for(ucCnt=0; ucCnt<ucMaxNum; ucCnt++)
            {
                //  ()
                //PubBcd2Asc(&glProcInfo.stTranLog.szDescriptor[ucCnt],1,glSendPack.szField61);
                //  memcpy(glSendPack.szField61+2, glProcInfo.stTranLog.szDescriptor[ucCnt],2);
				
				//2014-9-16 Lois revised field 61 value for AMEX
				ucTemp = glProcInfo.stTranLog.szDescriptor[ucCnt] - '0';				  
				PubASSERT( ucTemp<MAX_DESCRIPTOR );
				memcpy(glSendPack.szField61+2*ucCnt, glSysParam.stDescList[ucTemp].szCode/*.szText*/,2);
            }
        }
    }
    PubDebugTx("FILE_%s,func:%s-LineNo:%d-ChkIfIndirectCupAcq:%d\n,TranType=%d",__FILE__,__FUNCTION__, __LINE__, ChkIfIndirectCupAcq(),glProcInfo.stTranLog.ucTranType);

    //field61, add by richard 20161111, request for DAHSING BANK
    if(ChkIfCupDsb()&& (glProcInfo.stTranLog.ucTranType==REFUND || glProcInfo.stTranLog.ucTranType==PREAUTH_COMP))
    {
        DAHSING_SetField61(&glSendPack);
    }
    
    // bit 62, ROC/SOC
    if ((PPDCC_ChkIfDccAcq() && (glProcInfo.stTranLog.ucTranType==SETTLEMENT))
        || ChkIfIndirectCupAcq()) // PP-DCC  add Dah by richard 20161108, Dah no field 62
    {
 		;
    }
    else
    {
        PackInvoice(&glSendPack, glProcInfo.stTranLog.ulInvoiceNo);
    }

    // bit 63, PP DCC
    //  if(ChkIfBnu())
    //  {
    //      if (glProcInfo.stTranLog.ucTranType == SETTLEMENT)
    //      {
    //          PubAscAdd(glTransTotal.uiSaleCnt, glTransTotal.uiTipCnt, 12, szTotalAmt);
    //          sprintf((char *)glSendPack.sField63, "%d%.*s", LEN_TRAN_AMT, LEN_TRAN_AMT, szTotalAmt);
    //      }
    //  }

    // Dasing Instalment add field63
    if (glProcInfo.stTranLog.ucTranType==INSTALMENT
        ||glProcInfo.stTranLog.ucTranType==TC_SEND)
    {
        if (ChkCurAcqName("DSB",FALSE))
        {
            DAHSING_setInstalmentField63(&glSendPack,&glProcInfo.stTranLog);
        }
    }
    else if(ChkIfIndirectCupAcq()) //add by richard 20161108, For indirect CUP Dah
    {
        DAHSING_CUPField63(&glSendPack);
    }
    
    if (PPDCC_ChkIfDccAcq())
    {
        PPDCC_PackField63(glProcInfo.stTranLog.ucTranType, &glSendPack);
    }

    if ( (ChkIfShanghaiCB() && !(glProcInfo.stTranLog.uiStatus & (TS_ADJ|TS_NOSEND)))
         ||(ChkIfICBC()&&(!(glProcInfo.stTranLog.uiStatus & TS_ADJ)))
         ||( ChkIfWLB() && !(glProcInfo.stTranLog.uiStatus & TS_ADJ) ))
    {
        SetEMVTermIndic(&glProcInfo.stTranLog);
    }


    if( ChkIfNeedMac() )
    {
        PubLong2Char((ulong)LEN_MAC, 2, glSendPack.sMac);
    }
}

// Bit 3 definition:
// AMEX 0200:
// For a sale transaction                                   00 40 0x
// For a refund transaction                                 20 40 0x
// For an on line void of on line sale                      02 40 0x
// For an on line void of on line refund                    22 40 0x
// For an on line void of off line sale seen by host        02 40 0x
// For an on line void of off line sale not seen by host    00 40 0x
//
// AMEX 0220:
// For a sale transaction (referred/sale comp or off line sale)
//                                                  00 40 0x
// For an sale adjustment (i.e. add tip or void of sale):
//      When original sale not seen by host         00 40 0x
//      When original sale seen by host             02 40 0x
// For an off line refund transaction               20 40 0x
// For a void refund transaction:
//      When Trickle fed refund to host             22 40 0x
//      When Void off line refund not trickle fed   20 40 0x
void ModifyProcessCode(void)
{
    if( ChkIfAmex() )
    {
        glSendPack.szProcCode[2] = '4';
        if( glProcInfo.stTranLog.ucTranType==OFFLINE_SEND )
        {
            if( glProcInfo.stTranLog.ucOrgTranType==SALE     ||
                glProcInfo.stTranLog.ucOrgTranType==OFF_SALE ||
                glProcInfo.stTranLog.ucOrgTranType==SALE_COMP )
            {
                if( glProcInfo.stTranLog.uiStatus & (TS_ADJ|TS_VOID) )
                {
                    if( glProcInfo.stTranLog.szRRN[0]!=0 )
                    {
                        glSendPack.szProcCode[1] = '2';
                    }
                }
            }
            else if( glProcInfo.stTranLog.ucOrgTranType==REFUND )
            {
                glSendPack.szProcCode[0] = '2';
                if( glProcInfo.stTranLog.uiStatus & TS_VOID )
                {
                    if( glProcInfo.stTranLog.ucTranType!=SETTLEMENT )
                    {                        // trickle feed
                        glSendPack.szProcCode[1] = '2';
                    }
                }
            }
        }
        else if( glProcInfo.stTranLog.ucTranType==VOID )
        {
            if( glProcInfo.stTranLog.ucOrgTranType==SALE )
            {
                glSendPack.szProcCode[1] = '2';
            }
            else if( glProcInfo.stTranLog.ucOrgTranType==REFUND )
            {
                glSendPack.szProcCode[0] = '2';
                glSendPack.szProcCode[1] = '2';
            }
            else if( glProcInfo.stTranLog.ucOrgTranType==OFF_SALE ||
                     glProcInfo.stTranLog.ucOrgTranType==SALE_COMP )
            {
                if( glProcInfo.stTranLog.szRRN[0]!=0 )
                {
                    glSendPack.szProcCode[1] = '2';
                }
            }
        }
    }
    else
    {
        if( glProcInfo.stTranLog.ucTranType==OFFLINE_SEND )
        {
            if( glProcInfo.stTranLog.ucOrgTranType==SALE     ||
                glProcInfo.stTranLog.ucOrgTranType==OFF_SALE ||
                glProcInfo.stTranLog.ucOrgTranType==SALE_COMP )
            {
                if( glProcInfo.stTranLog.uiStatus & (TS_ADJ|TS_VOID) )
                {
                    if( glProcInfo.stTranLog.szRRN[0]!=0 )
                    {
                        glSendPack.szProcCode[1] = '2';
                    }
                }
            }
            else if( glProcInfo.stTranLog.ucOrgTranType==REFUND )
            {
                glSendPack.szProcCode[0] = '2';
                if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
                {
                    if( glProcInfo.stTranLog.szRRN[0]!=0 )
                    {
                        glSendPack.szProcCode[1] = '2';
                    }
                }
            }
        }
        else if( glProcInfo.stTranLog.ucTranType==VOID )
        {
            if( glProcInfo.stTranLog.ucOrgTranType==SALE )
            {
                glSendPack.szProcCode[1] = '2';
            }
            else if( glProcInfo.stTranLog.ucOrgTranType==REFUND )
            {
                glSendPack.szProcCode[0] = '2';
                glSendPack.szProcCode[1] = '2';
            }
        }
    }

    PubStrUpper(glSendPack.szProcCode);
}

static int DAHSING_setInstalmentField63(void *psSendPack,TRAN_LOG *stTranlog)
{

    uchar   *ptr;
    int     iLen;
    uchar   tmp[10];
    STISO8583 *pstPack = (STISO8583 *)psSendPack;


    memset(tmp,0,sizeof(tmp));
    ptr = pstPack->sField63;
    memcpy(ptr,"\x00\x1B",2);
    ptr += 2;
    memset(ptr,'\x20',16);
    ptr += 16;
    memcpy(ptr,"\x00\x04",2);
    ptr += 2;
    memcpy(ptr,"\x32\x30",2);
    ptr += 2;
    memcpy(ptr,glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode,2);
    ptr += 2;
    memcpy(ptr,"\x00\x04",2);
    ptr += 2;
    memcpy(ptr,"\x32\x33",2);
    ptr += 2;
    sprintf(tmp,"%02d",stTranlog->ucInstalment);
//      glProcInfo.stTranLog.ucInstalment);
    PubAsc2Bcd(tmp,2,ptr);
    //*ptr = (int)(glProcInfo.stTranLog.ucInstalment);

    return 0;
}

static int DAHSING_CUPField63(void *psSendPack)
{
    uchar   *ptr=NULL;
    STISO8583 *pstPack = (STISO8583 *)psSendPack;

    ptr = pstPack->sField63;
    
    memcpy(ptr, "\x00\x17", 2);   //TODO, the message total length is not 21?
    memcpy(ptr+2, "\x02\x10", 2); 
    memcpy(ptr+4, "                ", 16);
    memcpy(ptr+20, "\x00\x30", 2);
    memcpy(ptr+22, "24", 2);  //Terminal entry capability detail
    memcpy(ptr+24, "9", 1);   //"9" = CUP card transaction

    return 0;
}

static int DAHSING_SetField61(void *psSendPack)
{
    STISO8583 *pstPack = (STISO8583 *)psSendPack;

    memcpy(pstPack->szField61, "                     ", 21);
    return 0;
}

void SetEntryMode(TRAN_LOG *pstLog)
{
    sprintf((char *)glSendPack.szEntryMode, "0000");

    if( ChkIfAmex() )
    {
        if( pstLog->uiEntryMode & MODE_PIN_INPUT )
        {
            sprintf((char *)glSendPack.szEntryMode, "0001");
        }
        else
        {
            sprintf((char *)glSendPack.szEntryMode, "0002");
        }
        SetAmexEntryMode(pstLog);
    }
    else if(ChkIfIndirectCupAcq()) //add by richard 20161110, for indirect CUP entry mode setting 
    {
        SetIndirectCUPEntryMode(pstLog);
    }
    else 
    {
        SetStdEntryMode(pstLog);
    }
}

//add by richard 20161110, setting indirect-CUP bank entry mode
void SetIndirectCUPEntryMode(TRAN_LOG *pstLog)
{
    if(pstLog->uiEntryMode & MODE_FALLBACK_SWIPE)
    {
        glSendPack.szEntryMode[0] = '9'; 
    }
    else if(pstLog->uiEntryMode & MODE_MANUAL_INPUT)
    {
        glSendPack.szEntryMode[1] = '1'; 
    }
    else if(pstLog->uiEntryMode & MODE_SWIPE_INPUT)
    {
        glSendPack.szEntryMode[1] = '2'; 
    }
    else if(pstLog->uiEntryMode & MODE_CHIP_INPUT)
    {
        glSendPack.szEntryMode[1] = '5'; 
    }
    else if((ChkIfCupWlb || ChkIfCupChb) && (pstLog->uiEntryMode & MODE_CONTACTLESS))  //different from DSB
    {
        glSendPack.szEntryMode[1] = '7'; 
    }

    if(glProcInfo.stTranLog.uiEntryMode & MODE_PIN_INPUT)
    {
        glSendPack.szEntryMode[2] = '1'; 
    }
    else 
    {
        glSendPack.szEntryMode[2] = '2'; 
    }
    
}

void SetAmexEntryMode(TRAN_LOG *pstLog)
{
    glSendPack.szEntryMode[3] = '2';

    if (ChkIfPinReqdAllIssuer())
    {
        glSendPack.szEntryMode[3] = '1';    // pin capable
    }

#ifdef ENABLE_EMV
    if( ChkAcqOption(ACQ_EMV_FEATURE) )
    {
        EMVGetParameter(&glEmvParam);
        if (glEmvParam.Capability[1] & 0x40)
        {
            glSendPack.szEntryMode[3] = '1';    // pin capable
        }
        if (glEmvParam.Capability[1] & 0x90)
        {
            glSendPack.szEntryMode[3] = '3';    // offline pin capable
        }
    }
#endif

#ifdef ENABLE_EMV
    if( ChkAcqOption(ACQ_EMV_FEATURE) )
    {
        glSendPack.szEntryMode[1] = '5';
    }
#endif


    if( pstLog->uiEntryMode & MODE_SWIPE_INPUT )
    {
	
        if (pstLog->uiEntryMode & MODE_SECURITYCODE) //Gillian 20161107
        {
            glSendPack.szEntryMode[2] = '6'; //SWIPE
        }
        else
        {
            glSendPack.szEntryMode[2] = '2';
        }
    }
#ifdef ENABLE_EMV
    else if( pstLog->uiEntryMode & MODE_CHIP_INPUT )
    {
        glSendPack.szEntryMode[2] = '5';
    }
    else if( pstLog->uiEntryMode & MODE_FALLBACK_SWIPE )
    {
        glSendPack.szEntryMode[1] = '6';
        glSendPack.szEntryMode[2] = (glProcInfo.szSecurityCode[0]!=0) ? '6' : '2';
	}
    else if( pstLog->uiEntryMode & MODE_FALLBACK_MANUAL )
    {
        // ????;
    }
#endif
    else if( pstLog->uiEntryMode & MODE_MANUAL_INPUT )
    {
        glSendPack.szEntryMode[2] = (glProcInfo.szSecurityCode[0]!=0) ? '7' : '1';
    }
	else if( pstLog->uiEntryMode & MODE_CONTACTLESS )//2016-3-31
    {
        glSendPack.szEntryMode[2] = '9';
    }
}

void SetStdEntryMode(TRAN_LOG *pstLog)
{
    //------------------------------------------------------------------------------
    // Entry mode digit 1
    if( ChkAcqOption(ACQ_EMV_FEATURE) )
    {
        if( ChkIfBoc() || ChkIfBea() )
        {
            glSendPack.szEntryMode[0] = '5';
        }
    }

    //------------------------------------------------------------------------------
    // Entry mode digit 2 and digit 3
    if( pstLog->uiEntryMode & MODE_MANUAL_INPUT )
    {
        memcpy(&glSendPack.szEntryMode[1], "01", 2);
    }
    else if( pstLog->uiEntryMode & MODE_CONTACTLESS )
    {
        //      if (ChkIfBeaHalf())//build 132 squall 2013.04.22
        //      {
        //
        //      }

        memcpy(&glSendPack.szEntryMode[1], "07", 2);
        if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MSTR)
        {
            memcpy(&glSendPack.szEntryMode[1], "91", 2);
        }
    }
    else if( pstLog->uiEntryMode & MODE_SWIPE_INPUT )
    {
        memcpy(&glSendPack.szEntryMode[1], "02", 2);
        if (PPDCC_ChkIfDccAcq())
        {
            memcpy(&glSendPack.szEntryMode[1], "90", 2);
        }
        if (ChkIfDahOrBCM() && ChkAcqOption(ACQ_EMV_FEATURE))
        {
            memcpy(&glSendPack.szEntryMode[1], "90", 2);
        }
        if (ChkIfBea())
        {
            memcpy(&glSendPack.szEntryMode[1], "90", 2);
        }
        if (ChkIfCiti())//squall 2012.12.09,synchronized with P60
        {
            memcpy(&glSendPack.szEntryMode[1], "90", 2);
        }
    }
    else if( pstLog->uiEntryMode & MODE_CHIP_INPUT )
    {
        memcpy(&glSendPack.szEntryMode[1], "05", 2);
    }
    else if( pstLog->uiEntryMode & MODE_FALLBACK_SWIPE )
    {
        memcpy(&glSendPack.szEntryMode[1], "80", 2);

        // sort by banks (acquirer)
        if( ChkIfFubon() )
        {
            glSendPack.szEntryMode[2] = '1';
        }
        else if( ChkIfBoc())
        {
            if( pstLog->szPan[0]=='4' )
            {
                memcpy(&glSendPack.szEntryMode[1], "90", 2);
            }
            else if(/* pstLog->szPan[0]=='5'*/ChkIfMC(pstLog->szPan) )//2016-2-15
            {
                memcpy(&glSendPack.szEntryMode[1], "80", 2);
            }
            else if ( memcmp(pstLog->szPan, "35", 2)==0 )
            {
                memcpy(&glSendPack.szEntryMode[1], "97", 2);    // "971"
            }
        }
        else if (ChkIfBea())
        {
            if( /* pstLog->szPan[0]=='5'*/ChkIfMC(pstLog->szPan) )//2016-2-15
            {
                memcpy(&glSendPack.szEntryMode[1], "80", 2);
            }
            else if( pstLog->szPan[0]=='4' )
            {
                if (IsChipCardSvcCode(glSendPack.szTrack2))
                {
                    //if (LastRecordIsFallback())//2015-5-22 if BEA ACQ, do not show "92": don't why/who set to "92"
                    //{
                    //    memcpy(&glSendPack.szEntryMode[1], "92", 2);
                    //}
                    //else
                    {
                        memcpy(&glSendPack.szEntryMode[1], "80", 2);//2013.06.13
                    }
                }
                else
                {
                    memcpy(&glSendPack.szEntryMode[1], "90", 2);
                }
            }
            else if ( memcmp(pstLog->szPan, "35", 2)==0 )
            {
                memcpy(&glSendPack.szEntryMode[1], "97", 2);    // "971"
            }
        }
        else if( ChkIfICBC() || ChkIfWLB() )//====================ChkIfWLB();2011-7-28=====================
        {
            if( /* pstLog->szPan[0]=='5'*/ChkIfMC(pstLog->szPan) )//2016-2-15
            {
                memcpy(&glSendPack.szEntryMode[1], "80", 2);
            }
            else if( pstLog->szPan[0]=='4' )
            {
                if(ChkifWLBfalse())
                {
                    memcpy(&glSendPack.szEntryMode[1], "90", 2);//squall 1.00.0128 WLB requirement
                }
                else
                {
                    memcpy(&glSendPack.szEntryMode[1], "95", 2);//No specified description in the requirement,
                    //therefore,i supposed that "unreliable" means fallback.
                }
            }
        }
        else if (ChkifWLBfalse())//wlb part match
        {
            if( pstLog->szPan[0]=='4' )
            {
                memcpy(&glSendPack.szEntryMode[1], "90", 2);//squall 1.00.0128
            }
        }
        else if (ChkIfDahOrBCM()||ChkIfShanghaiCB())
        {
            // Similar with BOC VISA/MC
            if( pstLog->szPan[0]=='4' )
            {
                memcpy(&glSendPack.szEntryMode[1], "90", 2);
            }
            else if( /* pstLog->szPan[0]=='5'*/ChkIfMC(pstLog->szPan) )//2016-2-15
            {
                memcpy(&glSendPack.szEntryMode[1], "80", 2);
            }
        }
    }
    else if( pstLog->uiEntryMode & MODE_FALLBACK_MANUAL )
    {
        if (ChkIfBea() || PPDCC_ChkIfDccAcq()
            || ((ChkIfShanghaiCB()||ChkIfICBC()||ChkIfWLB()) && /* pstLog->szPan[0]=='5'*/ChkIfMC(pstLog->szPan)))//2016-2-15
        {
            memcpy(&glSendPack.szEntryMode[1], "79", 2);
        }
    }
    else
    {
        if(ChkIfShanghaiCB())
        {
            memcpy(&glSendPack.szEntryMode[1],"00",2);
        }
    }

    //------------------------------------------------------------------------------
    // Entry mode digit 4
    if( ChkAcqOption(ACQ_EMV_FEATURE) )
    {
        glSendPack.szEntryMode[3] = '1';    // default : support offline-PIN
        if ( ChkIfBoc() )
        {
            if( ChkIfIccTran(pstLog->uiEntryMode) )
            {
                glSendPack.szEntryMode[3] = '1';
            }
        }
        if ( ChkIfBea() )
        {
            // 0=Unspecified or unknown
            // 1=Terminal has PIN entry capability
            // 2=Terminal does not have PIN entry capability
            // 8=Terminal has PIN entry capability but PIN pad is not currently operative. (M/Chip only)
            // 9=PIN verified by terminal device. (M/Chip only)
            glSendPack.szEntryMode[3] = '1';
            if (ChkIfIccTran(pstLog->uiEntryMode) && (/* pstLog->szPan[0]=='5'*/ChkIfMC(pstLog->szPan) ))//2016-2-15
            {
                // M/Chip  ????  To be done : check offline-PIN entered or not
            }
        }
        if (ChkIfICBC())    //===================ChkIfWLB()  2011-7-28=====================
        {
            glSendPack.szEntryMode[3] = '0';
            if (glProcInfo.stTranLog.uiEntryMode & MODE_PIN_INPUT)
            {
                glSendPack.szEntryMode[3] = '1';
            }
            else
            {
                glSendPack.szEntryMode[3] = '2';
            }
        }
        if (PPDCC_ChkIfDccAcq())
        {
            if (glProcInfo.stTranLog.uiEntryMode & MODE_PIN_INPUT)
            {
                glSendPack.szEntryMode[3] = '9';
            }
        }
        if (ChkIfCiti())
        {
            if( pstLog->uiEntryMode & MODE_CONTACTLESS )
            {
                if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MSTR)
                {
                    glSendPack.szEntryMode[3] = '2';
                }
            }

        }

    }
    else
    {
        //squall 2013.12.10 , synchronized with P60, all citi acq should set last bit into 1;
        if (ChkIfCiti())
        {
            glSendPack.szEntryMode[3] = '1';
        }
        ;//to do for future
    }
}

// set bit 25
void SetCondCode(void)
{
    if( ChkIfAmex() )
    {
        // condition code==06: Preauth, Auth, SaleComplete, sale below floor
        sprintf((char *)glProcInfo.stTranLog.szCondCode, "00");
        if( (glProcInfo.stTranLog.ucTranType==PREAUTH) || (glProcInfo.stTranLog.ucTranType==AUTH) )
        {
            sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");
        }
        if( glProcInfo.stTranLog.ucTranType==OFFLINE_SEND )
        {
            if( !(glProcInfo.stTranLog.uiStatus & (TS_ADJ|TS_VOID)) &&
                (glProcInfo.stTranLog.ucOrgTranType==SALE_COMP || glProcInfo.stTranLog.ucOrgTranType==SALE) )
            {
                sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");
            }
        }
    }
    else
    {
        sprintf((char *)glProcInfo.stTranLog.szCondCode, "00");
        if( glProcInfo.stTranLog.ucTranType==PREAUTH )
        {
            sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");
        }
        else if( glProcInfo.stTranLog.ucTranType==VOID || glProcInfo.stTranLog.ucTranType==OFFLINE_SEND )
        {
            if( glProcInfo.stTranLog.ucOrgTranType==SALE_COMP )
            {
                sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");
            }
        }
    }

    sprintf((char *)glSendPack.szCondCode, "%.2s", glProcInfo.stTranLog.szCondCode);
}

// RFU for HK (bit 48, 63)
void SetInstAndAirTicketData(void)
{
    uchar   sBuff[32];

    if (ChkIfBea())
    {
        if (glProcInfo.stTranLog.ucInstalment!=0)
        {
            memcpy(glSendPack.sField63, "\x00\x02", 2);
            PubLong2Bcd((ulong)glProcInfo.stTranLog.ucInstalment, 1, sBuff);
            PubBcd2Asc0(sBuff, 1, glSendPack.sField63+2);
        }
    }

    if ((glProcInfo.stTranLog.ucInstalment!=0) && (ChkAcqOption(ACQ_CITYBANK_INSTALMENT_FEATURE)))
    {//Build0120 citybank instalment

        memset(glSendPack.sField48,'0',sizeof(glSendPack.sField48));
;
        memcpy(glSendPack.sField48, "\x00\x07", 2);


        glSendPack.sField48[2] = '0';       //tag
        glSendPack.sField48[3] = '1';       //tag
        sprintf((char *)&(glSendPack.sField48[4]), "%03lu", glProcInfo.stTranLog.ucInstalment);
        glSendPack.sField48[7] = '0';
    }

}
//for SHANGHAI commercial bank field 63
void SetEMVTermIndic(TRAN_LOG * pstLog)
{
    if (pstLog->uiEntryMode & MODE_CHIP_INPUT
        || pstLog->uiEntryMode & MODE_FALLBACK_SWIPE
        || pstLog->uiEntryMode & MODE_FALLBACK_MANUAL)
    {
        if ( (pstLog->ucTranType==AUTH)
             || pstLog->ucTranType==SALE
             || (pstLog->ucTranType==OFFLINE_SEND)
             || pstLog->ucTranType== REVERSAL
             ||((pstLog->ucTranType== OFF_SALE)&&ChkIfICBC())
             ||((pstLog->ucTranType== UPLOAD)&&ChkIfICBC())
             || (pstLog->ucTranType== OFF_SALE && ChkIfWLB())
             || (pstLog->ucTranType== PREAUTH && ChkIfWLB())
             || (pstLog->ucTranType== INSTALMENT && ChkIfWLB()) )//bit63 ChkIfWLB()
        {
            glSendPack.sField63[0]=0x00;
            glSendPack.sField63[1]=0x03;
            if(pstLog->szPan[0]=='4')
            {
                glSendPack.sField63[2] = '5';
            }
            else if(/* pstLog->szPan[0]=='5'*/ChkIfMC(pstLog->szPan) )//2016-2-15
            {
                glSendPack.sField63[2] = '8';
            }
            if(pstLog->uiEntryMode & MODE_FALLBACK_SWIPE)
            {
                glSendPack.sField63[3] = '1';
            }
            else
            {
                glSendPack.sField63[3] = '0';
            }
            if (ChkIfWLB())
            {
                glSendPack.sField63[4] = ucWLBBuf[0];
            }
            else
            {
                if(pstLog->uiEntryMode & MODE_FALLBACK_SWIPE)
                {
                    glSendPack.sField63[4] = '2';
                }
                else
                {
                    glSendPack.sField63[4] = '1';
                }
            }
        }
    }
    else if ( pstLog->uiEntryMode & MODE_SWIPE_INPUT
              || pstLog->uiEntryMode & MODE_MANUAL_INPUT)
    {
        if ( pstLog->ucTranType==AUTH
             || pstLog->ucTranType==SALE
             ||(pstLog->ucTranType==OFF_SALE && ChkIfICBC())/*Added by Arthur, 2010.12.07*/
             ||(pstLog->ucTranType==OFFLINE_SEND && ChkIfICBC()) /*Added by Arthur, 2010.12.07*/
             || (pstLog->ucTranType== OFFLINE_SEND && ChkIfWLB())
             || (pstLog->ucTranType== OFF_SALE && ChkIfWLB())
             || (pstLog->ucTranType== PREAUTH && ChkIfWLB())
             || (pstLog->ucTranType== INSTALMENT && ChkIfWLB()))//bit63 ChkIfWLB()
        {
            glSendPack.sField63[0]=0x00;
            glSendPack.sField63[1]=0x01;
            if(pstLog->szPan[0]=='4')
            {
                glSendPack.sField63[2] = '5';
            }
            else if(/* pstLog->szPan[0]=='5'*/ChkIfMC(pstLog->szPan) )//2016-2-15
            {
                glSendPack.sField63[2] = '8';
            }
        }
    }
}



// 交易初始化:检查交易是否允许,显示交易标题
// Init transaction, check allowance, display title.
int TransInit(uchar ucTranType)
{
    ScrCls();
    glProcInfo.stTranLog.ucTranType = ucTranType;
    DispTransName();

    if( !ChkIfTranAllow(ucTranType) )
    {
        return ERR_NO_DISP;
    }

    return 0;
}

int TransCapture(void)
{
    int iRet;

    if( glProcInfo.stTranLog.ucTranType==CASH )
    {
        iRet = TransCashSub();
    }
    else if ( glProcInfo.stTranLog.ucTranType==SALE ||
              glProcInfo.stTranLog.ucTranType==INSTALMENT )
    {
        iRet = TransSaleSub();
    }
    else
    {
        if( ChkEdcOption(EDC_AUTH_PREAUTH) )
        {
            iRet = TransAuthSub(AUTH);
        }
        else
        {
            iRet = TransAuthSub(PREAUTH);
        }
    }

    return iRet;
}

int InstallmentMenu(void)
{
    int     iRet, iMenuNo;
    static  MenuItem stInstMenu[10] =
    {
        {TRUE, _T_NOOP("INSTAL SALE "), NULL},
        {TRUE, _T_NOOP("INSTAL VOID "), NULL},
        {TRUE, "", NULL},
    };


    iMenuNo = PubGetMenu((uchar *)_T("PLS SELECT:"), stInstMenu, MENU_AUTOSNO|MENU_PROMPT, USER_OPER_TIMEOUT);
    switch( iMenuNo )
    {
        case 0:
            iRet = TransSale(TRUE);
            break;

        case 1:
			INSTAL_VOID = 1; //Gillian 20161020
            iRet = TransVoid();
            break;

        default:
            return ERR_NO_DISP;
    }

    CommOnHook(FALSE);
    return iRet;
}

int TransCash(void)
{
    int     iRet;

    iRet = TransInit(CASH);
    if( iRet!=0 )
    {
        return iRet;
    }

    iRet = GetCard(CARD_INSERTED|CARD_SWIPED|CARD_KEYIN);
    if( iRet!=0 )
    {
        return iRet;
    }

    return TransSaleSub();
}

int TransCashSub(void)
{
    int     iRet;

    if( !ChkIfTranAllow(glProcInfo.stTranLog.ucTranType) )
    {
        return ERR_NO_DISP;
    }
    if( !ChkSettle() )
    {
        return ERR_NO_DISP;
    }

    iRet = GetAmount();
    if( iRet!=0 )
    {
        return iRet;
    }

    iRet = GetDescriptor();
    if( iRet!=0 )
    {
        return iRet;
    }

    iRet = GetAddlPrompt();
    if( iRet!=0 )
    {
        return iRet;
    }

    ScrCls();
    DispTransName();
    iRet = GetPIN(FALSE);
    if( iRet!=0 )
    {
        return iRet;
    }

    SetCommReqField();
    iRet = TranProcess();
    if( iRet!=ERR_NEED_FALLBACK )
    {
        return iRet;
    }

    // continue fallback process
    glProcInfo.bIsFallBack = TRUE;
    glProcInfo.stTranLog.uiEntryMode &= 0xF0;
    iRet = GetCard(FALLBACK_SWIPE|CARD_SWIPED);
    if( iRet!=0 )
    {
        return iRet;
    }

    SetCommReqField();
    return TranProcess();
}

// 普通消费、分期消费
int TransSale(uchar ucInstallment)
{
    int     iRet;
    uchar   ucEntryMode;
    ST_EVENT_MSG    stEventMsg;
	uchar			ucBuff[6+1];//2014-11-7
	uchar			szMaxAmt[12+1];//2014-11-7

    iRet = TransInit((uchar)(ucInstallment ? INSTALMENT : SALE));
    if( iRet!=0 )
    {
        return iRet;
    }

	PubLong2Bcd(glSysParam.stEdcInfo.ulClssMaxLmt,6,ucBuff);
	PubBcd2Asc0(ucBuff,6,szMaxAmt);
	szMaxAmt[13]=0;

    //非接触卡开启处理非接流程////獶钡牟秨币矪瞶獶钡瑈祘
    if (glSysParam.stEdcInfo.ucClssFlag == 1 &&
        ((glSysParam.stEdcInfo.ucClssMode == 0)||(glSysParam.stEdcInfo.ucClssMode == 2))
        && ucInstallment == FALSE)          //2014-6-19 if INSTAL SALE, do not use contactless
    {

        iRet = GetAmount();
        if (iRet == 0)
        {
			if( memcmp(glProcInfo.stTranLog.szAmount, szMaxAmt, 12) < 0 )//2014-11-7 add new condition
			{
#if defined(_P60_S1_) || defined(_S_SERIES_)
				SetOffBase(OffBaseDisplay);
#endif
				CLSSPreTrans();
				memset(glAmount, 0, sizeof(glAmount));
				return 0;
			}
			else
			{
				goto CONTACT_PROC;
			}
        }
        else if (iRet == KEYFN || iRet == KEYUP || iRet == KEYDOWN||iRet==KEYMENU)
        {
            memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));
            stEventMsg.MsgType  = KEYBOARD_MSG;
            stEventMsg.KeyValue = iRet;
            event_main(&stEventMsg);
            CommOnHook(FALSE);
            return 0;
        }
        else
        {
            if(iRet !=ERR_CLSS_AMT )//Jason 2014.08.21 11:41  膥尿ユぃ癶
            {
                return iRet;
            }
        }
    }
CONTACT_PROC:
    ucEntryMode = CARD_SWIPED|CARD_KEYIN;
    //  ucEntryMode = CARD_SWIPED|CARD_KEYIN;
    //  if (!ucInstallment)
    //  {
    //      ucEntryMode |= CARD_INSERTED;
    //  }

    if(iRet !=ERR_CLSS_AMT )//Jason 2014.08.21 11:41  ERR_CLSS_AMT ぃノ╃よΑ
    {
        if (glSysParam.stEdcInfo.ucClssFlag == 1)
        {
            ucEntryMode |= CARD_TAPPED;
        }
    }
    if (/*!*/ucInstallment)  //Gillian 20161013
    {
        ucEntryMode |= CARD_INSERTED;
    }
    if (ucInstallment && ChkIfWLB())
    {
        ucEntryMode |= CARD_INSERTED;//build118
    }

    iRet = GetCard(ucEntryMode);
    if( iRet!=0 )
    {
        return iRet;
    }
    return TransSaleSub();
}

// 负责SALE和INSTALLMENT
int TransSaleSub(void)
{
    int     iRet;

    if( !ChkIfTranAllow(glProcInfo.stTranLog.ucTranType) )
    {
        return ERR_NO_DISP;
    }

    // instalment时，仅当选择plan之后才最终确认ACQ，因此现在不需要检查settle状态
    if (glProcInfo.stTranLog.ucTranType!=INSTALMENT)
    {
        if( !ChkSettle() )
        {
            return ERR_NO_DISP;
        }
    }

    iRet = GetAmount();
    if( iRet!=0 )
    {
        return iRet;
    }

    iRet = GetInstalPlan();
    if( iRet!=0 )
    {
        return iRet;
    }

    // instalment时，仅当选择plan之后才最终确认ACQ
    if (glProcInfo.stTranLog.ucTranType==INSTALMENT)
    {
        if( !ChkSettle() )
        {
            return ERR_NO_DISP;
        }
    }

    iRet = GetDescriptor();
    if( iRet!=0 )
    {
        return iRet;
    }

    iRet = GetAddlPrompt();
    if( iRet!=0 )
    {
        return iRet;
    }

    ScrCls();
    DispTransName();
    iRet = GetPIN(FALSE);
    if( iRet!=0 )
    {
        return iRet;
    }

    if (ChkIfDccBOC())
    {
        if (BOCDCC_RateLookup())
        {
            return ERR_NO_DISP;
        }
    }
    else if (PPDCC_ChkIfDccAcq())
    {
        iRet = PPDCC_TransRateEnquiry();
        if (iRet)
        {
            return iRet;
        }
    }

RUN_AGAIN:  //build88S 1.0D
    ScrCls();
    DispTransName();

    if( ChkIfBelowMagFloor() && !glProcInfo.bExpiryError && !ChkIfIccTran(glProcInfo.stTranLog.uiEntryMode) )
    {
        //      sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");
        sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%02ld", glSysCtrl.ulSTAN % 100);
        glProcInfo.stTranLog.uiStatus |= TS_CHANGE_APPV|TS_FLOOR_LIMIT;

        return FinishOffLine();
    }

    SetCommReqField();
    iRet = TranProcess();
    if( iRet!=ERR_NEED_FALLBACK )
    {
        //build88S 1.0D: switch to domestic transaction processing
        if(iRet == ERR_NEED_DOMESTIC)
        {
            goto RUN_AGAIN;
        }
        //end build88S 1.0D
        return iRet;
    }

    // continue fallback process
    glProcInfo.bIsFallBack = TRUE;
    glProcInfo.stTranLog.uiEntryMode &= 0xF0;
    iRet = GetCard(FALLBACK_SWIPE|CARD_SWIPED);
    if( iRet!=0 )
    {
        return iRet;
    }

    SetCommReqField();
    iRet = TranProcess();

    //build88S 1.0D: switch to domestic transaction processing
    if(iRet  == ERR_NEED_DOMESTIC)
        goto RUN_AGAIN;
    else
        return iRet;
    //end build88S 1.0D
}

// 授权/预授权交易
int TransAuth(uchar ucTranType)
{
    int     iRet;
    uchar   ucEntryMode;
    PubASSERT(ucTranType==AUTH || ucTranType==PREAUTH|| ucTranType==PREAUTH_VOID
        || ucTranType==PREAUTH_COMP|| ucTranType==PERAUTH_COMP_VOID);

    iRet = TransInit(ucTranType);
    if( iRet!=0 )
    {
        return iRet;
    }

    if((ucTranType==PREAUTH_VOID|| ucTranType==PREAUTH_COMP|| 
        ucTranType==PERAUTH_COMP_VOID)&& !ChkIfIndirectCupAcq()) //add by richard  20161114 don't support
    {
        return ERR_NO_DISP;
    }

    if(ucTranType==AUTH && ChkIfIndirectCupAcq()) //add by richard 20161114, CUP no AUTH 
    {
        return ERR_NO_DISP;
    }
    
    ucEntryMode = CARD_INSERTED|CARD_SWIPED|CARD_KEYIN;
    if (glSysParam.stEdcInfo.ucClssFlag == 1)
    {
        ucEntryMode |= CARD_TAPPED;
    }
    iRet = GetCard(ucEntryMode);
    if( iRet!=0 )
    {
        return iRet;
    }

    return TransAuthSub(ucTranType);
}

int TransAuthSub(uchar ucTranType)
{
    int     iRet;

    if( !ChkIfTranAllow(ucTranType) )
    {
        return ERR_NO_DISP;
    }
    if( !ChkSettle() )
    {
        return ERR_NO_DISP;
    }

    if( ChkIssuerOption(ISSUER_NO_PREAUTH) )
    {
        DispBlockFunc();
        return ERR_NO_DISP;
    }

    iRet = GetAmount();
    if( iRet!=0 )
    {
        return iRet;
    }

    if(ucTranType==PREAUTH_VOID || ucTranType==PERAUTH_COMP_VOID) //add by richard 20161114 input CUP RRN NO.
    {
        iRet = GetRRN();
        if( iRet!=0 )
        {
            return iRet;
        }
    }

    if(ucTranType==PREAUTH_COMP || ucTranType==PERAUTH_COMP_VOID) //add by  richard 20161114, input AUTH no.
    {
        iRet = GetPreAuthCode();
        if( iRet!=0 )
        {
            return iRet;
        }
    }

    if( !ChkIfAmex() )
    {
        iRet = GetDescriptor();
        if( iRet!=0 )
        {
            return iRet;
        }

        iRet = GetAddlPrompt();
        if( iRet!=0 )
        {
            return iRet;
        }
    }

    ScrCls();
    DispTransName();
    iRet = GetPIN(FALSE);
    if( iRet!=0 )
    {
        return iRet;
    }

    if (PPDCC_ChkIfDccAcq()) // PP-DCC
    {
        iRet = PPDCC_TransRateEnquiry();
        if (iRet)
        {
            return iRet;
        }
    }

RUN_AGAIN: //build88S 1.0D
    ScrCls();
    DispTransName();

    SetCommReqField();
    iRet = TranProcess();
    if( iRet!=ERR_NEED_FALLBACK )
    {
        //build88S 1.0D: switch to domestic transaction processing
        if(iRet == ERR_NEED_DOMESTIC)
            goto RUN_AGAIN;
        //end build88S 1.0D
        return iRet;
    }

    // continue fallback process
    glProcInfo.bIsFallBack = TRUE;
    glProcInfo.stTranLog.uiEntryMode &= 0xF0;
    iRet = GetCard(FALLBACK_SWIPE|CARD_SWIPED);
    if( iRet!=0 )
    {
        return iRet;
    }

    SetCommReqField();
    iRet = TranProcess();

    //build88S 1.0D: switch to domestic transaction processing
    if(iRet == ERR_NEED_DOMESTIC)
        goto RUN_AGAIN;
    else
        return iRet;
    //end build88S 1.0D
}

int FinishOffLine(void)
{
    uchar   ucTranAct, sCurrencyCode[2], szAmount[12+1];
    uchar   ucResult;
	int     iLength;  

#if defined(_P60_S1_) || defined(_S_SERIES_)
    SetOffBase(OffBaseDisplay);
#endif

    ScrCls();
    DispTransName();
    DispProcess();

	/*============BEGIN: Lois  2014-11-5 modify==========*/
    if(ChkIfAmex())
    {
        EMVGetTLVData(0x9F26, glProcInfo.stTranLog.sAppCrypto, &iLength);
        EMVGetTLVData(0x95,   glProcInfo.stTranLog.sTVR,       &iLength);
        EMVGetTLVData(0x9B,   glProcInfo.stTranLog.sTSI,       &iLength);
    }
    /*====================== END======================== */

    if( !(glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) &&
        (glProcInfo.stTranLog.ucTranType!=SALE_COMP) )
    {
        sprintf((char *)glProcInfo.stTranLog.szRspCode, "00");
    }
    glProcInfo.stTranLog.ulInvoiceNo = glSysCtrl.ulInvoiceNo;
    ucTranAct = glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct;

    if (ucTranAct & ACT_INC_TRACE)
    {
        GetNewTraceNo();
    }

    if( ucTranAct & WRT_RECORD )
    {
        glProcInfo.stTranLog.uiStatus |= TS_NOSEND;
        if (ChkIfWLB())
        {
            if (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE )        //for test
            {
                iWLBfd = open("WLBBit63.dat", O_RDWR);
                ucResult = write(iWLBfd, "21", 2);
                ucWLBBuf[0] = 2;
                ucWLBBuf[1] = 1;
                close(iWLBfd);
            }
            else
            {
                iWLBfd = open("WLBBit63.dat", O_RDWR);
                ucResult = write(iWLBfd, "11", 2);
                ucWLBBuf[0] = 1;
                ucWLBBuf[1] = 1;
                close(iWLBfd);
            }
        }

        SaveTranLog(&glProcInfo.stTranLog);
#ifdef AMT_PROC_DEBUG
		glProcInfo.stTranLog.cntAmt = 0;//2014-9-19 ack POS to erase amt records
#endif

        // PP-DCC
        if ((glProcInfo.stTranLog.ucTranType==SALE) ||
            (glProcInfo.stTranLog.ucTranType==OFF_SALE))
        {
            // Amount
            PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szAmount);
            // Currency code
            memcpy(sCurrencyCode, "\x00\x00", 2);
            if (PPDCC_ChkIfDccAcq() ||
                (!PPDCC_ChkIfDccAcq() && (glProcInfo.stTranLog.ucDccType==PPTXN_OPTOUT)))
            {
                memcpy(sCurrencyCode, glProcInfo.stTranLog.stHolderCurrency.sCurrencyCode, 2);
            }
            if (PPDCC_ChkIfDccAcq())
            {
                PPDCC_UpdateTransStatistic(sCurrencyCode, szAmount, '+', NULL, 0);
            }
            else
            {
                PPDCC_UpdateTransStatistic(sCurrencyCode, NULL, 0, szAmount, '+');
            }
        }
    }

    EcrSendTransSucceed();

    if( ucTranAct & PRN_RECEIPT )   // print slip
    {
        CommOnHook(FALSE);
        GetNewInvoiceNo();
        PrintReceipt(PRN_NORMAL);
    }

    // PP-DCC
    if ((ucTranAct & WRT_RECORD) && PPDCC_ChkIfNeedFinalSelect())
    {
        PPDCC_FinalSelect();
    }
    else
    {
        DispResult(0);
        PubWaitKey(glSysParam.stEdcInfo.ucAcceptTimeout);
    }

    return 0;
}

int TranReversal(void)
{
#ifdef ENABLE_EMV
    int iLength;
#endif
    int iRet;
    SYS_PROC_INFO   stProcInfoBak;

    if( glProcInfo.stTranLog.ucTranType==LOGON ||
        glProcInfo.stTranLog.ucTranType==LOAD_PARA ||
        glProcInfo.stTranLog.ucTranType==ECHO_TEST ||
        glProcInfo.stTranLog.ucTranType==LOAD_CARD_BIN ||
        glProcInfo.stTranLog.ucTranType==LOAD_RATE_REPORT )
    {
        return 0;
    }

    if( !glSysCtrl.stRevInfo[glCurAcq.ucIndex].bNeedReversal )
    {
        return 0;
    }

    // backup current process information
    memcpy(&glProcInfo.stSendPack, &glSendPack, sizeof(STISO8583));
    memcpy(&stProcInfoBak, &glProcInfo, sizeof(SYS_PROC_INFO));
    glProcInfo.stTranLog.ucTranType = REVERSAL;

    ScrCls();
    DispTransName();

    memcpy(&glSendPack, &glSysCtrl.stRevInfo[glCurAcq.ucIndex].stRevPack, sizeof(STISO8583));
    sprintf((char *)glSendPack.szMsgCode, "0400");
    if( ChkIfBoc() )  // Boc erase F55
    {
        memset(glSendPack.sICCData, 0, 2);
    }
    if( ChkIfAmex() )
    {
        memset(glSendPack.sICCData, 0, 2);
        memset(glSendPack.szLocalDate, 0, sizeof(glSendPack.szLocalDate));
        memset(glSendPack.szLocalTime, 0, sizeof(glSendPack.szLocalTime));
        memset(glSendPack.szRRN,       0, sizeof(glSendPack.szRRN));
        memset(glSendPack.szAuthCode,  0, sizeof(glSendPack.szAuthCode));
    }
    memset(glSendPack.sPINData, 0, sizeof(glSendPack.sPINData));    // erase PIN block

#ifdef ENABLE_EMV
    if( (glSysCtrl.stRevInfo[glCurAcq.ucIndex].uiEntryMode & MODE_CHIP_INPUT) &&
        ChkIfAcqNeedDE56() )
    {
        iLength = glSysCtrl.stField56[glCurAcq.ucIndex].uiLength;
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
    if( ChkIfDahOrBCM() || ChkIfCiti() ||ChkIfShanghaiCB() ||ChkIfICBC() || ChkIfWLB())
    {
        memset(glSendPack.sICCData2, 0, 2);
    }
    if( (glSysCtrl.stRevInfo[glCurAcq.ucIndex].uiEntryMode & MODE_FALLBACK_SWIPE) ||
        (glSysCtrl.stRevInfo[glCurAcq.ucIndex].uiEntryMode & MODE_FALLBACK_MANUAL) )
    {
        if (ChkIfBoc())
        {
            memset(glSendPack.sICCData2, 0, 2);
        }
    }
#endif

    if( ChkIfBoc() || ChkIfWLB())//bit12/bit13/bit37/bit38
    {
        memset(glSendPack.szLocalDate, 0, sizeof(glSendPack.szLocalDate));
        memset(glSendPack.szLocalTime, 0, sizeof(glSendPack.szLocalTime));
        memset(glSendPack.szRRN, 0, sizeof(glSendPack.szRRN));
        memset(glSendPack.szAuthCode, 0, sizeof(glSendPack.szAuthCode));
    }
    if (ChkIfWLB())
    {
        if (!(glSysCtrl.stRevInfo[glCurAcq.ucIndex].uiEntryMode & MODE_CHIP_INPUT))
        {
            memset(glSendPack.szPan, 0, sizeof(glSendPack.szPan));
            memset(glSendPack.szExpDate, 0, sizeof(glSendPack.szExpDate));
        }
    }

    //Build 1.00.0110
    //  if (ChkIfICBC() || ChkIfWLB()) //bit35 ChkIfWLB()
    //  {
    //      sprintf((char *)glSendPack.szTrack2, "%.*s", LEN_TRACK2, glProcInfo.szTrack2);
    //  }
    while( 1 )
    {
        iRet = SendRecvPacket();
        if( iRet!=0 )
        {
            break;
        }
        if( memcmp(glRecvPack.szRspCode, "00", 2)==0 )
        {
            break;
        }
        if( ChkIfAmex() && (memcmp(glRecvPack.szRspCode, "08", 2)==0 || memcmp(glRecvPack.szRspCode, "88", 2)==0) )
        {
            break;
        }

        sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", glRecvPack.szRspCode);
        DispResult(ERR_HOST_REJ);
        //      iRet = ERR_NO_DISP;
        iRet = ERR_TRAN_FAIL;
        break;
    }
    if( iRet==0 )
    {// clear reversal flag
  		//2015-1-23 for AMEX, record cnt of reversal
		if( ChkIfAmex() )
		{
			glProcInfo.stTranLog.ucReversalFlag = 1;
			stProcInfoBak.stTranLog.ucReversalFlag = glProcInfo.stTranLog.ucReversalFlag;
		}
      SaveRevInfo(FALSE);
    }

    if (iRet==0)
    {
        // increase invoice for coming AMEX transaction
        if (ChkIfAmex())
        {
            if (glTranConfig[stProcInfoBak.stTranLog.ucTranType].ucTranAct & PRN_RECEIPT)
            {
                stProcInfoBak.stTranLog.ulInvoiceNo = GetNewInvoiceNo();
                PackInvoice(&stProcInfoBak.stSendPack, stProcInfoBak.stTranLog.ulInvoiceNo);
            }
        }
    }

    // restore process information
    memcpy(&glProcInfo, &stProcInfoBak, sizeof(SYS_PROC_INFO));
    memcpy(&glSendPack, &glProcInfo.stSendPack, sizeof(STISO8583));

    ScrCls();
    DispTransName();

    return iRet;
}

int GetOfflineTrans(uchar ucTypeFlag)
{
    int     iRet;
    ushort  uiCnt;

    for(uiCnt=0; uiCnt<MAX_TRANLOG; uiCnt++)
    {
        if( glSysCtrl.sAcqKeyList[uiCnt]!=glCurAcq.ucKey )
        {
            continue;
        }

        memset(&glProcInfo.stTranLog, 0, sizeof(TRAN_LOG));
        iRet = LoadTranLog(&glProcInfo.stTranLog, uiCnt);
        if( iRet!=0 )
        {
            return FALSE;
        }

        if ( (ucTypeFlag & OFFSEND_TC) &&
             (glProcInfo.stTranLog.uiStatus & TS_NEED_TC) )
        {
            glProcInfo.uiRecNo = uiCnt;
            return TRUE;
        }
        else if ( (ucTypeFlag & OFFSEND_TRAN) &&
                  (glProcInfo.stTranLog.uiStatus & TS_NOSEND) )
        {
            glProcInfo.uiRecNo = uiCnt;
            return TRUE;
        }
    }

    return FALSE;
}

int OfflineSend(uchar ucTypeFlag)
{
    uchar szFrnAmt[13], szFrnTipAmt[13];  //build88R
    uchar  szBuff[128];
    uchar  szBuff1[128];
#ifdef ENABLE_EMV
    int iLength;
#endif
    int iRet;
    SYS_PROC_INFO   stProcInfoBak;

    if( glProcInfo.stTranLog.ucTranType!=SETTLEMENT )
    {// the time to load 400 txn log is about 1-2 seconds
        if( ChkAcqOption(ACQ_DISABLE_TRICK_FEED) )
        {
            return 0;
        }
    }

    memcpy(&glProcInfo.stSendPack, &glSendPack, sizeof(STISO8583));
    memcpy(&stProcInfoBak, &glProcInfo,  sizeof(SYS_PROC_INFO));
    while( 1 )
    {
        InitTransInfo();
        if( !GetOfflineTrans(ucTypeFlag) )
        {
            iRet = 0;
            break;
        }

        if (glProcInfo.stTranLog.uiStatus & TS_NEED_TC)
        {
            glProcInfo.stTranLog.ucOrgTranType = glProcInfo.stTranLog.ucTranType;
            glProcInfo.stTranLog.ucTranType    = TC_SEND;

            DispTransName();
            SetCommReqField();

            //          2010-6-8 : ICBC MACAU want to use 0320
            if (ChkIfICBC() || ChkIfCiti())//added chifciti() by laurence
            {
                sprintf((char *)glSendPack.szMsgCode, "0220");
            }
            //bit 61
            //          if (ChkIfBea())
            //          {
            //              strcpy(glSendPack.szField60,glProcInfo.stTranLog.szPPDccTxnID);
            //          }
            // bit39
            //sprintf((char *)glSendPack.szRspCode, "%.2s", glProcInfo.stTranLog.szRspCode);

#ifdef ENABLE_EMV
            // bit55
            //SetDE55(DE55_TC, glSendPack.sICCData+2, &iLength);
            SetTCDE55(&glProcInfo.stTranLog, glSendPack.sICCData+2, &iLength);
            PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
#endif

            iRet = SendRecvPacket();
            if( iRet!=0 )
            {
                break;
            }

            // update txn status
            glProcInfo.stTranLog.uiStatus &= ~TS_NEED_TC;
            glProcInfo.stTranLog.ucTranType = glProcInfo.stTranLog.ucOrgTranType;
            UpdateTranLog(&glProcInfo.stTranLog, glProcInfo.uiRecNo);
#ifdef AMT_PROC_DEBUG
			glProcInfo.stTranLog.cntAmt = 0;//2014-9-19 ack POS to erase amt records
#endif

            if( stProcInfoBak.stTranLog.ucTranType!=SETTLEMENT )
            {
                break;
            }
            if(ChkIfCiti())
            {
                //脱机上送后，流水号会增加，所以当前交易的流水号需用增加后的流水号 build 1.00.0108b
                sprintf((char *)glSendPack.szSTAN, "%06lu", glSysCtrl.ulSTAN);  //
                glProcInfo.stTranLog.ulSTAN = glSysCtrl.ulSTAN;
            }
            continue;
        }

        glProcInfo.stTranLog.ucOrgTranType = glProcInfo.stTranLog.ucTranType;
        glProcInfo.stTranLog.ucTranType    = OFFLINE_SEND;
        DispTransName();

        SetCommReqField();

        // bit 4

        if( (glProcInfo.stTranLog.uiStatus & TS_VOID) )
        {
            glSendPack.szTranAmt[0] = 0;
        }
        // bit 37, 39
        if( !ChkIfAmex() )
        {
            if ((ChkIfICBC() || ChkIfWLB()) && (glProcInfo.stTranLog.uiStatus & TS_ADJ))
            {
                sprintf((char *)glSendPack.szRspCode, "%.2s", glProcInfo.stTranLog.szRspCode);
            }//added end

            if (PPDCC_ChkIfDccAcq())
            {
                if (memcmp(glProcInfo.stTranLog.szRspCode, "Y1", 2)==0) // Y1 is assigned by EMV kernel if offline approved.
                {
                    sprintf((char *)glSendPack.szRspCode, "00");
                }
            }

            if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
            {
                if (!ChkIfICBC())
                {
                    if(ChkIfBnu())
                    {
                        if(!(glProcInfo.stTranLog.uiStatus & OFFLINE_SEND))
                        {
                            sprintf((char *)glSendPack.szRspCode, "%.2s", glProcInfo.stTranLog.szRspCode);
                        }
                    }
                    else
                    {
                        sprintf((char *)glSendPack.szRspCode, "%.2s", glProcInfo.stTranLog.szRspCode);
                    }

                    if( glProcInfo.stTranLog.szRRN[0]!=0 )  // seen by host
                    {
                        glSendPack.szRspCode[0] = 0;
                    }
                }
                //Build 1.00.0112
                if (ChkIfBea())
                {
                    if (memcmp(glProcInfo.stTranLog.szRspCode, "Y1", 2)==0) // Y1 is assigned by EMV kernel if offline approved.
                    {
                        sprintf((char *)glSendPack.szAuthCode, "Y1    ");
                    }
                }
            }
        }
        if( ChkAcqOption(ACQ_DBS_FEATURE) )
        {// 香港星展银行,sale_complete上送时, VISA/MASTER需要上送 BIT37 BIT39
            if( glProcInfo.stTranLog.ucOrgTranType==SALE_COMP &&
                glProcInfo.stTranLog.szRRN[0]==0 )
            {
                sprintf((char *)glSendPack.szRRN, "%.12s", &glProcInfo.stTranLog.szRRN[1]);
            }
        }

        if( ChkIfAmex() && (glProcInfo.stTranLog.uiStatus & TS_FLOOR_LIMIT) )
        {
            if( !(glProcInfo.stTranLog.uiStatus & (TS_ADJ|TS_VOID)) )
            {
                sprintf((char *)glSendPack.szAddlRsp, "03");
            }
        }

        // bit 55
        if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
        {
            if( ChkIfAmex() )
            {
                if( !(glProcInfo.stTranLog.uiStatus & TS_ADJ) &&
                    ((glProcInfo.stTranLog.ucOrgTranType==OFF_SALE) ||
                     (glProcInfo.stTranLog.ucOrgTranType==SALE_COMP) ||
                     (glProcInfo.stTranLog.ucOrgTranType==SALE && (glProcInfo.stTranLog.uiStatus & TS_NOSEND))) )
                {
                    PubLong2Char((ulong)glProcInfo.stTranLog.uiIccDataLen, 2, glSendPack.sICCData);
                    memcpy(&glSendPack.sICCData[2], glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen);
                }
            }
            else
            {// only send ICC sale below floor
                if( glProcInfo.stTranLog.szRRN[0]==0 )
                {
                    PubLong2Char((ulong)glProcInfo.stTranLog.uiIccDataLen, 2, glSendPack.sICCData);
                    memcpy(&glSendPack.sICCData[2], glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen);
                }
            }
        }

        // bit 60
        if( !ChkIfAmex() && (glProcInfo.stTranLog.uiStatus & (TS_ADJ|TS_VOID)) )
        {
            if( glProcInfo.stTranLog.uiStatus & TS_VOID )
            {

                sprintf((char *)glSendPack.szField60, "%.12s", glProcInfo.stTranLog.szAmount);
#ifdef AMT_PROC_DEBUG
			//2014-9-18 each time come to glAmount or szAmount
			sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(53)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(53)%.13s",glProcInfo.stTranLog.szAmount);
			glProcInfo.stTranLog.cntAmt++;
#endif

            }
            else
            {
                //squall 2013.12.04,here the original amount has been modified
                //the former version set new adjusted amount into Original amount that should be a bug
                //  if (ChkCurAcqName("CITI", FALSE)||ChkIfDiners())//squall add diner send original amt
                //  {
                sprintf((char *)glSendPack.szField60, "%.12s", glProcInfo.stTranLog.szOrgAmount);
                //  }

                /*
                if (ChkCurAcqName("CITI", FALSE)||ChkIfDiners())//squall add diner send original amt
                {
                sprintf((char *)glSendPack.szField60, "%.12s", glProcInfo.stTranLog.szAmount);
                }
                else
                {
                sprintf((char *)glSendPack.szField60, "%.12s", glProcInfo.stTranLog.szOrgAmount);
                }
                */
            }
        }
        //BIT 61//build 1.00.0125
        if (ChkIfBea())
        {
            memset(szBuff,0,sizeof(szBuff));
            //strcpy(szBuff,"           ");//after 3 byes length,add 8 more bytes for Product code (can be space)
            strcpy(szBuff,"        ");//no length here,only 8 bytes product code
            if (glProcInfo.stTranLog.ucDccType==PPTXN_OPTOUT)
            {
                strcat(szBuff,"Y");
            }
            else
            {
                strcat(szBuff,"N");
            }
            if (glProcInfo.stTranLog.ucDccType!=PPTXN_OPTOUT)
            {
                strcat(szBuff,"000000000000000");
            }
            else
            {
                if (strstr(glProcInfo.stTranLog.szPPDccTxnID,"  ")!=NULL)//MASTERCARD
                {
                    strncat(szBuff,&glProcInfo.stTranLog.szPPDccTxnID[4],3);
                    strncat(szBuff,&glProcInfo.stTranLog.szPPDccTxnID[7],6);
                    strncat(szBuff,&glProcInfo.stTranLog.szPPDccTxnID[0],4);
                    strcat(szBuff,"  ");
                }
                else//visa
                {
                    strcat(szBuff,glProcInfo.stTranLog.szPPDccTxnID);
                }
            }
            //sprintf(szBuff1,"%03d",strlen(szBuff));
            //  memcpy(szBuff,szBuff1,3);
            strcpy(glSendPack.szField61,szBuff);
        }
        //build88R: bug fix - backup foreign amount & tips
        memset(szFrnAmt, 0, 13);
        memset(szFrnTipAmt, 0, 13);

        if( !ChkIfZeroAmt(glProcInfo.stTranLog.szFrnAmount) )
        {
            memcpy(szFrnAmt, glProcInfo.stTranLog.szFrnAmount, 12);

            if( !ChkIfZeroAmt(glProcInfo.stTranLog.szFrnTip))
            {
                memcpy(szFrnTipAmt, glProcInfo.stTranLog.szFrnTip, 12);
            }
        }
        //end build88R

        iRet = SendRecvPacket();
        if( iRet!=0 )
        {
            break;
        }

        //build88R: bug fix - restore foreign amount and tips if they are modified after transmission
        if( ChkIfZeroAmt(glProcInfo.stTranLog.szFrnAmount) )
        {
            if(szFrnAmt[0] != 0)
            {
                memcpy(glProcInfo.stTranLog.szFrnAmount, szFrnAmt, 12);
            }

            if( ChkIfZeroAmt(glProcInfo.stTranLog.szFrnTip) && szFrnTipAmt[0] != 0)
            {
                memcpy(glProcInfo.stTranLog.szFrnTip, szFrnTipAmt, 12);
            }
        }
        //end build88R

        // update txn status
        glProcInfo.stTranLog.uiStatus &= ~TS_NOSEND;
        glProcInfo.stTranLog.uiStatus |= TS_OFFLINE_SEND;
        glProcInfo.stTranLog.ucTranType = glProcInfo.stTranLog.ucOrgTranType;
        UpdateTranLog(&glProcInfo.stTranLog, glProcInfo.uiRecNo);
#ifdef AMT_PROC_DEBUG
		glProcInfo.stTranLog.cntAmt = 0;//2014-9-19 ack POS to erase amt records
#endif

        if( stProcInfoBak.stTranLog.ucTranType!=SETTLEMENT )
        {// is trickle feed, only need send one txn
            break;
        }
    }
    memcpy(&glProcInfo, &stProcInfoBak, sizeof(SYS_PROC_INFO));
    memcpy(&glSendPack, &glProcInfo.stSendPack, sizeof(STISO8583));

    return iRet;
}


//build1.0S: offline send adjustment advice message during void operation
int OfflineSendAdjust(void)
{
    uchar szFrnAmt[13], szFrnTipAmt[13];

    int iLength;
    int iRet;
    SYS_PROC_INFO   stProcInfoBak;

    if( glProcInfo.stTranLog.ucTranType!=SETTLEMENT )
    {// the time to load 400 txn log is about 1-2 seconds
        if( ChkAcqOption(ACQ_DISABLE_TRICK_FEED) )
        {
            return 0;
        }
    }

    memcpy(&glProcInfo.stSendPack, &glSendPack, sizeof(STISO8583));
    memcpy(&stProcInfoBak, &glProcInfo,  sizeof(SYS_PROC_INFO));

    {
        //---------------------------------------------------------------------------
        // This part is for TC-ADVICE
        if (glProcInfo.stTranLog.uiStatus & TS_NEED_TC)
        {
            glProcInfo.stTranLog.ucOrgTranType = glProcInfo.stTranLog.ucTranType;
            glProcInfo.stTranLog.ucTranType    = TC_SEND;

            DispTransName();
            SetCommReqField();

            //2010-6-8 : ICBC MACAU want to use 0320
            if (ChkIfICBC() || ChkIfCiti())//added chifciti() by laurence
            {
                sprintf((char *)glSendPack.szMsgCode, "0220");
            }

            // bit39
            //sprintf((char *)glSendPack.szRspCode, "%.2s", glProcInfo.stTranLog.szRspCode);
#ifdef ENABLE_EMV
            // bit55
            //SetDE55(DE55_TC, glSendPack.sICCData+2, &iLength);
            SetTCDE55(&glProcInfo.stTranLog, glSendPack.sICCData+2, &iLength);
            PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
#endif

            iRet = SendRecvPacket();
            if( iRet!=0 )
            {
                return iRet;
            }

            // update txn status
            glProcInfo.stTranLog.uiStatus &= ~TS_NEED_TC;
            glProcInfo.stTranLog.ucTranType = glProcInfo.stTranLog.ucOrgTranType;
            UpdateTranLog(&glProcInfo.stTranLog, glProcInfo.uiRecNo);
#ifdef AMT_PROC_DEBUG
			glProcInfo.stTranLog.cntAmt = 0;//2014-9-19 ack POS to erase amt records
#endif
        }

        //---------------------------------------------------------------------------
        glProcInfo.stTranLog.ucOrgTranType = glProcInfo.stTranLog.ucTranType;
        glProcInfo.stTranLog.ucTranType    = OFFLINE_SEND;
        DispTransName();

        SetCommReqField();

        // bit 4
        if( glProcInfo.stTranLog.uiStatus & TS_VOID )
        {
            glSendPack.szTranAmt[0] = 0;
        }

        // bit 37, 39
        if( !ChkIfAmex() )
        {
            if (ChkIfICBC() && (glProcInfo.stTranLog.uiStatus & TS_ADJ))//added by Arthur, 2010.12.09
            {
                sprintf((char *)glSendPack.szRspCode, "%.2s", glProcInfo.stTranLog.szRspCode);
            }//added end

            if (PPDCC_ChkIfDccAcq())
            {
                if (memcmp(glProcInfo.stTranLog.szRspCode, "Y1", 2)==0) // Y1 is assigned by EMV kernel if offline approved.
                {
                    sprintf((char *)glSendPack.szRspCode, "00");
                }
            }

            if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
            {
                if (!ChkIfICBC())
                {
                    if(ChkIfBnu())
                    {
                        if(!(glProcInfo.stTranLog.uiStatus & OFFLINE_SEND))
                        {
                            sprintf((char *)glSendPack.szRspCode, "%.2s", glProcInfo.stTranLog.szRspCode);
                        }
                    }
                    else
                    {
                        sprintf((char *)glSendPack.szRspCode, "%.2s", glProcInfo.stTranLog.szRspCode);
                    }

                    if( glProcInfo.stTranLog.szRRN[0]!=0 )  // seen by host
                    {
                        glSendPack.szRspCode[0] = 0;
                    }
                }
                //Build 1.00.0112
                if (ChkIfBea())
                {
                    if (memcmp(glProcInfo.stTranLog.szRspCode, "Y1", 2)==0) // Y1 is assigned by EMV kernel if offline approved.
                    {
                        sprintf((char *)glSendPack.szAuthCode, "Y1    ");
                    }
                }
            }
        }

        if( ChkAcqOption(ACQ_DBS_FEATURE) )
        {// 香港星展银行,sale_complete上送时, VISA/MASTER需要上送 BIT37 BIT39
            if( glProcInfo.stTranLog.ucOrgTranType==SALE_COMP &&
                glProcInfo.stTranLog.szRRN[0]==0 )
            {
                sprintf((char *)glSendPack.szRRN, "%.12s", &glProcInfo.stTranLog.szRRN[1]);
            }
        }

        if( ChkIfAmex() && (glProcInfo.stTranLog.uiStatus & TS_FLOOR_LIMIT) )
        {
            if( !(glProcInfo.stTranLog.uiStatus & (TS_ADJ|TS_VOID)) )
            {
                sprintf((char *)glSendPack.szAddlRsp, "03");
            }
        }

        // bit 55
        if( glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT )
        {
            if( ChkIfAmex() )
            {
                if( !(glProcInfo.stTranLog.uiStatus & TS_ADJ) &&
                    ((glProcInfo.stTranLog.ucOrgTranType==OFF_SALE) ||
                     (glProcInfo.stTranLog.ucOrgTranType==SALE_COMP) ||
                     (glProcInfo.stTranLog.ucOrgTranType==SALE && (glProcInfo.stTranLog.uiStatus & TS_NOSEND))) )
                {
                    PubLong2Char((ulong)glProcInfo.stTranLog.uiIccDataLen, 2, glSendPack.sICCData);
                    memcpy(&glSendPack.sICCData[2], glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen);
                }
            }
            else
            {// only send ICC sale below floor
                if( glProcInfo.stTranLog.szRRN[0]==0 )
                {
                    PubLong2Char((ulong)glProcInfo.stTranLog.uiIccDataLen, 2, glSendPack.sICCData);
                    memcpy(&glSendPack.sICCData[2], glProcInfo.stTranLog.sIccData, glProcInfo.stTranLog.uiIccDataLen);
                }
            }
        }

        // bit 60
        if( !ChkIfAmex() && (glProcInfo.stTranLog.uiStatus & (TS_ADJ|TS_VOID)) )
        {
#ifdef DISNEY //build1.0M: For Disney project, tips is added by adjustment. Therefore, the original amount is always the base amount
            sprintf((char *)glSendPack.szField60, "%.12s", glProcInfo.stTranLog.szAmount);
#else
            if( glProcInfo.stTranLog.uiStatus & TS_VOID )
            {
                sprintf((char *)glSendPack.szField60, "%.12s", glProcInfo.stTranLog.szAmount);
#ifdef AMT_PROC_DEBUG
			//2014-9-18 each time come to glAmount or szAmount
			sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(53)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(53)%.13s",glProcInfo.stTranLog.szAmount);
			glProcInfo.stTranLog.cntAmt++;
	//		//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 27: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
            }
            else
            {
                //squall 2013.12.04,here the original amount has been modified
                //the former version set new adjusted amount into Original amount that should be a bug
                if (ChkCurAcqName("CITI", FALSE)||ChkIfDiners())//squall add diner send original amt
                {
                    sprintf((char *)glSendPack.szField60, "%.12s", glProcInfo.stTranLog.szOrgAmount);
                }
                /*
                //!Lois: revised
                if (ChkCurAcqName("CITI", FALSE)||ChkIfDiners())//squall modify,Diners send amt without adjust
                {
                sprintf((char *)glSendPack.szField60, "%.12s", glProcInfo.stTranLog.szAmount);
                }
                else
                {
                sprintf((char *)glSendPack.szField60, "%.12s", glProcInfo.stTranLog.szOrgAmount);
                }
                */

            }
#endif
        }

        //bug fix - backup foreign amount & tips
        memset(szFrnAmt, 0, 13);
        memset(szFrnTipAmt, 0, 13);

        if( !ChkIfZeroAmt(glProcInfo.stTranLog.szFrnAmount) )
        {
            memcpy(szFrnAmt, glProcInfo.stTranLog.szFrnAmount, 12);

            if( !ChkIfZeroAmt(glProcInfo.stTranLog.szFrnTip))
            {
                memcpy(szFrnTipAmt, glProcInfo.stTranLog.szFrnTip, 12);
            }
        }
        //end bug fix


        iRet = SendRecvPacket();
        if( iRet!=0 )
        {
            return iRet;
        }


        //bug fix - restore foreign amount and tips if they are modified after transmission
        if( ChkIfZeroAmt(glProcInfo.stTranLog.szFrnAmount) )
        {
            if(szFrnAmt[0] != 0)
            {
                memcpy(glProcInfo.stTranLog.szFrnAmount, szFrnAmt, 12);
            }

            if( ChkIfZeroAmt(glProcInfo.stTranLog.szFrnTip) && szFrnTipAmt[0] != 0)
            {
                memcpy(glProcInfo.stTranLog.szFrnTip, szFrnTipAmt, 12);
            }
        }
        //end bug fix


        // update txn status
        glProcInfo.stTranLog.uiStatus &= ~TS_NOSEND;
        glProcInfo.stTranLog.uiStatus |= TS_OFFLINE_SEND;
        glProcInfo.stTranLog.ucTranType = glProcInfo.stTranLog.ucOrgTranType;
        UpdateTranLog(&glProcInfo.stTranLog, glProcInfo.uiRecNo);
#ifdef AMT_PROC_DEBUG
		glProcInfo.stTranLog.cntAmt = 0;//2014-9-19 ack POS to erase amt records
#endif
    }

    memcpy(&glProcInfo, &stProcInfoBak, sizeof(SYS_PROC_INFO));
    memcpy(&glSendPack, &glProcInfo.stSendPack, sizeof(STISO8583));

    return 0;
}
//end build1.0S



// transaction complete for voice referral
int TranSaleComplete(void)
{
#ifdef ENABLE_EMV
    int     iLength;
#endif
    int     iRet;
    uchar   ucKey;

    if( !ChkIssuerOption(ISSUER_EN_VOICE_REFERRAL) )
    {
        return ERR_TRAN_FAIL;
    }

    if( (glProcInfo.stTranLog.ucTranType!=PREAUTH) &&
        (glProcInfo.stTranLog.ucTranType!=AUTH) &&
        (glProcInfo.stTranLog.ucTranType!=SALE) &&
        (glProcInfo.stTranLog.ucTranType!=INSTALMENT) &&
        (glProcInfo.stTranLog.ucTranType!=CASH) )
    {

        return ERR_TRAN_FAIL;
    }

    if( glProcInfo.stTranLog.ucTranType>=SALE )
    {
        glProcInfo.stTranLog.ucTranType = SALE_COMP;
    }

    CommOnHook(FALSE);
    ScrPrint(0, 0, CFONT|REVER, "CALL:%-11.11s", glCurIssuer.szRefTelNo);
    PubDispString(_T("PLS CALL BANK"), 4|DISP_LINE_LEFT);

    if( ChkEdcOption(EDC_REFERRAL_DIAL) )
    {
        iRet = ReferralDial(glCurIssuer.szRefTelNo);
        if( iRet!=0 )
        {
            return iRet;
        }
    }
    else
    {
        ucKey = PubWaitKey(USER_OPER_TIMEOUT);
        if( ucKey==KEYCANCEL || ucKey==NOKEY )
        {
            ScrClrLine(4, 7);
            PubDispString(_T("TRANS CANCELED"), 4|DISP_LINE_LEFT);
            PubWaitKey(3);
            return ERR_NO_DISP;
        }
    }

    iRet = GetPreAuthCode();
    if( iRet!=0 )
    {
        ScrClrLine(4, 7);
        PubDispString(_T("TRANS CANCELED"), 4|DISP_LINE_LEFT);
        PubWaitKey(3);
        return ERR_NO_DISP;
    }

    // 香港星展银行,sale_complete上送时, VISA/MASTER需要上送 BIT37 BIT39
    // For DBS in HongKong.
    memmove(&glProcInfo.stTranLog.szRRN[1], glProcInfo.stTranLog.szRRN, 12);
    glProcInfo.stTranLog.szRRN[0] = 0;
    //  sprintf((char *)glProcInfo.stTranLog.szCondCode, "06");

#ifdef ENABLE_EMV
    if( (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) &&
        ChkIfAcqNeedDE56() )
    {
        SetDE56(glProcInfo.stTranLog.sField56, &iLength);
        glProcInfo.stTranLog.uiField56Len = (ushort)iLength;
    }
#endif

    return FinishOffLine();
}

// 退货
int TransRefund(void)
{
    int     iRet;
	ST_EVENT_MSG	stEventMsg;

    iRet = TransInit(REFUND);
    if( iRet!=0 )
    {
        return iRet;
    }

    ScrCls();
    DispTransName();
    if( !ChkEdcOption(EDC_NOT_REFUND_PWD) )
    {
        if( PasswordRefund()!=0 )
        {
            return ERR_NO_DISP;
        }
    }

    ScrCls();
    DispTransName();

//2016-3-29 get amount 
	/*=======BEGIN: Jason 34-08 2016.02.29  17:31 modify===========*/
    if (glSysParam.stEdcInfo.ucClssFlag == 1 && 
		((glSysParam.stEdcInfo.ucClssMode == 0)||(glSysParam.stEdcInfo.ucClssMode == 2))) //for cotactless ,  input amt at first.
    {
        iRet = GetAmount();
		if (iRet != 0)		
		{
			return ERR_USERCANCEL;
		}
    }
    /*====================== END======================== */
	/*=======BEGIN: Jason 2015.01.06  17:48 modify===========*/
    //Half EMV flow for refund and offline should be supported
    if(1/*glSysParam.stEdcInfo.ucHalfEmv  == 1*/)//2016-3-29 ???
    {
        /*=======BEGIN: Jason 34-08 2016.02.29  17:45 modify===========*/
        if((glSysParam.stEdcInfo.ucClssFlag == 1)&& (glProcInfo.stTranLog.ucTranType == REFUND))
        {
            iRet = GetCard(CARD_INSERTED|CARD_SWIPED|CARD_TAPPED|CARD_KEYIN);
        }
        else
			/*====================== END======================== */
        {
            iRet = GetCard(CARD_INSERTED|CARD_SWIPED|CARD_KEYIN);
        }
    }
    else
        /*====================== END======================== */
    {
        /*=======BEGIN: Jason 34-08 2016.02.29  17:45 modify===========*/
        if((glSysParam.stEdcInfo.ucClssFlag == 1)&& (glProcInfo.stTranLog.ucTranType == REFUND))
        {
            iRet = GetCard(SKIP_CHECK_ICC|CARD_SWIPED|CARD_TAPPED|CARD_KEYIN);
        }
        else
            /*====================== END======================== */
        {
            iRet = GetCard(SKIP_CHECK_ICC|CARD_SWIPED|CARD_KEYIN);
        }
    }

	
	//iRet = GetCard(FALSE, SKIP_CHECK_ICC|CARD_SWIPED|CARD_KEYIN|CARD_TAPPED, FALSE); //2016-2-5 AMEX Refund
	//2016-3-7 AE Refund
	if( glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS || iRet == 77)//Jason 34-08 2016.03.01 10:15
	{
		if(iRet == 77) 
		{
			iRet = 0;
		}
		return iRet;
	}
	if( iRet!=0 )
    {
        return iRet;
    }

    if( !ChkSettle() )
    {
        return ERR_NO_DISP;
    }

    if( ChkIssuerOption(ISSUER_NO_REFUND) )
    {
        DispBlockFunc();
        return ERR_NO_DISP;
    }

    iRet = GetAmount();
    if( iRet!=0 )
    {
        return iRet;
    }

    iRet = GetDescriptor();
    if( iRet!=0 )
    {
        return iRet;
    }

    iRet = GetAddlPrompt();
    if( iRet!=0 )
    {
        return iRet;
    }

    if(ChkIfBnu())
    {
        if (!ChkAcqOption(ACQ_ONLINE_REFUND))
        {
            iRet = GetPreAuthCode();
            if( iRet!=0 )
            {
                return iRet;
            }

        }
    }

    ScrCls();
    DispTransName();
    iRet = GetPIN(FALSE);
    if( iRet!=0 )
    {
        return iRet;
    }

    if (PPDCC_ChkIfDccAcq()) // PP-DCC
    {
        iRet = PPDCC_TransRateEnquiry();
        if (iRet)
        {
            return iRet;
        }
    }

    //build88S 1.0C: get RRN
    if (PPDCC_ChkIfDccAcq())
    {
        iRet = GetRRN();

        if (iRet)
        {
            return iRet;
        }
    }
    //end build88S

    ScrCls();
    DispTransName();

    if( ChkAcqOption(ACQ_ONLINE_REFUND) )
    {
        SetCommReqField();
        return TranProcess();
    }
    else
    {
        return FinishOffLine();
    }
}

// 离线
int TransOffSale(void)
{
    int     iRet;
    uchar   bHaveCode;

	
	iRet = TransInit(OFF_SALE);
    if( iRet!=0 )
    {
        return iRet;
    }

	iRet = GetCard(SKIP_CHECK_ICC|CARD_SWIPED|CARD_KEYIN);
    if( iRet!=0 )
    {
        return iRet;
    }


    if( !ChkSettle() )
    {
        return ERR_NO_DISP;
    }
    if( !ChkIssuerOption(ISSUER_EN_OFFLINE) )
    {
        DispBlockFunc();
        return ERR_NO_DISP;
    }
	
    iRet = GetAmount();
    if( iRet!=0 )
    {
        return iRet;
    }
    ScrCls();
    DispTransName();
    iRet = GetDescriptor();
    if( iRet!=0 )
    {
        return iRet;
    }

    iRet = GetAddlPrompt();
    if( iRet!=0 )
    {
        return iRet;
    }

    bHaveCode = FALSE;
    if (!PPDCC_ChkIfDccAcq())
    {
        iRet = GetPreAuthCode();
        if( iRet!=0 )
        {
            return iRet;
        }
        bHaveCode = TRUE;
    }

    ScrCls();
    DispTransName();
    iRet = GetPIN(FALSE);
    if( iRet!=0 )
    {
        return iRet;
    }

    if (PPDCC_ChkIfDccAcq()) // PP-DCC
    {
        iRet = PPDCC_TransRateEnquiry();
        if (iRet)
        {
            return iRet;
        }
    }
    CommOnHook(FALSE);
    if (!bHaveCode)
    {
        ScrCls();
        DispTransName();
        iRet = GetPreAuthCode();
        if( iRet!=0 )
        {
            return iRet;
        }
        bHaveCode = TRUE;
    }


    return FinishOffLine();
}

int TransVoid(void)
{
    int         iRet;
    ulong       ulInvoice;
    uchar       ucTranAct, bOnlineFlag, szTempAmt[12+1], sCurrencyCode[2];

    iRet = TransInit(VOID);
    if( iRet!=0 )
    {
        return iRet;
    }

    ScrCls();
    DispTransName();
    if( !ChkEdcOption(EDC_NOT_VOID_PWD) )
    {
        if( PasswordVoid()!=0 )
        {
            return ERR_NO_DISP;
        }
    }
    ulInvoice = EcrGetTxnID();
    if ((glProcInfo.ucEcrCtrl==ECR_BEGIN) && (ulInvoice>0))
    {
        iRet = GetRecordByInvoice(ulInvoice, TS_OK|TS_NOSEND|TS_ADJ, &glProcInfo.stTranLog);
        if( iRet!=0 )
        {
            return iRet;
        }
        ucTranAct = glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct;
        if (!(ucTranAct & VOID_ALLOW))
        {
            ScrClrLine(2, 7);
            PubDispString(_T("NOT ALLOW VOID"), 4|DISP_LINE_LEFT);
            PubBeepErr();
            PubWaitKey(3);
            return ERR_NO_DISP;
        }

        // PP-DCC
        /*Build88R: block void operation if the transaction is zero amount
        if (PPDCC_ChkIfDccAcq())
        */
        {
            if (ChkIfZeroAmt(glProcInfo.stTranLog.szAmount))
            {
                ScrClrLine(2, 7);
                PubDispString(_T("NOT ALLOW VOID"), 4|DISP_LINE_LEFT);
                PubBeepErr();
                PubWaitKey(3);
                return ERR_NO_DISP;
            }
        }
        if (ChkIfDCC_CITI())
        {
            //build88S: when void, offline send if the transaction is SALE or OFF_SALE + adjusted + not yet online
            if(
                (
                    (glProcInfo.stTranLog.ucTranType == SALE) && ChkEdcOption(EDC_TIP_PROCESS) &&
                    (glProcInfo.stTranLog.uiStatus & TS_NOSEND) && (glProcInfo.stTranLog.uiStatus & TS_ADJ)
                )
                ||
                (
                    glProcInfo.stTranLog.ucTranType == OFF_SALE && (glProcInfo.stTranLog.uiStatus & TS_NOSEND)
                )
            )
            {
                FindAcq(glProcInfo.stTranLog.ucAcqKey);
                FindIssuer(glProcInfo.stTranLog.ucIssuerKey);

                OfflineSendAdjust();
                DispTransName();

                iRet = GetRecordByInvoice(ulInvoice, TS_OK|TS_NOSEND|TS_ADJ, &glProcInfo.stTranLog);

                if( iRet!=0 )
                {
                    return iRet;
                }
            }
            //end buid88S
        }
        glProcInfo.stTranLog.ucOrgTranType = glProcInfo.stTranLog.ucTranType;
        glProcInfo.stTranLog.ucTranType    = VOID;
    }
    else
    {
        while( 1 )
        {
            iRet = GetRecord(TS_OK|TS_NOSEND|TS_ADJ, &glProcInfo.stTranLog);
            if( iRet!=0 )
            {
                return iRet;
            }
            //2014-8-13 if record is auth/pre-auth, do not allow void
            if(glProcInfo.stTranLog.ucTranType == AUTH || glProcInfo.stTranLog.ucTranType == PREAUTH)
            {
                ScrClrLine(2, 7);
                PubDispString(_T("NOT ALLOW VOID"), 4|DISP_LINE_LEFT);
                PubBeepErr();
                PubWaitKey(3);
                return ERR_NO_DISP;
            }
            ucTranAct = glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct;

            // PP-DCC
            /*Build88R: block void operation if the transaction is zero amount
            if (PPDCC_ChkIfDccAcq() &&
            ChkIfZeroAmt(glProcInfo.stTranLog.szAmount) )
            */
            if ( ChkIfZeroAmt(glProcInfo.stTranLog.szAmount) )
            {
                ScrClrLine(2, 7);
                PubDispString(_T("NOT ALLOW VOID"), 4|DISP_LINE_LEFT);
                PubBeepErr();
                PubWaitKey(3);
                continue;
            }

            //build88S: when void, offline send if the transaction is SALE or OFF_SALE + adjusted + not yet online
            if (ChkIfDCC_CITI())//squall 2013.12.13,restore Raymond modified change for CITI_DCC but don't know why
            {
                if(
                    (
                        (glProcInfo.stTranLog.ucTranType == SALE) && ChkEdcOption(EDC_TIP_PROCESS) &&
                        (glProcInfo.stTranLog.uiStatus & TS_NOSEND) && (glProcInfo.stTranLog.uiStatus & TS_ADJ)
                    )
                    ||
                    (
                        glProcInfo.stTranLog.ucTranType == OFF_SALE && (glProcInfo.stTranLog.uiStatus & TS_NOSEND)
                    )
                )
                {
                    ulInvoice = glProcInfo.stTranLog.ulInvoiceNo;

                    FindAcq(glProcInfo.stTranLog.ucAcqKey);
                    FindIssuer(glProcInfo.stTranLog.ucIssuerKey);

                    OfflineSendAdjust();//send adjust if ok status=SEND,else keep oringinal status
                    DispTransName();

                    iRet = GetRecordByInvoice(ulInvoice, TS_OK|TS_NOSEND|TS_ADJ, &glProcInfo.stTranLog);
                    //update glProcInfo.stTranLog

                    if( iRet!=0 )
                    {
                        return iRet;
                    }
                }
            }
            //end build88S


            if( ucTranAct & VOID_ALLOW )
            {
                glProcInfo.stTranLog.ucOrgTranType = glProcInfo.stTranLog.ucTranType;
                glProcInfo.stTranLog.ucTranType    = VOID;
                break;
            }

            PubDispString(_T("NOT ALLOW VOID"), 4|DISP_LINE_LEFT);
            PubBeepErr();
            PubWaitKey(3);
        }
    }
    FindAcq(glProcInfo.stTranLog.ucAcqKey);
    FindIssuer(glProcInfo.stTranLog.ucIssuerKey);

    ScrCls();
    DispTransName();
    if( ChkIssuerOption(ISSUER_NO_VOID) )
    {
        DispBlockFunc();
        return ERR_NO_DISP;
    }

    if( !ChkSettle() )
    {
        return ERR_NO_DISP;
    }

    if( ChkAcqOption(ACQ_ONLINE_VOID) )
    {
        ScrCls();
        DispTransName();
        DispWait();
        PreDial();
    }

    PubDispString(_T("TOTAL"), 4|DISP_LINE_LEFT);
    PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTempAmt);
    DispAmount(4, szTempAmt);   // show total amount
    PubDispString(_T("VOID ? Y/N"), DISP_LINE_LEFT|6);
    if( PubYesNo(USER_OPER_TIMEOUT) )
    {
        return ERR_USERCANCEL;
    }

    ScrCls();
    DispTransName();

    bOnlineFlag = TRUE;

    if( glProcInfo.stTranLog.uiStatus & TS_NOSEND )
    {// offsale/sale below/sale comp.../adjust
        if( glProcInfo.stTranLog.uiStatus & TS_ADJ )
        {// 该调整还未上送,作废调整后的金额
            sprintf(glProcInfo.stTranLog.szAbolishedTip,"%12s",glProcInfo.stTranLog.szTipAmount);
            //in order to print adjusted amount,here need to save Abolished tip. squall 2013.12.14
            PubAscSub(glProcInfo.stTranLog.szOrgAmount, glProcInfo.stTranLog.szAmount, 12, szTempAmt);
            sprintf((char *)glProcInfo.stTranLog.szTipAmount, "%.12s", szTempAmt);
        }
        if( glProcInfo.stTranLog.szRRN[0]==0)
        {
            bOnlineFlag = FALSE;
        }
    }

    if( !ChkAcqOption(ACQ_ONLINE_VOID) )
    {
        bOnlineFlag = FALSE;
    }

    if( bOnlineFlag )
    {
        SetCommReqField();
        return TranProcess();
    }
    else    // offline void
    {
        glProcInfo.stTranLog.uiStatus |= (TS_VOID|TS_NOSEND);
        if( glProcInfo.stTranLog.szRRN[0]==0 )
        {// not seen by host(orginal txn is offsale/sale below/salecomp/ ...)
            glProcInfo.stTranLog.uiStatus &= ~(TS_NOSEND);
        }
        PubGetDateTime(glProcInfo.stTranLog.szDateTime);
        UpdateTranLog(&glProcInfo.stTranLog, glProcInfo.uiRecNo);
#ifdef AMT_PROC_DEBUG
		glProcInfo.stTranLog.cntAmt = 0;//2014-9-19 ack POS to erase amt records
#endif

        //build88S
        glSysCtrl.uiRePrnRecNo = glProcInfo.uiRecNo;
        SaveSysCtrlBase();
        //end build88S

        // PP-DCC
        if ((glProcInfo.stTranLog.ucOrgTranType==SALE) ||
            (glProcInfo.stTranLog.ucOrgTranType==OFF_SALE))
        {
            // Amount
            memset(szTempAmt, 0, sizeof(szTempAmt));
            PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTempAmt);

            // Currency code
            memcpy(sCurrencyCode, "\x00\x00", 2);
            if (PPDCC_ChkIfDccAcq() ||
                (!PPDCC_ChkIfDccAcq() && (glProcInfo.stTranLog.ucDccType==PPTXN_OPTOUT)))
            {
                memcpy(sCurrencyCode, glProcInfo.stTranLog.stHolderCurrency.sCurrencyCode, 2);
            }
            if (PPDCC_ChkIfDccAcq())
            {
                PPDCC_UpdateTransStatistic(sCurrencyCode, szTempAmt, '-', NULL, 0);
            }
            else
            {
                PPDCC_UpdateTransStatistic(sCurrencyCode, NULL, 0, szTempAmt, '-');
            }
        }
		EcrSendTransSucceed();//2014-10-31 need return
        PrintReceipt(PRN_NORMAL);
        DispResult(0);

        return 0;
    }
}

int TransOther(void)
{
    int     iRet, iMenuNo;
    uchar   ucTranType;
    static  MenuItem stTranMenu[10] =
    {
        {TRUE, _T_NOOP("VOID    "),     NULL},
        {TRUE, _T_NOOP("OFFLINE "),     NULL},
        {TRUE, _T_NOOP("REFUND  "),     NULL},
        {TRUE, _T_NOOP("ADJUST  "),     NULL},
        {TRUE, _T_NOOP("SETTLE  "),     NULL},
        {TRUE, "", NULL},   // reserved for auth/preauth
		{TRUE, "", NULL},   // reserved for auth/preauth
		{TRUE, "", NULL},   // reserved for preauth void
		{TRUE, "", NULL},   // reserved for preauth complete
		{TRUE, "", NULL},   // reserved for preauth complete void
        //{TRUE, _T_NOOP("INSTALLMENT"),  NULL},
    };
    static  uchar   szPreAuthTitle[] =         _T_NOOP("PREAUTH ");
    static  uchar   szPreAuthVoidTitle[] =     _T_NOOP("PREAUTH VOID");
    static  uchar   szPreAuthCompTitle[] =     _T_NOOP("PREAUTH COMP");
    static  uchar   szPreAuthCompVoidTitle[] = _T_NOOP("PREAUTH COMP_V");
    static  uchar   szAuthTitle[]    = _T_NOOP("AUTH    ");
    static  uchar   szPrompt[]       = _T_NOOP("PLS SELECT:");
	static  uchar   szFqTitle[]      = _T_NOOP("INSTALLMENT ");
    if( ChkEdcOption(EDC_AUTH_PREAUTH) )
    {
        ucTranType = AUTH;
        sprintf((char *)stTranMenu[5].szMenuName, "%s", szAuthTitle);
    }
    else
    {
        ucTranType = PREAUTH;
        sprintf((char *)stTranMenu[5].szMenuName, "%s", szPreAuthTitle);
        sprintf((char *)stTranMenu[6].szMenuName, "%s", szPreAuthVoidTitle);
        sprintf((char *)stTranMenu[7].szMenuName, "%s", szPreAuthCompTitle);
        sprintf((char *)stTranMenu[8].szMenuName, "%s", szPreAuthCompVoidTitle);
    }

	//Gillian 20160824
    //if (/*(glSysParam.ucPlanNum==0) ||*/ !ChkEdcOption(EDC_ENABLE_INSTALMENT))
    //{
    //    stTranMenu[6].szMenuName[0] = 0;    // delete installment item.
    //} //removed by richard 20161110

	if( ChkEdcOption(EDC_ENABLE_INSTALMENT) )
	{
        if(ChkEdcOption(EDC_AUTH_PREAUTH))
        {
		    sprintf((char *)stTranMenu[6].szMenuName, "%s", szFqTitle);
        }
        else 
        {
            sprintf((char *)stTranMenu[9].szMenuName, "%s", szFqTitle);
        }
	}
    iMenuNo = PubGetMenu((uchar *)_T(szPrompt), stTranMenu, MENU_AUTOSNO|MENU_PROMPT, USER_OPER_TIMEOUT);
    switch( iMenuNo )
    {
        case 0:
            iRet = TransVoid();
            break;

        case 1:
			if(ChkAnyIndirectCupAcq())  //modified by jeff_xiehuan 20170401 for cancle offline and add adjust instead of offline 
			{
				ScrCls();
				DispBlockFunc();
				iRet = ERR_NO_DISP;
			}
			else
			{
				iRet = TransOffSale();
			}
            break;

        case 2:
            iRet = TransRefund();
            break;

        case 3:
			//if(ChkAnyIndirectCupAcq()) //modified by jeff_xiehuan 20170401 for change adjust to offline.
			//{
			//	iRet = TransOffSale();
			//}
			//else
			{
				TransAdjust();
				iRet = 0;
			}
            break;

        case 4:
            iRet = TransSettle();
            break;

        case 5:
            iRet = TransAuth(ucTranType);
            break;

        case 6:
            if(ChkEdcOption(EDC_AUTH_PREAUTH))
            {
                iRet = InstallmentMenu();
            }
            else 
            {
                iRet = TransAuth(PREAUTH_VOID);
            }
			break;

        case 7:
            iRet = TransAuth(PREAUTH_COMP);
            break; 
            
        case 8:
            iRet = TransAuth(PERAUTH_COMP_VOID);
            break;

        case 9:
            iRet = InstallmentMenu();
			break;

        default:
            return ERR_NO_DISP;
    }

    CommOnHook(FALSE);

    return iRet;
}

// 调整交易输入
int AdjustInput(void)
{
    int     iRet;

    while( 1 )
    {
        InitTransInfo();
        DispAdjustTitle();
        iRet = GetRecord(TS_NOSEND|TS_ADJ, &glProcInfo.stTranLog);
        if( iRet!=0 )
        {
            return iRet;
        }
        //2014-8-13 if record is auth/pre-auth, do not allow adjustment
        if(glProcInfo.stTranLog.ucTranType == AUTH || glProcInfo.stTranLog.ucTranType == PREAUTH)
        {
            ScrClrLine(2, 7);
            PubDispString(_T("NOT ALLOW ADJUST"), 4|DISP_LINE_LEFT);
            PubBeepErr();
            PubWaitKey(3);
            return ERR_NO_DISP;
        }
        ScrCls();
        DispAdjustTitle();
        if( ChkIfZeroAmt(glProcInfo.stTranLog.szAmount) &&
            ChkIfZeroAmt(glProcInfo.stTranLog.szTipAmount) )
        {
            PubDispString(_T("DO NOT ADJUST"), 4|DISP_LINE_LEFT);
            PubBeepErr();
            PubWaitKey(3);
            continue;
        }

        if( glProcInfo.stTranLog.ucTranType==REFUND && ChkIfAmex() )
        {
            PubDispString(_T("DO NOT ADJUST"), 4|DISP_LINE_LEFT);
            PubBeepErr();
            PubWaitKey(3);
            continue;
        }

        FindAcq(glProcInfo.stTranLog.ucAcqKey);
        FindIssuer(glProcInfo.stTranLog.ucIssuerKey);
        if( !ChkIssuerOption(ISSUER_EN_ADJUST) )
        {
            DispBlockFunc();
            continue;
        }
        if( ChkSettle() )
        {
            break;
        }
        return ERR_NO_DISP;
    }

    return 0;
}

static int GetValidAdjustAmount(void)
{
    uchar   ucResult, bIsDcc;
    char    szCurrName[32], szTemp[12+1];
    uchar   szTotalAmt[LEN_TRAN_AMT+1], szForeignAmt[LEN_TRAN_AMT+1];

    bIsDcc = PPDCC_ChkIfDccAcq();

    ScrCls();
    DispAdjustTitle();

    while (1)
    {
        if (bIsDcc)
        {
            DispAmount_Currency(2, glProcInfo.stTranLog.szFrnAmount, &glProcInfo.stTranLog.stHolderCurrency);
            PubDispString(_T("NEW TOTAL"), 4|DISP_LINE_LEFT);
            sprintf(szCurrName, "%.3s", glProcInfo.stTranLog.stHolderCurrency.szName);
            memset(szForeignAmt, 0, sizeof(szForeignAmt));
            ucResult = PubGetAmount(szCurrName, glProcInfo.stTranLog.stTranCurrency.ucDecimal,
                                    1, 8, szForeignAmt, USER_OPER_TIMEOUT, 0,0);
            if( ucResult!=0 )
            {
                return ERR_USERCANCEL;
            }

            AmtConvToBit4Format(szForeignAmt, glProcInfo.stTranLog.stTranCurrency.ucIgnoreDigit);
//          if (memcmp(glProcInfo.stTranLog.stTranCurrency.szName,"IDR",3)==0)
//          {
//              glProcInfo.stTranLog.stTranCurrency.ucIgnoreDigit=2;//IDR is special,
//          }
            PPDCC_CalcLocalAmount(szForeignAmt, &glProcInfo.stTranLog.stHolderCurrency,
                                  glProcInfo.stTranLog.szDccRate,
                                  &glSysParam.stEdcInfo.stLocalCurrency, szTotalAmt);
            //NO DECIMAL
            if(ChkOptionExt(glSysParam.stEdcInfo.sExtOption, EDC_EXT_DCC_ADJUST_AMT_NO_DECIMAL))
            {

                int DecimalPos;
                uchar   Mod[LEN_TRAN_AMT]= {0};
                DecimalPos = LEN_TRAN_AMT-glSysParam.stEdcInfo.stLocalCurrency.ucDecimal;
                if (szTotalAmt[DecimalPos]>'4')
                {
                    memset(&szTotalAmt[DecimalPos],'0',glSysParam.stEdcInfo.stLocalCurrency.ucDecimal);
                    memset(Mod,'0',LEN_TRAN_AMT);
                    Mod[DecimalPos-1]='1';
                    PubAscAdd(szTotalAmt, Mod, LEN_TRAN_AMT, szTotalAmt);
                }
                else
                {
                    memset(&szTotalAmt[DecimalPos],'0',glSysParam.stEdcInfo.stLocalCurrency.ucDecimal);
                }
            }
            //build88R: if the foreign tips is zero after DCC adjust, the local tips should also be zero to ensure the returned local amount the same as the ECR
            memset(szTemp, 0, 13);

            PubAscSub(szForeignAmt, glProcInfo.stTranLog.szFrnAmount, 12, szTemp);

            if(ChkIfZeroAmt(szTemp))
            {
                memcpy(szTotalAmt, glProcInfo.stTranLog.szAmount, 12);
            }

            memset(szTemp, 0, 13);
            //end build88R
        }
        else
        {

            DispAmount(2, glProcInfo.stTranLog.szAmount);
            PubDispString(_T("NEW TOTAL"), 4|DISP_LINE_LEFT);
            sprintf(szCurrName, "%.3s", glSysParam.stEdcInfo.stLocalCurrency.szName);
            memset(szTotalAmt, 0, sizeof(szTotalAmt));
            ucResult = PubGetAmount(szCurrName, glProcInfo.stTranLog.stTranCurrency.ucDecimal,
                                    1, 8, szTotalAmt, USER_OPER_TIMEOUT, 0,0);
            if( ucResult!=0 )
            {
                return ERR_USERCANCEL;
            }

            AmtConvToBit4Format(szTotalAmt, glProcInfo.stTranLog.stTranCurrency.ucIgnoreDigit);
        }

        if( !ValidAdjustAmount(glProcInfo.stTranLog.szAmount, szTotalAmt) )
        {
#ifdef AMT_PROC_DEBUG
			//2014-9-18 each time come to glAmount or szAmount
			sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(60)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(60)%.13s",glProcInfo.stTranLog.szAmount);
			glProcInfo.stTranLog.cntAmt++;
	//		//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 28: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
            continue;
        }
        if (!ValidBigAmount(szTotalAmt))
        {
            continue;
        }
        if (bIsDcc && !ValidBigAmount(szForeignAmt))
        {
            continue;
        }

        if (bIsDcc)
        {
            ScrClrLine(2, 7);
            if( !ConfirmAmount_PPDCC(szForeignAmt, &glProcInfo.stTranLog.stHolderCurrency,
                                     szTotalAmt, &glSysParam.stEdcInfo.stLocalCurrency) )
            {
                return ERR_NO_DISP;
            }
        }
        else
        {
            if( !ConfirmAmount(NULL, szTotalAmt) )
            {
                return ERR_NO_DISP;
            }
        }
        //---------------------squall 2013.12.04------------------------------------------------
        if( !(glProcInfo.stTranLog.uiStatus & TS_NOSEND) )
        {
            // Here bug modify, Original amount should set before calculate new amount
            PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szTemp);
            sprintf((char *)glProcInfo.stTranLog.szOrgAmount, "%.12s", szTemp);
            glProcInfo.stTranLog.uiStatus |= (TS_ADJ|TS_NOSEND);
        }
        //----------------------------------------------------------------
        // Calculate new tip amount
        PubAscSub(szTotalAmt, glProcInfo.stTranLog.szAmount, 12, szTemp);
        sprintf((char *)glProcInfo.stTranLog.szTipAmount, "%.12s", szTemp);
        // Calculate new foreign tip amount
        if (bIsDcc)
        {
            PubAscSub(szForeignAmt, glProcInfo.stTranLog.szFrnAmount, 12, szTemp);
            sprintf((char *)glProcInfo.stTranLog.szFrnTip, "%.12s", szTemp);
        }
        return 0;
    }
}

// 调整
void TransAdjust( void )
{
    int     iRet;
    char    cAction;
    uchar   szTemp[12+1], sCurrencyCode[2], szOrgAmount[12+1], szNewAmount[12+1];
    uchar   szCurTime[14+1];

    PubShowTitle(TRUE, (uchar *)_T("ADJUST          "));

    if (!ChkEdcOption(EDC_TIP_PROCESS))
    {
        DispBlockFunc();
        return;
    }

    if( !ChkEdcOption(EDC_NOT_ADJUST_PWD) )
    {
        if( PasswordAdjust()!=0 )
        {
            return;
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

        memset(szOrgAmount, 0, sizeof(szOrgAmount));
        PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szOrgAmount);

        while( 2 )
        {
            iRet = GetValidAdjustAmount();
            if (iRet==ERR_USERCANCEL)
            {
                return;
            }
            else if (iRet==ERR_NO_DISP)
            {
                break;
            }

            PubGetDateTime(szCurTime);
            sprintf((char *)glProcInfo.stTranLog.szDateTime, "%.14s", szCurTime);

            iRet = UpdateTranLog(&glProcInfo.stTranLog, glProcInfo.uiRecNo);
#ifdef AMT_PROC_DEBUG
			glProcInfo.stTranLog.cntAmt = 0;//2014-9-19 ack POS to erase amt records
#endif
            if( iRet!=0 )
            {
                return;
            }

            //          PrintReceipt(PRN_NORMAL);   // no receipt for adjust
            glSysCtrl.uiLastRecNo = glProcInfo.uiRecNo;     // use reprint to print adjust
#ifdef SUPPORT_TABBATCH
			glSysCtrl.ucLastTransIsAuth = FALSE;
#endif
            glSysCtrl.uiRePrnRecNo = glSysCtrl.uiLastRecNo;  //build88S

            SaveSysCtrlBase();

            // PP-DCC
            // PP-DCC
            if ((glProcInfo.stTranLog.ucTranType==SALE) ||
                (glProcInfo.stTranLog.ucTranType==OFF_SALE))
            {
                // Currency code
                memcpy(sCurrencyCode, "\x00\x00", 2);
                if (PPDCC_ChkIfDccAcq() ||
                    (!PPDCC_ChkIfDccAcq() && (glProcInfo.stTranLog.ucDccType==PPTXN_OPTOUT)))
                {
                    memcpy(sCurrencyCode, glProcInfo.stTranLog.stHolderCurrency.sCurrencyCode, 2);
                }
                // Amount, action
                PubAscAdd(glProcInfo.stTranLog.szAmount, glProcInfo.stTranLog.szTipAmount, 12, szNewAmount);
                if (memcmp(szNewAmount, szOrgAmount, 12)>0)
                {
                    PubAscSub(szNewAmount, szOrgAmount, 12, szNewAmount);
                    cAction = '>';
                }
                else
                {
                    PubAscSub(szOrgAmount, szNewAmount, 12, szOrgAmount);
                    strcpy(szNewAmount, szOrgAmount);
                    cAction = '<';
                }
                // Opt-in / opt-out
                if (PPDCC_ChkIfDccAcq())
                {
                    PPDCC_UpdateTransStatistic(sCurrencyCode, szNewAmount, cAction, NULL, 0);
                }
                else
                {
                    PPDCC_UpdateTransStatistic(sCurrencyCode, NULL, 0, szNewAmount, cAction);
                }
            }

            DispAdjustTitle();
            DispAccepted();
            PubBeepOk();
            PubWaitKey(3);
            break;  // continue to adjust
        }   // end of while( 2
    }   // end of while( 1
}

// 线路测试 Echo test
void TransEchoTest(void)
{
    int     iRet;
    uchar   ucAcqIndex, szTitle[16+1];

    TransInit(ECHO_TEST);

    sprintf((char *)szTitle, "%.16s", glSysParam.ucAcqNum>8 ? "SELECT ACQ:" : "SELECT ACQUIRER");
    iRet = SelectAcq(FALSE, szTitle, &ucAcqIndex);
    if( iRet!=0 )
    {
        return;
    }

    ScrCls();
    DispTransName();

    SetCommReqField();
    iRet = SendRecvPacket();
    DispResult(iRet);
}

// 结算
int TransSettle(void)
{
    int     iRet;
    uchar   ucAcqIndex, szTitle[16+1];
    uchar   ucForceSettle;
	//uchar i, iEPPCnt=0;//2016-2-15 remove AMEX EPP for now

    TransInit(SETTLEMENT);


    if ((glSysCtrl.ucLoginStatus == LOGIN_MODE) && (glSysCtrl.ucCurrAcqIdx != 0xff))
    {
        SetCurAcq(glSysCtrl.ucCurrAcqIdx);
        ucAcqIndex = glSysCtrl.ucCurrAcqIdx;
    }
    else
    {
        sprintf((char *)szTitle, "%.16s", glSysParam.ucAcqNum>8 ? "SELECT ACQ:" : "SELECT ACQUIRER");
        iRet = SelectAcq(TRUE, szTitle, &ucAcqIndex);
        if( iRet!=0 )
        {
            return ERR_NO_DISP;
        }
    }
    ScrCls();
    DispTransName();
    DispProcess();
    if( ucAcqIndex==MAX_ACQ )
    {
        CalcTotal(ACQ_ALL);
 		//for(ucAcqIndex=0; ucAcqIndex<glSysParam.ucAcqNum; ucAcqIndex++)//2016-1-11
  //      {
		//	for(i=0; i<glSysParam.ucPlanNum; i++)//2016-1-11  EPP Summary
		//	{
		//		if(glSysParam.stPlanList[i].ucAcqIndex ==ucAcqIndex)
		//		{
		//			iEPPCnt += glAcqTotal[ucAcqIndex].uiSaleCnt;					
		//		}
		//	}
		//}
		//ucAcqIndex=MAX_ACQ;//2016-2-15 remove AMEX EPP for now
   }
    else
    {
        CalcTotal(glCurAcq.ucKey);
    }

    iRet = DispTransTotal(FALSE);
    if( iRet!=0 )
    {
        return ERR_NO_DISP;
    }
    //CHB if batch is empty just return with no settle  build131 squall
    if( ChkIfZeroTotal(&glTransTotal) )
    {
        if(ChkIfCHBFalse())
        {
            ScrCls();
            DispTransName();
            PubDispString(_T("  EMPTY BATCH"),     3|DISP_LINE_LEFT);
            PubDispString(_T("SETTLE NOT ALOW"), 5|DISP_LINE_LEFT);
            PubWaitKey(USER_OPER_TIMEOUT);
            return ERR_NO_DISP;
        }
    }
    ucForceSettle = FALSE;
    if( ChkIfZeroTotal(&glTransTotal) )
    {
        if (PPDCC_ChkIfDccAcq() && (glTransTotal.uiZeroCount!=0)) // HJJ 20110928
        {
        }
        else
        {

            ScrCls();
            DispTransName();
            PubDispString(_T("EMPTY BATCH"),     3|DISP_LINE_LEFT);
            PubDispString(_T("SETTLE ANYWAY ?"), 5|DISP_LINE_LEFT);
            if (PubYesNo(USER_OPER_TIMEOUT)!=0)
            {
                if ((glSysCtrl.ucLoginStatus == LOGIN_MODE) && (glSysCtrl.ucCurrAcqIdx != 0xff))
                {
                    glSysCtrl.ucLoginStatus = LOGOFF_MODE;
                    glSysCtrl.ucCurrAcqIdx = 0xff;
                    SaveSysCtrlBase();
                }
                return ERR_NO_DISP;
            }
            ucForceSettle = TRUE;
        }
    }

    ScrCls();
    DispTransName();
    if( !ChkEdcOption(EDC_NOT_SETTLE_PWD) )
    {
        if( PasswordSettle()!=0 )
        {
            return ERR_NO_DISP;
        }
    }

    if( ucAcqIndex!=MAX_ACQ  && glSysParam.ucAcqNum!=1 )//2014-6-13 in case there is only one CITI acq
    {
        return TransSettleSub();
    }
    else
    {
        //2013-11-12 need to print terminal total!
        //2014-12-6 only CITI acquirers need print this total
        for(ucAcqIndex=0; ucAcqIndex<glSysParam.ucAcqNum; ucAcqIndex++)
        {
            SetCurAcq(ucAcqIndex);
            if(ChkIfCiti()||glSysParam.stEdcInfo.ucPrnStlTotalFlag /*|| iEPPCnt != 0*/)//2014-11-3 add a switch to control //2016-1-11 add for EPP//2016-2-15 remove AMEX EPP for now
            {
                PrnTotalAll(0);
                break;
            }
        }
    }
    for(ucAcqIndex=0; ucAcqIndex<glSysParam.ucAcqNum; ucAcqIndex++)
    {
        SetCurAcq(ucAcqIndex);
        memcpy(&glTransTotal, &glAcqTotal[ucAcqIndex], sizeof(TOTAL_INFO));
        if( ChkIfZeroTotal(&glTransTotal) )
        {
            if(ChkIfCHBFalse())//CHB do not allow 0 settle continue to next acq
            {
                ScrCls();
                DispTransName();
                PubDispString(_T("  EMPTY BATCH"),     3|DISP_LINE_LEFT);
                PubDispString(_T("SETTLE NOT ALOW"), 5|DISP_LINE_LEFT);
                PubWaitKey(USER_OPER_TIMEOUT);
                continue;
            }
        }
        if (ChkIfZeroTotal(&glTransTotal) && PPDCC_ChkIfDccAcq() && (glTransTotal.uiZeroCount!=0)) // HJJ 20110928
        {

        }
        else if (ChkIfZeroTotal(&glTransTotal) && !ucForceSettle )
        {			
            continue;
        }
        if (ucAcqIndex==(glSysParam.ucAcqNum-1))
        {
            glLastPageOfSettle = 1;

        }
        else
        {
            glLastPageOfSettle = 0;
        }


        iRet = TransSettleSub();

        if( iRet!=0 )
        {
            if( iRet!=ERR_NO_DISP )
            {
                return iRet;
            }
        }
        // last error ?
    }


    return 0;
}

// 结算当前收单行(glCurAcq)
// Settle current acquirer.
int TransSettleSub(void)
{
#ifdef ENABLE_EMV
    int     iLength;
#endif
    int     iRet, bFirstSettle;
    uchar   szBuff[20];

    // don't need to check zero total. it has been check outside.

    TransInit(SETTLEMENT);

    sprintf((char*)glProcInfo.szSettleMsg, "CLOSED");
    if( glCurAcq.ucPhoneTimeOut<150 )   // ???? How about Tcp
    {// 结算时，timeout加长
        glCurAcq.ucPhoneTimeOut += glCurAcq.ucPhoneTimeOut/2;
    }
    else
    {
        glCurAcq.ucPhoneTimeOut = 250;
    }

    iRet = TranReversal();

    if( iRet!=0 )
    {
        return iRet;
    }

    iRet = OfflineSend(OFFSEND_TC | OFFSEND_TRAN);

    if( iRet!=0 )
    {
        return iRet;
    }

    bFirstSettle = TRUE;
    while( 1 )
    {
        TransInit(SETTLEMENT);
        ScrPrint(48, 0, CFONT|REVER, "%10.10s", glCurAcq.szName);
        SetCommReqField();
        sprintf((char *)glSendPack.szProcCode, "%s", bFirstSettle ? "920000" : "960000");
        if( ChkIfAmex() )
        {
            memcpy((char *)glSendPack.sField62, "\x00\x06", 2);
            sprintf((char *)glSendPack.sField62+2, "%06lu", glSysCtrl.ulInvoiceNo);
        }
        //PubAddHeadChars(glTransTotal.szSaleAmt,   12, '0');  no need: already 12 digits
        //PubAddHeadChars(glTransTotal.szRefundAmt, 12, '0');  no need: already 12 digits
        sprintf((char *)&glSendPack.sField63[2], "%03d%12.12s%03d%12.12s",
                glTransTotal.uiSaleCnt,   glTransTotal.szSaleAmt,
                glTransTotal.uiRefundCnt, glTransTotal.szRefundAmt);
        memset(&glSendPack.sField63[30+2], '0', 60); //should be 30+2
        PubLong2Char(90L, 2, glSendPack.sField63);

#ifdef ENABLE_EMV
        if( bFirstSettle && !ChkIfBnu() && !ChkIfAmex() && ChkIfEmvEnable() && ChkIfAcqNeedDE56() )
        {// 送F56
            iLength = glSysCtrl.stField56[glCurAcq.ucIndex].uiLength;
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
#endif
        // bit 60
		if( ChkIfIndirectCupAcq())
		{
 #ifdef APP_DEBUG_RICHARD
    PubDebugTx("File_%s,LineNo:%d,Function():%s,,glCurAcq.ulCurBatchNo=%d\n",__FILE__,__LINE__, __FUNCTION__,glCurAcq.ulCurBatchNo);
 #endif
		   sprintf((char *)glSendPack.szField60, "%06lu",glCurAcq.ulCurBatchNo);
		}
		else
		{
			sprintf((char *)glSendPack.szField60, "%06lu", glCurAcq.ulCurBatchNo);
		}

        // bit 62
        if( ChkIfAmex() )
        {
            memcpy((char *)glSendPack.sField62, "\x00\x06", 2);
            sprintf((char *)glSendPack.sField62+2, "%06lu", glSysCtrl.ulInvoiceNo);
        }
        else if (PPDCC_ChkIfDccAcq()) // PP-DCC
        {
            PPDCC_PackField62(SETTLEMENT, &glSendPack);
        }

        // bit 63
        if (PPDCC_ChkIfDccAcq()) // PP-DCC
        {
            memset(&glSendPack.sField63[2], '0', 90);
            sprintf((char *)&glSendPack.sField63[2], "%03d%12.12s%03d%12.12s%03d%03d",
                    (int)glTransTotal.uiSaleCnt,   glTransTotal.szSaleAmt,
                    (int)glTransTotal.uiRefundCnt, glTransTotal.szRefundAmt,
                    PPDCC_GetReversalCnt(),        (int)glTransTotal.uiTipCnt);
            glSendPack.sField63[2+3+12+3+12+3+3] = '0'; // sprintf() caused 0x00 at the end of string
            PubLong2Char(90L, 2, glSendPack.sField63);
        }
        else if( ChkIfAmex() )//2015-1-23 mark AMEX field 63
        {
#if 1
			//build88R: bug fix AMEX settlement
            sprintf((char *)&glSendPack.sField63[2], "%03d%12.12s%03d%12.12s%03d%03d",
                    glTransTotal.uiSaleCnt,   glTransTotal.szSaleAmt,
                    glTransTotal.uiRefundCnt, glTransTotal.szRefundAmt,
					glTransTotal.uiReversalCnt,glTransTotal.uiAdjustCnt);//2015-1-22 add reversal counts & adjustment counts

            memset(&glSendPack.sField63[30+2+6], '0', 60); //should be 30+2
            //end build88R

            // AMEX settlement时, bit63 len = 36 byte
            PubLong2Char(36L, 2, glSendPack.sField63);
            glSendPack.sField63[2+36] = 0;
#else
            //build88R: bug fix AMEX settlement
            sprintf((char *)&glSendPack.sField63[2], "%03d%12.12s%03d%12.12s",
                    glTransTotal.uiSaleCnt,   glTransTotal.szSaleAmt,
                    glTransTotal.uiRefundCnt, glTransTotal.szRefundAmt);

            memset(&glSendPack.sField63[30+2], '0', 60); //should be 30+2
            //end build88R

            // AMEX settlement时, bit63 len = 36 byte
            PubLong2Char(36L, 2, glSendPack.sField63);
            glSendPack.sField63[2+36] = 0;
#endif
        }
        else
        {
            sprintf((char *)&glSendPack.sField63[2], "%03d%12.12s%03d%12.12s",
                    glTransTotal.uiSaleCnt,   glTransTotal.szSaleAmt,
                    glTransTotal.uiRefundCnt, glTransTotal.szRefundAmt);
            memset(&glSendPack.sField63[30+2], '0', 60); //should be 30+2
            PubLong2Char(90L, 2, glSendPack.sField63);
        }

        iRet = SendRecvPacket();

        if( iRet!=0 )
        {
            return iRet;
        }
        if( !bFirstSettle )
        {
            if( memcmp(glRecvPack.szRspCode, "95", 2)==0 )
            {
                memcpy(glRecvPack.szRspCode, "77", 2);
            }
            break;
        }

        if( memcmp(glRecvPack.szRspCode, "95", 2)!=0 )
        {
            break;
        }
        if( glSysCtrl.stField56[glCurAcq.ucIndex].uiLength>0 )
        {
            glSysCtrl.stField56[glCurAcq.ucIndex].uiLength = 0; // erase bit 56
            SaveField56();
        }

        // display error information
        DispResult(ERR_HOST_REJ);
		if( ChkIfAmex() )//2015-1-23 if batch upload occur, Amex need to increase trace number for next settle "0500" request
		{
			GetNewInvoiceNo();
		}
        sprintf((char*)glProcInfo.szSettleMsg, "CLOSED(95)");
        iRet = TransUpLoad();
		
        if( iRet!=0 )
        {
            return iRet;
        }
        bFirstSettle = FALSE;
    }
    if( memcmp(glRecvPack.szRspCode, "77", 2)==0 )
    {
        sprintf((char*)glProcInfo.szSettleMsg, "CLOSED(77 RECONCILE ERROR)");
    }

    if( memcmp(glRecvPack.szRspCode, "00", 2)==0 ||
        (!ChkIfAmex() && memcmp(glRecvPack.szRspCode, "77", 2)==0) )
    {
        if ((glSysCtrl.ucLoginStatus == LOGIN_MODE) && (glSysCtrl.ucCurrAcqIdx != 0xff))
        {
            glSysCtrl.ucLoginStatus = LOGOFF_MODE;
            glSysCtrl.ucCurrAcqIdx = 0xff;
        }
        glSysCtrl.sAcqStatus[glCurAcq.ucIndex] = S_CLR_LOG;
        SaveSysCtrlBase();
        CommOnHook(FALSE);
        ClearRecord(glCurAcq.ucKey);
		if( !ChkIfAmex() )//2015-1-23 if batch upload occur, Amex need to increase trace number for next settle "0500" request
		{
			GetNewInvoiceNo();
		}

        if (PPDCC_ChkIfDccAcq())
        {
            PPDCC_ResetTransStatistic();
        }

        DispPrinting();
        iRet = PrintSettle(PRN_NORMAL);
		
		if( ChkIfAmex() )//2015-1-23 if batch upload occur, Amex need to increase trace number for next settle "0500" request
		{
			GetNewInvoiceNo();
		}
        if (ChkIfWLB())
        {
            iWLBfd = open("WLBBit63.dat", O_RDWR);  //for test
            write(iWLBfd, "11", 2);
            ucWLBBuf[0] = 1;
            ucWLBBuf[1] = 0;
            close(iWLBfd);
        }

        if( iRet!=0 )
        {
            PubDispString(_T("PLEASE REPRINT"), 4|DISP_LINE_CENTER);
            PubWaitKey(3);
            iRet = ERR_NO_DISP;
        }

        ScrClrLine(2,7);
        if( memcmp(glRecvPack.szRspCode, "77", 2)==0 )
        {
            PubDispString(_T("RECONCILE ERROR"), 4|DISP_LINE_CENTER);
        }
        else if( glTransTotal.uiSaleCnt==0 && glTransTotal.uiRefundCnt==0 )
        {
            PubDispString(_T("NO TXN TOTAL"), 4|DISP_LINE_CENTER);
        }
        else
        {
            sprintf((char *)szBuff, "BATCH %06ld", glCurAcq.ulCurBatchNo);
            PubDispString(szBuff,   3|DISP_LINE_CENTER);
            PubDispString("CLOSED", 5|DISP_LINE_CENTER);
        }
        PubBeepOk();
        PubWaitKey(5);

#ifdef SUPPORT_TABBATCH
		glSysCtrl.ucLastTransIsAuth = FALSE;
		SaveSysCtrlBase();

        if (glSysParam.stEdcInfo.ucEnableAuthTabBatch==1)
        {
            ClearTabBatchRecord(glCurAcq.ucKey);
        }
        if (glSysParam.stEdcInfo.ucEnableAuthTabBatch==2)
        {
		    ScrClrLine(2,7);
            PubDispString(_T("DEL TAB BATCH ?"), 4|DISP_LINE_CENTER);
            if (AskYesNo())
            {
                ClearTabBatchRecord(glCurAcq.ucKey);
            }
        }
#endif
        return iRet;
    }
	

    //build88R Patch: show error message
    else if( memcmp(glRecvPack.szRspCode, "00", 2) != 0)
    {
        DispHostRspMsg(glProcInfo.stTranLog.szRspCode, glHostErrMsg);

        PubBeepErr();
        PubWaitKey(5);
    }
    //end show error message

    return 0;
}

// 批上送
int TransUpLoad(void)
{
    int         iRet;
    ushort      uiIndex, uiTranNum, uiCnt;
    uchar       szTotalAmt[12+1], szBuff[128],szBuff1[128], psTemp[1];
    TRAN_LOG    stLog;
    uchar  szAmtBuf[12+1];      //build1.0R

    TransInit(UPLOAD);
    uiTranNum = GetTranLogNum(glCurAcq.ucKey);
    for(uiCnt=uiIndex=0; uiIndex<MAX_TRANLOG; uiIndex++)
    {
        if( glSysCtrl.sAcqKeyList[uiIndex]!=glCurAcq.ucKey )
        {
            continue;
        }
        uiCnt++;
        ScrPrint(70, 0, CFONT|REVER, "%03d/%03d", uiCnt, uiTranNum);

        memset(&stLog, 0, sizeof(TRAN_LOG));
        iRet = LoadTranLog(&stLog, uiIndex);
        if( iRet!=0 )
        {
            return iRet;
        }
        if( stLog.ucTranType==PREAUTH || (stLog.uiStatus & TS_NOT_UPLOAD) )
        {
            continue;
        }
        if( (stLog.uiStatus & TS_VOID) && !ChkIfAmex() )
        {// AMEX 需要upload void，其它acquirer不用
            continue;
        }
        // PPDCC
        if (PPDCC_ChkIfDccAcq() && ChkIfZeroAmt(stLog.szAmount))
        {
            continue;
        }

        SetCommReqField();
		//2015-5-22 change for DahSing
		if (/*stLog.ucTranType == UPLOAD) && */ChkIfDahOrBCM())//dashing need tc share the same stan with original transaction
		//2015-2-3 DahSing already handled the TC issue!!!
		{
			sprintf((char *)glSendPack.szSTAN, "%06lu", stLog.ulSTAN);
			//        glProcInfo.stTranLog.ulSTAN = glSysCtrl.ulSTAN;
		}
        sprintf((char *)glSendPack.szPan, "%.19s", stLog.szPan);
        sprintf((char *)glSendPack.szProcCode, "%.*s", LEN_PROC_CODE,
                glTranConfig[stLog.ucTranType].szProcCode);
        glSendPack.szProcCode[LEN_PROC_CODE-1] = '1';
        if (ChkIfAmex())
        {
            glSendPack.szProcCode[1] = '0';
            glSendPack.szProcCode[2] = '4';
        }

        if (ChkIfAmex() && (stLog.uiStatus & TS_VOID))
        {
            sprintf(glSendPack.szTranAmt, "%012lu", 0L);
            glSendPack.szExtAmount[0] = 0;
        }
        else
        {
            PubAscAdd(stLog.szAmount, stLog.szTipAmount, 12, szTotalAmt);
            //PubAddHeadChars(szTotalAmt, 12, '0');  no need: already 12 digits
            sprintf((char *)glSendPack.szTranAmt, "%.12s", szTotalAmt);
            if ( PPDCC_ChkIfDccAcq() &&
                 (!ChkIfZeroAmt(stLog.szTipAmount) || !ChkIfZeroAmt(stLog.szFrnTip)) ) // HJJ 20111004
            {
                sprintf((char *)glSendPack.szExtAmount, "%.12s%.12s", stLog.szTipAmount, stLog.szFrnTip);
            }
            else if( !ChkIfZeroAmt(stLog.szTipAmount) )
            {
                sprintf((char *)glSendPack.szExtAmount, "%.12s", stLog.szTipAmount);
            }
        }
        if (ChkIfCiti())
        {
            if ((stLog.ucInstalment!=0) && (ChkAcqOption(ACQ_CITYBANK_INSTALMENT_FEATURE)))
            {//Build0120 citybank instalment
                memset(glSendPack.sField48,'0',sizeof(glSendPack.sField48));
                memcpy(glSendPack.sField48, "\x00\x0E", 2);
                glSendPack.sField48[2] = '0';       //tag
                glSendPack.sField48[3] = '1';       //tag
                sprintf((char *)&(glSendPack.sField48[4]), "%03lu", stLog.ucInstalment);
                glSendPack.sField48[7] = '0';
            }
        }

		// bit 48  Gillian 20161031
		if(ChkCurAcqName("AMEX_INST", FALSE))//&& (glProcInfo.stTranLog.ucTranType = INSTALMENT))  //Gillian 20161019
		{
			memcpy(glSendPack.sField48, "\x00\x02", 2);
			PubLong2Bcd((unsigned long)glSysParam.stEdcInfo.szEppType, 1, psTemp);
	
			memcpy(&glSendPack.sField48[2], psTemp, 1);

			memcpy(&glSendPack.sField48[3], glTemp, 1);
		
		}

        // HJJ add 20110928
        if (PPDCC_ChkIfDccAcq())
        {
            // bit 6, 10
            //build88R: bit 6 - cardholder billing should include the foreign tips
            if( !ChkIfZeroAmt(stLog.szFrnTip))
            {
                memset(szAmtBuf, 0, 13);
                PubAscAdd(stLog.szFrnAmount, stLog.szFrnTip, 12, szAmtBuf);
                sprintf((char *)glSendPack.szFrnAmtPP, "%.*s", LEN_FRN_AMT, szAmtBuf);
            }
            else
                //end build88R
            {
                sprintf((char *)glSendPack.szFrnAmtPP, "%.*s", LEN_FRN_AMT, stLog.szFrnAmount);
            }


            sprintf((char *)glSendPack.szDccRatePP, "%.*s", LEN_DCC_RATE, stLog.szDccRate);
            // bit 49
            PubBcd2Asc0(glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, 2, glSendPack.szTranCurcyCode);
            memmove(glSendPack.szTranCurcyCode, glSendPack.szTranCurcyCode+1, strlen(glSendPack.szTranCurcyCode)+1);
            // bit 51
            PubBcd2Asc0(stLog.stHolderCurrency.sCurrencyCode, 2, szBuff);
            sprintf((char *)glSendPack.szHolderCurcyCode, "%.*s", LEN_CURCY_CODE, szBuff+1);
        }
        //BIT 61//build 1.00.0129
        if (stLog.ucDccType==PPTXN_OPTOUT)
        {
            if (ChkIfBea())
            {
                memset(szBuff,0,sizeof(szBuff));
                //strcpy(szBuff,"           ");//after 3 byes length,add 8 more bytes for Product code (can be space)
                strcpy(szBuff,"        ");
                if (stLog.ucDccType==PPTXN_OPTOUT)
                {
                    strcat(szBuff,"Y");
                }
                else
                {
                    strcat(szBuff,"N");
                }
                if (stLog.ucDccType!=PPTXN_OPTOUT)
                {
                    strcat(szBuff,"000000000000000");
                }
                else
                {
                    if (strstr(stLog.szPPDccTxnID,"  ")!=NULL)//MASTERCARD
                    {
                        strncat(szBuff,&stLog.szPPDccTxnID[4],3);
                        strncat(szBuff,&stLog.szPPDccTxnID[7],6);
                        strncat(szBuff,&stLog.szPPDccTxnID[0],4);
                        strcat(szBuff,"  ");
                    }
                    else//visa
                    {
                        strcat(szBuff,stLog.szPPDccTxnID);
                    }
                }
                //sprintf(szBuff1,"%03d",strlen(szBuff));
                //memcpy(szBuff,szBuff1,3);
                strcpy(glSendPack.szField61,szBuff);
            }
        }

        sprintf((char *)glSendPack.szLocalTime, "%.6s", &stLog.szDateTime[8]);
        sprintf((char *)glSendPack.szLocalDate, "%.4s", &stLog.szDateTime[4]);
        sprintf((char *)glSendPack.szExpDate,   "%.4s", stLog.szExpDate);   // [1/11/2007 Tommy]
        SetEntryMode(&stLog);
        sprintf((char *)glSendPack.szCondCode, "%.2s",  stLog.szCondCode);
        sprintf((char *)glSendPack.szRRN,      "%.12s", stLog.szRRN);
        sprintf((char *)glSendPack.szAuthCode, "%.6s",  stLog.szAuthCode);
        if(ChkIfWLB())
        {
            sprintf((char *)glSendPack.szRspCode, "%.2s", stLog.szRspCode);//ChkIfWLB()
        }

        if( !ChkIfAmex() )
        {
            sprintf((char *)glSendPack.szRspCode, "%.2s", stLog.szRspCode);
            if (PPDCC_ChkIfDccAcq())
            {
                if (memcmp(stLog.szRspCode, "Y1", 2)==0) // Y1 is assigned by EMV kernel if offline approved.
                {
                    sprintf((char *)glSendPack.szRspCode, "00");
                }
            }
            if( stLog.uiEntryMode & MODE_CHIP_INPUT )
            {
                PubLong2Char((ulong)stLog.uiIccDataLen, 2, glSendPack.sICCData);
                memcpy(&glSendPack.sICCData[2], stLog.sIccData, stLog.uiIccDataLen);
                //Build 1.00.0112
                if (ChkIfBea())
                {
                    if (memcmp(glProcInfo.stTranLog.szRspCode, "Y1", 2)==0) // Y1 is assigned by EMV kernel if offline approved.
                    {
                        sprintf((char *)glSendPack.szAuthCode, "Y1    ");
                    }
                }
            }
        }

        if (ChkIfWLB() && (stLog.ucTranType == INSTALMENT))
        {
            memcpy(glSendPack.sField62, "\x00\x12", 2);
            sprintf((char *)&(glSendPack.sField62[2]), "%06lu", stLog.ulInvoiceNo);
            sprintf((char *)&(glSendPack.sField62[8]), "%02lu", stLog.ucInstalment);
            memset(&glSendPack.sField62[10], 0x20, 10);
        }
        else if (!PPDCC_ChkIfDccAcq()) // HJJ exclude DE62 for PP-DCC 20110928
        {
            memcpy(glSendPack.sField62, "\x00\x06", 2);
            sprintf((char *)&glSendPack.sField62[2], "%06ld", stLog.ulInvoiceNo);
        }

        if(!ChkIfBnu() && !PPDCC_ChkIfDccAcq()) // HJJ exclude DE60 for PP-DCC 20110928
        {
            memcpy(glSendPack.szField60, glTranConfig[stLog.ucTranType].szTxMsgID, 4);
            if( stLog.uiStatus & TS_OFFLINE_SEND )
            {
                memcpy(glSendPack.szField60, "0220", 4);
                if( ChkAcqOption(ACQ_DBS_FEATURE) )
                {
                    if( stLog.ucTranType!=OFF_SALE && stLog.ucTranType==SALE_COMP &&
                        !(stLog.uiStatus & TS_FLOOR_LIMIT) )
                    {
                        memcpy(glSendPack.szField60, "0200", 4);
                    }
                }
            }
            else if( stLog.uiStatus & TS_VOID )
            {
                memcpy(glSendPack.szField60, "0200", 4);
            }
            sprintf((char *)&glSendPack.szField60[4], "%06ld%12s", stLog.ulSTAN, "");
        }


        if (ChkIfShanghaiCB()||ChkIfICBC() || ChkIfWLB())
        {
            SetEMVTermIndic(&stLog);
        }

        if (PPDCC_ChkIfDccAcq())
        {
            PPDCC_PackField63(UPLOAD, &glSendPack); // HJJ add 20110928
        }
        if (ChkCurAcqName("DSB",FALSE))
        {
            DAHSING_setInstalmentField63(&glSendPack,&stLog);
        }
        iRet = SendRecvPacket();
        if( iRet!=0 )
        {
            return iRet;
        }
        DispResult(0);
    }

    return 0;
}

// end of file

