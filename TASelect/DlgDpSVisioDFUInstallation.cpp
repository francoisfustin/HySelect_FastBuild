#include "stdafx.h"
#include "afxdialogex.h"
#include "DpSVisioIncludes.h"
#include "DlgDpSVisioDFUInstallation.h"


CDlgDpSVisioDFUInstallation::CDlgDpSVisioDFUInstallation( CWnd *pParent )
	: CDialogEx( CDlgDpSVisioDFUInstallation::IDD, pParent )
{
	m_bOKPressed = false;
}

void CDlgDpSVisioDFUInstallation::SetText( CString strText, COLORREF TextColor, int iSize, bool bBold )
{
	m_StaticInfo.SetWindowTextW( strText );
	m_StaticInfo.SetTextColor( TextColor );
	m_StaticInfo.SetFontSize( iSize );
	m_StaticInfo.SetFontBold( bBold );
	m_StaticInfo.Invalidate();
	m_StaticInfo.UpdateWindow();
}

void CDlgDpSVisioDFUInstallation::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_STATICINFO, m_StaticInfo );
	DDX_Control( pDX, IDC_BUTTONOK, m_ButtonOK );
}

BOOL CDlgDpSVisioDFUInstallation::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_ButtonOK.SetWindowTextW( TASApp.LoadLocalizedString( IDS_OK ) );
	m_StaticInfo.SetWindowText( _T("") );

	// HYS-1336 : If the background color is not defined, the 'CExtStatic::CtlColor' considers the object as transparent.
	// So the text will overlapped with the previous each time it changes.
	m_StaticInfo.SetBackColor( ::GetSysColor( COLOR_3DFACE ) );

	return TRUE;
}

void CDlgDpSVisioDFUInstallation::OnOK()
{
	m_bOKPressed = true;
}

void CDlgDpSVisioDFUInstallation::OnCancel()
{
}

BEGIN_MESSAGE_MAP( CDlgDpSVisioDFUInstallation, CDialogEx )
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_BN_CLICKED( IDC_BUTTONOK, &OnOK )
END_MESSAGE_MAP()

void CDlgDpSVisioDFUInstallation::OnNcPaint()
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

void CDlgDpSVisioDFUInstallation::OnPaint()
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

	CRect rectButOK;
	m_ButtonOK.GetWindowRect( &rectButOK );
	ScreenToClient( &rectButOK );
	newPos.y = ( rectButOK.top - textCalc.Height() ) >> 1;

	m_StaticInfo.SetWindowPos( NULL, newPos.x, newPos.y, textCalc.Width(), textCalc.Height(), SWP_NOZORDER );

	pDC->BitBlt( ClientRect.left, ClientRect.top, ClientRect.Width(), ClientRect.Height(), &dcMemory, ClientRect.left, ClientRect.top, SRCCOPY );
	dcMemory.SelectObject( pOldBitmap );
	clBitmap.DeleteObject();
	dcMemory.DeleteDC();

	ReleaseDC( pDC );
}
