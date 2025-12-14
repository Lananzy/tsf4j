#include "stdafx.h"
#include "IniEntryMapParser.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_metalib_kernel_i.h"
#include "../comm/error_report.h"

//////////////////////////////////////////////////////////////////////////////////////
CMappNode::CMappNode(CString &szName, int iType):m_szEntryName(szName),m_iType(iType)
{
}

CMappNode::~CMappNode()
{

}

CLeafMappNode::CLeafMappNode(CString &szName, int iType, CString &szTitle, GETEXCELVALUEMETHOD enGetValueMethod/* = GET_EXCEL_VALUE_KEYWORDS*/):CMappNode(szName,iType),
m_szExcelTitle(szTitle),m_enGetValueMethod(enGetValueMethod)
{

}

CLeafMappNode::~CLeafMappNode()
{

}

CString &CLeafMappNode::GetExcelValueMethodStr()
{
	static CString szTmp;

	switch(m_enGetValueMethod)
	{
	case GET_EXCEL_VALUE_NUMBER:
		szTmp = INI_GET_EXCEL_VALUE_FLAG_NUMBER;
		break;
	case GET_EXCEL_VALUE_PERSENT:
		szTmp = INI_GET_EXCEL_VALUE_FLAG_PERSENT;
		break;
	case GET_EXCEL_VALUE_ALLINFO:
		szTmp = INI_GET_EXCEL_VALUE_FLAG_ALLINFO;
		break;
	case GET_ECCEL_VALUE_INTELLIGENT:
		szTmp = INI_GET_EXCEL_VALUE_FLAG_INTELLIGENT;
		break;
	default:
		szTmp = "";
		break;
	}

	return szTmp;		 
}



/////////////////////////////////////////////////////////////////////////////////////
CIniEntryMapParser::CIniEntryMapParser(void)
{

}

CIniEntryMapParser::~CIniEntryMapParser(void)
{
	Clear();
}

int CIniEntryMapParser::ParseEntryMap(IN LPTDRMETALIB &pstMetalib, IN LPTDRMETA pstMeta, IN const ENTRYMAPFILELIST &stFileList, IN const CString &szPath)
{
	CStdioFile fpFile;
	CFileFind fpFind;
	int iRet = 0;
	MAPSTACK stStack;
	CMappNode *pstRootData;
	ENTRYMAPNODE *pstRoot;
	STACKMETAITEM stTopItem;

	assert(NULL != pstMetalib);
	assert(NULL != pstMeta);

	Clear();

	if ((int)stFileList.size() <= 0)
	{
		WLogInfo(LOG_LEVEL_ERROR, _T("没有指定字段映射配置文件"));
		return -1;
	}

	m_szEntryMap = szPath + "\\" + stFileList[0];
	if( !fpFind.FindFile(m_szEntryMap) )
	{
		WLogInfo(LOG_LEVEL_SEVERE, _T("文件 %s 不存在！."), m_szEntryMap);
		return -1;
	}
	if( !fpFile.Open(m_szEntryMap, CFile::modeRead) )
	{
		WLogInfo(LOG_LEVEL_ERROR, _T("文件 %s 无法打开，请重新选择文件！\r\n"), m_szEntryMap);
		return -1;
	}	

	pstRootData = new CMappNode(CString(tdr_get_meta_name(pstMeta)), MAP_NODE_TYPE_MIDDLE);
	pstRoot = new ENTRYMAPNODE(pstRootData);
	stTopItem._pstNode = pstRoot;
	stTopItem._pstMeta = pstMeta;
	m_stEntryMapTree.SetRootNode(pstRoot);
	m_stMetaStack.push_back(stTopItem);

	CString szLine;
	while( fpFile.ReadString(szLine) )
	{
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
		if (szLine.Left(1) == INI_FILE_VAR_FLAG)
		{
			continue;
		}

		iRet = ParseEntryMapInfo(szLine);
		if (0 != iRet)
		{
			break;
		}
	}/*while( fpFile.ReadString(strLine) )*/

	fpFile.Close();


	return iRet;
}

int CIniEntryMapParser::ParseEntryMapInfo(CString &a_szLine)
{
	CString szTitle, szEntry;
	int iPos;
	int iRet;
	CString szTmp;
	CString szEntryMap;

	iPos = 0;
	szTitle = a_szLine.Tokenize("\n\t ", iPos);
	szEntry = a_szLine.Tokenize("\n\t ", iPos);
	szEntryMap = szEntry;
	if ("" == szEntry)
	{
		WLogInfo(LOG_LEVEL_ERROR, "ini配置文件<%s>中%s配置串缺少对应的成员字段串.", m_szEntryMap, szTitle);
		return -1;
	}

	iPos = szEntry.Find('.');
	if (0 > iPos)
	{
		//处理串: @aaa $aaa %aaa #aaa aaa
		iRet = AddOneEntryMap(szTitle, szEntry);
		if (0 != iRet)
		{
			WLogInfo(LOG_LEVEL_ERROR, "[%s]中不正确的配置项<%s %s> : %s", m_szEntryMap, szTitle, szEntryMap, szTmp);
		}
	}else
	{
		iPos = szEntry.Find('/');
		if (0 > iPos)
		{
			//处理串: aaaa.bbbb.@ccc
			iRet = PushMetasMap(szEntry);
			if (0 != iRet)
			{
				WLogInfo(LOG_LEVEL_ERROR, "[%s]中不正确的配置项<%s %s> : %s", m_szEntryMap, szTitle, szEntryMap, szTmp);
			}else
			{
				iRet = AddOneEntryMap(szTitle, szEntry);
			}
			if (0 != iRet)
			{
				WLogInfo(LOG_LEVEL_ERROR, "[%s]中不正确的配置项<%s %s> : %s", m_szEntryMap, szTitle, szEntryMap, szTmp);
			}


		}else /*if (0 > iPos)  */
		{
			int iPosDot = szEntry.Find('.');
			if (iPosDot < iPos)
			{
				//处理串: bbbb./bbbb.@ccc
				CString szTailer;
				CString szHeader = szEntry.Left(iPos);

				/*push header*/
				iRet = PushMetasMap(szHeader);
				if (0 != iRet)
				{
					WLogInfo(LOG_LEVEL_ERROR, "[%s]中不正确的配置项<%s %s> : %s", m_szEntryMap, szTitle, szEntryMap, szTmp);
				}				

				/*pop tailer*/
				szTailer = szEntry.Mid(iPos + 1);
				iPosDot = szTailer.ReverseFind('.');
				if (0 == iRet)
				{
					iRet = AddOneEntryMap(szTitle, szTailer.Mid(iPosDot + 1));
					if (0 != iRet)
					{
						WLogInfo(LOG_LEVEL_ERROR, "[%s]中不正确的配置项<%s %s> : %s", m_szEntryMap, szTitle, szEntryMap, szTmp);
					}
				}

				if (0 == iRet)
				{
					szTailer = szTailer.Left(iPosDot);
					iRet = PopMetasMap(szTailer);
					if (0 != iRet)
					{
						WLogInfo(LOG_LEVEL_ERROR, "[%s]中不正确的配置项<%s %s> : %s", m_szEntryMap, szTitle, szEntryMap, szTmp);
					}/*if (0 != iRet)*/
				}				

			}else
			{
				//处理串: /bbbb.@ccc
				CString szTailer = szEntry.Mid(iPos + 1);
				iPosDot = szTailer.ReverseFind('.');
				iRet = AddOneEntryMap(szTitle, szTailer.Mid(iPosDot + 1));
				if (0 != iRet)
				{
					WLogInfo(LOG_LEVEL_ERROR, "[%s]中不正确的配置项<%s %s> : %s", m_szEntryMap, szTitle, szEntryMap, szTmp);
				}

				if (0 == iRet)
				{
					szTailer = szTailer.Left(iPosDot);
					iRet = PopMetasMap(szTailer);
					if (0 != iRet)
					{
						WLogInfo(LOG_LEVEL_ERROR, "[%s]中不正确的配置项<%s %s> : %s", m_szEntryMap, szTitle, szEntryMap, szTmp);
					}/*if (0 != iRet)*/
				}				

			}/*if (iPosDot < iPos)*/
		}/*iPos = szEntry.Find('/'); if (0 > iPos)  */
	}/*iPos = szEntry.Find('.'); if (0 > iPos)*/

	return iRet;
}



int CIniEntryMapParser::AddOneEntryMap(CString &a_szTitle, CString &a_szEntry)
{
	GETEXCELVALUEMETHOD enMethod;
	STACKMETAITEM stTopItem;
	CString szEntryName;
	LPTDRMETA pstMeta;
	LPTDRMETAENTRY pstEntry;

	switch(a_szEntry[0])
	{
	case INI_GET_EXCEL_VALUE_FLAG_NUMBER:
		enMethod = GET_EXCEL_VALUE_NUMBER;
		szEntryName = a_szEntry.Mid(1);
		break;
	case INI_GET_EXCEL_VALUE_FLAG_PERSENT:
		enMethod = GET_EXCEL_VALUE_PERSENT;
		szEntryName = a_szEntry.Mid(1);
		break;
	case INI_GET_EXCEL_VALUE_FLAG_ALLINFO:
		enMethod = GET_EXCEL_VALUE_ALLINFO;
		szEntryName = a_szEntry.Mid(1);
		break;
	case INI_GET_EXCEL_VALUE_FLAG_INTELLIGENT:
		enMethod = GET_ECCEL_VALUE_INTELLIGENT;
		szEntryName = a_szEntry.Mid(1);
		break;
	default:
		enMethod = GET_EXCEL_VALUE_KEYWORDS;
		szEntryName = a_szEntry;
		break;
	}

	assert(0 < (int)m_stMetaStack.size());
	stTopItem = m_stMetaStack.back();
	assert(NULL != stTopItem._pstMeta);
	assert(NULL != stTopItem._pstNode);

	pstMeta = stTopItem._pstMeta;
	pstEntry = tdr_get_entryptr_by_name(pstMeta, (LPCTSTR)szEntryName);
	if (NULL == pstEntry)
	{
		WLogInfo(LOG_LEVEL_ERROR, "[%s]: %s结构体中没有定义名字为%s的成员\r\n", 
			m_szEntryMap, pstMeta->szName, szEntryName);
		return -1;
	}
	if (pstEntry->iType <= TDR_TYPE_COMPOSITE)
	{
		WLogInfo(LOG_LEVEL_ERROR, "[%s]: %s结构体名字为%s的成员不是基本数据类型\r\n", 
			m_szEntryMap, pstMeta->szName, szEntryName);
		return -2;
	}

	int iCount = GetMapNodeCountByName(stTopItem._pstNode, szEntryName);
	if ((0 < pstEntry->iCount) && (iCount >= pstEntry->iCount))
	{
		WLogInfo(LOG_LEVEL_ERROR, "[%s]:%s结构体名字为%s的成员是长度为%d的数组，之前已经有%d个字段映射到此成员\r\n", 
			m_szEntryMap, pstMeta->szName, szEntryName, pstEntry->iCount, pstEntry->iCount);
		return -3;
	}

	CLeafMappNode *pstData = new CLeafMappNode(szEntryName, MAP_NODE_TYPE_LEAF, a_szTitle, enMethod);
	ENTRYMAPNODE *pstChlid = new ENTRYMAPNODE(pstData);
	m_stEntryMapTree.AddChild(stTopItem._pstNode, pstChlid, false);

	return 0;
}





int CIniEntryMapParser::GenEntryCellIndex(OUT RESENTRYLIST &stList, IN CExcelSheet *pstSheet, IN LPTDRMETA pstMeta)
{
	int iRet = 0;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;	
	int iChange;
	int i;
	ENTRYMAPNODE *pstMapEntryNode;
	LPTDRMETALIB pstLib;

	assert(NULL != pstSheet);
	assert(NULL != pstMeta);


	ENTRYMAPNODE *pstRoot = m_stEntryMapTree.GetRootNode();
	if (NULL == pstRoot)
	{
		WLogInfo(LOG_LEVEL_ERROR, "Excel－metaentry映射树为空.");
		return -1;
	}

	stList.clear();
	pstLib = TDR_META_TO_LIB(pstMeta);
	pstCurMeta = pstMeta;
	pstStackTop = &stStack[0];
	pstStackTop->pszMetaSizeInfoTarget = NULL;	/*entry of meta*/
	pstStackTop->pszNetBase  = (char *)pstRoot; /*the entry map tree parent node */
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;	
	iStackItemCount = 1;
	pstStackTop->iMetaSizeInfoUnit = 0;
	pstStackTop->iMetaSizeInfoOff = 0; /*Host Off of Meta*/

	iChange = 0;
	while (0 < iStackItemCount)
	{
		LPTDRMETAENTRY pstEntry;		

		if ((0 != iChange) && (pstStackTop->iCount > 0))
		{
			pstEntry = (LPTDRMETAENTRY)pstStackTop->pszMetaSizeInfoTarget;
			pstStackTop->iMetaSizeInfoOff += pstEntry->iHUnitSize;
			pstMapEntryNode = GetNextMapNodeByName((ENTRYMAPNODE *)(pstStackTop-1)->pszNetBase, pstEntry->szName, (ENTRYMAPNODE *)pstStackTop->pszNetBase);
			if (NULL == pstMapEntryNode)
			{
				CEntryCellItem stItem;

				stItem.SetEntryHandler(pstEntry);
				for (i = pstStackTop->iCount; i > 0; i--)
				{
					stItem.SetEntryIdx(pstEntry->iCount - i);
					stItem.SetHOff((pstStackTop -1)->iMetaSizeInfoOff + pstEntry->iHOff);
					stList.push_back(stItem);
				}			
				
				//TRACE("Entry: %s HOff: %d Size: %d EntryIdx: %d  Index: %d\n", pstEntry->szName, stItem.GetHOff(), pstEntry->iHRealSize, stItem.GetEntryIdx(), -1);

				pstStackTop->iCount = 0;	/*找不到匹配的结构，则直接处理完*/
			}else
			{
				pstStackTop->pszNetBase = (char *)pstMapEntryNode;
			}/*if (NULL == pstMapEntryNode)*/			

			iChange = 0;
			continue;
		}
		iChange = 0;


		if (0 >= pstStackTop->iCount)
		{/*当前元数据数组已经处理完毕*/
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstCurMeta = pstStackTop->pstMeta;			
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;	
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			CEntryCellItem stItem;
			stItem.SetEntryHandler(pstEntry);
			stItem.SetHOff(pstStackTop->iMetaSizeInfoOff + pstEntry->iHOff);
			stList.push_back(stItem);
			TRACE("Entry: %s HOff: %d Size: %d EntryIdx: %d  Index: %d\n", pstEntry->szName, stItem.GetHOff(), pstEntry->iHRealSize, stItem.GetEntryIdx(), -1);

			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{/*复合数据类型*/
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = -2;
				//szError.AppendFormat("资源信息结构体<%s>嵌套层次太深，目前仅支持%d级结构\r\n", TDR_STACK_SIZE);
				WLogInfo(LOG_LEVEL_ERROR, "资源信息结构体<%s>嵌套层次太深，目前仅支持%d级结构.", TDR_STACK_SIZE);
				break;
			}

			pstMapEntryNode = GetNextMapNodeByName((ENTRYMAPNODE *)pstStackTop->pszNetBase, pstEntry->szName, NULL);
			if (NULL == pstMapEntryNode)
			{
				CEntryCellItem stItem;
				stItem.SetEntryHandler(pstEntry);
				for (i = 0; i < pstEntry->iCount; i++)
				{
					stItem.SetEntryIdx(i);
					stItem.SetHOff(pstStackTop->iMetaSizeInfoOff + pstEntry->iHOff);
					stList.push_back(stItem);
				}
				
				TRACE("Entry: %s HOff: %d Size: %d EntryIdx: %d  Index: %d\n", pstEntry->szName, stItem.GetHOff(), pstEntry->iHRealSize, stItem.GetEntryIdx(), -1);

				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}else
			{
				pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				iStackItemCount++;
				pstStackTop++;
				pstStackTop->pstMeta = pstCurMeta;
				pstStackTop->pszMetaSizeInfoTarget = (char *)pstEntry;
				pstStackTop->iCount = ((pstEntry->iCount <= 0) ? GetMapNodeCountByName((ENTRYMAPNODE *)pstStackTop->pszNetBase, CString(pstEntry->szName)) : pstEntry->iCount );
				pstStackTop->idxEntry = 0;
				pstStackTop->pszNetBase = (char *)pstMapEntryNode;
				pstStackTop->iMetaSizeInfoUnit = 1;
				pstStackTop->iMetaSizeInfoOff = (pstStackTop -1)->iMetaSizeInfoOff + pstEntry->iHOff;
			}/*if (NULL == pstMapEntryNode)*/			
		}else if (TDR_TYPE_WSTRING >= pstEntry->iType)
		{
			iRet = GenSimpleEntryCellIndex(stList, pstSheet,(ENTRYMAPNODE *)pstStackTop->pszNetBase, pstStackTop->iMetaSizeInfoOff, pstEntry);
			if (0 != iRet)
			{
				break;
			}
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/
	}/*while (0 < iStackItemCount)*/

	return iRet;
}

ENTRYMAPNODE *CIniEntryMapParser::GetNextMapNodeByName(ENTRYMAPNODE *pstParent, const char *pszName, ENTRYMAPNODE *pstChlid)
{
	ENTRYMAPNODE *pstTmpNode;

	assert(NULL != pstParent);
	assert(NULL != pszName);

	pstTmpNode = m_stEntryMapTree.GetNextChild(pstParent, pstChlid);
	while (NULL != pstTmpNode)
	{
		CMappNode *pstData = pstTmpNode->GetNodeData();
		if (NULL == pstData)
		{
			pstTmpNode = m_stEntryMapTree.GetNextSibling(pstTmpNode);
			continue;
		}
		if (pstData->GetName().Compare(pszName) == 0)
		{
			break;
		}
		pstTmpNode = m_stEntryMapTree.GetNextSibling(pstTmpNode);
	}/*while (NULL != pstTmpNode)*/

	return pstTmpNode;
}

int CIniEntryMapParser::GetMapNodeCountByName(ENTRYMAPNODE *pstNode, CString &szName)
{
	ENTRYMAPNODE *pstChlid;

	int iCount = 0;
	assert(NULL != pstNode);
	pstChlid = m_stEntryMapTree.GetNextChild(pstNode, NULL);
	while (NULL != pstChlid)
	{
		CMappNode *pstData = pstChlid->GetNodeData();
		if ((NULL != pstData) && (szName == pstData->GetName()) )
		{
			iCount++;
		}
		pstChlid = m_stEntryMapTree.GetNextSibling(pstChlid);
	}

	return iCount;
}


/*导出映射树，调试时使用*/
void CIniEntryMapParser::DumpMapInfo(CString &szFile)
{
	CStdioFile fpFile;
	CMappNode *pstData;
	CString szLine;

	CFileException e;

	if( !fpFile.Open(szFile, CFile::modeCreate | CFile::modeWrite  | CFile::shareDenyNone) )
	{
		return;
	}

	std::list<ENTRYMAPNODE *> stStack;
	ENTRYMAPNODE *pstNode = m_stEntryMapTree.GetRootNode();
	if (NULL == pstNode)
	{
		fpFile.Close();
		return;
	}


	stStack.push_back(pstNode);
	while (!stStack.empty())
	{
		pstNode = stStack.back();
		stStack.pop_back();
		//		szSpace = szSpace.Mid(2);

		if (NULL == pstNode)
		{
			continue;
		}

		pstData = pstNode->GetNodeData();
		if (NULL == pstData)
		{
			continue;
		}

		if (MAP_NODE_TYPE_LEAF == pstData->GetType())
		{
			CLeafMappNode *pstLeafData = (CLeafMappNode*)pstData;
			szLine.Format("%s %s%s \n", pstLeafData->GetTitle(), 
				pstLeafData->GetExcelValueMethodStr(), pstLeafData->GetName());
			fpFile.WriteString(szLine);
		}else
		{
			szLine.Format("%s\n", pstData->GetName());
			fpFile.WriteString(szLine);
			ENTRYMAPNODE *pstChlid = m_stEntryMapTree.GetNextChild(pstNode, NULL);
			while (NULL != pstChlid)
			{
				stStack.push_back(pstChlid);
				pstChlid = m_stEntryMapTree.GetNextSibling(pstChlid);
			}
		}/*if (MAP_NODE_TYPE_LEAF == pstData->GetType())*/

	}/*while (!stStack.empty())*/

}

void CIniEntryMapParser::Clear()
{
	std::list<ENTRYMAPNODE *> stStack;
	ENTRYMAPNODE *pstRoot = m_stEntryMapTree.GetRootNode();
	if (NULL ==  pstRoot)
	{
		return;
	}
	stStack.push_back(pstRoot);

	while (!stStack.empty())
	{
		pstRoot = stStack.front();
		stStack.pop_front();

		ENTRYMAPNODE *pstChlid = m_stEntryMapTree.GetNextChild(pstRoot, NULL);
		while (NULL != pstChlid)
		{
			stStack.push_back(pstChlid);
			pstChlid = m_stEntryMapTree.GetNextSibling(pstChlid);
		}

		CMappNode *pstData = pstRoot->GetNodeData();
		if (NULL != pstData)
		{
			free(pstData);
		}
		free(pstRoot);
	}/*while (!stStack.empty())*/
}

inline int CIniEntryMapParser::GenSimpleEntryCellIndex(OUT RESENTRYLIST &stList, IN CExcelSheet *pstSheet, IN ENTRYMAPNODE *pstParentNode, IN int iHMetaOff, IN LPTDRMETAENTRY pstEntry)
{
	ENTRYMAPNODE *pstMapEntryNode;
	int i;
	int iRet = 0;
	int iCount;

	assert(NULL != pstSheet);
	assert(NULL != pstEntry);
	assert(NULL != pstParentNode);

#ifdef _DEBUG
	DumpMapInfo(CString("EntryMapTree.txt"));
#endif

	int iMaxCol = pstSheet->GetColCount();
	pstMapEntryNode = GetNextMapNodeByName(pstParentNode, pstEntry->szName, NULL);
	iCount = 0;
	while (NULL != pstMapEntryNode)
	{
		CLeafMappNode *pstData = (CLeafMappNode *)pstMapEntryNode->GetNodeData();
		assert(NULL != pstData);

		for (i = 1; i <= iMaxCol; i++)
		{
			CString szTitle = pstSheet->GetCell(EXCEL_ROW_OF_ENTRY_TITLE, i)->GetValue();			

			szTitle.TrimLeft(" \r\n");
			szTitle.TrimRight(" \r\n");

			if (szTitle.IsEmpty())
			{
				continue;
			}

			if (szTitle == pstData->GetTitle())
			{
				break;
			}
		}/*for (i = 1; i < iMaxCol; i++)*/

		if (i > iMaxCol)
		{
			//szError.AppendFormat("Warning: 在excel页表<%s>没有找到标题为<%s>的列, 请检查ini配置文化和Excel文件是否正确\r\n", pstSheet->GetName(), pstData->GetTitle());
			WLogInfo(LOG_LEVEL_WARN, "Warning: 在excel页表<%s>没有找到标题为<%s>的列, 请检查ini配置文化和Excel文件是否正确.", pstSheet->GetName(), pstData->GetTitle());
			pstMapEntryNode = GetNextMapNodeByName(pstParentNode, pstEntry->szName, pstMapEntryNode);
			continue;
		}

		CEntryCellItem stItem;
		stItem.SetCellIndex(i);
		stItem.SetEntryIdx(iCount);
		stItem.SetEntryHandler(pstEntry);
		stItem.SetGetValueMethod(pstData->GetGetExcelValueMethod());
		stItem.SetHOff(iHMetaOff + pstEntry->iHOff);
		stList.push_back(stItem);
		TRACE("Entry: %s HOff: %d Size: %d EntryIdx: %d  Index: %d M: %d Title: %s\n", pstEntry->szName, stItem.GetHOff(), pstEntry->iHRealSize, stItem.GetEntryIdx(),
			i, stItem.GetGetValueMethod(), pstData->GetTitle());

		iCount++;		
		if ((0 < pstEntry->iCount) && (iCount > pstEntry->iCount))
		{
			WLogInfo(LOG_LEVEL_ERROR, "名字为%s的成员是长度为%d的数组，Excel页表<%s>已经有%d个字段映射到此成员\r\n", 
				pstEntry->szName, pstEntry->iCount, pstSheet->GetName(), iCount);
			iRet =  -2;
			break;
		}

		pstMapEntryNode = GetNextMapNodeByName(pstParentNode, pstEntry->szName, pstMapEntryNode);
	}/*while (NULL != pstMapEntryNode)*/

	if ((0 == iRet) && (0 < pstEntry->iCount) && (iCount < pstEntry->iCount))
	{
		CEntryCellItem stItem;
		stItem.SetEntryHandler(pstEntry);
		stItem.SetHOff(iHMetaOff + pstEntry->iHOff);
		for (i = iCount; i < pstEntry->iCount; i++)
		{
			stItem.SetEntryIdx(iCount);
			stList.push_back(stItem);
		}
		
		
		TRACE("Entry: %s HOff: %d Size: %d EntryIdx: %d  Index: %d M: %d\n", pstEntry->szName, stItem.GetHOff(), pstEntry->iHRealSize, stItem.GetEntryIdx(), stItem.GetCellIndex(), stItem.GetGetValueMethod());
	}

	return iRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CNameIniEntryMapParser::CNameIniEntryMapParser()
{

}

CNameIniEntryMapParser::~CNameIniEntryMapParser()
{

}


int CNameIniEntryMapParser::PushMetasMap(INOUT CString &szEntryMap)
{
	CString szEntryName;
	LPTDRMETA pstMeta;
	LPTDRMETAENTRY pstEntry;

	int iPos;
	int iRet = 0;
	STACKMETAITEM stItem;

	assert(0 < (int)m_stMetaStack.size());


	iPos = szEntryMap.Find('.');
	do{

		stItem = m_stMetaStack.back();
		pstMeta = stItem._pstMeta;

		szEntryName = szEntryMap.Left(iPos);
		pstEntry = tdr_get_entryptr_by_name(pstMeta, szEntryName);
		if (NULL == pstEntry)
		{
			WLogInfo(LOG_LEVEL_ERROR, "[%s]: %s结构体中没有定义成员名为%s的成员\r\n", 
				m_szEntryMap, pstMeta->szName, szEntryName);
			iRet = -1;
			break;
		}

		int iCount = GetMapNodeCountByName(stItem._pstNode, szEntryName);
		if ((0 < pstEntry->iCount) && (iCount >= pstEntry->iCount))
		{
			WLogInfo(LOG_LEVEL_ERROR, "[%s]: %s结构体中类型为%s的成员是长度为%d的数组，之前已经有%d个字段映射到此成员\r\n", 
				m_szEntryMap, pstMeta->szName, szEntryName, pstEntry->iCount, pstEntry->iCount);
			iRet = -2;
			break;
		}

		CMappNode *pstData = new CMappNode(CString(pstEntry->szName), MAP_NODE_TYPE_MIDDLE);
		ENTRYMAPNODE *pstChlid = new ENTRYMAPNODE(pstData);
		m_stEntryMapTree.AddChild(stItem._pstNode, pstChlid, false);

		LPTDRMETALIB pstLib = TDR_META_TO_LIB(pstMeta);
		stItem._pstMeta = tdr_get_entry_type_meta(pstLib, pstEntry);
		stItem._pstNode = pstChlid;
		m_stMetaStack.push_back(stItem);

		szEntryMap = szEntryMap.Mid(iPos+1);
		iPos = szEntryMap.Find('.');
	}while( iPos >= 0 );

	return iRet;
}

int CNameIniEntryMapParser::PopMetasMap(CString &szEntryMap)
{
	STACKMETAITEM stTopItem;
	CString szEntryName;
	int iPosDot;
	int iRet = 0;
	

	assert(0 < (int)m_stMetaStack.size());

	
	do{
		iPosDot = szEntryMap.ReverseFind('.');
		szEntryName = szEntryMap.Mid(iPosDot+1);
		

		stTopItem = m_stMetaStack.back();
		assert(NULL != stTopItem._pstMeta);
		assert(NULL != stTopItem._pstNode);

		CMappNode *pstData = (CMappNode *)stTopItem._pstNode->GetNodeData();
		if (szEntryName.Compare(pstData->GetName()))
		{
			WLogInfo(LOG_LEVEL_ERROR, "[%s]: 当前期望匹配成员名为%s结构，而给定成员名为%s,请检查配置项.", 
				m_szEntryMap, pstData->GetName(), szEntryName);
			iRet = -1;
			break;
		}else
		{
			m_stMetaStack.pop_back();
		}

		if (0 < iPosDot)
		{
			szEntryMap = szEntryMap.Left(iPosDot);
		}
	}while( iPosDot >=0 );


	return iRet;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
CTypeIniEntryMapParser::CTypeIniEntryMapParser()
{

}

CTypeIniEntryMapParser::~CTypeIniEntryMapParser()
{

}

int CTypeIniEntryMapParser::PushMetasMap(INOUT CString &szEntryMap)
{
	CString szEntryName;
	LPTDRMETA pstMeta;
	LPTDRMETAENTRY pstEntry;
	int i;
	int iPos;
	int iRet = 0;
	LPTDRMETA pstTypeMeta;
	STACKMETAITEM stItem;

	assert(0 < (int)m_stMetaStack.size());


	iPos = szEntryMap.Find('.');
	do{

		stItem = m_stMetaStack.back();
		pstMeta = stItem._pstMeta;

		szEntryName = szEntryMap.Left(iPos);
		for (i = 0; i < pstMeta->iEntriesNum; i++)
		{
			pstEntry = pstMeta->stEntries + i;
			if (TDR_INVALID_PTR == pstEntry->ptrMeta)
			{
				continue;
			}
			LPTDRMETALIB pstLib = TDR_META_TO_LIB(pstMeta);
			pstTypeMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			if (szEntryName.Compare(pstTypeMeta->szName) == 0)
			{
				break;
			}
		}
		if (i >= pstMeta->iEntriesNum)
		{
			WLogInfo(LOG_LEVEL_ERROR, "[%s]: %s结构体中没有定义类型为%s的成员\r\n", 
				m_szEntryMap, pstMeta->szName, szEntryName);
			iRet = -1;
			break;
		}

		int iCount = GetMapNodeCountByName(stItem._pstNode, CString(pstEntry->szName));
		if ((0 < pstEntry->iCount) && (iCount >= pstEntry->iCount))
		{
			WLogInfo(LOG_LEVEL_ERROR, "[%s]: %s结构体中类型为%s的成员是长度为%d的数组，之前已经有%d个字段映射到此成员\r\n", 
				m_szEntryMap, pstMeta->szName, szEntryName, pstEntry->iCount, pstEntry->iCount);
			iRet = -2;
			break;
		}

		CMappNode *pstData = new CMappNode(CString(pstEntry->szName), MAP_NODE_TYPE_MIDDLE);
		ENTRYMAPNODE *pstChlid = new ENTRYMAPNODE(pstData);
		m_stEntryMapTree.AddChild(stItem._pstNode, pstChlid, false);

		stItem._pstMeta = pstTypeMeta;
		stItem._pstNode = pstChlid;
		m_stMetaStack.push_back(stItem);

		szEntryMap = szEntryMap.Mid(iPos+1);
		iPos = szEntryMap.Find('.');
	}while( iPos >= 0 );

	return iRet;
}

int CTypeIniEntryMapParser::PopMetasMap(CString &szEntryMap)
{
	STACKMETAITEM stTopItem;
	CString szEntryName;
	int iPosDot;
	int iRet = 0;

	assert(0 < (int)m_stMetaStack.size());

	
	do{
		iPosDot = szEntryMap.ReverseFind('.');
		szEntryName = szEntryMap.Mid(iPosDot+1);


		stTopItem = m_stMetaStack.back();
		assert(NULL != stTopItem._pstMeta);
		assert(NULL != stTopItem._pstNode);

		if (szEntryName.Compare(stTopItem._pstMeta->szName))
		{
			WLogInfo(LOG_LEVEL_ERROR, "[%s]: 当前期望匹配类型名为%s结构，而给定结构名为%s,请检查配置项.", 
				m_szEntryMap, stTopItem._pstMeta->szName, szEntryName);
			iRet = -1;
			break;
		}else
		{
			m_stMetaStack.pop_back();
		}

		if (0 < iPosDot)
		{
			szEntryMap = szEntryMap.Left(iPosDot);
		}
		
	}while( iPosDot >=0 );


	return iRet;
}


