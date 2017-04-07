
#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
/********************** Internal variables declaration *********************/

/********************** external reference declaration *********************/
extern void CLSSPreTrans(void);
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/
#ifdef APP_MANAGER_VER
int ManagerServStartUp(void)
{
	if(glSysParam.ucKbSound)
		glSysParam.ucKbSound = TRUE; //2014-4-24 sound should not reset after restart POS terminal
	else
		glSysParam.ucKbSound = FALSE;
	kbmute(glSysParam.ucKbSound);	// mute/enable keyboard sound according to setting.
	SaveSysParam();
    return 0;
}
#endif
// 处理用户自定义事件
// When the first time called by manager, the msg type must be USER_MSG
int ProcUserMsg(void)
{
	int		iRet;
  int		Counter;
	uchar	szEngTime[16+1];
	uchar	ucAcqIndex;
#ifdef APP_MANAGER_VER
#ifndef WIN32
	ManagerInit();//manager init
	ManagerServStartUp();
#endif
#endif
	// no need to init module if running as minor sub-app
	if( glSysParam.stTxnCommCfg.ucCommType==CT_GPRS  ||
		glSysParam.stTxnCommCfg.ucCommType==CT_3G    ||//2015-11-23
		glSysParam.stTxnCommCfg.ucCommType==CT_CDMA  ||
		glSysParam.stTxnCommCfg.ucCommType==CT_TCPIP ||
		glSysParam.stTxnCommCfg.ucCommType==CT_WIFI)
	{
		GetEngTime(szEngTime);
		PubShowTitle(TRUE, szEngTime);
		PubDispString(_T("INIT COMM..."), 4|DISP_LINE_LEFT);
		
		iRet = CommInitModule(&glSysParam.stTxnCommCfg);
		if (iRet!=0)
		{
			PubShowTitle(TRUE, szEngTime);
			PubDispString(_T("INIT FAIL"), 2|DISP_LINE_LEFT);
			if (glSysParam.stTxnCommCfg.ucCommType==CT_GPRS  ||
				glSysParam.stTxnCommCfg.ucCommType==CT_3G    ||//2015-11-23
				glSysParam.stTxnCommCfg.ucCommType==CT_CDMA)
			{
				PubDispString(_T("PLS CHECK SIM OR"), 4|DISP_LINE_LEFT);
				PubDispString(_T("HARDWARE/SIGNAL."), 6|DISP_LINE_LEFT);
			}
			if (glSysParam.stTxnCommCfg.ucCommType==CT_TCPIP)
			{
				PubDispString(_T("PLS CHECK CABLE"),  4|DISP_LINE_LEFT);
				PubDispString(_T("OR CONFIG."),       6|DISP_LINE_LEFT);
			}
			if (glSysParam.stTxnCommCfg.ucCommType==CT_WIFI)
			{
				PubDispString(_T("PLS CHECK SIGNAL"), 4|DISP_LINE_LEFT);
				PubDispString(_T("OR CONFIG."),       6|DISP_LINE_LEFT);
			}
			PubWaitKey(-1);
		}
	}

	//开机如果支持非接的话就自动POLL 一次
	if (glSysParam.stEdcInfo.ucClssFlag == 1 &&
		((glSysParam.stEdcInfo.ucClssMode == 0)||(glSysParam.stEdcInfo.ucClssMode == 2)))
	{
		ScrCls();
		DispWait();
		DelayMs(2000);//delay for poll synchronously

		PubShowTitle(TRUE, (uchar *)_T("      POLL      "));
		iRet = ClssInit();
		if(iRet)//build114
		{
			for (Counter=0;Counter<3;Counter++)//max poll 3 times squall 20121031
			{	
				iRet = ClssInit();  
				if (iRet)
				{
					ProcError_Wave(iRet);
				}
				else
					break;	
			}
			return iRet;
		}
	}
	
     // Report the utility to app-manager: EMV enable/disable; ECR enable/disable; ...
    if (strcmp(glEdcMsgPtr->MagMsg.track1, "MANAGER")==0)
    {
	
		//当有Manager时保证每次调用子应用只在开机时POLL一次R50
		if (glSysParam.stEdcInfo.ucClssFlag == 1 &&
			((glSysParam.stEdcInfo.ucClssMode == 0)||(glSysParam.stEdcInfo.ucClssMode == 2)))
		{
			memcpy(&glClssInfo,glEdcMsgPtr->UserMsg,sizeof(CLSS_INFO));
			memcpy(glAmount,glClssInfo.glAmount,13);
#ifdef AMT_PROC_DEBUG
			//2014-9-18 each time come to glAmount or szAmount
			sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(12)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(12)%.13s",glProcInfo.stTranLog.szAmount);
			glProcInfo.stTranLog.cntAmt++;
	//			//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 15: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
			bFirstPollFlag=glClssInfo.bFirstPollFlag;
		//	ScrCls();
		//	PubShowTitle(TRUE, (uchar *)_T("      POLL      "));
		//	DispWait();
 
			iRet = ClssInit();
			if(iRet)
			{
				ProcError_Wave(iRet);
				return iRet;
			}
		}
		bFirstPollFlag=1;
		glSysParam.ucManagerOn = 1;
        PackEDCToManagerMsg(glEdcMsgPtr->MagMsg.track1, sizeof(glEdcMsgPtr->MagMsg.track1));
		SaveSysParam();
    }
	
	
	// erase transaction log of last settlement
	for(ucAcqIndex=0; ucAcqIndex<glSysParam.ucAcqNum; ucAcqIndex++)
	{
		if( glSysCtrl.sAcqStatus[ucAcqIndex]==S_CLR_LOG )
		{
			ClearRecord(glSysParam.stAcqList[ucAcqIndex].ucKey);
		}
	}

	// Just for prompting user to download DCC BIN (if not yet)
    // PP-DCC
	if (PPDCC_SetCurAcq()==0)
	{
	  if (PPDCC_LoadBinFile()!=0)
          {
            PPDCC_PromptDownloadBin();
          }
	}

	return 0;
}

// 自动下载参数
// Auto download parameter, new Protims protocol
int AutoDownParam(void)
{
	return 0;
}

// 读取自动参数下载参数事件
// get auto download parameter and return it to app manager
int GetAutoDownParam(void)
{
	return 0;
}

// 处理ECR事件
// Process ECR message
int ProcEcrMsg(void)
{
	if (!ChkEdcOption(EDC_ECR_ENABLE))
    {
        return 0;
    }
    return EcrTrans(glEdcMsgPtr->MagMsg.track1+2);
}

// 处理磁卡刷卡事件
// Process magcard swipe event
int ProcMagMsg(void)
{
	int		iRet;

	glProcInfo.stTranLog.ucTranType = SALE_OR_AUTH;
	iRet = SwipeCardProc(TRUE);
	if (iRet == ERR_NEED_INSERT)
	{
		return iRet;
	}
	if( iRet!=0 )
	{
#ifdef AMT_PROC_DEBUG//2014-9-22
	glProcInfo.stTranLog.cntAmt = 0;
#endif
		EcrSendTranFail();//2014-5-13 tt enhance
		return ERR_NO_DISP;
	}
	iRet = ValidCard();
	if( iRet!=0 )
	{
#ifdef AMT_PROC_DEBUG//2014-9-22
	glProcInfo.stTranLog.cntAmt = 0;
#endif
		EcrSendTranFail();//2014-5-13 tt enhance
		return iRet;
	}

	iRet = TransCapture();
	if( iRet!=0 )
	{
#ifdef AMT_PROC_DEBUG//2014-9-22
	glProcInfo.stTranLog.cntAmt = 0;
#endif
		EcrSendTranFail();//2014-5-13 tt enhance
		CommOnHook(FALSE);
		return iRet;
	}

	return 0;
}

// 处理IC卡插卡事件
// Process IC card insertion event
int ProcICCMsg(void)
{
	int		iRet;

	glProcInfo.stTranLog.ucTranType = SALE_OR_AUTH;
	iRet = GetCard(SKIP_DETECT_ICC|CARD_INSERTED);
	if( iRet!=0 )
	{
#ifdef AMT_PROC_DEBUG//2014-9-22
	glProcInfo.stTranLog.cntAmt = 0;
#endif
		 EcrSendTranFail();//2014-5-13 tt enhance
		return iRet;
	}

	iRet = TransCapture();
	if( iRet!=0 )
	{
#ifdef AMT_PROC_DEBUG//2014-9-22
	glProcInfo.stTranLog.cntAmt = 0;
#endif
		EcrSendTranFail();//2014-5-13 tt enhance
		CommOnHook(FALSE);
	
		return iRet;
	}

	return 0;
}

// 处理功能键按键事件
// Process FN key pressed event
int ProcFuncKeyMsg(void)
{
	FunctionExe(FALSE, glEdcMsgPtr->KeyValue);
	CommOnHook(FALSE);
	if (strcmp(glEdcMsgPtr->MagMsg.track1, "MANAGER")==0)
    {
		glSysParam.ucManagerOn = 1;
        PackEDCToManagerMsg(glEdcMsgPtr->MagMsg.track1, sizeof(glEdcMsgPtr->MagMsg.track1));
		SaveSysParam();
    }
	return 0;
}

// 处理普通按键事件
// Process common key pressed event
int ProcKeyMsg(void)
{
	int		iRet, iFuncNo;
	// if UserMsg points to a string "CARDNO=XXXXXX..."
#ifndef APP_MANAGER_VER
	if ((glEdcMsgPtr->UserMsg!=NULL) &&
		(memcmp((char *)(glEdcMsgPtr->UserMsg), "CARDNO=", 7)==0))
	{
	
		glProcInfo.stTranLog.ucTranType = SALE_OR_AUTH;
		iRet = GetManualPanFromMsg((char *)(glEdcMsgPtr->UserMsg)+7);	// iRet determines the flow is continue or not
		if (iRet!=0)
		{
			return iRet;
		}

		iRet = TransCapture();
		CommOnHook(FALSE);
		return ERR_NO_DISP;
	}
#endif

#if !defined(APP_MANAGER_VER) && !defined(_WIN32)
	if (glEdcMsgPtr->KeyValue==KEYCANCEL)
	{
		if (ChkIfAllowExit())
		{
			ScrCls();
			PubDispString(_T("PLEASE WAIT..."), 2|DISP_LINE_CENTER);
			return ERR_EXIT_APP;
		}
	}
#endif

	if (glSysParam.ucManagerOn==1)
	{
	
	
		//if not support contactless then premit input card NO. from manager by manually Jolie 2012-12-14  build121
		if (glSysParam.stEdcInfo.ucClssFlag==0)
		{
			//squall coded 20121106,manager can manually input card NO.
			if( glEdcMsgPtr->KeyValue>=KEY0 && glEdcMsgPtr->KeyValue<=KEY9 )
			{
				glProcInfo.stTranLog.ucTranType = SALE_OR_AUTH;
				
				iRet = ManualInputPan(glEdcMsgPtr->KeyValue);
				if( iRet!=0 )
				{
					return iRet;
				}
				iRet = TransCapture();
				if( iRet!=0 )
				{
					CommOnHook(FALSE);
					return iRet;
				}
			}
		}
		
//------------------------------------------------------//
		if (glSysParam.stEdcInfo.ucClssFlag==1 && glSysParam.stEdcInfo.ucClssMode!=1 && !ChkOnBase())
		{
		
#if defined(_P60_S1_) || defined(_S_SERIES_)
			SetOffBase(OffBaseDisplay);
#endif
			//查询当前读卡器连接状态
			//DelayMs(2000);//delay for poll synchronously
	
		//	PubShowTitle(TRUE, (uchar *)_T("      POLL      "));
		//	DispWait();
 			iRet = ClssInit();
 			if (iRet)
 			{
 				ProcError_Wave(iRet);
 				return iRet;
 			}

			if (glSysParam.stEdcInfo.ucClssMode==2 )
			{
				if (glEdcMsgPtr->KeyValue == KEYENTER)
				{
#if defined(_P60_S1_) || defined(_S_SERIES_)
					SetOffBase(OffBaseDisplay);
#endif
					memcpy(glProcInfo.stTranLog.szAmount, glSysParam.stEdcInfo.ucClssFixedAmt, 12);
#ifdef AMT_PROC_DEBUG
					//2014-9-18 each time come to glAmount or szAmount
					sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(13)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(13)%.13s",glProcInfo.stTranLog.szAmount);
				glProcInfo.stTranLog.cntAmt++;
	//			//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 16: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
					CLSSPreTrans();
					return 0;
				}
			}
			if( (glEdcMsgPtr->KeyValue>=KEY0 && glEdcMsgPtr->KeyValue<=KEY9) && glSysParam.stEdcInfo.ucClssMode==0)
			{
				if (glSysParam.ucManagerOn==1)
				{
					memcpy(&glClssInfo,glEdcMsgPtr->UserMsg,sizeof(CLSS_INFO));
					memcpy(glAmount,glClssInfo.glAmount,13);//2014-9-18 glAmount
#ifdef AMT_PROC_DEBUG
					//2014-9-18 each time come to glAmount or szAmount
					sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(14)%.13s",glAmount);
			sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(14)%.13s",glProcInfo.stTranLog.szAmount);
				glProcInfo.stTranLog.cntAmt++;
	//			//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 17: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
				}
				iRet = TransSale(FALSE);
				if( iRet!=0 )
				{
					CommOnHook(FALSE);
					return iRet;
				}
			}
		}
	}
	else
	{
		
		if( glEdcMsgPtr->KeyValue>=KEY0 && glEdcMsgPtr->KeyValue<=KEY9 )
		{
			glProcInfo.stTranLog.ucTranType = SALE_OR_AUTH;
			iRet = ManualInputPan(glEdcMsgPtr->KeyValue);
			if( iRet!=0 )
			{
				return iRet;
			}
			iRet = TransCapture();
			if( iRet!=0 )
			{
				CommOnHook(FALSE);
				return iRet;
			}
		}
	}
	iRet = ERR_NO_DISP;
	switch( glEdcMsgPtr->KeyValue )
	{
	case KEYFN:
		iFuncNo = FunctionInput();
		if( iFuncNo>=0 )
		{
			glEdcMsgPtr->KeyValue = (uchar)iFuncNo;
			ProcFuncKeyMsg();
		}
		break;

	case KEYUP:
	case KEYDOWN:
		FunctionMenu();
		break;

	case KEYMENU:
		iRet = TransOther();
		PromptRemoveICC();
		break;

#ifdef _P80_
	case KEYF1:
		iRet = TransVoid();
		break;

	case KEYF2:
		iRet = TransSettle();
		break;

	case KEYF3:
		iRet = TransRefund();
		PromptRemoveICC();
		break;

	case KEYF4:
		TransAdjust();
		break;

	case KEYF6:
		PrnLastTrans();
		break;
#endif
#ifdef _P60_S1_
	case KEYF1:
		iRet = TransVoid();
		break;

	case KEYF2:
		TransAdjust();
		break;

	case KEYF3:
		iRet = TransSettle();
		break;

	case KEYF4:
#ifdef APP_MANAGER_VER
        if (ChkIfAppEnabled(APP_CUP))
        {
            RunCup();
            break;
        }
#endif
		if (ChkIfAllowExit())
		{
			ScrCls();
			PubDispString(_T("PLEASE WAIT..."), 2|DISP_LINE_CENTER);
		 	return ERR_EXIT_APP;
		}
		if (glSysParam.stEdcInfo.ucClssFlag == 1 && glSysParam.stEdcInfo.ucClssPARAFlag==1 && (glSysParam.stEdcInfo.ucClssMode==1 || glSysParam.stEdcInfo.ucClssMode==2))
		{
			iRet = GetAmount();
			if (iRet == 0)
			{
				CLSSPreTrans();
			}	
		}
		else
		{
			PrnLastTrans();
			break;
		}
		break;
#endif
#ifdef _P70_S_
	case KEYF1:
		iRet = TransVoid();
		break;

	case KEYF2:
		PrnLastTrans();
		break;

	case KEYF3:
		TransAdjust();
		break;

	case KEYF4:
		iRet = TransSettle();
		break;
#endif
#ifdef _S_SERIES_
	case KEYATM1:
		iRet = TransVoid();
		break;
		
	case KEYATM2:
		TransAdjust();
		break;

	case KEYATM3:
		iRet = TransSettle();
		break;

	case KEYATM4:
#ifdef APP_MANAGER_VER
        if (ChkIfAppEnabled(APP_CUP))
        {
            RunCup();
            break;
        }
#endif
#ifdef PAYPASS_DEMO
			if (gucIsPayWavePass == PAYWAVE)
			{
				gucIsPayWavePass = PAYPASS;	
			}
			else if(gucIsPayWavePass == PAYPASS	)
			{
				gucIsPayWavePass = PAYWAVE;	
			}
			break;
#endif
		if (glSysParam.stEdcInfo.ucClssFlag == 1 && glSysParam.stEdcInfo.ucClssPARAFlag==1 && (glSysParam.stEdcInfo.ucClssMode==1 || glSysParam.stEdcInfo.ucClssMode==2))
		{
			iRet = GetAmount();
			if (iRet == 0)
			{
				CLSSPreTrans();
			}	
		}
		else
		{
			PrnLastTrans();
			break;
		}
		break;
#endif
	default:
		break;
	}
	if( iRet!=0 )
	{
		CommOnHook(FALSE);
		return iRet;
	}

	return 0;
}

int ProcClssMsg(uchar *pszAmount)
{
	int iRet;
#if defined(_P60_S1_) || defined(_S_SERIES_)
	SetOffBase(OffBaseDisplay);
#endif
	
#ifdef _S60_ 
	if (glSysParam.stEdcInfo.ucClssFlag==1)
	{

		PortOpen(glSysParam.stEdcInfo.ucClssComm, "38400, 8, N, 1");
	}
#endif
	 
	iRet = ClssProcTxn(pszAmount,0, SALE);	//Modify by Gillian 2016/1/29//2016-2-5

	if (iRet != 0)
	{
#ifdef AMT_PROC_DEBUG//2014-9-22
	glProcInfo.stTranLog.cntAmt = 0;
#endif
		EcrSendTranFail();
	}

#ifdef _S60_ 
        //2014-6-20 if ECR not enable, trans VOID right after SALE will fail issue
	//if (((glSysParam.stEdcInfo.ucClssFlag==1) && (ChkEdcOption(EDC_ECR_ENABLE)))
	//	|| ((glSysParam.stEdcInfo.ucClssComm != glSysParam.stECR.ucPort) && ChkEdcOption(EDC_ECR_ENABLE)))//2014-5-27 test enhance
	//{
		ClssClose(); //2014-5-16 enhance
	//}
#endif
	return iRet;
}
// end of file

