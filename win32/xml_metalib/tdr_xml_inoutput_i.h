/**
*
* @file     tdr_xml_inoutput_i.h 
* @brief    xml数据文件输入输出相关接口
* 
* @author steve jackyai  
* @version 1.0
* @date 2009-04-30
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#ifndef TDR_XML_INOUTPUT_I_H
#define TDR_XML_INOUTPUT_I_H


#include <stdio.h>
#include <scew/scew.h>
#include "tdr/tdr_define.h"
#include "tdr/tdr_types.h"

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


int tdr_input_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot, 
				IN int a_iCutOffVersion, IN int a_iIOVersion);


/**从以TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME格式存储的xml文件中读取某个节点的属性信息
*@param[in] a_pstMeta 保存属性信息的数据结构的描述句柄
*@param[in] a_pstNode xml节点
*@param[in] a_iFirstEntry 指定起始成员，即从a_iFirstEntry指定的成员开始读取
*@param[in] a_pszHostStart 保存数据信息的内存缓冲区起始地址
*@param[in] a_iSize	保存数据信息的内存缓冲区可用大小，单位为字节
*@param[in] a_iVersion 数据的版本
@return succ: zero, failed: nonzero
*/
int tdr_input_meta_attrs_i(IN LPTDRMETA a_pstMeta, IN scew_element *a_pstNode, IN int a_iFirstEntry,
						   IN char *a_pszHostStart,  IN int a_iSize, int a_iVersion);

/**从以TDR_XML_DATA_FORMAT_ATTR_ENTRY_NAME格式存储的xml文件中读取信息
*@param[in] a_pstMeta 保存属性信息的数据结构的描述句柄
*@param[in] a_pstRoot xml节点
*@param[in,out] a_pstHost 保存数据的缓冲区信息
	-	in	a_pstHost->iBuff	数据缓冲区的可用空间大小
	-	out a_pstHost->iBuff	数据缓冲区的实际使用大小
*@param[in] a_iCutOffVersion 数据的版本
@return succ: zero, failed: nonzero
*/
int tdr_input_by_attr_xml_i(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstHost, IN scew_element *a_pstRoot, 
							IN int a_iCutOffVersion);

#endif /* TDR_XML_INOUTPUT_I_H*/
