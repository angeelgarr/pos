#include <posapi.h>

#define COMPORT 0
int glCount=0;
int glFd=0;
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
//#ifdef DEBUG
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
//#endif
ushort get_crc16_short(const uchar *data_block,int data_len)
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

void pack_up(const char *in_data,ushort *data_len,char *out_data)
{
	int stx;
	char *dataPoint;
	ushort crc,dataLen;
	uchar crcBuf[2];

#ifdef DEBUG
	int comRet;
	uchar temp_test[400000]={0};
#endif
	if(in_data==NULL || *data_len==0 || out_data==NULL)
	{
		ScrClrLine(0,3);
		ScrPrint(0,1,1,"param ertempLen+7r\n");
		return ;
	}
	dataLen=*data_len;
	dataPoint=out_data;
	stx=0x02;
	out_data[0]=stx;
	out_data[1]=(glCount>>8) & 0xFF;//store as bigEndian
	out_data[2]=glCount & 0xFF;
	out_data[3]=(dataLen>>8) & 0xFF;
	out_data[4]=dataLen & 0xFF;
    dataPoint+=5;
	memcpy(dataPoint,in_data,dataLen);
	crc=get_crc16_short(dataPoint-4,dataLen+4);
	dataPoint+=dataLen;
	crcBuf[0]=(crc>>8) & 0xFF;
	crcBuf[1]=crc & 0xFF;
	memcpy(dataPoint,crcBuf,2);
	dataPoint+=2;
	*data_len=(dataPoint-out_data);
	glCount++;

}

int rcv_packet(char *packet,ushort *pack_len)
{
	int comRet,iRet;
	uchar getData[3000];
	uchar *dataPoint;
	uint offset,tempLen;
    ushort getCRC;
    uchar bufCRC[2];

	memset(getData,0,sizeof(getData));
	dataPoint=getData;
	offset=0;
	TimerSet(0,300);
	while(1)
	{
		if(!TimerCheck(0))
		{
			ScrClrLine(0,3);
			ScrPrint(0,0,1,"COM timeout\n");
			getkey();
			//ScrClrLine(0,3);
			//ScrPrint(0,0,1,"SENDING DATA...");
			return 2;
		}
		comRet=PortRecv(COMPORT,&dataPoint[offset++],1000);
		if(comRet)return 2;
        tempLen=(dataPoint[3]<<8)+dataPoint[4];
		if(offset==tempLen+7) break;
		if(offset>2048)
		{
			ScrClrLine(0,3);
			ScrPrint(0,0,1,"OVER Arrange");
			//getkey();
			return 2;
		}
//		if(glRecCount==0)
//		ScrPrint(0,2,1,"0X%02X",dataPoint[0]);//屏幕查看发送情况
	}
	if(dataPoint[0]!=0X02)
		{ 
			ScrClrLine(0,3);
			ScrPrint(0,0,1,"dataPoint[0]!= 0X02");
			return 2;
		}
    getCRC=get_crc16_short(getData+1,offset-3);  //verify as crc value
	bufCRC[0]=(getCRC>>8) & 0xFF;
	bufCRC[1]=getCRC & 0xFF;
	if(bufCRC[0]!=getData[offset-2] || bufCRC[1]!=getData[offset-1])
	{
		ScrClrLine(0,3);
		ScrPrint(0,0,1,"CRC DIFF,%d",offset);
		return 2;
	}
	if((glCount-1)!=((getData[1]<<8)+getData[2]))
	{
		ScrClrLine(0,3);
		ScrPrint(0,0,1,"SEQ ERR,%d,%d",glCount-1,(getData[1]<<8)+getData[2]);
		iRet=rcv_packet(packet,pack_len);
		switch(iRet)
		{
		case 0:
			return 0;
		case 2:
			return 2;
		}
	}
	memcpy(packet,getData,offset);
	*pack_len=offset;
	write(glFd,packet+5,offset-7);
	return 0;
}
uchar GeneData(int fd)
{
	int i,fRet;
	uchar randomBuf[9];

	for(i=0;i<12800;i++)
		{
        memset(randomBuf,0,sizeof(randomBuf));
        PciGetRandom(randomBuf);
        fRet=write(fd,randomBuf,8);
        if(fRet!=8)
        {
            ScrClrLine(0,3);
            ScrPrint(0,0,1,"file write err%d",i);
            return 1;
        }
		}
	return 0;
}

uchar SendRecvData(int fd)
{
	uchar bufSend[3000],bufRec[3000],outputBuf[3000];
	uchar getAllRecv[10241];uchar getAllSend[10241];
    uint i,j,count,ret,comRet,lenSend;
	ushort outputLen,fRet;
	int fdRet;

	lenSend=102400;
	fdRet=seek(fd,0,SEEK_SET);
	if(fdRet<0)
    {
        ScrClrLine(0,3);
        ScrPrint(0,0,1,"open file errr%d",errno);
        return 1;
    }
	count=0;
	while(count<(lenSend/1024))
	{
	memset(bufSend,0,sizeof(bufSend));
	memset(outputBuf,0,sizeof(outputBuf));
    fRet=read(fd,bufSend,1024);
    if(fRet!=1024)
    {
        ScrClrLine(0,3);
        ScrPrint(0,0,1,"file read err8");
        return 1;
    }
	pack_up(bufSend,&fRet,outputBuf);
	//ScrClrLine(0,3);
	//ScrPrint(0,0,1,"SENDING DATA...");
repeat:
    comRet=PortSends(COMPORT,outputBuf,fRet);
    if(comRet)
    {
        ScrClrLine(0,1);            //如果线被拔掉，重发
        ScrPrint(0,0,1,"PortSends Ret:%d",comRet);
		ScrClrLine(2,3);
        ScrPrint(0,2,1,"sendind %d/100",count+1);
        goto repeat;

    }
	ScrClrLine(0,3);
    ScrPrint(0,0,1,"sending %d/100",count);
    memset(bufRec,0,sizeof(bufRec));
    outputLen=0;
    ret=rcv_packet(bufRec,&outputLen);
    switch(ret)
    {
        case 0:
            break;
        case 2:                   //if over time or plug off line ,send and rec again
            //ScrClrLine(0,3);
            //ScrPrint(0,0,1,"sending again");
            goto repeat;
    }
	count++;
	}//switch
	fd=open("send",O_CREATE|O_RDWR);
	if(fd<0)
	{
		ScrClrLine(0,3);
		ScrPrint(0,0,1,"open file errt%d",errno);
		return 1;
	}
	glFd=open("recv",O_CREATE|O_RDWR);
	if(glFd<0)
	{
		ScrClrLine(0,3);
		ScrPrint(0,0,1,"open recv err3%d",errno);
		return 1;
	}
	for(j=0;j<10;j++) //read 10 times to compare from files
	{
		memset(getAllSend,0,sizeof(getAllSend));
		memset(getAllRecv,0,sizeof(getAllRecv));
		fRet=read(fd,getAllSend,10240);
		if(fRet!=10240)
		{
			ScrClrLine(0,3);
			ScrPrint(0,0,1,"file read err2y");
			return 1;
		}
	
		fRet=read(glFd,getAllRecv,10240);
		if(fRet!=10240)
		{
			ScrClrLine(0,3);
			ScrPrint(0,0,1,"file read err3");
			return 1;
		}
		for(i=0;i<10240;i++) //compare 10kbs every time
		{
			if(getAllSend[i]!=getAllRecv[i])
			{
				ScrClrLine(0,3);
				ScrPrint(0,0,1,"DATA DIFF:i=%d",i);
				getkey();
				return 1;
			}
		}
	}
	ScrClrLine(0,3);
	ScrPrint(0,0,1,"SAME DATA");
	PortClose(COMPORT);glCount=0;
	close(fd);close(glFd);
	//remove("send");remove("recv");
	getkey();
	return 0;
}

int event_main(ST_EVENT_MSG *msg)
{
	return 0;
}

int main(void)
{
    uchar ucKey,ret;
	uint comRet;
	int fd;

	SystemInit();
    fd=open("send",O_CREATE|O_RDWR);
    if(fd<0)
    {
        ScrClrLine(0,3);
        ScrPrint(0,0,1,"open file err%d",errno);
        return 1;
    }
    glFd=open("recv",O_CREATE|O_RDWR);
    if(glFd<0)
    {
        ScrClrLine(0,3);
        ScrPrint(0,0,1,"open recv err%d",errno);
        return 1;
    }
	comRet=PortOpen(COMPORT,"115200,8,n,1");
	if(comRet)
    {
	ScrClrLine(0,3);
	ScrPrint(0,0,1,"PortOpen err0X%02X\n",comRet);
	getkey();
	return 1;
    }
	while(1)
	{
		ScrCls();
		kbflush();
		ScrPrint(0,0,0x01,"1.TEST COMM     ");
		ScrPrint(0,2,0x01,"2.CREATE FILE   ");
	do{
	   ucKey=getkey();
	} while(ucKey!=KEY1 && ucKey!=KEY2 && ucKey!=KEYENTER &&ucKey!=KEYCANCEL);
	switch(ucKey)
	{
	case KEY2:
		ScrClrLine(0,3);
		ScrPrint(0,0,0x01,"Creating File");
		ret=GeneData(fd);
		if(1==ret)return 1;
		break;
	case KEY1:
	case KEYENTER:
		ret=SendRecvData(fd);
		if(1==ret)return 1;
		break;//case KEY1 or KEYENTER
		case KEYCANCEL:
		      return 1;
	}//switch
	}//while(1)
	return 0;
}

