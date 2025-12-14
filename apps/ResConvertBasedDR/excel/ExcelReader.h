// ExcelReader.h: interface for the CExcelReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXCELREADER_H__D59EF9CF_BAB5_4AAA_8F73_7D14D901E75A__INCLUDED_)
#define AFX_EXCELREADER_H__D59EF9CF_BAB5_4AAA_8F73_7D14D901E75A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CApplication.h"
#include "CWorkbooks.h"
#include "CWorkbook.h"
#include "CWorksheet.h"
#include "CWorksheets.h"
#include "CRange.h"
#include <vector>

class CExcelCell
{
public:
	CExcelCell(CString &value, int row, int col);
	int GetRow() { return row; }
	int GetCol() { return col; }
	CString &GetValue() { return value; }
private:
	CString value;
	int row;
	int col;
};

class CExcelSheet : public CWorksheet
{
public:
	CExcelSheet(LPDISPATCH pDisPatch);
	virtual ~CExcelSheet();
public:
	CString GetName();
	int GetRowCount(); 
	int GetColCount();
	void Load();
	void UnLoad();
	CExcelCell *GetCell(int r, int c);
protected:
	void ClearCells();
	inline bool IsEmptyRow(COleSafeArray &sa, int iRow, int iMaxCol);
private:
	bool loaded;
	std::vector<CExcelCell *> cells;
	long rowCount, colCount;
};

class CExcelWorkBook : public CWorkbook
{
public:
	CExcelWorkBook() {};
	CExcelWorkBook(LPDISPATCH pDispatch);
	virtual ~CExcelWorkBook();
	int SheetCount();
	CExcelSheet *GetSheet(int index);

protected:
	void CloseWorkbook();

protected:
	std::vector<CExcelSheet *>sheets_;
};

class CExcelApp : public CApplication
{
public:
	static CExcelApp &Instance();
	CExcelApp();
	virtual ~CExcelApp();
	LPDISPATCH OpenWorkBook(const CString &filePath);
	void QuitApp();
protected:
	bool CreateApp();

protected:
	bool opened;
};


inline CString &ExcelType2String(const VARIANT &value);

#endif // !defined(AFX_EXCELREADER_H__D59EF9CF_BAB5_4AAA_8F73_7D14D901E75A__INCLUDED_)
