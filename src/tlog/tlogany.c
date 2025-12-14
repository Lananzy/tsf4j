/*
**  @file $RCSfile: tlogany.c,v $
**  general description of this module
**  $Id: tlogany.c,v 1.3 2008/11/12 09:52:19 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/11/12 09:52:19 $
**  @version $Revision: 1.3 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include "tlog/tlogfilter.h"
#include "tlog/tlogfile.h"
#include "tlog/tlognet.h"
#include "tlog/tlogvec.h"
#include "tlog/tlogany.h"


int tlogany_init(TLOGANY* a_pstLogAny, TLOGDEVANY* a_pstDev)
{ 
	int iRet;

	assert(a_pstLogAny && a_pstDev);

	a_pstLogAny->pstDev		=	a_pstDev;

	a_pstLogAny->stInst.iType	=	a_pstDev->iType;

	switch(a_pstDev->iType)
	{
	case TLOG_DEV_NO:
		memset(&a_pstLogAny->stInst.stAny, 0, sizeof(a_pstLogAny->stInst.stAny));
		iRet = 0;
		break;
	case TLOG_DEV_FILE:
		iRet	=	tlogfile_init(&a_pstLogAny->stInst.stAny.stFile, &a_pstDev->stDevice.stFile);
		break;
	case TLOG_DEV_NET:
		iRet	=	tlognet_init(&a_pstLogAny->stInst.stAny.stNet, &a_pstDev->stDevice.stNet);
		break;
	case TLOG_DEV_VEC:
		iRet	=	tlogvec_init(&a_pstLogAny->stInst.stAny.stVec, &a_pstDev->stDevice.stVec);
		break;
	default:
		iRet	=	-1;
		break;
	}

	if( iRet<0 )
	{
		tlogany_fini(a_pstLogAny);
	}

	return iRet;
}


int tlogany_fini(TLOGANY* a_pstLogAny)
{

	assert( a_pstLogAny );

	switch( a_pstLogAny->stInst.iType )
	{
	case TLOG_DEV_FILE:
		tlogfile_fini(&a_pstLogAny->stInst.stAny.stFile);
		break;
	case TLOG_DEV_NET:
		tlognet_fini(&a_pstLogAny->stInst.stAny.stNet);
		break;
	case TLOG_DEV_VEC:
		tlogvec_fini(&a_pstLogAny->stInst.stAny.stVec);
		break;
	default:
		break;
	}

	return 0;
}


int tlogany_writev(TLOGANY* a_pstLogAny, int a_iID, int a_iCls, const TLOGIOVEC* a_pstIOVec, int a_iCount)
{
	int iRet=0;

	switch( a_pstLogAny->stInst.iType )
	{
	case TLOG_DEV_FILE:
		iRet	=	tlogfile_writev(&a_pstLogAny->stInst.stAny.stFile, a_pstIOVec, a_iCount);
		break;
	case TLOG_DEV_NET:
		iRet	=	tlognet_writev(&a_pstLogAny->stInst.stAny.stNet, a_pstIOVec, a_iCount);
		break;
	case TLOG_DEV_VEC:
		iRet	=	tlogvec_writev(&a_pstLogAny->stInst.stAny.stVec, a_iID, a_iCls, a_pstIOVec, a_iCount);
		break;
	default:
		iRet	=	-1;
		break;
	}

	return iRet;
}


int tlogany_write(TLOGANY* a_pstLogAny, int a_iID, int a_iCls, const char* a_pszBuff, int a_iBuff)
{
	TLOGIOVEC stIOVec;

	assert( a_pstLogAny && a_pszBuff && a_iBuff>0 );

	stIOVec.iov_base	=	(void*)a_pszBuff;
	stIOVec.iov_len		=	(size_t)a_iBuff;

	return tlogany_writev(a_pstLogAny, a_iID, a_iCls, &stIOVec, 1);
}


