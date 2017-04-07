/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2014-? PAX Technology, Inc. All rights reserved.
 * Description:    for  debugging
 * Revision History:
 * Date                  Author                 Action
 * 20141202         Kim.L               created
 * ============================================================================
 */

#ifndef DEBUG_H
#define DEBUG_H

/***************************************************************************************
For Debug use
***************************************************************************************/
#include "global.h"
#ifdef APP_DEBUG

/****************************************************************************
 Function:      Show HEX message on screen.
 Param In:		pszTitle    Title of the message.
				psMsg       Pointer of message to be displayed.
				uiMsgLen    Bytes of the message.
				nTimeOut    Seconds between user interaction.
 Param Out:
				none
 Return Code:
				none
 Description:
****************************************************************************/
void DispHexMsg(const uchar *pszTitle, const uchar *psMsg, uint uiMsgLen, short nTimeOut);

/****************************************************************************
 Function:      print a line as hexadecimal format
 Param In:		pszBuffInOput		data buffer
				uiOffset    offset of the data buffer
				psMsg		Pointer of message to be displayed.
				uiMsgLen    Bytes of the message.
 Param Out:
				pszBuffInOput
 Return Code:
				none
 Description:
****************************************************************************/
int DispHexLine(uchar *pszBuffInOput, uint uiOffset, const uchar *psMsg, uint uiMsgLen);

/****************************************************************************
 Function:      For Debug use, display file name and line
 Param In:		pszFileName		file name
				ulLineNo		line no
 Param Out:
				none
 Return Code:
				none
 Description:
****************************************************************************/
void DispAssert(const uchar *pszFileName, ulong ulLineNo);

    // debug macro for boolean expression
#define _POS_DEBUG_WAIT		15
#define PubASSERT(expr) if( !(expr) ){DispAssert((uchar *)__FILE__, (ulong)(__LINE__));}

// print string debug information

#define PubTRACE0(sz) {OsLog(LOG_DEBUG, sz);}
#define PubTRACE1(sz, p1) {OsLog(LOG_DEBUG, sz, p1);}
#define PubTRACE2(sz, p1, p2) {OsLog(LOG_DEBUG, sz, p1, p2);}
#define PubTRACE3(sz, p1, p2, p3) {OsLog(LOG_DEBUG, sz, p1, p2, p3);}
#define PubTRACEHEX(t, s, l)    DispHexMsg((t), (s), (l), _POS_DEBUG_WAIT)

#else /* APP_DEBUG */

    #define PubASSERT(expr)
    #define PubTRACE0(sz)
    #define PubTRACE1(sz, p1)
    #define PubTRACE2(sz, p1, p2)
    #define PubTRACE3(sz, p1, p2, p3)
    #define PubTRACEHEX(t, s, l)

#endif /* APP_DEBUG */

#endif
