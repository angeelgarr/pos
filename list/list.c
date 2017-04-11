#include<stdio.h>

typedef struct _listEntry listEntry;
struct _listEntry
{
	void *data;
	listEntry *prev;
	listEntry *next;
};

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
		return newEntry;
	}
	newEntry->next = *list;
	newEntry->prev = NULL;
	*list = newEntry;
	return newEntry;
}

void test(listEntry *s)
{
	int i = 0;
	
	while(i++ <= 100)
	{
		
		printf("find the data =%d\n",s->data);
		s = s->prev;//or s = s->next;
	}
}

int main(void)
{
	listEntry *head;
	listEntry *tail;
	listEntry *temp;
	int i;
	
	temp = AppendList(&head,(void*)100);
	for(i = 0;i < 100;i++)
	{
		tail = AppendList(&head,(void*)i);
	}
	temp->next = tail;
	tail->prev = temp;
	test(temp);
	return 0;
}

