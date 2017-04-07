#include "kms_ex.h"
#include "ini.h"

static int  s_CheckPara(T_INCOMMPARA *ptInPara);

void KmsGetLastErr(int* pLastError)
{	
	pLastError[0] = p_gbVars.g_ErrorCode[0];
	pLastError[1] = p_gbVars.g_ErrorCode[1];
}

int KmsLoadLanguage()
{
	unsigned char out_info[32] = {0};
	
	GetTermInfo(out_info);
	if(out_info[13])//1-chinese font
	{
		p_gbVars.pDisplay = DISPLAY_CN;
		KmsWriteVar(PARA_CURRENT_LANG, "1");
	}
	else
	{
		p_gbVars.pDisplay = DISPLAY_DEFAULT;//ENGLISH
		KmsWriteVar(PARA_CURRENT_LANG, "0");
	}
	
	return PROTIMS_SUCCESS;
}


int KmsInit()
{
	char szSetFlag[2] = {0};
	char szValue[5] = {0};

	memset(&p_gbVars, 0x00, sizeof(SProtims_gbVar));	// global var init
	p_gbVars.pDisplay = DISPLAY_DEFAULT;

	// 秸ㄧ计
	pcTMSStart     =  CommConnect;
	pcTMSReset     =  CommReset;
	pcTMSSendPack  =  CommSendPack;
	pcTMSRecvByte  =  CommRecvByte;
	pcTMSExitClose =  CommExitClose;
	pcTMSNetRecv   =  CommRecvPack;

	KmsReadVar(PARA_SET_DEFAULT, szSetFlag,sizeof(szSetFlag));
	if(szSetFlag[0] != '1')
		KmsSetDefaultCfg();

	// from mointor  砞竚Modem纐粄把计
	KmsReadVar(MODEM_PARA_DP, szValue,sizeof(szValue));
	p_gbVars.stModemPara.DP = atoi(szValue);
	KmsReadVar(MODEM_PARA_CHDT, szValue,sizeof(szValue));
	p_gbVars.stModemPara.CHDT = atoi(szValue);
	KmsReadVar(MODEM_PARA_DT1, szValue,sizeof(szValue));
	p_gbVars.stModemPara.DT1 = atoi(szValue);
	KmsReadVar(MODEM_PARA_DT2, szValue,sizeof(szValue));
	p_gbVars.stModemPara.DT2 = atoi(szValue);
	KmsReadVar(MODEM_PARA_HT, szValue,sizeof(szValue));
	p_gbVars.stModemPara.HT = atoi(szValue);
	KmsReadVar(MODEM_PARA_WT, szValue,sizeof(szValue));
	p_gbVars.stModemPara.WT = atoi(szValue);
	KmsReadVar(MODEM_PARA_SSETUP, szValue,sizeof(szValue));
	p_gbVars.stModemPara.SSETUP = atoi(szValue);
	KmsReadVar(MODEM_PARA_DTIMES, szValue,sizeof(szValue));
	p_gbVars.stModemPara.DTIMES = atoi(szValue);
	KmsReadVar(MODEM_PARA_TIMEOUT, szValue,sizeof(szValue));
	p_gbVars.stModemPara.TimeOut = atoi(szValue);
	KmsReadVar(MODEM_PARA_ASMODE, szValue,sizeof(szValue));
	p_gbVars.stModemPara.AsMode = atoi(szValue);
	
	//memcpy(&p_gbVars.stModemPara, "\x00\x00\x05\x05\x64\x06\xe7\x01\x00\xd0", 10); // 33600bps
	KmsLoadLanguage();
	
	return PROTIMS_SUCCESS;
} 

int KmsDownload(T_INCOMMPARA *ptInPara)
{
	int i;
	int handle = 0;	    // comm handle
	int RecvLen = 0;
	unsigned char RecvBuf[1024] = {0};
	
	if (i = s_CheckPara(ptInPara))	// 块把计浪代籔﹍て纐粄把计
		return KMS_INIT_COMM_ERROR;

	ScrClrLine(2, 7);	// 硄癟﹍て眔硄癟琡
	ScrPrint(0, 4,CFONT, p_gbVars.pDisplay[COMM_INIT]);
	if(PROTIMS_SUCCESS!=pcTMSStart(ptInPara, &handle)) 
		return KMS_INIT_COMM_ERROR;

	ScrClrLine(2, 7);
	ScrPrint(0, 4,CFONT, p_gbVars.pDisplay[COMM_HANDSHAKE]);
	if (PROTIMS_SUCCESS != KmsHandshake(handle))
	{
		DelayMs(500);
		pcTMSExitClose(handle);
		return KMS_HANDSHAKE_ERROR;
	}

	ScrClrLine(2, 7);
	PubDispString(p_gbVars.pDisplay[RSA_PUBKEY], DISP_LINE_CENTER|2);
	PubDispString(p_gbVars.pDisplay[DOWNLOADING], DISP_LINE_CENTER|4);
	if (PROTIMS_SUCCESS != KmsGetRSAKey(handle, ptInPara->psTermID, RecvBuf,&RecvLen))
	{
		DelayMs(500);
		pcTMSExitClose(handle);
		return KMS_RSAKEY_GET_ERROR;
	}

	if(RecvBuf[0] == 1 && RecvLen > 1)//RSA key exist.
	{
		ScrClrLine(2, 7);
		PubDispString(p_gbVars.pDisplay[MAIN_KEY], DISP_LINE_CENTER|2);
		PubDispString(p_gbVars.pDisplay[DOWNLOADING], DISP_LINE_CENTER|4);
		memset(p_gbVars.g_aucRecvBuff,0,sizeof(p_gbVars.g_aucRecvBuff));
		if (PROTIMS_SUCCESS != KmsGetMainKey(handle,RecvBuf+1,p_gbVars.g_aucRecvBuff,&RecvLen))//ignore 1st byte
		{
			DelayMs(500);
			pcTMSExitClose(handle);
			return KMS_MAINKEY_GET_ERROR;
		}
	}
	else
	{		
		DelayMs(500);
		pcTMSExitClose(handle);
		s_SetErrorCode(PROTIMS_NO_RSAKEY_ERROR, 0);
		return KMS_RSAKEY_GET_ERROR;
	}

	if(p_gbVars.g_aucRecvBuff[0] > 0 && RecvLen > 1)//KMS PC send mainkey successful.
	{
		ScrClrLine(2, 7);
		PubDispString(p_gbVars.pDisplay[MAIN_KEY], DISP_LINE_CENTER|2);
		PubDispString(p_gbVars.pDisplay[INJECTING], DISP_LINE_CENTER|4);			
		if (PROTIMS_SUCCESS != KmsWriteMainKey(p_gbVars.g_aucRecvBuff,RecvLen))//2014-7-4 write main key 
		{
			DelayMs(500);
			pcTMSExitClose(handle);
			return KMS_MAINKEY_WRITE_ERROR;
		}
		////2015-3-10 ttt
		//ScrCls();
		//ScrPrint(0,0,0,"[LEN:%d]",RecvLen);
		//ScrPrint(0,1,0,"Recv Value: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
		//	,p_gbVars.g_aucRecvBuff[0],p_gbVars.g_aucRecvBuff[1],p_gbVars.g_aucRecvBuff[2],p_gbVars.g_aucRecvBuff[3]
		//,p_gbVars.g_aucRecvBuff[4],p_gbVars.g_aucRecvBuff[5],p_gbVars.g_aucRecvBuff[6],p_gbVars.g_aucRecvBuff[7]
		//,p_gbVars.g_aucRecvBuff[8],p_gbVars.g_aucRecvBuff[9],p_gbVars.g_aucRecvBuff[10],p_gbVars.g_aucRecvBuff[11]
		//,p_gbVars.g_aucRecvBuff[12],p_gbVars.g_aucRecvBuff[13],p_gbVars.g_aucRecvBuff[14],p_gbVars.g_aucRecvBuff[15]
		//,p_gbVars.g_aucRecvBuff[16],p_gbVars.g_aucRecvBuff[17],p_gbVars.g_aucRecvBuff[18],p_gbVars.g_aucRecvBuff[19]
		//,p_gbVars.g_aucRecvBuff[20],p_gbVars.g_aucRecvBuff[21],p_gbVars.g_aucRecvBuff[22],p_gbVars.g_aucRecvBuff[23]
		//,p_gbVars.g_aucRecvBuff[24],p_gbVars.g_aucRecvBuff[25],p_gbVars.g_aucRecvBuff[26],p_gbVars.g_aucRecvBuff[27]
		//,p_gbVars.g_aucRecvBuff[28],p_gbVars.g_aucRecvBuff[29],p_gbVars.g_aucRecvBuff[30],p_gbVars.g_aucRecvBuff[31]
		//,p_gbVars.g_aucRecvBuff[32],p_gbVars.g_aucRecvBuff[33],p_gbVars.g_aucRecvBuff[34],p_gbVars.g_aucRecvBuff[35]
		//,p_gbVars.g_aucRecvBuff[36],p_gbVars.g_aucRecvBuff[37],p_gbVars.g_aucRecvBuff[38],p_gbVars.g_aucRecvBuff[39]);
		//getkey();//KeyData
	}
	else
	{
		DelayMs(500);
		pcTMSExitClose(handle);
		s_SetErrorCode(PROTIMS_KEY_VALIDATE_ERROR, 0);
		return KMS_MAINKEY_GET_ERROR;
	}
	//	祇癳ЧΘ计沮
	if (PROTIMS_SUCCESS != KmsExchangeKeyState(handle))
	{
		DelayMs(500);
		pcTMSExitClose(handle);
		return KMS_KEY_DOWNLOAD_ERROR;
	} 

	DelayMs(2000);
	pcTMSExitClose(handle);		// 更ЧΘ闽超硈钡

	ScrClrLine(2, 7);
	PubDispString(p_gbVars.pDisplay[DOWNLOAD_SUCCESS], DISP_LINE_CENTER|2);
	Beep();
	PubWaitKey(60);
	
	//2014-7-7 here add a global flag
	glKeyInjectOK = 1;
	return PROTIMS_SUCCESS;
}


int  s_CheckPara(T_INCOMMPARA *ptInPara)
{
	// ****************************** bCommMode ******************************
	if (ptInPara->bCommMode > UNKNOWN_COMM)
	{
		return s_SetErrorCode(PROTIMS_COMM_PARA_INCORRECT, PROTIMS_COMM_PARA_INCORRECT);
	}

	// ****************************** bLoadType ******************************
	if (ptInPara->bLoadType == 0x00)	// 侣Τ0x00秸ノ
	{
		ptInPara->bLoadType = LOAD_APPL_FLAG | LOAD_PARA_FLAG | LOAD_DATA_FLAG;
	}

#ifndef _SXX_PROTIMS
	ptInPara->bLoadType &= 0x07;		// p系列只下载程序，参数，数据文件
#endif

	// ****************************** psTermID ******************************
	if (ptInPara->psTermID == NULL)
		return s_SetErrorCode(PROTIMS_TID_ERROR, PROTIMS_TID_ERROR);
	else if (strlen(ptInPara->psTermID) != 8 || memcmp(ptInPara->psTermID, "00000000", 8)==0)
		return s_SetErrorCode(PROTIMS_TID_ERROR, PROTIMS_TID_ERROR);

	switch (ptInPara->bCommMode)
	{
	case MODEM_COMM:
		if(ptInPara->tUnion.tModem.ptModPara == NULL || (char*)ptInPara->tUnion.tModem.ptModPara == "")
			ptInPara->tUnion.tModem.ptModPara = &p_gbVars.stModemPara;
		break;
	case GPRS_COMM:
		if (ptInPara->tUnion.tGPRS.psPIN_CODE == NULL)
			ptInPara->tUnion.tGPRS.psPIN_CODE = "";
		if (ptInPara->tUnion.tGPRS.psAT_CMD == NULL)
			ptInPara->tUnion.tGPRS.psAT_CMD = "";
		break;
	case CDMA_COMM:
		if (ptInPara->tUnion.tCDMA.psPIN_CODE == NULL)
			ptInPara->tUnion.tCDMA.psPIN_CODE = "";
		if (ptInPara->tUnion.tCDMA.psAT_CMD == NULL)
			ptInPara->tUnion.tCDMA.psAT_CMD = "";
		break;
	case PPP_COMM:
		if (ptInPara->tUnion.tPPP.ptModPara == NULL || (char*)ptInPara->tUnion.tPPP.ptModPara == "")
			ptInPara->tUnion.tPPP.ptModPara = &p_gbVars.stModemPara;
		break;
	case WIFI_COMM:
	default:
		break;
	}

	return PROTIMS_SUCCESS;

}













