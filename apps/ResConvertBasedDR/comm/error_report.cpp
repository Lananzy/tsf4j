#include "stdafx.h"
#include "error_report.h"

CEdit *g_pedtResult = NULL;
int g_iLogLevel = LOG_LEVEL_ERROR;


/** 初始化日志系统
*@param[in] pstEdit	纪录日志信息的CEdit控件指针
*@param[in] iLogLevel	纪录日志信息的级别
*/
void WLogInit(CEdit *pstEdit, int iLogLevel/* = LOG_LEVEL_ERROR*/)
{
	g_pedtResult = pstEdit;
	if ((iLogLevel >= LOG_LEVEL_MIN) && (iLogLevel <= LOG_LEVEL_MAX))
	{
		g_iLogLevel = iLogLevel;
	}
}

/** 设置日志级别
*@param[in] iLogLevel	纪录日志信息的级别
*/
void WLogSetLogLevel(int iLogLevel)
{
	if ((iLogLevel >= LOG_LEVEL_MIN) && (iLogLevel <= LOG_LEVEL_MAX))
	{
		g_iLogLevel = iLogLevel;
	}
}

/** 纪录一行日志
*@param[in] pstEdit	纪录日志信息的CEdit控件指针
*@param[in] iLogLevel	纪录日志信息的级别
*/
void WLogInfo(int iLogLevel, const char *fmt, ...)
{
	char x[2048];
	va_list va;

	if (iLogLevel < g_iLogLevel)
	{
		return;
	}

	va_start(va, fmt);
	vsnprintf(x, sizeof(x),fmt, va);
	va_end(va);

	if (iLogLevel == LOG_LEVEL_SEVERE)
	{
		AfxMessageBox(x);
	}

	CString text;
	ASSERT(g_pedtResult);
	g_pedtResult->GetWindowText(text);
	text += x;
	text += "\r\n";
	g_pedtResult->SetWindowText(text);
}