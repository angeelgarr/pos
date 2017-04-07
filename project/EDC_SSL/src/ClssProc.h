/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2006-? PAX Technology, Inc. All rights reserved.
 * Description: ���彻�״���ģ��
 * Revision History:
 * Date                  Author                 Action
 * 2006.09.12         shengjx               created
 * ============================================================================
 */

#ifndef _TRANCLSS_H
#define _TRANCLSS_H

/************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//CLSS status added by kevinliu 2016/01/26
typedef enum clssLightStatus{
	CLSSLIGHTSTATUS_INIT,
	CLSSLIGHTSTATUS_NOTREADY,
	CLSSLIGHTSTATUS_IDLE,
	CLSSLIGHTSTATUS_READYFORTXN,
	CLSSLIGHTSTATUS_PROCESSING,
	CLSSLIGHTSTATUS_READCARDDONE,
	CLSSLIGHTSTATUS_REMOVECARD,
	CLSSLIGHTSTATUS_COMPLETE,
	CLSSLIGHTSTATUS_DIALING,
	CLSSLIGHTSTATUS_SENDING,
	CLSSLIGHTSTATUS_RECEIVING1,
	CLSSLIGHTSTATUS_RECEIVING2,
	CLSSLIGHTSTATUS_PRINTING,
	CLSSLIGHTSTATUS_ERROR,
} CLSSLIGHTSTATUS;

//PICC LED added by kevinliu 2016/01/26
#define PICC_LED_RED    0x01
#define PICC_LED_GREEN  0x02
#define PICC_LED_YELLOW 0x04
#define PICC_LED_BLUE   0x08
#define PICC_LED_CLSS   0x10
#define PICC_LED_ALL   0xFF

//functions
int TransClssSale(void);
void PiccLightOpen(void);
void PiccLightClose(void);
void SetClssLightStatus(CLSSLIGHTSTATUS eStatus);
CLSSLIGHTSTATUS GetClssLightStatus(void);
void ClearClssLightCtrl(void);
int ClssVirtualLight(gui_callbacktype_t type, void *pvData, int *piDataLen);

int nSetDETData(uchar *pucTag, uchar ucTagLen, uchar *pucData, uchar ucDataLen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _TRANPROC_H

// end of file


