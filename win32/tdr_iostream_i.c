/**
*
* @file     tdr_iostream_i.c  
* @brief    TSF4G-IO输出输入模块
* 
* @author jackyai  
* @version 1.0
* @date 2007-04-23 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "tdr_os.h"
#include "tdr_iostream_i.h"
#include "tdr/tdr_error.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

int tdr_iostream_write(LPTDRIOSTREAM a_pstIOStream, const char *a_pszFormat, ...)
{
	va_list ap;
	int iTempLen;
	int iRet = TDR_SUCCESS;

	assert(NULL != a_pstIOStream);
	assert(NULL != a_pszFormat);

	

	if ((TDR_IOSTREAM_STRBUF != a_pstIOStream->emIOStreamType) && 
		(TDR_IOSTREAM_FILE != a_pstIOStream->emIOStreamType))
	{
		return TDR_ERRIMPLE_UNSPORTED_IOSTREAM;
	}

	va_start(ap, a_pszFormat);
	if (TDR_IOSTREAM_STRBUF == a_pstIOStream->emIOStreamType)
	{
		assert(NULL != a_pstIOStream->stParam.stBuffIOParam.pszBuff);
		assert(0 < a_pstIOStream->stParam.stBuffIOParam.iBuffLen);
		
		iTempLen = tdr_vsnprintf(a_pstIOStream->stParam.stBuffIOParam.pszBuff, a_pstIOStream->stParam.stBuffIOParam.iBuffLen,
			a_pszFormat, ap);
		if ((0 > iTempLen) || (iTempLen >= a_pstIOStream->stParam.stBuffIOParam.iBuffLen)) 
		{   
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_SPACE_TO_WRITE); 
		}else
		{
			a_pstIOStream->stParam.stBuffIOParam.pszBuff += iTempLen;
			a_pstIOStream->stParam.stBuffIOParam.iBuffLen -= iTempLen;
		}
	}
	
	if (TDR_IOSTREAM_FILE == a_pstIOStream->emIOStreamType)
	{
		assert(NULL != a_pstIOStream->stParam.stFileIOParam.a_fp);
		iTempLen = vfprintf(a_pstIOStream->stParam.stFileIOParam.a_fp, a_pszFormat, ap);
		if (0 > iTempLen)
		{
			iRet = TDR_ERRIMPLE_FAILED_TO_WRITE_FILE;
		}
	}

	va_end(ap);
	
	return iRet;
}
