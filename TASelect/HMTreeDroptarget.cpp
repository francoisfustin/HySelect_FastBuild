#include "stdafx.h"
#include "TASelect.h"
#include "HydroMod.h"
#include "HMTreeDroptarget.h"
#include "HMTreeListCtrl.h"

#define RECT_BORDER	10
#define TIMER_EXPAND 500

CHMTreeDropTarget::CHMTreeDropTarget()
{
	m_pSourceTreeCtrl = NULL;
	m_pDestTreeCtrl = NULL;
	m_RectLine = CRect( 0, 0, 0, 0 );
	m_MarkerNexthItem = NULL;
	m_hTItemToExpand = NULL;
	m_hDestItem = NULL;
	m_bAllowProperDragDrop = false;
}

DROPEFFECT CHMTreeDropTarget::OnDragEnter( CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point )
{
	DROPEFFECT dropeffectRet = DROPEFFECT_COPY;

	// Check what is the source.
	_RetrieveSource( pDataObject );

	// If the source tree and the destination tree are the same, allow only a move.
	if( NULL != m_pSourceTreeCtrl && m_pSourceTreeCtrl == pWnd )
	{
		dropeffectRet = DROPEFFECT_MOVE;
	}

	return dropeffectRet;
}

DROPEFFECT CHMTreeDropTarget::OnDragOver( CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point )
{
	// By default the dropeffectRet is a move.
	DROPEFFECT dropeffectRet = DROPEFFECT_MOVE;

	// Check what is the source.
	_RetrieveSource( pDataObject );

	if( NULL == m_pSourceTreeCtrl )
	{
		return DROPEFFECT_NONE;
	}

	// Recuperate the source tree.
	HTREEITEM hScrSelectedItem = m_pSourceTreeCtrl->GetSelectedItem();
	
	// If the source and the destination are different, allow copy.
	if( m_pSourceTreeCtrl != pWnd )
	{
		dropeffectRet = DROPEFFECT_COPY;
	}
	
	// Accept drag and drop move on other window than tree if the source hItem is already imported (to allow delete function).
	if( NULL == dynamic_cast<CHMTreeListCtrl *>( pWnd ) )
	{		
		CHMTreeListCtrl::m_UserData *pUD = (CHMTreeListCtrl::m_UserData *)m_pSourceTreeCtrl->GetUserData( hScrSelectedItem );

		if( true == pUD->bImported )
		{
			return DROPEFFECT_MOVE;
		}
		else
		{
			return DROPEFFECT_NONE;
		}
	}

	m_pDestTreeCtrl = (CHMTreeListCtrl *)pWnd;
	HTREEITEM hTItem = m_pDestTreeCtrl->HitTest( point );

	// Do not accept drag and drop on the same tree.
	if( false == m_bAllowProperDragDrop )
	{
		return DROPEFFECT_NONE;
	}

	if( NULL != hTItem )
	{
		// Expand after a delay the hTItem.
		if( NULL == m_hTItemToExpand || m_hTItemToExpand != hTItem )
		{
			m_hTItemToExpand = hTItem;
			m_pDestTreeCtrl->ExpandNodeAfterDelay( m_hTItemToExpand, TIMER_EXPAND );
		}
		
		// Highlight the hTItem.
		m_pDestTreeCtrl->SelectDropTarget( hTItem );

		// Do not accept drag and drop an imported hItem to his child.
		if( DROPEFFECT_MOVE == dropeffectRet )
		{
			if( hScrSelectedItem == hTItem )
			{
				return DROPEFFECT_NONE;
			}
			
			HTREEITEM hParent = hTItem;
			bool bSameBranch = false;
			
			while( m_pDestTreeCtrl->GetRootItem() != hParent && false == bSameBranch )
			{
				hParent = m_pDestTreeCtrl->GetParentItem( hParent );

				if( hScrSelectedItem == hParent )
				{
					bSameBranch = true;
				}
			}

			if( true == bSameBranch )
			{
				return DROPEFFECT_NONE;
			}
		}

		// Calculate the rectangle of the current hItem.
		CRect RectItem;
		( (CTreeCtrl *)m_pDestTreeCtrl )->GetItemRect( hTItem, &RectItem, FALSE );
		
		// Define the zone where the cursor is :
		// It can be at 1/4 height from the top of the item then interpret module insertion and draw line.
		if( RectItem.top <= point.y && ( RectItem.top + ( RectItem.Height() / 4 ) ) >= point.y )
		{
			// Verify the user doesn't try to drag over the root module.
			if( hTItem == m_pDestTreeCtrl->GetRootItem() )
			{
				return DROPEFFECT_NONE;
			}

			// Do not accept to drag and drop TU in the root.
			if( m_pDestTreeCtrl->GetRootItem() == m_pDestTreeCtrl->GetParentItem(hTItem ) )
			{				
				CHMTreeListCtrl::m_UserData *pUD = (CHMTreeListCtrl::m_UserData *)m_pSourceTreeCtrl->GetUserData( m_pSourceTreeCtrl->GetSelectedItem() );

				if( NULL != pUD && false == pUD->pHM->IsaModule() )
				{
					return DROPEFFECT_NONE;
				}
			}

			// Calculate the rectangle's position for the line.
			CRect RectLine( RectItem.left, RectItem.top, RectItem.right, RectItem.top + 1 );
			
			// Draw the line with blue color.
			CPen pen;
			pen.CreatePen( PS_SOLID, 1, _LIGHTBLUE );
			CDC* pDC = pWnd->GetDC();
			pDC->SelectObject( &pen );
			pDC->MoveTo( RectLine.left, RectLine.top );
			pDC->LineTo( RectLine.right, RectLine.top );

			// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
			pWnd->ReleaseDC( pDC );
			
			// Specify the item's marker.
			m_MarkerNexthItem = hTItem;
			m_RectLine = RectItem;
			
			return dropeffectRet;
		}
		else
		{
			// In the other part (3/4 height from bottom) interpret the drop module function.

			// Delete markers.
			if( FALSE == m_RectLine.IsRectNull() )
			{
				pWnd->InvalidateRect( &m_RectLine );
			}
			
			m_RectLine.SetRectEmpty();
			m_MarkerNexthItem = NULL;

			// Do not accept to drag and drop TU in the root.
			if( hTItem == m_pDestTreeCtrl->GetRootItem() )
			{				
				CHMTreeListCtrl::m_UserData *pUD = (CHMTreeListCtrl::m_UserData *)m_pSourceTreeCtrl->GetUserData( m_pSourceTreeCtrl->GetSelectedItem() );

				if( NULL != pUD && false == pUD->pHM->IsaModule() )
				{
					return DROPEFFECT_NONE;
				}
			}
			else
			{				
				// Do not accept to drag and drop Module/TU in a TU.
				CHMTreeListCtrl::m_UserData *pUD = (CHMTreeListCtrl::m_UserData *)m_pDestTreeCtrl->GetUserData( hTItem );

				if( NULL != pUD && false == pUD->pHM->IsaModule() )
				{
					return DROPEFFECT_NONE;
				}
			}
		}
	}
	
	// Scroll Tree control depending on mouse position.
	CRect rectClient;
	pWnd->GetClientRect( &rectClient );
	pWnd->ClientToScreen( rectClient );
	pWnd->ClientToScreen( &point );
	int nScrollDir = -1;
	
	if( point.y >= rectClient.bottom - RECT_BORDER )
	{
		nScrollDir = SB_LINEDOWN;
	}
	else if ( ( point.y <= rectClient.top + RECT_BORDER ) )
	{
		nScrollDir = SB_LINEUP;
	}
	
	if( -1 != nScrollDir )
	{
		int nScrollPos = pWnd->GetScrollPos( SB_VERT );
		WPARAM wParam = MAKELONG( nScrollDir, nScrollPos );
		pWnd->SendMessage( WM_VSCROLL, wParam );
	}
	
	nScrollDir = -1;

	if( point.x <= rectClient.left + RECT_BORDER )
	{
		nScrollDir = SB_LINELEFT;
	}
	else if( point.x >= rectClient.right - RECT_BORDER )
	{
		nScrollDir = SB_LINERIGHT;
	}
	
	if( nScrollDir != -1 ) 
	{
		int nScrollPos = pWnd->GetScrollPos( SB_VERT );
		WPARAM wParam = MAKELONG( nScrollDir, nScrollPos );
		pWnd->SendMessage( WM_HSCROLL, wParam );
	}

	return dropeffectRet;
}

void CHMTreeDropTarget::OnDragLeave( CWnd *pWnd )
{
	// Remove highlighting.
	m_pDestTreeCtrl = NULL;
	pWnd->SendMessage( TVM_SELECTITEM, TVGN_DROPHILITE, 0 );

	// Remove insert line.
	if( FALSE == m_RectLine.IsRectNull() )
	{
		pWnd->InvalidateRect( &m_RectLine );
	}
}

BOOL CHMTreeDropTarget::OnDrop( CWnd *pWnd, COleDataObject *pDataObject, DROPEFFECT dropEffect, CPoint point )
{
	// Invalidate region of the line.
	if( FALSE == m_RectLine.IsRectNull() )
	{
		pWnd->InvalidateRect( &m_RectLine );
	}

	// Do not accept drag and drop on the same tree.
	if( false == m_bAllowProperDragDrop )
	{
		return FALSE;
	}

	// Check what is the source.
	_RetrieveSource( pDataObject );

	if( NULL == m_pSourceTreeCtrl )
	{
		return FALSE;
	}

	// Now we can free the memory.
	_FreeMemory( pDataObject );

	HTREEITEM hSelectedItem = m_pSourceTreeCtrl->GetSelectedItem();
	CHMTreeListCtrl::m_UserData *pUD = (CHMTreeListCtrl::m_UserData *)m_pSourceTreeCtrl->GetUserData( hSelectedItem );

	// Other controls than CHMTreeListCtrl can be defined as a DropTarget.
	// In that case it will only enable the possibility to delete the imported hItem from the source tree.
	if( NULL == dynamic_cast<CHMTreeListCtrl *>( m_pDestTreeCtrl ) )
	{		
		if( true == pUD->bImported )
		{
			m_pSourceTreeCtrl->DeleteItem( hSelectedItem );
		}

		return FALSE;
	}
	
	// Get the image indexes from ImageList, if one exists.
	int nImage = 0;
	int nSelImage = 0;
	_GetItemImages( hSelectedItem, nImage, nSelImage );

	// Get the selected item from Destination Tree control.
	HTREEITEM hTDropItem = m_pDestTreeCtrl->GetDropHilightItem();

	// Do nothing in case no hTDropItem is null.
	if( NULL == hTDropItem )
	{
		return FALSE;
	}

	if( NULL != m_MarkerNexthItem )
	{
		// In case the user drop an hItem with the markers add the hItem to the previous position of the marker.

		// Define the parent and after which item the item must be inserted.
		HTREEITEM hParent = m_pDestTreeCtrl->GetParentItem( m_MarkerNexthItem );
		HTREEITEM hInsAfter = m_pDestTreeCtrl->GetPrevSiblingItem( m_MarkerNexthItem );

		if( NULL == hInsAfter )
		{
			hInsAfter = TVI_FIRST;
		}

		// Add the parent.
		m_hDestItem = m_pDestTreeCtrl->AddItem( hParent, m_pSourceTreeCtrl->GetItemText( hSelectedItem ), hInsAfter, nImage, nSelImage, m_pSourceTreeCtrl->GetItemData( hSelectedItem ), pUD, dropEffect );

		// Add all children if exist.
		_AddItem( m_pSourceTreeCtrl->GetChildItem( hSelectedItem ), m_hDestItem, dropEffect );
	}
	else
	{
		// In case the user drop an hItem in a hItem.
		
		// Add the parent.
		m_hDestItem = m_pDestTreeCtrl->AddItem( hTDropItem, m_pSourceTreeCtrl->GetItemText( hSelectedItem ), TVI_LAST, nImage, nSelImage, m_pSourceTreeCtrl->GetItemData( hSelectedItem ), pUD, dropEffect );
		
		// Add all children if exist.
		_AddItem( m_pSourceTreeCtrl->GetChildItem( hSelectedItem ), m_hDestItem, dropEffect );
	}

	// Remove highlighting.
	m_pDestTreeCtrl->SendMessage( TVM_SELECTITEM, TVGN_DROPHILITE, 0 );

	// In case the drag and drop is a move, delete the previous hItem.
	if( DROPEFFECT_MOVE == dropEffect )
	{
		m_pDestTreeCtrl->DeleteItem( hSelectedItem );
	}

	return TRUE;	
}

void CHMTreeDropTarget::_RetrieveSource( COleDataObject *pDataObject )
{
	if( NULL == pDataObject || NULL != m_pSourceTreeCtrl )
	{
		return;
	}

	HGLOBAL hg = pDataObject->GetGlobalData ( CF_PRIVATEFIRST + 1 );

	if( NULL == hg )
	{
		return;
	}

	PHANDLE pHandle = (PHANDLE)GlobalLock( hg );
	
	if ( NULL == pHandle )
	{
		GlobalUnlock( hg );
		return;
	}

	// Get the selected item from Source Tree control.
	m_pSourceTreeCtrl = (CHMTreeListCtrl *)CWnd::FromHandlePermanent( (HWND)*pHandle );
}

void CHMTreeDropTarget::_FreeMemory( COleDataObject *pDataObject )
{
	if( NULL == pDataObject )
	{
		return;
	}
	
	HGLOBAL hg = pDataObject->GetGlobalData ( CF_PRIVATEFIRST + 1 );

	if( NULL == hg )
	{
		return;
	}

	GlobalFree( hg );
}

void CHMTreeDropTarget::_AddItem( HTREEITEM hSrcTItem, HTREEITEM hDestTItem, DROPEFFECT dropEffect )
{
	int nImage;
	int nSelImage;

	while( NULL != hSrcTItem )
	{
		// Get UserData from source.
		CHMTreeListCtrl::m_UserData *pUD = (CHMTreeListCtrl::m_UserData *)m_pSourceTreeCtrl->GetUserData( hSrcTItem );

		// Get the item text and Image indexes for source tree control.
		CString csItem = m_pSourceTreeCtrl->GetItemText( hSrcTItem );
		_GetItemImages( hSrcTItem, nImage, nSelImage );
		
		// Add item to destination tree control.
		HTREEITEM hDestChildItem = m_pDestTreeCtrl->AddItem( hDestTItem, csItem, TVI_LAST, nImage, nSelImage, m_pSourceTreeCtrl->GetItemData( hSrcTItem ), pUD, dropEffect );
		HTREEITEM hSrcChildItem = m_pSourceTreeCtrl->GetChildItem( hSrcTItem );
		
		if( NULL != hSrcChildItem ) 
		{
			_AddItem( hSrcChildItem, hDestChildItem, dropEffect );
		}
		
		hSrcTItem = m_pSourceTreeCtrl->GetNextSiblingItem(hSrcTItem );
		hDestTItem = m_pDestTreeCtrl->GetParentItem( hDestChildItem );
	}
}

void CHMTreeDropTarget::_GetItemImages( HTREEITEM hSrcTItem, int &nSelItemImage, int &nNonSelItemImage )
{
	CImageList *pImageList = m_pSourceTreeCtrl->GetImageList( TVSIL_NORMAL );
	
	// If no image list is associated with the tree control, return.
	if( NULL == pImageList )
	{
		nSelItemImage = 0;
		nNonSelItemImage = 0;
	}
	else
	{
		// If no image list is associated with Destination tree control.
		// Set the image list of source tree control.
		if( NULL == m_pDestTreeCtrl->GetImageList( TVSIL_NORMAL ) )
		{
			m_pDestTreeCtrl->SetImageList( pImageList, TVSIL_NORMAL );
		}
		
		// Get the image indexes.
		m_pSourceTreeCtrl->GetItemImage( hSrcTItem, nSelItemImage, nNonSelItemImage );
	}
}
