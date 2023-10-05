#include "stdafx.h"
#include "afxdialogex.h"
#include "DpSVisioIncludes.h"
#include "DlgDpSVisioUnplugPlug.h"


CDlgDpSVisioUnplugPlug::CDlgDpSVisioUnplugPlug( WaitMode eWaitMode, CWnd *pParent )
	: CDialogEx( CDlgDpSVisioUnplugPlug::IDD, pParent )
{
	m_eWaitMode = eWaitMode;
	m_nTimer = (UINT_PTR)0;
	m_bBmpAlreadyChanged = false;
}

CDlgDpSVisioUnplugPlug::~CDlgDpSVisioUnplugPlug()
{
	if( NULL != GetSafeHwnd () && (UINT_PTR)0 != m_nTimer )
	{
		KillTimer( m_nTimer );
	}

	if( INVALID_HANDLE_VALUE != (HBITMAP)m_clHourglassBkgBmp )
	{
		m_clHourglassBkgBmp.DeleteObject();
	}
	
	if( INVALID_HANDLE_VALUE != (HBITMAP)m_clHourglassFrgBmp )
	{
		m_clHourglassFrgBmp.DeleteObject();
	}

}

void CDlgDpSVisioUnplugPlug::SetText( CString strText, COLORREF TextColor, int iSize, bool bBold )
{
	m_StaticInfo.SetWindowTextW( strText );
	m_StaticInfo.SetTextColor( TextColor );
	m_StaticInfo.SetFontSize( iSize );
	m_StaticInfo.SetFontBold( bBold );
	m_StaticInfo.Invalidate();
	m_StaticInfo.UpdateWindow();
}

void CDlgDpSVisioUnplugPlug::SetHourglass( int iStep, int iFinal )
{
	m_eWaitMode = WM_Hourglass;
	m_ButtonCancel.EnableWindow( FALSE );
	m_ButtonCancel.ShowWindow( SW_HIDE );

	m_bCurrentAngle = 0.0;
	float fPi = (float)3.1415926535;
	m_bStep = (float)2.0 / ( (float)iFinal / (float)iStep ) * fPi;
	m_bFinal = 2 * fPi;

	m_nTimer = SetTimer( _TIMERID_DLGDPSVISIOUNPLUGPLUG, iStep, NULL );
}

void CDlgDpSVisioUnplugPlug::StopHourglass()
{
	if( (UINT_PTR)0 != m_nTimer )
	{
		KillTimer( m_nTimer );
		m_nTimer = (UINT_PTR)0;
	}
}

BOOL CDlgDpSVisioUnplugPlug::PreTranslateMessage( MSG *pMsg )
{
	if( WM_KEYDOWN == pMsg->message )
    {
        if( VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam )
        {
            return TRUE;                // Do not process further
        }
    }

    return CWnd::PreTranslateMessage( pMsg );
}

void CDlgDpSVisioUnplugPlug::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATICINFO, m_StaticInfo);
	DDX_Control(pDX, IDC_BUTTONCANCEL, m_ButtonCancel );
}

BOOL CDlgDpSVisioUnplugPlug::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( str );

	m_StaticInfo.SetWindowText( _T("") );

	// HYS-1336 : If the background color is not defined, the 'CExtStatic::CtlColor' considers the object as transparent.
	// So the text will overlapped with the previous each time it changes.
	m_StaticInfo.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

	if( WM_Hourglass == m_eWaitMode )
	{
		m_ButtonCancel.EnableWindow( FALSE );
		m_ButtonCancel.ShowWindow( SW_HIDE );
	}

	return TRUE;
}

void CDlgDpSVisioUnplugPlug::OnOK()
{
	// Don't call base class! (it's to avoid to loose focus when pressing [ENTER] key).
}

void CDlgDpSVisioUnplugPlug::OnCancel()
{
	// Don't call base class! (it's to avoid to loose focus when pressing [ESCAPE] key).
}

BEGIN_MESSAGE_MAP( CDlgDpSVisioUnplugPlug, CDialogEx )
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTONCANCEL, &CDlgDpSVisioUnplugPlug::OnBnClickedCancel)
END_MESSAGE_MAP()

void CDlgDpSVisioUnplugPlug::OnNcPaint()
{
	CDialogEx::OnNcPaint();

	CDC *pDC = GetWindowDC();
	CRect WindowRect;
	GetWindowRect( &WindowRect );

	// Draw outer-edge.
	CPen newPen;
	newPen.CreatePen( PS_SOLID, 1, RGB( 24, 131, 215 ) );
	CPen *pOldPen = pDC->SelectObject( &newPen );

	pDC->SelectStockObject( NULL_BRUSH );

	CRect DrawRect( WindowRect );
	DrawRect.OffsetRect( -WindowRect.TopLeft() );
	pDC->Rectangle( &DrawRect );

	pDC->SelectObject( &pOldPen );
	newPen.DeleteObject();

	ReleaseDC( pDC );
}

void CDlgDpSVisioUnplugPlug::OnPaint()
{
	CDialogEx::OnPaint();

	CDC *pDC = GetDC();

	// Create memory device context.
	CRect ClientRect;
	GetClientRect( &ClientRect );

	CDC dcMemory;
	dcMemory.CreateCompatibleDC( pDC );

	CBitmap clBitmap;
	clBitmap.CreateCompatibleBitmap( pDC, ClientRect.Width(), ClientRect.Height() );

	CBitmap *pOldBitmap = dcMemory.SelectObject( &clBitmap );

	// Inner edge.
	dcMemory.FillSolidRect( ClientRect, RGB( 24, 131, 215 ) );

	// Background.
	CRect DeflateRect = ClientRect;
	DeflateRect.DeflateRect( 2, 2, 1, 1 );
	dcMemory.FillSolidRect( DeflateRect, RGB( 240, 240, 240 ) );

	// Compute text height.
	CRect TextRect = DeflateRect;
	TextRect.DeflateRect( 2, 2, 1, 1 );
	CString strText;
	m_StaticInfo.GetWindowText( strText );
	CFont *pStaticFont = m_StaticInfo.GetFont();
	CFont *pOldFont = dcMemory.SelectObject( pStaticFont );
	CRect textCalc = TextRect;
	textCalc.OffsetRect( -TextRect.left, -TextRect.top );
	textCalc.DeflateRect( 10, 0, 10, 0 );
	dcMemory.DrawText( strText, &textCalc, DT_LEFT | DT_CALCRECT | DT_NOPREFIX | DT_WORDBREAK );
	dcMemory.SelectObject( pOldFont );

	// Align text.
	CPoint newPos;
	newPos.x = ( ClientRect.Width() - textCalc.Width() ) >> 1;

	if( WM_Infinite == m_eWaitMode || (UINT_PTR)0 == m_nTimer )
	{
		CRect rectButCancel;
		m_ButtonCancel.GetWindowRect( &rectButCancel );
		ScreenToClient( &rectButCancel );
		newPos.y = ( rectButCancel.top - textCalc.Height() ) >> 1;
	}
	else
	{
		newPos.y = ( ( ClientRect.Height() >> 1 ) - textCalc.Height() ) >> 1;
	}

	m_StaticInfo.SetWindowPos( NULL, newPos.x, newPos.y, textCalc.Width(), textCalc.Height(), SWP_NOZORDER );

	// Draw hourglass if exist.
	if( WM_Hourglass == m_eWaitMode && (UINT_PTR)0 != m_nTimer )
	{
		CPoint centerPos;
		centerPos.x = ClientRect.Width() >> 1;
		centerPos.y = ( ( ClientRect.Height() >> 2 ) * 3 ) - 10;

		// Draw bitmap.
		BITMAP rBitmap;

		// Prepare bitmaps for transparent points.
		if( false == m_bBmpAlreadyChanged )
		{
			m_clHourglassBkgBmp.LoadBitmap( IDB_HOURGLASS_BKG );
			m_clHourglassBkgBmp.GetBitmap( &rBitmap );

			DWORD dwPixelCount = rBitmap.bmWidth * rBitmap.bmHeight * sizeof( RGBQUAD );
			BYTE *pbBuffer = new BYTE[dwPixelCount];
			m_clHourglassBkgBmp.GetBitmapBits( dwPixelCount, pbBuffer );
			BYTE *pData = pbBuffer;

			for( int y = 0; y < rBitmap.bmHeight; y++ )
			{
				for( int x = 0; x < rBitmap.bmWidth; x++ )
				{
					 pData[0] = (BYTE)( (DWORD)pData[0] * pData[3] / 255 );
					 pData[1] = (BYTE)( (DWORD)pData[1] * pData[3] / 255 );
					 pData[2] = (BYTE)( (DWORD)pData[2] * pData[3] / 255 );
					 pData += 4;
				}
			}

			m_clHourglassBkgBmp.SetBitmapBits( dwPixelCount, pbBuffer );
			delete []pbBuffer;

			m_clHourglassFrgBmp.LoadBitmap( IDB_HOURGLASS_FRG );
			m_clHourglassFrgBmp.GetBitmap( &rBitmap );

			dwPixelCount = rBitmap.bmWidth * rBitmap.bmHeight * sizeof( RGBQUAD );
			pbBuffer = new BYTE[dwPixelCount];
			m_clHourglassFrgBmp.GetBitmapBits( dwPixelCount, pbBuffer );
			pData = pbBuffer;

			for( int y = 0; y < rBitmap.bmHeight; y++ )
			{
				for( int x = 0; x < rBitmap.bmWidth; x++ )
				{
					 pData[0] = (BYTE)( (DWORD)pData[0] * pData[3] / 255 );
					 pData[1] = (BYTE)( (DWORD)pData[1] * pData[3] / 255 );
					 pData[2] = (BYTE)( (DWORD)pData[2] * pData[3] / 255 );
					 pData += 4;
				}
			}

			m_clHourglassFrgBmp.SetBitmapBits( dwPixelCount, pbBuffer );
			delete []pbBuffer;

			m_bBmpAlreadyChanged = true;
		}
		else
		{
			m_clHourglassBkgBmp.GetBitmap( &rBitmap );
		}

		// Draw hourglass background.
		CDC dcMemoryHourglassBkg;
		dcMemoryHourglassBkg.CreateCompatibleDC( &dcMemory );

		CBitmap *pOldBmpHourglassBkg = dcMemoryHourglassBkg.SelectObject( &m_clHourglassBkgBmp );
		BLENDFUNCTION rBlendFunction = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
		dcMemory.AlphaBlend( centerPos.x - ( rBitmap.bmWidth >> 1 ) - 1, centerPos.y - ( rBitmap.bmHeight >> 1 ) - 1,  rBitmap.bmWidth, rBitmap.bmHeight, 
			&dcMemoryHourglassBkg, 0, 0, rBitmap.bmWidth, rBitmap.bmHeight, rBlendFunction );
		
		dcMemoryHourglassBkg.SelectObject( pOldBmpHourglassBkg );
		dcMemoryHourglassBkg.DeleteDC();

		if( 0.0 == m_bCurrentAngle )
		{
			// Just draw a vertical line.
			CPen newPen;
			newPen.CreatePen( PS_SOLID, 1, RGB( 205, 0, 0 ) );
			CPen *pOldPen = dcMemory.SelectObject( &newPen );

			dcMemory.MoveTo( centerPos );
			dcMemory.LineTo( CPoint( centerPos.x, centerPos.y - ( rBitmap.bmWidth >> 1 ) ) );

			dcMemory.SelectObject( pOldPen );
			newPen.DeleteObject();
		}
		else
		{
			int iOffset = ( rBitmap.bmWidth >> 1 );

			POINT ptStart;
			ptStart.x = centerPos.x;
			ptStart.y = centerPos.y - iOffset;

			double fSin = sin( m_bCurrentAngle );
			double fCos = cos( m_bCurrentAngle );

			POINT ptEnd;
			ptEnd.x = (LONG)( (float)centerPos.x + ( (float)iOffset * fSin ) );
			ptEnd.y = (LONG)( (float)centerPos.y - ( (float)iOffset * fCos ) );

			CRect BoundingRect = CRect( centerPos.x, centerPos.y, centerPos.x, centerPos.y );
			BoundingRect.InflateRect( iOffset, iOffset );

			// Create a path that will served as a clip region.
			dcMemory.BeginPath();

			dcMemory.MoveTo( centerPos.x, centerPos.y );
			dcMemory.LineTo( ptStart.x, ptStart.y );
			dcMemory.SetArcDirection( AD_CLOCKWISE );
			dcMemory.ArcTo( &BoundingRect, ptStart, ptEnd );
			dcMemory.LineTo( centerPos.x, centerPos.y );

			dcMemory.CloseFigure();
			dcMemory.EndPath();

			CRgn clRgn;
			clRgn.CreateFromPath( &dcMemory );
			dcMemory.SelectClipRgn( &clRgn );

			CDC dcMemoryHourglassFrg;
			dcMemoryHourglassFrg.CreateCompatibleDC( &dcMemory );

			CBitmap *pOldBmpHourglassFrg = dcMemoryHourglassFrg.SelectObject( &m_clHourglassFrgBmp );
			dcMemory.AlphaBlend( centerPos.x - iOffset - 1, centerPos.y - iOffset - 1,  rBitmap.bmWidth, rBitmap.bmHeight, 
				&dcMemoryHourglassFrg, 0, 0, rBitmap.bmWidth, rBitmap.bmHeight, rBlendFunction );
		
			dcMemoryHourglassFrg.SelectObject( pOldBmpHourglassFrg );
			dcMemoryHourglassFrg.DeleteDC();
		}
	}

	pDC->BitBlt( ClientRect.left, ClientRect.top, ClientRect.Width(), ClientRect.Height(), &dcMemory, ClientRect.left, ClientRect.top, SRCCOPY );
	dcMemory.SelectObject( pOldBitmap );
	clBitmap.DeleteObject();
	dcMemory.DeleteDC();

	ReleaseDC( pDC );
}

void CDlgDpSVisioUnplugPlug::OnTimer( UINT_PTR nIDEvent )
{
	if( _TIMERID_DLGDPSVISIOUNPLUGPLUG != nIDEvent )
	{
		return;
	}

	if( m_bFinal == m_bCurrentAngle )
	{
		KillTimer( m_nTimer );
		m_nTimer = (UINT_PTR)0;
		return;
	}

	m_bCurrentAngle += m_bStep;
	
	if( m_bCurrentAngle > m_bFinal )
	{
		m_bCurrentAngle = m_bFinal;
	}

	Invalidate();
	UpdateWindow();
}

void CDlgDpSVisioUnplugPlug::OnBnClickedCancel()
{
	CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOUNPLUGPLUG_CANCEL );
	
	if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONWARNING) )
	{
		::SendMessage( m_pParentWnd->GetSafeHwnd(), WM_USER_DPSVISIO_STOPDLGUNPLUGPLUG, 0, 0 );		
	}
}
