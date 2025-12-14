#pragma once

#include <stdio.h>
#include "../comm/ConvDefine.h"
#include "../comm/ConvResTree.h"
#include "../comm/ConvTreeParser.h"
#include "../excel/ExcelReader.h"
#include "tdr/tdr.h"
#include "EntryMapParser.h"
#include "CkeywordParser.h"
#include "ResBinFile.h"



/** ×ÊÔ´×ª»»Æ÷
*/
class CResTranslator
{
public:
	CResTranslator(void);
public:
	~CResTranslator(void);

public:
	int Translate(IN ConvTreeParser *pstConvTree, IN CResNode *pstResNode, IN EXCELFILELIST &stSelExcels);

protected:
	virtual int CreateEntryMapParser(IN ConvTreeParser *pstConvTree, IN CResNode *pstResNode);
	virtual int ConvExcleSheet2Bin(IN CExcelSheet *pstSheet, IN CResNode *pstResNode);

	inline int CreateMetalib(IN ConvTreeParser *pstConvTree, IN CResNode *pstResNode);
	inline int CreateKeywordParser(IN ConvTreeParser *pstConvTree, IN CResNode *pstResNode);

	inline int ConvExcels2Bin(IN CResNode *pstResNode, IN EXCELFILELIST &stSelExcels, IN const CString &szExcelsPath);
	
	int GetOneMetaFromSheet(INOUT TDRDATA &stData, IN RESENTRYLIST &stEntryList, IN CExcelSheet *pstSheet, IN int iRow);

	inline int GetEntryValue(INOUT TDRDATA &stHostData, IN const CEntryCellItem &stEntryMap, IN CExcelSheet *pstSheet, IN int iRow);

	inline int String2EntryValue(INOUT TDRDATA &stHostData, IN CString &szValue, IN LPTDRMETAENTRY pstEntry);

	inline void CalcMetaUnitSize(IN LPTDRMETA pstMeta);

	inline void Clear();
protected:
	EntryMapParser *m_pstEntryMapParser;
	CkeywordParser *m_pstKeywordParser;
	LPTDRMETALIB m_pstMetalib;
	LPTDRMETA m_pstMeta;
	CResBinFile m_stBinFile;
	int m_iMetaCount;
	int m_iMetaUnit;	
	CString m_szKeywordFile;
};
