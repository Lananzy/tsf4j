/**
*
* @file     tdr_tools.h 
* @brief    一些辅助工具
* 
* @author jackyai  
* @version 1.0
* @date 2007-06-01 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#ifndef TDR_OS_DIRSEP
#ifdef WIN32
#define TDR_OS_DIRSEP		'\\'
#else
#define TDR_OS_DIRSEP		'/'
#endif
#endif

/**将表示版本信息的整数转换成约定的字符串
*版本:00101010
*分解:00        1        01              010
*变换:0.1       beta    01   build    010
*/
char *tdr_parse_version_string(unsigned int unVer);

char *tdr_basename(const char *a_pszObj);
