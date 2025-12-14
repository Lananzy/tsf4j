/*
**  @file $RCSfile: swaitque.c,v $
**  general description of this module
**  $Id: swaitque.c,v 1.1 2008/07/03 11:18:14 steve Exp $
**  @author $Author: steve $
**  @date $Date: 2008/07/03 11:18:14 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include <stdlib.h>
#include "comm/swaitque.h"

int swq_check(void* pvBuff, int iBuff, int iMax, int iUnit)
{
	LPSWAITQUE pstQue;

	pstQue	=	(LPSWAITQUE) pvBuff;

	if( pstQue->cbSize!=sizeof(SWAITQUE) || SWQ_VERSION!=pstQue->iVersion
		|| SWQF_INITED!=pstQue->uiFlags 
		|| pstQue->iBuff!=iBuff || pstQue->iMax!=iMax || pstQue->iDataUnitMin!=iUnit )
		return -1;

	return 0;
}

LPSWAITQUE swq_create(int iMax, int iUnit, int* piBuff)
{
	LPSWAITQUE pstQue=NULL;
	int iBuff;

	iBuff	=	SWQ_SIZE(iMax, iUnit);

	if( iBuff<=0 )
		return NULL;

	pstQue	=	calloc(1, iBuff);

	if( !pstQue )
		return NULL;

	pstQue->uiFlags |=	SWQF_NEEDFREE;

	swq_init(pstQue, iBuff, iMax, iUnit);

	if( piBuff )
		*piBuff	=	iBuff;

	return pstQue;
}

void swq_invalid_all_i(LPSWAITQUE pstQue)
{
	LPSWQITEM pstItem;
	int i;

	pstQue->iItem	=	0;

	for(i=0; i<pstQue->iMax; i++)
	{
		pstItem	=	SWQ_GET_ITEM(pstQue, i);

		pstItem->iIsValid	=	0;
		pstItem->iPrev		=	-1;
		pstItem->iNext		=	-1;
		pstItem->iID		=	0;
	}
}

void swq_make_freechain_i(LPSWAITQUE pstQue)
{
	LPSWQITEM pstItem;
	LPSWQITEM pstPrev=NULL;
	int i;

	pstQue->iItem	=	0;

	for(i=0; i<pstQue->iMax; i++)
	{
		pstItem	=	SWQ_GET_ITEM(pstQue, i);
		
		if( pstItem->iIsValid )
		{
			pstQue->iItem++;
		}
		else
		{
			pstItem->iNext	=	-1;

			if( NULL==pstPrev )
			{
				pstQue->iFreeHead	=	i;
			}
			else
			{
				pstPrev->iNext	=	i;
			}

			pstPrev	=	pstItem;
		}
	}

}

LPSWAITQUE swq_init(void* pvBuff, int iBuff, int iMax, int iUnit)
{
	LPSWAITQUE pstQue;

	if( iBuff!=(int)SWQ_SIZE(iMax, iUnit) )
		return NULL;

	pstQue	=	(LPSWAITQUE) pvBuff;

	pstQue->iBuff	=	iBuff;
	pstQue->iMax	=	pstQue->iMax;

	pstQue->iItem	=	0;

	swq_invalid_all_i(pstQue);
	swq_make_freechain_i(pstQue);

	pstQue->uiFlags |=	SWQF_INITED;

	return pstQue;
}

LPSWAITQUE swq_attach(void* pvBuff, int iBuff, int iMax, int iUnit)
{
	if( swq_check(pvBuff, iBuff, iMax, iUnit)<0 )
		return NULL;
	else
		return (LPSWAITQUE) pvBuff;
}

void* swq_alloc(LPSWAITQUE pstQue, int iID)
{
	LPSWQITEM pstItem;
	LPSWQITEM pstNext;

	if( -1==pstQue->iFreeHead )
		return NULL;

	pstItem	=	SWQ_GET_ITEM(pstQue, pstQue->iFreeHead);

	pstQue->iFreeHead	=	pstItem->iNext;

	if( -1!=pstQue->iFreeHead )
	{
		pstNext		=	SWQ_GET_ITEM(pstQue, pstQue->iFreeHead);
		pstNext->iPrev	=	-1;
	}

	pstItem->iIsValid	=	1;
	pstItem->iID		=	iID;
	pstQue->iItem++;

	return pstItem->szData;
}

void* swq_remove(LPSWAITQUE pstQue, int iPos)
{
	LPSWQITEM pstItem;
	LPSWQITEM pstPrev;
	LPSWQITEM pstNext;
	LPSWQITEM pstHead;

	if( iPos<0 || iPos>=pstQue->iMax )
		return NULL;

	pstItem	=	SWQ_GET_ITEM(pstQue, iPos);

	if( !pstItem )
		return NULL;

	if( !pstItem->iIsValid )
		return NULL;

	if( -1!=pstItem->iPrev )
	{
		pstPrev			=	SWQ_GET_ITEM(pstQue, pstItem->iPrev);

		pstPrev->iNext	=	pstItem->iNext;
	}

	if( -1!=pstItem->iNext )
	{
		pstNext			=	SWQ_GET_ITEM(pstQue, pstItem->iNext);

		pstNext->iPrev	=	pstItem->iPrev;
	}

	pstItem->iPrev		=	-1;
	pstItem->iNext		=	pstQue->iFreeHead;

	if( -1==pstQue->iFreeHead )
	{
		pstQue->iFreeHead=	iPos;
	}
	else
	{
		pstHead			=	SWQ_GET_ITEM(pstQue, pstQue->iFreeHead);
		pstHead->iPrev	=	iPos;
	}

	pstItem->iIsValid	=	0;
	pstQue->iItem--;

	return pstItem->szData;
}

void* swq_find(LPSWAITQUE pstQue, int* piPos, int iID)
{
	LPSWQITEM pstItem;
	int iNext;

	if( NULL==piPos || -1==*piPos )
	{
		iNext	=	pstQue->iQueHead;
	}
	else
	{
		iNext	=	*piPos;
		if( iNext<0 || iNext>=pstQue->iMax )
			return NULL;

		pstItem	=	SWQ_GET_ITEM(pstQue, iNext);

		iNext	=	pstItem->iNext;
	}	

	while( -1!=iNext )
	{
		pstItem	=	SWQ_GET_ITEM(pstQue, iNext);

		if( iID==pstItem->iID )
			return pstItem->szData;

		iNext	=	pstItem->iNext;
	}

	return NULL;
}

void* swq_pos(LPSWAITQUE pstQue, int iPos, int* piIsValid)
{
	LPSWQITEM pstItem;

	pstItem	=	SWQ_GET_ITEM(pstQue, iPos);

	if( piIsValid )
		*piIsValid	=	pstItem->iIsValid;

	return pstItem->szData;
}

int swq_destroy(LPSWAITQUE* ppstQue)
{
	if( !ppstQue )
		return -1;

	if( (*ppstQue)->uiFlags & SWQF_NEEDFREE )
	{
		free(*ppstQue);
		*ppstQue	=	NULL;
	}

	return 0;
}

int swq_rebuild(LPSWAITQUE pstQue)
{
	pstQue->iItem	=	0;

	swq_make_freechain_i(pstQue);

	return 0;
}

int swq_is_empty(LPSWAITQUE pstQue)
{
	if( 0==pstQue->iItem )
		return 1;
	else
		return 0;
}

int swq_is_full(LPSWAITQUE pstQue)
{
	if( pstQue->iMax==pstQue->iItem )
		return 1;
	else
		return 0;
}

int swq_dump_all(LPSWAITQUE pstQue, FILE* fp, SWQ_PRINT pfnPrint)
{
	LPSWQITEM pstItem;
	int i;
	int iTotal=0;

	fprintf(fp, "--------------Dump swaitque (ALL) start----------------\n");
	fprintf(fp, "Unit:%d, Buff:%d\n", pstQue->iDataUnitMin, pstQue->iBuff);
	fprintf(fp,"Max Items:%d Total Items:%d\n", pstQue->iMax, pstQue->iItem);

	for(i=0; i<pstQue->iMax; i++)
	{
		pstItem	=	SWQ_GET_ITEM(pstQue, i);

		if( pstItem->iIsValid )
		{
			iTotal++;
			fprintf(fp, "\t(VALID) Item pos=%d id=%08x ", i, pstItem->iID);
			pfnPrint(fp, pstItem->szData);
			fprintf(fp, "\n");
		}
		else
		{
			fprintf(fp, "\t(FREE) Item pos=%d code=%08x \n", i, pstItem->iID);
		}
	}

	fprintf(fp, "\t Total valid=%d.\n", iTotal);
	fprintf(fp, "--------------Dump swaitque(ALL) end--------------------\n");

	return 0;
}

int swq_dump_valid(LPSWAITQUE pstQue, FILE* fp, SWQ_PRINT pfnPrint)
{
	LPSWQITEM pstItem;
	int iNext;
	int iTotal=0;

	fprintf(fp, "--------------Dump swaitque (VALID) start----------------\n");
	fprintf(fp, "Unit:%d, Buff:%d\n", pstQue->iDataUnitMin, pstQue->iBuff);
	fprintf(fp,"Max Items:%d Total Items:%d\n", pstQue->iMax, pstQue->iItem);

	iNext	=	pstQue->iQueHead;

	while( -1!=iNext )
	{
		pstItem	=	SWQ_GET_ITEM(pstQue, iNext);
		iTotal++;
		fprintf(fp, "\t(VALID) Item pos=%d id=%08x ", iNext, pstItem->iID);
		pfnPrint(fp, pstItem->szData);
		fprintf(fp, "\n");

		iNext	=	pstItem->iNext;
	}

	fprintf(fp, "\t Total valid=%d.\n", iTotal);
	fprintf(fp, "--------------Dump swaitque(VALID) end--------------------\n");

	return 0;
}


