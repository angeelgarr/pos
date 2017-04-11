#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct _listEntry listEntry;
struct _listEntry
{
	void *data;
	listEntry *prev;
	listEntry *next;
};

typedef struct _list
{
	listEntry *head;
	int count;
}list;

list listHead;
listEntry *AppendList(listEntry **list,void *data)
{
	listEntry *newEntry;

	if(list == NULL)
	{
		return NULL;
	}
	newEntry = (listEntry*)malloc(sizeof(listEntry));
	newEntry->data = data;
	if(*list != NULL)
	{
		(*list)->prev = newEntry;
	}
	else
	{
		*list = newEntry;
		listHead.count++;
		return newEntry;
	}
	listHead.count++;
	newEntry->next = *list;
	newEntry->prev = NULL;
	*list = newEntry;
	return newEntry;
}

void test(listEntry *s)
{
	int i = 0;

	while(i++ < listHead.count)
	{

		printf("find the data =%d\n",s->data);
		s = s->prev;//or s = s->next;
	}
}

int main(void)
{
	//listEntry *head;
	listEntry *tail;
	listEntry *temp;
	int i;

	//head = NULL;
	listHead.count = 0;
	listHead.head = NULL;


	temp = AppendList(&listHead.head,(void*)100);
	for(i = 0;i < 100;i++)
	{
		tail = AppendList(&listHead.head,(void*)i);
	}
	temp->next = tail;
	tail->prev = temp;
	test(temp);
	return 0;
}
