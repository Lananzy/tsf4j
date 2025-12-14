/*
Module : TreeOptionsCtrl.cpp
Purpose: Implementation for an MFC class to implement a tree options control 
         similiar to the advanced tab as seen on the "Internet options" dialog in 
         Internet Explorer 4 and later

Created: PJN / 31-03-1999
History: PJN / 21-04-1999 Added full support for enabling / disabling all the item types
         PJN / 05-10-1999 Made class more self contained by internally managing the image list
         PJN / 07-10-1999 1. Added support for including combo boxes aswell as edit boxes into the
                          edit control.
                          2. Added support for customizing the image list to use
         PJN / 29-02-2000 Removed a VC 6 level 4 warning
                          
Copyright (c) 1999 by PJ Naughter.  
All rights reserved.
*/

//////////////// Includes ////////////////////////////////////////////
#include "stdafx.h"
#include "resource.h"
#include "TreeOptionsCtrl.h"


//////////////// Macros / Locals /////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const UINT TREE_OPTIONS_COMBOBOX_ID = 100;
const UINT TREE_OPTIONS_EDITBOX_ID = 100;



//////////////// Implementation //////////////////////////////////////
IMPLEMENT_DYNAMIC(CTreeOptionsCtrl, CTreeCtrl)

BEGIN_MESSAGE_MAP(CTreeOptionsCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CTreeOptionsCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CTreeOptionsCtrl::CTreeOptionsCtrl()
{
	m_nilID = IDB_TREE_CTRL_OPTIONS;
}

void CTreeOptionsCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	UINT uFlags=0;
	HTREEITEM hItem = HitTest(point, &uFlags);
	
	//If the mouse was over the label, icon or to the left or right of the item ?
	//|| (uFlags & TVHT_ONITEMRIGHT)
	if ( (uFlags & TVHT_ONITEM) || (uFlags & TVHT_ONITEMINDENT) )
	{
		if (IsCheckBox(hItem))
		{
			BOOL bEnable;
			if ( GetCheckBoxEnable(hItem, bEnable) )
			{
				//Toggle the state of check items
				int bCheck;
				BOOL bSet;
				bSet = GetCheckBox(hItem, bCheck);
				VERIFY(SetCheckBox(hItem, !bSet));
			}
		}
		//Pass on to the parent now that we that we have done our stuff
		CTreeCtrl::OnLButtonDown(nFlags, point);
	}
	else
	{
		//Pass on to the parent since we didn't handle it
		CTreeCtrl::OnLButtonDown(nFlags, point);
	}
}

void CTreeOptionsCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
 	if (nChar == VK_SPACE)
	{
		HTREEITEM hItem = GetSelectedItem();
		
		//If the space key is hit, and the item is a combo item, then toggle the check value
		if( IsCheckBox(hItem) )
		{
			BOOL bEnable;
			if( GetCheckBoxEnable(hItem, bEnable) )
			{
				int bCheck;
				BOOL bSet;
				bSet = GetCheckBox(hItem, bCheck);
				VERIFY(SetCheckBox(hItem, !bSet));
			}
			else
			{
				//Pass on to the parent since we didn't handle it
				CTreeCtrl::OnChar(nChar, nRepCnt, nFlags);
			}
		}
		else
		{
			//Pass on to the parent since we didn't handle it
			CTreeCtrl::OnChar(nChar, nRepCnt, nFlags);
		}
	}
	else
	{
		//Pass on to the parent since we didn't handle it
		CTreeCtrl::OnChar(nChar, nRepCnt, nFlags);
	}
}

HTREEITEM CTreeOptionsCtrl::InsertGroup( CString sText, HTREEITEM hParent, HTREEDATA hData )
{
	HTREEITEM hItem = InsertItem((LPCTSTR)sText, 9, 9, hParent);
	SetItemData(hItem, hData);	
	return hItem;
}

HTREEITEM CTreeOptionsCtrl::InsertCheckBox( CString sText, HTREEITEM hParent, BOOL bCheck , HTREEDATA hData )
{
	HTREEITEM hItem = InsertItem((LPCTSTR)sText, 0, 0, hParent);
	SetItemData(hItem, hData);
	
	SetCheckBox(hItem, bCheck);
	return hItem;
}

BOOL CTreeOptionsCtrl::IsCheckBox(HTREEITEM hItem)
{
	int nImage;
	int nSelectedImage;
	GetItemImage(hItem, nImage, nSelectedImage);
	
	return(nImage == 0 || nImage == 1 || nImage ==2 );
}

BOOL CTreeOptionsCtrl::SetCheckBox(HTREEITEM hItem, BOOL bCheck)
{
	//Validate our parameters
	ASSERT(IsCheckBox(hItem)); //Must be a combo item to check it
	
	BOOL bSuccess = TRUE;
	if(bCheck)
		SetItemImage(hItem, 1, 1);
	else
		SetItemImage(hItem, 0, 0);

	SetChildCheckBox(hItem, bCheck);
	SetParentCheckBox(hItem, bCheck);
	
	return bSuccess;
}

BOOL CTreeOptionsCtrl::GetCheckBox(HTREEITEM hItem, int& bCheck)
{
	//Validate our parameters
//	ASSERT(IsCheckBox(hItem)); //Must be a combo item to check it
	
	
	int nImage;
	int nSelectedImage;
	BOOL bSet;
	GetItemImage(hItem, nImage, nSelectedImage);
	
	if( nImage == 1 )
		bCheck = 1;
	else if( nImage == 2 )
		bCheck = 2;
	else
		bCheck = 0;

	bSet = ( nImage == 1 || nImage == 2 );
	return bSet;
}

BOOL CTreeOptionsCtrl::SetGroupEnable(HTREEITEM hItem, BOOL bEnable)
{
	//Allows you to quickly enable / disable all the items in a group
//	ASSERT(IsGroup(hItem)); //Must be group item

	//Iterate through the child items and enable / disable all the items
	HTREEITEM hChild = GetNextItem(hItem, TVGN_CHILD);

	//Turn of redraw to Q all the changes we're going to make here
	SetRedraw(FALSE);

	while (hChild)
	{
		if(IsCheckBox(hChild))
			VERIFY(SetCheckBoxEnable(hChild, bEnable));

		//Move onto the next child
		hChild = GetNextItem(hChild, TVGN_NEXT);
	}

	//Reset the redraw flag
	SetRedraw(TRUE);

	return TRUE;
}

BOOL CTreeOptionsCtrl::SetCheckBoxEnable(HTREEITEM hItem, BOOL bEnable)
{
	ASSERT(IsCheckBox(hItem)); //Must be a check box
	BOOL bSuccess = FALSE;

	if (bEnable)
	{
		int bCheck;		
		if( GetCheckBox(hItem, bCheck) )
			bSuccess = SetItemImage(hItem, 1, 1);
		else
			bSuccess = SetItemImage(hItem, 0, 0);
	}
	else
	{
		int bCheck;
		if ( GetCheckBox(hItem, bCheck) )
			bSuccess = SetItemImage(hItem, 5, 5);
		else
			bSuccess = SetItemImage(hItem, 4, 4);
	}

	return bSuccess;
}

BOOL CTreeOptionsCtrl::GetCheckBoxEnable(HTREEITEM hItem, BOOL& bEnable)
{
	ASSERT(IsCheckBox(hItem)); //Must be a check box

	int nImage;
	int nSelectedImage;
	BOOL bSuccess = GetItemImage(hItem, nImage, nSelectedImage);

	bEnable = (nImage == 0 || nImage == 1);
	return bSuccess;  
}

void CTreeOptionsCtrl::PreSubclassWindow() 
{
	//Let the parent class do its thing	
	CTreeCtrl::PreSubclassWindow();

	//Loadup the image list
	VERIFY(m_ilTree.Create(m_nilID, 16, 1, RGB(255, 0, 255)));

	//Hook it up to the tree control
	SetImageList(&m_ilTree, TVSIL_NORMAL);
}

void CTreeOptionsCtrl::SetChildCheckBox(HTREEITEM hItem, BOOL bCheck)
{
	HTREEITEM hChild;
	if( bCheck )
	{		
		hChild = GetNextItem(hItem, TVGN_CHILD);
		while( hChild!=NULL )
		{
			SetItemImage(hChild, 1, 1);
			SetChildCheckBox(hChild, bCheck);
			hChild = GetNextItem(hChild, TVGN_NEXT);
		}
	}
	else
	{
		hChild = GetNextItem(hItem, TVGN_CHILD);
		while( hChild!=NULL )
		{
			SetItemImage(hChild, 0, 0);
			SetChildCheckBox(hChild, bCheck);
			hChild = GetNextItem(hChild, TVGN_NEXT);
		}
	}
}

void CTreeOptionsCtrl::SetParentCheckBox(HTREEITEM hItem, BOOL bCheck)
{
	HTREEITEM hParent, hChild;
	int bSet;
	int flagt = 0, flags = 0;

		hParent = GetNextItem(hItem, TVGN_PARENT);
		if( hParent )
		{
			hChild = GetNextItem(hParent, TVGN_CHILD);
			while( hChild )
			{
				GetCheckBox(hChild, bSet);

				if(bSet==2)
				{
					flagt = 2;
					break;
				}
				else if(bSet==1)
					flagt = 1;
				else
					flags = 1;
				hChild = GetNextItem(hChild, TVGN_NEXT);
			}

			if( flagt==2 )
				SetItemImage(hParent, 2, 2);
			else if( flagt==0 )
				SetItemImage(hParent, 0, 0);
			else if( flags==0 )
				SetItemImage(hParent, 1, 1);
			else
				SetItemImage(hParent, 2, 2);
			
			SetParentCheckBox(hParent, bCheck);
		}
}
