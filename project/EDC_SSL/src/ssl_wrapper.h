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

 
#ifndef SHARED_LIBRARY_H_ssl_wrapper
#define SHARED_LIBRARY_H_ssl_wrapper

#ifdef __cplusplus
extern "C"
{
#endif

#include <openssl/ssl.h>

#define SSLERR          -100
#define SSLERR_CONNECT  -101
#define SSLERR_TIMEOUT  -102
#define SSLERR_READ     -103

#define CIPHERS_LIMIT   "ALL:!RC4"

typedef struct SSLCon {
    int socket;
    SSL *sslHandle;
    char szCA[25];
    char szCert[25];
    char szKey[25];
} SSLCon;

void sslLoadFile(SSLCon* con, const char *CA, const char *Cert, const char *Key);
int sslConnect(SSLCon* con, const char* ip, short port, int timeoutMs);
int sslRead(SSLCon* con, char *data, int size, int timeoutMs);
int sslWrite(SSLCon* con, const char *data, int len);
void sslClose(SSLCon *con);

#ifdef __cplusplus
};
#endif

#endif /* SHARED_LIBRARY_H_ssl_wrapper */

