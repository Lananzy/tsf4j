/**
*
* @file     tdr_dbdriver_mysql.c 
* @brief    TSF-G-DR mysql数据库管理系统驱动
* 
* @author jackyai  
* @version 1.0
* @date 2007-07-24 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <assert.h>
#include <string.h>
#include <errmsg.h>
#include <tdr/tdr_error.h>
#include "tdr_dbms_comm.h"
#include "tdr_dbdriver_mysql.h"
#include "tdr/tdr_os.h"
#include "tdr/tdr_sql.h"
#include "tdr/tdr_net.h"
#include "tdr/tdr_metalib_kernel_i.h"
#include "tdr/tdr_net_i.h"
#include "tdr/tdr_metalib_manage_i.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif


#if !defined(MYSQL_VERSION_ID) ||  MYSQL_VERSION_ID < 40000
#define mysql_real_connect(m,h,u,p,d,port,usocket,flag) mysql_connect((m),(h),(u),(p))
#endif

#if !defined(MYSQL_VERSION_ID) ||  MYSQL_VERSION_ID < 32314
#define mysql_real_escape_string(conn,to,from,len) mysql_escape_string(to, from, len)
#endif

#ifdef _TDR_SORT_FIELDS
static void tdr_sort_result_fields(LPTDRDBMYSQLRESULT a_pstRes);
static void tdr_quick_sort_result_fields(LPTDRDBMYSQLRESULT a_pstRes, IN int a_iBegin, IN int a_iEnd);
#endif


#define TDR_DBMS_CHECK_SWAP_BUFF(a_pstLink, a_pstMeta) \
{														\
	if ((a_pstLink)->iSwapSize < ((a_pstMeta)->iHUnitSize + TDR_DBMS_SWAP_BUFF_MIN_SIZE))\
	{													\
		if (NULL != (a_pstLink)->szSwapBuff)				\
		{													\
			free((a_pstLink)->szSwapBuff);					\
		}													\
		(a_pstLink)->szSwapBuff = (char *)malloc((a_pstMeta)->iHUnitSize + TDR_DBMS_SWAP_BUFF_MIN_SIZE);\
		if (NULL == (a_pstLink)->szSwapBuff)							\
		{																\
			(a_pstLink)->iSwapSize = 0;									\
		}else															\
		{																\
			(a_pstLink)->iSwapSize = (a_pstMeta)->iHUnitSize + TDR_DBMS_SWAP_BUFF_MIN_SIZE;	\
		}																\
	}/*if (pstLink->iSwapSize < (a_pstMeta)->iHUnitSize)*/				\
}

/*创建数据库表时设置autoincrement的初值*/
static int tdr_set_table_autoincrement(IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
int tdr_dbdriver_open_handle(OUT LPTDRDBHANDLE a_phDBHandle, IN LPTDRDBMS a_pstDBMS, IN char *a_pszErrMsg)
{
	int iRet = TDR_SUCCESS;
	LPTDRDBMYSQLLINK pstLink;
	LPTDRDBMYSQLCONN pstConn;
	MYSQL *pstMysql;
	char *pszSock = NULL;

	assert(NULL != a_phDBHandle);
	assert(NULL != a_pstDBMS);

	pstLink = (LPTDRDBMYSQLLINK)calloc(1, sizeof(TDRDBMYSQLLINK));
	if (NULL == pstLink)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
	}
	memset(pstLink, 0, sizeof(TDRDBMYSQLLINK));
	pstLink->szSwapBuff = (char *)malloc(TDR_DBMS_DEFAULT_SWAP_BUFF_SIZE);
	if (NULL == pstLink->szSwapBuff )
	{
		free(pstLink);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
	}
	pstLink->iSwapSize = TDR_DBMS_DEFAULT_SWAP_BUFF_SIZE;

	pstConn = &pstLink->stMysqlConn;
	memcpy(&pstConn->stDbmsInfo, a_pstDBMS, sizeof(TDRDBMS));
	

	//初始化mysql的连接
	mysql_init(&(pstConn->stMysql));

	// set reconnect flag
	mysql_options ( &(pstConn->stMysql), MYSQL_OPT_RECONNECT, "1" ) ;

	// 从选项文件中读取client端的选项以代替缺省值
	mysql_options ( &(pstConn->stMysql), MYSQL_READ_DEFAULT_GROUP, "client");

	/*connect*/
	if ('\0' != pstConn->stDbmsInfo.szDBMSSock[0])
	{
		pszSock = &pstConn->stDbmsInfo.szDBMSSock[0];
	}
	pstMysql = mysql_real_connect(&(pstConn->stMysql), pstConn->stDbmsInfo.szDBMSConnectionInfo,
					pstConn->stDbmsInfo.szDBMSUser, pstConn->stDbmsInfo.szDBMSPassword, pstConn->stDbmsInfo.szDBMSCurDatabaseName,
					0, pszSock, 0);
	if (NULL == pstMysql)
	{
		if (NULL != a_pszErrMsg)
		{
			sprintf(a_pszErrMsg, "failed to connect to mysql: %s\n", mysql_error(&pstConn->stMysql));
		}
		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_FAILD_TO_CONNECT_SERVER);
	}

	if (TDR_ERR_IS_ERROR(iRet))
	{
		free(pstLink);
	}else
	{
		pstConn->iIsDBConnected = 1;
		pstLink->stRes.enStat = TDR_DBRESULT_NONE;
		pstLink->stRes.pstConn = &pstConn->stMysql;
		*a_phDBHandle = (TDRDBHANDLE)pstLink;
	}
	
	return iRet;
}

int tdr_keep_dbmsconnection(IN TDRDBHANDLE a_hDBHandle)
{
	int iRet = TDR_SUCCESS;
	LPTDRDBMYSQLLINK pstLink;
	LPTDRDBMYSQLCONN pstConn;
	MYSQL *pstMysql;
	char *pszSock = NULL;

	//assert(NULL != a_hDBHandle);
	if (NULL == a_hDBHandle)
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	pstLink = (LPTDRDBMYSQLLINK)a_hDBHandle;
	pstConn = &pstLink->stMysqlConn;
	iRet = mysql_ping ( &(pstConn->stMysql)) ;
	if ( 0 == iRet )
	{
		return iRet;
	}

	/*连接已断，则重建连接*/
	if (0 != pstLink->stRes.iIsResNotNull)
	{
		tdr_free_dbresult((LPTDRDBRESULTHANDLE)&(pstLink->stRes));
	}
	mysql_close(&pstConn->stMysql);

	if ('\0' != pstConn->stDbmsInfo.szDBMSSock[0])
	{
		pszSock = &pstConn->stDbmsInfo.szDBMSSock[0];
	}
	pstMysql = mysql_real_connect(&(pstConn->stMysql), pstConn->stDbmsInfo.szDBMSConnectionInfo,
		pstConn->stDbmsInfo.szDBMSUser, pstConn->stDbmsInfo.szDBMSPassword, pstConn->stDbmsInfo.szDBMSCurDatabaseName,
		0, pszSock, 0);
	if (NULL == pstMysql)
	{	
		iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_FAILD_TO_CONNECT_SERVER);
		pstConn->iIsDBConnected = 1;
		pstLink->stRes.enStat = TDR_DBRESULT_NONE;
	}
	
	return iRet;
}

void tdr_close_dbhanlde(IN LPTDRDBHANDLE a_phDBHandle)
{
	LPTDRDBMYSQLLINK pstLink;

	if (NULL == a_phDBHandle)
	{
		return ;
	}

	pstLink = (LPTDRDBMYSQLLINK)*a_phDBHandle;
	if (NULL == pstLink)
	{
		return ;
	}
	if (0 != pstLink->stRes.iIsResNotNull)
	{
		mysql_free_result(pstLink->stRes.pstRes);
	}

	mysql_close(&pstLink->stMysqlConn.stMysql);
	if (NULL != pstLink->szSwapBuff)
	{
		free(pstLink->szSwapBuff);
		pstLink->szSwapBuff = NULL;
	}
	free(pstLink);

	*a_phDBHandle = NULL;
	
	
}

int tdr_dbms_escape_string(IN TDRDBHANDLE a_hDBHandle, OUT char *a_pszTo, IN char *a_pszFrom, int a_iLen)
{
	LPTDRDBMYSQLLINK pstLink;
	unsigned long iToLen;

	assert(NULL != (LPTDRDBMYSQLLINK *)a_hDBHandle);
	assert(NULL != a_pszTo);
	assert(NULL != a_pszFrom);

	pstLink = (LPTDRDBMYSQLLINK)a_hDBHandle;
	iToLen = mysql_real_escape_string(&(pstLink->stMysqlConn.stMysql), a_pszTo, a_pszFrom, (unsigned long)a_iLen);

	return (int)iToLen;
}

int tdr_query(INOUT LPTDRDBRESULTHANDLE a_phDBResult, IN TDRDBHANDLE a_hDBHandle, IN LPTDRDATA a_pstSql)
{
	int iRet = TDR_SUCCESS;
	LPTDRDBMYSQLLINK pstLink;
	LPTDRDBMYSQLCONN pstConn;
	LPTDRDBMYSQLRESULT pstResult;
	unsigned long ulSqlLen;

	/*assert(NULL != a_phDBResult);
	assert(NULL != (LPTDRDBMYSQLLINK *)a_hDBHandle);
	assert(NULL != a_pstSql);*/

	pstLink = (LPTDRDBMYSQLLINK)a_hDBHandle;
	if ((NULL == a_hDBHandle)||(NULL == pstLink)||(NULL == a_pstSql)||
		(NULL == a_pstSql->pszBuff)||(0 >= a_pstSql->iBuff))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}

	pstConn = &pstLink->stMysqlConn;	
	if (0 != pstLink->stRes.iIsResNotNull)
	{
		*a_phDBResult = &pstLink->stRes;
		tdr_free_dbresult(a_phDBResult);
	}

	if (1 != pstConn->iIsDBConnected)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_FAILD_TO_CONNECT_SERVER);
	}

	/*去掉最后'\0'字符*/
	for (ulSqlLen = (unsigned long)a_pstSql->iBuff; ulSqlLen > 0;)
	{
		if ('\0' != a_pstSql->pszBuff[ulSqlLen-1])
		{
			break;
		}
		ulSqlLen--;
	}
	/*指定查询语句*/
	iRet = mysql_real_query(&pstConn->stMysql, a_pstSql->pszBuff, ulSqlLen);
	if (0 != iRet)
	{
		if (((CR_SERVER_LOST  == iRet)||(CR_SERVER_GONE_ERROR  == iRet)) && (0 != pstConn->stDbmsInfo.iReconnectOpt))
		{
			iRet = tdr_keep_dbmsconnection(a_hDBHandle);
			if (0 == iRet)
			{
				iRet = mysql_real_query(&pstConn->stMysql, a_pstSql->pszBuff, ulSqlLen);
			}
			if (0 != iRet )
			{
				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_FAILED_TO_QUERY);
			}
		}else
		{
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_FAILED_TO_QUERY);
		}/*if (((CR_SERVER_LOST  == iRet)||(CR_SERVER_GONE_ERROR  == iRet)) && (0 != pstConn->stDbmsInfo.iReconnectOpt))*/		
	}/*if (0 != iRet)*/
	

	/*查询成功，通过mysql_store_result决定是否返回结果集*/
	pstResult = &pstLink->stRes;
	pstResult->pstRes = mysql_store_result(&pstConn->stMysql);
	if (NULL != pstResult->pstRes)
	{
		/*查询返回结果集*/
		pstResult->iIsResNotNull = 1;
		pstResult->dwFieldsNum = mysql_num_fields(pstResult->pstRes);
		pstResult->astFields = mysql_fetch_fields(pstResult->pstRes);
		pstResult->enStat = TDR_DBRESULT_STORE;
#ifdef _TDR_SORT_FIELDS
		tdr_sort_result_fields(pstResult);
#endif
	}else
	{
		/*没有取到结果集，则判断是查询语句不会返回结果集还是取结果集失败*/
		if (0 != mysql_field_count(&pstConn->stMysql))
		{
			/*查询会返回结果集,但没有取到*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_FAILED_TO_GET_QUERY_RESULT);
		}
	}/*if (NULL != pstResult->pstRes)*/
	
	*a_phDBResult = pstResult;

	return iRet;
}

void tdr_free_dbresult(IN LPTDRDBRESULTHANDLE a_phDBResult)
{
	LPTDRDBMYSQLRESULT pstResult;
	
	if (NULL == a_phDBResult)
	{
		return ;
	}

	pstResult = (LPTDRDBMYSQLRESULT)*a_phDBResult;
	if (NULL == pstResult)
	{
		return;
	}

	if (0 != pstResult->iIsResNotNull)
	{
		mysql_free_result(pstResult->pstRes);
		pstResult->iIsResNotNull = 0;
		pstResult->enStat = TDR_DBRESULT_NONE;
		pstResult->dwFieldsNum = 0;
#ifdef _TDR_SORT_FIELDS
		if (NULL != pstResult->aSortedFieldIdx)
		{
			free(pstResult->aSortedFieldIdx);
			pstResult->aSortedFieldIdx = NULL;
		}
#endif
	}

	*a_phDBResult = NULL;

}

int tdr_dbms_errno(IN TDRDBHANDLE a_hDBHandle)
{
	//assert(NULL != (LPTDRDBMYSQLLINK)a_hDBHandle);
	if (NULL == (LPTDRDBMYSQLLINK)a_hDBHandle)
	{
		return 0;
	}

	return mysql_errno(&((LPTDRDBMYSQLLINK)a_hDBHandle)->stMysqlConn.stMysql);
}


const char *tdr_dbms_error(IN TDRDBHANDLE a_hDBHandle)
{
	//assert(NULL != (LPTDRDBMYSQLLINK)a_hDBHandle);
	if (NULL == (LPTDRDBMYSQLLINK)a_hDBHandle)
	{
		return "";
	}

	return mysql_error(&((LPTDRDBMYSQLLINK)a_hDBHandle)->stMysqlConn.stMysql);
}


int tdr_query_quick(INOUT TDRDBRESULTHANDLE *a_phDBResult, IN TDRDBHANDLE a_hDBHandle, IN LPTDRDATA a_pstSql)
{
	int iRet = TDR_SUCCESS;
	LPTDRDBMYSQLLINK pstLink;
	LPTDRDBMYSQLCONN pstConn;
	LPTDRDBMYSQLRESULT pstResult;
	unsigned long ulSqlLen;

	/*assert(NULL != a_phDBResult);
	assert(NULL != (LPTDRDBMYSQLLINK *)a_hDBHandle);
	assert(NULL != a_pstSql);*/

	pstLink = (LPTDRDBMYSQLLINK)a_hDBHandle;
	if ((NULL == a_hDBHandle)||(NULL == pstLink)||(NULL == a_pstSql)||
		(NULL == a_pstSql->pszBuff)||(0 >= a_pstSql->iBuff))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}


	pstConn = &pstLink->stMysqlConn;
	if (0 != pstLink->stRes.iIsResNotNull)
	{
		tdr_free_dbresult(a_phDBResult);
	}

	if (1 != pstConn->iIsDBConnected)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_FAILD_TO_CONNECT_SERVER);
	}

	/*去掉最后'\0'字符*/
	for (ulSqlLen = (unsigned long)a_pstSql->iBuff; ulSqlLen > 0;)
	{
		if ('\0' != a_pstSql->pszBuff[ulSqlLen-1])
		{
			break;
		}
		ulSqlLen--;
	}
	

	/*指定查询语句*/
	iRet = mysql_real_query(&pstConn->stMysql, a_pstSql->pszBuff, ulSqlLen);
	if (0 != iRet)
	{
		if (((CR_SERVER_LOST  == iRet)||(CR_SERVER_GONE_ERROR  == iRet)) && (0 != pstConn->stDbmsInfo.iReconnectOpt))
		{
			iRet = tdr_keep_dbmsconnection(a_hDBHandle);
			if (0 == iRet)
			{
				iRet = mysql_real_query(&pstConn->stMysql, a_pstSql->pszBuff, ulSqlLen);
			}
			if (0 != iRet )
			{
				return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_FAILED_TO_QUERY);
			}
		}else
		{
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_FAILED_TO_QUERY);
		}/*if (((CR_SERVER_LOST  == iRet)||(CR_SERVER_GONE_ERROR  == iRet)) && (0 != pstConn->stDbmsInfo.iReconnectOpt))*/		
	}/*if (0 != iRet)*/

	/*查询成功，通过mysql_store_result决定是否返回结果集*/
	pstResult = &pstLink->stRes;
	pstResult->pstRes = mysql_use_result(&pstConn->stMysql);
	if (NULL != pstResult->pstRes)
	{
		/*查询返回结果集*/
		pstResult->iIsResNotNull = 1;
		pstResult->dwFieldsNum = mysql_num_fields(pstResult->pstRes);
		pstResult->astFields = mysql_fetch_fields(pstResult->pstRes);
		pstResult->enStat = TDR_DBRESULT_USE;
#ifdef _TDR_SORT_FIELDS
		tdr_sort_result_fields(pstResult);
#endif
		
	}else
	{
		/*没有取到结果集，则判断是查询语句不会返回结果集还是取结果集失败*/
		if (0 != mysql_errno(&pstConn->stMysql))
		{
			/*查询会返回结果集,但没有取到*/
			iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_FAILED_TO_GET_QUERY_RESULT);
		}
	}/*if (NULL != pstResult->pstRes)*/

	/*返回结果句柄*/
	*a_phDBResult = pstResult;

	return iRet;
}


unsigned long tdr_num_rows(IN TDRDBRESULTHANDLE a_hDBResult)
{
	LPTDRDBMYSQLRESULT pstResult;

	//assert(NULL != (LPTDRDBMYSQLRESULT )a_hDBResult);

	pstResult = (LPTDRDBMYSQLRESULT )a_hDBResult;
	if (NULL == pstResult)
	{
		return 0;
	}

	return (unsigned long)mysql_num_rows(pstResult->pstRes);
}

long tdr_affected_rows(IN TDRDBRESULTHANDLE a_hDBResult)
{
	LPTDRDBMYSQLRESULT pstResult;
	MYSQL  *pstConn;

	//assert(NULL != (LPTDRDBMYSQLRESULT )a_hDBResult);

	pstResult = (LPTDRDBMYSQLRESULT )a_hDBResult;
	if (NULL == pstResult)
	{
		return 0;
	}

	pstConn = pstResult->pstConn;

	return (long)mysql_affected_rows(pstConn);
}

int tdr_dbms_fetch_row(IN TDRDBRESULTHANDLE a_hDBResult)
{
	LPTDRDBMYSQLRESULT pstResult;

	assert(NULL != (LPTDRDBMYSQLRESULT )a_hDBResult);

	pstResult = (LPTDRDBMYSQLRESULT )a_hDBResult;
	if (0 == pstResult->iIsResNotNull)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_NO_RESULT_SET);
	}

	pstResult->stRow = mysql_fetch_row(pstResult->pstRes);
	if (NULL == pstResult->stRow)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_NO_RECORD_IN_RESULTSET);
	}

	/*取出该数据行各列的信息*/
	pstResult->aulFieldsLength = mysql_fetch_lengths(pstResult->pstRes);

	return TDR_SUCCESS;
}


int tdr_dbms_fetch_field(OUT char **a_ppszFieldVal, OUT unsigned int *a_pulLen, IN TDRDBRESULTHANDLE a_hDBResult, IN const char *a_szName)
{
	LPTDRDBMYSQLRESULT pstResult;
	unsigned int i;
	unsigned int iFieldNum;
	MYSQL_FIELD *pstFields;

	assert(NULL != a_ppszFieldVal);
	assert(NULL != a_pulLen);
	assert(NULL != a_szName);
	assert(NULL != (LPTDRDBRESULTHANDLE)a_hDBResult);	

	pstResult = (LPTDRDBMYSQLRESULT )a_hDBResult;
	assert(NULL != pstResult->stRow);

	pstFields = pstResult->astFields;
	iFieldNum = pstResult->dwFieldsNum;

#ifdef _TDR_SORT_FIELDS
	{
		unsigned int iMin = 0;
		unsigned int iMax = iFieldNum -1;
		int iRet;
		unsigned int *aFieldIdx = pstResult->aSortedFieldIdx;

		while(iMin <= iMax)
		{
			i = (iMin+iMax)>>1;

			iRet = strcmp(a_szName, pstFields[aFieldIdx[i]].name);
			if( iRet>0 )
			{
				iMin    =       i + 1;
			}
			else if( iRet<0 )
			{
				iMax    =       i - 1;
			}
			else
			{
				break;
			}
		}/*while(iMin <= iMax)*/
		
		if (iMin > iMax)
		{
			return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_NO_EXPECTED_FIELD);
		}
		*a_ppszFieldVal = (char *)pstResult->stRow[aFieldIdx[i]];
		*a_pulLen = (unsigned int)pstResult->aulFieldsLength[aFieldIdx[i]];
	}	
#else
	for (i = 0; i < iFieldNum; i++)
	{
		if (0 == strcmp(a_szName, pstFields->name))
		{
			break;
		}
		pstFields++;
	}
	if (i >= pstResult->dwFieldsNum)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_NO_EXPECTED_FIELD);
	}
	*a_ppszFieldVal = (char *)pstResult->stRow[i];
	*a_pulLen = (unsigned int)pstResult->aulFieldsLength[i];
#endif	

	
	
	return TDR_SUCCESS;
}

#ifdef _TDR_SORT_FIELDS
void tdr_sort_result_fields(LPTDRDBMYSQLRESULT a_pstRes)
{
	unsigned int iFieldNum,i;
	unsigned int *aFieldInx;

	assert(NULL != a_pstRes);

	iFieldNum = (int)a_pstRes->dwFieldsNum;
	if (0 >= iFieldNum)
	{
		return ;
	}

	a_pstRes->aSortedFieldIdx = (unsigned int *)malloc(sizeof(unsigned int)* iFieldNum);
	if (NULL == a_pstRes->aSortedFieldIdx)
	{
		return ;
	}

	aFieldInx = a_pstRes->aSortedFieldIdx;
	for (i = 0; i < iFieldNum; i++)
	{
		*aFieldInx++ = i; 
	}

	tdr_quick_sort_result_fields(a_pstRes, 0, iFieldNum-1);

}

void tdr_quick_sort_result_fields(LPTDRDBMYSQLRESULT a_pstRes, IN int a_iBegin, IN int a_iEnd)
{
	unsigned int i;
	unsigned int j;
	unsigned int iTemp;
	unsigned int *aFieldInx;
	char *pszVotName;
	MYSQL_FIELD *astFields; 

	assert(NULL != a_pstRes);

	if (a_iBegin >= a_iEnd)
	{
		return ;
	}

	aFieldInx = a_pstRes->aSortedFieldIdx;
	astFields = a_pstRes->astFields;
	iTemp = aFieldInx[a_iBegin]; /*pvot*/
	pszVotName = astFields[aFieldInx[a_iBegin]].name;
	i = a_iBegin ;
	j = a_iEnd;

	while (i < j)
	{
		while (i < j)
		{
			if (0 < strcmp(pszVotName, astFields[aFieldInx[j]].name))
			{
				aFieldInx[i] = aFieldInx[j];
				break;
			}
			j--;
		}

		while (i < j)
		{
			if (0 > strcmp(pszVotName, astFields[aFieldInx[i]].name))
			{
				aFieldInx[j] = aFieldInx[i];
				break;
			}
			i++;
		}			

		if (i >= j)
		{
			break;
		}		
	}/*while (i < j)*/

	aFieldInx[i] = iTemp;	

	tdr_quick_sort_result_fields(a_pstRes, a_iBegin, i - 1);
	tdr_quick_sort_result_fields(a_pstRes, i + 1, a_iEnd);
}
#endif

int tdr_dbms_meta2sql(INOUT LPTDRDATA a_pstSql, IN TDRDBHANDLE a_hDBHandle, IN LPTDRMETA a_pstMeta, IN LPTDRDATA a_pstData, IN int a_iVersion)
{
	LPTDRDBMYSQLLINK pstLink;
	char *pch;
	TDRDATA stNetInfo;
	int iRet = TDR_SUCCESS;
	char *a_pszSql;
	int iWriteLen;

	assert(NULL != a_hDBHandle);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pstSql);
	assert(NULL != a_pstData);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);
	assert(NULL != a_pstData->pszBuff);
	assert(0 < a_pstData->iBuff);	
	assert(a_pstMeta->iBaseVersion <= a_iVersion);

	pstLink = (LPTDRDBMYSQLLINK)a_hDBHandle;
	

	if (a_iVersion > a_pstMeta->iCurVersion)
	{
		a_iVersion = a_pstMeta->iCurVersion;
	}

	/*检查swap空间大小是否足够,如果不够则重新分配*/
	TDR_DBMS_CHECK_SWAP_BUFF(pstLink, a_pstMeta);
	if (NULL == pstLink->szSwapBuff)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
	}
	pch = pstLink->szSwapBuff;

	/*记录版本信息*/
	*(unsigned long*)(pch)	= htonl((unsigned long)(a_iVersion));
	pch += sizeof(long);

	/*将信息打包*/
	stNetInfo.iBuff = (int)(pstLink->iSwapSize - TDR_DBMS_SWAP_BUFF_MIN_SIZE);
	stNetInfo.pszBuff = pstLink->szSwapBuff + TDR_DBMS_SWAP_BUFF_MIN_SIZE;
	iRet = tdr_hton(a_pstMeta, &stNetInfo, a_pstData, a_iVersion);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		a_pstSql->iBuff = 0;
		return iRet;
	}

	/*记录长度信息*/
	*(unsigned long*)(pch)	=	htonl((unsigned long)(stNetInfo.iBuff));

	/*将所有信息转换成串*/
	a_pszSql = a_pstSql->pszBuff;
	*a_pszSql++ = '\'';																						
	a_pstSql->iBuff--;																								
	iWriteLen = mysql_real_escape_string(&(pstLink->stMysqlConn.stMysql), a_pszSql, 
		pstLink->szSwapBuff, (unsigned long)(stNetInfo.iBuff + TDR_DBMS_SWAP_BUFF_MIN_SIZE));
	a_pszSql += iWriteLen;																				
	*a_pszSql++ = '\'';	

	a_pstSql->iBuff = a_pszSql - a_pstSql->pszBuff;


	return iRet;
}

int tdr_dbms_sql2meta( IN LPTDRMETA a_pstMeta, INOUT LPTDRDATA a_pstData,IN LPTDRDATA a_pstSql, IN int a_iVersion)
{
	char *pch;
	TDRDATA stNetInfo;
	int iRet = TDR_SUCCESS;
	int iVersion;

	assert(NULL != a_pstMeta);
	assert(NULL != a_pstSql);
	assert(NULL != a_pstData);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);
	assert(NULL != a_pstData->pszBuff);
	assert(0 < a_pstData->iBuff);	

	if (a_pstSql->iBuff <= (int)(sizeof(long)*2))
	{
		a_pstData->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);
	}


	/*解包版本信息*/
	if (a_iVersion < a_pstMeta->iBaseVersion)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}
	pch = a_pstSql->pszBuff;	
	iVersion = (int)ntohl(*(unsigned long*)(pch));
	pch += sizeof(long);
	if (a_iVersion > iVersion)
	{
		a_iVersion = iVersion;
	}

	/*解包长度信息*/
	stNetInfo.iBuff	= (int)ntohl(*(unsigned long*)(pch));
	pch += sizeof(long);
	if (stNetInfo.iBuff > (int)(a_pstSql->iBuff - sizeof(long)*2))
	{
		a_pstData->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);
	}


	/*将信息打包*/
	stNetInfo.pszBuff = pch;
	iRet = tdr_ntoh(a_pstMeta, a_pstData, &stNetInfo, a_iVersion);
	
	return iRet;
}

int tdr_dbms_union2sql(IN TDRDBHANDLE a_hDBHandle, INOUT LPTDRDATA a_pstSql, IN LPTDRMETA a_pstMeta, IN int  a_idxEntry,  IN LPTDRDATA a_pstData, IN int a_iVersion)
{
	LPTDRDBMYSQLLINK pstLink;
	char *pch;
	TDRDATA stNetInfo;
	int iRet = TDR_SUCCESS;
	char *a_pszSql;
	int iWriteLen;

	assert(NULL != a_hDBHandle);
	assert(NULL != a_pstMeta);
	assert(NULL != a_pstSql);
	assert(NULL != a_pstData);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);
	assert(NULL != a_pstData->pszBuff);
	assert(0 < a_pstData->iBuff);	
	assert(TDR_TYPE_UNION == a_pstMeta->iType);
	assert((0 <= a_idxEntry) && (a_idxEntry < a_pstMeta->iEntriesNum));

	

	/*检查版本*/
	if (a_iVersion < a_pstMeta->iBaseVersion)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}
	if (a_iVersion > a_pstMeta->iCurVersion)
	{
		a_iVersion = a_pstMeta->iCurVersion;
	}


	pstLink = (LPTDRDBMYSQLLINK)a_hDBHandle;
	
	/*检查swap空间大小是否足够,如果不够则重新分配*/
	TDR_DBMS_CHECK_SWAP_BUFF(pstLink, a_pstMeta);
	if (NULL == pstLink->szSwapBuff)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_MEMORY);
	}
	pch = pstLink->szSwapBuff;

	/*记录版本信息*/
	*(unsigned long*)(pch)	= htonl((unsigned long)(a_iVersion));
	pch += sizeof(long);

	/*将信息打包*/
	stNetInfo.iBuff = (int)(pstLink->iSwapSize - TDR_DBMS_SWAP_BUFF_MIN_SIZE);
	stNetInfo.pszBuff = &pstLink->szSwapBuff[0] + TDR_DBMS_SWAP_BUFF_MIN_SIZE;
	iRet = tdr_pack_union_entry_i(a_pstMeta, a_idxEntry, &stNetInfo, a_pstData, a_iVersion);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		a_pstSql->iBuff = 0;
		return iRet;
	}

	/*记录长度信息*/
	*(unsigned long*)(pch)	=	htonl((unsigned long)(stNetInfo.iBuff));

	/*将所有信息转换成串*/
	a_pszSql = a_pstSql->pszBuff;
	*a_pszSql++ = '\'';																						
	a_pstSql->iBuff--;																								
	iWriteLen = mysql_real_escape_string(&(pstLink->stMysqlConn.stMysql), a_pszSql, 
		pstLink->szSwapBuff, (unsigned long)(stNetInfo.iBuff + TDR_DBMS_SWAP_BUFF_MIN_SIZE));
	a_pszSql += iWriteLen;																				
	*a_pszSql++ = '\'';	

	a_pstSql->iBuff = a_pszSql - a_pstSql->pszBuff;


	return iRet;
}

int tdr_dbms_sql2union(IN LPTDRMETA a_pstMeta, IN int  a_idxEntry,  INOUT LPTDRDATA a_pstData, IN LPTDRDATA a_pstSql, IN int a_iVersion)
{
	char *pch;
	TDRDATA stNetInfo;
	int iRet = TDR_SUCCESS;
	int iVersion;


	assert(NULL != a_pstMeta);
	assert(NULL != a_pstSql);
	assert(NULL != a_pstData);
	assert(NULL != a_pstSql->pszBuff);
	assert(0 < a_pstSql->iBuff);
	assert(NULL != a_pstData->pszBuff);
	assert(0 < a_pstData->iBuff);
	assert(TDR_TYPE_UNION == a_pstMeta->iType);
	assert((0 <= a_idxEntry) && (a_idxEntry < a_pstMeta->iEntriesNum));


	/*检查版本*/
	if (a_iVersion < a_pstMeta->iBaseVersion)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_INVALID_CUTOFF_VERSION);
	}

	
	/*解包版本信息*/
	if (a_pstSql->iBuff <= (int)(sizeof(long)*2))
	{
		a_pstData->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);
	}
	pch = a_pstSql->pszBuff;	
	iVersion = (int)ntohl(*(unsigned long*)(pch));
	pch += sizeof(long);
	if (a_iVersion > iVersion)
	{
		a_iVersion = iVersion;
	}

	/*解包长度信息*/
	stNetInfo.iBuff	= (int)ntohl(*(unsigned long*)(pch));
	pch += sizeof(long);
	if (stNetInfo.iBuff > (int)(a_pstSql->iBuff - sizeof(long)*2))
	{
		a_pstData->iBuff = 0;
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERR_NET_NO_NETBUFF_SPACE);
	}


	/*将信息打包*/
	stNetInfo.pszBuff = pch;
	iRet = tdr_unpack_union_entry_i(a_pstMeta, a_idxEntry, a_pstData, &stNetInfo, iVersion);

	return iRet;
}

int tdr_set_table_options(IN LPTDRDBMS a_pstDBMS, IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream)
{
	int iRet = TDR_SUCCESS;

	assert(NULL != a_pstDBMS);
	assert(NULL != a_pstIOStream);
	assert(NULL != a_pstMeta);

	/*only support mysql*/
	if (0 != tdr_stricmp(a_pstDBMS->szDBMSName, TDR_DBMS_MYSQL))
	{
		return TDR_SUCCESS;
	}

	/*engine*/
	if ('\0' != a_pstDBMS->szDBMSEngine[0])
	{
		iRet = tdr_iostream_write(a_pstIOStream, " ENGINE=%s", a_pstDBMS->szDBMSEngine);
	}else
	{
		iRet = tdr_iostream_write(a_pstIOStream, " ENGINE=%s", TDR_MYSQL_DEFAULT_TABLE_ENGINE);
	}

	/*charset*/
	if ('\0' != a_pstDBMS->szDBMSCharset[0])
	{
		iRet = tdr_iostream_write(a_pstIOStream, " DEFAULT CHARSET=%s", a_pstDBMS->szDBMSCharset);
	}else
	{
		iRet = tdr_iostream_write(a_pstIOStream, " DEFAULT CHARSET=%s", TDR_MYSQL_DEFAULT_TABLE_CHARTSET);
	}

	/*set starting value of auto increment */
	iRet = tdr_set_table_autoincrement(a_pstMeta, a_pstIOStream);
	
	return iRet;
}

int tdr_set_table_autoincrement(IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream)
{
	int iRet = TDR_SUCCESS;
	TDRSTACK stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;
	LPTDRMETA pstCurMeta;
	LPTDRMETAENTRY pstEntry;
	LPTDRMETALIB pstLib;
	int iChange;

	assert(NULL != a_pstIOStream);
	assert(NULL != a_pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0; 
	pstStackTop->pstMeta = a_pstMeta;
	pstCurMeta = a_pstMeta;
	iStackItemCount = 1;
	pstLib = TDR_META_TO_LIB(a_pstMeta);


	while (0 < iStackItemCount)
	{
		if (0 >= pstStackTop->iCount)
		{
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstStackTop--;
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}/*if (0 >= pstStackTop->iCount)*/

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}
		

		if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))
		{
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}

			pstStackTop++;
			iStackItemCount++;
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = 1;  /*only one column can be auto increment */
			pstStackTop->idxEntry = 0;			
			continue;
		}/*if ((TDR_TYPE_STRUCT == pstEntry->iType) && TDR_ENTRY_DO_EXTENDABLE(pstEntry))*/

		/*simple entry*/
		if (TDR_ENTRY_IS_AUTOINCREMENT(pstEntry) && (TDR_INVALID_PTR != pstEntry->ptrDefaultVal))
		{
			char *pszDefault;
			char *pstHostEnd;

			pszDefault = TDR_GET_STRING_BY_PTR(pstLib, pstEntry->ptrDefaultVal);
			pstHostEnd = pszDefault + pstEntry->iDefaultValLen;
			tdr_iostream_write(a_pstIOStream, " AUTO_INCREMENT=");
			tdr_ioprintf_basedtype_i(a_pstIOStream, pstLib, pstEntry, &pszDefault, pstHostEnd);			
			break; /*only one column can be auto increment */
		}/*if (TDR_ENTRY_IS_UNIQUE(pstEntry))*/	
		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
	}/*while (0 < iStackItemCount)*/		

	return iRet;
}

tdr_ulonglong tdr_dbms_insert_id(IN TDRDBHANDLE a_hDBHandle)
{
	//assert(NULL != (LPTDRDBMYSQLLINK)a_hDBHandle);
	if (NULL == (LPTDRDBMYSQLLINK)a_hDBHandle)
	{
		return 0;
	}

	return (tdr_ulonglong)mysql_insert_id(&((LPTDRDBMYSQLLINK)a_hDBHandle)->stMysqlConn.stMysql);
}

int  tdr_get_records_count(IN TDRDBHANDLE a_hDBHandle, IN const char *a_pszTableName, 
						   IN const char *a_pszWhereDef, OUT unsigned int *a_pdwCount)
{
	int iRet = TDR_SUCCESS;
	LPTDRDBMYSQLLINK pstLink;
	char szSql[1024] = {0};
	int iWriteLen;
	TDRDATA stSQLInfo;
	TDRDBRESULTHANDLE hDBResult =0;
	LPTDRDBMYSQLRESULT pstResult = NULL;
	
	//assert(NULL != a_pszTableName);
	//assert(NULL!= a_pdwCount);
	pstLink = (LPTDRDBMYSQLLINK)a_hDBHandle;
	if ((NULL == a_pszTableName)||(NULL == a_pdwCount)||(NULL == pstLink))
	{
		return TDR_ERRIMPLE_INVALID_PARAM;
	}
	

	if (NULL == a_pszWhereDef)
	{
		a_pszWhereDef = "";
	}
	iWriteLen = tdr_snprintf(szSql, sizeof(szSql), "SELECT COUNT(*) FROM %s %s", 
		a_pszTableName, a_pszWhereDef);
	if ((0 > iWriteLen) || (iWriteLen >= (int)sizeof(szSql)))
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_NO_SPACE_TO_WRITE); 
	}

	stSQLInfo.iBuff = iWriteLen;
	stSQLInfo.pszBuff = szSql;
	iRet = tdr_query(&hDBResult, a_hDBHandle, &stSQLInfo);
	if (0 != iRet)
	{
		return iRet;
	}

	pstResult = (LPTDRDBMYSQLRESULT)hDBResult;
	assert(NULL != pstResult);
	pstResult->stRow = mysql_fetch_row(pstResult->pstRes);
	if (NULL == pstResult->stRow)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_DB_NO_RECORD_IN_RESULTSET);
	}

	*a_pdwCount = atoi(pstResult->stRow[0]);

	tdr_free_dbresult(&hDBResult);

	return iRet;	
}

