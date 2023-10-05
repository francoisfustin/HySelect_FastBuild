#include "stdafx.h"
#include "MainFrm.h"
#include "Taselect.h"
#include "ExtStatic.h"
#include "ExtNumEdit.h"
#include "ExtComboBox.h"
#include "DialogExt.h"


IMPLEMENT_DYNCREATE( CDialogExt, CDialogEx )

CDialogExt::CDialogExt( UINT nID, CWnd *pParent, bool bUseThemeColor )
	: CDialogEx( nID, pParent )
{
	// Recuperate the default style from the main application.
	if( bUseThemeColor )
	{
		this->SetBackgroundColor( TASApp.GetBckColor() );
	}
	else
	{
		// Background color of MS Word Dialogs
		this->SetBackgroundColor( 0xf0f0f0 );
	}

	m_strSectionName = _T( "" );
	m_bInitialized = false;
}

BEGIN_MESSAGE_MAP( CDialogExt, CDialogEx )
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_MOVE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CDialogExt::OnInitDialog()
{
	if( FALSE == CDialogEx::OnInitDialog() )
	{
		return FALSE;
	}

	if( true == m_strSectionName.IsEmpty() )
	{
		return TRUE;
	}

	// Set the window position to the last stored position in registry. If window position is not yet stored in the registry, the
	// window is centered by default in the mainframe rect.
	CRect apprect, rect;
	::AfxGetApp()->m_pMainWnd->GetWindowRect( &apprect );
	GetWindowRect( &rect );
	int x = ::AfxGetApp()->GetProfileInt( m_strSectionName, _T("ScreenXPos"), apprect.CenterPoint().x - rect.Width() / 2 );
	int y = ::AfxGetApp()->GetProfileInt( m_strSectionName, _T("ScreenYPos"), apprect.CenterPoint().y - rect.Height() / 2 );
	SetWindowPos( NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

	// Set the window size to the last stored size in registry.
	int iWidth = ::AfxGetApp()->GetProfileInt( m_strSectionName, _T("DialogWidth"), rect.Width() );
	int iHeight = ::AfxGetApp()->GetProfileInt( m_strSectionName, _T("DialogHeight"), rect.Height() );
	SetWindowPos( NULL, 0, 0, iWidth, iHeight, SWP_NOZORDER | SWP_NOMOVE );

	// If the window is placed on a screen that is currently deactivated, the windows is centered to the application.
	HMONITOR hMonitor = NULL;
	hMonitor = MonitorFromRect( &rect, MONITOR_DEFAULTTONULL );

	if( NULL == hMonitor )
	{
		CenterWindow();
	}

	m_bInitialized = true;
	return TRUE;
}

// See the 'TAS Project\TASelect-Sln\Doc\Code\CStatic.md' for a well understanding of this code.
HBRUSH CDialogExt::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
{
	HBRUSH hbr = CDialog::OnCtlColor( pDC, pWnd, nCtlColor );

	if( NULL != m_brBkgr.GetSafeHandle() || NULL != m_hBkgrBitmap )
	{
		TCHAR lpszClassName[AFX_MAX_CLASS_NAME + 1];
		::GetClassName( pWnd->GetSafeHwnd(), lpszClassName, AFX_MAX_CLASS_NAME );
		CString strClass = lpszClassName;

		if( AFX_STATIC_CLASS == strClass || AFX_EDIT_CLASS == strClass || AFX_BUTTON_CLASS == strClass || AFX_SLIDER_CLASS == strClass
				|| AFX_COMBOBOX_CLASS == strClass )
		{
			pDC->SetBkMode( TRANSPARENT );
		
			if( hbr == ::GetStockObject( NULL_BRUSH ) 
					|| ( ( AFX_STATIC_CLASS != strClass || NULL == dynamic_cast<CExtStatic *>( pWnd ) )
							&& ( AFX_EDIT_CLASS != strClass || NULL == dynamic_cast<CExtNumEdit *>( pWnd ) ) 
							&& ( AFX_COMBOBOX_CLASS != strClass || NULL == dynamic_cast<CExtNumEditComboBox *>( pWnd ) ) ) )
			{
				hbr = (HBRUSH)m_brBkgr;	
			}
		}
	}

	return hbr;
}

BOOL CDialogExt::OnEraseBkgnd( CDC *pDC )
{
	CBrush *pOldBrush = pDC->SelectObject( &m_brBkgr );

	CRect rect;
	pDC->GetClipBox( &rect );
	pDC->PatBlt( rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY );
	pDC->SelectObject( pOldBrush );

	return TRUE;
}

void CDialogExt::OnMove( int x, int y )
{
	CDialogEx::OnMove( x, y );

	if( true == m_bInitialized && false == m_strSectionName.IsEmpty() )
	{
		CRect rect;

		// Form DialogCircuit into the screen.
		GetWindowRect( &rect );
		ScreenToClient( &rect );

		// Take into account position of DialogCircuit Form into dialog wizard dialog window.
		y += rect.top;
		x += rect.left;
		::AfxGetApp()->WriteProfileInt( m_strSectionName, _T("ScreenXPos"), x );
		::AfxGetApp()->WriteProfileInt( m_strSectionName, _T("ScreenYPos"), y );
	}
}

void CDialogExt::OnSize( UINT nType, int cx, int cy )
{
	CDialogEx::OnSize( nType, cx, cy );

	if( true == m_bInitialized && false == m_strSectionName.IsEmpty() )
	{
		CRect rect;
		GetWindowRect( &rect );
		::AfxGetApp()->WriteProfileInt( m_strSectionName, _T("DialogWidth"), rect.Width() );
		::AfxGetApp()->WriteProfileInt( m_strSectionName, _T("DialogHeight"), rect.Height() );
	}
}

void CDialogExt::SetLocalizedText( int nControlID, UINT uiStringID )
{
	CWnd *control = GetDlgItem( nControlID );

	if( NULL == control )
	{
		ASSERT( control );
		return;
	}

	control->SetWindowText( TASApp.LoadLocalizedString( uiStringID ) );
}

void CDialogExt::SetLocalizedText( CButton &button, UINT uiStringID )
{
	button.SetWindowText( TASApp.LoadLocalizedString( uiStringID ) );
}

void CDialogExt::SetLocalizedTitle( UINT uiStringID )
{
	SetWindowText( TASApp.LoadLocalizedString( uiStringID ) );
}
