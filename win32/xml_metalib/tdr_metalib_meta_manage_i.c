/**
*
* @file     tdr_metalib_manage.c
* @brief    TSF4G-元数据描述库核心管理模块
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-03-22 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <assert.h>
#include <stdlib.h>

#include "tdr/tdr_os.h"
#include "tdr/tdr_metalib_manage_i.h"
#include "tdr/tdr_error_i.h"
#include "tdr_XMLtags_i.h"
#include "tdr/tdr_ctypes_info_i.h"
#include "tdr_metaleb_param_i.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_auxtools.h"
#include "tdr_metalib_entry_manage_i.h"

#include "tdr_metalib_meta_manage_i.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

static void tdr_init_meta_data_i(LPTDRMETA a_pstMeta, LPTDRMETALIB a_pstLib, int a_iMetaSize, int a_iEntryNum);






/**获取meta结构的type信息 
*/
static int tdr_get_meta_type_i(LPTDRMETA a_pstMeta, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError);

/**获取 struct元素的size属性
*/
static int tdr_get_struct_meta_size_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

/**获取struct元素的字节对齐align 属性
*/
static int tdr_get_struct_meta_align_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

/**获取struct元素的versionindicator属性 
*/
static int tdr_get_stuct_meta_versionindicator_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

/**获取struct元素的sizetype属性 
*/
static int tdr_get_struct_meta_packsize_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError, const char *a_pszAttr);

/**获取struct元素的建表分表因子 
*/
static int tdr_get_struct_meta_splittablefactor_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_get_struct_meta_splittablekey_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_get_struct_meta_splittablerule_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_get_struct_meta_strictinput_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_get_struct_meta_dependonstruct_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_get_struct_meta_uniqueentryname_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);


/**获取struct元素的主键属性 
*/
static int tdr_get_struct_meta_primarykey_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);


/**根据meta的ID在metalib中查找此meta 
*/
LPTDRMETA tdr_get_meta_by_id_i(TDRMETALIB* a_pstLib, int a_iID);



/** 检查原数据描述库是否有足够的空间存储stuct/union元素
*@param[inout] pstLib 需要更新的元数据库
*@param[in] iMetaSize meta需占的空间
*@param[in] iStrBufSize meta需要占的字符串缓冲区大小
*@param[in] fpError 用来记录处理错误信息的文件句柄
*
*@return 成功返回TDR_SUCCESS,否则返回错误号
*
*@pre \e pstLib 不能为NULL
*@pre \e fpError 不能为NULL
*/
static int tdr_check_space_for_meta_i(LPTDRMETALIB a_pstLib, int iMetaSize, int iStrBufSize, FILE *fpError);

/** 读取meta的公共属性
*@param[out] a_pstMeta    需要更新的meta
*@param[inout] a_pstLib 需要更新的元数据库
*@param[in] a_pstElement 需添加的元素
*@param[in] a_fpError 用来记录处理错误信息的文件句柄
*
*@return 成功返回TDR_SUCCESS,否则返回错误号
*
*@pre \e a_pstLib 不能为NULL
*@pre \e a_pstElement 不能为NULL
*@pre \e a_fpError 不能为NULL
*@pre \e 调用此函数之前，必须先调用tdr_get_meta_type_i获取meta的类型
*/
static int tdr_get_mata_common_attribute_i(LPTDRMETA a_pstMeta, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError);


static int tdr_parse_meta_versionindicator_i(LPTDRMETA a_pstMeta, const char* a_pszName);


/**检查与Table对应的meta结构db相关的属性是否合法 
*/
static int tdr_check_table_meta_dbattributes_i(IN LPTDRMETA a_pstMeta, IN FILE *a_fpError);

/**检查依赖meta结构db相关的属性,进行正规化
*/
static int tdr_check_depend_meta_dbattributes_i(IN LPTDRMETA a_pstMeta, IN FILE *a_fpError);


/**检查将struct成员扩展生成建表语句时，是否有重名的成员出现
将a_pstCheckMeta的成员与a_pstBaseMeta的前iMaxIdx个成员进行比较
*@return 0： 没有重名； 非零值：有重名
*/
static int tdr_check_meta_extendable_to_table_i(LPTDRMETA a_pstBaseMeta, int iMaxIdx, LPTDRMETA a_pstCheckMeta, OUT char *pszConflictName);

static int tdr_check_dbtable_name_conflict_i(LPTDRMETA a_pstBaseMeta, int iMaxIdx, const char *pszName);


/**根据成员名在结构中查找 
*/
static int tdr_find_tablecolumn_entry_i(IN LPTDRDBKEYINFO a_pstDBKeyInfo, IN LPTDRMETA a_pstMeta, IN const char *a_pszName);

/**获取meta的sortkey信息
*/
static  int tdr_get_meta_sortkey_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *fpError);

/*取宏定义组数据区已经使用的空间*/
static int tdr_get_macrogroup_used_size_i(IN LPTDRMETALIB a_pstLib);

static int  tdr_create_macrosgroup_i(INOUT LPTDRMACROSGROUP *a_ppstMacrosGroup, IN TDRMETALIB* a_pstLib, IN scew_element *a_pstElement, IN FILE *a_fpError);

/** 获取macrosgroup元素的属性
*@return 成功返回0，否则返回非零值
*/
static int tdr_get_macrosgroup_attributes(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib, IN scew_element *a_pstElement, IN FILE *a_fpError);

/** 将macrosgroup元素的macro子元素加入到元数据库中
*@return 成功返回0，否则返回非零值
*/
static int tdr_add_macros_of_group_i(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib, IN scew_element *a_pstElement, IN FILE *a_fpError);

static void tdr_sort_macrosgroup_valueidxmap_i(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib);
static void tdr_sort_macrosgroup_nameidxmap_i(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib);

/*分析在DB中使用的成员的偏移值*/
static int tdr_parse_entryoff_db_i(IN LPTDRMETA a_pstMeta, OUT LPTDRSelector a_pstSelector, IN const char *a_pszAttr,
								   IN const char* a_pszName, IN FILE *a_fpError);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int tdr_add_macros_i(TDRMETALIB* a_pstLib, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_element* pstItem = NULL;
    int iMacroNum = 0;
    int iRet = TDR_SUCCESS;
	LPTDRMACRO pstMacro = NULL;

    assert(NULL != a_pstLib);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);


    iMacroNum =	scew_element_count(a_pstElement);    
    
    
    pstItem = scew_element_next(a_pstElement, NULL);    
    while (NULL != pstItem)
    {
        if (0 != tdr_stricmp(scew_element_name(pstItem), TDR_TAG_MACRO))
        {
            pstItem = scew_element_next(a_pstElement, pstItem);  
            continue;
        }

        if( a_pstLib->iCurMacroNum >= a_pstLib->iMaxMacroNum )
        {
            
            fprintf(a_fpError, "error:\t metalib最多能存储%d个宏定义，目前已存储%d个,不能再加入宏定义.\n",
                a_pstLib->iMaxMacroNum, a_pstLib->iCurMacroNum);
            iRet =  TDR_ERRIMPLE_TO_MUCH_MACROS;
            break;
        }

        iRet = tdr_add_macro_i(a_pstLib, pstItem, a_fpError, &pstMacro);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }        

        pstItem = scew_element_next(a_pstElement, pstItem);         
    }    
    
    return iRet;
}




int tdr_add_macro_i(TDRMETALIB* a_pstLib, scew_element *a_pstElement, FILE *a_fpError, INOUT LPTDRMACRO *a_ppMacro)
{
    int iRet = TDR_SUCCESS;
    LPTDRMACRO pstMacrosTable = NULL;
    LPTDRMACRO pstMacro = NULL;
    scew_attribute *pstAttr = NULL;
    const char *pszMacroValTag = NULL;
    char szMacro[TDR_MACRO_LEN] = {0};
    int idxFindMacro = TDR_INVALID_INDEX;

    assert(NULL != a_pstLib);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);
	assert(NULL != a_ppMacro);

    /*获得当前空闲宏定义块的地址*/
    pstMacrosTable = TDR_GET_MACRO_TABLE(a_pstLib);    
    pstMacro = pstMacrosTable + a_pstLib->iCurMacroNum;
    memset(pstMacro, 0, sizeof(TDRMACRO));

    /*获取宏定义的名字*/
    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_NAME);    
    if ( NULL == pstAttr)
    {
        fprintf(a_fpError, "\nerror:\t 第%d个(从最前面开始)macro元素没有name属性\n", a_pstLib->iCurMacroNum +1);
        
        iRet = TDR_ERRIMPLE_MACRO_NO_NAME_ATTR;        
    }else
    {
        const char *pszName = scew_attribute_value(pstAttr);
        assert(NULL != pszName);
  
		tdr_normalize_string(szMacro, (int)sizeof(szMacro), pszName);
    }/*if ( NULL == pstAttr) */    


     /*检查宏定义name的有效性*/
    if (!TDR_ERR_IS_ERROR(iRet) && (szMacro[0] == '\0'))
    {
        
        fprintf(a_fpError, "\nerror:\t 第%d个(从最前面开始)macro元素的name属性值为空串\n", a_pstLib->iCurMacroNum +1);
        iRet = TDR_ERRIMPLE_MACRO_NO_NAME_ATTR;
    }
    


    /*查看是否有重名宏定义*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        idxFindMacro = tdr_get_macro_index_by_name_i(a_pstLib, szMacro);
        if (TDR_INVALID_INDEX != idxFindMacro)
        {
             fprintf(a_fpError, "\nwarning:\t  第%d个(从最前面开始)macro宏定义元素<name=%s> 和前面的macro同名,则当前宏的值会取代前面定义的值\n",
                a_pstLib->iCurMacroNum +1, szMacro);
            iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_MACRO_NAME_CONFLICT);
            pstMacro = pstMacrosTable + idxFindMacro;
        }else
        {
            pstMacro = pstMacrosTable + a_pstLib->iCurMacroNum;
        }
        TDR_STRNCPY(pstMacro->szMacro, szMacro, sizeof(pstMacro->szMacro));

    }/*if (!TDR_ERR_IS_ERROR(iRet))*/ 
    
    
    /*获取宏定义的值*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        TDR_GET_MACRO_VALUE_TAG(pszMacroValTag, a_pstLib->iXMLTagSetVer);
        assert(NULL != pszMacroValTag);

        pstAttr	= scew_attribute_by_name(a_pstElement, pszMacroValTag);
        if(NULL == pstAttr )
        {
            fprintf(a_fpError, "\nerror:\t  第%d个(从最前面开始)macro宏定义元素<name=%s> 没有定义%s属性\n",
                a_pstLib->iCurMacroNum +1, pstMacro->szMacro, pszMacroValTag);
            
            iRet = TDR_ERRIMPLE_MACRO_NO_VALUE;
        }else
        {
			int idx;
			char szMacroValue[TDR_MACRO_LEN];

			tdr_normalize_string(szMacroValue, (int)sizeof(szMacroValue), scew_attribute_value(pstAttr));
			iRet = tdr_get_macro_int_i(&pstMacro->iValue, &idx, a_pstLib, szMacroValue);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				fprintf(a_fpError, "macro宏定义元素<name=%s> 的属性值<%s>无效,目前宏定义属性值仅支持整数和已定于宏名两种方式\n", pstMacro->szMacro, scew_attribute_value(pstAttr));
			}
        }
    } /*if (!TDR_ERR_IS_ERROR(iRet))*/ 


	/*获取宏定义的描述值*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_get_desc_attribute_i(&pstMacro->ptrDesc, a_pstLib, a_pstElement, a_fpError);		
	} /*if (!TDR_ERR_IS_ERROR(iRet))*/ 


    /*没有重名的宏，则增长宏定义的表*/
    if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_INDEX == idxFindMacro))
    {
        a_pstLib->iCurMacroNum++;
    }
    
	*a_ppMacro = pstMacro;
    
    return iRet;
}




int tdr_check_space_for_meta_i(LPTDRMETALIB a_pstLib, int a_iMetaSize, int a_iStrBufSize, FILE *a_fpError)
{
    int iFreeMataSpace = 0;
    
    
    assert(NULL != a_pstLib);
    assert(0 <= a_iMetaSize);
    assert(0 <= a_iStrBufSize);
    assert(NULL != a_fpError);

    if (a_pstLib->iCurMetaNum >= a_pstLib->iMaxMetaNum )
    {
        fprintf(a_fpError, "\nerror:\t 元数据描述库最多能存储%d个自定义类型，目前已经添加了%d个",
            a_pstLib->iMaxMetaNum, a_pstLib->iCurMetaNum);
        
        return TDR_ERRIMPLE_NO_SPACE_FOR_MATA;
    }

    /*检查存储meta的剩余空间是否足够*/
    iFreeMataSpace = TDR_GET_FREE_META_SPACE(a_pstLib);
    if (iFreeMataSpace < a_iMetaSize)
    {
        fprintf(a_fpError, "\nerror:\t 元数据描述库存储meta信息剩余空间为%d字节，但实际需要%d个字节\n",
            iFreeMataSpace, a_iMetaSize);
        
        return TDR_ERRIMPLE_NO_SPACE_FOR_MATA;
    }
    
    /*检查字符串缓冲的空间是否足够*/
    if (a_pstLib->iFreeStrBufSize < a_iStrBufSize)
    {
        fprintf(a_fpError, "\nerror:\t 元数据描述库字符串缓冲区剩余空间为%d字节，但实际需要%d个字节\n",
            a_pstLib->iFreeStrBufSize, a_iStrBufSize);
        
        return TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
    }

    return TDR_SUCCESS;
}

int tdr_get_mata_common_attribute_i(LPTDRMETA a_pstMeta, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError)
{
    int iRet = 0;
    LPTDRCTYPEINFO pstTypeInfo = NULL;
    LPTDRMETA pstFindedMeta = NULL;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstLib);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    pstTypeInfo = tdr_idx_to_typeinfo(a_pstMeta->idxType);
    assert(NULL != pstTypeInfo);


    /*获取name 属性*/
    iRet = tdr_get_name_attribute_i(&a_pstMeta->szName[0], sizeof(a_pstMeta->szName), a_pstElement);
    if ((!TDR_ERR_IS_ERROR(iRet)) && (TDR_XML_TAGSET_VERSION_0 < a_pstLib->iXMLTagSetVer))
    {
        /*新版定义需检查name属性的有效性*/
        if (a_pstMeta->szName[0] == '\0') /*空串或没有定义*/
        {  
            fprintf(a_fpError, "error:\t 第%d个自定义<type =%s>数据元素没有定义name属性或其值为空串\n", 
                a_pstLib->iCurMetaNum + 1, pstTypeInfo->pszCName);
            
            iRet = TDR_ERRIMPLE_META_NO_NAME;
        }else 
        {
            /*判断是否有重名meta存在*/            
            pstFindedMeta = tdr_get_meta_by_name_i(a_pstLib, &a_pstMeta->szName[0]);
            if (NULL != pstFindedMeta)
            {
                fprintf(a_fpError, "error:\t 元数据描述库<%s>存在两个名字<name = %s>相同的%s元素\n", a_pstLib->szName, a_pstMeta->szName, pstTypeInfo->pszCName);
                
                iRet =TDR_ERRIMPLE_META_NAME_CONFLICT;
            }
        }/*if (a_pstMeta->szName[0] == '\0')*/
    }/*if ((!TDR_ERR_IS_ERROR(iRet)) && (TDR_XML_TAGSET_VERSION_0 < a_pstLib->iXMLTagSetVer))*/
   
    /*读取ID属性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_id_attribute_i(&a_pstMeta->iID, &a_pstMeta->idxID, a_pstLib, a_pstElement, a_fpError);
        if (TDR_SUCCESS == iRet)
        {
            TDR_META_SET_HAVE_ID(a_pstMeta);

			if (0 > a_pstMeta->iID)
			{
				fprintf(a_fpError, "error:\t %s元素<name = %s>的ID<%d>不能为负数\n", 
					pstTypeInfo->pszCName, a_pstMeta->szName, a_pstMeta->iID);

				iRet =TDR_ERRIMPLE_META_ID_CONFLICT;
			}else	if ( TDR_XML_TAGSET_VERSION_0 < a_pstLib->iXMLTagSetVer)
			{
					/*同层节点Id不容许重复*/
				pstFindedMeta = tdr_get_meta_by_id_i(a_pstLib, a_pstMeta->iID);
				if (NULL != pstFindedMeta)
				{
					fprintf(a_fpError, "error:\t %s元素<name = %s>的ID<%d>与前面定义的元素相同\n", 
						pstTypeInfo->pszCName, a_pstMeta->szName, a_pstMeta->iID);

					iRet =TDR_ERRIMPLE_META_ID_CONFLICT;
				}  
			}
                     
        }/*if ((TDR_SUCCESS == iRet) && (TDR_XML_TAGSET_VERSION_0 < a_pstLib->iXMLTagSetVer))*/
    }   

    /*version*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_version_attribute_i(&a_pstMeta->iBaseVersion, &a_pstMeta->idxVersion, a_pstLib, a_pstElement, a_fpError);
        if ((TDR_SUCCESS != iRet) && (TDR_XML_TAGSET_VERSION_0 < a_pstLib->iXMLTagSetVer))
        {
            fprintf(a_fpError, "\nerror:\t <%s>类型<name = %s>必须包含version属性，以指定此类型加入元数据库时的基准版本\n", 
                pstTypeInfo->pszCName, a_pstMeta->szName);
            
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_VERSION);
        }else if (TDR_SUCCESS == iRet)
        {
            a_pstMeta->iCurVersion = a_pstMeta->iBaseVersion;   /*将当前版本先设置为基准版本*/
        }        
    }

    
    if (!TDR_ERR_IS_ERROR(iRet) && (TDR_XML_TAGSET_VERSION_0 < a_pstLib->iXMLTagSetVer))
    {        
        /*desc*/
        iRet = tdr_get_desc_attribute_i(&a_pstMeta->ptrDesc, a_pstLib, a_pstElement, a_fpError);

        if (!TDR_ERR_IS_ERROR(iRet))
        {
            /*cname*/
            iRet = tdr_get_cname_attribute_i(&a_pstMeta->ptrChineseName, a_pstLib, a_pstElement, a_fpError);
        }

		/*align*/
		if (!TDR_ERR_IS_ERROR(iRet))
		{
			iRet = tdr_get_struct_meta_align_attribute_i(a_pstMeta, a_pstElement, a_fpError);
		}

    }    
    
    return iRet;
}





void tdr_init_meta_data_i(LPTDRMETA a_pstMeta, LPTDRMETALIB a_pstLib, int a_iMetaSize, int a_iEntryNum)
{
    memset( a_pstMeta, 0 , sizeof(TDRMETA) );

	a_pstMeta->iMemSize = a_iMetaSize;
    a_pstMeta->iType = TDR_TYPE_UNKOWN;
    a_pstMeta->iIdx = a_pstLib->iCurMetaNum;
    a_pstMeta->iMaxSubID = TDR_INVALID_ID;
    
    a_pstMeta->idxID = TDR_INVALID_INDEX;
	a_pstMeta->iID = TDR_INVALID_INDEX;
    a_pstMeta->idxVersion = TDR_INVALID_INDEX;	
    
    a_pstMeta->ptrMeta = a_pstLib->ptrLaseMeta;

	a_pstMeta->idxType = TDR_INVALID_INDEX;
    a_pstMeta->ptrDesc = TDR_INVALID_PTR;

    a_pstMeta->ptrChineseName = TDR_INVALID_PTR;
    a_pstMeta->ptrDesc = TDR_INVALID_PTR;

	if (TDR_XML_TAGSET_VERSION_0 == a_pstLib->iXMLTagSetVer)
	{
		a_pstMeta->iBaseVersion = a_pstLib->lVersion;
		a_pstMeta->iCurVersion = a_pstMeta->iBaseVersion ;
	}

    a_pstMeta->iCustomAlign = TDR_DEFAULT_ALIGN_VALUE;
	a_pstMeta->iValidAlign = TDR_DEFAULT_ALIGN_VALUE;
    a_pstMeta->idxCustomHUnitSize = TDR_INVALID_INDEX;

	a_pstMeta->stVersionIndicator.iHOff = TDR_INVALID_OFFSET;
	a_pstMeta->stVersionIndicator.iNOff = TDR_INVALID_OFFSET;

	a_pstMeta->stSizeType.idxSizeType = TDR_INVALID_INDEX;
	a_pstMeta->stSizeType.iHOff = TDR_INVALID_OFFSET;
	a_pstMeta->stSizeType.iNOff = TDR_INVALID_OFFSET;

	a_pstMeta->ptrPrimayKeyBase = TDR_CALC_MIN_META_SIZE(a_iEntryNum);
	a_pstMeta->stSplitTableKey.ptrEntry = TDR_INVALID_PTR;
	a_pstMeta->stSplitTableKey.iHOff = TDR_INVALID_OFFSET;
	a_pstMeta->idxSplitTableFactor = TDR_INVALID_INDEX;

	a_pstMeta->ptrDependonStruct = TDR_INVALID_PTR;

	a_pstMeta->stSortKey.iSortKeyOff = TDR_INVALID_OFFSET;
	a_pstMeta->stSortKey.idxSortEntry = TDR_INVALID_ID;
	a_pstMeta->stSortKey.ptrSortKeyMeta = TDR_INVALID_PTR;


	TDR_META_CLR_HAVE_ID(a_pstMeta);
}


int tdr_add_meta_base_i(TDRMETALIB* a_pstLib, scew_element *a_pstElement, scew_element *a_pstRoot, FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETA pstFreeMeta = NULL;
    int iMetaSize = 0;
    int iStrBufSize = 0;
    int iEntrisNum = 0;
    LPTDRMAPENTRY pstMetaMap = NULL;
    LPTDRNAMEENTRY pstMetaNameMap = NULL;
    LPTDRIDENTRY    pstMetaIDMap = NULL;

    assert(NULL != a_pstLib);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    /*获取meta的size*/
    tdr_get_meta_size_i(&iEntrisNum, &iMetaSize, &iStrBufSize, a_pstLib->iXMLTagSetVer, a_pstElement, a_pstRoot);
    if (0 >= iEntrisNum)
    {
        char szName[TDR_NAME_LEN] = {0};
        const char *pszItemName = NULL;

        TDR_GET_ENTRY_TAG_NAME(pszItemName, a_pstLib->iXMLTagSetVer);
        tdr_get_name_attribute_i(&szName[0], sizeof(szName), a_pstElement);
        fprintf(a_fpError, "\nerror:\t 自定义类型《name = %s> 没有包含任何\"%s\"子元素成员", szName, pszItemName);
        iRet = TDR_ERRIMPLE_META_NO_ENTRY;
    }
    

    /*检查是否有足够的空间存储此meta*/
   if (!TDR_ERR_IS_ERROR(iRet))
   {
        iRet = tdr_check_space_for_meta_i(a_pstLib, iMetaSize, iStrBufSize, a_fpError);
   }
    
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        /*取一个空闲meta结构并初始化*/
        pstFreeMeta = (LPTDRMETA) (a_pstLib->data + a_pstLib->ptrLaseMeta);
        tdr_init_meta_data_i(pstFreeMeta, a_pstLib, iMetaSize, iEntrisNum);        
        
        /*读取meta的类型信息*/
        iRet = tdr_get_meta_type_i(pstFreeMeta, a_pstLib, a_pstElement, a_fpError);       
    }

     /*读取此meta公共属性的*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_mata_common_attribute_i(pstFreeMeta, a_pstLib, a_pstElement, a_fpError);
    }
    

    /*生成meta的映射信息*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        pstMetaMap = TDR_GET_MAP_TABLE(a_pstLib) + a_pstLib->iCurMetaNum;
        pstMetaMap->iPtr = a_pstLib->ptrLaseMeta;
        pstMetaMap->iSize = iMetaSize;

        pstMetaNameMap = TDR_GET_META_NAME_MAP_TABLE(a_pstLib) + a_pstLib->iCurMetaNum;
        TDR_STRNCPY(pstMetaNameMap->szName, pstFreeMeta->szName, sizeof(pstMetaNameMap->szName));
        pstMetaNameMap->iIdx = a_pstLib->iCurMetaNum;

        pstMetaIDMap = TDR_GET_META_ID_MAP_TABLE(a_pstLib) + a_pstLib->iCurMetaNum;
		if (TDR_META_DO_HAVE_ID(pstFreeMeta))
		{
			pstMetaIDMap->iID = pstFreeMeta->iID;
		}else
		{
			pstMetaIDMap->iID = TDR_INVALID_ID;
		}
        
        pstMetaIDMap->iIdx = a_pstLib->iCurMetaNum;

        a_pstLib->iCurMetaNum++;
        a_pstLib->ptrLaseMeta += pstMetaMap->iSize;
    }
   
    return iRet;
}

int tdr_get_meta_type_i(LPTDRMETA a_pstMeta, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    scew_attribute *pstAttr = NULL;
    const char *pszTypeName = NULL;
    LPTDRCTYPEINFO pstTypeInfo = NULL;
    int iTypeIndex = TDR_INVALID_INDEX;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstLib);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    /*获取此元素的具体类型*/
    if (TDR_XML_TAGSET_VERSION_0 == a_pstLib->iXMLTagSetVer)
    {
        pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_CLASS);    
        if( NULL != pstAttr )
        {
            pszTypeName = scew_attribute_value(pstAttr);     
        }else
        {   
            pszTypeName = TDR_TAG_STRUCT;           
        }/*if( NULL != pstAttr )*/     
    }else 
    {
        pszTypeName = scew_element_name(a_pstElement); 
    }

     
    iTypeIndex = tdr_typename_to_idx(pszTypeName);    
    if (TDR_INVALID_INDEX != iTypeIndex)
    {
        pstTypeInfo = tdr_idx_to_typeinfo(iTypeIndex);
        a_pstMeta->iType = pstTypeInfo->iType;  
        a_pstMeta->idxType = iTypeIndex; 
    }else
    {
        char szName[TDR_NAME_LEN] = {0};
  
        tdr_get_name_attribute_i(&szName[0], sizeof(szName), a_pstElement);
        fprintf(a_fpError, "\nerror:\t 不支持第%d个自定义结构<name = %s>的数据类型: %s", 
            a_pstLib->iCurMetaNum + 1, szName, pszTypeName);
        
        iRet = TDR_ERRIMPLE_UNSUPPORTED_TYPE;
    }

    return iRet;
}



LPTDRMETA tdr_get_meta_by_id_i(TDRMETALIB* a_pstLib, int a_iID)
{
    int i;
    LPTDRMAPENTRY pstMap;
    LPTDRMETA pstMeta;

    assert(NULL != a_pstLib);
    
    pstMap = TDR_GET_MAP_TABLE(a_pstLib);
    
    for(i=0; i< a_pstLib->iCurMetaNum; i++)
    {
        pstMeta	= (LPTDRMETA) (a_pstLib->data + pstMap[i].iPtr);
        if (TDR_META_DO_HAVE_ID(pstMeta) && ( pstMeta->iID == a_iID ))
        {
            return pstMeta;
        }
    }
    
    return NULL;
}

int tdr_add_meta_entries_i(LPTDRMETA a_pstMeta, scew_element *a_stElement, FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    scew_element *pstSubItem = NULL;
    LPTDRMETALIB pstLib = NULL;
    const char *pszEntryName = NULL;

    assert(NULL != a_pstMeta);
    assert(NULL != a_stElement);
    assert(NULL != a_fpError);

    

   
    pstLib = TDR_META_TO_LIB(a_pstMeta);
    TDR_GET_ENTRY_TAG_NAME(pszEntryName, pstLib->iXMLTagSetVer);
    assert(NULL != pszEntryName);

     /*添加成员变量*/
    pstSubItem = scew_element_next(a_stElement, NULL);
    while (NULL != pstSubItem)
    {
        if (0 != tdr_stricmp(scew_element_name(pstSubItem), pszEntryName))
        {
			fprintf(a_fpError, "\nwarning:\t 自定义类型<name = %s>包含无效的成员<tag = %s>定义,请使用标签<%s>来定义成员\n",
				a_pstMeta->szName, scew_element_name(pstSubItem), pszEntryName);
            pstSubItem = scew_element_next(a_stElement, pstSubItem);
            continue;
        }


        iRet = tdr_add_entry_to_meta_i(a_pstMeta, pstSubItem, a_fpError);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            break;
        }
        pstSubItem = scew_element_next(a_stElement, pstSubItem);
    }  

    return iRet;
}

int tdr_get_meta_specail_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = NULL;
    

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);
	

    pstLib = TDR_META_TO_LIB(a_pstMeta);
    if (TDR_TYPE_STRUCT == a_pstMeta->iType)
    {
        /*读取struct结构的特殊属性*/
        iRet = tdr_get_struct_meta_size_attribute_i(a_pstMeta, a_pstElement, a_fpError);
       
        /*versionindictor*/
        if (!TDR_ERR_IS_ERROR(iRet))
        {
            iRet = tdr_get_stuct_meta_versionindicator_attribute_i(a_pstMeta, a_pstElement, a_fpError);
        }

        /*sizetype属性,此属性已被sizeinfo属性所替代*/
        if (!TDR_ERR_IS_ERROR(iRet))
        {
            iRet = tdr_get_struct_meta_packsize_attribute_i(a_pstMeta, a_pstElement, a_fpError, TDR_TAG_SIZETYPE);
        }

		/*sizeinfo属性*/
		if (!TDR_ERR_IS_ERROR(iRet))
		{
			iRet = tdr_get_struct_meta_packsize_attribute_i(a_pstMeta, a_pstElement, a_fpError, TDR_TAG_SIZEINFO);
		}

		if (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer)
		{
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_meta_sortkey_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}


			/*primarykey*/
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_struct_meta_primarykey_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}

			/*splittablefactor*/
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_struct_meta_splittablefactor_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}	

			/*splittablekey*/
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_struct_meta_splittablekey_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}	

			/*splittablerule*/
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_struct_meta_splittablerule_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}	

			/*strictinput*/
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_struct_meta_strictinput_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}

			/*dependonstruct*/
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_struct_meta_dependonstruct_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}

			/*uniqueentryname*/
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_get_struct_meta_uniqueentryname_attribute_i(a_pstMeta, a_pstElement, a_fpError);
			}
		}/*if (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer)*/

    }/*if ((!TDR_ERR_IS_ERROR(iRet)) && (TDR_TYPE_STRUCT == pstMeta->iType))*/

    if (!TDR_ERR_IS_ERROR(iRet) &&
        (TDR_XML_TAGSET_VERSION_0 == pstLib->iXMLTagSetVer) && (TDR_TYPE_UNION == a_pstMeta->iType))
    {
        iRet = tdr_get_struct_meta_packsize_attribute_i(a_pstMeta, a_pstElement, a_fpError, TDR_TAG_SIZETYPE);
    }
  
    return iRet;
}

int tdr_get_struct_meta_size_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = NULL;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);


    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SIZE);    
    if( NULL != pstAttr )
    {
        const char *pszVal = scew_attribute_value(pstAttr);

		pstLib = TDR_META_TO_LIB(a_pstMeta);
        iRet = tdr_get_macro_int_i(&a_pstMeta->iCustomHUnitSize, &a_pstMeta->idxCustomHUnitSize, pstLib, pszVal);
        if ((TDR_ERR_IS_ERROR(iRet)) || (0 >= a_pstMeta->iCustomHUnitSize))
        {
            fprintf(a_fpError, "error:\t 元素<name = %s>的size属性值<%s>无效,请确定其值是正整数或已定义的宏\n",
                a_pstMeta->szName, pszVal);
            
            iRet = TDR_ERRIMPLE_META_INVALID_SIZE_VALUE;
        }     
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/

    return iRet;
}

int tdr_get_struct_meta_align_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    int iRet = TDR_SUCCESS;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);


    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_ALIGN);    
    if( NULL != pstAttr )
    {
        const char *pszVal = scew_attribute_value(pstAttr);
        a_pstMeta->iCustomAlign = (int)strtol(pszVal, NULL, 0);
        if (0 >= a_pstMeta->iCustomAlign)
        {
            fprintf(a_fpError, "error:\t 自定义类型<name = %s>的align属性值<%s>无效,请确定其值是正整数或已定义的宏\n",
                a_pstMeta->szName, pszVal);
            
            iRet = TDR_ERRIMPLE_META_INVALID_ALIGN_VALUE;
        }    
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/

    return iRet;
}


int tdr_get_stuct_meta_versionindicator_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib =NULL;       

    const char *pszVersion = NULL; 

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);


    pstLib = TDR_META_TO_LIB(a_pstMeta);
	a_pstMeta->iVersionIndicatorMinVer = a_pstMeta->iBaseVersion;

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_VERSIONINDICATOR);    
    if( NULL == pstAttr )
    {
        return TDR_SUCCESS;  
    }
    
    pszVersion = scew_attribute_value(pstAttr);
	iRet = tdr_parse_meta_versionindicator_i(a_pstMeta, pszVersion);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的versionindicator属性值<%s>无效\n",
			a_pstMeta->szName, pszVersion);
		if ((int)(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE)) == iRet)
		{
			fprintf(a_fpError, "根据其属性值无法定位到指定元素,请确定指定正确的元素数据路径，且路径中涉及的各元素不是指针或引用类型.\n");
		}
		if ((int)(TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE)) == iRet)
		{
			fprintf(a_fpError, "其指向成员的数据类型只能为<char-uint>,且此元素不能为数组,此成员前面的成员存储空间必须是固定的\n");
		}
		iRet = TDR_ERRIMPLE_META_INVALID_VERSIONINDICATOR_VALUE;
	}     
    

    return iRet;
}

int tdr_get_struct_meta_packsize_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError, const char *a_pszAttr)
{
    int iRet = TDR_SUCCESS;
    scew_attribute *pstAttr = NULL;
    const char *pszSizeType = NULL;
    
    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);
	assert(NULL != a_pszAttr);

    pstAttr = scew_attribute_by_name(a_pstElement, a_pszAttr);
    if (NULL == pstAttr)
    {
        /*没有指定sizetype属性*/
        return TDR_SUCCESS;
    }

    pszSizeType = scew_attribute_value(pstAttr);
	iRet = tdr_sizeinfo_to_off_i(&a_pstMeta->stSizeType, a_pstMeta, TDR_INVALID_INDEX, pszSizeType);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的sizetype属性值<%s>无效,其属性值只能为<tinychar-uint>或当前元素的子元素成员的Path\n",
			a_pstMeta->szName, pszSizeType);
	}      
    

    return iRet;
}




int tdr_check_meta_i(LPTDRMETA a_pstMeta, FILE *a_fpError)
{
    int iRet = 0;
    int i = 0;
    TDRBOOLEAN bDyn = TDR_FALSE; 
    LPTDRMETALIB pstLib = NULL;
	int iPadding;

    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);

    pstLib = TDR_META_TO_LIB(a_pstMeta);

	/*检查自定义空间*/
	if ((0 < a_pstMeta->iCustomHUnitSize) && (a_pstMeta->iCustomHUnitSize < a_pstMeta->iHUnitSize))
	{
		fprintf(a_fpError, "error:\t 自定义类型<name = %s>指定的存储空间<%d>不能比实际需要的存储空间<%d>小\n",
			a_pstMeta->szName, a_pstMeta->iCustomHUnitSize, a_pstMeta->iHUnitSize);
		return TDR_ERRIMPLE_META_INVALID_SIZE_VALUE;      
	}


	/*结构体填充与对齐*/
	if ((0 < a_pstMeta->iCustomHUnitSize) && (a_pstMeta->iHUnitSize < a_pstMeta->iCustomHUnitSize))
	{
		iPadding = a_pstMeta->iCustomHUnitSize - a_pstMeta->iHUnitSize;
		a_pstMeta->iHUnitSize += iPadding;
	}
	a_pstMeta->iValidAlign = TDR_MIN(a_pstMeta->iValidAlign, a_pstMeta->iCustomAlign);
	if (0 >= a_pstMeta->iValidAlign)
	{		         
		fprintf(a_fpError, "error:\t 自定义类型<name = %s>的有效字节对齐值<%d>不正确, 请检查align属性\n",
			a_pstMeta->szName, a_pstMeta->iCustomHUnitSize);
		return TDR_ERRIMPLE_META_INVALID_ALIGN_VALUE;       
	}	
	if ((0 < a_pstMeta->iValidAlign ) && (0 != (a_pstMeta->iHUnitSize % a_pstMeta->iValidAlign)))
	{
		/*没有对齐,填充字节*/
		iPadding = (a_pstMeta->iValidAlign - (a_pstMeta->iHUnitSize % a_pstMeta->iValidAlign));	
		a_pstMeta->iHUnitSize += iPadding;
	}/*if ((0 < iEntryValidAlign ) && (0 != (iCurHOff % iEntryValidAlign))*/
    

    /*结构成员检查*/
	for (i = 0; i < a_pstMeta->iEntriesNum; i++)
	{
		LPTDRMETAENTRY pstEntry = a_pstMeta->stEntries + i;        

		/*检查成员为可变数组或者没有指定size的string类型成员的情况*/
		if ((0 == pstEntry->iCount ) || 
			(((TDR_TYPE_WSTRING == pstEntry->iType) || (TDR_TYPE_STRING == pstEntry->iType)) && (0 >= pstEntry->iCustomHUnitSize)))
		{
			if( (TDR_TYPE_UNION != a_pstMeta->iType) && (i+1 != a_pstMeta->iEntriesNum) )
			{
				fprintf(a_fpError,"error:\t 非union类型<name = %s>只能最后一个成员<%d>为可变数组或没有指定size的string类型,但第%d个成员<name = %s>就不符合此约束\n",
					a_pstMeta->szName, a_pstMeta->iEntriesNum, i+1, pstEntry->szName);

				return TDR_ERRIMPLE_ENTRY_INVALID_COUNT_VALUE;
			}/* only the last can be variable. */

			TDR_META_SET_VARIABLE(a_pstMeta);
		}

		/*检查成员为自定义数据类型*/
		if (TDR_INVALID_INDEX != pstEntry->ptrMeta)
		{
			LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);

			/*meta的sizetype和sizeinfo不能同时指定*/
			if ((0 < pstTypeMeta->stSizeType.iUnitSize) && ( 0 < pstEntry->stSizeInfo.iUnitSize))
			{
				fprintf(a_fpError,"error:\t 自定义类型<name = %s>的第%d个成员<name = %s>指定sizeinfo/target属性，但同时此成员指向的结构又指定了sizetype属性，目前不支持这种情况\n",
					a_pstMeta->szName, i+1, pstEntry->szName);
				return TDR_ERRIMPLE_META_INVALID_SIZETYPE_VALUE;
			}

			if (TDR_META_IS_VARIABLE(pstTypeMeta))
			{
				if (( i+1 != a_pstMeta->iEntriesNum) && (TDR_TYPE_UNION != a_pstMeta->iType))
				{
					fprintf(a_fpError,"error:\t 自定义类型<name = %s>的第%d个成员<name = %s>存储空间是可变的，但此成员不是最后一个成员<%d>\n",
						a_pstMeta->szName, i+1, pstEntry->szName,  a_pstMeta->iEntriesNum);
					return TDR_ERRIMPLE_ENTRY_INVALID_COUNT_VALUE;
				}

				TDR_META_SET_VARIABLE(a_pstMeta);					
			}				                                            
		}/*if (TDR_INVALID_INDEX != pstEntry->ptrMeta)*/      

		/*结构体存储空间是否固定*/
		if (!TDR_ENTRY_IS_FIXSIZE(pstEntry))
		{
			bDyn = TDR_TRUE;
		}        
	}/*for (i = 0; i < a_pstMeta->iEntriesNum; i++)*/
	if (TDR_FALSE == bDyn)
	{
		TDR_META_SET_FIXSIZE(a_pstMeta);
	}
    
	/*union结构检查*/
	if (TDR_TYPE_UNION == a_pstMeta->iType)
	{
		/*union结构中不指定id的成员只能有一个*/
		int iNoIdEntris = 0;
		for (i = 0; i < a_pstMeta->iEntriesNum; i++)
		{
			LPTDRMETAENTRY pstEntry = a_pstMeta->stEntries + i;
			if (!TDR_ENTRY_IS_VALID_SELECTID(pstEntry))
			{
				iNoIdEntris++;
			}
		}/*for (i = 0; i < a_pstMeta->iEntriesNum; i++)*/
		if (1 < iNoIdEntris)
		{
			fprintf(a_fpError, "error:\t union类型<name = %s>的子元素只能有且仅有一个可以没有id/(maxid,minid)属性\n",
				a_pstMeta->szName);

			return TDR_ERRIMPLE_ENTRY_INVALID_MAXMINID_VALUE;
		}
	} /*if (TDR_TYPE_UNION == a_pstMeta->iType)*/  


	/*检查与DB相关的属性*/
	if (TDR_INVALID_PTR != a_pstMeta->ptrDependonStruct)
	{
		iRet = tdr_check_depend_meta_dbattributes_i(a_pstMeta, a_fpError);
	}else
	{
		iRet = tdr_check_table_meta_dbattributes_i(a_pstMeta, a_fpError);
	}	
    
    TDR_META_SET_RESOLVED(a_pstMeta);    
    
    return iRet;
}





int tdr_parse_meta_versionindicator_i(LPTDRMETA a_pstMeta, const char* a_pszName)
{
	TDRMETALIB* pstLib;
	TDRMETAENTRY* pstEntry = NULL;
	LPTDRMETA pstSearchMeta;
	int iRet = TDR_SUCCESS;
	TDRBOOLEAN bFirstNode = TDR_FALSE;

	char szBuff[TDR_NAME_LEN];
	const char* pszPtr;
	int idx;
	int iNOff;
	int iHOff;
	int iUnit;
	int i;

	assert( NULL != a_pstMeta);
	assert(NULL != a_pszName);

	pstLib = TDR_META_TO_LIB(a_pstMeta);    
	pszPtr = a_pszName;

	/*如果是meta的sizeinfo属性,先跳过'this'*/
	pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
	if( 0 != tdr_stricmp(szBuff, TDR_TAG_THIS) )
	{
		pszPtr = a_pszName;
	}
	

	iNOff = 0;
	iHOff = 0;
	iUnit = 0; 
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

		idx	= tdr_get_entry_by_name_i(pstSearchMeta->stEntries, pstSearchMeta->iEntriesNum, szBuff);
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


		pstEntry = pstSearchMeta->stEntries + idx;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SIZETYPE_VALUE);
			break;
		}

		if (TDR_FALSE == bFirstNode)
		{
			a_pstMeta->iVersionIndicatorMinVer = pstEntry->iVersion;
			bFirstNode = TDR_TRUE;
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


	a_pstMeta->stVersionIndicator.iHOff	=	iHOff;
	a_pstMeta->stVersionIndicator.iNOff = iNOff;
	a_pstMeta->stVersionIndicator.iUnitSize	=	iUnit;

	return TDR_SUCCESS;
}



int tdr_get_struct_meta_splittablefactor_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_attribute *pstAttr = NULL;
	char szFactor[TDR_MACRO_LEN] = {0};
	LPTDRMETALIB pstLib;


	assert(NULL != a_pstMeta);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SPLITTABLEFACTOR);
	if (NULL == pstAttr)
	{
		/*没有指定sizetype属性*/
		return TDR_SUCCESS;
	}	

	tdr_normalize_string(szFactor, sizeof(szFactor), scew_attribute_value(pstAttr));
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	iRet = tdr_get_macro_int_i(&a_pstMeta->iSplitTableFactor, &a_pstMeta->idxSplitTableFactor, pstLib, szFactor);
	if ((TDR_ERR_IS_ERROR(iRet)) || (0 > a_pstMeta->iSplitTableFactor))
	{
		fprintf(a_fpError, "error:\t 元素<name = %s>的splittablefacotr属性值<%s>无效,请确定其值是正整数或已定义的宏\n",
			a_pstMeta->szName, szFactor);

		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SPLITTABLEFACTOR);
	}  

	return iRet;
}

int tdr_get_struct_meta_primarykey_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_attribute *pstAttr = NULL;
	LPTDRMETALIB pstLib;
	char *pszKey = NULL;
	char *pchDot;
	char *pchBegin;
	LPTDRMETAENTRY pstEntry;
	TDRSelector stSelector;
	LPTDRDBKEYINFO pstDBKey;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_PRIMARY_KEY);
	if (NULL == pstAttr)
	{
		/*没有指定sizetype属性*/
		return TDR_SUCCESS;
	}

	pstLib = TDR_META_TO_LIB(a_pstMeta);	
	pstDBKey = TDR_GET_PRIMARYBASEPTR(a_pstMeta);

	pszKey = strdup(scew_attribute_value(pstAttr));	
	pchBegin = pszKey;
	
	while (1)
	{
		pchDot = strchr(pchBegin, ',');
		if (NULL != pchDot)
		{
			*pchDot = '\0';
		}
		tdr_trim_str(pchBegin);
		if (*pchBegin == '\0') 
		{
			if (NULL == pchDot)
			{
				break;
			}else
			{
				pchDot++;
				pchBegin = pchDot;
				continue;
			}
		}/*if (*pchBegin == '\0') */

		iRet = tdr_parse_entryoff_db_i(a_pstMeta, &stSelector, TDR_TAG_PRIMARY_KEY, pchBegin, a_fpError);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			if (iRet != (int)TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_EXTENABLE_ARRAY))
			{
				fprintf(a_fpError, "error:\t 结构<name = %s>的%s属性值<%s>无效,请确定其值是用\',\'分隔开的子成员名.\n",
					a_pstMeta->szName, TDR_TAG_PRIMARY_KEY, pchBegin);
			}
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_PRIMARYKEY);
			break;
		}

		pstEntry = TDR_PTR_TO_ENTRY(pstLib, stSelector.ptrEntry);
		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{
			fprintf(a_fpError, "error:\t 结构<name = %s>的primarykey属性值<%s>无效,目前不支持使用复合类型成员作主键\n",
				a_pstMeta->szName, pszKey);
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_PRIMARYKEY);
			break;
		}
		if ((1 < pstEntry->iCount) && 
			((TDR_TYPE_CHAR > pstEntry->iType) || (TDR_TYPE_BYTE < pstEntry->iType)))
		{
			fprintf(a_fpError, "error:\t 结构<name = %s>的primarykey属性值<%s>无效,除char，byte数组外，目前不支持其他类型的数组成员作为主键.\n",
				a_pstMeta->szName, pszKey);
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_PRIMARYKEY);
			break;
		}

		if ((((char *)pstDBKey - (char*)a_pstMeta) + (int)sizeof(TDRDBKEYINFO))> a_pstMeta->iMemSize)
		{
			fprintf(a_fpError, "error:\t 存储结构<name = %s>的primarykey属性值<%s>时剩余空间不够,目前已存储了%d个主键元素.\n",
				a_pstMeta->szName, pszKey, a_pstMeta->nPrimayKeyMemberNum);

			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_PRIMARYKEY);
			break;
		}
		a_pstMeta->nPrimayKeyMemberNum++;
		pstDBKey->ptrEntry = stSelector.ptrEntry;
		pstDBKey->iHOff = stSelector.iHOff;
		TDR_ENTRY_SET_PRIMARYKEY(pstEntry);
		pstDBKey++;
		
		if (NULL == pchDot)
		{
			break;
		}
		pchDot++;
		pchBegin = pchDot;
	}/*while ('\0' != *pchBegin)*/

	free(pszKey);

	return iRet;
}

int tdr_parse_entryoff_db_i(IN LPTDRMETA a_pstMeta, OUT LPTDRSelector a_pstSelector, IN const char *a_pszAttr,
							IN const char* a_pszName, IN FILE *a_fpError)
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
	assert(NULL != a_pstSelector);
	assert(NULL != a_pszAttr);


	pstLib = TDR_META_TO_LIB(a_pstMeta); 


	/*meta属性跳过this*/
	pszPtr = a_pszName;
	pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
	if( 0 != tdr_stricmp(szBuff, TDR_TAG_THIS))
	{
		pszPtr = a_pszName;
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
		iIdx = tdr_get_entry_by_name_i(pstSearchMeta->stEntries, pstSearchMeta->iEntriesNum, szBuff);  
		if( TDR_INVALID_INDEX == iIdx )
		{
			break;
		}
            
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
			if (!TDR_ENTRY_DO_EXTENDABLE(pstEntry)  || (1 < pstEntry->iCount))
			{
				fprintf(a_fpError, "error:\t 存储结构<name = %s>的属性<%s=%s>目前只支持定义在可扩展的结构体非数组成员之中,而成员<name=%s>不符合此此约定.\n",
					a_pstMeta->szName, a_pszAttr, a_pszName, pstEntry->szName);

				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_EXTENABLE_ARRAY);
			}
		}else
		{
			pstSearchMeta	=	NULL;	
		}/*if( TDR_INVALID_PTR != pstEntry->ptrMeta )*/

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

int tdr_get_struct_meta_splittablekey_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_attribute *pstAttr = NULL;
	LPTDRMETALIB pstLib;
	LPTDRMETAENTRY pstEntry;
	TDRSelector stSelector;
	const char *pszKey = NULL;


	assert(NULL != a_pstMeta);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SPLITTABLEKEY);
	if (NULL == pstAttr)
	{
		/*没有指定sizetype属性*/
		return TDR_SUCCESS;
	}

	pstLib = TDR_META_TO_LIB(a_pstMeta);	
	pszKey = scew_attribute_value(pstAttr);	
	iRet = tdr_parse_entryoff_db_i(a_pstMeta, &stSelector, TDR_TAG_SPLITTABLEKEY, pszKey, a_fpError);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}

	pstEntry = TDR_PTR_TO_ENTRY(pstLib, stSelector.ptrEntry);
	if ( TDR_TYPE_COMPOSITE >= pstEntry->iType)
	{
		if (iRet != (int)TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_EXTENABLE_ARRAY))
		{
			fprintf(a_fpError, "error:\t 结构<name = %s>的splittablekey属性值<%s>无效,请确定其值是当前结构的子元素且其类型只能是基本数据类型.\n",
				a_pstMeta->szName, pszKey);
		}

		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SPLITTABLEFACTOR);
	}  

	a_pstMeta->stSplitTableKey.ptrEntry = stSelector.ptrEntry;
	a_pstMeta->stSplitTableKey.iHOff = stSelector.iHOff;

	return iRet;
}

int tdr_get_struct_meta_splittablerule_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_attribute *pstAttr = NULL;
	LPTDRMETALIB pstLib;
	char szRule[256] = {0};


	assert(NULL != a_pstMeta);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SPLITTABLERULE);
	if (NULL == pstAttr)
	{
		/*没有指定sizetype属性*/
		return TDR_SUCCESS;
	}

	pstLib = TDR_META_TO_LIB(a_pstMeta);	
	tdr_normalize_string(&szRule[0], sizeof(szRule), scew_attribute_value(pstAttr));	
	if (0 == tdr_stricmp(szRule, TDR_TAG_SPLITTABLERULE_BY_MOD))
	{
		a_pstMeta->nSplitTableRuleID = TDR_SPLITTABLE_RULE_BY_MOD;
	}else
	{
		fprintf(a_fpError, "error:\t struct元素<name = %s>的splittablerule属性值<%s>无效,目前只支持\"%s\"规则\n",
			a_pstMeta->szName, szRule, TDR_TAG_SPLITTABLERULE_BY_MOD);

		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SPLITTABLERULE);
	}


	return iRet;
}


int tdr_check_table_meta_dbattributes_i(IN LPTDRMETA a_pstMeta, IN FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	int i;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETA pstTypeMeta;
	LPTDRMETALIB pstLib;

	assert(NULL != a_pstMeta);
	assert(NULL != a_fpError);

	pstLib = TDR_META_TO_LIB(a_pstMeta);

	/*检查建表时主键属性的成员是否设置为NotNULL，如果没有强制设置为NOTNULL*/
	if (0 <= a_pstMeta->nPrimayKeyMemberNum)
	{
		LPTDRDBKEYINFO pstKey = TDR_GET_PRIMARYBASEPTR(a_pstMeta);

		for (i = 0; i < a_pstMeta->nPrimayKeyMemberNum; i++)
		{
			pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
			if (!TDR_ENTRY_IS_NOT_NULL(pstEntry))
			{
				TDR_ENTRY_SET_NOT_NULL(pstEntry);
			}
		}
	}/*if (0 <= a_pstMeta->nPrimayKeyMemberNum)*/

	/*检查建表时分表属性是否有效*/
	if (0 < a_pstMeta->iSplitTableFactor)
	{
		/*需要分表，则必须指定分表关键字*/
		if ((TDR_INVALID_OFFSET == a_pstMeta->stSplitTableKey.iHOff) || 
			(TDR_INVALID_PTR == a_pstMeta->stSplitTableKey.ptrEntry))
		{
			fprintf(a_fpError, "Error:\t 结构<name=%s>生成建表语句时需分成%d个数据库表进行存储，但没有指定splittablekey属性，即进行分表映射的成员。\n",
				a_pstMeta->szName, a_pstMeta->iSplitTableFactor);
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SPLITTABLEKEY);
		}

		/*没有指定映射规则时，缺省使用“分表因子模”规则*/
		if (TDR_SPLITTABLE_RULE_NONE == a_pstMeta->nSplitTableRuleID)
		{
			a_pstMeta->nSplitTableRuleID = TDR_SPLITTABLE_RULE_BY_MOD;
		}

		/*“分表因子模”规则,分表主键只能是整数类型的成员*/
		if (TDR_SPLITTABLE_RULE_BY_MOD == a_pstMeta->nSplitTableRuleID)
		{
			pstEntry = TDR_PTR_TO_ENTRY(pstLib, a_pstMeta->stSplitTableKey.ptrEntry);
			if ((TDR_TYPE_CHAR > pstEntry->iType) || (TDR_TYPE_WSTRING < pstEntry->iType))
			{
				fprintf(a_fpError, "Error:\t 结构<name=%s>生成建表语句时采用\"分表因子模\"规则，其分表主键<%s>只能为除void以外的基本数据类型\n",
					a_pstMeta->szName, pstEntry->szName);
				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SPLITTABLEKEY);
			}
			if (((TDR_TYPE_STRING == pstEntry->iType) || (TDR_TYPE_WSTRING == pstEntry->iType)) && (0 >= pstEntry->iCustomHUnitSize))
			{
				fprintf(a_fpError, "Error:\t 结构<name=%s>生成建表语句时采用\"分表因子模\"规则，其分表主键<%s>为string类型,此主键的存储空间必须是固定的\n",
					a_pstMeta->szName, pstEntry->szName);
				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_SPLITTABLEKEY);
			}
		}
	}/*if (0 < a_pstMeta->iSplitTableFactor)*/
	

	/*检查建表时struct成员扩展的合法性，目前只对指定了主键属性
	且uniqueentryname属性为true的meta结构进行检查*/	
	if ((0 <= a_pstMeta->nPrimayKeyMemberNum) && !(TDR_META_DO_NEED_PREFIX(a_pstMeta)))
	{
		char szConflictName[TDR_NAME_LEN * TDR_STACK_SIZE] = {0};
		

		for (i = 0; i < a_pstMeta->iEntriesNum; i++)
		{
			pstEntry = a_pstMeta->stEntries + i;
			if (!TDR_ENTRY_DO_EXTENDABLE(pstEntry))
			{
				continue;
			}
			if ((1 < pstEntry->iCount) || (0 == pstEntry->iCount))
			{
				fprintf(a_fpError, "Error:\t 结构体<name=%s>成员<name=%s>为复合数据类型的数组成员，展开存储时会出现两个相同名字的成员。\n",
					a_pstMeta->szName, pstEntry->szName);
				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NAME_CONFLICT_WHEN_EXTEND);
			}

			pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			iRet = tdr_check_meta_extendable_to_table_i(a_pstMeta, i - 1, pstTypeMeta, szConflictName);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				fprintf(a_fpError, "Error:\t 按照元数据描述，结构<name=%s>的成员<name=%s>将展开来生成建表语句，但展开后会出现两个相同名字<%s>的成员。\n",
					a_pstMeta->szName, pstEntry->szName, szConflictName);
				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NAME_CONFLICT_WHEN_EXTEND);
			}
		}/*for (i = 0; i < a_pstMeta->iEntriesNum; i++)*/		
	}/*if (TDR_INVALID_PTR != a_pstMeta->ptrPrimayKeyBase)*/

	

	/*联合主键不容许其成员为unique*/
	if (1 < a_pstMeta->nPrimayKeyMemberNum)
	{
		LPTDRDBKEYINFO pstKey = TDR_GET_PRIMARYBASEPTR(a_pstMeta);
		for (i = 0; i < a_pstMeta->nPrimayKeyMemberNum; i++)
		{
			pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
			if (TDR_ENTRY_IS_UNIQUE(pstEntry))
			{
				TDR_ENTRY_SET_NOT_NULL(pstEntry);
				fprintf(a_fpError, "Error:\t 结构<name=%s>的成员<name=%s>设置了<%s>属性，这与此结构对应的数据库表由多列数据<%d>组成联合主键的特性冲突。\n",
					a_pstMeta->szName, pstEntry->szName, TDR_TAG_UNIQUE, a_pstMeta->nPrimayKeyMemberNum);
				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_PRIMARYKEY);
			}
		}
	}/*if (1 < a_pstMeta->nPrimayKeyMemberNum)*/

	/*检查autoincrement属性的唯一性*/
	for (i = 0; i < a_pstMeta->iEntriesNum; i++)
	{
		pstEntry = a_pstMeta->stEntries + i;
		if (!TDR_ENTRY_DO_EXTENDABLE(pstEntry))
		{
			continue;
		}

		assert(TDR_INVALID_PTR != pstEntry->ptrMeta);
		pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
		if (!TDR_META_DO_HAVE_AUTOINCREMENT_ENTRY(pstTypeMeta))
		{
			continue;
		}

		if (1 != pstEntry->iCount)
		{
			fprintf(a_fpError, "\nerror:\t 结构体<name=%s>的成员<name = %s>的%s属性无效,只有非数组成员才能定义此属性\n",
				a_pstMeta->szName, pstEntry->szName, TDR_TAG_AUTOINCREMENT);

			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_AUTOINCREMENT);
		}
		if (TDR_META_DO_HAVE_AUTOINCREMENT_ENTRY(a_pstMeta))
		{
			fprintf(a_fpError, "\nerror:\t 结构体中只能有一个成员定义%s属性，结构体<name=%s>中已经包含了定义%s属性的成员，成员<name = %s>不能再定义此属性\n",
				TDR_TAG_AUTOINCREMENT, a_pstMeta->szName, TDR_TAG_AUTOINCREMENT, pstEntry->szName);

			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_AUTOINCREMENT);
		}else
		{
			TDR_META_SET_HAVE_AUTOINCREMENT_ENTRY(a_pstMeta);
		}		
	}/*for (i = 0; i < a_pstMeta->iEntriesNum; i++)*/
	

	return iRet;
}

int tdr_check_depend_meta_dbattributes_i(IN LPTDRMETA a_pstMeta, IN FILE *a_fpError)
{
	LPTDRMETA pstDependOnMeta;
	LPTDRMETALIB pstLib;
	int i;
	int iRet = TDR_SUCCESS;
	LPTDRDBKEYINFO pstKey;
	LPTDRDBKEYINFO pstMetaKey;
	LPTDRDBKEYINFO pstDependMetaKeyBase;
	LPTDRMETAENTRY pstEntry;

	assert(NULL != a_pstMeta);
	assert(NULL != a_fpError);
	assert(TDR_INVALID_PTR != a_pstMeta->ptrDependonStruct);


	/*定位本结构中的主键信息只能为依赖结构中定义的成员*/
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstDependOnMeta = TDR_PTR_TO_META(pstLib, a_pstMeta->ptrDependonStruct);	
	pstMetaKey = TDR_GET_PRIMARYBASEPTR(a_pstMeta);
	pstDependMetaKeyBase = TDR_GET_PRIMARYBASEPTR(pstDependOnMeta);

	/*先清除本结构定义的主键信息*/
	for (i = 0; i < pstDependOnMeta->nPrimayKeyMemberNum; i++)
	{
		pstKey = pstDependMetaKeyBase + i;
		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
		TDR_ENTRY_CLR_PRIMARYKEY(pstEntry);
	}/*for (i = 0; i < pstDependOnMeta->nPrimayKeyMemberNum; i++)*/


	/*将依赖结构中定义的成员,加到本结构中*/
	if (((char *)pstMetaKey - (char *)a_pstMeta ) > (a_pstMeta->iMemSize - (int)(sizeof(TDRDBKEYINFO)*pstDependOnMeta->nPrimayKeyMemberNum)))
	{
		fprintf(a_fpError, "error:\t 结构体<name = %s>的剩余空间不够存储%d个成员组成的主键信息.\n",
			a_pstMeta->szName, pstDependOnMeta->nPrimayKeyMemberNum);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_PRIMARYKEY);
	}
	a_pstMeta->nPrimayKeyMemberNum = 0;
	for (i = 0; i < pstDependOnMeta->nPrimayKeyMemberNum; i++)
	{
		pstKey = pstDependMetaKeyBase + i;
		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstKey->ptrEntry);
		if (TDR_ERR_IS_ERROR(tdr_find_tablecolumn_entry_i(pstMetaKey, a_pstMeta, pstEntry->szName)))
		{
			continue;
		}

		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstMetaKey->ptrEntry);
		TDR_ENTRY_SET_PRIMARYKEY(pstEntry);
		pstMetaKey++;		
		a_pstMeta->nPrimayKeyMemberNum++;
	}


	/*本结构中必须出现dependonstruct中定义的分表主键成员*/
	a_pstMeta->stSplitTableKey.iHOff = TDR_INVALID_OFFSET;
	a_pstMeta->stSplitTableKey.ptrEntry = TDR_INVALID_PTR;
	if (TDR_INVALID_PTR != pstDependOnMeta->stSplitTableKey.ptrEntry)
	{
		pstEntry = TDR_PTR_TO_ENTRY(pstLib, pstDependOnMeta->stSplitTableKey.ptrEntry);
		tdr_find_tablecolumn_entry_i(&a_pstMeta->stSplitTableKey, a_pstMeta, pstEntry->szName);		
	}

	return iRet;
}

int tdr_check_meta_extendable_to_table_i(LPTDRMETA a_pstBaseMeta, int iMaxIdx, LPTDRMETA a_pstCheckMeta, OUT char *a_pszConflictName)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	LPTDRMETAENTRY pstEntry;
	int iStackItemNum;
	LPTDRMETA pstCurMeta;
	int iChange;
	LPTDRMETALIB pstLib;

	assert(NULL != a_pstBaseMeta);
	assert(NULL != a_pstCheckMeta);

	if (0 > iMaxIdx)
	{
		return TDR_SUCCESS;
	}

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = a_pstCheckMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	iStackItemNum = 1;
	pstCurMeta = a_pstCheckMeta;
	pstLib = TDR_META_TO_LIB(a_pstBaseMeta);

	while (0 < iStackItemNum)
	{
		if (0 >= pstStackTop->iCount)
		{
			iStackItemNum--;
			if (0 < iStackItemNum)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
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
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		pstStackTop->pszHostBase = pstEntry->szName; 
		if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))
		{
			LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);

			/*检查此成员和当前结构成员是否重名*/
			iRet = tdr_check_meta_extendable_to_table_i(pstCurMeta, pstStackTop->idxEntry -1, pstTypeMeta, a_pszConflictName);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

			/*检查此成员和基本结构成员是否重名*/
			if (TDR_STACK_SIZE <=  iStackItemNum)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pstStackTop++;
			iStackItemNum++;
			pstCurMeta = pstTypeMeta;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = 1;
			pstStackTop->idxEntry = 0;
			continue;
		}/*if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))*/

		iRet = tdr_check_dbtable_name_conflict_i(a_pstBaseMeta, iMaxIdx, pstEntry->szName);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			int i;
			
			/*记录冲突名字路径*/
			strcat(a_pszConflictName, stStack[0].pszHostBase);
			for (i = 1; i < iStackItemNum; i++)
			{
				strcat(a_pszConflictName, ".");
				strcat(a_pszConflictName, stStack[i].pszHostBase);
			}
			break;
		}	
		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemNum)*/


	return iRet;
}

int tdr_check_dbtable_name_conflict_i(LPTDRMETA a_pstBaseMeta, int iMaxIdx, const char *pszName)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETA pstCurMeta;
	int iStackItemCount;
	LPTDRMETALIB pstLib;
	int iChange;

	assert(NULL != a_pstBaseMeta);
	assert(NULL != pszName);

	if (0 > iMaxIdx)
	{
		return TDR_SUCCESS;
	}

	pstStackTop = &stStack[0];
	pstCurMeta = a_pstBaseMeta;
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->idxEntry = 0;
	pstStackTop->iCount = 1;
	pstStackTop->iMetaSizeInfoUnit = iMaxIdx;  /*前iMaxIdx＋1个成员进行比较*/
	pstStackTop->iChange = 0;
	iStackItemCount = 1;
	pstLib = TDR_META_TO_LIB(a_pstBaseMeta);

	while (0 < iStackItemCount)
	{
		if ((0 >= pstStackTop->iCount) || (pstStackTop->idxEntry > pstStackTop->iMetaSizeInfoUnit))
		{
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
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
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))
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
			pstStackTop->iCount = 1;
			pstStackTop->idxEntry = 0;
			pstStackTop->iMetaSizeInfoUnit = pstCurMeta->iEntriesNum;
			pstStackTop->iChange = 0;
			continue;
		}/*if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))*/

		if (0 == strcmp(pstEntry->szName, pszName))
		{
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_NAME_CONFLICT);
			break;
		}	
		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/

	return iRet;
}

int tdr_get_struct_meta_strictinput_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;


	assert(NULL != a_pstMeta);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_STRICTINPUT);    
	if( NULL != pstAttr )
	{
		char szVal[256] = {0};

		tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstAttr));
		if (0 == tdr_stricmp(szVal, TDR_TAG_TRUE))
		{
			TDR_META_SET_STRICT_INPUT(a_pstMeta);
		}else
		{
			fprintf(a_fpError, "error:\t 结构体<name = %s>的strictinput属性值<%s>无效,其值为\"false\"或者\"true\"\n",
				a_pstMeta->szName, szVal);

			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_STRICTINPUT);
		}/*if ( 0== strcmp(pszVal, TDR_TAG_FALSE))*/
	}

	return iRet ;
}

/** 此
*/
int tdr_get_struct_meta_dependonstruct_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;
	char szName[TDR_NAME_LEN] = {0};
	LPTDRMETA pstDependOnMeta;
	LPTDRMETALIB pstLib;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	LPTDRMETAENTRY pstEntry;
	int iStackItemNum;
	LPTDRMETA pstCurMeta;
	int iChange;
	TDRDBKEYINFO stDBKeyInfo;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_DEPENDONSTRUCT);  
	if (NULL == pstAttr)
	{
		return TDR_SUCCESS;
	}

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	tdr_normalize_string(&szName[0], sizeof(szName), scew_attribute_value(pstAttr));
	pstDependOnMeta = tdr_get_meta_by_name_i(pstLib, szName);
	if ((NULL ==  pstDependOnMeta) || (TDR_TYPE_UNION == pstDependOnMeta->iType) ||
		(0 >= pstDependOnMeta->iNUnitSize))
	{
		fprintf(a_fpError, "error:\t 结构体<name = %s>的dependonstruc属性值<%s>无效,其值必须为之前已经定义struct元数据的名字。\n",
			a_pstMeta->szName, szName);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_DEPENDONSTRUCT);
	}
	a_pstMeta->ptrDependonStruct = pstDependOnMeta->ptrMeta;


	/*当前meta中定义的成员必须是dependonstruct元数据中定义的成员*/
	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = a_pstMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	iStackItemNum = 1;
	pstCurMeta = a_pstMeta;

	while (0 < iStackItemNum)
	{
		LPTDRMETAENTRY pstFindEntry;

		if (0 >= pstStackTop->iCount)
		{
			iStackItemNum--;
			if (0 < iStackItemNum)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
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
		if (TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}


		if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))
		{
			LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);

			/*检查此成员和基本结构成员是否重名*/
			if (TDR_STACK_SIZE <=  iStackItemNum)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}			

			pstStackTop++;
			iStackItemNum++;
			pstCurMeta = pstTypeMeta;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = 1;
			pstStackTop->idxEntry = 0;
			continue;
		}/*if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))*/

		iRet = tdr_find_tablecolumn_entry_i(&stDBKeyInfo, pstDependOnMeta, pstEntry->szName);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			fprintf(a_fpError, "error:\t 结构体<name = %s>的指定dependonstruc属性值<%s>，因此结构体<name = %s>中定义的成员必须是在结构体<name = %s>中定义的\n",
				a_pstMeta->szName, szName, a_pstMeta->szName, pstDependOnMeta->szName);
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_DEPENDONSTRUCT);
			break;
		}

		/*检查两个成员是否匹配*/
		pstFindEntry = TDR_PTR_TO_ENTRY(pstLib, stDBKeyInfo.ptrEntry);
		if (pstFindEntry->iCount != pstEntry->iCount)
		{
			fprintf(a_fpError, "error:\t 结构体<name = %s>指定了dependonstruc，其成员<name=%s>的count属性值<%d>必须与依赖结构体<name = %s>对应成员的count属性值<%d>相同\n",
				a_pstMeta->szName, pstEntry->szName, pstEntry->iCount, pstDependOnMeta->szName, pstFindEntry->iCount);
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_DEPENDONSTRUCT);
			break;
		}
		if (pstFindEntry->iHRealSize != pstEntry->iHRealSize)
		{
			fprintf(a_fpError, "error:\t 结构体<name = %s>指定了dependonstruc，其成员<name=%s>的存储空间<%d>必须与依赖结构体<name = %s>对应成员的存储空间<%d>相同\n",
				a_pstMeta->szName, pstEntry->szName, pstEntry->iHRealSize, pstDependOnMeta->szName, pstFindEntry->iHRealSize);
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_DEPENDONSTRUCT);
			break;
		}
		if ((pstFindEntry->iType != pstEntry->iType)|| (pstFindEntry->ptrMeta != pstEntry->ptrMeta))
		{
			fprintf(a_fpError, "error:\t 结构体<name = %s>指定了dependonstruc，其成员<name=%s>的数据类型<%d>必须与依赖结构体<name = %s>对应成员的数据类型<%d>相同\n",
				a_pstMeta->szName, pstEntry->szName, pstEntry->iType, pstDependOnMeta->szName, pstFindEntry->iType);
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_DEPENDONSTRUCT);
			break;
		}


		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemNum)*/
	
	
	return iRet;
}


int tdr_find_tablecolumn_entry_i(IN LPTDRDBKEYINFO a_pstDBKeyInfo, IN LPTDRMETA a_pstMeta, IN const char *a_pszName)
{
	int iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_META_INVALID_DEPENDONSTRUCT);
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETA pstCurMeta;
	int iStackItemCount;
	LPTDRMETALIB pstLib;
	int iChange;
	

	assert(NULL != a_pstDBKeyInfo);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pszName);

	pstStackTop = &stStack[0];
	pstCurMeta = a_pstMeta;
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->idxEntry = 0;
	pstStackTop->iCount = 1;
	iStackItemCount = 1;
	pstStackTop->iMetaSizeInfoOff = 0; /*meta的便宜*/
	pstLib = TDR_META_TO_LIB(pstCurMeta);


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
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
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

		if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))
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
			pstStackTop->iCount = 1;
			pstStackTop->idxEntry = 0;
			pstStackTop->iMetaSizeInfoOff = (pstStackTop-1)->iMetaSizeInfoOff + pstEntry->iHOff;
			continue;
		}/*if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))*/

		if (0 == strcmp(pstEntry->szName, a_pszName))
		{
			a_pstDBKeyInfo->iHOff = pstStackTop->iMetaSizeInfoOff + pstEntry->iHOff;
			a_pstDBKeyInfo->ptrEntry = (TDRPTR)((char *)pstEntry - pstLib->data);
			iRet = TDR_SUCCESS;
			break;
		}	
		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/

	return iRet;
}

int tdr_get_cname_attribute_i(TDRPTR *a_pptrName, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iStrLen = 0;
	int iRet = TDR_SUCCESS;

	assert(NULL != a_pptrName);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	*a_pptrName = TDR_INVALID_PTR;

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_CNNAME);    
	if( NULL != pstAttr )
	{
		const char *pszName = scew_attribute_value(pstAttr);

		assert(NULL != pszName);
		iStrLen = strlen(pszName) + 1; /*加1是为了保留结束字符*/

		/*太长将截断*/
		if (iStrLen > TDR_CHNAME_LEN)
		{
			iStrLen = TDR_CHNAME_LEN;
			fprintf(a_fpError, "\nwarning:\t <cname = %s>超过最大长度<%d>,将会截断",
				pszName, TDR_CHNAME_LEN); 
			iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_CNAME_BE_TRANCATED);
		}

		if ((1 < iStrLen) && (iStrLen > a_pstLib->iFreeStrBufSize))
		{
			fprintf(a_fpError, "\nerror:\t metalib字符串缓冲区剩余空间<freeSpace = %d>, 不能存储cname属性: %s",
				a_pstLib->iFreeStrBufSize, pszName); 

			iRet = TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
		}else
		{
			TDR_COPY_STRING_TO_BUF(*a_pptrName, pszName, iStrLen, a_pstLib);
		}
	}

	return iRet;
}

int tdr_get_name_attribute_i(char *pszName, int iNameSize, scew_element *pstElement)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;


	assert(NULL != pszName);
	assert(0 < iNameSize);
	assert(NULL != pstElement);


	/*获取此union的名字*/
	pstAttr = scew_attribute_by_name(pstElement, TDR_TAG_NAME); 
	if (NULL != pstAttr)
	{
		tdr_normalize_string(pszName, iNameSize, scew_attribute_value(pstAttr));
	}else
	{
		iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_NO_NAME);
	}

	return iRet;    
}

int tdr_get_id_attribute_i(int *a_piID, int *a_piIdx, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;

	assert(NULL != a_piID);
	assert(NULL != a_piIdx);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_ID);

	if( NULL != pstAttr )
	{
		char szID[1024] = {0};

		tdr_normalize_string(&szID[0], sizeof(szID), scew_attribute_value(pstAttr));
		iRet = tdr_get_macro_int_i(a_piID, a_piIdx, a_pstLib, &szID[0]);  
		if (TDR_ERR_IS_ERROR(iRet))
		{
			fprintf(a_fpError, "error:\t不正确的ID属性值<id = %s>,如果此属性值为宏，请确定此宏已经定义.\n", szID);
			iRet = TDR_ERRIMPLE_ENTRY_INVALID_ID_VALUE;
		}
	}else
	{
		iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_NO_ID);
	}

	return iRet;
}

int tdr_get_desc_attribute_i(TDRPTR *a_pptrDesc, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iStrLen = 0;
	int iRet = TDR_SUCCESS;

	assert(NULL != a_pptrDesc);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	*a_pptrDesc = TDR_INVALID_PTR;

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_DESCIPTION);    
	if( NULL != pstAttr )
	{
		const char *pszName = scew_attribute_value(pstAttr);

		assert(NULL != pszName);
		iStrLen = strlen(pszName) + 1; /*加1是为了保留结束字符*/

		/*太长将截断*/
		if (iStrLen > TDR_DESC_LEN)
		{
			iStrLen = TDR_DESC_LEN;
			fprintf(a_fpError, "\nwarning:\t <desc = %s>属性值超过最大长度<%d>,将会截断",
				pszName, TDR_DESC_LEN); 
			iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_DESC_VALUE_BE_TRANCATED);
		}

		if ((1 < iStrLen) && (iStrLen > a_pstLib->iFreeStrBufSize))
		{
			/*字符串缓冲区空闲空间不够*/
			fprintf(a_fpError, "\nerror:\t metalib字符串缓冲区剩余空间<freeSpace = %d>, 不能存储desc属性: %s",
				a_pstLib->iFreeStrBufSize, pszName); 

			iRet = TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
		}else
		{
			TDR_COPY_STRING_TO_BUF(*a_pptrDesc, pszName, iStrLen, a_pstLib);
		}
	}

	return iRet;
}


int tdr_get_version_attribute_i(int *a_piVersion, int *a_piIdxVersion, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;

	assert(NULL != a_piVersion);
	assert(NULL != a_piIdxVersion);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_VERSION);

	if( NULL != pstAttr )
	{
		char szVersion[1024] = {0};

		tdr_normalize_string(&szVersion[0], sizeof(szVersion), scew_attribute_value(pstAttr));

		iRet = tdr_get_macro_int_i(a_piVersion, a_piIdxVersion, a_pstLib, &szVersion[0]);  
		if (TDR_ERR_IS_ERROR(iRet))
		{
			fprintf(a_fpError, "\nerror:\t不正确的version属性值<version = %s>,如果此属性值为宏，请确定此宏已经定义.", szVersion);
		}
	}else
	{
		iRet =  TDR_SUCCEESS_WARNING(TDR_SUCWARN_NO_VERSION);
	}

	return iRet;
}

int tdr_get_meta_sortkey_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;

	LPTDRMETALIB pstLib =NULL;
	int iRet = TDR_SUCCESS;
  
	const char *pszPtr = NULL;
	char szBuff[TDR_NAME_LEN] = {0};


	assert(NULL != a_pstElement);
	assert(NULL != a_pstMeta);
	assert(NULL != a_fpError);


	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SORTKEY);    
	if( NULL == pstAttr )
	{
		return TDR_SUCCESS;  
	}	
	if (TDR_TYPE_UNION == a_pstMeta->iType)
	{
		fprintf(a_fpError, "\nwarnning:\t %s为union数据类型,目前不支持sortkey属性,将忽略此属性sortkey属性",
			a_pstMeta->szName);
		return TDR_SUCCESS;
	}

	pszPtr = scew_attribute_value(pstAttr);
	pszPtr = tdr_get_first_node_name_from_path_i(szBuff, sizeof(szBuff), pszPtr);
	tdr_trim_str(szBuff);
	if (0 != tdr_stricmp(szBuff, TDR_TAG_THIS)) 
	{
		pszPtr = scew_attribute_value(pstAttr);
	}


	/*根据sortkey的path信息分级定位sortkey在类型中的位置信息*/        
	iRet = tdr_parse_meta_sortkey_info_i(&a_pstMeta->stSortKey, a_pstMeta, pszPtr);    
	if (TDR_ERR_IS_ERROR(iRet))
	{
		fprintf(a_fpError, "\nerror:\t %s的sortkey属性值<%s>无效，其属性值只能是当前元素为根的子元素,请确定指定正确的元素数据路径，且路径中涉及的各元素不是指针或引用类型",
			a_pstMeta->szName, scew_attribute_value(pstAttr));
	}

	return iRet;
}

/** 将宏定义组加到元数据描述库中
*@param[in,out] a_pstLib 需要更新的元数据库
*@param[in] a_pstElement 宏定义组元素
*@param[in] a_fpError 用来记录处理错误信息的文件句柄
*
*@return 成功返回TDR_SUCCESS,否则返回错误号
*
*@pre \e a_pstLib 不能为NULL
*@pre \e a_pstElement 不能为NULL
*@pre \e a_fpError 不能为NULL
*/
int tdr_add_macrosgroup_i(IN TDRMETALIB* a_pstLib, IN scew_element *a_pstElement, IN FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	LPTDRMACROSGROUP pstMacrosGroup = NULL;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	/*检查是否有足够的存储空间保存此宏定义组*/
	iRet = tdr_create_macrosgroup_i(&pstMacrosGroup, a_pstLib, a_pstElement, a_fpError);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return iRet;
	}	


	/*读取宏定义组的基本信息*/
	iRet = tdr_get_macrosgroup_attributes(pstMacrosGroup, a_pstLib, a_pstElement, a_fpError);

	/*添加此宏定义组的宏,并记录到宏定义组结构中*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_add_macros_of_group_i(pstMacrosGroup, a_pstLib, a_pstElement, a_fpError);
	}

	/*正规化处理宏定义组数据结构：对索引区进行排序*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		tdr_sort_macrosgroup_nameidxmap_i(pstMacrosGroup, a_pstLib);
		tdr_sort_macrosgroup_valueidxmap_i(pstMacrosGroup, a_pstLib);

		a_pstLib->iCurMacrosGroupNum++;
	}	

	return iRet;
}

int  tdr_create_macrosgroup_i(INOUT LPTDRMACROSGROUP *a_ppstMacrosGroup, IN TDRMETALIB* a_pstLib, IN scew_element *a_pstElement, IN FILE *a_fpError)
{

	int iMacro = 0;
	int iGroupSize = 0;
	int iStrLen = 0;
	int iUsedSize;
	LPTDRMAPENTRY pstMap;
	LPTDRMACROSGROUP pstGroup;
	TDRIDX *pIndex;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	if (a_pstLib->iCurMacrosGroupNum >= a_pstLib->iMaxMacrosGroupNum)
	{
		fprintf(a_fpError, "Error:元数据库最多能存储%d个宏定义组，目前以达到这个数目，不能再添加宏定义组",
			a_pstLib->iMaxMacrosGroupNum);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_MACROGSROUP_ATTRIBUTE);
	}

	/*检查宏定义组剩余空间*/
	tdr_get_macrosgroup_size_i(&iMacro, &iGroupSize, &iStrLen, a_pstElement);
	iUsedSize = tdr_get_macrogroup_used_size_i(a_pstLib);
	if ((a_pstLib->iMacrosGroupSize - iUsedSize) < iGroupSize)
	{
		fprintf(a_fpError, "Error:元数据库用于存储宏定义组的剩余空间为%d字节，新的宏定义组需要%d个字节的存储空间",
			a_pstLib->iMacrosGroupSize - iUsedSize, iGroupSize);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_MACROGSROUP_ATTRIBUTE);
	}

	/*检查字符串缓冲的空间是否足够*/
	if (a_pstLib->iFreeStrBufSize < iStrLen)
	{
		fprintf(a_fpError, "\nerror:\t 处理宏定义组失败，元数据描述库字符串缓冲区剩余空间为%d字节，但实际需要%d个字节\n",
			a_pstLib->iFreeStrBufSize, iStrLen);

		return TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
	}

	/*建立索引*/
	pstMap = TDR_GET_MACROSGROUP_MAP_TABLE(a_pstLib);
	pstMap[a_pstLib->iCurMacrosGroupNum].iPtr = a_pstLib->ptrMacrosGroup + iUsedSize;
	pstMap[a_pstLib->iCurMacrosGroupNum].iSize = iGroupSize;

	pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, pstMap[a_pstLib->iCurMacrosGroupNum].iPtr);
	memset(pstGroup, 0, offsetof(TDRMACROSGROUP, data));
	pstGroup->ptrDesc = TDR_INVALID_PTR;
	pstGroup->iMaxMacroCount = iMacro;
	pstGroup->ptrNameIdxMap = offsetof(TDRMACROSGROUP, data);
	pstGroup->ptrValueIdxMap = pstGroup->ptrNameIdxMap + sizeof(int)*iMacro;
	pIndex = TDR_GET_MACROSGROUP_NAMEIDXMAP_TAB(pstGroup);
	memset(pIndex, TDR_INVALID_INDEX, sizeof(TDRIDX)*iMacro);
	pIndex = TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(pstGroup);
	memset(pIndex, TDR_INVALID_INDEX, sizeof(TDRIDX)*iMacro);
	*a_ppstMacrosGroup = pstGroup;

	return TDR_SUCCESS;
}

/*取宏定义组数据区已经使用的空间*/
int tdr_get_macrogroup_used_size_i(IN LPTDRMETALIB a_pstLib)
{
	int iSize = 0;
	int i;
	LPTDRMAPENTRY pstMap;

	assert(NULL != a_pstLib);

	pstMap = TDR_GET_MACROSGROUP_MAP_TABLE(a_pstLib);
	for (i =0; i < a_pstLib->iCurMacrosGroupNum; i++)
	{
		iSize += pstMap->iSize;
		pstMap++;
	}
	
	return iSize;
}

/** 获取macrosgroup元素的属性
*@return 成功返回0，否则返回非零值
*/
int tdr_get_macrosgroup_attributes(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib, IN scew_element *a_pstElement, IN FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;

	assert(NULL != a_pstMacrosGroup);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	
	/*获取name 属性*/
	iRet = tdr_get_name_attribute_i(&a_pstMacrosGroup->szName[0], sizeof(a_pstMacrosGroup->szName), a_pstElement);
	if (a_pstMacrosGroup->szName[0] == '\0') /*空串或没有定义*/
	{  
		fprintf(a_fpError, "error:\t 第%d个宏定义组<%s>元素没有定义name属性或其值为空串\n", 
			a_pstLib->iCurMacrosGroupNum + 1, TDR_TAG_MACROSGROUP);
		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_MACROGSROUP_ATTRIBUTE);;
	}else 
	{
		/*判断是否有重名的宏定义组存在*/  
		LPTDRMACROSGROUP pstTmpGroup = tdr_find_macrosgroup_i(a_pstLib, a_pstMacrosGroup->szName);
		if (NULL != pstTmpGroup)
		{
			fprintf(a_fpError, "error:\t 元数据描述库<%s>存在两个名字<name = %s>相同的宏定义组(%s)元素\n",
				a_pstLib->szName, a_pstMacrosGroup->szName, TDR_TAG_MACROSGROUP);
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_MACROGSROUP_ATTRIBUTE);;
		}
	}/*if (a_pstMacrosGroup->szName[0] == '\0')*/

	/*desc*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_get_desc_attribute_i(&a_pstMacrosGroup->ptrDesc, a_pstLib, a_pstElement, a_fpError);
	}	 

	return iRet;
}

/**根据宏定义组的名称查找此宏定义组数据结构
*/
LPTDRMACROSGROUP tdr_find_macrosgroup_i(IN const LPTDRMETALIB a_pstLib, IN const char *a_pszName)
{
	int i;
	LPTDRMAPENTRY pstMap;
	LPTDRMACROSGROUP pstTmpGroup = NULL;

	assert(NULL != a_pstLib);
	assert(NULL != a_pszName);

	pstMap = TDR_GET_MACROSGROUP_MAP_TABLE(a_pstLib);
	for (i = 0; i < a_pstLib->iCurMacrosGroupNum; i++)
	{
		pstTmpGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, pstMap[i].iPtr);
		if (0 == tdr_stricmp(pstTmpGroup->szName, a_pszName))
		{
			break;
		}
	}
	if (i >= a_pstLib->iCurMacrosGroupNum)
	{
		pstTmpGroup = NULL;
	}

	return pstTmpGroup;
}

/** 将macrosgroup元素的macro子元素加入到元数据库中
*@return 成功返回0，否则返回非零值
*/
int tdr_add_macros_of_group_i(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib, IN scew_element *a_pstElement, IN FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_element* pstItem = NULL;
	TDRIDX *pNameIdxTab;
	TDRIDX *pValueIdxTab;
	LPTDRMACRO pstMacro = NULL;
	LPTDRMACRO pstMacroTable = NULL;
	int i;

	assert(NULL != a_pstMacrosGroup);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstItem = scew_element_next(a_pstElement, NULL);  
	pNameIdxTab = TDR_GET_MACROSGROUP_NAMEIDXMAP_TAB(a_pstMacrosGroup);
	pValueIdxTab = TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(a_pstMacrosGroup);
	pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);    
	while (NULL != pstItem)
	{
		if (0 != tdr_stricmp(scew_element_name(pstItem), TDR_TAG_MACRO))
		{
			pstItem = scew_element_next(a_pstElement, pstItem);  
			continue;
		}

		/*检查空间是否足够*/
		if( a_pstLib->iCurMacroNum >= a_pstLib->iMaxMacroNum )
		{

			fprintf(a_fpError, "error:\t metalib最多能存储%d个宏定义，目前已存储%d个,不能再加入宏定义.\n",
				a_pstLib->iMaxMacroNum, a_pstLib->iCurMacroNum);
			iRet =  TDR_ERRIMPLE_TO_MUCH_MACROS;
			break;
		}
		if (a_pstMacrosGroup->iCurMacroCount >= a_pstMacrosGroup->iMaxMacroCount)
		{
			fprintf(a_fpError, "error:\t 宏定义组<name=%s>最多能存储%d个宏定义，目前已存储%d个,不能再加入宏定义.\n",
				a_pstMacrosGroup->szName, a_pstMacrosGroup->iMaxMacroCount, a_pstMacrosGroup->iCurMacroCount);
			iRet =  TDR_ERRIMPLE_TO_MUCH_MACROS;
			break;
		}

		/*将宏定义加入元数据描述库宏定义表中*/
		iRet = tdr_add_macro_i(a_pstLib, pstItem, a_fpError, &pstMacro);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}   

		/*检查宏定义组中是否有值相同的宏存在*/
		for (i = 0; i < a_pstMacrosGroup->iCurMacroCount; i++)
		{
			LPTDRMACRO pstTmpMacro = pstMacroTable + pValueIdxTab[i];
			if (pstTmpMacro->iValue == pstMacro->iValue)
			{
				fprintf(a_fpError, "error:\t 宏定义组<name=%s>中宏定义<name=%s>的值<value=%d>与宏定义<name=%s>的值相同\n",
					a_pstMacrosGroup->szName, pstTmpMacro->szMacro, pstTmpMacro->iValue, pstMacro->szMacro);
				iRet =  TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_MACROGSROUP_ATTRIBUTE);;
				break;
			}
		}/*for (i = 0; i < a_pstMacrosGroup->iCurMacroCount; i++)*/
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}

		/*宏定义组中记录此宏定义*/
		pNameIdxTab[a_pstMacrosGroup->iCurMacroCount] = pstMacro - pstMacroTable;
		pValueIdxTab[a_pstMacrosGroup->iCurMacroCount] = pNameIdxTab[a_pstMacrosGroup->iCurMacroCount];
		a_pstMacrosGroup->iCurMacroCount++;

		pstItem = scew_element_next(a_pstElement, pstItem);         
	}/*while (NULL != pstItem)*/    

	return iRet;
}

void tdr_sort_macrosgroup_nameidxmap_i(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib)
{
	int i,j;
	LPTDRMACRO pstMacro1 = NULL;
	LPTDRMACRO pstMacro2 = NULL;
	LPTDRMACRO pstMacroTable = NULL;
	TDRIDX *pNameIdxTab;
	TDRIDX idxTmp;


	assert(NULL != a_pstMacrosGroup);
	assert(NULL != a_pstLib);

	/*冒泡排序*/
	pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);  
	pNameIdxTab = TDR_GET_MACROSGROUP_NAMEIDXMAP_TAB(a_pstMacrosGroup);
	for (i = 0; i < a_pstMacrosGroup->iCurMacroCount -1; i++)
	{
		for (j = i+1; j < a_pstMacrosGroup->iCurMacroCount; j++)
		{
			pstMacro1 = pstMacroTable + pNameIdxTab[i];
			pstMacro2 = pstMacroTable + pNameIdxTab[j];
			if (0 < tdr_stricmp(pstMacro1->szMacro, pstMacro2->szMacro))
			{
				idxTmp = pNameIdxTab[i];
				pNameIdxTab[i] = pNameIdxTab[j];
				pNameIdxTab[j] = idxTmp;
			}
		}/*for (j = i+1; j < a_pstMacrosGroup->iCurMacroCount; j++)*/		
	}/*for (i = 0; i < a_pstMacrosGroup->iCurMacroCount -1; i++)*/
}

void tdr_sort_macrosgroup_valueidxmap_i(IN LPTDRMACROSGROUP a_pstMacrosGroup, IN TDRMETALIB* a_pstLib)
{
	int i,j;
	LPTDRMACRO pstMacro1 = NULL;
	LPTDRMACRO pstMacro2 = NULL;
	LPTDRMACRO pstMacroTable = NULL;
	TDRIDX *pValueIdxTab;
	TDRIDX idxTmp;

	assert(NULL != a_pstMacrosGroup);
	assert(NULL != a_pstLib);

	/*冒泡排序*/
	pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);  
	pValueIdxTab = TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(a_pstMacrosGroup);
	for (i = 0; i < a_pstMacrosGroup->iCurMacroCount -1; i++)
	{
		for (j = i+1; j < a_pstMacrosGroup->iCurMacroCount; j++)
		{
			pstMacro1 = pstMacroTable + pValueIdxTab[i];
			pstMacro2 = pstMacroTable + pValueIdxTab[j];
			if (pstMacro1->iValue > pstMacro2->iValue)
			{
				idxTmp = pValueIdxTab[i];
				pValueIdxTab[i] = pValueIdxTab[j];
				pValueIdxTab[j] = idxTmp;

			}
		}/*for (j = i+1; j < a_pstMacrosGroup->iCurMacroCount; j++)*/		
	}/*for (i = 0; i < a_pstMacrosGroup->iCurMacroCount -1; i++)*/
}

int tdr_get_struct_meta_uniqueentryname_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;

	LPTDRMETALIB pstLib =NULL;
	int iRet = TDR_SUCCESS;
	const char *pszPtr = NULL;
	char szVal[TDR_NAME_LEN] = {0};


	assert(NULL != a_pstElement);
	assert(NULL != a_pstMeta);
	assert(NULL != a_fpError);


	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_UNIQUEENTRYNAME);    
	if( NULL == pstAttr )
	{
		return TDR_SUCCESS;  
	}	
	if (TDR_TYPE_UNION == a_pstMeta->iType)
	{
		fprintf(a_fpError, "\nwarnning:\t 复合结构<name = %s>为union数组类型,目前不支持%s属性,此属性将忽略",
			a_pstMeta->szName, TDR_TAG_UNIQUEENTRYNAME);
		return TDR_SUCCESS;
	}

	pszPtr = scew_attribute_value(pstAttr);
	tdr_normalize_string(szVal, sizeof(szVal), pszPtr);
	if (0 == tdr_stricmp(szVal, TDR_TAG_TRUE))
	{
		TDR_META_CLR_NEED_PREFIX(a_pstMeta);
	}else if ( 0== tdr_stricmp(szVal, TDR_TAG_FALSE))
	{
		TDR_META_SET_NEED_PREFIX(a_pstMeta);
	}else
	{
		fprintf(a_fpError, "error:\t 结构体元素<name = %s>的%s属性值<%s>无效,其值为\"false\"或者\"true\"\n",
			a_pstMeta->szName, TDR_TAG_UNIQUEENTRYNAME, szVal);

		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_UNIQUEENTRYNAMEVALUE);
	}/*if ( 0== tdr_stricmp(pszVal, TDR_TAG_FALSE))*/

	return iRet;
}

