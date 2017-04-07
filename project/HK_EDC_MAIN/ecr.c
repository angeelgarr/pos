
#include "global.h"

/********************** Internal macros declaration ************************/
#define ECR_TIME_ID     2		/*ECR 通讯使用2 timer*/
#define ECR_BUFFER_LEN  1024	/*ECR 通讯 buffer*/
#define BYTE_ACK        0x06
#define BYTE_NAK        0x15
#define	BYTE_STX        0x02
#define BYTE_ETX        0x03
#define STR_ECR_START   "\"<START>\""
#define STR_ECR_ACK     "\"<START>\",\"<END>\""

/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
#ifdef AMT_PROC_DEBUG//2014-11-4
static int UnpackEcrAmt(char *pszTag, char *pszValue);
static int GetEcrAmtElement(void *pszEcrMsg);
#endif
/********************** Internal variables declaration *********************/
unsigned char   now_app_num,app_num;
const unsigned char ecr_version[] = "6.13";	
/********************** external reference declaration *********************/
extern void UpdateEpsExpiryTime(void);
extern void EpsUpdateReverseState(uchar needReverse);//2014-7-10 EPS T8 issue
/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/
// 打开ECR端口
#ifdef APP_MANAGER_VER

void EcrOpen(void)
{
    uchar   ucRet;

    if (glSysParam.stECR.ucSpeed==ECRSPEED_9600)
    {
        ucRet = PortOpen(glSysParam.stECR.ucPort, (char*)"9600,8,n,1");
    }
    else
    {
        ucRet = PortOpen(glSysParam.stECR.ucPort, (char*)"115200,8,n,1");
    }

    PortReset(glSysParam.stECR.ucPort);
}

static uchar EcrPortReset(void)
{
    return PortReset(glSysParam.stECR.ucPort);
}

static int EcrRecvDataOnly(uchar *psDataBuff, ushort usDataBuffSize, ushort usTimeoutMs)
{
    uchar   ucRet;
    int     iLen;

    if (usDataBuffSize<1)
    {
        return -1;
    }
    
    iLen = 0;
	while(1)
	{
		ucRet = PortRecv(glSysParam.stECR.ucPort, psDataBuff+iLen, usTimeoutMs);
	
		if (ucRet!=0)
		{
            break;
        }

		iLen++;
        usTimeoutMs = 10;   // 10ms between char
		if (iLen>=usDataBuffSize)
		{
			break;
		}
	}

    return iLen;
}

static uchar EcrSendData_Sub(uchar *psData, ushort usLen, uchar bNeedAck)
{
    int ii, iCnt, iRet;
    uchar   ucRet, sBuff[512];

    // Max try 3 times if no ack from ECR
    for (ii=0; ii<3; ii++)
    {
        for (iCnt=0; iCnt<usLen; iCnt++)
        {
            ucRet = PortSend(glSysParam.stECR.ucPort, psData[iCnt]);
		
            if (ucRet!=0)
            {
                return ucRet;
            }
        }

        if (!bNeedAck)
        {
            return 0;
        }

        // check ECR confirmation.
		DelayMs(1000);//???not sure if need this delay
        memset(sBuff, 0, sizeof(sBuff));
        iRet = EcrRecvDataOnly(sBuff, sizeof(sBuff), 500);
        if ((iRet>0) && (strstr(sBuff, STR_ECR_ACK)!=NULL))
        {
            return 0;
        }
    }
    return 0x80;
}

uchar EcrSendData(uchar *psData, ushort usLen)
{    
    return EcrSendData_Sub(psData, usLen, TRUE);
}

static void PackEcrMsgToEvent(ST_EVENT_MSG *pstEvent, int iMsgType, uchar *pszEcrMsgBuff)
{
    int iLen = strlen(pszEcrMsgBuff);

    InitEventMsg(pstEvent, iMsgType);
    pstEvent->MagMsg.track1[0] = iLen / 0x100;
    pstEvent->MagMsg.track1[1] = iLen % 0x100;
    pstEvent->UserMsg = (void *)EcrSendData;  // pass the function to sub-app
    strcpy(&(pstEvent->MagMsg.track1[2]), pszEcrMsgBuff);
}
void PackEcrMsgToEPS(ST_EVENT_MSG *pstEvent, int iMsgType, uchar *pszEcrMsgBuff, int msgLen)//build 136 eps Jardine
{
    memset(pstEvent, 0, sizeof(ST_EVENT_MSG));
    pstEvent->MsgType = iMsgType;
    pstEvent->MagMsg.RetCode = (unsigned char) msgLen;
    strcpy(pstEvent->MagMsg.track1, pszEcrMsgBuff);
}
int ProcEcrEvent(void)//EPS should change here
{
    int     ii=0, iLen=0,DataLen=0;
	uchar   sCurTime[7];//2014-11-4
    uchar   ucRet, szBuff[ECR_BUFFER_LEN],ucLRC;
    ST_EVENT_MSG    stEvent;
	uchar sDataOut[100];
	int nDataOutLen;

	memset(szBuff,0,sizeof(szBuff));
    iLen = EcrRecvDataOnly(szBuff, sizeof(szBuff), 0);
    if (iLen<1 || iLen>sizeof(szBuff))
    {
        return -1;
    }
	DelayMs(100);//2014-5-15 0.1s

	//PubDebugOutput("msg1",szBuff,iLen,DEVICE_PRN,HEX_MODE);
    // find first quote in data
    for (ii=0; ii<iLen; ii++)
    {
        if (szBuff[ii]=='\"')
        {
            break;
        }
		if (szBuff[ii]==0x02)//eps jardine 4mat
		{
			break;
		}
    }
    if (ii>=iLen)
    {
        return -1;
    }

    // continue receive from first quote
    if (ii!=0)
    {
        memmove(szBuff, szBuff+ii, iLen-ii);
        iLen = iLen-ii;
    }
    ii = EcrRecvDataOnly(szBuff+iLen, (ushort)(sizeof(szBuff)-iLen), 0);
    iLen += ii;
	
	if(0x02==szBuff[0]) //EPS jardine
 	{
		DataLen = PubBcd2Long(szBuff+1,2,NULL);

		if (szBuff[0]!=0x02|| szBuff[iLen-2]!=0x03)
		{
			PortSend(glSysParam.stECR.ucPort, NAK);
			return -3;
		}
		//*** Check LRC ***
		ucLRC = szBuff[iLen-1];
		for (ii = 1; ii <= DataLen+3 ; ii++)
		{
			ucLRC ^= szBuff[ii];
		}	
		if (ucLRC != 0)
		{
			PortSend(glSysParam.stECR.ucPort, NAK);
			return -3;
		}

		//OK
		 PortSend(glSysParam.stECR.ucPort, ACK);
	//	PubDebugOutput("msg",szBuff+3,DataLen,DEVICE_SCR,ASC_MODE);
		PackEcrMsgToEPS(&stEvent, EDC_ECR_MSG, szBuff+3,DataLen);   
		AppDoEvent(glSysParam.astSubApp[APP_EPS].ucAppNo, &stEvent);
		EpsUpdateReverseState(stEvent.MagMsg.track1[0]);//2014-7-10 update EPS state
 	}
	else
	{

		// ensure start/end format
		if ((strstr(szBuff, "\"<START>\"")==NULL) || (strstr(szBuff, "\"<END>\"")==NULL))
		{
			return -3;
		}
    ucRet = EcrSendData_Sub((uchar *)STR_ECR_ACK, (ushort)strlen(STR_ECR_ACK), FALSE);

/*	PubDebugOutput("Here","02",2,DEVICE_PRN,ASC_MODE);//testttttt*/
    //Beep();
	Beef(6, 80);//2014-5-12 use Beef instead of beep ?
/*	PubDebugOutput("Here","03",2,DEVICE_PRN,ASC_MODE);//testttttt*/
	if (strstr(szBuff,(uchar *)STR_ECR_ACK)!=NULL)//STR_ECR_ACK will send to eventmain which will cause excption
	{
		return 0;
	}
    // TODO : add processing here
    if (strstr(szBuff, "\"FUNC=CUP\"")!=NULL)
    {
		//2014-5-14 enhancement for ECR & Modem connection
#ifdef _S60_  
			ClssClose(); //2014-5-16 enhance		
#endif 
#ifdef AMT_PROC_DEBUG//2014-11-4 record amt before send
		if(GetEcrAmtElement(szBuff)==0)
		{
			//time
			GetTime(sCurTime);
			sprintf((char *)glEcrAMT.glEcrAmtLog[glEcrAmtCnt].szTime, "%02X%02X%02X",sCurTime[3], sCurTime[4], sCurTime[5]);
			//txntype
			glEcrAMT.glEcrAmtLog[glEcrAmtCnt].szTranType = 1;
			
			////2014-11-4 ttt
			//ScrCls();
			//ScrPrint(0,0,0, "1_time: %s",glEcrAMT.glEcrAmtLog[glEcrAmtCnt].szTime);
			//ScrPrint(0,2,0, "1_type: %d",glEcrAMT.glEcrAmtLog[glEcrAmtCnt].szTranType);
			//ScrPrint(0,4,0, "1_amt : %s",glEcrAMT.glEcrAmtLog[glEcrAmtCnt].szEcrAmt);
			//getkey();

			if(glEcrAmtCnt++ > 99)
			{
				glEcrAmtCnt = 0;
			}
		}
#endif	
		//11 Apr 2012 - build23: add CUP interface
      // TODO : Call CUP
      // CUP use EDC_ECR_MSG too     
      PackEcrMsgToEvent(&stEvent, EDC_ECR_MSG, szBuff);        //EDC_ECR_MSG, CUP_ECR_MSG share the same value 1231
      //Gillian 20161129
	  if (glSysParam.stEdcInfo.ucClssFlag == 1)//send cmd to change R50 application to CUP
	  {
		 //2014-5-13 enhance tt
		 ClssOpen(); //2014-5-16 enhance	
		 WaveProcOtherProtocol("\xE5", 1, sDataOut, &nDataOutLen); 
		 //2014-5-13 enhance tt
		 ClssClose(); //2014-5-16 enhance
	  }
	  AppDoEvent(glSysParam.astSubApp[APP_CUP].ucAppNo, &stEvent);

	 
	  //2014-5-14 enhancement for ECR & Modem connection
#ifdef _S60_  
		if (glSysParam.stEdcInfo.ucClssFlag==1)//2014-5-16 enhance
		{
			ClssOpen(); //2014-5-16 enhance
		}
#endif
    }
    else if (strstr(szBuff, "\"FUNC=EPS\"")!=NULL)
    {
        // TODO : Call EPS
    }
    else
    {
        // Call EDC
        PackEcrMsgToEvent(&stEvent, EDC_ECR_MSG, szBuff);
		event_main(&stEvent);
        //AppDoEvent(glSysParam.astSubApp[APP_EDC].ucAppNo, &stEvent);
    }
	}
    EcrPortReset();
    return 0;
}
#endif
#ifdef AMT_PROC_DEBUG//2014-11-4
static int UnpackEcrAmt(char *pszTag, char *pszValue)
{
    int ii, iCnt;

    if (strcmp(pszTag, "AMT")==0)
    {
        // save amount in ISO8583 DE4 format
        if (strlen(pszValue)<14)
        {
			glEcrAMT.glEcrAmtLog[glEcrAmtCnt].szEcrAmt[12] = 0;
            for (ii=strlen(pszValue)-1, iCnt=11; ii>=0 && iCnt>=0; ii--)
            {
                if (isdigit(pszValue[ii]))
                {
                    glEcrAMT.glEcrAmtLog[glEcrAmtCnt].szEcrAmt[iCnt] = pszValue[ii];
                    iCnt--;
                }
            }
            while (iCnt>=0)
            {
                glEcrAMT.glEcrAmtLog[glEcrAmtCnt].szEcrAmt[iCnt] = '0';
                iCnt--;
            }
        }
    }
	//else
	//{
	//	return -1;
	//}

    return 0;
}

static int GetEcrAmtElement(void *pszEcrMsg)
{
    char    szBuff[512+1], szTag[32+1], szValue[64+1];
    char    *pStart, *pEnd, *pEqual, *pNow;
    int     iLen, iRet;

    iLen = strlen((char*)pszEcrMsg);    // Check length
    if (iLen>512)
    {
        return -1;
    }

    pStart = strstr(pszEcrMsg, "\"<START>\",");    // Locate start
    if (pStart==NULL)
    {
        return -1;
    }
    pStart += strlen("\"<START>\",");

    pEnd = strstr(pStart, "\"<END>\"");    // Locate end
    if (pEnd==NULL)
    {
        return -1;
    }

    memset(szBuff, 0, sizeof(szBuff));    // Get msg body
    memcpy(szBuff, pStart, pEnd-pStart-1);// ignore last ',' in string

    pNow = szBuff;
    while (strlen(pNow)!=0)
    {
        pStart = strstr(pNow, "\"");
        if (pStart==NULL)
        {
            return -1;
        }
        pEnd = strstr(pNow+1, "\"");
        if (pEnd==NULL)
        {
            return -1;
        }
        pEqual = strstr(pNow+1, "=");
        if ((pEqual==NULL) || (pEqual>pEnd) || (pEqual<pStart))
        {
            return -1;
        }

        iLen = pEqual-pStart-1;
        if ((iLen==0) || (iLen>32))
        {
            return -1;
        }

        iLen = pEnd-pEqual-1;
        if ((iLen==0) || (iLen>64))
        {
            return -1;
        }

        sprintf(szTag,   "%.*s", pEqual-pStart-1, pStart+1);
        sprintf(szValue, "%.*s", pEnd-pEqual-1,   pEqual+1);
        iRet = UnpackEcrAmt(szTag, szValue);

        pNow = pEnd+1;
        if (*pNow==',')
        {
            pNow++;
        }
    }

    return 0;
}
#endif
// end of file

