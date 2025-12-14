// ConvListIni2XmlDlg.h : 头文件
//

#pragma once

#include "../comm/ResStyleList.h"
#include "../comm/ConvTreeIniParser.h"

// CConvListIni2XmlDlg 对话框
class CConvListIni2XmlDlg : public CDialog
{
// 构造
public:
	CConvListIni2XmlDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CONVLISTINI2XML_DIALOG };

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
	afx_msg void OnBnClickedButton3();
public:
	afx_msg void OnBnClickedSelectEntryMap();
public:
	afx_msg void OnEnChangeEntryMappath();
public:
	CString m_szEntryMapPath;
public:
	CString m_szXmlFilePath;
public:
	CString m_szResult;
protected:

	void GenXMLPathByIniPath(const CString & szIniPath);	
public:
	afx_msg void OnBnClickedSelectxmlfile();
public:
	CString m_szResStylePath;
public:
	afx_msg void OnBnClickedSelectresstylefile();
protected:
	int AnalyzeResStyles(ConvTreeIniParser &stParser);	

protected:
	CString m_szMetalibFile;
public:
	afx_msg void OnBnClickedSelectmetalib();
};
