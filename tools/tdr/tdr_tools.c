

/**
*
* @file     tdr_tools.c 
* @brief    一些辅助工具
* 
* @author jackyai  
* @version 1.0
* @date 2007-06-01 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <stdio.h>
#include <string.h>
#include "tdr_tools.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

char *tdr_parse_version_string(unsigned int unVer)
{
	static char szVersion[64];
	char szVer[32];
	sprintf(szVer,"%08d",unVer);

	sprintf(szVersion," %c.%c ",szVer[0],szVer[1]);
	if (szVer[2] == '0')
	{
		strcat(szVersion,"Alpha");
	}
	else if (szVer[2] == '1')
	{
		strcat(szVersion,"Beta");
	}	
	else
	{
		strcat(szVersion,"Release");
	}
	sprintf(szVersion+strlen(szVersion),"%c%c Build%c%c%c",szVer[3],szVer[4],szVer[5],szVer[6],szVer[7]);


	return &szVersion[0];
}

char *tdr_basename(const char *a_pszObj)
{
	char *pch;

	pch = strrchr(a_pszObj, TDR_OS_DIRSEP);
	if (NULL == pch)
	{
		return (char *)a_pszObj;
	}
	pch ++;

	return pch;
}


