// ResConvertDlg.h : header file
//

#if !defined(AFX_RESCONVERTDLG_H__65AF0AA7_A0B6_4A9A_A315_7FA3725CED57__INCLUDED_)
#define AFX_RESCONVERTDLG_H__65AF0AA7_A0B6_4A9A_A315_7FA3725CED57__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TreeOptionsCtrl.h"	/*to remove*/
#include <comdef.h>
#include "excel.h"

#include <vector>
#include "../comm/ConvTreeXmlParser.h"
#include "tdr/tdr.h"

/////////////////////////////////////////////////////////////////////////////
// CResConvertDlg dialog

#define MAX_STYLE_SELECT	128
#define MAX_PARAM_NUM		128
#define MAX_FILE_LINE		512
#define MAX_LENGTH_LINE		1024

typedef std::vector<HTREEITEM> CONVNODELIST;



class CResConvertDlg : public CDialog
{
// Construction
public:
    BOOL         m_bRunTag  ;
  
public:
	HANDLE m_hKillThreadEvent;		// 通知子线程关闭的事件
	HANDLE m_hThreadKilledEvent;	// 子线程宣告关闭的事件

    static UINT CheckTimerThread(LPVOID lpParam);	// 短消息收发处理子线程
    void CloseExcelMsg() ;
	void AddUILog(CString &sInfo);
	void AddUILog(const char *fmt, ...);

	void SetTreeTab(HTREEITEM hNode, RESIDLIST &stSelectedIDs);
	void GetSelectTab(HTREEITEM hNode, CONVNODELIST &stList);
	int IsInsertTree(CString XlsFile);
	void InitTreeTab();




	void AnalyzeResConvTree();

	CResConvertDlg(CWnd* pParent = NULL);	// standard constructor


// Dialog Data
	//{{AFX_DATA(CResConvertDlg)
	enum { IDD = IDD_RESCONVERT_DIALOG };
	CButton	m_chkRunTag;
	CEdit	m_ctlInfo;
	CProgressCtrl	m_ctlProgress;
	CListCtrl	m_cStyleList;
	CTreeOptionsCtrl	m_pTree;	
	CString	m_szResFilePath;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResConvertDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
	
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CResConvertDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelectfile();
	afx_msg void OnDestroy();
	afx_msg void OnReschange();
	afx_msg void OnSelect();
	afx_msg void OnRefresh();
	afx_msg void OnBtnClean();
	afx_msg void OnCheckRuntag();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool InsertChildsToCtrlTree(HTREEITEM hRoot, CRESTREE &stTree, CRESTREENODE *pstResRoot);
	int LoadMetalib(CString &szMetalibFile);
	int ConvOneRes(HTREEITEM h);
private:
	ConvTreeXmlParser m_stConvTreeParser;	/*资源转换树解析器*/
	
	CString m_szResult;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESCONVERTDLG_H__65AF0AA7_A0B6_4A9A_A315_7FA3725CED57__INCLUDED_)
