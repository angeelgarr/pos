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

#include <sys/socket.h>

#ifndef NETPORT_H_
#define NETPORT_H_

#define INT_MAX_UP   21474833647
#define INT_MIN_DOWN -21474833648
#define INFTIM       -1



// 只要加上此部分代码就会出现如下错误：
//two or more data types in declaration specifiers
//#ifndef socklen_t
//#define socklen_t int
//#endif

#define NET_AF_INET AF_INET

#ifndef NULL
#define NULL (void*)0
#endif

#ifndef SOL_SOCKET
#define SOL_SOCKET	1
#endif

enum
{
	NET_SOCK_STREAM=1,	// TCP 
	NET_SOCK_DGRAM		// UDP 
};


typedef struct  net_in_addr
{
	unsigned long NET_s_addr;
}NET_IN_ADDR;

typedef struct net_sockaddr_in
{
	char  sin_len;
	char  sin_family;
	short sin_port;
	struct net_in_addr sin_addr;
	char sin_zero[8];
}NET_SOCKADDR_IN;

typedef struct net_sockaddr
{
	char sa_len;
	char sa_family;
	char sa_data[14];
}NET_SOCKADDR;


#define NOBLOCK 1
#define BLOCK   0	// 阻塞标志位
#define SOCK_EVENT_READ    (1<<0)
#define SOCK_EVENT_WRITE   (1<<1)
#define SOCK_EVENT_CONN    (1<<2)
#define SOCK_EVENT_ACCEPT  (1<<3)
#define SOCK_EVENT_ERROR   (1<<4)
#define SOCK_EVENT_MASK    (0xff)
#define SOCK_EVENT          0    // 获取事件

enum
{
	CMD_IO_SET=1,
	CMD_IO_GET,
	CMD_TO_SET,// timeout set
	CMD_TO_GET,//timeout get
	CMD_IF_SET,// net_socket bind dev IF (interface)
	CMD_IF_GET,// get the dev if net_socket bind
	CMD_EVENT_GET,// get event , such as SOCK_EVENT_READ,SOCK_EVENT_WRITE, etc
	CMD_BUF_GET,// get send or recv buffer , only for SOCK_STREAM
	CMD_FD_GET,//get free net_socket fd num
	CMD_KEEPALIVE_SET,	// 配置sock的KeepAlive功能，该命令只对NET_SOCK_STREAM有效
	CMD_KEEPALIVE_GET,	// 获取sock的KeepAlive情况，该命令只对NET_SOCK_STREAM有效
};

// 网络设备状态标志位
#define FLAGS_UP 0x1
#define FLAGS_TX 0x2
#define FLAGS_RX 0x4



#define TCP_SND_BUF_MAX 1
#define TCP_RCV_BUF_MAX 2
#define TCP_SND_BUF_AVAIL 3

int MIDDnsResolve(char *name, char *result, int TimeOutMs);
int NetPing(char *dst_ip_str,  long timeout,  int size);
int RouteSetDefault(int ifIndex);
int RouteGetDefault(void);
int NetSocket(int domain, int type, int protocol);
int SockAddrSet(struct net_sockaddr *addr, char *ip_str, short port);
int NetConnect(int socket,  struct net_sockaddr *addr,  int addrlen);
int NetBind(int socket, struct net_sockaddr *addr,  int addrlen);
int NetCloseSocket (int socket);
int NetListen(int socket, int backlog);
int SockAddrGet(struct sockaddr *addr , char *ip_str, short *port);
int NetAccept(int socket, struct net_sockaddr *addr, int *addrlen);
int NetSend(int socket, void *buf, int size, int flags);
int NetRecv (int socket, void *buf, int size, int flags);
int NetSendto(int socket, void *buf, int size, int flags, struct net_sockaddr *addr, int addrlen);
int NetRecvfrom(int socket, void *buf, int size, int flags, struct sockaddr *addr, int *addrlen);
int Netioctl(int socket, int cmd, int arg);
int NetDevGet(int Dev, char *HostIp, char *Mask, char *GateWay, char *Dns);



#endif /* NETPROT_H_ */
