#ifndef CONVTREEINIPARSE_H
#define CONVTREEINIPARSE_H


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "convtreeparser.h"
#include "ConvDefine.h"

class ConvTreeIniParser :
	public ConvTreeParser
{
public:
	ConvTreeIniParser(void);
	virtual ~ConvTreeIniParser(void);

	/**解析资源转换树配置文件
	*@param[in] szFile	配置文件名
	*@param[out] szError	如果解析失败，则通过此参数获取失败原因
	*@return	成功： true ；失败：false
	*/
	virtual bool ParseFile(CString &szFile, CString &szError);

private:
	

	
	void ParseSortMethod(CResNode *pstData, CString &szStyles);
	bool ParseEntryMap(CString &szPath, CResNode *pstData, CString &szEntryMap, CString &szError);
	void ParseBinFile(CResNode *pstData, CString &szBinFile);
	void ParseExcelFile(CResNode &stData, CString sParam[]);
};

#endif
