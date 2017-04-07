
#include "global.h"
#include "SxxCom.h"
#include "sslapi.h"

/********************** Internal macros declaration ************************/
#define TCPMAXSENDLEN		10240

/********************** Internal structure declaration *********************/

/********************** Internal functions declaration *********************/
static int SxxTcpConnect(char *pszIP, short sPort, int iTimeout);
static uchar SocketCheck(int sk);

/********************** Internal variables declaration *********************/
static int sg_iSocket;

/********************** external reference declaration *********************/
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

static int sg_Timeout;
#define BYTE	unsigned char

typedef struct{
	//first part are transaction independent data. should be initialised after power on.
	BYTE    AcquireID[6];           //tag'9F01' Acquirer ID
	BYTE	TermCapab[3];			//tag'9F33' terminal capability
	BYTE	TermAddCapab[5];		//tag'9F40' terminal additional capability
	BYTE	IFD_SN[8];              //tag'9F1E' IFD(POS device) serial no. asc8
	BYTE    TermID[8];				//tag'9F1C' Terminal ID
	BYTE    MerchCateCode[2];       //tag'9F15' Merchant Category Code
	BYTE    MerchID[15];            //tag'9F16' Merchant ID
	BYTE	CountryCode[2];			//tag'9F1A' Terminal country code BCD
	BYTE	TRMDataLen;
	BYTE	TRMData[8];				//tag'9F1D' Terminal Risk Management Data
	BYTE	TermType;				//tag'9F35' Terminal type
	BYTE	AppVer[2];				//tag'9F09' Application Version Number in terminal//VIS1.3.1--0x0083;VIS1.3.2--0x0084;VIS140--0x008c;
	BYTE    TransCurcyCode[2];      //tag'5F2A'
	BYTE    TransCurcyExp;          //tag'5F36'
	BYTE    TransReferCurcyCode[2]; //tag'9F3C'
	BYTE    TransReferCurcyExp;     //tag'9F3D'
	BYTE	TACDenial[5];			//Terminal action code-denial
	BYTE	TACOnline[5];			//Terminal action code-online
	BYTE	TACDefault[5];			//Terminal action code-default
	BYTE    TransType;              //for distinguish different trans types such as goods and service.	//0x01,0x00,0x00,0x09
	BYTE    TransTypeValue;         //tag'9C',transtype value(first two digits of processing code) as stated in EMV2000. goods and service are both 0x00.
	BYTE	VLPTransLimit[6];		//tag'9F7B',n12, new added in VIS1.4.0.
	BYTE	VLPTACDenial[5];		//Terminal action code-denial for VLP
	BYTE	VLPTACOnline[5];		//Terminal action code-online for VLP
	BYTE	VLPTACDefault[5];		//Terminal action code-default for VLP
	BYTE	Language;				//CHINESE or ENGLISH for display and print language.
	BYTE	bTermDDOL;              //0-no default DDOl in terminal;1- has default DDOL in terminal  
	BYTE	bForceAccept;			//this two set according test script V2CM080.00,V2CM081.00
	BYTE	bForceOnline;			//also see emvterm.pdf p32
	BYTE	bBatchCapture;          //private set for send different msg to host-AuthRQ and FinaRQ.
	BYTE    bTermSupportVLP;        //0-not support;1-support. configurable terminal parameter to indicate if VLP is supported.
	BYTE	MaxTargetPercent;
	BYTE	TargetPercent;          //Preset by terminal. range 0-99, and MTP>=TP
	BYTE	TermDDOLLen;
	BYTE	TermDDOL[128];          //term hold of default DDOL,must be initialised in init.
	BYTE	TermTDOLLen;
	BYTE	TermTDOL[128];          //terminal stored default TDOL.
	BYTE    MerchNameLocateLen;
	BYTE    MerchNameLocate[128];   //EMV2000 new added
	BYTE	TransLogMaxNum[2];		//max transLogs stored for check floor limit(default 20)
	BYTE	Threshold[4];				//threshold for random selection.
	BYTE	FloorLimit[4];				//tag'9F1B' terminal floor limit
	
	//sencond part are transaction various data and need to be saved.
	BYTE	AmtTrans[4];               //used in online financial or batch capture msg.
	BYTE	AmtNet[4];                 //total accumulative amount for reconciliation.
	BYTE	BatchTransNum[2];	//number of trans stored in terminal.used for reconciliation
	BYTE	TransNum[2];        //numbers of floorlimit translog for floorlimit check.
	BYTE	TransIndex[2];      //added for new floorlimit translog insert position.
	BYTE	TransSeqCount[4];		//increment by 1 for each trans. BCD numeric.

	//third part are transaction dependent data and needn't be save.
	//But they may be used in processing DOL. 
	BYTE	AmtAuthBin[4];				//tag'81' Authorised amount of binary
	BYTE    AmtAuthNum[6];			//tag'9F02' Authorised amount of BCD numeric
	BYTE	AmtOtherBin[4];			//tag'9F04' Other(cashback) amount of binary
	BYTE    AmtOtherNum[6];         //tag'9F03' Other(cashback) amount of BCD numeric
	BYTE	AmtReferCurcy[4];			//tag'9F3A' Authorised amount in the reference currency
	BYTE	AIDLen;
	BYTE	AID[16];				//tag'9F06' Application Identifier for selected application,5-16
	BYTE    AuthorCode[6];          //tag'89'   ret from issuer.move to TermInfo from global variable in P70.
	BYTE	AuthRespCode[2];        //tag'8A'   Authorised respose code received from host.
	BYTE	CVMResult[3];			//tag'9F34' cardholder verification methods perform result
	BYTE	POSEntryMode;			//tag'9F39' POS entry mode,BCD
	BYTE    PINLen;
	BYTE    PIN[12];                //tag'99'
	BYTE	TVR[5];					//tag'95'   Terminal Verification Results
	BYTE    TSI[2];                 //tag'9B' Transaction Status Information 
	BYTE	VLPIndicator;			//tag'9F7A' //0-not support; 1-support; 2-VLP only. variable parameter to indicate if this trans is VLP supported.
	BYTE    TransDate[3];           //tag'9A'   YYMMDD
	BYTE    TransTime[3];           //tag'9F21',HHMMSS,BCD
	BYTE    TCHashValue[20];        //tag'98'
	BYTE    UnpredictNum[4];        //tag'9F37' Terminal created for each transaction.
	BYTE    IssuerAuthenDataLen;
	BYTE    IssuerAuthenData[16];   //tag'91'   Issuer Authentication Data.
	BYTE    MCHIPTransCateCode;     //tag '9F53' Transaction Category Code, Mastercard M/Chip private data.
}TERMINFO;

/*EXTERN */TERMINFO TermInfo;

#define SSLMAXSENDLEN		4000
#define SSL_SERVER_CERTIATE1   "SSL_SERVER_CERTIATE1.dat"
#define SSL_SERVER_CERTIATE2   "SSL_SERVER_CERTIATE2.dat"
#define SSL_LOCAL_CERT_LOCAL_KEY   "SSL_LOCAL_CERT_LOCAL_KEY.dat"

int SxxSSLConnect(uchar *RemoteIP, uchar *RemotePort, int iTimeOut);
int SxxSSLSend(char *pBuffer, int nSize, ushort sTimeOut);
int SxxSSLRecv(char *pBuffer, int nSize, ushort sTimeOut, ushort *sOutLen);
int SxxSSLOnHook(void);
int ReadPemFile(uchar* pFileName, uchar* psCA, uchar* header, uchar* footer);

CURCOMM_PARA gCurComPara; //define set
#define RESOLVE_DNS_RETRY    3
#define CA_MAX_FILESIZE                         4000
#define MAX_CA          4
#define IP_RETRY          3
#define MAX_PEM_FILE_LEN			6000

#define PP_PEM          /*"PP.PEM"*/"Go_Daddy_SHA2_Root_CA(PEM).pem"
#define PEM_HEADER      "-----BEGIN CERTIFICATE-----"
#define PEM_FOOTER      "-----END CERTIFICATE-----"
//#define PP_CA1          "PP_CA1.PEM"    //HASE GENERIC BUILD111
//#define PP_CA2          "PP_CA2.PEM"    //HASE GENERIC BUILD111

#define PP_CA_PEM_HEADER      "-----BEGIN CERTIFICATE-----"//"-----PP CA BEGIN CERTIFICATE-----"
#define PP_CA_PEM_FOOTER      "-----END CERTIFICATE-----"//"-----PP CA END CERTIFICATE-----" 


#define PP_ROOT_PEM_HEADER    "-----PP ROOT BEGIN CERTIFICATE-----"
#define PP_ROOT_PEM_FOOTER    "-----PP ROOT END CERTIFICATE-----"

//add by richard 20161209 for S90 3G ssl test.
#define FUBON_PEM      "zeta-selfsign-sha2.pem" 
#define FUBON_CA_PEM_HEADER   "-----BEGIN CERTIFICATE-----"
#define FUBON_CA_PEM_FOOTER   "-----END CERTIFICATE-----"


static char caPEM[CA_MAX_FILESIZE];
static char rootCaPEM[CA_MAX_FILESIZE];
static char caPEM1[CA_MAX_FILESIZE];
static char caPEM2[CA_MAX_FILESIZE];

BYTE fileWriteConnect(void);

//Gillian 2016-8-19
  #ifdef PP_SSL
int SSL_PP_GetTCPInfo(TCPIP_PARA *pTcpPara, char* hostCN);
int SSL_GetTCPInfo(TCPIP_PARA *pTcpPara, char* hostCN);
void SSL_PrintError(int errID);
  #endif  //end PP_SSL
typedef struct _SSLTIMEZONE
{
	uchar   sCountryCode[2];   
	short hour;/* <0表示-, >0表示+ */
	short min;
}SSLTIMEZONE;


SSLTIMEZONE TimeZoneTable[]=
{
    "\x04\x62",-5,0,
    "\x03\x56",-5,30,
    "\x01\x44",-5,30,
    "", 0, 0,
};

#if 0
static int net_open(char *remote_addr, short remote_port, short local_port, long flag)
{
    int s, iErr;
    struct net_sockaddr addr;

    /*  if(glCommCfg.ucCommType == CT_WIFI)
        {
            //TCP/IP连接
            //TCP/IP connection
            s=tcpipConnect(remote_addr,remote_port,0, (ushort)sg_Timeout);
            if(s<0 || s>9)
            {
                //如果连接建立失败，则需要重新初始化，然后再尝试连接，仍然失败就返回错误
                //if connection failed, then initialize wifi module and try again. if fialed again, then quit with error code.
                tcpipAbortSocketCommand();
                glCommCfg.stTcpIpPara.ucDhcp = 1;
                WIFIInit(&glCommCfg);
                tcpipForceHardReset();
                SetEchoMode(0);

                s=tcpipConnect(remote_addr,remote_port,0, (ushort)sg_Timeout);
                if(s<0 || s>9)
                {
                    tcpipAbortSocketCommand();
                    return ERR_SSL_TIMEOUT;
                }
                else
                {
                    return s;
                }
            }
            else
            {
                return s;
            }
            return ERR_SSL_TIMEOUT;
        }
        else*/
    {
        s = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);
        if (s < 0)
        {
 		#if 0  
  		kbflush();
		ScrCls();
		Lcdprintf("net_open(1) s=%d", s);	
		PubWaitKey(40);
		#endif        
            return s;   //FAIL
        }
        iErr = SockAddrSet(&addr, remote_addr, remote_port);

        iErr = Netioctl(s, CMD_TO_SET, sg_Timeout*1000);

        if (iErr!=0)
        {
 		#if 0  
  		kbflush();
		ScrCls();
		Lcdprintf("net_open(2) iErr=%d", iErr);	
		PubWaitKey(40);
		#endif          
            return iErr;
        }

        iErr = NetConnect(s, &addr, sizeof(addr));
        if (iErr < 0)
        {
  		#if 0  
  		kbflush();
		ScrCls();
		Lcdprintf("net_open(3)  iErr=%d", iErr);	
		PubWaitKey(40);
		#endif         
            NetCloseSocket(s);
            return iErr;
        }

 		#if 0  
  		kbflush();
		ScrCls();
		Lcdprintf("net_open(4) s=%d", s);	
		PubWaitKey(40);
		#endif  		
        return s;
    }

}


static int net_send(int net_hd, uchar *psTxdData, int uiDataLen)
{

    int iRet;
    int iSendLen;
    int iSumLen, iExitFlg;

    /*  if(glCommCfg.ucCommType == CT_WIFI)
        {
            //发送数据
            //send tcp data
            iSumLen = 0;
            iExitFlg = 1;
            while(1)
            {
                if (uiDataLen > SSLMAXSENDLEN)
                {
                    iSendLen = SSLMAXSENDLEN;
                    uiDataLen = uiDataLen - SSLMAXSENDLEN;
                    iExitFlg = 0;
                }
                else
                {
                    iSendLen = uiDataLen;
                    iExitFlg = 1;
                }
                iRet=tcpipSendBuffer(net_hd, psTxdData+iSumLen,iSendLen, (ushort)sg_Timeout);
                //debug_print("tcpipSendBuffer", iRet);
                if (iRet != 0)
                {
                    return ERR_SSL_TIMEOUT;
                }
                iSumLen = iSumLen + iSendLen;
                if (iExitFlg)
                {
                    break;
                }
            }
            return iSumLen;
        }
        else*/
    {
        //
        iRet = Netioctl(net_hd, CMD_TO_SET, sg_Timeout*1000);
        if (iRet < 0)
        {
            return iRet;
        }

        iSumLen = 0;
        iExitFlg = 1;
        while(1)
        {
            if (uiDataLen > SSLMAXSENDLEN)
            {
                iSendLen = SSLMAXSENDLEN;
                uiDataLen = uiDataLen - SSLMAXSENDLEN;
                iExitFlg = 0;
            }
            else
            {
                iSendLen = uiDataLen;
                iExitFlg = 1;
            }

            iRet = NetSend(net_hd, psTxdData+iSumLen, iSendLen, 0);
            PubDebugTx("NetSend:%d sg_Timeout:%d iRet:%d\n", iSendLen, sg_Timeout*1000, iRet);
            if (iRet < 0)
            {
                return iRet;
            }

            if (iRet != iSendLen)
            {
                return -1;
            }
            iSumLen = iSumLen + iSendLen;
            if (iExitFlg)
            {
                break;
            }
        }
        return iSumLen;
    }
}

static int net_recv(int net_hd, uchar *buf, int size)
{
    int iRet;
    int iCountTimeout = 0;

    iRet =  Netioctl(net_hd, CMD_TO_SET, sg_Timeout * 1000);

    if (iRet < 0)
    {
        return iRet;
    }

    while(1)
    {
        iRet = NetRecv(net_hd, buf, size, 0);

        if (iRet < 0)
        {
            if (iRet == -13)           /*try 3 times if error is -13*/
            {
                iCountTimeout++;

                if (iCountTimeout < 3)
                {
                    continue;
                }
                else                /*already got 3 times error -13*/
                {
                    break;
                }
            }
            /*other error code,break*/
            break;
        }
        break;
    }

    return iRet;
}

#if 0
static int net_recv(int net_hd, void *psRxdData, int uiExpLen)
{
    int iRet;
    int iLen = 0;
    uchar szBuff[64] = "";
    uchar *pData;

    pData = psRxdData;

    /*  if(glCommCfg.ucCommType == CT_WIFI)
        {

            //debug_print("tcpipRecvBuffer len", uiExpLen);
            iRet=tcpipRecvBuffer(net_hd ,psRxdData, uiExpLen, &iLen);
            //debug_print("tcpipRecvBuffer", iRet);
            if(iRet!=0)
            {
                return ERR_SSL_TIMEOUT;
            }

            return iLen;
        }
        else*/
    {
        //
        iRet =  Netioctl(net_hd, CMD_TO_SET, sg_Timeout*1000);
        if (iRet < 0)
        {
            return iRet;
        }
//        TimerSet(4, 60);//Jason  2014.08.25 14:56
        iRet = 0;
//        while(1)
//        {
//            iResult = UpdateTimer(4, (ushort *)&sg_Timeout);//Jason  2014.08.25 14:56
//            if( iResult != 0)
//            {
//                break;
//            }

            iRet = NetRecv(net_hd, psRxdData, uiExpLen, 0);
            PubDebugTx("NetRecv:%d sg_Timeout:%d iRet:%d\n",uiExpLen, sg_Timeout*1000, iRet);

//            if(iRet>=0)
//            {
//                break;
//            }
        }
//
//        if (iRet < 0)
//        {
//            return iRet;
//        }

        return iRet;
//    }
}
#endif

static int net_close(int net_hd)
{
  int iRet;

    /*  if(glCommCfg.ucCommType == CT_WIFI)
        {
            //断开TCP连接
            //disconnect TCP
            tcpipDisConnect(net_hd);
            //取消正在使用的SOCKET
            //abort using socket
            iRet = tcpipAbortSocketCommand();
            if (iRet != 0)
            {
                return ERR_SSL_HANDLE;
            }
            return 0;
        }
        else*/
    {
        iRet = NetCloseSocket(net_hd);
        PubDebugTx("net_close:%d\n", iRet);
        return 0;
    }
}
#else if
static int net_open(char *remote_addr, short remote_port, short local_port, 
long flag)
{
    int s, err;
    struct net_sockaddr addr;

    int timeout;   //ver20160406

    s = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);

    if (s < 0)
    {
        return s;               //FAIL
    }
    err = SockAddrSet(&addr, remote_addr, remote_port);

    /*ver20160406: bound the connection timeout, 3s ~ 5s
    if (sg_Timeout < 3)
    {
        sg_Timeout = 3;
    }
    */
    timeout = sg_Timeout;

    if (timeout < 3)
    {
      timeout = 3;
    }

    if (timeout > 5)
    {
      timeout = 5;
    }
    //end 20160406


    err = Netioctl(s, CMD_TO_SET, timeout * 1000);

    if (err != 0)
    {
        return err;
    }
    err = NetConnect(s, &addr, sizeof(addr));

    if (err < 0)
    {
        NetCloseSocket(s);
        return err;
    }
    return s;
}

static int net_send(int net_hd, uchar *buf, int size)
{
    int iRet;
    int iSendLen;
    int iSumLen, iExitFlg;

    iRet = Netioctl(sg_iSocket, CMD_TO_SET, sg_Timeout * 1000);

    if (iRet < 0)
    {
        return iRet;
    }
    iSumLen = 0;
    iExitFlg = 1;

    while(1)
    {
        if (size > SSLMAXSENDLEN)
        {
            iSendLen = SSLMAXSENDLEN;
            size = size - SSLMAXSENDLEN;
            iExitFlg = 0;
        }
        else
        {
            iSendLen = size;
            iExitFlg = 1;
        }
        iRet = NetSend(sg_iSocket, buf + iSumLen, iSendLen, 0);

        if (iRet < 0)
        {
            return iRet;
        }

        if (iRet != iSendLen)
        {
            return -1;
        }
        iSumLen = iSumLen + iSendLen;

        if (iExitFlg)
        {
            break;
        }
    }

    return iSumLen;
}

static int net_recv(int net_hd, uchar *buf, int size)
{
    int iRet;
    int iCountTimeout = 0;

    iRet =  Netioctl(net_hd, CMD_TO_SET, sg_Timeout * 1000);

    if (iRet < 0)
    {
        return iRet;
    }

    while(1)
    {
        iRet = NetRecv(net_hd, buf, size, 0);

        if (iRet < 0)
        {
            if (iRet == -13)           /*try 3 times if error is -13*/
            {
                iCountTimeout++;

                if (iCountTimeout < 3)
                {
                    continue;
                }
                else                /*already got 3 times error -13*/
                {
                    break;
                }
            }
            /*other error code,break*/
            break;
        }
        break;
    }

    return iRet;
}

static int net_close(int net_hd)
{
    NetCloseSocket(net_hd);
    return 0;
}


#endif

SSL_NET_OPS  s80_ssl_ops=
{
    net_open,
    net_send,
    net_recv,
    net_close
};




static int ReadSysTime(SYSTEM_TIME_T *t)
{
    uchar time[6+1];
    int i;
    ulong tTransfer;
    SYSTEM_TIME_T stSystemTime;
    memset(&stSystemTime,0,sizeof(stSystemTime));

    GetTime(time);
    //Year
    PubBcd2Long(time,1,&tTransfer);
    stSystemTime.year = (short)(2000+tTransfer);



    //month
    PubBcd2Long(&time[1],1,&tTransfer);
    stSystemTime.month=(short)tTransfer;

    //day
    PubBcd2Long(&time[2],1,&tTransfer);
    stSystemTime.day=(short)tTransfer;


    //hour
    PubBcd2Long(&time[3],1,&tTransfer);
    stSystemTime.hour=(short)tTransfer;

    //min
    PubBcd2Long(&time[4],1,&tTransfer);
    stSystemTime.min=(short)tTransfer;

    //sec

    PubBcd2Long(&time[5],1,&tTransfer);
    stSystemTime.sec=(short)tTransfer;


    //time zone
    stSystemTime.zone.hour=-8;
    stSystemTime.zone.min=0;
    for (i=0; i<3; i++)
    {
        if (memcmp(TermInfo.CountryCode,&TimeZoneTable[i].sCountryCode,2)==0)
        {
            stSystemTime.zone.hour=TimeZoneTable[i].hour;
            stSystemTime.zone.min=TimeZoneTable[i].min;
        }
    }


    memcpy(t,&stSystemTime,sizeof(stSystemTime));

//  ScrCls();
//  ScrPrint(1,0,ASCII,"YEAR=%d",t->year);
//  ScrPrint(1,2,ASCII,"month=%d",t->month);
//  ScrPrint(1,4,ASCII,"day=%d",t->day);
//  ScrPrint(1,6,ASCII,"hour=%d",t->hour);
//
//  getkey();//testtttt
//  ScrCls();
//  ScrPrint(1,2,ASCII,"min=%d",t->min);
//  ScrPrint(1,4,ASCII,"sec=%d",t->sec);
//  getkey();//testtttt


    return 0;
}

#if 0
static int GetRandom(unsigned char *buf/* OUT */, int len)
{
    int value, size;
    while(len>0)
    {
        value = rand();
        size = len>4?4:len;
        memcpy(buf, &value, size);
        len -= size;
    }
    return 0;
}
#endif

static char caPEM[]=
{
    "MIIE0DCCBDmgAwIBAgIQJQzo4DBhLp8rifcFTXz4/TANBgkqhkiG9w0BAQUFADBf\n"
    "MQswCQYDVQQGEwJVUzEXMBUGA1UEChMOVmVyaVNpZ24sIEluYy4xNzA1BgNVBAsT\n"
    "LkNsYXNzIDMgUHVibGljIFByaW1hcnkgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkw\n"
    "HhcNMDYxMTA4MDAwMDAwWhcNMjExMTA3MjM1OTU5WjCByjELMAkGA1UEBhMCVVMx\n"
    "FzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQLExZWZXJpU2lnbiBUcnVz\n"
    "dCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJpU2lnbiwgSW5jLiAtIEZv\n"
    "ciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxWZXJpU2lnbiBDbGFzcyAz\n"
    "IFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9yaXR5IC0gRzUwggEi\n"
    "MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvJAgIKXo1nmAMqudLO07cfLw8\n"
    "RRy7K+D+KQL5VwijZIUVJ/XxrcgxiV0i6CqqpkKzj/i5Vbext0uz/o9+B1fs70Pb\n"
    "ZmIVYc9gDaTY3vjgw2IIPVQT60nKWVSFJuUrjxuf6/WhkcIzSdhDY2pSS9KP6HBR\n"
    "TdGJaXvHcPaz3BJ023tdS1bTlr8Vd6Gw9KIl8q8ckmcY5fQGBO+QueQA5N06tRn/\n"
    "Arr0PO7gi+s3i+z016zy9vA9r911kTMZHRxAy3QkGSGT2RT+rCpSx4/VBEnkjWNH\n"
    "iDxpg8v+R70rfk/Fla4OndTRQ8Bnc+MUCH7lP59zuDMKz10/NIeWiu5T6CUVAgMB\n"
    "AAGjggGbMIIBlzAPBgNVHRMBAf8EBTADAQH/MDEGA1UdHwQqMCgwJqAkoCKGIGh0\n"
    "dHA6Ly9jcmwudmVyaXNpZ24uY29tL3BjYTMuY3JsMA4GA1UdDwEB/wQEAwIBBjA9\n"
    "BgNVHSAENjA0MDIGBFUdIAAwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly93d3cudmVy\n"
    "aXNpZ24uY29tL2NwczAdBgNVHQ4EFgQUf9Nlp8Ld7LvwMAnzQzn6Aq8zMTMwbQYI\n"
    "KwYBBQUHAQwEYTBfoV2gWzBZMFcwVRYJaW1hZ2UvZ2lmMCEwHzAHBgUrDgMCGgQU\n"
    "j+XTGoasjY5rw8+AatRIGCx7GS4wJRYjaHR0cDovL2xvZ28udmVyaXNpZ24uY29t\n"
    "L3ZzbG9nby5naWYwNAYIKwYBBQUHAQEEKDAmMCQGCCsGAQUFBzABhhhodHRwOi8v\n"
    "b2NzcC52ZXJpc2lnbi5jb20wPgYDVR0lBDcwNQYIKwYBBQUHAwEGCCsGAQUFBwMC\n"
    "BggrBgEFBQcDAwYJYIZIAYb4QgQBBgpghkgBhvhFAQgBMA0GCSqGSIb3DQEBBQUA\n"
    "A4GBABMC3fjohgDyWvj4IAxZiGIHzs73Tvm7WaGY5eE43U68ZhjTresY8g3JbT5K\n"
    "lCDDPLq9ZVTGr0SzEK0saz6r1we2uIFjxfleLuUqZ87NMwwq14lWAyMfs77oOghZ\n"
    "tOxFNfeKW/9mz1Cvxm1XjRl4t7mi0VfqH5pLr7rJjhJ+xr3/\n"
};

static char rootCaPEM[]=
{
    "MIICPDCCAaUCEDyRMcsf9tAbDpq40ES/Er4wDQYJKoZIhvcNAQEFBQAwXzELMAkG\n"
    "A1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMTcwNQYDVQQLEy5DbGFz\n"
    "cyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTk2\n"
    "MDEyOTAwMDAwMFoXDTI4MDgwMjIzNTk1OVowXzELMAkGA1UEBhMCVVMxFzAVBgNV\n"
    "BAoTDlZlcmlTaWduLCBJbmMuMTcwNQYDVQQLEy5DbGFzcyAzIFB1YmxpYyBQcmlt\n"
    "YXJ5IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIGfMA0GCSqGSIb3DQEBAQUAA4GN\n"
    "ADCBiQKBgQDJXFme8huKARS0EN8EQNvjV69qRUCPhAwL0TPZ2RHP7gJYHyX3KqhE\n"
    "BarsAx94f56TuZoAqiN91qyFomNFx3InzPRMxnVx0jnvT0Lwdd8KkMaOIG+YD/is\n"
    "I19wKTakyYbnsZogy1Olhec9vn2a/iRFM9x2Fe0PonFkTGUugWhFpwIDAQABMA0G\n"
    "CSqGSIb3DQEBBQUAA4GBABByUqkFFBkyCEHwxWsKzH4PIRnN5GfcX6kb5sroc50i\n"
    "2JhucwNhkcV8sEVAbkSdjbCxlnRhLQ2pRdKkkirWmnWXbj9T/UWZYB2oK0z5XqcJ\n"
    "2HUw19JlYD1n1khVdWk/kfVIC0dpImmClr7JyDiGSnoscxlIaU5rfGW/D/xwzoiQ\n"
};

//年份表
static const unsigned int  gYearTable[100] =
{
    0,  366,  731, 1096, 1461, 1827, 2192, 2557, 2922, 3288,
    3653, 4018, 4383, 4749, 5114, 5479, 5844, 6210, 6575, 6940,
    7305, 7671, 8036, 8401, 8766, 9132, 9497, 9862,10227,10593,
    10958,11323,11688,12054,12419,12784,13149,13515,13880,14245,
    14610,14976,15341,15706,16071,16437,16802,17167,17532,17898,
    18263,18628,18993,19359,19724,20089,20454,20820,21185,21550,
    21915,22281,22646,23011,23376,23742,24107,24472,24837,25203,
    25568,25933,26298,26664,27029,27394,27759,28125,28490,28855,
    29220,29586,29951,30316,30681,31047,31412,31777,32142,32508,
    32873,33238,33603,33969,34334,34699,35064,35430,35795,36160
};

//非闰年
static const unsigned int  gMonthTable1[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
//闰年
static const unsigned int  gMonthTable2[12] = {0,31,60,91,121,152,182,213,244,274,305,335};

static unsigned long Time(unsigned long *t)
{

    uchar time[7];
    ulong tlong;
    uchar ch,ch1;
    GetTime(time);
    // year
    ch = (time[0]/16)*10+time[0]%16;
    tlong = (ulong)((uint)gYearTable[ch]);
    // month
    ch1 = (time[1]/16)*10+time[1]%16;
    if(ch%4 == 0)
        tlong += (ulong)((uint)gMonthTable2[ch1-1]);
    else
        tlong += (ulong)((uint)gMonthTable1[ch1-1]);
    // day
    ch = (time[2]/16)*10+time[2]%16;
    tlong += (ulong)((uchar)(ch-1));
    tlong *= 24;
    // hout
    ch = (time[3]/16)*10+time[3]%16;
    tlong += (ulong)((uchar)ch);
    tlong *= 60;
    // minute
    ch = (time[4]/16)*10+time[4]%16;
    tlong += (ulong)((uchar)ch);
    tlong *= 60;
    // second
    ch = (time[5]/16)*10+time[5]%16;
    tlong += (ulong)((uchar)ch);

//  tlong +=(ulong)((22*365+8*366)*24*60*60);
    if (t != NULL) *t = tlong;
    return tlong;

}
static char * reason_str(CERT_INVAL_CODE reason)
{
    char * descr[]=
    {
        "CERT_BAD",
        "CERT_TIME",
        "CERT_CRL",
        "CERT_SIGN",
        "CERT_CA_TIME",
        "CERT_CA_CRL",
        "CERT_CA_SIGN",
        "CERT_MAC",
        "CERT_MEM",
        "CERT_ISSUER",
    };
    if(reason>0&&reason<=(sizeof(descr)/sizeof(char*)))
    {
        return descr[reason-1];
    }
    return "N/A";
}

static int ServCertAck(CERT_INVAL_CODE reason)
{
    int ret;
    if(reason == CERT_ISSUER)
    {
        ret = 0;//ignore, continue;
    }
    else
    {
        ret = -1;
    }

    //PubDebugTx("ServCertAck reason:%d\n", reason);
    return 0; //add by richard 201612
//  if (reason != CERT_ISSUER)  {
    ScrClrLine(1,7);
    ScrPrint(0,2,ASCII,"Because of %d(%s), %s connect!",reason, reason_str(reason),
             ret==0?"Continue":"Cancel");
    if (getkey()==KEYENTER)
    {
        return 0;
    }
    //  else
    //  {
    //      return ret;
    //  }
//  }
//  else
//  {
    return ret;
//  }

}

/*
SSL_SYS_OPS  s80_sys_ops=
{
    ReadSysTime,
    GetRandom,
    Time,
    ServCertAck
};*/


static int s80_sys_time(SYSTEM_TIME_T *date)
{
#define BCD2I(v) ((((v) >> 4) & 0xf) * 10 + ((v) & 0xf))
    unsigned char t[7];
    GetTime(t);
    date->year = 2000 + BCD2I(t[0]);
    date->month = BCD2I(t[1]);
    date->day = BCD2I(t[2]);
    date->hour = BCD2I(t[3]);
    date->min = BCD2I(t[4]);
    date->sec = BCD2I(t[5]);
    date->zone.hour = 0;
    date->zone.min = 0;
    return 0;
}

static void GetRandom(unsigned char *pucDataOut)
{
    int i;

    for(i = 0; i < 8; i++)
    {
        pucDataOut[i] = i;
    }
}

static int s80_random(unsigned char *buf, int len)
{
    unsigned char value[8];
    int i;

    while(len > 0)
    {
        GetRandom(value);

        for(i = 0; i < 8 && len > 0; i++, len--, buf++)
        {
            *buf = value[i];
        }
    }

    return 0;
}

SSL_SYS_OPS s80_sys_ops =
{
    s80_sys_time,
    s80_random,
    NULL,
    ServCertAck,
};


int ReadLine(int fs, void *vptr, int MaxLen)
{
    int n;
    unsigned char *ptr;

    ptr = vptr;
    for(n=0; n<MaxLen; n++)
    {
        if(read(fs, ptr, 1) <= 0)
            return -1;

        if((*ptr == 0x0a) || (*ptr == 0x0d))
        {
            *ptr = 0;
            break;
        }
        ptr ++;
    }

    return n;
}


#if defined(SXX_IP_MODULE) || defined(SXX_WIRELESS_MODULE)



int GetCertKey(uchar *filename, uchar *szCert, uchar *szKey)
{
    int j;
    unsigned char buff[80], value[80];
    int ParamsFid;
#define NONE        0
#define  PRIV_KEY 1
#define  CERTIATE 2
#define  CA  3
    uchar type = NONE;

    ParamsFid = open(filename, O_RDWR);	
    if (ParamsFid < 0)
    {
        return 1;
    }

    for(;;)
    {
        memset(buff, 0x00, sizeof(buff));
        memset(value, 0x00, sizeof(value));
        j = ReadLine(ParamsFid, value, sizeof(value));
        if ( j<0 )
        {
            break;/*文件结束*/
        }
        if ( j>=0 )/*读到一行没有'='*/
        {
            if(PubStrNoCaseCmp((uchar *)value, (uchar *)"-----BEGIN RSA PRIVATE KEY-----")==0 )
            {
                type = PRIV_KEY;
                continue;
            }
            else if(PubStrNoCaseCmp((uchar *)value, (uchar *)"-----END RSA PRIVATE KEY-----")==0 )
            {
                type = NONE;
                continue;
            }
            else if(PubStrNoCaseCmp((uchar *)value, (uchar *)"-----BEGIN CERTIFICATE-----")==0 )
            {
                type = CERTIATE;
                continue;
            }
            else if(PubStrNoCaseCmp((uchar *)value, (uchar *)"-----END CERTIFICATE-----")==0 )
            {
                type = NONE;
                continue;
            }
            else
            {
                strcpy(buff, value);
                strcat(buff, "\n");
                switch(type)
                {
                    case PRIV_KEY:
                        //szKey->size += strlen(buff);
                        if (szKey[0] != 0)
                        {
                            strcat(szKey, buff);
                        }
                        else
                        {
                            strcpy(szKey, buff);
                        }
                        break;
                    case CERTIATE:
                        //szCert->size += strlen(buff);
                        if (szCert[0] != 0)
                        {
                            strcat(szCert, buff);
                        }
                        else
                        {
                            strcpy(szCert, buff);
                        }
                        break;
                }
                continue;
            }
        }
        if (value[0] == 0x0a)
        {
            continue;
        }
    }
    close(ParamsFid);
    return 0;
}



int SslExtraKey(uchar *filename, uchar *szCert, int *iCertLen, uchar *szKey, int *iKeyLen)
{
    int iRet, length;
    char pos_cer_buf[2048];
    char pos_privatekey_buf[6000];

	if(filename == NULL)
	{
		return -1;
	}

    memset(pos_cer_buf, 0, sizeof(pos_cer_buf));
    memset(pos_privatekey_buf, 0, sizeof(pos_privatekey_buf));
    iRet = GetCertKey(filename, pos_cer_buf, pos_privatekey_buf);
    if (iRet != 0)
    {
		if(iCertLen != NULL)
			*iCertLen = 0;

		if(iKeyLen != NULL)
			*iKeyLen = 0; 
        return -2;
    }

	if(szCert != NULL && iCertLen != NULL)
	{
	    length = SslDecodePem(pos_cer_buf, strlen(pos_cer_buf), szCert, *iCertLen); //SSL11
	    if (length <= 0)
	    {
			if(iCertLen != NULL)
				*iCertLen = 0;

			if(iKeyLen != NULL)			
				*iKeyLen = 0;  
	        return -3;
	    }
		
		if(iCertLen != NULL)
	    	*iCertLen = length;
	}	

	if(szKey != NULL && iKeyLen != NULL)
	{
	    length = SslDecodePem(pos_privatekey_buf, strlen(pos_privatekey_buf), szKey, *iKeyLen); //SSL11
	    if (length <= 0)
	    {
		if(iCertLen != NULL)
			*iCertLen = 0; // ???

		if(iKeyLen != NULL)	
			*iKeyLen = 0;     
	        return -4;
	    }

		if(iKeyLen != NULL)
	    	*iKeyLen = length;
	}	

    return 0;
}

BYTE fileWriteConnect(void)
{
	//notice
    return PubFileWrite(CONNECTONE_FILE, 0, (BYTE *)&gCurComPara, sizeof(gCurComPara));
}

#ifdef PP_SSL //HASE GENERIC BUILD106
int SSL_GetTCPInfo(TCPIP_PARA *pTcpPara, char* hostCN)
{
    int iRet, retry;
    char ipBuf[URL_MAX_LEN], urlBuf[URL_MAX_LEN];
    char ipPort[PORT_MAX_LEN];

    pTcpPara->ucDhcp = FALSE;
		//local IP info
    memcpy(pTcpPara->szLocalIP, gCurComPara.LocalParam.szLocalIP, strlen(gCurComPara.LocalParam.szLocalIP));
    memcpy(pTcpPara->szNetMask, gCurComPara.LocalParam.szNetMask, strlen(gCurComPara.LocalParam.szNetMask));
    memcpy(pTcpPara->szGatewayIP, gCurComPara.LocalParam.szGatewayIP, strlen(gCurComPara.LocalParam.szGatewayIP));  
    memcpy(pTcpPara->szDNSIP, gCurComPara.LocalParam.szRemoteIP_1, strlen(gCurComPara.LocalParam.szDNSIP));


    //SSL	Host
    memset(ipBuf, 0, URL_MAX_LEN);
    memset(urlBuf, 0, URL_MAX_LEN);
    memset(ipPort, 0, PORT_MAX_LEN);


    memcpy(urlBuf, glSysParam.stTxnCommCfg.szSSL_URL, URL_MAX_LEN);		
    memcpy(hostCN, glSysParam.stTxnCommCfg.szSSL_URL, URL_MAX_LEN);		//host CN same as host domain name
    memcpy(ipPort, glSysParam.stTxnCommCfg.szSSL_Port, PORT_MAX_LEN);

    SxxTcpOnHook();	   //modified by richard 20161221, v1.00.0218
    if(!ChkTerm(_TERMINAL_S90_))
    {
        EthSet(pTcpPara->szLocalIP, pTcpPara->szNetMask, pTcpPara->szGatewayIP, pTcpPara->szDNSIP);
    }

	for(retry = 0; retry < RESOLVE_DNS_RETRY; retry++)
	{
		iRet = DnsResolve(urlBuf, ipBuf, URL_MAX_LEN);		
		if(iRet == 0)
		{
			break;
		}
	}

    /*IK76s.09: to make sure the resolved IP exist
    if(iRet == 0)*/
    if(iRet == 0 && strlen(ipBuf) != 0 && ipBuf[0] != 0)
    {
      //Host 1
      strcpy((char*)pTcpPara->szRemoteIP_1, (char*)ipBuf);//, strlen(ipBuf));
      memcpy(pTcpPara->szRemotePort_1, ipPort, PORT_MAX_LEN);

      //Host 2
      memcpy(pTcpPara->szRemoteIP_2, ipBuf, strlen(ipBuf));
      memcpy(pTcpPara->szRemotePort_2, ipPort, PORT_MAX_LEN);
 
      if(ChkTerm(_TERMINAL_S90_)) //copy the resolved IP to wireless param
      {
          //IP
          memcpy(gCurComPara.LocalParam.szRemoteIP_1, ipBuf, strlen(ipBuf));
          memcpy(gCurComPara.LocalParam.szRemoteIP_2, ipBuf, strlen(ipBuf));

          //s9BLD095:  5 Jun 15, copy port# into wireless parameter
          memcpy(gCurComPara.LocalParam.szRemotePort_1, ipPort, PORT_MAX_LEN); 
          memcpy(gCurComPara.LocalParam.szRemotePort_2, ipPort, PORT_MAX_LEN);
      }

      //*** save SSL backup IP
      if((memcmp(ipBuf, glSysParam.stTxnCommCfg.szSSL_BackupIP, strlen(ipBuf)) != 0) || 
         (strlen(ipBuf) != strlen(glSysParam.stTxnCommCfg.szSSL_BackupIP)))
      {        
        memset(glSysParam.stTxnCommCfg.szSSL_BackupIP, 0, IP_ADDRESS_MAX_LEN);
        memcpy(glSysParam.stTxnCommCfg.szSSL_BackupIP, ipBuf, strlen(ipBuf));
        fileWriteConnect();        
      }
      //end save SSL backup IP
    }
    else
    {
      //make use of SSL backup IP by resolving DNS
      if(glSysParam.stTxnCommCfg.szSSL_BackupIP[0] != 0)
      {
        //Host 1
        memcpy(pTcpPara->szRemoteIP_1, glSysParam.stTxnCommCfg.szSSL_BackupIP, strlen(glSysParam.stTxnCommCfg.szSSL_BackupIP));
        memcpy(pTcpPara->szRemotePort_1, ipPort, PORT_MAX_LEN);

        //Host 2
        memcpy(pTcpPara->szRemoteIP_2, glSysParam.stTxnCommCfg.szSSL_BackupIP, strlen(glSysParam.stTxnCommCfg.szSSL_BackupIP));
        memcpy(pTcpPara->szRemotePort_2, ipPort, PORT_MAX_LEN);

        if(ChkTerm(_TERMINAL_S90_))  //copy the backup IP to wireless param
        {
            //IP
            memcpy(gCurComPara.LocalParam.szRemoteIP_1, glSysParam.stTxnCommCfg.szSSL_BackupIP, strlen(glSysParam.stTxnCommCfg.szSSL_BackupIP));
            memcpy(gCurComPara.LocalParam.szRemoteIP_2, glSysParam.stTxnCommCfg.szSSL_BackupIP, strlen(glSysParam.stTxnCommCfg.szSSL_BackupIP));

            //s9BLD095:  5 Jun 15, copy port# into wireless parameter
            memcpy(gCurComPara.LocalParam.szRemotePort_1, ipPort, PORT_MAX_LEN); 
            memcpy(gCurComPara.LocalParam.szRemotePort_2, ipPort, PORT_MAX_LEN);
        }

        iRet = 0;
      }
      // end SSL backup 
    }
	
		return iRet;
}


int SSL_PP_GetTCPInfo(TCPIP_PARA *pTcpPara, char* hostCN)
{
    int iRet, retry;
    char ipBuf[URL_MAX_LEN], urlBuf[URL_MAX_LEN];
    char ipPort[PORT_MAX_LEN];

    //if(!ChkTerm(_TERMINAL_S90_))
    //{
        pTcpPara->ucDhcp = FALSE;

        //local IP info
        memcpy(pTcpPara->szLocalIP, gCurComPara.LocalParam.szLocalIP, strlen(gCurComPara.LocalParam.szLocalIP));
        memcpy(pTcpPara->szNetMask, gCurComPara.LocalParam.szNetMask, strlen(gCurComPara.LocalParam.szNetMask));
        memcpy(pTcpPara->szGatewayIP, gCurComPara.LocalParam.szGatewayIP, strlen(gCurComPara.LocalParam.szGatewayIP));  
    	memcpy(pTcpPara->szDNSIP, gCurComPara.LocalParam.szDNSIP, strlen(gCurComPara.LocalParam.szDNSIP));
    //}

    //SSL	Host
    memset(ipBuf, 0, URL_MAX_LEN);
    memset(urlBuf, 0, URL_MAX_LEN);
    memset(ipPort, 0, PORT_MAX_LEN);


    memcpy(urlBuf, glSysParam.stTxnCommCfg.szPP_URL, URL_MAX_LEN);		
    memcpy(hostCN, glSysParam.stTxnCommCfg.szPP_URL, URL_MAX_LEN);		//host CN same as host domain name
    memcpy(ipPort, glCurAcq.stTxnTCPIPInfo[0].szPort, PORT_MAX_LEN);  //ENV file 

		 
    SxxTcpOnHook();	 //关闭套接字
    if(!ChkTerm(_TERMINAL_S90_))
    {
        iRet = EthSet(pTcpPara->szLocalIP, pTcpPara->szNetMask, pTcpPara->szGatewayIP, pTcpPara->szDNSIP);
    }

    for(retry = 0; retry < RESOLVE_DNS_RETRY; retry++)
    {
      iRet = DnsResolve(urlBuf, ipBuf, URL_MAX_LEN);	
      //PubDebugTx("SSL_PP_GetTCPInfo->DnsResolve iRet:%d urlBuf:%s ipBuf:%s\n", iRet,urlBuf ,ipBuf);
      if(iRet == 0)
      {
        break;
      }
    }


    if(iRet == 0 && strlen(ipBuf) != 0 && ipBuf[0] != 0)
    {
      if(ChkTerm(_TERMINAL_S90_))
      {
         //copy the resolved IP to wireless param
         memcpy(gCurComPara.LocalParam.szRemoteIP_1, ipBuf, strlen(ipBuf));
         memcpy(gCurComPara.LocalParam.szRemoteIP_2, ipBuf, strlen(ipBuf));
     
         //copy port# into wireless parameter
         memcpy(gCurComPara.LocalParam.szRemotePort_1, ipPort, PORT_MAX_LEN); 
         memcpy(gCurComPara.LocalParam.szRemotePort_2, ipPort, PORT_MAX_LEN);
      }

      //Host 1
	// memset(pTcpPara->szRemoteIP_1, 0, IP_ADDRESS_MAX_LEN);
      strcpy((char*)pTcpPara->szRemoteIP_1, (char*)ipBuf);//, strlen(ipBuf));

	  strcpy((char*)pTcpPara->szRemotePort_1, glCurAcq.stTxnTCPIPInfo[0].szPort);//, PORT_MAX_LEN);
      //Host 2
	  strcpy((char*)pTcpPara->szRemoteIP_2, ipBuf);//, strlen(ipBuf));
      strcpy((char*)pTcpPara->szRemotePort_2, glCurAcq.stTxnTCPIPInfo[0].szPort);//, PORT_MAX_LEN);

      //*** save SSL backup IP
      if((memcmp(ipBuf,glSysParam.stTxnCommCfg.szPP_SSL_BackupIP, strlen(ipBuf)) != 0) || 
         (strlen(ipBuf) != strlen(glSysParam.stTxnCommCfg.szPP_SSL_BackupIP)))
      {        
		strcpy((char*)glSysParam.stTxnCommCfg.szPP_SSL_BackupIP, ipBuf);//, strlen(ipBuf));

        fileWriteConnect();        
      }
      //end save SSL backup IP
    }
    else
    {
      //make use of SSL backup IP by resolving DNS
      if(glSysParam.stTxnCommCfg.szPP_SSL_BackupIP[0] != 0 && !ChkIfFubon())
      {     
        //Host 1
        memcpy(pTcpPara->szRemoteIP_1, glSysParam.stTxnCommCfg.szPP_SSL_BackupIP, strlen(glSysParam.stTxnCommCfg.szPP_SSL_BackupIP));
        memcpy(pTcpPara->szRemotePort_1, glSysParam.stTxnCommCfg.szPP_SSL_BackupIP, PORT_MAX_LEN);
		

        //Host 2
        memcpy(pTcpPara->szRemoteIP_2, glSysParam.stTxnCommCfg.szPP_SSL_BackupIP, strlen(glSysParam.stTxnCommCfg.szPP_SSL_BackupIP));
        memcpy(pTcpPara->szRemotePort_2, glCurAcq.stTxnTCPIPInfo[0].szPort, PORT_MAX_LEN);         

        iRet = 0;
      }
      else if(glSysParam.stTxnCommCfg.szSSL_BackupIP[0] != 0)
      {
          //Host 1
          memcpy(pTcpPara->szRemoteIP_1, glSysParam.stTxnCommCfg.szSSL_BackupIP, strlen(glSysParam.stTxnCommCfg.szSSL_BackupIP));
          memcpy(pTcpPara->szRemotePort_1, glSysParam.stTxnCommCfg.szSSL_Port, PORT_MAX_LEN);
          
          //Host 2
          memcpy(pTcpPara->szRemoteIP_2, glSysParam.stTxnCommCfg.szSSL_BackupIP, strlen(glSysParam.stTxnCommCfg.szSSL_BackupIP));
          memcpy(pTcpPara->szRemotePort_2, glSysParam.stTxnCommCfg.szSSL_Port, PORT_MAX_LEN); 
          iRet = 0;
      }
      // end SSL backup 
    }
    return iRet;
}
  #endif  //end PP_SSL


void SSL_PrintError(int errID)
{
  ScrCls();

  switch(errID)
  {
    case NET_ERR_DNS:
    PubDispString("DNS Resolve Err.", DISP_LINE_CENTER|2);
    break;
    case NET_ERR_CA_MISS:
    PubDispString("CA Missing", DISP_LINE_CENTER|2);
    break;
    default:
	PubDispString("Unknown SSL Err.", DISP_LINE_CENTER|2);
    break;
  }

  PubWaitKey(3);
}
//build AE76s.e4
int ReadPemFile(uchar* pFileName, uchar* psCA, uchar* header, uchar* footer)
{
	int		fd, iRet, iTempLen;
	uchar	*ptr1, *ptr2;
	uchar ucBuf[MAX_PEM_FILE_LEN]; 

	if (pFileName == NULL)
	{
		ScrClrLine(4,7);
        ScrPrint(0, 4, CFONT, "%s", pFileName);
		ScrPrint(0, 6, CFONT, "PEM MISS");
		PubWaitKey(4);
		return -1;
	}
		
	fd = open(pFileName, O_RDWR);
	if (fd < 0)
	{
		return fd;
	}
	

	memset(ucBuf, 0, MAX_PEM_FILE_LEN);
	iRet = read(fd, ucBuf, MAX_PEM_FILE_LEN);
	if (iRet < 0)
	{
		ScrClrLine(4,7);
		ScrPrint(0, 4, CFONT, "%s", pFileName);
		ScrPrint(0, 6, 1, "PEM FILE ERR:%d", iRet);
		PubWaitKey(4);

		close(fd);
		return iRet;	
	}
	close(fd);


	ptr1 = strstr(ucBuf, header);
    ptr2 = strstr(ucBuf, footer);


	if (ptr1 && ptr2)
	{
		iTempLen = strlen(ptr1) - strlen(ptr2) - strlen(header);
		memcpy(psCA, ptr1 + strlen(header), iTempLen);
        return 0;
	}
	else
	{
		ScrClrLine(6,7);
		ScrPrint(0, 6, 1, "CA MISS");
		PubWaitKey(4);

		return -1;	
	}	
}  

int SSL_LANTcpDial(TCPIP_PARA *pstTcpPara, char* hostCN)
{
  unsigned short portID;
  int iRet, socket, nRet;
  int length, i, totalCA = 0;

  char caCerBuf[CA_MAX_FILESIZE], rootCaCerBuf[CA_MAX_FILESIZE];
  char caCerBuf1[CA_MAX_FILESIZE], caCerBuf2[CA_MAX_FILESIZE];
  uchar RemoteIP[16];

  SSL_BUF_T  caCert, rootCaCert, caCert1, caCert2;
  SSL_BUF_T  serverCerts[MAX_CA];

  //关闭SSL 连接
  SslClose(sg_iSocket);

  if(!ChkTerm(_TERMINAL_S90_))
  {
    //set the Local IP for each TCP connection
    nRet= EthSet(pstTcpPara->szLocalIP, pstTcpPara->szNetMask, pstTcpPara->szGatewayIP, pstTcpPara->szDNSIP);
  }

  //*** CA process ***
  memset(caCerBuf, 0, CA_MAX_FILESIZE); 
  memset(caCerBuf1, 0, CA_MAX_FILESIZE); 
  memset(caCerBuf2, 0, CA_MAX_FILESIZE); 
  memset(rootCaCerBuf, 0, CA_MAX_FILESIZE); 
  
  totalCA = 0;

  //*** CA certification conversion
  memset(caPEM, 0, CA_MAX_FILESIZE);

  //只有一个.PEM文件
  if(ChkIfFubon())
  {
     iRet = ReadPemFile(FUBON_PEM, caPEM, FUBON_CA_PEM_HEADER, FUBON_CA_PEM_FOOTER);
  }
  else 
  {
     iRet = ReadPemFile(PP_PEM, caPEM, PP_CA_PEM_HEADER, PP_CA_PEM_FOOTER);
  }

  if(iRet == 0)
  {
    length = SslDecodePem(caPEM, strlen(caPEM), caCerBuf, CA_MAX_FILESIZE); //SSL11
    //PubDebugTx("After SslDecodePem length:%d\n", length);
    if(length > 0)
    {
      caCert.size = length;
      caCert.ptr = caCerBuf;
      serverCerts[totalCA] = caCert;
      totalCA++;      
    }
  }    
   
  //*** CA missing
  if(totalCA == 0)
  {
    SSL_PrintError(NET_ERR_CA_MISS);  
    return -1;
  }
  //end BUILD111

  iRet = -1;
  InitSSL(); //add by richard 20131223   
  for(i = 0; i < IP_RETRY; i++)
  {
    //创建一个SSL 套接字
    socket = SslCreate(); //SSL3
    if(socket < 0)
    {
      sg_iSocket = -1;
      iRet = socket;
      continue;
    }
    else 
    {
      sg_iSocket = socket;
    }
    //PubDebugTx("SslCreate sg_iSocket:%d\n",sg_iSocket);

	//连接SSL(HTTPS) 服务端
    iRet = SslCertsSet(sg_iSocket, serverCerts, totalCA, NULL, 0, 0, 0); //SSL4
    //PubDebugTx("SslCertsSet iRet:%d totalCA:%d\n",iRet, totalCA);

    //*** SSL Connection
    portID = atoi(pstTcpPara->szRemotePort_1);	

	memset(RemoteIP, 0, sizeof(RemoteIP));

    strcpy((char*)RemoteIP, (char*)pstTcpPara->szRemoteIP_1);

    iRet = SslConnect(sg_iSocket, RemoteIP, portID, 0, NULL); //SSL7
    //PubDebugTx("SslConnect iRet:%d\n",iRet);

    if(iRet == 0)
    {
      break;
    }
    else 
    {		
      SslClose(sg_iSocket); //SSL10
      continue;
    }
  }
  return iRet;
}

/*砞竚IP*/
//Gillian 2016-8-29 
static int get_ip_addr(uchar x, uchar y, char *ip_addr) 
{
    int pos = 0;
    unsigned char ch;
    unsigned char disp_buf[2];
    unsigned char tmp_buf[20];

    strcpy((char*)tmp_buf, "___.___.___.___");
    ScrGotoxy(x, y);
    //printf((char*)tmp_buf);
    ScrPrint(x, y, CFONT, tmp_buf);


    while (1)
    {
        ScrGotoxy((BYTE)(x + pos * 8), (BYTE) y);
        ScrAttrSet(1);
        ScrPrint((BYTE)(x + pos * 8), y, ASCII, " ");
        ScrAttrSet(0);

        ch = getkey();

        if (ch == KEYCANCEL)
        {
            return -1;
        }

        if ((ch >= KEY0) && (ch <= KEY9) && (pos < 15))
        {
            ScrGotoxy((BYTE)(x + pos * 8), y);
            disp_buf[0] = ch;
            disp_buf[1] = 0;
            //printf((char*)disp_buf);
            ScrPrint((BYTE)(x + pos * 8), y, ASCII, disp_buf);

            tmp_buf[pos++] = ch;

            if ((pos == 3) || (pos == 7) || (pos == 11))
            {
                pos++;
            }
        }
        else if ((ch == KEYENTER) && (pos == 15))
        {
            int i, l;
            BYTE tmpbuf[20];

            memset(tmpbuf, 0x00, 20);

            for(i = 0, l = 0; i < 15; i++, l++)
            {
                if ( ((i % 4) == 0) && (tmp_buf[i] == '0'))
                {
                    i++;

                    if ( ((i % 4) == 1) && (tmp_buf[i] == '0'))
                    {
                        i++;
                    }
                }
                tmpbuf[l] = tmp_buf[i];
            }

            tmpbuf[l] = 0x00;
            strcpy((char*)tmp_buf, (char*) tmpbuf);
            strcpy((char*)ip_addr, (char*) tmp_buf);
            return 0;
        }
        else if ((ch == KEYCLEAR) && (pos > 0))
        {
            ScrGotoxy((BYTE)(x + pos * 8), y);
            ScrPrint((BYTE)(x + pos * 8), y, ASCII, "_");
            //printf("_");

            pos--;

            if ((pos == 3) || (pos == 7) || (pos == 11))
            {
                pos--;
            }
        }
    }
}


//Gillian 2016-8-29 
  #ifdef PP_SSL 
#define DNS_INIT_RETRY   5
void PP_InitDnsIp(uchar ucDisplay)
{
	int iRet, retry;
	char ipBuff[100], urlBuff[100];

  if(ucDisplay == TRUE)
  {
    ScrCls();
    PubDispString("DNS RESOLVE", 1|DISP_LINE_LEFT);
    PubDispString(" PROCESSING....", 3|DISP_LINE_LEFT);
  }

  iRet = -1;
  if(glSysParam.stTxnCommCfg.szPP_URL[0] != 0)
  {
    if(ChkTerm(_TERMINAL_S90_) && glCommCfg.ucCommType ==  CT_3G)
    {
      iRet = WlPppCheck();  //HASE GENERIC BUILD107

      if(iRet != 0)
      {
        iRet =  WlPppLogin(glSysParam.stTxnCommCfg.stWirlessPara.szAPN,glSysParam.stTxnCommCfg.stWirlessPara.szUID, glSysParam.stTxnCommCfg.stWirlessPara.szPwd, 0XFF, 10000, 0);
      }
    }

    memset(urlBuff, 0, sizeof(urlBuff));
    memcpy(urlBuff, glSysParam.stTxnCommCfg.szPP_URL, URL_MAX_LEN);
    
    for(retry=0; retry<DNS_INIT_RETRY; retry++)
    {
        memset(ipBuff, 0, sizeof(ipBuff));
        iRet = DnsResolve(urlBuff, ipBuff, URL_MAX_LEN);

        if(iRet == 0 && ipBuff[0] != 0)
        {
            memset(glSysParam.stTxnCommCfg.szPP_SSL_BackupIP, 0, IP_ADDRESS_MAX_LEN);
            memcpy(glSysParam.stTxnCommCfg.szPP_SSL_BackupIP, ipBuff, strlen(ipBuff));        

            if(ChkTerm(_TERMINAL_S90_)&& glCommCfg.ucCommType ==  CT_3G)
            {
            memset(gCurComPara.LocalParam.szRemoteIP_1, 0, sizeof(gCurComPara.LocalParam.szRemoteIP_1));
            memset(gCurComPara.LocalParam.szRemoteIP_2, 0, sizeof(gCurComPara.LocalParam.szRemoteIP_2));
            memcpy(gCurComPara.LocalParam.szRemoteIP_1, ipBuff, strlen(ipBuff));
            memcpy(gCurComPara.LocalParam.szRemoteIP_2, ipBuff, strlen(ipBuff));
            }
        }
        break;
      }
    }

    if(ChkTerm(_TERMINAL_S90_) && glCommCfg.ucCommType ==  CT_3G && ucDisplay == TRUE)
    {
      WlPppLogout();
    }


    if(ucDisplay == TRUE)
    {
        ScrCls();

        if(iRet == 0 && ipBuff[0] != 0)
        {      
            PubDispString("PP SSL URL IP", 3|DISP_LINE_LEFT);
            ScrPrint(0, 2, CFONT, glSysParam.stTxnCommCfg.szPP_SSL_BackupIP);
            PubWaitKey(15);
        }
        else 
        {
            PubDispString("PP SSL URL IP", 3|DISP_LINE_LEFT);
            ScrPrint(0, 2, CFONT, glSysParam.stTxnCommCfg.szPP_SSL_BackupIP);
            PubDispString("MANUAL INPUT IP?", 4|DISP_LINE_LEFT);

            while(1)
            {
                if(getkey() == KEYENTER)
                {
                  ScrClrLine(2, 7);
                  PubDispString("ENTER URL IP", 2|DISP_LINE_LEFT);

                  memset(ipBuff, 0, sizeof(ipBuff));

                  if (get_ip_addr(0, 6, ipBuff) == 0)
                  {
                    strcpy(glSysParam.stTxnCommCfg.szPP_SSL_BackupIP, ipBuff);
                    break;
                  }
                }
                else
                {
                  break;
                }
            }
        }    
    }
}
  #endif //end PP_SSL

//******************************************************
//Function:
//init the SSL network operation & the related operation
//******************************************************
void InitSSL(void) //SSL1 SSL2 //Gillian 2016-8-29 
{
    SslSetNetOps(&s80_ssl_ops);
    SslSetSysOps(&s80_sys_ops);
}


int SxxSSLConnect(uchar *RemoteIP, uchar *RemotePort, int iTimeOut)
{
    int ret, ret2;
    int num = 0;
    ushort usLen=0;
    uchar SerCerBuf1[3000], SerCerBuf2[3000];
	uchar LocalCertBuf[3000], LocalKeyBuf[3000];
	SSL_BUF_T LocalCerts; 
	SSL_BUF_T LocalKey;
    SSL_BUF_T  ServerCert1, ServerCert2;
    SSL_BUF_T  ServerCertLst[2];
	SSL_BUF_T *pSerCertChains;
	int SerCertChainsNum = 0;	
	SSL_BUF_T *pLclCert = NULL;
	int LclCertNum = 0;
	SSL_BUF_T *pLclkey = NULL;

	//Gillian 2016-8-19
	int iRet;
    char hostCN[URL_MAX_LEN];
    TCPIP_PARA tcpPara;



	memset(SerCerBuf1, 0, sizeof(SerCerBuf1));
	memset(SerCerBuf2, 0, sizeof(SerCerBuf2));
	memset(LocalCertBuf, 0, sizeof(LocalCertBuf));
	memset(LocalKeyBuf, 0, sizeof(LocalKeyBuf));
	
    sg_Timeout = iTimeOut;
    memset(&ServerCert1, 0, sizeof(SSL_BUF_T));
    memset(&ServerCert2, 0, sizeof(SSL_BUF_T));
    memset(ServerCertLst, 0, 2*sizeof(SSL_BUF_T));
	memset(&LocalCerts, 0, sizeof(SSL_BUF_T));
	memset(&LocalKey, 0, sizeof(SSL_BUF_T));
	
    ServerCert1.size = sizeof(SerCerBuf1);
    ServerCert1.ptr = SerCerBuf1;
    ServerCert2.size = sizeof(SerCerBuf2);
    ServerCert2.ptr = SerCerBuf2;
    LocalCerts.size = sizeof(LocalCertBuf);
    LocalCerts.ptr = LocalCertBuf;
    LocalKey.size = sizeof(LocalKeyBuf);
    LocalKey.ptr = LocalKeyBuf;

    
	//Gillian 2016-8-19
	//Terminal IP info.
    memset(hostCN, 0, URL_MAX_LEN);
    memset(&tcpPara, 0, sizeof(TCPIP_PARA));

#ifdef PP_SSL
    //获取tcp相关参数
    if(SSL_PP_GetTCPInfo(&tcpPara, hostCN) != 0)
    {
       SSL_PrintError(NET_ERR_DNS);
       return ERR_COMM_TCPIP_BASE;
    }
#endif
    PubDebugTx("tcpPara->szRemoteIP_1:%s\n", tcpPara.szRemoteIP_1);
    PubDebugTx("tcpPara->szRemotePort_1:%s\n", tcpPara.szRemotePort_1);
  
	iRet = SSL_LANTcpDial(&tcpPara, hostCN);
    PubDebugTx("SSL_LANTcpDial:%d\n", iRet);
	if(iRet != 0)
    {
       if(iRet == NET_ERR_CN_NOT_MATCH)
       {
          SSL_PrintError(NET_ERR_CN_NOT_MATCH);
       }

       iRet = ERR_COMM_TCPIP_BASE;
     }
	return iRet;
}


int SxxSSLSend(char *pBuffer, int nSize, ushort sTimeOut)
{
    int iRet;
    sg_Timeout = sTimeOut;
    iRet = SslSend(sg_iSocket, pBuffer, nSize); //SSL8
    //PubDebugTx("SslSend:%d sg_iSocket:%d size:%d\n", iRet,sg_iSocket, nSize);
	/*PubDebugOutput((uchar *)_T("SSL SEND MSG"), pBuffer, nSize, DEVICE_PRN, ISO_MODE); //Gillian debug

	PubDebugOutput((uchar *)_T("SSL HEX SEND MSG"), pBuffer, nSize, DEVICE_PRN, HEX_MODE); //Gillian debug*/
    ///////////////////////////////////////////////
    if (iRet < 0)
    {
    		#if 0  
  		kbflush();
		ScrCls();
		Lcdprintf("SSL Send(1) iRet=%d", iRet);	
		PubWaitKey(40);
		#endif 
        return iRet;
    }
    if (iRet != nSize)
    {
      		#if 0  
  		kbflush();
		ScrCls();
		Lcdprintf("SSL Send(2) iRet=%d", iRet);	
		PubWaitKey(40);
		#endif   
        return -1;
    }

      		#if 0  
  		kbflush();
		ScrCls();
		Lcdprintf("SSL Send(3)   OK");	
		PubWaitKey(40);
		#endif   
    return 0;
}


int SxxSSLRecv(char *pBuffer, int nSize, ushort sTimeOut, ushort *sOutLen)
{
    int ret;
    int offset = 0;

    sg_Timeout = sTimeOut;
    ret = SslRecv(sg_iSocket, pBuffer, nSize); //SSL9
    //PubDebugTx("SslRecv:%d sg_iSocket:%d size:%d\n", ret,sg_iSocket, nSize);

    
//  //ttttttttttttttt
//  ScrCls();
//  getkey();
//  PortOpen(0, "115200,8,N,1");
//  for (ret=0;ret<=nSize;ret++)
//  {
//      PortSend(0, *(pBuffer+ret));
//  }
////tttttttttttttttttttttttttttt
    if( ret<0 )
    {
		#if 0  
  		kbflush();
		ScrCls();
		Lcdprintf("SSL Recv(1)  ret=%d", ret);	
		PubWaitKey(40);
		#endif     
        return ret;
    }

    if (sOutLen != NULL)
    {
        *sOutLen = ret;
    }

		#if 0  
  		kbflush();
		ScrCls();
		Lcdprintf("SSL Recv(2)   OK");	
		PubWaitKey(40);
		#endif  	
    return 0;

}


int SxxSSLOnHook(void)
{
    int ret;

    ret = SslClose(sg_iSocket); //SSL10
    if (ret<0)
    {
		#if 0  
  		kbflush();
		ScrCls();
		Lcdprintf("SSL OnHook(1) ret=%d, sg_iSocket=%d", ret, sg_iSocket);	
		PubWaitKey(40);
		#endif 
        return ret;
    }
	
    return 0;
}

// Sxx TCP connection
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpConnect(char *pszIP, short sPort, int iTimeoutSec)
{
	int		iRet;
	struct net_sockaddr stServer_addr;

#ifdef APP_DEBUG_RICHARD
    PubDebugTx("SxxTcpConnect:%s\n", pszIP);
    PubDebugTx("ucEnableSSL_URL:%d\n", glSysParam.stTxnCommCfg.ucEnableSSL_URL);
#endif

//	if(ChkEdcOption(EDC_PP_SSL_ENABLE))    //Gillian 2016-8-18
    WlTcpRetryNum(1); //add by richard 20170120, set the retry nem to 0 ,decrease the timeout time

	if(glSysParam.stTxnCommCfg.ucEnableSSL_URL )//&& ((!ChkIfAmex()) || (!ChkIfDiners())))
	{		
		uchar szPort[5] = "";
		
		#if 0  
  		kbflush();
		ScrCls();
		Lcdprintf("SSL Connect");	
		PubWaitKey(40);
		#endif 
		//gCurComPara.stComm.ucCommType = SSL_COMM;
		sprintf(szPort, "%d", sPort);
		return SxxSSLConnect(pszIP, szPort, iTimeoutSec);//SSL6
	}
	else		
	{
		// Setup socket
		iRet = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);
        PubDebugTx("NetSocket:%d\n", iRet);
		if (iRet!=0)
		{
			return iRet;	
		}
		sg_iSocket = iRet;

		// Bind IP
		iRet = SockAddrSet(&stServer_addr, pszIP, sPort);
        PubDebugTx("SockAddrSet:%d %s %d\n", iRet, pszIP, sPort);
		if (iRet!=0)
		{
			return iRet;
		}

		// set connection timeout
		if (iTimeoutSec<3)
		{
			iTimeoutSec = 3;
		}
		iRet = Netioctl(sg_iSocket, CMD_TO_SET, iTimeoutSec*1000);
		if (iRet!=0)
		{
			return iRet;
		}

		// Connect to remote IP
		PubDebugTx("Before NetConnect!\n");
		iRet = NetConnect(sg_iSocket, &stServer_addr, sizeof(stServer_addr));
        PubDebugTx("After NetConnect:%d\n", iRet);
		if (iRet!=0)
		{
			NetCloseSocket(sg_iSocket);
		}

		return iRet;
	}
}

//Sxx TCP/IP send data
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpTxd(uchar *psTxdData, ushort uiDataLen, ushort uiTimeOutSec)
{
	int iRet;
	int iSendLen;
	int iSumLen;

	//if(ChkEdcOption(EDC_PP_SSL_ENABLE))   //Gillian 2016-8-18
	if(glSysParam.stTxnCommCfg.ucEnableSSL_URL)//  && ((!ChkIfAmex()) || (!ChkIfDiners())))
	{
		#if 0  
  		kbflush();
		ScrCls();
		Lcdprintf("SSL Send");	
		PubWaitKey(40);
		#endif 
	
		return SxxSSLSend(psTxdData, uiDataLen, uiTimeOutSec);//ttttttttttttttttttt //SSL8
	}
	else
	{
		iRet = Netioctl(sg_iSocket, CMD_TO_SET, uiTimeOutSec*1000);
		if (iRet < 0)
		{
			return iRet;
		}

		iSumLen = 0;
		while(1)
		{
			if (uiDataLen > TCPMAXSENDLEN)
			{
				iSendLen = TCPMAXSENDLEN;
				uiDataLen = uiDataLen - TCPMAXSENDLEN;
			}
			else
			{
				iSendLen = uiDataLen;
			}	
			iRet = NetSend(sg_iSocket, psTxdData+iSumLen, iSendLen, 0);
			if (iRet < 0)
			{
				return iRet;
			}
			if (iRet != iSendLen)
			{
				return -1;
			}
			iSumLen = iSumLen + iSendLen;
			if (iSendLen <= TCPMAXSENDLEN)
			{
				break;
			}	
		}
		return 0;
	}
}

//Sxx TCP/IP receive
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, ushort *puiOutLen)
{
	int iRet;

	DelayMs(200);

	//Gillian 2016-8-18
	//if(ChkEdcOption(EDC_PP_SSL_ENABLE))
	if(glSysParam.stTxnCommCfg.ucEnableSSL_URL)//  && ((!ChkIfAmex()) || (!ChkIfDiners())))
	{
		#if 0  
  		kbflush();
		ScrCls();
		Lcdprintf("SSL Rcv");	
		PubWaitKey(40);
		#endif 
	
		return SxxSSLRecv(psRxdData, LEN_MAX_COMM_DATA, uiTimeOutSec, puiOutLen);//SSL9
	}
	else
	{
		iRet = 	Netioctl(sg_iSocket, CMD_TO_SET, uiTimeOutSec*1000);
		if (iRet < 0)
		{
			return iRet;
		}

		iRet = NetRecv(sg_iSocket, psRxdData, uiExpLen, 0);
		if (iRet < 0)
		{
			return iRet;
		}
		*puiOutLen = iRet;

		return 0;
	}
}

//Sxx TCP/IP close socket
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpOnHook(void)
{
	int iRet;

    if(glSysParam.stTxnCommCfg.ucEnableSSL_URL && !ChkIfAmex() && !ChkIfDiners())
    {
        SxxSSLOnHook();
    }
    else 
    {
        iRet = NetCloseSocket(sg_iSocket);
        if (iRet < 0)
        {
        	return iRet;
        }
    }
	return 0;
}

uchar SocketCheck(int sk)
{
	int event;
	if(sk<0) return RET_TCPCLOSED;
	
	event = Netioctl(sk, CMD_EVENT_GET, 0);
	if(event<0)
	{
         NetCloseSocket(sk);
         return RET_TCPCLOSED;
	}	
	
	if(event&(SOCK_EVENT_CONN|SOCK_EVENT_WRITE|SOCK_EVENT_READ))
	{
         return RET_TCPOPENED;
	}
	else if(event&(SOCK_EVENT_ERROR))
	{
         NetCloseSocket(sk);
         return RET_TCPCLOSED;
	}

	return RET_TCPOPENING;
}

#endif	 // SXX_WIRELESS_MODULE || SXX_IP_MODULE

#ifdef SXX_IP_MODULE

int SxxDhcpStart(uchar ucForceStart, uchar ucTimeOutSec)
{
	int	iRet;

	if (ucForceStart && (DhcpCheck()==0))
	{
		DhcpStop();
	}

	if (ucForceStart || (DhcpCheck()!=0))
	{
		iRet = DhcpStart();
		if (iRet < 0)
		{
			return iRet;
		}

		TimerSet(TIMER_TEMPORARY, (ushort)(ucTimeOutSec*10));
		while (TimerCheck(TIMER_TEMPORARY)!=0)
		{
			DelayMs(200);
			iRet = DhcpCheck();
			if (iRet==0)
			{
				return 0;
			}
		}

		return iRet;
	}

	return 0;
}

int SxxLANTcpDial(TCPIP_PARA *pstTcpPara)
{
	int		iRet;
	uchar	ucRedoDhcp, ucSecondIP;

	CommOnHook(FALSE);

	if (pstTcpPara->ucDhcp)
	{
		iRet = SxxDhcpStart(FALSE, 30);
		if (iRet!=0)
		{	
			return iRet;
		}
	}

	ucRedoDhcp = FALSE;
	ucSecondIP = FALSE;

TAG_RETRY_IP:

	// Connect to remote IP
	if (ucSecondIP)
	{	
		iRet = SxxTcpConnect(pstTcpPara->szRemoteIP_2, (short)atoi(pstTcpPara->szRemotePort_2), 8);
	}
	else
	{
		iRet = SxxTcpConnect(pstTcpPara->szRemoteIP_1, (short)atoi(pstTcpPara->szRemotePort_1), 8);
	}

	if (iRet!=0)
	{
		if (!ucSecondIP)
		{
			if (pstTcpPara->ucDhcp && !ucRedoDhcp)
			{
				// If fail, suspect the DHCP
				iRet = SxxDhcpStart(FALSE, 10);
				if (iRet!=0)
				{
					return iRet;
				}
				ucRedoDhcp = TRUE;
				goto TAG_RETRY_IP;
			}

			if (ChkIfValidIp(pstTcpPara->szRemoteIP_2) && ChkIfValidPort(pstTcpPara->szRemotePort_2) &&
				(strcmp((char *)(pstTcpPara->szRemoteIP_1), (char *)(pstTcpPara->szRemoteIP_2))!=0))
			{
				ucSecondIP = TRUE;
				goto TAG_RETRY_IP;
			}
		}
	}

	return iRet;
}

#endif	// SXX_IP_MODULE

#ifdef SXX_WIRELESS_MODULE

//初始化无线模块
int SXXWlInit(WIRELESS_PARAM *pstWlPara)
{
	int iRet;

	iRet = WlInit(pstWlPara->szSimPin);
	if (iRet == -212)	//模块已初始化
		iRet = 0;

	SXXWlDispSignal();

	return iRet;
}

//先判断PPP链路是否联通，若没有联通先进行PPP链接，成功后再进行TCP连接；若已联通则直接进行TCP连接
int SXXWlDial(WIRELESS_PARAM *pstWlPara, int iTimeOut, int iAliveInterval, uchar ucPredial)
{
	int		iRet;
	int		iRetryTime;
	uchar	ucSecondIP;

	SXXWlDispSignal();
	
	if (iTimeOut<1)
	{
		iTimeOut = 1;
	}

	// ********** Pre-dial **********
	if (ucPredial)
	{
		iRet = WlPppCheck();
		if ((iRet==0) || (iRet==WL_RET_ERR_DIALING) || (iRet==1))	// ret=1 means module busy
		{
			return 0;
		}

		iRet = WlPppLogin(pstWlPara->szAPN, pstWlPara->szUID, pstWlPara->szPwd, 0xFF, 0, iAliveInterval);
		return 0;
	}

	// ********** Full-dial **********

	// ********** Check PPP connection **********
	TimerSet(TIMER_TEMPORARY, (ushort)(10*10));
	while (TimerCheck(TIMER_TEMPORARY)!=0)
	{
		iRet = WlPppCheck();

		if (iRet == 0)
		{
			goto TCPCONNECT;
		}
	}

	// ********** Take PPP dial action **********
	iRetryTime = 3;
	while(iRetryTime--)
	{
		iRet = WlPppLogin(pstWlPara->szAPN, pstWlPara->szUID, pstWlPara->szPwd, 0xFF, iTimeOut*1000, iAliveInterval);
		if (iRet != 0)
		{
			DelayMs(100);
			continue;
		}

		iRet = WlPppCheck();
		if (iRet == 0)
		{
			break;
		}
	}

	if (iRetryTime <= 0 && iRet != 0)
	{
		return iRet;
	}

	// ********** Connect IP **********
TCPCONNECT:	
	iRet = SocketCheck(sg_iSocket);  //come from R&D, tom
//	ScrPrint(0, 7, ASCII, "tang[SocketCheck(%i)]",iRet); DelayMs(1000);
	if (iRet == RET_TCPOPENED)
	{
		return 0;
	}

	ucSecondIP = FALSE;

_RETRY_SECOND_IP:
	if (ucSecondIP)
	{
		iRet = SxxTcpConnect(pstWlPara->szRemoteIP_2, (short)atoi(pstWlPara->szRemotePort_2), 8);
	} 
	else
	{
		iRet = SxxTcpConnect(pstWlPara->szRemoteIP_1, (short)atoi(pstWlPara->szRemotePort_1), 8);
	}

	if (iRet < 0)
	{
		if (ChkIfValidIp(pstWlPara->szRemoteIP_2) && ChkIfValidPort(pstWlPara->szRemotePort_2) &&
			(strcmp((char *)(pstWlPara->szRemoteIP_1), (char *)(pstWlPara->szRemoteIP_2))!=0)
			&& !ucSecondIP) //add by richard 20170120
		{
			ucSecondIP = TRUE;
			goto _RETRY_SECOND_IP;
		}

		return iRet;
	}

	return 0;
}

//发送数据
int SXXWlSend(uchar *psTxdData, ushort usDataLen, ushort uiTimeOutSec)
{
	return SxxTcpTxd(psTxdData, usDataLen, uiTimeOutSec);
}

//接收数据
int SXXWlRecv(uchar *psRxdData, ushort usExpLen, ushort uiTimeOutSec, ushort *pusOutLen)
{
	DelayMs(200);
	return SxxTcpRxd(psRxdData, usExpLen, uiTimeOutSec, pusOutLen);
}

//关闭TCP连接
int SXXWlCloseTcp(void)
{
	int iRet;

	iRet = NetCloseSocket(sg_iSocket);
	if (iRet < 0)
	{
		return iRet;
	}
	return 0;
}

//关闭PPP链路
void SXXWlClosePPP(void)
{
	WlPppLogout(); 
	return;
}

//显示信号强度
void SXXWlDispSignal(void)
{
	uchar	ucRet, ucLevel;
	
	
	ucRet = WlGetSignal(&ucLevel);
	if( ucRet!=RET_OK )
	{
		ScrSetIcon(ICON_SIGNAL, CLOSEICON);
		return;
	}
	
	ScrSetIcon(ICON_SIGNAL, (uchar)(5-ucLevel));
}

#endif	// SXX_WIRELESS_MODULE

