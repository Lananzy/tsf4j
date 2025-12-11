#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "pal/pal.h"
#include "tdr/tdr.h"

#include "tapp/tapp.h"
#include "tbus/tbus.h"

#include "pal/tree.h"
#include "pal/queue.h"
#include "taa/tagentapi.h"

#include "apps/centerdapi/centerdapi.h"

#include "tconnddef.h"
#include "tconndkeymgr_conn.h"
#include "tconndkeymgr_proc.h"

extern unsigned char g_szMetalib_tconnd[];

static TAPPCTX		gs_stAppCtx;
static RSYSENV		gs_stSysEnv;
static SERVER			gs_stServer;
LPTLOGCATEGORYINST	g_pstLogCat;

static int bus_msg_dispatch(TAPPCTX* pstAppCtx, void* pstEnv)
{
	int			iRet;
	int			iSize;
	int			iSrc;

	TDRDATA	stHost;
	TDRDATA	stNet;
	
	char			buffer[1024 * 128];
	
	LPRSYSENV	pstSysEnv = (LPRSYSENV)pstEnv;
	tagPkgHead 	stHead;

	TCONNDINNERMSG	stInnerMsg;
	
	(void)pstAppCtx;
	iSrc		=	-1;
	iSize		=	sizeof(buffer);
	
	iRet		=	centerdapi_recv(pstSysEnv->iHandle, &iSrc, buffer, &iSize, &stHead);	
	if (iRet < 0 || iSize <= 0)
	{
		return -1;
	}
	
	stNet.pszBuff	=	buffer;
	stNet.iBuff	=	iSize;
	
	stHost.pszBuff	=	(char *)&stInnerMsg;
	stHost.iBuff	=	sizeof(stInnerMsg);
	
	iRet 			=	tdr_ntoh(pstSysEnv->pstServer->pstMeta, &stHost, &stNet, 0);
	if (iRet)
	{
		tlog_log(g_pstLogCat, TLOG_PRIORITY_ERROR, 0,0, "tdr_ntoh:%s", tdr_error_string(iRet));
		return -1;
	}
	
	switch (stInnerMsg.stHead.wCmdID)
	{
		case ID_MSG_TCONND_REPORT:		return process_0x30(pstSysEnv, pstSysEnv->pstServer, &stHead, &stInnerMsg);
		case ID_MSG_TCONND_ASKREPORT :	return 0;
		case ID_MSG_TCONND_UPDATE:		return 0;
		
		default:;
	}
	
	return 0;
}

int tconndkeymgr_init(TAPPCTX* pstAppCtx, void* pstEnv)
{
	int			iRet;
	TAPPDATA	stAppData;
	TDRDATA	stHost;
	
	LPRSYSENV 	pstSysEnv = (LPRSYSENV)pstEnv;
	SERVER *	pstSrv;
	
	tapp_get_category(NULL, (int*)&g_pstLogCat);
	if( !g_pstLogCat )
	{
		printf("error: can not get default log category.\n");
		return -1;
	}
	
	if( !pstAppCtx->stConfData.iMeta )
	{
		printf("error: can not find confdata meta \'%s\'.\n", pstAppCtx->stConfData.pszMetaName);
		return -1;
	}
	
	stAppData.pszMetaName			= "tconndkeymgr";
	stAppData.iMeta					= (int) tdr_get_meta_by_name((LPTDRMETALIB)pstAppCtx->iLib, stAppData.pszMetaName);
	
	assert (stAppData.iMeta);
	
	if(stAppData.iMeta)
	{
		stAppData.iLen				= tdr_get_meta_size((LPTDRMETA)stAppData.iMeta);
		stAppData.pszBuff				= (char *)malloc(stAppData.iLen);
			
		assert (stAppData.pszBuff);
		
		stHost.pszBuff					=	stAppData.pszBuff;
		stHost.iBuff					=	stAppData.iLen;
		
		iRet	=	tdr_input_file((LPTDRMETA)stAppData.iMeta, &stHost, pstAppCtx->pszConfFile, 0, 0);
		if (iRet)
		{
			printf ("load conf file %s error: %s\n", pstAppCtx->pszConfFile, tdr_error_string(iRet));
			return -1;
		}
		
		pstSysEnv->pstTconndKeyMgr	=	(TCONNDKEYMGR *)stHost.pszBuff;
		pstSysEnv->pstServer 			=	&gs_stServer;	
		pstSrv 						=	pstSysEnv->pstServer;
		pstSrv->pstConfinst 			=	pstSysEnv->pstTconndKeyMgr;
	}
	
	pstSrv->stBusidList.pszMetaName	= "BusidList";
	pstSrv->stBusidList.iMeta			= (int) tdr_get_meta_by_name((LPTDRMETALIB)pstAppCtx->iLib, pstSrv->stBusidList.pszMetaName);
	if(pstSrv->stBusidList.iMeta)
	{
		pstSrv->stBusidList.iLen 		= tdr_get_meta_size((LPTDRMETA)pstSrv->stBusidList.iMeta);
		pstSrv->stBusidList.pszBuff 		= (char *)malloc(pstSrv->stBusidList.iLen);
		
		assert (pstSrv->stBusidList.pszBuff);
	}
	
	pstSrv->stSvrskeyList.pszMetaName	= "SvrsKeyList";
	pstSrv->stSvrskeyList.iMeta			= (int) tdr_get_meta_by_name((LPTDRMETALIB)pstAppCtx->iLib, pstSrv->stSvrskeyList.pszMetaName);
	if(pstSrv->stSvrskeyList.iMeta)
	{
		pstSrv->stSvrskeyList.iLen 		= tdr_get_meta_size((LPTDRMETA)pstSrv->stSvrskeyList.iMeta);
		pstSrv->stSvrskeyList.pszBuff	= (char *)malloc(pstSrv->stSvrskeyList.iLen);
		
		assert (pstSrv->stSvrskeyList.pszBuff);
	}
	
	pstSrv->pstMeta					= tdr_get_meta_by_name((LPTDRMETALIB)pstAppCtx->iLib, "TconndinnerMsg");
	assert (pstSrv->pstMeta);
	
	if (centerdapi_initialize(pstAppCtx->pszGCIMKey, pstAppCtx->iBusinessID))
	{
		printf ("mailapi_initialize error\n");
		return -1;
	}
	
	if (centerdapi_create(pstAppCtx->iId, &pstSysEnv->iHandle, ID_APPID_TCONND))
	{
		printf ("mailapi_create error\n");
		return -1;
	}
	
	pstSysEnv->iDst	=	pstSrv->pstConfinst->ulCenterdAddr;
	if (centerdapi_connect(pstSysEnv->iHandle, pstSysEnv->iDst))
	{
		printf ("mailapi_connect error\n");
		return -1;
	}
	
	/* register2centerd */
	if (centerdapi_register(pstSysEnv->iHandle, pstSysEnv->iDst))
	{
		printf ("centerdapi_register error\n");
	}
	
	/* connection init */
	if (tconndkeymgr_connection_init(pstSrv) < 0)
	{
		printf("error: tconndkeymgr_connection_init.\n");
		return -1;
	}
	
	if (readXMLData(pstAppCtx, pstSrv) < 0)
	{
		printf("Error: readXMLData.\n");
		return -1;
	}
	
	tlog_log(g_pstLogCat, TLOG_PRIORITY_FATAL, 0,0, "tconndkeymgr start\n");
	
	return 0;
}

int tconndkeymgr_fini(TAPPCTX* pstAppCtx, void* pstEnv)
{
 	(void)pstAppCtx;
	LPRSYSENV pstSysEnv = (LPRSYSENV)pstEnv;	
	tconndkeymgr_connection_free(pstSysEnv->pstServer);
	return 0;
}

int tconndkeymgr_proc(TAPPCTX* pstAppCtx, void* pstEnv)
{
	int 			iRet = 0;
	LPRSYSENV	pstSysEnv = (LPRSYSENV)pstEnv;

	
	iRet 		=	bus_msg_dispatch(pstAppCtx, pstEnv);
	
	tconndkeymgr_connection_poll(pstSysEnv->pstServer);
	
	return iRet;
}

int tconndkeymgr_tick(TAPPCTX* pstAppCtx, void* pstEnv)
{
	static time_t tsPeriod 	= 0;
	LPRSYSENV pstSysEnv	= (LPRSYSENV)pstEnv;
	
	if (pstAppCtx->stCurr.tv_sec - tsPeriod > 10)
	{
		tsPeriod	=	pstAppCtx->stCurr.tv_sec;
		centerdapi_register(pstSysEnv->iHandle, pstSysEnv->iDst);
		
		/* flush data to xml */
		writeXMLData(pstAppCtx, pstSysEnv->pstServer);
	}
	
	return 0;
}

int tconndkeymgr_reload(TAPPCTX* pstAppCtx, void* pstEnv)
{
	(void)pstAppCtx;
	(void)pstEnv;
	
	return 0;
}

int main(int argc, char* argv[])
{
	int iRet;
	void* pvArg				= &gs_stSysEnv;

	gs_stAppCtx.argc			= argc;
	gs_stAppCtx.argv			= argv;
	
	gs_stAppCtx.iTimer		= 100;

	gs_stAppCtx.pfnInit		= (PFNTAPPFUNC)tconndkeymgr_init;
	gs_stAppCtx.pfnFini		= (PFNTAPPFUNC)tconndkeymgr_fini;
	gs_stAppCtx.pfnProc		= (PFNTAPPFUNC)tconndkeymgr_proc;
	gs_stAppCtx.pfnTick		= (PFNTAPPFUNC)tconndkeymgr_tick;
	gs_stAppCtx.pfnReload	= (PFNTAPPFUNC)tconndkeymgr_reload;
	gs_stAppCtx.iLib			= (int)g_szMetalib_tconnd;
	
	iRet = tapp_def_init(&gs_stAppCtx, pvArg);
	if (iRet < 0)
	{
		printf("Error: Initialization failed.\n");
		return iRet;
	}
	
	iRet = tapp_def_mainloop(&gs_stAppCtx, pvArg);

	tapp_def_fini(&gs_stAppCtx, pvArg);

	return 0;
}
