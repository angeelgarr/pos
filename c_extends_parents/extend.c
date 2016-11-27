#include "extends.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>


void* print(void *arg)
{
	animal *p;

	p = NULL;
	if(NULL == arg)
	{
		printf("param error\n");
		exit(-2);
	}
	p = arg;
	printf("print data=%d\n",p->data.color);
	return (void*)0;
}

void* printDog(void *arg)
{
	dog *d;

	d = NULL;
	if(NULL == arg)
	{
		printf("param error\n");
		exit(-2);
	}
	d = arg;
	printf("print dog data%s\n",d->dog.name);
	return (void*)0;
}
void* printDog1(void *arg)
{
	if(NULL == arg)
	{
		printf("param error\n");
		exit(-2);
	}
	printf("print a =%d\n",(void*)arg);
	return (void*)0;
}


void *execFunc(void)
{

	animal *an;
	dog *d;
        
	an = (animal*)malloc(sizeof(animal));
	if(NULL == an)
	{
		printf("malloc fail\n");
		exit(-1);
	}
	d  = (dog*)malloc(sizeof(dog));
	if(NULL == d)
	{
		printf("malloc fail\n");
		exit(-1);
	}
	memset(an,0,sizeof(animal));
	memset(d,0,sizeof(dog));
	an->print = print;
	an->data.color = 342;
	strcpy(an->data.name,"xieAnimal");

	d->dog.color = 343;
	strcpy(d->dog.name,"xieDog");
	memcpy(&(d->dogExtendsAnimal),an,sizeof(animal));
	d->dogExtendsAnimal.print = printDog;
	d->dogExtendsAnimal.print(d);
	an->print(an);
	free(an);
	free(d);
	return (void*)0;
}

int main(void)
{
	void *p;

	p = NULL;
        p = execFunc();
	return 0;
}

