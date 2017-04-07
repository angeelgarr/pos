
#include "sxxcomm.h"
#ifdef _SXX_PROTIMS

static int P_ModemDialOperator(COMM_PARA *ptModPara, unsigned char *psTelNo, unsigned char mode, unsigned short usDelayTime, unsigned char display);
static int TcpipStart(MODEM_SETUP *pConfigData, int* ipHandle);
static int PppStart(T_INCOMMPARA *ptInPara, int* ipHandle);
static int WnetStart(const T_INCOMMPARA *ptInPara, int* ipHandle);
static int	WlLogin_Dial(void);
static int _SerPort_Send(int handle, unsigned char *buff, int SndLen);
static int _WIFI_CloseComPort(void);
static unsigned char ISAllCharInString(unsigned char *pAllChar, unsigned int AllCharLen, unsigned char *str, unsigned int strLen);
static unsigned char CharIsInString(unsigned char ch, unsigned char *str, unsigned int strLen);
static int DwordToString(unsigned long dwIn, unsigned char *str);

/*****************************************ALL COMMS HERE***********************************************/
//////////////////////////////////////////////////////////////////////////
// comm start
// in: ptInpara
// out: handle, used in the other operate, such as reset, send, recv, close
// return: error code
// handle:  Modem - 0, COM - Port NO
//			TCP/IP, PPP, CDMA/GPRS - handle
//			WIFI - UNSUPPORT
int CommConnect(T_INCOMMPARA *ptInPara, int* handle)
{
	int Relogin;
	int nRet;
	MODEM_SETUP sPpp_info;

	p_gbVars.gbSerialLoad = ptInPara->bCommMode;	// save the comm mode to global var

	switch (ptInPara->bCommMode)
	{
	case MODEM_COMM:  // modem通讯方式
		*handle = 0x00;
		for(Relogin=0;Relogin<2;Relogin++)	// Re-dail 1 time
		{
			OnHook();
			DelayMs(700);
			nRet = P_ModemDialOperator(ptInPara->tUnion.tModem.ptModPara, ptInPara->tUnion.tModem.psTelNo, 
				1, (unsigned short)(ptInPara->tUnion.tModem.bTimeout*1000), 1);
			if(PROTIMS_SUCCESS!=nRet)
			{
				DelayMs(200);
				continue;
			}
			else 
				break;
		}
		if (Relogin >= 2)
			return nRet;
		else
			break;

	case SERIAL_COMM: 
		*handle = PROTIMS_PORT_COM1;
		if ((nRet = PortOpen(PROTIMS_PORT_COM1, (char *)ptInPara->tUnion.tSerial.psPara)) != 0)
		{
			return s_SetErrorCode(PROTIMS_PORTOPEN, nRet);
		}
		break;

	case TCPIP_COMM: 

		memset(&sPpp_info, 0, sizeof(sPpp_info));								//初始化结构体

		strcpy(sPpp_info.m_aIPAddress, ptInPara->tUnion.tLAN.psRemote_IP_Addr);  // 远端IP地址
		DwordToString(ptInPara->tUnion.tLAN.wRemotePortNo, sPpp_info.m_aPppPort); // 远端端口号
		strcpy(sPpp_info.m_aLocalIPAddr, ptInPara->tUnion.tLAN.psLocal_IP_Addr); // 本地IP地址
		strcpy(sPpp_info.m_aSubnetMask, ptInPara->tUnion.tLAN.psSubnetMask); // 子网掩码
		strcpy(sPpp_info.m_aGatewayAddr, ptInPara->tUnion.tLAN.psGatewayAddr); // 网关

		nRet = TcpipStart(&sPpp_info, handle);
		if (PROTIMS_SUCCESS != nRet)
			return nRet;
		else
			break;

	case PPP_COMM:  
		nRet=PppStart(ptInPara, handle);
		if (PROTIMS_SUCCESS != nRet)
			return nRet;
		else
			break;

	case CDMA_COMM:
	case GPRS_COMM:
		nRet = WnetStart(ptInPara, handle);
		if (PROTIMS_SUCCESS != nRet)
			return nRet;
		else
			break;

	case WIFI_COMM:		// unsupport
        //nRet = WifiStart(ptInPara);
       // if (PROTIMS_SUCCESS != nRet)
          //  return nRet;
	//break;	
	default:
		return PROTIMS_COMM_PARA_INCORRECT;
	}

	return PROTIMS_SUCCESS;
}

int CommRecvPack(int socket, void *buf, int size, int flags)
{
	return NetRecv(socket, buf, size, flags);
}


/******************************************MODEM*****************************************/
//
int P_ModemDialOperator(COMM_PARA *ptModPara, unsigned char *psTelNo, unsigned char mode, unsigned short usDelayTime, unsigned char display)
{
	int ret;
	ret = ModemCheck();

	if ((0x00 != ret) && (0x08 != ret))	// 检查Modem是否已拨通，如果已拨通则不再拨号
	{
		if (display)
		{
			ScrClrLine(2, 7);
			ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[COMM_DIALING]);
		}

		ret = ModemDial(ptModPara, psTelNo, mode);
		if (ret != 0x00)
		{
			(void)OnHook();
			return s_SetErrorCode(PROTIMS_MODEMDIAL, ret);
		}

//		switch (ret)
//		{
//		case 0x00: // 成功
//			break;
//		case 0x03: // 电话线未接好或者并线电话占用 goto next
//		case 0x33: // 电话线未接 goto next
// 		case 0x83: // 旁置电话、并线电话均空闲(仅用于发号转人工接听方式)
// 			(void)OnHook();
// 			return PROTIMS_NO_PHONE_LINE;
// 		case 0x04: // 线路载波丢失(同步建链失败)
// 			(void)OnHook();
// 			return PROTIMS_NO_CARRIER;
// 		case 0x05: // 拨号无应答
// 			(void)OnHook();
// 			return PROTIMS_NO_ANSWER;
// 		case 0x0d: // 被叫线路忙
// 			(void)OnHook();
// 			return PROTIMS_LINE_BUSY;
// 		case 0xfd: // CANCEL键按下( Modem将终止拨号操作 )
// 			(void)OnHook();
// 			return PROTIMS_USER_CANCEL;
// 		default:
// 			(void)OnHook();
// 			return PROTIMS_DAIL_ERR; //DAIL_ERR;
// 		}
	}
	DelayMs(usDelayTime);
	return PROTIMS_SUCCESS;
}


/**************************************TCP***********************************************/
int TcpipStart(MODEM_SETUP *pConfigData, int* ipHandle)
{
	struct net_sockaddr server_addr;
	int ret=0, ipHdl;
	short port = 0;
	char *serv_ip_str = NULL;

	ScrClrLine(2,7);
	ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[COMM_SET_IP]);
	ret=RouteSetDefault(0); // 用Ethernet  //配置缺省路由 0 表示以太网网卡 ；10 表示 modem PPP 链路； 11 表示无线(GPRS/CDMA)PPP 链路 
	if(ret<0)
		return s_SetErrorCode(PROTIMS_ROUTERSETDEFAULT, ret);

	if (PROTIMS_DHCP_SUPPORT == p_gbVars.g_byTcpipDHCPFlag)  //使用DHCP
	{
		ret = DhcpStart();  // 使能DHCP
		if(ret !=0)
			return s_SetErrorCode(PROTIMS_DHCPSTART, ret);

		TimerSet(TIMER_TEMPORARY,300);//30s
		while (TimerCheck(TIMER_TEMPORARY)!=0)
		{
			DelayMs(200);
			ret = DhcpCheck();
			if (ret==0)
				break;
		}
		if(ret!=0)
			return s_SetErrorCode(PROTIMS_DHCPCHECK, ret); 
		
	}
	else// 非DHCP 模式
	{
		DhcpStop(); // 停止DHCP
		ret=EthSet(pConfigData->m_aLocalIPAddr, pConfigData->m_aSubnetMask,pConfigData->m_aGatewayAddr,NULL);
		if(ret<0)
			return s_SetErrorCode(PROTIMS_ETHSET, ret);
	}

	ipHdl = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0); 
	if(ipHdl < 0)
		return s_SetErrorCode(PROTIMS_NETSOCKET, ipHdl);

	(void)Netioctl(ipHdl, CMD_IO_SET,0); // 阻塞模式
	(void)Netioctl(ipHdl, CMD_TO_SET,15000); // 等待时间5秒

	if (0 == ISAllCharInString(pConfigData->m_aIPAddress, (unsigned int)strlen(pConfigData->m_aIPAddress), "0123456789.", 11))
	{				// DNS解析域名
		int Mi;
		int MyRet=-1;
		char Result[32]={0};

		ScrClrLine(2, 7);
		ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[COMM_DNS_RESLOVE]);
		for(Mi=0;Mi<3;Mi++)
		{
			MyRet=DnsResolve(pConfigData->m_aIPAddress, Result, 32);
			if(!MyRet)
			{
				strcpy(pConfigData->m_aIPAddress,Result);
				break;
			}
		}
		if(3==Mi)
		{
			return s_SetErrorCode(PROTIMS_DNSRESLOVE, MyRet);
		}
	}

	ScrClrLine(2, 7);
	ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[COMM_TCP_CONNECT]);

	port=atoi(pConfigData->m_aPppPort);
	ret=SockAddrSet(&server_addr, pConfigData->m_aIPAddress, (short)port);
	if(ret!=0)
	{
		NetCloseSocket(ipHdl);
		return s_SetErrorCode(PROTIMS_SOCKETADDSET, ret);
	}

	ret = NetConnect(ipHdl, &server_addr, sizeof(server_addr));
	if(ret<0)
	{
		NetCloseSocket(ipHdl);
		return s_SetErrorCode(PROTIMS_NETCONNECT, ret);
	} 

	*ipHandle = ipHdl;
	return PROTIMS_SUCCESS;
}


/*************************************PPP***********************************/
int PppStart(T_INCOMMPARA *ptInPara, int* ipHandle)
{
	int Relogin=0;
	int nRet=-1;
	int i;
	int ipHdl;
	struct net_sockaddr server_addr;

	ScrFontSet(1);
	
	for(Relogin=0;Relogin<3;Relogin++)
	{
		nRet = P_ModemDialOperator(ptInPara->tUnion.tModem.ptModPara,ptInPara->tUnion.tModem.psTelNo, 1, 0, 1);
		if (PROTIMS_SUCCESS == nRet)
			break;
	}
	if (Relogin >= 3)
		return nRet;

	(void)ModemReset();		// 清除modem缓冲区数据，只在拨号后清除

	ScrClrLine(2, 7);
	ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[COMM_PPP_LOGINING]);
	for(Relogin=0;Relogin<3;Relogin++)
	{
		nRet=PPPLogin(ptInPara->tUnion.tPPP.psUserName, ptInPara->tUnion.tPPP.psPasswd, PPP_ALG_PAP , 15000);	// 15秒超时
		if (PROTIMS_SUCCESS == nRet)
			break;
	}
	if (Relogin >= 3)
		return s_SetErrorCode(PROTIMS_PPPLOGIN, nRet);

	// save para to global variables
//	strcpy(p_gbVars.g_PPPLogInfo.UserName,ptInPara->tUnion.tPPP.psUserName);
//	strcpy(p_gbVars.g_PPPLogInfo.PassWord,ptInPara->tUnion.tPPP.psPasswd);
//	strcpy(p_gbVars.g_PPPLogInfo.IPAddr,ptInPara->tUnion.tPPP.psIP_Addr);
//	p_gbVars.g_PPPLogInfo.port = ptInPara->tUnion.tPPP.nPortNo;

	nRet=PPPCheck();
	if(nRet < 0)
		return s_SetErrorCode(PROTIMS_PPPCHECK, nRet);

	if(nRet == 1)
	{
		for(i=0;i<5;i++)
		{
			DelayMs(1000);
			if ((nRet=PPPCheck()) == 0)
				break;	
		}
		if(i == 5) 
			return s_SetErrorCode(PROTIMS_PPPCHECK, nRet);
	}
	
	if(nRet = RouteSetDefault(10))		// it is 10 so Selecte ppp , if it is 0 then selete Eth
		return s_SetErrorCode(PROTIMS_PPP_ROUTERSETDEFAULT, nRet);

	ScrClrLine(2, 7);
	ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[COMM_PPP_OPENING]);

	ipHdl = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0); 
	if(ipHdl < 0)
	{
		NetCloseSocket(ipHdl);
		PPPLogout();
		return s_SetErrorCode(PROTIMS_PPP_NETSOCKET, ipHdl);
	}

	(void)Netioctl(ipHdl, CMD_IO_SET,0); // 阻塞模式
	(void)Netioctl(ipHdl, CMD_TO_SET,15000); // 等待时间15秒

	if (0 == ISAllCharInString(ptInPara->tUnion.tPPP.psIP_Addr, (unsigned char)strlen(ptInPara->tUnion.tPPP.psIP_Addr), "0123456789.", 11))
	{
		int Mi;
		char Result[32]={0};

		ScrClrLine(2, 7);
		ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[COMM_DNS_RESLOVE]);

		for(Mi=0;Mi<3;Mi++)
		{
			if((nRet = DnsResolve(ptInPara->tUnion.tPPP.psIP_Addr, Result, 32)) == 0)
			{
				strcpy(ptInPara->tUnion.tPPP.psIP_Addr,Result);
				break;
			}
		}
		if(3==Mi)
		{
			return s_SetErrorCode(PROTIMS_PPP_DNSRESLOVE, nRet);
		}
	}

	nRet=SockAddrSet(&server_addr, ptInPara->tUnion.tPPP.psIP_Addr, ptInPara->tUnion.tPPP.nPortNo);
	if(nRet)
	{
		NetCloseSocket(ipHdl);
		PPPLogout();
		return s_SetErrorCode(PROTIMS_PPP_SOCKETADDRSET, nRet);
	}

	ScrClrLine(2, 7);
	ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[COMM_PPP_CONNECTING]);
	if((nRet = NetConnect(ipHdl, &server_addr, sizeof(server_addr))) < 0)
	{
		NetCloseSocket(ipHdl);
		PPPLogout();
		return s_SetErrorCode(PROTIMS_PPP_NETCONNECT, nRet);
	} 

	*ipHandle = ipHdl;
	return PROTIMS_SUCCESS;	
}

/********************************WNET******************************************************/
int WnetStart(const T_INCOMMPARA *ptInPara, int* ipHandle)
{
	int connect_state;
	struct net_sockaddr server_addr;
	short port;
	unsigned char SingnalLevelOut;
	int WnetDialTims;
	int byRet=-1;
	int tcp_connect_fail_count = 0;
	int ipHdl;
	int errNo = 0;
	unsigned char  abyRemoteIpAddr[128], abyRemotePort[40];
	
	ScrClrLine(2, 7);
	ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[COMM_WNET_INIT]);

	memset(p_gbVars.szPINCode, 0, sizeof(p_gbVars.szPINCode));
	if (CDMA_COMM==ptInPara->bCommMode)
		strcpy((char *)p_gbVars.szPINCode, (char *)ptInPara->tUnion.tCDMA.psPIN_CODE);
	else
		strcpy((char *)p_gbVars.szPINCode, (char *)ptInPara->tUnion.tGPRS.psPIN_CODE);
	//printk("\r\n[WnetStart]---------------------------szPINCode:%s\r\n",szPINCode);
	
	byRet = WlInit(p_gbVars.szPINCode);//byRet = WlInit(NULL);//modified by chens 2013.11.18
	if (RET_OK !=byRet && WL_RET_ERR_INIT_ONCE !=byRet)
		return s_SetErrorCode(PROTIMS_WLINIT, byRet);

	memset(p_gbVars.abyTempStr, 0, sizeof(p_gbVars.abyTempStr));
	memset(p_gbVars.abyPwd, 0, sizeof(p_gbVars.abyPwd));
	memset(p_gbVars.abyUserName, 0, sizeof(p_gbVars.abyUserName));
	memset(abyRemoteIpAddr, 0, sizeof(abyRemoteIpAddr));
	memset(abyRemotePort, 0 ,sizeof(abyRemotePort));

	if (CDMA_COMM==ptInPara->bCommMode)
	{
		strcpy(p_gbVars.abyPwd, ptInPara->tUnion.tCDMA.psPasswd);
		strcpy(p_gbVars.abyUserName, ptInPara->tUnion.tCDMA.psUserName);
		strcpy(p_gbVars.abyTempStr, ptInPara->tUnion.tCDMA.psTelNo);
		strcpy(abyRemoteIpAddr, ptInPara->tUnion.tCDMA.psIP_Addr);
		sprintf(abyRemotePort,"%d",ptInPara->tUnion.tCDMA.nPortNo);
	}
	else
	{
		strcpy(p_gbVars.abyPwd, ptInPara->tUnion.tGPRS.psPasswd);
		strcpy(p_gbVars.abyUserName, ptInPara->tUnion.tGPRS.psUserName);
		strcpy(p_gbVars.abyTempStr, ptInPara->tUnion.tGPRS.psAPN);
		strcpy(abyRemoteIpAddr, ptInPara->tUnion.tGPRS.psIP_Addr);
		sprintf(abyRemotePort,"%d",ptInPara->tUnion.tGPRS.nPortNo);
	}

	// 无线连通的情况下，不需要重新连接了
//	WlPppLogout();		// 先断开
//	WlClosePort();
//	DelayMs(200);
	byRet = WlPppCheck();

	if (byRet != 0)// && byRet != 1)
	{
		WlClosePort();
		byRet = WlOpenPort();
		if (byRet)
			return s_SetErrorCode(PROTIMS_WLOPENPORT, byRet);

		ScrClrLine(2, 7);
		ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[COMM_WNET_CHECK_SIGNAL]);
		DelayMs(500);

		// modify by wjc at 2009-04-29 开机第一次取不到信号
		WnetDialTims = 0;
		while(WnetDialTims++<20) //最多检测信号量	
		{
			WlGetSignal(&SingnalLevelOut); 
			if (SingnalLevelOut<=3)  
				break;

			DelayMs(1000);
			if(!kbhit() && (getkey() == KEYCANCEL)) 
				return  s_SetErrorCode(PROTIMS_USERCANCEL, PROTIMS_USERCANCEL);
		}

		if (WnetDialTims>=20)  //信号太弱
			return s_SetErrorCode(PROTIMS_WLGETSINGAL, SingnalLevelOut);

		ScrClrLine(2, 7);
		ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[COMM_DIALING]);

		if((byRet = WlLogin_Dial()) != PROTIMS_SUCCESS)
			return byRet;
	}

	connect_state=0;
	while(1)
	{
		if(connect_state>10)
			return s_SetErrorCode(PROTIMS_WNET_CONNECT_FAILED,errNo);
		ipHdl = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0); 
		if(ipHdl < 0)
		{
			connect_state++;
			errNo = ipHdl;
			continue;
		}

		(void)Netioctl(ipHdl, CMD_IO_SET,0); // 阻塞模式
		(void)Netioctl(ipHdl, CMD_TO_SET,5000); // 等待时间5秒
		if (0 == ISAllCharInString(abyRemoteIpAddr, (unsigned char)strlen(abyRemoteIpAddr), "0123456789.", 11))
		{
			int Mi;
			int MyRet=-1;
			char Result[32]={0};

			for(Mi=0;Mi<3;Mi++)
			{
				MyRet=DnsResolve(abyRemoteIpAddr, Result, 32);
				if(!MyRet)
				{
					strcpy(abyRemoteIpAddr,Result);
					break;
				}	
			}
			if(3==Mi)
			{				
				return s_SetErrorCode(PROTIMS_WNET_DNSRESLOVE, MyRet);
			}
		}

		ScrClrLine(2, 7);
		ScrPrint(0, 4, CFONT, p_gbVars.pDisplay[COMM_TCP_CONNECT]);

		port=atoi(abyRemotePort);
		byRet=SockAddrSet(&server_addr, abyRemoteIpAddr, (short)port);
		if(byRet!=0)
		{
			connect_state++;
			errNo = byRet;
			NetCloseSocket(ipHdl);
			continue;
		}

		byRet = NetConnect(ipHdl, &server_addr, sizeof(server_addr));
		if(byRet<0)
		{	
			connect_state++;
			errNo = byRet;
			NetCloseSocket(ipHdl);
			tcp_connect_fail_count++;
			if(tcp_connect_fail_count>3)
			{
				NetCloseSocket(ipHdl);
				WlPppLogout();	
				while(WlPppCheck()==1){};
				if((byRet = WlLogin_Dial()) != PROTIMS_SUCCESS)
					return s_SetErrorCode(PROTIMS_WLLOGINDIAL, byRet);
			}
		} 
		else //connect OK
		{
			break;
		}
	}

	*ipHandle = ipHdl;
	return PROTIMS_SUCCESS;
}

/***************************************GPRS********************************************/
int	WlLogin_Dial(void)
{
	int	WnetDialTims;
	int byRet=-1;
	int Wl_PowerOn;

	byRet = WlOpenPort();
	if (byRet)
		return s_SetErrorCode(PROTIMS_WLOPENPORT, byRet);

	for (Wl_PowerOn = 0; Wl_PowerOn < 2; Wl_PowerOn++)	// 2次
	{
		for (WnetDialTims = 0; WnetDialTims < 3; WnetDialTims++)//最多重新拨号3次	
		{
			byRet = WlPppLogin(p_gbVars.abyTempStr,p_gbVars.abyUserName, p_gbVars.abyPwd,0xff,0,30000);
			while(1)
			{	
				if(!kbhit() && (getkey() == KEYCANCEL)) 
					return  s_SetErrorCode(PROTIMS_USERCANCEL, PROTIMS_USERCANCEL);
				byRet=WlPppCheck();
				if(byRet !=1) 
					break;
			}
			if(byRet)
				DelayMs(500);
			else
				break;
		}

		if (WnetDialTims >= 3) // 拨号失败	restart	
		{
			WlSwitchPower(0);		// 下电再上电		
			DelayMs(8000);
			WlSwitchPower(1);
			byRet = WlInit(p_gbVars.szPINCode);//byRet = WlInit(NULL);
			if (RET_OK != byRet)
				return s_SetErrorCode(PROTIMS_WLINIT, byRet);
		}
		else
			break;
	}

	if(Wl_PowerOn >= 2)
	{
		return  s_SetErrorCode(PROTIMS_WLLOGINDIAL, PROTIMS_WLLOGINDIAL);
	}
	else
	{
		return PROTIMS_SUCCESS;
	}
}

//////////////////////////////////////////////////////////////////////////
// reset
int CommReset(int handle)
{
	switch (p_gbVars.gbSerialLoad)
	{
	case MODEM_COMM:  // modem 通讯方式
	case PPP_COMM:  			// 基于modem的PPP通讯方式
		return s_SetErrorCode(PROTIMS_MODEMRESET, ModemReset()); // 清除异步通讯缓冲区数据
	case SERIAL_COMM:  // 串口通讯方式
		return s_SetErrorCode(PROTIMS_PORTRESET, PortReset((unsigned char)handle)); // 复位串口，清除缓冲区中接收和发送的所有数据
		// 已在拨号后进行缓冲区清除，这里不再执行清除命令
		// return ModemReset();   // 清除异步通讯缓冲区数据
	case TCPIP_COMM:
	case CDMA_COMM:
	case GPRS_COMM:
	default:  // 其他或者错误的通讯方式
		break;
	}

	return PROTIMS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// send bytes
int CommSendPack(int handle, unsigned char *buff, int SndLen)
{
	unsigned char ucRet;
	int iRet;

	if (!kbhit() && (KEYCANCEL == getkey()))  // 检测用户是否按下“Cancel”键
		return s_SetErrorCode(PROTIMS_USERCANCEL, PROTIMS_USERCANCEL);

	switch (p_gbVars.gbSerialLoad)
	{
	case MODEM_COMM: 
		(void)ModemReset();
		ucRet = ModemTxd(buff, (unsigned short)SndLen);
		if (0 == ucRet)
		{
			ucRet = ModemCheck();
			if ((0==ucRet) || (0x08==ucRet))
				return PROTIMS_SUCCESS;

			DelayMs(300);
		}

		return s_SetErrorCode(PROTIMS_MODEMTXD, ucRet);

	case SERIAL_COMM:
		(void)PortReset((unsigned char)handle);		
		return _SerPort_Send(handle, buff, SndLen);

	case CDMA_COMM:
	case GPRS_COMM:
	case TCPIP_COMM:
	case PPP_COMM:
		if(iRet = NetSend(handle,buff, SndLen,0) == SndLen)	// 返回实际发送字节数与待发送字节数一样，成功
			return PROTIMS_SUCCESS;
		else 
			return s_SetErrorCode(PROTIMS_NETSEND, iRet);

	case WIFI_COMM:
	default:
		if (!kbhit() && (KEYCANCEL == getkey()))
			return s_SetErrorCode(PROTIMS_USERCANCEL,PROTIMS_USERCANCEL);
		else 
			return s_SetErrorCode(PROTIMS_COMM_PARA_INCORRECT,PROTIMS_COMM_PARA_INCORRECT);
	}
}

/**************************************PORT************************************************/
int _SerPort_Send(int handle, unsigned char *buff, int SndLen)
{
	int i, ret;

	for (i = 0; i < SndLen; i++)
	{
		ret = PortSend((unsigned char)handle, buff[i]);  // 向指定端口发送一个字节
		if(PROTIMS_SUCCESS!=ret) 
			return s_SetErrorCode(PROTIMS_PORTSEND, ret);
	}

	return PROTIMS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// Recv bytes
int CommRecvByte(int handle, unsigned char *ch, int usTimeOut)
{
	unsigned short RxLen=0;
	unsigned char ucRet = 0;
	int iRet = 0;
	int j = 0;

	if ((PPP_COMM==p_gbVars.gbSerialLoad)||(MODEM_COMM==p_gbVars.gbSerialLoad))
		TimerSet((unsigned char)PROTIMS_RECV_BYTE_TIMER, (unsigned short)(usTimeOut*2));  // PPP和modem模式下超时时间增大2倍
	else
		TimerSet((unsigned char)PROTIMS_RECV_BYTE_TIMER, (unsigned short)usTimeOut);  //

	while (0 != TimerCheck(PROTIMS_RECV_BYTE_TIMER))
	{
		switch (p_gbVars.gbSerialLoad)
		{
		case MODEM_COMM:      // modem通讯方式
			ucRet = ModemAsyncGet(ch);
			if (ucRet == PROTIMS_SUCCESS)
				return PROTIMS_SUCCESS;
			else if (ucRet == 0x04)
				return s_SetErrorCode(PROTIMS_MODEMSAYNCGET, ucRet);
			break;

		case SERIAL_COMM:      // 串口通讯方式
			ucRet = PortRecv((unsigned char)handle, ch, 0);
			if (PROTIMS_SUCCESS == ucRet)
				return PROTIMS_SUCCESS;
			else if (0x02 == ucRet)		// 通道号非法
				return s_SetErrorCode(PROTIMS_PORTRECV, ucRet);
			break;

		case GPRS_COMM:
		case CDMA_COMM:
		case PPP_COMM:   // PPP通讯方式
		case TCPIP_COMM:   // LAN 通讯方式
			iRet = NetRecv(handle,ch,1,0);
			if (iRet <= 0)
				return s_SetErrorCode(PROTIMS_NETRECV, iRet);
			else
				return PROTIMS_SUCCESS;

		default:
			return s_SetErrorCode(PROTIMS_COMM_PARA_INCORRECT, PROTIMS_COMM_PARA_INCORRECT);
		}

		if(!kbhit() && getkey()==KEYCANCEL)
			return s_SetErrorCode(PROTIMS_USERCANCEL, PROTIMS_USERCANCEL);
	} // end while
	return s_SetErrorCode(PROTIMS_COMM_TIMEOUT_, PROTIMS_COMM_TIMEOUT_);
}

//////////////////////////////////////////////////////////////////////////
// Exit & Close
int CommExitClose(int handle)
{	
	int ret = 0;
	switch (p_gbVars.gbSerialLoad)
	{
	case MODEM_COMM:
		return OnHook();

	case SERIAL_COMM:
		return PortOpen((unsigned char)handle, (unsigned char *)"9600,8,n,1");

	case TCPIP_COMM:
		return NetCloseSocket(handle);

	case PPP_COMM:
		(void)NetCloseSocket(handle);
		PPPLogout();
		return OnHook();

	case CDMA_COMM:
	case GPRS_COMM:
		ret = NetCloseSocket(handle);
		WlPppLogout();	
		while(1 == WlPppCheck());
		return WlClosePort();

	case WIFI_COMM:
		return _WIFI_CloseComPort();

	default:
		return ret;
	}
}

/*****************************************WIFI******************************************/
int _WIFI_CloseComPort(void)
{
	unsigned char  ucRet;
	int iCount;

	for(iCount=0; iCount<PROTIMS_RETRY_NUMBER; iCount++)
	{
		ucRet = PortClose(PROTIMS_PORT_WIFI);
		if( ucRet==0 )
			break;

		DelayMs(10);
	}

	return ucRet;
}

// Check String, return 0 = NO, 1 = YES
// NOTICE: AllCharLen, strlen < 255
unsigned char ISAllCharInString(unsigned char *pAllChar, unsigned int AllCharLen, unsigned char *str, unsigned int strLen)
{
	unsigned char i;
	unsigned char flag = 1;
	for (i=0; i<AllCharLen; i++)
	{
		if (1 != CharIsInString(pAllChar[i], str, strLen))
		{
			flag = 0;
			break;
		}
	}
	return flag;
}

// Check Char, return 0 = NO, 1 = YES
unsigned char CharIsInString(unsigned char ch, unsigned char *str, unsigned int strLen)
{
	unsigned char i;
	for (i = 0; i<strLen; i++)
	{
		if (ch == str[i])
		{
			return 1;
		}
	}
	return 0;
}


// NOTICE: str at least 10+1 bytes
int DwordToString(unsigned long dwIn, unsigned char *str)
{
	unsigned char abyTemp[12];
	int len = 9;
//	signed char len = 9;

	if (NULL == str)
		return 1;

	memset(abyTemp, 0, sizeof(abyTemp));
	while (0 != dwIn && len >= 0)
	{
		abyTemp[len--] = (unsigned char)(dwIn%10 + '0');
		dwIn = dwIn/10;
	}
	strcpy(str, &abyTemp[len+1]);
	return 0;
}

int s_SetErrorCode(int Err1, int Err2)
{
	p_gbVars.g_ErrorCode[0] = Err1;
	p_gbVars.g_ErrorCode[1] = Err2;

	return Err2;
}

#endif
