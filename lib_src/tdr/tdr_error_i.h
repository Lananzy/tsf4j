/**
*
* @file     tdr_error.h 
* @brief    Internal error functions
* 
* @author jackyai  
* @version 1.0
* @date 2007-03-28 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/




#ifndef TDR_XERROR_I_H
#define TDR_XERROR_I_H

#include "tdr/tdr_error.h"

#ifdef TDR_TRACE_LAST_ERROR 
#define TDR_SET_LAST_ERROR  tdr_set_last_error
#else
#define TDR_SET_LAST_ERROR  //
#endif

/* Sets TDR internal last error. */
void tdr_set_last_error(int code);

/* Gets TDR internal last error. */
int tdr_get_last_error();

/**
* 获取最近的错误代码
* @return 错误代码
* @see tdr_error_string
*/
int tdr_error_code();

#endif /* TDR_XERROR_I_H */
