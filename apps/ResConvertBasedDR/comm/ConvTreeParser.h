#ifndef CONVTREEPARSER_H
#define CONVTREEPARSER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ConvResTree.h"

class ConvTreeParser
{
public:
	ConvTreeParser(void);
	virtual ~ConvTreeParser(void);

	/** 解析资源转换树配置文件
	*@param[in] szFile	配置文件名
	*@param[out] szError	如果解析失败，则通过此参数获取失败原因
	*@return	成功： true ；失败：false
	*/
	virtual bool ParseFile(CString &szFile, CString &szError) = 0;

	/** 将资源转换树信息导出到XML文件中
	*@param[in] szFile	导出的XML文件名
	*@param[in] szError 如果出错，输出出错信息
	*@return	成功： true ；失败：false
	*/
	bool Dump2XML(CString &szFile, CString &szError);
	
	/*获取资源转换配置文件是否采用新的格式*/
	bool IsNewFormat(){return m_bIsNewFormat;};
	void SetNewFormat(bool bIs) {m_bIsNewFormat = bIs;};

	/*获取资源树
	*@return 返回资源树引用
	*/
	CRESTREE &GetResTree() {return m_stResTree;};

	void SetMetalibFile(CString &szFileName) {m_szMetalibFile = szFileName;};
	void SetMetalibFile(const char *pszFileName) {assert(NULL != pszFileName);m_szMetalibFile = pszFileName;};
	CString &GetMetalibFile(){return m_szMetalibFile;};
	
	void SetEntryMapFilesPath(CString &szPath) {m_szEntryMapFilesPath = szPath;};
	void SetEntryMapFilesPath(const char *pszPath) {assert(NULL != pszPath);m_szEntryMapFilesPath = pszPath;};
	CString &GetEntryMapFilesPath(){return m_szEntryMapFilesPath;};

	void SetBinFilesPath(CString &szPath){m_szBinFilesPath = szPath;};
	void SetBinFilesPath(const char *pszPath){assert(NULL != pszPath);m_szBinFilesPath = pszPath;};
	CString &GetBinFilesPath(){return m_szBinFilesPath;};

	void SetExcelFilesPath(const char *pszPath){assert(NULL != pszPath);m_szExcelFilesPath = pszPath;};
	void SetExcelFilesPath(CString &szPath) {m_szExcelFilesPath;};
	CString &GetExcelFilesPath(){return m_szExcelFilesPath;};
	
	CResStyleManager &GetResStyleManager(){return m_stResStyleList;};

	CString &GetKeywordFile() {return m_szKeywordFile;};


	/*释放资源树*/
	void DeleteResTree();

protected:
	CRESTREENODE *InsertCommNode(CRESTREENODE *pstParent, CString &szNodeName);
	
	void ParseResStyleList(CResNode *pstData, CString &szStyles, CString &szError);

	/** 对资源转换树树进行正规话处理
	*1.如果根节点name为空，且只有一个儿子节点，则将此儿子节点调整为根节点
	*/
	void NomalizeResTree();

	void FreeSubTree(CRESTREENODE *pstNode);
	void FreeNode(CRESTREENODE *pstNode);

private:
	CString &GenBinStyleStr(RESIDLIST &aiIds);
	bool ResNode2Xml(CStdioFile & stFile, const CString &szSpace, CRESTREENODE * pstNode);
protected:
	bool m_bIsNewFormat;	/*配置文件是否是新的格式：是:true, 不是: false*/
	CRESTREE	m_stResTree;	/*资源转换树*/
	CString	m_szMetalibFile;	/*元数据文件*/
	CString m_szEntryMapFilesPath;	/*EntryMapFile保存路径*/
	CString m_szBinFilesPath;	/*bin文件保存路径*/
	CString m_szExcelFilesPath;	/*excel文件保存路径*/
	CResStyleManager m_stResStyleList;
	CString m_szKeywordFile;	/*keyword file*/

};

#endif