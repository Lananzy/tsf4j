#pragma once
#include "ckeywordparser.h"

class CXmlKeyordParser :
	public CkeywordParser
{
public:
	CXmlKeyordParser(void);
	virtual ~CXmlKeyordParser(void);

	virtual int Parse(IN CString &szFile);
};
