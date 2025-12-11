/*
**  @file $RCSfile: tconnmgr.c,v $
**  general description of this module
**  $Id: tconnmgr.c,v 1.7 2009/04/09 07:31:43 hardway Exp $
**  @author $Author: hardway $
**  @date $Date: 2009/04/09 07:31:43 $
**  @version $Revision: 1.7 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include "tdr/tdr.h"
#include "tbus/tbus.h"
#include "obus/oi_misc.h"
#include "obus/oi_log.h"
#include "obus/oi_cfg.h"
#include "obus/oi_cistr.h"
#include "obus/oi_str.h"
#include "obus/og_ini.h"
#include "obus/og_bus.h"

#include "tdr/tdr_metalib_manage_i.h"
#include "tconnmgr.h"

#include "apps/tcltapi/tqqdef.h"
#include "apps/tcltapi/tpdudef.h"
#include "apps/tcltapi/tcltapi.h"
#include "apps/tcltapi/tqqapi.h"

#ifdef WIN32
	#pragma warning(disable:4996)
#endif


extern int tconnd_get_pkglen(TCONNINST* pstInst, TCONNTHREAD* pstThread);
extern int tconnd_get_thttp_pkglen(TCONNINST* pstInst, TCONNTHREAD* pstThread);
extern int tconnd_get_none_pkglen(TCONNINST* pstInst, TCONNTHREAD* pstThread);
extern int tconnd_get_qq_pkglen(TCONNINST* pstInst, TCONNTHREAD* pstThread);

int tconnd_fini_tdrinstlist(TDRINSTLIST* pstTDRInstList)
{
	int i;

	for(i=0; i<pstTDRInstList->iCount; i++)
	{
		tdr_free_lib(&pstTDRInstList->astInsts[i].pstLib);
	}

	pstTDRInstList->iCount	=	0;

	return 0;
}

int tconnd_init_tdrinstlist(TDRINSTLIST* pstTDRInstList, TDRLIST* pstTDRList)
{
	TDR* pstTDR;
	TDRINST* pstTDRInst;
	int iIsFail	=	0;

	pstTDRInstList->iCount	=	0;

	while( pstTDRInstList->iCount < pstTDRList->iCount )
	{
		if( pstTDRInstList->iCount >= (int) (sizeof(pstTDRInstList->astInsts)/sizeof(pstTDRInstList->astInsts[0])) )
		{
			iIsFail	=	1;
			break;
		}

		pstTDR		=	pstTDRList->astTDRs + pstTDRInstList->iCount;
		pstTDRInst	=	pstTDRInstList->astInsts + pstTDRInstList->iCount;

		if( tdr_load_metalib(&pstTDRInst->pstLib, pstTDR->szPath)<0 )
		{
			iIsFail	=	1;
			printf("tdr_load_metalib failed:path = %s\n",pstTDR->szPath);
			break;
		}

		STRNCPY(pstTDRInst->szName, pstTDR->szName, sizeof(pstTDRInst->szName));

		pstTDRInstList->iCount++;
	}

	if( !iIsFail )
	{
		return 0;
	}

	tconnd_fini_tdrinstlist(pstTDRInstList);

	return -1;
}

int tconnd_find_lib(LPTDRINSTLIST pstTDRInstList, const char* pszName, LPTDRMETALIB* ppstLib)
{
	int i;
	int iIsFind = 0;
	LPTDRMETALIB pstFind = NULL;

	if( !pszName || !pszName[0] )
	{
		if( ppstLib )
		{
			*ppstLib	=	NULL;
		}

		return 0;
	}

	for(i=0; i<pstTDRInstList->iCount; i++)
	{
		if( !strcasecmp(pszName, pstTDRInstList->astInsts[i].szName) )
		{
			pstFind	=	pstTDRInstList->astInsts[i].pstLib;
			iIsFind	=	1;
			break;
		}
	}

	if( !iIsFind )
	{
		return -1;
	}

	if( ppstLib )
	{
		*ppstLib	=	pstFind;
	}

	return 0;
}

int tconnd_find_meta(TDRINSTLIST* pstTDRInstList, const char* pszName, LPTDRMETALIB pstPrefer, LPTDRMETA* ppstFind)
{
	LPTDRMETALIB pstLib;
	LPTDRMETA pstMeta;
	int iIsFind;
	int i;

	if( !pszName || !pszName[0] )
		return 0;

	if( pstPrefer )
	{
		pstMeta	=	tdr_get_meta_by_name(pstPrefer, pszName);

		if( !pstMeta )
		{
			return -1;
		}
		else
		{
			*ppstFind	=	pstMeta;
			return 0;
		}
	}

	iIsFind	=	0;

	for(i=0; i<pstTDRInstList->iCount; i++)
	{
		pstLib	=	pstTDRInstList->astInsts[i].pstLib;

		if( pstLib==pstPrefer )
			continue;

		pstMeta	=	tdr_get_meta_by_name(pstLib, pszName);

		if( pstMeta )
		{
			iIsFind	=	1;
			break;
		}
	}

	if( iIsFind )
	{
		*ppstFind	=	pstMeta;
		return 0;
	}
	else
	{
		return -1;
	}
}

int tconnd_init_pdulen_tdrparser(TDRINSTLIST* pstTDRInstList, PDUINST* pstPDUInst, PDU* pstPDU)
{
	LPTDRMETALIB pstPrefer=NULL;
	int iRet ;
	TDRSIZEINFO stSizeInfo;
	LPPDULENTDRPARSERINST pstTDRParserInst; 
	LPPDULENTDRPARSER pstTDRParser ;

	pstTDRParserInst = &pstPDUInst->stLenParser.stTDRParser;
	pstTDRParser = &pstPDU->stLenParser.stTDRParser;
	if( tconnd_find_lib(pstTDRInstList, pstTDRParser->szTDR, &pstPrefer)<0 )
	{
              printf("tconnd_find_lib load Metalib failed!TDR Name = %s\n",pstTDRParser->szTDR);
		return -1;
	}

	
	if( tconnd_find_meta(pstTDRInstList, pstTDRParser->szPkg, pstPrefer, &pstTDRParserInst->pstPkg)<0 )
	{
              printf("tconnd_find_meta find meta failded!Meta Name = %s\n",pstTDRParser->szPkg);
		return -1;
	}

	iRet = tdr_sizeinfo_to_off_i(&stSizeInfo, pstTDRParserInst->pstPkg, TDR_INVALID_INDEX, pstTDRParser->szPkgLen);
	if (TDR_SUCCESS == iRet)
	{
		pstTDRParserInst->iPkgLenNOff = stSizeInfo.iNOff;
		pstTDRParserInst->iPkgLenUnitSize = stSizeInfo.iUnitSize;
	}
	iRet = tdr_sizeinfo_to_off_i(&stSizeInfo, pstTDRParserInst->pstPkg, TDR_INVALID_INDEX, pstTDRParser->szHeadLen);
	if (TDR_SUCCESS == iRet)
	{
		pstTDRParserInst->iHeadLenNOff = stSizeInfo.iNOff;
		pstTDRParserInst->iHeadLenUnitSize = stSizeInfo.iUnitSize;
	}
	iRet = tdr_sizeinfo_to_off_i(&stSizeInfo, pstTDRParserInst->pstPkg, TDR_INVALID_INDEX, pstTDRParser->szBodyLen);
	if (TDR_SUCCESS == iRet)
	{
		pstTDRParserInst->iBodyLenNOff = stSizeInfo.iNOff;
		pstTDRParserInst->iBodyLenUnitSize = stSizeInfo.iUnitSize;
	}
	if ((0 >= pstTDRParserInst->iPkgLenUnitSize) && (
		(0 >= pstTDRParserInst->iHeadLenUnitSize) || (0 >= pstTDRParserInst->iBodyLenUnitSize)))
	{
		printf("failed to get the entry contained length of PDU<%s>", pstTDRParser->szPkg);
		return -1;
	}	
	
	
       pstPDUInst->iUpUnit   =     pstPDU->iUpSize;
	pstPDUInst->iDownUnit =   pstPDU->iDownSize;
	pstPDUInst->iUnit	=	tdr_get_meta_size(pstTDRParserInst->pstPkg);

	if( pstPDUInst->iUpUnit <= 0 )
	{
		pstPDUInst->iUpUnit	=	pstPDUInst->iUnit;
	}
	if( pstPDUInst->iDownUnit <= 0 )
	{
		pstPDUInst->iDownUnit	=	pstPDUInst->iUnit;
	}

	
       pstPDUInst->iUnit              =     (pstPDUInst->iDownUnit > pstPDUInst->iUpUnit)?(pstPDUInst->iDownUnit):(pstPDUInst->iUpUnit);
	pstPDUInst->iUnit	=	(pstPDUInst->iUnit + 0x400 - 1)/0x400*0x400;
	pstPDUInst->iUpUnit      	=	(pstPDUInst->iUpUnit + 0x400 - 1)/0x400*0x400;
	
      
	pstTDRParserInst->iHeadLenMultiplex	=	pstTDRParser->iHeadLenMultiplex;
	pstTDRParserInst->iBodyLenMultiplex	=	pstTDRParser->iBodyLenMultiplex;
	pstTDRParserInst->iPkgLenMultiplex	=	pstTDRParser->iPkgLenMultiplex;

	pstPDUInst->pfnGetPkgLen = tconnd_get_pkglen;
	return 0;
}

int tconnd_init_pdulen_thttpparser(PDUINST* pstPDUInst, PDU* pstPDU)
{

	pstPDUInst->iUpUnit   =     pstPDU->iUpSize;
	pstPDUInst->iDownUnit =   pstPDU->iDownSize;
	if( pstPDUInst->iUpUnit <= 0 )
	{
           pstPDUInst->iUpUnit = TCONND_DEFAULT_UP_PKG_LEN;
	}
	if( pstPDUInst->iDownUnit <= 0 )
	{
           pstPDUInst->iDownUnit = TCONND_DEFAULT_DOWN_PKG_LEN;
	}

	pstPDUInst->iUnit = ( pstPDUInst->iDownUnit > pstPDUInst->iUpUnit )?( pstPDUInst->iDownUnit ):( pstPDUInst->iUpUnit );
	pstPDUInst->iUnit	=	(pstPDUInst->iUnit + 0x400 - 1)/0x400*0x400;
	pstPDUInst->iUpUnit	=	(pstPDUInst->iUpUnit + 0x400 - 1)/0x400*0x400;

	pstPDUInst->pfnGetPkgLen = tconnd_get_thttp_pkglen;
	
	return 0;
}

int tconnd_init_pdulen_noneparser(PDUINST* pstPDUInst, PDU* pstPDU)
{

	pstPDUInst->iUpUnit   =     pstPDU->iUpSize;
	pstPDUInst->iDownUnit =   pstPDU->iDownSize;
	if( pstPDUInst->iUpUnit <= 0 )
	{
           pstPDUInst->iUpUnit = TCONND_DEFAULT_UP_PKG_LEN;
	}
	if( pstPDUInst->iDownUnit <= 0 )
	{
           pstPDUInst->iDownUnit = TCONND_DEFAULT_DOWN_PKG_LEN;
	}

	pstPDUInst->iUnit = ( pstPDUInst->iDownUnit > pstPDUInst->iUpUnit )?( pstPDUInst->iDownUnit ):( pstPDUInst->iUpUnit );
	pstPDUInst->iUnit	=	(pstPDUInst->iUnit + 0x400 - 1)/0x400*0x400;
	pstPDUInst->iUpUnit	=	(pstPDUInst->iUpUnit + 0x400 - 1)/0x400*0x400;

	pstPDUInst->pfnGetPkgLen = tconnd_get_none_pkglen;
	
	return 0;
}


int tconnd_init_pdulen_qqparser(TDRINSTLIST* pstTDRInstList, PDUINST* pstPDUInst, PDU* pstPDU)
{
	LPTDRMETALIB pstPrefer=NULL;
	int iRet ;
	TDRSIZEINFO stSizeInfo;
	LPPDULENQQPARSERINST pstQQParserInst; 
	LPPDULENQQPARSER pstQQParser ;
	LPTDRMETALIB pstMetaLibQQ;
	LPTDRMETA pstMeta;
	int iSize;

	pstQQParserInst = &pstPDUInst->stLenParser.stQQParser;
	pstQQParser = &pstPDU->stLenParser.stQQParser;

	pstPDUInst->iUpUnit          = pstPDU->iUpSize;
	pstPDUInst->iDownUnit      = pstPDU->iDownSize;

	
	if( pstQQParser->szTDR[0] )
	{
		if( tconnd_find_lib(pstTDRInstList, pstQQParser->szTDR, &pstPrefer)<0 )
		{
                     printf("Error:tconnd_find_lib failed! TDR Name =%s\n",pstQQParser->szTDR);
			return -1;
		}
	
		if( pstQQParser->szSendPkg[0] )
		{
			if( tconnd_find_meta(pstTDRInstList, pstQQParser->szSendPkg, pstPrefer, &pstMeta)<0 )
			{
                            printf("Error:tconnd_find_meta failed! Send Meta Name =%s\n",pstQQParser->szSendPkg);
				return -1;
			}

			iSize	=	tdr_get_meta_size(pstMeta);

			if( pstPDUInst->iUpUnit <= 0 )
			{
				pstPDUInst->iUpUnit	=	iSize + sizeof(TPDUFRAME);
			}
			else if( pstPDUInst->iUpUnit < (iSize+sizeof(TPDUFRAME)) )
			{
                            printf("Warning:PDU MaxSize Exceeds Upsize,MaxSize = %d,Upsize = %d \n",(iSize+sizeof(TPDUFRAME)),pstPDUInst->iUpUnit);
			}
		}
		else
		{
                    printf("Send Meta Name For QQParser Incorrect\n");
		      return -1;
		}
		
		if( pstQQParser->szRecvPkg[0] )
		{
			if( tconnd_find_meta(pstTDRInstList, pstQQParser->szRecvPkg, pstPrefer, &pstMeta)<0 )
			{
                            printf("tconnd_find_meta failed!Recv Meta Name = %s\n",pstQQParser->szRecvPkg);
				return -1;
			}

			iSize	=	tdr_get_meta_size(pstMeta);

			if( pstPDUInst->iDownUnit <= 0 )
			{
				pstPDUInst->iDownUnit =	iSize + sizeof(TPDUFRAME);
			}
			else if( pstPDUInst->iDownUnit < (iSize+sizeof(TPDUFRAME)) )
			{
                            printf("Warning:PDU MaxSize Exceeds Downsize,MaxSize = %d,Downsize = %d \n",(iSize+sizeof(TPDUFRAME)),pstPDUInst->iDownUnit);
			}
		}
		else
		{
                    printf("Recv Meta Name For QQParser Incorrect\n");
		      return -1;
		}
	}
	else
	{
           printf("TDR Name For QQParser Incorrect\n");
	    return -1;
	}
       pstPDUInst->iUnit = ( pstPDUInst->iDownUnit > pstPDUInst->iUpUnit )?( pstPDUInst->iDownUnit ):( pstPDUInst->iUpUnit );
	pstPDUInst->iUnit	=	(pstPDUInst->iUnit + 0x400 - 1)/0x400*0x400;
	pstPDUInst->iUpUnit	=	(pstPDUInst->iUpUnit + 0x400 - 1)/0x400*0x400;

	pstMetaLibQQ		=	(LPTDRMETALIB)tqqapi_get_meta_data();
	pstQQParserInst->pstMetaHead	=	tdr_get_meta_by_name(pstMetaLibQQ, "TPDUHead");
	if (NULL == pstQQParserInst->pstMetaHead)
	{
		printf("failed to get meta by name %s", "TPDUHead");
		return -1;
	}
	pstQQParserInst->pstMetaGameSig	=	tdr_get_meta_by_name(pstMetaLibQQ, "TQQGameSig");
	if (NULL == pstQQParserInst->pstMetaGameSig)
	{
		printf("failed to get meta by name %s", "TQQGameSig");
		return -1;
	}
	pstQQParserInst->pstMetaSigForS2=	tdr_get_meta_by_name(pstMetaLibQQ, "TQQSigForS2");
	if (NULL == pstQQParserInst->pstMetaSigForS2)
	{
		printf("failed to get meta by name %s", "TQQSigForS2");
		return -1;
	}

	//add for unify auth
       pstQQParserInst->pstMetaUniSig=	tdr_get_meta_by_name(pstMetaLibQQ, "TQQUnifiedSig");
	if (NULL == pstQQParserInst->pstMetaUniSig)
	{
		printf("failed to get meta by name %s", "TQQUnifiedSig");
		return -1;
	}
	
	
	pstQQParserInst->pstMetaUniEncSig=	tdr_get_meta_by_name(pstMetaLibQQ, "TQQUnifiedEncrySig");
	if (NULL == pstQQParserInst->pstMetaUniEncSig)
	{
		printf("failed to get meta by name %s", "TQQUnifiedEncrySig");
		return -1;
	}

	//syn
	pstQQParserInst->pstMetaSyn=	tdr_get_meta_by_name(pstMetaLibQQ, "TPDUSynInfo");
	if (NULL == pstQQParserInst->pstMetaSyn)
	{
		printf("failed to get meta by name %s", "TPDUSynInfo");
		return -1;
	}

	
	pstQQParserInst->pstMetaUserIdent=	tdr_get_meta_by_name(pstMetaLibQQ, "TQQUserIdent");
	if (NULL == pstQQParserInst->pstMetaUserIdent)
	{
		printf("failed to get meta by name %s", "TQQUserIdent");
		return -1;
	}

	pstQQParserInst->pstMetaPDUIdent=	tdr_get_meta_by_name(pstMetaLibQQ, "TPDUIdentInfo");
	if (NULL == pstQQParserInst->pstMetaPDUIdent)
	{
		printf("failed to get meta by name %s", "TPDUIdentInfo");
		return -1;
	}
	
	

	iRet = tdr_sizeinfo_to_off_i(&stSizeInfo, pstQQParserInst->pstMetaHead, TDR_INVALID_INDEX, "Base.HeadLen");
	if (TDR_SUCCESS == iRet)
	{
		pstQQParserInst->iHeadLenNOff 		= stSizeInfo.iNOff;
		pstQQParserInst->iHeadLenUnitSize 	= stSizeInfo.iUnitSize;
	}
	else
	{
		return -1;
	}

	iRet = tdr_sizeinfo_to_off_i(&stSizeInfo, pstQQParserInst->pstMetaHead, TDR_INVALID_INDEX, "Base.BodyLen");
	if (TDR_SUCCESS == iRet)
	{
		pstQQParserInst->iBodyLenNOff 		= stSizeInfo.iNOff;
		pstQQParserInst->iBodyLenUnitSize 	= stSizeInfo.iUnitSize;
	}
	else
	{
		return -1;
	}


	pstPDUInst->pfnGetPkgLen = 	tconnd_get_qq_pkglen;

	return 0;
}


int tconnd_init_pduinstlist(TDRINSTLIST* pstTDRInstList, PDUINSTLIST* pstPDUInstList, PDULIST* pstPDUList)
{

	int iRet = 0;

	pstPDUInstList->iCount	=	0;
	while( pstPDUInstList->iCount<pstPDUList->iCount)
	{
		PDUINST* pstPDUInst;
		PDU* pstPDU;

		if( pstPDUInstList->iCount >= (int) (sizeof(pstPDUInstList->astInsts)/sizeof(pstPDUInstList->astInsts[0]) )  )
		{
			iRet = -1;
			break;
		}
		
		pstPDU = pstPDUList->astPDUs + pstPDUInstList->iCount;
		pstPDUInst = pstPDUInstList->astInsts + pstPDUInstList->iCount;
		memset(pstPDUInst, 0, sizeof(PDUINST));
		STRNCPY(pstPDUInst->szName, pstPDU->szName, sizeof(pstPDUInst->szName));

		pstPDUInst->pstPDU	=	pstPDU;

		pstPDUInst->iLenParsertype = pstPDU->iLenParsertype;
		switch(pstPDUInst->iLenParsertype)
		{
		case PDULENPARSERID_BY_TDR:
			iRet = tconnd_init_pdulen_tdrparser(pstTDRInstList, pstPDUInst, pstPDU);
			break;
		case PDULENPARSERID_BY_NULL:
			iRet = tconnd_init_pdulen_thttpparser(pstPDUInst, pstPDU);
			break;
		case PDULENPARSERID_BY_NONE:
			iRet = tconnd_init_pdulen_noneparser(pstPDUInst, pstPDU);
			break;
		case PDULENPARSERID_BY_QQ:
			iRet = tconnd_init_pdulen_qqparser(pstTDRInstList, pstPDUInst, pstPDU);
			break;
		default:
			iRet = -1;
			break;
		}/*switch(pstPDUInst->iLenParsertype)*/

		if(0 != iRet)
		{
			break;
		}
		pstPDUInstList->iCount++;
	}/*while( pstPDUInstList->iCount<pstPDUList->iCount)*/

	return iRet;
}

int tconnd_init_lisinstlist(LISINSTLIST* pstLisInstList, LISTENERLIST* pstListenerList)
{
	LISINST* pstLisInst;
	LISTENER* pstListener;

	pstLisInstList->iCount	=	0;

	while( pstLisInstList->iCount < pstListenerList->iCount )
	{
		pstLisInst	=	pstLisInstList->astInsts + pstLisInstList->iCount;
		pstListener	=	pstListenerList->astListeners + pstLisInstList->iCount;

		memcpy(pstLisInst->szName, pstListener->szName, sizeof(pstLisInst->szName));

		pstLisInst->iRef	=	0;

		pstLisInst->pstListener	=	pstListener;

		pstLisInstList->iCount++;
	}

	return 0;
}

int tconnd_fini_lisinstlist(LISINSTLIST* pstLisInstList)
{
	LISINST* pstLisInst;
	int i;

	for(i=0; i<pstLisInstList->iCount; i++)
	{
		pstLisInst	=	pstLisInstList->astInsts + i;
		pstLisInst->iRef	=	0;
	}

	return 0;
}

int tconnd_init_serinstlist(SERINSTLIST* pstSerInstList, SERIALIZERLIST* pstSerializerList)
{
	SERINST* pstSerInst;
	SERIALIZER* pstSerializer;

	pstSerInstList->iCount	=	0;

	while( pstSerInstList->iCount < pstSerializerList->iCount )
	{
		pstSerInst	=	pstSerInstList->astInsts + pstSerInstList->iCount;
		pstSerializer=	pstSerializerList->astSerializers + pstSerInstList->iCount;

		tbus_addr_aton(pstSerializer->szUrl, 
(TBUSADDR * )&pstSerInst->iDst);
		//pstSerInst->iDst	=	(int)inet_addr(pstSerializer->szUrl);

		memcpy(pstSerInst->szName, pstSerializer->szName, sizeof(pstSerInst->szName));

		pstSerInst->iRef	=	0;
		pstSerInst->pstSerializer=	pstSerializer;

		pstSerInstList->iCount++;
	}

	return 0;
}

int tconnd_fini_serinstlist(SERINSTLIST* pstSerInstList)
{
	SERINST* pstSerInst;
	int i;

	for(i=0; i<pstSerInstList->iCount; i++)
	{
		pstSerInst	=	pstSerInstList->astInsts + i;

		pstSerInst->iDst	=	0;

		pstSerInst->iRef	=	0;
	}

	return 0;
}

int tconnd_load_conffile(TCONND* pstConnd, LPTDRMETA pstMeta, const char* pszPath)
{
	TDRDATA stInput;

	stInput.pszBuff	=	(char*)pstConnd;
	stInput.iBuff	=	(int) sizeof(*pstConnd);

	if( !pstMeta || !pszPath )
		return -1;

	if( tdr_input_file(pstMeta, &stInput, pszPath, 0, 0)<0 )
		return -1;

	return 0;
}

int tconnd_init_transinstlist(TRANSINSTLIST* pstTransInstList, TCONND* pstConnd, LISINSTLIST* pstLisInstList, SERINSTLIST* pstSerInstList)
{
	NETTRANSLIST* pstNetTransList;
	NETTRANS* pstNetTrans;
	TRANSINST* pstTransInst;
	LISTENERLIST* pstListenerList;
	SERIALIZERLIST* pstSerializerList;
	TDRLIST* pstTDRList;
	PDULIST* pstPDUList;
	int iIsFind;
	int i;
	int j;

	pstNetTransList		=	&pstConnd->stNetTransList;
	pstTDRList			=	&pstConnd->stTDRList;
	pstPDUList			=	&pstConnd->stPDUList;
	pstListenerList		=	&pstConnd->stListenerList;
	pstSerializerList	=	&pstConnd->stSerializerList;

	if( pstNetTransList->iCount>(int)(sizeof(pstTransInstList->astInsts)/sizeof(pstTransInstList->astInsts[0])) )
	{
		return -1;
	}

	/* first, check the tdrs. */
	for(i=0; i<pstNetTransList->iCount; i++)
	{
		pstNetTrans	=	pstNetTransList->astNetTrans + i;
		pstTransInst=	pstTransInstList->astInsts + i;

		pstTransInst->iWaitQueueHead	=	-1;
		pstTransInst->iWaitQueueTail	=	-1;
		pstTransInst->uiTokenAlloc	=	0;
		pstTransInst->uiTokenPass	=	0;

		pstTransInst->iSendCheckInterval=	TCONND_DEF_SENDCHECK_INTERVAL;
		pstTransInst->iPrevSendFailed	=	0;
		pstTransInst->iSendFailed	=	0;
		pstTransInst->tLastSendCheck	=	0;

		pstTransInst->iRecvCheckInterval=	TCONND_DEF_RECVCHECK_INTERVAL;
		pstTransInst->iRecvPkg		=	0;
		pstTransInst->iRecvByte		=	0;
		pstTransInst->tLastRecvCheck	=	0;

		pstTransInst->iConnPermit	=	pstNetTrans->stConnLimit.iPermit;
		pstTransInst->iConnPermitLow	=	pstNetTrans->stConnLimit.iPermit/2;
		pstTransInst->iConnPermitHigh	=	pstNetTrans->stConnLimit.iPermit;
		pstTransInst->iConnMaxSpeed	=	0;

		pstTransInst->iPkgMaxSpeed	=	pstNetTrans->stTransLimit.iPkgSpeed;;
		pstTransInst->iPkgPermit	=	pstTransInst->iPkgMaxSpeed*pstTransInst->iRecvCheckInterval;
		pstTransInst->iPkgPermitLow	=	pstTransInst->iPkgPermit/2;
		pstTransInst->iPkgPermitHigh	=	pstTransInst->iPkgPermit;

		pstTransInst->iByteMaxSpeed	=	pstNetTrans->stTransLimit.iByteSpeed;
		pstTransInst->iBytePermit	=	pstTransInst->iByteMaxSpeed*pstTransInst->iRecvCheckInterval;
		pstTransInst->iBytePermitLow	=	pstTransInst->iBytePermit/2;
		pstTransInst->iBytePermitHigh	=	pstTransInst->iBytePermit;

		iIsFind		=	0;

		for(j=0; j<pstPDUList->iCount; j++)
		{
			if( !strcasecmp(pstNetTrans->szPDU, pstPDUList->astPDUs[j].szName) )
			{
				pstTransInst->iPDULoc	=	j;
				iIsFind	=	1;
				break;
			}
		}

		if( !iIsFind )
		{
			printf("Error: Find PDU \'%s\' failed.\n", pstNetTrans->szPDU);
			return -1;
		}

		iIsFind	=	0;

		pstTransInst->iLisCount	=	0;

		for(j=0; j<pstListenerList->iCount; j++)
		{
			if( !strcasecmp(pstNetTrans->szListener, pstListenerList->astListeners[j].szName) )
			{
				iIsFind	=	1;

				if( pstTransInst->iLisCount>=(int)(sizeof(pstTransInst->aiLisLoc)/sizeof(pstTransInst->aiLisLoc[0])) )
					break;

				if( pstLisInstList->astInsts[j].iRef>=1 )
				{
					printf("Error: Listener \'%s\' be used more than 1 times.\n", pstNetTrans->szListener);
					return -1;
				}

				pstLisInstList->astInsts[j].iRef++;

				pstTransInst->aiLisLoc[pstTransInst->iLisCount]   =	j;
				pstTransInst->iLisCount++;
			}
		}

		if( !iIsFind )
		{
			printf("Error: Find Listener \'%s\' failed.\n", pstNetTrans->szListener);
			return -1;
		}


		iIsFind	=	0;

		pstTransInst->iSerCount	=	0;

		for(j=0; j<pstSerializerList->iCount; j++)
		{
			if( !strcasecmp(pstNetTrans->szSerializer, pstSerializerList->astSerializers[j].szName) )
			{
				iIsFind	=	1;

				if( pstTransInst->iSerCount>=(int)(sizeof(pstTransInst->aiSerLoc)/sizeof(pstTransInst->aiSerLoc[0])) )
				{
					break;
				}

				pstSerInstList->astInsts[j].iRef++;

				pstTransInst->aiSerLoc[pstTransInst->iSerCount]		=	j;
				pstTransInst->iSerCount++;
			}
		}

		if( !iIsFind )
		{
			printf("Error: Find Serializer \'%s\' failed.\n", pstNetTrans->szSerializer);
			return -1;
		}

		if( pstNetTrans->szLisViewer[0] )
		{
			iIsFind	=	0;
	
			for(j=0; j<pstSerializerList->iCount; j++)
			{
				if( !strcasecmp(pstNetTrans->szLisViewer, pstSerializerList->astSerializers[j].szName) )
				{
					iIsFind	=	1;
					pstTransInst->iLisViewerLoc	=	j;
					break;
				}
			}
	
			if( !iIsFind )
			{
				printf("Error: Find LisViewer\'%s\' failed.\n", pstNetTrans->szLisViewer);
				return -1;
			}
		}
		else
		{
			pstTransInst->iLisViewerLoc	=	-1;
		}


		if( pstNetTrans->szSerViewer[0] )
		{
			iIsFind	=	0;

			for(j=0; j<pstSerializerList->iCount; j++)
			{
				if( !strcasecmp(pstNetTrans->szSerViewer, pstSerializerList->astSerializers[j].szName) )
				{
					iIsFind	=	1;
					pstTransInst->iSerViewerLoc	=	j;
					break;
				}
			}
	
			if( !iIsFind )
			{
				printf("Error: Find SerViewer\'%s\' failed.\n", pstNetTrans->szSerViewer);
				return -1;
			}
		}
		else
		{
			pstTransInst->iSerViewerLoc	=	-1;
		}

		//pstTransInst->iLoadRatio	=	pstNetTrans->iLoadRatio;
	}

	pstTransInstList->iCount	=	pstNetTransList->iCount;

	return 0;

}

int tconnd_fini_confinst(CONFINST* pstConfInst)
{
	assert(pstConfInst);

	return 0;
}

int tconnd_init_confinst(CONFINST* pstConfInst, TCONND* pstConnd)
{
	if( tconnd_init_tdrinstlist(&pstConfInst->stTDRInstList, &pstConnd->stTDRList)<0 )
	{
              printf("tconnd_init_tdrinstlist failed\n");
		return -1;
	}
	if( tconnd_init_pduinstlist(&pstConfInst->stTDRInstList, &pstConfInst->stPDUInstList, &pstConnd->stPDUList)<0 )
	{
              printf("tconnd_init_pduinstlist failed\n");
		return -1;
	}
	if( tconnd_init_lisinstlist(&pstConfInst->stLisInstList, &pstConnd->stListenerList)<0 )
	{
              printf("tconnd_init_lisinstlist failed\n");
		return -1;
	}
	if( tconnd_init_serinstlist(&pstConfInst->stSerInstList, &pstConnd->stSerializerList)<0 )
	{     
	       printf("tconnd_init_serinstlist failed\n");
		return -1;
	} 
	if( tconnd_init_transinstlist(&pstConfInst->stTransInstList, pstConnd, &pstConfInst->stLisInstList, &pstConfInst->stSerInstList)<0 )
	{
              printf("tconnd_init_transinstlist failed\n");  
		return -1;
	}

	return 0;
}

int tconnd_init_tconndrun(TAPPCTX* pstAppCtx, TCONND* pstConnd)
{
	int i = 0;
	TCONNDRUN_CUMULATE *pstRunCum;

	
       pstRunCum = (TCONNDRUN_CUMULATE *)pstAppCtx->stRunDataCumu.pszBuff;
	memset(pstRunCum, 0, sizeof(*pstRunCum));

	for(i=0; i<pstConnd->stListenerList.iCount;i++)
	{
		strcpy(pstRunCum->stListenerRunList.astListeners[i].szName, pstConnd->stListenerList.astListeners[i].szName);
	}

	for(i=0; i<pstConnd->stSerializerList.iCount;i++)
	{
		strcpy(pstRunCum->stSerializerRunList.astSerializers[i].szName, pstConnd->stSerializerList.astSerializers[i].szName);
	}

	for(i=0; i<pstConnd->stNetTransList.iCount;i++)
	{
		strcpy(pstRunCum->stNetTransRunList.astNetTrans[i].szName, pstConnd->stNetTransList.astNetTrans[i].szName);
	}

	pstRunCum->stNetTransRunList.iCount	=	pstConnd->stNetTransList.iCount;
	pstRunCum->stListenerRunList.iCount	=	pstConnd->stListenerList.iCount;
	pstRunCum->stSerializerRunList.iCount	=	pstConnd->stSerializerList.iCount;

	return 0;
}
