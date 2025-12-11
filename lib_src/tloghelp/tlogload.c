/*
**  @file $RCSfile: tlogload.c,v $
**  general description of this module
**  $Id: tlogload.c,v 1.7 2009/03/27 06:17:19 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009/03/27 06:17:19 $
**  @version $Revision: 1.7 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include "mng/tmib.h"
#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"
#include "tdr/tdr.h"

TLOGCONF* tlog_get_cfg_from_mib(int a_iMib, const char *a_pszDomain, const char *a_pszName, int a_iProcID)
{
	int iRet = 0;
	TMIBDATA stData;

	memset(&stData, 0, sizeof(stData));
	snprintf(stData.szDomain, sizeof(stData.szDomain), "%s", a_pszDomain);
	snprintf(stData.szName, sizeof(stData.szName), "%s", a_pszName);
	stData.iProcID  =   a_iProcID;

	iRet = tmib_get_data((HTMIB)a_iMib, &stData, 1);

	if( 0 == iRet && stData.iSize<(int)sizeof(TLOGCONF) )
		iRet	=	-1;
	
	return -1==iRet? NULL : (TLOGCONF*) stData.pszData;	
}


int tlog_init_cfg_from_file(TLOGCONF* a_pstConf, const char *a_pszPath)
{
	LPTDRMETALIB pstLib;
	LPTDRMETA pstMeta;
	TDRDATA stData;
	int iRet;

	pstLib	=	(LPTDRMETALIB) tlog_get_meta_data();

	pstMeta	=	tdr_get_meta_by_name(pstLib, "TLOGConf");

	if( !pstMeta )
		return -1;

	stData.pszBuff	=	(char*)a_pstConf;
	stData.iBuff	=	(int)sizeof(*a_pstConf);

	iRet = tdr_input_file(pstMeta, &stData, a_pszPath, 0, 0);
	if( iRet<0 )
	{
		printf("input tlog conf file err:%s\n", tdr_error_string(iRet));
		return iRet;
	}
	else
		return 0;
}

int tlog_init_cfg_default(TLOGCONF* a_pstConf, const char* a_pszPath)
{
	TLOGCATEGORY* pstCat;

	memset(a_pstConf, 0, sizeof(*a_pstConf));

	a_pstConf->iCount	=	5;
	a_pstConf->iPriorityHigh  =	TLOG_PRIORITY_NULL;
	a_pstConf->iPriorityLow  =  TLOG_PRIORITY_NOTSET;
	a_pstConf->iDelayInit	=	0;
	a_pstConf->iSuppressError	=	1;

	//text category
	pstCat	=	a_pstConf->astCategoryList + 0;
	STRNCPY(pstCat->szName, TLOG_DEF_CATEGORY_TEXTROOT, sizeof(pstCat->szName));
	STRNCPY(pstCat->szForwardCat, TLOG_DEF_CATEGORY_TEXTTRACE, sizeof(pstCat->szForwardCat));
	pstCat->stDevice.iType	=	TLOG_DEV_NO;

	//texttrace
	pstCat	=	a_pstConf->astCategoryList + 1;
	STRNCPY(pstCat->szName, TLOG_DEF_CATEGORY_TEXTTRACE, sizeof(pstCat->szName));
	STRNCPY(pstCat->szFormat, TLOG_DEF_LAYOUT_FORMAT, sizeof(pstCat->szFormat));
	STRNCPY(pstCat->szForwardCat, TLOG_DEF_CATEGORY_TEXTERR, sizeof(pstCat->szForwardCat));
	pstCat->iPriorityHigh  =	 TLOG_PRIORITY_NULL;
	pstCat->iPriorityLow  =  TLOG_PRIORITY_TRACE; 
	pstCat->iLevelDispatch =  0;
	pstCat->iMustSucc	=	 0;
	pstCat->iMaxMsgSize	=	 0;
	pstCat->stDevice.iType	=	TLOG_DEV_FILE;
	pstCat->stDevice.stDevice.stFile.szPattern[sizeof(pstCat->stDevice.stDevice.stFile.szPattern)-1] = 0;
	snprintf(pstCat->stDevice.stDevice.stFile.szPattern, sizeof(pstCat->stDevice.stDevice.stFile.szPattern)-1, "%s.log", a_pszPath);
	pstCat->stDevice.stDevice.stFile.iSyncTime	=	0;
	pstCat->stDevice.stDevice.stFile.iPrecision =	1;
	pstCat->stDevice.stDevice.stFile.iSizeLimit =	20*1024*1024;
	pstCat->stDevice.stDevice.stFile.iMaxRotate =	3;

	//texterr
	pstCat	=	a_pstConf->astCategoryList + 2;
	STRNCPY(pstCat->szName, TLOG_DEF_CATEGORY_TEXTERR, sizeof(pstCat->szName));
	STRNCPY(pstCat->szFormat, TLOG_DEF_LAYOUT_FORMAT, sizeof(pstCat->szFormat));
	pstCat->iPriorityHigh  =	 TLOG_PRIORITY_NULL;
	pstCat->iPriorityLow  =  TLOG_PRIORITY_ERROR; 
	pstCat->iLevelDispatch =  0;
	pstCat->iMustSucc	=	 0;
	pstCat->iMaxMsgSize	=	 0;
	pstCat->stDevice.iType	=	TLOG_DEV_FILE;
	pstCat->stDevice.stDevice.stFile.szPattern[sizeof(pstCat->stDevice.stDevice.stFile.szPattern)-1] = 0;
	snprintf(pstCat->stDevice.stDevice.stFile.szPattern, sizeof(pstCat->stDevice.stDevice.stFile.szPattern)-1, "%s.error", a_pszPath);
	pstCat->stDevice.stDevice.stFile.iSyncTime	=	0;
	pstCat->stDevice.stDevice.stFile.iPrecision =	1;
	pstCat->stDevice.stDevice.stFile.iSizeLimit =	10*1024*1024;
	pstCat->stDevice.stDevice.stFile.iMaxRotate =	2;

	//texttrace.bus
	pstCat	=	a_pstConf->astCategoryList + 3;
	STRNCPY(pstCat->szName, TLOG_DEF_CATEGORY_BUS, sizeof(pstCat->szName));
	STRNCPY(pstCat->szFormat, "[%d%u]%p %m %F:%l%n", sizeof(pstCat->szFormat));
	pstCat->iPriorityHigh  =	 TLOG_PRIORITY_NULL;
	pstCat->iPriorityLow  =  TLOG_PRIORITY_DEBUG; 
	pstCat->iLevelDispatch =  1;
	pstCat->iMustSucc	=	 0;
	pstCat->iMaxMsgSize	=	 0;
	pstCat->stDevice.iType	=	TLOG_DEV_FILE;
	pstCat->stDevice.stDevice.stFile.szPattern[sizeof(pstCat->stDevice.stDevice.stFile.szPattern)-1] = 0;
	snprintf(pstCat->stDevice.stDevice.stFile.szPattern, sizeof(pstCat->stDevice.stDevice.stFile.szPattern)-1, "%s_tbus.log", a_pszPath);
	pstCat->stDevice.stDevice.stFile.iSyncTime	=	0;
	pstCat->stDevice.stDevice.stFile.iPrecision =	1;
	pstCat->stDevice.stDevice.stFile.iSizeLimit =	5*1024*1024;
	pstCat->stDevice.stDevice.stFile.iMaxRotate =	3;

	//data
	pstCat	=	a_pstConf->astCategoryList + 4;
	STRNCPY(pstCat->szName, TLOG_DEF_CATEGORY_DATAROOT, sizeof(pstCat->szName));
	STRNCPY(pstCat->szFormat, TLOG_DEF_LAYOUT_FORMAT, sizeof(pstCat->szFormat));
	pstCat->iPriorityHigh  =	TLOG_PRIORITY_NULL;
	pstCat->iPriorityLow =  TLOG_PRIORITY_TRACE;
	pstCat->iLevelDispatch =  0;
	pstCat->iMustSucc	=	 0;
	pstCat->iMaxMsgSize	=	 0;
	pstCat->stDevice.iType	=	TLOG_DEV_FILE;
	pstCat->stDevice.stDevice.stFile.szPattern[sizeof(pstCat->stDevice.stDevice.stFile.szPattern)-1] = 0;
	snprintf(pstCat->stDevice.stDevice.stFile.szPattern, sizeof(pstCat->stDevice.stDevice.stFile.szPattern)-1, "%s_rundata.log", a_pszPath);
	pstCat->stDevice.stDevice.stFile.iSyncTime	=	0;
	pstCat->stDevice.stDevice.stFile.iPrecision =	1;
	pstCat->stDevice.stDevice.stFile.iSizeLimit =	20*1024*1024;
	pstCat->stDevice.stDevice.stFile.iMaxRotate =	3;
	
	return 0;
}
