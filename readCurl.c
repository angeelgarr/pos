/*
 * test.c
 *
 *  Created on: 2017年5月1日
 *      Author: jeff
 */

#include<stdio.h>

int main(void)
{
	int iFd;
	FILE *stream;
	char buff[100] = {0};
	stream = fopen("tempko","w+");
	int j = 0;
	int iLen;

	for(int i = 0;i < 3;i++){
		fwrite("buff",4,1,stream);
	}
	fseek(stream,0,SEEK_SET);
    while(1)
    {
    	iLen = fread(&buff[j],100,1,stream);
    	printf("iLen=%d",iLen);
    	if(iLen <= 0)
    	{
    		break;
    	}
	j +=`100;
    }
	fclose(stream);
	printf("\nbuff=%s\n",buff);

	return 0;
}

