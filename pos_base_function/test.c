#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>

#include "applib.h"
void ComOutput( unsigned char *buf,int len,int mode,char *string, ...);

int main(void)
{
	FIELD_ATTR attr;
uchar szInput[20] = "\x23\x43\x56";
uchar szOnput[30] = {0};
uint len;

len = 0;
	memset(&attr,0,sizeof(FIELD_ATTR));
	attr.eElementAttr = Attr_n;
	attr.eLengthAttr = Attr_fix;
	attr.uiLength = 6;
	UnPackElement(&attr,szInput,szOnput,&len);
	printf("len=%d\n",len);
	ComOutput(szOnput,len * 2,4,"output");s
	return 0;
}

int UnPackElement(FIELD_ATTR *pAttr, uchar *pusIn, uchar *pusOut,
                   uint *puiInLen)
{
    uint    i, j, iInLen, iTmpLen;

    memset(pusOut, 0, pAttr->uiLength);
    *puiInLen = 0;

    switch( pAttr->eLengthAttr ){
    case Attr_fix:
        iInLen  = pAttr->uiLength;
        iTmpLen = iInLen;
        break;

    case Attr_var1:
        iTmpLen= (pusIn[0]>>4)*10 + (pusIn[0]&0x0F);
        pusIn++;
        iInLen= 1+iTmpLen;
        break;

    case Attr_var2:
        iTmpLen = (pusIn[0]&0x0F)*100 + (pusIn[1]>>4)*10 + (pusIn[1]&0x0F);
        pusIn += 2;
        iInLen = 2+iTmpLen;
        break;
    }   /*** switch(pAttr->eLengthAttr ***/

    if( iTmpLen>pAttr->uiLength ){
        return -1;
    }
    if( pAttr->eElementAttr==Attr_b ){
        pusOut[0] = (uchar)(iTmpLen>>8);
        pusOut[1] = (uchar)iTmpLen;
        pusOut   += 2;
    }

    switch( pAttr->eElementAttr ){
    case Attr_n:
        switch( pAttr->eLengthAttr ){
        case Attr_fix:
            iInLen= (pAttr->uiLength+1)/2;

            for(i=0,j=0; i<pAttr->uiLength; i+=2,j++){
                if( (pusIn[iInLen-j-1]&0x0F)<0x0A ){
                    pusOut[pAttr->uiLength-i-1] = (pusIn[iInLen-j-1]&0x0F)|0x30;
                }else{
                    pusOut[pAttr->uiLength-i-1] =
                        (pusIn[iInLen-j-1]&0x0F)-0x0A+'A';
                }

                if( i!=pAttr->uiLength-1 ){
                    if( (pusIn[iInLen-j-1]>>4)<0x0A ){
                        pusOut[pAttr->uiLength-i-2] =
                           (pusIn[iInLen-j-1]>>4)|0x30;
                    }else{
                        pusOut[pAttr->uiLength-i-2]=
                           (pusIn[iInLen-j-1]>>4)-0x0A+'A';
                    }
                }
            }
            break;

        case Attr_var1:
        case Attr_var2:
            iInLen = iInLen - iTmpLen + (iTmpLen+1)/2;

#ifndef VarAttrN_RightJustify
            for(i=0; i<(iTmpLen/2); i++){
                if( (pusIn[i]>>4)<0x0A ){
                    pusOut[2*i] = (pusIn[i]>>4) | 0x30;
                }else{
                     pusOut[2*i] = (pusIn[i]>>4) -0x0A + 'A';
                }

                if( (pusIn[i]&0x0F)<0x0A ){
                     pusOut[2*i+1] = (pusIn[i]&0x0F) | 0x30;
                }else{
                     pusOut[2*i+1] = (pusIn[i]&0x0F) - 0x0A + 'A';
                }
            }

            if( iTmpLen%2 ){
                if( (pusIn[i]>>4)<0x0A ){
                     pusOut[2*i] = (pusIn[i]>>4) | 0x30;
                }else{
                     pusOut[2*i] = (pusIn[i]>>4) - 0x0A + 'A';
                }
            }
#else
            for(i=0,j=0; i<iTmpLen; i+=2,j++){
                if( (pusIn[(iTmpLen+1)/2-j-1]&0x0F)<0x0A ){
                    pusOut[iTmpLen-i-1] = (pusIn[(iTmpLen+1)/2-j-1]&0x0F)|0x30;
                }else{
                     pusOut[iTmpLen-i-1] =
                        (pusIn[(iTmpLen+1)/2-j-1]&0x0F)-0x0A+'A';
                }

                if( i!=pAttr->uiLength-1 ){
                    if( (pusIn[(iTmpLen+1)/2-j-1]>>4)<0x0A ){
                        pusOut[iTmpLen-i-2] =
                           (pusIn[(iTmpLen+1)/2-j-1]>>4)|0x30;
                    }else{
                        pusOut[iTmpLen-i-2] =
                           (pusIn[(iTmpLen+1)/2-j-1]>>4)-0x0A+'A';
                    }
                }
            }
#endif
            break;
        }   /*** switch(pAttr->eLengthAttr) ***/
        break;

    case Attr_z:
        switch( pAttr->eLengthAttr ){
        case Attr_fix:
            iInLen = (pAttr->uiLength+1)/2;

            for(i=0,j=0; i<pAttr->uiLength; i+=2,j++){
                pusOut[pAttr->uiLength-i-1]= (pusIn[iInLen-j-1]&0x0F)|0x30;
                if( i!=pAttr->uiLength-1 ){
                     pusOut[pAttr->uiLength-i-2]= (pusIn[iInLen-j-1]>>4)|0x30;
                }
            }
            break;

        case Attr_var1:
        case Attr_var2:
            iInLen = iInLen - iTmpLen + (iTmpLen+1)/2;

            for(i=0; i<(iTmpLen/2); i++){
                pusOut[2*i]   = (pusIn[i]>>4) | 0x30;
                pusOut[2*i+1] = (pusIn[i]&0x0F) | 0x30;
            }
            if( iTmpLen%2 ){
                pusOut[2*i]= (pusIn[i]>>4) | 0x30;
            }
            break;
        }   /*** switch(pAttr->eLengthAttr) ***/
        break;

    case Attr_b:
        memcpy(pusOut, pusIn, iTmpLen);
        break;

    case Attr_a:
        memcpy(pusOut, pusIn, iTmpLen);
        break;
    }    /*** switch(pAttr->eElementAttr) ***/

    *puiInLen = iInLen;

    return 0;
}
void ComOutput( unsigned char *buf,int len,int mode,char *string, ...)
{
	int i;
	uchar printBuf[1024 * 4];
	va_list varg;
	char varBuf[100];
	
    
	if(NULL == string || buf == NULL)
	{
		
		printf("ComOutput para error");
		
	}
	memset(printBuf,0,sizeof(printBuf));
	memset(varBuf,0,sizeof(varBuf));
	if(mode == HEX_MODE)
	{
		for(i = 0;i < len;i++)
		{
			sprintf(printBuf + i * 3,"%02X ",buf[i]);
		}
	}
	else
	{
		for(i = 0;i < len;i++)
		{
			sprintf(printBuf + i,"%c",buf[i]);
		}
	}
	va_start(varg,string);
	vsprintf(varBuf,string,varg);
	for(i = 0;i < strlen(varBuf);i++)
	{
		printf("%c",varBuf[i]);
	}

	for(i = 0;i < strlen(printBuf);i++)
	{
		printf("%c",printBuf[i]);
	}
	printf("\n");
}