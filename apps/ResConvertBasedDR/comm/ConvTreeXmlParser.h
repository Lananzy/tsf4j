#pragma once
#include "convtreeparser.h"
#include <scew/scew.h>

class ConvTreeXmlParser :
	public ConvTreeParser
{
public:
	ConvTreeXmlParser(void);
public:
	virtual ~ConvTreeXmlParser(void);

	/**解析资源转换树配置文件
	*@param[in] szFile	配置文件名
	*@param[out] szError	如果解析失败，则通过此参数获取失败原因
	*@return	成功： true ；失败：false
	*/
	virtual bool ParseFile(CString &szFile, CString &szError);
public:
	
private:
	bool ParserBaseConfInfo(scew_element & stRoot, CString & szError);
	bool ParseResStyles(scew_element *pstRoot, CString & szError);
	bool ParseConvTree(scew_element *pstRoot, CString & szError);
	bool ParseConvTreeNode(scew_element *pstRoot, CRESTREENODE *pstResRoot, CString & szError);
	inline bool ParseMutilTableAttribute(scew_element *pstRoot, CResNode *pstResData, CString & szError);
	

	inline const char *GetXmlNodeNameAttr(scew_element *pstNode);
	inline bool ParseNodeExcelFiles(scew_element *pstNode, CResNode *pstResData, CString &szError);
	inline bool ParseNodeExcludeSheets(scew_element *pstNode, CResNode *pstResData, CString &szError);
	/*清除前一次解析遗留的信息*/
	void ClearAllInfo();
};
