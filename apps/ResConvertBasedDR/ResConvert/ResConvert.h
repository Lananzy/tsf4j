// ResConvert.h : main header file for the RESCONVERT application
//

#if !defined(AFX_RESCONVERT_H__3889FD3D_1694_4686_B415_293F1BB0DE67__INCLUDED_)
#define AFX_RESCONVERT_H__3889FD3D_1694_4686_B415_293F1BB0DE67__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CResConvertApp:
// See ResConvert.cpp for the implementation of this class
//

class CResConvertApp : public CWinApp
{
public:
	CResConvertApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResConvertApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CResConvertApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESCONVERT_H__3889FD3D_1694_4686_B415_293F1BB0DE67__INCLUDED_)
