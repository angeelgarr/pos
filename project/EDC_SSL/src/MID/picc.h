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

#ifndef PICC_H_
#define PICC_H_

#include "structDefine.h"

uchar PiccOpen(void);
uchar PiccSetup(uchar mode, PICC_PARA *picc_para);	
uchar PiccDetect(uchar Mode,uchar *CardType,uchar *SerialInfo,uchar *CID,uchar *Other);
uchar PiccIsoCommand(uchar cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv);
uchar PiccRemove(uchar mode, uchar cid);
uchar PiccClose(void);

uchar M1Authority(uchar Type, uchar BlkNo, uchar *Pwd, uchar *SerialNo);
uchar M1ReadBlock(uchar BlkNo, uchar *BlkValue);
uchar M1WriteBlock (uchar BlkNo, uchar *BlkValue);
uchar M1Operate(uchar Type, uchar BlkNo, uchar *Value, uchar UpdateBlkNo);
uchar PiccInitFelica(uchar ucRate, uchar ucPol);
uchar PiccCmdExchange(int uiSendLen, uchar* paucInData, int* puiRecLen, uchar* paucOutData);


#endif
