#include<posapi.h>

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


ushort get_crc(const char *buf,ushort len)
{
    uchar leftmost_bits;
    long i,j,tmpl,dividend,total_bits;
	char tmps[200];
	
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
		if((dividend & 0x10000))
		{
			//--search for the first 1 bit
// 			for(j=0;j<17;j++)
// 			{
// 				tmpl=dividend & 0x10000>>j;
// 				if(tmpl)break;
// 			}
// 			leftmost_bits=17-j;
// 			continue;
			dividend^=0x11021;
		}
		
		//4--perform XOR between the dividend and the divider
		//sprintf(tmps,"i:%d,DV0:%X",i,dividend);
		//AfxMessageBox(tmps);
	/*	dividend^=0x11021;*/
		
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

ushort get_crc16(const uchar *data_block,int data_len)
{
  long totalBit,i,j,crc16;
  int rightNumber;

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
int main(void)
{
	uchar ucKey;
	ushort tmpu1;/*,tmpu2;*/
	
	// tmpu2=cal_crc("\x12\x34\x56\x78",4,0);//cal_crc是采用查表算法实现的已有函数
	
	/*tmpu2=cal_crc("\x12\x34\x56\x78\x90",5,0);*/
	//tmpu1=get_crc("\x01",1);
	//tmpu2=cal_crc("\x01",1,0);
	//tmpu1=get_crc("\x80",1);
	//tmpu2=cal_crc("\x80",1,0);


	
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
	tmpu1=get_crc16_short("\x12\x34\x56\x78",4);
	//	tmpu1=get_crc16_1("\x12",1);
			Lcdprintf("0x%x\n",tmpu1);

			tmpu1=get_crc16_short("\x12\x34\x56\x78\x90",5);
			DelayMs(1000);
			ScrCls();
			Lcdprintf("0x%x\n",tmpu1);
			getkey();
		}
		if(ucKey==KEYCANCEL) return 1;
	}
	return 0;
}
