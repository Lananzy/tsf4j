/*
**  @file $RCSfile: tlog_category.c,v $
**  general description of this module
**  $Id: tlog_category.c,v 1.11 2009/03/27 06:17:02 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009/03/27 06:17:02 $
**  @version $Revision: 1.11 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include "tlog/tlog_event.h"
#include "tlog/tlogfilter.h"
#include "tlog/tlogbin.h"
#include "tlog/tlogfile.h"
#include "tlog/tlognet.h"
#include "tlog/tlogvec.h"
#include "tlog/tlogany.h"
#include "tlog/tlog_layout.h"
#include "tlog/tlog_category.h"

#define evt_is_enabled(cat, priority) \
	( ( TLOG_DEV_NO != (cat)->stDevice.iType ) && ( TLOG_PRIORITY_NULL==(cat)->iPriorityHigh || (priority)>=(cat)->iPriorityHigh ) \
	&& ( TLOG_PRIORITY_NULL==(cat)->iPriorityLow || (priority)<=(cat)->iPriorityLow ) )


int tlog_category_append(TLOGCATEGORYINST *a_pstCatInst, TLOGEVENT *a_pstEvt)
{
	TLOGIOVEC astIOVec[2];
	int iVec;
	int iRet;

	if (!evt_is_enabled(a_pstCatInst->pstCat, a_pstEvt->evt_priority))
	{
		return 0;
	}

	if( !a_pstCatInst->iInited )
	{
		if( -1==tlogany_init(&a_pstCatInst->stLogAny, &a_pstCatInst->pstCat->stDevice) )
			return -1;

		a_pstCatInst->iInited	=	1;
	}

	if( a_pstEvt->evt_rendered_msg )
	{
		iVec	=	1;
		astIOVec[0].iov_base	=	(void*)a_pstEvt->evt_rendered_msg;
		astIOVec[0].iov_len		=	(size_t)a_pstEvt->evt_rendered_msg_len;
	}
	else if( a_pstEvt->evt_buffer.buf_data )
	{
		iVec	=	2;
		astIOVec[0].iov_base	=	(void*)a_pstEvt->evt_buffer.buf_data;
		astIOVec[0].iov_len		=	(size_t)a_pstEvt->evt_buffer.buf_size;
		astIOVec[1].iov_base	=	(void*)a_pstEvt->evt_msg;
		astIOVec[1].iov_len		=	(size_t)a_pstEvt->evt_msg_len;
	}
	else
	{
		iVec	=	1;
		astIOVec[0].iov_base	=	(void*)a_pstEvt->evt_msg;
		astIOVec[0].iov_len		=	(size_t)a_pstEvt->evt_msg_len;
	}

	iRet	=	tlogany_writev(&a_pstCatInst->stLogAny, a_pstEvt->evt_id, a_pstEvt->evt_cls, astIOVec, iVec);

	return iRet;
}

int tlog_category_logv(TLOGCATEGORYINST *a_pstCatInst, TLOGEVENT *a_pstEvt, const char* a_pszFmt, ...)
{
	va_list ap;
	char szBuff[TLOG_BUFFER_SIZE_DEFAULT];
	

	char *pszMessage = NULL;
	int iMessageLen;
	int iRet;

	szBuff[0] = '\0';
	if( a_pstEvt->evt_is_msg_binary )
		return -1;

	va_start(ap, a_pszFmt);

	if( a_pstCatInst->pstCat->iMaxMsgSize<=(int)sizeof(szBuff) ) 
	{
		pszMessage = szBuff;
		iMessageLen = (int)sizeof(szBuff);
	}
	else 
	{
		iMessageLen = a_pstCatInst->pstCat->iMaxMsgSize;
		pszMessage = alloca(iMessageLen);	

		if( !pszMessage )
		{
			pszMessage = szBuff;
			iMessageLen = (int)sizeof(szBuff);
		}
	}

	errno = 0;
	a_pstEvt->evt_msg_len	=	vsnprintf(pszMessage, iMessageLen, a_pszFmt, ap);
	a_pstEvt->evt_msg 		=	pszMessage;

#ifdef WIN32
	if (a_pstEvt->evt_msg_len < 0)
	{
		if (EINVAL == errno)
		{
			return -1;
		}
		else
		{
			a_pstEvt->evt_msg_len = iMessageLen;
		}
	}

	if (a_pstEvt->evt_msg_len == iMessageLen)
	{
		a_pstEvt->evt_msg_len = iMessageLen - 1;
		*(pszMessage + a_pstEvt->evt_msg_len - 1) = '\0';
	}
#else 
	if( a_pstEvt->evt_msg_len<0 )
	{
		return -1;
	}
	
	if (a_pstEvt->evt_msg_len >= iMessageLen)
	{
		a_pstEvt->evt_msg_len = iMessageLen - 1;
		*(pszMessage + a_pstEvt->evt_msg_len - 1) = '\0';
	}
#endif
		
	iRet	=	tlog_category_log(a_pstCatInst, a_pstEvt);
	va_end(ap);
	return iRet;
}


int tlog_category_log_in(TLOGCATEGORYINST *a_pstCatInst, TLOGEVENT *a_pstEvt)
{
	TLOGCATEGORYINST *pstCatInst;
	int iRet=0;

	TLOGBINHEAD stHead;
	char szBuff[TLOGBIN_MAX_DATA + sizeof(TLOGBINHEAD)];
	char* pszMsg;
	int iMsgLen;

	
	if( !a_pstCatInst )
		return -1;
	
	if (!evt_is_enabled(a_pstCatInst->pstCat, a_pstEvt->evt_priority))
	{
		return 0;
	}
	
	szBuff[0]	=	'\0';
	
	a_pstEvt->evt_category = a_pstCatInst->pstCat->szName;
	
	gettimeofday(&a_pstEvt->evt_timestamp, NULL);
	
	a_pstCatInst->iSeq++;
	
	if(a_pstEvt->evt_is_msg_binary)
	{
	
		TLOGBIN_INIT_HEAD(&stHead, a_pstCatInst->iSeq, 
				a_pstEvt->evt_id, a_pstEvt->evt_cls, 
				a_pstEvt->evt_type, a_pstEvt->evt_version,
				a_pstEvt->evt_msg_len, &a_pstEvt->evt_timestamp );
	
		if( a_pstEvt->evt_msg_len < TLOGBIN_MAX_DATA )
		{
			a_pstEvt->evt_rendered_msg		=	szBuff;
			a_pstEvt->evt_rendered_msg_len	=	(size_t)tlogbin_make_pkg(szBuff, (int)sizeof(szBuff), &stHead, (char*)a_pstEvt->evt_msg, (int)a_pstEvt->evt_msg_len);
		}
		else
		{
			a_pstEvt->evt_rendered_msg		=	NULL;
			a_pstEvt->evt_rendered_msg_len	=	0;
			a_pstEvt->evt_buffer.buf_maxsize=	sizeof(szBuff);
			a_pstEvt->evt_buffer.buf_data	=	szBuff;
			a_pstEvt->evt_buffer.buf_size	=	(size_t)tlogbin_hton_head(szBuff, (int)sizeof(szBuff), &stHead);
		}
	}
	else
	{
		if( a_pstCatInst->pstCat->iMaxMsgSize<=(int)sizeof(szBuff) ) 
		{
			pszMsg	= szBuff;
			iMsgLen = (int)sizeof(szBuff);
		}
		else 
		{
			iMsgLen = a_pstCatInst->pstCat->iMaxMsgSize;
			pszMsg	= alloca(iMsgLen);	
	
			if( !pszMsg )
			{
				pszMsg	= szBuff;
				iMsgLen = (int)sizeof(szBuff);
			}
		}
	
		a_pstEvt->evt_buffer.buf_maxsize	=	iMsgLen;
		a_pstEvt->evt_buffer.buf_size		=	0;
		a_pstEvt->evt_buffer.buf_data		=	pszMsg;
	
		if( -1==tlog_layout_format(a_pstEvt, a_pstCatInst->pstCat->szFormat) )
		{
			a_pstEvt->evt_rendered_msg		=	a_pstEvt->evt_msg;
			a_pstEvt->evt_rendered_msg_len	=	a_pstEvt->evt_msg_len;
		}
	}
	
	for (pstCatInst = a_pstCatInst; pstCatInst; pstCatInst = pstCatInst->pstParent) 
	{
		iRet = tlog_category_append(pstCatInst, a_pstEvt);
	
		if (!pstCatInst->pstCat->iLevelDispatch)
		{
			break;	
		}
	}
	
	return iRet;
}

int tlog_category_log(TLOGCATEGORYINST *a_pstCatInst, TLOGEVENT *a_pstEvt)
{
	TLOGCATEGORYINST *pstCatInst;
	int iRet=0;

	for (pstCatInst = a_pstCatInst; pstCatInst; pstCatInst = pstCatInst->pstForward) 
	{
		iRet = tlog_category_log_in(pstCatInst, a_pstEvt);
	}

	return iRet;
}


