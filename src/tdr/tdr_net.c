/**
*
* @file     tdr_net.c 
* @brief    TDR元数据网络交换消息编解码模块
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-04-28 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <assert.h>
#include <wchar.h>


#include "tdr_os.h"
#include "tdr/tdr_net.h"
#include "tdr_metalib_kernel_i.h"
#include "tdr_define_i.h"
#include "tdr/tdr_error.h"
#include "tdr_ctypes_info_i.h"
#include "tdr_net_i.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif


#define TDR_CALC_META_SIZEINFO_TARGET(pstStackTop, pszNet)			\
{																	\
	if (0 > pstStackTop->iMetaSizeInfoOff)							\
	{																\
		pstStackTop->pszMetaSizeInfoTarget = pszNet;				\
		pszNet -= pstStackTop->iMetaSizeInfoOff;					\
		pstStackTop->pszNetBase = pszNet;							\
	}else															\
	{																\
		pstStackTop->pszNetBase = pszNet;							\
		pstStackTop->pszMetaSizeInfoTarget = pstStackTop->pszNetBase + pstStackTop->iMetaSizeInfoOff ; \
	}																					\
}



#define TDR_GET_VERSION_INDICATOR_NET(a_pszNetBase, a_pszNetEnd,pstCurMeta, iCutOffVersion, a_iVersion) \
{																   \
if (0 < pstCurMeta->stVersionIndicator.iUnitSize)				\
{																\
	tdr_longlong lval;											\
	char *pszPtr = a_pszNetBase + pstCurMeta->stVersionIndicator.iNOff;	\
	if ((a_pszNetEnd - pszPtr) >= pstCurMeta->stVersionIndicator.iUnitSize)\
	{																	\
		TDR_GET_INT(lval, pstCurMeta->stVersionIndicator.iUnitSize, pszPtr);	\
		iCutOffVersion = TDR_MIN((int)lval, a_iVersion);							\
	}else														\
	{															\
		iCutOffVersion = a_iVersion;							\
	}															\
}else															\
{																\
	iCutOffVersion = a_iVersion;							\
}																\
}

#define TDR_PUSH_STACK_ITEM(a_pstStackTop,  a_pstTypeMeta, a_pstEntry, a_iCount, a_idx, a_pszHostStart, a_pszNetStart) \
{																						\
	a_pstStackTop->pstMeta = a_pstTypeMeta;												\
	a_pstStackTop->iCount = a_iCount;													\
	a_pstStackTop->idxEntry = a_idx;													\
	a_pstStackTop->pszHostBase = a_pszHostStart;										\
	a_pstStackTop->pszNetBase = a_pszNetStart;											\
	if (a_pstTypeMeta->stSizeType.iUnitSize > 0)										\
	{																					\
		a_pstStackTop->iMetaSizeInfoUnit = a_pstTypeMeta->stSizeType.iUnitSize;			\
		if (TDR_INVALID_INDEX != a_pstTypeMeta->stSizeType.idxSizeType)					\
		{																				\
			a_pstStackTop->pszMetaSizeInfoTarget = a_pszNetStart;						\
			a_pstStackTop->iMetaSizeInfoOff = - a_pstStackTop->iMetaSizeInfoUnit;		\
			a_pszNetStart += a_pstStackTop->iMetaSizeInfoUnit;							\
		}else																			\
		{																				\
			a_pstStackTop->pszMetaSizeInfoTarget = a_pszNetStart + a_pstTypeMeta->stSizeType.iNOff;		\
			a_pstStackTop->iMetaSizeInfoOff = a_pstTypeMeta->stSizeType.iNOff;			\
		}																				\
	}else if (a_pstEntry->stSizeInfo.iUnitSize > 0)										\
	{																					\
		a_pstStackTop->iMetaSizeInfoUnit = a_pstEntry->stSizeInfo.iUnitSize;			\
		if (TDR_INVALID_INDEX != a_pstEntry->stSizeInfo.idxSizeType)					\
		{																				\
			a_pstStackTop->pszMetaSizeInfoTarget = a_pszNetStart;						\
			a_pszNetStart += a_pstStackTop->iMetaSizeInfoUnit;							\
			a_pstStackTop->iMetaSizeInfoOff = - a_pstStackTop->iMetaSizeInfoUnit;		\
		}else  if (a_pstEntry->stSizeInfo.iNOff >= pstEntry->iNOff)						\
		{																				\
			a_pstStackTop->iMetaSizeInfoOff = a_pstEntry->stSizeInfo.iNOff - a_pstEntry->iNOff;	\
			a_pstStackTop->pszMetaSizeInfoTarget = a_pszNetStart + a_pstStackTop->iMetaSizeInfoOff ; \
		}else																			\
		{																				\
			a_pstStackTop->pszMetaSizeInfoTarget = (a_pstStackTop - 1)->pszNetBase + a_pstEntry->stSizeInfo.iNOff; \
		}																				\
	}else																				\
	{																					\
		a_pstStackTop->iMetaSizeInfoUnit = 0;											\
	}																					\
	a_pstStackTop->pszNetBase = a_pszNetStart;											\
}



int tdr_hton(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstNet, INOUT LPTDRDATA a_pstHost, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	int iChange; 
	LPTDRMETAENTRY pstEntry;
	int iArrayRealCount ;	
	char *pszNetStart;
	char *pszNetEnd;
	char *pszHostStart;
	char *pszHostEnd;
	int idxSubEntry;		

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_pstNet);
	assert(NULL != a_pstNet->pszBuff);
	assert(0 < a_pstNet->iBuff);
	assert(NULL != a_pstHost);
	assert(NULL != a_pstHost->pszBuff);
	assert(0 < a_pstHost->iBuff);
	assert(TDR_TYPE_UNION != a_pstMeta->iType);
	assert(0 < a_pstMeta->iEntriesNum);*/
	if ((NULL == a_pstMeta)||(TDR_TYPE_UNION == a_pstMeta->iType)||(NULL == a_pstNet)||(NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}
	if ((NULL == a_pstNet->pszBuff)||(0 >= a_pstNet->iBuff)||
		(NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	if (0 == a_iVersion)
	{
		a_iVersion = TDR_MAX_VERSION;
	}
		
	
	pszNetStart = a_pstNet->pszBuff;
	pszNetEnd = a_pstNet->pszBuff + a_pstNet->iBuff;
	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	pstCurMeta = a_pstMeta;
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->pszHostBase = a_pstHost->pszBuff;
	pstStackTop->pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	if (0 < pstCurMeta->stSizeType.iUnitSize)
	{
		pstStackTop->iMetaSizeInfoUnit = pstCurMeta->stSizeType.iUnitSize;
		if( TDR_INVALID_INDEX != pstCurMeta->stSizeType.idxSizeType )
		{
			pstStackTop->pszMetaSizeInfoTarget = pszNetStart;
			pszNetStart += pstStackTop->iMetaSizeInfoUnit;			
		}else
		{
			pstStackTop->pszMetaSizeInfoTarget = pszNetStart + pstCurMeta->stSizeType.iNOff;
		}
	}else
	{
		pstStackTop->iMetaSizeInfoUnit = 0;
		pstStackTop->iMetaSizeInfoOff =	0;
		pstStackTop->pszMetaSizeInfoTarget = NULL;
	}
	pstStackTop->pszNetBase = pszNetStart;
	pstStackTop->iChange = 1;
	pstStackTop->iCode = 0;
	
	TDR_GET_VERSION_INDICATOR(iRet, a_pstHost->pszBuff, pszHostEnd, pstCurMeta, pstStackTop->iCutOffVersion, a_iVersion);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		a_pstHost->iBuff = 0;
		a_pstNet->iBuff = 0;
		return iRet;
	}
	if (pstCurMeta->iBaseVersion > pstStackTop->iCutOffVersion)
	{
		a_pstHost->iBuff = 0;
		a_pstNet->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}
	iStackItemCount = 1;


	iChange = 0;
	while (0 < iStackItemCount)
	{
		if (0 != iChange)
		{
			iChange = 0;
			if (0 < pstStackTop->iMetaSizeInfoUnit)
			{
				tdr_longlong lLen = pszNetStart - pstStackTop->pszNetBase;
				TDR_SET_INT_NET(pstStackTop->pszMetaSizeInfoTarget, pstStackTop->iMetaSizeInfoUnit,	lLen);	
				
			}

			
			if (0 < pstStackTop->iCount)											
			{		
				(pstStackTop-1)->iCode += pstCurMeta->iHUnitSize;
				if (pszNetStart > pszNetEnd)
				{
					iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);
					break;
				}
				if (0 < pstStackTop->iMetaSizeInfoUnit)
				{
					TDR_CALC_META_SIZEINFO_TARGET(pstStackTop, pszNetStart);
				}				
			}	
		}/*if (0 != iChange)*/


		if (0 >= pstStackTop->iCount)
		{/*当前元数据数组已经处理完毕*/
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstStackTop->iCode += (pstStackTop+1)->iCode;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		pstStackTop->iCode = pstEntry->iHOff;
	

		if (pstEntry->iVersion > pstStackTop->iCutOffVersion)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		/*指针不编码*/
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
		pszHostEnd = pstStackTop->pszHostEnd;
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			pszHostStart = *(char **)pszHostStart;
			pszHostEnd = pszHostStart + pstEntry->iNRealSize;
		}
		
		

		/*取出此entry的数组计数信息*/	
		TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, pstStackTop->iCutOffVersion); 
		if ((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount))
		{/*实际数目为负数或比数组最大长度要大，则无效*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}
		if (0 >= iArrayRealCount)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}


		/*简单数据类型*/
		if (TDR_TYPE_COMPOSITE < pstEntry->iType)
		{
			char *pcTmp = pszHostStart;
			TDR_PACK_SIMPLE_TYPE_ENTRY(iRet, pstEntry, iArrayRealCount, pszNetStart, pszNetEnd, pszHostStart, pszHostEnd);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			pstStackTop->iCode += (pszHostStart - pcTmp);
			
			continue;
		}

		/*复合数据类型*/		
		if (TDR_STACK_SIZE <=  iStackItemCount)
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
			break;
		}

		if (TDR_TYPE_UNION == pstEntry->iType)
		{
			TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, pstStackTop->iCutOffVersion, pstCurMeta, idxSubEntry);
			if(NULL == pstCurMeta) 
			{																									
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				continue;																				
			}	
		}else
		{
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			idxSubEntry = 0;
		}

		/*递归进行结构体内部进行处理*/
		iStackItemCount++;
		pstStackTop++;
		TDR_GET_VERSION_INDICATOR(iRet, pszHostStart, pszHostEnd, pstCurMeta, pstStackTop->iCutOffVersion, (pstStackTop-1)->iCutOffVersion);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}
		if (pstCurMeta->iBaseVersion > pstStackTop->iCutOffVersion)
		{
			iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
			break;
		}
		TDR_PUSH_STACK_ITEM(pstStackTop, pstCurMeta, pstEntry, iArrayRealCount, idxSubEntry,
			pszHostStart, pszNetStart);
		pstStackTop->pszHostEnd = pszHostEnd;
		pstStackTop->iCode = 0;
		pstStackTop->iChange = 1;
	}/*while (0 < iStackItemCount)*/

	a_pstHost->iBuff = stStack[0].iCode;
	if (TDR_ERR_IS_ERROR(iRet))
	{
		for(iChange= 1; iChange < iStackItemCount; iChange++)
		{
			a_pstHost->iBuff += stStack[iChange].iCode;
		
		}
	}
	a_pstNet->iBuff = pszNetStart - a_pstNet->pszBuff;

	return iRet;		 
}

int tdr_ntoh(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, INOUT LPTDRDATA a_pstNet, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	int iChange;
	char *pszNetStart;
	char *pszNetEnd;
	char *pszHostStart;
	char *pszHostEnd;

	int idxSubEntry;
	
	/*assert(NULL != a_pstMeta);
	assert(NULL != a_pstNet);
	assert(NULL != a_pstNet->pszBuff);
	assert(0 < a_pstNet->iBuff);
	assert(NULL != a_pstHost);
	assert(NULL != a_pstHost->pszBuff);
	assert(0 < a_pstHost->iBuff);
	assert(TDR_TYPE_UNION != a_pstMeta->iType);
	assert(0 < a_pstMeta->iEntriesNum);*/
	if ((NULL == a_pstMeta)||(TDR_TYPE_UNION == a_pstMeta->iType)||(NULL == a_pstNet)||(NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}
	if ((NULL == a_pstNet->pszBuff)||(0 >= a_pstNet->iBuff)||
		(NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	if (0 == a_iVersion)
	{
		a_iVersion = TDR_MAX_VERSION;
	}
	


	pszNetStart = a_pstNet->pszBuff;
	pszNetEnd = a_pstNet->pszBuff + a_pstNet->iBuff;
	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;

	pstCurMeta = a_pstMeta;
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->pszNetBase = pszNetStart;
	pstStackTop->pszHostBase = a_pstHost->pszBuff;
	pstStackTop->pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	if (0 < pstCurMeta->stSizeType.iUnitSize)
	{
		pstStackTop->iMetaSizeInfoUnit = pstCurMeta->stSizeType.iUnitSize;
		if( TDR_INVALID_INDEX != pstCurMeta->stSizeType.idxSizeType )
		{
			pstStackTop->pszMetaSizeInfoTarget = pszNetStart;
			pszNetStart += pstStackTop->iMetaSizeInfoUnit;			
		}else
		{
			pstStackTop->pszMetaSizeInfoTarget = pszNetStart + pstCurMeta->stSizeType.iNOff;
		}
	}else
	{
		pstStackTop->iMetaSizeInfoUnit = 0;
		pstStackTop->iMetaSizeInfoOff =	0;
		pstStackTop->pszMetaSizeInfoTarget = NULL;
	}
	TDR_GET_VERSION_INDICATOR_NET(pszNetStart, pszNetEnd, pstCurMeta, pstStackTop->iCutOffVersion, a_iVersion);
	if (pstCurMeta->iBaseVersion > pstStackTop->iCutOffVersion)
	{
		a_pstHost->iBuff = 0;
		a_pstNet->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}
	iStackItemCount = 1;
	pstStackTop->iCode = 0;
	pstStackTop->iChange = 1;


	iChange = 0;
	while (0 < iStackItemCount)
	{
		LPTDRMETAENTRY pstEntry;
		int iArrayRealCount ;
		tdr_longlong iLen;

		if (0 != iChange)
		{
			if (0 < pstStackTop->iMetaSizeInfoUnit)
			{
				TDR_GET_INT_NET(iLen, pstStackTop->iMetaSizeInfoUnit, pstStackTop->pszMetaSizeInfoTarget);
				pszNetStart	= pstStackTop->pszNetBase + iLen;
				if ( pszNetStart > pszNetEnd )
				{
					iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);
					break;
				}
				//TDR_CALC_META_SIZEINFO_TARGET(pstStackTop, pszNetStart);
			}
			
			iChange	= 0;
			
			if (0 < pstStackTop->iCount)
			{
				(pstStackTop-1)->iCode += pstCurMeta->iHUnitSize;
				if (0 < pstStackTop->iMetaSizeInfoUnit)
				{
					TDR_CALC_META_SIZEINFO_TARGET(pstStackTop, pszNetStart);
				}					
			}
		}/*if (0 != iChange)*/

		if (0 >= pstStackTop->iCount)
		{/*当前元数据数组已经处理完毕*/
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{		
				pstStackTop->iCode += (pstStackTop+1)->iCode;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		pstStackTop->iCode = pstEntry->iHOff;
		

		/*指针不编码*/		
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}	

		/*取出此entry的数组计数信息*/	
		TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, pstStackTop->iCutOffVersion); 
		if ((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount))
		{/*实际数目为负数或比数组最大长度要大，则无效*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}
		if (0 >= iArrayRealCount)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
		pszHostEnd = pstStackTop->pszHostEnd;
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			pszHostStart = *(char **)pszHostStart;
			pszHostEnd = pszHostStart + pstEntry->iNRealSize;
		}

		if (pstEntry->iVersion > pstStackTop->iCutOffVersion)
		{
			char *pcTmp = pszHostStart;
			TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, iArrayRealCount);
			pstStackTop->iCode += (pszHostStart - pcTmp);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		/*简单数据类型*/
		if (TDR_TYPE_COMPOSITE < pstEntry->iType)
		{
			char *pcTmp = pszHostStart;
			TDR_UNPACK_SIMPLE_TYPE_ENTRY(iRet, pstEntry, iArrayRealCount, pszNetStart, pszNetEnd, pszHostStart, pszHostEnd);
			pstStackTop->iCode += (pszHostStart - pcTmp);
			
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		/*复合数据类型*/		
		if (TDR_STACK_SIZE <=  iStackItemCount)
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
			break;
		}

		if (TDR_TYPE_UNION == pstEntry->iType)
		{
			TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, pstStackTop->iCutOffVersion, pstCurMeta, idxSubEntry);
			if(NULL == pstCurMeta) 
			{																									
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				continue;																				
			}	
		}else
		{
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			idxSubEntry = 0;
		}

		/*递归进行结构体内部进行处理*/
		iStackItemCount++;
		pstStackTop++;
		TDR_GET_VERSION_INDICATOR_NET(pszNetStart, pszNetEnd, pstCurMeta, pstStackTop->iCutOffVersion, (pstStackTop-1)->iCutOffVersion);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}
		if (pstCurMeta->iBaseVersion > pstStackTop->iCutOffVersion)
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
			break;
		}
		TDR_PUSH_STACK_ITEM(pstStackTop, pstCurMeta, pstEntry, iArrayRealCount, idxSubEntry,
			pszHostStart, pszNetStart);
		pstStackTop->pszHostEnd = pszHostEnd;
		pstStackTop->iCode = 0;
		pstStackTop->iChange = 1;
	}/*while (0 < iStackItemCount)*/


	a_pstHost->iBuff = stStack[0].iCode;
	if (TDR_ERR_IS_ERROR(iRet))
	{
		for(iChange= 1; iChange < iStackItemCount; iChange++)
		{
			a_pstHost->iBuff += stStack[iChange].iCode;
		
		}
	}
	
	
	a_pstNet->iBuff = pszNetStart - a_pstNet->pszBuff;

	return iRet;		
}

int tdr_pack_union_entry_i(IN LPTDRMETA a_pstMeta, IN int a_idxEntry, INOUT LPTDRDATA a_pstNetData, IN LPTDRDATA a_pstHostData, IN int a_iVersion)
{
	LPTDRMETAENTRY pstSelEntry;
	char *pch;
	int iLeftLen;
	int j;
	int iRet = TDR_SUCCESS;
	char *pszHostEnd;
	char *pszNetEnd;
	char *pszMetaBase;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstNetData);
	assert(NULL != a_pstNetData->pszBuff);
	assert(0 < a_pstNetData->iBuff);
	assert(NULL != a_pstHostData);
	assert(NULL != a_pstHostData->pszBuff);
	assert(0 < a_pstHostData->iBuff);
	assert(TDR_TYPE_UNION == a_pstMeta->iType);
	assert((0 <= a_idxEntry) && (a_idxEntry < a_pstMeta->iEntriesNum));


	pstSelEntry = a_pstMeta->stEntries + a_idxEntry;
	pch = a_pstNetData->pszBuff;
	iLeftLen = a_pstNetData->iBuff;
	pszNetEnd = a_pstNetData->pszBuff + a_pstNetData->iBuff;
	pszHostEnd = a_pstHostData->pszBuff + a_pstHostData->iBuff;
	pszMetaBase = a_pstHostData->pszBuff ;
	if (TDR_TYPE_STRUCT == pstSelEntry->iType)
	{
		TDRDATA stNetInfo;
		TDRDATA stHostInfo;
		LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
		LPTDRMETA pstType = TDR_PTR_TO_META(pstLib, pstSelEntry->ptrMeta);
		for (j = 0; j < pstSelEntry->iCount; j++)
		{
			stNetInfo.iBuff = iLeftLen;
			stNetInfo.pszBuff = pch;
			stHostInfo.pszBuff = pszMetaBase + j * pstSelEntry->iHUnitSize;
			stHostInfo.iBuff = pszHostEnd - stHostInfo.pszBuff;
			iRet = tdr_hton(pstType, &stNetInfo, &stHostInfo, a_iVersion);
			if (0 != iRet)
			{
				break;
			}
			iLeftLen -= stNetInfo.iBuff;
			pch += stNetInfo.iBuff;
		}
	}else
	{
		TDR_PACK_SIMPLE_TYPE_ENTRY(iRet, pstSelEntry, pstSelEntry->iCount, pch, pszNetEnd, pszMetaBase, pszHostEnd);				
	}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

	a_pstNetData->iBuff = pch - a_pstNetData->pszBuff;

	return iRet;
}


int tdr_unpack_union_entry_i(IN LPTDRMETA a_pstMeta, IN int a_idxEntry, IN LPTDRDATA a_pstHostData, INOUT LPTDRDATA a_pstNetData, IN int a_iVersion)
{
	LPTDRMETAENTRY pstSelEntry;
	char *pch;
	int iLeftLen;
	char *pszMetaBase;
	int j;
	int iRet = TDR_SUCCESS;
	char *pszHostEnd;
	char *pszNetEnd;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstNetData);
	assert(NULL != a_pstNetData->pszBuff);
	assert(0 < a_pstNetData->iBuff);
	assert(NULL != a_pstHostData);
	assert(NULL != a_pstHostData->pszBuff);
	assert(0 < a_pstHostData->iBuff);
	assert(TDR_TYPE_UNION == a_pstMeta->iType);
	assert((0 <= a_idxEntry) && (a_idxEntry < a_pstMeta->iEntriesNum));

	
	pstSelEntry = a_pstMeta->stEntries + a_idxEntry;
	pch = a_pstNetData->pszBuff;
	iLeftLen = a_pstNetData->iBuff;
	pszNetEnd = a_pstNetData->pszBuff + a_pstNetData->iBuff;
	pszHostEnd = a_pstHostData->pszBuff + a_pstHostData->iBuff;	
	pszMetaBase = a_pstHostData->pszBuff ;
	if (TDR_TYPE_STRUCT == pstSelEntry->iType)
	{
		TDRDATA stNetInfo;
		TDRDATA stHostInfo;
		LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
		LPTDRMETA pstType = TDR_PTR_TO_META(pstLib, pstSelEntry->ptrMeta);
		for (j = 0; j < pstSelEntry->iCount; j++)
		{
			stNetInfo.iBuff = iLeftLen;
			stNetInfo.pszBuff = pch;
			stHostInfo.pszBuff = pszMetaBase + j * pstSelEntry->iHUnitSize;;
			stHostInfo.iBuff = pszHostEnd - stHostInfo.pszBuff;
			iRet = tdr_ntoh(pstType, &stHostInfo, &stNetInfo, a_iVersion);
			if (0 != iRet)
			{
				break;
			}
			iLeftLen -= stNetInfo.iBuff;
			pch += stNetInfo.iBuff;
		}
	}else
	{
		TDR_UNPACK_SIMPLE_TYPE_ENTRY(iRet, pstSelEntry, pstSelEntry->iCount, pch, pszNetEnd, pszMetaBase, pszHostEnd);
	}
	a_pstNetData->iBuff = pch - a_pstNetData->pszBuff;

	return iRet;
}

