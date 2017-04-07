/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2016-? PAX Technology, Inc. All rights reserved.
 * Description:
 * Revision History:
 * Date                  Author                 Action
 * 20160112          Kim.L                    create
 * ============================================================================
 */

#include "posapi.h"
#include "print.h"

#define DATA_ERROR1  1
#define DATA_ERROR2  2
#define DATA_ERROR3  3
#define DATA_ERROR4  4

struct PAX_PRN
{
	unsigned char  count[2];
	unsigned char  content[384];
} Pax_prn;

struct PAX_LOGO
{
	unsigned char  line;
	struct PAX_PRN  prn_txt[255];
} Pax_logo;


typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned int       DWORD;

typedef long LONG;

#define min(x, y)   ((x) > (y) ? (y) : (x))

#ifndef _WIN32
#pragma pack(1)
typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;
#pragma pack()
#endif

//using bilinear
int _24bitBmpScaling_Prn(const char *srcFile, const char *desFile, int desW, int desH)
{
    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bmiHeader;

    int fd;
    if ((fd = open(srcFile, O_RDONLY)) < 0)
    {
        printf("open bmp file error.");
        return -1;
    }

    lseek(fd, 0, SEEK_SET);
    read(fd, &bmfHeader, sizeof(BITMAPFILEHEADER));
    read(fd, &bmiHeader, sizeof(BITMAPINFOHEADER));

    int bitCount = bmiHeader.biBitCount;
    int srcW = bmiHeader.biWidth;
    int srcH = bmiHeader.biHeight;

    if (bitCount != 24)
    {
        return -2;
    }

    int lineSize = ((srcW * bitCount + 31) / 32) * 4;
    int alignBytes = (int)(lineSize - srcW * bitCount * 1.0 / 8L);
    int srcBufSize = lineSize * srcH;
    BYTE* srcBuf = (BYTE *)malloc(srcBufSize * sizeof(BYTE));
    memset(srcBuf, 0, srcBufSize * sizeof(BYTE));
    int i, j, k;

    for (i = 0; i < srcH; i++)
    {
        read(fd, &srcBuf[lineSize * i], lineSize - alignBytes);
        lseek(fd, alignBytes, SEEK_CUR);
    }

    close(fd);

    int desLineSize = ((desW * bitCount + 31) / 32) * 4;
    int desBufSize = desLineSize * desH;
    BYTE *desBuf = (BYTE *)malloc(desBufSize * sizeof(BYTE));
    double rateH = (double)srcH / desH;
    double rateW = (double)srcW / desW;

    for (i = 0; i < desH; i++)
    {
        int tH = (int)(rateH * i);
        int tH1 = min(tH + 1, srcH - 1);
        float u = (float)(rateH * i - tH);
        for (j = 0; j < desW; j++)
        {
            int tW = (int)(rateW * j);
            int tW1 = min(tW + 1, srcW - 1);
            float v = (float)(rateW * j - tW);

            //f(i+u,j+v) = (1-u)(1-v)f(i,j) + (1-u)vf(i,j+1) + u(1-v)f(i+1,j) + uvf(i+1,j+1)
            for (k = 0; k < 3; k++)
            {
                desBuf[i * desLineSize + j * bitCount / 8 + k] =
                    (1 - u)*(1 - v) * srcBuf[tH * lineSize + tW * bitCount / 8 + k] +
                    (1 - u)*v*srcBuf[tH1 * lineSize + tW * bitCount / 8 + k] +
                    u * (1 - v) * srcBuf[tH * lineSize + tW1 * bitCount / 8 + k] +
                    u * v * srcBuf[tH1 * lineSize + tW1 * bitCount / 8 + k];
            }
        }
    }

    free(srcBuf);

    int newBitCount = 1;
    lineSize = ((desW * newBitCount + 31) & ~31) / 8L;
    desBufSize = desH * lineSize;
    BYTE *desBuf2 = (BYTE *)malloc(desBufSize * sizeof(BYTE));
    memset(desBuf2, 0, desH * lineSize *sizeof(BYTE));
    int y = 0;
    for (i = 0; i < desH; i++)
    {
        y = 0;
        for (j = 0; j < desW; j++)
        {
            int index = i * desLineSize + j * bitCount / 8;
            if (j != 0 && 0 == j % 8)
                ++y;
            BYTE bit = (desBuf[index] + desBuf[index + 1] + desBuf[index + 2])/3 >120;
            desBuf2[i * lineSize + y] |= (bit << (7 - j%8));
        }
    }

    free(desBuf);
    remove(desFile);

    fd = open(desFile, O_CREAT | O_RDWR);
    BITMAPFILEHEADER nbmfHeader;
    nbmfHeader.bfType = 0x4D42;
    nbmfHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2 + desBufSize;
    nbmfHeader.bfReserved1 = 0;
    nbmfHeader.bfReserved2 = 0;
    nbmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2;
    BITMAPINFOHEADER   bmi;
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biWidth = desW;
    bmi.biHeight = desH;
    bmi.biPlanes = 1;
    bmi.biBitCount = 1;
    bmi.biCompression = 0;
    bmi.biSizeImage = desBufSize;
    bmi.biXPelsPerMeter = 0;
    bmi.biYPelsPerMeter = 0;
    bmi.biClrUsed = 0;
    bmi.biClrImportant = 0;

    write(fd, &nbmfHeader, sizeof(BITMAPFILEHEADER));
    write(fd, &bmi, sizeof(BITMAPINFOHEADER));
    RGBQUAD color[2] = { {0,0,0,0}, {255,255,255,0} };
    write(fd, color, sizeof(RGBQUAD) * 2);
    write(fd, desBuf2, desBufSize * sizeof(BYTE));
    close(fd);

    free(desBuf2);
    return 0;
}

void conv_bmp(unsigned char *dest, unsigned char *src, int len, int jplen, unsigned char off)
{
	int i, tlen;
	//the last pix need reversed
	if (off != 0)
	{
		tlen = len - jplen-1;
	}
	else
	{
		tlen = len - jplen;
	}
	for(i=0; i<tlen; i++)
	{
		dest[i] = src[i] ^ 0xff;
	}

	if(off!= 0)
	{
		dest[tlen] = src[tlen] ^ (0xff<<(8-off));
	}
}

int PrnBmp(unsigned char *filename,int mode,char alignment, unsigned char *gMallocBuffer)
{
	int fp;
	unsigned char *bmp = NULL;
	long LineByte;
	uint i=0;
	int iFileLen;
	int iBmpPixWidth, iBmpWidth, iBmpFillLen ,iBmpPixBitCount,iBmpByteSize;
	unsigned char ucOff;
	unsigned short usLeftIndent = 0;
	LineByte=filesize(filename);
	if(LineByte < 0)
		return -2;

	fp=open(filename,O_RDWR);
	if(fp < 0) {
		return -1;
	}

	bmp = (unsigned char *)malloc(LineByte);
	memset(bmp,0,sizeof(bmp));
	read(fp,bmp,LineByte);
	close(fp);

	if(!(bmp[0x00]=='B' && bmp[0x01]=='M')){
		free(bmp);
		return  DATA_ERROR1;
	}

	iFileLen = ((int)bmp [0x05]) * 16777216 +
		((int)bmp [0x04]) * 65536 +
		((int)bmp [0x03]) * 256  +
		(int)bmp [0x02];

	iBmpPixWidth = ((int)bmp [0x15]) * 16777216 +
		((int)bmp [0x14]) * 65536 +
		((int)bmp [0x13]) * 256  +
		(int)bmp [0x12];
	if(iBmpPixWidth>384) {
		free(bmp);
		return  DATA_ERROR4;
	}

	iBmpPixBitCount = ((int)bmp[0x1d]) * 256 + ((int)bmp[0x1c]);
	if(iBmpPixBitCount != 1) {
		free(bmp);
		return DATA_ERROR2;
	}

	iBmpByteSize = ((int)bmp[0x25]) * 16777216 +
	    ((int)bmp [0x24]) * 65536 +
		((int)bmp [0x23]) * 256  +
		(int)bmp [0x22];
	if(iBmpByteSize > 36864) {
		free(bmp);
		return DATA_ERROR3;
	}

	ucOff = iBmpPixWidth%8;
	if(ucOff !=0) {
		iBmpPixWidth +=8;
	}

	if(((iBmpPixWidth/8)%4) != 0) {
		iBmpFillLen = 4-(iBmpPixWidth/8)%4;
	}
	else {
		iBmpFillLen = 0;
	}
	iBmpWidth = iBmpPixWidth/8+iBmpFillLen;

	switch(alignment) {
	case 1: usLeftIndent = (384/8 - iBmpWidth)/2;break;
	case 2: usLeftIndent = 384/8 - iBmpWidth;break;
	case 0:
	default:
		break;
	}

	Pax_logo.line = bmp[0x16];
	for(i=0; i<Pax_logo.line; i++) {
		memset(Pax_logo.prn_txt[i].content, 0, 72);
		Pax_logo.prn_txt[i].count[0] = 0;
		Pax_logo.prn_txt[i].count[1] = 72;
		if(mode)
			memcpy(Pax_logo.prn_txt[i].content + usLeftIndent, &bmp[iFileLen-(i+1)*iBmpWidth], iBmpWidth);
		else
			conv_bmp(Pax_logo.prn_txt[i].content + usLeftIndent, &bmp[iFileLen-(i+1)*iBmpWidth], iBmpWidth, iBmpFillLen, ucOff);
	}

	int iMaxLineInOneBuf = 20000 / 74;
	gMallocBuffer[0] = Pax_logo.line;

	int iCnt = 0;
	for(i= 0;i<Pax_logo.line; i++) {
		memcpy(&gMallocBuffer[iCnt*74+1], Pax_logo.prn_txt[i].count, 2);
		memcpy(&gMallocBuffer[iCnt*74+2+1], Pax_logo.prn_txt[i].content, 72);
		++iCnt;
		if(iCnt >= iMaxLineInOneBuf)
		{
		    PrnLogo(gMallocBuffer);
		    iCnt = 0;
		}
	}
	if(iMaxLineInOneBuf > Pax_logo.line){
	    PrnLogo(gMallocBuffer);
	}

	free(bmp);
	return 0;
}
