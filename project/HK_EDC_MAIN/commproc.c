
#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static int  AskModemReDial(int iRet);
static int  GenSendPacket(void);
static int  EncryptMsg(COMM_DATA *pstCommData);//2016-2-29 *** //Gillian 2016-7-14
static int  DecryptMsg(COMM_DATA *pstCommData);//2016-2-29 *** //Gillian 2016-7-14
static int  ProcRecvPacket(void);
static void SaveRecvPackData(void);
static int  AdjustCommParam(void);
static void DispCommErrMsg(int iErrCode);
static uchar AddHttpsHeader_FUBON(uchar *context, ushort *length); //add by richard 20161216
static uchar PhaseHeepsHeader_FUBON();
/*----------------2014-5-20 IP encryption----------------*/
#ifdef IP_ENCRYPT
static int EncryptPack8583(uchar* pusSendData, uint* puiSendLen, EFTSec_Control* stEFTSec_Control);
static int CalEFTSecEncry(uchar* pusSendData, uint* puiSendLen, EFTSec_Control* stEFTSec_Control);
#endif
/*----------------2014-5-20 IP encryption----------------*/
/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/
/*----------------2014-5-20 IP encryption----------------*/
#ifdef IP_ENCRYPT
//tripe des cbc mode dual key Alog by Jerome
//猔種矪pusSendData珹TPUD繷
//return 0 OK, -1 error;
int CalEFTSecEncry(uchar* pusSendData, uint* puiSendLen, EFTSec_Control* stEFTSec_Control)
{
	uchar strMac[8];
	uchar strIV[8];
	//uchar strTemp[8];
	int iRet;
	int ii;
	uint uitemp;
	ulong uiStart;
	ulong uiLength;
	uiStart = stEFTSec_Control->ulEDSStart;
	uiLength = stEFTSec_Control->ulEDSLength;
	uitemp = 12 + uiStart;
	memset(strIV, 0, sizeof(strIV));
	//memset(strTemp, 0, sizeof(strTemp));
	while (uiLength >= 8)
	{
		for (ii=0; ii<8; ii++)
		{
			strMac[ii] = strIV[ii] ^ pusSendData[uitemp + ii];
		}
		iRet = PedCalcDES(stEFTSec_Control->ucKeyIdx, strMac, 8, strIV, 0x01);//0x01 means encrypt -301 KEY not exist
		if (iRet != 0)
		{
			////2014-8-25 ttt
			//	ScrCls();
			//	ScrPrint(0,0,0,"tttt 2: %d",iRet);
			//	ScrPrint(0,2,0,"ucKeyIdx: %d",stEFTSec_Control->ucKeyIdx);//3
			//	ScrPrint(0,4,0,"dataIn: %02x%02x%02x%02x%02x%02x%02x%02x",strMac[0],strMac[1],strMac[2],strMac[3]
			//	,strMac[4],strMac[5],strMac[6],strMac[7]);
			//	ScrPrint(0,6,0,"pusSendData:%02x%02x%02x%02x%02x%02x%02x%02x",pusSendData[22],pusSendData[23],
			//	pusSendData[24],pusSendData[25],pusSendData[26],pusSendData[27],pusSendData[28],pusSendData[29]);
			//	PubWaitKey(7);
			return -1;
		}
		memcpy(pusSendData+uitemp, strIV, 8);
		uitemp += 8;
		uiLength -= 8;
	}

	if (uiLength >0) //8竊0. 琌硂妓糤猔種э硂ㄠ璶э
	{				//ぃ璶ㄏノuitemp,uitemp⊿Τ竒筁while粂

		memmove(pusSendData+12+uiStart+stEFTSec_Control->ulEDSLength + 8 - uiLength, 
			pusSendData+12+uiStart+stEFTSec_Control->ulEDSLength, 
			(*puiSendLen)-stEFTSec_Control->ulEDSLength -12 -uiStart);//盢计沮簿惠璶簿笆ぃ穦7竊

		ii=0;
		while (uiLength--)
		{
			strMac[ii] = strIV[ii] ^ pusSendData[uitemp + ii];
			ii++;
		}
		while (ii<8)
		{
			strMac[ii] = strIV[ii] ^ 0x00;    //attention the pad bytes is 0x00. Is it true?
			ii++;
			//	(*puiSendLen)++;
		}
		iRet = PedCalcDES(stEFTSec_Control->ucKeyIdx, strMac, 8, strIV, 0x01);//0x01 means encrypt
		if (iRet != 0)
		{			
			return -1;
		}
		memcpy(pusSendData+12+uiStart+stEFTSec_Control->ulEDSLength- (stEFTSec_Control->ulEDSLength % 8)
			    , strIV, 8);
	}
	//	*puiOutLen = 8 + stEFTSec_Control->uiEDSLength - (stEFTSec_Control->uiEDSLength % 8);
	*puiSendLen  = (*puiSendLen) + 8 - (stEFTSec_Control->ulEDSLength % 8);
	return 0;
}

//encrypt part of pack8583
//pusSendData琌珹TPDU㎝EDS琌ぃ度琌赣ず甧Τ场だ砆盞τ砆盞计沮琌8计
// *puiSendLen 琌厨ゅ珹TPDU㎝EDSのゼ盞玡Application场だ祘挡*puiSendLen跑璶俱埃8.
//return 0 OK, -1 error;
int EncryptPack8583(uchar* pusSendData, uint* puiSendLen, EFTSec_Control* stEFTSec_Control)
{
	uchar ucChecksum;
	uint uiLength;
	uint uiStart;
	//	uint uiOffSet;
	//	uchar strMac[1024];
	int iRet;
	
	if (!pusSendData)
	{
		return -1;
	}
	/************************************************************************/
	/* generate TPDU pack and EDS                                           */
	/************************************************************************/
	if ((stEFTSec_Control->ulEDSKIN > 0x0ffffff) || //KINぃ禬筁12
		(stEFTSec_Control->ulEDSStart > 0x0ffff) ||
		(stEFTSec_Control->ulEDSLength > 0x0ffff))
	{
		return -1;
	}
  /*!!!raymond: it looks the length is (*puiSendLen)-7-5-2 because message type, TPU and EDC are excluded, please verify
    memmove(pusSendData+12, pusSendData+5, (*puiSendLen)-7-5-2);
  */
	memmove(pusSendData+12, pusSendData+5, (*puiSendLen)-7-5);
	pusSendData[0] = stEFTSec_Control->ucTPDUid;   //TPDU
	/*pusSendData[1] = stEFTSec_Control->strTPDUNII[0];
	pusSendData[2] = stEFTSec_Control->strTPDUNII[1];
	pusSendData[3] = stEFTSec_Control->strTPDUSRC[0];
	pusSendData[4] = stEFTSec_Control->strTPDUSRC[1];
*/	
	
	pusSendData[5] = (stEFTSec_Control->ucEDSControl << 4) | (uchar)(stEFTSec_Control->ulEDSKIN >> 8);
	pusSendData[6] = stEFTSec_Control->ulEDSKIN & 0x0ff;
	
	pusSendData[7] = (stEFTSec_Control->ulEDSStart >> 8) & 0x0ff; //EDSStart
	pusSendData[8] = (uchar)(stEFTSec_Control->ulEDSStart & 0x0ff);
	
	pusSendData[9] = (stEFTSec_Control->ulEDSLength >> 8) & 0x0ff; //EDSLength
	pusSendData[10] = (uchar)(stEFTSec_Control->ulEDSLength & 0x0ff);
	
	ucChecksum = 0;
	uiStart = stEFTSec_Control->ulEDSStart;
	uiLength = stEFTSec_Control->ulEDSLength;
	while (uiLength--)
	{
		ucChecksum += pusSendData[12 + (uiStart++)];
	}//get checksum

	stEFTSec_Control->ucEDSChecksum = ucChecksum;
	pusSendData[11] = ucChecksum;
	
	////2014-10-8 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"EDS2: ");
	//		ScrPrint(0,1,0,"KIN: %d",stEFTSec_Control->ulEDSKIN);
	//		ScrPrint(0,2,0,"Start: %ld",stEFTSec_Control->ulEDSStart);
	//		ScrPrint(0,3,0,"Checksum:%d (%02x)",stEFTSec_Control->ucEDSChecksum, stEFTSec_Control->ucEDSChecksum);
	//		ScrPrint(0,4,0,"Length:%ld",stEFTSec_Control->ulEDSLength);
	//PubWaitKey(7);
	////2014-8-25 ttt
	//			ScrCls();
	//			ScrPrint(0,0,0,"ucChecksum:%d",ucChecksum);//16
 //               ScrPrint(0,2,0,"content2:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",pusSendData[0],pusSendData[1],pusSendData[2],pusSendData[3],
	//			pusSendData[4],pusSendData[5],pusSendData[6],pusSendData[7],pusSendData[8],pusSendData[9],pusSendData[10],
	//			pusSendData[11],pusSendData[12],pusSendData[13],pusSendData[14],pusSendData[15],pusSendData[16],pusSendData[17],pusSendData[18],
	//			pusSendData[19],pusSendData[20],
	//			pusSendData[21],pusSendData[22],pusSendData[23],pusSendData[24],pusSendData[25],pusSendData[26],pusSendData[27],pusSendData[28],
	//			pusSendData[29]);				
	//			PubWaitKey(7);
	//PubDebugOutput("MEMMOVE 2", glSendData.sContent, *puiSendLen+1, DEVICE_PRN, HEX_MODE);//2014-11-20 ttt
	iRet = CalEFTSecEncry(pusSendData, puiSendLen, stEFTSec_Control);
	
	if (iRet != 0)
	{
		return -1;
	}
	
	return 0;
}

#endif
/*----------------2014-5-20 IP encryption----------------*/
// 交易处理
// process transaction
int TranProcess(void)
{
    int		iRet, iRetryPIN;

 #ifdef APP_DEBUG_RICHARD
    PubDebugTx("File_%s,LineNo:%d,CURRENT ACQ:%s\n",__FILE__,__LINE__, glCurAcq.szName);
 #endif

    
#ifdef ENABLE_EMV
    // EMV卡交易处理
	if( (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) &&
        (glProcInfo.stTranLog.ucTranType==SALE || glProcInfo.stTranLog.ucTranType==CASH ||
        glProcInfo.stTranLog.ucTranType==AUTH || glProcInfo.stTranLog.ucTranType==PREAUTH ||
        glProcInfo.stTranLog.ucTranType==INSTALMENT) )
    {
#ifdef APP_DEBUG_RICHARD //added by jeff_xiehuan20170322
	PubDebugTx("FILE_%s,func:%s,LineNo:%d,%s",__FILE__,__FUNCTION__, __LINE__,"Entry EMV Tran flow");
#endif
        return FinishEmvTran();
    }
#endif
   
#ifdef APP_DEBUG_RICHARD
	PubDebugTx("FILE_%s,func:%s,LineNo:%d,%s",__FILE__,__FUNCTION__, __LINE__,"Entry,No EMV Trans");
#endif
    // 判断上次交易是否需要进行冲正等
    iRet = TranReversal();
    if( iRet!=0 )
    {
        return iRet;
    }
	// Some banks may want TC to be sent prior to sale
// 	if(ChkIfCiti())
// 	{
// 		iRet=OfflineSend(OFFSEND_TC);
// 		if(iRet!=0)
// 		{
// 			return iRet;
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
        //    GetNewInvoiceNo();
        //}

        iRet = SendRecvPacket();
        if( iRet!=0 )
        {
            return iRet;
        }
        
        if( memcmp(glRecvPack.szRspCode, "55", 2)!=0 || ++iRetryPIN>3 || !ChkIfNeedPIN() )
        {
            break;
        }
        
        // 重新输入PIN
        iRet = GetPIN(GETPIN_RETRY);
        if( iRet!=0 )
        {
            return iRet;
        }
        sprintf((char *)glSendPack.szSTAN, "%06lu", glSysCtrl.ulSTAN);
        memcpy(&glSendPack.sPINData[0], "\x00\x08", 2);
        memcpy(&glSendPack.sPINData[2], glProcInfo.sPinBlock, 8);
    }
    
    return AfterTranProc();
}

// 交换通讯报文
// exchange package(send request & wait response)
int SendRecvPacket(void)
{
    int		iRet;
    
#ifdef _S60_  
	if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
		|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
	{
		//if(glProcInfo.bIfClssTxn != 1)//2014-9-10 close this port on any condition, in case glProcInfo.bIfClssTxn == 1 but port is open
		{
			ClssClose(); //2014-5-16 enhance
			glProcInfo.bIfClssTxn = 1;
		}
	}
#endif    
	
    iRet = SendPacket();
    //PubDebugTx("\nSendPacket,iRet=:%d\n", iRet);
    if( iRet!=0 )
    {
        return iRet;
    }
    
    // 保存结算状态/冲正数据
    if( glProcInfo.stTranLog.ucTranType==SETTLEMENT )
    {
        glSysCtrl.sAcqStatus[glCurAcq.ucIndex] = S_PENDING;
        SaveSysCtrlBase();
    }
    SaveRevInfo(TRUE);
    
    iRet = RecvPacket();
    if( iRet!=0 )
    {
        return iRet;
    }
    return 0;
}

// 生成发送报文
// generate request package
int GenSendPacket(void)
{
    int		iRet;
    uint	uiLength;
    uchar	szBuff[20], sMacOut[LEN_MAC];
    /*----------------2014-5-20 IP encryption----------------*/
#ifdef IP_ENCRYPT
	  //Jerome
	uint uiLen;
	int ii;
	  //end of Jerome

#endif
/*----------------2014-5-20 IP encryption----------------*/
    // prepare TPDU header
    memset(&glSendData, 0, sizeof(COMM_DATA));
	memset(&glSendDataDebug, 0, sizeof(COMM_DATA));//2014-9-1 
    sprintf((char *)szBuff, "600%3.3s0000", glCurAcq.szNii);

	if (ChkEdcOption(EDC_WINGLUNG_ENCRY))  //Gillian 2016-8-1
	{
		////Gillian 2016-7-14
		if (glProcInfo.stTranLog.ucTranType!=LOAD_PARA)//2016-2-29 ***
		{
			if ((glCommCfg.ucCommType==CT_TCPIP || glCommCfg.ucCommType==CT_3G) && ChkOptionExt(glSysParam.stEdcInfo.sExtOption, EDC_EXT_TCP_ENC))//2015-3-16 IP needs encryption//2016-2-29 ***//2016-4-7 3G encryption
			{
	    		szBuff[1] = '3';
				//Kidx: TMK Index 
			}
		}
	}

    PubAsc2Bcd(szBuff, 10, glSendData.sContent);

/*----------------2014-5-20 IP encryption----------------*/
#ifdef IP_ENCRYPT
    //Jerome
	//﹍てglEFTSec_Control竚Jerome
	
	if (ChkEdcOption(EDC_BEA_IP_ENCRY))  //Gillian 20160923
	{
		if (glCurAcq.ucIsSupportKIN == SupportKIN && glCommCfg.ucCommType==CT_TCPIP) //Gillian 20160923既礚羛诀爹┮璶やKIN碞﹚穦Τ癸莱key,ぃノ耞key seed琌
		{
		/*	if (glCurAcq.ucIsKeyBlank == KeyNotBlank) //既ぃ耞
			{
			}
			*/

			glEFTSec_Control.ulEDSKIN = glCurAcq.ulKIN;
			glEFTSec_Control.ucKeyIdx = glCurAcq.ulKeyIdx;

		/*	glEFTSec_Control.strTPDUNII[0] = 1;
			glEFTSec_Control.strTPDUNII[1] = 1;
			glEFTSec_Control.strTPDUSRC[0] = 8;
			glEFTSec_Control.strTPDUSRC[1] = 8;
			*/
			/*raymond: bug fix 
		glEFTSec_Control.ucTPDUid = 70;*/
			glEFTSec_Control.ucTPDUid = 0x70;
			glEFTSec_Control.ucEDSControl = 0;
		}
	}
	//end of Jerome
#endif
/*----------------2014-5-20 IP encryption----------------*/    
    // generate iso8583 data
    if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
    {
        iRet = PubPack8583(glTMSDataDef, &glTMSSend, &glSendData.sContent[5], &uiLength);
	}
    else
    {
        iRet = PubPack8583(glEdcDataDef, &glSendPack, &glSendData.sContent[5], &uiLength);
    }

    if( iRet<0 )
    {
        Get8583ErrMsg(TRUE, iRet, szBuff);
        PubDispString(szBuff, 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(5);
        return ERR_NO_DISP;
    }

    glSendData.uiLength = (ushort)(uiLength + 5);

    // 如果DEBIT不发MAC
    // If don't need MAC
    if( !ChkIfNeedMac() )
    {
/*----------------2014-5-20 IP encryption----------------*/
#ifdef IP_ENCRYPT
		//Jerome
		if (ChkEdcOption(EDC_BEA_IP_ENCRY))  //Gillian 20160923
		{	
			if (glCurAcq.ucIsSupportKIN == SupportKIN  && glCommCfg.ucCommType==CT_TCPIP)//2014-9-1 only TCP/IP mode need encrypt Gillian 20160923
			{
				iRet = EncryptPack8583(glSendData.sContent, &glEFTSec_Control.ulDataLen, &glEFTSec_Control);
				if (iRet != 0)
				{
					return -1;
				}
				glSendData.uiLength = (ushort)glEFTSec_Control.ulDataLen;  //奇怪，前面的glSendData.uiLength在未加密前总是比实际的大小小8个字节。
			}
			//end of Jerome
		}
#endif
/*----------------2014-5-20 IP encryption----------------*/
        return 0;
    }
    
    // fill mac data
    memset(sMacOut, 0, sizeof(sMacOut));
    // !!!! the MAC key ID (1) should be per acquirer specified.
    // !!!! the algorithm should be per acquirer specified.
    iRet = GetMAC(MAC_ANSIX919, &glSendData.sContent[5], (ushort)(uiLength-LEN_MAC), 1, sMacOut);
    if( iRet!=0 )
    {
        return iRet;
    }
    memcpy(&glSendData.sContent[glSendData.uiLength-LEN_MAC], sMacOut, LEN_MAC);
    
/*----------------2014-5-20 IP encryption----------------*/
#ifdef IP_ENCRYPT
	//Jerome
	if (ChkEdcOption(EDC_BEA_IP_ENCRY))  //Gillian 20160923
	{
		if (glCurAcq.ucIsSupportKIN == SupportKIN && glCommCfg.ucCommType==CT_TCPIP)//2014-9-1 only TCP/IP mode need encrypt Gillian 20160923
		{
			iRet = EncryptPack8583(glSendData.sContent, &glEFTSec_Control.ulDataLen, &glEFTSec_Control);
			if (iRet != 0)
			{
				return -1;
			}
			glSendData.uiLength = (ushort)glEFTSec_Control.ulDataLen;
		}
	}
	//end of Jerome
#endif
/*----------------2014-5-20 IP encryption----------------*/
    return 0;
}

// 预拨号
// pre-connect to host
int PreDial(void)
{
    int		iRet;
    
    if( !glSysParam.stEdcInfo.bPreDial )
    {
        return 0;
    }

#ifdef _S60_  
	if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
		|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
	{
		//if(glProcInfo.bIfClssTxn != 1)//2014-9-10 close this port on any condition, in case glProcInfo.bIfClssTxn == 1 but port is open
		{
			ClssClose(); //2014-5-16 enhance
			glProcInfo.bIfClssTxn = 1;
		}
	}
#endif
	
#if defined(_P60_S1_) || defined(_S_SERIES_)
    if( ChkOnBase()!=0 )
    {
        return 0;
    }
#endif
    
    iRet = AdjustCommParam();
    if( iRet!=0 )
    {
        return iRet;
    }
    
    return CommDial(DM_PREDIAL);
}

// 连接主机
// connect to host
int ConnectHost(void)
{
    int		iRet, MyRet=-1;
    char Result[32]= {0}, Port_INST[4] = "443";

	if(!ChkIfAmex() && !ChkIfDiners())
	{
		if(glSysParam.stTxnCommCfg.ucEnableSSL_URL)   //Gillian 2016-8-18
		{
			memset(Result, 0, sizeof(Result));
		    memset(glSysParam.stTxnCommCfg.szPP_URL, 0, sizeof(glSysParam.stTxnCommCfg.szPP_URL));

            if(ChkIfFubon()) //add by richard 20161209, for fubon S90 3G SSL
            {
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
            else 
            {
    			//Gillian 20160930
    			memcpy(glSysParam.stTxnCommCfg.szPP_URL, "terminal.planetpayment.net", strlen("terminal.planetpayment.net"));
    			MyRet = DnsResolve(glSysParam.stTxnCommCfg.szPP_URL, Result, 32);
    			strcpy(gCurComPara.LocalParam.szRemoteIP_1, Result);
    			strcpy(gCurComPara.LocalParam.szRemoteIP_2, Result);
            }

			if (ChkCurAcqName("DCC_BEA", TRUE))
			{
				strcpy(glCurAcq.stTxnTCPIPInfo[0].szIP, gCurComPara.LocalParam.szRemoteIP_1);
				strcpy(glCurAcq.stTxnTCPIPInfo[1].szIP, gCurComPara.LocalParam.szRemoteIP_1);
			}
			else if (ChkCurAcqName("BEA", FALSE) || ChkCurAcqName("JCB", FALSE))
			{
				strcpy(glCurAcq.stTxnTCPIPInfo[0].szIP, gCurComPara.LocalParam.szRemoteIP_2);
				strcpy(glCurAcq.stTxnTCPIPInfo[1].szIP, gCurComPara.LocalParam.szRemoteIP_2);
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
    while( 1 )
    {
        // 设置通信参数（从ACQ取得IP，电话号码）
        iRet = AdjustCommParam();
        if (iRet!=0)
        {
            if ((glCommCfg.ucCommTypeBak!=CT_NONE) && (glCommCfg.ucCommType!=glCommCfg.ucCommTypeBak))		// switch to next connection
            {
                // 如果第一套通信的参数不存在，则切换到备用通信方式
                glCommCfg.ucCommType = glSysParam.stTxnCommCfg.ucCommTypeBak;
                CommSwitchType(glSysParam.stTxnCommCfg.ucCommTypeBak);
                continue;
            }
            
            if( iRet!=ERR_NO_TELNO )
            {
                DispCommErrMsg(iRet);
                return ERR_NO_DISP;
            }
            return iRet;
        }
        
        // 连接
        kbflush();
#if defined(_P60_S1_) || defined(_S_SERIES_)
		SetOffBase(OffBaseDisplay);
#endif 

    //add by richard for S90 test
#ifdef APP_DEBUG_RICHARD
    PubDebugTx("glCommCfg.ucCommType:%d\n", glCommCfg.ucCommType);
    PubDebugTx("glCommCfg.stPSTNPara.szTelNo:%s\n", glCommCfg.stPSTNPara.szTelNo);
    PubDebugTx("glCommCfg.stWirlessPara.szRemoteIP_1:%s\n", glCommCfg.stWirlessPara.szRemoteIP_1);
    PubDebugTx("glCommCfg.stWirlessPara.szRemotePort_1:%s\n", glCommCfg.stWirlessPara.szRemotePort_1);
#endif    
// #ifdef _S60_ 
// 		if ((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
// 		{
// 			if (glProcInfo.bIfClssTxn != 1)
// 			{
// 				PortClose(glSysParam.stEdcInfo.ucClssComm);
// 				glProcInfo.bIfClssTxn = 1;
// 			}
// 		}
// #endif

        DispDial();
        iRet = CommDial(DM_DIAL);
        PubDebugTx("CommDial:%d\n", iRet);
        if (iRet==0)
        {
            return 0;
        }
        
        // 是否按过取消
        if ((kbhit()==0) && (getkey()==KEYCANCEL))
        {
            return ERR_USERCANCEL;
        }
        
        // 如果第一套通信方式连接失败，则切换到备用通信方式
        if ((glCommCfg.ucCommTypeBak!=CT_NONE) && (glCommCfg.ucCommType!=glCommCfg.ucCommTypeBak))
        {
 			//2016-1-2 switch to dial up! 
			ScrClrLine(2, 7);
			PubDispString("SWITCH TO MODEM", 3|DISP_LINE_CENTER);
			PubDispString("PLEASE WAIT...",   5|DISP_LINE_CENTER);
			ScrPrint(80, 7, ASCII, "LAN:%d", iRet);
			kbflush();
			PubWaitKey(3);

           glCommCfg.ucCommType = glCommCfg.ucCommTypeBak;
            CommSwitchType(glCommCfg.ucCommTypeBak);
            continue;
        }
        
        // 獶Modem 岿粇钡
        if( (iRet & MASK_COMM_TYPE)!=ERR_COMM_MODEM_BASE )
        {
            DispCommErrMsg(iRet);
            return ERR_NO_DISP;
        }
        
        // Modem 岿粇高拜琌挤
        if (AskModemReDial(iRet))
        {
            return ERR_USERCANCEL;
        }
    }
    
    return 0;
}

int AskModemReDial(int iRet)
{
    uchar	ucKey;
    
    if( iRet==ERR_COMM_MODEM_OCCUPIED || iRet==ERR_COMM_MODEM_NO_LINE ||
        iRet==ERR_COMM_MODEM_NO_LINE_2 )
    {
        DispCommErrMsg(iRet);
    }
    else if( iRet==ERR_COMM_MODEM_LINE || iRet==ERR_COMM_MODEM_NO_ACK ||
        iRet==ERR_COMM_MODEM_LINE_BUSY )
    {
        DispResult(iRet);
        ScrClrLine(2, 7);
        PubDispString(_T("RE DIAL ?    "), 4|DISP_LINE_LEFT);
    }
    else
    {
        PubBeepErr();
        ScrClrLine(2, 7);
        PubDispString(_T("DIAL FAIL,RETRY?"), 4|DISP_LINE_LEFT);
        ScrPrint(114, 7, ASCII, "%02X", (uchar)(iRet & MASK_ERR_CODE));
    }
    ucKey = PubWaitKey(USER_OPER_TIMEOUT/2);
    if( ucKey==KEYCANCEL || ucKey==NOKEY )
    {
        return ERR_USERCANCEL;
    }
    
    return 0;
}

// 发送报文
// send request package
int SendPacket(void)
{
    int		iRet;

#ifdef APP_DEBUG_RICHARD
    int i;
    uchar szSendBuf[1024] = {0};
#endif    
    
    iRet = GenSendPacket();
    if( iRet!=0 )
    {
        return iRet;
    }
#if 1  //debug test by jeff_xiehuan
    iRet = ConnectHost();
    PubDebugTx("ConnectHost:%d\n", iRet);
    if( iRet!=0 )
    {
        return iRet;
    }
#endif
	if (ChkEdcOption(EDC_WINGLUNG_ENCRY))  //Gillian 2016-8-1
	{
		//2016-2-29 *** 
		//Gillian 2016-7-14
		if (glProcInfo.stTranLog.ucTranType!=LOAD_PARA)
		{
			if ((glCommCfg.ucCommType==CT_TCPIP || glCommCfg.ucCommType==CT_3G) && ChkOptionExt(glSysParam.stEdcInfo.sExtOption, EDC_EXT_TCP_ENC))
			{
				PubDebugOutput("TXN PLAIN", glSendData.sContent, glSendData.uiLength, DEVICE_PRN, ISO_MODE);//2016-4-14 tttttt
				iRet = EncryptMsg(&glSendData);//2016-2-29 ***
				if( iRet!=0 )
				{
					//Disp2String("盞岿粇", "ERROR ENCRYPT.");
					ScrClrLine(2, 7);
					PubShowMsg(3, (uchar *)"盞岿粇");
					PubShowMsg(5, (uchar *)"ERROR ENCRYPT.");
					PubBeepErr();
					PubWaitKey(5);
					return ERR_NO_DISP;
				}
				PubDebugOutput("TXN ENCPT", glSendData.sContent, glSendData.uiLength, DEVICE_PRN, HEX_MODE);//2016-4-14 ttt
			}
		}
	}

    DispSend();
    if( glCommCfg.ucCommType==CT_TCPIP ||
        glCommCfg.ucCommType==CT_GPRS  ||
        glCommCfg.ucCommType==CT_CDMA  ||
		glCommCfg.ucCommType==CT_3G )//2015-11-23
    {
        //add https header, by richard 20161216
        if(ChkIfFubon() && glCommCfg.ucCommType==CT_3G && glSysParam.stTxnCommCfg.ucEnableSSL_URL) //ChkIfFubon() && 
        {
            AddHttpsHeader_FUBON(glSendData.sContent, &glSendData.uiLength);
            //PhaseHeepsHeader_FUBON();
        }
        else 
        {
            memmove(&glSendData.sContent[2], glSendData.sContent, glSendData.uiLength);
            if (glCommCfg.ucTCPClass_BCDHeader)
            {// BCD 格式的长度字节
                PubLong2Bcd((ulong)glSendData.uiLength,  2, glSendData.sContent);
            } 
            else
            {
                PubLong2Char((ulong)glSendData.uiLength, 2, glSendData.sContent);
            }
            glSendData.uiLength += 2;
        }
    }

	memcpy(&glSendDataDebug,&glSendData,sizeof(glSendData));//2014-9-1
	////2015-9-22 ttt Gillian debug
//#ifdef Gillian debug
/*	PubDebugOutput((uchar *)_T("SEND MSG"), glSendData.sContent, glSendData.uiLength,
                        DEVICE_COM1, ISO_MODE); //Gillian debug

	PubDebugOutput((uchar *)_T("HEX SEND MSG"), glSendData.sContent, glSendData.uiLength,
                        DEVICE_PRN, HEX_MODE); //Gillian debug*/
//#endif
	//PubDebugOutput((uchar *)_T("SEND MSG1"), glSendData.sContent, glSendData.uiLength,
 //                      DEVICE_PRN, ISO_MODE);
#ifdef APP_DEBUG_RICHARD
   /* for(i=0; i<glSendData.uiLength; i++)
        sprintf((uchar*)(szSendBuf+3*i), "%02X ", glSendData.sContent[i]);
    PubDebugTx("glSendData:\n%s\n\n", szSendBuf);*/
	//PubDebugTx("\nFILE_%s,func:%s,LineNo:%d,%s\n",__FILE__,__FUNCTION__, __LINE__,"send");
	PubDebugOutput("\nsend data",glSendData.sContent,glSendData.uiLength,DEVICE_COM1,HEX_MODE);
	 DelayMs(100);
	PubDebugOutput("\nsend data",glSendData.sContent,glSendData.uiLength,DEVICE_COM1,ISO_MODE);

#endif
    iRet = CommTxd(glSendData.sContent, glSendData.uiLength, USER_OPER_TIMEOUT);	// 不能用 no timeout
    if( iRet!=0 )
    {
        DispCommErrMsg(iRet);	// !!!! 转移到CommTxd
        return ERR_NO_DISP;
    }
    GetNewTraceNo();
    
    return 0;
}

// 接收报文
int RecvPacket(void)
{
    int		iRet;
    ushort	uiTimeOut;
    ulong	ulTemp;
#ifdef APP_DEBUG_RICHARD
    int i;
    uchar szRecvBuf[2048] = {0};
#endif    

    
_RE_RECEIVE:
    DispReceive();
    uiTimeOut = glCurAcq.ucPhoneTimeOut;	// !!!! should according to comm type
    memset(&glRecvData, 0, sizeof(COMM_DATA));
	memset(&glRecvDataDebug, 0, sizeof(COMM_DATA));//2014-9-1
    iRet = CommRxd(glRecvData.sContent, LEN_MAX_COMM_DATA, uiTimeOut, &glRecvData.uiLength);
#ifdef APP_DEBUG_RICHARD
	PubDebugTx("FILE_%s,func:%s,LineNo:%d,CommRxd iRet=:%d,RecvLen=%d\n",__FILE__,__FUNCTION__, __LINE__,iRet,glRecvData.uiLength);
#endif

    //PubDebugTx("CommRxd:%d %d\n", iRet, glRecvData.uiLength);
#ifdef APP_DEBUG_RICHARD
        /*for(i=0; i<glRecvData.uiLength; i++)
            sprintf(szRecvBuf+i*3, "%02X ", glRecvData.sContent[i]);
		PubDebugTx("FILE_%s,func:%s,LineNo:%d,glRecvData:\n%s\n",__FILE__,__FUNCTION__, __LINE__,szRecvBuf);*/
			//PubDebugTx("FILE_%s,func:%s,LineNo:%d,%s\n",__FILE__,__FUNCTION__, __LINE__,"receive");
			PubDebugOutput("receive data_1:",glRecvData.sContent,glRecvData.uiLength,DEVICE_COM1,HEX_MODE);
			DelayMs(100);
			PubDebugOutput("receive data_1:",glRecvData.sContent,glRecvData.uiLength,DEVICE_COM1,ISO_MODE);
       // PubDebugTx("glRecvData:\n%s\n\n", szRecvBuf);
#endif

	/*	PubDebugOutput((uchar *)_T("HEX RECV MSG"), glRecvData.sContent, glRecvData.uiLength,
                       DEVICE_PRN, HEX_MODE); //Gillian debug
	PubDebugOutput((uchar *)_T("ISO RECV MSG"), glRecvData.sContent, glRecvData.uiLength,
                       DEVICE_PRN, ISO_MODE); //Gillian debug*/
    if( iRet!=0 )
    {
        // PP-DCC
		PPDCC_UpdateReverStatistic();

        DispCommErrMsg(iRet);	//!!!! 转移到CommRxd
        return ERR_NO_DISP;
    }
    
    if( glCommCfg.ucCommType==CT_TCPIP ||
        glCommCfg.ucCommType==CT_GPRS  ||
        glCommCfg.ucCommType==CT_CDMA  ||
		glCommCfg.ucCommType==CT_3G )//2015-11-23
    {

        //add by richard 20161216, for https header parse.
        if(ChkIfFubon() && glCommCfg.ucCommType==CT_3G && glSysParam.stTxnCommCfg.ucEnableSSL_URL)
        {
            PhaseHeepsHeader_FUBON();

            memset(&glRecvData, 0, sizeof(COMM_DATA));
            iRet = CommRxd(glRecvData.sContent, LEN_MAX_COMM_DATA, uiTimeOut, &glRecvData.uiLength);
#ifdef APP_DEBUG_RICHARD
           /* for(i=0; i<glRecvData.uiLength; i++)
                sprintf((uchar*)(szRecvBuf+3*i), "%02X ", glRecvData.sContent[i]);
			PubDebugTx("FILE_%s,func:%s,LineNo:%d,glRecvData:\n%s\n",__FILE__,__FUNCTION__, __LINE__,szRecvBuf);*/
			//PubDebugTx("FILE_%s,func:%s,LineNo:%d,%s\n",__FILE__,__FUNCTION__, __LINE__,"receive");
			PubDebugOutput("receive data_2:",glRecvData.sContent,glRecvData.uiLength,DEVICE_COM1,HEX_MODE);
			DelayMs(100);
			PubDebugOutput("receive data_2:",glRecvData.sContent,glRecvData.uiLength,DEVICE_COM1,ISO_MODE);
            //PubDebugTx("glRecvData:\n%s\n\n", szRecvBuf);
#endif  
            SxxTcpOnHook();
        }
        else 
        {
            if (glCommCfg.ucTCPClass_BCDHeader)
            {// BCD 格式的长度字节
                PubBcd2Long(glRecvData.sContent,  2, &ulTemp);
            }
            else
            {
                PubChar2Long(glRecvData.sContent, 2, &ulTemp);
            }

            if( ulTemp+2!=(ulong)glRecvData.uiLength )
            {
                ScrClrLine(2, 7);
                PubDispString(_T("RECV DATA ERR1"), 4|DISP_LINE_LEFT);
                PubBeepErr();
                PubWaitKey(2);
                return ERR_NO_DISP;
            }
            memmove(glRecvData.sContent, &glRecvData.sContent[2], (uint)ulTemp);
            glRecvData.uiLength = (ushort)ulTemp;
        }
    }
    
    memcpy(&glRecvDataDebug,&glRecvData,sizeof(glSendData));//2014-9-1
    if (glCommCfg.ucCommType==CT_DEMO)
    {
        iRet = Simu_CreatDummyRecvData(&glProcInfo, &glRecvPack);
        return iRet;
    }
	
	
	if (ChkEdcOption(EDC_WINGLUNG_ENCRY)) //Gillian 2016-8-1
	{
		//Gillian 2016-7-14
		//2016-2-29 ***
		if (glProcInfo.stTranLog.ucTranType!=LOAD_PARA)
		{
			if ((glCommCfg.ucCommType==CT_TCPIP || glCommCfg.ucCommType==CT_3G) && ChkOptionExt(glSysParam.stEdcInfo.sExtOption, EDC_EXT_TCP_ENC))
			{
				//PubDebugOutput("RECV ENCPT", glRecvData.sContent, glRecvData.uiLength, DEVICE_PRN, HEX_MODE);//2016-4-14 ttt
				iRet = DecryptMsg(&glRecvData);//2016-2-29 ***
				if( iRet!=0 )
				{
					//Disp2String("盞岿粇", "ERROR ENCRYPT.");
					ScrClrLine(2, 7);
					PubShowMsg(3, (uchar *)"秆盞岿粇");
					PubShowMsg(5, (uchar *)"ERROR DECRYPT.");
					PubBeepErr();
					PubWaitKey(5);
					return ERR_NO_DISP;
				}
				//PubDebugOutput("RECV PLAIN", glRecvData.sContent, glRecvData.uiLength, DEVICE_PRN, ISO_MODE);//2016-4-14 ttt
			}
		}
	}

    iRet = ProcRecvPacket();
    if (FALSE)
    {
        goto _RE_RECEIVE;   // reserved for DahSing "0830" message type -- waiting prolong message
    }
    if( iRet!=0 )
    {
        return iRet;
    }
//    //2014-11-5 ttt 
//#ifdef Gillian debug
/*	PubDebugOutput((uchar *)_T("HEX RECV MSG"), glRecvData.sContent, glRecvData.uiLength,
                       DEVICE_PRN, HEX_MODE); //Gillian debug
	PubDebugOutput((uchar *)_T("ISO RECV MSG"), glRecvData.sContent, glRecvData.uiLength,
                       DEVICE_PRN, ASC_MODE); //Gillian debug*/
//#endif
	//PubDebugOutput((uchar *)_T("RECV MSG2"), glRecvData.sContent, glRecvData.uiLength,
 //                     DEVICE_PRN, ISO_MODE);
    return 0;
}


//Gillian 2016-7-14
//2016-2-29 add line encryption for Wing Lung
#ifdef ENC_USE_TESTKEY
//uchar  sMasterKey[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38};/*{0x1A, 0xF3, 0xD5, 0x89, 0xB6, 0x95, 0xA8, 0xAD};*/
uchar  sMasterKey[] = {0x1A, 0xF3, 0xD5, 0x89, 0xB6, 0x95, 0xA8, 0xAD}; //add  by richard 20161221
#else
//uchar  sMasterKey[] = {0x1a, 0xf3, 0xd5, 0x89, 0xb6, 0x95, 0xa8, 0xad, 0x25, 0x67, 0x42, 0x1b, 0xf6, 0xfa, 0xa6, 0x7c};
uchar  sMasterKey[] = {0xBC, 0xA6, 0x5C, 0x7A, 0x89, 0xE3, 0x3F, 0x1d};
#endif

int EncryptMsg(COMM_DATA *pstCommData)//2016-2-29 ***
{
	ushort	uiISOLen, uiPadLen, uiTotalLen;
	uchar	sTime[16], sTmpBuff[32];
	uchar	*pISOMsg, *pFinalKey;
	int		ii;
	uchar	*pMsgPtr;
	ushort	uiOrgLen;
	//	ushort	uiNewLen;

#if 0
	PubDebugOutput("TXD PLAIN", pstCommData->sContent, pstCommData->uiLength, DEVICE_PRN, ISO_MODE);
#endif

	if (pstCommData->uiLength > LEN_MAX_COMM_DATA-2-2-16)
	{
		return -1;
	}

	// ﹍厨ゅ
	pMsgPtr  = pstCommData->sContent;
	uiOrgLen = pstCommData->uiLength;

	//璸衡ISO,の惠璶pad,舱8计
	uiISOLen = uiOrgLen - 5;
	uiPadLen = (8-(uiISOLen%8)) % 8;
	// !!! pad 8 bytes even when multiple of 8
	if (uiPadLen==0)
	{
		uiPadLen = 8;
	}

	// Locate the ISO message / key segment start address
	pISOMsg   = pMsgPtr + 5; 
	pFinalKey = pMsgPtr + 5 + 2 + uiISOLen + uiPadLen;

#if 1
	// insert ISO length
	memmove(pISOMsg+2, pISOMsg, uiISOLen);
	//PubLong2Char((ulong)(uiISOLen+5), 2, pISOMsg);
	pISOMsg[0] = (uiISOLen+5) / 256;
	pISOMsg[1] = (uiISOLen+5) % 256;
	pISOMsg += 2;
#else
	// insert ISO length
	// 沮ゅ郎Original Length琌础TPDUぇ玡τぃ琌ISO8583玡
	// Msg俱砰笆ㄢ竊
	memmove(pMsgPtr+2, pMsgPtr, uiOrgLen);
	PubLong2Char((ulong)uiOrgLen, 2, pMsgPtr);
	pISOMsg += 2;
#endif

	// New total length after insert and pad and attach key
	uiTotalLen = 5 + 2 + uiISOLen + uiPadLen +16;

	// random create Session Key seed
#ifdef _S_SERIES_
	PciGetRandom(glProcInfo.sSessionKey);
	PciGetRandom(glProcInfo.sSessionKey + 8);
#else
	for (ii=0; ii<16; ii++)
	{
		glProcInfo.sSessionKey[ii] = (uchar)rand();
	}
#endif

	//籔讽玡丁暗des,ㄏ痷タ繦诀て
	GetTime(sTime);
	memset(sTmpBuff, 0, sizeof(sTmpBuff));
	des(glProcInfo.sSessionKey,   sTmpBuff,   sTime, ENCRYPT);
	des(glProcInfo.sSessionKey+8, sTmpBuff+8, sTime, DECRYPT);
	memcpy(glProcInfo.sSessionKey, sTmpBuff, 16);

#ifdef ENC_USE_TESTKEY
	PubAsc2Bcd("11111111111111112222222222222222", 32, glProcInfo.sSessionKey);
	//    sprintf(sTmpBuff, "ISO LEN=%d, PAD=%d\n", (int)uiISOLen, (int)uiPadLen);
	//    PubDebugOutput(NULL, sTmpBuff, strlen(sTmpBuff), DEVICE_PRN, ASC_MODE);
#endif

	// pad to end of ISO
	for (ii=0; ii<uiPadLen; ii++)
	{
		*(pISOMsg + uiISOLen + ii) = 0x2A;
	}

	// Encrypt
	for (ii=0; ii< (uiISOLen+uiPadLen)/8; ii++)
	{
		if (ii>0)
		{
			PubXor(pISOMsg+8*ii, pISOMsg+8*(ii-1), 8,pISOMsg+8*ii);
		}

		memset(sTmpBuff, 0, sizeof(sTmpBuff));
		PubDes(TRI_ENCRYPT, pISOMsg+8*ii, glProcInfo.sSessionKey, sTmpBuff);
		memcpy(pISOMsg+8*ii, sTmpBuff, 8);
	}

	// Attache Final Key. 玡8竊琌盞
	memset(sTmpBuff, 0, sizeof(sTmpBuff));
	des(glProcInfo.sSessionKey, sTmpBuff, sMasterKey, ENCRYPT);
	memcpy(pFinalKey,   sTmpBuff, 8);
	memcpy(pFinalKey+8, glProcInfo.sSessionKey+8, 8);

	pstCommData->uiLength = uiTotalLen;
	return 0;
}

int DecryptMsg(COMM_DATA *pstCommData)//2016-2-29 ***
{
	//计沮len(2)+TPUD(5)+ISO(8计)+FinalKey(16)
	int		ii;
	uchar	sTmpBuff[32];
	uchar	*pISOMsg;
	ushort	uiISOLen, uiPadLen;

	// debug : print message here

	//璸衡
	uiISOLen = pstCommData->sContent[5]*256 + pstCommData->sContent[6];	// 0xFF.ぐ或ぃ琌256?
	uiPadLen = (8-(uiISOLen%8)) % 8;

	//惠璶璸衡皐
	pISOMsg = pstCommData->sContent + 2 + 5;

	//秆盞ISO8583
	for (ii=(uiISOLen+uiPadLen)/8-1; ii>=0; ii--)
	{
		PubDes(TRI_DECRYPT, pISOMsg+8*ii, glProcInfo.sSessionKey, sTmpBuff);
		memcpy(pISOMsg+8*ii, sTmpBuff, 8);

		if (ii>0)
		{
			PubXor(pISOMsg+8*ii, pISOMsg+8*(ii-1), 8,pISOMsg+8*ii);
		}
	}

	//簿
	memmove(pISOMsg-2, pISOMsg, uiISOLen);
	pstCommData->uiLength = uiISOLen;
	return 0;
}

// 处理接收包，并检查公共数据要素
// Process receive packet content
int ProcRecvPacket(void)
{
    int		iRet;
    long	lTemp1, lTemp2;
    uchar	bFailFlag, szBuff[20];
    
    DispProcess();
    // 检查接收到的数据长度(TPDU+MTI+Bitmap+stan+tid)
    // Check received length
    if( glRecvData.uiLength<5+2+8+8+3 )
    {
        ScrClrLine(2, 7);
        PubDispString(_T("RECV DATA ERR2"), 4|DISP_LINE_LEFT);
        PubBeepErr();
#ifdef APP_DEBUG
		kbflush();
		ScrPrint(1,6,1,"Recv Length %d",(int)glRecvData.uiLength);
		PubDebugOutput((uchar *)_T("ProcRecvPacket VIEW MSG"), glSendData.sContent, glSendData.uiLength,
			DEVICE_PRN, ISO_MODE); //Gillian debug
	/*	PubDebugOutput((uchar *)_T("VIEW MSG"), glRecvData.sContent, glRecvData.uiLength,
					DEVICE_PRN, ISO_MODE);*/
		getkey();
#endif
        PubWaitKey(5);
        return ERR_NO_DISP;
    }
    bFailFlag = FALSE;
    // 处理TPDU(5)
    // Processing TPDU
    if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
    {
        memset(&glTMSRecv, 0, sizeof(STTMS8583));
        iRet = PubUnPack8583(glTMSDataDef, &glRecvData.sContent[5], glRecvData.uiLength-5, &glTMSRecv);
    }
    else
    {
        memset(&glRecvPack, 0, sizeof(STISO8583));
        iRet = PubUnPack8583(glEdcDataDef, &glRecvData.sContent[5], glRecvData.uiLength-5, &glRecvPack);
    }
    if( iRet<0 )
    {
        Get8583ErrMsg(FALSE, iRet, szBuff);
        ScrClrLine(2, 7);
        PubDispString(szBuff, 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(5);
        return ERR_NO_DISP;
    }
    
    // 检查响应数据包的要素并与请求包匹配
    // Check message type
    if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
    {
        lTemp1 = atol((char *)glTMSSend.szMsgCode);
        lTemp2 = atol((char *)glTMSRecv.szMsgCode);
    }
    else
    {
        lTemp1 = atol((char *)glSendPack.szMsgCode);
        lTemp2 = atol((char *)glRecvPack.szMsgCode);
    }

    if (lTemp2==830)
    {
        // 0830 is a "please wait" message
        if (ChkIfDahOrBCM())
        {
            // ????
        }
    } 
    else if( lTemp2!=(lTemp1+10) )
    {
        ScrClrLine(2, 7);
        PubDispString(_T("MSG ID MISMATCH"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(5);
        return ERR_NO_DISP;
    }    
    
    // 保存请求/响应数据
    // Save received response packet.
    SaveRecvPackData();
    
    // 检查处理码
    // Check processing code
    if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
    {
        if( (glTMSSend.szProcCode[0]!=0) &&
            (memcmp(glTMSSend.szProcCode, glTMSRecv.szProcCode, LEN_PROC_CODE-1)!=0) )
        {
            bFailFlag = TRUE;
        }
    }
    else
    {
        if( (glSendPack.szProcCode[0]!=0) &&
            (memcmp(glSendPack.szProcCode, glRecvPack.szProcCode, LEN_PROC_CODE-1)!=0) )
        {
            bFailFlag = TRUE;
        }
    }
    if( bFailFlag )
    {
        ScrClrLine(2, 7);
        PubDispString(_T("BIT 3 MISMATCH"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(5);
        return ERR_NO_DISP;
    }
    
    // 检查STAN
    // Check STAN (Sys Trace Audit Number, bit11)
    if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
    {
        if( glTMSSend.szSTAN[0]!=0 &&
            memcmp(glTMSSend.szSTAN, glTMSRecv.szSTAN, LEN_STAN)!=0 )
        {
            bFailFlag = TRUE;
        }
    }
    else
    {
        if (ChkIfICBC_MACAU() && (glProcInfo.stTranLog.ucTranType==UPLOAD))
        {
            // skip checking.
        }
        else if( glSendPack.szSTAN[0]!=0 &&
            memcmp(glSendPack.szSTAN, glRecvPack.szSTAN, LEN_STAN)!=0 )
        {
            bFailFlag = TRUE;
        }
    }
    if( bFailFlag )
    {
        ScrClrLine(2, 7);
        PubDispString(_T("STAN MISMATCH"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(5);
        return ERR_NO_DISP;
    }
    
    // 检查终端号/商户号
    // Check TID, MID
    if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
    {
        if( memcmp(glTMSSend.szTermID, glTMSRecv.szTermID, LEN_TERM_ID)!=0 )
        {
            bFailFlag = TRUE;
        }
    }
    else
    {
        if( memcmp(glSendPack.szTermID, glRecvPack.szTermID, LEN_TERM_ID)!=0 )
        {
            bFailFlag = TRUE;
        }
    }
    if( bFailFlag )
    {
        ScrClrLine(2, 7);
        PubDispString(_T("TID MISMATCH"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(5);
        return ERR_NO_DISP;
    }
    
    // More basic checks can be placed here.
    
    return 0;
}

// 保存接收报文要素到交易信息结构
// save some data element from receive packet to transaction data.
void SaveRecvPackData(void)
{
    int iRet;
    uchar	szLocalTime[14+1];
    
    if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
    {
        sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", glTMSRecv.szRspCode);
        return;
    }
    
    if( (glProcInfo.stTranLog.ucTranType!=OFFLINE_SEND) &&
        (glProcInfo.stTranLog.ucTranType!=TC_SEND) )
    {
        sprintf((char *)glProcInfo.stTranLog.szRspCode, "%.2s", glRecvPack.szRspCode);
    }
    if( glProcInfo.stTranLog.ucTranType==SETTLEMENT ||
        glProcInfo.stTranLog.ucTranType==UPLOAD     ||
        glProcInfo.stTranLog.ucTranType==REVERSAL ||
		glProcInfo.stTranLog.ucTranType == TC_SEND)
    {
        return;
    }
    
    UpdateLocalTime(NULL, glRecvPack.szLocalDate, glRecvPack.szLocalTime);
    
    PubGetDateTime(szLocalTime);
    sprintf((char *)glProcInfo.stTranLog.szDateTime, "%.14s", szLocalTime);
    
    if( glRecvPack.szAuthCode[0]!=0 )
    {
        sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%.6s", glRecvPack.szAuthCode);
    }
    if( glProcInfo.stTranLog.szRRN[0]==0 )
    {
        sprintf((char *)glProcInfo.stTranLog.szRRN, "%.12s", glRecvPack.szRRN);
    }
    sprintf((char *)glProcInfo.stTranLog.szCondCode,  "%.2s",  glSendPack.szCondCode);

    // PP-DCC
    if (PPDCC_ChkIfDccAcq())
    {
        //get RRN from PP for REFUND trans
        if(glProcInfo.stTranLog.ucTranType == REFUND)
        {
          sprintf((char *)glProcInfo.stTranLog.szRRN, "%.12s", glRecvPack.szRRN);
        }

        if (strlen(glRecvPack.szFrnAmtPP))
        {
    //build88S: The returned szFrnAmtPP includes the tips 
          if((!ChkIfZeroAmt(glProcInfo.stTranLog.szFrnAmount)) && (!ChkIfZeroAmt(glProcInfo.stTranLog.szFrnTip)))
          {          
          }
          else 
    //end build88S
          {
            sprintf((char *)glProcInfo.stTranLog.szFrnAmount, "%.12s", glRecvPack.szFrnAmtPP);
          }
        }        
        else
        {
            PPDCC_CalForeignAmount(glProcInfo.stTranLog.szAmount, glRecvPack.szDccRatePP, glProcInfo.stTranLog.szFrnAmount);
#ifdef AMT_PROC_DEBUG
			//2014-9-18 each time come to glAmount or szAmount
			sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(35)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(35)%.13s",glProcInfo.stTranLog.szAmount);
			glProcInfo.stTranLog.cntAmt++;
	//		//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 9: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
        }

        if (strlen((char *)glRecvPack.szDccRatePP))
        {
            sprintf(glProcInfo.stTranLog.szDccRate, "%.*s", (int)LEN_DCC_RATE, glRecvPack.szDccRatePP);
        }

        iRet = PPDCC_RetriveAllPDS(glRecvPack.sField63, &glProcInfo);
    }
    
    FindCurrency(glRecvPack.szHolderCurcyCode, &glProcInfo.stTranLog.stHolderCurrency);
}

int AfterTranProc(void)
{
    int		iRet;
    int		iTransResult;
	char    cAction;
	uchar   bCount;
    uchar   sCurrencyCode[2];
    uchar   szAmount[12+1];
	uchar ucStateFlag;
    
    if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
    {
        if( memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)!=0 )
        {
            DispResult(ERR_HOST_REJ);
            return ERR_NO_DISP;
        }
        
        DispResult(0);
        return 0;
    }
    
    iTransResult = ERR_HOST_REJ;
    if ( memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)==0 )
    {
        iTransResult = 0;
    }
    if ( (memcmp(glProcInfo.stTranLog.szRspCode, "08", 2)==0) ||
        (memcmp(glProcInfo.stTranLog.szRspCode, "88", 2)==0) )
    {
	
        if (ChkIfAmex())
        {
            if ((glProcInfo.stTranLog.ucTranType==SALE) ||
                (glProcInfo.stTranLog.ucTranType==AUTH) ||
                (glProcInfo.stTranLog.ucTranType==PREAUTH) )
            {
                iTransResult = 0;
            }
        }

    }
    
  //Build88S 1.0D - ADD Ivy 20120201: 
  //if (PPDCC_ChkIfDccAcq())	 
  if ((PPDCC_ChkIfDccAcq()) && (glProcInfo.stTranLog.ucDccType == PPTXN_DCC))	 
  {
		if((strncmp(glProcInfo.stTranLog.szRspCode, "03", 2) == 0) ||
			(strncmp(glProcInfo.stTranLog.szRspCode, "96", 2) == 0))
		{
			glProcInfo.stTranLog.ucDccType = PPTXN_DOMESTIC;

      if(DccSwitchDomestic() == 0)
			{
				glProcInfo.stTranLog.stTranCurrency = glSysParam.stEdcInfo.stLocalCurrency;
				DispResult(ERR_HOST_REJ);
				return ERR_NEED_DOMESTIC;
			}
		}	 
  }
  //end build88S 1.0D

    
    // 交易失败处理
    if (iTransResult)
    {
        CommOnHook(FALSE);
        SaveRevInfo(FALSE);
        
        DispResult(ERR_HOST_REJ);
        if(memcmp(glProcInfo.stTranLog.szRspCode, "01", 2)==0 ||
            memcmp(glProcInfo.stTranLog.szRspCode, "02", 2)==0 ||
            (glProcInfo.stTranLog.ucInstalment!=0 && ChkAcqOption(ACQ_BOC_INSTALMENT_FEATURE)) )
        {
            // BOC 银行instalment交易时假如交易结果不是"00",都进入referrel交易
            iRet = TranSaleComplete();
            if( iRet==0 )
            {
                return 0;
            }
        }
        return ERR_NO_DISP;
    }
	if (ChkIfWLB())
	{
		if (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE )//for test
		{	
			iWLBfd = open("WLBBit63.dat", O_RDWR);
			write(iWLBfd, "21", 2);
			ucWLBBuf[0] = 2;
			ucWLBBuf[1] = 1;
			close(iWLBfd);
		}
		else
		{
			iWLBfd = open("WLBBit63.dat", O_RDWR);
			write(iWLBfd, "11", 2);
			ucWLBBuf[0] = 1;
			ucWLBBuf[1] = 1;
			close(iWLBfd);
		}
	}


#ifdef SUPPORT_TABBATCH
	ucStateFlag = glSysCtrl.ucLastTransIsAuth;

    if (glSysParam.stEdcInfo.ucEnableAuthTabBatch)
    {
        if ((glProcInfo.stTranLog.ucTranType==AUTH) || (glProcInfo.stTranLog.ucTranType==PREAUTH))
        {
            SaveTabBatchLog(&glProcInfo.stTranLog);
			glSysCtrl.ulLastAuthInvoiceNo = glProcInfo.stTranLog.ulInvoiceNo;
            glSysCtrl.ucLastTransIsAuth = TRUE;
        }
		else
		{
			glSysCtrl.ucLastTransIsAuth = FALSE;
		}
    }
	else
	{
		glSysCtrl.ucLastTransIsAuth = FALSE;
	}

	if((glSysCtrl.ucLastTransIsAuth != ucStateFlag) || (ucStateFlag == TRUE && glSysCtrl.ucLastTransIsAuth == TRUE))
    {
      SaveSysCtrlBase();
    }
#endif

    if( glProcInfo.stTranLog.ucTranType==VOID )
    {
        glProcInfo.stTranLog.uiStatus |= TS_VOID;
        glProcInfo.stTranLog.uiStatus &= ~(TS_ADJ|TS_NOSEND|TS_OFFLINE_SEND|TS_NEED_TC);
        UpdateTranLog(&glProcInfo.stTranLog, glProcInfo.uiRecNo);
#ifdef AMT_PROC_DEBUG
		glProcInfo.stTranLog.cntAmt = 0;//2014-9-19 ack POS to erase amt records
#endif
        //build88S: bug fix reprint last transaction
        glSysCtrl.uiRePrnRecNo = glProcInfo.uiRecNo;
        SaveSysCtrlBase();
        //end 88S
    }
    else if( ChkIfSaveLog() )
    {
        SaveTranLog(&glProcInfo.stTranLog);
#ifdef AMT_PROC_DEBUG
		glProcInfo.stTranLog.cntAmt = 0;//2014-9-19 ack POS to erase amt records
	//	//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 1: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
    }


	if( ChkIfPrnReceipt() )
    {
        if( glProcInfo.stTranLog.ucTranType!=VOID )
        {
            GetNewInvoiceNo();
        }
	}
    SaveRevInfo(FALSE);
 
	EcrSendTransSucceed();
 

    if( !ChkIfBoc() && !ChkIfShanghaiCB() && ChkAcqOption(ACQ_EMV_FEATURE) )
    {
		if ((glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE) ||
			(glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
        {
            if( glProcInfo.stTranLog.ucTranType==SALE || glProcInfo.stTranLog.ucTranType==CASH ||
                glProcInfo.stTranLog.ucTranType==INSTALMENT )
            {
                glSysCtrl.stField56[glCurAcq.ucIndex].uiLength = 0;	// erase bit 56
                SaveField56();
            }
        }
    }
    
    if( glProcInfo.stTranLog.ucTranType!=RATE_BOC &&
        glProcInfo.stTranLog.ucTranType!=RATE_SCB &&
        glProcInfo.stTranLog.ucTranType!=LOAD_CARD_BIN &&
        glProcInfo.stTranLog.ucTranType!=LOAD_RATE_REPORT)
    {
		//who did this ?Citi should also send tc immediately after a success transaction
		//squall 2013.12.09
		/*
		if(ChkIfCiti())//added by laurenc
		{
			OfflineSend(OFFSEND_TRAN);
            DispTransName();
		}
		else*/
		{
			OfflineSend(OFFSEND_TC | OFFSEND_TRAN);
			DispTransName();
		}
    }
    
    if (glProcInfo.stTranLog.ucTranType!=LOAD_CARD_BIN &&
        glProcInfo.stTranLog.ucTranType!=LOAD_RATE_REPORT )
    {
        CommOnHook(FALSE);
    }
    
    if( ChkIfPrnReceipt() )
    {
        DispTransName();
        PrintReceipt(PRN_NORMAL);
    }
    
	// PP-DCC
    if (PPDCC_ChkIfHaveDccAcq())
    {
        // HJJ 20110928 : AUTH should not counted
        bCount = 0;
        if ((glProcInfo.stTranLog.ucTranType==SALE) || (glProcInfo.stTranLog.ucTranType==OFF_SALE))
        {
            bCount = 1;
        }
        if (glProcInfo.stTranLog.ucTranType==VOID)
        {
            if ((glProcInfo.stTranLog.ucOrgTranType==SALE) || (glProcInfo.stTranLog.ucOrgTranType==OFF_SALE))
            {
                bCount = 1;
            }
        }
		
        if (bCount)
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
            // Action
            cAction = '+';
            if (glProcInfo.stTranLog.ucTranType==VOID)
            {
                cAction = '-';
            }
			
            if (PPDCC_ChkIfDccAcq())
            {
                PPDCC_UpdateTransStatistic(sCurrencyCode, szAmount, cAction, NULL, 0);
            } 
            else
            {
                PPDCC_UpdateTransStatistic(sCurrencyCode, NULL, 0, szAmount, cAction);
            }
        }
    }



    // PP-DCC
    if ( (glProcInfo.stTranLog.ucTranType!=VOID) &&
         ChkIfSaveLog() &&
         PPDCC_ChkIfNeedFinalSelect())
    {
        PPDCC_FinalSelect();
    }
    else
    {
        DispResult(0);
    }

    return 0;
}

// voice referral dialing
int ReferralDial(uchar *pszPhoneNo)
{
#ifndef _P90_
    uchar	ucRet, szTelNo[50];
    
    if( pszPhoneNo==NULL || *pszPhoneNo==0 )
    {
        ScrClrLine(2, 7);
        PubDispString(_T("TEL# ERROR"), 4|DISP_LINE_LEFT);
        PubBeepErr();
        PubWaitKey(5);
        return ERR_NO_DISP;
    }
    
    sprintf((char*)szTelNo, "%s%s.", glSysParam.stEdcInfo.szPabx, pszPhoneNo);
    while( 1 )
    {
        OnHook();
#if !defined(WIN32) && !defined(_P60_S1_) && !defined(_P70_S_)
        ucRet = ModemExCommand("AT-STE=0",NULL,NULL,0);
#endif
        ucRet = ModemDial(NULL, szTelNo, 1);
        if( ucRet==0x83 )
        {	// 旁置电话、并线电话均空闲(仅用于发号转人工接听方式)
            PubDispString(_T("PLS USE PHONE "), 4|DISP_LINE_LEFT);
            PubBeepErr();
        }
        DelayMs(1000);
        if( ucRet==0x06 || ucRet==0x00 || PubWaitKey(0)==KEYENTER )
        {
            return 0;
        }
        if( ucRet==0x0D )
        {
            ScrClrLine(2, 7);
            PubDispString(_T("LINE BUSY"), 4|DISP_LINE_LEFT);
            OnHook();
            PubWaitKey(5);
            return ERR_NO_DISP;
        }
        PubWaitKey(1);
    }
#else
    return 0;
#endif
}

// 调整通讯参数(只是设置Modem电话号码)
// get communication parameter from appropriate source, and set into glCommCfg
int AdjustCommParam(void)
{
    uchar	szNewTelNo[100+1];
    
    // 构造拨号电话号码
    if (glCommCfg.ucCommType==CT_MODEM)
    {
        if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
        {
            sprintf((char *)szNewTelNo, "%s.", glSysParam.stEdcInfo.szDownTelNo);
        }
        else if( glProcInfo.stTranLog.ucTranType==SETTLEMENT )
        {
            if( (glCurAcq.stStlPhoneInfo[0].szTelNo[0]!=0) &&
                (glCurAcq.stStlPhoneInfo[1].szTelNo[0]!=0) )
            {	// 都不是空
                sprintf((char *)szNewTelNo, "%s%s;%s%s.",
                    glSysParam.stEdcInfo.szPabx, glCurAcq.stStlPhoneInfo[0].szTelNo,
                    glSysParam.stEdcInfo.szPabx, glCurAcq.stStlPhoneInfo[1].szTelNo);
            }
            else if( glCurAcq.stStlPhoneInfo[0].szTelNo[0]!=0 )
            {
                sprintf((char *)szNewTelNo, "%s%s.",
                    glSysParam.stEdcInfo.szPabx, glCurAcq.stStlPhoneInfo[0].szTelNo);
            }
            else if( glCurAcq.stStlPhoneInfo[1].szTelNo[0]!=0 )
            {
                sprintf((char *)szNewTelNo, "%s%s.",
                    glSysParam.stEdcInfo.szPabx, glCurAcq.stStlPhoneInfo[1].szTelNo);
            }
            else
            {
                return ERR_NO_TELNO;
            }
        }
        else
        {
            if( (glCurAcq.stTxnPhoneInfo[0].szTelNo[0]!=0) &&
                (glCurAcq.stTxnPhoneInfo[1].szTelNo[0]!=0) )
            {	// 都不是空
                sprintf((char *)szNewTelNo, "%s%s;%s%s.",
                    glSysParam.stEdcInfo.szPabx, glCurAcq.stTxnPhoneInfo[0].szTelNo,
                    glSysParam.stEdcInfo.szPabx, glCurAcq.stTxnPhoneInfo[1].szTelNo);
            }
            else if( glCurAcq.stTxnPhoneInfo[0].szTelNo[0]!=0 )
            {
                sprintf((char *)szNewTelNo, "%s%s.",
                    glSysParam.stEdcInfo.szPabx, glCurAcq.stTxnPhoneInfo[0].szTelNo);
            }
            else if( glCurAcq.stTxnPhoneInfo[1].szTelNo[0]!=0 )
            {
                sprintf((char *)szNewTelNo, "%s%s.",
                    glSysParam.stEdcInfo.szPabx, glCurAcq.stTxnPhoneInfo[1].szTelNo);
            }
            else
            {
                return ERR_NO_TELNO;
            }
        }
        sprintf((char *)glCommCfg.stPSTNPara.szTelNo, "%s", szNewTelNo);
        
        if( !CommChkIfSameTelNo(szNewTelNo) )
        {
            CommOnHook(FALSE);
        }
    }
    
    // 从ACQUIRER提取目标IP
    if ((glCommCfg.ucCommType==CT_TCPIP) || (glCommCfg.ucCommType==CT_WIFI) )//2015-11-23
    {
        if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
        {
            sprintf((char *)glCommCfg.stTcpIpPara.szRemoteIP_1, "%.15s", glSysParam.stEdcInfo.stDownIpAddr.szIP);
            sprintf((char *)glCommCfg.stTcpIpPara.szRemotePort_1, "%.5s", glSysParam.stEdcInfo.stDownIpAddr.szPort);
        }
        else
        {
            // 使用transaction IP 1,2. 不使用 settle IP
            sprintf((char *)glCommCfg.stTcpIpPara.szRemoteIP_1, "%.15s", glCurAcq.TxnTcpIp1);
            sprintf((char *)glCommCfg.stTcpIpPara.szRemotePort_1, "%.5s", glCurAcq.TxnTcpPort1);
            sprintf((char *)glCommCfg.stTcpIpPara.szRemoteIP_2, "%.15s", glCurAcq.TxnTcpIp2);
            sprintf((char *)glCommCfg.stTcpIpPara.szRemotePort_2, "%.5s", glCurAcq.TxnTcpPort2);
        }
    }
    
    // 从ACQUIRER提取目标IP
    if ((glCommCfg.ucCommType==CT_GPRS) || (glCommCfg.ucCommType==CT_CDMA) || (glCommCfg.ucCommType==CT_3G))//add CT_3G by richard 20161209
    {
        if( glProcInfo.stTranLog.ucTranType==LOAD_PARA )
        {
            sprintf((char *)glCommCfg.stWirlessPara.szRemoteIP_1, "%.15s", glSysParam.stEdcInfo.stDownIpAddr.szIP);
            sprintf((char *)glCommCfg.stWirlessPara.szRemotePort_1, "%.5s", glSysParam.stEdcInfo.stDownIpAddr.szPort);
        }
        else
        {
            // 使用transaction IP 1,2. 不使用 settle IP
            sprintf((char *)glCommCfg.stWirlessPara.szRemoteIP_1, "%.15s", glCurAcq.TxnGPRSIp1);
            sprintf((char *)glCommCfg.stWirlessPara.szRemotePort_1, "%.5s", glCurAcq.TxnGPRSPort1);
            sprintf((char *)glCommCfg.stWirlessPara.szRemoteIP_2, "%.15s", glCurAcq.TxnGPRSIp2);
            sprintf((char *)glCommCfg.stWirlessPara.szRemotePort_2, "%.5s", glCurAcq.TxnGPRSPort2);
        }
    }
    
    return CommSetCfgParam(&glCommCfg);
}

void DispCommErrMsg(int iErrCode)
{
    COMM_ERR_MSG	stCommErrMsg;
    
    CommGetErrMsg(iErrCode, &stCommErrMsg);
    
    ScrClrLine(2, 7);
    PubDispString(_T(stCommErrMsg.szMsg), 4|DISP_LINE_LEFT);
    if (iErrCode<0)
    {
        ScrPrint(100, 7, ASCII, "%d", iErrCode);
    }
    else
    {
        ScrPrint(100, 7, ASCII, "%02X", (iErrCode & MASK_ERR_CODE));
    }
    PubBeepErr();
    PubWaitKey(5);
}

//add by richard 20161216, for S90 3G SSL https
uchar AddHttpsHeader_FUBON(uchar *context, ushort *length)
{
    uchar szDefPath[] = "/afe/pos";
    uchar szElemBuf[32] = {0};
    uchar szHeaderBuf[256]={0};
    ushort iOffset=0;
    int i;
    uchar szSendBuf[1024] = {0};

    if(NULL==context || NULL==length)
        return -1;

    memcpy(szSendBuf, context, *length);
    
    sprintf(szElemBuf, "POST %s HTTP/1.1\r\n", szDefPath);
    memcpy(&szHeaderBuf[iOffset], szElemBuf, strlen(szElemBuf));
    iOffset += strlen(szElemBuf);

    memset(szElemBuf, 0, sizeof(szElemBuf));
    sprintf(szElemBuf, "Host: %s:%s\r\n", gCurComPara.LocalParam.szRemoteIP_1, gCurComPara.LocalParam.szRemotePort_1);
    memcpy(&szHeaderBuf[iOffset], szElemBuf, strlen(szElemBuf));
    iOffset += strlen(szElemBuf);

    memset(szElemBuf, 0, sizeof(szElemBuf));
    sprintf(szElemBuf, "Content-Type: application/xml\r\n");
    memcpy(&szHeaderBuf[iOffset], szElemBuf, strlen(szElemBuf));
    iOffset += strlen(szElemBuf);

    memset(szElemBuf, 0, sizeof(szElemBuf));
    sprintf(szElemBuf, "Content-Length: %d\r\n\r\n", *length);
    memcpy(&szHeaderBuf[iOffset], szElemBuf, strlen(szElemBuf));
    iOffset += strlen(szElemBuf);

    //PubDebugTx("szHeaderBuf:\n%s \niOffset:%d\n", szHeaderBuf, iOffset);

    memcpy(context, szHeaderBuf, iOffset);
    memcpy(context+iOffset, szSendBuf, *length);
    *length += iOffset;

#ifdef APP_DEBUG_RICHARD
    PubDebugTx("context:\n%s length:%d\n", context, *length);

    /*memset(szSendBuf, 0, sizeof(szSendBuf));
    for(i=iOffset; i<*length; i++)
        sprintf((uchar*)(szSendBuf+3*(i-iOffset)), "%02X ", *(context+i));
    PubDebugTx("glSendData:\n%s\n\n", szSendBuf);*/
#endif

    return 0;
    
}

uchar PhaseHeepsHeader_FUBON()
{
    uchar *p = NULL;
    uchar szHeader[256] = {0};
    uchar szRecvBuf[LEN_MAX_COMM_DATA+10]={0};
    int i;
    uchar szRecvdBuf[2048] = {0};


    //test
    //memcpy(glRecvData.sContent, glSendData.sContent, glSendData.uiLength);
    //glRecvData.uiLength=glSendData.uiLength;

    memcpy(szRecvBuf, glRecvData.sContent, glRecvData.uiLength);
    //PubDebugTx("glRecvData.uiLength:%d\n",glRecvData.uiLength);

    p = strstr(szRecvBuf, "\r\n\r\n");
    if(NULL == p)
    {
        return -1;
    }
    p+=4;

    //PubDebugTx("szRecvBuf+glRecvData.uiLength-p:%d\n",(szRecvBuf+glRecvData.uiLength-p));
    memset(glRecvData.sContent, 0, sizeof(glRecvData.sContent));
    memcpy(glRecvData.sContent, p, szRecvBuf+glRecvData.uiLength-p);
    memcpy(szHeader, szRecvBuf, p-szRecvBuf);

#ifdef APP_DEBUG_RICHARD
    PubDebugTx("szHeader:\n%s\n",szHeader);

    /*for(i=0; i<glRecvData.uiLength; i++)
        sprintf((uchar*)(szRecvdBuf+3*i), "%02X ", glRecvData.sContent[i]);
    PubDebugTx("glRecvData:\n%s\n\n", szRecvdBuf);*/
#endif

    //TODO, can do something with the https header
    
    return 0;
}

// end of file

