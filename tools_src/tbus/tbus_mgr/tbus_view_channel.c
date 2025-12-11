#include <assert.h>
#include "tbus_mgr.h"
#include "tbus/tbus.h"
#include "tbus/tbus_addr_templet.h"
#include "tbus/tbus_kernel.h"
#include "tbus/tbus_misc.h"
#include "tbus/tbus_comm.h"
#include "tbus_view_channel.h"

extern TBUSMGROPTIONS	g_stOption;


void DumpBusInterface(int iBusHandle, int iProcID, LPVIEWCHANNELTOOLS pstTool);
int DumpBusChannel(LPTBUSCHANNEL pstChannel, LPVIEWCHANNELTOOLS pstTool);




int ViewChannels(IN LPTBUSSHMGCIM a_pShmGCIM, LPTBUSMGROPTIONS a_pstOption)
{
	int iRet =0;
	int iBusHandle;
	TBUSADDR iSrcAddr;
	VIEWCHANNELTOOLS stTools;

	assert(NULL != a_pShmGCIM);
	assert(NULL != a_pstOption);

	if ('\0' == a_pstOption->szProcID[0])
	{
		printf("Error: processid not specified, use --view=proc option.\n");
		return -1;
	}

	iRet = tbus_initialize(a_pstOption->szShmKey, a_pShmGCIM->stHead.dwBusiID);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus init error\n");	
		return -1;
	}

	iRet = tbus_new(&iBusHandle);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus new error\n");
		return -1;
	}

	iRet = tbus_addr_aton(&a_pstOption->szProcID[0], &iSrcAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("failed to convert %s to tbus address\n", a_pstOption->szProcID);
		return -1;
	}
	iRet = tbus_bind(iBusHandle, iSrcAddr);
	if (TBUS_SUCCESS != iRet)
	{
		printf("tbus bind error by addr %s\n", a_pstOption->szProcID);
		return -1;
	}

	memset(&stTools, 0, sizeof(stTools));
	if ('\0' != a_pstOption->szMetalibFile[0])
	{
		iRet = tdr_load_metalib(&stTools.pstLib, a_pstOption->szMetalibFile);
		if (TDR_SUCCESS != iRet)
		{
			stTools.pstLib = NULL;
			printf("warnsing: failed to load metalib %s, for %s\n", a_pstOption->szMetalibFile, tdr_error_string(iRet));
		}
	}

	if (('\0' != a_pstOption->szMetaName[0]) && (NULL != stTools.pstLib))
	{
		stTools.pstMsgMeta = tdr_get_meta_by_name(stTools.pstLib, a_pstOption->szMetaName);
		if (NULL == stTools.pstMsgMeta)
		{
			printf("warnning: failed to get meta by name %s\n", a_pstOption->szMetaName);
		}else
		{
			stTools.iSize = tdr_get_meta_size(stTools.pstMsgMeta);
			stTools.pszBuff = (char *)malloc(stTools.iSize);
		}
	}/*if (('\0' != a_pstOption->szMetaName[0]) && (NULL != stTools.pstLib))*/

	if ('\0' != a_pstOption->szOutFile[0])
	{
		stTools.fpOut = fopen(a_pstOption->szOutFile, "w");
		if (NULL == stTools.fpOut)
		{
			stTools.fpOut = stdout;
		}
	}else
	{
		stTools.fpOut = stdout;
	}
	stTools.iMaxSeeMsg = a_pstOption->iMaxSeeMsg;

	DumpBusInterface(iBusHandle, iSrcAddr, &stTools);

	if (stdout != stTools.fpOut)
	{
		fclose(stTools.fpOut);
	}
	if (NULL != stTools.pszBuff)
	{
		free(stTools.pszBuff);
	}

	return 0;
}

void DumpBusInterface(int iBusHandle, int iProcID, LPVIEWCHANNELTOOLS pstTool)
{
	int i;
	TBUSHANDLE *pstHandle;
	
	assert(NULL != pstTool);

	pstHandle = tbus_get_handle( iBusHandle ) ;
	if ( NULL == pstHandle)
	{
		printf("DumpBusInterface failed, invalid handle  %d\n", iBusHandle);
		return  ;
	}

	fprintf(pstTool->fpOut, "================================================\n");
	fprintf(pstTool->fpOut, "Total bind channel: %d, self:[%s].\n", pstHandle->dwChannelCnt, inet_ntoa(*(struct in_addr*)&iProcID));

	for(i=0; i< (int)pstHandle->dwChannelCnt; i++)
	{
		DumpBusChannel(pstHandle->pastChannelSet[i], pstTool);
	}
}


int PrintBuffMsg(CHANNELHEAD *a_pstHead ,LPCHANNELVAR a_pstVar, const char* pszData, LPVIEWCHANNELTOOLS pstTool)
{
	TBUSHEAD stHead;
	int iData;
	int iTailLen;
	TDRDATA stHostInfo;
	TDRDATA stNetInfo;
	int iPkg;
	int iMsg=0;
	int iRet;
	const char *pszPkg = NULL;
	int iSize;
	int iHead;
	int iTail;

#define  TBUS_PRINT_MSG_SLEEP_GAP	20

	assert(NULL != pszData);
	assert(NULL != pstTool);
	assert(NULL != pstTool->pstMsgMeta);
	assert(NULL != pstTool->pszBuff);
	assert(0 < pstTool->iSize);
	assert(NULL != a_pstVar);
	assert(NULL != a_pstHead);

	iHead = a_pstVar->dwHead;
	iTail = a_pstVar->dwTail;
	iSize = a_pstVar->dwSize;
	if( iHead==iTail )
		return 0;

	iData = iTail - iHead;
	if (iData< 0) iData	+=	iSize;

	

	while( iData>0 )
	{
		if (TBUS_SUCCESS != tbus_get_pkghead(&stHead, pszData, iSize, (unsigned int *)&iHead, iTail))
		{
			break;
		}


		iPkg	=	stHead.bHeadLen + stHead.iBodyLen;
		TBUS_CALC_ALIGN_VALUE_BY_LEVEL(iPkg, a_pstHead->dwAlignLevel);
		if( iData < iPkg )
			break;

		assert(0 <= iHead);
		pszPkg = pszData + iHead + stHead.bHeadLen;
		

		/*解析数据包*/
		if (pstTool->iCurSeeMsg >= pstTool->iMaxSeeMsg)
		{
			break;
		}
		fprintf(pstTool->fpOut, "Msg[%d]:\n", iMsg+1);
		stHostInfo.iBuff = pstTool->iSize;
		stHostInfo.pszBuff = pstTool->pszBuff;
		stNetInfo.pszBuff = (char *)pszPkg;
		stNetInfo.iBuff = stHead.iBodyLen;
		iRet = tdr_ntoh(pstTool->pstMsgMeta, &stHostInfo, &stNetInfo, 0);
		if (0 < stHostInfo.iBuff)
		{
			tdr_fprintf(pstTool->pstMsgMeta, pstTool->fpOut, &stHostInfo, 0);
		}else
		{
			printf("failed to unpack pkg(len:%d):%s\n", stHead.iBodyLen, tdr_error_string(iRet));
		}
		pstTool->iCurSeeMsg++;
	
		tbus_moveto_next_pkg(&stHead, a_pstHead, iSize, &iHead, iTail);
		iData	-=	iPkg;
		iMsg++;
		if ((iMsg % TBUS_PRINT_MSG_SLEEP_GAP) == 0)
		{
			usleep(1000);
		}
	}

	

	return iMsg;
}

void DumpBusVar(CHANNELHEAD *a_pstHead ,LPCHANNELVAR pstVar, char *pszData, LPVIEWCHANNELTOOLS pstTool)
{
	int iMsg=0;
	int iDataLen=0;
	int iHSeq = 0;
	int iGSeq = 0;
	TBUSHEAD stHead;

	assert(NULL != pstVar);
	assert(NULL != pszData);
	assert(NULL != pstTool);
	assert(NULL != a_pstHead);

	iDataLen = pstVar->dwTail - pstVar->dwHead;
	if (iDataLen < 0) 
	{
		iDataLen += pstVar->dwSize;
	}	

	if ( (0 < iDataLen) && 
		(TBUS_SUCCESS == tbus_get_pkghead(&stHead, pszData, pstVar->dwSize, (unsigned int*)&pstVar->dwHead, pstVar->dwTail)))
	{
		iHSeq = stHead.iSeq;
	}
	if ((0 != pstVar->chGStart) && (0 < iDataLen) &&
		(TBUS_SUCCESS == tbus_get_pkghead(&stHead, pszData, pstVar->dwSize, &pstVar->dwGet, pstVar->dwTail)))
	{
		iGSeq = stHead.iSeq;
	}

	/*设计更改：统计通道上的消息数只需将队列上的序列号减去队列头部第一个消息的序列号即可，因为队列中的
	每一个消息都有序列号，且序列号是递增的*/
	//iMsg = CalcBuffMsg(pszData, pstVar->dwSize, pstVar->dwHead, pstVar->dwTail);
	if (0 < iDataLen)
	{
		iMsg = pstVar->iSeq - iHSeq;
	}else
	{
		iMsg = 0;
	}

	fprintf(pstTool->fpOut, "{MsgNum:%d, Bytes:%d H:%u/T:%u G: %u Size:%u HSeq: %u GSeq: %u} seq=%u rseq=%u Align:%d\n",
		iMsg, iDataLen, pstVar->dwHead, pstVar->dwTail, pstVar->dwGet, pstVar->dwSize, 
		iHSeq, iGSeq, pstVar->iSeq, pstVar->iRecvSeq,
		1<<a_pstHead->dwAlignLevel);

	/*尝试打印通道中的数据*/
	if ((0 < iMsg) && (NULL != pstTool->pstMsgMeta) 
		&& (NULL != pstTool->pszBuff) && (0 < pstTool->iSize))
	{
		PrintBuffMsg(a_pstHead, pstVar, pszData, pstTool);
	}

}


int DumpBusChannel(LPTBUSCHANNEL pstChannel, LPVIEWCHANNELTOOLS pstTool)
{
	LPCHANNELVAR pstGet;
	char* pszGet;
	LPCHANNELVAR pstPut;
	char* pszPut;
	char szSrc[128] = {0};
	char szDst[128] = {0};
	CHANNELHEAD *pstHead ;

	assert(NULL != pstChannel);
	assert(NULL != pstTool);

	pstHead = pstChannel->pstHead;
	pstGet	=	&pstHead->astQueueVar[pstChannel->iRecvSide];
	pszGet	=	pstChannel->pszQueues[pstChannel->iRecvSide];
	pstPut	=	&pstHead->astQueueVar[pstChannel->iSendSide];
	pszPut	=	pstChannel->pszQueues[pstChannel->iSendSide];
	STRNCPY(szSrc,tbus_addr_ntoa(pstHead->astAddr[pstChannel->iRecvSide]), sizeof(szSrc));
	STRNCPY(szDst,tbus_addr_ntoa(pstHead->astAddr[pstChannel->iSendSide]), sizeof(szDst));

		
	fprintf(pstTool->fpOut, "[%10s <-- %10s]: ", szSrc, szDst);

	DumpBusVar(pstHead, pstGet, pszGet, pstTool);

	fprintf(pstTool->fpOut, "[%10s --> %10s]: ", szSrc, szDst);
	DumpBusVar(pstHead, pstPut, pszPut, pstTool);

	return 0;

}








