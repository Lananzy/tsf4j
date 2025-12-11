/**
*
* @file     Tdr_ctypes_info.h
* @brief    DR支持的C/C++基本数据类型信息
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-03-26 
*
* 定义DR支持的c/c++各内置数据类型的详细信息，并提供访问方法
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#ifndef TDR_CTYPES_INFO_H
#define TDR_CTYPES_INFO_H

#include "tdr/tdr_types.h"


/**
* This is the type info for c/c++ base date type info.
*/
typedef struct tagTDRCTypeInfo	TDRCTYPEINFO;

/**
*brief struct to store c/c++ base type info
*/
struct tagTDRCTypeInfo
{
    char* pszName;		///< type name
    char* pszCName;		///< name used in c/c++ head file. */
    char* pszSQLName;   /* name used in sql */
    char* pszSPrefix;	/* single prefix  in c/c++ head file. */
    char* pszMPrefix;	/* multiple prefix(for array). */
    
    int iType;
    int iSize;
    unsigned int uiFlags;
};


#pragma pack(1)

/**tdr存取date基本数据类型的结构 
*/
struct tagTDRDate 
{
	short nYear;
	unsigned char bMonth;
	unsigned char bDay;
};
typedef struct tagTDRDate TDRDATE;
typedef struct tagTDRDate *LPTDRDATE;

/**tdr存取time基本数据类型的结构 
*/
struct tagTDRTime 
{
	short nHour;  /**<小时*/
	unsigned char bMin; /**<分钟*/
	unsigned char bSec; /**<秒*/
};
typedef struct tagTDRTime TDRTIME;
typedef struct tagTDRTime *LPTDRTIME;

/**tdr存取datetime基本数据类型的结构 
*/
struct tagTDRDateTime 
{
	TDRDATE stDate;  /**<日期*/
	TDRTIME stTime;   /**<时间*/
};
typedef struct tagTDRDateTime TDRDATETIME;
typedef struct tagTDRDateTime *LPTDRDATETIME;

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/**
*@brief 根据类型名获取此类型信息的在详细信息结构中的索引
*
*@param name 类型名
*@param iXMLTagVer 元素数据的XML标签定义版本
*@retval >=0 有效的存储索引
*@retval TDR_INVALID_INDEX 无效索引值，即没有此类型定义
*/   
int tdr_typename_to_idx(IN const char *name);
    
/**根据索引获取类型信息的指针
*/
LPTDRCTYPEINFO tdr_idx_to_typeinfo(IN int iIdx);


    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TDR_CTYPES_INFO_H */
