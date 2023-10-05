#include "stdafx.h"
#include <float.h>
#include "TASelect.h"
#include "Utilities.h"
#include "ExtNumEdit.h"
#include "ExtButton.h"


CExtButton::CExtButton()
{
	m_crText = _BLACK;
	m_crTextDisabled = ::GetSysColor( COLOR_3DFACE );
	m_crBackGnd = ::GetSysColor( COLOR_WINDOW );

	m_bDrawBorderActivated = false;
	m_crBorderColor = 0;
	::GetObject( (HFONT)GetStockObject( DEFAULT_GUI_FONT ), sizeof( m_lf ), &m_lf );
	m_clFont.CreateFontIndirect( &m_lf );
	m_bLeftClickEnabled = true;
}

CExtButton::~CExtButton()
{
	// Delete brush.
	if( m_brBackGnd.GetSafeHandle() )
	{
		m_brBackGnd.DeleteObject();
	}

	if( NULL != m_clFont.GetSafeHandle() )
	{
		m_clFont.DeleteObject();
	}
}

void CExtButton::SetTextColor( COLORREF rgb )
{
	// Set text color ref.
	m_crText = rgb;

	// Redraw.
	Invalidate( TRUE );
	UpdateWindow();
}

void CExtButton::SetTextDisabledColor( COLORREF rgb )
{
	// Set text color ref.
	m_crTextDisabled = rgb;

	// Redraw.
	Invalidate( TRUE );
	UpdateWindow();
}

void CExtButton::SetBackColor( COLORREF rgb )
{
	// Set background color ref (used for text's background).
	m_crBackGnd = rgb;
	
	// Free brush.
	if( NULL != m_brBackGnd.GetSafeHandle() )
	{
		m_brBackGnd.DeleteObject();
	}
	
	// Set brush to new color.
	m_brBackGnd.CreateSolidBrush( rgb );
	
	// Redraw.
	Invalidate( TRUE );
	UpdateWindow();
}

void CExtButton::SetDrawBorder( bool bActivate, COLORREF crBorderColor )
{
	m_bDrawBorderActivated = bActivate;
	m_crBorderColor = crBorderColor;
	RedrawWindow( NULL, NULL, RDW_FRAME | RDW_INVALIDATE );
}

void CExtButton::ResetDrawBorder( void )
{
	m_bDrawBorderActivated = false;
	RedrawWindow( NULL, NULL, RDW_FRAME | RDW_INVALIDATE );
}

BEGIN_MESSAGE_MAP( CExtButton, CButton )
	ON_WM_CTLCOLOR_REFLECT()
	ON_MESSAGE( WM_USER_NEWDOCUMENT, OnNewDocument )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

// See the 'TAS Project\TASelect-Sln\Doc\Code\CStatic.md' for a well understanding of this code.
HBRUSH CExtButton::CtlColor( CDC *pDC, UINT nCtlColor )
{
	// Set text color.
	pDC->SetTextColor( ( WS_DISABLED != ( GetStyle() & WS_DISABLED ) ) ? m_crText : m_crTextDisabled );
	
	// Set the static background color only if has been defined by calling 'SetBackColor'.
	if( NULL != m_brBackGnd.GetSafeHandle() )
	{
		pDC->SetBkColor( m_crBackGnd );
	}
	else
	{
		pDC->SetBkMode( TRANSPARENT );
	}

	// Return the brush used for background if set otherwise NULL.
	return ( NULL != m_brBackGnd.GetSafeHandle() ) ? m_brBackGnd : (HBRUSH)GetStockObject( NULL_BRUSH );
}

LRESULT CExtButton::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	if( true == m_bDrawBorderActivated )
	{
		_CancelDrawBorderMode();
	}

	return 0;
}

void CExtButton::OnPaint() 
{
	Default();

	if( true == m_bDrawBorderActivated )
	{
		CRect rectClient;
		GetClientRect( &rectClient );
		CBrush clBrush( m_crBorderColor );

		CWindowDC dc( this );
		dc.FrameRect( &rectClient, &clBrush );
	}
}

void CExtButton::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( true == m_bLeftClickEnabled )
	{
		CButton::OnLButtonDown( nFlags, point );
	}
}

void CExtButton::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	if( true == m_bLeftClickEnabled )
	{
		CButton::OnLButtonDblClk( nFlags, point );
	}
}

void CExtButton::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	// This class works only for push buttons.
	ASSERT( lpDrawItemStruct->CtlType == ODT_BUTTON );

	// Get the device context pointer to draw into.
	CDC *pDC = CDC::FromHandle( lpDrawItemStruct->hDC );

	pDC->SetBkMode( TRANSPARENT );

	CRect r = lpDrawItemStruct->rcItem;
	pDC->FillSolidRect( &r, m_crBackGnd );

	pDC->SelectObject( &m_clFont );
	pDC->SetTextColor( m_crText );
	UINT uiStyle = DT_CENTER | DT_SINGLELINE | DT_VCENTER;
	CString str;
	GetWindowText( str );

// 	if( lpDrawItemStruct->itemState & ODS_DISABLED )
// 	{
// 		pDC->DrawState( r.TopLeft(), pDC->GetTextExtent( str ), str, DSS_DISABLED, TRUE, 0, (HBRUSH)NULL );
// 	}
// 	else
// 	{
		pDC->DrawText( str, &r, uiStyle);
//	}

	// .......................................................................
	// IF CHECK BOX HAS FOCUS DRAW THE FOCUS BOX
	// .......................................................................

	if( lpDrawItemStruct->itemState & ODS_FOCUS )
	{
		CSize size = pDC->GetTextExtent( str );
		r.right = r.left + size.cx +1;
		r.left -=1 ;
		pDC->DrawFocusRect( &r );
	}
}

void CExtButton::_CancelDrawBorderMode()
{
	if( true == m_bDrawBorderActivated )
	{
		CWindowDC dc( this );

		CRect rectClient;
		GetClientRect( &rectClient );

		HTHEME hTheme = OpenThemeData( GetSafeHwnd(), _T("EDIT") );
		COLORREF crColor;
		HRESULT hResult = GetThemeColor( hTheme, EP_EDITBORDER_NOSCROLL, EPSN_NORMAL, TMT_BORDERCOLOR, &crColor );
		CloseThemeData( hTheme );
		CBrush crBrush( crColor );
		dc.FrameRect( &rectClient, &crBrush );

		m_bDrawBorderActivated = false;
	}
}
