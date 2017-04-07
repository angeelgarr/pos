
/****************************************************************************
NAME
    print.h - 定义打印模块

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.12      - created
****************************************************************************/

#ifndef _PRINT_H
#define _PRINT_H

#define PRN_NORMAL		0
#define PRN_REPRINT		1

#ifndef PRN_OK
#define PRN_OK			0
#define PRN_BUSY		1
#endif

#define PRN_NO_PAPER	2
#define PRN_DATA_ERR	3
#define PRN_ERR			4
#define PRN_NO_DOT		0xFC
#define PRN_DATA_BIG	0xFE
#define PRN_CANCEL		0xA1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int   PrnCurAcqTransList(uchar prnAllFlag);//2013-10-2 Lois: when select all, check all acquirers for transaction record, unless all empty, POS display "EMPTY BATCH"
void  PrnAllList(void);
void  ConvTransName(uchar *pszOrgName, uchar *pszOutName);
void  PrnSetSmall(void);
void  PrnSetNormal(void);
void  PrnSetBig(void);
int   PrintReceipt(uchar ucPrnFlag);
#ifdef AMT_PROC_DEBUG
void  PrintAmount(void);//2014-9-18 ttt: debug printing all amounts in process
void  PrintEcrAmount(void);//2014-11-4
#endif
int   PrnTotalIssuer(uchar ucIssuerKey);
int   PrnTotalAcq(void);
int   PrnTotalAll(int flag);//2013-10-3 tttttttt
int   PrnEPPTotal(void);//2016-1-11 EPP summary printing
int   PrintSettle(uchar ucPrnFlag);
void  PrintParam(void);
int   StartPrinter(void);
void  PrnEngTime(void);
uchar PrnRateReport(uchar *pPrnData, int iDataLen, uchar ucLastPage);
int ChkIfPrnLogo(void);
int PrnAcqLogo(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */ 

#endif	// _PRINT_H

// end of file
