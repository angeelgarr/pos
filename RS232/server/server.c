#include <posapi.h>
#include "com.h"

#define COMPORT   0
#define DEBUGPORT 3
#define byte uchar
#define word ushort
#define DEBUG_COM
int glCount=0;
int glFd;
int recLastLen=0;
int glRecLen=0;
int recLastSeq=0;


const APPINFO AppInfo={
	"COM_SERVER",
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

#if 0
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
#endif
#if 1   //新版CRC
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

int initShell(struct shellData *shell)
{
    if(shell==NULL)
	{
		ScrClrLine(3,4);
		ScrPrint(0,3,0,"iniShell ERR");
		return 1;
	}
	
	shell->cmd=NEW_RESPONSE;
	memcpy(shell->pkgMaxSize,"\x08\xFF",2);
	memcpy(shell->taskNo,"\x00\x0E",2);
	return 0;
}

void pack_up(const struct shellData *shell,char *out_data,ushort *data_len)
{
	char *dataPoint;
	ushort crc;
	uchar crcBuf[2];

	if(out_data==NULL)
	{
		ScrClrLine(3,4);
		ScrPrint(0,3,0,"param ertempLen+7r\n");
		return ;
	}
	dataPoint=out_data;
	out_data[0]=0x02;
	out_data[1]=(glCount>>8) & 0xFF;//store as bigEndian
	out_data[2]=glCount & 0xFF; 
	
	out_data[5]=shell->cmd;
	dataPoint+=4;

	switch(shell->cmd)
	{
	case NEW_RESPONSE:
		out_data[3]=0x00;
		out_data[4]=32;
		dataPoint+=2;
		dataPoint[6]=shell->confirmSign; //确认标示
		dataPoint+=1;
		memcpy(dataPoint,shell->taskNo,2);//分配的任务号
		dataPoint+=2;
		memcpy(dataPoint,shell->fileName,20);//文件名
		dataPoint+=20;
		memcpy(dataPoint,shell->fileSize,4);//文件大小
		dataPoint+=4;
		memcpy(dataPoint,shell->pkgMaxSize,2);//文件容许的最大尺寸
		dataPoint+=2;
		memcpy(dataPoint,shell->fileCRC,2);//文件的CRC校验码
		dataPoint+=2;
		crc=gen_crc(dataPoint-36,36);
		crcBuf[0]=(crc>>8) & 0xFF;
		crcBuf[1]=crc & 0xFF;
		memcpy(dataPoint,crcBuf,2);
		dataPoint+=2;
		break;
	case SEND_RESPOSE:
		out_data[3]=0x00;
		out_data[4]=4;
		dataPoint+=2;
		dataPoint[6]=shell->confirmSign; //
		dataPoint+=1;
		memcpy(dataPoint,shell->taskNo,2);
		dataPoint+=2;
		crc=gen_crc(dataPoint-8,8);
		crcBuf[0]=(crc>>8) & 0xFF;
		crcBuf[1]=crc & 0xFF;
		memcpy(dataPoint,crcBuf,2);
		dataPoint+=2;
		break;
	case END_TASK_RESPONSE:
		out_data[3]=0x00;
		out_data[4]=4;
		dataPoint+=2;
		dataPoint[6]=shell->confirmSign; //
		dataPoint+=1;
		memcpy(dataPoint,shell->taskNo,2);
		dataPoint+=2;
		crc=gen_crc(dataPoint-8,8);
		crcBuf[0]=(crc>>8) & 0xFF;
		crcBuf[1]=crc & 0xFF;
		memcpy(dataPoint,crcBuf,2);
		dataPoint+=2;
		break;
	default:
		break;
	}
	*data_len=dataPoint-out_data;
}


int rcv_packet(struct shellData *shell,char *packet,uint *pack_len)
{
	int comRet;
	uchar *dataPoint;
	uint offset,tempLen;
	int fid,seqReSign=0;
	uchar repSend[11];
	ushort crc;
	uchar crcBuf[2];
	uchar buf[10];

	dataPoint=packet;
	offset=0;
	TimerSet(0,300);
	while(1)
	{
		if(!TimerCheck(0))
		{
			ScrClrLine(3,4);
			ScrPrint(0,3,0,"COM timeout\n");
			getkey();
			return waitAllpkgOverTime;
		}
		comRet=PortRecv(COMPORT,&dataPoint[offset++],1000);
		if(comRet)
		{
			ScrClrLine(2,2);
			ScrPrint(0,2,0,"WAITING...%d",glCount+1);
			ScrClrLine(5,5);
			ScrPrint(0,5,0,"WAITING FOR DATA");
			return onePkgOverTime;
		}
		if(offset==((dataPoint[3] << 8) + (dataPoint[4] & 0xFF) + 7))
			break;

		if(offset>8000)
		{
			ScrClrLine(3,4);
			ScrPrint(0,3,0,"OVER Arrange");
			//getkey();
#ifdef DEBUG_COM
	sprintf(buf,"%s","recv");
	PortSends(3,buf,strlen(buf));/*send to pinpad port for debug*/
	PrintCom(dataPoint,offset);
#endif
			return recUnexpected;
		}
	}//while
	//if(((dataPoint[4]<<8)+dataPoint[5]) == recLastSeq) return onePkgOverTime;
#ifdef DEBUG_COM
	sprintf(buf,"%s","recv");
	PortSends(3,buf,strlen(buf));/*send to pinpad port for debug*//*send to pinpad port for debug*/
	PrintCom(dataPoint,offset);
#endif

	if(dataPoint[5]!=NEW_REQUEST && dataPoint[5]!=SEND_REQUEST && dataPoint[5]!= END_TASK_REQUEST)
	{
		ScrClrLine(3,4);
		ScrPrint(0,3,0,"error cmdCode%d",dataPoint[0]);
		getkey();
		return recUnexpected;
	}
	crc=gen_crc(dataPoint+1,offset-3);
	crcBuf[0]=(crc>>8) & 0xFF;
	crcBuf[1]=crc & 0xFF;
	if(memcmp(crcBuf,dataPoint+offset-2,2))
	{
		ScrClrLine(3,4);
		ScrPrint(0,3,0,"crc err %0x,%0x,%0x,%0x",crcBuf[0],crcBuf[1],dataPoint[offset-2],dataPoint[offset-1]);
		shell->cmd = END_TASK_RESPONSE;
		shell->confirmSign = ERROR_CRC;
		return recUnexpected;
	}
 /*处理业务放在while()外面，以免影响数据接收时长*/
	switch(dataPoint[5])
	{
		case NEW_REQUEST:
			memcpy(shell->fileName,dataPoint+6,20); //文件名与请求包一致
			memcpy(shell->fileSize,dataPoint+26,4);//文件大小与请求包一致
			memcpy(shell->fileCRC,dataPoint+32,2);//文件校验码与请求包一致
			shell->cmd=NEW_RESPONSE;
			glRecLen=0;
			recLastSeq=0;
			fid=seek(glFd,0,SEEK_SET);
			if(0!=fid)
			{
				ScrClrLine(3,4);
				ScrPrint(0,3,0,"seek err%d",errno);
				shell->cmd = END_TASK_RESPONSE;
				shell->confirmSign = SERVICE_STOP;
				return recUnexpected;
			}
			ScrClrLine(2,2);
			ScrPrint(0,2,0,"sending open response");
			break;
		case SEND_REQUEST:
			if(((dataPoint[1]<<8) + dataPoint[2])==recLastSeq)
			{
				memset(repSend,0,sizeof(repSend));
				repSend[0]=0x02;
				repSend[1]=(glCount >> 8) & 0xFF;
				repSend[2]=glCount  & 0xFF;
				repSend[3]=0x00;
				repSend[4]=0x04;
				repSend[5]=SEND_RESPOSE;
				repSend[6]=shell->confirmSign;
				memcpy(repSend+7,shell->taskNo,2);
				crc=gen_crc(repSend+1,8);
				crcBuf[0]=(crc>>8) & 0xFF;
				crcBuf[1]=crc & 0xFF;
				memcpy(repSend+9,crcBuf,2);
				comRet=PortSends(COMPORT,(uchar*)repSend,11);
				if(comRet)
				{
					ScrClrLine(3,4);
					ScrPrint(0,3,0,"PortSends Ret:%d",comRet);
					return portSendFail;
				}
				return onePkgOverTime;
			}
			
			if(memcmp(dataPoint+6,shell->taskNo,2))  //如果被分配的任务号和0x81响应包不一致
			{
				ScrClrLine(3,4);
				ScrPrint(0,3,0,"send_req_unmatched taskNO:,%0x,%0x,%0x,%0x,%0x,%0x,%0x,%0x",dataPoint[5],dataPoint[6],dataPoint[7],dataPoint[8],dataPoint[9],dataPoint[10],dataPoint[11],dataPoint[12]);
				shell->cmd=END_TASK_RESPONSE;
				shell->confirmSign = UNMATCHED_TASKNO;
				return recUnexpected;
			}
			
			if(offset > ( ((shell->pkgMaxSize[0])<<8) + ((shell->pkgMaxSize[1]) & 0xFF) ) )//如果文件数据的最大字节数超过0x81响应包中规定的字节数
			{
				ScrClrLine(3,4);
				ScrPrint(0,3,0,"over maxPkgData:%d",(dataPoint[25]<<8)+(dataPoint[2] & 0xFF));
				shell->cmd=END_TASK_RESPONSE;
				shell->confirmSign = PACKAGE_OVER_LEN;
				return recUnexpected;
			}
			tempLen=(dataPoint[3] << 8) + dataPoint[4] - 3;
			fid=write(glFd,(uchar*)packet+8,tempLen);
			if(fid!=tempLen)
			{
				ScrClrLine(3,4);
				ScrPrint(0,3,0,"write err,pack_len=%d,fid=%d,temp=%d",*pack_len,fid,tempLen);
				shell->cmd = END_TASK_RESPONSE;
				shell->confirmSign = SERVICE_STOP;
				return recUnexpected;
			}
			*pack_len = tempLen;
			glRecLen+=*pack_len;
			glCount=(dataPoint[1]<<8) + dataPoint[2];
			shell->cmd=SEND_RESPOSE;
			ScrClrLine(5,5);
			ScrPrint(0,5,0,"sending response%d",glCount+1);
			glCount++;
			recLastLen=*pack_len;
			recLastSeq=(dataPoint[1]<<8) + dataPoint[2];
			
			break;
		case END_TASK_REQUEST:
			if(memcmp(dataPoint+6,shell->taskNo,2))  //如果被分配的任务号和0x81响应包不一致
			{
				ScrClrLine(3,4);
				ScrPrint(0,3,0,"endTask unmatched taskNO:%d",(dataPoint[1]<<8)+(dataPoint[2] & 0xFF));
				shell->cmd = END_TASK_RESPONSE;
				shell->confirmSign = UNMATCHED_TASKNO;
				return recUnexpected;
			}
			shell->cmd=END_TASK_RESPONSE;
			shell->confirmSign = CONFRIM;//CONFRIM;
			ScrClrLine(5,5);
			switch(dataPoint[8])
			{
			case endOk:
				//ScrPrint(0,5,0,"正常结束");
				ScrPrint(0,5,0,"END SUCCESS");
				break;
			case useCancel:
				//ScrPrint(0,5,0,"用户强制取消");
				ScrPrint(0,5,0,"CANCLE BY USER");
				close(glFd);
				break;
			case overTime:
				//ScrPrint(0,5,0,"用户端超时");
				ScrPrint(0,5,0,"USER SIDE TIME OUT");
				close(glFd);
				break;
			case SystemError:
				//ScrPrint(0,5,0,"不可恢复系统错误");
				ScrPrint(0,5,0,"UNRECOVERY SYS ERR");
				close(glFd);
				break;
			default:
				//ScrPrint(0,5,0,"不可知结束码");
				ScrPrint(0,5,0,"UNKNOWN END CODE");
				close(glFd);
			}
			return dataPoint[8];  //返回客户端的结束码
		default:
			ScrClrLine(5,5);
			ScrPrint(0,5,0,"Not Vaild command");
			shell->cmd = END_TASK_RESPONSE;
			shell->confirmSign = SERVICE_STOP;
			return recUnexpected;
	}
	return 0x10;
}

uchar SendRecvData()
{
	char bufRec[3000],outputBuf[100],allRecBuf[200000];
    uint ret,comRet;
	ushort outputLen,getCRC;
	struct shellData shell;
	uint fid,recLen,rNum;
	uchar crcBuf[2];
	uchar buf[10];
	int errFlag=0;

	memset(&shell,0,sizeof(struct shellData));
	ret=initShell(&shell);
	while(1)
    {
		memset(bufRec,0,sizeof(bufRec));
		memset(outputBuf,0,sizeof(outputBuf));
		outputLen=0;
		ret=rcv_packet(&shell,bufRec,&recLen);
		switch(ret)
		{
			case recUnexpected:
			case waitAllpkgOverTime:
			case useCancel:
			case overTime:
			case SystemError:
				errFlag=1;
				break;
			case onePkgOverTime:
				continue;
			case recOk:   //最后一次返回ok
				ScrClrLine(6,6);
				ScrPrint(0,6,0,"sending last response%d",glCount+1);
				memset(allRecBuf,0,sizeof(allRecBuf));
				fid=seek(glFd,0,SEEK_SET);
				if(0!=fid)
				{
					ScrClrLine(3,4);
					ScrPrint(0,3,0,"last seek err%d",errno);
					shell.cmd = END_TASK_RESPONSE;
				    shell.confirmSign = SERVICE_STOP;
					break;
					//return recUnexpected;
				}
				rNum=read(glFd,allRecBuf,glRecLen);
				if(rNum!=glRecLen)
				{
					ScrClrLine(3,4);
					ScrPrint(0,3,0,"last readData err%d,%d",rNum,glRecLen);
					shell.cmd = END_TASK_RESPONSE;
				    shell.confirmSign = SERVICE_STOP;
					break;
					//return recUnexpected;  
				}
				close(glFd);
				getCRC=gen_crc(allRecBuf,glRecLen);
				crcBuf[0]=(getCRC >> 8) & 0xFF;
				crcBuf[1]=(getCRC) & 0xFF;
				if(getCRC != ((shell.fileCRC[0] << 8 ) + shell.fileCRC[1]))
				{
					ScrClrLine(3,4);
					ScrPrint(0,3,0,"END CRC unMatched %0X,%0X,%0X,%0X",crcBuf[0],crcBuf[1],shell.fileCRC[0],shell.fileCRC[1]);
					ScrPrint(0,7,0,"recLen=%d",glRecLen);
					shell.cmd=END_TASK_RESPONSE;
				    shell.confirmSign = ERROR_CRC;
					errFlag=1;
					//ret = recUnexpected;
					break;
				}
				break;
			default:
				break;
		}
		pack_up(&shell,outputBuf,&outputLen);
		if(0x00==PortTxPoolCheck(COMPORT))
		{
		//DelayMs(500);
			ScrClrLine(2,2);
			ScrPrint(0,2,0,"sending%d,%d",glCount+1,outputLen);
			comRet=PortSends(COMPORT,(uchar*)outputBuf,outputLen);
			if(comRet)
			{
				ScrClrLine(3,4);
				ScrPrint(0,3,0,"PortSends Ret:%d",comRet);
				return portSendFail;
			}
#ifdef DEBUG_COM
	sprintf(buf,"%s","send:");
	PortSends(3,buf,strlen(buf));/*send to pinpad port for debug*/
	PrintCom(outputBuf,outputLen);
#endif
		}
		if(1==errFlag)
		{
			errFlag = 0;
			ScrClrLine(7,7);
			ScrPrint(0,7,0,"Fail,confirm_sign=%d",shell.confirmSign);
			getkey();
			return ret;
		}
		if(ret==recOk)  //if endCode is recOk ,disply communication success
		{
			ScrClrLine(2,2);
			ScrPrint(0,2,0,"communication success");
			Beep(); /* 嘀 */DelayMs(20); /* 20ms的停顿 */ 
			Beep();DelayMs(20);Beep();
			getkey();
			return 0;
		}
    }
	return 0;
}

int event_main(ST_EVENT_MSG *msg)
{
	return 0;
}

int main(void)
{
    uchar ret;
	uint comRet;

	SystemInit();
#if 1
	PortOpen(DEBUGPORT,"115200,8,n,1");
#endif
	glFd=open("data",O_CREATE | O_RDWR);
	if(glFd<0)
	{
		ScrClrLine(3,4);
		ScrPrint(0,3,0,"open data_file err%d",errno);
		return 1;
	}
	comRet=PortOpen(COMPORT,"115200,8,n,1");
	if(comRet)
    {
		ScrClrLine(3,4);
		ScrPrint(0,3,0,"PortOpen err0X%02X",comRet);
		return 1;
    }
	ScrCls();
	ScrPrint(0,0,0x01|0x80,"     SERVER     ");
	ScrPrint(0,2,0,"WATIING...");
	ret=SendRecvData();
	glCount=0;
	recLastSeq=0;
	PortClose(COMPORT);
	PortClose(DEBUGPORT);
	close(glFd);
	if(ret) getkey();//如果返回异常
	return ret;
}
