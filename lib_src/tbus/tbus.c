/** @file $RCSfile: tbus.c,v $
  general description of this module
  $Id: tbus.c,v 1.30 2009/03/30 07:37:58 jacky Exp $
@author $Author: jacky $
@date $Date: 2009/03/30 07:37:58 $
@version $Revision: 1.30 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/



#include "tbus/tbus.h"
#include "tbus/tbus_error.h"
#include "tbus_dyemsg.h"
#include "tbus_log.h"
#include "tbus_kernel.h"
#include "tbus_misc.h"
#include "tbus_channel.h"



TBUSGLOBAL g_stBusGlobal = {TBUS_MODULE_NOT_INITED,PTHREAD_MUTEX_INITIALIZER, TBUS_DEFAUL_BUSSINESS_ID,0,0} ;



/**
  various internal function defined
*/
int handle_check_in ( IN const int a_iHandle ) ;
int tbus_select_channel_i( IN TBUSCHANNEL *a_ptAddr, INOUT int *a_piSrc, INOUT int *a_piDst );


int forward_channel_select_in ( IN  TBUSCHANNEL *a_ptAddr, INOUT int *a_piSrc, INOUT int *a_piDst, IN const int a_iFilterDst ) ;
int backward_channel_select_in ( IN  TBUSCHANNEL *ptAddr, INOUT int *a_piSrc, INOUT int *a_piDst ) ; 




/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int tbus_init ( IN const unsigned int a_iMapKey, IN const unsigned int a_iSize )
{
	int iRet = TBUS_SUCCESS;

	
	if (0 == a_iMapKey)
	{
		iRet = tbus_init_ex(TBUS_DEFAULT_GCIM_KEY, TBUS_INIT_FLAG_NONE);
	}else
	{
		char szKey[32] = {0};
		snprintf(szKey, sizeof(szKey), "%d", a_iMapKey);
		iRet = tbus_init_ex(szKey, TBUS_INIT_FLAG_NONE);	
	}
	
	return iRet ;
}


int tbus_new ( INOUT int *a_piHandle )
{
	int iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_HANDLE_FULL) ;
	int i = 0 ;
	LPTBUSHANDLE pastTbusHandle;

	tbus_set_logpriority(TLOG_PRIORITY_TRACE);


	if ( NULL == a_piHandle )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"parameter error, a_piHandle is null" ) ;
		iRet = TBUS_ERR_ARG ;
		return iRet ;
	}
	if ( TBUS_MODULE_INITED != g_stBusGlobal.dwInited )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"global bus module not inited, so cannot assign handle" ) ;
		iRet = TBUS_ERR_INIT ;
		return iRet ;
	}


	/*find one free position for handle */
	for ( i=1; i<HANDLE_COUNT; i++ )
	{
		if ( NULL != g_stBusGlobal.pastTbusHandle[i] )
		{
			continue;
		}

		g_stBusGlobal.pastTbusHandle[i] = (TBUSHANDLE *) malloc ( sizeof(TBUSHANDLE) ) ;
		if ( NULL == g_stBusGlobal.pastTbusHandle[i] )
		{
			tbus_log(TLOG_PRIORITY_ERROR,"malloc() size failed %i", sizeof(TBUSHANDLE) ) ;
			iRet = TBUS_ERROR_NO_MEMORY ;
			break;
		}

		pastTbusHandle = g_stBusGlobal.pastTbusHandle[i];
		memset(pastTbusHandle , 0x0, sizeof(TBUSHANDLE) ) ;
		pastTbusHandle->iRecvPos = TBUS_RECV_POS_INVALID ;
		pastTbusHandle->dwFlag |= TBUS_HANDLE_FLAG_ENABLE; 
		pastTbusHandle->dwGCIMVersion = g_stBusGlobal.pstGCIM->stHead.dwVersion;


		*a_piHandle = i ;
		iRet = TBUS_SUCCESS ;
		tbus_log(TLOG_PRIORITY_DEBUG,"new handle(%i) successfully", i) ;
		break ;		
	}/*for ( i=1; i<HANDLE_COUNT; i++ )*/

	
	tbus_set_logpriority(TLOG_PRIORITY_ERROR);

	return iRet ;
}



int tbus_bind( IN const int a_iHandle, IN const TBUSADDR a_iSrcAddr)
{
	char szTmpIP[128];
	STRNCPY(szTmpIP, tbus_addr_ntoa(a_iSrcAddr), sizeof(szTmpIP));
	return tbus_bind_by_str ( a_iHandle,  &szTmpIP[0]) ;
}

int tbus_add_channels_associated_addr( IN LPTBUSHANDLE a_pstBusHandle, IN TBUSADDR a_iBindAddr, TBUSSHMGCIM *a_pstGCIM)
{
	LPTBUSSHMGCIMHEAD pstHead ;
	unsigned int i;
	unsigned int iRouteCnt;
	int iRet = 0;
	TBUSADDR iPeerAddr = 0;

	assert(NULL != a_pstGCIM);

	pstHead = &a_pstGCIM->stHead;
	iRouteCnt = pstHead->dwUsedCnt ;
	for ( i=0; i<iRouteCnt; i++ )
	{
		TBUSSHMCHANNELCNF *pstRoute = &g_stBusGlobal.pstGCIM->astChannels[i];
		tbus_log(TLOG_PRIORITY_TRACE,"channel(%d): Flag:0x%x  address(0x%08x <-->0x%08x\n", 
			i, pstRoute->dwFlag,  pstRoute->astAddrs[0], pstRoute->astAddrs[1]) ;
		if (!TBUS_GCIM_CHANNEL_IS_ENABLE(pstRoute) )
		{
			continue ;
		}
		if (a_iBindAddr == pstRoute->astAddrs[0])
		{
			iPeerAddr = pstRoute->astAddrs[1];
		}else if (a_iBindAddr == pstRoute->astAddrs[1])
		{
			iPeerAddr = pstRoute->astAddrs[0];
		}else
		{
			continue;
		}		

		/* enable source route */
		iRet = tbus_enable_addr(a_iBindAddr, pstRoute,a_pstBusHandle, pstHead) ;		
		if ( TBUS_SUCCESS != iRet )
		{
			char szTmpIP[128];
			STRNCPY(szTmpIP, tbus_addr_ntoa(a_iBindAddr), sizeof(szTmpIP));
			tbus_log(TLOG_PRIORITY_ERROR,"tbus_bind(), failed bind %ith channel(%s<-->%s), Ret: %d",
				i+1, szTmpIP, tbus_addr_ntoa(iPeerAddr), iRet ) ;
			break ;
		}		
	}/*for ( i=0; i<iRouteCnt; i++ )*/

	return iRet;
}

int tbus_bind_by_str ( IN const int a_iHandle, IN const char *a_szSrcAddr )
{
	int iRet = TBUS_SUCCESS ;
	TBUSADDR iAddr = 0 ;
	TBUSHANDLE *pstTbusHandle;	
	LPTBUSSHMGCIMHEAD pstHead ;

	tbus_set_logpriority(TLOG_PRIORITY_TRACE);
	if ( NULL == a_szSrcAddr )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"parameter error, a_szSrcAddr is null" ) ;
		iRet = TBUS_ERR_ARG ;
		return iRet ;
	}

	tbus_log(TLOG_PRIORITY_TRACE,"begin to  bind addr(%s) to handle(%d)\n", a_szSrcAddr, a_iHandle) ;
	pstTbusHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstTbusHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}
	iRet = tbus_addr_aton(a_szSrcAddr, &iAddr);
	if (0 != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"tbus_addr_aton failed to convert %s to tbusaddr, iret",
			a_szSrcAddr, iRet);
		return iRet;
	}

	/*记录绑定的地址*/
	if (pstTbusHandle->iBindAddrNum < TBUS_MAX_BIND_ADDR_NUM_PREHANDLE)
	{
		pstTbusHandle->aiBindAddr[pstTbusHandle->iBindAddrNum] = iAddr;
		pstTbusHandle->iBindAddrNum++;
	}
	

	/* 以读锁，锁住GCIM共享内存，以从GCIM中读出数据*/
	pstHead = &g_stBusGlobal.pstGCIM->stHead;
	tbus_wrlock(&pstHead->stRWLock);
	iRet = tbus_add_channels_associated_addr(pstTbusHandle, iAddr, g_stBusGlobal.pstGCIM);	
	tbus_unlock(&pstHead->stRWLock);	

	if ((TBUS_SUCCESS == iRet) && (0 >= pstTbusHandle->dwChannelCnt))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"cannot bind any channel by addrress: %s",	a_szSrcAddr) ;
		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_BIND_FAILED);
	}	

	tbus_log(TLOG_PRIORITY_TRACE,"end of tbus_bind addr %s to handle('%d'), iRet=%i", a_szSrcAddr, a_iHandle, iRet ) ;
	tbus_set_logpriority(TLOG_PRIORITY_ERROR);

	return iRet ;
}

int IsChannelInGCIM(CHANNELHEAD * a_pstHead, TBUSSHMGCIM *a_pstGCIM)
{
	unsigned int i;

	assert(NULL != a_pstHead);
	assert(NULL != a_pstGCIM);

	for (i = 0; i < a_pstGCIM->stHead.dwUsedCnt; i++)
	{
		LPTBUSSHMCHANNELCNF  pstChannelCnf = &a_pstGCIM->astChannels[i];
		if (!TBUS_GCIM_CHANNEL_IS_ENABLE(pstChannelCnf))
		{
			continue;
		}
		if ((a_pstHead->astAddr[0] == pstChannelCnf->astAddrs[0]) &&
			(a_pstHead->astAddr[1] == pstChannelCnf->astAddrs[1]))
		{
			return 1;
		}
	}/*for (i = 0; i < a_pstGCIM->stHead.dwUsedCnt; i++)*/

	return 0;
}

/**
@brief 根据全局GCIM中的配置刷新tbus句柄管理的相关通道
如果绑定地址相关通道有添加则自动添加到tbus中；如果tbus句柄管理的相关通道已经不再gcim配置
中则定期回收
@param[in] a_iHandle tbus处理句柄，通过调用tbus_new() 获取

@retval 0 success
@retval <0 failed, 可能的错误代码如下:
-	TBUS_ERR_ARG	传递给接口的参数不对
-	TBUS_ERROR_NOT_INITIALIZED	bus系统还没有初始化
-	TBUS_ERROR_CHANNEL_NUM_LIMITED
-	TBUS_ERROR_NO_MEMORY
-	TBUS_ERROR_SHMGET_FAILED
-	TBUS_ERROR_SHMAT_FAILED
-	TBUS_ERROR_CHANNEL_ADDRESS_CONFLICT
-	TBUS_ERROR_BIND_FAILED
@note tbus API使用者可以定时调用本接口以刷新该句柄下相关通道配置

@pre a_iHandle 为有效句柄
@see tbus_new
@see tbus_bind
*/
TSF4G_API int tbus_refresh_handle( IN const int a_iHandle)
{
	int iRet = TBUS_SUCCESS ;
	unsigned int i = 0 ;
	TBUSCHANNEL *pstChannel;
	time_t tNow;
	TBUSHANDLE *pstTbusHandle;
	LPTBUSSHMGCIMHEAD pstHead ;


	pstTbusHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstTbusHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}

	/*首先检查回收不再使用的通道*/
	time(&tNow);
	for (i = 0; i < pstTbusHandle->dwChannelCnt; i++)
	{
		pstChannel = pstTbusHandle->pastChannelSet[i];
		if (TBUS_CHANNEL_IS_ENABLE(pstChannel))
		{
			continue;
		}

		/*如果不在配置中的通道预留一段时间后超时，则关闭或尝试清除它*/		

		if ((tNow - pstChannel->tBeginDisable) >= TBUS_DISABLE_CHANNEL_CLEAR_DISABLE_TIMEOUTGAP)
		{
			tbus_log(TLOG_PRIORITY_FATAL, "the time which the channel(0x%08x <-->0x%08x) is disabled reach"
				"%d second, so clear the channel", pstChannel->pstHead->astAddr[0],
				pstChannel->pstHead->astAddr[1], TBUS_DISABLE_CHANNEL_CLEAR_DISABLE_TIMEOUTGAP);
			tbus_close_channel(&pstTbusHandle->pastChannelSet[i]);
			if (i < pstTbusHandle->dwChannelCnt -1 )
			{
				pstTbusHandle->pastChannelSet[i] = pstTbusHandle->pastChannelSet[pstTbusHandle->dwChannelCnt -1];
				pstTbusHandle->pastChannelSet[pstTbusHandle->dwChannelCnt -1] = NULL;
			}
			pstTbusHandle->dwChannelCnt--;
			i--;
		}/*if ((tNow - pstChannel->tBeginDisable) >= TBUS_DISABLE_CHANNEL_CLEAR_DISABLE_TIMEOUTGAP)*/		
	}/*for (i = 0; i < pstTbusHandle->dwChannelCnt; i++)*/


	/*通过版本号检查gcim的配置是否有变化*/
	pstHead = &g_stBusGlobal.pstGCIM->stHead;
	if (pstHead->dwVersion == pstTbusHandle->dwGCIMVersion)
	{
		return TBUS_SUCCESS;
	}
	tbus_log(TLOG_PRIORITY_TRACE, "the gicmversion(%u) recorded at handle(%d) diff from "
		"the shm gcim version(%u), so try to refer configure", pstTbusHandle->dwGCIMVersion,
		a_iHandle, pstHead->dwVersion);

	/*尝试锁gcim配置，如果锁失败则下次再尝试*/
	if (0 != pthread_rwlock_trywrlock(&pstHead->stRWLock))
	{
		return TBUS_SUCCESS;
	}

	/*首先检查通用配置中去掉的通道*/
	for (i = 0; i < pstTbusHandle->dwChannelCnt; i++)
	{
		pstChannel = pstTbusHandle->pastChannelSet[i];
		if (IsChannelInGCIM(pstChannel->pstHead, g_stBusGlobal.pstGCIM))
		{
			continue;
		}else
		{
			/*如果此通道不在gcim通道中，则设置此通道不可用*/
			TBUS_CHANNEL_CLR_ENABLE(pstChannel);
			TBUS_CHANNEL_SET_NOT_IN_GCIM(pstChannel);
			pstChannel->tBeginDisable = tNow;
			tbus_log(TLOG_PRIORITY_DEBUG, "the time which the channel(0x%08x <-->0x%08x) is not in gcim "
				"so set the NOT_IN_GCIM flag and disable it", pstChannel->pstHead->astAddr[0],
				pstChannel->pstHead->astAddr[1]);
		}		
	}/*for (i = 0; i < pstTbusHandle->dwChannelCnt; i++)*/

	/*通过绑定地址查询是否有新的通道需要添加到管理句柄中*/
	for (i = 0; i < (unsigned int)pstTbusHandle->iBindAddrNum; i++)
	{
		iRet = tbus_add_channels_associated_addr(pstTbusHandle, pstTbusHandle->aiBindAddr[i], g_stBusGlobal.pstGCIM);
		if (0 != iRet)
		{
			break;
		}
	}/*for (i = 0; i < (int)pstTbusHandle->iBindAddrNum; i++)*/

	if (0 == iRet)
	{
		pstTbusHandle->dwGCIMVersion = pstHead->dwVersion;
	}

	/*解锁*/
	pthread_rwlock_unlock(&pstHead->stRWLock);


	return iRet ;
}

int tbus_connect_by_str ( IN const int a_iHandle, IN const char *a_szDstAddr )

{
	int iRet = TBUS_SUCCESS,
		i = 0,
		iAddrCnt = 0 ;
	TBUSADDR iAddr = 0 ;
	TBUSCHANNEL *pstChannel = NULL ;
	TBUSHANDLE *pstTbusHandle; 

	tbus_set_logpriority(TLOG_PRIORITY_TRACE);
	if ( NULL == a_szDstAddr )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"parameter error,a_szDstAddr is null" ) ;
		iRet = TBUS_ERR_ARG ;
		return iRet ;
	}
	tbus_log(TLOG_PRIORITY_DEBUG,"Use handle(%d) connect to ('%s')", a_iHandle, a_szDstAddr ) ;

	pstTbusHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstTbusHandle)
	{
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}	
	iAddrCnt = pstTbusHandle->dwChannelCnt ;
	if ( 0 >= iAddrCnt )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"no available channel" ) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
	}
	iRet = tbus_addr_aton(a_szDstAddr, &iAddr);
	if (0 != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"tbus_addr_aton failed to convert %s to tbusaddr, iret",
			a_szDstAddr, iRet);
		return iRet;
	}


	for ( i=0; i<iAddrCnt; i++ )
	{
		CHANNELHEAD *pstHead; 
		pstChannel = pstTbusHandle->pastChannelSet[i] ;
		

		if ( !(pstChannel->dwFlag & TBUS_CHANNEL_FLAG_ENABLE) )
		{
			tbus_log(TLOG_PRIORITY_TRACE,"channel(%d) is  disable ", i) ;
			continue ;
		}
		if ( !(pstChannel->dwFlag & TBUS_CHANNEL_FLAG_SRC_ENABLE) )
		{
			tbus_log(TLOG_PRIORITY_TRACE,"source address disable %i", i ) ;
			continue ;
		}

		pstHead = pstChannel->pstHead;
		if (iAddr == pstHead->astAddr[pstChannel->iSendSide])
		{
			tbus_log(TLOG_PRIORITY_DEBUG,"connect channel(%s<-->%s) succefully, ",
				tbus_addr_ntoa(pstHead->astAddr[pstChannel->iRecvSide]), a_szDstAddr) ;

			/* remote address found, enable remote address bit */
			pstChannel->dwFlag |= TBUS_CHANNEL_FLAG_DST_ENABLE ;
			break;
		}
	}/*for ( i=0; i<iAddrCnt; i++ )*/
	
	if (i >= iAddrCnt)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to connect %s, no channel peer address match with it", a_szDstAddr);
		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_PEER_ADDRESS_MATCHED);
	}

	tbus_set_logpriority(TLOG_PRIORITY_ERROR);

	return iRet ;
}


int tbus_send (	IN const int a_iHandle,	INOUT TBUSADDR *a_piSrc,	INOUT TBUSADDR *a_piDst,
	IN const void *a_pvData,	IN const size_t a_iDataLen,	IN const int a_iFlag)
{
	int iRet = TBUS_SUCCESS;	
	struct iovec astVectors[1];

	if ((NULL == a_pvData)||(0 >= a_iDataLen))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid param: a_pvData:%p a_iDataLen:%d", a_pvData, a_iDataLen);
		return TBUS_ERR_ARG;
	}
	astVectors[0].iov_base = (void *)a_pvData;
	astVectors[0].iov_len = a_iDataLen;

	iRet = tbus_sendv(a_iHandle, a_piSrc, a_piDst, &astVectors[0], 1, a_iFlag);
	
	return iRet ;
}


int tbus_sendv (
	IN const int a_iHandle,
	INOUT int *a_piSrc,
	INOUT int *a_piDst,
	IN const struct iovec *a_ptVector,
	IN const int a_iVecCnt,
	IN const int a_iFlag
)
{
	int iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED),
		iAddrCnt = 0,
		i = 0;
	TBUSADDR iSrcAddr = 0,
		iDstAddr = 0,
		iRetSrc = *a_piSrc,
		iRetDst = *a_piDst ;
	TBUSHEAD stHead;
	LPTBUSHANDLE pstHandle;

	if ( (NULL == a_piSrc) || (NULL == a_piDst) || (NULL == a_ptVector) || (0 >= a_iVecCnt) )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, parameter error, null paramter or VecCnt(%d) less than 0 ", a_iVecCnt ) ;
		return TBUS_ERR_ARG ;
	}	
	
	tbus_log(TLOG_PRIORITY_TRACE,"hande:%i, src:%i, dst:%i,  Flag:%i, veccnt:%d",
		a_iHandle, *a_piSrc, *a_piDst, a_iFlag, a_iVecCnt) ;

	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}
	iAddrCnt = pstHandle->dwChannelCnt ;
	if ( 0 >= iAddrCnt )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"no channel(count:%d) to send data", iAddrCnt ) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
	}


	/*构造tbus消息头部*/
	memset(&stHead, 0, sizeof(TBUSHEAD));
	stHead.bCmd = TBUS_HEAD_CMD_TRANSFER_DATA;
	tbus_dye_pkg(&stHead, NULL, a_iFlag);

	for ( i=0; i<iAddrCnt; i++ )
	{
		LPTBUSCHANNEL ptAddr = pstHandle->pastChannelSet[i] ;

		iSrcAddr = *a_piSrc ;
		iDstAddr = *a_piDst ;
		if ( TBUS_SUCCESS != tbus_select_channel_i(ptAddr, &iSrcAddr, &iDstAddr ))
		{
			continue ;
		}		

		/* fill address */
		stHead.iDst = iDstAddr;
		stHead.iSrc = iSrcAddr;
		TBUS_SET_LAST_ROUTE(stHead, iSrcAddr);
		iRet = tbus_push_channel_pkgv(ptAddr, &stHead, a_ptVector, a_iVecCnt) ;
		if (iRet != TBUS_SUCCESS)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"tbus_send, failed to push data to channel(%d), iRet:%d", i, iRet ) ;
			if ( (TBUS_ADDR_ALL == *a_piSrc) || (TBUS_ADDR_ALL == *a_piDst) )
			{
				continue ; /* batch sent mode, skipped individual error */
			}else
			{
				/* failed return, break here, not batch sent mode */
				break ;
			}
		}/*if (iRet != TBUS_SUCCESS)*/

		tbus_log(TLOG_PRIORITY_DEBUG,
			"Send data(len:%d) to channel %d,   destination  address %s", stHead.iBodyLen, i, tbus_addr_ntoa(iDstAddr) ) ;
		if(stHead.bFlag & TBUS_HEAD_FLAG_TACE)
		{
			tbus_log_dyedpkg(&stHead, "Send");
		}
		iRet = TBUS_SUCCESS ;

		/* record first successful channel address */
		iRetSrc = iSrcAddr ;
		iRetDst = iDstAddr ;
		if ( (TBUS_ADDR_ALL != *a_piSrc) && (TBUS_ADDR_ALL != *a_piDst) )
		{
			/* break here, not batch sent mode */
			break ;
		}

	} /* end for ( i=0; i<iAddrCnt; i++ ) */

	/* set return address */
	if (0 != iRetSrc)
	{
		*a_piSrc = iRetSrc ;
	}
	if (0 != iRetDst)
	{
		*a_piDst = iRetDst ;
	}
		

	return iRet ;
}


int tbus_recv (	IN const int a_iHandle,	INOUT int *a_piSrc,	INOUT int *a_piDst,
	OUT void *a_pvData,	INOUT size_t *a_piDataLen,	IN const int a_iFlag)
{
	int iRet = TBUS_ERR_CHANNEL_EMPTY;
	TBUSADDR	iSrcAddr = 0;
	TBUSADDR	iDstAddr = 0;

	int i = 0 ;
	TBUSHANDLE *pstHandle = NULL ;
	TBUSCHANNEL *ptChannel = NULL ;
	int iAddrCnt;
	int iCurPos;

	if ( (NULL == a_piSrc) || (NULL == a_piDst) || (NULL == a_pvData) || (NULL == a_piDataLen))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"parameter error, null parameter " ) ;
		return TBUS_ERR_ARG ;
	}	
	if (0 >= *a_piDataLen)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"parameter error, size(%d) of buffer to recv data is less than 0", *a_piDataLen) ;
		return TBUS_ERR_ARG ;
	}
	tbus_log(TLOG_PRIORITY_TRACE,"CALL: tbus_recv(Handle:%i, src:%i, Dst:%i, BufferSize: %i,Flag: %i)",
		a_iHandle, *a_piSrc, *a_piDst, *a_piDataLen, a_iFlag ) ;


	/*Get handle*/
	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}
	iAddrCnt = pstHandle->dwChannelCnt ;
	if ( 0 >= iAddrCnt )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"no channel(count:%d) to recv data", iAddrCnt ) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
	}

	/*select position of channel which search channel to recv data from it */
	if ( TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piSrc) ||	TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piDst))
	{
		/* not specified source or destination address, get current received position */
		if ( TBUS_RECV_POS_INVALID == pstHandle->iRecvPos )
		{
			pstHandle->iRecvPos = 0 ; /* first inited */
			pstHandle->dwRecvPkgCnt = 0 ;
		}else if ( TBUS_MAX_RECV_CNT <= pstHandle->dwRecvPkgCnt )
		{
			pstHandle->iRecvPos = (pstHandle->iRecvPos + 1) % iAddrCnt ;
			pstHandle->dwRecvPkgCnt = 0 ;
		}
		iCurPos = pstHandle->iRecvPos ;
	}else
	{
		/* specified source and destination address, select channel from 1st one */
		iCurPos = 0 ;
	}/*if ( TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piSrc) ||	TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piDst))*/

	for( i= iCurPos; i< iAddrCnt; i++ )
	{
		iSrcAddr = *a_piSrc ;
		iDstAddr = *a_piDst ;

		ptChannel = pstHandle->pastChannelSet[i];
		if ( TBUS_SUCCESS != tbus_select_channel_i(ptChannel, &iDstAddr, &iSrcAddr) )
		{
			continue;
		}

		iRet = tbus_get_channel_pkgv(ptChannel, &pstHandle->stRecvHead, a_pvData, (int *)a_piDataLen) ;
		if ( TBUS_SUCCESS != iRet )
		{
			if ( TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piSrc) ||	TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piDst))
			{
				/* batch received, channel empty or error occured, try next channel */
				pstHandle->iRecvPos = (i + 1) % iAddrCnt ;
				pstHandle->dwRecvPkgCnt = 0 ;
				continue;
			}else
			{
				break;
			}
		}/*if ( TBUS_SUCCESS != iRet )*/

		/*recv data successfully*/
		tbus_log(TLOG_PRIORITY_DEBUG,"recv one msg(len:%d) from %s\n", *a_piDataLen, tbus_addr_ntoa(iSrcAddr));		
		if (TBUS_FLAG_START_DYE_MSG & a_iFlag)
		{
			/*dye msg*/
			tbus_dye_pkg(&pstHandle->stRecvHead, NULL, TBUS_FLAG_START_DYE_MSG);
		}			
		if(pstHandle->stRecvHead.bFlag & TBUS_HEAD_FLAG_TACE)
		{
			tbus_log_dyedpkg(&pstHandle->stRecvHead, "Recv");
		}

		/* not specified source and destination address, update current received position */
		if ( TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piSrc) ||	TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piDst))
		{
			pstHandle->dwRecvPkgCnt ++ ;
		}
		if ( TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piSrc))
		{
			/* set return address */
			*a_piSrc = iSrcAddr ;
		}
		if (TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piDst))
		{
			*a_piDst = iDstAddr ;
		}

		break ; /* ok return */		
	}/*for( i= iCurPos; i< iAddrCnt; i++ )*/

	tbus_log(TLOG_PRIORITY_TRACE,"(Handle:%i, Src:%i, Dst:%i, DataLen:%d, Flag:%i, iRet:%i)",
		a_iHandle, *a_piSrc, *a_piDst, *a_piDataLen, a_iFlag, iRet ) ;

	return iRet ;
}

TSF4G_API int tbus_peek_msg(IN const int a_iHandle,	INOUT TBUSADDR *a_piSrc,	INOUT TBUSADDR *a_piDst,
								INOUT const char **a_ppvData,	OUT size_t *a_piDataLen,	IN const int a_iFlag)
{
	int iRet = TBUS_ERR_CHANNEL_EMPTY;
	TBUSADDR	iSrcAddr = 0;
	TBUSADDR	iDstAddr = 0;

	int i = 0 ;
	TBUSHANDLE *pstHandle = NULL ;
	TBUSCHANNEL *ptChannel = NULL ;
	int iAddrCnt;
	int iCurPos;

	if ( (NULL == a_piSrc) || (NULL == a_piDst) || (NULL == a_ppvData) || (NULL == a_piDataLen))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"parameter error, null parameter " ) ;
		return TBUS_ERR_ARG ;
	}	
	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}
	iAddrCnt = pstHandle->dwChannelCnt ;
	if ( 0 >= iAddrCnt )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"no channel(count:%d) to recv data", iAddrCnt ) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
	}

	/*select position of channel which search channel to recv data from it */
	if ( TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piSrc) ||	TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piDst))
	{
		/* not specified source or destination address, get current received position */
		if ( TBUS_RECV_POS_INVALID == pstHandle->iRecvPos )
		{
			pstHandle->iRecvPos = 0 ; /* first inited */
			pstHandle->dwRecvPkgCnt = 0 ;
		}else if ( TBUS_MAX_RECV_CNT <= pstHandle->dwRecvPkgCnt )
		{
			pstHandle->iRecvPos = (pstHandle->iRecvPos + 1) % iAddrCnt ;
			pstHandle->dwRecvPkgCnt = 0 ;
		}
		iCurPos = pstHandle->iRecvPos ;
	}else
	{
		/* specified source and destination address, select channel from 1st one */
		iCurPos = 0 ;
	}/*if ( TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piSrc) ||	TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piDst))*/

	for( i= iCurPos; i< iAddrCnt; i++ )
	{
		iSrcAddr = *a_piSrc ;
		iDstAddr = *a_piDst ;

		ptChannel = pstHandle->pastChannelSet[i];
		if ( TBUS_SUCCESS != tbus_select_channel_i(ptChannel, &iDstAddr, &iSrcAddr) )
		{
			continue;
		}

		iRet = tbus_peek_channel_pkgv(ptChannel, &pstHandle->stRecvHead, a_ppvData, (int *)a_piDataLen) ;
		if ( TBUS_SUCCESS != iRet )
		{
			if ( TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piSrc) ||	TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piDst))
			{
				/* batch received, channel empty or error occured, try next channel */
				pstHandle->iRecvPos = (i + 1) % iAddrCnt ;
				pstHandle->dwRecvPkgCnt = 0 ;
				continue;
			}else
			{
				break;
			}
		}/*if ( TBUS_SUCCESS != iRet )*/

		/*recv data successfully*/
		tbus_log(TLOG_PRIORITY_DEBUG,"recv one msg(len:%d) from %s\n", *a_piDataLen, tbus_addr_ntoa(iSrcAddr));		
		if (TBUS_FLAG_START_DYE_MSG & a_iFlag)
		{
			/*dye msg*/
			tbus_dye_pkg(&pstHandle->stRecvHead, NULL, TBUS_FLAG_START_DYE_MSG);
		}			
		if(pstHandle->stRecvHead.bFlag & TBUS_HEAD_FLAG_TACE)
		{
			tbus_log_dyedpkg(&pstHandle->stRecvHead, "Recv");
		}

		/* not specified source and destination address, update current received position */
		if ( TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piSrc) ||	TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piDst))
		{
			pstHandle->dwRecvPkgCnt ++ ;
		}
		if ( TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piSrc))
		{
			/* set return address */
			*a_piSrc = iSrcAddr ;
		}
		if (TBUS_IS_NON_SPECIFIED_ADDRESS(*a_piDst))
		{
			*a_piDst = iDstAddr ;
		}

		break ; /* ok return */		
	}/*for( i= iCurPos; i< iAddrCnt; i++ )*/

	tbus_log(TLOG_PRIORITY_TRACE,"(Handle:%i, Src:%i, Dst:%i, DataLen:%d, Flag:%i, iRet:%i)",
		a_iHandle, *a_piSrc, *a_piDst, *a_piDataLen, a_iFlag, iRet ) ;

	return iRet ;
}

int tbus_delete_msg(IN const int a_iHandle,	IN TBUSADDR a_iSrc,	INOUT TBUSADDR a_iDst)
{
	int iRet = TBUS_ERR_CHANNEL_EMPTY;
	int i = 0 ;
	TBUSHANDLE *pstHandle = NULL ;
	TBUSCHANNEL *ptChannel = NULL ;
	int iAddrCnt;
	
	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}

	iAddrCnt = pstHandle->dwChannelCnt ;
	for( i= 0; i< iAddrCnt; i++ )
	{
		ptChannel = pstHandle->pastChannelSet[i];
		if ((TBUS_CHANNEL_LOCAL_ADDR(ptChannel) == a_iDst) && 
			(TBUS_CHANNEL_PEER_ADDR(ptChannel) == a_iSrc))
		{	
			break;
		}
	}/*for( i= 0; i< iAddrCnt; i++ )*/
	if (i >= iAddrCnt)
	{
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED);
	}

	iRet = tbus_delete_channel_headpkg(ptChannel);


	return iRet ;	
}


int tbus_forward (IN const int a_iHandle,	INOUT TBUSADDR *a_piSrc,	INOUT TBUSADDR *a_piDst,
				  IN const void *a_pvData,	IN const size_t a_iDataLen,	IN const int a_iFlag)
{
	int iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED);

	int	iAddrCnt = 0,
		i = 0;
	TBUSADDR iSrcAddr = 0,
		iDstAddr = 0,
		iFilterDst = 0,
		iRetSrc = 0,
		iRetDst = 0 ;
	TBUSHANDLE *pstHandle = NULL ;
	TBUSHEAD stHead;
	TBUSHEAD *pstPreHead = NULL;
	struct iovec astVectors[1];

	

	if ( (NULL == a_piSrc) || (NULL == a_piDst) || (NULL == a_pvData) || (0 >= a_iDataLen) )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"parameter error,null paramter or invalid data len %i", a_iDataLen ) ;
		return TBUS_ERR_ARG ;
	}	
	tbus_log(TLOG_PRIORITY_TRACE,"handle: %i, src: %i, Dst: %i, datalen: %i,Flag: %i)",
		a_iHandle, *a_piSrc, *a_piDst, a_iDataLen, a_iFlag ) ;

	/*Get handle*/
	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}
	iAddrCnt = pstHandle->dwChannelCnt ;
	if ( 0 >= iAddrCnt )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"no channel(count:%d) to recv data", iAddrCnt ) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
	}

	
	/*构造消息头部*/
	pstPreHead = &pstHandle->stRecvHead;	
	memset(&stHead, 0, sizeof(TBUSHEAD)) ;
	stHead.bCmd = TBUS_HEAD_CMD_TRANSFER_DATA;
	tbus_dye_pkg(&stHead, pstPreHead, a_iFlag);	

	/*添加路由信息*/
	if (TBUS_FORWARD_MAX_ROUTE < pstPreHead->stExtHead.stDataHead.bRoute)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"the route num(%d) will beyond the max limit(%d), so cannot forward",
			pstPreHead->stExtHead.stDataHead.bRoute, TBUS_FORWARD_MAX_ROUTE);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_FORWARD_ROUTE_LIMITED);
	}
	stHead.stExtHead.stDataHead.bRoute = pstPreHead->stExtHead.stDataHead.bRoute;
	memcpy(&stHead.stExtHead.stDataHead.routeAddr[0], &pstPreHead->stExtHead.stDataHead.routeAddr[0],
		sizeof(int)*stHead.stExtHead.stDataHead.bRoute);
	stHead.bFlag |= TBUS_HEAD_FLAG_WITH_ROUTE;	
	if ( TBUS_ADDR_DEFAULT == *a_piSrc )
	{
		/* get last package's dst addr as forward's src addr */
		*a_piSrc = pstPreHead->iDst ;
		//iFilterDst = pstPreHead->iSrc ; /*设计变更:不对目的地址做过滤，由应用层保证消息流转的正确性*/
	}
	stHead.stExtHead.stDataHead.routeAddr[stHead.stExtHead.stDataHead.bRoute] = *a_piSrc;
	stHead.stExtHead.stDataHead.bRoute++;

	/*转发数据*/
	astVectors[0].iov_base = (void *)a_pvData;
	astVectors[0].iov_len = a_iDataLen;
	for ( i=0; i<iAddrCnt; i++ )
	{
		LPTBUSCHANNEL pstChannel = pstHandle->pastChannelSet[i] ;	
		iSrcAddr = *a_piSrc ;
		iDstAddr = *a_piDst ;
		if ( TBUS_SUCCESS != forward_channel_select_in (pstChannel, &iSrcAddr, &iDstAddr, iFilterDst ) )
		{
			continue ;
		}		

		/* fill address */
		stHead.iDst = iDstAddr;
		stHead.iSrc = iSrcAddr;
		TBUS_SET_LAST_ROUTE(stHead, iSrcAddr);
		iRet = tbus_push_channel_pkgv(pstChannel, &stHead, &astVectors[0], 1) ;
		if (iRet != TBUS_SUCCESS)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"failed to push data to channel(%d), iRet:%d", i, iRet ) ;
			if ( (TBUS_ADDR_ALL == *a_piSrc) || (TBUS_ADDR_ALL == *a_piDst) )
			{
				continue ; /* batch sent mode, skipped individual error */
			}else
			{
				/* failed return, break here, not batch sent mode */
				break ;
			}
		}/*if (iRet != TBUS_SUCCESS)*/

		tbus_log(TLOG_PRIORITY_DEBUG,
			"Send data(len:%d) to channel %d,   distination address %s", stHead.iBodyLen, i, tbus_addr_ntoa(iDstAddr) ) ;
		if(stHead.bFlag & TBUS_HEAD_FLAG_TACE)
		{
			tbus_log_dyedpkg(&stHead, "Send(forward)");
		}
		iRet = TBUS_SUCCESS ;

		/* record first successful channel address */
		iRetSrc = iSrcAddr ;
		iRetDst = iDstAddr ;
		if ( (TBUS_ADDR_ALL != *a_piSrc) && (TBUS_ADDR_ALL != *a_piDst) )
		{
			/* break here, not batch sent mode */
			break ;
		}		
	} /* end for ( i=0; i<iAddrCnt; i++ ) */

	/* set return address */
	if (0 != iRetSrc)
	{
		*a_piSrc = iRetSrc ;
	}
	if (0 != iRetDst)
	{
		*a_piDst = iRetDst ;
	}

	return iRet ;
}




int tbus_backward (IN const int a_iHandle,	INOUT TBUSADDR *a_piSrc,	INOUT TBUSADDR *a_piDst,
				   IN const void *a_pvData,	IN const size_t a_iDataLen,	IN const int a_iFlag)
{
	int iRet = TBUS_ERROR,
		iAddrCnt = 0,
		i = 0;
	TBUSADDR iSrcAddr = 0,
		iDstAddr = 0,
		iRetSrc = 0,
		iRetDst = 0 ;
	TBUSHANDLE *pstHandle = NULL ;
	LPTBUSHEAD pstPreHead = NULL;
	TBUSHEAD stHead;
	struct iovec astVectors[1];

	if ( (NULL == a_piSrc) || (NULL == a_piDst) || (NULL == a_pvData) || (0 >= a_iDataLen) )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"parameter error,null parameter of invalid  len %i", a_iDataLen ) ;
		return TBUS_ERR_ARG ;
	}
	tbus_log(TLOG_PRIORITY_TRACE,"begin to tbus_backward, handle: %i, src:%i, Dst: %i, datalen:%i, flag:%i)",
		a_iHandle, *a_piSrc, *a_piDst, a_iDataLen, a_iFlag ) ;


	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}
	iAddrCnt = pstHandle->dwChannelCnt ;
	if ( 0 >= iAddrCnt )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"no channel(count:%d) to recv data", iAddrCnt ) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
	}


	/*构造消息头部*/
	pstPreHead = &pstHandle->stRecvHead;	
	memset(&stHead, 0, sizeof(TBUSHEAD)) ;
	stHead.bCmd = TBUS_HEAD_CMD_TRANSFER_DATA;
	tbus_dye_pkg(&stHead, pstPreHead, a_iFlag);	

	/*添加路由信息*/
	if ((0 >= pstPreHead->stExtHead.stDataHead.bRoute ) || 
		(TBUS_FORWARD_MAX_ROUTE < pstPreHead->stExtHead.stDataHead.bRoute))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid  route num(%d), the max route limit is (%d), so cannot backward",
			pstPreHead->stExtHead.stDataHead.bRoute, TBUS_FORWARD_MAX_ROUTE);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ROUTE);
	}
	if ( TBUS_ADDR_DEFAULT == *a_piDst )
	{
		/* get last package's dst addr as forward's src addr */
		if (1 > pstPreHead->stExtHead.stDataHead.bRoute)
		{
			/* weird, should *NOT* has this value at this function */
			tbus_log(TLOG_PRIORITY_ERROR,"no dst address in the route info of the pre_recved msg", *a_piDst  ) ;
			return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED) ; ;
		}
		*a_piDst = pstPreHead->stExtHead.stDataHead.routeAddr[pstPreHead->stExtHead.stDataHead.bRoute-1] ;
	}
	for (i = 0; i < pstPreHead->stExtHead.stDataHead.bRoute-1; i++)
	{
		stHead.stExtHead.stDataHead.routeAddr[i] = pstPreHead->stExtHead.stDataHead.routeAddr[i]; 
	}
	stHead.stExtHead.stDataHead.bRoute = i;
	
	astVectors[0].iov_base = (void *)a_pvData;
	astVectors[0].iov_len = a_iDataLen;
	for ( i=0; i<iAddrCnt; i++ )
	{
		LPTBUSCHANNEL pstChannel = pstHandle->pastChannelSet[i] ;
		
		iSrcAddr = *a_piSrc ;
		iDstAddr = *a_piDst ;
		if ( TBUS_SUCCESS != backward_channel_select_in ( pstChannel, &iSrcAddr, &iDstAddr ))
		{
			continue ;
		}		

		/* fill address */
		stHead.iDst = iDstAddr;
		stHead.iSrc = iSrcAddr;
		iRet = tbus_push_channel_pkgv(pstChannel, &stHead, &astVectors[0], 1) ;
		if (iRet != TBUS_SUCCESS)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"failed to push data to channel(%d), iRet:%d", i, iRet ) ;
			if ( (TBUS_ADDR_ALL == *a_piSrc) || (TBUS_ADDR_ALL == *a_piDst) )
			{
				continue ; /* batch sent mode, skipped individual error */
			}else
			{
				/* failed return, break here, not batch sent mode */
				break ;
			}
		}/*if (iRet != TBUS_SUCCESS)*/

		tbus_log(TLOG_PRIORITY_DEBUG,
			"Send data(len:%d) to channel %d,   distination address %s", stHead.iBodyLen, i, tbus_addr_ntoa(iDstAddr) ) ;
		if(stHead.bFlag & TBUS_HEAD_FLAG_TACE)
		{
			tbus_log_dyedpkg(&stHead, "Send(backward)");
		}
		iRet = TBUS_SUCCESS ;

		/* record first successful channel address */
		iRetSrc = iSrcAddr ;
		iRetDst = iDstAddr ;
		if ( (TBUS_ADDR_ALL != *a_piSrc) && (TBUS_ADDR_ALL != *a_piDst) )
		{
			/* break here, not batch sent mode */
			break ;
		}		
	} /* end for ( i=0; i<iAddrCnt; i++ ) */

	/* set return address */
	if (0 != iRetSrc)
	{
		*a_piSrc = iRetSrc ;
	}
	if (0 != iRetDst)
	{
		*a_piDst = iRetDst ;
	}

	return iRet;
}


void tbus_delete ( INOUT int *a_piHandle )
{

	LPTBUSHANDLE pstTbusHandle; 
	int i;

	if ( NULL == a_piHandle )
	{
		return  ; /* treat as deleted ok */
	}
	if ((0 > *a_piHandle) || (*a_piHandle >= HANDLE_COUNT))
	{
		return ;
	}

	pstTbusHandle = g_stBusGlobal.pastTbusHandle[*a_piHandle];
	if (NULL == pstTbusHandle)
	{
		return ;
	}

	/*close all channel handle with this bus handle*/
	for (i = 0; i < (int)pstTbusHandle->dwChannelCnt; i++)
	{
		tbus_close_channel(&pstTbusHandle->pastChannelSet[i]);
	}

	free (pstTbusHandle);
	g_stBusGlobal.pastTbusHandle[*a_piHandle] = NULL ;
	*a_piHandle = TBUS_INVALID_HANDLE ;


}


void tbus_fini ( )
{

	int	i = 0 ;

	/* release handle set */
	for ( i=1; i<HANDLE_COUNT; i++ )
	{
		if ( NULL != g_stBusGlobal.pastTbusHandle[i] )
		{
			int iHandle = i;
			tbus_delete(&iHandle) ;
		}
		g_stBusGlobal.pastTbusHandle[i] = NULL ;
	}

	/* release route map structure */
	g_stBusGlobal.pstGCIM = NULL ;

}



int tbus_get_pkg_route ( IN const int a_iHandle, INOUT HEADROUTE *a_ptRouteVec, INOUT int *a_piCnt )
{
	int iRet = TBUS_SUCCESS,
		iHeadCnt = 0,
		i = 0 ;
	TBUSHEAD *ptHead = NULL ;
	LPTBUSHANDLE pstHandle;

	if ( (NULL == a_ptRouteVec) || (NULL == a_piCnt) || (0 >= *a_piCnt) )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"parameter error, null paramter or invalid vector coount%d", *a_piCnt) ;
		return  TBUS_ERR_ARG ;
	}

	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}

	ptHead = &pstHandle->stRecvHead ;
	iHeadCnt = ptHead->stExtHead.stDataHead.bRoute;
	if ( 0 >= iHeadCnt )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"head cnt error %i", iHeadCnt ) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ROUTE);
	}else if ( *a_piCnt < iHeadCnt )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"vector not large enough %i < %i", *a_piCnt, iHeadCnt ) ;
		return  TBUS_ERR_ARG ;
	}

	for ( i=0; i<iHeadCnt; i++ )
	{
		a_ptRouteVec[i].iSrc = ptHead->stExtHead.stDataHead.routeAddr[i];
	}
	*a_piCnt = i ;

	return iRet ;
}


int tbus_set_pkg_route ( IN const int a_iHandle, IN const HEADROUTE *a_ptRouteVec, IN const int a_iCnt )
{
	int iRet = TBUS_SUCCESS;

	int	i = 0 ;
	TBUSHEAD *ptHead = NULL ;
	LPTBUSHANDLE pstHandle;

	if ( (NULL == a_ptRouteVec) || (0 >= a_iCnt) )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"tbus_set_pkg_route, parameter error, cnt %i", a_iCnt ) ;
		return TBUS_ERR_ARG ;
	}

	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}
	if (TBUS_FORWARD_MAX_ROUTE <= a_iCnt)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"the route number(%d) is beyond the max limit(%d)", 
			a_iCnt, TBUS_FORWARD_MAX_ROUTE ) ;
		return TBUS_ERR_ARG;
	}

	ptHead = &pstHandle->stRecvHead ;
	for ( i=0; i<a_iCnt; i++ )
	{
		ptHead->stExtHead.stDataHead.routeAddr[i] = a_ptRouteVec[i].iSrc ;
	}
	ptHead->stExtHead.stDataHead.bRoute = i;
	
	return iRet ;
}




int tbus_peer_ctrl ( IN const int a_iHandle, IN const TBUSADDR a_iDstAddr, IN const int a_iMode, IN const int a_iBatch )
{
	int iRet = TBUS_SUCCESS,
		i = 0,
		iAddrCnt = 0 ;
	TBUSCHANNEL *ptAddr = NULL ;
	LPTBUSHANDLE pstHandle;

	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}
	

	iAddrCnt = pstHandle->dwChannelCnt ;
	if ( 0 >= iAddrCnt )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"tbus_peer_ctrl(), no available address" ) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
	}

	for ( i=0; i<iAddrCnt; i++ )
	{
		ptAddr = pstHandle->pastChannelSet[i] ;
		if (!(ptAddr->dwFlag & TBUS_CHANNEL_FLAG_ENABLE) )
		{
			tbus_log(TLOG_PRIORITY_TRACE,"tbus_peer_ctrl(), address info disable %i", i ) ;
			continue ;
		}
		if (!(ptAddr->dwFlag & TBUS_CHANNEL_FLAG_SRC_ENABLE) )
		{
			tbus_log(TLOG_PRIORITY_TRACE,"tbus_peer_ctrl(), source address disable %i", i ) ;
			continue ;
		}
		if (a_iDstAddr != ptAddr->pstHead->astAddr[ptAddr->iSendSide])
		{
			continue;
		}
		
		if ( TBUS_MODE_ENABLE == a_iMode )
		{
			ptAddr->dwFlag |= TBUS_CHANNEL_FLAG_DST_ENABLE ;
			tbus_log(TLOG_PRIORITY_DEBUG,"enable channel which peer address is %s", tbus_addr_ntoa(a_iDstAddr));
		}
		else if ( TBUS_MODE_DISABLE == a_iMode )
		{
			ptAddr->dwFlag = ptAddr->dwFlag & ~TBUS_CHANNEL_FLAG_DST_ENABLE ;
			tbus_log(TLOG_PRIORITY_DEBUG,"disable channel which peer address is %s", tbus_addr_ntoa(a_iDstAddr));
		}
		if ( TBUS_PEER_CTRL_ONE == a_iBatch )
		{
			break ;
		}
		
	}/*for ( i=0; i<iAddrCnt; i++ )*/

	return iRet ;

}



int tbus_save_pkg_header ( IN const int a_iHandle )
{
	LPTBUSHANDLE pstHandle;

	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}

	memcpy(&pstHandle->stHeadBak, &pstHandle->stRecvHead, sizeof(TBUSHEAD));

	return TBUS_SUCCESS ;
}


int tbus_restore_pkg_header ( IN const int a_iHandle )
{
	LPTBUSHANDLE pstHandle;

	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}

	memcpy(&pstHandle->stRecvHead, &pstHandle->stHeadBak, sizeof(TBUSHEAD));

	return TBUS_SUCCESS ;
}


int tbus_get_pkg_header ( IN const int a_iHandle, INOUT void *a_pvBuffer, INOUT int *a_piLen )
{
	LPTBUSHANDLE pstHandle;
	int iRet = TBUS_SUCCESS;

	if ( (NULL == a_pvBuffer) || (NULL == a_piLen) || (0 >= *a_piLen) )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"parameter error, null paramter or invalid len(%d)", *a_piLen) ;
		iRet = TBUS_ERR_ARG ;
		return iRet ;
	}	

	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}

	iRet = tbus_encode_head(&pstHandle->stRecvHead, a_pvBuffer, a_piLen, 0);
	
	return iRet;
}


int tbus_set_pkg_header ( IN const int a_iHandle, IN const void *a_pvBuffer, IN const int a_iLen )
{
	int iRet = TBUS_SUCCESS ;
	TBUSHEAD stHead;
	LPTBUSHANDLE pstHandle;

	if ( (NULL == a_pvBuffer) || (0 >= a_iLen) )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"parameter error, len %d", a_iLen ) ;
		return TBUS_ERR_ARG ;
	}

	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid handle %d", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}

	iRet = tbus_decode_head(&stHead, (char *)a_pvBuffer, a_iLen, 0);
	if (TBUS_SUCCESS == iRet)
	{
		memcpy(&pstHandle->stRecvHead, &stHead, sizeof(TBUSHEAD));
	}
	
	return iRet ;
}


/******* internal functions *******/
int tbus_select_channel_i( IN TBUSCHANNEL *a_ptAddr, INOUT int *a_piSrc, INOUT int *a_piDst )
{
	int iRet = TBUS_ERROR ;
	int iSrcAddr ;
	int iDstAddr;
	CHANNELHEAD *pstHead ;

	assert(NULL != a_ptAddr);
	assert(NULL != a_piSrc);
	assert(NULL != a_piDst);


	if ( !(a_ptAddr->dwFlag & TBUS_ADDR_ENABLE) )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"address info disable" ) ;
		return TBUS_ERROR ;
	}
	if (  ! (a_ptAddr->dwFlag & TBUS_ADDR_SRC_ENABLE) )
	{
		tbus_log(TLOG_PRIORITY_TRACE,"source address disable" ) ;
		return TBUS_ERROR ;
	}
	if ( ! (a_ptAddr->dwFlag & TBUS_ADDR_DST_ENABLE) )
	{
		tbus_log(TLOG_PRIORITY_TRACE,"remote address disable" ) ;
		return TBUS_ERROR ;
	}

	/* locate channel */
	pstHead = a_ptAddr->pstHead;
	iSrcAddr = pstHead->astAddr[a_ptAddr->iRecvSide];
	iDstAddr = pstHead->astAddr[a_ptAddr->iSendSide];
	tbus_log(TLOG_PRIORITY_TRACE,"channel srcaddr %i dstaddr %i",
		iSrcAddr, iDstAddr) ;
	if ( (TBUS_ADDR_DEFAULT == *a_piSrc) || (TBUS_ADDR_ALL == *a_piSrc) )
	{
		if ( (TBUS_ADDR_DEFAULT == *a_piDst) || (TBUS_ADDR_ALL == *a_piDst) )
		{
			*a_piSrc = iSrcAddr;
			*a_piDst = iDstAddr;
			iRet = TBUS_SUCCESS ;
		}else
		{
			if (*a_piDst == iDstAddr)
			{
				*a_piSrc = iSrcAddr;
				iRet = TBUS_SUCCESS ;
			}			
		}/*if ( (TBUS_ADDR_DEFAULT == *a_piDst) || (TBUS_ADDR_ALL == *a_piDst) )*/
	}else
	{	
		if ( (TBUS_ADDR_DEFAULT == *a_piDst) || (TBUS_ADDR_ALL == *a_piDst) )
		{
			if (*a_piSrc == iSrcAddr)
			{
				*a_piDst = iDstAddr;
				iRet = TBUS_SUCCESS ;
			}
		}else
		{
			if ((*a_piSrc == iSrcAddr) && 	(*a_piDst == iDstAddr))
			{
				iRet = TBUS_SUCCESS ;
			}
		}/*if ( (TBUS_ADDR_DEFAULT == *a_piDst) || (TBUS_ADDR_ALL == *a_piDst) )*/
	}/*if ( (TBUS_ADDR_DEFAULT == *a_piSrc) || (TBUS_ADDR_ALL == *a_piSrc) )*/


	return iRet ;
}



int forward_channel_select_in ( IN  TBUSCHANNEL *a_ptAddr, INOUT TBUSADDR *a_piSrc, INOUT TBUSADDR *a_piDst, IN const int a_iFilterDst )
{
	int iRet = TBUS_ERROR ;
	
	/* tbus_log(TLOG_PRIORITY_DEBUG,"forward_channel_select_in, enable flag %i", a_ptAddr->dwFlag ) ; */

	if ( TBUS_ADDR_DEFAULT == *a_piSrc )
	{
		/* weird, should *NOT* has this value at this function */
		tbus_log(TLOG_PRIORITY_ERROR,"ERROR: forward_channel_select_in, src addr weird %i", *a_piSrc  ) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED) ;
	}

	iRet = tbus_select_channel_i(a_ptAddr, a_piSrc, a_piDst);
	if (TBUS_SUCCESS == iRet)
	{
		if ((TBUS_ADDR_DEFAULT != a_iFilterDst) && (TBUS_ADDR_ALL != a_iFilterDst) &&
			(a_iFilterDst == *a_piDst))
		{
			tbus_log(TLOG_PRIORITY_ERROR,"dst addr<%d> is equal to filter addr", a_iFilterDst ) ;
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED) ;;
		}
	}	

	tbus_log(TLOG_PRIORITY_TRACE,"RETN: forward_channel_select_in()=%i, src %i, dst %i", iRet, *a_piSrc, *a_piDst ) ;
	return iRet ;
}


int backward_channel_select_in ( IN  TBUSCHANNEL *a_ptAddr, INOUT TBUSADDR *a_piSrc, INOUT TBUSADDR *a_piDst )
{
	int iRet = TBUS_ERROR ;
	
	/* tbus_log(TLOG_PRIORITY_DEBUG,"backward_channel_select_in, enable flag %i", a_ptAddr->dwFlag ) ; */

	if ( TBUS_ADDR_DEFAULT == *a_piDst  )
	{
		/* weird, should *NOT* has this value at this function */
		tbus_log(TLOG_PRIORITY_ERROR,"ERROR: backward_channel_select_in, dst addr weird %i", *a_piDst  ) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED) ; ;
	}

	iRet = tbus_select_channel_i(a_ptAddr, a_piSrc, a_piDst);
	
	tbus_log(TLOG_PRIORITY_TRACE,"RETN: backward_channel_select_in()=%i, src %i, dst %i", iRet, *a_piSrc, *a_piDst ) ;
	return iRet ;
}


int tbus_get_dst_list ( IN const int a_iHandle, INOUT unsigned int *a_piDstList, INOUT int *a_piVecCnt )
{

	int	iAddrCnt = 0,
		iTotal = 0,
		iVecCnt = 0,
		i = 0 ;
	TBUSHANDLE *pstHandle = NULL ;

	if ((NULL == a_piDstList)||(NULL == a_piVecCnt))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid param: a_piDstList: %p a_piVecCnt: %p",
			a_piDstList, a_piVecCnt);
		return TBUS_ERR_ARG;
	}

	iTotal = *a_piVecCnt ;
	*a_piVecCnt = 0 ; /* reset */

	pstHandle = tbus_get_handle(a_iHandle);
	if (NULL == pstHandle)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to get handle(%d)", a_iHandle);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_HANDLE);
	}
	iAddrCnt = pstHandle->dwChannelCnt ;
	if ( 0 >= iAddrCnt )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"no channel(count:%d) to send data", iAddrCnt ) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL) ;
	}

	if ( iAddrCnt > iTotal )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"dst list exceed %i > %i", iTotal, iTotal ) ;
		return TBUS_ERR_ARG ;
	}

	for ( i=0; i<iAddrCnt; i++ )
	{
		LPTBUSCHANNEL pstChannel = pstHandle->pastChannelSet[i];
		if ( !(pstChannel->dwFlag & TBUS_ADDR_ENABLE) )
		{
			tbus_log(TLOG_PRIORITY_TRACE,"tbus_get_dst_list, address info disable %i", i ) ;
			continue ;
		}

		if ( ! (pstChannel->dwFlag & TBUS_ADDR_SRC_ENABLE) )
		{
			tbus_log(TLOG_PRIORITY_TRACE,"tbus_get_dst_list, source address disable %i", i ) ;
			continue ;
		}

		a_piDstList[iVecCnt] = pstChannel->pstHead->astAddr[pstChannel->iSendSide] ;
		iVecCnt ++ ;
	}

	*a_piVecCnt = iVecCnt ;

	
	return TBUS_SUCCESS ;
}



int tbus_connect ( IN const int a_iHandle, IN const TBUSADDR a_iDstAddr )
{
	char szTmpIP[128];
	STRNCPY(szTmpIP, tbus_addr_ntoa(a_iDstAddr), sizeof(szTmpIP));
	return tbus_connect_by_str ( a_iHandle, &szTmpIP[0]) ;
}

LPTBUSHANDLE tbus_get_handle(IN int a_iHandle)
{
	if ( TBUS_MODULE_INITED != g_stBusGlobal.dwInited )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"global bus module not inited %i\n", a_iHandle ) ;
		return NULL;
	}

	if ((0 > a_iHandle) || (HANDLE_COUNT <= a_iHandle))
	{
		tlog_log(g_pstBusLogCat, TLOG_PRIORITY_ERROR,0,0,"handle illegal %i %i\n", HANDLE_COUNT, a_iHandle ) ;
		return NULL;
	}
	
	return g_stBusGlobal.pastTbusHandle[a_iHandle];
}

int tbus_generate_counterfeit_pkg(LPTBUSHEAD a_pstHead, int *a_piPkgLen)
{
	int iRet = 0;
	TBUSHEAD stCounterfeit;
	
	assert(NULL != a_pstHead);
	assert(NULL != a_piPkgLen);

	/*为了方便tbus处理，当消息通道的尾部不够保存一个完整消息时，则尝试放置一个
	伪造的假消息占满深入空间。此假消息的判断特征：
	1.flag字段的值设置为TBUS_HEAD_FLAG_COUNTERFEIT_DATA
	2.BodyLen 字段设置为0
	*/
	memset(&stCounterfeit,0, sizeof(stCounterfeit));
	stCounterfeit.bCmd = TBUS_HEAD_CMD_TRANSFER_DATA;
	stCounterfeit.bFlag = TBUS_HEAD_FLAG_COUNTERFEIT_DATA;
	stCounterfeit.bVer = TDR_METALIB_TBUSHEAD_VERSION;
	
	*a_piPkgLen = sizeof(TBUSHEAD);
	iRet = tbus_encode_head(&stCounterfeit, (char *)a_pstHead, a_piPkgLen, 0);
	if (0 != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR, "failed to generate counterfeit pkg:%s", tbus_error_string(iRet));
	}else
	{
		tbus_log(TLOG_PRIORITY_INFO, "generate counterfeit pkg(Len:%d) successfully:", *a_piPkgLen);
		tbus_log_data(TLOG_PRIORITY_INFO, g_stBusGlobal.pstHeadMeta, &stCounterfeit, sizeof(TBUSHEAD));
	}

	return iRet;
}

int tbus_init_ex(IN const char *a_pszShmkey, IN int a_iFlag)
{

	int iRet = TBUS_SUCCESS;
	int iTempBussID =0;
	LPTBUSSHMGCIMHEAD pstHead;
	pthread_mutexattr_t attr;

	/*if it has been initialized, just return*/
	if ( TBUS_MODULE_INITED == g_stBusGlobal.dwInited )
	{
		return TBUS_SUCCESS ;
	}

	/*初始化互斥变量锁*/
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&g_stBusGlobal.tMutex, &attr);

	
	TBUS_MUTEX_LOCK(g_stBusGlobal.tMutex);

	/* init log system*/
	iRet = tbus_init_log();
	if (0 != iRet)
	{
		printf("failed to init log system");
	}else
	{
		tbus_set_logpriority(TLOG_PRIORITY_DEBUG);
	}
	
	

	/* init global handle set */
	
	iTempBussID = g_stBusGlobal.iBussId;
	memset ( &g_stBusGlobal, 0, sizeof(g_stBusGlobal) ) ;
	g_stBusGlobal.iBussId = iTempBussID;

	tbus_register_bussid(g_stBusGlobal.iBussId);

	/* get GCIM */
	iRet = tbus_get_gcimshm(&g_stBusGlobal.pstGCIM, a_pszShmkey,g_stBusGlobal.iBussId,
		0, TBUS_ATTACH_SHM_TIMEOUT);
	if ( 0 == iRet )
	{
		pstHead = &g_stBusGlobal.pstGCIM->stHead;
		g_stBusGlobal.dwGCIMVersion = pstHead->dwVersion;
	}else
	{
		tbus_log(TLOG_PRIORITY_ERROR,"tbus_get_gcimshm() failed, iret=%x, %s\n", 
			iRet, tbus_error_string(iRet)) ;	
	}/*if ( 0 == iRet )*/


	/*get tbus head meta*/
	if (0 == iRet)
	{
		iRet = tbus_init_headmeta();
	}
	
	/*构造伪造数据包*/
	if (0 == iRet)
	{
		iRet = tbus_generate_counterfeit_pkg(&g_stBusGlobal.stCounterfeitPkg, &g_stBusGlobal.iCounterfeitPkgLen);
	}
	
	if (0 == iRet)
	{
		g_stBusGlobal.dwInited = TBUS_MODULE_INITED ;
		tbus_log(TLOG_PRIORITY_DEBUG,"tbus_init_ex successfully, shmkey(%d) size(%d) channelnum(%d)",
			pstHead->dwShmKey, pstHead->dwShmSize, pstHead->dwUsedCnt);

		tbus_set_logpriority(TLOG_PRIORITY_ERROR);
	}else
	{
		tbus_fini() ;
	}
	TBUS_MUTEX_UNLOCK(g_stBusGlobal.tMutex);

	return iRet ;
}


int tbus_set_bussid(IN const int a_iBussId )
{
	g_stBusGlobal.iBussId = a_iBussId;

	return tbus_register_bussid(a_iBussId);
}


