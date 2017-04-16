#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include"list.h"


list_t *list_init(list_t *list,listcount_t maxcount)
{
  list->nilnode.next = &list->nilnode;
	list->nilnode.prev = &list->nilnode;
	list->nodecount = 0;
	list->maxcount = maxcount;
	return list;
}

list_t *list_create(listcount_t maxcount)
{
	list_t *new = malloc(sizeof(*new));

	if(new)
	{
		assert(maxcount != 0);
		new->nilnode.next = &new->nilnode;
		new->nilnode.prev = &new->nilnode;
		new->nodecount = 0;
		new->maxcount = maxcount;
	}
	return new;
}

int list_isempty(list_t *list)
{
	return (list->maxcount == 0);
}

void list_destroy(list_t *list)
{
	assert(lis)
}



int main(void)
{
	printf("start\n");
	list_create(0);
	printf("end\n");
	return 0;
}
