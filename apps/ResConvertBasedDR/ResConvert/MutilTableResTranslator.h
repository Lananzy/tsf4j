#pragma once
#include "restranslator.h"

class CMutilTableResTranslator :
	public CResTranslator
{
public:
	CMutilTableResTranslator(void);
	virtual ~CMutilTableResTranslator(void);
protected:
	virtual int CreateEntryMapParser(IN ConvTreeParser *pstConvTree, IN CResNode *pstResNode);
	virtual int ConvExcleSheet2Bin(IN CExcelSheet *pstSheet, IN CResNode *pstResNode);
private:
	inline int GetSubTableID(IN TDRDATA &stData, IN LPTDRMETAENTRY pstEntry, IN CExcelSheet *pstSheet, IN int iIDCol);
	inline int GetSubTableData(OUT int &iCount, IN TDRDATA &stData, IN LPTDRMETAENTRY pstEntry, IN CExcelSheet *pstSheet);
private:
	CString m_szSubTableID;	/*保存SubTableID的成员名*/	
	LPTDRMETAENTRY m_pstSubTableEntry;
};
