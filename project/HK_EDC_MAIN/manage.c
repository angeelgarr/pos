
#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static void ClearReversalSub(void);
static int  ViewTranSub(int iStartRecNo);

/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/
extern void PrnHead_T(void);
extern void PrnHead(uchar ucFreeFmat, uchar bCompact);
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

void UnLockTerminal(void)
{
	if( ChkEdcOption(EDC_NOT_KEYBOARD_LOCKED) )
	{
		return;
	}

	while( !ChkEdcOption(EDC_NOT_KEYBOARD_LOCKED) )
	{
		PubShowTitle(TRUE, (uchar *)_T("TERMINAL  LOCKED"));
		PubDispString(_T("MERCHANT PWD"), 4|DISP_LINE_LEFT);
		PubWaitKey(-1);
		if( PasswordMerchant()==0 )
		{
			glSysParam.stEdcInfo.sOption[EDC_NOT_KEYBOARD_LOCKED>>8] |= (EDC_NOT_KEYBOARD_LOCKED & 0xFF);
			SaveEdcParam();
			PubBeepOk();
		}
	}
}

void LockTerm(void)
{
	PubShowTitle(TRUE, (uchar *)_T("LOCK TERM"));
	PubDispString(_T("TERMINAL LOCK? "), 3|DISP_LINE_LEFT);
	if( AskYesNo() )
	{
		glSysParam.stEdcInfo.sOption[EDC_NOT_KEYBOARD_LOCKED>>8] &= ~(EDC_NOT_KEYBOARD_LOCKED & 0xFF);
		SaveEdcParam();
		PubBeepOk();
		UnLockTerminal();
	}
}

void ClearAllRecord(void)
{
	PubShowTitle(TRUE, (uchar *)_T("CLR BATCH"));
	PubDispString(_T("CONFIRM CLEAR"), 3|DISP_LINE_CENTER);
	if( !AskYesNo() )
	{
		return;
	}

	DispProcess();

	ClearRecord(ACQ_ALL);
#ifdef SUPPORT_TABBATCH
    ClearTabBatchRecord(ACQ_ALL); 
#endif
	PPDCC_ResetTransStatistic(); // PP-DCC
	ScrClrLine(2, 7);
	DispClearOk();
}

void ClearConfig(void)
{
	PubShowTitle(TRUE, (uchar *)_T("CLR CONFIG"));
	PubDispString(_T("CONFIRM CLEAR"), 3|DISP_LINE_CENTER);
	if( !AskYesNo())
	{
		return;
	}
	DispProcess();

	LoadEdcDefault();

#ifdef ENABLE_EMV
	LoadEmvDefault();
#endif

	ScrClrLine(2, 7);
	DispClearOk();
}

void ClearPassword(void)
{
	PubShowTitle(TRUE, (uchar *)_T("CLR PASSWORD"));
	PubDispString(_T("CONFIRM CLEAR"), 3|DISP_LINE_CENTER);
	if( !AskYesNo())
	{
		return;
	}

	DispProcess();

	ResetPwdAll();
	SavePassword();
	ScrClrLine(2, 7);
	DispClearOk();
}

void ClearReversal(void)
{
	PubShowTitle(TRUE, (uchar *)_T("CLR REVERSAL"));
	PubDispString(_T("CONFIRM CLEAR"), 3|DISP_LINE_CENTER);
	if( !AskYesNo())
	{
		return;
	}

	DispProcess();

	ClearReversalSub();
	ScrClrLine(2, 7);
	DispClearOk();
}

void ClearReversalSub(void)
{
	int		iCnt;

	for(iCnt=0; iCnt<(int)glSysParam.ucAcqNum; iCnt++)
	{
		glSysCtrl.stRevInfo[iCnt].bNeedReversal = FALSE;
		if( glSysCtrl.sAcqStatus[iCnt]==S_PENDING )	// 为何删除结算标志???
		{
			glSysCtrl.sAcqStatus[iCnt] = S_USE;
		}
	}
	SaveSysCtrlNormal();

	// glSysCtrl.stField56
}

// 删除特定acquirer的交易记录
// Delete transaction records belonging to specific acquirer
void ClearRecord(uchar ucAcqKey)
{
	int		iCnt;

	if( ucAcqKey==ACQ_ALL )
	{
#ifdef SUPPORT_TABBATCH
		glSysCtrl.ucLastTransIsAuth = FALSE;
#endif
		glSysCtrl.uiRePrnRecNo = 0xFFFF;  //build88S

		glSysCtrl.uiLastRecNo = 0xFFFF;
		// 删除交易日志
		// Delete record
		for(iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
		{
			glSysCtrl.sAcqKeyList[iCnt]    = INV_ACQ_KEY;
			glSysCtrl.sIssuerKeyList[iCnt] = INV_ISSUER_KEY;
		}

		// 恢复收单行状态及清除冲正标志
		// Reset status and reversal flag
		for(iCnt=0; iCnt<(int)glSysParam.ucAcqNum; iCnt++)
		{
			glSysCtrl.sAcqStatus[iCnt]         = S_USE;
			glSysCtrl.stRevInfo[iCnt].bNeedReversal = FALSE;
			glSysCtrl.uiLastRecNoList[iCnt]    = 0xFFFF;
			glSysCtrl.stField56[iCnt].uiLength = 0;	// erase bit 56
			if( !(glSysParam.stAcqList[iCnt].ulCurBatchNo>0 &&
				  glSysParam.stAcqList[iCnt].ulCurBatchNo<=999999L) )
			{
				glSysParam.stAcqList[iCnt].ulCurBatchNo = 1L;
			}
		}
		SaveSysCtrlAll();
	}
	else
	{
		if( glSysCtrl.uiLastRecNo<MAX_TRANLOG &&
			glSysCtrl.sAcqKeyList[glSysCtrl.uiLastRecNo]==ucAcqKey )
		{
			glSysCtrl.uiLastRecNo = 0xFFFF;
		}



      //build88S
		if( glSysCtrl.uiRePrnRecNo < MAX_TRANLOG &&
			glSysCtrl.sAcqKeyList[glSysCtrl.uiRePrnRecNo] == ucAcqKey )
		{
			glSysCtrl.uiRePrnRecNo = 0xFFFF;
		}
      //end build88S

		// 删除交易日志
		for(iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
		{
			if( glSysCtrl.sAcqKeyList[iCnt]==ucAcqKey )
			{
				glSysCtrl.sAcqKeyList[iCnt]    = INV_ACQ_KEY;
				glSysCtrl.sIssuerKeyList[iCnt] = INV_ISSUER_KEY;
			}
		}

		// 恢复收单行状态及清除冲正标志
		for(iCnt=0; iCnt<(int)glSysParam.ucAcqNum; iCnt++)
		{
			if( glSysParam.stAcqList[iCnt].ucKey==ucAcqKey )
			{
				glSysCtrl.sAcqStatus[iCnt] = S_USE;
				glSysCtrl.stRevInfo[iCnt].bNeedReversal = FALSE;
				glSysCtrl.uiLastRecNoList[iCnt] = 0xFFFF;
				if( glSysCtrl.stField56[iCnt].uiLength>0 )
				{
					glSysCtrl.stField56[iCnt].uiLength = 0;	// erase bit 56
					SaveField56();
				}

				// increase batch no
				glSysParam.stAcqList[iCnt].ulCurBatchNo = GetNewBatchNo(glSysParam.stAcqList[iCnt].ulCurBatchNo);

				break;
			}
		}
		SaveSysCtrlNormal();
	}

	SaveSysParam();
}


#ifdef SUPPORT_TABBATCH
void ClearTabBatchRecord(uchar ucAcqKey)
{
	int		iCnt;

	if( ucAcqKey==ACQ_ALL )
	{
		// Delete record
		for(iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
		{
			glSysCtrl.astAuthRecProf[iCnt].ucAcqKey = INV_ACQ_KEY;
            memset(glSysCtrl.astAuthRecProf[iCnt].sDateBCD, 0, LEN_DATE_BCD);
		}

		// Reset status and reversal flag
		for(iCnt=0; iCnt<(int)glSysParam.ucAcqNum; iCnt++)
		{
			//glSysCtrl.sAcqStatus[iCnt]         = S_USE;
			//glSysCtrl.stRevInfo[iCnt].bNeedReversal = FALSE;
		}
		SaveSysCtrlAll();
	}
	else
	{
		// 删除交易日志
		for(iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
		{
			if( glSysCtrl.astAuthRecProf[iCnt].ucAcqKey==ucAcqKey )
			{
				glSysCtrl.astAuthRecProf[iCnt].ucAcqKey = INV_ACQ_KEY;
                memset(glSysCtrl.astAuthRecProf[iCnt].sDateBCD, 0, LEN_DATE_BCD);
			}
		}

		// 恢复收单行状态及清除冲正标志
		for(iCnt=0; iCnt<(int)glSysParam.ucAcqNum; iCnt++)
		{
			if( glSysParam.stAcqList[iCnt].ucKey==ucAcqKey )
			{
				//glSysCtrl.sAcqStatus[iCnt] = S_USE;
				//glSysCtrl.stRevInfo[iCnt].bNeedReversal = FALSE;
				break;
			}
		}
		SaveSysCtrlNormal();
	}

	SaveSysParam();
}
#endif


#ifdef SUPPORT_TABBATCH
void AutoRemoveExpiredAuth(void)
{
    uchar   ucSign, sLogTime[16], sNowTime[16];
    int     iCnt;
    ulong   ulInterval;
    uchar   bNeedSave = FALSE;

    if (glSysParam.stEdcInfo.ucEnableAuthTabBatch!=3)
    {
        return;
    }
    if (glSysParam.stEdcInfo.ucTabBatchExpDates==0)
    {
        return;
    }

    PubGetDateTime(sNowTime);
    //PubAsc2Bcd(sLogTime, 14, sNowTime);

	// Delete expired record
	for(iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
	{
        if (glSysCtrl.astAuthRecProf[iCnt].ucAcqKey==INV_ACQ_KEY)
        {
            continue;
        }
        memcpy(sLogTime, glSysCtrl.astAuthRecProf[iCnt].sDateBCD, LEN_DATE_BCD);
        PubBcd2Asc0(glSysCtrl.astAuthRecProf[iCnt].sDateBCD, LEN_DATE_BCD, sLogTime);
        ulInterval = PubCalInterval(sLogTime, sNowTime, "DD", &ucSign);
        if (ulInterval>=glSysParam.stEdcInfo.ucTabBatchExpDates)
        {
            // expired. erase.
            glSysCtrl.astAuthRecProf[iCnt].ucAcqKey = INV_ACQ_KEY;
            memset(glSysCtrl.astAuthRecProf[iCnt].sDateBCD, 0, LEN_DATE_BCD);
            bNeedSave = TRUE;
        }
	}

    if (bNeedSave)
    {
        SaveSysCtrlAll();
	    SaveSysParam();
    }
}
#endif

#ifdef APP_MANAGER_VER
void EnableProc(void)
{
	int			ii, iCnt;
	MenuItem	stAppList[APP_NUM_MAX+2];
	uchar		ucAppNum, sAppNumCandidate[APP_NUM_MAX+2];
	 ST_EVENT_MSG    StEvent;
	if (PasswordBank()!=0)
	{
		return;	
	}
	
	while (1)
	{
		memset(stAppList, 0, sizeof(stAppList));
		
		// 苯核莱ノ垫虫
		iCnt = 0;
		for (ii=1; ii<APP_NUM_MAX; ii++)
		{
			if (glSysParam.astSubApp[ii].ucExist)
			{
				sAppNumCandidate[iCnt] = ii;
				stAppList[iCnt].bVisible = TRUE;
				stAppList[iCnt].pfMenuFunc = NULL;
				sprintf(stAppList[iCnt].szMenuName, "%-14.14s %.3s",
					glSysParam.astSubApp[ii].stAppInf.AppName,
					(glSysParam.astSubApp[ii].ucEnabled ? "ON" : "OFF"));
				iCnt++;
			}
		}
		
		if (iCnt==0)
		{
            PubShowTitle(TRUE, _T("SUB-APP CONTROL"));
            PubDispString(_T("NO OTHER APP"), 4|DISP_LINE_LEFT);
            PubWaitKey(USER_OPER_TIMEOUT);
			return;
		}
		
		ii = PubGetMenu("SUB-APP CONTROL", stAppList, MENU_AUTOSNO|MENU_ASCII, USER_OPER_TIMEOUT);
		if (ii<0)
		{
            SaveSysParam();
			return;
		}
		

		ucAppNum = sAppNumCandidate[ii];
		glSysParam.astSubApp[ucAppNum].ucEnabled = !glSysParam.astSubApp[ucAppNum].ucEnabled;
		if (glSysParam.astSubApp[ucAppNum].ucEnabled)
		{
			if (ucAppNum==APP_AMEX)
			{
				InitEventMsg(&StEvent, USER_MSG);
				AppDoEvent(glSysParam.astSubApp[APP_AMEX].ucAppNo, &StEvent);
			}
// 			else
// 			{
// 				InitEventMsg(&StEvent, USER_MSG);
// 				AppDoEvent(glSysParam.astSubApp[APP_EDC].ucAppNo, &StEvent);
// 			}
		}
	
	}
}
#endif


// 清除终端数据界面
// Interface of "Clear". (FUNC99) 
void DoClear(void)
{

	static MenuItem stClearMenu[] =
	{
		{TRUE, "CLR CONFIG",	ClearConfig},
		{TRUE, "CLR BATCH",		ClearAllRecord},
		{TRUE, "CLR REVERSAL",	ClearReversal},
		{TRUE, "CLR PWD",		ClearPassword},
		{TRUE, "",	NULL},
	};
	PubShowTitle(TRUE, (uchar *)_T("CLEAR"));
	if( PasswordBank()!=0 )
	{
		return;
	}

	PubGetMenu((uchar *)_T("CLEAR"),            stClearMenu, MENU_AUTOSNO|MENU_ASCII, USER_OPER_TIMEOUT);
}

// 查看交易汇总
// View total. (glTransTotal)
void ViewTotal(void)
{
	CalcTotal(ACQ_ALL);
	DispTransTotal(TRUE);
}
#ifdef APP_MANAGER_VER
int SelectAppNum(uchar ucIncluMngr, uchar ucForEventMode, uchar *pucAppNum)
{
	int			ii, iCnt;
	MenuItem	stAppList[APP_NUM_MAX+2];
	uchar		sAppNumCandidate[APP_NUM_MAX+2];
	
	memset(stAppList, 0, sizeof(stAppList));
	iCnt = 0;

	// Add Manager into menu
	if (ucIncluMngr)
	{
		sAppNumCandidate[iCnt] = 0;
		stAppList[iCnt].bVisible = TRUE;
		stAppList[iCnt].pfMenuFunc = NULL;
		sprintf(stAppList[iCnt].szMenuName, "%.14s", AppInfo.AppName);
		iCnt++;
	}
	// Scan all fulfilled applications
	for (ii=0; ii<APP_NUM_MAX; ii++)
	{
        // Exist and enabled
		if (!glSysParam.astSubApp[ii].ucExist || !glSysParam.astSubApp[ii].ucEnabled)
        {	
            continue;
        }
        // whether this app support DoEvent()
		if (ucForEventMode && !glSysParam.astSubApp[ii].ucEventMode)
        {
            continue;
		}
        // Add into menu
		sAppNumCandidate[iCnt] = glSysParam.astSubApp[ii].ucAppNo;
		stAppList[iCnt].bVisible = TRUE;
		stAppList[iCnt].pfMenuFunc = NULL;
		sprintf(stAppList[iCnt].szMenuName, "%.14s", glSysParam.astSubApp[ii].stAppInf.AppName);
		iCnt++;
	}
	
	if (iCnt==0)
	{
		return -1;
	}
	
	if (iCnt==1)
	{
		*pucAppNum = sAppNumCandidate[0];
		return 0;
	}
	
#ifdef _S_SERIES_   
	//raymond Build020: SXX EPS - show Title "SELECT APP" when multiple apps is available
	ii = PubGetMenu((uchar *)_T("SELECT APP"), stAppList, MENU_AUTOSNO, USER_OPER_TIMEOUT); 
#else
	ii = PubGetMenu(NULL, stAppList, MENU_AUTOSNO, USER_OPER_TIMEOUT);
#endif
	
	if (ii<0)
	{
		return -1;
	}
	*pucAppNum = sAppNumCandidate[ii];
	return 0;
}
#endif
// 查看所有交易记录
// View all transaction record
void ViewTranList(void)
{
	if( GetTranLogNum(ACQ_ALL)==0 )
	{
		PubShowTitle(TRUE, (uchar *)_T("EDC       REVIEW"));
		PubDispString(_T("EMPTY BATCH"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(5);
		return;
	}

	ViewTranSub(-1);	// 浏览所以交易
}

// 查看指定交易记录
// View specific record
void ViewSpecList(void)
{
	int			iRet;
	TRAN_LOG	stLog;

	while (1)
	{
		PubShowTitle(TRUE, (uchar *)"EDC       REVIEW");
		if( GetTranLogNum(ACQ_ALL)==0 )
		{
			PubShowTitle(TRUE, (uchar *)"EDC       REVIEW");
			PubDispString(_T("EMPTY BATCH"), 4|DISP_LINE_LEFT);
			PubBeepErr();
			PubWaitKey(5);
			return;
		}

		iRet = GetRecord(TS_ALL_LOG, &stLog);
		if( iRet!=0 )
		{
			return;
		}

		if (ViewTranSub((int)glProcInfo.uiRecNo)!=0)
		{
			break;
		}
	}
}

// 交易记录浏览控制
// 返回：ERR_USERCANCEL--取消或超时退出；0--其它按键（或原因）退出
int ViewTranSub(int iStartRecNo)
{
	uchar		ucKey;
	int			iRecNo, iStep, iCnt, iActRecNo;
	TRAN_LOG	stLog;

	iRecNo = iStartRecNo;
	iStep  = iStartRecNo<0 ? 1 : 0;
	while( 1 )
	{
		iRecNo = iRecNo + iStep;
		if( iRecNo>=MAX_TRANLOG )
		{
			ScrCls();
			PubDispString(_T("END OF BATCH"), 4|DISP_LINE_LEFT);
			DelayMs(1000);
			iRecNo = 0;
		}
		else if( iRecNo<0 )
		{
			ScrCls();
			PubDispString(_T("START OF BATCH"), 4|DISP_LINE_LEFT);
			DelayMs(1000);
			iRecNo = MAX_TRANLOG-1;
		}
		if( glSysCtrl.sAcqKeyList[iRecNo]==INV_ACQ_KEY )
		{
			continue;
		}
		memset(&stLog, 0, sizeof(TRAN_LOG));
		LoadTranLog(&stLog, (ushort)iRecNo);
		FindIssuer(stLog.ucIssuerKey);//squall revise 2013.11.11
		for(iActRecNo=iCnt=0; iCnt<=iRecNo; iCnt++)
		{
			if( glSysCtrl.sAcqKeyList[iCnt]!=INV_ACQ_KEY )
			{
				iActRecNo++;
			}
		}
		DispTranLog((ushort)iActRecNo, &stLog);

		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( ucKey==KEYCANCEL || ucKey==NOKEY )
		{
			return ERR_USERCANCEL;
		}
		else if( ucKey==KEYDOWN || ucKey==KEYENTER )
		{
			iStep = 1;
		}
		else if( ucKey==KEYUP || ucKey==KEYCLEAR )
		{
			iStep = -1;
		}
		else
		{
			iStep = 0;
		}

		if (iStartRecNo>=0)		// 查阅指定记录。不上下翻页
		{
			return 0;
		}
	}
}


#ifdef SUPPORT_TABBATCH
void ViewAuthList(void)
{
	ViewAuthSub(-1);	// 浏览所有交易
}
#endif


#ifdef SUPPORT_TABBATCH
int ViewAuthSub(int iStartRecNo)
{
	uchar		ucKey;
	int			iRecNo, iStep, iCnt;
	TRAN_LOG	stLog;
    ushort      uiSequence[MAX_TRANLOG];

    // Sort the records by time
    memset(uiSequence, 0, sizeof(uiSequence));
    iRecNo = 0;
    for (iCnt=0; iCnt<MAX_TRANLOG; iCnt++)
    {
        if (glSysCtrl.astAuthRecProf[iCnt].ucAcqKey==INV_ACQ_KEY)
        {
            continue;
        }

        if (iRecNo==0)
        {
            uiSequence[iRecNo++] = (ushort)iCnt;    
        }
        else
        {
            // compare date one by one
            for (iStep=0; iStep<iRecNo; iStep++)
            {
                if (memcmp(glSysCtrl.astAuthRecProf[iCnt].sDateBCD,
                    glSysCtrl.astAuthRecProf[uiSequence[iStep]].sDateBCD,
                    LEN_DATE_BCD) < 0)
                {
                    // insert
                    memmove(&uiSequence[iStep+1], &uiSequence[iStep], sizeof(ushort)*(iRecNo-iStep));
                    uiSequence[iStep] = (ushort)iCnt;
                    iRecNo++;
                    break;
                }
            }
            if (iStep>=iRecNo)
            {
                // append
                uiSequence[iStep] = (ushort)iCnt;
                iRecNo++;
            }
        }
    }

	if( iRecNo==0 )
	{
		PubShowTitle(TRUE, (uchar *)_T("EDC AUTH REVIEW "));
		PubDispString(_T("EMPTY BATCH"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(5);
		return ERR_NO_DISP;
	}

	iStep = 0;
	while( 1 )
	{
		if( iStep>=iRecNo )
		{
			ScrCls();
			PubDispString(_T("END OF BATCH"), 4|DISP_LINE_LEFT);
			DelayMs(1000);
			iStep = 0;
		}
		else if( iStep<0 )
		{
			ScrCls();
			PubDispString(_T("START OF BATCH"), 4|DISP_LINE_LEFT);
			DelayMs(1000);
			iStep = iRecNo-1;
		}
        else
        {
		    memset(&stLog, 0, sizeof(TRAN_LOG));
		    LoadTabBatchLog(&stLog, uiSequence[iStep]);
            FindAcq(stLog.ucAcqKey);
            FindIssuer(stLog.ucIssuerKey);
		    DispAuthLog(iStep+1, iRecNo, &stLog);

	        ucKey = PubWaitKey(USER_OPER_TIMEOUT);
	        if( ucKey==KEYCANCEL || ucKey==NOKEY )
	        {
		        return ERR_USERCANCEL;
	        }
	        else if( ucKey==KEYDOWN || ucKey==KEYENTER )
	        {
		        iStep++;
	        }
	        else if( ucKey==KEYUP || ucKey==KEYCLEAR )
	        {
		        iStep--;
	        }
        }
	}
}
#endif

void PrnLastTrans(void)
{
	int			iRet;

	PubShowTitle(TRUE, (uchar *)_T("REPRINT         "));
        /*build88S
	if( glSysCtrl.uiLastRecNo>=MAX_TRANLOG )*/
#ifdef SUPPORT_TABBATCH
	if(( glSysCtrl.uiRePrnRecNo >= MAX_TRANLOG ) && (glSysCtrl.ucLastTransIsAuth == FALSE))
#else
        if( glSysCtrl.uiRePrnRecNo >= MAX_TRANLOG )
#endif
	{
		PubDispString(_T("EMPTY BATCH"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(5);
		return;
	}

	InitTransInfo();
        /*build88S
	iRet = LoadTranLog(&glProcInfo.stTranLog, glSysCtrl.uiLastRecNo);*/
#ifdef SUPPORT_TABBATCH
	if(glSysCtrl.ucLastTransIsAuth == TRUE)
	{
		iRet = GetAuthRecordByInvoice(TS_ALL_LOG, glSysCtrl.ulLastAuthInvoiceNo, &glProcInfo.stTranLog);
	}
	else
	{
		iRet = LoadTranLog(&glProcInfo.stTranLog, glSysCtrl.uiRePrnRecNo);
	}
#else
	iRet = LoadTranLog(&glProcInfo.stTranLog, glSysCtrl.uiRePrnRecNo);
#endif
	if( iRet!=0 )
	{
		return;
	}

	PubShowTitle(TRUE, (uchar *)_T("REPRINT         "));
	FindAcq(glProcInfo.stTranLog.ucAcqKey);
	FindIssuer(glProcInfo.stTranLog.ucIssuerKey);
	PrintReceipt(PRN_REPRINT);
}

void RePrnSpecTrans(void)
{
	int			iRet;

	PubShowTitle(TRUE, (uchar *)_T("REPRINT         "));
#ifdef SUPPORT_TABBATCH
	PubDispString("AUTH TXN ? ", 3|DISP_LINE_LEFT); 
	if( AskYesNo() )
	{
		RePrnSpecAuth();
		return;
	}
	else
	{
#endif
	if( GetTranLogNum(ACQ_ALL)==0 )
	{
		PubDispString(_T("EMPTY BATCH"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(5);
		return;
	}

	InitTransInfo();
	iRet = GetRecord(TS_ALL_LOG, &glProcInfo.stTranLog);
	if( iRet!=0 )
	{
		return;
	}

	PubShowTitle(TRUE, (uchar *)_T("REPRINT         "));
	FindAcq(glProcInfo.stTranLog.ucAcqKey);
	FindIssuer(glProcInfo.stTranLog.ucIssuerKey);
	PrintReceipt(PRN_REPRINT);
#ifdef SUPPORT_TABBATCH
	}
#endif
}

#ifdef SUPPORT_TABBATCH
void RePrnSpecAuth(void)
{
	int			iRet;

	PubShowTitle(TRUE, (uchar *)_T("REPRINT AUTH    "));
	if( GetTabBatchLogNum(ACQ_ALL)==0 )
	{
		PubDispString(_T("EMPTY BATCH"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(5);
		return;
	}

	InitTransInfo();
	iRet = GetAuthRecord(TS_ALL_LOG, &glProcInfo.stTranLog);
	if( iRet!=0 )
	{
		return;
	}

	PubShowTitle(TRUE, (uchar *)_T("REPRINT AUTH    "));
	FindAcq(glProcInfo.stTranLog.ucAcqKey);
	FindIssuer(glProcInfo.stTranLog.ucIssuerKey);
	PrintReceipt(PRN_REPRINT);
}
#endif

void PrnTotal(void)//func74
{
	int		iRet;
	uchar	szTitle[16+1], ucIndex;

	PubShowTitle(TRUE, (uchar *)_T("PRINT TOTAL     "));
	sprintf((char *)szTitle, _T("PRINT TOTAL     "));
	iRet = SelectAcq(TRUE, szTitle, &ucIndex);
	if( iRet!=0 )
	{
		return;
	}
	//MAX_ACQ
	if( ucIndex==MAX_ACQ )
	{
		CalcTotal(ACQ_ALL);
	}
	else
	{
		CalcTotal(glCurAcq.ucKey);
	}


	PubShowTitle(TRUE, (uchar *)_T("PRINT TOTAL     "));
	if (ChkIfZeroTotal(&glTransTotal))
	{
		PubDispString(_T("EMPTY BATCH"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(3);
		return ;
	}

	if( ucIndex!=MAX_ACQ )
	{
		return PrnTotalAcq();
	}
	for(ucIndex=0; ucIndex<glSysParam.ucAcqNum; ucIndex++)
	{
		SetCurAcq(ucIndex);
		memcpy(&glTransTotal, &glAcqTotal[ucIndex], sizeof(TOTAL_INFO));
		PrnTotalAcq();
	}
	//2013-10-3 need to print terminal total!
	PrnTotalAll(1);
}

void RePrnSettle(void)
{
	int		iRet;
	uchar	szTitle[16+1];

	PubShowTitle(TRUE, (uchar *)_T("REPRINT SETTLE  "));
	sprintf((char *)szTitle, _T("REPRINT SETTLE  "));
	iRet = SelectAcq(FALSE, szTitle, NULL);
	if( iRet!=0 )
	{
		return;
	}

	PubShowTitle(TRUE, (uchar *)_T("REPRINT SETTLE  "));
	if( !glSysCtrl.stRePrnStlInfo.bValid[glCurAcq.ucIndex] )
	{
		PubDispString(_T("NO RECEIPT"), 4|DISP_LINE_LEFT);
		PubWaitKey(3);
		return;
	}

	sprintf((char *)glProcInfo.szSettleMsg, "%s", glSysCtrl.stRePrnStlInfo.szSettleMsg[glCurAcq.ucIndex]);
	memcpy(&glTransTotal, &glSysCtrl.stRePrnStlInfo.stAcqTotal[glCurAcq.ucIndex], sizeof(TOTAL_INFO));
	memcpy(glIssuerTotal, glSysCtrl.stRePrnStlInfo.stIssTotal[glCurAcq.ucIndex], sizeof(glIssuerTotal));
	glCurAcq.ulCurBatchNo = glSysCtrl.stRePrnStlInfo.ulBatchNo[glCurAcq.ucIndex];

	DispPrinting();
	PrintSettle(PRN_REPRINT);
}

#ifdef ENABLE_EMV
// Print EMV error log message
void PrintEmvErrLog(void)
{
	ushort			uiCnt, uiActNum, uiTemp;
	uchar			szBuff[50];
	EMV_ERR_LOG		stErrLog;

	PubShowTitle(TRUE, (uchar *)_T("PRINT ERROR LOG"));
	if( PasswordBank()!=0 )
	{
		return;
	}

	PubShowTitle(TRUE, (uchar *)_T("PRINT ERROR LOG"));
	DispProcess();

	PrnInit();
	PrnSetNormal();
	PrnStr("  EMV ERROR LOG\n\n");

	for(uiActNum=uiCnt=0; uiCnt<MAX_ERR_LOG; uiCnt++)
	{
		memset(&stErrLog, 0, sizeof(EMV_ERR_LOG));
		LoadErrLog(uiCnt, &stErrLog);
		if( !stErrLog.bValid )
		{
			continue;
		}

		uiActNum++;
		PrnStr("\nSTAN: %06lu\n", stErrLog.ulSTAN);
		PubBcd2Asc0(stErrLog.sAID, stErrLog.ucAidLen, szBuff);
		PrnStr("AID: %s\n", szBuff);
		PrnStr("PAN: %s\n", stErrLog.szPAN);
		PrnStr("PAN SEQ #: %02X\n", stErrLog.ucPANSeqNo);
		PrnStr("AMT: %.12s\n", stErrLog.szAmount);
		PrnStr("TIP: %.12s\n", stErrLog.szTipAmt);
		PrnStr("RSP: %.2s\n",  stErrLog.szRspCode);
		PrnStr("RRN: %.12s\n", stErrLog.szRRN);
		PrnStr("AUT: %.6s\n",  stErrLog.szAuthCode);
		PrnStr("TVR: %02X %02X %02X %02X %02X\n", stErrLog.sTVR[0], stErrLog.sTVR[1],
				stErrLog.sTVR[2], stErrLog.sTVR[3], stErrLog.sTVR[4]);
		PrnStr("TSI: %02X %02X\n", stErrLog.sTSI[0], stErrLog.sTSI[1]);

		PrnStr("REQ BIT 55:\n");
		for(uiTemp=0; uiTemp<stErrLog.uiReqICCDataLen; uiTemp++)
		{
			PrnStr("%02X %s", stErrLog.sReqICCData[uiTemp], (uiTemp%10)==9 ? "\n" : "");
		}
		PrnStr("%s", uiTemp>0 ? "\n" : "");

		PrnStr("REQ BIT 56:\n");
		for(uiTemp=0; uiTemp<stErrLog.uiReqField56Len; uiTemp++)
		{
			PrnStr("%02X %s", stErrLog.sReqField56[uiTemp], (uiTemp%10)==9 ? "\n" : "");
		}
		PrnStr("%s", uiTemp>0 ? "\n" : "");

		PrnStr("RSP BIT 55:\n");
		for(uiTemp=0; uiTemp<stErrLog.uiRspICCDataLen; uiTemp++)
		{
			PrnStr("%02X ", stErrLog.sRspICCData[uiTemp]);
		}
		PrnStr("%s", uiTemp>0 ? "\n" : "");

		if( (uiActNum%5)==4 )
		{
			if( StartPrinter()!=0 )
			{
				return;
			}

			PrnInit();
			PrnSetNormal();
		}
	}

	if (uiActNum>0)
	{
		PrnStr("%s", (ChkIfThermalPrinter() ? "\n\n\n\n\n\n" : "\f"));
	}
	else
	{
		PrnStr("\n  ( NO RECORD )\n\n\n\n\n\n");
	}

	StartPrinter();
}
#endif

// end of file

