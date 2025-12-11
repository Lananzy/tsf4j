/** @file $RCSfile: tbus_channel.c,v $
  general description of this module
  $Id: tbus_channel.c,v 1.23 2009/04/01 10:51:50 jacky Exp $
@author $Author: jacky $
@date $Date: 2009/04/01 10:51:50 $
@version $Revision: 1.23 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/

#include "pal/pal.h"
#include "tbus/tbus.h"
#include "tbus_misc.h"
#include "tbus_dyemsg.h"
#include "tbus_config_mng.h"
#include "tbus_head.h"
#include "tbus_kernel.h"
#include "tbus_log.h"
#include "tbus_channel.h"

extern LPTLOGCATEGORYINST	g_pstBusLogCat;




///////////////////////////////////////////////////////////////////////////////////////////////////////



/*设计说明：tbus数据通道设计为循环队列，将数据放入队列时，之前的设计可能将数据分拆成两段放入队列中，
* 这意味着数据不保持在地址连续的缓冲区中，当需要访问消息时，需要将消息拷贝出来。
*为加快数据访问速度，更改上述设计方法，当数据放入队列尾部时，总是将数据保存在地址连续的缓冲区中。
*按照这种设计思路，当从数据尾指针开始到队列尾部之间的空间不够存储一个完整数据时，将这些剩余的空间放入一个
*假数据，将数据尾指针移植队列最前面保持数据。
*通过宏定义TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE来同时保存这两种方法的代码，编译时如果定义了TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE
*则采用分拆数据的方式存储；否则采用将数据保持在连续空间的方式存储，此方法也是缺省提供的实现方式
*/


#ifdef TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE
int tbus_push_channel_pkgv(IN TBUSCHANNEL* a_pstChannel, IN LPTBUSHEAD a_pstHead, 
						   IN const struct iovec *a_ptVector, IN const int a_iVecCnt)
{
	CHANNELVAR *pstVar = NULL;
	int i;
	char szNet[TBUS_HEAD_CODE_BUFFER_SIZE];
	int iHeadLen = 0;
	int iRet = TBUS_SUCCESS;
	int iRoom;
	int iPkgLen;
	int iTailRoom;
	char *pszQueue = NULL ;
	unsigned int dwTmpTail;

	assert(NULL != a_pstChannel);
	assert(NULL != a_pstHead);
	assert(NULL != a_ptVector);

	/*填充并将头部消息打包*/
	pstVar = &a_pstChannel->pstHead->astQueueVar[a_pstChannel->iSendSide];
	tbus_log(TLOG_PRIORITY_TRACE,"queue(%d --> %d) size:%d head:%d tail:%d seq:%d", 
		a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],
		a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
		pstVar->dwSize, pstVar->dwHead, 
		pstVar->dwTail, pstVar->iSeq);

	a_pstHead->iSeq = pstVar->iSeq;
	a_pstHead->bFlag = a_pstHead->bFlag & ~TBUS_HEAD_FLAG_SYN ; /* avoid illegal bit set */
	a_pstHead->iBodyLen = 0;
	for ( i=0; i<a_iVecCnt; i++ )
	{
		if ((0 >= a_ptVector[i].iov_len) || (NULL == a_ptVector[i].iov_base))
		{
			continue;
		}
		a_pstHead->iBodyLen += (int)a_ptVector[i].iov_len;		
	}
	if (0 >= a_pstHead->iBodyLen)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid data len %d", a_pstHead->iBodyLen);
		return TBUS_ERR_ARG;
	}
	iHeadLen = sizeof(szNet);
	iRet = tbus_encode_head(a_pstHead, &szNet[0], &iHeadLen, 0);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to pack tbus head,iRet %x", iRet);
		return iRet;
	}
	if (iHeadLen > TBUS_HEAD_MAX_LEN)
	{
		tbus_log(TLOG_PRIORITY_FATAL,"length(%d) of tbus head packed is beyond the max scope of Head.bHeadLen", iHeadLen);
		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_TOO_BIG_HEAD);
	}
	a_pstHead->bHeadLen = (unsigned char)iHeadLen;

	/*计算消息队列空余长度*/
	iRoom = (int)pstVar->dwHead - (int)pstVar->dwTail - 1;
	if (iRoom < 0)
	{
		iRoom += pstVar->dwSize;
	}
	iPkgLen =(int)(a_pstHead->bHeadLen + a_pstHead->iBodyLen);
	if (iRoom < iPkgLen)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"channel(Dst:%s) is, full head %i, tail %i, freeroom:%d, pkglen:%d", 
			tbus_addr_ntoa(a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide]),
			pstVar->dwHead, pstVar->dwTail, iRoom, iPkgLen) ;
		return TBUS_ERR_CHANNEL_FULL;
	}

	
	/* store tbus head */
	pszQueue = a_pstChannel->pszQueues[a_pstChannel->iSendSide];
	dwTmpTail = pstVar->dwTail;
	if (pstVar->dwTail + iHeadLen > pstVar->dwSize) 
	{
		iTailRoom = pstVar->dwSize - dwTmpTail;
		memcpy(pszQueue + dwTmpTail, &szNet[0], iTailRoom);
		memcpy(pszQueue, &szNet[0] + iTailRoom, iHeadLen - iTailRoom);
		dwTmpTail = iHeadLen - iTailRoom;
	} else 
	{
		memcpy(pszQueue + dwTmpTail, &szNet[0], iHeadLen);
		dwTmpTail = (dwTmpTail + iHeadLen) % pstVar->dwSize;
	}


	/* store data */
	for ( i=0; i<a_iVecCnt; i++ )
	{
		char *pszData = (char *)a_ptVector[i].iov_base;
		int iDataLen = (int)a_ptVector[i].iov_len;
		if ((0 >= iDataLen) || (NULL == pszData))
		{
			continue;
		}
		if (dwTmpTail + iDataLen > pstVar->dwSize) 
		{
			iTailRoom = pstVar->dwSize - dwTmpTail;
			memcpy(pszQueue + dwTmpTail, pszData, iTailRoom);
			memcpy(pszQueue, pszData + iTailRoom, iDataLen - iTailRoom);
			dwTmpTail = iDataLen - iTailRoom;
		} else 
		{
			memcpy(pszQueue + dwTmpTail, pszData, iDataLen);
			dwTmpTail = (dwTmpTail + iDataLen) % pstVar->dwSize;
		}				
	}/*for ( i=0; i<a_iVecCnt; i++ )*/

	pstVar->dwTail = dwTmpTail;
	pstVar->iSeq++; /*序列号增加*/
	tbus_log(TLOG_PRIORITY_TRACE,"queue(%d --> %d) size:%d head:%d tail:%d headlen:%d bodyLen:%d", 
		a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],
		a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
		pstVar->dwSize, pstVar->dwHead, 
		pstVar->dwTail, iHeadLen, a_pstHead->iBodyLen);	

	return iRet;
}

#else /*#ifdef TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE*/


int tbus_push_channel_pkgv(IN TBUSCHANNEL* a_pstChannel, IN LPTBUSHEAD a_pstHead, 
						   IN const struct iovec *a_ptVector, IN const int a_iVecCnt)
{
	CHANNELVAR *pstVar = NULL;
	CHANNELHEAD *pstChlHead;
	int i;
	char szNet[TBUS_HEAD_CODE_BUFFER_SIZE];
	int iHeadLen = 0;
	int iRet = TBUS_SUCCESS;
	int iRoom;
	int iPkgLen;
	int iTailRoom;
	char *pszQueue = NULL ;

	char *pch;

	assert(NULL != a_pstChannel);
	assert(NULL != a_pstHead);
	assert(NULL != a_ptVector);

	
	pstVar = TBUS_CHANNEL_VAR_PUSH(a_pstChannel);
	pszQueue = TBUS_CHANNEL_QUEUE_PUSH(a_pstChannel);
	pstChlHead = a_pstChannel->pstHead;
	assert(NULL != pstChlHead);
	tbus_log(TLOG_PRIORITY_TRACE,"queue(0x%08x --> 0x%08x) size:%d head:%d tail:%d seq:%d", 
		pstChlHead->astAddr[a_pstChannel->iRecvSide],
		pstChlHead->astAddr[a_pstChannel->iSendSide],
		pstVar->dwSize, pstVar->dwHead, 
		pstVar->dwTail, pstVar->iSeq);

	/*统计数据长度，并构造tbus头部结构*/
	a_pstHead->iSeq = pstVar->iSeq;
	a_pstHead->bFlag &= ~TBUS_HEAD_FLAG_SYN ; /* avoid illegal bit set */
	a_pstHead->iBodyLen = 0;
	for ( i=0; i<a_iVecCnt; i++ )
	{
		if ((0 >= a_ptVector[i].iov_len) || (NULL == a_ptVector[i].iov_base))
		{
			continue;
		}
		a_pstHead->iBodyLen += (int)a_ptVector[i].iov_len;		
	}
	if (0 >= a_pstHead->iBodyLen)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid data len %d", a_pstHead->iBodyLen);
		return TBUS_ERR_ARG;
	}
	iHeadLen = sizeof(szNet);
	iRet = tbus_encode_head(a_pstHead, &szNet[0], &iHeadLen, 0);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to pack tbus head,iRet %x", iRet);
		return iRet;
	}
	assert(iHeadLen <= TBUS_HEAD_MAX_LEN);	
	a_pstHead->bHeadLen = (unsigned char)iHeadLen;


	/*计算消息队列空余长度，并判断剩余空间是否足够保存此消息*/
	iRoom = (int)pstVar->dwHead - (int)pstVar->dwTail - 1;
	if (iRoom < 0)
	{
		iRoom += pstVar->dwSize;
	}
	iPkgLen =(int)(a_pstHead->bHeadLen + a_pstHead->iBodyLen);
	TBUS_CALC_ALIGN_VALUE_BY_LEVEL(iPkgLen, pstChlHead->dwAlignLevel);
	if (0 >= iPkgLen)
	{
		/*快速计算对齐方式溢出，只能用简单费时的方法计算*/
		iPkgLen =(int)(a_pstHead->bHeadLen + a_pstHead->iBodyLen);
		TBUS_CALC_ALIGN_VALUE(iPkgLen, (1<<pstChlHead->dwAlignLevel));
		tbus_log(TLOG_PRIORITY_WARN,"failed to calc pkglen(%d) by align level :%d, so change the calc method", 
			(a_pstHead->bHeadLen + a_pstHead->iBodyLen), pstChlHead->dwAlignLevel) ;
	}
	tbus_log(TLOG_PRIORITY_DEBUG, "by alignlevel(%d) pkglen from %d change to %d",
		pstChlHead->dwAlignLevel, (int)(a_pstHead->bHeadLen + a_pstHead->iBodyLen),iPkgLen);
	if (iRoom < iPkgLen)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"channel(Dst:%s) is, full head %i, tail %i, freeroom:%d, pkglen:%d", 
			tbus_addr_ntoa(TBUS_CHANNEL_PEER_ADDR(a_pstChannel)),
			pstVar->dwHead, pstVar->dwTail, iRoom, iPkgLen) ;
		return TBUS_ERR_CHANNEL_FULL;
	}


	/*检查队列末尾剩余空间是否能保存下整个数据,如果尾部剩余空间不够保存完整消息，则将尾指针调整到队列最前面
	*/
	iTailRoom = pstVar->dwSize - pstVar->dwTail;
	if (iTailRoom < iPkgLen)
	{
		if (iTailRoom > g_stBusGlobal.iCounterfeitPkgLen)
		{
			/*如果对尾剩余空间还可以保存一个伪造消息，则放置一个假消息*/
			tbus_log(TLOG_PRIORITY_INFO, "the queue tail room(%d) is bigger than the counterfeit pkglen(%d), "
				"so push counterfeit pkg into the queue", iTailRoom, g_stBusGlobal.iCounterfeitPkgLen);
			memcpy(pszQueue + pstVar->dwTail, &g_stBusGlobal.stCounterfeitPkg, g_stBusGlobal.iCounterfeitPkgLen);
		}/*if (iTailRoom > g_stBusGlobal.iCounterfeitPkgLen)*/
		
		tbus_log(TLOG_PRIORITY_INFO,"the queue tail room(%d) is less than pkg len(%d), so need move tail to zero",
			iTailRoom, iPkgLen);
		pstVar->dwTail = 0;

		/*重新计算队列剩余空间，并检查剩余空空是否足够保存一个完整消息*/
		iRoom = (int)pstVar->dwHead - (int)pstVar->dwTail - 1;
		if (iRoom < iPkgLen)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"channel(Dst:%s) is, full head %i, tail %i, freeroom:%d, pkglen:%d", 
				tbus_addr_ntoa(TBUS_CHANNEL_PEER_ADDR(a_pstChannel)),
				pstVar->dwHead, pstVar->dwTail, iRoom, iPkgLen) ;
			return TBUS_ERR_CHANNEL_FULL;
		}
	}/*if (iTailRoom < iPkgLen)*/


	/*保存整个消息，先保存tbus头部信息*/
	pch = pszQueue + pstVar->dwTail;
	memcpy(pch, &szNet[0], iHeadLen);
	pch += iHeadLen;


	/* store data */
	for ( i=0; i<a_iVecCnt; i++ )
	{
		char *pszData = (char *)a_ptVector[i].iov_base;
		int iDataLen = (int)a_ptVector[i].iov_len;
		if ((0 >= iDataLen) || (NULL == pszData))
		{
			continue;
		}
		memcpy(pch, pszData, iDataLen);
		pch += iDataLen;						
	}/*for ( i=0; i<a_iVecCnt; i++ )*/

	pstVar->dwTail += iPkgLen;
	pstVar->iSeq++; /*序列号增加*/

	tbus_log(TLOG_PRIORITY_TRACE,"queue(0x%08x --> 0x%08x) size:%d head:%d tail:%d headlen:%d bodyLen:%d pkglen:%d", 
		pstChlHead->astAddr[a_pstChannel->iRecvSide],
		pstChlHead->astAddr[a_pstChannel->iSendSide],
		pstVar->dwSize, pstVar->dwHead, 
		pstVar->dwTail, iHeadLen, a_pstHead->iBodyLen, iPkgLen);	

	return iRet;
}
#endif /*#ifdef TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE*/

#ifdef TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE
int	tbus_get_channel_pkgv(IN TBUSCHANNEL* a_pstChannel, OUT LPTBUSHEAD a_pstHead,
						  IN char *a_pszData, INOUT int *a_piLen)
{
	int iRet = TBUS_SUCCESS;
	CHANNELVAR *pstVar = NULL;
	char *pszQueue;
	int iPkgLen = 0,
		iDataLen = 0,
		iTailLen = 0 ;
	unsigned int dwHead;

	assert(NULL != a_pstChannel);
	assert(NULL != a_pstHead);
	assert(NULL != a_pszData);
	assert(NULL != a_piLen);

	pstVar = &a_pstChannel->pstHead->astQueueVar[a_pstChannel->iRecvSide];	
	if (pstVar->dwHead == pstVar->dwTail) 
	{
		return TBUS_ERR_CHANNEL_EMPTY;
	}
	tbus_log(TLOG_PRIORITY_TRACE,"queue(%d <-- %d) size:%d head:%d tail:%d ", 
		a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],
		a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
		pstVar->dwSize, pstVar->dwHead, 
		pstVar->dwTail);
	
	/*get head */
	pszQueue = a_pstChannel->pszQueues[a_pstChannel->iRecvSide];
	iRet = tbus_get_pkghead(a_pstHead, pszQueue, pstVar->dwSize, &pstVar->dwHead, pstVar->dwTail);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to get pkg head length, iRet %x", iRet);
		if (iRet != TBUS_ERR_CHANNEL_EMPTY)
		{
			tbus_log(TLOG_PRIORITY_FATAL,"failed to get tbus head from channel queue(%d <-- %d), iRet %x, so clear remain data",
				a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide], 
				a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide], iRet);
			pstVar->dwHead = pstVar->dwTail;
		}
		return iRet;
	}/*if (TBUS_SUCCESS != iRet)*/
	

	/*get body*/
	iPkgLen = a_pstHead->bHeadLen + a_pstHead->iBodyLen;
	iDataLen = (int)(pstVar->dwTail - pstVar->dwHead);
	if (iDataLen < 0) 
	{
		iDataLen += pstVar->dwSize;
	}
	if (iDataLen < iPkgLen)
	{
		tbus_log(TLOG_PRIORITY_FATAL,"data length(%d) in channel(src:%d, dst:%d) is less than the length(%d) of tbus pkg, so clear remain data",
			iDataLen, a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
			a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],iPkgLen);
		pstVar->dwHead = pstVar->dwTail;
		return TBUS_ERR_CHANNEL_CONFUSE;	
	}
	if (*a_piLen < (int)a_pstHead->iBodyLen)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"the size(%d) of buffer to recv data is less than the length(%d) of data",
			 *a_piLen, a_pstHead->iBodyLen);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_RECV_BUFFER_LIMITED);
	}

	/*copy body*/
	dwHead = (pstVar->dwHead + a_pstHead->bHeadLen) % pstVar->dwSize;
	iTailLen = pstVar->dwSize - dwHead;
	*a_piLen = a_pstHead->iBodyLen;
	if (iTailLen < *a_piLen) 
	{
		memcpy(a_pszData, pszQueue + dwHead, iTailLen);
		memcpy(a_pszData + iTailLen, pszQueue, *a_piLen - iTailLen);
	} else  
	{
		memcpy(a_pszData, pszQueue + dwHead, *a_piLen);
	}
	pstVar->dwHead = (dwHead + *a_piLen) % pstVar->dwSize;

	tbus_log(TLOG_PRIORITY_TRACE,"queue(%d <-- %d) size:%d head:%d tail:%d headlen:%d bodylen:%d", 
		a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],
		a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
		pstVar->dwSize, pstVar->dwHead, 
		pstVar->dwTail, a_pstHead->bHeadLen, *a_piLen);

	return 0;

}

#else /*TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE*/


int	tbus_peek_channel_pkgv(IN TBUSCHANNEL* a_pstChannel, OUT LPTBUSHEAD a_pstHead,
						  IN const char **a_ppszData, OUT int *a_piLen)
{
	int iRet = TBUS_SUCCESS;
	CHANNELVAR *pstVar = NULL;
	char *pszQueue;
	int iPkgLen = 0,
		iDataLen = 0;


	CHANNELHEAD *pstChlHead;

	assert(NULL != a_pstChannel);
	assert(NULL != a_pstHead);
	assert(NULL != a_ppszData);
	assert(NULL != a_piLen);

	pstVar = TBUS_CHANNEL_VAR_GET(a_pstChannel);	
	if (pstVar->dwHead == pstVar->dwTail) 
	{
		return TBUS_ERR_CHANNEL_EMPTY;
	}

	pstChlHead = a_pstChannel->pstHead;
	pszQueue = TBUS_CHANNEL_QUEUE_GET(a_pstChannel);
	tbus_log(TLOG_PRIORITY_TRACE,"queue(0x%08x <-- 0x%08x) size:%d head:%d tail:%d ", 
		pstChlHead->astAddr[a_pstChannel->iRecvSide],
		pstChlHead->astAddr[a_pstChannel->iSendSide],
		pstVar->dwSize, pstVar->dwHead, 
		pstVar->dwTail);

	/*检查从头指针到队列末尾是否足够保存一条消息*/
	TBUS_CHECK_QUEUE_HEAD_VAR(pszQueue, pstVar->dwSize, pstVar->dwHead);
	
	/*再次检查数据队列中是否有数据*/
	iDataLen = (int)(pstVar->dwTail - pstVar->dwHead);
	if (iDataLen < 0) 
	{
		iDataLen += pstVar->dwSize;
	}
	if (0 >= iDataLen) 
	{   
		return TBUS_ERR_CHANNEL_EMPTY;
	}

	/*get head */
	iRet = tbus_decode_head(a_pstHead, pszQueue +pstVar->dwHead, iDataLen, 0);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to unpack tbus head from channel queue(0x%08x <-- 0x%08x),iRet %x", 
			TBUS_CHANNEL_LOCAL_ADDR(a_pstChannel),TBUS_CHANNEL_PEER_ADDR(a_pstChannel),
			iRet);		
		pstVar->dwHead = pstVar->dwTail;
		return iRet;
	}	


	/*检查数据长度与与队列中数据长度是否有效*/
	iPkgLen = a_pstHead->bHeadLen + a_pstHead->iBodyLen;
	if ((0 >= iPkgLen) || (iPkgLen > (int)pstVar->dwSize))
	{
		tbus_log_data(TLOG_PRIORITY_FATAL, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));
		tbus_log(TLOG_PRIORITY_FATAL,"queue(0x%08x <-- 0x%08x) size:%d head:%d tail:%d ", 
			pstChlHead->astAddr[a_pstChannel->iRecvSide],
			pstChlHead->astAddr[a_pstChannel->iSendSide],
			pstVar->dwSize, pstVar->dwHead, 
			pstVar->dwTail);
		tbus_log(TLOG_PRIORITY_FATAL,"invalid pkg length(%d) in channel(src:0x%08x, dst:0x%08x), "
			"its should be bigger than 0 and less than queuesize(%d),so clear remain data",
			iPkgLen, a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
			a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],pstVar->dwSize);
		pstVar->dwHead = pstVar->dwTail;
		return TBUS_ERR_CHANNEL_CONFUSE;
	}
	TBUS_CALC_ALIGN_VALUE_BY_LEVEL(iPkgLen, pstChlHead->dwAlignLevel);
	tbus_log(TLOG_PRIORITY_DEBUG, "by alignlevel(%d) pkglen from %d change to %d, iHeadLen:%d",
		pstChlHead->dwAlignLevel, (int)(a_pstHead->bHeadLen + a_pstHead->iBodyLen),
		iPkgLen,a_pstHead->bHeadLen);

	if (iDataLen < iPkgLen) 
	{
		tbus_log(TLOG_PRIORITY_FATAL,"data length(%d) in channel(src:0x%08x, dst:0x%08x) is less than the length(%d) of tbus pkg, so clear remain data",
			iDataLen, a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
			a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],iPkgLen);
		pstVar->dwHead = pstVar->dwTail;
		return TBUS_ERR_CHANNEL_CONFUSE;	
	}
	
	/*设置数据的位置及长度信息*/
	*a_ppszData = pszQueue + pstVar->dwHead + a_pstHead->bHeadLen;
	*a_piLen = a_pstHead->iBodyLen;

	return 0;

}

int tbus_moveto_next_pkg(IN IN LPTBUSHEAD a_pstHead, CHANNELHEAD *a_pstChlHead, IN unsigned int dwSize,
					 INOUT unsigned int *a_pdwHead, IN unsigned int dwTail)
{
	int iRet = TBUS_SUCCESS;




	int iPkgLen = 0;
	int iDataLen = 0;
	unsigned int dwHead;

	assert(NULL != a_pstHead);
	assert(NULL != a_pstChlHead);
	assert(NULL != a_pdwHead);


	dwHead = *a_pdwHead;
	if (dwHead == dwTail)
	{
		return 0;
	}
	tbus_log(TLOG_PRIORITY_TRACE,"queue size:%d head:%d tail:%d ", 
		dwSize, dwHead, dwTail);

	iPkgLen = a_pstHead->bHeadLen + a_pstHead->iBodyLen;
	TBUS_CALC_ALIGN_VALUE_BY_LEVEL(iPkgLen, a_pstChlHead->dwAlignLevel);
	tbus_log(TLOG_PRIORITY_DEBUG, "by alignlevel(%d) pkglen from %d change to %d, iHeadLen:%d",
		a_pstChlHead->dwAlignLevel, (int)(a_pstHead->bHeadLen + a_pstHead->iBodyLen),
		iPkgLen,a_pstHead->bHeadLen);
	iDataLen = dwTail - dwHead;
	if (iDataLen < 0) 
	{
		iDataLen += dwSize;
	}
	if (iDataLen < iPkgLen) 
	{
		tbus_log(TLOG_PRIORITY_FATAL,"data length(%d) in queue is less than the length(%d) of tbus pkg",
			iDataLen, iPkgLen);
		return TBUS_ERR_CHANNEL_CONFUSE;	
	}
	if ((0 >= iPkgLen) || (iPkgLen >= (int)dwSize))
	{
		tbus_log_data(TLOG_PRIORITY_FATAL, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));
		tbus_log(TLOG_PRIORITY_FATAL,"queue size:%d head:%d tail:%d ", 
			dwSize, *a_pdwHead, dwTail);
		tbus_log(TLOG_PRIORITY_FATAL,"invalid pkg length(%d) in queue, "
			"its should be bigger than 0 and less than queuesize(%d),so clear remain data",
			iPkgLen, dwSize);
		return TBUS_ERR_CHANNEL_CONFUSE;
	}

	/*修改头指针，以将数据包移除*/
	*a_pdwHead += iPkgLen;
	tbus_log(TLOG_PRIORITY_TRACE,"queue size:%d head:%d tail:%d ", 
		dwSize, *a_pdwHead, dwTail);


	return iRet;
}

int tbus_delete_channel_headpkg(IN TBUSCHANNEL* a_pstChannel)
{
	int iRet = TBUS_SUCCESS;
	TBUSHEAD stTmpHead;
	CHANNELVAR *pstVar = NULL;
	char *pszQueue;
	CHANNELHEAD *pstChlHead;



	assert(NULL != a_pstChannel);

	pstVar = TBUS_CHANNEL_VAR_GET(a_pstChannel);	
	if (pstVar->dwHead == pstVar->dwTail) 
	{
		return TBUS_ERR_CHANNEL_EMPTY;
	}
	pstChlHead = a_pstChannel->pstHead;
	pszQueue = TBUS_CHANNEL_QUEUE_GET(a_pstChannel);
	tbus_log(TLOG_PRIORITY_TRACE,"queue(0x%08x <-- 0x%08x) size:%d head:%d tail:%d ", 
		pstChlHead->astAddr[a_pstChannel->iRecvSide],
		pstChlHead->astAddr[a_pstChannel->iSendSide],
		pstVar->dwSize, pstVar->dwHead, 
		pstVar->dwTail);


	iRet = tbus_get_pkghead( &stTmpHead, pszQueue, pstVar->dwSize, (unsigned int *)&pstVar->dwHead, pstVar->dwTail);
	if (0 != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR, "failed to get tbus  head iRet:%x (queue: size:%u Head:%d tail:%u)", 
			iRet, pstVar->dwSize, pstVar->dwHead, pstVar->dwTail);
	}


	if (0 == iRet)
	{
		iRet = tbus_moveto_next_pkg( &stTmpHead, pstChlHead, pstVar->dwSize, (unsigned int *)&pstVar->dwHead, pstVar->dwTail);
	}else
	{
		tbus_log(TLOG_PRIORITY_FATAL,"failed to move to next  pkg in channel(src:0x%08x, dst:0x%08x), "
			"so clear remain data",
			a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide],
			a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide],pstVar->dwSize);
		pstVar->dwHead = pstVar->dwTail;
	}


	tbus_log(TLOG_PRIORITY_TRACE,"queue(0x%08x <-- 0x%08x) size:%d head:%d tail:%d ", 
		pstChlHead->astAddr[a_pstChannel->iRecvSide],
		pstChlHead->astAddr[a_pstChannel->iSendSide],
		pstVar->dwSize, pstVar->dwHead, 
		pstVar->dwTail);


	return iRet;
}

int	tbus_get_channel_pkgv(IN TBUSCHANNEL* a_pstChannel, OUT LPTBUSHEAD a_pstHead,
						  IN char *a_pszData, INOUT int *a_piLen)
{
	int iRet = TBUS_SUCCESS;
	const char *pszGetData = NULL;
	CHANNELVAR *pstVar = NULL;
	int iDataLen = 0;



	assert(NULL != a_pstChannel);
	assert(NULL != a_pstHead);
	assert(NULL != a_pszData);
	assert(NULL != a_piLen);

	iRet = tbus_peek_channel_pkgv(a_pstChannel, a_pstHead, &pszGetData, &iDataLen);
	if (0 != iRet)
	{
		if (TBUS_ERR_CHANNEL_EMPTY == iRet)
		{
			tbus_log(TLOG_PRIORITY_TRACE,"no pkg in the channel");
		}else
		{
			tbus_log(TLOG_PRIORITY_ERROR,"failed to peek pkg iRet %x", iRet);
		}
		
		return iRet;
	}
	
	if (*a_piLen < iDataLen)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"the size(%d) of buffer to recv data is less than the length(%d) of data",
			*a_piLen, iDataLen);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_RECV_BUFFER_LIMITED);
	}

	/*copy body*/
	memcpy(a_pszData, pszGetData, iDataLen);
	*a_piLen = iDataLen;

	pstVar = TBUS_CHANNEL_VAR_GET(a_pstChannel);
	iRet = tbus_moveto_next_pkg(a_pstHead, a_pstChannel->pstHead, pstVar->dwSize, (unsigned int *)&pstVar->dwHead, pstVar->dwTail);
	
	return 0;

}

#endif /*TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE*/

int tbus_open_channel(INOUT LPTBUSCHANNEL *a_ppstChannel, IN TBUSADDR a_iLocalAddr,
					  IN TBUSADDR a_iPeerAddr,IN int a_iShmkey, IN int a_iSize)
{
	int iRet = TBUS_SUCCESS;
	LPTBUSCHANNEL pstChannel = NULL;
	TBUSSHMCHANNELCNF stChannelCnf;
	int iFlag;
	int iSize;
	char *sShm;
	int iCreated = 0;
	CHANNELHEAD *pstHead ;
	int iChannelHeadLen;

	//assert(NULL != a_ppstChannel);
	//assert(0 < a_iSize);
	if ((NULL == a_ppstChannel)||(0 >= a_iSize))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid param: a+ppstChannel:%p a_iSize:%d",
			a_ppstChannel, a_iSize);
		return TBUS_ERR_ARG;
	}

	if (NULL == g_pstBusLogCat)
	{
		iRet = tbus_init_log();
		if (0 != iRet)
		{
			printf("failed to init log system");
			return iRet;
		}
	}
	iRet = tbus_init_headmeta();
	if (TBUS_SUCCESS != iRet)
	{
		printf("failed to get meta of bus head, iRet %x", iRet);
		return iRet;
	}

	
	memset(&stChannelCnf, 0, sizeof(stChannelCnf));
	if (a_iLocalAddr < a_iPeerAddr)
	{
		stChannelCnf.astAddrs[TBUS_CHANNEL_SIDE_INDEX_1] = a_iLocalAddr;
		stChannelCnf.astAddrs[TBUS_CHANNEL_SIDE_INDEX_2] = a_iPeerAddr;
	}else
	{
		stChannelCnf.astAddrs[TBUS_CHANNEL_SIDE_INDEX_2] = a_iLocalAddr;
		stChannelCnf.astAddrs[TBUS_CHANNEL_SIDE_INDEX_1] = a_iPeerAddr;
	}
	TBUS_CALC_ALIGN_VALUE(a_iSize, TBUS_DEFAULT_CHANNEL_DATA_ALIGN);
	stChannelCnf.dwRecvSize = a_iSize;
	stChannelCnf.dwSendSize = a_iSize;


	iFlag = 0666 | IPC_CREAT;
	iChannelHeadLen = sizeof(CHANNELHEAD);
	TBUS_CALC_ALIGN_VALUE(iChannelHeadLen, TBUS_DEFAULT_CHANNEL_DATA_ALIGN);
	iSize = iChannelHeadLen + a_iSize*TBUS_CHANNEL_SIDE_NUM;
	iRet = tbus_auto_get_shm ((void**)&sShm, &iCreated, a_iShmkey, &iSize, iFlag);
	if ( iRet < 0 )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"ERROR: tbus_attach_channel, getShmAny() failed %i", iRet ) ;
		return iRet;
	}

	pstChannel = (LPTBUSCHANNEL)calloc(1, sizeof(TBUSCHANNEL));
	if (NULL == pstChannel)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to malloc memory for charnnel\n");
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_MEMORY);
	}
	pstChannel->pstHead = (CHANNELHEAD *)sShm;
	pstChannel->pszQueues[TBUS_CHANNEL_SIDE_INDEX_1] = sShm + iChannelHeadLen;
	pstChannel->pszQueues[TBUS_CHANNEL_SIDE_INDEX_2] = sShm + iChannelHeadLen + stChannelCnf.dwRecvSize;
	pstHead = pstChannel->pstHead;
	if ( 0 != iCreated )
	{
		/* first inited */
		int i;
		memset(pstHead, 0, sizeof(CHANNELHEAD));
		for (i = 0; i < TBUS_CHANNEL_SIDE_NUM; i++)
		{
			pstHead->astAddr[i] = stChannelCnf.astAddrs[i];
			pstHead->astQueueVar[i].dwSize =  a_iSize;
		}
		TBUS_CALC_ALIGN_LEVEL(pstHead->dwAlignLevel,TBUS_DEFAULT_CHANNEL_DATA_ALIGN) ;
		tbus_log(TLOG_PRIORITY_FATAL,"init bus channel shm(%d, addr:%p),size %i", 
			a_iShmkey, sShm, iSize ) ;
	}else
	{		
		/* queue share memory exists */
		if (pstHead->astAddr[0] != stChannelCnf.astAddrs[0] || pstHead->astAddr[1] != stChannelCnf.astAddrs[1]) 
		{
			tbus_log(TLOG_PRIORITY_ERROR,"Channel config ERROR!\n");
			tbus_log(TLOG_PRIORITY_ERROR,"address1 in Channel Shm : %s   ", tbus_addr_ntoa(pstHead->astAddr[0]));
			tbus_log(TLOG_PRIORITY_ERROR,"address2 in Channel Shm : %s", tbus_addr_ntoa(pstHead->astAddr[1]));
			tbus_log(TLOG_PRIORITY_ERROR,"address1 in GCIM : %s   ", tbus_addr_ntoa(stChannelCnf.astAddrs[0]));
			tbus_log(TLOG_PRIORITY_ERROR,"address1 in GCIM : %s", tbus_addr_ntoa(stChannelCnf.astAddrs[1]));
			free(pstChannel);
			pstChannel = NULL;
			return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHANNEL_ADDRESS_CONFLICT);
		}		
	}/*if ( 0 == iCreated )*/

	/* set channel side: a_iAddr is source address*/
	if ( a_iLocalAddr == pstHead->astAddr[TBUS_CHANNEL_SIDE_INDEX_1] )
	{
		pstChannel->iRecvSide = TBUS_CHANNEL_SIDE_INDEX_1 ;
		pstChannel->iSendSide = TBUS_CHANNEL_SIDE_INDEX_2 ;
	}else
	{
		pstChannel->iRecvSide = TBUS_CHANNEL_SIDE_INDEX_2 ;
		pstChannel->iSendSide = TBUS_CHANNEL_SIDE_INDEX_1;
	}
	*a_ppstChannel = pstChannel;
		

	return iRet;
	
}

int tbus_open_channel_by_str(INOUT LPTBUSCHANNEL *a_ppstChannel, IN const char *a_pszLocalAddr,
							 IN const char *a_pszPeerAddr,IN int a_iShmkey, IN int a_iSize)
{
	unsigned int dwLocal=0;
	unsigned int dwPeer=0;


	//assert(NULL != a_pszLocalAddr);
	//assert(NULL != a_pszPeerAddr);
	if ((NULL == a_ppstChannel)||(NULL ==  a_pszLocalAddr)||(NULL == a_pszPeerAddr)||
		(0 >= a_iSize))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid param: a_ppstChannel:%p a_pszLocalAddr:%p "
			"a_pszPeerAddr: %p a_iSize:%d",
			a_ppstChannel, a_pszLocalAddr, a_pszPeerAddr, a_iSize);
		return TBUS_ERR_ARG;
	}


	/*转换地址*/
	if (0 == inet_aton((char *)a_pszLocalAddr,  (struct in_addr *)&dwLocal))
	{
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ADDR_STRING);	
	}
	if (0 == inet_aton((char *)a_pszPeerAddr,  (struct in_addr *)&dwPeer))
	{
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ADDR_STRING);	
	}

	return  tbus_open_channel(a_ppstChannel, dwLocal, dwPeer, a_iShmkey, a_iSize);

}


/**
@brief 在指定通道上发送一个数据包
*	@param[in] a_pstChannel 通道句柄
*	@param[in] a_pvData 保存数据的缓冲区首地址
*	@param[in] a_iDataLen 数据长度
*	@param[in] a_iFlag 控制字段
*	@retval 0 success, 
*	@retval !0 failed.
*/
int tbus_channel_send(IN LPTBUSCHANNEL a_pstChannel,IN const void *a_pvData,  
					  IN const int a_iDataLen, IN const int a_iFlag) 
{
	int iRet = TBUS_SUCCESS;
	TBUSHEAD stHead ;
	struct iovec astVectors[1];


	if ((NULL == a_pstChannel) || (NULL == a_pvData) ||
		(0 >= a_iDataLen))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, parameter error, datalen(%i)", a_iDataLen ) ;
		return TBUS_ERR_ARG ;
	}
	
	/*构造tbus消息*/
	memset(&stHead, 0, sizeof(TBUSHEAD));
	stHead.bCmd = TBUS_HEAD_CMD_TRANSFER_DATA;
	stHead.iSrc = a_pstChannel->pstHead->astAddr[a_pstChannel->iRecvSide];
	stHead.iDst = a_pstChannel->pstHead->astAddr[a_pstChannel->iSendSide];
	tbus_dye_pkg(&stHead, NULL, a_iFlag);

	astVectors[0].iov_base = (void *)a_pvData;
	astVectors[0].iov_len = a_iDataLen;

	iRet = tbus_push_channel_pkgv(a_pstChannel, &stHead, &astVectors[0], 1) ;
	if (iRet != 0)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"tbus_push_channel_pkgv() failed, iRet", iRet ) ;			
	}else
	{
		tbus_log(TLOG_PRIORITY_DEBUG,"DEBUG: tbus_channel_send ok, source address %s", 
		inet_ntoa(*(struct in_addr *)&stHead.iSrc)) ;
	}
	if(stHead.bFlag & TBUS_HEAD_FLAG_TACE)
	{
		tbus_log_dyedpkg(&stHead, "channel_send");
	}
	
	return iRet;
}

/**
@brief 在指定通道上接受一个数据包
*	@param[in] a_pstChannel 通道句柄
*	@param[in] a_pvData 保存数据的缓冲区首地址
*	@param[in,out] a_iDataLen 保存数据长度的指针
*	in	-	接受缓冲区最大大小
*	out -	接受数据的实际大小
*	@param[in] a_iFlag 控制字段
*	@retval 0 success, 
*	@retval !0 failed.
*/
int tbus_channel_recv(IN LPTBUSCHANNEL a_pstChannel, INOUT void *a_pvData,
					  INOUT int *a_piDataLen,
					  IN const int a_iFlag) 
{
	int iRet = TBUS_SUCCESS;
	TBUSHEAD stHead ;


	if ((NULL == a_pstChannel) || (NULL == a_pvData) ||
		(NULL == a_piDataLen) || (0 >= *a_piDataLen))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, parameter error, datalen(%i)", *a_piDataLen) ;
		return TBUS_ERR_ARG ;
	}
	
	iRet = tbus_get_channel_pkgv(a_pstChannel, &stHead, a_pvData, a_piDataLen) ;
	if (iRet != 0)
	{
		if (TBUS_ERR_CHANNEL_EMPTY != iRet)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"tbus_get_channel_pkgv() failed, iRet %x", iRet ) ;
		}else
		{
			tbus_log(TLOG_PRIORITY_TRACE,"no pkg in the channel") ;
		}
					
	}else
	{
		tbus_log(TLOG_PRIORITY_DEBUG,"DEBUG: tbus_channel_recv ok, source address %s", 
			inet_ntoa(*(struct in_addr *)&stHead.iSrc)) ;
	}
	tbus_dye_pkg(&stHead, NULL, a_iFlag);
	if(stHead.bFlag & TBUS_HEAD_FLAG_TACE)
	{
		tbus_log_dyedpkg(&stHead, "channel_recv");
	}
		
	return iRet;
}

int tbus_channel_peek_msg(IN LPTBUSCHANNEL a_pstChannel, INOUT const char **a_ppvData,	OUT int *a_piDataLen,
									IN const int a_iFlag)
{
	int iRet = TBUS_SUCCESS;
	TBUSHEAD stHead ;


	if ((NULL == a_pstChannel) || (NULL == a_piDataLen) ||
		(NULL == a_piDataLen) )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, parameter error") ;
		return TBUS_ERR_ARG ;
	}	

	iRet = tbus_peek_channel_pkgv(a_pstChannel, &stHead, a_ppvData, a_piDataLen);

	tbus_dye_pkg(&stHead, NULL, a_iFlag);
	if(stHead.bFlag & TBUS_HEAD_FLAG_TACE)
	{
		tbus_log_dyedpkg(&stHead, "tbus_channel_peek_msg");
	}
	
	return iRet;

}

int tbus_channel_delete_msg(IN LPTBUSCHANNEL a_pstChannel)
{
	int iRet = TBUS_SUCCESS;

	if (NULL == a_pstChannel)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, parameter error") ;
		return TBUS_ERR_ARG ;
	}	

	iRet = tbus_delete_channel_headpkg(a_pstChannel);

	return iRet;
}

/**
@brief 关闭一个通信通道
*	@param[in,out] a_ppstChannel 保存打开的通道句柄的指针
*	@retval 0 success, 
*	@retval !0 failed.
@note a_ppstChannel所指的通道句柄必须是调用tbus_open_channel打开的
*	@see tbus_open_channel
*/
void tbus_close_channel(INOUT LPTBUSCHANNEL *a_ppstChannel) 
{
	if (NULL != a_ppstChannel)
	{
		if (NULL != *a_ppstChannel)
		{
			LPTBUSCHANNEL pstChannel = *a_ppstChannel;
			shmdt((void *)pstChannel->pstHead);
			free(pstChannel);
			*a_ppstChannel = NULL;
		}
	}
}

int tbus_get_channel( IN const int a_iHandle, OUT LPTBUSCHANNEL *a_ppstChannel, IN TBUSADDR a_iSrcAddr, IN TBUSADDR a_iDstAddr)
{
	int iRet = TBUS_SUCCESS;
	int iChannelCnt;
	TBUSCHANNEL *pstChannel = NULL ;
	LPTBUSHANDLE pstHandle;
	int i;
	
	//assert(NULL != a_ppstChannel);
	if (NULL == a_ppstChannel)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"a_ppstChnnel is null cannot get channel by addr(src:%u dst :%u)",
			a_iSrcAddr, a_iDstAddr);
		return TBUS_ERR_ARG;
	}

	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}
	iChannelCnt = pstHandle->dwChannelCnt ;
	if ( 0 >= iChannelCnt )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"tbus_peer_ctrl(), no available address" ) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
	}

	for (i = 0; i < iChannelCnt; i++)
	{
		CHANNELHEAD *pstHead ;
		pstChannel = pstHandle->pastChannelSet[i];
		if (!(pstChannel->dwFlag & TBUS_CHANNEL_FLAG_ENABLE) )
		{
			tbus_log(TLOG_PRIORITY_TRACE,"tbus_peer_ctrl(), address info disable %i", i ) ;
			continue ;
		}
		if (!(pstChannel->dwFlag & TBUS_CHANNEL_FLAG_SRC_ENABLE) )
		{
			tbus_log(TLOG_PRIORITY_TRACE,"tbus_peer_ctrl(), source address disable %i", i ) ;
			continue ;
		}
		if (!(pstChannel->dwFlag & TBUS_CHANNEL_FLAG_DST_ENABLE) )
		{
			tbus_log(TLOG_PRIORITY_TRACE,"tbus_peer_ctrl(), peer address disable %i", i ) ;
			continue ;
		}
		pstHead = pstChannel->pstHead;
		if ((a_iSrcAddr == pstHead->astAddr[pstChannel->iRecvSide]) &&
			(a_iDstAddr == pstHead->astAddr[pstChannel->iSendSide]))
		{
			break;
		}
	}/*for (i = 0; i < iChannelCnt; i++)*/

	if (i >= iChannelCnt)
	{
		*a_ppstChannel = NULL;
		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL);
	}else
	{
		*a_ppstChannel = pstHandle->pastChannelSet[i];
	}

	return iRet;
}

int tbus_enable_addr (
					  IN const TBUSADDR a_iAddr,
					  IN TBUSSHMCHANNELCNF *a_ptRoute,
					  IN LPTBUSHANDLE a_pstBusHandle,
					  IN LPTBUSSHMGCIMHEAD a_pstHead
					  )
{
	int iRet = TBUS_SUCCESS ;
	unsigned int i;
	CHANNELHEAD *pstHead;
	LPTBUSCHANNEL  pstChannel;

	assert(NULL != a_ptRoute);
	assert(NULL != a_pstBusHandle);
	assert(NULL != a_pstHead);

	/*检查此通道是否已经在tbus通道管理域中*/
	for (i = 0; i < a_pstBusHandle->dwChannelCnt; i++)
	{
		pstChannel = a_pstBusHandle->pastChannelSet[i];
		pstHead = pstChannel->pstHead;
		if ((pstHead->astAddr[0] == a_ptRoute->astAddrs[0]) && 
			(pstHead->astAddr[1] == a_ptRoute->astAddrs[1]))
		{
			TBUS_CHANNEL_CLR_NOT_IN_GCIM(pstChannel);
			TBUS_CHANNEL_SET_ENABLE(pstChannel);
			break;
		}
	}/*for (i = 0; i < a_pstBusHandle->dwChannelCnt; i++)*/
	if (i < a_pstBusHandle->dwChannelCnt)
	{
		return TBUS_SUCCESS;
	}


	/*不存在则添加到tbus句柄中*/
	if (TUBS_MAX_CHANNEL_COUNT_PREHANDLE <= a_pstBusHandle->dwChannelCnt)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"tbus_bind, num<%d> of binded channel reach max num(%d) perhandle, so cannot bind any more channel(SrcAddr:%s)",
			a_pstBusHandle->dwChannelCnt, TUBS_MAX_CHANNEL_COUNT_PREHANDLE, tbus_addr_ntoa(a_iAddr)) ;	
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHANNEL_NUM_LIMITED);
	}

	/*新分配一个通道管理句柄*/
	pstChannel = (TBUSCHANNEL *) malloc ( sizeof(TBUSCHANNEL) ) ;
	if (NULL == pstChannel)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to malloc memory(size:%d)", sizeof(TBUSCHANNEL));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_MEMORY);
	}

	memset(pstChannel, 0, sizeof(TBUSCHANNEL) ) ;
	
	
	pstChannel->dwFlag |= TBUS_CHANNEL_FLAG_ENABLE ;
	pstChannel->dwFlag |= TBUS_CHANNEL_FLAG_SRC_ENABLE ;
	pstChannel->dwFlag |= TBUS_CHANNEL_FLAG_DST_ENABLE ;


	iRet = tbus_attach_channel ( a_iAddr, a_ptRoute, pstChannel, a_pstHead) ;
	if (0 != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"tbus_attach_channel failed, iRet %d", iRet);
		free(pstChannel);
	}else
	{
		char szTmpIP[128];
		CHANNELHEAD *pstHead = pstChannel->pstHead;
		STRNCPY(szTmpIP, tbus_addr_ntoa(pstHead->astAddr[0]), sizeof(szTmpIP));
		a_pstBusHandle->pastChannelSet[a_pstBusHandle->dwChannelCnt] = pstChannel;
		a_pstBusHandle->dwChannelCnt++;	
		tbus_log(TLOG_PRIORITY_DEBUG, "add channel(%s <--> %s) to handle, now channel num in the hanndle is:%u",
			szTmpIP, tbus_addr_ntoa(pstHead->astAddr[1]), a_pstBusHandle->dwChannelCnt);
	}

	return iRet ;
}


int tbus_attach_channel (
						 IN const TBUSADDR a_iAddr,
						 IN TBUSSHMCHANNELCNF *a_ptRoute,
						 INOUT TBUSCHANNEL *a_ptChannel,
						 IN LPTBUSSHMGCIMHEAD a_pstHead
						 )
{
	int iRet = TBUS_SUCCESS;
	char *sShm = NULL ;
	
	CHANNELHEAD *pstHead;
	int iChannelHeadLen;



	assert(NULL != a_ptRoute);
	assert(NULL != a_ptChannel);
	assert(NULL != a_pstHead);

	iRet = tbus_check_channel_shm_i(a_ptRoute, a_pstHead);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to check share memory of channel, iRet %x", iRet);
		return iRet;
	}

	
#ifdef WIN32
	{
		//TODO windows下shmget不支持IPC_PRIVATE,目前仅仅是临时实现
		char szKey[TIPC_MAX_NAME];
		int iShmID;

		snprintf(szKey, sizeof(szKey), "%u_%u", (unsigned int)a_ptRoute->astAddrs[0], 
			(unsigned int)a_ptRoute->astAddrs[1]);

		iShmID = (int) OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, szKey );
		sShm =	shmat(iShmID, NULL, 0) ;
		a_ptRoute->iShmID = iShmID;
	}
#else
	sShm =	shmat(a_ptRoute->iShmID, NULL, 0 ) ;
#endif
	if (NULL == sShm)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"shmat failed by id %d, for %s\n", a_ptRoute->iShmID, strerror(errno));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
	}
	

	iChannelHeadLen = sizeof(CHANNELHEAD);
	TBUS_CALC_ALIGN_VALUE(iChannelHeadLen, a_pstHead->iAlign);
#ifndef WIN32
	/*check size */	
	{
		struct shmid_ds stShmStat ;
		int iSize;
		iSize = iChannelHeadLen + a_ptRoute->dwRecvSize + a_ptRoute->dwSendSize;
		memset(&stShmStat, 0, sizeof(stShmStat));
		if ( 0 != shmctl(a_ptRoute->iShmID, IPC_STAT, &stShmStat))
		{
			tbus_log(TLOG_PRIORITY_ERROR,"shmctl stat failed by id %d, for %s\n", a_ptRoute->iShmID, strerror(errno));
			return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
		}

		if (iSize != (int)stShmStat.shm_segsz)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"shm size(%d) is diff from the channel size(%d) configuerd\n", 
				(int)stShmStat.shm_segsz, iSize);
			return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
		}
	}
	
#endif	

	a_ptChannel->pstHead = (CHANNELHEAD *)sShm;
	a_ptChannel->pszQueues[TBUS_CHANNEL_SIDE_INDEX_1] = sShm + iChannelHeadLen;
	a_ptChannel->pszQueues[TBUS_CHANNEL_SIDE_INDEX_2] = sShm + iChannelHeadLen + a_ptRoute->dwRecvSize;
	pstHead = a_ptChannel->pstHead;
	if (pstHead->astAddr[0] != a_ptRoute->astAddrs[0] || pstHead->astAddr[1] != a_ptRoute->astAddrs[1]) {
		tbus_log(TLOG_PRIORITY_ERROR,"Channel config ERROR!\n");
		tbus_log(TLOG_PRIORITY_ERROR,"address1 in Channel Shm : %s   ", tbus_addr_ntoa(pstHead->astAddr[0]));
		tbus_log(TLOG_PRIORITY_ERROR,"address2 in Channel Shm : %s", tbus_addr_ntoa(pstHead->astAddr[1]));
		tbus_log(TLOG_PRIORITY_ERROR,"address1 in GCIM : %s   ", tbus_addr_ntoa(a_ptRoute->astAddrs[0]));
		tbus_log(TLOG_PRIORITY_ERROR,"address1 in GCIM : %s", tbus_addr_ntoa(a_ptRoute->astAddrs[1]));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHANNEL_ADDRESS_CONFLICT);
	}	

	/* set channel side: a_iAddr is source address*/
	if ( a_iAddr == pstHead->astAddr[TBUS_CHANNEL_SIDE_INDEX_1] )
	{
		a_ptChannel->iRecvSide = TBUS_CHANNEL_SIDE_INDEX_1 ;
		a_ptChannel->iSendSide = TBUS_CHANNEL_SIDE_INDEX_2 ;
	}else
	{
		a_ptChannel->iRecvSide = TBUS_CHANNEL_SIDE_INDEX_2 ;
		a_ptChannel->iSendSide = TBUS_CHANNEL_SIDE_INDEX_1;
	}	

	tbus_log(TLOG_PRIORITY_DEBUG,"attach to channel(shmid:%d) successfully, localside:%d, peerside:%d",
		a_ptRoute->iShmID, a_ptChannel->iRecvSide, a_ptChannel->iSendSide);

	return TBUS_SUCCESS ;
}


