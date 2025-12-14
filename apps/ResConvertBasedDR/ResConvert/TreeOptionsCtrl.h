/*
Module : TreeOptionsCtrl.H
Purpose: Defines the interface for an MFC class to implement a tree options control 
         similiar to the advanced tab as seen on the "Internet options" dialog in 
         Internet Explorer 4 and later
Created: PJN / 31-03-1999

Copyright (c) 1999 by PJ Naughter.  
All rights reserved.

Modified£º20-09-2007
Modifier:	jackyai
*/


/////////////////////////////// Defines ///////////////////////////////////////
#ifndef __TREEOPTIONSCTRL_H__
#define __TREEOPTIONSCTRL_H__

typedef unsigned int	HTREEDATA;
/////////////////////////////// Classes ///////////////////////////////////////

//The actual tree options control class
class CTreeOptionsCtrl : public CTreeCtrl
{
public:
//Constructors / Destructors
  CTreeOptionsCtrl();

//Misc
  void SetImageListToUse(UINT nResourceID) { m_nilID = nResourceID; };


//Inserting items into the control
  HTREEITEM InsertGroup( CString sText, HTREEITEM hParent = NULL, HTREEDATA hData = NULL );
  HTREEITEM InsertCheckBox( CString sText, HTREEITEM hParent, BOOL bCheck = FALSE , HTREEDATA hData= NULL);

//Validation methods
  BOOL IsCheckBox(HTREEITEM hItem);

//Setting / Getting combo states
  BOOL SetCheckBox(HTREEITEM hItem, BOOL bCheck);
  BOOL GetCheckBox(HTREEITEM hItem, int& bCheck);

//Enable / Disbale items
  BOOL SetGroupEnable(HTREEITEM hItem, BOOL bEnable);
  BOOL SetCheckBoxEnable(HTREEITEM hItem, BOOL bEnable);
  BOOL GetCheckBoxEnable(HTREEITEM hItem, BOOL& bEnable);

private:
	void SetParentCheckBox(HTREEITEM hItem, BOOL bCheck);
	void SetChildCheckBox(HTREEITEM hItem, BOOL bCheck);

protected:
	CImageList m_ilTree;
	UINT m_nilID;

	//{{AFX_VIRTUAL(CTreeOptionsCtrl)
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTreeOptionsCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_DYNAMIC(CTreeOptionsCtrl)

	DECLARE_MESSAGE_MAP()
};


#endif //__TREEOPTIONSCTRL_H__



