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
 * 2012-12-4          wenjf                    created
 * ============================================================================
 */

#ifndef PORT_H_
#define PORT_H_

#include "structDefine.h"

typedef struct{
	int prolin_ret;
	uchar monitor_ret;
}PORT_RESULT;

/******************通信口******************/
uchar PortOpen(uchar channel, uchar *Attr);
uchar PortClose(uchar channel);
uchar PortSend(uchar channel, uchar ch);
uchar PortRecv(uchar channel, uchar *ch, uint ms);
uchar PortReset(uchar channel);
uchar PortSends(uchar channel, uchar *str, ushort str_len);
uchar PortTxPoolCheck(uchar channel);
//int PortPeep(uchar channel,uchar *buf,ushort len);
int PortRecvs(uchar channel, uchar * pszBuf, ushort usBufLen, ushort usTimeoutMs);
int SetHeartBeat(int SwOn, int MsgLen, const unsigned char *Msg, ushort interval100Ms,int COMn);


#endif /* PORT_H_ */
