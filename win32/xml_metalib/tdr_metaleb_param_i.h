/**
*
* @file     tdr_metaleb_param_i.h  
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

#ifndef TDR_METALIB_PARAM_I_H
#define TDR_METALIB_PARAM_I_H


#include <stdio.h>
#include <scew/scew.h>

#include "tdr/tdr_define.h"
#include "tdr/tdr_types.h"


/**初始化metalib参数库
*参数结构体的空间在函数外分配
* @pre \e a_pstParam 不能为空
*/
void tdr_init_lib_param_i(LPTDRLIBPARAM a_pstParam);

/** 
*根据XML解析树的信息添加metalib的参数，如果处理过程中发现错误将
*记录到fpError指向的文件中
*@param[out] a_pstParam 需要添加更新的参数结构
*@param[in] a_pstTree XML解析树
*@param[in] a_iTagsVersion XML标签集版本
*@param[in] a_fpError 记录处理错误信息的文件句柄
*@retval <0 错误代码
*@retval 0 成功
*@retval >0 成功，但出现特殊情况
*
*@pre \e a_pstParam 不能为NULL
*@pre \e a_pstTree 不能为NULL
*@pre \e a_fpError 不能为NULL
*@pre \e a_iTagsVersion 为合法的版本号
*/
int tdr_add_lib_param_i(LPTDRLIBPARAM a_pstParam, IN scew_tree* a_pstTree, IN int a_iTagsVersion, IN FILE *a_fpError);


/** 
*根据XML解析树的生成metalib的参数，如果处理过程中发现错误将
*记录到fpError指向的文件中
*@param[out] a_pstParam 需要添加更新的参数结构
*@param[in] a_pstTree XML解析树
*@param[in] a_iTagsVersion XML标签集版本
*@param[in] a_fpError 记录处理错误信息的文件句柄
*@retval <0 错误代码
*@retval 0 成功
*@retval >0 成功，但出现特殊情况
*
*@pre \e a_pstParam 不能为NULL
*@pre \e a_pstTree 不能为NULL
*@pre \e a_fpError 不能为NULL
*/
int tdr_get_lib_param_i(LPTDRLIBPARAM a_pstParam, IN scew_tree* a_pstTree, IN int a_iTagsVersion, IN FILE *a_fpError);


/**获取单个结构体所需内存空间
*@param[out] a_piEntirsNum 获取此meta包含的entry的数目
*@param[out] a_piMetaSize 获取此meta元素存储所占用的字节数
*@param[out] a_piStrBufSize 获取此meta元素在字符串缓冲区中所占的字节数
*@param[in] a_iTagversion 此meta描述标签的版本
*@param[in] a_pstElement 需考察的元素
*@param[in] a_pstRoot 根元素
*/
int tdr_get_meta_size_i(int *a_piEntirsNum, int *a_piMetaSize, int *a_piStrBufSize, IN int a_iTagversion,
						scew_element *a_pstElement, scew_element *a_pstRoot);

/**获取单个宏定义组所需内存空间
*@param[out] a_piMacrosNum 获取此宏定义组包含的macro数目
*@param[out] a_piGroupSize 获取此宏定义组元素存储所占用的字节数
*@param[out] a_piStrBufSize 获取此meta元素在字符串缓冲区中所占的字节数
*@param[in] a_pstElement 需考察的元素
*/
int tdr_get_macrosgroup_size_i(int *a_piMacrosNum, int *a_piGroupSize, int *a_piStrBufSize, scew_element *a_pstElement);



/** 
*取出XML解析树根元素的version属性值，如果处理过程中发现错误将
*记录到fpError指向的文件中
*@param[out] a_plVersion 版本值
*@param[in] pstTree XML解析树
*@param[in] fpError 记录处理错误信息的文件句柄
*@retval <0 错误代码
*@retval 0 成功
*@retval >0 成功，但出现特殊情况
*
*@pre \e a_plVersion 不能为NULL
*@pre \e pstTree 不能为NULL
*@pre \e fpError 不能为NULL
*/
int tdr_get_metalib_version_i(OUT long *a_plVersion, IN scew_element* a_pstRoot);

/**获取metalib的tagsversion属性
*/
int tdr_get_metalib_tagsversion_i(OUT int *a_piTagVersion, IN scew_element *pstRoot, IN int iTagsVersion);

#endif /* TDR_METALIB_PARAM_I_H*/
