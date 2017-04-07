
#ifndef PROTIMS_SXX_COMM_H_
#define PROTIMS_SXX_COMM_H_
#include "global.h"
#ifdef _SXX_PROTIMS

int CommConnect(T_INCOMMPARA *ptInPara, int* handle);
int CommReset(int handle);
int CommSendPack(int handle, unsigned char *buff, int SndLen);
int CommRecvByte(int handle, unsigned char *ch, int usTimeOut);
int CommExitClose(int handle);
int CommRecvPack(int socket, void *buf, int size, int flags);

int s_SetErrorCode(int Err1, int Err2);
#endif
#endif
