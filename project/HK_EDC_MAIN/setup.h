
/****************************************************************************
NAME
    setup.h - 定义终端参数设置、查询模块

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.12      - created
****************************************************************************/

#ifndef _SETUP_H
#define _SETUP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int GetIpLocalSettings(void *pstParam);
int GetRemoteIp(uchar *pszHalfText, uchar bAllowNull, void *pstIPAddr);
int ChkIfValidIp(uchar *pszIP);
int ChkIfValidPort(uchar *pszPort);

int  SetTcpIpParam(void *pstParam);
void SyncTcpIpParam(void *pstDst, void *pstSrc);

int  SetWirelessParam(WIRELESS_PARAM *pstParam);
int  Set3GParam(WIRELESS_PARAM *pstParam);//2015-11-23
void SyncWirelessParam(WIRELESS_PARAM *pstDst, WIRELESS_PARAM *pstSrc);

void GetAllSupportFunc(char *pszBuff);
void FunctionExe(uchar bUseInitMenu, int iFuncNo);
void FunctionMenu(void);
void FunctionInit(void);
void SetSystemParamAll(void);
void SetSysLang(uchar ucSelectMode);
void SetEdcLangExt(char *pszDispName);
static void SetManagerService(void);
int SetReaderLan(void);//2016-3-9 add for set R50 Language
/*----------------2014-5-20 IP encryption----------------*/
#ifdef IP_ENCRYPT
/************************************************************************/
/* defined by jerome                                                    */
/************************************************************************/
extern int WriteKeyIdx(char* pszFileName, uint uiKIN, uchar ucKeyIdx);
extern int FindKeyidx(char* pszFileName, uint uiKIN, uchar* puiKeyIdx);
extern int InitKINFile(char* pszFileName, uint uiKIN, uchar ucKeyIdx);
extern void FileTest(char* psFileName);
extern int KeyInit(char* szTitle);//2015-11-17  Gillian 20160923
/* end of Jerome*/
#endif
/*----------------2014-5-20 IP encryption----------------*/
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _SETUP_H

// end of file
