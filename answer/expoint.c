#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct _arraylist arraylist;

struct _arraylist
{
	void **data; //point to array of pointer;
	int length;
};


void test(void)
{

	arraylist *alist;
	char *c;
		
	alist = (arraylist*)malloc(sizeof(arraylist));	
	alist->data = malloc(2 *  sizeof(void*));
	if(alist == NULL)
	{
		printf("malloc error\n");
		exit(-1);
	}
	alist->data[0] = "xiehuan";
	alist->data[1] = "cece";
	printf("alist->data[1]=%s,alist->data[1][2]=%c\n",alist->data[1],((char**)(alist->data))[1][2]);
}

	 

int main(void)
{
	test();
	return 0;
}
