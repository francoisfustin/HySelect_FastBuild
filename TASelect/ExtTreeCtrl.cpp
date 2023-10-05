#include "stdafx.h"
#include "TASelect.h"
#include "Utilities.h"
#include "ExtTreeCtrl.h"


IMPLEMENT_DYNAMIC( CExtTreeCtrl, CTreeCtrl )

void CExtTreeCtrl::ResetAllColors()
{
	m_mapItemColors.clear();
}

void CExtTreeCtrl::SetItemColor( HTREEITEM hItem, COLORREF color )
{
	m_mapItemColors[hItem] = color;
}

BEGIN_MESSAGE_MAP( CExtTreeCtrl, CTreeCtrl )
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CExtTreeCtrl::PreTranslateMessage(MSG* pMsg)
{
	// Notify Enter key to the tree
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		NMHDR hdr;
		hdr.hwndFrom = GetSafeHwnd();
		hdr.idFrom   = GetDlgCtrlID();
		hdr.code     = WM_USER_ENTERKEYPRESSED;
		GetParent()->SendMessage(WM_NOTIFY,GetDlgCtrlID(),(LPARAM)&hdr);
	}

	return CTreeCtrl::PreTranslateMessage(pMsg);
}

void CExtTreeCtrl::OnPaint() 
{
	CPaintDC dc( this );

	// Create a memory DC compatible with the paint DC.
	CDC memDC;
	memDC.CreateCompatibleDC( &dc );

	CRect clRectClip;
	CRect clRectClient;
	dc.GetClipBox( &clRectClip );
	GetClientRect( &clRectClient );

	// Select a compatible bitmap into the memory DC.
	CBitmap clBitmap;
	clBitmap.CreateCompatibleBitmap( &dc, clRectClient.Width(), clRectClient.Height() );
	memDC.SelectObject( &clBitmap );
	
	// Set clip region to be same as that in paint DC.
	CRgn clRegion;
	clRegion.CreateRectRgnIndirect( &clRectClip );
	memDC.SelectClipRgn( &clRegion );
	clRegion.DeleteObject();
	
	// First let the control do its default drawing.
	CWnd::DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );

	HTREEITEM hItem = GetFirstVisibleItem();
	int iItemCount = GetVisibleCount() + 1;

	while( NULL != hItem && 0 != iItemCount-- )
	{		
		CRect rect;

		// Do not meddle with selected items or drop highlighted items.
		UINT selflag = TVIS_DROPHILITED | TVIS_SELECTED;
	
		if( ( 0 != ( GetItemState( hItem, selflag ) & selflag ) ) && ( ::GetFocus() == m_hWnd ) )
		{
			// Do nothing.
		}
		else if( 0 != m_mapItemColors.count( hItem ) )
		{
			CFont *pFontDC;
			CFont fontDC;
			LOGFONT logfont;
			CFont *pFont = GetFont();
			pFont->GetLogFont( &logfont );

			fontDC.CreateFontIndirect( &logfont );
			pFontDC = memDC.SelectObject( &fontDC );

			if( m_mapItemColors[hItem] != (COLORREF) - 1)
			{
				memDC.SetTextColor( m_mapItemColors[hItem] );
			}
			else
			{
				memDC.SetTextColor( GetSysColor( COLOR_WINDOWTEXT ) );
			}

			CString sItem = GetItemText( hItem );

			GetItemRect( hItem, &rect, TRUE );
			memDC.SetBkColor( GetSysColor( COLOR_WINDOW ) );
			memDC.TextOut( rect.left + 2, rect.top + 1, sItem );
			
			memDC.SelectObject( pFontDC );
		}

		hItem = GetNextVisibleItem( hItem );
	}

	dc.BitBlt( clRectClip.left, clRectClip.top, clRectClip.Width(), clRectClip.Height(), &memDC, clRectClip.left, clRectClip.top, SRCCOPY );

	memDC.DeleteDC();
}
