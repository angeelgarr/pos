#include <posapi.h>

int glCount=0;
uint glRecCount=0,array[100];
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
	char *tmpPoint;

#ifdef DEBUG
	int comRet;
	uchar temp_test[400000]={0};	
#endif
	if(in_data==NULL || *data_len==0 || out_data==NULL)
	{
		ScrClrLine(0,1);
		ScrPrint(0,1,1,"param err\n");
		return ;
	}
	tmpPoint=out_data;
	dataLen=*data_len;
	dataPoint=tmpPoint;
	stx=0x02;
	out_data[0]=stx;
	out_data[1]=(glCount>>8) & 0xFF;//store as bigEndian
	out_data[2]=glCount & 0xFF;
	dataPoint+=3;
	memcpy(dataPoint,in_data,dataLen);
	crc=get_crc16_short(dataPoint-2,dataLen+2);
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
	uchar getData[10000];
	uchar *dataPoint;
	int offset,i;
   // uchar temp_test[20000];

	memset(getData,0,sizeof(getData));
	dataPoint=getData;
	offset=0;
	TimerSet(0,300);
    for(i=0;i<array[glRecCount];i++)
	{
		if(!TimerCheck(0))
		{
			ScrClrLine(0,1);
			ScrPrint(0,0,1,"COM timeout\n");
			getkey();
			ScrClrLine(0,1);
			ScrPrint(0,0,1,"SENDING DATA...");
			return 2;
		}
		comRet=PortRecv(11,&dataPoint[offset++],1000);
		if(comRet)return 2;
	}
	memcpy(packet,getData,array[glRecCount]);
	*pack_len=array[glRecCount];
	glRecCount++;
	return 0;
}

int event_main(ST_EVENT_MSG *msg)
{
	return 0;
}

int main(void)
{
    uchar bufSend[200000],outputBuf[200000],ucKey;
	uchar bufAllRec[200000];uchar bufRec[10000];
	uint allOutPutLen;
    uint i,k,j,count,ret,comRet;
	ushort getCRC;uchar bufCRC[2];
	ushort outputLen,lenSend;
	//uchar test_buf[40000];

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
		while(count<1)
		{
		memset(bufSend,0,sizeof(bufSend));
		memset(outputBuf,0,sizeof(outputBuf));
		for(i=0;i<8000;i++) //5000*8
		PciGetRandom(bufSend+i*8);
		//memcpy(bufSend,"\x00\x00\x56\x78",4);
		lenSend=i<<3;
		pack_up(bufSend,&lenSend,outputBuf);
		ScrClrLine(0,1);
		ScrPrint(0,0,1,"SENDING DATA...");
		if(lenSend<=8192) 
		{
			j=1;
			array[0]=lenSend;
		}
		else
		{
			if(lenSend%8192==0) 
			{
					j=lenSend/8192;
					for(i=0;i<j;i++) array[i]=8192;
			}
			else 
			{
				j=lenSend/8192+1;
				for(i=0;i<j-1;i++)array[i]=8192;
				array[i]=lenSend-8192*i;
			}
		}
repeat: 
		memset(bufAllRec,0,sizeof(bufAllRec));
		allOutPutLen=0;
		comRet=PortOpen(11,"115200,8,n,1");
		if(comRet)
	{
		ScrClrLine(0,1);
	    ScrPrint(0,0,1,"PortOpen err0X%02X\n",comRet);
		getkey();
		return 1;
	}
	    for(k=0;k<j;k++)
		{
			memset(bufRec,0,sizeof(bufRec));
			for(i=0;i<array[k];i++)
			{
				comRet=PortSend(11,outputBuf[i+k*8192]);
				if(comRet)
				{
// 					ScrClrLine(0,1);
// 					ScrPrint(0,0,1,"PortSend err comRet:%d",comRet);
					goto repeat;
				
				}
			} 
			ret=rcv_packet(bufRec,&outputLen);
			switch(ret)
			{
			 case 0: 
				  memcpy(bufAllRec+allOutPutLen,bufRec,outputLen);
				  allOutPutLen+=outputLen;
				  break;
			 case 2:                   //if over time or plug off line ,send and rec again
					PortClose(11);
					goto repeat;
			}
		}
//	ScrClrLine(0,1);
//	ScrPrint(0,0,1,"len=%d.output=%d,allOut=%d",lenSend,outputLen,allOutPutLen);
//	getkey();

	getCRC=get_crc16_short(&bufAllRec[1],allOutPutLen-3);  //verify as crc value
	bufCRC[0]=(getCRC>>8) & 0xFF;
	bufCRC[1]=getCRC & 0xFF;
	if(bufAllRec[0]!=0x02 || strncmp(bufCRC,&bufAllRec[allOutPutLen-2],2))
	{
		ScrClrLine(0,1);
		ScrPrint(0,0,1,"crc diff");
		getkey();
		return 1;
	}
	for(i=0;i<lenSend;i++)
	{
		if(outputBuf[i]!=bufAllRec[i])
		{
			ScrClrLine(0,1);
			ScrPrint(0,0,1,"DATA DIFF:i=%d",i);
			getkey();
			return 1;
		}
		
	}
		count++;
		}//while
		ScrClrLine(0,3);
		ScrPrint(0,0,1,"SAME DATA");
		PortClose(11);
		glRecCount=0;
		glCount=0;
		getkey();
		}//if(ucKey==KEY1 || ucKey==KEYENTER)
		if(ucKey==KEYCANCEL) return 1;
	}
	return 0;
}
