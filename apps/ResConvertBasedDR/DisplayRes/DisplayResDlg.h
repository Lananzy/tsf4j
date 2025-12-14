// DisplayResDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "tdr/tdr.h"

#define  ROW_FOR_RES_TITLE	0

// CDisplayResDlg 对话框
class CDisplayResDlg : public CDialog
{
// 构造
public:
	CDisplayResDlg(CWnd* pParent = NULL);	// 标准构造函数
	
// 对话框数据
	enum { IDD = IDD_DISPLAYRES_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedDisplayres();
protected:
	CListCtrl m_stResList;
	CString m_szResBinFile;
	CString m_szMetalibFile;
	CString m_szMeta;

protected:
	afx_msg void OnBnClickedSelectbinres();

	afx_msg void OnBnClickedSelectmetalib();

protected:
	inline int GetResTitle(LPTDRMETA pstMeta);
	inline int DisPlayOneRes(int idx, LPTDRMETA pstMeta, unsigned char *pszData, int iDataLen);
};
