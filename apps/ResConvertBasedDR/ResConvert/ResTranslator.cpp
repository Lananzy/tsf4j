#include "stdafx.h"
#include <algorithm>
#include <limits>
#include "../comm/error_report.h"
#include "ResTranslator.h"
#include "EntryMapParser.h"
#include "IniEntryMapParser.h"
#include "CkeywordParser.h"
#include "IniKeywordParser.h"
#include "XmlKeyordParser.h"
#include "../comm/error_report.h"
#include "../comm/ConvBase.h"
#include "ResBinFile.h"
#include "ResPrint.h"
#include "ResSorter.h"
#include "XmlEntryMapParser.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_metalib_kernel_i.h"



int GetMacroInt(OUT long &iID, IN TDRMETALIB* pstLib, IN const  char *pszValue);
inline CString &ColNumber2ExcelColStr(int iCol);



///////////////////////////////////////////////////////////////////////////////////////////////
CResTranslator::CResTranslator(void)
{
	m_pstEntryMapParser = NULL;
	m_pstMetalib = NULL;
	m_iMetaCount = 0;
	m_pstMeta = NULL;
	m_pstKeywordParser = NULL;
	m_iMetaUnit = 0;
}

CResTranslator::~CResTranslator(void)
{
	if (NULL != m_pstEntryMapParser)
	{
		delete m_pstEntryMapParser;
	}
	if (NULL != m_pstKeywordParser)
	{
		delete m_pstKeywordParser;
	}
}

inline int CResTranslator::CreateMetalib(IN ConvTreeParser *pstConvTree, IN CResNode *pstResNode)
{
	int iRet = 0;

	assert(NULL != pstResNode);
	assert(NULL != pstConvTree);

	ENTRYMAPFILELIST &stList = pstResNode->GetEntryMapFiles();
	if ((pstConvTree->IsNewFormat()) && (((int)stList.size() > 0)))
	{
		int i;
		const char **paszFile = new const char *[stList.size()];
		CString *paszPathFile = new CString[stList.size()];
		CString szPath = MakePath(GetRootPath() , pstConvTree->GetEntryMapFilesPath()) ;
		for (i =0; i < (int)stList.size(); i++)
		{
			paszPathFile[i] = szPath + "\\" + stList[i];
			paszFile[i] = (LPCTSTR)paszPathFile[i];
		}
		iRet = tdr_create_lib_multifile(&m_pstMetalib, paszFile, i, TDR_XML_TAGSET_VERSION_1, stderr);	
		if (TDR_ERR_IS_ERROR(iRet))
		{
			WLogInfo(LOG_LEVEL_ERROR, "通过文件<%s>加载元数据文件失败: %s", paszFile[0], tdr_error_string(iRet));
		}
		delete []paszFile;
		delete []paszPathFile;
	}else
	{
		CString szMetalib = MakePath(GetRootPath(), pstConvTree->GetMetalibFile());
		iRet = tdr_load_metalib(&m_pstMetalib, (LPCTSTR)szMetalib);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			WLogInfo(LOG_LEVEL_ERROR, "通过文件<%s>加载元数据文件失败: %s", szMetalib, tdr_error_string(iRet));
		}
	}

	return iRet;
}

int CResTranslator::CreateEntryMapParser(IN ConvTreeParser *pstConvTree, IN CResNode *pstResNode)
{
	int iRet = 0;
	CString szPath;

	assert(NULL != pstResNode);
	assert(NULL != pstConvTree);
	
	iRet = CreateMetalib(pstConvTree, pstResNode);
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		m_pstMeta = tdr_get_meta_by_name(m_pstMetalib, (LPCTSTR)pstResNode->GetMetaName());
		if (NULL == m_pstMeta)
		{
			WLogInfo(LOG_LEVEL_ERROR, "根据资源元数据名<%s>在元数据库<%s>中找不到其定义", pstResNode->GetMetaName(),
				tdr_get_metalib_name(m_pstMetalib));
			iRet = TDR_ERR_ERROR;
		}
		if (TDR_META_IS_VARIABLE(m_pstMeta))
		{
			WLogInfo(LOG_LEVEL_ERROR, "资源结构体<%s>的存储空间不固定,目前资源文件不支持", pstResNode->GetMetaName());
			iRet = TDR_ERR_ERROR;
		}
	}	
	
	if (!TDR_ERR_IS_ERROR(iRet))
	{		
		szPath = MakePath(GetRootPath(), pstConvTree->GetEntryMapFilesPath());
		if (pstConvTree->IsNewFormat())
		{
			m_pstEntryMapParser = new CXmlEntryMapParser();
		}else
		{	
			if (pstResNode->IsMapEntryByName())
			{
				m_pstEntryMapParser = new CNameIniEntryMapParser();	
			}else
			{
				m_pstEntryMapParser = new CTypeIniEntryMapParser();	
			}					
		}/*if (pstConvTree->IsNewFormat())*/

		iRet = m_pstEntryMapParser->ParseEntryMap(m_pstMetalib, m_pstMeta, pstResNode->GetEntryMapFiles(), szPath);
	}	

	return iRet;
}

inline void CResTranslator::Clear()
{
	m_iMetaCount = 0;

	if (NULL != m_pstEntryMapParser)
	{
		delete m_pstEntryMapParser;
		m_pstEntryMapParser = NULL;
	}
	if (NULL != m_pstKeywordParser)
	{
		delete m_pstKeywordParser;
		m_pstKeywordParser = NULL;
	}
	tdr_free_lib(&m_pstMetalib);
}

inline int CResTranslator::CreateKeywordParser(IN ConvTreeParser *pstConvTree, IN CResNode *pstResNode)
{
	int iRet = 0;

	assert(NULL != pstResNode);
	assert(NULL != pstConvTree);

	if (pstConvTree->IsNewFormat())
	{
		m_pstKeywordParser = new CXmlKeyordParser;
	}else
	{
		m_pstKeywordParser = new CIniKeywordParser;
	}

	if (pstResNode->GetKeywordFile().GetLength() > 0)
	{
		iRet = m_pstKeywordParser->Parse(pstResNode->GetKeywordFile());		
	}else if (pstConvTree->GetKeywordFile().GetLength() > 0)
	{
		iRet = m_pstKeywordParser->Parse(pstConvTree->GetKeywordFile());
	}
	
	return iRet;
}

int CResTranslator::Translate(IN ConvTreeParser *pstConvTree, IN CResNode *pstResNode, IN EXCELFILELIST &stSelExcels)
{
	int iRet = 0;

	assert(NULL != pstResNode);	
	
	
	/*ini file parser*/
	iRet = CreateEntryMapParser(pstConvTree, pstResNode);
	if (0 != iRet)
	{
		return iRet;
	}
	
	/*parse keywords file*/
	iRet = CreateKeywordParser(pstConvTree, pstResNode);
	if (0 != iRet)
	{
		return iRet;
	}
	
	
	/*打开bin文件，并预留出资源头部，以便写入数据*/
	CString szBinFile = MakePath(GetRootPath(), pstConvTree->GetBinFilesPath()) + "\\" + pstResNode->GetBinFile();
	if (!m_stBinFile.Open(szBinFile, CFile::modeCreate | CFile::modeWrite  | CFile::shareDenyWrite))
	{
		WLogInfo(LOG_LEVEL_ERROR, "以写的方式打开资源文件%s 失败\r\n", szBinFile);
		return -3;
	}
	
	/*conv excel file*/
	CString szExcelsPath = MakePath(GetRootPath(),pstConvTree->GetExcelFilesPath()) + "\\";
	iRet = ConvExcels2Bin(pstResNode, stSelExcels, szExcelsPath);
	m_stBinFile.Close();

	/*排序*/
	if ((0 == iRet) && (SORT_METHOD_NO != pstResNode->GetSortMethod()))
	{
		CResSorter stSorter;
		iRet = stSorter.Sort(m_pstMetalib, pstResNode->GetMetaName(), szBinFile, pstResNode->GetSortMethod());			
	}
	

	/*打印数据*/
	if (0 == iRet)
	{
		CResPrint Printer;
		iRet = Printer.Print(m_pstMetalib, pstResNode->GetMetaName(), szBinFile, pstResNode->GetRecordSetName(),
			pstResNode->GetRecordCountName());
	}	
	
	return iRet;
}

inline int CResTranslator::ConvExcels2Bin(IN CResNode *pstResNode, IN EXCELFILELIST &stSelExcels, IN const CString &szExcelsPath)
{
	int i;
	int iRet =0;

	assert(NULL != pstResNode);
	
	for (i = 0; i < (int)stSelExcels.size(); i++)
	{
		WLogInfo(LOG_LEVEL_ANY, "开始将[%s]中的信息转换到资源文件[%s]中.......\r\n", stSelExcels[i], pstResNode->GetBinFile());

		CString szExcleFile = szExcelsPath + stSelExcels[i];
		LPDISPATCH p = CExcelApp::Instance().OpenWorkBook(szExcleFile);
		if (NULL == p)
		{
			WLogInfo(LOG_LEVEL_SEVERE,"打开Excel文件 %s 失败", szExcleFile);
			iRet = -1;
			break;
		}

		CExcelWorkBook stWorkBook(p);
		for (int j = 1; j <= stWorkBook.SheetCount(); j++)
		{
			CExcelSheet *pstSheet = stWorkBook.GetSheet(j);
			if (NULL == pstSheet)
			{
				ASSERT(false);
				return -2;
			}

			/*跳过不需要处理的Excel*/
			CString name = pstSheet->GetName();
			INCLUDESHEETLIST &stIncludeSheets = pstResNode->GetIncludeSheetList();
			if (0 < (int)stIncludeSheets.size())
			{
				INCLUDESHEETLIST::iterator it = std::find(stIncludeSheets.begin(), stIncludeSheets.end(), name);
				if (it == stIncludeSheets.end())
				{
					continue;
				}
			}else
			{
				EXCLUDESHEETLIST &stExcludeSheets = pstResNode->GetExcludeSheetList();
				EXCLUDESHEETLIST::iterator it = std::find(stExcludeSheets.begin(), stExcludeSheets.end(), name);
				if (it != stExcludeSheets.end())
				{
					continue;
				}
			}		


			TRACE("Handle sheet : %s\n", name);
			iRet = ConvExcleSheet2Bin(pstSheet, pstResNode);
			if (0 != iRet)
			{
				break;
			}
		}/*for (int j = 1; j < stWorkBook.SheetCount(); j++)*/

		if (0 != iRet)
		{
			break;
		}
	}/*for (i = 0; i < (int)stSelExcels.size(); i++)*/


	if (0 == iRet)
	{
		iRet = m_stBinFile.WriteResHead(m_iMetaCount, m_iMetaUnit);
	}

	return iRet;
}

void  CResTranslator::CalcMetaUnitSize(IN LPTDRMETA pstMeta)
{
	assert(NULL != pstMeta);	
	
	m_iMetaUnit = tdr_get_meta_size(pstMeta);
}

int CResTranslator::ConvExcleSheet2Bin(IN CExcelSheet *pstSheet, IN CResNode *pstResNode)
{
	int iRet = 0;
	int i;
	RESENTRYLIST stEntryList;
	CEntryCellItem stItem;
	int iMaxRow;
	unsigned char *pszDatabuf;


	assert(NULL != pstSheet);
	assert(NULL != pstResNode);

	/*将Excel中的数据读出来*/
	pstSheet->Load();

	/*检查行数*/
	iMaxRow = pstSheet->GetRowCount();
	if (iMaxRow <= EXCEL_ROW_OF_ENTRY_TITLE)
	{
		WLogInfo(LOG_LEVEL_WARN, "Warning: excel页表<%s>中没有任何数据，将忽略此页表\r\n", pstSheet->GetName());
		return 0;
	}

	/*创建能保存一个资源结构的缓冲区*/
	try
	{
		CalcMetaUnitSize(m_pstMeta);
		pszDatabuf = new unsigned char[m_iMetaUnit];
	}
	catch (CException* e)
	{
		WLogInfo(LOG_LEVEL_ERROR, "Error: 分配内存空间[%d]失败\r\n", m_iMetaUnit);
		return -2;
	}
	

	/*生成数据索引*/
	iRet = m_pstEntryMapParser->GenEntryCellIndex(stEntryList, pstSheet, m_pstMeta);


	/*读取数据，excel一行对应一个资源结构*/
	if (0 == iRet)
	{
		for (i = EXCEL_ROW_OF_ENTRY_TITLE + 1; i <= iMaxRow; i++)
		{
			TDRDATA stData;

			stData.pszBuff = (char *)pszDatabuf;
			stData.iBuff = m_iMetaUnit;			
			iRet = GetOneMetaFromSheet(stData, stEntryList, pstSheet, i);
			if (0 != iRet)
			{
				break;
			}
			
					

			/*将读取的资源结构写道文件中*/
			m_iMetaCount++;
			iRet = m_stBinFile.AppendWriteBinData(pszDatabuf, m_iMetaUnit);	
					
		}/*for (i = EXCEL_ROW_OF_ENTRY_TITLE + 1; i <= iMaxRow; i++)*/
	}/*if (0 != iRet)*/
	

	/*释放资源*/
	delete pszDatabuf;
	pstSheet->UnLoad();

	return iRet;
}



int CResTranslator::GetOneMetaFromSheet(INOUT TDRDATA &stData, IN RESENTRYLIST &stEntryList, IN CExcelSheet *pstSheet, IN int iRow)
{
	int i;
	int iEntryNum;
	int iLeftLen;
	char *pszHostStart;
	char *pszHostBase;
	TDRDATA stEntryData;
	int iRet = 0;

	assert(NULL != stData.pszBuff);
	assert(0 < stData.iBuff);
	assert(NULL != pstSheet);
	assert(EXCEL_ROW_OF_ENTRY_TITLE < iRow);

	iEntryNum = (int)stEntryList.size();
	if (0 >= iEntryNum)
	{
		stData.iBuff = 0;
		return -1;
	}

	iLeftLen = stData.iBuff;
	pszHostBase = stData.pszBuff;
	pszHostStart = pszHostBase;

	for (i = 0; i < iEntryNum; i++)
	{		
		LPTDRMETAENTRY pstEntry = stEntryList[i].GetEntryHandler();
		assert(NULL != pstEntry);
		if (stEntryList[i].GetEntryIdx() == 0)
		{
			pszHostStart = pszHostBase + stEntryList[i].GetHOff(); 
		}

		stEntryData.pszBuff = pszHostStart;
		stEntryData.iBuff = iLeftLen;
		iRet = GetEntryValue(stEntryData, stEntryList[i], pstSheet, iRow);
		if (0 != iRet)
		{
			break;
		}			

		
		pszHostStart += stEntryData.iBuff;
		iLeftLen -= stEntryData.iBuff;
	}/*for (i = 0; i < iEntryNum; i++)*/

	stData.iBuff = (int)(pszHostStart - stData.pszBuff);

	return iRet;
}


inline int CResTranslator::GetEntryValue(INOUT TDRDATA &stHostData, IN const CEntryCellItem &stEntryMap, IN CExcelSheet *pstSheet, IN int iRow)
{
	char *pszHostStart;
	int iRet = 0;

	assert(NULL != pstSheet);
	assert(stHostData.pszBuff != NULL);
	assert(0 < stHostData.iBuff);
	assert(EXCEL_ROW_OF_ENTRY_TITLE < iRow);

	LPTDRMETAENTRY pstEntry = stEntryMap.GetEntryHandler();
	int idxCell = stEntryMap.GetCellIndex();
	int idxEntry = stEntryMap.GetEntryIdx();

	

	pszHostStart = stHostData.pszBuff;
	TRACE("Entry: %s count %d idx: %d  OFF:%d Host: %p idxCell: %d\n", pstEntry->szName, pstEntry->iCount, idxEntry, pstEntry->iHOff, pszHostStart, idxCell);
	if (0 < idxCell)
	{
		CExcelCell *pstCell = pstSheet->GetCell(iRow, idxCell);
		assert(NULL != pstCell);

		CString szValue;
		CString sztmp;
		CString &szCell = pstCell->GetValue();
		switch(stEntryMap.GetGetValueMethod())

		{
		case GET_EXCEL_VALUE_ALLINFO:
			szValue = szCell;
			break;
		case GET_EXCEL_VALUE_NUMBER:
			GetNumber(szCell, szValue);
			break;
		case GET_EXCEL_VALUE_PERSENT:
			sztmp = m_pstKeywordParser->GetKeyMapValue(szCell);
			if ("" == sztmp)
			{
				sztmp = szCell;
			}
			GetPercent(sztmp, szValue);
			break;
		case GET_ECCEL_VALUE_INTELLIGENT:
			sztmp = m_pstKeywordParser->GetKeyMapValue(szCell);
			if ("" == sztmp)
			{
				sztmp = szCell;
			}
			GetNumber(sztmp, szValue);
			break;
		default:
			szValue = m_pstKeywordParser->GetKeyMapValue(szCell);
			if ("" == szValue)
			{
				WLogInfo(LOG_LEVEL_ERROR,"将Excel页表<%s>单元格数据<第:%d行%s 列>转换到结构体成员<%s>出错,在关键字映射表中找不到此单元格内容<%s>的替换项.",
					pstSheet->GetName(), iRow, ColNumber2ExcelColStr(idxCell), pstEntry->szName, szCell);
				return -1;
			}
			break;
		}		

		iRet = String2EntryValue(stHostData, szValue, pstEntry);
		if (0 != iRet)
		{
			WLogInfo(LOG_LEVEL_ERROR,"将Excel 单元格数据<%s>转换到结构体成员<%s>出错,Excel数据位置: Excel页表<%s>第%d行第%s列.",
				szValue, pstEntry->szName, pstSheet->GetName(), iRow, ColNumber2ExcelColStr(idxCell));
		}

	}else
	{	
		/*不从excel文档中读取,处理方法: 如果有缺省值,则设置缺省值,否则memset为0*/
		char *pszHostEnd = pszHostStart + stHostData.iBuff;
		TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, m_pstMetalib, pstEntry, 1);
		stHostData.iBuff = (int)(pszHostStart - stHostData.pszBuff);
	}/*if (0 < idxCell)*/

	

	return iRet;
}

inline int CResTranslator::String2EntryValue(INOUT TDRDATA &stHostData, IN CString &szValue, IN LPTDRMETAENTRY pstEntry)
{
	int iRet = 0;
	int iSize;
	long lVal;
	tdr_longlong llVal;

	assert(NULL != pstEntry);
	assert(stHostData.pszBuff != NULL);
	assert(0 < stHostData.iBuff);

	char *pszHostStart = stHostData.pszBuff;
	char *pszHostEnd = pszHostStart + stHostData.iBuff;
	
	/*空串设置缺省值*/
	if ("" == szValue)
	{
		TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, m_pstMetalib, pstEntry, 1);
		stHostData.iBuff = (int)(pszHostStart - stHostData.pszBuff);
		return 0;
	}

	/*非空串，从中分析数据*/
	const char *pszData = (LPCTSTR)szValue;	
	if ((TDR_TYPE_STRING == pstEntry->iType) || (TDR_TYPE_WSTRING == pstEntry->iType))
	{
		if (0 < pstEntry->iCustomHUnitSize)													
		{																						
			iSize = pstEntry->iCustomHUnitSize;												
		}else																					
		{																						
			iSize = pszHostStart - pszHostEnd;												
		}	
	}else
	{
		iSize = pstEntry->iHUnitSize;
	}
	if (iSize > stHostData.iBuff)
	{
		return -1;
	}

	switch(pstEntry->iType)
	{
	case TDR_TYPE_STRING:
		{
			int iLen;
			
			iLen = szValue.GetLength() + 1;
			if ((iLen > iSize) || ((pszHostStart + iLen) > pszHostEnd) )
			{
				iRet = -1; /*溢出*/
			}else
			{
				TDR_STRNCPY(pszHostStart, pszData, iLen);
			}
			pszHostStart += iSize;			
			break;
		}		
	case TDR_TYPE_CHAR:
		{
			iRet = GetMacroInt(lVal, m_pstMetalib, pszData);
			if (0 == iRet)
			{
				if ((-128 > lVal) || (127 < lVal) )
				{
					WLogInfo(LOG_LEVEL_ERROR,"error: 成员<name = %s>是char数据类型,给定值<%ld>超过了有效取值范围(-128~127)",
						pstEntry->szName, lVal);
					iRet = -2;
				}else
				{
					*pszHostStart = (char)lVal;	
					pszHostStart += iSize;	
				}
			}else
			{
				WLogInfo(LOG_LEVEL_ERROR,"error: 将字符串<%s>转换成整数失败，请检查<%s>中是否定义了此字符串对应的宏或数字字符串;或请检查Excel单元格数据是否有效",
					pszData, m_szKeywordFile);
			}
			break;
		}
	case TDR_TYPE_UCHAR:
		{
			iRet = GetMacroInt(lVal, m_pstMetalib, pszData);
			if (0 == iRet)
			{
				if ((0 > lVal) || (255 < lVal) )
				{
					WLogInfo(LOG_LEVEL_ERROR,"error: 成员<name = %s>是unsinged char数据类型,给定值<%ld>超过了有效取值范围(0~255).",
						pstEntry->szName, lVal);
					iRet = -2;
				}else
				{
					*(unsigned char *)pszHostStart = (unsigned char)lVal;	
					pszHostStart += iSize;	
				}
			}else
			{
				WLogInfo(LOG_LEVEL_ERROR,"error: 将字符串<%s>转换成整数失败，请检查<%s>中是否定义了此字符串对应的宏或数字字符串;或请检查Excel单元格数据是否有效",
					pszData, m_szKeywordFile);
			}
			break;
		}
	case TDR_TYPE_SHORT:
		{
			iRet = GetMacroInt(lVal, m_pstMetalib, pszData);
			if (0 == iRet)
			{
				if ((-32768 > lVal) || (0x7FFF < lVal) )
				{
					WLogInfo(LOG_LEVEL_ERROR,"error: 成员<name = %s>是short数据类型,给定值<%ld>超过了有效取值范围(-32768~32767).",
						pstEntry->szName, lVal);
					iRet = -2;
				}else
				{
					*(short *)pszHostStart = (short)lVal;	
					pszHostStart += iSize;	
				}
			}else
			{
				WLogInfo(LOG_LEVEL_ERROR,"error: 将字符串<%s>转换成整数失败，请检查<%s>中是否定义了此字符串对应的宏或数字字符串;或请检查Excel单元格数据是否有效",
					pszData, m_szKeywordFile);
			}
			break;
		}
	case TDR_TYPE_USHORT:
		{
			iRet = GetMacroInt(lVal, m_pstMetalib, pszData);
			if (0 == iRet)
			{
				if ((0 > lVal) || (0xFFFF < lVal) )
				{
					WLogInfo(LOG_LEVEL_ERROR,"error: 成员<name = %s>是unsigned short数据类型,给定值<%ld>超过了有效取值范围(0~65535).",
						pstEntry->szName, lVal);
					iRet = -2;
				}else
				{
					*(unsigned short *)pszHostStart = (unsigned short)lVal;	
					pszHostStart += iSize;	
				}
			}else
			{
				WLogInfo(LOG_LEVEL_ERROR,"error: 将字符串<%s>转换成整数失败，请检查<%s>中是否定义了此字符串对应的宏或数字字符串;或请检查Excel单元格数据是否有效",
					pszData, m_szKeywordFile);
			}
			break;
		}
	case TDR_TYPE_INT:
	case TDR_TYPE_LONG:
		{
			iRet = GetMacroInt(lVal, m_pstMetalib, pszData);
			if (0 == iRet)
			{
				if (((int)0x80000000 > lVal) || (0x7FFFFFFF < lVal) )
				{
					WLogInfo(LOG_LEVEL_ERROR,"error: 成员<name = %s>是int/long数据类型,给定值<%ld>超过了有效取值范围(-2147483648~2147483647).",
						pstEntry->szName, lVal);
					iRet = -2;
				}else
				{
					*(int *)pszHostStart = (int)lVal;	
					pszHostStart += iSize;	
				}
			}else
			{
				WLogInfo(LOG_LEVEL_ERROR,"error: 将字符串<%s>转换成整数失败，请检查<%s>中是否定义了此字符串对应的宏或数字字符串;或请检查Excel单元格数据是否有效",
					pszData, m_szKeywordFile);
			}
			break;
		}
	case TDR_TYPE_UINT:
	case TDR_TYPE_ULONG:
		{
			llVal = _atoi64(pszData);
			if (0 > llVal) 
			{
				WLogInfo(LOG_LEVEL_ERROR,"error: 成员<name = %s>是long long数据类型,给定值<%ld>超过了有效取值范围(0~4294967295).",
					pstEntry->szName, llVal);
				iRet = -2;
			}else
			{
				*(unsigned int *)pszHostStart = (unsigned int)llVal;
				pszHostStart += iSize;	
			}
			break;
		}
	case TDR_TYPE_LONGLONG:
		{
			llVal = _atoi64(pszData);
			*(tdr_longlong *)pszHostStart = llVal;	
			pszHostStart += iSize;	
			break;
		}
	case TDR_TYPE_ULONGLONG:
		{
			llVal = _atoi64(pszData);
			if (0 > llVal)
			{
				WLogInfo(LOG_LEVEL_ERROR,"error: 成员<name = %s>是unsigned long long数据类型,给定值<%ld>超过了有效取值范围.",
					pstEntry->szName, llVal);
				iRet = -2;
			}else
			{
				*(tdr_ulonglong *)pszHostStart = (tdr_ulonglong)llVal;	
				pszHostStart += iSize;	
			}
			break;
		}
	case TDR_TYPE_FLOAT:
		{
			float  fVal;

			fVal = (float)atof(pszData);
			*(float *)pszHostStart = fVal;
			pszHostStart += iSize;	
			break;
		}
	case TDR_TYPE_DOUBLE:
		{
			double  dVal;

			dVal = strtod(pszData, NULL);
			*(double *)pszHostStart = dVal;
			pszHostStart += iSize;	
			break;
		}
	case TDR_TYPE_IP:
		{
			iRet = tdr_ineta_to_tdrip((tdr_ip_t *)pszHostStart , pszData);
			pszHostStart += iSize;	
			break;
		}	
	case TDR_TYPE_DATETIME:
		{
			iRet = tdr_str_to_tdrdatetime((tdr_datetime_t *)pszHostStart, pszData);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				WLogInfo(LOG_LEVEL_ERROR,"error: <name = %s>日期/时间(datetime)类型的值无效,正确datetime类型其值格式应为\"YYYY-MM-DD HH:MM:SS\".",
					pstEntry->szName);				
			}
			pszHostStart += iSize;	
			break;
		}
	case TDR_TYPE_DATE:
		{
			iRet = tdr_str_to_tdrdate((tdr_date_t *)pszHostStart, pszData);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				WLogInfo(LOG_LEVEL_ERROR,"error: 成员<name = %s>日期(date)类型的值无效,正确date类型其值格式应为\"YYYY-MM-DD\".",
					pstEntry->szName);
			}
			pszHostStart += iSize;	
			break;
		}
	case TDR_TYPE_TIME:
		{
			iRet = tdr_str_to_tdrtime((tdr_time_t *)pszHostStart, pszData);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				WLogInfo(LOG_LEVEL_ERROR,"error:\t 成员<name = %s>时间(time)类型的值无效,正确date类型其值格式应为\"HHH:MM:SS\".",
					pstEntry->szName);
			}
			pszHostStart += iSize;	
			break;
		}
	default:			
		TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, m_pstMetalib, pstEntry, 1);
		break;
	}

	stHostData.iBuff = (int)(pszHostStart - stHostData.pszBuff);

	return iRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

int GetMacroInt(OUT long &lID, IN TDRMETALIB* pstLib, IN const  char *pszValue)
{
	int iRet = 0;

	assert( NULL != pstLib);
	assert(NULL != pszValue);


	if( ( pszValue[0] >= '0' && pszValue[0] <= '9' ) ||
		('+' == pszValue[0]) || ('-' == pszValue[0]) )
	{
		lID = (int)strtol(pszValue, NULL, 0);;
	}
	else
	{
		iRet = tdr_get_macro_value((int *)&lID, pstLib, pszValue);		
	}

	return iRet;
}


inline CString &ColNumber2ExcelColStr(int iCol)
{
	static CString szCol;
	int iTmp;

	szCol = "";
	
	while (iCol > 0)
	{
		iCol -= 1;
		iTmp = iCol % 26;
		szCol += (char)('A' + iTmp);
		iCol /= 26;
	}

	szCol.MakeReverse();

	return szCol;
}

