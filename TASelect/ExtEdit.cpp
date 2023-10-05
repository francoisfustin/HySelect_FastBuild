#include "stdafx.h"


#include "TASelect.h"
#include "Utilities.h"
#include "ExtEdit.h"


IMPLEMENT_DYNAMIC( CExtEdit, CEdit )

CExtEdit::CExtEdit()
{
	m_crText = _BLACK;
	m_crTextDisabled = ::GetSysColor( COLOR_GRAYTEXT );

	// By default, we set white for all edit text controls.
	m_crBackGnd = _WHITE;
	m_brBackGnd.CreateSolidBrush( m_crBackGnd );

	m_eFontBold = eBool3::eb3Undef;
	m_iFontSize = -1;
	m_bBlockOnEnChangeNotification = false;
	m_bOnCharHandler = true;
	m_bOnSetFocusHandler = true;
	m_bOnKillFocusHandler = true;
	m_bOnUnitChangeHandler = true;
	m_bOnNewDocumentHandler = true;
	m_bSpecialValidationActive = false;
	m_bBlockSelection = false;
	m_bBlockCursorChange = false;
	m_bDrawBorderActivated = false;
	m_crBorderColor = 0;
	m_bBlockSelection = false;
}

CExtEdit::~CExtEdit()
{
	// Delete brush.
	if( NULL != m_brBackGnd.GetSafeHandle() )
	{
		m_brBackGnd.DeleteObject();
	}
}

void CExtEdit::SetTextColor( COLORREF rgb )
{
	// Set text color ref.
	m_crText = rgb;

	// Redraw.
	Invalidate( TRUE );
	UpdateWindow();
}

void CExtEdit::SetTextDisabledColor( COLORREF rgb )
{
	// Set text color ref.
	m_crTextDisabled = rgb;

	// Redraw.
	Invalidate( TRUE );
	UpdateWindow();
}

void CExtEdit::SetBackColor( COLORREF rgb )
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

COLORREF CExtEdit::GetBackColor( void )
{
	COLORREF clReturn = -1;

	if( NULL !=  m_brBackGnd.GetSafeHandle() )
	{
		clReturn = m_crBackGnd;
	}

	return clReturn;
}

void CExtEdit::SetDrawBorder( bool bActivate, COLORREF crBorderColor )
{
	m_bDrawBorderActivated = bActivate;
	m_crBorderColor = crBorderColor;
	RedrawWindow( NULL, NULL, RDW_FRAME | RDW_INVALIDATE );
}

void CExtEdit::ResetDrawBorder( void )
{
	m_bDrawBorderActivated = false;
	RedrawWindow( NULL, NULL, RDW_FRAME | RDW_INVALIDATE );
}

void CExtEdit::SetToolTip( bool bActivate, CWnd *pParent, CString str )
{
	if( true == bActivate )
	{
		if( NULL == pParent || true == str.IsEmpty() )
		{
			return;
		}

		if( NULL == m_ToolTip.GetSafeHwnd() )
		{
			if( 0 == m_ToolTip.Create( pParent, TTS_NOPREFIX ) )
			{
				return;
			}

			if( 0 == m_ToolTip.AddTool( this, str ) )
			{
				return;	
			}
		}
	}
	else
	{
		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			m_ToolTip.DelTool( this );
			m_ToolTip.DestroyWindow();
		}
	}
}

BEGIN_MESSAGE_MAP( CExtEdit, CEdit )
	ON_MESSAGE( WM_USER_NEWDOCUMENT, OnNewDocument )
	ON_CONTROL_REFLECT_EX( EN_CHANGE, OnEnChange )
	ON_WM_CHAR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_PAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_GETDLGCODE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

BOOL CExtEdit::PreTranslateMessage( MSG *pMsg )
{
	// Do not interpret right button click.
	if( ( WM_RBUTTONDOWN == pMsg->message || WM_RBUTTONDBLCLK == pMsg->message ) && VK_RBUTTON == pMsg->wParam )
	{
		return TRUE;
	}

	// Notify Enter key to the parent window.
	if( WM_KEYDOWN == pMsg->message && ( VK_RETURN == pMsg->wParam || VK_UP == pMsg->wParam || VK_DOWN == pMsg->wParam || VK_TAB == pMsg->wParam ) )
	{
		if( false == m_bSpecialValidationActive && ( VK_UP == pMsg->wParam || VK_DOWN == pMsg->wParam || VK_TAB == pMsg->wParam ) )
		{
			return CEdit::PreTranslateMessage( pMsg );
		}

		if( false == m_bSpecialValidationActive )
		{
			NMHDR hdr;
			hdr.hwndFrom = GetSafeHwnd();
			hdr.idFrom = GetDlgCtrlID();
			hdr.code = WM_USER_ENTERKEYPRESSED;
			GetParent()->SendMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&hdr );
		}
		else if( NULL != m_pclNotificationHandler )
		{
			m_pclNotificationHandler->OnSpecialValidation( this, pMsg->wParam );
		}
	}
	
	return CEdit::PreTranslateMessage( pMsg );
}

LRESULT CExtEdit::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	if( false == m_bOnNewDocumentHandler )
	{
		return 0;
	}

	if( true == m_bDrawBorderActivated )
	{
		_CancelDrawBorderMode();
	}

	return 0;
}

BOOL CExtEdit::OnEnChange()
{
	// If notification is not allowed, we return 'TRUE' to warn that the notification has been eaten by the
	// child. And in this case, the parent is not notify.
	return ( true == m_bBlockOnEnChangeNotification ) ? TRUE : FALSE;
}

void CExtEdit::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	if( true == m_bDrawBorderActivated )
	{
		_CancelDrawBorderMode();
	}

	// Get the string before adding new character.
	CString str;
	GetWindowText( str );

	// Do the default function.
	// Remark: will fire the 'ON_EN_CHANGE' event! This is why we block here to allow the parent
	// to be notified at the end of this method when the current SI value is well updated.
	m_bBlockOnEnChangeNotification = true;
	CEdit::OnChar( nChar, nRepCnt, nFlags );
	m_bBlockOnEnChangeNotification = false;

	// HYS-885: update when Backspace key down 
	if( false == m_bOnCharHandler || VK_ESCAPE == nChar )
	{
		return;
	}

	// Manually notify the parent.
	GetParent()->SendMessage( WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(), EN_CHANGE ), (LPARAM)GetSafeHwnd() );
}

void CExtEdit::OnSetFocus( CWnd *pOldWnd )
{
	if( false == m_bOnSetFocusHandler )
	{
		return CEdit::OnSetFocus( pOldWnd );
	}
	
	PostMessage( EM_SETSEL, 0, -1 );
	CEdit::OnSetFocus( pOldWnd );
}

void CExtEdit::OnKillFocus( CWnd *pNewWnd )
{
	if( false == m_bOnKillFocusHandler )
	{
		return CEdit::OnKillFocus( pNewWnd );
	}
	
	// Do the default function.
	CEdit::OnKillFocus( pNewWnd );
}

HBRUSH CExtEdit::CtlColor( CDC *pDC, UINT nCtlColor )
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

void CExtEdit::OnPaint( )
{
	CEdit::OnPaint();

	if( WS_DISABLED == ( GetStyle() & WS_DISABLED ) && ES_MULTILINE != ( GetStyle() & WS_DISABLED )
			&& m_crTextDisabled != ::GetSysColor( COLOR_GRAYTEXT ) )
	{
		CRect clientRect;
		GetRect( &clientRect );

		CDC *pDC = GetDC();
		pDC->SetTextColor( m_crTextDisabled );
		pDC->SetBkColor( GetBackColor() );
		CFont *pOldFont = pDC->SelectObject( GetFont() );

		CString strText;
		GetWindowText( strText );
		pDC->TextOut( clientRect.left, clientRect.top, strText );

		pDC->SelectObject( pOldFont );
		ReleaseDC( pDC );
	}
}

void CExtEdit::OnNcPaint() 
{
	Default();

	CWindowDC dc( this );

	if( true == m_bDrawBorderActivated )
	{
		CRect rectClient;
		GetClientRect( &rectClient );
		rectClient.right += 4;
		rectClient.bottom += 4;
		CBrush clBrush( m_crBorderColor );
		dc.FrameRect( &rectClient, &clBrush );
	}
}

void CExtEdit::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( false == m_bBlockSelection )
	{
		CEdit::OnLButtonDown( nFlags, point );
	}
}

void CExtEdit::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	if( false == m_bBlockSelection )
	{
		CEdit::OnLButtonDblClk( nFlags, point );
	}
}

BOOL CExtEdit::OnSetCursor( CWnd *pWnd, UINT nHitTest, UINT message )
{
	if( false == m_bBlockCursorChange )
	{
		return CEdit::OnSetCursor( pWnd, nHitTest, message );
	}

	return 1;
}

void CExtEdit::_CancelDrawBorderMode()
{
	if( true == m_bDrawBorderActivated )
	{
		CWindowDC dc( this );

		CRect rectClient;
		GetClientRect( &rectClient );
		rectClient.right += 4;
		rectClient.bottom += 4;

		HTHEME hTheme = OpenThemeData( GetSafeHwnd(), _T("EDIT") );
		COLORREF crColor;
		HRESULT hResult = GetThemeColor( hTheme, EP_EDITBORDER_NOSCROLL, EPSN_NORMAL, TMT_BORDERCOLOR, &crColor );
		CloseThemeData( hTheme );
		CBrush crBrush( crColor );
		dc.FrameRect( &rectClient, &crBrush );

		m_bDrawBorderActivated = false;
	}
}
