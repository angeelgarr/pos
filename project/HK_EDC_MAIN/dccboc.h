/************************************************************************
NAME
    DccPP.h - Declares the BOC DCC public functions

DESCRIPTION
	None.

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    HeJiajiu(Gary Ho)     2010.02.25      - draft
************************************************************************/

#ifndef DCC_BOC_HEADER_FILE
#define DCC_BOC_HEADER_FILE


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Function declarations here
uchar BOCDCC_ChkIfAllow(void);
int BOCDCC_RateLookup(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

// end of file
