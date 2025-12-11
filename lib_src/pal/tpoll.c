/*
**  @file $RCSfile: tpoll.c,v $
**  general description of this module
**  $Id: tpoll.c,v 1.2 2008/06/13 09:13:20 steve Exp $
**  @author $Author: steve $
**  @date $Date: 2008/06/13 09:13:20 $
**  @version $Revision: 1.2 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/


#include <stddef.h>
#include <assert.h>
#include "pal/tos.h"
#include "pal/ttypes.h"
#include "pal/tsocket.h"
#include "pal/tpoll.h"

#ifdef WIN32
	#include "mswsock.h"

	#define TPOLL_BUFF_SIZE		128
	#define TPOLL_BAD_POS		-1
	#define TPOLL_KEY_CANCEL	-1

	#define TPOLLF_USED			1

struct tagTPollFd
{
	HANDLE hFile;
	LPFN_ACCEPTEX lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS lpfnAddr;
	int sAccept;
	int iState;
	int iBuff;
	int iData;
	int iOff;
	tpoll_event_t ev;
	OVERLAPPED o;
	int fStream;
	int iSrcLen;
	struct sockaddr stSrcAddr;
	char szBuff[TPOLL_BUFF_SIZE];
};

typedef struct tagTPollFd		TPOLLFD;
typedef struct tagTPollFd		*LPTPOLLFD;
typedef struct tagTPollFd		*PTPOLLFD;

struct tagTPollItem
{
	int iPrev;
	int iNext;
	int iSeq;
	int iFlag;
	TPOLLFD stFd;
};

typedef struct tagTPollItem		TPOLLITEM;
typedef struct tagTPollItem		*LPTPOLLITEM;

struct tagTPollPort
{
	int iMaxFd;
	int iUsed;
	int iHead;
	int iTail;
	int iFree;
	HANDLE hIOCP;
	TPOLLITEM items[1];
};

typedef struct tagTPollPort		TPOLLPORT;
typedef struct tagTPollPort		*LPTPOLLPORT;

int tpoll_create (int a_iSize)
{
	int iLen;
	int iNext;
	LPTPOLLPORT pstPort;
	int i;

	if( a_iSize>TPOLL_MAX_FD )
		return -1;

	iLen	=	offsetof(TPOLLPORT, items) + a_iSize*sizeof(TPOLLITEM);
	if( iLen<0 )
		return -1;

	pstPort	=	calloc(1, iLen);
	if(!pstPort)
		return -1;

	pstPort->iMaxFd	=	a_iSize;
	pstPort->iUsed	=	0;

	iNext	=	TPOLL_BAD_POS;

	/* initialize the free chain. */
	for(i=a_iSize-1; i>=0; i--)
	{
		pstPort->items[i].iNext	=	iNext;
		pstPort->items[i].iPrev	=	TPOLL_BAD_POS;
		pstPort->items[i].iSeq	=	0;
		pstPort->items[i].iFlag	=	0;
		pstPort->items[i].stFd.hFile	=	INVALID_HANDLE_VALUE;
		pstPort->items[i].stFd.sAccept=	-1;
		pstPort->items[i].stFd.iBuff	=	(int)sizeof(pstPort->items[i].stFd.szBuff);
		iNext	=	i;
	}

	pstPort->iFree	=	0;
	pstPort->iHead	=	TPOLL_BAD_POS;
	pstPort->iTail	=	TPOLL_BAD_POS;

	pstPort->hIOCP	=	CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if( !pstPort->hIOCP )
	{
		free(pstPort);
		return -1;
	}

	return (int)pstPort;
}

int tpoll_start_recv_i(LPTPOLLFD a_pstFD)
{
	WSABUF stBuff;
	int iRet;
	DWORD dwFlags;

	assert( !a_pstFD->iState && !a_pstFD->lpfnAcceptEx && HasOverlappedIoCompleted(&a_pstFD->o) );
	
	a_pstFD->iData	=	0;
	a_pstFD->iOff		=	0;
	a_pstFD->iState	=	-1;

	stBuff.len	=	a_pstFD->iBuff;
	stBuff.buf	=	a_pstFD->szBuff;
	dwFlags	=	0;

	iRet	=	WSARecv((int)a_pstFD->hFile, &stBuff, 1, (LPDWORD)&a_pstFD->iData, &dwFlags, &a_pstFD->o, NULL);

	if( !iRet )
		a_pstFD->iState	=	0;
	else if( WSA_IO_PENDING==WSAGetLastError() )
		a_pstFD->iState	=	WSA_IO_PENDING;

	return iRet;
}

int tpoll_start_accept_i(LPTPOLLFD a_pstFD)
{
	int iRet;

	assert( !a_pstFD->iState && a_pstFD->lpfnAcceptEx && HasOverlappedIoCompleted(&a_pstFD->o) );

	a_pstFD->iData	=	0;
	a_pstFD->iOff		=	0;
	a_pstFD->iState	=	-1;

	a_pstFD->sAccept	=	socket(AF_INET, SOCK_STREAM, 0);
	if( -1==a_pstFD->sAccept )
		return -1;
	
	iRet	=	a_pstFD->lpfnAcceptEx((int)a_pstFD->hFile, a_pstFD->sAccept, a_pstFD->szBuff, 0, 
		sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16, (LPDWORD)&a_pstFD->iData, &a_pstFD->o);

	if( !iRet )
		a_pstFD->iState	=	0;
	else if( WSA_IO_PENDING==WSAGetLastError() )
		a_pstFD->iState	=	WSA_IO_PENDING;

	return iRet;
}

int tpoll_ctl_add_i(LPTPOLLPORT a_pstPort, int a_iFd, TPOLL_EVENT* a_pstEvent)
{
	int iRet;
	BOOL bAccept;
	int iLen;
	int iFree;
	ULONG_PTR ulKey;
	TPOLLITEM* pstItem;
	int iType;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidAddrEx = WSAID_GETACCEPTEXSOCKADDRS;
	DWORD dwBytes;

	assert(a_pstPort && a_pstEvent);

	/* not support flags other than EPOLLIN */
	if( a_pstEvent->events & ~EPOLLIN )
		return -1;

	bAccept	=	0;
	iLen	=	(int)sizeof(bAccept);

	if( -1==getsockopt(a_iFd, SOL_SOCKET, SO_ACCEPTCONN, (char*)&bAccept, &iLen) )
		return -1;

	iLen	=	(int)sizeof(bAccept);

	if( -1==getsockopt(a_iFd, SOL_SOCKET, SO_TYPE, (char*)&iType, &iLen) )
		return -1;

	if( TPOLL_BAD_POS==a_pstPort->iFree )
		return -1;

	iFree			=	a_pstPort->iFree;
	pstItem			=	a_pstPort->items + iFree;

	memset(&pstItem->stFd, 0, sizeof(pstItem->stFd));

	pstItem->stFd.sAccept	=	-1;

	if( SOCK_STREAM==iType )
		pstItem->stFd.fStream	=	1;
	else
		pstItem->stFd.fStream	=	0;

	pstItem->stFd.iBuff	=	sizeof(pstItem->stFd.szBuff);

	if( bAccept )
	{
		WSAIoctl(a_iFd, 
			SIO_GET_EXTENSION_FUNCTION_POINTER, 
			&GuidAcceptEx, 
			sizeof(GuidAcceptEx),
			&pstItem->stFd.lpfnAcceptEx, 
			sizeof(pstItem->stFd.lpfnAcceptEx), 
			&dwBytes, 
			NULL, 
			NULL);

		WSAIoctl(a_iFd, 
			SIO_GET_EXTENSION_FUNCTION_POINTER, 
			&GuidAddrEx, 
			sizeof(GuidAddrEx),
			&pstItem->stFd.lpfnAddr, 
			sizeof(pstItem->stFd.lpfnAddr), 
			&dwBytes, 
			NULL, 
			NULL);

		if( !pstItem->stFd.lpfnAcceptEx || !pstItem->stFd.lpfnAddr )
			return -1;
	}

	pstItem->iSeq++;
	ulKey	=	(ULONG_PTR) TPOLL_MAKE_KEY(iFree, pstItem->iSeq);

	/* now begin initialize the tpoll item. */
	if( NULL==CreateIoCompletionPort( (HANDLE)a_iFd, a_pstPort->hIOCP, ulKey, 0) )
		return -1;

	pstItem->stFd.hFile	=	(HANDLE)a_iFd;
	a_pstEvent->ulKey	=	ulKey;
	memcpy(&pstItem->stFd.ev, a_pstEvent, sizeof(*a_pstEvent));

	if( bAccept )
		iRet	=	tpoll_start_accept_i(&pstItem->stFd);
	else
		iRet	=	tpoll_start_recv_i(&pstItem->stFd);

	if( -1==pstItem->stFd.iState )
		return -1;

	/* remove it from free chain */
	a_pstPort->iFree	=	pstItem->iNext;

	/* put it into used chain. */
	pstItem->iNext	=	a_pstPort->iHead;
	pstItem->iPrev	=	TPOLL_BAD_POS;

	pstItem->iFlag	=	TPOLLF_USED;

	if( TPOLL_BAD_POS!=a_pstPort->iHead )
		a_pstPort->items[a_pstPort->iHead].iPrev	=	iFree;

	if( TPOLL_BAD_POS==a_pstPort->iTail )
		a_pstPort->iTail	=	iFree;

	a_pstPort->iUsed++;

	return 0;
}

int tpoll_ctl_mod_i(LPTPOLLPORT a_pstPort, int a_iFd, TPOLL_EVENT* a_pstEvent)
{
	int iIdx;
	LPTPOLLITEM pstItem;

	iIdx	=	TPOLL_GET_IDX(a_pstEvent->ulKey);
	if( iIdx<0 || iIdx>=a_pstPort->iMaxFd )
		return -1;

	pstItem	=	a_pstPort->items + iIdx;

	if( a_pstEvent->ulKey!=pstItem->stFd.ev.ulKey || pstItem->stFd.hFile!=(HANDLE)a_iFd)
		return -1;

	/* not implementation. */

	return 0;
}

int tpoll_ctl_del_i(LPTPOLLPORT a_pstPort, int a_iFd, TPOLL_EVENT* a_pstEvent)
{
	int iIdx;
	LPTPOLLITEM pstItem;

	assert(a_pstPort && a_pstEvent);

	iIdx	=	TPOLL_GET_IDX(a_pstEvent->ulKey);
	if( iIdx<0 || iIdx>=a_pstPort->iMaxFd )
		return -1;

	pstItem	=	a_pstPort->items + iIdx;

	if( a_pstEvent->ulKey!=pstItem->stFd.ev.ulKey || pstItem->stFd.hFile!=(HANDLE)a_iFd )
		return -1;

	CancelIo((HANDLE)a_iFd);

	/* remove it from the used chain. */
	if( TPOLL_BAD_POS==pstItem->iPrev )
		a_pstPort->iHead	=	pstItem->iNext;
	else
		a_pstPort->items[pstItem->iPrev].iNext	=	pstItem->iNext;

	if( TPOLL_BAD_POS==pstItem->iNext )
		a_pstPort->iTail	=	pstItem->iPrev;
	else
		a_pstPort->items[pstItem->iNext].iPrev	=	pstItem->iPrev;

	/* put it into free chain. */
	pstItem->iNext	=	a_pstPort->iFree;
	pstItem->iPrev	=	TPOLL_BAD_POS;
	pstItem->iFlag	=	0;
	a_pstPort->iFree	=	iIdx;
	a_pstPort->iUsed--;

	pstItem->stFd.hFile=	INVALID_HANDLE_VALUE;

	return 0;
}

int tpoll_ctl(int a_iTpfd, int a_iOp, int a_iFd, TPOLL_EVENT* a_pstEvent)
{
	LPTPOLLPORT pstPort;
	int iRet;

	if( -1==a_iTpfd || 0==a_iTpfd )
		return -1;

	pstPort	=	(LPTPOLLPORT) a_iTpfd;

	switch( a_iOp )
	{
	case TPOLL_CTL_ADD:
		iRet	=	tpoll_ctl_add_i(pstPort, a_iFd, a_pstEvent);
		break;
	case TPOLL_CTL_MOD:
		iRet	=	tpoll_ctl_mod_i(pstPort, a_iFd, a_pstEvent);
		break;
	case TPOLL_CTL_DEL:
		iRet	=	tpoll_ctl_del_i(pstPort, a_iFd, a_pstEvent);
		break;
	default:
		return -1;
	}

	return iRet;
}


int tpoll_wait(int a_iTpfd, struct epoll_event* a_pstEvents, int a_iMaxEvents, int a_iTimeout)
{
	LPTPOLLPORT pstPort;
	LPTPOLLFD pstFD;
	DWORD dwCount;
	ULONG_PTR ulKey;
	int iIdx;
	LPOVERLAPPED pstOverlap;
	BOOL bRet;
	int i;

	assert( a_pstEvents );

	if( -1==a_iTpfd || 0==a_iTpfd || a_iMaxEvents<1 )
		return -1;

	pstPort	=	(LPTPOLLPORT) a_iTpfd;
	
	pstOverlap=	NULL;
	bRet	=	GetQueuedCompletionStatus(pstPort->hIOCP, &dwCount, &ulKey, &pstOverlap, a_iTimeout);

	if( !bRet && !pstOverlap )
	{
		if( WAIT_TIMEOUT==GetLastError() )
			return 0;
		else
			return -1;
	}

	assert(pstOverlap);

	iIdx	=	TPOLL_GET_IDX(ulKey);
	i		=	0;

	if( iIdx>=0 && iIdx<pstPort->iMaxFd && TPOLLF_USED==pstPort->items[iIdx].iFlag &&
		ulKey==pstPort->items[iIdx].stFd.ev.ulKey )
	{
		pstFD	=	&pstPort->items[iIdx].stFd;

		if( WSA_IO_PENDING==pstFD->iState )
		{
			pstFD->iData	=	(int)dwCount;
			pstFD->iState	=	0;
		}

		assert(pstFD->iData>=0 && pstFD->iData<=pstFD->iBuff);
		a_pstEvents[i].events	=	EPOLLIN;
		a_pstEvents[i].ulKey	=	ulKey;
		memcpy(&a_pstEvents[i].data, &pstFD->ev.data, sizeof(pstFD->ev.data));
		i++;
	}

	while(i<a_iMaxEvents)
	{
		pstOverlap=	NULL;
		bRet	=	GetQueuedCompletionStatus(pstPort->hIOCP, &dwCount, &ulKey, &pstOverlap, 0);

		if( !bRet && !pstOverlap )
		{
			if( i>0 )	return i;

			if( WAIT_TIMEOUT==GetLastError() )
				return 0;
			else
				return -1;
		}

		assert(pstOverlap);

		iIdx	=	TPOLL_GET_IDX(ulKey);
		i		=	0;

		if( iIdx>=0 && iIdx<pstPort->iMaxFd && TPOLLF_USED==pstPort->items[iIdx].iFlag &&
			ulKey==pstPort->items[iIdx].stFd.ev.ulKey )
		{
			pstFD	=	&pstPort->items[iIdx].stFd;

			if( WSA_IO_PENDING==pstFD->iState )
			{
				pstFD->iData	=	(int)dwCount;
				pstFD->iState	=	0;
			}

			assert(pstFD->iData>=0 && pstFD->iData<=pstFD->iBuff);
			a_pstEvents[i].events	=	EPOLLIN;
			a_pstEvents[i].ulKey	=	ulKey;
			memcpy(&a_pstEvents[i].data, &pstFD->ev.data, sizeof(pstFD->ev.data));
			i++;
		}
	}

	return i;
}

int tpoll_destroy(int a_iTpfd)
{
	LPTPOLLPORT pstPort;
	LPTPOLLITEM pstItem;
	int iPtr;

	if( -1==a_iTpfd || 0==a_iTpfd )
		return 0;

	pstPort	=	(LPTPOLLPORT) a_iTpfd;

	CloseHandle(pstPort->hIOCP);
	pstPort->hIOCP	=	NULL;

	iPtr	=	pstPort->iHead;

	while(TPOLL_BAD_POS!=iPtr)
	{
		pstItem	=	pstPort->items + iPtr;

		if( pstItem->stFd.hFile )
			CancelIo(pstItem->stFd.hFile);

		iPtr	=	pstItem->iNext;
	}

	free(pstPort);

	return 0;
}

int tpoll_recvfrom(int a_iTpfd, int a_iFd, struct epoll_event* a_pstEvent, char* a_pszBuff, int a_iBuff, struct sockaddr* a_pstSrcAddr, int* a_piSrcLen)
{
	LPTPOLLPORT pstPort;
	int iIdx;
	LPTPOLLFD pstFD;
	int iData;
	int iRecv;

	assert( a_pstEvent && a_pszBuff && a_iBuff>0 );

	if( -1==a_iTpfd || 0==a_iTpfd || a_iBuff<0 )
		return -1;

	pstPort	=	(LPTPOLLPORT) a_iTpfd;

	iIdx	=	TPOLL_GET_IDX(a_pstEvent->ulKey);
	if( iIdx<0 || iIdx>=pstPort->iMaxFd || TPOLLF_USED!=pstPort->items[iIdx].iFlag )
		return -1;

	pstFD	=	&pstPort->items[iIdx].stFd;
	if( pstFD->hFile!=(HANDLE)a_iFd || a_pstEvent->ulKey!=pstFD->ev.ulKey || pstFD->iState )
		return -1;

	if( pstFD->iData<=0 )
		return pstFD->iData;

	assert(pstFD->iOff<=pstFD->iBuff);
	
	if( pstFD->iData>a_iBuff )
	{
		memcpy(a_pszBuff, pstFD->szBuff + pstFD->iOff, a_iBuff );
		pstFD->iData	-=	a_iBuff;
		pstFD->iOff		+=	a_iBuff;
		return a_iBuff;
	}

	memcpy(a_pszBuff, pstFD->szBuff + pstFD->iOff, pstFD->iData );
	iData	=	pstFD->iData;
	pstFD->iData	=	0;
	pstFD->iOff		=	0;

	a_iBuff	-=	iData;
	a_pszBuff	+=	iData;

	if( a_iBuff>0 )
		iRecv	=	recv(a_iFd, a_pszBuff, a_iBuff, 0);
	else
		iRecv	=	0;

	if( iRecv>0 )
		iData	+=	iRecv;

	tpoll_start_recv_i(pstFD);
	
	if( pstFD->iState<0 )
		return -1;
	else
		return iData;
}


int tpoll_recv(int a_iTpfd, int a_iFd, struct epoll_event* a_pstEvent, char* a_pszBuff, int a_iBuff)
{
	return tpoll_recvfrom(a_iTpfd, a_iFd, a_pstEvent, a_pszBuff, a_iBuff, NULL, NULL);
}

int tpoll_sendto(int a_iTpfd, int a_iFd, TPOLL_EVENT* a_pstEvent, char* a_pszBuff, int a_iBuff, struct sockaddr* a_pstDst, int a_iDstLen)
{
	LPTPOLLPORT pstPort;
	int iIdx;
	LPTPOLLFD pstFD;

	assert( a_pstEvent && a_pszBuff && a_iBuff>0 );

	if( -1==a_iTpfd || 0==a_iTpfd )
		return -1;

	pstPort	=	(LPTPOLLPORT) a_iTpfd;
	
	iIdx	=	TPOLL_GET_IDX(a_pstEvent->ulKey);
	if( iIdx<0 || iIdx>=pstPort->iMaxFd )
		return -1;
	
	pstFD	=	&pstPort->items[iIdx].stFd;
	if( pstFD->hFile!=(HANDLE)a_iFd || pstFD->ev.ulKey!=a_pstEvent->ulKey )
		return -1;

	if( pstFD->fStream )
		return send(a_iFd, a_pszBuff, a_iBuff, 0);
	else
		return sendto(a_iFd, a_pszBuff, a_iBuff, 0, a_pstDst, a_iDstLen);
}

int tpoll_send(int a_iTpfd, int a_iFd, TPOLL_EVENT* a_pstEvent, char* a_pszBuff, int a_iBuff)
{
	return tpoll_sendto(a_iTpfd, a_iFd, a_pstEvent, a_pszBuff, a_iBuff, NULL, 0);
}

int tpoll_accept(int a_iTpfd, int a_iFd, struct epoll_event* a_pstEvent, struct sockaddr* a_pstAddr, int* a_piLen)
{
	LPTPOLLPORT pstPort;
	int iIdx;
	LPTPOLLFD pstFD;
	int iConnTime;
	int iBuff;
	int iRet;
	int s;
	int iLocal;
	struct sockaddr* pstLocal;
	int iRemote;
	struct sockaddr* pstRemote;

	assert( a_pstEvent );

	s	=	-1;

	if( -1==a_iTpfd || 0==a_iTpfd )
	{
		WSASetLastError(WSAEINVAL);
		return -1;
	}

	pstPort	=	(LPTPOLLPORT) a_iTpfd;

	iIdx	=	TPOLL_GET_IDX(a_pstEvent->ulKey);
	if( iIdx<0 || iIdx>=pstPort->iMaxFd )
	{
		WSASetLastError(WSAEINVAL);
		return -1;
	}

	pstFD	=	&pstPort->items[iIdx].stFd;
	if( pstFD->hFile!=(HANDLE)a_iFd || pstFD->ev.ulKey!=a_pstEvent->ulKey || 
		-1==pstFD->sAccept || pstFD->iState )
	{
		WSASetLastError(WSAEINVAL);
		return -1;
	}

	iBuff		= (int)sizeof(iConnTime);
	iConnTime	= -1;
	iRet = getsockopt( pstFD->sAccept, SOL_SOCKET, SO_CONNECT_TIME, (char *)&iConnTime, &iBuff);

	if( iRet<0 )
	{
		WSASetLastError(WSAEINVAL);
		return -1;
	}

	if( -1==iConnTime )
	{
		s	=	-1;
		closesocket(pstFD->sAccept);
		pstFD->sAccept	=	-1;
	}
	else
	{
		pstFD->lpfnAddr(pstFD->szBuff, 0, sizeof(struct sockaddr_in)+16, 
			sizeof(struct sockaddr_in)+16, &pstLocal, &iLocal, &pstRemote, &iRemote);

		if( a_piLen && a_pstAddr && *a_piLen>=iRemote )
			memcpy(a_pstAddr, pstRemote, iRemote);

		if( a_piLen )
			*a_piLen	=	iRemote;

		s	=	pstFD->sAccept;
		pstFD->sAccept	=	-1;
	}

	tpoll_start_accept_i(pstFD);
	
	return s;
}

#else /* WIN32 */

int tpoll_recvfrom(int a_iTpfd, int a_iFd, struct epoll_event* a_pstEvent, char* a_pszBuff, int a_iBuff, struct sockaddr* pstSrcAddr, int* piSrcLen)
{
	return recvfrom(a_iFd, a_pszBuff, (size_t)a_iBuff, 0, pstSrcAddr, (unsigned int*)piSrcLen);
}

int tpoll_recv(int a_iTpfd, int a_iFd, TPOLL_EVENT* a_pstEvent, char* a_pszBuff, int a_iBuff)
{
	return recv(a_iFd, a_pszBuff, (size_t)a_iBuff, 0);
}

int tpoll_sendto(int a_iTpfd, int a_iFd, TPOLL_EVENT* a_pstEvent, char* a_pszBuff, int a_iBuff, struct sockaddr* pstDst, int iDstLen)
{
	return sendto(a_iFd, a_pszBuff, (size_t)a_iBuff, 0, pstDst, iDstLen);
}

int tpoll_send(int a_iTpfd, int a_iFd, TPOLL_EVENT* a_pstEvent, char* a_pszBuff, int a_iBuff)
{
	return send(a_iFd, a_pszBuff, (size_t)a_iBuff, 0);
}

int tpoll_accept(int a_iTpfd, int a_iFd, TPOLL_EVENT* a_pstEvent, struct sockaddr* a_pstAddr, int* a_piLen)
{
	int iRet;
	socklen_t iLen;

	if( a_piLen )
	{
		iLen		= (socklen_t) *a_piLen;
		iRet		= accept(a_iFd, a_pstAddr, &iLen);
		*a_piLen	= iLen;
	}
	else
	{
		iRet		= accept(a_iFd, a_pstAddr, NULL);
	}

	return iRet;
}

#endif /* WIN32 */
