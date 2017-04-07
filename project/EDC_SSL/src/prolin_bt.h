#ifndef __PAX_BT_H__
#define __PAX_BT_H__






//��������ֵ�б�
#define ERR_BT_NONE	            0       //�޴���
#define ERR_BT_NOT_PAIR	        -3451   //����δ���
#define ERR_BT_MODE_ERR	        -3452	//ģʽ����
#define ERR_BT_STATE_FAIL	    -3453	//��ȡ״̬ʧ��
#define ERR_BT_SCAN_FAIL	    -3454	//ɨ��ʧ��
#define ERR_BT_SCAN_DOING	    -3455	//�Ѿ���ɨ��
#define BT_CONN_REJ_SAFE	    -3456	//��֤���ܾ�
#define BT_CONN_KEY_MISS	    -3457	//Զ�����붪ʧ
#define BT_CONN_CHANNEL_EXIST	-3458	//ͨ���Ѿ�����
#define BT_CONN_SDC_FAILED	    -3459	//�豸��������
#define BT_CONN_NO_RESPONE	    -3460	//Զ������Ӧ
#define ERR_BT_PAIR_AUTH	    -3461	//��֤ʧ��
#define ERR_BT_PAIR_TIMEOUT	    -3462	//��Գ�ʱ
#define ERR_BT_PAIR_DOING	    -3463	//�Ѿ����



//����״̬
typedef enum
{
	BT_STATE_CLOSED,		/*bluetooth is close*/
	BT_STATE_OPENED,		/*bluetooth is open*/
	BT_STATE_CONNTING,		/*bluetooth is connecting to another device*/
	BT_STATE_CONNECT_OK,	/*bluetooth's connecting is successful*/
} BT_STATE;



//��������
typedef enum
{
	BT_ACTION_NONE,				/*action none*/
	BT_ACTION_DISPLAY_ONLY,		/*show password only*/
	BT_ACTION_DISPLAY_YESNO,	/*confirm password, no input*/
	BT_ACTION_DISPLAY_PASSKEY,	/*show password only*/
	BT_ACTION_INPUT_PASSKEY,	/*input password only*/
} BT_ACTION;


//������֤��ʽ
typedef enum
{
	BT_PAIR_AUTH_DISPLAY_ONLY,	/*display only*/
	BT_PAIR_AUTH_DISPLAY_YESNO,	/*display yes/no to confirm*/
	BT_PAIR_AUTH_PASSKEY,		/*input password*/
	BT_PAIR_AUTH_NONE,			/*none*/
} BT_PAIR_AUTH;

//�����豸ɨ��ṹ
typedef struct
{
	char Name[64];			/*bluetooth device name*/
	char Mac[32];			/*bluetooth device mac address*/
} ST_BT_SCAN_RESULT;


//���������Ϣ�ṹ
typedef struct
{
	char Mac[32];		/*mac address*/
	char LinkKey[64];	/*link key*/
} ST_BT_PAIRED;



typedef int (*PAIR_FUNC)(BT_ACTION Action, 
                         char *Mac, 
                         char *Key);

typedef void (*DISCONN_FUNC)(void);

typedef void (*PAIR_RESULT_FUNC)(char *Mac, int Result);

typedef void (*CONN_RESULT_FUNC)(int Result);

typedef void (*SCAN_RESULT_FUNC)(
                                 ST_BT_SCAN_RESULT Scan[],
                                 int Count);







extern int OsBluetoothOpen(const char *BtName, int Visible);
int OsBluetoothSetCallback(PAIR_FUNC pOnPair, 
                           DISCONN_FUNC pOnDisConn, 
                           PAIR_RESULT_FUNC pPairRet, CONN_RESULT_FUNC pConnRet);
void OsBluetoothClose(void);
int OsBluetoothStartScan(SCAN_RESULT_FUNC pOnScan);
int OsBluetoothCheck(void);
int OsBluetoothSetPairAuth(BT_PAIR_AUTH Auth, 
                           int Mitm, 
                           char *LegacyPin);
int OsBluetoothStartPair(const char *Mac);
int OsBluetoothGetPairedList(ST_BT_PAIRED Result[], 
                             int ResultCount);
int OsBluetoothRemovePaired(const char *Mac);
int OsBluetoothStartSPPConnect(const char *Mac);
int OsBluetoothSPPDisconnect(void);
int OsBluetoothGetSPPDevice(char *OutSppDev, int OutSppDevSize);

int OsBluetoothGetLocalMac(char* sMac);

#endif //__PAX_BT_H__
