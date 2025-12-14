#pragma once
#include "Ckeywordparser.h"

class CIniKeywordParser :
	public CkeywordParser
{
public:
	CIniKeywordParser(void);
	virtual ~CIniKeywordParser(void);

	virtual int Parse(IN CString &szFile);
};
