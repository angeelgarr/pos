/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2012-? PAX Technology, Inc. All rights reserved.
 * Description:
 * Revision History:
 * Date                  Author                 Action
 * 2012-11-29          xiaowy                     created
 * ============================================================================
 */

#ifndef MIDSYS_H_
#define MIDSYS_H_

#include "structDefine.h"


int GetMidVersion(char* ver);
void Mid_OpenDevice();
void Mid_CloseDevice();

uchar SystemInit(void);

void Beep(void);
void Beef(uchar mode, int DlyTime);
void BeepF(uchar mode, int DlyTime);

uchar SetTime(uchar *time);
void GetTime(uchar *time);

void DelayMs(int Ms);

void TimerSet(uchar TimerNo, int Cnts);
int TimerCheck(uchar TimerNo);

void ReadSN(uchar *SerialNo);
void EXReadSN(uchar *SN);

uchar ReadVerInfo(uchar *VerInfo);
int GetTermInfo(uchar *out_info);
//int GetTermInfoExt(uchar *InfoOut, int InfoOutLen);


int ReadFontLib(unsigned long Offset, uchar *FontData, int ReadLen);
//int EnumFont (ST_FONT *Fonts, int MaxFontNums);
//int EnumBaseFont (ST_FONT *Fonts, int MaxFontNums);

uchar GetEnv(char *name, uchar *value);
uchar PutEnv(char *name, uchar *value);

int SysConfig(uchar *ConfigInfoIn, int InfoInLen);
void LedDisplay(unsigned char type, unsigned char *str);

int Reboot();
void PowerOff(void);

void PciGetRandom(uchar *random);
int SysSleep(uchar *DownCtrl);

void SysIdle(void);
uchar OnBase(void);

uchar BatteryCheck(void);

#endif /* MIDSYS_H_ */
