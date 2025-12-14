#pragma once

#include "../comm/ConvDefine.h"
#include "tdr/tdr.h"

class CResPrint
{
public:
	CResPrint(void);
	~CResPrint(void);

	int Print(IN LPTDRMETALIB pstMetalib, IN const CString &szMetaName, IN const CString &szBinFile,\
		IN const CString &szRecordSetName, IN const CString &szRecordCountName);
};
