#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>

#define offset(TYPE,NUMBER) ((size_t)&((TYPE*)0)->NUMBER)

#define assert(p) \
	do{   \
		if(p > 0)  \
		{   \
			printf("p > 0");\
		}  \
	}while(0);

typedef struct student
{
  char gender;
  int id;
  int age;
  char name[20];
  char *next;
}student;


#define WARNING(test) \
	printf("warnin %s\n",#test"\n");

int main(void)
{
	student s1 = {'2',9527,25,"jeff",NULL};

  //	printf("first test and the name = %s\n",((student*)((char*)(&(s1.next)) - offset(student,next)))->name);
	assert(2);
	return 0;

}

