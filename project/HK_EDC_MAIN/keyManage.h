/*
*===========================================================================
*              PAX CORPORATION PROPRIETARY INFORMATION
*  This software is supplied under the terms of a license agreement or
*  nondisclosure agreement with PAX Corporation and may not be copied
*  or disclosed except in accordance with the terms in that agreement.
*      Copyright(c) 2016? PAX Corporation. All rights reserved.
*
*  Description: implement UnionPay International key management function.
*               KeyDownload() - TMK download
*               TerminalSignOn() -sign on, TPK download
*
*  Revision History:
*  Date                  Author            Action
*  20161104              Richard           Created
*===========================================================================
*/




#ifndef _KEY_MANAGE_H_
#define _KEY_MANAGE_H_

#include "global.h"


#define CUP_TMK_ID  7
#define CUP_TPK_ID  7

#define KEY_TYPE_RSA   0
#define KEY_TYPE_TMK1  1  //down TMK
#define KEY_TYPE_TMK2  2  //active TMK
#define KEY_TYPE_TPK   3 



/************************************************************************/
/* RSA密钥下载参数信息                                                  */
/* ken add for key download for HK CUP                                  */
/************************************************************************/
typedef struct
{
    unsigned char PubKeyRID[5 + 1];                             //公钥RID
    unsigned char PubKeyIndex;                                  //公钥索引
    unsigned char PubKeyModule[248 + 1];                //公钥模
    unsigned char PubKeyExponential[3 + 1];             //公钥指数
    unsigned char EncryptTMK[128 + 1];                          //TMK密文
    unsigned char TMKCheckValue[4 + 1];                         //TMK校验值
    unsigned char RadomEncryptKey[128 + 1];             //随机密钥密文
    unsigned char RadomEncryptKeyCheck[4 + 1];          //随机密钥校验值
} RSA_KEY_DOWN_INFO;


extern void KeyDownload(void);
extern void TerminalSignOn(void);



#endif





