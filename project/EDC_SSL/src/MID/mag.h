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

#ifndef _MAG_H_H
#define _MAG_H_H

#include "structDefine.h"

int MagOpen(void);
void MagClose(void);
void MagReset(void);
uchar MagSwiped(void);
uchar MagRead(uchar *Track1, uchar *Track2, uchar *Track3);

#endif
