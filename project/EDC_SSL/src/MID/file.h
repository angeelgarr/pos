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
 * 2012-11-30         xiaowy                    created
 * ============================================================================
 */

#ifndef _FILE_H
#define _FILE_H

#include "structDefine.h"

#define	FILE_EXIST		1
#define	FILE_NOEXIST	2
#define	MEM_OVERFLOW	3
#define	TOO_MANY_FILES	4
#define	INVALID_HANDLE	5
#define	INVALID_MODE	6
#define NO_FILESYS		7
#define FILE_NOT_OPENED	8
#define	FILE_OPENED		9
#define END_OVERFLOW	10
#define TOP_OVERFLOW	11
#define NO_PERMISSION	12
#define FS_CORRUPT		13





int MIDopen(const char *pathname, uchar mode);
int ex_open(char *filename, int flags, mode_t mode);
int MIDread(int fid, uchar *data, int len);
int MIDwrite(int fid, uchar *data, int len);
int MIDclose(int fid);
int MIDremove(const char *filename);
int seek(int fid, long offset, uchar fromwhere);
long filesize(const char *filename);
int MIDtruncate(int fid, long len);
long freesize(void);
int fexist(const char *filename);
//int GetFileInfo(FILE_INFO* finfo);
int FileToApp(uchar * FileName);
int FileToParam (uchar *FileName, uchar *AppName, int iType);
int FileToFont(uchar *FileName);
int FileToMonitor(uchar *FileName);
int FileToPuk(int PukType, int PukIdx, uchar *FileName);
int DelAppFile(uchar *AppName);
int GetLastError(void);
void FsRecycle(int NeedSize);
long tell(int fid);



#endif
