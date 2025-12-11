/**
*
* @file     tdr_data_inoutput.c 
* @brief    TDR数据输入输出模块
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-09-11 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <scew/scew.h>
#include "tdr/tdr_os.h"
#include "scew/scew.h"
#include "tdr/tdr_define.h"
#include "tdr/tdr_data_io.h"
#include "tdr/tdr_error.h"
#include "tdr/tdr_metalib_init.h"
#include "tdr/tdr_metalib_kernel_i.h"
#include "tdr/tdr_ctypes_info_i.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_iostream_i.h"
#include "tdr_XMLMetalib_i.h"
#include "tdr/tdr_XMLtags.h"
#include "tdr/tdr_auxtools.h"
#include "tdr/tdr_metalib_manage_i.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

						   
#define TDR_SET_ARRAY_REAL_COUNT(a_iArrayRealCount, a_pstEntry, a_pszHostBase) \
{										\
	if (0 < a_pstEntry->stRefer.iUnitSize)	\
	{										\
		char *pszPtr =	a_pszHostBase + a_pstEntry->stRefer.iHOff;	\
		TDR_SET_INT(pszPtr, a_pstEntry->stRefer.iUnitSize, a_iArrayRealCount);			\
	}\
}





#define TDR_GET_NEXT_ELEMET(a_pstItem, a_pstRoot, a_pstCurItem, a_pszName) \
{																						 \
	a_pstItem = scew_element_next((scew_element *)a_pstRoot,				 \
		(scew_element *)a_pstCurItem);     						\
	while (NULL != a_pstItem)															\
	{																					\
		if (0 == strcmp(scew_element_name(a_pstItem), a_pszName))						\
		{																				\
			break;																		\
		}else																			\
		{																				\
			a_pstItem = scew_element_next((scew_element *)a_pstRoot, a_pstItem);  	\
		}																				\
	}																					\
}

#define TDR_CLEAR_COUNT(a_pstTop, a_pstMeta, a_pszHostStart, a_pszHostEnd)	\
{																	\
	int iLen;															\
	for (; a_pstTop->iCount > 0; a_pstTop->iCount--)				\
	{																\
		a_pszHostStart = a_pstTop->pszHostBase;						\
		if (a_pszHostStart > a_pszHostEnd)							\
		{															\
			a_pstTop->iCount = 0;									\
			break;													\
		}															\
		if ((a_pszHostEnd-a_pszHostStart) < a_pstMeta->iHUnitSize)	\
		{															\
		iLen = (int)(a_pszHostEnd-a_pszHostStart);				\
		}else														\
		{															\
		iLen = a_pstMeta->iHUnitSize;							\
		}															\
		memset(a_pszHostStart, 0, iLen);							\
		a_pszHostStart += iLen;										\
		a_pstTop->pszHostBase += a_pstMeta->iHUnitSize;				\
	}																\
}



#define TDR_GET_VERSION_INDICATOR_BY_XML(a_pstMeta, a_pstRoot, a_iCutOffVersion, a_iBaseCutVersion) \
{																								\
	a_iCutOffVersion = a_iBaseCutVersion;														\
}

/**
将本地内存数据结构按元数据描述转换成XML格式的数据
*/
static int tdr_output_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRDATA a_pstHost,
						IN int a_iCufOffVersion);

/**
将本地内存数据结构按元数据描述转换成之前旧的XML格式的数据
*/
static int tdr_output_oldversion_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRDATA a_pstHost,
						IN int a_iCufOffVersion);


static int tdr_input_by_strict_xml_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot, 
			  IN int a_iCutOffVersion, IN int a_iIOVersion);

static int tdr_input_by_xml_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot, 
									 IN int a_iCutOffVersion);



static scew_element * tdr_input_root_i(IN scew_element *a_pstRoot, IN const char *a_pszRootName);

static int tdr_input_by_xml_oldversion_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot, 
										 IN int a_iCutOffVersion);

/*从XML数据树节点中读入简单类型成员的值到内存空间中
*@param[in] a_pstLib 元数据描述库指针
*@param[in] a_pstEntry 指定成员描述结构的指针
*@param[in,out] a_piCount 
*	- in 指定最大读取的元素个数
*	- out	输出实际读取的元素个数
*@param[in] a_pstRoot xml数据的根节点指针
*@param[in,out] a_ppstItem
*	- in	起始查找的儿子节点指针
*	- out	下一次查找的儿子节点指针
*@param[in, out] a_ppszHostStart 保存成员值的内存空间指针
*	- in	保存数据内存空间起始地址
*	- out	下一个成员可以使用的数据内存空间起始地址
*@param[in] a_pszHostEnd 可使用内存空间终止地址
*/
int tdr_input_simple_entry_i(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, INOUT int *a_piCount, 
							 IN scew_element *a_pstRoot, INOUT scew_element **a_ppstItem,
							 INOUT char **a_ppszHostStart, IN char *a_pszHostEnd);


extern char g_szEncoding[128];

////////////////////////////////////////////////////////////////////////////////////////////////////
int tdr_input(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN LPTDRDATA a_pstXml, 
			  IN int a_iCutOffVersion, IN int a_iIOVersion)
{
	scew_tree* pstTree = NULL;
	scew_element *pstRoot;
	int iRet = TDR_SUCCESS;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_pstHost);
	assert(NULL != a_pstXml); */
	if ((NULL == a_pstMeta) || (NULL == a_pstXml)|| (NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if ((NULL == a_pstXml->pszBuff) || (0 >= a_pstXml->iBuff) || 
		(NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

	if ((TDR_IO_OLD_XML_VERSION != a_iIOVersion) && (TDR_IO_NEW_XML_VERSION != a_iIOVersion))
	{
		a_iIOVersion = TDR_IO_NEW_XML_VERSION;
	}
	if (0 == a_iCutOffVersion)
	{
		a_iCutOffVersion = TDR_MAX_VERSION;
	}
	if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
	{
		a_pstHost->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	/*创建XML元素树*/    
	iRet = tdr_create_XMLParser_tree_byBuff_i(&pstTree, a_pstXml->pszBuff, a_pstXml->iBuff, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	pstRoot = scew_tree_root(pstTree);
	if (NULL == pstRoot)
	{
		a_pstHost->iBuff = 0;
		iRet = TDR_ERRIMPLE_NO_XML_ROOT;
	}


	if (!TDR_ERR_IS_ERROR(iRet))
	{		
		iRet = tdr_input_i(a_pstMeta, a_pstHost, pstRoot, a_iCutOffVersion, a_iIOVersion);		
	}

	scew_tree_free( pstTree );

	return iRet;
}

int tdr_input_fp(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN FILE *a_fp, 
				 IN int a_iCutOffVersion, IN int a_iIOVersion)
{
	scew_tree* pstTree = NULL;
	scew_element *pstRoot;
	int iRet = TDR_SUCCESS;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_pstHost);
	assert(NULL != a_fp);*/
	if ((NULL == a_pstMeta) || (NULL == a_fp)|| (NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if ((NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

	if ((TDR_IO_OLD_XML_VERSION != a_iIOVersion) && (TDR_IO_NEW_XML_VERSION != a_iIOVersion))
	{
		a_iIOVersion = TDR_IO_NEW_XML_VERSION;
	}
	if (0 == a_iCutOffVersion)
	{
		a_iCutOffVersion = TDR_MAX_VERSION;
	}
	if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
	{
		a_pstHost->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	/*创建XML元素树*/    
	iRet = tdr_create_XMLParser_tree_byfp(&pstTree, a_fp, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	pstRoot = scew_tree_root(pstTree);
	if (NULL == pstRoot)
	{
		a_pstHost->iBuff = 0;
		iRet = TDR_ERRIMPLE_NO_XML_ROOT;
	}

	if (!TDR_ERR_IS_ERROR(iRet))
	{		
		iRet = tdr_input_i(a_pstMeta, a_pstHost, pstRoot, a_iCutOffVersion, a_iIOVersion);		
	}

	scew_tree_free( pstTree );

	return iRet;
}

int tdr_input_file(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN const char *a_pszFile, 
				   IN int a_iCutOffVersion, IN int a_iIOVersion)
{
	scew_tree* pstTree = NULL;
	scew_element *pstRoot;
	int iRet = TDR_SUCCESS;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_pstHost);
	assert(NULL != a_pszFile); */
	if ((NULL == a_pstMeta) || (NULL == a_pszFile)|| (NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if ((NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

	if ((TDR_IO_OLD_XML_VERSION != a_iIOVersion) && (TDR_IO_NEW_XML_VERSION != a_iIOVersion))
	{
		a_iIOVersion = TDR_IO_NEW_XML_VERSION;
	}
	if (0 == a_iCutOffVersion)
	{
		a_iCutOffVersion = TDR_MAX_VERSION;
	}
	if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
	{
		a_pstHost->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	/*创建XML元素树*/    
	iRet = tdr_create_XMLParser_tree_byFileName(&pstTree, a_pszFile, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	pstRoot = scew_tree_root(pstTree);
	if (NULL == pstRoot)
	{
		a_pstHost->iBuff = 0;
		iRet = TDR_ERRIMPLE_NO_XML_ROOT;
	}

	if (!TDR_ERR_IS_ERROR(iRet))
	{		
		iRet = tdr_input_i(a_pstMeta, a_pstHost, pstRoot, a_iCutOffVersion, a_iIOVersion);		
	}

	scew_tree_free( pstTree );

	return iRet;
}

int tdr_input_by_strict_xml_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot, 
							  IN int a_iCutOffVersion, IN int a_iIOVersion)
{
	int iRet = TDR_SUCCESS;	
	LPTDRMETALIB pstLib;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	int iCutOffVersion;
	char *pszHostStart;
	char *pszHostEnd;
	int iChange;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstRoot);
	assert(NULL != a_pstHost);
	assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);


	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstCurMeta = a_pstMeta;
	pstStackTop = &stStack[0];
	pstStackTop->pszNetBase  = (char *)a_pstRoot; /*the parent node */
	pstStackTop->pszMetaSizeInfoTarget = NULL; /*the current node*/
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->pszHostBase = pszHostStart;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->iChange = 0;
	pstStackTop->iEntrySizeInfoOff = 0; /*实际读取的结构数*/
	TDR_GET_VERSION_INDICATOR_BY_XML(pstCurMeta, a_pstRoot, iCutOffVersion, a_iCutOffVersion);	
	pstStackTop->iCutOffVersion = iCutOffVersion;
	pstStackTop->szMetaEntryName[0] = '\0';
	iStackItemCount = 1;
	pstStackTop->pszHostEnd = pszHostEnd;

	iChange = 0;
	while (0 < iStackItemCount)
	{
		LPTDRMETAENTRY pstEntry;
		int iCount;
		scew_element *pstParent;
		scew_element *pstCurNode;


		if (0 != iChange) 
		{
			if (1 >= iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pstStackTop->iEntrySizeInfoOff++;  /*已经读取一个结构/uion*/
			if (TDR_IO_NEW_XML_VERSION == a_iIOVersion)
			{
				(pstStackTop -1)->pszMetaSizeInfoTarget = pstStackTop->pszNetBase;
				if (0 < pstStackTop->iCount)
				{
					TDR_GET_NEXT_ELEMET(pstParent, (pstStackTop-1)->pszNetBase, (pstStackTop-1)->pszMetaSizeInfoTarget, pstStackTop->szMetaEntryName);
					if (NULL == pstParent)
					{
						iChange = 0;
						TDR_CLEAR_COUNT(pstStackTop, pstCurMeta, pszHostStart, pszHostEnd);
						continue;
					}

					(pstStackTop -1)->pszMetaSizeInfoTarget = (char *)pstParent;					
					pstStackTop->pszNetBase = (char *)pstParent;
					pstStackTop->pszMetaSizeInfoTarget = NULL;
				}/*if (0 < pstStackTop->iCount)*/	
			}else
			{
				if (TDR_TYPE_STRUCT == pstCurMeta->iType)
				{
					(pstStackTop -1)->pszMetaSizeInfoTarget = pstStackTop->pszNetBase;
					if (0 < pstStackTop->iCount)
					{
						TDR_GET_NEXT_ELEMET(pstParent, (pstStackTop-1)->pszNetBase, (pstStackTop-1)->pszMetaSizeInfoTarget,  pstCurMeta->szName);
						if (NULL == pstParent)
						{
							iChange = 0;
							TDR_CLEAR_COUNT(pstStackTop, pstCurMeta, pszHostStart, pszHostEnd);
							continue;
						}

						(pstStackTop -1)->pszMetaSizeInfoTarget = (char *)pstParent;					
						pstStackTop->pszNetBase = (char *)pstParent;
						pstStackTop->pszMetaSizeInfoTarget = NULL;
					}/*if (0 < pstStackTop->iCount)*/	
				}else
				{
					(pstStackTop -1)->pszMetaSizeInfoTarget = pstStackTop->pszMetaSizeInfoTarget;
					if (0 < pstStackTop->iCount)
					{
						pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
						if (TDR_TYPE_STRUCT == pstEntry->iType)
						{
							LPTDRMETA pstType = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
							TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget,  pstType->szName);
						}else
						{
							TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget, pstEntry->szName);	
						}
						if (NULL == pstParent)
						{
							iChange = 0;
							TDR_CLEAR_COUNT(pstStackTop, pstCurMeta, pszHostStart, pszHostEnd);
							continue;
						}
					}/*if (0 < pstStackTop->iCount)*/												
				}/*if (TDR_TYPE_STRUCT == pstCurMeta->iType)*/
			}/*if (TDR_IO_NEW_XML_VERSION == a_iIOVersion)*/			
		}/*if ((0 != iChange)*/

		iChange = 0;

		if (0 >= pstStackTop->iCount)
		{/*当前元数据数组已经处理完毕*/
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstCurMeta = pstStackTop->pstMeta;
				pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
				TDR_SET_ARRAY_REAL_COUNT((pstStackTop + 1)->iEntrySizeInfoOff, pstEntry, pstStackTop->pszHostBase);
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;

		if (TDR_IO_NOINPUT & pstEntry->iIO)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		/*取出此entry的数组计数信息*/	
		TDR_GET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase, a_iCutOffVersion); 
		if (0 > iCount)
		{	/*实际数目为负数或比数组最大长度要大，则无效*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}


		/* 设置最大读取成员数目：
		*  1)如果通过refer属性指定了成员的数目，则最大读取数目按此refer指定数目读取；
		*  2)如果没有通过refer属性指定了成员的数目:
		*		2.1）如果成员数组长度确定(icount > 0),则最大读取数目按成员数组长度设置；
		*		2.2) 如果成员数组长度确定(icount == 0),则最大读取数目设置为最大整数;
		*/
		if ((0 < pstEntry->iCount) && (pstEntry->iCount < iCount)) 
		{
			iCount = pstEntry->iCount;
		}
		if( (0 == iCount) && (pstEntry->iCount == 0))
		{
			iCount = TDR_MAX_INT;			
		}
		

		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
		pszHostEnd = pstStackTop->pszHostEnd;
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			pszHostStart = *(char **)pszHostStart;
			pszHostEnd = pszHostStart + pstEntry->iHRealSize;
		}
		if (pstEntry->iVersion > pstStackTop->iCutOffVersion)
		{
			TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, pstEntry->iCount);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
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
			}/*if (TDR_TYPE_UNION == pstEntry->iType)*/

			/*定位节点*/
			if (TDR_IO_NEW_XML_VERSION == a_iIOVersion)
			{
				TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget, pstEntry->szName);
				pstCurNode = NULL;
			}else
			{
				if (TDR_TYPE_UNION == pstEntry->iType)
				{
					pstParent = (scew_element *)pstStackTop->pszNetBase;
					pstCurNode = (scew_element *)pstStackTop->pszMetaSizeInfoTarget;				
				}else
				{
					TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget, pstTypeMeta->szName);
					pstCurNode = NULL;
				}
			}/*if (TDR_IO_NEW_XML_VERSION == a_iIOVersion)*/
			if (NULL == pstParent)
			{
				TDR_SET_ARRAY_REAL_COUNT(0, pstEntry, pstStackTop->pszHostBase);
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				continue;
			}

			pstCurMeta = pstTypeMeta;
			iStackItemCount++;
			pstStackTop++;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = iCount;
			pstStackTop->idxEntry = idxSubEntry;
			pstStackTop->pszHostBase = pszHostStart;
			pstStackTop->pszNetBase = (char *)pstParent;
			pstStackTop->pszMetaSizeInfoTarget = (char *)pstCurNode;
			pstStackTop->pszHostEnd = pszHostEnd;

			/*实际读取的结构数*/
			pstStackTop->iEntrySizeInfoOff = 0; 
			TDR_GET_VERSION_INDICATOR_BY_XML(pstCurMeta, a_pstRoot, iCutOffVersion, a_iCutOffVersion);	
			pstStackTop->iCutOffVersion = iCutOffVersion;
			if (TDR_IO_NEW_XML_VERSION == a_iIOVersion)
			{
				TDR_STRNCPY(pstStackTop->szMetaEntryName, pstEntry->szName, sizeof(pstStackTop->szMetaEntryName));
			}			
			pstStackTop->iChange = 1;
			continue;
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/
		
		iRet = tdr_input_simple_entry_i(pstLib, pstEntry, &iCount, (scew_element *)pstStackTop->pszNetBase,
			(scew_element **)&pstStackTop->pszMetaSizeInfoTarget, &pszHostStart, pszHostEnd);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}
		TDR_SET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase);
		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/

	a_pstHost->iBuff = pszHostStart - a_pstHost->pszBuff;
	return iRet;
}

int tdr_output(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstXml, IN LPTDRDATA a_pstHost,
			   IN int a_iCutOffVersion, IN int a_iIOVersion)
{
	int iRet = TDR_SUCCESS;	
	TDRIOSTREAM stIOStream;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_pstXml);
	assert(NULL != a_pstXml->pszBuff);
	assert(0 < a_pstXml->iBuff);
	assert(NULL != a_pstHost);
	assert(NULL != a_pstHost->pszBuff);
	assert(0 < a_pstHost->iBuff);*/
	if ((NULL == a_pstMeta) || (NULL == a_pstXml)|| (NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if ((NULL == a_pstXml->pszBuff) || (0 >= a_pstXml->iBuff) || 
		(NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

	if ((TDR_IO_OLD_XML_VERSION != a_iIOVersion) && (TDR_IO_NEW_XML_VERSION != a_iIOVersion))
	{
		a_iIOVersion = TDR_IO_NEW_XML_VERSION;
	}
	if ((0 == a_iCutOffVersion) || (a_iCutOffVersion > a_pstMeta->iCurVersion))
	{
		a_iCutOffVersion = a_pstMeta->iCurVersion;;
	}
	if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}
	
	stIOStream.emIOStreamType = TDR_IOSTREAM_STRBUF;
	stIOStream.pszTDRIOBuff = a_pstXml->pszBuff;
	stIOStream.iTDRIOBuffLen = a_pstXml->iBuff;

	if (TDR_IO_NEW_XML_VERSION == a_iIOVersion)
	{
		iRet = tdr_output_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
	}else
	{
		iRet = tdr_output_oldversion_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
	}
	a_pstXml->iBuff = stIOStream.pszTDRIOBuff - a_pstXml->pszBuff;


	return iRet;
}

int tdr_output_fp(IN LPTDRMETA a_pstMeta, IN FILE *a_fpXml, IN LPTDRDATA a_pstHost,
				  IN int a_iCutOffVersion, IN int a_iIOVersion)
{
	int iRet = TDR_SUCCESS;	
	TDRIOSTREAM stIOStream;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_fpXml);	
	assert(NULL != a_pstHost);
	assert(NULL != a_pstHost->pszBuff);
	assert(0 < a_pstHost->iBuff);*/
	if ((NULL == a_pstMeta) || (NULL == a_fpXml)|| (NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if ((NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}

	if ((TDR_IO_OLD_XML_VERSION != a_iIOVersion) && (TDR_IO_NEW_XML_VERSION != a_iIOVersion))
	{
		a_iIOVersion = TDR_IO_NEW_XML_VERSION;
	}
	if ((0 == a_iCutOffVersion) || (a_iCutOffVersion > a_pstMeta->iCurVersion))
	{
		a_iCutOffVersion = a_pstMeta->iCurVersion;;
	}
	if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
	{
		a_pstHost->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	stIOStream.emIOStreamType = TDR_IOSTREAM_FILE;
	stIOStream.fpTDRIO = a_fpXml;
	
	if (TDR_IO_NEW_XML_VERSION == a_iIOVersion)
	{
		iRet = tdr_output_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
	}else
	{
		iRet = tdr_output_oldversion_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
	}
	return iRet;
}

int tdr_output_file(IN LPTDRMETA a_pstMeta, IN const char *a_szFile, IN LPTDRDATA a_pstHost,
				  IN int a_iCutOffVersion, IN int a_iIOVersion)
{
	int iRet = TDR_SUCCESS;	
	TDRIOSTREAM stIOStream;
	FILE *fp;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_szFile);	
	assert(NULL != a_pstHost);
	assert(NULL != a_pstHost->pszBuff);
	assert(0 < a_pstHost->iBuff);*/
	if ((NULL == a_pstMeta) || (NULL == a_szFile)|| (NULL == a_pstHost))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}
	if ((NULL == a_pstHost->pszBuff)||(0 >= a_pstHost->iBuff))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_PARAM);
	}



	if ((TDR_IO_OLD_XML_VERSION != a_iIOVersion) && (TDR_IO_NEW_XML_VERSION != a_iIOVersion))
	{
		a_iIOVersion = TDR_IO_NEW_XML_VERSION;
	}
	if ((0 == a_iCutOffVersion) || (a_iCutOffVersion > a_pstMeta->iCurVersion))
	{
		a_iCutOffVersion = a_pstMeta->iCurVersion;;
	}
	if (a_pstMeta->iBaseVersion > a_iCutOffVersion)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	fp = fopen(a_szFile, "w");
	if (NULL == fp)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_OPEN_FILE_TO_WRITE);
	}
	
	
	fprintf(fp,"<?xml version=\"1.0\" encoding=\"%s\" standalone=\"yes\" ?>\n",
		g_szEncoding);
	
	stIOStream.emIOStreamType = TDR_IOSTREAM_FILE;
	stIOStream.fpTDRIO = fp;
	if (TDR_IO_NEW_XML_VERSION == a_iIOVersion)
	{
		iRet = tdr_output_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
	}else
	{
		iRet = tdr_output_oldversion_i(a_pstMeta, &stIOStream, a_pstHost, a_iCutOffVersion);
	}
	

	fclose(fp);

	return iRet;
}

static int tdr_output_simpleentry_i(INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETALIB a_pstLib, 
									IN LPTDRMETAENTRY a_pstEntry, IN int a_iCount,
									IN char *a_pszHostStart, IN char *a_pszHostEnd, IN char *a_pszSpace)
{
	int iRet = TDR_SUCCESS;
	int i;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstEntry);
	assert(NULL != a_pszHostStart);
	assert(NULL != a_pszHostEnd);
	assert(NULL != a_pszSpace);


	switch(a_pstEntry->iType)
	{
	case TDR_TYPE_STRING:
		{
#define TDR_XML_ESC_MIN_BUFF	128
			int iSize, iXmlLen;
			char *pszXml = NULL;
			char *pchEnd;
			char *pSrc;
			char *pDst;
			int iEscLen;
			for (i = 0; i < a_iCount; i++)												
			{		
				iRet = tdr_iostream_write(a_pstIOStream, "%s<%s>",a_pszSpace, a_pstEntry->szName);

				if (0 < a_pstEntry->iCustomHUnitSize)												
				{																					
					iSize = a_pstEntry->iCustomHUnitSize;											
				}else																				
				{																					
					iSize = a_pszHostEnd - a_pszHostStart;
				}

				/*对xml转换字符进行处理*/
				pchEnd = a_pszHostStart + iSize;
				pszXml = malloc(iSize + TDR_XML_ESC_MIN_BUFF);
				if (NULL == pszXml)
				{
					return  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
				}
				iXmlLen = iSize + TDR_XML_ESC_MIN_BUFF;
				pSrc = a_pszHostStart;
				pDst = pszXml;
				iEscLen = 0;
				for (;(pSrc != pchEnd);)
				{
					switch(*pSrc)
					{
					case '<': /*&lt;*/
						if (iEscLen >= (TDR_XML_ESC_MIN_BUFF - 4))
						{
							int iDataLen = pDst - pszXml;
							pszXml = realloc(pszXml, iXmlLen + TDR_XML_ESC_MIN_BUFF);
							if (NULL == pszXml)
							{
								return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
							}
							iXmlLen += TDR_XML_ESC_MIN_BUFF;
							iEscLen = 0;
							pDst = pszXml + iDataLen;
						}
						*pDst++ = '&';
						*pDst++ = 'l';
						*pDst++ = 't';
						*pDst++ = ';';
						iEscLen += 4;
						break;
					case '>':/*&gt;*/
						if (iEscLen >= (TDR_XML_ESC_MIN_BUFF - 4))
						{
							int iDataLen = pDst - pszXml;
							pszXml = realloc(pszXml, iXmlLen + TDR_XML_ESC_MIN_BUFF);
							if (NULL == pszXml)
							{
								return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
							}
							iXmlLen += TDR_XML_ESC_MIN_BUFF;
							iEscLen = 0;
							pDst = pszXml + iDataLen;
						}
						*pDst++ = '&';
						*pDst++ = 'g';
						*pDst++ = 't';
						*pDst++ = ';';
						iEscLen += 4;
						break;
					case '&':/*&amp;*/
						if (iEscLen >= (TDR_XML_ESC_MIN_BUFF - 5))
						{
							int iDataLen = pDst - pszXml;
							pszXml = realloc(pszXml, iXmlLen + TDR_XML_ESC_MIN_BUFF);
							if (NULL == pszXml)
							{
								return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
							}
							iXmlLen += TDR_XML_ESC_MIN_BUFF;
							iEscLen = 0;
							pDst = pszXml + iDataLen;
						}
						*pDst++ = '&';
						*pDst++ = 'a';
						*pDst++ = 'm';
						*pDst++ = 'p';
						*pDst++ = ';';
						iEscLen += 5;
						break;
					case '\'': /*&apos;*/
						if (iEscLen >= (TDR_XML_ESC_MIN_BUFF - 6))
						{
							int iDataLen = pDst - pszXml;
							pszXml = realloc(pszXml, iXmlLen + TDR_XML_ESC_MIN_BUFF);
							if (NULL == pszXml)
							{
								return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
							}
							iXmlLen += TDR_XML_ESC_MIN_BUFF;
							iEscLen = 0;
							pDst = pszXml + iDataLen;
						}
						*pDst++ = '&';
						*pDst++ = 'a';
						*pDst++ = 'p';
						*pDst++ = 'o';
						*pDst++ = 's';
						*pDst++ = ';';
						iEscLen += 6;
						break;
					case '"':/*&quot;*/
						if (iEscLen >= (TDR_XML_ESC_MIN_BUFF - 6))
						{
							int iDataLen = pDst - pszXml;
							pszXml = realloc(pszXml, iXmlLen + TDR_XML_ESC_MIN_BUFF);
							if (NULL == pszXml)
							{
								return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
							}
							iXmlLen += TDR_XML_ESC_MIN_BUFF;
							iEscLen = 0;
							pDst = pszXml + iDataLen;
						}
						*pDst++ = '&';
						*pDst++ = 'q';
						*pDst++ = 'u';
						*pDst++ = 'o';
						*pDst++ = 't';
						*pDst++ = ';';
						iEscLen += 6;
						break;
					default:
						/*非转义字符*/
						*pDst++ = *pSrc;
					}/*switch(*pSrc)*/	
					if (*pSrc == '\0')
					{
						break;
					}
					pSrc++;
				}/*for (;pSrc != pchEnd;)*/
				if (pSrc == pchEnd)
				{
					free(pszXml);
					return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NET_INVALID_STRING_LEN);				
				}	
				iRet = tdr_iostream_write(a_pstIOStream, "%s", pszXml);				
				iRet = tdr_iostream_write(a_pstIOStream, "</%s>\n", a_pstEntry->szName);				
				if (TDR_ERR_IS_ERROR(iRet))
				{
					break;
				}
				free(pszXml);
				a_pszHostStart += iSize;
			}/*for (i = 0; i < a_iCount; i++)	*/
			break;
		}
	case TDR_TYPE_DATETIME:
		{
			if((a_pszHostStart + a_pstEntry->iHUnitSize*a_iCount) > a_pszHostEnd)
			{
				iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			for (i = 0; i < a_iCount; i++)												
			{																						
				iRet = tdr_iostream_write(a_pstIOStream, "%s<%s>",a_pszSpace, a_pstEntry->szName);
				iRet = tdr_iostream_write(a_pstIOStream, "%s", tdr_tdrdatetime_to_str((tdr_datetime_t *)a_pszHostStart));
				a_pszHostStart += a_pstEntry->iHUnitSize;				
				iRet = tdr_iostream_write(a_pstIOStream, "</%s>\n", a_pstEntry->szName);								
				if (TDR_ERR_IS_ERROR(iRet))
				{
					break;
				}
			}/*for (i = 0; i < a_iCount; i++)	*/
		}
		break;
	default:
		{
			if((a_pszHostStart + a_pstEntry->iHUnitSize*a_iCount) > a_pszHostEnd)
			{
				iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
	
			iRet = tdr_iostream_write(a_pstIOStream, "%s<%s>",a_pszSpace, a_pstEntry->szName);		
			for (i = 0; i < a_iCount; i++)												
			{		
				iRet = tdr_ioprintf_basedtype_i(a_pstIOStream, a_pstLib, a_pstEntry, &a_pszHostStart, a_pszHostEnd);				
			}/*for (i = 0; i < a_iCount; i++)	*/
			iRet = tdr_iostream_write(a_pstIOStream, "</%s>\n", a_pstEntry->szName);
		}
		break;
	}/*switch(a_pstEntry->iType)*/

	return iRet;
}

int tdr_output_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRDATA a_pstHost,
			   IN int a_iCufOffVersion)
{
	int iRet = TDR_SUCCESS;	
	LPTDRMETALIB pstLib;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	int iCutOffVersion;
	char szSpace[TDR_STACK_SIZE*TDR_TAB_SIZE+1];
	int i;
	
	char *pszHostStart;
	char *pszHostEnd;
	int iChange;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstIOStream);
	assert(NULL != a_pstHost);
	assert(a_pstMeta->iBaseVersion <= a_iCufOffVersion);

	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	pstCurMeta = a_pstMeta;
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	for (i = 0; i < (int)(sizeof(szSpace)); i++)
	{
		szSpace[i] = ' ';
	}
	szSpace[sizeof(szSpace) -1] = '\0';

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->pszHostBase = pszHostStart;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->iChange = 0;
	TDR_GET_VERSION_INDICATOR(iRet, pszHostStart, pszHostEnd, pstCurMeta, iCutOffVersion, a_iCufOffVersion);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		a_pstHost->iBuff = 0;
		return iRet;
	}
	pstStackTop->iCutOffVersion = iCutOffVersion;
	pstStackTop->szMetaEntryName[0] = '\0';
	pstStackTop->iMetaSizeInfoOff = 1; /*框架层次*/
	iStackItemCount = 1;
	
	

	iRet = tdr_iostream_write(a_pstIOStream, "<%s version=\"%d\">\n", a_pstMeta->szName, pstStackTop->iCutOffVersion);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}
	
	iChange = 0;
	while (0 < iStackItemCount)
	{
		LPTDRMETAENTRY pstEntry;
		int iArrayRealCount ;	

		if (0 != iChange) 
		{
			iChange = 0;	
			szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
			iRet = tdr_iostream_write(a_pstIOStream, "%s</%s>\n", szSpace, pstStackTop->szMetaEntryName);			
			szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

			if (0 < pstStackTop->iCount)
			{
				szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
				if (a_iCufOffVersion != pstStackTop->iCutOffVersion)
				{
					iRet = tdr_iostream_write(a_pstIOStream, "%s<%s type=\"%s\" version=\"%d\">\n", 
						szSpace, pstStackTop->szMetaEntryName, pstCurMeta->szName, pstStackTop->iCutOffVersion);
				}else
				{
					iRet = tdr_iostream_write(a_pstIOStream, "%s<%s type=\"%s\">\n", 
						szSpace, pstStackTop->szMetaEntryName, pstCurMeta->szName);
				}
				szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';
				if (TDR_ERR_IS_ERROR(iRet))
				{
					break;
				}
			}/*if (0 < pstStackTop->iCount)*/
		}/*if ((0 != iChange) && (TDR_TYPE_STRUCT == pstCurMeta->iType))*/

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
		if (TDR_IO_NOOUTPUT & pstEntry->iIO)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

		/*取出此entry的数组计数信息*/	
		TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, pstStackTop->iCutOffVersion); 
		if ((iArrayRealCount < 0) || 
			((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount)))
		{/*实际数目为负数或比数组最大长度要大，则无效*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}
		if (0 >= iArrayRealCount)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}


		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{
			/*复合数据类型*/
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
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = iArrayRealCount;
			pstStackTop->idxEntry = idxSubEntry;
			pstStackTop->pszHostBase = pszHostStart;			
			TDR_GET_VERSION_INDICATOR(iRet, pszHostStart, pszHostEnd,
				pstCurMeta, iCutOffVersion, a_iCufOffVersion);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			pstStackTop->iCutOffVersion = iCutOffVersion;
			pstStackTop->iMetaSizeInfoOff = (pstStackTop - 1)->iMetaSizeInfoOff + 1;
			pstStackTop->iChange = 1;
			szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
			if (a_iCufOffVersion != pstStackTop->iCutOffVersion)
			{
				iRet = tdr_iostream_write(a_pstIOStream, "%s<%s type=\"%s\" version=\"%d\">\n", 
					szSpace, pstEntry->szName, pstTypeMeta->szName, pstStackTop->iCutOffVersion);
			}else
			{
				iRet = tdr_iostream_write(a_pstIOStream, "%s<%s type=\"%s\">\n", 
					szSpace, pstEntry->szName, pstTypeMeta->szName);
			}					
			TDR_STRNCPY(pstStackTop->szMetaEntryName, pstEntry->szName, sizeof(pstStackTop->szMetaEntryName));
			szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';				
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

			continue;
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/
		
		/*基本数据类型*/
		szSpace[pstStackTop->iMetaSizeInfoOff*TDR_TAB_SIZE] = '\0';
		iRet = tdr_output_simpleentry_i(a_pstIOStream, pstLib, pstEntry, iArrayRealCount,
			pszHostStart, pszHostEnd, &szSpace[0]);
		szSpace[pstStackTop->iMetaSizeInfoOff*TDR_TAB_SIZE] = ' ';		
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}

		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
		
	}/*while (0 < iStackItemCount)*/

	
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_iostream_write(a_pstIOStream, "</%s>\n", a_pstMeta->szName);		
	}
	return iRet;
}

int tdr_output_oldversion_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream, IN LPTDRDATA a_pstHost,
				 IN int a_iCufOffVersion)
{
	int iRet = TDR_SUCCESS;	
	LPTDRMETALIB pstLib;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	char szSpace[TDR_STACK_SIZE*TDR_TAB_SIZE+1];
	int i;

	char *pszHostStart;
	char *pszHostEnd;
	int iChange;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstIOStream);
	assert(NULL != a_pstHost);
	assert(a_pstMeta->iBaseVersion <= a_iCufOffVersion);

	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	pstCurMeta = a_pstMeta;
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	for (i = 0; i < (int)(sizeof(szSpace)); i++)
	{
		szSpace[i] = ' ';
	}
	szSpace[sizeof(szSpace) -1] = '\0';

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->pszHostBase = pszHostStart;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->iChange = 0;	
	pstStackTop->szMetaEntryName[0] = '\0';
	pstStackTop->iMetaSizeInfoOff = 1; /*框架层次*/
	iStackItemCount = 1;
	pstStackTop->iCutOffVersion = a_iCufOffVersion;

	
	iRet = tdr_iostream_write(a_pstIOStream, "<%s version=\"%d\">\n", a_pstMeta->szName, a_iCufOffVersion);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	iChange = 0;
	while (0 < iStackItemCount)
	{
		LPTDRMETAENTRY pstEntry;
		int iArrayRealCount ;	

		if (0 != iChange) 
		{
			iChange = 0;	
			szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
			iRet = tdr_iostream_write(a_pstIOStream, "%s</%s>\n", szSpace, pstCurMeta->szName);			
			szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

			if (0 < pstStackTop->iCount)
			{
				szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
				if (a_iCufOffVersion != pstStackTop->iCutOffVersion)
				{
					iRet = tdr_iostream_write(a_pstIOStream, "%s<%s version=\"%d\">\n",
						szSpace, pstCurMeta->szName, pstStackTop->iCutOffVersion);
				}else
				{
					iRet = tdr_iostream_write(a_pstIOStream, "%s<%s>\n",
						szSpace, pstCurMeta->szName);
				}
				szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';
				if (TDR_ERR_IS_ERROR(iRet))
				{
					break;
				}
			}/*if (0 < pstStackTop->iCount)*/
		}/*if ((0 != iChange) && (TDR_TYPE_STRUCT == pstCurMeta->iType))*/

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
		if (pstEntry->iVersion > a_iCufOffVersion)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (TDR_IO_NOOUTPUT & pstEntry->iIO)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}		
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;

		/*取出此entry的数组计数信息*/	
		TDR_GET_ARRAY_REAL_COUNT(iArrayRealCount, pstEntry, pstStackTop->pszHostBase, a_iCufOffVersion); 
		if ((iArrayRealCount < 0) || 
			((0 < pstEntry->iCount) && (pstEntry->iCount < iArrayRealCount)))
		{/*实际数目为负数或比数组最大长度要大，则无效*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}
		if (0 >= iArrayRealCount)
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}


		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{
			/*复合数据类型*/
			int idxSubEntry;
			LPTDRMETA pstTypeMeta;


			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			if (TDR_TYPE_UNION == pstEntry->iType)
			{
				TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, a_iCufOffVersion, pstTypeMeta, idxSubEntry);
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
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = iArrayRealCount;
			pstStackTop->idxEntry = idxSubEntry;
			pstStackTop->pszHostBase = pszHostStart;			
			pstStackTop->iMetaSizeInfoOff = (pstStackTop - 1)->iMetaSizeInfoOff + 1;	
			TDR_GET_VERSION_INDICATOR(iRet, pszHostStart, pszHostEnd,
				pstCurMeta, pstStackTop->iCutOffVersion, a_iCufOffVersion);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			if (TDR_TYPE_STRUCT == pstEntry->iType)
			{	
				pstStackTop->iChange = 1;
				szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = '\0';
				if (a_iCufOffVersion != pstStackTop->iCutOffVersion)
				{
					iRet = tdr_iostream_write(a_pstIOStream, "%s<%s version=\"%d\">\n", 
						szSpace, pstTypeMeta->szName, pstStackTop->iCutOffVersion);
				}else
				{
					iRet = tdr_iostream_write(a_pstIOStream, "%s<%s>\n", 
						szSpace, pstTypeMeta->szName);
				}
				szSpace[(pstStackTop->iMetaSizeInfoOff-1)*TDR_TAB_SIZE] = ' ';
			}else
			{
				pstStackTop->iChange = 0;
			}
			
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

			continue;
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/

		/*基本数据类型*/
		szSpace[pstStackTop->iMetaSizeInfoOff*TDR_TAB_SIZE] = '\0';
		iRet = tdr_output_simpleentry_i(a_pstIOStream, pstLib, pstEntry, iArrayRealCount,
			pszHostStart, pszHostEnd, &szSpace[0]);
		szSpace[pstStackTop->iMetaSizeInfoOff*TDR_TAB_SIZE] = ' ';

		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}

		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/

	
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_iostream_write(a_pstIOStream, "</%s>\n", a_pstMeta->szName);		
	}
	return iRet;
}

scew_element * tdr_input_root_i(IN scew_element *a_pstRoot, IN const char *a_pszRootName)
{
	scew_element *pstInputRoot = NULL;

	assert(NULL != a_pstRoot);
	assert(NULL != a_pszRootName);

	/* first check the root element. */
	if (0 == tdr_stricmp(scew_element_name(a_pstRoot), TDR_TAG_DATASET))
	{
		pstInputRoot = scew_element_by_name(a_pstRoot, a_pszRootName);
		if (NULL != pstInputRoot)
		{
			return pstInputRoot;
		}
	} 
	
	/*与根节点匹配*/
	if (0 == tdr_stricmp(scew_element_name(a_pstRoot), a_pszRootName))
	{
		return a_pstRoot;
	}
	
	/*匹配根节点的儿子节点*/
	pstInputRoot = scew_element_next(a_pstRoot, NULL);
	while (NULL != pstInputRoot)
	{
		scew_attribute *pstAttr;
		
		if (0 == tdr_stricmp(scew_element_name(pstInputRoot), a_pszRootName))
		{
			break;
		}	

		
		pstAttr = scew_attribute_by_name(pstInputRoot, TDR_TAG_TYPE);    
    		if( NULL == pstAttr )
    		{
       		pstInputRoot = scew_element_next(a_pstRoot, pstInputRoot);
       		continue;
    		}
    		if (0 == tdr_stricmp(scew_attribute_value(pstAttr),  a_pszRootName))
		{
			break;
		}	
    		
		pstInputRoot = scew_element_next(a_pstRoot, pstInputRoot);
	}/*while (NULL != pstInputRoot)*/

	return pstInputRoot;
}

int tdr_input_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot, 
					   IN int a_iCutOffVersion, IN int a_iIOVersion)
{
	int iRet = TDR_SUCCESS;
	scew_element *pstInputRoot;
	

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstRoot);
	assert(NULL != a_pstHost);
	assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);
	assert(NULL != a_pstHost->pszBuff);
	assert(0 < a_pstHost->iBuff);

	pstInputRoot = tdr_input_root_i(a_pstRoot, a_pstMeta->szName);
	if (NULL == pstInputRoot)
	{
		a_pstHost->iBuff = 0;
		return TDR_ERRIMPLE_NO_XML_ROOT;
	}

	if (TDR_META_IS_STRICT_INPUT(a_pstMeta))
	{
		iRet = tdr_input_by_strict_xml_i(a_pstMeta, a_pstHost, pstInputRoot, a_iCutOffVersion, a_iIOVersion);
	}else
	{
		if (TDR_IO_NEW_XML_VERSION == a_iIOVersion)
		{
			iRet = tdr_input_by_xml_i(a_pstMeta, a_pstHost, pstInputRoot, a_iCutOffVersion);
		}else
		{
			iRet = tdr_input_by_xml_oldversion_i(a_pstMeta, a_pstHost, pstInputRoot, a_iCutOffVersion);
		}		
	}

	return iRet;
	
}

int tdr_input_by_xml_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot, 
							  IN int a_iCutOffVersion)
{
	int iRet = TDR_SUCCESS;	
	LPTDRMETALIB pstLib;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	int iCutOffVersion;
	char *pszHostStart;
	char *pszHostEnd;
	int iChange;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstRoot);
	assert(NULL != a_pstHost);
	assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);


	
	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstCurMeta = a_pstMeta;
	pstStackTop = &stStack[0];
	pstStackTop->pszNetBase  = (char *)a_pstRoot; /*the parent node */
	pstStackTop->pszMetaSizeInfoTarget = NULL; /*the current node*/
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->pszHostBase = pszHostStart;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->iChange = 0;
	pstStackTop->iEntrySizeInfoOff = 0; /*实际读取的结构数*/
	TDR_GET_VERSION_INDICATOR_BY_XML(pstCurMeta, a_pstRoot, iCutOffVersion, a_iCutOffVersion);	
	pstStackTop->iCutOffVersion = iCutOffVersion;
	pstStackTop->szMetaEntryName[0] = '\0';
	iStackItemCount = 1;
	pstStackTop->pszHostEnd = pszHostEnd;

	iChange = 0;
	while (0 < iStackItemCount)
	{
		LPTDRMETAENTRY pstEntry;
		int iCount;
		scew_element *pstParent;


		if (0 != iChange) 
		{
			if (1 >= iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pstStackTop->iEntrySizeInfoOff++;  /*已经读取一个结构/uion*/
			(pstStackTop -1)->pszMetaSizeInfoTarget = pstStackTop->pszNetBase;
			if (0 < pstStackTop->iCount)
			{
				TDR_GET_NEXT_ELEMET(pstParent, (pstStackTop-1)->pszNetBase, (pstStackTop-1)->pszMetaSizeInfoTarget, pstStackTop->szMetaEntryName);	
				if (NULL == pstParent)
				{
					iChange = 0;
					TDR_CLEAR_COUNT(pstStackTop, pstCurMeta, pszHostStart, pszHostEnd);
					continue;
				}
				pstStackTop->pszNetBase = (char *)pstParent;
			}/*if (0 < pstStackTop->iCount)*/
		}/*if ((0 != iChange)*/

		iChange = 0;

		if (0 >= pstStackTop->iCount)
		{/*当前元数据数组已经处理完毕*/
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstCurMeta = pstStackTop->pstMeta;
				pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
				TDR_SET_ARRAY_REAL_COUNT((pstStackTop + 1)->iEntrySizeInfoOff, pstEntry, pstStackTop->pszHostBase);
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}	

		/*取出此entry的数组计数信息*/	
		TDR_GET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase, pstStackTop->iCutOffVersion); 
		if (0 > iCount)
		{	/*实际数目为负数或比数组最大长度要大，则无效*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}
		

		/* 设置最大读取成员数目：
		*  1)如果通过refer属性指定了成员的数目，则最大读取数目按此refer指定数目读取；
		*  2)如果没有通过refer属性指定了成员的数目:
		*		2.1）如果成员数组长度确定(icount > 0),则最大读取数目按成员数组长度设置；
		*		2.2) 如果成员数组长度确定(icount == 0),则最大读取数目设置为最大整数;
		*/
		if ((0 < pstEntry->iCount) && (pstEntry->iCount < iCount)) 
		{
			iCount = pstEntry->iCount;
		}
		if( (0 == iCount) && (pstEntry->iCount == 0))
		{
			iCount = TDR_MAX_INT;			
		}

		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
		pszHostEnd = pstStackTop->pszHostEnd;
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			pszHostStart = *(char **)pszHostStart;
			pszHostEnd = pszHostStart + pstEntry->iHRealSize;
		}
		if (TDR_IO_NOINPUT & pstEntry->iIO)
		{
			TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, pstEntry->iCount);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (pstEntry->iVersion > pstStackTop->iCutOffVersion)
		{
			TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, pstEntry->iCount);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		pstStackTop->pszMetaSizeInfoTarget = NULL;  /*从头开始搜索*/	

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
			}/*if (TDR_TYPE_UNION == pstEntry->iType)*/

			/*定位节点*/			
			TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget, pstEntry->szName);			
			if (NULL == pstParent)
			{
				TDR_SET_ARRAY_REAL_COUNT(0, pstEntry, pstStackTop->pszHostBase);
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				continue;
			}

			pstCurMeta = pstTypeMeta;
			iStackItemCount++;
			pstStackTop++;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = iCount;
			pstStackTop->idxEntry = idxSubEntry;
			pstStackTop->pszHostBase = pszHostStart;
			pstStackTop->pszNetBase = (char *)pstParent;
			pstStackTop->pszMetaSizeInfoTarget = NULL;	
			pstStackTop->iEntrySizeInfoOff = 0; /*实际读取的结构数*/
			TDR_GET_VERSION_INDICATOR_BY_XML(pstCurMeta, a_pstRoot, iCutOffVersion, a_iCutOffVersion);	
			pstStackTop->iCutOffVersion = iCutOffVersion;
			TDR_STRNCPY(pstStackTop->szMetaEntryName, pstEntry->szName, sizeof(pstStackTop->szMetaEntryName));						
			pstStackTop->iChange = 1;
			pstStackTop->pszHostEnd = pszHostEnd ;
			continue;
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/
		
		iRet = tdr_input_simple_entry_i(pstLib, pstEntry, &iCount, (scew_element *)pstStackTop->pszNetBase,
			(scew_element **)&pstStackTop->pszMetaSizeInfoTarget, &pszHostStart, pszHostEnd);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}
		TDR_SET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase);
		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/

	a_pstHost->iBuff = pszHostStart - a_pstHost->pszBuff;
	return iRet;
}


int tdr_input_by_xml_oldversion_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot, 
					   IN int a_iCutOffVersion)
{
	int iRet = TDR_SUCCESS;	
	LPTDRMETALIB pstLib;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	char *pszHostStart;
	char *pszHostEnd;
	int iChange;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstRoot);
	assert(NULL != a_pstHost);
	assert(a_pstMeta->iBaseVersion <= a_iCutOffVersion);



	pszHostStart = a_pstHost->pszBuff;
	pszHostEnd = a_pstHost->pszBuff + a_pstHost->iBuff;
	pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstCurMeta = a_pstMeta;
	pstStackTop = &stStack[0];
	pstStackTop->pszNetBase  = (char *)a_pstRoot; /*the parent node */
	pstStackTop->pszMetaSizeInfoTarget = NULL; /*the current node*/
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->pszHostBase = pszHostStart;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	pstStackTop->iChange = 0;
	pstStackTop->iEntrySizeInfoOff = 0; /*实际读取的结构数*/	
	pstStackTop->szMetaEntryName[0] = '\0';
	iStackItemCount = 1;
	pstStackTop->pszHostEnd = pszHostEnd;

	iChange = 0;
	while (0 < iStackItemCount)
	{
		LPTDRMETAENTRY pstEntry;
		int iCount;
		scew_element *pstParent;


		if (0 != iChange) 
		{
			if (1 >= iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pstStackTop->iEntrySizeInfoOff++;  /*已经读取一个结构/uion*/
			if (TDR_TYPE_STRUCT == pstCurMeta->iType)
			{
				(pstStackTop -1)->pszMetaSizeInfoTarget = pstStackTop->pszNetBase;
				if (0 < pstStackTop->iCount)
				{
					TDR_GET_NEXT_ELEMET(pstParent, (pstStackTop-1)->pszNetBase, (pstStackTop-1)->pszMetaSizeInfoTarget, pstCurMeta->szName);	
					if (NULL == pstParent)
					{
						iChange = 0;
						TDR_CLEAR_COUNT(pstStackTop, pstCurMeta, pszHostStart, pszHostEnd);
						continue;
					}
					pstStackTop->pszNetBase = (char *)pstParent;
				}/*if (0 < pstStackTop->iCount)*/	
			}else
			{
				(pstStackTop -1)->pszMetaSizeInfoTarget = pstStackTop->pszMetaSizeInfoTarget;
				if (0 < pstStackTop->iCount)
				{
					pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
					if (TDR_TYPE_STRUCT == pstEntry->iType)
					{
						LPTDRMETA pstType = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
						TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget, pstType->szName);
					}else
					{
						TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget, pstEntry->szName);	
					}
					if (NULL == pstParent)
					{
						iChange = 0;
						TDR_CLEAR_COUNT(pstStackTop, pstCurMeta, pszHostStart, pszHostEnd);
						continue;
					}
				}/*if (0 < pstStackTop->iCount)*/												
			}/*if (TDR_TYPE_STRUCT == pstCurMeta->iType)*/
		}/*if ((0 != iChange)*/

		iChange = 0;

		if (0 >= pstStackTop->iCount)
		{/*当前元数据数组已经处理完毕*/
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstCurMeta = pstStackTop->pstMeta;
				pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
				TDR_SET_ARRAY_REAL_COUNT((pstStackTop + 1)->iEntrySizeInfoOff, pstEntry, pstStackTop->pszHostBase);
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}	

		/*取出此entry的数组计数信息*/	
		TDR_GET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase, pstStackTop->iCutOffVersion); 
		if (0 > iCount)
		{	/*实际数目为负数或比数组最大长度要大，则无效*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_REFER_VALUE);
			break;
		}


		/* 设置最大读取成员数目：
		*  1)如果通过refer属性指定了成员的数目，则最大读取数目按此refer指定数目读取；
		*  2)如果没有通过refer属性指定了成员的数目:
		*		2.1）如果成员数组长度确定(icount > 0),则最大读取数目按成员数组长度设置；
		*		2.2) 如果成员数组长度确定(icount == 0),则最大读取数目设置为最大整数;
		*/
		if ((0 < pstEntry->iCount) && (pstEntry->iCount < iCount)) 
		{
			iCount = pstEntry->iCount;
		}
		if( (0 == iCount) && (pstEntry->iCount == 0))
		{
			iCount = TDR_MAX_INT;			
		}

		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
		pszHostEnd = pstStackTop->pszHostEnd;
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			pszHostStart = *(char **)pszHostStart;
			pszHostEnd = pszHostStart + pstEntry->iHRealSize;
		}	
		if (TDR_IO_NOINPUT & pstEntry->iIO)
		{
			TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, pstEntry->iCount);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		if (pstEntry->iVersion > a_iCutOffVersion)
		{
			TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, pstLib, pstEntry, pstEntry->iCount);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		if (TDR_TYPE_UNION != pstCurMeta->iType)
		{
			pstStackTop->pszMetaSizeInfoTarget = NULL;  /*从头开始搜索*/
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
				TDR_GET_UNION_ENTRY_TYPE_META_INFO(pstStackTop->pszHostBase, pstLib, pstEntry, a_iCutOffVersion, pstTypeMeta, idxSubEntry);
				if (NULL == pstTypeMeta)
				{
					TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
					continue;
				}		
			}else
			{
				pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				idxSubEntry = 0;				
			}/*if (TDR_TYPE_UNION == pstEntry->iType)*/

			/*定位节点*/
			if (TDR_TYPE_UNION == pstEntry->iType)
			{
				pstParent = (scew_element *)pstStackTop->pszNetBase;		
			}else
			{
				TDR_GET_NEXT_ELEMET(pstParent, pstStackTop->pszNetBase, pstStackTop->pszMetaSizeInfoTarget, pstTypeMeta->szName);
			}		
			if (NULL == pstParent)
			{
				TDR_SET_ARRAY_REAL_COUNT(0, pstEntry, pstStackTop->pszHostBase);
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				continue;
			}

			pstCurMeta = pstTypeMeta;
			iStackItemCount++;
			pstStackTop++;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = iCount;
			pstStackTop->idxEntry = idxSubEntry;
			pstStackTop->pszHostBase = pszHostStart;
			pstStackTop->pszNetBase = (char *)pstParent;
			pstStackTop->pszMetaSizeInfoTarget = NULL;	
			pstStackTop->iEntrySizeInfoOff = 0; /*实际读取的结构数*/					
			pstStackTop->iChange = 1;
			pstStackTop->pszHostEnd = pszHostEnd;
			continue;
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/
		
		iRet = tdr_input_simple_entry_i(pstLib, pstEntry, &iCount, (scew_element *)pstStackTop->pszNetBase,
			(scew_element **)&pstStackTop->pszMetaSizeInfoTarget, &pszHostStart, pszHostEnd);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}
		TDR_SET_ARRAY_REAL_COUNT(iCount, pstEntry, pstStackTop->pszHostBase);
		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/

	a_pstHost->iBuff = pszHostStart - a_pstHost->pszBuff;
	return iRet;
}

/*从XML数据树节点中读入简单类型成员的值到内存空间中
*@param[in] a_pstLib 元数据描述库指针
*@param[in] a_pstEntry 指定成员描述结构的指针
*@param[in,out] a_piCount 
*	- in 指定最大读取的元素个数
*	- out	输出实际读取的元素个数
*@param[in] a_pstRoot xml数据的根节点指针
*@param[in,out] a_ppstItem
*	- in	起始查找的儿子节点指针
*	- out	下一次查找的儿子节点指针
*@param[in, out] a_ppszHostStart 保存成员值的内存空间指针
*	- in	保存数据内存空间起始地址
*	- out	下一个成员可以使用的数据内存空间起始地址
*@param[in] a_pszHostEnd 可使用内存空间终止地址
*/
int tdr_input_simple_entry_i(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, INOUT int *a_piCount, 
						   IN scew_element *a_pstRoot, INOUT scew_element **a_ppstItem,
						   INOUT char **a_ppszHostStart, IN char *a_pszHostEnd)
{
	int iRet = TDR_SUCCESS;
	int iSize;
	scew_element *pstItem;
	const char *pszVal;
	char *pszHostStart;
	int iCount;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstEntry);
	assert(NULL != a_piCount);
	assert(NULL != a_pstRoot);
	assert(NULL != a_ppstItem);
	assert(NULL != a_ppszHostStart);
	assert(NULL != *a_ppszHostStart);
	assert(NULL != a_pszHostEnd);

	pstItem = *a_ppstItem;
	pszHostStart = *a_ppszHostStart;
	iCount = 0;
	switch(a_pstEntry->iType)
	{
	case TDR_TYPE_STRING:
	case TDR_TYPE_WSTRING:
	case TDR_TYPE_DATETIME:
		{
			for (iCount = 0; iCount < *a_piCount; iCount++)																
			{									
				TDR_GET_NEXT_ELEMET(pstItem, a_pstRoot, pstItem,  a_pstEntry->szName);							
				if (NULL == pstItem)										                        	
				{																						
					TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, a_pszHostEnd, a_pstLib, a_pstEntry, (*a_piCount-iCount));
					break;																				
				}	
				pszVal = (char*) scew_element_contents(pstItem);										
				if (NULL == pszVal)																		
				{																						
					pszVal = "";																		
				}
				iSize = a_pszHostEnd - pszHostStart;
				iRet = tdr_ioscanf_basedtype_i(a_pstLib, a_pstEntry, pszHostStart, &iSize, pszVal);				
				if (TDR_ERR_IS_ERROR(iRet))
				{																						
					break;																				
				}	
				pszHostStart += iSize;													
				*a_ppstItem = pstItem;								
			}/*for (iCount = 0; iCount < *a_piCount; iCount++)*/			
		}		
		break;
	default:
		{
			char *pszTok = NULL;

			TDR_GET_NEXT_ELEMET(pstItem, a_pstRoot, pstItem,  a_pstEntry->szName);							
			if (NULL == pstItem)										                        	
			{																						
				if (TDR_ENTRY_IS_COUNTER(a_pstEntry))												
				{																					
					LPTDRMETAENTRY pstUsedEntry =  TDR_PTR_TO_ENTRY(a_pstLib, a_pstEntry->stRefer.ptrEntry);	
					TDR_SET_INT(pszHostStart, a_pstEntry->iHUnitSize, pstUsedEntry->iCount);
					pszHostStart += a_pstEntry->iHUnitSize;
				}else																					
				{																					
					TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, a_pszHostEnd, a_pstLib, a_pstEntry, (*a_piCount));
				}						
				break;																				
			}/*if (NULL == pstItem)	*/	

			pszVal = (char*) scew_element_contents(pstItem);										
			if (NULL == pszVal)																		
			{																						
				pszVal = "";																		
			}
			pszTok = strtok((char *)pszVal, " \r\n\t");
			for (iCount = 0; iCount < *a_piCount; iCount++)
			{
				if (NULL == pszTok)										                        	
				{																						
					TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, a_pszHostEnd, a_pstLib, a_pstEntry, (*a_piCount-iCount));
					break;																				
				}
				iSize = a_pszHostEnd - pszHostStart;
				iRet = tdr_ioscanf_basedtype_i(a_pstLib, a_pstEntry, pszHostStart, &iSize, pszTok);				
				if (TDR_ERR_IS_ERROR(iRet))
				{																						
					break;																				
				}	
				pszHostStart += iSize;	
				pszTok = strtok(NULL, " \r\n\t");				
			}/*for (iCount = 0; iCount < *a_piCount; iCount++)*/			
			*a_ppstItem = pstItem;
		}
		break;
	}/*switch(a_pstEntry->iType)*/

	*a_ppszHostStart = pszHostStart;
	*a_piCount = iCount;

	return iRet;
}

