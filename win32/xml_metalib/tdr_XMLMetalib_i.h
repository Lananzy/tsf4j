/**
*
* @file     tdr_XMLMetalib_i.h 
* @brief    元数据描述库与XML描述相互转换 内部函数
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

#ifndef TDR_XMLMETALIB_I_H
#define TDR_XMLMETALIB_I_H

#include <scew/scew.h>
#include <stdio.h>

/**根据XML文件句柄创建XML元素树 
*/
int tdr_create_XMLParser_tree_byFileName(scew_tree **a_ppstTree, const char *a_pszFileName, FILE *a_fpError);

/**根据XML缓冲信息生成XML元素树
*@param[out] a_ppstTree 创建的元素树的指针
*@Param[in]	a_pszXml 保存XML信息的缓冲区地址
*@Param[in]	a_iXml 保存XML信息的缓冲区大小
*@param[in]	a_fpError	用来输出错误信息的文件句柄
*@return 成功返回0，否则返回错误代码
*/
int tdr_create_XMLParser_tree_byBuff_i(scew_tree **a_ppstTree, IN const char* a_pszXml, IN int a_iXml, FILE* a_fpError);

/**根据XML文件句柄创建XML元素树 
*/
int tdr_create_XMLParser_tree_byfp(scew_tree **a_ppstTree, FILE *a_fpXML, FILE *a_fpError);

/**根据参数为元数据库分配空间，并初始化
*@param[out] pstLib 创建的元数据库
*@Param[in]	stLibParam 初始化元数据的参数
*@return 成功返回0，否则返回错误代码
*/
int tdr_init_metalib_i(LPTDRMETALIB *ppstLib, LPTDRLIBPARAM pstLibParam);

int tdr_input_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot, 
				IN int a_iCutOffVersion, IN int a_iIOVersion);

#endif /*TDR_XMLMETALIB_I_H*/
