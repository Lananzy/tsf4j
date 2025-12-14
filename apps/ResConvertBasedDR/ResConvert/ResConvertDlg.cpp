// ResConvertDlg.cpp : implementation file
//

#include "stdafx.h"

#include <string.h>
#include <shlwapi.h>
#include "ResConvert.h"
#include "MutilTableResTranslator.h"
#include "ResConvertDlg.h"
#include "../comm/ConvBase.h"
#include "ResTranslator.h"
#include "../comm/error_report.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResConvertDlg dialog

CResConvertDlg::CResConvertDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CResConvertDlg::IDD, pParent)
	, m_szResult(_T(""))
{
	//{{AFX_DATA_INIT(CResConvertDlg)
	m_szResFilePath = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_bRunTag = TRUE ;


	WLogInit(&m_ctlInfo, ((m_bRunTag)? LOG_LEVEL_ERROR: LOG_LEVEL_WARN));	
}

void CResConvertDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResConvertDlg)
	DDX_Control(pDX, IDC_CHECK_RUNTAG, m_chkRunTag);
	DDX_Control(pDX, IDC_EDIT_INFO, m_ctlInfo);
	DDX_Control(pDX, IDC_PROGRESS_STATUS, m_ctlProgress);
	DDX_Control(pDX, IDC_STYLELIST, m_cStyleList);
	DDX_Control(pDX, IDC_RESTREE, m_pTree);
	DDX_Text(pDX, IDC_RESFILEPATH, m_szResFilePath);
	//}}AFX_DATA_MAP

	DDX_Text(pDX, IDC_EDIT_INFO, m_szResult);
}

BEGIN_MESSAGE_MAP(CResConvertDlg, CDialog)
	//{{AFX_MSG_MAP(CResConvertDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SELECTFILE, OnSelectfile)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RESCHANGE, OnReschange)
	ON_BN_CLICKED(IDC_SELECT, OnSelect)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_BN_CLICKED(IDC_BTN_CLEAN, OnBtnClean)
	ON_BN_CLICKED(IDC_CHECK_RUNTAG, OnCheckRuntag)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResConvertDlg message handlers

CString szLog = "";

BOOL CResConvertDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	m_cStyleList.SetExtendedStyle(LVS_EX_CHECKBOXES);

	 

    m_chkRunTag.SetCheck(m_bRunTag) ;

	m_hKillThreadEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThreadKilledEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	// 启动子线程
	::AfxBeginThread(CheckTimerThread, this, THREAD_PRIORITY_NORMAL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CResConvertDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CResConvertDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CResConvertDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CResConvertDlg::OnSelectfile() 
{
	// TODO: Add your control notification handler code here
	CFileDialog fdlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, "Xml(*.xml)|*.xml||");
	if( fdlg.DoModal()==IDOK )
	{
		m_szResFilePath = fdlg.GetPathName();
		UpdateData(FALSE);
	}

	AnalyzeResConvTree();

}

void CResConvertDlg::OnDestroy() 
{
	::SetEvent(m_hKillThreadEvent);			// 发出关闭子线程的信号
	::WaitForSingleObject(m_hThreadKilledEvent, INFINITE);	// 等待子线程关闭
	

	::CloseHandle(m_hKillThreadEvent);
	::CloseHandle(m_hThreadKilledEvent);


	CDialog::OnDestroy();
}

// 添加日志.

inline void CResConvertDlg::AddUILog(const char *fmt, ...)
{
	char x[2048];
	va_list va;

	if ( !IsWindow(m_ctlInfo.GetSafeHwnd()) ) return ;

	va_start(va, fmt);
	vsnprintf(x, sizeof(x),fmt, va);
	va_end(va);	

	int nLength = m_ctlInfo.SendMessage(WM_GETTEXTLENGTH);

	
	if ( nLength >= 0 )
	{
		m_ctlInfo.SetSel(nLength, nLength);
		m_ctlInfo.ReplaceSel( x ); 
	}	
}

inline void CResConvertDlg::AddUILog(CString &sInfo)
{
	int nLength = m_ctlInfo.SendMessage(WM_GETTEXTLENGTH);


	if ( nLength >= 0 )
	{
		m_ctlInfo.SetSel(nLength, nLength);
		m_ctlInfo.ReplaceSel((LPCTSTR)sInfo); 
	}	
}


void CResConvertDlg::AnalyzeResConvTree()
{
	
	CString strTmp;
	HTREEITEM hRoot = NULL;
	CRESTREENODE *pstResRoot;
	
	m_pTree.DeleteAllItems();
	m_pTree.SetTextColor(RGB(63, 174, 47));
	m_pTree.SetItemHeight(25);
	m_cStyleList.DeleteAllItems();
	OnBtnClean();

	/*解析XML转换配置库*/
	strTmp = "";
	if (!m_stConvTreeParser.ParseFile(m_szResFilePath,strTmp))
	{
		MessageBox(strTmp, "错误", MB_OK | MB_ICONWARNING);
		return ;
	}
	if ("" != strTmp)
	{
		AddUILog((LPCTSTR)strTmp);
	}

#ifdef _DEBUG_
	m_stConvTreeParser.Dump2XML(m_szResFilePath +"_dump", strTmp);
#endif
	
	/*生成界面展示树*/
	CRESTREE &stTree = m_stConvTreeParser.GetResTree();
	pstResRoot = stTree.GetRootNode();
	if ((NULL == pstResRoot) || (NULL == pstResRoot->GetNodeData()))
	{
		strTmp.Format("资源转换配置文件<%s>中没有包含任何转换配置信息", m_szResFilePath);
		MessageBox(strTmp, strTmp, MB_OK | MB_ICONWARNING);
		return ;
	}
	hRoot = m_pTree.InsertCheckBox(pstResRoot->GetNodeData()->GetNodeName(), NULL, FALSE, (HTREEDATA)pstResRoot);

	InsertChildsToCtrlTree(hRoot, stTree, pstResRoot);

	hRoot = m_pTree.GetNextItem(NULL, TVGN_CHILD);
	m_pTree.Expand(hRoot, TVE_EXPAND);

	
	
	/*建立资源类型表*/
	RESSTYLELIST &stStyles = m_stConvTreeParser.GetResStyleManager().GetResStyleList();
	for (int i = 0; i < (int)stStyles.size(); i++)
	{
		m_cStyleList.InsertItem(i, stStyles[i].m_szResStyleName, 0);
	}
}

void CResConvertDlg::OnReschange() 
{
	// TODO: Add your control notification handler code here
	CONVNODELIST stList;
	HTREEITEM hRoot;
	CString sInfo;
	int iRet ;
	CString szPath, szTmp, szResPath;
	CString szFile[MAX_PARAM_NUM];
	int i;
	CString sBeginTime;


	hRoot = m_pTree.GetNextItem(NULL, TVGN_CHILD);
	if (NULL == hRoot)
	{
		return;
	}

	GetSelectTab(hRoot, stList);
	if (0 >= (int)stList.size())
	{
		return ;
	}
	
	if( !SetCurrentDirectory(GetRootPath()))
	{
		AddUILog("SetCurrentDirectory failed (%d)\n", GetLastError());
		return ;
	}


	AddUILog("需要转换生成[%d]个资源文件，开始批量转化.......\r\n", (int)stList.size());
	sBeginTime = "\r\n起始时间:" + CTime::GetCurrentTime().Format( "%Y.%m.%d %H:%M:%S" )  + "\r\n" ;	

	m_ctlProgress.SetRange(0, stList.size()) ;
    m_ctlProgress.SetPos(0) ;


	for(i=0; i < (int)stList.size(); i++)
	{
		CResTranslator stTranslator;
		CMutilTableResTranslator stMutilTableTranslator;

		CResNode *pstResData = (CResNode *)m_pTree.GetItemData(stList[i]);
		if (NULL == pstResData)
		{
			continue;
		}
		

		/*get selected excel files*/
		int bCheck;
		EXCELFILELIST  stSelExcels;
		HTREEITEM hChild = m_pTree.GetNextItem(stList[i], TVGN_CHILD);		
		while (NULL != hChild)
		{
			if (m_pTree.GetCheckBox(hChild, bCheck))
			{
				stSelExcels.push_back(m_pTree.GetItemText(hChild));
			}
			hChild = m_pTree.GetNextItem(hChild, TVGN_NEXT);
		}

		
		sInfo.Format("\r\n开始转换[%s]资源文件.......\r\n", pstResData->GetNodeName());
		AddUILog(sInfo.GetBuffer(0));

		if (pstResData->IsMutilTables())
		{
			iRet = stMutilTableTranslator.Translate(&m_stConvTreeParser, pstResData, stSelExcels);
		}else
		{
			iRet = stTranslator.Translate(&m_stConvTreeParser, pstResData, stSelExcels);
		}
		if (0 != iRet)
		{
			i++;
			break;
		}		
		
		m_ctlProgress.SetPos(i+1) ;
	}

    
	AddUILog(sBeginTime);
	
    CString sEndTime = "结束时间:" + CTime::GetCurrentTime().Format( "%Y.%m.%d %H:%M:%S" ) + "\r\n\r\n" ;
	AddUILog(sEndTime);

	AddUILog("批量转化结束，共处理 [%d] ......\r\n\r\n", i) ;

}



void CResConvertDlg::OnSelect() 
{
	HTREEITEM hRoot;
	RESIDLIST stSelectedIDs;
	int i;

	RESSTYLELIST &stStyles = m_stConvTreeParser.GetResStyleManager().GetResStyleList();

	for( i=0; i<m_cStyleList.GetItemCount(); i++ )
	{
		if( m_cStyleList.GetCheck(i) )
		{
			stSelectedIDs.push_back(stStyles[i].m_iID);			
		}
	}

	/*取根节点进行处理*/
	hRoot = m_pTree.GetNextItem(NULL, TVGN_CHILD);
	if (NULL == hRoot)
	{
		return;
	}
	m_pTree.SetCheckBox(hRoot, FALSE);
	
	if( 0 >= (int)stSelectedIDs.size() ) 
	{	/*如果没有选择任何资源，则清空之前的选择*/
		return;
	}

	HTREEITEM hChild;

	hChild = m_pTree.GetNextItem(hRoot, TVGN_CHILD);
	while (NULL != hChild)
	{
		SetTreeTab(hChild, stSelectedIDs);
		hChild = m_pTree.GetNextItem(hChild, TVGN_NEXT);
	}
}



void CResConvertDlg::OnRefresh() 
{
	AnalyzeResConvTree();
}



void CResConvertDlg::GetSelectTab(HTREEITEM hNode, CONVNODELIST &stList)
{
	CCommNode *pstData;
	HTREEITEM hChild;
	int bCheck;

	assert(NULL != hNode);

	pstData = (CCommNode *)m_pTree.GetItemData(hNode);
	if (NULL == pstData)
	{
		return;
	}

	/*资源节点，检查资源ID是否匹配*/
	if (RES_TREE_NODE_TYPE_RES == pstData->GetNodeType())
	{
		if (m_pTree.GetCheckBox(hNode, bCheck))
		{
			stList.push_back(hNode);
		}
	}/*if (RES_TREE_NODE_TYPE_RES == pstData->GetNodeType())*/	

	/*中间节点，继续检查子节点*/
	hChild = m_pTree.GetNextItem(hNode, TVGN_CHILD);
	while (NULL != hChild)
	{
		GetSelectTab(hChild, stList);
		hChild = m_pTree.GetNextItem(hChild, TVGN_NEXT);
	}
}

void CResConvertDlg::SetTreeTab(HTREEITEM hNode, RESIDLIST &stSelectedIDs)
{
	CCommNode *pstData;
	HTREEITEM hChild;

	assert(NULL != hNode);

	pstData = (CCommNode *)m_pTree.GetItemData(hNode);
	if (NULL == pstData)
	{
		return;
	}

	/*中间节点，继续检查子节点*/
	if (RES_TREE_NODE_TYPE_COMM == pstData->GetNodeType())
	{
		hChild = m_pTree.GetNextItem(hNode, TVGN_CHILD);
		while (NULL != hChild)
		{
			SetTreeTab(hChild, stSelectedIDs);
			hChild = m_pTree.GetNextItem(hChild, TVGN_NEXT);
		}
	}/*if (RES_TREE_NODE_TYPE_COMM == pstData->GetNodeType())*/

	/*资源节点，检查资源ID是否匹配*/
	if (RES_TREE_NODE_TYPE_RES == pstData->GetNodeType())
	{
		CResNode *pstResData = (CResNode *)pstData;

		for (int i = 0; i < (int) stSelectedIDs.size(); i++)
		{
			if (pstResData->IsExpectedResStyles(stSelectedIDs[i]))
			{
				m_pTree.SetCheckBox(hNode, TRUE);
				break;
			}
		}/*for (int i = 0; i < (int) stSelectedIDs.size(); i++)*/
	}/*if (RES_TREE_NODE_TYPE_RES == pstData->GetNodeType())*/
	
}


UINT CResConvertDlg::CheckTimerThread(LPVOID lParam)
{
	CResConvertDlg* p=(CResConvertDlg *)lParam;	// this
 
	int nCount = 0 ;
	int nState = 1 ;

	while(nState != 999)
	{
        if ( p->m_bRunTag )
        {
		    p->CloseExcelMsg() ;
        }
	
		Sleep(100) ;
		
		// 检测是否有关闭本线程的信号
		DWORD dwEvent = ::WaitForSingleObject(p->m_hKillThreadEvent, 20);
		if(dwEvent == WAIT_OBJECT_0)  nState = 999;
	}
	
	// 置该线程结束标志
	::SetEvent(p->m_hThreadKilledEvent);

	return 1 ;
}

// 自动关闭 Excel 弹出的提示框
void CResConvertDlg::CloseExcelMsg()
{

	HWND hWnd ,child_hwnd ;
	
	//找到本进程弹出的网页的弹出式对话框
	hWnd = ::FindWindow(NULL,"Microsoft Excel") ;
	if ( hWnd )
	{
        char sWrkBuf[512] = {0} ;
		
		{
			child_hwnd = ::GetWindow(hWnd, GW_CHILD); //找到按钮的hWnd
			::GetWindowText(child_hwnd, sWrkBuf, 100) ;

            if ( strstr(sWrkBuf, "是") )
            {
                char sTmpBuf[512] = {0} ;
                HWND hWndTmp = ::GetWindow(child_hwnd, GW_HWNDNEXT); //找下一个
                hWndTmp = ::GetWindow(hWndTmp, GW_HWNDNEXT); //找下一个
                hWndTmp = ::GetWindow(hWndTmp, GW_HWNDNEXT); //找下一个
                ::GetWindowText(hWndTmp, sTmpBuf, 500) ;

                if ( strstr(sTmpBuf, "保存为 XML 数据表") )
                {
                    ::SendMessage(hWnd,WM_COMMAND, IDYES,NULL) ;  //发送确定消息
                }
                else if ( strstr(sTmpBuf, "其它数据源的链接") )
                {
                    ::SendMessage(hWnd,WM_COMMAND, IDNO,NULL) ;  //发送确定消息
                }

                return ;
            }

            if ( strstr(sWrkBuf, "确定") )
            {
                ::SendMessage(hWnd,WM_COMMAND, IDOK, NULL) ;
                //::SendMessage(hWnd,WM_COMMAND,1,(LPARAM)child_hwnd) ;  //发送确定消息
                return ;
            }  

            // 找第二个按钮 .
            child_hwnd = ::GetWindow(child_hwnd, GW_HWNDNEXT); //找下一个
            ::GetWindowText(child_hwnd, sWrkBuf, 100) ;
            
            if ( strstr(sWrkBuf, "不更新") )
            {
                ::SendMessage(hWnd,WM_COMMAND, IDNO,NULL) ;
                //::SendMessage(hWnd,WM_COMMAND,1,(LPARAM)child_hwnd) ;  //发送确定消息
                return ;
            }    
		}
	}	

}

// 清空
void CResConvertDlg::OnBtnClean() 
{
	m_ctlInfo.SetWindowText("") ;
}

void CResConvertDlg::OnCheckRuntag() 
{
    m_bRunTag =  m_chkRunTag.GetCheck() ;
	if (m_bRunTag == TRUE)
	{
		WLogSetLogLevel(LOG_LEVEL_ERROR);
	}else
	{
		WLogSetLogLevel(LOG_LEVEL_WARN);
	}
}

bool CResConvertDlg::InsertChildsToCtrlTree(HTREEITEM hRoot, CRESTREE &stTree, CRESTREENODE *pstResRoot)
{
	CRESTREENODE *pstResChild =NULL;
	HTREEITEM hChild;
	CCommNode *pstData;

	assert(NULL != pstResRoot);

	pstResChild = stTree.GetNextChild(pstResRoot, NULL);
	while (NULL != pstResChild)
	{
		pstData = pstResChild->GetNodeData();
		if (NULL == pstData)
		{
			pstResChild = stTree.GetNextSibling(pstResChild);
			continue;
		}

		if (RES_TREE_NODE_TYPE_COMM == pstData->GetNodeType())
		{
			hChild = m_pTree.GetNextItem(hRoot, TVGN_CHILD);
			while (NULL != hChild)
			{
				if ((DWORD_PTR)pstData ==m_pTree.GetItemData(hChild))
				{
					break;
				}
				hChild = m_pTree.GetNextItem(hRoot, TVGN_NEXT);
			}
			if (NULL == hChild)
			{
				hChild = m_pTree.InsertCheckBox(pstData->GetNodeName(), hRoot, FALSE, (HTREEDATA)pstData);
			}
			InsertChildsToCtrlTree(hChild, stTree, pstResChild);
		}/*if (RES_TREE_NODE_TYPE_COMM == pstData->GetNodeType())*/

		if (RES_TREE_NODE_TYPE_RES == pstData->GetNodeType())
		{
			CResNode *pstResData = (CResNode *)pstData;
			hChild = m_pTree.InsertCheckBox(pstResData->GetNodeName(), hRoot, FALSE, (HTREEDATA)pstResData);

			/*以excel文件为节点插入子节点*/
			EXCELFILELIST &Excels = pstResData->GetExcelFiles();
			for (int i = 0; i < (int) Excels.size(); i++)
			{
				m_pTree.InsertItem(Excels[i], hChild);				
			}			
		}/*if (RES_TREE_NODE_TYPE_RES == pstData->GetNodeType())*/
		
		pstResChild = stTree.GetNextSibling(pstResChild);
	}/*while (NULL != pstResChild)*/


	return true;
}



int CResConvertDlg::LoadMetalib(CString &szMetalibFile)
{
	int iRet = 0;

	CString szMetalib;
	if (PathIsRelativeA(szMetalibFile))
	{
		szMetalib = GetRootPath() + szMetalibFile;
	}else
	{
		szMetalib = szMetalib;
	}
	
	return iRet;
}
