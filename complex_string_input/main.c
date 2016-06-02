
#include <posapi.h>


#define TIMENO 1
int i,cursorFlag=1;

const APPINFO AppInfo={
	"string_input",
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

uchar get_key(unsigned int timeout);
void DrawChar(int startX,int startY,int showSwitch);
uchar input_string(const char *prompt, char *out_str);
uchar OutputUpperNexKey(uchar keyvalue);
uchar OutputLowerNexKey(uchar keyvalue);

uchar OutputUpperNexKey(uchar keyvalue)
{
    uchar i;
    uchar *temp;
	uchar *keyCollect[]=
	{
		(uchar*)"1QZ.",
		(uchar*)"2ABC",
		(uchar*)"3DEF",
		(uchar*)"4GHI",
		(uchar*)"5JKL",
		(uchar*)"6MNO",
		(uchar*)"7PRS",
		(uchar*)"8TUV",
		(uchar*)"9WXY",
		(uchar*)"0,*#",
	};

	for(i = 0;i<10;i++)
	{
	    temp=keyCollect[i];
		while(*keyCollect[i]!=0)
		{
		if(keyvalue == *keyCollect[i]++)
			{
				keyvalue = *keyCollect[i];
				if(keyvalue == 0)
				{
					keyvalue = *temp;
				}
				i=10;
				break;

			}
		}
	}
   
	return keyvalue;
}

uchar OutputLowerNexKey(uchar keyvalue)
{
    uchar i;
    uchar *temp;
	uchar *keyCollect[]=
	{
		(uchar*)"1qz.",
		(uchar*)"2abc",
		(uchar*)"3def",
		(uchar*)"4ghi",
		(uchar*)"5jkl",
		(uchar*)"6mno",
		(uchar*)"7prs",
		(uchar*)"8tuv",
		(uchar*)"9wxy",
		(uchar*)"0,*#",
	};

	for(i = 0;i<10;i++)
	{
	    temp=keyCollect[i];
		while(*keyCollect[i]!=0)
		{
		if(keyvalue == *keyCollect[i]++)
			{
				keyvalue = *keyCollect[i];
				if(keyvalue == 0)
				{
					keyvalue = *temp;
				}
				i=10;
				break;

			}
		}
	}
   
	return keyvalue;
}
void DrawChar(int startX,int startY,int showSwitch)
{
	int x;

    for(x=0;x<8;x++)
		if((0x01<<x) & 0xFF)
		 ScrPlot(startX+7-x,startY,showSwitch);
}

uchar get_key(unsigned int timeout)
{
	uchar key;
	int cursorFlag=1;
	
	if(0==timeout) return getkey();
	if(1==cursorFlag) DrawChar(120,63,1);
	TimerSet(TIMENO,timeout*10);
	while(1)
	{
		if(0==kbhit())
		{
			key=getkey();
			return key;
		}
		if(!TimerCheck(TIMENO))
		{
            return 0xFE;
		}
			if(!TimerCheck(TIMENO+2))
		{
				DrawChar(120,63,0);; 
				if(1==cursorFlag) DrawChar(120,63,1);
				cursorFlag*=-1;
				TimerSet(TIMENO+2,5);
		}
	  
	}
}

 uchar input_string(const char *prompt, char *out_str)
 {
	char *digit_str=NULL;
	uchar uKey;
	int isUpper=1;
	int pressClear=0;
	uchar keyTemp,keyTemp1,j=1;
	
	digit_str=out_str;
	ScrCls();
	if(NULL==prompt || NULL==out_str) 
	{
		ScrPrint(0,0,0x01,"param error");
		Beef(6,700);
		return 1;
	}
	ScrPrint(118,0,0x01|0x80,"%c",'A');
   ScrPrint(0,3,1,prompt);
    while(1)
	{
    uKey=get_key(30);
    
	switch(uKey)
	{
	case KEY0:
	case KEY1:
	case KEY2:
	case KEY3:
	case KEY4:
	case KEY5:
	case KEY6:
	case KEY7:
	case KEY8:
	case KEY9:
	case KEYDOWN:
		if(i>=30)
		{
			Beef(6,700);
			break;
		}

		if(uKey==KEYDOWN)
			{
				if(digit_str[i]!=0)
				i++;
				digit_str[i]=' ';
				j=1;
				digit_str[i+1]=0x00;
				break;
			}

		
		if(1==isUpper)
		{
	
			if(uKey==keyTemp1 && j==2)
			{
			keyTemp=OutputUpperNexKey(keyTemp);
			//ScrPrint(119,6,0x01|0x80,"%c",keyTemp);
			}
			if(uKey!=keyTemp1 && 2==j)
			{
				 i++;
				keyTemp=uKey;
				j=1;
			 }	     
		}
	
		
		if(-1==isUpper)
		{
			if(uKey==keyTemp1 && j==2)
			keyTemp=OutputLowerNexKey(keyTemp);
			if(uKey!=keyTemp1 && 2==j)
			 {
				 i++;
				keyTemp=uKey;
				 j=1;
			 }	     
	
		}

		if(j==1)         /*记录每次按下的第一个键*/
		{ 
		keyTemp1=uKey;
		keyTemp=uKey;
		j=2;
			if(digit_str[i]==' ' ||(digit_str[i]!=0 && pressClear==1))
				{
					i++;
					pressClear=0;
				}
		}
		
		digit_str[i]=keyTemp;
		digit_str[i+1]=0x00;
		break;
	case KEYALPHA:
		isUpper*=-1;
		ScrClrLine(0,1);
		if(1==isUpper)ScrPrint(118,0,0x01|0x80,"%c",'A');
		if(-1==isUpper)ScrPrint(118,0,0x01|0x80,"%c",'a');
		break;
	case 0xFE:
		ScrPrint(0,0,0x01,"input time out");
		Beef(6,700);
		return 2;
	case KEYENTER:
		if(i>30)
		{
			Beef(6,700);
			break;
		}
	   else
	   { 
		   ScrCls();
		   digit_str[i+1]=0x00;
           ScrPrint(2,0,0x01,"%s",digit_str);
		   i=0;
		   memset(digit_str,0,sizeof(digit_str));
		   DelayMs(3000);
		   return 0;
	   }
	case KEYCANCEL:
		i=0;
		memset(digit_str,0,sizeof(digit_str));
		return 1;
	case KEYCLEAR:
		if(i<0) {
			i=0;
			Beef(6,700);
		}
		else
		{
		digit_str[i]=0x00;
		pressClear=1;
		j=1;   /*让键盘重新接收*/
		ScrClrLine(6,7);
		if(i<=15)
		{
			ScrPrint(0,6,1,"%16s",digit_str);
			
		}
	    else ScrPrint(0,6,1,"%16s",digit_str+i-15);
	   	--i;
		}
	 continue;
	default:  
		Beef(6,700);
		}
	ScrClrLine(6,7);
   if(i<=15) 
   {
	   ScrPrint(0,6,1,"%16s",digit_str);
	   if(digit_str[i]!=' ')
	   ScrPrint(119,6,0x01|0x80,"%c",digit_str[i]);
   }
    else 
	{
	ScrPrint(0,6,1,"%16s",digit_str+i-15);
	if(digit_str[i]!=' ')
	ScrPrint(119,6,0x01|0x80,"%c",digit_str[i]);
	}
   kbflush();
	}
	return 0;
}

int event_main(ST_EVENT_MSG *msg)
{
	return 0;
}


int main(void)
{
    uchar BUFSIZ[300]={0},ucKey;
	
	SystemInit();
	while(1)
	{
		ScrCls();
		kbflush();
		ScrPrint(0,0,0x01|0x80,"1.KEY INPUT     ");

	do {
		ucKey=getkey();
	} while(ucKey!=KEY1 && ucKey!=KEYENTER &&ucKey!=KEYCANCEL);
	if(ucKey==KEY1 || ucKey==KEYENTER)
	input_string("input please:",BUFSIZ);
	if(ucKey==KEYCANCEL) return 1;
	}
	return 0;
}
