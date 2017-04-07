/*
*===========================================================================
*              PAX CORPORATION PROPRIETARY INFORMATION
*  This software is supplied under the terms of a license agreement or
*  nondisclosure agreement with PAX Corporation and may not be copied
*  or disclosed except in accordance with the terms in that agreement.
*      Copyright(c) 2016? PAX Corporation. All rights reserved.
*
*  Description: Implement key managerment function
*
*  Revision History:
*  Date                  Author            Action
*  20161110              Richard           Created
*===========================================================================
*/



#include "keyManage.h"
#include "RSAEuro/global.h"
#include "RSAEuro/rsaeuro.h"
#include "RSAEuro/rsa.h"


static RSA_KEY_DOWN_INFO rsa_down = {0};
static int s_expLen = 3;

//only for GETTLV USE, add by richard 20170227, tag length is 2
uchar *getPosition(uchar *TlvData, uchar *tag) 
{
    int i, length=0;
    length = TlvData[0]*256 + TlvData[1];
    //PubDebugTx("length:%d\n", length);

    for(i=0; i<length; i++)
    {
        if(0 == memcmp((TlvData+i+2), tag, 2))
        {
            return TlvData+i+2;
        }
    }

    return NULL;
}


int GetTlvData(uchar *TlvData, uchar *tag, uchar *data)
{
    int i, iRet, idataLen=0;
    uchar ucByteLen;
    uchar *p;

    if ((TlvData == NULL) || (tag == NULL) || (data == NULL) )
		return -1;

    p = getPosition(TlvData, tag);
    if(p == NULL)
        return -1;

    p+=2;
    /*if(*p & 0x80)
    {
       ucByteLen = *p &0x7f;

       for(i=1; i<=ucByteLen; i++)
          idataLen = idataLen*256 + *(p+i);

       p+=ucByteLen+1;
    }
    else 
    {*/  //only one byte for every tag, removed by richard 20170227
        idataLen = *p;
        p+=1;
    //}
    
    memcpy(data, p, idataLen);

    return idataLen;
}

int SetTlvData(uchar *TlvData, uchar *tag, uchar *data, int iDataLen)
{
    int iRet;
    int offset = 0;

    if ((TlvData == NULL) || (tag == NULL) || (data == NULL) )
		return -1;
    
    memcpy(TlvData, tag, 2);      //set tag

    //idataLen = strlen(data);  //set length
    /*if(idataLen >= 128 && idataLen< 256)
    {
        memcpy(TlvData+2, '\x81', 1); 
        *(TlvData+3) = (uchar)idataLen;
        offset+=4;
    }
    else if(idataLen >= 256)
    {
        memcpy(TlvData+2, '\x82', 1); 
        *(TlvData+3) = (uchar)(idataLen/256);
        *(TlvData+4) = (uchar)(idataLen%256);
        offset+=5;
    }
    else 
    {*/  //only one byte for TLV data, removed by richard 20170227
        *(TlvData+2) = (uchar)iDataLen; 
        offset+=3;
    //} 

    memcpy(TlvData+offset, data, iDataLen);
    offset +=iDataLen;

    return offset;
}

void SetKeyCommfield(uchar KeyType)
{
    memset(&glSendPack, 0, sizeof(STISO8583));
    sprintf((char *)glSendPack.szMsgCode,    "%.*s", LEN_MSG_CODE,
        glTranConfig[glProcInfo.stTranLog.ucTranType].szTxMsgID);

    //bit3
    if(KEY_TYPE_RSA == KeyType)
    {
        memcpy(glSendPack.szProcCode, "000000", LEN_PROC_CODE);
    }

    //bit 11
    sprintf((char *)glSendPack.szSTAN, "%06lu", glSysCtrl.ulSTAN);

    //bit 22&25
    if(KEY_TYPE_TPK==KeyType)
    {
        sprintf((char *)glSendPack.szEntryMode, "0002");
        sprintf((char *)glSendPack.szCondCode, "00");
    }

    //bit24
    if(ChkExistAcq("CUP_DSB")
       || KEY_TYPE_TMK1==KeyType || KEY_TYPE_TPK==KeyType)  //only CUP_DSB need Nii information
    {
        sprintf((char *)glSendPack.szNii,        "%.*s", LEN_NII,         glCurAcq.szNii);
    }
    
    sprintf((char *)glSendPack.szTermID,     "%.*s", LEN_TERM_ID,     glCurAcq.szTermID);
    sprintf((char *)glSendPack.szMerchantID, "%.*s", LEN_MERCHANT_ID, glCurAcq.szMerchantID);

    //set field60, self-defined
    memcpy(glSendPack.szField60, "00", 2);
    sprintf((char *)glSendPack.szField60+2, "%06ld", glCurAcq.ulCurBatchNo);

    switch(KeyType)
    {
        case KEY_TYPE_RSA:
           memcpy(glSendPack.szField60+8, "352", 3);
           break;
        case KEY_TYPE_TMK1:
            memcpy(glSendPack.szField60+8, "350", 3);
            break;  
        case KEY_TYPE_TMK2:
            memcpy(glSendPack.szField60+8, "351", 3);
            break;
        case KEY_TYPE_TPK:
            memcpy(glSendPack.szField60+8, "003", 3);
            break;
        default:
            break;
    }
}

int RsaKeyDownload(void)
{
    int iRet; 
    ST_RSA_KEY stRsaKey={0};

     iRet = TransInit(LOGON);
    if( iRet!=0 )
    {
        return iRet;
    }

    ScrCls();
    PubDispString(_T("RSA KEY DOWNLOAD"), 0 | DISP_LINE_LEFT | DISP_LINE_REVER);
    
    //set Rsa request field
    SetKeyCommfield(KEY_TYPE_RSA);

    iRet = SendRecvPacket();
    PubDebugTx("SendRecvPacket:%d\n", iRet);
    if(iRet!=0)
    {
        return iRet;
    }

    PubDebugTx("glProcInfo.stTranLog.szRspCode:%s\n", glProcInfo.stTranLog.szRspCode);
    if( memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)!=0 )   //response code  incorrect
    {
        return -1;
    }

    //field62, RSA information
    //memcpy(glRecvPack.sField62, "\x9F\x06\x05\x00\x00\x02\x77\xD6", 8);
    /*for(iRet=0; iRet<185; iRet++)
        PubDebugTx(" %02X", glRecvPack.sField62[iRet]);
    PubDebugTx("\n");*/
    iRet = GetTlvData((uchar *)glRecvPack.sField62, "\x9F\x06", rsa_down.PubKeyRID);
    //PubDebugTx("GetTlvData 9f06:%d %02X %02X\n", iRet, rsa_down.PubKeyRID[3], rsa_down.PubKeyRID[4]);
    if(iRet<0)
    {
        return iRet;
    }

    iRet = GetTlvData(glRecvPack.sField62, "\x9F\x22", &rsa_down.PubKeyIndex);
    //PubDebugTx("GetTlvData 9f22:%d %d\n", iRet, rsa_down.PubKeyIndex);
    if(iRet<0)
    {
        return iRet;
    }

    iRet = GetTlvData(glRecvPack.sField62, "\xDF\x02", rsa_down.PubKeyModule);
    if(iRet<0)
    {
        return iRet;
    }
    memmove(rsa_down.PubKeyModule, &rsa_down.PubKeyModule[29], 128);
    //PubDebugTx("GetTlvData DF02:%d %02X %02X\n", iRet, rsa_down.PubKeyModule[0], rsa_down.PubKeyModule[1]);

    iRet = GetTlvData(glRecvPack.sField62, "\xDF\x04", rsa_down.PubKeyExponential);
    s_expLen = iRet;
    //PubDebugTx("GetTlvData DF04:%d %02X %02X\n", iRet, rsa_down.PubKeyExponential[0], rsa_down.PubKeyExponential[1]);
    if(iRet<0)
    {
        return iRet;
    }

    stRsaKey.iModulusLen = 1024;
    memcpy(&stRsaKey.aucModulus[512-128], rsa_down.PubKeyModule, 128);
    stRsaKey.iExponentLen = s_expLen*8;
    memcpy(&stRsaKey.aucExponent[512-s_expLen], rsa_down.PubKeyExponential, s_expLen);
    
    iRet = PedWriteRsaKey(1, &stRsaKey);
	if(iRet!=0)
	{
    	PubDebugTx("PedWriteRsaKey: %d\n", iRet);
		return iRet;
	}
    
    return 0;
}


int TmkDownload(void)
{
    int i, iRet, offset=0;
    uchar szBuff[32 + 1], TmpBuff[32 + 1], ck_value[8 + 1];
    uchar randomkeybuf[16 + 1];
    uchar TMKResult[16];
    uchar RSAData[256]={0};
    uchar szDataIn[256]={0};
    int mod_len = 2048, RSADataLen=0;
    //uchar dataIn_en[1024] = {0};
    //uchar inch[300], exp[4], modulch[300], outch[300];
    R_RSA_PUBLIC_KEY_CS publicKey = {0};
	R_RANDOM_STRUCT randomStruct = {0};
    ST_KEY_INFO KeyInfoIn = {0};
    ST_KCV_INFO KcvInfoIn = {0};
    
    iRet = TransInit(LOGON);
    if( iRet!=0 )
    {
        return iRet;
    }

    ScrCls();
    PubDispString(_T("TMK DOWNLOAD"), 0 | DISP_LINE_LEFT | DISP_LINE_REVER);

    for (i = 0; i < 16; i++)  //generating random data
    {
        szBuff[i] = (uchar)rand();    
    }

    memcpy(randomkeybuf, szBuff, 16);

    //encrypt 0 data with the random key, and get check value
    memset(TmpBuff, 0x00, sizeof(TmpBuff));
    PubDes(TRI_ENCRYPT, TmpBuff, randomkeybuf, ck_value);

    memset(szDataIn, 0xFF, 128);
    memcpy(szDataIn, "\x00\x02", 2);
    memcpy(szDataIn + 128 - 16 - 1, "\x00", 1);
    memcpy(szDataIn + 128 - 16, randomkeybuf, 16);

    
	iRet = PedRsaRecover(1, szDataIn, RSAData, NULL);
	if (0 != iRet)
		return iRet;

    //set common field for TMK Exchange field
    SetKeyCommfield(KEY_TYPE_TMK1);

    memcpy(rsa_down.RadomEncryptKey, RSAData, 128);
    memcpy(rsa_down.RadomEncryptKeyCheck, ck_value, 4);

    offset = 2; 
    offset += SetTlvData(glSendPack.sField62+offset, "\x9F\x06", rsa_down.PubKeyRID, 5);
    offset += SetTlvData(glSendPack.sField62+offset, "\x9F\x22", &rsa_down.PubKeyIndex, 1);
    /*if(ChkExistAcq("CUP_DSB"))
    {
        offset += SetTlvData(glSendPack.sField62+offset, "\xDF\x02", rsa_down.RadomEncryptKey, 128);
        offset += SetTlvData(glSendPack.sField62+offset, "\xDF\x04", rsa_down.RadomEncryptKeyCheck, 4);
    }
    else 
    {*/
        offset += SetTlvData(glSendPack.sField62+offset, "\xDF\x23", rsa_down.RadomEncryptKey, 128);
        offset += SetTlvData(glSendPack.sField62+offset, "\xDF\x24", rsa_down.RadomEncryptKeyCheck, 4);
    //}
    PubLong2Char((ulong)offset-2, 2, glSendPack.sField62);

    iRet = SendRecvPacket();
    if(iRet!=0)
    {
        return iRet;
    }

    if( memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)!=0 )   //response code  incorrect
    {
        return -1;
    }

    iRet = GetTlvData(glRecvPack.sField62, "\xDF\x21", rsa_down.EncryptTMK);
    if(iRet<0)
    {
        return iRet;
    }
    
    iRet = GetTlvData(glRecvPack.sField62, "\xDF\x22", rsa_down.TMKCheckValue);
    if(iRet<0)
    {
        return iRet;
    }

    for (i = 0; i < 2; i++)
    {
        PubDes(TRI_DECRYPT, rsa_down.EncryptTMK + i * 8, randomkeybuf, TMKResult + i * 8);
    }

    memset(TmpBuff, 0x00, sizeof(TmpBuff));
    PubDes(TRI_ENCRYPT, TmpBuff, TMKResult, ck_value);

    //write TMK, and verify check value
    if(memcmp(rsa_down.TMKCheckValue, ck_value, 4) == 0)
    {
        PedErase();
        KeyInfoIn.ucSrcKeyType = PED_TMK;
        KeyInfoIn.ucSrcKeyIdx  = 0;
        KeyInfoIn.ucDstKeyType = PED_TMK;
        KeyInfoIn.ucDstKeyIdx  = CUP_TMK_ID;
        KeyInfoIn.iDstKeyLen  = 16;
        memcpy(KeyInfoIn.aucDstKeyValue, TMKResult, KeyInfoIn.iDstKeyLen);
        KcvInfoIn.iCheckMode = 0x00;

        iRet = PedWriteKey(&KeyInfoIn, &KcvInfoIn);
        return iRet;
    }
    else 
    {
        return -1;//check value incorrect
    }
}



int TmkActivation(void)
{
    int i, iRet;

    iRet = TransInit(LOGON);
    if( iRet!=0 )
    {
        return iRet;
    }

    ScrCls();
    PubDispString(_T("TMK ACTIVATION"), 0 | DISP_LINE_LEFT | DISP_LINE_REVER);

    //set common field for TMK Activation field
    SetKeyCommfield(KEY_TYPE_TMK2);

    iRet = SendRecvPacket();
    if(iRet!=0)
    {
        return iRet;
    }

    if( memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)!=0 )
    {
        return -1;
    }

    return 0;
}

//1.RSA 2,TMK 3,Active TMK
void KeyDownload(void)
{
    int iRet; 

    memcpy(&glCurAcq, &glSysParam.stAcqList[1], sizeof(ACQUIRER));  //set the default  Acq by richard 20170223

    iRet = RsaKeyDownload();
    if(iRet !=0)
    {
        ScrClrLine(2, 7);
        PubDispString(_T("RSA DOWN ERR"), 3 | DISP_LINE_CENTER);
        PubWaitKey(3);
        return;
    }

    iRet = TmkDownload();
    if(iRet !=0)
    {
        ScrClrLine(2, 7);
        PubDispString(_T("TMK DOWN ERR"), 3 | DISP_LINE_CENTER);
        PubWaitKey(3);
        return;
    }

    iRet = TmkActivation();
    if(iRet !=0)
    {
        ScrClrLine(2, 7);
        PubDispString(_T("TMK ACTIVE ERR"), 3 | DISP_LINE_CENTER);
        PubWaitKey(3);
    }
    else 
    {
        PubBeepOk();
        ScrClrLine(2, 7);
        PubDispString(_T("DOWNLOAD SUCCESS!"), 3 | DISP_LINE_CENTER);
        PubWaitKey(3);
    }

}

void TerminalSignOn(void)
{
    int iRet; 
    ST_KEY_INFO KeyInfoIn = {0};
    ST_KCV_INFO KcvInfoIn = {0};

    memcpy(&glCurAcq, &glSysParam.stAcqList[1], sizeof(ACQUIRER));  //set the default  Acq by richard 20170223

    iRet = TransInit(LOGON);
    if( iRet!=0 )
    {
        return iRet;
    }

    SaveRevInfo(FALSE); //for test, TODO need to delete, 

    ScrCls();
    PubDispString(_T("TERM SIGNON"), 0 | DISP_LINE_LEFT | DISP_LINE_REVER);

    SetKeyCommfield(KEY_TYPE_TPK);

    //set field63
    memcpy(glSendPack.sField63, "\x00\x03", 2);
    memcpy(glSendPack.sField63+2, "000", 3);

    iRet = SendRecvPacket();
    if(iRet!=0)
    {
        return iRet;
    }

    if( memcmp(glProcInfo.stTranLog.szRspCode, "00", 2)!=0 )
    {
        return -1;
    }
    
    //TODO , deal with other response field data.

    //get the TPK and check value
    KeyInfoIn.iDstKeyLen  = 16;
    KcvInfoIn.aucCheckBuf[0] = 0x04;
    memcpy(KeyInfoIn.aucDstKeyValue, glRecvPack.sField62+2, 16);
    memcpy(KcvInfoIn.aucCheckBuf+1, glRecvPack.sField62+18, 4);

    KeyInfoIn.ucSrcKeyType = PED_TMK;
    KeyInfoIn.ucSrcKeyIdx  = CUP_TMK_ID;
    KeyInfoIn.ucDstKeyType = PED_TPK;
    KeyInfoIn.ucDstKeyIdx  = CUP_TPK_ID;
    KcvInfoIn.iCheckMode = 0x01;

    iRet = PedWriteKey(&KeyInfoIn, &KcvInfoIn);
    if(iRet != 0)
    {
        ScrClrLine(2, 7);
        PubDispString(_T("SignOn Failed!"), 3 | DISP_LINE_CENTER);
        PubWaitKey(3);
    }
    else 
    {
        PubBeepOk();
        ScrClrLine(2, 7);
        PubDispString(_T("SignOn SUCCESS!"), 3 | DISP_LINE_CENTER);
        PubWaitKey(3);
    }
    
}


