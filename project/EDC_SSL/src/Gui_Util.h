/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2016-? PAX Technology, Inc. All rights reserved.
 * Description:     implemented some dialogs based on XUI.
 * Revision History:
 * Date                  Author                 Action
 * 2016-1-6         Kim.L                    created
 * ============================================================================
 */
/*
 *   20160112 Kim  Merge ft lib
 */

#ifndef GUI_UTIL_H_
#define GUI_UTIL_H_

/************FT*****************/

enum{
    FT_ALIGN_NONE = 0,
    FT_ALIGN_LEFT,
    FT_ALIGN_CENTER,
    FT_ALIGN_RIGHT
};

#define DEFAULT_FONT_SIZE 32

#ifdef __cplusplus
extern "C"
{
#endif

int     FtFontLoad (const char* path);
int     FtFontLoadMem   (const unsigned char*  file_base, unsigned long  file_size);
int     FtFontFree      (void);
int     FtFontSize      (unsigned char size);
int     FtFontDoubleSize(unsigned char bDoubleWidth, unsigned char bDoubleHeight);
int     FtFontStyle     (unsigned char bold, unsigned char italic);
int     FtFontReverse   (unsigned char reverse);
int     FtFontAlign     (unsigned char align);
int     FtGetStrArea    (const char* str, int* width, int* height);
unsigned char FtPrnStr        (const char *str, ...);
unsigned char FtNextLine(void);

const unsigned char *FtGetPrnBuffer(int *iBufferLen);



/************StatusBar*****************/

#define GUI_SB_OK                           0
#define GUI_SB_ERR_COMMON       -1
#define GUI_SB_ERR_PARAM            -2
#define GUI_SB_ERR_RESOURCE       -3
#define GUI_SB_ERR_MEMORY         -4
#define GUI_SB_ERR_NEED_UNREG   -5

enum ICONTYPE{
    GUI_SB_ICON_START = 0,
    GUI_SB_ICON_UP_DOWN,
    GUI_SB_ICON_PRINTER,

    GUI_SB_ICON_MODEM,
    GUI_SB_ICON_ETHERNET,
    GUI_SB_ICON_WIRELESS,
    GUI_SB_ICON_WIFI,
    GUI_SB_ICON_BLUETOOTH,
    GUI_SB_ICON_CARDINSERTED,
    GUI_SB_ICON_BATTERY,
    GUI_SB_ICON_END,
};

/*-------------------------------------------------------------------------*/
/**
  @brief    bind an ini file which records resources & xui handle to lib
  @param    statusbar   a xui statusbar handle
  @param    cfg_ini   ini file which recorded paths of all icons.
  @return   0 ok; others error
 */
/*--------------------------------------------------------------------------*/
int Gui_SB_BindResource(void *statusbar, const char *cfg_ini);

/*-------------------------------------------------------------------------*/
/**
  @brief    register an area for updating a specific icon
  @param    eType       type id
  @param    x               relative coordinates
  @param    y               relative coordinates
  @param    width
  @param    height
  @param    refreshFrequency_ms
                      the frequency of refreshing,
                      unit is millisecond,
                      if is -1, it can only be updated by call GUI_SB_ManuallyUpdate()
                      only affect the icon which's type id >= GUI_SB_ICON_MODEM
  @return       0 ok ; <0 error
 */
/*--------------------------------------------------------------------------*/
int Gui_SB_RegIcon(enum ICONTYPE eType, int x, int y, int width, int height, int refreshFrequency_ms);

/*-------------------------------------------------------------------------*/
/**
  @brief    refresh an icon manually
  @param    eType      type id
  @param    iconFile   if it is NULL, then will clear that icon area
  @return     0 ok; others error
 */
/*--------------------------------------------------------------------------*/
int Gui_SB_ManuallyRefresh(enum ICONTYPE eType, const char *iconFile);

/*-------------------------------------------------------------------------*/
/**
  @brief    un-register an icon
  @param    eType       icon type
  @return   0 ok; others error
 */
/*--------------------------------------------------------------------------*/
int Gui_SB_UnRegIcon(enum ICONTYPE eType);





/************Other******************/

typedef int (*CALCWIDTH)(const char *str, int len, int fontSize);
char *AutoWrap(const char *str, int lineWidth, int fontSize, CALCWIDTH calc_func);


#ifdef __cplusplus
}
#endif

#endif /* GUI_UTIL_H_ */
