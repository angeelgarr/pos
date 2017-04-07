
/****************************************************************************
NAME
    checkopt.h - 定义选项检查函数

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.12      - created
****************************************************************************/

#ifndef _CHECKOPT_H
#define _CHECKOPT_H

#define HW_NONE			0
// offsets in the string returned by GetTermInfo()
#define HWCFG_MODEL		0
#define HWCFG_PRINTER	1
#define HWCFG_MODEM		2
#define HWCFG_M_SYNC	3
#define HWCFG_M_ASYNC	4
#define HWCFG_PCI		5
#define HWCFG_USBHOST	6
#define HWCFG_USBDEV	7
#define HWCFG_LAN		8
#define HWCFG_GPRS		9
#define HWCFG_CDMA		10
#define HWCFG_WIFI		11
#define HWCFG_CONTACT	12
#define HWCFG_CFONT		13
#define HWCFG_FONTVER	14
#define HWCFG_ICCREAD	15
#define HWCFG_MSR		16
#define HWCFG_WCDMA     18//2015-11-23 3G
// 17--29 reserved
#define HWCFG_END		30
// TIMER, TIMEOUT & RETRY DEFINE
#define PROTIMS_RECV_PACKET_TIME_OUT 150   // 钡Μ计沮禬15
#define PROTIMS_RECV_PACKECT_TIMER   4     // 钡Μ计沮﹚竟
#define PROTIMS_RECV_BYTE_TIMER      3     //
#define PROTIMS_TOTAL_SEND_PACKET_TIMER       2   // 祇癳﹚竟
#define PROTIMS_TOTAL_RECEV_PACKET_TIMER      2   // 钡Μ﹚竟
#define	PROTIMS_COUNT_TIMER			 1	   // 丁羆璸﹚竟

#define PROTIMS_RETRY_NUMBER			 2
#define PROTIMS_RECON_NUM				 5    // 礚絬硈钡Ω计

#define PROTIMS_HANDSHAKE_RETRY_TIMES 15  // 搐も祇Ω计
#define PROTIMS_DATA_RETRY_TIMES      3   // 计沮祇Ω计
#define PROTIMS_GET_PLAN_RETRY_TIMES  10  // 莉璸购沽刚Ω计
#define PROTIMS_RECV_CONTROL_BYTE_TIMEOUT     2   // 北竊钡Μ禬丁200ms
#define PROTIMS_RECV_BYTE_TIMEOUT             80  // 钡Μ竊禬8

#define PROTIMS_TOTAL_SEND_PACKET_TIME_OUT    600 // 计沮祇癳程60
#define PROTIMS_TOTAL_RECEV_PACKET_TIME_OUT   600 // 计沮钡Μ程60
#define PROTIMS_ONE_RECEV_PACKET_TIME_OUT     300


// ProTims command code
#define PROTIMS_MSG_MODEM_OK              0x05
#define PROTIMS_MSG_LOAD_DATA             0x36
#define PROTIMS_MSG_AUTH_TERMINAL         0x37
#define PROTIMS_MSG_GET_TASK_TABLE        0x38
#define PROTIMS_MSG_DOWNLOAD_OK           0x3A
#define PROTIMS_MSG_SIN_AUTH_FLAG         0x3B  //帽粄靡㏑絏 
												//20130416
#define PROTIMS_MSG_GET_MULTITASK_TABLE   0X3C  //叫―莉ヴ叭

#define PROTIMS_MSG_NOTIFY_UPDATE_REQ     0x3d   
#define PROTIMS_MSG_GET_PARA_INFO         0x3e   // 琩高把计獺㏑
#define PROTIMS_MSG_SAVE_PARA_INFO_STATUS 0x3f   // 把计獺玂篈㏑絏
#define PROTIMS_MSG_SPECIAL_COMMAND       0xF0   // 疭㏑絏

#define PROTIMS_MSG_UPLOAD_REQ        		0x40
#define PROTIMS_MSG_UPLOAD_DATA           	0x41

#define PROTIMS_MSG_GET_RSAKEY				0x51
#define PROTIMS_MSG_GET_MAINKEY				0x52
#define PROTIMS_MSG_GET_KEYSTATE				0x53

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

uchar ChkTerm(uchar ucTermType);	// 判断当前机型
uchar ChkHardware(uchar ucChkType, uchar ucValue);
uchar ChkIfIrDAPrinter(void);
uchar ChkTermPEDMode(uchar ucMode);
uchar ChkIfEmvEnable(void);// 判断EDC是否支持EMV交易
uchar ChkIssuerOption(ushort uiOption);
uchar ChkAcqOption(ushort uiOption);
uchar ChkEdcOption(ushort uiOption);
uchar ChkOptionExt(uchar *psExtOpt, ushort uiOption);
#define OPT_CLR     0
#define OPT_SET     1
void  SetOptionExt(uchar *psExtOpt, ushort uiOption, ushort uiAction);
uchar ChkIfNeedPIN(void);
uchar ChkIfPinReqdAllIssuer(void);
uchar ChkIfAmex(void);
uchar ChkIfBoc(void);
uchar ChkIfCiti(void);
uchar ChkIfCitiTaxi(void);
uchar ChkIfFubon(void);
uchar ChkIfDahOrBCM(void);
uchar ChkifBEADCC(void);
uchar ChkIfBea(void);
uchar ChkIfScb(void);
uchar ChkIfDCC_CITI(void);
uchar ChkIfWordCard(void);
uchar ChkIfUob(void);
uchar ChkIfUobIpp(void);
uchar ChkIfDiners(void);
uchar ChkIfJCB(void);
uchar ChkIfICBC(void);
uchar ChkIfICBC_MACAU(void);
uchar ChkIfWingHang(void);
uchar ChkIfShanghaiCB(void);
uchar ChkIfHSBC(void);
uchar ChkIfBelowMagFloor(void);
uchar ChkIfPrnSignature(void);
void  CheckCapture(void);
uchar ChkInstalmentAllAcq(void);
uchar ChkIfCurAcqEMVEnable(void);
uchar ChkIfDispMaskPan2(void);
uchar ChkIfInstalmentPara(void);
uchar ChkIfTransMaskPan(uchar ucCurPage);
uchar ChkCurAcqName(void *pszKeyword, uchar ucPrefix);
uchar ChkIfTranAllow(uchar ucTranType);
uchar ChkIfZeroAmt(uchar *pszIsoAmountStr);
uchar ChkIfBatchEmpty(void);
#ifdef SUPPORT_TABBATCH
uchar ChkIfTabBatchEmpty(void);
#endif
uchar ChkIfZeroTotal(void *pstTotal);
uchar ChkSettle(void);
uchar ChkIfNeedTip(void);
uchar ChkIfDccBOC(void);
uchar ChkIfDccAcquirer(void);
uchar ChkIfDccBocOrTas(void);
uchar ChkIfIccTran(ushort uiEntryMode);
uchar ChkIfPrnReceipt(void);
uchar ChkIfNeedReversal(void);
uchar ChkIfSaveLog(void);
uchar ChkIfThermalPrinter(void);
uchar ChkIfNeedSecurityCode(void);
uchar ChkIfNeedMac(void);
uchar ChkIfAcqNeedTC(void);
uchar ChkIfAcqNeedDE56(void);
uchar ChkIfAllowExit(void);
uchar ChkIfBnu(void);
uchar ChkIfCasinoMode(void);  //Lijy 2010-12-31
uchar ChkIfWLB(void);
uchar ChkIfCHB(void);
uchar ChkifWLBfalse(void);
int ChkIfIssuerVMJ(void);

uchar ChkIfCupDsb(void); //add by richard 20170221
uchar ChkIfCupChb(void);
uchar ChkIfCupWlb(void);
uchar ChkIfCupFubon(void);
uchar ChkIfIndirectCupAcq(void);

uchar ChkAnyIndirectCupAcq(void); //addec by jeff_xiehuan20170401
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _CHECKOPT_H

// end of file
