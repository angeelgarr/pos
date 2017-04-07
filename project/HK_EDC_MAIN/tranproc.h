
/****************************************************************************
NAME
    tranproc.h - 定义交易处理模块

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.12      - created
****************************************************************************/

#ifndef _TRANPROC_H
#define _TRANPROC_H

/************************************************************************
 * 定义交易类型 查询 批上送 签到 预授权 授权销售 退款 冲正 销售 结算 测试
 * 撤消预授权 撤消
************************************************************************/
// !!!! Every time a new transaction type is added, glTranConfig[] also add a new item accordingly.
enum
{
    PREAUTH,
    AUTH,
    SALE,
    INSTALMENT,
    RATE_BOC,
    RATE_SCB,
    UPLOAD,
    LOGON,
    REFUND,
    REVERSAL,
    SETTLEMENT,
    LOAD_PARA,
    VOID,
    OFFLINE_SEND,
    OFF_SALE,
    SALE_COMP,
    CASH,
    SALE_OR_AUTH,
    TC_SEND,
    ECHO_TEST,
    ENQUIRE_BOC_RATE,
    LOAD_CARD_BIN,
    LOAD_RATE_REPORT,
    PREAUTH_VOID,
    PREAUTH_COMP,
    PERAUTH_COMP_VOID,
    // 在此之前加新的交易类型宏名称. ADD NEW TRANS BEFORE MAX_TRANTYPE
    // 必须同步地在glTranConfig增加交易配置. Must sync with glTranConfig
    MAX_TRANTYPE,
};

// 错误代码(具体定义待定!!!!!)
#define ERR_BASE            0x10000
#define ERR_PINPAD          (ERR_BASE+0x01)
#define ERR_NO_TELNO        (ERR_BASE+0x03)
#define ERR_SWIPECARD       (ERR_BASE+0x05)
#define ERR_USERCANCEL      (ERR_BASE+0x06)
#define ERR_TRAN_FAIL       (ERR_BASE+0x07)
#define ERR_UNSUPPORT_CARD  (ERR_BASE+0x08)
#define ERR_SEL_ACQ         (ERR_BASE+0x09)
#define ERR_HOST_REJ        (ERR_BASE+0x0A)

#define ERR_FILEOPER        (ERR_BASE+99)
// #define ERR_NOT_EMV_CARD (ERR_BASE+100)
#define ERR_NEED_INSERT     (ERR_BASE+101)
#define ERR_NEED_FALLBACK   (ERR_BASE+102)
#define ERR_NEED_SWIPE      (ERR_BASE+103)
#define ERR_NEED_DOMESTIC   (ERR_BASE+104)  //Build88S 1.0D: Ivy Add 20120201

#define ERR_CLSS_AMT    (ERR_BASE+105)//Jason 2014.08.21 11:23
#define ERR_EXIT_APP        (ERR_BASE+990)
#define ERR_NO_DISP         (ERR_BASE+999)
/*----------------2014-5-20 IP encryption----------------*/
#ifdef IP_ENCRYPT
//Jerome
#define ERR_ENCRYPT         (ERR_BASE+1116)
//end of Jerome
#endif
/*----------------2014-5-20 IP encryption----------------*/

#define OFFSEND_TC      0x01
#define OFFSEND_TRAN    0x02

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void SetCommReqField(void);
int TransInit(uchar ucTranType);
int TransCapture(void);
int InstallmentMenu(void);
int TransSale(uchar ucInstallment);
int TransCash(void);
int TransAuth(uchar ucTranType);
int FinishOffLine(void);
int TranReversal(void);
int OfflineSend(uchar ucTypeFlag);
int TranSaleComplete(void);
int TransRefund(void);
int TransOffSale(void);
int TransVoid(void);
int TransOther(void);
int AdjustInput(void);
void TransAdjust(void);
void TransEchoTest(void);
int TransSettle(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // _TRANPROC_H

// end of file
