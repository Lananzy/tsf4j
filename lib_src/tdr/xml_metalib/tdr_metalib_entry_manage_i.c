/**
*
* @file     tdr_metalib_entry_manage_i.h   
* @brief    TSF4G-元数据描述库entry元素管理模块
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-04-16 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <assert.h>
#include <stdlib.h>
#include <wchar.h>
#include "tdr/tdr_os.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_ctypes_info_i.h"
#include "tdr/tdr_ctypes_info.h"
#include "tdr_XMLtags_i.h"
#include "tdr/tdr_metalib_kernel_i.h"
#include "tdr_metalib_entry_manage_i.h"
#include "tdr_metalib_meta_manage_i.h"
#include "tdr/tdr_error_i.h"
#include "tdr/tdr_metalib_manage_i.h"
#include "tdr/tdr_auxtools.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

static void tdr_init_entry_data_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta);

/** 读取entry不需要计算偏移的公共属性: id version name cname desc
*@param[out] a_pstEntry 保存此元素信息的数据结构
*@param[in] a_pstMeta pstEntry所在的自定义结构
*@param[in] a_pstElement 要处理的元素
*@param[in] a_fpError 保存错误信息的文件句柄
*
*@return 成功返回TDR_SUCCESS，否则返回错误代码
*
*@pre \e a_pstEntry 不能为NULL
*@pre \e a_pstMeta 不能为NULL
*@pre \e a_pstElement 不能为NULL
*@pre \e a_fpError 不能为NULL
*/
static int tdr_get_entry_no_off_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

/**获取entry的type属性
*/
static int tdr_get_entry_type_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_get_entry_count_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);


/**检查entry数据的有效性 
*/
static int tdr_check_entry_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, FILE *a_fpError);



LPTDRMETAENTRY tdr_get_entry_by_id_i(TDRMETAENTRY pstEntry[], int iMax, int iID);




static int tdr_get_entry_io_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);
static int tdr_get_entry_refer_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError);
static int tdr_get_entry_defaultvalue_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);
/**获取entry的size属性
*@pre \e 必须先获取type属性
*/
static int tdr_get_entry_size_attribute_i(LPTDRMETAENTRY pstEntry, LPTDRMETA pstMeta, scew_element *pstElement, FILE *fpError);
static int tdr_get_entry_notnull_attribute_i(LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError);
static int tdr_get_entry_unique_attribute_i(LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError);
static int tdr_get_entry_sizeinfo_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError);
static int tdr_get_entry_sortmethod_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);
static int tdr_get_entry_extendtotable_attribute_i(LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement,FILE *a_fpError);


/**获取select属性
*/
static int tdr_get_entry_select_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError);

/**获取maxid minid属性
*@pre \e 必须先获取元素的id属性
*/
static int tdr_get_entry_maxmin_id_attribute_i(LPTDRMETAENTRY pstEntry, LPTDRMETA pstMeta, scew_element *pstElement, FILE *fpError);

/**获取tagsetversion = 0 entry元素特有的属性:target
*/
static int tdr_get_entry_target_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError);

/**获取tagsetversion = 1 entry元素特有的不需计算偏移的属性:unique nonull size defaultvalue  sortmethod 
*/
static int tdr_get_entry_version1_nooff_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);

/**计算meta第idxEntry个元素的偏移
*@note 此函数只能在创建过程调用,idxEntry取值范围: [0, a_pstMeta->iEntriesNum]
*/
static void tdr_calc_meta_entry_off_i(LPTDRMETA a_pstMeta, int idxEntry);

static int tdr_parse_entry_sizeinfo_off_i(LPTDRMETA a_pstMeta, int a_iEntry, const char* a_pszName, FILE *a_fpError);

/** 获取bindmacrosgroup属性信息
*/
static int tdr_get_entry_bindmacrosgroup_attribute_i(IN LPTDRMETALIB a_pstLib, LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError);

static int tdr_get_entry_autoincrement_attribute_i(LPTDRMETA a_pstMeta, LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement,FILE *a_fpError);

/**读取成员的customattr属性 
*/
static int tdr_get_customattr_attribute_i(IN LPTDRMETA a_pstMeta, IN LPTDRMETAENTRY a_pstEntry, IN scew_element *a_pstElement,
								   IN FILE *a_fpError);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int tdr_add_entry_to_meta_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    LPTDRMETAENTRY pstEntry = NULL;
    int iRet = 0;
    LPTDRMETALIB pstLib = NULL;
	int idxEntry;


    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    pstLib = TDR_META_TO_LIB(a_pstMeta);
	idxEntry = a_pstMeta->iEntriesNum;
    pstEntry = a_pstMeta->stEntries + idxEntry;
    tdr_init_entry_data_i(pstEntry, a_pstMeta);

    iRet = tdr_get_entry_no_off_attribute_i(pstEntry, a_pstMeta, a_pstElement, a_fpError);    

    /*计算偏移值*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		tdr_calc_meta_entry_off_i(a_pstMeta, a_pstMeta->iEntriesNum);
		a_pstMeta->iEntriesNum++;
	}
   
   
    /*获取refer属性*/
    if ((!TDR_ERR_IS_ERROR(iRet)) && (TDR_TYPE_UNION != a_pstMeta->iType))
    {
        iRet = tdr_get_entry_refer_attribute_i(a_pstMeta, idxEntry, a_pstElement, a_fpError);
    }  

    /*select */
    if ((!TDR_ERR_IS_ERROR(iRet)) && (TDR_TYPE_UNION == pstEntry->iType))
    {
        iRet = tdr_get_entry_select_attribute_i(a_pstMeta, idxEntry, a_pstElement, a_fpError);        
    }

    if (!TDR_ERR_IS_ERROR(iRet) && (TDR_XML_TAGSET_VERSION_0 == pstLib->iXMLTagSetVer))
    {
        iRet = tdr_get_entry_target_attribute_i(a_pstMeta, idxEntry, a_pstElement, a_fpError);
    }else if (!TDR_ERR_IS_ERROR(iRet) && (TDR_XML_TAGSET_VERSION_1 == pstLib->iXMLTagSetVer))
    {
        /*sizeinfo*/
        iRet = tdr_get_entry_sizeinfo_attribute_i(a_pstMeta, idxEntry, a_pstElement, a_fpError);
  
       
    }

    /*检查entry数据的有效性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_check_entry_i(pstEntry, a_pstMeta, a_fpError);
    }


    if (!TDR_ERR_IS_ERROR(iRet))
    {  
        /*更新meta的当前版本*/
        if (a_pstMeta->iCurVersion < pstEntry->iVersion)
        {
            a_pstMeta->iCurVersion = pstEntry->iVersion;
        }

        if (a_pstMeta->iMaxSubID < pstEntry->iID)
        {
            a_pstMeta->iMaxSubID = pstEntry->iID;
        }		
    }

    return iRet;
}

void tdr_calc_meta_entry_off_i(LPTDRMETA a_pstMeta, int idxEntry)
{
    LPTDRMETAENTRY pstEntry;
    int iEntryAlign = TDR_DEFAULT_ALIGN_VALUE;
    int iRealAlign = TDR_DEFAULT_ALIGN_VALUE;
    int iPadding;
    int iCount;

    assert(NULL != a_pstMeta);
    assert((0 <= idxEntry) && (idxEntry <= a_pstMeta->iEntriesNum));

    pstEntry = a_pstMeta->stEntries + idxEntry;

    if (0 >= pstEntry->iCount)
    {
    	 iCount = 1;
    }else
    {
       iCount = pstEntry->iCount;
    }
    if (0 < pstEntry->iCustomHUnitSize)
    {
        pstEntry->iHRealSize = pstEntry->iCustomHUnitSize * iCount;
    }else
    {
        pstEntry->iHRealSize = pstEntry->iHUnitSize * iCount;
    }
    pstEntry->iNRealSize = pstEntry->iNUnitSize * iCount;


	if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
	{
		iEntryAlign = TDR_POINTER_TYPE_ALIGN;
	}else  if (TDR_INVALID_PTR != pstEntry->ptrMeta)
    {
        LPTDRMETALIB pstLib = TDR_META_TO_LIB(a_pstMeta);
        LPTDRMETA pstType = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);

        iEntryAlign = pstType->iValidAlign;
    }else
    {
        LPTDRCTYPEINFO pstType = tdr_idx_to_typeinfo(pstEntry->idxType);
        iEntryAlign = pstType->iSize;
    }
    if (iEntryAlign > a_pstMeta->iValidAlign)
    {
        a_pstMeta->iValidAlign = iEntryAlign;
    }

    if( TDR_TYPE_UNION == a_pstMeta->iType )
    {
        pstEntry->iNOff = 0;   
        pstEntry->iHOff = 0;
        if( pstEntry->iHRealSize > a_pstMeta->iHUnitSize )
        {
            a_pstMeta->iHUnitSize = pstEntry->iHRealSize;
        }
        if (pstEntry->iNRealSize > a_pstMeta->iNUnitSize)
        {
            a_pstMeta->iNUnitSize = pstEntry->iNRealSize;
        }

        return ;
    }    

    /*struct 类型偏移计算*/      
    iPadding = 0;
    if( 0 < a_pstMeta->iEntriesNum )
    {   
        int iDiff;

        pstEntry->iNOff = (pstEntry - 1)->iNOff + (pstEntry - 1)->iNRealSize;  /*网络偏移*/

        iRealAlign = TDR_MIN(iEntryAlign, a_pstMeta->iCustomAlign);
        assert(0 != iRealAlign);
        pstEntry->iHOff = (pstEntry - 1)->iHOff + (pstEntry - 1)->iHRealSize;  /*本地存储偏移*/
        iDiff = pstEntry->iHOff % iRealAlign;
        if (0 != iDiff)
        {
            iPadding = iRealAlign - iDiff;
            pstEntry->iHOff += iPadding;
        }        
    } else 
    {
        pstEntry->iHOff = 0;
        pstEntry->iNOff = 0;
    } /*if( 0 < a_pstMeta->iEntriesNum )*/  

    a_pstMeta->iHUnitSize += iPadding;
    a_pstMeta->iHUnitSize += pstEntry->iHRealSize; 
    a_pstMeta->iNUnitSize += pstEntry->iNRealSize;

}


void tdr_init_entry_data_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta)
{
    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);

    memset(a_pstEntry, 0, sizeof(TDRMETAENTRY));

	a_pstEntry->iCount = 1;
    a_pstEntry->iVersion = a_pstMeta->iBaseVersion;
    a_pstEntry->iType = TDR_TYPE_UNKOWN;

    a_pstEntry->idxID = TDR_INVALID_INDEX;
    a_pstEntry->idxCount = TDR_INVALID_INDEX;
    a_pstEntry->idxIO = TDR_INVALID_INDEX;
    a_pstEntry->idxType = TDR_INVALID_INDEX;
    a_pstEntry->idxVersion = TDR_INVALID_INDEX;
    a_pstEntry->idxCustomHUnitSize = TDR_INVALID_INDEX;

    
    a_pstEntry->ptrDesc = TDR_INVALID_PTR;
    a_pstEntry->ptrChineseName = TDR_INVALID_PTR;
    a_pstEntry->ptrDefaultVal = TDR_INVALID_PTR;   
    a_pstEntry->ptrMeta = TDR_INVALID_PTR;

	
	a_pstEntry->chDBFlag = TDR_ENTRY_DB_FLAG_NONE;

	a_pstEntry->iID = TDR_INVALID_INDEX;
	a_pstEntry->iMaxIdIdx = TDR_INVALID_INDEX;
	a_pstEntry->iMinIdIdx = TDR_INVALID_INDEX;
	a_pstEntry->iMinId = a_pstEntry->iMaxId + 1; /*无效的maxid minid属性*/

    a_pstEntry->stRefer.iHOff = TDR_INVALID_OFFSET;
    a_pstEntry->stRefer.ptrEntry = TDR_INVALID_PTR;

    a_pstEntry->stSizeInfo.iHOff = TDR_INVALID_OFFSET;
    a_pstEntry->stSizeInfo.iNOff = TDR_INVALID_OFFSET;
	a_pstEntry->stSizeInfo.idxSizeType = TDR_INVALID_INDEX;

    a_pstEntry->stSelector.iHOff = TDR_INVALID_OFFSET;
    a_pstEntry->stSelector.ptrEntry = TDR_INVALID_PTR;
	a_pstEntry->chOrder = TDR_SORTMETHOD_NONE_SORT;

	a_pstEntry->ptrMacrosGroup = TDR_INVALID_PTR;

	TDR_ENTRY_CLEAR_ALL_FLAG(a_pstEntry);

	TDR_ENTRY_CLR_HAVE_ID(a_pstEntry);

	/*TO DO 目前仅定义值域属性的数据结构，为将来实现用*/
	a_pstEntry->ptrCustomAttr = TDR_INVALID_PTR;
}

int tdr_get_entry_no_off_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    int iRet = 0;
    LPTDRMETALIB pstLib;

    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);


    pstLib = TDR_META_TO_LIB(a_pstMeta);

    /*获取name 属性*/
    iRet = tdr_get_name_attribute_i(&a_pstEntry->szName[0], sizeof(a_pstEntry->szName), a_pstElement);   
    

     /*获取type属性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_entry_type_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);       
    }

	/*check name*/
	if (!TDR_ERR_IS_ERROR(iRet) && (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer))
	{
		if (a_pstEntry->szName[0] == '\0') /*空串*/
		{  
			fprintf(a_fpError, "error:\t %s元素的第%d个子元素没有name属性或其值不能为空串.\n", a_pstMeta->szName, a_pstMeta->iEntriesNum + 1);

			iRet = TDR_ERRIMPLE_META_NO_NAME;
		}else 
		{   /*判断meta下是否有同名的entry存在*/      
			int iIdx = tdr_get_entry_by_name_i(&a_pstMeta->stEntries[0], a_pstMeta->iEntriesNum, &a_pstEntry->szName[0]);
			if (TDR_INVALID_INDEX != iIdx)
			{       
				if (TDR_TYPE_UNION == a_pstMeta->iType)
				{
					LPTDRMETAENTRY pstEntry = a_pstMeta->stEntries + iIdx;
					if ((pstEntry->iType != a_pstEntry->iType) || (pstEntry->ptrMeta != a_pstEntry->ptrMeta))
					{
						fprintf(a_fpError, "error:\t %s元素存在两个名字<name = %s>相同的成员,union结构中只容许出现类型相同的同名成员.\n", a_pstMeta->szName, a_pstEntry->szName);
						iRet =TDR_ERRIMPLE_META_NAME_CONFLICT;  
					}
				}else /*if (TDR_TYPE_UNION == a_pstMeta->iType)*/
				{
					fprintf(a_fpError, "error:\t %s元素存在两个名字<name = %s>相同的成员.\n", a_pstMeta->szName, a_pstEntry->szName);
					iRet =TDR_ERRIMPLE_META_NAME_CONFLICT;  
				}				             
			}/*if (TDR_INVALID_INDEX != iIdx)*/
		}/*if (a_pstMeta->szName[0] == '\0')*/
	}/*if ((!TDR_ERR_IS_ERROR(iRet)) && (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer))*/

    /*读取ID属性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_id_attribute_i(&a_pstEntry->iID, &a_pstEntry->idxID, pstLib, a_pstElement, a_fpError);
        if (TDR_SUCCESS == iRet)
        {
            TDR_ENTRY_SET_HAVE_ID(a_pstEntry);

			a_pstEntry->iMaxId = a_pstEntry->iID;
			a_pstEntry->iMinId = a_pstEntry->iMaxId;
            if (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer)
            {
				/*检查ID属性的有效性，同父元素下子元素的id必须不同*/
				LPTDRMETAENTRY pstFindEntry = tdr_get_entry_by_id_i(&a_pstMeta->stEntries[0], a_pstMeta->iEntriesNum, a_pstEntry->iID);
				if (NULL != pstFindEntry)
				{              
					fprintf(a_fpError, "error:\t %s元素成员<name=%s>的ID属性值<%d>不容许另一个成员<name=%s>的ID属性值相同.\n", a_pstMeta->szName, 
						a_pstEntry->szName, a_pstEntry->iID, pstFindEntry->szName);
					iRet =TDR_ERRIMPLE_META_ID_CONFLICT;
				} 
				if (0 > a_pstEntry->iID)
				{
					fprintf(a_fpError, "error:\t %s元素成员<name=%s>其ID属性值<%d>不正确，其属性值只能为非负数.\n", a_pstMeta->szName, 
						a_pstEntry->szName, a_pstEntry->iID);
					iRet =TDR_ERRIMPLE_META_ID_CONFLICT;
				}
            }/*if (TDR_XML_TAGSET_VERSION_0 =< pstLib->iXMLTagSetVer)*/
           
        }/*if (TDR_SUCCESS == iRet)*/
    }    
    
    /*version属性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_version_attribute_i(&a_pstEntry->iVersion, &a_pstEntry->idxVersion, pstLib, a_pstElement, a_fpError);		
    }

    /*desc*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_desc_attribute_i(&a_pstEntry->ptrDesc, pstLib, a_pstElement, a_fpError);
    }     

    /*获取count属性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_entry_count_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);
    }

    /*获取IO属性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_entry_io_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);
    }   

    if (!TDR_ERR_IS_ERROR(iRet) && (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer))
    {
        iRet = tdr_get_entry_version1_nooff_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);
    }    
  
    return iRet;
}



int tdr_get_entry_count_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib =NULL;
    int iRet = TDR_SUCCESS;

    assert(NULL != a_pstElement);
    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);


    pstLib = TDR_META_TO_LIB(a_pstMeta);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_COUNT);    
    if( NULL != pstAttr )
    {
		char szCount[256] = {0};

        tdr_normalize_string(&szCount[0], sizeof(szCount),scew_attribute_value(pstAttr));
        iRet = tdr_get_macro_int_i(&a_pstEntry->iCount, &a_pstEntry->idxCount, pstLib, &szCount[0]);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            fprintf(a_fpError, "\nerror:\t entry元素<name = %s>的count属性值<%s>无效,请确定其值是整数或宏\n",
                a_pstEntry->szName, szCount);
            
            iRet = TDR_ERRIMPLE_ENTRY_INVALID_COUNT_VALUE;
        }else if (0 > a_pstEntry->iCount)
        {
            fprintf(a_fpError, "error:\t entry元素<name = %s>的count属性值<count = %d>不能小于0\n",
                a_pstEntry->szName, a_pstEntry->iCount);
            
            iRet = TDR_ERRIMPLE_ENTRY_INVALID_COUNT_VALUE;
        }

		/*按照老的定义，对于string类型其count属性对应新定义的size属性，因此进行转换*/
		if ((TDR_TYPE_STRING == a_pstEntry->iType) && (TDR_XML_TAGSET_VERSION_0 == pstLib->iXMLTagSetVer))
		{
			if (0 == a_pstEntry->iCount)
			{
				fprintf(a_fpError, "error:\t entry元素<name = %s>为string类型其count属性值<count = %d>不能为0\n",
					a_pstEntry->szName, a_pstEntry->iCount);
				iRet = TDR_ERRIMPLE_ENTRY_INVALID_COUNT_VALUE;
			}else
			{
				a_pstEntry->idxCustomHUnitSize = a_pstEntry->idxCount;
				a_pstEntry->iCustomHUnitSize = a_pstEntry->iCount;
				a_pstEntry->idxCount = TDR_INVALID_INDEX;
				a_pstEntry->iCount = 1;
			}
		}
    }/*if( NULL != pstAttr )*/

    return iRet;
}

int tdr_get_entry_io_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib =NULL;
    int iRet = TDR_SUCCESS;
    char szIO[128] = {0};
    
    assert(NULL != a_pstElement);
    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);


    pstLib = TDR_META_TO_LIB(a_pstMeta);

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_IO);    
    if (NULL == pstAttr)
    {
    	return TDR_SUCCESS;
    }

    tdr_normalize_string(&szIO[0], sizeof(szIO), scew_attribute_value(pstAttr));
    if (0 == tdr_stricmp(szIO, TDR_TAG_IO_NO_LIMIT))
    {
    	a_pstEntry->iIO  = TDR_IO_NOLIMIT;
    }else if (0 == tdr_stricmp(szIO, TDR_TAG_IO_NO_INPUT))
    {
    	a_pstEntry->iIO = TDR_IO_NOINPUT;
    }else if (0 == tdr_stricmp(szIO, TDR_TAG_IO_NO_OUTPUT))
    {
    	a_pstEntry->iIO = TDR_IO_NOOUTPUT;
    }else if (0 == tdr_stricmp(szIO, TDR_TAG_IO_NO_IO))
    {
    	 a_pstEntry->iIO = TDR_IO_NOIO;
    }else
    {
        iRet = tdr_get_macro_int_i(&a_pstEntry->iIO, &a_pstEntry->idxIO, pstLib, szIO);
        if (TDR_ERR_IS_ERROR(iRet))
        {
            fprintf(a_fpError, "\nerror:\t entry元素<name = %s>的io属性值<%s>无效,请确定其值是整数或宏",
                a_pstEntry->szName, szIO);
			iRet = TDR_ERRIMPLE_ENTRY_INVALID_IO_VALUE;
        }else if ((TDR_IO_MIN_VALUE > a_pstEntry->iIO) ||
            (TDR_IO_MAX_VALUE < a_pstEntry->iIO))
        {
            fprintf(a_fpError, "\nerror:\t entry元素<name = %s>的io属性值<%s>无效,请确定其值是整数或宏",
                a_pstEntry->szName, szIO);
            
            iRet = TDR_ERRIMPLE_ENTRY_INVALID_IO_VALUE;
        }       
   }

    return iRet;
}

int tdr_get_entry_refer_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib =NULL;
    const char *pszRefer = NULL;
    int iRet = TDR_SUCCESS;
	LPTDRMETAENTRY pstEntry;

    assert(NULL != a_pstElement);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);
	assert((0 <= a_idxEntry) && (a_idxEntry < a_pstMeta->iEntriesNum));


    pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstEntry = a_pstMeta->stEntries + a_idxEntry;

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_REFER);    
    if( NULL != pstAttr )
    {
		LPTDRMETAENTRY pstFindEntry = NULL;

		if (TDR_TYPE_UNION == a_pstMeta)
		{
			fprintf(a_fpError, "\nerror:\t union类型<name = %s>不容许其成员元素<name = %s>指定refer属性值<%s>\n",
				a_pstMeta->szName, pstEntry->szName, pszRefer);
			return TDR_ERRIMPLE_ENTRY_INVALID_REFER_VALUE;

		}
		
        
        pszRefer = scew_attribute_value(pstAttr);
        iRet = tdr_name_to_off_i(a_pstMeta, &pstEntry->stRefer, a_idxEntry, pszRefer);
        if (TDR_SUCCESS !=iRet)
        {
            fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员元素<name = %s>的refer属性值<%s>无效，请确定指定正确的元素数据路径，且路径中涉及的各元素不是指针或引用类型\n",
                a_pstMeta->szName, pstEntry->szName, pszRefer);
            iRet = TDR_ERRIMPLE_ENTRY_INVALID_REFER_VALUE;
        }else
        {
			pstFindEntry = TDR_PTR_TO_ENTRY(pstLib, pstEntry->stRefer.ptrEntry);
            if((pstFindEntry->iType <= TDR_TYPE_COMPOSITE)
                || (pstFindEntry->iType>TDR_TYPE_UINT) )
            {
                fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员元素<name = %s>的refer属性值<%s>无效\n",
                    a_pstMeta->szName, pstEntry->szName, pszRefer);
                iRet = TDR_ERRIMPLE_ENTRY_INVALID_REFER_VALUE;
            }else if (pstEntry->stRefer.iHOff >= pstEntry->iHOff)
            {
				fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员元素<name = %s>的refer属性值<%s>无效,必须是该元素的直接父元素包含的特定子元素\n",
					a_pstMeta->szName, pstEntry->szName, pszRefer);
				iRet = TDR_ERRIMPLE_ENTRY_INVALID_REFER_VALUE;
			}else if (1 < pstFindEntry->iCount)
			{
				fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员元素<name = %s>的refer属性值<%s>不能为数组\n",
					a_pstMeta->szName, pstEntry->szName, pszRefer);
				iRet = TDR_ERRIMPLE_ENTRY_INVALID_REFER_VALUE;
			}

			/*设置计数器标志 并关联两个entry*/
			TDR_ENTRY_SET_COUNTER(pstFindEntry);
			pstFindEntry->stRefer.ptrEntry = TDR_ENTRY_TO_PTR(pstLib, pstEntry);  
        } 
    }/*if( NULL != pstAttr )*/

    return iRet;
}

int tdr_get_entry_defaultvalue_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    int iDefaultLen = 0;
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = NULL;
	char szDefault[TDR_DEFAULT_VALUE_LEN] = {0};
	LPTDRCTYPEINFO pstTypeInfo;
	int iSize;
	char *pszFreeBuf;

    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

	pstLib = TDR_META_TO_LIB(a_pstMeta);
    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_DEFAULT_VALUE);   
	if (NULL == pstAttr)
	{
		return TDR_SUCCESS;
	}

	if (TDR_TYPE_COMPOSITE >= a_pstEntry->iType)
	{
		fprintf(a_fpError, "\nwarning:\t 自定义类型<name = %s>的成员<name = %s>为自定义复杂数据类型，目前暂不支持设置缺省值",
			a_pstMeta->szName, a_pstEntry->szName); 
		return TDR_SUCCESS;
	}

	if (TDR_ENTRY_IS_POINTER_TYPE(a_pstEntry) || TDR_ENTRY_IS_REFER_TYPE(a_pstEntry))
	{
		fprintf(a_fpError, "\nwarning:\t 自定义类型<name = %s>的成员<name = %s>为指针或引用数据类型，目前暂不支持设置缺省值",
			a_pstMeta->szName, a_pstEntry->szName); 
		return TDR_SUCCESS;
	}


	tdr_normalize_string(&szDefault[0], sizeof(szDefault), scew_attribute_value(pstAttr));
	pstTypeInfo = tdr_idx_to_typeinfo(a_pstEntry->idxType);
	if (TDR_TYPE_STRING > a_pstEntry->iType)
	{
		iDefaultLen = pstTypeInfo->iSize; /*加1是为了保留结束字符*/		
	}else if (TDR_TYPE_STRING == a_pstEntry->iType)
	{
		iDefaultLen = strlen(&szDefault[0]) + 1; /*加1是为了保留结束字符*/
	}else if (TDR_TYPE_WSTRING == a_pstEntry->iType)
	{
		iDefaultLen = strlen(&szDefault[0]) + 1;
		iDefaultLen *= sizeof(tdr_wchar_t);
	}


	if (iDefaultLen > pstLib->iFreeStrBufSize)
	{
		fprintf(a_fpError, "\nwarning:\t 元数据字符串缓冲区剩余空间<%d>不足以存储自定义类型<name = %s>的成员<name = %s>的缺省值<%s>",
			pstLib->iFreeStrBufSize, a_pstMeta->szName, a_pstEntry->szName, szDefault); 
		return iRet = TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
	}
	

	
	pszFreeBuf = TDR_GET_FREE_BUF(pstLib);
	iSize = iDefaultLen;	
	switch(a_pstEntry->iType)
	{
	case TDR_TYPE_CHAR:		
	case TDR_TYPE_UCHAR:		
	case TDR_TYPE_SMALLINT:		
	case TDR_TYPE_SMALLUINT:		
	case TDR_TYPE_INT:
	case TDR_TYPE_LONG:		
	case TDR_TYPE_UINT:
	case TDR_TYPE_ULONG:		
	case TDR_TYPE_LONGLONG:		
	case TDR_TYPE_ULONGLONG:
		iRet = tdr_ioscanf_basedtype_i(pstLib, a_pstEntry, pszFreeBuf, &iSize, &szDefault[0]);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			if (TDR_ERROR_VALUE_BEYOND_TYPE_FIELD == TDR_ERR_GET_ERROR_CODE(iRet))
			{
				fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员<name = %s>的数据类型为<type=%s>，其缺省值<%s>与类型不匹配",
					a_pstMeta->szName, a_pstEntry->szName, pstTypeInfo->pszName, szDefault);
			}else
			{
				fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员<name = %s>的数据类型为<type=%s>，读取其缺省值<%s>失败",
					a_pstMeta->szName, a_pstEntry->szName, pstTypeInfo->pszName, szDefault);
			}
		}/*if (TDR_ERR_IS_ERROR(iRet))*/		
		break;
	case TDR_TYPE_FLOAT:		
	case TDR_TYPE_DOUBLE:		
	case TDR_TYPE_IP:		
	case TDR_TYPE_WCHAR:
	case TDR_TYPE_STRING:		
	case TDR_TYPE_WSTRING:
		iRet = tdr_ioscanf_basedtype_i(pstLib, a_pstEntry, pszFreeBuf, &iSize, &szDefault[0]);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员<name = %s>的数据类型为<type=%s>，读取其缺省值<%s>失败",
				a_pstMeta->szName, a_pstEntry->szName, pstTypeInfo->pszName, szDefault);
		}/*if (TDR_ERR_IS_ERROR(iRet))*/	
		break;
	case TDR_TYPE_DATETIME:
		{
			iRet = tdr_ioscanf_basedtype_i(pstLib, a_pstEntry, pszFreeBuf, &iSize, &szDefault[0]);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				fprintf(a_fpError, "\nwarning:\t 自定义类型<name = %s>的成员<name = %s>日期/时间(datetime)类型的缺省值无效,正确datetime类型其值格式应为\"YYYY-MM-DD HH:MM:SS\".",
					a_pstMeta->szName, a_pstEntry->szName);
			}
			break;
		}
	case TDR_TYPE_DATE:
		{
			iRet = tdr_ioscanf_basedtype_i(pstLib, a_pstEntry, pszFreeBuf, &iSize, &szDefault[0]);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				fprintf(a_fpError, "\nwarning:\t 自定义类型<name = %s>的成员<name = %s>日期(date)类型的缺省值无效,正确date类型其值格式应为\"YYYY-MM-DD\".",
					a_pstMeta->szName, a_pstEntry->szName);
			}
			break;
		}
	case TDR_TYPE_TIME:
		{
			iRet = tdr_ioscanf_basedtype_i(pstLib, a_pstEntry, pszFreeBuf, &iSize, &szDefault[0]);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员<name = %s>时间(time)类型的缺省值无效,正确date类型其值格式应为\"HHH:MM:SS\".",
					a_pstMeta->szName, a_pstEntry->szName);
			}
			break;
		}
	default:			
		fprintf(a_fpError, "\nwarning:\t 自定义类型<name = %s>的成员<name = %s>的类型<type=%s>目前暂不支持设置缺省值.",
			a_pstMeta->szName, a_pstEntry->szName, pstTypeInfo->pszName);
		iDefaultLen = 0;
		break;
	}

	if (TDR_SUCCESS == iRet)
	{
		a_pstEntry->ptrDefaultVal = pstLib->ptrFreeStrBuf;
		a_pstEntry->iDefaultValLen = iDefaultLen;
		pstLib->ptrFreeStrBuf += iDefaultLen;
		pstLib->iFreeStrBufSize -= iDefaultLen;
	}
	
    return iRet;
}

int tdr_get_entry_size_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = NULL;

    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

	pstLib = TDR_META_TO_LIB(a_pstMeta);
    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SIZE);    
    if( NULL != pstAttr )
    {
		char szSize[256] = {0};
		
		tdr_normalize_string(&szSize[0], sizeof(szSize), scew_attribute_value(pstAttr));
        iRet = tdr_get_macro_int_i(&a_pstEntry->iCustomHUnitSize, &a_pstEntry->idxCustomHUnitSize, pstLib, &szSize[0]);
        if ((TDR_SUCCESS != iRet) || (0 >= a_pstEntry->iCustomHUnitSize))
        {
            fprintf(a_fpError, "\nerror:\t 结构体<name=%s>的成员<name = %s>的size属性值<%s>无效,请确定其值是正整数或已定义的宏",
                a_pstMeta->szName, a_pstEntry->szName, &szSize[0]);
            
            iRet = TDR_ERRIMPLE_ENTRY_INVALID_SIZE_VALUE;
        }else if (0 < a_pstEntry->iCustomHUnitSize)
        {/*检查预指定大小是否比实际需要的大*/
            LPTDRCTYPEINFO pstTypeInfo = tdr_idx_to_typeinfo(a_pstEntry->idxType);

			if ((TDR_TYPE_STRING == a_pstEntry->iType) || (TDR_TYPE_WSTRING == a_pstEntry->iType))
			{
				a_pstEntry->iCustomHUnitSize *= pstTypeInfo->iSize;
			}

            if ((TDR_TYPE_COMPOSITE < a_pstEntry->iType) && (TDR_TYPE_STRING > a_pstEntry->iType))
            {
                fprintf(a_fpError, "error:\t 自定义类型<name =%s>的成员<name = %s>为存储单元固定的数据类型<type =%s>不能随意指定存储size<%d>\n",
                    a_pstMeta->szName, a_pstEntry->szName, pstTypeInfo->pszName, a_pstEntry->iCustomHUnitSize);
                
                iRet = TDR_ERRIMPLE_ENTRY_INVALID_SIZE_VALUE;
            }else if (a_pstEntry->iCustomHUnitSize < a_pstEntry->iHUnitSize)
            {
                /*指定size比实际计算的size要小*/ 
                fprintf(a_fpError, "error:\t 自定义类型<name =%s>的成员<name = %s>指定的存储空间<size = %d>不能小于实际需要存储空间<%d>\n",
                    a_pstMeta->szName, a_pstEntry->szName, a_pstEntry->iCustomHUnitSize, a_pstEntry->iHUnitSize);
                
                iRet = TDR_ERRIMPLE_ENTRY_INVALID_SIZE_VALUE;
            }
        } /*else if (0 < a_pstEntry->iExpectedUnit)*/     
    }/* if( NULL != pstAttr )*/

    return iRet;
}


int tdr_get_entry_notnull_attribute_i(LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    int iRet = TDR_SUCCESS;


    assert(NULL != a_pstEntry);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_NOTNULL);    
    if( NULL != pstAttr )
    {
		char szVal[256] = {0};

        tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstAttr));
        if (0 == tdr_stricmp(szVal, TDR_TAG_TRUE))
        {
            TDR_ENTRY_SET_NOT_NULL(a_pstEntry);
        }else if ( 0== tdr_stricmp(szVal, TDR_TAG_FALSE))
        {
            TDR_ENTRY_CLR_NOT_NULL(a_pstEntry);
        }else
        {
            fprintf(a_fpError, "error:\t entry元素<name = %s>的notnull属性值<%s>无效,其值为\"false\"或者\"true\"\n",
                a_pstEntry->szName, szVal);
            
            iRet = TDR_ERRIMPLE_ENTRY_INVALID_NOTNULL_VALUE;
        }/*if ( 0== strcmp(pszVal, TDR_TAG_FALSE))*/
    }

    return iRet ;
}

int tdr_get_entry_unique_attribute_i(LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    int iRet = TDR_SUCCESS;


    assert(NULL != a_pstEntry);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_UNIQUE);    
    if( NULL != pstAttr )
    {
		char szVal[256] = {0};

		tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstAttr));
        if (0 == tdr_stricmp(szVal, TDR_TAG_TRUE))
        {
            TDR_ENTRY_SET_UNIQUE(a_pstEntry);
        }else if ( 0== tdr_stricmp(szVal, TDR_TAG_FALSE))
        {
            TDR_ENTRY_CLR_UNIQUE(a_pstEntry);
        }else
        {
            fprintf(a_fpError, "\nerror:\t entry元素<name = %s>的unique属性值<%s>无效,其值为\"false\"或者\"true\"\n",
                a_pstEntry->szName, szVal);
            
            iRet = TDR_ERRIMPLE_ENTRY_INVALID_UNIQUE_VALUE;
        }/*if ( 0== strcmp(pszVal, TDR_TAG_FALSE))*/
    }

    return iRet;
}

int tdr_get_entry_sizeinfo_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib =NULL;
    const char *pszVal = NULL;
    int iRet = TDR_SUCCESS;
	LPTDRMETAENTRY pstEntry;

    assert(NULL != a_pstElement);
	assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);
	assert((0 <= a_idxEntry) && (a_idxEntry <= a_pstMeta->iEntriesNum));


    pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstEntry = a_pstMeta->stEntries + a_idxEntry;

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SIZEINFO);    
    if( NULL != pstAttr )
    {
        pszVal = scew_attribute_value(pstAttr);
        iRet = tdr_parse_entry_sizeinfo_off_i(a_pstMeta, a_idxEntry, pszVal, a_fpError);	        
    }

    return iRet; 
}

int tdr_get_entry_sortmethod_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    int iRet = TDR_SUCCESS;

    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SORTMETHOD);    
    if( NULL != pstAttr )
    {
		char szVal[32] = {0};

        tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstAttr));
        if (0 == tdr_stricmp(szVal, TDR_TAG_NO))
        {
            a_pstEntry->chOrder = TDR_SORTMETHOD_NONE_SORT;
        }else if ( 0== tdr_stricmp(szVal, TDR_TAG_ASC))
        {
            a_pstEntry->chOrder = TDR_SORTMETHOD_ASC_SORT;
        }else if ( 0== tdr_stricmp(szVal, TDR_TAG_DESC))
        {
            a_pstEntry->chOrder = TDR_SORTMETHOD_DSC_SORT;
        }else
        {
            fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员元素<name = %s>的sortmethod属性值<%s>无效,\n",
                a_pstMeta->szName, a_pstEntry->szName, szVal);
            iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_SORTMETHOD_VALUE);
        }
    }

    return iRet ;
}


int tdr_get_entry_select_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib =NULL;
    const char *pszVal = NULL;
    int iRet = TDR_SUCCESS;
	LPTDRMETAENTRY pstEntry;

    assert(NULL != a_pstElement);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);
	assert((0 <= a_idxEntry) && (a_idxEntry < a_pstMeta->iEntriesNum));

    pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstEntry = a_pstMeta->stEntries + a_idxEntry;

    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_SELECT);    
    if( NULL != pstAttr )
    {
        LPTDRMETAENTRY pstFindEntry = NULL;

        pszVal = scew_attribute_value(pstAttr);
        iRet = tdr_name_to_off_i(a_pstMeta, &pstEntry->stSelector, a_idxEntry, pszVal);
        if (TDR_SUCCESS !=iRet)
        {
            fprintf(a_fpError, "error:\t 自定义类型<name = %s>的成员元素<name = %s>的select属性值<%s>无效,请确定指定正确的元素数据路径，且路径中涉及的各元素不是指针或引用类型\n",
                 a_pstMeta->szName, pstEntry->szName, pszVal);
            
            iRet = TDR_ERRIMPLE_ENTRY_INVALID_SELECT_VALUE;
        }else
        {
			pstFindEntry = TDR_PTR_TO_ENTRY(pstLib, pstEntry->stSelector.ptrEntry);
            if((pstFindEntry->iType <= TDR_TYPE_COMPOSITE)
                || (pstFindEntry->iType>TDR_TYPE_UINT) )
            {
                fprintf(a_fpError, "error:\t 自定义类型<name = %s>的成员元素<name = %s>的select属性值<%s>无效,\n",
                    a_pstMeta->szName, pstEntry->szName, pszVal);
                
                iRet = TDR_ERRIMPLE_ENTRY_INVALID_SELECT_VALUE;
			}else if (pstEntry->stRefer.iHOff >= pstEntry->iHOff)
			{
				fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员元素<name = %s>的select属性值<%s>无效,必须是该元素的直接父元素包含的特定子元素",
					a_pstMeta->szName, pstEntry->szName, pszVal);
				iRet = TDR_ERRIMPLE_ENTRY_INVALID_REFER_VALUE;
			}
        }
    }/*if( NULL != pstAttr )*/

    return iRet;
}



int tdr_get_entry_maxmin_id_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstMaxIdAttr = NULL;
    scew_attribute *pstMinIdAttr = NULL;

    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = NULL;
    int iMaxID = 0;
    int iMinID = 0;
    int idxMax = TDR_INVALID_INDEX;
    int idxMin = TDR_INVALID_INDEX;
	char szVal[256] = {0};


    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_pstElement);
    assert(NULL != a_fpError);


 
	pstLib = TDR_META_TO_LIB(a_pstMeta);
    
    pstMaxIdAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_MAXID);  
    pstMinIdAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_MINID);
    if ((NULL == pstMaxIdAttr) && (NULL == pstMinIdAttr))
    {
        return TDR_SUCCESS;
    }else if ((NULL == pstMaxIdAttr) || (NULL == pstMinIdAttr))
    {
         /*maxid和minid必须同时出现*/
        fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员元素<name = %s>的maxid和minid属性值必须同时出现",
            a_pstMeta->szName, a_pstEntry->szName);
        
        iRet = TDR_ERRIMPLE_ENTRY_INVALID_MAXMINID_VALUE;
    }

    /*读取maxid属性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstMaxIdAttr));
        iRet = tdr_get_macro_int_i(&iMaxID, &idxMax, pstLib, szVal);
        if (TDR_ERR_IS_ERROR(iRet)) 
        {
            fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员元素<name = %s>的maxid属性值<%s>无效,请确定其值是整数或已定义的宏",
                a_pstMeta->szName, a_pstEntry->szName, szVal);
            
            iRet = TDR_ERRIMPLE_ENTRY_INVALID_MAXID_VALUE;
        }
    }
    
    
    /*读取minid属性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
		tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstMinIdAttr));
        iRet = tdr_get_macro_int_i(&iMinID, &idxMin, pstLib, szVal);
        if (TDR_SUCCESS != iRet)
        {
            fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员元素<name = %s>的minid属性值<%s>无效,请确定其值是整数或已定义的宏",
                a_pstMeta->szName, a_pstEntry->szName, szVal);
            
            iRet = TDR_ERRIMPLE_ENTRY_INVALID_MINID_VALUE;
        }
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/
    
    /*检查maxid, minid属性的正确性*/
    if ((!TDR_ERR_IS_ERROR(iRet)) && (iMaxID < iMinID))
    {
        fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员元素<name = %s>的maxid和minid属性值必须同时出现",
            a_pstMeta->szName, a_pstEntry->szName);
        
        iRet = TDR_ERRIMPLE_ENTRY_INVALID_MAXMINID_VALUE;
    }/*if ((!TDR_ERR_IS_ERROR(iRet)) && (iMaxID < iMinID))*/

    
    if (!TDR_ERR_IS_ERROR(iRet))
    {
		TDR_ENTRY_SET_HAVE_MAXMIN_ID(a_pstEntry);
        a_pstEntry->iMaxId = iMaxID;
        a_pstEntry->iMaxIdIdx = idxMax;
        a_pstEntry->iMinId = iMinID;
        a_pstEntry->iMinIdIdx = idxMin;
    }


    return iRet;
}


int tdr_get_entry_target_attribute_i(LPTDRMETA a_pstMeta, int a_idxEntry, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    const char *pszTargetVal = NULL;
    int iRet = TDR_SUCCESS;
	

    assert(NULL != a_pstElement);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);
	assert((0 <= a_idxEntry) && (a_idxEntry <= a_pstMeta->iEntriesNum));
   

    /*获取target属性,union结构没有此属性*/
    if (TDR_TYPE_UNION != a_pstMeta->iType)
    {
        pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_TARGET);    
        if( NULL != pstAttr )
        {
            pszTargetVal = scew_attribute_value(pstAttr);
            iRet = tdr_parse_entry_sizeinfo_off_i(a_pstMeta, a_idxEntry, pszTargetVal, a_fpError);			
        }
    }/*if (TDR_TYPE_UNION != a_pstMeta->iType)*/    

    return iRet;
}

int tdr_parse_entry_sizeinfo_off_i(LPTDRMETA a_pstMeta, int a_idxEntry, const char* a_pszName, FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETAENTRY pstEntry;
	LPTDRSIZEINFO pstSizeInfo;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pszName);
	assert(NULL != a_fpError);
	assert((0 <= a_idxEntry) && (a_idxEntry <= a_pstMeta->iEntriesNum));

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	pstEntry = a_pstMeta->stEntries + a_idxEntry;
	pstSizeInfo = &pstEntry->stSizeInfo;
	iRet = tdr_sizeinfo_to_off_i(pstSizeInfo, a_pstMeta, a_idxEntry, a_pszName);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员<name=%s>sizeinfo属性值<%s>无效,其属性值只能为<tinyuint-uint>或者该元素的直接父元素包含的特定子元素成员的Path\n",
			a_pstMeta->szName, pstEntry->szName, a_pszName);
		return iRet;
	} 

	/*存储空间固定的元素，其sizeinfo属性被忽略*/
	if (((TDR_TYPE_COMPOSITE < pstEntry->iType) && (TDR_TYPE_STRING > pstEntry->iType)) ||
		(TDR_TYPE_VOID == pstEntry->iType))
	{
		pstSizeInfo->idxSizeType = TDR_INVALID_INDEX;
		pstSizeInfo->iHOff = TDR_INVALID_OFFSET;
		pstSizeInfo->iNOff = TDR_INVALID_OFFSET;
		pstSizeInfo->iUnitSize = 0;
		fprintf(a_fpError, "\nwarning:\t 自定义类型<name = %s>的成员<name=%s>的存储空间是固定的，其sizeinfo属性被忽略\n",
			a_pstMeta->szName, pstEntry->szName);
		return TDR_SUCCESS;
	}

	/*string,wstring成员其sizetype属性只能是tinyint-unit*/
	if ((TDR_TYPE_STRING == pstEntry->iType) || (TDR_TYPE_WSTRING == pstEntry->iType))
	{
		if (TDR_INVALID_INDEX == pstSizeInfo->idxSizeType)
		{
			fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员<name=%s>为string类型，其sizeinfo属性值只能为<tinyuint-uint>\n",
				a_pstMeta->szName, pstEntry->szName);
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_SIZEINFO_VALUE);
		}
	}

	
	if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
	{
		/*复合数据类型，如果其结构描述中已经指定了sizetype属性，则不能在指定sizeinfo属性*/
		LPTDRMETA pstType = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
		if (pstType->stSizeType.iUnitSize > 0)
		{
			fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员<name=%s>对应的结构体<name=%s>已经指定了sizetype属性，此成员不能再指定sizeinfo属性了\n",
				a_pstMeta->szName, pstEntry->szName, pstType->szName);
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_SIZEINFO_VALUE);
		}

		/*复合数据类型数组成员，其sizeinfo属性值不能为此成员前面兄弟节点元素为根的Path*/
		if ((pstSizeInfo->iNOff < pstEntry->iNOff) && ((1 < pstEntry->iCount) || (0 == pstEntry->iCount)))
		{
			fprintf(a_fpError, "\nerror:\t 自定义类型<name = %s>的成员<name=%s>为复合类型数组，其sizeinfo属性值只能为<tinyuint-uint>或以当前元素为根的成员Path\n",
				a_pstMeta->szName, pstEntry->szName);
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_SIZEINFO_VALUE);
		}
	}
	
	return iRet;
}

int tdr_get_entry_version1_nooff_attribute_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    LPTDRMETALIB pstLib = NULL;
    int iRet = TDR_SUCCESS;

    assert(NULL != a_pstElement);
    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);

    pstLib = TDR_META_TO_LIB(a_pstMeta);

	/*bindmacrosgroup*/	
	iRet = tdr_get_entry_bindmacrosgroup_attribute_i(pstLib, a_pstEntry, a_pstElement, a_fpError);
	

    /*获取unique属性*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		 iRet = tdr_get_entry_unique_attribute_i(a_pstEntry, a_pstElement, a_fpError);   
	}
    
    /*cname属性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {      
        iRet = tdr_get_cname_attribute_i(&a_pstEntry->ptrChineseName, pstLib, a_pstElement, a_fpError);
    } 


    /*获取notnull属性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_entry_notnull_attribute_i(a_pstEntry, a_pstElement, a_fpError);         
    } 

    /*获取size属性*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_entry_size_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);   
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/
   

    /*defaultvalue*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
       iRet = tdr_get_entry_defaultvalue_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/

   

    /*sortmethod*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        iRet = tdr_get_entry_sortmethod_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);
    }   

	/*extendtotable*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_get_entry_extendtotable_attribute_i(a_pstEntry, a_pstElement, a_fpError);
	} 

	/*auto increment*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_get_entry_autoincrement_attribute_i(a_pstMeta, a_pstEntry, a_pstElement, a_fpError);
	} 
	
	/*customattr属性*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{      
		iRet = tdr_get_customattr_attribute_i(a_pstMeta, a_pstEntry, a_pstElement, a_fpError);
	} 
    
    /*如果是union meta则读取union entry特殊的属性: maxid minid*/
    if (!TDR_ERR_IS_ERROR(iRet) && (TDR_TYPE_UNION == a_pstMeta->iType))
    {
         iRet = tdr_get_entry_maxmin_id_attribute_i(a_pstEntry, a_pstMeta, a_pstElement, a_fpError);        
    }

    return iRet;
}



int tdr_check_entry_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, FILE *a_fpError)
{
    int iRet = TDR_SUCCESS;
    LPTDRMETALIB pstLib = NULL;
    TDRBOOLEAN bDyn = TDR_FALSE; 

    assert(NULL != a_pstEntry);  
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);

    pstLib = TDR_META_TO_LIB(a_pstMeta);    

	/*union 类型检查*/
	if ((TDR_TYPE_UNION == a_pstMeta->iType) && (TDR_TYPE_UNION == a_pstEntry->iType))
	{
		fprintf(a_fpError, "\nerror:\t union类型<name = %s>不能再包含union成员<name = %s>", 
			a_pstMeta->szName, a_pstEntry->szName);
		return TDR_ERRIMPLE_ENTRY_INVALID_TYPE_VALUE;
	}

	/*检查count属性*/
    if (0 == a_pstEntry->iCount)
    {
    	  if (((TDR_INVALID_OFFSET == a_pstEntry->stRefer.iHOff) || (TDR_INVALID_PTR == a_pstEntry->stRefer.ptrEntry) ||
       	 (0 >= a_pstEntry->stRefer.iUnitSize)))
   	 {
       	 fprintf(a_fpError, "error:\t %s结构的成员<name = %s>为可变数组，必须通过refer属性确定其实际大小\n",
           		 a_pstMeta->szName, a_pstEntry->szName);
       	 return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VARIABLE_ARRAY_NO_REFER);
  	  }
  	  if (((TDR_TYPE_STRING == a_pstEntry->iType) || (TDR_TYPE_WSTRING == a_pstEntry->iType)) && (0 >= a_pstEntry->iCustomHUnitSize)) 
         {
       	fprintf(a_fpError, "error:\t %s结构的成员<name = %s>为不指定存储空间的字符串类型，不能为可变数组\n",
           		a_pstMeta->szName, a_pstEntry->szName);
       	 return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_VARIABLE_ARRAY_NO_REFER);
        }
    }/* if (0 == a_pstEntry->iCount)*/
    
    /*select 属性检查*/
    if (TDR_TYPE_UNION == a_pstEntry->iType)
    {
        if ((TDR_INVALID_INDEX == a_pstEntry->stSelector.iHOff) || 
            (TDR_INVALID_PTR == a_pstEntry->stSelector.ptrEntry) ||
            (0 >= a_pstEntry->stSelector.iUnitSize))
        {
            fprintf(a_fpError, "error:\t %s结构的成员<name = %s>为union数据类型，没有select属性或select属性值无效\n",
            a_pstMeta->szName, a_pstEntry->szName);
            return TDR_ERRIMPLE_ENTRY_INVALID_SELECT_VALUE;
        }
    }	
    
    if (0 == a_pstEntry->iCount)
    {
        bDyn = TDR_TRUE;
    }
    if ((TDR_FALSE == bDyn) && (0 < a_pstEntry->iCount) && (a_pstEntry->stRefer.iUnitSize>0))
    {
        bDyn = TDR_TRUE;
    }

    if ( (TDR_FALSE == bDyn) && (TDR_TYPE_UNION == a_pstEntry->iType) &&
        ( a_pstEntry->stSelector.iUnitSize > 0  ))
    {
        bDyn = TDR_TRUE;
    }  
    
    if ( (TDR_FALSE == bDyn) && (TDR_TYPE_STRING == a_pstEntry->iType))
    {
        bDyn = TDR_TRUE;
    } 
	if ( (TDR_FALSE == bDyn) && (TDR_TYPE_WSTRING == a_pstEntry->iType))
	{
		bDyn = TDR_TRUE;
	} 

    if ((TDR_FALSE == bDyn) && (TDR_INVALID_INDEX != a_pstEntry->ptrMeta))
    {
        LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, a_pstEntry->ptrMeta);
        if (!TDR_META_IS_FIXSIZE(pstTypeMeta))
        {
            bDyn = TDR_TRUE;
        }
    }

    if (TDR_FALSE == bDyn)
    {
        TDR_ENTRY_SET_FIXSIZE(a_pstEntry);
    }

	/*string wstring类型 设置缺省sizeinfo属性*/
	if (((TDR_TYPE_STRING == a_pstEntry->iType) || (TDR_TYPE_WSTRING == a_pstEntry->iType)) && (0 >= a_pstEntry->stSizeInfo.iUnitSize))
	{
		LPTDRCTYPEINFO pstType;
		a_pstEntry->stSizeInfo.idxSizeType = tdr_typename_to_idx(TDR_STRING_DEFULT_SIZEINFO_VALUE);
		pstType = tdr_idx_to_typeinfo(a_pstEntry->stSizeInfo.idxSizeType);
		assert(NULL != pstType);
		a_pstEntry->stSizeInfo.iUnitSize = pstType->iSize;
	}

	
	
	/*检查sortmethod属性，目前不支持对存储单元不固定的数组进行排序*/
	if (a_pstEntry->chOrder != TDR_SORTMETHOD_NONE_SORT)
	{
		TDRBOOLEAN bDynEntry = TDR_FALSE;

		if (((TDR_TYPE_WSTRING == a_pstEntry->iType) || (TDR_TYPE_STRING == a_pstEntry->iType)) &&
			(0 >= a_pstEntry->iCustomHUnitSize))
		{
			bDynEntry = TDR_TRUE;
		}
		if (TDR_INVALID_PTR != a_pstEntry->ptrMeta)
		{
			LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, a_pstEntry->ptrMeta);
			if (TDR_META_IS_VARIABLE(pstTypeMeta))
			{
				bDynEntry = TDR_TRUE;
			}
		}
		if (TDR_TRUE == bDynEntry)
		{
			LPTDRCTYPEINFO pstTypeInfo = tdr_idx_to_typeinfo(a_pstEntry->idxType);
			fprintf(a_fpError, "error:\t %s结构的成员<name = %s>其类型为<%s>,此成员为单元存储空间不固定的数组，目前不支持对其进行排序\n",
				a_pstMeta->szName, a_pstEntry->szName, pstTypeInfo->pszName);
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_SORTMETHOD_VALUE);
		}
	}/*if (a_pstEntry->chOrder != TDR_SORTMETHOD_NONE_SORT)*/
    
	/*检查版本属性*/
	if ((a_pstEntry->iVersion < a_pstMeta->iBaseVersion) && (pstLib->iXMLTagSetVer > TDR_XML_TAGSET_VERSION_0))
	{
		/*结构成员的版本值不能小于其基础版本*/
		fprintf(a_fpError, "error:\t %s结构的成员<name=%s>的版本<%d>不能比结构的基础版本<%d>低.\n", a_pstMeta->szName, a_pstEntry->szName,
			a_pstEntry->iVersion, a_pstMeta->iBaseVersion);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_VERSION);
	}
	if ((TDR_INVALID_PTR != a_pstEntry->ptrMeta) && (pstLib->iXMLTagSetVer > TDR_XML_TAGSET_VERSION_0))
	{
		LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(pstLib, a_pstEntry->ptrMeta);
		if (a_pstEntry->iVersion < pstTypeMeta->iBaseVersion)
		{
			/*结构成员的版本值不能小于其基础版本*/
			fprintf(a_fpError, "error:\t %s结构的成员<name=%s>是复合数据类型<type=%s>,此成员的版本<%d>不能比其数据类型的基础版本<%d>低.\n", a_pstMeta->szName, a_pstEntry->szName,
				pstTypeMeta->szName, a_pstEntry->iVersion, pstTypeMeta->iBaseVersion);
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_VERSION);
		}
	}

    return iRet;
}



LPTDRMETAENTRY tdr_get_entry_by_id_i(TDRMETAENTRY pstEntry[], int iMax, int iID)
{
    int i;

    assert( NULL != pstEntry );

    for(i=0; i<iMax; i++)
    {
        if (TDR_ENTRY_DO_HAVE_ID(pstEntry) && ( iID == pstEntry[i].iID ))
        {
            break;
        }
    }

    return (i < iMax) ? &pstEntry[i] : NULL;
}

int tdr_get_entry_type_i(LPTDRMETAENTRY a_pstEntry, LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError)
{
    scew_attribute *pstAttr = NULL;
    LPTDRMETALIB pstLib =NULL;
    const char *pszType = NULL;
    int idx = TDR_INVALID_INDEX;
    LPTDRCTYPEINFO pstTypeInfo = NULL;
    int iRet = TDR_SUCCESS;
    char szType[1024] = {0};

    assert(NULL != a_pstElement);
    assert(NULL != a_pstEntry);
    assert(NULL != a_pstMeta);
    assert(NULL != a_fpError);


    pstLib = TDR_META_TO_LIB(a_pstMeta);

    /* type attribute must be exist. */
    pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_TYPE);    
    if( NULL == pstAttr )
    {
        fprintf(a_fpError, "error:\t 自定义类型<name = %s>的成员元素<name = %s>没有包含type属性.\n",
            a_pstMeta->szName, a_pstEntry->szName);
        
        return TDR_ERRIMPLE_ENTRY_NO_TYPE;
    }

    tdr_normalize_string(&szType[0], sizeof(szType), scew_attribute_value(pstAttr));
    pszType = &szType[0];
    if (*pszType == '\0')
    {
        fprintf(a_fpError, "error:\t 结构<name = %s>的成员元素<name = %s>的type属性不能为空串.\n",
            a_pstMeta->szName, a_pstEntry->szName);
        
        return TDR_ERRIMPLE_ENTRY_NO_TYPE;
    }


    /*tagsetversion1支持指针和引用*/
    if (TDR_XML_TAGSET_VERSION_0 < pstLib->iXMLTagSetVer)
    {
        if (TDR_TAG_POINTER_TYPE == *pszType)
        {
			TDR_ENTRY_SET_POINT_TYPE(a_pstEntry);
            pszType++;
        }else if (TDR_TAG_REFER_TYPE == *pszType)
        {
			TDR_ENTRY_SET_REFER_TYPE(a_pstEntry);
            pszType++;
        }
    }


    /*获取entry的type*/
    idx = tdr_typename_to_idx(pszType);   
    if (TDR_INVALID_INDEX != idx)
    { 
        /*内置数据类型*/
        pstTypeInfo = tdr_idx_to_typeinfo(idx); 
        if (TDR_TYPE_COMPOSITE < pstTypeInfo->iType)
        {   
            a_pstEntry->idxType = idx;       
            a_pstEntry->iType	 = pstTypeInfo->iType;   
            a_pstEntry->iHUnitSize = pstTypeInfo->iSize;
			a_pstEntry->iNUnitSize = pstTypeInfo->iSize;
        }else/*if (TDR_TYPE_COMPOSITE < pstTypeInfo->iType)*/
        {
            fprintf(a_fpError, "error:\t 结构<name = %s>的成员元素<name = %s>的type属性值<%s>无效.\n",
                a_pstMeta->szName, a_pstEntry->szName, pszType);
            
            iRet = TDR_ERRIMPLE_ENTRY_INVALID_TYPE_VALUE;
        }/*if (TDR_TYPE_COMPOSITE < pstTypeInfo->iType)*/ 

    }else /*if (TDR_INVALID_INDEX != idx)*/
    {   
        /*自定义类型*/
        LPTDRMETA pstTypeMeta = tdr_get_meta_by_name_i(pstLib, pszType);
		if ((NULL == pstTypeMeta) ||
			((0 >= pstTypeMeta->iNUnitSize) && !TDR_ENTRY_IS_POINTER_TYPE(a_pstEntry) && !TDR_ENTRY_IS_REFER_TYPE(a_pstEntry)))
		{
			fprintf(a_fpError, "error:\t 自定义类型<name = %s>的成员元素<name = %s>的类型<type=%s>没有定义,除指针和引用外，成员的数据类型不支持后向引用。\n",
				a_pstMeta->szName, a_pstEntry->szName, szType);

			iRet = TDR_ERRIMPLE_ENTRY_INVALID_TYPE_VALUE;
		}else
		{
			a_pstEntry->ptrMeta = pstTypeMeta->ptrMeta;
			a_pstEntry->idxType = pstTypeMeta->idxType;
			a_pstEntry->iType = pstTypeMeta->iType;   
			a_pstEntry->iNUnitSize = pstTypeMeta->iNUnitSize;
			a_pstEntry->iHUnitSize = pstTypeMeta->iHUnitSize;
		}		
    }/*if (TDR_INVALID_INDEX != idx)*/


    /*如果是指针和引用类型，则重新计算unitsize*/
    if (!TDR_ERR_IS_ERROR(iRet))
    {
        if (TDR_ENTRY_IS_POINTER_TYPE(a_pstEntry))
        {
			a_pstEntry->iHUnitSize = TDR_POINTER_UINT_SIZE;
			a_pstEntry->iNUnitSize = 0;
        }
		if (TDR_ENTRY_IS_REFER_TYPE(a_pstEntry))
		{
			a_pstEntry->iHUnitSize = TDR_POINTER_UINT_SIZE;
		}
    }/*if (!TDR_ERR_IS_ERROR(iRet))*/


	/*void 类型支持void指针类型*/
	if (!TDR_ERR_IS_ERROR(iRet) && (a_pstEntry->iType == TDR_TYPE_VOID) && !(TDR_ENTRY_IS_POINTER_TYPE(a_pstEntry)))
	{
		fprintf(a_fpError, "error:\t 自定义类型<name = %s>的成员元素<name = %s>的类型为<type=%s>，目前只支持通用指针（*void）数据类型。\n",
			a_pstMeta->szName, a_pstEntry->szName, szType);

		iRet = TDR_ERRIMPLE_ENTRY_INVALID_TYPE_VALUE;
	}
	
    return iRet;
}

int tdr_sortkeyinfo_to_path_i(LPTDRMETALIB a_pstLib, LPTDRSORTKEYINFO a_pstSortKey, char* a_pszBuff, int a_iBuff)
{
	int iRet = TDR_SUCCESS;
	int iCurOff = 0;
	LPTDRMETAENTRY pstEntry = NULL;
	LPTDRMETA pstSearchMeta = NULL;
	int iTempLen = 0;
	char *pszBuff = NULL;
	int iLeftLen = 0;
	TDRBOOLEAN bGetFirstName = TDR_FALSE;
	int i = 0;
	char *pszName = NULL;
	int iSearchHOff;

	assert(NULL != a_pstLib);
	assert(NULL != a_pszBuff);
	assert(0 < a_iBuff);
	assert(NULL != a_pstSortKey);
	assert(TDR_INVALID_PTR != a_pstSortKey->ptrSortKeyMeta);
	assert(TDR_INVALID_OFFSET != a_pstSortKey->iSortKeyOff);
	

	i = 0;
	pszBuff = a_pszBuff;
	iLeftLen = a_iBuff;
	pstSearchMeta = TDR_PTR_TO_META(a_pstLib, a_pstSortKey->ptrSortKeyMeta);
	iSearchHOff = a_pstSortKey->iSortKeyOff;
	while (iCurOff < iSearchHOff )
	{
		if ( i >= pstSearchMeta->iEntriesNum )  
		{   
			/*已经搜索完所有entry，但没有entry的偏移和指定的偏移值匹配*/
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
		pszName = pstEntry->szName;
		
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
			pstSearchMeta = TDR_PTR_TO_META(a_pstLib, pstEntry->ptrMeta);
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
	if (0 == a_pstSortKey->idxSortEntry)
	{
		while( NULL != pstSearchMeta )
		{
			pstEntry = pstSearchMeta->stEntries;  /*取第一个元素*/ 
			pszName = pstEntry->szName;
			

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

			if (pstSearchMeta->ptrMeta == a_pstSortKey->ptrSortKeyMeta) 
			{
				break;
			}

			/*如果此meta不是要查找的meta，则*/
			if (( TDR_INVALID_PTR == pstEntry->ptrMeta ) ||
				TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
			{
				pstSearchMeta = NULL;
				break;
			}else
			{
				pstSearchMeta = TDR_PTR_TO_META(a_pstLib, pstEntry->ptrMeta);        
			}       
		}/*while( NULL != pstSearchMeta )*/
	}/*if (0 == a_pstSortKey->idxSortEntry)*/
	


	return iRet;
}



int tdr_get_entry_extendtotable_attribute_i(LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;
	char szVal[256] = {0};


	assert(NULL != a_pstEntry);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_EXTENDTOTABLE);    
	if (NULL == pstAttr)
	{
		return TDR_SUCCESS;
	}

	/*只有struct成员指定此数据才有效*/
	if (TDR_TYPE_STRUCT != a_pstEntry->iType)
	{
		fprintf(a_fpError, "\nWarnnig:\t entry元素<name = %s>不是struct数据类型，其extendtotable属性将忽略。\n",
			a_pstEntry->szName);
		return TDR_SUCCESS;
	}

	tdr_normalize_string(&szVal[0], sizeof(szVal), scew_attribute_value(pstAttr));
	if (0 == tdr_stricmp(szVal, TDR_TAG_TRUE))
	{
		TDR_ENTRY_SET_EXTENDABLE(a_pstEntry);			
	}else if ( 0== tdr_stricmp(szVal, TDR_TAG_FALSE))
	{
		TDR_ENTRY_CLR_EXTENDABLE(a_pstEntry);
	}else
	{
		fprintf(a_fpError, "\nerror:\t entry元素<name = %s>的extendtotable属性值<%s>无效,其值为\"false\"或者\"true\"\n",
			a_pstEntry->szName, szVal);

		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_EXTENDTOTABLE_VALUE);
	}/*if ( 0== strcmp(pszVal, TDR_TAG_FALSE))*/


	return iRet;
}

/** 获取bindmacrosgroup属性信息
*/
int tdr_get_entry_bindmacrosgroup_attribute_i(IN LPTDRMETALIB a_pstLib, LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement, FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;
	char szName[TDR_NAME_LEN] = {0};
	LPTDRMACROSGROUP pstTmpGroup;

	assert(NULL != a_pstEntry);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);


	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_BIND_MACROSGROUP);    
	if (NULL == pstAttr)
	{
		return TDR_SUCCESS;
	}

	/*只有整数类型成员指定此数据才有效*/
	if ((TDR_TYPE_CHAR > a_pstEntry->iType) || (TDR_TYPE_ULONG < a_pstEntry->iType))
	{
		fprintf(a_fpError, "\nWarnnig:\t entry元素<name = %s>不是整数数据类型，其%s属性将忽略。\n",
			a_pstEntry->szName, TDR_TAG_BIND_MACROSGROUP);
		return TDR_SUCCESS;
	}

	tdr_normalize_string(&szName[0], sizeof(szName), scew_attribute_value(pstAttr));
	pstTmpGroup = tdr_find_macrosgroup_i(a_pstLib, szName);
	if (NULL != pstTmpGroup)
	{
		a_pstEntry->ptrMacrosGroup = (TDRPTR)((char *)pstTmpGroup - (char *)a_pstLib - offsetof(TDRMETALIB, data));
	}else
	{
		fprintf(a_fpError, "error:\t entry元素<name = %s>的%s属性值<%s>无效,在元数据库中找不到相应的宏定义组\n",
			a_pstEntry->szName, TDR_TAG_BIND_MACROSGROUP, szName);

		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_BINDMACROSGROUP);
	}/*if ( 0== strcmp(pszVal, TDR_TAG_FALSE))*/


	return iRet;
}


int tdr_get_entry_autoincrement_attribute_i(LPTDRMETA a_pstMeta, LPTDRMETAENTRY a_pstEntry, scew_element *a_pstElement,FILE *a_fpError)
{
	scew_attribute *pstAttr = NULL;
	int iRet = TDR_SUCCESS;
	char szValue[256] = {0};

	assert(NULL != a_pstEntry);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);
	assert(NULL != a_pstMeta);


	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_AUTOINCREMENT);    
	if (NULL == pstAttr)
	{
		return TDR_SUCCESS;
	}

	/*只有整数类型成员指定此数据才有效*/
	if ((TDR_TYPE_CHAR > a_pstEntry->iType) || (TDR_TYPE_ULONGLONG < a_pstEntry->iType))
	{
		fprintf(a_fpError, "\nError:\t 结构体<name=%s>的成员<name = %s type=%d>不是整数数据类型，不能定义%s属性。\n",
			a_pstMeta->szName, a_pstEntry->szName, a_pstEntry->iType,TDR_TAG_AUTOINCREMENT);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_AUTOINCREMENT);
	}

	tdr_normalize_string(&szValue[0], sizeof(szValue), scew_attribute_value(pstAttr));
	if (0 == tdr_stricmp(szValue, TDR_TAG_TRUE))
	{
		TDR_ENTRY_SET_AUTOINCREMENT(a_pstEntry);
	}else if ( 0== tdr_stricmp(szValue, TDR_TAG_FALSE))
	{
		TDR_ENTRY_SET_AUTOINCREMENT(a_pstEntry);
	}else
	{
		fprintf(a_fpError, "\nerror:\t 结构体<name=%s>的成员<name = %s>的%s属性值<%s>无效,其值为\"false\"或者\"true\"\n",
			a_pstMeta->szName, a_pstEntry->szName, TDR_TAG_AUTOINCREMENT, szValue);

		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_AUTOINCREMENT);
	}/*if ( 0== tdr_stricmp(szValue, TDR_TAG_FALSE))*/

	if (1 != a_pstEntry->iCount)
	{
		fprintf(a_fpError, "\nerror:\t 结构体<name=%s>的成员<name = %s>的%s属性无效,只有非数组成员才能定义此属性\n",
			a_pstMeta->szName, a_pstEntry->szName, TDR_TAG_AUTOINCREMENT);

		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_AUTOINCREMENT);
	}

	if (TDR_META_DO_HAVE_AUTOINCREMENT_ENTRY(a_pstMeta))
	{
		fprintf(a_fpError, "\nerror:\t 结构体中只能有一个成员定义%s属性，结构体<name=%s>中已经包含了定义%s属性的成员，成员<name = %s>不能再定义此属性\n",
			TDR_TAG_AUTOINCREMENT, a_pstMeta->szName, TDR_TAG_AUTOINCREMENT, a_pstEntry->szName);

		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_AUTOINCREMENT);
	}else
	{
		TDR_META_SET_HAVE_AUTOINCREMENT_ENTRY(a_pstMeta);
	}
	

	return iRet;
}

/**读取成员的customattr属性 
*/
int tdr_get_customattr_attribute_i(IN LPTDRMETA a_pstMeta, IN LPTDRMETAENTRY a_pstEntry, IN scew_element *a_pstElement,
								   IN FILE *a_fpError)
{
	int iRet = TDR_SUCCESS;
	scew_attribute *pstAttr = NULL;
	const char *pszValue;
	int iStrLen = 0;
	LPTDRMETALIB pstLib;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstEntry);
	assert(NULL != a_pstElement);
	assert(NULL != a_fpError);

	pstAttr = scew_attribute_by_name(a_pstElement, TDR_TAG_CUSTOMATTR);    
	if( NULL == pstAttr )
	{
		return TDR_SUCCESS;
	}	
	pszValue = scew_attribute_value(pstAttr);
	if (NULL == pszValue)
	{
		return TDR_SUCCESS;
	}


	iStrLen = strlen(pszValue) + 1; /*加1是为了保留结束字符*/	
	if (iStrLen > TDR_MAX_CUMTOMATTR_VALUE_LEN)
	{
		fprintf(a_fpError, "\nerror:\t 复合数据类型<name=%s>的成员<name = %s>的%s属性值"
			"长度(%d)超过了最大长度<%d>限制", 
			a_pstMeta->szName,  a_pstEntry->szName, TDR_TAG_CUSTOMATTR,
			iStrLen, TDR_MAX_CUMTOMATTR_VALUE_LEN); 
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUSTOMATTR);
	}

	pstLib = TDR_META_TO_LIB(a_pstMeta);
	if ((1 < iStrLen) && (iStrLen > pstLib->iFreeStrBufSize))
	{
		fprintf(a_fpError, "\nerror:\t metalib字符串缓冲区剩余空间<freeSpace = %d>, 不能存储custom属性: %s",
			pstLib->iFreeStrBufSize, pszValue); 

		iRet = TDR_ERRIMPLE_NO_SPACE_FOR_STRBUF;
	}else
	{
		TDR_COPY_STRING_TO_BUF(a_pstEntry->ptrCustomAttr, pszValue, iStrLen, pstLib);
	}


	return iRet;
}

