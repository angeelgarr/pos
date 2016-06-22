
#include <posapi.h>
#define DEBUG

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
	""comRet=PortOpen(0,"115200,8,n,1");
	if(comRet)
	{
		ScrClrLine(0,1);
		ScrPrint(0,1,1,"rcv_PortOpen err0X%02X\n",comRet);
		return 1;
	}
};
#ifdef DEBUG
#define byte uchar
#define word ushort
void DatBcdToAsc(byte *Asc, byte *Bcd, word Asc_len)
{
	/*~~~~~~~~~~~~~*/
	byte	is_first;
	byte	by;
	/*~~~~~~~~~~~~~*/

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

int pack_up(const char *in_data,ushort *data_len,char *out_data)
{
	int stx,comRet,i;
	static int seq=0;
	char *dataPoint;
	ushort crc,dataLen;
	uchar crcBuf[2];
	char *tmpPoint;

#ifdef DEBUG
	uchar temp_test[600]={0};	
#endif
	if(in_data==NULL || *data_len==0 || out_data==NULL)
	{
		ScrClrLine(0,1);
		ScrPrint(0,1,1,"param err\n");
		return 1;
	}
	tmpPoint=out_data;
	dataLen=*data_len;

	dataPoint=tmpPoint;
	stx=0x02;
	out_data[0]=stx;
	out_data[1]=(seq>>8) & 0xFF;//store as bigEndian
#ifdef DEBUG
	out_data[1]=0;
#endif
	
	out_data[2]=seq & 0xFF;
	dataPoint+=3;
	memcpy(dataPoint,in_data,dataLen);
	dataPoint+=dataLen;
	crc=get_crc16_short(in_data,dataLen);
	crcBuf[0]=(crc>>8) & 0xFF;
	crcBuf[1]=crc & 0xFF;
	memcpy(dataPoint,crcBuf,2);
	dataPoint+=2;
	*data_len=(dataPoint-out_data);
#ifdef DEBUG
	PrnInit();
	DatBcdToAsc(temp_test,out_data,(*data_len)*2);
	PrnStr("%s\n",temp_test);
	PrnStart();
#endif

	
	comRet=PortOpen(0,"115200,8,n,1");
	if(comRet)
	{
		ScrClrLine(0,1);
		ScrPrint(0,1,1,"PortOpen err0X%02X\n",comRet);
		return 1;
	}
	for(i=0;i<*data_len;i++)
	{
		comRet=PortSend(0,out_data[i]);
		if(comRet)
		{
			ScrClrLine(0,1);
			ScrPrint(0,1,1,"PortSend err\n",comRet);
			return 1;
		}
	}
	seq++;
	PortClose(0);

}

int rcv_packet(char *packet,ushort *pack_len)
{
	ushort packLen;
	int comRet,offset;
	uchar getData[300];
#ifdef DEBUG
	uchar temp_test[600]={0};
	
#endif
	offset=0;
	
	TimerSet(0,30);
	while(1)
	{
	          comRet=PortOpen(0,"115200,8,n,1");
         	if(comRet)
			{
		       ScrClrLine(0,1);
	     	   ScrPrint(0,1,1,"rcv_PortOpen err0X%02X\n",comRet);
		       getkey();
		       return 1;
	}
			comRet=PortRecv(0,&getData[offset++],1000);
			if(comRet==0xff)
		{
			ScrClrLine(0,1);
			ScrPrint(0,1,1,"recdata timeout\n",comRet);
			getkey();
			return 0xff;
		}
		else if(comRet!=0) break;
		if(!TimerCheck(0))
		{
			ScrClrLine(0,1);
			ScrPrint(0,1,1,"comm timeout\n",comRet);
			getkey();
			return 2;
		}
		PortClose(0);
	}
#ifdef DEBUG
	PrnInit();
	DatBcdToAsc(temp_test,getData,offset*2);
	PrnStr("%s\n",rcv_packet);
	PrnStr("%s\n",temp_test);
	PrnStart();
#endif
	return 0;
}
int event_main(ST_EVENT_MSG *msg)
{
	return 0;
}
int main(void)
{
    uchar bufSend[300],ucKey;
	uchar bufRec[300];
	ushort lenSend;
    int i;
	
	SystemInit();
	while(1)
	{
		ScrCls();
		kbflush();
		ScrPrint(0,0,0x01|0x80,"1.TEST COMM     ");

	do {
		ucKey=getkey();
	} while(ucKey!=KEY1 && ucKey!=KEYENTER &&ucKey!=KEYCANCEL);
	if(ucKey==KEY1 || ucKey==KEYENTER)
	{
		i=0;
		
		while(i<2)
		{
		memset(bufSend,0,sizeof(bufSend));
		memset(bufRec,0,sizeof(bufRec));
		PciGetRandom(bufSend);
		//strcpy(bufSend,"\x12\x34\x56\x78");
		lenSend=strlen(bufSend);
		
		pack_up(bufSend,&lenSend,bufRec);
		rcv_packet(bufRec,&lenSend);
      
		i++;
		}
	}

   
	if(ucKey==KEYCANCEL) return 1;
	}
	return 0;
}
