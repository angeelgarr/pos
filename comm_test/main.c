
#include <posapi.h>

int glCount=0;
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
	""
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

void pack_up(const char *in_data,ushort *data_len,char *out_data)
{
	int stx,comRet,i;
	char *dataPoint;
	ushort crc,dataLen;
	uchar crcBuf[2];
	char *tmpPoint;

#ifdef DEBUG
	uchar temp_test[200]={0};	
#endif
	if(in_data==NULL || *data_len==0 || out_data==NULL)
	{
		ScrClrLine(0,1);
		ScrPrint(0,1,1,"param err\n");
		return ;
	}
	tmpPoint=out_data;
	dataLen=*data_len;
//loop:
//while(1)
//{
	dataPoint=tmpPoint;
	stx=0x02;
	out_data[0]=stx;
	out_data[1]=(glCount>>8) & 0xFF;//store as bigEndian

	out_data[2]=glCount & 0xFF;
	dataPoint+=3;
	memcpy(dataPoint,in_data,dataLen);
	dataPoint+=dataLen;
	crc=get_crc16_short(in_data,dataLen);
	crcBuf[0]=(crc>>8) & 0xFF;
	crcBuf[1]=crc & 0xFF;
	memcpy(dataPoint,crcBuf,2);
	dataPoint+=2;
	*data_len=(dataPoint-out_data);
	comRet=PortOpen(0,"115200,8,n,1");
#ifdef DEBUG
	PrnInit();
	PrnStr("\n1.comRet=%d\n",comRet);
	PrnStart();
#endif
	if(comRet)
	{
		ScrClrLine(0,1);
		ScrPrint(0,1,1,"PortOpen err0X%02X\n",comRet);
		getkey();
		return;
	}

#ifdef DEBUG
	PrnInit();
	DatBcdToAsc(temp_test,out_data,(*data_len)*2);
	PrnStr("sendlen=%d :s\n",*data_len);
	PrnStr("%s\n",temp_test);
	PrnStart();
#endif

	for(i=0;i<*data_len;i++)
	{
		comRet=PortSend(0,out_data[i]);
	}

	glCount++;
	PortClose(0); 
//}//while

}

int rcv_packet(char *packet,ushort *pack_len)
{
	ushort packLen;
	int comRet,offset;
	uchar getData[100];
#ifdef DEBUG
	uchar temp_test[200]={0};
#endif
	offset=0;
	memset(getData,0,sizeof(getData));
	comRet=PortOpen(0,"115200,8,n,1");
    PortReset(0);
         	if(comRet)
			{
		       ScrClrLine(0,1);
	     	   ScrPrint(0,1,1,"rcv_PortOpen err0X%02X\n",comRet);
		       getkey();
		       return 1;
			}
	comRet=PortRecvs(0,getData,15,1000);
	if(comRet>0) PortClose(0);
	else 
	{
		glCount--;
		PortClose(0);
		return 2;
	}

#ifdef DEBUG
	PrnInit();
	DatBcdToAsc(temp_test,getData,comRet*2);
	PrnStr("offset=%d r:\n",comRet);
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
    uchar bufSend[100],ucKey;
	uchar bufRec[100];
	ushort lenSend,*outputLen;
    int i,count,ret;
	
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
		count=0;
		while(count<20)
		{
		memset(bufSend,0,sizeof(bufSend));
		memset(bufRec,0,sizeof(bufRec));
//		for(i=0;i<100;i++)
		PciGetRandom(bufSend);;
		//memcpy(bufSend,"\x00\x00\x56\x78",4);
		lenSend=8;
		TimerSet(0,300);
repeat: 
		pack_up(bufSend,&lenSend,bufRec);
		ret=rcv_packet(bufRec,&outputLen);
		if(!TimerCheck(0))
		{
			ScrClrLine(0,1);
			ScrPrint(0,1,1,"comm timeout\n");
			getkey();
			return 1;
		}
		if(ret==1) break;
		if(ret==2) 
		{	
			memset(bufRec,0,sizeof(bufRec));
			lenSend=8;
			goto repeat;
		}
		count++;
		}//while
	}
	if(ucKey==KEYCANCEL) return 1;
	}
	return 0;
}
