/************************************************************************
NAME
    dccboc.c - Definitions of the BOC DCC public functions

DESCRIPTION
	None

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    HeJiajiu(Gary Ho)     2010.02.25      - draft
************************************************************************/
#include "global.h"

uchar BOCDCC_ChkIfAllow(void)
{
	if (!ChkEdcOption(EDC_ENABLE_DCC))
	{
		return FALSE;
	}
	// there still be another check point
	return TRUE;
}

int BOCDCC_RateLookup(void)
{
	if (!BOCDCC_ChkIfAllow())
	{
		return ERR_NO_DISP;
	}
	return ERR_NO_DISP;
}

// end of file
