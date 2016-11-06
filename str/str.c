#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<time.h>

int CheckStringCount(unsigned char *source,unsigned char *subString)
{
	unsigned char *p;
	int count ;
	
	if(source == NULL || subString == NULL)
	{
		printf("param error\n");
		return -1;
	}
	count = 0;
	while(1)
	{
		p = strstr(source,subString);
		if(NULL == p)
		{
			return count;
		}
		source = p + strlen(subString);
		count ++;
	}
	return count;
}
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
	unsigned char str3[2001];
	unsigned char str4[2001];
	unsigned char tempBuf[1001];
	unsigned char *p,*q ,*pTemp,*qTemp;
	int iControlP;
	int iControlq;
	int i;
	int iRandInterger;
	int pCount;
	int qCount;
    int iLoopStopFlag;
    int iInsertSubStrCount;
	int iCheckQCount;
	int iCheckPCount;
	int iTestCount;
	
	iCheckQCount = 0;
	iCheckPCount = 0;
	iInsertSubStrCount = 0;
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
#if 0
		for(i = 0;i < iRandInterger;i++)
		{
			str1[i] = 0x01 + (rand() % 0xFF);
		}
		iInsertSubStrCount = (rand() % 3) + 1;
		for(i = 0;i < iInsertSubStrCount;i++)   //随机插进子串
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
#endif
		
		strcpy(str1,"wefew\\W=w==\\W=w==\\W=w==\\W=w==\\W=w===5757/w=/w"  \
		"wefew\\W=w==\\W=w==/w==jyukj/w=/w==\\W==wed\\W=\\W=w==\\W=w==\\W=w===wewef/w=/w"  \
		"\\W=w==/w/w/w==/w==W=w==wdedew/w==/w=/w==\\W==\\W=\\W=w==\\W=w==\\W=w===wewef/w=/w"  \
		"\\W=w==\\W=w==/w\\W====/w=/w==\\W==newk\\Ww=jfw\\W=/w=/w==/w=\\Ww=jytj\\W=w==\\W=w==\\W=w===wewef/w=/w"  \
		"/w=/w=/w===\\W=w==/w==/w=/w==\\W==\\W=/w=/w==/w=\\Ww=\\W=w==\\W=w==\\W=w===wewef/w=/w"  \
		"=/w==\\W=w==\\W=w=="); //for test
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
				//printf("now p =%s\n",p);
				pCount ++;
			 }
			 if(q!= NULL && q != qTemp)
			 {
				//printf("now q =%s\n",q);
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
    //memset(str3,0,sizeof(str3));
	//memset(str4,0,sizeof(str4));
    memset(tempBuf,0,sizeof(tempBuf));
	do
	{
		qCount = ReplaceString(tempBuf,str1,"\\W=","/v=/");
		pCount = ReplaceString(str2,tempBuf,"/w=","\\W");
		iCheckQCount = CheckStringCount(str2,"/w=");
		iCheckPCount = CheckStringCount(str2,"\\W=");
		if(iCheckQCount | iCheckPCount == 0)
		{
			printf("DONOT EXIST the subString\n");
			printf("str2=%s\n",str2);
			break;
		}
		else if(iCheckPCount == -1 || iCheckQCount == -1)
		{
			return -1;
		}
		else
		{
			printf("EXIST the subString\n");
			//qCount = ReplaceString(str3,str2,"\\W=","/v=/");
			//pCount = ReplaceString(str4,str3,"/w=","\\W");
			//strcpy(str1,str4); //循环处理
			strcpy(str1,str2);
		}
	}while((iCheckQCount | iCheckPCount) != 0);
	iTestCount = CheckStringCount(str2,"\\W=");
	if(iTestCount != 0)
	{
		printf("test fail\n");
		return -1;
	}
	iTestCount = CheckStringCount(str2,"/w=");
	if(iTestCount != 0)
	{
		printf("test fail\n");
		return -1;
	}
 	printf("test success\n");
	return 0;
}
