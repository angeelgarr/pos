#include "global.h"

/********************** Internal macros declaration ************************/

#define AC_AAC			0x00
#define AC_TC			0x01
#define AC_ARQC			0x02

#define DE55_MUST_SET			0x10	// 必须存在
#define DE55_OPT_SET			0x20	// 可选择存在
#define DE55_COND_SET			0x30	// 根据条件存在

/********************** Internal structure declaration *********************/
typedef struct _tagDE55Tag
{
	ushort	uiEmvTag;
	uchar	ucOption;
	uchar	ucLen;	
}DE55ClSSTag;

typedef struct _tagVisaDRL//2015-11-12 add APID lists
{
    uchar szProgramID[5];
    uchar ulRdClssTxnLmt[12];
    uchar ulRdCVMLmt[12];
    uchar ulRdClssLmt[12];
    //Τ惠璶睰
} VisaDRL;
/********************** Internal functions declaration *********************/

int ClssProcPoll(uchar ucDispType);
void ProcStartTransRet(int iRet);

int UnpackTransData(uchar *pData, ushort nDataLen);
void UnpackOtherData(uchar *pDataIn, uchar ucDataInLen);
int ProcClssTxnOnline(void);
int ProcClssTxnOffline(void);
int FinishClssTxn(int iErrCode);

int SetClSSDE55(uchar bForUpLoad, uchar *psOutData, int *piOutLen);
uchar* GetClssTagDFEF(void);
int SetStdDEClSS55(uchar bForUpLoad, DE55ClSSTag *pstList, uchar *psOutData, int *piOutLen);
int SetExtAMEXClSS55(DE55ClSSTag *pstList, uchar *psOutData, int *piOutLen);//2016-3-31
uchar SearchSpecTLV(ushort nTag, uchar *sDataIn, ushort nDataInLen, uchar *sDataOut, ushort *pnDataOutLen);
int GetSpecTLVData(ushort usTag, uchar *pDataOut, ushort *pnDataOutLen);
void BuildCLSSTLVString(ushort uiEmvTag, uchar *psData, int iLength, uchar **ppsOutData);
int GetPanFromTrack2(uchar *pszPAN, uchar *pszExpDate);

int InitClssSchemeID(void);
int InitRdDispInfo(void);
int InitClssEmvCAPK(void);

int SaveVisaOrJcbPara(uchar ucParamType, EMV_APPLIST *stEmvApp);
int SaveMcPara(uchar ucParamType, EMV_APPLIST *stEmvApp);
int SavePbocPara(uchar ucParamType, EMV_APPLIST *stEmvApp);
int SaveAEPara(uchar ucParamType, EMV_APPLIST *stEmvApp);    // Added by Gillian//2016-2-5

int SaveReaderParam(void);

void SetSupportClss(void);
int SetSupportClssMode(void);
void SetRdTime(void);
void SetClssKey(void);
void SetClssComm(void);
void SetClssFlrLmt(void);
void SetClssAPID(void);//2015-10-13
void GenerateClssKey(uchar ucKeyType, uchar *pKey);
void Long2Str(unsigned long ldat, unsigned char *str);
int SetInternalPara(void);
int SetTLVData(void);
/********************** Internal variables declaration *********************/

READER_INTERNAL_PARAM sg_RdInternalPara;
ushort sg_nPollTime;
uchar sg_bCommOpen;//2014-5-16 enhance
static int default_Clsslmt = 0;

//2015-11-12 add APID info
static VisaDRL sgVisaDRLList[] =
{
	{"\x40\x03\x44\x03\x44", "100000000000","000000100000","000000050000"},
	{"\x40\x04\x46\x04\x46", "100000000000","000000030000","000000000000"},
	{"\x00\x00\x00\x00\x00", "100000000000","000000030000","000000000000"},
	{"\x00\x00\x00\x00\x00", "100000000000","000000030000","000000000000"},
	{"\x00\x00\x00\x00\x00", "100000000000","000000030000","000000000000"},
	{"\x00\x00\x00\x00\x00", "100000000000","000000030000","000000000000"},
	{"\x00\x00\x00\x00\x00", "100000000000","000000030000","000000000000"},
	{"\x00\x00\x00\x00\x00", "100000000000","000000030000","000000000000"},
	{"\x00\x00\x00\x00\x00", "100000000000","000000030000","000000000000"},
	{"\x00\x00\x00\x00\x00", "100000000000","000000030000","000000000000"},//default 
	{0},
};
//2016-2-5 AMEX
int  glProFlag ;//Jason  2016.01.25 14:47//2016-3-4 
#define DE55_LEN_FIXED      0x01    // for amex
#define DE55_LEN_VAR1       0x02
#define DE55_LEN_VAR2       0x03
#define READER_POLL_TIME                300/*800*/            //Jason 33-08 2016.03.03 11:32//2016-3-21

static DE55ClSSTag sgAmexClssTagList[] =
{
    /*{0x9F26, DE55_OPT_SET, 8},
    {0x9F10, DE55_OPT_SET,  32},
    {0x9F37, DE55_OPT_SET, 4},
    {0x9F36, DE55_OPT_SET, 2},
    {0x95,   DE55_OPT_SET, 5},
    {0x9A,   DE55_OPT_SET, 3},
    {0x9C,   DE55_OPT_SET, 1},
    {0x9F02, DE55_OPT_SET, 6},
    {0x5F2A, DE55_OPT_SET, 2},
    {0x9F1A, DE55_OPT_SET, 2},
    {0x82,   DE55_OPT_SET, 2},
    {0x9F03, DE55_OPT_SET, 6},
    {0x5F34, DE55_OPT_SET, 1},
    {0x9F27, DE55_OPT_SET, 1},
    {0x9F06, DE55_OPT_SET,  16},
    {0x9F09, DE55_OPT_SET, 2},
    {0x9F34, DE55_OPT_SET, 3},
    {0x9F0E, DE55_OPT_SET, 5},
    {0x9F0F, DE55_OPT_SET, 5},
    {0x9F0D, DE55_OPT_SET, 5},*/
	    {0x9F26, DE55_LEN_FIXED, 8},
    {0x9F10, DE55_LEN_VAR1,  32},//2016-3-4
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

// 非接消费55域标签,目前与EMV的标签一致  F55 TLV format 
static DE55ClSSTag sgStdClssTagList[] =
{
	{0x57,   DE55_OPT_SET,  0},
//	{0x5A,   DE55_OPT_SET,  0},
//	{0x5F24, DE55_OPT_SET,  0},
	{0x5F2A, DE55_MUST_SET,  0},
	{0x5F34, DE55_MUST_SET,  0},//2013-7-10 must set, if cannot get from card, hard code it to "01"
	{0x82,   DE55_MUST_SET,  0},
	{0x84,   DE55_MUST_SET,  0},
	{0x95,   DE55_MUST_SET,  0},
	{0x8A,   DE55_COND_SET,  0},
	{0x9A,   DE55_MUST_SET,  0},
	{0x9B,   DE55_OPT_SET,  0}, 
	{0x9C,   DE55_MUST_SET,  0},
	{0x9F02, DE55_MUST_SET,  0},
	{0x9F03, DE55_MUST_SET,  0},
	{0x9F08, DE55_OPT_SET,  0},  //paywave do not need
	{0x9F09, DE55_MUST_SET,  0}, //paywave do not need
	{0x9F10, DE55_MUST_SET,  0},
	{0x9F1A, DE55_MUST_SET,  0},
	{0x9F1E, DE55_MUST_SET,  0},
//	{0x9F1F, DE55_OPT_SET,  0},
	{0x9F26, DE55_MUST_SET,  0},
	{0x9F27, DE55_MUST_SET,  0},
	{0x9F33, DE55_MUST_SET,  0},
	{0x9F34, DE55_MUST_SET,  0}, //paywave do not need
	{0x9F35, DE55_OPT_SET,  0},
	{0x9F36, DE55_MUST_SET,  0},
	{0x9F37, DE55_MUST_SET,  0},
	{0x9F41, DE55_MUST_SET,  0},
//	{0x9F5B, DE55_OPT_SET,  0},
	{0},
};

// TC-UPLOAD, TLV format
static DE55ClSSTag sgTcClssTagList[] =
{
	{0x5A,   DE55_OPT_SET,  0},
	{0x5F24, DE55_OPT_SET,  0},
	{0x5F2A, DE55_MUST_SET,  0},
	{0x5F34, DE55_OPT_SET,  0},
	{0x82,   DE55_MUST_SET,  0},
	{0x84,   DE55_MUST_SET,  0},
	{0x8A,   DE55_OPT_SET,  0},
	{0x95,   DE55_MUST_SET,  0},
	{0x8A,   DE55_COND_SET,  0},
	{0x9A,   DE55_MUST_SET,  0},
	{0x9B,   DE55_OPT_SET,  0},
	{0x9C,   DE55_MUST_SET,  0},
	{0x9F02, DE55_MUST_SET,  0},
	{0x9F03, DE55_MUST_SET,  0},
	{0x9F08, DE55_OPT_SET,  0},
	{0x9F09, DE55_OPT_SET,  0},
	{0x9F10, DE55_OPT_SET,  0},
	{0x9F18, DE55_OPT_SET,  0},
	{0x9F1A, DE55_MUST_SET,  0},
	{0x9F1E, DE55_OPT_SET,  0},
	{0x9F26, DE55_MUST_SET,  0},
	{0x9F27, DE55_MUST_SET,  0},
	{0x9F33, DE55_MUST_SET,  0},
	{0x9F34, DE55_MUST_SET,  0},
	{0x9F35, DE55_OPT_SET,  0},
	{0x9F36, DE55_MUST_SET,  0},
	{0x9F37, DE55_MUST_SET,  0},
	{0x9F41, DE55_OPT_SET,  0},
	{0x9F5B, DE55_OPT_SET,  0},
	{0x9F06, DE55_OPT_SET,  0}, //2016-2-5 AMEX
	{0x9F09, DE55_OPT_SET,  0},
	{0x9F0E, DE55_OPT_SET,  0},
	{0x9F0F, DE55_OPT_SET,  0},
	{0x9F0D, DE55_OPT_SET,  0},
	{0},
};

// 非接消费56域标签,目前与EMV的标签一致  F56 TLV format 
static DE55ClSSTag sgStdClssField56TagList[] =
{
	{0x5A,   DE55_MUST_SET, 0},
	{0x95,   DE55_MUST_SET, 0},
	{0x9B,   DE55_MUST_SET, 0},
	{0x9F10, DE55_MUST_SET, 0},
	{0x9F26, DE55_MUST_SET, 0},
	{0x9F27, DE55_MUST_SET, 0},
	{0},
};

// 20160921 Kenneth add this constant to avoid key in 11111....., 22222... - start
#define ALL_ONE_32 "11111111111111111111111111111111"
#define ALL_TWO_32 "22222222222222222222222222222222"
// 20160921 Kenneth add this constant to avoid key in 11111....., 22222... - end

/********************** external reference declaration *********************/
extern int  ChkIfMC(uchar *pszPAN); //2016-2-15 check if master card! add cardbin "222100272099"
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/
//----------------------------------------------------------------------------------
//                                 
//                                     L3回调函数 Callback functions
//
//-----------------------------------------------------------------------------------
#ifdef POS_DEBUG
#define WAVETEST_CHANNEL	0
#endif
int cWavePortReset(int nChannel)
{
#ifdef POS_DEBUG
	PortReset(WAVETEST_CHANNEL);	
#endif
	return PortReset((uchar)nChannel);	
}

int cPortOpen(int nChannel, uchar *ucPara)
{
#ifdef POS_DEBUG
	PortOpen(WAVETEST_CHANNEL, "115200,8,n,1");	
#endif
	return PortOpen((uchar)nChannel, ucPara);
}

int cPortClose(int nChannel)
{
#ifdef POS_DEBUG
	PortClose(WAVETEST_CHANNEL);	
#endif
	return PortClose((uchar)nChannel);
}

int cWavePortSend(int nChannel, uchar ucData)
{
#ifdef POS_DEBUG
	PortSend(WAVETEST_CHANNEL, ucData);	
#endif
	return PortSend((uchar)nChannel, ucData);
}

int cWavePortSends(int nChannel, uchar *paucStr, ushort usStrLen)
{
#ifdef POS_DEBUG
	PortSendstr(WAVETEST_CHANNEL, paucStr, usStrLen);	
#endif

	return PortSendstr((uchar)nChannel, paucStr, usStrLen);	
}

int cWavePortRecv(int nChannel, uchar *pucData, uint uiTimeOutMs)
{
#ifdef POS_DEBUG
	int iRet;
	iRet = PortRecv((uchar)nChannel, pucData, (ushort)uiTimeOutMs);
	if (iRet == 0)
	{
		PortSendstr(WAVETEST_CHANNEL, pucData, 1);
	}
	return iRet;
#else

	return PortRecv((uchar)nChannel, pucData, (ushort)uiTimeOutMs);
#endif
}

void cWaveTimerSet(int nTimerNo, int nMs)
{
	TimerSet((uchar)nTimerNo, (ushort)nMs);
}

int cWaveTimerCheck(int nTimerNo)
{
	return TimerCheck((uchar)nTimerNo);
}

int  cFileOpen(char *filename, uchar mode)
{
	return open(filename,mode);
}

int cFileRead(int fid, uchar *dat, int len)
{
	return read(fid, dat, len);
}

int cFileWrite(int fid, uchar *dat, int len)
{
	return write(fid, dat, len);
}

int cFileClose(int fid)
{
	return close(fid);
}

int cFileSeek(int fid, long offset, uchar fromwhere)
{
	return seek(fid, offset, fromwhere);	
}

long  cFileGetSize(char *filename)
{
	return filesize(filename);	
}

int cWaveDetStopTrans(void)
{
	uchar glKey;
//	uchar szBuff[100];
//	int iReadCnt, iResult, uiTimeOutSec;

	if( MagSwiped()==0 )
	{
		return SWIPED_MAGCARD;
	}
	
	if( ChkIfEmvEnable() && IccDetect(ICC_USER)==0 )
	{
		return INSERTED_ICCARD;
	}
	
	if( PubKeyPressed() )
	{
		glKey = getkey();
		if (glKey == KEYCANCEL)
		{
			return CLSS_USER_CANCEL;		
		}
		if (glKey>=KEY0&&glKey<=KEY9)
		{
			glKeyValue=glKey;
			return CLSS_USER_CANCEL;
		}
	}

//	memset(szBuff, 0, sizeof(szBuff))
//	iReadCnt = 0;
//	uiTimeOutSec = 1;
//	if (PortRecv(glSysParam.stEdcInfo.ucClssComm, &szBuff[iReadCnt], 0) == 0)
//	{
//		if (szBuff[0] == 'A')
//		{
//			iReadCnt++;
//			TimerSet(TIMER_TEMPORARY, 10);
//			while( 1 )
//			{
//				if( TimerCheck(TIMER_TEMPORARY)==0 )
//				{
//					if( uiTimeOutSec==0 )
//					{
//						break;
//					}
//
//					uiTimeOutSec--;
//					TimerSet(TIMER_TEMPORARY, 10);
//				}
//
//				ucRet = PortRecv(glSysParam.stEdcInfo.ucClssComm, &szBuff[iReadCnt], 0);
//				if( ucRet!=0 )
//				{
//					if( ucRet==0xFF )
//					{
//						continue;
//					}
//					break;
//				}
//				if(szBuff[1]!='M')
//				{
//					break;
//				}
//				if (iReadCnt==2) && (szBuff[2]!='T')
//				{
//					break;
//				}
//
//				if( szBuff[iReadCnt]==';' )
//				{
//					szBuff[iReadCnt] = 0;
//					ScrClrLine(2);
//					DispAmount(2, &szBuff[3]);
//					break;
//				}
//				iReadCnt++;
//			}
//			
//		}
//	}

	return RC_SUCCESS;	
}

//----------------------------------------------------------------------------------
//                                 
//                                     ㄑ场砐拜ㄧ计
//									   For external call functions
//
//-----------------------------------------------------------------------------------
void ClssOpen(void)//2014-5-16 enhance 
{
	////ttttttttt
	//ScrCls();
	//ScrPrint(0,0,0,"ClssOpen()_0 ");
	//PubWaitKey(7);
    if (sg_bCommOpen == 0)
    {
	//	//ttttttttt
	//ScrCls();
	//ScrPrint(0,0,0,"ClssOpen()_1 ");
	//PubWaitKey(7);
        PortOpen(glSysParam.stEdcInfo.ucClssComm, "38400,8,n,1");
        PortReset(glSysParam.stEdcInfo.ucClssComm);
        sg_bCommOpen = 1;
        //ClssProcPoll(FALSE);
    }
//	ClssInit();
}

void ClssClose(void)
{
	////ttttttttt
	//ScrCls();
	//ScrPrint(0,0,0,"ClssClose()_0 ");
	//PubWaitKey(7);
    if (sg_bCommOpen)
    {
	//	//ttttttttt
	//ScrCls();
	//ScrPrint(0,0,0,"ClssClose()_1 ");
	//PubWaitKey(7);
        PortClose(glSysParam.stEdcInfo.ucClssComm);
        sg_bCommOpen = 0;
    }
}
//程序初始化
int ClssInit(void)
{
	int		iRet;	
//	if (bFirstPollFlag==0)
//	{
//		return 0;
//	}
#if defined(_P60_S1_) || defined(_S_SERIES_)
	SetOffBase(OffBaseDisplay);
#endif
#ifdef _P60_S1_
	PortClose(0);
	PortClose(3);
#endif
	
	ClssOpen(); //2014-5-16 enhance
	
	if(!sg_bWaveInitPara) 
    {
		sg_bWaveInitPara = 1;
		iRet = WaveInitPara(glSysParam.stEdcInfo.ucClssComm);
		if (iRet)
		{
			ProcError_Wave(iRet);
			return ;
		}
    }
	
	//查询当前读卡器连接状态
	iRet = ClssProcPoll(TRUE);
	if (iRet)
	{
		ProcError_Wave(iRet);
		return iRet;
	}
	SetReaderLan();//2016-3-9 add for set R50 Language
	SetInternalPara();
	//SetRdTime();
	//启动POLL定时器
	iRet = WaveMngInternalPara('G', &sg_RdInternalPara);
	if (iRet == 0)
	{
		sg_nPollTime = ((sg_RdInternalPara.aucPollMsg[0]<<8) + sg_RdInternalPara.aucPollMsg[1])*10-3;
		TimerSet(TIMER_POLL, sg_nPollTime);
	}	
	bFirstPollFlag = 0;

#ifdef _S60_ 
	if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
		|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
	{
		ClssClose(); //2014-5-16 enhance
		glProcInfo.bIfClssTxn = 1;
	}
#endif
	return 0;
}

int uL3WavePortSent(uchar ucPort, uchar *usData, int iLen)
{
	uchar ucRet, ucSendBuf[1024];
	int  iCnt, iSendLen;

	memset(ucSendBuf,0,sizeof(ucSendBuf));
	ucSendBuf[0] = STX;
	ucSendBuf[1] = (iLen/1000)<<4    | (iLen/100)%10;	// convert to BCD
	ucSendBuf[2] = ((iLen/10)%10)<<4 | iLen%10;
	memcpy(&ucSendBuf[3], usData, iLen);
	//ucSendBuf[3+iLen]   = ETX;
	PubCalcLRC(&ucSendBuf[1], iLen+2, &ucSendBuf[iLen+3]);
	iSendLen = iLen+4;
    
	for(iCnt= 0; iCnt<iSendLen; iCnt++)
	{
		ucRet = PortSend(ucPort, ucSendBuf[iCnt]);
		if (ucRet)
		{
			return EX_PORT_SEND_ERR;
		}
	}
	return 0;
}

int uL3WavePortRece(uchar gChannel, uchar *usRcvData, int *iRecLen, ushort TimeOut)
{
    unsigned short  i, Len;
    unsigned char LRC, usRcvBuf[1024], ucKey;

	memset(usRcvBuf, 0, sizeof(usRcvBuf));
	
	TimerSet(4, (ushort)TimeOut*10);

	kbflush();
    while (1) 
	{
		if(TimerCheck(4) != 0)
		{
			if( MagSwiped()==0 )
			{
				return SWIPED_MAGCARD;
			}
			
			if( ChkIfEmvEnable() && IccDetect(ICC_USER)==0 )
			{
				return INSERTED_ICCARD;
			}
			
			if( PubKeyPressed() )
			{
				ucKey = getkey();
				if (ucKey == KEYCANCEL)
				{
					return CLSS_USER_CANCEL;		
				}
			}
			if (PortRecv((uchar)gChannel, usRcvBuf, 50) == 0) 
			{
				if (usRcvBuf[0] == STX) 
				{
					break;    //receive the prefix byte,break loop
				}
				PortReset(gChannel);
//				if (usRcvBuf[0] == 0x03) 
//				{
//					PortRecv((uchar)gChannel, usRcvBuf, 20);
//					if (usRcvBuf[0] == 0x04) 
//					{
//						usRcvBuf[0] = 0x02;
//					}
//					if (usRcvBuf[0] == 0x05) 
//					{
//						PortSend((uchar)gChannel, 'R');
//						continue;
//					}
//				}
			}
		}
		else
		{
			return EX_PORT_TIMEOUT;
		}
    }
	
	//get the next 2 char
    for (i = 1; i < 3; i++)
	{
		if(PortRecv((uchar)gChannel, usRcvBuf + i, 100)) 
		{
			return EX_PORT_RECV_ERR;
		}		
	}	

	//check the length is valid
	//    Len = RcvBuf[1];
	//    Len <<= 8;
	//      Len += RcvBuf[2];
	Len = usRcvBuf[1] * 256 + usRcvBuf[2];
	if (Len > 1024) 
	{
		return EX_PORT_LEN_ERR;
	}
	
	//LRC code check
    LRC = usRcvBuf[1]^usRcvBuf[2];
    for (i = 0; i < Len + 1; i++) 
	{
        if (PortRecv((uchar)gChannel, usRcvBuf + i, 100)) 
		{
			return EX_PORT_RECV_ERR;
		}
        LRC ^= usRcvBuf[i];
    }

    if (LRC != 0) 
	{
		return EX_PORT_LRC_ERR;
	}
	
	memcpy(usRcvData, usRcvBuf, Len);
	*iRecLen = Len;

    return 0;
}

int uL3WaveGetAmount(uchar *usAmt)
{
	uchar ucSendBuf[1024], ucRcvBuf[1024];
	int iRet, iRecLen, iRRet;
	
	memset(ucRcvBuf,0,sizeof(ucRcvBuf));
	memset(ucSendBuf,0,sizeof(ucSendBuf));
	
	iRet = PortClose(glSysParam.stEdcInfo.ucClssComm);
	iRet = PortOpen(glSysParam.stEdcInfo.ucClssComm,  "38400, 8, N, 1");
	if (iRet)
	{
		return EX_PORT_OPEN_ERR;
	}

	iRet = PortReset(glSysParam.stEdcInfo.ucClssComm);
	ucSendBuf[0] = 0x01;
	memcpy(&ucSendBuf[1], usAmt, 6);
	iRet = uL3WavePortSent(glSysParam.stEdcInfo.ucClssComm, ucSendBuf, 7);
	if (iRet!= 0)
	{
		return iRet;
	}
	//Build 1.00.0109
	PubDispString(_T("PLS INSERT/SWIPE"), DISP_LINE_CENTER|4);
	PubDispString(_T("/TAP CARD..."), DISP_LINE_CENTER|6); 
	//iRet = PortReset(glSysParam.stEdcInfo.ucClssComm);
	iRecLen = 0;
	while (1)
	{
		iRet = uL3WavePortRece(glSysParam.stEdcInfo.ucClssComm, ucRcvBuf, &iRecLen, 70);
		if((iRet == 0) && (ucRcvBuf[0] == 0x03))
		{
			memcpy(glProcInfo.stTranLog.szAmount,ucRcvBuf+1,12);
#ifdef AMT_PROC_DEBUG
			//2014-9-18 each time come to glAmount or szAmount
			sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(01)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(01)%.13s",glProcInfo.stTranLog.szAmount);
			glProcInfo.stTranLog.cntAmt++;
	//		//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 0: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
			DispAmount(2, glProcInfo.stTranLog.szAmount);
			iRecLen = 0;
		}
		else
		{
			break;
		}
	}

	uL3WavePortSent(glSysParam.stEdcInfo.ucClssComm, "\xFE", 1);


	DelayMs(1000);
	ClssClose(); //2014-5-16 enhance
//	iRRet = PortOpen(glSysParam.stEdcInfo.ucClssComm,  "38400, 8, N, 1");
//	if (iRRet)
//	{
//		return EX_PORT_OPEN_ERR;
//	}
	
	iRRet = WaveInitPara(glSysParam.stEdcInfo.ucClssComm);
	if (iRRet)
	{
		ProcError_Wave(iRRet);
		return iRRet;
	}
	
	//查询当前读卡器连接状态
	iRRet = ClssProcPoll(TRUE);
	if (iRRet)
	{
		ProcError_Wave(iRRet);
		return iRet;
	}

	
	if (iRet!= 0)
	{
		return iRet;
	}

	if (ucRcvBuf[0] == 0xFE)
		return ERR_USERCANCEL;
	else if (ucRcvBuf[0] == 0)
	{
		return 0;
	}
	else
		return ucRcvBuf[0];
}

//獶钡ユ
//Modify by Gillian 2016/1/29 //2016-2-5
int ClssProcTxn(uchar *pszAmount, int rtTag, uchar ucTransType)//2016-5-11
{
	int		iRet;
	 ST_EVENT_MSG    stEvent;
	uchar	sDataOut[1024], sMsgIDList[40], ucACType;
	ushort	nDataOutLen,usLength, usAmtLen;
	uchar	szTemAmt[12+1],szClssAmt[12+1];
	uchar   sBuff[17];//2015-10-13
	uchar	ucBuff[6+1];
	uchar	szForeignAmt[12+1];
    uchar ucTemp[64] = {0};
    uchar szBuff[64] = {0};
    uchar szTmpName[64] = {0};
	uchar   ucTagNum=1, ucTagListIn[10] = {0}, pTlvData_Sale[20]= {0}, pTlvData_Refund[20]= {0};//2016-4-25
	ushort  usTagLenIn = 1, pusTlvDataLen=3;//2016-4-25


	if (glSysParam.stEdcInfo.ucClssPARAFlag == 0)
	{
		ScrClrLine(2,7);
		PubDispString(_T("PLS DOWN RD PARA"), DISP_LINE_CENTER|4);
		kbflush();
		getkey();
		return 0;
	}
	//PortReset(glSysParam.stEdcInfo.ucClssComm);

	//交易类型
	//Modify by Gillian 2016/1/29 //2016-2-5 
	glProcInfo.stTranLog.ucTranType = ucTransType;  
	
    //Gillian 20160930
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

	//显示界面
	ScrCls();
	DispTransName();
	DispAmount(2, pszAmount);
	PubDispString(_T("PLS INSERT/SWIPE"), DISP_LINE_CENTER|4);
	PubDispString(_T("/TAP CARD..."), DISP_LINE_CENTER|6); //Gillian debug 0930
	//发起交易
	memset(sDataOut, 0, sizeof(sDataOut));
	memcpy(glProcInfo.stTranLog.szAmount, pszAmount, 12);
#ifdef AMT_PROC_DEBUG
	//2014-9-18 each time come to glAmount or szAmount
	sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(02)%.13s",glAmount);
	sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(02)%.13s",glProcInfo.stTranLog.szAmount);
	glProcInfo.stTranLog.cntAmt++;
	////2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 1: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
	memset(pszAmount, 0, sizeof(pszAmount));
	PubAsc2Bcd(glProcInfo.stTranLog.szAmount, 12, pszAmount);

	//发送金额，等待输入小费
	if (glSysCtrl.ucLoginStatus == LOGIN_MODE)
	{
		//Build 1.00.0109
		ScrClrLine(4, 7);
		PubDispString(_T("PROCESSING..."), DISP_LINE_CENTER|4);
//#if 0
		WaveProcOtherProtocol("\xE0",1,sDataOut,&nDataOutLen); //Jolie add for R50 communication with other protocols
	//	if (memcmp(&sDataOut[2], "OTHER", 5) !=0)
	//	{
	//		DispTransName();
	//		PubDispString("R50 NOT SUPPORT", 4|DISP_LINE_CENTER);
	//		WaveShowStatus(0, "\x02\x01", 2); 
	//		PubBeepErr();
	//		PubWaitKey(2);
	//		return ERR_NO_DISP;	
	//	}	


		DelayMs(1500);
		iRet = uL3WaveGetAmount(pszAmount);
		DelayMs(500);
		if (iRet == ERR_USERCANCEL)
		{
			DispTransName();
			PubDispString("USER CANCEL", 4|DISP_LINE_CENTER);
			WaveShowStatus(0, "\x02\x01", 2); 
			PubBeepErr();
			PubWaitKey(2);
			return ERR_NO_DISP;	
		}
		else if ((iRet == SWIPED_MAGCARD) || (iRet == INSERTED_ICCARD))
		{
			WaveProcOtherProtocol("\xDB",3,sDataOut,&nDataOutLen); //Mandy add for R50 press key to add amount
			if (nDataOutLen>=12)
			{
				memcpy(glProcInfo.stTranLog.szAmount,sDataOut+2,12);
#ifdef AMT_PROC_DEBUG
				//2014-9-18 each time come to glAmount or szAmount
				sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(03)%.13s",glAmount);
				sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(03)%.13s",glProcInfo.stTranLog.szAmount);
				glProcInfo.stTranLog.cntAmt++;
	//			//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 2: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
			}	
			DispTransName();
			DispAmount(2, glProcInfo.stTranLog.szAmount);
			DelayMs(1500);
			ProcStartTransRet(iRet);

			WaveShowStatus(0, "\x02\x01", 2); //mandy add
			return 0;
		}
		else if (iRet != RC_SUCCESS)
		{

			DispTransName();
			DispAmount(2, glProcInfo.stTranLog.szAmount);
			WaveShowStatus(0, "\x02\x01", 2); //mandy add
			PubBeepErr();
			PubWaitKey(2);
			return ERR_NO_DISP;	
		}
		WaveProcOtherProtocol("\xDB",3,sDataOut,&nDataOutLen);
		if (nDataOutLen>=12)
		{
			memcpy(glProcInfo.stTranLog.szAmount,sDataOut+2,12);
#ifdef AMT_PROC_DEBUG
			//2014-9-18 each time come to glAmount or szAmount
			sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(04)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(04)%.13s",glProcInfo.stTranLog.szAmount);
				glProcInfo.stTranLog.cntAmt++;
	//			//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 3: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
			PubAsc2Bcd(glProcInfo.stTranLog.szAmount, 12, pszAmount);
			DispAmount(2, glProcInfo.stTranLog.szAmount);
		}	

		WaveProcOtherProtocol("\xE1",3,sDataOut,&nDataOutLen); //Jolie add for R50 communication with L3 protocols
		if (memcmp(&sDataOut[2], "L3", 2) !=0)
		{
			DispTransName();
			PubDispString("R50 NOT SUPPORT", 4|DISP_LINE_CENTER);
			WaveShowStatus(0, "\x02\x01", 2);
			PubBeepErr();
			PubWaitKey(2);
			return ERR_NO_DISP;	
		}	
	//	#endif
	}
	//2016-3-31 *****************start*************
		
    /*=======BEGIN: Jason 2016.01.15  9:58 modify===========*/
    //echo test
    iRet = WaveProcOtherProtocol("\xDE", 2, sDataOut, &nDataOutLen);
    memset(sDataOut, 0, sizeof(sDataOut));
    nDataOutLen = 0;
    if(iRet != RC_SUCCESS)
    {
		if(iRet == SWIPED_MAGCARD || iRet == INSERTED_ICCARD)
		{			
			return iRet;
		}
        ProcStartTransRet(iRet);//?

        return ERR_NO_DISP;//Jason 34-08 2016.03.01 11:36
    }
    /*====================== END======================== */

    memcpy(ucTagListIn, "\x9C", 1);
    memcpy(pTlvData_Sale, "\x9C\x01\x00", 3);
	memcpy(pTlvData_Refund, "\x9C\x01\x20", 3);
	if(glProcInfo.stTranLog.ucTranType == SALE)
    {
        ClssOpen();
        iRet = WaveMngTLVData('S', ucTagListIn, usTagLenIn, &ucTagNum, pTlvData_Sale, &pusTlvDataLen);
	}
	else if(glProcInfo.stTranLog.ucTranType == REFUND)
	{
		ClssOpen();
		iRet = WaveMngTLVData('S', ucTagListIn, usTagLenIn, &ucTagNum, pTlvData_Refund, &pusTlvDataLen);
	}
	if (iRet)
	{
		PubShowTitle(TRUE, (uchar *)_T("Set TLVData"));
		ProcError_Wave(iRet);
		return iRet;
	}

	ClssOpen();  //Added by Gillian 2016/1/29

	//2016-3-31 ***************** end *************
//	WavePollAuthProc();//here needs to auth
	//发起交易
	ClssOpen();  //Added by Gillian 2016/1/29//2016-2-5
	kbflush();
	iRet = WaveStartTrans(pszAmount, sDataOut, &nDataOutLen);

	if (iRet != RC_SUCCESS)
	{
		if( rtTag == 1 )//2016-5-11
		{
			if(iRet == SWIPED_MAGCARD || iRet == INSERTED_ICCARD)
			{				
				return iRet;
			}
			else //Gillian dubug
			{
				return 77; 
			}
		}
			if (glKeyValue>=KEY0&&glKeyValue<=KEY9)
			{
				
						stEvent.MsgType = KEYBOARD_MSG;
						stEvent.KeyValue = glKeyValue;
						memcpy(glAmount, glProcInfo.stTranLog.szAmount, 12);
#ifdef AMT_PROC_DEBUG
						//2014-9-18 each time come to glAmount or szAmount
						sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(05)%.13s",glAmount);
						sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(05)%.13s",glProcInfo.stTranLog.szAmount);
						glProcInfo.stTranLog.cntAmt++;
	//					//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 4: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
						glKeyValue = 0x00;
						WaveShowStatus(0, "\x02\x01", 2); //mandy add
						event_main(&stEvent);
						return 0;
			}
			else
			{			
				ProcStartTransRet(iRet);	
				WaveShowStatus(0, "\x02\x01", 2); //mandy add
				return 0;
			}
	return 0; //Gillian 20160930
	}
	if (glSysCtrl.ucLoginStatus == LOGIN_MODE)
	{
	WaveProcOtherProtocol("\xDB",3,sMsgIDList,&usAmtLen); //Mandy add for R50 press key to add amount
	if (usAmtLen>=12)
	{
		memcpy(glProcInfo.stTranLog.szAmount,sMsgIDList+2,12);
#ifdef AMT_PROC_DEBUG
		//2014-9-18 each time come to glAmount or szAmount	
		sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(06)%.13s",glAmount);
		sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(06)%.13s",glProcInfo.stTranLog.szAmount);
		glProcInfo.stTranLog.cntAmt++;
	//	//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 5: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
	}	
	DispTransName();
	DispAmount(2, glProcInfo.stTranLog.szAmount);
	DelayMs(1500);
	}
	ScrClrLine(2,7);
	PubDispString(_T("PROCESSING..."), DISP_LINE_CENTER|4);

#ifdef POS_DEBUG
	PortSendstr(WAVETEST_CHANNEL, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 10);	
	PortSendstr(WAVETEST_CHANNEL, sDataOut, nDataOutLen);
	PortSendstr(WAVETEST_CHANNEL, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 10);	
#endif

	//分析返回数据	
	iRet = UnpackTransData(sDataOut, nDataOutLen);
	if (iRet)
	{
		WaveShowStatus(1, "\x05\x01", 2);
		PubBeepErr();
		PubWaitKey(2);
		return ERR_NO_DISP;	
	}

	//输入模式
	glProcInfo.stTranLog.uiEntryMode = MODE_CONTACTLESS;

	// 从2磁道信息分析出卡号(PAN)
	iRet = GetPanFromTrack2(glProcInfo.stTranLog.szPan, glProcInfo.stTranLog.szExpDate);
	if( iRet!=0 )
	{
		WaveShowStatus(1, "\x05\x01", 2);
		DispMagReadErr();	
		PubBeepErr();
		PubWaitKey(2);
		return ERR_NO_DISP;
	}

	//通过卡号确认发卡行/收单行
	iRet = MatchCardTable(glProcInfo.stTranLog.szPan,
						glProcInfo.stTranLog.ucTranType,
						0);

	if (ChkIfDccAcquirer())//CLSS transaction do not match DCC acq. squall 2012.12.11
	{
		iRet = MatchCardTable(glProcInfo.stTranLog.szPan,
			glProcInfo.stTranLog.ucTranType,
                            1);
	}
	if( iRet!=0 )
	{
		ScrClrLine(2, 7);
		WaveShowStatus(1, "\x05\x01", 2);
		PubDispString(_T("UNSUPPORT CARD"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(2);
		return ERR_NO_DISP;
	}

	if (GetSpecTLVData(0x5F34, &glProcInfo.stTranLog.ucPanSeqNo, &usLength) == 0)
	{
		glProcInfo.stTranLog.bPanSeqOK = TRUE;
	}
	if(GetSpecTLVData(0x9F02,szTemAmt,&usLength)==0)
	{
		PubBcd2Asc0(szTemAmt,6,szClssAmt);
		if(strcmp(szClssAmt,glProcInfo.stTranLog.szAmount) !=0)
		{
			strcpy(glProcInfo.stTranLog.szAmount,szClssAmt);
#ifdef AMT_PROC_DEBUG
			//2014-9-18 each time come to glAmount or szAmount			
			sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(07)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(07)%.13s",glProcInfo.stTranLog.szAmount);
			glProcInfo.stTranLog.cntAmt++;
	//		//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 6: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
		}
	}
	////2016-2-5
	//    if(GetSpecTLVData(0x9F12,   ucTemp, &usLength) == 0)
 //   {
 //       memcpy(glProcInfo.stTranLog.szAppPreferName, ucTemp, usLength);
 //       glProcInfo.stTranLog.szAppPreferName[usLength]=0;
 //   }
 //   if(GetSpecTLVData(0x5F20, ucTemp, &usLength) == 0)
 //   {
 //       memcpy(szBuff, ucTemp, usLength);
 //       szBuff[usLength]=0;
 //       ConvertHolderName(szBuff, szTmpName);
 //       sprintf(glProcInfo.stTranLog.szHolderName, "%.26s", szTmpName);
 //   }



    //if (glProcInfo.stTranLog.szCVMRslt[0] == CLSS_CVM_ONLINE_PIN)//2016-2-5
    //{
    //    iRet = GetPIN(GETPIN_FORCE);
    //    if( iRet!=0 )
    //    {
    //        return iRet;
    //    }
    //}

	//处理交易返回
	SetCommReqField();
	nDataOutLen = 0;
	memset(sDataOut, 0, sizeof(sDataOut));
/****************************************************************************************************/
/*  处理规则																						*/
/*	VISA&JCB,根据9F10来处理,参考VISA L2规范里的139页,有一个表,是由第5个字节的第6-5bit来表示的;		*/
/*	MC,根据9F27来处理,参考EMV L2第三册,类似EMV;														*/
/*	SCHEME_VISA_WAVE_2&SCHEME_JCB_WAVE_2也是根据9F27来处理;											*/
/*	MSD直接联机,MSD对应的SCHEME ID:SCHEME_VISA_MSD_20,SCHEME_MC_MSTR;								*/
/****************************************************************************************************/
#ifdef PAYPASS_DEMO
	if ( ((glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_2
		||glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_3
		||glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_MSD_20) && !(gucIsPayWavePass == PAYWAVE)) 
		||(glProcInfo.stWaveTransData.ucSchemeId == SCHEME_MC_MCHIP ||
			glProcInfo.stWaveTransData.ucSchemeId ==SCHEME_MC_MCHIP) && !(gucIsPayWavePass ==PAYPASS)
		)
	{
		ProcError_Wave(RC_FAILURE);
		memcpy(sMsgIDList,"\x05\x01",2);// WAVE_MSG_DECLINE
		WaveShowStatus(1, sMsgIDList, 2);
		return ERR_NO_DISP;
	}
#endif

	if ( (glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_WAVE_2 ||
		 glProcInfo.stWaveTransData.ucSchemeId == SCHEME_JCB_WAVE_2 ) )
	{	
		if (0 == GetSpecTLVData(0x9F27, sDataOut, &nDataOutLen) ) // GAC online auth
		{
			iRet = ProcClssTxnOnline();
			return FinishClssTxn(iRet);
		}
		else if ( glProcInfo.stWaveTransData.stWaveOtherData.ucVLP_IAC_Flg == 1	)	// offline approval
		{
			ProcClssTxnOffline();
		}
		else
		{
			ProcError_Wave(RC_FAILURE);
			//2014-5-13 tt enhance START
			CommOnHook(FALSE);
			ClssOpen(); //2014-5-16 enhance
			//2014-5-13 tt enhance END

			memcpy(sMsgIDList,"\x05\x01",2);// WAVE_MSG_DECLINE
			WaveShowStatus(1, sMsgIDList, 2);
			return ERR_NO_DISP;
		}
	}
	else if (glProcInfo.stWaveTransData.ucSchemeId == SCHEME_MC_MCHIP )
	{
		if (0 == GetSpecTLVData(0x9F27, sDataOut, &nDataOutLen)) // GAC online auth
		{
			if(sDataOut[0] == 0x80) // ARQC
			{
				iRet = ProcClssTxnOnline();
				return FinishClssTxn(iRet);
			}
			else if(sDataOut[0] == 0x40) // TC
			{
				ProcClssTxnOffline();
			}
			else
			{
				ProcError_Wave(RC_FAILURE);
				//2014-5-13 tt enhance START
				CommOnHook(FALSE);
				ClssOpen(); //2014-5-16 enhance
				//2014-5-13 tt enhance END
				memcpy(sMsgIDList,"\x05\x01",2);	// WAVE_MSG_DECLINE
				WaveShowStatus(1, sMsgIDList, 2);
				return ERR_NO_DISP;
			}
		}
		else
		{
			ProcError_Wave(RC_FAILURE);
			//2014-5-13 tt enhance START
			CommOnHook(FALSE);
			ClssOpen(); //2014-5-16 enhance
			//2014-5-13 tt enhance END
			memcpy(sMsgIDList,"\x05\x01",2);		// WAVE_MSG_DECLINE
			WaveShowStatus(1, sMsgIDList, 2);
			return ERR_NO_DISP;
		}
	}
	else if ( glProcInfo.stWaveTransData.stWaveOtherData.ucDDAFailFlg == 1 || 
		      glProcInfo.stWaveTransData.ucSchemeId == SCHEME_VISA_MSD_20  ||
			  glProcInfo.stWaveTransData.ucSchemeId == SCHEME_MC_MSTR ) // send the offline TC for online authorization
	{
		iRet = ProcClssTxnOnline();
		return FinishClssTxn(iRet);
	}
	else
	{ 	
		//2016-2-5 AMEX
		// Modify by Gillian  2016/1/21 
		// AE
        if (0 == GetSpecTLVData(0x9F27, sDataOut, &nDataOutLen)) // GAC online auth
        {
            if(sDataOut[0] == 0x80) // ARQC
            {
                ProcClssTxnOnline();
                return FinishClssTxn(iRet);
            }
            else if(sDataOut[0] == 0x40) // TC
            {
                ProcClssTxnOffline();
            }
            else
            {
                ProcError_Wave(RC_FAILURE);
                CommOnHook(FALSE);
                ClssOpen();
                memcpy(sMsgIDList,"\x05\x01",2);    // WAVE_MSG_DECLINE
                WaveShowStatus(1, sMsgIDList, 2);
            }
        }
        else
        {
			//ProcClssTxnOnline();//2016-4-25 error
			//return FinishClssTxn(iRet);
			// Raymond 祇瞷拜肈. //2016-5-17
            ProcError_Wave(RC_FAILURE);
            CommOnHook(FALSE);
            ClssOpen();
            memcpy(sMsgIDList,"\x05\x01",2);    // WAVE_MSG_DECLINE
            WaveShowStatus(1, sMsgIDList, 2);
        }
		//2016-2-5 marked
		//// 根据IAD 判断密文类型
		//if (0 == GetSpecTLVData(0x9F10, sDataOut, &nDataOutLen))
		//{
		//	ucACType = (sDataOut[4] >> 4) & 0x03; // 20090316 修改类型计算方式
		//}

		//if (ucACType == AC_ARQC)
		//{
		//	iRet = ProcClssTxnOnline();
		//	return FinishClssTxn(iRet);
		//}
		//else if (ucACType == AC_TC)
		//{
		//	ProcClssTxnOffline();
		//}
		//else
		//{
		//	ProcError_Wave(RC_FAILURE);
		//	//2014-5-13 tt enhance START
		//	CommOnHook(FALSE);
		//	ClssOpen(); //2014-5-16 enhance
		//	//2014-5-13 tt enhance END
		//	memcpy(sMsgIDList,"\x05\x01",2);// WAVE_MSG_DECLINE
		//	WaveShowStatus(1, sMsgIDList, 2);
		//	return ERR_NO_DISP;
		//}
	}

	return 0;
}

//非接定时器检测
void ClssCheckTimer(void)
{
	int		iRet;

	if (TimerCheck(TIMER_POLL) == 0)
	{
		TimerSet(TIMER_POLL, sg_nPollTime);
		iRet = ClssProcPoll(FALSE);
		ClssClose(); //2014-5-16 enhance
		if (iRet)
		{
			ProcError_Wave(iRet);
		}
	}
}

//----------------------------------------------------------------------------------
//                                 
//                                     供内部访问函数
//									   Forinternal call functions
//
//-----------------------------------------------------------------------------------

//POLL
int ClssProcPoll(uchar ucDispType)
{
	int iRet;

	iRet = WavePOLL();
	if (iRet == RC_POLL_P)
	{	
		if (ucDispType)
		{
			ScrCls();
			ScrPrint(0,2,0,_T("Auth With Reader..."));
		}
		
		iRet = WavePollAuthProc();
		if (iRet)
		{ 
			return iRet;
		}		
		else
		{	
			if (ucDispType)
			{
				ScrPrint(0,3,0,_T("Auth  OK"));
			}

			//POLL
			iRet = WavePOLL();
		}
	}
	else if (iRet != RC_POLL_A)
	{
		return iRet;
	}

	return 0;
}




void ClssPoll(void)// function added when press down key
{
	int		iRet;	
#if defined(_P60_S1_) || defined(_S_SERIES_)
	SetOffBase(OffBaseDisplay);
#endif
#ifdef _P60_S1_
	PortClose(0);
	PortClose(3);
#endif
	
	ClssOpen(); //2014-5-16 enhance
	if(!sg_bWaveInitPara) 
    {
		sg_bWaveInitPara = 1;
		iRet = WaveInitPara(glSysParam.stEdcInfo.ucClssComm);
		if (iRet)
		{
		ProcError_Wave(iRet);
		return ;
	}
    }

	
	//查询当前读卡器连接状态
	iRet = ClssProcPoll(TRUE);
	if (iRet)
	{
		ProcError_Wave(iRet);
		return ;
	}
	SetReaderLan();//2016-3-9 add for set R50 Language
	SetInternalPara();
	//SetRdTime();
	//启动POLL定时器
	iRet = WaveMngInternalPara('G', &sg_RdInternalPara);
	if (iRet == 0)
	{
		sg_nPollTime = ((sg_RdInternalPara.aucPollMsg[0]<<8) + sg_RdInternalPara.aucPollMsg[1])*10-3;
		TimerSet(TIMER_POLL, sg_nPollTime);
	}	
	bFirstPollFlag = 0;
#ifdef _S60_ 
	if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
		|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
	{
		ClssClose(); //2014-5-16 enhance
		glProcInfo.bIfClssTxn = 1;
	}
#endif
	return ;

}

//处理START TRANS的返回值
void ProcStartTransRet(int iRet)
{
	ST_EVENT_MSG	stEventMsg;
	int iResult;
	memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));

	switch(iRet)
	{
	case SWIPED_MAGCARD:
		ClssClose(); //2014-5-16 enhance
		 if(glProcInfo.stTranLog.ucTranType != REFUND)//Jason 34-08 2016.03.01 15:24//2016-3-7 AE Refund
		 {
		stEventMsg.MsgType = MAGCARD_MSG;
		stEventMsg.MagMsg.RetCode = MagRead(stEventMsg.MagMsg.track1,
			stEventMsg.MagMsg.track2,
			stEventMsg.MagMsg.track3);
		
		memcpy(glAmount, glProcInfo.stTranLog.szAmount, 12);
#ifdef AMT_PROC_DEBUG
		//2014-9-18 each time come to glAmount or szAmount
		sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(08)%.13s",glAmount);
		sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(08)%.13s",glProcInfo.stTranLog.szAmount);
		glProcInfo.stTranLog.cntAmt++;
	//	//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 7: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
		if (ERR_NEED_INSERT==event_main(&stEventMsg))
		{
			//2014-5-13 tt enhance START
			ClssOpen(); //2014-5-16 enhance
			//2014-5-13 tt enhance END
		}
		else
		{
			CommOnHook(FALSE);
			//2014-5-13 tt enhance START
			ClssOpen(); //2014-5-16 enhance
			//2014-5-13 tt enhance END
			break;
		}
		 }

	case INSERTED_ICCARD:
		ClssClose(); //2014-5-16 enhance
		if(glProcInfo.stTranLog.ucTranType != REFUND)//Jason 34-08 2016.03.01 15:24 //2016-3-7 AE Refund
		{
		stEventMsg.MsgType = ICCARD_MSG;
	 	memcpy(glAmount, glProcInfo.stTranLog.szAmount, 12);
#ifdef AMT_PROC_DEBUG
		//2014-9-18 each time come to glAmount or szAmount
		sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(09)%.13s",glAmount);
		sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(09)%.13s",glProcInfo.stTranLog.szAmount);
		glProcInfo.stTranLog.cntAmt++;
	//	//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 8: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
		event_main(&stEventMsg);
		CommOnHook(FALSE);
		//2014-5-13 tt enhance START
		ClssOpen(); //2014-5-16 enhance
		//2014-5-13 tt enhance END
		}
		break;

	case CLSS_USER_CANCEL:
// 		stEventMsg.MsgType  = KEYBOARD_MSG;
// 		stEventMsg.KeyValue = getkey();
// 		if (stEventMsg.KeyValue >= KEY0 && stEventMsg.KeyValue <= KEY9)
// 		{
// 			memcpy(glAmount, glProcInfo.stTranLog.szAmount, 12);
// 			event_main(&stEventMsg);
// 			CommOnHook(FALSE);
// 		}
// 		 if (stEventMsg.KeyValue == KEYCANCEL)
// 		{

			EcrSendTranFail();
			ScrClrLine(2,7);
			PubDispString(_T("USER CANCEL")        ,4|DISP_LINE_CENTER);
			PubWaitKey(1);
	// 	}
		break;

	default:
#ifdef AMT_PROC_DEBUG//2014-9-22
	glProcInfo.stTranLog.cntAmt = 0;
#endif
		EcrSendTranFail(); //2015-8-26 ttt
		ProcError_Wave(iRet);
		// Gillian remove 20160930
		//2015-8-26 PayPass3.0 Test======================================START
		//ClssClose();
		/*PortClose(glSysParam.stEdcInfo.ucClssComm);
        sg_bCommOpen = 0;
		//ScrCls();
		//Disp2StringBox("┶荡╃ユ", "PayPass Declined");
		//DelayMs(2000);

		DispTransName(); 
		DispSwipeCard(FALSE);
		//DelayMs(2000);
		memcpy(glAmount, glProcInfo.stTranLog.szAmount, 12);
		iRet = TransSale(FALSE);*/
		break;
	}
}

//处理PayWave返回值
void ProcError_Wave(int nRspCode)
{
	ScrClrLine(2,7);
	switch (nRspCode)
	{
	case RC_SUCCESS              : PubDispString(_T("SUCCESS")          ,4|DISP_LINE_CENTER); break;       
	case RC_DATA                 : PubDispString(_T("CARD DATA")        ,4|DISP_LINE_CENTER); break;         
	case RC_POLL_A               : PubDispString(_T("POLL A")           ,4|DISP_LINE_CENTER); break;     
	case RC_POLL_P               : PubDispString(_T("POLL P")           ,4|DISP_LINE_CENTER); break;          
	case RC_SCHEME_SUPPORTED     : PubDispString(_T("SCHEME")           ,4|DISP_LINE_CENTER); break;         
	case RC_SIGNATURE            : PubDispString(_T("SIGNATURE")        ,4|DISP_LINE_CENTER); break;      
	case RC_ONLINE_PIN           : PubDispString(_T("PIN_ONLINE")       ,4|DISP_LINE_CENTER); break;      
	case RC_OFFLINE_PIN          : PubDispString(_T("PIN_OFFLINE")      ,4|DISP_LINE_CENTER); break;    
	case RC_FAILURE              : PubDispString(_T("FAILURE")          ,4|DISP_LINE_CENTER); break;         
	case RC_ACCESS_NOT_PERFORMED : PubDispString(_T("NO ACCESS")        ,4|DISP_LINE_CENTER); break;     
	case RC_ACCESS_FAILURE       : PubDispString(_T("FAIL ACCESS")      ,4|DISP_LINE_CENTER); break;        
	case RC_AUTH_FAILURE         : PubDispString(_T("FAIL AUTH")        ,4|DISP_LINE_CENTER); break;         
	case RC_AUTH_NOT_PERFORMED   : PubDispString(_T("NO AUTH")          ,4|DISP_LINE_CENTER); break;      
	case RC_DDA_AUTH_FAILURE     : PubDispString(_T("CARD FAIL")        ,4|DISP_LINE_CENTER); break;        
	case RC_INVALID_COMMAND      : PubDispString(_T("NO MSG ID")        ,4|DISP_LINE_CENTER); break;    
	case RC_INVALID_DATA         : PubDispString(_T("DATA INCORRECT")   ,4|DISP_LINE_CENTER); break;     
	case RC_INVALID_PARAM        : PubDispString(_T("INVALID PARA")     ,4|DISP_LINE_CENTER); break;       
	case RC_INVALID_KEYINDEX     : PubDispString(_T("BAD KEYID")        ,4|DISP_LINE_CENTER); break;     
	case RC_INVALID_SCHEME       : PubDispString(_T("NO SCHEME")        ,4|DISP_LINE_CENTER); break;         
	case RC_INVALID_VISA_CA_KEY  : PubDispString(_T("BAD Visa CAKEYID") ,4|DISP_LINE_CENTER); break;    
	case RC_MORE_CARDS           : PubDispString(_T("Pls Slt 1 Card")   ,4|DISP_LINE_CENTER); break;    
	case RC_NO_CARD              : PubDispString(_T("NO CARD")          ,4|DISP_LINE_CENTER); break;    
	case RC_NO_EMV_TAGS          : PubDispString(_T("NO TAGS")          ,4|DISP_LINE_CENTER); break;      
	case RC_NO_PARAMETER         : PubDispString(_T("NO PARA")          ,4|DISP_LINE_CENTER); break;       
	case RC_POLL_N               : PubDispString(_T("POLL N")           ,4|DISP_LINE_CENTER); break;      
	case RC_OTHER_AP_CARDS       : PubDispString(_T("Other AP Cards")   ,4|DISP_LINE_CENTER); break;    
	case RC_US_CARDS             : PubDispString(_T("US Cards")         ,4|DISP_LINE_CENTER); break;    
	case RC_NO_PIN               : PubDispString(_T("No PIN")           ,4|DISP_LINE_CENTER); break;    
	case RC_NO_SIG               : PubDispString(_T("No Signature")     ,4|DISP_LINE_CENTER); break;    
	case RC_INVALID_JCB_CA_KEY   : PubDispString(_T("BAD JCB CAKEYID")  ,4|DISP_LINE_CENTER); break;    		
	case INPUT_ERROR             : PubDispString(_T("Input Error!")     ,4|DISP_LINE_CENTER); break;    
	case CLSS_USER_CANCEL		 : PubDispString(_T("Cancelled!")       ,4|DISP_LINE_CENTER); break;    
	case INSERTED_ICCARD         : PubDispString(_T("Card Inserted!")   ,4|DISP_LINE_CENTER); break;    
	case SWIPED_MAGCARD          : PubDispString(_T("Mag Card Swiped!") ,4|DISP_LINE_CENTER); break;    
	case RC_DECLINED             : PubDispString(_T("Declined")         ,4|DISP_LINE_CENTER); break;    
	case 0xE9                    : PubDispString(_T("PLS USE CONTACT!") ,4|DISP_LINE_CENTER); break; //2016-3-31
	default : 
		if (nRspCode < 0)
		{
			ScrPrint(0,4,1,_T("PLS TRY AGAIN"));
			ScrPrint(0,6,0,_T("System Error: %d "), nRspCode); 
		}
		else
		{
			ScrPrint(0,4,1,_T("PLS TRY AGAIN"));
			ScrPrint(0,6,0,_T("System Error: %02X "), nRspCode); 
		}
		break;
	} 

	PubWaitKey(3);
}

//设置支持非接 
void SetSupportClss(void)
{
	uchar 	ucKey;

	//while( 1 ) //Gillian 20160930
	if(1)
	{
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("SET SUPPORT CLSS"));

		if(glSysParam.stEdcInfo.ucClssFlag == 0)
		{
			PubDispString(_T("Status:Disable"), DISP_LINE_RIGHT|3);
		}
		else
		{
			PubDispString(_T("Status:Enable"), DISP_LINE_RIGHT|3);
		}	

		PubDispString(_T("0.Dis   1.Enable"), DISP_LINE_CENTER|6);
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( /*ucKey==KEYENTER || */ucKey==KEYCANCEL || ucKey==NOKEY)
		{
			return;
		//	break;
		}
		if(ucKey == KEY0) 
		{
			glSysParam.stEdcInfo.ucClssFlag = 0;
			SaveSysParam();		
//			ScrClrLine(2,7);
//			PubDispString(_T("SUCCESS")          ,4|DISP_LINE_CENTER);
//			PubWaitKey(2);
//			break;
		}
		if(ucKey == KEY1) 
		{
			glSysParam.stEdcInfo.ucClssFlag = 1;
			SaveSysParam();		
//			ScrClrLine(2,7);
//			PubDispString(_T("SUCCESS")          ,4|DISP_LINE_CENTER);
//			PubWaitKey(2);
//			break;
		}

		//support PayWave
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("SUPPORT PAYWAVE"));

		if(glSysParam.stEdcInfo.ucClssEnbFlag & CLSS_TYPE_VISA)
		{
			PubDispString(_T("Status:Enable"), DISP_LINE_RIGHT|3);
		}
		else
		{
			PubDispString(_T("Status:Disable"), DISP_LINE_RIGHT|3);
		}	

		PubDispString(_T("0.Dis   1.Enable"), DISP_LINE_CENTER|6);
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( /*ucKey==KEYENTER || */ucKey==KEYCANCEL || ucKey==NOKEY)
		{
			return;
		//	break;
		}
		if(ucKey == KEY0) 
		{
			glSysParam.stEdcInfo.ucClssEnbFlag &= ~CLSS_TYPE_VISA;
			SaveSysParam();		
		}
		if(ucKey == KEY1) 
		{
			glSysParam.stEdcInfo.ucClssEnbFlag |= CLSS_TYPE_VISA;
			SaveSysParam();		
		}

		//support PayPass
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("SUPPORT PAYPASS"));

		if(glSysParam.stEdcInfo.ucClssEnbFlag & CLSS_TYPE_MC)
		{
			PubDispString(_T("Status:Enable"), DISP_LINE_RIGHT|3);
		}
		else
		{
			PubDispString(_T("Status:Disable"), DISP_LINE_RIGHT|3);
		}	

		PubDispString(_T("0.Dis   1.Enable"), DISP_LINE_CENTER|6);
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( /*ucKey==KEYENTER ||*/ ucKey==KEYCANCEL || ucKey==NOKEY)
		{
			return;
		//	break;
		}
		if(ucKey == KEY0) 
		{
			glSysParam.stEdcInfo.ucClssEnbFlag &= ~CLSS_TYPE_MC;
			SaveSysParam();		
		}
		if(ucKey == KEY1) 
		{
			glSysParam.stEdcInfo.ucClssEnbFlag |= CLSS_TYPE_MC;
			SaveSysParam();		
		}

		//support AMEX ExpressPay
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("SUPPORT EXPRESS"));

		if(glSysParam.stEdcInfo.ucClssEnbFlag & CLSS_TYPE_AE)
		{
			PubDispString(_T("Status:Enable"), DISP_LINE_RIGHT|3);
		}
		else
		{
			PubDispString(_T("Status:Disable"), DISP_LINE_RIGHT|3);
		}	

		PubDispString(_T("0.Dis   1.Enable"), DISP_LINE_CENTER|6);
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( /*ucKey==KEYENTER || */ucKey==KEYCANCEL || ucKey==NOKEY)
		{
			return;
		//	break;
		}
		if(ucKey == KEY0) 
		{
			glSysParam.stEdcInfo.ucClssEnbFlag &= ~CLSS_TYPE_AE;
			SaveSysParam();		
		}
		if(ucKey == KEY1) 
		{
			glSysParam.stEdcInfo.ucClssEnbFlag |= CLSS_TYPE_AE;
			SaveSysParam();		
		}
#if 0
		//===================Gillian 20161101====================//
		//support AMEX ExpressPay
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("SUPPORT J/Speedy"));

		if(glSysParam.stEdcInfo.ucClssEnbFlag & CLSS_TYPE_JSpeedy)
		{
			PubDispString(_T("Status:Enable"), DISP_LINE_RIGHT|3);
		}
		else
		{
			PubDispString(_T("Status:Disable"), DISP_LINE_RIGHT|3);
		}	

		PubDispString(_T("0.Dis   1.Enable"), DISP_LINE_CENTER|6);
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( /*ucKey==KEYENTER || */ucKey==KEYCANCEL || ucKey==NOKEY)
		{
			return;
		//	break;
		}
		if(ucKey == KEY0) 
		{
			glSysParam.stEdcInfo.ucClssEnbFlag &= ~CLSS_TYPE_JSpeedy;
			SaveSysParam();		
		}
		if(ucKey == KEY1) 
		{
			glSysParam.stEdcInfo.ucClssEnbFlag |= CLSS_TYPE_JSpeedy;
			SaveSysParam();		
		}
		//=========================END=========================//
#endif	
	}
}

//设置非接交易进入的模式
int SetSupportClssMode(void)
{
	uchar 	ucKey, ucRet;
	uchar	szCurrName[4+1];
	uchar	*pszAmt;

	while( 1 )
	{
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("SET CLSS MODE"));

		if(glSysParam.stEdcInfo.ucClssMode == 0)
		{
			PubDispString(_T("Status:Idle AMT"), DISP_LINE_RIGHT|2);
		}
		else if(glSysParam.stEdcInfo.ucClssMode == 1)
		{
			PubDispString(_T("Status:F4 SWITCH"), DISP_LINE_RIGHT|2);
		}
		else if(glSysParam.stEdcInfo.ucClssMode == 2)
		{
			PubDispString(_T("Status:FIXED AMT"), DISP_LINE_RIGHT|2);
		}
		
		PubDispString(_T("0.IdleAmt 1.F4Sw"), DISP_LINE_LEFT|4);
		PubDispString(_T("2.FixedAmt      "), DISP_LINE_LEFT|6);

		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( ucKey==KEYENTER || ucKey==KEYCANCEL || ucKey==NOKEY)
		{
			break;
		}
		if(ucKey == KEY0) 
		{
			glSysParam.stEdcInfo.ucClssMode = 0;
			SaveSysParam();
			break;
		}
		if(ucKey == KEY1) 
		{
			glSysParam.stEdcInfo.ucClssMode = 1;
			SaveSysParam();
			break;
		}
		if(ucKey == KEY2) 
		{
			//set fixed amount
			ScrClrLine(2,7);
			PubDispString(_T("ENTER AMOUNT"), 2|DISP_LINE_LEFT);
			
			pszAmt = glSysParam.stEdcInfo.ucClssFixedAmt;
			sprintf((char *)szCurrName, "%.3s", glSysParam.stEdcInfo.stLocalCurrency.szName);
			ucRet = PubGetAmount(szCurrName, glProcInfo.stTranLog.stTranCurrency.ucDecimal, 1, 6, pszAmt, USER_OPER_TIMEOUT, 0,0);
			if (ucRet != 0)
			{
				return ucRet;
			}
			AmtConvToBit4Format(pszAmt, glProcInfo.stTranLog.stTranCurrency.ucIgnoreDigit);
			glSysParam.stEdcInfo.ucClssMode = 2;
			SaveSysParam();

			break;
		}
	}

	ScrClrLine(2,7);
	PubDispString("SUCCESS"          ,4|DISP_LINE_CENTER);
	PubWaitKey(2);
	return 0;
}

//设置时间
void SetRdTime(void)
{
	int		iRet;
	uchar	szDateTime[16+1], szBuf[7+1];
#ifdef _S60_ 
	if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
		|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
	{
		ClssOpen(); //2014-5-16 enhance
	}
#endif
	ScrCls();
	PubShowTitle(TRUE, (uchar *)_T("    SET TIME    "));

	PubGetDateTime(szDateTime);
	memset(szBuf, 0, sizeof(szBuf));
	PubAsc2Bcd(szDateTime, 14, szBuf);

	//仿真发现如果不获取,设置就会失败
	iRet = WaveMngDateTime('G', szDateTime);
	iRet = WaveMngDateTime('S', szBuf);
	ProcError_Wave(iRet);
#ifdef _S60_ 
	if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
		|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
	{
		ClssClose(); //2014-5-16 enhance
	}
#endif
}

//设置非接连接reader的端口
void SetClssComm(void) 
{
	int	 iMenuNo;
	static	MenuItem stTranMenu[5] =
	{
		{TRUE, _T_NOOP("RS232"),		NULL},
		{TRUE, _T_NOOP("PAD"),		NULL},
		{TRUE, "", NULL},
	};
	static	uchar	szPrompt[]       = _T_NOOP("PLS SELECT:");

	while( 1 )
	{
		iMenuNo = PubGetMenu((uchar *)_T(szPrompt), stTranMenu, MENU_AUTOSNO|MENU_PROMPT, USER_OPER_TIMEOUT);
		switch( iMenuNo )
		{
		case 0:
			PubShowTitle(TRUE, (uchar *)_T("RS232"));
			glSysParam.stEdcInfo.ucClssComm = 0;
			SaveSysParam();
			ScrClrLine(2,7);
			PubDispString(_T("RS232 SELECTED"), 3|DISP_LINE_CENTER);
			PubWaitKey(3);
			break;
		case 1:
			PubShowTitle(TRUE, (uchar *)_T("PINPAD"));
			glSysParam.stEdcInfo.ucClssComm = 3;
			SaveSysParam();
			ScrClrLine(2,7);
			PubDispString(_T("PINPAD SELECTED"), 3|DISP_LINE_CENTER);
			PubWaitKey(3);
			break;
		}
		return;
	}
}
//设置非接offline limit 和 signature limit 和最高交易限额
void SetClssFlrLmt(void) 
{
	int		iMenuNo;
	uchar	pszAmt[9/*6*/], ucRet;  // /Gillian 2016-6-23
	uchar	szCurrName[4+1], szCurrAmt[6+1],szTem[16+1],szCurrAmt2[16+1],szCurrAmt3[16+1];
	uchar	szBuff[5+1];//2015-10-13
	

	static	MenuItem stTranMenu[7] =
	{
		{TRUE, _T_NOOP("CLSS MAX LIMIT"),		NULL},
		{TRUE, _T_NOOP("OFFLINE LIMIT"),		NULL},
		{TRUE, _T_NOOP("CVM LIMIT"),		NULL},
		{TRUE, _T_NOOP("ODCV FLOOR LIMIT"),		NULL},//2015-10-13
		{TRUE, _T_NOOP("FOREIGN CVM LMT"),		NULL},//2015-10-13
		{TRUE, _T_NOOP("FOREIGN OFF LMT"),		NULL},//2015-11-12
		{TRUE, "", NULL},
	};
	static	uchar	szPrompt[]       = _T_NOOP("PLS SELECT:");
	
	while( 1 )
	{
		iMenuNo = PubGetMenu((uchar *)_T(szPrompt), stTranMenu, MENU_AUTOSNO|MENU_PROMPT, USER_OPER_TIMEOUT);
		switch( iMenuNo )
		{
		case 0:
			PubShowTitle(TRUE, (uchar *)_T("CLSS MAX LIMIT"));
			ScrClrLine(2,7);
			PubLong2Bcd(glSysParam.stEdcInfo.ulClssMaxLmt,5,szCurrAmt);
			PubBcd2Asc0(szCurrAmt, 5,szCurrAmt3);

			PubConvAmount(NULL,szCurrAmt3,
			glProcInfo.stTranLog.stTranCurrency.ucDecimal,0,
			szCurrAmt2,0);
			sprintf(szTem, _T("CURR:%s"),szCurrAmt2);

			PubDispString(_T(szTem), 2|DISP_LINE_LEFT);
			PubDispString(_T("ENTER AMOUNT"), 4|DISP_LINE_LEFT);
			
			sprintf((char *)szCurrName, "%.3s", glSysParam.stEdcInfo.stLocalCurrency.szName);
			ucRet = PubGetAmount(szCurrName, glProcInfo.stTranLog.stTranCurrency.ucDecimal, 1, 9/*6*/, pszAmt, USER_OPER_TIMEOUT, 0,0); //Gillian 2016-6-23
			if (ucRet != 0)
			{
				return;
			}
		
			if (ConfirmAmount("MAX LIMIT",pszAmt))
			{
				PubAsc2Long(pszAmt,9/*6*/, &glSysParam.stEdcInfo.ulClssMaxLmt);
			}
			else
			{
				break;
			}
			SaveSysParam();
			InitClssParam();
			ScrClrLine(2,7);
			PubDispString(_T("SUCCESS"), 4|DISP_LINE_CENTER);
			PubWaitKey(3);
			break;
		case 1:
			PubShowTitle(TRUE, (uchar *)_T("OFFLINE LIMIT"));
			ScrClrLine(2,7);
			PubLong2Bcd(glSysParam.stEdcInfo.ulClssOffFLmt,4,szCurrAmt);
			PubBcd2Asc0(szCurrAmt,4,szCurrAmt3);
			PubConvAmount(NULL,szCurrAmt3,
							glProcInfo.stTranLog.stTranCurrency.ucDecimal,0,
							szCurrAmt2,0);
			sprintf(szTem, _T("CURR: %s"),szCurrAmt2);
			PubDispString(_T(szTem), 2|DISP_LINE_LEFT);
			PubDispString(_T("ENTER AMOUNT"), 4|DISP_LINE_LEFT);
		
			sprintf((char *)szCurrName, "%.3s", glSysParam.stEdcInfo.stLocalCurrency.szName);
			ucRet = PubGetAmount(szCurrName, glProcInfo.stTranLog.stTranCurrency.ucDecimal, 1, 6, pszAmt, USER_OPER_TIMEOUT, 0,0);
			if (ucRet != 0)
			{
				return;
			}
			if (ConfirmAmount("OFFLINE LIMIT",pszAmt))
			{
				PubAsc2Long(pszAmt,6, &glSysParam.stEdcInfo.ulClssOffFLmt);
			}
			else
			{
				break;
			}
			SaveSysParam();
			InitClssParam();
			ScrClrLine(2,7);
			PubDispString(_T("SUCCESS"), 4|DISP_LINE_CENTER);
			PubWaitKey(3);
			break;
		case 2:
			PubShowTitle(TRUE, (uchar *)_T("CVM LIMIT"));
			ScrClrLine(2,7);
			PubLong2Bcd(glSysParam.stEdcInfo.ulClssSigFlmt,4,szCurrAmt);
			PubBcd2Asc0(szCurrAmt,4,szCurrAmt3);
			PubConvAmount(NULL,szCurrAmt3,
				glProcInfo.stTranLog.stTranCurrency.ucDecimal,0,
				szCurrAmt2,0);
			sprintf(szTem, _T("CURR: %s"),szCurrAmt2);
			PubDispString(_T(szTem), 2|DISP_LINE_LEFT);
			PubDispString(_T("ENTER AMOUNT"), 4|DISP_LINE_LEFT);

			sprintf((char *)szCurrName, "%.3s", glSysParam.stEdcInfo.stLocalCurrency.szName);
			ucRet = PubGetAmount(szCurrName, glProcInfo.stTranLog.stTranCurrency.ucDecimal, 1, 6, pszAmt, USER_OPER_TIMEOUT, 0,0);
			if (ucRet != 0)
			{
				return;
			}
			if (ConfirmAmount("CVM LIMIT",pszAmt))
			{
				PubAsc2Long(pszAmt,6, &glSysParam.stEdcInfo.ulClssSigFlmt);
			}
			else
			{
				break;
			}
			SaveSysParam();
			InitClssParam();
			ScrClrLine(2,7);
			PubDispString(_T("SUCCESS"), 4|DISP_LINE_CENTER);
			PubWaitKey(3);
			break;
		case 3:
			PubShowTitle(TRUE, (uchar *)_T("ODCV FLOOR LIMIT"));
			ScrClrLine(2,7);
			PubLong2Bcd(glSysParam.stEdcInfo.ulODCVClssFLmt,4,szCurrAmt);
			PubBcd2Asc0(szCurrAmt,4,szCurrAmt3);
			PubConvAmount(NULL, szCurrAmt3, glProcInfo.stTranLog.stTranCurrency.ucDecimal, 0,szCurrAmt2,0);			//2013-6-20 disp amount limit

			sprintf(szTem, _T("CURR:%s"),szCurrAmt2);
			PubDispString(_T(szTem), 2|DISP_LINE_LEFT);
			PubDispString(_T("ENTER AMOUNT"), 4|DISP_LINE_LEFT);
			sprintf((char *)szCurrName, "%.3s", glSysParam.stEdcInfo.stLocalCurrency.szName);//stLocalCurrency.szName
			ucRet = PubGetAmount(szCurrName, glProcInfo.stTranLog.stTranCurrency.ucDecimal, 1, 9, pszAmt, USER_OPER_TIMEOUT, 0,0);
			if (ucRet != 0)
			{
				return;
			}
			if (ConfirmAmount(NULL, pszAmt))		//for test
			{
				PubAsc2Long(pszAmt,9, &glSysParam.stEdcInfo.ulODCVClssFLmt);
			}
			else	//for test
			{
				break;
			}			
			SaveSysParam();
			InitClssParam();
			ScrClrLine(2,7);
			PubDispString(_T("SUCCESS"), 4|DISP_LINE_CENTER);
			PubWaitKey(3);
			break;
		case 4://2015-10-7 
			PubShowTitle(TRUE, (uchar *)_T("FOREIGN CVM LMT"));
			ScrClrLine(2,7);
			PubLong2Bcd(glSysParam.stEdcInfo.ulForeignCVMLmt,4,szCurrAmt);
			PubBcd2Asc0(szCurrAmt,4,szCurrAmt3);
			PubConvAmount(NULL, szCurrAmt3, glProcInfo.stTranLog.stTranCurrency.ucDecimal, 0,szCurrAmt2,0);			//2013-6-20 disp amount limit

			sprintf(szTem, _T("CURR:%s"),szCurrAmt2);
			PubDispString(_T(szTem), 2|DISP_LINE_LEFT);
			PubDispString(_T("ENTER AMOUNT"), 4|DISP_LINE_LEFT);
			sprintf((char *)szCurrName, "%.3s", glProcInfo.stTranLog.stTranCurrency.szName);//stLocalCurrency.szName
			ucRet = PubGetAmount(szCurrName, glProcInfo.stTranLog.stTranCurrency.ucDecimal, 1, 6, pszAmt, USER_OPER_TIMEOUT, 0,'a');
			if (ucRet != 0)
			{
				return;
			}
			DispAmount(2, pszAmt);//2015-11-12
#ifndef ENGLISH_VER
			PubDispString("琌タ絋",      DISP_LINE_LEFT|4);
#endif
			PubDispString("CORRECT ? Y/N", DISP_LINE_LEFT|6);
			if (!PubYesNo(USER_OPER_TIMEOUT))		//for test//2016-3-24 bug fix
			{
				PubAsc2Long(pszAmt,6, &glSysParam.stEdcInfo.ulForeignCVMLmt);
			}
			else	//for test
			{
				break;
			}			
			SaveSysParam();
			//InitClssParam();
			ScrClrLine(2,7);
			PubDispString(_T("SUCCESS"), 4|DISP_LINE_CENTER);
			PubWaitKey(3);
			break;
		case 5://2015-10-7 
			PubShowTitle(TRUE, (uchar *)_T("FOREIGN OFF LMT"));
			ScrClrLine(2,7);
			PubLong2Bcd(glSysParam.stEdcInfo.ulForeignOFFLmt,4,szCurrAmt);
			PubBcd2Asc0(szCurrAmt,4,szCurrAmt3);
			PubConvAmount(NULL, szCurrAmt3, glProcInfo.stTranLog.stTranCurrency.ucDecimal, 0,szCurrAmt2,0);			//2013-6-20 disp amount limit

			sprintf(szTem, _T("CURRENT:%s"),szCurrAmt2);
			PubDispString(_T(szTem), 2|DISP_LINE_LEFT);
			PubDispString(_T("ENTER AMOUNT"), 4|DISP_LINE_LEFT);
			sprintf((char *)szCurrName, "%.3s",  glProcInfo.stTranLog.stTranCurrency.szName);//stLocalCurrency.szName
			ucRet = PubGetAmount(szCurrName, glProcInfo.stTranLog.stTranCurrency.ucDecimal, 1, 6, pszAmt, USER_OPER_TIMEOUT, 0, 'a');
			if (ucRet != 0)
			{
				return;
			}
			DispAmount(2, pszAmt);
#ifndef ENGLISH_VER
			PubDispString("琌タ絋",      DISP_LINE_LEFT|4);
#endif
			PubDispString("CORRECT ? Y/N", DISP_LINE_LEFT|6);
			if (!PubYesNo(USER_OPER_TIMEOUT))		//for test//2016-3-24 bug fix
			{
				PubAsc2Long(pszAmt,6, &glSysParam.stEdcInfo.ulForeignOFFLmt);
			}
			else	//for test
			{
				break;
			}			
			SaveSysParam();
			//InitClssParam();
			ScrClrLine(2,7);
			PubDispString(_T("SUCCESS"), 4|DISP_LINE_CENTER);
			PubWaitKey(3);
			break;
		}
		default_Clsslmt = 1;
		return;
	}

}
//2015-10-14 
void SetClssAPID(void)
{
	uchar ucRet, ucKey;
	ushort ucLen;
	int i;
	uchar szBuff[20+1];
	
	memset(szBuff, 0, 17);
	PubShowTitle(TRUE, (uchar *)_T("SET APID"));
	ScrClrLine(2,7);
	
	if( glSysParam.stEdcInfo.ucClssAPID[0]!=0 )
	{
		PubDispString(glSysParam.stEdcInfo.ucClssAPID, 2|DISP_LINE_LEFT);
	}
	ucRet = PubGetString(NUM_IN, 10, 10, szBuff, USER_OPER_TIMEOUT);
	ucLen = strlen((char*)szBuff);

	memcpy(glSysParam.stEdcInfo.ucClssAPID, szBuff,10); 
	//memcpy(glSysParam.stEdcInfo.ucClssAPID,"\x40\x03\x44\x03\x44",5);//2015-10-7

	////2015-10-7 ttt
	//ScrCls();
	//ScrPrint(0,2,0, "APID: %s",glSysParam.stEdcInfo.ucClssAPID);
	//getkey();
	if( ucRet == 0) 
	{
		ScrClrLine(2,7);
		PubDispString(glSysParam.stEdcInfo.ucClssAPID, 2|DISP_LINE_LEFT);
		PubDispString("Y/N ?", 4|DISP_LINE_LEFT);
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if(ucKey==KEYENTER)
		{
			SaveSysParam();
			ScrClrLine(2,7);
			PubDispString(_T("SUCCESS"), 4|DISP_LINE_CENTER);
			PubWaitKey(3);
		}
	}
	return;
}

//设置密钥
void SetClssKey(void)
{
	int		iRet, iMenuNo;
	uchar	ucTmpBuf[33], ucTmpKey[17];

	static	MenuItem stTranMenu[10] =
	{
		{TRUE, _T_NOOP("SET IMEK_MDK"),		NULL},
		{TRUE, _T_NOOP("SET IAEK_MDK"),		NULL},
		{TRUE, _T_NOOP("SET IMEK"),			NULL},
		{TRUE, _T_NOOP("SET IAEK"),			NULL},
		{TRUE, _T_NOOP("CLEAR MEK"),		NULL},
		{TRUE, _T_NOOP("CLEAR AEK"),		NULL},
		{TRUE, "", NULL},
	};
	
	static	uchar	szPrompt[]       = _T_NOOP("PLS SELECT:");
#ifdef _S60_ 
	if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
		|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
	{
	ClssOpen(); //2014-5-16 enhance
	}
#endif
	while( 1 )
	{
		iMenuNo = PubGetMenu((uchar *)_T(szPrompt), stTranMenu, MENU_AUTOSNO|MENU_PROMPT, USER_OPER_TIMEOUT);
		switch( iMenuNo )
		{
		case 0:
			memset(ucTmpBuf, 0, sizeof(ucTmpBuf));
			memset(ucTmpKey, 0, sizeof(ucTmpKey));
			
			ScrCls();
			PubShowTitle(TRUE, (uchar *)_T("  SET IMEK MDK  "));

			// 20160921 Kenneth add this constant to avoid key in 11111....., 22222... - start
#ifdef ALL_ONE_32			
			memmove(ucTmpBuf, ALL_ONE_32, 32);
			iRet = PubGetString(ALPHA_IN|ECHO_IN, 32, 32, ucTmpBuf, USER_OPER_TIMEOUT);
#else
			iRet = PubGetString(ALPHA_IN, 32, 32, ucTmpBuf, USER_OPER_TIMEOUT);
#endif
			if( iRet!=0 )
			{
				return ;
			}

			PubAsc2Bcd(ucTmpBuf, 32, ucTmpKey);
			GenerateClssKey(WAVE_KEY_IMEK_MDK, ucTmpKey);
			break;
		
		case 1:	
			memset(ucTmpBuf, 0, sizeof(ucTmpBuf));
			memset(ucTmpKey, 0, sizeof(ucTmpKey));

			ScrCls();
			PubShowTitle(TRUE, (uchar *)_T("  SET IAEK MDK  "));
// 20160921 Kenneth add this constant to avoid key in 11111....., 22222... - start
#ifdef ALL_TWO_32			
			memmove(ucTmpBuf, ALL_TWO_32, 32);
			iRet = PubGetString(ALPHA_IN|ECHO_IN, 32, 32, ucTmpBuf, USER_OPER_TIMEOUT);
#else
			iRet = PubGetString(ALPHA_IN, 32, 32, ucTmpBuf, USER_OPER_TIMEOUT);
#endif	
			if( iRet!=0 )
			{
				return ;
			}

			PubAsc2Bcd(ucTmpBuf, 32, ucTmpKey);
			GenerateClssKey(WAVE_KEY_IAEK_MDK, ucTmpKey);
			break;

		case 2:
			memset(ucTmpBuf, 0, sizeof(ucTmpBuf));
			memset(ucTmpKey, 0, sizeof(ucTmpKey));
			
			ScrCls();
			PubShowTitle(TRUE, (uchar *)_T("    SET IMEK    "));
			iRet = PubGetString(ALPHA_IN, 32, 32, ucTmpBuf, USER_OPER_TIMEOUT);
			if( iRet!=0 )
			{
				return ;
			}

			PubAsc2Bcd(ucTmpBuf, 32, ucTmpKey);
			GenerateClssKey(WAVE_KEY_IMEK, ucTmpKey);
			break;
		
		case 3:	
			memset(ucTmpBuf, 0, sizeof(ucTmpBuf));
			memset(ucTmpKey, 0, sizeof(ucTmpKey));

			ScrCls();
			PubShowTitle(TRUE, (uchar *)_T("    SET IAEK    "));
			iRet = PubGetString(ALPHA_IN, 32, 32, ucTmpBuf, USER_OPER_TIMEOUT);
			if( iRet!=0 )
			{
				return ;
			}

			PubAsc2Bcd(ucTmpBuf, 32, ucTmpKey);
			GenerateClssKey(WAVE_KEY_IAEK, ucTmpKey);
			break;
			
		case 4:	
			ScrCls();
			PubShowTitle(TRUE, (uchar *)_T("    CLEAR MEK   "));
			GenerateClssKey(WAVE_KEY_MEK, NULL);
			break;

		case 5:	
			ScrCls();
			PubShowTitle(TRUE, (uchar *)_T("    CLEAR AEK   "));
			GenerateClssKey(WAVE_KEY_AEK, NULL);
			break;

		case -4:
			return;

		default:
			break;	
		}
	}	
}

//导入密钥
void GenerateClssKey(uchar ucKeyType, uchar *pKey)
{
	int		iRet;
	uchar	ucKeyId;
	ushort	usLength;
	uchar	ucTmpBuf[21];
	
	switch(ucKeyType)
	{
	case WAVE_KEY_IMEK_MDK:
	case WAVE_KEY_IAEK_MDK:	
		ucKeyId = 0;
		iRet = WaveKeySetting(ucKeyType, ucKeyId, pKey);
		if (iRet)
		{
			ProcError_Wave(iRet);
		}

		//导入Reader密钥
		memset(ucTmpBuf, 0, sizeof(ucTmpBuf));
		ucTmpBuf[0] = 'L';
		ucTmpBuf[1] = ucKeyType;
		ucTmpBuf[2] = ucKeyId;
		memcpy(ucTmpBuf+3, pKey, 16);
		usLength = 19;
		PortSendstr(glSysParam.stEdcInfo.ucClssComm, ucTmpBuf, usLength);

		ScrClrLine(2,7);
		PubDispString("SUCCESS"          ,4|DISP_LINE_CENTER);
		PubWaitKey(2);
		break;

	case WAVE_KEY_IMEK:
	case WAVE_KEY_IAEK:
		ucKeyId = 0;
		iRet = WaveKeySetting(ucKeyType, ucKeyId, pKey);
		if (iRet)
		{
			ProcError_Wave(iRet);
		}

		//导入Reader密钥
		memset(ucTmpBuf, 0, sizeof(ucTmpBuf));
		ucTmpBuf[0] = 'L';
		ucTmpBuf[1] = ucKeyType;
		ucTmpBuf[2] = ucKeyId;
		memcpy(ucTmpBuf+3, pKey, 16);
		usLength = 19;
		PortSendstr(glSysParam.stEdcInfo.ucClssComm, ucTmpBuf, usLength);

		ScrClrLine(2,7);
		PubDispString(_T("SUCCESS")          ,4|DISP_LINE_CENTER);
		PubWaitKey(2);
		break;
		
	case WAVE_KEY_AEK:
	case WAVE_KEY_MEK:
		ucKeyId = 1;
		iRet = WaveKeySetting(ucKeyType, ucKeyId, NULL);
		ProcError_Wave(iRet);
		break;

	default : 
		ScrPrint(0,4,1,_T("Para Error: %02x "), ucKeyType); 
		PubWaitKey(2);
		break;
	} 
}

//Reader应用更新
int WaveUpdateApp(uchar *pszAppFileName)
{
	int		fd, iRet, iSTartNum, iTotalNum;
	long	lOffset;
	uchar	ucKey, ucSendData[1027], ucRecvData[256];
	ushort  usLenSend, usLenRecv;
	
	if (pszAppFileName == NULL)
	{
		return 1;
	}
	
	fd = open(DOWNRDAPP_FILE, O_RDWR);
	if( fd<0 )
	{
		return fd;
	}

	//按照宏定义, 拆分应用包, 计算包的个数
	lOffset   = filesize(DOWNRDAPP_FILE);
	iTotalNum = ((lOffset%DOWNRDAPP_LEN)==0) ?  (lOffset/DOWNRDAPP_LEN) : (lOffset/DOWNRDAPP_LEN + 1);
	iSTartNum = 0;

	//判断是否断点续传
	if (glSysParam.stEdcInfo.iClssLoadNum != 0)
	{
		//是否断点续传
		kbflush();
		ScrClrLine(2,7);
		ScrPrint(0, 2, ASCII, "Last download unfinished!");
		ScrPrint(0, 4, CFONT, "ENTER : Continue");
		ScrPrint(0, 6, CFONT, "CANCEL: Restart");

		while(1)
		{
			ucKey = PubWaitKey(0);
			switch(ucKey)
			{
			case KEY1:			
			case KEYENTER:
				iSTartNum = glSysParam.stEdcInfo.iClssLoadNum + 1;
				break;

			case KEY2:
			case KEYCANCEL:
				iSTartNum = 0;
				break;

			default:
				continue;
			}
		}
	}

	ScrClrLine(2,7);
	PubDispString(_T("Loading RD APP..."), 4|DISP_LINE_LEFT);

	//发送数据包
	for (; iSTartNum<iTotalNum; iSTartNum++)
	{
		memset(ucSendData, 0, sizeof(ucSendData));
		memset(ucRecvData, 0, sizeof(ucRecvData));
		usLenRecv = 0;

		ucSendData[0] = DOWNRDAPP_FLAG;	//表明是LOAD APP
		ucSendData[1] = (iSTartNum & 0xff00) >> 8;		//第几包
		ucSendData[2] =  iSTartNum & 0x00ff;
		ucSendData[3] = ((iTotalNum-1-iSTartNum) & 0xff00) >> 8;	//后续还有几个包
		ucSendData[4] =  (iTotalNum-1-iSTartNum) & 0x00ff;

		if ((iTotalNum-1-iSTartNum == 0) && ((lOffset%DOWNRDAPP_LEN) != 0)) 
		{
			//最后一个包
			usLenSend = lOffset - (iTotalNum-1)*DOWNRDAPP_LEN + 5;
			memcpy(ucSendData+5, pszAppFileName+(iSTartNum*DOWNRDAPP_LEN), usLenSend);
		}
		else
		{
			//普通包
			memcpy(ucSendData+5, pszAppFileName+(iSTartNum*DOWNRDAPP_LEN), DOWNRDAPP_LEN);
			usLenSend = DOWNRDAPP_LEN+5;
		}	
		
		//发送数据
		iRet = WaveProcOtherProtocol(ucSendData, usLenSend, ucRecvData, &usLenRecv);
		if (iRet)
		{
			ProcError_Wave(iRet);
			return iRet;
		}

		//如果返回的长度为0，说明无返回
		if (usLenRecv == 0)
		{
			return 1;
		}

		//如果不为0,说明Reader出问题
		if (ucRecvData[2] != 0)
		{
			break;
		}

		glSysParam.stEdcInfo.iClssLoadNum = iSTartNum;
		SaveSysParam();
	}	

	//返回数据的机构STX+DATALEN+DATA+ETX
	switch(ucRecvData[2])
	{
	case 0:
		remove(DOWNRDAPP_FILE);
		ScrClrLine(2,7);
		ScrPrint(0,4,1,"Update Success"); 
		PubWaitKey(3);
		break;

	case -1:
		ScrClrLine(2,7);
		ScrPrint(0,4,1,"File NULL"); 
		PubWaitKey(3);
		break;

	case -2:
		ScrClrLine(2,7);
		ScrPrint(0,4,1,"Read Error"); 
		PubWaitKey(3);
		break;

	case -3:
		ScrClrLine(2,7);
		ScrPrint(0,4,1,"Fixed Error"); 
		PubWaitKey(3);
		break;

	case -4:
		ScrClrLine(2,7);
		ScrPrint(0,4,1,"Open Error"); 
		PubWaitKey(3);
		break;

	case -5:
		ScrClrLine(2,7);
		ScrPrint(0,4,1,"File Error"); 
		PubWaitKey(3);
		break;

	case -6:
		ScrClrLine(2,7);
		ScrPrint(0,4,1,"Space Error"); 
		PubWaitKey(3);
		break;

	case -7:
		ScrClrLine(2,7);
		ScrPrint(0,4,1,"Create Error"); 
		PubWaitKey(3);
		break;

	case -8:
		ScrClrLine(2,7);
		ScrPrint(0,4,1,"Open App Error"); 
		PubWaitKey(3);
		break;

	case -9:
		ScrClrLine(2,7);
		ScrPrint(0,4,1,"Fixed App Error"); 
		PubWaitKey(3);
		break;

	default : 
		ScrPrint(0,4,1," Updat Error: %d ", iRet); 
		break;;
	}

	return 0;
}

//设置非接参数
void SetClssParam(void)
{
	int		iRet, iMenuNo;
	

	static	MenuItem stTranMenu[10] =
	{
		{TRUE, _T_NOOP("SUPPORT CLSS"),		NULL},
		{TRUE, _T_NOOP("WAVE POLL"),			NULL},
		{TRUE, _T_NOOP("INIT RD PARA"),		NULL},
		{TRUE, _T_NOOP("SET CLSS MODE"),		NULL},
		{TRUE, _T_NOOP("SET CLSS KEY"),		NULL},
		{TRUE, _T_NOOP("SET RD TIME"),		NULL},
//		{TRUE, _T("UPDATE RD APP"),		NULL},
		{TRUE, _T_NOOP("SET COMM"),			NULL},
		{TRUE, _T_NOOP("SET LIMIT"),   NULL},
		//{TRUE, _T_NOOP("SET APID"),   NULL},//2015-10-14 set local APID  //Gillian 20160920 EDC MAIN remove it
		{TRUE, "", NULL},
	};
	
	static	uchar	szPrompt[]       = _T_NOOP("PLS SELECT:");

#ifdef _S60_ 
	if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
		|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
	{
		ClssClose(); //2014-5-16 enhance
	}
#endif

	// Gillian 2016-6-23

	if(default_Clsslmt == 0)
	{
		glSysParam.stEdcInfo.ulClssMaxLmt = 999999999;
		glSysParam.stEdcInfo.ulClssOffFLmt = 100000;
		glSysParam.stEdcInfo.ulClssSigFlmt = 100000;
	}
	while (1)
	{
		iMenuNo = PubGetMenu((uchar *)_T(szPrompt), stTranMenu, MENU_AUTOSNO|MENU_PROMPT|MENU_CFONT, USER_OPER_TIMEOUT);
		switch( iMenuNo )
		{
		case 0:
			SetSupportClss();
			break;
			
		case 1:
			ScrCls();
			PubShowTitle(TRUE, (uchar *)_T("      POLL      "));
			DispWait();
			iRet = ClssInit();
			if(iRet)
			{
				ProcError_Wave(iRet);
				break;
			}
// 			iRet = SetTLVData();
// 			ProcError_Wave(iRet);
			break;

		case 2:
			//2014-5-13 tt enhance START
			ClssOpen(); //2014-5-16 enhance
			//2014-5-13 tt enhance END

			iRet = InitClssParam();
			ProcError_Wave(iRet);
			break;

		case 3:
			SetSupportClssMode();
			break;	
					
		case 4:
			//2014-5-13 tt enhance START
			ClssOpen(); //2014-5-16 enhance
			//2014-5-13 tt enhance END
			SetClssKey();
#ifdef _S60_ 
			if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
				|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
			{
				ClssClose(); //2014-5-16 enhance
			}
#endif
			break;

		case 5:
			SetRdTime();
			break;

		case 6:
			SetClssComm();
			break;
		case 7:
			//2014-5-13 tt enhance START
			ClssOpen(); //2014-5-16 enhance
			//2014-5-13 tt enhance END
#ifdef _S60_ 
			if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
				|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
			{
			PortOpen(glSysParam.stEdcInfo.ucClssComm, "38400,8,n,1");
			}
#endif
			SetClssFlrLmt();
#ifdef _S60_ 
			if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
				|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
			{
				ClssClose(); //2014-5-16 enhance
			}
#endif
			break;
			
// 		case 7:
// 			WaveUpdateApp(DOWNRDAPP_FILE);
// 			break;
		case 8:
			ClssOpen();
			SetClssAPID();//2015-10-14 set local APID 
			break;
		case -4:
#ifdef _S60_ 
			if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
				|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
			{
				ClssClose(); //2014-5-16 enhance
			}
#endif
			return;

		default:
			break;
		}
	}
}

//----------------------------------------------------------------------------------
//                                 
//									  非接TAG数据的处理
//
//-----------------------------------------------------------------------------------
//解包:交易数据
int UnpackTransData(uchar *pData, ushort nDataLen)
{
	int	j,k;
	ushort nIndex = 0;
	uchar ucLen;

// 		PrnInit();
// 		PrnStr("Recv Length %d",nDataLen);
// 		PrnStr("\n");
// 		PrnStr("Recv Data \n");
// 		for (i=0;i<nDataLen;i++)
// 		{
// 			PrnStr("[%02x] ", *pData++);
// 		}	
// 	 	PrnStr("\n");
// 		PrnStart();

	//Scheme Identifier 1B
	glProcInfo.stWaveTransData.ucSchemeId = *(pData + nIndex); 
	nIndex += 1;

	//Date and Time YYYYMMDDHHMMSS 14 BCD
	PubBcd2Asc0(pData + nIndex, 7, glProcInfo.stTranLog.szDateTime); 
	nIndex += 7;

	while (nIndex < nDataLen)
	{	
		switch (pData[nIndex])
		{		
		case 0xD1:		//Track 1 data
			nIndex += 1;
			ucLen = *(pData + nIndex); 
			nIndex += 1;
			if (ucLen > 79)
			{
				WaveShowStatus(1, "\x05\x01", 2);
				PubDispString(_T("READ TRACK1 ERR"), 2|DISP_LINE_CENTER);
				PubBeepErr();
				PubWaitKey(3);
				return ERR_NO_DISP;
			}
			memcpy(glProcInfo.szTrack1, pData + nIndex, ucLen); 
			nIndex += ucLen;
			break;

		case 0xD2:		//Track 2 data
			nIndex += 1;
			ucLen = *(pData + nIndex); 
			nIndex += 1;	
			if (ucLen > 20)
			{
				WaveShowStatus(1, "\x05\x01", 2);
				PubDispString(_T("READ TRACK2 ERR"), 2|DISP_LINE_CENTER);
				PubBeepErr();
				PubWaitKey(3);
				return ERR_NO_DISP;
			}
			glProcInfo.ucTrack2Len = ucLen;
			memcpy(glProcInfo.szTrack2, pData + nIndex, ucLen);	
			nIndex += ucLen;
			break;

		case 0xD3:		//Chip data
			nIndex += 1;
			ucLen = *(pData + nIndex);
			nIndex += 1;	
			glProcInfo.stWaveTransData.nChipDataLen = ucLen;
			if (ucLen & 0x80) 
			{
				j = (ucLen & 0x7F);            
				ucLen = 0;
				for (k = 0; k < j; k++) 
				{
					ucLen <<= 8;
					ucLen += pData[nIndex];
					nIndex++;
				}
				glProcInfo.stWaveTransData.nChipDataLen = ucLen;
			}
			memcpy(glProcInfo.stWaveTransData.sChipData, pData + nIndex, ucLen);
			nIndex += ucLen;
			//PubDebugOutput("SEND MSG D3:\n",glProcInfo.stWaveTransData.sChipData, glProcInfo.stWaveTransData.nChipDataLen, DEVICE_COM1,HEX_MODE);
			
			break;

		case 0xD4:	//other data
			nIndex += 1;
			ucLen = *(pData + nIndex); 
			nIndex += 1;	
			glProcInfo.stWaveTransData.ucOtherDataLen = ucLen;
			memcpy(glProcInfo.stWaveTransData.sOtherData, pData + nIndex, ucLen);
			nIndex += ucLen;
			UnpackOtherData(glProcInfo.stWaveTransData.sOtherData, ucLen);
			//PubDebugOutput("\n\n\n\n\n\n\nSEND MSG D4:\n",glProcInfo.stWaveTransData.sOtherData, glProcInfo.stWaveTransData.ucOtherDataLen, DEVICE_COM1,HEX_MODE);
			break;

		case 0xD5:		//TRACK 1 Discretionary DATA	Master card
			nIndex += 1;
			ucLen = *(pData + nIndex); nIndex += 1;
			glProcInfo.stWaveTransData.nDDcardTrack1Len = ucLen;
			memcpy(glProcInfo.stWaveTransData.szDDcardTrack1, pData + nIndex, ucLen); 
			nIndex += ucLen;
			break;

		case 0xD6:		//TRACK 2 Discretionary DATA	Master card
			nIndex += 1;
			ucLen = *(pData + nIndex);
			nIndex += 1;	
			glProcInfo.stWaveTransData.nDDcardTrack2Len = ucLen;
			memcpy(glProcInfo.stWaveTransData.szDDcardTrack2, pData + nIndex, ucLen);
			nIndex += ucLen;
			break;

		default:
			nIndex++;
			break;	
		}
	}	

	return 0;
}

//解包:交易数据外的其它数据
void UnpackOtherData(uchar *pDataIn, uchar ucDataInLen)
{
	int nRet;
	uchar sDataOut[256];
	ushort nDataOutLen;
	
	memset(sDataOut, 0, sizeof(sDataOut));
	nDataOutLen = 0;

	if (0 == SearchSpecTLV(0x9F74, pDataIn, ucDataInLen, sDataOut, &nDataOutLen))
	{
		if (nDataOutLen)
		{
			glProcInfo.stWaveTransData.stWaveOtherData.ucVLP_IAC_Flg = 1;
			memcpy(glProcInfo.stWaveTransData.stWaveOtherData.sVLP_IAC, sDataOut, nDataOutLen);
		}
	}
	
	memset(sDataOut, 0, sizeof(sDataOut));
	nDataOutLen = 0;

	if (0 == SearchSpecTLV(0x9F5D, pDataIn, ucDataInLen, sDataOut, &nDataOutLen))
	{
		if (nDataOutLen)
		{
			memcpy(glProcInfo.stWaveTransData.stWaveOtherData.sOffSpendAmt, sDataOut, nDataOutLen);
		}
	}

	memset(sDataOut, 0, sizeof(sDataOut));
	nDataOutLen = 0;

	if (0 == SearchSpecTLV(0x99, pDataIn, ucDataInLen, sDataOut, &nDataOutLen))
	{
		if (nDataOutLen)
		{
			if (nDataOutLen == 1 && sDataOut[0] == 0x00) // 需要Tm执行联机PIN操作
			{
				nRet = cEMVGetHolderPwd(0, 0, NULL);
			}
			else
			{
				memcpy(glProcInfo.stWaveTransData.stWaveOtherData.sOnlinePin, sDataOut, nDataOutLen);
			}
		}
	}

	memset(sDataOut, 0, sizeof(sDataOut));
	nDataOutLen = 0;

	if (0 == SearchSpecTLV(0x55, pDataIn, ucDataInLen, sDataOut, &nDataOutLen))
	{
		if (nDataOutLen)
		{
			if (nDataOutLen == 1 && sDataOut[0] == 0x00) // 需要Tm执行 signature 操作
			{
				glClssCVMFlag = 1;//2016-4-19
//				memset(glProcInfo.stWaveTransData.stWaveOtherData.sSignature, 0, sizeof(glProcInfo.stWaveTransData.stWaveOtherData.sSignature));
			}
			else 
			{
				glClssCVMFlag = 0;//2016-4-19 //Gillian 20160922
//				memcpy(glProcInfo.stWaveTransData.stWaveOtherData.sSignature, sDataOut, nDataOutLen);
			}
		}
	}

	////Gillian 2016-8-12
	memset(sDataOut, 0, sizeof(sDataOut));
	nDataOutLen = 0;
	// Gillian 20160629 applepay signature issue
	if (0 == SearchSpecTLV(0x66, pDataIn, ucDataInLen, sDataOut, &nDataOutLen))
	{
		if (nDataOutLen)
		{
			if (nDataOutLen == 1 && sDataOut[0] == 0x00) // ??nTm???? signature ??@
			{
				 glProcInfo.stTranLog.szCVMRslt[0] = CLSS_CVM_CONSUMER_DEVICE; //by Gillian 20160607

			}
		}
	}


	memset(sDataOut, 0, sizeof(sDataOut));
	nDataOutLen = 0;

	if (0 == SearchSpecTLV(0x01, pDataIn, ucDataInLen, sDataOut, &nDataOutLen))
	{
		if (nDataOutLen)
		{
			// 9F4B 1-dda failed,the iss want to send 
			// the offline cryptogram(TC) for online authorizatio
			glProcInfo.stWaveTransData.stWaveOtherData.ucDDAFailFlg = sDataOut[0]; 
		}
	}
	return;
}

//非接脱机交易处理
int ProcClssTxnOffline(void)
{
	int		iLength;
	ushort	usLength;
	uchar	ucTranAct;

	glProcInfo.stTranLog.uiStatus |= TS_NOSEND;
	GetSpecTLVData(0x9F26, glProcInfo.stTranLog.sAppCrypto, &usLength);
	GetSpecTLVData(0x8A,   glProcInfo.stTranLog.szRspCode,  &usLength);
	GetSpecTLVData(0x95,   glProcInfo.stTranLog.sTVR,       &usLength);
	GetSpecTLVData(0x9B,   glProcInfo.stTranLog.sTSI,       &usLength);
	GetSpecTLVData(0x9F36, glProcInfo.stTranLog.sATC,       &usLength);
	if (GetSpecTLVData(0x5F34, &glProcInfo.stTranLog.ucPanSeqNo, &usLength) == 0)
	{
		glProcInfo.stTranLog.bPanSeqOK = TRUE;
	}
	GetSpecTLVData(0x50,   glProcInfo.stTranLog.szAppLabel, &usLength);		// application label
	GetSpecTLVData(0x4F,   glProcInfo.stTranLog.sAID, &usLength);	// AID
	glProcInfo.stTranLog.ucAidLen = (uchar) usLength;

	// save for upload
	SetClSSDE55(FALSE, glProcInfo.stTranLog.sIccData, &iLength);
	glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;

	GetNewTraceNo();
	sprintf((char *)glProcInfo.stTranLog.szRspCode, "00");
	glProcInfo.stTranLog.ulInvoiceNo = glSysCtrl.ulInvoiceNo;
	if( (glProcInfo.stTranLog.ucTranType==SALE) && (glProcInfo.stTranLog.uiStatus & TS_NOSEND))//!Lois 2013-5-9 offline "APPV:" = "TRACE:"//2016-4-25
	{
		sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%06lu",glSysCtrl.ulInvoiceNo);
	}
	else
	{
		sprintf((char *)glProcInfo.stTranLog.szAuthCode, "%06lu", glSysCtrl.ulSTAN);
	}
	
#if defined(_P60_S1_) || defined(_S_SERIES_)
	SetOffBase(OffBaseDisplay);
#endif
	
	DispTransName();
	DispProcess();
	
	ucTranAct = glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct;

	if (ucTranAct & ACT_INC_TRACE)
	{
		GetNewTraceNo();
	}

	if( ucTranAct & WRT_RECORD )
	{
		glProcInfo.stTranLog.uiStatus |= TS_NOSEND;
		SaveTranLog(&glProcInfo.stTranLog);
#ifdef AMT_PROC_DEBUG
		glProcInfo.stTranLog.cntAmt = 0;//2014-9-19 ack POS to erase amt records
#endif
	}

	EcrSendTransSucceed();

	if( ucTranAct & PRN_RECEIPT )	// print slip
	{
		CommOnHook(FALSE);
		GetNewInvoiceNo();
		PrintReceipt(PRN_NORMAL);
	}
	//2014-5-13 tt enhance START
	ClssOpen(); //2014-5-16 enhance
	//2014-5-13 tt enhance END

	WaveShowStatus(0, "\x02\x01", 2);
	DispResult(0);
	PubWaitKey(glSysParam.stEdcInfo.ucAcceptTimeout);

	return 0;
}

//非接联机交易处理
int ProcClssTxnOnline(void)
{
	int iRet, iLength, iRetryPIN;
    ushort usLength;

#if 0
		// prepare online DE55 data
	iRet = SetClSSDE55(FALSE, &glSendPack.sICCData[2], &iLength);

	if( iRet!=0 )
	{
		return ONLINE_FAILED;
	}

	PubASSERT( iLength<LEN_ICC_DATA );
	PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
	memcpy(glProcInfo.stTranLog.sIccData, &glSendPack.sICCData[2], iLength);	// save for batch upload
	glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
#endif
	//2016-3-7 AE Refund//2016-3-31
    if((ChkIfAmex()) || (glProcInfo.stTranLog.ucTranType != REFUND) && (!ChkIfAmex()))//Jason 34-08 2016.03.01 16:3
    {
		if(glProcInfo.stWaveTransData.ucSchemeId != AE_MAGMODE) //Gillian_1
		{
			// prepare online DE55 data
			iRet = SetClSSDE55(FALSE, &glSendPack.sICCData[2], &iLength);

			if( iRet!=0 )
			{
				CommOnHook(FALSE);
				ClssOpen();
				WaveShowStatus(1, "\x09", 1);
			return ONLINE_FAILED;
			}
			PubASSERT( iLength<LEN_ICC_DATA );
			PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
			memcpy(glProcInfo.stTranLog.sIccData, &glSendPack.sICCData[2], iLength);	// save for batch upload
			glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
		}
	}

	if (GetSpecTLVData(0x5F34, &glProcInfo.stTranLog.ucPanSeqNo, &usLength) == 0)
	{
		glProcInfo.stTranLog.bPanSeqOK = TRUE;
	}
	// 冲正交易处理 & 离线交易上送
	// 判断上次交易是否需要进行冲正等
	// send reversal here. If required by bank, also send offline here
	iRet = TranReversal();

	if( iRet!=0 )
	{	
		return ONLINE_FAILED;
	}

	// citibank need TC before sale, added by laurenc
// 	if(ChkIfCiti())
// 	{
// 		iRet=OfflineSend(OFFSEND_TC);
// 		if(iRet!=0)
// 		{
// 			return ONLINE_FAILED;
// 		}
// 		//脱机上送后，流水号会增加，所以当前交易的流水号需用增加后的流水号 build 1.00.0108b
// 		sprintf((char *)glSendPack.szSTAN, "%06lu", glSysCtrl.ulSTAN);  //
// 		glProcInfo.stTranLog.ulSTAN = glSysCtrl.ulSTAN;
// 	}

	iRetryPIN = 0;
	while( 1 )
	{
		ClssClose(); //2014-5-16 enhance

		iRet = SendRecvPacket();
	
		//2014-5-13 tt enhance START
		ClssOpen(); //2014-5-16 enhance
		//2014-5-13 tt enhance END

		if( iRet!=0 )
		{
			return iRet;
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

	// set response code
	glProcInfo.ucOnlineStatus = ST_ONLINE_APPV;

	if( memcmp(glRecvPack.szRspCode, "00", LEN_RSP_CODE)!=0 )
	{
		return ERR_TRAN_FAIL;
	}
	
	return 0;
}

//非接交易收尾
int FinishClssTxn(int iErrCode)
{
	int		iLength;
	ushort	usLength;
	int iRet;


	if( (glProcInfo.ucOnlineStatus == ST_ONLINE_APPV) && (memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)==0) && (iErrCode == 0))
	{
		// update for reversal(maybe have script result)
		SetClSSDE55(FALSE, &glSendPack.sICCData[2], &iLength);
		PubLong2Char((ulong)iLength, 2, glSendPack.sICCData);
		glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
		SaveRevInfo(TRUE);	// update reversal information

		GetSpecTLVData(0x9F26, glProcInfo.stTranLog.sAppCrypto, &usLength);
		GetSpecTLVData(0x8A,   glProcInfo.stTranLog.szRspCode,  &usLength);
		GetSpecTLVData(0x95,   glProcInfo.stTranLog.sTVR,       &usLength);
		GetSpecTLVData(0x9B,   glProcInfo.stTranLog.sTSI,       &usLength);
		GetSpecTLVData(0x9F36, glProcInfo.stTranLog.sATC,       &usLength);
		GetSpecTLVData(0x50,   glProcInfo.stTranLog.szAppLabel, &usLength);		// application label
		GetSpecTLVData(0x4F,   glProcInfo.stTranLog.sAID,  &usLength);	// AID
		glProcInfo.stTranLog.ucAidLen = (uchar) usLength;
		// save for upload
		SetClSSDE55(TRUE, glProcInfo.stTranLog.sIccData, &iLength);
		glProcInfo.stTranLog.uiIccDataLen = (ushort)iLength;
	}
	
	// 交易失败处理
	if (iErrCode)
	{
		SaveRevInfo(FALSE);
		//2014-5-13 tt enhance START
		ClssOpen(); //2014-5-16 enhance
		//2014-5-13 tt enhance END
		WaveShowStatus(1, "\x9", 1);
		DispResult(iErrCode);
		return ERR_NO_DISP;
	}	
	if (memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)!=0 )
	{
		SaveRevInfo(FALSE);
		DispResult(ERR_HOST_REJ);
		return ERR_NO_DISP;
	}

	if( ChkIfSaveLog() )
	{
		SaveTranLog(&glProcInfo.stTranLog);
#ifdef AMT_PROC_DEBUG
		glProcInfo.stTranLog.cntAmt = 0;//2014-9-19 ack POS to erase amt records
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
	//2014-5-13 tt enhance START
	ClssOpen(); //2014-5-16 enhance
	//2014-5-13 tt enhance END
	WaveShowStatus(0, "\x04", 1);

	EcrSendTransSucceed();

	if( glProcInfo.stTranLog.ucTranType!=RATE_BOC &&
        glProcInfo.stTranLog.ucTranType!=RATE_SCB &&
        glProcInfo.stTranLog.ucTranType!=LOAD_CARD_BIN &&
        glProcInfo.stTranLog.ucTranType!=LOAD_RATE_REPORT)
    {
	//who did this ?Citi should also send tc after a success transaction
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

	CommOnHook(FALSE);

	if( ChkIfPrnReceipt() )
	{
		DispTransName();
		PrintReceipt(PRN_NORMAL);
	}
	DispResult(0);

	return 0;
}

int SetClSSDE55(uchar bForUpLoad, uchar *psOutData, int *piOutLen)
{
	if (bForUpLoad)
	{
		return SetStdDEClSS55(bForUpLoad, sgTcClssTagList, psOutData, piOutLen);
	}
	else
	{
		if((glProcInfo.stTranLog.szPan[0]== '4') || (ChkIfMC(glProcInfo.stTranLog.szPan)))//2016-4-25
        {
            return SetStdDEClSS55(bForUpLoad, sgStdClssTagList, psOutData, piOutLen);
        }
        else  //AMEX
        {
            return SetExtAMEXClSS55(sgAmexClssTagList, psOutData, piOutLen);
        }
	}
}

uchar* GetClssTagDFEF(void)
{
	int		iLength;
	uchar	sTVR[20];

	memset(sTVR, 0, sizeof(sTVR));
	EMVGetTLVData(0x95, sTVR, &iLength);

	if (sTVR[3]&0x80)
	{
		return "1510";		//over floor limit
	}
	else if (sTVR[3]&0x08)	//merchant forced to online
	{
		return "1506";  
	}
	else if (sTVR[3]&0x01)  //randomly selected to online 
	{
		return "1502";  
	}
	else if (sTVR[0]&0xff)  //autentication failed 
	{
		return "1511";  
	}
	else
	{//default
		return "1500";		 
	}
}

// set ADVT/TIP bit 55
int SetStdDEClSS55(uchar bForUpLoad, DE55ClSSTag *pstList, uchar *psOutData, int *piOutLen)
{
	int		iRet, iCnt;
	ushort	iLength;
	uchar	*psTemp, sBuff[200];

	*piOutLen = 0;
	psTemp    = psOutData;

	if (glProcInfo.stWaveTransData.ucSchemeId==SCHEME_MC_MSTR)
	{
		return 0;
	}

	for(iCnt=0; pstList[iCnt].uiEmvTag!=0; iCnt++)
	{
		memset(sBuff, 0, sizeof(sBuff));
		//在非接触L2 的qPBOC及payWave中,'终端性能(9F33)'数据元无法从这两个库中获取。
		if (pstList[iCnt].uiEmvTag == 0x9F33)
		{
			EMVGetParameter(&glEmvParam);
			memcpy(sBuff, glEmvParam.Capability, 3);
			iLength = 3;
			BuildCLSSTLVString(pstList[iCnt].uiEmvTag, sBuff, iLength, &psTemp);
		}
		else
		{
			/*if(ChkIfBeaHalf())
			{
				if(glProcInfo.stTranLog.szPan[0] == '4') //BEA paywave do not need 9F08 9F09 0F34
				{
					if((pstList[iCnt].uiEmvTag == 0x9F08)||(pstList[iCnt].uiEmvTag == 0x9f09)||(pstList[iCnt].uiEmvTag == 0x9f34))
					{
						continue;
					}
				}
			}*/
			if ((ChkIfCiti()) || (ChkIfCitiTaxi()))
			{
				if (/*(pstList[iCnt].uiEmvTag == 0x9B)Gillian debug BEA ||*/(pstList[iCnt].uiEmvTag == 0x9F08)||(pstList[iCnt].uiEmvTag == 0x5F34))
				{
					continue;
				}
			}
			//BCM 厨獶钡ユ癳綝┶荡, 55办?ゅ郎⊿Τ璶―祇癳 sub fields, 
			//だ: 57, DFED, DFEE, DFEF,珿埃57//squall 2014.6.6 build156
			if (ChkIfDahOrBCM())//squall add in 2014.06.16 BCM don't need those tag below
			{
				if ((pstList[iCnt].uiEmvTag == 0x5A)||(pstList[iCnt].uiEmvTag == 0x9B)||(pstList[iCnt].uiEmvTag == 0x5F24)
					||(pstList[iCnt].uiEmvTag == 0x9F08)||(pstList[iCnt].uiEmvTag == 0x57))
				{
					continue;
				}
			}
			iRet = GetSpecTLVData(pstList[iCnt].uiEmvTag, sBuff, &iLength);
			
			if( (glProcInfo.stTranLog.szPan[0]=='4') && (pstList[iCnt].uiEmvTag==0x9F09||pstList[iCnt].uiEmvTag==0x9F34))
			{
		
				if (pstList[iCnt].uiEmvTag==0x9F09)
				{
					BuildCLSSTLVString(pstList[iCnt].uiEmvTag, "\x00\x00",2, &psTemp); //Gillian 2016-8-16
				}
				if (pstList[iCnt].uiEmvTag==0x9F34)
				{
					BuildCLSSTLVString(pstList[iCnt].uiEmvTag, "\x00\x00\x00",3, &psTemp); //Gillian 2016-8-16
				}
				
				/*	else  //Gillian 2016-8-16
					{
					//"\x02\x00\x00"?
						BuildCLSSTLVString(pstList[iCnt].uiEmvTag, "\x00\x00\x00",pstList[iCnt].ucLen, &psTemp);//BEA and bcm NOT ACCEPT 0 LENGTH
					}*/
				
				/*else{
				if (!ChkIfBeaHalf()&&!ChkIfDahOrBCM())
				{
					//PubDebugOutput("Tag",pstList[iCnt].uiEmvTag,2,DEVICE_SCR,HEX_MODE);//testtt
					BuildCLSSTLVString(pstList[iCnt].uiEmvTag, NULL, 0, &psTemp);//BEA and bcm NOT ACCEPT 0 LENGTH
				}*/
			}
			else if( /*(ChkIfBeaHalf()) && */ (glProcInfo.stTranLog.szPan[0]=='4') && (pstList[iCnt].uiEmvTag==0x9F08) )  //Gillian 2016-8-16
			{
				BuildCLSSTLVString(pstList[iCnt].uiEmvTag, "\x00\x00",2, &psTemp);//BEA and bcm NOT ACCEPT 0 LENGTH
			}
			else if( iRet==EMV_OK )//&& (iRet==EMV_OK &&((glProcInfo.stTranLog.szPan[0]=='4') && (!pstList[iCnt].uiEmvTag==0x9F08) && (!pstList[iCnt].uiEmvTag==0x9F09) && (!pstList[iCnt].uiEmvTag==0x9F34))))
			{
				BuildCLSSTLVString(pstList[iCnt].uiEmvTag, sBuff, iLength, &psTemp);
			}
		}
#if 0
			if( pstList[iCnt].ucOption==DE55_MUST_SET )
			{
				if( glProcInfo.stTranLog.szPan[0]=='4' )
				{
					if (pstList[iCnt].uiEmvTag==0x9F09||pstList[iCnt].uiEmvTag==0x9F34)
					{
						//if(ChkIfBeaHalf())  //Gillian 2016-8-16 
						//{
							if (pstList[iCnt].uiEmvTag==0x9F09)
							{
								BuildCLSSTLVString(pstList[iCnt].uiEmvTag, "\x02\x00\x00",pstList[iCnt].ucLen, &psTemp); //Gillian 2016-8-16
							}
							if (pstList[iCnt].uiEmvTag==0x9F34)
							{
								BuildCLSSTLVString(pstList[iCnt].uiEmvTag, "\x03\x00\x00",pstList[iCnt].ucLen, &psTemp); //Gillian 2016-8-16
							}
						//}
						/*else  //Gillian 2016-8-16
						{
						//"\x02\x00\x00"?
							BuildCLSSTLVString(pstList[iCnt].uiEmvTag, "\x00\x00\x00",pstList[iCnt].ucLen, &psTemp);//BEA and bcm NOT ACCEPT 0 LENGTH
						}*/
					}
					/*else{
					if (!ChkIfBeaHalf()&&!ChkIfDahOrBCM())
					{
						//PubDebugOutput("Tag",pstList[iCnt].uiEmvTag,2,DEVICE_SCR,HEX_MODE);//testtt
						BuildCLSSTLVString(pstList[iCnt].uiEmvTag, NULL, 0, &psTemp);//BEA and bcm NOT ACCEPT 0 LENGTH
					}

					}*/
				}
			}
			else if( /*(ChkIfBeaHalf()) &&*/ (glProcInfo.stTranLog.szPan[0]=='4') && (pstList[iCnt].uiEmvTag==0x9F08) )  //Gillian 2016-8-16
			{
				BuildCLSSTLVString(pstList[iCnt].uiEmvTag, "\x02\x00\x00",pstList[iCnt].ucLen, &psTemp);//BEA and bcm NOT ACCEPT 0 LENGTH
			}
			else if( iRet==EMV_OK)
			{
				BuildCLSSTLVString(pstList[iCnt].uiEmvTag, sBuff, iLength, &psTemp);
			}
		}
#endif
	}
#if 0 //Gillian 20160928
	if(!ChkIfMC(glProcInfo.stTranLog.szPan))  //Gillian 2016-8-9
	{
		if (ChkIfBeaHalf()||ChkIfDahOrBCM())//2015-6-25 Dah Sing need to add 9f6e in paywave trans
		{
			if( glProcInfo.stTranLog.szPan[0]=='4' )//paywave send 9f6e only
			{	
				iRet = GetSpecTLVData(0x9F6E,sBuff,&iLength);
		
				if( iRet==EMV_OK )
				{
					BuildCLSSTLVString(0x9F6E, sBuff, iLength, &psTemp);
				}
			}
		}//BUILD 132 squall 2013.4.22
	}
#endif
	//2015-10-14 add 9F7C
	if( glProcInfo.stTranLog.szPan[0]=='4' )
	{	
		iRet = GetSpecTLVData(0x9F7C,sBuff,&iLength);//len:33
		
		////2015-6-25 ttt
		//ScrCls();
		//ScrPrint(0,0,0, "[%d][%d]9f7c: %02x%02x%02x%02x",iRet,iLength, sBuff[0], sBuff[1], sBuff[2], sBuff[3], sBuff[4]);
		//getkey();

		if( iRet==EMV_OK )
		{
			BuildCLSSTLVString(0x9F7C, sBuff, iLength, &psTemp);
		}
	}
	//2015-10-14 add 9F6E
	//if( glProcInfo.stTranLog.szPan[0]=='4' )//paywave send 9f6e only
	if(!ChkIfMC(glProcInfo.stTranLog.szPan))  //Gillian 2016-8-9
	{
		if( glProcInfo.stTranLog.szPan[0]=='4' )//paywave send 9f6e only
		{
			iRet = GetSpecTLVData(0x9F6E,sBuff,&iLength);//len:4

			////2015-6-25 ttt
			//ScrCls();
			//ScrPrint(0,0,0, "[%d][%d]9f6e: %02x%02x%02x%02x",iRet,iLength, sBuff[0], sBuff[1], sBuff[2], sBuff[3], sBuff[4]);
			//getkey();

			if( iRet==EMV_OK )
			{
				BuildCLSSTLVString(0x9F6E, sBuff, iLength, &psTemp);
			}
		}
	}


	//	if( /*glProcInfo.stTranLog.szPan[0]=='5'*/(!ChkIfBeaHalf()) && ChkIfMC(glProcInfo.stTranLog.szPan) )//2016-2-15
	    if(ChkIfMC(glProcInfo.stTranLog.szPan) )
		{	// for master card TCC = "R" -- retail
			BuildCLSSTLVString(0x9F53, (uchar *)"R", 1, &psTemp);	
			if (bForUpLoad)
			{
				memset(sBuff, 0, sizeof(sBuff));
				iRet = GetSpecTLVData(0x91, sBuff, &iLength);
				if( iRet==EMV_OK )
				{
					BuildCLSSTLVString(0x91, sBuff, iLength, &psTemp);
				}
			}
		}
	
	//}

	//BCM 厨獶钡ユ癳綝┶荡, 5办?ゅ郎⊿Τ璶―祇癳 sub fields, 
	//だ: 57, DFED, DFEE, DFEF//squall 2014.6.6 build156
	
	if ((!ChkIfCiti()) && (!ChkIfCitiTaxi())&&(!ChkIfDahOrBCM()))
	{
		memcpy(psTemp, "\xDF\xED\x01", 3);
		psTemp += 3;
		*psTemp++ = 0x01;
		
		memcpy(psTemp, "\xDF\xEE\x01\x05", 4);
		psTemp += 4;
		
		memcpy(psTemp, "\xDF\xEF\x04", 3);
		psTemp += 3;
		memcpy(psTemp, GetClssTagDFEF(), 4);
		psTemp += 4;
	}


	*piOutLen = (psTemp-psOutData);

	return 0;
}
//2016-3-8 AE bug
int SetExtAMEXClSS55(DE55ClSSTag *pstList, uchar *psOutData, int *piOutLen)
{
    int     iRet, iCnt;
    ushort usLength;
    uchar   *psTemp, sBuff[200];

    *piOutLen = 0;
    memcpy(psOutData, "\xC1\xC7\xD5\xE2\x00\x01", 6);   // AMEX header
    psTemp = psOutData+6;

    for(iCnt=0; pstList[iCnt].uiEmvTag!=0; iCnt++)
    {
        usLength = 0;
        memset(sBuff, 0, sizeof(sBuff));
        iRet = GetSpecTLVData(pstList[iCnt].uiEmvTag, sBuff, &usLength);
        // iRet = Clss_GetAETLVData(pstList[iCnt].uiEmvTag, sBuff, &iLength);
        if( iRet!=EMV_OK&&iRet!=EMV_NO_DATA)
        {
            return ERR_TRAN_FAIL;
        }
        if (iRet == EMV_NO_DATA)//squall add here for empty TAG Value
        {
            usLength = pstList[iCnt].ucLen;
            memset(sBuff,0,usLength);
        }
        if( pstList[iCnt].ucOption==DE55_LEN_VAR1 )
        {
            *psTemp++ = (uchar)usLength;
        }
        else if( pstList[iCnt].ucOption==DE55_LEN_VAR2 )
        {
            *psTemp++ = (uchar)(usLength>>8);
            *psTemp++ = (uchar)usLength;
        }
        memcpy(psTemp, sBuff, usLength);
        psTemp += usLength;
    }
    *piOutLen = (psTemp-psOutData);

    return 0;
}
//获取指定TAG的VALUE
int GetSpecTLVData(ushort usTag, uchar *pDataOut, ushort *pnDataOutLen)
{	
	if (usTag == 0x57)
	{
		if (glProcInfo.szTrack2[0] != 0)
		{
			*pnDataOutLen = glProcInfo.ucTrack2Len;
			memcpy(pDataOut, glProcInfo.szTrack2, *pnDataOutLen);
			return 0;
		}
	}

	if( 0 == SearchSpecTLV(usTag, glProcInfo.stWaveTransData.sChipData, glProcInfo.stWaveTransData.nChipDataLen, pDataOut, pnDataOutLen))
	{
		return 0;
	}
	
	if( 0 == SearchSpecTLV(usTag, glProcInfo.stWaveTransData.sOtherData, glProcInfo.stWaveTransData.ucOtherDataLen, pDataOut, pnDataOutLen))
	{
		return 0;
	}

	return EMV_NO_DATA;
}

//在数据中搜索指定Tag的Value
uchar SearchSpecTLV(ushort nTag, uchar *sDataIn, ushort nDataInLen, uchar *sDataOut, ushort *pnDataOutLen)
{
    int i, j, iOneTag;
	ushort nLen;
	uchar *pDataEnd;

	pDataEnd = sDataIn + nDataInLen;
    while (sDataIn < pDataEnd) 
	{
        iOneTag = *sDataIn++;
        if (iOneTag == 0xFF || iOneTag == 0x00) continue;
        if ((iOneTag & 0x1F) == 0x1F) 
		{
            iOneTag <<= 8;
            iOneTag += *sDataIn++;
            if (iOneTag & 0x80) 
			{
                while (sDataIn < pDataEnd && (*sDataIn & 0x80)) sDataIn++;
                if (sDataIn >= pDataEnd) return 1;
                iOneTag = 0;
            }
        }
        if (*sDataIn & 0x80) 
		{
            i = (*sDataIn & 0x7F);
            if (sDataIn + i > pDataEnd) return 1;
            sDataIn++;
            for (j = 0, nLen = 0; j < i; j++) 
			{
                nLen <<= 8;
                nLen += *sDataIn++;
            }
        }
        else nLen = *sDataIn++;
		
        if (iOneTag == nTag) 
		{
            if (pnDataOutLen != NULL) *pnDataOutLen = nLen;
			memcpy(sDataOut, sDataIn, nLen);
            return 0;
        }
        if (iOneTag & 0xFF00) 
		{
            if (iOneTag & 0x2000) continue;
        }
        else if (iOneTag & 0x20) continue;

		sDataIn += nLen;
    }
    return 1;
}

// 只处理基本数据元素Tag,不包括结构/模板类的Tag
// Build Clss basic TLV data, exclude structure/template.
void BuildCLSSTLVString(ushort uiEmvTag, uchar *psData, int iLength, uchar **ppsOutData)
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
		memcpy(psTemp, psData, iLength);
		psTemp += iLength;
	}

	*ppsOutData = psTemp;
}

// 从2磁道信息分析出卡号(PAN)
int GetPanFromTrack2(uchar *pszPAN, uchar *pszExpDate)
{
	int		iPanLen;
	char	*p, pszTemp[41];

	// 从2磁道开始到'D'
	iPanLen = glProcInfo.ucTrack2Len;
	if( iPanLen>0 )
	{
		memset(pszTemp, 0, sizeof(pszTemp));
		PubBcd2Asc0(glProcInfo.szTrack2, iPanLen, pszTemp);
	}
	else
	{	// 2磁道都没有
		return ERR_SWIPECARD;
	}

	p = strchr((char *)pszTemp, 'D');
	if( p==NULL )
	{
		return ERR_SWIPECARD;
	}
	iPanLen = strlen(pszTemp) - strlen(p);
	if( iPanLen<13 || iPanLen>19 )
	{
		return ERR_SWIPECARD;
	}

	sprintf((char *)pszPAN, "%.*s", iPanLen, pszTemp);
	sprintf((char *)pszExpDate, "%.4s", p+1);

	return 0;
}

//----------------------------------------------------------------------------------
//                                 
//                                     参数相关函数
//
//-----------------------------------------------------------------------------------
//参数初始化	//第一次联接
int InitClssParam(void)
{
	int	iRet;

	ScrCls();
	PubShowTitle(TRUE, (uchar *)_T(" ClSS PARAM INIT"));
#ifdef _S60_ 
// 	iRet = ClssInit();
// 	if(iRet)
// 	{
// 		ProcError_Wave(iRet);
// 		return iRet;
// 	}
	if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
		|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
	{
		PortOpen(glSysParam.stEdcInfo.ucClssComm, "38400,8,n,1");
	}
#endif
	iRet = InitClssSchemeID();
	if (iRet)
	{
		return iRet;
	}

	iRet = InitClssEmvCAPK();
	if (iRet)
	{
		return iRet;
	}

	iRet = SaveReaderParam();
	if (iRet)
	{
		return iRet;
	}
//	iRet = SetInternalPara();
//	if (iRet)
//	{
//		return iRet;
//	}
	glSysParam.stEdcInfo.ucClssPARAFlag = 1;
	SaveSysParam();

/*	SetTLVData();*/
#ifdef _S60_ 
	if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
		|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
	{
		ClssClose(); //2014-5-16 enhance
	}
#endif
	return 0;
}

//SchemeID初始化
int InitClssSchemeID(void)
{
	int		iRet;
	uchar	ucNum ;
	uchar	ucIDListIn[21];
	Clss_SchemeID_Info stSchemeInfo[MAX_RD_SCHEME_NUM];
	
	ScrClrLine(2,7);
	PubDispString(_T("SCHEMEID INIT..."), 4|DISP_LINE_CENTER);

	ucNum = 0;
	memset(ucIDListIn, 0, sizeof(ucIDListIn));
	memset(stSchemeInfo, 0, MAX_RD_SCHEME_NUM*sizeof(Clss_SchemeID_Info));

	ucIDListIn[ucNum] = SCHEME_VISA_WAVE_2;
	stSchemeInfo[ucNum].ucSchemeID = SCHEME_VISA_WAVE_2;
	if (glSysParam.stEdcInfo.ucClssEnbFlag & CLSS_TYPE_VISA)
	{
		stSchemeInfo[ucNum].ucSupportFlg = 1;
	}
	else
	{
		stSchemeInfo[ucNum].ucSupportFlg = 0;
	}
	ucNum ++;

	ucIDListIn[ucNum] = SCHEME_VISA_WAVE_3;
	stSchemeInfo[ucNum].ucSchemeID = SCHEME_VISA_WAVE_3;
	if (glSysParam.stEdcInfo.ucClssEnbFlag & CLSS_TYPE_VISA)
	{
		stSchemeInfo[ucNum].ucSupportFlg = 1;
	}
	else
	{
		stSchemeInfo[ucNum].ucSupportFlg = 0;
	}
	ucNum ++;

	ucIDListIn[ucNum] = SCHEME_VISA_MSD_20;
	stSchemeInfo[ucNum].ucSchemeID = SCHEME_VISA_MSD_20;
	stSchemeInfo[ucNum].ucSupportFlg = 1;
	if (glSysParam.stEdcInfo.ucClssEnbFlag & CLSS_TYPE_VISA)
	{
		stSchemeInfo[ucNum].ucSupportFlg = 1;
	}
	else
	{
		stSchemeInfo[ucNum].ucSupportFlg = 0;
	}
	ucNum ++;

	ucIDListIn[ucNum] = SCHEME_JCB_WAVE_1;
	stSchemeInfo[ucNum].ucSchemeID = SCHEME_JCB_WAVE_1;
	//stSchemeInfo[ucNum].ucSupportFlg = 1;
	stSchemeInfo[ucNum].ucSupportFlg = 0; //Gillian 20161101
	ucNum ++;

	ucIDListIn[ucNum] = SCHEME_JCB_WAVE_2;
	stSchemeInfo[ucNum].ucSchemeID = SCHEME_JCB_WAVE_2;
	//stSchemeInfo[ucNum].ucSupportFlg = 1;
	stSchemeInfo[ucNum].ucSupportFlg = 0; //Gillian 20161101
	ucNum ++;

	ucIDListIn[ucNum] = SCHEME_JCB_WAVE_3;
	stSchemeInfo[ucNum].ucSchemeID = SCHEME_JCB_WAVE_3;
	//stSchemeInfo[ucNum].ucSupportFlg = 1;
	stSchemeInfo[ucNum].ucSupportFlg = 0; //Gillian 20161101
	ucNum ++;

	ucIDListIn[ucNum] = SCHEME_MC_MCHIP;
	stSchemeInfo[ucNum].ucSchemeID = SCHEME_MC_MCHIP;
	if (glSysParam.stEdcInfo.ucClssEnbFlag & CLSS_TYPE_MC)
	{
		stSchemeInfo[ucNum].ucSupportFlg = 1;
	}
	else
	{
		stSchemeInfo[ucNum].ucSupportFlg = 0;
	}
	ucNum ++;

// 	ucIDListIn[ucNum] = SCHEME_MC_MSTR; //这里已经不再支持设置这个scheme ID
// 	stSchemeInfo[ucNum].ucSchemeID = SCHEME_MC_MSTR;
// 	stSchemeInfo[ucNum].ucSupportFlg = 1;
// 	ucNum ++;
// 	
// 	ucIDListIn[ucNum] = SCHEME_PBOC_QPBOC;
// 	stSchemeInfo[ucNum].ucSchemeID = SCHEME_PBOC_QPBOC;
// 	stSchemeInfo[ucNum].ucSupportFlg = 1;
// 	ucNum ++;
// 	
// 	ucIDListIn[ucNum] = SCHEME_PBOC_MSD;
// 	stSchemeInfo[ucNum].ucSchemeID = SCHEME_PBOC_MSD;
// 	stSchemeInfo[ucNum].ucSupportFlg = 1;
// 	ucNum ++;

	iRet = WaveMngRdSchemeInfo('S', &ucNum,  ucIDListIn, stSchemeInfo);
	if (iRet)
	{
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("   SCHEME ID    "));
		return iRet;
	}
	return 0;
}

//公钥初始化
//CLSS与EMV的公钥/参数相同
int InitClssEmvCAPK(void)
{
	int writeKeyCnt;//2015-8-26 ttt

	int				iFlag, iCnt, iRet;
	EMV_CAPK		stEmvCapk;
	EMV_APPLIST		stEmvApp;

	ScrClrLine(2,7);
	PubDispString(_T("CAPK INIT..."), 4|DISP_LINE_CENTER);

	//删掉所有公钥
	iCnt = -1;
	iRet = WaveMngCAPKey('D', NULL, iCnt, NULL);
	if (iRet)
	{
		return iRet;
	}

	writeKeyCnt = 0;//2015-8-26

	//加载公钥
	iFlag = 0;
	for(iCnt=0; iCnt<MAX_KEY_NUM; iCnt++)
	{
		memset(&stEmvCapk, 0, sizeof(EMV_CAPK));
		iRet = EMVGetCAPK(iCnt, &stEmvCapk);
		if( iRet==EMV_OK )
		{
			if (memcmp(stEmvCapk.RID, RID_TYPE_VISA, 5) == 0 ||
				memcmp(stEmvCapk.RID, RID_TYPE_JCB,  5) == 0 ||
				memcmp(stEmvCapk.RID, RID_TYPE_MC,   5) == 0 ||
				memcmp(stEmvCapk.RID, RID_TYPE_AE,   5) == 0 )//2016-2-5 AMEX
//				memcmp(stEmvCapk.RID, RID_TYPE_PBOC, 5) == 0 )
			{
				iFlag = 1;
				iRet = WaveMngCAPKey('S',stEmvCapk.RID, iCnt, &stEmvCapk);
				if (iRet)
				{
					ScrCls();
					ScrPrint(0,0,0,"Rid=%02x%02x%02x%02x%02x", stEmvCapk.RID[0],stEmvCapk.RID[1],stEmvCapk.RID[2],stEmvCapk.RID[3],stEmvCapk.RID[4]);
					ScrPrint(0,1,0,"CA PKEY Add: %02X", iCnt);
					//return iRet;//2013-10-11 
				}
				else
				{
					writeKeyCnt++;  //2015-10-14
				}

			}
		}
	}

    ScrPrint(0,7,ASCII, "Key Written = %d", writeKeyCnt);//2015-10-14
	PubWaitKey(3);

	//表示没有相应的CAPK
	if (iFlag == 0)
	{
		return RC_NO_PARAMETER;
	}

	ScrClrLine(2,7);
	PubDispString(_T("AID INIT..."), 4|DISP_LINE_CENTER);

	//加载AID 参数
	iFlag = 0;
	for(iCnt=0; iCnt<MAX_APP_NUM; iCnt++)
	{
		memset(&stEmvApp, 0, sizeof(EMV_APPLIST));
		iRet = EMVGetApp(iCnt, &stEmvApp);
		if( iRet==EMV_OK )
		{
			//VISA
			if (memcmp(stEmvApp.AID, RID_TYPE_VISA, 5) == 0)
			{
				iFlag = 1;
				iRet = SaveVisaOrJcbPara(PARAM_TYPE_VISA, &stEmvApp);
				if (iRet)
				{
					return iRet;
				}
			}
			
			//JCB
			if (memcmp(stEmvApp.AID, RID_TYPE_JCB, 5) == 0)
			{
				iFlag = 1;
				iRet = SaveVisaOrJcbPara(PARAM_TYPE_JCB, &stEmvApp );
				if (iRet)
				{
					return iRet;
				}
			}

			//MC
			if (memcmp(stEmvApp.AID, RID_TYPE_MC, 5) == 0)
			{
				iFlag = 1;
				iRet = SaveMcPara(PARAM_TYPE_MC, &stEmvApp);
				if (iRet)
				{
					return iRet;
				}
			}

			//2016-2-5 AMEX
			//AE    
			// Added by Gillian  2016/1/21
            if (memcmp(stEmvApp.AID, RID_TYPE_AE, 5) == 0)
            {
                iFlag = 1;
                iRet = SaveAEPara(PARAM_TYPE_AE, &stEmvApp);	
                if (iRet)
                {
                    return iRet;
                }
            }
/*
			//QPBOC
			if (memcmp(stEmvApp.AID, RID_TYPE_PBOC, 5) == 0)
			{
				iFlag = 1;
				iRet = SavePbocPara(PARAM_TYPE_PBOC, &stEmvApp);
				if (iRet)
				{
					return iRet;
				}
			}
*/
		}
	}

	//表示没有相应的AID
	if (iFlag == 0)
	{
		return RC_NO_PARAMETER;
	}

	return 0;
}

int SaveVisaOrJcbPara(uchar ucParamType, EMV_APPLIST *stEmvApp)
{
	int		iRet;
	ushort	usParamLen;
	uchar	ucTmpBuf[1000];	
	Clss_VisaAidParam stAidParam;
	Clss_PreProcInfo  stPreProcInfo;
	
	//取得AID数值
	memset(&stAidParam, 0, sizeof(Clss_VisaAidParam));
	stAidParam.ulTermFLmt = glSysParam.stEdcInfo.ulClssOffFLmt;	
//	stAidParam.ulTermFLmt = 20000;
	stAidParam.ucDomesticOnly = 0x00;
	stAidParam.ucCvmReqNum =  0;	//读卡器R50&R30不支持签名,也不支持PIN输入
    memset(stAidParam.aucCvmReq, 0, sizeof(stAidParam.aucCvmReq));
	stAidParam.ucEnDDAVerNo = 0;

	//打成规定的格式
	usParamLen = 0;
	memset(ucTmpBuf, 0, sizeof(ucTmpBuf));
	Long2Str(stAidParam.ulTermFLmt, ucTmpBuf+usParamLen);
	usParamLen += 4;
	ucTmpBuf[usParamLen++] = stAidParam.ucDomesticOnly;
	ucTmpBuf[usParamLen++] = stAidParam.ucCvmReqNum;
	memcpy(ucTmpBuf+usParamLen, stAidParam.aucCvmReq, stAidParam.ucCvmReqNum);
	usParamLen += stAidParam.ucCvmReqNum;
	ucTmpBuf[usParamLen++] = stAidParam.ucEnDDAVerNo;

	iRet = WaveMngAppParam('S', ucParamType, ucTmpBuf, &usParamLen);
	if (iRet)
	{
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("SaveVisa&JcbPara"));
		return iRet;
	}

	//取得相对应的预处理结构数据
	memset(&stPreProcInfo, 0, sizeof(Clss_PreProcInfo));
	stPreProcInfo.ulTermFLmt = glSysParam.stEdcInfo.ulClssOffFLmt;
	memcpy(stPreProcInfo.aucAID, stEmvApp->AID, stEmvApp->AidLen);
	stPreProcInfo.ucAidLen = stEmvApp->AidLen;
	stPreProcInfo.ucKernType = ucParamType;

	stPreProcInfo.ulRdClssTxnLmt = glSysParam.stEdcInfo.ulClssMaxLmt;		//CHANGING POINT		//需参数	//EMV中没有对应参数,先默认
	stPreProcInfo.ulRdCVMLmt = /*glSysParam.stEdcInfo.ulClssMaxLmt;*/glSysParam.stEdcInfo.ulClssSigFlmt;	//Gillian 2016-8-15		//需参数	//EMV中没有对应参数,先默认
//	stPreProcInfo.ulRdClssFLmt = 20000;
	stPreProcInfo.ulRdClssFLmt = glSysParam.stEdcInfo.ulClssOffFLmt;	//需参数	//EMV中没有对应参数,先默认 Mandy manual set the Offline floor limit of visa
	
	stPreProcInfo.ucTermFLmtFlg = 1;
	stPreProcInfo.ucRdClssTxnLmtFlg = 1; 
	stPreProcInfo.ucRdCVMLmtFlg = 1;   
	stPreProcInfo.ucRdClssFLmtFlg = 1; 	 
	stPreProcInfo.ucCrypto17Flg = 1;			//需参数	//EMV中没有对应参数,先默认
	stPreProcInfo.ucZeroAmtNoAllowed = 0;
	stPreProcInfo.ucStatusCheckFlg = 0;
    memcpy(stPreProcInfo.aucReaderTTQ, "\xA3\x00\x00\x00", 4);	//需参数	//EMV中没有对应参数,先默认

	//打成规定的格式
	usParamLen = 0;
	memset(ucTmpBuf,0,sizeof(ucTmpBuf));
	Long2Str(stPreProcInfo.ulTermFLmt, ucTmpBuf+usParamLen);
	usParamLen += 4;
	Long2Str(stPreProcInfo.ulRdClssTxnLmt, ucTmpBuf+usParamLen);
	usParamLen += 4;
	/*=======BEGIN: Jason BUILD096 2015.03.17  10:1 modify===========*/ //2015-10-14
    Long2Str(glSysParam.stEdcInfo.ulODCVClssFLmt, ucTmpBuf + usParamLen);
    usParamLen += 4;
    /*====================== END======================== */
	Long2Str(stPreProcInfo.ulRdCVMLmt, ucTmpBuf+usParamLen);
	usParamLen += 4;
	Long2Str(stPreProcInfo.ulRdClssFLmt, ucTmpBuf+usParamLen);
	usParamLen += 4;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucAidLen;
	memcpy(ucTmpBuf+usParamLen, stPreProcInfo.aucAID, stPreProcInfo.ucAidLen);
	usParamLen += stPreProcInfo.ucAidLen;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucKernType;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucCrypto17Flg;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucZeroAmtNoAllowed;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucStatusCheckFlg;
	memcpy(ucTmpBuf+usParamLen,stPreProcInfo.aucReaderTTQ, 4);
	usParamLen += 4;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucTermFLmtFlg;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucRdClssTxnLmtFlg;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucRdCVMLmtFlg;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucRdClssFLmtFlg;
	ucTmpBuf[usParamLen++] = stEmvApp->SelFlag;

	iRet = WaveMngAppParam('S', PARAM_TYPE_PREPROC, ucTmpBuf, &usParamLen);
	if (iRet)
	{
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("SaveVisa&JcbPara"));
		return iRet;
	}

// test
// 	usParamLen = stPreProcInfo.ucAidLen;
// 	memset(ucTmpBuf,0,sizeof(ucTmpBuf));
// 	memcpy(ucTmpBuf, stPreProcInfo.aucAID, stPreProcInfo.ucAidLen);
// 	iRet = WaveMngAppParam('G', PARAM_TYPE_PREPROC, ucTmpBuf, &usParamLen);
// 	if (iRet)
// 	{
// 		ScrCls();
// 		PubShowTitle(TRUE, (uchar *)_T("SaveVisa&JcbPara"));
// 		return iRet;
// 	}
//end

	return 0;
}

int SaveMcPara(uchar ucParamType, EMV_APPLIST *stEmvApp)
{
	int	iRet;
	ushort	usParamLen, usTmplen;
	uchar	ucTmpBuf[1000];	
	Clss_MCAidParam   stMCAidParam;
	Clss_PreProcInfo  stPreProcInfo;	
 	
	//取得AID数值
	memset(&stMCAidParam, 0, sizeof(Clss_MCAidParam));
	stMCAidParam.FloorLimit = glSysParam.stEdcInfo.ulClssOffFLmt;	
	stMCAidParam.Threshold = stEmvApp->Threshold;

	stMCAidParam.usUDOLLen = 3;						//需参数	//EMV中没有对应参数,先默认
	memcpy(stMCAidParam.uDOL,"\x9F\x6A\x04",3);		//需参数	//EMV中没有对应参数,先默认

	stMCAidParam.TargetPer = stEmvApp->TargetPer;         
	stMCAidParam.MaxTargetPer = stEmvApp->MaxTargetPer;  
 	stMCAidParam.FloorLimitCheck = stEmvApp->FloorLimitCheck; 
	stMCAidParam.RandTransSel = stEmvApp->RandTransSel; 
	stMCAidParam.VelocityCheck = stEmvApp->VelocityCheck;
 	memcpy(stMCAidParam.TACDenial, stEmvApp->TACDenial, 5);      
	memcpy(stMCAidParam.TACOnline, stEmvApp->TACOnline, 5);     
	memcpy(stMCAidParam.TACDefault, stEmvApp->TACDefault, 5);    
    memcpy(stMCAidParam.AcquierId, stEmvApp->AcquierId, 6); 
	memcpy(stMCAidParam.dDOL, stEmvApp->dDOL, strlen(stEmvApp->dDOL));        
	memcpy(stMCAidParam.tDOL, stEmvApp->tDOL, strlen(stEmvApp->tDOL));        
	memcpy(stMCAidParam.Version, stEmvApp->Version, 2);

	stMCAidParam.ForceOnline = 0;					//需参数	//EMV中没有对应参数,先默认
	memcpy(stMCAidParam.MagAvn, "\x00\x00", 2);		//需参数	//EMV中没有对应参数,先默认
	stMCAidParam.ucMagSupportFlg = 1;				//需参数	//EMV中没有对应参数,先默认

	//打成规定的格式
	usTmplen = 0;
	usParamLen = 0;
	memset(ucTmpBuf, 0, sizeof(ucTmpBuf));
	Long2Str(stMCAidParam.FloorLimit, ucTmpBuf+usParamLen);
	usParamLen += 4;			
	Long2Str(stMCAidParam.Threshold, ucTmpBuf+usParamLen);
	usParamLen += 4;			
	ucTmpBuf[usParamLen++] = stMCAidParam.usUDOLLen >> 8;
	ucTmpBuf[usParamLen++] = stMCAidParam.usUDOLLen & 0xFF;
	memcpy(ucTmpBuf+usParamLen, stMCAidParam.uDOL, stMCAidParam.usUDOLLen);
	usParamLen += stMCAidParam.usUDOLLen;
	ucTmpBuf[usParamLen++] = stMCAidParam.TargetPer;
	ucTmpBuf[usParamLen++] = stMCAidParam.MaxTargetPer;
	ucTmpBuf[usParamLen++] = stMCAidParam.FloorLimitCheck;
	ucTmpBuf[usParamLen++] = stMCAidParam.RandTransSel;
	ucTmpBuf[usParamLen++] = stMCAidParam.VelocityCheck; 
	memcpy(ucTmpBuf+usParamLen, stMCAidParam.TACDenial, 5);
	usParamLen += 5;
	memcpy(ucTmpBuf+usParamLen, stMCAidParam.TACOnline, 5);
	usParamLen += 5;
	memcpy(ucTmpBuf+usParamLen, stMCAidParam.TACDefault, 5);
	usParamLen += 5;
	memcpy(ucTmpBuf+usParamLen, stMCAidParam.AcquierId, 6);
	usParamLen += 6;
	usTmplen = strlen(stMCAidParam.dDOL);
	ucTmpBuf[usParamLen++] = (uchar)usTmplen;
	memcpy(ucTmpBuf+usParamLen, stMCAidParam.dDOL, usTmplen);
	usParamLen += usTmplen;
	usTmplen = strlen(stMCAidParam.tDOL);
	ucTmpBuf[usParamLen++] = (uchar)usTmplen;
	memcpy(ucTmpBuf+usParamLen, stMCAidParam.tDOL, usTmplen);
	usParamLen += usTmplen;
	memcpy(ucTmpBuf+usParamLen, stMCAidParam.Version, 2);
	usParamLen += 2;
	ucTmpBuf[usParamLen++] = stMCAidParam.ForceOnline;
	memcpy(ucTmpBuf+usParamLen, stMCAidParam.MagAvn, 2);
	usParamLen += 2;
	ucTmpBuf[usParamLen++] = stMCAidParam.ucMagSupportFlg;

	iRet = WaveMngAppParam('S', ucParamType, ucTmpBuf, &usParamLen);
	if (iRet)
	{
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("   SaveMCPara   "));
		return iRet;
	}

	//取得相对应的预处理结构数据
	memset(&stPreProcInfo, 0, sizeof(Clss_PreProcInfo));
	stPreProcInfo.ulTermFLmt = glSysParam.stEdcInfo.ulClssOffFLmt;
	memcpy(stPreProcInfo.aucAID, stEmvApp->AID, stEmvApp->AidLen);
	stPreProcInfo.ucAidLen = stEmvApp->AidLen;
	stPreProcInfo.ucKernType = ucParamType;

	stPreProcInfo.ulRdClssTxnLmt = glSysParam.stEdcInfo.ulClssMaxLmt;		//需参数	//EMV中没有对应参数,先默认
	//stPreProcInfo.ulRdCVMLmt = glSysParam.stEdcInfo.ulClssMaxLmt;			//需参数	//EMV中没有对应参数,先默认
	stPreProcInfo.ulRdCVMLmt = glSysParam.stEdcInfo.ulClssSigFlmt;	    //Gillian 2016-08-09
	stPreProcInfo.ulRdClssFLmt = glSysParam.stEdcInfo.ulClssOffFLmt;	//需参数	//EMV中没有对应参数,先默认

	stPreProcInfo.ucTermFLmtFlg = 1;
	stPreProcInfo.ucRdClssTxnLmtFlg = 1; 
	stPreProcInfo.ucRdCVMLmtFlg = 1;   
	stPreProcInfo.ucRdClssFLmtFlg = 1; 	 
	stPreProcInfo.ucCrypto17Flg = 1;			//需参数	//EMV中没有对应参数,先默认
	stPreProcInfo.ucZeroAmtNoAllowed = 0;
	stPreProcInfo.ucStatusCheckFlg = 0;
    memcpy(stPreProcInfo.aucReaderTTQ, "\xA0\x00\x00\x00", 4);	//需参数	//EMV中没有对应参数,先默认

	//打成规定的格式
	usParamLen = 0;
	memset(ucTmpBuf,0,sizeof(ucTmpBuf));
	Long2Str(stPreProcInfo.ulTermFLmt, ucTmpBuf+usParamLen);
	usParamLen += 4;
	Long2Str(stPreProcInfo.ulRdClssTxnLmt, ucTmpBuf+usParamLen);
	usParamLen += 4;
	/*=======BEGIN: Jason BUILD096 2015.03.17  10:1 modify===========*/ //2015-10-14 硂ㄇ肂琌璶癸莱R50┮璶浪琩抖
    Long2Str(glSysParam.stEdcInfo.ulODCVClssFLmt, ucTmpBuf + usParamLen);
    usParamLen += 4;
    /*====================== END======================== */
	Long2Str(stPreProcInfo.ulRdCVMLmt, ucTmpBuf+usParamLen);
	usParamLen += 4;
	Long2Str(stPreProcInfo.ulRdClssFLmt, ucTmpBuf+usParamLen);
	usParamLen += 4;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucAidLen;
	memcpy(ucTmpBuf+usParamLen, stPreProcInfo.aucAID, stPreProcInfo.ucAidLen);
	usParamLen += stPreProcInfo.ucAidLen;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucKernType;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucCrypto17Flg;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucZeroAmtNoAllowed;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucStatusCheckFlg;
	memcpy(ucTmpBuf+usParamLen,stPreProcInfo.aucReaderTTQ, 4);
	usParamLen += 4;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucTermFLmtFlg;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucRdClssTxnLmtFlg;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucRdCVMLmtFlg;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucRdClssFLmtFlg;
	ucTmpBuf[usParamLen++] = stEmvApp->SelFlag;

	iRet = WaveMngAppParam('S', PARAM_TYPE_PREPROC, ucTmpBuf, &usParamLen);
	if (iRet)
	{
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("   SaveMCPara   "));
		return iRet;
	}

	return 0;
}

int SavePbocPara(uchar ucParamType, EMV_APPLIST *stEmvApp)
{
	int	iRet;
	ushort	usParamLen;
	uchar	ucTmpBuf[1000];
	Clss_PbocAidParam stPbocAidParam;
	Clss_PreProcInfo  stPreProcInfo;	
 	
	//取得AID数值
	memset(&stPbocAidParam,	   0, sizeof(Clss_PbocAidParam));
	stPbocAidParam.ulTermFLmt = glSysParam.stEdcInfo.ulClssOffFLmt;
	
	//打成规定的格式
	usParamLen = 0;
	memset(ucTmpBuf, 0, sizeof(ucTmpBuf));
	Long2Str(stPbocAidParam.ulTermFLmt, ucTmpBuf);
	usParamLen = 4;

	iRet = WaveMngAppParam('S', ucParamType, ucTmpBuf, &usParamLen);
	if (iRet)
	{
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("   SavePbocPara  "));
		return iRet;
	}

	//取得相对应的预处理结构数据
	memset(&stPreProcInfo, 0, sizeof(Clss_PreProcInfo));
	stPreProcInfo.ulTermFLmt = glSysParam.stEdcInfo.ulClssOffFLmt;
	memcpy(stPreProcInfo.aucAID, stEmvApp->AID, stEmvApp->AidLen);
	stPreProcInfo.ucAidLen = stEmvApp->AidLen;
	stPreProcInfo.ucKernType = ucParamType;

	stPreProcInfo.ulRdClssTxnLmt = glSysParam.stEdcInfo.ulClssMaxLmt;		//需参数	//EMV中没有对应参数,先默认
	stPreProcInfo.ulRdCVMLmt = glSysParam.stEdcInfo.ulClssSigFlmt;		//Gillian 20160809	//需参数	//EMV中没有对应参数,先默认
	stPreProcInfo.ulRdClssFLmt = glSysParam.stEdcInfo.ulClssOffFLmt;	//需参数	//EMV中没有对应参数,先默认

	stPreProcInfo.ucTermFLmtFlg = 1;
	stPreProcInfo.ucRdClssTxnLmtFlg = 1; 
	stPreProcInfo.ucRdCVMLmtFlg = 1;   
	stPreProcInfo.ucRdClssFLmtFlg = 1; 	 
	stPreProcInfo.ucCrypto17Flg = 1;			//需参数	//EMV中没有对应参数,先默认
	stPreProcInfo.ucZeroAmtNoAllowed = 0;
	stPreProcInfo.ucStatusCheckFlg = 0;
    memcpy(stPreProcInfo.aucReaderTTQ, glEmvParam.ExCapability, 5);	//需参数	//EMV中没有对应参数,先默认

	//打成规定的格式
	usParamLen = 0;
	memset(ucTmpBuf,0,sizeof(ucTmpBuf));
	Long2Str(stPreProcInfo.ulTermFLmt, ucTmpBuf+usParamLen);
	usParamLen += 4;
	Long2Str(stPreProcInfo.ulRdClssTxnLmt, ucTmpBuf+usParamLen);
	usParamLen += 4;
	/*=======BEGIN: Jason BUILD096 2015.03.17  10:1 modify===========*/ //2015-10-14 硂ㄇ肂琌璶癸莱R50┮璶浪琩抖
    Long2Str(glSysParam.stEdcInfo.ulODCVClssFLmt, ucTmpBuf + usParamLen);
    usParamLen += 4;
    /*====================== END======================== */
	Long2Str(stPreProcInfo.ulRdCVMLmt, ucTmpBuf+usParamLen);
	usParamLen += 4;
	Long2Str(stPreProcInfo.ulRdClssFLmt, ucTmpBuf+usParamLen);
	usParamLen += 4;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucAidLen;
	memcpy(ucTmpBuf+usParamLen, stPreProcInfo.aucAID, stPreProcInfo.ucAidLen);
	usParamLen += stPreProcInfo.ucAidLen;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucKernType;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucCrypto17Flg;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucZeroAmtNoAllowed;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucStatusCheckFlg;
	memcpy(ucTmpBuf+usParamLen,stPreProcInfo.aucReaderTTQ, 4);
	usParamLen += 4;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucTermFLmtFlg;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucRdClssTxnLmtFlg;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucRdCVMLmtFlg;
	ucTmpBuf[usParamLen++] = stPreProcInfo.ucRdClssFLmtFlg;
	ucTmpBuf[usParamLen++] = stEmvApp->SelFlag;

	iRet = WaveMngAppParam('S', PARAM_TYPE_PREPROC, ucTmpBuf, &usParamLen);
	if (iRet)
	{
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("   SavePbocPara  "));
		return iRet;
	}

	return 0;
}
//2016-2-5 AMEX
// Added by Gillian  2016/1/21
int SaveAEPara(uchar ucParamType, EMV_APPLIST *stEmvApp)
{
    int iRet;
    ushort  usParamLen, usTmplen;
    uchar   ucTmpBuf[1000];
    Clss_PreProcInfo  stPreProcInfo;


    //取得相对应的预处理结构数据
    memset(&stPreProcInfo, 0, sizeof(Clss_PreProcInfo));
    stPreProcInfo.ulTermFLmt = glSysParam.stEdcInfo.ulClssOffFLmt;
    memcpy(stPreProcInfo.aucAID, stEmvApp->AID, stEmvApp->AidLen);
    stPreProcInfo.ucAidLen = stEmvApp->AidLen;
    stPreProcInfo.ucKernType = ucParamType;

    stPreProcInfo.ulRdClssTxnLmt = glSysParam.stEdcInfo.ulClssMaxLmt;       //需参数    //EMV中没有对应参数,先默认

    stPreProcInfo.ulRdCVMLmt = glSysParam.stEdcInfo.ulClssSigFlmt;           //需参数    //EMV中没有对应参数,先默认

    stPreProcInfo.ulRdClssFLmt = glSysParam.stEdcInfo.ulClssOffFLmt;    //需参数    //EMV中没有对应参数,先默认

    stPreProcInfo.ucTermFLmtFlg = 1;
    stPreProcInfo.ucRdClssTxnLmtFlg = 1;
    stPreProcInfo.ucRdCVMLmtFlg = 1;
    stPreProcInfo.ucRdClssFLmtFlg = 1;
    stPreProcInfo.ucCrypto17Flg = 1;            //需参数    //EMV中没有对应参数,先默认
    stPreProcInfo.ucZeroAmtNoAllowed = 0;
    stPreProcInfo.ucStatusCheckFlg = 0;
  memcpy(stPreProcInfo.aucReaderTTQ, "\x36\x00\x40\x00", 4);  //需参数    //EMV中没有对应参数,先默认
    //memcpy(stPreProcInfo.aucReaderTTQ, glSysParam.stEdcInfo.ucWaveCap, 4);//2016-2-5 AE ????

    //打成规定的格式
    usParamLen = 0;
    memset(ucTmpBuf,0,sizeof(ucTmpBuf));
    Long2Str(stPreProcInfo.ulTermFLmt, ucTmpBuf+usParamLen);
    usParamLen += 4;
    Long2Str(stPreProcInfo.ulRdClssTxnLmt, ucTmpBuf+usParamLen);
    usParamLen += 4;

    //on device cvm limit
    Long2Str(glSysParam.stEdcInfo.ulODCVClssFLmt, ucTmpBuf + usParamLen);
    usParamLen += 4;

    Long2Str(stPreProcInfo.ulRdCVMLmt, ucTmpBuf+usParamLen);
    usParamLen += 4;
    Long2Str(stPreProcInfo.ulRdClssFLmt, ucTmpBuf+usParamLen);
    usParamLen += 4;
    ucTmpBuf[usParamLen++] = stPreProcInfo.ucAidLen;
    memcpy(ucTmpBuf+usParamLen, stPreProcInfo.aucAID, stPreProcInfo.ucAidLen);
    usParamLen += stPreProcInfo.ucAidLen;
    ucTmpBuf[usParamLen++] = stPreProcInfo.ucKernType;
    ucTmpBuf[usParamLen++] = stPreProcInfo.ucCrypto17Flg;
    ucTmpBuf[usParamLen++] = stPreProcInfo.ucZeroAmtNoAllowed;
    ucTmpBuf[usParamLen++] = stPreProcInfo.ucStatusCheckFlg;
    memcpy(ucTmpBuf+usParamLen,stPreProcInfo.aucReaderTTQ, 4);
    usParamLen += 4;
    ucTmpBuf[usParamLen++] = stPreProcInfo.ucTermFLmtFlg;
    ucTmpBuf[usParamLen++] = stPreProcInfo.ucRdClssTxnLmtFlg;
    ucTmpBuf[usParamLen++] = stPreProcInfo.ucRdCVMLmtFlg;
    ucTmpBuf[usParamLen++] = stPreProcInfo.ucRdClssFLmtFlg;
    ucTmpBuf[usParamLen++] = stEmvApp->SelFlag;

    iRet = WaveMngAppParam('S', PARAM_TYPE_PREPROC, ucTmpBuf, &usParamLen);
    if (iRet)
    {
        ScrCls();
        PubShowTitle(TRUE, (uchar *)_T("   SaveAEPara   "));
        return iRet;
    }

    return 0;
}

//读卡器参数
int SaveReaderParam(void)
{
	int	iRet,i;
	uchar ucAmt[13], ucBuff[7], sBuff[20];
	ushort	usParamLen, ucTmpLen;
	uchar	ucTmpBuf[1000];
	Clss_ReaderParam  stReaderParam; 

	ScrClrLine(2,7);
	PubDispString(_T("READER PAPA INIT"), 4|DISP_LINE_CENTER);

	//取得数据
	EMVGetParameter(&glEmvParam);
	memset(&stReaderParam, 0, sizeof(Clss_ReaderParam));
	stReaderParam.usMchLocLen = strlen("PAX EMV LIBRARY");
	memcpy(stReaderParam.aucMchNameLoc, "PAX EMV LIBRARY", stReaderParam.usMchLocLen); 
	memcpy(stReaderParam.aucMerchantID, "123456789012345", 15);
	memcpy(stReaderParam.AcquierId,"\x00\x00\x00\x12\x34\x56",6);
	memcpy(stReaderParam.aucTmID, "12345678", 8);

	stReaderParam.ulReferCurrCon = glEmvParam.ReferCurrCon;
	memcpy(stReaderParam.aucMerchCatCode, glEmvParam.MerchCateCode, 2);
	stReaderParam.ucTmType = glEmvParam.TerminalType;
#ifdef PAYPASS_CAPABILITY
	memcpy(stReaderParam.aucTmCap, EMV_CLSS_CAPABILITY, 3);
#else
	memcpy(stReaderParam.aucTmCap, glEmvParam.Capability, 3);
#endif
	memcpy(stReaderParam.aucTmCapAd, glEmvParam.ExCapability, 5);
	memcpy(stReaderParam.aucTmCntrCode, glEmvParam.CountryCode, 2);
	memcpy(stReaderParam.aucTmTransCur, glEmvParam.TransCurrCode, 2);
	stReaderParam.ucTmTransCurExp = glEmvParam.TransCurrExp;
	memcpy(stReaderParam.aucTmRefCurCode, glEmvParam.ReferCurrCode, 2);
	stReaderParam.ucTmRefCurExp = glEmvParam.ReferCurrExp;	

	//2015-11-12 ttt
	memcpy( stReaderParam.auVisaDRLList, sgVisaDRLList, 10*(5+12*3));	
	
	for(i=0;i<10;i++)
	{
		sprintf((char*)sBuff,"%02x%02x%02x%02x%02x",stReaderParam.auVisaDRLList[i].szProgramID[0],stReaderParam.auVisaDRLList[i].szProgramID[1],
			stReaderParam.auVisaDRLList[i].szProgramID[2],stReaderParam.auVisaDRLList[i].szProgramID[3],stReaderParam.auVisaDRLList[i].szProgramID[4]);
		if( (glSysParam.stEdcInfo.ucClssAPID[0] !=0 && memcmp(glSysParam.stEdcInfo.ucClssAPID,sBuff,10)==0) ||
			memcmp(stReaderParam.auVisaDRLList[i].szProgramID,"\x40\x03\x44\x03\x44",5) == 0)//default HKID
		{
			memset(ucAmt,0,12);
			PubLong2Bcd(glSysParam.stEdcInfo.ulODCVClssFLmt,6,ucBuff);
			PubBcd2Asc0(ucBuff,6,ucAmt);
			memcpy(stReaderParam.auVisaDRLList[i].ulRdClssTxnLmt, ucAmt,12);//Reader Contactless Transaction Limit

			memset(ucAmt,0,12);
			PubLong2Bcd(glSysParam.stEdcInfo.ulClssMaxLmt,6,ucBuff);
			PubBcd2Asc0(ucBuff,6,ucAmt);
			memcpy(stReaderParam.auVisaDRLList[i].ulRdCVMLmt, ucAmt,12);//Reader CVM Required Limit

			memset(ucAmt,0,12);
			PubLong2Bcd(glSysParam.stEdcInfo.ulClssOffFLmt,6,ucBuff);
			PubBcd2Asc0(ucBuff,6,ucAmt);
			memcpy(stReaderParam.auVisaDRLList[i].ulRdClssLmt, ucAmt,12);//Reader Contactless Floor Limit			
		}
		else//foreign currency
		{
			memset(ucAmt,0,12);
			PubLong2Bcd(glSysParam.stEdcInfo.ulODCVClssFLmt,6,ucBuff);
			PubBcd2Asc0(ucBuff,6,ucAmt);
			memcpy(stReaderParam.auVisaDRLList[i].ulRdClssTxnLmt, ucAmt,12);//Reader Contactless Transaction Limit

			memset(ucAmt,0,12);
			PubLong2Bcd(glSysParam.stEdcInfo.ulForeignCVMLmt,6,ucBuff);
			PubBcd2Asc0(ucBuff,6,ucAmt);
			memcpy(stReaderParam.auVisaDRLList[i].ulRdCVMLmt, ucAmt,12);//Reader CVM Required Limit

			memset(ucAmt,0,12);
			PubLong2Bcd(glSysParam.stEdcInfo.ulForeignOFFLmt,6,ucBuff);
			PubBcd2Asc0(ucBuff,6,ucAmt);
			memcpy(stReaderParam.auVisaDRLList[i].ulRdClssLmt, ucAmt,12);//Reader Contactless Floor Limit	
		}
	}
	//打成规定的格式
	usParamLen = 1;
	memset(ucTmpBuf, 0, sizeof(ucTmpBuf));
	Long2Str(stReaderParam.ulReferCurrCon, ucTmpBuf + usParamLen);
	usParamLen += 4;
	ucTmpBuf[usParamLen++] = stReaderParam.usMchLocLen >> 8;
	ucTmpBuf[usParamLen++] = stReaderParam.usMchLocLen & 0xFF;
	memcpy(ucTmpBuf+usParamLen, stReaderParam.aucMchNameLoc, stReaderParam.usMchLocLen);
	usParamLen += stReaderParam.usMchLocLen;
	memcpy(ucTmpBuf+usParamLen, stReaderParam.aucMerchCatCode, 2);
	usParamLen += 2;
	memcpy(ucTmpBuf+usParamLen, stReaderParam.aucMerchantID, 15);
	usParamLen += 15;
	memcpy(ucTmpBuf+usParamLen, stReaderParam.AcquierId, 6);
	usParamLen += 6;
	memcpy(ucTmpBuf+usParamLen, stReaderParam.aucTmID, 8);
	usParamLen += 8;
	ucTmpBuf[usParamLen++] = stReaderParam.ucTmType;
	memcpy(ucTmpBuf+usParamLen, stReaderParam.aucTmCap, 3);
	usParamLen += 3;
	memcpy(ucTmpBuf+usParamLen, stReaderParam.aucTmCapAd, 5);
	usParamLen += 5;
	memcpy(ucTmpBuf+usParamLen, stReaderParam.aucTmCntrCode, 2);
	usParamLen += 2;
	memcpy(ucTmpBuf+usParamLen, stReaderParam.aucTmTransCur, 2);
	usParamLen += 2;
	ucTmpBuf[usParamLen++] = stReaderParam.ucTmTransCurExp;
	memcpy(ucTmpBuf+usParamLen, stReaderParam.aucTmRefCurCode, 2);
	usParamLen += 2;
	ucTmpBuf[usParamLen++] = stReaderParam.ucTmRefCurExp;
	//2015-11-12 =========================add APID=====================START
	for(i=0;i<10;i++)
	{
		memcpy(ucTmpBuf+usParamLen,stReaderParam.auVisaDRLList[i].szProgramID,5);
		usParamLen += 5;
		memcpy(ucTmpBuf+usParamLen,stReaderParam.auVisaDRLList[i].ulRdClssTxnLmt,12);
		usParamLen += 12;
		memcpy(ucTmpBuf+usParamLen,stReaderParam.auVisaDRLList[i].ulRdCVMLmt,12);
		usParamLen += 12;
		memcpy(ucTmpBuf+usParamLen,stReaderParam.auVisaDRLList[i].ulRdClssLmt,12);
		usParamLen += 12;
	}
	//2015-11-12 =========================add APID=====================END

	//VISA
	ucTmpBuf[0] = PARAM_TYPE_VISA;
	ucTmpLen = usParamLen;
	iRet = WaveMngAppParam('S', PARAM_TYPE_READER, ucTmpBuf, &usParamLen);
	if (iRet)
	{
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("VisaReaderParam"));
		return iRet;
	}

	//JCB
	ucTmpBuf[0] = KERNTYPE_JCB;
	usParamLen = ucTmpLen;
	iRet = WaveMngAppParam('S', PARAM_TYPE_READER, ucTmpBuf, &usParamLen);
	if (iRet)
	{
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("JcbReaderParam"));
		return iRet;
	}

	//MC
	ucTmpBuf[0] = KERNTYPE_MC;
	usParamLen = ucTmpLen;
	iRet = WaveMngAppParam('S', PARAM_TYPE_READER, ucTmpBuf, &usParamLen);
	if (iRet)
	{
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("McReaderParam"));
		return iRet;
	}
	//2016-2-5 AMEX
	//Add by Gillian
	//AE
  //  ucTmpBuf[0] = 5;
	ucTmpBuf[0] = KERNTYPE_AE;
    usParamLen = ucTmpLen;
    iRet = WaveMngAppParam('S', PARAM_TYPE_READER, ucTmpBuf, &usParamLen);
    if (iRet)
    {
        ScrCls();
        PubShowTitle(TRUE, (uchar *)_T("AEReaderParam"));
        return iRet;
    }
/*
	//PBOC
	ucTmpBuf[0] = KERNTYPE_PBOC;
	usParamLen = ucTmpLen;
	iRet = WaveMngAppParam('S', PARAM_TYPE_READER, ucTmpBuf, &usParamLen);
	if (iRet)
	{
		ScrCls();
		PubShowTitle(TRUE, (uchar *)_T("PbocReaderParam"));
		return iRet;	
	}
*/
	return 0;
}

void Long2Str(unsigned long ldat, unsigned char *str)
{
    str[0] = (unsigned char)(ldat >> 24);
    str[1] = (unsigned char)(ldat >> 16);
    str[2] = (unsigned char)(ldat >> 8);
    str[3] = (unsigned char)(ldat);
}

int SetInternalPara(void)
{
	int	iRet;
	uchar   ucRecvData[256] = {0};//2016-3-9
    ushort  usLenRecv;//2016-3-9

	memset(&sg_RdInternalPara, 0, sizeof(READER_INTERNAL_PARAM));

	memcpy(sg_RdInternalPara.aucMsgTmOut,  "\x01\xF4", 2);//500ms
	memcpy(sg_RdInternalPara.aucSaleTmOut, "\xEA\x60", 2);
	memcpy(sg_RdInternalPara.aucPollMsg,   "\x00\x1E", 2);
	memcpy(sg_RdInternalPara.aucBufTmOut,  "\x13\x88", 2);
	sg_RdInternalPara.ucEncryptFlg = 0;
	sg_RdInternalPara.ucDisplayFlg = 0;
	memcpy(sg_RdInternalPara.aucMaxBufSize, "\x04\x00", 2);
	memcpy(sg_RdInternalPara.aucDoubleDip,  "\x13\x88", 2);
	memcpy(sg_RdInternalPara.aucReaderId,   "\x0E\x01", 2);
			
	// 根据所选语言初始化R50语言
 	if (glSysParam.stEdcInfo.ucLanguage == 0)//english
 	{ 	
		memcpy(sg_RdInternalPara.aucLanguage, "\x06\x65\x6E\x01\x7A\x68\x00", 7); // 值域为6字节
 	}
 	else// chinese
 	{
		memcpy(sg_RdInternalPara.aucLanguage, "\x06\x65\x6E\x00\x7A\x68\x01", 7); // 值域为6字节
	}
	//2016-3-9 add to set R50 Language ======================start
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
	//2016-3-9 add to set R50 Language ======================end
	// 支持英语和中文，第一行显示英语，第二行显示中文
	//	memcpy(sg_RdInternalPara.aucLanguage, "\x06\x65\x6E\x01\x7A\x68\x01", 7); 
			
	memcpy(sg_RdInternalPara.aucDisp_S_Msg,  "\x07\xD0", 2);// 0x07 D0, 2000ms
	memcpy(sg_RdInternalPara.aucDisp_L_Msg,  "\x13\x88", 2);// 0x13 88, 5000ms Mandy changed to 500 ms
	memcpy(sg_RdInternalPara.aucDisp_SS_Msg, "\x27\x10", 2);// 0x27 10, 10000MS
	memcpy(sg_RdInternalPara.aucDisp_SR_Msg, "\x13\x88", 2);// 0x13 88, 5000MS
	memcpy(sg_RdInternalPara.aucDisp_PIN_Msg,"\x27\x10", 2);// 0x27 10, 10000ms
	memcpy(sg_RdInternalPara.aucDisp_E_Msg,  "\x0B\xB8", 2);// 0x0B B8, 3000ms
			
	iRet = WaveMngInternalPara('S', &sg_RdInternalPara);
	if (iRet)
	{
		ProcError_Wave(iRet);
		return iRet;
	}

	return 0;
}

int SetTLVData(void)
{
	int		iRet;
	uchar	ucTagNum, ucTagListIn[10]={0}, pTlvData[100]={0};
	ushort  usTagLenIn, pusTlvDataLen;
	
	memcpy(ucTagListIn, "\xDF\x00\xDF\x01\xDF\x02", 6);
	usTagLenIn = 6;
	ucTagNum = 3;
/*	memcpy(pTlvData,   "\xDF\x00\x06\x00\x00\x00\x30\x01\x00", 9);	
	memcpy(pTlvData+9, "\xDF\x02\x06\x00\x00\x00\x30\x00\x00", 9);	
	pusTlvDataLen = 18;
	
	iRet = WaveMngTLVData('S', ucTagListIn, usTagLenIn, &ucTagNum, pTlvData, &pusTlvDataLen);
	if (iRet)
	{
		PubShowTitle(TRUE, (uchar *)_T("Set TLVData"));
		ProcError_Wave(iRet);
		return iRet;
	}
*/
	//test
	pusTlvDataLen = 0;
	memset(pTlvData, 0, sizeof(pTlvData));
	iRet = WaveMngTLVData('G', ucTagListIn, usTagLenIn, &ucTagNum, pTlvData, &pusTlvDataLen);
	if (iRet)
	{
		ProcError_Wave(iRet);
		return iRet;
	}
	ScrClrLine(2,7);
	ScrPrint(0,0,0,"%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
				pTlvData[0],pTlvData[1],pTlvData[2],pTlvData[3],pTlvData[4],
				pTlvData[5],pTlvData[6],pTlvData[7],pTlvData[8],pTlvData[9],
				pTlvData[10],pTlvData[11],pTlvData[12],pTlvData[13],pTlvData[14],
				pTlvData[15],pTlvData[16],pTlvData[17],pTlvData[18],
				pTlvData[19],pTlvData[20],pTlvData[21],pTlvData[22],
				pTlvData[23],pTlvData[24],pTlvData[25],pTlvData[26]);
				getkey();
	//end

	return 0;
}
