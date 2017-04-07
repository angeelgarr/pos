
// 定义全局变量、常量等
#include "global.h"
//linda
MULTIAPP_EVENTS      MultiAppEvent;

SYS_PARAM		glSysParam, glSysParamBak;		// 系统配置参数
SYS_CONTROL		glSysCtrl;		// 系统控制参数
SYS_PROC_INFO	glProcInfo;		// 交易处理信息

uchar  glClssCVMFlag;//2016-4-25 CLSS_CVM_SIG
#ifdef AMT_PROC_DEBUG//2014-11-4
ECR_AMT_LOG      glEcrAMT;//2014-11-4MAX_TRANLOG
uchar glEcrAmtCnt;
#endif
COMM_DATA		glSendData, glRecvData, glSendDataDebug, glRecvDataDebug;		// 通讯数据//2014-9-1 
STISO8583		glSendPack;		// 发送报文
STISO8583		glRecvPack;		// 接收报文

STTMS8583		glTMSSend;		// TMS发送报文
STTMS8583		glTMSRecv;		// TMS接收报文
 

ST_EVENT_MSG	*glEdcMsgPtr;	// 管理器事件

ACQUIRER		glCurAcq;		// 当前收单行
ISSUER			glCurIssuer;	// 当前发卡行

COMM_CONFIG		glCommCfg;		// 当前通讯配置参数

TOTAL_INFO		glAcqTotal[MAX_ACQ];		// 收单行汇总信息
TOTAL_INFO		glIssuerTotal[MAX_ISSUER];	// 发卡行汇总信息
TOTAL_INFO		glEdcTotal;		// 终端汇总信息
TOTAL_INFO		glTransTotal;	// 交易处理使用的汇总信息
TOTAL_INFO		glPrnTotal;		// 单据打印使用的汇总信息
		
uchar           glLastPageOfSettle;
/*----------------2014-5-20 IP encryption----------------*/
#ifdef IP_ENCRYPT
int glKeyInjectOK;//2015-11-17 add a global flag to identify if key injection is success
//defined by Jerome
EFTSec_Control glEFTSec_Control;   //盞厨ゅEDS獺㎝盞芲獺

//end of Jerome's define
#endif
/*----------------2014-5-20 IP encryption----------------*/
#ifdef ENABLE_EMV
EMV_PARAM		glEmvParam;
EMV_STATUS		glEmvStatus;
#endif
int HardCodeFlag = 0; //Gillian 20160803 

uchar			glAmount[13];
uchar			bFirstPollFlag;
CLSS_INFO		glClssInfo;

uchar  sg_bWaveInitPara = 0;

int	iWLBfd;
uchar ucWLBBuf[2];		//临时buf
uchar glTSI[2];  //Gillian 20161020
uchar glTVR[5];  //Gillian 20161020
uchar glCVMR[5];
uchar glCVR[32];
int INSTAL_VOID = 0;
uchar glTemp[1]; //Gillian 201611101
uchar    glKeyValue;
#ifdef _S_SERIES_                 //raymond 27 Jun 2011: SXX EPS implementation
uchar glReversalPending;       //0 - no reversal pending, 1 - reversal pending
#endif
#ifdef PAYPASS_DEMO
	uchar gucIsPayWavePass;
#endif

//W204
int			EPS_AutoRecap;
char			EPS_Recap_Time[4+1];
char			EPS_Last_Recap_Date[4+1];
//end W204


// 所有支持的语言列表
const LANG_CONFIG glLangList[] = 
{
	{"English",      "",               CHARSET_WEST},
#if defined(_P60_S1_) || defined(_P70_S_)
	{"Chinese Trad", "CHS.LNG",        CHARSET_GB2312},		// in P60s1, use GB2312 code but traditional appearance.
#else
	{"Chinese Simp", "CHS.LNG",        CHARSET_GB2312},
	{"Chinese Trad", "CHT.LNG",        CHARSET_BIG5},
#endif
	{"Vietnamese",   "VIETNAMESE.LNG", CHARSET_VIETNAM},
	{"Thai",         "THAI.LNG",       CHARSET_WEST},
	{"Japanese",     "JAPANESE.LNG",   CHARSET_SHIFT_JIS},
	{"Korea",        "KOREA.LNG",      CHARSET_KOREAN},
	{"Arabia",       "ARABIA.LNG",	   CHARSET_ARABIA},
	{"", "", 0},
	// need pending
};

const CURRENCY_CONFIG glCurrency[] =
{
	"HKD", "\x03\x44", "\x03\x44", 2, 0, //Hong Kong Dollars
	"HK",  "\x03\x44", "\x03\x44", 2, 0,
	"HK$", "\x03\x44", "\x03\x44", 2, 0,
	"CNY", "\x01\x56", "\x01\x56", 2, 0, //Chinese Yuan
	"RMB", "\x01\x56", "\x01\x56", 2, 0,
	"JPY", "\x03\x92", "\x03\x92", 0, 2, //Japanese Yen				// special
	"EUR", "\x09\x78", "\x00\x00", 2, 0, //Euro -- Country not determined
	"MOP", "\x04\x46", "\x04\x46", 2, 0, //Macao Pataca
	"MYR", "\x04\x58", "\x04\x58", 2, 0, //Malaysian Ringgit
	"PHP", "\x06\x08", "\x06\x08", 2, 0, //Philippine Pesos
	"SGD", "\x07\x02", "\x07\x02", 2, 0, //Singapore Dollars
	"THB", "\x07\x64", "\x07\x64", 2, 0, //Thai Baht
	"TWD", "\x09\x01", "\x01\x58", 2, 0, //New Taiwanese Dollars
	"NT",  "\x09\x01", "\x01\x58", 2, 0,
	"NT$", "\x09\x01", "\x01\x58", 2, 0,
	"USD", "\x08\x40", "\x08\x40", 2, 0, //US Dollars
	"VND", "\x07\x04", "\x07\x04", 0, 2, //Vietnam DONG				// special
	"AED", "\x07\x84", "\x07\x84", 2, 0, //United Arab Durham
	"AUD", "\x00\x36", "\x00\x36", 2, 0, //Australian Dollars
	"CAD", "\x01\x24", "\x01\x24", 2, 0, //Canadian Dollars
	"CYP", "\x01\x96", "\x01\x96", 2, 0, //Cypriot Pounds
	"CHF", "\x07\x56", "\x07\x56", 2, 0, //Swiss Francs
	"DKK", "\x02\x08", "\x02\x08", 2, 0, //Danish Krone
	"GBP", "\x08\x26", "\x08\x26", 2, 0, //British Pounds Sterling
	"IDR", "\x03\x60", "\x03\x60", 0, 0, //Indonesia Rupiah			// special //decimal 0 or 2??????
	"INR", "\x03\x56", "\x03\x56", 2, 0, //Indian Rupee
	"ISK", "\x03\x52", "\x03\x52", 2, 0, //Icelandic krone
	"KRW", "\x04\x10", "\x04\x10", 0, 2, //South Korean Won			// special
	"LKR", "\x01\x44", "\x01\x44", 2, 0, //Sri-Lanka Rupee
	"MTL", "\x04\x70", "\x04\x70", 2, 0, //Maltese Lira
	"NOK", "\x05\x78", "\x05\x78", 2, 0, //Norwegian Krone
	"NZD", "\x05\x54", "\x05\x54", 2, 0, //New Zealand Dollars
	"RUB", "\x06\x43", "\x06\x43", 2, 0, //Russian Ruble
	"SAR", "\x06\x82", "\x06\x82", 2, 0, //Saudi Riyal
	"SEK", "\x07\x52", "\x07\x52", 2, 0, //Swedish krone
	"TRL", "\x07\x92", "\x07\x92", 2, 0, //Turkey Lira
	"VEF", "\x09\x37", "\x08\x62", 2, 0, //Bolivar Fuerte (Venezuela)
	"ZAR", "\x07\x10", "\x07\x10", 2, 0, //South African Rand
	"KWD", "\x04\x14", "\x04\x14", 3, 0, //Kuwaiti Dinar			// special
	"CLP", "\x01\x52", "\x01\x52", 0, 2, //Chilean Piso				//Gillian 2016-7-14// special//2016-3-14 for BEA DCC //2016-3-24 remove tempt
	"VEB", "\x08\x62", "\x08\x62", 2, 0, //Venezuela Bolivar
	"QAR", "\x06\x34", "\x06\x34", 2, 0, //Qatari Rial - Qatar
	"KZT", "\x03\x98", "\x03\x98", 2, 0, //Tenge - Kazakhstan
	"EGP", "\x08\x18", "\x08\x18", 2, 0, //Egyptian Pound - Egypt
	"OMR", "\x05\x12", "\x05\x12", 3, 0, //Rial Omani - Oman
	"BHD", "\x00\x48", "\x00\x48", 3, 0, //Bahraini Dinar - Bahrain
	"JOD", "\x04\x00", "\x04\x00", 3, 0, //Jordanian Dinar - Jordan
	"PKR", "\x05\x86", "\x05\x86", 2, 0, //Pakistan Rupee - Pakistan
	"LBP", "\x04\x22", "\x04\x22", 2, 0, //Lebanese Pound - Lebanon
	"BRL", "\x09\x86", "\x09\x86", 2, 0, //Brzilian Real - Brazil
	"UAH", "\x09\x80", "\x09\x80", 2, 0, //Hryvnia - Ukraine
	"CZK", "\x02\x03", "\x02\x03", 2, 0,//Czech Koruna - Czech Republic
	"LVL", "\x04\x28", "\x04\x28", 2, 0, //Latvian Lats
	"MUR", "\x04\x80", "\x04\x80", 2, 0, //Mauritius - Mauritius Rupee
	//2016-3-24 remove tempt
	////2016-3-14 BEA new requests
	//"MVR","\x04\x62","\x04\x62", 2, 0,//Rufiyaa
	//"ILS","\x03\x76","\x03\x76", 2, 0,//Israel
	//"MXN","\x04\x84","\x04\x84", 2, 0,//
	//"TRY","\x09\x49","\x09\x49", 2, 0,//
	//"BDT","\x00\x50","\x00\x50", 2, 0,//
	//"BND","\x00\x96","\x00\x96", 2, 0,
	//"NGN","\x05\x66","\x05\x66", 2, 0,
	//"KES","\x04\x04","\x04\x04", 2, 0,
	//"GHS","\x09\x36","\x09\x36", 2, 0,
	//"NAD","\x05\x16","\x05\x16", 2, 0,
	//"XOF","\x09\x52","\x09\x52", 2, 0,//the Ivory Coast
	//"TZS","\x08\x34","\x08\x34", 2, 0,
	//"RON","\x09\x46","\x09\x46", 2, 0,
	//"PLN","\x09\x85","\x09\x85", 2, 0,//Polish zloty 
	//"ARS","\x00\x32","\x00\x32", 2, 0,
	//"XAF","\x09\x50","\x09\x50", 2, 0,
	//"MZN","\x09\x43","\x09\x43", 2, 0,
	//"SCR","\x06\x90","\x06\x90", 2, 0,
	//"COP","\x01\x70","\x01\x70", 2, 0,//Columbia
	//"AOA","\x09\x73","\x09\x73", 2, 0,
	//"HUF","\x03\x48","\x03\x48", 2, 0,//Hungarian forint
	//"RWF","\x06\x46","\x06\x46", 2, 0,
	//"BWP","\x00\x72","\x00\x72", 2, 0,
	//"HRK","\x01\x91","\x01\x91", 2, 0,
	//"PGK","\x05\x98","\x05\x98", 2, 0,
	//"BGN","\x09\x75","\x09\x75", 2, 0,
	//"JMD","\x03\x88","\x03\x88", 2, 0,//Jamaican dollar
	//"TTD","\x07\x80","\x07\x80", 2, 0,//Trinidad
	//"PAB","\x05\x90","\x05\x90", 2, 0,//Panama
	//"DOP","\x02\x14","\x02\x14", 2, 0,
	//"CRC","\x01\x88","\x01\x88", 2, 0,//Costa rica
	//"RSD","\x09\x41","\x09\x41", 2, 0,
	//"PEN","\x06\x04","\x06\x04", 2, 0,
	//"BMD","\x00\x60","\x00\x60", 2, 0,
	//"BIF","\x01\x08","\x01\x08", 2, 0,
	//"FJD","\x02\x42","\x02\x42", 2, 0,
	//"TND","\x07\x88","\x07\x88", 2, 0,//Tunisia
	//"PYG","\x06\x00","\x06\x00", 2, 0,//Paraguay
	//"BAM","\x09\x77","\x09\x77", 2, 0,
	//"BZD","\x00\x84","\x00\x84", 2, 0,
	//"XCD","\x09\x51","\x09\x51", 2, 0,
	//"BOB","\x00\x68","\x00\x68", 2, 0,//Boliviano
	//"MAD","\x05\x04","\x05\x04", 2, 0,
	//"ALL","\x00\x08","\x00\x08", 2, 0,
	//"DZD","\x00\x12","\x00\x12", 2, 0,
	//"MWK","\x04\x54","\x04\x54", 2, 0,
	//"MNT","\x04\x96","\x04\x96", 2, 0,
	//"MDL","\x04\x98","\x04\x98", 2, 0,
	//"NIO","\x05\x58","\x05\x58", 2, 0,
	//"ZWL","\x09\x32","\x09\x32", 2, 0,
	//"UZS","\x08\x60","\x08\x60", 2, 0,
	//"UYU","\x08\x58","\x08\x58", 2, 0,
	//"GTQ","\x03\x20","\x03\x20", 2, 0,//Guatemala
	//"HNL","\x03\x40","\x03\x40", 2, 0,//Honduras
    //Gillian 2016-7-14
	//2016-3-14 BEA new requests
	"MVR","\x04\x62","\x04\x62", 2, 0,//Rufiyaa
	"ILS","\x03\x76","\x03\x76", 2, 0,//Israel
	"MXN","\x04\x84","\x04\x84", 2, 0,//
	"TRY","\x09\x49","\x09\x49", 2, 0,//
	"BDT","\x00\x50","\x00\x50", 2, 0,//
	"BND","\x00\x96","\x00\x96", 2, 0,
	"NGN","\x05\x66","\x05\x66", 2, 0,
	"KES","\x04\x04","\x04\x04", 2, 0,
	"GHS","\x09\x36","\x09\x36", 2, 0,
	"NAD","\x05\x16","\x05\x16", 2, 0,
	"XOF","\x09\x52","\x09\x52", 2, 0,//the Ivory Coast
	"TZS","\x08\x34","\x08\x34", 2, 0,
	"RON","\x09\x46","\x09\x46", 2, 0,
	"PLN","\x09\x85","\x09\x85", 2, 0,//Polish zloty 
	"ARS","\x00\x32","\x00\x32", 2, 0,
	"XAF","\x09\x50","\x09\x50", 2, 0,
	"MZN","\x09\x43","\x09\x43", 2, 0,
	"SCR","\x06\x90","\x06\x90", 2, 0,
	"COP","\x01\x70","\x01\x70", 2, 0,//Columbia
	"AOA","\x09\x73","\x09\x73", 2, 0,
	"HUF","\x03\x48","\x03\x48", 2, 0,//Hungarian forint
	"RWF","\x06\x46","\x06\x46", 2, 0,
	"BWP","\x00\x72","\x00\x72", 2, 0,
	"HRK","\x01\x91","\x01\x91", 2, 0,
	"PGK","\x05\x98","\x05\x98", 2, 0,
	"BGN","\x09\x75","\x09\x75", 2, 0,
	"JMD","\x03\x88","\x03\x88", 2, 0,//Jamaican dollar
	"TTD","\x07\x80","\x07\x80", 2, 0,//Trinidad
	"PAB","\x05\x90","\x05\x90", 2, 0,//Panama
	"DOP","\x02\x14","\x02\x14", 2, 0,
	"CRC","\x01\x88","\x01\x88", 2, 0,//Costa rica
	"RSD","\x09\x41","\x09\x41", 2, 0,
	"PEN","\x06\x04","\x06\x04", 2, 0,
	"BMD","\x00\x60","\x00\x60", 2, 0,
	"BIF","\x01\x08","\x01\x08", 2, 0,
	"FJD","\x02\x42","\x02\x42", 2, 0,
	"TND","\x07\x88","\x07\x88", 2, 0,//Tunisia
	"PYG","\x06\x00","\x06\x00", 2, 0,//Paraguay
	"BAM","\x09\x77","\x09\x77", 2, 0,
	"BZD","\x00\x84","\x00\x84", 2, 0,
	"XCD","\x09\x51","\x09\x51", 2, 0,
	"BOB","\x00\x68","\x00\x68", 2, 0,//Boliviano
	"MAD","\x05\x04","\x05\x04", 2, 0,
	"ALL","\x00\x08","\x00\x08", 2, 0,
	"DZD","\x00\x12","\x00\x12", 2, 0,
	"MWK","\x04\x54","\x04\x54", 2, 0,
	"MNT","\x04\x96","\x04\x96", 2, 0,
	"MDL","\x04\x98","\x04\x98", 2, 0,
	"NIO","\x05\x58","\x05\x58", 2, 0,
	"ZWL","\x09\x32","\x09\x32", 2, 0,
	"UZS","\x08\x60","\x08\x60", 2, 0,
	"UYU","\x08\x58","\x08\x58", 2, 0,
	"GTQ","\x03\x20","\x03\x20", 2, 0,//Guatemala
	"HNL","\x03\x40","\x03\x40", 2, 0,//Honduras
	"", "", 0, 0,
};

// !!!! Here must be one-one and accordant to the sequence of enum{PREAUTH,AUTH,...}
// !!!! See definition of "SALE","PREAUTH", ...
TRAN_CONFIG		glTranConfig[] =
{
	{_T_NOOP("PRE-AUTH"),       "0100", "300000", PRN_RECEIPT+ACT_INC_TRACE+NEED_REVERSAL},
	{_T_NOOP("AUTH"),           "0100", "000000", PRN_RECEIPT+ACT_INC_TRACE+NEED_REVERSAL},
	{_T_NOOP("SALE"),           "0200", "000000", PRN_RECEIPT+ACT_INC_TRACE+IN_SALE_TOTAL+NEED_REVERSAL+WRT_RECORD+VOID_ALLOW},
	{_T_NOOP("INSTAL"),         "0200", "000000", PRN_RECEIPT+ACT_INC_TRACE+IN_SALE_TOTAL+NEED_REVERSAL+WRT_RECORD+VOID_ALLOW},
	{_T_NOOP("ENQUIRE"),        "0800", "970000", ACT_INC_TRACE},
	{_T_NOOP("ENQUIRE"),        "0100", "000000", ACT_INC_TRACE},
	{_T_NOOP("UPLOAD"),         "0320", "000000", ACT_INC_TRACE},
	{_T_NOOP("LOGON"),          "0805", "920000", ACT_INC_TRACE}, //0800,  modified by richard 20161107
	{_T_NOOP("REFUND"),         "0200", "200000", PRN_RECEIPT+ACT_INC_TRACE+IN_REFUND_TOTAL+NEED_REVERSAL+WRT_RECORD+VOID_ALLOW},
	{_T_NOOP("REVERSAL"),       "0400", "000000", 0},
	{_T_NOOP("SETTLEMENT"),     "0500", "920000", ACT_INC_TRACE},
	{_T_NOOP("INITIALIZATION"), "0800", "920000", ACT_INC_TRACE},
	{_T_NOOP("VOID"),           "0200", "020000", PRN_RECEIPT+NEED_REVERSAL+ACT_INC_TRACE},
	{_T_NOOP("UPLOAD OFFLINE"), "0220", "000000", PRN_RECEIPT+ACT_INC_TRACE},
	{_T_NOOP("OFFLINE"),        "0220", "000000", PRN_RECEIPT+ACT_INC_TRACE+IN_SALE_TOTAL+WRT_RECORD+VOID_ALLOW},
	{_T_NOOP("SALECOMP"),       "0220", "000000", PRN_RECEIPT+ACT_INC_TRACE+IN_SALE_TOTAL+NEED_REVERSAL+WRT_RECORD+VOID_ALLOW},
	{_T_NOOP("CASH"),           "0200", "010000", PRN_RECEIPT+ACT_INC_TRACE+IN_SALE_TOTAL+NEED_REVERSAL+WRT_RECORD+VOID_ALLOW},
	{_T_NOOP("SALE_OR_AUTH"),   "0200", "000000", PRN_RECEIPT+ACT_INC_TRACE+IN_SALE_TOTAL+NEED_REVERSAL+WRT_RECORD+VOID_ALLOW},
	{_T_NOOP("TC-ADVICE"),      "0320", "940000", ACT_INC_TRACE},
	{_T_NOOP("ECHO-TEST"),      "0800", "990000", 0},
	{_T_NOOP("ENQUIRE"),        "0800", "930000", ACT_INC_TRACE},
	{_T_NOOP("BIN DOWNLOAD"),   "0900", "000000", ACT_INC_TRACE},
	{_T_NOOP("RATE REPORT"),    "0900", "010000", ACT_INC_TRACE},
    {_T_NOOP("PREAUTH-VOID"),   "0100", "200000", PRN_RECEIPT+ACT_INC_TRACE+NEED_REVERSAL},
    {_T_NOOP("PREAUTH-COMP"),   "0200", "000000", PRN_RECEIPT+ACT_INC_TRACE+NEED_REVERSAL},//modified by jeff_xiehuan20170330
    {_T_NOOP("PREAUTH-COMP-V"), "0200", "200000", PRN_RECEIPT+ACT_INC_TRACE+NEED_REVERSAL},
};

HOST_ERR_MSG	glHostErrMsg[] =
{
	{"00", _T_NOOP("TXN. ACCEPTED")},
	{"01", _T_NOOP("PLS CALL BANK")},
	{"02", _T_NOOP("CALL REFERRAL")},
	{"03", _T_NOOP("INVALID MERCHANT")},
	{"04", _T_NOOP("PLS PICK UP CARD")},
	{"05", _T_NOOP("DO NOT HONOUR")},
	{"08", _T_NOOP("APPROVED WITH ID")},
	{"12", _T_NOOP("INVALID TXN")},
	{"13", _T_NOOP("INVALID AMOUNT")},
	{"14", _T_NOOP("INVALID ACCOUNT")},
	{"19", _T_NOOP("RE-ENTER TRANS.")},
	{"21", _T_NOOP("APPROVED. IDLE")},
	{"25", _T_NOOP("INVALID TERMINAL")},
	{"30", _T_NOOP("FORMAT ERROR")},
	{"41", _T_NOOP("PLEASE CALL-LC")},
	{"43", _T_NOOP("PLEASE CALL-CC")},
	{"51", _T_NOOP("TXN DECLINED")},
	{"54", _T_NOOP("EXPIRED CARD")},
	{"55", _T_NOOP("INCORRECT PIN")},
	{"58", _T_NOOP("INVALID TXN")},
	{"60", _T_NOOP("CALL ACQUIRER")},
	{"76", _T_NOOP("BAD PRODUCT CODE")},
	{"77", _T_NOOP("RECONCILE ERROR")},
	{"78", _T_NOOP("TRACE NOT FOUND")},
	{"80", _T_NOOP("BAD BATCH NUMBER")},
	{"85", _T_NOOP("BATCH NOT FOUND")},
	{"88", _T_NOOP("APPRV, CALL AMEX")},
	{"89", _T_NOOP("BAD TERMINAL ID")},
	{"91", _T_NOOP("SYSTEM NOT AVAIL")},
	{"94", _T_NOOP("DUPLICATE TRACE")},
	{"95", _T_NOOP("BATCH TRANSFER")},
	{"96", _T_NOOP("System Error")},
	{"97", _T_NOOP("Host Unavailable")},
	{"N1", _T_NOOP("Not DCC Eligible")},	//  "DCC 无 效"
	{"Q1", _T_NOOP("CARD AUTH FAIL")},		//  "CARD AUTH FAIL"
	{"Y1", _T_NOOP("OFFLINE APPROVAL")},	//  "离线授权"
	{"Z1", _T_NOOP("OFFLINE DECLINE")},		//  "离线拒绝交易"
	{"Y2", _T_NOOP("APPROVED")},			//  "APPROVED"
	{"Z2", _T_NOOP("DECLINED")},			//  "DECLINED"
	{"Y3", _T_NOOP("GO ONLINE FAIL")},		//  "GO ONLINE FAIL"
	{"Z3", _T_NOOP("GO ONLINE FAIL")},		//  "GO ONLINE FAIL"
	{"NA", _T_NOOP("NO UPDATE")},			//  "无 效 信 息"
	{"P0", _T_NOOP("ERROR SERIAL NO.")},	//  "无 效 信 息"
	{"XY", _T_NOOP("Duplicate Trans")},		//  "无 效 信 息"
	{"XX", _T_NOOP("NO DCC SGD REQ")},		//  "无 效 信 息"
	{"**", _T_NOOP("NO RESPONSE CODE")},
	{"\0\0", _T_NOOP("PLS CALL BANK")},
};

HOST_ERR_MSG	glHostErrMsg_AE_Sett[] =
{
	{"00", _T_NOOP("TXN. ACCEPTED")},
	{"02", _T_NOOP("CALL REFERRAL")},
	{"25", _T_NOOP("TERM. INACTIVE")},
	{"95", _T_NOOP("BATCH TRANSFER")},
	{"96", _T_NOOP("ALREADY CLOSED")},
	{"\0\0", _T_NOOP("PLS CALL BANK")},
};

HOST_ERR_MSG	glHostErrMsg_AE_Tran[] =
{
	{"00", _T_NOOP("TXN. ACCEPTED")},
	{"01", _T_NOOP("PLS CALL ISSUER")},
	{"02", _T_NOOP("CALL REFERRAL")},
	{"03", _T_NOOP("ERR-S/E NUMBER")},
	{"04", _T_NOOP("PLS PICK UP CARD")},
	{"05", _T_NOOP("TXN DECLINED")},
	{"08", _T_NOOP("APPROVED.ID.SIG")},
	{"21", _T_NOOP("APPROVED. IDLE")},
	{"25", _T_NOOP("TERM. INACTIVE")},
	{"58", _T_NOOP("INVALID TXN")},
	{"78", _T_NOOP("ROC NOT FOUND")},
	{"88", _T_NOOP("APPRV, CALL AMEX")},
	{"96", _T_NOOP("INVALID MSG")},
	{"\0\0", _T_NOOP("PLS CALL BANK")},
};

// 终端错误信息
// error message for internal error.
TERM_ERR_MSG	glTermErrMsg[] = 
{
	{ERR_COMM_MODEM_OCCUPIED,  _T_NOOP("PHONE OCCUPIED")},
	{ERR_COMM_MODEM_NO_LINE,   _T_NOOP("TRY AGAIN - NC")},
	{ERR_COMM_MODEM_LINE,      _T_NOOP("TRY AGAIN - CE")},
	{ERR_COMM_MODEM_NO_ACK,    _T_NOOP("NO ACK")},
	{ERR_COMM_MODEM_LINE_BUSY, _T_NOOP("LINE BUSY")},
	{ERR_NO_TELNO,             _T_NOOP("NO TEL NO")},
	{ERR_USERCANCEL,           _T_NOOP("USER CANCELED")},
	{ERR_TRAN_FAIL,            _T_NOOP("PROCESS FAILED")},
	{0, ""},
};


void LoadConnectOnePara(unsigned char ucForTransUse)
{
    uchar buf[100], ret, len; //HASE GENERIC BUILD111
	char  Port_DCC[5] = "9457", Port_DCD[6] = "48081", Port_INST[4] = "443", Result[32]= {0};
	int MyRet=-1;

	if(!ChkIfAmex() && !ChkIfDiners())
	{
		if(glSysParam.stTxnCommCfg.ucEnableSSL_URL)   //Gillian 2016-8-18
		{
			memset(Result, 0, sizeof(Result));
		    memset(glSysParam.stTxnCommCfg.szPP_URL, 0, sizeof(glSysParam.stTxnCommCfg.szPP_URL));
#ifdef PP_SSL   //HASE GENERIC BUILD106

            if(ChkIfFubon())  //add fubon by richard 21161209
            {
                memcpy(glSysParam.stTxnCommCfg.szPP_URL, S90_3G_SSL_URL, strlen(S90_3G_SSL_URL));            
                memcpy(glSysParam.stTxnCommCfg.szPP_Port_DCC, S90_3G_SSL_PORT, strlen(S90_3G_SSL_PORT));
                strcpy(gCurComPara.LocalParam.szRemoteIP_1, S90_3G_SSL_URL);
    			strcpy(gCurComPara.LocalParam.szRemoteIP_2, S90_3G_SSL_URL);

                strcpy(glSysParam.stTxnCommCfg.szPP_Port_DCC, S90_3G_SSL_PORT);
    			strcpy(glSysParam.stTxnCommCfg.szPP_Port_DCD, S90_3G_SSL_PORT);
            }
            else 
            {
#ifdef PP_SSL_UAT
                memcpy(glSysParam.stTxnCommCfg.szPP_URL, "terminal.uat.planetpayment.com", strlen("terminal.uat.planetpayment.com"));            
                memcpy(glSysParam.stTxnCommCfg.szPP_Port, "30100", 5);
#else
                memcpy(glSysParam.stTxnCommCfg.szPP_URL, "terminal.planetpayment.net", strlen("terminal.planetpayment.net"));

    			DnsResolve(glSysParam.stTxnCommCfg.szPP_URL, Result, 32);
    	
    			strcpy(gCurComPara.LocalParam.szRemoteIP_1, Result);
    			strcpy(gCurComPara.LocalParam.szRemoteIP_2, Result);

                strcpy(glSysParam.stTxnCommCfg.szPP_Port_DCC, Port_DCC);
    			strcpy(glSysParam.stTxnCommCfg.szPP_Port_DCD, Port_DCD);
                //end BUILD111           
#endif
            }
		}
	}
	else if (ChkCurAcqName("AMEX_INST", FALSE))
	{
		memset(Result, 0, sizeof(Result));
		memset(glSysParam.stTxnCommCfg.szPP_URL, 0, sizeof(glSysParam.stTxnCommCfg.szPP_URL));
		//Gillian 20161011
		memcpy(glSysParam.stTxnCommCfg.szPP_URL, "qwww318.americanexpress.com", strlen("qwww318.americanexpress.com"));
		MyRet = DnsResolve(glSysParam.stTxnCommCfg.szPP_URL, Result, 32);
		strcpy(glCurAcq.stTxnTCPIPInfo[0].szIP, Result);
		strcpy(glCurAcq.stTxnTCPIPInfo[0].szPort, Port_INST);
		strcpy(glCurAcq.stTxnTCPIPInfo[1].szIP, Result);
		strcpy(glCurAcq.stTxnTCPIPInfo[1].szPort, Port_INST);
#endif  //end PP_SSL        
	}
}

void Richard_test()
{
    int i,iRet;
    int iModlueLen = 128;
    ST_RSA_KEY stRsaKey={0};
    RSA_KEY_DOWN_INFO rsa_down = {0};
    uchar szBuff[32 + 1], TmpBuff[32 + 1], ck_value[8 + 1];
    uchar randomkeybuf[16 + 1];
    uchar RSAData[256]={0};
    uchar szDataIn[256]={0};

//\x1D
    memcpy(rsa_down.PubKeyModule, "\x99\x4C\x9A\x71\xF5\xA5\x69\x17\x14\x14\xBF\xBC\x44\xFD\x90\xC2\xBB\x65\x2F\x18\x2E\xE3\x20\x8C\x52\x91\x37\xDE\x42\x29\xAC\xCF\x0C\x81\x2A\xED\xD7\x84\xE4\x3A\xED\x9E\x9B\x8C\xAE\x70\xE3\xB6\x23\xE9\x63\xB5\x86\x7C\x4C\x69\xD1\x7C\x5A\xDA\x52\x21\x47\x25\x37\xFD\xED\xAC\x1A\xAB\x19\xA9\xE0\x25\x58\x01\x0B\x1D\xC2\x18\x8F\x1B\x58\x0B\x2F\xB1\xBF\x02\xF4\x5C\x29\x96\xC4\x19\x93\x3F\x1C\x6D\x41\x6A\xC1\x93\x99\xEA\x7A\xDF\xC3\xB7\x2D\x88\x62\xE7\xBA\xDE\xA1\x8F\x61\xAB\x0E\x80\xE3\x1F\xF5\x6C\xD7\xFC\x23",127);
    memcpy(rsa_down.PubKeyExponential, "\x01\x00\x01", 3);
    //memcpy(rsa_down.PubKeyModule, "\xC8\x90\x20\x2E\xB6\x66\x51\xC4\xFC\x2C\xBE\x6D\xA9\xC5\xBE\x9C\xB6\x5D\x25\x7F\x40\x62\x6F\x47\x96\x12\x3A\x41\xBF\x2A\x43\x29\xDB\x84\xFB\x13\xA7\x99\x17\xEA\x7F\xEF\x27\xA6\x63\xDE\x65\x77\x67\x5F\xF2\x64\x2C\xAB\xDE\x9F\xCB\x8A\x30\x2D\x9F\xF4\x81\xBC\x94\xC5\x49\xF5\x11\xC1\xCE\x17\x6D\x48\xA7\xA6\x8A\x66\x29\x7C\xDE\xBA\xC0\xDD\x4D\x1A\x32\xFD\x9A\x34\x31\x6F\xF4\x47\x6C\x5E\x88\x48\xAB\x6E\x6F\xD1\xF7\xBE\xFA\x8C\x28\xE7\x02\x8B\xEA\x5F\xBC\x66\xF0\x60\x24\xDC\x66\x9E\x6F\x49\x88\xDD\xDA\x37\x4D\x57",iModlueLen);

    

    stRsaKey.iModulusLen = 1024;
    memcpy(&stRsaKey.aucModulus[512-127], rsa_down.PubKeyModule, 127);
    stRsaKey.iExponentLen = 24;
    memcpy(&stRsaKey.aucExponent[512-3], rsa_down.PubKeyExponential, 3);

    iRet = PedWriteRsaKey(1, &stRsaKey);
    PubDebugTx("PedWriteRsaKey: %d\n", iRet);

    for (i = 0; i < 16; i++)  //generating random data
    {
        szBuff[i] = (uchar)rand();    
    }

    memcpy(randomkeybuf, szBuff, 16);

    //encrypt 0 data with the random key, and get check value
    memset(TmpBuff, 0x00, sizeof(TmpBuff));
    PubDes(TRI_ENCRYPT, TmpBuff, randomkeybuf, ck_value);

    /* memset(dataIn_en, 0xFF, mod_len / 8);
    memcpy(dataIn_en, "\x00\x02", 2);
    memcpy(dataIn_en + 128 - 16 - 1, "\x00", 1);
    memcpy(dataIn_en + 128 - 16, randomkeybuf, 16);


   memcpy(inch, dataIn_en, mod_len / 8);
    memcpy(modulch, rsa_down.PubKeyModule, mod_len / 8);
    memcpy(exp, rsa_down.PubKeyExponential, 3);
    RSARecover(modulch, mod_len / 8, exp, 3, inch, outch);*/


    memset(szDataIn, 0xFF, iModlueLen);
    memcpy(szDataIn, "\x01\x02", 2);
    memcpy(szDataIn + iModlueLen - 16 - 1, "\x00", 1);
    memcpy(szDataIn + iModlueLen - 16, randomkeybuf, 16);

    
	iRet = PedRsaRecover(1, szDataIn, RSAData, NULL);

    PubDebugTx("PedRsaRecover:%d\n RSAData:%02X %02X %02X\n", iRet, RSAData[0], RSAData[1], RSAData[2]);
    
}

// end of file

