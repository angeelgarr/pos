/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2006-? PAX Technology, Inc. All rights reserved.
 * Description:     定义所有文件操作的函数
 * Revision History:
 * Date                  Author                 Action
 * 2006.09.12         shengjx               created
 * ============================================================================
 */

#ifndef _FILEOPER_H
#define _FILEOPER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int InitTranLogFile(void);
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
int UpdateTranLog(const void *pstLog, ushort uiIndex);
int SaveTranLog(const void *pstLog);
int RecoverTranLog(void);
ushort GetTranLogNum(uchar ucAcqKey);
uchar AllowDuplicateTran(void);
int GetRecord(int uiStatus, void *pstOutTranLog);
int WriteEPSPara(const void *pPara);
void CalcTotal(uchar ucAcqKey);
int SaveEmvStatus(void);
int LoadEmvStatus(void);
int SaveEmvErrLog(void);
int LoadErrLog(ushort uiRecNo, void *pOutErrLog);
int SyncPassword(void);
int LastRecordIsFallback(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _FILEOPER_H

// end of file
