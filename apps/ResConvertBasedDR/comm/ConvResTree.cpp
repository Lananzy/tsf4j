#include "StdAfx.h"
#include <assert.h>
#include "ConvResTree.h"

CCommNode::CCommNode():m_szName(_T(""))
{
	m_iType = RES_TREE_NODE_TYPE_COMM;
}

CCommNode::CCommNode(const CString &szName, int iType /*= RES_TREE_NODE_TYPE_COMM*/):m_szName(szName),m_iType(iType)
{
}



CCommNode::~CCommNode()
{
}

////////////////////////////////////////////////////////////////////
CResNode::CResNode():CCommNode(_T(""),RES_TREE_NODE_TYPE_RES)
{
	m_enSortMethod = SORT_METHOD_NO;
}

CResNode::CResNode(CString &szName):CCommNode(szName, RES_TREE_NODE_TYPE_RES)
{
	m_enSortMethod = SORT_METHOD_NO;
	m_bIsMutilTables = false;
	m_iSubTableIDColumn = 1;
	m_bIsMapEntryByName = true;
}

CResNode::~CResNode()
{
}


void CResNode::SetEntryMapFile(CString &stName)
{
	stName.TrimLeft(" \\/");
	m_aszEntryMapFiles.push_back(stName);
}

 ENTRYMAPFILELIST &CResNode::GetEntryMapFiles()
{
	return m_aszEntryMapFiles;
}

 void CResNode::SetBinFile(CString &szBinFile)
{
	szBinFile.TrimLeft(" \\/");
	m_szBinFile = szBinFile;
}

 CString &CResNode::GetBinFile()
{
	return m_szBinFile;
}

 void CResNode::SetSortMethod(SORTMETHOD enSortMethod)
{
	m_enSortMethod = enSortMethod;
}

void CResNode::SetSortMethod(const char *pszMethod)
{
	assert(NULL != pszMethod);
	if (_stricmp(pszMethod, "Asc") == 0)
	{
		m_enSortMethod = SORT_METHOD_ASC;
	}else if (_stricmp(pszMethod, "Desc") == 0)
	{
		m_enSortMethod = SORT_METHOD_DESC;
	}
}

 SORTMETHOD CResNode::GetSortMethod()
{
	return m_enSortMethod;
}

 void CResNode::AddResStyleID(int iID)
{
	m_aiResStyleIDs.push_back(iID);
}

 bool CResNode::IsExpectedResStyles(int iStyleID)
{
	int i;
	int iCount = (int)m_aiResStyleIDs.size();

	for (i = 0; i < iCount; i++)
	{
		if (iStyleID == m_aiResStyleIDs[i])
		{
			break;
		}
	}

	if (i < iCount)
	{
		return true;
	}

	return false;
}

 void CResNode::AddExcelFile(CString &szExcelFile)
{
	szExcelFile.TrimLeft(" \\/");
	m_astExcelFiles.push_back(szExcelFile);
}

 EXCELFILELIST &CResNode::GetExcelFiles()
{
	return m_astExcelFiles;
}

