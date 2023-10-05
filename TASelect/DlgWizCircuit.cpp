#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "DlgHMTree.h"
#include "DlgInfoSSelDpC.h"
#include "DlgSizeRad.h"
#include ".\dlgwizcircuit.h"


IMPLEMENT_DYNAMIC( CDlgWizCircuit, CWizardManager )

CDlgWizCircuit *pDlgWizCircuit = NULL;

CDlgWizCircuit::CDlgWizCircuit( CWnd* pParent )
	: CWizardManager( IDS_WIZCIRCUIT_TITLE, CDlgWizCircuit::IDD )
{
	pDlgWizCircuit = this;
	m_bModified = false;
	m_bEditMode = false;
	m_pInitialTab = NULL;
	m_ppTab = NULL;
	m_pCurrentHM = NULL;
	m_SectionName = _T("DlgWizCircuit");
	m_bInitialized = false;
}

CDlgWizCircuit::~CDlgWizCircuit()
{
	pDlgWizCircuit = NULL;
}

bool CDlgWizCircuit::Init( CTable* pTab, bool fEdit, CTable** ppTab )
{
	Add( &m_PanelCirc1 );
	Add( &m_PanelCirc2 );
	m_bEditMode = fEdit;
	m_ppTab = ppTab;
	if( true == fEdit )
	{
		m_pCurrentHM = (CDS_HydroMod *)pTab;
		m_pInitialTab = (CTable *)( m_pCurrentHM->GetIDPtr().PP );
	}
	else
	{
		m_pCurrentHM = NULL;
		m_pInitialTab = pTab;
	}

	return true;
}

void CDlgWizCircuit::SetFocusToSSheet()
{
	PANEL* pPanel = GetCurrentPanel();
	if( NULL == pPanel || NULL == pPanel->m_pPanel )
		return;

	if( NULL != m_PanelCirc1.GetSafeHwnd() && NULL != m_PanelCirc1.GetFPSpreadSheet() && pPanel->m_pPanel == &m_PanelCirc1 )
		m_PanelCirc1.GetFPSpreadSheet()->SetFocus();
	else if( NULL != m_PanelCirc2.GetSafeHwnd() && NULL != m_PanelCirc2.GetFPSpreadSheet() && pPanel->m_pPanel == &m_PanelCirc2 )
		m_PanelCirc2.GetFPSpreadSheet()->SetFocus();
}

bool CDlgWizCircuit::IsAtLeastOneCircuitAdded( void )
{
	return m_PanelCirc2.IsAtLeastOneCircuitAdded();
}

CTable* CDlgWizCircuit::GetTableWhereLastInsertOccured( void )
{
	return m_PanelCirc2.GetTableWhereLastInsertOccured();
}

BEGIN_MESSAGE_MAP( CDlgWizCircuit, CWizardManager )
	ON_BN_CLICKED( IDC_BUTTONPRINT, OnBnClickedButtonprint )
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_CREATE()
END_MESSAGE_MAP()

void CDlgWizCircuit::DoDataExchange( CDataExchange* pDX )
{
	CWizardManager::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_BUTTONPRINT, m_ButtonPrint  );
	DDX_Control( pDX, IDC_STATICNUMADD, m_StaticNumAdd );
	DDX_Control( pDX, IDC_EDITNUMADD, m_EditNumAdd );
	DDX_Control( pDX, IDC_SPIN, m_Spin );
}

void CDlgWizCircuit::ChangeWizard( PANEL* pNewWiz, PANEL* pOldWiz, bool fCallAfterActivate )
{
	CWizardManager::ChangeWizard( pNewWiz, pOldWiz, fCallAfterActivate );

	// Do some specific initialization specific to DialogWizCircuit and depending of displayed panel.
	if( pNewWiz->m_pPanel == &m_PanelCirc1 )
	{
		m_ButtonPrint.ShowWindow( SW_HIDE );
	}
	else if( pNewWiz->m_pPanel == &m_PanelCirc2 )
	{
		m_ButtonPrint.ShowWindow( SW_HIDE );
		m_EditNumAdd.ShowWindow( SW_SHOW );
		m_Spin.ShowWindow( SW_SHOW );
		m_StaticNumAdd.ShowWindow( SW_SHOW );
	}
	
	if( true == m_bEditMode || pNewWiz->m_pPanel == &m_PanelCirc1 )
	{
		m_EditNumAdd.ShowWindow( SW_HIDE );
		m_Spin.ShowWindow( SW_HIDE );
		m_StaticNumAdd.ShowWindow( SW_HIDE );
	}
}

BOOL CDlgWizCircuit::OnInitDialog()
{
	if( FALSE == CWizardManager::OnInitDialog() )
		return FALSE;
 
	_UpdateLayout();
	
	m_Spin.SetBuddy( GetDlgItem( IDC_EDITNUMADD ) );
 
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGWIZCIRCUIT_PRINT );
	GetDlgItem( IDC_BUTTONPRINT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGWIZCIRCUIT_STATNUMADD );
	GetDlgItem( IDC_STATICNUMADD )->SetWindowText( str );

	m_Spin.SetRange( 1, 99 );
	m_Spin.SetPos( 1 );

	// Set the window position to the last stored position in registry. If window position is not yet stored in the registry, the 
	// window is centered by default in the mainframe rect.
 	CRect apprect, rect;
 	::AfxGetApp()->m_pMainWnd->GetWindowRect( &apprect );
 	GetWindowRect( &rect );
// 	int x = ::AfxGetApp()->GetProfileInt( m_SectionName, _T("ScreenXPos"), apprect.CenterPoint().x - rect.Width() / 2 );
// 	int y = ::AfxGetApp()->GetProfileInt( m_SectionName, _T("ScreenYPos"), apprect.CenterPoint().y - rect.Height() / 2 );
// 	SetWindowPos( NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

	// Set the window size to the last stored size in registry.
	int iWidth = ::AfxGetApp()->GetProfileInt( m_SectionName, _T("DialogWidth"), rect.Width() );
	int iHeight = ::AfxGetApp()->GetProfileInt( m_SectionName, _T("DialogHeight"), rect.Height() );
	SetWindowPos( NULL, 0, 0, iWidth, iHeight, SWP_NOZORDER | SWP_NOMOVE );

	// If the window is placed on a screen that is currently deactivated, the windows is centered to the application.
// 	HMONITOR hMonitor = NULL;
// 	GetWindowRect( &rect );
// 	hMonitor = MonitorFromRect( &rect, MONITOR_DEFAULTTONULL );
// 	if( NULL == hMonitor)
// 		CenterWindow();
	
	m_bInitialized = true;
	return TRUE;
}

void CDlgWizCircuit::OnCancel()
{
	// Bypass 'CDlgWizard'.
	CDialogEx::OnCancel();
}

void CDlgWizCircuit::OnBnClickedButtonprint()
{
	m_PanelCirc2.Print();
}

void CDlgWizCircuit::OnMove( int x, int y )
{
	CWizardManager::OnMove( x, y );

	if( true == m_bInitialized )
	{
		CRect rect;
		
		// Form DialogCircuit into the screen.
		GetWindowRect( &rect );
		ScreenToClient( &rect );
		
		// Take into account position of DialogCircuit Form into dialog wizard dialog window.
		y += rect.top;
		x += rect.left;
		::AfxGetApp()->WriteProfileInt( m_SectionName, _T("ScreenXPos"), x );
		::AfxGetApp()->WriteProfileInt( m_SectionName, _T("ScreenYPos"), y );
	}
}

void CDlgWizCircuit::OnSize( UINT nType, int cx, int cy )
{
	CWizardManager::OnSize( nType, cx, cy );
	_UpdateLayout();

	if( true == m_bInitialized )
	{
		CRect rect;
		GetWindowRect( &rect );
		::AfxGetApp()->WriteProfileInt( m_SectionName, _T("DialogWidth"), rect.Width() );
		::AfxGetApp()->WriteProfileInt( m_SectionName, _T("DialogHeight"), rect.Height() );
	}
}

void CDlgWizCircuit::_UpdateLayout( void )
{
	// Is ready?
	if( NULL == m_ButWizFinish.GetSafeHwnd() )
		return;
	
	// Set print button position.
	int iMargin = 11;
	CRect rectBackButton, rectTreeButton;
	
	// Retrieve 'Back' button position relative to the dialog.
	m_ButWizBack.GetWindowRect( rectBackButton );
	ScreenToClient( rectBackButton );

	// Move 'Print' button to a new left coordinate, move at the same top as 'Finish' button and take same width and height.
	iMargin += ( rectTreeButton.Width() + 5 );
	m_ButtonPrint.SetWindowPos( NULL, iMargin, rectBackButton.top, rectBackButton.Width(), rectBackButton.Height(), SWP_NOZORDER );
	
	// Retrieve position of 'Static', 'Edit' and 'Spin' buttons.
	CRect rectStaticText, rectEditField, rectSpinButton;
	m_StaticNumAdd.GetWindowRect( rectStaticText );
	m_EditNumAdd.GetWindowRect( rectEditField );
	m_Spin.GetWindowRect( rectSpinButton );
	ScreenToClient( rectStaticText );
	ScreenToClient( rectEditField );
	ScreenToClient( rectSpinButton );
	
	// Centralize spin button in vertical with the 'Back' button.
	// Remark: '11' is the same hard coded margin as in the 'CWizardManager::_UpdateLayout' method.
	iMargin = rectBackButton.left - 11 - rectSpinButton.Width();
	int iStaticTop = rectBackButton.top + ( rectBackButton.Height() - rectSpinButton.Height() ) / 2;
	m_Spin.SetWindowPos( NULL, iMargin, iStaticTop, rectSpinButton.Width(), rectSpinButton.Height(), SWP_NOOWNERZORDER );

	// Centralize edit text in vertical with the 'Back' button.
	iMargin -= rectEditField.Width();
	iStaticTop = rectBackButton.top + ( rectBackButton.Height() - rectEditField.Height() ) / 2;
	m_EditNumAdd.SetWindowPos( NULL, iMargin, iStaticTop, rectEditField.Width(), rectEditField.Height(), SWP_NOOWNERZORDER );

	// Centralize static text in vertical with the 'Back' button.
	iMargin -= ( 5 + rectStaticText.Width() );
	iStaticTop = rectBackButton.top + ( rectBackButton.Height() - rectStaticText.Height() ) / 2;
	m_StaticNumAdd.SetWindowPos( NULL, iMargin, iStaticTop, rectStaticText.Width(), rectStaticText.Height(), SWP_NOOWNERZORDER );
}
