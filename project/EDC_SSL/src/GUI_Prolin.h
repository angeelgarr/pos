/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2014-? PAX Technology, Inc. All rights reserved.
 * Description:     implemented some dialogs based on XUI.
 * Revision History:
 * Date                  Author                 Action
 * 2014-4-25         Kim.L                    created
 * ============================================================================
 */

#ifndef _GUI_H_
#define _GUI_H_

#pragma pack(4)
#define 	GUI_OK							0
#define		GUI_ERR_UNSUPPORTED				-1
#define		GUI_ERR_NOTINIT					-2
#define		GUI_ERR_INVALIDPARAM			-3
#define		GUI_ERR_USERCANCELLED			-4
#define		GUI_ERR_TIMEOUT					-5
#define		GUI_ERR_EXCEED					-6
#define		GUI_ERR_CANTCALLFROMCALLBACK	-9
#define		GUI_RETURNFROMCALLBACK 			-10
#define		GUI_ERR_SYSTEM		 			-11

//for info page
#ifndef 	GUI_KEYPREV
#define 	GUI_KEYPREV		600
#define 	GUI_KEYNEXT		601
#define    GUI_KEYSWITCH   700
#endif
#define		GUI_OK_PREVIOUS		1
#define		GUI_OK_NEXT			2

//0xFFRRGGBB
#ifndef _GUI_RGB_INT_
#define _GUI_RGB_INT_(_r, _g, _b) 	((unsigned int)(((unsigned int)(0xFF << 24)) | ((unsigned int)(_r << 16)) | ((unsigned int)(_g << 8)) | ((unsigned int)(_b))))
#endif
#define _GUI_GetRValue(_argb)		((((unsigned int)(_argb)) >> 16) & 0x000000FF)
#define _GUI_GetGValue(_argb)		((((unsigned int)(_argb)) >> 8) & 0x000000FF)
#define _GUI_GetBValue(_argb)		(((unsigned int)(_argb))  & 0x000000FF)

typedef enum GUI_ALIGNMENT {
    GUI_ALIGN_LEFT = 0, GUI_ALIGN_CENTER, GUI_ALIGN_RIGHT
}gui_alignment_t;

typedef enum GUI_FONTSTYLE {
    GUI_FONT_STD = 0, GUI_FONT_BOLD = 0x01, GUI_FONT_ITALIC = 0x02, GUI_FONT_REVERSAL = 0x04,
}gui_fontstyle_t;

typedef enum GUI_MSGBOXTYPE {
    GUI_BUTTON_NONE, GUI_BUTTON_OK, GUI_BUTTON_CANCEL, GUI_BUTTON_YandN
}gui_msgboxtype_t;

typedef enum GUI_MENUSTYLE {
    GUI_MENU_MANUAL_INDEX = 0x00000001,
    GUI_MENU_0_START = 0x00000001 << 1, //index from 0, off when GUI_MENU_ARROW is on(def if from 1)
    GUI_MENU_DIRECT_RETURN = 0x00000001 << 2, //cancel to exit menu
    GUI_MENU_PANE = 0x00000001 << 3,
}gui_menustyle_t;

typedef enum GUI_KEYBOARD {
    GUI_KEYBOARD_EN_LOWERCASE,
    GUI_KEYBOARD_EN_UPPERCASE,
    GUI_KEYBOARD_NUM,
    GUI_KEYBOARD_NUM_SYMBOL,
    GUI_KEYBOARD_ARABIC,
}gui_keyboard_t;

//////////////////////////////////////////////////////////////////////////

typedef struct _Gui_ResText_Attr {
	unsigned char ucFontSize;
	gui_alignment_t eAlign;
	gui_fontstyle_t eStyle;
	char isAutoWrap;
} GUI_TEXT_ATTR;

typedef int (*MENUFUNC)(void);
typedef struct _Gui_ResMenuItem {
    unsigned char szText[50];
    int nValue; /*id or key value*/
    unsigned char bVisible;
    MENUFUNC vFunc;
} GUI_MENUITEM;

typedef struct _Gui_ResMenu {
    GUI_TEXT_ATTR stTitleAttr;
    GUI_TEXT_ATTR stItemAttr;
    unsigned int nSize;
    GUI_MENUITEM *pstContent;
    unsigned char szTitle[50];
} GUI_MENU;

typedef struct _Gui_InputBox_ATTR {
    unsigned char isAutoReturn;
    unsigned char nMinLen;
    unsigned char nMaxLen;

    /*
     available when mode num/mix
     */
    unsigned char bSensitive;

    /*
     unavailable when bSensitive
     */
    unsigned char bEchoMode;
} GUI_INPUTBOX_ATTR;

typedef struct _Gui_PageLine {
    unsigned char szLine[50];
    GUI_TEXT_ATTR stLineAttr;
} GUI_PAGELINE;

typedef struct _Gui_ResPage {
    GUI_TEXT_ATTR stTitleAttr;
    unsigned int nLine;
    unsigned char szTitle[50];
    GUI_PAGELINE *pstContent;
} GUI_PAGE;

typedef struct _Gui_ResOptItemData{
    unsigned char isChangeable;
    unsigned char szText[50];
    enum {
        OptItemSwitcher,
        OptItemCaller,
    }type;
    union{
        int valueInt;
        char valueStr[255];
    };
}GUI_OPTIONITEM_DATA;

typedef int (*OPTFUNC)(int index, GUI_OPTIONITEM_DATA *data);
typedef struct _Gui_ResOptItem{
    GUI_OPTIONITEM_DATA data;
    OPTFUNC vFunc;
}GUI_OPTIONITEM;

typedef struct _Gui_ResOpt{
    GUI_TEXT_ATTR stTitleAttr;
    GUI_TEXT_ATTR stItemAttr;
    unsigned int nSize;
    GUI_OPTIONITEM *pstContent;
    unsigned char szTitle[50];
    void *pOnImgHandler;
    void *pOffImgHandler;
}GUI_OPTION;


#ifdef __cplusplus
extern "C" {
#endif

// Modified by Kim_LinHB 2014-08-25 v0.4
//************************************
// Function:    Gui_Init
// Description: clear screen with background color, and set foreground color
// Returns:     int
//				GUI_OK
//				GUI_ERR_INVALIDPARAM
// Parameter:   unsigned int nBgColor
// Parameter:   unsigned int nColor
//************************************
int Gui_Init(unsigned int nBgColor, unsigned int nColor, const unsigned char *pszFontFile);

//************************************
// Function:    Gui_ClearScr
// Description: 
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//************************************
int Gui_ClearScr();

//************************************
// Function:    Gui_DrawText
// Description: 
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
// Parameter:   const unsigned char * pszText
// Parameter:   const GUI_TEXT_ATTR stTextAttr
// Parameter:   unsigned int x_percent
// Parameter:   unsigned int y_percent
//************************************
int Gui_DrawText(const unsigned char *pszText, const GUI_TEXT_ATTR stTextAttr,
		unsigned int x_percent, unsigned int y_percent);

//************************************
// Function:    Gui_DrawImage
// Description: 
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_UNSUPPORTED
//				GUI_ERR_INVALIDPARAM
// Parameter:   const unsigned char * pszImagePath
// Parameter:   unsigned int x_percent
// Parameter:   unsigned int y_percent
//************************************
int Gui_DrawImage(const unsigned char *pszImagePath,
		unsigned int x_percent, unsigned int y_percent);

//************************************
// Function:    Gui_ShowMsgBox
// Description: show message box
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
//				GUI_ERR_USERCANCELLED
//				GUI_ERR_TIMEOUT
// Parameter:   const unsigned char * pszTitle
// Parameter:   GUI_TEXT_ATTR stTitleAttr
// Parameter:   const char *pszIconFile
// Parameter:   const unsigned char * pszContent
// Parameter:   GUI_TEXT_ATTR stContentAttr
// Parameter:   enum MSGBOXTYPE eType
// Parameter:   int timeoutSec
//				< 0  no limit
//				>=0	 timeoutSec Second(s)
// Parameter:   int * pucKeyValue
//************************************
int Gui_ShowMsgBox(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr,
		const char *pszIconFile, const unsigned char *pszContent, GUI_TEXT_ATTR stContentAttr,
		gui_msgboxtype_t eType, int timeoutSec, int *pucKeyValue);

//************************************
// Function:    Gui_ShowInputBox
// Description: show input box
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
//				GUI_ERR_USERCANCELLED
//				GUI_ERR_TIMEOUT
// Parameter:   const unsigned char * pszTitle
// Parameter:   GUI_TEXT_ATTR stTitleAttr
// Parameter:   const unsigned char * pszPrompt
// Parameter:   GUI_TEXT_ATTR stPromptAttr
// Parameter:   unsigned char * pszContent
// Parameter:   GUI_TEXT_ATTR stContentAttr
// Parameter:   const GUI_INPUTBOX_ATTR * pstAttr
// Parameter:   int timeoutSec
//				< 0  no limit
//				>=0	 timeoutSec Second(s)
//************************************
int Gui_ShowInputBox(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr,
		const unsigned char *pszPrompt, GUI_TEXT_ATTR stPromptAttr,
		unsigned char *pszContent, GUI_TEXT_ATTR stContentAttr,
		const GUI_INPUTBOX_ATTR *pstAttr, int timeoutSec);

//************************************
// Function:    Gui_LoadKeyboard
// Description: load some new keyboards for input box
// Returns:     int
//              GUI_OK
//              GUI_ERR_NOTINIT
//              GUI_ERR_INVALIDPARAM
// Parameter:   const gui_keyboard_t *keyboardList
// Parameter:   int keyboardNum
//************************************
int Gui_LoadKeyboard(const gui_keyboard_t *keyboardList, int keyboardNum);

//************************************
// Function:    Gui_RemoveKeyboard
// Description: remove keyboards from the input box
// Returns:     int
//              GUI_OK
//              GUI_ERR_NOTINIT
//              GUI_ERR_INVALIDPARAM
// Parameter:   const gui_keyboard_t *keyboardList
// Parameter:   int keyboardNum
//************************************
int Gui_RemoveKeyboard(const gui_keyboard_t *keyboardList, int keyboardNum);

//************************************
// Function:    Gui_ShowTimeBox
// Description: show time setting box
// Returns:		int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
//				GUI_ERR_USERCANCELLED
//				GUI_ERR_TIMEOUT
// Parameter:   const unsigned char * pszTitle
// Parameter:   GUI_TEXT_ATTR stTitleAttr
// Parameter:   unsigned char * pszTime   (YYYYMMDD or hhmmss)
// Parameter:   GUI_TEXT_ATTR stContentAttr
// Parameter:   unsigned char isTime
// Parameter:   int timeoutSec
//				< 0  no limit
//				>=0	 timeoutSec Second(s)
//************************************
int Gui_ShowTimeBox(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr,
		unsigned char *pszTime, GUI_TEXT_ATTR stContentAttr,
		unsigned char isTime, int timeoutSec);

//************************************
// Function:    GUI_BindMenu
// Description: bind menu items, menu title to a menu
// Returns:     int
//				GUI_OK
//				GUI_ERR_INVALIDPARAM
// Parameter:   const unsigned char * psztitle
// Parameter:   GUI_TEXT_ATTR stTextAttr
// Parameter:   GUI_MENU * pstMenu
// Parameter:   GUI_MENUITEM * pstMenuItem
//************************************
int Gui_BindMenu(const unsigned char *psztitle, GUI_TEXT_ATTR stTitleAttr,
		GUI_TEXT_ATTR stTextAttr, const GUI_MENUITEM *pstMenuItem, GUI_MENU *pstMenu);

//************************************
// Function:    Gui_ShowMenuList
// Description: show menu list
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
//				GUI_ERR_USERCANCELLED
//				GUI_ERR_TIMEOUT
// Parameter:   const GUI_MENU * pstMenu
// Parameter:   enum MENUSTYLE eMode
// Parameter:   int timeoutSec
//				< 0  no limit
//				>=0	 timeoutSec Second(s)
// Parameter:   int *piSelValue
//************************************
int Gui_ShowMenuList(const GUI_MENU *pstMenu, gui_menustyle_t eMode,
					int timeoutSec, int *piSelValue);

//************************************
// Function:    Gui_ShowAlternative
// Description: 
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
//				GUI_ERR_TIMEOUT
//				GUI_ERR_USERCANCELLED
// Parameter:   const unsigned char * pszTitle
// Parameter:   GUI_TEXT_ATTR stTitleAttr
// Parameter:   const unsigned char * pszPrompt
// Parameter:   GUI_TEXT_ATTR stContentAttr
// Parameter:   const unsigned char * pszOption1
// Parameter:   int ucValue1
// Parameter:   const unsigned char * pszOption2
// Parameter:   int ucValue2
// Parameter:   int timeoutSec
// Parameter:   [input & output]int * pucSelOption
//************************************
int Gui_ShowAlternative(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr,
		const unsigned char *pszPrompt, GUI_TEXT_ATTR stContentAttr,
		const unsigned char *pszOption1, int iValue1,
		const unsigned char *pszOption2, int iValue2,
		int timeoutSec, int *piSelOption);

//************************************
// Function:    Gui_CreateInfoPage
// Description: 
// Returns:     int
//				GUI_OK
//				GUI_ERR_INVALIDPARAM
// Parameter:   const unsigned char * psztitle
// Parameter:   GUI_TEXT_ATTR stTextAttr
// Parameter:   const GUI_PAGELINE *pstContent
// Parameter:   unsigned int nLine
// Parameter:   GUI_PAGE * pstPage
//************************************
int Gui_CreateInfoPage(const unsigned char *psztitle, GUI_TEXT_ATTR stTitleAttr,
		const GUI_PAGELINE *pstContent, unsigned int nLine, GUI_PAGE *pstPage);

// Modified by Kim_LinHB 2014-8-8 v0.3
//************************************
// Function:    Gui_ShowInfoPage
// Description: 
// Returns:     int
//				GUI_OK
//				GUI_OK_NEXT
//				GUI_OK_PREVIOUS
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
//				GUI_ERR_USERCANCELLED
//				GUI_ERR_TIMEOUT
// Parameter:   const GUI_PAGE * pstPage
// Parameter:	unsigned char isMultiChapters
// Parameter:   int timeoutSec
//				< 0  no limit
//				>=0	 timeoutSec Second(s)
//************************************
int Gui_ShowInfoPage(const GUI_PAGE *pstPage, unsigned char isMultiChapters, int timeoutSec);

//************************************
// Function:    Gui_ShowSignatureBoard
// Description:
// Returns:     int
//				GUI_OK
//				GUI_ERR_UNSUPPORTED
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
//				GUI_ERR_USERCANCELLED
//				GUI_ERR_TIMEOUT
// Parameter:   const unsigned char *pszTitle
// Parameter:	GUI_TEXT_ATTR stTitleAttr,
// Parameter:   const unsigned char *pszPrompt,
// Parameter:   GUI_TEXT_ATTR stContentAttr,
// Parameter:   const unsigned char *pszOutputFile,
// Parameter:   char nMode  (reserved)
//				0: png
//				1: coordinate
// Parameter:   int timeoutSec
//				< 0  no limit
//				>=0	 timeoutSec Second(s)
//************************************
int Gui_ShowSignatureBoard(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr,
		const unsigned char *pszPrompt, GUI_TEXT_ATTR stContentAttr,
		const unsigned char *pszOutputFile,
		char nMode, int timeoutSec);

//************************************
// Function:    Gui_BindOption
// Description: bind option items, page title to a option list
// Returns:     int
//              GUI_OK
//              GUI_ERR_INVALIDPARAM
// Parameter:   const unsigned char * pszTitle
// Parameter:   GUI_TEXT_ATTR stTextAttr
// Parameter:   GUI_OPTION * pstOpt
// Parameter:   GUI_OPTIONITEM * pstOptItem
//************************************
int Gui_BindOption(const unsigned char *pszTitle, GUI_TEXT_ATTR stTitleAttr,
        GUI_TEXT_ATTR stTextAttr, const char *pszOnImg, const char *pszOffImg,
        const GUI_OPTIONITEM *pstOptItem, GUI_OPTION *pstOpt);

//************************************
// Function:    Gui_ShowOptionList
// Description: show a option list
// Returns:     int
//              GUI_OK
//              GUI_ERR_NOTINIT
//              GUI_ERR_INVALIDPARAM
//              GUI_ERR_TIMEOUT
// Parameter:   GUI_OPTION * pstOpt
// Parameter:   int timeoutSec
//              < 0  no limit
//              >=0  timeoutSec Second(s)
//************************************
int Gui_ShowOptionList(GUI_OPTION *pstOpt, int timeoutSec);

//************************************
// Function:    Gui_SetVirtualButton
// Description: Set virtual button(func, menu) visible when display idle
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
// Parameter:   char HasVirtualFuncBtn
// Parameter:	char HasVirtualMenuBtn
//************************************
int Gui_SetVirtualButton(char HasVirtualFuncBtn, char HasVirtualMenuBtn);

// Added by Kim_LinHB 2014-08-13 v0.3
//************************************
// Function:    Gui_GetImageSize
// Description: get the size of the image
// Returns:     int
// Parameter:   const unsigned char * File
// Parameter:   int * pWidth
// Parameter:   int * pHeight
//************************************
int Gui_GetImageSize(const unsigned char *File, unsigned int *pWidth, unsigned int *pHeight);

// Added by Kim_LinHB 2014-08-13 v0.3
//************************************
// Function:    Gui_GetScrWidth
// Description: Get the content screen width
// Returns:     int Width
//************************************
int Gui_GetScrWidth(void);

//************************************
// Function:    Gui_GetScrHeight
// Description: Get the content screen height
// Returns:     int Height
//************************************
int Gui_GetScrHeight(void);

//************************************
// Function:    Gui_UpdateTitle
// Description: update the title
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
// Parameter:   const char * pszTitle
// Parameter:	GUI_TEXT_ATTR stTitleAttr
//************************************
int Gui_UpdateTitle(const char *pszTitle, GUI_TEXT_ATTR stTitleAttr);

//************************************
// Function:    Gui_SetBgImage
// Description: Set the global background image
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
// Parameter:   const char *bg
//************************************
int Gui_SetBgImage(const char *bg);

//************************************
// Function:    Gui_SetGlobalBtnImg
// Description: Update image for all buttons
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_INVALIDPARAM
// Parameter:   const char *normal
// Parameter:	const char *pressed
//************************************
int Gui_SetGlobalBtnImg(const char *normal, const char *pressed);

//************************************
// Function:    Gui_UpdateKey
// Description: set text for a key value
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
//				GUI_ERR_EXCEED
// Parameter:   int index	[limit 1000]
// Parameter:	const char* text
// Parameter:	const char* normalImg
// Parameter:	const char* pressedImg
//************************************
int Gui_UpdateKey(int index, const char* text, const char *normalImg, const char *pressedImg);



typedef enum GUI_UPDOWNSTATUS {
    GUI_NONE = 0x00, GUI_UP = 0x01, GUI_DOWN = 0x02
}gui_updownstatus_t;

struct _Gui_Callback_Text{
	int x,y;
	int size;
	char *pStr;
};

typedef enum GUI_CALLBACKTYPE{
	GUI_CALLBACK_LISTEN_EVENT,		// just allow GUI_DrawXXX in this callback function
	GUI_CALLBACK_UPDATE_TEXT, 		//(for ex. update number to Persian number, special symbol)
									// data: struct _Gui_Callback_Text
	GUI_CALLBACK_UPDATE_UPDOWN_ICON,// data: gui_updownstatus_t
	GUI_CALLBACK_UPDATE_CALENDAR,	// like: Miladi2Shamsy data:char pYYYYMMDD
}gui_callbacktype_t;

typedef int (*GUI_CALLBACK)(gui_callbacktype_t type, void *data, int *dataLen);
//************************************
// Function:    Gui_RegCallback
// Description: register/un-register a callback function for some special events need to be handled during displaying page
// Returns:     int
//				GUI_OK
//				GUI_ERR_NOTINIT
// Parameter:   gui_callbacktype_t type
// Parameter:	GUI_CALLBACK func
//************************************
int Gui_RegCallback(gui_callbacktype_t type, GUI_CALLBACK func);

#ifdef __cplusplus
}
#endif
#pragma pack()

#endif
