/**
*
* @file     tdr_metalib_meta_manage_i.h 
* @brief    TSF4G-元数据描述库meta核心管理模块
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


#ifndef TDR_METALIB_META_MANAGE_I_H
#define TDR_METALIB_META_MANAGE_I_H

#include <stddef.h>
#include <stdio.h>

#include <scew/scew.h>
#include "tdr/tdr_metalib_kernel_i.h"

/** 将宏定义元数加到原数据描述库
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
int tdr_add_macro_i(TDRMETALIB* a_pstLib, scew_element *a_pstElement, FILE *a_fpError, INOUT LPTDRMACRO *a_ppMacro);

/** 将struct/union 等meta元素的基本信息加到原数据库
*@param[inout] a_pstLib 需要更新的元数据库
*@param[in] a_pstElement 宏定义组元素
*@param[in] a_fpError 用来记录处理错误信息的文件句柄
*
*@return 成功返回TDR_SUCCESS,否则返回错误号
*
*@pre \e a_pstLib 不能为NULL
*@pre \e a_pstElement 不能为NULL
*@pre \e a_fpError 不能为NULL
*/
int tdr_add_meta_base_i(TDRMETALIB* a_pstLib, scew_element *a_pstElement, scew_element *a_pstRoot, FILE *a_fpError);

/**将meta元素的特殊属性读取到元数据库中 
*@param[inout] a_pstMeta 需要更新的meta
*@param[in] a_pstElement meta元素
*@param[in] a_fpError 用来记录处理错误信息的文件句柄
*/
int tdr_get_meta_specail_attribute_i(LPTDRMETA a_pstMeta, scew_element *a_pstElement, FILE *a_fpError);


/**将meta元素下的子元素添加到meta结构体中
*@param[out] a_pstMeta 正在处理的meta
*@param[in] a_stElement meta所指的元素
*/
int tdr_add_meta_entries_i(LPTDRMETA a_pstMeta, scew_element *a_stElement, FILE *a_fpError);

/**检查meta数据的有效性
*/
int tdr_check_meta_i(LPTDRMETA a_pstMeta, FILE *a_fpError);

/** 将tagsetversion=0版本的宏定义组下的宏加到原数据库
*@param[inout] a_pstLib 需要更新的元数据库
*@param[in] a_pstElement 宏定义组元素
*@param[in] a_fpError 用来记录处理错误信息的文件句柄
*
*@return 成功返回TDR_SUCCESS,否则返回错误号
*
*@pre \e a_pstLib 不能为NULL
*@pre \e a_pstElement 不能为NULL
*@pre \e a_fpError 不能为NULL
*/
int tdr_add_macros_i(TDRMETALIB* a_pstLib, scew_element *a_pstElement, FILE *a_fpError);



/** 根据cname属性
*@param[out] a_ptrName cname字符串在字符串缓冲区中的指针
*@param[out] a_shNameLen cname字符串的长度
*@param[in] a_pstLib 需要更新的元数据库
*@param[in] a_pstElement 要处理的元素
*@param[in] a_fpError 保存错误信息的文件句柄
*
*@return 成功返回meta的指针，否则返回NULL
*
*@pre \e a_ptrName 不能为NULL
*@pre \e a_shNameLen 不能小于等于0
*@pre \e a_pstLib 不能为NULL
*@pre \e a_pstElement 不能为NULL
*/
int tdr_get_cname_attribute_i(TDRPTR *a_pptrName, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError);



/** 读该元素的name属性性
*@param[out] pszName    保存名字的缓冲区
*@param[in] iNameSize name缓冲区的大小
*@param[in] pstElement 需添加的元素
*
*@return 成功返回TDR_SUCCESS,否则返回错误号
*
*@pre \e pszName 不能为NULL
*@pre \e iNameSize 必须大于0
*@pre \e pstElement 不能为NULL
*/
int tdr_get_name_attribute_i(char *pszName, int iNameSize, scew_element *pstElement);


/**读取元素的id属性
*@retval <0  失败
*@retval 0  成功读取到id属性
*@retval >0 没有id属性
*/
int tdr_get_id_attribute_i(int *a_piID, int *a_piIdx, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError);

/**获取元素的desc属性
*/
int tdr_get_desc_attribute_i(TDRPTR *a_pptrDesc, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError);




/**获取version属性
*/
int tdr_get_version_attribute_i(int *a_piVersion, int *a_piIdxVersion, LPTDRMETALIB a_pstLib, scew_element *a_pstElement, FILE *a_fpError);


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
int tdr_add_macrosgroup_i(TDRMETALIB* a_pstLib, scew_element *a_pstElement, FILE *a_fpError);


/**根据宏定义组的名称查找此宏定义组数据结构
*/
LPTDRMACROSGROUP tdr_find_macrosgroup_i(IN const LPTDRMETALIB a_pstLib, IN const char *a_pszName);

#endif /* TDR_METALIB_META_MANAGE_I_H */
