
#include <posapi.h>

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

ushort get_crc161(const uchar *data_block,int data_len)
{	
	 uchar i,flag_1=1;
	 int j;
	 ushort crc16=0;
	 ulong temp=0;
	
	 if(NULL==data_block)
	 {
		 Lcdprintf("Param Error\n");
		 return 1;
	 }
	
	
	for(j=0;j<data_len;j++)
	{
	 for(i=0x80;i!=0x00;i=i>>1)
	 {		
		 temp<<=1;
		 if(temp & 0x10000) 
		 {
			 temp=temp ^ 0x11021;
		 }
		 
		 if(*data_block & i)
		 {	
			 if(flag_1==1)  temp=0x10000 ^ 0x11021;
			 if(flag_1==0)  temp+=1;
			 flag_1=0;
		 }
		
	 }

	 data_block++;
	}
	crc16=temp;
	return crc16;
}

ushort get_crc16(const uchar *data_block,int data_len)
{	
	 uchar i;
	 int j;
	 ushort crc16;
	 ulong temp=0;

	 if(NULL==data_block)
	 {
		 Lcdprintf("Param Error\n");
		 return 1;
	 }
	for(j=0;j<data_len;j++)
	{
	 for(i=0x80;i!=0x00;i=i>>1)
	 {		
		   temp<<=1;
		 if(temp & 0x10000)  
			 temp=temp ^ 0x11021;

		 if(*data_block & i)
		 {
			 temp^=0x1021;
			
		 }
	 }

	 data_block++;
	}
	crc16=temp;
	return crc16;
}

int event_main(ST_EVENT_MSG *msg)
{
	return 0;
}


int main(void)
{
	uchar ucKey;
	ushort ret;

	uchar *data="\x12\x34\x56\x78";
	int len=strlen(data);

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
				ret=get_crc16(data,len);
				Lcdprintf("0x%x\n",ret);
				getkey();
		}
		if(ucKey==KEYCANCEL) return 1;
	}
	return 0;
}

