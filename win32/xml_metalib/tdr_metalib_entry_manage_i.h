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

#ifndef TDR_METALIB_ENTRY_MANAGE_H
#define TDR_METALIB_ENTRY_MANAGE_H

#include <scew/scew.h>

#include "tdr/tdr_types.h"
#include "tdr_XMLtags_i.h"

/**< string wstring sizeinfo属性缺省值*/
#define  TDR_STRING_DEFULT_SIZEINFO_VALUE  TDR_TAG_INT

/** 将pstElement指向的元素作为成员加到meta结构中
*@param[out] a_pstMeta 保存此成员的自定义数据结构
*@param[in] a_pstElement 要处理的元素
*@param[in] a_fpError 保存错误信息的文件句柄
*
*@return 成功返回TDR_SUCCESS，否则返回错误代码
*
*@pre \e a_pstMeta 不能为NULL
*@pre \e a_pstElement 不能为NULL
*@pre \e a_fpError 不能为NULL
*/
int tdr_add_entry_to_meta_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);



/**根据本地偏移植，在meta中定位此偏移值对应的路径 
*@param[in] a_pstLib 需要定位的meta
*@param[in] a_pstSortKey    需要定位的偏移
*/
int tdr_sortkeyinfo_to_path_i(LPTDRMETALIB a_pstLib, LPTDRSORTKEYINFO a_pstSortKey, char* a_pszBuff, int a_iBuff);






#endif /*TDR_METALIB_ENTRY_MANAGE_H*/
