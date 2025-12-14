#ifndef RESSTYLELIST_H
#define RESSTYLELIST_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <cstring>
#include <vector>

class CResStyle 
{
public:
	int m_iID;	/*id of res style */
	CString m_szResStyleName;

public:
	CResStyle():m_iID(-1),m_szResStyleName(_T("")){};
	~CResStyle() {};
};

typedef std::vector<CResStyle> RESSTYLELIST;

class CResStyleManager
{
public:
	CResStyleManager(void);
	~CResStyleManager(void);
	
	/** 根据 res styleid 获取在列表中获取此Res Style
	*@param[in] iID 待查找ResStyle的ID
	*@return  CResStyle的指针：找到；	NULL： 没有找到
	*/
	CResStyle *GetResStyle(int iID);
	RESSTYLELIST &GetResStyleList(){return m_stStyleList;};

	/** 清除Res style列表
	*/
	void ClearResStyleList();

	/** 插入一个res stryle
	*@param[in] stResStyle 要插入的ResStyle
	*@param[out] szError 如果插入失败，则通过此参数获得出错原因
	*@return  0: 插入成功 非0值：插入失败
	*/
	int InsertResStyle(CResStyle &stResStyle, CString &szError);

	

private:
	RESSTYLELIST m_stStyleList;
};

#endif
