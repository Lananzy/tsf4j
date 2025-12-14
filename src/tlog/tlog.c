/*
**  @file $RCSfile: tlog.c,v $
**  general description of this module
**  $Id: tlog.c,v 1.7 2009/03/27 06:17:02 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009/03/27 06:17:02 $
**  @version $Revision: 1.7 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/


#include "pal/pal.h"
#include "tlog/tlogfilter.h"
#include "tlog/tlogfile.h"
#include "tlog/tlognet.h"
#include "tlog/tlogvec.h"
#include "tlog/tlogany.h"
#include "tlog/tlog.h"

extern unsigned char g_szMetalib_tlogdef[];

const void* tlog_get_meta_data()
{
	return (const void*) g_szMetalib_tlogdef;
}

TLOGCTX* tlog_alloc(void)
{
	TLOGCTX* pstCtx;

	pstCtx	=	(TLOGCTX*) calloc(1, sizeof(TLOGCTX));

	return pstCtx;
}

int tlog_free(TLOGCTX** a_ppstCtx)
{
	if( *a_ppstCtx )
	{
		free(*a_ppstCtx);

		*a_ppstCtx	=	NULL;

		return 0;
	}

	return -1;
}

int tlog_resolve_i(TLOGCTX* a_pstCtx)
{
	int i;
	int j;
	TLOGCATEGORYINST* pstCatInst1;
	TLOGCATEGORYINST* pstCatInst2;
	char szParentName[TLOG_NAME_LEN];
	char *pcTmp;

	for(i=0; i<a_pstCtx->stInst.iCount; i++)
	{
		pstCatInst1	=	a_pstCtx->stInst.astCatInsts + i;
		pstCatInst1->pstParent = NULL;
		pstCatInst1->pstForward = NULL;

		pcTmp	=	strrchr(pstCatInst1->szName, TLOG_CHAR_DOT);

		szParentName[0] = 0;
		if (pcTmp)
		{
			size_t iLen;

			iLen = pcTmp - &pstCatInst1->szName[0];

			if (iLen > 0 && iLen < sizeof(szParentName))
			{
				memcpy(szParentName, pstCatInst1->szName, iLen);
				szParentName[iLen] = 0;
			}
		}
		

		for(j=0; j<a_pstCtx->stInst.iCount; j++)
		{
			if( i==j )
				continue;

			pstCatInst2	=	a_pstCtx->stInst.astCatInsts + j;

			if (NULL == pstCatInst1->pstParent && szParentName[0] && 0 == strcasecmp(szParentName , pstCatInst2->szName))
			{
				pstCatInst1->pstParent	=	pstCatInst2;
			}

			if (NULL == pstCatInst1->pstForward && pstCatInst1->pstCat->szForwardCat[0] 
				&& 0 == strcasecmp(pstCatInst1->pstCat->szForwardCat , pstCatInst2->szName))
			{
				pstCatInst1->pstForward = pstCatInst2;
			}

			if (pstCatInst1->pstParent && pstCatInst1->pstForward)
			{
				break;
			}
		}
	}

	return 0;
}

static int tlog_check_forward_loop(TLOGCTX* pstCtx)
{
	TLOGCATEGORYINST *pstCatInst;
	TLOGCATEGORYINST *pstCatHead;
	TLOGCATEGORYINST *pstCatTail;
	int i, j;

	for (i=0; i<pstCtx->stInst.iCount; i++)
	{
		pstCatInst = &pstCtx->stInst.astCatInsts[i];
		pstCatHead = pstCatTail = pstCatInst;

		do
		{
			if (pstCatHead->pstForward)
			{
				pstCatHead = pstCatHead->pstForward;
			}

			for (j=0; j<2; j++)
			{
				if (pstCatTail->pstForward)
				{
					pstCatTail = pstCatTail->pstForward;
				}
			}
		}while(pstCatHead != pstCatTail);

		if (pstCatHead->pstForward)
		{
			return 1;
		}
	}
	
	return 0;
}

int tlog_init(TLOGCTX* a_pstCtx, TLOGCONF* a_pstConf)
{
	TLOGCATEGORY* pstCat;
	TLOGCATEGORYINST* pstCatInst;
	int iErr;
	int i;
	int iRet = 0;

	assert( a_pstCtx && a_pstConf );

	memset(a_pstCtx, 0, sizeof(*a_pstCtx));

	a_pstCtx->pstConf   =   a_pstConf;
	
	iErr	=	0;

	while(a_pstCtx->stInst.iCount<a_pstConf->iCount)
	{
		pstCat		=	a_pstConf->astCategoryList + a_pstCtx->stInst.iCount;
		pstCatInst	=	a_pstCtx->stInst.astCatInsts+ a_pstCtx->stInst.iCount;

		pstCatInst->piPriorityHigh		=	&a_pstConf->iPriorityHigh;
		pstCatInst->piPriorityLow       =   &a_pstConf->iPriorityLow;

		STRNCPY(pstCatInst->szName, pstCat->szName, sizeof(pstCatInst->szName));
		pstCatInst->pstCat	=	pstCat;

		if( !a_pstConf->iDelayInit )
		{	
			iRet = tlogany_init(&pstCatInst->stLogAny, &pstCat->stDevice);
			if(0 == iRet)
			{
				pstCatInst->iInited	=	1;
			}
			else
			{
				if( !a_pstConf->iSuppressError || pstCat->iMustSucc )
				{
					iErr++;
					break;
				}
			}
		}

		a_pstCtx->stInst.iCount++;
	}

	if( iErr )
	{
		tlog_fini(a_pstCtx);

		return iRet;
	}

	tlog_resolve_i(a_pstCtx);

	if (tlog_check_forward_loop(a_pstCtx))
	{
		printf("tlog conf forward category form loop\n");
		tlog_fini(a_pstCtx);

		return TLOG_ERR_MAKE_ERROR(TLOG_ERROR_LOOP);
	}

	//for more effective
	for (i=0; i<a_pstCtx->stInst.iCount; i++)
	{
		pstCatInst	=	a_pstCtx->stInst.astCatInsts + i;
		pstCat = pstCatInst->pstCat;

		if (NULL != pstCatInst->pstForward)
		{
			continue;
		}

		if (pstCat->iPriorityHigh > a_pstConf->iPriorityHigh || TLOG_PRIORITY_NULL == a_pstConf->iPriorityHigh)
		{
			pstCatInst->piPriorityHigh		=	&pstCat->iPriorityHigh;
		}

		if (pstCat->iPriorityLow < a_pstConf->iPriorityLow || TLOG_PRIORITY_NULL == a_pstConf->iPriorityLow)
		{
			pstCatInst->piPriorityLow = &pstCat->iPriorityLow;
		}
	}
	
	return 0;
}


int tlog_fini(TLOGCTX* a_pstCtx)
{
	TLOGCATEGORYINST* pstCatInst;

	assert( a_pstCtx );

	while(a_pstCtx->stInst.iCount>0)
	{
		a_pstCtx->stInst.iCount--;

		pstCatInst	=	a_pstCtx->stInst.astCatInsts + a_pstCtx->stInst.iCount;

		if( pstCatInst->iInited )
		{
			tlogany_fini(&pstCatInst->stLogAny);

			pstCatInst->iInited	=	0;
		}
	}
	
	return 0;
}


TLOGCATEGORYINST* tlog_get_category(TLOGCTX* a_pstCtx, const char* a_pszName)
{
	TLOGCATEGORYINST* pstCatInst;
	int i;

	for(i=0; i<a_pstCtx->stInst.iCount; i++)
	{
		pstCatInst	=	a_pstCtx->stInst.astCatInsts + i;

		if( 0==strcasecmp(pstCatInst->szName, a_pszName) )
			return pstCatInst;
	}

	return NULL;
}

