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

struct TCP_SEG_HEAD
{
   struct TCP_SEG_HEAD *next;//指向下一个TCP包结构的指针。若该指针为NULL，表示下一包为空、已到链表的尾部
};

struct TCP_SEG_HEAD *outofseq_list;//乱序包的链表指针

struct TCP_SEG tcp_seg[100];
static struct TCP_SEG *sTemp = NULL;

void PrintTcpData(struct TCP_SEG *s)
{
  int k;
	
  printf("s->seg=%d ",s->start_seqno);
 for(k = 0;k < s->data_len;k++)//数据数据
 {
    printf("%02X ",s->data[k]);
 }
 printf("\n");
 
}

void AddTcpList(struct TCP_SEG_HEAD **seqList,struct TCP_SEG *p)
{
	if(seqList == NULL)
	{
		return;
	}
	p->next = NULL;
	(*seqList)->next = p;
	*seqList = p;
}

void HandleTcpData(struct TCP_SEG *s)
{
	int i;
	int sameSeqCount;
	struct TCP_SEG_HEAD *temp;
	
	if((outofseq_list = (struct TCP_SEG_HEAD*)malloc(sizeof(struct TCP_SEG_HEAD))) == NULL)
	{
		printf("malloc error\n");
		return -1;
	}
	temp = outofseq_list;
	sameSeqCount = 0;
	for(i = 0;i < 100;i++)
	{
		if(s[i].start_seqno == i && )
		{
			PrintTcpData(&s[i]);
			sameSeqCount++;
		}
		else
		{
			AddTcpList(outofseq_list,&s[i]);
		}
	}
}

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

void ExchangeStru(struct TCP_SEG *d,struct TCP_SEG *s)
{
	d->data_len = s->data_len;
    d->start_seqno = s->start_seqno;
    memcpy(d->data,s->data,s->data_len);
}
int main(void)
{
  int i;
  struct TCP_SEG temp;
  int count[100];
  int randA;
  int randB;
  int  k;
  int fixA;
  int fixB;
  int aCount;
  int bCount;
  int compareLen;
  int checkDataSeq[100];
  int checkSeqCount;
  int iTemp;
  struct TCP_SEG *p;
  struct TCP_SEG tempTcpPkg;
  struct TCP_SEG *output;

  p = tcp_seg;
  iTemp = 0;
  for(i = 0;i < 100;i++) //生成从0到99的数据序号
  {
    count[i] = i;
  }
  srand(time(NULL));

  while(1)   //为随机插入0和1400长度的数据做准备
  {
    fixA = rand() % 100;
    fixB = rand() % 100;
    if(fixA != fixB)
    {
      break;
    }
  }

  for(i = 0;i < 100;i++) //开始组包
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
       tempTcpPkg.data[k] = (iTemp + (k % 256)) % 256;
     }
	 iTemp = tempTcpPkg.data[k - 1] + 1;
    AddTcpPackage(p + i,&tempTcpPkg);
  }
  
 for(i = 0;i < 1000;i++)    //随机打乱交换数据
  {
    randA = rand() % 100;
    randB = rand() % 100;
	ExchangeStru(&temp,&tcp_seg[randA]);
	ExchangeStru(&tcp_seg[randA],&tcp_seg[randB]);
	ExchangeStru(&tcp_seg[randB],&temp);
  }
  HandleTcpData(tcp_seg);
  
  
  return 0;
}