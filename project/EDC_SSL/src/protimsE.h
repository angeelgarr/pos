
//============================================
//  key values
//============================================

#ifndef _POSAPI_H
#define _POSAPI_H

#include <xui.h>

//================================================
//         structure for modem operation
//=================================================

// setup of communication parameter
typedef struct
{
    unsigned char bCommMode;        // 0:Serial, 1:Modem, 2:LAN, 3:GPRS, 4:CDMA, 5:PPP

    unsigned char *psAppName;       //the name of app, if it is set for null, it will download all task.
    unsigned char bLoadType;        //the type of download.bit0:app bit1:para bit2:datafile
    unsigned char psProtocol;       //the information of download protocol.0-protims
    unsigned char bPedMode;          //not used

    unsigned char bDMK_index;      //not used
    unsigned char sTransCode[5];    // not used

    unsigned char ucCallMode;    //not used

    unsigned char *psTermID;     // the terminal ID for downloaded task

    union
    {
        struct
        {
            unsigned char *psPara; //the para for serial connection.
        }tSerial;      // bCommMode = 0
        struct
        {
            unsigned char *psTelNo;         //the telephone number.
            COMM_PARA *ptModPara;  		//the para for Modem communication
            unsigned char bTimeout;         //
        }tModem;      //  bCommMode = 1
        struct
        {
            unsigned char *psLocal_IP_Addr;       //local IP
            unsigned short wLocalPortNo;           //not used
            unsigned char *psRemote_IP_Addr;      //remote IP
            unsigned short wRemotePortNo;          //remote Port
            unsigned char *psSubnetMask;          //subnetMask
            unsigned char *psGatewayAddr;         //gateway
        }tLAN; // bCommMode=2
        struct
        {
            unsigned char *psAPN;          //APN
            unsigned char *psUserName;     //username
            unsigned char *psPasswd;       //password
            unsigned char *psIP_Addr; 		//remote IP
            unsigned short nPortNo;			//remote Port
            unsigned char *psPIN_CODE;     //pin code
            unsigned char *psAT_CMD;       //AT command.
        }tGPRS; //bCommMode=3
        struct
        {
            unsigned char *psTelNo;        //telephone number
            unsigned char *psUserName;
            unsigned char *psPasswd;
            unsigned char *psIP_Addr;
            unsigned short nPortNo;
            unsigned char *psPIN_CODE;
            unsigned char *psAT_CMD;
        }tCDMA; //bCommMode=4
        struct
        {
            unsigned char *psTelNo;			//telephone number
            COMM_PARA *ptModPara; 			//the para for Modem communication
            unsigned char *psUserName;     //username
            unsigned char *psPasswd;       //password
            unsigned char *psIP_Addr;      //remote IP
            unsigned short nPortNo;         //remote Port
        }tPPP; // bCommMode=5
        struct //WIFI 参数bCommMode=6时该参数才有效
        {

            unsigned char *Wifi_SSID;        // SSID
            unsigned char *Wifi_BSSID;
            unsigned char *psPasswd;       //

//          unsigned char *Local_IP;       //
//
//          unsigned short Local_PortNo;

            unsigned char *Remote_IP_Addr;      //
            unsigned short RemotePortNo;          //
//          unsigned char *SubnetMask;          //
//          unsigned char *GatewayAddr;         //
//          unsigned char *Dns1;
//          unsigned char *Dns2;
//          unsigned char Encryption_Mode; //  1: WEP 64(ASCII); 2 WEP 128 (ASCII);    3  WPA_TKIP;  4 WPA_AES
//          unsigned char Encryption_Index; //
//          unsigned char DHCP_Flag; //
            unsigned char Wifi_Mode;
            unsigned char Wifi_SecMode;
            unsigned char Wifi_AuthMode;
            unsigned char KeyLen;

		}tWIFI;
        struct
        {
        	unsigned char *psTelNo;        //telephone number
            unsigned char *psUserName;
            unsigned char *psPasswd;
            unsigned char *psIP_Addr;
            unsigned short nPortNo;
            unsigned char *psPIN_CODE;
            unsigned char *psAT_CMD;
        }tWCDMA; //bCommMode=7
    }tUnion;
}T_INCOMMPARA;


//the API for remote download.
typedef int (*cTMSStart)(T_INCOMMPARA *ptInPara, int *handle);	    //
typedef int (*cTMSReset)(int handle);								//
typedef int (*cTMSSendPack) (int handle, unsigned char *buff, int SndLen);	//
typedef int (*cTMSRecvByte) (int handle, unsigned char *ch, int usTimeOut);	//
typedef int (*cTMSExitClose)(int handle);				            //
typedef int (*cTMSNetRecv)(int socket, void *buf, int size, int flags);	//

extern cTMSStart     pcTMSStart    ;
extern cTMSReset     pcTMSReset    ;
extern cTMSSendPack  pcTMSSendPack ;
extern cTMSRecvByte  pcTMSRecvByte ;
extern cTMSExitClose pcTMSExitClose;
extern cTMSNetRecv   pcTMSNetRecv  ;

int ProTimsRemoteLoadApp(T_INCOMMPARA *ptCommPara);
int ProTimsMain(XuiWindow *pWindow, int iKeyValue);
int ProTimsSetStyle(XuiWindow *XuiWindow);
int GetLoadedTaskNum(void);

#endif

