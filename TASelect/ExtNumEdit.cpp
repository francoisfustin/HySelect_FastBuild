#include "stdafx.h"
#include <float.h>
#include "TASelect.h"
#include "Utilities.h"
#include "ExtNumEdit.h"


IMPLEMENT_DYNAMIC( CExtNumEdit, CEdit )

CExtNumEdit::CExtNumEdit()
{
	m_crText = _BLACK;
	m_crTextDisabled = ::GetSysColor( COLOR_GRAYTEXT );

	// By default, we set white for all edit text controls.
	m_crBackGnd = _WHITE;
	m_brBackGnd.CreateSolidBrush( m_crBackGnd );

	m_eFontBold = eBool3::eb3Undef;
	m_iFontSize = -1;
	m_bEmptyMode = false;
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
	m_eVertAlignStyle = VertAlignStyle::VAS_Undefined;
	m_rectNCBottom = CRect( 0, 0, 0, 0 );
	m_rectNCTop = CRect( 0, 0, 0, 0 );
	m_bBlockSelection = false;
}

CExtNumEdit::~CExtNumEdit()
{
	// Delete brush.
	if( NULL != m_brBackGnd.GetSafeHandle() )
	{
		m_brBackGnd.DeleteObject();
	}
}

void CExtNumEdit::SetTextColor( COLORREF rgb )
{
	// Set text color ref.
	m_crText = rgb;

	// Redraw.
	Invalidate( TRUE );
	UpdateWindow();
}

void CExtNumEdit::SetTextDisabledColor( COLORREF rgb )
{
	// Set text color ref.
	m_crTextDisabled = rgb;

	// Redraw.
	Invalidate( TRUE );
	UpdateWindow();
}

void CExtNumEdit::SetBackColor( COLORREF rgb )
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

COLORREF CExtNumEdit::GetBackColor( void )
{
	COLORREF clReturn = -1;

	if( NULL !=  m_brBackGnd.GetSafeHandle() )
	{
		clReturn = m_crBackGnd;
	}

	return clReturn;
}

void CExtNumEdit::SetVertAlignStyle( VertAlignStyle eVertAlignStyle )
{
	m_eVertAlignStyle = eVertAlignStyle;

	if( VertAlignStyle::VAS_Undefined != m_eVertAlignStyle )
	{
		SetWindowPos( NULL, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED );
	}
}

void CExtNumEdit::SetWindowTextW( CString strText )
{
	if( true == m_bEmptyMode )
	{
		return;
	}

	CEdit::SetWindowText( strText );

	if( VertAlignStyle::VAS_Undefined != m_eVertAlignStyle )
	{
		SetWindowPos( NULL, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED );
	}
}

void CExtNumEdit::SetEmpty( bool bEmpty )
{
	m_bEmptyMode = bEmpty;

	if( true == bEmpty )
	{
		// Directly call 'CEdit' to avoid the 'CExtNumEdit::SetWindowTextW' method to be called.
		CEdit::SetWindowText( _T("") );
	}
	else
	{
		Update();
	}
}

void CExtNumEdit::SetDrawBorder( bool bActivate, COLORREF crBorderColor )
{
	m_bDrawBorderActivated = bActivate;
	m_crBorderColor = crBorderColor;
	RedrawWindow( NULL, NULL, RDW_FRAME | RDW_INVALIDATE );
}

void CExtNumEdit::ResetDrawBorder( void )
{
	m_bDrawBorderActivated = false;
	RedrawWindow( NULL, NULL, RDW_FRAME | RDW_INVALIDATE );
}

void CExtNumEdit::SetToolTip( bool bActivate, CWnd *pParent, CString str )
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

void CExtNumEdit::Update()
{
	if( true == m_bEmptyMode )
	{
		return;
	}

	CString str = CNumString::Update();
	SetWindowText( str );
}

BEGIN_MESSAGE_MAP( CExtNumEdit, CEdit )
	ON_MESSAGE( WM_USER_NEWDOCUMENT, OnNewDocument )
	ON_CONTROL_REFLECT_EX( EN_CHANGE, OnEnChange )
	ON_WM_CHAR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_MESSAGE( WM_USER_UNITCHANGE, OnUnitChange )
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_PAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_GETDLGCODE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

BOOL CExtNumEdit::PreTranslateMessage( MSG *pMsg )
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

		_ValidateInput();
		
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
	
	// Do not interpret keys between 'A' to 'Z' (Exception made for 'E').
	if( pMsg->message == WM_KEYDOWN && pMsg->wParam >= 0x41 && pMsg->wParam <= 0x5A )
	{
		if( pMsg->wParam != 0x45 ) //VK_E (0x45)
		{
			return true;
		}
	}

	return CEdit::PreTranslateMessage( pMsg );
}

LRESULT CExtNumEdit::OnNewDocument( WPARAM wParam, LPARAM lParam )
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

BOOL CExtNumEdit::OnEnChange()
{
	// If notification is not allowed, we return 'TRUE' to warn that the notification has been eaten by the
	// child. And in this case, the parent is not notify.
	return ( true == m_bBlockOnEnChangeNotification ) ? TRUE : FALSE;
}

void CExtNumEdit::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
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

	// Get the string after adding new character.
	CString NewStr;
	GetWindowText( NewStr );

	// Define the cursor position.
	int iPosCursor, iEndChar;
	GetSel( iPosCursor, iEndChar );

	// Format Numerical string.
	CString FormattedStr = CNumString::FormatChar( nChar, str, NewStr, &iPosCursor );

	// Write formatted string into the edit.
	// Remark: will fire the 'ON_EN_CHANGE' event! This is why we block here to allow the parent
	// to be notified at the end of this method when the current SI value is well updated.
	m_bBlockOnEnChangeNotification = true;
	SetWindowText( FormattedStr );
	m_bBlockOnEnChangeNotification = false;

	SetSel( iPosCursor, iPosCursor );

	// Update internal variables.
	double dValue = 0.0;
	ReadDouble( NewStr, &dValue );
	SetCurrentValSI( CDimValue::CUtoSI( m_ePhysType, dValue ) );

	// Manually notify the parent.
	GetParent()->SendMessage( WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(), EN_CHANGE ), (LPARAM)GetSafeHwnd() );
}

void CExtNumEdit::OnSetFocus( CWnd *pOldWnd )
{
	if( false == m_bOnSetFocusHandler )
	{
		return CEdit::OnSetFocus( pOldWnd );
	}
	
	PostMessage( EM_SETSEL, 0, -1 );
	CEdit::OnSetFocus( pOldWnd );
}

void CExtNumEdit::OnKillFocus( CWnd *pNewWnd )
{
	if( false == m_bOnKillFocusHandler )
	{
		return CEdit::OnKillFocus( pNewWnd );
	}
	
	_ValidateInput();

	// Do the default function.
	CEdit::OnKillFocus( pNewWnd );
}

LRESULT CExtNumEdit::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	if( true == m_bEmptyMode || false == m_bOnUnitChangeHandler )
	{
		return 0;
	}

	// Verify one value already exist, if it doesn't, do not refresh the edit control.
	CString str, strUnknown;
	GetWindowText( str );
	strUnknown = TASApp.LoadLocalizedString( IDS_UNKNOWN );
	
	if( str != _T("") && str != strUnknown )
	{
		SetWindowTextW( GetCurrentString() );
	}

	return 0;
}

HBRUSH CExtNumEdit::CtlColor( CDC *pDC, UINT nCtlColor )
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

void CExtNumEdit::OnPaint( )
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

void CExtNumEdit::OnNcCalcSize( BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR *lpncsp )
{
	CEdit::OnNcCalcSize( bCalcValidRects, lpncsp );

	if( VertAlignStyle::VAS_Undefined != m_eVertAlignStyle )
	{
		CRect rectWnd, rectClient;
	
		// Calculate client area height needed for a font.
		CFont *pFont = GetFont();
		CRect rectText;
		rectText.SetRectEmpty();
	
		CDC *pDC = GetDC();
	
		CFont *pOld = pDC->SelectObject( pFont );
		CString str;
		GetWindowText( str );
		pDC->DrawText( str, rectText, DT_CALCRECT | DT_LEFT );
		UINT uiVClientHeight = rectText.Height();
	
		pDC->SelectObject( pOld );
		ReleaseDC( pDC );
	
		// Calculate NC area to center text.
	
		GetClientRect( rectClient );
		GetWindowRect( rectWnd );

		ClientToScreen( rectClient );

		switch( m_eVertAlignStyle )
		{
			case VertAlignStyle::VAS_Top:
				rectWnd.OffsetRect( -rectWnd.left, -rectWnd.top );
				
				m_rectNCTop = rectWnd;
				m_rectNCTop.DeflateRect( 0, 0, 0, rectWnd.Height() );

				m_rectNCBottom = rectWnd;
				m_rectNCBottom.DeflateRect( 0, uiVClientHeight, 0, 0 );

				lpncsp->rgrc[0].bottom -= ( rectWnd.Height() - uiVClientHeight );
				break;
			
			case VertAlignStyle::VAS_Center:
				{
					UINT uiCY = ( rectWnd.Height() - rectClient.Height() ) / 2;
					UINT uiCX = ( rectWnd.Width() - rectClient.Width() ) / 2;
					UINT uiCenterOffset = ( rectClient.Height() - uiVClientHeight ) / 2;

					rectWnd.OffsetRect( -rectWnd.left, -rectWnd.top );

					m_rectNCTop = rectWnd;
					m_rectNCTop.DeflateRect( uiCX, uiCY, uiCX, uiCenterOffset + uiVClientHeight + uiCY );

					m_rectNCBottom = rectWnd;
					m_rectNCBottom.DeflateRect( uiCX, uiCenterOffset + uiVClientHeight + uiCY, uiCX, uiCY );

					lpncsp->rgrc[0].top +=uiCenterOffset;
					lpncsp->rgrc[0].bottom -= uiCenterOffset;

					lpncsp->rgrc[0].left +=uiCX;
					lpncsp->rgrc[0].right -= uiCY;
				}
				break;

			case VertAlignStyle::VAS_Bottom:
				rectWnd.OffsetRect( -rectWnd.left, -rectWnd.top );

				m_rectNCTop = rectWnd;
				m_rectNCTop.DeflateRect( 0, 0, 0, uiVClientHeight );

				m_rectNCBottom = rectWnd;
				m_rectNCBottom.DeflateRect( 0, rectWnd.Height(), 0, 0 );

				lpncsp->rgrc[0].top += ( rectWnd.Height() - uiVClientHeight );
				break;
			}
	}
}

void CExtNumEdit::OnNcPaint() 
{
	Default();

	CWindowDC dc( this );

	if( VertAlignStyle::VAS_Undefined != m_eVertAlignStyle )
	{
		CBrush Brush( GetSysColor( COLOR_3DFACE ) );

		dc.FillRect( m_rectNCBottom, &Brush );
		dc.FillRect( m_rectNCTop, &Brush );
	}

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

UINT CExtNumEdit::OnGetDlgCode() 
{
	if( VertAlignStyle::VAS_Undefined != m_eVertAlignStyle && TRUE == m_rectNCTop.IsRectEmpty() )
	{
		SetWindowPos( NULL, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED );
	}
	
	return CEdit::OnGetDlgCode();
}

void CExtNumEdit::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( false == m_bBlockSelection )
	{
		CEdit::OnLButtonDown( nFlags, point );
	}
}

void CExtNumEdit::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	if( false == m_bBlockSelection )
	{
		CEdit::OnLButtonDblClk( nFlags, point );
	}
}

BOOL CExtNumEdit::OnSetCursor( CWnd *pWnd, UINT nHitTest, UINT message )
{
	if( false == m_bBlockCursorChange )
	{
		return CEdit::OnSetCursor( pWnd, nHitTest, message );
	}

	return 1;
}

void CExtNumEdit::_CancelDrawBorderMode()
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

void CExtNumEdit::_ValidateInput( void )
{
	// Init variables.
	CString str;
	double val = 0.0;
	GetWindowText( str );
	ReadDouble( str, &val );

	// Get the value expressed in SI units.
	m_dCurrentValSI = CDimValue::CUtoSI( m_ePhysType, val );

	// HYS-900: Verify we do not go higher than the min or max values. Previously done in CNumString::FormatChar
	if (eDouble == m_EditType && 0.0 != m_dCurrentValSI &&
		(m_dCurrentValSI < m_dMinDblValue || m_dCurrentValSI > m_dMaxDblValue))
	{
		if (m_dCurrentValSI < m_dMinDblValue)
		{
			m_dCurrentValSI = m_dMinDblValue;
		}
		else if (m_dCurrentValSI > m_dMaxDblValue)
		{
			m_dCurrentValSI = m_dMaxDblValue;
		}
		SetWindowTextW(GetCurrentString());
	}

	// Correct the string to be sure the units are shown correctly, following the m_bUnitsUsed value.
	// It could append if the user push the "delete" button on the units and kill the focus.
	if( _T("") != str && true == IsUnitsUsed() )
	{
		SetWindowTextW( GetCurrentString() );
	}
}
