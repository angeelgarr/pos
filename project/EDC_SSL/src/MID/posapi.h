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
 * 2013-1-4          xiaowy                    created
 * ============================================================================
 */


#ifndef POSAPI_H_
#define POSAPI_H_

#include <assert.h>
//#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/mount.h>
#include <time.h>
#include <linux/limits.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <osal.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <xui.h>
#include <termios.h>

#include "MID/structDefine.h"	// struct define
#include "MID/crypt.h"
#include "MID/pub.h"	// PUB
#include "MID/file.h"	//file
#include "MID/log.h"	// LOG




#include "MID/mag.h"		// magic card
#include "MID/ICcard.h"		// IC card
#include "MID/picc.h"		// PICC

#include "MID/port.h"		// RS 232 Port
#include "MID/wireless.h"	// wireless module
#include "MID/modem.h"		// Modem
#include "MID/netConfig.h"	// TCP/IP net config
#include "MID/netPort.h"	// net port


#include "MID/ped.h"		// PED
#include "MID/keyboard.h"
//#include "MID/printf.h"		//

#include "MID/sys.h"		// system module

#include "MID/manager.h"	// system manager

#define MAGCARD_MSG   0x01
#define ICCARD_MSG    0x02
#define KEYBOARD_MSG  0x03
#define USER_MSG      0x04

/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// file.h

//===========================================
//        Defined for file system
//============================================














#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif



//==================================================
//		MACROS for LCD functions
//===================================================
#define ASCII		0x00
#define CFONT		0x01
#define REVER		0x80


#define ICON_PHONE	   1	// phone 
#define ICON_SIGNAL	   2	// wireless signal 
#define ICON_PRINTER   3	// printer 
#define ICON_ICCARD	   4	// smart card IC
#define ICON_LOCK      5	// lock 
#define ICON_SPEAKER   6	// speeker 
#define ICON_BATTERY   6	// 
#define ICON_UP        7	// up 
#define ICON_DOWN      8	// down 

#define CLOSEICON	   0	// 
#define OPENICON       1   // 
#define OPENICON_PRINTER "./res/printer.png"
#define CLOSEICON_PRINTER NULL
#define OPENICON_LOCK "./res/lock.png"
#define CLOSEICON_LOCK NULL
#define OPENICON_UP				"./res/up.png"
#define OPENICON_DOWN			"./res/down.png"
#define OPENICON_UP_DOWN		"./res/up_down.png"

#ifndef open		// 20120106
#define open MIDopen
#endif

#ifndef truncate
#define truncate MIDtruncate
#endif

#ifndef read
#define read MIDread
#endif

#ifndef write
#define write MIDwrite
#endif

#ifndef close
#define close MIDclose
#endif

#ifndef remove
#define remove MIDremove
#endif



#ifdef KEY1
#undef KEY1
#endif
#define KEY1 XUI_KEY1

#ifdef KEY2
#undef KEY2
#endif
#define KEY2 XUI_KEY2

#ifdef KEY3
#undef KEY3
#endif
#define KEY3 XUI_KEY3

#ifdef KEY4
#undef KEY4
#endif
#define KEY4 XUI_KEY4

#ifdef KEY5
#undef KEY5
#endif
#define KEY5 XUI_KEY5

#ifdef KEY6
#undef KEY6
#endif
#define KEY6 XUI_KEY6

#ifdef KEY7
#undef KEY7
#endif
#define KEY7 XUI_KEY7

#ifdef KEY8
#undef KEY8
#endif
#define KEY8 XUI_KEY8

#ifdef KEY9
#undef KEY9
#endif
#define KEY9 XUI_KEY9

#ifdef KEY0
#undef KEY0
#endif
#define KEY0 XUI_KEY0

#ifdef KEYUP
#undef KEYUP
#endif
#define KEYUP XUI_KEYUP

#ifdef KEYDOWN
#undef KEYDOWN
#endif
#define KEYDOWN XUI_KEYDOWN

#ifdef KEYFN
#undef KEYFN
#endif
#define KEYFN XUI_KEYFUNC

#ifdef KEYMENU
#undef KEYMENU
#endif
#define KEYMENU XUI_KEYMENU

#ifdef KEYALPHA
#undef KEYALPHA
#endif
#define KEYALPHA XUI_KEYALPHA

#ifdef KEYCANCEL
#undef KEYCANCEL
#endif
#define KEYCANCEL XUI_KEYCANCEL

#ifdef KEYCLEAR
#undef KEYCLEAR
#endif
#define KEYCLEAR XUI_KEYCLEAR

#ifdef KEYENTER
#undef KEYENTER
#endif
#define KEYENTER XUI_KEYENTER

#ifdef KEYF1
#undef KEYF1
#endif
#define KEYF1 XUI_KEYF1

#ifdef KEYF2
#undef KEYF2
#endif
#define KEYF2 XUI_KEYF2


//#define strcpy(a, b)				strcpy((char *)(a), (const char *)(b))
//#define strcmp(a, b)				strcmp((const char *)(a), (const char *)(b))
//#define sprintf(a, b, ...)			sprintf((char *)(a), (const char *)(b), ##__VA_ARGS__)
//#define atol(a)						atol((const char *)(a))
//#define atoi(a)						atoi((const char *)(a))
//#define strlen(a)					strlen((const char *)(a))
//#define strcat(a, b)				strcat((char *)(a), (const char *)(b))
//#define Lcdprintf(a, ...)			Lcdprintf((unsigned char *)(a), ##__VA_ARGS__)

#define IPLEN 4
#define KEY_WEP_LEN_MAX 16
#define KEY_WEP_LEN_64 5
#define KEY_WEP_LEN_128 13
#define KEY_WEP_LEN_152 16
#define KEY_WPA_MAXLEN 63
#define SSID_MAXLEN 32
#define SCAN_AP_MAX 15
#define WLAN_SEC_UNSEC 0
#define WLAN_SEC_WEP 1
#define WLAN_SEC_WPA_WPA2 2
#define WLAN_SEC_WPAPSK_WPA2PSK 3
#define WIFI_ROUTE_NUM 12


typedef struct {
	uchar Key[4][KEY_WEP_LEN_MAX];
	int KeyLen; int Idx;
} ST_WEP_KEY;

typedef ST_WifiApSet ST_WIFI_AP;

typedef struct {
	char DhcpEnable;
	uchar Ip[IPLEN]; 	// 静态IP
	uchar Mask[IPLEN]; 	// 掩码 
	uchar Gate[IPLEN]; 	// 网关 
	uchar Dns[IPLEN]; 	// DNS 
	ST_WifiApInfo stInfo;
}ST_WIFI_PARAM;

#define WifiOpen OsWifiOpen
#define WifiDisconnect OsWifiDisconnect







#endif /* POSAPI2_H_ */
