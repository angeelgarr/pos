/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2013-? PAX Technology, Inc. All rights reserved.
 * Description:
 * Revision History:
 * Date                  Author                 Action
 * 2013-10-18        xiaowy                    created
 * ============================================================================
 */

#ifndef _ICCARD_H_
#define _ICCARD_H_

#include "structDefine.h"

#ifdef __cplusplus
extern "C"
{
#endif

////////////////////////////////////////////////////////////////////
// 函数定义

uchar IccInit(uchar slot, uchar *ATR);
uchar IccClose(uchar slot);
uchar IccAutoResp(uchar slot, uchar autoresp);
uchar IccIsoCommand(uchar slot, APDU_SEND *ApduSend, APDU_RESP *ApduResp);
uchar IccDetect(uchar slot);

#ifdef __cplusplus
};
#endif
#endif
