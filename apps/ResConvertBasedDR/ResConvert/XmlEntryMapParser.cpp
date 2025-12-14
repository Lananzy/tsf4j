#include "stdafx.h"
#include <assert.h>
#include "../comm/error_report.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_metalib_kernel_i.h"
#include "XmlEntryMapParser.h"

CXmlEntryMapParser::CXmlEntryMapParser(void)
{
}

CXmlEntryMapParser::~CXmlEntryMapParser(void)
{
}

int CXmlEntryMapParser::ParseEntryMap(IN LPTDRMETALIB &pstMetalib, IN LPTDRMETA pstMeta, IN const ENTRYMAPFILELIST &stFileList, IN const CString &szPath)
{
	/*映射关键已经在元数据描述中*/
	return 0;
}

int CXmlEntryMapParser::GenEntryCellIndex(OUT RESENTRYLIST &stList, IN CExcelSheet *pstSheet, IN LPTDRMETA pstMeta)
{
	int iRet = 0;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;	
	int iChange;
	LPTDRMETALIB pstLib;
	CString szPreTitle("");

	assert(NULL != pstSheet);
	assert(NULL != pstMeta);	

	stList.clear();
	pstLib = TDR_META_TO_LIB(pstMeta);
	pstCurMeta = pstMeta;
	pstStackTop = &stStack[0];
	pstStackTop->pszMetaSizeInfoTarget = NULL;	/*entry of meta*/
	pstStackTop->iEntrySizeInfoOff = 0; /*len of pretitle at curmeta*/
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
			szPreTitle = szPreTitle.Left(pstStackTop->iEntrySizeInfoOff);			
			szPreTitle.AppendFormat("%d", pstEntry->iCount - pstStackTop->iCount + 1);						

			iChange = 0;
			continue;
		}/*if ((0 != iChange) && (pstStackTop->iCount > 0))*/
		iChange = 0;


		if (0 >= pstStackTop->iCount)
		{/*当前元数据数组已经处理完毕*/
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstCurMeta = pstStackTop->pstMeta;			
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				szPreTitle = szPreTitle.Left(pstStackTop->iEntrySizeInfoOff);
			}			
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;	
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			CEntryCellItem stItem;
			stItem.SetEntryHandler(pstEntry);
			stItem.SetHOff(pstStackTop->iMetaSizeInfoOff + pstEntry->iHOff);
			for (int i = 0;i < pstEntry->iCount; i++)
			{
				stItem.SetEntryIdx(i);
				stList.push_back(stItem);
				TRACE("Entry: %s HOff: %d Size: %d EntryIdx: %d  Index: %d\n", pstEntry->szName, stItem.GetHOff(), pstEntry->iHRealSize, stItem.GetEntryIdx(), -1);
			}			

			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		/*如果没有cname属性，则此成员设置缺省值*/
		if (TDR_INVALID_PTR == pstEntry->ptrChineseName)
		{
			CEntryCellItem stItem;
			stItem.SetEntryHandler(pstEntry);
			stItem.SetHOff(pstStackTop->iMetaSizeInfoOff + pstEntry->iHOff);
			for (int i = 0;i < pstEntry->iCount; i++)
			{
				stItem.SetEntryIdx(i);
				stList.push_back(stItem);
				TRACE("Entry: %s HOff: %d Size: %d EntryIdx: %d  Index: %d\n", pstEntry->szName, stItem.GetHOff(), pstEntry->iHRealSize, stItem.GetEntryIdx(), -1);
			}		
		
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{/*复合数据类型*/
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = -2;
				WLogInfo(LOG_LEVEL_ERROR, "资源信息结构体<%s>嵌套层次太深，目前仅支持%d级结构.", TDR_STACK_SIZE);
				break;
			}

			
			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			iStackItemCount++;
			pstStackTop++;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = pstEntry->iCount;
			pstStackTop->idxEntry = 0;
			pstStackTop->pszMetaSizeInfoTarget = (char *)pstEntry;
			pstStackTop->iMetaSizeInfoUnit = 1;
			pstStackTop->iMetaSizeInfoOff = (pstStackTop -1)->iMetaSizeInfoOff + pstEntry->iHOff;

			szPreTitle.AppendFormat("%s", TDR_GET_STRING_BY_PTR(pstLib, pstEntry->ptrChineseName));
			pstStackTop->iEntrySizeInfoOff = szPreTitle.GetLength();
			if (pstStackTop->iCount > 0)
			{
				szPreTitle += "1";
			}
					
		}else if (TDR_TYPE_WSTRING >= pstEntry->iType)
		{
 			iRet = GenSimpleEntryCellIndex(stList, pstSheet, pstLib, pstStackTop->iMetaSizeInfoOff, pstEntry, szPreTitle);
			if (0 != iRet)
			{
				break;
			}					
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/
	}/*while (0 < iStackItemCount)*/

	return iRet;
}

inline int CXmlEntryMapParser::GenSimpleEntryCellIndex(OUT RESENTRYLIST &stList, IN CExcelSheet *pstSheet, IN LPTDRMETALIB pstLib, IN int iHMetaOff, IN LPTDRMETAENTRY pstEntry, IN CString szPreTile)
{
	int i;
	int iRet = 0;
	char *pszCName;
	CString szTitle;
	CEntryCellItem stItem;
	int idxCell;
	GETEXCELVALUEMETHOD eMethod;
	int iEntryOff;

	assert(NULL != pstSheet);
	assert(NULL != pstLib);
	assert(NULL != pstEntry);
	assert(TDR_INVALID_PTR != pstEntry->ptrChineseName);
	assert(0 < pstEntry->iCount);

	iEntryOff = iHMetaOff + pstEntry->iHOff;
	eMethod = ParseEntryGetValueMethod(pstEntry);
	pszCName = TDR_GET_STRING_BY_PTR(pstLib, pstEntry->ptrChineseName);
	stItem.SetEntryHandler(pstEntry);
	stItem.SetGetValueMethod(eMethod);
	stItem.SetHOff(iEntryOff);
	
	if (1 == pstEntry->iCount)
	{
		szTitle.Format("%s%s",szPreTile,pszCName);
		idxCell = MatchSheetTitle(pstSheet, szTitle);		
		stItem.SetCellIndex(idxCell);		
		stList.push_back(stItem);
		TRACE("Entry: %s HOff: %d Size: %d EntryIdx: %d  Index: %d M: %d Title: %s\n", pstEntry->szName, stItem.GetHOff(), pstEntry->iHRealSize, stItem.GetEntryIdx(),
			stItem.GetCellIndex(), stItem.GetGetValueMethod(), szTitle);
	}else
	{
		for (i = 0; i < pstEntry->iCount; i++)
		{
			szTitle.Format("%s%s%d",szPreTile,pszCName, i+1);
			idxCell = MatchSheetTitle(pstSheet, szTitle);			
			stItem.SetCellIndex(idxCell);
			stItem.SetEntryIdx(i);
			stList.push_back(stItem);
			TRACE("Entry: %s HOff: %d Size: %d EntryIdx: %d  Index: %d M: %d Title: %s\n", pstEntry->szName, stItem.GetHOff(), pstEntry->iHRealSize, stItem.GetEntryIdx(),
				stItem.GetCellIndex(), stItem.GetGetValueMethod(), szTitle);
		}/*for (i = 0; i < pstEntry->iCount; i++)*/
	}/*if (0 ==pstEntry->iCount)*/


	return iRet;
}


inline int CXmlEntryMapParser::MatchSheetTitle(IN CExcelSheet *pstSheet, IN const CString &szEntryTitle)
{
	int i;

	assert(NULL != pstSheet);

	int iMaxCol = pstSheet->GetColCount();
	for (i = 1; i <= iMaxCol; i++)
	{
		CString szTitle = pstSheet->GetCell(EXCEL_ROW_OF_ENTRY_TITLE, i)->GetValue();			

		szTitle.TrimLeft(" \r\n");
		szTitle.TrimRight(" \r\n");

		if (szTitle.IsEmpty())
		{
			continue;
		}
		if (szTitle == szEntryTitle)
		{
			break;
		}
	}/*for (i = 1; i < iMaxCol; i++)*/

	if (i > iMaxCol)
	{		
		WLogInfo(LOG_LEVEL_WARN, "Warning: 在excel页表<%s>没有找到标题为<%s>的列, 请检查ini配置文化和Excel文件是否正确.", pstSheet->GetName(), szEntryTitle);
		return -1;
	}

	return i;
}