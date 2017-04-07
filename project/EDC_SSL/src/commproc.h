/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2006-? PAX Technology, Inc. All rights reserved.
 * Description:     实现通讯控制
 * Revision History:
 * Date                  Author                 Action
 * 2006.09.14         shengjx               created
 * ============================================================================
 */

#ifndef _COMMPROC_H
#define _COMMPROC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int TranProcess(void);
int SendRecvPacket(void);
int SendPacket(void);
int RecvPacket(void);
int PreDial(void);
int ConnectHost(void);
int AfterTranProc(void);
int ReferralDial(const uchar *pszPhoneNo);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _COMMPROC_H

// end of file
