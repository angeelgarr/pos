/****************************************************************************
NAME
    clss.h - 定义非接触相关处理

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    Sabrina Fung     2009.10.29      - created
****************************************************************************/

#ifndef _CLSS_H
#define _CLSS_H

#define TIMER_POLL 0 //POLL定时器

#define MAX_RD_SCHEME_NUM		20   //最大Scheme ID个数

#define DOWNRDAPP_FILE		"DOWN_RD_APP"
#define DOWNRDAPP_FLAG		0XDA
#define DOWNRDAPP_LEN		512

#define RID_TYPE_VISA "\xA0\x00\x00\x00\x03"
#define RID_TYPE_MC   "\xA0\x00\x00\x00\x04"
#define RID_TYPE_PBOC "\xA0\x00\x00\x03\x33"
#define RID_TYPE_JCB  "\xA0\x00\x00\x00\x65"
#define RID_TYPE_AE   "\xA0\x00\x00\x00\x25"//2016-2-5


#define EX_PORT_OPEN_ERR	0x50
#define EX_PORT_SEND_ERR	0x51
#define EX_PORT_RECV_ERR	0x52
#define EX_PORT_TIMEOUT		0x53
#define EX_PORT_LEN_ERR		0x54
#define EX_POLL_LRC_ERR		0x55
#define EX_PORT_LRC_ERR		0x56
#define EX_PORT_RETURN_NOK	0x57

#define EX_PAPA_NULL		0x60
#define EX_GET_PARA_ERR		0x61

#define EX_UNDEFINED_ERR	0X70


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	//2014-5-16 enhance
void ClssOpen(void);
void ClssClose(void);
int ClssInit(void);
int ClssProcTxn(uchar *pszAmount, int rtTag, uchar ucTransType); //Modify by Gillian//2016-5-11 AMEX
void ClssCheckTimer(void);
void ProcError_Wave(int nRspCode);
int WaveUpdateApp(uchar *pszAppFileName);
void ClssPoll(void);
void SetClssParam(void);
int InitClssParam(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _CLSS_H

// end of file

