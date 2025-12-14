#include "StdAfx.h"
#include "ConvTreeXmlParser.h"
#include "../comm/error_report.h"
#include "../comm/scew_if.h"
#include "ConvDefine.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

ConvTreeXmlParser::ConvTreeXmlParser(void)
{
}

ConvTreeXmlParser::~ConvTreeXmlParser(void)
{
}

bool ConvTreeXmlParser::ParseFile(CString &szFile, CString &szError)
{
	bool bSucc = true;
	scew_tree *pstTree;
	scew_element *pstRoot;
	scew_attribute *pstAttr;

	ClearAllInfo();

	if (0 != CreateXmlTreeByFileName(pstTree, szFile, szError))
	{
		return false;
	}

	pstRoot = scew_tree_root(pstTree);
	if (NULL == pstRoot)
	{
		szError.AppendFormat(_T("xml文件<%s>中没有定义任何元素\r\n"), szFile);
		bSucc = false;
	}
	if (0 != strcmp(scew_element_name(pstRoot), CONV_TREE_XML_TAG_ROOT))
	{
		WLogInfo(LOG_LEVEL_ERROR, "转换列表文件<%s>的根据节点名必须是<%s>", szFile, CONV_TREE_XML_TAG_ROOT);
		bSucc = false;
	}
	pstAttr = scew_attribute_by_name(pstRoot, CONV_TREE_XML_TAG_ISNEWCONFIGFORMAT);
	if ((NULL != pstAttr) && (0 == strcmp(scew_attribute_value(pstAttr), CONV_TREE_XML_TAG_FALSE)))
	{
		m_bIsNewFormat = false;
	}

	if (bSucc)
	{
		bSucc = ParserBaseConfInfo(*pstRoot, szError);
	}	

	if (bSucc)
	{
		bSucc = ParseResStyles(pstRoot, szError);
	}

	/*conv tree*/
	if (bSucc)
	{
		bSucc = ParseConvTree(pstRoot,szError);
	}

	scew_tree_free(pstTree);
	return bSucc;
}



bool ConvTreeXmlParser::ParserBaseConfInfo(scew_element & stRoot, CString & szError)
{
	scew_element* pstSubElement = NULL;
	scew_attribute *pstAttr;
	bool bSucc = true;

	pstSubElement = scew_element_by_name(&stRoot, CONV_TREE_XML_TAG_METALIBFILE);
	if (NULL == pstSubElement)
	{
		szError.AppendFormat(_T("xml文件中必须包含%s属性，以指定元数据描述库文件\r\n"), CONV_TREE_XML_TAG_METALIBFILE);
		return false;
	}
	pstAttr = scew_attribute_by_name(pstSubElement, CONV_TREE_XML_TAG_FILENAME);
	if (NULL == pstAttr)
	{
		szError.AppendFormat(_T("xml文件中必须包含%s属性，以指定元数据描述库文件\r\n"), CONV_TREE_XML_TAG_METALIBFILE);
		return false;
	}else
	{
		SetMetalibFile(scew_attribute_value(pstAttr));		
	}

	/*EntryMapPath*/
	pstSubElement = scew_element_by_name(&stRoot, CONV_TREE_XML_TAG_EntryMapPATH);
	if (NULL != pstSubElement)
	{
		pstAttr = scew_attribute_by_name(pstSubElement, CONV_TREE_XML_TAG_PATH);
		if (NULL != pstAttr)
		{		
			SetEntryMapFilesPath(scew_attribute_value(pstAttr));			
		}
	}

	/*BinFilePath*/
	pstSubElement = scew_element_by_name(&stRoot, CONV_TREE_XML_TAG_BINFILEPATH);
	if (NULL != pstSubElement)
	{
		pstAttr = scew_attribute_by_name(pstSubElement, CONV_TREE_XML_TAG_PATH);
		if (NULL != pstAttr)
		{		
			SetBinFilesPath(scew_attribute_value(pstAttr));		
		}
	}/*if (0 == _stricmp( scew_element_name(pstSubElement), CONV_TREE_XML_TAG_METALIBFILE))*/ 


	/*ExcelFilePath*/
	pstSubElement = scew_element_by_name(&stRoot, CONV_TREE_XML_TAG_EXCELFILEPATH);
	if (NULL != pstSubElement)
	{
		pstAttr = scew_attribute_by_name(pstSubElement, CONV_TREE_XML_TAG_PATH);
		if (NULL != pstAttr)
		{		
			SetExcelFilesPath(scew_attribute_value(pstAttr));				
		}
	}/*if (0 == _stricmp( scew_element_name(pstSubElement), CONV_TREE_XML_TAG_METALIBFILE))*/ 		

	/*ExcelFilePath*/
	pstSubElement = scew_element_by_name(&stRoot, CONV_TREE_XML_TAG_KEYWORDFILE);
	if (NULL != pstSubElement)
	{
		pstAttr = scew_attribute_by_name(pstSubElement, CONV_TREE_XML_TAG_PATH);
		if (NULL != pstAttr)
		{		
			m_szKeywordFile = scew_attribute_value(pstAttr);				
		}
	}

	return bSucc;
}

bool ConvTreeXmlParser::ParseResStyles(scew_element *pstRoot, CString & szError)
{
	scew_element* pstResStyleList = NULL;
	scew_attribute *pstAttr;
	scew_element** pstResStyle = NULL;
	bool bSucc = true;
	unsigned int dwCount = 0;
	unsigned int i;

	assert(NULL != pstRoot);

	pstResStyleList = scew_element_by_name(pstRoot, CONV_TREE_XML_TAG_RESSTYLELIST);
	if (NULL == pstResStyleList)
	{
		return true;
	}

	pstResStyle = scew_element_list(pstResStyleList, CONV_TREE_XML_TAG_RESSTYLE, &dwCount);
	for (i = 0; i < dwCount; i++)
	{
		CResStyle stResStyle;

		pstAttr = scew_attribute_by_name(pstResStyle[i], CONV_TREE_XML_TAG_RESNAME);
		if (NULL == pstAttr)
		{		
			continue;
		}
		stResStyle.m_szResStyleName = scew_attribute_value(pstAttr);
		
		pstAttr = scew_attribute_by_name(pstResStyle[i], CONV_TREE_XML_TAG_RESID);
		if (NULL == pstAttr)
		{		
			continue;
		}
		stResStyle.m_iID = atoi(scew_attribute_value(pstAttr));

		if (0 != m_stResStyleList.InsertResStyle(stResStyle, szError))
		{
			bSucc = false;
			break;
		}	
	}/*for (i = 0; i < dwCount; i++)*/
	
	scew_element_list_free(pstResStyle);

	return bSucc;
}

bool ConvTreeXmlParser::ParseConvTree(scew_element *pstRoot, CString & szError)
{
	scew_element* pstConvTree = NULL;
	bool bSucc = true;

	assert(NULL != pstRoot);

	pstConvTree = scew_element_by_name(pstRoot, CONV_TREE_XML_TAG_CONVTREE);
	if (NULL == pstConvTree)
	{
		szError.AppendFormat("在转换列表xml配置文件中没有包含任何需要转换的excel文件，请通过<%s>节点指定\r\n");
		return false;
	}

	
	CRESTREENODE *pstResRoot;
	CCommNode *pstNode;

	pstResRoot = new CRESTREENODE;
	pstNode = new CCommNode(GetXmlNodeNameAttr(pstConvTree)); 
	pstResRoot->SetNodeData(pstNode);
	m_stResTree.SetRootNode(pstResRoot);

	bSucc = ParseConvTreeNode(pstConvTree, pstResRoot, szError);

	return bSucc;
}

bool ConvTreeXmlParser::ParseConvTreeNode(scew_element *pstRoot, CRESTREENODE *pstResRoot, CString & szError)
{
	bool bSucc = true;
	scew_element* pstNode = NULL;
	scew_attribute *pstAttr;
	const char *pszName = NULL;
	CRESTREENODE *pstResChild = NULL;

	assert(NULL != pstRoot);
	assert(NULL != pstResRoot);

	
	pstNode = scew_element_next(pstRoot, NULL);
	while (NULL != pstNode)
	{
		/*CommNode*/
		if (0 == _stricmp( scew_element_name(pstNode), CONV_TREE_XML_TAG_COMMNODE))
		{
			pszName = GetXmlNodeNameAttr(pstNode);
			pstResChild = InsertCommNode(pstResRoot, CString(pszName));
			bSucc = ParseConvTreeNode(pstNode, pstResChild, szError);
			if (!bSucc)
			{
				break;
			}else if (m_stResTree.GetChildNum(pstResChild) <= 0)
			{
				szError.AppendFormat(_T("Warnning！：%s元素<Name=%s>下没有包含任何转换信息节点\r\n"),
					CONV_TREE_XML_TAG_COMMNODE, pszName);
			}			
		}/*if (0 == _stricmp( scew_element_name(pstNode), CONV_TREE_XML_TAG_COMMNODE))*/

		/*ResNode*/
		if (0 == _stricmp( scew_element_name(pstNode), CONV_TREE_XML_TAG_RESNODE))
		{
			CResNode *pstResNode;

			pszName = GetXmlNodeNameAttr(pstNode);
			pstResNode = new CResNode(CString(pszName));
			pstResChild = new CRESTREENODE(pstResNode);
			m_stResTree.AddChild(pstResRoot, pstResChild, false);
			
			/*Meta*/
			pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_META);
			if (NULL == pstAttr)
			{		
				szError.AppendFormat(_T("Error: %s元素<Name=%s>必须包含%s属性指定保存资源信息的结构体名称\r\n"),
					CONV_TREE_XML_TAG_RESNODE, pszName,CONV_TREE_XML_TAG_META);
				bSucc = false;
				break;
			}else
			{
				pstResNode->SetMetaName(CString(scew_attribute_value(pstAttr)));
			}

			/*EntryMap File*/
			pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_ENTRYMAPFILE);
			if ((NULL == pstAttr))
			{		
				if (!m_bIsNewFormat)
				{
					szError.AppendFormat(_T("Error: %s元素<Name=%s>必须包含%s属性以指定转化所需的字段影射配置文件\r\n"),
						CONV_TREE_XML_TAG_RESNODE, pszName,CONV_TREE_XML_TAG_ENTRYMAPFILE);
					bSucc = false;
					break;
				}				
			}else
			{
				pstResNode->SetEntryMapFile(CString(scew_attribute_value(pstAttr)));
			}

			/*bin File*/
			pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_BINFILE);
			if (NULL == pstAttr)
			{		
				szError.AppendFormat(_T("Error: %s元素<Name=%s>必须包含%s属性指定保存二进制资源文件的文件名\r\n"),
					CONV_TREE_XML_TAG_RESNODE, pszName,CONV_TREE_XML_TAG_BINFILE);
				bSucc = false;
				break;
			}else
			{
				pstResNode->SetBinFile(CString(scew_attribute_value(pstAttr)));
			}

			/*sort method*/
			pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_SORTMETHOD);
			if (NULL != pstAttr)
			{		
				pstResNode->SetSortMethod(scew_attribute_value(pstAttr));	
			}
			
			/*res style*/
			pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_BINSTYLES);
			if (NULL != pstAttr)
			{		
				ParseResStyleList(pstResNode,CString(scew_attribute_value(pstAttr)),szError);	
			}

			/*excel files*/
			bSucc = ParseNodeExcelFiles(pstNode, pstResNode, szError);
			if (!bSucc)
			{
				break;
			}

			pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_INCLUDESHEET);
			if (NULL == pstAttr)
			{		
				/*如果没有制定"include sheet属性"则使用exclude sheet属性*/
				bSucc = ParseNodeExcludeSheets(pstNode, pstResNode, szError);
				if (!bSucc)
				{
					break;
				}
			}else
			{
				char *pszFile = strtok((char *)scew_attribute_value(pstAttr), "\n\t ");
				while (NULL != pszFile)
				{
					pstResNode->AddIncludeSheet(CString(pszFile));
					pszFile = strtok(NULL, "\n\t ");
				}
			}			

			/*keyword files*/
			pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_KEYWORDFILE);
			if (NULL != pstAttr)
			{		
				pstResNode->SetKeywordFile(CString(scew_attribute_value(pstAttr)));	
			}
			

			bSucc = ParseMutilTableAttribute(pstNode, pstResNode, szError);
			if (!bSucc)
			{
				break;
			}
				
			/*Is MutilTables*/
			pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_MAPBYENTRYNAME);
			if (NULL != pstAttr)
			{		
				if (0 == _stricmp(CONV_TREE_XML_TAG_FALSE, scew_attribute_value(pstAttr)))
				{
					pstResNode->SetIsMapEntryByName(false);	
				}					
			}			


			/*RECRDSETNAME*/
			pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_RECORDSETNAME);
			if (NULL != pstAttr)
			{			
				pstResNode->SetRecordSetName(CString(scew_attribute_value(pstAttr)));
			}

			pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_RECORDCOUNTNAME);
			if (NULL != pstAttr)
			{			
				pstResNode->SetRecordCountName(CString(scew_attribute_value(pstAttr)));
			}

			if (0 < scew_element_count(pstNode))
			{
				szError.AppendFormat(_T("Warnning: 按约定%s元素<Name=%s>为叶子节点元素，其下的子元素将被忽略\n"),
					CONV_TREE_XML_TAG_RESNODE, pszName);
			}
		}/*if (0 == _stricmp( scew_element_name(pstNode), CONV_TREE_XML_TAG_RESNODE))*/

		pstNode = scew_element_next(pstRoot, pstNode);
	}/*while (NULL != pstNode)*/

	return bSucc;
}

inline bool ConvTreeXmlParser::ParseMutilTableAttribute(scew_element *pstNode, CResNode *pstResNode, CString & szError)
{
	scew_attribute *pstAttr;

	assert(NULL != pstNode);
	assert(NULL != pstResNode);

	/*Is MutilTables*/
	pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_ISMUTILTABLES);
	if (NULL == pstAttr)
	{		
		return true;		
	}
	if (0 != _stricmp(CONV_TREE_XML_TAG_TRUE, scew_attribute_value(pstAttr)))
	{
		return true;
	}
	pstResNode->SetIsMutilTables(true);


	pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_SUBTABLENAME);
	if (NULL == pstAttr)
	{		
		szError.AppendFormat(_T("Error: %s元素<Name=%s>为多表转换节点,必须包含%s属性指定孙结构体的元数据描述名\r\n"),
			CONV_TREE_XML_TAG_RESNODE, pstResNode->GetNodeName(),CONV_TREE_XML_TAG_SUBTABLENAME);
		return false;
	}
	pstResNode->SetSubTableName(CString(scew_attribute_value(pstAttr)));

	

	pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_SUBTABLEIDCOL);
	if (NULL == pstAttr)
	{		
		szError.AppendFormat(_T("Error: %s元素<Name=%s>为多表转换节点,必须包含%s属性指定孙结构体的ID值所在的Excel单元列\r\n"),
			CONV_TREE_XML_TAG_RESNODE, pstResNode->GetNodeName(),CONV_TREE_XML_TAG_SUBTABLEIDCOL);
		return false;
	}
	pstResNode->SetSubTableIDCol(atoi(scew_attribute_value(pstAttr)));
	if (0 >= pstResNode->GetSubTableIDCol())
	{
		szError.AppendFormat(_T("Error: %s元素<Name=%s>为多表转换节点,必须包含%s属性值<%d>无效\r\n"),
			CONV_TREE_XML_TAG_RESNODE, pstResNode->GetNodeName(),CONV_TREE_XML_TAG_SUBTABLEIDCOL, pstResNode->GetSubTableIDCol());
		return false;
	}

	pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_SUBTABLEIDNAME);
	if (NULL == pstAttr)
	{		
		szError.AppendFormat(_T("Error: %s元素<Name=%s>为多表转换节点,必须包含%s属性指定保存孙结构体ID值的成员的元数据描述名\r\n"),
			CONV_TREE_XML_TAG_RESNODE, pstResNode->GetNodeName(),CONV_TREE_XML_TAG_SUBTABLEIDNAME);
		return false;
	}
	pstResNode->SetSubTableIDName(CString(scew_attribute_value(pstAttr)));

	return true;
}

inline const char *ConvTreeXmlParser::GetXmlNodeNameAttr(scew_element *pstNode)
{
	const char *pszName = NULL;
	scew_attribute *pstAttr;

	assert(NULL != pstNode);

	pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_RESNAME);
	if (NULL == pstAttr)
	{		
		pszName = "";
	}else
	{
		pszName = scew_attribute_value(pstAttr);
	}

	return pszName;
}

inline bool ConvTreeXmlParser::ParseNodeExcelFiles(scew_element *pstNode, CResNode *pstResData, CString &szError)
{
	scew_attribute *pstAttr;
	char *pszFile;

	assert(NULL != pstNode);
	assert(NULL != pstResData);

	pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_EXCELFILE);
	if (NULL == pstAttr)
	{		
		szError.AppendFormat(_T("Error: %s元素<Name=%s>至少要包含一个%s属性，以指定转换所需的Excel文件\r\n"),
			CONV_TREE_XML_TAG_RESNODE ,pstResData->GetNodeName(), CONV_TREE_XML_TAG_EXCELFILE);
		return false;
	}

	pszFile = strtok((char *)scew_attribute_value(pstAttr), "\n\t ");
	while (NULL != pszFile)
	{
		pstResData->AddExcelFile(CString(pszFile));
		pszFile = strtok(NULL, "\n\t ");
	}
	
	return true;
}

void ConvTreeXmlParser::ClearAllInfo()
{
	m_szBinFilesPath = _T(".\\");
	m_szEntryMapFilesPath = _T(".\\");
	m_szExcelFilesPath = _T(".\\");
	m_szMetalibFile = _T("");
	m_bIsNewFormat = true;
	m_stResStyleList.GetResStyleList().clear();

	DeleteResTree();
}

inline bool ConvTreeXmlParser::ParseNodeExcludeSheets(scew_element *pstNode, CResNode *pstResData, CString &szError)
{
	scew_attribute *pstAttr;
	char *pszFile;

	assert(NULL != pstNode);
	assert(NULL != pstResData);

	pstAttr = scew_attribute_by_name(pstNode, CONV_TREE_XML_TAG_EXCLUDESHEET);
	if (NULL == pstAttr)
	{		
		return true;
	}

	pszFile = strtok((char *)scew_attribute_value(pstAttr), "\n\t ");
	while (NULL != pszFile)
	{
		pstResData->AddExcludeSheet(CString(pszFile));
		pszFile = strtok(NULL, "\n\t ");
	}

	return true;
}


