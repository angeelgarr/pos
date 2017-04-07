
/****************************************************************************
NAME
    ecrtrans.h - 定义管理器事件处理模块

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    HeJJ     2010.09.20      - created
****************************************************************************/

#ifndef EDC_MAIN_HK_ECRTRANS_H
#define EDC_MAIN_HK_ECRTRANS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// TODO : add function declarations here.
int EcrTrans(void *pszEcrMsg);
int EcrGetAmount(uchar *pszAmount, uchar *pszTip);
ulong EcrGetTxnID(void);
ulong EcrGetInvoice(void);
int EcrGetInstPlan(uchar *pucMonth);
int EcrSendAckStr(void);
int EcrSendTransSucceed(void);
int EcrSendTranFail(void);
int EcrSendAcqTotal(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// EDC_MAIN_HK_ECRTRANS_H

// end of file
