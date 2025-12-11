/**
*
* @file     tlogerr.c 
* @brief    tlog错误处理函数
* 
* @author kent
* @version 1.0
* @date 2007-11-28 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include "tdr/tdr_error.h"
#include "tlog/tlogerr.h"


static const char *s_errmsg[] = {
	"没有错误",
	"日志文件打开出错",
	"打开网络连接失败",
	"forward 关系有循环",	
};

const char * tlog_error_string(int iErrorCode)
{
	const char *pchStr = NULL;

	if (!TERR_IS_ERROR(iErrorCode))
	{
		return "没有错误";
	}
	
	switch(TERR_GET_MOD(iErrorCode))
	{
	case TMODID_DR:
		pchStr = "tdr mod err";
		break;
	case TMODID_LOG:
		{
			int iErrno = TERR_GET_ERRNO(iErrorCode);
			iErrno -= TLOG_ERR_START_ERRNO;
			if (iErrno < 0 || iErrno >= TLOG_ERROR_COUNT)
			{
				pchStr =  "未知错误";
			}
			else
			{
				pchStr = s_errmsg[iErrno];
			}
		};
		break;
	default:
		pchStr =  "未知错误";
	}
	
	return pchStr;
}

