/**
*
* @file     tdr_metalib_manage_i.h    
* @brief    TSF4G-元数据描述库管理模块
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-07-19 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#ifndef TDR_METALIB_MANAGE_I_H
#define TDR_METALIB_MANAGE_I_H


#include "tdr/tdr_types.h"
#include "tdr_metalib_kernel_i.h"
#include "tdr_iostream_i.h"




/**根据网络偏移植，在meta中定位此偏移值对应的路径 
*@param[in] a_pstMeta 需要定位的meta
*@param[in iEntry   当meta中第iEntry个entry的偏移与iOff匹配时，将相应名字记录为this
*@param[in] a_iNetOff    需要定位的偏移
*/
int tdr_netoff_to_path_i(LPTDRMETA a_pstMeta, int a_iEntry, int a_iNetOff, char* a_pszBuff, int a_iBuff);


/**根据本地偏移植，在meta中定位此偏移值对应的路径 
*@param[in] a_pstMeta 需要定位的meta
*@param[in iEntry   当meta中第iEntry个entry的偏移与iOff匹配时，将相应名字记录为this
*@param[in] a_iHostOff    需要定位的偏移
*/
int tdr_hostoff_to_path_i(LPTDRMETA a_pstMeta, int a_iEntry, int a_iHostOff, char* a_pszBuff, int a_iBuff);



/**从DR路径中取出第1级节点的名字
*@return 返回下一级节点指针
*/
char *tdr_get_first_node_name_from_path_i(char *pszName, int iNameSize, const char *pszPath);

/** 根据target元素路径获取此元素在meta中的偏移
*@param[out] a_pstRedirector 此元素的偏移
*@param[in] a_pstMeta pstEntry所在的自定义结构
*@param[in] a_iEntry 在前iEntry个元素中查找
*@param[in] a_pszName 要处理的名字
*
*@return 成功返回TDR_SUCCESS，否则返回错误代码
*
*@pre \e a_piOff 不能为NULL
*@pre \e a_pstMeta 不能为NULL
*@pre \e a_piUnit 不能为NULL
*@pre \e a_pszName 不能为NULL
*@pre \e 必须先获取元素的type,count ,refer属性
*/
int tdr_sizeinfo_to_off_i(LPTDRSIZEINFO a_pstRedirector, LPTDRMETA a_pstMeta, int a_iEntry, const char* a_pszName);


/**根据entry名查找entry在meta中的索引
*/
int tdr_get_entry_by_name_i(TDRMETAENTRY pstEntry[], int iMax, const char* pszName);






/** 根据mata名字在元数据库中查找元素
*@param[in] pstLib 需要更新的元数据库
*@param[in] pszName 要查找的meta的名字
*
*@return 成功返回meta的指针，否则返回NULL
*
*@pre \e pstLib 不能为NULL
*@pre \e pszName 不能为NULL
*/
LPTDRMETA tdr_get_meta_by_name_i(TDRMETALIB* pstLib, const char* pszName);




/** 根据宏名获得其数值
*@param[out] a_piID 获得其数值
*@param[out] a_piIdx 获得其macro定义的索引
*@param[in] a_pstLib 需要更新的元数据库
*@param[in] a_pszValue 宏名
*
*@return 成功返回0，否则返回非0
*
*@pre \e a_piID 不能为NULL
*@pre \e a_piIdx 不能为NULL
*@pre \e a_pstLib 不能为NULL
*@pre \e a_pszValue 不能为NULL
*/
int tdr_get_macro_int_i(int* a_piID, int* a_piIdx, TDRMETALIB* a_pstLib, const  char *a_pszValue);

/** 根据宏定义名字在元数据库中查找宏定义信息的索引
*@param[inout] pstLib 元数据库
*@param[in] pszName 宏定义名字
*
*@return 成功返回宏定义的索引，否则返回TDR_INVALID_INDEX
*
*@pre \e pstLib 不能为NULL
*@pre \e pszName 不能为NULL
*/
int tdr_get_macro_index_by_name_i(TDRMETALIB* pstLib, const char* pszName);


int tdr_parse_meta_sortkey_info_i(IN LPTDRSORTKEYINFO a_pstSortKey, LPTDRMETA a_pstMeta, const char *a_pszSortKeyPath);

/** 根据元素路径获取此元素在meta中的偏移
*@param[in] a_pstMeta pstEntry所在的自定义结构
*@param[out] a_pstRedirector 此元素的偏移
*@param[in] a_iEntry 在前iEntry个元素中查找
*@param[in] a_pszName 要处理的名字
*@note 如果a_iEntry等于TDR_INVALID_INDEX,则表示为meta的属性，将在全部子元素中查找
*
*@return 成功返回TDR_SUCCESS，否则返回错误代码
*
*@pre \e a_piOff 不能为NULL
*@pre \e a_pstMeta 不能为NULL
*@pre \e a_piUnit 不能为NULL
*@pre \e a_pszName 不能为NULL
*/
int tdr_name_to_off_i(IN LPTDRMETA a_pstMeta, OUT LPTDRSelector a_pstSelector, IN int a_iEntry, IN const char* a_pszName);

/*根据成员的值获取此值绑定的宏定义，只有当成员定义了macrosgroup属性时才有效*/
LPTDRMACRO tdr_get_bindmacro_by_value(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, IN int a_iValue);

/*根据指定名字获取此值绑定的宏定义，只有当成员定义了macrosgroup属性时才有效*/
LPTDRMACRO tdr_get_bindmacro_by_name(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, IN const char *a_pszName);

/**输出基本数据类型成员的值
*@param[in] a_pstIOStream 输出信息的流句柄
*@param[in] a_pstLib 元数据描述库指针
*@param[in] a_pstEntry 成员的描述结构指针
*@param[in,out] a_ppszHostStart 此成员值存储空间的起始地址
*@param[in] a_pszHostEnd 此成员值存储空间的结束地址
*@return 成功返回0，否则返回非零值
*/
int tdr_ioprintf_basedtype_i(IN LPTDRIOSTREAM a_pstIOStream, IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, 
									INOUT char **a_ppszHostStart, IN const char *a_pszHostEnd);

/**根据从字符串值中输入基本数据类型成员的值
*@param[in] a_pstLib 元数据描述库指针
*@param[in] a_pstEntry 成员的描述结构指针
*@param[in] a_pszHostStart 此成员值存储空间的起始地址
*@param[in,out] a_piSize 保存输入数据内存空间大小的指针
*	-	输入	可使用的内存空间大小
*	-	输出	实际以使用的内存空间大小
*@return 成功返回0，否则返回非零值
*/
int tdr_ioscanf_basedtype_i(IN LPTDRMETALIB a_pstLib, IN LPTDRMETAENTRY a_pstEntry, 
							IN char *a_pszHostStart, INOUT int *a_piSize, IN const char *a_pszValue);

#endif /*TDR_METALIB_MANAGE_I_H*/
