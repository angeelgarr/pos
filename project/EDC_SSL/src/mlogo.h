/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2010-? PAX Technology, Inc. All rights reserved.
 * Description:    defines multiple logos and processing functions
 *
 * Revision History:
 * Date                  Author                 Action
 * 2010.01.26         Gary.Ho                      created
 * ============================================================================
 */

#ifndef _MLOGO_H
#define _MLOGO_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int GetNowDispLogo(uchar **ppsData);
int GetNowPrnLogo(uchar **ppsData);
int GetLogoWidthHeigh(uchar *psData, int *piWidth, int *piHeigh);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _MLOGO_H

// end of file
