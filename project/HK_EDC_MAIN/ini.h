/*
 ============================================================================
 Name        : ini.h
 Author      : Verne
 Version     : 
 Copyright   : PAX Computer Technology(Shenzhen) CO., LTD
 Description : PAX POS Shared Library
 Memo        : 1. case sensitive
               2. 2 same key in 1 section, read the 1st one
			   3. not ignore 0x20(space)
			   4. error message only support English
			   5. key, section, max 256 bytes, file path and name max 2048
			   6. the comment char is fixed as double ';'
 ============================================================================
 */
 
#ifndef SHARED_LIBRARY_H_INI
#define SHARED_LIBRARY_H_INI


enum{
	INIFILE_SUCCESSED = 0,				// successes return
	INIFILE_ERROR_INDEX = -100,	// INI Module error code begin
	INIFILE_ERROR_PARA,					// in para illegal
	INIFILE_ERROR_OPENFILE,				// fopen failed
	INIFILE_ERROR_FILESIZE,				// file size too big, resize it in IniSetMaxFileSize
	INIFILE_ERROR_KEY_NOTFOUND,			// key missing
	INIFILE_ERROR_VALUE_NOTFOUND,		// value missing
	INIFILE_WRITE_ERROR,				// fput failed
	INIFILE_ERROR_MALLOC,				// malloc failed
	INIFILE_ERROR_UNKNOW				// unknown error, end of INI module error code
};

#ifdef __cplusplus
extern "C"
{
#endif

/*
* @brief read string in initialization file retrieves a string from the specified section in an initialization file
*
* @param pszSection [in] name of the section containing the key name
* @param pszKey [in] name of the key pairs to value
* @param pszValue [out] pointer to the buffer that receives the retrieved string
* @param iSize [in] size of result's buffer
* @param pszDefaultValue [in] default value of result
* @param pszFile [in] path of the initialization file
*
* @return 0 : read success;
* @return INIFILE_ERROR_PARA : para error
* @return INIFILE_ERROR_OPENFILE :       open ini file failed, then value=default_value
* @return INIFILE_ERROR_FILESIZE :       ini file size is too big, then value=default_value
* @return INIFILE_ERROR_KEY_NOTFOUND :   key is not found, then value=default_value
* @return INIFILE_ERROR_VALUE_NOTFOUND : found key but not value, then value=default_value
*/
int IniReadString(const char *pszSection, const char *pszKey,char *pszValue, int iSize,const char *pszDefaultValue, const char *pszFile);//ex_open
int IniReadString2(const char *pszSection, const char *pszKey,char *pszValue, int iSize,const char *pszDefaultValue, const char *pszFile);//open
/*
 * @brief write a profile string to a ini file
 *
 * @param pszSection [in] name of the section,can't be NULL and empty string
 * @param pszKey [in] name of the key pairs to value, can't be NULL and empty string
 * @param pszValue [out] profile string value
 * @param pszFile [in] path of ini file
 *
 * @return 0 : write success
 * @return INIFILE_ERROR_FILESIZE : write file error, file size too big
 * @return INIFILE_ERROR_OPENFILE : open file failed
 * @return INIFILE_WRITE_ERROR : write file failed;
 */
int IniWriteString(const char *pszSection, const char *pszKey,const char *pszValue, const char *pszFile);//ex_open
int IniWriteString2(const char *pszSection, const char *pszKey,const char *pszValue, const char *pszFile);//open
/*
 * @brief read int value in initialization file retrieves int value from the specified section in an initialization file
 *
 * @param pszSection [in] name of the section containing the key name
 * @param pszKey [in] name of the key pairs to value
 * @param iDefaultValue [in] default value of result
 * @param pszFile [in] path of the initialization file
 *
 * @return profile int value,if read fail, return default value
*/
int IniReadInt(const char *pszSection, const char *pszKey,int iDefaultValue, const char *pszFile);

/*
 * @brief set the max file size, default is 16*1024
 * @param iFileSize [in] define max file size
 * @return none
*/
void IniSetMaxFileSize(int iFileSize);

/*
 * @brief return the error message for display
 * @param none
 * @return the error message
*/
const char* IniGetLastErrMsg();

/*
 * @brief return the module version string like "1.0.0.0"
 * @param none
 * @return the version NO. string
*/
const char* IniGetVer();

int LoadLangString(const char *pszSection, char pszKey[][24], char pszValue[][24], int iSize, char pszDefaultValue[][24], const char *pszFile);	

#ifdef __cplusplus
};
#endif

#endif /* SHARED_LIBRARY_H_INI */







