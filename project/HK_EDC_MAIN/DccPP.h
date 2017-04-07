/************************************************************************
NAME
    DccPP.h - Declares the PlanetPayment DCC public functions

DESCRIPTION
	PlanetPayment DCC is used by several banks. it has 2 basic modes: single-batch / dual-batch
	For dual-batch mode, a DCC acquirer is followed by a non-DCC-acquirer.

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    HeJiajiu(Gary Ho)     2009.11.04      - draft
************************************************************************/

#ifndef DCC_PP_HEADER_FILE
#define DCC_PP_HEADER_FILE


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void  PPDCC_SelfTest(void);

// main content
uchar PPDCC_ChkIfDccAcq(void);
uchar PPDCC_ChkIfHaveDccAcq(void);
uchar PPDCC_SetCurAcq(void);
uchar PPDCC_SetDccDefaults(void);

int   PPDCC_PackField62(uchar ucTranType, void *psSendPack);
int   PPDCC_PackField63(uchar ucTranType, void *psSendPack);
int   PPDCC_RetriveAllPDS(uchar *psField63, void *pProcInfo);
void  PPDCC_CreateDummyRspData(void *pstProcInfo);
void  PPDCC_PromptDownloadBin(void);
uchar PPDCC_LoadBinFile(void);
uchar PPDCC_ChkIfLocalCard(char *pszPAN);

void  PPDCC_CardBinMenu(void);

int   PPDCC_CalcLocalAmount(char *pszDivisor, CURRENCY_CONFIG *pstFrnCurrency,
                            char *psRateStr,
                            CURRENCY_CONFIG *pstLocalCurrency, char *pszOut);
int   PPDCC_CalcForeignAmount(CURRENCY_CONFIG *pstLocalCur, char *pszLocalAmt,
                              char *psRateStr,
                              CURRENCY_CONFIG *pstForeignCur, char *pszForeignAmt);
void  PPDCC_FormatRate(uchar *psRateStr, char *pszOut);
uchar PPDCC_ChkIfAutoOptIn(void);
int   PPDCC_CalForeignAmount(uchar *pszLocalAmt, uchar *psRate, uchar *pszForeignAmt);
void  PPDCC_TransRateReport(void);
int   PPDCC_TransRateEnquiry(void);
void  PPDCC_TransOptOut(void);
uchar PPDCC_ChkIfNeedFinalSelect(void);
int   PPDCC_FinalSelect(void);

void  PPDCC_ResetTransStatistic(void);
int   PPDCC_UpdateTransStatistic(uchar *psCurrency,
                                 uchar *pszOptInAmt,  char bIsAddOptIn,
                                 uchar *pszOptOutAmt, char bIsAddOptOut);
int   PPDCC_UpdateReverStatistic(void);
int   PPDCC_GetAllTransStatistic(void);
int   PPDCC_GetReversalCnt(void);
void  PPDCC_ViewStatistic(void);
void  PPDCC_DispTranLogDetail(ushort uiActRecNo, void *pTranLog);

int DccSwitchDomestic(void);  //build88S 1.0D

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

// end of file
