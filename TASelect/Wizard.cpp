#include "stdafx.h"
#include "resource.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "wizard.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgWizard dialog

CDlgWizard::CDlgWizard( UINT DiagID, CWnd *pParent )
	: CDialogEx( DiagID, pParent )
{
	m_bInitialized = false;
	m_ResID = DiagID;
	m_pParent = NULL;
}

BOOL CDlgWizard::Create( CWizardManager *pParent )
{ 
	ASSERT( NULL != pParent ); 
	m_pParent = pParent;
	return CDialogEx::Create( m_ResID, (CWnd*)pParent );
}

BEGIN_MESSAGE_MAP( CDlgWizard, CDialogEx )
END_MESSAGE_MAP()

void CDlgWizard::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );

	if( NULL != GetDlgItem( IDC_EDITTITLE ) )
	{
		DDX_Control( pDX, IDC_EDITTITLE, m_EditTitle );
	}
}

BOOL CDlgWizard::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if( NULL != GetDlgItem( IDC_EDITTITLE ) )
	{
		m_EditTitle.SetBlockCursorChange( true );
		m_EditTitle.SetBlockSelection( true );
	}

	return TRUE;
}

BOOL CDlgWizard::PreTranslateMessage( MSG *pMsg )
{
	if( WM_KEYDOWN == pMsg->message && VK_ESCAPE == pMsg->wParam )
	{
		// First call the 'OnEscapeKeyPressed' virtual method of the current 'CDlgWizard' inherited class.
		// If returns 'true', we can quit.
		if( true == OnEscapeKeyPressed() )
		{
			m_pParent->EscapeKeyPressed();
		}
	}
	
	return FALSE;
}

CWizardManager *CDlgWizard::GetWizMan()
{
	return (CWizardManager *)GetParent();
}

void CDlgWizard::OnCancel()
{
	// Call virtual function OnWizCancel if the button was clicked, do nothing if the escape key was pressed.
	CWnd *pCtrl = GetDlgItem( IDCANCEL );

	if( NULL != pCtrl && ( pCtrl == GetFocus() ) )
	{
		if( TRUE == OnWizCancel() )
		{
			GetWizMan()->Cancel();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWizardManager dialog
IMPLEMENT_DYNAMIC( CWizardManager, CDialogEx )

CWizardManager::CWizardManager( UINT uiTitleRes, UINT IDD )
	: CDialogEx( IDD, NULL )
{
	m_iPanelCount = 0;
	m_iPanelActive = -1;
	m_uiTitleRes = uiTitleRes;
	m_uiIDD = IDD;
	m_iAvailableHeightBelowPanel = 0;
	m_iButtonReferencePoint = 0;
}

CWizardManager::~CWizardManager()
{
	if( m_vecPanelList.size() > 0 )
	{
		for( int iLoop = 0; iLoop < (int)m_vecPanelList.size(); iLoop++ )
		{
			delete m_vecPanelList[iLoop];
		}

		m_vecPanelList.clear();
	}
}

void CWizardManager::Add( CDlgWizard *pDlg, int iID )
{
	m_vecPanelList.push_back( new PANEL );
	m_vecPanelList[m_iPanelCount]->m_iID = iID;
	m_vecPanelList[m_iPanelCount]->m_bCreated = false;
	m_vecPanelList[m_iPanelCount]->m_bInitialized = false;
	m_vecPanelList[m_iPanelCount]->m_pPanel = pDlg;
	m_vecPanelList[m_iPanelCount++]->m_ptInitialSize = CPoint( -1, -1 );
}

void CWizardManager::ChangeWizard( PANEL *pNewWiz, PANEL *pOldWiz, bool bCallAfterActivate )
{
	if( pOldWiz == pNewWiz )
	{
		return;
	}

	// Signal that we leave the old panel.
	if( NULL != pOldWiz )
	{
		ASSERT( pOldWiz->m_bCreated );
		pOldWiz->m_pPanel->OnQuitPane();
	}
	
	bool bFlag = false;
	
	// Display the new panel.
	if( false == pNewWiz->m_bCreated )
	{
		// Retrieve current position of the dialog client area.
		CRect rectDialog;
		GetClientRect( &rectDialog );

		// Retrieve current position of the separator.
		CRect rectSeparator;
		m_StaticSeparator.GetWindowRect( &rectSeparator );
		ScreenToClient( &rectSeparator );

		pNewWiz->m_pPanel->Create( this );
		pNewWiz->m_pPanel->SetWindowPos( NULL, rectDialog.left, rectDialog.top, rectDialog.Width(), rectSeparator.top - rectDialog.top, SWP_NOZORDER );
		pNewWiz->m_bCreated = true;
		pNewWiz->m_bInitialized = false;
		pNewWiz->m_ptInitialSize = CPoint( rectDialog.Width(), rectSeparator.top - rectDialog.top );
		bFlag = pNewWiz->m_pPanel->OnInitialActivate();
	}
	else
	{
		bFlag = pNewWiz->m_pPanel->OnActivate();
	}

	// Display the new panel.
	pNewWiz->m_pPanel->ShowWindow( SW_SHOW );
	
	// Hide old pane.
	if( NULL != pOldWiz )
	{
		ASSERT( pOldWiz->m_bCreated );
		pOldWiz->m_pPanel->ShowWindow( SW_HIDE );
	}
	
	EnableButtons( CWizardManager::WizButFinish | CWizardManager::WizButNext | CWizardManager::WizButBack );

	// Display or hide wizard buttons.
	int iButtons = pNewWiz->m_pPanel->OnWizButtons();
	
	// Default functionalities.
	if( WizButDefault == ( iButtons & WizButDefault ) )
	{	
		// Display or hide back and next button.
		m_ButWizNext.ShowWindow( ( m_iPanelActive == ( m_iPanelCount - 1 ) ) ? SW_HIDE : SW_SHOW );
		m_ButWizBack.ShowWindow( ( 0 == m_iPanelActive ) ? SW_HIDE : SW_SHOW );
	}
	else //Custom functionalities
	{
		m_ButWizFinish.ShowWindow( ( WizButFinish == ( iButtons & WizButFinish ) ) ? SW_SHOW : SW_HIDE );
		m_ButWizNext.ShowWindow( ( WizButNext == ( iButtons & WizButNext ) ) ? SW_SHOW : SW_HIDE );
		m_ButWizBack.ShowWindow( ( WizButBack == ( iButtons & WizButBack ) ) ? SW_SHOW : SW_HIDE );
	}
	
	// Initialize wizard buttons.
	CString str; 
	str = TASApp.LoadLocalizedString( pNewWiz->m_pPanel->OnWizButFinishTextID() );
	m_ButWizFinish.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( pNewWiz->m_pPanel->OnWizButBackTextID() );
	m_ButWizBack.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( pNewWiz->m_pPanel->OnWizButNextTextID() );
	m_ButWizNext.SetWindowText( str );

	if( false == bFlag )
	{
		DoNext();
	}
	
	if( true == bCallAfterActivate )
	{
		pNewWiz->m_pPanel->OnAfterActivate();
	}
}

void CWizardManager::DisplayWizard()
{
	DoModal();
}

void CWizardManager::EscapeKeyPressed()
{
	// Call the 'OnWizFinish' of this class.
	if( FALSE == OnWizFinish() )
	{
		return;
	}
	
	Finish();
}

void CWizardManager::DoNext()
{
	PANEL *pOldWiz = m_vecPanelList[m_iPanelActive];

	if( ++m_iPanelActive >= m_iPanelCount )
	{
		m_iPanelActive = m_iPanelCount - 1;
	}

	ChangeWizard( m_vecPanelList[m_iPanelActive], pOldWiz );
}

void CWizardManager::DoBack()
{
	PANEL *pOldWiz = m_vecPanelList[m_iPanelActive];

	if( --m_iPanelActive < 0 )
	{
		m_iPanelActive = 0;
	}

	ChangeWizard( m_vecPanelList[m_iPanelActive], pOldWiz );
}
 
void CWizardManager::Cancel()
{
	// First call the 'OnWizCancel' virtual method of the current 'CDlgWizard' inherited class.
	if( FALSE == m_vecPanelList[m_iPanelActive]->m_pPanel->OnWizCancel() )
	{
		return;
	}
	
	// Call now the 'OnCancel' of this class.
	OnCancel();

	// Signal that we leave to the panel.
	m_vecPanelList[m_iPanelActive]->m_pPanel->OnQuitPane();
	
	// Destroy this one.
	EndDialog( IDCANCEL );
}
 
void CWizardManager::Finish()
{
	OnFinish();
	
	// Signal that we leave to the panel.
	m_vecPanelList[m_iPanelActive]->m_pPanel->OnQuitPane();

	// Destroy this one.
	EndDialog( IDCANCEL );
}

bool CWizardManager::JumpToPanel( const int iID )
{
	if( 0 == m_iPanelCount )
	{
		return false;
	}

	PANEL *pOldWiz = m_vecPanelList[m_iPanelActive];

	if( iID >= m_iPanelCount )
	{
		return false;
	}
	
	// Try to find target panel.
	for( int i = 0; i < m_iPanelCount; i++ )
	{
		if( iID == m_vecPanelList[i]->m_iID )
		{
			m_iPanelActive = i;
			ChangeWizard( m_vecPanelList[m_iPanelActive], pOldWiz );
			return true;
		}
	}
	
	return false;
}

void CWizardManager::EnableButtons( int iButton )
{
	if( WizButFinish == ( iButton & WizButFinish ) )
	{
		m_ButWizFinish.EnableWindow();
	}

	if( WizButNext == ( iButton & WizButNext ) )
	{
		m_ButWizNext.EnableWindow();
	}

	if( WizButBack == ( iButton & WizButBack) )
	{
		m_ButWizBack.EnableWindow();
	}
}

void CWizardManager::DisableButtons( int iButton )
{
	if( WizButFinish == ( iButton & WizButFinish ) )
	{
		m_ButWizFinish.EnableWindow( FALSE );
	}

	if( WizButNext == ( iButton & WizButNext ) )
	{
		m_ButWizNext.EnableWindow( FALSE );
	}

	if( WizButBack == ( iButton & WizButBack ) )
	{
		m_ButWizBack.EnableWindow( FALSE );
	}
}

void CWizardManager:: ShowButtons( int iButton, int nCmdShow )
{
	if( WizButFinish == ( iButton & WizButFinish ) )
	{
		m_ButWizFinish.ShowWindow( nCmdShow );
	}

	if( WizButNext == ( iButton & WizButNext ) )
	{
		m_ButWizNext.ShowWindow( nCmdShow );
	}

	if( WizButBack == ( iButton & WizButBack ) )
	{
		m_ButWizBack.ShowWindow( nCmdShow );
	}
}

void CWizardManager::SetButtonText( enum_WizBut eButton, CString *pstrText )
{
	if( WizButFinish == ( eButton & WizButFinish ) )
	{
		m_ButWizFinish.SetWindowText( *pstrText );
	}

	if( WizButNext == ( eButton & WizButNext ) )
	{
		m_ButWizNext.SetWindowText( *pstrText );
	}

	if( WizButBack == ( eButton & WizButBack ) )
	{
		m_ButWizBack.SetWindowText(*pstrText );
	}
}

void CWizardManager::SetButtonText( enum_WizBut eButton, int ids )
{
	CString str =TASApp.LoadLocalizedString( ids );
	SetButtonText( eButton, &str );
}

CWizardManager::PANEL *CWizardManager::GetCurrentPanel()
{
	return ( -1 == m_iPanelActive ) ? NULL : m_vecPanelList[m_iPanelActive];
}

BEGIN_MESSAGE_MAP( CWizardManager, CDialogEx )
	ON_BN_CLICKED( IDC_WIZBACK, OnWizB )
	ON_BN_CLICKED( IDC_WIZFINISH, OnWizF )
	ON_BN_CLICKED( IDC_WIZNEXT, OnWizN )
	ON_WM_GETMINMAXINFO()
	ON_WM_CLOSE()
	ON_MESSAGE( WM_USER_WIZARDMANAGER_DOBACK, OnDoBack )
END_MESSAGE_MAP()

void CWizardManager::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICSEPARATOR, m_StaticSeparator );
	DDX_Control( pDX, IDC_WIZNEXT, m_ButWizNext );
	DDX_Control( pDX, IDC_WIZFINISH, m_ButWizFinish );
	DDX_Control( pDX, IDC_WIZBACK, m_ButWizBack );
}

BOOL CWizardManager::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Prepare 2 member variables.
	////////////////////////////////////////////////////////////////////////////////////////////////

	// Retrieve the dialog client area.
	CRect rectDialog;
	GetClientRect( &rectDialog );

	// Retrieve the separator client area.
	CRect rectSeparator;
	m_StaticSeparator.GetWindowRect( &rectSeparator );
	ScreenToClient( rectSeparator );

	// Keep always same height between the bottom of the panel and the bottom of the dialog.
	// m_iAvailableHeightBelowPanel = rectDialog.bottom - rectSeparator.top;
	
	// TODO: At now we have a problem with low resolution. In some cases, 'GetClientRect()' doesn't return the size defined in the resource.
	// For the moment we fix this value to 44.
	m_iAvailableHeightBelowPanel = 0;
	int iDialogID = GetDlgCtrlID();
	DWORD dwError = GetLastError();
	HINSTANCE hInst = AfxFindResourceHandle( MAKEINTRESOURCE( GetDlgCtrlID() ), RT_DIALOG );
	HRSRC hResource = ::FindResource( hInst, MAKEINTRESOURCE( GetDlgCtrlID() ), RT_DIALOG );
	HGLOBAL hDialogTemplate = LoadResource( hInst, hResource );
	LPCDLGTEMPLATE lpDialogTemplate = NULL;

	if( hDialogTemplate != NULL )
	{
		lpDialogTemplate = (LPCDLGTEMPLATE)LockResource( hDialogTemplate );
	}

	if( lpDialogTemplate != NULL  )
	{
		CRect rectDialogSize( 0, 0, lpDialogTemplate->cx, lpDialogTemplate->cy );
		MapDialogRect( &rectDialogSize );
		
		WORD *ptrPointer = (WORD *)( lpDialogTemplate + sizeof( LPCDLGTEMPLATE ) );
		
		// Align to WORD.
		ptrPointer = (WORD *)( (DWORD)( ptrPointer + 3 ) & (DWORD)(-4) );
		
		// Read menu, dialog class and caption.
		for( int iLoopItem = 0; iLoopItem < 3; iLoopItem++ )
		{
			if( 0 == *ptrPointer )
			{
				ptrPointer++;
			}
			else if( 0xFFFF == *ptrPointer )
			{
				ptrPointer += 2;
			}
			else
			{
				TCHAR *ptChar = (TCHAR *)ptrPointer;
				CString str( *ptChar );
				ptrPointer += ( str.GetLength() + 1 );
			}
			
			ptrPointer = (WORD *)( (DWORD)( ptrPointer + 3 ) & (DWORD)(-4) );
		}

		// Run all dialog items.
		for( int iLoopItem = 0; iLoopItem < lpDialogTemplate->cdit; iLoopItem++ )
		{
			LPDLGITEMTEMPLATE lpDialogItemTemplate = (LPDLGITEMTEMPLATE)ptrPointer;
		
			if( IDC_STATICSEPARATOR == lpDialogItemTemplate->id )
			{
				CRect rectSeparator( lpDialogItemTemplate->x, lpDialogItemTemplate->y, 
									 lpDialogItemTemplate->x + lpDialogItemTemplate->cx, 
									 lpDialogItemTemplate->y + lpDialogItemTemplate->cy );
				MapDialogRect( &rectSeparator );
				m_iAvailableHeightBelowPanel = rectDialogSize.bottom - rectSeparator.top;
				break;
			}
			else
			{
				ptrPointer = (WORD *)( lpDialogItemTemplate + sizeof( LPDLGITEMTEMPLATE ) );
				ptrPointer = (WORD *)( (DWORD)( ptrPointer + 3 ) & (DWORD)(-4) );
				
				// Read class name and text.
				for( int iLoopName = 0; iLoopName < 2; iLoopName++ )
				{
					if( 0 == *ptrPointer )
					{
						ptrPointer++;
					}
					else if( 0xFFFF == *ptrPointer )
					{
						ptrPointer += 2;
					}
					else
					{
						TCHAR *ptChar = (TCHAR *)ptrPointer;
						CString str( *ptChar );
						ptrPointer += ( str.GetLength() + 1 );
					}
					
					ptrPointer = (WORD *)( (DWORD)( ptrPointer + 3 ) & (DWORD)(-4) );
				}
				
				// Read creation data.
				if( 0 == *ptrPointer )
				{
					ptrPointer++;
				}
				else
				{
					ptrPointer += *ptrPointer;
				}

				ptrPointer = (WORD *)( (DWORD)( ptrPointer + 3 ) & (DWORD)(-4) );
			}
		}
	}

	if( 0 == m_iAvailableHeightBelowPanel )
	{
		m_iAvailableHeightBelowPanel = 44;
	}

	// Retrieve the separator client area.
	CRect rectButton;
	m_ButWizFinish.GetWindowRect( &rectButton );
	ScreenToClient( rectButton );

	// Keep always the same position in regards to bottom of the dialog.
	// m_iButtonReferencePoint = rectDialog.bottom - rectButton.top;

	// TODO: At now we have a problem with low resolution. In some cases, 'GetClientRect()' doesn't return the size defined in the resource.
	// For the moment we fix this value to 33.
	m_iButtonReferencePoint = 33;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Modify coordinates and size if screen resolution doesn't allow to completely show the dialog.
	////////////////////////////////////////////////////////////////////////////////////////////////

	HMONITOR hCurrentMonitor = MonitorFromWindow( pMainFrame->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST );

	if( hCurrentMonitor != NULL )
	{
		MONITORINFOEX MonitorInfo;
		MonitorInfo.cbSize = sizeof( MONITORINFOEX );
		::GetMonitorInfo( hCurrentMonitor, &MonitorInfo );
		HDC hdc = ::CreateDC( MonitorInfo.szDevice, MonitorInfo.szDevice, NULL, NULL );

		CRect rectWork( MonitorInfo.rcWork );

		if( rectDialog.Width() > rectWork.Width() || rectDialog.Height() > rectWork.Height() )
		{
			int iNewWidth = ( rectDialog.Width() <= rectWork.Width() ) ? rectDialog.Width() : rectWork.Width();
			int iNewHeight = ( rectDialog.Height() <= rectWork.Height() ) ? rectDialog.Height() : rectWork.Height();
			
			int iNewX = 0;
			if( rectDialog.Width() <= rectWork.Width() )
				iNewX = ( rectWork.Width() - rectDialog.Width() ) >> 1;
			
			int iNewY = 0;

			if( rectDialog.Height() <= rectWork.Height() )
			{
				iNewY = ( rectWork.Height() - rectDialog.Height() ) >> 1;
			}

			APPBARDATA Data;
			Data.cbSize = sizeof( Data );
			Data.hWnd = GetSafeHwnd();
			SHAppBarMessage( ABM_GETTASKBARPOS, &Data );

			if( 0 == iNewX && rectDialog.Width() > rectWork.Width() && ABE_LEFT == Data.uEdge )
			{
				iNewX = Data.rc.right;
			}

			if( rectDialog.Height() > rectWork.Height() && ABE_TOP == Data.uEdge )
			{
				iNewY = Data.rc.bottom;
			}

			SetWindowPos( NULL, iNewX, iNewY, iNewWidth, iNewHeight, SWP_NOZORDER );
		}

		::DeleteDC( hdc );
	}

	_UpdateLayoutBelowPanel();
	
	// Select the first panel.
	m_iPanelActive = 0;

	if( m_vecPanelList.size() > 0 )
	{
		ChangeWizard( m_vecPanelList[m_iPanelActive], NULL, false );
	}

	_UpdateLayoutPanel();

	m_vecPanelList[m_iPanelActive]->m_pPanel->OnAfterActivate();
 
	// Set title.
	CString text;

	if( text = TASApp.LoadLocalizedString( m_uiTitleRes ) )
	{
		SetWindowText( text );
	}
	
	return TRUE;
}

void CWizardManager::OnWizB() 
{
	// First call the 'OnWizBack' virtual method of the current 'CDlgWizard' inherited class.
	if( TRUE == m_vecPanelList[m_iPanelActive]->m_pPanel->OnWizBack() )
	{
		DoBack();
	}
}

void CWizardManager::OnWizF()
{
	// First call the 'OnWizFinish' virtual method of the current 'CDlgWizard' inherited class.
	if( FALSE == m_vecPanelList[m_iPanelActive]->m_pPanel->OnWizFinish() )
	{
		return;
	}
	
	// Call now the 'OnWizFinish' of this class.
	if( FALSE == OnWizFinish() )
	{
		return;
	}
	
	Finish();
}

void CWizardManager::OnWizN() 
{
	// First call the virtual function to process the derived class code.
	if( TRUE == m_vecPanelList[m_iPanelActive]->m_pPanel->OnWizNext() )
	{
		DoNext();
	}
}

void CWizardManager::OnSize( UINT nType, int cx, int cy )
{ 
	_UpdateLayoutBelowPanel();
	_UpdateLayoutPanel();
}

void CWizardManager::OnGetMinMaxInfo( MINMAXINFO* lpMMI )
{
	if( m_iPanelCount > 0 && m_iPanelActive != -1 && m_vecPanelList[m_iPanelActive]->m_pPanel != NULL
			&& m_vecPanelList[m_iPanelActive]->m_pPanel->GetSafeHwnd() != NULL )
	{
		m_vecPanelList[m_iPanelActive]->m_pPanel->OnGetMinMaxInfo( lpMMI );
	}
}

void CWizardManager::OnClose()
{
	// Called when user clicks on the 'Close' button in the dialog title bar.
	Cancel();
}

LRESULT CWizardManager::OnDoBack( WPARAM wParam, LPARAM lParam )
{
	DoBack();
	return 0;
}

void CWizardManager::_UpdateLayoutPanel( void )
{
	// Is ready?
	if( NULL == m_StaticSeparator.GetSafeHwnd() )
	{
		return;
	}

	// Is panel ready?
	if( 0 == (int)m_vecPanelList.size() || -1 == m_iPanelActive || NULL == m_vecPanelList[m_iPanelActive]->m_pPanel->GetSafeHwnd() )
	{
		return;
	}

	for( auto &iter : m_vecPanelList )
	{
		// Not yet ready ?
		if( NULL == iter->m_pPanel->GetSafeHwnd() )
		{
			continue;
		}

		if( false == iter->m_bInitialized )
		{
			// If original size of panel is not yet saved...
			if( iter->m_ptInitialSize == CPoint( -1, -1 ) )
			{
				CRect rectDialog;
				iter->m_pPanel->GetClientRect( &rectDialog );
			}

			// Memorize current window size.
			CRect rectWindow;
			GetWindowRect( &rectWindow );

			// Check needed area.
			int iTotalWidth = iter->m_ptInitialSize.x;
			int iTotalHeight = iter->m_ptInitialSize.y + m_iAvailableHeightBelowPanel;

			CRect rectDesiredClient( 0, 0, iTotalWidth, iTotalHeight );
			AdjustWindowRectEx( &rectDesiredClient, (DWORD)GetWindowLong( GetSafeHwnd(), GWL_STYLE ), FALSE, (DWORD)GetWindowLong( GetSafeHwnd(), GWL_EXSTYLE ) );
			SetWindowPos( NULL, 0, 0, rectDesiredClient.Width(), rectDesiredClient.Height(), SWP_NOZORDER | SWP_NOMOVE );

			iter->m_bInitialized = true;
		}
		else
		{
			// Get current client area.
			CRect rectDialogClient;
			GetClientRect( &rectDialogClient );

			// Set panel width.
			int iPanelWidth = rectDialogClient.Width();

			// Set panel height.
			int iPanelHeight = rectDialogClient.Height() - m_iAvailableHeightBelowPanel;

			// Resize panel.
			iter->m_pPanel->SetWindowPos( NULL, -1, -1, iPanelWidth, iPanelHeight, SWP_NOZORDER | SWP_NOMOVE );

			// Call 'OnSize' in panel to let it change its internal controls.
			iter->m_pPanel->OnSize( CRect( 0, 0, iPanelWidth, iPanelHeight ) );
		}
	}
}

void CWizardManager::_UpdateLayoutBelowPanel( void )
{
	// Is ready?
	if( NULL == m_StaticSeparator.GetSafeHwnd() )
	{
		return;
	}

	// Get current client area.
	CRect rectDialogClient;
	GetClientRect( &rectDialogClient );

	// Get separator dimension.
	CRect rectSeparator;
	m_StaticSeparator.GetWindowRect( &rectSeparator );

	// Move separator just 1 pixel below the current panel.
	m_StaticSeparator.SetWindowPos( NULL, rectDialogClient.left, rectDialogClient.bottom - m_iAvailableHeightBelowPanel, rectDialogClient.Width(), rectSeparator.Height(), SWP_NOZORDER );

	// Get one button dimension.
	CRect rectButton;
	m_ButWizFinish.GetClientRect( &rectButton );

	int iMargin = 11;

	// We move the three buttons to align them with the right edge of the dialog and with the bottom of the separator.
	m_ButWizFinish.SetWindowPos( NULL, rectDialogClient.right - rectButton.Width() - iMargin - 2, 
			rectDialogClient.bottom - m_iButtonReferencePoint,
			rectButton.Width(), rectButton.Height(), SWP_NOZORDER );

	m_ButWizNext.SetWindowPos( NULL, rectDialogClient.right - 2 * rectButton.Width() - 2 * iMargin - 2, 
			rectDialogClient.bottom - m_iButtonReferencePoint,
			rectButton.Width(), rectButton.Height(), SWP_NOZORDER );

	m_ButWizBack.SetWindowPos( NULL, rectDialogClient.right - 3 * rectButton.Width() - 3 * iMargin - 2, 
			rectDialogClient.bottom - m_iButtonReferencePoint,
			rectButton.Width(), rectButton.Height(), SWP_NOZORDER );

	// Invalidate this area.
	Invalidate();
}
