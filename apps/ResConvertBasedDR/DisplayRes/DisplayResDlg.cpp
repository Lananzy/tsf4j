// DisplayResDlg.cpp : 实现文件
//

#include "stdafx.h"
#include <assert.h>
#include "DisplayRes.h"
#include "DisplayResDlg.h"
#include "../ResConvert/ResBinFile.h"
#include "tdr/tdr.h"
#include "tdr/tdr_metalib_kernel_i.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

inline char *GetEntryCName(LPTDRMETALIB pstLib, LPTDRMETAENTRY pstEntry)
{
	assert(NULL != pstLib);
	assert(NULL != pstEntry);


}

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


// CDisplayResDlg 对话框




CDisplayResDlg::CDisplayResDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDisplayResDlg::IDD, pParent)
	, m_szResBinFile(_T(""))
	, m_szMetalibFile(_T(""))
	, m_szMeta(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
}

void CDisplayResDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RESLIST, m_stResList);
	DDX_Text(pDX, IDC_RESBINFILE, m_szResBinFile);
	DDX_Text(pDX, IDC_METALIBFILE, m_szMetalibFile);
	DDX_Text(pDX, IDC_METANAME, m_szMeta);
}

BEGIN_MESSAGE_MAP(CDisplayResDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_DISPLAYRES, &CDisplayResDlg::OnBnClickedDisplayres)
	ON_BN_CLICKED(IDC_SELECTBINRES, &CDisplayResDlg::OnBnClickedSelectbinres)
	ON_BN_CLICKED(IDC_SELECTMETALIB, &CDisplayResDlg::OnBnClickedSelectmetalib)
END_MESSAGE_MAP()


// CDisplayResDlg 消息处理程序

BOOL CDisplayResDlg::OnInitDialog()
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
	m_stResList.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CDisplayResDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDisplayResDlg::OnPaint()
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
HCURSOR CDisplayResDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDisplayResDlg::OnBnClickedDisplayres()
{
	// TODO: 在此添加控件通知处理程序代码
	/*检查输入变量的合法性*/
	UpdateData();
	if (m_szResBinFile.IsEmpty())
	{
		CButton *pstSelIniFile = (CButton *)GetDlgItem(IDC_RESBINFILE);
		pstSelIniFile->SetFocus();
		return;
	}
	if (m_szMetalibFile.IsEmpty())
	{
		CButton *pstSelStyleFile = (CButton *)GetDlgItem(IDC_METALIBFILE);
		pstSelStyleFile->SetFocus();
		return;
	}
	if (m_szMeta.IsEmpty())
	{
		CButton *pstSelStyleFile = (CButton *)GetDlgItem(IDC_METANAME);
		pstSelStyleFile->SetFocus();
		return;
	}

	// Delete all of the columns and items.

	// Delete all of the items from the list view control.
	m_stResList.DeleteAllItems();
	int nColumnCount = m_stResList.GetHeaderCtrl()->GetItemCount();	
	for (int i=0;i < nColumnCount;i++)
	{
		m_stResList.DeleteColumn(0);
	}
	
	

	LPTDRMETALIB pstMetalib = NULL;
	LPTDRMETA pstMeta = NULL;
	int iRet = 0;
	CString szError;

	iRet = tdr_load_metalib(&pstMetalib, (LPCTSTR)m_szMetalibFile);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		szError.Format("通过元数据描述文件<%s>加载元数据库失败: %s", m_szMetalibFile, tdr_error_string(iRet));
		AfxMessageBox(szError);
		return;
	}
	pstMeta = tdr_get_meta_by_name(pstMetalib, (LPCTSTR)m_szMeta);
	if (NULL == pstMeta)
	{
		szError.Format("在元数据库<%s>中找不到名字为<%s>的资源结构体描述: %s", pstMetalib->szName, m_szMeta);
		AfxMessageBox(szError);
		iRet = -1;
	}
	if (TDR_META_IS_VARIABLE(pstMeta))
	{
		szError.Format("资源结构体<%s>的存储空间不固定,目前资源文件不支持", pstMeta->szName);
		AfxMessageBox(szError);
		iRet = -1;
	}

	/*生成资源标题*/
	if (0 == iRet)
	{
		iRet = GetResTitle(pstMeta);
	}

	/*加载资源*/
	CResBinFile stBinFile;
	if (0 == iRet)
	{
		if( !stBinFile.Open(m_szResBinFile, CFile::modeRead))
		{
			szError.Format(_T("无法打开资源文件 %s 读取信息\r\n"), m_szResBinFile);
			AfxMessageBox(szError);
			iRet = -2;
		}
	}	

	/*读取资源头部*/
	RESHEAD stHead;
	if (0 == iRet)
	{
		iRet = stBinFile.ReadResHead(stHead);
		if (0 != iRet)
		{
			szError.Format(_T("读取资源文件的头部信息失败<ret:%d>,请确定%s为有效的二进制资源文件！\r\n"), iRet, m_szResBinFile);
			AfxMessageBox(szError);
		}	
	}	

	/*分配读取资源信息的内存空间*/
	unsigned char *pszDatabuf = NULL;
	if (0 == iRet)
	{
		try
		{
			pszDatabuf = new unsigned char[stHead.iUnit];
		}
		catch (CException* e)
		{
			szError.Format("Error: 分配内存空间[%d]失败\r\n", stHead.iUnit);
			AfxMessageBox(szError);
			iRet = -3;
		}
	}	

	/*将资源数据输出来*/
	if (0 == iRet)
	{
		for (int i = 1; i <= stHead.iCount; i++)
		{
			iRet = stBinFile.ReadBinData(pszDatabuf, stHead.iUnit);
			if (0 != iRet)
			{
				break;
			}
			iRet = DisPlayOneRes(i, pstMeta, pszDatabuf, stHead.iUnit);
			if (0 != iRet)
			{
				break;
			}
		}/*for (int i = 0; i < stHead.iCount; i++)*/
	}/*if (0 == iRet)*/	

	/*释放资源*/
	stBinFile.Close();
	if (NULL != pszDatabuf)
	{
		delete pszDatabuf;
	}
	tdr_free_lib(&pstMetalib);


}

inline int CDisplayResDlg::GetResTitle(LPTDRMETA pstMeta)
{
	int iRet = 0;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;	
	int iChange;
	LPTDRMETALIB pstLib;
	CString szPreTitle("");
	CString szEntryPath("");
	int iCol = 0;
	int i;
	CString szTmp;

	assert(NULL != pstMeta);

	pstLib = TDR_META_TO_LIB(pstMeta);
	pstCurMeta = pstMeta;
	pstStackTop = &stStack[0];

	pstStackTop->iEntrySizeInfoOff = 0; /*len of pretitle at curmeta*/
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;	
	iStackItemCount = 1;
	pstStackTop->iMetaSizeInfoUnit = 0; 
	pstStackTop->iMetaSizeInfoOff = 0; /*len of szEntryPath*/


	iChange = 0;
	m_stResList.InsertColumn(iCol, "", LVCFMT_LEFT, 40);
	m_stResList.InsertItem(ROW_FOR_RES_TITLE, "No.");
	iCol++;
	while (0 < iStackItemCount)
	{
		LPTDRMETAENTRY pstEntry;		

		if ((0 != iChange) && (pstStackTop->iCount > 0))
		{		
			pstEntry = (LPTDRMETAENTRY)pstStackTop->pszMetaSizeInfoTarget;
			szEntryPath = szEntryPath.Left(pstStackTop->iMetaSizeInfoOff);
			szPreTitle = szPreTitle.Left(pstStackTop->iEntrySizeInfoOff);			
			szPreTitle.AppendFormat("%d", pstEntry->iCount - pstStackTop->iCount + 1);
			szEntryPath.AppendFormat("[%d].", pstEntry->iCount - pstStackTop->iCount + 1);			

			iChange = 0;
			continue;
		}/*if ((0 != iChange) && (pstStackTop->iCount > 0))*/
		iChange = 0;


		if (0 >= pstStackTop->iCount)
		{/*当前元数据数组已经处理完毕*/
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstCurMeta = pstStackTop->pstMeta;			
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
				szPreTitle = szPreTitle.Left(pstStackTop->iEntrySizeInfoOff);
				szEntryPath = szEntryPath.Left(pstStackTop->iMetaSizeInfoOff);
			}			
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;	
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			for (i = 0; i < pstEntry->iCount; i++)
			{				
				m_stResList.InsertColumn(iCol, pstEntry->szName, LVCFMT_LEFT, 100);
				szTmp.Format("%s%s[%d]", szEntryPath, pstEntry->szName, i);
				m_stResList.SetItemText(ROW_FOR_RES_TITLE, iCol, szTmp);
				iCol++;
			}
			
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		
		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{/*复合数据类型*/
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = -2;
				break;
			}


			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			iStackItemCount++;
			pstStackTop++;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = pstEntry->iCount;
			pstStackTop->idxEntry = 0;
			pstStackTop->iMetaSizeInfoUnit = 1;
			pstStackTop->pszMetaSizeInfoTarget = (char *)pstEntry;
			
			szEntryPath.AppendFormat("%s", pstEntry->szName);

			if (TDR_INVALID_PTR != pstEntry->ptrChineseName)
			{
				szPreTitle.AppendFormat("%s", TDR_GET_STRING_BY_PTR(pstLib, pstEntry->ptrChineseName));
			}else
			{
				szPreTitle.AppendFormat("%s", pstEntry->szName);
			}
			
			
			pstStackTop->iMetaSizeInfoOff = szEntryPath.GetLength();
			pstStackTop->iEntrySizeInfoOff = szPreTitle.GetLength();
			if (pstStackTop->iCount > 0)
			{
				szPreTitle += "1";
				szEntryPath += "[0].";
			}else
			{
				szEntryPath += ".";
			}

		}else if (TDR_TYPE_WSTRING >= pstEntry->iType)
		{
			char *pszName;
			if (TDR_INVALID_PTR != pstEntry->ptrChineseName)
			{
				pszName = TDR_GET_STRING_BY_PTR(pstLib,pstEntry->ptrChineseName);
			}else
			{
				pszName = pstEntry->szName;
			}
			if (1 == pstEntry->iCount)
			{	
				szTmp.Format("%s%s", szPreTitle, pszName);
				m_stResList.InsertColumn(iCol, szTmp,LVCFMT_LEFT, 100);
				szTmp.Format("%s%s", szEntryPath, pstEntry->szName, i);
				m_stResList.SetItemText(ROW_FOR_RES_TITLE, iCol, szTmp);
				iCol++;
			}else
			{
				for (i = 0; i < pstEntry->iCount; i++)
				{
					szTmp.Format("%s%s%d", szPreTitle, pszName, i + 1);
					m_stResList.InsertColumn(iCol, szTmp,LVCFMT_LEFT, 100);
					szTmp.Format("%s%s[%d]", szEntryPath, pstEntry->szName, i);
					m_stResList.SetItemText(ROW_FOR_RES_TITLE, iCol, szTmp);
					iCol++;
				}/*for (i = 0; i < pstEntry->iCount; i++)*/
			}/*if (0 ==pstEntry->iCount)*/					
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
		}/*if (TDR_TYPE_COMPOSITE >= pstEntry->iType)*/
	}/*while (0 < iStackItemCount)*/

	UpdateData(false);
	return iRet;
}

void CDisplayResDlg::OnBnClickedSelectbinres()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog fdlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, _T("Bin(*.bin)|*.bin|All Files (*.*)|*.*||"));
	if( fdlg.DoModal()==IDOK )
	{
		m_szResBinFile = fdlg.GetPathName();
		UpdateData(false);
	}
}

void CDisplayResDlg::OnBnClickedSelectmetalib()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog fdlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, _T("tdr(*.tdr)|*.tdr|All Files (*.*)|*.*||"));
	if( fdlg.DoModal()==IDOK )
	{
		m_szMetalibFile = fdlg.GetPathName();
		UpdateData(false);
	}
}

inline int CDisplayResDlg::DisPlayOneRes(int idx, LPTDRMETA pstMeta, unsigned char *pszData, int iDataLen)
{
	int iRet = TDR_SUCCESS;	
	LPTDRMETALIB pstLib;
	LPTDRMETA pstCurMeta;
	TDRSTACK  stStack;
	LPTDRSTACKITEM pstStackTop;
	int iStackItemCount;

	char *pszHostStart;
	char *pszHostEnd;

	int i;
	int iChange = 0;
	int iRow;
	int iCol = 0;
	CString szTmp;

	assert(NULL != pstMeta);
	assert(NULL != pszData);
	assert(0 <= idx);
	assert(0 < iDataLen);

	pszHostStart = (char *)pszData;
	pszHostEnd = (char *)(pszData + iDataLen);
	pstCurMeta = pstMeta;
	pstLib = TDR_META_TO_LIB(pstMeta);

	pstStackTop = &stStack[0];
	pstStackTop->pstMeta = pstCurMeta;
	pstStackTop->pszHostBase = pszHostStart;
	pstStackTop->iCount = 1;
	pstStackTop->idxEntry = 0;
	iStackItemCount = 1;

	szTmp.Format("%d", idx);
	iRow = ROW_FOR_RES_TITLE + idx;
	m_stResList.InsertItem(iRow,szTmp);
	iCol = 1;
	
	while (0 < iStackItemCount)
	{
		LPTDRMETAENTRY pstEntry;
		int iArrayRealCount ;		

		if (0 >= pstStackTop->iCount)
		{
			/*当前元数据数组已经处理完毕*/
			pstStackTop--;
			iStackItemCount--;
			if (0 < iStackItemCount)
			{
				pstCurMeta = pstStackTop->pstMeta;
				TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			}
			continue;
		}

		pstEntry = pstCurMeta->stEntries + pstStackTop->idxEntry;
		if (TDR_ENTRY_IS_POINTER_TYPE(pstEntry) || TDR_ENTRY_IS_REFER_TYPE(pstEntry))
		{
			for (i = 0; i < pstEntry->iCount; i++)
			{				
				m_stResList.SetItemText(iRow, iCol, "0");
				iCol++;
			}
			TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
			continue;
		}

		pszHostStart = pstStackTop->pszHostBase + pstEntry->iHOff;
		iArrayRealCount = pstEntry->iCount;
		
		if (TDR_TYPE_COMPOSITE >= pstEntry->iType)
		{/*复合数据类型*/
			if (TDR_STACK_SIZE <=  iStackItemCount)
			{
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_TOO_COMPLIEX_META);
				break;
			}			

			pstCurMeta = TDR_PTR_TO_META(pstLib, pstEntry->ptrMeta);;
			iStackItemCount++;
			pstStackTop++;
			pstStackTop->pstMeta = pstCurMeta;
			pstStackTop->iCount = iArrayRealCount;
			pstStackTop->idxEntry = 0;
			pstStackTop->pszHostBase = pszHostStart;		
			continue;
		}

		for(i=0; i< iArrayRealCount; i++ )
		{
			int iSize = pstEntry->iHUnitSize;	
			switch( pstEntry->iType )
			{
			case TDR_TYPE_CHAR:
				if( pszHostStart[0]>' ' && pszHostStart[0]<0x7f )
					szTmp.Format("%d[%c] ", (int)(unsigned char)pszHostStart[0], pszHostStart[0]);
				else
					szTmp.Format("%d ", (int)(unsigned char)pszHostStart[0]);
				break;
			case TDR_TYPE_UCHAR:
			case TDR_TYPE_BYTE:
				if( pszHostStart[0]>' ' && pszHostStart[0]<0x7f )
					szTmp.Format("%x[%c] ", (int)(unsigned char)pszHostStart[0], pszHostStart[0]);
				else
					szTmp.Format("%x ", (int)(unsigned char)pszHostStart[0]);
				break;
			case TDR_TYPE_SHORT:
				szTmp.Format("%d ", (int)*(short*)pszHostStart);
				break;
			case TDR_TYPE_USHORT:
				szTmp.Format("%u ", (unsigned int)*(unsigned short*)pszHostStart);
				break;
			case TDR_TYPE_LONG:
			case TDR_TYPE_INT:
				szTmp.Format("%d ", *(int*)pszHostStart);
				break;
			case TDR_TYPE_ULONG:
			case TDR_TYPE_UINT:
				szTmp.Format("%u ", *(int*)pszHostStart);
				break;
			case TDR_TYPE_LONGLONG:
#if _MSC_VER < 1400  /*vc7,vc6,,*/
				szTmp.Format("%I64i ", *(tdr_longlong*)pszHostStart);
#else
				szTmp.Format("%lld ", *(tdr_longlong*)pszHostStart);
#endif
				break;
			case TDR_TYPE_ULONGLONG:
#if  _MSC_VER < 1400  /*vc7,vc6,,*/
				szTmp.Format("%I64u ", *(tdr_ulonglong*)pszHostStart);
#else
				szTmp.Format("%llu ", *(tdr_ulonglong*)pszHostStart);
#endif
				break;			
			case TDR_TYPE_FLOAT:
				szTmp.Format("%f ", *(float*)pszHostStart);
				break;
			case TDR_TYPE_DOUBLE:
				szTmp.Format("%f ", *(double*)pszHostStart);
				break;
			case TDR_TYPE_DATETIME:
				szTmp.Format("%s", tdr_tdrdatetime_to_str((tdr_datetime_t *)pszHostStart));
				break;
			case TDR_TYPE_DATE:
				szTmp.Format("%s ", tdr_tdrdate_to_str((tdr_date_t *)pszHostStart));
				break;
			case TDR_TYPE_TIME:
				szTmp.Format("%s ", tdr_tdrtime_to_str((tdr_time_t *)pszHostStart));
				break;
			case TDR_TYPE_IP:
				szTmp.Format("%s ", tdr_tdrip_to_ineta(*(tdr_ip_t *)pszHostStart));
				break;
			case TDR_TYPE_STRING:
				{
					if (0 < pstEntry->iCustomHUnitSize)												
					{																					
						iSize = pstEntry->iCustomHUnitSize;											
					}else																				
					{																					
						iSize = pszHostStart - pszHostEnd;											
					}	
					szTmp = pszHostStart;
					break;
				}	
			case TDR_TYPE_WSTRING:
				{
					if (0 < pstEntry->iCustomHUnitSize)												
					{																					
						iSize = pstEntry->iCustomHUnitSize;											
					}else																				
					{																					
						iSize = pszHostStart - pszHostEnd;											
					}
					szTmp = "NULL";
					break;
				}
			default:	/* must be 8 bytes. */
				szTmp = "0";
				break;
			}/*switch( pstEntry->iType )*/

			m_stResList.SetItemText(iRow, iCol, szTmp);
			iCol++;
			pszHostStart += iSize;			
		}/*for(i=0; i< iArrayRealCount; i++ )*/
		
		TDR_GET_NEXT_ENTRY(pstStackTop, pstCurMeta, iChange);
		
	}/*while (0 < iStackItemCount)*/

	return iRet;
}
