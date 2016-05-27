
#include <posapi.h>

#define TIMENO 1
int i;
uchar alphaFlag=0;
char count=4;

const APPINFO AppInfo={
	"key",
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
uchar input_digit(uchar min_len,uchar max_len,uchar *digit_str);

void error(void)
{
	Beef(6,700);
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
	if(1==cursorFlag) DrawChar(119,48,1);
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
				DrawChar(119,48,0);; 
				if(1==cursorFlag) DrawChar(119,48,1);
				cursorFlag*=-1;
				TimerSet(TIMENO+2,5);
		}
	  
	}
}


uchar get_amount(const char *prompt_str,ulong *amount)
{
	
	uchar uKey;
	int j;
	int k;
	char temp;
	double amt;
	char *digit_str;
	digit_str=prompt_str;
	
	ScrCls();
	ScrPrint(0,2,1,"请输入金额:");
	if(NULL==prompt_str) 
	{
		ScrPrint(0,0,0x01,"param error");
		error();
		return 1;
	}
 
    while(1)
	{
   
    uKey=get_key(30);
    alphaFlag=0;
	for(k=0;k<i;k++)
	{
		if(digit_str[k]=='.') 
		alphaFlag=1;
	}
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
		if(alphaFlag==0 && (i>=10 && uKey!=KEYALPHA)) /*当前十位未出现小数点，输入的第11位数不是小数点的时候提示报错*/
		{
			Beef(6,700);
			break;
		}
		if(digit_str[0]==KEY0 && i==1)   /*当第一位输入0以后，第二位不允许输入数字*/
		{
			error();
			break;
		}

		if(alphaFlag==1)                /*当输入小数以后后面只允许输入两位数字*/
				{
					temp=count--;
					if(temp<=2)
					{
						count=2;
						temp=2;
					}
					if(temp>2)
					{
						 digit_str[i]=uKey;
						 digit_str[++i]=0;
					}

				}
		else
				{
				digit_str[i]=uKey;
			    digit_str[++i]=0;
				}
		break;

	case KEYALPHA:
		if(i==0)Beef(6,700);
		
		if(i!=0 && alphaFlag==0) 
		{
			digit_str[i]='.';
			digit_str[++i]=0;
		}
		break;
	case 0xFE:
		ScrPrint(0,0,0x01,"input time out");
		Beef(6,700);
		return 2;
	case KEYENTER:
		if(i<1 || i>13)
		{
			Beef(6,700);
			break;
		}
	   else
	   { 
		   
		   ScrCls();
		   digit_str[i]=0x00;
		   amt=atof(digit_str);
		   *amount=(long)(amt*100);
		   memset(digit_str,0,sizeof(digit_str));
		   i=0;alphaFlag=0; count=4;
		   return 0;
	   }
	case KEYCANCEL:
		i=0;alphaFlag=0;count=4;
		return 1;
	case KEYCLEAR:
		if(0==i) Beef(6,700);
		else
		{
		digit_str[i-1]=0x00;
		ScrClrLine(4,5);
	    ScrPrint(0,4,1,"%16s",digit_str);
		--i;
		}
		for(k=0;k<i;k++)
		{
		if(digit_str[k]=='.') 
		{
			alphaFlag=1;
			break;
		}
		}
		if(alphaFlag==1 && ((i-k)>=1)) count++;
		break;
	default:  
	Beef(6,700);
		}
	 ScrClrLine(4,5);
     ScrPrint(0,4,0x01,"%16s",digit_str);
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
	ulong amount;
	
	SystemInit();
	while(1)
	{
		ScrCls();
		kbflush();
		ScrPrint(0,0,0x01|0x80,"   ShowAmount   ");

	do {
		ucKey=getkey();
	} while(ucKey!=KEY1 && ucKey!=KEYENTER &&ucKey!=KEYCANCEL);

	if(ucKey==KEY1 || ucKey==KEYENTER)
	{
	get_amount(BUFSIZ,&amount);
	ScrPrint(2,0,0x01,"%ld",amount);
	DelayMs(3000);
	}
	if(ucKey==KEYCANCEL) return 1;

	}
	return 0;
}
