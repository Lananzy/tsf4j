/*
**  @file $RCSfile: tserver.c,v $
**  general description of this module
**  $Id: tserver.c,v 1.3 2009/03/25 09:20:55 hardway Exp $
**  @author $Author: hardway $
**  @date $Date: 2009/03/25 09:20:55 $
**  @version $Revision: 1.3 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/
#include "tserver.h"


#define MSGNUM 500
static TAPPCTX gs_stAppCtx;
static TSERVERENV gs_stEnv;
extern unsigned char g_szMetalib_CSPKG[];



int tserver_init(TAPPCTX* pstAppCtx, TSERVERENV* pstEnv)
{
       int iRet=0;

	if( !pstAppCtx->pszId )
	{
		printf("error: no specified server id!\n");
		return -1;
	}

	if(!pstEnv->iConndID)
	{
              printf("error:no specified connd id!\n");
	       return -1;

	}
	
        iRet=tconnapi_init(atoi(pstAppCtx->pszGCIMKey));
	if(iRet !=0 )
	{
	      printf("error:init connd api failed!\n");
	      return -1;
	}

	iRet=tconnapi_create((int)inet_addr(pstAppCtx->pszId), &pstEnv->iConHandle);

	if( iRet != 0 )
	{
            printf("error:create connd api handle failed!\n");
	     return -1;
	}

	iRet=tconnapi_connect(pstEnv->iConHandle, pstEnv->iConndID);
	if(iRet!=0)
	{
           printf("error:connect to connd failed,ConndID=%s,errorstring=%s\n",inet_ntoa(*(struct in_addr *)&pstEnv->iConndID),tdr_error_string(iRet));
	    return -1;
	}

	//get CSPkg meta
	pstEnv->pstCSPkgMeta=tdr_get_meta_by_name((LPTDRMETALIB)pstAppCtx->iLib, CSPKGNAME);
	if(!pstEnv->pstCSPkgMeta)
	{
           printf("error:get cspkg meta failed!\n");
	    return -1;
	}

	iRet=tplayer_init(&pstEnv->pstPlayerPool);
	if(iRet != 0)
	{
            printf("init play mempool failed!\n");
	     return -1;
	}

	//get log category
	pstEnv->pstLog	=	NULL;
	tapp_get_category(NULL, (int*)&pstEnv->pstLog);
	if(!pstEnv->pstLog)
	{
             printf("error: can not get log category.\n");
	      return -1;

	}	

	if( pstEnv->iEchoFactor<=0 )
	{
           pstEnv->iEchoFactor=1;  
	}
	else if( pstEnv->iEchoFactor > MAX_ECHOFACTOR)
	{
           pstEnv->iEchoFactor=MAX_ECHOFACTOR;
	}

       tlog_info(pstEnv->pstLog, 0, 0, "initiaion success\n");
	return 0;
}

int tserver_fini(TAPPCTX* pstAppCtx, TSERVERENV* pstEnv)
{
	tconnapi_free(&pstEnv->iConHandle);

	tconnapi_fini();
     
	tplayer_fini(&pstEnv->pstPlayerPool);

	return 0;
}

int tserver_tick(TAPPCTX* pstAppCtx, TSERVERENV* pstEnv)
{
      return 0;
}



int tserver_proc(TAPPCTX* pstAppCtx, TSERVERENV* pstEnv)
{
       
	int iSrc = 0;
	int iLen;
	TFRAMEHEAD stFrameHead;
	int iRet=0;
	int iMsgNum=0;


	iLen	=	(int)sizeof(pstEnv->szBuffer);

	while( iMsgNum < MSGNUM )
	{
           iRet = tconnapi_recv(pstEnv->iConHandle, &iSrc, pstEnv->szBuffer, &iLen, &stFrameHead);

            if(iRet<0)
            {
               
		  return -1;
	     }

	      iMsgNum++;		
	    
		 
            //client msg
	     if( iSrc == pstEnv->iConndID)
	     {
                 //deal with package
                  tserver_deal_client_msg(pstEnv, &stFrameHead,pstEnv->szBuffer,iLen); 	  
	     }
	    
	}

	return iMsgNum-1;
}

int tserver_deal_client_msg(TSERVERENV* pstEnv,TFRAMEHEAD *pstFrame,char *pszbuff,int ibuff)
{
       int iRet=0;
	TDRDATA stHost;
	TDRDATA stNet;
	LPPlayer pstPlayer;

	   
	switch(pstFrame->chCmd)
	{
	     case TFRAMEHEAD_CMD_START:
		  iRet=tplayer_open_connection(pstEnv, pstFrame);
	         return iRet;
	     case TFRAMEHEAD_CMD_STOP:
		  iRet=tplayer_close_connection(pstEnv,pstFrame);
		  return iRet;   
	     case TFRAMEHEAD_CMD_RELAY:
		  //to do
		   return iRet;
	     case TFRAMEHEAD_CMD_NOTIFY:
		 //to do
		 return iRet;
	     case TFRAMEHEAD_CMD_INPROC:
		  //deal with package body
		  break;
	     default:
		  tlog_error(pstEnv->pstLog, 0, 0, "unknown comdiD=%d",pstFrame->chCmd);
	         return -1;		
	}

	//deal with package body
	if( ibuff > 0  )
       {
            //get player   
	     stHost.pszBuff=(char *)&pstEnv->stpkg;
	     stHost.iBuff=sizeof(pstEnv->stpkg);
	     stNet.pszBuff=pszbuff;
	     stNet.iBuff=ibuff;
            iRet= tdr_ntoh(pstEnv->pstCSPkgMeta,&stHost,&stNet,0);
	     if(iRet != 0)
	     {
                tlog_error(pstEnv->pstLog, 0, 0, "convert net cspkg to host failed!errorstring-%s",tdr_error_string(iRet));
		  return iRet;		
	     }

	     iRet=tserver_deal_client_pkg(pstEnv, pstFrame,&pstEnv->stpkg);

	}
	
       return iRet;
}

int tserver_deal_client_pkg(TSERVERENV* pstEnv,TFRAMEHEAD *pstFrame,CSPKG *pstPkg)
{
      int iRet=0;

      switch(pstPkg->stHead.wCmd)
      {
          case CMD_LOGIN:
          case CMD_LOGOUT:
   		  tlog_error(pstEnv->pstLog, 0, 0, "undealed msg type cmd=%d",pstPkg->stHead.wCmd);
		  return -1;   	
          case CMD_HELLO:
		  //echo hello package	
		  iRet=tserver_deal_hello_pkg(pstEnv,pstFrame,pstPkg);
		  break;
      }

      return iRet;
}
int tserver_deal_hello_pkg(TSERVERENV* pstEnv,TFRAMEHEAD *pstFrame,CSPKG *pstPkg)
{
     int iRet=0;
     LPPlayer pstPlayer=NULL;
     int i=0;

     pstPlayer=tplayer_get(pstEnv->pstPlayerPool, pstFrame->iID);
     if(!pstPlayer)
     {
          tlog_error(pstEnv->pstLog, 0, 0, "player id do net exsit!iID=%d",pstFrame->iID);
          return -1;
     }

    for(i=0; i<pstEnv->iEchoFactor; i++)
    {
          iRet=tserver_send_client_pkg(pstEnv, &pstPlayer->stFrame, pstPkg);

	    if(iRet != 0)
	    {
	          tlog_error(pstEnv->pstLog, 0, 0, "send hello package failed!uin=%d\n",pstPlayer->uin);
	          return iRet;
	    }
    }
     
    
	
    return 0;
}

int tplayer_open_connection(TSERVERENV* pstEnv,TFRAMEHEAD *pstFrame)
{
    int iRet=0;
    int iUin=0;
    int iID=0;
    LPPlayer pstPlayer=NULL;

    if(pstFrame->chCmd!=TFRAMEHEAD_CMD_START)
    {
         return -1;
    }

    iUin=pstFrame->stCmdData.stStart.stAuthData.stAuthQQUnified.lUin;
    
    //to do:check whether uin has login before 
   iID=tplayer_alloc(pstEnv->pstPlayerPool);
	
   if(iID<0)
   {
         tlog_error(pstEnv->pstLog, 0, 0, "allocate player buffer failed!uin=%d",iUin);
         return -1;
   }

   pstPlayer=tplayer_get(pstEnv->pstPlayerPool,iID);
   if(!pstPlayer)
   {
         tlog_error(pstEnv->pstLog, 0, 0, "get player from mempool failed!iID=%d",iID);
         return -1;
   }
   
   pstPlayer->uin = iUin;
   pstPlayer->iID = iID;
   memcpy(&pstPlayer->stFrame,pstFrame,sizeof(pstPlayer->stFrame));

   pstPlayer->stFrame.iID=pstPlayer->iID;

   //send start package head response
   iRet=tserver_send_client_pkg(pstEnv,&pstPlayer->stFrame,NULL);
   if( iRet != 0)
   {
         tlog_error(pstEnv->pstLog, 0, 0, "send start reponse failed!uin=%d",iUin);
         return -1;
   }

   //save global frame data
    pstPlayer->stFrame.chCmd=TFRAMEHEAD_CMD_INPROC;
    pstPlayer->stFrame.stCmdData.stInProc.chValid=1;
    pstPlayer->stFrame.stCmdData.stInProc.nCount=1;
    pstPlayer->stFrame.stCmdData.stInProc.astIdents[0].iConnIdx=pstPlayer->stFrame.iConnIdx;
    pstPlayer->stFrame.stCmdData.stInProc.astIdents[0].iID=iID;

    if(pstEnv->iNoEncrypt)
    {
          pstPlayer->stFrame.stCmdData.stInProc.chNoEnc=1;
    }
    else
    {

          pstPlayer->stFrame.stCmdData.stInProc.chNoEnc=0;
    }

    return 0;
}

int tplayer_close_connection(TSERVERENV* pstEnv,TFRAMEHEAD *pstFrame)
{
    int iRet=0;
    int iID=pstFrame->iID;
    LPPlayer pstPlay=NULL;

    pstPlay=tplayer_get(pstEnv->pstPlayerPool, iID);

    if(!pstPlay)
    {
         tlog_error(pstEnv->pstLog, 0, 0, "can not find player for close!iID=%d",iID);
         return -1;     
    }

     iRet=tplayer_free(pstEnv->pstPlayerPool, iID);
     if(iRet!=0)
     {
         tlog_error(pstEnv->pstLog, 0, 0, "free player failed!iID=%d",iID);
         return iRet; 
     }
	 
      return 0;	
}


int tserver_send_client_pkg(TSERVERENV* pstEnv,TFRAMEHEAD *pstFrame,CSPKG *pstPkg)
{
     int iRet=0;
     TDRDATA stHost;
     TDRDATA stNet;

     if(pstPkg)
     {
           stHost.pszBuff=(char *)pstPkg;
	    stHost.iBuff=sizeof(CSPKG);
	    stNet.pszBuff=pstEnv->szBuffer;
	    stNet.iBuff=sizeof(pstEnv->szBuffer);
	    iRet=tdr_hton(pstEnv->pstCSPkgMeta, &stNet, &stHost, 0);
	    if(iRet != 0)
	    {
                 tlog_error(pstEnv->pstLog,0,0,"convert cspkg host to net failed!");
		   return iRet;
	    }
     }
     else
     {
          stNet.iBuff=0;

     }

     iRet=tconnapi_send(pstEnv->iConHandle, pstEnv->iConndID, stNet.pszBuff, stNet.iBuff, pstFrame);
     if(iRet != 0)
     {
              tlog_error(pstEnv->pstLog,0,0,"connapi send msg  failed!");
        	return iRet;
     }

     return 0;
}


int tserver_def_opt(TAPPCTX* pstAppCtx, TSERVERENV* pstEnv)
{
       int opt;
	int iOldOptErr;
	static struct option s_astLongOptions[] = {
		{"conndid", 1, NULL, 'u'},		
	       {"echofactor",1,NULL,'e'},
	       {"no-encrypt",0,NULL,'c'},
		{0, 0, 0, 0}
	};

	assert(NULL != pstEnv);
	assert(0 < pstAppCtx->argc);
	assert(NULL != pstAppCtx->argv);

	iOldOptErr	=	opterr;
	opterr	=	0;	
	while (1)
	{
		int option_index = 0;
		
		opt = getopt_long (pstAppCtx->argc, pstAppCtx->argv, "ue:",
			s_astLongOptions, &option_index);

		if (opt == -1)
			break;
		
		switch(opt)
		{
		case 'u':
			pstEnv->iConndID= inet_addr(optarg);
			break;
		case 'e':
			pstEnv->iEchoFactor=atoi(optarg);	
			break;
		case 'c':
			pstEnv->iNoEncrypt=1;
		default:
			break;
		}
	}/*while (1)*/


	/* restore the getopt environment. */
	opterr	=	iOldOptErr;
	optarg	=	NULL;	
	optind	=	1;	
	optopt	=	'?';

}


int main(int argc, char* argv[])
{
	int iRet;
	void* pvArg	=	&gs_stEnv;

	memset(&gs_stEnv, 0, sizeof(gs_stEnv));
	memset(&gs_stAppCtx, 0, sizeof(gs_stAppCtx));

	gs_stAppCtx.argc	=	argc;
	gs_stAppCtx.argv	=	argv;

	gs_stAppCtx.iTimer	=	100;
	gs_stAppCtx.iNoLoadConf = 1;
       gs_stAppCtx.iLib=(int)g_szMetalib_CSPKG;
	

	gs_stAppCtx.pfnInit	=	(PFNTAPPFUNC)tserver_init;
	gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)tserver_fini;
	gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)tserver_proc;
	gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)tserver_tick;

       tserver_def_opt(&gs_stAppCtx, &gs_stEnv);
	iRet	=	tapp_def_init(&gs_stAppCtx, pvArg);
	if( iRet<0 )
	{
		printf("Error: Initialization failed.\n");
		return iRet;
	}

	iRet	=	tapp_def_mainloop(&gs_stAppCtx, pvArg);

	tapp_def_fini(&gs_stAppCtx, pvArg);

	return iRet;
}

