#include <posapi.h>

typedef int (*cTMSStart)(T_INCOMMPARA *ptInPara, int *handle);	    // 3?那??‘㏒?米?米?赤“????㊣迆
typedef int (*cTMSReset)(int handle);	//
typedef int (*cTMSSendPack) (int handle, unsigned char *buff, int SndLen);	// ﹞⊿?赤
typedef int (*cTMSRecvByte) (int handle, unsigned char *ch, int usTimeOut);	// ℅??迆?車那?
typedef int (*cTMSExitClose)(int handle);				            // 赤?3?
typedef int (*cTMSNetRecv)(int socket, void *buf, int size, int flags);	// ?角℅??迆?車那?

extern cTMSStart     pcTMSStart    ;
extern cTMSReset     pcTMSReset    ;
extern cTMSSendPack  pcTMSSendPack ;
extern cTMSRecvByte  pcTMSRecvByte ;
extern cTMSExitClose pcTMSExitClose;
extern cTMSNetRecv   pcTMSNetRecv  ;
