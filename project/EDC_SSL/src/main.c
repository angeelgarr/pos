/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2006-? PAX Technology, Inc. All rights reserved.
 * Description:
 *
 * Revision History:
 * Date                  Author                 Action
 * 2006.09.12         shengjx                    created
 * ============================================================================
 */

#include "global.h"

/********************** Internal macros declaration ************************/
#define TIMER_TEMPORARY		4
#define TIMERCNT_MAX		48000

/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static int   main_sub(const ST_EVENT_MSG *pstEventMsg);
static int   CheckTermSatus(const ST_EVENT_MSG *pstEventMsg);
static void  FirstRunProc(const ST_EVENT_MSG *pstEventMsg);
static void  SetIdleTimer(void);
static uchar ChkIdleTimer(int iSeconds);
static void  ShowIdleMainUI(uchar *pbForceUpdate, uchar bShowGallery, int iGallery_Image_Num);
static void  ShowGallery(int num);
static int   CheckInitTerminal(void);
static int  MainIdleProc(void);
/********************** Internal variables declaration *********************/
int Refresh_UpDown(gui_callbacktype_t type, void *data, int *len);

/********************** external reference declaration *********************/
extern int event_main(ST_EVENT_MSG *pstEventMsg);

const APPINFO AppInfo =
{
	APP_NAME,
	EDCAPP_AID,
	EDC_VER_INTERN,
	"PAX TECHNOLOGY",
	"EDC TEMPLATE FOR VISA/MC PROJECTS",
	"",
	0xFF,
	0xFF,
	0x01,
	""
};

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/


int event_main(ST_EVENT_MSG *pstEventMsg)
{
	glEdcMsgPtr = pstEventMsg;	// save the event pointer 
	return main_sub(pstEventMsg);
}

int main(void)
{
	uchar			bUpdateUI, bGallery = FALSE;
	int				iRet;
	ST_EVENT_MSG	stEventMsg;
	int iGallery_Image_Num = 0;
	uchar szGallery_Image_Num[5]= {0};
#ifdef APP_DEBUG
 	ST_KEY_INFO     stTmp_Key;
 	ST_KCV_INFO		stTmp_Kcv;
#endif

	OsLogSetTag(APP_NAME);
    OsRegGetValue("ro.fac.mach", gl_szTerminalType);

    // Application initialization
	memset(&stEventMsg, 0, sizeof(stEventMsg));
	stEventMsg.MsgType = USER_MSG;
	iRet = event_main(&stEventMsg);
	if (iRet == ERR_EXIT_APP)
	{
//	    CommOnHook(TRUE);
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//temp
#ifdef APP_DEBUG
	OsPedEraseKeys();
 	stTmp_Key.ucSrcKeyType = PED_TMK;
 	stTmp_Key.ucSrcKeyIdx  = 0;
 	stTmp_Key.ucDstKeyType = PED_TMK;
 	stTmp_Key.ucDstKeyIdx  = MASTER_KEY_ID;
 	memcpy(stTmp_Key.aucDstKeyValue,
 	        "\xAB\xCD\xEF\x01\x23\x45\x67\x89"\
 	        "\xAB\xCD\xEF\x01\x23\x45\x67\x89",
 	        16);
 	stTmp_Key.iDstKeyLen = 16;
 	stTmp_Kcv.iCheckMode =0;
	PedWriteKey(&stTmp_Key, &stTmp_Kcv);

 	stTmp_Key.ucSrcKeyType = PED_TMK;
 	stTmp_Key.ucSrcKeyIdx  = MASTER_KEY_ID;
 	stTmp_Key.ucDstKeyType = PED_TPK;
 	stTmp_Key.ucDstKeyIdx  = DEF_PIN_KEY_ID;
 	memcpy(stTmp_Key.aucDstKeyValue,
 	        "\xAB\xCD\xEF\x01\x23\x45\x67\x89"\
 	        "\xAB\xCD\xEF\x01\x23\x45\x67\x89",
 	        16);
 	stTmp_Key.iDstKeyLen = 16;
 	stTmp_Kcv.iCheckMode =0;
 	PedWriteKey(&stTmp_Key, &stTmp_Kcv);

    stTmp_Key.ucSrcKeyType = PED_TMK;
    stTmp_Key.ucSrcKeyIdx  = MASTER_KEY_ID;
    stTmp_Key.ucDstKeyType = PED_TDK;
    stTmp_Key.ucDstKeyIdx  = DEF_DATA_KEY_ID;
    memcpy(stTmp_Key.aucDstKeyValue,
            "\x12\x12\x12\x12\x12\x12\x12\x12"\
            "\x12\x12\x12\x12\x12\x12\x12\x12",
            16);
    stTmp_Key.iDstKeyLen = 16;
    stTmp_Kcv.iCheckMode =0;
    PedWriteKey(&stTmp_Key, &stTmp_Kcv);
#endif
	//////////////////////////////////////////////////////////////////////////

    // Main loop
	bUpdateUI = TRUE;
	if(0 == GetEnv("PIC_NO", szGallery_Image_Num))
	{
	    iGallery_Image_Num = atoi(szGallery_Image_Num);
	}
	else
	{
	    iGallery_Image_Num = 0;
	}

	//RunApp(1); test

	while( 1 )
	{
		// Setup idle timer
		if (bUpdateUI)
		{
			SetIdleTimer();
		}

		ShowIdleMainUI(&bUpdateUI, bGallery, iGallery_Image_Num);	// bUpdateUI value may change

        // When magstripe card swiped
		if( MagSwiped()==0 )
		{
			memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));
			stEventMsg.MsgType = MAGCARD_MSG;
			stEventMsg.MagMsg.RetCode = MagRead(stEventMsg.MagMsg.track1,
												stEventMsg.MagMsg.track2,
												stEventMsg.MagMsg.track3);
			iRet = event_main(&stEventMsg);
			if (iRet==ERR_EXIT_APP)
			{
//			    CommOnHook(TRUE);
				return 0;
			}
			bUpdateUI = TRUE;
			bGallery = FALSE;
			CommOnHook(FALSE);
			continue;
		}

        // When chip card inserted
		if( ChkIfEmvEnable() && IccDetect(ICC_USER)==0 )
		{
			memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));
			stEventMsg.MsgType = ICCARD_MSG;
			iRet = event_main(&stEventMsg);
			if (iRet==ERR_EXIT_APP)
			{
//			    CommOnHook(TRUE);
				return 0;
			}
			bUpdateUI = TRUE;
			bGallery = FALSE;
			CommOnHook(FALSE);
			continue;
		}

        // If any key is pressed
		if( 0==kbhit() )
		{
			memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));
			stEventMsg.MsgType  = KEYBOARD_MSG;
			stEventMsg.KeyValue = getkey();
			// Add by lirz v1.02.0000
			// If key enter,it repaint the main UI(mainly logo) again and again,do not do this
			if(KEYENTER == stEventMsg.KeyValue)
			{
			    //RunApp(1);  test
				continue;
			}
			iRet = event_main(&stEventMsg);
			if (iRet==ERR_EXIT_APP)
			{
			    SetCurrTitle(_T("Exit"));
//			    CommOnHook(TRUE);
				if(PasswordBank() == 0)
				{
					return 0;
				}
			}
			bUpdateUI = TRUE;
			bGallery = FALSE;
			CommOnHook(FALSE);
			continue;
		}

        // bug 803 change the sequence of 2 ChkIdleTimer
        if (ChkIdleTimer(glSysParam.stEdcInfo.ucIdleMinute * 60 /2))
        {
            bGallery = TRUE;
        }

		// One of idle-processing
		if (ChkIdleTimer(glSysParam.stEdcInfo.ucIdleMinute * 60))
		{
			if(0 == MainIdleProc())
			{
                bUpdateUI = TRUE;
                bGallery = FALSE;
			}
			continue;
		}

        // TODO:add your event-handlers here...

	} // end of while( 1
	return 0;
}

// event processing
int main_sub(const ST_EVENT_MSG *pstEventMsg)
{
	int	iRet;

	SystemInit();

#ifdef ENABLE_EMV
	iRet = EMVCoreInit();
	if( iRet==EMV_KEY_EXP )
	{
		EraseExpireCAPK();
	}
#endif

	SetOffBase(NULL);

	CheckTermSatus(pstEventMsg);

#ifndef APP_MANAGER_VER
	// Process manager attached administrative message.
	// Not implemented
#endif

	iRet = ERR_NO_DISP;
	switch( pstEventMsg->MsgType )
	{
	case USER_MSG:
		ProcUserMsg();
		break;
	
	case EDCAPP_AUTOUPDATE:
		AutoDownParam();
	    break;
	
	case EDCAPP_GETPARA:
		GetAutoDownParam();
	    break;

	case EDC_ECR_MSG:
		iRet = ProcEcrMsg();
		break;
	
	case MAGCARD_MSG:
		iRet = ProcMagMsg();
		break;
	
	case ICCARD_MSG:
		iRet = ProcICCMsg();
		PromptRemoveICC();
	    break;
	case EPSAPP_FUNCKEYMSG:
		ProcFuncKeyMsg();
	    break;
	case KEYBOARD_MSG:
		iRet = ProcKeyMsg();
		break;

	case APPMSG_SYNC:
		break;

	default:
	    break;
	}
	if( iRet!=0 )
	{
		DispResult(iRet);
		Gui_ClearScr(); // Added by Kim_LinHB 2014-08-13 v1.01.0003 bug512
	}

	SetOffBase(NULL);

    kbflush();
	CheckInitTerminal();
	UnLockTerminal();

	if (iRet==ERR_EXIT_APP)
	{
		return ERR_EXIT_APP;
	}

#ifndef APP_MANAGER_VER
	// Response to manager admin msg
#endif

	return iRet; // Modified by Kim_LinHB 2014-08-13 v1.01.0003 bug 512
}

// read config parameters, check terminal status(e.g. if need to download parameters, if it's locked, etc.)
// and reset reversal flag etc
int CheckTermSatus(const ST_EVENT_MSG *pstEventMsg)
{
	FirstRunProc(pstEventMsg);
	LoadEdcLang();

	memcpy(&glSysParamBak, &glSysParam, sizeof(glSysParam));
	LoadSysDownEdcFile();	// load the files downloaded from Protims

	CheckInitTerminal();
	UnLockTerminal();

	RecoverTranLog();	// must called after system initialization
	InitTransInfo();

#ifndef APP_MANAGER_VER
	// load the synchronous parameters file received from main application if main application is existed
	// Not implemented
#endif

	return 0;
}

// 第一次运行时候处理(事实上每次main_sub都会运行)
// process for the first run
static char bInitGui_1 = 0;
static char bInitGui_2 = 0;
void FirstRunProc(const ST_EVENT_MSG *pstEventMsg)
{
	uchar	szEngTime[16+1];
	uchar	ucNeedUpdateParam;

	// Added by Kim_LinHB 2014-7-2
	if(!bInitGui_1)
	{
		unsigned char sTermInfo[60];

		int iDefWidth = XuiStatusbarCanvas()->width/6;
		int iDefHeight = XuiStatusbarCanvas()->height;
		_GUI_INIT_(NULL); //default

		OsLog(LOG_ERROR, "it's a %dx%d device", Gui_GetScrWidth(), Gui_GetScrHeight());

		char szWidthStr[10];
		sprintf(szWidthStr, "%dx", Gui_GetScrWidth());
		int i;
        SetPrefixResPath(glResources[0].path);
        UpdateGlobalFont(glResources[0].fontSize);
		for(i = 1; glResources[i].path[0] != 0; ++i){
		    if(strstr(glResources[i].path, szWidthStr)){
	            SetPrefixResPath(glResources[i].path);
	            UpdateGlobalFont(glResources[i].fontSize);
	            break;
		    }
		}

        Gui_SetBgImage(AddPrefixResPath("background.png"));
        Gui_SetGlobalBtnImg(AddPrefixResPath("button_normal.png"), AddPrefixResPath("button_pressed.png"));

        char szTemp[32];
        if(OsRegGetValue("ro.fac.touchscreen", szTemp) > 0)
        {
            PreLoadAllKeyboard(); //EPT-20
        }


		Gui_SB_BindResource(XuiStatusbarCanvas(), "./res/icons.ini");

		Gui_SB_RegIcon(GUI_SB_ICON_BATTERY, iDefWidth * 5, 0, iDefWidth, iDefHeight, 500);
		Gui_SB_RegIcon(GUI_SB_ICON_UP_DOWN, iDefWidth * 4, 0, iDefWidth, iDefHeight, -1);
		Gui_RegCallback(GUI_CALLBACK_UPDATE_UPDOWN_ICON, Refresh_UpDown);
		Gui_SB_RegIcon(GUI_SB_ICON_CARDINSERTED, iDefWidth * 3, 0, iDefWidth, iDefHeight, 200);
		if (OsRegGetValue("ro.fac.printer", sTermInfo) > 0)
			Gui_SB_RegIcon(GUI_SB_ICON_PRINTER, iDefWidth * 2, 0, iDefWidth, iDefHeight, -1);

		bInitGui_1 = 1;
	}

    // Modified by Kim_LinHB 2014-7-2
    if( ExistSysFiles() )
    {
        if (ValidSysFiles())
        {
            int i;
            uchar *pType  = NULL;
            LoadSysParam();
            LoadSysCtrlAll();

            if(!bInitGui_2)
            {
                //update language
                _GUI_INIT_(glSysParam.stEdcInfo.stLangCfg.szFilePath);

                ChangeLangOrder();
                for(i = 0; i < 2; ++i)
                {
                    pType = (0 == i? &glSysParam.stTxnCommCfg.ucCommType: &glSysParam.stTxnCommCfg.ucCommTypeBak);
                    Reg_CommIcon(*pType, i);
                }
                bInitGui_2 = 1;
            }

            ucNeedUpdateParam = FALSE;
            if (pstEventMsg->MsgType==USER_MSG)
            {
                if (UpdateTermInfo() || InitMultiAppInfo())
                {
                    ucNeedUpdateParam = TRUE;
                }
            }
            if( glSysParam.stTxnCommCfg.pfUpdWaitUI!=DispWaitRspStatus )
            {
                glSysParam.stTxnCommCfg.pfUpdWaitUI = DispWaitRspStatus;
                glSysParam.stTMSCommCfg.pfUpdWaitUI = DispWaitRspStatus;
                ucNeedUpdateParam = TRUE;
            }

            if (ucNeedUpdateParam)
            {
                SaveSysParam();
            }
            return;
        } // if (ValidSysFiles())
        else
        {
            int iRet;
            Gui_ClearScr();
            iRet = Gui_ShowMsgBox(NULL, gl_stTitleAttr, NULL, _T("APP AND DATA\nINCONSIST.\nRESET CONFIG?"), gl_stCenterAttr, GUI_BUTTON_YandN, -1, NULL);
            if (iRet != GUI_OK)
            {
                SysHaltInfo("PLS REPLACE APP");
            }

            Gui_ClearScr();
            iRet = Gui_ShowMsgBox(NULL, gl_stTitleAttr, NULL, _T("WARNING\nDATA WILL BE CLEAR\nCONTINUE ?"), gl_stCenterAttr, GUI_BUTTON_YandN, -1, NULL);
            if (iRet != GUI_OK)
            {
                SysHaltInfo("PLS REPLACE APP");
            }

            RemoveSysFiles();
        }
    } // if( ExistSysFiles() )

    GetEngTime(szEngTime);
    Gui_ClearScr();
    Gui_ShowMsgBox(szEngTime, gl_stTitleAttr, NULL, _T("PLEASE WAIT...\nSYSTEM INIT..."), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

    LoadEdcDefault();	// set EDC default values
    Gui_ClearScr();
    Gui_ShowMsgBox(szEngTime, gl_stTitleAttr, NULL, _T("PLEASE WAIT...\nSYSTEM FILE UPDATE..."), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);
    InitTranLogFile();	// Init transaction log file
    NoDownloadInit();

#ifdef ENABLE_EMV
	LoadEmvDefault();	// Init EMV kernel
#endif
}

// 设置空闲计时。设置一个比较长的倒计时，以用于不止一种的空闲事件处理
// set a idle timer with a long period of time, for processing several idle events
void SetIdleTimer(void)
{
	TimerSet(TIMER_TEMPORARY, TIMERCNT_MAX);
}

// 检查空闲计时，看是否已经流过了指定的分钟数
// check if the timer counted the specific time(uint:minute)
uchar ChkIdleTimer(int iSeconds)
{
	int	iCnt = TIMERCNT_MAX-TimerCheck(TIMER_TEMPORARY);
	PubASSERT(TIMERCNT_MAX > iSeconds*10);	//	ScrPrint(0,7,ASCII,"%d  ", iCnt/10);
	return (iCnt >= iSeconds*10);
}

// 显示空闲时用户界面
// show an idle UI
void ShowIdleMainUI(uchar *pbForceUpdate, uchar bShowGallery, int iGallery_Image_Num)
{
	static	uchar	szLastTime[5+1] = {"00000"};
	uchar	szCurTime[16+1];

	GetEngTime(szCurTime);
	if( *pbForceUpdate || memcmp(szLastTime, &szCurTime[11], 4)!=0 )	// Reset magstripe reader every 10 minutes
	{
		MagClose();
		MagOpen();
		MagReset();
	}

	if(bShowGallery){
	    ShowGallery(iGallery_Image_Num);
	}

	if( *pbForceUpdate || memcmp(szLastTime, &szCurTime[11], 5)!=0)
	{
		// refresh UI
		sprintf((char *)szLastTime, "%.5s", &szCurTime[11]); 

		// Gui_ClearScr(); // removed by Kim_LinHB 2014-08-13 v1.01.0003 bug512
		// Modified by Kim_LinHB 2014-8-11 v1.01.0003
        //Gui_ShowMsgBox(szCurTime, gl_stTitleAttr, NULL, gl_stCenterAttr, GUI_BUTTON_NONE, 0,NULL);
        Gui_UpdateTitle(szCurTime, gl_stTitleAttr);
		//Gui_DrawText(szCurTime, gl_stTitleAttr, 0, 5);
        if(*pbForceUpdate){
            if(!bShowGallery)
            {
                Gui_UpdateKey(XUI_KEYFUNC, _T("FUNC"), NULL, NULL);
                Gui_UpdateKey(XUI_KEYMENU, _T("MENU"), NULL, NULL);
                Gui_SetVirtualButton(1, 1);
                DispSwipeCard(TRUE);
            }
            else
                Gui_SetVirtualButton(0,0);
        }
		*pbForceUpdate = FALSE;
	}
#ifdef _WIN32
	DelayMs(100);
#endif
}

static void ShowGallery(int num)
{
    static int iCurrImages = 0;
    static unsigned long ulTimer_Late = 0;
    static unsigned long ulTimer_Curr = 0;

    if(num <= 0)
        return;

    ulTimer_Curr = OsGetTickCount();
    if(ulTimer_Curr - ulTimer_Late >= 10000)
    {
        uchar szImageKey[8];
        uchar szImageValue[255] = {0};
        sprintf(szImageKey, "PIC_%d", iCurrImages);
        if(0 == GetEnv(szImageKey, szImageValue))
        {
            char szId[10];
            ulTimer_Late = ulTimer_Curr;
            Gui_ClearScr();
            char *p = strstr(szImageValue, "???");
            if(p)
            {
                p+=3;
                strcpy(szId, p);
                p-=3;
                strcpy(p, gl_szTerminalType);
                strcat(szImageValue, szId);
                Gui_DrawImage(szImageValue, 0,0);
            }
        }

        ++iCurrImages;
        if(iCurrImages >= num)
            iCurrImages = 0;
    }

}

// Modified by Kim_LinHB 2014-7-8
int CheckInitTerminal(void)
{
	uchar	szCurTime[16+1], szLastTime[16+1];
	uchar	ucKey;
	uchar	szBuff[50];
	
	if( !(glSysParam.ucTermStatus & INIT_MODE) )
	{
		return 0;
	}
	
	TimerSet(0, 0);
	memset(szCurTime,  0, sizeof(szCurTime));
	memset(szLastTime, 0, sizeof(szLastTime));
	while( glSysParam.ucTermStatus & INIT_MODE )
	{
	    Gui_UpdateKey(XUI_KEYFUNC, _T("FUNC"), NULL, NULL);
	    Gui_SetVirtualButton(1, 0);
		if( TimerCheck(0)==0 )
		{
			TimerSet(0, 10);
			GetEngTime(szCurTime);
			if (strcmp(szCurTime, szLastTime)!=0)
			{
				Gui_ClearScr();
				sprintf(szBuff, "%s\n[%.14s]", _T("PLEASE INIT"), AppInfo.AppName);
				Gui_UpdateTitle(szCurTime, gl_stTitleAttr);
				Gui_DrawText(szBuff, gl_stCenterAttr, 0, 50);
				memcpy(szLastTime, szCurTime, sizeof(szLastTime));
			}
		}

		ucKey = PubWaitKey(10);
		if(
			(ucKey==KEYF1 && ChkTermEx(_TERMINAL_D200_)) || (ucKey==KEYFN && !ChkTermEx(_TERMINAL_D200_))
			)
		{
			InitTransInfo();
			FunctionInit();
			TimerSet(0, 0);
			memset(szLastTime, 0, sizeof(szLastTime));
		}
	}

	return 0;
}

int MainIdleProc(void)
{
	// should not let POS go into sleep mode when running simulator
	int	iRet;

	if (POWER_BATTERY == OsCheckPowerSupply())
	{
		if (glSysParam.stEdcInfo.ucIdleShutdown)
		{
			PowerOff();
		}
		else
		{
			// Modified by Kim_LinHB 2014-7-8
			Gui_ClearScr();
			Gui_ShowMsgBox(_T("POWER SAVING"), gl_stTitleAttr, NULL, _T("PRESS ANY KEY\nTO RECOVER"), gl_stCenterAttr, GUI_BUTTON_NONE, 0, NULL);

			do 
			{
				iRet = SysSleep("00");
			} while((iRet==-3) && (kbhit()!=0));
			// 很奇怪的现象：除非在上次SysSleep返回之后调用DelayMs(3000)，否则即使间隔1分钟，调用SysSleep仍会直接返回-3。
			// 因此我在这里加了判断，如果返回-3而且没有按键则继续重复SysSleep
			// 在外部已经保证了进入MainIdleProc的间隔>=1分钟
			// it needs to delay 3 seconds after return from SysSleep, otherwise SysSleep will return -3 even the period of calling SysSleep is over 1 min,
			// so here is a special processing, if return -3 from SysSleep and no key was pressed then continue calling SysSleep.

			DelayMs(100);
			kbflush();
			Gui_ClearScr(); // Added by Kim_LinHB 2014-08-13 v1.01.0003
		}
		return 0;
	}
	return -1;
}

// Added by Kim 2014-12-19
int Refresh_UpDown(gui_callbacktype_t type, void *data, int *len)
{
	int iRet;
	gui_updownstatus_t status = *(gui_updownstatus_t *)data;
	if (GUI_UP == status)
		iRet = Gui_SB_ManuallyRefresh(GUI_SB_ICON_UP_DOWN, OPENICON_UP);
	else if (GUI_DOWN == status)
		iRet = Gui_SB_ManuallyRefresh(GUI_SB_ICON_UP_DOWN, OPENICON_DOWN);
	else if ((GUI_UP & status) && (GUI_DOWN & status))
		iRet = Gui_SB_ManuallyRefresh(GUI_SB_ICON_UP_DOWN, OPENICON_UP_DOWN);
	else
		iRet = Gui_SB_ManuallyRefresh(GUI_SB_ICON_UP_DOWN, NULL);

	return iRet;
}

// end of file

