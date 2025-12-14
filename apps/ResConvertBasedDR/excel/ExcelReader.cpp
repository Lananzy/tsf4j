// ExcelReader.cpp: implementation of the CExcelReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ExcelReader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COleVariant vOpt((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
void RemoveTailInvalidZero(CString& cNumber);

CExcelApp &CExcelApp::Instance()
{
	static CExcelApp inst;
	return inst;
}

CExcelApp::CExcelApp()
{
	opened = false;
	if (!CreateApp()) 
		exit(0);
}

CExcelApp::~CExcelApp()
{
	QuitApp();
}

bool CExcelApp::CreateApp()
{
	if(!CreateDispatch("Excel.Application"))
	{
		AfxMessageBox("Cannot start Excel and get Application object.");
		return false;
	}
	opened = true;

	TRACE("CExcelApp::CreateApp CreateDispatch\n");
	return true;
}

void CExcelApp::QuitApp()
{
	if (opened)
	{
		Quit();
		ReleaseDispatch(); 
		opened = false;
		TRACE("CExcelApp::QuitApp ReleaseDispatch\n");
	}
}

LPDISPATCH CExcelApp::OpenWorkBook(const CString &filePath)
{
	CWorkbooks books = get_Workbooks();
	LPDISPATCH book = books.Open(filePath, vOpt, vOpt, vOpt, vOpt, vOpt, vOpt, vOpt, vOpt, vOpt, vOpt, vOpt, vOpt, vOpt, vOpt);
	books.ReleaseDispatch();
	return book;
}

CExcelWorkBook::CExcelWorkBook(LPDISPATCH pDispatch)
: CWorkbook(pDispatch)
{
	CWorksheets sheets = get_Sheets();
	int sheetCount = sheets.get_Count();

	for (int i = 1; i <= sheetCount; ++i)
	{	
		CExcelSheet *sheet = new CExcelSheet(sheets.get_Item(COleVariant((short)i)));
		sheets_.push_back(sheet);
	}
	sheets.ReleaseDispatch();
}

int CExcelWorkBook::SheetCount()
{
	return sheets_.size();
}

CExcelSheet *CExcelWorkBook::GetSheet(int index)
{
	if (index < 1 || index > (int)sheets_.size())
	{
		ASSERT(false);
		return NULL;
	}
	return sheets_[index - 1];
}

void CExcelWorkBook::CloseWorkbook()
{
	Close(COleVariant((short)FALSE), vOpt, vOpt);
}

CExcelWorkBook::~CExcelWorkBook()
{
	for (int i = 0; i < (int)sheets_.size(); ++i)
	{
		CExcelSheet *sheet = sheets_[i];
		delete sheet;
	}
	CloseWorkbook();
}

CExcelSheet::CExcelSheet(LPDISPATCH pDisPatch)
: CWorksheet(pDisPatch)
{
	loaded = false;
}

CString CExcelSheet::GetName()
{
	return get_Name();
}

int CExcelSheet::GetRowCount()
{
	if (!loaded)
		Load();
	return rowCount;
}

int CExcelSheet::GetColCount()
{
	if (!loaded)
		Load();
	return colCount;
}

CExcelSheet::~CExcelSheet()
{
	ClearCells();
}

CExcelCell *CExcelSheet::GetCell(int r, int c)
{
	if (!loaded) Load();

	if (r <= 0 || r > rowCount
		|| c <= 0 || c > colCount)
	{
		ASSERT(FALSE);
		return NULL;
	}
	return cells[(r - 1) * colCount + c - 1];
}


void CExcelSheet::ClearCells()
{
	while (cells.size())
	{
		delete cells.back();
		cells.pop_back();
	}
}

void CExcelSheet::UnLoad()
{
	ClearCells();
	loaded = false;
}

void CExcelSheet::Load()
{
	if (loaded) return;

	ClearCells();

	CRange usedRange;
	CRange range;

	usedRange = get_UsedRange();	
	/*
	range.AttachDispatch(usedRange.get_Rows());
	rowCount = range.get_Count();

	range.AttachDispatch(usedRange.get_Columns());
	colCount = range.get_Count();
	*/

	VARIANT ret = usedRange.get_Value2();
	if (!(ret.vt & VT_ARRAY))
	{
		rowCount = 0;
		colCount = 0;
		loaded = true;
		return;
	}
	
	//Create the SAFEARRAY from the VARIANT ret.
	COleSafeArray sa(ret);

	//Determine the array's dimensions.
	sa.GetUBound(1, &rowCount);
	sa.GetUBound(2, &colCount);  
	

	//Display the elements in the SAFEARRAY.
	long index[2];
	VARIANT val;
	long lRealRowCount = 0;
	//TRACE("=====================\n\t");
	for (int r = 1; r <= rowCount; ++r)
	{
		if (IsEmptyRow(sa, r, colCount))
		{
			break;
		}

		lRealRowCount++;
		//TRACE("Row %d", lRealRowCount);
		for (int c = 1; c <= colCount; ++c)
		{
			index[0] = r;
			index[1] = c;
			sa.GetElement(index, &val);

			//TRACE("row: %d col: %d Data: %s\n", r,c, ExcelType2String(val));

			CExcelCell *cell = (new CExcelCell(ExcelType2String(val), lRealRowCount, c));
			cells.push_back(cell);
		}
		//TRACE("\n");
	}
	rowCount = lRealRowCount;
	loaded = true;
}
inline bool CExcelSheet::IsEmptyRow(COleSafeArray &sa, int iRow, int iMaxCol)
{
	long index[2];
	VARIANT val;
	int c;

	for (c = 1; c <= iMaxCol; ++c)
	{
		index[0] = iRow;
		index[1] = c;
		sa.GetElement(index, &val);

		if ("" != ExcelType2String(val))
		{
			break;
		}		
	}
	if (c > iMaxCol)
	{
		return true;
	}

	return false;
}

CExcelCell::CExcelCell(CString &value, int row, int col)
{
	this->value = value;
	this->row = row;
	this->col = col;
}

inline CString &ExcelType2String(const VARIANT &value)
{
	static CString result;

	switch (value.vt)
	{
	case VT_EMPTY:
		result = "";
		break;
	case VT_R8:
		result.Format("%f", value.dblVal);
		RemoveTailInvalidZero(result);
		break;
	case VT_BSTR:
		result = value.bstrVal;
		result.TrimLeft(" \r\n");
		result.TrimRight(" \r\n");
		break;
	default:
		{
			CString msg;
			msg.Format("未处理的cell类型 %d", value.vt);
			AfxMessageBox(msg);
			ASSERT(FALSE);
		}
		break;
	}
	return result;
}

void RemoveTailInvalidZero(CString& cNumber)
{
	int iDotPos;
	int iLen;

	iLen = cNumber.GetLength()-1;
	iDotPos = cNumber.ReverseFind('.');
	if (0 > iDotPos)
	{
		return;
	}

	while (iLen > iDotPos)
	{
		if (cNumber[iLen] == '0')
		{
			iLen--;
		}else
		{
			break;
		}
	}

	if (0 == iLen)
	{
		cNumber = "";
	}else
	{
		cNumber = cNumber.Left(iLen);
	}
}




