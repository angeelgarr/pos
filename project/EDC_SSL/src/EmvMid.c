/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2014-? PAX Technology, Inc. All rights reserved.
 * Description:     Prolin EMV MID
 * Revision History:
 * Date                  Author                 Action
 * 20141210         Kim.L               created
 * ============================================================================
 */

#include "global.h"

extern uchar	 sPinBlock[8];

uchar cEMVIccIsoCommand(uchar ucslot, APDU_SEND *tApduSend, APDU_RESP *tApduRecv);

static void EmvGetOfflinePinBlock(unsigned char *psPin)
{
    int iLen;
	unsigned char sBuff[30];

	if(psPin == NULL)
	{
		return;
	}

	iLen = strlen((char *)psPin);
    if(iLen > 12)
    {
    	iLen = 12;
    }

    memset(sBuff, 0, sizeof(sBuff));
    memcpy(sBuff, psPin, iLen);
    psPin[0] = (0x20 | iLen);	//C="2"+N book3 pp.89
    memset(sBuff + iLen, 'F', 14);
    PubAsc2Bcd(sBuff, 14, (psPin+1));
}

static void EmvGetRand(unsigned char *psRandom, int iRandomLen)
{
	if(psRandom == NULL)
	{
		return;
	}

	OsGetRandom(psRandom, iRandomLen);
}


/**
*	@fn static int EmvVerifyOfflinePin(int iPinType, unsigned char *psPin, int iPinLen)
*	@brief	EMV在非PCI模式下校验脱机PIN
*	@param	[in] iPinType  PIN的类型
*	@param	[in] psPin	   存放PIN的指针
*	@param	[in] iPinLen   PIN的长度
*	@param	[out] void
*	@return int
*	@author	Prolin App developer
*	@date	2013-05-20
*/
static int EmvVerifyOfflinePin(int iPinType, unsigned char *psPin, int iPinLen)
{
	APDU_SEND stApduSend;
	APDU_RESP stApduRecv;

	if(psPin == NULL)
	{
		return EMV_PARAM_ERR;
	}

	memset(&stApduSend, 0, sizeof(APDU_SEND));
	memset(&stApduRecv, 0, sizeof(APDU_RESP));

    memcpy(stApduSend.Command, "\x00\x20\x00", 3);
    stApduSend.Command[3] = iPinType;
    stApduSend.Lc = iPinLen;
    stApduSend.Le = 0;
    memcpy(stApduSend.DataIn, psPin, iPinLen);

    if(cEMVIccIsoCommand(0, &stApduSend, &stApduRecv))
    {
    	return ICC_CMD_ERR;
    }

    if(stApduRecv.SWA == 0x90 && stApduRecv.SWB == 0x00)
    {
    	return EMV_OK;
    }

    if(stApduRecv.SWA == 0x63 && (stApduRecv.SWB & 0xc0) == 0xc0)
    {
        return ((stApduRecv.SWB & 0x0F) + 1);
    }

    if(stApduRecv.SWA == 0x69 && (stApduRecv.SWB == 0x83 || stApduRecv.SWB == 0x84))
    {
        return EMV_RSP_ERR;
    }

    return EMV_RSP_ERR;
}
/**
*	@fn	int cEMVPedVerifyPlainPin (uchar IccSlot,uchar *ExpPinLenIn,uchar *IccRespOut,uchar Mode,ulong TimeoutMs)
*	@brief	EMV回调函数，实现脱机明文PIN获取及明文PIN的校验功能
*	@param	[in] IccSlot     卡片所在的卡座号
*	@param	[in] ExpPinLenIn 可输入的合法密码长度字符串
*	@param	[in] Mode		 IC卡命令模式
*	@param	[in] TimeoutMs	 输入PIN的超时时间
*	@param	[out] IccRespOut 卡片响应的状态码
*	@return int
*	@author	Prolin App developer
*	@date	2013-05-20
*/
int cEMVPedVerifyPlainPin (uchar IccSlot,uchar *ExpPinLenIn,uchar *IccRespOut,uchar Mode,ulong TimeoutMs)
{
	int iRet;
	OsSleep(50);
	if(glSysParam.stEdcInfo.ucPedMode == PED_INT_PCI)	//内置PED
	{
		iRet = OsPedSetAsteriskLayout(getPEDAsterisk_X(), getPEDAsterisk_Y(), getPEDAsterisk_H(),RGB(0x00, 0x00, 0x00),PED_ASTERISK_ALIGN_CENTER);
		if(0 != iRet)
		{
			return iRet;
		}

		iRet = OsPedVerifyPlainPin(0, "0,4,5,6,7,8,9,10,11,12", 0x00, 60000, IccRespOut);
		if(RET_OK == iRet)
		{
			return PED_RET_OK;
		}
		else if(ERR_PED_NO_PIN_INPUT == iRet)
		{
			return PED_RET_ERR_NO_PIN_INPUT;
		}
		else if(ERR_PED_PIN_INPUT_CANCEL == iRet)
		{
			return PED_RET_ERR_INPUT_CANCEL;
		}
		else if(ERR_PED_ICC_INIT_ERR == iRet)
		{
			return PED_RET_ERR_ICC_NO_INIT;
		}
		else if(ERR_PED_NO_ICC == iRet)
		{
			return PED_RET_ERR_NO_ICC;
		}
		else if(ERR_PED_WAIT_INTERVAL == iRet)
		{
			return PED_RET_ERR_WAIT_INTERVAL;
		}
		else
		{
			return iRet;
		}
	}
	else	//外置PIN PAD
	{
		EmvGetOfflinePinBlock((unsigned char *)sPinBlock);
		iRet = EmvVerifyOfflinePin(0x80, (unsigned char *)sPinBlock, 8);
		if (iRet == 0)
		{
			cEMVVerifyPINOK();
			memcpy(IccRespOut, "\x90\x00", 2);
			return PED_RET_OK;
		}
		else
		{
			return PED_RET_ERR_ICC_NO_INIT;
		}
	}
	return EMV_OK;
}

/**
*	@fn int cEMVPedVerifyCipherPin (uchar IccSlot,uchar *ExpPinLenIn,RSA_PINKEY *RsaPinKeyIn, uchar *IccRespOut, uchar Mode, ulong TimeoutMs)
*	@brief	EMV回调函数，实现脱机密文PIN的获取和密文PIN的校验
*	@param	[in] IccSlot 卡片所在的卡座号
*	@param	[in] ExpPinLenIn	可输入的合法密码长度字符串
*	@param	[in] RsaPinKeyIn	加密所需数据结构
*	@param	[in] Mode	IC卡命令模式
*	@param	[in] TimeoutMs	输入PIN的超时时间
*	@param	[out] IccRespOut 卡片响应的状态码
*	@return int
*	@author	Prolin App developer
*	@date	2013-05-20
*/
int cEMVPedVerifyCipherPin (uchar IccSlot,uchar *ExpPinLenIn,RSA_PINKEY *RsaPinKeyIn, uchar *IccRespOut, uchar Mode, ulong TimeoutMs)
{
	int		iRet,iDataLen;
	uchar	sBuff[100], sData[10];
	ST_RSA_PINKEY stRSAPINKEY;

	OsSleep(50);
	if(glSysParam.stEdcInfo.ucPedMode == PED_INT_PCI)	//内置PED
	{
		memset(&stRSAPINKEY, 0, sizeof(ST_RSA_PINKEY));
		stRSAPINKEY.ModulusLen = RsaPinKeyIn->modlen;
		memcpy(stRSAPINKEY.Modulus, RsaPinKeyIn->mod, sizeof(RsaPinKeyIn->mod));
		memcpy(stRSAPINKEY.Exponent, RsaPinKeyIn->exp, sizeof(RsaPinKeyIn->exp));
		stRSAPINKEY.IccRandomLen = RsaPinKeyIn->iccrandomlen;
		memcpy(stRSAPINKEY.IccRandom, RsaPinKeyIn->iccrandom, sizeof(RsaPinKeyIn->iccrandom));

		iRet = OsPedSetAsteriskLayout(getPEDAsterisk_X(), getPEDAsterisk_Y(), getPEDAsterisk_H(),RGB(0x00, 0x00, 0x00),PED_ASTERISK_ALIGN_CENTER);
		if(0 != iRet)
		{
			return iRet;
		}

		iRet = OsPedVerifyCipherPin(0, &stRSAPINKEY, "0,4,5,6,7,8,9,10,11,12", 0x00, 60000, IccRespOut);
		if(RET_OK == iRet)
		{
			return PED_RET_OK;
		}
		else if(ERR_PED_NO_PIN_INPUT == iRet)
		{
			return PED_RET_ERR_NO_PIN_INPUT;
		}
		else if(ERR_PED_PIN_INPUT_CANCEL == iRet)
		{
			return PED_RET_ERR_INPUT_CANCEL;
		}
		else if(ERR_PED_ICC_INIT_ERR == iRet)
		{
			return PED_RET_ERR_ICC_NO_INIT;
		}
		else if(ERR_PED_NO_ICC == iRet)
		{
			return PED_RET_ERR_NO_ICC;
		}
		else if(ERR_PED_WAIT_INTERVAL == iRet)
		{
			return PED_RET_ERR_WAIT_INTERVAL;
		}
		else
		{
			return iRet;
		}
	}
	else	//外置PIN PAD
	{

		EmvGetOfflinePinBlock((unsigned char *)sPinBlock);

		memset(sBuff, 0, sizeof(sBuff));
		sBuff[0] = 0x7F;
		memcpy(sBuff + 1, sPinBlock, 8);
		memcpy(sBuff+9, RsaPinKeyIn->iccrandom, RsaPinKeyIn->iccrandomlen);
		EmvGetRand(sBuff + 17, RsaPinKeyIn->modlen - 17);
		iDataLen = 0;
		memset(sData, 0, sizeof(sData));
		EMVGetTLVData(0x9F47, sData, &iDataLen);	// 获取(ICC) Public Key Exponent

		OsRSA(RsaPinKeyIn->mod, RsaPinKeyIn->modlen, sData, iDataLen, sBuff, (unsigned char *)sPinBlock);
		iDataLen = RsaPinKeyIn->modlen;
		iRet = EmvVerifyOfflinePin(0x88, (unsigned char *)sPinBlock, iDataLen);
		if (iRet == 0)
		{
			cEMVVerifyPINOK();
			memcpy(IccRespOut, "\x90\x00", 2);
			return PED_RET_OK;
		}
		else
		{
			return PED_RET_ERR_ICC_NO_INIT;
		}
	}
	return EMV_OK;
}

/**
*	@fn	int  cEMVIccIsoCommand(uchar ucslot, APDU_SEND *tApduSend, APDU_RESP *tApduRecv)
*	@brief	EMV回调函数，实现接触式读卡操作
*	@param	[in] ucslot 卡片逻辑通道号
*	@param	[in] tApduSend	发送给ICC卡命令数据结构
*	@param	[out] tApduRecv 从ICC卡返回的数据结构
*	@return int
*	@author	Prolin App developer
*	@date	2013-05-20
*/
uchar cEMVIccIsoCommand(uchar ucslot, APDU_SEND *tApduSend, APDU_RESP *tApduRecv)
{
	int iRet;

	iRet = IccIsoCommand(ucslot, tApduSend, tApduRecv);
	if(iRet != 0)
	{
		return 0x01;
	}
	///////////////////////////////////////////////////////////////////
	{
		char szLogBuf[1024];
		int iLogCnt;
#ifdef APP_DEBUG
		//modified by Kevin change HEX_MODE to ASC_MODE, for JIRA EPT-4
		PubDebugOutput("IC_IsoCommand", NULL, 0, DEVICE_COM1, ASC_MODE);
#endif
		for(iLogCnt = 0; iLogCnt < 4; iLogCnt++)
		{
			sprintf(szLogBuf+iLogCnt*3, "%02X ", *(tApduSend->Command+iLogCnt));
		}
		if (tApduSend->Lc != 0)
		{
			sprintf(szLogBuf+4*3, "%02X ", (char)tApduSend->Lc);
			for(iLogCnt = 0; iLogCnt < tApduSend->Lc; iLogCnt++)
			{
				sprintf(szLogBuf+(iLogCnt+5)*3, "%02X ", *(tApduSend->DataIn+iLogCnt));
			}
			if (tApduSend->Le != 0)
			{
				sprintf(szLogBuf+(5+tApduSend->Lc)*3, "%02X ", (char)tApduSend->Le);
			}
		}
		else
		{
			sprintf(szLogBuf+4*3, "%02X ", (char)tApduSend->Le);
		}
#ifdef APP_DEBUG
		//modified by Kevin change HEX_MODE to ASC_MODE, for JIRA EPT-4
		PubDebugOutput("POS-->IC CARD", szLogBuf, strlen(szLogBuf), DEVICE_COM1, ASC_MODE);
#endif


		for(iLogCnt = 0; iLogCnt < tApduRecv->LenOut; iLogCnt++)
		{
			sprintf(szLogBuf+iLogCnt*3, "%02X ", *(tApduRecv->DataOut+iLogCnt));
		}
		sprintf(szLogBuf+iLogCnt*3, "%02X ", tApduRecv->SWA);
		sprintf(szLogBuf+(iLogCnt+1)*3, "%02X ", tApduRecv->SWB);
#ifdef APP_DEBUG
		//modified by Kevin change HEX_MODE to ASC_MODE, for JIRA EPT-4
        PubDebugOutput("POS<--IC CARD", szLogBuf, strlen(szLogBuf), DEVICE_COM1, ASC_MODE);
#endif
	}
	///////////////////////////////////////////////////////////////////

	return EMV_OK;
}


// 保存CAPK信息
int SaveCAPKFile(EMV_CAPK  *pstCAPK)
{
	int i, iRet;
	EMV_CAPK stCAPKTemp;
	int fd;

	if(pstCAPK == NULL)
	{
		_LOG(E_LOG, "P1 is NULL");
		return -1;
	}

	// 先读，有没有重复的
	fd = open(CAPKINFO, O_RDWR|O_CREATE);
	if(fd<0)
	{
		_LOG(E_LOG, "open return %d[%d], P1=[%s]", fd, errno, CAPKINFO);
		return(ERR_FILEOPER);
	}
	for(i=0; i<glSysParam.uiCapkNum; i++)
	{
		memset(&stCAPKTemp, 0, sizeof(EMV_CAPK));
		if(read(fd,(uchar *)&stCAPKTemp,sizeof(EMV_CAPK)) != sizeof(EMV_CAPK))
		{
			_LOG(W_LOG, "read return [%d], P1=[%d]", errno, fd);
			break;
		}

		if(memcmp(&stCAPKTemp, pstCAPK, sizeof(EMV_CAPK)) == 0)
		{
			close(fd);
			return 0;
		}
	}

	// 后写,说明终端文件中没有
	iRet = seek(fd, (long)(glSysParam.uiCapkNum*sizeof(EMV_CAPK)), SEEK_SET);
	if(iRet < 0)
	{
		_LOG(E_LOG, "seek return %d[%d], P1=[%d],P2=[%d]", iRet, errno, fd,(glSysParam.uiCapkNum*sizeof(EMV_CAPK)));
		close(fd);
		return (ERR_FILEOPER);
	}
	iRet = write(fd, (uchar *)pstCAPK, sizeof(EMV_CAPK));
	close(fd);
	if( iRet != sizeof(EMV_CAPK) )
	{
		_LOG(E_LOG, "write return %d[%d], P1=[%d]", iRet, errno, fd);
		return(ERR_FILEOPER);
	}
	glSysParam.uiCapkNum++;
	SaveSysParam();
	_LOG(T_LOG, "Capk Num = %d", glSysParam.uiCapkNum);
	return 0;
}

int ReadCAPKFile(int index, EMV_CAPK  *pstCAPK)
{
    int i;
    EMV_CAPK stCAPKTemp;
    int fd = open(CAPKINFO, O_RDWR);
    if(fd<0)
    {
        _LOG(E_LOG, "open return %d[%d], P1=[%s]", fd, errno, CAPKINFO);
        return(ERR_FILEOPER);
    }

    for(i=0; i<glSysParam.uiCapkNum; i++)
    {
        memset(&stCAPKTemp, 0, sizeof(EMV_CAPK));
        if(read(fd,(uchar *)&stCAPKTemp,sizeof(EMV_CAPK)) != sizeof(EMV_CAPK))
        {
            _LOG(W_LOG, "read return [%d], P1=[%d]", errno, fd);
            break;
        }

        if(index == i)
        {
            if(pstCAPK)
                memcpy(pstCAPK, &stCAPKTemp, sizeof(EMV_CAPK));
            close(fd);
            return 0;
        }
    }
    close(fd);
    return(ERR_FILEOPER);
}

// 从文件中读取公钥信息并添加到EMV内核中
int AddCAPKIntoEmvLib(uchar ucKeyID, uchar *sRID)
{
	int i,iSuccessNum,iRet;
	EMV_CAPK stCAPKTemp;
	uchar ucKeyFlag;
	int iCapk;
	EMV_CAPK stCAPKDel;
	int  fd;

	iRet = ERR_NO_DISP;
	iSuccessNum = 0;

//	_LOG(T_LOG, "AddCAPKIntoEmvLib %d", glSysParam.uiAidNum);
	//ucKeyID<0 则可能要写入多条公钥,先清除一下内核
	if (ucKeyID < 0)
	{
		for(iCapk=0; iCapk < MAX_KEY_NUM; iCapk++)
		{
			if(EMVGetCAPK(iCapk, &stCAPKDel )== EMV_OK)
			{
				EMVDelCAPK(stCAPKDel.KeyID, stCAPKDel.RID);
			}
		}
	}


	fd = open(CAPKINFO, O_RDWR);
	if(fd<0)
	{
		_LOG(E_LOG, "open return %d[%d], P1=[%s]", fd, errno, CAPKINFO);
		return(ERR_FILEOPER);
	}

	for(i=0; i<glSysParam.uiCapkNum; i++)
	{
		// 先读， 从终端文件
		memset(&stCAPKTemp, 0, sizeof(EMV_CAPK));

		if(read(fd,(uchar *)&stCAPKTemp,sizeof(EMV_CAPK))!= sizeof(EMV_CAPK))
		{
			_LOG(W_LOG, "read return [%d], P1=[%d]", errno, fd);
			close(fd);
			return(ERR_FILEOPER);
		}

		if (memcmp(stCAPKTemp.RID, sRID, sizeof(stCAPKTemp.RID)) == 0)
		{
			if (ucKeyID >= 0)
			{
				if (stCAPKTemp.KeyID == ucKeyID)
				{
					ucKeyFlag = TRUE;
				}
				else
				{
					ucKeyFlag = FALSE;
				}
			}
			else
			{
				ucKeyFlag = TRUE;
			}
			if (ucKeyFlag)
			{
				iRet = EMVAddCAPK(&stCAPKTemp);
				if (iRet == 0)
				{
					iSuccessNum++;
					if (ucKeyID > 0)
					{
						close(fd);
						return 0;
					}
				}
				else
				{
					for(iCapk=0; iCapk < MAX_KEY_NUM; iCapk++)
					{
						if(EMVGetCAPK(iCapk, &stCAPKDel )== EMV_OK)
						{
							EMVDelCAPK(stCAPKDel.KeyID, stCAPKDel.RID);
						}
					}
					iRet = EMVAddCAPK(&stCAPKTemp);
					if (iRet == 0)
					{
						iSuccessNum++;
						if (ucKeyID > 0)
						{
							close(fd);
							return 0;
						}
					}
				}
			}
		}
	}
	close(fd);
	if(iSuccessNum > 0)
	{
		return 0;
	}
	else
	{
		return iRet;
	}
}



// 保存AID信息
int SaveAIDFile(EMV_APPLIST *pstApp)
{
	int i, iRet;
	EMV_APPLIST stAppTemp;

	int fd;

	if(pstApp == NULL )
	{
		_LOG(E_LOG, "P1 is NULL");
		return -1;
	}


	fd = open(AIDINFO, O_RDWR|O_CREATE);
	if(fd<0)
	{
		_LOG(E_LOG, "open return %d[%d], P1=[%s]", fd, errno, AIDINFO);
		return(ERR_FILEOPER);
	}
	// 先读，有没有重复的
	for(i=0; i<glSysParam.uiAidNum; i++)
	{
		memset(&stAppTemp, 0, sizeof(EMV_APPLIST));
		if(read(fd,(uchar *)&stAppTemp,sizeof(EMV_APPLIST))!= sizeof(EMV_APPLIST))
		{
			_LOG(W_LOG, "read return [%d], P1=[%d]", errno, fd);
			break;
//			close(fd);
//			return(ERR_FILEOPER);
		}

		if(memcmp(&stAppTemp, pstApp, sizeof(EMV_APPLIST)) == 0)
		{
			return 0;
		}
	}

	// 后写,说明终端文件中没有
	iRet = seek(fd, (long)(glSysParam.uiAidNum*sizeof(EMV_APPLIST)), SEEK_SET);
	if(iRet <0)
	{
		_LOG(E_LOG, "seek return %d[%d], P1=[%d],P2=[%d]", iRet, errno, fd,(glSysParam.uiAidNum*sizeof(EMV_APPLIST)));
		close(fd);
		return (ERR_FILEOPER);
	}
	iRet = write(fd, (uchar *)pstApp, sizeof(EMV_APPLIST));
	close(fd);
	if( iRet != sizeof(EMV_APPLIST) )
	{
		_LOG(E_LOG, "write return %d[%d], P1=[%d]", iRet, errno, fd);
		return(ERR_FILEOPER);
	}

	glSysParam.uiAidNum++;
	SaveSysParam();
	_LOG(T_LOG, "Aid Num = %d", glSysParam.uiAidNum);
	return 0;
}

int ReadAIDFile(int index, EMV_APPLIST *pstApp)
{
    int i;
    EMV_APPLIST stAppTemp;
     int fd = open(AIDINFO, O_RDWR);
     if(fd<0)
     {
         _LOG(E_LOG, "open return %d[%d], P1=[%s]", fd, errno, AIDINFO);
         return(ERR_FILEOPER);
     }

     for(i=0; i<glSysParam.uiAidNum; i++)
     {
         memset(&stAppTemp, 0, sizeof(EMV_APPLIST));
         if(read(fd,(uchar *)&stAppTemp,sizeof(EMV_APPLIST)) != sizeof(EMV_APPLIST))
         {
             _LOG(W_LOG, "read return [%d], P1=[%d]", errno, fd);
             break;
         }

         if(index == i)
         {
             if(pstApp)
                 memcpy(pstApp, &stAppTemp, sizeof(EMV_APPLIST));
             close(fd);
             return 0;
         }
     }
     close(fd);
     return(ERR_FILEOPER);
}

// 从文件中读取应用AID信息并添加到EMV内核中
int AddAppIntoEmvLib(void)
{
	int i, iSuccessNum, iRet;
	EMV_APPLIST	stAppTemp;
	int  fd;

	iRet = ERR_NO_DISP;
	iSuccessNum = 0;

	fd = open(AIDINFO, O_RDWR);
	if(fd<0)
	{
		_LOG(E_LOG, "open return %d[%d], P1=[%s]", fd, errno, AIDINFO);
		return(ERR_FILEOPER);
	}
	for(i=0; i<glSysParam.uiAidNum; i++)
	{
		if(read(fd,(uchar *)&stAppTemp,sizeof(EMV_APPLIST))!= sizeof(EMV_APPLIST))
		{
			_LOG(E_LOG, "read return [%d], P1=[%d]", errno, fd);
			close(fd);
			return(ERR_FILEOPER);
		}
		iRet = EMVAddApp(&stAppTemp);
		if(iRet != 0)
		{
			_LOG(W_LOG, "EMVAddApp return [%d], total=[%d]",  iRet, glSysParam.uiAidNum);
			continue;
		}
		iSuccessNum++;
	}
	close(fd);
	if(iSuccessNum > 0)
	{
		return 0;
	}
	else
	{
		return iRet;
	}
}
int EMVProcTrans(void)
{
	int iRet, iAuthCodeLen, iAuthDataLen, iScriptLen, iCommuStatus;
	unsigned char ACType;
	unsigned long ulAuthAmt;
	unsigned char RspCode[3], AuthCode[7],AuthData[17], Script[256];


	//EMVSetScriptProcMethod(EMV_SCRIPT_PROC_UNIONPAY);// 仅适合银联
	// 在prolin平台下，EMVStartTrans()函数中两个金额的设置目前暂时不起作用，也就是说实际的金额仍为cEMVInputAmount()函数输入的金额
	ulAuthAmt = atol((char *)glProcInfo.stTranLog.szAmount);
	iRet = EMVStartTrans(ulAuthAmt, 0, &ACType);

	if(iRet)
	{
//	    int iRet2 = 0, iRet3 = 0;
//      char sTemp[4096] = {0};
//      EMVGetTLVData(0x95, sTemp, &iRet2);
//      PubDebugOutput("TVR", sTemp, iRet2, DEVICE_COM1, HEX_MODE);
//      memset(sTemp, 0, sizeof(sTemp));
//      iRet2 = 0;
//      EMVGetTLVData(0x9B, sTemp, &iRet2);
//      PubDebugOutput("TSI", sTemp, iRet2, DEVICE_COM1, HEX_MODE);
        return iRet;
	}

	if(ACType == AC_TC)
	{
		return EMV_OK;
	}

	if(ACType == AC_AAC)
	{
		return EMV_DENIAL;
	}

	iAuthCodeLen = iAuthDataLen = iScriptLen = 0;
	memset(RspCode,  0, sizeof(RspCode));
	memset(AuthCode, 0, sizeof(AuthCode));
	memset(AuthData, 0, sizeof(AuthData));
	memset(Script,	 0, sizeof(Script));

	iRet = cEMVOnlineProc(RspCode, AuthCode, &iAuthCodeLen, AuthData, &iAuthDataLen, Script, &iScriptLen);

	if(ONLINE_APPROVE == iRet)
	{
		iCommuStatus = ONLINE_APPROVE;
	}
	else if(ONLINE_ABORT == iRet)
	{
		iCommuStatus = ONLINE_FAILED;
	}
	else
	{
		iCommuStatus = ONLINE_DENIAL;
	}

	if(strlen((char *)RspCode) == 2)
	{
		EMVSetTLVData(0x8A, RspCode, 2);
	}
	else
	{
		return EMV_DENIAL;
	}

	if(iAuthCodeLen != 0)
	{
		EMVSetTLVData(0x89, AuthCode, iAuthCodeLen);
	}

	if(iAuthDataLen != 0)
	{
		EMVSetTLVData(0x91, AuthData, iAuthDataLen);
	}

	ACType = 0;
	iRet = EMVCompleteTrans(iCommuStatus, Script, &iScriptLen, &ACType);
	if(iRet)
	{
		return iRet;
	}

	return EMV_OK;
}

// TODO
// This is for multiple application
/*
int FileToStruct(char *pszFileName, ST_EVENT_MSG *pstEvent, ST_TMS_MSG *pstTmsMsg)
{
	int fd;
	int iRet;
	long lOffset;

	fd = open(pszFileName, O_RDWR);
	if(fd<0)
	{
		return(ERR_FILEOPER);
	}

	lOffset = 0;



	// 读取结构1
	iRet = seek(fd, lOffset, SEEK_SET);
	if(iRet < 0)
	{
		close(fd);
		return (ERR_FILEOPER);
	}
	iRet = read(fd, (uchar *)pstEvent, sizeof(ST_EVENT_MSG));
	if (iRet != sizeof(ST_EVENT_MSG))
	{
		close(fd);
		return (ERR_FILEOPER);
	}
	lOffset += iRet;


	// 读取结构2
	iRet = seek(fd, lOffset, SEEK_SET);
	if(iRet < 0)
	{
		close(fd);
		return (ERR_FILEOPER);
	}
	iRet = read(fd, (uchar *)pstTmsMsg, sizeof(ST_TMS_MSG));
	if (iRet != sizeof(ST_TMS_MSG))
	{
		close(fd);
		return (ERR_FILEOPER);
	}
	lOffset += iRet;

	close(fd);
	return 0;
}
int StructToFile(char *pszFileName, ST_EVENT_MSG *pstEvent, ST_TMS_MSG *pstTmsMsg)
{
	int fd;
	int iRet;
	long lOffset;

	fd = open(pszFileName, O_RDWR|O_CREATE);
	if(fd<0)
	{
		return(ERR_FILEOPER);
	}

	lOffset = 0;


	// 写入结构1
	iRet = seek(fd, lOffset, SEEK_SET);
	if(iRet < 0)
	{
		close(fd);
		return (ERR_FILEOPER);
	}
	iRet = write(fd, (uchar *)pstEvent, sizeof(ST_EVENT_MSG));
	if (iRet != sizeof(ST_EVENT_MSG))
	{
		close(fd);
		return (ERR_FILEOPER);
	}
	lOffset += iRet;



	// 写入结构2
	iRet = seek(fd, lOffset, SEEK_SET);
	if(iRet < 0)
	{
		close(fd);
		return (ERR_FILEOPER);
	}
	iRet = write(fd, (uchar *)pstTmsMsg, sizeof(ST_TMS_MSG));
	if (iRet != sizeof(ST_TMS_MSG))
	{
		close(fd);
		return (ERR_FILEOPER);
	}
	lOffset += iRet;



	close(fd);
	return 0;
}
*/
