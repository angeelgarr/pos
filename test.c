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
static struct TCP_SEG *sTemp = NULL;

void AddTcpPackage(struct TCP_SEG *p,struct TCP_SEG *source)
{
    if(sTemp != NULL)
    {
      sTemp->next = p;
    }
    p->data_len = source->data_len;
    p->start_seqno = source->start_seqno;
    memcpy(p->data,source->data,source->data_len);
    sTemp = p;
}

int main(void)
{
  int i;
  int temp;
  int count[100];
  int control = 0;
  struct TCP_SEG *p;
  struct TCP_SEG tempTcpPkg;
  struct TCP_SEG *output;
  int randA;
  int randB;
  int  k;
  int fixA;
  int fixB;

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
  while(1)
  {
    fixA = rand() % 100;
    fixB = rand() % 100;
    if(fixA != fixB)
    {
      break;
    }
  }

  for(i = 0;i < 100;i++)
  {
     memset(&tempTcpPkg,0,sizeof(struct TCP_SEG));
     tempTcpPkg.next = NULL;
     tempTcpPkg.start_seqno = count[i];
     tempTcpPkg.data_len = rand() % 1401;
     if(i == fixA)
     {
       tempTcpPkg.data_len = 0;
     }
     if(i == fixB)
     {
       tempTcpPkg.data_len = 1400;
     }
     for(k = 0;k < tempTcpPkg.data_len;k++)
     {
       tempTcpPkg.data[k] = (k % 256);
     }
    AddTcpPackage(p + i,&tempTcpPkg);
  }
 output = tcp_seg;
 while(output)
 {
  printf("len=%d ",output->data_len);
  for(k = 0;k < output->data_len;k++)
  {
    printf("%02X ",output->data[k]);
  }
  printf("\n");
  output = output->next;
}

}
