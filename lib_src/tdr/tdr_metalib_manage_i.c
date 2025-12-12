/**
*
* @file     tdr_metalib_manage.c  
* @brief    元数据库管理相关模块
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-04-02 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <assert.h>
#include <ctype.h>
#include "tdr/tdr_error.h"
#include "tdr/tdr_metalib_manage.h"
#include "tdr/tdr_XMLtags.h"
#include "tdr/tdr_metalib_init.h"
#include "tdr_define_i.h"
#include "tdr_os.h"
#include "tdr_auxtools.h"
#include "tdr_metalib_manage_i.h"
#include "tdr_ctypes_info_i.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif


#define TDR_SCANF_INT_FROM_STR(a_lVal, a_pstLib, a_pstEntry, a_pszValue) \
{																	\
	LPTDRMACRO pstMacro = NULL;											\
	if (TDR_INVALID_PTR != (a_pstEntry)->ptrMacrosGroup)					\
	{																	\
		pstMacro = tdr_get_bindmacro_by_name(a_pstLib, a_pstEntry, a_pszValue);\
		if (NULL == pstMacro)													\
		{																		\
			LPTDRMACROSGROUP pstGroup;											\
			char szValueAddPrefix[TDR_NAME_LEN*2 + 2];							\
			char *pDst = &szValueAddPrefix[0];									\
			char *pSrc;															\
			pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, a_pstEntry->ptrMacrosGroup); \
			pSrc = &pstGroup->szName[0];										\
			while ('\0' != *pSrc)												\
			{																	\
				*pDst = (char)toupper(*pSrc);									\
				pDst++;															\
				pSrc++;															\
			}																	\
			*pDst++ = '_';														\
			TDR_STRNCPY(pDst, a_pszValue, (sizeof(szValueAddPrefix)- (pDst - &szValueAddPrefix[0]))); \
			pstMacro = tdr_get_bindmacro_by_name(a_pstLib, a_pstEntry, &szValueAddPrefix[0]);		\
		}/*if (NULL == pstMacro)*/												\
	}																		\
	if (NULL != pstMacro)													\
	{																		\
		a_lVal = pstMacro->iValue;											\
	}else																	\
	{																		\
		int iID = 0, iIndex;														\
		tdr_get_macro_int_i(&iID, &iIndex, a_pstLib, a_pszValue);				\
		a_lVal = iID;															\
	}																		\
}

static char *tdr_get_bindmacro_name_i(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, IN int a_iValue);

/**
*获取复合元数据类型各成员中最大id属性值
*@param[in] a_pstMeta 元数据描述的指针
*
*@return 最大Id值
*
* @pre \e a_pstMeta 不能为 NULL
*/
int tdr_get_max_id_of_meta_entries(IN LPTDRMETA a_pstMeta);


/**
*获取复合数据类型(mata)元数据描述所在的元数据库的指针
*@param[in] a_pstMeta 元数据描述的指针
*
*@return 元数据库的指针
*
* @pre \e a_pstMeta 不能为 NULL
*/
LPTDRMETALIB tdr_get_metalib_of_meta(const LPTDRMETA a_pstMeta);


int tdr_get_meta_based_version(IN LPTDRMETA a_pstMeta)
{
	//assert(NULL != a_pstMeta);
	if (NULL == a_pstMeta)
	{
		return -1;
	}


	return a_pstMeta->iBaseVersion;
}

int tdr_get_meta_current_version(IN LPTDRMETA a_pstMeta)
{
	//assert(NULL != a_pstMeta);
	if (NULL == a_pstMeta)
	{
		return -1;
	}

	return a_pstMeta->iCurVersion;
}

int tdr_get_meta_type(IN LPTDRMETA a_pstMeta)
{
	//assert(NULL != a_pstMeta);
	if (NULL == a_pstMeta)
	{
		return TDR_TYPE_UNKOWN;
	}

	return a_pstMeta->iType;
}

LPTDRMETAENTRY tdr_get_entry_by_index(IN LPTDRMETA a_pstMeta, IN int a_idxEntry)
{
	assert(NULL != a_pstMeta);

	if ((0 > a_idxEntry) || (a_idxEntry >= a_pstMeta->iEntriesNum))
	{
		return NULL;
	}

	return a_pstMeta->stEntries + a_idxEntry;
}

LPTDRMETALIB tdr_get_metalib_of_meta(const LPTDRMETA a_pstMeta)
{
	assert(NULL != a_pstMeta);

	return TDR_META_TO_LIB(a_pstMeta);
}

int tdr_get_max_id_of_meta_entries(IN LPTDRMETA a_pstMeta)
{
	assert(NULL != a_pstMeta);

	return a_pstMeta->iMaxSubID;
}

int tdr_get_entry_num(IN LPTDRMETA a_pstMeta)
{
	//assert(NULL != a_pstMeta);
	if (NULL == a_pstMeta)
	{
		return -1;
	}

	return a_pstMeta->iEntriesNum;
}

const char *tdr_get_meta_name(IN LPTDRMETA a_pstMeta)
{
	//assert(NULL != a_pstMeta);
	if (NULL == a_pstMeta)
	{
		return "";
	}

	return a_pstMeta->szName;
}

const char *tdr_get_metalib_name(IN LPTDRMETALIB a_pstLib)
{
	//assert(NULL != a_pstLib);
	if (NULL == a_pstLib)
	{
		return "";
	}

	return a_pstLib->szName;
}

int tdr_get_metalib_version(IN LPTDRMETALIB a_pstLib)
{
	//assert(NULL != a_pstLib);
	if (NULL == a_pstLib)
	{
		return -1;
	}


	return (int)a_pstLib->lVersion;
}


int tdr_get_metalib_build_version(IN LPTDRMETALIB a_pstLib)
{
	//assert(NULL != a_pstLib);
	if (NULL == a_pstLib)
	{
		return -1;
	}

	return a_pstLib->nBuild;
}

int tdr_get_meta_size(IN LPTDRMETA a_pstMeta)
{
	int iSize;

	//assert(NULL != a_pstMeta);
	if (NULL == a_pstMeta)
	{
		return -1;
	}

	if (0 < a_pstMeta->iCustomHUnitSize)
	{
		iSize =  a_pstMeta->iCustomHUnitSize;
	}else
	{
		iSize = a_pstMeta->iHUnitSize;
	}

	return iSize;
}





int tdr_netoff_to_path_i(LPTDRMETA a_pstMeta, int a_iEntry, int a_iNetOff, char* a_pszBuff, int a_iBuff)
{
	int iRet = TDR_SUCCESS;
	int iCurOff = 0;
	LPTDRMETAENTRY pstEntry = NULL;
	LPTDRMETA pstSearchMeta = NULL;
	LPTDRMETALIB pstLib = NULL;
	int iTempLen = 0;
	char *pszBuff = NULL;
	int iLeftLen = 0;
	TDRBOOLEAN bGetFirstName = TDR_FALSE;
	int i = 0;
	char *pszName = NULL;
	int iSearchNOff;

	assert(NULL != a_pstMeta);
	assert(0 <= a_iNetOff);
	assert(NULL != a_pszBuff);
	assert(0 < a_iBuff);

	i = 0;
	pszBuff = a_pszBuff;
	iLeftLen = a_iBuff;
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstSearchMeta = a_pstMeta;
	iSearchNOff = a_iNetOff;
	while (iCurOff < iSearchNOff )
	{
		if ( i >= pstSearchMeta->iEntriesNum )  
		{   /*已经搜索完所有entry，但没有entry的偏移和指定的偏移值匹配*/
			iRet = TDR_ERRIMPLE_INVALID_OFFSET;
			break;
		}

		pstEntry = pstSearchMeta->stEntries + i;        
		if( ((iCurOff + pstEntry->iNOff) > iSearchNOff) ||
			((iCurOff + pstEntry->iNOff + pstEntry->iNUnitSize) <= iSearchNOff) )
		{
			i++;
			continue;
		}

		/*发现entry的范围包含a_iOff*/
		if( i == a_iEntry )
		{
			pszName = TDR_TAG_THIS;
		}
		else
		{
			pszName = pstEntry->szName;
		} 
		a_iEntry = -1;  /*只有path第一层名字会出现this*/


		/*记录entry的名字*/
		if( bGetFirstName == TDR_FALSE )
		{
			iTempLen = tdr_snprintf(pszBuff, iLeftLen, "%s", pszName);
			TDR_CHECK_BUFF(pszBuff, iLeftLen, iTempLen, iRet);
			bGetFirstName = TDR_TRUE;
		}else
		{
			iTempLen = tdr_snprintf(pszBuff, iLeftLen, ".%s", pszName);
			TDR_CHECK_BUFF(pszBuff, iLeftLen, iTempLen, iRet);
		}
		if (TDR_ERR_IS_ERROR(iRet))
		{/*缓冲区溢出*/
			break;
		}        


		iCurOff += pstEntry->iNOff;
		if (( TDR_INVALID_PTR == pstEntry->ptrMeta ) ||
			TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			pstSearchMeta = NULL;
			break;
		}else
		{
			pstSearchMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			i =	0;            
		}
	}

	/*找不到指定偏移值的entry，则直接返回*/
	if( iCurOff != iSearchNOff )
	{
		if (!TDR_ERR_IS_ERROR(iRet))
		{
			iRet = TDR_ERRIMPLE_INVALID_OFFSET;
		}
		return iRet;        
	}


	/*如果找到指定偏移值的entry，且此entry为复合meta则
	*继续向下递归获取entry名字，直至到简单类型的entry*/
	while( NULL != pstSearchMeta )
	{
		pstEntry = pstSearchMeta->stEntries;  /*取第一个元素*/ 
		if ( 0 == a_iEntry )
		{
			pszName = TDR_TAG_THIS;
		}
		else
		{
			pszName = pstEntry->szName;
		} 
		a_iEntry = -1;  /*只有path第一层名字会出现this*/


		/*记录entry的名字*/
		if( bGetFirstName == TDR_FALSE )
		{
			iTempLen = tdr_snprintf(pszBuff, iLeftLen, "%s", pszName);
			TDR_CHECK_BUFF(pszBuff, iLeftLen, iTempLen, iRet);
			bGetFirstName = TDR_TRUE;
		}else
		{
			iTempLen = tdr_snprintf(pszBuff, iLeftLen, ".%s", pszName);
			TDR_CHECK_BUFF(pszBuff, iLeftLen, iTempLen, iRet);
		}
		if (TDR_ERR_IS_ERROR(iRet))
		{/*缓冲区溢出*/
			break;
		}  


		if (( TDR_INVALID_PTR == pstEntry->ptrMeta ) ||
			TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			pstSearchMeta = NULL;
			break;
		}else
		{
			pstSearchMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);        
		}       
	}/*while( NULL != pstSearchMeta )*/


	return iRet;
}   


int tdr_hostoff_to_path_i(LPTDRMETA a_pstMeta, int a_iEntry, int a_iHostOff, char* a_pszBuff, int a_iBuff)
{
	int iRet = TDR_SUCCESS;
	int iCurOff = 0;
	LPTDRMETAENTRY pstEntry = NULL;
	LPTDRMETA pstSearchMeta = NULL;
	LPTDRMETALIB pstLib = NULL;
	int iTempLen = 0;
	char *pszBuff = NULL;
	int iLeftLen = 0;
	TDRBOOLEAN bGetFirstName = TDR_FALSE;
	int i = 0;
	char *pszName = NULL;
	int iSearchHOff;

	assert(NULL != a_pstMeta);
	assert(0 <= a_iHostOff);
	assert(NULL != a_pszBuff);
	assert(0 < a_iBuff);

	i = 0;
	pszBuff = a_pszBuff;
	iLeftLen = a_iBuff;
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstSearchMeta = a_pstMeta;
	iSearchHOff = a_iHostOff;
	while (iCurOff < iSearchHOff )
	{
		if ( i >= pstSearchMeta->iEntriesNum )  
		{   /*已经搜索完所有entry，但没有entry的偏移和指定的偏移值匹配*/
			iRet = TDR_ERRIMPLE_INVALID_OFFSET;
			break;
		}

		pstEntry = pstSearchMeta->stEntries + i;        
		if( ((iCurOff + pstEntry->iHOff) > iSearchHOff) ||
			((iCurOff + pstEntry->iHOff + pstEntry->iHUnitSize) <= iSearchHOff) )
		{
			i++;
			continue;
		}

		/*发现entry的范围包含a_iOff*/
		if( i == a_iEntry )
		{
			pszName = TDR_TAG_THIS;
		}
		else
		{
			pszName = pstEntry->szName;
		} 
		a_iEntry = -1;  /*只有path第一层名字会出现this*/


		/*记录entry的名字*/
		if( bGetFirstName == TDR_FALSE )
		{
			iTempLen = tdr_snprintf(pszBuff, iLeftLen, "%s", pszName);
			TDR_CHECK_BUFF(pszBuff, iLeftLen, iTempLen, iRet);
			bGetFirstName = TDR_TRUE;
		}else
		{
			iTempLen = tdr_snprintf(pszBuff, iLeftLen, ".%s", pszName);
			TDR_CHECK_BUFF(pszBuff, iLeftLen, iTempLen, iRet);
		}
		if (TDR_ERR_IS_ERROR(iRet))
		{/*缓冲区溢出*/
			break;
		}        


		iCurOff += pstEntry->iHOff;
		if (( TDR_INVALID_PTR == pstEntry->ptrMeta ) ||
			TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			pstSearchMeta = NULL;
			break;
		}else
		{
			pstSearchMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			i =	0;            
		}
	}

	/*找不到指定偏移值的entry，则直接返回*/
	if( iCurOff != iSearchHOff )
	{
		if (!TDR_ERR_IS_ERROR(iRet))
		{
			iRet = TDR_ERRIMPLE_INVALID_OFFSET;
		}
		return iRet;        
	}


	/*如果找到指定偏移值的entry，且此entry为复合meta则
	*继续向下递归获取entry名字，直至到简单类型的entry*/	
	while( NULL != pstSearchMeta )
	{
		pstEntry = pstSearchMeta->stEntries;  /*取第一个元素*/ 
		if ( 0 == a_iEntry )
		{
			pszName = TDR_TAG_THIS;
		}
		else
		{
			pszName = pstEntry->szName;
		} 
		a_iEntry = -1;  /*只有path第一层名字会出现this*/


		/*记录entry的名字*/
		if( bGetFirstName == TDR_FALSE )
		{
			iTempLen = tdr_snprintf(pszBuff, iLeftLen, "%s", pszName);
			TDR_CHECK_BUFF(pszBuff, iLeftLen, iTempLen, iRet);
			bGetFirstName = TDR_TRUE;
		}else
		{
			iTempLen = tdr_snprintf(pszBuff, iLeftLen, ".%s", pszName);
			TDR_CHECK_BUFF(pszBuff, iLeftLen, iTempLen, iRet);
		}
		if (TDR_ERR_IS_ERROR(iRet))
		{/*缓冲区溢出*/
			break;
		} 		

		if (( TDR_INVALID_PTR == pstEntry->ptrMeta ) ||
			TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			pstSearchMeta = NULL;
			break;
		}else
		{
			pstSearchMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);        
		}       
	}/*while( NULL != pstSearchMeta )*/


	return iRet;
}   



char *tdr_get_first_node_name_from_path_i(char *pszName, int iNameSize, const char *pszPath)
{
	char *pszDot = NULL;
	int iLen = 0;

	assert(NULL != pszName);
	assert(0 < iNameSize);
	assert(NULL != pszPath);

	pszName[0] = '\0';
	pszDot = strchr(pszPath, TDR_TAG_DOT);        
	if (NULL != pszDot)
	{
		iLen = pszDot - pszPath;
	}else
	{
		iLen = strlen(pszPath);
	}        
	if ( iLen >= iNameSize )
	{/*名字太长*/
		return NULL;
	}
	TDR_STRNCPY(pszName, pszPath, iLen + 1);

	if (NULL != pszDot)
	{
		pszDot++;
	}

	return pszDot;
}

int tdr_sizeinfo_to_off_i(LPTDRSIZEINFO a_pstRedirector, LPTDRMETA a_pstMeta, int a_iEntry, const char* a_pszName)
{
	TDRMETALIB* pstLib;
	TDRMETAENTRY* pstEntry = NULL;
	LPTDRMETA pstSearchMeta;
	int iRet = TDR_SUCCESS;

	char szBuff[TDR_NAME_LEN];
	const char* pszPtr;
	int idxThisEntry;
	int idx;
	int iNOff;
	int iHOff;
	int iUnit;
	int i;
	int iIdxType;

	assert( NULL != a_pstMeta);
	assert(NULL != a_pstRedirector);
	assert(NULL != a_pszName);

	   
	

	/*检查是不是通过简单数据类型来打包*/
	iIdxType = tdr_typename_to_idx(a_pszName);
	if (TDR_INVALID_INDEX != iIdxType)
	{
		/*基本数据类型*/
		LPTDRCTYPEINFO pstTypeInfo = tdr_idx_to_typeinfo(iIdxType);

		assert(NULL != pstTypeInfo);
		if ((TDR_TYPE_CHAR <= pstTypeInfo->iType) && (TDR_TYPE_UINT >= pstTypeInfo->iType))
		{
			a_pstRedirector->idxSizeType = iIdxType;
			a_pstRedirector->iUnitSize = pstTypeInfo->iSize;
		}else
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
		}/*if ((TDR_TYPE_CHAR <= pstTypeInfo->iType) && (TDR_TYPE_ULONG >= pstTypeInfo->iType))*/

		return iRet;
	}


	/*sizeinfo/sizetypy为path的情况, 如果是meta的sizeinfo属性,先跳过'this'*/
	pstLib = TDR_META_TO_LIB(a_pstMeta); 
	pszPtr = a_pszName;
	if (TDR_INVALID_INDEX == a_iEntry)
	{
		pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
		if( 0 != tdr_stricmp(szBuff, TDR_TAG_THIS) )
		{
			pszPtr = a_pszName;
		}
	}

	iNOff = 0;
	iHOff = 0;
	iUnit = 0; 
	idxThisEntry = a_iEntry;
	pstSearchMeta = a_pstMeta;
	do
	{
		if( NULL == pstSearchMeta )
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
			break;
		}

		pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
		if ('\0' == szBuff[0])
		{/*空串，不合法*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
			break;
		}
		tdr_trim_str(szBuff); 

		if( 0==tdr_stricmp(szBuff, TDR_TAG_THIS) )
		{
			idx	= idxThisEntry;
		}else
		{
			idx	= tdr_get_entry_by_name_i(pstSearchMeta->stEntries, pstSearchMeta->iEntriesNum, szBuff);
		}
		if( TDR_INVALID_INDEX == idx )
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
			break;
		}

		for(i=0; i < idx; i++)
		{
			pstEntry =	pstSearchMeta->stEntries  + i;
			if (!TDR_ENTRY_IS_FIXSIZE(pstEntry))
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VARIABLE_BEFOR_SIZEINFO);
				break;
			}
		}


		idxThisEntry = -1;
		pstEntry = pstSearchMeta->stEntries + idx;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
			break;
		}

		iNOff += pstEntry->iNOff;
		iHOff += pstEntry->iHOff;
		iUnit =	pstEntry->iNUnitSize;

		if(TDR_INVALID_PTR != pstEntry->ptrMeta )
		{
			pstSearchMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
		}
		else
		{
			pstSearchMeta	=	NULL;
		}
	}while( NULL != pszPtr );

	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	if ((NULL == pstEntry) || ( 1 != pstEntry->iCount) ||(pstEntry->stRefer.iUnitSize > 0 ))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VARIABLE_BEFOR_SIZEINFO);
	}  

	if( pstEntry->iType <= TDR_TYPE_COMPOSITE || pstEntry->iType > TDR_TYPE_UINT )
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VARIABLE_BEFOR_SIZEINFO);
	}


	a_pstRedirector->iHOff	=	iHOff;
	a_pstRedirector->iNOff = iNOff;
	a_pstRedirector->iUnitSize	=	iUnit;

	return TDR_SUCCESS;
}







LPTDRMETA tdr_get_meta_by_name_i(TDRMETALIB* pstLib, const char* pszName)
{
	int i;
	LPTDRMAPENTRY pstMap;
	LPTDRMETA pstMeta;

	assert(NULL != pstLib);
	assert(NULL != pszName);

	pstMap = (LPTDRMAPENTRY)((pstLib)->data + (pstLib)->ptrMap);

	for (i=0; i < pstLib->iCurMetaNum; i++)
	{
		pstMeta	= (LPTDRMETA) (pstLib->data + pstMap[i].iPtr);

		if ( 0 == strcmp(pstMeta->szName, pszName))
		{
			return pstMeta;
		}
	}

	return NULL;
}


int tdr_get_macro_int_i(int* a_piID, int* a_piIdx, TDRMETALIB* a_pstLib, const  char *a_pszValue)
{
	assert( NULL != a_pstLib);
	assert(NULL != a_pszValue);
	assert(NULL != a_piID);
	assert(NULL != a_piIdx );    

	if( ( a_pszValue[0] >= '0' && a_pszValue[0] <= '9' ) ||
		('+' == a_pszValue[0]) || ('-' == a_pszValue[0]) )
	{
		*a_piIdx = -1;
		*a_piID = (int)strtol(a_pszValue, NULL, 0);;
	}
	else
	{
		TDRMACRO* pstMacro;
		int iIdx = TDR_INVALID_INDEX;

		iIdx = tdr_get_macro_index_by_name_i(a_pstLib, a_pszValue);
		if (TDR_INVALID_INDEX == iIdx)
		{

			return TDR_ERRIMPLE_UNDEFINED_MACRO_NAME;
		}else        
		{
			pstMacro = TDR_GET_MACRO_TABLE(a_pstLib);
			*a_piIdx = iIdx;
			*a_piID = pstMacro[iIdx].iValue;
		}
	}

	return TDR_SUCCESS;
}

int tdr_get_macro_index_by_name_i(TDRMETALIB* pstLib, const char* pszName)
{
	LPTDRMACRO pstMacro;
	int i;


	assert(NULL != pstLib);
	assert(NULL != pszName);

	pstMacro = TDR_GET_MACRO_TABLE(pstLib);


	for (i=0; i< pstLib->iCurMacroNum; i++)
	{
		if (0 == strcmp(pstMacro[i].szMacro, pszName))
		{
			return i;
		}
	}

	return TDR_INVALID_INDEX;
}


int tdr_get_entry_by_name_i(TDRMETAENTRY pstEntry[], int iMax, const char* pszName)
{
    int i;

    assert(NULL != pstEntry );

    for (i=0; i<iMax; i++)
    {
        if ( 0 == strcmp(pstEntry[i].szName, pszName) )
        {
            break;
        }
    }

    return (i < iMax) ? i : TDR_INVALID_INDEX;
}

LPTDRMETA  tdr_get_meta_by_name(IN LPTDRMETALIB a_pstLib, IN const char* a_pszName)
{
	LPTDRNAMEENTRY pstName = NULL;
	int iRet;
	int iMin;
	int iMax;
	int i;
	LPTDRMETA pstMeta = NULL;

	//assert(NULL != a_pstLib);
	//assert(NULL != a_pszName);
	if ((NULL == a_pstLib)||(NULL == a_pszName))
	{
		return NULL;
	}

	pstName	= TDR_GET_META_NAME_MAP_TABLE(a_pstLib);
	iMin = 0;
	iMax = a_pstLib->iCurMetaNum -1;

	while(iMin <= iMax)
	{
		i = (iMin+iMax)>>1;

		iRet = strcmp(a_pszName, pstName[i].szName);
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
			pstMeta =  TDR_IDX_TO_META(a_pstLib, pstName[i].iIdx);
			break;
		}
	}	

	return pstMeta;
}



LPTDRMETA tdr_get_meta_by_id(IN LPTDRMETALIB a_pstLib, IN int a_iID)
{
	LPTDRIDENTRY pstIDMapping = NULL;
	int iRet;
	int iMin;
	int iMax;
	int i;
	LPTDRMETA pstMeta = NULL;

	//assert(NULL != a_pstLib);
	if (NULL == a_pstLib)
	{
		return NULL;
	}

	if (TDR_INVALID_ID == a_iID)
	{
		return NULL;
	}

	pstIDMapping = TDR_GET_META_ID_MAP_TABLE(a_pstLib);
	iMin = 0;
	iMax = a_pstLib->iCurMetaNum -1;

	while(iMin <= iMax)
	{
		i = (iMin+iMax)>>1;


		iRet = a_iID - pstIDMapping[i].iID;
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
			pstMeta =  TDR_IDX_TO_META(a_pstLib, pstIDMapping[i].iIdx);
			break;
		}
	}	

	return pstMeta;
}

int tdr_get_entry_by_id(OUT int* a_piIdx, IN LPTDRMETA a_pstMeta, IN int a_iId)
{
	int ids = TDR_INVALID_INDEX;
	

	//assert(NULL != a_piIdx);
	//assert(NULL != a_pstMeta);
	if ((NULL == a_pstMeta)||(NULL == a_piIdx))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	if (TDR_TYPE_UNION != a_pstMeta->iType)
	{
		int i;
		for (i = 0; i < a_pstMeta->iEntriesNum; i++)
		{
			if (TDR_ENTRY_DO_HAVE_ID(&a_pstMeta->stEntries[i]) && (a_iId == a_pstMeta->stEntries[i].iID))
			{
				*a_piIdx = i;
				return TDR_SUCCESS;
			}
		}

		*a_piIdx = TDR_INVALID_INDEX;
		return TDR_ERR_ERROR;
	}

	/*union 结构*/
	TDR_GET_ENTRY(ids, a_pstMeta->stEntries, a_pstMeta->iEntriesNum, a_iId);


	/*如果没有找到，则找缺省的成员*/
	*a_piIdx = ids;
	if (0 <= ids)
	{
		return TDR_SUCCESS;
	}

	return TDR_ERR_ERROR;
}

int tdr_get_entry_by_name(OUT int* a_piIdx, IN LPTDRMETA a_pstMeta, IN const char* a_pszName)
{
	int i;

	/*assert(NULL != a_piIdx);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pszName);*/
	if ((NULL == a_piIdx)||(NULL == a_pstMeta)||(NULL == a_pszName))
	{
		return	TDR_ERRIMPLE_INVALID_PARAM;
	}

	for (i = 0; i < a_pstMeta->iEntriesNum; i++)
	{
		if (0 == strcmp(a_pszName, a_pstMeta->stEntries[i].szName))
		{
			break;
		}
	}

	if (i >= a_pstMeta->iEntriesNum)
	{
		*a_piIdx = TDR_INVALID_INDEX;
		return TDR_ERR_ERROR;
	}

	*a_piIdx = i;

	return TDR_SUCCESS;
}

LPTDRMETAENTRY tdr_get_entryptr_by_name(IN LPTDRMETA a_pstMeta, IN const char* a_pszName)
{
	int i;

	//assert(NULL != a_pstMeta);
	//assert(NULL != a_pszName);
	if ((NULL == a_pstMeta)||(NULL == a_pszName))
	{
		return NULL;
	}

	for (i = 0; i < a_pstMeta->iEntriesNum; i++)
	{
		if (0 == strcmp(a_pszName, a_pstMeta->stEntries[i].szName))
		{
			break;
		}
	}

	if (i >= a_pstMeta->iEntriesNum)
	{
		return NULL;
	}


	return &a_pstMeta->stEntries[i];
}



int tdr_get_entry_id(IN LPTDRMETAENTRY a_pstEntry)
{
	int iID;

	assert(NULL != a_pstEntry);


	if (TDR_ENTRY_DO_HAVE_ID(a_pstEntry))
	{
		iID =  a_pstEntry->iID;
	}else
	{
		iID = TDR_INVALID_ID;
	}

	return iID;
}



LPTDRMETA tdr_get_entry_type_meta(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry)
{
	//assert(NULL != a_pstEntry);
	//assert(NULL != a_pstLib);
	if ((NULL == a_pstLib)||(NULL == a_pstEntry))
	{
		return NULL;
	}

	if (TDR_INVALID_PTR == a_pstEntry->ptrMeta)
	{
		return NULL;
	}

	return (LPTDRMETA)(TDR_PTR_TO_META(a_pstLib, a_pstEntry->ptrMeta));
}

int tdr_parse_meta_sortkey_info_i(IN LPTDRSORTKEYINFO a_pstSortKey, LPTDRMETA a_pstMeta, const char *a_pszSortKeyPath)
{
	LPTDRMETA pstSearchMeta;
	int idxEntry;
	int iHOff;
	const char *pszPtr;
	char szBuff[TDR_NAME_LEN] = {0};
	LPTDRMETAENTRY pstSearchEntry;
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib;

	assert(NULL != a_pstSortKey);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pszSortKeyPath);

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstSearchMeta = a_pstMeta;      /*从当前entry开始查找*/           
	idxEntry = TDR_INVALID_INDEX;
	iHOff = 0; 
	pszPtr = a_pszSortKeyPath;
	while( NULL != pszPtr )
	{
		if (NULL == pstSearchMeta)
		{
			break;
		}

		pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);     
		tdr_trim_str(szBuff);

		/*查找名字相同的entry*/
		pstSearchEntry = NULL;   
		idxEntry = tdr_get_entry_by_name_i(pstSearchMeta->stEntries, pstSearchMeta->iEntriesNum, szBuff);      
		if( TDR_INVALID_INDEX == idxEntry )
		{
			break;
		}

		/*找到名字相同的entry*/   
		pstSearchEntry = pstSearchMeta->stEntries + idxEntry;
		a_pstSortKey->ptrSortKeyMeta = pstSearchMeta->ptrMeta;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstSearchEntry))
		{
			idxEntry = TDR_INVALID_INDEX;
			break;
		}
		if (TDR_ENTRY_IS_REFER_TYPE(pstSearchEntry))
		{
			idxEntry = TDR_INVALID_INDEX;
			break;
		}
		iHOff +=	pstSearchEntry->iHOff; /*累加偏移*/

		if (TDR_INVALID_PTR != pstSearchEntry->ptrMeta)
		{
			pstSearchMeta = TDR_PTR_TO_META(pstLib, pstSearchEntry->ptrMeta);
		}else
		{
			pstSearchMeta = NULL;
		}
	}


	if ((NULL != pszPtr) || (TDR_INVALID_INDEX == idxEntry ))
	{
		iRet = TDR_ERRIMPLE_ENTRY_IVALID_SORTKEY_VALUE;
	}else 
	{
		a_pstSortKey->iSortKeyOff = iHOff;
		a_pstSortKey->idxSortEntry = idxEntry;
	}

	return iRet;
}

/**获取单个entry成员的本地存储空间大小
*@param[in] a_pstEntry entry元素描述结构的指针
*
*@note 通过调用tdr_get_entry_by_index可以获取entry元素描述结构的指针
*
*@return entry元素单个变量的本地存储空间大小
*
*@pre \e a_pstEntry 不能为NULL
*@see tdr_get_entry_by_index
*/
int tdr_get_entry_unitsize(IN LPTDRMETAENTRY a_pstEntry)
{
	int iSize;

	//assert(NULL != a_pstEntry);
	if (NULL == a_pstEntry)
	{
		return -1;
	}

	if (0 < a_pstEntry->iCustomHUnitSize)
	{
		iSize = a_pstEntry->iCustomHUnitSize;
	}else
	{
		iSize = a_pstEntry->iHUnitSize;
	}

	return iSize;
}

int tdr_get_macro_value(OUT int *a_piID, IN TDRMETALIB* a_pstLib, IN const  char *a_pszName)
{
	LPTDRMACRO pstMacro;
	int i;

	//assert(NULL != a_piID);
	//assert(NULL != a_pstLib);
	//assert(NULL != a_pszName);
	if ((NULL == a_piID)||(NULL == a_pstLib)||(NULL == a_pszName))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	pstMacro = TDR_GET_MACRO_TABLE(a_pstLib);
	for (i=0; i< a_pstLib->iCurMacroNum; i++)
	{
		if (0 == strcmp(pstMacro[i].szMacro, a_pszName))
		{
			*a_piID = pstMacro[i].iValue;
			return TDR_SUCCESS;
		}
	}

	return TDR_ERRIMPLE_UNDEFINED_MACRO_NAME;
}

char *tdr_entry_off_to_path(IN LPTDRMETA a_pstMeta, IN int a_iOff)
{
	int iRet = TDR_SUCCESS;
	static char szPath[(TDR_NAME_LEN+1)*TDR_STACK_SIZE] = {0};
	char *pszPath;

	/*assert(NULL != a_pstMeta);
	assert(0 <= a_iOff);*/
	if ((NULL == a_pstMeta)||(0 > a_iOff))
	{
		return "Unknow";
	}

	iRet = tdr_hostoff_to_path_i(a_pstMeta, TDR_INVALID_INDEX, a_iOff, &szPath[0], sizeof(szPath));

	if (TDR_ERR_IS_ERROR(iRet))
	{
		pszPath= &szPath[0];	
	}else
	{
		pszPath = "UnKnow";
	}
	return pszPath;
}

int tdr_get_entry_type(IN LPTDRMETAENTRY a_pstEntry)
{
	//assert(NULL != a_pstEntry);
	if (NULL == a_pstEntry)
	{
		return TDR_TYPE_UNKOWN;
	}

	return a_pstEntry->iType;
}

int tdr_get_entry_count(IN LPTDRMETAENTRY a_pstEntry)
{
	//assert(NULL != a_pstEntry);
	if (NULL == a_pstEntry)
	{
		return 1;
	}

	return a_pstEntry->iCount;
}
int tdr_name_to_off_i(IN LPTDRMETA a_pstMeta, OUT LPTDRSelector a_pstSelector, IN int a_iEntry, IN const char* a_pszName)
{
	TDRMETALIB* pstLib;
	TDRMETAENTRY* pstEntry = NULL; 
	LPTDRMETA pstSearchMeta;
	char szBuff[TDR_NAME_LEN];    
	int iRet = TDR_SUCCESS;
	const char* pszPtr = NULL;    

	int iHOff;
	int iIdx;

	assert( NULL != a_pstMeta);
	assert(NULL != a_pstSelector);
	assert(NULL != a_pszName);


	pstLib = TDR_META_TO_LIB(a_pstMeta); 


	/*meta属性跳过this*/
	pszPtr = a_pszName;
	if (TDR_INVALID_INDEX == a_iEntry)
	{
		pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
		if( 0 != tdr_stricmp(szBuff, TDR_TAG_THIS))
		{
			pszPtr = a_pszName;
		}
	}   


	iHOff = 0;
	pstSearchMeta = a_pstMeta;
	do
	{
		if( NULL == pstSearchMeta )
		{
			break;
		}


		pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
		if ('\0' == szBuff[0])
		{/*空串，不合法*/
			break;
		}       
		tdr_trim_str(szBuff);


		/*查找名字相同的entry*/
		pstEntry = NULL;
		if( 0 == tdr_stricmp(szBuff, TDR_TAG_THIS))
		{
			iIdx = a_iEntry;
		}else
		{
			iIdx = tdr_get_entry_by_name_i(pstSearchMeta->stEntries, pstSearchMeta->iEntriesNum, szBuff);
		}        
		if( TDR_INVALID_INDEX == iIdx )
		{
			break;
		}

		a_iEntry	= -1;  /*this只能出现在path的最前面*/                
		pstEntry = pstSearchMeta->stEntries + iIdx;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			iRet = TDR_ERRIMPLE_INVALID_PATH_VALUE;
			break;
		}

		iHOff += pstEntry->iHOff;


		if( TDR_INVALID_PTR != pstEntry->ptrMeta )
		{
			pstSearchMeta	=	TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
		}
		else
		{
			pstSearchMeta	=	NULL;	
		}

	}while( NULL != pszPtr );

	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	if ((NULL != pszPtr)  || (NULL == pstEntry))
	{
		iRet = TDR_ERRIMPLE_INVALID_PATH_VALUE;
	}else if (NULL != pstEntry)
	{
		a_pstSelector->iHOff = iHOff;
		a_pstSelector->iUnitSize = pstEntry->iNUnitSize;
		a_pstSelector->ptrEntry = TDR_ENTRY_TO_PTR(pstLib, pstEntry);
	}


	return iRet;    
}

LPTDRMETAENTRY tdr_get_entry_by_path(IN LPTDRMETA a_pstMeta, IN const char* a_pszEntryPath)
{
	TDRMETALIB* pstLib;
	TDRMETAENTRY* pstEntry = NULL; 
	LPTDRMETA pstSearchMeta;
	char szBuff[TDR_NAME_LEN];    

	const char* pszPtr = NULL;    


	//assert( NULL != a_pstMeta);
	//assert(NULL != a_pszEntryPath);
	if ((NULL == a_pstMeta)||(NULL == a_pszEntryPath))
	{
		return NULL;
	}


	pstLib = TDR_META_TO_LIB(a_pstMeta); 


	/*meta属性跳过this*/
	pszPtr = a_pszEntryPath;
	pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
	if( 0 != tdr_stricmp(szBuff, TDR_TAG_THIS))
	{
		pszPtr = a_pszEntryPath;
	}else if (NULL == pszPtr)
	{
		return NULL;
	}
	


	pstSearchMeta = a_pstMeta;
	do
	{
		pstEntry = NULL;
		if( NULL == pstSearchMeta )
		{
			break;
		}


		pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);		     
		tdr_trim_str(szBuff);
		if ('\0' == szBuff[0])
		{/*空串，不合法*/
			break;
		} 


		/*查找名字相同的entry*/
		pstEntry = tdr_get_entryptr_by_name(pstSearchMeta, szBuff);
		if(NULL  == pstEntry )
		{
			break;
		}

		if( TDR_INVALID_PTR != pstEntry->ptrMeta )
		{
			pstSearchMeta	=	TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
		}
		else
		{
			pstSearchMeta	=	NULL;	
		}
	}while( NULL != pszPtr );

	


	return pstEntry;    	
}


int tdr_entry_path_to_off(IN LPTDRMETA a_pstMeta, INOUT LPTDRMETAENTRY *a_ppstEntry, OUT int *a_piHOff, IN const char *a_pszPath)
{
	int iRet = TDR_SUCCESS;
	TDRSelector stSelector;
	LPTDRMETALIB pstLib;

	/*assert(NULL != a_pstMeta);
	assert(NULL != a_ppstEntry);
	assert(NULL != a_piHOff);
	assert(NULL != a_pszPath);*/
	if ((NULL == a_pstMeta)||(NULL == a_ppstEntry)||(NULL == a_piHOff)||(NULL == a_pszPath))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	iRet = tdr_name_to_off_i(a_pstMeta, &stSelector, TDR_INVALID_INDEX, a_pszPath);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	*a_ppstEntry = TDR_PTR_TO_ENTRY(pstLib, stSelector.ptrEntry);
	*a_piHOff = stSelector.iHOff;

	return iRet;
}


/*根据成员的值获取此值绑定的宏定义，只有当成员定义了macrosgroup属性时才有效*/
LPTDRMACRO tdr_get_bindmacro_by_value(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, IN int a_iValue)
{
	LPTDRMACRO pstMacro = NULL;
	LPTDRMACRO pstMacroTable = NULL;
	LPTDRMACROSGROUP pstGroup;
	TDRIDX *pValueIdxTab;
	int iMax,iMin,iMid;
	int iDiff;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstEntry);


	if (TDR_INVALID_PTR == a_pstEntry->ptrMacrosGroup)
	{
		return NULL;
	}

	pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);
	pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, a_pstEntry->ptrMacrosGroup);
	pValueIdxTab= TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(pstGroup);
	iMin = 0;
	iMax = pstGroup->iCurMacroCount -1;
	while (iMin <= iMax)
	{
		iMid = (iMin + iMax) / 2;
		pstMacro = pstMacroTable + pValueIdxTab[iMid];
		iDiff = pstMacro->iValue - a_iValue;
		if (iDiff < 0)
		{
			iMin = iMid + 1;
		}else if (iDiff > 0)
		{
			iMax = iMid - 1;
		}else
		{
			break;
		}
	}/*while (iMin <= iMax)*/

	if (iMin > iMax)
	{
		return NULL;
	}

	return pstMacro;
}

/*根据指定名字获取此值绑定的宏定义，只有当成员定义了macrosgroup属性时才有效*/
LPTDRMACRO tdr_get_bindmacro_by_name(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, IN const char *a_pszName)
{
	LPTDRMACRO pstMacro = NULL;
	LPTDRMACRO pstMacroTable = NULL;
	LPTDRMACROSGROUP pstGroup;
	TDRIDX *pNameIdxTab;
	int iMax,iMin,iMid;
	int iDiff;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstEntry);
	assert(NULL != a_pszName);


	if (TDR_INVALID_PTR == a_pstEntry->ptrMacrosGroup)
	{
		return NULL;
	}

	pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);
	pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, a_pstEntry->ptrMacrosGroup);
	pNameIdxTab= TDR_GET_MACROSGROUP_NAMEIDXMAP_TAB(pstGroup);
	iMin = 0;
	iMax = pstGroup->iCurMacroCount -1;
	while (iMin <= iMax)
	{
		iMid = (iMin + iMax) / 2;
		pstMacro = pstMacroTable + pNameIdxTab[iMid];
		iDiff = strcmp(pstMacro->szMacro, a_pszName);
		if (iDiff < 0)
		{
			iMin = iMid + 1;
		}else if (iDiff > 0)
		{
			iMax = iMid - 1;
		}else
		{
			break;
		}
	}/*while (iMin <= iMax)*/

	if (iMin > iMax)
	{
		return NULL;
	}

	return pstMacro;
}



/**输出基本数据类型成员的值
*@param[in] a_pstIOStream 输出信息的流句柄
*@param[in] a_pstLib 元数据描述库指针
*@param[in] a_pstEntry 成员的描述结构指针
*@param[in,out] a_ppszHostStart 此成员值存储空间的起始地址
*@param[in] a_pszHostEnd 此成员值存储空间的结束地址
*@return 成功返回0，否则返回非零值
*/
int tdr_ioprintf_basedtype_i(IN LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, 
							 INOUT char **a_ppszHostStart, IN const char *a_pszHostEnd)
{
	int iRet = TDR_SUCCESS;
	int iSize;																			
	int iLen;
	char *pszHostStart;
	char *pszVal = NULL;

	assert(NULL != a_pstIOStream);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstEntry);
	assert(NULL != a_ppszHostStart);
	assert(NULL != *a_ppszHostStart);
	assert(NULL != a_pszHostEnd);
	assert(TDR_TYPE_COMPOSITE < a_pstEntry->iType);

	pszHostStart = *a_ppszHostStart;
	
	switch(a_pstEntry->iType)
	{
	case TDR_TYPE_CHAR:
		{
			pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)(char)pszHostStart[0]);
			if(NULL != pszVal)
			{
				iRet = tdr_iostream_write(a_pstIOStream, "%s ", pszVal);
			}else 
			{
				iRet = tdr_iostream_write(a_pstIOStream, "%d ", (int)(char)pszHostStart[0]);
			}/*if(NULL != pstMacro)*/
			pszHostStart += a_pstEntry->iHUnitSize;
		}
		break;
	case TDR_TYPE_UCHAR:
	case TDR_TYPE_BYTE:	
		{
			pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)(unsigned char)pszHostStart[0]);
			if(NULL != pszVal)
			{
				iRet = tdr_iostream_write(a_pstIOStream, "%s ", pszVal);
			}else
			{
				iRet = tdr_iostream_write(a_pstIOStream, "0x%x ", (int)(unsigned char)pszHostStart[0]);
			}
			pszHostStart += a_pstEntry->iHUnitSize;
		}
		break;
	case TDR_TYPE_SMALLINT:
		{
			pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)*(short*)pszHostStart);
			if(NULL != pszVal)
			{
				iRet = tdr_iostream_write(a_pstIOStream, "%s ", pszVal);
			}else
			{
				iRet = tdr_iostream_write(a_pstIOStream, "%d ", (int)*(short*)pszHostStart);
			}
			pszHostStart += a_pstEntry->iHUnitSize;
			break;
		}		
	case TDR_TYPE_SMALLUINT:
		{
			pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)*(unsigned short*)pszHostStart);
			if(NULL != pszVal)
			{
				iRet = tdr_iostream_write(a_pstIOStream, "%s ", pszVal);
			}else
			{
				iRet = tdr_iostream_write(a_pstIOStream, "%d ", (int)*(unsigned short*)pszHostStart);
			}
			pszHostStart += a_pstEntry->iHUnitSize;
			break;
		}		
	case TDR_TYPE_LONG:
	case TDR_TYPE_INT:
		{
			pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)*(int*)pszHostStart);
			if(NULL != pszVal)
			{
				iRet = tdr_iostream_write(a_pstIOStream, "%s ", pszVal);
			}else
			{
				iRet = tdr_iostream_write(a_pstIOStream, "%d ", (int)*(int*)pszHostStart);
			}
			pszHostStart += a_pstEntry->iHUnitSize;
			break;
		}		
	case TDR_TYPE_ULONG:
	case TDR_TYPE_UINT:
		{
			pszVal = tdr_get_bindmacro_name_i(a_pstLib, a_pstEntry, (int)*(unsigned int*)pszHostStart);
			if(NULL != pszVal)
			{
				iRet = tdr_iostream_write(a_pstIOStream, "%s ", pszVal);
			}else
			{
				iRet = tdr_iostream_write(a_pstIOStream, "%u ", (unsigned 
					int)*(unsigned int*)pszHostStart);
			}
			pszHostStart += a_pstEntry->iHUnitSize;
			break;
		}		
	case TDR_TYPE_LONGLONG:
		{
#if defined(WIN32) &&  _MSC_VER < 1400  /*vc7,vc6,,*/
			iRet = tdr_iostream_write(a_pstIOStream, "%I64i ", *(tdr_longlong*)pszHostStart);
#else
			iRet = tdr_iostream_write(a_pstIOStream, "%lld ", *(tdr_longlong*)pszHostStart);
#endif
			pszHostStart += a_pstEntry->iHUnitSize;
			break;
		}

	case TDR_TYPE_ULONGLONG:
		{
#if defined(WIN32) &&  _MSC_VER < 1400  /*vc7,vc6,,*/
			iRet = tdr_iostream_write(a_pstIOStream, "%I64u ", *(tdr_ulonglong*)pszHostStart);
#else
			iRet = tdr_iostream_write(a_pstIOStream, "%llu ", *(tdr_ulonglong*)pszHostStart);
#endif
			pszHostStart += a_pstEntry->iHUnitSize;
			break;
		}		
	case TDR_TYPE_FLOAT:
		iRet = tdr_iostream_write(a_pstIOStream, "%f ", *(float*)pszHostStart);
		break;
	case TDR_TYPE_DOUBLE:
		iRet = tdr_iostream_write(a_pstIOStream, "%f ", *(double*)pszHostStart);
		pszHostStart += a_pstEntry->iHUnitSize;
		break;

	case TDR_TYPE_DATE:
		iRet = tdr_iostream_write(a_pstIOStream, "%s ", tdr_tdrdate_to_str((tdr_date_t *)pszHostStart));
		pszHostStart += a_pstEntry->iHUnitSize;
		break;
	case TDR_TYPE_TIME:
		iRet = tdr_iostream_write(a_pstIOStream, "%s ", tdr_tdrtime_to_str((tdr_time_t *)pszHostStart));
		pszHostStart += a_pstEntry->iHUnitSize;
		break;
	case TDR_TYPE_DATETIME:
		iRet = tdr_iostream_write(a_pstIOStream, "%s ", tdr_tdrdatetime_to_str((tdr_datetime_t *)pszHostStart));
		pszHostStart += a_pstEntry->iHUnitSize;
		break;
	case TDR_TYPE_IP:
		iRet = tdr_iostream_write(a_pstIOStream, "%s ", tdr_tdrip_to_ineta(*(tdr_ip_t *)pszHostStart));
		pszHostStart += a_pstEntry->iHUnitSize;
		break;
	case TDR_TYPE_WCHAR:
		{
			tdr_wchar_t szTemp[2] = {0};												
			char szMbs[4] = {0};														
			int iLen = sizeof(szMbs);	

			szTemp[0] = *(tdr_wchar_t *)pszHostStart;	
			tdr_wcstochinesembs(&szMbs[0], &iLen, &szTemp[0], 2);
			tdr_iostream_write(a_pstIOStream, "%s ",	szMbs);	
			pszHostStart += a_pstEntry->iHUnitSize;
			break;
		}
	case TDR_TYPE_STRING:
		{
			if (0 < a_pstEntry->iCustomHUnitSize)												
			{																					
				iSize = a_pstEntry->iCustomHUnitSize;											
			}else																				
			{																					
				iSize = a_pszHostEnd - pszHostStart;
			}																					
			iLen = tdr_strnlen(pszHostStart, iSize);												
			if (iLen >= iSize)																	
			{																					
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NET_INVALID_STRING_LEN);				
				break;																		    
			}
			iRet = tdr_iostream_write(a_pstIOStream, "%s ", pszHostStart);
			pszHostStart += iSize;
			break;	
		}
	case TDR_TYPE_WSTRING:
		{
			char *pszMbs = NULL;
			if (0 < a_pstEntry->iCustomHUnitSize)												
			{																					
				iSize = a_pstEntry->iCustomHUnitSize;											
			}else																				
			{																					
				iSize = a_pszHostEnd - pszHostStart;											
			}	
			pszMbs = tdr_wcstochinesembs_i((tdr_wchar_t *)pszHostStart, iSize/sizeof(tdr_wchar_t)); 
			if (NULL == pszMbs)
			{	
				iRet = tdr_iostream_write(a_pstIOStream, " ");
			}else
			{
				iRet = tdr_iostream_write(a_pstIOStream, "%s ", pszMbs);
				free(pszMbs);
			}
			pszHostStart += iSize;
			break;
		}
	default:	/* must be 8 bytes. */
		break;
	}/*switch(a_pstEntry->iType)*/

	*a_ppszHostStart = pszHostStart;

	return iRet;
}

/**根据从字符串值中输入基本数据类型成员的值
*@param[in] a_pstLib 元数据描述库指针
*@param[in] a_pstEntry 成员的描述结构指针
*@param[in] a_pszHostStart 此成员值存储空间的起始地址
*@param[in,out] a_piSize 保存输入数据内存空间大小的指针
*	-	输入	可使用的内存空间大小
*	-	输出	实际以使用的内存空间大小
*@return 成功返回0，否则返回非零值
*/
int tdr_ioscanf_basedtype_i(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, 
							 IN char *a_pszHostStart, INOUT int *a_piSize, IN const char *a_pszValue)
{
	int iRet = TDR_SUCCESS;
	long lVal;
	tdr_longlong llVal;
	int iSize;
	int iLen;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstEntry);
	assert(NULL != a_pszHostStart);
	assert(NULL != a_piSize);
	assert(NULL != a_pszValue);

	switch(a_pstEntry->iType)
	{
	case TDR_TYPE_CHAR:
		{
			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			TDR_SCANF_INT_FROM_STR(lVal, a_pstLib, a_pstEntry, a_pszValue);			
			if ((-128 > lVal) || (127 < lVal) )
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VALUE_BEYOND_TYPE_FIELD);
				break;
			}			
			*a_pszHostStart = (char)lVal;	
			*a_piSize = a_pstEntry->iHUnitSize;
			break;
		}
	case TDR_TYPE_BYTE:
	case TDR_TYPE_UCHAR:
		{
			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			TDR_SCANF_INT_FROM_STR(lVal, a_pstLib, a_pstEntry, a_pszValue);			
			if ((0 > lVal) || (0xFF < lVal) )
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VALUE_BEYOND_TYPE_FIELD);
				break;
			}			
			*a_pszHostStart = (unsigned char)lVal;	
			*a_piSize = a_pstEntry->iHUnitSize;
			break;			
		}
	case TDR_TYPE_SMALLINT:
		{
			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			TDR_SCANF_INT_FROM_STR(lVal, a_pstLib, a_pstEntry, a_pszValue);			
			if ((-32768 > lVal) || (0x7FFF < lVal) )
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VALUE_BEYOND_TYPE_FIELD);
				break;
			}			
			*(short *)a_pszHostStart = (short)lVal;	
			*a_piSize = a_pstEntry->iHUnitSize;
			break;
		}
	case TDR_TYPE_SMALLUINT:
		{
			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			TDR_SCANF_INT_FROM_STR(lVal, a_pstLib, a_pstEntry, a_pszValue);			
			if ((0 > lVal) || (0xFFFF < lVal) )
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VALUE_BEYOND_TYPE_FIELD);
				break;
			}			
			*(unsigned short *)a_pszHostStart = (unsigned short)lVal;	
			*a_piSize = a_pstEntry->iHUnitSize;
			break;			
		}
	case TDR_TYPE_INT:
	case TDR_TYPE_LONG:
		{
			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			TDR_SCANF_INT_FROM_STR(lVal, a_pstLib, a_pstEntry, a_pszValue);			
			if (((int)0x80000000 > (int)lVal) || (0x7FFFFFFF < (int)lVal) )
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VALUE_BEYOND_TYPE_FIELD);
				break;
			}			
			*(int *)a_pszHostStart = (int)lVal;	
			*a_piSize = a_pstEntry->iHUnitSize;
			break;				
		}
	case TDR_TYPE_UINT:
	case TDR_TYPE_ULONG:
		{
			LPTDRMACRO pstMacro = NULL;	
			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			if (TDR_INVALID_PTR != (a_pstEntry)->ptrMacrosGroup)					
			{																	
				pstMacro = tdr_get_bindmacro_by_name(a_pstLib, a_pstEntry, a_pszValue);
			}																		
			if (NULL != pstMacro)													
			{																																			
				llVal = pstMacro->iValue;											
			}else																	
			{	
				int iIdx = tdr_get_macro_index_by_name_i(a_pstLib, a_pszValue);
				if (TDR_INVALID_INDEX == iIdx)
				{
					llVal = TDR_ATOLL(a_pszValue);
				}else        
				{
					pstMacro = TDR_GET_MACRO_TABLE(a_pstLib);
					llVal = pstMacro[iIdx].iValue;
				}													
			}										
			if (0 > llVal) 
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VALUE_BEYOND_TYPE_FIELD);
				break;
			}			
			*(unsigned int *)a_pszHostStart = (unsigned int)llVal;	
			*a_piSize = a_pstEntry->iHUnitSize;			
			break;
		}
	case TDR_TYPE_LONGLONG:
		{
			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			llVal = TDR_ATOLL(a_pszValue);
			*(tdr_longlong *)a_pszHostStart = llVal;	
			*a_piSize = a_pstEntry->iHUnitSize;
			break;
		}
	case TDR_TYPE_ULONGLONG:
		{
			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			llVal = TDR_ATOLL(a_pszValue);
			*(tdr_ulonglong *)a_pszHostStart = (tdr_ulonglong)llVal;	
			*a_piSize = a_pstEntry->iHUnitSize;
			break;
		}
	case TDR_TYPE_FLOAT:
		{
			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			*(float *)a_pszHostStart = (float)atof(a_pszValue);	
			*a_piSize = a_pstEntry->iHUnitSize;
			break;
		}
	case TDR_TYPE_DOUBLE:
		{
			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			*(double *)a_pszHostStart = strtod(a_pszValue, NULL);	
			*a_piSize = a_pstEntry->iHUnitSize;
			break;
		}
	case TDR_TYPE_IP:
		{
			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			iRet = tdr_ineta_to_tdrip((tdr_ip_t *)a_pszHostStart , a_pszValue);
			*a_piSize = a_pstEntry->iHUnitSize;
			break;
		}	
	case TDR_TYPE_DATETIME:
		{
			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			iRet = tdr_str_to_tdrdatetime((tdr_datetime_t *)a_pszHostStart, a_pszValue);
			*a_piSize = a_pstEntry->iHUnitSize;
			break;
		}
	case TDR_TYPE_DATE:
		{
			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			iRet = tdr_str_to_tdrdate((tdr_date_t *)a_pszHostStart, a_pszValue);
			*a_piSize = a_pstEntry->iHUnitSize;
			break;			
		}
	case TDR_TYPE_TIME:
		{
			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			iRet = tdr_str_to_tdrtime((tdr_time_t *)a_pszHostStart, a_pszValue);
			*a_piSize = a_pstEntry->iHUnitSize;
			break;				
		}
	case TDR_TYPE_WCHAR:
		{
			tdr_wchar_t swTemp[8];
			int iLen = 8*sizeof(tdr_wchar_t);

			if (*a_piSize < a_pstEntry->iHUnitSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			iRet = tdr_chinesembstowcs((char *)&swTemp[0], &iLen, (char *)a_pszValue, 3);
			if (TDR_SUCCESS != iRet)
			{
				break;
			}
			*(tdr_wchar_t *)a_pszHostStart = swTemp[0];
			*a_piSize = a_pstEntry->iHUnitSize;
			break;
		}	
	case TDR_TYPE_WSTRING:
		{
			int iMbsLen;
			int iWcLen;
			if (0 < a_pstEntry->iCustomHUnitSize)													
			{																						
				iSize = a_pstEntry->iCustomHUnitSize;												
			}else
			{
				iSize = *a_piSize;
			}
			if ((int)(sizeof(tdr_wchar_t)) > iSize)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);
				break;
			}
			iMbsLen = strlen(a_pszValue) + 1;
			iWcLen = iSize;
			iRet = tdr_chinesembstowcs(a_pszHostStart, &iWcLen, (char *)a_pszValue, iMbsLen);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				*(tdr_wchar_t *)a_pszHostStart = L'\0';
				break;
			}
			*a_piSize = iSize;
			break;
		}
	case TDR_TYPE_STRING:
		{
			if (0 < a_pstEntry->iCustomHUnitSize)													
			{																						
				iSize = a_pstEntry->iCustomHUnitSize;												
			}else
			{
				iSize = *a_piSize;
			}
			
			iLen = tdr_strnlen(a_pszValue, iSize) + 1;																
			if (iLen > iSize) 						
			{																						
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NET_INVALID_STRING_LEN);					
				break;																				
			}	
			if (iLen > *a_piSize)
			{																						
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);					
				break;																				
			}	
			TDR_MEMCPY(a_pszHostStart, a_pszValue, iLen, TDR_MIN_COPY);   	
			*a_piSize = iSize;
			break;
		}
	default:			
		break;
	}

	return iRet;
}

char *tdr_get_bindmacro_name_i(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, IN int a_iValue)
{

	char *pchDst;
	char *pchSrc;
	char pch;
	LPTDRMACROSGROUP pstGroup;
	LPTDRMACRO pstMacro;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstEntry);

	if (TDR_INVALID_PTR == a_pstEntry->ptrMacrosGroup)
	{
		return NULL;
	}
	pstMacro = tdr_get_bindmacro_by_value(a_pstLib, a_pstEntry, a_iValue);
	if (NULL == pstMacro)
	{
		return NULL;
	}

	pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, a_pstEntry->ptrMacrosGroup);
	pchSrc = &pstMacro->szMacro[0];
	pchDst = &pstGroup->szName[0];
	while (*pchSrc && *pchDst)
	{
		pch = (char)toupper(*pchDst);
		if (*pchSrc != pch)
		{
			break;
		}
		pchSrc++;
		pchDst++;
	}

	if ('\0' == *pchDst)
	{
		return pchSrc + 1;
	}

	return &pstMacro->szMacro[0];
}

const char *tdr_get_entry_name(IN LPTDRMETAENTRY a_pstEntry)
{
	//assert(NULL != a_pstEntry);
	if (NULL == a_pstEntry)
	{
		return "";
	}

	return a_pstEntry->szName;
}

const char *tdr_get_entry_id_name(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry)
{
	LPTDRMACRO pstMacroTable = NULL;
	char *pszName = "";

	//assert(NULL != a_pstLib);
	//assert(NULL != a_pstEntry);
	if ((NULL == a_pstLib)||(NULL == a_pstEntry))
	{
		return "";
	}

	

	pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);
	if ((TDR_INVALID_INDEX != a_pstEntry->idxID) && (a_pstEntry->idxID < a_pstLib->iCurMacroNum))
	{
		pszName = pstMacroTable[a_pstEntry->idxID].szMacro;
	}

	return pszName;
}

int tdr_do_have_autoincrement_entry(IN LPTDRMETA a_pstMeta)
{
	//assert(NULL != a_pstMeta);
	if (NULL == a_pstMeta)
	{
		return 0;
	}

	return TDR_META_DO_HAVE_AUTOINCREMENT_ENTRY(a_pstMeta);
}

const char *tdr_get_entry_customattr(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry)
{
	char *pszCustomAttr = NULL;

	//assert(NULL != a_pstEntry);
	//assert(NULL != a_pstLib);
	if ((NULL == a_pstLib)||(NULL == a_pstEntry))
	{
		return "";
	}

	if (TDR_INVALID_PTR != a_pstEntry->ptrCustomAttr)
	{
		pszCustomAttr = TDR_GET_STRING_BY_PTR(a_pstLib, a_pstEntry->ptrCustomAttr);
	}

	return pszCustomAttr;
}
