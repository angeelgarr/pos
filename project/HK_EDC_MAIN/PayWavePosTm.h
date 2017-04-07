/*********************************************************************
* 功能：提供Pos端接口函数定义；

* 创建：luyj 2009－01－05  
*********************************************************************/

#ifndef _PAYWAVEPOS_H
#define _PAYWAVEPOS_H

#include <posapi.h>

#define CLSS_PARAM_ERR    -30  // EMV_PARAM_ERR

#define RC_FILE_ERROR           0xC2
#define INSERTED_ICCARD         0x50
#define SWIPED_MAGCARD          0x51

#define BPS_115200   0
#define BPS_57600    1 
#define BPS_38400    2
#define BPS_28800    3
#define BPS_19200    4
#define BPS_RFU      5 // ~10  


#define INPUT_ERROR  -5
#define CLSS_USER_CANCEL  -6


#define KERNTYPE_DEF 0
#define KERNTYPE_JCB 1	
#define KERNTYPE_MC  2
#define KERNTYPE_VIS 3
#define KERNTYPE_PBOC  4	
#define KERNTYPE_RFU 5

#define PARAM_TYPE_JCB     KERNTYPE_JCB // 0x01 // Clss_VisaAidParam  
#define PARAM_TYPE_MC      KERNTYPE_MC // 0x02 // Clss_MCAidParam    
#define PARAM_TYPE_VISA    KERNTYPE_VIS // 0x03 // Clss_VisaAidParam  
#define PARAM_TYPE_PBOC    KERNTYPE_PBOC // 0x04 // Clss_PbocAidParam  
#define PARAM_TYPE_PREPROC 0x05 // Clss_PreProcInfo   
#define PARAM_TYPE_READER  0x06 // Clss_ReaderParam 
#define PARAM_TYPE_RFU     0x07
#define PARAM_TYPE_AE    KERNTYPE_AE //2016-2-5 AMEX

#define RC_SUCCESS              0x00      // SUCCESS
#define RC_DATA                 0x01      // CARD DATA
#define RC_POLL_A               0x02      // POLL A
#define RC_POLL_P               0x03      // POLL P
#define RC_SCHEME_SUPPORTED     0x04      // SCHEME
#define RC_SIGNATURE            0x05      // SIGNATURE 
#define RC_ONLINE_PIN           0x06      // PIN_ONLINE 
#define RC_OFFLINE_PIN          0x07      // PIN_OFFLINE
#define RC_FAILURE              0xFF      // FAILURE
#define RC_ACCESS_NOT_PERFORMED 0xFE      // NO ACCESS
#define RC_ACCESS_FAILURE       0xFD      // FAIL ACCESS 
#define RC_AUTH_FAILURE         0xFC      // FAIL AUTH
#define RC_AUTH_NOT_PERFORMED   0xFB      // NO AUTH
#define RC_DDA_AUTH_FAILURE     0xFA      // CARD FAIL
#define RC_INVALID_COMMAND      0xF9      // NO MSG ID 
#define RC_INVALID_DATA         0xF8      // DATA INCORRECT
#define RC_INVALID_PARAM        0xF7      // NO PARA 
#define RC_INVALID_KEYINDEX     0xF6      // BAD KEYID
#define RC_INVALID_SCHEME       0xF5      // NO SCHEME
#define RC_INVALID_VISA_CA_KEY  0xF4      // BAD Visa CA KEYID
#define RC_MORE_CARDS           0xF3      // MORE THAN 1 CARD
#define RC_NO_CARD              0xF2      // NO CARD
#define RC_NO_EMV_TAGS          0xF1      // NO TAGS 
#define RC_NO_PARAMETER         0xF0      // NO PARA
#define RC_POLL_N               0xEF      // POLL N
#define RC_OTHER_AP_CARDS       0xEE      // Other AP Cards
#define RC_US_CARDS             0xED      // US Cards
#define RC_NO_PIN               0xEC      // No PIN
#define RC_NO_SIG               0xEB      // No Signature
#define RC_INVALID_JCB_CA_KEY   0xEA

#define RC_DECLINED             0x10      // offline declined 自定义


//KEY TYPE
#define WAVE_KEY_IMEK_MDK 0x00
#define WAVE_KEY_IMEK     0x01
#define WAVE_KEY_MEK      0x02
#define WAVE_KEY_MSESSION 0x03
#define WAVE_KEY_IAEK_MDK 0x04
#define WAVE_KEY_IAEK     0x05
#define WAVE_KEY_AEK      0x06
#define WAVE_KEY_ASESSION 0x07
#define WAVE_IMEK_RAND    0x08
#define WAVE_IAEK_RAND    0x09

#define WAVE_MSG_WELCOM 1
#define WAVE_MSG_THANK  2
#define WAVE_MSG_THANK_NAME 3
#define WAVE_MSG_TXN_CMP 4
#define WAVE_MSG_OTHER_CARD 5
#define WAVE_MSG_INS_CARD   6
#define WAVE_MSG_SLT_1_CARD 7
#define WAVE_MSG_INTL_INS   8
#define WAVE_MSG_TRY_AGAIN  9
#define WAVE_MSG_INTL_SWIPE 10
#define WAVE_MSG_SIGN_SCR   11
#define WAVE_MSG_SIGN_RCPT  12
#define WAVE_MSG_ENTER_PIN  13
#define WAVE_MSG_OFF_FUND   14
#define WAVE_MSG_PIN_REQ    15
#define WAVE_MSG_SIGN_REQ   16
#define WAVE_MSG_NOT_IN_USE 17
#define WAVE_MSG_PRESENT_CARD 18
#define WAVE_MSG_REMOVE_CARD  19
#define WAVE_MSG_PROCESSING   20
// self define [1/6/2010 yingl]
#define WAVE_MSG_ERROR        21
#define WAVE_MSG_DECLINE        22


#define SCHEME_ALL            0x09
#define SCHEME_VISA_MSI        0x10
#define SCHEME_VISA_MSI_FDDA   0x11
#define SCHEME_VISA_CDA        0x12
#define SCHEME_VISA_DCVV       0x13
#define SCHEME_VISA_FULL_VSDC  0x14  
#define SCHEME_VISA_FDDA_VLP   0x15  
#define SCHEME_VISA_WAVE_2     0x16  // SUPPORTED
#define SCHEME_VISA_WAVE_3     0x17  // SUPPORTED
#define SCHEME_VISA_MSD_20     0x18  // SUPPORTED

#define SCHEME_VISA_RFU        0x19 // ~0x1F

#define SCHEME_JCB_WAVE_1      0x60
#define SCHEME_JCB_WAVE_2      0x61
#define SCHEME_JCB_WAVE_3      0x62
#define SCHEME_JCB_RFU         0x63 // ~0x7F


#define SCHEME_MC_RFU          0x20 //~0x3F

#define SCHEME_MC_MCHIP		   0x21	// MasterCard PayPass MChip
#define SCHEME_MC_MSTR		   0x22	// MasterCard PayPass MChip

#define SCHEME_AE_RFU          0x40 // ~0x5F

#define SCHEME_RFU             0x80 // ~0xFF

#define SCHEME_PBOC_QPBOC	   0x81	// PBOC qPBOC
#define SCHEME_PBOC_MSD 	   0x82	// PBOC qPBOC

typedef struct  
{
	uchar ucSchemeID; // Scheme ID 
	uchar ucSupportFlg; // 是否支持该Scheme；0-不支持；1-支持
	uchar ucRFU[2];   // 预留字节
}Clss_SchemeID_Info;

typedef struct
{
	unsigned char aucMsgTmOut[2]; // 0x01 F4, 500ms
	unsigned char aucSaleTmOut[2]; // 0x3A 98, 15000ms
	unsigned char aucPollMsg[2]; // 0x00 1e, 30s
	unsigned char aucBufTmOut[2]; // default 0x13 88, 5000ms
	unsigned char ucEncryptFlg;
	unsigned char ucDisplayFlg;
	unsigned char aucMaxBufSize[2]; // defaule 0x04 00
	unsigned char aucDoubleDip[2]; // 0x13 88, 5000ms
	unsigned char aucReaderId[2]; // 
	
								  unsigned char aucLanguage[3*10+1]; /* 10 种语言,
																	 Length + Language definition + state (01: activated, 00: no activated).
																	 English = en = 65 6E, Chinese = zh = 7A 68.
																	 1. Chinese Message is displayed only. The data is as follows:
																	 5F 2D 06 65 6E 00 7A 68 01 (English deactivated, Chinese activated)
																	 2. Chinese Message displayed on the first line and English on the second line.
																	 The data is as follows:
								  5F 2D 06 7A 68 01 65 6E 01 (Chinese first and next to English) */
								  unsigned char aucDisp_S_Msg[2]; // 0x07 D0, 2000ms
								  unsigned char aucDisp_L_Msg[2]; // 0x13 88, 5000ms
								  unsigned char aucDisp_SS_Msg[2]; // 0x27 10, 10000MS
								  unsigned char aucDisp_SR_Msg[2]; // 0x13 88, 5000MS
								  unsigned char aucDisp_PIN_Msg[2]; // 0x27 10, 10000ms
								  unsigned char aucDisp_E_Msg[2]; // 0x0B B8, 3000ms
								  
								  uchar ucRFU[5]; // 20090511
}READER_INTERNAL_PARAM;

typedef struct
{
	uchar ucMsgID;  // 显示信息对应的ID号
	char  szMsg[99]; // 显示信息
}WAVEMSGINFO;
/*
typedef struct {
	unsigned char RID[5];            //应用注册服务商ID
	unsigned char KeyID;             //密钥索引
	unsigned char HashInd;           //HASH算法标志
	unsigned char ArithInd;          //RSA算法标志
	unsigned char ModulLen;          //模长度
	unsigned char Modul[248];        //模
	unsigned char ExponentLen;       //指数长度
	unsigned char Exponent[3];       //指数
	unsigned char ExpDate[3];        //有效期(YYMMDD)
	unsigned char CheckSum[20];      //密钥校验和
}EMV_CAPK;
*/
//应用开发的接口，包含了WaveL3InitTmKeys的处理//  [12/22/2009 yingl]
//*参数初始化以及相互认证API
int WaveInitPara(int nChannel);
int WavePOLL(void);
int WavePollAuthProc(void);

//*参数管理操作API
int WaveMngRdSchemeInfo(char cMode, uchar *pucNum, uchar *paucIDListIn, Clss_SchemeID_Info *pInfo);
int WaveMngInternalPara(char cMode, READER_INTERNAL_PARAM *pParam);
int WaveMngDateTime(char cMode, uchar *pucDateTime);
int WaveMngBaudRate(char cMode, uchar *pucBaudRateID);
int WaveMngTLVData(char cMode, uchar *pTagListIn, ushort usTagLenIn, uchar *ucTagNum, uchar *pTlvData, ushort *pusTlvDataLen);
int WaveMngDispMsg(char cMode, uchar *pucNum, uchar *paucIDListIn , WAVEMSGINFO *ptMsgInfo);
int WaveMngCAPKey(char cMode, uchar *pRid, int nKeyId, EMV_CAPK *pKey);
int WaveKeySetting(uchar ucKeyType, uchar ucKeyId, uchar *pKey);
//int WaveMngAidParam(char cMode, uchar *paucAid, uchar ucAidLen, uchar *pTlvData, ushort *pusTlvDataLen);
int WaveMngAppParam(char cMode, uchar ucParamType, uchar *pParamData, ushort *pusParamLen);
// 交易处理函数 [12/24/2009 yingl]
int WaveStartTrans(uchar *pucNumeric, uchar *pucOutData, ushort *pLenOut);
int WaveStartTransTList(uchar *pucNumeric, uchar *pTagListIn, uchar ucTagLen, uchar *pucOutData, ushort *pLenOut);
int WaveShowStatus(uchar ucStatus, uchar *pMsgIDList, ushort usMsgIDLen);
int WaveProcSelfDefCmd(uchar ucIns, uchar *pSendData, ushort usLenSend, uchar *pRecvData, ushort *pusLenRecv);
int WaveProcOtherProtocol(uchar *pSendData, ushort usLenSend, uchar *pRecvData, ushort *pusLenRecv);

// 回调函数 [12/24/2009 yingl]
int cPortOpen(int nChannel, uchar *ucPara);
int cPortClose(int nChannel);
int cWavePortReset(int nChannel);//复位通讯口，该函数将清除串口接收缓冲区中的所有数据
int cWavePortSend(int nChannel, uchar ucData);//使用指定的通信口发送一个字节的数据
int cWavePortSends(int nChannel, uchar *paucStr, ushort usStrLen);//使用指定的通信口发送指定长度的字符串
int cWavePortRecv(int nChannel, uchar *pucData, uint uiTimeOutMs);//使用指定的通信口发送指定长度的字符串
void cWaveTimerSet(int nTimerNo, int nMs);
int cWaveTimerCheck(int nTimerNo);
int  cFileOpen(char *filename, uchar mode);
int cFileRead(int fid, uchar *dat, int len);
int cFileWrite(int fid, uchar *dat, int len);
int cFileClose(int fid);
int cFileSeek(int fid, long offset, uchar fromwhere);
long  cFileGetSize(char *filename);
int cWaveDetStopTrans(void);
// 结束 [12/22/2009 yingl]



#endif

