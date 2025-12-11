/*
**  @file $RCSfile: tlogbin.c,v $
**  general description of this module
**  $Id: tlogbin.c,v 1.2 2008/08/08 02:29:44 steve Exp $
**  @author $Author: steve $
**  @date $Date: 2008/08/08 02:29:44 $
**  @version $Revision: 1.2 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include "tlog/tlogfilter.h"
#include "tlog/tlogfile.h"
#include "tlog/tlognet.h"
#include "tlog/tlogvec.h"
#include "tlog/tlogbin.h"

#define TLOGBIN_CONVERT_BASE(pstDst, pstSrc)	\
	do	\
	{	\
		(pstDst)->chVer	=	(pstSrc)->chVer;	\
		(pstDst)->chMagic	=	(pstSrc)->chMagic;\
		(pstDst)->chCmd	=	(pstSrc)->chCmd;	\
		(pstDst)->chHeadLen	=	(pstSrc)->chHeadLen;	\
		(pstDst)->iType	=	(int) NTOHL((unsigned long)(pstSrc)->iType);	\
		(pstDst)->stTime.iSec	=	(int) NTOHL((unsigned long)(pstSrc)->stTime.iSec);	\
		(pstDst)->stTime.iUsec	=	(int) NTOHL((unsigned long)(pstSrc)->stTime.iUsec);	\
		(pstDst)->iSeq		=	(int) NTOHL((unsigned long)(pstSrc)->iSeq);		\
		(pstDst)->iID		=	(int) NTOHL((unsigned long)(pstSrc)->iID);	\
		(pstDst)->iCls		=	(int) NTOHL((unsigned long)(pstSrc)->iCls);	\
		(pstDst)->iBodyLen	=	(int) NTOHL((unsigned long)(pstSrc)->iBodyLen);	\
		(pstDst)->iBodyVer	=	(int) NTOHL((unsigned long)(pstSrc)->iBodyVer);	\
		(pstDst)->iCheckSum	=	(int) NTOHL((unsigned long)(pstSrc)->iCheckSum);	\
	}	\
	while(0)



int tlogbin_hton_head(char* a_pszBuff, int a_iBuff, const TLOGBINHEAD* a_pstHead)
{
	const TLOGBINBASE* pstSrc;
	TLOGBINBASE* pstDst;

	if( a_iBuff<(int)sizeof(*a_pstHead) )
		return -1;

	pstSrc		=	(const TLOGBINBASE*)a_pstHead;
	pstDst		=	(TLOGBINBASE*)a_pszBuff;

	TLOGBIN_CONVERT_BASE(pstDst, pstSrc);

	return (int)sizeof(*a_pstHead);
}

int tlogbin_ntoh_head(TLOGBINHEAD* a_pstHead, const char* a_pszBuff, int a_iBuff)
{
	const TLOGBINBASE* pstSrc;
	TLOGBINBASE* pstDst;

	if( a_iBuff<(int)sizeof(*a_pstHead) )
		return -1;

	pstSrc		=	(const TLOGBINBASE*)a_pszBuff;
	pstDst		=	(TLOGBINBASE*)a_pstHead;

	TLOGBIN_CONVERT_BASE(pstDst, pstSrc);

	return (int)a_pstHead->chHeadLen;
}


int tlogbin_get_pkgsize(const char* a_pszBuff, int a_iBuff, int* piHead)
{
	TLOGBINBASE* pstBase;

	assert( a_pszBuff );

	if( a_iBuff<(int)sizeof(TLOGBINBASE) )
		return -1;

	pstBase		=	(TLOGBINBASE*)a_pszBuff;

	if( piHead )	*piHead	=	pstBase->chHeadLen;

	return (int)pstBase->chHeadLen + (int) NTOHL((unsigned long)pstBase->iBodyLen);
}

int tlogbin_make_pkg(char* a_pszPkg, int a_iPkg, TLOGBINHEAD* a_pstHead, const char* a_pszBody, int a_iBody)
{
	const TLOGBINBASE* pstSrc;
	TLOGBINBASE* pstDst;

	if( a_iPkg<(int)sizeof(TLOGBINHEAD) + a_iBody )
	{
		errno	=	ERANGE;
		return -1;
	}

	a_pstHead->chHeadLen	=	(char)sizeof(*a_pstHead);
	a_pstHead->iBodyLen		=	a_iBody;

	pstSrc	=	(const TLOGBINBASE*) a_pstHead;
	pstDst	=	(TLOGBINBASE*) a_pszPkg;


	TLOGBIN_CONVERT_BASE(pstDst, pstSrc);

	if( a_iBody>0 )
	{
		if(a_pszBody )
			memcpy(a_pszPkg+pstDst->chHeadLen, a_pszBody, a_iBody);
		else
			return -1;
	}

	return (int)pstDst->chHeadLen + a_iBody;
}


int tlogbin_get_head(TLOGBINHEAD* a_pstHead, const char* a_pszBuff, int a_iBuff, int* a_piPkg)
{
	const TLOGBINBASE* pstSrc;
	TLOGBINBASE* pstDst;

	assert( a_pstHead && a_pszBuff );

	if( a_iBuff<(int)sizeof(TLOGBINBASE) )
		return -1;

	pstSrc	=	(TLOGBINBASE*) a_pszBuff;
	pstDst	=	(TLOGBINBASE*) a_pstHead;

	TLOGBIN_CONVERT_BASE(pstDst, pstSrc);


	if( a_piPkg)	*a_piPkg	=	(int)a_pstHead->chHeadLen + a_pstHead->iBodyLen;

	return (int)a_pstHead->chHeadLen;
}

int tlogbin_filter(TLOGBINHEAD* a_pstHead, TLOGBINFILTER* a_pstFilter, const char* a_pszExt)
{
	int fMatch1;
	int fMatch2;

	assert( a_pstHead && a_pstFilter );

	if( a_pstFilter->iLastSec>0 && (a_pstHead->stTime.iSec<a_pstFilter->iStartTime || a_pstHead->stTime.iSec - a_pstFilter->iStartTime>=a_pstFilter->iLastSec ) )
		return -1;

	TLOGFILTER_MATCH_INT(fMatch1, &a_pstFilter->stID, a_pstHead->iID);
	TLOGFILTER_MATCH_INT(fMatch2, &a_pstFilter->stCls, a_pstHead->iCls);

	if( !fMatch1 || !fMatch2 )
		return -1;

	if( !a_pszExt )
		return 0;

	/* do extra filter. */

	return 0;
}

