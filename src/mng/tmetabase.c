/**************************************************************
 @file $RCSfile: tmetabase.c,v $
  meta lib managing 
 $Id: tmetabase.c,v 1.2 2008/12/31 02:22:43 jackyai Exp $
 @author $Author: jackyai $
 @date $Date: 2008/12/31 02:22:43 $
 @version 1.0
 @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
 @note Platform: Linux
 **************************************************************/

/*
#include "ov_os.h"
#include "ov_gfile.h"
#include "ov_gshm.h"
#include "ov_glock.h"
#include "ov_dr.h"
*/
#include "pal/pal.h"
#include "mng/tmetabase.h"

#define     MB_ACS_FLAG 0664

int tmb_create(OUT HTMBDESC* a_phDesc, IN const char* a_pszKey, IN int a_iMaxLibs, IN int a_iSize)
{
	HTMBDESC hDesc;
 
	assert( a_phDesc && a_pszKey && a_iMaxLibs>0 && a_iSize>0);

	if( a_iSize<=offsetof(TMETABASE, offs) + sizeof(int)*a_iMaxLibs )
		return -1;

	hDesc	=	(HTMBDESC) calloc(1, sizeof(TMBDESC));

	if( NULL==hDesc )
		return -1;

	hDesc->iLock	=	-1;
	hDesc->iShm	=	-1;

	//hDesc->iLock	=	tlockopen(a_pszKey, TLOCKF_CREATE | TLOCK_DFT_ACCESS, TMB_IPC_TYPE);
	hDesc->iLock	=	tlockopen(a_pszKey, TLOCKF_CREATE | MB_ACS_FLAG, TMB_IPC_TYPE);

	//hDesc->iShm	=	tshmopen(a_pszKey, a_iSize, TSHMF_CREATE | TSHM_DFT_ACCESS, TMB_IPC_TYPE);
	hDesc->iShm	=	tshmopen(a_pszKey, a_iSize, TSHMF_CREATE | MB_ACS_FLAG, TMB_IPC_TYPE);
	
	if( -1==hDesc->iLock || -1==hDesc->iShm )
	{
		tmb_close(&hDesc);
		return -1;
	}

	hDesc->pstBase	=	(LPTMETABASE) tshmat(hDesc->iShm, 0);

	if( !hDesc->pstBase )
	{
		tmb_close(&hDesc);
		return -1;
	}

	if( 0==hDesc->pstBase->iVersion )
	{
		hDesc->pstBase->iVersion	=	TMB_VERSION;
		hDesc->pstBase->iSize		=	a_iSize;
		hDesc->pstBase->iCurLibs	=	0;
		hDesc->pstBase->iMaxLibs	=	a_iMaxLibs;
	}
	else if( TMB_VERSION!=hDesc->pstBase->iVersion )
	{
		tmb_close(&hDesc);
		return -1;
	}

	*a_phDesc	=	hDesc;

	return 0;
}

int tmb_destroy(IN char* a_pszKey)
{
	tlockdelete(a_pszKey, TMB_IPC_TYPE);

	return tshmdelete(a_pszKey, TMB_IPC_TYPE);
}

int tmb_open(OUT HTMBDESC* a_phDesc, IN const char* a_pszKey, IN int a_fReadOnly)
{	
	HTMBDESC hDesc;
    int iFlag = 0;

	assert( a_phDesc && a_pszKey );

	hDesc	=	(HTMBDESC) calloc(1, sizeof(TMBDESC));

	if( NULL==hDesc )
		return -1;

	hDesc->iLock	=	-1;
	hDesc->iShm	=	-1;

	hDesc->iShm	=	tshmopen(a_pszKey, 0, 0, TMB_IPC_TYPE);
	
	if( -1==hDesc->iShm )
	{
		tmb_close(&hDesc);
		return -1;
	}

    if ( a_fReadOnly )
        iFlag = TSHMF_RDONLY;
	hDesc->pstBase	=	(LPTMETABASE) tshmat(hDesc->iShm, iFlag);

	if( !hDesc->pstBase || TMB_VERSION!=hDesc->pstBase->iVersion )
	{
		tmb_close(&hDesc);
		return -1;
	}

	*a_phDesc	=	hDesc;

	return 0;
}

int tmb_close(INOUT HTMBDESC* a_phDesc)
{
    HTMBDESC hDesc;
	assert( a_phDesc );

    hDesc = *a_phDesc;
    assert( hDesc);

    

	hDesc->pstLib	=	NULL;

	if( hDesc->pstBase)
	{
		tshmdt(hDesc->pstBase);
		hDesc->pstBase	=	NULL;
	}

	if( -1!=hDesc->iShm )
	{
		tshmclose(hDesc->iShm, 0);
		hDesc->iShm	=	-1;
	}

	if( -1!=hDesc->iLock )
	{
		tlockclose(hDesc->iLock, 0);
		hDesc->iLock	=	-1;
	}

	free( hDesc );
    *a_phDesc = NULL;

	return 0;
}

int tmb_lock(IN HTMBDESC a_hDesc)
{
	assert( a_hDesc );

	if( -1==a_hDesc->iLock )
		return -1;

	return tlockop(a_hDesc->iLock, 1);
}

int tmb_unlock(IN HTMBDESC a_hDesc)
{
	assert( a_hDesc );

	if( -1==a_hDesc->iLock )
		return -1;

	return tlockop(a_hDesc->iLock, 0);
}

int tmb_dump(IN HTMBDESC a_hDesc, IN int a_fDetail, IN FILE* a_fp)
{
	TMETABASE* pstBase;
	LPTDRMETALIB pstLib;
	int iOff;
	int i;

	assert( a_hDesc );

	pstBase	=	a_hDesc->pstBase;

	if( pstBase->iCurLibs<=0 )
		iOff	=	offsetof(TMETABASE, offs) + sizeof(int)*pstBase->iMaxLibs;
	else
	{
		iOff	=	pstBase->offs[pstBase->iCurLibs-1];

		pstLib	=	(LPTDRMETALIB)(((int)pstBase) + iOff);

		iOff	+=	tdr_size(pstLib);
	}

	fprintf(a_fp, "MetaBase Info:\n");
	fprintf(a_fp, "Size=%d Used=%d MaxLibs=%d CurLibs=%d:\n\n", pstBase->iSize, iOff, pstBase->iMaxLibs, pstBase->iCurLibs);

	if( !a_fDetail )
		return 0;

	for(i=0; i<pstBase->iCurLibs; i++)
	{
		pstLib	=	(LPTDRMETALIB) (((int)pstBase) + pstBase->offs[i]);
		fprintf(a_fp, "LibInfo (Index=%d):\n\n", i);
        tdr_dump_metalib(pstLib, a_fp);
		//tdr_save_metalib_fp(pstLib, a_fp);
	}

	return 0;
}

LPTDRMETALIB tmb_find_unlocked(IN HTMBDESC a_hDesc, IN const char* a_pszName, IN int a_iVersion)
{	
	TMETABASE* pstBase;
	LPTDRMETALIB pstLib;
	LPTDRMETALIB pstFind;
	int i;

	assert( a_hDesc && a_pszName );

	if( !a_iVersion )
		a_iVersion	=	TDR_MAX_VERSION;

	pstBase	=	a_hDesc->pstBase;
	pstFind	=	NULL;

	for(i=0; i<pstBase->iCurLibs; i++)
	{
		pstLib	=	(LPTDRMETALIB)(((int)pstBase) + pstBase->offs[i]);

		if( 0==stricmp(tdr_get_metalib_name(pstLib), a_pszName) )
		{
			if( tdr_get_metalib_version(pstLib)>a_iVersion )
				continue;

			if( !pstFind || tdr_get_metalib_version(pstFind)<tdr_get_metalib_version(pstLib) )
				pstFind	=	pstLib;
		}
	}

	return pstFind;
}

int tmb_append_unlocked(INOUT HTMBDESC a_hDesc, IN LPTDRMETALIB a_pstLib)
{	
	TMETABASE* pstBase;
	LPTDRMETALIB pstFind;
	char* pszCur;
	int iOff;
	int iRet;
	int iLibVersion;

	pstBase	=	a_hDesc->pstBase;

	if( pstBase->iCurLibs >= pstBase->iMaxLibs )
		return -1;

	iLibVersion = tdr_get_metalib_version(a_pstLib);
	pstFind	=	tmb_find_unlocked(a_hDesc, tdr_get_metalib_name(a_pstLib), iLibVersion);

	if( pstFind && iLibVersion == tdr_get_metalib_version(pstFind))
	{
		if( tdr_get_metalib_build_version(a_pstLib) == tdr_get_metalib_build_version(pstFind) &&
		    tdr_size(a_pstLib) == tdr_size(pstFind))
			return 0;
		else 
			return -2;
	}

	if( pstBase->iCurLibs>0 )
	{
		iOff	=	pstBase->offs[pstBase->iCurLibs-1];
		pszCur	=	(char*) (((int)pstBase) + iOff);

		iOff	+=	tdr_size((LPTDRMETALIB)pszCur);
		pszCur	=	(char*) (((int)pstBase) + iOff);
	}
	else
	{
		iOff	=	offsetof(TMETABASE, offs) + sizeof(int)*pstBase->iMaxLibs;
		pszCur	=	(char*) (((int)pstBase) + iOff);
	}

	if( iOff + tdr_size(a_pstLib) > pstBase->iSize )     
		iRet	=	-3;
	else
	{
		iRet	=	0;
		
		memcpy( pszCur, a_pstLib, tdr_size(a_pstLib));

		pstBase->offs[pstBase->iCurLibs]=	iOff;

		pstBase->iCurLibs++;
	}

	return iRet;
}

int tmb_append(INOUT HTMBDESC a_hDesc, IN const char* a_pszPath)
{
    LPTDRMETALIB pstLib;
    int iRet;

    if( tdr_load_metalib(&pstLib, a_pszPath)<0 )
        return -1;

    tmb_lock(a_hDesc);

    iRet    =   tmb_append_unlocked(a_hDesc, pstLib);

    tmb_unlock(a_hDesc);

    tdr_free_lib(&pstLib);

    return iRet;
}

int tmb_open_metalib(INOUT HTMBDESC a_hDesc, IN const char* a_pszLib, IN int a_iVersion)
{
	LPTDRMETALIB pstLib;

	assert( a_hDesc && a_pszLib );

	pstLib	=	tmb_find_unlocked(a_hDesc, a_pszLib, a_iVersion);

	if( !pstLib )
		return -1;

	a_hDesc->pstLib	=	pstLib;

	return 0;
}


int tmb_meta_by_name(IN HTMBDESC a_hDesc, IN const char* a_pszName, INOUT LPTDRMETA* a_ppstMeta)
{
	int iID;
	char* pszPos;
	int iLen;
	LPTDRMETA pstMeta;

	assert( a_hDesc && a_pszName && a_ppstMeta );

	if( !a_hDesc->pstLib )
		return -1;

	iLen	=	strlen( a_pszName );
	iID	=	strtol(a_pszName, &pszPos, 0);

	if( iLen==pszPos-a_pszName )
	{
		pstMeta = tdr_get_meta_by_id(a_hDesc->pstLib, iID);
	}
	else
	{
		pstMeta =  tdr_get_meta_by_name(a_hDesc->pstLib, a_pszName);
	}

	*a_ppstMeta = pstMeta;
	if (NULL == pstMeta)
	{
		return -1;
	}
	
	return 0;
}

int tmb_meta_by_id(IN HTMBDESC a_hDesc, IN int a_iID, INOUT LPTDRMETA* a_ppstMeta)
{
	assert( a_hDesc && a_ppstMeta );

	if( !a_hDesc->pstLib )
		return -1;

	*a_ppstMeta = tdr_get_meta_by_id(a_hDesc->pstLib, a_iID);

	if (NULL == *a_ppstMeta)
	{
		return -2;
	}
	
	return 0;
}

