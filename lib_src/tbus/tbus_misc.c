/** @file $RCSfile: tbus_misc.c,v $
  general description of this module
  $Id: tbus_misc.c,v 1.20 2009/03/24 11:48:07 jacky Exp $
@author $Author: jacky $
@date $Date: 2009/03/24 11:48:07 $
@version $Revision: 1.20 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/



#include <time.h>
#include "tbus/tbus.h"
#include "tbus_misc.h"
#include "tbus_kernel.h"
#include "tbus_log.h"
#include "tbus/tbus_error.h"

extern LPTLOGCATEGORYINST	g_pstBusLogCat;
extern TBUSGLOBAL g_stBusGlobal;
extern unsigned char g_szMetalib_TbusHead[];


int tbus_auto_get_shm ( INOUT void **a_ppvShm, OUT int *a_piCreated, IN const int a_iShmKey,  INOUT  int *a_piSize, IN  int a_iFlag )
{
	int iShmID = -1,
		iRet = 0 ;
	struct shmid_ds stShmStat ;

	assert(NULL != a_ppvShm);

	*a_piCreated = 0;
	if (a_iFlag & IPC_CREAT)
	{
		a_iFlag |= IPC_EXCL;
		iShmID = shmget ( a_iShmKey, *a_piSize, a_iFlag) ;
		if (0 <= iShmID)
		{
			*a_piCreated = 1; /*create*/			
		}else
		{
			a_iFlag &= ~IPC_CREAT;
			a_iFlag &= ~IPC_EXCL;
			iShmID = shmget ( a_iShmKey, *a_piSize, a_iFlag) ;	
		}
	}else
	{
		iShmID = shmget ( a_iShmKey, *a_piSize, a_iFlag) ;
	}/*if (a_iFlag & IPC_CREAT)*/
	if ( 0 > iShmID )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"shmget failed by key %d, for %s\n", a_iShmKey, strerror(errno));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMGET_FAILED);
	}	

	*a_ppvShm =	shmat ( iShmID, NULL, 0 ) ;
	if (NULL == *a_ppvShm)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"shmat failed by id %d, for %s\n", iShmID, strerror(errno));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
	}
	if ( 0 == shmctl( iShmID, IPC_STAT, &stShmStat))
	{
		*a_piSize = (int)stShmStat.shm_segsz;
	}

	tbus_log(TLOG_PRIORITY_DEBUG,"CALL: tbus_auto_get_shm(void **, %i, %i, %i)", a_iShmKey, *a_piSize, a_iFlag ) ;

	return iRet ;
}


char *tbus_conv_hexstr ( IN const unsigned char *a_pszStr, IN const int a_iStrLen )
{
    int iLen, iCount;
	static char sBuffer[20480]; /* 20 KB */

	for (iLen = iCount = 0; (iCount < a_iStrLen)&&(iCount <(int)sizeof(sBuffer) - 1); iCount++)
	{
		if (iLen > (int)sizeof(sBuffer) - 5) break;

		if (iCount % 25 == 24) sprintf(sBuffer + iLen, "%.2x\n", a_pszStr[iCount]);
		else sprintf(sBuffer + iLen, "%.2x ", a_pszStr[iCount]);

		iLen += 3;
	}

	return sBuffer;
}



unsigned short tbus_head_checksum ( IN const TBUSHEAD *a_ptHead )
{
	unsigned short shSum = 0 ;
	int i = 0 ;
#define TBUS_CALC_CHECKSUM_SIZE offsetof(TBUSHEAD,bFlag)

	for (i = 0; i < (int)TBUS_CALC_CHECKSUM_SIZE/2; i++)
	{
		shSum ^= *(short *)((char *)a_ptHead + i * 2);
	}

	return shSum;
}

/* Show Hex val of a string, if a_iFlag = 1, show printable character as char */
void tbus_print_hexstr ( IN const unsigned char *a_pszStr, IN const int a_iLen, IN const int a_iFlag )
{
	register int iCount = 0 ;

	for (iCount = 0; iCount < a_iLen; iCount++)
	{
		if (iCount % 25 == 0) printf("\n");
		if (a_iFlag && a_pszStr[iCount] > 0x1f) printf("%2c ", a_pszStr[iCount]);
		else printf("%.2x ", a_pszStr[iCount]);
	}
	if ((iCount - 1) % 25) printf("\n");
}




int tbus_addr_aton(IN const char *a_pszAddr, OUT TBUSADDR *a_piAddr)
{
	int iRet;
	LPTBUSSHMGCIMHEAD pstHead;
	
	//assert(NULL != a_pszAddr);
	//assert(NULL != a_piAddr);
	if ((NULL == a_pszAddr)||(NULL == a_piAddr))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid param: a_pszAddr:%p a_piAddr: %p", a_pszAddr, a_piAddr);
		return TBUS_ERR_ARG;
	}

	if ( TBUS_MODULE_INITED != g_stBusGlobal.dwInited )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"global bus module not inited, so cannot assign handle" ) ;
		iRet = TBUS_ERR_INIT ;
		return iRet ;
	}
	assert(NULL != g_stBusGlobal.pstGCIM);

	pstHead = &g_stBusGlobal.pstGCIM->stHead;

	iRet = tbus_addr_aton_by_addrtemplet(&pstHead->stAddrTemplet, a_pszAddr, a_piAddr);

	return iRet;
}

char *tbus_addr_ntoa(IN TBUSADDR a_iAddr)
{
	LPTBUSSHMGCIMHEAD pstHead;

	if ( TBUS_MODULE_INITED != g_stBusGlobal.dwInited )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"global bus module not inited, so cannot assign handle" ) ;
		return "";
	}

	assert(NULL != g_stBusGlobal.pstGCIM);

	pstHead = &g_stBusGlobal.pstGCIM->stHead;
	return tbus_addr_nota_by_addrtemplet(&pstHead->stAddrTemplet, a_iAddr);

}

int tbus_encode_head(IN LPTBUSHEAD a_pstHead, IN char *a_pszNet, INOUT int *a_piLen, IN int a_iVersion)
{
	int iRet = TBUS_SUCCESS;
	TDRDATA stHost;
	TDRDATA stNet;
	LPTBUSHEAD pstHead;

	assert(NULL != a_pstHead);
	assert(NULL != a_pszNet);
	assert(NULL != a_piLen);

	if (0 == a_iVersion)
	{
		a_iVersion = TDR_METALIB_TBUSHEAD_VERSION;
	}
	if (NULL == g_stBusGlobal.pstHeadMeta)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"meta of tbus head is null");
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GET_HEAD_META);
	}

	/*设置校验 初值*/
	a_pstHead->nCheckSum = 0;
	

	/*编码*/
	a_pstHead->bVer = (unsigned char)a_iVersion;
	stNet.iBuff = *a_piLen;
	stNet.pszBuff = a_pszNet;
	stHost.iBuff = sizeof(TBUSHEAD);
	stHost.pszBuff = (char *)a_pstHead;
	iRet = tdr_hton(g_stBusGlobal.pstHeadMeta, &stNet, &stHost, a_iVersion);
	*a_piLen = stNet.iBuff;
	if (TDR_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to pach tbus head, iRet %x, for %s", iRet, tdr_error_string(iRet));
		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_PACK_HEAD);
	}	

	/*设置校验值*/
	a_pstHead->bHeadLen = stNet.iBuff;
	a_pstHead->nCheckSum = tbus_head_checksum(a_pstHead);
	pstHead = (LPTBUSHEAD)a_pszNet;
	pstHead->nCheckSum = htons(a_pstHead->nCheckSum);

	tbus_log_data(TLOG_PRIORITY_TRACE, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));
	
	return iRet;
}

int tbus_decode_head(OUT LPTBUSHEAD a_pstHead, IN char *a_pszNet, IN int a_iLen, IN int a_iVersion)
{
	int iRet = TBUS_SUCCESS;
	TDRDATA stHost;
	TDRDATA stNet;

	assert(NULL != a_pstHead);
	assert(NULL != a_pszNet);


	if (0 == a_iVersion)
	{
		a_iVersion = TDR_METALIB_TBUSHEAD_VERSION;
	}
	if (NULL == g_stBusGlobal.pstHeadMeta)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"meta of tbus head is null");
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GET_HEAD_META);
	}

	
	stNet.iBuff = a_iLen;
	stNet.pszBuff = a_pszNet;
	stHost.iBuff = sizeof(TBUSHEAD);
	stHost.pszBuff = (char *)a_pstHead;
	iRet = tdr_ntoh(g_stBusGlobal.pstHeadMeta, &stHost, &stNet, a_iVersion);
	if (TDR_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to unpach tbus head(len:%d), iRet %x, for %s", 
			stNet.iBuff, iRet, tdr_error_string(iRet));
		tbus_log_data(TLOG_PRIORITY_ERROR, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));
		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_UNPACK_HEAD);
	}

	if (0 == iRet)
	{
		/*检验hash值*/
		if (tbus_head_checksum(a_pstHead))
		{
			tbus_log(TLOG_PRIORITY_FATAL,"failed to check headsum,headlen:%d msglen:%d src:0x%x dst:0x%x", 
				a_pstHead->bHeadLen, a_pstHead->iBodyLen, a_pstHead->iSrc, a_pstHead->iDst);
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_FAILED_CHECK_HEADSUM);
		}		
	}/*if (0 == iRet)*/
	

	

	tbus_log_data(TLOG_PRIORITY_TRACE, g_stBusGlobal.pstHeadMeta, a_pstHead, sizeof(TBUSHEAD));
	return iRet;
}

#ifdef TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE
int tbus_get_pkghead(OUT LPTBUSHEAD a_pstHead, const char* a_pszQueue, unsigned int dwSize,
									INOUT unsigned int *a_pdwHead, unsigned int dwTail)
{
	int iDataLen = 0;
	int iHeadLen = 0;
	int iHeadVer;
	int iTailLen;
	char szNet[TBUS_HEAD_CODE_BUFFER_SIZE];
	int iRet = TBUS_SUCCESS;
	unsigned int dwHead;

	assert(NULL != a_pszQueue);
	assert(NULL!= a_pstHead);
	assert(0 < dwSize);
	assert(NULL != a_pdwHead);

	dwHead = *a_pdwHead;
	if (dwHead == dwTail) 
	{
		return TBUS_ERR_CHANNEL_EMPTY;
	}
	iDataLen = (int)(dwTail - dwHead);
	if (iDataLen < 0) 
	{
		iDataLen += dwSize;
	}
	if (iDataLen < TBUS_MIN_NET_LEN_TO_GET_HEADLEN) 
	{
		/*first two byte stored vesion and headlen of tbus head*/
		tbus_log(TLOG_PRIORITY_FATAL,"data length(%d) is less than min length(%d) to get tbus head len",
			iDataLen, TBUS_MIN_NET_LEN_TO_GET_HEADLEN);
		return TBUS_ERR_CHANNEL_CONFUSE;
	}	

	iHeadVer = (int)a_pszQueue[dwHead];
	iHeadLen = (int)a_pszQueue[(dwHead+TBUS_HEAD_LEN_NET_OFFSET) % dwSize];
	if (0 >= iHeadLen)
	{
		tbus_log(TLOG_PRIORITY_FATAL,"invalid tbus head len (%d), version of head(%d) datalen %d",
			iHeadLen, iHeadVer, iDataLen);	
		return TBUS_ERR_CHANNEL_CONFUSE;
	}

	if (iDataLen < iHeadLen)
	{
		tbus_log(TLOG_PRIORITY_FATAL,"data length(%d) is less than the length(%d) of tbus head len",
			iDataLen, iHeadLen);	
		return TBUS_ERR_CHANNEL_CONFUSE;
	}
	iTailLen = dwSize - dwHead;
	if (iTailLen < iHeadLen) 
	{
		memcpy(&szNet[0], a_pszQueue + dwHead, iTailLen);
		memcpy(&szNet[iTailLen], a_pszQueue, iHeadLen - iTailLen);
	} else  
	{
		memcpy(&szNet[0], a_pszQueue + dwHead, iHeadLen);
	}
	iRet = tbus_decode_head(a_pstHead, &szNet[0], iHeadLen, 0);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to unpack tbus head,iRet %x", iRet);
	}

	return iRet;
}
#else /*TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE*/
int tbus_get_pkghead(OUT LPTBUSHEAD a_pstHead, const char* a_pszQueue, unsigned int dwSize,
					 INOUT unsigned int *a_pdwHead, unsigned int dwTail)
{
	int iDataLen = 0;

	int iRet = TBUS_SUCCESS;


	assert(NULL != a_pszQueue);
	assert(NULL!= a_pstHead);
	assert(0 < dwSize);
	assert(NULL != a_pdwHead);


	if (*a_pdwHead == dwTail) 
	{
		return TBUS_ERR_CHANNEL_EMPTY;
	}

	/*检查头指针是否需要移动*/
	TBUS_CHECK_QUEUE_HEAD_VAR(a_pszQueue, dwSize, *a_pdwHead);	
	iDataLen = dwTail - *a_pdwHead;
	if (0 > iDataLen) 
	{
		iDataLen += dwSize;
	}
	if (0 >= iDataLen) 
	{
		return TBUS_ERR_CHANNEL_EMPTY;
	}

	iRet = tbus_decode_head(a_pstHead, (char *)(a_pszQueue +*a_pdwHead), iDataLen, 0);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to unpack tbus head,iRet %x", iRet);
	}

	return iRet;
}
#endif /*TBUS_SPLIT_PKG_IN_RECYCLE_QUEUE*/

int tbus_init_headmeta()
{
	LPTDRMETALIB pstLib;

	if (NULL != g_stBusGlobal.pstHeadMeta)
	{
		return TBUS_SUCCESS;
	}

	/*get meta of tbus head*/
	pstLib = (LPTDRMETALIB)g_szMetalib_TbusHead;
	g_stBusGlobal.pstHeadMeta = tdr_get_meta_by_name(pstLib, tdr_get_metalib_name(pstLib));
	if (NULL == g_stBusGlobal.pstHeadMeta)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to get meta of tbus head by name %s", tdr_get_metalib_name(pstLib));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GET_HEAD_META);

	}

	return TBUS_SUCCESS;
}

void tbus_trim_str( char *a_strInput )
{
	char *pb;
	char *pe;
	int iTempLength;


	assert(NULL != a_strInput);
	iTempLength = strlen(a_strInput);
	if( iTempLength == 0 )
	{
		return;
	}

	pb = a_strInput;

	while (((*pb == ' ') || (*pb == '\t') || (*pb == '\n') || (*pb == '\r')) && (*pb != 0))
	{
		pb ++;
	}

	pe = &a_strInput[iTempLength-1];
	while ((pe >= pb) && ((*pe == ' ') || (*pe == '\t') || (*pe == '\n') || (*pe == '\r')))
	{
		pe --;
	}

	*(pe+1) = '\0';

	if (pb != a_strInput)
	{
		strcpy( a_strInput, pb );
	}

}

/**
@brief 获取通道两端队列上剩余消息的字节总数
@param[in] a_hHandle tbus句柄
@param[in] a_iLocalAddr 此tbus通道相对于本端的地址
@param[in] a_iPeerAddr 此tbus通道相对于对端的地址
@param[in] a_piInFlux 获取输入通道(a_iLocalAddr <-- a_iPeerAddr)剩余消息字节数的指针
@param[in] a_piOutFlux 获取输出通道(a_iLocalAddr --> a_iPeerAddr)剩余消息字节数的指针
@retval 0 -- success
@retval !0 -- failed
@note
*/
int tbus_get_channel_flux(IN int a_hHandle, IN TBUSADDR a_iLocalAddr, IN TBUSADDR a_iPeerAddr,
											 OUT int *a_piInFlux, OUT int *a_piOutFlux)
{
	int iRet = 0;
	TBUSHANDLE *pstHandle = NULL ;
	int iChannelNum;
	int i;

	if ((NULL == a_piInFlux)||(NULL == a_piOutFlux))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid param: a_piInFlux: %p a_piInFlux: %p",
			a_piInFlux, a_piOutFlux);
		return TBUS_ERR_ARG;
	}
	pstHandle = tbus_get_handle(a_hHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_hHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}

	iChannelNum = pstHandle->dwChannelCnt;
	for (i = 0; i < iChannelNum; i++)
	{
		TBUSCHANNEL *pstChannel = pstHandle->pastChannelSet[i];
		LPCHANNELHEAD pstHead = pstChannel->pstHead;
		if ((a_iLocalAddr == pstHead->astAddr[pstChannel->iRecvSide]) && 
			(a_iPeerAddr == pstHead->astAddr[pstChannel->iSendSide]))
		{
			LPCHANNELVAR pstGet = &pstHead->astQueueVar[pstChannel->iRecvSide];
			LPCHANNELVAR pstPut = &pstHead->astQueueVar[pstChannel->iSendSide];
			*a_piInFlux = pstGet->dwTail - pstGet->dwHead;
			if (0 > *a_piInFlux)
			{
				*a_piInFlux += pstGet->dwSize;
			}
			*a_piOutFlux = pstPut->dwTail - pstPut->dwHead;
			if (0 > *a_piOutFlux)
			{
				*a_piOutFlux += pstPut->dwSize;
			}
			return 0;
		}
	}/*for (i = 0; i < iChannelNum; i++)*/
	if (i >= iChannelNum)
	{
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED);
	}

	return iRet;
}

/** 读取数据通道消息输入队列指定位置所在消息的序列号
@param[in] a_pstChannel tbus通道数据结构的指针
@param[in] a_dwPos 消息在队列中的起始位置
*/
int tbus_get_pkgseq_by_pos(IN TBUSCHANNEL* a_pstChannel, IN unsigned int a_dwPos)
{
	LPCHANNELVAR a_pstVar;
	const char *a_pszQueue;
	LPTBUSHEAD pstHead;
	int iDataLen;

	assert(NULL != a_pstChannel);
	
	a_pstVar = TBUS_CHANNEL_VAR_GET(a_pstChannel);
	a_pszQueue = TBUS_CHANNEL_QUEUE_GET(a_pstChannel);;


	if (a_dwPos == a_pstVar->dwTail)
	{/*队列为空，返回下一个待接收消息的序列号*/
		return a_pstVar->iSeq;
	}

	TBUS_CHECK_QUEUE_HEAD_VAR(a_pszQueue, a_pstVar->dwSize, a_dwPos)
	if (a_dwPos == a_pstVar->dwTail)
	{/*队列为空，返回下一个待接收消息的序列号*/
		return a_pstVar->iSeq;
	}	


	/*指定位置存在消息，则取出此消息头部，并取出序列号*/
	iDataLen = a_pstVar->dwTail - a_dwPos;
	if (0 > iDataLen)
	{
		iDataLen += a_pstVar->dwSize;
	}
	if (0 >= iDataLen)
	{
		return a_pstVar->iSeq;
	}

	pstHead = (LPTBUSHEAD)(a_pszQueue + a_dwPos);

	
	return  ntohl(pstHead->iSeq);

}


