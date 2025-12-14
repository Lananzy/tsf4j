/**
*
* @file     tdr_meta_entries_index.h  
* @brief    TDR结构体内加快成员访问速度的索引模块实现文件
*
* @author jackyai  
* @version 1.0
* @date 2007-12-04 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "tdr/tdr_meta_entries_index.h"
#include "tdr/tdr_error.h"
#include "tdr_os.h"
#include "tdr_metalib_kernel_i.h"
#include "tdr_auxtools.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define TDR_META_ENTRY_FLAG_MALLOC_MEMORY		0x00000001		/**<索引表内存在接口内分配*/



static int tdr_calc_entry_name_index_num_i(OUT int *a_piIndexNum, IN LPTDRMETA a_pstMeta);

/**生成成名字－指针索引表
*/
static int tdr_gen_entry_name_index_i(INOUT LPTDRMETAENTRYNAMEINDEX a_pstIndexTable, IN int a_iMaxNum, IN LPTDRMETA a_pstMeta);

/**两个索引信息进行比较
*@retval >0 第一个索引信息比第二个“大”
*@retval 0 相等
*@retval <0 第一个索引信息比第二个“小”
*/
static int tdr_name_index_comp_i(const void *a_pv1, const void *a_pv2);

static int tdr_calc_entry_off_index_num_i(OUT int *a_piIndexNum, IN LPTDRMETA a_pstMeta);

/**生成成名字－指针索引表
*/
static int tdr_gen_entry_off_index_i(INOUT LPTDRMETAENTRYOFFINDEX a_pstIndexTable, IN int a_iMaxNum, IN LPTDRMETA a_pstMeta);

/**两个索引信息进行比较
*@retval >0 第一个索引信息比第二个“大”
*@retval 0 相等
*@retval <0 第一个索引信息比第二个“小”
*/
static int tdr_off_index_comp_i(const void *a_pv1, const void *a_pv2);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int tdr_create_entry_name_index(INOUT LPTDRMETAENTRYNAMEINDEX *a_pstIndex, IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstBuff)
{
	int iRet = TDR_SUCCESS;
	int iIndexNum = 0;
	size_t sIndexTableSize;	/*索引表的空间大小*/
	LPTDRMETAENTRYNAMEINDEX pstIndexTable = NULL;

	/*assert(NULL != a_pstIndex);
	assert(NULL != a_pstMeta);*/
	if ((NULL == a_pstIndex)||(NULL == a_pstMeta))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

	/*计算成员名字－指针索引表索引数目*/
	iRet = tdr_calc_entry_name_index_num_i(&iIndexNum, a_pstMeta);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	/*计算索引表存储区的空间大小，并分配空间*/
	sIndexTableSize = offsetof(TDRMETAENTRYNAMEINDEX, astIndexs) + iIndexNum * sizeof(TDRNAMEENTRYINDEXINFO);
	if (NULL != a_pstBuff)
	{
		if ((NULL == a_pstBuff->pszBuff) || (a_pstBuff->iBuff < (int)sIndexTableSize))
		{
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
		}
		pstIndexTable = (LPTDRMETAENTRYNAMEINDEX)a_pstBuff->pszBuff;
		a_pstBuff->iBuff = (int)sIndexTableSize;
		memset(pstIndexTable, 0, sIndexTableSize);
	}else
	{
		pstIndexTable = (LPTDRMETAENTRYNAMEINDEX)malloc(sIndexTableSize);
		if (NULL == pstIndexTable)
		{
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
		}
		memset(pstIndexTable, 0, sIndexTableSize);
		pstIndexTable->iFlag |= TDR_META_ENTRY_FLAG_MALLOC_MEMORY;
	}/*if (NULL != a_pstBuff)*/

	/*生成索引信息*/
	iRet = tdr_gen_entry_name_index_i(pstIndexTable, iIndexNum, a_pstMeta);


	/*根据成员名字对索引信息进行排序*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		tdr_qsort(&pstIndexTable->astIndexs[0], pstIndexTable->iEntryNum, sizeof(TDRNAMEENTRYINDEXINFO), tdr_name_index_comp_i);

		*a_pstIndex = pstIndexTable;
	}else if (pstIndexTable->iFlag & TDR_META_ENTRY_FLAG_MALLOC_MEMORY)
	{
		free(pstIndexTable);
	}

	return iRet;
}


int tdr_calc_entry_name_index_num_i(OUT int *a_piIndexNum, IN LPTDRMETA a_pstMeta)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	LPTDRMETA pstCurMeta;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;
	int iChange;	
	int iCount = 0;

	assert(NULL != a_piIndexNum);
	assert(NULL != a_pstMeta);

	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = a_pstMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstCurMeta = a_pstMeta;
	iStackItemCount = 1;

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
		iCount++;
		
		if ((TDR_TYPE_STRUCT != pstEntry->iType) || (!TDR_ENTRY_DO_EXTENDABLE(pstEntry)))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}	

		/*扩展结构，扩展此结构继续计算*/
		if (TDR_STACK_SIZE <=  iStackItemCount)
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
			break;
		}
		pstStackTop++;
		iStackItemCount++;
		pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
		pstStackTop->pstMeta = pstCurMeta;
		pstStackTop->iCount = 1;
		pstStackTop->idxEntry = 0;
	}/*while (0 < iStackItemCount)*/	

	*a_piIndexNum = iCount;

	return iRet;
}

int tdr_gen_entry_name_index_i(INOUT LPTDRMETAENTRYNAMEINDEX a_pstIndexTable, IN int a_iMaxNum, IN LPTDRMETA a_pstMeta)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	LPTDRMETA pstCurMeta;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;
	int iChange;	
	int iCount = 0;

	assert(NULL != a_pstIndexTable);
	assert(0 < a_iMaxNum);
	assert(NULL != a_pstMeta);

	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = a_pstMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstCurMeta = a_pstMeta;
	iStackItemCount = 1;
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
		//TDR_STRNCPY(a_pstIndexTable->astIndexs[iCount].szName, pstEntry->szName, sizeof(a_pstIndexTable->astIndexs[0].szName));
		a_pstIndexTable->astIndexs[iCount].pstEntry = pstEntry;
		if (1 < pstStackTop->iRealCount)
		{
			a_pstIndexTable->astIndexs[iCount].bIsInExtendArr = TDR_TRUE;
		}else
		{
			a_pstIndexTable->astIndexs[iCount].bIsInExtendArr = TDR_FALSE;
		}
		iCount++;
		if (iCount >= a_iMaxNum)
		{			
			break;
		}

		if ((TDR_TYPE_STRUCT != pstEntry->iType) || (!TDR_ENTRY_DO_EXTENDABLE(pstEntry)))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}	

		/*扩展结构，扩展此结构继续计算*/
		if (TDR_STACK_SIZE <=  iStackItemCount)
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
			break;
		}
		pstStackTop++;
		iStackItemCount++;
		pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
		pstStackTop->pstMeta = pstCurMeta;
		pstStackTop->iCount = 1;
		pstStackTop->idxEntry = 0;
		if (1 < pstEntry->iCount)
		{
			pstStackTop->iRealCount = pstEntry->iCount;
		}else
		{
			pstStackTop->iRealCount = (pstStackTop - 1)->iRealCount;
		}
	}/*while (0 < iStackItemCount)*/	

	a_pstIndexTable->iEntryNum = iCount;
	
	
	return iRet;
}

int tdr_name_index_comp_i(const void *a_pv1, const void *a_pv2)
{
	LPTDRNAMEENTRYINDEXINFO pstName1 = (LPTDRNAMEENTRYINDEXINFO)a_pv1;
	LPTDRNAMEENTRYINDEXINFO pstName2 = (LPTDRNAMEENTRYINDEXINFO)a_pv2;

	assert(NULL != pstName1);
	assert(NULL != pstName2);
	assert(NULL != pstName1->pstEntry);
	assert(NULL != pstName2->pstEntry);

	return strcmp(pstName1->pstEntry->szName, pstName2->pstEntry->szName);
}

void tdr_destroy_entry_name_index(INOUT LPTDRMETAENTRYNAMEINDEX *a_pstIndex)
{
	if (NULL != a_pstIndex)
	{
		if ((NULL != *a_pstIndex) && ((*a_pstIndex)->iFlag & TDR_META_ENTRY_FLAG_MALLOC_MEMORY))
		{
			free(*a_pstIndex);
		}
		*a_pstIndex = NULL;
	}
}

void tdr_dump_entry_name_index(IN FILE *a_fpFile, IN LPTDRMETAENTRYNAMEINDEX a_pstIndex)
{
	int i;

	//assert(NULL != a_fpFile);
	//assert(NULL != a_pstIndex);
	if ((NULL == a_fpFile)||(NULL == a_pstIndex))
	{
		return ;
	}
	
	fprintf(a_fpFile, "IndexCount: %d Flag:%x\n", a_pstIndex->iEntryNum, a_pstIndex->iFlag);
	for (i = 0; i < a_pstIndex->iEntryNum; i++)
	{
		if ((i % 4 == 0) && (i > 0))
		{
			fprintf(a_fpFile, "\n");
		}
		fprintf(a_fpFile, "<%-16s	ptr:%p> ", a_pstIndex->astIndexs[i].pstEntry->szName, a_pstIndex->astIndexs[i].pstEntry);
	}
	fprintf(a_fpFile, "\n");
	
}

LPTDRNAMEENTRYINDEXINFO tdr_find_entry_name_index(IN LPTDRMETAENTRYNAMEINDEX a_pstIndex, IN const char *a_pszName)
{
	int iRet;
	int iMin;
	int iMax;
	int i;
	LPTDRNAMEENTRYINDEXINFO pstIndex = NULL;
	LPTDRNAMEENTRYINDEXINFO pstIndexBase;

	//assert(NULL != a_pstIndex);
	//assert(NULL != a_pszName);
	if ((NULL == a_pszName)||(NULL == a_pstIndex))
	{
		return NULL;
	}

	iMin = 0;
	iMax = a_pstIndex->iEntryNum -1;

	pstIndexBase = &a_pstIndex->astIndexs[0];
	while(iMin <= iMax)
	{
		i = (iMin+iMax)>>1;

		iRet = strcmp(a_pszName, pstIndexBase[i].pstEntry->szName);
		if( iRet>0 )
		{
			iMin    =       i + 1;
		}
		else if( iRet<0 )
		{
			iMax    =       i - 1;
		}
		else
		{
			pstIndex = &pstIndexBase[i];
			break;
		}
	}	

	return pstIndex;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int tdr_create_entry_off_index(INOUT LPTDRMETAENTRYOFFINDEX *a_pstIndex, IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstBuff)
{
	int iRet = TDR_SUCCESS;
	int iIndexNum = 0;
	size_t sIndexTableSize;	/*索引表的空间大小*/
	LPTDRMETAENTRYOFFINDEX pstIndexTable = NULL;

	//assert(NULL != a_pstIndex);
	//assert(NULL != a_pstMeta);
	if ((NULL == a_pstIndex)||(NULL == a_pstMeta))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}


	/*计算成员名字－指针索引表索引数目*/
	iRet = tdr_calc_entry_off_index_num_i(&iIndexNum, a_pstMeta);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	/*计算索引表存储区的空间大小，并分配空间*/
	sIndexTableSize = offsetof(TDRMETAENTRYOFFINDEX, astIndexs) + iIndexNum * sizeof(TDRENTRYOFFINDEXINFO);
	if (NULL != a_pstBuff)
	{
		if ((NULL == a_pstBuff->pszBuff) || (a_pstBuff->iBuff < (int)sIndexTableSize))
		{
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
		}
		pstIndexTable = (LPTDRMETAENTRYOFFINDEX)a_pstBuff->pszBuff;
		a_pstBuff->iBuff = (int)sIndexTableSize;
		memset(pstIndexTable, 0, sIndexTableSize);
	}else
	{
		pstIndexTable = (LPTDRMETAENTRYOFFINDEX)malloc(sIndexTableSize);
		if (NULL == pstIndexTable)
		{
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
		}
		memset(pstIndexTable, 0, sIndexTableSize);
		pstIndexTable->iFlag |= TDR_META_ENTRY_FLAG_MALLOC_MEMORY;
	}/*if (NULL != a_pstBuff)*/

	/*生成索引信息*/
	iRet = tdr_gen_entry_off_index_i(pstIndexTable, iIndexNum, a_pstMeta);


	/*根据成员名字对索引信息进行排序*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		tdr_qsort(&pstIndexTable->astIndexs[0], pstIndexTable->iEntryNum, sizeof(TDRENTRYOFFINDEXINFO), tdr_off_index_comp_i);

		*a_pstIndex = pstIndexTable;
	}else if (pstIndexTable->iFlag & TDR_META_ENTRY_FLAG_MALLOC_MEMORY)
	{
		free(pstIndexTable);
	}

	return iRet;
}

void tdr_destroy_entry_off_index(INOUT LPTDRMETAENTRYOFFINDEX *a_pstIndex)
{
	if (NULL != a_pstIndex)
	{
		if ((NULL != *a_pstIndex) && ((*a_pstIndex)->iFlag & TDR_META_ENTRY_FLAG_MALLOC_MEMORY))
		{
			free(*a_pstIndex);
		}
		*a_pstIndex = NULL;
	}
}

void tdr_dump_entry_off_index(IN FILE *a_fpFile, IN LPTDRMETAENTRYOFFINDEX a_pstIndex)
{
	int i;

	assert(NULL != a_fpFile);
	assert(NULL != a_pstIndex);

	fprintf(a_fpFile, "IndexCount: %d Flag:%x\n", a_pstIndex->iEntryNum, a_pstIndex->iFlag);
	for (i = 0; i < a_pstIndex->iEntryNum; i++)
	{
		fprintf(a_fpFile, "<ptr: %p	HOff:%d>\n", a_pstIndex->astIndexs[i].pstEntry, a_pstIndex->astIndexs[i].iHOff);
	}
	
	fprintf(a_fpFile, "\n");
}

LPTDRENTRYOFFINDEXINFO tdr_find_entry_off_index(IN LPTDRMETAENTRYOFFINDEX a_pstIndex, IN LPTDRMETAENTRY a_pstEntry)
{
	int iRet;
	int iMin;
	int iMax;
	int i;
	LPTDRENTRYOFFINDEXINFO pstIndex = NULL;
	LPTDRENTRYOFFINDEXINFO pstIndexBase;

	/*assert(NULL != a_pstIndex);
	assert(NULL != a_pstEntry);*/
	if ((NULL == a_pstIndex)||(NULL == a_pstEntry))
	{
		return NULL;
	}

	iMin = 0;
	iMax = a_pstIndex->iEntryNum -1;

	pstIndexBase = &a_pstIndex->astIndexs[0];
	while(iMin <= iMax)
	{
		i = (iMin+iMax)>>1;

		iRet = (int)a_pstEntry - (int)pstIndexBase[i].pstEntry;
		if( iRet>0 )
		{
			iMin    =       i + 1;
		}
		else if( iRet<0 )
		{
			iMax    =       i - 1;
		}
		else
		{
			pstIndex = &pstIndexBase[i];
			break;
		}
	}	

	return pstIndex;
}

int tdr_calc_entry_off_index_num_i(OUT int *a_piIndexNum, IN LPTDRMETA a_pstMeta)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	LPTDRMETA pstCurMeta;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;
	int iChange;	
	int iCount = 0;

	assert(NULL != a_piIndexNum);
	assert(NULL != a_pstMeta);

	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = a_pstMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstCurMeta = a_pstMeta;
	iStackItemCount = 1;

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
		iCount++;

		if (TDR_TYPE_COMPOSITE < pstEntry->iType) 
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}	

		/*复合结构，扩展此结构继续计算*/
		if (TDR_STACK_SIZE <=  iStackItemCount)
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
			break;
		}
		pstStackTop++;
		iStackItemCount++;
		pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
		pstStackTop->pstMeta = pstCurMeta;
		pstStackTop->iCount = 1;
		pstStackTop->idxEntry = 0;
	}/*while (0 < iStackItemCount)*/	

	*a_piIndexNum = iCount;

	return iRet;
}

int tdr_gen_entry_off_index_i(INOUT LPTDRMETAENTRYOFFINDEX a_pstIndexTable, IN int a_iMaxNum, IN LPTDRMETA a_pstMeta)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	LPTDRMETA pstCurMeta;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;
	int iChange;	
	int iCount = 0;

	assert(NULL != a_pstIndexTable);
	assert(0 < a_iMaxNum);
	assert(NULL != a_pstMeta);

	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = a_pstMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstCurMeta = a_pstMeta;
	iStackItemCount = 1;
	pstStackTop->iMetaSizeInfoOff = 0; /*base offset of meta*/

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
		a_pstIndexTable->astIndexs[iCount].iHOff = pstStackTop->iMetaSizeInfoOff + pstEntry->iHOff;
		a_pstIndexTable->astIndexs[iCount].pstEntry = pstEntry;
		iCount++;
		if (iCount >= a_iMaxNum)
		{
			break;
		}

		if (TDR_TYPE_COMPOSITE < pstEntry->iType) 
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}	

		/*复合结构，扩展此结构继续计算*/
		if (TDR_STACK_SIZE <=  iStackItemCount)
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
			break;
		}
		pstStackTop++;
		iStackItemCount++;
		pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
		pstStackTop->pstMeta = pstCurMeta;
		pstStackTop->iCount = 1;
		pstStackTop->idxEntry = 0;
		pstStackTop->iMetaSizeInfoOff = pstEntry->iHOff;
	}/*while (0 < iStackItemCount)*/	

	
	a_pstIndexTable->iEntryNum = iCount;
	
	return iRet;
}

int tdr_off_index_comp_i(const void *a_pv1, const void *a_pv2)
{
	LPTDRENTRYOFFINDEXINFO pstOff1 = (LPTDRENTRYOFFINDEXINFO)a_pv1;
	LPTDRENTRYOFFINDEXINFO pstOff2 = (LPTDRENTRYOFFINDEXINFO)a_pv2;

	assert(NULL != pstOff1);
	assert(NULL != pstOff2);

	return (int)pstOff1->pstEntry - (int)pstOff2->pstEntry;
}

void *tdr_calc_entry_address(IN void *a_pMetaBase, IN LPTDRMETAENTRYNAMEINDEX a_pstNameIndex, IN LPTDRMETAENTRYOFFINDEX a_pstOffIndex, IN const char *a_pszName)
{
	LPTDRNAMEENTRYINDEXINFO pstName;
	LPTDRENTRYOFFINDEXINFO pstOff;

	/*assert(NULL != a_pMetaBase);
	assert(NULL != a_pstNameIndex);
	assert(NULL != a_pstOffIndex);
	assert(NULL != a_pszName);*/
	if ((NULL == a_pMetaBase) || (NULL == a_pstNameIndex)||
		(NULL == a_pstOffIndex)||(NULL == a_pszName))
	{
		return NULL;
	}

	pstName = tdr_find_entry_name_index(a_pstNameIndex, a_pszName);
	if (NULL == pstName)
	{
		return NULL;
	}

	pstOff = tdr_find_entry_off_index(a_pstOffIndex, pstName->pstEntry);
	if (NULL == pstOff)
	{
		return NULL;
	}

	return ((char *)a_pMetaBase + pstOff->iHOff);
}
