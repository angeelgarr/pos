
#include "global.h"

/********************** Internal macros declaration ************************/

// PED function for P8010 not implement [11/20/2006 Tommy]

/********************** Internal structure declaration *********************/
typedef struct _tagPASSWD_INFO
{
	uchar	szMsg[16+1];
	uchar	ucLen;
}PASSWD_INFO;

/********************** Internal functions declaration *********************/
static uchar PasswordSub(uchar ucPwdID);
static uchar PasswordNew(uchar *psOutPwd, uchar ucLen);
static void  ModifyPasswordSub(uchar ucPwdID);

/********************** Internal variables declaration *********************/
static PASSWD_INFO sgPwdInfo[] =
{
	// 顺序必须与PWD_xxx的定义顺序一致
	{_T_NOOP("BANK PWD   "), 6},
	{_T_NOOP("ENT TERMINAL PWD"), 4},
	{_T_NOOP("ENT MERCHANT PWD"), 4},
	{_T_NOOP("VOID PWD   "), 4},
	{_T_NOOP("REFUND PWD "), 4},
	{_T_NOOP("ADJUST PWD "), 4},
	{_T_NOOP("SETTLEMENT PWD  "), 4},
};

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

void ResetPwdAll(void)
{
	int	iCnt;
	
	for (iCnt=0; iCnt<PWD_MAX; iCnt++)
	{
		sprintf((char *)&glSysParam.sPassword[iCnt][0], "%0*d", sgPwdInfo[iCnt].ucLen, 0);
	}
}

uchar PasswordBank(void)
{
	return PasswordSub(PWD_BANK);
}

uchar PasswordTerm(void)
{
	return PasswordSub(PWD_TERM);
}

uchar PasswordMerchant(void)
{
	return PasswordSub(PWD_MERCHANT);
}

uchar PasswordVoid(void)
{
	return PasswordSub(PWD_VOID);
}

uchar PasswordRefund(void)
{
	return PasswordSub(PWD_REFUND);
}

uchar PasswordAdjust(void)
{
	return PasswordSub(PWD_ADJUST);
}

uchar PasswordSettle(void)
{
	return PasswordSub(PWD_SETTLE);
}

void ModifyPasswordBank(void)
{
	ModifyPasswordSub(PWD_BANK);
}

void ModifyPasswordTerm(void)
{
	ModifyPasswordSub(PWD_TERM);
}

void ModifyPasswordMerchant(void)
{
	ModifyPasswordSub(PWD_MERCHANT);
}

void ModifyPasswordVoid(void)
{
	ModifyPasswordSub(PWD_VOID);
}

void ModifyPasswordRefund(void)
{
	ModifyPasswordSub(PWD_REFUND);
}

void ModifyPasswordAdjust(void)
{
	ModifyPasswordSub(PWD_ADJUST);
}

void ModifyPasswordSettle(void)
{
	ModifyPasswordSub(PWD_SETTLE);
}

uchar PasswordSub(uchar ucPwdID)
{
	uchar	ucRet, szBuff[20];

	if( ucPwdID>sizeof(sgPwdInfo)/sizeof(sgPwdInfo[0]) )
	{
		return 1;
	}

	while( 1 )
	{
		//2014-10-22 ecr settle pwd Gillian 20161010
		//if(ChkEdcOption(EDC_ECR_ENABLE) /*&& (glProcInfo.stTranLog.ucTranType == SETTLEMENT)*/ && strlen((char*)glPwd) != 0)//2015-10-15 ECR Update:also apply for VOID
		/*{
			//memcpy(szBuff, glStlPwd, strlen((char*)glStlPwd));
			sprintf((char*)szBuff, "%.*s",strlen((char*)glPwd)+1,glPwd);//2014-10-23
		}
		else
		{*/
			ScrClrLine(2, 7);
			PubDispString(_T(sgPwdInfo[ucPwdID].szMsg), 2|DISP_LINE_LEFT);
			memset(szBuff, 0, sizeof(szBuff));
			ucRet = PubGetString(PASS_IN, sgPwdInfo[ucPwdID].ucLen, 6, szBuff, USER_OPER_TIMEOUT);	// 取消了用PED输入的做法
			if( ucRet!=0 )
			{
				return 1;
			}
		//}
		// 取消了用PED输入的做法
		if( memcmp(szBuff, glSysParam.sPassword[ucPwdID], sgPwdInfo[ucPwdID].ucLen)==0 &&
			strlen((char *)szBuff)==(int)sgPwdInfo[ucPwdID].ucLen )
		{
			return 0;
		}

		if( memcmp(szBuff, glSysParam.sPassword[PWD_BANK], sgPwdInfo[PWD_BANK].ucLen)==0 &&
			strlen((char *)szBuff)==(int)sgPwdInfo[PWD_BANK].ucLen )
		{
			return 0;
		}

		ScrClrLine(4, 7);
		PubDispString(_T("PWD ERROR!"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(3);
	}
}


void ModifyPasswordSub(uchar ucPwdID)
{
	if( PasswordSub(ucPwdID)!=0 )
	{
		return;
	}

	if( PasswordNew(glSysParam.sPassword[ucPwdID], sgPwdInfo[ucPwdID].ucLen)!=0 )
	{
		return;
	}

	SavePassword();
	PubShowTitle(TRUE, (uchar *)"CHANGE PWD");
	PubDispString(_T("PWD CHANGED!"), 4|DISP_LINE_LEFT);
	PubBeepOk();
	PubWaitKey(3);
}

uchar PasswordNew(uchar *psOutPwd, uchar ucLen)
{
	uchar	ucRet, sBuff1[20], sBuff2[20];

	while( 1 )
	{
		ScrClrLine(4, 7);
		PubDispString(_T("ENTER NEW PWD"), 4|DISP_LINE_LEFT);
		ucRet = PubGetString(PASS_IN, ucLen, ucLen, sBuff1, USER_OPER_TIMEOUT);		// 取消了用PED输入的做法
		if( ucRet!=0 )
		{
			return 1;
		}

		ScrClrLine(4, 7);
		PubDispString(_T("RE-ENTER NEW PWD"), 4|DISP_LINE_LEFT);
		ucRet = PubGetString(PASS_IN, ucLen, ucLen, sBuff2, USER_OPER_TIMEOUT);		// 取消了用PED输入的做法
		if( ucRet!=0 )
		{
			return 1;
		}

		// 取消了用PED输入的做法
		if( memcmp(sBuff1, sBuff2, ucLen)==0 )
		{
			memcpy(psOutPwd, sBuff1, ucLen);
			return 0;
		}

		ScrClrLine(4, 7);
		PubDispString(_T("NOT CONSISTENT"), 6|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(3);
	}
}

// 取消了用PED输入的做法
// uchar PasswordCheck(uchar *password6, uchar *password4, uchar *password_chk)

// end of file

