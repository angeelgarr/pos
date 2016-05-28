
#include <posapi.h>
#include <posapi_all.h>

const APPINFO AppInfo={
		"DRAW_DIGIT",
		"APP-TEST",
		"1.0",
		"pcteam",
		"demo program",
		"",
		0,
		0,
		0,
		""
};

int event_main(ST_EVENT_MSG *msg)
{
	return 0;
}


void showDigit5(int startX,int startY,int zoom,int isAngle,int showSwitch)
{
	int x,y,temp;
	char *code="\x7C\x40\x78\x04\x04\x44\x38\x00";

if(0==zoom)
{
	Lcdprintf("input Zoom Error\n");
	return 1;
}

  for(y=0;y<8*zoom;y++)
    for(x=0;x<8*zoom;x++)
		if((0x01<<x/zoom)&(code[y/zoom]))
		{
			if(0==isAngle) ScrPlot(startX+(8*zoom-1)-x,startY+y,showSwitch);
				
			else ScrPlot(startX-y,startY+(8*zoom-1)-x,showSwitch);
		}
		else
		{
			if(0==isAngle) ScrPlot(startX+(8*zoom-1)-x,startY+y,0);
				
			else ScrPlot(startX-y,startY+(8*zoom-1)-x,0);
		}

}

int main(void)
{	
	uchar ucKey;

	SystemInit();
	while(1)
	{
		ScrCls();
		kbflush();
		ScrPrint(0,0,0x01|0x80,"1.DRAW_DIGIT    ");
		do{

			ucKey=getkey();

		}while(ucKey!=KEY1 && ucKey!=KEYENTER && ucKey!=KEYCANCEL);

		if(ucKey==KEY1 || ucKey==KEYENTER)
		{	
				ScrCls();
				showDigit5(32,16,1,0,1);
				
				DelayMs(1000);
				showDigit5(32,16,1,0,0);
				showDigit5(32,16,1,1,1);

				DelayMs(1000);
				showDigit5(32,16,1,1,0);        //放大两倍
				showDigit5(32,16,2,0,1);
				
				DelayMs(1000);
				showDigit5(32,16,2,0,0);
				showDigit5(32,16,2,1,1);

				DelayMs(1000);
				showDigit5(32,16,2,1,0);   //放大三倍
				showDigit5(32,16,3,0,1);
				
				DelayMs(1000);
				showDigit5(32,16,3,0,0);
				showDigit5(32,16,3,1,1);
				getkey();
		}
		if(ucKey==KEYCANCEL) return 1;
	}
	return 0;
}
