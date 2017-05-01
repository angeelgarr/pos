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
   struct TCP_SEG_HEAD *next;//ָ����һ��TCP���ṹ��ָ�롣����ָ��ΪNULL����ʾ��һ��Ϊ�ա��ѵ������β��
};

struct TCP_SEG_HEAD *outofseq_list;//�����������ָ��

struct TCP_SEG tcp_seg[100];
static struct TCP_SEG *sTemp = NULL;

void PrintTcpData(struct TCP_SEG *s)
{
  int k;

  printf("s->seg=%d len=%d ",s->start_seqno,s->data_len);
 for(k = 0;k < s->data_len;k++)//��������
 {
    printf("%02X ",s->data[k]);
 }
 printf("\n");

}

void AddTcpList(struct TCP_SEG_HEAD *head,struct TCP_SEG *p)
{
	struct TCP_SEG_HEAD *this = head->next;//�ʼ��head-next = null������ͷ�巨

	head->next = p;
	p->next = this;
}

int FindSeq(struct TCP_SEG_HEAD *head,int seq) //�ҵ���ͬ��ŵİ�֮��ɾ���˰�
{
	struct TCP_SEG *this = head->next;
	struct TCP_SEG *temp;

	while(this && head->next)
	{
		if(((struct TCP_SEG*)(head->next))->start_seqno == seq)//��������ͷ�ڵ�
		{
			PrintTcpData((struct TCP_SEG*)(head->next));
			head->next = this->next;
			return 0;
		}
		else if(this->start_seqno != seq)//����ͷ���֮��Ľ��
		{
			temp = this;    //������һ�����
			this = this->next;
		}
		else if(this->start_seqno == seq)
		{
			PrintTcpData(this);
			temp->next = this->next;
			return 0;
		}

	}

	printf("cannot find this package\n");
	return -1;

}

int HandleTcpData(struct TCP_SEG *s)
{
	int i;
	int sameSeqCount;
	int iRet;

	if((outofseq_list = (struct TCP_SEG_HEAD*)malloc(sizeof(struct TCP_SEG_HEAD))) == NULL)
	{
		printf("malloc error\n");
		return -1;
	}
	sameSeqCount = 0;
	for(i = 0;i < 100;i++) //�������ͬ�İ���ӡ��������ͬ�ļ�������
	{
		if(s[i].start_seqno == sameSeqCount)
		{
			PrintTcpData(&s[i]);
			sameSeqCount++;
		}
		else
		{
			AddTcpList(outofseq_list,&s[i]);
		}
	}
	printf("start to look up\n");

	while(1)
	{
		if(sameSeqCount == 100)
		{
			break;
		}
		iRet = FindSeq(outofseq_list,sameSeqCount);
		if(iRet)
		{
			return iRet;
		}
		sameSeqCount++;
	}
	return 0;
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
  int i,iRet;
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
  for(i = 0;i < 100;i++) //���ɴ�0��99���������
  {
    count[i] = i;
  }
  srand(time(NULL));

  while(1)   //Ϊ�������0��1400���ȵ�������׼��
  {
    fixA = rand() % 100;
    fixB = rand() % 100;
    if(fixA != fixB)
    {
      break;
    }
  }

  for(i = 0;i < 100;i++) //��ʼ���
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
     if(k >= 1)
     {
    	 iTemp = tempTcpPkg.data[k - 1] + 1;
     }

    AddTcpPackage(p + i,&tempTcpPkg);
  }

 for(i = 0;i < 1000;i++)    //������ҽ�������
  {
    randA = rand() % 100;
    randB = rand() % 100;
	ExchangeStru(&temp,&tcp_seg[randA]);
	ExchangeStru(&tcp_seg[randA],&tcp_seg[randB]);
	ExchangeStru(&tcp_seg[randB],&temp);
  }
  iRet = HandleTcpData(tcp_seg);
  if(iRet)
  {
	  return iRet;
  }
  return 0;
}

