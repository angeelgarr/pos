
#include "global.h"

/********************** Internal macros declaration ************************/
#define DOWNPARA_FILE	"SYS_DOWN_EDC"

/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static void RemoveEmvAppCapk(void);
static void NoDownloadInit(void);
static int  SelectDownloadMode(uchar *pucCommType);
static int  OldTmsDownLoad(void);
static int  UnpackPara(uchar *psParaData, long lDataLen);
static void UnpackParaEdc(uchar *psPara);
static void TransformCommMode(uchar *psInTmsStr, uchar *psOutAcqComm);
static void TransformIP(uchar * ip_in, uchar * ip_out);
static void TransformPort(uchar * port_in, uchar * port_out);
static int  UnpackParaAcq(uchar ucIndex, uchar *psPara, ulong ulSubFieldLen);
static int  UnpackParaIssuer(uchar ucIndex, uchar *psPara);
static int  UnpackParaCard(uchar ucIndex, uchar *psPara);
static int  UnpackEPSPara(uchar *psPara);
static int  UnpackInstPara(uchar *psPara);
static int  UnpackDescPara(uchar *psPara);
static uchar SearchIssuerKeyArray(void);
static void AfterLoadParaProc(void);
static void  SearchIssuerKeyArraySub(uchar *sIssuerKey, uchar ucAcqKey);
static int  GetDownLoadTelNo(void);
static int  GetDownLoadGprsPara(void);
static int  GetDownLoadLanPara(void);
static int  GetDownLoadWIFIPara(void);
static int  GetDownLoadComm(uchar ucCommType);
static int  GetDownLoadTID(uchar *pszID);
static int  SaveEmvMisc(uchar *psPara);
static int  SaveEmvApp(uchar *psPara);
static int  SaveEmvCapk(uchar *psPara);
static void GetNextAutoDayTime(uchar *pszCurDateTime, ushort uiInterval);
static int  SaveDetail(uchar *psData);
static int  SaveCardBin(uchar *psCardBin);

void InitEdcParam(void);

/********************** Internal variables declaration *********************/
static uchar	sgSyncDial, sgNewTMS;
static uchar	sgTempBuf[1024*20];
static uchar	sgEMVDownloaded;

/********************** external reference declaration *********************/
// monitor return value "loaddef.h"
// #define SUCCESS						0
// #define NO_TERMINAL					1
// #define LOAD_FORBAD					2
// #define TERMINAL_FROZEN				3
// #define TERMINAL_FAULT				4
// #define COMM_TIMEOUT					5
// #define AUTH_FAILED					6
// ......
// #define NO_TASK_LIST				1012
// ......
// monitor下载应用程序和文件

#if defined(_P60_S1_) || defined(_P70_S_)
int RemoteLoadApp(T_INCOMMPARA *ptCommPara);
#endif
//extern int GetLoadedAppStatus(uchar *psAppName, TMS_LOADSTATUS *ptStat);	// 检查刚刚下载的文件是否成功。

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

static void GetDefCurrency(CURRENCY_CONFIG *pstConfig)
{
#ifdef AREA_TAIWAN
	*pstConfig = glCurrency[12];
#elif defined(AREA_HK)
	*pstConfig = glCurrency[0];
#elif defined(AREA_SG)
	*pstConfig = glCurrency[10];
#else
	*pstConfig = glCurrency[16];
#endif
}

// 设置EDC缺省参数
// Set to default EDC parameter
void LoadEdcDefault(void)
{
	int			iCnt;

	ResetAllPara(TRUE);

	memset(&glSysCtrl, 0, sizeof(SYS_CONTROL));
#ifdef SUPPORT_TABBATCH
	glSysCtrl.ucLastTransIsAuth = FALSE;
#endif
        glSysCtrl.uiRePrnRecNo = 0xFFFF;  //build88S

	glSysCtrl.ulInvoiceNo = 1L;
	glSysCtrl.ulSTAN      = 1L;
	glSysCtrl.uiLastRecNo = 0xFFFF;
	glSysCtrl.uiErrLogNo  = 0;
	for(iCnt=0; iCnt<MAX_ACQ; iCnt++)
	{
		glSysCtrl.sAcqStatus[iCnt] = S_RESET;
		glSysCtrl.stField56[iCnt].uiLength = 0;
		glSysCtrl.uiLastRecNoList[iCnt] = 0xFFFF;
	}

	for(iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
	{
		glSysCtrl.sAcqKeyList[iCnt]    = INV_ACQ_KEY;		// set to invalid acquirer key
		glSysCtrl.sIssuerKeyList[iCnt] = INV_ISSUER_KEY;	// set to invalid issuer key
	}
	glSysCtrl.stWriteInfo.bNeedSave = SAVE_NONEED;


    // for Tab Batch
#ifdef SUPPORT_TABBATCH
	for(iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
	{
		glSysCtrl.astAuthRecProf[iCnt].ucAcqKey = INV_ACQ_KEY;		// set to invalid acquirer key
        memset(glSysCtrl.astAuthRecProf[iCnt].sDateBCD, 0, LEN_DATE_BCD);
	}
	glSysCtrl.stWriteInfo_TabBatch.bNeedSave = SAVE_NONEED;
#endif

	SaveSysCtrlAll();

#ifdef ENABLE_EMV
	memset(&glEmvStatus, 0, sizeof(EMV_STATUS));
	SaveEmvStatus();
#endif
}

#ifdef ENABLE_EMV
void LoadEmvDefault(void)
{
#ifdef EMV_TEST_VERSION
	int	iRet;
#endif
	CURRENCY_CONFIG		stLocalCurrency;

	GetDefCurrency(&stLocalCurrency);
	RemoveEmvAppCapk();

	memset(&glEmvStatus, 0, sizeof(EMV_STATUS));
	SaveEmvStatus();
	
	// set default EMV library parameters
	EMVGetParameter(&glEmvParam);
	memcpy(glEmvParam.Capability,    EMV_CAPABILITY, 3);
	memcpy(glEmvParam.ExCapability,  "\xE0\x00\xF0\xA0\x01", 5);
	// 这里先设默认货币。参数下载之后根据protims设定的货币来修改EMV库参数
	memcpy(glEmvParam.CountryCode, stLocalCurrency.sCountryCode, 2);
	memcpy(glEmvParam.TransCurrCode, stLocalCurrency.sCurrencyCode, 2);
	memcpy(glEmvParam.ReferCurrCode, stLocalCurrency.sCurrencyCode, 2);
	memcpy(glEmvParam.MerchCateCode, "\x00\x00", 2);
//	glEmvParam.ReferCurrCon  = 0;
	glEmvParam.TransCurrExp  = stLocalCurrency.ucDecimal;
	glEmvParam.ReferCurrExp  = stLocalCurrency.ucDecimal;
	glEmvParam.ForceOnline   = 0;
	glEmvParam.GetDataPIN    = 1;
	glEmvParam.SurportPSESel = 1;
	EMVSetParameter(&glEmvParam);
	
#ifdef EMV_TEST_VERSION
	InitTestApps();
	InitTestKeys();
	if (glSysParam.stEdcInfo.ucClssFlag == 1)
	{
		iRet = InitClssParam();
		if (iRet)
		{
			ProcError_Wave(iRet);
		}
	}
#endif
	InitLiveApps();
	InitLiveKeys();
}
#endif

void LoadDefCommPara(void)
{
	uchar	sBuff[HWCFG_END+1];

	// ================================ 交易参数 ================================
	// 设置回调函数
	// Setup callback function used in waiting response display
	glSysParam.stTxnCommCfg.pfUpdWaitUI   = DispWaitRspStatus;

	// TMS comm type
	glSysParam.stTMSCommCfg.ucCommType    = CT_NONE;	// will not be set until fun0
	glSysParam.stTMSCommCfg.ucCommTypeBak = CT_NONE;

	// TXN default comm type
#if defined(_P60_S1_) || defined(_P70_S_)
	glSysParam.stTxnCommCfg.ucCommType = CT_MODEM;
#else
	GetTermInfo(sBuff);
	if ((sBuff[HWCFG_MODEL]==_TERMINAL_S90_) || (sBuff[HWCFG_MODEL]==_TERMINAL_P90_))
	{
		if (sBuff[HWCFG_GPRS]!=0)
		{
			glSysParam.stTxnCommCfg.ucCommType = CT_GPRS;
		}
		else if (sBuff[HWCFG_CDMA]!=0)
		{
			glSysParam.stTxnCommCfg.ucCommType = CT_CDMA;
		}
		else if(sBuff[HWCFG_WCDMA]!=0)
		{
			glSysParam.stTxnCommCfg.ucCommType = CT_3G;//2015-11-23
		}
	}
	else if (sBuff[HWCFG_MODEM]!=0)
	{
		glSysParam.stTxnCommCfg.ucCommType = CT_MODEM;
	}
	else
	{
		glSysParam.stTxnCommCfg.ucCommType = CT_RS232;
	}
#endif
	glSysParam.stTxnCommCfg.ucCommTypeBak = CT_NONE;

	// TCP length header format.
	glSysParam.stTMSCommCfg.ucTCPClass_BCDHeader = TRUE;
	glSysParam.stTxnCommCfg.ucTCPClass_BCDHeader = TRUE;

	// 交易拨号缺省参数
	// Default dial parameter in transaction
	glSysParam._TxnPSTNPara.ucSendMode = CM_SYNC;
    glSysParam._TxnPSTNPara.ucSignalLevel = 0;
	glSysParam._TxnModemPara.DP      = 0;
	glSysParam._TxnModemPara.CHDT    = 0;
	glSysParam._TxnModemPara.DT1     = 20;		// 等候拨号音的最长时间(20~~255), 单位: 100ms
	glSysParam._TxnModemPara.DT2     = 10;		// 拨外线时","等待时间(0~~255), 单位: 100ms
	glSysParam._TxnModemPara.HT      = 70;		// 双音拨号单一号码保持时间(单位:1ms,有效范围50~255)
	glSysParam._TxnModemPara.WT      = 5;		// 双音拨号两个号码之间的间隔时间(单位:10ms,有效范围5~25)
	glSysParam._TxnModemPara.SSETUP  = 0x05;	// 0x45: 9600bps, 0x05:1200 bps	// 通讯字节
	glSysParam._TxnModemPara.DTIMES  = 1;		// 循环拨号总次数,拨完拨号串的所有号码为一次[有效范围1~255]
	glSysParam._TxnModemPara.TimeOut = 6;		// 没有数据交换MODEM挂断等待时间;以10秒为单位,为0时无超时,最大值65
	glSysParam._TxnModemPara.AsMode  = 0;		// 异步通讯的速率
	glSysParam.stTxnCommCfg.ucEnableSSL_URL = 0;
	// RS232缺省参数
	// Default parameter in transaction for RS232
#ifdef _WIN32
#if defined(_S_SERIES_) || defined(_P58_)
	glSysParam._TxnRS232Para.ucPortNo   = PINPAD;
#else
	glSysParam._TxnRS232Para.ucPortNo   = COM2;
#endif
#else
	glSysParam._TxnRS232Para.ucPortNo   = COM1;
#endif
	glSysParam._TxnRS232Para.ucSendMode = CM_SYNC;
	sprintf((char *)glSysParam._TxnRS232Para.szAttr, "9600,8,n,1");

	// TCP/IP缺省参数
	// Default parameter in transaction for TCPIP
#if defined(_P60_S1_)
	glSysParam._TxnTcpIpPara.ucPortNo = COM1;
#elif defined(_P70_S_)
	glSysParam._TxnTcpIpPara.ucPortNo = COM2;
#elif defined(_P80_)
	glSysParam._TxnTcpIpPara.ucPortNo = LANPORT;
#endif
	glSysParam._TxnTcpIpPara.ucDhcp = 1;
	sprintf((char *)glSysParam._TxnTcpIpPara.szNetMask, "");
	sprintf((char *)glSysParam._TxnTcpIpPara.szGatewayIP, "");
	sprintf((char *)glSysParam._TxnTcpIpPara.szLocalIP,  "");
	sprintf((char *)glSysParam._TxnTcpIpPara.szRemoteIP_1,  "");
	sprintf((char *)glSysParam._TxnTcpIpPara.szRemotePort_1, "");
	sprintf((char *)glSysParam._TxnTcpIpPara.szRemoteIP_2,  "");
	sprintf((char *)glSysParam._TxnTcpIpPara.szRemotePort_2, "");
	sprintf((char *)glSysParam._TxnTcpIpPara.szDNSIP, "");

	// GPRS/CDMA缺省参数
	// GPRS/CDMA para in TMS download
	sprintf((char *)glSysParam._TxnWirlessPara.szAPN, "");
	sprintf((char *)glSysParam._TxnWirlessPara.szUID, "");
	sprintf((char *)glSysParam._TxnWirlessPara.szPwd, "");
	sprintf((char *)glSysParam._TxnWirlessPara.szSimPin, "");
	sprintf((char *)glSysParam._TxnWirlessPara.szDNS, "");
	sprintf((char *)glSysParam._TxnWirlessPara.szRemoteIP_1,   "");
	sprintf((char *)glSysParam._TxnWirlessPara.szRemotePort_1, "");
	sprintf((char *)glSysParam._TxnWirlessPara.szRemoteIP_2,   "");
	sprintf((char *)glSysParam._TxnWirlessPara.szRemotePort_2, "");


	// ================================ 下载参数 ================================
	// 设置回调函数
	// Setup callback function in TMS download
	glSysParam.stTMSCommCfg.pfUpdWaitUI = DispWaitRspStatus;
	glSysParam.stTMSCommCfg.ucCommType  = CT_MODEM;

	// 参数下载缺省参数
	// Default dial parameter in TMS download
#if defined(_P60_S1_) || defined(_P70_S_)
	glSysParam._TmsModemPara.DP      = 0;
	glSysParam._TmsModemPara.CHDT    = 0;
	glSysParam._TmsModemPara.DT1     = 5;
	glSysParam._TmsModemPara.DT2     = 7;
	glSysParam._TmsModemPara.HT      = 70;
	glSysParam._TmsModemPara.WT      = 5;
	glSysParam._TmsModemPara.SSETUP  = 0xA7;	/* asynchronise link */
	glSysParam._TmsModemPara.DTIMES  = 1;
	glSysParam._TmsModemPara.TimeOut = 6;
	glSysParam._TmsModemPara.AsMode  = 0;
#else
	glSysParam._TmsModemPara.DP      = 0;
	glSysParam._TmsModemPara.CHDT    = 0x40;
	glSysParam._TmsModemPara.DT1     = 5;
	glSysParam._TmsModemPara.DT2     = 7;
	glSysParam._TmsModemPara.HT      = 70;
	glSysParam._TmsModemPara.WT      = 5;
	glSysParam._TmsModemPara.SSETUP  = 0x87;	/* asynchronise link */
	glSysParam._TmsModemPara.DTIMES  = 1;
	glSysParam._TmsModemPara.TimeOut = 6;
	glSysParam._TmsModemPara.AsMode  = 0xF0;
#endif

	// RS232缺省参数(TMS)
	// RS232 para in TMS download
	memcpy(&glSysParam._TmsRS232Para, &glSysParam._TxnRS232Para, sizeof(glSysParam._TmsRS232Para));

	// TCP/IP缺省参数
	// TCP/IP para in TMS download
	memcpy(&glSysParam._TmsTcpIpPara, &glSysParam._TxnTcpIpPara, sizeof(glSysParam._TmsTcpIpPara));

	// GPRS/CDMA缺省参数
	// GPRS/CDMA para in TMS download
	memcpy(&glSysParam._TmsWirlessPara, &glSysParam._TxnWirlessPara, sizeof(glSysParam._TmsWirlessPara));
}

void ResetAllPara(uchar bFirstTime)
{
	int				iCnt;
	uchar			ucNewTmsBak, ucTMSSyncDial;
	uchar			szDownTelNo[25+1], szDownLoadTID[8+1], szPabx[10+1];	// 下载参数终端号
	IP_ADDR			stTmsIP;
	uchar			sEdcExtOptions[sizeof(glSysParam.stEdcInfo.sExtOption)];
	uchar			ucCommType, ucCommTypeBak, ucIdleMin, ucIdleOpt;
	TCPIP_PARA		stBakTmsTcpip, stBakTxnTcpip;
	WIRELESS_PARAM	stBakTmsWireless, stBakTxnWireless;
	// and WIFI, ...
	LANG_CONFIG		stLangBak;

	// Backup
	if( !bFirstTime )
	{
		ucNewTmsBak   = glSysParam.ucNewTMS;
		ucTMSSyncDial = glSysParam.ucTMSSyncDial;
		sprintf((char *)szDownTelNo,   "%.25s", glSysParam.stEdcInfo.szDownTelNo);
		sprintf((char *)szDownLoadTID, "%.8s",  glSysParam.stEdcInfo.szDownLoadTID);
		memcpy(&stTmsIP, &glSysParam.stEdcInfo.stDownIpAddr, sizeof(IP_ADDR));
		memcpy(sEdcExtOptions, glSysParam.stEdcInfo.sExtOption, sizeof(sEdcExtOptions));

		ucCommType    = glSysParam.stTxnCommCfg.ucCommType;
		ucCommTypeBak = glSysParam.stTxnCommCfg.ucCommTypeBak;
		memcpy(&stBakTmsTcpip, &glSysParam._TmsTcpIpPara, sizeof(TCPIP_PARA));
		memcpy(&stBakTxnTcpip, &glSysParam._TxnTcpIpPara, sizeof(TCPIP_PARA));
		memcpy(&stBakTmsWireless, &glSysParam._TmsWirlessPara, sizeof(WIRELESS_PARAM));
		memcpy(&stBakTxnWireless, &glSysParam._TxnWirlessPara, sizeof(WIRELESS_PARAM));
		memcpy(szPabx, glSysParam.stEdcInfo.szPabx, sizeof(szPabx));

		stLangBak = glSysParam.stEdcInfo.stLangCfg;
		ucIdleMin = glSysParam.stEdcInfo.ucIdleMinute;
		ucIdleOpt = glSysParam.stEdcInfo.ucIdleShutdown;
	}

	memset(&glSysParam, 0, sizeof(SYS_PARAM));

	LoadDefCommPara();
	if (bFirstTime)
	{
		LoadDefaultLang();
	}
	
	glSysParam.ucTermStatus              = INIT_MODE;
	glSysParam.stEdcInfo.bPreDial        = TRUE;
	glSysParam.stEdcInfo.ucScrGray       = 4;
	glSysParam.stEdcInfo.ucAcceptTimeout = 3;
	glSysParam.stEdcInfo.ucTMSTimeOut    = 60;
	glSysParam.stEdcInfo.ucIdleMinute    = 1;
	glSysParam.stEdcInfo.ucIdleShutdown  = 0;
	glSysParam.stEdcInfo.ulClssMaxLmt	 = 300000;
	glSysParam.stEdcInfo.ulClssOffFLmt   = 50000; 
	glSysParam.stEdcInfo.ulClssSigFlmt	 = 20000;
	glSysParam.stEdcInfo.ulODCVClssFLmt = 100000000;//2015-10-14
	//2015-10-14 add foreign cvm limit ============= START
	glSysParam.stEdcInfo.ulForeignOFFLmt = 0;//2015-11-12
	glSysParam.stEdcInfo.ulForeignCVMLmt = 0/*30000*/;
	//2015-10-14 add foreign cvm limit ============= END

	glSysParam.stEdcInfo.ulNoSignLmtVisa = 0;
	glSysParam.stEdcInfo.ulNoSignLmtMaster = 0;
	glSysParam.stEdcInfo.ulNoSignLmtAmex = 0;
	sprintf((char *)glSysParam.stEdcInfo.szTMSNii, "000");
	glSysParam.stEdcInfo.ucCasinoMode    = FALSE;
	sprintf((char *)glSysParam.stEdcInfo.szExchgeRate, "1.032");

	SetOptionExt(glSysParam.stEdcInfo.sOption, EDC_PRINT_LOGO_BMP, OPT_SET);

	glSysParam.stEdcInfo.ucPrnStlTotalFlag = 0;//2014-11-4
	glSysParam.stEdcInfo.ucPANmaskFlag = 1;//2014-11-11 defalut ecr pan masking
	glSysParam.stEdcInfo.ucEnableAuthTabBatch = 1;//2014-11-26 default settle delete auth/preauth data
	//2014-4-24 first time, default kbsound == 1
	//ScrCls();
	//ScrPrint(0,0,0,"first run !");
	//PubWaitKey(7);
	glSysParam.ucKbSound = TRUE;
	kbmute(glSysParam.ucKbSound);

	ResetPwdAll();

	// Recover
	if( !bFirstTime )
	{
		glSysParam.ucNewTMS      = ucNewTmsBak;
		glSysParam.ucTMSSyncDial = ucTMSSyncDial;
		sprintf((char *)glSysParam.stEdcInfo.szDownTelNo,   "%.25s", szDownTelNo);
		sprintf((char *)glSysParam.stEdcInfo.szDownLoadTID, "%.8s",  szDownLoadTID);
		memcpy(&glSysParam.stEdcInfo.stDownIpAddr, &stTmsIP, sizeof(IP_ADDR));
		memcpy(glSysParam.stEdcInfo.sExtOption, sEdcExtOptions, sizeof(glSysParam.stEdcInfo.sExtOption));
		
		glSysParam.stTxnCommCfg.ucCommType    = ucCommType;
		glSysParam.stTxnCommCfg.ucCommTypeBak = ucCommTypeBak;

		memcpy(&glSysParam._TmsTcpIpPara, &stBakTmsTcpip, sizeof(TCPIP_PARA));
		memcpy(&glSysParam._TxnTcpIpPara, &stBakTxnTcpip, sizeof(TCPIP_PARA));

		memcpy(&glSysParam._TmsWirlessPara, &stBakTmsWireless, sizeof(WIRELESS_PARAM));
		memcpy(&glSysParam._TxnWirlessPara, &stBakTxnWireless, sizeof(WIRELESS_PARAM));

		memcpy(glSysParam.stEdcInfo.szPabx, szPabx, sizeof(glSysParam.stEdcInfo.szPabx));

		glSysParam.stEdcInfo.stLangCfg = stLangBak;
		glSysParam.stEdcInfo.ucIdleMinute = ucIdleMin;
		glSysParam.stEdcInfo.ucIdleShutdown = ucIdleOpt;
	}

	glSysParam.stEdcInfo.ucAutoMode     = 0;	// Don't auto update parameter
	glSysParam.stEdcInfo.uiAutoInterval = 90;
	GetNextAutoDayTime(glSysParam.stEdcInfo.szAutoDayTime, glSysParam.stEdcInfo.uiAutoInterval);

	for(iCnt=0; iCnt<MAX_ACQ; iCnt++)
	{
		glSysParam.stAcqList[iCnt].ucKey = INV_ACQ_KEY;
	}
	for(iCnt=0; iCnt<MAX_ISSUER; iCnt++)
	{
		glSysParam.stIssuerList[iCnt].ucKey = INV_ISSUER_KEY;
	}

	UpdateTermInfo();
	if (ChkTerm(_TERMINAL_S60_) || ChkTerm(_TERMINAL_S80_) ||
		ChkTerm(_TERMINAL_S90_) || ChkTerm(_TERMINAL_SP30_))
	{
		glSysParam.stEdcInfo.ucPedMode = PED_INT_PCI;
	}
	else if (ChkTerm(_TERMINAL_P60_S1_) || ChkTerm(_TERMINAL_P90_))
	{
		glSysParam.stEdcInfo.ucPedMode = PED_INT_PXX;
	}
	else
	{
		glSysParam.stEdcInfo.ucPedMode = PED_EXT_PP;
	}

	InitMultiAppInfo();

	SaveSysParam();
	// 清除EMV APP CAPK的处理已经移到外面。
}

void NoDownloadInit(void)
{
#ifdef ALLOW_NO_TMS
	ScrCls();
	DispTransName();
	PubDispString(_T("LOAD DEFAULT ? "), 3|DISP_LINE_LEFT);
	if (!AskYesNo())
	{
		return;
	}

	ScrCls();
	DispTransName();
	DispProcess();

	InitEdcParam();
	glSysParam.ucTermStatus = TRANS_MODE;
	SaveSysParam();
	SaveSysCtrlAll();
	
#ifdef ENABLE_EMV
	memset(&glEmvStatus, 0, sizeof(EMV_STATUS));
	SaveEmvStatus();
#endif

	ScrCls();
	DispTransName();
	PubDispString(_T("LOADED OK."), 4|DISP_LINE_LEFT);
	PubWaitKey(3);

	SetSystemParamAll();
	SaveSysParam();
#endif
}

void DownLoadTMSPara_Manual(void)
{
	int iRet;
	DownLoadTMSPara(0);
	if (glSysParam.stEdcInfo.ucClssFlag == 1)
	{
		iRet =  InitClssParam();
		ProcError_Wave(iRet);
	}
}

void DownLoadTMSPara_Auto(void)
{
	DownLoadTMSPara(1);
}

void DownLoadTMSPara(uchar ucMode)
{
	uchar	ucCommType;
	int		iRet;
	
	InitTransInfo();
	TransInit(LOAD_PARA);

	ScrCls();
	DispTransName();
	if( PasswordBank()!=0 )
	{
		return;
	}

	ScrCls();
	DispTransName();
	if (!ChkIfBatchEmpty())
	{
		PubDispString(_T("PLS SETTLE BATCH"), 4|DISP_LINE_LEFT);
		PubWaitKey(USER_OPER_TIMEOUT);
		return;
	}

	// If never setup, switch to manual mode
	if ((ucMode!=0) && (glSysParam.stTMSCommCfg.ucCommType==CT_NONE))
	{
		ucMode = 0;
	}

	// 选择下载方式及输入数据
	if (ucMode==0)
	{
		iRet = SelectDownloadMode(&ucCommType);
		if( iRet!=0 )
		{
			return;
		}
	}
	else
	{
		sgNewTMS   = glSysParam.ucNewTMS;
		sgSyncDial = glSysParam.ucTMSSyncDial;
		ucCommType = glSysParam.stTMSCommCfg.ucCommType;
	}

	// 提取已输入的通信数据
	memcpy(&glCommCfg, &glSysParam.stTMSCommCfg, sizeof(COMM_CONFIG));
	if (glSysParam.ucTMSSyncDial)
	{
		memcpy(&glCommCfg.stPSTNPara.stPara, &glSysParam._TxnModemPara, sizeof(COMM_PARA));
	}

	ScrCls();
	DispTransName();
#if defined(_P60_S1_) || defined(_S_SERIES_)
	SetOffBase(OffBaseDisplay);
#endif

	// 下载
	if( sgNewTMS )
	{
		iRet = NewTmsDownLoad(ucCommType);
	}
	else
	{
		DispTransName();
		iRet = OldTmsDownLoad();
	}

#if defined(_P60_S1_) || defined(_S_SERIES_)
	SetOffBase(NULL);
#endif
	
	if( iRet!=0 )
	{
		CommOnHook(FALSE);
		ScrCls();
		DispTransName();
		PubDispString(_T("INITIAL FAIL"), 4|DISP_LINE_LEFT);
		ScrPrint(0, 7, ASCII, " %d ", iRet);
		PubBeepErr();
		PubWaitKey(3);
		return;
	}

	ScrCls();
	DispTransName();
	PubDispString(_T("INIT FINISHED."), 4|DISP_LINE_LEFT);
	PubBeepOk();
	PubWaitKey(3);

    // PP-DCC
    if (!sgNewTMS)
    {
        if (PPDCC_SetCurAcq()==0)
        {
            PPDCC_PromptDownloadBin();
        }
    }
}

#ifdef ENABLE_EMV
void RemoveEmvAppCapk(void)
{
	int				iCnt;
	int				iRet;
	EMV_CAPK		stEmvCapk;
	EMV_APPLIST		stEmvApp;

	for(iCnt=0; iCnt<MAX_KEY_NUM; iCnt++)
	{
		memset(&stEmvCapk, 0, sizeof(EMV_CAPK));
		iRet = EMVGetCAPK(iCnt, &stEmvCapk);
		if( iRet==EMV_OK )
		{
			EMVDelCAPK(stEmvCapk.KeyID, stEmvCapk.RID);
		}
	}
	for(iCnt=0; iCnt<MAX_APP_NUM; iCnt++)
	{
		memset(&stEmvApp, 0, sizeof(EMV_APPLIST));
		iRet = EMVGetApp(iCnt, &stEmvApp);
		if( iRet==EMV_OK )
		{
			EMVDelApp(stEmvApp.AID, (int)stEmvApp.AidLen);
		}
	}
}
#endif

int SelectDownloadMode(uchar *pucCommType)
{
	int		iRet;
	uchar	ucComm;
	SMART_MENU	stSmDownMode;
	static MENU_ITEM stCommMenu[] =
	{
		"SELECT MODE",	-1, NULL,
		"MODEM",		1, NULL,
		"TCPIP",		2, NULL,
		"GPRS",			3, NULL,
		/*"CDMA",			4, NULL,
		"OLD ASYNC",	5, NULL,
		"OLD SYNC",		6, NULL,
		"RS232",		7, NULL,
		"LOAD DEFAULT",	8, NULL,*/
		//2015-11-23
		"3G",			4, NULL,//2015-7-21
		"CDMA",			5, NULL,
		"OLD ASYNC",	6, NULL,
		"OLD SYNC",		7, NULL,
		"RS232",		8, NULL,
		"LOAD DEFAULT",	9, NULL,
		"",				0, NULL,
	};// This menu does not provide translation

	//--------------------------------------------------
	PubSmInit(&stSmDownMode, stCommMenu);

	if (ChkTerm(_TERMINAL_P60_S1_) || ChkTerm(_TERMINAL_P70_S_))
	{
		PubSmMask(&stSmDownMode, "TCPIP", SM_OFF);
		PubSmMask(&stSmDownMode, "GPRS", SM_OFF);
		PubSmMask(&stSmDownMode, "3G", SM_OFF);//2015-11-23
		PubSmMask(&stSmDownMode, "CDMA", SM_OFF);
	} 
	else
	{
#ifndef DEMO_HK
		if (ChkHardware(HWCFG_MODEM, HW_NONE))
		{
			PubSmMask(&stSmDownMode, "MODEM", SM_OFF);
			PubSmMask(&stSmDownMode, "OLD SYNC", SM_OFF);
			PubSmMask(&stSmDownMode, "OLD ASYNC", SM_OFF);
		}
#endif
		if (ChkHardware(HWCFG_LAN, HW_NONE))
		{
			PubSmMask(&stSmDownMode, "TCPIP", SM_OFF);
		}
		if (ChkHardware(HWCFG_GPRS, HW_NONE) && !ChkTerm(_TERMINAL_P90_))	// Sometimes P90 cannot get module info
		{
			PubSmMask(&stSmDownMode, "GPRS", SM_OFF);
		}
		if (ChkHardware(HWCFG_WCDMA, HW_NONE) && !ChkTerm(_TERMINAL_P90_))	// Sometimes P90 cannot get module info
		{
			PubSmMask(&stSmDownMode, "3G", SM_OFF);//2015-11-23
		}
		if (ChkHardware(HWCFG_CDMA, HW_NONE) && !ChkTerm(_TERMINAL_P90_))	// Sometimes P90 cannot get module info
		{
			PubSmMask(&stSmDownMode, "CDMA", SM_OFF);
		}
	}
#ifndef ALLOW_NO_TMS
	PubSmMask(&stSmDownMode, "LOAD DEFAULT", SM_OFF);
#endif

	iRet = PubSmartMenuEx(&stSmDownMode, SM_1ITEM_PER_LINE, USER_OPER_TIMEOUT);
	if (iRet<0)
	{
		return ERR_USERCANCEL;
	}

	sgSyncDial  = FALSE;
	sgNewTMS    = TRUE;
	ucComm      = CT_NONE;
	switch(iRet)
	{
	case 1:
		ucComm = CT_MODEM;
		break;
	case 2:
		ucComm = CT_TCPIP;
		break;
	case 3:
		ucComm = CT_GPRS;
	    break;
	/*case 4:
		ucComm = CT_CDMA;
	    break;
	case 5:
		ucComm = CT_MODEM;
		sgNewTMS = FALSE;
	    break;
	case 6:
		ucComm = CT_MODEM;
		sgNewTMS = FALSE;
		sgSyncDial = TRUE;
		break;
	case 7:
		ucComm = CT_RS232;
		sgNewTMS = FALSE;
		sgSyncDial = TRUE;
		break;
	case 8:
		NoDownloadInit();
		return ERR_NO_DISP;*/
	case 4://2015-11-23 3G
		ucComm = CT_3G;
	    break;
	case 5:
		ucComm = CT_CDMA;
	    break;
	case 6:
		ucComm = CT_MODEM;
		sgNewTMS = FALSE;
	    break;
	case 7:
		ucComm = CT_MODEM;
		sgNewTMS = FALSE;
		sgSyncDial = TRUE;
		break;
	case 8:
		ucComm = CT_RS232;
		sgNewTMS = FALSE;
		sgSyncDial = TRUE;
		break;
	case 9:
		NoDownloadInit();
		return ERR_NO_DISP;
	default:
	    return ERR_NO_DISP;
	}

	if( GetDownLoadComm(ucComm)!=0 )
	{
		return ERR_NO_DISP;
	}

	iRet = GetDownLoadTID(glSysParam.stEdcInfo.szDownLoadTID);
	if (iRet!=0)
	{
		return ERR_NO_DISP;
	}

	// save TMS download settings.
	glSysParam.stTMSCommCfg.ucCommType = ucComm;
	glSysParam.stTMSCommCfg.stRS232Para.ucSendMode = (sgSyncDial ? CM_SYNC : CM_ASYNC);
	glSysParam.stTMSCommCfg.stPSTNPara.ucSendMode  = (sgSyncDial ? CM_SYNC : CM_ASYNC);
	glSysParam.ucNewTMS      = sgNewTMS;
	glSysParam.ucTMSSyncDial = sgSyncDial;
	SaveSysParam();

	*pucCommType = ucComm;
	return 0;
}

// Monitor下载文件的参数下载
// New Protims download protocol, done by monitor level.
int NewTmsDownLoad(uchar ucCommType)
{
#ifndef _WIN32
	int				iRet;
	T_INCOMMPARA	stCommPara;
	TMS_LOADSTATUS	stLoadStatus;
	COMM_PARA		stModemPara;
	uchar			szTelNo[25+1+1], szTermID[8+1];

	memset(&stCommPara, 0, sizeof(T_INCOMMPARA));
	stCommPara.psAppName  = (uchar *)"";		//(uchar *)AppInfo.AppName;
	sprintf((char *)szTermID, "%.8s", glSysParam.stEdcInfo.szDownLoadTID);
	stCommPara.psTermID   = szTermID;
	//stCommPara.ucCallMode = 0x10;
	stCommPara.ucCallMode = 0x00;
	stCommPara.bLoadType  = 0xFF;	//0x07;
#ifdef _S_SERIES_
	stCommPara.psProtocol = 0x00;
#endif

	switch(ucCommType)
	{
	case CT_MODEM:
		memset(&stModemPara, 0, sizeof(COMM_PARA));
		stModemPara.CHDT    = 0x00;
		stModemPara.DT1     = 0x0A;//5;
		stModemPara.DT2     = 0x0A;//5;
		stModemPara.HT      = 0x64;
		stModemPara.WT      = 0x0A;
		stModemPara.SSETUP  = 0xE7; //0x87;	/* asynchronise link */
		stModemPara.DTIMES  = 0;
		stModemPara.AsMode  = 0x70;
		stModemPara.TimeOut = 6;	// 60 seconds
		stCommPara.bCommMode  = 1;		// modem
		if( glProcInfo.bAutoDownFlag )
		{
			stCommPara.ucCallMode = 0x11;
		}
		sprintf((char *)szTelNo, "%.25s.", glSysParam.stEdcInfo.szDownTelNo);
		stCommPara.tUnion.tModem.psTelNo   = szTelNo;
		stCommPara.tUnion.tModem.bTimeout  = 1;
		stCommPara.tUnion.tModem.ptModPara = &stModemPara;
		break;
	case CT_RS232:
		stCommPara.bCommMode = 0;
		stCommPara.tUnion.tSerial.psPara = (uchar *)"115200,8,n,1";
		break;
	case CT_TCPIP:
#ifdef _S_SERIES_
		stCommPara.bCommMode = 2;
		stCommPara.tUnion.tLAN.psLocal_IP_Addr = glSysParam._TmsTcpIpPara.szLocalIP;
		stCommPara.tUnion.tLAN.wLocalPortNo = 1010;
		//stCommPara.tUnion.tLAN.wLocalPortNo = 2;
		stCommPara.tUnion.tLAN.psSubnetMask	= glSysParam._TmsTcpIpPara.szNetMask; 
		stCommPara.tUnion.tLAN.psGatewayAddr = glSysParam._TmsTcpIpPara.szGatewayIP;
		stCommPara.tUnion.tLAN.psRemote_IP_Addr = glSysParam.stEdcInfo.stDownIpAddr.szIP;
		stCommPara.tUnion.tLAN.wRemotePortNo = (ushort)atol((char *)glSysParam.stEdcInfo.stDownIpAddr.szPort);
		break;
#else
		return ERR_NO_DISP;
#endif
	case CT_GPRS:
	case CT_CDMA:
    case CT_3G:
		stCommPara.bCommMode = ((ucCommType==CT_GPRS) ? 3 : 4);
		stCommPara.tUnion.tGPRS.psAPN      = stCommPara.tUnion.tCDMA.psTelNo    = glSysParam._TmsWirlessPara.szAPN;
		stCommPara.tUnion.tGPRS.psUserName = stCommPara.tUnion.tCDMA.psUserName = glSysParam._TmsWirlessPara.szUID;
		stCommPara.tUnion.tGPRS.psPasswd   = stCommPara.tUnion.tCDMA.psPasswd   = glSysParam._TmsWirlessPara.szPwd;
		stCommPara.tUnion.tGPRS.psPIN_CODE = stCommPara.tUnion.tCDMA.psPIN_CODE = glSysParam._TmsWirlessPara.szSimPin;
		stCommPara.tUnion.tGPRS.psIP_Addr  = stCommPara.tUnion.tCDMA.psIP_Addr  = glSysParam.stEdcInfo.stDownIpAddr.szIP;
		stCommPara.tUnion.tGPRS.nPortNo    = stCommPara.tUnion.tCDMA.nPortNo    = (ushort)atol((char *)glSysParam.stEdcInfo.stDownIpAddr.szPort);
		break;
//	case CT_WIFI:
//		break;
	default:
		return ERR_NO_DISP;
	}
	
	iRet = RemoteLoadApp(&stCommPara);

	CommOnHook(FALSE);
	ScrPrint(0, 7, ASCII, "%i", iRet);
	SaveEdcParam();
	PubWaitKey(3);
	if( iRet!=0 )
	{
		return iRet;
	}

	//memset(&stLoadStatus, 0, sizeof(TMS_LOADSTATUS));
	//iRet = GetLoadedAppStatus((uchar *)"", &stLoadStatus);
	//if( iRet!=0 )
	//{
	//	return iRet;
	//}

	if( stLoadStatus.bAppTotal!=0 )
	{
		ScrClrLine(2, 7);
		PubDispString(_T("SYSTEM UPDATED."), 3|DISP_LINE_LEFT);
		if (!ChkTerm(_TERMINAL_S90_))	// S90 do not support soft-reboot
		{
#ifdef _S_SERIES_
			PubDispString(_T("REBOOT..."), 5|DISP_LINE_LEFT);
			PubWaitKey(3);
			Reboot();
#endif
		}
		PubDispString(_T("PLS REBOOT POS."), 5|DISP_LINE_LEFT);
		while(1);
	}

	return 0;
#else
	ScrCls();
	PubDispString(_T("NOT IMPLEMENT"), 4|DISP_LINE_LEFT);
	PubWaitKey(3);
	return ERR_TRAN_FAIL;
#endif
}

// 传统的8583方式的参数下载
// Traditional Protims download protocol, by application level, through ISO8583 message packet
int OldTmsDownLoad(void)
{
	int		iRet;
	long	lDataLen;

	sgEMVDownloaded = 0;

	sprintf((char *)glCurAcq.szNii,    "%.3s", glSysParam.stEdcInfo.szTMSNii);
	sprintf((char *)glCurAcq.szTermID, "%.8s", glSysParam.stEdcInfo.szDownLoadTID);

	glCurAcq.ucPhoneTimeOut = glSysParam.stEdcInfo.ucTMSTimeOut;
	glCurAcq.ucTcpTimeOut   = glSysParam.stEdcInfo.ucTMSTimeOut;
	glCurAcq.ucPppTimeOut   = glSysParam.stEdcInfo.ucTMSTimeOut;
	glCurAcq.ucGprsTimeOut  = glSysParam.stEdcInfo.ucTMSTimeOut;

	SetCommReqField();
	ReadSN(glTMSSend.szField61);
	iRet = SendRecvPacket();
	if( iRet!=0 )
	{
		return iRet;
	}
	iRet = AfterTranProc();
	if( iRet!=0 )
	{
		return iRet;
	}

	ResetAllPara(FALSE);

	while( 1 )
	{
		memset(sgTempBuf, 0, sizeof(sgTempBuf));
		PubChar2Long(glTMSRecv.sField60, 2, (ulong *)&lDataLen);
		memcpy(sgTempBuf, &glTMSRecv.sField60[2], (int)lDataLen);

		if( glTMSRecv.szProcCode[LEN_PROC_CODE-1]!='0' )
		{
			SetCommReqField();
			glTMSSend.szProcCode[LEN_PROC_CODE-1] = '1';
			ReadSN(glTMSSend.szField61);
			iRet = SendPacket();
			if( iRet!=0 )
			{
				PubTRACE1("txd tms:%d", iRet);
				return iRet;
			}
		}

		DispProcess();
		if( UnpackPara(sgTempBuf, lDataLen)!=0 )
		{
			PubTRACE0("unpack tms");
			return ERR_NO_DISP;
		}
		if( glTMSRecv.szProcCode[LEN_PROC_CODE-1]=='0' )
		{
			break;
		}

		iRet = RecvPacket();
		if( iRet!=0 )
		{
			PubTRACE1("rxd tms:%d", iRet);
			return iRet;
		}
		iRet = AfterTranProc();
		if( iRet!=0 )
		{
			PubTRACE1("proc tms:%d", iRet);
			return iRet;
		}
#ifdef _WIN32
		DelayMs(300);
#endif
	}

	CommOnHook(FALSE);
	DispProcess();

	if( !SearchIssuerKeyArray() )
	{
		return ERR_NO_DISP;
	}

	AfterLoadParaProc();
	return 0;
}

// save EDC parameter
void UnpackParaEdc(uchar *psPara)
{
	TMS_EDC_INFO	*pstEdc;
	CURRENCY_CONFIG	stCurrency;

	pstEdc = (TMS_EDC_INFO*)psPara;
	glSysParam.stEdcInfo.ucDllTracking = pstEdc->ucDllTracking;
	glSysParam.stEdcInfo.bClearBatch   = pstEdc->bClearBatch;
	glSysParam.stEdcInfo.ucPrinterType = pstEdc->ucPrinterType;
	glSysParam.stEdcInfo.ucEcrSpeed    = pstEdc->ucEcrSpeed;
	//glSysParam.stEdcInfo.ucLanguage    = pstEdc->ucLanguage;
	glSysParam.stEdcInfo.ucCurrencySymbol = pstEdc->ucCurrencySymbol;
	glSysParam.stEdcInfo.ucTranAmtLen  = (uchar)PubBcd2Long(&pstEdc->ucTranAmtLen, 1, NULL);
	glSysParam.stEdcInfo.ucStlAmtLen   = (uchar)PubBcd2Long(&pstEdc->ucStlAmtLen,  1, NULL);
	glSysParam.stEdcInfo.stLocalCurrency.ucDecimal = (uchar)PubBcd2Long(&pstEdc->ucDecimalPos, 1, NULL);

	SetTime(pstEdc->sInitTime);
	memcpy(glSysParam.stEdcInfo.szInitTime, "20", 2);
	PubBcd2Asc(pstEdc->sInitTime, 6, &glSysParam.stEdcInfo.szInitTime[2]);

	PubBcd2Asc(pstEdc->sHelpTelNo, 12, glSysParam.stEdcInfo.szHelpTelNo);
	PubTrimTailChars(glSysParam.stEdcInfo.szHelpTelNo, 'F');

	glSysParam.stEdcInfo.sOption[0] = pstEdc->ucOption1;
	glSysParam.stEdcInfo.sOption[1] = pstEdc->ucOption2;
	glSysParam.stEdcInfo.sOption[2] = pstEdc->ucOption3;
	glSysParam.stEdcInfo.sOption[3] = pstEdc->ucPwdMask;
	glSysParam.stEdcInfo.sOption[4] = pstEdc->ucDialOption;

	glSysParam.stEdcInfo.sReserved[0] = pstEdc->ucUnused1;
	memcpy(&glSysParam.stEdcInfo.sReserved[1], pstEdc->sUnused2, 3);

	memcpy(glSysParam.stEdcInfo.szMerchantAddr, pstEdc->sMerchantAddr, 46);
	memcpy(glSysParam.stEdcInfo.szMerchantName, pstEdc->sMerchantName, 23);
	memcpy(glSysParam.stEdcInfo.szAddlPrompt,   pstEdc->sAddlPrompt,   20);
	sprintf(glSysParam.stEdcInfo.stLocalCurrency.szName, "%.3s", pstEdc->sCurrencyName);
	if (glSysParam.stEdcInfo.stLocalCurrency.szName[2]==' ')
	{
		glSysParam.stEdcInfo.stLocalCurrency.szName[2] = 0;
	}
	PubBcd2Long(pstEdc->sOfflineLimit, 5, &glSysParam.stEdcInfo.ulOfflineLimit);

	// Protims do not provide ignore digit, it should be determined by EDC
	if (FindCurrency(glSysParam.stEdcInfo.stLocalCurrency.szName, &stCurrency)==0)
	{
		glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit = stCurrency.ucIgnoreDigit;
		memcpy(glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, stCurrency.sCurrencyCode, 2);
        memcpy(glSysParam.stEdcInfo.stLocalCurrency.sCountryCode, stCurrency.sCountryCode, 2);
		// 如果查表得到该货币支持角分，但Protims设置decimal  position为0位（即设置为不输入角分）
		// 则应把角分计入忽略位
		if (glSysParam.stEdcInfo.stLocalCurrency.ucDecimal<stCurrency.ucDecimal)
		{
			glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit += (stCurrency.ucDecimal-glSysParam.stEdcInfo.stLocalCurrency.ucDecimal);
		}
	}
	else
	{
		glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit = 0;
	}

	memcpy(glSysParam.stEdcInfo.sInitSTAN, pstEdc->sInitialSTAN, 3);
	
	if (ChkEdcOption(EDC_WINGLUNG_ENCRY)) //Gillian 2016-8-1
	{
		glSysParam.stEdcInfo.sExtOption[0]  |= EDC_EXT_TCP_ENC;//2016-2-29 *** //Gillian 2016-7-14
	}
	// save password
	PubBcd2Asc0(pstEdc->sVoidPwd,   2, glSysParam.sPassword[PWD_VOID]);
	PubBcd2Asc0(pstEdc->sRefundPwd, 2, glSysParam.sPassword[PWD_REFUND]);
	PubBcd2Asc0(pstEdc->sSettlePwd, 2, glSysParam.sPassword[PWD_SETTLE]);
	PubBcd2Asc0(pstEdc->sAdjustPwd, 2, glSysParam.sPassword[PWD_ADJUST]);
	PubBcd2Asc0(pstEdc->sTermPwd,   2, glSysParam.sPassword[PWD_TERM]);

    // Printer type
	glSysParam.stEdcInfo.ucPrinterType = pstEdc->ucPrinterType;
	if (!ChkTerm(_TERMINAL_P60_S1_) && !ChkTerm(_TERMINAL_P70_S_) && !ChkTerm(_TERMINAL_S60_))
	{
		if (ChkHardware(HWCFG_PRINTER, 'T')==TRUE)
		{
			glSysParam.stEdcInfo.ucPrinterType = 1;
		}
		else
		{
			glSysParam.stEdcInfo.ucPrinterType = 0;
		}
	}
}

void TransformIP(uchar * ip_in, uchar * ip_out)
{
	sprintf(ip_out, "%u.%u.%u.%u", ip_in[0],ip_in[1],ip_in[2],ip_in[3]);
}

void TransformPort(uchar * port_in, uchar * port_out)
{
	int iPortNum;
	iPortNum = port_in[0]*256+port_in[1];
	sprintf(port_out, "%u", iPortNum );
}

// save acquirer parameters
int UnpackParaAcq(uchar ucIndex, uchar *psPara, ulong ulSubFieldLen)
{
	uchar			ucNum, szBuff[10];
	TMS_ACQUIRER	*pstAcq;

	if( glSysParam.ucAcqNum >= MAX_ACQ )
	{
		PubDispString(_T("MAX # OF ACQ"), 4|DISP_LINE_LEFT);
		PubWaitKey(2);
		return ERR_NO_DISP;
	}

	ucNum  = glSysParam.ucAcqNum;
	pstAcq = (TMS_ACQUIRER *)psPara;
//	PubTRACEHEX("acq", psPara, sizeof(TMS_ACQUIRER));

	glSysParam.stAcqList[ucNum].ucKey = pstAcq->ucKey;
	memcpy(glSysParam.stAcqList[ucNum].szName, pstAcq->sName, 10);
	memcpy(glSysParam.stAcqList[ucNum].szPrgName, pstAcq->sPrgName, 10);

	memcpy(glSysParam.stAcqList[ucNum].sOption, pstAcq->sOption, 4);

	PubBcd2Asc0(pstAcq->sNii, 2, szBuff);
// 	PubTRACE1("NII[%s]", szBuff);
	memcpy(glSysParam.stAcqList[ucNum].szNii,        &szBuff[1],          3);
	memcpy(glSysParam.stAcqList[ucNum].szTermID,     pstAcq->sTermID,     8);
	memcpy(glSysParam.stAcqList[ucNum].szMerchantID, pstAcq->sMerchantID, 15);

	glSysParam.stAcqList[ucNum].ucPhoneTimeOut = (uchar)PubBcd2Long(&pstAcq->ucTimeOut, 1, NULL);
	if( memcmp(pstAcq->sCurBatchNo, "\x00\x00\x00", 3)!=0 )
	{
		PubBcd2Long(pstAcq->sCurBatchNo, 3, &glSysParam.stAcqList[ucNum].ulCurBatchNo);
	}
	if( glSysParam.stAcqList[ucNum].ulCurBatchNo==0L )
	{
		glSysParam.stAcqList[ucNum].ulCurBatchNo++;
	}
	glSysParam.stAcqList[ucNum].ulNextBatchNo = GetNewBatchNo(glSysParam.stAcqList[ucNum].ulCurBatchNo);

	memcpy(glSysParam.stAcqList[ucNum].szVisa1TermID, pstAcq->sVisa1TermID, 23); //Alex
	memcpy(glSysParam.stAcqList[ucNum].sReserved,     pstAcq->sReserved,    4);

	//------------------------ process modem parameter ------------------------
	if(pstAcq->sTxnTelNo1[0]!=0xff)
	{
		PubBcd2Asc0(pstAcq->sTxnTelNo1 , 12, glSysParam.stAcqList[ucNum].TxnTelNo1);
		PubTrimTailChars(glSysParam.stAcqList[ucNum].TxnTelNo1, 'F');
		glSysParam.stAcqList[ucNum].stTxnPhoneInfo[0].ucDialWait     = pstAcq->ucTxnDialWait1;
		glSysParam.stAcqList[ucNum].stTxnPhoneInfo[0].ucDialAttempts = pstAcq->ucTxnDialAttempts1;

		PubBcd2Asc0(pstAcq->sTxnTelNo2, 12, glSysParam.stAcqList[ucNum].TxnTelNo2);
		PubTrimTailChars(glSysParam.stAcqList[ucNum].TxnTelNo2, 'F');
		glSysParam.stAcqList[ucNum].stTxnPhoneInfo[1].ucDialWait     = pstAcq->ucTxnDialWait2;
		glSysParam.stAcqList[ucNum].stTxnPhoneInfo[1].ucDialAttempts = pstAcq->ucTxnDialAttempts2;

		PubBcd2Asc0(pstAcq->sStlTelNo1, 12, glSysParam.stAcqList[ucNum].StlTelNo1);
		PubTrimTailChars(glSysParam.stAcqList[ucNum].StlTelNo1, 'F');
		glSysParam.stAcqList[ucNum].stStlPhoneInfo[0].ucDialWait     = pstAcq->ucStlDialWait1;
		glSysParam.stAcqList[ucNum].stStlPhoneInfo[0].ucDialAttempts = pstAcq->ucStlDialAttempts1;

		PubBcd2Asc0(pstAcq->sStlTelNo2, 12, glSysParam.stAcqList[ucNum].StlTelNo2);
		PubTrimTailChars(glSysParam.stAcqList[ucNum].StlTelNo2, 'F');
		glSysParam.stAcqList[ucNum].stStlPhoneInfo[0].ucDialWait     = pstAcq->ucStlDialWait2;
		glSysParam.stAcqList[ucNum].stStlPhoneInfo[0].ucDialAttempts = pstAcq->ucStlDialAttempts2;

		glSysParam.stAcqList[ucNum].ucTxnModemMode = pstAcq->ucTxnModemMode;
		glSysParam.stAcqList[ucNum].ucStlModemMode = pstAcq->ucStlModemMode;
	}

	{int iTemp = sizeof(TMS_ACQUIRER);}	// debug use

	// TMS_ACQUIRER is defined to same as in new protims
	// so if using old protims/tims, the input data length will far less than sizeof(TMS_ACQUIRER)
	// to avoid the error caused by memory align, use "sizeof(TMS_ACQUIRER)-8" for comparison
	if (ulSubFieldLen > sizeof(TMS_ACQUIRER)-8)
	{
		memcpy(glSysParam.stAcqList[ucNum].sCommTypes, pstAcq->COMM_Mode, 8);

		//------------------------ process GPRS parameter ------------------------
		if ( memcmp(pstAcq->WIR_txn_IPADD1,"\x00\x00\x00\x00",4)!=0 )
		{
			// For APN,UID,PWD,SIMPIN, EDC only adopt ACQ's first available set
			// All ACQ share them. So only need to setup in one (or none) of those acquirer
			if ((pstAcq->WIR_APN[0]!=0) && (pstAcq->WIR_APN[0]!=' '))
			{
				memcpy(glSysParam._TxnWirlessPara.szAPN, pstAcq->WIR_APN, 64);
				PubTrimTailChars(glSysParam._TxnWirlessPara.szAPN, ' ');
			}
			if ((pstAcq->WIR_USER[0]!=0) && (pstAcq->WIR_USER[0]!=' '))
			{
				memcpy(glSysParam._TxnWirlessPara.szUID, pstAcq->WIR_USER, 64);
				PubTrimTailChars(glSysParam._TxnWirlessPara.szUID, ' ');
			}
			if ((pstAcq->WIR_PWD[0]!=0) && (pstAcq->WIR_PWD[0]!=' '))
			{
				memcpy(glSysParam._TxnWirlessPara.szPwd, pstAcq->WIR_PWD, 64);
				PubTrimTailChars(glSysParam._TxnWirlessPara.szPwd, ' ');
			}
			if ((pstAcq->WIR_SIMPIN[0]!=0) && (pstAcq->WIR_SIMPIN[0]!=' '))
			{
				memcpy(glSysParam._TxnWirlessPara.szSimPin, pstAcq->WIR_SIMPIN, 16);
				PubTrimTailChars(glSysParam._TxnWirlessPara.szSimPin, ' ');
			}

			TransformIP(pstAcq->WIR_txn_IPADD1,glSysParam.stAcqList[ucNum].stTxnGPRSInfo[0].szIP);
			TransformPort(pstAcq->WIR_txn_IPPORT1, glSysParam.stAcqList[ucNum].stTxnGPRSInfo[0].szPort);

			TransformIP(pstAcq->WIR_txn_IPADD2,glSysParam.stAcqList[ucNum].stTxnGPRSInfo[1].szIP);
			TransformPort(pstAcq->WIR_txn_IPPORT2, glSysParam.stAcqList[ucNum].stTxnGPRSInfo[1].szPort);

			TransformIP(pstAcq->WIR_stl_IPADD1,glSysParam.stAcqList[ucNum].stStlGPRSInfo[0].szIP);
			TransformPort(pstAcq->WIR_stl_IPPORT1, glSysParam.stAcqList[ucNum].stStlGPRSInfo[0].szPort);

			TransformIP(pstAcq->WIR_stl_IPADD2,glSysParam.stAcqList[ucNum].stStlGPRSInfo[1].szIP);
			TransformPort(pstAcq->WIR_stl_IPPORT2, glSysParam.stAcqList[ucNum].stStlGPRSInfo[1].szPort);

			glSysParam.stAcqList[ucNum].ucGprsTimeOut = pstAcq->WIR_timeout_val;
		}

		if ( memcmp(pstAcq->TCP_txn_IPADD1,"\x00\x00\x00\x00",4)!=0 )
		{
			TransformIP(pstAcq->TCP_txn_IPADD1, glSysParam.stAcqList[ucNum].stTxnTCPIPInfo[0].szIP);
			TransformPort(pstAcq->TCP_txn_IPPORT1, glSysParam.stAcqList[ucNum].stTxnTCPIPInfo[0].szPort);

			TransformIP(pstAcq->TCP_txn_IPADD2, glSysParam.stAcqList[ucNum].stTxnTCPIPInfo[1].szIP);
			TransformPort(pstAcq->TCP_txn_IPPORT2, glSysParam.stAcqList[ucNum].stTxnTCPIPInfo[1].szPort);

			TransformIP(pstAcq->TCP_stl_IPADD1, glSysParam.stAcqList[ucNum].stStlTCPIPInfo[0].szIP);
			TransformPort(pstAcq->TCP_stl_IPPORT1, glSysParam.stAcqList[ucNum].stStlTCPIPInfo[0].szPort);

			TransformIP(pstAcq->TCP_stl_IPADD2, glSysParam.stAcqList[ucNum].stStlTCPIPInfo[1].szIP);
			TransformPort(pstAcq->TCP_stl_IPPORT2, glSysParam.stAcqList[ucNum].stStlTCPIPInfo[1].szPort);

			glSysParam.stAcqList[ucNum].ucTcpTimeOut = pstAcq->TCP_timeout_val;
		}

		if ( memcmp(pstAcq->PPP_txn_IPADD1,"\x00\x00\x00\x00",4)!=0 )
		{
			PubBcd2Asc0(pstAcq->PPP_txn_PHONE, 12, glSysParam.stAcqList[ucNum].stTxnPPPInfo[0].szTelNo);
			PubTrimTailChars(glSysParam.stAcqList[ucNum].stTxnPPPInfo[0].szTelNo, 'F');
			TransformIP(pstAcq->PPP_txn_IPADD1, glSysParam.stAcqList[ucNum].stTxnPPPInfo[0].szIPAddr);
			TransformPort(pstAcq->PPP_txn_IPPORT1, glSysParam.stAcqList[ucNum].stTxnPPPInfo[0].szIPPort);
			memcpy(glSysParam.stAcqList[ucNum].stTxnPPPInfo[0].szUserName, pstAcq->PPP_txn_USERNAME, 16);
			memcpy(glSysParam.stAcqList[ucNum].stTxnPPPInfo[0].szUserPWD, pstAcq->PPP_txn_USERPWD, 16);

			PubBcd2Asc0(pstAcq->PPP_txn_PHONE, 12, glSysParam.stAcqList[ucNum].stTxnPPPInfo[1].szTelNo);
			PubTrimTailChars(glSysParam.stAcqList[ucNum].stTxnPPPInfo[1].szTelNo, 'F');
			TransformIP(pstAcq->PPP_txn_IPADD2, glSysParam.stAcqList[ucNum].stTxnPPPInfo[1].szIPAddr);
			TransformPort(pstAcq->PPP_txn_IPPORT2, glSysParam.stAcqList[ucNum].stTxnPPPInfo[1].szIPPort);
			memcpy(glSysParam.stAcqList[ucNum].stTxnPPPInfo[1].szUserName, pstAcq->PPP_txn_USERNAME, 16);
			memcpy(glSysParam.stAcqList[ucNum].stTxnPPPInfo[1].szUserPWD, pstAcq->PPP_txn_USERPWD, 16);

			PubBcd2Asc0(pstAcq->PPP_stl_PHONE, 12, glSysParam.stAcqList[ucNum].stStlPPPInfo[0].szTelNo);
			PubTrimTailChars(glSysParam.stAcqList[ucNum].stStlPPPInfo[0].szTelNo, 'F');
			TransformIP(pstAcq->PPP_stl_IPADD1, glSysParam.stAcqList[ucNum].stStlPPPInfo[0].szIPAddr);
			TransformPort(pstAcq->PPP_stl_IPPORT1, glSysParam.stAcqList[ucNum].stStlPPPInfo[0].szIPPort);
			memcpy(glSysParam.stAcqList[ucNum].stStlPPPInfo[0].szUserName, pstAcq->PPP_stl_USERNAME, 16);
			memcpy(glSysParam.stAcqList[ucNum].stStlPPPInfo[0].szUserPWD, pstAcq->PPP_stl_USERPWD, 16);

			PubBcd2Asc0(pstAcq->PPP_stl_PHONE, 12, glSysParam.stAcqList[ucNum].stStlPPPInfo[1].szTelNo);
			PubTrimTailChars(glSysParam.stAcqList[ucNum].stStlPPPInfo[1].szTelNo, 'F');
			TransformIP(pstAcq->PPP_stl_IPADD2, glSysParam.stAcqList[ucNum].stStlPPPInfo[1].szIPAddr);
			TransformPort(pstAcq->PPP_stl_IPPORT2, glSysParam.stAcqList[ucNum].stStlPPPInfo[1].szIPPort);
			memcpy(glSysParam.stAcqList[ucNum].stStlPPPInfo[1].szUserName, pstAcq->PPP_stl_USERNAME, 16);
			memcpy(glSysParam.stAcqList[ucNum].stStlPPPInfo[1].szUserPWD, pstAcq->PPP_stl_USERPWD, 16);

			glSysParam.stAcqList[ucNum].ucPppTimeOut = pstAcq->PPP_timeout_val;
		}
	}

	glSysCtrl.sAcqStatus[ucNum] = S_USE;
	glSysParam.ucAcqNum++;
	return 0;
}

// save issuer parameters
int UnpackParaIssuer(uchar ucIndex, uchar *psPara)
{
	uchar		ucNum;
	TMS_ISSUER	*pstIssuer;

	if( glSysParam.ucIssuerNum >= MAX_ISSUER )
	{
		PubDispString(_T("MAX # OF ISSUER"), 4|DISP_LINE_LEFT);
		PubWaitKey(2);
		return ERR_NO_DISP;
	}

	pstIssuer = (TMS_ISSUER *)psPara;
	ucNum = glSysParam.ucIssuerNum;

	glSysParam.stIssuerList[ucNum].ucKey = pstIssuer->ucKey;
	memcpy(glSysParam.stIssuerList[ucNum].szName, pstIssuer->sName, 10);
	PubBcd2Asc(pstIssuer->sRefTelNo, 12, glSysParam.stIssuerList[ucNum].szRefTelNo);
	PubTrimTailChars(glSysParam.stIssuerList[ucNum].szRefTelNo, 'F');
	memcpy(glSysParam.stIssuerList[ucNum].sOption, pstIssuer->sOption, 4);

	glSysParam.stIssuerList[ucNum].ucDefAccount = pstIssuer->ucDefAccount;

	glSysParam.stIssuerList[ucNum].sPanMask[0] = pstIssuer->sReserved[0];
	glSysParam.stIssuerList[ucNum].sPanMask[1] = pstIssuer->sPanMask[1];
	glSysParam.stIssuerList[ucNum].sPanMask[2] = pstIssuer->sPanMask[0];

	PubBcd2Long(pstIssuer->sFloorLimit, 2, &glSysParam.stIssuerList[ucNum].ulFloorLimit);

	glSysParam.stIssuerList[ucNum].ucAdjustPercent = (uchar)PubBcd2Long(&pstIssuer->ucAdjustPercent, 1, NULL);
	glSysParam.stIssuerList[ucNum].ucReserved = pstIssuer->sReserved[1];

	glSysParam.ucIssuerNum++;

	return 0;
}

// save card range parameters
int UnpackParaCard(uchar ucIndex, uchar *psPara)
{
	CARD_TABLE	*pstCardTbl;

	if( glSysParam.ucCardNum >= MAX_CARD )
	{
		PubDispString(_T("MAX # OF CARD"), 4|DISP_LINE_LEFT);
		PubWaitKey(2);
		return ERR_NO_DISP;
	}

	pstCardTbl = &glSysParam.stCardTable[glSysParam.ucCardNum];
	memcpy(pstCardTbl, psPara, sizeof(CARD_TABLE));
	pstCardTbl->ucPanLength = (uchar)PubBcd2Long(&pstCardTbl->ucPanLength, 1, NULL);
	glSysParam.ucCardNum++;

	return 0;
}

// save EPS parameters
int UnpackEPSPara(uchar *psPara)
{
	TMS_EPS_PARAM		*pstTmsEpsPara;
	EPS_MULTISYS_PARAM	stEpsSysPara;

	pstTmsEpsPara = (TMS_EPS_PARAM *)psPara;
	memset(&stEpsSysPara, 0, sizeof(EPS_MULTISYS_PARAM));
	memcpy(stEpsSysPara.TXNCode, "000", 3);
	memcpy(stEpsSysPara.CurrencySymbol, "HKD", 3);
	memcpy(stEpsSysPara.CurrencyCode, "344", 3);
	memcpy(stEpsSysPara.MerchantPWD, "0000", 4);
	memcpy(stEpsSysPara.TerminalPWD, "0000", 4);
	memcpy(stEpsSysPara.BANKPWD, "000000", 6);
	memcpy(stEpsSysPara.TerminalNo, "721\x0", 4);
	memcpy(stEpsSysPara.StoreNo, "123\x0", 4);
	memcpy(stEpsSysPara.MerchantNo, "000000888", 10);

	stEpsSysPara.TerminalContrast = 4;
	stEpsSysPara.TerminalLocked   = EPS_TERMINAL_UNLOCKED;
	stEpsSysPara.InitialFlag      = EPS_NOT_INITIALIZED;
	stEpsSysPara.AnswerToneWait   = 50;
	stEpsSysPara.RedialWait       = 50;
	stEpsSysPara.WelcomeWait      = 20;
	stEpsSysPara.NextTxnWait      = 50;
	stEpsSysPara.DebitRespWait    = 30;
	stEpsSysPara.AdmRespWait      = 60;
	stEpsSysPara.ReverseRetryWait = 120;

	stEpsSysPara.DailMode         = 0; // DIAL_SYNCHRON=0;
	stEpsSysPara.DP   = 0;
	stEpsSysPara.CHDT = 0;
	stEpsSysPara.DT1  = 5;
	stEpsSysPara.DT2  = 7;
	stEpsSysPara.HT   = 70;
	stEpsSysPara.WT   = 5;
	stEpsSysPara.SSETUP = 0x05;	/* synchron link */
	stEpsSysPara.DTIMES = 1;
	stEpsSysPara.AsMode = 0;
	stEpsSysPara.HYear  = 0x20;

	memcpy(stEpsSysPara.BANKPWD,     pstTmsEpsPara->sBankPwd, 6);
	memcpy(stEpsSysPara.MerchantNo,  pstTmsEpsPara->sMerchantID, 9);
	memcpy(stEpsSysPara.StoreNo,     pstTmsEpsPara->sStoreID, 3);
	memcpy(stEpsSysPara.TerminalNo,  pstTmsEpsPara->sTerminalID, 3);
	memcpy(stEpsSysPara.TelNo[0],    pstTmsEpsPara->sTelNo[0], 24);
	memcpy(stEpsSysPara.TelNo[1],    pstTmsEpsPara->sTelNo[1], 24);
	memcpy(stEpsSysPara.MerchantPWD, pstTmsEpsPara->sMerchantPwd, 4);
	memcpy(stEpsSysPara.TerminalPWD, pstTmsEpsPara->sMerchantPwd, 4);
	if( pstTmsEpsPara->bGlobalPabx==1 )
	{
		stEpsSysPara.PABX[0] = pstTmsEpsPara->ucPabx1;
		memcpy(&stEpsSysPara.PABX[1], pstTmsEpsPara->sPabx2, 7);
		sprintf((char *)glSysParam.stEdcInfo.szPabx, "%.8s", stEpsSysPara.PABX);
	}
	memcpy(glSysParam.sPassword[PWD_BANK],     pstTmsEpsPara->sBankPwd, 6);
	memcpy(glSysParam.sPassword[PWD_MERCHANT], pstTmsEpsPara->sMerchantPwd, 4);
	memcpy(glSysParam.sPassword[PWD_TERM],     pstTmsEpsPara->sMerchantPwd, 4);


	if( pstTmsEpsPara->bEpsEnabled!=1 )
	{
		return 0;
	}

	return WriteEPSPara(&stEpsSysPara);
}

// save instalment parameters
int UnpackInstPara(uchar *psPara)
{
	uchar					ucNum;
	TMS_INSTALMENT_PLAN		*pstPlan;

	if( glSysParam.ucPlanNum >= MAX_PLAN )
	{
		PubDispString("MAX # OF PLAN", 4|DISP_LINE_LEFT);	// "分期计划溢出"
		PubWaitKey(2);
		return ERR_NO_DISP;
	}

	pstPlan = (TMS_INSTALMENT_PLAN *)psPara;
	ucNum   = glSysParam.ucPlanNum;
	glSysParam.stPlanList[ucNum].ucIndex = pstPlan->ucIndex;
	glSysParam.stPlanList[ucNum].ucAcqIndex = pstPlan->ucAcqIndex;
	sprintf((char *)glSysParam.stPlanList[ucNum].szName, "%.7s", pstPlan->sName);
	glSysParam.stPlanList[ucNum].ucMonths = (uchar)PubBcd2Long(&pstPlan->ucMonths, 1, NULL);
	PubBcd2Long(pstPlan->sBottomAmt, 6, &glSysParam.stPlanList[ucNum].ulBottomAmt);
	glSysParam.ucPlanNum++;

	return 0;
}

// save product descriptors
int UnpackDescPara(uchar *psPara)
{
	uchar				ucNum;
	TMS_DESCRIPTOR		*pstDesc;

	if( glSysParam.ucDescNum >= MAX_DESCRIPTOR )
	{
		PubDispString(_T("MAX # OF DESC"), 4|DISP_LINE_LEFT);
		PubWaitKey(2);
		return ERR_NO_DISP;
	}

	pstDesc = (TMS_DESCRIPTOR *)(psPara+1);
	ucNum   = glSysParam.ucDescNum;
	glSysParam.stDescList[ucNum].ucKey = pstDesc->ucKey;
	sprintf((char *)glSysParam.stDescList[ucNum].szCode, "%.2s", pstDesc->sCode);
	sprintf((char *)glSysParam.stDescList[ucNum].szText, "%.20s", pstDesc->sText);
	glSysParam.ucDescNum++;

	return 0;
}

#ifdef ENABLE_EMV
// save EMV parameters
int SaveEmvMisc(uchar *psPara)
{
	TMS_EMV_MISC	*pstEmvMisc;

	pstEmvMisc = (TMS_EMV_MISC *)psPara;
	EMVGetParameter(&glEmvParam);

	memcpy(glEmvParam.CountryCode,   pstEmvMisc->sCourtryCode,  2);
	memcpy(glEmvParam.TransCurrCode, pstEmvMisc->sCurcyCode,    2);
	memcpy(glEmvParam.ReferCurrCode, pstEmvMisc->sRefCurcyCode, 2);
	glEmvParam.TransCurrExp = pstEmvMisc->ucCurcyExp;
	glEmvParam.ReferCurrExp = pstEmvMisc->ucRefCurcyExp;
//	pstEmvMisc->Language;	// Unused

	EMVSetParameter(&glEmvParam);
	return 0;
}
#endif

#ifdef ENABLE_EMV
// save EMV application
int SaveEmvApp(uchar *psPara)
{
	int				iRet;
	TMS_EMV_APP		*pstApp;
	EMV_APPLIST		stEmvApp;

	pstApp = (TMS_EMV_APP *)(psPara+1);
	memset(&stEmvApp, 0, sizeof(EMV_APPLIST));

	if( pstApp->bLocalName )
	{
		memcpy(stEmvApp.AppName, pstApp->sLocalName, (int)MIN(16, pstApp->ucLocalNameLen));
	}

	memcpy(stEmvApp.AID, pstApp->sAID, (int)MIN(16, pstApp->ucAIDLen));
	stEmvApp.AidLen   = pstApp->ucAIDLen;
	stEmvApp.SelFlag  = (pstApp->ucASI==0) ? PART_MATCH : FULL_MATCH;
//	stEmvApp.Priority = returned by card, not used here;

	stEmvApp.TargetPer       = pstApp->ucTargetPer;
	stEmvApp.MaxTargetPer    = pstApp->ucMaxTargetPer;
	if( sgNewTMS )
	{
		stEmvApp.FloorLimitCheck = ((pstApp->ucTermRisk & TRM_FLOOR_CHECK)!=0);
		stEmvApp.RandTransSel    = ((pstApp->ucTermRisk & TRM_RANDOM_TRAN_SEL)!=0);
		stEmvApp.VelocityCheck   = ((pstApp->ucTermRisk & TRM_VELOCITY_CHECK)!=0);
	}
	else
	{	// 旧版协议不下载这些数据
		stEmvApp.FloorLimitCheck = 1;
		stEmvApp.RandTransSel    = 1;
		stEmvApp.VelocityCheck   = 1;
	}

	PubChar2Long(pstApp->sFloorLimit, 4, &stEmvApp.FloorLimit);
	stEmvApp.FloorLimit *= 100;		// floor 以元为单位
	PubChar2Long(pstApp->sThreshold,  4, &stEmvApp.Threshold);

	memcpy(stEmvApp.TACDenial,  pstApp->sTACDenial,  5);
	memcpy(stEmvApp.TACOnline,  pstApp->sTACOnline,  5);
	memcpy(stEmvApp.TACDefault, pstApp->sTACDefault, 5);
//	memcpy(stEmvApp.AcquierId, // not set here

	stEmvApp.dDOL[0] = pstApp->ucTermDDOLLen;
	memcpy(&stEmvApp.dDOL[1], pstApp->sTermDDOL, stEmvApp.dDOL[0]);
	if( sgNewTMS )
	{
		stEmvApp.tDOL[0] = strlen((char *)pstApp->sTDOL)/2;
		PubAsc2Bcd(pstApp->sTDOL, (uint)stEmvApp.tDOL[0], &stEmvApp.tDOL[1]);
	}
	memcpy(stEmvApp.Version, pstApp->sAppVer, 2);
//	stEmvApp.RiskManData

	iRet = EMVAddApp(&stEmvApp);
	if( iRet!=EMV_OK )
	{
		ScrClrLine(4, 7);
		PubDispString(_T("ERR SAVE EMV APP"), 4|DISP_LINE_LEFT);
		PubWaitKey(2);
		return iRet;
	}

	return 0;
}
#endif

#ifdef ENABLE_EMV
// save CAPK
int SaveEmvCapk(uchar *psPara)
{
	int				iRet;
	TMS_EMV_CAPK	*pstCapk;
	EMV_CAPK		stEmvCapk;

	pstCapk = (TMS_EMV_CAPK *)(psPara+1);
	memset(&stEmvCapk, 0, sizeof(EMV_CAPK));

	memcpy(stEmvCapk.RID, pstCapk->sRID, 5);
	stEmvCapk.KeyID    = pstCapk->ucKeyID;
	stEmvCapk.HashInd  = pstCapk->ucHashInd;
	stEmvCapk.ArithInd = pstCapk->ucArithInd;
	stEmvCapk.ModulLen = pstCapk->ucModulLen;
	memcpy(stEmvCapk.Modul, pstCapk->sModul, stEmvCapk.ModulLen);
	stEmvCapk.ExponentLen = pstCapk->ucExpLen;
	memcpy(stEmvCapk.Exponent, pstCapk->sExponent, stEmvCapk.ExponentLen);
// PubTRACE3("%02X %02X %02X", pstCapk->sExpiry[0], pstCapk->sExpiry[1], pstCapk->sExpiry[2]);

	if( memcmp(pstCapk->sExpiry, "\x00\x00\x00", 3)!=0 )
	{
		memcpy(stEmvCapk.ExpDate, pstCapk->sExpiry, 3);
	}
	else
	{
		memcpy(stEmvCapk.ExpDate, "\x20\x12\x31", 3); // ????
	}
	memcpy(stEmvCapk.CheckSum, pstCapk->sCheckSum, 20);

	iRet = EMVAddCAPK(&stEmvCapk);
	if( iRet!=EMV_OK )
	{
		ScrClrLine(4, 7);
		PubDispString(_T("ERR SAVE CAPK"), 4|DISP_LINE_LEFT);
		PubWaitKey(3);
//		return iRet;
	}

	return 0;
}
#endif

// parameter analyse control
int UnpackPara(uchar *psParaData, long lDataLen)
{
	uchar	ucSubType, ucTemp, *psCurPtr, *psBack;
	ulong	ulSubFieldLen;

	psCurPtr = psParaData;
	while( psCurPtr<(psParaData+lDataLen) )
	{
		ucSubType = *psCurPtr;
		psCurPtr++;

		// 长度超过9999字节时,bcd码第一个字节是0xFn,F表示长度超过9999,n表示后面有几个字节的bcd码
		// when length > 9999, first byte is 0xFn, "F" means >9999, "n" indicates number of following bytes
		if( (*psCurPtr & 0xF0)==0xF0 )
		{
			PubBcd2Long(psCurPtr+1, (uint)(*psCurPtr & 0x0F), &ulSubFieldLen);
			psCurPtr += (*psCurPtr & 0x0F) + 1;
		}
		else
		{
			PubBcd2Long(psCurPtr, 2, &ulSubFieldLen);
			psCurPtr += 2;
		}
		psBack = psCurPtr;
		ucTemp = (uchar)PubBcd2Long(psCurPtr, 1, NULL);
		if( ucTemp>0 )
		{
			ucTemp--;
		}

#ifdef ENABLE_EMV
		// If any EMV is downloaded, clear the old data
		switch(ucSubType)
		{
		case 10:
		case 11:
		case 12:
			if (!sgEMVDownloaded)
			{
				sgEMVDownloaded = 1;
				RemoveEmvAppCapk();
			}
			break;
		default:
			break;	
		}
#endif

		switch(ucSubType)
		{
		case 1:
			UnpackParaEdc(psCurPtr);
			break;

		case 2:
			if( UnpackParaCard(ucTemp, psCurPtr+1)!=0 )
			{
				return 1;
			}
			break;

		case 3:
			if( UnpackParaIssuer(ucTemp, psCurPtr+1)!=0 )
			{
				return 2;
			}
			break;

		case 4:
			if( UnpackParaAcq(ucTemp, psCurPtr+1, ulSubFieldLen)!=0 )
			{
				return 3;
			}
			break;

		case 5:
			if( UnpackDescPara(psCurPtr)!=0 )
			{
				return 4;
			}
			break;

		case 7:
			if( UnpackEPSPara(psCurPtr)!=0 )
			{
				return 5;
			}
			break;
		case 9:
			if( UnpackInstPara(psCurPtr)!=0 )//2015-12-16 AMEX EPP: mark! 1)
			{
				return 6;
			}
			break;

		case 10:
#ifdef ENABLE_EMV
			if( SaveEmvMisc(psCurPtr)!=0 )
			{
				return 7;
			}
#endif
			break;

		case 11:
#ifdef ENABLE_EMV
			if( SaveEmvApp(psCurPtr)!=0 )
			{
				return 8;
			}
#endif
			break;

		case 12:
#ifdef ENABLE_EMV
			if( SaveEmvCapk(psCurPtr)!=0 )
			{
				return 9;
			}
#endif
			break;

		case 20:
			if( SaveCardBin(psCurPtr)!=0 )
			{
				 return 20;
			}
			break;

		case 21:
		case 22:
			if( ulSubFieldLen>sizeof(glSysParam.sAdData) )
			{
				return 21;
			}
			glSysParam.bTextAdData = (ucSubType==21) ? TRUE : FALSE;
			memcpy(glSysParam.sAdData, psCurPtr, (int)ulSubFieldLen);
			break;

		case 23:
			memcpy(glSysParam.stEdcInfo.szCallInTime, psCurPtr, 8);
			break;

		default:
			break;
		}
		psCurPtr = psBack+ulSubFieldLen;
	}

	return 0;
}

// 生成每一个 acq 的sIssuerKey, TREU: 成功, FALSE: 失败
// Generates sIssuerKey for every acquirer.
uchar SearchIssuerKeyArray(void)
{
	uchar	ucCnt, ucFlag;

	ucFlag = FALSE;
	for(ucCnt=0; ucCnt<glSysParam.ucAcqNum; ucCnt++)
	{
		if( (glSysParam.stAcqList[ucCnt].ucKey!=INV_ACQ_KEY) &&
			(glSysCtrl.sAcqStatus[ucCnt]==S_USE) )
		{
			ucFlag = TRUE;
			memset(glSysParam.stAcqList[ucCnt].sIssuerKey, (uchar)INV_ISSUER_KEY, MAX_ISSUER);
			SearchIssuerKeyArraySub(glSysParam.stAcqList[ucCnt].sIssuerKey, glSysParam.stAcqList[ucCnt].ucKey);
		}
	}

	return ucFlag;
}

void AfterLoadParaProc(void)
{
	glSysParam.stEdcInfo.uiInitFlag = EDC_INIT_OK;
	glSysParam.ucTermStatus         = TRANS_MODE;
	if( glSysParam.stEdcInfo.bClearBatch )
	{
		ClearRecord(ACQ_ALL);
          PPDCC_ResetTransStatistic(); // PP-DCC
	}
	if( memcmp(glSysParam.stEdcInfo.sInitSTAN, "\x00\x00\x00", 3)!=0 )
	{
		PubBcd2Long(glSysParam.stEdcInfo.sInitSTAN, 3, &glSysCtrl.ulSTAN);
		PubBcd2Long(glSysParam.stEdcInfo.sInitSTAN, 3, &glSysCtrl.ulInvoiceNo);
	}
    
    // PP-DCC
	if (PPDCC_SetCurAcq()==0)
	{
          PPDCC_SetDccDefaults();
	}

	// ICBC HK/MACAU use HEX header
	if (FindAcqIdxByName("ICBC", FALSE)<MAX_ACQ)
	{
		glSysParam.stTMSCommCfg.ucTCPClass_BCDHeader = FALSE;
		glSysParam.stTxnCommCfg.ucTCPClass_BCDHeader = FALSE;
	}

	SaveSysCtrlAll();
	SaveSysParam();
	SyncPassword();

#ifdef ENABLE_EMV
	SyncEmvCurrency(glSysParam.stEdcInfo.stLocalCurrency.sCountryCode, glSysParam.stEdcInfo.stLocalCurrency.sCurrencyCode, glSysParam.stEdcInfo.stLocalCurrency.ucDecimal);
#endif
    
    if (memcmp(glSysParam.stEdcInfo.stLocalCurrency.sCountryCode, "\x00\x00", 2)==0)
    {
        PubShowTitle(TRUE, "");
        PubDispString(_T("COUNTRY CODE"), 4|DISP_LINE_LEFT);
        PubDispString(_T("MISSING"),      6|DISP_LINE_LEFT);
        PubWaitKey(USER_OPER_TIMEOUT);
    }
}

void SearchIssuerKeyArraySub(uchar *sIssuerKey, uchar ucAcqKey)
{
	uchar	ucCnt, sTempKey[256];
	int		iTemp, iKeyNum;

	memset(sTempKey, (uchar)INV_ISSUER_KEY, sizeof(sTempKey));
	for(ucCnt=0; ucCnt<glSysParam.ucCardNum; ucCnt++)
	{
		if( glSysParam.stCardTable[ucCnt].ucAcqKey==ucAcqKey &&
			glSysParam.stCardTable[ucCnt].ucAcqKey!=INV_ACQ_KEY )
		{	// 消除重复,因为可能一个issuer对多个card
			sTempKey[glSysParam.stCardTable[ucCnt].ucIssuerKey] = glSysParam.stCardTable[ucCnt].ucIssuerKey;
		}
	}

	for(iKeyNum=iTemp=0; iTemp<256; iTemp++)
	{
		if( sTempKey[iTemp]!=INV_ISSUER_KEY )
		{
			sIssuerKey[iKeyNum] = sTempKey[iTemp];
			iKeyNum++;
		}
	}
}

// get TMS telepnone number
int GetDownLoadTelNo(void)
{
	uchar	ucRet, szBuff[30];

	ScrClrLine(2, 7);
	PubDispString(_T("PHONE NO"), 2|DISP_LINE_LEFT);
	sprintf((char *)szBuff, "%.25s", glSysParam.stEdcInfo.szDownTelNo);
	ucRet = PubGetString(ALPHA_IN|ECHO_IN, 1, 25, szBuff, USER_OPER_TIMEOUT);
	if( ucRet!=0 )
	{
		return ERR_USERCANCEL;
	}

	sprintf((char *)glSysParam.stEdcInfo.szDownTelNo, "%.25s", szBuff);
	SaveEdcParam();

	return 0;
}

int GetDownLoadGprsPara(void)
{
	uchar	ucRet = SetWirelessParam(&glSysParam._TmsWirlessPara);
	SyncWirelessParam(&glSysParam._TxnWirlessPara, &glSysParam._TmsWirlessPara);

	return ucRet;
}

int GetDownLoadLanPara(void)
{
	uchar	ucRet;

	ScrCls();
	PubDispString("SETUP TCPIP", DISP_LINE_REVER);

	ucRet = SetTcpIpParam(&glSysParam._TmsTcpIpPara);
	if (ucRet==0)
	{
		SyncTcpIpParam(&glSysParam._TxnTcpIpPara, &glSysParam._TmsTcpIpPara);
		return 0;
	}

	return ucRet;
}

int GetDownLoadWIFIPara(void)
{
	return -1;
}

int GetDownLoadComm(uchar ucCommType)
{
	int		iRet;

	switch(ucCommType)
	{
	case CT_RS232:
		iRet = 0;
		break;
	case CT_MODEM:
		iRet = GetDownLoadTelNo();
		break;
	case CT_TCPIP:
		iRet = GetDownLoadLanPara();
		if (iRet==0)
		{
			iRet = GetRemoteIp("PROTIMS ", FALSE, &glSysParam.stEdcInfo.stDownIpAddr);
		}
		break;
	case CT_CDMA:
	case CT_GPRS:
	case CT_3G://2015-11-23
		iRet = GetDownLoadGprsPara();
		if (iRet==0)
		{
			iRet = GetRemoteIp("PROTIMS ", FALSE, &glSysParam.stEdcInfo.stDownIpAddr);
		}
		break;
// 	case CT_WIFI:
// 		GetDownLoadWIFIPara();
// 		break;
	default:
		iRet = ERR_NO_DISP;
		break;
	}

	return iRet;
}

// get TMS download terminal ID
int GetDownLoadTID(uchar *pszID)
{
	uchar	ucRet, szBuff[8+1];

	while (1)
	{
		ScrClrLine(2, 7);
		PubDispString(_T("DOWNLOAD ID"), DISP_LINE_LEFT|3);
		sprintf((char *)szBuff, "%.8s", pszID);
		ucRet = PubGetString(NUM_IN|ECHO_IN, 8, 8, szBuff, USER_OPER_TIMEOUT);
		if( ucRet!=0 )
		{
			return ERR_USERCANCEL;
		}
		if (atol((char *)szBuff)!=0)
		{
			break;
		}
	}

	sprintf((char *)pszID, "%.8s", szBuff);
	return 0;
}

//void UpdateCommType(void)

// 加载参数文件
// Load downloaded EDC parameter file (downloaded by new Protims protocol)
int LoadSysDownEdcFile(void)
{
	int		fd, iRet, iLength, iLeftLenBytes;
	long	lOffset, lMaxOffset, lSubFieldLen;
	uchar	szCurTime[16+1], ucHeadLen;

	fd = open(DOWNPARA_FILE, O_RDWR);
	if( fd<0 )
	{
		return 1;
	}

	sgEMVDownloaded = 0;

	sgNewTMS = TRUE;
	GetEngTime(szCurTime);
	PubShowTitle(TRUE, szCurTime);
	PubDispString(_T("Loading Param..."), 4|DISP_LINE_LEFT);

	ResetAllPara(FALSE);

	lMaxOffset = filesize(DOWNPARA_FILE);
	lOffset    = 0L;
	while( lOffset<lMaxOffset )
	{
		seek(fd, lOffset, SEEK_SET);
		memset(sgTempBuf, 0, sizeof(sgTempBuf));
		iLength = read(fd, sgTempBuf, 3);
		if( iLength!=3 )
		{
			break;
		}

		// 长度超过9999字节时,bcd码第一个字节是0xFn,F表示长度超过9999,n表示后面有几个字节的bcd码
		// when length > 9999, first byte is 0xFn, "F" means >9999, "n" indicates number of following bytes
		if( (sgTempBuf[1] & 0xF0)==0xF0 )
		{
			iLeftLenBytes = (sgTempBuf[1] & 0x0F) - 1;
			if( iLeftLenBytes<=0 )
			{
				break;
			}
			iLength = read(fd, &sgTempBuf[3], iLeftLenBytes);
			if( iLength!=iLeftLenBytes )
			{
				break;
			}
			PubBcd2Long(&sgTempBuf[2], (uint)(iLeftLenBytes+1), (ulong *)&lSubFieldLen);
			ucHeadLen = (uchar)(2+(sgTempBuf[1] & 0x0F));
		}
		else
		{
			PubBcd2Long(&sgTempBuf[1], 2, (ulong *)&lSubFieldLen);
			ucHeadLen = 3;
		}
		if( lSubFieldLen+ucHeadLen+1>sizeof(sgTempBuf) )	// reserved a byte to keep '\0'
		{
			PubTRACE1("SubFieldLen:%ld", lSubFieldLen);
			break;
		}

		iLength = read(fd, &sgTempBuf[ucHeadLen], (int)lSubFieldLen);
		if( iLength!=(int)lSubFieldLen )
		{
			PubTRACE1("iLength:%d", iLength);
			break;
		}

		iRet = UnpackPara(sgTempBuf, lSubFieldLen+ucHeadLen);
		if( iRet!=0 )
		{
			PubTRACE1("unpack:%d", iRet);
			break;
		}
		lOffset += (lSubFieldLen + ucHeadLen);
	}
	close(fd);

	if( !(lOffset>=lMaxOffset && SearchIssuerKeyArray()) )
	{
		remove(DOWNPARA_FILE);
		GetEngTime(szCurTime);
		PubShowTitle(TRUE, szCurTime);
		PubDispString(_T("Loading Failed!"), 4|DISP_LINE_LEFT);
		PubWaitKey(5);
		return ERR_TRAN_FAIL;
	}

	AfterLoadParaProc();
	remove(DOWNPARA_FILE);

	GetEngTime(szCurTime);
	PubShowTitle(TRUE, szCurTime);
	PubDispString(_T("Loading Success!"), 4|DISP_LINE_LEFT);
	PubWaitKey(3);

    // PP-DCC
    if (PPDCC_ChkIfHaveDccAcq())
    {
        PPDCC_PromptDownloadBin();
    }

	return 0;
}

// 产生下一个自动下载时间
// To figure when will be the next time to download auto
void GetNextAutoDayTime(uchar *pszDateTime, ushort uiInterval)
{
	uchar	szBuff[50];
	int		iYear, iMonth, iDate, iHour, iMinute, iMaxDay;

	PubGetDateTime(szBuff);
	iYear  = (int)PubAsc2Long(&szBuff[2], 2, NULL);
	iMonth = (int)PubAsc2Long(&szBuff[4], 2, NULL);
	iDate  = (int)PubAsc2Long(&szBuff[6], 2, NULL);
	srand((uint)PubAsc2Long(&szBuff[10], 4, NULL));
	iHour   = rand()%6;		// 自动下载时间的hour必须是 0-5之间
	iMinute = rand()%60;	// 自动下载时间的minute必须是 0-59之间
	if( uiInterval>999 )
	{
		uiInterval = 999;
	}

	iDate += uiInterval;
	while( 1 )
	{
		switch( iMonth )
		{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			iMaxDay = 31;
			break;

		case 2:
			iMaxDay = 29;
			break;

		default:
			iMaxDay = 30;
			break;
		}

		if( iDate<=iMaxDay )
		{
			break;
		}
		iDate -= iMaxDay;
		iMonth++;

		if( iMonth>12 )
		{
			iMonth -= 12;
			iYear++;
		}
	}	// while( 1

	sprintf((char *)pszDateTime, "%02d%02d%02d%02d%02d", iYear%100, iMonth,
			iDate, iHour, iMinute);
}

void GetOneLine(uchar **psCurPtr, uchar *psData, int Maxlen)
{
#define ISSPACE(ch) ( ((ch)==' ')  || ((ch)=='\t') || \
					  ((ch)=='\n') || ((ch)=='\r') )
#define ISLINEEND(ch) ( ((ch)=='\n') || ((ch)=='\r') )
	uchar	*p, *q;

	for(p=*psCurPtr; *p && ISSPACE(*p); p++);

	*psData = 0;
	for(q=psData; *p && (q-psData<Maxlen) && !ISLINEEND(*p); )	*q++ = *p++;
	*q = 0;
	PubTrimStr(psData);

	for(; *p && !ISLINEEND(*p); p++);  // erase characters of the left lines
	*psCurPtr = p;
#undef ISSPACE
#undef ISLINEEND
}

int SaveDetail(uchar *psData)
{
	uchar	*psCurPtr, *psBack, ucLen, ucCmpLen;
	uchar	szBuf[80], szStartNo[20+1], szEndNo[20+1];
	ushort	uiIndex;

	if( glSysParam.uiCardBinNum >= MAX_CARDBIN_NUM )
	{
		PubDispString("CARDBIN OVERFLOW", 4|DISP_LINE_LEFT);	// "卡 表 溢 出"
		PubWaitKey(5);
		return 1;
	}

	psBack   = psData;
	psCurPtr = (uchar *)strchr((char *)psBack, ',');
	if( psCurPtr==NULL )
	{
		return 1;
	}
	sprintf((char *)szBuf, "%.*s", (int)MIN(psCurPtr-psBack, 9), psBack);
	ucLen = (uchar)atol((char *)szBuf);

	psBack   = psCurPtr+1;
	psCurPtr = (uchar *)strchr((char *)psBack, ',');
	if( psCurPtr==NULL )
	{
		return 1;
	}
	sprintf((char *)szStartNo, "%.*s", (int)MIN(psCurPtr-psBack, 19), psBack);
	sprintf((char *)szEndNo,   "%.*s", (int)MIN(psCurPtr-psBack, 19), psCurPtr+1);
	if( strlen((char *)szStartNo)!=strlen((char *)szEndNo) )
	{
		return 1;
	}

	// save card bin record
	uiIndex = glSysParam.uiCardBinNum;
	glSysParam.stCardBinTable[uiIndex].ucPanLen      = ucLen;
	ucCmpLen = (uchar)strlen((char *)szStartNo);
	glSysParam.stCardBinTable[uiIndex].ucMatchLen    = ucCmpLen;
	glSysParam.stCardBinTable[uiIndex].ucIssuerIndex = (uchar)glSysParam.uiIssuerNameNum;

	memset(glSysParam.stCardBinTable[uiIndex].sStartNo, 0, sizeof(glSysParam.stCardBinTable[uiIndex].sStartNo));
	memset(glSysParam.stCardBinTable[uiIndex].sEndNo, (uchar)0xFF, sizeof(glSysParam.stCardBinTable[uiIndex].sEndNo));
	if( ucCmpLen % 2 )	// 卡号长度为奇数
	{
		szStartNo[ucCmpLen] = '0';
		PubAsc2Bcd(szStartNo, (uint)(ucCmpLen+1), glSysParam.stCardBinTable[uiIndex].sStartNo);
		szEndNo[ucCmpLen]   = 'F';
		PubAsc2Bcd(szEndNo,   (uint)(ucCmpLen+1), glSysParam.stCardBinTable[uiIndex].sEndNo);
	}
	else
	{
		PubAsc2Bcd(szStartNo, (uint)ucCmpLen, glSysParam.stCardBinTable[uiIndex].sStartNo);
		PubAsc2Bcd(szEndNo,   (uint)ucCmpLen, glSysParam.stCardBinTable[uiIndex].sEndNo);
	}

	glSysParam.uiCardBinNum++;

	return 0;
}

// save card bin table for HK
int SaveCardBin(uchar *psCardBin)
{
	uchar	*psCurPtr, *psBack, ucFlag;
	uchar	szBuf[80], szChineseName[16+1], szEnglishName[MAX_CARBIN_NAME_LEN+1];
	ushort	uiIndex;

	psCurPtr = psCardBin;
	while( *psCurPtr )
	{
		if( glSysParam.uiIssuerNameNum >= MAX_CARDBIN_ISSUER )
		{
			return 1;
		}

		// search Tag [Issuer]
		GetOneLine(&psCurPtr, szBuf, sizeof(szBuf)-1);
		if( PubStrNoCaseCmp((uchar *)szBuf, (uchar *)"[Issuer]")!=0 )
		{
			continue;
		}

		// get Chinese name
		psBack = psCurPtr;
		GetOneLine(&psCurPtr, szBuf, sizeof(szBuf)-1);
		if(PubStrNoCaseCmp((uchar *)szBuf, (uchar *)"[Issuer]")==0 )
		{
			psCurPtr = psBack;
			continue;
		}
		sprintf((char *)szChineseName, "%.16s", szBuf);

		// get English name
		psBack = psCurPtr;
		GetOneLine(&psCurPtr, szBuf, sizeof(szBuf)-1);
		if( PubStrNoCaseCmp((uchar *)szBuf, (uchar *)"[Issuer]")==0 )
		{
			psCurPtr = psBack;
			continue;
		}
		sprintf((char *)szEnglishName, "%.*s", MAX_CARBIN_NAME_LEN, szBuf);

		// get details
		ucFlag = 0;
		while( *psCurPtr )
		{
			psBack = psCurPtr;
			GetOneLine(&psCurPtr, szBuf, sizeof(szBuf)-1);
			if( (szBuf[0]==0) || (PubStrNoCaseCmp((uchar *)szBuf, (uchar *)"[Issuer]")==0) )
			{
				psCurPtr = psBack;
				break;
			}
			if( SaveDetail(szBuf)!=0 )
			{
				return 1;
			}
			ucFlag = 1;
		}
		if( !ucFlag )	// ignore null detail lines!
		{
			continue;
		}

		// save Chinese/English name of issuer
		uiIndex = glSysParam.uiIssuerNameNum;
		sprintf((char *)glSysParam.stIssuerNameList[uiIndex].szChineseName, "%.16s", szChineseName);
		sprintf((char *)glSysParam.stIssuerNameList[uiIndex].szEnglishName, "%.*s", MAX_CARBIN_NAME_LEN, szEnglishName);
		glSysParam.uiIssuerNameNum++;
	}

	return 0;
}

void LoadEdcLang(void)
{
	SetSysLang(1);
}

void LoadDefaultLang(void)
{
	SetSysLang(0);
}


#if defined(_WIN32)
#if defined(_P70_S_) || defined(_P60_S1_)
int RemoteLoadApp(T_INCOMMPARA *ptCommPara)
{
	PubDispString(_T("NOT IMPLEMENT"), 4|DISP_LINE_LEFT);
	PubWaitKey(3);

	return -1;
}
#endif
#endif

#ifdef _WIN32
int GetLoadedAppStatus(uchar *psAppName, TMS_LOADSTATUS *ptStat)
{
	PubDispString(_T("NOT IMPLEMENT"), 4|DISP_LINE_LEFT);
	PubWaitKey(3);

	return -1;
}
#endif

#ifdef ALLOW_NO_TMS
// 无下载初始化进程
void InitEdcParam(void)
{
	CURRENCY_CONFIG		stLocalCurrency;

	GetDefCurrency(&stLocalCurrency);
	sprintf((char *)glSysParam.stEdcInfo.szMerchantName, "MERCHANT NAME");
	sprintf((char *)glSysParam.stEdcInfo.szMerchantAddr, "MERCHANT ADDR");
	sprintf((char *)&glSysParam.stEdcInfo.szDownLoadTID, "00000000");
	memcpy(glSysParam.stEdcInfo.sOption, "\xE6\x28\x00\x09\x00", 5);
	glSysParam.stEdcInfo.ucTranAmtLen     = 10;
	glSysParam.stEdcInfo.ucStlAmtLen      = 12;

	// 这里先设默认。之后会有一个手动修改的机会
	glSysParam.stEdcInfo.stLocalCurrency = stLocalCurrency;
	glSysParam.stEdcInfo.ucCurrencySymbol = ' ';

#if defined(_P60_S1_) || defined(_P70_S_)
	glSysParam.stEdcInfo.ucPrinterType = 0;
#else
	if (ChkHardware(HWCFG_PRINTER, 'S')==TRUE)
	{
		glSysParam.stEdcInfo.ucPrinterType = 0;
	}
	else
	{
		glSysParam.stEdcInfo.ucPrinterType = 1;
	}
#endif

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// set acquirer parameters
	glSysCtrl.sAcqStatus[0] = S_USE;
	glSysParam.stAcqList[0].ucKey   = 0x01;
	glSysParam.stAcqList[0].ucIndex = 0;
	sprintf((char *)glSysParam.stAcqList[0].szNii, "610");
	sprintf((char *)glSysParam.stAcqList[0].szName, "BEA");
	sprintf((char *)glSysParam.stAcqList[0].szPrgName, "VISA");
	sprintf((char *)glSysParam.stAcqList[0].szTermID, "31686666");
	sprintf((char *)glSysParam.stAcqList[0].szMerchantID, "012800000009000");
	memcpy(glSysParam.stAcqList[0].sOption, "\x06\x20\x0E\x00", 4); //"\x06\x20\x0C\x00" unsupport EMV "\x06\x20\x0E\x00"Supprot EMV
	sprintf((char *)glSysParam.stAcqList[0].TxnTelNo1, "90085239205305");
	sprintf((char *)glSysParam.stAcqList[0].TxnTelNo2, "90085239205305");
	sprintf((char *)glSysParam.stAcqList[0].StlTelNo1, "90085239205305");
	sprintf((char *)glSysParam.stAcqList[0].StlTelNo2, "90085239205305");
	glSysParam.stAcqList[0].ulCurBatchNo = 1L;
	glSysParam.stAcqList[0].ulNextBatchNo = 2L;

	glSysParam.stAcqList[0].ucPhoneTimeOut = 100;
	glSysParam.stAcqList[0].ucTcpTimeOut   = 30;
	glSysParam.stAcqList[0].ucPppTimeOut   = 30;
	glSysParam.stAcqList[0].ucGprsTimeOut  = 30;

	memcpy(glSysParam.stAcqList[0].sIssuerKey, "\x01", 1);// visa master

	////////acquire 2///
	glSysCtrl.sAcqStatus[1] = S_USE;
	glSysParam.stAcqList[1].ucKey   = 0x02;
	glSysParam.stAcqList[1].ucIndex = 1;
	sprintf((char *)glSysParam.stAcqList[1].szNii, "007");
	sprintf((char *)glSysParam.stAcqList[1].szName, "DCD_BEA");
	sprintf((char *)glSysParam.stAcqList[1].szPrgName, "VISA");
	sprintf((char *)glSysParam.stAcqList[1].szTermID, "21700511");
	sprintf((char *)glSysParam.stAcqList[1].szMerchantID, "011603999000001");
	memcpy(glSysParam.stAcqList[1].sOption, "\x06\x20\x0E\x00", 4);
	sprintf((char *)glSysParam.stAcqList[1].TxnTelNo1, "90085229470417");
	sprintf((char *)glSysParam.stAcqList[1].TxnTelNo2, "90085229470417");
	sprintf((char *)glSysParam.stAcqList[1].StlTelNo1, "90085229470417");
	sprintf((char *)glSysParam.stAcqList[1].StlTelNo2, "90085229470417");
	glSysParam.stAcqList[1].ulCurBatchNo = 1L;
	glSysParam.stAcqList[1].ulNextBatchNo = 2L;
	
	glSysParam.stAcqList[1].ucPhoneTimeOut = 100;
	glSysParam.stAcqList[1].ucTcpTimeOut   = 30;
	glSysParam.stAcqList[1].ucPppTimeOut   = 30;
	glSysParam.stAcqList[1].ucGprsTimeOut  = 30;
	
	memcpy(glSysParam.stAcqList[1].sIssuerKey, "\x02\x03", 2);// visa master

	glSysParam.ucAcqNum = 2;

// 	// set acquirer parameters
// 	glSysCtrl.sAcqStatus[0] = S_USE;
// 	glSysParam.stAcqList[0].ucKey   = 0x01;
// 	glSysParam.stAcqList[0].ucIndex = 0;
// 	sprintf((char *)glSysParam.stAcqList[0].szNii, "007");
// 	sprintf((char *)glSysParam.stAcqList[0].szName, "CITI_TAXI01");
// 	sprintf((char *)glSysParam.stAcqList[0].szPrgName, "VISA");
// 	sprintf((char *)glSysParam.stAcqList[0].szTermID, "21700511");
// 	sprintf((char *)glSysParam.stAcqList[0].szMerchantID, "011603999000001");
// 	memcpy(glSysParam.stAcqList[0].sOption, "\x06\x20\x0E\x00", 4); //"\x06\x20\x0C\x00" unsupport EMV "\x06\x20\x0E\x00"Supprot EMV
// 	sprintf((char *)glSysParam.stAcqList[0].TxnTelNo1, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[0].TxnTelNo2, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[0].StlTelNo1, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[0].StlTelNo2, "0085229470417");
// 	glSysParam.stAcqList[0].ulCurBatchNo = 1L;
// 	glSysParam.stAcqList[0].ulNextBatchNo = 2L;
// 
// 	glSysParam.stAcqList[0].ucPhoneTimeOut = 100;
// 	glSysParam.stAcqList[0].ucTcpTimeOut   = 30;
// 	glSysParam.stAcqList[0].ucPppTimeOut   = 30;
// 	glSysParam.stAcqList[0].ucGprsTimeOut  = 30;
// 
// 	memcpy(glSysParam.stAcqList[0].sIssuerKey, "\x01", 1);// visa master
// 
// 	////////acquire 2///
// 	glSysCtrl.sAcqStatus[1] = S_USE;
// 	glSysParam.stAcqList[1].ucKey   = 0x02;
// 	glSysParam.stAcqList[1].ucIndex = 1;
// 	sprintf((char *)glSysParam.stAcqList[1].szNii, "007");
// 	sprintf((char *)glSysParam.stAcqList[1].szName, "CITI_TAXI02");
// 	sprintf((char *)glSysParam.stAcqList[1].szPrgName, "VISA");
// 	sprintf((char *)glSysParam.stAcqList[1].szTermID, "21700511");
// 	sprintf((char *)glSysParam.stAcqList[1].szMerchantID, "011603999000002");
// 	memcpy(glSysParam.stAcqList[1].sOption, "\x06\x20\x0E\x00", 4);
// 	sprintf((char *)glSysParam.stAcqList[1].TxnTelNo1, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[1].TxnTelNo2, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[1].StlTelNo1, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[1].StlTelNo2, "0085229470417");
// 	glSysParam.stAcqList[1].ulCurBatchNo = 1L;
// 	glSysParam.stAcqList[1].ulNextBatchNo = 2L;
// 	
// 	glSysParam.stAcqList[1].ucPhoneTimeOut = 100;
// 	glSysParam.stAcqList[1].ucTcpTimeOut   = 30;
// 	glSysParam.stAcqList[1].ucPppTimeOut   = 30;
// 	glSysParam.stAcqList[1].ucGprsTimeOut  = 30;
// 	
// 	memcpy(glSysParam.stAcqList[1].sIssuerKey, "\x02\x03", 2);// visa master
// 
// 
// 	////////acquire 3///
// 	glSysCtrl.sAcqStatus[2] = S_USE;
// 	glSysParam.stAcqList[2].ucKey   = 0x03;
// 	glSysParam.stAcqList[2].ucIndex = 1;
// 	sprintf((char *)glSysParam.stAcqList[2].szNii, "007");
// 	sprintf((char *)glSysParam.stAcqList[2].szName, "CITI_TAXI03");
// 	sprintf((char *)glSysParam.stAcqList[2].szPrgName, "VISA");
// 	sprintf((char *)glSysParam.stAcqList[2].szTermID, "21700511");
// 	sprintf((char *)glSysParam.stAcqList[2].szMerchantID, "011603999000003");
// 	memcpy(glSysParam.stAcqList[2].sOption, "\x06\x20\x0E\x00", 4);
// 	sprintf((char *)glSysParam.stAcqList[2].TxnTelNo1, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[2].TxnTelNo2, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[2].StlTelNo1, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[2].StlTelNo2, "0085229470417");
// 	glSysParam.stAcqList[2].ulCurBatchNo = 1L;
// 	glSysParam.stAcqList[2].ulNextBatchNo = 2L;
// 	
// 	glSysParam.stAcqList[2].ucPhoneTimeOut = 100;
// 	glSysParam.stAcqList[2].ucTcpTimeOut   = 30;
// 	glSysParam.stAcqList[2].ucPppTimeOut   = 30;
// 	glSysParam.stAcqList[2].ucGprsTimeOut  = 30;
// 	
// 	memcpy(glSysParam.stAcqList[2].sIssuerKey, "\x02\x03", 2);// visa master
// 
// 
// 	////////acquire 4///
// 	glSysCtrl.sAcqStatus[3] = S_USE;
// 	glSysParam.stAcqList[3].ucKey   = 0x04;
// 	glSysParam.stAcqList[3].ucIndex = 1;
// 	sprintf((char *)glSysParam.stAcqList[3].szNii, "007");
// 	sprintf((char *)glSysParam.stAcqList[3].szName, "CITI_TAXI04");
// 	sprintf((char *)glSysParam.stAcqList[3].szPrgName, "VISA");
// 	sprintf((char *)glSysParam.stAcqList[3].szTermID, "21700511");
// 	sprintf((char *)glSysParam.stAcqList[3].szMerchantID, "011603999000004");
// 	memcpy(glSysParam.stAcqList[3].sOption, "\x06\x20\x0E\x00", 4);
// 	sprintf((char *)glSysParam.stAcqList[3].TxnTelNo1, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[3].TxnTelNo2, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[3].StlTelNo1, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[3].StlTelNo2, "0085229470417");
// 	glSysParam.stAcqList[3].ulCurBatchNo = 1L;
// 	glSysParam.stAcqList[3].ulNextBatchNo = 2L;
// 	
// 	glSysParam.stAcqList[3].ucPhoneTimeOut = 100;
// 	glSysParam.stAcqList[3].ucTcpTimeOut   = 30;
// 	glSysParam.stAcqList[3].ucPppTimeOut   = 30;
// 	glSysParam.stAcqList[3].ucGprsTimeOut  = 30;
// 	
// 	memcpy(glSysParam.stAcqList[3].sIssuerKey, "\x02\x03", 2);// visa master
// 
// 
// 	////////acquire 5///
// 	glSysCtrl.sAcqStatus[4] = S_USE;
// 	glSysParam.stAcqList[4].ucKey   = 0x05;
// 	glSysParam.stAcqList[4].ucIndex = 1;
// 	sprintf((char *)glSysParam.stAcqList[4].szNii, "007");
// 	sprintf((char *)glSysParam.stAcqList[4].szName, "sder05");
// 	sprintf((char *)glSysParam.stAcqList[4].szPrgName, "VISA");
// 	sprintf((char *)glSysParam.stAcqList[4].szTermID, "21700511");
// 	sprintf((char *)glSysParam.stAcqList[4].szMerchantID, "011603999000005");
// 	memcpy(glSysParam.stAcqList[4].sOption, "\x06\x20\x0E\x00", 4);
// 	sprintf((char *)glSysParam.stAcqList[4].TxnTelNo1, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[4].TxnTelNo2, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[4].StlTelNo1, "0085229470417");
// 	sprintf((char *)glSysParam.stAcqList[4].StlTelNo2, "0085229470417");
// 	glSysParam.stAcqList[4].ulCurBatchNo = 1L;
// 	glSysParam.stAcqList[4].ulNextBatchNo = 2L;
// 	
// 	glSysParam.stAcqList[4].ucPhoneTimeOut = 100;
// 	glSysParam.stAcqList[4].ucTcpTimeOut   = 30;
// 	glSysParam.stAcqList[4].ucPppTimeOut   = 30;
// 	glSysParam.stAcqList[4].ucGprsTimeOut  = 30;
// 	
// 	memcpy(glSysParam.stAcqList[4].sIssuerKey, "\x02\x03", 2);// visa master
// 
// 	glSysParam.ucAcqNum = 5;


////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Mandy adds parameters for installment test
	glSysParam.ucPlanNum = 0;
	memcpy(glSysParam.stPlanList[0].szName, "PLAN01",8);
	glSysParam.stPlanList[0].ucAcqIndex = 0;
	glSysParam.stPlanList[0].ucIndex = 0;
	glSysParam.stPlanList[0].ucMonths = 12;
	glSysParam.stPlanList[0].ulBottomAmt = 100;
	
	memcpy(glSysParam.stPlanList[1].szName, "PLAN02",8);
	glSysParam.stPlanList[0].ucAcqIndex = 0;
	glSysParam.stPlanList[0].ucIndex = 1;
	glSysParam.stPlanList[0].ucMonths = 6;
	glSysParam.stPlanList[0].ulBottomAmt = 90;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// set issuer parameters
	glSysParam.stIssuerList[0].ucKey = 0x01;
	sprintf((char *)glSysParam.stIssuerList[0].szRefTelNo, "86");
	sprintf((char *)glSysParam.stIssuerList[0].szName, "VISA");
	memcpy(glSysParam.stIssuerList[0].sOption, "\xFC\x1E\x10\x00", 4);
	memcpy(glSysParam.stIssuerList[0].sPanMask, "\x00\xFF\xF0", 3);
	glSysParam.stIssuerList[0].ulFloorLimit    = 0;
	glSysParam.stIssuerList[0].ucAdjustPercent = 100;

	glSysParam.stIssuerList[1].ucKey = 0x02;
	sprintf((char *)glSysParam.stIssuerList[1].szRefTelNo, "86");
	sprintf((char *)glSysParam.stIssuerList[1].szName, "MASTER");
	memcpy(glSysParam.stIssuerList[1].sOption, "\xFC\x1E\x10\x00", 4);
	memcpy(glSysParam.stIssuerList[1].sPanMask, "\x00\xFF\xF0", 3);
	glSysParam.stIssuerList[1].ulFloorLimit    = 0;
	glSysParam.stIssuerList[1].ucAdjustPercent = 100;

	glSysParam.stIssuerList[2].ucKey = 0x03;
	sprintf((char *)glSysParam.stIssuerList[2].szRefTelNo, "86");
	sprintf((char *)glSysParam.stIssuerList[2].szName, "OTHER");
	memcpy(glSysParam.stIssuerList[2].sOption, "\xFC\x1E\x10\x00", 4);
	memcpy(glSysParam.stIssuerList[2].sPanMask, "\x00\xFF\xF0", 3);
	glSysParam.stIssuerList[2].ulFloorLimit    = 0;
	glSysParam.stIssuerList[2].ucAdjustPercent = 100;
	
	glSysParam.ucIssuerNum = 3;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// set card range
	memcpy(glSysParam.stCardTable[0].sPanRangeLow,  "\x00\x00\x00\x00\x00", 5);
	memcpy(glSysParam.stCardTable[0].sPanRangeHigh, "\x99\x99\x99\x99\x99", 5);
	glSysParam.stCardTable[0].ucIssuerKey = 0x01;
	glSysParam.stCardTable[0].ucAcqKey    = 0x01;
	glSysParam.stCardTable[0].ucPanLength = 0;
	glSysParam.stCardTable[0].ucOption    = 0x02;

	memcpy(glSysParam.stCardTable[1].sPanRangeLow,  "\x50\x00\x00\x00\x00", 5);
	memcpy(glSysParam.stCardTable[1].sPanRangeHigh, "\x59\x99\x99\x99\x99", 5);
	glSysParam.stCardTable[1].ucIssuerKey = 0x02;
	glSysParam.stCardTable[1].ucAcqKey    = 0x02;
	glSysParam.stCardTable[1].ucPanLength = 0;
	glSysParam.stCardTable[1].ucOption    = 0x02;

// 	memcpy(glSysParam.stCardTable[2].sPanRangeLow,  "\x00\x00\x00\x00\x00", 5);
// 	memcpy(glSysParam.stCardTable[2].sPanRangeHigh, "\x99\x99\x99\x99\x99", 5);
// 	glSysParam.stCardTable[2].ucIssuerKey = 0x01;
// 	glSysParam.stCardTable[2].ucAcqKey    = 0x03;
// 	glSysParam.stCardTable[2].ucPanLength = 0;
// 	glSysParam.stCardTable[2].ucOption    = 0x02;
// 
// 	memcpy(glSysParam.stCardTable[3].sPanRangeLow,  "\x00\x00\x00\x00\x00", 5);
// 	memcpy(glSysParam.stCardTable[3].sPanRangeHigh, "\x99\x99\x99\x99\x99", 5);
// 	glSysParam.stCardTable[3].ucIssuerKey = 0x01;
// 	glSysParam.stCardTable[3].ucAcqKey    = 0x04;
// 	glSysParam.stCardTable[3].ucPanLength = 0;
// 	glSysParam.stCardTable[3].ucOption    = 0x02;
// 
// 	memcpy(glSysParam.stCardTable[4].sPanRangeLow,  "\x00\x00\x00\x00\x00", 5);
// 	memcpy(glSysParam.stCardTable[4].sPanRangeHigh, "\x99\x99\x99\x99\x99", 5);
// 	glSysParam.stCardTable[4].ucIssuerKey = 0x01;
// 	glSysParam.stCardTable[4].ucAcqKey    = 0x05;
// 	glSysParam.stCardTable[4].ucPanLength = 0;
// 	glSysParam.stCardTable[4].ucOption    = 0x02;
// 
// 	memcpy(glSysParam.stCardTable[5].sPanRangeLow,  "\x50\x00\x00\x00\x00", 5);
// 	memcpy(glSysParam.stCardTable[5].sPanRangeHigh, "\x59\x99\x99\x99\x99", 5);
// 	glSysParam.stCardTable[5].ucIssuerKey = 0x02;
// 	glSysParam.stCardTable[5].ucAcqKey    = 0x02;
// 	glSysParam.stCardTable[5].ucPanLength = 0;
// 	glSysParam.stCardTable[5].ucOption    = 0x02;

	glSysParam.ucCardNum = 2;
}
#endif

// end of file

