#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPPageHCActuators.h"


IMPLEMENT_DYNAMIC( CDlgTPPageHCActuators, CDlgCtrlPropPage )

CDlgTPPageHCActuators::CDlgTPPageHCActuators( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_pTADS = NULL;
	m_fAutomaticSelect = false;
	m_fSelectByPackage = false;
	m_strPowerSupply = _T("");
	m_strInputSignal = _T("");
	m_iFailSafeFct = 0;
	m_eDefaultReturnPosFct = CDB_ControlValve::DRPFunction::drpfNone;
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCACTUATORS_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCACTUATORS_PAGENAME );
}

void CDlgTPPageHCActuators::Init( bool fResetToDefault )
{
	// Fill variables for combobox.
	if( true == fResetToDefault )
	{
		m_pTADS->GetpProjectParams()->GetpHmCalcParams()->ResetPrjParams( true );
	}
	
	CPrjParams *pHMGenParam = TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );

	m_fAutomaticSelect = pHMGenParam->GetActuatorAutomaticallySelect();
	m_fSelectByPackage = pHMGenParam->GetActuatorSelectedByPackage();
	m_CheckAutomaticSelect.SetCheck( ( true == m_fAutomaticSelect ) ? BST_CHECKED : BST_UNCHECKED );
	m_CheckSelectByPackage.SetCheck( ( true == m_fSelectByPackage ) ? BST_CHECKED : BST_UNCHECKED );

	m_strPowerSupply = pHMGenParam->GetPrjParamID( CPrjParams::ActuatorPowerSupplyID );
	m_strInputSignal = pHMGenParam->GetPrjParamID( CPrjParams::ActuatorInputSignalID );
	m_iFailSafeFct = pHMGenParam->GetActuatorFailSafeFct();
	ASSERT( -1 != m_iFailSafeFct );
	m_CheckFailSafeFct.SetCheck( ( 1 == m_iFailSafeFct ) ? BST_CHECKED :  BST_UNCHECKED );
	m_eDefaultReturnPosFct = pHMGenParam->GetActuatorDRPFct();
	FillComboBoxPowerSupply();
	FillComboBoxInputSignal();
	FillComboBoxDRP();
	if( BST_CHECKED == m_CheckFailSafeFct.GetCheck() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_FSP );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
	else
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_DRP );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}

	OnBnClickedAutomaticSelect();
}

void CDlgTPPageHCActuators::Save( CString strSectionName )
{
	CPrjParams *pHMGenParam = TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );

	if( NULL != GetSafeHwnd() )
	{
		pHMGenParam->SetActuatorAutomaticallySelect( m_fAutomaticSelect );
		pHMGenParam->SetActuatorSelectedByPackage( m_fSelectByPackage );
		pHMGenParam->SetPrjParamID( CPrjParams::ActuatorPowerSupplyID, m_strPowerSupply );
		pHMGenParam->SetPrjParamID( CPrjParams::ActuatorInputSignalID, m_strInputSignal );
		pHMGenParam->SetActuatorFailSafeFct( m_iFailSafeFct );
		pHMGenParam->SetActuatorDRPFct( m_eDefaultReturnPosFct );
	}
}

BEGIN_MESSAGE_MAP( CDlgTPPageHCActuators, CDlgCtrlPropPage )
	ON_BN_CLICKED( IDC_CHECKAUTOMATICSELECT, OnBnClickedAutomaticSelect )
	ON_BN_CLICKED( IDC_CHECKSELECTBYPACKAGE, OnBnClickedSelectByPackage )
	ON_CBN_SELCHANGE( IDC_COMBOPOWERSUPPLY, OnCbnSelChangePowerSupply )
	ON_CBN_SELCHANGE( IDC_COMBOINPUTSIGNAL, OnCbnSelChangeInputSafe )
	ON_CBN_SELCHANGE( IDC_COMBODRP, OnCbnSelChangeDRP )
	ON_BN_CLICKED( IDC_CHECKFAILSAFE, OnBnClickedFailSafe )
END_MESSAGE_MAP()

void CDlgTPPageHCActuators::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECKAUTOMATICSELECT, m_CheckAutomaticSelect );
	DDX_Control( pDX, IDC_CHECKSELECTBYPACKAGE, m_CheckSelectByPackage );
	DDX_Control( pDX, IDC_COMBOPOWERSUPPLY, m_ComboPowerSupply );
	DDX_Control( pDX, IDC_COMBOINPUTSIGNAL, m_ComboInputSignal );
	DDX_Control( pDX, IDC_COMBODRP, m_ComboDRP );
	DDX_Control( pDX, IDC_CHECKFAILSAFE, m_CheckFailSafeFct );
}

BOOL CDlgTPPageHCActuators::OnInitDialog() 
{
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}
	
	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCACTUATORS_STATICAUTOMATICSELECT );
	m_CheckAutomaticSelect.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCACTUATORS_STATICSELECTBYPACKAGE );
	m_CheckSelectByPackage.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCACTUATORS_STATICPOWERSUPPLY );
	GetDlgItem( IDC_STATICPOWERSUPPLY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCACTUATORS_STATICINPUTSIGNAL );
	GetDlgItem( IDC_STATICINPUTSIGNAL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCACTUATORS_STATICFAILSAFE );
	GetDlgItem( IDC_CHECKFAILSAFE )->SetWindowText( str ); 
	
	str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_DRP );
	GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	
	m_pTADS = TASApp.GetpTADS();

	Init();
	
	return TRUE;
}

void CDlgTPPageHCActuators::OnBnClickedAutomaticSelect()
{
	m_fAutomaticSelect = ( BST_CHECKED == m_CheckAutomaticSelect.GetCheck() ) ? true : false;	
	if( false == m_fAutomaticSelect )
	{
		m_ComboPowerSupply.EnableWindow( FALSE );
		m_ComboInputSignal.EnableWindow( FALSE );
		m_ComboDRP.EnableWindow( FALSE );
		m_CheckFailSafeFct.EnableWindow( FALSE );
	}
	else
	{
		m_ComboPowerSupply.EnableWindow( (m_ComboPowerSupply.GetCount() > 1) ? TRUE : FALSE );
		m_ComboInputSignal.EnableWindow( (m_ComboInputSignal.GetCount() > 1) ? TRUE : FALSE );
		m_ComboDRP.EnableWindow( (m_ComboDRP.GetCount() > 1) ? TRUE : FALSE );
		m_CheckFailSafeFct.EnableWindow( TRUE );
	}
}

void CDlgTPPageHCActuators::OnBnClickedSelectByPackage()
{
	m_fSelectByPackage = ( BST_CHECKED == m_CheckSelectByPackage.GetCheck() ) ? true : false;
}

void CDlgTPPageHCActuators::OnCbnSelChangePowerSupply()
{
	CDB_StringID* pStrIDPointer = dynamic_cast<CDB_StringID*>( (CData *)m_ComboPowerSupply.GetItemData( m_ComboPowerSupply.GetCurSel() ) );
	if( NULL == pStrIDPointer )
		return;
	m_strPowerSupply = pStrIDPointer->GetIDPtr().ID;
	
	// Refresh combobox.
	FillComboBoxInputSignal();
	SetCheckFailSafe(true);
	FillComboBoxDRP();
}

void CDlgTPPageHCActuators::OnCbnSelChangeInputSafe()
{
	CDB_StringID* pStrIDPointer = dynamic_cast<CDB_StringID*>( (CData *)m_ComboInputSignal.GetItemData( m_ComboInputSignal.GetCurSel() ) );
	if( NULL == pStrIDPointer )
		return;
	m_strInputSignal = pStrIDPointer->GetIDPtr().ID;
	
	// Refresh combobox.
	SetCheckFailSafe(true);
	FillComboBoxDRP();
}

void CDlgTPPageHCActuators::OnCbnSelChangeDRP()
{
	m_eDefaultReturnPosFct = (CDB_ControlValve::DRPFunction)m_ComboDRP.GetItemData( m_ComboDRP.GetCurSel() );
}

void CDlgTPPageHCActuators::OnBnClickedFailSafe()
{
	m_iFailSafeFct = ( BST_CHECKED == m_CheckFailSafeFct.GetCheck() ) ? 1 : 0;
	if( BST_CHECKED == m_CheckFailSafeFct.GetCheck() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_FSP );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
	else
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_DRP );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
	FillComboBoxDRP();
}

void CDlgTPPageHCActuators::FillComboBoxPowerSupply()
{
	CRank List( false );
	int iCount = 0;
	CTable* pActuatorTable = (CTable*)( TASApp.GetpTADB()->Get( _T("ACTUATOR_TAB") ).MP );			ASSERT( pActuatorTable != NULL );
	iCount = TASApp.GetpTADB()->GetActuatorPowerSupplyList( &List, pActuatorTable, CDB_ControlProperties::CvCtrlType::eCvNU, CTADatabase::FilterSelection::ForHMCalc );
	FillComboBox( &List, &m_ComboPowerSupply, iCount, &m_strPowerSupply );
}

void CDlgTPPageHCActuators::FillComboBoxInputSignal()
{
	CRank List( false );
	int iCount = 0;
	CTable* pActuatorTable = (CTable*)( TASApp.GetpTADB()->Get( _T("ACTUATOR_TAB") ).MP );			ASSERT( pActuatorTable != NULL );
	iCount = TASApp.GetpTADB()->GetActuatorInputSignalList( &List, pActuatorTable, CDB_ControlProperties::CvCtrlType::eCvNU, (LPCTSTR)m_strPowerSupply, CTADatabase::FilterSelection::ForHMCalc );
	FillComboBox( &List, &m_ComboInputSignal, iCount, &m_strInputSignal );
}

void CDlgTPPageHCActuators::FillComboBoxDRP()
{
	CRank List( false );
	int iCount = 0;
	CTable* pActuatorTable = (CTable*)( TASApp.GetpTADB()->Get( _T("ACTUATOR_TAB") ).MP );			ASSERT( pActuatorTable != NULL );
	iCount = TASApp.GetpTADB()->GetActuatorDRPList( &List, pActuatorTable, CDB_ControlProperties::CvCtrlType::eCvNU,  NULL, 
		(LPCTSTR)m_strPowerSupply, (LPCTSTR)m_strInputSignal, m_iFailSafeFct, CTADatabase::FilterSelection::ForHMCalc );
	FillComboBox( &List, &m_ComboDRP, iCount, NULL );
	for( int i = 0; i < m_ComboDRP.GetCount() ; i++ )
	{
		if( (CDB_ControlValve::DRPFunction)m_ComboDRP.GetItemData( i ) == m_eDefaultReturnPosFct )
		{
			m_ComboDRP.SetCurSel( i );
			break;
		}
	}
	if( m_eDefaultReturnPosFct != ( CDB_ControlValve::DRPFunction )m_ComboDRP.GetItemData( 0 ) )
	{
		m_eDefaultReturnPosFct = ( CDB_ControlValve::DRPFunction )m_ComboDRP.GetItemData( 0 );
	}
}

void CDlgTPPageHCActuators::SetCheckFailSafe( bool bKeepValueIfMatched )
{
	CRank List( false );
	int iCount = 0;
	CTable* pActuatorTable = (CTable*)( TASApp.GetpTADB()->Get( _T("ACTUATOR_TAB") ).MP );			ASSERT( pActuatorTable != NULL );
	int iResult = TASApp.GetpTADB()->GetActuatorFailSafeValues( pActuatorTable, CDB_ControlProperties::CvCtrlType::eCvNU,
		(LPCTSTR)m_strPowerSupply, (LPCTSTR)m_strInputSignal, CTADatabase::FilterSelection::ForHMCalc );

	if( ( true == bKeepValueIfMatched ) && ( 0 == m_iFailSafeFct
			&& ( CTADatabase::FailSafeCheckStatus::eFirst == iResult || CTADatabase::FailSafeCheckStatus::eOnlyWithoutFailSafe == iResult
			|| CTADatabase::FailSafeCheckStatus::eBothFailSafe == iResult ) ) )
	{
		if( CTADatabase::FailSafeCheckStatus::eBothFailSafe == iResult && TRUE != m_CheckFailSafeFct.IsWindowEnabled() )
		{
			m_CheckFailSafeFct.EnableWindow( TRUE );
		}
		else if( ( CTADatabase::FailSafeCheckStatus::eFirst == iResult || CTADatabase::FailSafeCheckStatus::eOnlyWithoutFailSafe == iResult )
			&& ( FALSE != m_CheckFailSafeFct.IsWindowEnabled() ) )
		{
			m_CheckFailSafeFct.EnableWindow( FALSE );
		}
	}
	else if( ( true == bKeepValueIfMatched ) && ( 1 == m_iFailSafeFct
		&& ( CTADatabase::FailSafeCheckStatus::eOnlyWithFailSafe == iResult || CTADatabase::FailSafeCheckStatus::eBothFailSafe == iResult ) ) )
	{
		if( CTADatabase::FailSafeCheckStatus::eBothFailSafe == iResult && TRUE != m_CheckFailSafeFct.IsWindowEnabled() )
		{
			m_CheckFailSafeFct.EnableWindow( TRUE );
		}
		else if( CTADatabase::FailSafeCheckStatus::eOnlyWithFailSafe == iResult && FALSE != m_CheckFailSafeFct.IsWindowEnabled() )
		{
			m_CheckFailSafeFct.EnableWindow( FALSE );
		}
	}
	else
	{
		if( CTADatabase::FailSafeCheckStatus::eFirst == iResult )
		{
			// Checkbox unchecked and disabled
			m_CheckFailSafeFct.SetCheck( BST_UNCHECKED );
			m_CheckFailSafeFct.EnableWindow( false );
		}
		else if( CTADatabase::FailSafeCheckStatus::eOnlyWithoutFailSafe == iResult )
		{
			// Checkbox unchecked and enable
			m_CheckFailSafeFct.SetCheck( BST_UNCHECKED );
			m_CheckFailSafeFct.EnableWindow( false );
		}
		else if( CTADatabase::FailSafeCheckStatus::eOnlyWithFailSafe == iResult )
		{
			// Checkbox checked and enable
			m_CheckFailSafeFct.SetCheck( BST_CHECKED );
			m_CheckFailSafeFct.EnableWindow( false );
		}
		else if( CTADatabase::FailSafeCheckStatus::eBothFailSafe == iResult )
		{
			// Checkbox checked and enable
			m_CheckFailSafeFct.SetCheck( BST_UNCHECKED );
			m_CheckFailSafeFct.EnableWindow( true );
		}
		m_iFailSafeFct = ( BST_CHECKED == m_CheckFailSafeFct.GetCheck() ) ? 1 : 0;
	}

	if( BST_CHECKED == m_CheckFailSafeFct.GetCheck() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_FSP );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
	else
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_DRP );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
}

void CDlgTPPageHCActuators::FillComboBox( CRank* pList, CComboBox* pCCombo, int iCount, CString* pCstr )
{
	CString str1;
	LPARAM lpParam;
	if( iCount > 0 && TRUE == pList->GetFirst( str1, lpParam ) )
	{
		pList->Transfer( pCCombo );
		int iNbre = pCCombo->GetCount();
		int iPos = -1;
		if( pCstr != NULL )
		{
			for( int i = 0; i < iNbre && -1 == iPos; i++ )
			{
				CDB_StringID* pStrIDPointer = dynamic_cast<CDB_StringID*>( (CData *)pCCombo->GetItemData( i ) );
				if( pStrIDPointer != NULL && pStrIDPointer->GetIDPtr().ID == *pCstr )
				{
					pCCombo->SetCurSel( i );
					iPos = i;
				}
			}
		}
		
		if( -1 == iPos && iNbre != 0 )
		{
			pCCombo->SetCurSel( 0 );
			if( pCstr != NULL )
			{
				CDB_StringID* pStrIDPointer = dynamic_cast<CDB_StringID*>( (CData *)pCCombo->GetItemData( 0 ) );
				*pCstr = pStrIDPointer->GetIDPtr().ID;
			}
		}
	}
	
	if( iCount < 2 )
		pCCombo->EnableWindow( FALSE );
	else
		pCCombo->EnableWindow( TRUE );
	
	pList->PurgeAll();
}
