/**
*
* @file     tdr_iostream_i.h  
* @brief    TSF4G-IO输出输入模块
* 
* @author jackyai  
* @version 1.0
* @date 2007-04-23 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#ifndef TDR_IOSTREAM_I_H
#define TDR_IOSTREAM_I_H

#include <stdio.h>

typedef enum tagIOStreamType
{
	TDR_IOSTREAM_UNKOWN = 0,
	TDR_IOSTREAM_STRBUF = 1,  /**<缓冲区IO流*/
	TDR_IOSTREAM_FILE = 2,		/**<文件流*/
}TDRIOSTREAMTYPE;

typedef struct  
{
	char *pszBuff;  /**<缓冲区首地址*/
	int iBuffLen;	/**<缓冲区的大小*/
}TDRBUFFIOSTREAMPARAM;

typedef struct  
{
	FILE *a_fp;  /**<文件句柄*/	
}TDRFILEIOSTREAMPARAM;

struct  tagTDRIOStream
{
	TDRIOSTREAMTYPE emIOStreamType;
	union 
	{
		TDRBUFFIOSTREAMPARAM stBuffIOParam;
		TDRFILEIOSTREAMPARAM stFileIOParam;
	}stParam;
#define pszTDRIOBuff stParam.stBuffIOParam.pszBuff
#define iTDRIOBuffLen stParam.stBuffIOParam.iBuffLen
#define fpTDRIO		stParam.stFileIOParam.a_fp
};
typedef struct tagTDRIOStream TDRIOSTREAM;
typedef struct tagTDRIOStream *LPTDRIOSTREAM;

/**将信息输出到IO流中
*@param[in] a_pstIOStream 接受信息的IO流
*@param[in]	a_pszFormat 输出格式
*return 成功返回0，出错返回错误代码
*/
int tdr_iostream_write(LPTDRIOSTREAM a_pstIOStream, const char *a_pszFormat, ...);

#endif /*TDR_IOSTREAM_I_H*/
