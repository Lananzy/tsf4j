/**
*
* @file     tdr_metaleb_param_i.c   
* @brief    元数据描述库参数获取
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-04-05 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <string.h>
#include <stdlib.h>
#include <assert.h>


#include "tdr/tdr_ctypes_info.h"
#include "tdr/tdr_os.h"

#include "tdr/tdr_define_i.h"
#include "tdr_metaleb_param_i.h"
#include "tdr_metalib_meta_manage_i.h"
#include "tdr/tdr_error_i.h"
#include "tdr_XMLtags_i.h"
#include "tdr/tdr_ctypes_info_i.h"
#include "tdr/tdr_auxtools.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif


/**根据元素名检查此元素是否是DR XML描述的根元素
*@pre \e 调用此方法之前，需调用tdr_get_metalib_root_attribte_i获取标签集的版本信息
*/
static int tdr_check_metalib_root_i(IN scew_element* pstRoot, IN int iTagsVersion, IN FILE *fpError);

/**从XML元素中获取 根元素的名字属性
*@note 如果pstParam中已经保存了名字，但pstRoot的名字与之不同，
* 则认为是两个不同的元数据描述，直接报错不继续处理
*@pre \e 调用此方法之前，需调用tdr_get_metalib_root_attribte_i获取标签集的版本信息
*/
static int tdr_metalib_get_root_name_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError);


/**获取metalib元素的ID属性*/
static int tdr_metalib_get_id_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot);


/*获取metalib根元素的属性*/
static int tdr_get_metalib_root_attribte_i(INOUT LPTDRLIBPARAM a_pstParam, IN scew_element *a_pstRoot, IN int a_iTagsVersion, IN FILE *a_fpError);

/*获取宏定义，结构体元素的数目，并计算原数据库的内存大小*/
static int tdr_get_metalib_size_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError);

static int tdr_get_metalib_ver0_size_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError);

static int tdr_get_metalib_ver1_size_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError);

/**返回缺省值所需的内存空间字节数 
*/
static int tdr_get_entry_defaultvalue_size_i(scew_element *a_pstItem);

/**返回description属性值所需的内存空间字节数 
*/
static int tdr_get_description_size_i(scew_element *a_pstItem);

/**返回cname属性值所需的内存空间字节数 
*/
static int tdr_get_cname_size_i(scew_element *a_pstItem);

static int tdr_get_meta_primaykey_size_i(scew_element *a_pstElement, scew_element *a_pstRoot);

static scew_attribute *tdr_get_primarykey_by_depended_strut_i(scew_element *a_pstElement, scew_element *a_pstRoot);

static int tdr_get_entry_customattr_size_i(scew_element *a_pstItem);


///////////////////////////////////////////////////////////////////

void tdr_init_lib_param_i(LPTDRLIBPARAM a_pstParam)
{
    assert(NULL != a_pstParam);

    memset(a_pstParam, 0, sizeof(TDRLIBPARAM));

    a_pstParam->iID = TDR_INVALID_ID;
    a_pstParam->lVersion = TDR_INVALID_VERSION;
    a_pstParam->iTagSetVersion = TDR_SUPPORTING_MAX_XMLTAGSET_VERSION;  /*缺省按最新版本进行解析*/
}


int tdr_get_lib_param_i(LPTDRLIBPARAM a_pstParam, IN scew_tree* a_pstTree, IN int a_iTagsVersion, IN FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;

    assert(NULL != a_pstParam);
    assert(NULL != a_pstTree);
    assert(NULL != a_fpError);
    
    /*assert((TDR_SUPPORTING_MIN_XMLTAGSET_VERSION <= a_iTagsVersion) &&
        (a_iTagsVersion <= TDR_SUPPORTING_MAX_XMLTAGSET_VERSION));*/

    tdr_init_lib_param_i(a_pstParam);

    iRet = tdr_add_lib_param_i(a_pstParam, a_pstTree, a_iTagsVersion, a_fpError);    
    if (TDR_ERR_IS_ERROR(iRet))
    {
        return iRet;
    }
    
    if ((a_pstParam->iMaxMacros <= 0) && (a_pstParam->iMaxMetas <= 0))
    {
        fprintf(a_fpError, "error:\t 此XML元素树中没有定义任何宏元素和union/struct元素\n");
        
        return TDR_ERRIMPLE_INVALID_METALIB_PARAM;
    }   
    
	
    return TDR_SUCCESS;
}

int tdr_add_lib_param_i(LPTDRLIBPARAM a_pstParam, IN scew_tree* a_pstTree, IN int a_iTagsVersion, IN FILE *a_fpError)
{
    int iRet = 0;
    scew_element* pstRoot;

    assert(NULL != a_pstParam);
    assert(NULL != a_pstTree);
    assert(NULL != a_fpError);
    /*assert((TDR_SUPPORTING_MIN_XMLTAGSET_VERSION <= a_iTagsVersion) &&
        (a_iTagsVersion <= TDR_SUPPORTING_MAX_XMLTAGSET_VERSION));*/


    /*取出根元素进行分析*/
    pstRoot = scew_tree_root( a_pstTree );
    if (NULL == pstRoot )
    {
        fprintf(a_fpError, "error:\t此XML元素树中没有根元素\n");
        
        return TDR_ERRIMPLE_NO_XML_ROOT;
    }
        

    /*获取根元素的属性值*/
    iRet = tdr_get_metalib_root_attribte_i(a_pstParam, pstRoot, a_iTagsVersion, a_fpError);
    

     /*检查a_pstTree的根元素是不是符合元数据描述语法*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_check_metalib_root_i(pstRoot, a_pstParam->iTagSetVersion, a_fpError);
    }

   
    /*获取metalib子孙元素信息，并计算元数据库需要的内存大小*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_metalib_size_i(a_pstParam, pstRoot, a_fpError);
    }  


    return iRet;

}

int tdr_get_metalib_root_attribte_i(INOUT LPTDRLIBPARAM a_pstParam, IN scew_element *a_pstRoot, IN int a_iTagsVersion, IN FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    
    /*获取metalib的tagsveriosn属性*/
    iRet = tdr_get_metalib_tagsversion_i(&a_pstParam->iTagSetVersion, a_pstRoot, a_iTagsVersion);
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        /*检查标签集版本*/
        if((TDR_SUPPORTING_MIN_XMLTAGSET_VERSION > a_pstParam->iTagSetVersion) ||
            (a_pstParam->iTagSetVersion > TDR_SUPPORTING_MAX_XMLTAGSET_VERSION))
        {
            fprintf(a_fpError, "error:\t不支持的元数据描述XML标签集版本<%d>，目前支持的最小版本号: %d, 最大版本号是: %d,", 
                    a_pstParam->iTagSetVersion, TDR_SUPPORTING_MIN_XMLTAGSET_VERSION, TDR_SUPPORTING_MAX_XMLTAGSET_VERSION);
            fprintf(a_fpError, "请检查传入的版本参数或XML信息根元素的tagsetversion属性.\n");               
            iRet = TDR_ERRIMPLE_INVALID_TAGSET_VERSION;
        }   
    }

    /*获取元数据库名字属性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_metalib_get_root_name_i(a_pstParam, a_pstRoot, a_fpError);
    }

    /*获取metalib的version属性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        long lVersion = TDR_INVALID_VERSION;

        iRet = tdr_get_metalib_version_i(&lVersion, a_pstRoot);
        if ((TDR_XML_TAGSET_VERSION_1 <= a_iTagsVersion) &&
            (TDR_ERR_IS_ERROR(iRet)))
        {
            /**从tagsversion1开始必须指定version属性*/
            fprintf(a_fpError, "Error:\t 从元数据描述XML标签集版本%d开始，根元素必须指定version属性。\n",
                TDR_XML_TAGSET_VERSION_1 );
            
            return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_VERSION);
        }else if (!TDR_ERR_IS_ERROR(iRet))
        {
            if (lVersion > a_pstParam->lVersion)
            {
                a_pstParam->lVersion = lVersion;
            }
        }
    }/*if (TDR_SUCCESS == iRet)*/

    /**获取metalib的ID属性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_metalib_get_id_i(a_pstParam, a_pstRoot);
    }    
    
    return iRet;
}

int tdr_check_metalib_root_i(IN scew_element* pstRoot, IN int iTagsVersion, IN FILE *fpError)
{
    const char *pszExpectedName = NULL;
    const char *pstTagName = NULL;

    assert(NULL != pstRoot);
    assert(NULL != fpError);

    assert((TDR_SUPPORTING_MIN_XMLTAGSET_VERSION <= iTagsVersion) &&
        (iTagsVersion <= TDR_SUPPORTING_MAX_XMLTAGSET_VERSION));
    

    TDR_GET_METALIB_ROOT_TAG(pszExpectedName, iTagsVersion);
    assert(NULL != pszExpectedName);

    pstTagName = scew_element_name(pstRoot);
    assert(NULL != pstTagName);
    
    /*如果符合标签集版本定义，则标签名应该相等*/
    if (0 != strcmp(pstTagName, pszExpectedName))
    {
        fprintf(fpError, "error:\tXML元素树的根元素标签不正确,按照元数据描述标签版本%d的定义: \n", iTagsVersion);
        fprintf(fpError, "正确的根元素标签应该是:%s ，而实际的根元素标签是: %s,请检查输入的元数据描述xml标签集版本是否正确\n", pszExpectedName, pstTagName);
        
        return TDR_ERRIMPLE_INVALID_XML_ROOT;
    }

    return TDR_SUCCESS;
}


int tdr_metalib_get_root_name_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError)
{
    scew_attribute *pstAttr = NULL;
    const char *pszName = NULL;

    
    assert(NULL != pstParam);
    assert(NULL != pstRoot);
    assert(NULL != fpError);

    pstAttr = scew_attribute_by_name(pstRoot, TDR_TAG_NAME);
    if ((NULL == pstAttr) && (TDR_XML_TAGSET_VERSION_1 <= pstParam->iTagSetVersion))
    {
        /**没有name属性*/
        fprintf(fpError, "error:\t 从元数据描述标签集版本%d开始，根元素必须指定name属性\n", TDR_XML_TAGSET_VERSION_1);
        
        return TDR_ERRIMPLE_METALIB_ROOT_NO_NAME;
    }

    pszName = scew_attribute_value(pstAttr);
	if ((pszName[0] == '\0'))
	{
		return TDR_SUCCESS;
	}


	if (pstParam->szName[0] == '\0')
    {
        /*参数体中还没有保存name属性*/
        TDR_STRNCPY(pstParam->szName, pszName, sizeof(pstParam->szName));
    }else if (0 != strcmp(pstParam->szName, pszName))
    {
        /*
        从之前的XML文件中已经读取了name属性，则此XML元素树必须有相同名字才能加到元素库中
        属性值区分大小写*/
        
        fprintf(fpError, "error:\t不能将不同名字空间的元数据描述加到同一个元数据库中, 请检查多个XML信息中根元素的name属性值是否相同:\n");
        fprintf(fpError, "\t\t已经处理了<name = %s>名字空间的元数据描述，现在要处理<name = %s>名字空间\n", pstParam->szName, pszName);
        
        return TDR_ERRIMPLE_NAMESPACE_CONFLICT;
    }/*if (0 != tdr_stricmp(pstParam->szName, pszName))*/
    

    return TDR_SUCCESS;
}


int tdr_metalib_get_id_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot)
{
    scew_attribute *pstAttr = NULL;
    int iId = TDR_INVALID_ID;

    
    assert(NULL != pstParam);
    assert(NULL != pstRoot);


    pstAttr = scew_attribute_by_name(pstRoot, TDR_TAG_ID);
    if (NULL == pstAttr)
    {
        /**没有ID属性*/
       return TDR_SUCCESS;
    }

    iId = atoi(scew_attribute_value(pstAttr));
	pstParam->iID = iId;
    
    return TDR_SUCCESS;
}


int tdr_get_metalib_tagsversion_i(OUT int *a_piTagVersion, IN scew_element *pstRoot, IN int iTagsVersion)
{
    scew_attribute *pstAttr = NULL;

    assert(NULL != a_piTagVersion);
    assert(NULL != pstRoot);
   

    pstAttr = scew_attribute_by_name(pstRoot, TDR_TAG_TAGSET_VERSION);
    if (NULL == pstAttr)
    {
        /*文件中没有指定 标签集版本则以参数中传入的版本为准*/
       *a_piTagVersion = iTagsVersion;
    }else
    {
        *a_piTagVersion = (int)strtol(scew_attribute_value(pstAttr), NULL, 0);
    }

    return TDR_SUCCESS;
}

int tdr_get_metalib_size_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError)
{
    int iRet = TDR_SUCCESS;
    int iTotalSize = 0;

    assert(NULL != pstParam);
    assert(NULL != pstRoot);
    assert(NULL != fpError);   

    if (TDR_XML_TAGSET_VERSION_0 == pstParam->iTagSetVersion)
    {
        iRet = tdr_get_metalib_ver0_size_i(pstParam, pstRoot, fpError);
    }else if (TDR_XML_TAGSET_VERSION_1 == pstParam->iTagSetVersion)
    {
        iRet = tdr_get_metalib_ver1_size_i(pstParam, pstRoot, fpError);
    }

    //计算整个metalib的大小
    iTotalSize = pstParam->iStrBufSize + pstParam->iMetaSize;

    iTotalSize += TDR_CALC_MIN_SIZE(pstParam->iMaxMetas, pstParam->iMaxMacros);
    
	/*计算宏定义组所需的内存空间总数*/
	iTotalSize += TDR_CALC_MACROSGROUP_MAP_SIZE(pstParam->iMaxMacrosGroupNum);
	iTotalSize += pstParam->iMacrosGroupSize;

    pstParam->iSize = iTotalSize;
    return iRet;  
    
}

int tdr_get_metalib_ver0_size_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError)
{
    scew_element *pstSubElement = NULL;


    int iRet = TDR_SUCCESS;
    int iTotalMetaSize = 0;
    int iTotalStrBufSize = 0;

    assert(NULL != pstParam);
    assert(NULL != pstRoot);
    assert(NULL != fpError);   

    pstSubElement = scew_element_next(pstRoot, NULL);
    while (NULL != pstSubElement)
    {
        if( 0==tdr_stricmp(scew_element_name(pstSubElement), TDR_TAG_MACROS) )
        {
            /*宏定义*/
            scew_element *pstItem = NULL;

            pstItem = scew_element_next(pstSubElement, NULL);
            while (NULL != pstItem)
            {
                if (0 == tdr_stricmp(scew_element_name(pstItem), TDR_TAG_MACRO))
                {
                     pstParam->iMaxMacros++;
					 
					 /*desc*/
					 pstParam->iStrBufSize += tdr_get_description_size_i(pstItem);					
                }else
                {
					fprintf(fpError, "warning:无效的宏定义标签<%s>,请使用标签<%s>来定义宏\n", scew_element_name(pstItem), TDR_TAG_MACRO);
                }
                pstItem = scew_element_next(pstSubElement, pstItem);
            }/*while (NULL != pstItem)*/           
        }else if (0 == tdr_stricmp(scew_element_name(pstSubElement), TDR_TAG_COMPOSITE_TYPE) )
        {
            int iMetaSize = 0;
            int iStrBufSize = 0;
            int iEntrisNum = 0;
            
            /*计算此结构体 struct/union*/
            iRet = tdr_get_meta_size_i(&iEntrisNum, &iMetaSize, &iStrBufSize, pstParam->iTagSetVersion, pstSubElement, pstRoot);
            if (0 >= iEntrisNum)
            {
                char szName[TDR_NAME_LEN] = {0};

                tdr_get_name_attribute_i(&szName[0], sizeof(szName), pstSubElement);
                fprintf(fpError, "\nerror:\t 自定义类型《name = %s> 没有包含任何\"item\"子元素成员", szName);
                iRet = TDR_ERRIMPLE_META_NO_ENTRY;
                break;
            }
      
            
            pstParam->iMaxMetas++;
            iTotalMetaSize += iMetaSize;
            iTotalStrBufSize += iStrBufSize;
        }//if( 0==tdr_stricmp(scew_element_name(pstSubElement), TDR_TAG_MACROS) )
        
        pstSubElement =	scew_element_next( pstRoot, pstSubElement ); 
    }

    pstParam->iMetaSize += iTotalMetaSize;
    pstParam->iStrBufSize += iTotalStrBufSize;

    return iRet;
}

scew_attribute *tdr_get_primarykey_by_depended_strut_i(scew_element *a_pstElement, scew_element *a_pstRoot)
{
	scew_attribute *pstAttr = NULL;	
	char szStructName[TDR_NAME_LEN] = {0};
	char szName[TDR_NAME_LEN] = {0};
	scew_element *pstSubItem;

	assert(NULL != a_pstElement);
	assert(NULL != a_pstRoot);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_DEPENDONSTRUCT);
	if (NULL == pstAttr)
	{
		return NULL;
	}

	tdr_normalize_string(&szStructName[0], sizeof(szStructName), scew_attribute_value(pstAttr));

	/*查找依赖的结构*/
	pstSubItem = scew_element_next(a_pstRoot, NULL);
	while( NULL != pstSubItem )
	{
		if (0 != strcmp(scew_element_name(pstSubItem), TDR_TAG_STRUCT))          
		{
			pstSubItem = scew_element_next( a_pstRoot, pstSubItem );
			continue;
		}  
		pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_NAME);
		if (NULL == pstAttr)
		{
			pstSubItem = scew_element_next( a_pstRoot, pstSubItem );
			continue;
		}

		tdr_normalize_string(&szName[0], sizeof(szName), scew_attribute_value(pstAttr));
		if (0 == strcmp(szName, szStructName))
		{
			pstAttr = scew_attribute_by_name(pstSubItem, TDR_TAG_PRIMARY_KEY);
			break;
		}
		pstSubItem = scew_element_next( a_pstRoot, pstSubItem );
	} 

	if (pstSubItem == NULL)
	{
		return NULL;
	}

	return pstAttr;
}

int tdr_get_meta_primaykey_size_i(scew_element *a_pstElement, scew_element *a_pstRoot)
{
	scew_attribute *pstAttr = NULL;
	char *pszKey;
	char *pchDot;
	char *pchBegin;
	int iCount;

	assert(NULL != a_pstElement);
	assert(NULL != a_pstRoot);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_PRIMARY_KEY);
	if (NULL == pstAttr)
	{
		/*查看此结构的主键信息是否依赖于其他结构*/
		pstAttr = tdr_get_primarykey_by_depended_strut_i(a_pstElement, a_pstRoot);
		if (NULL == pstAttr)
		{
			return 0;
		}		
	}

	pszKey = strdup(scew_attribute_value(pstAttr));

	iCount = 0;
	pchBegin = pszKey;
	pchDot = strchr(pchBegin, ',');
	while (NULL != pchDot)
	{
		*pchDot = '\0';
		tdr_trim_str(pchBegin);
		if (*pchBegin != '\0')
		{
			iCount++;
		}
		pchDot++;
		pchBegin = pchDot;
		pchDot = strchr(pchBegin, ',');
	}

	tdr_trim_str(pchBegin);
	if (*pchBegin != '\0')
	{
		iCount++;
	}	

	free(pszKey);

	return iCount * sizeof(TDRDBKEYINFO);
}

int tdr_get_meta_size_i(int *a_piEntirsNum, int *a_piMetaSize, int *a_piStrBufSize, IN int a_iTagversion,
						scew_element *a_pstElement, scew_element *a_pstRoot)
{
    int iEntryNum = 0;
 
    int iTotalLen = 0;
	int iPrimaryKeySize = 0;
	const char *pszItemName;
    scew_element *pstSubItem = NULL;


    assert(NULL != a_piEntirsNum);
    assert(NULL != a_piMetaSize);
    assert(NULL != a_piStrBufSize);
    assert(NULL != a_pstElement);
    assert((TDR_XML_TAGSET_VERSION_0 <= a_iTagversion) && (a_iTagversion <= TDR_XML_TAGSET_VERSION_1));
   

     /*从标签集版本1开始增加了 cname和defaultvalue两个值，这两个字符串保存在字符串缓冲区中*/
    if (TDR_XML_TAGSET_VERSION_1 <= a_iTagversion)
    {
        /*cname*/
		iTotalLen += tdr_get_cname_size_i(a_pstElement);
        
        
        /*desc*/
		iTotalLen += tdr_get_description_size_i(a_pstElement);
        

		/*获取建表主键信息*/
		iPrimaryKeySize = tdr_get_meta_primaykey_size_i(a_pstElement, a_pstRoot);	


    }

    /**计算自定义类型个成员的字符串信息所需的空间，目前定义的字符串属性有： name, cname,desc
     *default value. 在计算字符串所占空间大小时会将其长度加1，以保存结束字符*/
    TDR_GET_ENTRY_TAG_NAME(pszItemName, a_iTagversion);
    pstSubItem = scew_element_next(a_pstElement, NULL);
    iEntryNum = 0;
    while (NULL != pstSubItem)
    {
        if( 0 != tdr_stricmp(scew_element_name(pstSubItem), pszItemName) )
        {
            pstSubItem = scew_element_next(a_pstElement, pstSubItem);
            continue;
        }

        iEntryNum++;  
		iTotalLen += tdr_get_description_size_i(pstSubItem);
        
        
        /*从标签集版本1开始增加了 cname和defaultvalue两个值*/
        if (TDR_XML_TAGSET_VERSION_1 <= a_iTagversion)
        {
            /*cname*/
			iTotalLen += tdr_get_cname_size_i(pstSubItem);
           
            
            /*defalut*/
			iTotalLen += tdr_get_entry_defaultvalue_size_i(pstSubItem);

			iTotalLen += tdr_get_entry_customattr_size_i(pstSubItem);
        }

        pstSubItem = scew_element_next(a_pstElement, pstSubItem);
    }

   
    *a_piEntirsNum = iEntryNum;
    *a_piMetaSize = TDR_CALC_MIN_META_SIZE(iEntryNum) + iPrimaryKeySize;   
    *a_piStrBufSize = iTotalLen;
    return TDR_SUCCESS;
}

int tdr_get_entry_customattr_size_i(scew_element *a_pstItem)
{
	scew_attribute *pstAttr;
	const char *pszValue;
	int iSize = 0;


	assert(NULL != a_pstItem);


	pstAttr = scew_attribute_by_name(a_pstItem, TDR_TAG_CUSTOMATTR);
	if (NULL == pstAttr)
	{
		return 0;
	}
	pszValue = scew_attribute_value(pstAttr);
	if (NULL == pszValue)
	{
		return 0;
	}
	

	iSize = strlen(pszValue) + 1;
	

	return iSize;
}

int tdr_get_entry_defaultvalue_size_i(scew_element *a_pstItem)
{
	scew_attribute *pstAttr;
	scew_attribute *pszTypeAttr;
	const char *pszTypeName;
	int idxType;
	LPTDRCTYPEINFO pstTypeInfo;
	int iSize = 0;


	assert(NULL != a_pstItem);


	pstAttr = scew_attribute_by_name(a_pstItem, TDR_TAG_DEFAULT_VALUE);
	if (NULL == pstAttr)
	{
		return 0;
	}

	pszTypeAttr = scew_attribute_by_name(a_pstItem, TDR_TAG_TYPE);
	if (NULL == pszTypeAttr)
	{
		return 0;	
	}
	pszTypeName = scew_attribute_value(pszTypeAttr);
	idxType = tdr_typename_to_idx(pszTypeName);
	if (TDR_INVALID_INDEX == idxType)
	{
		return 0;
	}
	
	pstTypeInfo = tdr_idx_to_typeinfo(idxType);
	if (TDR_TYPE_COMPOSITE >= pstTypeInfo->iType)
	{
		return 0;
	}

	if (TDR_TYPE_STRING > pstTypeInfo->iType)
	{
		iSize = pstTypeInfo->iSize;
	}else if (TDR_TYPE_STRING == pstTypeInfo->iType)
	{
		iSize = strlen(scew_attribute_value(pstAttr)) + 1;
	}else if (TDR_TYPE_WSTRING == pstTypeInfo->iType)
	{
		iSize = (strlen(scew_attribute_value(pstAttr)) + 1)*sizeof(tdr_wchar_t);
	}


	return iSize;
}

int tdr_get_metalib_ver1_size_i(INOUT LPTDRLIBPARAM pstParam, IN scew_element *pstRoot, IN FILE *fpError)
{
    scew_element *pstSubElement = NULL;
    int iRet = TDR_SUCCESS;
    int iTotalMetaSize = 0;
    int iTotalStrBufSize = 0;
  

    assert(NULL != pstParam);
    assert(NULL != pstRoot);
    assert(NULL != fpError);   

    pstSubElement = scew_element_next(pstRoot, NULL);
    while (NULL != pstSubElement)
    {
         const char *pszName = NULL;

        pszName = scew_element_name(pstSubElement);
		assert(NULL != pszName);

		if ( 0==tdr_stricmp(pszName, TDR_TAG_MACRO) )
        {
            /*宏定义*/
            pstParam->iMaxMacros++;
			
			/*desc*/
			iTotalStrBufSize += tdr_get_description_size_i(pstSubElement);
			
        }

		if (0 == tdr_stricmp(pszName, TDR_TAG_MACROSGROUP))
		{
			int iMacroNum = 0;
			int iMacrosGroupSize = 0;
			int iStrBuffSize = 0;
			tdr_get_macrosgroup_size_i(&iMacroNum, &iMacrosGroupSize, &iStrBuffSize, pstSubElement);
			pstParam->iMaxMacros += iMacroNum;
			pstParam->iMacrosGroupSize += iMacrosGroupSize;
			iTotalStrBufSize += iStrBuffSize;
			pstParam->iMaxMacrosGroupNum++;
		}/*if (0 == tdr_stricmp(pszName, TDR_TAG_MACROSGROUP))*/
        
        if ((0 == tdr_stricmp(pszName, TDR_TAG_STRUCT)) ||
            (0 == tdr_stricmp(pszName, TDR_TAG_UNION)))
        {
            int iMetaSize = 0;
            int iStrBufSize = 0;
            int iEntrisNum = 0;
            
            /*计算此结构体 struct/union*/
            iRet = tdr_get_meta_size_i(&iEntrisNum, &iMetaSize, &iStrBufSize, pstParam->iTagSetVersion, pstSubElement, pstRoot);
            if (0 >= iEntrisNum)
            {
                char szName[TDR_NAME_LEN] = {0};

                tdr_get_name_attribute_i(&szName[0], sizeof(szName), pstSubElement);
                fprintf(fpError, "error:\t 自定义类型《name = %s> 没有包含任何\"entry\"子元素成员.\n", szName);
                iRet = TDR_ERRIMPLE_META_NO_ENTRY;
                break;
            }
                       
            pstParam->iMaxMetas++;
            iTotalMetaSize += iMetaSize;
            iTotalStrBufSize += iStrBufSize;
        }//if( 0==tdr_stricmp(scew_element_name(pstSubElement), TDR_TAG_MACROS) )		

        pstSubElement =	scew_element_next( pstRoot, pstSubElement ); 
    }

    pstParam->iMetaSize += iTotalMetaSize;
    pstParam->iStrBufSize += iTotalStrBufSize;

    return iRet;
}

int tdr_get_metalib_version_i(OUT long *a_plVersion, IN scew_element* a_pstRoot)
{
    scew_attribute *pstAttr = NULL;

    assert(NULL != a_plVersion);
    assert(NULL != a_pstRoot);



    *a_plVersion = TDR_INVALID_VERSION;
   
    pstAttr = scew_attribute_by_name(a_pstRoot, TDR_TAG_VERSION);
    if (NULL != pstAttr)
    {
        *a_plVersion = strtol(scew_attribute_value(pstAttr), NULL, 0);        
    }

    return TDR_SUCCESS;
}

/**返回description属性值所需的内存空间字节数 
*/
int tdr_get_description_size_i(scew_element *a_pstItem)
{
	int iStrLen = 0;
	scew_attribute *pstAttr = NULL;

	assert(NULL != a_pstItem);

	pstAttr = scew_attribute_by_name(a_pstItem, TDR_TAG_DESCIPTION);
	if (NULL != pstAttr)
	{
		iStrLen = strlen(scew_attribute_value(pstAttr)) + 1;
		iStrLen = TDR_MIN(iStrLen, TDR_DESC_LEN); /*超过预定最大长度就截断*/
	}

	return iStrLen;
}

/**返回cname属性值所需的内存空间字节数 
*/
int tdr_get_cname_size_i(scew_element *a_pstItem)
{
	int iStrLen = 0;
	scew_attribute *pstAttr = NULL;

	assert(NULL != a_pstItem);

	pstAttr = scew_attribute_by_name(a_pstItem, TDR_TAG_CNNAME);
	if (NULL != pstAttr)
	{
		iStrLen = strlen(scew_attribute_value(pstAttr)) + 1;
		iStrLen = TDR_MIN(iStrLen, TDR_CHNAME_LEN); /*超过预定最大长度就截断*/
	}

	return iStrLen;
}

/**获取单个宏定义组所需内存空间
*@param[out] a_piMacrosNum 获取此宏定义组包含的macro数目
*@param[out] a_piGroupSize 获取此宏定义组元素存储所占用的字节数
*@param[out] a_piStrBufSize 获取此meta元素在字符串缓冲区中所占的字节数
*@param[in] a_pstElement 需考察的元素
*/
int tdr_get_macrosgroup_size_i(int *a_piMacrosNum, int *a_piGroupSize, int *a_piStrBufSize, scew_element *a_pstElement)
{
	int iMacroNum = 0;
	int iStrLen = 0;

	scew_element *pstSubItem = NULL;


	assert(NULL != a_piMacrosNum);
	assert(NULL != a_piGroupSize);
	assert(NULL != a_piStrBufSize);
	assert(NULL != a_pstElement);
	

	iStrLen += tdr_get_description_size_i(a_pstElement);

	
	/*计算此宏定义组包含的macro个数及所需空间*/
	pstSubItem = scew_element_next(a_pstElement, NULL);
	while (NULL != pstSubItem)
	{
		if( 0 != tdr_stricmp(scew_element_name(pstSubItem), TDR_TAG_MACRO) )
		{
			pstSubItem = scew_element_next(a_pstElement, pstSubItem);
			continue;
		}

		iMacroNum++;  
		iStrLen += tdr_get_description_size_i(pstSubItem);
		pstSubItem = scew_element_next(a_pstElement, pstSubItem);
	}


	*a_piMacrosNum = iMacroNum;
	*a_piGroupSize = TDR_CALC_MIN_MACROSGROUP_SIZE(iMacroNum);   
	*a_piStrBufSize = iStrLen;
	return TDR_SUCCESS;
}

