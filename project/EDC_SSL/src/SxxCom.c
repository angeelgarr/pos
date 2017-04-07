/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2006-? PAX Technology, Inc. All rights reserved.
 * Description:   无线模块封装函数
 * Revision History:
 * Date                  Author                 Action
 * 2006.09.12          shengjx                     created
 * ============================================================================
 */

#include "global.h"
#include "SxxCom.h"
#include "ssl_wrapper.h"

/********************** Internal macros declaration ************************/
#define TCPMAXSENDLEN		10240

/********************** Internal structure declaration *********************/

/********************** Internal functions declaration *********************/
static uchar SocketCheck(int sk);

/********************** Internal variables declaration *********************/
static int sg_iSocket = -1;
static SSLCon sg_ssl_con = {0};

/********************** external reference declaration *********************/
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// Sxx TCP connection
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpConnect(const char *pszIP, short sPort, uchar bSSL, int iTimeoutSec)
{
	int		iRet;
	if(bSSL)
	{
	    uchar szCA[120], szCert[120], szKey[120], szBuffer[120];
        if(GetEnv("CA_CRT", szBuffer) != 0)
        {
            return ERR_COMM_INV_PARAM;
        }
        sprintf(szCA, "./data/%s", szBuffer);

        if(GetEnv("CLI_CRT", szBuffer) != 0)
        {
            return ERR_COMM_INV_PARAM;
        }
        sprintf(szCert, "./data/%s", szBuffer);

        if(GetEnv("CLI_KEY", szBuffer) != 0)
        {
            return ERR_COMM_INV_PARAM;
        }
        sprintf(szKey, "./data/%s", szBuffer);

	    sslLoadFile(&sg_ssl_con, szCA, szCert, szKey);
	    iRet = sslConnect(&sg_ssl_con, pszIP, sPort, iTimeoutSec * 1000);
	    if(iRet == 0)
	        sg_iSocket = sg_ssl_con.socket;
	}
	else
	{
	    struct net_sockaddr stServer_addr;

        // Bind IP
        iRet = SockAddrSet(&stServer_addr, (char *)pszIP, sPort);
        if (iRet!=0)
        {
            return iRet;
        }

        // Setup socket
        iRet = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);
        if (iRet < 0) // Modified By Kim 2014-12-10
        {
            return iRet;
        }
        sg_iSocket = iRet;

        // set connection timeout
        if (iTimeoutSec<3)
        {
            iTimeoutSec = 3;
        }

        iRet = Netioctl(sg_iSocket, CMD_TO_SET, iTimeoutSec*1000);
        if (iRet<0) // Modified By Kim 2014-12-10
        {
            return iRet;
        }

        // Connect to remote IP
        iRet = NetConnect(sg_iSocket, &stServer_addr, sizeof(stServer_addr));
        if (iRet!=0)
        {
            NetCloseSocket(sg_iSocket);
            sg_iSocket = -1;
        }
	}

	return iRet;
}

// Sxx TCP/IP send data
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpTxd(const uchar *psTxdData, ushort uiDataLen, uchar bSSL, ushort uiTimeOutSec)
{
	int iRet;
	int iSendLen;
	int iSumLen;

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
		if(bSSL)
		    iRet = sslWrite(&sg_ssl_con, (uchar*)psTxdData+iSumLen, iSendLen);
		else
		    iRet = NetSend(sg_iSocket, (uchar*)psTxdData+iSumLen, iSendLen, 0);
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

// Sxx TCP/IP receive
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpRxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, uchar bSSL, ushort *puiOutLen)
{
	int iRet;


	DelayMs(200);

	iRet = 	Netioctl(sg_iSocket, CMD_TO_SET, uiTimeOutSec*1000);

	if (iRet < 0)
	{
		return iRet;
	}

	if(bSSL)
	{
		iRet = sslRead(&sg_ssl_con, psRxdData, uiExpLen, uiTimeOutSec * 1000);
	}
	else
	{
		iRet = NetRecv(sg_iSocket, psRxdData, uiExpLen, 0);
	}

	if (iRet >= 0)
	{
		*puiOutLen = iRet;
		iRet = 0;
	}
	return iRet;
}

// Sxx TCP/IP close socket
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpOnHook(uchar bSSL)
{
	int iRet;

	if(bSSL){
	    sslClose(&sg_ssl_con);
	    sg_iSocket = -1;
	    return 0;
	}

	iRet = NetCloseSocket(sg_iSocket);
	if (iRet < 0)
	{
		sg_iSocket = -1;
		return iRet;
	}
	sg_iSocket = -1;
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

int SxxLANTcpDial(const TCPIP_PARA *pstTcpPara)
{
	int		iRet;
	uchar	ucRedoDhcp, ucSecondIP;
	uchar   szIpFromDNS[15 + 1] = {0};

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
	uchar bSSL = 0;
	uchar szSSL[120];
	if(0 == GetEnv("E_SSL", szSSL))
	{
	    bSSL = atoi(szSSL);
	}

TAG_RETRY_IP:

	// Connect to remote IP
	if (ucSecondIP)
	{
	    if(!ChkIfValidIp(pstTcpPara->stHost2.szIP)){
            iRet = OsNetDns(pstTcpPara->stHost2.szIP, szIpFromDNS, 8 * 1000);
            if(RET_OK == iRet)
                iRet = SxxTcpConnect(szIpFromDNS, (short)atoi((char *)pstTcpPara->stHost2.szPort), bSSL, 8);
        }
        else
            iRet = SxxTcpConnect(pstTcpPara->stHost2.szIP, (short)atoi((char *)pstTcpPara->stHost2.szPort), bSSL, 8);
	}
	else
	{
	    if(!ChkIfValidIp(pstTcpPara->stHost1.szIP) > 0){
	        iRet = OsNetDns(pstTcpPara->stHost1.szIP, szIpFromDNS, 8 * 1000);
	        if(RET_OK == iRet)
	            iRet = SxxTcpConnect(szIpFromDNS, (short)atoi((char *)pstTcpPara->stHost1.szPort), bSSL, 8);
	    }
	    else
	        iRet = SxxTcpConnect(pstTcpPara->stHost1.szIP, (short)atoi((char *)pstTcpPara->stHost1.szPort), bSSL, 8);
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

			if ( ChkIfValidPort(pstTcpPara->stHost2.szPort) &&
			      ChkIfValidIp(pstTcpPara->stHost2.szIP) &&
				strcmp((char *)(pstTcpPara->stHost2.szIP), (char *)(pstTcpPara->stHost2.szIP)))
			{
				ucSecondIP = TRUE;
				goto TAG_RETRY_IP;
			}
		}
	}

	return iRet;
}

// 初始化无线模块
// initial the wireless module
int SXXWlInit(const WIRELESS_PARAM *pstWlPara)
{
	int iRet;

	WlSelSim(pstWlPara->ucUsingSlot);
	iRet = WlInit(pstWlPara->szSimPin);
	if (iRet == WL_RET_ERR_INIT_ONCE)	//initiated
		iRet = 0;

	SXXWlDispSignal();

	return iRet;
}

// 先判断PPP链路是否联通，若没有联通先进行PPP链接，成功后再进行TCP连接；若已联通则直接进行TCP连接
// check if PPP is linked, if not, build PPP link firstly, and then build TCP link,
// otherwise build TCP link directly
int SXXWlDial(const WIRELESS_PARAM *pstWlPara, int iTimeOut, int iAliveInterval, uchar ucPredial)
{
	int		iRet;
	int		iRetryTime;
	uchar	ucSecondIP;
	uchar   szIpFromDNS[15 + 1] = {0};

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

		iRet = WlPppLogin((uchar *)pstWlPara->szAPN, (uchar *)pstWlPara->szUID, (uchar *)pstWlPara->szPwd, 0xFF, 0, iAliveInterval);
		return 0;
	}

	// ********** Full-dial **********

	// ********** Check PPP connection **********
	TimerSet(TIMER_TEMPORARY, (ushort)(iTimeOut*10));
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
		iRet = WlPppLogin((uchar *)pstWlPara->szAPN, (uchar *)pstWlPara->szUID, (uchar *)pstWlPara->szPwd, 0xFF, iTimeOut*1000, iAliveInterval);
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

    iRet = RouteGetDefault();
    if (iRet!= 11)
    {
        iRet = RouteSetDefault(11);
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

    uchar bSSL = 0;
    uchar szSSL[120];
    if(0 == GetEnv("E_SSL", szSSL))
    {
        bSSL = atoi(szSSL);
    }

_RETRY_SECOND_IP:
	if (ucSecondIP)
	{
	    if(!ChkIfValidIp(pstWlPara->stHost2.szIP)){
           iRet = OsNetDns(pstWlPara->stHost2.szIP, szIpFromDNS, 8 * 1000);
           if(RET_OK == iRet)
               iRet = SxxTcpConnect(szIpFromDNS, (short)atoi((char *)pstWlPara->stHost2.szPort), bSSL, 8);
       }
       else
           iRet = SxxTcpConnect(pstWlPara->stHost2.szIP, (short)atoi((char *)pstWlPara->stHost2.szPort), bSSL, 8);
	}
	else
	{
	    if(!ChkIfValidIp(pstWlPara->stHost1.szIP)){
             iRet = OsNetDns(pstWlPara->stHost1.szIP, szIpFromDNS, 8 * 1000);
             if(RET_OK == iRet)
                 iRet = SxxTcpConnect(szIpFromDNS, (short)atoi((char *)pstWlPara->stHost1.szPort), bSSL, 8);
         }
         else
             iRet = SxxTcpConnect(pstWlPara->stHost1.szIP, (short)atoi((char *)pstWlPara->stHost1.szPort), bSSL, 8);
	}

	if (iRet < 0)
	{
	    if ( ChkIfValidPort(pstWlPara->stHost2.szPort) &&
             ChkIfValidIp(pstWlPara->stHost2.szIP) &&
             strcmp((char *)(pstWlPara->stHost2.szIP), (char *)(pstWlPara->stHost2.szIP)))
        {
			ucSecondIP = TRUE;
			goto _RETRY_SECOND_IP;
		}

		return iRet;
	}

	return 0;
}

// 发送数据
// send data (wireless)
int SXXWlSend(const uchar *psTxdData, ushort usDataLen, uchar bSSL, ushort uiTimeOutSec)
{
	return SxxTcpTxd(psTxdData, usDataLen, bSSL, uiTimeOutSec);
}

// 接收数据
// receive data (wireless)
int SXXWlRecv(uchar *psRxdData, ushort usExpLen, ushort uiTimeOutSec, uchar bSSL, ushort *pusOutLen)
{
	DelayMs(200);
	return SxxTcpRxd(psRxdData, usExpLen, uiTimeOutSec, bSSL, pusOutLen);
}

// 关闭TCP连接
// close the TCP link
int SXXWlCloseTcp(uchar bSSL)
{
	return SxxTcpOnHook(bSSL);
}

// 关闭PPP链路
// close the PPP link
void SXXWlClosePPP(void)
{
	WlPppLogout(); 
	return;
}

// 显示信号强度
// display the wireless signal
void SXXWlDispSignal(void)
{
	uchar	ucRet, ucLevel;
	
	ucRet = WlGetSignal(&ucLevel);
	if( ucRet!=RET_OK )
	{
		return;
	}

	{
		char szBuff[255];
		OsRegGetValue("ro.fac.radio",szBuff);
		if(memcmp(szBuff, "MU509", 5) != 0)
		{
		    sprintf(szBuff, "./res/singal%d.png", (5-ucLevel));
            Gui_SB_ManuallyRefresh(GUI_SB_ICON_WIRELESS, szBuff);
        }
	}
}

