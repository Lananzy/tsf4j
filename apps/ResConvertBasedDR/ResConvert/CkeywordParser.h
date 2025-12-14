#pragma once

#include <string>
#include <map>
#include "../comm/ConvDefine.h"


class CkeywordParser
{
public:
	CkeywordParser(void);
	~CkeywordParser(void);

	/*解析keyword文件
	*@param[in] szFile 保存keyword信息的文件名
	*/
	virtual int Parse(IN CString &szFile) = 0;

	/*根据key值取对应的映射值
	*@param[in] szKey key值
	*@return 如果找到，返回有对应的值； 否则返回""
	*/
	CString GetKeyMapValue(IN CString &szKey);

protected:
	typedef std::map<CString, CString> KEYWORD_MAP;
	KEYWORD_MAP m_stkeywordMap;
};
