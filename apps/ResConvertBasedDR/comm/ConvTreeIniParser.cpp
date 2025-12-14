#include "StdAfx.h"
#include "ConvTreeIniParser.h"
#include "ConvResTree.h"
#include "ConvBase.h"

ConvTreeIniParser::ConvTreeIniParser(void)
{
}

ConvTreeIniParser::~ConvTreeIniParser(void)
{
}

bool ConvTreeIniParser::ParseFile(CString &szFile, CString &szError)
{
	CStdioFile fpFile;
	CFileFind fpFind;
	bool bSucc = true;
	int iPos;

	if( !fpFind.FindFile(szFile) )
	{
		szError.Format(_T("文件 %s 不存在！"), szFile);
		return false;
	}
	if( !fpFile.Open(szFile, CFile::modeRead) )
	{
		szError.Format(_T("文件 %s 无法打开，请重新选择文件！"), szFile);
		return false;
	}

	/*设置配置文件格式*/
	m_bIsNewFormat = false;

	CString szFilePath;
	/*取出文件的路径*/
	szFilePath = szFile;
	szFilePath.Replace('/', '\\');
	iPos = szFilePath.ReverseFind('\\');
	if (0 <= iPos)
	{
		szFilePath = szFilePath.Left(iPos + 1);
	}else
	{
		szFilePath = ".\\";
	}

	/*构建一个根节点，此根节点名字为空*/
	CRESTREENODE *pstRoot;
	
	pstRoot = new CRESTREENODE;
	pstRoot->SetNodeData(new CCommNode);
	m_stResTree.SetRootNode(pstRoot);

	/*解析ini文件*/
	CString strTmp, strLine;
	CRESTREENODE *pstParent = NULL, *pstChild = NULL;
	CString sRoot;
	CString sPre, sNext;
	CString sChildPre, sChildNext;
	CString sParam[MAX_PARAM_NUM];


	while( fpFile.ReadString(strLine) )
	{
		strLine.Replace("：", ":");
		strLine.Replace("，", ",");
		strLine.TrimLeft();
		strLine.TrimRight();
		if(strLine.Left(1)=="#" && strLine.Find(" ")>0 && strLine.Find(" ")<strLine.GetLength()-1)
		{
			continue; /*注释不处理*/
		}else if( strLine.Left(1)=="#" && strLine.Find(" ")<0 && strLine.Find(":")>0 )
		{
			/*"##aaa:bbbb  #####" 中间节点被":"分成两部分，前一部分为父节点*/
			iPos = strspn(strLine, "#");
			strLine = strLine.Mid(iPos);
			iPos = strLine.Find("#");
			strLine = strLine.Left(iPos);

			/*strLine：	aaa:bbbb*/
			iPos = strLine.Find(":");
			sRoot = strLine.Left(iPos);	
			pstParent = InsertCommNode(pstRoot, sRoot);
			

			/*strLine:	bbbb*/
			strLine = strLine.Mid(iPos+1);
			pstParent = InsertCommNode(pstParent, strLine);
			
		}else if( strLine.Left(1)!="#" )
		{
			char szToken[MAX_LENGTH_LINE];			
			char* szBuf;
			int  i;
			CResNode *pstData;

			if (NULL == pstParent)
			{
				continue;
			}
			memset(szToken, 0, sizeof(szToken));
			szBuf = (LPSTR)(LPCTSTR)strLine;
			memcpy(szToken, szBuf, strlen(szBuf)+1);
			for( i=0; i<MAX_PARAM_NUM; i++)
				sParam[i] = "";

			/*取出个配置项*/
			strTmp = strtok(szToken, "\n\t ");
			i = 0;
			while( strTmp!="" )
			{
				sParam[i] = strTmp;
				strTmp = strtok(NULL, "\n\t ");
				i++;
			}
			if (9 > i)
			{
				continue;
			}

			/*记录各配置项*/
			pstData = new CResNode(sParam[1]);
			pstChild = new CRESTREENODE(pstData);
			m_stResTree.AddChild(pstParent, pstChild, false);

			/*param[2] res stylelist*/
			ParseResStyleList(pstData, sParam[2], szError);

			/*sort method*/
			ParseSortMethod(pstData, sParam[4]);

			/*ini file*/
			bSucc = ParseEntryMap(szFilePath, pstData, sParam[5], szError);
	
			/*bin Files*/
			ParseBinFile(pstData, sParam[7]);

			/*excel files*/
			ParseExcelFile(*pstData, sParam);			
		}/*if( strLine.Left(1)!="#" )*/
	}/*while( fpFile.ReadString(strLine) )*/
	

	fpFile.Close();

	NomalizeResTree();
	return true;
}




void ConvTreeIniParser::ParseSortMethod(CResNode *pstData, CString &szStyles)
{
	switch(atoi((char   *)(LPCTSTR)szStyles))
	{
	case 1:
		pstData->SetSortMethod(SORT_METHOD_ASC);
		break;
	default:
		pstData->SetSortMethod(SORT_METHOD_NO);
		break;
	}
}

bool ConvTreeIniParser::ParseEntryMap(CString &szPath, CResNode *pstData, CString &szEntryMap, CString &szError)
{
	int iPos;
	bool bSucc = true;
	
	assert(NULL != pstData);
	szEntryMap.Replace("/", "\\");
	iPos = szEntryMap.ReverseFind('\\');
	if (0 < iPos)
	{
		if (m_szEntryMapFilesPath == _T(".\\"))
		{
			m_szEntryMapFilesPath = szEntryMap.Left(iPos);
		}		
		pstData->SetEntryMapFile(szEntryMap.Mid(iPos + 1));
	}else
	{
		pstData->SetEntryMapFile(szEntryMap);
	}	

	CString szEntryMapPath = MakePath(szPath, m_szEntryMapFilesPath) + "\\" + pstData->GetEntryMapFiles()[0];
	CStdioFile fpFile;
	if( !fpFile.Open(szEntryMapPath, CFile::modeRead) )
	{
		szError.AppendFormat(_T("文件 %s 无法打开，请重新选择文件！"), szEntryMapPath);
		return false;
	}

	CString szLine;
	while( fpFile.ReadString(szLine) )
	{
		CString szToken1, szToken2;

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

		if (szLine.Left(1) != INI_FILE_VAR_FLAG)
		{
			continue;
		}

		iPos = 0;
		szToken1 = szLine.Tokenize("\n\t ", iPos);
		if (0 == szToken1.CompareNoCase(INI_FILE_EXCLUDE_SHEET))
		{
			szToken2 = szLine.Tokenize("\n\t ", iPos);
			while ("" != szToken2)
			{
				pstData->AddExcludeSheet(szToken2);
				szToken2 = szLine.Tokenize("\n\t ", iPos);
			}
		}/*if (szToken1 == INI_FILE_EXCLUDE_SHEET)*/

		if (0 == szToken1.CompareNoCase(INI_FILE_KEYWORDS_FILE))
		{
			szToken2 = szLine.Tokenize("\n\t ", iPos);
			if ("" != szToken2)
			{
				szToken2.Replace("/", "\\");
				iPos = szToken2.ReverseFind('\\');
				if (m_szKeywordFile == _T(""))
				{
					m_szKeywordFile = szToken2;
				}
				if (m_szKeywordFile.Compare((LPCTSTR)szToken2) != 0)
				{
					pstData->SetKeywordFile(szToken2);
				}				
			}
		}/*if (0 == szToken1.CompareNoCase(INI_FILE_KEYWORDS_FILE))*/	

		if (0 == szToken1.CompareNoCase(INI_FILE_META_NAME))
		{
			szToken2 = szLine.Tokenize("\n\t ", iPos);
			if ("" == szToken2)
			{
				szError.AppendFormat("请在ini配置文件<%s>中通过<%s>配置项指定保存资源信息的结构体名称\r\n", szEntryMapPath, INI_FILE_META_NAME);
				bSucc = false;
				break;
			}
			pstData->SetMetaName(szToken2);
		}/*if (0 == szToken1.CompareNoCase(INI_FILE_META_NAME))*/

		if (0 == szToken1.CompareNoCase(INI_FILE_IS_MUTILTABLE))
		{
			szToken2 = szLine.Tokenize("\n\t ", iPos);
			if ("1" == szToken2)
			{
				pstData->SetIsMutilTables(true);
			}
		}

		if (0 == szToken1.CompareNoCase(INI_FILE_SUBTABLENAME))
		{
			szToken2 = szLine.Tokenize("\n\t ", iPos);
			if ("" != szToken2)
			{
				pstData->SetSubTableName(szToken2);
			}
		}

		if (0 == szToken1.CompareNoCase(INI_FILE_SUBTABLE_IDNAME))
		{
			szToken2 = szLine.Tokenize("\n\t ", iPos);
			if ("" != szToken2)
			{
				switch(szToken2[0])
				{
				case INI_GET_EXCEL_VALUE_FLAG_NUMBER:				
				case INI_GET_EXCEL_VALUE_FLAG_PERSENT:					
				case INI_GET_EXCEL_VALUE_FLAG_ALLINFO:					
				case INI_GET_EXCEL_VALUE_FLAG_INTELLIGENT:					
					szToken2 = szToken2.Mid(1);
					break;
				default:				
					break;
				}
				pstData->SetSubTableIDName(szToken2);
			}
		}

		if (0 == szToken1.CompareNoCase(INI_FILE_SUBTABLE_IDCOL))
		{
			szToken2 = szLine.Tokenize("\n\t ", iPos);
			if ("" != szToken2)
			{
				pstData->SetSubTableIDCol(atoi((LPCTSTR)szToken2));
			}
		}

		if (0 == szToken1.CompareNoCase(INI_FILE_RECORDSSETNAME))
		{
			szToken2 = szLine.Tokenize("\n\t ", iPos);
			if ("" != szToken2)
			{
				pstData->SetRecordSetName(szToken2);
			}
		}
		if (0 == szToken1.CompareNoCase(INI_FILE_RECORDCOUNTNAME))
		{
			szToken2 = szLine.Tokenize("\n\t ", iPos);
			if ("" != szToken2)
			{
				pstData->SetRecordCountName(szToken2);
			}
		}

	}/*while( fpFile.ReadString(strLine) )*/

	fpFile.Close();

	return bSucc;
}

void ConvTreeIniParser::ParseBinFile(CResNode *pstData, CString &szBinFile)
{
	int iPos;

	assert(NULL != pstData);

	
	szBinFile.Replace("/", "\\");
	iPos = szBinFile.ReverseFind('\\');
	if (0 < iPos)
	{
		if (m_szBinFilesPath == _T(".\\"))
		{
			m_szBinFilesPath = szBinFile.Left(iPos);
		}		
		pstData->SetBinFile(szBinFile.Mid(iPos + 1));
	}else
	{
		pstData->SetBinFile(szBinFile);
	}	
}

void ConvTreeIniParser::ParseExcelFile(CResNode &stData, CString sParam[])
{
	int iPos;

	int i = 8;
	if (m_szExcelFilesPath == _T(".\\"))
	{
		sParam[8].Replace("/", "\\");
		iPos = sParam[8].ReverseFind('\\');
		if (0 < iPos)
		{
			m_szExcelFilesPath = sParam[8].Left(iPos);
			stData.AddExcelFile(sParam[8].Mid(iPos + 1));
		}else
		{
			stData.AddExcelFile(sParam[8]);
		}	
		i++;
	}

	for (; sParam[i] != _T(""); i++)
	{
		sParam[i].Replace("/", "\\");
		iPos = sParam[i].ReverseFind('\\');
		if (0 < iPos)
		{
			stData.AddExcelFile(sParam[i].Mid(iPos + 1));
		}else
		{
			stData.AddExcelFile(sParam[i]);
		}	
	}
}
