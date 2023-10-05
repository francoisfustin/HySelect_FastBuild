//
// DlgTip.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TASelect.h"
#include <winreg.h>
#include <sys\stat.h>
#include <sys\types.h>
#include "resource.h"
#include "Global.h"

#include "Utilities.h"
#include "DlgTip.h"

// CG: This file added by 'Tip of the Day' component.

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgTip dialog

CDlgTip::CDlgTip( CWnd* pParent )
	: CDialogExt( IDD_DLGTIP, pParent )
{
	// We need to find out what if tip dialog must be shown whent HySelect starts.
	// If startup does not exist, we assume that the Tips on startup is checked TRUE.
	CWinApp* pApp = AfxGetApp();
	m_fShowStartup = !pApp->GetProfileInt( _TIPS_SECTIONNAME, _TIPS_SHOWSTARTUP, 0 );
	m_uiCurrentTip = max(1, pApp->GetProfileInt( _TIPS_SECTIONNAME, _TIPS_CURRENTTIP, 1 ) );
	m_uiIDCount = GetpXmlTipTab()->GetIDCount();
}

CDlgTip::~CDlgTip()
{
	AfxGetApp()->WriteProfileInt( _TIPS_SECTIONNAME, _TIPS_CURRENTTIP, (int)m_uiCurrentTip );
}
        
void CDlgTip::DoDataExchange( CDataExchange* pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Check( pDX, IDC_STARTUP, m_fShowStartup );
	DDX_Text( pDX, IDC_TIPSTRING, m_strTip );
}

BEGIN_MESSAGE_MAP( CDlgTip, CDialogExt )
	ON_BN_CLICKED( IDC_NEXTTIP, OnNextTip )
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgTip message handlers

void CDlgTip::OnNextTip()
{
	GetNextTipString();
	UpdateData( FALSE );
}

void CDlgTip::GetNextTipString( void )
{
	WCHAR *pStr = GetpXmlTipTab()->GetIDSStrByPos( m_uiCurrentTip++ );
	if( NULL != pStr )
		m_strTip = pStr;
	else
	{
		m_uiCurrentTip = 1;
		pStr = GetpXmlTipTab()->GetIDSStrByPos( m_uiCurrentTip++ );
	}

	if( m_uiCurrentTip >= m_uiIDCount )
		m_uiCurrentTip = 1;
}

HBRUSH CDlgTip::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	if( IDC_TIPSTRING == pWnd->GetDlgCtrlID() )
		return (HBRUSH)GetStockObject( WHITE_BRUSH );

	return CDialogExt::OnCtlColor( pDC, pWnd, nCtlColor );
}

void CDlgTip::OnOK()
{
	CDialogExt::OnOK();
	
    // Update the startup information stored in the INI file.
	CWinApp* pApp = AfxGetApp();
	pApp->WriteProfileInt( _TIPS_SECTIONNAME, _TIPS_SHOWSTARTUP, !m_fShowStartup );
}

BOOL CDlgTip::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TIPDLG_CAPTION );
	SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TIPDLG_STARTUP );
	GetDlgItem( IDC_STARTUP )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TIPDLG_NEXTTIP );
	GetDlgItem( IDC_NEXTTIP )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TIPDLG_OK );
	GetDlgItem( IDOK )->SetWindowText( str );


	m_BulbBmp.LoadBitmap( IDB_TAH_STAR_BLOW_UP_GRADIENT );
	CWnd* pStatic = GetDlgItem( IDC_STATICFRAME );
	CRect rect;
	pStatic->GetWindowRect( &rect );
	m_BulbBmp.ResizeImage( CSize( rect.Width(), rect.Height() ) );

	OnNextTip();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CDlgTip::OnPaint()
{
	// Device context for painting.
	CPaintDC dc( this );

	// Get paint area for the big static frame control.
	CWnd* pStatic = GetDlgItem( IDC_STATICFRAME );
	CRect rect;
	pStatic->GetWindowRect( &rect );
	ScreenToClient( &rect );

	// Paint the white background rectangle.
	CBrush wbrush;
	wbrush.CreateStockObject( WHITE_BRUSH );
	dc.FillRect( rect, &wbrush );
	rect.InflateRect( 1, 1, 1, 1 );
	dc.DrawEdge( rect, BDR_SUNKENOUTER, BF_TOPLEFT );
	dc.DrawEdge( rect, BDR_SUNKENOUTER, BF_BOTTOMRIGHT );
	rect.DeflateRect( 1, 1, 1, 1 );

	CRect rectBmp = m_BulbBmp.GetSizeImage();
	m_BulbBmp.DrawOnDC( &dc, CPoint( rect.left , rect.top ) );

	CWnd* pStaticTxtTip = GetDlgItem( IDC_TIPSTRING );
	CRect rectTxtTip;
	pStaticTxtTip->GetWindowRect( &rectTxtTip );
	ScreenToClient( &rectTxtTip );

	// Create a font because some unicode character are not well displayed with default font
	LOGFONT LogFont;
	LogFont.lfHeight = -12;
	LogFont.lfWidth = 0;
	LogFont.lfEscapement = 0;
	LogFont.lfOrientation = 0;
	LogFont.lfWeight = 700;
	LogFont.lfItalic = 0;
	LogFont.lfUnderline = 0;
	LogFont.lfStrikeOut = 0;
	LogFont.lfCharSet = 0;
	LogFont.lfOutPrecision = 0;
	LogFont.lfClipPrecision = 0;
	LogFont.lfQuality = 0;
	LogFont.lfPitchAndFamily = 0;
	wcscpy_s( LogFont.lfFaceName, _T("Arial") );

	CFont *pFontTitle = new CFont;
	pFontTitle->CreateFontIndirect( &LogFont );

	CFont *pOldFont = dc.SelectObject( pFontTitle );
	dc.SetTextColor( _TAH_BLACK );

	// Draw out "Did you know..." message next to the bitmap.
	CString strMessage;
	strMessage = TASApp.LoadLocalizedString( CG_IDS_DIDYOUKNOW );
	CRect rectTxt = rect;
	rectTxt.top = rect.top;
	rectTxt.bottom = rectTxtTip.top - 8 ;
	rectTxt.right = rect.right;
	rectTxt.left = rect.left + ( rectBmp.Width() * 11 / 10 ); // width + 10 %
	dc.DrawText( strMessage, -1, rectTxt, DT_VCENTER | DT_SINGLELINE );

	// revert to previous font
	dc.SelectObject(pOldFont);
	delete pFontTitle;

	// Draw a gray line below the Did you know...
	CPen pen;
	pen.CreatePen( PS_SOLID, 1, GetSysColor( COLOR_3DSHADOW ) );
	CPen *pOldPen = dc.SelectObject( &pen );
	dc.MoveTo( rectTxt.left, rectTxt.bottom);
	dc.LineTo( rectTxt.right, rectTxt.bottom);
	dc.SelectObject( pOldPen );
	pen.DeleteObject();
	
	// Do not call CDialogExt::OnPaint() for painting messages
}
