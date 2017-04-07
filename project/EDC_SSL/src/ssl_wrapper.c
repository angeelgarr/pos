/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2015-? PAX Technology, Inc. All rights reserved.
 * Description:
 * Revision History:
 * Date                  Author                 Action
 * 20150322          Kim.L                      created
 * ============================================================================
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <sys/epoll.h>
#include <signal.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <osal.h>

#include "ssl_wrapper.h"

static char *formatIP(const char *ip);

#define check0(x, y, ...) if(x) do { OsLog( LOG_ERROR, __VA_ARGS__); return y; } while(0)
#define check1(x, y,...) if(!(x)) do { OsLog(LOG_ERROR,  __VA_ARGS__); return y;} while(0)

static SSL_CTX *sslContext;

static int setNonBlock(int fd, int value) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return errno;
    }
    if (value) {
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
}

static char *formatIP(const char *ip){
    char    *p, *q, szBuf[5+1], szIp[16 + 1], *newIP = NULL;

    sprintf(szIp, "%.*s",sizeof(szIp), ip);

    newIP = (char *)malloc(20+1);
    *newIP = 0;

    q=szIp;
    while(1){
        int addr;
        p = strchr(q, '.');
        if(p == NULL){
            addr = atoi(q);
            if(*newIP == 0){
                sprintf(newIP, "%d", addr);
            }
            else{
                sprintf(newIP, "%s.%d", newIP, addr);
            }
            break;
        }
        sprintf(szBuf, "%.*s", (int)(p-q), q);
        addr = atoi(szBuf);
        if(*newIP == 0){
            sprintf(newIP, "%d", addr);
        }
        else{
            sprintf(newIP, "%s.%d", newIP, addr);
        }
        q = p + 1;
    }

    return newIP;
}

// Establish a regular tcp connection
static int tcpConnect(const char* ip, short port) {
    int handle = socket(AF_INET, SOCK_STREAM, 0);
    check1(handle >= 0, SSLERR_CONNECT, "socket return error");
    setNonBlock(handle, 1);
    struct sockaddr_in server;
    bzero(&server, sizeof server);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    char *newIP = formatIP(ip);
    inet_aton(ip, (struct in_addr *)&server.sin_addr.s_addr);
    free(newIP);
    int r = connect(handle, (struct sockaddr *) &server, sizeof(struct sockaddr));
    if (r < 0 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
        struct pollfd pfd;
        pfd.fd = handle;
        pfd.events = POLLOUT | POLLERR;
        while (r == 0) {
            r = poll(&pfd, 1, 100);
        }
        check1(pfd.revents == POLLOUT, SSLERR_CONNECT, "poll return error events: %d", pfd.revents);
    }
    return handle;
}

void sslLoadFile(SSLCon* con, const char *CA, const char *Cert, const char *Key)
{
    if(!con)
        return;
    if(CA)
        strcpy(con->szCA, CA);
    if(Cert)
        strcpy(con->szCert, Cert);
    if(Key)
        strcpy(con->szKey, Key);
}

int sslConnect(SSLCon* con, const char* ip, short port, int timeoutMs) {
    ST_TIMER timer = {0,0,0};
    SSL_load_error_strings();
    SSL_library_init();
    sslContext = SSL_CTX_new(SSLv23_client_method());// modify by Daniel 20151222
    if (sslContext == NULL ){
        ERR_print_errors_fp(stderr);
        return SSLERR;
    }
    
    SSL_CTX_set_options(sslContext, SSL_OP_ALL | SSL_OP_NO_SSLv2);
    SSL_CTX_set_cipher_list(sslContext, CIPHERS_LIMIT);

    // specify cipher list
    //int iRet = SSL_CTX_set_cipher_list(sslContext,"TLSv1.2");
    //if(iRet == 0){
    //    return SSLERR;
    //}

    // load CA cert
    //iRet = SSL_CTX_load_verify_locations(sslContext, con->szCA, NULL);
    //if(iRet == 0){
    //    return SSLERR;
    //}

    // Set flag in context to require peer (server) certificate verification
    //SSL_CTX_set_verify(sslContext,SSL_VERIFY_PEER,NULL);
    //SSL_CTX_set_verify_depth(sslContext,1);
    //SSL_CTX_set_default_passwd_cb_userdata(sslContext, "1234");

    // load client cert ssl3.0
    //SSL_CTX_use_certificate_file(sslContext, con->szCert, SSL_FILETYPE_PEM);
    
    // read private key
    //SSL_CTX_use_PrivateKey_file(sslContext, con->szKey,SSL_FILETYPE_PEM);
    
    // verify
    //SSL_CTX_check_private_key(sslContext);

    bzero(con, sizeof *con);
    con->socket = tcpConnect(ip, port);
    con->sslHandle = SSL_new(sslContext);
    if (con->sslHandle == NULL ) {
        ERR_print_errors_fp(stderr);
        check1(0, SSLERR, "SSL_new failed");
    }
    if (!SSL_set_fd(con->sslHandle, con->socket)) {
        ERR_print_errors_fp(stderr);
        check1(0, SSLERR, "SSL_set_fd failed");
    }
    SSL_set_connect_state(con->sslHandle);
    int r = 0;
    int events = POLLIN | POLLOUT | POLLERR;
    OsTimerSet(&timer, timeoutMs);
    while ((r = SSL_do_handshake(con->sslHandle)) != 1 && OsTimerCheck(&timer) > 0) {
        int err = SSL_get_error(con->sslHandle, r);
        if (err == SSL_ERROR_WANT_WRITE) {
            events |= POLLOUT;
            events &= ~POLLIN;
            OsLog( LOG_ERROR, "return want write set events %d\n", events);
        }
        else if (err == SSL_ERROR_WANT_READ) {
            events |= EPOLLIN;
            events &= ~EPOLLOUT;
            OsLog( LOG_ERROR, "return want read set events %d\n", events);
        }
        else {
            OsLog( LOG_ERROR, "SSL_do_handshake return %d error %d errno %d msg %s\n", r, err, errno, strerror(errno));
            ERR_print_errors_fp(stderr);
            check1(0, SSLERR_CONNECT, "do handshake error");
        }
        struct pollfd pfd;
        pfd.fd = con->socket;
        pfd.events = events;
        do {
            r = poll(&pfd, 1, 100);
        } while (r == 0 && OsTimerCheck(&timer) > 0);
        if(errno == EAGAIN){
        	OsTimerSet(&timer, timeoutMs);
        	continue;
        }
        check1(r == 1, SSLERR_CONNECT, "poll return %d error events: %d errno %d %s\n", r, pfd.revents, errno, strerror(errno));
    }
    if(1 == r && 0 == OsTimerCheck(&timer))
    {
        return SSLERR_TIMEOUT;
    }
    OsLog( LOG_ERROR, "ssl connected \n");
    return 0;
}

int sslRead(SSLCon* con, char *data, int size, int timeoutMs) {
	char *buf=NULL;
	int rd = 0;
	int r = 1;
	ST_TIMER timer = {0,0,0};
	if(!con || !data || !size)
	{
		return SSLERR;
	}

	buf = (char *)malloc(size);
	OsTimerSet(&timer, timeoutMs);

	int flags = fcntl(con->socket, F_GETFL, 0);
	fcntl(con->socket, F_SETFL, flags | O_NONBLOCK);

	while (rd < size && r)
	{
		if(0 == OsTimerCheck(&timer))
		{
			free(buf);
			fcntl(con->socket, F_SETFL, flags &(~O_NONBLOCK));
			return SSLERR_TIMEOUT;
		}

		int iPending = SSL_pending(con->sslHandle);
		if(rd > 0 && iPending == 0)
			break;

		r = SSL_read(con->sslHandle, buf + rd, size);

		if (r <= 0) {
			int err = SSL_get_error(con->sslHandle, r);
			if (err == SSL_ERROR_WANT_READ ||
				err == SSL_ERROR_WANT_WRITE) {
				continue;
			}
			//ERR_print_errors_fp(stderr);
			OsLog(LOG_ERROR, "SSL_read ret %d true:%d errno:%ul socket:%d\n",
				r, SSL_get_error(con->sslHandle, r), ERR_get_error(), con->socket);
		}
		if(r < 0)
		{
			free(buf);
			fcntl(con->socket, F_SETFL, flags &(~O_NONBLOCK));
			return SSLERR;
		}

		OsLog(LOG_ERROR, "read %d bytes\n", r);

		rd += r;

		if(rd >= size)
			break;
	}
	OsLog(LOG_ERROR, "read %d bytes contents:\n%.*s\n", rd, rd, buf);

	memcpy(data, buf, rd);
	free(buf);
	fcntl(con->socket, F_SETFL, flags &(~O_NONBLOCK));
	return rd;
}

int sslWrite(SSLCon* con, const char *data, int len) {
    if(con && con->sslHandle)
        return SSL_write(con->sslHandle, data, len);
    return SSLERR;
}

void sslClose(SSLCon *con)
{
    if(con && con->sslHandle)
    {
        SSL_shutdown(con->sslHandle);
        SSL_free(con->sslHandle);
        close(con->socket);
    }
    if(sslContext)
        SSL_CTX_free(sslContext);
    con->sslHandle = NULL;
    con = NULL;
    sslContext = NULL;
}

