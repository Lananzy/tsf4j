/*
**  @file $RCSfile: tlogvec.c,v $
**  general description of this module
**  $Id: tlogvec.c,v 1.3 2009/03/27 06:17:02 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009/03/27 06:17:02 $
**  @version $Revision: 1.3 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include "tlog/tlogfilter.h"
#include "tlog/tlogfile.h"
#include "tlog/tlognet.h"
#include "tlog/tlogvec.h"


int tlogvec_init(TLOGVEC* a_pstLogVec, TLOGDEVVEC* a_pstDev)
{ 
	TLOGELEMENT* pstLogElement;
	TLOGDEVELEMENT* pstDevElement;
	int iRet = 0;
	int i;

	assert(a_pstLogVec && a_pstDev);

	a_pstLogVec->pstDev = a_pstDev;
	a_pstLogVec->stInst.iCount	=	0;

	for(i=0; i<a_pstDev->iCount; i++)
	{
		pstDevElement	=	a_pstDev->astElements + i;
		pstLogElement	=	a_pstLogVec->stInst.astElements + i;

		pstLogElement->iType	=	pstDevElement->iType;
		pstLogElement->iIndex   = i;
		
		iRet	=	0;

		switch(pstLogElement->iType)
		{
		case TLOG_DEV_FILE:
			iRet	=	tlogfile_init(&pstLogElement->stFileOrNet.stFile, &pstDevElement->stDevice.stFile);
			break;
		case TLOG_DEV_NET:
			iRet	=	tlognet_init(&pstLogElement->stFileOrNet.stNet, &pstDevElement->stDevice.stNet);
			break;
		default:
			iRet	=	-1;
			break;
		}

		if( iRet<0 )
			break;

		a_pstLogVec->stInst.iCount++;
	}

	if( a_pstLogVec->stInst.iCount!=a_pstDev->iCount )
	{
		tlogvec_fini(a_pstLogVec);

		return iRet;
	}
	else
	{
		return 0;
	}
}


int tlogvec_fini(TLOGVEC* a_pstLogVec)
{
	TLOGELEMENT* pstLogElement;
	int i;

	assert( a_pstLogVec );

	for(i=0; i<a_pstLogVec->stInst.iCount; i++)
	{
		pstLogElement	=	a_pstLogVec->stInst.astElements + i;

		switch( pstLogElement->iType )
		{
		case TLOG_DEV_FILE:
			tlogfile_fini(&pstLogElement->stFileOrNet.stFile);
			break;
		case TLOG_DEV_NET:
			tlognet_fini(&pstLogElement->stFileOrNet.stNet);
			break;
		default:
			break;
		}
	}

	a_pstLogVec->stInst.iCount	=	0;

	return 0;
}


TLOGELEMENT* tlognet_find_element_i(TLOGVEC* a_pstLogVec, int* a_piPos, int a_iID, int a_iCls)
{
	int i;
	TLOGDEVELEMENT* pstDevElement;
	TLOGELEMENT* pstLogElement;

	assert( a_pstLogVec && a_piPos );

	if( *a_piPos<0 )
		return NULL;

	for(i=*a_piPos; i<a_pstLogVec->stInst.iCount; i++)
	{
		pstLogElement	=	a_pstLogVec->stInst.astElements + i;
		pstDevElement	=	a_pstLogVec->pstDev->astElements + pstLogElement->iIndex;

		if( 0==tlogfilter_match_vec(&pstDevElement->stFilterVec, a_iID, a_iCls) )
		{
			*a_piPos	=	i + 1;
			return pstLogElement;
		}
	}

	*a_piPos	=	-1;

	return NULL;
}


int tlogvec_write_elementv(TLOGELEMENT* a_pstLogElement, const TLOGIOVEC* a_pstIOVec, int a_iCount)
{
	int iRet=0;

	switch( a_pstLogElement->iType )
	{
	case TLOG_DEV_FILE:
		iRet	=	tlogfile_writev(&a_pstLogElement->stFileOrNet.stFile, a_pstIOVec, a_iCount);
		break;
	case TLOG_DEV_NET:
		iRet	=	tlognet_writev(&a_pstLogElement->stFileOrNet.stNet, a_pstIOVec, a_iCount);
		break;
	default:
		iRet	=	-1;
		break;
	}

	return iRet;
}


int tlogvec_write_element(TLOGELEMENT* a_pstLogElement, const char* a_pszBuff, int a_iBuff)
{
	int iRet=0;

	switch( a_pstLogElement->iType )
	{
	case TLOG_DEV_FILE:
		iRet	=	tlogfile_write(&a_pstLogElement->stFileOrNet.stFile, a_pszBuff, a_iBuff);
		break;
	case TLOG_DEV_NET:
		iRet	=	tlognet_write(&a_pstLogElement->stFileOrNet.stNet, a_pszBuff, a_iBuff);
		break;
	default:
		iRet	=	-1;
		break;
	}

	return iRet;
}


int tlogvec_writev(TLOGVEC* a_pstLogVec, int a_iID, int a_iCls, const TLOGIOVEC* a_pstIOVec, int a_iCount)
{
	TLOGELEMENT* pstLogElement;
	int iPos=0;
	int iCount=0;

	assert( a_pstLogVec && a_pstIOVec );

	while( 1 )
	{
		pstLogElement	=	tlognet_find_element_i(a_pstLogVec, &iPos, a_iID, a_iCls);
		if( !pstLogElement)
			break;

		if( tlogvec_write_elementv(pstLogElement, a_pstIOVec, a_iCount)<0 )
			continue;

		iCount++;
	}

	return iCount;
}


int tlogvec_write(TLOGVEC* a_pstLogVec, int a_iID, int a_iCls, const char* a_pszBuff, int a_iBuff)
{
	TLOGELEMENT* pstLogElement;
	int iPos=0;
	int iCount=0;

	assert( a_pstLogVec && a_pszBuff && a_iBuff>0 );

	while( 1 )
	{
		pstLogElement	=	tlognet_find_element_i(a_pstLogVec, &iPos, a_iID, a_iCls);
		if( !pstLogElement)
			break;

		if( tlogvec_write_element(pstLogElement, a_pszBuff, a_iBuff)<0 )
			continue;

		iCount++;
	}

	return iCount;
}


