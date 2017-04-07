#ifndef PROTIMS_COMM_H_
#define PROTIMS_COMM_H_

#include "sxxcomm.h"

int  KmsHandshake(int handle);
int  KmsEnd(int handle);
int KmsGetRSAKey(int handle,unsigned char *psTermID,unsigned char *RcvBuff, int *RecvLen);
int KmsGetMainKey(int handle,unsigned char *Buff,unsigned char *RcvBuff, int *RecvLen);
int KmsWriteMainKey(unsigned char *Buff,int BufLen);
int KmsCalculateKeyNum(unsigned char *InBuf,int BufLen);
int KmsExchangeKeyState(int handle);
#endif






