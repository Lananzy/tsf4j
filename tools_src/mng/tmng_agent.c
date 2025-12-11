/**************************************************************
 @file $RCSfile: tmng_agent.c,v $
  tsf managing agent process
  $Id: tmng_agent.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
 @author $Author: kent $
 @date $Date: 2008/05/28 07:35:00 $
 @version 1.0
 @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
 @note Platform: Linux
 **************************************************************/
#include    <stdio.h>
#include    <stdlib.h>

#include    "tapp/tapp.h"
#include    "tbus/tbus.h"
#include    "mng/tmng.h"
#include    "comm/tini.h"
#include    "tdr/tdr.h"

#define     MNG_CFG_FILE        "tmng.conf"

typedef struct tagGlobal
{
    int     iAgentID;
    HTMNG   hMng;
    
    char    szMibKey[KEY_SIZE];
    char    szMetabaseKey[KEY_SIZE];
}   AGENT;  

static TAPPCTX  gs_stAppCtx={0};
static AGENT    gs_stAgent={0};

int get_mng_cfg(AGENT *a_pstAgent, char* a_szCfgFile)
{
    int iRet;

    iRet = tini_read (a_szCfgFile, "MIB", "Key", a_pstAgent->szMibKey, sizeof(a_pstAgent->szMibKey));
    if( iRet )  
        strcpy(  a_pstAgent->szMibKey, DEFAULT_MIB_KEY ); 

    
    iRet = tini_read (a_szCfgFile, "METABASE", "Key", a_pstAgent->szMetabaseKey, sizeof(a_pstAgent->szMetabaseKey));
    if( iRet )  
        strcpy(  a_pstAgent->szMetabaseKey, DEFAULT_METABASE_KEY ); 

    return 0;
}

int tmagent_init (TAPPCTX* pstAppCtx, AGENT* pstAgent)
{
    int             i, iRet;
    HTMNG           hMng = NULL;
    LPHOSTDEPLOY    pstDeploy = NULL;
    char*           pchDeploy = NULL;
    TMIBDATA stData;
    LPMNGPROCATTR   pstProcAttr;

    if ( NULL==pstAppCtx->pszMibKey || NULL==pstAppCtx->pszMBaseKey )
    {
        get_mng_cfg(pstAgent, MNG_CFG_FILE);
    }
    else 
    {
        strcpy(pstAgent->szMibKey, pstAppCtx->pszMibKey);
        strcpy(pstAgent->szMetabaseKey, pstAppCtx->pszMBaseKey);
    }

    iRet = tmng_init(&pstAgent->hMng, pstAppCtx->argc, pstAppCtx->argv, 0, 
                                   pstAgent->szMibKey, pstAgent->szMetabaseKey);
    if ( iRet || NULL== pstAgent->hMng )
    {   
        printf( "tmng_init return %d\n", iRet);
        return -1;
    }

    hMng = pstAgent->hMng;
    //tmib_dump_all(hMng->hMib, pstAgent->szMetabaseKey, stdout);
    iRet = tmng_get_cfg(hMng, DEPOLY_CFG, 0,  &pchDeploy);
    if ( iRet )
    {   
        printf( "tmng_get_cfg return %d\n", iRet);
        return -1;
    }

    pstDeploy = (LPHOSTDEPLOY)pchDeploy;
    
    printf("agentid=%s, procnum=%d\n", pstDeploy->szAgentId, pstDeploy->iProcNum );
      
    pstAgent->iAgentID = inet_addr(pstDeploy->szAgentId);

    for ( i=0; i<pstDeploy->iProcNum; i                                                                                                                       ++)
    {
        printf("%s %s %s %s \n", 
                pstDeploy->astProcessList[i].szProcID, 
                pstDeploy->astProcessList[i].szProcName, 
                pstDeploy->astProcessList[i].szWorkDir,
                pstDeploy->astProcessList[i].szStartCmd);

        memset(&stData, 0, sizeof(TMIBDATA));

        strcpy( stData.szDomain, PROC_LIST_DOMAIN);
        strcpy( stData.szName,  pstDeploy->astProcessList[i].szProcID);
        strcpy( stData.szLib,   MNG_LIB);
        strcpy( stData.szMeta,  PROC_ATTR);
        stData.iSize = sizeof(MNGPROCATTR);
        
        iRet = tmib_register_data(hMng->hMib, &stData);
        if ( iRet )   
        {  
            printf("tmib_register_data return %d.", iRet);
            return -2;
        }
        tmib_get_data(hMng->hMib, &stData, 0);
        pstProcAttr = (MNGPROCATTR*)stData.pszData;
        memcpy( pstProcAttr, &pstDeploy->astProcessList[i], sizeof(MNGPROCATTR));
    }

    if ( pstAgent->iAgentID )
    {
	    if (0 > (iRet = tbus_init(0, 0)))
	    {
		    printf("tbus_init fail %d\n", iRet);
		    return -1;
	    }
    	if (0 > (iRet =tbus_new(&pstAppCtx->iBus)))
    	{
    		printf("tbus_new fail %d\n", iRet);
    		return -1;
    	}
    	if( 0 > (iRet = tbus_bind(pstAppCtx->iBus, pstAgent->iAgentID)) )
    	{
    		printf("tbus_bind fail %d\n", iRet);
    		return -1;
    	}    
    }    
    return 0;
}


int tmagent_proc(TAPPCTX *pstAppCtx,  AGENT* pstAgent)
{
	int         i, iRet = -1;
    static char sBuf[1024];
    int         iSrc, iDst, iSize;
    LPMNGPKG    pstPkg=NULL;
        
    for (i=0;i<50;i++)
    {
        iSrc = -1;
        iDst = 0;
        iSize= sizeof(sBuf);
    	iRet = tbus_recv(pstAppCtx->iBus, &iSrc, &iDst, sBuf, (size_t *)&iSize, 0);
        //if ( !iRet )    printf( "Agent recv from %s %d:: %s\n", inet_ntoa(*(struct in_addr *)&iSrc), iSize, sBuf);

        if ( !iRet )
        {
            pstPkg = (LPMNGPKG) sBuf;
            if ( pstPkg->stHead.bMsgType == MNG_ALERT )
            {
                  printf( "Agent recv from %s %d:: %s\n", inet_ntoa(*(struct in_addr *)&iSrc), iSize, pstPkg->stBody.stAlert.szTitle);
            }
        }
    }
	return iRet;
}


int tmagent_fini (TAPPCTX* pstAppCtx, AGENT* pstAgent)
{
    return tmng_fini(&pstAgent->hMng );
}

int main(int argc,char ** argv)
{
    int             iRet;

	gs_stAppCtx.argc	=	argc;
	gs_stAppCtx.argv	=	argv;

	gs_stAppCtx.iTimer	=	100;

	gs_stAppCtx.pfnInit	=	(PFNTAPPFUNC)tmagent_init;
	gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)tmagent_fini;
	gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)tmagent_proc;
	gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)NULL;
	gs_stAppCtx.pfnReload	=	(PFNTAPPFUNC)NULL;


	iRet = tapp_def_init(&gs_stAppCtx, &gs_stAgent);
	if( iRet<0 )
	{
		printf("Error: Initialization failed.\n");
		return iRet;
	}
    
	iRet = tapp_def_mainloop(&gs_stAppCtx, &gs_stAgent);

	tapp_def_fini(&gs_stAppCtx, &gs_stAgent);

    return 0;
}
