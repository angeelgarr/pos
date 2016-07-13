#include <posapi.h>

#define COMPORT 0
int glCount=0;
int glFd=0;
//#define DEBUG

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
		ScrPrint(0,1,1,"PARA ERR");
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
	return 0;
}

uchar SendRecvData(void)
{
	uchar bufRec[3000],outputBuf[3000];
    uint ret,comRet;
	ushort outputLen;

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
            return 2;
        default:
            return 1;
    }
	pack_up(bufRec,&outputLen,outputBuf);
    comRet=PortSends(COMPORT,outputBuf,outputLen);
    if(comRet)
    {
        ScrClrLine(0,1);
        ScrPrint(0,0,1,"PortSends Ret:%d",comRet);
        return 2;
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
    uchar ucKey,ret;
	uint comRet;

	SystemInit();
	comRet=PortOpen(COMPORT,"115200,8,n,1");
	if(comRet)
    {
	ScrClrLine(0,3);
	ScrPrint(0,0,1,"PortOpen err0X%02X\n",comRet);
	return 1;
    }
	while(1)
	{
		ScrCls();
		kbflush();
		ScrPrint(0,0,0x01,"1.COM SERVER    ");
	do{

	   ucKey=getkey();

	}  while(ucKey!=KEY1 && ucKey!=KEYENTER &&ucKey!=KEYCANCEL);
	switch(ucKey)
	{
	case KEY1:
	case KEYENTER:
		ret=SendRecvData();
		return ret;
		case KEYCANCEL:
		      return 1;
	}
	}//while(1)
	return 0;
}
