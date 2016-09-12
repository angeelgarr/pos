#ifndef _COM_H_
#define _COM_H_
/*请求命令*/
#define NEW_REQUEST				0x01 /*1—  打开一新上送任务之请求*/
#define SEND_REQUEST			0x02 /* 2—  上送数据之请求*/
#define END_TASK_REQUEST		0x03 /*3—  结束当前上送任务之请求*/

/*响应命令*/
#define NEW_RESPONSE			0x81 /*0x81--打开一新的上送任务之响应*/
#define SEND_RESPOSE			0x82  /*0x82--上送数据之响应*/
#define END_TASK_RESPONSE		0x83  /*0x83--结束当前上送任务之响应*/

/*请求任务确认标志*/
/*上送数据确认标志*/
/*结束任务确认标志*/
enum
{
	CONFRIM	=0,
	FILENAME_INVAILD,
	FILE_OVER_LEN,
	SERVICE_STOP,
	UNMATCHED_TASKNO,
	PACKAGE_OVER_LEN,
	ERROR_CRC,
};
//接收函数的返回值
#define recOk				0x00
#define recUnexpected		0x80
#define onePkgOverTime		0x81
#define waitAllpkgOverTime	0x82

//发送函数返回值
#define portSendFail		0x83
//结束返回码
#define endOk				0x00
#define useCancel			0x01
#define overTime			0x02
#define SystemError			0x03

int processData(const struct shellData *shell,const char *indata,ushort *data_len,char *recBuf,unsigned short *recLen);
/*客户端包壳*/
typedef struct shellData{
	
	uchar fileName[20];//文件名字
	uchar fileSize[4];  //文件大小
	uchar pkgMaxSize[2];  //容许的单个数据包最大尺寸
	uchar taskNo[2];  //分配的任务号
	uchar fileNameLen;//文件名长度
	uchar confirmSign;  //确认标示
	uchar fileCRC[2];  //文件CRC校验码
	int cmd;           //命令码
}shellData;

#endif
