/****************************************************************************
ProTimsLib

百富计算机技术(深圳)有限公司

修改历史：
v1.0.0.0_120611    2012年6月11日    肖文佑

文件包括:
ProTimsDef.h					 ProTimsLib主要定义，包括数据结构，错误码，全局变量定义等
ProTims_SXX_COMM.h				 S系列的通讯实现
ProTims_PXX_COMM.h				 P系列的通讯实现
ProTims_comm.h			    	 主要实现代码
ProTimsEx.h				         ProTimsLib对外接口与逻辑流程，include此文件即可使用
appLib.h                         应用标准库，调用了里面的部分函数如PubGetString
****************************************************************************/

#ifndef PROTIMS_H_
#define PROTIMS_H_

#include "kms_comm.h"


int KmsRemoteLoadApp(const T_INCOMMPARA *ptCommPara);   

//////////////////////////////////////////////////////////////////////////
// New API

/****************************************************************************
功能： 	    初始化ProTims远程下载的调用，设置功能参数
输入参数	crtlBit
				PROTIMS_RESUME_DOWNLOAD		0x0001	// 是否支持断点续传
				PROTIMS_UPDATE_INSTANT		0x0002	// 是否立刻更新下载的文件
				PROTIMS_SET_TIME			0x0004	// 是否设置POS时间为ProTims系统时间
				PROTIMS_LOADTYPE_NEWVER		0x0008	// 1, T_INCOMMPARA->bLoadType全部起作用， 0，只有后三位起作用   （为兼容老版本的RemoteLoadApp）											
输出参数	无
返回：  	0 成功  其余失败
****************************************************************************/
int KmsInit();

/****************************************************************************
功能： 	    ProTims远程下载
输入参数	ptInPara
输出参数	无
返回：  	0 成功  其余失败
****************************************************************************/
int KmsDownload(T_INCOMMPARA *ptInPara);


/****************************************************************************
功能： 	    获取ProTims出错的详细信息
输入参数	无
输出参数	pLastError，长度至少为2的int数组，其中
            pLastError[0]为出错的函数，详细代码请参照PROTIMS_ERROR_CODE定义
			pLastError[1]为改函数的返回码，详细代码请参照API编程手册
返回：  	无
****************************************************************************/

void KmsGetLastErr(int* pLastError);						// Get last error code

/****************************************************************************
功能： 	    设置ProTims提示语言的种类
输入参数	
	KindOfLang
		PROTIMS_PROMPT_EN,   
		PROTIMS_PROMPT_CN,

输出参数	无
返回：  	0 success -1 failed
****************************************************************************/

int KmsSetLanguage(int LangType);

/****************************************************************************
功能： 	   加载语言资源
输入参数	无

输出参数	无
返回：  	0 success -1 failed
****************************************************************************/

int KmsLoadLanguage(); 


/****************************************************************************
功能： 	    得到key对应的value
输入参数	pszKey 
输入参数	iSize 	pszValue 对应内存大小
输出参数	pszValue
返回：  	0 success
****************************************************************************/

int KmsReadVar( const char *InKey, char *OutValue,int iLen);

/****************************************************************************
功能： 	    设置key value 键值对
输入参数	pszKey,pszValue
输出参数	无
返回：  	0 success
****************************************************************************/

int KmsWriteVar(const char *InKey, const char *InValue);

/****************************************************************************
功能： 	    设置缺省参数
输入参数	无
输出参数	无
返回：  	0 success
****************************************************************************/
static int ParaTransformers(T_CSCOMMPARA *pComPara, T_INCOMMPARA *ptOutPara);
static int LoadDownloadPara(T_CSCOMMPARA *pComPara);
static int DisplayErrorMessage(int errCode);
static void DispTitle(void *pTitle);
static int GetRemoteLoadVer(unsigned char *Ver);

int KmsSetDefaultCfg(void);
int KmsRemoteLoadApp(const T_INCOMMPARA *ptCommPara);   

int KmsMain();
#endif








