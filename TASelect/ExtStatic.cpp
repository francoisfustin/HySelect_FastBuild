#include "stdafx.h"
#include "ExtStatic.h"
#include "global.h"

IMPLEMENT_DYNAMIC( CExtStatic, CStatic )

CExtStatic::CExtStatic()
	:CStatic()
{
	m_crText = _BLACK;
	m_crTextDisabled = ::GetSysColor( COLOR_3DFACE );
	m_crThinBorderColor = ::GetSysColor( COLOR_3DFACE );
	m_eFontBold = eBool3::eb3Undef;
	m_iFontSize = -1;
	m_bFontPropertyChanged = false;
	m_fThinBorder = false;

	LOGFONT rLogFont;
	::GetObject( (HFONT)GetStockObject( DEFAULT_GUI_FONT ), sizeof( rLogFont ), &rLogFont );
	m_clFictiveFont.CreateFontIndirect( &rLogFont );
}

CExtStatic::~CExtStatic()
{
	// Delete brush.
	if( NULL != m_brBackGnd.GetSafeHandle() )
	{
		m_brBackGnd.DeleteObject();
	}

	if( NULL != m_clFictiveFont.GetSafeHandle() )
	{
		m_clFictiveFont.DeleteObject();
	}

	if( NULL != m_clFont.GetSafeHandle() )
	{
		m_clFont.DeleteObject();
	}

	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		m_ToolTip.DestroyWindow();
	}
}

void CExtStatic::SetTextColor( COLORREF rgb )
{
	// Set text color ref.
	m_crText = rgb;

	// Redraw.
	Invalidate( TRUE );
}

void CExtStatic::SetTextDisabledColor( COLORREF rgb )
{
	// Set text color ref.
	m_crTextDisabled = rgb;

	// Redraw.
	Invalidate( TRUE );
}

void CExtStatic::SetBackColor( COLORREF rgb )
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
}

COLORREF CExtStatic::GetBackColor()
{
	COLORREF clReturn = -1;

	if( NULL !=  m_brBackGnd.GetSafeHandle() )
	{
		clReturn = m_crBackGnd;
	}

	return clReturn;
}

void CExtStatic::SetThinBorder( bool fThinBorder, COLORREF crThinBorderColor )
{
	m_crThinBorderColor = crThinBorderColor;
	m_fThinBorder = fThinBorder;
}

void CExtStatic::SetFontBold( bool fBold )
{
	eBool3 eNewFontBold = ( true == fBold ) ? eBool3::eb3True : eBool3::eb3False;
	
	if( m_eFontBold != eNewFontBold )
	{
		m_eFontBold = eNewFontBold;
		m_bFontPropertyChanged = true;
	}

	if( eBool3::eb3Undef != m_eFontBold )
	{
		CFont *pclCurrentFont = GetFont();
		LOGFONT rLogFont;
		pclCurrentFont->GetLogFont( &rLogFont );

		rLogFont.lfWeight = ( eBool3::eb3True == m_eFontBold ) ? FW_BOLD : FW_NORMAL;

		if( NULL != m_clFont.GetSafeHandle() )
		{
			m_clFont.DeleteObject();
		}

		m_clFont.CreateFontIndirect( &rLogFont );
		SetFont( &m_clFont, FALSE );
	}
}

void CExtStatic::SetFontSize( int iSize )
{
	if( m_iFontSize != iSize )
	{
		m_iFontSize = iSize;
		m_bFontPropertyChanged = true;
	}

	if( -1 != m_iFontSize )
	{
		CFont *pclCurrentFont = GetFont();
		LOGFONT rLogFont;
		pclCurrentFont->GetLogFont( &rLogFont );

		CDC *pDC = GetDC();
		rLogFont.lfHeight = -MulDiv( m_iFontSize, GetDeviceCaps( pDC->GetSafeHdc(), LOGPIXELSY ), 72 );
		ReleaseDC( pDC );

		if( NULL != m_clFont.GetSafeHandle() )
		{
			m_clFont.DeleteObject();
		}

		m_clFont.CreateFontIndirect( &rLogFont );
		SetFont( &m_clFont, FALSE );
	}
}

void CExtStatic::SetToolTip( CString strToolTip )
{
	_VerifyTooltip();
	m_ToolTip.Activate( TRUE );
	m_ToolTip.UpdateTipText( strToolTip, this );
}

void CExtStatic::SetTextAndToolTip( CString strText, CString strToolTip )
{
	SetWindowText( strText );
	SetToolTip( strToolTip );
}

void CExtStatic::SetTextAndToolTip( CString strTextAndToolTip )
{
	SetWindowText( strTextAndToolTip );
	SetToolTip( strTextAndToolTip );
}

void CExtStatic::ResetToolTip()
{
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		m_ToolTip.DelTool( this, 1 );
		m_ToolTip.DestroyWindow();
	}
}

BEGIN_MESSAGE_MAP( CExtStatic, CStatic )
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_PAINT()
END_MESSAGE_MAP()

// See the 'TAS Project\TASelect-Sln\Doc\Code\CStatic.md' for a well understanding of this code.
HBRUSH CExtStatic::CtlColor( CDC *pDC, UINT nCtlColor )
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

void CExtStatic::OnPaint( )
{
	CStatic::OnPaint();

	if( true == m_fThinBorder )
	{
		CDC *pDC = GetDC();
		CBrush clBrushBorder;
		clBrushBorder.CreateSolidBrush( m_crThinBorderColor );

		CRect rectClient;
		GetClientRect( &rectClient );
		pDC->FrameRect( &rectClient, &clBrushBorder );
		
		ReleaseDC( pDC );
	}
}

BOOL CExtStatic::PreTranslateMessage( MSG *pMsg )
{
	switch( pMsg->message )
	{
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:

			if( NULL != m_ToolTip.GetSafeHwnd() )
			{
				m_ToolTip.RelayEvent( pMsg );
			}

			break;

		default:
			break;
	}

	return CStatic::PreTranslateMessage( pMsg );
}

void CExtStatic::_VerifyTooltip()
{
	if( NULL == m_ToolTip.GetSafeHwnd() )
	{
		ModifyStyle( 0, SS_NOTIFY );
		
		// Create the tooltip.
		m_ToolTip.Create( this, TTS_NOPREFIX );
		m_ToolTip.SetMaxTipWidth( 1000 );

		CRect Rect;
		GetClientRect( &Rect );
		m_ToolTip.AddToolWindow( this, (LPCTSTR)_T( "" ) );

		m_ToolTip.SetDelayTime( TTDT_AUTOPOP, 5000 );
	}
}
