#include "stdafx.h"
#include "../comm/error_report.h"
#include "../comm/ConvBase.h"
#include "MutilTableResTranslator.h"
#include "EntryMapParser.h"
#include "IniEntryMapParser.h"
#include "XmlEntryMapParser.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_metalib_kernel_i.h"


CMutilTableResTranslator::CMutilTableResTranslator(void)
{
	m_pstSubTableEntry = NULL;
}

CMutilTableResTranslator::~CMutilTableResTranslator(void)
{
}

int CMutilTableResTranslator::CreateEntryMapParser(IN ConvTreeParser *pstConvTree, IN CResNode *pstResNode)
{
	int iRet;
	CString szPath;
	LPTDRMETAENTRY pstEntry;

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
	

	/*检查是否存在保存ID的成员*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		m_szSubTableID = pstResNode->GetSubTableIDName();
		LPTDRMETAENTRY pstEntry = tdr_get_entryptr_by_name(m_pstMeta, (LPCTSTR)m_szSubTableID);
		if (NULL == pstEntry)
		{
			WLogInfo(LOG_LEVEL_ERROR, "结构体<%s>不成在名字为<%s>的成员,请检查转换列表配置文件中节点<%s>的配置.", 
				pstResNode->GetMetaName(), m_szSubTableID, pstResNode->GetNodeName());
			iRet = -2;
		}
		
	}
	
	
	
	/*检查孙结构体成员*/
	if (0 == iRet)
	{
		if (!pstConvTree->IsNewFormat())
		{
			int i;

			for (i = 0; i < m_pstMeta->iEntriesNum;i++)
			{
				pstEntry = m_pstMeta->stEntries + i;
				if (pstEntry->ptrMeta == TDR_INVALID_PTR)
				{
					continue;
				}
				LPTDRMETA pstTypeMeta = TDR_PTR_TO_META(m_pstMetalib, pstEntry->ptrMeta);
				if (0 == _stricmp(pstTypeMeta->szName, (LPCTSTR)pstResNode->GetSubTableName()))
				{
					break;
				}
			}/*for (int i = 0; i < m_pstMeta->iEntriesNum;i++)*/
			if (i < m_pstMeta->iEntriesNum)
			{
				m_pstSubTableEntry = pstEntry;
			}
		}else
		{
			m_pstSubTableEntry = tdr_get_entryptr_by_name(m_pstMeta, (LPCTSTR)pstResNode->GetSubTableName());
		}/*if (INI_FILE_VERSION_OLD == pstResNode->GetEntryMapVersion())*/	
		
		if (NULL == m_pstSubTableEntry)
		{
			WLogInfo(LOG_LEVEL_ERROR, "结构体<%s>不成在名字为<%s>的成员,请检查转换列表配置文件中节点<%s>的配置.", 
				pstResNode->GetMetaName(), pstResNode->GetSubTableName(), pstResNode->GetNodeName());
			iRet = -2;
		}else if (m_pstSubTableEntry->ptrMeta == TDR_INVALID_PTR)
		{
			WLogInfo(LOG_LEVEL_ERROR, "多表转换模式, 根据配置结构体<%s>名字为<%s>的成员必须是复合数据类型.", 
				pstResNode->GetMetaName(), pstResNode->GetSubTableName());
			iRet = -3;
		}
	}/*if (0 == iRet)*/

	/*解析孙结构体映射关系*/
	if (0 == iRet)
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

		LPTDRMETA pstSubMeta = TDR_PTR_TO_META(m_pstMetalib, m_pstSubTableEntry->ptrMeta);
		iRet = m_pstEntryMapParser->ParseEntryMap(m_pstMetalib, pstSubMeta, pstResNode->GetEntryMapFiles(), szPath);
	}	

	return iRet;


}

int CMutilTableResTranslator::ConvExcleSheet2Bin(IN CExcelSheet *pstSheet, IN CResNode *pstResNode)
{
	int iRet = 0;
	int i;
	RESENTRYLIST stEntryList;
	CEntryCellItem stItem;
	int iMaxRow;
	unsigned char *pszDatabuf;
	LPTDRMETAENTRY pstEntry;
	int iLeftLen;
	TDRDATA stData;

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
		WLogInfo(LOG_LEVEL_SEVERE, "Error: 分配内存空间[%d]失败\r\n", m_iMetaUnit);
		return  -2;
	}

	/*读取数据,一个页表对应一个结构,需要读两块数据: 孙结构ID,孙结构体数组*/	
	char *pszHostStart = (char *)pszDatabuf;
	char *pszHostEnd = pszHostStart + m_pstMeta->iHUnitSize;
	iLeftLen = m_pstMeta->iHUnitSize;
	for (i = 0; i < m_pstMeta->iEntriesNum; i++)
	{

		pstEntry = m_pstMeta->stEntries +i;

		/*检查subtableID属性*/
		if (m_szSubTableID.Compare(pstEntry->szName) == 0)
		{
			stData.iBuff = iLeftLen;
			stData.pszBuff = pszHostStart;
			iRet = GetSubTableID(stData, pstEntry, pstSheet, pstResNode->GetSubTableIDCol());
			if (0 != iRet)
			{
				break;
			}

			pszHostStart += stData.iBuff;
			iLeftLen -= stData.iBuff;
			continue;
		} 

		
		if (pstEntry == m_pstSubTableEntry)
		{
			/*填充孙结构*/
			int iCount = 0;

			stData.iBuff = iLeftLen;
			stData.pszBuff = pszHostStart;
			iRet = GetSubTableData(iCount, stData, pstEntry, pstSheet);
			if (0 != iRet)
			{
				break;
			}

			/*设置Refer属性值*/
			if (TDR_INVALID_PTR != pstEntry->stRefer.iHOff)
			{
				char *pszReferHostStart = (char *)pszDatabuf + pstEntry->stRefer.iHOff;
				TDR_SET_INT(pszReferHostStart, pstEntry->stRefer.iUnitSize, iCount);				
			}/*if (TDR_INVALID_PTR != pstEntry->stRefer.iHOff)*/

			/*剩下的数据填缺省值*/
			pszHostStart += stData.iBuff;
			iLeftLen -= stData.iBuff;
			if (iCount < pstEntry->iCount)
			{
				TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, m_pstMetalib, pstEntry, pstEntry->iCount - iCount);
			}		

			continue;
		}/*if (pstResNode->GetSubTableName().Compare(pstEntry->szName) == 0)*/

		/*其他结构填缺省值或填0*/
		TDR_SET_DEFAULT_VALUE(iRet, pszHostStart, pszHostEnd, m_pstMetalib, pstEntry, pstEntry->iCount);	
		if (0 != iRet)
		{
			break;
		}
	}/*for (i = 0; i < m_pstMeta->iEntriesNum; i++)*/

	if (0 == iRet)
	{
		m_iMetaCount++;
		iRet = m_stBinFile.AppendWriteBinData(pszDatabuf, m_iMetaUnit);	
	}

	/*释放资源*/
	if (NULL != pszDatabuf)
	{
		delete pszDatabuf;
	}
	pstSheet->UnLoad();

	return iRet;
}

inline int CMutilTableResTranslator::GetSubTableID(IN TDRDATA &stData, IN LPTDRMETAENTRY pstEntry, IN CExcelSheet *pstSheet, IN int iIDCol)
{
	int iRet = 0;
	GETEXCELVALUEMETHOD enMethod; /*SubTableID的成员的取值方法*/

	assert(NULL != stData.pszBuff);
	assert(0 < stData.iBuff);
	assert(NULL != pstEntry);
	assert(NULL != pstSheet);

	int iMaxCol = pstSheet->GetColCount();
	if (iIDCol > iMaxCol)
	{
		WLogInfo(LOG_LEVEL_SEVERE, "Error: Excel页表<%s>最大单元列为%d,给定的SubTableIDColumn值<%d>无效\r\n", pstSheet->GetName(),iMaxCol, iIDCol);
		return  -1;
	}

	CExcelCell *pstCell = pstSheet->GetCell(EXCEL_ROW_OF_ENTRY_TITLE, iIDCol);
	assert(NULL != pstCell);

	CString szValue;
	CString sztmp;
	CString &szCell = pstCell->GetValue();
	enMethod = ParseEntryGetValueMethod(pstEntry);
	switch(enMethod)
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
			szValue = szCell;
		}
	}
	TRACE("SubID: %s th: %d", pstEntry->szName, enMethod);
	iRet = String2EntryValue(stData, szValue, pstEntry);

	return iRet;
}


inline int CMutilTableResTranslator::GetSubTableData(OUT int &iCount, IN TDRDATA &stData, IN LPTDRMETAENTRY pstEntry, IN CExcelSheet *pstSheet)
{
	int iRet = 0;
	RESENTRYLIST stEntryList;
	CEntryCellItem stItem;
	char *pszHostStart;
	int iLeftLen;
	LPTDRMETA pstMeta;
	int i;

	assert(NULL != stData.pszBuff);
	assert(0 < stData.iBuff);
	assert(NULL != pstEntry);
	assert(NULL != pstSheet);
	assert(TDR_INVALID_PTR !=  pstEntry->ptrMeta);

	pstMeta = TDR_PTR_TO_META(m_pstMetalib, pstEntry->ptrMeta);
	iRet = m_pstEntryMapParser->GenEntryCellIndex(stEntryList, pstSheet, pstMeta);
	if (iRet != 0)
	{
		return iRet;
	}

	/*读取数据，excel一行对应一个资源结构*/
	int iMaxRow = pstSheet->GetRowCount();
	if (pstEntry->iCount < iMaxRow - 1)
	{
		iMaxRow = pstEntry->iCount + 1;
	}	
	iLeftLen = stData.iBuff;
	pszHostStart = stData.pszBuff;
	iCount = 0;
	for (i = EXCEL_ROW_OF_ENTRY_TITLE + 1; i <= iMaxRow; i++)
	{
		TDRDATA stSubData;

		stSubData.pszBuff = pszHostStart;
		stSubData.iBuff = iLeftLen;			
		iRet = GetOneMetaFromSheet(stSubData, stEntryList, pstSheet, i);
		if (0 != iRet)
		{
			break;
		}
		

		pszHostStart += stSubData.iBuff;
		iLeftLen -= stSubData.iBuff;
		iCount++;
	}/*for (i = EXCEL_ROW_OF_ENTRY_TITLE + 1; i <= iMaxRow; i++)*/



	stData.iBuff = (int)(pszHostStart - stData.pszBuff);
	

	return iRet;
}