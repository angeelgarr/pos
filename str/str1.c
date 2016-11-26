#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int CheckCharCount(char *source,char c,char **pSaveArray)
{
	char *p;
	int count ;
	
	if(source == NULL)
	{
		printf("param error\n");
		return -1;
	}
	count = 0;
	while(1)
	{
		p = strchr(source,(int)c);
		if(NULL == p)
		{
			return count;
		}
		pSaveArray[count] = p;
		source = p + 1;
		count ++;
	}
	return count;
}

void PrintArray(char (*array)[51],int row,int col)
{
	int i;
	int j;
	char printBuf[200];
	
	for(i = 0;i < row;i++)
	{
		
		memset(printBuf,0,sizeof(printBuf));
		for(j = 0;j < col;j++)
		{
			sprintf(printBuf + j * 1,"%c",array[i][j]);
		}
		printf("buf=%s\n",printBuf);
	}
}

int GenArray(char (*array)[51])
{
	int i;
	int iSmall;
	int iBig;
	int j;
	int iRet;
	int randCount;
	const char srandArray[] = "1234567890abcdefghijklmnopqrstuvwxyz " \
	"ABCDEFGHIJKL\"MNOPQRSTU'VWXYZ/";
	int srandArrayLen;
	char cTemp;
	char tempBuf[60];
	char tempBuf2[60];
	char *pSaveArrayOne[10];
	char *pSaveArrayTwo[10];
	char *pSaveArrayTemp[10];
	int  smallTimes;
	int bigTimes;
	int iHidePrintFlag;
	int iInsertCount;
	int iLoopCondition = 0;
	
	if(array == NULL)
	{
		printf("param error\n");
		return -1;
	}
	iInsertCount = 0;
	smallTimes = 0;
	iHidePrintFlag = 0;
	bigTimes = 0;
	srandArrayLen = strlen(srandArray);
	randCount = 0;
	srand(time(NULL));
	//printf("enter GenArray,len=%d\n",srandArrayLen);
	for(i = 0;i < 10;)
	{
		randCount =  ( (rand() % 50) + 1);
	    memset(array[i],0,51);
		memset(tempBuf,0,sizeof(tempBuf));
		for(j = 0;j < randCount;j++)
		{
			array[i][j] = srandArray[rand() % srandArrayLen];
			//cTemp = array[i][j];	
			
		}
	
		//printf("array[i]=%s\n",array[i]);
		
		for(j = 0;j < 2;j++)   //随机插进子串，因为随机生成时间太长
		{
			switch(rand() % 2)
			{
			case 1:
				*(array[i] + (rand() % randCount)) = '\'';
				*(array[i] + (rand() % randCount)) = '\'';
				//printf("debug_array[%d]=%d array[%d]+ rand() & randCount = %d\n ",i,array[i],i,array[i] +(rand() & randCount));
				//memcpy(array[i] + ( rand() & (randCount)),"\"",1);
				//memcpy(array[i] + ( rand() & (randCount)),"\"",1);
				break;
			case 0:
				*(array[i] + (rand() % randCount)) = '"';
				*(array[i] + (rand() % randCount)) = '"';
				//memcpy(array[i] + ( rand() & (randCount)),"'",1);
			   // memcpy(array[i] + ( rand() &(randCount)),"'",1);
				break;
			default :
				break;
			}
		}
		memcpy(tempBuf,array[i],randCount);
		
		
		//memset(tempBuf,0,sizeof(tempBuf));
		//strcpy(tempBuf,"jeff'\"j\"h'");
	
		//printf("tempBuf=%s\n",tempBuf);
		/*保证引号成对出现*/

		if((((smallTimes = CheckCharCount(tempBuf,'\'',pSaveArrayOne)) % 2) == 0) && (bigTimes = CheckCharCount(tempBuf,'"',pSaveArrayTwo)) % 2 == 0)
		{
			iLoopCondition |= 0x01;
		}
		else 
		{
			continue;
		}

	     //printf("sa=%d,big=%d\n",smallTimes,bigTimes);
		/*保证每组对应的单引号或者双引号之间不能存在单一的单双引号，而且至少有一个字符串*/
		for(iSmall = 0;iSmall < smallTimes-1;iSmall+=2)
		{
			if(pSaveArrayOne[iSmall + 1] - pSaveArrayOne[iSmall] == 1)
			{
				//printf("pOne = %dpOnee=%dsmallMount=%d,\n",pSaveArrayOne[iSmall + 1],pSaveArrayOne[iSmall],smallTimes);
				iLoopCondition &= 0x0D;
				break;
			}
			memset(tempBuf2,0,sizeof(tempBuf2));
			memcpy(tempBuf2,pSaveArrayOne[iSmall],pSaveArrayOne[iSmall + 1] - pSaveArrayOne[iSmall]);
			if((iRet = CheckCharCount(tempBuf2,'"',pSaveArrayTemp)) % 2 == 1)
			{
				iLoopCondition &= 0x0D;
				break;
			}
			iLoopCondition |= 0x02;
		}

		for(iBig = 0;iBig < bigTimes-1;iBig+=2)
		{
			if(pSaveArrayTwo[iBig + 1] - pSaveArrayTwo[iBig] == 1)
			{
				iLoopCondition &= 0x0B;
				break;
			}
			memset(tempBuf2,0,sizeof(tempBuf2));
			memcpy(tempBuf2,pSaveArrayTwo[iBig],pSaveArrayTwo[iBig + 1] - pSaveArrayTwo[iBig]);
			if((iRet = CheckCharCount(tempBuf2,'\'',pSaveArrayTemp)) % 2 == 1)
			{
				iLoopCondition &= 0x0B;
				break;
			}
			iLoopCondition |= 0x04;
		}

		//printf("sa=%d,big=%d\n",smallTimes,bigTimes);//以上三个条件同时满足才生成接下来的字符串
		if(iLoopCondition == 0x07)
		{
			//printf("result buf=%s\n",tempBuf);
		    memset((array[i]),0,51);
		    strcpy((array[i]),tempBuf);
			// memset(*(array + i),0,51);
		    //strcpy(*(array + i),tempBuf);
			//printf("array[%d] buf=%s\n",i,array[i]);
			i++;
		}
		else
		{
			continue;
		}
		
			
		
	}//for
	return 0;
}

void sort(char (*list)[51],int num,char **output)
{
	int i,j;
    char *temp;
 
	for(i = 0;i < num;i++)
	{
		output[i] = list[i];
	}

	for(i = 0;i < num;i++)
	{
		for(j = i + 1;j < num;j++)
		{
			if(strcmp(output[i],output[j]) > 0)
			{
				temp = output[i];
				output[i] = output[j];
				output[j] = temp;
			}
		}
	}
}

int main(void)
{
	char A[10][51];
	char *B[10];
	int i;

	memset(A,0,sizeof(A));
	GenArray(A);
	//PrintArray(A,10,51);
    printf("\n\nstart to sort\n\n");
	sort(A,10,B);
	for(i = 0;i < 10;i++)
	{
		printf("NewBuf=%s\n",B[i]);
	}
	return 0;
	
}