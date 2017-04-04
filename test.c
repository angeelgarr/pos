#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

struct TCP_SEG
{
  struct TCP_SEG *next;
  unsigned short data_len;
  unsigned int start_seqno;
  unsigned char data[1400];
};

struct TCP_SEG tcp_seg[100];

void AddTcpPackage(struct TCP_SEG *p,struct TCP_SEG *source)
{
    p->data_len = source->data_len;
    p->start_seqno = source->start_seqno;
    memcpy(p->data,source->data,source->data_len);
}

int main(void)
{
  int i;
  int temp;
  int count[100];
  int control = 0;
  struct TCP_SEG *p;
  struct TCP_SEG tempTcpPkg;
  int randA;
  int randB;
  int  k;

  p = tcp_seg;
  for(i = 0;i < 100;i++)
  {
    count[i] = i;
  }
  srand(time(NULL));
  for(i = 0;i < 100;i++)
  {

    randA = rand() % 100;
    randB = rand() % 100;
    temp = count[randA];
    count[randA] = count[randB];
    count[randB] = temp;

  }
  for(i = 0;i < 100;i++)
  {
     memset(&tempTcpPkg,0,sizeof(struct TCP_SEG));
     tempTcpPkg.next = NULL;
     tempTcpPkg.start_seqno = count[i];
     tempTcpPkg.data_len = count[i];
     for(k = 0;k < count[i];k++)
     {
       tempTcpPkg.data[k] = (k % 256);
     }
    AddTcpPackage(p + i,&tempTcpPkg);
  }
for(i = 0;i < 100;i++)
{
  printf("tcp_seg[%d]_len =%d ",i,tcp_seg[i].start_seqno);
  for(k = 0;k < tcp_seg[i].data_len;k++)
  {
    printf("%d ",tcp_seg[i].data[k]);
  }
  printf("\n");
}
}
