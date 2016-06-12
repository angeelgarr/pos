#include<posapi.h>
//#define DEBUG
const APPINFO AppInfo={
		"CRC-16",
		"APP-DEMO",
		"1.0",
		"XH",
		"demo program",
		"",
		0,
		0,
		0,
		""
};


ushort get_crc(const char *buf,uint len)
{
    uchar leftmost_bits;
    long i,j,tmpl,dividend,total_bits;
	char tmps[200];
	if(len>((0xffffffff>>3)-2))
  {
	  ScrPrint(0,6,1,"OVER LENGTH ERR");
	  return 1;
  }
    //1--calculate the total length in bits
    total_bits=(long)(len+2)<<3;
	
    dividend=0;
    leftmost_bits=0;
    i=0;
    while(i<total_bits)
    {
		//2--load bits into the dividend bit by bit till it reaches 17 bits
		for(j=leftmost_bits;j<17 && i<total_bits;j++,i++)
		{
			dividend<<=1;
			if(i<len<<3)//--the tail is appended with two zeroes
				dividend|=(buf[i/8]>>(7-i%8))&0x01;
		}
		//sprintf(tmps,"i:%d,j:%d,dv:%X",i,j,dividend);
		//AfxMessageBox(tmps);
		if(j<17)break;//stop if it has reached the end
		
		//3--ensure the D17 bit of the loaded dividend is 1
		if(!(dividend & 0x10000))
		{
			//--search for the first 1 bit
 			for(j=0;j<17;j++)
 			{
 				tmpl=dividend & 0x10000>>j;
 				if(tmpl)break;
 			}
 			leftmost_bits=17-j;
 			continue;
			
		}
		
		//4--perform XOR between the dividend and the divider
		//sprintf(tmps,"i:%d,DV0:%X",i,dividend);
		//AfxMessageBox(tmps);
		dividend^=0x11021;
		
		//--search for the first 1 bit
		for(j=0;j<17;j++)
		{
			tmpl=dividend&(0x10000>>j);
			if(tmpl)break;
		}
		leftmost_bits=17-j;
		//sprintf(tmps,"i:%d,DV1:%X,left_bits:%d",i,dividend,leftmost_bits);
		//AfxMessageBox(tmps);
		
    }//while(1)
	
    return dividend;
}

ushort get_crc16(const uchar *data_block,uint data_len)
{
  long totalBit,i,j,crc16;
  int rightNumber;

  if(data_len>((0xffffffff>>3)-2))
  {
	  ScrPrint(0,6,1,"OVER LENGTH ERR");
	  return 1;
  }
  totalBit=(long)(data_len+2)<<3;
  rightNumber=0;
  crc16=0;
  i=0;
 while(i<totalBit)
 {

  for(j=rightNumber;j<17 && i<totalBit;i++,j++) //保证加载17位
  {
    crc16<<=1;
	if(i<data_len<<3)
	crc16|=(data_block[i/8]>>(7-i%8))&0x01;
  }
	if(j<17)break;
	if(crc16 & 0x10000) crc16^=0x11021;

	for(j=0;j<17;j++)   //从左找到第一位1
	{
	if(crc16 & 0x10000 >>j)
	break;
	}
	rightNumber=17-j;
 }//while()
 return crc16;
}


ushort get_crc16_short(const uchar *data_block,uint data_len)
{
  long totalBit,i,j;
  int rightNumber;
  ushort crc16;
  uchar flag;

  if(data_len>((0xffffffff>>3)-2))
  {
	  ScrPrint(0,6,1,"OVER LENGTH ERR");
	  return 1;
  }
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

int event_main(ST_EVENT_MSG *msg)
{
	return 0;
}
int main(void)
{
	uchar ucKey;
	ushort tmpu1,tmpu2,remTime,realTime,timeSum;
	int i;
	char *testBuf;
	uchar randBuf[1024*8];
	
    char *buf="\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13  \
		\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29  \
		\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f  \
		\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50\x51\x52\x53\x54\x55   \
		\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b   \
		\x6c\x6d\x6e\x6f\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\x7f\x80         \
		\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95     \
		\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa    \
		\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0   \
		\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7  \
		\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee  \
		\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xff" ;
	timeSum=0;
	SystemInit();
	while(1)
	{
		ScrCls();
		kbflush();
		ScrPrint(0,0,0x01|0x80,"CRC-16          ");
		do{
			ucKey=getkey();
		}while(ucKey!=KEY1 && ucKey!=KEYENTER && ucKey!=KEYCANCEL);
		
		if(ucKey==KEY1 || ucKey==KEYENTER)
		{	


			ScrCls();
			/******************************************************************************************
			长度遍历检查
			*******************************************************************************************/
			for(i=0;i<256;i++)
			{
				if((get_crc16_short(buf,i)!=get_crc(buf,i)) || (get_crc16(buf,i)!=get_crc(buf,i) ))
				{
					ScrPrint(0,0,0x01,"1.compare different\n");
					getkey();
				}
			}
			/******************************************************************************************
			内容遍历检查
			*******************************************************************************************/
				for(i=0;i<256;i++)
			{
				if((get_crc16_short(&buf[i],1)!=get_crc(&buf[i],1)) || (get_crc16(&buf[i],1)!=get_crc(&buf[i],1) ))
				{
					ScrPrint(0,0,0x01,"2.compare different\n");
					getkey();
				}
			}
			/******************************************************************************************
			随机数据测试
			*******************************************************************************************/
			    for(i=0;i<256;i++)
				{
					PciGetRandom(randBuf+i*8);
				}
#ifdef DEBUG
				
				randBuf[8*i]=0x00;
				ScrPrint(0,4,1,"%s",randBuf);
				getkey();
#endif
				if((get_crc16_short(randBuf,i*8)!=get_crc(randBuf,i*8)) || (get_crc16(randBuf,i*8)!=get_crc(randBuf,i*8) ))
				{
					ScrPrint(0,0,0x01,"3.compare different\n");
					getkey();
				}
             /******************************************************************************************
			边界测试和速度测试
			*******************************************************************************************/
	     testBuf=(char*)malloc(sizeof(char)*0xfffff);  // 不能malloc(sizeof(char)*0xffffffff)
		  if(NULL==testBuf) 
		   {
			   ScrPrint(0,6,0x01,"malloc error\n");
		   }
		 for(i=0;i<0xfffff;i++)
		 {
			 testBuf[i]=i%256;
		 }
		  
			ScrCls();

			tmpu1=get_crc16(testBuf,0);
			tmpu2=get_crc16(testBuf,(0xffffffff>>3)-2);
			ScrPrint(0,0,0x01,"1func:0x%X,0X%X\n",tmpu1,tmpu2);
			
			tmpu1=get_crc(testBuf,0);
			tmpu2=get_crc(testBuf,(0xffffffff>>3)-2);
			ScrPrint(0,2,0x01,"2func:0x%X,0X%X\n",tmpu1,tmpu2);

			tmpu1=get_crc16_short(testBuf,0);
			tmpu2=get_crc16_short(testBuf,(0xffffffff>>3)-2);
			ScrPrint(0,4,0x01,"3func:0x%X,0X%X\n",tmpu1,tmpu2);

			DelayMs(500);
			ScrCls();

			for(i=0;i<50;i++)
			{
			TimerSet(0,5000);
			tmpu1=get_crc(testBuf,0xfffff);
			remTime=TimerCheck(0);
			realTime=5000-remTime;
			timeSum+=realTime;
			}
			ScrPrint(0,0,1,"time=%ds",(timeSum/50)*100);
			timeSum=0;
		
			
            for(i=0;i<50;i++)
			{
			TimerSet(1,5000);
			tmpu1=get_crc16(testBuf,0xfffff);
			remTime=TimerCheck(1);
			realTime=5000-remTime;
			timeSum+=realTime;
			}
			ScrPrint(0,2,1,"time=%ds",(timeSum/50)*100);
			timeSum=0;


			for(i=0;i<50;i++)
			{
			TimerSet(2,5000);
			tmpu1=get_crc16_short(testBuf,0xfffff);
			remTime=TimerCheck(2);
			realTime=5000-remTime;
			timeSum+=realTime;
			}
			ScrPrint(0,4,1,"time=%ds",(timeSum/50)*100);
			timeSum=0;

			free(testBuf);
			testBuf=NULL;
							
			getkey();
		}
		if(ucKey==KEYCANCEL) return 1;
	}
	return 0;
}
