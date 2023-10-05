#include "stdafx.h" 
#include "WinUser.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "HydroMod.h"
#include "DlgLeftTabProject.h"
#include "ProjTreeCtrl.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__; 
#endif

CProjTreeCtrl::CProjTreeCtrl()
{
	m_hDragItem = NULL; 
	m_pImageList = NULL;
	m_fDragging = FALSE;
	m_iDelayInterval = 500;     // Default delay interval = 500 milliseconds
	m_iScrollInterval = 200;    // Default scroll interval = 200 milliseconds
	m_iScrollMargin = 10;       // Default scroll margin = 10 pixels
	m_nTimer = (UINT_PTR)0;
	m_vechItemCut.clear();
}

CProjTreeCtrl::~CProjTreeCtrl() 
{   
	// Delete the image list created by CreateDragImage.
	if( NULL != m_pImageList )
		delete m_pImageList;
}

CMainFrame::RightViewList CProjTreeCtrl::GetRightViewAt( HTREEITEM hItem )
{
	POSITION pos = (POSITION)GetItemData( hItem );
	if( NULL != pos )
		return m_List.GetAt( pos ).eRightView;
	else
		return CMainFrame::RightViewList::eRVFirst;
}

void* CProjTreeCtrl::GetObjectAt( HTREEITEM hItem )
{
	if( NULL == hItem)
		return NULL;
	
	POSITION pos = (POSITION)GetItemData( hItem );
	if( pos != NULL )
		return ( (void *)m_List.GetAt( pos ).pObject );
	else
		return NULL;
}

POSITION CProjTreeCtrl::AddToNodeList( CMainFrame::RightViewList eRightView, void* pObject )
{
	STreeNode TreeNode; 
	TreeNode.pObject = pObject; 
	TreeNode.eRightView = eRightView;
	POSITION Pos = m_List.AddTail( TreeNode );
	return Pos; 
}

void CProjTreeCtrl::SetItemCut( HTREEITEM hTreeItem )
{
	SetItemState( hTreeItem, TVIS_CUT, TVIS_CUT );
	m_vechItemCut.push_back( hTreeItem );
}

void CProjTreeCtrl::ResetItemCut( HTREEITEM hTreeItem )
{
	if( NULL != hTreeItem )
	{
		if( true == IsItemCut( hTreeItem ) )
		{
			SetItemState( hTreeItem, 0, TVIS_CUT );
			m_vechItemCut.erase( std::find( m_vechItemCut.begin(), m_vechItemCut.end(), hTreeItem ) );
		}
	}
	else
	{
		// Reset all.
		for( int iLoop = 0; iLoop < (int)m_vechItemCut.size(); iLoop++ )
			SetItemState( m_vechItemCut[iLoop], 0, TVIS_CUT );

		m_vechItemCut.clear();
	}
}

bool CProjTreeCtrl::IsItemCut( HTREEITEM hTreeItem )
{
	return ( TVIS_CUT == ( TVIS_CUT & GetItemState( hTreeItem, TVIS_CUT ) ) ) ? true : false;
}

bool CProjTreeCtrl::IsAtLeastOneItemCut( void )
{
	return ( 0 == (int)m_vechItemCut.size() ) ? false : true;
}

BEGIN_MESSAGE_MAP(CProjTreeCtrl, CTreeCtrl) 
	ON_WM_PAINT()
	ON_NOTIFY_REFLECT( TVN_BEGINDRAG, OnBeginDrag )
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CProjTreeCtrl::PreCreateWindow(CREATESTRUCT& cs)   
{
	// Make sure the control's TVS_DISABLEDRAGDROP flag is not set.
	// If you subclass an existing tree view control rather than create
	// a CProjTreeCtrl outright, it's YOUR responsibility to see that
	// this flag isn't set.
	cs.style &= ~TVS_DISABLEDRAGDROP;
	return  CTreeCtrl::PreCreateWindow( cs );
}

void CProjTreeCtrl::OnPaint()
{
	if( 0 == (int)m_vechItemCut.size() )
		CTreeCtrl::OnPaint();
	else
	{
		CPaintDC dc( this );

		// Create a memory DC compatible with the paint DC.
		CDC memDC;
		memDC.CreateCompatibleDC( &dc );

		CRect rcClip, rcClient;
		dc.GetClipBox( &rcClip );
		GetClientRect( &rcClient );

		// Select a compatible bitmap into the memory DC.
		CBitmap bitmap;
		bitmap.CreateCompatibleBitmap( &dc, rcClient.Width(), rcClient.Height() );
		memDC.SelectObject( &bitmap );
	
		// Set clip region to be same as that in paint DC.
		CRgn rgn;
		rgn.CreateRectRgnIndirect( &rcClip );
		memDC.SelectClipRgn( &rgn );
		rgn.DeleteObject();
	
		// First let the control do its default drawing.
		CWnd::DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );

		HTREEITEM hItem = GetFirstVisibleItem();

		int iItemCount = GetVisibleCount() + 1;
		while( NULL != hItem && iItemCount-- )
		{		
			// Is item is in the vector?
			bool fFound = false;
			for( int iLoopCutItem = 0; iLoopCutItem < (int)m_vechItemCut.size() && false == fFound; iLoopCutItem++ )
			{
				if( hItem == m_vechItemCut[iLoopCutItem ])
					fFound = true;
			}

			if( true == fFound )
			{
				// Use window font
				CFont *pFont = GetFont();
				LOGFONT logfont;
				pFont->GetLogFont( &logfont );

				CFont fontDC;
				fontDC.CreateFontIndirect( &logfont );
				CFont *pFontDC = memDC.SelectObject( &fontDC );

				memDC.SetTextColor( _TAH_BLACK_LIGHT );
				CString sItem = GetItemText( hItem );

				CRect rect;
				GetItemRect( hItem, &rect, TRUE );
				memDC.SetBkColor( GetSysColor( COLOR_WINDOW ) );
				memDC.TextOut( rect.left + 2, rect.top + 1, sItem );
			
				memDC.SelectObject( pFontDC );
			}

			hItem = GetNextVisibleItem(hItem);
		}

		dc.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), &memDC, 
					rcClip.left, rcClip.top, SRCCOPY );

		memDC.DeleteDC();
	}
}

void CProjTreeCtrl::OnBeginDrag( NMHDR* pNMHDR, LRESULT* pResult )
{
	*pResult = 0;
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*) pNMHDR;
 
	// Do nothing if the user is attempting to drag a something different than a hydromod.
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	if( CMainFrame::RightViewList::eRVHMCalc != GetRightViewAt( hItem ) )
		return;
 
	// Create a drag image. If the assertion fails, you probably forgot to assign an image list to the control with SetImageList.
	// CreateDragImage will not work if the control hasn't been assigned an image list!
	m_pImageList = CreateDragImage( hItem );                                    ASSERT( m_pImageList != NULL );
 
	if( m_pImageList != NULL )
	{ 
		// Compute the coordinates of the "hot spot"--the location of the cursor relative to the upper left corner of the item rectangle.  
		CRect rect;
		GetItemRect( hItem, rect, TRUE );
		CPoint point( pNMTreeView->ptDrag.x, pNMTreeView->ptDrag.y );
		CPoint hotSpot = point;
		hotSpot.x -= rect.left;
		hotSpot.y -= rect.top;  
 
		// Convert the client coordinates in "point" to coordinates relative to the upper left corner of the control window. 
		CPoint client( 0, 0 );
		ClientToScreen( &client );
		GetWindowRect( rect );
		point.x += client.x - rect.left;
		point.y += client.y - rect.top;
 
		// Capture the mouse and begin dragging.
		SetCapture(); 
		m_pImageList->BeginDrag( 0, hotSpot );
		m_pImageList->DragEnter( this, point );
		m_hDragItem = hItem;
		m_fDragging = TRUE;
	}
}
 
void CProjTreeCtrl::OnMouseMove( UINT nFlags, CPoint point ) 
{
	CTreeCtrl::OnMouseMove( nFlags, point );
 
	if( TRUE == m_fDragging && m_pImageList != NULL )
	{ 
		// Stop the scroll timer if it's running.
		KillTimer( m_nTimer );
		m_nTimer = (UINT_PTR)0;
 
		// Erase the old drag image and draw a new one.
		m_pImageList->DragMove( point );
 
		// Highlight the drop target if the cursor is over a valid item, check if drop target is valid, if it's an existing hydromod.
		HTREEITEM hItem = HighlightDropTarget( point );
		if( hItem != NULL )
		{
			if( ( m_hDragItem == hItem ) ||
				( CMainFrame::RightViewList::eRVHMCalc != GetRightViewAt( hItem ) && CMainFrame::RightViewList::eRVHMSumm != GetRightViewAt(hItem) ) )
				hItem = NULL;
			else
			{
				// Can't drag on his parent.
				// Get a pointer on dragged Item's parent.
				CTable *pDragItemParent = (CTable*)( ( (CTable*)GetObjectAt( m_hDragItem ) )->GetIDPtr().PP );
				if( GetObjectAt( hItem ) == (void *)pDragItemParent )
					hItem = NULL;
				else
				{
					// Can't drag on his children.
					if( TRUE == IsChildOf( hItem, m_hDragItem ) )
						hItem = NULL;

					// Can't drag on a terminal unit.
					CDS_HydroMod *pDropTarget = static_cast<CDS_HydroMod *>( GetObjectAt( hItem ) );

					if( pDropTarget && false == pDropTarget->IsaModule() )
					{
						hItem = NULL;
					}
				}

				// If current item is in a cut mode, we don't allow to drop on it.
				// Remark: it seems that 'GetItemState' with 'TVIS_CUT' argument for 'nStateMask' will return not only 'TVIS_CUT' value but
				//         also other. This is why we can't directly do 'TVIS_CUT == GetItemState( hItem, TVIS_CUT )'.
				if( NULL != hItem && TVIS_CUT == ( TVIS_CUT & GetItemState( hItem, TVIS_CUT ) ) )
					hItem = NULL;
			}
		}
		
		// Modify the cursor to provide visual feedback to the user.
		// Note: It's important to do this AFTER the call to DragMove. 
		::SetCursor( hItem == NULL ? AfxGetApp ()->LoadStandardCursor( IDC_NO ) : (HCURSOR)::GetClassLongPtr( m_hWnd, GCLP_HCURSOR ) );
 
		// Set a timer if the cursor is at the top or bottom of the window, or if it's over a collapsed item.
		CRect rect;
		GetClientRect( rect );
		int cy = rect.Height();
 
		if( ( point.y >= 0 && point.y <= m_iScrollMargin ) ||
			( point.y >= cy - m_iScrollMargin && point.y <= cy ) ||
			( hItem  != NULL && ItemHasChildren( hItem ) &&	FALSE == IsItemExpanded( hItem ) ) )
			m_nTimer = SetTimer( _TIMERID_PROJTREECTRL, m_iDelayInterval, NULL );
	 }
}
 
void CProjTreeCtrl::OnLButtonUp( UINT nFlags, CPoint point )
{
	CTreeCtrl::OnLButtonUp( nFlags, point );
 
	if( TRUE == m_fDragging && NULL != m_pImageList )
	{ 
		// Stop the scroll timer if it's running.
		KillTimer( m_nTimer );
		m_nTimer = (UINT_PTR)0;
 
		// Terminate the dragging operation and release the mouse.
		m_pImageList->DragLeave( this );
		m_pImageList->EndDrag();
		::ReleaseCapture();

		m_fDragging = FALSE;
		SelectDropTarget( NULL );
 
		// Delete the image list created by 'CreateDragImage'.
		if( m_pImageList != NULL )
			delete m_pImageList;
		m_pImageList = NULL;
 
		// Get the HTREEITEM of the drop target and exit now if it's NULL. 
		UINT nFlags;
		HTREEITEM hItem = HitTest( point, &nFlags );
		if( NULL == hItem ) 
			return; 
		
		if( ( m_hDragItem == hItem) ||
			( CMainFrame::RightViewList::eRVHMCalc != GetRightViewAt( hItem ) && CMainFrame::RightViewList::eRVHMSumm != GetRightViewAt( hItem ) ) )
			return; 
		else
		{
			// Can't drag on his parent.
			// Get a pointer on dragged item's parent.
			CTable *pDragItemParent = (CTable*)( ( (CTable*)GetObjectAt( m_hDragItem ) )->GetIDPtr().PP );
			if( GetObjectAt( hItem ) == (void *)pDragItemParent )
				return;

			// Can't drag on a terminal unit.
			CDS_HydroMod *pDropTarget = static_cast<CDS_HydroMod *>( GetObjectAt( hItem ) );

			if( pDropTarget && false == pDropTarget->IsaModule() )
			{
				return;
			}

			// If current item is in a cut mode, we don't allow to drop on it.
			// Remark: it seems that 'GetItemState' with 'TVIS_CUT' argument for 'nStateMask' will return not only 'TVIS_CUT' value but
			//         also other. This is why we can't directly do 'TVIS_CUT == GetItemState( hItem, TVIS_CUT )'.
			if( TVIS_CUT == ( TVIS_CUT & GetItemState( hItem, TVIS_CUT ) ) )
				return;
		}

		// Move the dragged item and its subitems (if any) to the drop point.
		CTADatastruct *pTADS = TASApp.GetpTADS();
		unsigned short uid;
		uid = pTADS->MoveHydroMod( (CDS_HydroMod *)GetObjectAt( m_hDragItem ), (CTable *)GetObjectAt( hItem ), true );

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->ResetTreeAndSelectHM( (CDS_HydroMod *)GetObjectAt( hItem ) );
		}

		m_hDragItem = NULL; 
	}
}

void CProjTreeCtrl::OnTimer( UINT_PTR nIDEvent )
{
	CTreeCtrl::OnTimer( nIDEvent );

	if( (UINT_PTR)0 == m_nTimer || m_nTimer != nIDEvent )
		return;

	// Reset the timer.
	m_nTimer = SetTimer( _TIMERID_PROJTREECTRL, m_iScrollInterval, NULL );

	// Get the current cursor position and window height.
	DWORD dwPos = ::GetMessagePos();
	CPoint point( LOWORD( dwPos ), HIWORD( dwPos ) ); 
	ScreenToClient( &point );

	CRect rect;
	GetClientRect( rect );
	int cy = rect.Height();

	// Scroll the window if the cursor is near the top or bottom.
	if( point.y >= 0 && point.y <= m_iScrollMargin )
	{
		HTREEITEM hFirstVisible = GetFirstVisibleItem();
		m_pImageList->DragShowNolock( FALSE );
		SendMessage( WM_VSCROLL, MAKEWPARAM( SB_LINEUP, 0 ), NULL );
		m_pImageList->DragShowNolock( TRUE );

		// Kill the timer if the window did not scroll, or redraw the
		// drop target highlight if the window did scroll. 
		if( GetFirstVisibleItem() == hFirstVisible )
		{
			KillTimer( m_nTimer );
			m_nTimer = (UINT_PTR)0;
		}
		else 
		{  
			HighlightDropTarget( point );
			return;
		}
	}
	else if( point.y >= cy - m_iScrollMargin && point.y <= cy )
	{
		HTREEITEM hFirstVisible = GetFirstVisibleItem();
		m_pImageList->DragShowNolock( FALSE );
		SendMessage( WM_VSCROLL, MAKEWPARAM( SB_LINEDOWN, 0 ), NULL );
		m_pImageList->DragShowNolock( TRUE );
 
		// Kill the timer if the window did not scroll, or redraw the
		// drop target highlight if the window did scroll. 
		if( GetFirstVisibleItem () == hFirstVisible )
		{
			KillTimer( m_nTimer );
			m_nTimer = (UINT_PTR)0;
		}
		else
		{  
			HighlightDropTarget( point );
			return;
		}
	}

	// If the cursor is hovering over a collapsed item, expand the tree.
	UINT nFlags;
	HTREEITEM hItem = HitTest( point, &nFlags );

	if( hItem != NULL && TRUE == ItemHasChildren( hItem ) && FALSE == IsItemExpanded( hItem ) )
	{ 
		m_pImageList->DragShowNolock( FALSE );
		Expand( hItem, TVE_EXPAND );
		m_pImageList->DragShowNolock( TRUE );
		KillTimer( m_nTimer );
		m_nTimer = (UINT_PTR)0;
		return;
	}
}

HTREEITEM CProjTreeCtrl::HighlightDropTarget( CPoint point )
{
	// Find out which item (if any) the cursor is over.
	UINT nFlags;
	HTREEITEM hItem = HitTest( point, &nFlags );

	// Highlight the item, or unhighlight all items if the cursor isn't over an item.
	m_pImageList->DragShowNolock( FALSE );
	SelectDropTarget( hItem );
	m_pImageList->DragShowNolock( TRUE );

	// Return the handle of the highlighted item.
	return  hItem;
}

BOOL CProjTreeCtrl::IsItemExpanded( HTREEITEM hItem )
{
	return ( GetItemState( hItem, TVIS_EXPANDED ) & TVIS_EXPANDED );
}

void CProjTreeCtrl::CopyChildren( HTREEITEM hDest, HTREEITEM hSrc )
{
	// Get the first subitem.
	HTREEITEM hItem = GetChildItem( hSrc );						ASSERT( hItem != NULL );

	// Create a copy of it at the destination. 
	int iImage, iSelectedImage;
	GetItemImage( hItem, iImage, iSelectedImage );
	CString string = GetItemText( hItem );
	HTREEITEM hNewItem = InsertItem( string, iImage, iSelectedImage, hDest );

	LPARAM lparam = GetItemData( hItem );
	SetItemData( hNewItem, lparam );

	// If the subitem has subitems, copy the M, too.
	if( TRUE == ItemHasChildren( hItem ) )
		CopyChildren( hNewItem, hItem );
 
	// Do the same for other subitems of hSrc. 
	while( ( hItem = GetNextSiblingItem( hItem ) ) != NULL )
	{ 
		GetItemImage( hItem, iImage, iSelectedImage );
		string = GetItemText( hItem );
		hNewItem = InsertItem( string, iImage, iSelectedImage, hDest );
		LPARAM lparam = GetItemData( hItem );
		SetItemData( hNewItem, lparam );
		if( TRUE == ItemHasChildren( hItem ) )
			CopyChildren( hNewItem, hItem );
	}
}

void CProjTreeCtrl::CopyTree( HTREEITEM  hDest, HTREEITEM hSrc, bool fNewItem )
{
	// Get the attributes of item to be copied.
	int iImage, iSelectedImage;
	GetItemImage( hSrc, iImage, iSelectedImage );
	CString string = GetItemText( hSrc );

	// Create an exact copy of the item at the destination.
	HTREEITEM hNewItem;
	if( true == fNewItem )
		hNewItem = InsertItem( string, iImage, iSelectedImage, hDest );
	else
	{
		hNewItem = hDest;
		SetItemText( hNewItem,string );
		SetItemImage( hNewItem, iImage, iSelectedImage );
	}
	
	LPARAM lparam = GetItemData( hSrc );
	SetItemData( hNewItem, lparam );

	// If the item has subitems, copy the M, too.
	if( TRUE == ItemHasChildren( hSrc ) )
		CopyChildren( hNewItem, hSrc );

	// Select the newly added item.
	SelectItem( hNewItem );
}

void CProjTreeCtrl::MoveTree( HTREEITEM hDest, HTREEITEM hSrc, bool fNewItem )
{
	CopyTree( hDest, hSrc, fNewItem );
	DeleteItem( hSrc );
}

BOOL CProjTreeCtrl::IsChildOf( HTREEITEM hItem1, HTREEITEM hItem2 )
{
	HTREEITEM hParent = hItem1;
	while( ( hParent = GetParentItem( hParent ) ) != NULL )
	{
		if( hParent == hItem2 )
			return TRUE;
	}
	return FALSE;
}
