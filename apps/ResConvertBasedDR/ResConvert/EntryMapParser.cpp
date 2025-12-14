#include "stdafx.h"
#include <list>
#include "EntryMapParser.h"
#include "tdr/tdr.h"
#include "tdr/tdr_metalib_kernel_i.h"
#include "../comm/error_report.h"

//////////////////////////////////////////////////////////////////////////////////////////////
GETEXCELVALUEMETHOD ParseEntryGetValueMethod(IN LPTDRMETAENTRY pstEntry)
{
	GETEXCELVALUEMETHOD eMethod;

	assert(NULL != pstEntry);

	switch(pstEntry->iType)
	{
	case TDR_TYPE_CHAR:
	case TDR_TYPE_UCHAR:
	case TDR_TYPE_BYTE:
	case TDR_TYPE_SHORT:
	case TDR_TYPE_USHORT:
	case TDR_TYPE_INT:
	case TDR_TYPE_UINT:
	case TDR_TYPE_LONG:	
	case TDR_TYPE_ULONG:
	case TDR_TYPE_LONGLONG:	
	case TDR_TYPE_ULONGLONG:
	case TDR_TYPE_MONEY:
	case TDR_TYPE_IP:
		eMethod = GET_ECCEL_VALUE_INTELLIGENT;
		break;
	case TDR_TYPE_FLOAT:
	case TDR_TYPE_DOUBLE:
		eMethod = GET_EXCEL_VALUE_KEYWORDS;
		break;
	case TDR_TYPE_STRING:
	case TDR_TYPE_WSTRING:
	case TDR_TYPE_DATE:
	case TDR_TYPE_TIME:
	case TDR_TYPE_DATETIME:
		eMethod = GET_EXCEL_VALUE_ALLINFO;
		break;
	default:
		eMethod = GET_EXCEL_VALUE_KEYWORDS;
		break;
	}

	return eMethod;
}




///////////////////////////////////////////////////////////////////////////////////////////////
CEntryCellItem::CEntryCellItem()
{
	_iIdxEntry = 0;
	_iIdxCell = -1;
	_pstEntry = NULL;
	_enGetValueMethod = GET_EXCEL_VALUE_KEYWORDS;
	_iHOff = 0;
}

CEntryCellItem::~CEntryCellItem()
{
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
EntryMapParser::EntryMapParser(void)
{
}

EntryMapParser::~EntryMapParser(void)
{
}

