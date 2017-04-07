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
 * 2012-11-30          mahui                    created
 * ============================================================================
 */

#ifndef _MANAGER_H_
#define _MANAGER_H_

#include "structDefine.h"

// rename it cuz it will conflict with ProtimsE lib
int MIDReadAppInfo(unsigned char AppNo, APPINFO* ai);
int RunAppByName(const char *appnum);
int RunApp(unsigned char  appnum);	
int DoEvent(unsigned char AppNo, ST_EVENT_MSG *msg);
int OS_MultiThread(void *vThread);

#endif 
