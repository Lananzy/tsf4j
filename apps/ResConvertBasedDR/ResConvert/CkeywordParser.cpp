#include "stdafx.h"
#include "../comm/ConvBase.h"
#include "../comm/error_report.h"
#include "CkeywordParser.h"


CkeywordParser::CkeywordParser(void)
{
}

CkeywordParser::~CkeywordParser(void)
{
	m_stkeywordMap.clear();
}


CString CkeywordParser::GetKeyMapValue(IN CString &szKey)
{
	CString keyTmp;

	keyTmp = szKey.MakeLower();
	KEYWORD_MAP::iterator it = m_stkeywordMap.find((LPCTSTR)keyTmp);
	if ( it != m_stkeywordMap.end() )
	{
		return (*it).second;
	}
	
	return "";	
}
