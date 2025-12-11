/**
*
* @file     tdr_ctypes_info.c 
* @brief    DR支持的C/C++基本数据类型信息
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-03-26 
*
* 定义DR支持的c/c++各内置数据类型的详细信息，并提供访问方法
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/



#include <assert.h>
#include <string.h>
#include <time.h>

#include "tdr_os.h"
#include "tdr/tdr_error.h"
#include "tdr/tdr_XMLtags.h"
#include "tdr/tdr_define.h"
#include "tdr_ctypes_info_i.h"
#include "tdr_define_i.h"
#include "tdr/tdr_ctypes_info.h"
#include "tdr/tdr_define.h"
#include "tdr_os.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

/*typde xml  tag set vsersion 0 */
static TDRCTYPEINFO	s_types_XMLTag[] = 
{
    /* typename, 		cname,				SQLName,		sprefix, mPrefix,	type,			size,	flags. */
    { TDR_TAG_UNION,	"union",			"",				"st",	 "ast",		TDR_TYPE_UNION,		0,	0}, 
    { TDR_TAG_STRUCT,	"struct",			"",				"st",	 "ast",		TDR_TYPE_STRUCT,		0,	0},
    { TDR_TAG_TINYINT,		"char",				"TINYINT",		"ch",	 "sz",		TDR_TYPE_CHAR,		TDR_SIZEOF_BYTE,	0},
    { TDR_TAG_TINYUINT,	"unsigned char",	"TINYINT UNSIGNED",		"b",	 "sz",		TDR_TYPE_UCHAR,		TDR_SIZEOF_BYTE,	0},
    { TDR_TAG_SMALLINT,	"short",			"SMALLINT",			"n",	 "",		TDR_TYPE_SMALLINT,		TDR_SIZEOF_SHORT,	TDR_FLAG_NETORDER},
    { TDR_TAG_SMALLUINT,	"unsigned short",	"SMALLINT UNSIGNED",	"w",	 "",		TDR_TYPE_SMALLUINT,		TDR_SIZEOF_SHORT,	TDR_FLAG_NETORDER},
    { TDR_TAG_INT,		"int",				"INT",			"i",	 "",		TDR_TYPE_INT,		TDR_SIZEOF_INT,	TDR_FLAG_NETORDER},
    { TDR_TAG_UINT,		"unsigned int",		"INT UNSIGNED",			"dw",	 "",		TDR_TYPE_UINT,		TDR_SIZEOF_INT,	TDR_FLAG_NETORDER},
    { TDR_TAG_BIGINT,	"tdr_longlong",			"BIGINT",		"ll",	 "",		TDR_TYPE_LONGLONG,	TDR_SIZEOF_LONGLONG,	TDR_FLAG_NETORDER},
    { TDR_TAG_BIGUINT,	"tdr_ulonglong",	"BIGINT UNSIGNED",		"ull",	 "",		TDR_TYPE_ULONGLONG,	TDR_SIZEOF_LONGLONG,	TDR_FLAG_NETORDER},
    { TDR_TAG_FLOAT,	"float",			"FLOAT",		"f",	 "",		TDR_TYPE_FLOAT,		TDR_SIZEOF_INT,	TDR_FLAG_NETORDER},
    { TDR_TAG_DOUBLE,	"double",			"DOUBLE",		"d",	 "",		TDR_TYPE_DOUBLE,		TDR_SIZEOF_LONGLONG,	TDR_FLAG_NETORDER},
    { TDR_TAG_DECIMAL,	"float",			"FLOAT",		"f",	 "",		TDR_TYPE_FLOAT,		TDR_SIZEOF_INT,	TDR_FLAG_NETORDER},
    { TDR_TAG_DATE,		"tdr_date_t",				"DATE",			"t",	 "",		TDR_TYPE_DATE,		TDR_SIZEOF_INT,	TDR_FLAG_NETORDER},
    { TDR_TAG_TIME,		"tdr_time_t",				"TIME",			"t",	 "",		TDR_TYPE_TIME,		TDR_SIZEOF_INT,	TDR_FLAG_NETORDER},
    { TDR_TAG_DATETIME,	"tdr_datetime_t",				"DATETIME",		"t",	 "",		TDR_TYPE_DATETIME,	TDR_SIZEOF_LONGLONG,	TDR_FLAG_NETORDER},
    { TDR_TAG_STRING,	"char",				"VARCHAR",		"sz",	 "asz",		TDR_TYPE_STRING,		1,	0},
	{ TDR_TAG_BYTE,		"unsigned char",	"TINYINT UNSIGNED",		"b",	 "sz",		TDR_TYPE_UCHAR,		1,	0},
	{ TDR_TAG_IP,	"tdr_ip_t",				"INT UNSIGNED",		"ul",	 "",		TDR_TYPE_IP,	TDR_SIZEOF_INT,	TDR_FLAG_NETORDER},
	{ TDR_TAG_WCHAR,	"tdr_wchar_t",	"SMALLINT UNSIGNED",		"w",	 "",		TDR_TYPE_WCHAR,	TDR_SIZEOF_SHORT,	TDR_FLAG_NETORDER},
	{ TDR_TAG_WSTRING,	"tdr_wchar_t",	"VARCHAR",		"sz",	 "asz",		TDR_TYPE_WSTRING,	2,	0},
	{ TDR_TAG_VOID,	"void",	"VOID",		"",	 "",		TDR_TYPE_VOID,	1,	0},


    /*typde xml  tag set vsersion 0 */
    { TDR_TAG_CHAR,		"char",				"TINYINT",		"ch",	 "sz",		TDR_TYPE_CHAR,		1,	0},
    { TDR_TAG_UCHAR,	"unsigned char",	"TINYINT UNSIGNED",		"b",	 "sz",		TDR_TYPE_UCHAR,		1,	0},
    { TDR_TAG_SHORT,	"short",			"SMALLINT",		"n",	 "",		TDR_TYPE_SMALLINT,		TDR_SIZEOF_SHORT,	TDR_FLAG_NETORDER},
    { TDR_TAG_USHORT,	"unsigned short",	"SMALLINT UNSIGNED",	"w",	 "",		TDR_TYPE_SMALLUINT,		TDR_SIZEOF_SHORT,	TDR_FLAG_NETORDER},
    { TDR_TAG_LONG,	    "long",			"INT",		"l",	 "",		TDR_TYPE_LONG,	TDR_SIZEOF_INT,	TDR_FLAG_NETORDER},
    { TDR_TAG_ULONG,	"unsigned long",	"INT UNSIGNED",		"ul",	 "",		TDR_TYPE_ULONG,TDR_SIZEOF_INT,	TDR_FLAG_NETORDER},
    { TDR_TAG_LONGLONG,	"tdr_longlong",			"BIGINT",		"ll",	 "",		TDR_TYPE_LONGLONG,	TDR_SIZEOF_LONGLONG,	TDR_FLAG_NETORDER},
    { TDR_TAG_ULONGLONG,	"tdr_ulonglong",	"BIGINT UNSIGNED",		"ull",	 "",		TDR_TYPE_ULONGLONG,	TDR_SIZEOF_LONGLONG,	TDR_FLAG_NETORDER},
    { TDR_TAG_MONEY,	"int",		        "INT",          "m",	 "",		TDR_TYPE_MONEY,	TDR_SIZEOF_INT,	TDR_FLAG_NETORDER},
  
};

static TDRBOOLEAN tdr_isValid_tdrdate_i(LPTDRDATE pstDate);

static TDRBOOLEAN tdr_isValid_tdrtime_i(LPTDRTIME pstTime);

static TDRBOOLEAN tdr_isValid_tdrdatetime_i(LPTDRDATETIME pstDateTime);



int tdr_typename_to_idx(IN const char *name)
{
    int i = TDR_INVALID_INDEX;
    LPTDRCTYPEINFO pstCtypeInfo = NULL;

    assert(name != NULL);
    

    pstCtypeInfo = &s_types_XMLTag[0];
    
    for (i = 0; i < (int)(sizeof(s_types_XMLTag)/sizeof(TDRCTYPEINFO)); i++)
    {
        if (0 == tdr_stricmp(pstCtypeInfo->pszName, name))
        {
            return i;
        }
        pstCtypeInfo++;
    }
    
    
    return TDR_INVALID_INDEX;
}

LPTDRCTYPEINFO tdr_idx_to_typeinfo(IN int iIdx)
{
    assert((0 <= iIdx) && (iIdx < (int)(sizeof(s_types_XMLTag)/sizeof(TDRCTYPEINFO))));
    
   
    return s_types_XMLTag + iIdx;
    
}


TDR_API int tdr_ineta_to_tdrip(OUT tdr_ip_t *a_piTDRIp, const char *a_pszIP)
{
	int iRet = 0;
	struct in_addr addr;

	//assert(NULL != a_piTDRIp);
	//assert(NULL != a_pszIP);
	if ((NULL == a_piTDRIp) || (NULL == a_pszIP))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

#ifdef WIN32
	addr.s_addr = inet_addr(a_pszIP);
	if (INADDR_NONE == addr.s_addr)
	{
		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_IP_VALUE);
	}
#else
	iRet = inet_aton(a_pszIP, &addr);
	if (0 != iRet)
	{
		iRet = TDR_SUCCESS;
	}else
	{
		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_IP_VALUE);
	}
#endif
	*a_piTDRIp = addr.s_addr;
	return iRet;
}

TDR_API int tdr_tdrip_to_inetaddr(OUT struct in_addr *a_pInp, IN tdr_ip_t a_iTDRIp)
{
	//assert(NULL != a_pInp);
	if (NULL == a_pInp)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

	a_pInp->s_addr = a_iTDRIp;

	return TDR_SUCCESS;
}

TDR_API char *tdr_tdrip_to_ineta(IN tdr_ip_t a_iTDRIp)
{
	 struct in_addr addr;

	 addr.s_addr = a_iTDRIp;

	 return inet_ntoa(addr);
}

TDR_API int tdr_str_to_tdrdate(OUT tdr_date_t *a_piTdrDate, IN const char *a_pszDate)
{
	char *pchStop;
	struct tm stTm;
	LPTDRDATE  pstDate;

	//assert(NULL != a_piTdrDate);
	//assert(NULL != a_pszDate);
	if ((NULL == a_piTdrDate) || (NULL == a_pszDate))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	assert(sizeof(TDRDATE) == sizeof(tdr_date_t));

	memset(&stTm, 0, sizeof(stTm));
	pstDate = (LPTDRDATE)a_piTdrDate;
	pchStop = tdr_strptime(a_pszDate, "%Y-%m-%d", &stTm);
	if (NULL == pchStop)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVAILD_DATE_VALUE);
	}
	pstDate->nYear = (short)(stTm.tm_year + 1900);
	pstDate->bMonth = (unsigned char)stTm.tm_mon + 1;
	pstDate->bDay = (unsigned char)stTm.tm_mday;
	if (TDR_FALSE == tdr_isValid_tdrdate_i(pstDate))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVAILD_DATE_VALUE);
	}

	return TDR_SUCCESS;
}

TDRBOOLEAN tdr_isValid_tdrdate_i(LPTDRDATE pstDate)
{
	assert(pstDate);

	if ((-9999 > pstDate->nYear) || (pstDate->nYear > 9999))
	{
		return TDR_FALSE;
	}

	if (12 < pstDate->bMonth)
	{
		return TDR_FALSE;
	}

	if (31 < pstDate->bDay)
	{
		return TDR_FALSE;
	}
	switch(pstDate->bDay)
	{
	case 2:
		{
			if ((pstDate->nYear % 4) == 0 && ((pstDate->nYear % 100) != 0 || 
				(pstDate->nYear % 400) == 0))
			{
				if (28 < pstDate->bDay)
				{
					return TDR_FALSE;
				}
			}else
			{
				if (29 < pstDate->bDay)
				{
					return TDR_FALSE;
				}
			}/*if ((pstDate->nYear % 4) == 0 && ((pstDate->nYear % 100) != 0 || */
			break;
		}
	case 4:
	case 6:
	case 9:
	case 11:
		{
			if (30 < pstDate->bDay)
			{
				return TDR_FALSE;
			}
			break;
		}
	default:
		break;
	}

	return TDR_TRUE;
}

TDR_API char *tdr_tdrdate_to_str(IN const tdr_date_t *a_piTdrDate)
{
	LPTDRDATE pstDate;
	static char szDate[32];

	//assert(NULL != a_piTdrDate);
	if (NULL == a_piTdrDate)
	{
		return "";
	}
	pstDate = (LPTDRDATE)a_piTdrDate;
	//assert(TDR_TRUE == tdr_isValid_tdrdate_i(pstDate));

	pstDate = (LPTDRDATE)a_piTdrDate;
	tdr_snprintf(szDate, sizeof(szDate), "%4d-%02d-%02d", pstDate->nYear, pstDate->bMonth, pstDate->bDay);

	return &szDate[0];
}


TDR_API int tdr_str_to_tdrtime(OUT tdr_time_t *a_piTdrTime, IN const char *a_pszTime)
{
	char *pchStop;
	struct tm stTm;
	LPTDRTIME  pstTime;

	//assert(NULL != a_piTdrTime);
	//assert(NULL != a_pszTime);
	if ((NULL == a_piTdrTime) || (NULL == a_pszTime))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	assert(sizeof(TDRTIME) == sizeof(tdr_time_t));

	memset(&stTm, 0, sizeof(stTm));
	pstTime = (LPTDRTIME)a_piTdrTime;
	pchStop = tdr_strptime(a_pszTime, "%H:%M:%S", &stTm);
	if (NULL == pchStop)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_TIME_VALUE);
	}
	pstTime->nHour = (short)stTm.tm_hour;
	pstTime->bMin = (unsigned char)stTm.tm_min;
	pstTime->bSec = (unsigned char)stTm.tm_sec;
	if (TDR_FALSE == tdr_isValid_tdrtime_i(pstTime))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_TIME_VALUE);
	}

	return TDR_SUCCESS;
}

TDRBOOLEAN tdr_isValid_tdrtime_i(LPTDRTIME pstTime)
{
	assert(pstTime);

	if ((-999 > pstTime->nHour) || (999 < pstTime->nHour))
	{
		return TDR_FALSE;
	}
	if ((59 < pstTime->bMin))
	{
		return TDR_FALSE;
	}

	if ((59 < pstTime->bSec))
	{
		return TDR_FALSE;
	}
	
	
	return TDR_TRUE;
}


TDR_API char *tdr_tdrtime_to_str(IN tdr_time_t *a_piTdrTime)
{
	LPTDRTIME pstTime ;
	static char szTime[32];

	//assert(NULL != a_piTdrTime);
	if (NULL == a_piTdrTime)
	{
		return "";
	}
	pstTime = (LPTDRTIME)a_piTdrTime;
	//assert(TDR_TRUE == tdr_isValid_tdrtime_i(pstTime));

	pstTime = (LPTDRTIME)a_piTdrTime;
	
	tdr_snprintf(szTime, sizeof(szTime), "%3d:%02d:%02d", pstTime->nHour, pstTime->bMin, pstTime->bSec);


	return &szTime[0];
}

TDR_API int tdr_str_to_tdrdatetime(OUT tdr_datetime_t *a_piTdrDateTime, IN const char *a_pszDateTime)
{
	char *pchStop;
	struct tm stTm;
	LPTDRDATETIME  pstDateTime;

	//assert(NULL != a_piTdrDateTime);
	//assert(NULL != a_pszDateTime);
	if ((NULL == a_piTdrDateTime) || (NULL == a_pszDateTime))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	assert(sizeof(TDRDATETIME) == sizeof(tdr_datetime_t));

	memset(&stTm, 0, sizeof(stTm));
	pstDateTime = (LPTDRDATETIME)a_piTdrDateTime;

	pchStop = tdr_strptime(a_pszDateTime, "%Y-%m-%d %H:%M:%S", &stTm);
	if (pchStop == NULL)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_DATETIME_VALUE);
	}

	pstDateTime->stDate.nYear = (short)(stTm.tm_year + 1900);
	pstDateTime->stDate.bMonth = (unsigned char)stTm.tm_mon + 1;
	pstDateTime->stDate.bDay = (unsigned char)stTm.tm_mday;
	pstDateTime->stTime.nHour = (short)stTm.tm_hour;
	pstDateTime->stTime.bMin = (unsigned char)stTm.tm_min;
	pstDateTime->stTime.bSec = (unsigned char)stTm.tm_sec;
	if (TDR_FALSE == tdr_isValid_tdrdatetime_i(pstDateTime))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_DATETIME_VALUE);
	}

	return TDR_SUCCESS;
}

TDR_API int tdr_utctime_to_tdrdatetime(OUT tdr_datetime_t *a_piTdrDateTime, IN time_t a_tTimer)
{
	struct tm *pstTm;
	LPTDRDATETIME  pstDateTime;
	
	//assert(NULL != a_piTdrDateTime);
	if (NULL == a_piTdrDateTime)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	pstDateTime = (LPTDRDATETIME)a_piTdrDateTime;

	pstTm = gmtime(&a_tTimer);
	if (NULL == pstTm)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_DATETIME_VALUE);
	}

	pstDateTime->stDate.nYear = (short)(pstTm->tm_year + 1900);
	pstDateTime->stDate.bMonth = (unsigned char)pstTm->tm_mon + 1;
	pstDateTime->stDate.bDay = (unsigned char)pstTm->tm_mday;
	pstDateTime->stTime.nHour = (short)pstTm->tm_hour;
	pstDateTime->stTime.bMin = (unsigned char)pstTm->tm_min;
	pstDateTime->stTime.bSec = (unsigned char)pstTm->tm_sec;
	if (TDR_FALSE == tdr_isValid_tdrdatetime_i(pstDateTime))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_DATETIME_VALUE);
	}

	return TDR_SUCCESS;
}


TDR_API char *tdr_tdrdatetime_to_str(IN const tdr_datetime_t *a_piTdrDateTime)
{
	LPTDRDATETIME pstDateTime ;
	static char szDateTime[32];

	//assert(NULL != a_piTdrDateTime);
	if (NULL == a_piTdrDateTime)
	{
		return "";
	}
	pstDateTime = (LPTDRDATETIME)a_piTdrDateTime;
	//assert(TDR_TRUE == tdr_isValid_tdrdatetime_i(pstDateTime));

	pstDateTime = (LPTDRDATETIME)a_piTdrDateTime;
	
	tdr_snprintf(szDateTime, sizeof(szDateTime), "%4d-%02d-%02d %02d:%02d:%02d", pstDateTime->stDate.nYear, pstDateTime->stDate.bMonth,
		pstDateTime->stDate.bDay, pstDateTime->stTime.nHour, pstDateTime->stTime.bMin, pstDateTime->stTime.bSec);


	return &szDateTime[0];
}


TDRBOOLEAN tdr_isValid_tdrdatetime_i(LPTDRDATETIME pstDateTime)
{
	TDRBOOLEAN bValid;

	assert(pstDateTime);


	bValid = tdr_isValid_tdrdate_i(&pstDateTime->stDate);
	if (TDR_FALSE == bValid)
	{
		return bValid;
	}

	if ((23 < pstDateTime->stTime.nHour))
	{
		return TDR_FALSE;
	}
	if ( (59 < pstDateTime->stTime.bMin))
	{
		return TDR_FALSE;
	}

	if ((59 < pstDateTime->stTime.bSec))
	{
		return TDR_FALSE;
	}


	return TDR_TRUE;
}

TDR_API int tdr_compare_datetime(IN const tdr_datetime_t *a_piTdrDateTime1, IN const tdr_datetime_t *a_piTdrDateTime2)
{
	int iRet;

	LPTDRDATETIME pnValue1 = (LPTDRDATETIME )a_piTdrDateTime1;
	LPTDRDATETIME pnValue2 = (LPTDRDATETIME )a_piTdrDateTime2;

	if ((NULL == a_piTdrDateTime1) || (NULL == a_piTdrDateTime2))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	iRet = tdr_compare_date((tdr_date_t *)&pnValue1->stDate, (tdr_date_t *)&pnValue2->stDate);
	if (0 != iRet)
	{
		return iRet;
	}

	iRet = tdr_compare_time((tdr_time_t *)&pnValue1->stTime, (tdr_time_t *)&pnValue2->stTime);

	return iRet;
}

TDR_API int tdr_compare_date(IN const tdr_date_t *a_piTdrDate1, IN const tdr_date_t *a_piTdrDate2)
{
	LPTDRDATE pnValue1 = (LPTDRDATE )a_piTdrDate1;
	LPTDRDATE pnValue2 = (LPTDRDATE )a_piTdrDate2;

	if ((NULL == a_piTdrDate1) || (NULL == a_piTdrDate2))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if (pnValue1->nYear > pnValue2->nYear)
	{
		return 1;
	}else if (pnValue1->nYear < pnValue2->nYear)
	{
		return -1;
	}

	if (pnValue1->bMonth > pnValue2->bMonth)
	{
		return 1;
	}else if (pnValue1->bMonth < pnValue2->bMonth)
	{
		return -1;
	}

	if (pnValue1->bDay > pnValue2->bDay)
	{
		return 1;
	}else if (pnValue1->bDay < pnValue2->bDay)
	{
		return -1;
	}

	return 0;
}

TDR_API int tdr_compare_time(IN const tdr_time_t *a_piTdrTime1, IN const tdr_time_t *a_piTdrTime2)
{
	LPTDRTIME pnValue1 = (LPTDRTIME )a_piTdrTime1;
	LPTDRTIME pnValue2 = (LPTDRTIME )a_piTdrTime2;

	if ((NULL == a_piTdrTime1) || (NULL == a_piTdrTime2))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if (pnValue1->nHour > pnValue2->nHour)
	{
		return 1;
	}else if (pnValue1->nHour < pnValue2->nHour)
	{
		return -1;
	}

	if (pnValue1->bMin > pnValue2->bMin)
	{
		return 1;
	}else if (pnValue1->bMin < pnValue2->bMin)
	{
		return -1;
	}

	if (pnValue1->bSec > pnValue2->bSec)
	{
		return 1;
	}else if (pnValue1->bSec < pnValue2->bSec)
	{
		return -1;
	}

	return 0;
}


