
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
	uchar uKey,j,temp;

	digit_str=out_str;
	ScrCls();
	if(NULL==prompt || NULL==out_str) 
	{
		ScrPrint(0,0,0x01,"param error");
		Beef(6,700);
		return 1;
	}
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
		digit_str[i]=uKey;
		digit_str[++i]=0x00;
		break;
	case 0xFE:
		ScrPrint(0,0,0x01,"input time out");
		Beef(6,700);
		return 2;
	case KEYENTER:
		if(i<1 || i>30)
		{
			Beef(6,700);
			break;
		}
	   else
	   { 
		   ScrCls();
		   digit_str[i]=0x00;
           ScrPrint(2,0,0x01,"%s",digit_str);
		   DelayMs(3000);
		   return 0;
	   }
	case KEYCANCEL:
		return 1;
	case KEYCLEAR:
		if(0==i) 	Beef(6,700);
		else
		{
		digit_str[i-1]=0x00;
		ScrClrLine(6,7);
		if(i<16)ScrPrint(0,6,1,"%16s",digit_str);
	    else ScrPrint(0,6,1,"%16s",digit_str+i-16);
		--i;
		}
		break;
	default:  
		Beef(6,700);
		}
	ScrClrLine(6,7);
   if(i<=16) ScrPrint(0,6,1,"%16s",digit_str);
    else 
	{
	ScrPrint(0,6,1,"%16s",digit_str+i-16);
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
	input_string("请输入:",BUFSIZ);
	if(ucKey==KEYCANCEL) return 1;
	}
	return 0;
}
