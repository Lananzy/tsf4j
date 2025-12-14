#include "stdafx.h"
#include "../comm/error_report.h"
#include "IniKeywordParser.h"

CIniKeywordParser::CIniKeywordParser(void)
{
}

CIniKeywordParser::~CIniKeywordParser(void)
{
}




int CIniKeywordParser::Parse(IN CString &szFile)
{
	CFileFind fpFind;
	CStdioFile fpFile;
	int iRet = 0;

	if( !fpFind.FindFile(szFile) )
	{
		WLogInfo(LOG_LEVEL_SEVERE, _T("文件 %s 不存在！."), szFile);
		return -1;
	}
	if( !fpFile.Open(szFile, CFile::modeRead) )
	{
		WLogInfo(LOG_LEVEL_ERROR, "Error: 打开文件%s读失败，请确定设置了正确的关键子文件！", szFile);
		return -1;
	}

	m_stkeywordMap.clear();

	CString szLine;
	while( fpFile.ReadString(szLine))
	{
		CString szKey;
		CString szValue;
		int iPos;


		szLine.TrimLeft(" ");
		szLine.TrimRight(" ");
		if (szLine.IsEmpty() || (szLine == "\n"))
		{
			continue;
		}		
		if (szLine.Left(3).CompareNoCase("rem") == 0)
		{
			continue;
		}

		iPos = 0;
		szKey = szLine.Tokenize("\n\t ", iPos);
		if ("" == szKey)
		{
			continue;
		}
		szValue = szLine.Tokenize("\n\t ", iPos);
		if ("" == szValue)
		{
			WLogInfo(LOG_LEVEL_WARN, "Warning: 关键子文件%s中的<%s>没有指定对应的映射值\r\n", szFile, szKey);
			continue;
		}		

		szKey.MakeLower();
		CString szValueByKey = GetKeyMapValue(szKey);
		if ("" != szValueByKey)
		{
			if (szValueByKey == szValue)
			{
				WLogInfo(LOG_LEVEL_WARN, "warn: 关键子文件%s中定义了相同关键字<%s>的配置项\r\n", szFile, szKey);
				continue;
			}else
			{
				WLogInfo(LOG_LEVEL_ERROR, "Error: 关键子文件%s中定义了相同关键字<%s>的配置项,且其对应要替换的值不同\r\n", szFile, szKey);
				iRet = -2;
				break;
			}		
		}
		m_stkeywordMap.insert(std::make_pair(szKey, szValue));

	}/*while( fpFile.ReadString(szLine))*/


	fpFile.Close();
	return iRet;
}

