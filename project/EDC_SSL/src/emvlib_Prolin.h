/*****************************************************/
/* emvlib_prolin.h                                   */
/* Define the Application Program Interface          */
/* of EMV L2 for PAX Prolin POS terminals     		 */
/*****************************************************/

#ifndef _EMV_LIB_H
#define _EMV_LIB_H

//---------------------------------------------------------------------------
/******************************* Return Codes ******************************/
//---------------------------------------------------------------------------
#ifndef _CLSS_COMMON_H
#define EMV_OK                          0         //Success
#define ICC_RESET_ERR                   -1        //IC card reset is failed
#define ICC_CMD_ERR                     -2        //IC card command is failed
#define ICC_BLOCK                       -3        //IC card is blocked 
#define EMV_RSP_ERR                     -4        //Return code of IC card command is error
#define EMV_APP_BLOCK                   -5        //EMV Application is blocked
#define EMV_NO_APP                      -6        //There is no EMV Application in the IC card 
#define EMV_USER_CANCEL                 -7        //User canceled current operation or transaction
#define EMV_TIME_OUT                    -8        //User��s operation is timeout
#define EMV_DATA_ERR                    -9        //IC card data error
#define EMV_NOT_ACCEPT                  -10       //Transaction is not accepted
#define EMV_DENIAL                      -11       //Transaction is denied
#define EMV_KEY_EXP                     -12       //Key is Expired
#define EMV_NO_PINPAD                   -13       //No PIN Pad or PIN Pad can��t be used 
#define EMV_NO_PASSWORD                 -14       //No PIN Pad or PIN Pad can��t be used 
#define EMV_SUM_ERR                     -15       //Sum of Key verify is error
#define EMV_NOT_FOUND                   -16       //Appointed Data Element can��t be found
#define EMV_NO_DATA                     -17       //There is no data in the appointed Data Element
#define EMV_OVERFLOW                    -18       //Memory is overflow
#define NO_TRANS_LOG                    -19       //There is no Transaction log
#define RECORD_NOTEXIST                 -20       //Appointed log is not exist
#define LOGITEM_NOTEXIST                -21       //Appointed Label is not exist in current log record
#define ICC_RSP_6985                    -22       //Return Status Code of GPO is '6985'
#define EMV_FILE_ERR                    -24       //File error
#define EMV_PARAM_ERR                   -30       //Parameter error.

#endif //_CLSS_COMMON_H
#define EMV_API_ORDER_ERR               -42       //API is called in wrong order.
//---------------------------------------------------------------------------
/******************************* End ***************************************/
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
/******************************* Macros declaration ************************/
//---------------------------------------------------------------------------

#ifndef _CLSS_COMMON_H
#define MAX_REVOCLIST_NUM               30        //Maximum number of Issuer Public Key Certificate Recover List for EMV kernel
#define MAX_APP_NUM                     100       //Maximum number of application list.
#define MAX_APP_ITEMS                   17        //Maximum number of candidate list.

#define MAX_KEY_NUM                     7         //Maximum number of public key stored in CA public key list of kernel for Prolin platform.


#define PART_MATCH                      0x00      //Application selection matching flag (partial matching).
#define FULL_MATCH                      0x01      //Application selection matching flag (full matching).

//Transaction type
#define EMV_CASH                        0x01      //Cash
#define EMV_GOODS                       0x02      //Goods
#define EMV_SERVICE                     0x04      //Service
#define EMV_CASHBACK                    0x08      //CashBack
#define EMV_INQUIRY                     0x10      //Inquiry
#define EMV_TRANSFER                    0x20      //Transfer
#define EMV_PAYMENT                     0x40      //Payment
#define EMV_ADMIN                       0x80      //Administer
#define EMV_CASHDEPOSIT                 0x90      //Cash Deposit

//Online Result
#define REFER_APPROVE                   0x01      //Reference Return code (choose Approved)
#define REFER_DENIAL                    0x02      //Reference Return code (choose Denial)
#define ONLINE_APPROVE                  0x00      //Online Return code (Online Approved) 
#define ONLINE_FAILED                   0x01      //Online Return code (Online Failed)
#define ONLINE_REFER                    0x02      //Online Return code (Online Reference)
#define ONLINE_DENIAL                   0x03      //Online Return code (Online Denial)
#define ONLINE_ABORT                    0x04      //Compatible PBOC(Transaction Terminate)

//AC Type
#define AC_AAC                          0x00      //The AC type is AAC
#define AC_TC                           0x01      //The AC type is TC
#define AC_ARQC                         0x02      //The AC type is ARQC
#endif  //_CLSS_COMMON_H                                         	

#define AC_AAC_HOST                     0x03      //The AC type is AAC since the online result is ONLINE_DENIAL.

#define CLSS_SLOT                       0xff
#define CONTACT_SLOT                    0x00

#define EMV_PED_TIMEOUT                 0x01      // PCI verify offline PIN, PIN input timeout.
#define EMV_PED_WAIT                    0x02      // PCI verify offline PIN, PIN input interval not enough.
#define EMV_PED_FAIL                    0x03      // PCI verify offline PIN, other failure.


#define EMV_SCRIPT_PROC_UNIONPAY        1         //PBOC unionpay
#define EMV_SCRIPT_PROC_NORMAL          0

#define PED_RET_ERR_START               -300       //Start value of error code.

#define PED_RET_OK                       0         //Return values of PED are correct.

#define PED_RET_ERR_NO_PIN_INPUT   (PED_RET_ERR_START-5)   //No PIN input
#define PED_RET_ERR_INPUT_CANCEL   (PED_RET_ERR_START-6)   //Cancel to enter PIN.
#define PED_RET_ERR_WAIT_INTERVAL  (PED_RET_ERR_START-7)   //Calling function interval is less than minimum interval time.

#define PED_RET_ERR_INPUT_TIMEOUT  (PED_RET_ERR_START-15)  //PIN input timeout
#define PED_RET_ERR_NO_ICC         (PED_RET_ERR_START-16)  //IC card does not exist
#define PED_RET_ERR_ICC_NO_INIT    (PED_RET_ERR_START-17)  //IC card is not initialized
//---------------------------------------------------------------------------
/******************************* End ***************************************/
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
/******************************* Structure declaration *********************/
//---------------------------------------------------------------------------
#ifdef WIN32
#pragma warning(disable:4103)
#pragma pack(1)
#endif

#ifndef _APDU_DEFINE
#define _APDU_DEFINE
typedef struct
{
    unsigned char       Command[4];
    unsigned short      Lc;
    unsigned char       DataIn[512];
    unsigned short      Le;
}APDU_SEND;

typedef struct
{
    unsigned short      LenOut;
    unsigned char       DataOut[512];
    unsigned char       SWA;
    unsigned char       SWB;
}APDU_RESP;
#endif

#ifndef _CLSS_COMMON_H
typedef struct {
	unsigned char RID[5];                 //Registered Application Provider Identifier
	unsigned char KeyID;                  //Key Index
	unsigned char HashInd;                //Hash arithmetic indicator
	unsigned char ArithInd;               //RSA arithmetic indicator
	unsigned char ModulLen;               //Length of Module
	unsigned char Modul[248];             //Module
	unsigned char ExponentLen;            //Length of exponent
	unsigned char Exponent[3];            //Exponent
	unsigned char ExpDate[3];             //Expiration Date (YYMMDD) 
	unsigned char CheckSum[20];           //Check Sum of Key
}EMV_CAPK;

typedef  struct 
{
	unsigned char ucRid[5];               // Registered Application Provider Identifier
	unsigned char ucIndex;                // Certification Authenticate Public Key Index.
	unsigned char ucCertSn[3];            // Issuer Certificate Serial Number.
}EMV_REVOCLIST;

typedef struct CLSS_TRANSPARAM
{
	unsigned long  ulAmntAuth;            // Authorize amount, for Cash back, the amount required to include the amount of ulAmntOther.
	unsigned long  ulAmntOther;           // Other amount 
	unsigned long  ulTransNo;             // Transaction Sequence Counter(4 BYTE)
	unsigned char  ucTransType;           // Transaction type'9C',00-Goods/Services 01-Cash 09-Cash back
	unsigned char  aucTransDate[4];       // Transaction Date YYMMDD
	unsigned char  aucTransTime[4];       // Transaction time HHMMSS
}Clss_TransParam;
#endif //_CLSS_COMMON_H

typedef struct{
	unsigned char AppName[33];            //Local application name. The string ends with '\x00' and is 32 bytes in maximum.
	unsigned char AID[17];                //Application ID. 
	unsigned char AidLen;                 //the length of AID
	unsigned char SelFlag;                //Application selection flag (partial matching PART_MATCH or full matching FULL_MATCH)      
	unsigned char Priority;               //priority indicator
	unsigned char TargetPer;              //Target percent (0 �C 99) (provided by acquirer)
	unsigned char MaxTargetPer;           //Max target percent(0 �C 99) (provided by acquirer)
 	unsigned char FloorLimitCheck;        //Check the floor limit or not (1: check(default), 0 : not check)
	unsigned char RandTransSel;           //Perform random transaction selection or not (1: perform(default), 0 : not perform)
	unsigned char VelocityCheck;          //Perform velocity check or not (1 : perform(default), 0 not perform)
	unsigned long FloorLimit;             //Floor limit (provided by acquirer)
	unsigned long Threshold;              //Threshold (provided by acquirer)
 	unsigned char TACDenial[6];           //Terminal action code - denial
	unsigned char TACOnline[6];           //Terminal action code �C online
	unsigned char TACDefault[6];          //Terminal action code �C default
	unsigned char AcquierId[6];           //Acquirer identifier 
	unsigned char dDOL[256];              //terminal default DDOL
	unsigned char tDOL[256];              //terminal default TDOL
	unsigned char Version[3];             //application version
	unsigned char RiskManData[10];        //Risk management data
}EMV_APPLIST;

typedef struct{
	unsigned char aucAppPreName[17];      //Application preferred name, ending with '\0'.
	unsigned char aucAppLabel[17];        //Application label, end with '\0'.
	unsigned char aucIssDiscrData[244];   //Data in template "BFOC" or "73", in the format of length+value, where 1st byte for length and other bytes for value.
	unsigned char aucAID[17];             //AID of ICC
	unsigned char ucAidLen;               //Length of AID of ICC
}APPLABEL_LIST;

typedef struct{
	unsigned char MerchName[256];         //Merchant name
	unsigned char MerchCateCode[2];       //Merchant catalog code
	unsigned char MerchId[15];            //Merchant identification
	unsigned char TermId[8];              //Terminal identification
	unsigned char TerminalType;           //Terminal type
	unsigned char Capability[3];          //Terminal capabilities
	unsigned char ExCapability[5];        //Additional terminal capabilities
	unsigned char TransCurrExp;           //Transaction currency exponent
	unsigned char ReferCurrExp;           //Reference currency exponent     
	unsigned char ReferCurrCode[2];       //Reference currency code
	unsigned char CountryCode[2];         //Terminal country code 
	unsigned char TransCurrCode[2];       //Transaction currency code 
	unsigned long ReferCurrCon;           //the conversion quotients between transaction currency and reference currency (default : 1000)
	                                      //(the exchange rate of transaction currency to reference currency *1000)
	unsigned char TransType;              //Set current transaction type(EMV_CASH EMV_GOODS EMV_SERVICE EMV_GOODS&EMV_CASHBACK EMV_SERVICE&EMV_CASHBACK)
	unsigned char ForceOnline;            //Merchant force online (1 means always require online transaction)
	unsigned char GetDataPIN;             //Read the IC card PIN retry counter before verify the PIN or not (1 : read(default), 0 : not read)
	unsigned char SurportPSESel;          //Support PSE selection mode or not (1:support(default), 0:not support)
}EMV_PARAM;

typedef struct  
{
	unsigned char ucUseTermAIPFlg;        //0-TRM is based on AIP of card(default),1-TRM is based on AIP of Terminal
	unsigned char aucTermAIP[2];          //The bit4 of byte1 decide whether force to perform TRM: "08 00"-Yes;"00 00"-No(default)
	unsigned char ucBypassAllFlg;         //whether bypass all other pin when one pin has been bypassed 1-Yes,0-No
}EMV_EXTMPARAM;

typedef struct 
{
	unsigned char  ucBypassPin;           // 0-Not supported,1-Supported(default)
	unsigned char  ucBatchCapture;        // 0-Online data capture,1-batch capture(default)
	void *pvoid;
}EMV_MCKPARAM;

//PBOC Electronic Cash(EC) Terminal Support Indicator(TSI)
//PBOC Electronic Cash(EC) Terminal Transaction Limit(TTL)
typedef struct 
{
	unsigned char ucECTSIFlg;             // TSI value is exist or not
	unsigned char ucECTSIVal;             // TSI value
	unsigned char ucECTTLFlg;             // TTL value is exist or not
	unsigned long ulECTTLVal;             // TTL value
}EMV_TMECPARAM;

typedef struct{
	int MaxLen;                           //The maximum length for this tag
	unsigned short Tag;                   //Tag value
	unsigned short Attr;                  //The format of this data
	unsigned short usTemplate[2];         //The template which this tag belongs, fill zero if none.
	unsigned char ucSource;               //The source of data element(EMV_SRC_TM/EMV_SRC_ICC/EMV_SRC_ISS).
}ELEMENT_ATTR;

typedef struct
{
     unsigned int  modlen;                //Module length of public key
     unsigned char mod[256];              //Module of public key.
     unsigned char exp[4];                //Exponent of public key.
     unsigned char iccrandomlen;          //Length of random data from IC.
     unsigned char iccrandom[8];          //Random data from ICC.
}RSA_PINKEY;
//---------------------------------------------------------------------------
/******************************* End ***************************************/
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/****************************** Function declaration ***********************/
//---------------------------------------------------------------------------
//Parmeter and Data Management
int  EMVCoreInit(void);

void EMVGetParameter(EMV_PARAM *tParam);
void EMVSetParameter(EMV_PARAM *tParam);

int  EMVGetTLVData(unsigned short Tag, unsigned char *DataOut, int *OutLen);
int  EMVSetTLVData(unsigned short Tag, unsigned char *DataIn, int DataLen);

int  EMVGetScriptResult(unsigned char *Result, int *RetLen);

int  EMVReadVerInfo(char *paucVer);

int  EMVAddIccTag(ELEMENT_ATTR tEleAttr[], int nAddNum);
void EMVSetScriptProcMethod(unsigned char ucScriptMethodIn);

int  EMVGetMCKParam(EMV_MCKPARAM *pMCKParam);
int  EMVSetMCKParam(EMV_MCKPARAM *pMCKParam);

int  EMVSetTmECParam(EMV_TMECPARAM *tParam);

void EMVSetConfigFlag(int nConfigflag);
int  EMVGetParamFlag(unsigned char ucParam, int *pnFlg);

//Terminal Application List Management
int  EMVAddApp(EMV_APPLIST *App);
int  EMVGetApp(int Index, EMV_APPLIST *App);
int  EMVDelApp(unsigned char *AID, int AidLen);
int  EMVDelAllApp(void);
int  EMVModFinalAppPara(EMV_APPLIST *ptEMV_APP);
int  EMVGetFinalAppPara(EMV_APPLIST *ptEMV_APP);
int  EMVGetLabelList(APPLABEL_LIST  *ptAppLabel, int *pnAppNum); 

//Certification Authority (CA) Public Key Management
int  EMVAddCAPK(EMV_CAPK  *capk );
int  EMVGetCAPK(int Index, EMV_CAPK  *capk );
int  EMVDelCAPK(unsigned char KeyID, unsigned char *RID);
int  EMVCheckCAPK(unsigned char *KeyID, unsigned char *RID);

//Terminal Revoked Issuer Public Key Certification List Management
int  EMVAddRevocList(EMV_REVOCLIST *pRevocList);
int  EMVDelRevocList(unsigned char ucIndex, unsigned char *pucRID);
void EMVDelAllRevocList(void);

//Transaction Processing
void EMVInitTLVData(void);
int  EMVAppSelect(int Slot,unsigned long TransNo);
int  EMVReadAppData(void);
int  EMVCardAuth(void);
int  EMVSetAmount(unsigned char * szAuthAmount, unsigned char * szCashBackAmount);
int  EMVStartTrans(unsigned long ulAuthAmt, unsigned long ulCashBackAmt, unsigned char *pACType); 
int  EMVCompleteTrans(int nCommuStatus, unsigned char *paucScript, int *pnScriptLen, unsigned char *pACType); 
int  EMVSwitchClss(Clss_TransParam *ptTransParam,unsigned char *pucSelData, int nSelLen, unsigned char *pucGPOData, int nGPOLen);

int  EMVGetCardECBalance(unsigned long *plBalance);

int  EMVAppSelectForLog(int Slot, unsigned char ucFlg);
int  ReadLogRecord(int RecordNo);
int  GetLogItem(unsigned short Tag, unsigned char *TagData, int *TagLen);

int  EMVReadSingleLoadLog(int nRecordNoIn);
int  EMVGetSingleLoadLogItem(unsigned short usTagIn, unsigned char *paucDataOut, int *pnLenOut);
int  EMVReadAllLoadLogs(unsigned char *paucLogDataOut, int *pnLenOut);
int  EMVGetLogData(unsigned char *paucLogDataOut, int *pnLenOut);

//Callback Functions
int  cEMVWaitAppSel(int TryCnt, EMV_APPLIST List[], int AppNum);  
int  cEMVInputAmount(unsigned long *AuthAmt, unsigned long *CashBackAmt);
int  cEMVGetHolderPwd(int TryFlag, int RemainCnt, unsigned char *pin);

void cEMVAdviceProc(void);
void cEMVVerifyPINOK(void);
int  cEMVUnknowTLVData(unsigned short Tag, unsigned char *dat, int len);
int  cCertVerify(void);
int  cEMVSetParam(void); 

unsigned char cEMVPiccIsoCommand(unsigned char cid, APDU_SEND *ApduSend, APDU_RESP *ApduRecv);
unsigned char cEMVIccIsoCommand(unsigned char ucslot, APDU_SEND *tApduSend, APDU_RESP *tApduRecv);
unsigned char cEMVSM2Verify(unsigned char *paucPubkeyIn,unsigned char *paucMsgIn,int nMsglenIn,
							   unsigned char *paucSignIn, int nSignlenIn);
unsigned char cEMVSM3(unsigned char *paucMsgIn, int nMsglenIn,unsigned char *paucResultOut);

int cEMVPedVerifyPlainPin(unsigned char ucIccSlot, unsigned char *pucExpPinLenIn, unsigned char *ucIccRespOut, unsigned char ucMode,unsigned long ulTimeoutMs);
int cEMVPedVerifyCipherPin(unsigned char ucIccSlot, unsigned char *pExpPinLenIn, RSA_PINKEY *tRsaPinKeyIn, unsigned char *pucIccRespOut, unsigned char ucMode,unsigned long ulTimeoutMs);

//Debug Interface
void EMVGetICCStatus(unsigned char *SWA, unsigned char *SWB);
int  EMVGetVerifyICCStatus(unsigned char *pucSWA, unsigned char *pucSWB);
int  EMVGetDebugInfo(int nExpAssistInfoLen, unsigned char *paucAssistInfo, int *pnErrorCode);
//---------------------------------------------------------------------------
/******************************* End ***************************************/
//---------------------------------------------------------------------------
#endif

