#include "stdafx.h"
#include "TASelect.h"
#include "DlgCtrlPropTree.h"
#include "DlgTPPageHC.h"


IMPLEMENT_DYNAMIC( CDlgCtrlPropTree, CDlgCtrlPropPageMng )

CDlgCtrlPropTree::CDlgCtrlPropTree( CWnd *pParent )
	: CDlgCtrlPropPageMng( CDlgCtrlPropTree::IDD, pParent )
{
	m_iMargin = 6;
	m_iHeaderHeight = 32;
	m_pCurPage = NULL;
	m_bAllowNewSelectionTree = true;
}

BEGIN_MESSAGE_MAP( CDlgCtrlPropTree, CDlgCtrlPropPageMng )
	ON_BN_CLICKED( IDOK, OnBnClickedOk )
	ON_BN_CLICKED( IDCANCEL, OnBnClickedCancel )
	ON_NOTIFY( NM_CLICK, IDC_TREECTRLPROP, OnNMClickTreectrlprop )
	ON_NOTIFY( TVN_SELCHANGED, IDC_TREECTRLPROP, OnTvnSelchangedTreectrlprop )
END_MESSAGE_MAP()

void CDlgCtrlPropTree::DoDataExchange( CDataExchange *pDX )
{
	CDlgCtrlPropPageMng::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_TREECTRLPROP, m_Tree );
}

BOOL CDlgCtrlPropTree::OnInitDialog()
{
	if( FALSE == CDlgCtrlPropPageMng::OnInitDialog() )
	{
		return FALSE;
	}

	// Find the position to place the current page.
	// The upper-left corner is found with the  coordinates for the tree and the lower-right corner is found with the coordinates for the 
	// CANCEL button.
	CRect rect;
	m_Tree.GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_PageRect.top = rect.top;
	m_PageRect.left = rect.right + m_iMargin;

	GetDlgItem( IDCANCEL )->GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_PageRect.bottom = rect.top - m_iMargin;
	m_PageRect.right = rect.right;

	return true;
}

void CDlgCtrlPropTree::OnBnClickedOk()
{
	OnOK();
}

void CDlgCtrlPropTree::OnBnClickedCancel()
{
	OnCancel();
}

void CDlgCtrlPropTree::OnNMClickTreectrlprop( NMHDR *pNMHDR, LRESULT *pResult )
{
	// Verify the consistence of informations on the previous page's edit controls.
	if( false == m_pCurPage->VerifyInfo() )
	{
		m_bAllowNewSelectionTree = false;
		return;
	}
	
	m_bAllowNewSelectionTree = true;

	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x= ( (int)(short)LOWORD( dwpos ) );		//GET_X_LPARAM(dwpos);
	ht.pt.y= ( (int)(short)HIWORD( dwpos ) );		//GET_Y_LPARAM(dwpos);

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );

	if( ht.hItem != NULL && ( TVHT_ONITEM == ( TVHT_ONITEM & ht.flags ) ) )
	{
		m_Tree.Expand( ht.hItem, TVE_EXPAND );
	}

	*pResult = 0;
}

void CDlgCtrlPropTree::OnTvnSelchangedTreectrlprop( NMHDR *pNMHDR, LRESULT *pResult )
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>( pNMHDR );
	
	// Get back to the previous selection if one parameter was not correctly integrated.
	if( false == m_bAllowNewSelectionTree && pNMTreeView->action > 0 )
	{
		m_Tree.SelectItem( pNMTreeView->itemOld.hItem );
	}
	else if( true == m_bAllowNewSelectionTree && pNMTreeView->action > 0 )
	{
		DWORD_PTR DWptr = 0;
		DWptr = m_Tree.GetItemData( pNMTreeView->itemNew.hItem );
		
		// Display the page according to the selection.
		CDlgCtrlPropPage *pPage = dynamic_cast<CDlgCtrlPropPage *>( (CDlgCtrlPropPage *)DWptr );
	
		// HYS-1221 : When a page is activated update combo regarding the value of checkbox that
		// allows building project with deleted products
		CDlgTPPageHC *pPageProduct = dynamic_cast< CDlgTPPageHC *>( pPage );
		
		if( NULL != pPageProduct )
		{
			pPage->UpdateComboWithDeleted();
		}
		
		if( pPage != NULL )
		{
			DisplayPage( pPage );
		}
	}
	
	*pResult = 0;
}

bool CDlgCtrlPropTree::AddPageToTree( CDlgCtrlPropPage *pTreePage, HTREEITEM *pItemAdded )
{
	HTREEITEM hItem = NULL;
	hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, pTreePage->GetPageName(), 0, 0, 0, 0, (LPARAM)pTreePage, NULL, NULL );
	
	if( pItemAdded != NULL )
	{
		*pItemAdded = hItem;
	}
	
	return ( ( hItem != NULL ) ? true : false );
}

bool CDlgCtrlPropTree::AddChildPageToTree( CDlgCtrlPropPage *pTreePageParent, CDlgCtrlPropPage *pTreePage, HTREEITEM *pItemAdded )
{
	CString str = pTreePage->GetPageName();
	HTREEITEM hItem = NULL;
	HTREEITEM hParentItem = NULL;
	
	for( hParentItem = m_Tree.GetFirstVisibleItem(); hParentItem != NULL ; hParentItem = m_Tree.GetNextVisibleItem( hParentItem ) )
	{
		DWORD_PTR DWptr = m_Tree.GetItemData( hParentItem );
		CDlgCtrlPropPage *pPropPageParent = (CDlgCtrlPropPage *)( DWptr );
		ASSERT( NULL != pPropPageParent );

		if( pPropPageParent == pTreePageParent )
		{
			break;	
		}
	}
	
	if( hParentItem != NULL )
	{
		hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_PARAM, str, 0, 0, 0, 0, (LPARAM)pTreePage, hParentItem, NULL );
	}

	if( pItemAdded != NULL )
	{
		*pItemAdded = hItem;
	}
	
	return ( ( hItem != NULL ) ? true : false );
}
