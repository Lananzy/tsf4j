/*
**  @file $RCSfile: tmempool.c,v $
**  general description of this module
**  $Id: tmempool.c,v 1.4 2009/01/14 08:16:46 sean Exp $
**  @author $Author: sean $
**  @date $Date: 2009/01/14 08:16:46 $
**  @version $Revision: 1.4 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "comm/tmempool.h"

int tmempool_destroy(TMEMPOOL** ppstPool)
{
	if( ppstPool && *ppstPool  )
	{
		if( (*ppstPool)->iIsCalloc )
		{
			free(*ppstPool);
		}

		*ppstPool	=	NULL;

		return 0;
	}

	return -1;
}

void tmempool_init_head_i(TMEMPOOL* pstPool, int iMax, int iUnit, int iSize)
{
	pstPool->iMagic	=	TMEMPOOL_MAGIC;
	pstPool->iBuild	=	TMEMPOOL_BUILD;

	pstPool->iMax	=	iMax;
	pstPool->iUnit	=	iUnit;

	pstPool->iRealUnit	=	TMEMPOOL_REAL_UNIT(iUnit);
	pstPool->iSize	=	TMEMPOOL_CALC(iMax, iUnit);

	pstPool->iRealSize	=	iSize;

	pstPool->iUsed	=	0;
	pstPool->iLastIdx	=	0;

	pstPool->iStart	=	0;
	pstPool->iEnd	=	0;

	pstPool->iFreeHead	=	-1;
	pstPool->iFreeTail	=	-1;

	pstPool->iMethod	=	TMEMPOOL_FIFO;

	return;
}

void tmempool_init_body_i(TMEMPOOL* pstPool)
{
	TMEMBLOCK* pstTail;
	TMEMBLOCK* pstBlock;
	int i;

	if( pstPool->iMax<=0 )
		return;

	pstPool->iFreeHead	=	0;
	pstPool->iFreeTail	=	0;

	pstTail	=	TMEMPOOL_GET_PTR(pstPool, pstPool->iFreeTail);
	pstTail->iNext	=	-1;

	for(i=1; i<pstPool->iMax; i++)
	{
		pstTail	=	TMEMPOOL_GET_PTR(pstPool, pstPool->iFreeTail);
		pstBlock	=	TMEMPOOL_GET_PTR(pstPool, i);

		pstBlock->iNext	=	-1;
		pstTail->iNext	=	i;

		pstPool->iFreeTail	=	i;
	}

	return;
}

int tmempool_check_head_i(TMEMPOOL* pstPool, int iMax, int iUnit, int iSize)
{
	if( !pstPool->iInited || TMEMPOOL_MAGIC!=pstPool->iMagic || TMEMPOOL_BUILD!=pstPool->iBuild ) 
		return -1;

	if( iSize<(int)TMEMPOOL_CALC(iMax, iUnit) )
		return -1;

	if(	pstPool->iMax!=iMax || pstPool->iUnit!=iUnit || iSize<pstPool->iSize )
		return -1;

	if( pstPool->iStart!=pstPool->iEnd )
		return tmempool_fix(pstPool);
	else
		return 0;
}


int tmempool_init(TMEMPOOL** ppstPool, int iMax, int iUnit, void* pvBase, int iSize)
{
	TMEMPOOL* pstHead;
	int iNeed;

	assert( ppstPool && pvBase );

	pstHead	=	(TMEMPOOL*)pvBase;

	iNeed	=	TMEMPOOL_CALC(iMax, iUnit);

	if( iNeed/iUnit<iMax || iNeed>iSize )
		return -1;

	tmempool_init_head_i(pstHead, iMax, iUnit, iSize);
	tmempool_init_body_i(pstHead);

	pstHead->iInited	=	1;

	*ppstPool			=	pstHead;

	return 0;
}

int tmempool_new(TMEMPOOL** ppstPool, int iMax, int iUnit)
{
	unsigned int iSize;

	iSize		=	TMEMPOOL_CALC(iMax, iUnit);

	iSize		=	TMEMPOOL_ROUND_POOL(iSize);

	if( !ppstPool )
		return iSize;

	*ppstPool	=	(TMEMPOOL*) calloc(1, iSize);

	if( !*ppstPool )
		return -1;

	tmempool_init_head_i(*ppstPool, iMax, iUnit, iSize);
	tmempool_init_body_i(*ppstPool);

	(*ppstPool)->iIsCalloc	=	1;
	(*ppstPool)->iInited	=	1;

	return 0;
}

int tmempool_attach(TMEMPOOL** ppstPool, int iMax, int iUnit, void* pvBase, int iSize)
{
	TMEMPOOL* pstHead;

	assert( ppstPool && pvBase );

	pstHead	=	(TMEMPOOL*) pvBase;

	if( tmempool_check_head_i(pstHead, iMax, iUnit, iSize)<0 )
		return -1;

	*ppstPool	=	pstHead;

	return 0;
}

int tmempool_set_method(TMEMPOOL* pstPool, int iMethod)
{
	int iOld;

	iOld	=	pstPool->iMethod;

	pstPool->iMethod	=	iMethod;

	return iOld;
}

void* tmempool_get(TMEMPOOL* pstPool, int iIdx)
{
	TMEMBLOCK* pstBlock;

	pstBlock	=	TMEMPOOL_GET_PTR(pstPool, iIdx);

	if( !pstBlock->fValid || pstBlock->iIdx!=iIdx )
		return NULL;
	else
		return pstBlock->szData;
}

int tmempool_alloc(TMEMPOOL* pstPool)
{
	int iPos;
	int iIdx;
	TMEMBLOCK* pstBlock;

	if( pstPool->iUsed>=pstPool->iMax || -1==pstPool->iFreeHead )
		return -1;

	if( pstPool->iStart!=pstPool->iEnd )
		return -1;

	pstPool->iStart++;

	iPos		=	pstPool->iFreeHead;

	pstBlock	=	TMEMPOOL_GET_PTR(pstPool, iPos);

	if( 0==pstBlock->iIdx )
	{
		iIdx	=	iPos;
	}
	else
	{
		iIdx	=	(pstBlock->iIdx/pstPool->iMax)*pstPool->iMax + pstPool->iMax + iPos;

		if( iIdx<0 )
			iIdx	=	iPos;
	}

	pstPool->iFreeHead	=	pstBlock->iNext;

	if( -1==pstPool->iFreeHead )
		pstPool->iFreeTail	=	-1;

	pstBlock->fValid	=	1;
	pstPool->iUsed++;

	pstBlock->iIdx		=	iIdx;
	pstPool->iLastIdx	=	iIdx;

	pstPool->iEnd	=	pstPool->iStart;

	return iIdx;
}

int tmempool_free(TMEMPOOL* pstPool, int iIdx)
{
	int iPos;
	TMEMBLOCK* pstBlock;
	TMEMBLOCK* pstTail;

	if( pstPool->iMax<=0 )
		return -1;

	pstBlock	=	TMEMPOOL_GET_PTR(pstPool, iIdx);

	if( !pstBlock->fValid  )
		return -1;

	if( pstBlock->iIdx!=iIdx )
		return -1;

	if( pstPool->iStart!=pstPool->iEnd )
		return -1;


	pstPool->iStart++;

	iPos		=	iIdx % pstPool->iMax;

	pstBlock->fValid	=	0;

	pstPool->iUsed--;

	switch( pstPool->iMethod )
	{
	case TMEMPOOL_LIFO:

		pstBlock->iNext		=	pstPool->iFreeHead;

		if( -1==pstPool->iFreeHead )
		{
			pstPool->iFreeTail	=	iPos;
		}

		pstPool->iFreeHead	=	iPos;

		break;

	default:

		pstBlock->iNext		=	-1;

		if( -1==pstPool->iFreeTail )
		{
			pstPool->iFreeHead	=	iPos;
		}
		else
		{
			pstTail				=	TMEMPOOL_GET_PTR(pstPool, pstPool->iFreeTail);
			pstTail->iNext		=	iPos;
		}

		pstPool->iFreeTail	=	iPos;

		break;
	}

	pstPool->iEnd		=	pstPool->iStart;

	return 0;
}

int tmempool_fix(TMEMPOOL* pstPool)
{
	TMEMBLOCK* pstTail;
	TMEMBLOCK* pstBlock;
	int i;

	if( pstPool->iStart==pstPool->iEnd )
		return 0;

	pstPool->iFreeTail	=	-1;
	pstPool->iFreeHead	=	-1;

	pstPool->iUsed		=	0;

	if( pstPool->iMax<=0 )
	{
		pstPool->iEnd	=	pstPool->iStart;
		return 0;
	}

	for(i=0; i<pstPool->iMax; i++)
	{
		pstBlock	=	TMEMPOOL_GET_PTR(pstPool, i);

		if( pstBlock->fValid )
		{
			pstPool->iUsed++;
			continue;
		}

		pstBlock->iNext		=	-1;

		if( -1==pstPool->iFreeTail )
		{
			pstPool->iFreeHead	=	i;
			pstPool->iFreeTail	=	i;
		}
		else
		{
			pstTail	=	TMEMPOOL_GET_PTR(pstPool, pstPool->iFreeTail);

			pstTail->iNext	=	i;
			pstPool->iFreeTail	=	i;
		}
	}

	return 0;
}

int tmempool_find_used_first(TMEMPOOL* pstPool, int* piPos)
{
	int i;
	TMEMBLOCK* pstBlock;

	for(i=0; i<pstPool->iMax; i++)
	{
		pstBlock	=	TMEMPOOL_GET_PTR(pstPool, i);

		if( pstBlock->fValid )
		{
			*piPos	=	i;
			return 0;
		}
	}

	return -1;
}

int tmempool_find_used_next(TMEMPOOL* pstPool, int* piPos)
{
	int i;
	int iIdx;
	int iFind;
	TMEMBLOCK* pstBlock;

	if( -1==*piPos )
		return -1;

	pstBlock=	TMEMPOOL_GET_PTR(pstPool, *piPos);

	iIdx	=	pstBlock->iIdx;

	iFind	=	-1;

	for(i=*piPos+1; i<pstPool->iMax; i++)
	{
		pstBlock	=	TMEMPOOL_GET_PTR(pstPool, i);

		if( pstBlock->fValid )
		{
			iFind	=	i;
			break;
		}
	}

	*piPos	=	iFind;

	return iIdx;
}

