// XGroupBox.cpp  Version 1.0 - see article at www.codeproject.com
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// Description:
//     XGroupBox is an MFC control that displays a flicker-free groupbox 
//     with text and/or icon.  You can use this to display either a 
//     standard groupbox or a header-only groupbox.
//
// History
//     Version 1.0 - 2008 September 2
//     - Initial public release
//
// License:
//     This software is released under the Code Project Open License (CPOL),
//     which may be found here:  http://www.codeproject.com/info/eula.aspx
//     You are free to use this software in any way you like, except that you 
//     may not sell this source code.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     I accept no liability for any damage or loss of business that this 
//     software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <VersionHelpers.h>
#include "XGroupBox.h"
#include "XVisualStyles.h"
#include "TASelect.h"


#ifndef __noop
#if _MSC_VER < 1300
#define __noop ((void)0)
#endif
#endif

#undef TRACE
#undef TRACERECT
#define TRACE __noop
#define TRACERECT __noop

//=============================================================================
// if you want to see the TRACE output, uncomment this line:
//#include "XTrace.h"
//=============================================================================

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable : 4996)			// disable bogus deprecation warning

//=============================================================================
static CXVisualStyles g_xpStyle;
COLORREF CXGroupBox::m_defaultGrayPalette[256] = { 0 };
//=============================================================================

//=============================================================================
// set resource handle (in case used in DLL)
//=============================================================================
#ifdef _USRDLL
#define AFXMANAGESTATE AfxGetStaticModuleState
#else
#define AFXMANAGESTATE AfxGetAppModuleState
#endif

#define ID_CHECKBOX	0xFFFF
IMPLEMENT_DYNAMIC( CXGroupBox, CButton )

BEGIN_MESSAGE_MAP( CXGroupBox, CButton )
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_COMMAND( ID_CHECKBOX, OnClicked )
	ON_WM_LBUTTONDOWN()
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

CXGroupBox::CXGroupBox() :
	m_crText( GetSysColor( COLOR_BTNTEXT ) ),
	m_rectInitialClientSize( CRect( 0, 0, 0, 0 ) ),
	m_bInOffice2007Mainframe( false ),
	m_bUseRibbonColor( false ),
	m_crBackground( GetSysColor( COLOR_BTNFACE ) ),
	m_crBorder( CLR_INVALID ),
	m_iXMargin( DEFAULT_X_MARGIN ),
	m_iYMargin( 0 ),
	m_iYOffset( -1 ),
	m_iHeaderHeight( -1 ),
	m_iIconSpacing( DEFAULT_ICON_SPACING ),
	m_hIcon( NULL ),
	m_hGrayIcon( NULL ),
	m_bDestroyIcon( false ),
	m_iIconSize( 0 ),
	m_rectIcon( CRect( 0, 0, 0, 0 ) ),
	m_rect( CRect( 0, 0, 0, 0 ) ),
	m_CheckBoxRect( CRect( 0, 0, 0, 0 ) ),
	m_sizeText( CSize( 0, 0 ) ),
	m_bThemed( false ),
	m_bEnableTheme( true ),
	m_bShowDisabledState( true ),
	m_bForceShowDisable( false ),
	m_bGrayPaletteSet( false ),
	m_eStyle( groupbox ),
	m_eBorderStyle( flat ),
	m_eIconAlignment( left ),
	m_eControlAlignment( left ),
	m_bExpandCollapseMode( false ),
	m_uiGroupID( 0 ),
	m_pfNotificationHandler( NULL )
{
	TRACE( _T("in CXGroupBox::CXGroupBox\n") );
}

CXGroupBox::~CXGroupBox()
{
	if( NULL != m_clFont.GetSafeHandle() )
	{
		m_clFont.DeleteObject();
	}
	
	if( NULL != m_hIcon && true == m_bDestroyIcon )
	{
		::DestroyIcon( m_hIcon );
	}
	
	if( NULL != m_hGrayIcon )
	{
		::DestroyIcon( m_hGrayIcon );
	}

	m_hGrayIcon = NULL;
}

void CXGroupBox::SetAlignment( ALIGNMENT eAlign, bool bRedraw )
{
	m_eControlAlignment = eAlign;

	if( true == bRedraw )
	{
		RedrawWindow();
	}
}

void CXGroupBox::SetBold( bool bBold, bool bRedraw )
{
	LOGFONT lf;
	memset( &lf, 0, sizeof( lf ) );

	m_clFont.GetLogFont( &lf );

	lf.lfWeight = ( true == bBold ) ? FW_BOLD : FW_NORMAL;

	// Force string width to be recalculated.
	m_sizeText.cx = 0;

	SetFont( &lf, bRedraw );
}

void CXGroupBox::SetBckgndColor( COLORREF cr, bool bRedraw )
{
	m_crBackground = cr;

	if( true == bRedraw )
	{
		RedrawWindow(); 
	}
}

void CXGroupBox::SetBorderColor( COLORREF cr, bool bRedraw )
{
	m_crBorder = cr; 

	if( true == bRedraw )
	{
		RedrawWindow(); 
	}
}

void CXGroupBox::SetBorderStyle( BORDER_STYLE eStyle, bool bRedraw )
{
	TRACE( _T("in CXGroupBox::SetBorderStyle: %d\n"), eStyle );

	m_eBorderStyle = eStyle;

	if( true == bRedraw )
	{
		RedrawWindow(); 
	}
}

void CXGroupBox::SetControlStyle( CONTROL_STYLE eStyle, bool bRedraw )
{
	m_eStyle = eStyle;

	if( true == bRedraw )
	{
		RedrawWindow();
	}
}

void CXGroupBox::SetDisabledStyle( bool bShowDisabledState, bool bRedraw )
{
	m_bShowDisabledState = bShowDisabledState;

	if( true == bRedraw )
	{
		RedrawWindow();
	}
}

void CXGroupBox::SetForceShowDisable( bool bForceShowDisable, bool bRedraw )
{
	m_bForceShowDisable = bForceShowDisable;

	if( true == bRedraw )
	{
		RedrawWindow();
	}
}

void CXGroupBox::SetFont( CFont *pFont, bool bRedraw )
{
	ASSERT( NULL != pFont );

	if( NULL == pFont )
	{
		pFont = GetSafeFont();
	}

	if( pFont != NULL )
	{
		LOGFONT lf;
		memset( &lf, 0, sizeof( lf ) );

		pFont->GetLogFont( &lf );

		SetFont( &lf, bRedraw );
	}
}

void CXGroupBox::SetFont( LOGFONT *pLogFont, bool bRedraw )
{
	TRACE( _T("in CXGroupBox::SetFont =================\n") );
	ASSERT( pLogFont );

	if( pLogFont != NULL )
	{
		if( m_clFont.GetSafeHandle() != NULL )
		{
			m_clFont.DeleteObject();
		}

		VERIFY( m_clFont.CreateFontIndirect( pLogFont ) );

		// Force string width to be recalculated.
		m_sizeText.cx = 0;

		if( true == bRedraw )
		{
			RedrawWindow();
		}
	}
}

void CXGroupBox::SetFont( LPCTSTR lpszFaceName, int iPointSize, bool bRedraw )
{
	// Null face name is ok - we will use current font.

	LOGFONT lf;
	memset( &lf, 0, sizeof( lf ) );

	if( ( NULL == lpszFaceName ) || ( _T('\0') == lpszFaceName[0] ) )
	{
		m_clFont.GetLogFont( &lf );
	}
	else
	{
		_tcsncpy( lf.lfFaceName, lpszFaceName, sizeof( lf.lfFaceName ) / sizeof( TCHAR ) - 1 );
	}

	lf.lfHeight = GetFontHeight( iPointSize );

	SetFont( &lf, bRedraw );
}

void CXGroupBox::SetIcon( HICON hIcon, bool bForDisableMode, UINT uiIconSize, bool bRedraw )
{
	TRACE( _T("in CXGroupBox::SetIcon 1\n") );

	m_iIconSize = uiIconSize;

	if( true == bForDisableMode && NULL != m_hGrayIcon )
	{
		::DestroyIcon( m_hGrayIcon );
		m_hGrayIcon = NULL;
	}

	if( false == bForDisableMode && NULL != m_hIcon && true == m_bDestroyIcon )
	{
		::DestroyIcon( m_hIcon );
		m_hIcon = NULL;
	}
	
	if( false == bForDisableMode )
	{
		m_hIcon = hIcon;
	}
	else
	{
		m_hGrayIcon = hIcon;
	}

	if( NULL == m_hIcon )
	{
		m_iIconSize = 0;
	}

	m_bDestroyIcon = false;

	if( true == bRedraw )
	{
		RedrawWindow();
	}
}

void CXGroupBox::SetIcon( UINT uiIconId, bool bForDisableMode, UINT uiIconSize, bool bRedraw )
{
	TRACE( _T("in CXGroupBox::SetIcon 2\n") );
	AFX_MANAGE_STATE( AFXMANAGESTATE() );

	m_iIconSize = uiIconSize;

	if( true == bForDisableMode && NULL != m_hGrayIcon )
	{
		::DestroyIcon( m_hGrayIcon );
		m_hGrayIcon = NULL;
	}

	if( false == bForDisableMode && NULL != m_hIcon && true == m_bDestroyIcon )
	{
		::DestroyIcon( m_hIcon );
		m_hIcon = NULL;
	}

	if( 0 != uiIconId )
	{
		if( false == bForDisableMode )
		{
			m_hIcon = (HICON) ::LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( uiIconId ), IMAGE_ICON, m_iIconSize, m_iIconSize, 0 );
		}
		else
		{
			m_hGrayIcon = (HICON) ::LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( uiIconId ), IMAGE_ICON, m_iIconSize, m_iIconSize, 0 );
		}

		ASSERT( NULL != m_hIcon );
	}
	else
	{
		m_hIcon = NULL;
	}

	if( NULL == m_hIcon )
	{
		m_iIconSize = 0;
	}

	m_bDestroyIcon = true;

	if( true == bRedraw )
	{
		RedrawWindow();
	}
}

void CXGroupBox::SetIconAlignment( ALIGNMENT eAlign, bool bRedraw )
{
	m_eIconAlignment = eAlign;

	if( true == bRedraw )
	{
		RedrawWindow();
	}
}

void CXGroupBox::SetIconSpacing( int iIconSpacing, bool bRedraw )
{ 
	m_iIconSpacing = iIconSpacing; 

	if( true == bRedraw )
	{
		RedrawWindow(); 
	}
}

void CXGroupBox::SetImageList( CImageList *pclImageList, int iIconNo, bool bForDisableMode, bool bRedraw )
{
	m_iIconSize = 0;

	if( false == bForDisableMode && NULL != m_hIcon && true == m_bDestroyIcon )
	{
		::DestroyIcon( m_hIcon );
		m_hIcon = NULL;
	}

	if( true == bForDisableMode && NULL != m_hGrayIcon )
	{
		::DestroyIcon( m_hGrayIcon );
		m_hGrayIcon = NULL;
	}

	if ( NULL == pclImageList )
	{
		ASSERT_RETURN;
	}

	if ( NULL == pclImageList->m_hImageList )
	{
		ASSERT_RETURN;
	}

	ASSERT( iIconNo < pclImageList->GetImageCount() );

	if( iIconNo < pclImageList->GetImageCount() )
	{
		// Get image size from CImageList.
		int cx = 0;
		::ImageList_GetIconSize( pclImageList->m_hImageList, &cx, &m_iIconSize );

		if( false == bForDisableMode )
		{
			m_hIcon = (HICON)pclImageList->ExtractIcon( iIconNo );
			ASSERT( NULL != m_hIcon );
		}
		else
		{
			m_hGrayIcon = (HICON)pclImageList->ExtractIcon( iIconNo );
			ASSERT( NULL != m_hGrayIcon );
		}

		if( NULL == m_hIcon )
		{
			m_iIconSize = 0;
		}

		m_bDestroyIcon = true;

		if( true == bRedraw )
		{
			RedrawWindow();
		}
	}
}

void CXGroupBox::SetMargins( int iXMargin, int iYMargin, bool bRedraw )
{ 
	m_iXMargin = iXMargin;
	m_iYMargin = iYMargin;

	if( true == bRedraw )
	{
		RedrawWindow();
	}
}

void CXGroupBox::SetTextColor( COLORREF cr, bool bRedraw )
{ 
	m_crText = cr;

	if( true == bRedraw )
	{
		RedrawWindow();
	}
}

void CXGroupBox::SetWindowText( LPCTSTR lpszText, bool bRedraw )
{
	AFX_MANAGE_STATE( AFXMANAGESTATE() );

	CString strText = _T("");

	// Force string width to be recalculated.
	m_sizeText.cx = 0;

	// lpszText can be pointer to normal text string,
	// or it can be resource id constructed with
	// MAKEINTRESOURCE()

	if( lpszText != NULL )
	{
		// Is this a text string or an id?
		if( 0 == HIWORD( lpszText ) )
		{
			// id.
			UINT uiId = LOWORD( (UINT)(UINT_PTR)lpszText );
			VERIFY( strText.LoadString( uiId ) );
		}
		else
		{
			// String.
			strText = lpszText;
		}
	}

	CButton::SetWindowText( strText );

	if( true == bRedraw )
	{
		RedrawWindow();
	}
}

bool CXGroupBox::GetBold()
{
	bool bReturn = false;

	CFont *pFont = GetFont(); 

	if( pFont != NULL )
	{
		LOGFONT lf; 
		pFont->GetLogFont( &lf );
		bReturn = ( FW_BOLD == lf.lfWeight ) ? true : false;
	}

	return bReturn;
}

bool CXGroupBox::GetFont( LOGFONT *pLF )
{
	bool bReturn = false;

	ASSERT( NULL != pLF );

	if( pLF != NULL )
	{
		memset( pLF, 0, sizeof( LOGFONT ) );
		CFont *pFont = GetFont(); 

		if( pFont != NULL )
		{
			pFont->GetLogFont( pLF );
			bReturn = TRUE;
		}
	}
	
	return bReturn;
}

CString CXGroupBox::GetFontFaceName()
{
	CString strFaceName = _T("MS Sans Serif");

	CFont *pFont = GetFont(); 
	
	if( pFont != NULL )
	{
		LOGFONT lf; 
		pFont->GetLogFont( &lf );
		strFaceName = lf.lfFaceName;
	}

	return strFaceName;
}

int CXGroupBox::GetFontPointSize()
{
	int rc = 8;

	CFont *pFont = GetFont(); 
	
	if( pFont != NULL )
	{
		LOGFONT lf; 
		pFont->GetLogFont( &lf );
		rc = GetFontPointSize( lf.lfHeight );
	}
	
	return rc;
}

int CXGroupBox::GetHeaderHeight()
{
	if( -1 == m_iHeaderHeight )
	{
		ComputeHeaderHeight();
	}

	return m_iHeaderHeight;
}

void CXGroupBox::EnableTheme( bool bEnable, bool bRedraw )
{
	m_bEnableTheme = bEnable;

	if( true == m_bEnableTheme && true == m_bThemed )
	{
		m_crBorder = CLR_INVALID;
		m_crText = CLR_INVALID;

		// Get theme color for text.
		HRESULT hr = g_xpStyle.GetThemeColor( BP_GROUPBOX, GBS_NORMAL, TMT_TEXTCOLOR, &m_crText );

		if( ( hr != S_OK ) || ( CLR_INVALID == m_crText ) )
		{
			TRACE( _T("WARNING - GetThemeColor failed\n") );
			m_crText = GetSysColor( COLOR_HIGHLIGHT );
		}
	}

	if( true == bRedraw )
	{
		RedrawWindow(); 
	}
}

void CXGroupBox::SaveChildList()
{
	m_vechWndChild.clear();

	RECT rectWindow;
	::GetWindowRect( m_hWnd, &rectWindow );

	HWND hWndChild = 0;
	HWND hWndParent = ::GetParent( m_hWnd );
	
	if( TRUE == IsWindow( hWndParent ) )
	{
		hWndChild = ::GetWindow( hWndParent, GW_CHILD );
	}


	while( hWndChild != NULL )
	{
		if( hWndChild != m_hWnd )
		{
			RECT rectChild;
			::GetWindowRect( hWndChild, &rectChild );

			// Check if child rect is entirely contained within window.
			if( ( rectChild.left >= rectWindow.left ) && ( rectChild.right <= rectWindow.right ) 
					&& ( rectChild.top >= rectWindow.top ) && ( rectChild.bottom <= rectWindow.bottom ) )
			{
				TRACE( _T("found child window 0x%X\n"), hWndChild );
				m_vechWndChild.push_back( hWndChild );
			}
		}
		
		// Get next child control.
		hWndChild = ::GetWindow( hWndChild, GW_HWNDNEXT );
	}
}

bool CXGroupBox::EnableWindow( bool bEnable, bool bRecurseChildren )
{
	bool bReturn = ( TRUE == CButton::EnableWindow( ( true == bEnable ) ? TRUE : FALSE ) ) ? true : false;

	if( NULL != m_CheckBoxWnd.GetSafeHwnd() )
	{
		m_CheckBoxWnd.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	}

	m_bShowDisabledState = !bEnable;

	if( true == bRecurseChildren )
	{
		EnableChildWindows( m_hWnd, bEnable, false );
	}
	else
	{
		::InvalidateRect( m_hWnd, NULL, FALSE );
	}
	
	return bReturn;
}

bool CXGroupBox::ShowWindow( bool bShow, bool bRecurseChildren )
{
	bool bReturn = ( TRUE == CButton::ShowWindow( ( true == bShow ) ? TRUE : FALSE ) ) ? true : false;

	if( NULL != m_CheckBoxWnd.GetSafeHwnd() )
	{
		m_CheckBoxWnd.ShowWindow( ( true == bShow ) ? TRUE : FALSE );
	}

	m_bShowDisabledState = !bShow;

	if( true == bRecurseChildren )
	{
		ShowChildWindows( m_hWnd, bShow, false );
	}
	else
	{
		::InvalidateRect( m_hWnd, NULL, FALSE );
	}
	
	return bReturn;
}

void CXGroupBox::MoveGroupBox( CRect rect )
{
	if ( 0 == m_vechWndChild.size() )
	{
		return;
	}

	HWND hWndParent = ::GetParent( m_hWnd );
	CRect rectWindow;
	::GetWindowRect( m_hWnd, &rectWindow );
	CWnd::FromHandle( hWndParent )->ScreenToClient( &rectWindow );
	// Extract x and y offset.
	int xOffset = rect.left - rectWindow.left;
	int yOffset = rect.top - rectWindow.top;

	// Move GroupBox.
	::SetWindowPos( m_hWnd, NULL, rectWindow.left + xOffset, rectWindow.top + yOffset, -1, -1, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER );

	// Move Child Windows.
	for( std::vector<HWND>::iterator It = m_vechWndChild.begin(); It != m_vechWndChild.end(); ++It )
	{
		CRect rectChild;
		::GetWindowRect( *It, &rectChild );
		CWnd::FromHandle( hWndParent )->ScreenToClient( &rectChild );
		::SetWindowPos( *It, NULL, rectChild.left+xOffset, rectChild.top+yOffset, -1, -1, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER );
	}
	
	::InvalidateRect( hWndParent, NULL, FALSE );
}

void CXGroupBox::SetCheckBoxStyle( UINT uiStyle )
{
	// Use Text size to determine check box size, useful when we use large font.
	CString strText = _T(" ");

	CClientDC dc( this );
	CFont *pOldFont = dc.SelectObject( GetFont() );
	CSize czText = dc.GetTextExtent( strText );
	dc.SelectObject( pOldFont );
	
	// Move the check box on top of the groupbox.
	CRect rc;
	GetWindowRect( rc );
	this->ScreenToClient( rc );
	rc.left += DEFAULT_X_MARGIN + 3;	// +3 to create space with the horizontal bar.
	rc.top +=1;
	rc.right = rc.left + czText.cy;		// Check box size is linked to the character height (square cy/cy).
	rc.bottom = rc.top + czText.cy;
	m_CheckBoxRect = rc;
	
	if( BS_AUTOCHECKBOX == uiStyle || BS_AUTORADIOBUTTON == uiStyle || BS_CHECKBOX == uiStyle )
	{
		m_CheckBoxWnd.Create( strText, uiStyle | WS_CHILD  | WS_TABSTOP, rc, this, ID_CHECKBOX );
	}
	
	m_CheckBoxWnd.SetFont( GetFont(), true );
	COLORREF crBckgrnd;
	
	if( true == m_bUseRibbonColor )
	{
		crBckgrnd = TASApp.GetBckColor();
	}
	else
	{
		crBckgrnd = m_crBackground;
	}

	CDC *pDC = m_CheckBoxWnd.GetDC();
	pDC->SetBkColor( crBckgrnd );
	
	// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
	m_CheckBoxWnd.ReleaseDC( pDC );
	
	m_CheckBoxWnd.ShowWindow( SW_SHOW );
}

int CXGroupBox::GetCheck() const
{
	int iRet = 0;

	if( m_CheckBoxWnd.GetSafeHwnd() != NULL )
	{
		iRet = m_CheckBoxWnd.GetCheck();
	}

	return iRet;
}

void CXGroupBox::SetCheck( int iCheck )
{
	if( m_CheckBoxWnd.GetSafeHwnd() != NULL )
	{
		m_CheckBoxWnd.SetCheck( iCheck );
		UINT uiStyle = m_CheckBoxWnd.GetButtonStyle();

		if( uiStyle != BS_CHECKBOX )
		{
			EnableChildWindows( m_hWnd, ( BST_CHECKED == iCheck ) ? TRUE : FALSE, false );
		}
	}
}

void CXGroupBox::SetGroupID( UINT uiGroup )
{
	m_uiGroupID = uiGroup;
}

UINT CXGroupBox::GetGroupID() const
{
	return m_uiGroupID;
}

void CXGroupBox::ClickedOnGroup( void )
{
	::PostMessage( m_CheckBoxWnd.GetSafeHwnd(), BM_CLICK, 0, 0 );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CXGroupBox::OnEraseBkgnd( CDC *pDC ) 
{
	// We draw everything.
	return TRUE;
}

void CXGroupBox::OnPaint() 
{
	static bool bInPaint = false;

	if( true == bInPaint )
	{
		return;
	}

	bInPaint = TRUE;
	CPaintDC dc( this );
	CRect rect;
	GetWindowRect( &rect );
	int iWidth = rect.Width();
	int iHeight = rect.Height();
	rect.left = 0;
	rect.top = 0;
	rect.right = iWidth;
	rect.bottom = iHeight;
	DrawItem( &dc, rect );

	bInPaint = FALSE;

	// Do not call CButton::OnPaint() for painting messages
}

LRESULT CXGroupBox::OnNcHitTest( CPoint point )
{
	// From the 'stackoverflow' website:
	//
	// A standard Windows groupbox (i.e., a BUTTON control with BS_GROUPBOX style) appears to return HTTRANSPARENT in response to WM_NCHITTEST. 
	// Since the control claims to be transparent, Windows sends the mouse-move events to its parent window instead.
	// If you handle WM_NCHITTEST yourself and return HTCLIENT, then the groupbox will get mouse-move events. 
	// What's not clear is why Windows returns HTTRANSPARENT by default, but at least the problem has been independently confirmed.

	LRESULT lResult = HTTRANSPARENT;

	if( true == m_bExpandCollapseMode )
	{
		if( CONTROL_STYLE::groupbox == m_eStyle )
		{
			lResult = HTCLIENT;
		}
		else
		{
			// Check if point is well on the icon.
			ScreenToClient( &point );

			if( NULL != m_hIcon && FALSE == m_rectIcon.IsRectNull() && TRUE == m_rectIcon.PtInRect( point ) )
			{
				lResult = HTCLIENT;
			}
		}
	}
	else
	{
		lResult = CButton::OnNcHitTest( point );
	}

	return lResult;
}

void CXGroupBox::OnLButtonDown( UINT nFlags, CPoint point )
{
	bool bCallBase = true;

	if( true == m_bExpandCollapseMode && NULL != m_hIcon )
	{
		// Check if user has clicked on the icon.
		if( TRUE == m_rectIcon.PtInRect( point ) )
		{
			// Change state.
			SetControlStyle( ( CONTROL_STYLE::groupbox == m_eStyle ) ? CONTROL_STYLE::header : CONTROL_STYLE::groupbox );
			
			// Change size of the client area.
			if( TRUE == m_rectInitialClientSize.IsRectNull() )
			{
				GetClientRect( &m_rectInitialClientSize );
			}

			CRect rectNewSize( m_rectInitialClientSize );

			if( CONTROL_STYLE::groupbox == m_eStyle )
			{
				SetWindowPos( NULL, -1, -1, m_rectInitialClientSize.Width(), m_rectInitialClientSize.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER  );

				if( m_CheckBoxWnd.GetSafeHwnd() != NULL )
				{
					m_CheckBoxWnd.ShowWindow( SW_SHOW );
					m_CheckBoxWnd.EnableWindow( TRUE );
				}
			}
			else
			{
				rectNewSize.bottom = rectNewSize.top + GetHeaderHeight();

				SetWindowPos( NULL, -1, -1, rectNewSize.Width(), rectNewSize.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER );

				if( m_CheckBoxWnd.GetSafeHwnd() != NULL )
				{
					m_CheckBoxWnd.ShowWindow( SW_HIDE );
					m_CheckBoxWnd.EnableWindow( FALSE );
				}
			}

			// Draw state.
			CDC *pDC = GetDC();
			DrawItem( pDC, rectNewSize );

			// Prevent parent.
			if( m_pfNotificationHandler != NULL )
			{
				m_pfNotificationHandler->OnGroupIconClicked( this );
			}

			bCallBase = false;
		}
	}
	
	if( true == bCallBase )
	{
		CButton::OnLButtonDown( nFlags, point );
	}
}

void CXGroupBox::OnClicked() 
{
	int iCheck = m_CheckBoxWnd.GetCheck();
	UINT uiStyle = m_CheckBoxWnd.GetButtonStyle();
	
	if( uiStyle != BS_CHECKBOX )
	{
		SetCheck( iCheck );
	}
	
	if( BS_AUTORADIOBUTTON == uiStyle )
	{
		CWnd *pWnd = GetParent()->GetWindow( GW_CHILD );
		CRect rcWnd, rcTest;

		while( pWnd != NULL )
		{
			if( TRUE == pWnd->IsKindOf( RUNTIME_CLASS( CXGroupBox ) ) )
			{
				CXGroupBox *pT = (CXGroupBox*)pWnd;

				if( pT->GetGroupID() == m_uiGroupID && pT != this )
				{
					pT->SetCheck( 0 );
				}
			}
			
			pWnd = pWnd->GetWindow( GW_HWNDNEXT );
		}		
	}
	
	// Warn the parent.
	::SendMessage( GetParent()->GetSafeHwnd(), WM_COMMAND, GetDlgCtrlID(), 0 );
}

void CXGroupBox::PreSubclassWindow() 
{
	TRACE( _T("in CXGroupBox::PreSubclassWindow\n") );

	// Remove any border bits.
	ModifyStyle( WS_BORDER | SS_SUNKEN, 0 );
	ModifyStyleEx( WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE, 0 );

	// Check theming.
	if( TRUE == ( g_xpStyle.IsThemeActive() && g_xpStyle.IsAppThemed() ) )
	{
		m_bThemed = ( TRUE == g_xpStyle.OpenThemeData( NULL, L"BUTTON" ) ) ? true : false;
	}
	else
	{
		m_bThemed = false;
	}

	if( true == m_bThemed )
	{
		TRACE( _T("themed\n") );
		m_crText = CLR_INVALID;
		HRESULT hr = g_xpStyle.GetThemeColor( BP_GROUPBOX, GBS_NORMAL, TMT_TEXTCOLOR, &m_crText );
		
		if( ( hr != S_OK ) || ( CLR_INVALID == m_crText ) )
		{
			TRACE( _T("WARNING - GetThemeColor failed\n") );
			m_crText = GetSysColor( COLOR_HIGHLIGHT );
		}
	}
	else
	{
		TRACE( _T("not themed\n") );
		m_crText = GetSysColor( COLOR_BTNTEXT );
	}

	// Create font for this control.
	CFont *pFont = GetSafeFont();
	ASSERT( NULL != pFont );

	if( pFont != NULL )
	{
		ASSERT( NULL != pFont->GetSafeHandle() );

		// Create the font for this control.
		LOGFONT lf;
		pFont->GetLogFont( &lf );
		lf.lfCharSet = DEFAULT_CHARSET;

		if( m_clFont.GetSafeHandle() != NULL )
		{
			m_clFont.DeleteObject();
		}

		VERIFY( m_clFont.CreateFontIndirect( &lf ) );
	}
	
	CButton::PreSubclassWindow();
}

HICON CXGroupBox::CreateGrayscaleIcon( HICON hIcon )
{
	HICON hGrayIcon = NULL;

	ASSERT( NULL != hIcon );

	if( hIcon != NULL )
	{
		if( false == m_bGrayPaletteSet )
		{
			for( int i = 0; i < 256; i++ )
			{
				m_defaultGrayPalette[i] = RGB( 255 - i, 255 - i, 255 - i );
			}

			m_bGrayPaletteSet = TRUE;
		}

		hGrayIcon = CreateGrayscaleIcon( hIcon, m_defaultGrayPalette );
	}

	return hGrayIcon;
}

//=============================================================================
// Author:  Gladstone, with fixes by poxui
//          http://www.codeproject.com/KB/graphics/Create_GrayscaleIcon.aspx
//
HICON CXGroupBox::CreateGrayscaleIcon( HICON hIcon, COLORREF *pPalette )
{
	HICON hGrayIcon = NULL;

	if( NULL == hIcon )
	{
		ASSERTA_RETURN( NULL );
	}

	HDC hdc = ::GetDC( NULL );

	BITMAPINFO bmpInfo = { 0 };
	bmpInfo.bmiHeader.biSize  = sizeof( BITMAPINFOHEADER );

	ICONINFO icInfo = { 0 };

	if( TRUE == ::GetIconInfo( hIcon, &icInfo ) )
	{
		if( ::GetDIBits( hdc, icInfo.hbmColor, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS ) != 0 )
		{
			// ===== fix by poxui to adjust size of BITMAPINFO
			const int iSize = sizeof( BITMAPINFOHEADER ) + sizeof( RGBQUAD ) * bmpInfo.bmiHeader.biClrUsed;
			BITMAPINFO* pBmpInfo = (BITMAPINFO*)new BYTE[iSize + 100];
			memset( pBmpInfo, 0, iSize + 100 );
			pBmpInfo->bmiHeader = bmpInfo.bmiHeader;
			pBmpInfo->bmiHeader.biCompression = BI_RGB;
			
			LPDWORD lpBits = NULL;

			if( ::GetDIBits( hdc, icInfo.hbmColor, 0, bmpInfo.bmiHeader.biHeight, lpBits, pBmpInfo, DIB_RGB_COLORS ) != 0 )
			{
				// ===== fix by poxui to force 32bit loading =====
				bmpInfo.bmiHeader.biPlanes = 1;
				bmpInfo.bmiHeader.biBitCount = 32;
				bmpInfo.bmiHeader.biSizeImage = bmpInfo.bmiHeader.biWidth * 4 * bmpInfo.bmiHeader.biHeight;
				bmpInfo.bmiHeader.biClrUsed = 0;
				bmpInfo.bmiHeader.biClrImportant = 0;
				bmpInfo.bmiHeader.biCompression = BI_RGB;

				SIZE sz;
				sz.cx = bmpInfo.bmiHeader.biWidth;
				sz.cy = bmpInfo.bmiHeader.biHeight;
				DWORD c1 = sz.cx * sz.cy;
	
				lpBits = (LPDWORD)::GlobalAlloc( GMEM_FIXED, c1 * 4 );
	
				if( lpBits != NULL && ::GetDIBits( hdc, icInfo.hbmColor, 0, sz.cy, lpBits, &bmpInfo, DIB_RGB_COLORS ) != 0 )
				{
					LPBYTE lpBitsPtr = (LPBYTE)lpBits;
					UINT off  = 0;
	
					for( UINT i = 0; i < c1; i++ )
					{
						DWORD dwBits = lpBitsPtr[0] + lpBitsPtr[1] + lpBitsPtr[2];
						off = (UINT)( 255 - ( dwBits / 3 ) );
	
						if( lpBitsPtr[3] != 0 || off != 255 )
						{
							if( 0 == off )
							{
								off = 1;
							}
	
							lpBits[i] = pPalette[off] | (lpBitsPtr[3] << 24 );
						}
	
						lpBitsPtr += 4;
					}
	
					ICONINFO icGrayInfo = { 0 };
					icGrayInfo.hbmColor = ::CreateCompatibleBitmap( hdc, sz.cx, sz.cy );
	
					if( icGrayInfo.hbmColor != NULL )
					{
						::SetDIBits( hdc, icGrayInfo.hbmColor, 0, sz.cy, lpBits, &bmpInfo, DIB_RGB_COLORS );
	
						icGrayInfo.hbmMask = icInfo.hbmMask;
						icGrayInfo.fIcon = TRUE;
	
						hGrayIcon = ::CreateIconIndirect( &icGrayInfo );
	
						::DeleteObject( icGrayInfo.hbmColor );
					}
	
					::GlobalFree( lpBits );
					lpBits = NULL;
				}
			}
			delete [] (BYTE*) pBmpInfo;
		}

		::DeleteObject( icInfo.hbmColor );
		::DeleteObject( icInfo.hbmMask );
	}

	::ReleaseDC( NULL, hdc );

	return hGrayIcon;
}

void CXGroupBox::DrawBorder( CDC *pDC, CRect &rect )
{
	if( groupbox == m_eStyle )
	{
		TRACE( _T("DrawBorder: groupbox\n") );

		// Alen: Verify the OS version. JC: Disable theme only if GroupBox is in Mainframe.
		// Do not allow Windows versions previous to Vista to use the border theme if GroupBox is in Mainframe.
		// That avoids barely visible group box border in Office 2007 mainframe in default xpStyle on Windows XP.
		bool bInMainframeOnWinXP = false;

		if( FALSE == IsWindowsVistaOrGreater() && true == m_bInOffice2007Mainframe )
		{
			m_bEnableTheme = false;
			bInMainframeOnWinXP = true;
		}

		// User color overrides themes.
		if( m_crBorder != CLR_INVALID )
		{
			DrawEtchedRectangle( pDC, rect, m_crBorder );
		}
		else if( g_xpStyle.m_hTheme != NULL && true == m_bEnableTheme )
		{
			TRACE( _T("drawing with theme\n") );
			g_xpStyle.DrawThemeBackground( pDC->m_hDC, BP_GROUPBOX, GBS_NORMAL, &rect, NULL );
		}
		else
		{
			// Not themed and no user color.
			if( true == bInMainframeOnWinXP )
			{
				DrawEtchedRectangle( pDC, rect, GetSysColor( COLOR_BTNHILIGHT ) );
			}
			else
			{
				DrawEtchedRectangle( pDC, rect, GetSysColor( COLOR_BTNSHADOW ) );
			}
		}
	}
	else
	{
		// Header style.

		TRACE( _T("DrawBorder: header\n") );

		if( ( g_xpStyle.m_hTheme != NULL && true == m_bEnableTheme) || ( m_crBorder != CLR_INVALID ) )
		{
			COLORREF crBorder = m_crBorder;

			if( CLR_INVALID == crBorder )
			{
				crBorder = GetSysColor( COLOR_BTNSHADOW );
			}

			DrawEtchedLine( pDC, rect, crBorder );
		}
		else
		{
			TRACE( _T("not themed ====================================================\n") );
			// Not themed and no user color - draw etched line.

			DrawEtchedLine( pDC, rect, GetSysColor( COLOR_BTNSHADOW ) );
		}
	}
}

void CXGroupBox::DrawEtchedLine( CDC *pDC, CRect &rect, COLORREF cr )
{
	COLORREF crDark = ( TRUE == IsWindowEnabled() ) ? cr : GetSysColor( COLOR_BTNSHADOW );
	CPen penDark( PS_SOLID, 1, crDark );
	CPen *pOldPen = pDC->SelectObject( &penDark );

	pDC->MoveTo( rect.left, rect.top );
	pDC->LineTo( rect.right, rect.top );

	if( etched == m_eBorderStyle )
	{
		CPen penLight( PS_SOLID, 1, GetSysColor( COLOR_BTNHIGHLIGHT ) );
		pDC->SelectObject( &penLight );
		pDC->MoveTo( rect.left, rect.top + 1 );
		pDC->LineTo( rect.right, rect.top + 1 );
	}

	if( pOldPen != NULL )
	{
		pDC->SelectObject( pOldPen );
	}
}

void CXGroupBox::DrawEtchedRectangle( CDC *pDC, CRect &rect, COLORREF cr )
{
	TRACE( _T("in CXGroupBox::DrawEtchedRectangle\n") );
	COLORREF crDark = ( TRUE == IsWindowEnabled() ) ? cr : GetSysColor( COLOR_BTNSHADOW );
	CPen penDark( PS_SOLID, 1, crDark );
	CPen *pOldPen = pDC->SelectObject( &penDark );
		
	if( etched == m_eBorderStyle )
	{
		// Border is 2 pixels, so deflate bottom and right sides by 1.
		CPen penLight( PS_SOLID, 2, GetSysColor( COLOR_BTNHIGHLIGHT ) );
		pDC->SelectObject( &penLight );
		rect.DeflateRect( 0, 0, 1, 1 );
		rect.OffsetRect( 1, 1 );
		// White rectangle, 2 pixels wide.
		pDC->Rectangle( &rect );
		rect.OffsetRect( -1, -1 );
	}

	// Dark rectangle - overlay white rectangle.
	pDC->SelectObject( &penDark );
	pDC->Rectangle( &rect );

	if( pOldPen != NULL )
	{
		pDC->SelectObject( pOldPen );
	}
}

void CXGroupBox::DrawHeader( CDC *pDC, CRect &rectText, CRect &rectIcon )
{
	if( right == m_eIconAlignment )
	{
		// Icon is on the right.

		// There is no spacing if left-aligned 'header'.
		if( ( groupbox == m_eStyle ) || ( left != m_eControlAlignment ) )
		{
			rectText.left += 3;
		}

		rectIcon.left = DrawText( pDC, rectText );

		if( 0 != m_sizeText.cx )
		{
			rectIcon.left += m_iIconSpacing;
		}

		DrawIcon( pDC, rectIcon );
	}
	else
	{
		// Icon is on left.

		// There is no spacing if left-aligned 'header'.
		if( ( groupbox == m_eStyle ) || ( left != m_eControlAlignment ) )
		{
			rectIcon.left += 3;
		}

		rectIcon.right = rectIcon.left + m_iIconSize;
		rectText.left = DrawIcon( pDC, rectIcon );
		
		if( NULL != m_hIcon )
		{
			rectText.left += m_iIconSpacing;
		}

		DrawText( pDC, rectText );
	}
}

int CXGroupBox::DrawIcon( CDC *pDC, CRect &rect )
{
	int rc = rect.left;		// if no icon just return start position

	if( NULL != m_hIcon )
	{
		HICON hIcon = m_hIcon;

		if( ( FALSE == IsWindowEnabled() && true == m_bShowDisabledState )
				|| true == m_bForceShowDisable )
		{
			if( NULL == m_hGrayIcon )
			{
				m_hGrayIcon = CreateGrayscaleIcon( m_hIcon );
			}

			hIcon = m_hGrayIcon;
		}

		VERIFY( ::DrawIconEx( pDC->m_hDC, rect.left, rect.top, hIcon, m_iIconSize, m_iIconSize, 0, 0, DI_IMAGE | DI_MASK ) );

		rc = rect.right;	// return new left drawing margin
	}

	return rc;
}

void CXGroupBox::DrawItem( CDC *pDC, CRect &rect )
{
	TRACE( _T("in CXGroupBox::DrawItem\n") );

	if( NULL == pDC )
	{
		ASSERT_RETURN;
	}

#if 0
	// Get control alignment (for icon and text)
	DWORD dwStyle = GetStyle() & 0xFF;
	switch( dwStyle )
	{
		default:
		case SS_LEFT:
			m_eControlAlignment = left;
			break;

		case SS_CENTER:
			m_eControlAlignment = center;
			break;

		case SS_RIGHT:
			m_eControlAlignment = right;
			break;
	}
#endif

	CRect rectItem( rect );
	CRect rectDraw( rectItem );
	TRACERECT( rectItem );
	CRect rectText( rectDraw );
	m_rectIcon = rectDraw;
	CRect rectFrame( rectDraw );
	CRect rectHeader( rectDraw );
	
	if( 0 == m_sizeText.cx )
	{
		// Get string width.
		CString strText = _T("");
		GetWindowText( strText );

		CFont *pOldFont = pDC->SelectObject( &m_clFont );

		if( false == strText.IsEmpty() )
		{
			m_sizeText = pDC->GetTextExtent( strText );
		}

		if( pOldFont != NULL )
		{
			pDC->SelectObject( pOldFont );
		}
	}

	m_iYOffset = __max( m_sizeText.cy, m_iIconSize ) + 1;
	TRACE( _T("..... m_nYOffset=%d  m_sizeText.cy=%d  m_nIconSize=%d\n"), m_iYOffset, m_sizeText.cy, m_iIconSize );
	rectText.bottom = rectText.top + m_iYOffset;
	rectHeader.bottom = rectHeader.top + m_iYOffset;
	m_iYOffset = m_iYOffset / 2;
	rectFrame.top += m_iYOffset;

	m_rectIcon.top = rectFrame.top - m_iIconSize / 2;
	m_rectIcon.bottom = m_rectIcon.top + m_iIconSize;

	// Set up for double buffering.
	CDC memDC;
	memDC.CreateCompatibleDC( pDC );
	CBitmap bmp;
	bmp.CreateCompatibleBitmap( pDC, rectDraw.Width(), rectDraw.Height() );
	CBitmap *pOldBitmap = memDC.SelectObject( &bmp );

	// Fill in entire window.
	// Alen modification, retrieve the background color of TASelect.
	COLORREF crBckgrnd;

	if( true == m_bUseRibbonColor )
	{
		crBckgrnd = TASApp.GetBckColor();
	}
	else
	{
		crBckgrnd = m_crBackground;
	}

	memDC.FillSolidRect( &rectItem, crBckgrnd );

	if( groupbox == m_eStyle )
	{
		TRACE( _T("groupbox\n") );
		
		// get parent DC and copy current contents - this will emulate transparent blt'ing.

		CRect rectWindow;
		GetWindowRect( &rectWindow );
		GetParent()->ScreenToClient( &rectWindow );
		rectWindow.top += m_iYOffset * 2;

		CDC *pParentDC = GetParent()->GetDC();
		memDC.BitBlt( 0, m_iYOffset * 2, rectWindow.Width(), rectWindow.Height(), pParentDC, rectWindow.left, rectWindow.top, SRCCOPY );
		VERIFY( ReleaseDC( pParentDC ) );
	}

	// Select NULL brush for border drawing.
	HBRUSH hBrush = (HBRUSH)GetStockObject( NULL_BRUSH );
	HBRUSH hOldBrush = (HBRUSH)::SelectObject( memDC.m_hDC, hBrush );

	if( groupbox == m_eStyle )
	{
		// Erase the old border that exists in the parent DC.
		EraseBorder( &memDC, m_rect, crBckgrnd );
		m_rect = rectFrame;
	}
	
	DrawBorder( &memDC, rectFrame );

	if( hOldBrush != NULL )
	{
		::SelectObject( memDC.m_hDC, hOldBrush );
	}

	// Erase caption area where icon and text will be displayed - this removes frame from that area.
	if( m_sizeText.cx != 0 || m_hIcon != NULL || ( m_hGrayIcon != NULL && ( true == m_bForceShowDisable || FALSE == IsWindowEnabled() ) ) )
	{
		EraseHeaderArea( &memDC, rectHeader, crBckgrnd );
	}

	int iOffset = 0;

	if( m_CheckBoxRect.Width() > 0 && CONTROL_STYLE::groupbox == m_eStyle )
	{
		iOffset = m_CheckBoxRect.left + 6;
	}

	rectText.left = rectHeader.left + iOffset;

	m_rectIcon.left = rectHeader.left + iOffset;

	DrawHeader( &memDC, rectText, m_rectIcon );

	// End double buffering.
	pDC->BitBlt( 0, 0, rectDraw.Width(), rectDraw.Height(), &memDC, 0, 0, SRCCOPY );

	// Swap back the original bitmap.
	if( pOldBitmap != NULL )
	{
		memDC.SelectObject( pOldBitmap );
	}

	if( bmp.GetSafeHandle() != NULL )
	{
		bmp.DeleteObject();
	}

	memDC.DeleteDC();
}

int CXGroupBox::DrawText( CDC *pDC, CRect &rect )
{
	// If no string just return start position.
	int rc = rect.left;

	CString strText = _T("");
	GetWindowText( strText );
	TRACE( _T("strText=<%s>\n"), strText );

	if( false == strText.IsEmpty() )
	{
		COLORREF crText = m_crText;
		
		if( ( FALSE == IsWindowEnabled() && true == m_bShowDisabledState )
				|| true == m_bForceShowDisable )
		{
			crText = GetSysColor( COLOR_GRAYTEXT );
		}

		pDC->SetTextColor( crText );
		pDC->SetBkColor( GetSysColor( COLOR_BTNFACE ) );
		pDC->SetBkMode( TRANSPARENT );

		CFont *pOldFont = pDC->SelectObject( &m_clFont );

		// Always left aligned within drawing rect.
		UINT uiFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX;

		CRect rectText( rect );
		// Get size of text so we can update rect.
		CSize size = pDC->GetTextExtent( strText );
		rectText.right = rectText.left + size.cx;

		rectText.left -= 4;
		pDC->DrawText( strText, rectText, uiFormat );

		rc = rectText.right;

		if( pOldFont != NULL )
		{
			pDC->SelectObject( pOldFont );
		}
	}

	return rc;
}

//=============================================================================
//
// EnableChildWindows()
//
// Purpose:     This function enables/disables all the controls that are
//              completely contained within a parent.
//
// Parameters:  hWnd          - HWND of parent control
//              bEnable       - TRUE = enable controls within parent
//              bEnableParent - TRUE = also enable/disable parent window
//
// Returns:     int     - number of controls enabled/disabled.  If zero is
//                        returned, it means that no controls lie within the
//                        rect of the parent.
//
int CXGroupBox::EnableChildWindows( HWND hWnd, bool bEnable, bool bEnableParent )
{
	if( true == bEnableParent )
	{
		::EnableWindow( hWnd, bEnable );
	}

	int rc = 0;

	if( 0 == m_vechWndChild.size() )
	{
		return rc;
	}

	// Enable child windows.
	for( std::vector<HWND>::iterator It = m_vechWndChild.begin(); It != m_vechWndChild.end(); ++It )
	{
		::EnableWindow( *It, bEnable );
		rc++;
	}
	
	HWND hWndParent = ::GetParent( hWnd );
	::InvalidateRect( hWndParent, NULL, FALSE );

	return rc;
}

int CXGroupBox::ShowChildWindows( HWND hWnd, bool bShow, bool bShowParent )
{
	if( true == bShowParent )
	{
		::ShowWindow( hWnd, bShow );
	}

	int rc = 0;

	if( 0 == m_vechWndChild.size() )
	{
		return rc;
	}

	// Move child windows.
	for( std::vector<HWND>::iterator It = m_vechWndChild.begin(); It != m_vechWndChild.end(); ++It )
	{
		::ShowWindow( *It, bShow );
		rc++;
	}
	
	HWND hWndParent = ::GetParent( hWnd );
	::InvalidateRect( hWndParent, NULL, FALSE );

	return rc;
}

void CXGroupBox::EraseBorder( CDC *pDC, CRect &rect, COLORREF crBackground )
{
	if( false == rect.IsRectEmpty() )
	{
		CRect r( rect );
		CPen pen( PS_SOLID, 5, crBackground );
		CPen *pOldPen = pDC->SelectObject( &pen );
		r.DeflateRect( 2, 2 );
		pDC->Rectangle( &r );

		if( pOldPen != NULL )
		{
			pDC->SelectObject( pOldPen );
		}
	}
}

void CXGroupBox::EraseHeaderArea( CDC *pDC, CRect &rect, COLORREF crBackground )
{
	GetHeaderRect( rect );
	pDC->FillSolidRect( &rect, crBackground );
}

int CXGroupBox::GetFontHeight( int iPointSize )
{
	HDC hdc = ::CreateDC( _T("DISPLAY"), NULL, NULL, NULL );
	ASSERT( NULL != hdc );

	int cyPixelsPerInch = ::GetDeviceCaps( hdc, LOGPIXELSY );
	::DeleteDC( hdc );

	int iHeight = -MulDiv( iPointSize, cyPixelsPerInch, 72 );

	return iHeight;
}

int CXGroupBox::GetFontPointSize( int iHeight )
{
	HDC hdc = ::CreateDC( _T("DISPLAY"), NULL, NULL, NULL );
	ASSERT( NULL != hdc );

	int iYPixelsPerInch = ::GetDeviceCaps( hdc, LOGPIXELSY );
	::DeleteDC( hdc );

	int iPointSize = MulDiv( iHeight, 72, iYPixelsPerInch );

	if( iPointSize < 0 )
	{
		iPointSize = -iPointSize;
	}

	return iPointSize;
}

void CXGroupBox::GetHeaderRect( CRect &rect )
{
	CRect rectItem( rect );

	rect.top += m_iYMargin;
	rect.bottom += m_iYMargin;

	int nTotalWidth = m_sizeText.cx;
	nTotalWidth += m_iIconSize;				// this will be 0 if no icon

	if( m_CheckBoxRect.Width() > 0 && CONTROL_STYLE::groupbox == m_eStyle )
	{
		nTotalWidth += m_CheckBoxRect.Width() + 6;
	}

	if( ( 0 != m_sizeText.cx ) && NULL != m_hIcon )
	{
		nTotalWidth += m_iIconSpacing;
	}

	if( groupbox == m_eStyle )
	{
		TRACE( _T("groupbox\n") );
		nTotalWidth += 6;			// 3 pixels before and after
		
		if( left == m_eControlAlignment )
		{
			rect.left += m_iXMargin;
		}
		else if( center == m_eControlAlignment )
		{
			rect.left = rect.left + ( rectItem.Width() - nTotalWidth ) / 2;
		}
		else	// right
		{
			rect.left = rectItem.right - nTotalWidth - m_iXMargin;
		}
		
		rect.right = rect.left + nTotalWidth;
	}
	else	// header style
	{
		TRACE( _T("header\n") );
		nTotalWidth += 3;			// 3 pixels after
		
		if( left == m_eControlAlignment )
		{
			
		}
		else if( center == m_eControlAlignment )
		{
			nTotalWidth += 3;			// 3 pixels before
			rect.left = rect.left + ( rectItem.Width() - nTotalWidth ) / 2;
		}
		else	// right
		{
			rect.left = rectItem.right - nTotalWidth;// - m_nXMargin;
		}
		
		rect.right = rect.left + nTotalWidth;
	}
}

CFont *CXGroupBox::GetSafeFont()
{
	// Get current font.
	CFont *pFont = CWnd::GetFont();

	if( NULL == pFont )
	{
		// Try to get parent font.
		CWnd *pParent = GetParent();

		if( pParent != NULL && TRUE == IsWindow( pParent->m_hWnd ) )
		{
			pFont = pParent->GetFont();
		}

		if( NULL == pFont )
		{
			// No font, so get a system font.
			HFONT hFont = (HFONT)::GetStockObject( DEFAULT_GUI_FONT );

			if( NULL == hFont )
			{
				hFont = (HFONT)::GetStockObject( SYSTEM_FONT );
			}

			if( NULL == hFont )
			{
				hFont = (HFONT)::GetStockObject( ANSI_VAR_FONT );
			}

			if( hFont != NULL )
			{
				pFont = CFont::FromHandle( hFont );
			}
		}
	}

	return pFont;
}

void CXGroupBox::ComputeHeaderHeight()
{
	CRect rectHeader;
	GetClientRect( &rectHeader );
	
	CDC *pDC = GetDC();
	CSize clSizeText = 0;
	
	// Get string width.
	CString strText = _T("");
	GetWindowText( strText );

	if( false == strText.IsEmpty() )
	{
		CFont *pOldFont = pDC->SelectObject( &m_clFont );
		clSizeText = pDC->GetTextExtent( strText );

		if( pOldFont != NULL )
		{
			pDC->SelectObject( pOldFont );
		}
	}

	rectHeader.bottom = rectHeader.top + __max( clSizeText.cy, m_iIconSize ) + 1;
	m_iHeaderHeight = rectHeader.Height();
	ReleaseDC( pDC );
}
