/****************************************************************************
NAME
    Lang_arabic_Salesman.h

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    LinHB     2012.12.31      - created
****************************************************************************/
#ifndef _LANG_ARABIC_SALESMAN_H 
#define _LANG_ARABIC_SALESMAN_H

#ifdef FARSI
#include "appLib.h"

MENU_ITEM OrderMenu_Arabic[]= 
{
	{ "��� ����",	 ID_CashOrder,		NULL },
	{ "Credit Order",ID_CreditOrder,	NULL },
	{ "Return Order",ID_ReturnOrder,	NULL },
	{ "Credit Transfer",ID_CreditTransfer,	NULL },
	{ "",			 0,					NULL },
};

MENU_ITEM OfflineMenu_Arabic[] = 
{
	{ "����� ���",		ID_OfflineOrder,			NULL },
	{ "����� �����",		ID_OfflineVisit,			NULL },
	{ "����� ��� ���",		ID_OfflineCancelLastOrder,	NULL },
	{ "����� ��������",		ID_OfflineBatchUpload,		NULL },
	{ "",				  0,						NULL },
};

MENU_ITEM CashCollectionMenu_Arabic[] = 
{
	{ "  By Merchant   ", ID_CashCollectionByMerchant,	NULL },
	{ "    By Order    ", ID_CashCollectionByOrder,		NULL },
	{ "",				  0,							NULL },
};

MENU_ITEM BankDepositMenu_Arabic[] = 
{
	{ "By Order Number ", ID_BankDepositByOrderNumber,			NULL },
	{ " Order By Batch ", ID_BankDepositOrderNumberByBatch,		NULL },
	{ " Cash. By Batch ", ID_BankDepositCashCollectionByBatch,	NULL },
	{ "",				  0,									NULL },
};

MENU_ITEM InformationMenu_Arabic[] =
{
	{ "��� ����",	ID_Pending_Orders_ECC,	NULL },
	{ "������� ������",	ID_Salesman_Infos_ECC,	NULL },
	{ "�� ���",		ID_SalesmanRoute,		NULL },
	{ "����� �����",	ID_PrintDiscount,		NULL },
	{ "  Change   PWD  ", ID_ChangePWD,		NULL },
	{ "",				  0,				NULL },
};

MENU_ITEM ReportMenu_Arabic[] =
{
	{ "������� �������",		ID_Offline_Batch_Upload_Rep,	NULL },
	{ "����� �������-����",	ID_Offline_Physical_Stock_Rep,	NULL },
	{ "����� ��������",		ID_Order_Rep,				NULL },
	{ "OrderReport_Prod", ID_Order_Rep_Prod,		NULL },
	{ "����� �������-������", ID_Online_Stocks_Rep,		NULL },
	{ "��� ��� �����",		ID_Last_Consign_Order_Rep,NULL },
	{ "���� ��������",		ID_Summary_Rep,			NULL },
	{ "",				  0,						NULL },
};

MENU_ITEM SERVER_CALLMenu_Arabic[]=
{
	{ "��� �������",	ID_SettingsDownload,	NULL },    
	{ "����� ������",	ID_SoftDownload,		NULL },
	//{ "����?������", ID_Config_Auto,		NULL },
	//{ "������� �������� ������������ �����", ID_First_Init__Auto,	NULL },
	{ "",				  0,					NULL },
};

// Added by Kim_LinHB 2013/10/25 v4.68
MENU_ITEM ConsignmentMenu_Arabic[]=
{
	{ "��� �����",	ID_ConsignOrder,	NULL },
	{ "����� �����",	ID_ReturnConsignOrder,NULL },
	{ "Consign Confirm ", ID_ConsignConfirm,		NULL },
	{ "Pendin ConsigOrd", ID_PendingConsignOrder,	NULL },
	{ "",				  0,					NULL },
};

MENU_ITEM MainMenu_Arabic[]= 
{
	{ "��� �����",	ID_Order,			NULL },
	{ "����� �����",	ID_Offline,			NULL },      
	{ "Cash Collection ", ID_CashCollection,	NULL },  
	{ "  Bank Deposit  ", ID_BankDeposit,		NULL },
	{ "����� �����",	ID_RecordVisit,		NULL },
	{ "  Consignment   ", ID_Consignment,	NULL},
	{ "����� ��� ���",	ID_CancelLastOrder,	NULL },
	{ "�������",		ID_Information,		NULL },
	{ "�����",		ID_Reports,			NULL },
	//	{ "�������", ID_Settings,			NULL },
	{ "�����",		ID_Maintenance,		NULL },
	{ "������",		ID_Transmission,	NULL },
	{ "",				  0,			NULL },
};


unsigned char STR_DEFAUT_NUMERO_PAD_PRINC_FA[] = "0156291628";
unsigned char STR_DEFAUT_NUMERO_PAD_SEC_FA[] = "0156291628";
unsigned char STR_DEFAUT_ADRESSE_X25_FA[] = "195314853500";
unsigned char STR_AUTHENTIFICATION_FA[] = "CONNECT SERV ABM";
unsigned char STR_EUR_FA[] = "EUR";
unsigned char STR_BLANC_FA[] = "                 ";
unsigned char STR_TAB_FA[] = "����� ������";
unsigned char STR_AUTHENTI_PAP_FA[] = "PAP";
unsigned char STR_AUTHENTI_SHAP_FA[] = "SHAP";
unsigned char STR_VIDE_FA[] = " ";
unsigned char STR_ETOILES_FA[] = "         *****";
unsigned char STR_GPRS_FA[] = "��� ����� �������";
unsigned char STR_PPP_FA[] = "PPP";
unsigned char STR_ETOILES2_FA[] = "************************";
unsigned char STR_TIRETS_FA[] = "------------------------";
unsigned char STR_CHOIX_OPERATEUR_FA[] = "����� ������ :";
unsigned char STR_PASSWORD_DISPLAY_FA[] = "���� ����";
unsigned char STR_ERROR_FA[] = "ERROR";
unsigned char STR_COUPER_LE_TICKET_FA[] = "�� �������";
unsigned char STR_PRESS_ENTER_FA[] = "���� ��� �����";
unsigned char STR_VAL_CANCEL_FA[] = "������/�����";
unsigned char STR_VAL_CANCEL_COR_FA[] = "������/�����/����� �";
unsigned char STR_IDENT_MANAGER_FA[] = "����� ������";
unsigned char STR_PASSWORD_SECRET_FA[] = "���� �� ������";
unsigned char STR_PASSWORDS_CASHIERS_FA[] = "���� �� ��������";
unsigned char STR_PASSWORD_CASHIER_FA[] = "���� �� ������";
unsigned char STR_TIME_SOFTWAREDOWNLOAD_FA[] = "��� ����� ���������";
unsigned char STR_TIME_SALESPOLLING_FA[] = "��� ���������";
unsigned char STR_TIME_SETTINGSDOWNLOAD_FA[] = "��� ����� ���������";
unsigned char STR_PRINT_PARAM_FA[] = "����� ���������";
unsigned char STR_NUMERO_PAD_FA[] = "PAD �����";
unsigned char STR_NUMERO_PAD_SECOURS_FA[] = "PAD �����";
unsigned char STR_MENU_PPP_FA[] = "PPP MENU:";
unsigned char STR_NUM_TLCHGT_FA[] = "����� �������:";
unsigned char STR_ID_TERMINAL_FA[] = "���� ������ :";
unsigned char STR_TYPE_IMPRIMANTE_FA[] = "��� �������:";
unsigned char STR_REINIT_APPLI_FA[] = "����� ����� ���������";
unsigned char STR_INFOS_TRACE_FA[] = "����� �����";
unsigned char STR_NUMEROTATION_FA[] = "���� ��� �������";
unsigned char STR_FIN_CONNEXION_FA[] = "��� ��� �������";
unsigned char STR_PRINT_FA[] = "����� ���������";
unsigned char STR_ON_GOING_FA[] = "�����";
unsigned char STR_TICKET_CLIENT_FA[] = "����� ������";
unsigned char STR_TICKET_MANAGER_FA[] = "����� ������";
unsigned char STR_DUPLICATA2_FA[] = "�����";
unsigned char STR_NON_DISPONIBLE_FA[] = "��� �����";
unsigned char STR_PAS_D_OPERATION_FA[] = "�� ���� ������ ";
unsigned char STR_DONE_FA[] = "�� �������";
unsigned char STR_DEJA_PRESENT_FA[] = "�� �������� �� ���";
unsigned char STR_REPORT_COMMER_FA[] = "����� ������";
unsigned char STR_REPORT_CASHIER_FA[] = "�����������";
unsigned char STR_REPORT_DAILY_FA[] = "������� ������ �";
unsigned char STR_EFFACEMENT_DES_FA[] = "���";
unsigned char STR_RAPP_VAL_ANN_FA[] = "������� : ������/�����";
unsigned char STR_PASSWORD_MANAGER_FA[] = "PASSW. MANAGER ?";
unsigned char STR_ENTRER_CODE_FA[] = "���� ���� ���� :";
unsigned char STR_REPORTS_FA[] = "������ ";
unsigned char STR_APPLICATION_FA[] = "�������";
unsigned char STR_INITIALISEE_FA[] = "����� �����";
unsigned char STR_PARAMETRES_APPLI_FA[] = "������� ���������";
unsigned char STR_INCOMPLETS_FA[] = "��� �����";
unsigned char STR_TICKET_ABANDON_FA[] = "�� �������";
unsigned char STR_ATTENTE_REPONSE_FA[] = "�����";
unsigned char STR_SERVEUR_FA[] = "����� (����)";
unsigned char STR_MEMOIRE_PLEINE_FA[] = "������� �����";
unsigned char STR_ACTIVER_TELECH_FA[] = "����� �������";
unsigned char STR_ANNUL_TELECH_FA[] = "����� ����� �������";
unsigned char STR_TELECHARGEMENT_FA[] = "����� ���������";
unsigned char STR_ACTIVE_FA[] = "����� �������";
unsigned char STR_DESACTIVE_FA[] = "���� �����";
unsigned char STR_MODIFICATION_FA[] = "�����";
unsigned char STR_ENTRER_LE_CODE_FA[] = "���� ���� ����:";
unsigned char STR_CONNEXION_OK_FA[] = "���� ������� �����";
unsigned char STR_NUMEROTATION_OK_FA[] = "���� ��� ������� �����";
unsigned char STR_CONNEXION_FA[] = "���� ������� �����";
unsigned char STR_SERVEUR_KO_FA[] = "������� (������) ����";
unsigned char STR_NUMERO_OCCUPE_FA[] = "���� ��������";
unsigned char STR_NUMERO_INTERDIT_FA[] = "��� ����� �����";
unsigned char STR_PAS_DE_PORTEUSE_FA[] = "�� ���� ����";
unsigned char STR_LINE_ABSENTE_FA[] = "�� ���� ���� ��� �������";
unsigned char STR_ABANDON_FA[] = "�� �������";
unsigned char STR_PROBLEME_MODEM_FA[] = "��� ����";
unsigned char STR_PRODUCT_FA[] = "���� :";
unsigned char STR_GESTION_RECH_FA[] = "������ ������ ����� ������� ";
unsigned char STR_INGEVOUCHER_FA[] = "�����������";
unsigned char STR_RECHARGEMENT_FA[] = "����� ����� ���� �������";
unsigned char STR_LANCER_RECHARGE_FA[] = "��� ����� ����� ���� �������";
unsigned char STR_PASSWORDS_FA[] = "���� �� ��������";
unsigned char STR_TELEPARAMETRAGE_FA[] = "����� ���������";
unsigned char STR_TELEPARAMETRAGE_PRINT_FA[] = "�������";
unsigned char STR_A_FAIRE_FA[] = "���� ������";
unsigned char STR_DEJA_EFECTUE_FA[] = "�� ������ �� ���";
unsigned char STR_TABLE_OPERATEURS_FA[] = "����� ������";
unsigned char STR_TABLE_PRODUCTS_FA[] = "������ ������";
unsigned char STR_TABLE_TICKET_FA[] = "����� ������";
unsigned char STR_TABLE_MAINTENANCE_FA[] = "����� ������";
unsigned char STR_TABLE_TERMINAL_FA[] = "������ ������� ������";
unsigned char STR_TABLE_HORODATAGE_FA[] = "��� ������ ������";
unsigned char STR_ECHEC_FA[] = "���";
unsigned char STR_SUCCES_FA[] = "����";
unsigned char STR_IDENTIFICATION_FA[] = "�����";
unsigned char STR_VENTE_FA[] = "���";
unsigned char STR_SALESPOLLING_FA[] = "������� ��������";
unsigned char STR_INFO_TELECOL_FA[] = "������� ���������";
unsigned char STR_DEMANDE_VERSION_FA[] = "��� �������";
unsigned char STR_REEL_FA[] = "REAL V22 BIS";
unsigned char STR_REEL_V22_FA[] = "REAL V22";
unsigned char STR_SIMUL_FA[] = "SIMUL";
unsigned char STR_PINS_EXPIRED_FA[] = "���� ������ ��������";
unsigned char STR_PINS_EXPIRED2_FA[] = "���� ������ ��������";
unsigned char STR_PINS_USED_FA[] = "���������� ���������";
unsigned char STR_PINS_USED2_FA[] = "���������� ���������";
unsigned char STR_ANOMALIE_VENTE_FA[] = "��� �� �����";
unsigned char STR_ANOMALIE_VENTE2_FA[] = "��� �� �����";
unsigned char STR_INTERDIT_FA[] = "�����";
unsigned char STR_COMMANDE_FA[] = "���";
unsigned char STR_PARAM_ENREGISTRE_FA[] = "����� ���������";
unsigned char STR_VAL_POUR_ENTRER_FA[] = "���� ��� �����";
unsigned char STR_NOM_CATEGORY_FA[] = "Network:";
unsigned char STR_NOM_PRODUCT_FA[] = "���� :";
unsigned char STR_ID_TERMINAL_Min_FA[] = "���� ������ �������:";
unsigned char STR_ID_COMM_FA[] = "���� ������";
unsigned char STR_NUM_TRANS_FA[] = "��� ��������";
unsigned char STR_VALEUR_FA[] = "������ : /0";
unsigned char STR_No_SERIE_FA[] = "������ :";
unsigned char STR_FIN_VALIDITE_FA[] = "����� �������� :";
unsigned char STR_PASSWORD_ERREUR_FA[] = "��� :";
unsigned char STR_Op_Min_FA[] = "���� :";
unsigned char STR_RECAPITULATIF_FA[] = "����";
unsigned char STR_NB_CODES_CHARGES_FA[] = "���� ���� ������� ���� �� �������";
unsigned char STR_NB_CODES_A_CHARGER_FA[] = "��� ���� ������� ���� ������� :";
unsigned char STR_Amount_Tot_FA[] = "������ ������";
unsigned char STR_TOTAL_FA[] = "�������";
unsigned char STR_CASHIER_Min_FA[] = "������ :";
unsigned char STR_Nb_Codes_Telecol_FA[] = "���� ���� ������� ���� �� ������� :";
unsigned char STR_Envoi_FA[] = "�����";
unsigned char STR_SALESPOLLING2_FA[] = "������� ��������";
unsigned char STR_NB_CODES_FA[] = "���� ���� ������� ���� �� �������";
unsigned char STR_TYPE_IMPRIMANTE_Min_FA[] = "��� ������� :";
unsigned char STR_Serv_MDP_FA[] = "����� ������� (������) ������ ���������";
unsigned char STR_HOUR_FA[] = "����";
unsigned char STR_ETAT_DU_STOCK_FA[] = "���� �������";
unsigned char STR_DISP_FA[] = "�����";
unsigned char STR_VEND_FA[] = "�� ���� ";
unsigned char STR_IDENT_FA[] = "����� ������";
unsigned char STR_Version_Ticket_FA[] = "����� ������� :";
unsigned char STR_Version_Category_FA[] = "����� ������� :";
unsigned char STR_Version_Product_FA[] = "������ ������� :";
unsigned char STR_Version_Maintenance_FA[] = "����� ������� :";
unsigned char STR_Version_Terminal_FA[] = "���� ������� :";
unsigned char STR_Version_Horodatage_FA[] = "��� ������ ������� :";
unsigned char STR_RECHARGEMENT_CODES_FA[] = "����� ����� ���� �������";
unsigned char STR_REPORT_FA[] = "����� ������";
unsigned char STR_MANAGER_FA[] = "����";
unsigned char STR_CASHIER_FA[] = "������ :";
unsigned char STR_CASHIER_NB_AMOUNT_FA[] = "������     ������       ������";
unsigned char STR_PRODUCT_NB_AMOUNT_FA[] = "������        ������      ������";
unsigned char STR_PXUNITAIRE_FA[] = "��� ������";
unsigned char STR_NB_FA[] = "������";
unsigned char STR_AMOUNT_FA[] = "������";
unsigned char STR_DUPLICATA_FA[] = "�����";
unsigned char STR_TELECHARGEMENT_APPLI_FA[] = "����� ���������";
unsigned char STR_RECHARGEMENT_CODES_2_FA[] = "����� ����� ���� �������";
unsigned char STR_SALESPOLLING_2_FA[] = "������� ��������";
unsigned char STR_PAS_DE_TELECH_FA[] = "��� ����� ���������";
unsigned char STR_PAS_DE_RECHARGE_FA[] = "��� ����� ����� ���� �������";
unsigned char STR_PAS_DE_TELECOL_FA[] = "��� ������� ��������";
unsigned char STR_PAS_DE_TELEPARA_FA[] = "��� ����� ���������";
unsigned char STR_PLUS_DE_CODE_FA[] = "��� ���� ������ �� ���� ������";
unsigned char STR_DISPONIBLE_FA[] = "�����";
unsigned char STR_ON_LINE_FA[] = "�����";
unsigned char STR_TITRE_TICKET_PARAMETRE_FA[] = " ������";
unsigned char STR_SOUS_TITRE_MANAGER_FA[] = "����";
unsigned char STR_SOUS_TITRE_PRODUCT_FA[] = "����";
unsigned char STR_SOUS_TITRE_TERMINAL_FA[] = "���� ����";
unsigned char STR_SOUS_TITRE_CONNEXION_FA[] = "����";
unsigned char STR_SOUS_TITRE_TELECHARGEMENT_FA[] = "����?��������";
unsigned char STR_SOUS_TITRE_CASHIERS_FA[] = "������";
unsigned char STR_SERVICE_FA[] = "����";
unsigned char STR_DEMARRE_FA[] = "����";
unsigned char STR_ARRETE_FA[] = "����";
unsigned char STR_DEMARRER_SERVICE_FA[] = "���� ������";
unsigned char STR_ARRETER_SERVICE_FA[] = "���� ������";
unsigned char STR_VISUALISE_STOCK_FA[] = "���� �������";
unsigned char STR_PRINT_STOCK_FA[] = "���� �������";
unsigned char STR_CHARGER_CLE_FA[] = "����� �������";
unsigned char STR_CHARGEMENT_CLE_FA[] = "���� ����� �������";
unsigned char STR_CREDIT_EPUISE_FA[] = "�� ���� ����";
unsigned char STR_PASSWORD_UPPER_FA[] = "PASSWORD :";
unsigned char STR_REPORT_DAILY_PRINT_FA[] = "������� ������ ";
unsigned char STR_STOCK_SERVEUR_FA[] = "����� ������� (������)";
unsigned char STR_EPUISE_FA[] = "����";
unsigned char STR_NOUVEAU_CODE_FA[] = "���� �� ����� :";
unsigned char STR_CONFIRMER_CODE_FA[] = "��� ���� ���� :";
unsigned char STR_AUTRE_VENTE_FA[] = "��� ���";
unsigned char STR_PRINT_CODE_BARRE_FA[] = "���� ��������";
unsigned char STR_PRINT_LOGO_FA[] = "���� ������";
unsigned char STR_OUI_FA[] = "���";
unsigned char STR_NON_FA[] = "��� �����";
unsigned char STR_VALIDE_COMMANDE_FA[] = "���� ����� �";
unsigned char STR_CHARGEMENT_LOGO_FA[] = "����� ��������";
unsigned char STR_MASTER_APPLI_FA[] = "��������� ��������";
unsigned char STR_TYPE_CNX_TELECH_FA[] = "����� ��� ������";
unsigned char STR_RTC_FA[] = "RTC";
unsigned char STR_X25_FA[] = "X25";
unsigned char STR_PRINT_COMANDE_FA[] = "����� ��� ��� �������";
unsigned char STR_INFOS_CODES_FA[] = "������� ������";
unsigned char STR_ENVOI_NUM_SERIE_FA[] = "���� ����� ��������";
unsigned char STR_ENVOI_RECLAMATION_FA[] = "���� ��������";
unsigned char STR_FULL_AUTO_FA[] = "���������� ���� ����";
unsigned char STR_IMP_TELECOL_FA[] = "����� ������� ��������";
unsigned char STR_RECHARGEMENT_CODES2_FA[] = "��� ������� ��� ����";
unsigned char STR_RECHARGEMENT_CODES3_FA[] = "����� ���� �������� �������";
unsigned char STR_IMP_NOM_APPLI_FA[] = "����  ��� ��������";
unsigned char STR_IMP_REPORT_AUTO_FA[] = "����� �������� ������������";
unsigned char STR_PRINT_ZERO_FA[] = "���� ���";
unsigned char STR_IMP_DETAIL_RECH_FA[] = "����� ������ ������� ��� ����� ����� ���� �������";
unsigned char STR_IMP_RAPP_DAILY_FA[] = "���� ������� ������";
unsigned char STR_MODE_DEMO_FA[] = "����� �������";
unsigned char STR_RECLAMATION_FA[] = "������";
unsigned char STR_RECPT_MESSAGE_FA[] = "������� ������";
unsigned char STR_CONFIRM_MESSAGE_FA[] = "����� ������";
unsigned char STR_INIT_MANAGER_FA[] = "������� ������";
unsigned char STR_ANNULER_FA[] = "����";
unsigned char STR_DERNIERE_VENTE_FA[] = "����� ������";
unsigned char STR_ENVOI_ANNULATION_FA[] = "���� �������";
unsigned char STR_PROPOSITION_FA[] = "���� ������";
unsigned char STR_ANNUL_VENTE_FA[] = "���� �����";
unsigned char STR_REFUS_FA[] = "���";
unsigned char STR_Vendu_le_FA[] = "�� ����� :";
unsigned char STR_CONFIRMATION_FA[] = "�����";
unsigned char STR_DIFFERENT_FA[] = "�����";
unsigned char STR_STOCK_FA[] = "����� ������� (������)";
unsigned char STR_STOCK_GENERAL_FA[] = "����� ���";
unsigned char STR_STOCK_DESIZE_FA[] = "����� ������";
unsigned char STR_ACTUALISE_STOCK_FA[] = "��� �������";
unsigned char STR_IMP_RAPP_VENTE_FA[] = "���� ����� �����";
unsigned char STR_DIAL_GPRS_FA[] = "���� ������� ��� ���� �� ��� �� ��";
unsigned char STR_CONNEXION_KO_FA[] = "��� �������";
unsigned char STR_INVALID_ENTRY_FA[] = "����� ��� ���� !";
unsigned char STR_LIMIT_PORT_NUMBER_FA[] = "(65535-0) :";
unsigned char STR_APN_FA[] = "��� ���� ������";
unsigned char STR_IP_ADDRESS_FA[] = "����� �������� �������� :";
unsigned char STR_IP_PORT_FA[] = "���� �������� �������� :";
unsigned char STR_LOCAL_IP_PORT_FA[] = "Local IP Port:";
unsigned char STR_USERNAME_FA[] = "��� �������� :";
unsigned char STR_PASSWORD_FA[] = "���� ����";
unsigned char STR_AUTHENTI_FA[] = "�����";
unsigned char STR_PRINT_GPRS_CONFIG_FA[] = "���� ������� ���� �� ��� �� ��";
unsigned char STR_RESET_GPRS_CONFIG_FA[] = "����� ����� ���������";
unsigned char STR_PASSWORD_PIN_FA[] = "��� ����� ����� SIM";
unsigned char STR_PARAMETRES_GPRS_FA[] = "������� ���� �� ��� �� ��";
unsigned char STR_GPRS_EFFACES_FA[] = "��� ����� �������";
unsigned char STR_NOT_SPECIFIED_FA[] = "��� ����";
unsigned char STR_CONNECTION_FA[] = "����";
unsigned char STR_SERIE_FA[] = "���� ������� ...";
unsigned char STR_STANDARD_FA[] = "�����";
unsigned char STR_ITEM_CODE_FA[] = "���";
unsigned char STR_ITEM_MENU_FA[] = "�����";
unsigned char STR_ENREGISTRER_FA[] = "����";
unsigned char STR_VOTRE_TPE_FA[] = "";
unsigned char STR_VERIFICATION_FA[] = "������";
unsigned char STR_NOUVEAU_SMS_FA[] = "����?����";
unsigned char STR_INITIALISATION_FA[] = "INIT.";
unsigned char STR_LOGICIEL_FA[] = "��������";
unsigned char STR_A_ECHOUER_FA[] = "�� ��?";
unsigned char STR_VEUILLEZ_REDEMMARER_FA[] = "������ ����?������";
unsigned char STR_SERVEUR_CENTRE_FA[] = "������";
unsigned char STR_CONFIG_AUTO_FA[] = "������?������?";
unsigned char STR_MENU_PLANNING_FA[] = "���������";
unsigned char STR_MENU_PRINT_FA[] = "����?������?";
unsigned char STR_MENU_CONNECTION_FA[] = "����� ��������";
unsigned char STR_PLUS_DE_PAPIER_FA[] = "������ ��?s1\In";
unsigned char STR_PAPIER_REMPLACE_FA[] = "����?����";
unsigned char STR_FIN_PRINT_FA[] = "����?������?In";
unsigned char STR_TICKET_FA[] = "����� ������";
unsigned char STR_DHCP_FA[] = "DHCP";
unsigned char STR_IP_POS_FA[] = "IP TERMINAL";
unsigned char STR_MASQUE_SS_RESEAU_FA[] = "NETWORK MASK";
unsigned char STR_PASSERELLE_FA[] = "GATEWAY";
unsigned char STR_DNS_1_FA[] = "DNS 1";
unsigned char STR_DNS_2_FA[] = "DNS 2";
unsigned char STR_ETHERNET_SETTINGS_FA[] = "ETHERNET PARAM";
unsigned char STR_MENU_ETHERNET_FA[] = "MENU ETHERNET";
unsigned char STR_ETHERNET_FA[] = "ETHERNET PARAM";
unsigned char STR_VEUILLEZ_FA[] = "������ ����?������";
unsigned char STR_PATIENTER_FA[] = "����������?";
unsigned char STR_MULTISALE_FA[] = "�������� ������";
unsigned char STR_ARRETER_VENTE_FA[] = "����?������??";
unsigned char STR_REPRENDRE_VENTE_FA[] = "�������� ������??";
unsigned char STR_DOUBLEPAPER_FA[] = "����?����";
unsigned char STR_REPORT_DISPO_FA[] = "��������?������?";
unsigned char STR_REPORT_BY_SALER_FA[] = "��?����?";
unsigned char STR_REPORT_BY_NETWORK_FA[] = "��?������";
unsigned char STR_GLOBAL_CAISSIER_REPORT_FA[] = "������?����?";
unsigned char STR_REPORT_NETWORK_FA[] = "������?������?";
unsigned char STR_TEMPORAIRE_FA[] = "��������?";
unsigned char STR_REPORT_VENTES_FA[] = "�������� ��������";
unsigned char STR_CHOIX_RACCOURCI_FA[] = "����������";
unsigned char STR_PRINT_RACCOURCI_FA[] = "����?��������?";
unsigned char STR_TITRE_LISTE_RACCOURCI_FA[] = "SHORTCUT";
unsigned char STR_USE_MODE_DEGRADE_FA[] = "������?��������??";
unsigned char STR_GESTION_EQUIPE_FA[] = "���������� ������?";
unsigned char STR_EQUIPE_EN_COURS_FA[] = "�������� ������?";
unsigned char STR_DEMARRER_SERVICE_EQUIPE_FA[] = "������ ������?";
unsigned char STR_ARRETER_SERVICE_EQUIPE_FA[] = "����?������?";
unsigned char STR_PLEASE_PRINT_FA[] = "�������� ������?";
#ifndef DUBAI
unsigned char STR_SHIFT_REPORT_FA[] = "SHIFT REPORT";
unsigned char STR_MENU_REPORT_SHIFT_FA[] = "SHIFT REPORT";
unsigned char STR_MENU_REPORT__CURRENT_SHIFT_FA[] = "CURRENT SHIFT";
#else
unsigned char STR_SHIFT_REPORT_FA[] = "SHIFT REPORT";
unsigned char STR_MENU_REPORT_SHIFT_FA[] = "SHIFT REPORT";
unsigned char STR_MENU_REPORT__CURRENT_SHIFT_FA[] = "CURRENT SHIFT";
#endif
unsigned char STR_SHIFT_CASHIER_REPORT_FA[] = "����. ����. ������?";
unsigned char STR_EQUIPE_FA[] = "�������� ������?";
unsigned char STR_DU_FA[] = "�����";
unsigned char STR_AU_FA[] = "��� ���";
unsigned char STR_INACTIF_FA[] = "����������������";
unsigned char STR_NO_CURRENT_SHIFT_FA[] = "������ ����. ������";
unsigned char STR_NAME_FA[] = "Name :";
unsigned char STR_SOLDE_FA[] = "Balance:";
unsigned char STR_SITUATION_CLIENT_FA[] = "MERCHANT SOLDE";
unsigned char STR_RESTOCK_CAPACITY_FA[] = "PINS RESTOCK CAP.:";
unsigned char STR_CREDIT_CLIENT_FA[] = " Merchant Credit";
unsigned char STR_CREDIT_AUTORISE_FA[] = "Credit allowed:";
unsigned char STR_QUANTITE_OK_FA[] = "������� ����ɿ";
unsigned char STR_QUANTITE_FA[] = "������� ����ɿ";
unsigned char STR_INVOICED_AMOUNT_FA[] = "���� ����� ";
unsigned char STR_MERCHANT_ID_FA[] = "��� ������";
unsigned char STR_SALESMAN_ID_FA[] = "��� �������";
unsigned char STR_CHECK_ORDER_FA[] = "���� �� �������";
unsigned char STR_CHECKING_ORDER_FA[] = "������ ����";
unsigned char STR_VALIDATE_ORDER_FA[] = "���� �������";
unsigned char STR_INCOMPLETE_ORDER_FA[] = "����� ����� ";
unsigned char STR_PB_ORDER_EMPTY_FA[] = "����� ����� ";
unsigned char STR_PB_ORDER_AMOUNT_FA[] = "������";
unsigned char STR_PB_ORDER_MERCHANT_ID_FA[] = "��� ������";
unsigned char STR_PB_ORDER_TERMINAL_ID_FA[] = "��� ������";
unsigned char STR_PB_ORDER_SALESMAN_ID_FA[] = "��� �������";
unsigned char STR_PB_ORDER_PASSWORD_FA[] = "���� ������";
unsigned char STR_EXIT_ORDER_FA[] = "  ���̿";
unsigned char STR_SEND_ORDER_FA[] = "����� �������";
unsigned char STR_ORDER_VALIDATED_FA[] = "������ �������";
unsigned char STR_ORDER_REFUSED_FA[] = "���� ������� ";
unsigned char STR_ORDER_FAILED_FA[] = "��� ������� ";
unsigned char STR_PENDING_ORDER_FA[] = "����� ����� ";
unsigned char STR_NO_ORDER_FA[] = "�� ���� ����� ";
unsigned char STR_ORDER_NUMBER_FA[] = "��� ������� ";
unsigned char STR_ORDER_FA[] = "������ �������";
unsigned char STR_TICKET_MERCHANT_ID_FA[] = "��� ������ ";
unsigned char STR_TICKET_SALESMAN_ID_FA[] = "��� ������� ";
unsigned char STR_TICKET_TERMINAL_ID_FA[] = "��� ������ ";
unsigned char STR_SALESMAN_COPY_FA[] = "���� ������� ";
unsigned char STR_MERCHANT_COPY_FA[] = "���� ������";
unsigned char STR_SUMMARY_FA[] = "������";
unsigned char STR_PINS_QUANTITY_FA[] = "���� ������";
unsigned char STR_FACE_VALUE_FA[] = "���� ����� ";
unsigned char STR_PAID_FA[] = "�����";
unsigned char STR_MERCHANT_SIGNATURE_FA[] = "����� ������";
unsigned char STR_SIGNATURE_FA[] = "�������";
unsigned char STR_BAD_SALESMAN_FA[] = "������� ��� ";
unsigned char STR_AUTHENTIFICATION_MAJ_FA[] = "�������� ";
unsigned char STR_PRODUCT_UNAVAILABLE_FA[] = "���� ��� �����";
unsigned char STR_FOR_THIS_MERCHANT_FA[] = "���� ������";
unsigned char STR_SERVER_STOCK_EMPTY_FA[] = "������� ����";
unsigned char STR_INSUFFICIENT_CREDIT_FA[] = "���� ��� ����";
unsigned char STR_BAD_MERCHANT_ID_FA[] = "��� ���� ���";
unsigned char STR_OR_TERMINAL_ID_FA[] = "�� ��� ������";
unsigned char STR_ORDER_MODE_FA[] = "����� �����";
unsigned char STR_STOCK2_FA[] = "����?.";
unsigned char STR_PERC_DISCOUNT_FA[] = "���� �����";
unsigned char STR_SCAN_BARCODE_FA[] = "����� �����";
unsigned char STR_TRY_AGAIN_FA[] = "��� ��������";
unsigned char STR_PLEASE_INPUT_FA[] = "���� �������";
unsigned char STR_VALID_POURC_FA[] = "���� �����";
unsigned char STR_MERCHANT_SERIAL_FA[] = "��� ����� ������";
unsigned char STR_SERIAL_NUMBER_FA[] = "��� ��������";
unsigned char STR_ORDER_PREVIEW_FA[] = "������ �����";
unsigned char STR_OPERATION_REFUSED_FA[] = "����� ������";
unsigned char STR_OPERATION_FAILED_FA[] = "����� �����";
unsigned char STR_NO_FA[] = "��� �����";
unsigned char STR_DTU_FA[] = "����� �����";
unsigned char STR_TELEPHONE_FA[] = "��� ������";
unsigned char STR_CONFIRMER_TRANSACTION_FA[] = "���� �������";
unsigned char STR_IMP_CONFIRM_DTU_FA[] = "����";
unsigned char STR_PASSWORD_PARTIE_1_FA[] = "��� �����";
unsigned char STR_PASSWORD_PARTIE_2_FA[] = "��� ������";
unsigned char STR_RESET_CODE_FA[] = "��� ����� �����";
unsigned char STR_CHANGE_DISCOUNT_FA[] = "���� �����";
unsigned char STR_SALESMAN_NAME_FA[] = "��� �������";
unsigned char STR_MERCHANT_NAME_FA[] = "��� ������";
unsigned char STR_SOFT_DOWNLOAD_FA[] = "����� ������";
unsigned char STR_REASON_FA[] = " ����� ";
unsigned char STR_NOT_ALLOWED_FOR_FA[] = "��� �����";
unsigned char STR_THIS_MERCHANT_FA[] = "���� ������";
unsigned char STR_GET_SALESMAN_FA[] = "   ��� �����";
unsigned char STR_INFORMATIONS_FA[] = "   ���������";
unsigned char STR_REEL_V32BIS_FA[] = "V32 BIS(14400)";
unsigned char STR_PLEASE_CHECK_FA[] = "���� ������";
unsigned char STR_PENDING_ORDERS_FA[] = "����� �����";
unsigned char STR_PARAM_DOWNLOAD_FA[] = "Parameter";
unsigned char STR_SALESMAN_CREDIT_FA[] = "Salesman Credit:";
unsigned char STR_PHYSICAL_CARDS_STOCK_FA[] = "Physical Cards Stock:";
unsigned char STR_CASH_ORDER_FA[] = "CASH ORDER";
unsigned char STR_CONSIGN_ORDER_FA[] = "CONSIGN ORDER";
unsigned char STR_RETURN_ORDER_FA[] = "RETURN ORDER";
unsigned char STR_ACTIVATION_NUMBER_FA[] = "Activation number:";
unsigned char STR_EXPIRY_DATE_FA[] = "Expiry date:";
unsigned char STR_SN_FA[] = "SN:";
unsigned char STR_CURRENT_DEPOSIT_FA[] = "Current Deposit";
unsigned char STR_SR_FA[] = "SR";
unsigned char STR_CUSTOMER_COPY_FA[] = "(Customer Copy)";
unsigned char STR_MERCHANT_COPY2_FA[] = "(Merchant Copy)";
unsigned char STR_POWERED_BY_FA[] = "Powered by";
unsigned char STR_TRACE_NO_FA[] = "Trace No:";
unsigned char STR_RESPONSE_CODE_FA[] = "Response Code:";
unsigned char STR_ERROR_MSG_FA[] = "Error Message:";
unsigned char STR_PINS_RESTOCKED_FA[] = "Pins Restocked:";

unsigned char *gl_pStr_Arabic[] = 
{
	STR_DEFAUT_NUMERO_PAD_PRINC_FA,
	STR_DEFAUT_NUMERO_PAD_SEC_FA,
	STR_DEFAUT_ADRESSE_X25_FA,
	STR_AUTHENTIFICATION_FA,
	STR_EUR_FA,
	STR_BLANC_FA,
	STR_TAB_FA,
	STR_AUTHENTI_PAP_FA,
	STR_AUTHENTI_SHAP_FA,
	STR_VIDE_FA,
	STR_ETOILES_FA,
	STR_GPRS_FA,
	STR_PPP_FA,
	STR_ETOILES2_FA,
	STR_TIRETS_FA,
	STR_CHOIX_OPERATEUR_FA,
	STR_PASSWORD_DISPLAY_FA,
	STR_ERROR_FA,
	STR_COUPER_LE_TICKET_FA,
	STR_PRESS_ENTER_FA,
	STR_VAL_CANCEL_FA,
	STR_VAL_CANCEL_COR_FA,
	STR_IDENT_MANAGER_FA,
	STR_PASSWORD_SECRET_FA,
	STR_PASSWORDS_CASHIERS_FA,
	STR_PASSWORD_CASHIER_FA,
	STR_TIME_SOFTWAREDOWNLOAD_FA,
	STR_TIME_SALESPOLLING_FA,
	STR_TIME_SETTINGSDOWNLOAD_FA,
	STR_PRINT_PARAM_FA,
	STR_NUMERO_PAD_FA,
	STR_NUMERO_PAD_SECOURS_FA,
	STR_MENU_PPP_FA,
	STR_NUM_TLCHGT_FA,
	STR_ID_TERMINAL_FA,
	STR_TYPE_IMPRIMANTE_FA,
	STR_REINIT_APPLI_FA,
	STR_INFOS_TRACE_FA,
	STR_NUMEROTATION_FA,
	STR_FIN_CONNEXION_FA,
	STR_PRINT_FA,
	STR_ON_GOING_FA,
	STR_TICKET_CLIENT_FA,
	STR_TICKET_MANAGER_FA,
	STR_DUPLICATA2_FA,
	STR_NON_DISPONIBLE_FA,
	STR_PAS_D_OPERATION_FA,
	STR_DONE_FA,
	STR_DEJA_PRESENT_FA,
	STR_REPORT_COMMER_FA,
	STR_REPORT_CASHIER_FA,
	STR_REPORT_DAILY_FA,
	STR_EFFACEMENT_DES_FA,
	STR_RAPP_VAL_ANN_FA,
	STR_PASSWORD_MANAGER_FA,
	STR_ENTRER_CODE_FA,
	STR_REPORTS_FA,
	STR_APPLICATION_FA,
	STR_INITIALISEE_FA,
	STR_PARAMETRES_APPLI_FA,
	STR_INCOMPLETS_FA,
	STR_TICKET_ABANDON_FA,
	STR_ATTENTE_REPONSE_FA,
	STR_SERVEUR_FA,
	STR_MEMOIRE_PLEINE_FA,
	STR_ACTIVER_TELECH_FA,
	STR_ANNUL_TELECH_FA,
	STR_TELECHARGEMENT_FA,
	STR_ACTIVE_FA,
	STR_DESACTIVE_FA,
	STR_MODIFICATION_FA,
	STR_ENTRER_LE_CODE_FA,
	STR_CONNEXION_OK_FA,
	STR_NUMEROTATION_OK_FA,
	STR_CONNEXION_FA,
	STR_SERVEUR_KO_FA,
	STR_NUMERO_OCCUPE_FA,
	STR_NUMERO_INTERDIT_FA,
	STR_PAS_DE_PORTEUSE_FA,
	STR_LINE_ABSENTE_FA,
	STR_ABANDON_FA,
	STR_PROBLEME_MODEM_FA,
	STR_PRODUCT_FA,
	STR_GESTION_RECH_FA,
	STR_INGEVOUCHER_FA,
	STR_RECHARGEMENT_FA,
	STR_LANCER_RECHARGE_FA,
	STR_PASSWORDS_FA,
	STR_TELEPARAMETRAGE_FA,
	STR_TELEPARAMETRAGE_PRINT_FA,
	STR_A_FAIRE_FA,
	STR_DEJA_EFECTUE_FA,
	STR_TABLE_OPERATEURS_FA,
	STR_TABLE_PRODUCTS_FA,
	STR_TABLE_TICKET_FA,
	STR_TABLE_MAINTENANCE_FA,
	STR_TABLE_TERMINAL_FA,
	STR_TABLE_HORODATAGE_FA,
	STR_ECHEC_FA,
	STR_SUCCES_FA,
	STR_IDENTIFICATION_FA,
	STR_VENTE_FA,
	STR_SALESPOLLING_FA,
	STR_INFO_TELECOL_FA,
	STR_DEMANDE_VERSION_FA,
	STR_REEL_FA,
	STR_REEL_V22_FA,
	STR_SIMUL_FA,
	STR_PINS_EXPIRED_FA,
	STR_PINS_EXPIRED2_FA,
	STR_PINS_USED_FA,
	STR_PINS_USED2_FA,
	STR_ANOMALIE_VENTE_FA,
	STR_ANOMALIE_VENTE2_FA,
	STR_INTERDIT_FA,
	STR_COMMANDE_FA,
	STR_PARAM_ENREGISTRE_FA,
	STR_VAL_POUR_ENTRER_FA,
	STR_NOM_CATEGORY_FA,
	STR_NOM_PRODUCT_FA,
	STR_ID_TERMINAL_Min_FA,
	STR_ID_COMM_FA,
	STR_NUM_TRANS_FA,
	STR_VALEUR_FA,
	STR_No_SERIE_FA,
	STR_FIN_VALIDITE_FA,
	STR_PASSWORD_ERREUR_FA,
	STR_Op_Min_FA,
	STR_RECAPITULATIF_FA,
	STR_NB_CODES_CHARGES_FA,
	STR_NB_CODES_A_CHARGER_FA,
	STR_Amount_Tot_FA,
	STR_TOTAL_FA,
	STR_CASHIER_Min_FA,
	STR_Nb_Codes_Telecol_FA,
	STR_Envoi_FA,
	STR_SALESPOLLING2_FA,
	STR_NB_CODES_FA,
	STR_TYPE_IMPRIMANTE_Min_FA,
	STR_Serv_MDP_FA,
	STR_HOUR_FA,
	STR_ETAT_DU_STOCK_FA,
	STR_DISP_FA,
	STR_VEND_FA,
	STR_IDENT_FA,
	STR_Version_Ticket_FA,
	STR_Version_Category_FA,
	STR_Version_Product_FA,
	STR_Version_Maintenance_FA,
	STR_Version_Terminal_FA,
	STR_Version_Horodatage_FA,
	STR_RECHARGEMENT_CODES_FA,
	STR_REPORT_FA,
	STR_MANAGER_FA,
	STR_CASHIER_FA,
	STR_CASHIER_NB_AMOUNT_FA,
	STR_PRODUCT_NB_AMOUNT_FA,
	STR_PXUNITAIRE_FA,
	STR_NB_FA,
	STR_AMOUNT_FA,
	STR_DUPLICATA_FA,
	STR_TELECHARGEMENT_APPLI_FA,
	STR_RECHARGEMENT_CODES_2_FA,
	STR_SALESPOLLING_2_FA,
	STR_PAS_DE_TELECH_FA,
	STR_PAS_DE_RECHARGE_FA,
	STR_PAS_DE_TELECOL_FA,
	STR_PAS_DE_TELEPARA_FA,
	STR_PLUS_DE_CODE_FA,
	STR_DISPONIBLE_FA,
	STR_ON_LINE_FA,
	STR_TITRE_TICKET_PARAMETRE_FA,
	STR_SOUS_TITRE_MANAGER_FA,
	STR_SOUS_TITRE_PRODUCT_FA,
	STR_SOUS_TITRE_TERMINAL_FA,
	STR_SOUS_TITRE_CONNEXION_FA,
	STR_SOUS_TITRE_TELECHARGEMENT_FA,
	STR_SOUS_TITRE_CASHIERS_FA,
	STR_SERVICE_FA,
	STR_DEMARRE_FA,
	STR_ARRETE_FA,
	STR_DEMARRER_SERVICE_FA,
	STR_ARRETER_SERVICE_FA,
	STR_VISUALISE_STOCK_FA,
	STR_PRINT_STOCK_FA,
	STR_CHARGER_CLE_FA,
	STR_CHARGEMENT_CLE_FA,
	STR_CREDIT_EPUISE_FA,
	STR_PASSWORD_UPPER_FA,
	STR_REPORT_DAILY_PRINT_FA,
	STR_STOCK_SERVEUR_FA,
	STR_EPUISE_FA,
	STR_NOUVEAU_CODE_FA,
	STR_CONFIRMER_CODE_FA,
	STR_AUTRE_VENTE_FA,
	STR_PRINT_CODE_BARRE_FA,
	STR_PRINT_LOGO_FA,
	STR_OUI_FA,
	STR_NON_FA,
	STR_VALIDE_COMMANDE_FA,
	STR_CHARGEMENT_LOGO_FA,
	STR_MASTER_APPLI_FA,
	STR_TYPE_CNX_TELECH_FA,
	STR_RTC_FA,
	STR_X25_FA,
	STR_PRINT_COMANDE_FA,
	STR_INFOS_CODES_FA,
	STR_ENVOI_NUM_SERIE_FA,
	STR_ENVOI_RECLAMATION_FA,
	STR_FULL_AUTO_FA,
	STR_IMP_TELECOL_FA,
	STR_RECHARGEMENT_CODES2_FA,
	STR_RECHARGEMENT_CODES3_FA,
	STR_IMP_NOM_APPLI_FA,
	STR_IMP_REPORT_AUTO_FA,
	STR_PRINT_ZERO_FA,
	STR_IMP_DETAIL_RECH_FA,
	STR_IMP_RAPP_DAILY_FA,
	STR_MODE_DEMO_FA,
	STR_RECLAMATION_FA,
	STR_RECPT_MESSAGE_FA,
	STR_CONFIRM_MESSAGE_FA,
	STR_INIT_MANAGER_FA,
	STR_ANNULER_FA,
	STR_DERNIERE_VENTE_FA,
	STR_ENVOI_ANNULATION_FA,
	STR_PROPOSITION_FA,
	STR_ANNUL_VENTE_FA,
	STR_REFUS_FA,
	STR_Vendu_le_FA,
	STR_CONFIRMATION_FA,
	STR_DIFFERENT_FA,
	STR_STOCK_FA,
	STR_STOCK_GENERAL_FA,
	STR_STOCK_DESIZE_FA,
	STR_ACTUALISE_STOCK_FA,
	STR_IMP_RAPP_VENTE_FA,
	STR_DIAL_GPRS_FA,
	STR_CONNEXION_KO_FA,
	STR_INVALID_ENTRY_FA,
	STR_LIMIT_PORT_NUMBER_FA,
	STR_APN_FA,
	STR_IP_ADDRESS_FA,
	STR_IP_PORT_FA,
	STR_LOCAL_IP_PORT_FA,
	STR_USERNAME_FA,
	STR_PASSWORD_FA,
	STR_AUTHENTI_FA,
	STR_PRINT_GPRS_CONFIG_FA,
	STR_RESET_GPRS_CONFIG_FA,
	STR_PASSWORD_PIN_FA,
	STR_PARAMETRES_GPRS_FA,
	STR_GPRS_EFFACES_FA,
	STR_NOT_SPECIFIED_FA,
	STR_CONNECTION_FA,
	STR_SERIE_FA,
	STR_STANDARD_FA,
	STR_ITEM_CODE_FA,
	STR_ITEM_MENU_FA,
	STR_ENREGISTRER_FA,
	STR_VOTRE_TPE_FA,
	STR_VERIFICATION_FA,
	STR_NOUVEAU_SMS_FA,
	STR_INITIALISATION_FA,
	STR_LOGICIEL_FA,
	STR_A_ECHOUER_FA,
	STR_VEUILLEZ_REDEMMARER_FA,
	STR_SERVEUR_CENTRE_FA,
	STR_CONFIG_AUTO_FA,
	STR_MENU_PLANNING_FA,
	STR_MENU_PRINT_FA,
	STR_MENU_CONNECTION_FA,
	STR_PLUS_DE_PAPIER_FA,
	STR_PAPIER_REMPLACE_FA,
	STR_FIN_PRINT_FA,
	STR_TICKET_FA,
	STR_DHCP_FA,
	STR_IP_POS_FA,
	STR_MASQUE_SS_RESEAU_FA,
	STR_PASSERELLE_FA,
	STR_DNS_1_FA,
	STR_DNS_2_FA,
	STR_ETHERNET_SETTINGS_FA,
	STR_MENU_ETHERNET_FA,
	STR_ETHERNET_FA,
	STR_VEUILLEZ_FA,
	STR_PATIENTER_FA,
	STR_MULTISALE_FA,
	STR_ARRETER_VENTE_FA,
	STR_REPRENDRE_VENTE_FA,
	STR_DOUBLEPAPER_FA,
	STR_REPORT_DISPO_FA,
	STR_REPORT_BY_SALER_FA,
	STR_REPORT_BY_NETWORK_FA,
	STR_GLOBAL_CAISSIER_REPORT_FA,
	STR_REPORT_NETWORK_FA,
	STR_TEMPORAIRE_FA,
	STR_REPORT_VENTES_FA,
	STR_CHOIX_RACCOURCI_FA,
	STR_PRINT_RACCOURCI_FA,
	STR_TITRE_LISTE_RACCOURCI_FA,
	STR_USE_MODE_DEGRADE_FA,
	STR_GESTION_EQUIPE_FA,
	STR_EQUIPE_EN_COURS_FA,
	STR_DEMARRER_SERVICE_EQUIPE_FA,
	STR_ARRETER_SERVICE_EQUIPE_FA,
	STR_PLEASE_PRINT_FA,
#ifndef DUBAI
	STR_SHIFT_REPORT_FA,
	STR_MENU_REPORT_SHIFT_FA,
	STR_MENU_REPORT__CURRENT_SHIFT_FA,
#else
	STR_SHIFT_REPORT_FA,
	STR_MENU_REPORT_SHIFT_FA,
	STR_MENU_REPORT__CURRENT_SHIFT_FA,
#endif
	STR_SHIFT_CASHIER_REPORT_FA,
	STR_EQUIPE_FA,
	STR_DU_FA,
	STR_AU_FA,
	STR_INACTIF_FA,
	STR_NO_CURRENT_SHIFT_FA,
	STR_NAME_FA,
	STR_SOLDE_FA,
	STR_SITUATION_CLIENT_FA,
	STR_RESTOCK_CAPACITY_FA,
	STR_CREDIT_CLIENT_FA,
	STR_CREDIT_AUTORISE_FA,
	STR_QUANTITE_OK_FA,
	STR_QUANTITE_FA,
	STR_INVOICED_AMOUNT_FA,
	STR_MERCHANT_ID_FA,
	STR_SALESMAN_ID_FA,
	STR_CHECK_ORDER_FA,
	STR_CHECKING_ORDER_FA,
	STR_VALIDATE_ORDER_FA,
	STR_INCOMPLETE_ORDER_FA,
	STR_PB_ORDER_EMPTY_FA,
	STR_PB_ORDER_AMOUNT_FA,
	STR_PB_ORDER_MERCHANT_ID_FA,
	STR_PB_ORDER_TERMINAL_ID_FA,
	STR_PB_ORDER_SALESMAN_ID_FA,
	STR_PB_ORDER_PASSWORD_FA,
	STR_EXIT_ORDER_FA,
	STR_SEND_ORDER_FA,
	STR_ORDER_VALIDATED_FA,
	STR_ORDER_REFUSED_FA,
	STR_ORDER_FAILED_FA,
	STR_PENDING_ORDER_FA,
	STR_NO_ORDER_FA,
	STR_ORDER_NUMBER_FA,
	STR_ORDER_FA,
	STR_TICKET_MERCHANT_ID_FA,
	STR_TICKET_SALESMAN_ID_FA,
	STR_TICKET_TERMINAL_ID_FA,
	STR_SALESMAN_COPY_FA,
	STR_MERCHANT_COPY_FA,
	STR_SUMMARY_FA,
	STR_PINS_QUANTITY_FA,
	STR_FACE_VALUE_FA,
	STR_PAID_FA,
	STR_MERCHANT_SIGNATURE_FA,
	STR_SIGNATURE_FA,
	STR_BAD_SALESMAN_FA,
	STR_AUTHENTIFICATION_MAJ_FA,
	STR_PRODUCT_UNAVAILABLE_FA,
	STR_FOR_THIS_MERCHANT_FA,
	STR_SERVER_STOCK_EMPTY_FA,
	STR_INSUFFICIENT_CREDIT_FA,
	STR_BAD_MERCHANT_ID_FA,
	STR_OR_TERMINAL_ID_FA,
	STR_ORDER_MODE_FA,
	STR_STOCK2_FA,
	STR_PERC_DISCOUNT_FA,
	STR_SCAN_BARCODE_FA,
	STR_TRY_AGAIN_FA,
	STR_PLEASE_INPUT_FA,
	STR_VALID_POURC_FA,
	STR_MERCHANT_SERIAL_FA,
	STR_SERIAL_NUMBER_FA,
	STR_ORDER_PREVIEW_FA,
	STR_OPERATION_REFUSED_FA,
	STR_OPERATION_FAILED_FA,
	STR_NO_FA,
	STR_DTU_FA,
	STR_TELEPHONE_FA,
	STR_CONFIRMER_TRANSACTION_FA,
	STR_IMP_CONFIRM_DTU_FA,
	STR_PASSWORD_PARTIE_1_FA,
	STR_PASSWORD_PARTIE_2_FA,
	STR_RESET_CODE_FA,
	STR_CHANGE_DISCOUNT_FA,
	STR_SALESMAN_NAME_FA,
	STR_MERCHANT_NAME_FA,
	STR_SOFT_DOWNLOAD_FA,
	STR_REASON_FA,
	STR_NOT_ALLOWED_FOR_FA,
	STR_THIS_MERCHANT_FA,
	STR_GET_SALESMAN_FA,
	STR_INFORMATIONS_FA,
	STR_REEL_V32BIS_FA,
	STR_PLEASE_CHECK_FA,
	STR_PENDING_ORDERS_FA,
	STR_PARAM_DOWNLOAD_FA,
	STR_SALESMAN_CREDIT_FA,
	STR_PHYSICAL_CARDS_STOCK_FA,
	STR_CASH_ORDER_FA,
	STR_CONSIGN_ORDER_FA,
	STR_RETURN_ORDER_FA,
	STR_ACTIVATION_NUMBER_FA,
	STR_EXPIRY_DATE_FA,
	STR_SN_FA,
	STR_CURRENT_DEPOSIT_FA,
	STR_SR_FA,
	STR_CUSTOMER_COPY_FA,
	STR_MERCHANT_COPY2_FA,
	STR_POWERED_BY_FA,
	STR_TRACE_NO_FA,
	STR_RESPONSE_CODE_FA,
	STR_ERROR_MSG_FA,
	STR_PINS_RESTOCKED_FA,
};
#endif

#endif
