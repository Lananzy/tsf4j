#include "stdafx.h"
#include "ResStyleList.h"
#include <algorithm>

using namespace std;

CResStyleManager::CResStyleManager(void)
{
}

CResStyleManager::~CResStyleManager(void)
{
}


int CResStyleManager::InsertResStyle(CResStyle &stResStyle, CString &szError)
{
	int iRet = 0;
	int iResCount;
	int i;

	iResCount = (int)m_stStyleList.size();
	for (i= 0; i < iResCount; i++)
	{
		if (m_stStyleList[i].m_iID != stResStyle.m_iID) 
		{
			continue;
		}
		if (m_stStyleList[i].m_szResStyleName != stResStyle.m_szResStyleName)
		{
			szError.Format(_T("ResStyle<%s> 的ID和ResStyle<%s>的ID相同，ResStyleList中的ID必须是唯一的"),
				stResStyle.m_szResStyleName, m_stStyleList[i].m_szResStyleName);
			iRet = -1;
		}
		break;	
	}

	if (i >= iResCount)
	{
		m_stStyleList.push_back(stResStyle);
	}

	return iRet;
}

void CResStyleManager::ClearResStyleList()
{
	m_stStyleList.clear();
}

CResStyle *CResStyleManager::GetResStyle(int iID)
{
	int iResStyleCount = (int)m_stStyleList.size();

	for (int i= 0; i < iResStyleCount; i++)
	{
		if (m_stStyleList[i].m_iID == iID)
		{
			return &m_stStyleList[i];
		}
	}

	return NULL;
}
