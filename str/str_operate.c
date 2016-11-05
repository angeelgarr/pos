#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<time.h>

int ReplaceString(unsigned char *result,unsigned char *source,
		unsigned char *subString,unsigned char*repString)
{
	unsigned char *p;
	int count;

	count = 0;
	if(result == NULL || source == NULL || subString == NULL || repString == NULL)
	{
		printf("param error\n");
		return -1;
	}
	while(1)
	{
		p = strstr(source,subString);
		if(p == NULL)
		{
			strcpy(result,source);
			return count;
		}
		count ++;
		memcpy(result,source,(p - source));
		memcpy(result + (p - source),repString,strlen(repString));
		result = result + (p - source) + strlen(repString);
	    source = p + strlen(subString);
	}
	return count;
}

int main(void) {
	unsigned char str1[1001];
	unsigned char str2[2001];
	unsigned char tempBuf[1001];
	unsigned char *p,*q ,*pTemp,*qTemp;
	int iControlP;
	int iControlq;
	int i;
	int iRandInterger;
	int pCount;
	int qCount;
    int iLoopStopFlag;

    iLoopStopFlag = 0;
    pCount = 0;
    qCount = 0;
	iControlP = 0;
	iControlq = 0;
	p = str1;
	q = str1;
	srand(time(NULL));

	while(1)
	{
		iRandInterger = 100 + (rand() % 901);
		printf("iRandInterger = %d\n",iRandInterger);
		if(iLoopStopFlag == 1)
		{
			break;
		}
		memset(str1,0,sizeof(str1));
		for(i = 0;i < iRandInterger;i++)
		{
			str1[i] = 0x01 + (rand() % 0xFF);
		}
		for(i = 0;i < (rand() % 3) +1;i++)   //随机插进子串，如果随机生成时间太长
		{
			switch(rand() % 2)
			{
			case 1:
				memcpy(str1 + (rand() % (iRandInterger-2)),"\\W=",3);
				break;
			case 0:
				memcpy(str1 + (rand() % (iRandInterger-2)),"/w=",3);
				break;
			default :
				break;
			}
		}
		 printf("Now str1=%s\n",str1);
		while(1)
		{
			pTemp = p;
			qTemp = q;
			if(iControlP == 0)
			{
				p = strstr(p ,"\\W=");
			}
		    if(iControlq == 0)
		    {
		    	q = strstr(q ,"/w=");
		    }
		    if(p == NULL)
		    {
		   iControlP = 1;
		  }
		  if(q == NULL)
		  {
		   iControlq = 1;
		  }
		 if((p!= NULL && p != pTemp))
		 {
			printf("now p =%s\n",p);
			pCount ++;
		 }
		 if(q!= NULL && q != qTemp)
		 {
			printf("now q =%s\n",q);
			qCount ++;
		 }
		 printf("count=%d\n",pCount + qCount);
	    if(p != NULL)
	    {
		   p += 3;
	    }
	    if(q != NULL)
	   {
	      q += 3;
	   }
		if(p == NULL && q == NULL)
		{
			if(pCount + qCount == 0)
			{
				break;
			}
			else
			{
				iLoopStopFlag = 1;
				break;
			}
		}
		}
	}
    printf("str1=%s\n",str1);
    memset(str2,0,sizeof(str2));
    memset(tempBuf,0,sizeof(tempBuf));
    qCount = ReplaceString(tempBuf,str1,"/w=","\\v");
    pCount = ReplaceString(str2,tempBuf,"\\W=","/w=\\");
    printf("str2 = %s\n qcount=%d\n,pCount=%d\n",str2,qCount,pCount);
	return ０;
}
