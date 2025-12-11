 /**
*
* @file     tdr_date_initialize.c 
* @brief    TDR数据初始化/正则化
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-06-05 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <assert.h>
#include <string.h>
#include "tdr/tdr_error.h"
#include "tdr/tdr_operate_data.h"
#include "tdr_os.h"
#include "tdr_define_i.h"
#include "tdr_ctypes_info_i.h"
#include "tdr_metalib_kernel_i.h"
#include "tdr_net_i.h"
#include "tdr/tdr_metalib_manage.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

TDR_API int tdr_init(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN int a_iVersion)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	int iChange; 
	char *pszHostStart;
	char *pszHostEnd;
	int iCount;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_pstHost);
	assert(NULL != a_pstHost->pszBuff);
	assert(0 < a_pstHost->iBuff);*/
	if ((NULL == a_pstMeta)||(NULL == a_pstHost)||(NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	if (0 == a_iVersion)
	{
		a_iVersion = TDR_MAX_VERSION;
	}
	if (a_pstMeta->iBaseVersion > a_iVersion)
	{
		a_pstHost->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}
	

	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	pstCurMeta = a_pstMeta;
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->pszHostBase = pszHostStart;
	pstStackTop->iCutOffVersion = a_iVersion;
	iStackItemCount = 1;

	iChange = 0;
	while (0 < iStackItemCount)
	{
		LPTDRMETAENTRY pstEntry;

		if (0 >= pstStackTop->iCount)
		{/*当前元数据数组已经处理完毕*/
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (pstEntry->iVersion > pstStackTop->iCutOffVersion)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			pszHostStart = *(char **)pszHostStart;
		}
		if (0 == pstEntry->iCount)
		{
			iCount = ((int)(pszHostEnd - pszHostStart)) / pstEntry->iHUnitSize;
		}else
		{
			iCount = pstEntry->iCount;
		}
		

		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{/*复合数据类型*/
			int idxSubEntry;
			LPTDRMETA pstTypeMeta;

			
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			if (TDR_TYPE_UNION == pstEntry->iType)
			{
				TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, pstStackTop->iCutOffVersion, pstTypeMeta, idxSubEntry);
				if (NULL == pstTypeMeta)
				{
					TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
					continue;
				}
			}else
			{
				pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				idxSubEntry = 0;
			}

			pstCurMeta = pstTypeMeta;
			iStackItemCount++;
			pstStackTop++;
			pstStackTop->idxEntry = idxSubEntry;
			pstStackTop->iCount = iCount;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->pszHostBase = pszHostStart;
			pstStackTop->iCutOffVersion = a_iVersion;			
		}else if (TDR_TYPE_MAX >= pstEntry->iType)
		{
			TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, iCount);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
		}else
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NET_UNSUPPORTED_TYPE);
			break;
		}
	}/*while (0 < iStackItemCount)*/
	
	return iRet;		 
}

int tdr_copy(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHostDst, IN LPTDRDATA a_pstHostSrc)
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
	char *pszDstStart;
	char *pszDstEnd;
	char *pszSrcStart;
	char *pszSrcEnd;
	int idxSubEntry;		


	/*assert(NULL != a_pstMeta);
	assert(NULL != a_pstHostDst);
	assert(NULL != a_pstHostDst->pszBuff);
	assert(0 < a_pstHostDst->iBuff);
	assert(NULL != a_pstHostSrc);
	assert(0 < a_pstHostSrc->iBuff);
	assert(NULL != a_pstHostSrc->pszBuff);*/
	if ((NULL == a_pstMeta)||(NULL == a_pstHostDst)||(NULL == a_pstHostSrc))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}
	if ((NULL == a_pstHostSrc->pszBuff)||(0 >= a_pstHostSrc->iBuff)||
		(NULL == a_pstHostDst->pszBuff)||(0 >= a_pstHostDst->iBuff))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	if (TDR_TYPE_STRUCT != a_pstMeta->iType)
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	pszDstStart = a_pstHostDst->pszBuff;
	pszDstEnd = a_pstHostDst->pszBuff + a_pstHostDst->iBuff;
	pszSrcStart = a_pstHostSrc->pszBuff;
	pszSrcEnd = a_pstHostSrc->pszBuff + a_pstHostSrc->iBuff;
	pstCurMeta = a_pstMeta;
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->pszHostBase = pszSrcStart;
	pstStackTop->pszHostEnd = pszSrcEnd;
	pstStackTop->pszNetBase = pszDstStart;
	pstStackTop->iChange = 0;
	pstStackTop->iCode = 0;
	iStackItemCount = 1;
	pstStackTop->iCutOffVersion = TDR_MAX_VERSION;

	iChange = 0;
	while (0 < iStackItemCount)
	{
		if (0 != iChange)
		{
			if (0 < pstStackTop->iCount)
			{
				pstStackTop->pszNetBase += pstCurMeta->iHUnitSize;
			}
			iChange = 0;
		}


		if (0 >= pstStackTop->iCount)
		{/*当前元数据数组已经处理完毕*/
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;	

		pszSrcStart = pstStackTop->pszHostBase + pstEntry->iHOff;
		pszDstStart = pstStackTop->pszNetBase + pstEntry->iHOff;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{	/*直接拷贝指针值*/
			*(char **)pszDstStart = *(char **)pszSrcStart;
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


		/*简单数据类型*/
		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{
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
			pstStackTop->pszHostBase = pszSrcStart;
			pstStackTop->pszNetBase = pszDstStart;
			pstStackTop->iChange = 1;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = iArrayRealCount;
			pstStackTop->idxEntry = idxSubEntry;
			pstStackTop->iCutOffVersion = TDR_MAX_VERSION;
		}else
		{
			switch(pstEntry->iType)																							
			{																													
			case TDR_TYPE_STRING:																							
			case TDR_TYPE_WSTRING:
				TDR_COPY_STRING_ENTRY_DATA(iRet, pstEntry, iArrayRealCount, pszDstStart, pszDstEnd, pszSrcStart, pszSrcEnd);
				break;																										
			default:																										
				TDR_COPY_INTEGER_ENTRY_DATA(iRet, pstEntry, iArrayRealCount, pszDstStart, pszDstEnd, pszSrcStart, pszSrcEnd);
				break;																										
			}

			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/
	}/*while (0 < iStackItemCount)*/

	a_pstHostDst->iBuff = pszDstStart - a_pstHostDst->pszBuff;

	return iRet;		
}

TDR_API int tdr_browse(IN LPTDRMETA a_pstMeta, IN void *a_pHandle, IN LPTDRDATA a_pstHost, IN int a_iVersion,
					   IN PFN_ENTER_META pfnEnter, IN PFN_VISIT_SIMPLE_ENTRY pfnVistEntry)
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
	char *pszHostStart;
	char *pszHostEnd;
	int idxSubEntry;
	void *pChildHandle;
	int index;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_pstHost);
	assert(NULL != a_pstHost->pszBuff);
	assert(0 < a_pstHost->iBuff);
	assert(NULL != a_pHandle);
	assert(NULL != pfnEnter);
	assert(NULL != pfnVistEntry);*/
	if ((NULL == a_pstMeta)||(NULL == a_pstHost)||(a_pHandle))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}
	if ((NULL == pfnEnter)||(NULL == pfnVistEntry))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	if (TDR_TYPE_STRUCT != a_pstMeta->iType)
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}
	
	if (0 == a_iVersion)
	{
		a_iVersion = TDR_MAX_VERSION;
	}
	if (a_pstMeta->iBaseVersion > a_iVersion)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	
	pstCurMeta = a_pstMeta;
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->pszHostBase = pszHostStart;
	pstStackTop->pszNetBase = a_pHandle;  /*save handle */
	pstStackTop->pszMetaSizeInfoTarget = NULL;  /*save root handle */
	pstStackTop->iChange = 0;
	pstStackTop->iCode = 0;
	iStackItemCount = 1;
	pstStackTop->iCutOffVersion = TDR_MAX_VERSION;
	pstStackTop->iRealCount = 1; /*real count of meta*/
	pstStackTop->pstEntry = NULL;

	iChange = 0;
	while (0 < iStackItemCount)
	{
		if (0 != iChange)
		{
			if (0 < pstStackTop->iCount)
			{
				assert(NULL != pstStackTop->pstEntry);
				if (1 == pstStackTop->pstEntry->iCount)
				{
					index = -1;
				}else
				{
					index = pstStackTop->iRealCount - pstStackTop->iCount;
				}
				iRet = (*pfnEnter)((void *)pstStackTop->pszMetaSizeInfoTarget,pstStackTop->pstEntry, 
					pstStackTop->iRealCount,index, &pChildHandle);
				if (0 != iRet)
				{
					iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_TO_CALL_ENTER_META);
					break;
				}
				pstStackTop->pszNetBase = pChildHandle;
			}/*if (0 < pstStackTop->iCount)*/
			iChange = 0;
		}/*if (0 != iChange)*/

		if (0 >= pstStackTop->iCount)
		{/*当前元数据数组已经处理完毕*/
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}
		
		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{	
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

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
		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{
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

			if (1 == pstEntry->iCount)
			{  /*只有结构数组成员才传入具体的索引值，否则设置为-1*/
				index = -1;
			}else
			{
				index = 0;
			}
			iRet = (*pfnEnter)((void *)pstStackTop->pszNetBase,pstEntry, iArrayRealCount,index, &pChildHandle);
			if (0 != iRet)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_TO_CALL_ENTER_META);
				break;
			}
			if (0 >= iArrayRealCount)
			{
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				continue;
			}

			/*递归进行结构体内部进行处理*/
			iStackItemCount++;
			pstStackTop++;			
			pstStackTop->pszHostBase = pszHostStart;
			pstStackTop->pszMetaSizeInfoTarget = (pstStackTop-1)->pszNetBase;  /*save root handle */	
			pstStackTop->pszNetBase = pChildHandle;
			pstStackTop->iChange = 0;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = iArrayRealCount;
			pstStackTop->idxEntry = idxSubEntry;
			pstStackTop->iCutOffVersion = TDR_MAX_VERSION;
			pstStackTop->iRealCount = iArrayRealCount;
			pstStackTop->pstEntry = pstEntry;
			pstStackTop->iChange = 1;
		}else
		{
			/*简单数据类型*/
			int iSize;

			iSize = tdr_get_entry_unitsize(pstEntry) * iArrayRealCount;
			iSize = TDR_MIN(iSize, (pszHostEnd - pszHostStart));
			iRet = (*pfnVistEntry)((void *)pstStackTop->pszNetBase, pstEntry, 
				iArrayRealCount, pszHostStart, iSize);
			if (0 != iRet)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_TO_CALL_VIST_ENTRY);
				break;
			}

			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/
	}/*while (0 < iStackItemCount)*/

	return iRet;		
}
