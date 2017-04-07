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
 * 2012-12-4          wenjf                     created
 * ============================================================================
 */

#ifndef WIRELESS_H_
#define WIRELESS_H_

//#include "osal.h"
#include "structDefine.h"

#define WIRELESS_PORT_AMA "/dev/ttyAMA1"
#define WIRELESS_PORT_MUX "/dev/mux1"
//================================================
//           Error code for (GPRS & CDMA) API
//================================================
#define WNET_RET_OK			0
// 模块口被占用
#define ERR_WNET_PORTINUSE 	-1
// 模块没有应答
#define ERR_WNET_NORSP		-2
// 模块应答错误
#define ERR_WNET_RSPERR		-3
// 模块串口没有打开
#define ERR_WNET_PORTNOOPEN -4
// 需要PIN码
#define ERR_WNET_NEEDPIN	-5
// 需要PUK解PIN码
#define ERR_WNET_NEEDPUK	-6
// 参数错误
#define ERR_WNET_PARAMER	-7
// 密码错误
#define ERR_WNET_ERRPIN		-8
// 没有插入SIM卡
#define	ERR_WNET_NOSIM      -9
// 不支持的类型
#define ERR_WNET_NOTYPE		-10
// 网络没有注册
#define ERR_WNET_NOREG		-11
// 模块曾初始化
#define ERR_WNET_INIT_ONCE	-12
// 没有连接
#define ERR_WNET_NOCONNECT  -13
// 线路断开
#define ERR_WNET_LINEOFF	-14
// 没有socket可用
#define ERR_WNET_NOSOCKETUSE	-15
// 超时
#define ERR_WNET_TIMEOUT		-16
//正在拨号中
#define ERR_WNET_CALLING 		-17
//重复的socket请求
#define ERR_WNET_REPEAT_SOCKET	-18
//socket 已经断开
#define ERR_WNET_SOCKET_DROP	-19
//socket 正在连接中
#define ERR_WNET_CONNECTING     -20
//socket 正在关闭
#define ERR_WNET_SOCKET_CLOSING	-21
//网络注册中
#define ERR_WNET_REGING			-22
//关闭串口出错
#define ERR_WNET_PORTCLOSE  	-23

typedef struct{
	int prolin_ret;
	int monitor_ret;
}WIRELESS_RESULT;

typedef enum __E_WIRELESS_TYPE
{
	WIRELESS_BEGIN = 0,
	G610,
	MG323,
	MU509,
	WIRELESS_UNKNOWN
}E_WIRELESS_TYPE;

#define IMSI_NUM_LEN                20
#define CCID_NUM_LEN                25
#define MAX_CELL_NUM                7

typedef struct WlCellInfo
{
    char mcc[3+1];//mobile country code
    char mnc[3+1];//moblie network code
    char lac[5+1];//location code
    char cell[5+1];//cell code
    char bsic[5+1];//base station identifier code
    char chann[5+1];//absolute Frequency channel number
    char rxlev[5+1];//receive level
    char reserver[32]; //resever for future use.
}WlCellInfo_T;

typedef struct WlInfo
{
    char imsi[IMSI_NUM_LEN+1];//the imsi number of sim card
    char ccid[CCID_NUM_LEN+1];//the ccid of sim card
    char isp_name[32];//CHINA MOBLE, etc
    char module_name[64]; //MG323,etc
    char manufacture[64]; //HUAWEI,etc
    char revision[64]; //11.810.01.00.00 ,etc
    char rssi;//31, etc
    char reserver[256]; //resever for future use.
    int net_status;// 1-5
    WlCellInfo_T cell_info[MAX_CELL_NUM]; //cell_info[0], main cell information. cell_info[1]-cell_info[6], neighbor cell information.
}WlInfo_T;


//----------------------------------------------------------
// nicm add for new wireless module begin
//----------------------------------------------------------

#define WL_RET_ERR_PORTINUSE -201 // 模块口被占用
#define WL_RET_ERR_NORSP -202 // 模块没有应答
#define WL_RET_ERR_RSPERR -203 // 模块应答错误
#define WL_RET_ERR_PORTNOOPEN -204 // 模块串口没有打开
#define WL_RET_ERR_NEEDPIN -205	// 需要PIN码
#define WL_RET_ERR_NEEDPUK -206	// 需要PUK解PIN码
#define WL_RET_ERR_PARAMER -207	// 参数错误
#define WL_RET_ERR_ERRPIN -208 // 密码错误
#define WL_RET_ERR_NOSIM -209 // 没有插入SIM卡
#define WL_RET_ERR_NOTYPE -210 // 不支持的类型
#define WL_RET_ERR_NOREG -211 // 网络没有注册
#define WL_RET_ERR_INIT_ONCE -212 // 模块已初始化
#define WL_RET_ERR_LINEOFF -214	// 连接断开
#define WL_RET_ERR_TIMEOUT -216	// 超时
#define WL_RET_ERR_REGING -222 // 网络注册中
#define WL_RET_ERR_PORTCLOSE -223 // 关闭串口出错
#define WL_RET_ERR_MODEVER -225	// 错误的模块版本
#define WL_RET_ERR_DIALING -226	// 拨号中
#define WL_RET_ERR_ONHOOK -227 // 关机中
#define WL_RET_ERR_PPP_BRK -228	// 发现PPP断开
#define WL_RET_ERR_NOSIG -229 // 网络无信号
#define WL_RET_ERR_POWEROFF -230 // 模块已下电
#define WL_RET_ERR_BUSY -231 // 模块忙
#define WL_RET_ERR_OTHER -300 // 其他未知错误

/******************无线模块******************/

int WlInit(const uchar *SimPinIn);
int WlGetSignal(uchar * SingnalLevelOut);
int WlPppLogin(uchar *APNIn, uchar * UidIn, uchar * PwdIn, long Auth, int TimeOut, int AliveInterval );
void WlPppLogout (void);
int WlPppCheck(void);
int WlOpenPort (void );
int WlClosePort (void );
int WlSendCmd(const uchar * ATstrIn, uchar *RspOut, ushort Rsplen, ushort TimeOut, ushort Mode);
void WlSwitchPower (uchar Onoff );
int WlSelSim (uchar simno);
int WlAutoStart(unsigned char * pin, unsigned char * APNIn, unsigned char * UidIn, unsigned char * PwdIn, long Auth, int TimeOut, int AliveInterval);
int WlAutoCheck(void);


#endif /* WIRELESS_H_ */
