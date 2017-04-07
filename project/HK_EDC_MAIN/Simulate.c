#include "global.h"

/* ********************* Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// 检查是否处于练习/演示模式
uchar ChkIfTrainMode(void)
{
	return (glSysParam.stTxnCommCfg.ucCommType==CT_DEMO);
}

// 练习/演示模式下用于模拟后台数据
int Simu_CreatDummyRecvData(void *pstProcInfo, void *pstRecvPacket)
{
	uchar	szBuff[32];
	STISO8583		*pstRecv;
	SYS_PROC_INFO	*pstProc;

	pstRecv = (STISO8583 *)pstRecvPacket;
	pstProc = (SYS_PROC_INFO *)pstProcInfo;

	// Date/time
	PubGetDateTime(szBuff);
	if ( (strlen(pstProc->stTranLog.szDateTime)==0) || (pstProc->stTranLog.ucTranType==VOID) )
	{
		sprintf((char *)(pstProc->stTranLog.szDateTime), "%.14s", (char *)szBuff);
	}
	// Response code
	sprintf((char *)(pstRecv->szRspCode), "00");
	if ((pstProc->stTranLog.ucTranType==SETTLEMENT) && (glSendPack.szProcCode[1]=='2'))
	{
		sprintf((char *)(pstRecv->szRspCode), "95");
	}
	
	sprintf((char *)(pstProc->stTranLog.szRspCode), (char *)(pstRecv->szRspCode));
	// Authorization code
	sprintf(pstProc->stTranLog.szAuthCode, "      ");
	if ((pstProc->stTranLog.ucTranType!=VOID) && (strlen(pstProc->stTranLog.szAuthCode)==0))
	{
		sprintf(pstProc->stTranLog.szAuthCode, "*DEMO*");
	}

	// RRN
	sprintf((char *)(pstProc->stTranLog.szRRN), "111111%6.6s", (char *)szBuff+8);

	if (pstProc->stTranLog.ucTranType==RATE_SCB) // PP-DCC
	{
		PPDCC_CreateDummyRspData(pstProc);
	}
	
	if (pstProc->stTranLog.ucTranType==RATE_SCB)
	{
		PPDCC_CreateDummyRspData(pstProc);
	}


	return 0;
}

// 在POS运行时简单显示debug信息
int PosDebugShow(char *pszStr, ...)
{
#ifdef APP_DEBUG
	uchar		szBuff[1024+1];
	va_list		pVaList;

	if( pszStr==NULL || *(uchar *)pszStr==0 )
	{
		return 0;
	}

	va_start(pVaList, pszStr);
	vsprintf((char*)szBuff, (char*)pszStr, pVaList);
	va_end(pVaList);

	ScrClrLine(0,1);
	ScrPrint(0,0,0,szBuff);
	kbflush();getkey();
#endif
	return 0;
}

// end of file
