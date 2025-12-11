/*
**  @file $RCSfile: tcltapi.c,v $
**  general description of this module
**  $Id: tcltapi.c,v 1.4 2009/01/19 06:53:15 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009/01/19 06:53:15 $
**  @version $Revision: 1.4 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include "tdr/tdr.h"
#include "apps/tcltapi/tcltapi.h"

#define MAX_ERR_STR 2048

struct tagTCltapiHandle
{
	int s;
	int iErr;
	int iSelfVersion;
	int iPeerVersion;
	LPTDRMETA pstRecvMeta;
	LPTDRMETA pstSendMeta;
	char* pszRecvBuff;
	int iRecvBuff;
	int iOff;
	int iData;
	char* pszSendBuff;
	int iSendBuff;
	char szErrString[MAX_ERR_STR];
};

int tcltapi_make_connect(const char* a_pszUri, int a_iTimeout)
{
	return tnet_connect(a_pszUri, a_iTimeout);
}

int tcltapi_new(HTCLTAPI* a_phClt)
{
	HTCLTAPI hClt;

	if( !a_phClt )
		return -1;

	hClt	=	(HTCLTAPI) calloc(1, sizeof(TCLTAPIHANDLE));

	if( !hClt )
		return -1;
	
	hClt->s		=	-1;
	*a_phClt	=	hClt;

	return 0;
}

int tcltapi_free(HTCLTAPI* a_phClt)
{
	if( !a_phClt )
		return -1;

	if( *a_phClt )
	{
		if( -1!=(*a_phClt)->s )
		{
			tnet_close((*a_phClt)->s);
			(*a_phClt)->s	=	-1;
		}

		free(*a_phClt);

		*a_phClt	=	NULL;
	}

	return 0;
}

int tcltapi_detach(HTCLTAPI a_hClt)
{
	int s;

	s		=	a_hClt->s;

	a_hClt->s	=	-1;

	return s;
}

void tcltapi_attach(HTCLTAPI a_hClt, int a_s)
{
	if( -1!=a_hClt->s )
	{
		tnet_close(a_hClt->s);

		a_hClt->s	=	-1;
	}

	a_hClt->s	=	a_s;
}

int tcltapi_open(const char* a_pszUri, int a_iTimeout, HTCLTAPI* a_phClt)
{
	int s=-1;

	s	=	tnet_connect(a_pszUri, a_iTimeout);

	if( -1==s )
		return -1;

	if( -1==tcltapi_new(a_phClt) )
	{
		tnet_close(s);

		return -1;
	}

	tcltapi_attach(*a_phClt, s);
	
	return 0;
}

int tcltapi_set_pdu_meta(HTCLTAPI a_hClt, LPTDRMETA a_pstRecvMeta, LPTDRMETA a_pstSendMeta)
{
	int iSendBuff;
	int iRecvBuff;

	assert(a_pstRecvMeta && a_pstSendMeta);

	a_hClt->pstRecvMeta	=	a_pstRecvMeta;
	a_hClt->pstSendMeta	=	a_pstSendMeta;

	iSendBuff	=	tdr_get_meta_size(a_pstSendMeta);
	iRecvBuff	=	tdr_get_meta_size(a_pstRecvMeta);

	if( iSendBuff>a_hClt->iSendBuff )
	{
		if( a_hClt->pszSendBuff )
		{
			free(a_hClt->pszSendBuff);
			a_hClt->pszSendBuff	=	NULL;
			a_hClt->iSendBuff	=	0;
		}

		a_hClt->pszSendBuff	=	(char*)calloc(1, iSendBuff);

		if( a_hClt->pszSendBuff )
			a_hClt->iSendBuff	=	iSendBuff;
	}

	if( iRecvBuff>a_hClt->iRecvBuff )
	{
		if( a_hClt->pszRecvBuff )
		{
			free(a_hClt->pszRecvBuff);
			a_hClt->pszRecvBuff	=	NULL;
			a_hClt->iRecvBuff	=	0;
		}

		a_hClt->pszRecvBuff	=	(char*)calloc(1, iRecvBuff);

		if( a_hClt->pszRecvBuff )
			a_hClt->iRecvBuff	=	iRecvBuff;
	}

	if( a_hClt->pszSendBuff && a_hClt->pszRecvBuff )
		return 0;
	else
		return -1;
}

void tcltapi_set_version(HTCLTAPI a_hClt, int a_iSelfVersion, int a_iPeerVersion)
{
	a_hClt->iSelfVersion	=	a_iSelfVersion;
	a_hClt->iPeerVersion	=	a_iPeerVersion;
}


int tcltapi_send_buff(HTCLTAPI a_hClt, const char* a_pszBuff, int a_iBuff, int a_iTimeout)
{
	return tnet_send(a_hClt->s, a_pszBuff, a_iBuff, a_iTimeout);
}

int tcltapi_recv_buff(HTCLTAPI a_hClt, char* a_pszBuff, int a_iBuff, int a_iTimeout)
{
	return tnet_recv(a_hClt->s, a_pszBuff, a_iBuff, a_iTimeout);
}

int tcltapi_send_msg(HTCLTAPI a_hClt, const char* a_pszMsg, int a_iMsg, int a_iTimeout)
{
	int iSend;
	int iRet;
	TDRDATA stHost;
	TDRDATA stNet;

	stHost.pszBuff	=	(char*) a_pszMsg;
	stHost.iBuff	=	a_iMsg;

	stNet.pszBuff	=	a_hClt->pszSendBuff;
	stNet.iBuff	=	a_hClt->iSendBuff;

	if( tdr_hton(a_hClt->pstSendMeta, &stNet, &stHost, a_hClt->iPeerVersion)<0 )
		return -1;

	iSend	=	tcltapi_send_buff(a_hClt, stNet.pszBuff, stNet.iBuff, a_iTimeout);
	
	if( iSend==stNet.iBuff )
	{
		iRet	=	0;
	}
	else
	{
		iRet	=	-1;
	}

	return iRet;
}

int tcltapi_get_err(HTCLTAPI a_hClt)
{
	return a_hClt->iErr;
}

int tcltapi_get_msg(HTCLTAPI a_hClt, char* a_pszMsg, int* a_piMsg)
{
	TDRDATA stHost;
	TDRDATA stNet;
	int iRet;

	if (0 >= a_hClt->iData)
		return -1;
	
	stHost.pszBuff	=	a_pszMsg;
	stHost.iBuff	=	*(a_piMsg);

	stNet.pszBuff	=	a_hClt->pszRecvBuff + a_hClt->iOff;
	stNet.iBuff	=	a_hClt->iData;


	iRet	=	tdr_ntoh(a_hClt->pstRecvMeta, &stHost, &stNet, a_hClt->iPeerVersion);

	if( 0==iRet )
	{
		a_hClt->iOff	+=	stNet.iBuff;
		a_hClt->iData	-=	stNet.iBuff;

		*a_piMsg	=	stNet.iBuff;

		return 0;
	}
	
	return -1;
}

int tcltapi_recv_msg(HTCLTAPI a_hClt, char* a_pszMsg, int* a_piMsg, int a_iTimeout)
{
	char* pszRecvBuff;
	int iRecvBuff;
	int iRecv;

	if( a_hClt->iErr )
		return -1;

	if( 0==tcltapi_get_msg(a_hClt, a_pszMsg, a_piMsg) )
	{
		return 1;
	}
	else if( a_hClt->iOff )
	{
		if( a_hClt->iData )
			memmove(a_hClt->pszRecvBuff, a_hClt->pszRecvBuff+a_hClt->iOff, a_hClt->iData);
			
		a_hClt->iOff	=	0;
	}

	pszRecvBuff	=	a_hClt->pszRecvBuff + a_hClt->iOff + a_hClt->iData;
	iRecvBuff	=	a_hClt->iRecvBuff - a_hClt->iOff - a_hClt->iData;

	if( iRecvBuff<=0 )
	{
		a_hClt->iErr	=	TCLTAPI_ERR_BUF;
		snprintf(a_hClt->szErrString, sizeof(a_hClt->szErrString), "XXXXX ERR BUFF, iRecvBuff=%d, iOff=%d, iData=%d\n", 
				a_hClt->iRecvBuff, a_hClt->iOff, a_hClt->iData);
		return -1;
	}
	
	iRecv	=	tnet_recv(a_hClt->s, pszRecvBuff, iRecvBuff, a_iTimeout);

	if( iRecv<0 )
	{
		a_hClt->iErr	=	TCLTAPI_ERR_NET;
		return -1;
	}
	else if( 0==iRecv )
	{
		return 0;
	}

	a_hClt->iData	+=	iRecv;
	
	
	if( 0==tcltapi_get_msg(a_hClt, a_pszMsg, a_piMsg) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int tcltapi_get_sock(HTCLTAPI a_hClt)
{
	return a_hClt->s;
}

int tcltapi_set_recvbuff(HTCLTAPI a_hClt, int a_iSize)
{
	return tnet_set_recvbuff(a_hClt->s, a_iSize);
}

int tcltapi_set_sendbuff(HTCLTAPI a_hClt, int a_iSize)
{
	return tnet_set_sendbuff(a_hClt->s, a_iSize);
}

char *tcltapi_get_errstring(HTCLTAPI a_hClt)
{
	return &a_hClt->szErrString[0];
}

