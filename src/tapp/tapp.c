/*
**  @file $RCSfile: tapp.c,v $
**  general description of this module
**  $Id: tapp.c,v 1.28 2009/03/27 06:17:35 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009/03/27 06:17:35 $
**  @version $Revision: 1.28 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "tapp/tapp.h"

/* if you use related module, you need to define the relative macro */

/* libtbus : TAPP_TBUS */
/* libtdr : TAPP_TDR */
/* libtdr_xml: TAPP_TDR_XML */

#define TAPP_TDR

#define TAPP_MIB
#define TAPP_METABASE

//#define TAPP_OBUS
#define TAPP_TBUS
#define TAPP_TDR_XML
#define TAPP_TLOG

#include "pal/pal.h"

#ifdef TAPP_TDR
#include "tdr/tdr.h"
#endif /* TAPP_TDR */

#ifdef TAPP_TBUS
#include "tbus/tbus.h"
#endif /* TAPP_TBUS */

#ifdef TAPP_OBUS
#include "obus/common.h"
#include "obus/oi_cfg.h"
#include "obus/oi_log.h"
#include "obus/og_bus.h"
#include "obus/og_channel.h"
#endif /* TAPP_OBUS */

#ifdef TAPP_MIB
#include "mng/tmib.h"
#endif /* TAPP_MIB */

#ifdef TAPP_METABASE
#include "mng/tmetabase.h"
#endif

#if defined(TAPP_METABASE) && defined(TAPP_MIB)
#include "mng/tmng_def.h"
#endif /* TAPP_METABASE && TAPP_MIB */

#ifdef TAPP_TLOG
#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"
#endif

#include "taa/tagentapi.h"
#include "tapp_basedata_def.h"
#include "tapp_rundata_timer_def.h"

#define TAPP_DOMAIN_BASE 	"base"
#define TAPP_DOMAIN_CFG 	"cfg"
#define TAPP_DOMAIN_CFGPRE   "cfgpre"
#define TAPP_DOMAIN_CFGBAK 	"cfgbak"
#define TAPP_DOMAIN_RT_STATUS		"runtime_status"
#define TAPP_DOMAIN_RT_CUMU    "runtime_cumulate"
#define TAPP_DOMAIN_RT_TIMER "runtime_timer"
#define TAPP_DOMAIN_LOGCFG 	"logcfg"
#define TAPP_DOMAIN_LOGCFGPRE    "logcfg_pre"
#define TAPP_DOMAIN_LOGCFGBAK 	"logcfg_bak"

#define TAPP_MAJOR			0x00
#define TAPP_MINOR			0x00
#define TAPP_REV			0x01
#define TAPP_BUILD			0x01

#define TAPP_DEF_VER		TAPP_MAKE_VERSION(TAPP_MAJOR, TAPP_MINOR, TAPP_REV, TAPP_BUILD)

#define TAPP_BUILD			0x01

#define TAPP_TIMER			100

#define TAPP_KILL_WAIT		10
#define TAPP_EPOLL_WAIT		10

#define TAPP_IDLE_COUNT		10
#define TAPP_IDLE_SLEEP		10

#define TAPP_BASEDATA_TIMER 300000
#define TAPP_RUNDATA_STATUS_TIMER     300000
#define TAPP_RUNDATA_CUMU_TIMER     300000

#define TAPP_START			"start"
#define TAPP_STOP			"stop"
#define TAPP_KILL			"kill"
#define TAPP_RELOAD			"reload"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

static int gs_iIsExit	=	0;
static int gs_iReload	=	0;
static int gs_iExitMainloop	=	0;

static char gs_szPidFile[PATH_MAX];
static char gs_szLogFile[PATH_MAX];
static char gs_szConfFile[PATH_MAX];
static char gs_szConfBakFile[PATH_MAX];
static char gs_szLogConfFile[PATH_MAX];
static char gs_szLogConfBakFile[PATH_MAX];
static char gs_szRunDataTimerConfFile[PATH_MAX];

#define TAPP_META_NAME_LEN	128

static char gs_szDefRunDataStatusMeta[TAPP_META_NAME_LEN];
static char gs_szDefRunDataCumuMeta[TAPP_META_NAME_LEN];

#ifdef TAPP_TLOG
static TLOGCTX gs_stLogCtx;
static TLOGCATEGORYINST *gs_pstAppCatText;
static TLOGCATEGORYINST *gs_pstAppCatData;
#endif /* TAPP_TLOG */

extern unsigned char g_szMetalib_tapp_basedata_def[];
extern unsigned char g_szMetalib_tapp_rundata_timer_def[];

/* signal handler. */

void tapp_on_sigterm(int iSig)
{
	switch( iSig )
	{
	default:
		gs_iIsExit	=	TAPP_EXIT_QUIT;
		break;
	}
}

void tapp_on_sigusr1(int iSig)
{
	switch( iSig )
	{
	default:
		gs_iIsExit	=	TAPP_EXIT_STOP;
		break;
	}
}

void tapp_on_sigusr2(int iSig)
{
	switch( iSig )
	{
	default:
		gs_iReload	=	1;
		break;
	}
}

/* initialization. */

int tapp_def_usage(int argc, char* argv[] )
{
	const char* pszApp;

	pszApp	=	basename(argv[0]);

	printf("Common Usage: %s [options] [%s| %s| %s |%s]\n", pszApp, TAPP_START, TAPP_STOP, TAPP_RELOAD, TAPP_KILL);
	printf("options:\n");
	printf("\t--id=[name]: specify the identifier of this process.\n");
	printf("\t--timer=[millisec]: specify the timer period, default %d ms.\n", TAPP_TIMER);
	printf("\t--wait=[sec]: specify the seconds to wait the previous process to exit, default %d s.\n", TAPP_KILL_WAIT);
	printf("\t--epoll-wait=[millisec]: specify the millisecondsfor epall_wait to timeout, default %d ms.\n", TAPP_EPOLL_WAIT);
	printf("\t--idle-sleep=[millisec]: specify the millisecondsfor for sleep when the process enter idle status, default %d ms.\n", TAPP_IDLE_SLEEP);
	printf("\t--idle-count=[num]: specify the count for idle cycle to enter idle status, default %d.\n", TAPP_IDLE_COUNT);
	printf("\t--conf-file=[path]: specify the path of config file for this process.\n");
	printf("\t--noloadconf: specify do not load configure file.\n");
	printf("\t--pid-file=[path]: specify the path of the file for storing the process\'s pid.\n");
	printf("\t--log-file=[path]: specify the path of the file for logging.\n");
	printf("\t--log-level=[num]: specify the logging level for this process.\n");

#ifdef TAPP_TDR
	printf("\t--tdr-file=[path]: specify the path of the tdr file for data presentation.\n");
	printf("\t--meta=[name] --conf-meta=[name]: specify the name of the config meta used in tdr file.\n");
#endif

#ifdef TAPP_TDR_XML
	printf("\t--generate: generate the template file for the meta used.\n");
#endif

#ifdef TAPP_TBUS
	printf("\t--use-bus: whether use tbus as communication mechanism.\n");
	printf("\t--bus-key: specify tbus_mgr key, default %s .\n", TBUS_DEFAULT_GCIM_KEY);
	printf("\t--refresh-bus-timer: specify the timer to refresh tbus channel configure. default %d seconds .\n",
		TAPP_REFRESH_TBUS_CONFIGRE_DEFAULT_TIMER);
#endif

#ifdef TAPP_OBUS
	printf("\t--use-oldbus: whether use old bus as communication mechanism.\n");
	printf("\t--bus-dir=[path]: specify the directory for storing bus configuration files.\n");
#endif

#ifdef TAPP_MIB
	printf("\t--use-mib: specify whether use mib for storing configration and runtime info.\n");
	printf("\t--mib-key=[key]: specify the key for mib.\n");
#endif /* TAPP_MIB */

#ifdef TAPP_METABASE
	printf("\t--metabase-key=[key], --mbase-key=[key]: specify the key for metabase.\n");
#endif /* TAPP_METABASE*/

#ifdef TAPP_TLOG
	printf("\t --tlogconf=[path] : specify the tlog cfg file path. if path is mib, then get tlog cfg from mib\n");
#endif

	printf("\t--business_id=[id]: specify the business id assgined by tagent system, default %d.\n",
		TAGENT_DEFAULT_BUSINESS_ID);

	printf("\t--daemon, -D: specify whether start this process as a daemon.\n");
	printf("\t--version, -v: print version information. \n");
	printf("\t--help, -h: print help information. \n");

	return 0;
}

int tapp_init_id(TAPPCTX* pstCtx, void* pvArg)
{
	if( !pstCtx->pszPidFile )
	{
		gs_szPidFile[sizeof(gs_szPidFile)-1]	=	'\0';

		if( pstCtx->pszId )
			snprintf(gs_szPidFile, sizeof(gs_szPidFile)-1, "%s%s.pid", TOS_TMP_DIR, pstCtx->pszId);
		else
			snprintf(gs_szPidFile, sizeof(gs_szPidFile)-1, "%s%s.pid", TOS_TMP_DIR, pstCtx->pszApp);

		pstCtx->pszPidFile	=	gs_szPidFile;
	}

	return 0;
}

int tapp_init_bus(TAPPCTX* pstCtx, void* pvArg)
{
	int iRet=0;

	switch( pstCtx->iUseBus )
	{
	case TAPP_USEBUS_TBUS:

#ifdef TAPP_TBUS
		pstCtx->iBus	=	-1;
		tbus_set_bussid(pstCtx->iBusinessID);

		iRet = tbus_init_ex(pstCtx->pszGCIMKey, 0);
		if( 0 != iRet)
		{
			printf("%s:error:tbus_init failed: %s.\n", pstCtx->pszApp, tbus_error_string(iRet));
			iRet	=	-1;
			break;
		}

		iRet = tbus_new(&pstCtx->iBus);
		if(0 != iRet )
		{
			printf("%s:error:tbus_new failed:%s.\n", pstCtx->pszApp, tbus_error_string(iRet));
			iRet	=	-1;
			break;
		}

		if( !pstCtx->pszBusDir )
			pstCtx->pszBusDir	=	TAPP_DEF_BUSDIR;

		if (NULL != pstCtx->pszId)
		{
			iRet = tbus_addr_aton(pstCtx->pszId, (TBUSADDR *)&pstCtx->iId);
			if (0 != iRet)
			{
				printf("%s: error failed to convert %s to tbus addr, for %s\n",
					pstCtx->pszApp, pstCtx->pszId, tbus_error_string(iRet));
				iRet	=	-1;
				break;
			}	

			iRet =   tbus_bind(pstCtx->iBus, pstCtx->iId);
			if(0 != iRet )
			{
				pstCtx->iBus	=	-1;
				printf("%s:error:can not bind addr %s, for %s.\n", pstCtx->pszApp,
					pstCtx->pszId, tbus_error_string(iRet));
				iRet	=	-1;
				break;
			}
		}/*if (NULL != pstCtx->pszId)*/
		
#endif /* TAPP_TBUS */

		break;

	case TAPP_USEBUS_OBUS:

#ifdef TAPP_OBUS	
		pstCtx->iBus		=	(int) calloc(1, sizeof(BusInterface));

		if(!pstCtx->iBus)
		{
			printf("%s:error:can not create bus.\n", pstCtx->pszApp);
			iRet	=	-1;
			break;
		}

		if( !pstCtx->pszBusDir )
			pstCtx->pszBusDir	=	TAPP_DEF_BUSDIR;

		if(BusConnectDir((BusInterface*)pstCtx->iBus, (char*)pstCtx->pszBusDir, pstCtx->iId, ".bus")<0 )
		{
			free((void*)pstCtx->iBus);
			pstCtx->iBus	=	0;
			printf("%s:error:can not connect bus.\n", pstCtx->pszApp);

			iRet	=	-1;
			break;
		}
#endif

		break;

	default:
		break;
	}

	return iRet;
}

int tapp_init_tdr(TAPPCTX* pstCtx, void* pvArg)
{
	int iRet=0;

#ifdef TAPP_TDR	
	if( pstCtx->pszTdrFile && !pstCtx->iLib )
	{
		iRet = tdr_load_metalib((LPTDRMETALIB*)&pstCtx->iLib, pstCtx->pszTdrFile);
		if( iRet < 0 )
		{
			printf("%s:error:can not load metalib file \'%s\', %s.\n", pstCtx->pszApp, 
				pstCtx->pszTdrFile, tdr_error_string(iRet));

			return -1;
		}

		pstCtx->iNeedFreeLib	=	1;
	}

	if( !pstCtx->iLib )
		return 0;

	if( !pstCtx->stConfData.pszMetaName )
	{
		pstCtx->stConfData.pszMetaName		=	(char*)tdr_get_metalib_name((LPTDRMETALIB)pstCtx->iLib);
	}

	if( pstCtx->stConfData.pszMetaName && !pstCtx->stConfData.iMeta )
	{
		pstCtx->stConfData.iMeta	=	(int) tdr_get_meta_by_name((LPTDRMETALIB)pstCtx->iLib, pstCtx->stConfData.pszMetaName);

		if( pstCtx->stConfData.iMeta )
		{
			pstCtx->stConfData.iLen	=	tdr_get_meta_size((LPTDRMETA)pstCtx->stConfData.iMeta);
		}
	}

	pstCtx->stConfPrepareData.pszMetaName = pstCtx->stConfBackupData.pszMetaName = pstCtx->stConfData.pszMetaName;
	pstCtx->stConfPrepareData.iLen = pstCtx->stConfBackupData.iLen = pstCtx->stConfData.iLen;
	pstCtx->stConfPrepareData.iMeta = pstCtx->stConfBackupData.iMeta = pstCtx->stConfData.iMeta;

	if (!pstCtx->stRunDataStatus.pszMetaName)
	{
		gs_szDefRunDataStatusMeta[sizeof(gs_szDefRunDataStatusMeta)-1]	=	'\0';
		snprintf(gs_szDefRunDataStatusMeta, sizeof(gs_szDefRunDataStatusMeta)-1, "%srun_status", pstCtx->pszApp);	
		pstCtx->stRunDataStatus.pszMetaName = gs_szDefRunDataStatusMeta;
	}

	if( pstCtx->stRunDataStatus.pszMetaName && !pstCtx->stRunDataStatus.iMeta )
	{
		pstCtx->stRunDataStatus.iMeta	=	(int) tdr_get_meta_by_name((LPTDRMETALIB)pstCtx->iLib, pstCtx->stRunDataStatus.pszMetaName);

		if( pstCtx->stRunDataStatus.iMeta )
		{
			pstCtx->stRunDataStatus.iLen	=	tdr_get_meta_size((LPTDRMETA)pstCtx->stRunDataStatus.iMeta);
		}
	}

	if (!pstCtx->stRunDataCumu.pszMetaName)
	{
		gs_szDefRunDataCumuMeta[sizeof(gs_szDefRunDataCumuMeta)-1]	=	'\0';
		snprintf(gs_szDefRunDataCumuMeta, sizeof(gs_szDefRunDataCumuMeta)-1, "%srun_cumulate", pstCtx->pszApp);	
		pstCtx->stRunDataCumu.pszMetaName = gs_szDefRunDataCumuMeta;
	}

	if( pstCtx->stRunDataCumu.pszMetaName && !pstCtx->stRunDataCumu.iMeta )
	{
		pstCtx->stRunDataCumu.iMeta	=	(int) tdr_get_meta_by_name((LPTDRMETALIB)pstCtx->iLib, pstCtx->stRunDataCumu.pszMetaName);

		if( pstCtx->stRunDataCumu.iMeta )
		{
			pstCtx->stRunDataCumu.iLen	=	tdr_get_meta_size((LPTDRMETA)pstCtx->stRunDataCumu.iMeta);
		}
	}

#endif /* TAPP_TDR */

	return iRet;
}

int tapp_do_generate(TAPPCTX* pstCtx, void* pvArg)
{
#ifdef TAPP_TDR_XML
	int iSize;
	TDRDATA stData;

	if( !pstCtx->stConfData.iMeta )
	{
		printf("%s:error:tdr-file or meta is not valid.\n", pstCtx->pszApp);
		return -1;
	}

	iSize	=	tdr_get_meta_size((LPTDRMETA)pstCtx->stConfData.iMeta);

	if( iSize<=0 )
	{
		printf("%s:error:bad meta size.\n", pstCtx->pszApp);
		return -1;
	}

	stData.iBuff	=	iSize;

	stData.pszBuff	=	calloc(1, stData.iBuff);
	if( !stData.pszBuff )
	{
		printf("%s:error:memory allocate error.\n", pstCtx->pszApp);
		return -1;
	}

	if( tdr_init((LPTDRMETA)pstCtx->stConfData.iMeta, &stData, 0)<0 )
	{
		printf("%s:error:data initialize by meta failed.\n", pstCtx->pszApp);
		return -1;
	}

	stData.iBuff	=	iSize;

	if( pstCtx->pszConfFile )
		tdr_output_file((LPTDRMETA)pstCtx->stConfData.iMeta, pstCtx->pszConfFile, &stData, 0, 0);
	else
		tdr_output_fp((LPTDRMETA)pstCtx->stConfData.iMeta, stdout, &stData, 0, 0);

#endif /* TAPP_TDR_XML */	

	return 0;
}

int tapp_fini_tdr(TAPPCTX* pstCtx, void* pvArg)
{
	if( pstCtx->iLib && pstCtx->iNeedFreeLib )
	{
		tdr_free_lib((LPTDRMETALIB*)&pstCtx->iLib);
	}

	return 0;
}

int tapp_init_metabase(TAPPCTX* pstCtx, void* pvArg)
{
#ifdef TAPP_METABASE
	if( pstCtx->iUseMib && tmb_open((HTMBDESC*)&pstCtx->iMBase, pstCtx->pszMBaseKey, 1)<0 )
	{
		printf("%s:error:initialize metabase failed.\n", pstCtx->pszApp);
		return -1;
	}
#endif /* TAPP_METABASE */

	return 0;
}

int tapp_fini_metabase(TAPPCTX* pstCtx, void* pvArg)
{
#ifdef TAPP_METABASE
	if( pstCtx->iUseMib && pstCtx->iMBase )
	{
		tmb_close((HTMBDESC*)&pstCtx->iMBase);
		pstCtx->iMBase	=	0;
	}
#endif /* TAPP_METABASE */

	return 0;
}

int tapp_init_mib(TAPPCTX* pstCtx, void* pvArg)
{
#ifdef TAPP_MIB
	if( pstCtx->iUseMib && tmib_open((HTMIB*)&pstCtx->iMib, pstCtx->pszMibKey)<0 )
	{
		printf("%s:error:tmib_open failed.\n", pstCtx->pszApp);
		return -1;
	}
#endif /* TAPP_MIB */

	return 0;
}

int tapp_fini_mib(TAPPCTX* pstCtx, void* pvArg)
{
#ifdef TAPP_MIB
	if( pstCtx->iUseMib && pstCtx->iMib )
	{
		tmib_close((HTMIB*)&pstCtx->iMib);
		pstCtx->iMib	=	0;
	}
#endif /* TAPP_MIB */

	return 0;
}

static int tapp_init_data_by_file(TAPPDATA* pstAppData, const char* pszPath)
{
	int iRet=0;

	if( !pstAppData->pszBuff || pstAppData->iLen<=0 || !pstAppData->iMeta )
		return -1;

#ifdef TAPP_TDR_XML
	{	
		TDRDATA stHost;

		stHost.pszBuff	=	pstAppData->pszBuff;
		stHost.iBuff	=	pstAppData->iLen;

		iRet	=	tdr_input_file((LPTDRMETA)pstAppData->iMeta, &stHost, pszPath, 0, 0);
		if (0 > iRet)
		{
			printf("input file %s error:%s\n", pszPath, tdr_error_string(iRet));
		}
	}
#endif /* TAPP_TDR_XML */
	
	return iRet;
}


int tapp_init_log(TAPPCTX* pstCtx, void* pvArg)
{
	int iRet = -1;
	TLOGCONF* pstLogConf;

#ifdef TAPP_TLOG

	if (NULL == pstCtx->stLogConfData.pszBuff)
	{
		return -1;
	}

	pstLogConf = (TLOGCONF*) pstCtx->stLogConfData.pszBuff;

	if( NULL == pstCtx->pszLogFile )
	{
		gs_szLogFile[sizeof(gs_szLogFile)-1]    =       '\0';

		if( pstCtx->pszId )
			snprintf(gs_szLogFile, sizeof(gs_szLogFile)-1, "%s%s_%s", TOS_TMP_DIR, pstCtx->pszApp, pstCtx->pszId);
		else
			snprintf(gs_szLogFile, sizeof(gs_szLogFile)-1, "%s%s", TOS_TMP_DIR, pstCtx->pszApp);

        pstCtx->pszLogFile      =       gs_szLogFile;
    }

	if (0 == pstCtx->iUseMib)
	{
		struct stat stStat;
		
		if (NULL == pstCtx->pszLogConfFile)
		{
			gs_szLogConfFile[sizeof(gs_szLogConfFile)-1] = 0;
			snprintf(gs_szLogConfFile, sizeof(gs_szLogConfFile)-1, "%s_log.xml", pstCtx->pszApp);
			pstCtx->pszLogConfFile = gs_szLogConfFile;
		}
		
		if (0 == access(pstCtx->pszLogConfFile, R_OK))
		{
			if (0 > tapp_init_data_by_file(&pstCtx->stLogConfData, pstCtx->pszLogConfFile))
			{
				printf("cant load log conf file %s\n", pstCtx->pszLogConfFile);
				return -1;
			}
		}
		else
		{
			TDRDATA stData;
			
			if (0 > tlog_init_cfg_default(pstLogConf, pstCtx->pszLogFile))
			{
				return -1;
			}

			if (pstCtx->pszLogLevel)
			{
				pstLogConf->iPriorityLow= atoi(pstCtx->pszLogLevel);
			}

			stData.iBuff	=	pstCtx->stLogConfData.iLen;
			stData.pszBuff	=	pstCtx->stLogConfData.pszBuff;
			tdr_output_file((LPTDRMETA)pstCtx->stLogConfData.iMeta, pstCtx->pszLogConfFile, &stData, 0, 0);
		}

		if (0 == stat(pstCtx->pszLogConfFile, &stStat))
		{
			pstCtx->tLogConfMod = stStat.st_mtime;
		}
	}

	gs_szLogConfBakFile[sizeof(gs_szLogConfBakFile)-1] = 0;	
	if (pstCtx->pszLogConfFile)
	{
		snprintf(gs_szLogConfBakFile, sizeof(gs_szLogConfBakFile)-1, "%s.bak", pstCtx->pszLogConfFile);
	}
	else
	{
		snprintf(gs_szLogConfBakFile, sizeof(gs_szLogConfBakFile)-1, "%s_log.xml.bak", pstCtx->pszApp);
	}
	
	if (pstCtx->pszLogLevel)
	{
		pstLogConf->iPriorityLow= atoi(pstCtx->pszLogLevel);
	}

	iRet = tlog_init(&gs_stLogCtx, pstLogConf);
	if (0 > iRet)
	{
		printf("log init fail %s, please check log conf file again\n", 
			tlog_error_string(iRet));
		return -1;
	}

	if (TAPP_USEBUS_TBUS == pstCtx->iUseBus)
	{
		tbus_set_logcat(tlog_get_category(&gs_stLogCtx, TLOG_DEF_CATEGORY_BUS));
	}

	gs_pstAppCatText = tlog_get_category(&gs_stLogCtx, TLOG_DEF_CATEGORY_TEXTROOT);
	gs_pstAppCatData = tlog_get_category(&gs_stLogCtx, TLOG_DEF_CATEGORY_DATAROOT);
		
	return 0;
	
#endif /* TAPP_TLOG */

	return iRet;
}

int tapp_init_rundata_timer(TAPPCTX* pstCtx, void* pvArg)
{
	TAPP_RUNDATA_TIMER *pstConf;
		
	if (NULL == pstCtx->stRunDataTimer.pszBuff)
	{
		return -1;
	}

	pstConf = (TAPP_RUNDATA_TIMER*) pstCtx->stRunDataTimer.pszBuff;

	if (0 == pstCtx->iUseMib)
	{
		struct stat stStat;
		
		if (NULL == pstCtx->pszRundataTimerConf)
		{
			gs_szRunDataTimerConfFile[sizeof(gs_szRunDataTimerConfFile) -1] = 0;
			snprintf(gs_szRunDataTimerConfFile, sizeof(gs_szRunDataTimerConfFile) -1, 
				"%s_rundata_timer.xml", pstCtx->pszApp);
		}
		else
		{
			STRNCPY(gs_szRunDataTimerConfFile, pstCtx->pszRundataTimerConf,
						sizeof(gs_szRunDataTimerConfFile));
		}
		
		if (0 == access(gs_szRunDataTimerConfFile, R_OK))
		{
			if (0 > tapp_init_data_by_file(&pstCtx->stRunDataTimer, gs_szRunDataTimerConfFile))
			{
				printf("cant load rundata timer conf file %s\n", gs_szRunDataTimerConfFile);
				return -1;
			}
		}
		else
		{
			TDRDATA stData;

			memset(pstConf, 0, sizeof(*pstConf));
			pstConf->stBasedata_timer.lGlobalTime = TAPP_BASEDATA_TIMER;
			pstConf->stRundata_cumu_timer.lGlobalTime = TAPP_RUNDATA_CUMU_TIMER;
			pstConf->stRundata_status_timer.lGlobalTime = TAPP_RUNDATA_STATUS_TIMER;
			
			stData.iBuff	=	pstCtx->stRunDataTimer.iLen;
			stData.pszBuff	=	pstCtx->stRunDataTimer.pszBuff;
			tdr_output_file((LPTDRMETA)pstCtx->stRunDataTimer.iMeta, gs_szRunDataTimerConfFile, &stData, 0, 0);
		}

		if (0 == stat(gs_szRunDataTimerConfFile, &stStat))
		{
			pstCtx->tRunTimerConfMod = stStat.st_mtime;
		}
	}

	return 0;	
}


int tapp_fini_log(TAPPCTX* pstCtx, void* pvArg)
{
	int iRet = -1;

#ifdef TAPP_TLOG

	iRet	=	tlog_fini(&gs_stLogCtx);

#endif /* TAPP_TLOG */

	return iRet;
}

static int tapp_alloc_one_data(TAPPDATA* pstAppData)
{
	if( pstAppData->pszBuff || 0==pstAppData->iLen )
		return 0;

	pstAppData->pszBuff	=	(char*) calloc(1, pstAppData->iLen);

	return pstAppData->pszBuff ? 0 : -1;
}

static int tapp_update_logconf(TLOGCONF *pstLogConf, TLOGCONF *pstLogConfPre)
{
	int i, j;
	TLOGCATEGORY *pstCat;
	TLOGCATEGORY *pstCatPre;
	TLOGCATEGORYINST* pstCatInst;
	
	pstLogConf->iPriorityHigh= pstLogConfPre->iPriorityHigh;
	pstLogConf->iPriorityLow= pstLogConfPre->iPriorityLow;

	for (i=0; i<pstLogConf->iCount; i++)
	{
		pstCat = &pstLogConf->astCategoryList[i];

		for (j=0; j<pstLogConfPre->iCount; j++)
		{
			pstCatPre = &pstLogConfPre->astCategoryList[j];
			if (strcasecmp(pstCat->szName, pstCatPre->szName))
			{
				continue;
			}
					
			memcpy(pstCat, pstCatPre, sizeof(*pstCat));
			pstCatInst = tlog_get_category(&gs_stLogCtx, pstCat->szName);
			if (pstCatInst)
			{
				if( pstCatInst->iInited )
				{
					tlogany_fini(&pstCatInst->stLogAny);
					pstCatInst->iInited	=	0;
				}
				
				if (0 == tlogany_init(&pstCatInst->stLogAny, &pstCat->stDevice))
				{
					pstCatInst->iInited = 1;
				}
			}
		}
	}
	
	return 0;
}

int tapp_reload_log(TAPPCTX* pstCtx, void* pvArg)
{	
	int iRet = -1;

#ifdef TAPP_TLOG
	TDRDATA stHost;
	TLOGCONF *pstLogConfPre;
	TLOGCONF *pstLogConf;

	if (NULL == pstCtx->stLogConfData.pszBuff || NULL == pstCtx->stLogConfBackupData.pszBuff || NULL == pstCtx->stLogConfPrepareData.pszBuff)
	{
		return -1;
	}

	memcpy(pstCtx->stLogConfBackupData.pszBuff, pstCtx->stLogConfData.pszBuff, pstCtx->stLogConfBackupData.iLen);
		
	if( 0 == pstCtx->iUseMib )
	{
		struct stat stStat;
		if (0 == stat(pstCtx->pszLogConfFile, &stStat))
		{
			if (pstCtx->tLogConfMod == stStat.st_mtime)
			{
				return 0;
			}
			pstCtx->tLogConfMod = stStat.st_mtime;
		}
		
		if (0 > tapp_init_data_by_file(&pstCtx->stLogConfPrepareData, pstCtx->pszLogConfFile))
		{
			return -1;
		}
	}

	stHost.pszBuff	=	(char*)pstCtx->stLogConfBackupData.pszBuff;
	stHost.iBuff	=	(int) pstCtx->stLogConfBackupData.iLen;
	tdr_output_file((LPTDRMETA)pstCtx->stLogConfBackupData.iMeta, gs_szLogConfBakFile, &stHost, 0, 0);
		
 	pstLogConf = (TLOGCONF *)pstCtx->stLogConfData.pszBuff;
	pstLogConfPre = (TLOGCONF *)pstCtx->stLogConfPrepareData.pszBuff;

	tapp_update_logconf(pstLogConf, pstLogConfPre);
		
	printf("---------reload log conf-----------\n");
	printf("old log conf in file %s, new log conf:\n", gs_szLogConfBakFile);
	stHost.pszBuff	=	(char*)pstCtx->stLogConfData.pszBuff;
	stHost.iBuff	=	(int) pstCtx->stLogConfData.iLen;
	tdr_output_fp((LPTDRMETA)pstCtx->stLogConfData.iMeta, stdout, &stHost, 0, 0);
	printf("---------reload log conf end--------\n");

	if (TAPP_USEBUS_TBUS == pstCtx->iUseBus)
	{
		tbus_set_logcat(tlog_get_category(&gs_stLogCtx, TLOG_DEF_CATEGORY_BUS));
	}

#endif /* TAPP_TLOG */

	return iRet;
}


static int tapp_prepare_one_data(TAPPCTX* pstCtx, TAPPDATA* pstAppData, const char* pszDomain)
{
	int iRet=0;
	TMIBDATA stMibData;

	if( !pstCtx->iUseMib )
		return -1;

#ifdef TAPP_MIB

	memset(&stMibData, 0, sizeof(stMibData));
	STRNCPY(stMibData.szName, pstCtx->pszApp, sizeof(stMibData.szName));
	stMibData.iProcID  =   pstCtx->iId;

	STRNCPY(stMibData.szDomain, pszDomain, sizeof(stMibData.szDomain));

	if( tmib_get_data((HTMIB)pstCtx->iMib, &stMibData, 1)<0 )
		return -1;

	pstAppData->pszBuff	=	stMibData.pszData;
	pstAppData->iLen	=	stMibData.iSize;

#ifdef TAPP_TDR
	iRet	=	tmb_open_metalib((HTMBDESC)pstCtx->iMBase, stMibData.szLib, stMibData.iVersion);
	
	if( 0==iRet )
	{
		iRet	=	tmb_meta_by_name((HTMBDESC)pstCtx->iMBase, stMibData.szMeta, (LPTDRMETA*)&pstAppData->iMeta);
	}
#endif /* TAPP_TDR */

#endif /* TAPP_MIB */

	return iRet;
}



int tapp_init_data(TAPPCTX* pstCtx, void* pvArg)
{
	int iRet=0;

	if( pstCtx->iUseMib )
	{
		iRet	=	tapp_prepare_one_data(pstCtx, &pstCtx->stBaseData, TAPP_DOMAIN_BASE);
		if (0 > iRet)
		{
			printf("warning: there is no %s domain in mib\n", TAPP_DOMAIN_BASE);
		}
		else
		{
			memset(pstCtx->stBaseData.pszBuff, 0, pstCtx->stBaseData.iLen);
		}
		
		iRet	=	tapp_prepare_one_data(pstCtx, &pstCtx->stConfData, TAPP_DOMAIN_CFG);
		if (0 > iRet)
		{
			printf("warning: there is no %s domain in mib\n", TAPP_DOMAIN_CFG);
		}
		
		iRet	=	tapp_prepare_one_data(pstCtx, &pstCtx->stConfPrepareData, TAPP_DOMAIN_CFGPRE);
		if (0 > iRet)
		{
			printf("warning: there is no %s domain in mib\n", TAPP_DOMAIN_CFGPRE);
		}
		
		iRet	=	tapp_prepare_one_data(pstCtx, &pstCtx->stConfBackupData, TAPP_DOMAIN_CFGBAK);
		if (0 > iRet)
		{
			printf("warning: there is no %s domain in mib\n", TAPP_DOMAIN_CFGBAK);
		}
		
		iRet	=	tapp_prepare_one_data(pstCtx, &pstCtx->stRunDataStatus, TAPP_DOMAIN_RT_STATUS);
		if (0 > iRet)
		{
			printf("warning: there is no %s domain in mib\n", TAPP_DOMAIN_RT_STATUS);
		}
		else
		{
			memset(pstCtx->stRunDataStatus.pszBuff, 0, pstCtx->stRunDataStatus.iLen);
		}
		
		iRet	=	tapp_prepare_one_data(pstCtx, &pstCtx->stRunDataCumu, TAPP_DOMAIN_RT_CUMU);
		if (0 > iRet)
		{
			printf("warning: there is no %s domain in mib\n", TAPP_DOMAIN_RT_CUMU);
		}
		else
		{
			memset(pstCtx->stRunDataCumu.pszBuff, 0, pstCtx->stRunDataCumu.iLen);
		}
		
		iRet	=	tapp_prepare_one_data(pstCtx, &pstCtx->stRunDataTimer, TAPP_DOMAIN_RT_TIMER);
		if (0 > iRet)
		{
			printf("warning: there is no %s domain in mib\n", TAPP_DOMAIN_RT_TIMER);
		}
		
		iRet	=	tapp_prepare_one_data(pstCtx, &pstCtx->stLogConfData, TAPP_DOMAIN_LOGCFG);
		if (0 > iRet)
		{
			printf("warning: there is no %s domain in mib\n", TAPP_DOMAIN_LOGCFG);
		}
		
		iRet	=	tapp_prepare_one_data(pstCtx, &pstCtx->stLogConfPrepareData, TAPP_DOMAIN_LOGCFGPRE);
		if (0 > iRet)
		{
			printf("warning: there is no %s domain in mib\n", TAPP_DOMAIN_LOGCFGPRE);
		}
		
		iRet	=	tapp_prepare_one_data(pstCtx, &pstCtx->stLogConfBackupData, TAPP_DOMAIN_LOGCFGBAK);
		if (0 > iRet)
		{
			printf("warning: there is no %s domain in mib\n", TAPP_DOMAIN_LOGCFGBAK);
		}
	}
	else
	{
		pstCtx->stBaseData.iLen = (int) sizeof(TAPP_BASEDATA);
		pstCtx->stBaseData.pszMetaName = "tapp_basedata";
		pstCtx->stBaseData.iMeta = (int)tdr_get_meta_by_name((LPTDRMETALIB)(&g_szMetalib_tapp_basedata_def[0]), pstCtx->stBaseData.pszMetaName);

		pstCtx->stRunDataTimer.iLen = (int)sizeof(TAPP_RUNDATA_TIMER);
		pstCtx->stRunDataTimer.pszMetaName = "tapp_rundata_timer";
		pstCtx->stRunDataTimer.iMeta = (int)tdr_get_meta_by_name((LPTDRMETALIB)(&g_szMetalib_tapp_rundata_timer_def[0]), pstCtx->stRunDataTimer.pszMetaName);
				
		pstCtx->stLogConfData.iLen	=	(int) sizeof(TLOGCONF);
		pstCtx->stLogConfData.pszMetaName = "TLOGConf";
		pstCtx->stLogConfData.iMeta = (int)tdr_get_meta_by_name((LPTDRMETALIB)(tlog_get_meta_data()), pstCtx->stLogConfData.pszMetaName);

		pstCtx->stLogConfPrepareData.iLen = pstCtx->stLogConfBackupData.iLen = pstCtx->stLogConfData.iLen;
		pstCtx->stLogConfPrepareData.pszMetaName = pstCtx->stLogConfBackupData.pszMetaName = pstCtx->stLogConfData.pszMetaName;
		pstCtx->stLogConfPrepareData.iMeta = pstCtx->stLogConfBackupData.iMeta = pstCtx->stLogConfData.iMeta;
		
		iRet	=	tapp_alloc_one_data(&pstCtx->stBaseData);
		iRet	=	tapp_alloc_one_data(&pstCtx->stConfData);
		iRet	=	tapp_alloc_one_data(&pstCtx->stConfPrepareData);
		iRet	=	tapp_alloc_one_data(&pstCtx->stConfBackupData);
		iRet	=	tapp_alloc_one_data(&pstCtx->stRunDataStatus);
		iRet	=	tapp_alloc_one_data(&pstCtx->stRunDataCumu);
		iRet    =   tapp_alloc_one_data(&pstCtx->stRunDataTimer);
		iRet	=	tapp_alloc_one_data(&pstCtx->stLogConfData);
		iRet	=	tapp_alloc_one_data(&pstCtx->stLogConfPrepareData);
		iRet    =   tapp_alloc_one_data(&pstCtx->stLogConfBackupData);

		if( !pstCtx->iNoLoadConf )
		{
			iRet	=	tapp_init_data_by_file(&pstCtx->stConfData, pstCtx->pszConfFile);
			if (0 > iRet)
			{
				printf("cant load app conf file %s\n", pstCtx->pszConfFile);
			}
			else
			{
				struct stat stStat;
				
				if (0 == stat(pstCtx->pszConfFile, &stStat))
				{
					pstCtx->tConfMod = stStat.st_mtime;
				}
			}
		}
		
	}

	return iRet;
}

static int tapp_fini_one_data(TAPPDATA* pstAppData)
{
	if( pstAppData->pszBuff )
	{
		free(pstAppData->pszBuff);
		pstAppData->pszBuff	=	NULL;
		pstAppData->iLen	=	0;
	}

	return 0;
}

int tapp_fini_data(TAPPCTX* pstCtx, void* pvArg)
{
	if( pstCtx->iUseMib )
	{
		/* do nothing. */
	}
	else
	{
		tapp_fini_one_data(&pstCtx->stBaseData);
		tapp_fini_one_data(&pstCtx->stConfData);
		tapp_fini_one_data(&pstCtx->stConfPrepareData);
		tapp_fini_one_data(&pstCtx->stConfBackupData);
		tapp_fini_one_data(&pstCtx->stRunDataStatus);
		tapp_fini_one_data(&pstCtx->stRunDataCumu);
		tapp_fini_one_data(&pstCtx->stLogConfData);
		tapp_fini_one_data(&pstCtx->stLogConfPrepareData);
		tapp_fini_one_data(&pstCtx->stLogConfBackupData);
	}

	return 0;
}

int tapp_def_opt(TAPPCTX* pstCtx, void* pvArg)
{
	const char TAPP_SUFFIX_XML[]=".xml";
	int iRet;
	int opt;
	int iOldOptErr;
	int iIsStart=0;
	int iIsDaemon=0;
	int iIsGenerate=0;
	char* pszApp;
	char* pszDot;
	int iLen;

	int iOptIdx=0;

	static int s_iOptChar=0;
    static struct option s_astLongOptions[] = {
		{"id", 1, &s_iOptChar, 'i'},
		{"pid-file", 1, &s_iOptChar, 'P'},
		{"conf-file", 1, &s_iOptChar, 'C'},
		{"noloadconf", 0, &s_iOptChar, 'n'},
		{"log-file", 1, &s_iOptChar, 'L'},
		{"log-level", 1, &s_iOptChar, 'l'},

#ifdef TAPP_TDR		
		{"tdr-file", 1, &s_iOptChar, 'T'},
		{"meta", 1, &s_iOptChar, 'm'},
		{"conf-meta", 1, &s_iOptChar, 'm'},
#endif /* TAPP_TDR */

#ifdef TAPP_TDR_XML
		{"generate", 0, &s_iOptChar, 'g'},
#endif /* TAPP_TDR_XML */

		{"timer", 1, &s_iOptChar, 't'},
		{"wait", 1, &s_iOptChar, 'w'},
		{"epoll-wait", 1, &s_iOptChar, 'e'},
		{"idle-sleep", 1, &s_iOptChar, 's'},
		{"idle-count", 1, &s_iOptChar, 'c'},

#ifdef TAPP_TBUS
		{"use-bus", 0, &s_iOptChar, 'b'},
		{"bus-key", 1, &s_iOptChar, 'y'},
		{"refresh-bus-timer", 1, &s_iOptChar, 'f'},
#endif /* TAPP_TBUS */

//#ifdef TAPP_OBUS
		{"use-obus", 0, &s_iOptChar, 'o'},
		{"bus-dir", 1, &s_iOptChar, 'B'},
//#endif /* TAPP_OBUS */

#ifdef TAPP_METABASE
		{"metabase-key", 1, &s_iOptChar, 'k'},
		{"mbase-key", 1, &s_iOptChar, 'k'},
#endif /* TAPP_METABASE */

#ifdef TAPP_MIB
		{"use-mib", 0, &s_iOptChar, 'M'},
		{"mib-key", 1, &s_iOptChar, 'K'},
#endif /* TAPP_MIB */

#ifdef TAPP_TLOG
		{"tlogconf", 1, &s_iOptChar, 'G'},
		{"rundata_timer", 1, &s_iOptChar, 'r'}, 
#endif

		{"idle_count", 1, &s_iOptChar, 'c'},
		{"business_id", 1, &s_iOptChar, 'u'},
		{"daemon", 0, 0, 'D'},
		{"version", 0, 0, 'v'},
		{"help", 0, 0, 'h'},

		{0, 0, 0, 0}
	};

	assert( pstCtx );

	pszApp		=	basename(pstCtx->argv[0]);

	pstCtx->pszApp	=	pszApp;

	pszDot	=	strrchr(pszApp, TOS_DIRDOT);

	if( pszDot )
		iLen	=	pszDot - pszApp;
	else
		iLen	=	strlen(pszApp);

	if( iLen + (int)sizeof(TAPP_SUFFIX_XML) > (int) sizeof(gs_szConfFile) )
		iLen	=	(int) sizeof(gs_szConfFile) - (int) sizeof(TAPP_SUFFIX_XML);

	memcpy(gs_szConfFile, pszApp, iLen);
	strcpy(gs_szConfFile + iLen, TAPP_SUFFIX_XML);
	
	pstCtx->pszConfFile	=	gs_szConfFile;

	iOldOptErr	=	opterr;

	opterr	=	0;

	if( 0==pstCtx->iWait )
		pstCtx->iWait	=	TAPP_KILL_WAIT;

	if( 0==pstCtx->iEpollWait )
		pstCtx->iEpollWait	=	TAPP_EPOLL_WAIT;

	if( 0==pstCtx->iIdleCount )
		pstCtx->iIdleCount	=	TAPP_IDLE_COUNT;

	if( 0==pstCtx->iIdleSleep )
		pstCtx->iIdleSleep	=	TAPP_IDLE_SLEEP;

	if( 0==pstCtx->iTimer )
		pstCtx->iTimer  =  TAPP_TIMER;

	if( 0==pstCtx->uiVersion )
		pstCtx->uiVersion	=	TAPP_DEF_VER;
	

#if defined(TAPP_METABASE) && defined(TAPP_MIB)

	if( NULL==pstCtx->pszMBaseKey )
		pstCtx->pszMBaseKey	=	DEFAULT_METABASE_KEY;

	if( NULL==pstCtx->pszMibKey )
		pstCtx->pszMibKey	=	DEFAULT_MIB_KEY;

#endif /* TAPP_METABASE && TAPP_MIB */

#ifdef TAPP_TBUS
	if (0 == pstCtx->pszGCIMKey)
		pstCtx->pszGCIMKey = TBUS_DEFAULT_GCIM_KEY;
	if (0 >= pstCtx->stOption.iRefeshTbusTimer)
		pstCtx->stOption.iRefeshTbusTimer = TAPP_REFRESH_TBUS_CONFIGRE_DEFAULT_TIMER;
#endif
	

	while(-1 != (opt=getopt_long(pstCtx->argc, pstCtx->argv, "Dvh", s_astLongOptions, &iOptIdx)) )
	{
		switch(opt)
		{
		case '\0':
			switch(*s_astLongOptions[iOptIdx].flag)
			{
			case 't':
				pstCtx->iTimer	=	strtol(optarg, NULL, 0);

				if( LONG_MAX==pstCtx->iTimer && ERANGE==errno )
				{
					printf("%s:error: bad timer value.\n", pszApp);
					exit(EINVAL);
				}

				break;

			case 'w':
				pstCtx->iWait	=	strtol(optarg, NULL, 0);

				if( LONG_MAX==pstCtx->iWait && ERANGE==errno )
				{
					printf("%s:error: bad wait time.\n", pszApp);
					exit(EINVAL);
				}

				break;

			case 'e':
				pstCtx->iEpollWait	=	strtol(optarg, NULL, 0);

				if( LONG_MAX==pstCtx->iEpollWait && ERANGE==errno )
				{
					printf("%s:error: bad epoll-wait time.\n", pszApp);
					exit(EINVAL);
				}

				break;

			case 'c':
				pstCtx->iIdleCount	=	strtol(optarg, NULL, 0);

				if( LONG_MAX==pstCtx->iIdleCount && ERANGE==errno )
				{
					printf("%s:error: bad idle-count value.\n", pszApp);
					exit(EINVAL);
				}

				break;

			case 's':
				pstCtx->iIdleSleep	=	strtol(optarg, NULL, 0);

				if( LONG_MAX==pstCtx->iIdleSleep && ERANGE==errno )
				{
					printf("%s:error: bad idle-sleep value.\n", pszApp);
					exit(EINVAL);
				}

				break;

			case 'P':
				pstCtx->pszPidFile	=	optarg;
				break;

			case 'C':
				pstCtx->pszConfFile	=	optarg;
				break;

			case 'n':
				pstCtx->iNoLoadConf	=	1;
				break;

			case 'L':
				pstCtx->pszLogFile	=	optarg;
				break;

			case 'l':
				pstCtx->pszLogLevel	=	optarg;
				break;

			case 'i':
				pstCtx->pszId	=	optarg;

#ifdef TAPP_TBUS
				pstCtx->iUseBus	=	TAPP_USEBUS_TBUS;
#endif /* TAPP_TBUS */

				break;

#ifdef TAPP_TDR
			case 'T':
				pstCtx->pszTdrFile	=	optarg;
				break;

			case 'm':
				pstCtx->stConfData.pszMetaName	=	optarg;
				pstCtx->stConfPrepareData.pszMetaName	=	optarg;
				pstCtx->stConfBackupData.pszMetaName	=	optarg;
				break;
			
#endif /* TAPP_TDR 	*/

#ifdef TAPP_TBUS
			case 'b':
				pstCtx->iUseBus	=	TAPP_USEBUS_TBUS;
				break;
			case 'y':
				if (NULL != optarg)
					pstCtx->pszGCIMKey = optarg;
				break;
			case 'f':
				{
					if (NULL != optarg)
						pstCtx->stOption.iRefeshTbusTimer = strtol(optarg, NULL, 10);
					if (0 >= pstCtx->stOption.iRefeshTbusTimer)
						pstCtx->stOption.iRefeshTbusTimer = TAPP_REFRESH_TBUS_CONFIGRE_DEFAULT_TIMER;
				}
				break;
#endif /* TAPP_TBUS */

//#ifdef TAPP_OBUS
			case 'o':
				pstCtx->iUseBus	=	TAPP_USEBUS_OBUS;
				break;

			case 'B':
				pstCtx->iUseBus	=	TAPP_USEBUS_OBUS;
				pstCtx->pszBusDir	=	optarg;

				break;
//#endif /* TAPP_OBUS */

#ifdef TAPP_METABASE
			case 'k':
				pstCtx->pszMBaseKey	=	optarg;
				break;
#endif /* TAPP_METABASE */

#ifdef TAPP_MIB				
			case 'K':
				pstCtx->pszMibKey	=	optarg;
				break;

			case 'M':
				pstCtx->iUseMib		=	1;
				break;
#endif /* TAPP_MIB */

#ifdef TAPP_TLOG
			case 'G':
				pstCtx->pszLogConfFile = optarg;
				break;
			case 'r':
				pstCtx->pszRundataTimerConf = optarg;
				break;
#endif

			case 'g':
				iIsGenerate	=	1;
				break;
			case 'u' :
				if (NULL != optarg)
					pstCtx->iBusinessID = strtol(optarg, NULL, 10);
				break;
			default:
				break;
			}

			break;

		case 'D':
			iIsDaemon	=	1;
			break;

		case 'v':
			/* show version. */

			printf("%s:version %lld.%lld.%lld  build at %lld.\n", pszApp, TAPP_GET_MAJOR(pstCtx->uiVersion), 
					TAPP_GET_MINOR(pstCtx->uiVersion), TAPP_GET_REV(pstCtx->uiVersion), TAPP_GET_BUILD(pstCtx->uiVersion));
			exit(0);

			break;
	
		case 'h':
			/* show help information. */

			if( pstCtx->pfnUsage )
				iRet	=	(*pstCtx->pfnUsage)(pstCtx->argc, pstCtx->argv);
			else
				iRet	=	0;

			if( 0==iRet )
				tapp_def_usage(pstCtx->argc, pstCtx->argv);

			exit(0);

			break;

		case '?':
			break;

		default:
			break;
		}
	}

		
	if (NULL != pstCtx->pszId)	
	{		
		pstCtx->iId	=	(int) inet_addr(pstCtx->pszId);		
	}
	if( tapp_init_id(pstCtx, pvArg)<0)
	{
		exit(-1);
	}

	/* process start/stop/refresh */
	for( opt=optind; opt<pstCtx->argc; opt++ )
	{
		if( 0==stricmp(pstCtx->argv[opt], TAPP_STOP) )
		{
			iRet	=	tos_send_signal( pstCtx->pszPidFile, pszApp, SIGUSR1, NULL );
			exit( iRet );
		}
		else if( 0==stricmp(pstCtx->argv[opt], TAPP_RELOAD) )
		{
			iRet	=	tos_send_signal( pstCtx->pszPidFile, pszApp, SIGUSR2, NULL );
			exit( iRet );
		}
		else if( 0==stricmp(pstCtx->argv[opt], TAPP_KILL) )
		{
			iRet	=	tos_kill_prev( pstCtx->pszPidFile, pszApp, pstCtx->iWait);
			exit( iRet );
		}
		else if( 0==stricmp(pstCtx->argv[opt], TAPP_START) )
		{
			iIsStart	=	1;
			break;
		}
	}

	gs_szConfBakFile[sizeof(gs_szConfBakFile)-1] = 0;
	snprintf(gs_szConfBakFile, sizeof(gs_szConfBakFile)-1, "%s.bak", pstCtx->pszConfFile);
	
	
	if(tapp_init_tdr(pstCtx, pvArg)<0 )
	{
		exit(-1);
	}

	if( iIsGenerate )
	{
		if (pstCtx->pfnGenerate)
			iRet = (*pstCtx->pfnGenerate)(pstCtx, pvArg);
		else
			iRet = tapp_do_generate(pstCtx, pvArg);			
		exit(iRet);
	}

	if( !iIsStart )
	{
		printf("Info: to start this service, use \'%s start\'(the keyword \'start\' must be appended ).\n", pszApp);
		exit(-2);
	}

	if( tapp_init_metabase(pstCtx, pvArg)<0 ||
		tapp_init_mib(pstCtx, pvArg)<0 )
	{
		exit(-1);
	}

	if( tapp_init_data(pstCtx, pvArg)<0 )
	{
		exit(-1);
	}

	/*可能使用到mib，所以放到mib初始化的后面*/
	iRet = tapp_init_log(pstCtx, pvArg);
	if(iRet)
	{
		printf("init log fail ret %d\n", iRet);
		exit(-1);
	}

	if( tapp_init_bus(pstCtx, pvArg)<0)
	{
		exit(-1);
	}

	tapp_init_rundata_timer(pstCtx, pvArg);


	if( iIsDaemon )
		tos_make_daemon(NULL);

	/* restore the getopt environment. */
	opterr	=	iOldOptErr;

	optarg	=	NULL;
	optind	=	1;
	optopt	=	'?';

	return 0;

}

TSF4G_API int tapp_def_init(TAPPCTX* pstCtx, void* pvArg)
{
	struct sigaction stAct;

	tapp_def_opt(pstCtx, pvArg);

	tos_init(NULL);
	tos_ignore_pipe();

	memset(&stAct, 0, sizeof(stAct));

	stAct.sa_handler	=	tapp_on_sigterm;

	sigaction(SIGTERM, &stAct, NULL);
	sigaction(SIGQUIT, &stAct, NULL);
	sigaction(SIGINT, &stAct, NULL);
	sigaction(SIGABRT, &stAct, NULL);

	stAct.sa_handler	=	tapp_on_sigusr1;
	sigaction(SIGUSR1, &stAct, NULL);

	stAct.sa_handler	=	tapp_on_sigusr2;
	sigaction(SIGUSR2, &stAct, NULL);

	if( tos_kill_prev(pstCtx->pszPidFile, pstCtx->pszApp, pstCtx->iWait)<0 )
		return -1;

	tos_write_pid(pstCtx->pszPidFile);

	if( pstCtx->pfnInit )
		return (*pstCtx->pfnInit)(pstCtx, pvArg);
	else
		return 0;
}

TSF4G_API int tapp_def_fini(TAPPCTX* pstCtx, void* pvArg)
{
	if( pstCtx->pfnFini )
		pstCtx->pfnFini(pstCtx, pvArg);	

	switch( pstCtx->iUseBus )
	{
	case TAPP_USEBUS_TBUS:

#ifdef TAPP_TBUS
		tbus_delete(&pstCtx->iBus);

		tbus_fini();
#endif /* TAPP_TBUS */

		break;

	case TAPP_USEBUS_OBUS:

#ifdef TAPP_OBUS
		if( pstCtx->iBus )
		{
			free((void*)pstCtx->iBus);
			pstCtx->iBus	=	0;
		}
#endif /* TAPP_OBUS */

		break;

	default:
		break;
	}

	tapp_fini_data(pstCtx, pvArg);

	tapp_fini_metabase(pstCtx, pvArg);

	tapp_fini_mib(pstCtx, pvArg);

	tos_fini();

	tapp_fini_log(pstCtx, pvArg);

	return 0;
}


TSF4G_API int tapp_def_reload(TAPPCTX* pstCtx, void* pvArg)
{
#if defined(TAPP_TDR) && defined(TAPP_TDR_XML)
	TDRDATA stHost;

	if( pstCtx->stConfData.pszBuff && pstCtx->stConfData.iMeta )
	{
		stHost.pszBuff	=	(char*)pstCtx->stConfData.pszBuff;
		stHost.iBuff	=	(int) pstCtx->stConfData.iLen;

		fprintf(stdout, "-------------------configuration data-------------------------------\n");
		tdr_output_fp((LPTDRMETA)pstCtx->stConfData.iMeta, stdout, &stHost, 0, 0);
		fprintf(stdout, "--------------------------------------------------------------------\n");
	}

	/*return 0;*/
#else /* TAPP_TDR */
	/*return 0;*/
#endif /* TAPP_TDR */

	return 0;
}

static int tapp_reload_conf(TAPPCTX* pstCtx, void* pvArg)
{
	int iRet;
	
	if (pstCtx->stConfData.pszBuff && pstCtx->stConfBackupData.pszBuff)
	{
		memcpy(pstCtx->stConfBackupData.pszBuff, pstCtx->stConfData.pszBuff, pstCtx->stConfBackupData.iLen);
	}

	if (0 == pstCtx->iUseMib && 0 == pstCtx->iNoLoadConf)
	{
		struct stat stStat;

		if (0 == stat(pstCtx->pszConfFile, &stStat))
		{
			if (pstCtx->tConfMod == stStat.st_mtime)
			{
				return 0;
			}

			pstCtx->tConfMod = stStat.st_mtime;
		}
		
		//put conf file info to ConfPrepareData
		if (pstCtx->stConfPrepareData.pszBuff && pstCtx->stConfPrepareData.iMeta 
			&& 0 > tapp_init_data_by_file(&pstCtx->stConfPrepareData, pstCtx->pszConfFile))
		{
			printf("reload conf file %s fail\n", pstCtx->pszConfFile);
			return -1;
		}
	}
	
	if( pstCtx->pfnReload )
	{
		iRet = (*pstCtx->pfnReload)(pstCtx, pvArg);
	}
	else
	{
		iRet = tapp_def_reload(pstCtx, pvArg);
	}
	
	if (pstCtx->stConfData.pszBuff && pstCtx->stConfBackupData.pszBuff && pstCtx->stConfBackupData.iMeta)
	{
		TDRDATA stHost;
	
		stHost.pszBuff	=	(char*)pstCtx->stConfBackupData.pszBuff;
		stHost.iBuff	=	(int) pstCtx->stConfBackupData.iLen;
	
		tdr_output_file((LPTDRMETA)pstCtx->stConfBackupData.iMeta, gs_szConfBakFile, &stHost, 0, 0);
	}

	return iRet;
}

static int tapp_reload_rundata_timer(TAPPCTX* pstCtx, void* pvArg)
{
	TAPP_RUNDATA_TIMER stPrepare;
	TAPPDATA stTappData;
		
	if (NULL == pstCtx->stRunDataTimer.pszBuff)
	{
		return -1;
	}

	if (0 == pstCtx->iUseMib)
	{
		struct stat stStat;

		if (0 == stat(gs_szRunDataTimerConfFile, &stStat))
		{
			if (pstCtx->tRunTimerConfMod == stStat.st_mtime)
			{
				return 0;
			}

			pstCtx->tRunTimerConfMod = stStat.st_mtime;
		}

		stTappData.pszBuff = (char *)&stPrepare;
		stTappData.iLen = pstCtx->stRunDataTimer.iLen;
		stTappData.iMeta = pstCtx->stRunDataTimer.iMeta;
		if (0 > tapp_init_data_by_file(&stTappData, gs_szRunDataTimerConfFile))
		{
			printf("cant load rundata timer conf file %s\n", gs_szRunDataTimerConfFile);
			return -1;
		}

		memcpy(pstCtx->stRunDataTimer.pszBuff, &stPrepare, pstCtx->stRunDataTimer.iLen);
	}
	
	return 0;
}

static int tapp_reload(TAPPCTX* pstCtx, void* pvArg)
{
	int iRet;

	iRet = tapp_reload_conf(pstCtx, pvArg);
	
	tapp_reload_log(pstCtx, pvArg);
	tapp_reload_rundata_timer(pstCtx, pvArg);

	return iRet;
}

static int tapp_rundata_log(TAPPDATA *pstTappData)
{
	char szMsg[4096];
	TDRDATA stOut;
	TDRDATA stHost;
	
	if (0 == pstTappData->iMeta || 0 == pstTappData->iLen || NULL == pstTappData->pszBuff)
		return -1;

	stOut.iBuff = sizeof(szMsg);
	stOut.pszBuff = szMsg;
	stHost.iBuff = pstTappData->iLen;
	stHost.pszBuff = pstTappData->pszBuff;
	
	if (0 > tdr_sprintf((LPTDRMETA)(pstTappData->iMeta), &stOut, &stHost, 0))
		return -1;

	tlog_info(gs_pstAppCatData, 0, 0, "%s", szMsg);
	
	return 0;
}

static int tapp_comm_tick(TAPPCTX* pstCtx, void* pvArg)
{
	int iRet = 0;
	static time_t tLastRefreshTbus = 0;
	
	assert(NULL != pstCtx);

#ifdef TAPP_TBUS
	/*定时刷新tbus配置*/
	if ((0 < pstCtx->iBus) &&
		((pstCtx->stCurr.tv_sec - tLastRefreshTbus) >= pstCtx->stOption.iRefeshTbusTimer))
	{
		tLastRefreshTbus = pstCtx->stCurr.tv_sec;
		iRet = tbus_refresh_handle(pstCtx->iBus);
	}
#endif

	return iRet;
}

TSF4G_API int tapp_def_mainloop(TAPPCTX* pstCtx, void* pvArg)
{
	TAPP_BASEDATA *pstBaseData;
	TAPP_RUNDATA_TIMER *pstRunDataTimer;
	struct timeval stSub;
	struct timeval stBaseDataLast;
	struct timeval stRunDataCumuLast;
	struct timeval stRunDataStatusLast;
	int iProcCostInit = 0;
	int iTickCostInit = 0;
	int iMs;
	int iRet=0;
	int iIdle=0;

	assert( pstCtx );

	pstBaseData = (TAPP_BASEDATA *)pstCtx->stBaseData.pszBuff;
	if (NULL == pstBaseData)
	{
		pstBaseData = calloc(1, sizeof(*pstBaseData));
	}

	pstRunDataTimer = (TAPP_RUNDATA_TIMER *)pstCtx->stRunDataTimer.pszBuff;
	
	gettimeofday(&pstCtx->stCurr, NULL);
	TV_CLONE(pstCtx->stLastTick, pstCtx->stCurr);
	TV_CLONE(stBaseDataLast, pstCtx->stCurr);
	TV_CLONE(stRunDataCumuLast, pstCtx->stCurr);
	TV_CLONE(stRunDataStatusLast, pstCtx->stCurr);

	while( !gs_iExitMainloop )
	{
		switch( gs_iIsExit )
		{
		case TAPP_EXIT_QUIT:
			
			if( pstCtx->pfnQuit )
				iRet	=	(*pstCtx->pfnQuit)(pstCtx, pvArg);
			else
				iRet	=	-1;

			if( iRet<0 )
				gs_iExitMainloop	=	1;

			break;
			
		case TAPP_EXIT_STOP:

			if( pstCtx->pfnStop )
				iRet	=	(*pstCtx->pfnStop)(pstCtx, pvArg);
			else
				iRet	=	-1;

			if( iRet<0 )
				gs_iExitMainloop	=	1;

			break;

		default:
			break;
		}

		if( gs_iReload )
		{
			gs_iReload	=	0;

			if ( 0 <= tapp_reload(pstCtx, pvArg))
			{
				printf("reload succ\n");
				tlog_info(gs_pstAppCatText, 0, 0, "reload succ");
			}
			else
			{
				printf("reload fail\n");
				tlog_info(gs_pstAppCatText, 0, 0, "reload fail");
			}
		}

		if( pstCtx->pfnProc )
		{
			struct timeval stEnd;
			struct timeval stStart;
			float fCost;

			gettimeofday(&stStart, NULL);
			iRet	=	(*pstCtx->pfnProc)(pstCtx, pvArg);
			gettimeofday(&stEnd, NULL);
			
			TV_DIFF(stSub, stEnd, stStart);
			fCost = stSub.tv_sec*1000 + stSub.tv_usec/1000.0;
			if (0 == iProcCostInit)
			{
				iProcCostInit = 1;
				pstBaseData->fProcMinTime = fCost;
				pstBaseData->fProcMaxTime = fCost;
			}
			
			if (fCost > pstBaseData->fProcMaxTime)
			{
				pstBaseData->fProcMaxTime = fCost;
			}
			else if (fCost < pstBaseData->fProcMinTime)
			{
				pstBaseData->fProcMinTime = fCost;
			}

			pstBaseData->fProcTotalTime += fCost;
		}
		else
			iRet	=	-1;

		if( iRet<0 )
		{
			iIdle++;

			pstBaseData->lIdleTotalNum++;
		}
		else
		{
			iIdle = 0;

			pstBaseData->lProcTotalNum++;
		}

		if( iIdle>pstCtx->iIdleCount )
		{
			iIdle	=	0;
			if (NULL == pstCtx->pfnIdle || 0 > (*pstCtx->pfnIdle)(pstCtx, pvArg))
			{
				usleep(pstCtx->iIdleSleep*1000);
			}
		}

		if( pstCtx->iTimer<=0)
			continue;

		gettimeofday(&pstCtx->stCurr, NULL);

		//AppTick
		TV_DIFF(stSub, pstCtx->stCurr, pstCtx->stLastTick);
		TV_TO_MS(iMs, stSub);
		if(( iMs > pstCtx->iTimer ) && (NULL != pstCtx->pfnTick))
		{
			struct timeval stEnd;
			float fCost;
			
			pstCtx->iTickCount++;
			pstBaseData->lTickTotalNum++;

			(*pstCtx->pfnTick)(pstCtx, pvArg);

			tapp_comm_tick(pstCtx, pvArg);
			gettimeofday(&stEnd, NULL);

			TV_DIFF(stSub, stEnd, pstCtx->stCurr);
			fCost = stSub.tv_sec*1000 + stSub.tv_usec/1000.0;
			if (0 == iTickCostInit)
			{
				iTickCostInit = 1;
				pstBaseData->fTickMinTime = fCost;
				pstBaseData->fTickMaxTime = fCost;
			}
			
			if (fCost > pstBaseData->fTickMaxTime)
			{
				pstBaseData->fTickMaxTime = fCost;
			}
			else if (fCost < pstBaseData->fTickMinTime)
			{
				pstBaseData->fTickMinTime = fCost;
			}

			pstBaseData->fTickTotalTime += fCost;

			TV_CLONE(pstCtx->stLastTick, pstCtx->stCurr);
		}

		//RunData Timer
		if (NULL == pstRunDataTimer)
			continue;
		
		TV_DIFF(stSub, pstCtx->stCurr, stBaseDataLast);
		TV_TO_MS(iMs, stSub);
		if (iMs > pstRunDataTimer->stBasedata_timer.lGlobalTime)
		{
			TV_CLONE(stBaseDataLast, pstCtx->stCurr);
			tapp_rundata_log(&pstCtx->stBaseData);
			memset(pstBaseData, 0, sizeof(*pstBaseData));
			iProcCostInit = iTickCostInit = 0;
		}

		TV_DIFF(stSub, pstCtx->stCurr, stRunDataCumuLast);
		TV_TO_MS(iMs, stSub);
		if (iMs > pstRunDataTimer->stRundata_cumu_timer.lGlobalTime)
		{
			TV_CLONE(stRunDataCumuLast, pstCtx->stCurr);
			tapp_rundata_log(&pstCtx->stRunDataCumu);
		}

		TV_DIFF(stSub, pstCtx->stCurr, stRunDataStatusLast);
		TV_TO_MS(iMs, stSub);
		if (iMs > pstRunDataTimer->stRundata_status_timer.lGlobalTime)
		{
			TV_CLONE(stRunDataStatusLast, pstCtx->stCurr);
			tapp_rundata_log(&pstCtx->stRunDataStatus);
		}
	}

	return gs_iIsExit;
}


/* log handler. */

/* extra helper function. */

TSF4G_API int tapp_is_exit(void)
{
	return gs_iIsExit;
}

TSF4G_API void tapp_exit_mainloop(void)
{
	gs_iExitMainloop	=	1;
}

TSF4G_API int tapp_get_category(const char* pszName, int* piCatInst)
{
	int iRet = -1;

#ifdef TAPP_TLOG

	TLOGCATEGORYINST* pstGet=NULL;

	if( !pszName || '\0'==pszName[0] )
		pszName	=	TLOG_DEF_CATEGORY_TEXTROOT;

	pstGet	=	tlog_get_category(&gs_stLogCtx, pszName);

	if( piCatInst )
		*piCatInst	=	(int)pstGet;

	iRet	=	pstGet ? 0 : -1;

#endif /* TAPP_TLOG */

	return iRet;
}



