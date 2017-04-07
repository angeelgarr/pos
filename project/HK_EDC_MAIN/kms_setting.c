#include "posapi.h"
#include "applib.h"
#include "kms_ex.h"
#include "ini.h"

#if 0
void printk(const char *str, ...)
{

    va_list varg;
    int retv;
    char sbuffer[20 * 1024];
    static int first_flag = 0;

	if (!first_flag)
	{
		PortOpen(0, "115200,8,N,1");
		first_flag = 1;
	}

	memset(sbuffer, 0, sizeof(sbuffer));
	va_start(varg, str);
	retv = vsprintf(sbuffer, (char *)str, varg);
	va_end(varg);
	strlen(sbuffer);
	PortSends(0, sbuffer, (ushort)strlen(sbuffer));
	DelayMs(100);

}
#endif

int KmsReadVar( const char *pszKey, char *pszValue,int iSize)
{
	int ret = 0;
	
	ret = IniReadString2(SECTION_CONFIG,pszKey,pszValue,iSize,"",KMSCFG);
	//printk("\r\n[KmsReadVar]----------------------------------------pszKey:%s pszValue:%s ret:%d\r\n",pszKey,pszValue,ret);
	return ret;
}
int KmsWriteVar(const char *pszKey, const char *pszValue)
{
	int ret = 0;
	
	ret = IniWriteString2(SECTION_CONFIG,pszKey,pszValue,KMSCFG);
	//printk("\r\n[KmsWriteVar]----------------------------------------pszKey:%s pszValue:%s ret:%d\r\n",pszKey,pszValue,ret);
	return ret;
}


int KmsRemoteLoadApp(const T_INCOMMPARA *ptCommPara)
{
	int iRet = 0;
	
	iRet = KmsInit();
	
	if (iRet = KmsLoadLanguage())
		return DisplayErrorMessage(iRet);
 
	// download
	iRet = KmsDownload((T_INCOMMPARA *)ptCommPara);
	if (iRet != PROTIMS_SUCCESS)	//download failed
		return DisplayErrorMessage(iRet);

	return PROTIMS_SUCCESS;
}

int KmsSetDefaultCfg(void)
{
	KmsWriteVar(PARA_APPNAME, "");// 应用名，若为空则下载所有的应用及参数、数据文件
	KmsWriteVar(PARA_LOADTYPE, "255");// 下载类型，bit0:应用，bit1:参数文件，bit2:数据文件 default:0x27=39
	KmsWriteVar(PARA_PROTOCOL, "0");// 协议信息：0-protims；1-ftims；2-tims
	KmsWriteVar(PARA_CALLMODE, "0");// 0: Monitor called, not enable
	KmsWriteVar(COM_BAUD_RATE, "115200,8,n,1");//串口通讯参数
	KmsWriteVar(MODEM_TIMEOUT, "4");

	//	// from mointor  设置Modem默认参数
	// "\x00\x00\x05\x05\x64\x06\xe7\x01\x00\xd0", 10); // 33600bps
	KmsWriteVar(MODEM_PARA_DP, "0");//0x00
	KmsWriteVar(MODEM_PARA_CHDT, "0");//0x00
	KmsWriteVar(MODEM_PARA_DT1, "5");//0x05
	KmsWriteVar(MODEM_PARA_DT2, "5");//0x05
	KmsWriteVar(MODEM_PARA_HT, "100");//0x64 = 100
	KmsWriteVar(MODEM_PARA_WT, "6");//0x06
	KmsWriteVar(MODEM_PARA_SSETUP, "231");//0xe7 = 231
	KmsWriteVar(MODEM_PARA_DTIMES, "1");//0x01
	KmsWriteVar(MODEM_PARA_TIMEOUT, "0");//0x00
	KmsWriteVar(MODEM_PARA_ASMODE, "208");//0xd0 = 208
	
	//睲ㄤウ硄獺把计
	KmsWriteVar(PARA_COMMODE,"");
	KmsWriteVar(PARA_TID,"00000000");
	//Modem
	KmsWriteVar(MODEM_OUTSIDE_NUM,"");
	KmsWriteVar(MODEM_PHONE_NUM,"");

	//GPRS
	KmsWriteVar(GPRS_APN, "");
	KmsWriteVar(GPRS_USERNAME, "");
	KmsWriteVar(GPRS_PASSWORD, "");
	KmsWriteVar(GPRS_REMOTE_IP, "");
	KmsWriteVar(GPRS_REMOTE_PORT, "");
	KmsWriteVar(GPRS_PIN_CODE, "");

	//CDMA
	KmsWriteVar(CDMA_PHONE_NUM, "");
	KmsWriteVar(CDMA_USERNAME, "");
	KmsWriteVar(CDMA_PASSWORD, "");
	KmsWriteVar(CDMA_REMOTE_IP, "");
	KmsWriteVar(CDMA_REMOTE_PORT, "");
	KmsWriteVar(CDMA_PIN_CODE, "");

	//TCP
	KmsWriteVar(TCP_REMOTE_IP, "");
	KmsWriteVar(TCP_REMOTE_PORT, "");
	KmsWriteVar(TCP_ENABLE_DHCP,"");
	KmsWriteVar(TCP_LOCAL_IP, "");
	KmsWriteVar(TCP_NETMASK, "");
	KmsWriteVar(TCP_GATEWAY, "");

	//set flag
	KmsWriteVar(PARA_SET_DEFAULT, "1");
	
	return PROTIMS_SUCCESS;
}

int KmsSetLanguage(int LangType)
{
	int iRet = 0;
	
	if(LangType == PROTIMS_PROMPT_EN)
		iRet = KmsWriteVar(PARA_CURRENT_LANG, "0");
	else if(LangType == PROTIMS_PROMPT_CN)
		iRet = KmsWriteVar(PARA_CURRENT_LANG, "1");
	else
		iRet = KmsWriteVar(PARA_CURRENT_LANG, "0");
	
	return iRet;
}
int DisplayErrorMessage(int errCode)
{
	int err[2];

	ScrClrLine(2,7);
	switch(errCode)
	{
	case KMS_INIT_COMM_ERROR:
		ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[COMM_INIT_ERROR]);
		break;
	case KMS_HANDSHAKE_ERROR:
		ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[HANDSHAKE_ERROR]);
		break;
	case KMS_RSAKEY_GET_ERROR:
		ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[RSAKEY_GET_ERROR]);
		break;
	case KMS_MAINKEY_GET_ERROR:
		ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[MAINKEY_GET_ERROR]);
		break;	
	case KMS_MAINKEY_WRITE_ERROR:
		ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[MAINKEY_WRITE_ERROR]);
		break;		
	case KMS_KEY_DOWNLOAD_ERROR:
		ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[KEY_DOWNLOAD_ERROR]);
		break;
	default:
		break;
	}

	KmsGetLastErr(err);
	if (err[0] >= PROTIMS_MODEM_BASE && err[0] < PROTIMS_MODEM_END)	// Modem error
	{
		switch(err[0])
		{
		case 0x05:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[NO_RESPONSE]);
			break;
		case 0x0d:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[LINE_BUSY]);
			break;
		case 0x33:
		case 0x03:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[NO_PHONE_LINE]);
			break;
		case 0xfd:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[USER_CANCEL]);
			break;			
		default:
			break;
		}
	}
	else if (err[0] >= PROTIMS_SERIAL_BASE && err[0] < PROTIMS_PORT_END)	// Serial error
	{
		switch(err[0])
		{
		case PROTIMS_PORTOPEN:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[PORT_OPEN_ERR]);
			break;
		case PROTIMS_PORTSEND:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[PORT_SEND_ERR]);
			break;
		case PROTIMS_PORTRESET:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[PORT_RESET_ERR]);
			break;
		case PROTIMS_PORTRECV:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[PORT_RECV_ERR]);
			break;
		default:
			break;
		}
	}
	else if (err[0] >= PROTIMS_TCP_BASE && err[0] < PROTIMS_TCP_END)	// TCP error
	{
		switch(err[0])
		{
		case PROTIMS_ROUTERSETDEFAULT:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_ROUTERSETDEFAULT_ERR]);
			break;
		case PROTIMS_DHCPSTART:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_DHCPSTART_ERR]);
			break;
		case PROTIMS_DHCPCHECK:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_DHCPCHECK_ERR]);
			break;
		case PROTIMS_ETHSET:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_ETHSET_ERR]);
			break;
		case PROTIMS_NETSOCKET:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_NETSOCKET_ERR]);
			break;
		case PROTIMS_DNSRESLOVE:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[DNSRESLVE_ERROR]);
			break;
		case PROTIMS_SOCKETADDSET:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_SOCKETADDSET_ERR]);
			break;
		case PROTIMS_NETCONNECT:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_NETCONNECT_ERR]);
			break;
		case PROTIMS_NETSEND:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_NETSEND_ERR]);
			break;
		case PROTIMS_NETRECV:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_NETRECV_ERR]);
			break;		
		default:
			break;
		}
	}
	else if (err[0] >= PROTIMS_PPP_BASE && err[0] < PROTIMS_PPP_END)	// PPP error
	{
		switch(err[0])
		{
		case PROTIMS_PPPLOGIN:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[PPP_LOGIN_ERR]);
			break;
		case PROTIMS_PPPCHECK:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[PPP_CHECK_ERR]);
			break;
		case PROTIMS_PPP_ROUTERSETDEFAULT:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_ROUTERSETDEFAULT_ERR]);
			break;
		case PROTIMS_PPP_NETSOCKET:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_NETSOCKET_ERR]);
			break;
		case PROTIMS_PPP_DNSRESLOVE:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[DNSRESLVE_ERROR]);
			break;
		case PROTIMS_PPP_SOCKETADDRSET:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_SOCKETADDSET_ERR]);
			break;
		case PROTIMS_PPP_NETCONNECT:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_NETCONNECT_ERR]);
			break;
		default:
			break;
		}
	}
	else if (err[0] >= PROTIMS_WNET_BASE && err[0] < PROTIMS_WIRELESS_END)	// Wireless error
	{
		switch(err[0])
		{
		case PROTIMS_WLINIT:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[WL_INIT_ERR]);
			break;
		case PROTIMS_WLOPENPORT:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[WL_OPENPORT_ERR]);
			break;
		case PROTIMS_WLGETSINGAL:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[WL_GETSIGNAL_ERR]);
			break;
		case PROTIMS_WLLOGINDIAL:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[WL_LOGINDIAL_ERR]);
			break;
		case PROTIMS_WLCLOSEPORT:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[WL_CLOSEPORT_ERR]);
			break;
		case PROTIMS_WNET_DNSRESLOVE:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[DNSRESLVE_ERROR]);
			break;
		case PROTIMS_WNET_CONNECT_FAILED:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_NETCONNECT_ERR]);
			break;
		case PROTIMS_WNET_NETRECV:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TCP_NETRECV_ERR]);
			break;			
		default:
			break;
		}
	}
	else if (err[0] >= PROTIMS_USERCANCEL && err[0] < PROTIMS_COMM_END)	// Comm error
	{
		switch(err[0])
		{
		case PROTIMS_USERCANCEL:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[USER_CANCEL]);
			break;
		case PROTIMS_COMM_PARA_INCORRECT:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[PARA_ERROR]);
			break;			
		case PROTIMS_COMM_TIMEOUT_:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[COMM_TIMEOUT]);
			break;
		case PROTIMS_COMM_WNET_RECV_ERROR:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[RECV_ERROR_DATA]);
			break;
		case PROTIMS_COMM_VERIFY_ERROR:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[DATA_CRC_ERROR]);
			break;
		case PROTIMS_SERVER_DEALERROR:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[GET_ERROR_CODE]);
			break;
		case PROTIMS_TIMEOUT:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[DOWNLOAD_TIMEOUT]);
			break;
		case PROTIMS_TID_ERROR:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[TID_ERROR]);
			break;		
		default:
			break;
		}
	}
	else if (err[0] >= PROTIMS_FILE_OPERATE && err[0] < PROTIMS_FILE_END)	// File operate error
	{
		switch(err[0])
		{
		case PROTIMS_SEEK_FILE_FAILED_:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[FILE_SEEK_ERR]);
			break;
		case PROTIMS_OPEN_FILE_FAILED_:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[FILE_OPEN_ERR]);
			break;			
		case PROTIMS_READ_FILE_FAILED_:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[FILE_READ_ERR]);
			break;
		case PROTIMS_WRITE_FILE_FAILED_:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[FILE_WRITE_ERR]);
			break;
		case PROTIMS_NOENOUGH_SPACE:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[FILE_NOENOUGH_SPACE_ERR]);
			break;
		case PROTIMS_REMOVE_FILE_FAILED_:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[FILE_REMOVE_ERR]);
			break;
		default:
			break;
		}
	}
	else if (err[0] >= PROTIMS_KEY_DOWNLOAD_BASE && err[0] < PROTIMS_KEY_DOWNLOAD_END)	// File operate error
	{
		switch(err[0])
		{
		case PROTIMS_NO_RSAKEY_ERROR:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[NO_RSAKEY_ERROR]);
			break;
		case PROTIMS_KEY_VALIDATE_ERROR:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[KEY_VALIDATE_ERROR]);
			break;
		case PROTIMS_RSA_ENCRYPT_ERROR:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[RSA_ENCRYPT_ERROR]);
			break;
		case PROTIMS_KEY_NUM_ERROR:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[KEY_NUM_ERROR]);
			break;
		case PROTIMS_KEY_LENGTH_ERROR:
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[KEY_LENGTH_ERROR]);
			break;			
		default:
			break;
		}
	}
	else if(err[0] == 0)
	{
	}
	else
	{
		ScrPrint(0, 4, CFONT, "%d,%d", err[0], err[1]);
	}
	
	PubBeepErr();
	kbflush();
	if(getkey()==KEY1 && getkey()==KEY3 && getkey()==KEY1 && getkey()==KEY4)
	{
		ScrPrint(0, 6, CFONT, "%d,%d", err[0], err[1]);
		getkey();
	}

	return errCode;
}

void DispTitle(void *pTitle)
{
	ScrCls();
	PubDispString(pTitle, 0|DISP_LINE_REVER|DISP_LINE_CENTER);
}

static int GetTerminalID()
{
	char szTid[17] = {0};

	DispTitle("KMS");
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[GET_TID]);
	KmsReadVar(PARA_TID, szTid,sizeof(szTid));
	while(1)
	{
		if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 1, 16, szTid, 60))
			return PROTIMS_USERCANCEL;
		if(strcmp(szTid,"00000000"))
			break;
		PubBeepErr();
	}
	
	KmsWriteVar(PARA_TID,szTid);

	return PROTIMS_SUCCESS;
}

static int GetModemPara()
{
	char telNo[23] = {0};
	char szOutsideNo[11] = {0};
	
	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[OUTSIDE_NUM]);
	KmsReadVar(MODEM_OUTSIDE_NUM, szOutsideNo,sizeof(szOutsideNo));
	if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 0, 10, szOutsideNo, 60)){
		return PROTIMS_USERCANCEL;
	}
	KmsWriteVar(MODEM_OUTSIDE_NUM,szOutsideNo);

	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[MODEM_PARA_PHONE_NUM]);
	KmsReadVar(MODEM_PHONE_NUM, telNo,sizeof(telNo));	
	if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 0, 22, telNo, 60)){
		return PROTIMS_USERCANCEL;
	}	
	KmsWriteVar(MODEM_PHONE_NUM,telNo);

	if(PROTIMS_SUCCESS != GetTerminalID()){
		return PROTIMS_USERCANCEL;
	}

	KmsWriteVar(PARA_COMMODE,COMMODE_MODEM);
	
	return PROTIMS_SUCCESS;

}

static int GetCOMPara()
{
	char szSerialPara[16] = {0};
	char szTid[9] = {0};
	unsigned char ch =  0;

	DispTitle(p_gbVars.pDisplay[BAUD_RATE]);
	ScrPrint(0, 2, CFONT, "1-9600  4-38400");
	ScrPrint(0, 4, CFONT, "2-14400 5-57600");
	ScrPrint(0, 6, CFONT, "3-19200 6-115200");

	while (1)
	{
		ch = getkey();
		if ((ch>='1') && (ch<='6'))	break;
		else if (KEYCANCEL == ch)	return PROTIMS_USERCANCEL;	
	}
		
	switch(ch)
	{
	case KEY1:
		strcpy(szSerialPara,"9600,8,n,1");
		break;
	case KEY2:
		strcpy(szSerialPara,"14400,8,n,1");
		break;
	case KEY3:
		strcpy(szSerialPara,"19200,8,n,1");
		break;
	case KEY4:
		strcpy(szSerialPara,"38400,8,n,1");
		break;
	case KEY5:
		strcpy(szSerialPara,"57600,8,n,1");
		break;
	case KEY6:
	default:
		strcpy(szSerialPara,"115200,8,n,1");
		break;
	}	

	KmsWriteVar(COM_BAUD_RATE,szSerialPara);
	
	if(PROTIMS_SUCCESS != GetTerminalID()){
		return PROTIMS_USERCANCEL;
	}

	KmsWriteVar(PARA_COMMODE,COMMODE_COM);
	
	return PROTIMS_SUCCESS;
	
}

static int GetGPRSPara()
{
	char szAPN[33] = {0};
	char szUserName[33] = {0};
	char szPassword[33] = {0};
	char szRemoteIP[16] = {0};
	char szRemotePort[6] = {0};
	char szPINCode[11] = {0};
	
	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, "APN:");
	KmsReadVar(GPRS_APN, szAPN,sizeof(szAPN));
	if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 0, 32, szAPN, 60)){
		return PROTIMS_USERCANCEL;
	}	
	KmsWriteVar(GPRS_APN, szAPN);

	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[USERNAME]);
	KmsReadVar(GPRS_USERNAME, szUserName,sizeof(szUserName));	
	if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 0, 32, szUserName, 60)){
		return PROTIMS_USERCANCEL;
	}
	KmsWriteVar(GPRS_USERNAME, szUserName);

	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[PASSWORD]);
	KmsReadVar(GPRS_PASSWORD, szPassword,sizeof(szPassword));
	if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 0, 32, szPassword, 60)){
		return PROTIMS_USERCANCEL;
	}	
	KmsWriteVar(GPRS_PASSWORD, szPassword);
	
	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[REMOTE_IP]);
	KmsReadVar(GPRS_REMOTE_IP, szRemoteIP,sizeof(szRemoteIP));
	while(1)
	{
		if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 7, 15, szRemoteIP, 60))
			return PROTIMS_USERCANCEL;
		if(ValidIPAddress((char *)szRemoteIP) )
			break;
		PubBeepErr();		
	}	
	KmsWriteVar(GPRS_REMOTE_IP, szRemoteIP);

	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[REMOTE_PORT]);
	KmsReadVar(GPRS_REMOTE_PORT, szRemotePort,sizeof(szRemotePort));
	while(1)
	{
		if(PROTIMS_SUCCESS != PubGetString(NUM_IN|ECHO_IN, 1, 5, szRemotePort, 60))
			return PROTIMS_USERCANCEL;
		if(ValidPort((char *)szRemotePort) )
			break;
		PubBeepErr();		
	}
	KmsWriteVar(GPRS_REMOTE_PORT, szRemotePort);

	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[PINCODE]);
	KmsReadVar(GPRS_PIN_CODE, szPINCode,sizeof(szPINCode));
	if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 0, 10, szPINCode, 60)){
		return PROTIMS_USERCANCEL;
	}
	KmsWriteVar(GPRS_PIN_CODE, szPINCode);
	
	if(PROTIMS_SUCCESS != GetTerminalID()){
		return PROTIMS_USERCANCEL;
	}

	KmsWriteVar(PARA_COMMODE,COMMODE_GPRS);
	
	return PROTIMS_SUCCESS;
}

static int GetCDMAPara()
{
	char telNo[33] = {0};
	char szUserName[33] = {0};
	char szPassword[33] = {0};
	char szRemoteIP[16] = {0};
	char szRemotePort[6] = {0};
	char szPINCode[11] = {0};
	
	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[MODEM_PARA_PHONE_NUM]);
	KmsReadVar(CDMA_PHONE_NUM, telNo,sizeof(telNo));
	if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 0, 32, telNo, 60)){
		return PROTIMS_USERCANCEL;
	}
	KmsWriteVar(CDMA_PHONE_NUM, telNo);

	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[USERNAME]);
	KmsReadVar(CDMA_USERNAME, szUserName,sizeof(szUserName));
	if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 0, 32, szUserName, 60)){
		return PROTIMS_USERCANCEL;
	}
	KmsWriteVar(CDMA_USERNAME, szUserName);

	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[PASSWORD]);
	KmsReadVar(CDMA_PASSWORD, szPassword,sizeof(szPassword));
	if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 0, 32, szPassword, 60)){
		return PROTIMS_USERCANCEL;
	}
	KmsWriteVar(CDMA_PASSWORD, szPassword);
	
	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[REMOTE_IP]);
	KmsReadVar(CDMA_REMOTE_IP, szRemoteIP,sizeof(szRemoteIP));
	while(1)
	{
		if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 7, 15, szRemoteIP, 60))
			return PROTIMS_USERCANCEL;
		if(ValidIPAddress((char *)szRemoteIP) )
			break;
		PubBeepErr();
	}	
	KmsWriteVar(CDMA_REMOTE_IP, szRemoteIP);

	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[REMOTE_PORT]);
	KmsReadVar(CDMA_REMOTE_PORT, szRemotePort,sizeof(szRemotePort));	
	while(1)
	{
		if(PROTIMS_SUCCESS != PubGetString(NUM_IN|ECHO_IN, 1, 5, szRemotePort, 60))
			return PROTIMS_USERCANCEL;
		if(ValidPort((char *)szRemotePort) )
			break;
		PubBeepErr();		
	}
	KmsWriteVar(CDMA_REMOTE_PORT, szRemotePort);

	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[PINCODE]);
	KmsReadVar(CDMA_PIN_CODE, szPINCode,sizeof(szPINCode));
	if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 0, 10, szPINCode, 60)){
		return PROTIMS_USERCANCEL;
	}	
	KmsWriteVar(CDMA_PIN_CODE, szPINCode);
	
	if(PROTIMS_SUCCESS != GetTerminalID()){
		return PROTIMS_USERCANCEL;
	}

	KmsWriteVar(PARA_COMMODE,COMMODE_CDMA);
	
	return PROTIMS_SUCCESS;
}

static int GetTCPPara()
{
	char szRemoteIP[16] = {0};
	char szRemotePort[6] = {0};
	char szLocalIP[16] = {0};
	char szSubNetMask[16] = {0};
	char szGateWay[16] = {0};
	uchar ucKey = 0x00;
	
	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[REMOTE_IP]);
	KmsReadVar(TCP_REMOTE_IP, szRemoteIP,sizeof(szRemoteIP));	
	while(1)
	{
		if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 7, 15, szRemoteIP, 60))
			return PROTIMS_USERCANCEL;
		if( ValidIPAddress((char *)szRemoteIP) )
			break;
		PubBeepErr();
	}
	KmsWriteVar(TCP_REMOTE_IP, szRemoteIP);
	
	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[REMOTE_PORT]);
	KmsReadVar(TCP_REMOTE_PORT, szRemotePort,sizeof(szRemotePort));
	while(1)
	{
		if(PROTIMS_SUCCESS != PubGetString(NUM_IN|ECHO_IN, 1, 5, szRemotePort, 60))
			return PROTIMS_USERCANCEL;
		if( ValidPort((char *)szRemotePort) )
			break;
		PubBeepErr();	
	}
	KmsWriteVar(TCP_REMOTE_PORT, szRemotePort);

	ucKey = BabySelectOneTwo((char *)p_gbVars.pDisplay[ENABLE_DHCP], "YES", "NO");
	if(ucKey == KEYCANCEL)
	{
		return PROTIMS_USERCANCEL;
	}
	else if(ucKey == KEYENTER || ucKey == KEY1)
	{
		KmsWriteVar(TCP_ENABLE_DHCP,"1");
		
		if(PROTIMS_SUCCESS != GetTerminalID())
		{
			return PROTIMS_USERCANCEL;
		}

		KmsWriteVar(PARA_COMMODE,COMMODE_TCP);
		
		return PROTIMS_SUCCESS;
	}
	else
	{
		KmsWriteVar(TCP_ENABLE_DHCP,"0");
	}
	
	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[LOCAL_IP]);
	KmsReadVar(TCP_LOCAL_IP, szLocalIP,sizeof(szLocalIP));
	while(1)
	{
		if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 7, 15, szLocalIP, 60))
			return PROTIMS_USERCANCEL;
		if( ValidIPAddress((char *)szLocalIP) )
			break;
		PubBeepErr();	
	}
	KmsWriteVar(TCP_LOCAL_IP, szLocalIP);

	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[NETMASK]);
	KmsReadVar(TCP_NETMASK, szSubNetMask,sizeof(szSubNetMask));
	while(1)
	{
		if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 7, 15, szSubNetMask, 60))
			return PROTIMS_USERCANCEL;
		if( ValidIPAddress((char *)szSubNetMask) )
			break;
		PubBeepErr();		
	}	
	KmsWriteVar(TCP_NETMASK, szSubNetMask);

	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[GATEWAY]);
	KmsReadVar(TCP_GATEWAY, szGateWay,sizeof(szGateWay));
	while(1)
	{
		if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 7, 15, szGateWay, 60))
			return PROTIMS_USERCANCEL;
		if( ValidIPAddress((char *)szGateWay) )
			break;
		PubBeepErr();
	}	
	KmsWriteVar(TCP_GATEWAY, szGateWay);
			
	if(PROTIMS_SUCCESS != GetTerminalID()){
		return PROTIMS_USERCANCEL;
	}

	KmsWriteVar(PARA_COMMODE,COMMODE_TCP);
	
	return PROTIMS_SUCCESS;
}

static int GetWiFiPara()
{	
	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, "UNSUPPORT!");
	getkey();
	return PROTIMS_SUCCESS;
}

static int GetPPPPara()
{
	char szPhoneNUm[33] = {0};
	char szUserName[33] = {0};
	char szPassword[33] = {0};
	char szRemoteIP[16] = {0};
	char szRemotePort[6] = {0};
	
	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[MODEM_PARA_PHONE_NUM]);
	KmsReadVar(PPP_PHONENUM, szPhoneNUm,sizeof(szPhoneNUm));
	if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 0, 16, szPhoneNUm, 60)){
		return PROTIMS_USERCANCEL;
	}	
	KmsWriteVar(PPP_PHONENUM, szPhoneNUm);

	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[USERNAME]);
	KmsReadVar(PPP_USERNAME, szUserName,sizeof(szUserName));	
	if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 0, 32, szUserName, 60)){
		return PROTIMS_USERCANCEL;
	}
	KmsWriteVar(PPP_USERNAME, szUserName);

	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[PASSWORD]);
	KmsReadVar(PPP_PASSWORD, szPassword,sizeof(szPassword));
	if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 0, 32, szPassword, 60)){
		return PROTIMS_USERCANCEL;
	}	
	KmsWriteVar(PPP_PASSWORD, szPassword);
	
	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[REMOTE_IP]);
	KmsReadVar(PPP_REMOTE_IP, szRemoteIP,sizeof(szRemoteIP));
	while(1)
	{
		if(PROTIMS_SUCCESS != PubGetString(ALPHA_IN|ECHO_IN, 7, 15, szRemoteIP, 60))
			return PROTIMS_USERCANCEL;
		if(ValidIPAddress((char *)szRemoteIP) )
			break;
		PubBeepErr();
	}	
	KmsWriteVar(PPP_REMOTE_IP, szRemoteIP);

	ScrClrLine(2,7);
	ScrPrint(0, 2, CFONT, p_gbVars.pDisplay[REMOTE_PORT]);
	KmsReadVar(PPP_REMOTE_PORT, szRemotePort,sizeof(szRemotePort));
	while(1)
	{
		if(PROTIMS_SUCCESS != PubGetString(NUM_IN|ECHO_IN, 1, 5, szRemotePort, 60))
			return PROTIMS_USERCANCEL;
		if( ValidPort((char *)szRemotePort) )
			break;
		PubBeepErr();	
	}
	KmsWriteVar(PPP_REMOTE_PORT, szRemotePort);
	
	if(PROTIMS_SUCCESS != GetTerminalID()){
		return PROTIMS_USERCANCEL;
	}

	KmsWriteVar(PARA_COMMODE,COMMODE_PPP);
	
	return PROTIMS_SUCCESS;
}


int ParaTransformers(T_CSCOMMPARA *pComPara,T_INCOMMPARA *ptOutPara)
{
	ptOutPara->bCommMode = pComPara->bCommMode;
	ptOutPara->psAppName = pComPara->szAppName;
	ptOutPara->bLoadType = pComPara->bLoadType;
	ptOutPara->psProtocol = pComPara->psProtocol;
	ptOutPara->ucCallMode = pComPara->ucCallMode;
	ptOutPara->psTermID = pComPara->szTermID;
	
	if(SERIAL_COMM == pComPara->bCommMode)
	{
		//Serial
		ptOutPara->tUnion.tSerial.psPara = pComPara->tUnion.tSerial.szPara;
	}
	else if(MODEM_COMM == pComPara->bCommMode)
	{
		//Modem
		ptOutPara->tUnion.tModem.psTelNo = pComPara->tUnion.tModem.szTelNo;
		ptOutPara->tUnion.tModem.bTimeout = pComPara->tUnion.tModem.bTimeout;
	}
	else if(TCPIP_COMM == pComPara->bCommMode)
	{
		//LAN
		ptOutPara->tUnion.tLAN.psLocal_IP_Addr = pComPara->tUnion.tLAN.szLocal_IP_Addr;
		ptOutPara->tUnion.tLAN.wLocalPortNo = pComPara->tUnion.tLAN.wLocalPortNo;
		ptOutPara->tUnion.tLAN.psRemote_IP_Addr = pComPara->tUnion.tLAN.szRemote_IP_Addr;
		ptOutPara->tUnion.tLAN.wRemotePortNo = pComPara->tUnion.tLAN.wRemotePortNo;
		ptOutPara->tUnion.tLAN.psSubnetMask = pComPara->tUnion.tLAN.szSubnetMask;
		ptOutPara->tUnion.tLAN.psGatewayAddr = pComPara->tUnion.tLAN.szGatewayAddr;
	}
	else if(GPRS_COMM == pComPara->bCommMode)
	{
		//GPRS
		ptOutPara->tUnion.tGPRS.psAPN = pComPara->tUnion.tGPRS.szAPN;
		ptOutPara->tUnion.tGPRS.psUserName = pComPara->tUnion.tGPRS.szUserName;
		ptOutPara->tUnion.tGPRS.psPasswd = pComPara->tUnion.tGPRS.szPasswd;
		ptOutPara->tUnion.tGPRS.psIP_Addr = pComPara->tUnion.tGPRS.szIP_Addr;
		ptOutPara->tUnion.tGPRS.nPortNo = pComPara->tUnion.tGPRS.nPortNo;
		ptOutPara->tUnion.tGPRS.psPIN_CODE = pComPara->tUnion.tGPRS.szPIN_CODE;
		ptOutPara->tUnion.tGPRS.psAT_CMD = pComPara->tUnion.tGPRS.szAT_CMD;
	}
	else if(CDMA_COMM == pComPara->bCommMode)
	{
		//CDMA
		ptOutPara->tUnion.tCDMA.psTelNo = pComPara->tUnion.tCDMA.szTelNo;
		ptOutPara->tUnion.tCDMA.psUserName = pComPara->tUnion.tCDMA.szUserName;
		ptOutPara->tUnion.tCDMA.psPasswd = pComPara->tUnion.tCDMA.szPasswd;
		ptOutPara->tUnion.tCDMA.psIP_Addr = pComPara->tUnion.tCDMA.szIP_Addr;
		ptOutPara->tUnion.tCDMA.nPortNo = pComPara->tUnion.tCDMA.nPortNo;
		ptOutPara->tUnion.tCDMA.psPIN_CODE = pComPara->tUnion.tCDMA.szPIN_CODE;
		ptOutPara->tUnion.tCDMA.psAT_CMD = pComPara->tUnion.tCDMA.szAT_CMD;
	}
	else if(PPP_COMM == pComPara->bCommMode)
	{
		//PPP
		ptOutPara->tUnion.tPPP.psTelNo = pComPara->tUnion.tPPP.szTelNo;
		ptOutPara->tUnion.tPPP.psUserName = pComPara->tUnion.tPPP.szUserName;
		ptOutPara->tUnion.tPPP.psPasswd = pComPara->tUnion.tPPP.szPasswd;
		ptOutPara->tUnion.tPPP.psIP_Addr = pComPara->tUnion.tPPP.szIP_Addr;
		ptOutPara->tUnion.tPPP.nPortNo = pComPara->tUnion.tPPP.nPortNo;
	}	
	else if(WIFI_COMM == pComPara->bCommMode)
	{
		//WIFI
		ptOutPara->tUnion.tWIFI.Wifi_SSID = pComPara->tUnion.tWIFI.szWifi_SSID;
		ptOutPara->tUnion.tWIFI.psPasswd = pComPara->tUnion.tWIFI.szPasswd;
		ptOutPara->tUnion.tWIFI.Local_IP = pComPara->tUnion.tWIFI.szLocal_IP;
		ptOutPara->tUnion.tWIFI.Local_PortNo = pComPara->tUnion.tWIFI.Local_PortNo;
		ptOutPara->tUnion.tWIFI.Remote_IP_Addr = pComPara->tUnion.tWIFI.szRemote_IP_Addr;
		ptOutPara->tUnion.tWIFI.RemotePortNo = pComPara->tUnion.tWIFI.RemotePortNo;
		ptOutPara->tUnion.tWIFI.SubnetMask = pComPara->tUnion.tWIFI.szSubnetMask;
		ptOutPara->tUnion.tWIFI.GatewayAddr = pComPara->tUnion.tWIFI.szGatewayAddr;
		ptOutPara->tUnion.tWIFI.Dns1 = pComPara->tUnion.tWIFI.szDns1;
		ptOutPara->tUnion.tWIFI.Dns2 = pComPara->tUnion.tWIFI.szDns2;
		ptOutPara->tUnion.tWIFI.Encryption_Mode = pComPara->tUnion.tWIFI.Encryption_Mode;
		ptOutPara->tUnion.tWIFI.Encryption_Index = pComPara->tUnion.tWIFI.Encryption_Index;
		ptOutPara->tUnion.tWIFI.DHCP_Flag = pComPara->tUnion.tWIFI.DHCP_Flag;
	}
	
	return PROTIMS_SUCCESS;
}
int LoadDownloadPara(T_CSCOMMPARA *pComPara)
{
	char szAppName[33] = {0};
	char szValue[5] = {0};
	
	char szComMode[2] = {0};
	char szTid[16+1] = {0};
	//MODEM
	char telNo[23] = {0};
	char szOutsideNo[33] = {0};
	char szTimeout[3] = {0};
	//COM
	char szSerialPara[16] = {0};
	//GPRS/CDMA/PPP
	char szAPN[33] = {0};
	char szUserName[33] = {0};
	char szPassword[33] = {0};
	char szRemoteIP[16] = {0};
	char szRemotePort[6] = {0};
	char szPINCode[11] = {0};
	//TCP
	char szEnableDHCP[2] = {0};
	char szLocalIP[16] = {0};
	char szSubNetMask[16] = {0};
	char szGateWay[16] = {0};
	char szDNS[16] = {0};
	
	KmsReadVar(PARA_COMMODE, szComMode,sizeof(szComMode));
	//printk("\r\n[LoadDownloadPara]-----------------------------------------PARA_COMMODE:%s\r\n",szComMode);
	KmsReadVar(PARA_TID, szTid,sizeof(szTid));

	if(strcmp(szComMode,COMMODE_MODEM) == 0)
	{
		pComPara->bCommMode = MODEM_COMM;
		strcpy(pComPara->szTermID,szTid);
		KmsReadVar(MODEM_OUTSIDE_NUM, szOutsideNo,sizeof(szOutsideNo));
		KmsReadVar(MODEM_PHONE_NUM, telNo,sizeof(telNo));
		strcat(szOutsideNo,telNo);
		strcpy(pComPara->tUnion.tModem.szTelNo,szOutsideNo);
		KmsReadVar(MODEM_TIMEOUT, szTimeout,sizeof(szTimeout));
		pComPara->tUnion.tModem.bTimeout = atoi(szTimeout);
	}
	else if(strcmp(szComMode,COMMODE_COM) == 0)
	{
		pComPara->bCommMode = SERIAL_COMM;
		strcpy(pComPara->szTermID,szTid);
		KmsReadVar(COM_BAUD_RATE,szSerialPara,sizeof(szSerialPara));
		strcpy(pComPara->tUnion.tSerial.szPara, szSerialPara);
	}
	else if(strcmp(szComMode,COMMODE_GPRS) == 0)
	{
		pComPara->bCommMode = GPRS_COMM;
		strcpy(pComPara->szTermID,szTid);
		KmsReadVar(GPRS_APN, szAPN,sizeof(szAPN));
		strcpy(pComPara->tUnion.tGPRS.szAPN, szAPN);
		KmsReadVar(GPRS_USERNAME, szUserName,sizeof(szUserName));
		strcpy(pComPara->tUnion.tGPRS.szUserName, szUserName);
		KmsReadVar(GPRS_PASSWORD, szPassword,sizeof(szPassword));
		strcpy(pComPara->tUnion.tGPRS.szPasswd, szPassword);
		KmsReadVar(GPRS_REMOTE_IP, szRemoteIP,sizeof(szRemoteIP));
		strcpy(pComPara->tUnion.tGPRS.szIP_Addr, szRemoteIP);
		KmsReadVar(GPRS_REMOTE_PORT, szRemotePort,sizeof(szRemotePort));
		pComPara->tUnion.tGPRS.nPortNo = atoi(szRemotePort);
		KmsReadVar(GPRS_PIN_CODE, szPINCode,sizeof(szPINCode));
		strcpy(pComPara->tUnion.tGPRS.szPIN_CODE, szPINCode);
	}
	else if(strcmp(szComMode,COMMODE_CDMA) == 0)
	{
		pComPara->bCommMode = CDMA_COMM;
		strcpy(pComPara->szTermID,szTid);
		KmsReadVar(CDMA_PHONE_NUM, telNo,sizeof(telNo));
		strcpy(pComPara->tUnion.tCDMA.szTelNo, telNo);
		KmsReadVar(CDMA_USERNAME, szUserName,sizeof(szUserName));
		strcpy(pComPara->tUnion.tCDMA.szUserName, szUserName);
		KmsReadVar(CDMA_PASSWORD, szPassword,sizeof(szPassword));
		strcpy(pComPara->tUnion.tCDMA.szPasswd, szPassword);
		KmsReadVar(CDMA_REMOTE_IP, szRemoteIP,sizeof(szRemoteIP));
		strcpy(pComPara->tUnion.tCDMA.szIP_Addr, szRemoteIP);
		KmsReadVar(CDMA_REMOTE_PORT, szRemotePort,sizeof(szRemotePort));	
		pComPara->tUnion.tCDMA.nPortNo = atoi(szRemotePort);
		KmsReadVar(CDMA_PIN_CODE, szPINCode,sizeof(szPINCode));
		strcpy(pComPara->tUnion.tCDMA.szPIN_CODE, szPINCode);
	}
	else if(strcmp(szComMode,COMMODE_TCP) == 0)
	{
		pComPara->bCommMode = TCPIP_COMM;
		strcpy(pComPara->szTermID,szTid);
		KmsReadVar(TCP_REMOTE_IP, szRemoteIP,sizeof(szRemoteIP));
		strcpy(pComPara->tUnion.tLAN.szRemote_IP_Addr, szRemoteIP);
		KmsReadVar(TCP_REMOTE_PORT, szRemotePort,sizeof(szRemotePort));
		pComPara->tUnion.tLAN.wRemotePortNo = atoi(szRemotePort);	
		KmsReadVar(TCP_ENABLE_DHCP,szEnableDHCP,sizeof(szEnableDHCP));
		if(strcmp(szEnableDHCP,"1") == 0)
		{
			p_gbVars.g_byTcpipDHCPFlag = PROTIMS_DHCP_SUPPORT;
		}
		
		KmsReadVar(TCP_LOCAL_IP, szLocalIP,sizeof(szLocalIP));
		strcpy(pComPara->tUnion.tLAN.szLocal_IP_Addr, szLocalIP);
		KmsReadVar(TCP_NETMASK, szSubNetMask,sizeof(szSubNetMask));
		strcpy(pComPara->tUnion.tLAN.szSubnetMask, szSubNetMask);
		KmsReadVar(TCP_GATEWAY, szGateWay,sizeof(szGateWay));
		strcpy(pComPara->tUnion.tLAN.szGatewayAddr, szGateWay);
		
	}
	else if(strcmp(szComMode,COMMODE_PPP) == 0)
	{
		pComPara->bCommMode = PPP_COMM;
		strcpy(pComPara->szTermID,szTid);
		KmsReadVar(PPP_PHONENUM, szAPN,sizeof(szAPN));
		strcpy(pComPara->tUnion.tPPP.szTelNo, szAPN);
		KmsReadVar(PPP_USERNAME, szUserName,sizeof(szUserName));
		strcpy(pComPara->tUnion.tPPP.szUserName, szUserName);
		KmsReadVar(PPP_PASSWORD, szPassword,sizeof(szPassword));
		strcpy(pComPara->tUnion.tPPP.szPasswd, szPassword);
		KmsReadVar(PPP_REMOTE_IP, szRemoteIP,sizeof(szRemoteIP));
		strcpy(pComPara->tUnion.tPPP.szIP_Addr, szRemoteIP);
		KmsReadVar(PPP_REMOTE_PORT, szRemotePort,sizeof(szRemotePort));
		pComPara->tUnion.tPPP.nPortNo = atoi(szRemotePort);
	}	
	else
	{
		ScrClrLine(2,7);
		ScrPrint(4, 4, CFONT, p_gbVars.pDisplay[NOT_CONFIGURED]);
		PubWaitKey(5);
		return PROTIMS_COMM_PARA_INCORRECT;
	}

	KmsReadVar(PARA_APPNAME, szAppName,sizeof(szAppName));
	strcpy(pComPara->szAppName,szAppName);
	
	KmsReadVar(PARA_LOADTYPE, szValue,sizeof(szValue));
	pComPara->bLoadType = atoi(szValue);
	KmsReadVar(PARA_CALLMODE, szValue,sizeof(szValue));
	pComPara->ucCallMode = atoi(szValue);
	KmsReadVar(PARA_PROTOCOL, szValue,sizeof(szValue));
	pComPara->psProtocol = atoi(szValue);

	return PROTIMS_SUCCESS;
}
//蹦ノ纗更把计ぃ埃羬ゅン
static int ResumeDownload()
{
	T_INCOMMPARA comPara = {0};
	T_CSCOMMPARA CSComPara = {0};
	int iRet = 0;

	iRet = LoadDownloadPara(&CSComPara);
	if (iRet != PROTIMS_SUCCESS)	//load para failed
	{
		return iRet;
	}
	
	ParaTransformers(&CSComPara,&comPara);	
	
	// download
	iRet = KmsDownload(&comPara);
	if (iRet != PROTIMS_SUCCESS)	//download failed
	{
		return DisplayErrorMessage(iRet);
	}

	return PROTIMS_SUCCESS;
}

// 块更把计纗秈iniゅン程沮穝块更把计暗更笆
static int NewDownload()
{
	int iRet = 0;
	unsigned char out_info[32] = {0};
	SMART_MENU	stSmDownMode;
	static MENU_ITEM stCommMenu[] =
	{
			"",	-1, NULL,
			"MODEM",1, NULL,
			"TCP",	2, NULL,
			"COM",	3, NULL,
			"PPP",	4, NULL,
			"GPRS",	5, NULL,
			"CDMA",	6, NULL,
			"WCDMA",7, NULL,
			"WIFI",	8, NULL,
			"",		0, NULL,
	};
	
	strcpy(stCommMenu[0].pszText,"KMS");
	
	PubSmInit(&stSmDownMode, stCommMenu);

	iRet = GetTermInfo(out_info);
	if (iRet > 0)
	{
		if (out_info[HWCFG_MODEM] == 0)//MODEM
		{
			PubSmMask(&stSmDownMode, "MODEM", SM_OFF);
			PubSmMask(&stSmDownMode, "PPP", SM_OFF);
		} 
		
		if (out_info[HWCFG_LAN] == 0)//LAN
		{
			PubSmMask(&stSmDownMode, "TCP", SM_OFF);
		} 

		if (out_info[HWCFG_GPRS] == 0)//GPRS
		{
			PubSmMask(&stSmDownMode, "GPRS", SM_OFF);
		} 

		if (out_info[HWCFG_CDMA] == 0)//CDMA
		{
			PubSmMask(&stSmDownMode, "CDMA", SM_OFF);
		} 

		if (out_info[HWCFG_WIFI] == 0)//WIFI
		{
			PubSmMask(&stSmDownMode, "WIFI", SM_OFF);
		} 

		if (out_info[HWCFG_WCDMA] == 0)//WCDMA
		{
			PubSmMask(&stSmDownMode, "WCDMA", SM_OFF);
		}
	}
	
	while(1)
	{
		iRet = PubSmartMenuEx(&stSmDownMode, SM_MIDDLE_TITLE, 60);
		//printk("\r\n[NewDownload]-------------------PubSmartMenuEx:%d\r\n",iRet);
		if (iRet<0)
			return PROTIMS_USERCANCEL;
		else if(iRet >= 1 && iRet <=8)
			break;
	}
	
	switch(iRet)
	{
	case 1://MODEM
		if(PROTIMS_SUCCESS != GetModemPara())
			return PROTIMS_USERCANCEL;
		break;		
	case 2://TCP
		if(PROTIMS_SUCCESS != GetTCPPara())
			return PROTIMS_USERCANCEL;	
		break;
	case 3://COM
		if(PROTIMS_SUCCESS != GetCOMPara())
			return PROTIMS_USERCANCEL;	
		break;	
	case 4://PPP
		if(PROTIMS_SUCCESS != GetPPPPara())
			return PROTIMS_USERCANCEL;	
		break;
	case 5://GPRS
	case 7://WCDMA
		if(PROTIMS_SUCCESS != GetGPRSPara())
			return PROTIMS_USERCANCEL;
		break;
	case 6://CDMA
		if(PROTIMS_SUCCESS != GetCDMAPara())
			return PROTIMS_USERCANCEL;
		break;
	case 8://WIFI
		if(PROTIMS_SUCCESS != GetWiFiPara())
			return PROTIMS_USERCANCEL;
		break;
	default:
		break;
	}

	return ResumeDownload();
}

//块把计:Ver 絯侥跋ぃ20竊
int GetRemoteLoadVer(unsigned char *Ver)
{
	if(!Ver)  return -1;
	memset(Ver,0,16);
	
	sprintf(Ver,"%s-%s",LIB_VER,PROTIMS_VER);

	return 0;	
}

//int KmsMain()
//{
//	KmsInit();
//	return NewDownload();
//}

int KmsMain()
{
	KmsInit();
	return NewDownload();
}




