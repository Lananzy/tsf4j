/*============================================================================*/
// File Name : qslog.h
// Purpose : QSpace System Log Function Extern
// Notes :
//     Lane Zhang Create, 2003/02/27
/*============================================================================*/

#ifndef _QSLOG_H
#define _QSLOG_H

#define QSLOGPATH                         "/usr/local/qqpet/log/"

// format template:
// 日期|进程号|结果|应用类型|用户QQ号码|宠物种类ID|宠物ID||||||应用处理字段A|应用处理字段B|...

// clientIP|GoodsType|Cosumed|PetAcct|Quota
#define PET_CHARGED_OK 4000
// clientIP|GoodsType|Cosumed|PetAcct|Quota
#define PET_BUYGOODS   4001
// clientIP|ErrCode|GoodsType|Cosumed|PetAcct|Quota
#define PET_PREPAY_YB  4002
// clientIP|ErrCode|GoodsType|Cosumed|PetAcct|Quota
#define PET_PREPAY_QC  4003
// clientIP|ErrCode|GoodsType|Cosumed|PetAcct|Quota
#define PET_CONFIRM_YB 4004
// clientIP|ErrCode|GoodsType|Cosumed|PetAcct|Quota
#define PET_CONFIRM_QC 4005
// clientIP|YuanBao|Duration
#define PET_WORKING    4006
// clientIP
#define PET_STAT_DISCARD 4007
// clientIP|GoodsType(food/health)|GoodsID|Strave|Cleanness|Strong|Defence|IQ|Charm
#define PET_FEED_TYPE  4008
// clientIP
#define PET_STAT_LOGIN 4009
// clientIP
#define PET_STAT_LOGOUT 4010
// clientIP
#define PET_STAT_DIVORCE 4011
#define PET_STUDYING		4012
#define PET_PRESENT_YB		4013
#define PET_GET_TITLE		4014
#define PET_OFFLINE_GROW	4015

// for TPCP Client API
#define PET_TPCP_REG 4012		//register pet
#define PET_TPCP_YB 4013		//present Yuanbao
#define PET_TPCP_GOODS 4014		//present Goods
#define PET_TPCP_ADOPT 4015		//get Adopt Relation
////////////////////////////////////////////////
#define LOG_COUNTER_GET 4021
#define LOG_COUNTER_INC 4022
#define LOG_COUNTER_DEC 4023
#define LOG_COUNTER_RESET 4024

//////////////////////////////////////////////////
//define webapp
#define PET_WEB_DOWORK 3000		//宠物打工模块
#define PET_WEB_HOSPITAL 3001	//宠物看病模块
#define PET_WEB_CLUB	3002		//宠物俱乐部

#define LOGTYPE_LOG_INFO    1
#define LOGTYPE_LOG_ERR     2
#define LOGTYPE_LOG_WARNING 2
#define LOGTYPE_LOG_DEBUG   3
#define LOGTYPE_LOG_TRACE	4

#define LOGRESULT_SUCCESS 0
#define LOGRESULT_FAILURE 1

#ifdef __cplusplus
extern "C" {
#endif

void OpenQSLog(const char* sModuleName, int iLogLevel, const char* sLogFilePath );
void QSLog( int iPriority, char* sFormat, ... );
void QSBackupLog( char* sFormat, ... );
void CloseQSLog( void );

#ifdef __cplusplus
}
#endif

#endif //_QSLOG_H

/*=====================End Of File============================================*/

