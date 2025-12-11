/*
**  @file $RCSfile: tconndkeymgr_proc.c,v $
**  general description of this module
**  $Id: tconndkeymgr_proc.c,v 1.3 2009/02/04 04:38:30 sean Exp $
**  @author $Author: sean $
**  @date $Date: 2009/02/04 04:38:30 $
**  @version $Revision: 1.3 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "pal/pal.h"
#include "taa/tagentapi.h"
#include "apps/centerdapi/centerdapi.h"

#include "tbus/tbus.h"
#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"
#include "tconndkeymgr_conn.h"
#include "tconndkeymgr_proc.h"

extern LPTLOGCATEGORYINST g_pstLogCat;

inline static int compare(struct PDUNode *p0, struct PDUNode *p1)
{
	return strcmp(p0->stSvrsKey.szPDUName, p1->stSvrsKey.szPDUName);
}

RB_PROTOTYPE(PDUTree, PDUNode, entry, compare);
RB_GENERATE (PDUTree, PDUNode, entry, compare);

PDUNode *new_PDUNode(tconndNode *pstTconndNode, SVRSKEYDATA *pstSvrskeyData)
{
	PDUNode *pstPDUNode;
	
	pstPDUNode = (PDUNode *)malloc(sizeof(PDUNode));
	assert (pstPDUNode);
	
	pstPDUNode->stSvrsKey= *pstSvrskeyData;
	RB_INSERT(PDUTree, &pstTconndNode->pduNodeRoot, pstPDUNode);
	
	return pstPDUNode;
}

PDUNode *find_PDUNode(tconndNode *pstTconndNode, SVRSKEYDATA *pstSvrskeyData)
{
	PDUNode *pstPDUNode;
	RB_FOREACH(pstPDUNode, PDUTree, &pstTconndNode->pduNodeRoot)
	{
		if (strcmp(pstSvrskeyData->szPDUName, pstPDUNode->stSvrsKey.szPDUName) == 0)
		{
			return pstPDUNode;
		}
	}
	
	return 0;
}

void free_PDUNode(tconndNode *pstTconndNode)
{
	PDUNode *pstPDUNode;
	
	while ( !RB_EMPTY(&pstTconndNode->pduNodeRoot) ) 
	{
		pstPDUNode = RB_ROOT(&pstTconndNode->pduNodeRoot);
		RB_REMOVE(PDUTree, &pstTconndNode->pduNodeRoot, pstPDUNode);
		
		free (pstPDUNode);
	}
}

tconndNode *new_tconndNode(SERVER *pstSrv, unsigned int iBusid)
{
	tconndNode *pstTconndNode;

	pstTconndNode = (tconndNode *)malloc(sizeof(tconndNode));	
	assert (pstTconndNode);
	
	pstTconndNode->iBusid = iBusid;
	
	TAILQ_INSERT_TAIL(&pstSrv->stTcndQueue, pstTconndNode, next);
	RB_INIT(&pstTconndNode->pduNodeRoot);
	
	return pstTconndNode;
}

tconndNode *find_tconndNode(SERVER *pstSrv, unsigned int iBusid)
{
	tconndNode *pstTconndNode;

	TAILQ_FOREACH(pstTconndNode, &pstSrv->stTcndQueue, next)
	{
		if (iBusid == pstTconndNode->iBusid)
		{
			return pstTconndNode;
		}
	}
	
	return 0;
}

void free_tconndNode(SERVER *pstSrv)
{
	tconndNode *pstTconndNode;
	while ((pstTconndNode = TAILQ_FIRST(&pstSrv->stTcndQueue)))
	{
		TAILQ_REMOVE(&pstSrv->stTcndQueue, pstTconndNode, next);
		
		free_PDUNode (pstTconndNode);
		free(pstTconndNode);
	}
}

static void free_Nodes(SERVER *pstSrv)
{
	free_tconndNode(pstSrv);
}

static int add_tconndNode(SERVER * pstSrv, BUSIDLIST *pstBuidList)
{
	int i;
	tconndNode *pstTconndNode;

	for (i = 0; i < pstBuidList->iCount; i++)
	{
		pstTconndNode = find_tconndNode(pstSrv, pstBuidList->busid[i]);
		if (!pstTconndNode)
		{
			pstTconndNode = new_tconndNode(pstSrv, pstBuidList->busid[i]);
		}
	}

	return 0;
	
}

static int add_PDUNode(SERVER * pstSrv, unsigned int iBusid, SVRSKEYLIST * pstSvrskeyList)
{
	int i;

	PDUNode		*pstPDUNode;
	tconndNode	*pstTconndNode;
	
	pstTconndNode = find_tconndNode(pstSrv, iBusid);
	if (!pstTconndNode)
	{
		return -1;
	}

	for (i = 0; i < pstSvrskeyList->iCount; i++)
	{
		pstPDUNode = find_PDUNode(pstTconndNode, &pstSvrskeyList->astSvrsKey[i]);
		if (!pstPDUNode)
		{
			pstPDUNode = new_PDUNode(pstTconndNode, &pstSvrskeyList->astSvrsKey[i]);
		}
	}
	
	return 0;
}

int process_0x30(LPRSYSENV pstEnv, SERVER *pstSrv, tagPkgHead* pstHead, TCONNDINNERMSG* pstInnerMsg)
{
	int 					i, iRet;
	int 					iNewTconndNode = 0;
	int					iCount = 0;
	
	char					szBuffer[1024 * 128];
	
	TDRDATA			stHost;
	TDRDATA			stNet;
	
	SVRSKEYDATA*		pstSvrskeyData;
	
	PDUNode *			pstPDUNode;
	tconndNode*			pstTconndNode;

	TCONNDINNERMSG	stInnerMsg;
	
	tlog_log(g_pstLogCat, TLOG_PRIORITY_DEBUG, 0, 0, "==========process_0x30 begin===========\n");	
	
	pstTconndNode 		= find_tconndNode(pstSrv, pstHead->uiBusid);
	if (!pstTconndNode)
	{
		/* new tconndNode */
		pstTconndNode 	= new_tconndNode(pstSrv, pstHead->uiBusid);
		iNewTconndNode 	= 1;
	}
	
	iCount =	0;
	for (i = 0; i < pstInnerMsg->stBody.stTconndSvrsKeyVerReport.stSvrsKeyList.iCount; i++)
	{
		pstSvrskeyData	= &pstInnerMsg->stBody.stTconndSvrsKeyVerReport.stSvrsKeyList.astSvrsKey[i];
		pstPDUNode		= find_PDUNode(pstTconndNode, pstSvrskeyData);
		if (!pstPDUNode)
		{
			/* add new pdu */
			pstPDUNode	= new_PDUNode(pstTconndNode, pstSvrskeyData);
		}
		else if (!iNewTconndNode)
		{
			/* compare */
			if (memcmp(&pstPDUNode->stSvrsKey, pstSvrskeyData, sizeof(SVRSKEYDATA)))
			{
				if (iCount < TCONND_MAX_PDU)
				{
					stInnerMsg.stBody.stTconndSvrsKeyUpdate.stSvrsKeyList.astSvrsKey[iCount] = pstPDUNode->stSvrsKey;
					iCount ++;
				}
			}
		}
	}
	
	if (iCount > 0)
	{
		stInnerMsg.stHead			=	pstInnerMsg->stHead;
		stInnerMsg.stHead.wCmdID	=	ID_MSG_TCONND_UPDATE;
		stInnerMsg.stBody.stTconndSvrsKeyUpdate.stSvrsKeyList.iCount = iCount;
		
		stHost.pszBuff				=	(char *)&stInnerMsg;
		stHost.iBuff				=	sizeof(stInnerMsg);
		
		stNet.pszBuff				=	szBuffer;
		stNet.iBuff				=	sizeof(szBuffer);
		
		iRet	=	tdr_hton(pstSrv->pstMeta, &stNet, &stHost, 0);
		if (iRet)
		{
			tlog_log(g_pstLogCat, TLOG_PRIORITY_ERROR, 0,0, "tdr_ntoh:%s", tdr_error_string(iRet));
			return -1;
		}
		
		iRet	=	centerdapi_unicast(pstEnv->iHandle, pstEnv->iDst, stNet.pszBuff, stNet.iBuff, pstHead->uiBusid, pstHead->uiSourceIP);
		if (iRet)
		{
			tlog_log(g_pstLogCat, TLOG_PRIORITY_ERROR, 0,0, "centerdapi_unicast error:%s", tbus_error_string(iRet));
			return -1;
		}
	}	
	
	tlog_log(g_pstLogCat, TLOG_PRIORITY_DEBUG, 0, 0, "==========process_0x30 end=============\n");
	return 0;
}

/*
int process_0x31(TAPPCTX* pstAppCtx, SERVER *pstSrv, unsigned int uiBusid)
{
	int iRet;
	int iCount, iLeft;
	int iBus, iSrc, iDst;
	
	tagPkgHead stHead;
	
	PDUNode *pstPDUNode;
	tconndNode *pstTconndNode;

	SVRSKEYDATA stSvrsKey;
	char szBuffer[65536] = {0};
	
	pstTconndNode = find_tconndNode(pstSrv, stHead.uiBusid);
	if (!pstTconndNode)
	{
		return 0;
	}
	
	iCount = 1;
	RB_FOREACH(pstPDUNode, PDUTree, &pstTconndNode->pduNodeRoot)
	{
		memcpy(&stSvrsKey, &pstPDUNode->stSvrsKey, sizeof(SVRSKEYDATA));
		stSvrsKey.iSvrSKeyLen = htonl(stSvrsKey.iSvrSKeyLen);
		stSvrsKey.iSvrSKey2Len = htonl(stSvrsKey.iSvrSKey2Len);
		
		memcpy(&szBuffer[PKGHEADLEN+(iCount-1)*sizeof(SVRSKEYDATA)], &stSvrsKey, sizeof(SVRSKEYDATA));
		if (iCount % 16 == 0)
		{
			stHead.uLen 		= htons(PKGHEADLEN+16*sizeof(SVRSKEYDATA));
			stHead.uMsgCmd	= htons(ID_MSG_TCONND_ASKREPORT);
			stHead.uiAppid	= htonl(ID_APPID_TCONND);
			stHead.uiBusid	= htonl(uiBusid);
			stHead.cMsgVer	= 0x0;
			stHead.cType		= ID_CMD_DISPATCH;
			stHead.uMagic		= htons(MSG_MAGIC);
			stHead.uiSourceIP	= htonl(0);
			stHead.uiDestIP	= htonl(0);
			
			memcpy(szBuffer, &stHead, PKGHEADLEN);
			iBus = pstAppCtx->iBus;
			iSrc = pstSrv->pstConfinst->ulTconndMgrAddr;
			iDst = pstSrv->pstConfinst->ulCenterdAddr;
			
			iRet = tbus_send(iBus, &iSrc, &iDst, szBuffer, ntohs(stHead.uLen), 0);
			if (0 != iRet)
			{
				tlog_log(g_pstLogCat, TLOG_PRIORITY_FATAL, 0,0, "process_0x31:tbus_send Error\n");
				return -1;
			}
			
			iCount++;
		}
	}
	
	iLeft = iCount % 16;
	if (iLeft)
	{
		stHead.uLen 		= htons(PKGHEADLEN+iLeft*sizeof(SVRSKEYDATA));
		stHead.uMsgCmd	= htons(ID_MSG_TCONND_ASKREPORT);
		stHead.uiAppid	= htonl(ID_APPID_TCONND);
		stHead.uiBusid	= htonl(uiBusid);
		stHead.cMsgVer	= 0x0;
		stHead.cType		= ID_CMD_DISPATCH;
		stHead.uMagic		= htons(MSG_MAGIC);
		stHead.uiSourceIP	= htonl(0);
		stHead.uiDestIP	= htonl(0);
		
		iBus = pstAppCtx->iBus;
		iSrc = pstSrv->pstConfinst->ulTconndMgrAddr;
		iDst = pstSrv->pstConfinst->ulCenterdAddr;
		
		iRet = tbus_send(iBus, &iSrc, &iDst, szBuffer, ntohs(stHead.uLen), 0);
		if (0 != iRet)
		{
			tlog_log(g_pstLogCat, TLOG_PRIORITY_FATAL, 0,0, "process_0x31:tbus_send Error\n");
			return -1;
		}
	}
	
	return 0;
}
*/

int readXMLData(TAPPCTX* pstAppCtx, SERVER* pstSrv)
{
	int 				i, iRet;
	struct stat	 	sbuf;
	
	char 			szFile[80];	
	TDRDATA 		stHost;
	
	BUSIDLIST *		pstBusidList;
	SVRSKEYLIST*	pstSvrskeyList;
	
	(void)pstAppCtx;
	free_Nodes(pstSrv);
	
	stHost.pszBuff 	=	pstSrv->stBusidList.pszBuff;
	stHost.iBuff 		=	pstSrv->stBusidList.iLen;
	
	memset (&sbuf, 0x00, sizeof(sbuf));
	if (stat(".index.xml", &sbuf) == -1)
	{
		tdr_init((LPTDRMETA)pstSrv->stBusidList.iMeta, &stHost, 0);
		iRet = tdr_output_file((LPTDRMETA)pstSrv->stBusidList.iMeta, ".index.xml", &stHost, 0, 0);
		
		if (iRet)
		{
			unlink(".index.xml");
			printf ("Error: tdr_output\n");
			return -1;
		}
	}
	else
	{
		iRet = tdr_input_file((LPTDRMETA)pstSrv->stBusidList.iMeta, &stHost, ".index.xml", 0, 0);
		if (iRet)
		{
			printf ("Error: load .index file:%d\n", iRet);
			return -1;
		}
		
		pstBusidList = (LPBUSIDLIST)stHost.pszBuff;		
		if (add_tconndNode(pstSrv, pstBusidList) < 0)
		{
			printf ("Error:add_tconndNode fail\n");
			return -1;
		}
		
		for (i = 0; i < pstBusidList->iCount; i++)
		{
			stHost.pszBuff = pstSrv->stSvrskeyList.pszBuff;
			stHost.iBuff = pstSrv->stSvrskeyList.iLen;
			
			snprintf (szFile, 80, ".sec.%d.xml", pstBusidList->busid[i]);
			
			iRet = tdr_input_file((LPTDRMETA)pstSrv->stSvrskeyList.iMeta, &stHost, szFile, 0, 0);
			if (iRet)
			{
				printf ("Error: load %s\n", szFile);
				continue;
			}
			
			pstSvrskeyList = (LPSVRSKEYLIST)stHost.pszBuff;
			if (add_PDUNode(pstSrv, pstBusidList->busid[i], pstSvrskeyList) < 0)
			{
				printf ("Error:add_tconndNode fail\n");
				return -1;
			}
		}
	}
	
	return 0;
}

int writeXMLData(TAPPCTX* pstAppCtx, SERVER* pstSrv)
{
	int iRet;
	char 			szFile[80];
	
	TDRDATA 		stHost;
	TDRDATA 		stPrint;
	
	BUSIDLIST *		pstBusidList;
	SVRSKEYLIST*	pstSvrskeyList;
	
	PDUNode	*		pstPDUNode;
	tconndNode*		pstTconndNode;

	(void)pstAppCtx;
	
	stHost.pszBuff		= pstSrv->stBusidList.pszBuff;
	stHost.iBuff		= pstSrv->stBusidList.iLen;
	
	pstBusidList 		= (LPBUSIDLIST)stHost.pszBuff;
	pstBusidList->iCount = 0;
	
	TAILQ_FOREACH(pstTconndNode, &pstSrv->stTcndQueue, next)
	{
		if (pstBusidList->iCount < TCONND_MAX_BUSIUNIT)
		{
			pstBusidList->busid[pstBusidList->iCount] = pstTconndNode->iBusid;
			pstBusidList->iCount ++;
		}
	}
	
	iRet = tdr_output_file((LPTDRMETA)pstSrv->stBusidList.iMeta, ".index.xml", &stHost, 0, 0);
	if (iRet)
	{
		tlog_log(g_pstLogCat, TLOG_PRIORITY_FATAL, 0,0, "tdr_output_file:%d\n", iRet);
		return -1;
	}
	
	TAILQ_FOREACH(pstTconndNode, &pstSrv->stTcndQueue, next)
	{
		stHost.pszBuff		= pstSrv->stSvrskeyList.pszBuff;
		stHost.iBuff 		= pstSrv->stSvrskeyList.iLen;
		
		pstSvrskeyList 	= (LPSVRSKEYLIST)stHost.pszBuff;
		pstSvrskeyList->iCount = 0;
		
		RB_FOREACH(pstPDUNode, PDUTree, &pstTconndNode->pduNodeRoot)
		{
			if (pstSvrskeyList->iCount < TCONND_MAX_PDU)
			{
				pstSvrskeyList->astSvrsKey[ pstSvrskeyList->iCount ] = pstPDUNode->stSvrsKey;
				pstSvrskeyList->iCount ++;
			}
		}

		stPrint = stHost;
		
		snprintf (szFile, 80, ".sec.%d.xml", pstTconndNode->iBusid);
		iRet = tdr_output_file((LPTDRMETA)pstSrv->stSvrskeyList.iMeta, szFile, &stHost, 0, 0);	
		if (iRet)
		{
			tlog_log(g_pstLogCat, TLOG_PRIORITY_FATAL, 0,0, "tdr_output_file:%s\n", szFile);
			continue;
		}
		
		//tdr_fprintf((LPTDRMETA)pstSrv->stSvrskeyList.iMeta, stdout, &stPrint, 0);
	}
	
	return 0;
}

