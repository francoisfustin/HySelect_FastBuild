#include "stdafx.h"


#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Utilities.h"
#include "Units.h"
#include "Hydromod.h"
#include "DlgHMDirSel.h"
#include "DlgKvCvInput.h"
#include "DlgCreateModule.h"
#include "DlgLeftTabProject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgCreateModule::CDlgCreateModule( CWnd* pParent, INotification *pclNotification )
	: CDialogEx( CDlgCreateModule::IDD, pParent )
{
	m_eDlgMode = CreateModule;
	m_pView = NULL;
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pUnitDB = NULL;
	m_strCaption = _T("");
	m_bInitialized = false;
	m_iPosition = -1;
	m_iMax = -1;
	m_strReference = _T("");
	m_strDescription = _T("");
	m_dFlow = 0.0;
	m_dSelPresetting = 0.0;
	m_dKvCv = 0.0;
	m_bValveChanged = false;
	m_strSelTADBID = _T("");
	m_strSelTADBCategory = _T("");
	m_strCBIType = _T("");
	m_strCBISize = _T("");
	m_pPHM = NULL;
	m_eVDescrType = enum_VDescriptionType::edt_Undefined;
	m_pclNotification = pclNotification;
}

CDlgCreateModule::CDlgCreateModule( CView* pView, INotification *pclNotification )
{
	m_eDlgMode = CreateModule;
	m_pView = pView;
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pUnitDB = NULL;
	m_strCaption = _T("");
	m_bInitialized = false;
	m_iPosition = -1;
	m_iMax = -1;
	m_strReference = _T("");
	m_strDescription = _T("");
	m_dFlow = 0.0;
	m_dSelPresetting = 0.0;
	m_dKvCv = 0.0;
	m_bValveChanged = false;
	m_strSelTADBID = _T("");
	m_strSelTADBCategory = _T("");
	m_strCBIType = _T("");
	m_strCBISize = _T("");
	m_pPHM = NULL;
	m_eVDescrType = enum_VDescriptionType::edt_Undefined;
	m_pclNotification = pclNotification;
}

int CDlgCreateModule::Display(	DialogMode eDlgMode, CString strCaption, CString strReference, CDS_HydroMod* pPHM, int iIndex, int iMaxIndex, double dFlow,
								CString strSelValveID, enum_VDescriptionType eVDescrType, double dKvCv, double dPresetting )
{
	m_strCaption = strCaption;
	m_pPHM = pPHM;
	m_eDlgMode = eDlgMode;
	m_strReference = strReference;
	m_iPosition = iIndex;
	m_iMax = iMaxIndex;
	m_dFlow = dFlow;
	m_strSelTADBID = strSelValveID;
	m_eVDescrType = eVDescrType;
	m_dKvCv = dKvCv;
	m_dSelPresetting = dPresetting;

	if( NULL == m_pView )
		return DoModal();
	else
		return CDialogEx::Create( CDlgCreateModule::IDD );
}

int CDlgCreateModule::Display( DialogMode eDlgMode, CString strCaption, CDS_HydroMod* pHM, int iMaxIndex )
{
	m_strDescription = pHM->GetDescription();
	m_strCBIType = pHM->GetCBIType();
	m_strCBISize = pHM->GetCBISize();
	m_strSelTADBID = _T("");

	return Display( eDlgMode, strCaption, pHM->GetHMName(), (CDS_HydroMod *)( pHM->GetIDPtr().PP ), pHM->GetPos(), iMaxIndex,
					pHM->GetQDesign(), pHM->GetCBIValveID(), pHM->GetVDescrType(), pHM->GetKvCv(), pHM->GetPresetting() );
}

BEGIN_MESSAGE_MAP( CDlgCreateModule, CDialogEx )
	ON_WM_MOVE()
	ON_CBN_SELCHANGE( IDC_COMBOPARENT, OnCbnSelChangeLocatedIn )
	ON_NOTIFY( UDN_DELTAPOS, IDC_SPIN, OnDeltaSpinPosition )
	ON_BN_CLICKED( IDC_CHECKPARTNER, OnCheckPartnerValve )
	ON_EN_CHANGE( IDC_EDITFLOW, OnEnChangeFlow )
	ON_BN_CLICKED( IDC_BUTTONKVCVINPUT, OnButtonKvCvInput )
	ON_BN_CLICKED( IDC_BUTTONDIRECTSEL, OnButtonDirectSelection )
END_MESSAGE_MAP()

void CDlgCreateModule::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOPARENT, m_ComboLocatedIn  );
	DDX_Control( pDX, IDC_EDITPOSITION, m_EditPosition );
	DDX_Control( pDX, IDC_SPIN, m_SpinPosition );
	DDX_Control( pDX, IDC_EDITREF, m_clEditReference );
	DDX_Control( pDX, IDC_EDITDESCRIPT, m_EditDescription );
	DDX_Control( pDX, IDC_CHECKPARTNER, m_CheckPartnerValve );
	DDX_Control( pDX, IDC_EDITFLOW, m_EditFlow );
	DDX_Control( pDX, IDC_EDITVALVE, m_EditValve );
	DDX_Control( pDX, IDC_BUTTONKVCVINPUT, m_ButtonKvCvInput );
	DDX_Control( pDX, IDC_BUTTONDIRECTSEL, m_ButtonDirectSel );
}

BOOL CDlgCreateModule::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Set the window position to the last stored position in registry.
	// If window position is not yet stored in the registry, the window is centered by default in the mainframe rect.
	CRect apprect, rect;
	::AfxGetApp()->m_pMainWnd->GetWindowRect( &apprect );
	GetWindowRect( &rect );
	int x = ::AfxGetApp()->GetProfileInt( _T("DialogCreateModule"), _T("ScreenXPos"), apprect.CenterPoint().x - rect.Width() / 2 );
	int y = ::AfxGetApp()->GetProfileInt( _T("DialogCreateModule"), _T("ScreenYPos"), apprect.CenterPoint().y - rect.Height() / 2 );
	SetWindowPos( NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

	// If the window is placed on a screen that is currently deactivated, the window is centered to the application.
	HMONITOR hMonitor = NULL;
	GetWindowRect( &rect );
	hMonitor = MonitorFromRect( &rect, MONITOR_DEFAULTTONULL );
	if( NULL == hMonitor )
		CenterWindow();

	SetWindowText( m_strCaption );
	m_pTADB = TASApp.GetpTADB();
	m_pTADS = TASApp.GetpTADS();

	// Load bitmap and affect it to the static.
	if( DialogMode::CreateModule == m_eDlgMode || DialogMode::EditModule == m_eDlgMode )
		m_Bitmap.LoadMappedBitmap( IDB_INSERTMODULE );
	else
		m_Bitmap.LoadMappedBitmap( IDB_INSERTVALVE );
	HBITMAP hBitmap = (HBITMAP)m_Bitmap.GetSafeHandle();
	( (CStatic*)GetDlgItem( IDC_STATICBITMAP ) )->SetBitmap( hBitmap );

	// 'Located in' static.
	CString str;
	if( DialogMode::CreateModule == m_eDlgMode || DialogMode::CreateValve == m_eDlgMode )
		str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_STATICPARENT );
	else
		str = TASApp.LoadLocalizedString(IDS_DLGCREATEMODULE_STATICPARENTLOC );
	GetDlgItem( IDC_STATICPARENT )->SetWindowText( str );

	// Fill 'Located in' combo.
	CTable *pTab = m_pTADS->GetpHydroModTable();
	ASSERT( NULL != pTab );

	for( IDPTR HMIDPtr = pTab->GetFirst(); _T('\0') != *HMIDPtr.ID; HMIDPtr = pTab->GetNext() )
	{
		_FillComboParent( (CDS_HydroMod *)HMIDPtr.MP );
	}

	// Select the current selected module.
	int iIndex = 0;

	if( NULL != m_pPHM && false == m_pPHM->IsClass( CLASS( CTableHM ) ) )
	{
		iIndex = m_ComboLocatedIn.FindStringExact( -1, m_pPHM->GetHMName() );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK );
		iIndex = m_ComboLocatedIn.AddString( str );
		m_ComboLocatedIn.SetItemDataPtr( iIndex, 0 );
		m_ComboLocatedIn.EnableWindow( FALSE );
	}

	m_ComboLocatedIn.SetCurSel( iIndex );

	// 'Position' and 'spin'.
	if( DialogMode::CreateModule == m_eDlgMode || DialogMode::CreateValve == m_eDlgMode )
		str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_STCPOSCREATE );
	else
		str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_STATICPOS );
	GetDlgItem( IDC_STATICPOS )->SetWindowText( str );
	
	if( 0 == m_iPosition || 0 == m_iMax )
	{
		m_EditPosition.ShowWindow( SW_HIDE );
		m_SpinPosition.ShowWindow( SW_HIDE );
		GetDlgItem( IDC_STATICPOS )->ShowWindow( SW_HIDE );
	}

	m_SpinPosition.SetRange( 1, m_iMax );
	m_SpinPosition.SetPos( m_iPosition );
	m_clEditReference.SetWindowText( (LPCTSTR)m_strReference );
	m_clEditReference.LimitText( TASApp.GetModuleNameMaxChar() );	
	m_EditDescription.LimitText( MAXMODULEDESCRIPTIONCHAR );

	// 'Reference' static.
	str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_STATICREF );
	GetDlgItem( IDC_STATICREF )->SetWindowText( str );

	// 'Description' static.
	str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_STATICDESCRIPT );
	GetDlgItem( IDC_STATICDESCRIPT )->SetWindowText(str);
	m_EditDescription.SetWindowText( m_strDescription );

	// 'Partner valve' checkbox.
	if( DialogMode::CreateModule == m_eDlgMode )
		str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_CHECKCREATEPARTNER );
	else if( DialogMode::EditModule == m_eDlgMode )
		str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_CHECKPARTNER );
	else
		m_CheckPartnerValve.ShowWindow( SW_HIDE );
	m_CheckPartnerValve.SetWindowText( str );

	if( DialogMode::CreateValve == m_eDlgMode || DialogMode::EditValve == m_eDlgMode )
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
	else if( DialogMode::EditModule == m_eDlgMode )
	{
		m_EditFlow.SetReadOnly( TRUE );
		m_EditFlow.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_FLOW, m_dFlow ), 3, 0, 1 ) );
	}
	else
		m_EditFlow.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_FLOW, m_dFlow ), 3, 0, 1 ) );

	// 'Design flow' unit.
	GetNameOf( m_pUnitDB->GetUnit( _U_FLOW, m_pUnitDB->GetDefaultUnitIndex( _U_FLOW ) ), name );
	GetDlgItem( IDC_STATICFLOWUNIT )->SetWindowText( name );

	// 'Valve' static.
	str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_STATICVALVE );
	GetDlgItem( IDC_STATICVALVE )->SetWindowText( str );

	// 'Valve' edit.
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

	m_bValveChanged = false;
	OnCheckPartnerValve();

	// 'OK' button.
	if( DialogMode::CreateModule == m_eDlgMode )
		str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_BUTCREATE );
	else if( DialogMode::CreateValve == m_eDlgMode )
		str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_BUTADD );
	else
	{
		str = TASApp.LoadLocalizedString( IDS_OK );
	}
	GetDlgItem( IDOK )->SetWindowText( str );

	// 'Cancel' button.
	if( DialogMode::CreateModule == m_eDlgMode || DialogMode::CreateValve == m_eDlgMode )
		str = TASApp.LoadLocalizedString( IDS_DLGCREATEMODULE_BUTCLOSE );
	else
		str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	
	m_bInitialized = true;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCreateModule::OnOK() 
{
	m_clEditReference.GetWindowText( m_strReference );
	m_EditDescription.GetWindowText( m_strDescription );

	// If user has changed the parent module...
	if( (CDS_HydroMod *)m_ComboLocatedIn.GetItemDataPtr( m_ComboLocatedIn.GetCurSel() ) != m_pPHM )
		m_pPHM = (CDS_HydroMod *)m_ComboLocatedIn.GetItemDataPtr( m_ComboLocatedIn.GetCurSel() );
	else
	{
		if( false == _CheckEditQ() )
			return;
	}
	
	if( NULL != m_pView )
		m_pView->PostMessage( WM_USER_DESTROYDIALOGADDVALVE, IDOK );
	else
		CDialogEx::OnOK();
}

void CDlgCreateModule::OnCancel() 
{
	if( NULL != m_pView )
		m_pView->PostMessage( WM_USER_DESTROYDIALOGADDVALVE, IDCANCEL );
	else
		CDialogEx::OnCancel();
}

void CDlgCreateModule::OnMove( int x, int y )
{
	CDialogEx::OnMove( x,  y );
	
	if( true == m_bInitialized )
	{
		CRect rect;
		GetWindowRect( &rect );
		::AfxGetApp()->WriteProfileInt( _T("DialogCreateModule"), _T("ScreenXPos"), x );
		::AfxGetApp()->WriteProfileInt( _T("DialogCreateModule"), _T("ScreenYPos"), y );
	}
}

void CDlgCreateModule::OnCbnSelChangeLocatedIn() 
{
	// User changed the parent module...
	if( m_pPHM != (CDS_HydroMod *)m_ComboLocatedIn.GetItemDataPtr( m_ComboLocatedIn.GetCurSel() ) )
	{
		m_pPHM = (CDS_HydroMod *)m_ComboLocatedIn.GetItemDataPtr( m_ComboLocatedIn.GetCurSel() );
		if( NULL != m_pclNotification )
		{
			m_pclNotification->OnChangeLocatedIn( this );

			// Change spin value and range.
			m_iMax = m_pPHM->GetCount() + 1;
			m_iPosition = m_iMax;
			m_SpinPosition.SetRange( 1, m_iMax );
			m_SpinPosition.SetPos( m_iPosition );

			_ChangeReference();
		}
	}
	else
	{
		// User did not change the parent module, do nothing.
		return;
	}
}

void CDlgCreateModule::OnDeltaSpinPosition( NMHDR* pNMHDR, LRESULT* pResult )
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	m_iPosition = pNMUpDown->iPos + pNMUpDown->iDelta;
	int iLow, iUp;
	m_SpinPosition.GetRange( iLow, iUp );
	if( m_iPosition > iUp )
		m_iPosition = iUp;
	if( m_iPosition < iLow )
		m_iPosition = iLow;
	_ChangeReference();

	*pResult = 0;
}

void CDlgCreateModule::OnCheckPartnerValve() 
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
		m_bValveChanged = true;
		m_eVDescrType = edt_TADBValve;
		m_dSelPresetting = 0.0;
	}
}

void CDlgCreateModule::OnEnChangeFlow() 
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

void CDlgCreateModule::OnButtonKvCvInput() 
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

		m_bValveChanged = true;

		// Refresh the read-only valve edit box.
		if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			str = TASApp.LoadLocalizedString( IDS_KV );
		else
			str = TASApp.LoadLocalizedString( IDS_CV );
		str += (CString)_T(" = ") + WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, m_dKvCv ), 3, 0 );
		m_EditValve.SetWindowText( str );
	}
}

void CDlgCreateModule::OnButtonDirectSelection() 
{
	CDlgHMDirSel dlg;
	IDPTR IDPtr = m_pTADB->Get( m_strSelTADBID );
	CString CurrentID;

	// One selection is in course but not yet validated into the TADS.
	if( false == m_strSelTADBID.IsEmpty() )
	{
		CurrentID = m_strSelTADBID;
	}

	int nRet;

	if( enum_VDescriptionType::edt_CBISizeValve == GetVDescrType() || enum_VDescriptionType::edt_CBISizeInchValve == GetVDescrType() )
	{
		nRet = dlg.Display( m_strCBIType, m_strCBISize, m_dSelPresetting );
	}
	else
	{
		nRet = dlg.Display( CurrentID, m_dSelPresetting );
	}

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
			m_bValveChanged = true;
		}

		// Refresh the Read-only valve edit box.
		IDPTR IDPtr = m_pTADB->Get( m_strSelTADBID );
		m_EditValve.SetWindowText( ( (CDB_TAProduct *)IDPtr.MP )->GetName() );
	}
}

void CDlgCreateModule::_ChangeReference( void )
{
	m_clEditReference.GetWindowText( m_strReference );

	// Change reference if it's set to automatic name.
	if( '*' == m_strReference.GetAt( 0 ) )
	{
		if( NULL == m_pPHM )
		{
			// The parent is the main root module (hydraulic network).
			// Remark: index in 'CTADatastruct::ComposeRMName' is 0-indexed (in the 'CDS_HydroMod' class it's 1-indexed!).
			m_pTADS->ComposeRMName( m_strReference, 0, m_iPosition - 1 );
		}
		else
		{
			CString strPrefix = ( '*' == m_pPHM->GetHMName().GetAt( 0 ) ) ? _T("") : _T("*");
			m_strReference.Format( _T("%s%s.%d"), strPrefix, m_pPHM->GetHMName(), m_iPosition );
				
			// If resulting valve name is larger than max. size, make it "Unnamed".
			if( m_strReference.GetLength() > TASApp.GetModuleNameMaxChar() )
			{
				m_strReference = _T("*") + TASApp.LoadLocalizedString( IDS_VALVENONAME );
				
				CTable *pTable = TASApp.GetpTADS()->GetpHydroModTable();
				ASSERT( NULL != pTable );
				
				m_pTADS->DifferentiateHMName( pTable, m_strReference );
			}
		}
		m_clEditReference.SetWindowText( m_strReference );
	}
}

bool CDlgCreateModule::_CheckEditQ()
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
			if( ( DialogMode::CreateValve == m_eDlgMode || DialogMode::EditValve == m_eDlgMode ) && m_dFlow <= 0.0 )
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				m_EditFlow.SetSel( 0, -1 );
				m_EditFlow.SetFocus();
				return false;
			}
			else
			{
				// In case of module the total flow will be computed later.
				if( DialogMode::CreateModule == m_eDlgMode || DialogMode::EditModule == m_eDlgMode )
					m_dFlow = 0.0;
				else
					m_dFlow = CDimValue::CUtoSI( _U_FLOW, m_dFlow );
			}
			break;
	};
	return true;
}

void CDlgCreateModule::_FillComboParent( CDS_HydroMod *pHM )
{
	if( false == pHM->IsaModule() )
		return;
	int iIndex = m_ComboLocatedIn.AddString( pHM->GetHMName() );
	m_ComboLocatedIn.SetItemDataPtr( iIndex, (void *)pHM );
	// Add children.
	for( IDPTR HMIDPtr = pHM->GetFirst(); '\0' != *HMIDPtr.ID; HMIDPtr = pHM->GetNext() )
		_FillComboParent( (CDS_HydroMod *)HMIDPtr.MP );
}
