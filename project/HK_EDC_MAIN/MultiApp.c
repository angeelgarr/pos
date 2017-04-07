#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static uchar ChkIfValidAppMsgHeader(void *pstMsg);
static int ProcRecvAppMsg(uchar ucAppNo, void *pszRetStr);
static void SyncSubAppParam(char *pszSyncData, uchar ucExclueAppNo);
static int SubAppFileWrite(uchar ucAppNo, char *pszFileName, long lOffset, void *psData, int iLen);
static int GetMsgElement(char *pszSource, char *pszTag, char *pszValue, int *piLen);
static int SetMsgElement(char *pszDest, char *pszTag, char *pszValue);
static int ProcMsgElement(uchar ucAppNo, char *pszTag, char *pszValue, char *pszNewMsgBuff, uchar *pucManagerUpdated, uchar *pucSyncToSubApp);
static int GetFuncMapFromApp(uchar ucAppIdx, char *pszFuncList);
void ManagerTimerPrepare(void);
extern void GetDateTime(uchar *pszDateTime);

/********************** Internal constant declaration *********************/
// #ifdef _S_SERIES_
// //raymond 27 Jun 2011: The updated SXX EPS function number
// char szEpsFuncMapDef[] = "FUNC=1,2,4,5,11,12,21,30,40,50,72,73,74,75,76,81,87,88,89,90,91,95,99;";	
// #else
// char szEpsFuncMapDef[] = "FUNC=1,2,3,5,6,8,11,21,30,40,50,72,73,74,75,76,90,91,99;";	// 默认情况下EPS支持的FN号
// #endif
// char szEdcFuncMapDef[] = "FUNC=0,1,2,3,4,5,9,10,11,12,21,22,50,60,71,72,73,74,75,81,90,91,95,96,99;";	// 默认情况下EDC支持的FN号
// 						//build 131 squall func 12 added for manager to select app if eps is open 
/********************** Internal variables declaration *********************/
char	sg_szMsgBuff[SIZE_APPMSG_BUFF+2];

/********************** external reference declaration *********************/
extern void UpdateEpsExpiryTime(void);
extern void EpsUpdateReverseState(uchar needReverse);//2014-7-10 EPS T8 issue
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

#ifdef APP_MANAGER_VER

#ifdef _S_SERIES_
//raymond 6 Jul 2011: Save/Restore T8 timer for SXX EPS
ushort SaveEpsTimer(void)
{  
	return TimerCheck(TIMER_TEMPORARY);
}

void RestoreEpsTimer(ushort timeCnt)
{
	//elapse ~2s for in/out the app, however, the app operation cannot be counted / predicted
	//	if(timeCnt >20)
	//		timeCnt -= 20;
	
	TimerSet(TIMER_TEMPORARY, timeCnt);
}
#endif

static void ShowTime(void)
{
	uchar	szCurTime[16+1];
	GetEngTime(szCurTime);
	PubShowTitle(TRUE, szCurTime);
}

void InitEventMsg(ST_EVENT_MSG *pstEvent, int iMsgType)
{
	memset(pstEvent, 0, sizeof(ST_EVENT_MSG));
	pstEvent->MsgType = iMsgType;
}


int AppDoEvent(uchar AppNo, ST_EVENT_MSG *msg)
{
    int     iRet;
	//2014-7-17 delete old EPS T8 handling
//#ifdef _S_SERIES_        
//	ulong stTime = 0;
//	ulong endTime = 0;
//	uchar currentDayTime[15];
//	
//	long diff = 0;
//	
//    ushort timeCnt;           
//	
//    //raymond: SXX EPS - save EPS Timer
//    timeCnt = SaveEpsTimer(); 
//	
//	memset(currentDayTime, 0x00, sizeof(currentDayTime));
//	PubGetDateTime(currentDayTime);
//	stTime = PubTime2Long(currentDayTime);
//	
//	
//#endif
	if (AppNo==glSysParam.astSubApp[APP_EDC].ucAppNo)
	{
		event_main(msg);
	}

    iRet = DoEvent(AppNo, msg);

	//2014-7-17 delete old EPS T8 handling
    //if ((AppNo==glSysParam.astSubApp[APP_EPS].ucAppNo) &&
    //    glSysParam.astSubApp[APP_EPS].ucExist &&
    //    glSysParam.astSubApp[APP_EPS].ucEnabled)
    //{
    //    // Get updated EventMap. (by EPS)
    //    FileReadEventMap();
    //}
  
	//2014-7-17 delete old EPS T8 handling
//#ifdef _S_SERIES_        
//    //raymond: SXX EPS - restore EPS Timer
//	PubGetDateTime(currentDayTime);
//	endTime = PubTime2Long(currentDayTime);
//	diff = endTime - stTime;
//	if (diff >= 0)
//	{
//		diff = timeCnt - diff*10;
//		if (diff > 0)
//			RestoreEpsTimer((ushort)diff);
//		else
//			RestoreEpsTimer(10); 
//	}
//	else if (diff < 0)
//		RestoreEpsTimer(10); 
//	//    RestoreEpsTimer(timeCnt); 
//#endif
	
    return iRet;
}
// Init manager itself. 管理器自身初始化，读取应用存在信息
int ManagerInit(void)
{
	int				iRet, total_app_num, unknow_app_num;
	uchar			loop_a;
    uchar           szBuff[128], szBuff2[128];
	APPINFO			st_app_info, ast_sub_app[APP_NUM_MAX];
    ST_EVENT_MSG    StEvent;

	SystemInit();
	ScrCls();

#ifdef WIN32
    ScrPrint(0, 4, CFONT, "CANNOT SIMULATE");
    while (1);
#endif

	DispWait();
	//=========================================================
	// Check exist parameter, load or reset(if incorrect)
	if (ChkIfExistSysParam())
	{
        memset(&glSysParam,    0, sizeof(glSysParam));
        memset(&glSysParamBak, 0, sizeof(glSysParamBak));
		iRet = LoadSysParam();
		if (iRet!=0)	// When parameter inconsist. 参数大小与当前应用版本不匹配，则清除参数文件
		{
			PubShowTitle(TRUE, _T("INVALID CONFIG"));
			ScrPrint(0, 2, ASCII, "MANAGER:%.12s", AppInfo.AppVer);
			PubDispString(_T("RESET CONFIG ?  "), 6|DISP_LINE_LEFT);
			if (PubYesNo(USER_OPER_TIMEOUT)!=0)
			{
				SysHaltInfo(_T("PLS REPLACE APP"));
			}

			PubShowTitle(TRUE, _T("WARNING"));
			PubDispString(_T("DATA WILL CLEAR "), 4|DISP_LINE_LEFT);
			PubDispString(_T("CONTINUE ?      "), 6|DISP_LINE_LEFT);
			if (PubYesNo(USER_OPER_TIMEOUT)!=0)
			{
				SysHaltInfo(_T("PLS REPLACE APP"));
			}			
			ResetSysParam(TRUE);
		}
	}
	else
	{
		ResetSysParam(TRUE);
	}

	//=========================================================
	// Enumerates all apps.
    // this must do every time start-up to scan new/changed apps
	total_app_num = 0;
    unknow_app_num = 0;
	memset(ast_sub_app, 0, sizeof(ast_sub_app));
    for (loop_a=0; loop_a<APP_NUM_MAX; loop_a++)
    {
        glSysParam.astSubApp[loop_a].ucExist = FALSE;
    }
	for (loop_a=1; (loop_a<24 && total_app_num<APP_NUM_MAX); loop_a++)
	{
		iRet = ReadAppInfo(loop_a, &st_app_info);
		if (iRet!=0)
		{
			continue;
		}

        sprintf((char *)szBuff,  "%.16s", st_app_info.AID);
        sprintf((char *)szBuff2, "%.32s", st_app_info.AppName);

        if ( strcmp(szBuff, AID_EDC_MAIN)==0)
        {
			if (glSysParam.astSubApp[APP_EDC].ucExist  != EXIST_APP_OLD)
			{
				glSysParam.astSubApp[APP_EDC].ucExist   = EXIST_APP_NEW;
				glSysParam.astSubApp[APP_EDC].ucAppNo   = loop_a;
				glSysParam.astSubApp[APP_EDC].ucEnabled = TRUE;
				glSysParam.astSubApp[APP_EDC].stAppInf  = st_app_info;
				glSysParam.astSubApp[APP_EDC].ucEventMode = TRUE;
			}
			else if (glSysParam.astSubApp[APP_EDC].ucExist  == EXIST_APP_OLD)
			{
				glSysParam.astSubApp[APP_EDC_OLD].ucExist   = EXIST_APP_OLD;
				glSysParam.astSubApp[APP_EDC_OLD].ucAppNo   = glSysParam.astSubApp[APP_EDC].ucAppNo;
				glSysParam.astSubApp[APP_EDC_OLD].ucEnabled = TRUE;
				glSysParam.astSubApp[APP_EDC_OLD].stAppInf  = glSysParam.astSubApp[APP_EDC].stAppInf;
				glSysParam.astSubApp[APP_EDC_OLD].ucEventMode = glSysParam.astSubApp[APP_EDC].ucEventMode;
				
				glSysParam.astSubApp[APP_EDC].ucExist   = EXIST_APP_NEW;
				glSysParam.astSubApp[APP_EDC].ucAppNo   = loop_a;
				glSysParam.astSubApp[APP_EDC].ucEnabled = TRUE;
				glSysParam.astSubApp[APP_EDC].stAppInf  = st_app_info;
				glSysParam.astSubApp[APP_EDC].ucEventMode = TRUE;
			}           
        }
        else if (strcmp(szBuff, AID_EDC_OLD)==0)
        {
			if (glSysParam.astSubApp[APP_EDC].ucExist!=EXIST_APP_NEW)
			{
				glSysParam.astSubApp[APP_EDC].ucExist   = EXIST_APP_OLD;
				glSysParam.astSubApp[APP_EDC].ucAppNo   = loop_a;
				glSysParam.astSubApp[APP_EDC].ucEnabled = TRUE;
				glSysParam.astSubApp[APP_EDC].stAppInf  = st_app_info;
				glSysParam.astSubApp[APP_EDC].ucEventMode = TRUE;
			} 
			else if (glSysParam.astSubApp[APP_EDC].ucExist==EXIST_APP_NEW)
			{
				glSysParam.astSubApp[APP_EDC_OLD].ucExist   = EXIST_APP_OLD;
				glSysParam.astSubApp[APP_EDC_OLD].ucAppNo   = loop_a;
				glSysParam.astSubApp[APP_EDC_OLD].ucEnabled = TRUE;
				glSysParam.astSubApp[APP_EDC_OLD].stAppInf  = st_app_info;
				glSysParam.astSubApp[APP_EDC_OLD].ucEventMode = TRUE;
			}
            // Old EDC that originally run on P60S1 but trnasplanted to SXX       
        }
		else if (strcmp(szBuff, AID_EMV_BOC)==0)
		{
			glSysParam.astSubApp[APP_EDC_OLD].ucExist  = EXIST_APP_OLD;
            glSysParam.astSubApp[APP_EDC_OLD].ucAppNo  = loop_a;
			glSysParam.astSubApp[APP_EDC_OLD].ucEnabled = TRUE;
            glSysParam.astSubApp[APP_EDC_OLD].stAppInf = st_app_info;
            glSysParam.astSubApp[APP_EDC_OLD].ucEventMode = TRUE;
		}
        else if (strcmp(szBuff, AID_EDC_MULTI)==0)
        {
            // DO NOT SET ENABLE or NOT
            glSysParam.astSubApp[APP_AMEX].ucExist  = TRUE;
            glSysParam.astSubApp[APP_AMEX].ucAppNo  = loop_a;
            glSysParam.astSubApp[APP_AMEX].stAppInf = st_app_info;
            glSysParam.astSubApp[APP_AMEX].ucEventMode = TRUE;
        }
        else if (strcmp(szBuff2, APPNAME_CUP)==0)
        {
            // DO NOT SET ENABLE or NOT
            glSysParam.astSubApp[APP_CUP].ucExist  = EXIST_APP_NEW;
            glSysParam.astSubApp[APP_CUP].ucAppNo  = loop_a;
            glSysParam.astSubApp[APP_CUP].stAppInf = st_app_info;
            glSysParam.astSubApp[APP_CUP].ucEventMode = FALSE;
        }
        else if (strcmp(szBuff2, APPNAME_CUPOLD)==0)
        {
            // DO NOT SET ENABLE or NOT
            glSysParam.astSubApp[APP_CUP].ucExist  = EXIST_APP_OLD;
            glSysParam.astSubApp[APP_CUP].ucAppNo  = loop_a;
            glSysParam.astSubApp[APP_CUP].stAppInf = st_app_info;
            glSysParam.astSubApp[APP_CUP].ucEventMode = FALSE;
        }
 			else if ( strcmp(szBuff2, APPNAME_VA)==0)//squall20121126
         {
 			glSysParam.astSubApp[APP_VA].ucExist   = TRUE;
 			glSysParam.astSubApp[APP_VA].ucAppNo   = loop_a;
 			glSysParam.astSubApp[APP_VA].ucEnabled = TRUE;
 			glSysParam.astSubApp[APP_VA].stAppInf  = st_app_info;
 			glSysParam.astSubApp[APP_VA].ucEventMode = TRUE;
         }
        else if (strstr(szBuff, AID_EPS)!=NULL)
        {
            // DO NOT SET ENABLE or NOT
            glSysParam.astSubApp[APP_EPS].ucExist  = TRUE;
            glSysParam.astSubApp[APP_EPS].ucAppNo  = loop_a;
            glSysParam.astSubApp[APP_EPS].stAppInf = st_app_info;
            glSysParam.astSubApp[APP_EPS].ucEventMode = TRUE;
        }
        else
        {
            // DO NOT SET ENABLE or NOT
            glSysParam.astSubApp[APP_OTHER_START+unknow_app_num].ucExist  = TRUE;
            glSysParam.astSubApp[APP_OTHER_START+unknow_app_num].ucAppNo  = loop_a;
            glSysParam.astSubApp[APP_OTHER_START+unknow_app_num].stAppInf = st_app_info;
            glSysParam.astSubApp[APP_OTHER_START+unknow_app_num].ucEventMode = FALSE;
            unknow_app_num++;
        }

		total_app_num++;
	}

	SaveSysParam();


// 	//=========================================================
//     // Init EDC
// 	memset(&glClssInfo,0,sizeof(CLSS_INFO));
// 	glClssInfo.bFirstPollFlag=1;
//     glSysParam.ucEnableEMV = 0;
// 	InitEventMsg(&StEvent, USER_MSG);
// 	StEvent.UserMsg=&glClssInfo;
// 	AppDoEvent(glSysParam.astSubApp[APP_EDC].ucAppNo, &StEvent);
    // TODO : how to sync pwd between EDC and manager ?

	//=========================================================
    // Init EPS
	if (glSysParam.astSubApp[APP_EPS].ucExist &&
        glSysParam.astSubApp[APP_EPS].ucEnabled &&
        glSysParam.astSubApp[APP_EPS].ucEventMode)
	{
		//2014-7-17 delete old EPS T8 handling
		//ManagerTimerPrepare();		
		//if (FileReadEventMap()!=0)
		//{
  //          // Ensure the EventMap file exist before calling EPS application
		//	FileWriteEventMapDefault();
		//}
        // Initial EPS
		InitEventMsg(&StEvent, USER_MSG);
		AppDoEvent(glSysParam.astSubApp[APP_EPS].ucAppNo, &StEvent);//2014-6-13 restart POS, go to EPS reversal
		EpsUpdateReverseState(StEvent.MagMsg.track1[0]);//2014-7-10 update EPS state
	}

	//=========================================================
    // Init AMEX, if exist and enabled.
	if (glSysParam.astSubApp[APP_AMEX].ucExist &&
        glSysParam.astSubApp[APP_AMEX].ucEnabled &&
        glSysParam.astSubApp[APP_AMEX].ucEventMode)
	{
		InitEventMsg(&StEvent, USER_MSG);
		AppDoEvent(glSysParam.astSubApp[APP_AMEX].ucAppNo, &StEvent);
	}

	//=========================================================
    // Others may be init later

 //   InitFuncMapDefault();
    SaveSysParam();

	DispWait();
	return 0;
}

int ChkIfAppEnabled(uchar ucApp)
{
	PubASSERT(ucApp<APP_NUM_MAX);
	return (glSysParam.astSubApp[ucApp].ucExist && glSysParam.astSubApp[ucApp].ucEnabled);
}

void RunCup(void)
{
	RunApp(glSysParam.astSubApp[APP_CUP].ucAppNo);
}

#endif


// To determine the running mode of current application.
// return: 0--No need save; 1--Need save
int InitMultiAppInfo(void)
{
	return 0;
}

// To pack the EDC infomation for app-manager.
int PackEDCToManagerMsg(char *pszMsgBuff, int iBuffSize)
{
    char    szBuff[1024], szTmpBuf[128];

    memset(szBuff, 0, sizeof(szBuff));
	memset(szTmpBuf, 0 ,sizeof(szTmpBuf));

	// Support EMV or NOT
#ifdef ENABLE_EMV
    if (ChkIfEmvEnable())
    {
#ifdef EMV_TEST_VERSION
	    strcat(szBuff, "EMV=2;");
#else
        strcat(szBuff, "EMV=1;");
#endif
    }
    else
    {
        strcat(szBuff, "EMV=0;");
    }
#endif

    // Report : ECR enabled in EDC
    if (ChkEdcOption(EDC_ECR_ENABLE))
    {
        strcat(szBuff, "ECR=1;");
    }
    else
    {
        strcat(szBuff, "ECR=0;");
    }

	//Report : CLSS enabled
	if (glSysParam.stEdcInfo.ucClssFlag == 1)
	{
		if (glSysParam.stEdcInfo.ucClssMode == 0 )
		{
			sprintf(szTmpBuf,"CLSS=1;DECI=%.1s;",
					&glSysParam.stEdcInfo.stLocalCurrency.ucDecimal);
			strcat(szBuff, szTmpBuf);
		}	
		else if (glSysParam.stEdcInfo.ucClssMode == 2)
		{
			sprintf(szTmpBuf,"CLSS=2;AMT=%s;DECI=%.1s;",
					glSysParam.stEdcInfo.ucClssFixedAmt,
					&glSysParam.stEdcInfo.stLocalCurrency.ucDecimal);
			strcat(szBuff, szTmpBuf);			
		}
		else if (glSysParam.stEdcInfo.ucClssMode == 1)
		{
			sprintf(szTmpBuf,"CLSS=3;");
			strcat(szBuff, szTmpBuf);
		}
	} 
	else 
	{
		strcat(szBuff, "CLSS=0;");
	}
	//Report already wave pool
	if (bFirstPollFlag ==0)
	{
		strcat(szBuff, "POLL=0;");
	}
	else
	{
		strcat(szBuff, "POLL=1;");
	}

    // Report : language in use
	memset(szTmpBuf, 0, sizeof(szTmpBuf));
    strcpy(szTmpBuf, glSysParam.stEdcInfo.stLangCfg.szFileName);
    if (strstr(szTmpBuf, ".")!=NULL)
    {
        *strstr(szTmpBuf, ".") = 0;
        sprintf(szBuff+strlen(szBuff), "LANG=%s;", szTmpBuf);
    }
    else
    {
        strcat(szBuff, "LANG=ENG;");
    }
	//Report : DEMO MODE or NOT
	if (ChkIfTrainMode())
	{
		strcat(szBuff, "DEMO=1"); //  Demo Mode
	}
	else
	{
		strcat(szBuff,"DEMO=0"); // not Demo Mode
	}

    if (strlen(szBuff)+1<iBuffSize)
    {
        strcpy(pszMsgBuff, szBuff);
        return 0;
    }

    return -1;
}
void ManagerTimerPrepare(void)
{
	int	ii;
	
    for (ii=0; ii<5; ii++)
    {
		MultiAppEvent.Timers[ii].AID[0] = 0xff;
	}
}


// end of file

