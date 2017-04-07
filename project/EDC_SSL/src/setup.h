/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2006-? PAX Technology, Inc. All rights reserved.
 * Description:    定义终端参数设置、查询模块
 * Revision History:
 * Date                  Author                 Action
 * 2006.09.12          shengjx                      created
 * ============================================================================
 */

#ifndef _SETUP_H
#define _SETUP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int GetIpLocalSettings(void *pstParam);
int GetRemoteIp(const uchar *pszHalfText, uchar bAllowHostName, uchar bAllowNull, void *pstIPAddr);
int ChkIfValidIp(const uchar *pszIP);
int ChkIfValidPort(const uchar *pszPort);

int  SetTcpIpParam(void *pstParam);
void SyncTcpIpParam(void *pstDst, const void *pstSrc);

int  SetWirelessParam(WIRELESS_PARAM *pstParam);
void SyncWirelessParam(WIRELESS_PARAM *pstDst, const WIRELESS_PARAM *pstSrc);

void GetAllSupportFunc(char *pszBuff);
void FunctionExe(uchar bUseInitMenu, int iFuncNo);
void FunctionMenu(void);
void FunctionInit(void);
void SetSystemParamAll(void);
void SetSysLang(uchar ucSelectMode);
void SetEdcLangExt(const char *pszDispName);

int SetRs232Param(RS232_PARA *rs232);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _SETUP_H

// end of file
