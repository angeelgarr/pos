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
 * 2012-11-30          xiaowy                    created
 * ============================================================================
 */

#ifndef __KEYBOARD_H_
#define __KEYBOARD_H_

//#include "osal.h"



#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif

#ifndef ulong
#define ulong unsigned long
#endif

#ifndef ushort
#define ushort unsigned short
#endif

#ifndef DOUBLE
#define DOUBLE	double
#endif



//MidWare Return Value Macro
enum{
	KB_OK 	= 0,
	KB_OPEN_FAIL,
	KB_OPENED,
	KB_CLOSED,
	KB_CLOSE_FAIL,
};






void MidKbInit(void);

uchar kbhit(void);

void kbflush(void);

uchar getkey(void);

void kbmute(uchar flag);

void kblight(uchar mode);















#endif
//End of file
