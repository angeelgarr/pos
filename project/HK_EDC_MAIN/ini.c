/*
 ============================================================================
 Name        : ini.c
 Author      : Verne
 Version     : 
 Copyright   : PAX Computer Technology(Shenzhen) CO., LTD
 Description : PAX POS Shared Library
 ============================================================================
 */
/************************************************************************/
/* 20130417 修改                                                        */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
//                include file here                                     //
//////////////////////////////////////////////////////////////////////////
#include "stdio.h"
#include <posapi.h>
#include <posapi_all.h>
#include "string.h"
#include "stdlib.h"
#include "ctype.h"
#include "ini.h"
#include "global.h"
//////////////////////////////////////////////////////////////////////////
//                 define static var here                               //
//////////////////////////////////////////////////////////////////////////
#define MAX_FILE_BUFFER						1024*50
static int sg_iErrIndex;						// error index
static int sg_iMaxFileSize = MAX_FILE_BUFFER;		// max file size, default is 10k

//////////////////////////////////////////////////////////////////////////
//                 function define                                      //
//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
{
#endif

const char* IniGetLastErrMsg()
{
	switch(sg_iErrIndex)
		{
		case INIFILE_SUCCESSED:					
			return "SUCCESSED";
		case INIFILE_ERROR_PARA:				
			return "PARA ERROR";
		case INIFILE_ERROR_OPENFILE:			
			return "OPEN FILE FAILED";
		case INIFILE_ERROR_FILESIZE:			
			return "FILESIZE TOO BIG";
		case INIFILE_ERROR_KEY_NOTFOUND:		
			return "FIND KEY FAILED";
		case INIFILE_ERROR_VALUE_NOTFOUND:		
			return "FIND VALUE FAILED";
		case INIFILE_WRITE_ERROR:				
			return "WRITE FILE FAILED";
		case INIFILE_ERROR_MALLOC:				
			return "MALLOC MEMORY FAILED";
		default:								
			return "UNKNOWN ERROR";
		}
}

const char* IniGetVer()
{
	return "1.0.0.0"; // 版本号如何命名？  1 大  2 小 3 build号？
}

void IniSetMaxFileSize(int iFileSize)
{
	sg_iMaxFileSize = iFileSize;
}

// read ini file and save to pBufOut
// return 0: successed
//        1: open file failed
//        2: file size too big
//       -1: unknow Error
#if 1
int GetFileSize(unsigned char *szFileName)
{
	int fd, len;
	
	fd = ex_open(szFileName, O_RDWR, "\xFF\x04");
	if(fd < 0) return -1;

	seek(fd, 0, SEEK_END);
	len = tell(fd);
	if(len < 0) return -1;
	close(fd);
	
	return len;
}
#endif
static int load_ini_file(const char *pszFile, char *pBufOut,int *piFile_size)
{
	int fd_ini_file=0;
	int ret=0;
	int iLen=0;
	
	
	iLen = (int)GetFileSize((unsigned char*)pszFile);
	//iLen =(int)filesize(pszFile);
	if(iLen > sg_iMaxFileSize)
		return 2;
		
	fd_ini_file = ex_open((char *)pszFile,O_CREATE | O_RDWR,"\xFF\x04");
	//printk("\r\n[load_ini_file]----%d----ex_open:%s %d errno:%s(%d)\r\n",iLen,pszFile,fd_ini_file,strerror(errno),errno);
	if(fd_ini_file < 0)
	{
		return 1;
	}

	ret = read(fd_ini_file, pBufOut, iLen);
	//printk("[load_ini_file]----------------------------------------read:%d iLen:%d\n",ret,iLen);
	close(fd_ini_file);
	if ( ret >= 0 && ret == iLen )
	{
		*piFile_size = iLen;
		return 0;
	}
	else if ( -1 == ret)
	{
		return 2;
	}
	return 1;
}

static int load_ini_file2(const char *pszFile, char *pBufOut,int *piFile_size)
{
	int fd_ini_file=0;
	int ret=0;
	int iLen=0;
	
	iLen =(int)filesize((char *)pszFile);
	if(iLen > sg_iMaxFileSize)
		return 2;
		
	fd_ini_file = open((char *)pszFile,O_RDWR|O_CREATE);

	//printk("[load_ini_file]---------------------------iLen:%d\n",iLen);
	//printk("\r\n[load_ini_file]-------open:%s %d\r\n",pszFile,fd_ini_file);
	if(fd_ini_file < 0)
	{
		return 1;
	}

	ret = read(fd_ini_file, pBufOut, iLen);
	//printk("[load_ini_file]--------------------read-----------------ret:%d iLen:%d\n",ret,iLen);
	close(fd_ini_file);
	if ( ret >= 0 && ret == iLen )
	{
		*piFile_size = iLen;
		return 0;
	}
	else if ( -1 == ret)
	{
		return 2;
	}
	return 1;
}

static int newline(char c)
{
	return ('\n' == c ||  '\r' == c )? 1 : 0;
}

static int end_of_string(char c)
{
	return '\0'==c? 1 : 0;
}

//[]
static int left_barce(char c)	// used for find section
{
	return '[' == c? 1 : 0;
}

static int isright_brace(char c )	// used for find section
{
	return ']' == c? 1 : 0;
}


//////////////////////////////////////////////////////////////////////////
// @brief find section & key=value in the buf
// @brief return 0: successfully
//               1: not found
 //////////////////////////////////////////////////////////////////////////
static int parse_file(const char *pszSection, const char *pszKey, const char *pContentBuf, int *piSec_s,
		 int *piSec_e, int *piKey_s, int *piKey_e, int *piValue_s, int *piValue_e)
{
	const unsigned char *pusBuf = (unsigned char*)pContentBuf;	// pBuf must be unsigned
	int i = 0, j = 0;
	int iSectionStart = 0;
	int iNewLineStart = 0;
	int iKeyLen = 0;

	if(NULL != pszKey)
	{
		iKeyLen = strlen(pszKey);
	}

	*piSec_e = *piSec_s = *piKey_e = *piKey_s = *piValue_s = *piValue_e = -1;

	while( !end_of_string(pusBuf[i]) )
	{
		//find the section
		if( ( 0 == i ||  newline(pusBuf[i - 1]) ) && left_barce(pusBuf[i]) )
		{
			iSectionStart = i + 1;

			//find the ']'
			do {
				i++;
			} while( !isright_brace(pusBuf[i]) && !end_of_string(pusBuf[i]));

			if( 0 == strncmp((char*)(pusBuf + iSectionStart), pszSection, i - iSectionStart))	// case sensitive ???
			{
				iNewLineStart = 0;

				i++;

				//Skip over space char after ']'
				while(isspace(pusBuf[i]))		// if pBuf is char*, it will be overflow in isspace, but works in unsign char*, (12-9-14/Verne)
				{
					i++;
				}

				//find the section
				*piSec_s = iSectionStart;
				*piSec_e = i;

				while( ! (newline(pusBuf[i - 1]) && left_barce(pusBuf[i])) && !end_of_string(pusBuf[i]) )
				{
					j = 0;
					//get a new line
					iNewLineStart = i;

					while( !newline(pusBuf[i]) &&  !end_of_string(pusBuf[i]) )
					{
						i++;
					}

					//now i  is equal to end of the line
					j = iNewLineStart;

//					if(';' != pusBuf[j]) //skip over comment
					if(';' != pusBuf[j] && ';' != pusBuf[j + 1]) //skip over comment
					{
						while(j < i && '=' != pusBuf[j])
						{
							j++;
							if('=' == pusBuf[j])
							{
								if (iKeyLen != j - iNewLineStart)	// len not equal
								{
									break;
								}
								if(strncmp(pszKey, (char*)(pusBuf + iNewLineStart), j - iNewLineStart) == 0) 	// case sensitive and no trimleft,trimright  ???
								{
									//find the key ok
									*piKey_s = iNewLineStart;
									*piKey_e = j - 1;

									*piValue_s = j + 1;
									*piValue_e = i;

									return 0;
								}
							}// if('=' == pusBuf[j]) 
						} // end of while(j < i && pusBuf[j]!='=') 
					} // if(';' != pusBuf[j]) //skip over comment 

					i++;
				} // end of while( ! (newline(pusBuf[i-1]) && left_barce(pusBuf[i])) && !end_of_string(pusBuf[i]) ) 
			} // if( 0 == strncmp(pusBuf+uiSectionStart,pszSection, i-uiSectionStart))
		}// if( ( 0==i ||  newline(pusBuf[i-1]) ) && left_barce(pusBuf[i]) ) 
		else
		{
			i++;
		}
	} // end of while( !end_of_string(pusBuf[i]) ) 
	return 1;
}

int IniReadString(const char *pszSection, const char *pszKey, char *pszValue, int iSize, const char *pszDefaultValue, const char *pszFile)
{
	//char* psBuf;
	int iFileSize=0;
	int iSec_s, iSec_e, iKey_s, iKey_e, iValue_s, iValue_e;
	int iRetCode;
	int iCount;
	char psBuf[MAX_FILE_BUFFER];

	//check parameters
	if (NULL == pszSection || NULL == pszKey || NULL == pszValue || NULL == pszFile || iSize <= 0)
	{
		return INIFILE_ERROR_PARA;
	}

	if (0 == strlen(pszFile) || 0 == strlen(pszKey) || 0 == strlen(pszSection))
	{
		return INIFILE_ERROR_PARA;
	}

	memset(psBuf,0x00,sizeof(psBuf));
	iRetCode = load_ini_file(pszFile, psBuf, &iFileSize);
	switch(iRetCode)
	{
	case 1:
		if(NULL != pszDefaultValue)
		{
			strncpy(pszValue, pszDefaultValue, iSize);
		}
		return sg_iErrIndex = INIFILE_ERROR_OPENFILE;
	case 2:
		if(NULL != pszDefaultValue)
		{
			strncpy(pszValue, pszDefaultValue, iSize);
		}
		return sg_iErrIndex = INIFILE_ERROR_FILESIZE;
	case 0:	// OK
	default:
		break;
	}

	if(parse_file(pszSection, pszKey, psBuf, &iSec_s, &iSec_e, &iKey_s, &iKey_e, &iValue_s, &iValue_e))
	{
		if(NULL != pszDefaultValue)
		{
			strncpy(pszValue, pszDefaultValue, iSize);
		}

		return sg_iErrIndex = INIFILE_ERROR_KEY_NOTFOUND; //not find the key
	}
	else
	{
		iCount = iValue_e - iValue_s;
		if (iCount == 0) // uiCount=0，not value, fill with default value
		{
			if (NULL != pszDefaultValue)
			{
				strncpy(pszValue, pszDefaultValue, iSize);
			}

			return sg_iErrIndex = INIFILE_ERROR_VALUE_NOTFOUND;	// can not find value
		}

		if( iSize - 1 < iCount)
		{
			iCount =  iSize - 1;
		}

		memset(pszValue, 0x00, iSize);
		memcpy(pszValue, psBuf + iValue_s, iCount );
		pszValue[iCount] = '\0';

		return sg_iErrIndex = INIFILE_SUCCESSED;
	}
}

int IniReadString2(const char *pszSection, const char *pszKey, char *pszValue, int iSize, const char *pszDefaultValue, const char *pszFile)
{
	//char* psBuf;
	int iFileSize=0;
	int iSec_s, iSec_e, iKey_s, iKey_e, iValue_s, iValue_e;
	int iRetCode;
	int iCount;
	char psBuf[MAX_FILE_BUFFER];

	//check parameters
	if (NULL == pszSection || NULL == pszKey || NULL == pszValue || NULL == pszFile || iSize <= 0)
	{
		return INIFILE_ERROR_PARA;
	}

	if (0 == strlen(pszFile) || 0 == strlen(pszKey) || 0 == strlen(pszSection))
	{
		return INIFILE_ERROR_PARA;
	}

	memset(psBuf,0x00,sizeof(psBuf));
	iRetCode = load_ini_file2(pszFile, psBuf, &iFileSize);
	switch(iRetCode)
	{
	case 1:
		if(NULL != pszDefaultValue)
		{
			strncpy(pszValue, pszDefaultValue, iSize);
		}
		return sg_iErrIndex = INIFILE_ERROR_OPENFILE;
	case 2:
		if(NULL != pszDefaultValue)
		{
			strncpy(pszValue, pszDefaultValue, iSize);
		}
		return sg_iErrIndex = INIFILE_ERROR_FILESIZE;
	case 0:	// OK
	default:
		break;
	}

	if(parse_file(pszSection, pszKey, psBuf, &iSec_s, &iSec_e, &iKey_s, &iKey_e, &iValue_s, &iValue_e))
	{
		if(NULL != pszDefaultValue)
		{
			strncpy(pszValue, pszDefaultValue, iSize);
		}

		return sg_iErrIndex = INIFILE_ERROR_KEY_NOTFOUND; //not find the key
	}
	else
	{
		iCount = iValue_e - iValue_s;
		if (iCount == 0) // uiCount=0￡?not value, fill with default value
		{
			if (NULL != pszDefaultValue)
			{
				strncpy(pszValue, pszDefaultValue, iSize);
			}

			return sg_iErrIndex = INIFILE_ERROR_VALUE_NOTFOUND;	// can not find value
		}

		if( iSize - 1 < iCount)
		{
			iCount =  iSize - 1;
		}

		memset(pszValue, 0x00, iSize);
		memcpy(pszValue, psBuf + iValue_s, iCount );
		pszValue[iCount] = '\0';

		return sg_iErrIndex = INIFILE_SUCCESSED;
	}
}


int IniReadInt( const char *pszSection, const char *pszKey, int iDefaultValue, const char *pszFile)
{
#define INI_MAX_INT_LEN    32
	char pszValue[INI_MAX_INT_LEN] = {0};

	if(IniReadString(pszSection, pszKey, pszValue, INI_MAX_INT_LEN,"", pszFile))
	{
		return iDefaultValue;
	}
	else
	{
		return atoi(pszValue);
	}

#ifdef INI_MAX_INT_LEN
#undef INI_MAX_INT_LEN
#endif

}

int IniWriteString(const char *pszSection, const char *pszKey, const char *pszValue, const char *pszFile)
{
	char psBuf[MAX_FILE_BUFFER];
	char psW_buf[MAX_FILE_BUFFER];
	int iSec_s, iSec_e, iKey_s, iKey_e, iValue_s, iValue_e;
	int iValueLen = 0, iKeyLen = 0, iFileNameLen = 0, iSectionLen = 0;
	int iFileSize = 0;
	int iRetCode = 0;
	int len = 0;
	int file_fd = 0;
	int wr_len = 0;

	//check parameters
	if (NULL == pszSection || NULL == pszKey || NULL == pszFile || NULL == pszValue)
	{
		return sg_iErrIndex = INIFILE_ERROR_PARA;
	}

	memset(psBuf,0x00,sizeof(psBuf));
	memset(psW_buf,0x00,sizeof(psW_buf));
// 	iValueLen    = strnlen(pszValue, sg_iMaxFileSize);	// value = "" is ok
// 	iKeyLen      = strnlen(pszKey, 256);
// 	iFileNameLen = strnlen(pszFile, 2048);
// 	iSectionLen  = strnlen(pszSection, 256);
	
	iValueLen    = strlen(pszValue);	// value = "" is ok
	iKeyLen      = strlen(pszKey);
	iFileNameLen = strlen(pszFile);
	iSectionLen  = strlen(pszSection);

	if (0 == iSectionLen || 0 == iKeyLen || 0 == iFileNameLen)
	{
		return sg_iErrIndex = INIFILE_ERROR_PARA;
	}

	iRetCode = load_ini_file(pszFile, psBuf, &iFileSize);
	if(iRetCode == 1)	// file open failed
	{
		iSec_s = -1;
	}
	else if (iRetCode == 2)
	{
		return sg_iErrIndex = INIFILE_ERROR_FILESIZE;		// file size too big
	}
	else
	{
		parse_file(pszSection, pszKey, psBuf, &iSec_s, &iSec_e, &iKey_s, &iKey_e, &iValue_s, &iValue_e);
	}

	if( -1 == iSec_s)
	{
		if(0 == iFileSize)
		{
			if (iKeyLen + iSectionLen + iValueLen + 9 >= sg_iMaxFileSize)
			{
				return sg_iErrIndex = INIFILE_ERROR_FILESIZE;		// file size too big
			}

			sprintf(psW_buf, "[%s]\n%s=%s\n", pszSection, pszKey, pszValue);   // --- use snprintf to safe printf
//			snprintf(psW_buf, sg_iMaxFileSize, "[%s]\n%s=%s\n", pszSection, pszKey, pszValue);

		}
		else
		{
			if (iFileSize + iValueLen + iSectionLen + iKeyLen + 9 >= sg_iMaxFileSize)
			{
				return sg_iErrIndex = INIFILE_ERROR_FILESIZE;		// file size too big
			}

			//not find the section, then add the new section at end of the file
			memcpy(psW_buf, psBuf, iFileSize);
			sprintf(psW_buf + iFileSize, "\n[%s]\n%s=%s\n", pszSection, pszKey, pszValue);
//			snprintf(psW_buf + iFileSize, sg_iMaxFileSize - iFileSize, "\n[%s]\n%s=%s\n", pszSection, pszKey, pszValue);
		}
	}
	else if(-1 == iKey_s)
	{
		if (iFileSize + iValueLen + iKeyLen + 3 >= sg_iMaxFileSize)
		{
			return sg_iErrIndex = INIFILE_ERROR_FILESIZE;		// file size too big
		}

		//not find the key, then add the new key=value at end of the section
		memcpy(psW_buf, psBuf, iSec_e);
		sprintf(psW_buf + iSec_e, "%s=%s\n", pszKey, pszValue);
//		snprintf(psW_buf + iSec_e, sg_iMaxFileSize - iSec_e, "%s=%s\n", pszKey, pszValue);
		sprintf(psW_buf + iSec_e + iKeyLen + iValueLen + 2, psBuf + iSec_e, iFileSize - iSec_e);
//		snprintf(psW_buf + iSec_e + iKeyLen + iValueLen + 2, sg_iMaxFileSize- iSec_e - iKeyLen - iValueLen - 2 ,
//				psBuf+iSec_e, iFileSize - iSec_e);
	}
	else
	{
		if (iFileSize - iValue_e + iValue_s + iValueLen >= sg_iMaxFileSize)
		{
			return sg_iErrIndex = INIFILE_ERROR_FILESIZE;		// file size too big
		}

		//update value with new value
		memcpy(psW_buf, psBuf, iValue_s);
		memcpy(psW_buf+ iValue_s, pszValue, iValueLen);
		memcpy(psW_buf+ iValue_s + iValueLen, psBuf + iValue_e, iFileSize - iValue_e);
		psW_buf[iFileSize - iValue_e + iValue_s + iValueLen] = 0x00;
		
	}
	
	file_fd = ex_open((char *)pszFile,O_RDWR,"\xFF\x04");
	if(file_fd < 0)
	{
		return sg_iErrIndex = INIFILE_ERROR_OPENFILE;
	}
	
	wr_len = strlen(psW_buf);

	len = write(file_fd, psW_buf,wr_len);
	if(len <= 0)
	{
		close(file_fd);
		return sg_iErrIndex = INIFILE_WRITE_ERROR;
	}

	close(file_fd);
	return sg_iErrIndex = INIFILE_SUCCESSED;
}

int IniWriteString2(const char *pszSection, const char *pszKey, const char *pszValue, const char *pszFile)
{
	char psBuf[MAX_FILE_BUFFER];
	char psW_buf[MAX_FILE_BUFFER];
	int iSec_s, iSec_e, iKey_s, iKey_e, iValue_s, iValue_e;
	int iValueLen = 0, iKeyLen = 0, iFileNameLen = 0, iSectionLen = 0;
	int iFileSize = 0;
	int iRetCode = 0;
	int len = 0;
	int file_fd = 0;
	int wr_len = 0;

	//check parameters
	if (NULL == pszSection || NULL == pszKey || NULL == pszFile || NULL == pszValue)
	{
		return sg_iErrIndex = INIFILE_ERROR_PARA;
	}

	memset(psBuf,0x00,sizeof(psBuf));
	memset(psW_buf,0x00,sizeof(psW_buf));
// 	iValueLen    = strnlen(pszValue, sg_iMaxFileSize);	// value = "" is ok
// 	iKeyLen      = strnlen(pszKey, 256);
// 	iFileNameLen = strnlen(pszFile, 2048);
// 	iSectionLen  = strnlen(pszSection, 256);
	
	iValueLen    = strlen(pszValue);	// value = "" is ok
	iKeyLen      = strlen(pszKey);
	iFileNameLen = strlen(pszFile);
	iSectionLen  = strlen(pszSection);

	if (0 == iSectionLen || 0 == iKeyLen || 0 == iFileNameLen)
	{
		return sg_iErrIndex = INIFILE_ERROR_PARA;
	}

	iRetCode = load_ini_file2(pszFile, psBuf, &iFileSize);
	if(iRetCode == 1)	// file open failed
	{
		iSec_s = -1;
	}
	else if (iRetCode == 2)
	{
		return sg_iErrIndex = INIFILE_ERROR_FILESIZE;		// file size too big
	}
	else
	{
		parse_file(pszSection, pszKey, psBuf, &iSec_s, &iSec_e, &iKey_s, &iKey_e, &iValue_s, &iValue_e);
	}

	if( -1 == iSec_s)
	{
		if(0 == iFileSize)
		{
			if (iKeyLen + iSectionLen + iValueLen + 9 >= sg_iMaxFileSize)
			{
				return sg_iErrIndex = INIFILE_ERROR_FILESIZE;		// file size too big
			}

			sprintf(psW_buf, "[%s]\n%s=%s\n", pszSection, pszKey, pszValue);   // --- use snprintf to safe printf
//			snprintf(psW_buf, sg_iMaxFileSize, "[%s]\n%s=%s\n", pszSection, pszKey, pszValue);

		}
		else
		{
			if (iFileSize + iValueLen + iSectionLen + iKeyLen + 9 >= sg_iMaxFileSize)
			{
				return sg_iErrIndex = INIFILE_ERROR_FILESIZE;		// file size too big
			}

			//not find the section, then add the new section at end of the file
			memcpy(psW_buf, psBuf, iFileSize);
			sprintf(psW_buf + iFileSize, "\n[%s]\n%s=%s\n", pszSection, pszKey, pszValue);
//			snprintf(psW_buf + iFileSize, sg_iMaxFileSize - iFileSize, "\n[%s]\n%s=%s\n", pszSection, pszKey, pszValue);
		}
	}
	else if(-1 == iKey_s)
	{
		if (iFileSize + iValueLen + iKeyLen + 3 >= sg_iMaxFileSize)
		{
			return sg_iErrIndex = INIFILE_ERROR_FILESIZE;		// file size too big
		}

		//not find the key, then add the new key=value at end of the section
		memcpy(psW_buf, psBuf, iSec_e);
		sprintf(psW_buf + iSec_e, "%s=%s\n", pszKey, pszValue);
//		snprintf(psW_buf + iSec_e, sg_iMaxFileSize - iSec_e, "%s=%s\n", pszKey, pszValue);
		sprintf(psW_buf + iSec_e + iKeyLen + iValueLen + 2, psBuf + iSec_e, iFileSize - iSec_e);
//		snprintf(psW_buf + iSec_e + iKeyLen + iValueLen + 2, sg_iMaxFileSize- iSec_e - iKeyLen - iValueLen - 2 ,
//				psBuf+iSec_e, iFileSize - iSec_e);
	}
	else
	{
		if (iFileSize - iValue_e + iValue_s + iValueLen >= sg_iMaxFileSize)
		{
			return sg_iErrIndex = INIFILE_ERROR_FILESIZE;		// file size too big
		}

		//update value with new value
		memcpy(psW_buf, psBuf, iValue_s);
		memcpy(psW_buf+ iValue_s, pszValue, iValueLen);
		memcpy(psW_buf+ iValue_s + iValueLen, psBuf + iValue_e, iFileSize - iValue_e);
		psW_buf[iFileSize - iValue_e + iValue_s + iValueLen] = 0x00;
		
	}

	file_fd = open((char *)pszFile,O_RDWR);
	if(file_fd < 0)
	{
		return sg_iErrIndex = INIFILE_ERROR_OPENFILE;
	}
	
	wr_len = strlen(psW_buf);

	len = write(file_fd, psW_buf,wr_len);
	if(len <= 0)
	{
		close(file_fd);
		return sg_iErrIndex = INIFILE_WRITE_ERROR;
	}

	close(file_fd);
	return sg_iErrIndex = INIFILE_SUCCESSED;
}

int LoadLangString(const char *pszSection, char pszKey[][24], char pszValue[][24], int iSize, char pszDefaultValue[][24], const char *pszFile)
{
	//char* psBuf;
	int iFileSize=0;
	int iSec_s, iSec_e, iKey_s, iKey_e, iValue_s, iValue_e;
	int iRetCode;
	int iCount;
	char psBuf[MAX_FILE_BUFFER];
	int i = 0;
	
	//check parameters
	if (NULL == pszSection || NULL == pszKey || NULL == pszValue || NULL == pszFile || iSize <= 0)
	{
		return INIFILE_ERROR_PARA;
	}

	if (0 == strlen(pszFile)  || 0 == strlen(pszSection))
	{
		return INIFILE_ERROR_PARA;
	}
	memset(psBuf,0x00,sizeof(psBuf));
	iRetCode = load_ini_file(pszFile, psBuf, &iFileSize);
	if(iRetCode != 0)
	{
		return sg_iErrIndex = INIFILE_ERROR_OPENFILE;
	}

	while(strcmp("END_DISPLAY_INDEX",pszKey[i]))
	{
		if(parse_file(pszSection, pszKey[i], psBuf, &iSec_s, &iSec_e, &iKey_s, &iKey_e, &iValue_s, &iValue_e))
		{
			if(NULL != pszDefaultValue[i])
			{
				strncpy(pszValue[i], pszDefaultValue[i], iSize);
			}
		}
		else
		{
			iCount = iValue_e - iValue_s;
			if (iCount == 0) // uiCount=0，not value, fill with default value
			{
				if (NULL != pszDefaultValue[i])
				{
					strncpy(pszValue[i], pszDefaultValue[i], iSize);
				}
			}

			if( iSize - 1 < iCount)
			{
				iCount =  iSize - 1;
			}

			memset(pszValue[i], 0x00, iSize);
			memcpy(pszValue[i], psBuf + iValue_s, iCount );
			pszValue[i][iCount] = '\0';
		}
		i++;
	}
	return sg_iErrIndex = INIFILE_SUCCESSED;
}

//#ifdef LEFT_BRACE
//#undef LEFT_BRACE
//#endif

//#ifdef RIGHT_BRACE
//#undef RIGHT_BRACE
//#endif

#ifdef __cplusplus
}; 
#endif









