#include "stdafx.h"
#include "tdr/tdr.h"
#include "../comm/error_report.h"
#include "../comm/scew_if.h"
#include "XmlKeyordParser.h"


CXmlKeyordParser::CXmlKeyordParser(void)
{
}

CXmlKeyordParser::~CXmlKeyordParser(void)
{
}


int CXmlKeyordParser::Parse(IN CString &szFile)
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

	scew_tree *pstTree = NULL;
	scew_element *pstRoot;
	scew_element *pstItem;	

	if (0 == iRet)
	{
		CString szError;
		if (0 != CreateXmlTreeByFileName(pstTree, szFile, szError))
		{
			WLogInfo(LOG_LEVEL_ERROR, "Error: 解析XML格式的关键字替换文件<%s>失败: %s", szFile, szError);
			iRet = -2;
		}
	}
	
	if (0 == iRet)
	{
		pstRoot = scew_tree_root(pstTree);
		if (NULL == pstRoot)
		{
			WLogInfo(LOG_LEVEL_ERROR, "Error: 解析XML格式的关键字替换文件<%s>中没有定义任何元素\r\n", szFile);
			iRet = -2;
		}
	}
	
	if (0 == iRet)
	{
		pstItem = scew_element_next(pstRoot, NULL);
		while (NULL != pstItem)
		{
			scew_attribute *pstAttr;
			CString szKey;
			CString szValue;

			if (0 != stricmp( scew_element_name(pstItem), TDR_TAG_MACROS ))
			{
				pstItem = scew_element_next(pstRoot, pstItem);
				continue;
			}
			
			pstAttr = scew_attribute_by_name(pstItem, TDR_TAG_CNNAME);
			if (NULL == pstAttr)
			{
				pstItem = scew_element_next(pstRoot, pstItem);
				continue;
			}
			szKey = scew_attribute_value(pstAttr);

			pstAttr = scew_attribute_by_name(pstItem, TDR_TAG_MACRO_VALUE);
			if (NULL == pstAttr)
			{
				pstItem = scew_element_next(pstRoot, pstItem);
				continue;
			}
			szValue = scew_attribute_value(pstAttr);

			szKey.MakeLower();
			CString szValueByKey = GetKeyMapValue(szKey);
			if ("" != szValueByKey)
			{
				if (szValueByKey == szValue)
				{
					WLogInfo(LOG_LEVEL_WARN, "warn: 关键子文件%s中定义了相同关键字<%s>的配置项\r\n", szFile, szKey);
					pstItem = scew_element_next(pstRoot, pstItem);
					continue;
				}else
				{
					WLogInfo(LOG_LEVEL_ERROR, "Error: 关键子文件%s中定义了相同关键字<%s>的配置项,且其对应要替换的值不同\r\n", szFile, szKey);
					iRet = -3;
					break;
				}		
			}
			m_stkeywordMap.insert(std::make_pair(szKey, szValue));
			pstItem = scew_element_next(pstRoot, pstItem);
		}/*while (NULL != pstItem)*/
	}/*if (bSucc)*/


	fpFile.Close();
	scew_tree_free(pstTree);

	return iRet;
}

