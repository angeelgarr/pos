#include <posapi.h>

#define COMPORT 0
int glCount=0;
int fd,fd1;
int noSave;

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
		ScrClrLine(2,3);
		ScrPrint(0,2,1,"PARA ERR %d",*data_len);
		return;
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
	int comRet;
	uchar getData[3000];
	uchar *dataPoint;
	uint offset,tempLen;
    ushort getCRC;
    uchar bufCRC[2];
	int fid,temp;

	memset(getData,0,sizeof(getData));
	dataPoint=getData;
	offset=0;
	TimerSet(0,300);
	while(1)
	{
		if(!TimerCheck(0))
		{
			ScrClrLine(2,3);
			ScrPrint(0,2,1,"COM timeout\n");
			getkey();
			return 2;
		}
		if(kbhit()==0 && getkey()==KEYF1)
		{
			close(fd1);
			if(0==remove("seq"))
			{
				ScrClrLine(0,3);
				ScrPrint(0,0,1," seq cleared ,restart pls");
				while(1);
			}
			else
			{
				ScrClrLine(0,3);
				ScrPrint(0,0,1," seq cleared err%d",errno);
				return 3;
			}
		}
		comRet=PortRecv(COMPORT,&dataPoint[offset++],1000);
		if(comRet)
		{
		ScrClrLine(0,1);
		ScrPrint(0,0,1,"waitting...%d",glCount);
		return 2;
		}
        tempLen=(dataPoint[3]<<8)+dataPoint[4];
		if(offset==tempLen+7) break;
		if(offset>2048)
		{
			ScrClrLine(2,3);
			ScrPrint(0,2,1,"OVER Arrange");
			//getkey();
			return 2;
		}
	}
	if(dataPoint[0]!=0X02)
		{
			ScrClrLine(2,3);
			ScrPrint(0,2,1,"dataPoint[0]!= 0X02");
			return 2;
		}
    getCRC=get_crc16_short(getData+1,offset-3);  //verify as crc value
	bufCRC[0]=(getCRC>>8) & 0xFF;
	bufCRC[1]=getCRC & 0xFF; 
	if(bufCRC[0]!=getData[offset-2] || bufCRC[1]!=getData[offset-1])
	{
		ScrClrLine(2,3);
		ScrPrint(0,2,1,"CRC DIFF,%d",offset);
		return 2;
	}
	noSave++;
	if(noSave>=3) noSave=3;
	switch(glCount-((getData[1]<<8)+getData[2]))
	{
	case 0:
	case -1:
		//glCount=(getData[1]<<8)+getData[2];
		memcpy(packet,getData+5,offset-7);
		*pack_len=offset-7;
		if(noSave==1)   //保证上一次没有保存好数据，开机glCount+1;
		{
			glCount++;
			noSave=2;
		}
		fid=seek(fd,0,SEEK_SET);
		if(0!=fid)
		{
			ScrClrLine(2,3);
			ScrPrint(0,2,1,"seek err%d",errno);
			return 3;
		}
		fid=write(fd,(uchar*)packet,*pack_len);
		if(fid!=*pack_len)
		{
			ScrClrLine(2,3);
			ScrPrint(0,2,1,"write err");
			return 3;
		}
	  break;
	case 1:
		/**
		fileLen=filesize("data");
		if(fileLen<0)
		{
			ScrClrLine(2,3);
			ScrPrint(0,2,1,"filesize err");
			return 3;
		}
		**/
		fid=seek(fd,0,SEEK_SET);
		if(0!=fid)
		{
		ScrClrLine(2,3);
		ScrPrint(0,2,1,"seek err%d",errno);
		return 3;
		}
		fid=read(fd,(uchar*)packet,1024);
		if(fid<0)
		{
			ScrClrLine(2,3);
			ScrPrint(0,2,1,"read err%d",1024);
			return 3;
		}
		*pack_len=1024;
		break;
	default:
		ScrClrLine(2,3);
		ScrPrint(0,2,1,"sep err%d",glCount);
		getkey();
		return 2;
	}//switch
	return 0;
}

uchar SendRecvData(void)
{
	char bufRec[3000],outputBuf[3000];
    uint ret,comRet;
	ushort outputLen;
	int fid,temp;

	while(1)
    {
    memset(bufRec,0,sizeof(bufRec));
    memset(outputBuf,0,sizeof(outputBuf));
    outputLen=0;
    ret=rcv_packet(bufRec,&outputLen);
    switch(ret)
    {
        case 0:
            break;
		case 2:
			continue;
        default:
            break;
    }
	pack_up(bufRec,&outputLen,outputBuf);
	if(0x00==PortTxPoolCheck(COMPORT))
	{
	//DelayMs(500);
	ScrClrLine(0,1);
	ScrPrint(0,0,1,"sending%d,%d",glCount,(outputBuf[3]<<8)+outputBuf[4]);
	if(glCount==100) glCount=0;
	
    comRet=PortSends(COMPORT,(uchar*)outputBuf,outputLen);
    if(comRet)
    {
        ScrClrLine(2,3);
        ScrPrint(0,2,1,"PortSends Ret:%d",comRet);
        return 2;
    }
	fid=seek(fd1,0,SEEK_SET);
	if(0!=fid)
	{
		ScrClrLine(2,3);
		ScrPrint(0,2,1,"seek fd1 err%d",errno);
		return 3;
	}
	temp=glCount-1;
	fid=write(fd1,&temp,1);
	if(fid!=1)
	{
		ScrClrLine(2,3);
		ScrPrint(0,2,1,"write fd1 err");
		return 3;
	}
	}
    }
	PortClose(COMPORT);
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
	int fid;

	SystemInit();
	fd=open("data",O_CREATE | O_RDWR);
	if(fd<0)
	{
		ScrClrLine(2,3);
		ScrPrint(0,2,1,"open file err%d",errno);
		return 1;
	}
	fd1=open("seq",O_CREATE | O_RDWR);
	if(fd1<0)
	{
		ScrClrLine(2,3);
		ScrPrint(0,2,1,"open file seq err%d",errno);
		return 1;
	}
	fid=read(fd1,&glCount,1);
	if(1!=fid ) glCount =0;
	comRet=PortOpen(COMPORT,"115200,8,n,1");
	if(comRet)
    {
	ScrClrLine(2,3);
	ScrPrint(0,2,1,"PortOpen err0X%02X\n",comRet);
	return 1;
    }
	ScrClrLine(0,1);
	ScrPrint(0,0,1,"waitting...");
	ret=SendRecvData();
	return ret;
}
