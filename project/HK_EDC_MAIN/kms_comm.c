#include "kms_comm.h"
#include "ini.h"
#include "kms_ex.h"
#include "RSAEuro/global.h"
#include "RSAEuro/rsaeuro.h"
#include "RSAEuro/rsa.h"

static void stn_32bit_crc(unsigned char crcout[4], unsigned char *buf, unsigned short len);
static void GenLRC(const unsigned char *package, int len, unsigned char *EDC);
static int  KmsSendPack(int handle, unsigned char cmd, unsigned char *buff, int SndLen);
static int  KmsRecvControlPack(int handle, unsigned char cmd, unsigned char *buff, int *RecvLen);
static int  KmsRecvPack(int handle, unsigned char cmd, unsigned char *buff, int *RecvLen);
static int  KmsSendRecv(int handle, unsigned char cmd, unsigned char *SndBuff, int SndLen,unsigned char *RcvBuff, int *RecvLen);
static int  KmsWnetRecvPacket(int handle, unsigned char cmd, unsigned char *buff, int *RecvLen);
static int  s_getPOSTypeInfo();
static void KmsGenEDC(unsigned char *package, unsigned char *EDC);


void stn_32bit_crc(unsigned char crcout[4], unsigned char *buf, unsigned short len)
{
	unsigned long rsl, tl;
	unsigned short i;
	unsigned char tmpch, k;

	rsl = 0xffffffffL;
	for (i = 0; i < len; i++)
	{
		tmpch = (unsigned char )rsl; // 截断高位，取低八位
		tmpch = tmpch^buf[i];
		tl = (unsigned long)tmpch;
		for (k = 0; k < 8; k++)
		{
			if (tl & 1)
			{
				tl = 0xedb88320L^(tl>>1);
			}
			else
			{
				tl = tl>>1;
			}
		}
		rsl = tl^(rsl>>8);
	}
	rsl ^= 0xffffffffL;
	crcout[0] = (unsigned char )(rsl>>24);
	crcout[1] = (unsigned char )(rsl>>16);
	crcout[2] = (unsigned char )(rsl>>8);
	crcout[3] = (unsigned char )(rsl);
}


void GetRandHexStr(uchar *psOut, uchar ucOutLen)
{
    int ii;
    uchar   sBuff[16];

    GetTime(sBuff);
    ii = sBuff[3]*65536 + sBuff[4]*256 + sBuff[5];
    srand(ii);

    for (ii=0; ii<ucOutLen; ii++)
    {
        psOut[ii] = (uchar)rand();
    }
}


/*
unsigned char *TLVData		[IN]
int tag						[IN]
int *len						[OUT]
unsigned char *value			[OUT]
*/
int ParseTLVData(unsigned char *TLVData,int tag,int *len,unsigned char *value)
{
	int length = 0;

	if ((TLVData == NULL) || (value == NULL) || (tag == 0x00) )
		return -1;
	
	if(TLVData[0] != 0xff || TLVData[1] != tag )//all tag is FFXX
		return -2;

	length = TLVData[2];
	memcpy(value,TLVData+3,length);
	*len = length;
			
	return PROTIMS_SUCCESS;	
}

/*
unsigned char *TLVData		[OUT]
int tag						[IN]
int len						[IN]
unsigned char *value			[IN]
*/
int ConstructTLVData(unsigned char *TLVData,int tag,int len,unsigned char *value)
{

	if ((TLVData == NULL) || (value == NULL) || (tag == 0x00) || (len == 0x00))
		return -1;
	
	//tag
	TLVData[0] = 0xff;
	TLVData[1] = tag;//all tag is FFXX

	//length
	TLVData[2] = len;

	//value
	memcpy(TLVData+3,value,len);
	
	return PROTIMS_SUCCESS;	
}

void KmsGenEDC(unsigned char *package, unsigned char *EDC)
{
	unsigned short len;

	len = package[1] * 256 + package[2] + 3;
	stn_32bit_crc(EDC, package, len);
}

int KmsHandshake(int handle)
{
	int i, iRet, iRecvLen;
	unsigned char buff[20];

	for(i=0; i<PROTIMS_HANDSHAKE_RETRY_TIMES; i++)
	{
		pcTMSReset(handle);
		memcpy(buff, "READY", 5);
		buff[5] = 3;
		buff[6] = 5;  // 协议版本号为3.4
		iRet = KmsSendPack(handle, 0xF0, buff, 7);
		if (iRet != PROTIMS_SUCCESS)
		{
			DelayMs(900);
			continue;
		}
		DelayMs(200);

		iRet = KmsRecvControlPack(handle, 0xF0, buff, &iRecvLen);
//		iRet = KmsRecvPack(handle, 0xF0, buff, &iRecvLen);
		if ((PROTIMS_SUCCESS==iRet) && (7==iRecvLen))
		{
			if (0 == memcmp(buff, "READY", 5))
				break;
		}
		DelayMs(800);
	}

	if (i >= PROTIMS_HANDSHAKE_RETRY_TIMES)
	{
		iRet = KmsSendPack(handle, 0xF0, "EOT", 3);		// 断开信号
		return KMS_HANDSHAKE_ERROR;
	}

	return PROTIMS_SUCCESS;
}

int KmsGetRSAKey(int handle,unsigned char *psTermID,unsigned char *RcvBuff, int *RecvLen)
{
	unsigned char package[128] = {0};
	int iRet = 0,i = 0;
	
	memcpy(package+i, psTermID, strlen((char*)psTermID)); 
	i += 8;
	
	memset(p_gbVars.M_SN,0,sizeof(p_gbVars.M_SN));
	ReadSN(p_gbVars.M_SN);
	
	memcpy(package+i, p_gbVars.M_SN, 8);
	i += 8;

	package[i++] = s_getPOSTypeInfo();
	
	iRet = KmsSendRecv(handle, PROTIMS_MSG_GET_RSAKEY,package, i, RcvBuff, RecvLen);
	if (PROTIMS_SUCCESS != iRet)
		return iRet;
	
	return PROTIMS_SUCCESS;
}

int KmsGetMainKey(int handle,unsigned char *Buff,unsigned char *RcvBuff, int *RecvLen)
{
	int iRet = 0;
	int ModuleLen = 0,ExpLen = 0,RSADataLen = 0;
	unsigned char Module[256] = {0},Exp[256] = {0};
	unsigned char RSAData[256] = {0};
	unsigned char DesData[16] = {0};
	unsigned char SendBuff[512] = {0};
	R_RSA_PUBLIC_KEY_CS publicKey = {0};
	R_RANDOM_STRUCT randomStruct = {0};


	//parse TLV data
	ParseTLVData(Buff,0x01,&ModuleLen,Module);//FF01
	
	ParseTLVData(Buff+3+ModuleLen,0x02,&ExpLen,Exp);//FF02
	
	memset(p_gbVars.sTmpMKey,0,sizeof(p_gbVars.sTmpMKey));
	GetRandHexStr(p_gbVars.sTmpMKey,sizeof(p_gbVars.sTmpMKey));


#if 0
	printk("[KmsGetMainKey] sTmpMKey:%d\r\n",sizeof(p_gbVars.sTmpMKey));
	temp = sizeof(p_gbVars.sTmpMKey);
	pTemp = &p_gbVars.sTmpMKey;
	while(temp-->0)
	{
		printk("0x%02x ",*pTemp++);
	}
	
	printk("\r\n");
#endif	

	//iRet = RSARecover(Module,ModuleLen,Exp,ExpLen,p_gbVars.sTmpMKey,RSAData);
	publicKey.bits = ModuleLen*8;
	memcpy(publicKey.modulus,Module,ModuleLen);
	memcpy(publicKey.exponent+(MAX_RSA_MODULUS_LEN-ExpLen),Exp,ExpLen);

	iRet = RSAPublicEncrypt(RSAData, &RSADataLen, p_gbVars.sTmpMKey, sizeof(p_gbVars.sTmpMKey), &publicKey, &randomStruct);
	if (PROTIMS_SUCCESS != iRet)
		return s_SetErrorCode(PROTIMS_RSA_ENCRYPT_ERROR, iRet);

#if 0
	printk("[KmsGetMainKey] RSAData:%d\r\n",RSADataLen);
	temp = RSADataLen;
	pTemp = &RSAData;
	while(temp-->0)
	{
		printk("0x%02x ",*pTemp++);
	}
	printk("\r\n");
#endif	
	//3des encrypt data
	PubDes(TRI_ENCRYPT, (unsigned char *)"\x00\x00\x00\x00\x00\x00\x00\x00", p_gbVars.sTmpMKey, DesData);
	
	ConstructTLVData(SendBuff,0x07,RSADataLen,RSAData);//FF07
	
	ConstructTLVData(SendBuff+3+RSADataLen,0x08,4,DesData);//FF08
	
	iRet = KmsSendRecv(handle, PROTIMS_MSG_GET_MAINKEY,SendBuff,RSADataLen+10, RcvBuff, RecvLen);
	if (PROTIMS_SUCCESS != iRet)
		return iRet;	

	return PROTIMS_SUCCESS;
}

int KmsCalculateKeyNum(unsigned char *InBuf,int BufLen)
{
	int num = 0;
	unsigned char *pTemp = InBuf;

	if(InBuf == NULL || BufLen <= 0)
		return -1;
	
	while(BufLen-->0)
	{
		if(*pTemp++ == '|')
			num += 1;
	}
	
	return num;
}

int KmsWriteMainKey(unsigned char *Buff,int BufLen)
{
	int iRet = 0, len = 0,KeyLen = 0,i = 0;
	unsigned char KeyId = 0,KeyType = 0,KeyData[256] = {0},KCV[4] = {0},KSN[16] = {0};
	uint KIN;//2014-7-8 Key Index Number
	unsigned char SrcKeyType = 0,SrcKeyIdx = 0;
	unsigned char DesData[24] = {0};
	unsigned char Mode = 0,KeyNum = 0;
	unsigned char *pTemp;
	//uchar psKeyBCD[16]=  //test
	uchar ucKeyLen = 16; //test
	
	ST_KEY_INFO	stKeyInfoIn;
	ST_KCV_INFO	stKcvInfoIn;

	KeyNum = Buff[0];
	//KIN = (uint) KeyNum;//2014-9-1 KIN should be assigned by Acquirer but not the total number of keys
	pTemp = Buff+1;
	while(KeyNum-- > 0)//2014-7-7 穦盢赣TID Group┮ΤKey常load秈ㄓ
	{
		//parse TLV data
		ParseTLVData(pTemp+i,0x03,&len,&KeyId);//FF03 4bytes
		i += 4;
		
		ParseTLVData(pTemp+i,0x04,&len,&KeyType);//FF04 4bytes
		i += 4;
		
		ParseTLVData(pTemp+i,0x05,&KeyLen,KeyData);//FF05 (3+KeyLen)bytes
		i += 3+KeyLen;
		
		ParseTLVData(pTemp+i,0x06,&len,KCV);//FF06 7bytes
		i += 7;

		ParseTLVData(pTemp+i,0x0f,&len,&SrcKeyIdx);//FF0F 4bytes
		i += 4;

		ParseTLVData(pTemp+i,0x10,&len,&SrcKeyType);//FF10 4bytes
		i += 4;

	/*	PrnInit();
		PrnStr("[3]KEY Value: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",KeyData[0],KeyData[1],KeyData[2],KeyData[3]
		,KeyData[4],KeyData[5],KeyData[6],KeyData[7],KeyData[8],KeyData[9],KeyData[10],KeyData[11],KeyData[12],KeyData[13],KeyData[14],KeyData[15]);*/
	

		if(KeyType == PED_DUKPT)
		{
			ParseTLVData(pTemp+i,0x0b,&len,KSN);//FF0B 11bytes
			i += 3+len;
		}

		if(*(pTemp+i) != '|')
			return s_SetErrorCode(PROTIMS_KEY_NUM_ERROR, PROTIMS_KEY_NUM_ERROR);
		i += 1;
		
		//3des decrypt data
		if(KeyLen == 8)
		{
			PubDes(TRI_DECRYPT, KeyData, p_gbVars.sTmpMKey, DesData);
		}
		else if(KeyLen == 16)
		{
			PubDes(TRI_DECRYPT, KeyData, p_gbVars.sTmpMKey, DesData);
			PubDes(TRI_DECRYPT, KeyData+8, p_gbVars.sTmpMKey, DesData+8);
		}
		else if(KeyLen == 24)
		{
			PubDes(TRI_DECRYPT, KeyData, p_gbVars.sTmpMKey, DesData);
			PubDes(TRI_DECRYPT, KeyData+8, p_gbVars.sTmpMKey, DesData+8);
			PubDes(TRI_DECRYPT, KeyData+16, p_gbVars.sTmpMKey, DesData+16);	
		}	
		else
			return s_SetErrorCode(PROTIMS_KEY_LENGTH_ERROR, PROTIMS_KEY_LENGTH_ERROR);

		memset(&stKeyInfoIn, 0, sizeof(stKeyInfoIn));
		memset(&stKcvInfoIn, 0, sizeof(stKcvInfoIn));

		stKeyInfoIn.ucSrcKeyType   = SrcKeyType;
		//stKeyInfoIn.ucSrcKeyType = 2; 
		stKeyInfoIn.ucSrcKeyIdx  = SrcKeyIdx;
		stKeyInfoIn.ucDstKeyType = KeyType;
		stKeyInfoIn.ucDstKeyIdx  = KeyId;
		stKeyInfoIn.iDstKeyLen   = KeyLen;
		KIN = KeyId;//2014-9-1 KIN is the Key assigned by Acquirer
		memcpy(stKeyInfoIn.aucDstKeyValue, DesData, KeyLen);
		////2015-3-10 ttt
		//ScrCls();
		//ScrPrint(0,0,0,"KeyNum: %d",KeyNum);
		//ScrPrint(0,2,0,"SrcKeyType: %02x",SrcKeyType);
		//ScrPrint(0,4,0,"DstKeyType: %02x",KeyType);
		//ScrPrint(0,6,0,"DstKeyId: %d",KeyId);
		//getkey();
		//ScrCls();
		//ScrPrint(0,0,0,"[LEN:%d]; KIN: %d",KeyLen,KIN);
		//ScrPrint(0,2,0,"KEY Value: %02x%02x%02x%02x%02x%02x%02x%02x",stKeyInfoIn.aucDstKeyValue[0],stKeyInfoIn.aucDstKeyValue[1],stKeyInfoIn.aucDstKeyValue[2],stKeyInfoIn.aucDstKeyValue[3]
		//,stKeyInfoIn.aucDstKeyValue[4],stKeyInfoIn.aucDstKeyValue[5],stKeyInfoIn.aucDstKeyValue[6],stKeyInfoIn.aucDstKeyValue[7]);
		//ScrPrint(0,5,0,"KEY Data: %02x%02x%02x%02x%02x%02x%02x%02x",KeyData[0],KeyData[1],KeyData[2],KeyData[3]
		//,KeyData[4],KeyData[5],KeyData[6],KeyData[7]);
		//getkey();//KeyData

		stKcvInfoIn.iCheckMode = 1;
		stKcvInfoIn.aucCheckBuf[0] = 4;
		memcpy(stKcvInfoIn.aucCheckBuf+1, KCV, 4);

		if (0 == stKeyInfoIn.ucSrcKeyIdx && PED_BKLK == stKeyInfoIn.ucSrcKeyType)
		{
			PedErase();
		}
			
		if(KeyType == PED_DUKPT)
		{
			iRet = PedWriteTIK(KeyId,SrcKeyIdx,(unsigned char)KeyLen,DesData,KSN, &stKcvInfoIn);
			if (PROTIMS_SUCCESS != iRet)
				return s_SetErrorCode(iRet, iRet);
		}
		else
		{
			//stKeyInfoIn.ucDstKeyType = PED_TDK; //Gillian debug
			iRet = PedWriteKey(&stKeyInfoIn, &stKcvInfoIn);//2014-7-4 1) KMS write key //2015-3-10 mark!!!
			////2015-3-10 ttt Gillian debug
			
		/*	ScrCls();
			ScrPrint(0,1,0,"[1]Key ucSrcKeyIdx: %02x",stKeyInfoIn.ucSrcKeyIdx); //KMS TMK 00
			ScrPrint(0,3,0,"[1]Key ucSrcKeyType: %02x",stKeyInfoIn.ucSrcKeyType); //01
			getkey();

			ScrCls();
			ScrPrint(0,0,0,"iRet: %d",iRet);
			ScrPrint(0,1,0,"[1]KeyType: %02x, Index: %d",stKeyInfoIn.ucDstKeyType,stKeyInfoIn.ucDstKeyIdx); // 02
			ScrPrint(0,2,0,"[1]Key Idx: %02x",stKeyInfoIn.ucDstKeyIdx); //06
			ScrPrint(0,3,0,"[1]Key LEN: %02x",stKeyInfoIn.iDstKeyLen);  //10
			ScrPrint(0,4,0,"[1]KEY Value: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",stKeyInfoIn.aucDstKeyValue[0],stKeyInfoIn.aucDstKeyValue[1],stKeyInfoIn.aucDstKeyValue[2],stKeyInfoIn.aucDstKeyValue[3]
			,stKeyInfoIn.aucDstKeyValue[4],stKeyInfoIn.aucDstKeyValue[5],stKeyInfoIn.aucDstKeyValue[6],stKeyInfoIn.aucDstKeyValue[7]
			,stKeyInfoIn.aucDstKeyValue[8],stKeyInfoIn.aucDstKeyValue[9],stKeyInfoIn.aucDstKeyValue[10],stKeyInfoIn.aucDstKeyValue[11]
			,stKeyInfoIn.aucDstKeyValue[12],stKeyInfoIn.aucDstKeyValue[13],stKeyInfoIn.aucDstKeyValue[14],stKeyInfoIn.aucDstKeyValue[15]);
			getkey();

			//print
			PrnStr("stKeyInfoIn.ucDstKeyType = PED_TMK\n");
			PrnStr("iRet: %d\n",iRet);
			PrnStr("[1]Key ucSrcKeyIdx: %02x\n",stKeyInfoIn.ucSrcKeyIdx); //KMS TMK 00
			PrnStr("[1]Key ucSrcKeyType: %02x\n",stKeyInfoIn.ucSrcKeyType); //01
			PrnStr("[1]KeyType: %02x, Index: %d\n",stKeyInfoIn.ucDstKeyType,stKeyInfoIn.ucDstKeyIdx); // 02
			PrnStr("[1]Key Idx: %02x\n",stKeyInfoIn.ucDstKeyIdx); //06
			PrnStr("[1]Key LEN: %02x\n",stKeyInfoIn.iDstKeyLen);  //10
			PrnStr("[1]KEY Value: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",stKeyInfoIn.aucDstKeyValue[0],stKeyInfoIn.aucDstKeyValue[1],stKeyInfoIn.aucDstKeyValue[2],stKeyInfoIn.aucDstKeyValue[3]
			,stKeyInfoIn.aucDstKeyValue[4],stKeyInfoIn.aucDstKeyValue[5],stKeyInfoIn.aucDstKeyValue[6],stKeyInfoIn.aucDstKeyValue[7]
			,stKeyInfoIn.aucDstKeyValue[8],stKeyInfoIn.aucDstKeyValue[9],stKeyInfoIn.aucDstKeyValue[10],stKeyInfoIn.aucDstKeyValue[11]
			,stKeyInfoIn.aucDstKeyValue[12],stKeyInfoIn.aucDstKeyValue[13],stKeyInfoIn.aucDstKeyValue[14],stKeyInfoIn.aucDstKeyValue[15]);*/
			

			//Gillian 20160727
			if(stKeyInfoIn.ucDstKeyType == PED_TMK)// || stKeyInfoIn.ucDstKeyType == PED_TPK)
			{
				stKeyInfoIn.ucDstKeyType = PED_TDK;
				
				//memcpy(stKeyInfoIn.aucDstKeyValue, "3333333333333333" , ucKeyLen);
				iRet = PedWriteKey(&stKeyInfoIn, &stKcvInfoIn);//2014-7-4 1) KMS write key
	

				/*PrnStr("stKeyInfoIn.ucDstKeyType = PED_TDK\n");
				PrnStr("iRet: %d\n",iRet);
				PrnStr("[1]Key ucSrcKeyIdx: %02x\n",stKeyInfoIn.ucSrcKeyIdx); //KMS TMK 00
				PrnStr("[1]Key ucSrcKeyType: %02x\n",stKeyInfoIn.ucSrcKeyType); //01
				PrnStr("[1]KeyType: %02x, Index: %d\n",stKeyInfoIn.ucDstKeyType,stKeyInfoIn.ucDstKeyIdx); // 02
				PrnStr("[1]Key Idx: %02x\n",stKeyInfoIn.ucDstKeyIdx); //06
				PrnStr("[1]Key LEN: %02x\n",stKeyInfoIn.iDstKeyLen);  //10
				PrnStr("[1]KEY Value: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",stKeyInfoIn.aucDstKeyValue[0],stKeyInfoIn.aucDstKeyValue[1],stKeyInfoIn.aucDstKeyValue[2],stKeyInfoIn.aucDstKeyValue[3]
				,stKeyInfoIn.aucDstKeyValue[4],stKeyInfoIn.aucDstKeyValue[5],stKeyInfoIn.aucDstKeyValue[6],stKeyInfoIn.aucDstKeyValue[7]
				,stKeyInfoIn.aucDstKeyValue[8],stKeyInfoIn.aucDstKeyValue[9],stKeyInfoIn.aucDstKeyValue[10],stKeyInfoIn.aucDstKeyValue[11]
				,stKeyInfoIn.aucDstKeyValue[12],stKeyInfoIn.aucDstKeyValue[13],stKeyInfoIn.aucDstKeyValue[14],stKeyInfoIn.aucDstKeyValue[15]);
				StartPrinter();*/
			}
			

			if (PROTIMS_SUCCESS != iRet)
				return s_SetErrorCode(iRet, iRet);
			//2014-7-8 ttt
			if(stKeyInfoIn.ucDstKeyType == PED_TDK)
			{
				glCurAcq.ulKeyIdx = KeyId;
				glCurAcq.ucIsSupportKIN = SupportKIN;
				glCurAcq.ucIsKeyBlank = KeyNotBlank;
				glCurAcq.ulKIN = KIN;//????
			}
		}
		
	}
	return PROTIMS_SUCCESS;
}

int KmsExchangeKeyState(int handle)
{
	unsigned char state = 1;
	int iRet = 0;
	
	iRet = KmsSendPack(handle, PROTIMS_MSG_GET_KEYSTATE, &state, 1);
	if (PROTIMS_SUCCESS != iRet)
		return iRet;
		
	return PROTIMS_SUCCESS;
}
int  KmsEnd(int handle)
{
	return KmsSendPack(handle, 0xF0, "EOT", 3);
}


int KmsSendPack(int handle, unsigned char cmd, unsigned char *buff, int SndLen)
{
#ifdef VERNE_DEBUG_
	int i;
#endif

	p_gbVars.g_aucSendBuff[0] = 0x02;  // 夹醚才
	p_gbVars.g_aucSendBuff[1] = cmd;   // ㏑絏
	p_gbVars.g_aucSendBuff[2] = (unsigned char)(SndLen/256);  // 计沮蔼
	p_gbVars.g_aucSendBuff[3] = (unsigned char)(SndLen%256);  // 计沮

	memcpy(p_gbVars.g_aucSendBuff+4, buff, SndLen);
	KmsGenEDC(p_gbVars.g_aucSendBuff+1, p_gbVars.g_aucSendBuff+SndLen+4);

	// Len(2 hex) + TPDU(5 byte), suixingpay, verne, 20120611
	if (p_gbVars.g_bTosanDial)
	{
		SndLen += 8;
		memmove(p_gbVars.g_aucSendBuff + 7, p_gbVars.g_aucSendBuff, SndLen);
		SndLen += 5;
		p_gbVars.g_aucSendBuff[0] = SndLen / 256;
		p_gbVars.g_aucSendBuff[1] = SndLen % 256;
		memcpy(&p_gbVars.g_aucSendBuff[2], p_gbVars.TPDU_Tosan, 5);
		SndLen += 2;

		if (MODEM_COMM == p_gbVars.gbSerialLoad)
		{
			memmove(&p_gbVars.g_aucSendBuff[1], p_gbVars.g_aucSendBuff, SndLen);
			p_gbVars.g_aucSendBuff[0] = 0x02;
			PubLong2Bcd(SndLen - 2, 2, p_gbVars.g_aucSendBuff+1);
			p_gbVars.g_aucSendBuff[SndLen + 1] = 0x03;
			GenLRC(&p_gbVars.g_aucSendBuff[1], SndLen + 1, &p_gbVars.g_aucSendBuff[SndLen + 2]);
			SndLen += 3;
		}		
	}
	else
		SndLen += 8;

#ifdef VERNE_DEBUG_
	if (cmd != PROTIMS_MSG_LOAD_DATA)
	{
		PrnInit();
		PrnStr("Send len=%d | ", SndLen);
		for(i = 0; i < SndLen; i++)
		{
			PrnStr("%02X ", p_gbVars.g_aucSendBuff[i]);
		}
		PrnStart();
	}
#endif

	return pcTMSSendPack(handle, p_gbVars.g_aucSendBuff, SndLen);

//	return pcTMSSendPack(handle, p_gbVars.g_aucSendBuff, SndLen + 8);
}


int KmsSendRecv(int handle, unsigned char cmd, unsigned char *SndBuff, int SndLen,unsigned char *RcvBuff, int *RecvLen)
{
	int i, ret;
#ifdef VERNE_DEBUG_
	int j;
#endif

	for (i=0; i<3; i++)
	{
		pcTMSReset(handle);
		ret = KmsSendPack(handle, cmd, SndBuff, SndLen);
		if (PROTIMS_USERCANCEL == ret)
			return PROTIMS_USERCANCEL;

		if (PROTIMS_SUCCESS != ret)
			continue;

		ret = KmsRecvPack(handle, cmd, RcvBuff, RecvLen);
		if (PROTIMS_USERCANCEL == ret)
			return PROTIMS_USERCANCEL;
#if 0
		/*only for test*/
		{
			int temp = *RecvLen;
			unsigned char *pTemp = RcvBuff;
			printk("\r\n[KmsSendRecv]--------receive data:%d\r\n",temp);
			while(temp-->0)
				printk("0x%02x ",*pTemp++);
			
			printk("\r\n over!\r\n");
		}
#endif

		if ((PROTIMS_SUCCESS==ret) || (PROTIMS_COMM_TIMEOUT_==ret))
		{
#ifdef VERNE_DEBUG_
			if (PROTIMS_SUCCESS == ret && *RecvLen > 0 && cmd != PROTIMS_MSG_LOAD_DATA)
			{
				PrnInit();
				PrnStr("Recv Len=%d | ", *RecvLen);
				for (j = 0; j < *RecvLen; j++)
				{
					PrnStr("%02X ", RcvBuff[j]);
				}
				PrnStart();
			}
#endif
			return ret;  // 接收成功或者超时就退出
		}
	}


	return ret;

}

int KmsRecvPack(int handle, unsigned char cmd, unsigned char *buff, int *RecvLen)
{
	int i, len, ret;
	unsigned char bCount = 0;
	unsigned char edc[4];
	int iCounter = 0;

	if((CDMA_COMM==p_gbVars.gbSerialLoad) || (GPRS_COMM==p_gbVars.gbSerialLoad) || (PPP_COMM==p_gbVars.gbSerialLoad)
		|| (p_gbVars.gbSerialLoad == TCPIP_COMM ))
		return KmsWnetRecvPacket(handle, cmd, buff, RecvLen);
	else if(WIFI_COMM==p_gbVars.gbSerialLoad)		// UNSUPPORT WIFI
		return PROTIMS_SUCCESS;
	else
	{

		TimerSet(PROTIMS_RECV_PACKECT_TIMER, PROTIMS_RECV_PACKET_TIME_OUT*4);
		while (1)
		{
			ret = pcTMSRecvByte(handle, p_gbVars.g_aucSendBuff, PROTIMS_RECV_BYTE_TIMEOUT);
			if (0 == TimerCheck(PROTIMS_RECV_PACKECT_TIMER))
				return s_SetErrorCode(PROTIMS_COMM_TIMEOUT_, PROTIMS_COMM_TIMEOUT_);

			if(PROTIMS_SUCCESS!=ret) 
				return ret;

			if (p_gbVars.g_aucSendBuff[0] != 0x02)
			{
				iCounter++;
				if (iCounter > 100)	// 100个废弃字符
				{
					pcTMSReset(handle);
					return s_SetErrorCode(PROTIMS_COMM_WNET_RECV_ERROR, PROTIMS_COMM_WNET_RECV_ERROR);
				}
				else
					continue;
			}
			iCounter = 0;

			ret = pcTMSRecvByte(handle, p_gbVars.g_aucSendBuff+1, PROTIMS_RECV_BYTE_TIMEOUT);
			if (0 == TimerCheck(PROTIMS_RECV_PACKECT_TIMER))
				return s_SetErrorCode(PROTIMS_COMM_TIMEOUT_, PROTIMS_COMM_TIMEOUT_);

			if(PROTIMS_SUCCESS!=ret) 
				return ret;

			if (p_gbVars.g_aucSendBuff[1] != cmd)
				continue;		// 0x02, 0x02, cmd 的情况下，会出现丢包

			for (i = 0; i < 3; i++)
			{
				ret = pcTMSRecvByte(handle, p_gbVars.g_aucSendBuff+2+i, PROTIMS_RECV_BYTE_TIMEOUT);
				if (0 == TimerCheck(PROTIMS_RECV_PACKECT_TIMER))
					return s_SetErrorCode(PROTIMS_COMM_TIMEOUT_, PROTIMS_COMM_TIMEOUT_);

				if(PROTIMS_SUCCESS!=ret) 
					return ret;
			}

			// 计算数据包长度，然后根据数据包长度接收后面的数据
			len = p_gbVars.g_aucSendBuff[2] * 256 + p_gbVars.g_aucSendBuff[3];

			for (i=0; i<len+4-1; i++)
			{
				ret = pcTMSRecvByte(handle, p_gbVars.g_aucSendBuff+5+i, PROTIMS_RECV_BYTE_TIMEOUT);
				if (0 == TimerCheck(PROTIMS_RECV_PACKECT_TIMER))
					return s_SetErrorCode(PROTIMS_COMM_TIMEOUT_, PROTIMS_COMM_TIMEOUT_);

				if(PROTIMS_SUCCESS!=ret) 
					return ret;
			}

			// 计算接收到的数据包的校验码
			KmsGenEDC(p_gbVars.g_aucSendBuff+1, edc);
			// 比较发送过来的校验码和计算的校验码是否相同，如果不同则接收错误
			if (0 != memcmp(p_gbVars.g_aucSendBuff+4+len, edc, 4))
			{
				// 进入接收到的数据包校验码错误处理
				if(++bCount >= 3)  
					return s_SetErrorCode(PROTIMS_COMM_WNET_RECV_ERROR, PROTIMS_COMM_WNET_RECV_ERROR);
				else
					continue;
			}

			// 检查返回码正确否？返回码为零才正确
			if (p_gbVars.g_aucSendBuff[4] != 0)
			{
				// 返回码错误
				return s_SetErrorCode(PROTIMS_SERVER_DEALERROR, PROTIMS_SERVER_DEALERROR);
			}
			
			memcpy(buff, p_gbVars.g_aucSendBuff+5, len);
			*RecvLen = len-1;
			return PROTIMS_SUCCESS;
		}
	}
}

int KmsWnetRecvPacket(int handle, unsigned char cmd, unsigned char *buff, int *RecvLen)
{
	int RecvCount=0;
	int i = 0;
	int HaveRecvLen = 0;
	int DataLen = 0;
	unsigned char edc[4];
	int WnetRecvLen = -1;

	TimerSet(PROTIMS_TOTAL_RECEV_PACKET_TIMER, PROTIMS_ONE_RECEV_PACKET_TIME_OUT);

	while(1)	
	{
		// 接收第一个包     
		WnetRecvLen = pcTMSNetRecv(handle, p_gbVars.g_aucSendBuff, sizeof(p_gbVars.g_aucSendBuff), 0);
		
		if (WnetRecvLen<0)
		{        
			if ((WnetRecvLen==-13)&& (0!=TimerCheck(PROTIMS_TOTAL_RECEV_PACKET_TIMER)))
				continue;
			else
				return s_SetErrorCode(PROTIMS_WNET_NETRECV, WnetRecvLen);
		}
		else // suixingpay, verne, 20120611
		{
			if (p_gbVars.g_bTosanDial)
			{
				for (i = 0; i < WnetRecvLen -1; i++)
				{
					if (0x02 == p_gbVars.g_aucSendBuff[i] && cmd == p_gbVars.g_aucSendBuff[i+1])
					{
						break;
					}
				}
				if (i < WnetRecvLen)
				{
					memmove(p_gbVars.g_aucSendBuff, p_gbVars.g_aucSendBuff + i, WnetRecvLen - i);
					WnetRecvLen -= i;
					break;
				}
			}
			else
				break;
		}
	}

	HaveRecvLen = 0;

	// 5．10 回应请求下载任务数据包（服务端发送)
	// 在这里POS对服务端发送数据包解析
	if ((0x02!=p_gbVars.g_aucSendBuff[0]) || (cmd!=p_gbVars.g_aucSendBuff[1]))
		return s_SetErrorCode(PROTIMS_COMM_WNET_RECV_ERROR, PROTIMS_COMM_WNET_RECV_ERROR);	

	//返回码，为0表示正确
	if (0 != p_gbVars.g_aucSendBuff[4])
		return s_SetErrorCode(PROTIMS_SERVER_DEALERROR, PROTIMS_SERVER_DEALERROR); 

	// 解析结束
	DataLen = p_gbVars.g_aucSendBuff[2]*256 + p_gbVars.g_aucSendBuff[3];
	HaveRecvLen = WnetRecvLen;	

	// 重新设置超时时间
	TimerSet(PROTIMS_TOTAL_RECEV_PACKET_TIMER, PROTIMS_TOTAL_RECEV_PACKET_TIME_OUT);

	// 接收 数据在传输的过程中有可能被拆解成几个数据包，这里需要拼包
	while (HaveRecvLen < (DataLen+8))
	{		
		WnetRecvLen = pcTMSNetRecv(handle, p_gbVars.g_aucSendBuff+HaveRecvLen, sizeof(p_gbVars.g_aucSendBuff)-HaveRecvLen, 0);

		RecvCount++;  
		if (WnetRecvLen<0)
		{        
			if ((WnetRecvLen==-13)&&(RecvCount < PROTIMS_DATA_RETRY_TIMES) && (0!=TimerCheck(PROTIMS_TOTAL_RECEV_PACKET_TIMER)))
				continue;
			else
				return s_SetErrorCode(PROTIMS_WNET_NETRECV, WnetRecvLen);
		}

		HaveRecvLen += WnetRecvLen;
	}

	// 接收数据长度不对	
	if (HaveRecvLen != (DataLen+8))
		return s_SetErrorCode(PROTIMS_COMM_WNET_RECV_ERROR, PROTIMS_COMM_WNET_RECV_ERROR);

	KmsGenEDC(p_gbVars.g_aucSendBuff+1, edc);

	// 数据长度+ 4[表示符，命令码，数据长度[2BYTE] +wDataLen
	//刚好是数据服务器发送数据包DEC码
	//这里POS在计算DEC码，两者比较相同相同表示数据CRC校验正确
	//接收到的数据包是对的
	if (0 != memcmp(p_gbVars.g_aucSendBuff+4+DataLen, edc, 4)) 
	{
		// 进入接收到的数据包校验码错误处理
		return s_SetErrorCode(PROTIMS_COMM_VERIFY_ERROR, PROTIMS_COMM_VERIFY_ERROR);
	}
	memcpy(buff, p_gbVars.g_aucSendBuff+5, DataLen);
	
	*RecvLen = DataLen-1; //减去g_aucSendBuff[4] 校验数据正确性标志

	return PROTIMS_SUCCESS;
}

int KmsRecvControlPack(int handle, unsigned char cmd, unsigned char *buff, int *RecvLen)
{
	int i, len, ret;
	unsigned char edc[4];
	unsigned char aucBuff[256];
	int byRet = 0;
	int Wifi_RecLen=0;
	*RecvLen = 0;

	if((TCPIP_COMM==p_gbVars.gbSerialLoad)||(CDMA_COMM==p_gbVars.gbSerialLoad) 
	     || (GPRS_COMM==p_gbVars.gbSerialLoad)||(PPP_COMM==p_gbVars.gbSerialLoad))  
	{
		byRet = pcTMSNetRecv(handle, aucBuff, sizeof(aucBuff), 0);
//#ifdef _SXX_PROTIMS
//		byRet=NetRecv(handle, aucBuff, sizeof(aucBuff), 0);
//#else
//		byRet = PXX_NetRecv(handle, aucBuff, sizeof(aucBuff), 0);
//#endif

		// suixingpay, verne, 20120611
		if (p_gbVars.g_bTosanDial)
		{
			for (i = 0; i < byRet - 8; i ++)
			{
				if (aucBuff[i] == 0x02 && cmd == aucBuff[i + 1])
				{
					memmove(aucBuff, aucBuff + i, byRet - i);
					byRet -= i;
					break;
				}
			}
		}

		if ((0x02!=aucBuff[0]) || (cmd!=aucBuff[1])) // 包头字节和命令字验证
		{
			return s_SetErrorCode(PROTIMS_COMM_WNET_RECV_ERROR, byRet);
		}

		*RecvLen = aucBuff[2]*256 + aucBuff[3];

		if (*RecvLen != (byRet - 8)) // 数据包长度验证
		{
			return s_SetErrorCode(PROTIMS_COMM_WNET_RECV_ERROR, byRet);
		}

		KmsGenEDC(aucBuff+1, edc);
		if (memcmp(aucBuff+4+*RecvLen, edc, 4))  // 检查数据包校验码
		{
			return s_SetErrorCode(PROTIMS_SERVER_DEALERROR, PROTIMS_SERVER_DEALERROR);
//			return PROTIMS_PARITY_ERROR;
		}
		memcpy(buff, aucBuff+4, *RecvLen);
		return PROTIMS_SUCCESS;

	}
	else if(WIFI_COMM==p_gbVars.gbSerialLoad)	// UNSUPPORT WIFI
	{
		return PROTIMS_SUCCESS;
	}
	else	// MODEM
	{
		while(1)
		{
			ret = pcTMSRecvByte(handle, aucBuff, PROTIMS_RECV_CONTROL_BYTE_TIMEOUT);
			if (ret)
				return ret;

			ret = pcTMSRecvByte(handle, aucBuff+1, PROTIMS_RECV_CONTROL_BYTE_TIMEOUT);
			if (ret)
				return ret;

			if (aucBuff[1] != cmd)
			{
				continue;
			}

			for(i=0; i<2; i++)
			{
				ret = pcTMSRecvByte(handle, aucBuff+2+i, PROTIMS_RECV_CONTROL_BYTE_TIMEOUT);
				if (ret)
					return ret;
			}

			len = aucBuff[2]*256 + aucBuff[3];

			for (i=0; i<len+4; i++)
			{
				ret = pcTMSRecvByte(handle, aucBuff+4+i, PROTIMS_RECV_BYTE_TIMEOUT);
				if (PROTIMS_SUCCESS != ret)
					return ret;
			}

			KmsGenEDC(aucBuff+1, edc);
			if (memcmp(aucBuff+4+len, edc, 4))
			{
				return s_SetErrorCode(PROTIMS_SERVER_DEALERROR, PROTIMS_SERVER_DEALERROR);
			}

			memcpy(buff, aucBuff+4, len);
			*RecvLen = len;
			return 0;
		}
	}
}


int s_getPOSTypeInfo()
{
	unsigned char out_info[32];
	if (GetTermInfo(out_info) <= 0)
	{
		return -1;
	}

	return out_info[0];
#if 0	
	switch(out_info[0])
	{
	case 1:		// P60-S
	case 3:		// P60-S1
		return REMOTE_P60;
	case 2:
		return REMOTE_P70_4;
	case 4:
		return REMOTE_P80;
	case 5:
		return REMOTE_P78;
	case 6:
		return REMOTE_P90;
	case 7:
		return REMOTE_S80;
	case 8:
		return REMOTE_SP30;
	case 9:
		return REMOTE_S60;
	case 10:
		return REMOTE_S90;
	case 11:
		return REMOTE_S78;
	case 0x0e:
		return REMOTE_S58;//add by chens 2013.5.16
	case 0x80:	// R50
	case 0x81:	// P50
	case 0x82:	// P58
	case 0x83:	// R30
	case 0x84:	// R50-M
	default:
		return -1;
	}	
#endif	
}

void GenLRC(const unsigned char *package, int len, unsigned char *EDC)
{
    int i;
    EDC[0] = 0;
    for (i = 0; i < len; i++)
    {
        *EDC ^= package[i];
    }
}
