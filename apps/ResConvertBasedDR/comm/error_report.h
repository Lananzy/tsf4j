/** 
*简单的图形日志系统，非线程安全
*/
#pragma once

#define LOG_LEVEL_MIN				0x00
#define LOG_LEVEL_DEBUG				0x01
#define LOG_LEVEL_WARN				0x02
#define LOG_LEVEL_ERROR				0x03
#define LOG_LEVEL_SEVERE			0x04
#define LOG_LEVEL_ANY				0x05
#define LOG_LEVEL_MAX				LOG_LEVEL_ANY

/** 初始化日志系统
*@param[in] pstEdit	纪录日志信息的CEdit控件指针
*@param[in] iLogLevel	纪录日志信息的级别
*/
void WLogInit(CEdit *pstEdit, int iLogLevel = LOG_LEVEL_ERROR);

/** 设置日志级别
*@param[in] iLogLevel	纪录日志信息的级别
*/
void WLogSetLogLevel(int iLogLevel);

/** 纪录一行日志
*@param[in] pstEdit	纪录日志信息的CEdit控件指针
*@param[in] iLogLevel	纪录日志信息的级别
*/
void WLogInfo(int iLogLevel, const char *fmt, ...);


extern CEdit *g_pedtResult;