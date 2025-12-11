/*
**  @file RCSfile
**  general description of this module
**  Id
**  @author Author
**  @date Date
**  @version Revision
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/
#include "tapp/tapp.h"
#include "tdr/tdr.h"
#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"
#include "test_logdump_conf_desc.h"
#include "version.h"
#include "logic_log.h"

extern unsigned char g_szMetalib_logic_log[];


struct tagtest_logdumpEnv
{
	TEST_LOGDUMPCONF *pstConf;
	TEST_LOGDUMPRUN_CUMULATE *pstRunCumu;
	TEST_LOGDUMPRUN_STATUS   *pstRunStat;
	TLOGCATEGORYINST* pstLogCat;
	TLOGCATEGORYINST* pstOpLogCat;
	TLOGCONF stOpLogConf;
	TLOGCTX stOpLogCtx;
	LPTDRMETA pstLogicLogMeta;
};

typedef struct tagtest_logdumpEnv		TEST_LOGDUMPENV;
typedef struct tagtest_logdumpEnv		*LTEST_LOGDUMPENV;


static TAPPCTX gs_stAppCtx;
extern unsigned char g_szMetalib_test_logdump[];

TEST_LOGDUMPENV gs_stEnv;
static TEST_LOGDUMPRUN_CUMULATE gs_stCumu;
static TEST_LOGDUMPRUN_STATUS gs_stStat;

int test_logdump_init(TAPPCTX *pstAppCtx, TEST_LOGDUMPENV *pstEnv)
{
	if (NULL == pstAppCtx->stConfData.pszBuff || 0 == pstAppCtx->stConfData.iMeta 
		|| NULL == pstAppCtx->pszConfFile )
	{
		return -1;
	}
	pstEnv->pstConf = (TEST_LOGDUMPCONF *)pstAppCtx->stConfData.pszBuff;

	pstEnv->pstRunCumu = (TEST_LOGDUMPRUN_CUMULATE *)pstAppCtx->stRunDataCumu.pszBuff;
	if (NULL == pstEnv->pstRunCumu)
	{
		pstEnv->pstRunCumu = &gs_stCumu;
	}
	
	pstEnv->pstRunStat = (TEST_LOGDUMPRUN_STATUS *)pstAppCtx->stRunDataStatus.pszBuff;
	if (NULL == pstEnv->pstRunStat)
	{
		pstEnv->pstRunStat = &gs_stStat;
	}

	if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, (int *)(&pstEnv->pstLogCat)))	
	{		
		printf("tapp_get_category run fail\n");
		return -1;	
	}

	if (0 > tlog_init_cfg_from_file(&pstEnv->stOpLogConf, pstEnv->pstConf->szOpLogCfg))
	{
		printf("load op log cfg file:%s fail\n", pstEnv->pstConf->szOpLogCfg);
		return -1;
	}
	if (0 > tlog_init(&pstEnv->stOpLogCtx, &pstEnv->stOpLogConf))
	{
		printf("load op log cfg file:%s fail\n", pstEnv->pstConf->szOpLogCfg);
		return -1;
	}
	pstEnv->pstOpLogCat = tlog_get_category(&pstEnv->stOpLogCtx, "oplog");
	if (NULL == pstEnv->pstOpLogCat)
	{
		printf("load op log cfg file:%s fail\n", pstEnv->pstConf->szOpLogCfg);
		return -1;
	}

	srand(pstAppCtx->stCurr.tv_sec);

	pstEnv->pstLogicLogMeta = tdr_get_meta_by_name((LPTDRMETALIB)g_szMetalib_logic_log, "logic_log");
	if (NULL == pstEnv->pstLogicLogMeta)
	{
		return -1;
	}
	
	printf("test_logdump start\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "test_logdump start");

	return 0;
}

int test_logdump_proc(TAPPCTX *pstAppCtx, TEST_LOGDUMPENV *pstEnv)
{
	int iBusy=0;
	
	if (iBusy)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int test_logdump_reload(TAPPCTX *pstAppCtx, TEST_LOGDUMPENV *pstEnv)
{
	TEST_LOGDUMPCONF   *pstPreConf;
	
	pstPreConf = (TEST_LOGDUMPCONF   *)pstAppCtx->stConfPrepareData.pszBuff;
	
	printf("test_logdump reload\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "test_logdump reload");

	return 0;
}

int test_logdump_fini(TAPPCTX *pstAppCtx, TEST_LOGDUMPENV *pstEnv)
{

	printf("test_logdump finish\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "test_logdump finish");
	return 0;
}

int test_logdump_tick(TAPPCTX *pstAppCtx, TEST_LOGDUMPENV *pstEnv)
{
	/*
	char szBuff[17];
	int i;
	int iLen;
	char c;

	iLen = rand()%16+1;
	c = rand()%26+'a';
	memset(szBuff, c, iLen);
	szBuff[iLen] = 0;
	
	for (i=0; i<8; i++)
	{
		tlog_info(pstEnv->pstOpLogCat, 0, 0, "%s\n", szBuff);
	}*/

	LOGIC_LOG stLogicLog;
	char sBuff[2*sizeof(LOGIC_LOG)];
	TDRDATA stHost;
	TDRDATA stNet;

	stLogicLog.iLogType = LOG_ITEM_PICK;
	stLogicLog.stLog.stItemPick.iItemID = 2081;
	stLogicLog.stLog.stItemPick.iRole = 42333;
	stNet.iBuff = sizeof(sBuff);
	stNet.pszBuff = sBuff;
	stHost.iBuff = sizeof(stLogicLog);
	stHost.pszBuff = (char *)&stLogicLog;

	if (0 > tdr_hton(pstEnv->pstLogicLogMeta, &stNet, &stHost, TDR_METALIB_LOGIC_LOG_VERSION))
	{
		return -1;;
	}

	tlog_info_bin(pstEnv->pstOpLogCat, 42333, 8, 10, TDR_METALIB_LOGIC_LOG_VERSION, stNet.pszBuff, stNet.iBuff);

	return 0;
}

int test_logdump_stop(TAPPCTX *pstAppCtx, TEST_LOGDUMPENV *pstEnv)
{
	int iReady=0;
	
	iReady = 1;
	if (iReady)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int main(int argc, char* argv[])
{
	int iRet;
	void* pvArg	=	&gs_stEnv;

	memset(&gs_stAppCtx, 0, sizeof(gs_stAppCtx));
	memset(&gs_stEnv, 0, sizeof(gs_stEnv));

	gs_stAppCtx.argc	=	argc;
	gs_stAppCtx.argv	=	argv;

	gs_stAppCtx.pfnInit	=	(PFNTAPPFUNC)test_logdump_init;
	
	gs_stAppCtx.iLib = (int)g_szMetalib_test_logdump;
	gs_stAppCtx.stConfData.pszMetaName = "test_logdumpconf";
	gs_stAppCtx.uiVersion = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);
		
	iRet	=	tapp_def_init(&gs_stAppCtx, pvArg);
	if( iRet<0 )
	{
		printf("Error: app Initialization failed.\n");
		return iRet;
	}

	gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)test_logdump_fini;
	gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)test_logdump_proc;
	gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)test_logdump_tick;
	gs_stAppCtx.pfnReload	=	(PFNTAPPFUNC)test_logdump_reload;
	gs_stAppCtx.pfnStop  =   (PFNTAPPFUNC)test_logdump_stop;
	
	iRet	=	tapp_def_mainloop(&gs_stAppCtx, pvArg);

	tapp_def_fini(&gs_stAppCtx, pvArg);
	
	return iRet;
}


