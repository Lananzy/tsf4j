/**
*
* @file     tdr_dbms_comm.h  
* @brief    TSF-G-DR 可以提供给上层进一步封装的DB结合处理函数
* 
* @author jackyai  
* @version 1.0
* @date 2007-12-13 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#ifndef TDR_DBMS_COMM_H
#define TDR_DBMS_COMM_H

#include "tdr/tdr_types.h"
#include "tdr/tdr_net.h"
#include "tdr/tdr_metalib_kernel_i.h"


/** 
按照连接的当前字符集，将“from”中的字符串编码为转义SQL字符串。将结果置于“to”中，并添加1个终结用NULL字节。
返回值是编码字符串的长度，不包括终结用Null字符。

*/
int tdr_dbms_escape_string(IN TDRDBHANDLE a_hDBHandle, OUT char *a_pszTo, IN char *a_pszFrom, int a_iLen);


/**将struct结构中数据打包成sql字符串，以便插入和更新此值
*打包的规则是：meta版本号（4字节）＋信息长度（4字节）＋调用hton编码后的信息串
*@param[in,out] a_pstSql 存储转换后的数据
*	- 输入	a_pstSql.pszBuff 指定保存本地结构信息的缓冲区起始地址
*	- 输入  a_pstSql.iBuff	指定缓冲区的大小
*	- 输出  a_pstSql.iBuff  实际打包后数据的总长度
*@param[in] a_hDBHandle 数据库处理句柄
*@param[in] a_pstMeta 数据的描述句柄
*@param[in] a_pstData 保存待转换数据的缓冲区
*	- 输入	a_pstData.pszBuff 保存待转换数据的缓冲区起始地址
*	- 输入  a_pstData.iBuff	指定缓冲区的大小
*@param[in] a_iVersion	剪裁版本
*
*@retval 0   处理成功
*@retval >0  处理成功，但发生某些特殊情况
*@retval <0  处理失败
*/
int tdr_dbms_meta2sql(INOUT LPTDRDATA a_pstSql, IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETA a_pstMeta, IN LPTDRDATA a_pstData, IN int a_iVersion);

/**将union联合体中数据打包成sql字符串，以便插入和更新此值
*打包的规则是：meta版本号（4字节）＋信息长度（4字节）＋调用hton编码后的信息串
*@param[in,out] a_pstSql 存储转换后的数据
*	- 输入	a_pstSql.pszBuff 指定保存本地结构信息的缓冲区起始地址
*	- 输入  a_pstSql.iBuff	指定缓冲区的大小
*	- 输出  a_pstSql.iBuff  实际打包后数据的总长度
*@param[in] a_hDBHandle 数据库处理句柄
*@param[in] a_pstMeta 数据的元数据描述句柄
*@param[in] a_idxEntry 选定成员索引
*@param[in] a_pstData 保存待转换数据的缓冲区
*	- 输入	a_pstData.pszBuff 保存待转换数据的缓冲区起始地址
*	- 输入  a_pstData.iBuff	指定缓冲区的大小
*@param[in] a_iVersion	剪裁版本
*
*@retval 0   处理成功
*@retval >0  处理成功，但发生某些特殊情况
*@retval <0  处理失败
*/
int tdr_dbms_union2sql(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, IN LPTDRMETA a_pstMeta, IN int  a_idxEntry, IN LPTDRDATA a_pstData, IN int a_iVersion);


/**将sql查询结果解包成struct结构
*解包的规则是：meta版本号（4字节）＋信息长度（4字节）＋调用ntoh编码后的信息串
*@param[in] a_pstMeta 数据的描述句柄
*@param[in,out] a_pstSql 存储待解包的数据
*	- 输入	a_pstSql.pszBuff 指定解包信息的缓冲区起始地址
*	- 输入  a_pstSql.iBuff	指定缓冲区的大小
*@param[in] a_pstData 保存转换后数据的缓冲区
*	- 输入	a_pstData.pszBuff 保存转换数据的缓冲区起始地址
*	- 输入  a_pstData.iBuff	指定缓冲区的大小
*	- 输出  a_pstData.iBuff  实际解包后数据的总长度
*@param[in] a_iVersion	剪裁版本
*
*@retval 0   处理成功
*@retval >0  处理成功，但发生某些特殊情况
*@retval <0  处理失败
*/
int tdr_dbms_sql2meta(IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstData, IN LPTDRDATA a_pstSql, IN int a_iVersion);

/**将sql查询结果解包成union结构
*解包的规则是：meta版本号（4字节）＋信息长度（4字节）＋调用ntoh编码后的信息串
*@param[in] a_pstMeta 数据的元数据描述句柄
*@param[in] a_idxEntry 选定成员索引
*@param[in,out] a_pstSql 存储待解包的数据
*	- 输入	a_pstSql.pszBuff 指定解包信息的缓冲区起始地址
*	- 输入  a_pstSql.iBuff	指定缓冲区的大小
*@param[in] a_pstData 保存转换后数据的缓冲区
*	- 输入	a_pstData.pszBuff 保存转换数据的缓冲区起始地址
*	- 输入  a_pstData.iBuff	指定缓冲区的大小
*	- 输出  a_pstData.iBuff  实际解包后数据的总长度
*@param[in] a_iVersion	剪裁版本
*
*@retval 0   处理成功
*@retval >0  处理成功，但发生某些特殊情况
*@retval <0  处理失败
*/
int tdr_dbms_sql2union(IN LPTDRMETA a_pstMeta, IN int  a_idxEntry,  INOUT LPTDRDATA a_pstData, IN LPTDRDATA a_pstSql, IN int a_iVersion);


#if defined(WIN32) &&  _MSC_VER < 1400
#define TDR_LONGLONG_ENTRY_VALUE2SQL_VALUE(a_iRet, a_pstEntry, a_pszSql, a_iSize, a_iWriteLen, a_pszHostStart) \
{																												\
	a_iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%I64i", (tdr_longlong)*(tdr_longlong *)a_pszHostStart);	\
	TDR_CHECK_BUFF(a_pszSql, a_iSize, a_iWriteLen, a_iRet);												\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
}																						
#else
#define TDR_LONGLONG_ENTRY_VALUE2SQL_VALUE(a_iRet, a_pstEntry, a_pszSql, a_iSize, a_iWriteLen, a_pszHostStart) \
{																											\
	a_iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%lld", (tdr_longlong)*(tdr_longlong *)a_pszHostStart);	\
	TDR_CHECK_BUFF(a_pszSql, a_iSize, a_iWriteLen, a_iRet);												\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
}																										
#endif

#if defined(WIN32) &&  _MSC_VER < 1400
#define TDR_ULONGLONG_ENTRY_VALUE2SQL_VALUE(a_iRet, a_pstEntry, a_pszSql, a_iSize, a_iWriteLen, a_pszHostStart) \
{																											\
	a_iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%I64u", (tdr_ulonglong)*(tdr_ulonglong *)a_pszHostStart);	\
	TDR_CHECK_BUFF(a_pszSql, a_iSize, a_iWriteLen, a_iRet);												\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
}
#else
#define TDR_ULONGLONG_ENTRY_VALUE2SQL_VALUE(a_iRet, a_pstEntry, a_pszSql, a_iSize, a_iWriteLen, a_pszHostStart) \
{																											\
	a_iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%llu", (tdr_ulonglong)*(tdr_ulonglong *)a_pszHostStart);	\
	TDR_CHECK_BUFF(a_pszSql, a_iSize, a_iWriteLen, a_iRet);												\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
}
#endif


/**将entry的值转换成SQL语句所需的值 
*/
#define TDR_ENTRY_VALUE2SQL_VALUE(a_iRet, a_hDBHandle, a_pstLib, a_pstEntry, a_iCount, a_idxEntry, a_iVersion, a_pszSql, a_iSize, a_pszHostStart, a_pszHostEnd) \
{																													\
	int iWriteLen;																									\
	switch(a_pstEntry->iType)																						\
	{																												\
	case TDR_TYPE_UNION:																							\
		{																											\
			TDRDATA stSql,stData;																					\
			LPTDRMETA pstType = TDR_PTR_TO_META(a_pstLib, a_pstEntry->ptrMeta);										\
			stSql.iBuff = a_iSize;																					\
			stSql.pszBuff = a_pszSql;																				\
			stData.iBuff = a_pszHostEnd - a_pszHostStart;																	\
			stData.pszBuff = a_pszHostStart;																		\
			a_iRet = tdr_dbms_union2sql(a_hDBHandle, &stSql, pstType, a_idxEntry, &stData, a_iVersion);										\
			a_pszSql += stSql.iBuff;																				\
			a_iSize -= stSql.iBuff;																					\
			if (0 < a_pstEntry->iCustomHUnitSize)												\
			{																					\
				a_pszHostStart += a_pstEntry->iCustomHUnitSize;											\
			}else																				\
			{																					\
				a_pszHostStart += a_pstEntry->iHUnitSize;											\
			}																					\
		}																											\
		break;																										\
	case TDR_TYPE_STRUCT:																						\
		{																											\
			TDRDATA stSql,stData;																					\
			LPTDRMETA pstMeta;																						\
			assert(a_pstEntry->ptrMeta);																			\
			stSql.iBuff = a_iSize;																					\
			stSql.pszBuff = a_pszSql;																				\
			stData.iBuff = a_pszHostEnd - a_pszHostStart;																	\
			stData.pszBuff = a_pszHostStart;																		\
			pstMeta = TDR_PTR_TO_META(a_pstLib, a_pstEntry->ptrMeta);												\
			a_iRet = tdr_dbms_meta2sql(&stSql, a_hDBHandle, pstMeta, &stData, a_iVersion);										\
			a_pszSql += stSql.iBuff;																				\
			a_iSize -= stSql.iBuff;																					\
			if (0 < a_pstEntry->iCustomHUnitSize)												\
			{																					\
				a_pszHostStart += a_pstEntry->iCustomHUnitSize;											\
			}else																				\
			{																					\
				a_pszHostStart += a_pstEntry->iHUnitSize;											\
			}																					\
		}																											\
		break;																										\
	case TDR_TYPE_CHAR:																								\
		{																											\
		if (1 == a_iCount)																						\
			{																										\
			iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%d", (int)(char)a_pszHostStart[0]);						\
			TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
			}else																									\
			{																										\
			if (a_iSize < 2*a_iCount + 2)																\
				{																										\
				a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);										\
				break;																								\
				}																										\
				*a_pszSql++ = '\'';																						\
				a_iSize--;																								\
				iWriteLen = tdr_dbms_escape_string(a_hDBHandle, a_pszSql, a_pszHostStart, a_iCount);		\
				a_pszSql += iWriteLen;																					\
				a_iSize -= (iWriteLen + 1);																					\
				*a_pszSql++ = '\'';																						\
			}																										\
		}																											\
		break;																										\
	case TDR_TYPE_UCHAR:																							\
	case TDR_TYPE_BYTE:																								\
		{																											\
		if (1 == a_iCount)																						\
			{																										\
			iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%d", (int)(unsigned char)a_pszHostStart[0]);						\
			TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
			}else																									\
			{																										\
			if (a_iSize < 2*a_iCount + 2)																\
				{																										\
				a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);										\
				break;																								\
				}																										\
				*a_pszSql++ = '\'';																						\
				a_iSize--;																								\
				iWriteLen = tdr_dbms_escape_string(a_hDBHandle, a_pszSql, a_pszHostStart, a_iCount);		\
				a_pszSql += iWriteLen;																					\
				a_iSize -= (iWriteLen + 1);																					\
				*a_pszSql++ = '\'';																						\
			}																										\
		}																											\
		break;																										\
	case TDR_TYPE_SMALLINT:																							\
		{																											\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%d", (short)*(short *)a_pszHostStart);					\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_SMALLUINT:																							\
	case TDR_TYPE_WCHAR:																							\
		{																										\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%d", (unsigned short)*(unsigned short *)a_pszHostStart);	\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);													\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_INT:																								\
	case TDR_TYPE_LONG:																								\
		{																											\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%d", (int)*(int *)a_pszHostStart);						\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_UINT:																								\
	case TDR_TYPE_ULONG:																								\
	case TDR_TYPE_IP:																								\
		{																											\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%u", (unsigned int)*(unsigned int *)a_pszHostStart);						\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_LONGLONG:																								\
		{																											\
		TDR_LONGLONG_ENTRY_VALUE2SQL_VALUE(a_iRet, a_pstEntry, a_pszSql, a_iSize, iWriteLen, a_pszHostStart);	\
		}																											\
		break;																										\
	case TDR_TYPE_ULONGLONG:																								\
		{																											\
		TDR_ULONGLONG_ENTRY_VALUE2SQL_VALUE(a_iRet, a_pstEntry, a_pszSql, a_iSize, iWriteLen, a_pszHostStart);	\
		}																											\
		break;																										\
	case TDR_TYPE_FLOAT:																								\
		{																											\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%f", (float)*(float *)a_pszHostStart);		\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_DOUBLE:																								\
		{																											\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "%f", (double)*(double *)a_pszHostStart);		\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_DATETIME:																								\
		{																											\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "'%s'", tdr_tdrdatetime_to_str((tdr_datetime_t *)a_pszHostStart));		\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_DATE:																								\
		{																											\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "'%s'", tdr_tdrdate_to_str((tdr_date_t *)a_pszHostStart));		\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_TIME:																								\
		{																											\
		iWriteLen = tdr_snprintf(a_pszSql, a_iSize, "'%s'", tdr_tdrtime_to_str((tdr_time_t *)a_pszHostStart));		\
		TDR_CHECK_BUFF(a_pszSql, a_iSize, iWriteLen, a_iRet);												\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_STRING:																								\
		{																											\
		int iSize;																			\
		int iLen;																			\
		if (0 < a_pstEntry->iCustomHUnitSize)												\
				{																					\
				iSize = a_pstEntry->iCustomHUnitSize;											\
				}else																				\
				{																					\
				iSize = a_pszHostStart - a_pszHostEnd;											\
				}																					\
				iLen = tdr_strnlen(a_pszHostStart, iSize);												\
				if (a_iSize < 2*iLen + 2)																\
				{																										\
				a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);										\
				break;																								\
				}																										\
				*a_pszSql++ = '\'';																						\
				a_iSize--;																								\
				iWriteLen = tdr_dbms_escape_string(a_hDBHandle, a_pszSql, a_pszHostStart, iLen);		\
				a_pszSql += iWriteLen;																					\
				a_iSize -= (iWriteLen + 1);																					\
				*a_pszSql++ = '\'';																						\
				a_pszHostStart += iSize;															\
		}																											\
		break;																										\
	case TDR_TYPE_WSTRING:																								\
			{																											\
			int iSize;																			\
			int iLen;																			\
			if (0 < a_pstEntry->iCustomHUnitSize)												\
					{																					\
					iSize = a_pstEntry->iCustomHUnitSize;											\
					}else																				\
					{																					\
					iSize = a_pszHostStart - a_pszHostEnd;											\
					}																					\
					iLen = tdr_wcsnlen((wchar_t*)pszHostStart, iSize);										\
					iLen *= sizeof(tdr_wchar_t);														\
					if (a_iSize < iLen + 2)																\
					{																										\
					a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);										\
					break;																								\
					}																										\
					*a_pszSql++ = '\'';																						\
					a_iSize--;																								\
					iWriteLen = tdr_dbms_escape_string(a_hDBHandle, a_pszSql, a_pszHostStart, iLen);		\
					a_pszSql += iWriteLen;																					\
					a_iSize -= (iWriteLen + 1);																					\
					*a_pszSql++ = '\'';																						\
					a_pszHostStart += iSize;															\
			}																											\
			break;																										\
	default:																											\
	break;																											\
	}																													\
}

#define TDR_SQL_VALUE2_ENTRY_VALUE(a_iRet, a_pstLib, a_pstEntry, a_iCount, a_idxEntry, a_iVersion, a_pszHostStart, a_pszHostEnd, a_pszFieldVal, a_dwFieldLen)\
{																													\
	int iLen;																										\
	long lVal;																										\
	tdr_longlong llVal;																								\
	switch(a_pstEntry->iType)																						\
	{																												\
	case TDR_TYPE_UNION:																							\
		{																											\
			TDRDATA stSql,stData;																					\
			LPTDRMETA pstType = TDR_PTR_TO_META(a_pstLib, a_pstEntry->ptrMeta);										\
			stSql.iBuff = a_dwFieldLen;																					\
			stSql.pszBuff = a_pszFieldVal;																				\
			stData.iBuff = a_pszHostEnd - a_pszHostStart;																	\
			stData.pszBuff = a_pszHostStart;																		\
			a_iRet = tdr_dbms_sql2union(pstType, a_idxEntry, &stData, &stSql, a_iVersion);										\
			if (0 < a_pstEntry->iCustomHUnitSize)												\
			{																					\
				a_pszHostStart += a_pstEntry->iCustomHUnitSize;											\
			}else																				\
			{																					\
				a_pszHostStart += a_pstEntry->iHUnitSize;											\
			}																					\
		}																											\
		break;																										\
	case TDR_TYPE_STRUCT:																						\
	{																											\
		TDRDATA stSql,stData;																					\
		LPTDRMETA pstMeta;																						\
		assert(a_pstEntry->ptrMeta);																			\
		stSql.iBuff = a_dwFieldLen;																					\
		stSql.pszBuff = a_pszFieldVal;																				\
		stData.iBuff = a_pszHostEnd - a_pszHostStart;																	\
		stData.pszBuff = a_pszHostStart;																		\
		pstMeta = TDR_PTR_TO_META(a_pstLib, a_pstEntry->ptrMeta);												\
		a_iRet = tdr_dbms_sql2meta(pstMeta, &stData, &stSql, a_iVersion);										\
		if (0 < a_pstEntry->iCustomHUnitSize)												\
		{																					\
			a_pszHostStart += a_pstEntry->iCustomHUnitSize;											\
		}else																				\
		{																					\
			a_pszHostStart += a_pstEntry->iHUnitSize;											\
		}																					\
	}																											\
	break;																										\
	case TDR_TYPE_CHAR:																								\
	{																											\
	if (1 == a_iCount)																						\
		{																										\
		lVal = strtol(a_pszFieldVal, NULL, 0);																	\
		*(char *)a_pszHostStart = (char)lVal;																	\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}else																									\
		{																										\
		iLen = (int)a_dwFieldLen;																				\
		if (iLen > a_iCount)																		\
			{																										\
			iLen = a_iCount;																		\
			a_iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_TRUNCATE_DATE);											\
			}																										\
			if (a_pszHostStart + iLen > a_pszHostEnd )																\
			{																										\
			a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);										\
			break;																								\
			}																										\
			TDR_MEMCPY(a_pszHostStart, a_pszFieldVal, iLen, TDR_MIN_COPY);											\
			a_pszHostStart += (a_pstEntry->iHRealSize - iLen);														\
		}																										\
	}																											\
	break;																										\
	case TDR_TYPE_UCHAR:																							\
	case TDR_TYPE_BYTE:																								\
	{																											\
	if (1 == a_iCount)																						\
		{																										\
		lVal = strtol(a_pszFieldVal, NULL, 0);																	\
		*(unsigned char *)a_pszHostStart = (unsigned char)lVal;																	\
		a_pszHostStart += a_pstEntry->iHUnitSize;															\
		}else																									\
		{																										\
		iLen = (int)a_dwFieldLen;																				\
		if (iLen > a_iCount)																		\
			{																										\
			iLen = a_iCount;																		\
			a_iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_TRUNCATE_DATE);											\
			}																										\
			if (a_pszHostStart + iLen > a_pszHostEnd )																\
			{																										\
			a_iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);										\
			break;																								\
			}																										\
			TDR_MEMCPY(a_pszHostStart, a_pszFieldVal, iLen, TDR_MIN_COPY);											\
			a_pszHostStart += (a_pstEntry->iHRealSize - iLen);														\
		}																										\
	}																											\
	break;																										\
	case TDR_TYPE_SMALLINT:																							\
	{																											\
	lVal = strtol(a_pszFieldVal, NULL, 0);																	\
	*(short *)a_pszHostStart = (short)lVal;																	\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_SMALLUINT:																							\
	case TDR_TYPE_WCHAR:																							\
	{																										\
	lVal = strtol(a_pszFieldVal, NULL, 0);																	\
	*(unsigned short *)a_pszHostStart = (unsigned short)lVal;																	\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_INT:																								\
	case TDR_TYPE_LONG:																								\
	{																											\
	lVal = strtol(a_pszFieldVal, NULL, 0);																	\
	*(int *)a_pszHostStart = (int)lVal;																	\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_UINT:																								\
	case TDR_TYPE_ULONG:																								\
	case TDR_TYPE_IP:																								\
	{																											\
	llVal = TDR_ATOLL(a_pszFieldVal);																	\
	*(unsigned int *)a_pszHostStart = (unsigned int)llVal;																	\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_LONGLONG:																								\
	{																											\
	llVal = TDR_ATOLL(a_pszFieldVal);																	\
	*(tdr_longlong *)a_pszHostStart = llVal;																	\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_ULONGLONG:																								\
	{																											\
	llVal = TDR_ATOLL(a_pszFieldVal);																	\
	*(tdr_ulonglong *)a_pszHostStart = (tdr_ulonglong)llVal;																	\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_FLOAT:																								\
	{																											\
	float  fVal;																							\
	fVal = (float)atof(a_pszFieldVal);																			\
	*(float *)a_pszHostStart = fVal;																			\
	a_pszHostStart += a_pstEntry->iHUnitSize;																\
	}																											\
	break;																										\
	case TDR_TYPE_DOUBLE:																								\
	{																											\
	double  dVal;																							\
	dVal = strtod(a_pszFieldVal, NULL);																			\
	*(double *)a_pszHostStart = dVal;																			\
	a_pszHostStart += a_pstEntry->iHUnitSize;																\
	}																											\
	break;																										\
	case TDR_TYPE_DATETIME:																								\
	{																											\
	a_iRet = tdr_str_to_tdrdatetime((tdr_datetime_t *)a_pszHostStart, &a_pszFieldVal[0]);						\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_DATE:																								\
	{																											\
	a_iRet = tdr_str_to_tdrdate((tdr_date_t *)a_pszHostStart, &a_pszFieldVal[0]);						\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_TIME:																								\
	{																											\
	a_iRet = tdr_str_to_tdrtime((tdr_time_t *)a_pszHostStart, &a_pszFieldVal[0]);						\
	a_pszHostStart += a_pstEntry->iHUnitSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_STRING:																								\
	{																											\
	int iSize;																			\
	if (0 < a_pstEntry->iCustomHUnitSize)												\
		{																					\
		iSize = a_pstEntry->iCustomHUnitSize;											\
		}else																				\
		{																					\
		iSize = a_pszHostStart - a_pszHostEnd;											\
		}																					\
		iLen = (int)a_dwFieldLen + 1;														\
		if (iLen > iSize) /* the string has no space for null. */						\
		{																				\
		a_iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_TRUNCATE_DATE);					\
		iLen = iSize;															\
		}																				\
		if( a_pszHostStart + iLen > a_pszHostEnd )											\
		{																				\
		a_iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);			\
		}																				\
		TDR_STRNCPY(a_pszHostStart, a_pszFieldVal, iLen);								\
		a_pszHostStart += iSize;															\
	}																											\
	break;																										\
	case TDR_TYPE_WSTRING:																								\
	{																											\
	int iSize;																			\
	if (0 < a_pstEntry->iCustomHUnitSize)												\
		{																					\
		iSize = a_pstEntry->iCustomHUnitSize;											\
		}else																				\
		{																					\
		iSize = a_pszHostStart - a_pszHostEnd;											\
		}																					\
		iLen = (int)a_dwFieldLen;														\
		if (iLen > iSize - 2) /* the string has no space for null. */							\
		{																					\
		a_iRet = TDR_SUCCEESS_WARNING(TDR_SUCWARN_TRUNCATE_DATE);						\
		iLen = iSize - 2;																	\
		}																					\
		if( a_pszHostStart + iLen > a_pszHostEnd )											\
		{																				\
		a_iRet =	TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_HOSTBUFF_SPACE);			\
		}																				\
		TDR_MEMCPY(a_pszHostStart, a_pszFieldVal, iLen, TDR_MIN_COPY);						\
		*a_pszHostStart++ = '\0';															\
		*a_pszHostStart++ = '\0';															\
		a_pszHostStart += (iSize - iLen -2);												\
	}																											\
	break;																										\
	default:																											\
	break;																											\
	}																													\
}



/**生成数据库表名
*@param[out] a_pszName 保存生成好的数据库表的名字的缓冲区
*@param[in] a_iNameSize 缓冲区的最大空间
*@param[in] a_pstMeta 相关数据的元数据描述句柄
*@param[in] a_pszMetaHost 保存数据的缓冲区首地址
*@return 成功返回0，否则返回负数
*/
int tdr_get_dbtable_name(OUT char *a_pszName, IN int a_iNameSize, IN LPTDRMETA a_pstMeta, IN char *a_pszMetaHost);

/**生成SQL语句的名字列表
*@param[in,out] a_pstSql 记录SQL信息
*@param[in] a_pstObj 数据对象信息
*@param[in] bCalcRefer 生成名字列表时是否使用refer属性,其值为true,则使用,否则不使用
*@return 处理成功返回0,否则返回负数
*/
int tdr_gen_columm_name_lists_i(INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj, IN TDRBOOLEAN bCalcRefer);

/**生成SQL语句的成员的值列表
*@param[in,out] a_pstSql 记录SQL信息
*@param[in] a_hDBHandle 数据库处理句柄
*@param[in] a_pstObj 数据对象信息
*@return 处理成功返回0,否则返回负数
*/
int tdr_gen_columm_value_lists_i(INOUT LPTDRDATA a_pstSql, IN TDRDBHANDLE a_hDBHandle, INOUT  LPTDRDBOBJECT a_pstObj);


/**生成insert语句 
*/
int tdr_obj2sql_insert_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj);

/**生成update语句 
*/
int tdr_obj2sql_update_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhereDefinition);

/** 
*/
int tdr_gen_update_columm_lists_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj);

/**生成select语句 
*/
int tdr_obj2sql_select_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhereDefinition);


/*生成WHERE条件*/
int tdr_gen_where_definition_i(INOUT LPTDRDATA a_pstSql, IN TDRDBHANDLE a_hDBHandle, INOUT  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhereDefinition);

/**生成delete语句 
*/
int tdr_obj2sql_delete_i(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, INOUT  LPTDRDBOBJECT a_pstObj, IN const char *a_pszWhereDefinition);

int tdr_dbms_fetch_row(IN TDRDBRESULTHANDLE a_hDBResult);

/** 从当前数据行中获取指定名字的数据域
*@param[out] a_ppszFieldVal 保存数据域地址指针的指针
*@param[out] a_pulLen 数据域长度的指针
*@param[in] a_hDBResult 保存DB操作结果集的句柄
*@param[in] a_szName 数据域的名字
*
* @pre \e a_ppszFieldVal 不能为NULL
* @pre \e a_pulLen 不能为NULL
* @pre \e	a_szName 不能为NULL
*
*@retval 0   处理成功
*@retval >0  处理成功，但发生某些特殊情况
*@retval <0  处理失败
*/
int tdr_dbms_fetch_field(OUT char **a_ppszFieldVal, OUT unsigned int *a_pulLen, IN TDRDBRESULTHANDLE a_hDBResult, IN const char *a_szName);

int tdr_fetch_all_fields(IN  TDRDBRESULTHANDLE a_hDBResult, INOUT  LPTDRDBOBJECT a_pstObj);

/** 根据结构体在堆栈中的信息，生成成员列表名字前缀 以支持结构体子成员数组展开存储
*/
char *tdr_gen_entry_columnname_prefix(TDRSTACKITEM	a_pstStack[], int a_iStackCount);

#endif

