/*****************************************************/
/* ClssApi_MC.h                                      */
/* Define the Application Program Interface          */
/* of MasterCard PayPass for all PAX Readers         */
/* Created by Liuxl at July 30, 2009                 */
/*****************************************************/

#ifndef _CLSS_LIB_MC_H
#define _CLSS_LIB_MC_H

#include "cl_common.h"
 
/*  
typedef struct{
    int MaxLen;
    unsigned short Tag;
    unsigned short Attr;
}ELEMENT_ATTR;
*/
//密钥和应用管理函数定义
int  Clss_CoreInit_MC(void);
int Clss_ReadVerInfo_MC(char *paucVer); 

int  Clss_GetTLVData_MC(ushort Tag, uchar *DtOut, int *nDtLen);
int  Clss_SetTLVData_MC(ushort usTag, uchar *pucDtIn, int nDtLen);

int Clss_AddIccTag_MC(ELEMENT_ATTR *ptTagarr, int nAddnum);

//密钥和应用管理函数定义 
int  Clss_AddCAPK_MC(EMV_CAPK  *capk );
int  Clss_GetCAPK_MC(int Index, EMV_CAPK  *capk );
int  Clss_DelCAPK_MC(unsigned char KeyID, unsigned char *RID);
void Clss_DelAllCAPK_MC(void);

// added by liuxl 20070710 因无法由应用程序绕过证书列表问题而增加接口
int  Clss_AddRevocList_MC(EMV_REVOCLIST *pRevocList);
int  Clss_DelRevocList_MC(unsigned char ucIndex, unsigned char *pucRID);
void Clss_DelAllRevocList_MC(void);
 
 
//设置(关闭)EMV－CORE的调试状态
void Clss_SetDebug_MC(int EnableFlag);
void Clss_GetICCStatus_MC(unsigned char *SWA, unsigned char *SWB);
  
// 
int Clss_SetReaderParam_MC(Clss_ReaderParam *ptParam);
int Clss_GetReaderParam_MC(Clss_ReaderParam *ptParam);

int Clss_SetMCAidParam_MC(Clss_MCAidParam *ptParam);
int Clss_SetTransData_MC(Clss_TransParam *ptTransParam,Clss_PreProcInterInfo *ptInfo);
int Clss_SetFinalSelectData_MC(uchar *pucRspIn, int nRspLen);
 
int Clss_ProcTrans_MC(uchar ucForceMagFlg, uchar *pucPathTypeOut); 

// MC MCHIP 
int Clss_TransProc_MC_MChip(uchar ucBlackFileFlg, ulong ulLastPanAmt, uchar *pucACTypeOut);

// MC MAG
int Clss_TransProc_MC_Mag(uchar *pucACTypeOut);

int Clss_CardAuth_MC(uchar *pucACTypeOut);
 

// 20090531 liuxl 
int Clss_nGetTrackFreeFieldData_MC(uchar ucTrackFlg, uchar *pDataOut, int *pLenOut);
int Clss_nGetCombiTrackData_MC(uchar ucTrackFlg, uchar *pDataOut, uchar *pucLenOut);

#endif


