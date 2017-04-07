/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2012-? PAX Technology, Inc. All rights reserved.
 * Description:
 * Revision History:
 * Date                  Author                 Action
 * 2012-11-30        xiaowy                    created
 * ============================================================================
 */
 
 
#ifndef SHARED_LIBRARY_H_LOG
#define SHARED_LIBRARY_H_LOG

#define T_LOG	3	// TRACE   LOG,流水日志
#define W_LOG	2	// WARNING LOG,警告日志
#define E_LOG	1	// ERROR   LOG,错误日志


#define LOG_OK					0
#define LOG_ERR_BASE 			10900
enum{
	LOG_ERR_INVALID_PARA	= LOG_ERR_BASE,
	LOG_ERR_OPEN_FILE,
	LOG_ERR_READ_FILE,
	LOG_ERR_WRITE_FILE,
	LOG_ERR_LEVEL_LOW,
	LOG_ERR_FILE_NOEXIST,
	LOG_ERR_TOO_LONG,
	LOG_ERR_NO_LOGFILE,
	LOG_ERR_REMOVE_FILE,
	LOG_ERR_MAKE_DIR,
	LOG_ERR_SYSTEM,
};



#ifdef __cplusplus
extern "C"
{
#endif


// 日志初始化
int LogInit(char cLogLevel, char cLogSaveDate, int iLogMaxLen);

int LogWrite(char cLogLevel, const char *pszFileName, int iLineNo, const char *pszParaInfo, ...);
int LogWriteHex(char cLogLevel, const char *pszFileName, int iLineNo, const char *psParaInfo, int iParaLen);
int LogQueAll(char * pszLogFileOut);
int LogDelOne(const char *pszDate);
int LogDelAll(void);
const char* LogGetLastErrMsg(void);
const char* LogGetVer(void);

#define _LOG(cLogLevel, pszParaInfo, ...) LogWrite((char)cLogLevel, __FILE__, __LINE__, (const char *)pszParaInfo, ##__VA_ARGS__)
#define _LOGHEX(cLogLevel, psParaInfo, iParaLen) LogWriteHex((char)(cLogLevel), __FILE__, __LINE__, (const char *)psParaInfo, (int)(iParaLen))
#ifdef __cplusplus
};
#endif

#endif /* SHARED_LIBRARY_H_LOG */

