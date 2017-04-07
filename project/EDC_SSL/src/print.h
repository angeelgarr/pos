/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2006-? PAX Technology, Inc. All rights reserved.
 * Description:    定义打印模块
 * Revision History:
 * Date                  Author                 Action
 * 2006.09.12        shengjx                      created
 * ============================================================================
 */

#ifndef _PRINT_H
#define _PRINT_H

#define PRN_NORMAL		0
#define PRN_REPRINT		1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int PrnCurAcqTransList(void);
int PrnAllList(void);
void  PrnSetSmall(void);
void  PrnSetNormal(void);
void  PrnSetBig(void);
int  PrintReceipt(uchar ucPrnFlag);
int  PrnTotalIssuer(uchar ucIssuerKey);
int  PrnTotalAcq(void);
int  PrintSettle(uchar ucPrnFlag);
int  PrintParam(void);
int  StartPrinter(void);
void PrintEmvErrLogSub(void);

int PrnInit();
void MultiLngPrnStr(uchar alignment, uchar size, const char *str, ...);
#define PrnStep OsPrnFeed
#define PrnLogo OsPrnPutImage
#define PrnLeftIndent(x) OsPrnSetIndent(x, 0)
#define PrnStart StartPrinter

#ifdef __cplusplus
}
#endif /* __cplusplus */ 

#endif	// _PRINT_H

// end of file
