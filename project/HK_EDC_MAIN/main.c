
#include "global.h"

/********************** Internal macros declaration ************************/


/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
static int  main_sub(ST_EVENT_MSG *pstEventMsg);
static int  CheckTermSatus(ST_EVENT_MSG *pstEventMsg);
static void FirstRunProc(ST_EVENT_MSG *pstEventMsg);
static void SetIdleTimer(void);
static uchar ChkIdleTimer(uchar ucMinutes);
static void ShowIdleMainUI(uchar *pbForceUpdate, uchar bDispTimeOnly);
static int  CheckInitTerminal(void);
static void MainIdleProc(void);
void CLSSPreTrans(void);
int  ProcKeyEvent(int KeyValue);

//2014-6-13 EPS T8 reversal
#ifdef _S_SERIES_   //SXX EPS
extern uchar IsLeapYear(unsigned long ulYear);
uchar IsEpsT8Expire(void);
void UpdateEpsExpiryTime(void);
void EpsReversal(void);
uchar epsT8Expiry[7];
#define EPS_T8_IN_MINUTE      2
#endif

/*#ifdef REAL_ECR
uchar         glPwd[6+1];//2014-11-17  Gillian 20161010
#endif*/
#ifdef BEA_SSL //HASE GENERIC BUILD106
  #ifdef PP_SSL //HASE GENERIC BUILD106
extern void PP_InitDnsIp(uchar ucDisplay);
  #endif  //end PP_SSL
#endif  //end BEA_SSl

int event_main(ST_EVENT_MSG *pstEventMsg);
/********************** Internal variables declaration *********************/

/********************** external reference declaration *********************/
extern void EpsUpdateReverseState(uchar needReverse);//2014-7-10

extern void EpsAutoSettUpdate(ST_EVENT_MSG* stEvent); //W204: EPS autosettlement state update

int ProcAutoRecap(void);    //W204: EPS auto settlement handler


const APPINFO AppInfo =
{
	APP_NAME,
	EDCAPP_AID,
	EDC_VER_INTERN,
	"PAX TECHNOLOGY",
	"SHARE EDC FOR VISA/MC",
	"",
	0xFF,
	0xFF,
	0x01,
	""
};

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

/*
static int TestFunc(void)
{
	int	ii, iRet, iCnt;
	TRAN_LOG	stTmpLog;

	TimerSet(TIMER_TEMPORARY, 10000);

	iCnt = 0;
	for (ii=0; ii<MAX_TRANLOG; ii++)
	{
		if (LoadTranLog(&stTmpLog, (ushort)ii))
		{
			iCnt++;
		}
	}
	ii = 10000-TimerCheck(TIMER_TEMPORARY);

	ScrRestore(0);
	ScrCls();
	ScrPrint(0,0,ASCII,"TIME=%d.%ds", (int)(ii/10), (int)(ii%10));
	ScrPrint(0,1,ASCII,"FAIL CNT=%d", iCnt);
	kbflush();getkey();
	ScrRestore(1);
}
*/
//***************************************************
//function:
//Check if the T8 is expired
//2014-6-13
//return: 1 - expire, 0 - not expire
//***************************************************
uchar IsEpsT8Expire(void)
{
  uchar current[7];    
  GetTime(current);

  if(memcmp(current, epsT8Expiry, 6) > 0)
    return 1;
  else
    return 0;  
}
//***************************************************
//function:
//BCD to decimal conversion
//
//parameter: BCD value
//
//return: decimal value
//***************************************************
uchar Bcd2Dec(uchar bcdValue)
{
  uchar decValue;

  decValue = (((bcdValue & 0xF0) >> 4) * 10) + (bcdValue & 0x0F);
  
  return decValue;
}


//***************************************************
//function:
//decimal to BCD conversion
//
//parameter: decimal value
//
//return: BCD value
//***************************************************
uchar Dec2Bcd(uchar decValue)
{
  uchar bcdValue;

  bcdValue = ((decValue / 10) << 4) | (decValue %10);

  return bcdValue;
}


enum {EPS_YEAR, EPS_MONTH, EPS_DAY, EPS_HR, EPS_MIN, EPS_SEC};


//**********************************************************************************************************
//function:
//update the T8 timer, resend the reversal after 2 mins
//calculate the expired time epsT8Expiry, keep checking on this value to determine initiating the reversal
//the epsT8Expiry is preserved even it switches to the other apps
//**********************************************************************************************************  
void UpdateEpsExpiryTime(void)
{  
  uchar min, hr, day, month, year;
  uchar carryFlag = 0;

  GetTime(epsT8Expiry);
  
  //convert BCD to Decimal  
  min = Bcd2Dec(epsT8Expiry[EPS_MIN]);        
  hr = Bcd2Dec(epsT8Expiry[EPS_HR]);
  day = Bcd2Dec(epsT8Expiry[EPS_DAY]);
  month = Bcd2Dec(epsT8Expiry[EPS_MONTH]);
  year = Bcd2Dec(epsT8Expiry[EPS_YEAR]);

  
  min += EPS_T8_IN_MINUTE;
  
  if(min > 59)
  {
    min -= 60;    
    hr += 1;
  }
    
  if(hr > 23)
  {
    hr -= 24;
    day += 1;

    if(day > 31)
    {
      day = 1;
      carryFlag = 1;
    }
    else if (day == 31)
    {
        switch(month)
        {
          case 4: 
          case 6:
          case 9:
          case 11:
            day = 1;
            carryFlag = 1;
            break;
          default:
            carryFlag = 0;
            break; 
        }      
    }
    else if (month == 2)
    {
        if(IsLeapYear(2000 + year))
        {
          if(day > 29)
          {
            day = 1;
            carryFlag = 1;
          }
          else
          {
            carryFlag = 0;          
          }
        }
        else
        {
          if(day > 28)
          {
            day = 1;
            carryFlag = 1;
          }
          else
          {
            carryFlag = 0;          
          }          
        }
    }
    else 
    {
      carryFlag = 0;
    }
  }

  if(carryFlag == 1)
  {
    month += 1;

    if(month > 12)
    {
      month = 1;
      year += 1;
    }
  }
    
  epsT8Expiry[EPS_MIN] = Dec2Bcd(min);        
  epsT8Expiry[EPS_HR] = Dec2Bcd(hr);
  epsT8Expiry[EPS_DAY] = Dec2Bcd(day);
  epsT8Expiry[EPS_MONTH] = Dec2Bcd(month);
  epsT8Expiry[EPS_YEAR] = Dec2Bcd(year);  
}
//2014-7-10 EPS update
//build009
//***********************************************************************************************************
//Function:
//SXX EPS - if EPS reversal pending, set timer
//
//Param: needReverse - a reversal needed (1) / not needed (0) in EPS
//***********************************************************************************************************
void EpsUpdateReverseState(uchar needReverse)
{  
  if(needReverse == 1 && glReversalPending == FALSE)
  {                  
    UpdateEpsExpiryTime();
    glReversalPending = TRUE;
	////2014-8-27 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"test 1:  %d",glReversalPending);
	//PubWaitKey(7);
  }
  else if(needReverse == 1 && glReversalPending == TRUE)
  {
    //do nothing, let the T8 timer proceed
	//  //2014-8-27 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"test 2:  %d",glReversalPending);
	//PubWaitKey(7);
  }
  else
  {
    glReversalPending = FALSE;
	////2014-8-27 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"test 3:  %d",glReversalPending);
	//PubWaitKey(7);
  }
}

//W204: EPS auto settlement state update
void EpsAutoSettUpdate(ST_EVENT_MSG* stEvent)
{
  //EPS auto settlement state update
  if (stEvent->MagMsg.track2[0] == 1)
  {
    // get recap time
    EPS_AutoRecap = 1;
    memcpy(EPS_Recap_Time, &(stEvent->MagMsg.track2[1]), 4);
  }
  else
  {
    // reset recap time
    EPS_AutoRecap = 0;
    memset(EPS_Recap_Time, 0x00, sizeof(EPS_Recap_Time));
  }

  if (stEvent->MagMsg.track3[0] != 0)
  {
    // get recap time
    memcpy(EPS_Last_Recap_Date, stEvent->MagMsg.track3, 4);
  }
  else
  {
    // reset recap time
    memset(EPS_Last_Recap_Date, 0x00, sizeof(EPS_Last_Recap_Date));
  }
  //end EPS auto settlement
}


//*********************************************************
//function:
//Carry out EPS reversal
//*********************************************************
void EpsReversal(void)
{
  ST_EVENT_MSG  stEvent;

  memset(&stEvent, 0, sizeof(ST_EVENT_MSG));          	
  stEvent.MsgType = EPSAPP_TIMEREVENT;
  DoEvent(glSysParam.astSubApp[APP_EPS].ucAppNo/*glSubAppEps.AppNum*/, (ST_EVENT_MSG *) &stEvent);
        
  if(stEvent.MagMsg.track1[0] == 1) // reversal fail again, update time and go to another round             
  {                                                                             
    UpdateEpsExpiryTime();          
    glReversalPending = TRUE;                          
  }                  
  else                            
  {                                
    glReversalPending = FALSE;                  
  }                                  

  //W204: EPS autosettlement state update
  EpsAutoSettUpdate(&stEvent);
}


int event_main(ST_EVENT_MSG *pstEventMsg)
{


	glEdcMsgPtr = pstEventMsg;	// 保存事件指针
	
	return main_sub(pstEventMsg);
}

int main(void)
{
	uchar			bUpdateUI;
	int				iRet;
	int iCnt;//2014-11-4
	  uchar   ecr_port_opened;
	ST_EVENT_MSG	stEventMsg;

  uchar dateTime[20], dateTimeBak[20];    //W204: EPS auto settlement handler

#ifdef _S_SERIES_             //raymond 27 Jun 2011: SXX EPS implementation
	uchar closeLineFlag;
	
	closeLineFlag = 0;              
	glReversalPending = 0;          //0 - no reversal pending, 1 - reversal pending

#endif
	//Gillian 2016-8-19
#ifdef PP_SSL
	LoadConnectOnePara(TRUE);                   //build011: load current communication setting

#endif  //end PP_SSL 
	#ifdef BEA_SSL
	if(!ChkIfAmex() && !ChkIfDiners())
	{
		if(glSysParam.stTxnCommCfg.ucEnableSSL_URL)  //Gillian 2016-8-29
		{
		  InitSSL();
	   //   InitDnsIp(FALSE);

			#ifdef PP_SSL   //HASE GENERIC BUILD106
		  PP_InitDnsIp(FALSE);
			#endif  //end PP_SSL 
		}
	}
#endif

#ifdef AMT_PROC_DEBUG//2014-11-4
		memset(&glEcrAMT,0,sizeof(ECR_AMT_LOG));//if turn off handset, ecr received amt will be erased
		glEcrAmtCnt = 0;
#endif
	memset(glAmount, 0, sizeof(glAmount));		
	bFirstPollFlag =1;
#ifdef PAYPASS_DEMO
	gucIsPayWavePass = PAYWAVE;
#endif

	ecr_port_opened     = FALSE;
	memset(&stEventMsg, 0, sizeof(stEventMsg));
	stEventMsg.MsgType = USER_MSG;
	iRet = event_main(&stEventMsg);
	if (iRet==ERR_EXIT_APP)
	{
		return 0;
	}

	if (FindAcqIdxByName("CITI_TAXI", FALSE) <MAX_ACQ)
	{
		if (glSysCtrl.ucLoginStatus != LOGIN_MODE)
		{
			iRet = SelectAcqMid("", &glSysCtrl.ucCurrAcqIdx);
			if (iRet!=0)
			{
				glSysCtrl.ucCurrAcqIdx=0xff;
				return 0;
			}
			glSysCtrl.ucLoginStatus = LOGIN_MODE;
			SaveSysCtrlBase();
		}
	}

//	TestFunc();
	bUpdateUI = TRUE;


  //W204:  EPS auto settlement handler
	memset(dateTime, 0x00, sizeof(dateTime));
	memset(dateTimeBak, 0x00, sizeof(dateTimeBak));
	GetTime(dateTime);
  //end W204

	while(1)
	{
    //W204 EPS auto settlement handler
 		GetTime(dateTime);

		if (memcmp(dateTimeBak, dateTime, 6)!= 0)
		{
			ProcAutoRecap();
		}

		memcpy(dateTimeBak, dateTime, 7);
    //end W204


		// Setup idle timer 设置空闲计时
		if (bUpdateUI)
		{
			SetIdleTimer();
			if (glSysCtrl.ucLoginStatus != LOGIN_MODE)
			{
				if (FindAcqIdxByName("CITI_TAXI", FALSE) <MAX_ACQ)
				{
					iRet = SelectAcqMid("", &glSysCtrl.ucCurrAcqIdx);
					if (iRet!=0)
					{
						glSysCtrl.ucCurrAcqIdx=0xff;
						return 0;
					}
					glSysCtrl.ucLoginStatus = LOGIN_MODE;
					SaveSysCtrlBase();
				}
			}
		}
	//	ShowIdleMainUI(&bUpdateUI, 1);
#ifdef APP_MANAGER_VER//if app manager
			ShowIdleMainUI(&bUpdateUI, 1);
		// ECR processing
        if (glSysParam.stECR.ucMode)
        {
            if ((!ecr_port_opened) && !ChkOnBase())
            {          
				EcrOpen();  // Open port for ECR
                ecr_port_opened = TRUE;
            }
            if (ecr_port_opened && ChkOnBase())
            {
                ecr_port_opened = FALSE;
            }
			//event
			if (ProcEcrEvent()==0)
			{
			
				bUpdateUI = TRUE;
				continue;
			}
			if( MagSwiped()==0 )
			{
				memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));
				stEventMsg.MsgType = MAGCARD_MSG;
				stEventMsg.MagMsg.RetCode = MagRead(stEventMsg.MagMsg.track1,
					stEventMsg.MagMsg.track2,
					stEventMsg.MagMsg.track3);
				iRet = event_main(&stEventMsg);
				//if (iRet==ERR_EXIT_APP)
				{
					return 0;
				}
				bUpdateUI = TRUE;
				CommOnHook(FALSE);
				continue;
			}
			if( ChkIfEmvEnable() && IccDetect(ICC_USER)==0 )
			{
				memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));
				stEventMsg.MsgType = ICCARD_MSG;
				iRet = event_main(&stEventMsg);
				if (iRet==ERR_EXIT_APP)
				{
					return 0;
				}
				bUpdateUI = TRUE;
				CommOnHook(FALSE);
				continue;
			}
			if (glSysParam.stEdcInfo.ucClssFlag == 1 && !ChkOnBase() &&
				((glSysParam.stEdcInfo.ucClssMode == 0)||(glSysParam.stEdcInfo.ucClssMode == 2))
				&&PubKeyPressed())
			{
			//	kbflush();
				InitTransInfo();
				memset(glAmount, 0, sizeof(glAmount));
				iRet = GetAmount();
				if (iRet == 0)
				{					
					CLSSPreTrans();
#ifdef PAYPASS_DEMO
					gucIsPayWavePass = PAYWAVE;
#endif
					bUpdateUI = TRUE;
					continue;
				}	
				
				else if (iRet == KEYFN || iRet == KEYATM3 || iRet == KEYATM4||iRet==KEYMENU 
					|| iRet == KEYATM2|| iRet == KEYATM1||iRet == KEYDOWN||iRet == KEYUP)
				{
					ProcKeyEvent(iRet);
					bUpdateUI = TRUE;
					CommOnHook(FALSE);
					continue;
				}
			}
			 if( PubKeyPressed()&&!glSysParam.stEdcInfo.ucClssFlag )//CTLS keyin conflict with normal KEYIN
			{

				ProcKeyEvent(NULL);
				bUpdateUI = TRUE;
				CommOnHook(FALSE);
				continue;
			}	
        }
		else
		{
			if (glSysParam.stEdcInfo.ucClssFlag == 1 && !ChkOnBase())
			{
				ShowIdleMainUI(&bUpdateUI, glSysParam.stEdcInfo.ucClssMode); // 会改变bUpdateUI值
			}
			else
			{
				ShowIdleMainUI(&bUpdateUI, 1);
			}
			if (glSysParam.stEdcInfo.ucClssFlag == 1 && !ChkOnBase() &&
				((glSysParam.stEdcInfo.ucClssMode == 0)||(glSysParam.stEdcInfo.ucClssMode == 2))
				&&!glSysParam.stECR.ucMode)
			{
				kbflush();
				InitTransInfo();
				memset(glAmount, 0, sizeof(glAmount));
#ifdef AMT_PROC_DEBUG
				//2014-9-18 each time come to glAmount or szAmount
				sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(17)%.13s",glAmount);
				sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(17)%.13s",glProcInfo.stTranLog.szAmount);
				glProcInfo.stTranLog.cntAmt++;
	//			//2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 18: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
				iRet = GetAmount();
				if (iRet == 0)
				{
					CLSSPreTrans();
#ifdef PAYPASS_DEMO
					gucIsPayWavePass = PAYWAVE;
#endif
					continue;
				}	

				else if (iRet == KEYFN || iRet == KEYATM3 || iRet == KEYATM4||iRet==KEYMENU 
					|| iRet == KEYATM2|| iRet == KEYATM1||iRet == KEYDOWN||iRet == KEYUP)
				{
					ProcKeyEvent(iRet);
					bUpdateUI = TRUE;
					CommOnHook(FALSE);
 					continue;
				}
			}
			else
			{
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
						return 0;
					}
					bUpdateUI = TRUE;
					CommOnHook(FALSE);
					continue;
				}
				if( ChkIfEmvEnable() && IccDetect(ICC_USER)==0 )
				{
					memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));
					stEventMsg.MsgType = ICCARD_MSG;
					iRet = event_main(&stEventMsg);
					if (iRet==ERR_EXIT_APP)
					{
						return 0;
					}
					bUpdateUI = TRUE;
					CommOnHook(FALSE);
					continue;
				}
				if( PubKeyPressed() )
				{

					ProcKeyEvent(NULL);
					bUpdateUI = TRUE;
					CommOnHook(FALSE);
 					continue;
				}
			}
		}
#else//if app not manager 
		if (glSysParam.stEdcInfo.ucClssFlag == 1 && !ChkOnBase())
		{
			ShowIdleMainUI(&bUpdateUI, glSysParam.stEdcInfo.ucClssMode); // 会改变bUpdateUI值
		}
		else
		{
			ShowIdleMainUI(&bUpdateUI, 1);
		}
		if (glSysParam.stEdcInfo.ucClssFlag == 1 && !ChkOnBase() &&
			((glSysParam.stEdcInfo.ucClssMode == 0)||(glSysParam.stEdcInfo.ucClssMode == 2)))
		{
			kbflush();
			InitTransInfo();
			memset(glAmount, 0, sizeof(glAmount));
			
			iRet = GetAmount();
			if (iRet == 0)
			{
				CLSSPreTrans();
#ifdef PAYPASS_DEMO
				gucIsPayWavePass = PAYWAVE;
#endif
				continue;
			}	
			else if (iRet == KEYFN || iRet == KEYUP || iRet == KEYDOWN||iRet==KEYMENU 
#ifdef _P60_S1_
			|| iRet ==KEYF4
#endif	
#ifdef PAYPASS_DEMO
			|| iRet == KEYATM4
#endif			
			)
			{
				memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));
				stEventMsg.MsgType  = KEYBOARD_MSG;
				stEventMsg.KeyValue = iRet;
				iRet = event_main(&stEventMsg);
				if (iRet==ERR_EXIT_APP)
				{
					return 0;
				}
				bUpdateUI = TRUE;
				CommOnHook(FALSE);
				continue;
			}
		}
		else
		{
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
					return 0;
				}
				bUpdateUI = TRUE;
				CommOnHook(FALSE);
				continue;
			}
			if( ChkIfEmvEnable() && IccDetect(ICC_USER)==0 )
			{
				memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));
				stEventMsg.MsgType = ICCARD_MSG;
				iRet = event_main(&stEventMsg);
				if (iRet==ERR_EXIT_APP)
				{
					return 0;
				}
				bUpdateUI = TRUE;
				CommOnHook(FALSE);
				continue;
			}
			if( PubKeyPressed() )
			{
				memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));
				stEventMsg.MsgType  = KEYBOARD_MSG;
				stEventMsg.KeyValue = getkey();
				iRet = event_main(&stEventMsg);
				if (iRet==ERR_EXIT_APP)
				{
					return 0;
				}
				bUpdateUI = TRUE;
				CommOnHook(FALSE);
				continue;
			}
		}
#endif//end if manager
		
		// add more event-handlers here here
		// 仿照上面，在这里添加更多事件判断和处理函数
		
		// One of idle-processing 空闲事件处理
		if (ChkIdleTimer(glSysParam.stEdcInfo.ucIdleMinute))
		{
			MainIdleProc();
			bUpdateUI = TRUE;
			continue;
		}

		//2014-6-13 EPS reversal handle
#ifdef _S_SERIES_  //raymond: SXX EPS - if EPS reversal pending, check T8
		if(glReversalPending == 1)    
		{
			if(IsEpsT8Expire())
			{
				if (ChkIfAppEnabled(APP_EPS)/*glSubAppHave & HAVE_EPS*/)
				{
					EpsReversal();
				}                        
			}
		}
#endif // _S_SERIES_
	}	// end of while( 1
}

// EDC  事件处理函数
int main_sub(ST_EVENT_MSG *pstEventMsg)
{
	int		iRet;

	

	SystemInit();
// 	glSysParam.ucKbSound = TRUE;
// 	kbmute(TRUE);//enable key sound

#ifdef ENABLE_EMV
	iRet = EMVCoreInit();
	//EMVSetDebug(1);
	if( iRet==EMV_KEY_EXP )
	{
		//EraseExpireCAPK();//2015-12-7 mark!
	}
#endif

#if defined(_P60_S1_) || defined(_S_SERIES_)
	SetOffBase(NULL);
#endif

	CheckTermSatus(pstEventMsg);
	if (glSysParam.stEdcInfo.ucClssFlag != 1 )
	{
		memset(glAmount, 0, sizeof(glAmount));
	}
#ifndef APP_MANAGER_VER
	// Process manager attached administrative message.
	// Not implemented
#endif

#ifdef APP_MANAGER_VER
#ifndef WIN32
	if (pstEventMsg->MsgType==USER_MSG)
    {
        ManagerInit();
		ManagerServStartUp();
    }
#endif

#endif

	iRet = ERR_NO_DISP;

	if(glSysParam.stEdcInfo.ucINPUT_ONLY==1)//key in only
	{	
	
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
		case EPSAPP_FUNCKEYMSG:
			ProcFuncKeyMsg();
			break;
			
		case KEYBOARD_MSG:
			iRet = ProcKeyMsg();
			break;
			
		case APPMSG_SYNC:
			break;
			
		case APPMSG_CLSS:
			//2014-5-14 enhancement for ECR & Modem connection
#ifdef _S60_  
			if ( glSysParam.stEdcInfo.ucClssFlag==1 )
			{
				ClssOpen(); //2014-5-16 enhance
			}
#endif
			ProcClssMsg(glAmount);
			break;
		case AMEX_RETURN:
			break;
		default:
			break;
		}
	}
	else
	{
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
			//2014-5-14 enhancement for ECR & Modem connection
#ifdef _S60_  
			ClssClose(); //2014-5-16 enhance
#endif 
			iRet = ProcMagMsg();
			break;
			
		case ICCARD_MSG:
			//2014-5-14 enhancement for ECR & Modem connection
#ifdef _S60_  
			ClssClose(); //2014-5-16 enhance
#endif 
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
			
		case APPMSG_CLSS:
			//2014-5-14 enhancement for ECR & Modem connection
#ifdef _S60_  
			if ( glSysParam.stEdcInfo.ucClssFlag==1 )
			{
				ClssOpen(); //2014-5-16 enhance
			}
#endif
			ProcClssMsg(glAmount);
			break;
			
		default:
			break;
		}
	}
	if( iRet!=0 )
	{ 
		if (iRet!=ERR_NEED_INSERT)//chip card swiped need insert 
		{
#ifdef AMT_PROC_DEBUG//2014-9-22
	glProcInfo.stTranLog.cntAmt = 0;
#endif
			EcrSendTranFail();
			DispResult(iRet);
		}
		else
		{
				return iRet;
		}
	
	}

#if defined(_P60_S1_) || defined(_S_SERIES_)
	SetOffBase(NULL);
#endif

  CommOnHook(FALSE);  //build88R: Make sure all connection are closed before get back to EDC manager
  
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
	memset(glAmount, 0, sizeof(glAmount));//2015-5-26 reset amount after one transaction
	return 0;
}

// 读取配置信息、检查终端参数/状态(如,是否下载b参数、是否锁定等)、
// 恢复冲正标志等
int CheckTermSatus(ST_EVENT_MSG *pstEventMsg)
{
	LoadWLBBit63Flag();//FOR TEST

	FirstRunProc(pstEventMsg);
	LoadEdcLang();

	memcpy(&glSysParamBak, &glSysParam, sizeof(glSysParam));
	LoadSysDownEdcFile();	// 加载PROTIMS下载的参数文件

	CheckInitTerminal();
	UnLockTerminal();

	RecoverTranLog();	// must called after system initialization


	InitTransInfo();


#ifdef SUPPORT_TABBATCH
    //if (ChkIfAllAcq_HaveMSSCB())
    {
        AutoRemoveExpiredAuth();
    }
#endif

#ifndef APP_MANAGER_VER
	// 加载管理器传给子应用的参数同步文件(当应用管理器存在)
	// Not implemented
#endif

	return 0;
}

// 第一次运行时候处理(事实上每次main_sub都会运行)
void FirstRunProc(ST_EVENT_MSG *pstEventMsg)
{
	uchar	szEngTime[16+1];
	uchar	ucNeedUpdateParam;

	if( ExistSysFiles() )
	{
		if (ValidSysFiles())
		{
			LoadSysParam();
			LoadSysCtrlAll();

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
		}
		else
		{
			ScrCls();
			PubDispString(_T("APP AND DATA    "), 0|DISP_LINE_LEFT);
			PubDispString(_T("INCONSIST.      "), 2|DISP_LINE_LEFT);
			PubDispString(_T("RESET CONFIG ?  "), 4|DISP_LINE_LEFT);
			if (!AskYesNo())
			{
				SysHaltInfo("PLS REPLACE APP");
			}

			ScrCls();
			PubDispString(_T("WARNING         "), DISP_LINE_REVER|DISP_LINE_LEFT);
			PubDispString(_T("DATA WILL CLEAR "), 2|DISP_LINE_LEFT);
			PubDispString(_T("CONTINUE ?      "), 4|DISP_LINE_LEFT);
			if (!AskYesNo())
			{
				SysHaltInfo(_T("PLS REPLACE APP"));
			}
			RemoveSysFiles();
		}
	}

	GetEngTime(szEngTime);
	PubShowTitle(TRUE, szEngTime);
	DispWait();
	PubDispString(_T("SYSTEM INIT..."), 6|DISP_LINE_LEFT);


	LoadEdcDefault();	// set EDC default values
	InitTranLogFile();	// Init transaction log file

#ifdef SUPPORT_TABBATCH
    MS_InitTabBatchLogFile();
#endif

#ifdef ENABLE_EMV
	LoadEmvDefault();	// Init EMV kernel
#endif
}

// 设置空闲计时。设置一个比较长的倒计时，以用于不止一种的空闲事件处理
void SetIdleTimer(void)
{
	TimerSet(TIMER_TEMPORARY, TIMERCNT_MAX);
}

// 检查空闲计时，看是否已经流过了指定的分钟数
uchar ChkIdleTimer(uchar ucMinutes)
{
	int	iCnt = TIMERCNT_MAX-TimerCheck(TIMER_TEMPORARY);

    if(0 == ucMinutes) //add by richard 20161222, if ucMinutes=0 no power saving mode
        return 0;
	
	PubASSERT(TIMERCNT_MAX > ucMinutes*60*10);	//	ScrPrint(0,7,ASCII,"%d  ", iCnt/10);
	return (iCnt >= ucMinutes*60*10);
}

// 显示空闲时用户界面
void ShowIdleMainUI(uchar *pbForceUpdate, uchar bDispTimeOnly)
{
	static	uchar	szLastTime[5+1] = {"00000"};
	uchar	szCurTime[16+1];
	
	GetEngTime(szCurTime);
	//2014-5-13 tt enhance START 
	if( *pbForceUpdate )
	{
		//2014-5-14 enhancement for ECR & Modem connection
#ifdef _S60_  
		ClssClose(); //2014-5-16 enhance
#endif 
		EcrOpen();
	}
	//2014-5-13 tt enhance END

	if( *pbForceUpdate || memcmp(szLastTime, &szCurTime[11], 4)!=0 )	// 每10分钟对磁头复位一次
	{
		MagClose();
		MagOpen();
		MagReset();
	}
	if( *pbForceUpdate || memcmp(szLastTime, &szCurTime[11], 5)!=0 )
	{
		// 刷新UI
		sprintf((char *)szLastTime, "%.5s", &szCurTime[11]);
		PubShowTitle(TRUE, szCurTime);
		if (bDispTimeOnly == 1)
		{
			if(glSysParam.stEdcInfo.ucINPUT_ONLY==1)//key in only
			{	
				DispKeyInOnly();
			}
			else
			{
	#ifdef APP_MANAGER_VER
				if (glSysParam.stECR.ucMode&&glSysParam.stEdcInfo.ucClssFlag == 1)//MANAGER ECR+CLSS
				{
					PubDispString(_T("PLS ENTER AMOUNT"), 2|DISP_LINE_CENTER);
					PubDispString(_T("/SWIPE/INSERT.."), 4|DISP_LINE_CENTER);
				}
				else
	#endif
				{
					DispSwipeCard(FALSE);
				}
			}
			if (ChkIfTrainMode())
			{
				PubDispString(_T("* DEMO ONLY *"), 2|DISP_LINE_CENTER);
			}
#ifdef POS_DEBUG
			PubDispString(_T("* DEBUG *    "), 2|DISP_LINE_CENTER);
#endif
		}
	
	
		*pbForceUpdate = FALSE;
	}
#ifdef APP_MANAGER_VER
	if (glSysParam.stECR.ucMode)
	{
		ScrPrint(96, 7, ASCII, "ECR");
    }
#endif

#ifdef _WIN32
	DelayMs(100);
#endif
}

int CheckInitTerminal(void)
{
	uchar	szCurTime[16+1], szLastTime[16+1];
	uchar	ucKey;
	
	if( !(glSysParam.ucTermStatus & INIT_MODE) )
    {
        return 0;
	}
	
	TimerSet(0, 0);
	memset(szCurTime,  0, sizeof(szCurTime));
	memset(szLastTime, 0, sizeof(szLastTime));
	while( glSysParam.ucTermStatus & INIT_MODE )
	{
		if( TimerCheck(0)==0 )
		{
			TimerSet(0, 10);
			GetEngTime(szCurTime);
			if (strcmp(szCurTime, szLastTime)!=0)
			{
				PubShowTitle(TRUE, szCurTime);
				PubDispString(_T("PLEASE INIT"), 4|DISP_LINE_LEFT);
				ScrPrint(0, 6, CFONT, "[%.14s]", AppInfo.AppName);
				memcpy(szLastTime, szCurTime, sizeof(szLastTime));
			}
		}

		ucKey = PubWaitKey(0);
		if( ucKey==KEYFN )
		{
			InitTransInfo();
			FunctionInit();
			TimerSet(0, 0);
			memset(szLastTime, 0, sizeof(szLastTime));
		}
	}

	return 0;
}

void MainIdleProc(void)
{
#if defined(_S_SERIES_) && !defined(_WIN32)
	// should not let POS go into sleep mode when running simulator
	int		iRet;

	if (ChkTerm(_TERMINAL_S90_))
	{
		if (glSysParam.stEdcInfo.ucIdleShutdown)
		{
			PowerOff();
		}
		else
		{
			ScrCls();
			PubDispString(_T("POWER SAVING"), DISP_LINE_LEFT|DISP_LINE_REVER);
			PubDispString(_T("PRESS ANY KEY"), 2|DISP_LINE_LEFT);
			PubDispString(_T("TO RECOVER"),    4|DISP_LINE_LEFT);
			do 
			{
				iRet = SysSleep("00");
			} while((iRet==-3) && (kbhit()!=0));
			// 很奇怪的现象：除非在上次SysSleep返回之后调用DelayMs(3000)，否则即使间隔1分钟，调用SysSleep仍会直接返回-3。
			// 因此我在这里加了判断，如果返回-3而且没有按键则继续重复SysSleep
			// 在外部已经保证了进入MainIdleProc的间隔>=1分钟

			DelayMs(100);
			kbflush();
		}
	}
#endif
}

void CLSSPreTrans(void)
{
	uchar			bUpdateUI;
	ST_EVENT_MSG	stEventMsg;
	uchar			ucBuff[6+1];
	uchar			szMaxAmt[12+1];
	memset(glAmount, 0, sizeof(glAmount));
	memcpy(glAmount, glProcInfo.stTranLog.szAmount, 12);
#ifdef AMT_PROC_DEBUG
	//2014-9-18 each time come to glAmount or szAmount
	sprintf((char *)glProcInfo.stTranLog.glAmt[glProcInfo.stTranLog.cntAmt],"(21)%.13s",glAmount);
	sprintf((char *)glProcInfo.stTranLog.szAmt[glProcInfo.stTranLog.cntAmt],"(21)%.13s",glProcInfo.stTranLog.szAmount);
	glProcInfo.stTranLog.cntAmt++;
	////2014-9-19 ttt
	//ScrCls();
	//ScrPrint(0,0,0,"tt 22: %d",glProcInfo.stTranLog.cntAmt);
	//PubWaitKey(7);
#endif
	//PubLong2Bcd(glSysParam.stEdcInfo.ulClssMaxLmt,6,ucBuff);
	PubLong2Bcd(glSysParam.stEdcInfo.ulODCVClssFLmt,6,ucBuff);//2015-10-14
	PubBcd2Asc0(ucBuff,6,szMaxAmt);
	szMaxAmt[13]=0;
 
	glProcInfo.ucEcrRFFlg =  1;
	//memcpy(&stEventMsgBak, glEdcMsgPtr, sizeof(ST_EVENT_MSG));
	if (memcmp(glProcInfo.stTranLog.szAmount, szMaxAmt, 12) <= 0)//2015-10-14
	{	
		memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));
		stEventMsg.MsgType = APPMSG_CLSS;
		stEventMsg.UserMsg = glEdcMsgPtr->UserMsg;

		//S60+ECR+R50+comm port not support at same time, the comm port should switch with R50 port 
#ifdef _S60_ 
		//2014-5-28 this may effects clss poll error, remove temporary
		//if ((glSysParam.stEdcInfo.ucClssFlag!=1) || (!ChkEdcOption(EDC_ECR_ENABLE)))
		//	PreDial();
#endif		
		event_main(&stEventMsg);
		bUpdateUI = TRUE;
		CommOnHook(FALSE);
		return;
	}
	bUpdateUI = TRUE;
	if (bUpdateUI)
	{
		SetIdleTimer();
	}
	ShowIdleMainUI(&bUpdateUI, 1);	// 会改变bUpdateUI值
	while( 1 )
	{	
		if( MagSwiped()==0 )
		{
			memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));
			stEventMsg.MsgType = MAGCARD_MSG;
			stEventMsg.MagMsg.RetCode = MagRead(stEventMsg.MagMsg.track1,
				stEventMsg.MagMsg.track2,
				stEventMsg.MagMsg.track3);
			stEventMsg.UserMsg = glEdcMsgPtr->UserMsg;
				
			event_main(&stEventMsg);
			bUpdateUI = TRUE;
			CommOnHook(FALSE);
			return;
		}
		if( ChkIfEmvEnable() && IccDetect(ICC_USER)==0 )
		{
			memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));
			stEventMsg.MsgType = ICCARD_MSG;
		
			stEventMsg.UserMsg = glEdcMsgPtr->UserMsg;
			event_main(&stEventMsg);
			bUpdateUI = TRUE;
			CommOnHook(FALSE);
			return;
		}
		if( PubKeyPressed() )
		{
			memset(&stEventMsg, 0, sizeof(ST_EVENT_MSG));
			stEventMsg.MsgType  = KEYBOARD_MSG;
			stEventMsg.KeyValue = getkey();
			stEventMsg.UserMsg = glEdcMsgPtr->UserMsg;
			event_main(&stEventMsg);
			bUpdateUI = TRUE;
			CommOnHook(FALSE);
			return;
		}
		// add more event-handlers here here
		// 仿照上面，在这里添加更多事件判断和处理函数
		
		// One of idle-processing 空闲事件处理
		if (ChkIdleTimer(glSysParam.stEdcInfo.ucIdleMinute))
		{
			MainIdleProc();
			bUpdateUI = TRUE;
			continue;
		}

#ifdef _S_SERIES_  //raymond: SXX EPS - if EPS reversal pending, check T8
		//2014-6-13 
		if(glReversalPending == TRUE)    
		{
		  if(IsEpsT8Expire())
		  {
			  if (ChkIfAppEnabled(APP_EPS)/*glSubAppHave & HAVE_EPS*/)
			  {
				  EpsReversal();
			  }                        
		  }
		}
#endif // _S_SERIES_
	}	// end of while( 1
}
#ifdef APP_MANAGER_VER

int  ProcKeyEvent(int KeyValue)
{
	uchar	ucAppNo;
	int iRet;
    ST_EVENT_MSG    stEvent;
	uchar sDataOut[100];
	int nDataOutLen;
	
    InitEventMsg(&stEvent, KEYBOARD_MSG);   // This is not the final type
	if (KeyValue == NULL)
	{
		memset(glClssInfo.glAmount,0,13);
		stEvent.UserMsg=&glClssInfo;
		 stEvent.KeyValue = getkey();
	}
	else
	{ 
		 stEvent.KeyValue = KeyValue;
	}

		
	
	// Cancel, power off ?
	if (stEvent.KeyValue==KEYCANCEL)
	{
		if (ChkOnBase())
		{
			ScrCls();
			PubShowMsg(3, "POWER OFF? Y/N");
			if(PubYesNo(-1) == 0)
			{
				PowerOff();
			}
		}

	}
	switch(stEvent.KeyValue)
	{
	case KEYF1:
#ifdef _S_SERIES_
	case KEYATM1:
#endif
		if (ChkIfAppEnabled(APP_EPS))
		{
			
#ifdef _S_SERIES_
			stEvent.MsgType = KEYBOARD_MSG;
			stEvent.KeyValue = KEYATM1;
#else
			stEvent.MsgType = EPSAPP_DEBITTRANS;
			stEvent.KeyValue = 0;
#endif		
			AppDoEvent(glSysParam.astSubApp[APP_EPS].ucAppNo, &stEvent);
			EpsUpdateReverseState(stEvent.MagMsg.track1[0]);//2014-7-10 update EPS state

    	    EpsAutoSettUpdate(&stEvent); //W204: EPS autosettlement state update
		}
		else
		{
			stEvent.MsgType = KEYBOARD_MSG;
			 event_main(&stEvent);
		//	AppDoEvent(glSysParam.astSubApp[APP_EDC].ucAppNo, &stEvent);
		}
		break;
	case KEYF2:
#ifdef _S_SERIES_
	case KEYATM2:
#endif
			if (SelectAppNum(TRUE, TRUE, &ucAppNo)==0)
			{	
				//2014-6-13 EPS T8 start
				if (ChkIfAppEnabled(APP_EPS) && ucAppNo==glSysParam.astSubApp[APP_EPS].ucAppNo)//2014-8-27 bug fix: EDC app also jump into EPS
				{

//#ifdef _S_SERIES_
//					stEvent.MsgType = KEYBOARD_MSG;
//					stEvent.KeyValue = 0;//KEYATM2;
//#else
					stEvent.MsgType = EPSAPP_NONEPS_DEBIT;//EPSAPP_DEBITTRANS;
					stEvent.KeyValue = 0;
//#endif		

					AppDoEvent(glSysParam.astSubApp[APP_EPS].ucAppNo, &stEvent);
					EpsUpdateReverseState(stEvent.MagMsg.track1[0]);//2014-7-10 update EPS state

                    EpsAutoSettUpdate(&stEvent); //W204: EPS autosettlement state update
				}
				//2014-6-13 EPS T8 end

				////Build 1.00.0029
				//if (ChkIfAppEnabled(APP_EPS)
				//	&& (ucAppNo==glSysParam.astSubApp[APP_EPS].ucAppNo))
				//{
				//	//stEvent.MsgType = EPSAPP_FUNCKEYMSG;
				//	//stEvent.KeyValue = 72;//EPS CUP
				//	//AppDoEvent(ucAppNo, &stEvent);
				//	//2014-5-19 EPS CUP app cannot jump in issue
				//	stEvent.MsgType =  EPSAPP_NONEPS_DEBIT;
				//	DoEvent(ucAppNo, (ST_EVENT_MSG *)&stEvent);
				//}
				else if (ChkIfAppEnabled(APP_AMEX) && (ucAppNo==glSysParam.astSubApp[APP_AMEX].ucAppNo)) 
				{
					 	RunApp(ucAppNo);
				//	AppDoEvent(ucAppNo, &stEvent);
				}
				else if (ChkIfAppEnabled(APP_VA) && (ucAppNo==glSysParam.astSubApp[APP_VA].ucAppNo))
				{
						RunApp(ucAppNo);
					//AppDoEvent(ucAppNo, &stEvent);
					break;
				}
				else
				{
					stEvent.MsgType = KEYBOARD_MSG;
					 event_main(&stEvent);
				}
			}
			break;

	case KEYF3:
#ifdef _S_SERIES_
	case KEYATM3:
#endif
			if (SelectAppNum(TRUE, TRUE, &ucAppNo)==0)
			{
				
	
				//Build 1.00.0029
				if (ChkIfAppEnabled(APP_EPS)
					&& (ucAppNo==glSysParam.astSubApp[APP_EPS].ucAppNo))
				{
#ifdef _S_SERIES_
					stEvent.MsgType = KEYBOARD_MSG;
					stEvent.KeyValue = KEYATM3;
#else
					stEvent.MsgType = EPSAPP_FUNCKEYMSG;
					stEvent.KeyValue = 0;
#endif				
					AppDoEvent(ucAppNo, &stEvent);
					//2014-7-17 although this point may not be reached, still need to do EPS T8 handling
					EpsUpdateReverseState(stEvent.MagMsg.track1[0]);//2014-7-10 update EPS state

         			EpsAutoSettUpdate(&stEvent); //W204: EPS autosettlement state update
				}
				else if (ChkIfAppEnabled(APP_AMEX) && (ucAppNo==glSysParam.astSubApp[APP_AMEX].ucAppNo)) 
				{
					AppDoEvent(ucAppNo, &stEvent);
				}
				else if (ChkIfAppEnabled(APP_VA) && (ucAppNo==glSysParam.astSubApp[APP_VA].ucAppNo))
				{
					AppDoEvent(ucAppNo, &stEvent);
					break;
				}
				else
				{
					stEvent.MsgType = KEYBOARD_MSG;
					AppDoEvent(ucAppNo, &stEvent);
				}
			}
// 			else
// 			{
// 				AppDoEvent(glSysParam.astSubApp[APP_EDC].ucAppNo, &stEvent);
// 			}
			break;
	case KEYF4:
#ifdef _S_SERIES_
	case KEYATM4:
#endif
        if (ChkIfAppEnabled(/*APP_OTHER_START*/APP_CUP))
        {
			if (glSysParam.stEdcInfo.ucClssFlag == 1)//send cmd to change R50 application to CUP
			{
				//2014-5-13 enhance tt
				ClssOpen(); //2014-5-16 enhance
				WaveProcOtherProtocol("\xE5",1,sDataOut,&nDataOutLen); 
				//2014-5-13 enhance tt
				ClssClose(); //2014-5-16 enhance
			}
           	RunApp(glSysParam.astSubApp[/*APP_OTHER_START*/APP_CUP].ucAppNo);//call CUP application 
			//2014-6-12 for new oversea CUP app occupies the communication ports, just close one to fix dial fail (F0) issue
			PortClose(3);
			//ScrCls();
		 //   ScrPrint(0,0,0,"tt 2:%d",PortClose(glSysParam.stEdcInfo.ucClssComm));
			//PubWaitKey(7);

			if (glSysParam.stEdcInfo.ucClssFlag == 1)
			{
				ClssOpen(); //2014-5-16 enhance
				ClssProcPoll(FALSE);//finish the CLSS transaction need to check R50 status
			}
        }
		else
		{
			 event_main(&stEvent);
		//	AppDoEvent(glSysParam.astSubApp[APP_EDC].ucAppNo, &stEvent);
		}
		break;
	case KEY1:
	case KEY2:
	case KEY3:
	case KEY4:
	case KEY5:
	case KEY6:
	case KEY7:
	case KEY8:
	case KEY9:
	case KEY0:
		if (!glSysParam.stECR.ucMode||glSysParam.stEdcInfo.ucClssFlag == 1)
		{	//CLSS open key in OK
			//ecr close key in OK
			event_main(&stEvent);
		}
		else if (glSysParam.stECR.ucMode!=2)//not ecr only key in OK
		{
				event_main(&stEvent);
		}

		break;
	case KEYFN:
		if (SelectAppNum(TRUE, TRUE, &ucAppNo)==0)
		{
			if (ucAppNo == 0)
			{
				 event_main(&stEvent);
		
			}
			else
			{
				AppDoEvent(ucAppNo, &stEvent);//2014-7-25 mark
				if(ucAppNo==glSysParam.astSubApp[APP_EPS].ucAppNo)//2014-8-28 EPS T8 handling
				{
					EpsUpdateReverseState(stEvent.MagMsg.track1[0]);

       			    EpsAutoSettUpdate(&stEvent); //W204: EPS autosettlement state update
				} 
			}	
		}
		break;
	case KEYMENU:
		if (ChkIfAppEnabled(APP_AMEX))
        {
           	if (SelectAppNum(TRUE, TRUE, &ucAppNo)==0)
			{
				AppDoEvent(ucAppNo, &stEvent);
			}
        }
		else
		{
			 event_main(&stEvent);
		}
		break;
	case KEYUP:
    case KEYDOWN: 	
	default: 
		 event_main(&stEvent);
		break;
	}
	ScrCls();//manager back

  return 0;
}
#endif // APP_MANAGER_VER
// end of file



//W204:  EPS auto settlement handler
int ProcAutoRecap(void)
{
  unsigned char xstr[30];
  unsigned char currentTime[20];
  ST_EVENT_MSG    stEvent;

  if (EPS_AutoRecap)
  {
    memset(xstr, 0x00, sizeof(xstr));
    memset(currentTime,0x00, sizeof(currentTime));
    GetTime(xstr);
    PubBcd2Asc0(xstr+1, 4, currentTime);

    if (memcmp(&currentTime[4], EPS_Recap_Time,4)==0)
    {
      if (memcmp(&currentTime[0], EPS_Last_Recap_Date,4) != 0)
      {
        // do auto recap
        InitEventMsg(&stEvent, EPSAPP_AUTORECAP);
        AppDoEvent(glSysParam.astSubApp[APP_EPS].ucAppNo, &stEvent);
				
        if (stEvent.MagMsg.track2[0] == 1)
		    {
          // get recap time
          EPS_AutoRecap = 1;
          memcpy(EPS_Recap_Time, &stEvent.MagMsg.track2[1], 4);
		    }
        else
        {
          // reset recap time
          EPS_AutoRecap = 0;
          memset(EPS_Recap_Time, 0x00, sizeof(EPS_Recap_Time));
        }

        if (stEvent.MagMsg.track3[0] != 0)
        {
          // get recap time
          memcpy(EPS_Last_Recap_Date, stEvent.MagMsg.track3, 4);
        }
        else
        {
          // reset recap time
          memset(EPS_Last_Recap_Date, 0x00, sizeof(EPS_Last_Recap_Date));
        }
      }
    }

    return 1;
  }
  else
  {
    return 1;
  }
}


