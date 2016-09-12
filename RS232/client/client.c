#include <posapi.h>
#include "com.h"

#define COMPORT 0
#define DEBUGPORT 3
#define DEBUG_COM
int glCount=0;
uint glPkgLen = 0;
//#define DEBUG
const APPINFO AppInfo={
	"COMM_TEST",
	"APP-TEST",
	"1.0",
	"XH",
	"demo program",
	"",
	0,
	0,
	0,
	""
};
#ifdef DEBUG
#define byte uchar
#define word ushort
void DatBcdToAsc(byte *Asc, byte *Bcd, word Asc_len)
{

	byte	is_first;
	byte	by;
	is_first = (Asc_len % 2);				/* change by wxk 98.11.06 */
	while(Asc_len-- > 0)
	{
		if(is_first)
		{
			by = *Bcd & 0x0f;
			Bcd++;
		}
		else
		{
			by = *Bcd >> 4;
		}

		by += (by >= 0x0a) ? 0x37 : '0';	/* 0x37 = 'A' - 0x0a */

		*Asc++ = by;
		is_first = !is_first;
	}
}
#endif

void PrintCom(const char* str,uint mount)
{
	uint i;
	unsigned char tempBuff[50];
	
	for(i=0;i<mount;i++)
	{
		memset(tempBuff,0,sizeof(tempBuff));
		sprintf(tempBuff,"%02X ",str[i]);
		PortSends(DEBUGPORT,tempBuff,strlen(tempBuff));
		//printf(tempBuff);
	}
}
ushort gen_crc(const char *buf,uint len)
{
    uchar a,leftmost_bits;
    long i,j,tmpl,dividend,total_bits;
 
    //1--calculate the total length in bits
    total_bits=(long)(len+2)<<3;
 
    dividend=0x00;
    leftmost_bits=0;
    i=0;
    while(i<total_bits)
    {
         //2--load bits into the dividend bit by bit till it reaches 17 bits
         for(j=leftmost_bits;j<17 && i<total_bits;j++,i++)
         {
             dividend<<=1;
             if(len>1 && i<len*8)//--the tail is appended with two zeroes
             {
                  a=buf[i/8];
                  if(i<16)a^=0xff;
                  dividend|=(a>>(7-i%8))&0x01;
             }
             else if(len==1 && i<16)
             {
                  if(i<8)a=buf[i/8]^0xff;
                  else a=0xff;
                  dividend|=(a>>(7-i%8))&0x01;
             }
         }
         if(j<17)break;//stop if it has reached the end
 
         //3--perform XOR between the dividend and the divider if D17 is 1
         if(dividend & 0x10000)
             dividend^=0x11021;
 
         //--search for the first 1 bit
         for(j=0;j<17;j++)
         {
             tmpl=dividend&(0x10000>>j);
             if(tmpl)break;
         }
         leftmost_bits=17-j;
    }//while(1)
 
    return dividend;
}
/*
ushort gen_crc(const uchar *data_block,int data_len)
{
  long totalBit,i,j;
  int rightNumber;
  ushort crc16;
  uchar flag;

  totalBit=(long)(data_len+2)<<3;
  rightNumber=0;
  crc16=0;
  i=0;
  flag=0;
 while(i<totalBit)
 {

  for(j=rightNumber;j<16 && i<totalBit;i++,j++) //保证short中加载16位bit
  {
    crc16<<=1;
	if(i<data_len<<3)
	crc16|=(data_block[i/8]>>(7-i%8))&0x01;
  }
	if(j<16)break;

	if(flag==1)
	{
		crc16^=0x1021;
		flag=0;
		goto end;
	}

	if(crc16 & 0x8000)  //如果第16位为1则向左移一位并向后补一位
	{
		rightNumber=15;
		flag=1;
		continue;
	}

end:
	for(j=0;j<16;j++)   //从左开始找第一位1
	{
	if(crc16 & 0x8000 >>j)
	break;
	}
	rightNumber=16-j;
 }//while()
 return crc16;
}

*/
void pack_up(const struct shellData *shell, const char *in_data,ushort *data_len,char *out_data)
{
	char *dataPoint;
	ushort crc,dataLen;
	uchar crcBuf[2];

	dataLen=*data_len;
	dataPoint=out_data;
	out_data[0]=0x02;
	out_data[1]=(glCount>>8) & 0xFF;//store as bigEndian
	out_data[2]=glCount & 0xFF; 
	out_data[3]=(dataLen >>8) & 0xFF;
	out_data[4]=dataLen & 0xFF;
	out_data[5]=shell->cmd;
	dataPoint+=6;

	switch(shell->cmd)
	{
	case NEW_REQUEST:
		memcpy(dataPoint,shell->fileName,20);
		dataPoint+=20;
		memcpy(dataPoint,shell->fileSize,4);
		dataPoint+=4;
		memcpy(dataPoint,shell->pkgMaxSize,2);
		dataPoint+=2;
		memcpy(dataPoint,shell->fileCRC,2);
		dataPoint+=2;
		crc=gen_crc(dataPoint-33,dataLen+4);
		crcBuf[0]=(crc>>8) & 0xFF;
		crcBuf[1]=crc & 0xFF;
		memcpy(dataPoint,crcBuf,2);
		dataPoint+=2;
		break;
	case SEND_REQUEST:
		memcpy(dataPoint,shell->taskNo,2);
		dataPoint+=2;
		memcpy(dataPoint,in_data,dataLen-3);
		dataPoint+=(dataLen-3);
		crc=gen_crc(dataPoint-dataLen-4,dataLen+4);
		crcBuf[0]=(crc >>8 ) & 0xFF;
		crcBuf[1]=crc & 0xFF;
		memcpy(dataPoint,crcBuf,2);
		dataPoint+=2;
		break;
	case END_TASK_REQUEST:
		memcpy(dataPoint,shell->taskNo,2);
		dataPoint+=2;
		memcpy(dataPoint,shell->endCode,1);
		dataPoint+=1;
		crc=gen_crc(dataPoint-8,dataLen+4);
		crcBuf[0]=(crc>>8) & 0xFF;
		crcBuf[1]=crc & 0xFF;
		memcpy(dataPoint,crcBuf,2);
		dataPoint+=2;
		break;
	default:
		ScrClrLine(3,4);
		ScrPrint(0,3,0,"cmd code err\n");
		return;
		break;
	}
	*data_len=(dataPoint-out_data);
}


int rcv_packet(char *packet,ushort *pack_len,int cmd)
{
	int comRet;
	uchar getData[300];
	uchar *dataPoint;
	uint offset;
	uchar buf[20];
	int n;

	memset(getData,0,sizeof(getData));
	dataPoint=getData;
	offset=0;
	TimerSet(0,300);
	while(1)
	{
		if(!TimerCheck(0))
		{
			ScrClrLine(3,4);
			ScrPrint(0,3,0,"COM timeout\n");
			getkey();
			//ScrClrLine(3,4);
			//ScrPrint(0,0,1,"SENDING DATA...");
			return waitAllpkgOverTime;
		}
		comRet=PortRecv(COMPORT,&dataPoint[offset++],1000);
		if(comRet)return onePkgOverTime; //return 2客户端重发
		if(offset==( (dataPoint[3] << 8) + (dataPoint[4] & 0xFF) +7)) break;
		if(offset>2048)
		{
			ScrClrLine(3,4);
			ScrPrint(0,3,0,"OVER Arrange");
			//getkey();
			return recUnexpected;
		}
//		if(glRecCount==0)
//		ScrPrint(0,2,1,"0X%02X",dataPoint[0]);//屏幕查看发送情况
	}

	memcpy(packet,getData,offset);
	*pack_len=offset;
#ifdef DEBUG_COM
	sprintf(buf,"%s","recv");
	PortSends(3,buf,strlen(buf));
	PrintCom(dataPoint,offset);
#endif
#if 0                           /*send to pinpad port for debug*/
	sprintf(buf,"send %d bytes...",offset);
	PortSends(3,buf,strlen(buf));
	for(n=0;n<offset;n++)
	{
		sprintf(buf,"%02X ",packet[n]),
		PortSends(3,buf,strlen(buf));
	}
#endif
	return recOk;
}

int initShell(struct shellData *shell,uint fileLen,int fd)
{
	uchar fileSize[4];
	uchar fileBuff[200000];
	uint pkgMaxLen;
	uchar initPkgMaxLen[2];
	uchar fileCRC[2];
	ushort crcInt;
	int fdRet,readNum;

    if(shell==NULL)
	{
		ScrClrLine(3,4);
		ScrPrint(0,3,3,"iniShell ERR");
		return 1;
	}
	fdRet=seek(fd,0,SEEK_SET);
	if(fdRet<0)
    {
        ScrClrLine(3,4);
        ScrPrint(0,3,1,"initShell seek file %d",errno);
        return 1;
    }
	readNum=read(fd,fileBuff,glPkgLen);
	if(readNum!=glPkgLen)
	{
		ScrClrLine(3,4);
		ScrPrint(0,3,0,"initShell read err");
		return 1;
	}
	crcInt=gen_crc(fileBuff,readNum);  /* get file crc value*/;
	pkgMaxLen=1024;
	memset(fileSize,0,sizeof(fileSize));
	memset(initPkgMaxLen,0,sizeof(initPkgMaxLen));
    memset(shell,0,sizeof(struct shellData));
	memset(fileCRC,0,sizeof(fileCRC));

	shell->cmd=NEW_REQUEST;   /*init cmd*/
	memcpy(shell->fileName,"generator.txt",13); /*init fileName*/
	fileSize[2]=(fileLen >>8) & 0xFF; /*init fileSize*/
	fileSize[3]=fileLen & 0xFF;
	memcpy(shell->fileSize,fileSize,4);

	initPkgMaxLen[0]=(pkgMaxLen >>8 ) & 0xFF;   /*init max_pkg_size*/
	initPkgMaxLen[1]=pkgMaxLen & 0xFF;
	memcpy(shell->pkgMaxSize,initPkgMaxLen,2);

	fileCRC[0]=(crcInt>>8) & 0xFF;   /*init file_CRC*/
	fileCRC[1]=(crcInt) & 0xFF;
	memcpy(shell->fileCRC,fileCRC,2);
	return 0;
}

int processData(const struct shellData *shell,const char *indata,ushort *data_len,char *recBuf,unsigned short *recLen)
{

	uchar outputBuf[3000];
	uint ret,comRet;
	uchar buf[20];
	int n;

	memset(outputBuf,0,sizeof(outputBuf));
	pack_up(shell,indata,data_len,outputBuf);;
repeat:
    comRet=PortSends(COMPORT,outputBuf,*data_len);
    if(comRet)
    {
        ScrClrLine(3,4);      
        ScrPrint(0,3,0,"PortSend error:%d",comRet);
		return 1;
    }
#ifdef DEBUG_COM
	sprintf(buf,"%s","send");
	PortSends(3,buf,strlen(buf));
	PrintCom(outputBuf,*data_len);
#endif 
#if 0/*send to pinpad port for debug*/
	sprintf(buf,"send %d bytes...",*data_len);
	PortSends(3,buf,strlen(buf));
	for(n=0;n<*data_len;n++)
	{
		sprintf(buf,"%02X ",outputBuf[n]);
		PortSends(3,buf,strlen(buf));
	}
	//PortSends(3,"\xFF\xFF\xFF\xFF\xFF",5);
	//PortSends(3,outputBuf,*data_len);
#endif
	ScrClrLine(3,4);
    ScrPrint(0,3,0,"sending %d,%d",glCount+1,*data_len);
    memset(recBuf,0,sizeof(recBuf));
    *recLen=0;
	ret=rcv_packet(recBuf,recLen,shell->cmd);
    switch(ret)
    {
    case recOk:
		glCount++;
        break;
	case onePkgOverTime:            /*if rcv_packet return 2  ,send again*/
		goto repeat;
	default:
		return ret;
    }//switch
	return 0;
}

uchar SendRecvData(int fd)
{

	uchar recBuf[3000],bufSend[1025];
	ushort  recLen=0;
    uint ret,lenSend,count=0;
	//ushort outputLen;
	int fdRet,readNum;	
	struct shellData shell;
	uint sendSize;
	uint sCount;   //times of sending to server 

	lenSend=glPkgLen;
	ret=initShell(&shell,lenSend,fd);
	if(ret)
	{
		getkey();
		return ret;
	}
	memset(recBuf,0,sizeof(recBuf));
	memset(bufSend,0,sizeof(bufSend));
	readNum=29;
	ret=processData(&shell,bufSend,&readNum,recBuf,&recLen);
	if(recBuf[5]!=NEW_RESPONSE || recBuf[6]!=0x00)
	{
		ScrClrLine(4,7);
		ScrPrint(0,4,0,"ERR RESPONSE:%d",recBuf[5]);
		ScrPrint(0,5,0,"CONFIRM SIGN:%d",recBuf[6]);
		ScrPrint(0,6,0,"C RESPONSE=%d",NEW_RESPONSE);
		ScrPrint(0,7,0,"C COMFRIM SIGN=%d",0);
	}
	if(memcmp(shell.fileName,recBuf+9,20))
	{
		ScrClrLine(4,7);
		ScrPrint(0,4,0,"ERR fileName%s",recBuf+9);
		ScrPrint(0,6,0,"C FILENAME=%s",shell.fileName);
	}
	if(memcmp(shell.fileSize,recBuf+29,4))
	{
		ScrClrLine(4,7);
		ScrPrint(0,4,0,"ERR fileSize%02X,%02X,%02X,%02X",recBuf[29],recBuf[30],recBuf[31,recBuf[32]]);
		ScrPrint(0,6,0,"C fileSize%02X,%02X,%02X,%02X",shell.fileSize[0],shell.fileSize[1],shell.fileSize[2,shell.fileSize[3]]);
	}
	if(memcmp(shell.fileCRC,recBuf+35,2))
	{
		ScrClrLine(4,7);
		ScrPrint(0,4,0,"ERR fileCRC:%02X,%02X",recBuf[35],recBuf[36]);
		ScrPrint(0,5,0,"C fileCRC:%02X,%02X",shell.fileCRC[0],shell.fileCRC[1]);
	}
	memcpy(shell.taskNo,recBuf+7,2);  /*after requesting,require the new taskNo and continue to send data  */
	shell.cmd=SEND_REQUEST;
	fdRet=seek(fd,0,SEEK_SET);
	if(fdRet<0)
    {
        ScrClrLine(3,4);
        ScrPrint(0,3,0,"open file errr%d",errno);
        return 1;
    }
	sendSize=lenSend/100;
	sCount = (lenSend/sendSize);
	if(lenSend == 200)  //for test send some times as fews of bytes.
	{
		sendSize = lenSend / 10;
		sCount   = 10;
	}
	if(lenSend == 50)  //for test send some times as fews of bytes.
	{
		sendSize = lenSend / 5;
		sCount   = 5;
	}
	else if(lenSend < 1000 && lenSend != 200 && lenSend != 50)
	{
		sendSize = lenSend;
		sCount   = 1;
	}
	while(count < sCount)  
	{
	memset(recBuf,0,sizeof(recBuf));
	memset(bufSend,0,sizeof(bufSend));
    readNum=read(fd,bufSend,sendSize);
    if(readNum!=sendSize)
    {
        ScrClrLine(3,4);
        ScrPrint(0,3,0,"file read err8");
        return 1;
    }
	recLen=0;
	memset(recBuf,0,sizeof(recBuf));
	readNum=sendSize+3;
	ret=processData(&shell,bufSend,&readNum,recBuf,&recLen);
	if(recBuf[5]!=SEND_RESPOSE || recBuf[6]!=0x00)
	{
		ScrClrLine(3,4);
		ScrPrint(0,3,0,"ERR CMDs %0x,%0x",recBuf[5],recBuf[6]);
		getkey();
		return recUnexpected;
	}
	if(memcmp(shell.taskNo,recBuf+7,2))
	{
		ScrClrLine(3,4);
		ScrPrint(0,3,0,"ERR teskNo %0x,%0x",recBuf[5],recBuf[6]);
		getkey();
		return recUnexpected;
	}
	count++;
	}//while

	shell.cmd=END_TASK_REQUEST;
	shell.endCode[0]=endOk;
	memset(recBuf,0,sizeof(recBuf));
	memset(bufSend,0,sizeof(bufSend));
	recLen=0;
	readNum=4;
	ret=processData(&shell,bufSend,&readNum,recBuf,&recLen);
	if(recBuf[5]!=END_TASK_RESPONSE || recBuf[6]!=0x00)
	{
		ScrClrLine(2,2);
		ScrPrint(0,2,0,"comm fail");  
		ScrClrLine(4,7);
		ScrPrint(0,4,0,"END_CODE=%d",recBuf[5]);
		ScrPrint(0,5,0,"CMD_CODE=%d",recBuf[6]);
		ScrPrint(0,6,0,"C END_CODE=%d",END_TASK_RESPONSE);
		ScrPrint(0,7,0,"CMD_CODE=%d",0x00);
		getkey();
		return 1;
	}
	ScrClrLine(2,2);
	ScrPrint(0,2,0,"comm success");  
	Beep(); /* 嘀 */DelayMs(20); /* 20ms的停顿 */ 
	Beep();DelayMs(20);Beep();
	PortClose(COMPORT);
	close(fd);
	glCount=0;
	getkey();
	//remove("send");remove("recv")
	return 0;
}

int event_main(ST_EVENT_MSG *msg)
{
	return 0;
}

int main(void)
{
    uchar ucKey,ret;
	uchar ucSecKey;
	uint comRet;
	int fd;
	int loopFlag;
	uchar inputStr[10];

	loopFlag=1;
	glPkgLen=102400;
	SystemInit();
#if 1
	PortOpen(DEBUGPORT,"115200,8,n,1");
#endif
    fd=open("generator.txt",O_RDWR);
    if(fd<0)
    {
		ScrClrLine(3,4);
		ScrPrint(0,3,0,"open generator err%d",errno);
		getkey();
		return 1;
    }
	comRet=PortOpen(COMPORT,"115200,8,n,1");
	if(comRet)
    {
		ScrClrLine(3,4);
		ScrPrint(0,3,0,"PortOpen err0X%02X\n",comRet);
		getkey();
	return 1;
    }
	while(loopFlag)
	{
			ScrCls();
			ScrPrint(0,0,0x01|0x80,"     CLIENT     ");
		do{
			ucKey=getkey();
			if(ucKey == KEYCLEAR)
			{
				ucSecKey = ucKey;
			}
			if(ucSecKey == KEYCLEAR && ucKey == KEY1)
			{
				ScrCls();
				ScrPrint(0,0,0x01|0x80,"  SET DATA LEN  ");
				ScrPrint(0,3,0x00,"INPUT VALUE:");
				ScrGotoxy(15,4);
				memset(inputStr,0,sizeof(inputStr));
				memcpy(inputStr,"102400",6);
				GetString(inputStr,0xA4,1,6);
				glPkgLen=atoi((uchar*)(inputStr+1));
				ScrCls();
				ScrPrint(0,3,0x00,"INPUT VALUE=%d",glPkgLen);
				getkey();
				ucSecKey = NOKEY;
				ScrCls();
				ScrPrint(0,0,0x01|0x80,"     CLIENT     ");
			}
		  }while(ucKey!=KEY1 && ucKey!=KEYENTER && ucKey!=KEYCANCEL);
		switch(ucKey)
		{
		case KEY1:
		case KEYENTER:
			ret=SendRecvData(fd);
			loopFlag = 0;
			break;
		case KEYCANCEL:
			ret=1;
			loopFlag = 0;
			break;
		default:
			break;
		}//switch
	}//while(1)
	PortClose(COMPORT);
	PortClose(DEBUGPORT);
	return ret;
}

