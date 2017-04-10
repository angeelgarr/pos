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
  
 output = tcp_seg;  //开始测试数据
 aCount = 0;
 bCount = 0;
 compareLen = -1;
 checkSeqCount = 0;
 while(output) 
 {
  printf("seq =%d,len=%d ",output->start_seqno,output->data_len);
  
  if(compareLen != -1)         //检查如果Tcp报文next数据节点的数据长度 是否与 结构体数组中向后一个数据长度相等
  {
	  if(output->data_len != compareLen)
	  {
		  printf("different data len\n");
		  return -1;
	  }
  }
  if(output->data_len < 0 || output->data_len > 1400) //检查数据长度
  {
	  printf("data length error\n");
	  return -1;
  }
  if(output->start_seqno < 0 || output->start_seqno > 100) //检查数据序号
  {
	  printf("data seq error\n");
	  return -1;
  }
  checkDataSeq[checkSeqCount++] = output->start_seqno;
  if(output->data_len == 0)   //查看是否有长度0和1400的数据
  {
	  aCount++;
  }
  if(output->data_len == 1400)
  {
	  bCount++;
  }
  
  for(k = 0;k < output->data_len;k++)//数据数据
  {
    printf("%02X ",output->data[k]);
  }
  printf("\n");
  if(output->next != NULL)
  {
	   printf("next_len=%d ",compareLen = output->next->data_len);
  }
  printf("\n");
  output = output->next;
}
printf("aCount=%d,bCount=%d\n",aCount,bCount);
if(checkSeqCount != 100)  //检查序号是否有一百个，是否有重复
{
	printf("data seq is not enough\n");
	return -1;
}

for(i = 0;i < checkSeqCount;i++)    
{
	
	for(k = i+1;k < checkSeqCount;k++)
	{
		if(checkDataSeq[i] == checkDataSeq[k])
		{
			printf("Repeated data seq\n");
			return -1;
		}
	}
}
return 0;
}
