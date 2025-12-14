#pragma once
#include "EntryMapParser.h"

enum tagMapNodeType
{
	MAP_NODE_TYPE_MIDDLE = 0,	/*中间节点:对应struct/union*/
	MAP_NODE_TYPE_LEAF,		/*叶子节点:对应entry成员*/
};

/*Excel CELL - Meta Entry映射关系*/
class CMappNode
{
public:
	CMappNode(CString &szName, int iType);
	~CMappNode();

	void SetName(CString &szName) {m_szEntryName = szName;};
	CString &GetName(){return m_szEntryName;};
	int GetType(){return m_iType;};
protected:
	CString m_szEntryName;	/*entry name*/
	int m_iType;	/*节点类型 取DR中定义的*/

};

class CLeafMappNode : public CMappNode
{
public:
	CLeafMappNode(CString &szName, int iType, CString &szTitle, GETEXCELVALUEMETHOD enGetValueMethod = GET_EXCEL_VALUE_KEYWORDS);
	~CLeafMappNode();

	void SetTitle(CString &szTitle) {m_szExcelTitle = szTitle;};
	CString &GetTitle(){return m_szExcelTitle;};

	void SetGetExcelValueMethod(GETEXCELVALUEMETHOD enMethod) {m_enGetValueMethod = enMethod;};
	GETEXCELVALUEMETHOD GetGetExcelValueMethod(){return m_enGetValueMethod;};
	CString &GetExcelValueMethodStr();
protected:
	GETEXCELVALUEMETHOD m_enGetValueMethod;	/*取值方法*/
	CString m_szExcelTitle;	/*Excel CELL 名称*/
private:
};

typedef CTreeNode<CMappNode *>  ENTRYMAPNODE;
typedef CTree<CMappNode *>		ENTRYMAPTREE;

struct tagStackMetaItem
{
	ENTRYMAPNODE *_pstNode;
	LPTDRMETA	_pstMeta;
};
typedef struct tagStackMetaItem STACKMETAITEM;

typedef std::list<STACKMETAITEM> MAPSTACK;

class CIniEntryMapParser :
	public EntryMapParser
{
public:
	CIniEntryMapParser(void);
	virtual ~CIniEntryMapParser(void);

	virtual int ParseEntryMap(IN LPTDRMETALIB &pstMetalib, IN LPTDRMETA pstMeta, IN const ENTRYMAPFILELIST &stFileList, IN const CString &szPath);

	/*根据Excel页表确定资源结构体中具体的映射成员列表
	*@param[out] stList 映射成员列表
	*@param[in] pstSheet Excel页表对象指针
	*@param[in] pstMeta 资源结构体句柄
	*/
	virtual int GenEntryCellIndex(OUT RESENTRYLIST &stList, IN CExcelSheet *pstSheet, IN LPTDRMETA pstMeta);
protected:

	int ParseEntryMapInfo(CString &a_szLine);

	int AddOneEntryMap(CString &a_szTitle, CString &a_szEntry);

	virtual int PushMetasMap(INOUT CString &szEntryMap) = 0;
	virtual int PopMetasMap(CString &EntyMap) = 0;

	/** 根据节点名称获取下一个儿子节点
	*@param[in] pstParent 父节点
	*@param[in] pszName 节点名
	*@param[in] pstChlid 儿子节点，如果其值为NULL，则取第一个儿子节点
	*/
	ENTRYMAPNODE *GetNextMapNodeByName(ENTRYMAPNODE *pstParent, const char *pszName, ENTRYMAPNODE *pstChlid = NULL);


	int GetMapNodeCountByName(ENTRYMAPNODE *pstNode, CString &szName);


	inline int GenSimpleEntryCellIndex(OUT RESENTRYLIST &stList, IN CExcelSheet *pstSheet, IN ENTRYMAPNODE *pstParentNode, IN int iHMetaOff, IN LPTDRMETAENTRY pstEntry);

	/*导出映射树，调试时使用*/
	void DumpMapInfo(CString &szFile);


	void Clear();
protected:
	MAPSTACK m_stMetaStack;

	ENTRYMAPTREE m_stEntryMapTree;	/*CELL-Entry映射树*/

	CString m_szEntryMap;
};

class CNameIniEntryMapParser: public CIniEntryMapParser 
{
public:
	CNameIniEntryMapParser();
	virtual ~CNameIniEntryMapParser();
protected:
	virtual int PushMetasMap(INOUT CString &szEntryMap);
	virtual int PopMetasMap(CString &EntyMap);
private:
};

class CTypeIniEntryMapParser: public CIniEntryMapParser 
{
public:
	CTypeIniEntryMapParser();
	virtual ~CTypeIniEntryMapParser();
protected:
	virtual int PushMetasMap(INOUT CString &szEntryMap);
	virtual int PopMetasMap(CString &EntyMap);
private:
};