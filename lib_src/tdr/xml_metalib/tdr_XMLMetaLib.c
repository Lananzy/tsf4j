/**
*
* @file     tdr_XMLMetaLib.c  
* @brief    元数据描述库与XML描述相互转换
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-03-26 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/


#include <scew/scew.h>
#include <assert.h>

#include "tdr/tdr_os.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_XMLMetaLib.h"

#include "tdr_scew_if.h"
#include "tdr/tdr_error.h"
#include "tdr/tdr_error_i.h"
#include "tdr/tdr_metalib_manage_i.h"
#include "tdr_metaleb_param_i.h"
#include "tdr/tdr_metalib_init.h"
#include "tdr/tdr_ctypes_info_i.h"
#include "tdr_XMLtags_i.h"
#include "tdr/tdr_iostream_i.h"
#include "tdr_XMLMetalib_i.h"
#include "tdr_metalib_entry_manage_i.h"
#include "tdr_metalib_meta_manage_i.h"
#include "tdr/tdr_ctypes_info.h"
#include "tdr/tdr_auxtools.h"
#include "tdr/tdr_auxtools.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif




/**将metalib库的根元素写到IO流中
*@param[in] a_pstLib 元数据库
*@Param[out]	a_pstIOStream 保存元数据XML信息的IO流
*@return 成功返回0，否则返回错误代码
*@pre \e a_pstLib 不能为NULL
*@pre \e a_pstIOStream 不能为NULL
*/
static int tdr_save_metalib_header_i(LPTDRMETALIB a_pstLib, LPTDRIOSTREAM a_pstIOStream);

/**将宏定义信息写到IO流中
*@param[in] a_pstMacro 需要保存的宏定义
*@Param[out]	a_pstIOStream 保存元数据XML信息的IO流
*@return 成功返回0，否则返回错误代码
*@pre \e a_pstLib 不能为NULL
*@pre \e a_pstIOStream 不能为NULL
*/
static int tdr_save_macro_i(LPTDRMETALIB a_pstLib, LPTDRMACRO a_pstMacro, LPTDRIOSTREAM a_pstIOStream);


/**将struct 结构信息写到IO流中
*@param[in] a_pstMeta 需要保存的struct定义
*@Param[out]	a_pstIOStream 保存元数据XML信息的IO流
*@return 成功返回0，否则返回错误代码
*@pre \e a_pstMeta 不能为NULL
*@pre \e a_pstIOStream 不能为NULL
*/
static int tdr_save_struct_meta_i(LPTDRMETA a_pstMeta, LPTDRIOSTREAM a_pstIOStream);

/**将struct /union结构元素的公共属性写到IO流中
*@param[in] a_pstMeta 需要保存的meta定义
*@param[out]	a_pstIOStream 保存元数据XML信息的IO流
*@return 成功返回0，否则返回错误代码
*@pre \e a_pstMeta 不能为NULL
*@pre \e a_pstIOStream 不能为NULL
*/
static int tdr_save_meta_common_attribute_i(LPTDRMETA a_pstMeta, LPTDRIOSTREAM a_pstIOStream);

/**将entry结构元素写到IO流中
*@param[in] a_pstMeta entry所在的meta
*@param[in] a_idxEntry 要处理的entry的索引
*@param[out]	a_pstIOStream 保存元数据XML信息的IO流
*@return 成功返回0，否则返回错误代码
*@pre \e a_pstMeta 不能为NULL
*@pre \e a_pstIOStream 不能为NULL
*/
static int tdr_save_meta_entry_i(LPTDRMETA a_pstMeta, int a_idxEntry, LPTDRIOSTREAM a_pstIOStream);

/**将entry最基本的属性<name version id cnname desc type size count>保存在IO流中
*@param[in] a_pstEntry 要处理的entry
*@param[in] a_pstMeta entry所在的meta
*@param[out]	a_pstIOStream 保存元数据XML信息的IO流
*@return 成功返回0，否则返回错误代码
*@pre \e a_pstEntry 不能为NULL
*@pre \e a_pstMeta 不能为NULL
*@pre \e a_pstIOStream 不能为NULL
*/
static int tdr_save_entry_base_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, LPTDRIOSTREAM a_pstIOStream);

static int tdr_save_entry_defaultval_i(LPTDRIOSTREAM a_pstIOStream, LPTDRMETALIB a_pstLib, LPTDRMETAENTRY a_pstEntry);

/**将entry的select属性保存在IO流中
*@param[in] a_pstMeta entry所在的meta
*@param[in] a_idxEntry 要处理的entry的索引
*@param[out]	a_pstIOStream 保存元数据XML信息的IO流
*@return 成功返回0，否则返回错误代码
*@pre \e a_pstMeta 不能为NULL
*@pre \e a_pstIOStream 不能为NULL
*/
static int tdr_save_entry_union_attribute(LPTDRMETA a_pstMeta, int a_idxEntry, LPTDRIOSTREAM a_pstIOStream);

/**将union meta保存在IO流中
*@param[in] a_pstMeta entry所在的meta
*@param[out]	a_pstIOStream 保存元数据XML信息的IO流
*@return 成功返回0，否则返回错误代码
*@pre \e a_pstMeta 不能为NULL
*@pre \e a_pstIOStream 不能为NULL
*/
static int tdr_save_union_meta_i(LPTDRMETA a_pstMeta, LPTDRIOSTREAM a_pstIOStream);

/**将XML元素树中的描述的数据添加到库中
*@param[inout] a_pstLib 需要更新的元数据库
*@param[in] a_pstTree XML元数树
*@param[in] a_fpError 用来记录处理错误信息的文件句柄
*
*@return 成功返回TDR_SUCCESS,否则返回错误号
*
*@pre \e a_pstLib 不能为NULL
*@pre \e a_pstTree 不能为NULL
*@pre \e a_fpError 不能为NULL
*/
static int tdr_add_matalib_i(TDRMETALIB* a_pstLib, scew_tree *a_pstTree, FILE *a_fpError);

/** 将tagsetversion=0版本的XML元素树加到原数据库
*@param[inout] a_pstLib 需要更新的元数据库
*@param[in] a_pstTree XML元数树
*@param[in] a_fpError 用来记录处理错误信息的文件句柄
*
*@return 成功返回TDR_SUCCESS,否则返回错误号
*
*@pre \e a_pstLib 不能为NULL
*@pre \e a_pstTree 不能为NULL
*@pre \e a_fpError 不能为NULL
*/
static int tdr_add_metalib_Ver0_i(TDRMETALIB* a_pstLib, scew_tree *a_pstTree, FILE *a_fpError);


/** 将tagsetversion=1版本的XML元素树加到原数据库
*@param[inout] a_pstLib 需要更新的元数据库
*@param[in] a_pstTree XML元数树
*@param[in] a_fpError 用来记录处理错误信息的文件句柄
*
*@return 成功返回TDR_SUCCESS,否则返回错误号
*
*@pre \e a_pstLib 不能为NULL
*@pre \e a_pstTree 不能为NULL
*@pre \e a_fpError 不能为NULL
*/
static int tdr_add_metalib_Ver1_i(TDRMETALIB* a_pstLib, scew_tree *a_pstTree, FILE *a_fpError);

static int tdr_union_entry_comp_i(const void* pv1, const void* pv2);

static int tdr_identry_comp_i(const void* pv1, const void* pv2);

static int tdr_name_entry_comp_i(const void* pv1, const void* pv2);

static int tdr_ckeck_metalib_i(TDRMETALIB* a_pstLib, FILE *a_fpError);

static int tdr_save_all_macros_i(IN LPTDRMETALIB a_pstLib, INOUT LPTDRIOSTREAM a_pstIOStream);



int  tdr_create_lib(INOUT LPTDRMETALIB *a_ppstLib, IN const char* a_pszXml, IN int a_iXml, IN int a_iTagSetVersion, IN FILE* a_fpError)
{
    LPTDRMETALIB pstLib = NULL;
    TDRLIBPARAM stLibParam;
    scew_tree* pstTree = NULL;
    int iRet = TDR_SUCCESS;
    
    /*assert(NULL != a_ppstLib);
    assert(NULL != a_pszXml);
    assert(0 < a_iXml);
    assert(NULL != a_fpError);  */
	if ((NULL == a_ppstLib)||(NULL == a_pszXml)||(0>= a_iXml))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}
	if (NULL == a_fpError)
	{
		a_fpError = stdout;
	}
    

    /*创建XML元素树*/    
    iRet = tdr_create_XMLParser_tree_byBuff_i(&pstTree, a_pszXml, a_iXml, a_fpError);
    
    
       
    /*获取创建元数据库所需的参数*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_lib_param_i(&stLibParam, pstTree, a_iTagSetVersion, a_fpError);
    }
    
    /*初始化元数据库*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
       iRet = tdr_init_metalib_i(&pstLib, &stLibParam);
       if (TDR_ERR_IS_ERROR(iRet))
       {
           fprintf(a_fpError, "error: 初始化元数据库失败: %s\n", tdr_error_string(iRet));
       }
    }
   
    
    /*将XML元素树中的信息加到metalib中*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_add_matalib_i(pstLib, pstTree, a_fpError);   
    }
    

    if (!TDR_ERR_IS_ERROR(iRet))
    {  
        *a_ppstLib = pstLib;        
    }else
    {
        tdr_free_lib(&pstLib);
    }

    /*释放资源*/    
    scew_tree_free( pstTree );
    
    
   return iRet;
}

int tdr_create_XMLParser_tree_byBuff_i(scew_tree **a_ppstTree, IN const char* a_pszXml, IN int a_iXml, FILE* a_fpError)
{
    scew_parser* pstParser = NULL;
    scew_tree *pstTempTree = NULL;
    int iRet = TDR_SUCCESS;

    assert(NULL != a_ppstTree);
    assert(NULL != a_pszXml);
    assert(0 < a_iXml);
    assert(NULL != a_fpError);

    *a_ppstTree = NULL;

    pstParser =	tdr_parser_create();
    if( NULL==pstParser )
    {
        fprintf(a_fpError, "error: \t创建XML解析器失败\n");
        
        return TDR_ERRIMPLE_FAILED_EXPACT_XML;
    }

    if (!parser_load_buffer(pstParser, a_pszXml, a_iXml) )
    {
        fprintf(a_fpError, "error: \t将XML缓冲区的信息加载到解析器失败: %s, 位置<line:%d, column:%d>\n", parser_expact_error_string(parser_expact_error_code(pstParser)),
            scew_error_expat_line(pstParser), scew_error_expat_column(pstParser));
        
        iRet = TDR_ERRIMPLE_FAILED_EXPACT_XML;
    }    
    
    //分析XML元素树
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        pstTempTree = tdr_parser_tree(pstParser);
        if (NULL == pstTempTree)
        {
            fprintf(a_fpError, "error: \t解析器解析XML失败: %s, 位置<line:%d, column:%d>\n", 
                parser_expact_error_string(parser_expact_error_code(pstParser)), scew_error_expat_line(pstParser), scew_error_expat_column(pstParser));
            
            iRet =  TDR_ERRIMPLE_FAILED_EXPACT_XML;
        }
    }
    

    //释放解析器的资源
    parser_free( pstParser );

    if (!TDR_ERR_IS_ERROR(iRet))
    {
        *a_ppstTree = pstTempTree;
    }
   
    
    return iRet;
}

int tdr_init_metalib_i(LPTDRMETALIB *ppstLib, LPTDRLIBPARAM pstLibParam)
{
    LPTDRMETALIB pstLib = NULL;
    int iRet = TDR_SUCCESS;

    assert(NULL != ppstLib);
    assert(NULL != pstLibParam);
    assert(0 < pstLibParam->iSize);

    pstLib = (LPTDRMETALIB) calloc(1, pstLibParam->iSize);    
    if (NULL != pstLib)
    {
        iRet = tdr_init_lib(pstLib, pstLibParam);
    }

    
    if (TDR_ERR_IS_ERROR(iRet))
    {
        tdr_free_lib(&pstLib);
        
        *ppstLib = NULL;
    }else
    {
        *ppstLib = pstLib;
    }

    return iRet;
}

int tdr_create_lib_fp(INOUT LPTDRMETALIB *a_ppstLib, IN FILE* a_fpXML, IN int a_iTagSetVersion, IN FILE* a_fpError)
{
    LPTDRMETALIB pstLib = NULL;
    TDRLIBPARAM stLibParam;
    scew_tree* pstTree = NULL;
    int iRet = TDR_SUCCESS;
    
    /*assert(NULL != a_ppstLib);
    assert(NULL != a_fpXML);
    assert(NULL != a_fpError);   */
	if ((NULL == a_ppstLib)||(NULL == a_fpXML))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}
	if (NULL == a_fpError)
	{
		a_fpError = stdout;
	}
    
    
    /*创建XML元素树*/
    iRet = tdr_create_XMLParser_tree_byfp(&pstTree, a_fpXML, a_fpError);
   
    
    
    /*获取创建元数据库所需的参数*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_lib_param_i(&stLibParam, pstTree, a_iTagSetVersion, a_fpError);
    }
    
    /*初始化元数据库*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_init_metalib_i(&pstLib, &stLibParam);
    }else
    {
        fprintf(a_fpError, "\nerror: 初始化元数据库失败: %s\n", tdr_error_string(iRet));
    }
    
    /*将XML元素树中的信息加到metalib中*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_add_matalib_i(pstLib, pstTree, a_fpError);   
    }
    
    
    if (!TDR_ERR_IS_ERROR(iRet))
    {  
        *a_ppstLib = pstLib;        
    }else
    {
        tdr_free_lib(&pstLib);
  
        *a_ppstLib = NULL;
    }
    
    /*释放资源*/    
    scew_tree_free( pstTree );
    
    
    return iRet; 
}

int tdr_create_XMLParser_tree_byfp(scew_tree **a_ppstTree, FILE *a_fpXML, FILE *a_fpError)
{
    scew_parser* pstParser = NULL;
    scew_tree *pstTempTree = NULL;
    int iRet = TDR_SUCCESS;

    assert(NULL != a_ppstTree);
    assert(NULL != a_fpXML);
    assert(NULL != a_fpError);

    *a_ppstTree = NULL;

    pstParser =	tdr_parser_create();
    if( NULL==pstParser )
    {
        fprintf(a_fpError, "\nerror: \t创建XML解析器失败, error<%d> for %s\n", parser_error_code(), parser_error_string(parser_error_code(pstParser)));
        
        return TDR_ERRIMPLE_FAILED_EXPACT_XML;
    }

    if (!TDR_ERR_IS_ERROR(iRet) &&  !parser_load_fp(pstParser, a_fpXML) )
    {
		fprintf(a_fpError, "\nerror: \t根据XML文件句柄加载到解析器失败: %s, 位置<line:%d, column:%d>\n",  
			parser_expact_error_string(parser_expact_error_code(pstParser)),scew_error_expat_line(pstParser), scew_error_expat_column(pstParser));
        
        iRet = TDR_ERRIMPLE_FAILED_EXPACT_XML;
    }    
    
    //分析XML元素树
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        pstTempTree = tdr_parser_tree(pstParser);
        if (NULL == pstTempTree)
        {
            fprintf(a_fpError, "\nerror: \t解析器解析XML失败: %s\n", 
                parser_expact_error_string(parser_expact_error_code(pstParser)));
            
            iRet = TDR_ERRIMPLE_FAILED_EXPACT_XML;
        }else
        {
            *a_ppstTree = pstTempTree;
        }
    }
   

    //释放解析器的资源
    parser_free( pstParser );
   
    
    return iRet;
}

int tdr_create_XMLParser_tree_byFileName(scew_tree **a_ppstTree, const char *a_pszFileName, FILE *a_fpError)
{
    scew_parser* pstParser = NULL;
    scew_tree *pstTempTree = NULL;
    int iRet = TDR_SUCCESS;

    assert(NULL != a_ppstTree);
    assert(NULL != a_pszFileName);
    assert(NULL != a_fpError);

    *a_ppstTree = NULL;

    pstParser =	tdr_parser_create();
    if( NULL==pstParser )
    {
        fprintf(a_fpError, "\nerror: \t创建XML解析器失败, error<%d> for %s\n", parser_error_code(), parser_error_string(parser_error_code(pstParser)));
        
        return TDR_ERRIMPLE_FAILED_EXPACT_XML;
    }

    if (!TDR_ERR_IS_ERROR(iRet) && !parser_load_file(pstParser, a_pszFileName) )
    {
		const char *pszExpactError = parser_expact_error_string(parser_expact_error_code(pstParser));
		int iLine = scew_error_expat_line(pstParser);
		int iColumn = scew_error_expat_column(pstParser);
        fprintf(a_fpError, "\nerror: \t将XML文件<%s>的信息加载到解析器失败: %s, 位置<line:%d, column:%d>\n", a_pszFileName, 
			pszExpactError, iLine, iColumn);
        iRet = TDR_ERRIMPLE_FAILED_EXPACT_XML;
    } 
	 
    
    //分析XML元素树
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        pstTempTree = tdr_parser_tree(pstParser);
        if (NULL == pstTempTree)
        {
            fprintf(a_fpError, "\nerror: \t解析器解析XML失败: %s\n", 
                parser_expact_error_string(parser_expact_error_code(pstParser)));
            
            iRet =  TDR_ERRIMPLE_FAILED_EXPACT_XML;
        }else
        {
            *a_ppstTree = pstTempTree;
        } 
    }
    

    //释放解析器的资源
    parser_free( pstParser );
   
    
    return iRet;
}


int tdr_create_lib_file(LPTDRMETALIB *a_ppstLib, IN const char* a_pszFile, IN int a_iXMLTagVersion, IN FILE* a_fpError)
{
    LPTDRMETALIB pstLib = NULL;
    TDRLIBPARAM stLibParam;
    scew_tree* pstTree = NULL;
    int iRet = TDR_SUCCESS;
    
    /*assert(NULL != a_ppstLib);
    assert(NULL != a_pszFile);
    assert(NULL != a_fpError);*/
	if ((NULL == a_ppstLib)||(NULL == a_pszFile))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}
	if (NULL == a_fpError)
	{
		a_fpError = stdout;
	}
    
    
    /*创建XML元素树*/
    iRet = tdr_create_XMLParser_tree_byFileName(&pstTree, a_pszFile, a_fpError);
    
    
    
    /*获取创建元数据库所需的参数*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_lib_param_i(&stLibParam, pstTree, a_iXMLTagVersion, a_fpError);
    }
    
    /*初始化元数据库*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_init_metalib_i(&pstLib, &stLibParam);
    }else
    {
        fprintf(a_fpError, "\nerror: 初始化元数据库失败: %s\n", tdr_error_string(iRet));
    }
    
    /*将XML元素树中的信息加到metalib中*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_add_matalib_i(pstLib, pstTree, a_fpError);   
    }
    
    
    if (!TDR_ERR_IS_ERROR(iRet))
    {  
        *a_ppstLib = pstLib;        
    }else
    {
        tdr_free_lib(&pstLib);
        
        *a_ppstLib = NULL;
    }
    
    /*释放资源*/    
    scew_tree_free( pstTree );
    
    
    return iRet; 
}


int tdr_create_lib_multifile(INOUT LPTDRMETALIB *a_ppstLib, IN const char* a_aszFile[], IN int a_iFile, IN int a_iXMLTagVersion, IN FILE* a_fpError)
{
    int iRet = TDR_SUCCESS;
    scew_tree* pastTreeSet[TDR_MAX_FILES_IN_ONE_PARSE] = {NULL};
    LPTDRMETALIB pstLib = NULL;
    TDRLIBPARAM stLibParam;
    int i;

    /*assert(NULL != a_ppstLib);
    assert(NULL != a_aszFile);
    assert(0 < a_iFile);
    assert(NULL != a_fpError);*/
	if ((NULL == a_ppstLib)||(NULL == a_aszFile)||(0 >= a_iFile))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}
	if (NULL == a_fpError)
	{
		a_fpError = stdout;
	}


    

    if (TDR_MAX_FILES_IN_ONE_PARSE < a_iFile)
    {
        fprintf(a_fpError, "\nerror:\t 一次最多能解析%d个文件,而需要解析的文件数%d\n", TDR_MAX_FILES_IN_ONE_PARSE, a_iFile);
        iRet = TDR_ERRIMPLE_INVALID_TAGSET_VERSION;
    }

    /*创建XML元素树*/
	memset(pastTreeSet, 0, sizeof(pastTreeSet));
    if (TDR_SUCCESS == iRet)
    {
        for (i = 0; i < a_iFile; i++)
        {
            iRet = tdr_create_XMLParser_tree_byFileName(&pastTreeSet[i], a_aszFile[i], a_fpError);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }
        }/*for (i = 0; i < a_iFile; i++)*/
    }    
    
    /*根据XML树生元数据的参数*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        tdr_init_lib_param_i(&stLibParam);

        for (i = 0; i < a_iFile; i++)
        {
            iRet = tdr_add_lib_param_i(&stLibParam, pastTreeSet[i], a_iXMLTagVersion, a_fpError);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }
        }/*for (i = 0; i < a_iFile; i++)*/
    }

    /*初始化元数据库*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_init_metalib_i(&pstLib, &stLibParam);
    }else
    {
        fprintf(a_fpError, "error: 初始化元数据库失败: %s\n", tdr_error_string(iRet));
    }
     

     /*将XML元素树中的信息加到metalib中*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        for (i = 0; i < a_iFile; i++)
        {
            iRet = tdr_add_matalib_i(pstLib, pastTreeSet[i], a_fpError);
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }
        }/*for (i = 0; i < a_iFile; i++)*/
    }
    
    
    if (!TDR_ERR_IS_ERROR(iRet))
    {  
        *a_ppstLib = pstLib;        
    }else
    {
        tdr_free_lib(&pstLib);      
        *a_ppstLib = NULL;
    }
    
    /*释放资源*/
    for (i = 0; i < a_iFile; i++)
    {
		if (NULL != pastTreeSet[i])
		{
			scew_tree_free(pastTreeSet[i]); 
		}         
    }/*for (i = 0; i < a_iFile; i++)*/

    return iRet;
}

// 在函数开头添加日志文件处理逻辑
int tdr_add_meta(INOUT LPTDRMETALIB a_pstLib, IN const char* a_pszXml, IN int a_iXml, IN int a_iXMLTagVersion, IN FILE* a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_tree* pstTree = NULL;
	TDRLIBPARAM stLibParam;
	int iInitialMetaCount = a_pstLib->iCurMetaNum;
	FILE* logFile = NULL; // 日志文件句柄

	// 处理日志文件：不存在则创建，存在则追加
	logFile = fopen("tdr_meta_load.log", "a+");
	if (logFile == NULL) {
		// 若创建日志文件失败，降级使用原错误输出句柄
		logFile = (a_fpError != NULL) ? a_fpError : stdout;
		fprintf(logFile, "[tdr_add_meta] 警告：无法创建日志文件，将日志输出到默认位置\n");
	}

	// 替换原有 a_fpError 为日志文件句柄（确保所有日志写入文件）
	if (a_fpError == NULL) {
		a_fpError = logFile;
	}

	// 以下为原有逻辑，日志输出均使用 a_fpError（已指向日志文件）
	if ((NULL == a_pstLib) || (NULL == a_pszXml) || (0 >= a_iXml))
	{
		fprintf(a_fpError, "[tdr_add_meta] 错误：无效参数（lib=%p, xml=%p, xml_len=%d）\n",
			a_pstLib, a_pszXml, a_iXml);
		fclose(logFile); // 关闭日志文件
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	fprintf(a_fpError, "[tdr_add_meta] 开始处理XML数据，长度：%d字节，标签版本：%d\n",
		a_iXml, a_iXMLTagVersion);

	/*创建XML元素树*/
	fprintf(a_fpError, "[tdr_add_meta] 解析XML内容，生成元素树...\n");
	iRet = tdr_create_XMLParser_tree_byBuff_i(&pstTree, a_pszXml, a_iXml, a_fpError);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		fprintf(a_fpError, "[tdr_add_meta] 错误：XML解析失败，返回码：%d\n", iRet);
		fclose(logFile); // 关闭日志文件
		return iRet;
	}
	if (NULL == pstTree)
	{
		fprintf(a_fpError, "[tdr_add_meta] 错误：解析后元素树为空\n");
		fclose(logFile); // 关闭日志文件
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	/*获取创建元数据库所需的参数*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		tdr_init_lib_param_i(&stLibParam);
		TDR_STRNCPY(stLibParam.szName, a_pstLib->szName, sizeof(stLibParam.szName));
		fprintf(a_fpError, "[tdr_add_meta] 解析XML中的元数据参数（预检查）...\n");
		iRet = tdr_add_lib_param_i(&stLibParam, pstTree, a_iXMLTagVersion, a_fpError);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			fprintf(a_fpError, "[tdr_add_meta] 错误：获取元数据参数失败，返回码：%d\n", iRet);
			scew_tree_free(pstTree);
			fclose(logFile); // 关闭日志文件
			return iRet;
		}
		fprintf(a_fpError, "[tdr_add_meta] 预检查结果：预计新增宏数量=%d，预计新增自定义类型数量=%d，所需空间=%d字节\n",
			stLibParam.iMaxMacros, stLibParam.iMaxMetas, stLibParam.iMetaSize);
	}

	/*检查空间*/
	if ((a_pstLib->iMaxMacroNum - a_pstLib->iCurMacroNum) < stLibParam.iMaxMacros)
	{
		fprintf(a_fpError, "[tdr_add_meta] 错误：宏定义空间不足，最大%d，已用%d，需新增%d\n",
			a_pstLib->iMaxMacroNum, a_pstLib->iCurMacroNum, stLibParam.iMaxMacros);
		iRet = TDR_ERRIMPLE_INVALID_METALIB_PARAM;
	}
	if (!TDR_ERR_IS_ERROR(iRet) && ((a_pstLib->iMaxMetaNum - a_pstLib->iCurMetaNum) < stLibParam.iMaxMetas))
	{
		fprintf(a_fpError, "[tdr_add_meta] 错误：自定义类型空间不足，最大%d，已用%d，需新增%d\n",
			a_pstLib->iMaxMetaNum, a_pstLib->iCurMetaNum, stLibParam.iMaxMetas);
		iRet = TDR_ERRIMPLE_INVALID_METALIB_PARAM;
	}
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_GET_FREE_META_SPACE(a_pstLib) < stLibParam.iMetaSize))
	{
		fprintf(a_fpError, "[tdr_add_meta] 错误：自定义类型存储空间不足，剩余%d，需%d\n",
			TDR_GET_FREE_META_SPACE(a_pstLib), stLibParam.iMetaSize);
		iRet = TDR_ERRIMPLE_INVALID_METALIB_PARAM;
	}
	if (!TDR_ERR_IS_ERROR(iRet) && (a_pstLib->iFreeStrBufSize < stLibParam.iStrBufSize))
	{
		fprintf(a_fpError, "[tdr_add_meta] 错误：字符串缓冲区不足，剩余%d，需%d\n",
			a_pstLib->iFreeStrBufSize, stLibParam.iStrBufSize);
		iRet = TDR_ERRIMPLE_INVALID_METALIB_PARAM;
	}

	/*将XML元素树中的信息加到metalib中*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		a_pstLib->iXMLTagSetVer = stLibParam.iTagSetVersion;
		fprintf(a_fpError, "[tdr_add_meta] 开始加载自定义类型到元数据库（当前已加载：%d个）...\n",
			a_pstLib->iCurMetaNum);
		iRet = tdr_add_matalib_i(a_pstLib, pstTree, a_fpError);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			fprintf(a_fpError, "[tdr_add_meta] 错误：加载自定义类型失败，返回码：%d\n", iRet);
			scew_tree_free(pstTree);
			fclose(logFile); // 关闭日志文件
			return iRet;
		}
		// 计算实际新增的自定义类型数量
		int iAddedMetaCount = a_pstLib->iCurMetaNum - iInitialMetaCount;
		fprintf(a_fpError, "[tdr_add_meta] 自定义类型加载完成，新增：%d个，当前总数量：%d个\n",
			iAddedMetaCount, a_pstLib->iCurMetaNum);
	}

	/*释放资源*/
	scew_tree_free(pstTree);
	fprintf(a_fpError, "[tdr_add_meta] 处理结束，返回码：%d\n", iRet);

	// 关闭日志文件
	fclose(logFile);
	return iRet;
}

int tdr_add_meta_file(INOUT LPTDRMETALIB a_pstLib, IN const char* a_pszFileName, IN int a_iXMLTagVersion, IN FILE* a_fpError)
{
    int iRet = TDR_SUCCESS;
    scew_tree* pstTree = NULL;
    TDRLIBPARAM stLibParam;
    
    /*assert(NULL != a_pstLib);
    assert(NULL != a_pszFileName);
    assert(NULL != a_fpError);  */
	if ((NULL == a_pstLib)||(NULL == a_pszFileName))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}
	if (NULL == a_fpError)
	{
		a_fpError = stdout;
	}
    
    
    /*创建XML元素树*/
    iRet = tdr_create_XMLParser_tree_byFileName(&pstTree, a_pszFileName, a_fpError);   
    
	/*获取创建元数据库所需的参数*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		tdr_init_lib_param_i(&stLibParam);
		TDR_STRNCPY(stLibParam.szName, a_pstLib->szName, sizeof(stLibParam.szName));
		iRet = tdr_add_lib_param_i(&stLibParam, pstTree, a_iXMLTagVersion, a_fpError);
	}

	/*检查空间*/
	if ((a_pstLib->iMaxMacroNum - a_pstLib->iCurMacroNum) < stLibParam.iMaxMacros)
	{
		fprintf(a_fpError, "\nerror: 元素数据库最多能存储%d个宏定义，目前已经存取了%d个定义,不能再加入%d个宏定义，请确定metalib初始化时分配了足够的空间\n",
			a_pstLib->iMaxMacroNum, a_pstLib->iCurMacroNum, stLibParam.iMaxMacros);
		iRet = TDR_ERRIMPLE_INVALID_METALIB_PARAM;
	}
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		if ((a_pstLib->iMaxMetaNum - a_pstLib->iCurMetaNum) < stLibParam.iMaxMetas)
		{
			fprintf(a_fpError, "\nerror: 元素数据库最多能存储%d个自定义类型，目前已经存取了%d个,不能再加入%d个，请确定metalib初始化时分配了足够的空间\n",
				a_pstLib->iMaxMetaNum, a_pstLib->iCurMetaNum, stLibParam.iMaxMetas);
			iRet = TDR_ERRIMPLE_INVALID_METALIB_PARAM;
		}
	}
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		if (TDR_GET_FREE_META_SPACE(a_pstLib) < stLibParam.iMetaSize)
		{
			fprintf(a_fpError, "\nerror: 元素数据库存储自定义类型的剩余空间为%d字节，目前实际需要%d字节，请确定metalib初始化时分配了足够的空间\n",
				TDR_GET_FREE_META_SPACE(a_pstLib), stLibParam.iMetaSize);
			iRet = TDR_ERRIMPLE_INVALID_METALIB_PARAM;
		}
	}
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		if (a_pstLib->iFreeStrBufSize < stLibParam.iStrBufSize)
		{
			fprintf(a_fpError, "\nerror: 元素数据库剩余空间字符串缓冲区为%d字节，目前实际需要%d字节，请确定metalib初始化时分配了足够的空间\n",
				a_pstLib->iFreeStrBufSize, stLibParam.iStrBufSize);
			iRet = TDR_ERRIMPLE_INVALID_METALIB_PARAM;
		}
	}


	/*将XML元素树中的信息加到metalib中*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		a_pstLib->iXMLTagSetVer = stLibParam.iTagSetVersion;
		iRet = tdr_add_matalib_i(a_pstLib, pstTree, a_fpError);   
	}      

	/*释放资源*/    
	scew_tree_free( pstTree );   
    
    return iRet;   
}

int tdr_save_xml(IN LPTDRMETALIB a_pstLib, IN char *a_pszXML, INOUT int *a_iXML)
{
    int iRet = TDR_SUCCESS;
    int i =0 ;
	TDRIOSTREAM stIOStream;


    /*assert(NULL != a_pstLib);
    assert(NULL != a_pszXML);
    assert(NULL != a_iXML);
    assert(0 < *a_iXML);*/
	if ((NULL == a_pstLib)||(NULL == a_pszXML)||
		(NULL == a_iXML)||(0 >= *a_iXML))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	stIOStream.emIOStreamType = TDR_IOSTREAM_STRBUF;
	stIOStream.pszTDRIOBuff = a_pszXML;
	stIOStream.iTDRIOBuffLen = *a_iXML;
   

    /*<metalib ... >*/
    iRet = tdr_save_metalib_header_i(a_pstLib, &stIOStream);
   
   
    /* <macro ...>*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_save_all_macros_i(a_pstLib, &stIOStream);
		
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/

	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_iostream_write(&stIOStream, "\n");
	}

    /*<union struct>*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        for (i = 0; i < a_pstLib->iCurMetaNum; i++)
        {
            LPTDRMETA pstMeta = TDR_IDX_TO_META(a_pstLib, i);

            if (TDR_TYPE_STRUCT == pstMeta->iType)
            {
                iRet = tdr_save_struct_meta_i(pstMeta, &stIOStream);  
            }else if (TDR_TYPE_UNION == pstMeta->iType)
            {
                iRet = tdr_save_union_meta_i(pstMeta, &stIOStream);  
            }
           
            /*移动指针*/
            if (TDR_ERR_IS_ERROR(iRet))
            {
                break;
            }          
        }/*for (i = 0; i < a_pstLib->iCurMetaNum; i++)*/ 
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/

    /*metalib end*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_iostream_write(&stIOStream, "</%s>\n", TDR_TAG_METALIB);
    }

    /*ok 计算实际写入长度*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        *a_iXML = stIOStream.pszTDRIOBuff - a_pszXML;
    }
    
    return iRet;
}

int tdr_save_metalib_header_i(IN LPTDRMETALIB a_pstLib, LPTDRIOSTREAM a_pstIOStream)
{
    int iRet = TDR_SUCCESS;
  

    assert(NULL != a_pstLib);
    assert(NULL != a_pstIOStream);
  
	iRet = tdr_iostream_write(a_pstIOStream,"<?xml version=\"1.0\" encoding=\"GBK\" standalone=\"yes\" ?>\n");

    /*metalib*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_iostream_write(a_pstIOStream, "<%s", TDR_TAG_METALIB);
    }
    
    /*tagsetversion*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
		int iVersion = a_pstLib->iXMLTagSetVer;

		if (TDR_XML_TAGSET_VERSION_0 == a_pstLib->iXMLTagSetVer)
		{/*强制将旧版本转换成最新版本*/
			iVersion = TDR_SUPPORTING_MAX_XMLTAGSET_VERSION;
		}
        iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_TAGSET_VERSION, iVersion);
    }
    
    /*name*/
    if (!TDR_ERR_IS_ERROR(iRet) && ('\0' != a_pstLib->szName[0]))
    {
        iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_NAME, a_pstLib->szName);
    }


    /*version*/
    if (!TDR_ERR_IS_ERROR(iRet) )
    {
        iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_VERSION, a_pstLib->lVersion);
    }

    /*id*/
    if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_ID != a_pstLib->iID))
    {
        iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_ID, a_pstLib->iID);
    }

    /*>*/
    if (!TDR_ERR_IS_ERROR(iRet) )
    {
        iRet = tdr_iostream_write(a_pstIOStream, " >\n");
    }

    return iRet;
}

int tdr_save_all_macros_i(IN LPTDRMETALIB a_pstLib, INOUT LPTDRIOSTREAM a_pstIOStream)
{
	int iRet = TDR_SUCCESS;
	TDRBOOLEAN *pastIsGroupNember;
	int i,j;
	LPTDRMAPENTRY pstMap;
	LPTDRMACROSGROUP pstGroup;
	TDRIDX *pValueTable;
	LPTDRMACRO pstMacroTable;
	LPTDRMACRO pstMacro;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstIOStream);
	
	if (0 >= a_pstLib->iCurMacroNum)
	{
		return TDR_SUCCESS;
	}

	/*建立macro与macrosgroup映射*/
	pastIsGroupNember = (TDRBOOLEAN  *)malloc(sizeof(TDRBOOLEAN)*(a_pstLib->iCurMacroNum));
	if (NULL == pastIsGroupNember)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
	}
	for (i = 0; i < a_pstLib->iCurMacroNum; i++)
	{
		pastIsGroupNember[i] = TDR_FALSE;
	}
	pstMap = TDR_GET_MACROSGROUP_MAP_TABLE(a_pstLib);
	for (i = 0; i < a_pstLib->iCurMacrosGroupNum; i++)
	{
		pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, pstMap[i].iPtr);
		pValueTable = TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(pstGroup);
		for (j = 0; j < pstGroup->iCurMacroCount; j++)
		{
			pastIsGroupNember[pValueTable[j]] = TDR_TRUE;	
		}
	}/*for (i = 0; i < a_pstLib->iCurMacrosGroupNum; i++)*/

	/*先输出不属于任何宏定义组的宏*/
	pstMacroTable = TDR_GET_MACRO_TABLE(a_pstLib);
	for (i = 0; i < a_pstLib->iCurMacroNum; i++)
	{
		if (TDR_FALSE == pastIsGroupNember[i])
		{
			pstMacro = pstMacroTable + i;
			iRet = tdr_save_macro_i(a_pstLib, pstMacro, a_pstIOStream);
		}
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}
	}/*for (i = 0; i < a_pstLib->iCurMacroNum; i++)*/

	/*输出宏定义组中的宏*/
	for (i = 0; i < a_pstLib->iCurMacrosGroupNum; i++)
	{
		pstGroup = TDR_PTR_TO_MACROSGROUP(a_pstLib, pstMap[i].iPtr);

		/*macros group的属性*/
		iRet = tdr_iostream_write(a_pstIOStream, "\t<%s", TDR_TAG_MACROSGROUP);		
		if (!TDR_ERR_IS_ERROR(iRet))
		{/*name*/
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_NAME, pstGroup->szName);
		}		
		if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_PTR != pstGroup->ptrDesc))
		{	/*desc*/
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_DESCIPTION,
				TDR_GET_STRING_BY_PTR(a_pstLib, pstGroup->ptrDesc));
		}		
		if (!TDR_ERR_IS_ERROR(iRet))
		{/*/>*/
			iRet = tdr_iostream_write(a_pstIOStream, " />\n", TDR_TAG_MACRO_VALUE);
		}

		/*输出宏定义*/
		pValueTable = TDR_GET_MACROSGROUP_VALUEIDXMAP_TAB(pstGroup);
		for (j = 0; j < pstGroup->iCurMacroCount; j++)
		{
			pstMacro = pstMacroTable + pValueTable[j];
			iRet = tdr_iostream_write(a_pstIOStream, "\t");
			iRet = tdr_save_macro_i(a_pstLib, pstMacro, a_pstIOStream);	
		}

		iRet = tdr_iostream_write(a_pstIOStream, "\t</%s>\n", TDR_TAG_MACROSGROUP);
	}/*for (i = 0; i < a_pstLib->iCurMacrosGroupNum; i++)*/

	return iRet;
}

int tdr_save_macro_i(LPTDRMETALIB a_pstLib, LPTDRMACRO a_pstMacro, LPTDRIOSTREAM a_pstIOStream)
{
    int iRet = TDR_SUCCESS;  

    assert(NULL != a_pstMacro);
    assert(NULL != a_pstIOStream);
	assert(NULL != a_pstLib);
  

    /*<macro*/
    iRet = tdr_iostream_write(a_pstIOStream, "\t<%s", TDR_TAG_MACRO);
    
    /*name*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_NAME, a_pstMacro->szMacro);
    }

    /*value*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_MACRO_VALUE, a_pstMacro->iValue);
    }

	/*desc*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_PTR != a_pstMacro->ptrDesc))
	{
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_DESCIPTION, TDR_GET_STRING_BY_PTR(a_pstLib, a_pstMacro->ptrDesc));
	}

    /*/>*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_iostream_write(a_pstIOStream, " />\n", TDR_TAG_MACRO_VALUE);
    }
   

    return iRet;
}

int tdr_save_struct_meta_i(LPTDRMETA a_pstMeta, LPTDRIOSTREAM a_pstIOStream)
{
    int iRet = TDR_SUCCESS;  
    LPTDRMACRO pstMacroTable = NULL;
    LPTDRMETALIB pstLib = NULL;
	int i;

    assert(NULL != a_pstMeta);
    assert(NULL != a_pstIOStream);
   
    pstLib = TDR_META_TO_LIB(a_pstMeta);
    pstMacroTable = TDR_GET_MACRO_TABLE(pstLib);

    /*<struct*/
    iRet = tdr_iostream_write(a_pstIOStream, "\t<%s", TDR_TAG_STRUCT);   

	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_save_meta_common_attribute_i(a_pstMeta, a_pstIOStream);
	}   

    /*size*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {   
		if ((TDR_INVALID_INDEX != a_pstMeta->idxCustomHUnitSize) && (a_pstMeta->idxCustomHUnitSize < pstLib->iCurMacroNum))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_SIZE, 
				pstMacroTable[a_pstMeta->idxCustomHUnitSize].szMacro);	  
		}else if (0 < a_pstMeta->iCustomHUnitSize)
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_SIZE, 
				a_pstMeta->iCustomHUnitSize);  
		}      
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/

    /*align*/
    if (!TDR_ERR_IS_ERROR(iRet) && (TDR_DEFAULT_ALIGN_VALUE != a_pstMeta->iCustomAlign) && (0 < a_pstMeta->iCustomAlign))
    {        
        iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_ALIGN, a_pstMeta->iCustomAlign);
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/

    /*versionindicator*/
    if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_OFFSET != a_pstMeta->stVersionIndicator.iNOff))
    {        
		char szPath[1024] = {0};

		iRet = tdr_netoff_to_path_i(a_pstMeta, -1, a_pstMeta->stVersionIndicator.iNOff, &szPath[0], sizeof(szPath));
		if (!TDR_ERR_IS_ERROR(iRet))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_VERSIONINDICATOR, szPath);
		}        
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/  

	/*sizeinfo*/
	if (!TDR_ERR_IS_ERROR(iRet) && (a_pstMeta->stSizeType.iUnitSize > 0))
	{        
		if (TDR_INVALID_INDEX != a_pstMeta->stSizeType.idxSizeType)
		{
			LPTDRCTYPEINFO pstTypeInfo = tdr_idx_to_typeinfo(a_pstMeta->stSizeType.idxSizeType);

			if (NULL != pstTypeInfo)
			{
				iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_SIZEINFO, pstTypeInfo->pszName);
			}
		}else if (TDR_INVALID_OFFSET != a_pstMeta->stSizeType.iNOff)
		{
			char szPath[1024] = {0};

			iRet = tdr_netoff_to_path_i(a_pstMeta, -1, a_pstMeta->stSizeType.iNOff, &szPath[0], sizeof(szPath));
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_SIZEINFO, szPath);
			}   
		}/*if (TDR_INVALID_INDEX != a_pstMeta->idxSizeType)*/		     
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*sortkey*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_OFFSET != a_pstMeta->stSortKey.iSortKeyOff))
	{        
		char szPath[1024] = {0};
		LPTDRMETA pstSortKeyMeta;
		LPTDRMETAENTRY pstSortKeyEntry;

		pstSortKeyMeta = TDR_PTR_TO_META(pstLib, a_pstMeta->stSortKey.ptrSortKeyMeta);
		pstSortKeyEntry = pstSortKeyMeta->stEntries + a_pstMeta->stSortKey.idxSortEntry;
		iRet = tdr_sortkeyinfo_to_path_i(pstLib, &a_pstMeta->stSortKey,
			&szPath[0], sizeof(szPath));
		if (!TDR_ERR_IS_ERROR(iRet))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_SORTKEY, szPath);
		}        
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*primary key*/
	if (!TDR_ERR_IS_ERROR(iRet) && (0 < a_pstMeta->nPrimayKeyMemberNum) && 
		(TDR_INVALID_PTR != a_pstMeta->ptrPrimayKeyBase) && (TDR_INVALID_PTR == a_pstMeta->ptrDependonStruct))
	{
		LPTDRDBKEYINFO pstDBKey = TDR_GET_PRIMARYBASEPTR(a_pstMeta);
		char szPath[1024] = {0};

		iRet = tdr_netoff_to_path_i(a_pstMeta, -1, pstDBKey->iHOff, &szPath[0], sizeof(szPath));
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s", TDR_TAG_PRIMARY_KEY, szPath);
		for (i = 1; i < a_pstMeta->nPrimayKeyMemberNum; i++ )
		{
			pstDBKey++;
			iRet = tdr_netoff_to_path_i(a_pstMeta, -1, pstDBKey->iHOff, &szPath[0], sizeof(szPath));
			iRet = tdr_iostream_write(a_pstIOStream, ",%s", szPath);
		}
		iRet = tdr_iostream_write(a_pstIOStream, "\"");
	}

	/*splittablefactor*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{   
		if ((TDR_INVALID_INDEX != a_pstMeta->idxSplitTableFactor) && (a_pstMeta->idxSplitTableFactor < pstLib->iCurMacroNum))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_SPLITTABLEFACTOR, 
				pstMacroTable[a_pstMeta->idxSplitTableFactor].szMacro);	  
		}else if (0 < a_pstMeta->iSplitTableFactor)
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_SPLITTABLEFACTOR, a_pstMeta->iSplitTableFactor);  
		}      
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*splittablekey*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_OFFSET != a_pstMeta->stSplitTableKey.iHOff) && 
		(0 < a_pstMeta->iSplitTableFactor))
	{   
		char szPath[1024] = {0};

		iRet = tdr_hostoff_to_path_i(a_pstMeta, -1, a_pstMeta->stSplitTableKey.iHOff, &szPath[0], sizeof(szPath));
		if (!TDR_ERR_IS_ERROR(iRet))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_SPLITTABLEKEY, szPath);
		}     
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*splittableRule*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_SPLITTABLE_RULE_NONE != a_pstMeta->nSplitTableRuleID))
	{   
		if (TDR_SPLITTABLE_RULE_BY_MOD == a_pstMeta->nSplitTableRuleID)
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_SPLITTABLERULE, TDR_TAG_SPLITTABLERULE_BY_MOD);
		}		
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*dependontable*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_PTR != a_pstMeta->ptrDependonStruct))
	{   
		LPTDRMETA pstDepend = TDR_PTR_TO_META(pstLib, a_pstMeta->ptrDependonStruct);
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_DEPENDONSTRUCT, pstDepend->szName);
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*uniqueentryname*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_META_DO_NEED_PREFIX(a_pstMeta)))
	{   
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_UNIQUEENTRYNAME, TDR_TAG_FALSE);
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/


	iRet = tdr_iostream_write(a_pstIOStream, " >\n");

	/*entry*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		int i;

		for (i = 0; i < a_pstMeta->iEntriesNum; i++)
		{
			iRet = tdr_save_meta_entry_i(a_pstMeta, i, a_pstIOStream);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}  
		}/*for (i = 0; i < a_pstMeta->iEntriesNum; i++)*/
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*</struct>*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_iostream_write(a_pstIOStream, "\t</%s>\n\n", TDR_TAG_STRUCT);
	}
	
	
	return iRet;
}

int tdr_save_union_meta_i(LPTDRMETA a_pstMeta, LPTDRIOSTREAM a_pstIOStream)
{
	int iRet = TDR_SUCCESS;	
	LPTDRMACRO pstMacroTable = NULL;
	LPTDRMETALIB pstLib = NULL;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstIOStream);
	
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstMacroTable = TDR_GET_MACRO_TABLE(pstLib);

	/*<union*/
	iRet = tdr_iostream_write(a_pstIOStream, "\t<%s", TDR_TAG_UNION);   

	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_save_meta_common_attribute_i(a_pstMeta, a_pstIOStream);	
	} 	
	
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_iostream_write(a_pstIOStream, " >\n");
	}
	


	/*entry*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		int i;

		for (i = 0; i < a_pstMeta->iEntriesNum; i++)
		{
			iRet = tdr_save_meta_entry_i(a_pstMeta, i, a_pstIOStream);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
		}/*for (i = 0; i < a_pstMeta->iEntriesNum; i++)*/
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*</union>*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_iostream_write(a_pstIOStream, "\t</%s>\n\n", TDR_TAG_UNION);
	}	

	return iRet;
}

int tdr_save_meta_common_attribute_i(LPTDRMETA a_pstMeta, LPTDRIOSTREAM a_pstIOStream)
{
	int iRet = TDR_SUCCESS;
	LPTDRMACRO pstMacroTable = NULL;
	LPTDRMETALIB pstLib = NULL;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstIOStream);
	
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstMacroTable = TDR_GET_MACRO_TABLE(pstLib);

	/*name*/
	if ('\0' != a_pstMeta->szName[0])
	{
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_NAME, a_pstMeta->szName);  
	}

	/*version*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		if ((TDR_INVALID_INDEX != a_pstMeta->idxVersion) && (a_pstMeta->idxVersion < pstLib->iCurMacroNum))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_VERSION, 
				pstMacroTable[a_pstMeta->idxVersion].szMacro);   
		}else 
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_VERSION, 
				a_pstMeta->iBaseVersion);   
		}
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/



	/*id*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_META_DO_HAVE_ID(a_pstMeta)))
	{
		if ((TDR_INVALID_INDEX != a_pstMeta->idxID) && (a_pstMeta->idxID < pstLib->iCurMacroNum))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_ID, 
				pstMacroTable[a_pstMeta->idxID].szMacro);   
		}else 
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_ID, a_pstMeta->iID);  
		}
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*cname*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_PTR != a_pstMeta->ptrChineseName))
	{
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_CNNAME, TDR_GET_STRING_BY_PTR(pstLib, a_pstMeta->ptrChineseName));
	}

	/*desc*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_PTR != a_pstMeta->ptrDesc))
	{
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_DESCIPTION, TDR_GET_STRING_BY_PTR(pstLib, a_pstMeta->ptrDesc));
	}
	

	return iRet;
}

int tdr_save_meta_entry_i(LPTDRMETA a_pstMeta, int a_idxEntry, LPTDRIOSTREAM a_pstIOStream)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib = NULL;
	LPTDRMACRO pstMacroTable = NULL;
	LPTDRMETAENTRY pstEntry = NULL;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstIOStream);	
	assert((0 <= a_idxEntry) && (a_idxEntry < a_pstMeta->iEntriesNum));
	
	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstMacroTable = TDR_GET_MACRO_TABLE(pstLib);
	pstEntry = a_pstMeta->stEntries + a_idxEntry;

	/*<entry */
	iRet = tdr_iostream_write(a_pstIOStream, "\t\t<%s", TDR_TAG_ENTRY);

	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_save_entry_base_attribute_i(pstEntry, a_pstMeta, a_pstIOStream);	
	}	

	/*unique*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_ENTRY_IS_UNIQUE(pstEntry)))
	{
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_UNIQUE, TDR_TAG_TRUE);
	}

	/*notnull*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_ENTRY_IS_NOT_NULL(pstEntry)))
	{
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_NOTNULL, TDR_TAG_TRUE);
	}	

	/*refer*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_OFFSET != pstEntry->stRefer.iHOff))
	{        
		char szPath[1024] = {0};

		iRet = tdr_hostoff_to_path_i(a_pstMeta, a_idxEntry, pstEntry->stRefer.iHOff, 
			&szPath[0], sizeof(szPath));
		if (!TDR_ERR_IS_ERROR(iRet))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_REFER, szPath);
		}        
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/ 

	/*default*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_PTR != pstEntry->ptrDefaultVal))
	{
		iRet = tdr_save_entry_defaultval_i(a_pstIOStream, pstLib, pstEntry);
	}

	/*sizeinfo*/
	if (!TDR_ERR_IS_ERROR(iRet) && (0 < pstEntry->stSizeInfo.iUnitSize))
	{   
		if (TDR_INVALID_INDEX != pstEntry->stSizeInfo.idxSizeType)
		{
			LPTDRCTYPEINFO pstTypeInfo = tdr_idx_to_typeinfo(pstEntry->stSizeInfo.idxSizeType);
			if ((NULL != pstTypeInfo) && 
				((TDR_TYPE_STRING != pstEntry->iType && TDR_TYPE_WSTRING != pstEntry->iType)|| (0 != tdr_stricmp(pstTypeInfo->pszName, TDR_STRING_DEFULT_SIZEINFO_VALUE))))
			{
				iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_SIZEINFO, pstTypeInfo->pszName);
			}
		}else if (TDR_INVALID_OFFSET != pstEntry->stSizeInfo.iNOff)
		{
			char szPath[1024] = {0};

			iRet = tdr_netoff_to_path_i(a_pstMeta, a_idxEntry, pstEntry->stSizeInfo.iNOff, &szPath[0], sizeof(szPath));
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_SIZEINFO, szPath);
			}      
		}/*if (TDR_INVALID_INDEX != pstEntry->stSizeInfo.
		 idxSizeType)*/			
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/ 

	

	/*sortMethod for array*/
	if (!TDR_ERR_IS_ERROR(iRet) && (1 != pstEntry->iCount) && (0 <= pstEntry->iCount))
	{   
		const char *pszVal = NULL;

		if (TDR_SORTMETHOD_ASC_SORT == pstEntry->chOrder)
		{
			pszVal = TDR_TAG_ASC;
		}else if (TDR_SORTMETHOD_DSC_SORT == pstEntry->chOrder)
		{
			pszVal = TDR_TAG_DESC;
		}

		if (NULL != pszVal)
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_SORTMETHOD, pszVal);
		}		      
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/ 

	

	/*io*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_IO_NOLIMIT != pstEntry->iIO))
	{
		switch(pstEntry->iIO)
		{
		case TDR_IO_NOINPUT:
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_IO, TDR_TAG_IO_NO_INPUT);  
			break;
		case TDR_IO_NOOUTPUT:
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_IO, TDR_TAG_IO_NO_OUTPUT);  
			break;
		case TDR_IO_NOIO:
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_IO, TDR_TAG_IO_NO_IO);  
			break;
		default:
			break;
		}		
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/


	/*union select */
	if (!TDR_ERR_IS_ERROR(iRet) )
	{   
		iRet = tdr_save_entry_union_attribute(a_pstMeta, a_idxEntry, a_pstIOStream);				       
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/ 

  


	/*extendtotable */
	if (!TDR_ERR_IS_ERROR(iRet) && TDR_ENTRY_DO_EXTENDABLE(pstEntry) )
	{   
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_EXTENDTOTABLE, TDR_TAG_TRUE); 				       
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/ 


	/*bindmacrosgroup*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_PTR != pstEntry->ptrMacrosGroup))
	{
		LPTDRMACROSGROUP pstGroup = TDR_PTR_TO_MACROSGROUP(pstLib, pstEntry->ptrMacrosGroup);
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_BIND_MACROSGROUP, pstGroup->szName); 
	}

	/*autoincrement*/
	if (!TDR_ERR_IS_ERROR(iRet) && TDR_ENTRY_IS_AUTOINCREMENT(pstEntry))
	{
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_AUTOINCREMENT, TDR_TAG_TRUE); 
	}

	/*customattr*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_PTR != pstEntry->ptrCustomAttr))
	{
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_CUSTOMATTR, 
			TDR_GET_STRING_BY_PTR(pstLib, pstEntry->ptrCustomAttr)); 
	}

	/*/>*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_iostream_write(a_pstIOStream, " />\n");
	}	


	return iRet;
}

int tdr_save_entry_base_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, LPTDRIOSTREAM a_pstIOStream)
{
	int iRet = TDR_SUCCESS;	
	LPTDRMETALIB pstLib = NULL;
	LPTDRMACRO pstMacroTable = NULL;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstIOStream);	
	assert(NULL != a_pstEntry);

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstMacroTable = TDR_GET_MACRO_TABLE(pstLib);

	/*name*/
	if ('\0' != a_pstEntry->szName[0])
	{
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_NAME, a_pstEntry->szName); 
	}

	/*type*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		char *pszType = NULL;
		char szTypePrefix[4] = {0};

		if (TDR_INVALID_PTR != a_pstEntry->ptrMeta)
		{
			LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, a_pstEntry->ptrMeta);

			pszType = pstTypeMeta->szName;
		}else if (TDR_INVALID_INDEX != a_pstEntry->idxType)
		{
			LPTDRCTYPEINFO pstTypeInfo = tdr_idx_to_typeinfo(a_pstEntry->idxType);
			if (NULL != pstTypeInfo)
			{
				pszType = pstTypeInfo->pszName;
			}
		}/*if (TDR_INVALID_PTR != a_pstEntry->ptrMeta)*/	

		if (TDR_ENTRY_IS_POINTER_TYPE(a_pstEntry))
		{
			tdr_snprintf(szTypePrefix, sizeof(szTypePrefix), "%c", TDR_TAG_POINTER_TYPE);
		}else if (TDR_ENTRY_IS_REFER_TYPE(a_pstEntry))
		{
			tdr_snprintf(szTypePrefix, sizeof(szTypePrefix), "%c", TDR_TAG_REFER_TYPE);
		}

		if (NULL != pszType)
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s%s\"", TDR_TAG_TYPE, szTypePrefix, pszType);
		}		
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*count*/
	if (!TDR_ERR_IS_ERROR(iRet) && (1 != a_pstEntry->iCount) && (0 <= a_pstEntry->iCount))
	{
		if ((TDR_INVALID_INDEX != a_pstEntry->idxCount) && (a_pstEntry->idxCount < pstLib->iCurMacroNum))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_COUNT, 
				pstMacroTable[a_pstEntry->idxCount].szMacro);   
		}else 
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_COUNT, a_pstEntry->iCount);
		}
	}

	/*version*/
	if (!TDR_ERR_IS_ERROR(iRet) && (a_pstEntry->iVersion != a_pstMeta->iBaseVersion))
	{
		if ((TDR_INVALID_INDEX != a_pstEntry->idxVersion) && (a_pstEntry->idxVersion < pstLib->iCurMacroNum))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_VERSION, 
				pstMacroTable[a_pstEntry->idxVersion].szMacro);  
		}else if (TDR_INVALID_VERSION != a_pstEntry->iVersion)
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_VERSION, 
				a_pstEntry->iVersion);  
		}
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*id*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_ENTRY_DO_HAVE_ID(a_pstEntry)))
	{
		if ((TDR_INVALID_INDEX != a_pstEntry->idxID) && (a_pstEntry->idxID < pstLib->iCurMacroNum))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_ID, 
				pstMacroTable[a_pstEntry->idxID].szMacro);  
		}else 
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_ID, a_pstEntry->iID);  
		}
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*size*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{   
		if ((TDR_INVALID_INDEX != a_pstEntry->idxCustomHUnitSize) && (a_pstEntry->idxCustomHUnitSize < pstLib->iCurMacroNum))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_SIZE, 
				pstMacroTable[a_pstEntry->idxCustomHUnitSize].szMacro);  
		}else if (0 < a_pstEntry->iCustomHUnitSize)
		{
			LPTDRCTYPEINFO pstTypeInfo = tdr_idx_to_typeinfo(a_pstEntry->idxType);
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_SIZE, 
				a_pstEntry->iCustomHUnitSize/pstTypeInfo->iSize);   
		}      
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*cname*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_PTR != a_pstEntry->ptrChineseName))
	{
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_CNNAME, TDR_GET_STRING_BY_PTR(pstLib, a_pstEntry->ptrChineseName));
	}

	/*desc*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_PTR != a_pstEntry->ptrDesc))
	{
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_DESCIPTION, TDR_GET_STRING_BY_PTR(pstLib, a_pstEntry->ptrDesc));
	}
	
	return iRet;
}

int tdr_save_entry_union_attribute(LPTDRMETA a_pstMeta, int a_idxEntry, LPTDRIOSTREAM a_pstIOStream)
{
	int iRet = TDR_SUCCESS;	
	LPTDRMETALIB pstLib = NULL;
	LPTDRMACRO pstMacroTable = NULL;
	LPTDRMETAENTRY pstEntry = NULL;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstIOStream);	
	assert((0 <= a_idxEntry) && (a_idxEntry < a_pstMeta->iEntriesNum));

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstMacroTable = TDR_GET_MACRO_TABLE(pstLib);
	pstEntry = a_pstMeta->stEntries + a_idxEntry;

	if ((TDR_TYPE_UNION == pstEntry->iType) && (TDR_INVALID_OFFSET != pstEntry->stSelector.iHOff))
	{
		char szPath[1024] = {0};

		iRet = tdr_hostoff_to_path_i(a_pstMeta, a_idxEntry, pstEntry->stSelector.iHOff, 
			&szPath[0], sizeof(szPath));
		if (!TDR_ERR_IS_ERROR(iRet))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_SELECT, szPath);
		}       
	}/*if (TDR_INVALID_OFFSET != pstEntry->iSelectOff)*/

	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_ENTRY_DO_HAVE_MAXMIN_ID(pstEntry)))
	{
		if (!TDR_ERR_IS_ERROR(iRet) && (TDR_INVALID_INDEX != pstEntry->iMinIdIdx) && (pstEntry->iMinIdIdx < pstLib->iCurMacroNum))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_MINID, 
				pstMacroTable[pstEntry->iMinIdIdx].szMacro);   
		}else 
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_MINID, 
				pstEntry->iMinId);   
		}

		if ((TDR_INVALID_INDEX != pstEntry->iMaxIdIdx) && (pstEntry->iMaxIdIdx < pstLib->iCurMacroNum))
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_MAXID, 
				pstMacroTable[pstEntry->iMaxIdIdx].szMacro);  
		}else 
		{
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_MAXID, 
				pstEntry->iMaxId);    
		} 		 

	}/*if (!TDR_ERR_IS_ERROR(iRet) && (TDR_ENTRY_DO_HAVE_MAXMIN_ID(pstEntry)))*/	

	return iRet;
}

int tdr_save_xml_fp(IN LPTDRMETALIB a_pstLib, OUT FILE* a_fp)
{
	TDRIOSTREAM stIOStream ;
	int iRet = TDR_SUCCESS;
	int i;

	/*assert(NULL != a_pstLib);
	assert(NULL != a_fp);*/
	if ((NULL == a_pstLib)||(NULL == a_fp))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	stIOStream.emIOStreamType = TDR_IOSTREAM_FILE;
	stIOStream.fpTDRIO = a_fp;

	/*<metalib ... >*/
	iRet = tdr_save_metalib_header_i(a_pstLib, &stIOStream);

	/* <macro ...>*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_save_all_macros_i(a_pstLib, &stIOStream);
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_iostream_write(&stIOStream, "\n");
	}

	/*<union struct>*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		for (i = 0; i < a_pstLib->iCurMetaNum; i++)
		{
			LPTDRMETA pstMeta = TDR_IDX_TO_META(a_pstLib, i);

			if (TDR_TYPE_STRUCT == pstMeta->iType)
			{
				iRet = tdr_save_struct_meta_i(pstMeta, &stIOStream);  
			}else if (TDR_TYPE_UNION == pstMeta->iType)
			{
				iRet = tdr_save_union_meta_i(pstMeta, &stIOStream);  
			}

			/*移动指针*/
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}          
		}/*for (i = 0; i < a_pstLib->iCurMetaNum; i++)*/ 
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*metalib end*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_iostream_write(&stIOStream, "</%s>\n", TDR_TAG_METALIB);
	}

	return iRet;
}

int tdr_save_xml_file(IN LPTDRMETALIB a_pstLib, IN const char* a_pszXmlFile)
{
	TDRIOSTREAM stIOStream ;
	int iRet = TDR_SUCCESS;
	FILE *fp = NULL;
	int i;

	//assert(NULL != a_pstLib);
	//assert(NULL != a_pszXmlFile);
	if ((NULL == a_pstLib)||(NULL == a_pszXmlFile))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	fp = fopen(a_pszXmlFile, "w");
	if (NULL == fp)
	{
		return TDR_ERRIMPLE_FAILED_OPEN_FILE_TO_WRITE;
	}

	stIOStream.emIOStreamType = TDR_IOSTREAM_FILE;
	stIOStream.fpTDRIO = fp;

	/*<metalib ... >*/
	iRet = tdr_save_metalib_header_i(a_pstLib, &stIOStream);

	/* <macro ...>*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_save_all_macros_i(a_pstLib, &stIOStream);
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_iostream_write(&stIOStream, "\n");
	}


	/*<union struct>*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		for (i = 0; i < a_pstLib->iCurMetaNum; i++)
		{
			LPTDRMETA pstMeta = TDR_IDX_TO_META(a_pstLib, i);

			if (TDR_TYPE_STRUCT == pstMeta->iType)
			{
				iRet = tdr_save_struct_meta_i(pstMeta, &stIOStream);  
			}else if (TDR_TYPE_UNION == pstMeta->iType)
			{
				iRet = tdr_save_union_meta_i(pstMeta, &stIOStream);  
			}

			/*移动指针*/
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}          
		}/*for (i = 0; i < a_pstLib->iCurMetaNum; i++)*/ 
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*metalib end*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_iostream_write(&stIOStream, "</%s>\n", TDR_TAG_METALIB);
	}

	fclose(fp);

	return iRet;
}

int tdr_save_entry_defaultval_i(LPTDRIOSTREAM a_pstIOStream, LPTDRMETALIB a_pstLib, LPTDRMETAENTRY a_pstEntry)
{
	int iRet = TDR_SUCCESS;
	char *pszDefault;

	assert(NULL != a_pstIOStream);
	assert(NULL != a_pstLib);
	assert(NULL != a_pstEntry);

	if (TDR_INVALID_PTR == a_pstEntry->ptrDefaultVal)
	{
		return TDR_SUCCESS;
	}

	pszDefault = TDR_GET_STRING_BY_PTR(a_pstLib, a_pstEntry->ptrDefaultVal);
	switch(a_pstEntry->iType)
	{
	case TDR_TYPE_STRING:
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_DEFAULT_VALUE, pszDefault);
		break;
	case TDR_TYPE_CHAR:
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_DEFAULT_VALUE, (int)pszDefault[0]);
		break;
	case TDR_TYPE_UCHAR:
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_DEFAULT_VALUE, (int)(unsigned char )pszDefault[0]);
		break;
	case TDR_TYPE_SMALLINT:
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_DEFAULT_VALUE,  (int)*(short *)pszDefault);
		break;
	case TDR_TYPE_SMALLUINT:
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_DEFAULT_VALUE, (int)*(unsigned short *)pszDefault);
		break;
	case TDR_TYPE_INT:
	case TDR_TYPE_LONG:
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%d\"", TDR_TAG_DEFAULT_VALUE,  (int)*(int *)pszDefault);
		break;
	case TDR_TYPE_UINT:
	case TDR_TYPE_ULONG:
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%u\"", TDR_TAG_DEFAULT_VALUE, (unsigned int)*(unsigned int *)pszDefault);
		break;
	case TDR_TYPE_LONGLONG:
		#if defined(WIN32) &&  _MSC_VER < 1400  /*vc7,vc6,,*/													
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%I64i\"", TDR_TAG_DEFAULT_VALUE,  (tdr_longlong)*(tdr_longlong *)pszDefault);
		#else
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%lld\"", TDR_TAG_DEFAULT_VALUE,  (tdr_longlong)*(tdr_longlong *)pszDefault);
		#endif																								
		
		break;
	case TDR_TYPE_ULONGLONG:
		#if defined(WIN32) &&  _MSC_VER < 1400  /*vc7,vc6,,*/													
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%I64u\"", TDR_TAG_DEFAULT_VALUE,  (tdr_ulonglong)*(tdr_ulonglong *)pszDefault);
		#else
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%llu\"", TDR_TAG_DEFAULT_VALUE,  (tdr_ulonglong)*(tdr_ulonglong *)pszDefault);
		#endif	
		break;
	case TDR_TYPE_FLOAT:
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%f\"", TDR_TAG_DEFAULT_VALUE, *(float *)pszDefault);
		break;
	case TDR_TYPE_DOUBLE:
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%f\"", TDR_TAG_DEFAULT_VALUE, *(double *)pszDefault);
		break;
	case TDR_TYPE_DATETIME:
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_DEFAULT_VALUE,
			tdr_tdrdatetime_to_str((tdr_datetime_t *)pszDefault));
		break;
	case TDR_TYPE_DATE:
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_DEFAULT_VALUE,
			tdr_tdrdate_to_str((tdr_date_t *)pszDefault));
		break;
	case TDR_TYPE_TIME:
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_DEFAULT_VALUE,
			tdr_tdrtime_to_str((tdr_time_t *)pszDefault));
		break;
	case TDR_TYPE_IP:
		iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_DEFAULT_VALUE,
			tdr_tdrip_to_ineta(*(tdr_ip_t *)pszDefault));
		break;
	case TDR_TYPE_WCHAR:
		{
			tdr_wchar_t szTemp[2] = {0};												
			char szMbs[4] = {0};														
			int iLen = sizeof(szMbs);	

			szTemp[0] = *(tdr_wchar_t *)pszDefault;	
			tdr_wcstochinesembs(&szMbs[0], &iLen, &szTemp[0], 2);
															
			iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_DEFAULT_VALUE, szMbs);			
																						
			break;
		}
	case TDR_TYPE_WSTRING:
		{
			char *pszMbs = tdr_wcstochinesembs_i((tdr_wchar_t *)pszDefault, a_pstEntry->iDefaultValLen/sizeof(tdr_wchar_t)); 
			if (NULL == pszMbs)
			{
				iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_DEFAULT_VALUE, "");
			}else
			{
				iRet = tdr_iostream_write(a_pstIOStream, " %s=\"%s\"", TDR_TAG_DEFAULT_VALUE, pszMbs);
			}
			break;
		}
	default:
		break;
	}

	return TDR_SUCCESS;
}


int tdr_add_matalib_i(TDRMETALIB* a_pstLib, scew_tree *a_pstTree, FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstTree);
	assert(NULL != a_fpError);

	if (TDR_XML_TAGSET_VERSION_0 == a_pstLib->iXMLTagSetVer)
	{
		iRet = tdr_add_metalib_Ver0_i(a_pstLib, a_pstTree, a_fpError);
	}else
	{
		iRet = tdr_add_metalib_Ver1_i(a_pstLib, a_pstTree, a_fpError);
	}

	/*做元数据库约束检查*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_ckeck_metalib_i(a_pstLib, a_fpError);
	}

	/*对metalib进行排序等优化*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		int i = 0;
		LPTDRIDENTRY pstID = NULL;
		LPTDRNAMEENTRY pstName = NULL;
		
		for (i = 0; i < a_pstLib->iCurMetaNum; i++)
		{
			LPTDRMETA pstMeta = TDR_IDX_TO_META(a_pstLib, i);

			/*union结构对成员进行排序*/
			if (TDR_TYPE_UNION == pstMeta->iType)
			{
				tdr_qsort(pstMeta->stEntries, pstMeta->iEntriesNum, sizeof(TDRMETAENTRY), tdr_union_entry_comp_i);
			}
		}        

		/* sort the id mapper. */
		
		pstID = TDR_GET_META_ID_MAP_TABLE(a_pstLib);
		tdr_qsort(pstID, a_pstLib->iCurMetaNum, sizeof(TDRIDENTRY), tdr_identry_comp_i);		
		/* sort the name mapper. */
		pstName = TDR_GET_META_NAME_MAP_TABLE(a_pstLib);
		tdr_qsort(pstName, a_pstLib->iCurMetaNum, sizeof(TDRNAMEENTRY), tdr_name_entry_comp_i);
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	

	return iRet;
}

int tdr_ckeck_metalib_i(TDRMETALIB* a_pstLib, FILE *a_fpError)
{
	int i,j;
	int iVersion;
	LPTDRMETAENTRY pstPreEntry = NULL;
	LPTDRMETA pstPreMeta = NULL;

	assert(NULL != a_pstLib);
	assert(NULL != a_fpError);


	iVersion = 0;
	for (i = 0; i < a_pstLib->iCurMetaNum; i++)
	{
		LPTDRMETA pstMeta = TDR_IDX_TO_META(a_pstLib,i);
		for (j = 0; j < pstMeta->iEntriesNum; j++)
		{
			/*更新结构体的当前版本*/
			int iTmpVer;
			LPTDRMETAENTRY pstEntry = pstMeta->stEntries + j;
			if (TDR_INVALID_PTR != pstEntry->ptrMeta)
			{
				LPTDRMETA pstTmpMeta = TDR_PTR_TO_META(a_pstLib, pstEntry->ptrMeta);
				iTmpVer = pstTmpMeta->iCurVersion;
			}else
			{
				iTmpVer = pstEntry->iVersion;
			}
			if (pstMeta->iCurVersion < iTmpVer)
			{
				pstMeta->iCurVersion = iTmpVer;
			}
			if (iVersion < iTmpVer)
			{
				iVersion = iTmpVer;
				pstPreEntry = pstEntry;
				pstPreMeta = pstMeta;
			}
		}/*for (j = 0; j < pstMeta->iEntriesNum; j++)*/
	}/*for (i = 0; i < a_pstLib->iCurMetaNum; i++)*/
		

	/*检查元数据库的版本:库版本不能小于meta和entry的版本*/
	if (TDR_XML_TAGSET_VERSION_0 < a_pstLib->iXMLTagSetVer)
	{
		if (a_pstLib->lVersion < iVersion)
		{
			fprintf(a_fpError, "Error: 按照元数据描述规则,元数据库的版本应该不低于此库中任何结构和成员的版本.但是结构<%s>的版本<%d>比库的版本<%ld>要大\n",
				pstPreMeta->szName, pstPreMeta->iCurVersion, a_pstLib->lVersion);
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_VERSION);
		}	
	} else 
	{
		a_pstLib->lVersion = iVersion;	
	}/*if (TDR_XML_TAGSET_VERSION_0 < a_pstLib->iXMLTagSetVer)*/
	
	
	return TDR_SUCCESS;
}

int tdr_add_metalib_Ver0_i(TDRMETALIB* a_pstLib, scew_tree *a_pstTree, FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_element* pstRoot = NULL;
	scew_element* pstSubElement = NULL;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstTree);
	assert(NULL != a_fpError);

	pstRoot = scew_tree_root(a_pstTree);
	assert(NULL != pstRoot);

	/*由于meta元素定义中要用到宏定义，因此先读取宏定义信息*/
	pstSubElement = scew_element_next(pstRoot, NULL);
	while( NULL != pstSubElement )
	{
		if (0 == tdr_stricmp( scew_element_name(pstSubElement), TDR_TAG_MACROS ))
		{
			iRet = tdr_add_macros_i( a_pstLib, pstSubElement, a_fpError);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
		}    

		pstSubElement =	scew_element_next( pstRoot, pstSubElement );
	}

	/*将meta的基本信息加到metalib中*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		pstSubElement = scew_element_next(pstRoot, NULL);
		while( NULL != pstSubElement )
		{
			/*添加stuct /union*/
			const char *szElementName = scew_element_name(pstSubElement);
			if (0 == tdr_stricmp( szElementName, TDR_TAG_TYPE ))           
			{
				iRet = tdr_add_meta_base_i( a_pstLib, pstSubElement, pstRoot, a_fpError);
				if (TDR_ERR_IS_ERROR(iRet))
				{
					break;
				}
			}            
			pstSubElement = scew_element_next( pstRoot, pstSubElement );
		} 
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*将meta的成员元素, 和其他属性加到metalib中*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		pstSubElement = scew_element_next(pstRoot, NULL);
		while( NULL != pstSubElement )
		{   
			char szMetaName[TDR_NAME_LEN] = {0};
			LPTDRMETA pstMeta = NULL;

			if (0 != tdr_stricmp( scew_element_name(pstSubElement), TDR_TAG_TYPE ))           
			{
				pstSubElement = scew_element_next( pstRoot, pstSubElement );    
				continue;
			}          


			/*检验pstMeta所指的结构与预处理的a_stElement对应, 目前仅检查名字是否相同*/
			strncpy(szMetaName,  "", sizeof(szMetaName));
			iRet = tdr_get_name_attribute_i(&szMetaName[0], sizeof(szMetaName), pstSubElement);
			if (TDR_SUCCESS != iRet)
			{
				fprintf(a_fpError, "error:\t 要处理的meta没有名字属性值\n");
				iRet = TDR_ERRIMPLE_META_NO_NAME;
				break;
			}
			pstMeta = tdr_get_meta_by_name_i(a_pstLib, szMetaName);
			if (NULL == pstMeta)
			{
				fprintf(a_fpError, "error:\t 元数据库中没有名字为<%s>的元数据\n", szMetaName);
				iRet = TDR_ERRIMPLE_META_NO_NAME;
				break;
			}

			/*添加成员*/
			iRet = tdr_add_meta_entries_i(pstMeta, pstSubElement, a_fpError);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

			/*读取meta的特殊属性*/
			iRet = tdr_get_meta_specail_attribute_i(pstMeta, pstSubElement, a_fpError);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

			/*检查这个结构的合法性*/
			iRet = tdr_check_meta_i(pstMeta, a_fpError);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

       
			pstSubElement = scew_element_next( pstRoot, pstSubElement );            
		} /*while( NULL == pstSubElement )*/
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/  


	return iRet;
}


int tdr_add_metalib_Ver1_i(TDRMETALIB* a_pstLib, scew_tree *a_pstTree, FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_element* pstRoot = NULL;
	scew_element* pstSubElement = NULL;
	LPTDRMACRO pstMacro = NULL;

	assert(NULL != a_pstLib);
	assert(NULL != a_pstTree);
	assert(NULL != a_fpError);

	pstRoot = scew_tree_root(a_pstTree);
	assert(NULL != pstRoot);

	/*由于meta元素定义中要用到宏定义，因此先读取宏定义信息*/
	pstSubElement = scew_element_next(pstRoot, NULL);
	while( NULL != pstSubElement )
	{
		if (0 == tdr_stricmp( scew_element_name(pstSubElement), TDR_TAG_MACRO ))
		{
			iRet = tdr_add_macro_i( a_pstLib, pstSubElement, a_fpError, &pstMacro);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
			pstSubElement =	scew_element_next( pstRoot, pstSubElement );
			continue;
		}   

		/*处理宏定义组*/
		if (0 == tdr_stricmp( scew_element_name(pstSubElement), TDR_TAG_MACROSGROUP ))
		{
			iRet = tdr_add_macrosgroup_i( a_pstLib, pstSubElement, a_fpError);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}
		}/*if (0 == tdr_stricmp( scew_element_name(pstSubElement), TDR_TAG_MACROSGROUP ))*/  

		pstSubElement =	scew_element_next( pstRoot, pstSubElement );
	}


	/*将meta的基本信息加到metalib中*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		pstSubElement = scew_element_next(pstRoot, NULL);
		while( NULL != pstSubElement )
		{
			const char *pszAttrName = scew_element_name(pstSubElement);
			/*添加stuct /union*/
			if (0 == strcmp(pszAttrName, TDR_TAG_STRUCT) || 
				(0 == strcmp(pszAttrName, TDR_TAG_UNION)))           
			{
				iRet = tdr_add_meta_base_i( a_pstLib, pstSubElement, pstRoot, a_fpError);
				if (TDR_ERR_IS_ERROR(iRet))
				{
					break;
				}
			}            
			pstSubElement = scew_element_next( pstRoot, pstSubElement );
		} 
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	/*将meta的成员元素, 和其他属性加到metalib中*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		pstSubElement = scew_element_next(pstRoot, NULL);
		while( NULL != pstSubElement )
		{   
			char szMetaName[TDR_NAME_LEN] = {0};
			LPTDRMETA pstMeta = NULL;
			const char *pszAttrName = scew_element_name(pstSubElement);

			if ((0 != tdr_stricmp( pszAttrName, TDR_TAG_STRUCT)) && 
				(0 != tdr_stricmp( pszAttrName, TDR_TAG_UNION)))           
			{
				pstSubElement = scew_element_next( pstRoot, pstSubElement );    
				continue;
			}            


			/*检验pstMeta所指的结构与预处理的a_stElement对应, 目前仅检查名字是否相同*/
			strncpy(szMetaName,  "", sizeof(szMetaName));
			iRet = tdr_get_name_attribute_i(&szMetaName[0], sizeof(szMetaName), pstSubElement);
			if (TDR_SUCCESS != iRet)
			{
				fprintf(a_fpError, "error:\t 要处理的meta没有名字属性值\n");
				iRet = TDR_ERRIMPLE_META_NO_NAME;
				break;
			}
			pstMeta = tdr_get_meta_by_name_i(a_pstLib, szMetaName);
			if (NULL == pstMeta)
			{
				fprintf(a_fpError, "error:\t 元数据库中没有名字为<%s>的元数据\n", szMetaName);
				iRet = TDR_ERRIMPLE_META_NO_NAME;
				break;
			}

			/*添加成员*/
			iRet = tdr_add_meta_entries_i(pstMeta, pstSubElement, a_fpError);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

			/*读取meta的特殊属性*/
			iRet = tdr_get_meta_specail_attribute_i(pstMeta, pstSubElement, a_fpError);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

			/*检查这个结构的合法性*/
			iRet = tdr_check_meta_i(pstMeta, a_fpError);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				break;
			}

          
			pstSubElement = scew_element_next( pstRoot, pstSubElement );            
		} /*while( NULL == pstSubElement )*/
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/   

	return iRet;
}

int tdr_union_entry_comp_i(const void* pv1, const void* pv2)
{
	LPTDRMETAENTRY pstEntry1;
	LPTDRMETAENTRY pstEntry2;

	pstEntry1 = (LPTDRMETAENTRY) pv1;
	pstEntry2 =	(LPTDRMETAENTRY) pv2;

	if (TDR_ENTRY_IS_VALID_SELECTID(pstEntry1) && (TDR_ENTRY_IS_VALID_SELECTID(pstEntry2)))
	{
		return pstEntry1->iMinId - pstEntry2->iMinId;
	}else if (!TDR_ENTRY_IS_VALID_SELECTID(pstEntry1) && (TDR_ENTRY_IS_VALID_SELECTID(pstEntry2)))
	{
		return -1;
	}else if (TDR_ENTRY_IS_VALID_SELECTID(pstEntry1) && (!TDR_ENTRY_IS_VALID_SELECTID(pstEntry2)))
	{
		return 1;
	}

	return 0;
}

int tdr_identry_comp_i(const void* pv1, const void* pv2)
{
	LPTDRIDENTRY pstID1;
	LPTDRIDENTRY pstID2;

	pstID1 = (LPTDRIDENTRY) pv1;
	pstID2 = (LPTDRIDENTRY) pv2;    

	return pstID1->iID - pstID2->iID;	
}


int tdr_name_entry_comp_i(const void* pv1, const void* pv2)
{
	LPTDRNAMEENTRY pstName1;
	LPTDRNAMEENTRY pstName2;

	pstName1 = (LPTDRNAMEENTRY) pv1;
	pstName2 = (LPTDRNAMEENTRY) pv2;

	return strcmp(pstName1->szName, pstName2->szName);
}

