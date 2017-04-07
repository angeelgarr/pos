/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2006-? PAX Technology, Inc. All rights reserved.
 * Description:    定义管理类交易模块
 *
 * Revision History:
 * Date                  Author                 Action
 * 2006.10.25         shengjx                    created
 * ============================================================================
 */

#ifndef _MANAGE_H
#define _MANAGE_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void UnLockTerminal(void);
int LockTerm(void);
int DoKeyInjection(void);
int ClearAllRecord(void);
int ClearConfig(void);
int ClearPassword(void);
int ClearReversal(void);
void ClearRecord(uchar ucAcqKey);
int DoClear(void);
int ViewTotal(void);
int ViewTranList(void);
int ViewSpecList(void);
int PrnLastTrans(void);
int RePrnSpecTrans(void);
int PrnTotal(void);
int RePrnSettle(void);
int PrintEmvErrLog(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _MANAGE_H

// end of file
