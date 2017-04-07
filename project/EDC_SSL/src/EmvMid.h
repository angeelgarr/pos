/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2014-? PAX Technology, Inc. All rights reserved.
 * Description:     Prolin EMV MID
 * Revision History:
 * Date                  Author                 Action
 * 20141210         Kim.L               created
 * ============================================================================
 */

#ifndef _EMV_MID_H
#define _EMV_MID_H

#define CAPKINFO	"./data/CAPKINFO"
#define AIDINFO		"./data/AIDINFO"


int SaveCAPKFile(EMV_CAPK  *pstCAPK);
int AddCAPKIntoEmvLib(uchar ucKeyID, uchar *sRID);
int AddAppIntoEmvLib(void);
int SaveAIDFile(EMV_APPLIST *pstApp);
int ReadCAPKFile(int index, EMV_CAPK  *pstCAPK);
int ReadAIDFile(int index, EMV_APPLIST *pstApp);

int EMVProcTrans(void);

/*
int FileToStruct(char *pszFileName, ST_EVENT_MSG *pstEvent, ST_TMS_MSG *pstTmsMsg);
int StructToFile(char *pszFileName, ST_EVENT_MSG *pstEvent, ST_TMS_MSG *pstTmsMsg);
*/

#endif // _EMV_MID_H
