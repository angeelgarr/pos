#ifndef LIST_H
#define LIST_H

typedef unsigned long listcount_t;
#define nilnode list_nilnode
#define nodecount list_nodecount
#define maxcount list_maxcount
#define next list_next
#define prev list_prev

#define list_first_priv(L) ((L)->nilnode->next)

typedef struct lnode_t{
#ifndef KAZ_DEBUG
  struct lnode_t *list_next;
  struct lnode_t *list_prev;
  void *list_data;
 #else
 int list_dummy;
 #endif
}lnode_t;

typedef struct list_t{
  #ifndef KAZ_DEBUG
  lnode_t list_nilnode;
  listcount_t list_nodecount;
  listcount_t list_maxcount;
  #else
  int list_dummy;
  #endif
}list_t;




#endif
