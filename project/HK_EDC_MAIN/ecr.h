#ifndef _ECR_H_
#define _ECR_H_

#define ECR_TIME_ID		2		/*ECR 通讯使用2 timer*/
#define ACK_STR "\"<START>\",\"<END>\""


/************************************************声明***************************************/
#ifdef __cplusplus
extern "C" {
#endif

void EcrOpen(void);
int  ProcEcrEvent(void);

#ifdef __cplusplus
}
#endif






#endif




// end of file

