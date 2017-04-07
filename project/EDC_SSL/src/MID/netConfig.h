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
 * 2012-11-28          mahui                    created
 * ============================================================================
 */

#include "cbinder.h"

#ifndef NETCONFIG_H_
#define NETCONFIG_H_

#define NET_OK  		0 	// 无错误,正常
#define NET_ERR_MEM 	-1 	// 内存不够
#define NET_ERR_BUF 	-2  // 缓冲区错误
#define NET_ERR_ABRT 	-3 	// 试图建立连接失败
#define NET_ERR_RST 	-4 	// 连接被对方复位(收到对方的Reset)
#define NET_ERR_CLSD 	-5 	// 连接已关闭
#define NET_ERR_CONN 	-6 	// 连接未成功建立
#define NET_ERR_VAL 	-7 	// 错误变量
#define NET_ERR_ARG 	-8 	// 错误参数
#define NET_ERR_RTE 	-9 	// 错误路由(route)
#define NET_ERR_USE 	-10 // 地址、端口使用中
#define NET_ERR_IF 		-11 // 底层硬件错误
#define NET_ERR_ISCONN 	-12 // 连接已建立
#define NET_ERR_TIMEOUT -13 // 超时
#define NET_ERR_AGAIN 	-14 // 请求资源不存在,请重试
#define NET_ERR_EXIST 	-15 // 已存在
#define NET_ERR_SYS 	-16 // 系统不支持
#define NET_ERR_PASSWD 	-17 // 错误密码
#define NET_ERR_MODEM 	-18 // 拨号失败
#define NET_ERR_LINKDOWN -19 // 数据链路已断开,请重新拨号
#define NET_ERR_LOGOUT 	-20 // Logout
#define NET_ERR_PPP 	-21 // PPP断开
#define NET_ERR_STR 	-22 // 字符串太长
#define NET_ERR_DNS 	-23 // 域名解析错误
#define NET_ERR_INIT 	-24 // 相应的功能系统没有初始化
#define NET_ERR_SERV 	-30 // 没有找到PPPoE服务器


#ifndef BID_LINKSTAT

#define BID_LINKSTAT 15

#endif

int DhcpStart (void);

void DhcpStop (void);

int DhcpCheck (void);

int PppoeCheck(void);

void PppoeLogout(void);

void EthSetRateDuplexMode(int mode);

int PppoeLogin(char *name, char *passwd, int timeout);

int EthSet(char *host_ip, char *host_mask,char *gw_ip,char *dns_ip);

int NetAddStaticArp(char *ip_str, unsigned char mac[6]);

int EthGet(char *host_ip,char *host_mask,char *gw_ip, char *dns_ip,long *state);

int EthGetFirstRouteMac (const char *dest_ip, unsigned char *mac, int len);


#endif /* NETCONFIG_H_ */
