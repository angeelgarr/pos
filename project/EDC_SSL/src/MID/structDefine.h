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
 * 2012-11-29          xiaowy                     created
 * ============================================================================
 */

#ifndef _STRUCT_DEFINE_H_
#define _STRUCT_DEFINE_H_

#include "osal.h"

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
#define ushort unsigned short int
#endif

#ifndef NULL
#define NULL (void*)0
#endif

#ifndef DOUBLE
#define DOUBLE	double
#endif

#ifndef OK
#define OK 			                    		0		//OK
#endif

#ifndef RET_OK
#define RET_OK 			                    	0		//OK
#endif



#define BID_ADD 1
#define BID_STRUPR 2
#define BID_MEMCPY 3
#define BID_SLEEP 4
#define BID_WRITE 5

#define BID_SETTIME	6
#define BID_REBOOT	7
#define BID_POWEROFF	8
#define BID_FILETOAPP	9
#define BID_FILETOPARAM	10
#define BID_FILETOFONT	11
#define BID_FILETOMON	12
#define BID_FILETOPUK	13
#define BID_DELAPPFILE	14



#ifndef ST_FONT
typedef struct{
	int CharSet;    // 字符集
	int Width;      // 字体宽度
	int Height;     // 字体高度
	int Bold;       //（1：黑体， 0：正常）
	int Italic;     //（1：斜体， 0：正常）
} ST_FONT;
#endif


//===========================================================
//           structure for smart card operation
//===========================================================
#define _APDU_DEFINE	// 避免和emv库中的定义冲突


typedef struct{
   	unsigned char Command[4];
   	unsigned short Lc;
   	unsigned char  DataIn[512];
   	unsigned short Le;
}APDU_SEND;

typedef struct{
	unsigned short LenOut;
   	unsigned char  DataOut[512];
   	unsigned char  SWA;
   	unsigned char  SWB;
}APDU_RESP;


//typedef struct
//{
//unsigned int  modlen;          	// 加密公钥模数长 
//unsigned char mod[256];        	// 加密公钥模数,高字节在前,低字节在后,不足位前补0 */
//unsigned char exp[4];           	// 加密公钥指数,高字节在前,低字节在后,不足位前补0 
//unsigned char iccrandomlen;     	// 从卡片取得的随机数长 
//unsigned char iccrandom[8];     	// 从卡片取得的随机数 
//}RSA_PINKEY;



// RSA
#define MAX_RSA_MODULUS_BITS 2048

#define MAX_RSA_MODULUS_LEN ((MAX_RSA_MODULUS_BITS + 7) / 8)
#define MAX_RSA_PRIME_BITS ((MAX_RSA_MODULUS_BITS + 1) / 2)
#define MAX_RSA_PRIME_LEN ((MAX_RSA_PRIME_BITS + 7) / 8)

typedef struct {
    unsigned short int bits;                   		// length in bits of modulus
    unsigned char modulus[MAX_RSA_MODULUS_LEN];     // modulus
    unsigned char exponent[MAX_RSA_MODULUS_LEN];    // public exponent 
} R_RSA_PUBLIC_KEY;

typedef struct {
    unsigned short int bits;                    		// length in bits of modulus 
    unsigned char modulus[MAX_RSA_MODULUS_LEN];     	// modulus 
    unsigned char publicExponent[MAX_RSA_MODULUS_LEN];  // public exponent 
    unsigned char exponent[MAX_RSA_MODULUS_LEN];        // private exponent 
    unsigned char prime[2][MAX_RSA_PRIME_LEN];     		// prime factors 
    unsigned char primeExponent[2][MAX_RSA_PRIME_LEN];  // exponents for CRT 
    unsigned char coefficient[MAX_RSA_PRIME_LEN];  		// CRT coefficient 
} R_RSA_PRIVATE_KEY;


typedef struct{
	unsigned char fid;		// 文件标识号
	unsigned char attr;		// 文件的属主
	unsigned char type;		// 文件类型
	char name[17];			// 文件名
	unsigned long length;	// 文件长度
} FILE_INFO;



typedef struct{
    uchar ucSrcKeyType;
    uchar ucDstKeyType;
    uchar ucSrcKeyIdx;
    uchar ucDstKeyIdx;
    int   iDstKeyLen;
    uchar aucDstKeyValue[24];
}ST_KEY_INFO;

typedef struct{
    int iCheckMode;
    uchar aucCheckBuf[128];
}ST_KCV_INFO;



typedef struct
{
	unsigned char AppName[32];
	unsigned char AID[16];
	unsigned char AppVer[16];
	unsigned char AppProvider[32];
	unsigned char Descript[64];
	unsigned char DownloadTime[14];
	unsigned long MainAddress;
	unsigned long EventAddress;
	unsigned char AppNum;
	unsigned char RFU[73];
}APPINFO;

typedef struct
{
	unsigned char RetCode; 		// 应用的处理结果
	unsigned char track1[256]; 	// 磁道1信息缓冲区
	unsigned char track2[256]; 	// 磁道2信息缓冲区
	unsigned char track3[256]; 	// 磁道3信息缓冲区
}ST_MAGCARD;

typedef struct
{
	int MsgType;			// 事件类型（包括磁卡,按键,IC卡及自定义事件）
	ST_MAGCARD MagMsg;		// 磁卡数据 
	unsigned char KeyValue;	// 键盘的按键值 
	unsigned char IccSlot;	// 接触式IC卡通道号
	void * UserMsg;			// 用户自定义的事件信息 
}ST_EVENT_MSG;


typedef struct
{
   long start;
   long stop;
   long timeleft;
}TTIMER;

/*
typedef struct{
    int CharSet;
    int Width;
    int Height;
    int Bold;
    int Italic;
}ST_FONT;
*/

typedef struct _pic_data pic_data;
struct _pic_data
{
	int width, height;		// 尺寸 
	int bit_depth;			// 位深 
	int flag; 				// 一个标志，表示是否有alpha通道
	unsigned char *rgba; 	// 图片数组 
};

//=================================================
//			  for RF card functions
//=================================================
typedef struct
{
	unsigned char  drv_ver[5];  	// e.g. "1.01A", read only
	unsigned char drv_date[12]; 	// e.g. "2006.08.25",read only

	unsigned char a_conduct_w;  	// Type A conduct write enable: 1--enable,else disable
	unsigned char a_conduct_val;	// Type A output conduct value,0~63

	unsigned char m_conduct_w;  	// M1 conduct write enable: 1--enable,else disable
	unsigned char m_conduct_val;	// M1 output conduct value,0~63

	unsigned char b_modulate_w;  // Type B modulate index write enable,1--enable,else disable
	unsigned char b_modulate_val;// Type B modulate index value

	uchar card_buffer_w;		// added in V1.00C,20061204
	ushort card_buffer_val;		// max_frame_size of PICC

	uchar wait_retry_limit_w;	// added in V1.00F,20071212
	ushort wait_retry_limit_val;// max retry count for WTX block requests,default 3
	// 20080617
	uchar card_type_check_w; 	// 卡片类型检查写入允许：1--允许，其它值--不允许，主要用于避免因卡片不规范引起的问题

	uchar card_type_check_val; 	// 0-检查卡片类型，其他－不检查卡片类型(默认为检查卡片类型)

	//uchar reserved[92];  // 保留字节，用于将来扩展；写入时应全清零

	//2009-10-30
	uchar card_RxThreshold_w; 	// 卡片类型检查写入允许：1--允许，其它值--不允许，主要用于避免因卡片不规范引起的问题

	uchar card_RxThreshold_val;	// 卡片接收灵敏度

	//2009-11-20

     uchar f_modulate_w; 	// felica调制深度写入允许

     uchar f_modulate_val;	// felica调制深度

	 // add by wls 2011.05.17
	 unsigned char a_modulate_w; // A卡调制指数写入允许：1--允许，其它值—不允许
	 unsigned char a_modulate_val; // A卡调制指数控制变量，有效范围0~63,超出时视为63

		//add by wls 2011.05.17
	 unsigned char a_card_RxThreshold_w; 	// 接收灵敏度检查写入允许：1--允许，其它值--不允许
	 unsigned char a_card_RxThreshold_val;	// A卡接收灵敏度

	 // add by liubo 2011.10.25, 针对A,B和C卡的天线增益
	 unsigned char a_card_antenna_gain_w;
	 unsigned char a_card_antenna_gain_val;

	 unsigned char b_card_antenna_gain_w;
	 unsigned char b_card_antenna_gain_val;

	 unsigned char f_card_antenna_gain_w;
	 unsigned char f_card_antenna_gain_val;

	 //added by liubo 2011.10.25，针对Felica的接收灵敏度
	 unsigned char f_card_RxThreshold_w;
	 unsigned char f_card_RxThreshold_val;

	 uchar f_conduct_w; 	// Felica的电导率
	 uchar f_conduct_val;

	 unsigned char reserved[76];  // 保留字节，用于将来扩展；写入时应全清零


}PICC_PARA;

//=============================================
//     Asynchronism communication functions
//=============================================
#define COM1        	0
//#define COM2          2     // P90
#define COM2        	1
#define RS485       	2
#define PINPAD      	3
#define IC_COMMPORT		4

#define BASECOM1    	0
#define BASECOM2    	1
#define BASERS485   	2
#define BASEPINPAD  	3
#define HANDSETCOM1 	4

#define RS232A			0
#define RS232B			1
#define LANPORT 		2
#define MODEM			4
#define WNETPORT        1

//#define	O_RDWR		0x01	// 02
#define O_CREATE	O_CREAT
//#define	O_CREATE	0x02

//#define O_CREAT		   	0100	// not fcntl 
//#define O_EXCL		   	0200	// not fcntl 
//#define O_NOCTTY	   		0400	// not fcntl 
//#define O_TRUNC		  	01000	// not fcntl 
#define	O_ENCRYPT   0x04

#define ENCRYPT 1
#define DECRYPT 0

#define NOKEY               0xff

//#define KEYDOWN	KEYF1		// 使用func key 1 作为向下键
//#define KEYUP	KEYF2			// 使用func key 2 作为向上键
//#define KEYMENU	KEYF3		// 使用func key 3 作为菜单键
//#define KEYFN	KEYF4			// 使用func key 4 作为功能键w
//#define KEY00	KEY0

#endif

