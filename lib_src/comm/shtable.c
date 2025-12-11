#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include "comm/shtable.h"

unsigned int sht_get_code(const char* szKey)
{
	unsigned int uCode=0;

	while( szKey[0] )
	{
		uCode += (uCode<<5) + ((unsigned char)szKey[0]);
		szKey++;
	}

	return uCode;
}

int sht_make_free_chain_i(LPSHTABLE pstTab)
{
	LPSHITEM pstItem;
	LPSHITEM pstNext;
	int i;
	int n;

	pstTab->iFreeHead	=	-1;
	n	=	0;

	for(i=pstTab->iMax-1; i>=0; i--)
	{
		pstItem	=	SHT_GET_ITEM(pstTab, i);

		if( pstItem->fValid )
			continue;

		n++;

		pstItem->iNext	=	pstTab->iFreeHead;
		pstItem->iPrev	=	-1;

		if( pstTab->iFreeHead<0 )
		{
			pstTab->iFreeHead	=	i;
		}
		else
		{
			pstNext	=	SHT_GET_ITEM(pstTab, pstTab->iFreeHead);
			pstNext->iPrev	=	i;
			pstTab->iFreeHead	=	i;
		}
	}

	return n;
}

int sht_alloc_node_i(LPSHTABLE pstTab)
{
	int iNode;
	LPSHITEM pstItem;
	LPSHITEM pstHead;

	if( pstTab->iFreeHead<0 || pstTab->iFreeHead>=pstTab->iMax )
		return -1;

	iNode	=	pstTab->iFreeHead;
	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	pstTab->iFreeHead	=	pstItem->iNext;

	pstItem->fValid	=	1;
	pstItem->uCode	=	0;
	pstItem->iNext	=	-1;
	pstItem->iPrev	=	-1;

	if( pstTab->iFreeHead>=0 && pstTab->iFreeHead<pstTab->iMax )
	{
		pstHead	=	SHT_GET_ITEM(pstTab, pstTab->iFreeHead);
		pstHead->iPrev	=	-1;
	}

	pstTab->iItem++;

	return iNode;
}

int sht_free_node_i(LPSHTABLE pstTab, int iNode)
{
	LPSHITEM pstItem;
	LPSHITEM pstHead;

	if( iNode<0 || iNode>=pstTab->iMax )
		return -1;

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	pstItem->fValid	=	0;
	pstItem->uCode	=	0;
	pstItem->iPrev	=	-1;
	pstItem->iNext	=	-1;

	if( pstTab->iFreeHead>=0 && pstTab->iFreeHead<pstTab->iMax )
	{
		pstHead	=	SHT_GET_ITEM(pstTab, pstTab->iFreeHead);
		pstHead->iPrev	=	iNode;
	}

	pstItem->iNext		=	pstTab->iFreeHead;
	pstTab->iFreeHead	=	iNode;

	pstTab->iItem--;

	return iNode;
}

void sht_insert_i(LPSHTABLE pstTab, int iNode, unsigned int uCode)
{
	int iBucket;
	LPSHBUCKET pstBucket;
	LPSHITEM pstItem;
	LPSHITEM pstHead;

	iBucket	=	(int) (uCode % (unsigned int)pstTab->iBucket);

	pstBucket	=	SHT_GET_BUCKET(pstTab, iBucket);

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	pstItem->uCode	=	uCode;

	if( pstBucket->iCount>0 )
		pstItem->iNext	=	pstBucket->iHead;
	else
		pstItem->iNext	=	-1;

	pstItem->iPrev	=	-1;

	if( pstBucket->iCount>0 && pstBucket->iHead>=0 && pstBucket->iHead<pstTab->iMax )
	{
		pstHead	=	SHT_GET_ITEM(pstTab, pstBucket->iHead);
		pstHead->iPrev	=	iNode;
	}

	pstBucket->iHead	=	iNode;
	pstBucket->iCount++;
}

void sht_remove_i(LPSHTABLE pstTab, int iNode, unsigned int uCode)
{
	int iBucket;
	LPSHBUCKET pstBucket;
	LPSHITEM pstItem;
	LPSHITEM pstPrev;
	LPSHITEM pstNext;

	iBucket	=	(int) (uCode % (unsigned int)pstTab->iBucket);

	pstBucket	=	SHT_GET_BUCKET(pstTab, iBucket);

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	if( pstItem->iPrev>=0 && pstItem->iPrev<pstTab->iMax )
	{
		pstPrev	=	SHT_GET_ITEM(pstTab, pstItem->iPrev);
		pstPrev->iNext	=	pstItem->iNext;
	}

	if( pstItem->iNext>=0 && pstItem->iNext<pstTab->iMax )
	{
		pstNext	=	SHT_GET_ITEM(pstTab, pstItem->iNext);
		pstNext->iPrev	=	pstItem->iPrev;
	}

	if( pstBucket->iHead==iNode )
		pstBucket->iHead	=	pstItem->iNext;

	pstBucket->iCount--;
}

int sht_find_i(LPSHTABLE pstTab, const void* pvData, SHT_CMP pfnCmp, unsigned int uCode)
{
	int iBucket;
	LPSHBUCKET pstBucket;
	LPSHITEM pstItem;
	int iNode;
	int n;

	iBucket	=	(int) (uCode % (unsigned int)pstTab->iBucket);
	pstBucket	=	SHT_GET_BUCKET(pstTab, iBucket);

	if( pstBucket->iCount<=0 )
		return -1;

	iNode	=	pstBucket->iHead;
	n		=	0;

	while(iNode>=0 && iNode<pstTab->iMax && n<pstBucket->iCount )
	{
		pstItem	=	SHT_GET_ITEM(pstTab, iNode);

		if( pstItem->uCode==uCode && !pfnCmp(pstItem->szData, pvData) )
			return iNode;

		iNode	=	pstItem->iNext;
		n++;
	}

	return -1;
}

LPSHTABLE sht_init(void* pvBuff, int iBuff, int iBucket, int iMax, int iUnit)
{
	LPSHTABLE pstTab;
	LPSHITEM pstItem;
	int i;

	if( iBuff<(int)SHT_HEADSIZE() || iBucket<0 || iMax<0 || iUnit<=0 )
		return NULL;

	memset(pvBuff, 0, sizeof(SHTABLE));

	pstTab	=	(LPSHTABLE) pvBuff;

	pstTab->cbSize		=	sizeof(SHTABLE);
	pstTab->uFlags		=	0;

	pstTab->iVersion	=	SHT_VERSION;
	pstTab->iBuff		=	iBuff;

	pstTab->iBucket		=	iBucket;
	pstTab->iMax		=	iMax;
	pstTab->iItem		=	0;
	pstTab->iHeadSize	=	SHT_HEADSIZE();

	pstTab->iFreeHead	=	-1;

	if( (iBuff - pstTab->iHeadSize)/(int)sizeof(SHBUCKET) < iBucket )
		return NULL;

	pstTab->iBucketOff	=	pstTab->iHeadSize;
	pstTab->iBucketSize	=	sizeof(SHBUCKET)*iBucket;

	pstTab->iDataOff		=	pstTab->iBucketOff + pstTab->iBucketSize;
	pstTab->iDataUnitMin	=	iUnit;
	pstTab->iDataUnitMax	=	SHT_DATAUNIT(iUnit);

	if( (iBuff - pstTab->iDataOff)/pstTab->iDataUnitMax < iMax )
		return NULL;

	pstTab->iDataSize		=	pstTab->iDataUnitMax*iMax;

	//2008.06.10 by kent, init item valid flag
	for (i = 0; i<pstTab->iMax; i++)
	{
		pstItem	=	SHT_GET_ITEM(pstTab, i);
		pstItem->fValid = 0;
	}
	//by kent end

	//2008.09.25 by kent, init bucket 
	memset(((char *)pstTab) + pstTab->iBucketOff, 0, pstTab->iBucketSize);
	//by kent end

	sht_make_free_chain_i(pstTab);

	return pstTab;
}

int sht_is_empty(LPSHTABLE pstTab)
{
	if( pstTab->iItem<=0 )
		return 1;
	else
		return 0;
}

int sht_is_full(LPSHTABLE pstTab)
{
	if( pstTab->iItem>=pstTab->iMax )
		return 1;
	else
		return 0;
}

LPSHTABLE sht_create(int iBucket, int iMax, int iUnit, int* piBuff)
{
	LPSHTABLE pstTab;
	int iAlloc;

	iAlloc	=	SHT_SIZE(iBucket, iMax, iUnit);
	if( iAlloc<=(int)SHT_HEADSIZE() )
		return NULL;

	pstTab	=	(LPSHTABLE) calloc(1, iAlloc);

	if( !pstTab )
		return NULL;

	if( piBuff )
		*piBuff	=	iAlloc;

	if( NULL==sht_init(pstTab, iAlloc, iBucket, iMax, iUnit) )
	{
		free(pstTab);
		return NULL;
	}

	pstTab->uFlags	=	SHTF_NEEDFREE;

	return pstTab;
}

int sht_check(void* pvBuff, int iBuff, int iBucket, int iMax, int iUnit)
{
	LPSHTABLE pstTab;
	int iSize;

	pstTab	=	(LPSHTABLE) pvBuff;

	if( iBuff<(int)SHT_HEADSIZE() || iBucket<=0 || iMax<=0 || iUnit<=0 )
		return -1;

	iSize	=	SHT_SIZE(iBucket, iMax, iUnit);
	if( iBuff<iSize )
		return -1;

	if( pstTab->iBuff!=iBuff || pstTab->iVersion!=SHT_VERSION ||
		pstTab->cbSize!=sizeof(SHTABLE) || pstTab->iBucket!=iBucket ||
		pstTab->iMax!=iMax || pstTab->iItem>pstTab->iMax ||
		pstTab->iHeadSize!=(int)SHT_HEADSIZE() )
		return -1;

	if( pstTab->iHeadSize>pstTab->iBucketOff || 
		pstTab->iBucketSize/(int)sizeof(SHBUCKET)!=iBucket ||
		iBuff - pstTab->iBucketOff<pstTab->iBucketSize ||
		pstTab->iBucketOff+pstTab->iBucketSize>pstTab->iDataOff || 
		pstTab->iDataUnitMin!=iUnit || 
		pstTab->iDataUnitMax!=(int)SHT_DATAUNIT(iUnit) || 
		pstTab->iDataSize/pstTab->iDataUnitMax!=iMax ||
		iBuff - pstTab->iDataOff<pstTab->iDataSize )
		return -1;

	return 0;
}

LPSHTABLE sht_attach(void* pvBuff, int iBuff, int iBucket, int iMax, int iUnit)
{
	if( sht_check(pvBuff, iBuff, iBucket, iMax, iUnit)<0 )
		return NULL;
	else
		return (LPSHTABLE)pvBuff;
}

void* sht_find(LPSHTABLE pstTab, const void* pvData, SHT_CMP pfnCmp, SHT_HASHCODE pfnHashCode)
{
	unsigned int uCode;
	int iNode;
	LPSHITEM pstItem;

	uCode	=	pfnHashCode(pvData);
	iNode	=	sht_find_i(pstTab, pvData, pfnCmp, uCode);

	if( iNode<0 )
		return NULL;

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	return pstItem->szData;
}

void* sht_insert_unique(LPSHTABLE pstTab, const void* pvData, SHT_CMP pfnCmp, SHT_HASHCODE pfnHashCode)
{
	if( sht_find(pstTab, pvData, pfnCmp, pfnHashCode) )
		return NULL;

	return sht_insert_multi(pstTab, pvData, pfnHashCode);
}

void* sht_insert_multi(LPSHTABLE pstTab, const void* pvData, SHT_HASHCODE pfnHashCode)
{
	unsigned int uCode;
	int iNode;
	LPSHITEM pstItem;

	iNode	=	sht_alloc_node_i(pstTab);

	if( iNode<0 )
		return NULL;

	uCode	=	pfnHashCode(pvData);

	sht_insert_i(pstTab, iNode, uCode);

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	return pstItem->szData;
}

void *sht_insert_force(LPSHTABLE pstTab, unsigned int uCode)
{
	int iNode;
	LPSHITEM pstItem;

	iNode	=	sht_alloc_node_i(pstTab);

	if( iNode<0 )
		return NULL;

	sht_insert_i(pstTab, iNode, uCode);

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	return pstItem->szData;
}

void* sht_remove(LPSHTABLE pstTab, const void* pvData, SHT_CMP pfnCmp, SHT_HASHCODE pfnHashCode)
{
	unsigned int uCode;
	int iNode;
	LPSHITEM pstItem;

	uCode	=	pfnHashCode(pvData);

	iNode	=	sht_find_i(pstTab, pvData, pfnCmp, uCode);
	if( iNode<0 )
		return NULL;

	sht_remove_i(pstTab, iNode, uCode);
	sht_free_node_i(pstTab, iNode);

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	return pstItem->szData;
}

void* sht_remove_by_pos(LPSHTABLE pstTab, int iPos)
{
	LPSHITEM pstItem;

	if( iPos<0 || iPos>=pstTab->iMax )
		return NULL;

	pstItem	=	SHT_GET_ITEM(pstTab, iPos);

	sht_remove_i(pstTab, iPos, pstItem->uCode);
	sht_free_node_i(pstTab, iPos);

	return pstItem->szData;
}

void* sht_pos(LPSHTABLE pstTab, int iPos, int* pfValid)
{
	LPSHITEM pstItem;

	if( iPos<0 || iPos>=pstTab->iMax )
		return NULL;

	pstItem	=	SHT_GET_ITEM(pstTab, iPos);

	if( pfValid )
		*pfValid	=	pstItem->fValid;

	return pstItem->szData;
}

int sht_destroy(LPSHTABLE* ppstTab)
{
	if( !ppstTab )
		return 0;

	if( !*ppstTab )
		return 0;

	if( (*ppstTab)->uFlags & SHTF_NEEDFREE )
	{
		free(*ppstTab);
		*ppstTab	=	NULL;
	}

	return 0;
}


int sht_rebuild(LPSHTABLE pstTab)
{
	LPSHITEM pstItem;
	LPSHBUCKET pstBucket;
	int i;

	sht_make_free_chain_i(pstTab);

	for(i=0; i<pstTab->iBucket; i++)
	{
		pstBucket	=	SHT_GET_BUCKET(pstTab, i);
		pstBucket->iCount	=	0;
		pstBucket->iHead	=	-1;
	}

	pstTab->iItem	=	0;

	for(i=0; i<pstTab->iMax; i++)
	{
		pstItem	=	SHT_GET_ITEM(pstTab, i);

		if( !pstItem->fValid )
			continue;

		sht_insert_i(pstTab, i, pstItem->uCode);

		pstTab->iItem++;
	}

	return 0;
}


int sht_dump_all(LPSHTABLE pstTab, FILE* fp, SHT_PRINT pfnPrint)
{
	int i;
	void* pvData;
	LPSHITEM pstItem;

	fprintf(fp, "--------------Dump hash table(ALL) start----------------\n");
	fprintf(fp, "Bucket:%d, Buff:%d\n", pstTab->iBucket, pstTab->iBuff);
	fprintf(fp,"Max Items:%d Total Items:%d\n", pstTab->iMax, pstTab->iItem);

	for(i=0; i<pstTab->iMax; i++)
	{
		pvData	=	sht_pos(pstTab, i, NULL);

		pstItem	=	SHT_DATA2ITEM(pvData);

		if( pstItem->fValid )
		{
			fprintf(fp, "\t(VALID) Item pos=%d code=%08x ", i, pstItem->uCode);
			pfnPrint(fp, pvData);
			fprintf(fp, "\n");
		}
		else
		{
			fprintf(fp, "\t(FREE) Item pos=%d code=%08x \n", i, pstItem->uCode);
		}
	}

	fprintf(fp, "--------------Dump hash table(ALL) end-------------------\n");

	return 0;
}

int sht_dump_valid(LPSHTABLE pstTab, FILE* fp, SHT_PRINT pfnPrint)
{
	int i;
	int n;
	LPSHBUCKET pstBucket;
	LPSHITEM pstItem;

	fprintf(fp, "--------------Dump hash table(VALID) start---------------\n");
	fprintf(fp, "Bucket:%d, Buff:%d\n", pstTab->iBucket, pstTab->iBuff);
	fprintf(fp,"Max Items:%d Total Items:%d\n", pstTab->iMax, pstTab->iItem);

	for(i=0; i<pstTab->iBucket; i++)
	{
		pstBucket	=	SHT_GET_BUCKET(pstTab, i);

		//fprintf(fp, "Bucket %4d Items:%d\n", i, pstBucket->iCount );

		if( pstBucket->iCount<=0 )
			continue;

		if( pstBucket->iHead<0 || pstBucket->iHead>pstTab->iMax )
		{
			fprintf(fp, "\t[ERROR] Head Pos=%d\n", pstBucket->iHead);
			continue; 
		}

		pstItem	=	SHT_GET_ITEM(pstTab, pstBucket->iHead);
		n		=	0;

		do
		{
			fprintf(fp, "\t(VALID) Item pos=%d code=%08x ", i, pstItem->uCode);
			pfnPrint(fp, pstItem->szData);
			fprintf(fp, "\n");

			if( pstItem->iNext<0 )
				break;
            
			pstItem = SHT_GET_ITEM(pstTab, pstItem->iNext);
			n++;
		}
		while( n<pstBucket->iCount );
	}

	fprintf(fp, "--------------Dump hash table(VALID) end-----------------\n");

	return 0;
}

void* sht_remove_by_addr(LPSHTABLE pstTab, LPSHITEM pstItem)
{
	LPSHITEM pstFirstItem;
	int iPos;

	if (NULL == pstItem)
	{
		return NULL;
	}

	pstFirstItem = SHT_GET_ITEM(pstTab, 0);
	iPos = pstItem - pstFirstItem;
	return sht_remove_by_pos(pstTab, iPos);
}


