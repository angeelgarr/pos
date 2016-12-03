#include<stdio.h>
#include<stdlib.h>

int gl_iUseMM = 0;

char gl_cArray[32000];

int FindFreeMM(void)
{
	int i;

	for(i = 0;i < 32;i++)
	{
		if(((gl_iUseMM >> i) & 1) == 0)
		{
			gl_iUseMM |= (1 << i);
			return i;
		}
	}
	return -1;
}

void *AllocMM(void)
{
	int i = FindFreeMM();
	if(i == -1)
	{
		return (void*)0;
	}
	else
		return (void*)(gl_cArray + i * 1024);

	return (void*)0;
}

void FreeMM(void *memAddr)
{
	int i;

	i = (gl_cArray - (char*)memAddr) / 1024;
	gl_iUseMM &= ~(1<<i);
}

int main(void)
{
	char *a;
	char *b;
	a = NULL;
	a = (char*)AllocMM();
	b = (char*)AllocMM();
	strcpy(b,"bbbbbbbb\n");
	strcpy(a,"aaaaaaa\n");
	printf("a=%sb =%s",a,b);	

	return 0;
}

