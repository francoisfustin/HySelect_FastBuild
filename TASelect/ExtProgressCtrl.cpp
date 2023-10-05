#include "stdafx.h"
#include "ExtProgressCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CExtProgressCtrl::CExtProgressCtrl()
{
	m_bShowText	= false;
	m_strText.Empty();
	m_ForegroundColor = ::GetSysColor( COLOR_HIGHLIGHT );
	m_BackgroundColor = RGB( 255, 255,  250 );
	m_TextForegroundColor = ::GetSysColor( COLOR_HIGHLIGHT );
	m_TextBackgroundColor = RGB( 255, 255,  250 );
}

void CExtProgressCtrl::SetShowText( bool bShow )
{
	if( TRUE == ::IsWindow( m_hWnd ) && m_bShowText != bShow )
	{
		Invalidate();
	}

	m_bShowText = bShow;
}

void CExtProgressCtrl::SetForegroundColor( COLORREF ForegroundColor )
{
	m_ForegroundColor = ForegroundColor;
}

void CExtProgressCtrl::SetBackgroundColor( COLORREF BackgroundColor )
{
	m_BackgroundColor = BackgroundColor;
}

void CExtProgressCtrl::SetTextForegroundColor( COLORREF TextForegroundColor )
{
	m_TextForegroundColor = TextForegroundColor;
}

void CExtProgressCtrl::SetTextBackgroundColor( COLORREF TextBackgroundColor )
{
	m_TextBackgroundColor = TextBackgroundColor;
}

COLORREF CExtProgressCtrl::GetForegroundColor()
{
	return m_ForegroundColor;
}

COLORREF CExtProgressCtrl::GetBackgorundColor()
{
	return m_BackgroundColor;
}

COLORREF CExtProgressCtrl::GetTextForegroundColor()
{
	return m_TextForegroundColor;
}

COLORREF CExtProgressCtrl::GetTextBackgorundColor()
{
	return m_TextBackgroundColor;
}

BEGIN_MESSAGE_MAP( CExtProgressCtrl, CProgressCtrl )
 	ON_WM_ERASEBKGND()
 	ON_WM_PAINT()
	ON_MESSAGE( WM_SETTEXT, OnSetText )
	ON_MESSAGE( WM_GETTEXT, OnGetText )
END_MESSAGE_MAP()

BOOL CExtProgressCtrl::OnEraseBkgnd( CDC *pDC )
{
	return TRUE;
}

void CExtProgressCtrl::OnPaint()
{
	CProgressCtrl::OnPaint();

	if( PBS_MARQUEE == ( PBS_MARQUEE & GetStyle() ) )
	{
		// Don't paint anything if the progress control bar is in marquee mode.
		return;
	}

	SetRedraw( FALSE );

	CDC *pDC = GetDC();

	int iMin;
	int iMax;
	GetRange( iMin, iMax );

	double dPercent = (double)( GetPos() - iMin ) / ( (double)( iMax - iMin ) );

	CRect ClientRect;
	CRect LeftRect;
	CRect RightRect;
	GetClientRect( ClientRect);
	LeftRect = ClientRect;
	RightRect = ClientRect;

	LeftRect.right = LeftRect.left + (int)( ( LeftRect.right - LeftRect.left ) * dPercent );
	RightRect.left = LeftRect.right;
	LeftRect.DeflateRect( 1, 1 );
	RightRect.DeflateRect( 1, 1 );

	CDC dcMemory;
	dcMemory.CreateCompatibleDC( pDC );

	CBitmap clBitmap;
	clBitmap.CreateCompatibleBitmap( pDC, ClientRect.Width(), ClientRect.Height() );

	CBitmap *pOldBitmap = dcMemory.SelectObject( &clBitmap );

	// Border.
	COLORREF refBorderColor = RGB( 188, 188, 188 );
	dcMemory.FillSolidRect( ClientRect, refBorderColor );

	// Background.
	CRect ClientRectDeflated( ClientRect );
	ClientRectDeflated.DeflateRect( 1, 1 );
	dcMemory.FillSolidRect( ClientRectDeflated, m_BackgroundColor );

	// Left part.
	dcMemory.FillSolidRect( LeftRect, m_ForegroundColor );

	if( true == m_bShowText )
	{
		CString str;

		if( 0 != m_strText.GetLength() )
		{
			str = m_strText;
		}
		else
		{
			str.Format( _T("%d%%"), (int)( dPercent * 100.0 ) );
		}

		dcMemory.SetBkMode( TRANSPARENT );

		CFont font;
		font.CreateFont( -MulDiv( 9, GetDeviceCaps( pDC->GetSafeHdc(), LOGPIXELSY ), 72 ), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("MS Shell Dlg") );
		
		CFont *pOldFont = dcMemory.SelectObject( &font );
		
		// Draw the left part of the text with the background color (on the progress bar).
		CRgn rgn;
		rgn.CreateRectRgn( LeftRect.left, LeftRect.top, LeftRect.right, LeftRect.bottom );
		dcMemory.SelectClipRgn( &rgn );
		dcMemory.SetTextColor( m_TextBackgroundColor );

		dcMemory.DrawText( str, ClientRectDeflated, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

		// Draw the right part of the text with the foreground color (not on the progress bar).
		rgn.DeleteObject();
		rgn.CreateRectRgn( RightRect.left, RightRect.top, RightRect.right, RightRect.bottom );
		dcMemory.SelectClipRgn( &rgn );
		dcMemory.SetTextColor( m_TextForegroundColor );

		dcMemory.DrawText( str, ClientRectDeflated, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

		rgn.DeleteObject();

		dcMemory.SelectObject( pOldFont );
		font.DeleteObject();
	}

	pDC->BitBlt( ClientRect.left, ClientRect.top, ClientRect.Width(), ClientRect.Height(), &dcMemory, ClientRect.left, ClientRect.top, SRCCOPY );
	dcMemory.SelectObject( pOldBitmap );
	clBitmap.DeleteObject();
	dcMemory.DeleteDC();

	ReleaseDC( pDC );

	SetRedraw( TRUE );
}

LRESULT CExtProgressCtrl::OnSetText( WPARAM wParam, LPARAM lParam )
{
    LRESULT result = Default();
    LPCTSTR ptcText = (LPCTSTR) lParam;

    if( ( NULL == ptcText && 0 != m_strText.GetLength() ) ||
        ( NULL != ptcText && ( m_strText != ptcText ) ) )
    {
        m_strText = ptcText;
        Invalidate();
    }

    return result;
}

LRESULT CExtProgressCtrl::OnGetText( WPARAM wParam, LPARAM lParam )
{
	LPTSTR szText = (LPTSTR)lParam;
	UINT uiTextMax = (UINT)wParam;

	if( 0 == _tcsncpy( szText, m_strText, uiTextMax ) )
	{
        return 0;
	}

	return min( uiTextMax, (UINT)m_strText.GetLength() );
}
