/**
*
* @file     tdr_dbdriver_mysql.h 
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

#ifndef TDR_DBDRIVER_MYSQL_H
#define TDR_DBDRIVER_MYSQL_H

#include "tdr/tdr_os.h"
#include <my_global.h>
#include <mysql.h>
#include "tdr/tdr_iostream_i.h"

#ifdef WIN32
#pragma comment(lib, "libmysql.lib")
#pragma comment(lib, "mysqlclient.lib")
#endif

#include "tdr/tdr_types.h"

#define TDR_DBMS_DEFAULT_SWAP_BUFF_SIZE		0x100000	/**<在做数据转换时的交换空间大小,缺省为1M */
#define TDR_DBMS_SWAP_BUFF_MIN_SIZE		(int)(sizeof(long) *2)    /*缓冲区最小大小,用于存储结构体的版本和长度*/


struct tagTDRDBMysqlConn
{
	TDRDBMS  stDbmsInfo;
	int			iIsDBConnected;			/**<是否已经连接上对应的Database, 0=断开，1=连接上*/
	MYSQL			stMysql;		/**<当前打开的Mysql连接*/
};
typedef struct tagTDRDBMysqlConn TDRDBMYSQLCONN;
typedef struct tagTDRDBMysqlConn *LPTDRDBMYSQLCONN;

enum tagTDRDBResultSetStat
{
	TDR_DBRESULT_NONE = 0,	/**<为知状态*/
	TDR_DBRESULT_STORE,		/**<通过mysql_strore_result获取结果集*/
	TDR_DBRESULT_USE,		/**<通过mysql_use_result获取结果集*/
};
typedef enum tagTDRDBResultSetStat TDRDBRESULTSETSTAT;

struct tagTDRDBMysqlResult
{
	int			iIsResNotNull;	/**<当前操作的RecordSet是否为空,0=空，1=非空*/	
	TDRDBRESULTSETSTAT enStat;	/**<结果集的状态*/

	unsigned int dwFieldsNum;		/**<数据列的个数*/	
	MYSQL_FIELD *astFields;			/**<结果集中各列的字段定义*/
	unsigned long *aulFieldsLength; /**<结果集中各列的长度*/
	unsigned int *aSortedFieldIdx;

	MYSQL_RES		*pstRes;		/**<当前操作的RecordSet*/
	MYSQL_ROW		stRow;			/**<当前操作的一行*/
	MYSQL			*pstConn;		/**<当前打开的Mysql连接*/
};
typedef struct tagTDRDBMysqlResult TDRDBMYSQLRESULT;
typedef struct tagTDRDBMysqlResult *LPTDRDBMYSQLRESULT;


struct tagTDRDBMysqlLink
{
	TDRDBMYSQLCONN		stMysqlConn;
	TDRDBMYSQLRESULT stRes;	
	char *szSwapBuff;
	int iSwapSize;	/*缓冲区的大小*/
};
typedef struct tagTDRDBMysqlLink TDRDBMYSQLLINK;
typedef struct tagTDRDBMysqlLink *LPTDRDBMYSQLLINK;





/**创建mysql连接句柄
*/
int tdr_dbdriver_open_handle(OUT LPTDRDBHANDLE a_phDBHandle, IN LPTDRDBMS a_pstDBMS, IN char *a_pszErrMsg);

/**关闭mysql连接句柄
*/
void tdr_dbdriver_close_handle(TDRDBHANDLE a_phDBHandle);


/**从结果集中取出一条记录 
*/
int tdr_dbms_fetch_row(IN TDRDBRESULTHANDLE a_hDBResult);

int tdr_set_table_options(IN LPTDRDBMS a_pstDBMS, IN LPTDRMETA a_pstMeta, INOUT LPTDRIOSTREAM a_pstIOStream);

#endif
