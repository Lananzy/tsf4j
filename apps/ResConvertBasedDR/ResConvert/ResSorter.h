#pragma once

#include "../comm/ConvDefine.h"
#include "../comm/ConvResTree.h"
#include "tdr/tdr.h"

class CResSorter
{
public:
	CResSorter(void);
	~CResSorter(void);

	int Sort(IN LPTDRMETALIB pstMetalib, IN const CString &szMetaName, IN const CString &szBinFile, SORTMETHOD enSort);
};
