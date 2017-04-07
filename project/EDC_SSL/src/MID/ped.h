/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2012-? PAX Technology, Inc. All rights reserved.
 * Description:
 * Revision History:
 * Date                  Author                 Action
 * 2012-12-4          wenjf                    created
 * ============================================================================
 */

#ifndef _H_PED_H_
#define _H_PED_H_

#include "structDefine.h"
#include "../emvlib_Prolin.h"
//#include "osal.h"

#define RED			RGB(0xff, 0, 0)
#define GREEN		RGB(0, 0xff, 0)
#define BLUE		RGB(0, 0, 0xff)
#define BLACK		RGB(0x00, 0x00, 0x00)
#define RIGHT_BLUE	RGB(0, 20,255)
#define WHITE		RGB(0xFF, 0xFF, 0xFF)
#define FINE		RGB(0xFB, 0xFB, 0xEE)
#define YELLOW		RGB(0xFF, 0xFF, 0)

#define FORE		RGB(0xff, 0, 0)
#define BACK		RGB(0xFB, 0xFB, 0xEE)

#define PED_TLK 	0x01	//Loading Key
#define PED_TMK 	0x02	//Master Key
#define PED_TPK 	0x03	//PIN Key
#define PED_TAK 	0x04	//MAC Key
#define PED_TDK 	0x05	//Data Key
#define PED_TIK 	0x10	//DUKPT Initial Key
#define PED_TRK 	0x07	//MSR Key
#define PED_KTK 	0x01	//Transport Key
#define PED_KTM 	0x02	//Master Key
#define PED_KEK 	0x03	//Key Exchange Key
#define PED_KMACS 	0x04	//MAC Key (receive)
#define PED_KMACR 	0x05	//MAC Key (send)
#define PED_KDR 	0x06	//Data Key (receive)
#define PED_KDS 	0x07	//Data Key (send)
#define PED_KPP 	0x08	//PIN key Protection Key
#define PED_KPE 	0x09	//PIN Encryption Key
#define PED_KHASH 	0x0A	//HASH Key
#define PED_PPID 	0x0B	//PPID
#define PED_PPASN 	0x0C	//PPASN



#define	PCI_OK				0x00	//
#define	PCI_ERROR			0x01	//
#define	PCI_PEDAAKEY_ERR	0x03	//
#define	PCI_KEYINDEX_ERR	0x05	//
#define	PCI_NO_KEY			0x06	//
#define	PCI_KEYAUTH_ERR		0x07    //
#define	PCI_KEYLEN_ERR		0x08	//
#define	PCI_CANCEL			0x09	//
#define	PCI_TIMEOUT			0x0a	//
#define	PCI_WAIT_INTERVAL	0x0b	//
#define	PCI_ICC_NOCARD		0x0c	//
#define	PCI_ICC_NOTINIT		0x0d	//
#define	PCI_COMM_ERR		0x0e	//
#define	PCI_NEED_ADMIN		0x0f	//
#define	PCI_LOCKED			0x10	//
#define	PCI_NO_PIN			0x11	//
#define	PCI_NO_PIN			0x11	//
#define	PCI_NO_UPK			0x12    //DUKPT OVER FLOW
#define	PCI_PED_NOMORE_BUF	0XFE
#define PCI_ADMIN_ERR		0x14;   //
#define	PCI_NOMORE_KEY		0XB0	//
#define	PCI_DUKPT_OVERFLOW	0xB1	//
#define	PCI_UNSPT_CMD 		0xff	//



#define PED_RET_ERR_START	-300	// PEDAPI 错误码起始值
#define PED_RET_ERR_END		-500	// PEDAPI 错误码结束值
#define PED_RET_OK				0	// PED函数返回正确
#define PED_RET_ERR_NO_KEY		PED_RET_ERR_START-1	 		// 密钥不存在
#define PED_RET_ERR_KEYIDX_ERR	PED_RET_ERR_START-2			// 密钥索引错,参数索引不在范围内
#define PED_RET_ERR_DERIVE_ERR	PED_RET_ERR_START-3			// 密钥写入时,源密钥的层次比目的密钥低
#define PED_RET_ERR_CHECK_KEY_FAIL	PED_RET_ERR_START-4		// 密钥验证失败

// 以下宏定义为了防止和emvlib.h中冲突，放在ped.c中定义
//#define PED_RET_ERR_NO_PIN_INPUT	PED_RET_ERR_START-5		// 没输入PIN
//#define PED_RET_ERR_INPUT_CANCEL	PED_RET_ERR_START-6		// 取消输入PIN
//#define PED_RET_ERR_WAIT_INTERVAL	PED_RET_ERR_START-7		// 函数调用小于最小间隔时间
#define PED_RET_ERR_CHECK_MODE_ERR	PED_RET_ERR_START-8		// KCV模式错,不支持
#define PED_RET_ERR_NO_RIGHT_USE	PED_RET_ERR_START-9		// 无权使用该密钥,当出现密钥标签不对,或者写入密钥时,源密钥类型的值大于目的密钥类型,都会返回该密钥
#define PED_RET_ERR_KEY_TYPE_ERR	PED_RET_ERR_START-10	// 密钥类型错
#define PED_RET_ERR_EXPLEN_ERR		PED_RET_ERR_START-11	// 期望PIN的长度字符串错
#define PED_RET_ERR_DSTKEY_IDX_ERR	PED_RET_ERR_START-12	// 目的密钥索引错,不在范围内
#define PED_RET_ERR_SRCKEY_IDX_ERR	PED_RET_ERR_START-13	// 源密钥索引错,不在范围内
#define PED_RET_ERR_KEY_LEN_ERR		PED_RET_ERR_START-14	// 密钥长度错

// 以下宏定义为了防止和emvlib.h中冲突，放在ped.c中定义
//#define PED_RET_ERR_INPUT_TIMEOUT	PED_RET_ERR_START-15		// 输入PIN超时
//#define PED_RET_ERR_NO_ICC			PED_RET_ERR_START-16	// IC卡不存在
//#define PED_RET_ERR_ICC_NO_INIT		PED_RET_ERR_START-17	// IC卡未初始化
#define PED_RET_ERR_GROUP_IDX_ERR	PED_RET_ERR_START-18	// DUKPT组索引号错
#define PED_RET_ERR_PARAM_PTR_NULL	PED_RET_ERR_START-19	// 指针参数非法为空
#define PED_RET_ERR_LOCKED			PED_RET_ERR_START-20	// PED已锁
#define PED_RET_ERROR				PED_RET_ERR_START-21	// PED通用错误
#define PED_RET_ERR_NOMORE_BUF		PED_RET_ERR_START-22	// 没有空闲的缓冲
#define PED_RET_ERR_NEED_ADMIN		PED_RET_ERR_START-23	// 需要取得高级权限
#define PED_RET_ERR_DUKPT_OVERFLOW	PED_RET_ERR_START-24	// DUKPT已经溢出
#define PED_RET_ERR_KCV_CHECK_FAIL	PED_RET_ERR_START-25	// KCV 校验失败
#define PED_RET_ERR_SRCKEY_TYPE_ERR	PED_RET_ERR_START-26	// 源密钥类型错
#define PED_RET_ERR_UNSPT_CMD		PED_RET_ERR_START-27	// 命令不支持
#define PED_RET_ERR_COMM_ERR		PED_RET_ERR_START-28	// 通讯错误
#define PED_RET_ERR_NO_UAPUK		PED_RET_ERR_START-29	// 没有用户认证公钥
#define PED_RET_ERR_ADMIN_ERR		PED_RET_ERR_START-30	// 取系统敏感服务失败
#define PED_RET_ERR_DOWNLOAD_INACTIVE	PED_RET_ERR_START-31	// PED处于下载非激活状态
#define PED_RET_ERR_KCV_ODD_CHECK_FAIL	PED_RET_ERR_START-32	// KCV 奇校验失败
#define PED_RET_ERR_PED_DATA_RW_FAIL	PED_RET_ERR_START-33	// 读取PED数据失败
#define PED_RET_ERR_ICC_CMD_ERR		PED_RET_ERR_START-34		// 卡操作错误(脱机明文、密文密码验证)
#define PED_RET_ERR_INPUT_CLEAR		PED_RET_ERR_START-39		// 用户按CLEAR键退出输入PIN
#define PED_RET_ERR_NO_FREE_FLASH	PED_RET_ERR_START-43		// PED存储空间不足
#define PED_RET_ERR_DUKPT_NEED_INC_KSN	PED_RET_ERR_START-44	// DUKPT KSN需要先加1
#define PED_RET_ERR_KCV_MODE_ERR		PED_RET_ERR_START-45	// KCV MODE错误
#define PED_RET_ERR_DUKPT_NO_KCV		PED_RET_ERR_START-46	// NO KCV
#define PED_RET_ERR_PIN_BYPASS_BYFUNKEY	PED_RET_ERR_START-47	// 按FN/ATM4键取消PIN输入
#define PED_RET_ERR_MAC_ERR			PED_RET_ERR_START-48		// 数据MAC校验错
#define PED_RET_ERR_CRC_ERR			PED_RET_ERR_START-49		// 数据CRC校验错




int PedWriteKey(ST_KEY_INFO * KeyInfoIn, ST_KCV_INFO * KcvInfoIn);
int PedWriteTIK(uchar GroupIdx, uchar SrcKeyIdx, uchar KeyLen,
		        uchar * KeyValueIn, uchar * KsnIn,  ST_KCV_INFO * KcvInfoIn);
int PedGetPinBlock(uchar KeyIdx, uchar *ExpPinLenIn,  uchar * DataIn,
                  uchar *PinBlockOut, uchar Mode,  ulong TimeOutMs);
int PedGetMac(uchar KeyIdx,  uchar *DataIn, ushort DataInLen,
              uchar *MacOut, uchar Mode);
int PedCalcDES(uchar KeyIdx, uchar * DataIn, ushort DataInLen,
               uchar * DataOut,  uchar Mode);
int PedGetPinDukpt(uchar GroupIdx, uchar *ExpPinLenIn,  uchar * DataIn, uchar* KsnOut,
                  uchar * PinBlockOut, uchar Mode, ulong TimeoutMs);
int PedGetMacDukpt (uchar GroupIdx, uchar *DataIn,  ushort DataInLen,
                    uchar *MacOut, uchar * KsnOut, uchar Mode);
int PedReGenPinBlock (uchar UpdateFlag, ST_KEY_INFO *KeyInfoIn, ST_KCV_INFO *KcvInfoIn,
                     uchar *DataIn, uchar *PinBlockOut, uchar Mode);
int PedVerifyPlainPin (uchar IccSlot, uchar *ExpPinLenIn, uchar *IccRespOut,
                       uchar Mode,ulong TimeoutMs);
int PedVerifyCipherPin (uchar IccSlot, uchar *ExpPinLenIn, RSA_PINKEY *RsaPinKeyIn,
                      uchar *IccRespOut, uchar Mode, ulong TimeoutMs);
//int PedGetKcv(uchar KeyType, uchar KeyIdx, ST_KCV_INFO *KcvInfoOut);
int PedWriteKeyVar (uchar KeyType, uchar SrcKeyIdx, uchar DstKeyIdx,
                   uchar * XorVarIn, ST_KCV_INFO * KcvInfoIn);
int PedGetVer(uchar * VerInfoOut);
int PedErase();
int PedSetIntervaltime(ulong TPKIntervalTimeMs, ulong TAKIntervalTimeMs);
//int PedSetKeyTag(uchar * KeyTagIn);
int PedSetFunctionKey(uchar ucKey);
int PedWriteRsaKey(uchar RSAKeyIndex, ST_RSA_KEY* pstRsakeyIn);
int PedRsaRecover (uchar RSAKeyIndex, uchar *pucDataIn, uchar * pucDataOut, uchar* pucKeyInfoOut);
int PedDukptDes(uchar GroupIdx, uchar KeyVarType, uchar * pucIV, ushort DataInLen,
                uchar * DataIn, uchar * DataOut, uchar * KsnOut, uchar Mode);
int PedGetDukptKSN(uchar GroupIdx, uchar * KsnOut);
int PedDukptIncreaseKsn(uchar GroupIdx);

int getPEDAsterisk_X();
int getPEDAsterisk_Y();
int getPEDAsterisk_H();

#endif
