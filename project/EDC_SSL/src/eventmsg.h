/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2006-? PAX Technology, Inc. All rights reserved.
 * Description:     定义管理器事件处理模块
 * Revision History:
 * Date                  Author                 Action
 * 2006.09.12         shengjx               created
 * ============================================================================
 */

#ifndef _EVENTMSG_H
#define _EVENTMSG_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int ProcUserMsg(void);
int AutoDownParam(void);
int GetAutoDownParam(void);
int ProcEcrMsg(void);
int ProcMagMsg(void);
int ProcICCMsg(void);
int ProcFuncKeyMsg(void);
int ProcKeyMsg(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _EVENTMSG_H

// end of file
