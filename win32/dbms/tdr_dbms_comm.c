/**
*
* @file     tdr_dbms_comm.c  
* @brief    TSF-G-DR 可以提供给上层进一步封装的DB结合处理函数
* 
* @author jackyai  
* @version 1.0
* @date 2007-12-13 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/
#include <assert.h>
#include "tdr/tdr_error.h"
#include "tdr_dbms_comm.h"
#include "tdr/tdr_auxtools.h"


int tdr_get_dbtable_name(OUT char *a_pszName, IN int a_iNameSize, IN LPTDRMETA a_pstMeta, IN char *a_pszMetaHost)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib;
	LPTDRMETA pstTableMeta;
	char *pszHostStart;
	LPTDRMETAENTRY pstEntry;
	tdr_ulonglong ulTableID;
	int iLen;
	
	assert(NULL != a_pszName);
	assert(0 < a_iNameSize);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pszMetaHost);

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	TDR_GET_DBTABLE_META(pstTableMeta, pstLib, a_pstMeta);

	if ((0 >= pstTableMeta->iSplitTableFactor) || 
		(TDR_SPLITTABLE_RULE_BY_MOD != pstTableMeta->nSplitTableRuleID))
	{
		TDR_STRNCPY(a_pszName, pstTableMeta->szName, a_iNameSize);
		return iRet;
	}

	/*分表*/
	if ((TDR_INVALID_PTR == a_pstMeta->stSplitTableKey.ptrEntry) ||
		(0 > a_pstMeta->stSplitTableKey.iHOff))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_NOSPLITTABLE_KEY);
	}

	pstEntry = TDR_PTR_TO_ENTRY(pstLib, a_pstMeta->stSplitTableKey.ptrEntry);
	assert(TDR_TYPE_COMPOSITE < pstEntry->iType);
	pszHostStart = a_pszMetaHost + a_pstMeta->stSplitTableKey.iHOff;

	switch(pstEntry->iType)
	{
	case TDR_TYPE_STRING:
		{
			iLen = tdr_strnlen(pszHostStart, pstEntry->iCustomHUnitSize)+1;
			ulTableID = tdr_get_hash_code(pszHostStart, iLen);
		}		
		break;
	case TDR_TYPE_WSTRING:
		{
			iLen = tdr_wcsnlen((wchar_t *)pszHostStart, pstEntry->iCustomHUnitSize)+1;
			iLen *= sizeof(tdr_wchar_t);
			ulTableID = tdr_get_hash_code(pszHostStart, iLen);
		}
		break;
	default:
		TDR_GET_INT(ulTableID, pstEntry->iHUnitSize, pszHostStart);	
		break;
	}

	tdr_snprintf(a_pszName, a_iNameSize, "%s_%llu", pstTableMeta->szName, ulTableID % (tdr_ulonglong)pstTableMeta->iSplitTableFactor);

	return iRet;
}


int tdr_gen_columm_name_lists_i(INOUT LPTDRDATA a_pstSql, IN  LPTDRDBOBJECT a_pstObj, IN TDRBOOLEAN bCalcRefer)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	LPTDRMETA pstCurMeta;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;
	int iArrayRealCount;
	char *pszHostStart;
	char *pchSql;
	int iLeftLen, iWriteLen;
	int iChange;
	int i;
	char *pszPreName="";

	assert(NULL != a_pstSql);
	assert(NULL != a_pstObj);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);
	assert(NULL != a_pstObj->pstMeta);
	assert(NULL != a_pstObj->pszObj);
	assert(0 < a_pstObj->iObjSize);

	pstStackTop = &stStack[0];
	pstCurMeta = a_pstObj->pstMeta;
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->pszHostBase = a_pstObj->pszObj;
	iStackItemCount = 1;
	pchSql = a_pstSql->pszBuff;
	iLeftLen = a_pstSql->iBuff;
	pstLib = TDR_META_TO_LIB(pstCurMeta);
	pstStackTop->pstEntry = NULL;
	pstStackTop->iRealCount = 1;

	while (0 < iStackItemCount)
	{
		if (0 >= pstStackTop->iCount)
		{
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}/*if (0 >= pstStackTop->iCount)*/

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (pstEntry->iVersion > a_pstObj->iVersion)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}		

		/*取出此entry的数组计数信息*/	
		if (TDR_TRUE == bCalcRefer)
		{
			TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, a_pstObj->iVersion); 
			if ((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount))
			{
				/*实际数目为负数或比数组最大长度要大，则无效*/
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
				break;
			}
			if (0  >= iArrayRealCount)
			{
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				continue;
			}
		}else
		{
			iArrayRealCount = pstEntry->iCount;
		}		

		/*复合数据类型*/
		if ((TDR_TYPE_COMPOSITE >= pstEntry->iType) && (TDR_ENTRY_DO_EXTENDABLE(pstEntry)))
		{			
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
			pstStackTop++;
			iStackItemCount++;
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = iArrayRealCount;
			pstStackTop->iRealCount = iArrayRealCount;
			pstStackTop->pstEntry = pstEntry;
			pstStackTop->idxEntry = 0;	
			pstStackTop->pszHostBase = pszHostStart;

			continue;
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

		pszPreName = tdr_gen_entry_columnname_prefix(&stStack[0], iStackItemCount);
		if ((1 == pstEntry->iCount) ||
			((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType)))
		{
			iWriteLen = tdr_snprintf(pchSql, iLeftLen, "%s%s,", pszPreName, pstEntry->szName);
			TDR_CHECK_BUFF(pchSql, iLeftLen, iWriteLen, iRet);
		}else if (TDR_TYPE_WSTRING >= pstEntry->iType)
		{
			for (i = 1; i <= iArrayRealCount; i++)
			{
				iWriteLen = tdr_snprintf(pchSql, iLeftLen, "%s%s_%d,", pszPreName, pstEntry->szName, i);
				TDR_CHECK_BUFF(pchSql, iLeftLen, iWriteLen, iRet);
			}
		}/*if ((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType))*/

		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/	

	/*去掉最后一个','字符*/
	pchSql--;

	a_pstSql->iBuff = pchSql - a_pstSql->pszBuff;
	return iRet;
}

int tdr_gen_columm_value_lists_i(INOUT LPTDRDATA a_pstSql, IN TDRDBHANDLE a_hDBHandle, INOUT  LPTDRDBOBJECT a_pstObj)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	int iChange;
	LPTDRMETA pstCurMeta;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;
	int iArrayRealCount;
	char *pszHostStart;
	char *pszHostEnd;
	char *pchSql;
	int iLeftLen;
	int i;



	assert(NULL != a_pstSql);
	assert(NULL != a_pstObj);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);
	assert(NULL != a_pstObj->pstMeta);
	assert(NULL != a_pstObj->pszObj);
	assert(0 < a_pstObj->iObjSize);

	pstStackTop = &stStack[0];
	pstCurMeta = a_pstObj->pstMeta;
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->pszHostBase = a_pstObj->pszObj;
	iStackItemCount = 1;
	pchSql = a_pstSql->pszBuff;
	iLeftLen = a_pstSql->iBuff;
	pstLib = TDR_META_TO_LIB(pstCurMeta);
	pszHostEnd = a_pstObj->pszObj + a_pstObj->iObjSize;


	while (0 < iStackItemCount)
	{
		int idxSelEntry = TDR_INVALID_INDEX;

		if (0 >= pstStackTop->iCount)
		{
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}/*if (0 >= pstStackTop->iCount)*/

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (pstEntry->iVersion > a_pstObj->iVersion)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}		

		/*取出此entry的数组计数信息*/	
		TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, a_pstObj->iVersion); 
		if ((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount))
		{
			/*实际数目为负数或比数组最大长度要大，则无效*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}
		if (0 >= iArrayRealCount)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

		/*union 类型*/ 
		if (TDR_TYPE_UNION == pstEntry->iType)
		{
			LPTDRMETA pstSelMeta;
			TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, a_pstObj->iVersion, pstSelMeta, idxSelEntry);
			if (TDR_INVALID_INDEX == idxSelEntry)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
				break;
			}
			if (NULL == pstSelMeta)
			{
				continue;
			}
		}/*if (TDR_TYPE_UNION == pstEntry->iType)*/
		

		/*复合数据类型*/
		if ((TDR_TYPE_STRUCT == pstEntry->iType) && (TDR_ENTRY_DO_EXTENDABLE(pstEntry)))
		{
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pstStackTop++;
			iStackItemCount++;
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = iArrayRealCount;			
			pstStackTop->idxEntry = 0;	
			pstStackTop->pszHostBase = pszHostStart;

			continue;
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

		/*生成成员的数据*/
		if ((1 == pstEntry->iCount) ||
			((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType)))
		{
			TDR_ENTRY_VALUE2SQL_VALUE(iRet, a_hDBHandle, pstLib, pstEntry, iArrayRealCount, idxSelEntry, a_pstObj->iVersion, pchSql, iLeftLen, 
				pszHostStart, pszHostEnd);	
			if (0 < iLeftLen)
			{
				iLeftLen--;
				*pchSql++ = ',';
			}
		}else if (TDR_TYPE_WSTRING >= pstEntry->iType)
		{
			for (i = 1; i <= iArrayRealCount; i++)
			{
				TDR_ENTRY_VALUE2SQL_VALUE(iRet, a_hDBHandle, pstLib, pstEntry, 1, idxSelEntry, a_pstObj->iVersion, pchSql, iLeftLen, 
					pszHostStart, pszHostEnd);	
				if (0 < iLeftLen)
				{
					iLeftLen--;
					*pchSql++ = ',';
				}
			}
		}/*if ((TDR_TYPE_CHAR <= pstEntry->iType) && (TDR_TYPE_BYTE >= pstEntry->iType))*/			

		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/	

	/*去掉最后一个','字符*/
	pchSql--;

	a_pstSql->iBuff = pchSql - a_pstSql->pszBuff;

	return iRet;
}


char *tdr_gen_entry_columnname_prefix(TDRSTACKITEM	a_pstStack[], int a_iStackCount)
{
	int i;
	static char szName[TDR_NAME_LEN*2*TDR_STACK_SIZE];
	char *pch;
	LPTDRSTACKITEM pstTempStack;
	int iWirte;
	int iLeft;

	assert(NULL != a_pstStack);
	assert(NULL != a_pstStack[0].pstMeta);

	if (!TDR_META_DO_NEED_PREFIX(a_pstStack[0].pstMeta))
	{
		return "";
	}

	pch = &szName[0];
	*pch = '\0';
	iLeft = sizeof(szName);
	for (i = 1; i < a_iStackCount; i++)
	{
		LPTDRMETAENTRY pstEntry;
		pstTempStack = a_pstStack + i;
		pstEntry = pstTempStack->pstEntry;
		assert(NULL != pstEntry);
		if (1 == pstEntry->iCount)
		{
			iWirte = tdr_snprintf(pch, iLeft, "%s_", pstEntry->szName);
		}else
		{
			iWirte = tdr_snprintf(pch, iLeft, "%s%d_", pstTempStack->pstEntry->szName,
				pstTempStack->iRealCount - pstTempStack->iCount + 1);
		}
		
		iLeft -= iWirte;
		pch += iWirte;
	}/*for (i = 1; i < a_iStackCount; i++)*/
	return &szName[0];
}


