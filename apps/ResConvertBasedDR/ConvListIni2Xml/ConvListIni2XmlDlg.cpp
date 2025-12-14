// ConvListIni2XmlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include <assert.h>
#include "ConvListIni2Xml.h"
#include "ConvListIni2XmlDlg.h"
#include "../comm/ConvDefine.h"
#include "../comm/error_report.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CConvListIni2XmlDlg 对话框




CConvListIni2XmlDlg::CConvListIni2XmlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConvListIni2XmlDlg::IDD, pParent)
	, m_szEntryMapPath(_T(""))
	, m_szXmlFilePath(_T(""))
	, m_szResult(_T(""))
	, m_szResStylePath(_T(""))
	, m_szMetalibFile(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CConvListIni2XmlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EntryMapPATH, m_szEntryMapPath);
	DDX_Text(pDX, IDC_XMLFILEPATH, m_szXmlFilePath);
	DDX_Text(pDX, IDC_RESULT, m_szResult);
	DDX_Text(pDX, IDC_RESSTYLEPATH, m_szResStylePath);
	DDX_Text(pDX, IDC_METALIBFILE, m_szMetalibFile);
}

BEGIN_MESSAGE_MAP(CConvListIni2XmlDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON3, &CConvListIni2XmlDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_SELECTEntryMap, &CConvListIni2XmlDlg::OnBnClickedSelectEntryMap)
	ON_EN_CHANGE(IDC_EntryMapPATH, &CConvListIni2XmlDlg::OnEnChangeEntryMappath)
	ON_BN_CLICKED(IDC_SELECTXMLFILE, &CConvListIni2XmlDlg::OnBnClickedSelectxmlfile)
	ON_BN_CLICKED(IDC_SELECTRESSTYLEFILE, &CConvListIni2XmlDlg::OnBnClickedSelectresstylefile)
	ON_BN_CLICKED(IDC_SELECTMETALIB, &CConvListIni2XmlDlg::OnBnClickedSelectmetalib)
END_MESSAGE_MAP()


// CConvListIni2XmlDlg 消息处理程序

BOOL CConvListIni2XmlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//g_pedtResult = (CEdit *)GetDlgItem(IDC_RESULT);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CConvListIni2XmlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CConvListIni2XmlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CConvListIni2XmlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CConvListIni2XmlDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	ConvTreeIniParser parser;

	m_szResult = "";

	UpdateData(TRUE);
	if (!m_szEntryMapPath.IsEmpty() && m_szXmlFilePath.IsEmpty())
	{
		GenXMLPathByIniPath(m_szEntryMapPath);
		UpdateData(FALSE);
	}

	/*检查输入变量的合法性*/
	if (m_szEntryMapPath.IsEmpty())
	{
		m_szResult = "请指定转换需要的资源列表ini文件";
		CButton *pstSelEntryMap = (CButton *)GetDlgItem(IDC_SELECTEntryMap);
		pstSelEntryMap->SetFocus();
		UpdateData(FALSE);
		return;
	}
	if (m_szResStylePath.IsEmpty())
	{
		m_szResult = "请指定转换需要的资源列表ini文件";
		CButton *pstSelStyleFile = (CButton *)GetDlgItem(IDC_SELECTRESSTYLEFILE);
		pstSelStyleFile->SetFocus();
		UpdateData(FALSE);
		return;
	}
	if (m_szMetalibFile.IsEmpty())
	{
		m_szResult = "请指定转换需要的元数据描述库文件";
		CButton *pstSelStyleFile = (CButton *)GetDlgItem(IDC_SELECTMETALIB);
		pstSelStyleFile->SetFocus();
		UpdateData(FALSE);
		return;
	}
	

	/*分析res stryle*/
	AnalyzeResStyles(parser);

	/*分析convlist ini文件*/
	if (!parser.ParseFile(m_szEntryMapPath, m_szResult))
	{
		UpdateData(FALSE);
		return;
	}	

	parser.SetMetalibFile(m_szMetalibFile);


	/*生成xml文件*/
	if (!parser.Dump2XML(m_szXmlFilePath, m_szResult))
	{
		m_szResult = "将ini转换配置文件转换程XML文件失败";
	}else
	{
		m_szResult = "成功生成转换配置xml文件";
	}
	UpdateData(FALSE);
}

void CConvListIni2XmlDlg::OnBnClickedSelectEntryMap()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog fdlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, _T("Ini(*.ini)|*.ini|All Files (*.*)|*.*||"));
	if( fdlg.DoModal()==IDOK )
	{
		m_szEntryMapPath = fdlg.GetPathName();
		GenXMLPathByIniPath(m_szEntryMapPath);
		UpdateData(FALSE);
	}
}



void CConvListIni2XmlDlg::OnEnChangeEntryMappath()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	
}


inline void CConvListIni2XmlDlg::GenXMLPathByIniPath(const CString & szIniPath)
{
	int iPos;

	if( (iPos = m_szEntryMapPath.ReverseFind('.')) >= 0 )
		m_szXmlFilePath = m_szEntryMapPath.Left(iPos);
	else
		m_szXmlFilePath = m_szEntryMapPath;

	m_szXmlFilePath += ".xml";
	
}
void CConvListIni2XmlDlg::OnBnClickedSelectxmlfile()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog fdlg(FALSE, NULL, NULL, OFN_OVERWRITEPROMPT, _T("Xml(*.xml)|*.xml||"));
	if( fdlg.DoModal()==IDOK )
	{
		m_szXmlFilePath = fdlg.GetPathName();
		UpdateData(FALSE);
	}
}

void CConvListIni2XmlDlg::OnBnClickedSelectresstylefile()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog fdlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, _T("Ini(*.ini)|*.ini|All Files (*.*)|*.*||"));
	if( fdlg.DoModal()==IDOK )
	{
		m_szResStylePath = fdlg.GetPathName();

		UpdateData(FALSE);
	}
}

int CConvListIni2XmlDlg::AnalyzeResStyles(ConvTreeIniParser &stParser)
{
	int iRet = 0;
	CStdioFile fpFile;

	
	if( !fpFile.Open(m_szResStylePath, CFile::modeRead) )
	{
		CString szTmp;
		szTmp.Format(_T("文件 %s 无法打开，请重新选择文件！"), m_szResStylePath);
		MessageBox(szTmp, _T("错误"), MB_OK | MB_ICONWARNING);
		return -1;
	}

	CResStyleManager &stResStyleList = stParser.GetResStyleManager();
	stResStyleList.ClearResStyleList();
	CString szLine;
	while( fpFile.ReadString(szLine) )
	{
		CResStyle stResStyle;
		int iCurPos = 0;
		CString szToken;

		szToken = szLine.Tokenize(_T("\n "), iCurPos);
		if (szToken == "")
		{
			continue;
		}
		stResStyle.m_iID = atoi((char   *)(LPCTSTR)szToken);
		stResStyle.m_szResStyleName = szLine.Tokenize(_T("\n "), iCurPos);
		if ("" == stResStyle.m_szResStyleName)
		{
			continue;
		}
		
		if (0 != stResStyleList.InsertResStyle(stResStyle, m_szResult))
		{
			UpdateData(FALSE);
			iRet = -2;
			break;
		}

	}
	fpFile.Close();

	return iRet;
}


void CConvListIni2XmlDlg::OnBnClickedSelectmetalib()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog fdlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, _T("DR(*.dr)|*.dr|All Files (*.*)|*.*||"));
	if( fdlg.DoModal()==IDOK )
	{
		m_szMetalibFile = fdlg.GetPathName();

		UpdateData(FALSE);
	}
}
