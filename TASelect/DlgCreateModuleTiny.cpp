#include "stdafx.h"
#include "TASelect.h"
#include "Global.h"
#include "Utilities.h"
#include "Units.h"
#include "Hydromod.h"
#include "DlgHMDirSel.h"
#include "DlgKvCvInput.h"
#include "DlgCreateModuleTiny.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgCreateModuleTiny::CDlgCreateModuleTiny( CWnd* pParent )
	: CDialogEx( CDlgCreateModuleTiny::IDD, pParent )
{
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pUnitDB = NULL;
	m_eDlgMode = DialogMode::Undefined;
	m_bInitialized = false;
	m_dFlow = 0.0;
	m_dSelPresetting = 0.0;
	m_dKvCv = 0.0;
	m_fValveChanged = false;
	m_strSelTADBID = _T("");
	m_strSelTADBCategory = _T("");
	m_strCBIType = _T("");
	m_strCBISize = _T("");
	m_eVDescrType = enum_VDescriptionType::edt_Undefined;
}

int CDlgCreateModuleTiny::Display(	DialogMode eDlgMode, double dFlow, CString strSelValveID, enum_VDescriptionType eVDescrType, double dKvCv, double dPresetting )
{
	m_eDlgMode = eDlgMode;
	m_dFlow = dFlow;
	m_strSelTADBID = strSelValveID;
	m_eVDescrType = eVDescrType;
	m_dKvCv = dKvCv;
	m_dSelPresetting = dPresetting;

	return DoModal();
}

BEGIN_MESSAGE_MAP( CDlgCreateModuleTiny, CDialogEx )
	ON_WM_MOVE()
	ON_BN_CLICKED( IDC_CHECKPARTNER, OnCheckPartnerValve )
	ON_EN_CHANGE( IDC_EDITFLOW, OnEnChangeFlow )
	ON_BN_CLICKED( IDC_BUTTONKVCVINPUT, OnButtonKvCvInput )
	ON_BN_CLICKED( IDC_BUTTONDIRECTSEL, OnButtonDirectSelection )
END_MESSAGE_MAP()

void CDlgCreateModuleTiny::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECKPARTNER, m_CheckPartnerValve );
	DDX_Control( pDX, IDC_EDITFLOW, m_EditFlow );
	DDX_Control( pDX, IDC_EDITVALVE, m_EditValve );
	DDX_Control( pDX, IDC_BUTTONKVCVINPUT, m_ButtonKvCvInput );
	DDX_Control( pDX, IDC_BUTTONDIRECTSEL, m_ButtonDirectSel );
}

BOOL CDlgCreateModuleTiny::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Set the window position to the last stored position in registry.
	// If window position is not yet stored in the registry, the window is centered by default in the mainframe rect.
	CRect apprect, rect;
	::AfxGetApp()->m_pMainWnd->GetWindowRect( &apprect );
	GetWindowRect( &rect );
	int x = ::AfxGetApp()->GetProfileInt( _T("DialogCreateModuleBurst"), _T("ScreenXPos"), apprect.CenterPoint().x - rect.Width() / 2 );
	int y = ::AfxGetApp()->GetProfileInt( _T("DialogCreateModuleBurst"), _T("ScreenYPos"), apprect.CenterPoint().y - rect.Height() / 2 );
	SetWindowPos( NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

	// If the window is placed on a screen that is currently deactivated, the window is centered to the application.
	HMONITOR hMonitor = NULL;
	GetWindowRect( &rect );
	hMonitor = MonitorFromRect( &rect, MONITOR_DEFAULTTONULL );
	if( NULL == hMonitor )
		CenterWindow();

	// Set caption.
	CString str;
	if( DialogMode::CreateModule == m_eDlgMode )
		SetWindowText( TASApp.LoadLocalizedString( IDS_DLGCREATEMODULETINY_MODULE ) );
	else
		SetWindowText( TASApp.LoadLocalizedString( IDS_DLGCREATEMODULETINY_VALVE ) );

	m_pTADB = TASApp.GetpTADB();
	m_pTADS = TASApp.GetpTADS();

	// 'Partner valve' checkbox.
	if( DialogMode::CreateModule == m_eDlgMode )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_CHECKCREATEPARTNER );
		m_CheckPartnerValve.SetWindowText( str );
	}
	else
		m_CheckPartnerValve.ShowWindow( SW_HIDE );

	if( DialogMode::AddValve == m_eDlgMode )
		m_CheckPartnerValve.SetCheck( TRUE );

	// 'Design flow' static.
	m_pUnitDB = CDimValue::AccessUDB();
	TCHAR name[_MAXCHARS];
	str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_STATICDESIGNQ );
	GetDlgItem( IDC_STATICFLOW )->SetWindowText( str );

	// 'Design flow' edit.
	if( DialogMode::CreateModule == m_eDlgMode )
	{
		m_EditFlow.SetReadOnly( TRUE );
		m_EditFlow.SetWindowText( _T("") );
	}
	else
		m_EditFlow.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_FLOW, m_dFlow ), 3, 0, 1 ) );

	// 'Design flow' unit.
	GetNameOf( m_pUnitDB->GetUnit( _U_FLOW, m_pUnitDB->GetDefaultUnitIndex( _U_FLOW ) ), name );
	GetDlgItem( IDC_STATICFLOWUNIT )->SetWindowText( name );

	// 'Valve' static.
	str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_STATICVALVE );
	GetDlgItem( IDC_STATICVALVE )->SetWindowText( str );

	// Valve' edit.
	if( enum_VDescriptionType::edt_TADBValve == m_eVDescrType && false == m_strSelTADBID.IsEmpty() )
	{
		m_CheckPartnerValve.SetCheck( TRUE );
		IDPTR IDPtr = m_pTADB->Get( m_strSelTADBID );
		m_EditValve.SetWindowText( ( (CDB_TAProduct *)IDPtr.MP )->GetName() );
	}
	else if( enum_VDescriptionType::edt_KvCv == m_eVDescrType && 0.0 != m_dKvCv )
	{
		m_CheckPartnerValve.SetCheck( TRUE );
		if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			str = TASApp.LoadLocalizedString( IDS_KV );
		else
			str = TASApp.LoadLocalizedString( IDS_CV );
		str += (CString)_T(" = ") + WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, m_dKvCv ), 3, 0 );
		m_EditValve.SetWindowText( str );
	}
	else if( ( enum_VDescriptionType::edt_CBISizeValve == m_eVDescrType || enum_VDescriptionType::edt_CBISizeInchValve == m_eVDescrType )
		&& false == m_strCBIType.IsEmpty() && false == m_strCBISize.IsEmpty() )
	{
		m_CheckPartnerValve.SetCheck( TRUE );
		str = m_strCBIType + (CString)_T(" ") + m_strCBISize + (CString)_T("*");		
		m_EditValve.SetWindowText( str );
	}

	// Compose string for the 'input Kv' button.
	CString tmpstr;
	if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		str = TASApp.LoadLocalizedString( IDS_KV );
	else
		str = TASApp.LoadLocalizedString( IDS_CV );
	FormatString( tmpstr, IDS_DLGCREATEMODULE_BUTTONKVCVINPUT, str );
	m_ButtonKvCvInput.SetWindowText( tmpstr );

	// Load string for 'Direct selection' button.
	str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_BUTTONDIRECTSEL );
	m_ButtonDirectSel.SetWindowText( str );

	m_fValveChanged = false;
	m_strSelTADBID = _T("");
	OnCheckPartnerValve();

	// 'OK' button.
	str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_BUTADD );
	GetDlgItem( IDOK )->SetWindowText( str );

	// 'Cancel' button.
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	
	m_bInitialized = true;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCreateModuleTiny::OnOK() 
{
	if( false == _CheckEditQ() )
		return;

	CDialogEx::OnOK();
}

void CDlgCreateModuleTiny::OnCancel() 
{
	CDialogEx::OnCancel();
}

void CDlgCreateModuleTiny::OnMove( int x, int y )
{
	CDialogEx::OnMove( x,  y );
	
	if( true == m_bInitialized )
	{
		CRect rect;
		GetWindowRect( &rect );
		::AfxGetApp()->WriteProfileInt( _T("DialogCreateModuleBurst"), _T("ScreenXPos"), x );
		::AfxGetApp()->WriteProfileInt( _T("DialogCreateModuleBurst"), _T("ScreenYPos"), y );
	}
}

void CDlgCreateModuleTiny::OnCheckPartnerValve() 
{
	if( BST_CHECKED == m_CheckPartnerValve.GetCheck() )
	{
		m_EditFlow.EnableWindow( TRUE );
		GetDlgItem( IDC_STATICFLOW )->EnableWindow( TRUE );
		GetDlgItem( IDC_STATICFLOWUNIT )->EnableWindow( TRUE );
		m_EditValve.EnableWindow( TRUE );
		GetDlgItem( IDC_STATICVALVE )->EnableWindow( TRUE );
		m_ButtonDirectSel.EnableWindow( TRUE );
		m_ButtonKvCvInput.EnableWindow( TRUE );
	}
	else
	{
		m_EditValve.SetWindowText( _T("") );
		m_EditFlow.EnableWindow( FALSE );
		GetDlgItem( IDC_STATICFLOW )->EnableWindow( FALSE );
		GetDlgItem( IDC_STATICFLOWUNIT )->EnableWindow( FALSE );
		m_EditValve.EnableWindow( FALSE );
		GetDlgItem( IDC_STATICVALVE )->EnableWindow( FALSE );
		m_ButtonDirectSel.EnableWindow( FALSE );
		m_ButtonKvCvInput.EnableWindow( FALSE );
		m_fValveChanged = true;
		m_eVDescrType = edt_TADBValve;
	}
}

void CDlgCreateModuleTiny::OnEnChangeFlow() 
{
	CString str;
	m_EditFlow.GetWindowText( str );
	switch( ReadDouble( str, &m_dFlow ) )
	{
		case RD_EMPTY:
			return;
	
		case RD_NOT_NUMBER:
			if( _T(".") == str || _T(",") == str || _T("e") == str.Right( 1 ) || _T("e+") == str.Right( 2 ) || _T("e-") == str.Right( 2 ) ) 
				return;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			m_EditFlow.SetSel( 0, -1 );
			m_EditFlow.SetFocus();
			return;
			break;
	
		case RD_OK:
			if( m_dFlow < 0.0 )
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				m_EditFlow.SetSel( 0, -1 );
				m_EditFlow.SetFocus();
				return;
			}
			else
				m_dFlow = CDimValue::CUtoSI( _U_FLOW, m_dFlow );
			break;
	};
	return;
}

void CDlgCreateModuleTiny::OnButtonKvCvInput() 
{
	CDlgExtKvCvInput dlg;
	CString str;
	double dKvCv = GetKvCv();
	double dFlow = GetFlow();
	if( dFlow <= 0.0 )
		dFlow = 0.0;

	int nRet = dlg.Display( &dKvCv, dFlow );
	if( IDOK == nRet )
	{
		// Assign KvCv value to the member variable for use in 'TabCDialogExtProj' through 'GetKvCv()'.
		m_dKvCv = dKvCv;
		m_eVDescrType = edt_KvCv;

		// Reset other fields.
		m_strSelTADBID = _T("");
		m_strSelTADBCategory = _T("");
		m_dSelPresetting = 0.0;  

		m_fValveChanged = true;

		// Refresh the read-only valve edit box.
		if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			str = TASApp.LoadLocalizedString( IDS_KV );
		else
			str = TASApp.LoadLocalizedString( IDS_CV );
		str += (CString)_T(" = ") + WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, m_dKvCv ), 3, 0 );
		m_EditValve.SetWindowText( str );
	}
}

void CDlgCreateModuleTiny::OnButtonDirectSelection() 
{
	CDlgHMDirSel dlg;
	IDPTR IDPtr = m_pTADB->Get( m_strSelTADBID );
	CString CurrentID;

	// One selection is in course but not yet validated into the TADS.
	if( false == m_strSelTADBID.IsEmpty() )
		CurrentID = m_strSelTADBID;

	int nRet;
	if( enum_VDescriptionType::edt_CBISizeValve == GetVDescrType() || enum_VDescriptionType::edt_CBISizeInchValve == GetVDescrType() )
		nRet = dlg.Display( m_strCBIType, m_strCBISize, m_dSelPresetting );
	else
		nRet = dlg.Display( CurrentID, m_dSelPresetting );

	if( IDOK == nRet )
	{
		m_strSelTADBID = dlg.GetSelThingID();
		m_strSelTADBCategory = dlg.GetCategoryTab();
		m_dSelPresetting = dlg.GetPresetting();

		// We can force 'edt_TADBValve' because in 'CDlgHMDirSel' we convert 'CBIType' and 'CBISize' to a TA valve equivalent found in TADB.
		m_eVDescrType = enum_VDescriptionType::edt_TADBValve;
		m_dKvCv = 0.0;
		
		// The current valve selection in Hydraulic module is invalid you must perform a new one.
		if( m_strSelTADBID != CurrentID )
		{
			m_fValveChanged = true;
		}

		// Refresh the Read-only valve edit box.
		IDPTR IDPtr = m_pTADB->Get( m_strSelTADBID );
		m_EditValve.SetWindowText( ( (CDB_TAProduct *)IDPtr.MP )->GetName() );
	}
}

bool CDlgCreateModuleTiny::_CheckEditQ()
{
	CString str, strValve;
	m_EditFlow.GetWindowText( str );
	m_EditValve.GetWindowText( strValve );
	switch( ReadDouble( str, &m_dFlow ) )
	{
		case RD_EMPTY:
			m_dFlow = 0.0;
			return true;
	
		case RD_NOT_NUMBER:
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			m_EditFlow.SetSel( 0, -1 );
			m_EditFlow.SetFocus();
			return false;
			break;
	
		case RD_OK:
			if( DialogMode::AddValve == m_eDlgMode && m_dFlow <= 0.0 )
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				m_EditFlow.SetSel( 0, -1 );
				m_EditFlow.SetFocus();
				return false;
			}
			else
			{
				// In case of module the total flow will be computed later.
				if( DialogMode::CreateModule == m_eDlgMode )
					m_dFlow = 0.0;
				else
					m_dFlow = CDimValue::CUtoSI( _U_FLOW, m_dFlow );
			}
			break;
	};
	return true;
}
