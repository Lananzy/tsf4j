#include "StdAfx.h"
#include "ConvTreeParser.h"
#include "ConvDefine.h"

ConvTreeParser::ConvTreeParser(void)
{
	m_szBinFilesPath = _T(".\\");
	m_bIsNewFormat = _T(".\\");
	m_szExcelFilesPath = _T(".\\");
	m_szKeywordFile = _T("");
	m_szMetalibFile = _T("");
	m_szEntryMapFilesPath =  _T(".\\");
	m_bIsNewFormat = true;
}

ConvTreeParser::~ConvTreeParser(void)
{
	DeleteResTree();
}

void ConvTreeParser::DeleteResTree()
{
	FreeSubTree(m_stResTree.GetRootNode());
	m_stResTree.SetRootNode(NULL);
}

void ConvTreeParser::FreeSubTree(CRESTREENODE *pstNode)
{
	CRESTREENODE *pstTempNode,*pstNextNode;


	if (NULL == pstNode)
	{
		return ;
	}

	/*释放子节点*/
	pstNextNode = m_stResTree.GetNextChild(pstNode, NULL);
	for (;NULL != pstNextNode;)
	{
		pstTempNode = pstNextNode;
		pstNextNode = m_stResTree.GetNextSibling(pstNextNode);
		FreeSubTree(pstTempNode);
	}

	/*释放本身的数据*/
	FreeNode(pstNode);
	
}

void ConvTreeParser::NomalizeResTree()
{
	CRESTREENODE *pstRoot = NULL;
	CRESTREENODE *pstChild = NULL;

	pstRoot = m_stResTree.GetRootNode();
	if (NULL == pstRoot)
	{
		return;
	}

	if (_T("") != pstRoot->GetNodeData()->GetNodeName())
	{
		return ;
	}

	

	pstChild = m_stResTree.GetNextChild(pstRoot, NULL);
	if (NULL == pstChild)
	{
		return;
	}
	if (NULL != m_stResTree.GetNextChild(pstRoot, pstChild))
	{
		return;
	}


	/*将儿子节点调整为根节点*/
	m_stResTree.SetRootNode(pstChild);
	FreeNode(pstRoot);
}

void ConvTreeParser::FreeNode(CRESTREENODE *pstNode)
{
	CCommNode *pstData;

	pstData = pstNode->GetNodeData();
	if (NULL == pstData)
	{
		delete pstData;
	}
	delete pstNode;
}

CRESTREENODE *ConvTreeParser::InsertCommNode(CRESTREENODE *pstParent, CString &szNodeName)
{
	CRESTREENODE *pstTmpNode;
	CCommNode *pstData;

	assert(NULL != pstParent);

	pstTmpNode = m_stResTree.GetNextChild(pstParent, NULL);
	for (;NULL != pstTmpNode;)
	{
		pstData = pstTmpNode->GetNodeData();
		if (szNodeName == pstData->GetNodeName())
		{
			break;
		}
		pstTmpNode = m_stResTree.GetNextChild(pstParent, pstTmpNode);
	}

	/*没有找到同名节点，则删除*/
	if (NULL == pstTmpNode)
	{
		pstData = new CCommNode(szNodeName);
		pstTmpNode = new CRESTREENODE(pstData);
		m_stResTree.AddChild(pstParent, pstTmpNode,false);
	}

	return pstTmpNode;
}

void ConvTreeParser::ParseResStyleList(CResNode *pstData, CString &szStyles, CString &szError)
{
	int iPos;
	CString szToken;

	szStyles.TrimLeft();
	szStyles.TrimRight();

	iPos = szStyles.Find(_T(","));
	while (iPos >= 0)
	{
		szToken = szStyles.Left(iPos);
		if (szToken != "")
		{	
			int iID;

			iID = atoi((char   *)(LPCTSTR)szToken); 
			if (NULL != m_stResStyleList.GetResStyle(iID))
			{
				pstData->AddResStyleID(iID);
			}else
			{
				szError.AppendFormat(_T("warnning: %s元素<Name=%s>的%s属性值中包含了未定义的资源ID<%d>\r\n"),
					CONV_TREE_XML_TAG_RESNODE, pstData->GetNodeName(), CONV_TREE_XML_TAG_BINSTYLES, iID);
			}
		}
		szStyles = szStyles.Right(iPos + 1);
		iPos = szStyles.Find(_T(","));
	}

	if (szStyles != "")
	{
		pstData->AddResStyleID(atoi((char   *)(LPCTSTR)szStyles));
	}
}

bool ConvTreeParser::Dump2XML(CString &szFile, CString &szError)
{
	CStdioFile fpFile;
	CFileFind fpFind;
	CString	szLine;

	if( !fpFile.Open(szFile, CFile::modeCreate | CFile::modeWrite  | CFile::shareDenyNone) )
	{
		szError.AppendFormat(_T("无法打开文件 %s 写，请重新选择文件！\r\n"), szFile);
		return false;
	}

	fpFile.WriteString(_T("<?xml version=\"1.0\" encoding=\"GBK\" standalone=\"yes\" ?>\n\n"));

	/*root element*/
	szLine.Format(_T("<%s "), CONV_TREE_XML_TAG_ROOT);
	fpFile.WriteString(szLine);
	if (!m_bIsNewFormat)
	{
		szLine.Format(_T("%s=\"%s\" "), CONV_TREE_XML_TAG_ISNEWCONFIGFORMAT, CONV_TREE_XML_TAG_FALSE);
		fpFile.WriteString(szLine);
	}
	fpFile.WriteString(">\n");
	

	/*base information*/
	szLine.Format(_T("  <%s %s=\"%s\" />\n"), CONV_TREE_XML_TAG_METALIBFILE, CONV_TREE_XML_TAG_FILENAME, m_szMetalibFile);
	fpFile.WriteString(szLine);
	szLine.Format(_T("  <%s %s=\"%s\" />\n"), CONV_TREE_XML_TAG_EntryMapPATH, CONV_TREE_XML_TAG_PATH, m_szEntryMapFilesPath);
	fpFile.WriteString(szLine);
	szLine.Format(_T("  <%s %s=\"%s\" />\n"), CONV_TREE_XML_TAG_BINFILEPATH, CONV_TREE_XML_TAG_PATH, m_szBinFilesPath);
	fpFile.WriteString(szLine);
	szLine.Format(_T("  <%s %s=\"%s\" />\n"), CONV_TREE_XML_TAG_EXCELFILEPATH, CONV_TREE_XML_TAG_PATH, m_szExcelFilesPath);
	fpFile.WriteString(szLine);
	szLine.Format(_T("  <%s %s=\"%s\" />\n"), CONV_TREE_XML_TAG_KEYWORDFILE, CONV_TREE_XML_TAG_FILENAME, m_szKeywordFile);
	fpFile.WriteString(szLine);

	/*res style lists*/
	RESSTYLELIST &ResList = m_stResStyleList.GetResStyleList();
	if (0 < (int)ResList.size())
	{
		fpFile.WriteString(_T("\n\n"));
		szLine.Format(_T("  <%s>\n"), CONV_TREE_XML_TAG_RESSTYLELIST);
		fpFile.WriteString(szLine);
		for (int i = 0; i < (int)ResList.size(); i++)
		{
			szLine.Format(_T("    <%s %s=\"%s\" %s=\"%d\" />\n"), CONV_TREE_XML_TAG_RESSTYLE, CONV_TREE_XML_TAG_RESNAME, ResList[i].m_szResStyleName,
				CONV_TREE_XML_TAG_RESID, ResList[i].m_iID);
			fpFile.WriteString(szLine);
		}
		szLine.Format(_T("  </%s>\n"), CONV_TREE_XML_TAG_RESSTYLELIST);
		fpFile.WriteString(szLine);
	}/*if (0 < (int)ResList.size())*/


	/*ConvRes Tree*/
	CRESTREENODE *pstRoot = m_stResTree.GetRootNode();
	if (NULL == pstRoot)
	{
		szError.AppendFormat(_T("Warnning: 文件<%s>中没有指定资源转换列表信息\r\n"));
		fpFile.Close();
		return true;
	}

	fpFile.WriteString(_T("\n\n"));
	szLine.Format(_T("  <%s %s=\"%s\"	>\n"), CONV_TREE_XML_TAG_CONVTREE, CONV_TREE_XML_TAG_RESNAME, pstRoot->GetNodeData()->GetNodeName());
	fpFile.WriteString(szLine);

	CRESTREENODE *pstChild = m_stResTree.GetNextChild(pstRoot, NULL);
	for (; NULL != pstChild; )
	{
		ResNode2Xml(fpFile, "    ", pstChild);
		pstChild = m_stResTree.GetNextChild(pstRoot, pstChild);
	}

	szLine.Format(_T("  </%s>\n"), CONV_TREE_XML_TAG_CONVTREE);
	fpFile.WriteString(szLine);

	szLine.Format(_T("</%s>\n"), CONV_TREE_XML_TAG_ROOT);
	fpFile.WriteString(szLine);

	fpFile.Close();

	return true;
}

bool ConvTreeParser::ResNode2Xml(CStdioFile & stFile, const CString &szSpace, CRESTREENODE * pstNode)
{
	CString szLine;
	CString szTmp;
	CRESTREENODE *pstChild = NULL;
	CCommNode *pstData;
	CResNode *pstResNode;


	if (NULL == pstNode)
	{
		return true;
	}

	pstChild = m_stResTree.GetNextChild(pstNode, NULL);

	bool bHaveChild = false;
	if (NULL != pstChild)
	{
		bHaveChild = true;
	}

	/*本节点基本信息*/
	pstData = pstNode->GetNodeData();
	switch(pstData->GetNodeType())
	{
	case RES_TREE_NODE_TYPE_COMM:
		{
			szLine.Format(_T("%s<%s %s=\"%s\" %s>\n"), szSpace, CONV_TREE_XML_TAG_COMMNODE, CONV_TREE_XML_TAG_RESNAME, pstData->GetNodeName(), bHaveChild?"":"/");
			stFile.WriteString(szLine);
			break;
		}			
	case RES_TREE_NODE_TYPE_RES:
		{
			pstResNode = (CResNode*)pstData;
			
			szLine.Format(_T("%s<%s %s=\"%s\" %s=\"%s\" %s=\"%s\" "), szSpace, CONV_TREE_XML_TAG_RESNODE, 
				CONV_TREE_XML_TAG_RESNAME, pstResNode->GetNodeName(), 
				CONV_TREE_XML_TAG_META, pstResNode->GetMetaName(),				
				CONV_TREE_XML_TAG_BINFILE, pstResNode->GetBinFile());
			stFile.WriteString(szLine);	

			ENTRYMAPFILELIST &EntryFiles = pstResNode->GetEntryMapFiles();
			if ((int)EntryFiles.size() > 0)
			{
				szLine.Format(_T("%s=\"%s\" "), CONV_TREE_XML_TAG_ENTRYMAPFILE, EntryFiles[0]);
				stFile.WriteString(szLine);	
			}			

			szLine.Format(_T("%s=\"%s\" "), CONV_TREE_XML_TAG_BINSTYLES, GenBinStyleStr(pstResNode->GetResIDList()));
			stFile.WriteString(szLine);

			switch(pstResNode->GetSortMethod())
			{
			case SORT_METHOD_ASC:
				szLine.Format(_T("%s=\"Asc\" "), CONV_TREE_XML_TAG_SORTMETHOD);
				stFile.WriteString(szLine);
				break;
			case SORT_METHOD_DESC:
				szLine.Format(_T("%s=\"Desc\" "), CONV_TREE_XML_TAG_SORTMETHOD);
				stFile.WriteString(szLine);
				break;
			default:				
				break;
			}

			szLine = "";
			EXCELFILELIST &stExcels = pstResNode->GetExcelFiles();
			if (0 < (int)stExcels.size())
			{
				szLine.Format(_T("%s=\"%s"), CONV_TREE_XML_TAG_EXCELFILE, stExcels[0]);
				for (int i = 1; i < (int)stExcels.size(); i++)
				{
					szTmp.Format(_T(" %s"), stExcels[i]);
					szLine += szTmp;
				}
				szLine +="\" ";
				stFile.WriteString(szLine);
			}	
			
			/*include / exclude sheet*/
			INCLUDESHEETLIST &stIncludeSheets = pstResNode->GetIncludeSheetList();
			if (0 < (int)stIncludeSheets.size())
			{
				szLine = "";
				if (0 < (int)stIncludeSheets.size())
				{
					szLine.Format(_T("%s=\"%s"), CONV_TREE_XML_TAG_INCLUDESHEET, stIncludeSheets[0]);
					for (int i = 1; i < (int)stIncludeSheets.size(); i++)
					{
						szTmp.Format(_T(" %s"), stIncludeSheets[i]);
						szLine += szTmp;
					}
					szLine +="\" ";
					stFile.WriteString(szLine);
				}
			}else
			{
				szLine = "";
				EXCLUDESHEETLIST &stExcludeSheets = pstResNode->GetExcludeSheetList();
				if (0 < (int)stExcludeSheets.size())
				{
					szLine.Format(_T("%s=\"%s"), CONV_TREE_XML_TAG_EXCLUDESHEET, stExcludeSheets[0]);
					for (int i = 1; i < (int)stExcludeSheets.size(); i++)
					{
						szTmp.Format(_T(" %s"), stExcludeSheets[i]);
						szLine += szTmp;
					}
					szLine +="\" ";
					stFile.WriteString(szLine);
				}
			}
			

			/*keyword*/
			if ("" != pstResNode->GetKeywordFile())
			{
				szLine.Format(_T("%s=\"%s\" "), CONV_TREE_XML_TAG_KEYWORDFILE, pstResNode->GetKeywordFile());
				stFile.WriteString(szLine);
			}

	
			/*mutil table*/
			if (pstResNode->IsMutilTables())
			{
				szLine.Format(_T("%s=\"%s\" "), CONV_TREE_XML_TAG_ISMUTILTABLES, CONV_TREE_XML_TAG_TRUE);
				stFile.WriteString(szLine);

				szLine.Format(_T("%s=\"%s\" "), CONV_TREE_XML_TAG_SUBTABLENAME, pstResNode->GetSubTableName());
				stFile.WriteString(szLine);
		

				szLine.Format(_T("%s=\"%d\" "), CONV_TREE_XML_TAG_SUBTABLEIDCOL, pstResNode->GetSubTableIDCol());
				stFile.WriteString(szLine);

				szLine.Format(_T("%s=\"%s\" "), CONV_TREE_XML_TAG_SUBTABLEIDNAME, pstResNode->GetSubTableIDName());
				stFile.WriteString(szLine);
			}/*if (pstResNode->IsMutilTables())*/
			
			if ("" != pstResNode->GetRecordSetName())
			{
				szLine.Format(_T("%s=\"%s\" "), CONV_TREE_XML_TAG_RECORDSETNAME, pstResNode->GetRecordSetName());
				stFile.WriteString(szLine);
			}
			if ("" != pstResNode->GetRecordCountName())
			{
				szLine.Format(_T("%s=\"%s\" "), CONV_TREE_XML_TAG_RECORDCOUNTNAME, pstResNode->GetRecordCountName());
				stFile.WriteString(szLine);
			}
			
			if (! pstResNode->IsMapEntryByName())
			{
				szLine.Format(_T("%s=\"%s\" "), CONV_TREE_XML_TAG_MAPBYENTRYNAME, CONV_TREE_XML_TAG_FALSE);
				stFile.WriteString(szLine);
			}
	
			szLine.Format(_T(" %s>\n"), bHaveChild?"":"/");
			stFile.WriteString(szLine);
			break;
		}	
	default:
		break;
	}/*switch(pstData->GetNodeType())*/

	if (bHaveChild)
	{
		const CString szTmp = szSpace + "  ";
		for (; NULL != pstChild;)
		{
			ResNode2Xml(stFile, szTmp, pstChild);
			pstChild = m_stResTree.GetNextChild(pstNode, pstChild);
		}

		switch(pstData->GetNodeType())
		{
		case RES_TREE_NODE_TYPE_COMM:
			{
				szLine.Format(_T("%s</%s>\n"), szSpace, CONV_TREE_XML_TAG_COMMNODE);
				stFile.WriteString(szLine);
				break;
			}			
		case RES_TREE_NODE_TYPE_RES:
			{
				szLine.Format(_T("%s</%s>\n"), szSpace,CONV_TREE_XML_TAG_RESNODE);
				stFile.WriteString(szLine);
				break;
			}	
		default:
			break;
		}/*switch(pstData->GetNodeType())*/
	}/*if (bHaveChild)*/

	return true;
}




CString & ConvTreeParser::GenBinStyleStr(RESIDLIST &aiIds)
{
	//TODO: insert return statement here
	static CString szMethod;
	int iIDCount;

	iIDCount = (int)aiIds.size();
	if (0 >= iIDCount)
	{
		szMethod = "";
	}else
	{
		szMethod.Format(_T("%d"), aiIds[0]);
		for (int i= 1; i < iIDCount; i++)
		{
			CString szTmp;
			szTmp.Format(_T(",%d"), aiIds[1]);
			szMethod += szTmp;
		}
	}	

	return szMethod;
}


