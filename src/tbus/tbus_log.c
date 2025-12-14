#include "tbus_log.h"
#include "tbus/tbus.h"

TLOGCTX	g_stBusLogCtx;
TLOGCONF	g_stBusLogConf;
LPTLOGCONF	g_pstBusLogConf = NULL;
LPTLOGCATEGORYINST	g_pstBusLogCat = NULL;


int tbus_init_log()
{
	int iRet = 0;


#ifdef TSF4G_THREAD_SAFE
	return 0;
#else
	
	if (NULL != g_pstBusLogCat)
	{
		return 0;
	}
	if (NULL == g_pstBusLogConf)
	{
		TLOGCATEGORY* pstCat;

		g_pstBusLogConf = &g_stBusLogConf;
		memset(&g_stBusLogConf, 0, sizeof(g_stBusLogConf));
		g_stBusLogConf.iPriorityHigh= TLOG_PRIORITY_NULL;;
		g_stBusLogConf.iPriorityLow = TLOG_PRIORITY_TRACE;

		g_stBusLogConf.iCount = 1;
		g_stBusLogConf.iSuppressError = 1;

		pstCat = &g_stBusLogConf.astCategoryList[0];
		STRNCPY(pstCat->szFormat,"[%d%u]%p %m %F:%l%n", sizeof(pstCat->szFormat));
		STRNCPY(pstCat->szName,"tsf4g.tbus", sizeof(pstCat->szName));

		pstCat->iPriorityHigh  =	TLOG_PRIORITY_NULL;
		pstCat->iPriorityLow   =   TLOG_PRIORITY_ERROR;
		pstCat->iLevelDispatch	=	1;

		pstCat->stDevice.iType	=	TLOG_DEV_FILE;
		snprintf(pstCat->stDevice.stDevice.stFile.szPattern, sizeof(pstCat->stDevice.stDevice.stFile.szPattern),
			"tbus.log");
		pstCat->stDevice.stDevice.stFile.iSyncTime	=	0;
		pstCat->stDevice.stDevice.stFile.iPrecision	=	1;
		pstCat->stDevice.stDevice.stFile.iSizeLimit	=	5*1024*1024;
		pstCat->stDevice.stDevice.stFile.iMaxRotate	=	3;
		
	}

	iRet = tlog_init(&g_stBusLogCtx, g_pstBusLogConf);
	if (0 == iRet)
	{
		g_pstBusLogCat = tlog_get_category(&g_stBusLogCtx, TBUS_LOGCAT_NAME);
		tlog_log(g_pstBusLogCat, TLOG_PRIORITY_FATAL, 0,0, "%s\n", "init log system ok");
	}else
	{
		printf("tlog_init failed, iRet %d\n", iRet);
		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INIT_LOGSYSTEM_FAILED);
	}
#endif

	return iRet;
}

void tbus_set_logcat( IN LPTLOGCATEGORYINST a_pstLogCat)
{
#ifndef TSF4G_THREAD_SAFE
	g_pstBusLogCat = a_pstLogCat;
#endif
}

void tbus_set_logpriority( IN int a_iPriority)
{
#ifndef TSF4G_THREAD_SAFE
	int i;
	for (i = 0; i < g_stBusLogConf.iCount; i++)
	{
		TLOGCATEGORY* pstCat = &g_stBusLogConf.astCategoryList[i];
		if (0 == stricmp(pstCat->szName, TBUS_LOGCAT_NAME))
		{
			pstCat->iPriorityLow = a_iPriority;
			break;
		}
	}/*for (i = 0; i < g_pstBusLogConf->iCount; i++)*/
#endif
}
