// ConvResTree.h  : 资源转换树头文件
//

#ifndef CONVRESTREE_H
#define CONVRESTREE_H


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <cstring>
#include <vector>
#include <string>

#include "ResStyleList.h"
#include "Tree.h"

/*资源转换节点类型*/
enum tagResTreeNodeType
{
	RES_TREE_NODE_TYPE_COMM = 0,	/*中间节点*/
	RES_TREE_NODE_TYPE_RES,		/*叶节点*/
};
typedef tagResTreeNodeType RESTREENODETYPE;


/*ini配置文件格式版本*/
enum tagEntryMapVersion
{
	INI_FILE_VERSION_OLD = 0,  /*之前FO的格式*/
	INI_FILE_VERSION_NEW = 1	/*当前资源转换工具定义的格式*/
};
typedef tagEntryMapVersion EntryMapVERSION;

/*资源信息排序方法*/
enum tagSortMethod
{
	SORT_METHOD_NO = 0,	/*不排序*/
	SORT_METHOD_ASC,	/*增序排序*/
	SORT_METHOD_DESC,	/*降序排序*/
};
typedef tagSortMethod SORTMETHOD;


/*资源转换树，采用儿子－兄弟法表示*/
class CCommNode
{
public:
	CCommNode();
	CCommNode(const CString &szName, int iType = RES_TREE_NODE_TYPE_COMM);
	~CCommNode();
	
	int GetNodeType(){return m_iType;};

	CString &GetNodeName(){return m_szName; };
protected:
	int m_iType;		/*节点类型，以区分叶子节点和中间节点*/
	CString m_szName;   /*节点名称*/
};


typedef std::vector<CString>  EXCELFILELIST;	/*待处理的Excel列表*/
typedef std::vector<CString>  EXCLUDESHEETLIST;	/*不需处理的页表名称*/
typedef std::vector<int> RESIDLIST;	/*资源ID分类*/
typedef std::vector<CString>  ENTRYMAPFILELIST; /*字段定义文件列表*/
typedef std::vector<CString>  INCLUDESHEETLIST;	/*只需处理的页表名称*/

/*包含转换信息的叶节点*/
class CResNode : public CCommNode
{
public:
	CResNode();
	CResNode(CString &szName);

	~CResNode();

public:
	 void SetEntryMapFile(CString &stEntryMap);
	 ENTRYMAPFILELIST &GetEntryMapFiles();

	 void SetBinFile(CString &szBinFile);
	 CString &GetBinFile();

	 void SetSortMethod(SORTMETHOD enSortMethod);
	 void SetSortMethod(const char *pszMethod);
	 SORTMETHOD GetSortMethod();

	 void AddResStyleID(int iID);
	 bool IsExpectedResStyles(int iStyleID);
	 RESIDLIST &GetResIDList(){return m_aiResStyleIDs;};

	 void AddExcelFile(CString &szExcelFile);
	 EXCELFILELIST &GetExcelFiles();

	void SetMetaName(CString &szName) {m_szMetaName = szName;};
	CString &GetMetaName(){return m_szMetaName; };

	void AddExcludeSheet(CString &szSheet){m_astExcludeSheets.push_back(szSheet);};
	EXCLUDESHEETLIST &GetExcludeSheetList(){return m_astExcludeSheets;};

	void SetKeywordFile(CString &szFile) {m_szKeywordFile = szFile;};
	CString GetKeywordFile(){return m_szKeywordFile;};

	bool IsMutilTables(){return m_bIsMutilTables;};
	void SetIsMutilTables(bool bMutil = false) {m_bIsMutilTables = bMutil;};

	
	int GetSubTableIDCol(){return m_iSubTableIDColumn;};
	void SetSubTableIDCol(int iCol) {m_iSubTableIDColumn = iCol;};

	CString &GetSubTableName(){return m_szSubTableName;};
	void SetSubTableName(CString &szName) {m_szSubTableName = szName;};

	CString &GetSubTableIDName(){return m_szSubTableIDName;};
	void SetSubTableIDName(CString &szName) {m_szSubTableIDName = szName;};

	void SetRecordCountName(CString &szName){m_szRecordCountName = szName;};
	CString &GetRecordCountName(){return m_szRecordCountName;};

	void SetRecordSetName(CString &szName) {m_szRecordSetName = szName;};
	CString &GetRecordSetName(){return m_szRecordSetName;};

	bool IsMapEntryByName(){return m_bIsMapEntryByName;};
	void SetIsMapEntryByName(bool bIsName=true) {m_bIsMapEntryByName = bIsName;};

	void AddIncludeSheet(CString &szSheet){m_astIncludeSheets.push_back(szSheet);};
	INCLUDESHEETLIST &GetIncludeSheetList(){return m_astIncludeSheets;};
protected:
	ENTRYMAPFILELIST m_aszEntryMapFiles;	/*ini 配置文件*/

	CString m_szBinFile;	/*生成的bin文件*/
	SORTMETHOD m_enSortMethod;	/*排序方法*/
	
	RESIDLIST m_aiResStyleIDs;	/*资源id列表*/

	CString m_szMetaName;				/*元数据名*/
	EXCELFILELIST m_astExcelFiles;	/*转换的Excel列表*/
	EXCLUDESHEETLIST m_astExcludeSheets;	/*不需处理的页表名称*/

	INCLUDESHEETLIST m_astIncludeSheets;	/*不需处理的页表名称*/

	CString m_szKeywordFile;	/*keyword文件*/

	bool m_bIsMutilTables;  /*是否是多表转换*/
	bool m_bIsMapEntryByName; /*对于嵌套结构指明是否使用成员名来建立影射关系:其值为true表示使用成员进行影射*/
	int m_iSubTableIDColumn;
	CString m_szSubTableName;
	CString m_szSubTableIDName;

	CString m_szRecordCountName;
	CString m_szRecordSetName;

};

typedef CTreeNode<CCommNode *> CRESTREENODE; /*资源转换树节点*/
typedef CTree<CCommNode *>	CRESTREE;		/*资源转换树*/


#endif
