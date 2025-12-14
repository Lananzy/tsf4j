/**************************************************************
 @file $RCSfile: tmib.c,v $
  tsf managing MIB interface
  $Id: tmib.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
 @author $Author: kent $
 @date $Date: 2008/05/28 07:35:00 $
 @version 1.0
 @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
 @note Platform: Linux
 **************************************************************/

#include    "mng/tmib.h"
#define     MIB_ACS_FLAG 0664


int tmib_create(OUT HTMIB* a_phMib, IN const char* a_pszKey, IN int a_iDatas, IN int a_iSize)
{    
    HTMIB hMib = NULL;
    int iPageSize;
    int iHeadSize;

    assert( a_phMib && a_pszKey && a_iDatas>0 && a_iSize>0);

    iPageSize = tos_get_pagesize();

    assert( iPageSize>0 );
 
    iHeadSize = offsetof(TMIBINFO, entries) + sizeof(TMIBENTRY)*a_iDatas;
    iHeadSize = (iHeadSize+iPageSize-1)/iPageSize*iPageSize;

    if( a_iSize <= iHeadSize )
    {
        return -1;
    }

    hMib = (HTMIB) calloc(1, sizeof(TMIBINFO));

    if( NULL==hMib )
    {
        return -1;
    }
    
    hMib->iLock = -1;
    hMib->iShm = -1;
    hMib->bCreate = 1;          /* owner is creater */

    //hMib->iLock = tlockopen(a_pszKey, TLOCKF_CREATE | TLOCK_DFT_ACCESS, TMIB_IPC_TYPE);
    hMib->iLock = tlockopen(a_pszKey, TLOCKF_CREATE | MIB_ACS_FLAG, TMIB_IPC_TYPE);
    //hMib->iShm = tshmopen(a_pszKey, a_iSize, TSHMF_CREATE | TSHM_DFT_ACCESS, TMIB_IPC_TYPE);
    hMib->iShm = tshmopen(a_pszKey, a_iSize, TSHMF_CREATE | MIB_ACS_FLAG, TMIB_IPC_TYPE);
    //hMib->iShm = shmget(atoi(a_pszKey), a_iSize, IPC_CREAT | MAC_SHM_FLAG );
    if( -1==hMib->iLock || -1==hMib->iShm )
    {
        tmib_close(&hMib);
        return -1;
    }

    hMib->pstInfo = (LPTMIBINFO) tshmat(hMib->iShm, 0);

    if( !hMib->pstInfo )
    {
        tmib_close(&hMib);
        return -1;
    }

    if( 0==hMib->pstInfo->iVersion )
    {
        hMib->pstInfo->iVersion     = TMIB_VERSION;
        hMib->pstInfo->iSize        = a_iSize;
        hMib->pstInfo->iCurDatas    = 0;
        hMib->pstInfo->iMaxDatas    = a_iDatas;
        hMib->pstInfo->iHeadSize    = iHeadSize;
        hMib->pstInfo->iCheckSum    = 0;
        hMib->pstInfo->iExternSize  = DEFAULT_MIB_DATA_SIZE;
    }
    else if( TMIB_VERSION!=hMib->pstInfo->iVersion )
    {
        tmib_close(&hMib);
        return -1;
    }

    *a_phMib = hMib;

    return 0;
}


int tmib_destroy(IN const char* a_pszKey)
{
    HTMIB hMib=NULL;
    TMIBINFO *pstInfo;
    TMIBENTRY *pstEntry;
    
    int i;
    int iRet;

    iRet = tmib_open(&hMib, a_pszKey);
    if( iRet<0 )
        return -1;

    pstInfo = hMib->pstInfo;
    
    for(i=0; i<pstInfo->iCurDatas; i++)
    {
        pstEntry    =    pstInfo->entries + i;
        if ( pstEntry->bExtern )
        {
            tshmclose(pstEntry->iExShm, TSHMF_DELETE);
        }
    }
    tmib_close(&hMib);
    tlockdelete(a_pszKey, TMIB_IPC_TYPE);    
    tshmdelete(a_pszKey, TMIB_IPC_TYPE);
    return 0;

}

int tmib_open(OUT HTMIB *a_phMib, IN const char* a_pszKey)
{
    HTMIB hMib=NULL;

    assert( a_phMib && a_pszKey );

    hMib    =    (HTMIB) calloc(1, sizeof(TMIBINFO));

    if( NULL==hMib )
    {
        return -1;
    }

    hMib->iLock = -1;
    hMib->iShm  = -1;
    hMib->iLock = tlockopen(a_pszKey, 0, TMIB_IPC_TYPE);
    hMib->iShm  = tshmopen(a_pszKey, 0, 0, TMIB_IPC_TYPE);
    //hMib->iShm = shmget(atoi(a_pszKey), 0, MAC_SHM_FLAG );
    
    if( -1==hMib->iShm )
    {
        tmib_close(&hMib);
        return -1;
    }

    hMib->pstInfo = (LPTMIBINFO) tshmat(hMib->iShm, 0);
    if( !hMib->pstInfo || TMIB_VERSION!=hMib->pstInfo->iVersion )
    {
        tmib_close(&hMib);
        return -1;
    }

    *a_phMib = hMib;

    return 0;
}

int tmib_close(IN HTMIB *a_phMib)
{    
    HTMIB hMib;

    assert( a_phMib );

    hMib = *a_phMib;
    assert( hMib );
    

    if( hMib->pstInfo )
    {
        tshmdt(hMib->pstInfo);
        hMib->pstInfo = NULL;
    }

    if( -1!=hMib->iShm )
    {
        tshmclose(hMib->iShm, 0);
        hMib->iShm = -1;
    }

    if( -1!=hMib->iLock )
    {
        tlockclose(hMib->iLock, 0);
        hMib->iLock = -1;
    }

    free( hMib );
    *a_phMib = NULL;
        
    return 0;
}

int tmib_lock_i(IN HTMIB a_hMib)
{
    assert( a_hMib );

    if( -1==a_hMib->iLock )
        return -1;

    return tlockop(a_hMib->iLock, 1);
}

int tmib_unlock_i(IN HTMIB a_hMib)
{
    assert( a_hMib );

    if( -1==a_hMib->iLock )
        return -1;

    return tlockop(a_hMib->iLock, 0);
}

int tmib_set_extern_size(INOUT HTMIB a_hMib, IN int a_iExSize)
{
    int iPageSize;
    int iSize;

    assert( a_hMib );

    iPageSize = tos_get_pagesize();
    iSize = a_iExSize;
    if( iSize <= 0 )
    {
        iSize = DEFAULT_MIB_DATA_SIZE;
    }
    
    iSize = (iSize+iPageSize-1)/iPageSize*iPageSize;    
    
    if( tmib_lock_i(a_hMib)<0 )
    {
        return -1;
    }
    a_hMib->pstInfo->iExternSize = iSize;
    tmib_unlock_i(a_hMib);    
    
    return 0;
}

int tmib_set_updtime_unlock(INOUT HTMIB a_hMib, INOUT TMIBDATA* a_pstData, IN time_t a_tUpdTime)
{
    TMIBINFO* pstInfo=NULL;
    TMIBENTRY* pstEntry=NULL;
    int iRet;
    int iEntry;

    assert( a_hMib && a_pstData );

    iRet = tmib_get_data(a_hMib, a_pstData, 0);
    if( iRet<0 )
    {
        return iRet;
    }

    pstInfo = a_hMib->pstInfo;
    iEntry = a_pstData->iEntry;
    
    if( iEntry<0 || iEntry>=a_hMib->pstInfo->iCurDatas )
    {
        return -1;
    }
    pstEntry = pstInfo->entries + iEntry;
    
    if( !a_hMib->bCreate )    /* enable write. */
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_RDWR);

    a_hMib->pstInfo->entries[iEntry].tUpdateTime= a_tUpdTime;

    if( !a_hMib->bCreate )    /* disable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_READ);
    }

    return 0;
}

int tmib_set_updtime_now(INOUT HTMIB a_hMib, INOUT TMIBDATA* a_pstData)
{
    int iRet;

    assert( a_hMib && a_pstData );

    if( tmib_lock_i(a_hMib)<0 )
    {
        return -1;
    }
    iRet = tmib_set_updtime_unlock(a_hMib, a_pstData, time(NULL));

    tmib_unlock_i(a_hMib);

    return iRet;
}

int tmib_find_data_unlocked(IN HTMIB a_hMib, INOUT TMIBDATA* a_pstData, IN int a_bIncludeDelete)
{    
    TMIBINFO* pstInfo=NULL;
    TMIBENTRY* pstEntry=NULL;
    int i;

    assert( a_hMib && a_pstData );

    pstInfo = a_hMib->pstInfo;

    for(i=0; i<pstInfo->iCurDatas; i++)
    {
        pstEntry = pstInfo->entries + i;

        if( pstEntry->bDelete && !a_bIncludeDelete )
        {
            continue;
        }
        if( a_pstData->szLib[0] && stricmp(pstEntry->szLib, a_pstData->szLib) )
        {
            continue;
        }
        if( a_pstData->szMeta[0] && stricmp(pstEntry->szMeta, a_pstData->szMeta) )
        {
            continue;
        }
        if( a_pstData->szDomain[0] && stricmp(pstEntry->szDomain, a_pstData->szDomain) )
        {
            continue;
        }
        if( stricmp(pstEntry->szName, a_pstData->szName) ||
            pstEntry->iProcID!=a_pstData->iProcID )
        {
            continue;
        }
        if( a_pstData->iVersion && pstEntry->iVersion!=a_pstData->iVersion)
        {
            continue;
        }
        if( a_pstData->iPeriods && pstEntry->iPeriods!=a_pstData->iPeriods)
        {
            continue;
        }
        a_pstData->iVersion = pstEntry->iVersion;
        a_pstData->iPeriods = pstEntry->iPeriods;
        a_pstData->iEntry = i;
        a_pstData->bExtern = pstEntry->bExtern;
        if ( pstEntry->bExtern )
        {
            a_pstData->iSize = pstEntry->iExSize;
            a_pstData->pstHead= (LPTMIBHEAD) tshmat(pstEntry->iExShm, 0);
        }
        else
        {
            a_pstData->iSize = pstEntry->iSize;
            a_pstData->pstHead = (LPTMIBHEAD)(((int)pstInfo) + pstEntry->iOff);
        }
        a_pstData->iSize = a_pstData->iSize - MIB_HEAD_SIZE;
        a_pstData->pszData = (char*)(((int)a_pstData->pstHead) + MIB_HEAD_SIZE);

        return 0;
    }

    return -1;
}

int tmib_register_data_unlocked(INOUT HTMIB a_hMib, INOUT TMIBDATA* a_pstData)
{
    TMIBINFO* pstInfo=NULL;
    TMIBENTRY* pstEntry=NULL;
    
    int iOff;
    int iPageSize;
    int iSize;

    assert( a_hMib && a_pstData );
    if( a_pstData->iSize<=0 )
    {
        return -1;
    }
    iPageSize = tos_get_pagesize();
    iSize = a_pstData->iSize + MIB_HEAD_SIZE;

    iSize = (iSize+iPageSize-1)/iPageSize*iPageSize;

    pstInfo = a_hMib->pstInfo;

    if( pstInfo->iCurDatas>=pstInfo->iMaxDatas )
    {
        return -1;
    }
    if( pstInfo->iCurDatas>0 )
    {
        pstEntry = pstInfo->entries + pstInfo->iCurDatas - 1;
        iOff = pstEntry->iOff + pstEntry->iSize;
    }
    else
    {
        iOff = pstInfo->iHeadSize;
    }
    
    if( !a_hMib->bCreate )    /* enable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_RDWR);
    }
    
    pstEntry = pstInfo->entries + pstInfo->iCurDatas;

    memcpy(pstEntry->szLib, a_pstData->szLib, sizeof(a_pstData->szLib)-1);
    memcpy(pstEntry->szMeta, a_pstData->szMeta, sizeof(a_pstData->szMeta)-1);
    memcpy(pstEntry->szDomain, a_pstData->szDomain,sizeof(a_pstData->szDomain)-1);
    memcpy(pstEntry->szName, a_pstData->szName, sizeof(a_pstData->szName)-1);

    pstEntry->iProcID = a_pstData->iProcID;
    pstEntry->iOff = iOff;
    pstEntry->iVersion = a_pstData->iVersion;
    pstEntry->iPeriods = a_pstData->iPeriods;

    if ( a_pstData->bExtern || a_pstData->iSize > a_hMib->pstInfo->iExternSize )
    {
        pstEntry->bExtern = 1;
        pstEntry->iExSize = iSize;
        pstEntry->iSize = 0;
        pstEntry->iExShm = tshmopen(NULL, pstEntry->iExSize, TSHMF_CREATE | TSHM_DFT_ACCESS | TSHMF_PRIVATE, TMIB_IPC_TYPE); 
        a_pstData->pstHead = (LPTMIBHEAD) tshmat(pstEntry->iExShm, 0);

    }
    else
    {
        pstEntry->bExtern = 0;
        pstEntry->iExSize = 0;        
        pstEntry->iSize = iSize;
        a_pstData->pstHead = (LPTMIBHEAD)(((int)pstInfo) + pstEntry->iOff);
    }
    a_pstData->pstHead->iPeriods = pstEntry->iPeriods;
    a_pstData->pstHead->tUpdateTime = 0;
    a_pstData->pstHead->tReportTime = 0;    

    a_pstData->pszData = (char*)(((int)a_pstData->pstHead) + MIB_HEAD_SIZE);
    a_pstData->iSize = iSize - MIB_HEAD_SIZE;
    a_pstData->iEntry = pstInfo->iCurDatas;
    pstInfo->iCurDatas++;
    
    if( !a_hMib->bCreate )    /* disable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_READ);
        mprotect(a_pstData->pstHead, pstEntry->iSize, PROT_READ);
    }

    return 0;
}

int tmib_check_entry_unlocked(INOUT HTMIB a_hMib, IN TMIBDATA* a_pstData)
{
    TMIBINFO* pstInfo=NULL;
    TMIBENTRY* pstEntry=NULL;

    assert( a_hMib && a_pstData );

    pstInfo = a_hMib->pstInfo;

    pstEntry =  pstInfo->entries + a_pstData->iEntry;

    if( pstEntry->bDelete )
    {
        if( !a_hMib->bCreate )    /* enable write. */
        {
            mprotect(pstInfo, pstInfo->iHeadSize, PROT_RDWR);
        }
        pstEntry->bDelete= 0;

        if( !a_hMib->bCreate )    /* disable write. */
        {
            mprotect(pstInfo, pstInfo->iHeadSize, PROT_READ);
        }
    }

    if( !a_hMib->bCreate )    /* disable write. */
    {
        mprotect(a_pstData->pstHead, a_pstData->iSize + MIB_HEAD_SIZE, PROT_READ);
    }

    return 0;
}

int tmib_register_data(INOUT  HTMIB a_hMib, INOUT TMIBDATA* a_pstData)
{    
    int iRet;

    assert( a_hMib && a_pstData );

    if( tmib_lock_i(a_hMib)<0 )
    {
        return -1;
    }
    
    if( tmib_find_data_unlocked(a_hMib, a_pstData, 1)<0 )
    {    
        iRet = tmib_register_data_unlocked(a_hMib, a_pstData);
    }
    else
    {
        iRet = tmib_check_entry_unlocked(a_hMib, a_pstData);
    }

    tmib_unlock_i(a_hMib);

    return iRet;
}


int tmib_register_data_by_mbhd(INOUT HTMIB a_hMib, IN HTMBDESC a_hDesc, INOUT TMIBDATA* a_pstData)
{
    TMIBINFO* pstInfo;
    LPTDRMETA pstMeta;
    int iPageSize;
    int iSize;
    int iRet;

    assert( a_hMib && a_hDesc && a_pstData );
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
    iPageSize = tos_get_pagesize();

    pstInfo = a_hMib->pstInfo;
    

    iRet = tmb_open_metalib(a_hDesc, a_pstData->szLib, a_pstData->iVersion);
    if( iRet<0 )
        return iRet;

    iRet = tmb_meta_by_name(a_hDesc, a_pstData->szMeta, &pstMeta);
    if( iRet<0 )
        return iRet;

    if( 0==a_pstData->iSize )
        iSize = tdr_get_meta_size(pstMeta);
    else
        iSize = a_pstData->iSize;

    iSize = (iSize+iPageSize-1)/iPageSize*iPageSize;

    if( iSize<tdr_get_meta_size(pstMeta) ) 
        return -1;

    a_pstData->iSize = iSize;

    return tmib_register_data(a_hMib, a_pstData);
}

    
int tmib_register_data_by_mbkey(INOUT HTMIB a_hMib, IN const char* a_pszBase, INOUT TMIBDATA* a_pstData)
{
    HTMBDESC hDesc;
    int iRet;

    iRet = tmb_open(&hDesc, a_pszBase, 1);
    if( iRet<0 )
        return -1;

    iRet = tmib_register_data_by_mbhd(a_hMib, hDesc, a_pstData);

    tmb_close(&hDesc);

    return iRet;
}


int tmib_protect_data(IN TMIBDATA* a_pstData, IN int a_bReadOnly)
{
    int iRet ;
    assert( a_pstData && a_pstData->pszData && a_pstData->iSize>0 );

    if( a_bReadOnly )
    {
        iRet = mprotect(a_pstData->pstHead, a_pstData->iSize+MIB_HEAD_SIZE, PROT_READ);
    }
    else
    {
        iRet = mprotect(a_pstData->pstHead, a_pstData->iSize+MIB_HEAD_SIZE, PROT_RDWR);
    }
    return iRet;
}

int tmib_unregister_data_unlocked(INOUT HTMIB a_hMib, IN TMIBDATA* a_pstData)
{
    TMIBINFO* pstInfo=NULL;
    TMIBENTRY* pstEntry=NULL;

    assert( a_hMib && a_pstData );

    pstInfo = a_hMib->pstInfo;

    a_pstData->szLib[0] = '\0';
    a_pstData->szMeta[0] = '\0';

    if( tmib_find_data_unlocked(a_hMib, a_pstData, 1)<0 )
    {
        return -1;
    }

    pstEntry = pstInfo->entries + a_pstData->iEntry;
    if( pstEntry->bDelete )
    {
        return 0;
    }

    if( !a_hMib->bCreate )    /* enable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_RDWR);
    }
    pstEntry->bDelete=    1;

    if( !a_hMib->bCreate )    /* disable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_READ);
    }
    return 0;
}


int tmib_unregister_data(INOUT HTMIB a_hMib, IN TMIBDATA* a_pstData)
{
    int iRet;

    assert( a_hMib && a_pstData );

    if( tmib_lock_i(a_hMib)<0 )
    {
        return -1;
    }

    iRet = tmib_unregister_data_unlocked(a_hMib, a_pstData);

    tmib_unlock_i(a_hMib);

    return iRet;
}

int tmib_unregister_domain_unlocked(INOUT HTMIB a_hMib, IN const char* a_pszDomain)
{    
    TMIBINFO *pstInfo=NULL;
    TMIBENTRY *pstEntry=NULL;
    int i;

    assert( a_hMib && a_pszDomain );

    pstInfo = a_hMib->pstInfo;

    if( !a_hMib->bCreate )    /* enable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_RDWR);
    }
    
    for(i=0; i<pstInfo->iCurDatas; i++)
    {
        pstEntry = pstInfo->entries + i;

        if( 0!=stricmp(pstEntry->szDomain, a_pszDomain) )
        {
            continue;
        }

        pstEntry->bDelete = 1;
    }

    if( !a_hMib->bCreate )    /* disable write. */
    {
        mprotect(pstInfo, pstInfo->iHeadSize, PROT_READ);
    }
    
    return 0;
}

int tmib_unregister_domain(INOUT HTMIB a_hMib, IN const char* a_pszDomain)
{    
    int iRet;

    assert( a_hMib && a_pszDomain );

    if( tmib_lock_i(a_hMib)<0 )
    {
        return -1;
    }

    iRet = tmib_unregister_domain_unlocked(a_hMib, a_pszDomain);

    tmib_unlock_i(a_hMib);

    return iRet;
}

int tmib_get_data(IN HTMIB a_hMib, INOUT TMIBDATA* a_pstData, IN int a_bReadOnly)
{
    LPTMIBINFO pstInfo=NULL;
    LPTMIBENTRY pstEntry=NULL;
    int iPageSize;
    int i;

    assert( a_hMib && a_pstData );

    pstInfo = a_hMib->pstInfo;
    iPageSize = tos_get_pagesize();

    for(i=0; i<pstInfo->iCurDatas; i++)
    {
        pstEntry = pstInfo->entries + i;

        if( pstEntry->bDelete )
        {
            continue;
        }

        if( 0==stricmp(pstEntry->szDomain, a_pstData->szDomain) &&
            0==stricmp(pstEntry->szName, a_pstData->szName) &&
            ( !a_pstData->iProcID || pstEntry->iProcID==a_pstData->iProcID ) && 
            ( !a_pstData->iVersion || !pstEntry->iVersion || 
               a_pstData->iVersion==pstEntry->iVersion) &&
            ( !a_pstData->iPeriods || !pstEntry->iPeriods || 
               a_pstData->iPeriods==pstEntry->iPeriods) )
        {
            if( pstEntry->iOff % iPageSize )
            {
                return -1;
            }
            
            memcpy(a_pstData->szLib, pstEntry->szLib, sizeof(pstEntry->szLib));
            a_pstData->szLib[sizeof(a_pstData->szLib)-1] = '\0';

            memcpy(a_pstData->szMeta, pstEntry->szMeta, sizeof(pstEntry->szMeta));
            a_pstData->szMeta[sizeof(a_pstData->szMeta)-1] = '\0';
            a_pstData->iVersion = pstEntry->iVersion;
            a_pstData->iPeriods= pstEntry->iPeriods;
            a_pstData->iEntry = i;

            if ( pstEntry->bExtern )
            {
                a_pstData->iSize = pstEntry->iExSize;
                a_pstData->pstHead= (LPTMIBHEAD) tshmat(pstEntry->iExShm, 0);
            }
            else
            {
                a_pstData->iSize = pstEntry->iSize;
                a_pstData->pstHead = (LPTMIBHEAD)(((int)pstInfo) + pstEntry->iOff);
            }
            a_pstData->iSize = a_pstData->iSize - MIB_HEAD_SIZE;
            a_pstData->pszData = (char*)(((int)a_pstData->pstHead) + MIB_HEAD_SIZE);

            if( a_bReadOnly )
            {
                return 0;
            }

            mprotect((char*)a_pstData->pstHead, a_pstData->iSize+MIB_HEAD_SIZE, PROT_RDWR);
            return 0;

        }
    }
    
    return -1;
}

int tmib_validate_data(IN HTMIB a_hMib, INOUT TMIBDATA* a_pstData, IN int a_bReadOnly)
{
    int iEntry;

    assert( a_hMib && a_pstData );

    iEntry = a_pstData->iEntry;

    if( iEntry<0 || iEntry>=a_hMib->pstInfo->iCurDatas )
    {
        return -1;
    }

    if( !a_hMib->pstInfo->entries[iEntry].bDelete )
    {
        return 0;
    }

    return tmib_get_data(a_hMib, a_pstData, a_bReadOnly);
}


int tmib_dump_head(IN HTMIB a_hMib, IN FILE* a_fp)
{
    TMIBINFO *pstInfo=NULL;
    TMIBENTRY *pstEntry=NULL;

    struct tm *pstTime=NULL;    
    char   szTempStr[40];
    
    int i;
    
    assert( a_hMib );
    if ( NULL==a_fp ) 
        a_fp = stdout;
    
    pstInfo = a_hMib->pstInfo;
    fprintf(a_fp, "\nMib Head Data:\n"
                     "\t\t size     = %d\n"
                     "\t\t maxdatas = %d\n"
                     "\t\t curdatas = %d\n"
                     "\t\t headsize = %d\n"
                     "\t\t bCreate  = %d\n"
                     ,
                     pstInfo->iSize,
                     pstInfo->iMaxDatas,
                     pstInfo->iCurDatas,
                     pstInfo->iHeadSize,
                     a_hMib->bCreate
                     );
    
    for(i=0; i<pstInfo->iCurDatas; i++)
    {
        pstEntry = pstInfo->entries + i;

        if ( pstEntry->tUpdateTime  )
        {
            pstTime = localtime( & pstEntry->tUpdateTime );
            strftime( szTempStr, 20, "%Y-%m-%d %H:%M:%S", pstTime);
        }
        
        fprintf(a_fp, "Data[%04d]: Domain=\"%s\" Name=\"%s\" size=%d exsize=%d, exshm=%d, ProcID=%d %s %s\n",
            i, pstEntry->szDomain, pstEntry->szName, pstEntry->iSize,pstEntry->iExSize, pstEntry->iExShm,
            pstEntry->iProcID,   pstEntry->tUpdateTime?szTempStr:"",
            pstEntry->bDelete?"[deleted]":"" );
    }

    return 0;    
}


int tmib_dump(IN HTMBDESC a_hDesc, IN TMIBDATA* a_pstData, IN FILE* a_fp)
{
    int iRet;
    LPTDRMETA pstMeta;
    TDRDATA stHost;
   

    assert( a_hDesc && a_pstData );

    iRet = tmb_open_metalib(a_hDesc, a_pstData->szLib, a_pstData->iVersion);
    if( iRet<0 )
        return iRet;

    iRet = tmb_meta_by_name(a_hDesc, a_pstData->szMeta, &pstMeta);
    if( iRet<0 )
        return iRet;

    stHost.pszBuff = a_pstData->pszData;
    stHost.iBuff = a_pstData->iSize;

    return tdr_fprintf(pstMeta, a_fp, &stHost, a_pstData->iVersion);
}

int tmib_dump_once(IN const char* a_pszBase, IN TMIBDATA* a_pstData, IN FILE* a_fp)
{    
    HTMBDESC hDesc;
    
    int iRet;

    iRet = tmb_open(&hDesc, a_pszBase, 1);
    if( iRet<0 )
        return -1;

    iRet = tmib_dump(hDesc, a_pstData, a_fp);

    tmb_close(&hDesc);

    return iRet;
}

void FormatTime_i(time_t tSecond, char* sBuf, int iSize) 
{
    struct tm *pstm;

    if ( iSize < 20 || 0>=tSecond) 
    {
        sBuf[0]=0;
        return;
    }
    pstm = localtime( &tSecond );
    strftime( sBuf, iSize, "%Y-%m-%d %H:%M:%S", pstm);
    return;
}


int tmib_dump_all(IN HTMIB a_hMib, IN const char* a_pszBase, IN FILE* a_fp)
{    
    TMIBDATA stData;
    HTMBDESC hDesc;
    TMIBINFO *pstInfo;
    TMIBENTRY *pstEntry;
    char     szReport[25], szUpdate[25];
   
    int i;
    int iRet;

    assert( a_hMib && a_pszBase && a_fp );

    iRet = tmb_open(&hDesc, a_pszBase, 1);
    if( iRet<0 )
        return -1;

    pstInfo = a_hMib->pstInfo;
    
    for(i=0; i<pstInfo->iCurDatas; i++)
    {
        pstEntry    =    pstInfo->entries + i;

        fprintf(a_fp, "Data %d: Domain=\"%s\" Name=\"%s\" Lib=\"%s\", Meta=\"%s\", ProcID=%d size=%d, ext=%d %s\n",
            i, pstEntry->szDomain, pstEntry->szName,  pstEntry->szLib, pstEntry->szMeta,
            pstEntry->iProcID,  pstEntry->iSize, pstEntry->iExSize, pstEntry->bDelete?"[deleted]":"" );

        memcpy(stData.szLib, pstEntry->szLib, sizeof(pstEntry->szLib));
        stData.szLib[sizeof(stData.szLib)-1]=    '\0';

        memcpy(stData.szMeta, pstEntry->szMeta, sizeof(pstEntry->szMeta));
        stData.szMeta[sizeof(stData.szMeta)-1]=    '\0';
        stData.iVersion = pstEntry->iVersion;
        stData.iEntry = i;
        stData.bExtern = pstEntry->bExtern;
        
        if ( stData.bExtern )
        {
            stData.iSize = pstEntry->iExSize;
            stData.pstHead = (LPTMIBHEAD) tshmat(pstEntry->iExShm, 0);
        }
        else
        {
            stData.iSize = pstEntry->iSize;
            stData.pstHead = (LPTMIBHEAD)(((int)pstInfo) + pstEntry->iOff);
        }        
        stData.iSize = stData.iSize - MIB_HEAD_SIZE;
        stData.pszData = (char*)(((int)stData.pstHead) + MIB_HEAD_SIZE);

        FormatTime_i(stData.pstHead->tReportTime, szReport, sizeof(szReport));
        FormatTime_i(stData.pstHead->tUpdateTime, szUpdate, sizeof(szUpdate));        
        fprintf( a_fp, "        periods=%d, update at %s, reprot at %s\n", stData.pstHead->iPeriods, szUpdate, szReport);
        tmib_dump(hDesc, &stData, a_fp);
    }

    tmb_close(&hDesc);

    return 0;
}


