
#include "global.h"

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal constants declaration *********************/
/********************** Internal functions declaration *********************/
static uchar ChkIfAcqAvail(uchar ucIndex);

/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/
#if defined(_P60_S1_) || defined(_P70_S_)
extern const APPINFO	AppInfo;
#endif

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

// 检测机器型号
// Check Term Model
uchar ChkTerm(uchar ucTermType)
{
#ifdef _P60_S1_
	return (ucTermType==_TERMINAL_P60_S1_);
#elif defined(_P70_S_)
	return (ucTermType==_TERMINAL_P70_S_);
#else
	return (glSysParam.sTermInfo[HWCFG_MODEL]==ucTermType);
#endif
}

// 检查硬件配置。信息源自GetTermInfo()的返回(缓存在glSysParam.sTermInfo[])
// Check Term Hardware Config, by Checking the info returned by GetTermInfo() (buffered in glSysParam.sTermInfo[])
uchar ChkHardware(uchar ucChkType, uchar ucValue)
{
	PubASSERT(ucChkType<HWCFG_END);

	return (glSysParam.sTermInfo[ucChkType]==ucValue);	// return value: TRUE/FALSE
}

// 检查是否红外通信的分离式打印机 Check whether it is an IrDA-communication printer
uchar ChkIfIrDAPrinter(void)
{
	return (ChkTerm(_TERMINAL_P60_S1_) || ChkTerm(_TERMINAL_S60_));
}

// 1.00.0009 delete
//uchar ChkIfSupportCommType(uchar ucCommType)

uchar ChkTermPEDMode(uchar ucMode)
{
	return (glSysParam.stEdcInfo.ucPedMode==ucMode);
}

// 扫瞄所有收单行信息,判断终端是否指出EMV
// Scan all Acquirer and check whether one of them support EMV
uchar ChkIfEmvEnable(void)
{
	uchar	i;

	for(i=0; i<glSysParam.ucAcqNum; i++)
	{
		if( ChkOptionExt(glSysParam.stAcqList[i].sOption, ACQ_EMV_FEATURE) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

// 检查当前发卡行的选项
// Check current Issuer OPtion
uchar ChkIssuerOption(ushort uiOption)
{
	PubASSERT( (uiOption>>8)<sizeof(glCurIssuer.sOption) );
	return ChkOptionExt(glCurIssuer.sOption, uiOption);
}

// 检查当前收单行选项
// Check Current Acquirer option
uchar ChkAcqOption(ushort uiOption)
{
	PubASSERT( (uiOption>>8)<sizeof(glCurAcq.sOption) );
	return ChkOptionExt(glCurAcq.sOption, uiOption);
}

// 检查终端选项
// Check EDC option
uchar ChkEdcOption(ushort uiOption)
{
	PubASSERT( (uiOption>>8)<sizeof(glSysParam.stEdcInfo.sOption) );
	return ChkOptionExt(glSysParam.stEdcInfo.sOption, uiOption);
}

// 扩展选项检查
// Extension of option checking
uchar ChkOptionExt(uchar *psExtOpt, ushort uiOption)
{
	return (psExtOpt[uiOption>>8] & (uiOption & 0xFF));
}

void SetOptionExt(uchar *psExtOpt, ushort uiOption, ushort uiAction)
{
    if (uiAction & OPT_SET)
    {
	psExtOpt[uiOption>>8] |= (uiOption & 0xFF);
    }
    else
    {
        psExtOpt[uiOption>>8] &= ~(uiOption & 0xFF);
    }
}

// 功能：是否必须输入密码 入口：无 出口： TRUE 必输入 FALSE 不必输入
// Check whether need PIN for current Issuer
uchar ChkIfNeedPIN(void)
{
	return ChkIssuerOption(ISSUER_EN_PIN);
}

uchar ChkIfPinReqdAllIssuer(void)
{
	uchar	i;

	for(i=0; i<glSysParam.ucIssuerNum; i++)
	{
		if( ChkOptionExt(glSysParam.stIssuerList[i].sOption, ISSUER_EN_PIN) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

uchar ChkIfAmex(void)
{
	return ChkAcqOption(ACQ_AMEX_SPECIFIC_FEATURE);
}

uchar ChkIfBoc(void)
{
	return ChkCurAcqName("BOC", TRUE);
}

uchar ChkIfBnu(void)
{
	return ChkCurAcqName("BNU", TRUE);
}


// Citibank HK
uchar ChkIfCiti(void)
{
	if (ChkCurAcqName("CITI", TRUE)||ChkCurAcqName("DCD_CITI",TRUE)
		||ChkCurAcqName("CITIBANK",TRUE)||ChkCurAcqName("CITI_VM",TRUE)||ChkCurAcqName("CITI_",FALSE))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

uchar ChkIfCitiTaxi(void)
{
	return ChkCurAcqName("CITI_TAXI", TRUE);
}


// Fubon Bank
uchar ChkIfFubon(void)
{
	return ChkCurAcqName("FUBO", TRUE);
}

// If now acquirer is DahSing bank or BCM Bank(Banco Comercial De Macau).
uchar ChkIfDahOrBCM(void)
{
	return (ChkCurAcqName("DAHSING", TRUE) || ChkCurAcqName("DSB", TRUE) 
			|| ChkCurAcqName("DAH SING", TRUE)||ChkCurAcqName("BCM",TRUE));
}

// Bank of East Asia
uchar ChkifBEADCC(void)
{
	if (ChkCurAcqName("DCC_BEA", TRUE))
		 	{
		return TRUE;
		 	}
	else
	{
		return FALSE;
	}
}
uchar ChkIfBeaHalf(void)
{
	if (ChkCurAcqName("BEA", FALSE))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
uchar ChkIfBea(void)
{
	if (ChkCurAcqName("BEA", TRUE))
	{
		return TRUE;
	}
	else if (ChkCurAcqName("BEA_EMV", TRUE))
	{
		return TRUE;
	}
	else if (ChkCurAcqName("DCD_BEA", TRUE))
	{
		return TRUE;
	}
	else
	{
	return FALSE;
	}
	
//	return (ChkCurAcqName("BEA", TRUE)||ChkCurAcqName("BEA_EMV", TRUE)||ChkCurAcqName("DCD_BEA", TRUE));
}//TTTTT BUILD117 SQUALL
// uchar ChkIfBeaEMV(void)
// {
// 	return ChkCurAcqName("BEA_EMV", TRUE);
// }
// uchar ChkIfBeaDCD(void)
// {
// 	return ChkCurAcqName("DCD_BEA", TRUE);
// }


// Standard Chartered
uchar ChkIfScb(void)
{
	return ChkCurAcqName("SCB", TRUE);
}
uchar ChkIfDCC_CITI(void)
{
	return ChkCurAcqName( "DCC_CITI",TRUE);
}
uchar ChkIfWordCard(void)
{
	return ChkCurAcqName("WORLDCARD", TRUE);
}

uchar ChkIfUob(void)
{
	return (memcmp(glCurAcq.szNii, "002", 3)==0);
//	return ChkCurAcqName("UOB", TRUE);
}

uchar ChkIfUobIpp(void)
{
	return ChkCurAcqName("UOB-IPP", TRUE);
}

// Diners for non-specific bank
uchar ChkIfDiners(void)
{
	return ChkCurAcqName("DINERS", FALSE);
}

// JCB for non-specific bank
uchar ChkIfJCB(void)
{
	return ChkCurAcqName("JCB", FALSE);
}

// ICBC(ASIA) in HK
uchar ChkIfICBC(void)
{
	return ChkCurAcqName("ICBC_HK", FALSE);
}

// ICBC Macau branch
uchar ChkIfICBC_MACAU(void)
{
	return ChkCurAcqName("ICBC_MACAU", FALSE);
}

// Wing Hang Bank
uchar ChkIfWingHang(void)
{
    return ChkCurAcqName("WINGHANG", FALSE);
}

#ifdef SUPPORT_TABBATCH
// Merchant Solutions - Standard Chartered Bank
uchar ChkIfMSSCB(void)
{
    if (ChkCurAcqName("DCC", TRUE))
    {
        return FALSE;
    }

    /*build88R: JCB will also be in MS format
    return (ChkCurAcqName("MS_", TRUE) || ChkCurAcqName("SCB", FALSE));
    */
    return (ChkCurAcqName("MS_", TRUE) || ChkCurAcqName("SCB", FALSE) || ChkCurAcqName("JCB", FALSE));
	//acquirer name which can do instalment 
}
#endif

// Shanghai Commercial Bank (HK)
uchar ChkIfShanghaiCB(void)
{
    return ChkCurAcqName("SHCB", TRUE);
}

uchar ChkIfHSBC(void)
{
	if( ChkIfAmex()   || ChkIfBoc()  || ChkIfBea() || ChkIfFubon() ||
		ChkIfDahOrBCM()    || ChkIfCiti() || ChkIfScb() || ChkIfUob()   ||
		ChkIfUobIpp() || ChkIfWordCard() || ChkIfDiners() || ChkIfJCB() ||
		ChkIfICBC() || ChkIfICBC_MACAU() || ChkIfWingHang() || ChkIfShanghaiCB()
		|| ChkIfWLB())

	{
		return FALSE;
	}

	return TRUE;
}

#ifdef SUPPORT_TABBATCH
// Check all acquirer to find if there's any Merchant Solutions SCB acquirer.
uchar ChkIfAllAcq_HaveMSSCB(void)
{
    int ii;
    ACQUIRER    stBakAcq;

    stBakAcq = glCurAcq;
    for (ii=0; ii<glSysParam.ucAcqNum; ii++)
    {
        SetCurAcq((uchar)ii);
        if (ChkIfMSSCB())
        {
            glCurAcq = stBakAcq;
            return TRUE;
        }
    }
    glCurAcq = stBakAcq;
    return FALSE;
}
#endif

uchar ChkifWLBfalse(void)
{
	if(ChkCurAcqName("WLB",FALSE))
	{
		return TRUE;
	}
	return FALSE;
}
uchar ChkIfWLB(void)
{
	if(ChkCurAcqName("WLB",TRUE)||ChkCurAcqName("CHB",TRUE))
	{
		return TRUE;
	}
	return FALSE;
}

uchar ChkIfCHBFalse(void)
{
	if (ChkCurAcqName("CHB",FALSE))
	{
		return TRUE;
	}
	return FALSE;
}

uchar ChkIfCHB(void)
{
	if (ChkCurAcqName("CHB",TRUE))
	{
		return TRUE;
	}
	return FALSE;
}

// 检查磁卡交易金额是否低于Floor Limit
// Check whether below floor limit. Only for Non-EMV.
uchar ChkIfBelowMagFloor(void)
{
	int		ii;
	uchar	szBuff[12+1];

	sprintf((char *)szBuff, "%lu", glCurIssuer.ulFloorLimit);
	for (ii=0; ii<glSysParam.stEdcInfo.stLocalCurrency.ucDecimal; ii++)
	{
		strcat(szBuff, "0");
	}
	AmtConvToBit4Format(szBuff, glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit);
	// Now szBuff is local floor limit
	if( memcmp(glProcInfo.stTranLog.szAmount, szBuff, 12)>=0 )
	{
		return FALSE;
	}

	return TRUE;
}
uchar ChkIfPrnSignature(void)
{
	uchar	szBuff[12+1];
	
	//2015-12-12 add DCC check: if DCC, must sign
	if(PPDCC_ChkIfDccAcq())
	{
		return TRUE;
	}
	if (glSysParam.stEdcInfo.ucClssFlag == 1 && glProcInfo.stTranLog.uiEntryMode & MODE_CONTACTLESS)
	{
		if(!ChkIfAmex())//2016-5-30
		{
			if(glProcInfo.stTranLog.ucTranType == REFUND)//Jason 33-08 2016.03.03 16:4//2016-3-31 
			{
				return TRUE;
			}
		}
		//if(glProcInfo.stTranLog.szCVMRslt[0]== CLSS_CVM_SIG )//2016-4-19 mark!
		if(glClssCVMFlag == 1)//2016-4-25
        {					
            return TRUE;//2016-4-19 need signature!//2016-5-30
        }

		//Gillian 20160729 Gillian 20160922 remove
		/*if((atol(glProcInfo.stTranLog.szAmount)>glSysParam.stEdcInfo.ulClssSigFlmt))
		{
			return TRUE;
		}*/

		//2016-5-30 remove
		//sprintf((char *)szBuff, "%lu", glSysParam.stEdcInfo.ulClssSigFlmt);
		//AmtConvToBit4Format(szBuff, glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit);
		//// Now szBuff is local floor limit
		//if( memcmp(glProcInfo.stTranLog.szAmount, szBuff, 12)>=0 )
		//{
		//	return TRUE;
		//}	
	}
	else 
	{
		if ( (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) || (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE))
		{
			return TRUE;
		}		
		if (glProcInfo.stTranLog.szPan[0]!= '4' && /*glProcInfo.stTranLog.szPan[0]!='5'*/!ChkIfMC(glProcInfo.stTranLog.szPan) //2016-2-15
			&& memcmp(glProcInfo.stTranLog.szPan,"34",2)!=0 
			&& memcmp(glProcInfo.stTranLog.szPan,"37",2)!=0)
		{
			return TRUE;
		}
		// Build 1.00.0116f
		if ((glProcInfo.stTranLog.ucTranType != SALE) && (glProcInfo.stTranLog.ucTranType != VOID))
		{
			return TRUE;
		}
		if (glProcInfo.stTranLog.szPan[0]=='4')
		{
			if (glSysParam.stEdcInfo.ulNoSignLmtVisa ==0)
			{
				return TRUE;
			}
			else
			{
				sprintf((char *)szBuff, "%lu", glSysParam.stEdcInfo.ulNoSignLmtVisa);
				AmtConvToBit4Format(szBuff, glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit);
				// Now szBuff is local floor limit
				if( memcmp(glProcInfo.stTranLog.szAmount, szBuff, 12)>=0 )
				{
					return TRUE;
				}
			}
		}
		else if (/*glProcInfo.stTranLog.szPan[0]=='5'*/ChkIfMC(glProcInfo.stTranLog.szPan) )//2016-2-15
		{
			if (glSysParam.stEdcInfo.ulNoSignLmtMaster == 0)
			{
				return TRUE;
			}
			else
			{
				sprintf((char *)szBuff, "%lu", glSysParam.stEdcInfo.ulNoSignLmtMaster);
				AmtConvToBit4Format(szBuff, glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit);
				// Now szBuff is local floor limit
				if( memcmp(glProcInfo.stTranLog.szAmount, szBuff, 12)>=0 )
				{
					return TRUE;
				}
			}
		}
		else if (memcmp(glProcInfo.stTranLog.szPan,"34",2)== 0 || memcmp(glProcInfo.stTranLog.szPan,"37",2)==0)
		{
			if (glSysParam.stEdcInfo.ulNoSignLmtAmex==0)
			{
				return TRUE;
			}
			else
			{
				sprintf((char *)szBuff, "%lu", glSysParam.stEdcInfo.ulNoSignLmtAmex);
				AmtConvToBit4Format(szBuff, glSysParam.stEdcInfo.stLocalCurrency.ucIgnoreDigit);
				// Now szBuff is local floor limit
				if( memcmp(glProcInfo.stTranLog.szAmount, szBuff, 12)>=0 )
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

// 在未事先确定交易类型的情况下，自动判断选择需要进行的交易
void CheckCapture(void)
{
	if( glProcInfo.stTranLog.ucTranType!=SALE_OR_AUTH )
	{
		return;
	}

	if (ChkInstalmentAllAcq() && ChkEdcOption(EDC_ENABLE_INSTALMENT))	// 当所有ACQ都是installment
	{
		glProcInfo.stTranLog.ucTranType = INSTALMENT;
	}
	else if( ChkEdcOption(EDC_CASH_PROCESS) )
	{
		glProcInfo.stTranLog.ucTranType = CASH;
	}
	else if( ChkIssuerOption(ISSUER_CAPTURE_TXN) )
	{
		glProcInfo.stTranLog.ucTranType = SALE;
	}
	else
	{
		if( ChkEdcOption(EDC_AUTH_PREAUTH) )
		{
			glProcInfo.stTranLog.ucTranType = AUTH;
		}
		else
		{
			glProcInfo.stTranLog.ucTranType = PREAUTH;
		}
	}
}

uchar ChkInstalmentAllAcq(void)
{
	uchar	sBuff[256];
	uchar	i;

	memset(sBuff, 0, sizeof(sBuff));
	for(i=0; i<glSysParam.ucPlanNum; i++)
	{
		sBuff[glSysParam.stPlanList[i].ucAcqIndex] = 1;
	}

	for(i=0; i<glSysParam.ucAcqNum; i++)
	{
		if( sBuff[i]==0 )
		{
			return FALSE;
		}
	}

	return TRUE;
}

uchar ChkIfCurAcqEMVEnable(void)
{
	return ChkAcqOption(ACQ_EMV_FEATURE);
}

uchar ChkIfDispMaskPan2(void)
{
	return ChkOptionExt(glCurIssuer.sPanMask, 0x0040);
}

uchar ChkIfInstalmentPara(void)
{
	if( !ChkEdcOption(EDC_ENABLE_INSTALMENT) )
	{
		return FALSE;
	}

	return (glSysParam.ucPlanNum > 0);
}

uchar ChkIfTransMaskPan(uchar ucCurPage)
{
	uchar	ucTrans;
	
	PubASSERT(ucCurPage>=0 && ucCurPage<4);
	
	if (ChkIfAmex())
	{
		ucTrans = glProcInfo.stTranLog.ucTranType;
		if (ucTrans==VOID)
		{
			ucTrans = glProcInfo.stTranLog.ucOrgTranType;
		}
		
		if ( (ucTrans==PREAUTH) || (ucTrans==AUTH) )
		{
			if ( ((glProcInfo.stTranLog.ucTranType==PREAUTH) || (glProcInfo.stTranLog.ucTranType==AUTH)) &&
				!ChkEdcOption(EDC_AUTH_PAN_MASKING))
			{
				return FALSE;
			}
		}
		if (glProcInfo.stTranLog.uiEntryMode & MODE_MANUAL_INPUT)
		{
			//if (ucCurPage!=0)
			if (ucCurPage!=1)
			{
				return FALSE;
			}
		}
	}
	else
	{
		if ( ((glProcInfo.stTranLog.ucTranType==PREAUTH) || (glProcInfo.stTranLog.ucTranType==AUTH)) &&
			!ChkEdcOption(EDC_AUTH_PAN_MASKING))
		{
			return FALSE;
		}
	}

	return TRUE;
}

// compare acquirer name in upper case
uchar ChkCurAcqName(void *pszKeyword, uchar ucPrefix)
{
	uchar	szBuff[10+1];

	sprintf((char *)szBuff, "%.10s", glCurAcq.szName);
	PubStrUpper(szBuff);

	if (ucPrefix)	// the specific string is only allowed at the beginning of the acquirer name
	{
		if( memcmp(szBuff, pszKeyword, strlen((char *)pszKeyword))==0 )
		{
			return TRUE;
		}
	} 
	else	// the specific string is allowed at any place within acquirer name string
	{
		if (strstr(szBuff, pszKeyword)!=NULL)
		{
			return TRUE;
		}
	}

	return FALSE;
}

uchar ChkIfTranAllow(uchar ucTranType)
{
	if( ucTranType==SETTLEMENT || ucTranType==UPLOAD )
	{
		return TRUE;
	}

	if( GetTranLogNum(ACQ_ALL)>=MAX_TRANLOG )
	{
		PubShowTitle(TRUE, (uchar *)_T("BATCH FULL"));
		PubDispString(_T("PLS SETTLE BATCH"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(5);
		return FALSE;
	}

	if( (glProcInfo.stTranLog.ucTranType==INSTALMENT) && !ChkIfInstalmentPara() )
	{
		return FALSE;
	}

	return TRUE;
}

uchar ChkIfZeroAmt(uchar *pszIsoAmountStr)
{
	if (strcmp(pszIsoAmountStr, "")==0)
	{
		return TRUE;
	}
	if (strcmp(pszIsoAmountStr, "0")==0)
	{
		return TRUE;
	}
	return (strcmp(pszIsoAmountStr, "000000000000")==0);
}

uchar ChkIfBatchEmpty(void)
{
	int	ii;
	for (ii=0; ii<MAX_TRANLOG; ii++)
	{
		if (glSysCtrl.sAcqKeyList[ii] != INV_ACQ_KEY)
		{
			return FALSE;
		}
	}
	return TRUE;
}


#ifdef SUPPORT_TABBATCH
uchar ChkIfTabBatchEmpty(void)
{
	int	ii;
	for (ii=0; ii<MAX_TRANLOG; ii++)
	{
		if (glSysCtrl.astAuthRecProf[ii].ucAcqKey != INV_ACQ_KEY)
		{
			return FALSE;
		}
	}
	return TRUE;
}
#endif

uchar ChkIfZeroTotal(void *pstTotal)
{
	TOTAL_INFO	*pTotal = (TOTAL_INFO *)pstTotal;

	if ( pTotal->uiSaleCnt==0 && pTotal->uiRefundCnt==0 &&
		pTotal->uiVoidSaleCnt==0 && pTotal->uiVoidRefundCnt==0 )
	{
		return TRUE;
	}
	return FALSE;
}

uchar ChkSettle(void)
{
	if( glSysCtrl.sAcqStatus[glCurAcq.ucIndex]==S_PENDING )
	{
		PubShowTitle(TRUE, (uchar *)_T("SETTLE PENDING"));
		PubDispString(_T("PLS SETTLE BATCH"), 4|DISP_LINE_LEFT);
		PubLongBeep();
		PubWaitKey(5);
		return FALSE;
	}

	return TRUE;
}

uchar ChkIfNeedTip(void)
{
	//Lijy
	if( ChkIfCasinoMode() || glSysParam.stEdcInfo.ucClssFlag == 1)  //Mandy add 11-06-09
	{
		return FALSE;
	}
	//
	if( ChkEdcOption(EDC_TIP_PROCESS) &&
		((glProcInfo.stTranLog.ucTranType==SALE)     ||
		 (glProcInfo.stTranLog.ucTranType==OFF_SALE) ||
		 (glProcInfo.stTranLog.ucTranType==CASH))      )
	{
		return TRUE;
	}

	return FALSE;
}

uchar ChkIfAcqAvail(uchar ucIndex)
{
	return (glSysCtrl.sAcqStatus[ucIndex]!=S_RESET);
}

uchar ChkIfDccBOC(void)	// BOC DCC acquirer
{
	return (ChkCurAcqName("DCC_BOC", TRUE) && ChkIfAcqAvail((uchar)(glCurAcq.ucIndex+1)));
}

uchar ChkIfDccAcquirer(void)
{
	return ChkCurAcqName("DCC", FALSE);
}

uchar ChkIfDccBocOrTas(void)	// !!!! to be applied.
{
	return (ChkCurAcqName("DCC_BOC", TRUE));
}

uchar ChkIfIccTran(ushort uiEntryMode)
{
	if( (uiEntryMode & MODE_CHIP_INPUT) ||
		(uiEntryMode & MODE_FALLBACK_SWIPE) ||
		(uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
		return TRUE;
	}

	return FALSE;
}

uchar ChkIfPrnReceipt(void)
{
	PubASSERT( glProcInfo.stTranLog.ucTranType<MAX_TRANTYPE );
	return (glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct & PRN_RECEIPT);
}

uchar ChkIfNeedReversal(void)
{
	PubASSERT( glProcInfo.stTranLog.ucTranType<MAX_TRANTYPE );
	return (glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct & NEED_REVERSAL);
}

uchar ChkIfSaveLog(void)
{
	PubASSERT( glProcInfo.stTranLog.ucTranType<MAX_TRANTYPE );
	return (glTranConfig[glProcInfo.stTranLog.ucTranType].ucTranAct & WRT_RECORD);
}

uchar ChkIfThermalPrinter(void)
{
#if defined(_P60_S1_) || defined(_P70_S_)
    return FALSE;
#endif
	return (glSysParam.stEdcInfo.ucPrinterType==1);
}
#if 0
uchar ChkIfNeedSecurityCode(void)
{
	if( glProcInfo.stTranLog.ucTranType==OFF_SALE || (glProcInfo.stTranLog.ucTranType==VOID && !ChkIfAmex()))
	{
		
		return FALSE;
	}

	if( ChkIfAmex() )
	{
		if( glProcInfo.stTranLog.ucTranType==REFUND && !ChkAcqOption(ACQ_ONLINE_REFUND) )
		{

			return FALSE;
		}
		else if(glProcInfo.stTranLog.ucTranType==VOID && (INSTAL_VOID == 1)) //Gillian 20161020
		{
			INSTAL_VOID = 0;
			return TRUE;
		}
	}
	else if( glProcInfo.stTranLog.ucTranType==REFUND )
	{
		ScrCls();
				ScrPrint(0,1,0,"FALSE3"); //Gillian debug
				getkey();
		return FALSE;
	}

	if( glProcInfo.stTranLog.uiEntryMode & MODE_SWIPE_INPUT ||
		glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE )
	{
		if( ChkIssuerOption(ISSUER_SECURITY_SWIPE) )
		{
			ScrCls();
				ScrPrint(0,1,0,"FALSE4"); //Gillian debug
				getkey();
			return TRUE;
		}
	}
	else if( (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) ||
			 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
			ScrCls();
				ScrPrint(0,1,0,"FALSE8"); //Gillian debug
				getkey();
		if( ChkIssuerOption(ISSUER_SECURITY_MANUL) )
		{
			ScrCls();
				ScrPrint(0,1,0,"FALSE5"); //Gillian debug
				getkey();
			return TRUE;
		}
	}
	else if(ChkIfAmex() && glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT)  //Gillian 20161020
	{
		if( ChkIssuerOption(ISSUER_SECURITY_INSERT) )
		{
			ScrCls();
				ScrPrint(0,1,0,"FALSE7"); //Gillian debug
				getkey();
			return TRUE;
		}
	}
	else if(ChkIfAmex() && glProcInfo.stTranLog.uiEntryMode & MODE_MANUAL_INPUT)
	{
		if( ChkIssuerOption(ISSUER_SECURITY_MANUL) )
		{
			return TRUE;
		}
	}
	ScrCls();
				ScrPrint(0,1,0,"FALSE6"); //Gillian debug
				getkey();
	return FALSE;
}
#endif


uchar ChkIfNeedSecurityCode(void)
{
	if( glProcInfo.stTranLog.ucTranType==OFF_SALE || glProcInfo.stTranLog.ucTranType==VOID )
	{
		return FALSE;
	}

	if( ChkIfAmex() )
	{
		if( glProcInfo.stTranLog.ucTranType==REFUND && !ChkAcqOption(ACQ_ONLINE_REFUND) )
		{
			return FALSE;
		}
		if( (glProcInfo.stTranLog.uiEntryMode & MODE_CHIP_INPUT) )
		{
			if( ChkIssuerOption(ISSUER_SECURITY_INSERT) )
			{
				return TRUE;
			}
		}
	}
	else if( glProcInfo.stTranLog.ucTranType==REFUND )
	{
		return FALSE;
	}

	if( glProcInfo.stTranLog.uiEntryMode & MODE_SWIPE_INPUT ||
		glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_SWIPE )
	{
		if( ChkIssuerOption(ISSUER_SECURITY_SWIPE) )
		{
			return TRUE;
		}
	}
	else if( (glProcInfo.stTranLog.uiEntryMode & MODE_MANUAL_INPUT) ||
			 (glProcInfo.stTranLog.uiEntryMode & MODE_FALLBACK_MANUAL) )
	{
		if( ChkIssuerOption(ISSUER_SECURITY_MANUL) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

uchar ChkIfNeedMac(void)
{
	return FALSE;
}

uchar ChkIfAcqNeedTC(void)
{
	if (ChkIfBnu() || ChkIfICBC_MACAU() || ChkIfDahOrBCM() || ChkIfWingHang() || ChkIfCiti() || ChkCurAcqName("AMEX_MACAU", TRUE)) //Gillian 20161201
	{
		return TRUE;
	}

	// more banks may need to add in later
	return FALSE;
}

uchar ChkIfAcqNeedDE56(void)
{
    if (ChkIfAcqNeedTC() || PPDCC_ChkIfDccAcq())
    {
        return FALSE;
    }
	//Gillian 20161117
	if(ChkCurAcqName("AMEX_MACAU", TRUE) /*&& (glProcInfo.stTranLog.ucTranType == SALE)*/)
	{
		return FALSE;
	}
    return TRUE;
}

// 检查是否允许退出程序
uchar ChkIfAllowExit(void)
{
#ifdef _WIN32
	return FALSE;
#else
	APPINFO	stTempAppInf;

	if (ReadAppInfo(0, &stTempAppInf)==0)
	{
		return TRUE;
	}
#ifdef _P60_S1_
	if ( (FindAcqIdxByName("DAHSING", FALSE)<MAX_ACQ) ||
		(FindAcqIdxByName("DSB", FALSE)<MAX_ACQ)||
		(FindAcqIdxByName("DAH SING", FALSE)<MAX_ACQ)||
		(FindAcqIdxByName("BCM", FALSE)<MAX_ACQ)||
		(FindAcqIdxByName("dahsing", FALSE)<MAX_ACQ) ||
		(FindAcqIdxByName("DahSing", FALSE)<MAX_ACQ)||
		(FindAcqIdxByName("Dahsing", FALSE)<MAX_ACQ)
		)
	{
		return TRUE;
	}
#endif
	return FALSE;
#endif
}
// Lijy 2010-12-31 
uchar ChkIfCasinoMode(void)
{
    return (glSysParam.stEdcInfo.ucCasinoMode == TRUE);
}
int ChkIfIssuerVMJ(void)
{
    uchar szIssuerName[10+1];

	ConvIssuerName(glCurIssuer.szName, szIssuerName);

	if (memcmp(szIssuerName, "MASTERCARD", 8) &&
        memcmp(szIssuerName, "MC", 2) &&
        memcmp(szIssuerName, "MASTER", 6) &&
        memcmp(szIssuerName, "VISA", 4) &&
        memcmp(szIssuerName, "JCB", 3))
	{
		return FALSE;
	}

    return TRUE;
}

//----------------add by richard 20161110 for indirect CUP Acq---------start
//Dahsing Bank
uchar ChkIfCupDsb(void)
{
	return (ChkCurAcqName("CUP_DSB", TRUE));
}

//ChongHing Bank
uchar ChkIfCupChb(void)
{
	return (ChkCurAcqName("CUP_CHB", TRUE));
}

//WingLung Bank
uchar ChkIfCupWlb(void)
{
	return (ChkCurAcqName("CUP_WLB", TRUE));
}

//FuBon Bank
uchar ChkIfCupFubon(void)
{
	return (ChkCurAcqName("CUP_FUBON", TRUE));
}



uchar ChkIfIndirectCupAcq(void)
{
	return (ChkIfCupDsb() ||ChkIfCupChb() || ChkIfCupWlb()|| ChkIfCupFubon());
}

//check if exist IndrectCupAcp anywhere in EDC
uchar ChkAnyIndirectCupAcq(void)
{
    int i;
    ACQUIRER    stBakAcq = glCurAcq;

    for ( i= 0; i < glSysParam.ucAcqNum; i++)
    {
        SetCurAcq((uchar)i);
        if (ChkIfIndirectCupAcq())
        {
            glCurAcq = stBakAcq;
            return TRUE;
        }
    }

    glCurAcq = stBakAcq;
    return FALSE;
}

uchar ChkExistAcq(uchar *acqName)
{
    uchar ucCounter;
    
    for (ucCounter=0;ucCounter<glSysParam.ucAcqNum;ucCounter++)
    {
        if(!strcmp(acqName, glSysParam.stAcqList[ucCounter].szName))
        {
            return TRUE;
        }
    }

    return FALSE;
}
//---------------add by richard 20161110 for indirect CUP Acq-----------end

// end of file


