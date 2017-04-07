
/****************************************************************************
NAME
    fileoper.h - 定义所有文件操作的函数

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.12      - created
****************************************************************************/

#ifndef _FILEOPER_H
#define _FILEOPER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int InitTranLogFile(void);
#ifdef SUPPORT_TABBATCH
int MS_InitTabBatchLogFile(void);
#endif
int LoadSysParam(void);
int SaveSysParam(void);
int SaveEdcParam(void);
int SavePassword(void);
int ExistSysFiles(void);
int ValidSysFiles(void);
void RemoveSysFiles(void);
int LoadSysCtrlAll(void);
int SaveSysCtrlAll(void);
int SaveSysCtrlBase(void);
int SaveSysCtrlNormal(void);
int SaveField56(void);
int SaveRevInfo(uchar bSaveRevInfo);
int SaveRePrnStlInfo(void);
int LoadTranLog(void *pstLog, ushort uiIndex);
int UpdateTranLog(void *pstLog, ushort uiIndex);
int SaveTranLog(void *pstLog);
int RecoverTranLog(void);
ushort GetTranLogNum(uchar ucAcqKey);
uchar AllowDuplicateTran(void);
int GetRecord(ushort uiStatus, void *pstOutTranLog);
int GetRecordByInvoice(ulong ulInvoiceNo, ushort uiStatus, void *pstOutTranLog);
#ifdef SUPPORT_TABBATCH
int LoadTabBatchLog(void *pstLog, ushort uiIndex);  
int UpdateTabBatchLog(void *pstLog, ushort uiIndex);
int SaveTabBatchLog(void *pstLog);
int RecoverTabBatchLog(void);
ushort GetTabBatchLogNum(uchar ucAcqKey);
int GetAuthRecord(ushort uiStatus, void *pstOutTranLog);
int GetAuthRecordByInvoice(ushort uiStatus, ulong ulInvoiceNo, void *pstOutTranLog);
#endif
int WriteEPSPara(void *pPara);
void CalcTotal(uchar ucAcqKey);
int SaveEmvStatus(void);
int LoadEmvStatus(void);
int SaveEmvErrLog(void);
int LoadErrLog(ushort uiRecNo, void *pOutErrLog);
int SyncPassword(void);
int LastRecordIsFallback(void);
int LoadWLBBit63Flag(void);		//创新银行用

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _FILEOPER_H

// end of file
