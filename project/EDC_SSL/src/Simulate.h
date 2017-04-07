/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2009-? PAX Technology, Inc. All rights reserved.
 * Description:    定义模拟、debug或培训模式所使用的功能
 * Revision History:
 * Date                  Author                 Action
 * 2009.05.07         Gary Ho                      created
 * ============================================================================
 */

#ifndef SIMULATE_H
#define SIMULATE_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

uchar ChkIfTrainMode(void);
int CreatDummyRecvData(void *pstProcInfo, void *pstRecvPacket);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

// end of file
