/*
**  @file $RCSfile: mod_tconnd.c,v $
**  general description of this module
**  $Id: mod_tconnd.c,v 1.3 2009/03/03 09:13:02 sean Exp $
**  @author $Author: sean $
**  @date $Date: 2009/03/03 09:13:02 $
**  @version $Revision: 1.3 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
       
#include <arpa/inet.h>
#include <assert.h>

#include "pal/pal.h"
#include "tdr/tdr.h"

#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"

#include "taa/tagentapi.h"

#include "../../../tagent/src/plugin.h"
#include "../../tconndsvr/tconnkey.h"

#include "tconnddef.h"


extern unsigned char g_szMetalib_tconnd[];

static char *version = "mod_tconnd.so.1.0.1";
static LPTLOGCATEGORYINST g_pstLogCat;

typedef struct plugin_data
{
	unsigned int 			uiAppid;
	unsigned int 			astUiBusid[256];
	int 					uiBusidSize;
	
	LPEXCHANGEMNG		pstExcMng;
	LPSVRSKEYINSTLIST	pstInstList;
	LPTDRMETA			pstMeta;
	
} plugin_data;

LPSVRSKEYINSTLIST tconnd_attach_svrskey(int iKey)
{
	int			iSize;
	int 			iShmID;
	int			iExist;
	
	char *		pvAddr;
	SVRSKEYINSTLIST*	pstListInst;
	
	iSize			=	sizeof(SVRSKEYINSTLIST);
	iExist		=	0;
	
	iShmID		=	shmget(iKey, iSize, 0666 | IPC_CREAT | IPC_EXCL);
	if (iShmID < 0)
	{
		iExist	=	1;
		iShmID	=	shmget(iKey, iSize, 0666);
	}
	
	if (iShmID < 0)
	{
		return NULL;
	}
	
	pvAddr		=	shmat(iShmID, NULL, 0);
	if (!pvAddr)
	{
		return NULL;
	}
	
	pstListInst	=	(SVRSKEYINSTLIST*)pvAddr;
	
	return pstListInst;
}


LPSVRSKEYINST tconnd_svrskey_instance_find(LPSVRSKEYINSTLIST pstInstList, int iBusid)
{
	int	i;
	
	for (i = 0; i < TCONND_SVRSKEY_MAX_INST; i++)
	{
		if (pstInstList->astSvrsKeyInstList[i].stHead.iBusid == iBusid)
		{
			return &pstInstList->astSvrsKeyInstList[i];
		}
	}
	
	return NULL;
}

static int process_report_version(struct plugin *p)
{
	int 					i, j;
	int 					iRet;

	TDRDATA			stHost;
	TDRDATA			stNet;
	
	char 				szBuf[65536];
	
	tagPkgHead 			stHead;
	LPSVRSKEYINST		pstInst;
	
	plugin_data *			pstPluginData;
	TCONNDINNERMSG	stInnerMsg;
	
	pstPluginData 	= p->data;
	/*
	重新读取一下
	*/
	iRet = agent_api_get_bussid(
			pstPluginData->pstExcMng, 
			ID_APPID_TCONND, 
			pstPluginData->astUiBusid, 
			&pstPluginData->uiBusidSize
		);
	
	if (iRet)
	{
		tlog_log(g_pstLogCat, TLOG_PRIORITY_ERROR, 0,0, "agent_api_get_bussid error:%d\n", iRet);
	}
	
	for (i = 0; i < pstPluginData->uiBusidSize; i++)
	{
		pstInst = tconnd_svrskey_instance_find(pstPluginData->pstInstList, pstPluginData->astUiBusid[i]);
		if (!pstInst)
		{
			tlog_log(g_pstLogCat, TLOG_PRIORITY_ERROR, 0,0, "(Busid:%d) seekShm Error\n", pstPluginData->astUiBusid[i]);
			continue;
		}
		
		tlog_log(g_pstLogCat, TLOG_PRIORITY_DEBUG, 0,0, "(Busid:%d) PDU:%d\n", pstPluginData->astUiBusid[i], pstInst->stHead.iPDU);
		if (pstInst->stHead.iPDU > TCONND_MAX_PDU)
		{
			continue;
		}
		
		stHead.uiBusid	= pstPluginData->astUiBusid[i];
		stHead.uiSourceIP	= 0;
		stHead.uiDestIP	= 0;
		
		stInnerMsg.stHead.wVersion	=	TDR_METALIB_TCONND_VERSION;
		stInnerMsg.stHead.wCmdID	=	ID_MSG_TCONND_REPORT;
		stInnerMsg.stBody.stTconndSvrsKeyVerReport.stSvrsKeyList.iCount = pstInst->stHead.iPDU;
		
		for (j = 0; j < pstInst->stHead.iPDU; j++)
		{
			stInnerMsg.stBody.stTconndSvrsKeyVerReport.stSvrsKeyList.astSvrsKey[j]	=	pstInst->stBody.astSvrsKey[j];
		}

		stHost.pszBuff		=	(char *)&stInnerMsg;
		stHost.iBuff		=	sizeof(stInnerMsg);

		stNet.pszBuff		=	szBuf;
		stNet.iBuff		=	sizeof(szBuf);

		iRet	=	tdr_hton(pstPluginData->pstMeta, &stNet, &stHost, 0);
		if (iRet)
		{
			tlog_log(g_pstLogCat, TLOG_PRIORITY_ERROR, 0,0, "%s", tdr_error_string(iRet));
			continue;
		}
		
		iRet = p->send(p, stNet.pszBuff, stNet.iBuff, &stHead);
		if (iRet)
		{
			tlog_log(g_pstLogCat, TLOG_PRIORITY_ERROR, 0,0, "%s:process_report_version write error\n", version, strerror(errno));
		}
	}
	
	return iRet;
}

int mod_tconnd_init(struct plugin *p)
{
	int 			i, iRet;
	plugin_data *	pstPluginData;
	
	pstPluginData 		= (plugin_data *)malloc(sizeof(plugin_data));
	if (!pstPluginData)
	{
		exit (-1);
	}
	
	memset(pstPluginData, 0x00, sizeof(plugin_data));
	
	pstPluginData->pstMeta	= tdr_get_meta_by_name((LPTDRMETALIB)g_szMetalib_tconnd, "TconndinnerMsg");
	if (!pstPluginData->pstMeta)
	{
		exit(-1);
	}
	
	p->data 	= 	pstPluginData;
	
	iRet 		=	agent_api_init(&(pstPluginData->pstExcMng));
	if (iRet)
	{
		tlog_log(g_pstLogCat, TLOG_PRIORITY_ERROR, 0,0, "agent_api_init error:%d\n", iRet);
		return -1;
	}
	
	iRet 		=	agent_api_get_bussid(pstPluginData->pstExcMng, ID_APPID_TCONND, pstPluginData->astUiBusid, &pstPluginData->uiBusidSize);
	
	for (i = 0; i < pstPluginData->uiBusidSize; i++)
	{
		tlog_log(g_pstLogCat, TLOG_PRIORITY_DEBUG, 0,0, "%s(appid:%d):bussid:%d\n", version, ID_APPID_TCONND, pstPluginData->astUiBusid[i]);
	}
	
	pstPluginData->pstInstList	= tconnd_attach_svrskey(TCONNDSHMKEY);
	if (!pstPluginData->pstInstList)
	{
		tlog_log(g_pstLogCat, TLOG_PRIORITY_DEBUG, 0,0, "%s(appid:%d):bussid:%d initShm Error\n", version, ID_APPID_TCONND, pstPluginData->astUiBusid[i]);
		
		return -1;
	}
	
	return 0;
}

int mod_tconnd_cleanup(struct plugin *p)
{
	plugin_data *pstPluginData = 0;

	if (p->name)
	{
		free (p->name);
	}
	
	if (p->version)
	{
		free (p->version);
	}
	
	pstPluginData = p->data;
	if (pstPluginData)
	{
		agent_api_destroy(pstPluginData->pstExcMng);
		shmdt((void *)(pstPluginData->pstInstList));
		
		free(pstPluginData);		
		p->data = 0;
	}
	
	return 0;
}

int mod_tconnd_timer(struct plugin *p)
{
	static size_t mod_timer = 0;
	plugin_data *pstPluginData = 0;

	assert (p);

	pstPluginData = p->data;
	if (!pstPluginData)
	{
		tlog_log(g_pstLogCat, TLOG_PRIORITY_FATAL, 0,0, "p->data == null, why?\n");
		return -1;
	}
	
	if (mod_timer++ % 60)
	{
		return 0;
	}
	
	return process_report_version(p);
}

int mod_tconnd_recv(struct plugin *p, char *ptr, size_t size, struct tagPkgHead *ahead)
{
	int					iRet;
	int 					i, j, k;
	
	TDRDATA			stHost;
	TDRDATA			stNet;
	
	plugin_data *			pstPluginData;
	
	LPSVRSKEYINST		pstInst;
	TCONNDINNERMSG	stInnerMsg;
	
	tlog_log(g_pstLogCat, TLOG_PRIORITY_DEBUG, 0,0, "mod_tconnd_triggeraaaaaaaaaaaaaaaaaaaaaaaa\n");
	
	pstPluginData = p->data;
	if (!pstPluginData)
	{
		tlog_log(g_pstLogCat, TLOG_PRIORITY_FATAL, 0,0, "p->data == null, why?\n");
		return -1;
	}
	
	stHost.pszBuff		=	(char *)&stInnerMsg;
	stHost.iBuff		=	sizeof(stInnerMsg);

	stNet.pszBuff		=	ptr;
	stNet.iBuff		=	size;
	
	iRet	=	tdr_ntoh(pstPluginData->pstMeta, &stHost, &stNet, 0);
	if (iRet)
	{
		tlog_log(g_pstLogCat, TLOG_PRIORITY_ERROR, 0,0, "%s", tdr_error_string(iRet));
		return -1;
	}
	
	if (ID_MSG_TCONND_UPDATE == stInnerMsg.stHead.wCmdID)
	{
		/* update exchange */
		for (i = 0; i < pstPluginData->uiBusidSize; i++)
		{
			if (pstPluginData->astUiBusid[i] != ahead->uiBusid)
			{
				continue;
			}
			
			pstInst = tconnd_svrskey_instance_find(pstPluginData->pstInstList, pstPluginData->astUiBusid[i]);
			if (!pstInst)
			{
				tlog_log(g_pstLogCat, TLOG_PRIORITY_ERROR, 0,0, "(Busid:%d) find instance Error\n", pstPluginData->astUiBusid[i]);
				continue;
			}
			
			for (j = 0; j < stInnerMsg.stBody.stTconndSvrsKeyUpdate.stSvrsKeyList.iCount; j++)
			{
				for (k = 0; k < pstInst->stHead.iPDU; k++)
				{
					if (strcmp(pstInst->stBody.astSvrsKey[k].szPDUName, stInnerMsg.stBody.stTconndSvrsKeyUpdate.stSvrsKeyList.astSvrsKey[j].szPDUName) == 0)
					{
						pstInst->stBody.astSvrsKey[k]	=	stInnerMsg.stBody.stTconndSvrsKeyUpdate.stSvrsKeyList.astSvrsKey[j];
					}
				}
			}
			
			tlog_log(g_pstLogCat, TLOG_PRIORITY_INFO, 0,0, "[%u,%u]Update tconndkey Suss!\n", ahead->uiAppid, ahead->uiBusid);
		}
		
		return 0;
		
	}
	else if (ID_MSG_TCONND_ASKREPORT == stInnerMsg.stHead.wCmdID)
	{
		return process_report_version(p);
	}
	else
	{
		fprintf (stderr, "%s verson:%s may not support!\n", p->name, version);
	}
	
	return 0;
}

int mod_tconnd_plugin_init(struct plugin *p) 
{
	p->name		= strdup("mod_tconnd");
	p->version		= strdup(version);
	p->init			= mod_tconnd_init;
	p->cleanup		= mod_tconnd_cleanup;
	p->timer			= mod_tconnd_timer;
	p->recv			= mod_tconnd_recv;
	p->data			= NULL;
	p->id			= ID_APPID_TCONND;
	/* set log  */
	g_pstLogCat		= p->log;
	
	return 0;
}

