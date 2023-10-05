// DlgTreeList.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"
#include "DlgTreeList.h"
#include "afxdialogex.h"


// CDlgTreeList dialog

IMPLEMENT_DYNAMIC( CDlgTreeList, CDialogEx )


CDlgTreeList::CDlgTreeList( CWnd *pParent /*=NULL*/ )
	: CDialogEx( CDlgTreeList::IDD, pParent )
{

}


CDlgTreeList::~CDlgTreeList()
{
}


void CDlgTreeList::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_BUTSELECT, m_ButSelectAll );
	//  DDX_Control( pDX, IDC_BUTUNSELECT, M_ButUnSelectAll );
	DDX_Control( pDX, IDC_TREE, m_TreeList );
	DDX_Control( pDX, IDC_BUTUNSELECT, m_ButUnSelectAll );
}


BEGIN_MESSAGE_MAP( CDlgTreeList, CDialogEx )
	ON_BN_CLICKED( IDC_BUTSELECT, &CDlgTreeList::OnBnClickedButselect )
	ON_BN_CLICKED( IDC_BUTUNSELECT, &CDlgTreeList::OnBnClickedButunselect )
	ON_NOTIFY( NM_CLICK, IDC_TREE, OnNMClickTree )
	ON_MESSAGE( WM_USER_CHECKSTATECHANGE, OnCheckStateChange )
END_MESSAGE_MAP()


// CDlgTreeList message handlers



BOOL CDlgTreeList::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CDialogEx::OnInitDialog();
	CString str;
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );


	// Load button images.
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );

	if( NULL != pclImgListButton )
	{
		m_ButSelectAll.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILB_SelectAll ) );
		m_ButUnSelectAll.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILB_UnselectAll ) );
	}
	else
	{
		ASSERT( 0 );
	}

	// ToolTips.
	m_ToolTip.Create( this, TTS_NOPREFIX );
	CString TTstr;
	TTstr = TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTSELECT );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTSELECT ), TTstr );
	TTstr = TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTUNSELECT );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTUNSELECT ), TTstr );

	SetWindowText( ( LPCTSTR )m_Title );

	m_TreeList.DeleteAllItems();

	for( int i = 0; i < m_StrDataAr.GetCount(); i++ )
	{
		m_TreeList.InsertItem( ( LPCTSTR )m_StrDataAr.GetAt( i ).GetStr(), NULL );
	}

	GetDlgItem( IDOK )->EnableWindow( FALSE );

	return TRUE;  
}



int CDlgTreeList::Display( CString Title )
{
	m_Title = Title;
	return DoModal();
}


int CDlgTreeList::Display( int IDTitle )
{
	m_Title = TASApp.LoadLocalizedString( IDTitle );
	return DoModal();
}

int CDlgTreeList::_TreeSelect( HTREEITEM hItem, BOOL bCheck, bool fSibling, bool fChildren )
{
	if( NULL == hItem )
	{
		return 0;
	}

	m_TreeList.SetCheck( hItem, bCheck );

	if( m_TreeList.ItemHasChildren( hItem ) && true == fChildren )
	{
		HTREEITEM hChildItem = m_TreeList.GetChildItem( hItem );
		_TreeSelect( hChildItem, bCheck, true, true );
	}

	if( true == fSibling )
	{
		HTREEITEM hNextItem = m_TreeList.GetNextSiblingItem( hItem );
		_TreeSelect( hNextItem, bCheck, true, fChildren );
	}

	return 0;
}



void CDlgTreeList::OnOK()
{
	// Check Selected Item
	HTREEITEM hItem = m_TreeList.GetRootItem();

	while( NULL != hItem )
	{
		for( int i = 0; i < m_StrDataAr.GetCount(); i++ )
		{
			if( 0 == wcscmp( m_TreeList.GetItemText( hItem ),m_StrDataAr.GetAt(i).GetStr() ) && FALSE == m_TreeList.GetCheck( hItem ) )
			{
				m_StrDataAr.RemoveAt( i );
			}
		}

		HTREEITEM hNextItem = m_TreeList.GetNextSiblingItem( hItem );
		hItem = hNextItem;
	}

	CDialogEx::OnOK();
}



void CDlgTreeList::OnCancel()
{
	m_StrDataAr.RemoveAll();
	CDialogEx::OnCancel();
}



void CDlgTreeList::OnBnClickedButselect()
{
	HTREEITEM hRI = m_TreeList.GetRootItem();
	_TreeSelect( hRI, TRUE, true, true );
}



void CDlgTreeList::OnBnClickedButunselect()
{
	HTREEITEM hRI = m_TreeList.GetRootItem();
	_TreeSelect( hRI, FALSE, true, true );
}


void CDlgTreeList::Add( CString str, CString data )
{
	m_StrDataAr.Add( CStrData( str, data ) );
}

void CDlgTreeList::OnNMClickTree( NMHDR *pNMHDR, LRESULT *pResult )
{
	// On a TreeView Control with the CheckBoxes, there is no notification that the check state
	// of the item has been changed, you can just determine that the user has clicked the state Icon.
	// The TreeView Control toggles the state AFTER the click notification is sent, the solution is 
	// post a user defined message as a notification that the check state has changed.
	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x = ( (int)(short)LOWORD( dwpos ) );
	ht.pt.y = ( (int)(short)HIWORD( dwpos ) );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );

	if( ( TVHT_ONITEMSTATEICON & ht.flags ) || ( TVHT_ONITEM & ht.flags ) )
	{
		// Save current state of item in WPARAM.
		WPARAM wpState = m_TreeList.GetCheck( ht.hItem );
		::PostMessage( m_hWnd, WM_USER_CHECKSTATECHANGE, wpState, (LPARAM)ht.hItem );
	}
	*pResult = 0;
}

LRESULT CDlgTreeList::OnCheckStateChange( WPARAM wParam, LPARAM lParam )
{
	GetDlgItem( IDOK )->EnableWindow( FALSE );
	// Check Selected Item
	HTREEITEM hItem = m_TreeList.GetRootItem();

	while( NULL != hItem )
	{
		// At least one Item checked
		if ( 0 != m_TreeList.GetCheck( hItem ) )
		{
			GetDlgItem( IDOK )->EnableWindow(TRUE);
			break;
		}			

		HTREEITEM hNextItem = m_TreeList.GetNextSiblingItem( hItem );
		hItem = hNextItem;
	}

	return 0;
}
