#include "stdafx.h"
#include "TASelect.h"
#include "DlgCtrlPropPage.h"
#include "DlgSearchReplace.h"
#include "DlgSearchAndReplaceActuator.h"
#include "DlgSRPageActuator.h"


IMPLEMENT_DYNAMIC( CDlgSRPageActuator, CDlgCtrlPropPage )

CDlgSRPageActuator::CDlgSRPageActuator( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGSRACTUATOR_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGSRACTUATOR_HEADERTITLE );
	m_pNotificationHandler = NULL;
	m_bFailSafeFct = false;
	m_iHMCalcOrIndSel = (int)CDlgSearchReplace::RadioState::RS_Uninitialized;
	m_eDefaultReturnPosFct = CDB_ControlValve::DRPFunction::drpfUndefined;
}

CDlgSRPageActuator::~CDlgSRPageActuator()
{
}

void CDlgSRPageActuator::SetNotificationHandler( IDlgSRPageActuatorNotificationHandler* pNotificationHandler )
{
	m_pNotificationHandler = pNotificationHandler;
}

bool CDlgSRPageActuator::IsRadioRemoveChecked( void )
{
	bool fChecked = false;
	CButton* pButton = (CButton*)GetDlgItem( IDC_RADIOREMOVEACTUATOR );
	if( NULL != pButton )
		fChecked = ( BST_CHECKED == pButton->GetCheck() ? true : false );
	return fChecked;
}

bool CDlgSRPageActuator::IsRadioAddChecked( void )
{
	bool fChecked = false;
	CButton* pButton = (CButton*)GetDlgItem( IDC_RADIOADDACTUATOR );
	if( NULL != pButton )
		fChecked = ( BST_CHECKED == pButton->GetCheck() ? true : false );
	return fChecked;
}

bool CDlgSRPageActuator::IsCheckRemoveForSet( void )
{
	return ( BST_CHECKED == m_CheckRemoveForSet.GetCheck() ? true : false );
}

bool CDlgSRPageActuator::IsCheckFailSafeFct( void )
{
	return ( BST_CHECKED == m_CheckFailSafeFct.GetCheck() ? true : false );
}

void CDlgSRPageActuator::Init( bool bRstToDefault )
{
	_FillComboPowerSupply();
	_FillComboInputSignal();
	_FillComboDRPFct();
}

bool CDlgSRPageActuator::OnBnClickedBtnStart( std::vector<LPARAM> *pvecParams )
{
	if( true == IsRadioAddChecked() )
	{
		pvecParams->push_back( (LPARAM)( &m_ComboPowerSupply.GetCBCurSelIDPtr() ) );
		pvecParams->push_back( (LPARAM)( &m_ComboInputSignal.GetCBCurSelIDPtr() ) );
		pvecParams->push_back( (LPARAM)( ( BST_CHECKED == m_CheckFailSafeFct.GetCheck() ) ? true : false ) );
        pvecParams->push_back( (LPARAM)( m_ComboDRPFct.GetItemData( m_ComboDRPFct.GetCurSel() ) ) );
		pvecParams->push_back( (LPARAM)( ( BST_CHECKED == m_CheckDowngrade.GetCheck() ) ? true : false ) );
		pvecParams->push_back( (LPARAM)( ( BST_CHECKED == m_CheckForceSet.GetCheck() ) ? true : false ) );
	}

	CDlgSearchAndReplaceActuator Dlg( this );
	Dlg.SetWorkingMode( ( true == IsRadioRemoveChecked() ) ? CDlgSearchAndReplaceActuator::WorkingMode::WM_Remove : CDlgSearchAndReplaceActuator::WorkingMode::WM_Add );
	
	if( false == Dlg.SetParams( pvecParams ) )
	{
		return false;
	}

	CRect rect;
	GetWindowRect( &rect );
	Dlg.DoModal();
	return true;
}

BEGIN_MESSAGE_MAP( CDlgSRPageActuator, CDlgCtrlPropPage )
	ON_BN_CLICKED( IDC_RADIOREMOVEACTUATOR, OnRadioClickedRemoveActuator )
	ON_BN_CLICKED( IDC_RADIOADDACTUATOR, OnRadioClickedAddActuator )
	ON_BN_CLICKED( IDC_CHECKREMOVEFORSET, OnBnClickedRemoveForSet )
	ON_BN_CLICKED( IDC_CHECKFAILSAFE, OnBnClickedFailSafe )
	ON_CBN_SELCHANGE( IDC_COMBOPOWERSUPPLY, OnCbnSelChangePowerSupply )
	ON_CBN_SELCHANGE( IDC_COMBOINPUTSIGNAL, OnCbnSelChangeInputSignal )
	ON_MESSAGE( WM_USER_REPLACEPOPUPFINDNEXT,  OnReplacePopupHM )
END_MESSAGE_MAP()

void CDlgSRPageActuator::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECKREMOVEFORSET, m_CheckRemoveForSet );
	DDX_Control( pDX, IDC_COMBOPOWERSUPPLY, m_ComboPowerSupply );
	DDX_Control( pDX, IDC_COMBOINPUTSIGNAL, m_ComboInputSignal );
	DDX_Control( pDX, IDC_COMBODRP, m_ComboDRPFct );
	DDX_Control( pDX, IDC_CHECKDOWNGRADE, m_CheckDowngrade );
	DDX_Control( pDX, IDC_CHECKFORCESET, m_CheckForceSet );
	DDX_Control( pDX, IDC_CHECKFAILSAFE, m_CheckFailSafeFct );
}

BOOL CDlgSRPageActuator::OnInitDialog()
{
	CDlgCtrlPropPage::OnInitDialog();

	CString str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_ACTRWHATTODO );
	GetDlgItem( IDC_STATIC_WHATTODO )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_ACTUATORREMOVE );
	GetDlgItem( IDC_RADIOREMOVEACTUATOR )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_ACTUATORADD );
	GetDlgItem( IDC_RADIOADDACTUATOR )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_ACTROPTIONS );
	GetDlgItem( IDC_STATIC_OPTIONS )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_ACTRPOWSUP );
	GetDlgItem( IDC_STATICPOWERSUPPLY )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_ACTRINPSIG );
	GetDlgItem( IDC_STATICINPUTSIGNAL )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_DRP );
	GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_ACTRFAILSAFE );
	GetDlgItem( IDC_CHECKFAILSAFE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_ACTRREMOVEFORSET );
	GetDlgItem( IDC_CHECKREMOVEFORSET )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_ACTRDOWNFCT );
	GetDlgItem( IDC_CHECKDOWNGRADE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_ACTRFORCESET );
	GetDlgItem( IDC_CHECKFORCESET )->SetWindowText( str );
	
	CPrjParams *pHMGenParam = TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );

	m_strPowerSupply = pHMGenParam->GetPrjParamID( CPrjParams::ActuatorPowerSupplyID );
	m_strInputSignal = pHMGenParam->GetPrjParamID( CPrjParams::ActuatorInputSignalID );
	m_bFailSafeFct = ( 1 == pHMGenParam->GetActuatorFailSafeFct() ) ? true : false;
	m_eDefaultReturnPosFct = pHMGenParam->GetActuatorDRPFct();

	( (CButton*)GetDlgItem( IDC_RADIOREMOVEACTUATOR ) )->SetCheck( BST_CHECKED );
	m_CheckRemoveForSet.SetCheck( BST_UNCHECKED );
	m_CheckDowngrade.SetCheck( BST_UNCHECKED );
	m_CheckForceSet.SetCheck( BST_UNCHECKED );
	m_CheckFailSafeFct.SetCheck( ( false == m_bFailSafeFct ) ? BST_UNCHECKED : BST_CHECKED );
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
	m_ComboInputSignal.SetCurSelWithID( m_strInputSignal );
	m_ComboPowerSupply.SetCurSelWithID( m_strPowerSupply );

	OnRadioClickedRemoveActuator();
	
	return TRUE;
}

void CDlgSRPageActuator::OnRadioClickedRemoveActuator()
{
	m_CheckRemoveForSet.ShowWindow( SW_SHOW );
	GetDlgItem( IDC_STATICPOWERSUPPLY )->ShowWindow( SW_HIDE );
	GetDlgItem( IDC_STATICINPUTSIGNAL )->ShowWindow( SW_HIDE );
	GetDlgItem( IDC_STATICDRP )->ShowWindow( SW_HIDE );
	m_ComboPowerSupply.ShowWindow( SW_HIDE );
	m_ComboInputSignal.ShowWindow( SW_HIDE );
	m_ComboDRPFct.ShowWindow( SW_HIDE );
	m_CheckDowngrade.ShowWindow( SW_HIDE );
	m_CheckForceSet.ShowWindow( SW_HIDE );
	m_CheckFailSafeFct.ShowWindow( SW_HIDE );

	if( NULL != m_pNotificationHandler )
		m_pNotificationHandler->SRPageActuator_OnRadioRemoveClicked();
}

void CDlgSRPageActuator::OnRadioClickedAddActuator()
{
	m_CheckRemoveForSet.ShowWindow( SW_HIDE );
	GetDlgItem( IDC_STATICPOWERSUPPLY )->ShowWindow( SW_SHOW );
	GetDlgItem( IDC_STATICINPUTSIGNAL )->ShowWindow( SW_SHOW );
	GetDlgItem( IDC_STATICDRP )->ShowWindow( SW_SHOW );
	m_ComboPowerSupply.ShowWindow( SW_SHOW );
	m_ComboInputSignal.ShowWindow( SW_SHOW );
	m_ComboDRPFct.ShowWindow( SW_SHOW );
	m_CheckDowngrade.ShowWindow( SW_SHOW );
	m_CheckForceSet.ShowWindow( SW_SHOW );
	m_CheckFailSafeFct.ShowWindow( SW_SHOW );

	if( NULL != m_pNotificationHandler )
		m_pNotificationHandler->SRPageActuator_OnRadioAddClicked();
}

void CDlgSRPageActuator::OnBnClickedRemoveForSet()
{
	if( NULL != m_pNotificationHandler )
		m_pNotificationHandler->SRPageActuator_OnCheckRemoveForSet();
}

void CDlgSRPageActuator::OnBnClickedFailSafe()
{
	m_bFailSafeFct = ( BST_CHECKED == m_CheckFailSafeFct.GetCheck() ) ? true : false;
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
	_FillComboDRPFct();
}

void CDlgSRPageActuator::OnCbnSelChangePowerSupply()
{
	m_strPowerSupply = m_ComboPowerSupply.GetCBCurSelIDPtr().ID;
	_FillComboInputSignal();
	OnCbnSelChangeInputSignal();
}

void CDlgSRPageActuator::OnCbnSelChangeInputSignal()
{
	m_strInputSignal = m_ComboInputSignal.GetCBCurSelIDPtr().ID;
	_SetCheckFailSafe();
	OnBnClickedFailSafe();
	
}

LRESULT CDlgSRPageActuator::OnReplacePopupHM( WPARAM wParam, LPARAM lParam )
{
	if( NULL != m_pNotificationHandler )
		m_pNotificationHandler->SRPageActuator_OnReplacePopupHM( (CDS_HydroMod *)wParam );
	return TRUE;
}

void CDlgSRPageActuator::_FillComboPowerSupply()
{
	CRank List( false );
	int iCount = 0;
	CTable *pActuatorTable = (CTable*)( TASApp.GetpTADB()->Get( _T("ACTUATOR_TAB") ).MP );
	ASSERT( NULL != pActuatorTable );
	
	CTADatabase::FilterSelection eFilterSelection = ( CDlgSearchReplace::RadioState::RS_HMCalc == (CDlgSearchReplace::RadioState)m_iHMCalcOrIndSel ) 
			? CTADatabase::FilterSelection::ForHMCalc : CTADatabase::FilterSelection::ForIndAndBatchSel;

	iCount = TASApp.GetpTADB()->GetActuatorPowerSupplyList( &List, pActuatorTable, CDB_ControlProperties::CvCtrlType::eCvNU, eFilterSelection );
	
	if( iCount > 0 )
	{
		List.Transfer( &m_ComboPowerSupply );
		
		if( iCount > 1 )
		{
			CString str = TASApp.LoadLocalizedString( IDS_COMBOTEXT_ALL_POWERSUPPLIES );
			m_ComboPowerSupply.InsertString( 0, str );
            m_ComboPowerSupply.SetItemData( 0, 0 );
		}
	}
	
	int iSelPos = max( m_ComboPowerSupply.FindCBIDPtr( m_strPowerSupply ), 0 );
	m_ComboPowerSupply.SetCurSel( iSelPos );

	if( iCount < 2 )
	{
		m_ComboPowerSupply.EnableWindow( FALSE );
	}
	else
	{
		m_ComboPowerSupply.EnableWindow( TRUE );
	}
}

void CDlgSRPageActuator::_FillComboInputSignal()
{
	CRank List( false );
	int iCount = 0;
	CTable *pActuatorTable = (CTable*)( TASApp.GetpTADB()->Get( _T("ACTUATOR_TAB") ).MP );
	ASSERT( NULL != pActuatorTable );

	CTADatabase::FilterSelection eFilterSelection = ( CDlgSearchReplace::RadioState::RS_HMCalc == (CDlgSearchReplace::RadioState)m_iHMCalcOrIndSel ) 
			? CTADatabase::FilterSelection::ForHMCalc : CTADatabase::FilterSelection::ForIndAndBatchSel;

	iCount = TASApp.GetpTADB()->GetActuatorInputSignalList( &List, pActuatorTable, CDB_ControlProperties::CvCtrlType::eCvNU, (LPCTSTR)m_strPowerSupply, eFilterSelection );
	
	if( iCount > 0 )
	{
		List.Transfer( &m_ComboInputSignal );
	
		if( iCount > 1 )
		{
			CString str = TASApp.LoadLocalizedString( IDS_COMBOTEXT_ALL_INPUTSIGNALS );
			m_ComboInputSignal.InsertString( 0, str );
			m_ComboInputSignal.SetItemData( 0, 0 );
		}
	}
	
	int iSelPos = max( m_ComboInputSignal.FindCBIDPtr( m_strInputSignal ), 0 );
	m_ComboInputSignal.SetCurSel( iSelPos );

	if( iCount < 2 )
	{
		m_ComboInputSignal.EnableWindow( FALSE );
	}
	else
	{
		m_ComboInputSignal.EnableWindow( TRUE );
	}
}

void CDlgSRPageActuator::_SetCheckFailSafe()
{
	CRank List;
	int iResult = 0;
	CTable *pActuatorTable = (CTable*)( TASApp.GetpTADB()->Get( _T("ACTUATOR_TAB") ).MP );
	ASSERT( NULL != pActuatorTable );

	CTADatabase::FilterSelection eFilterSelection = ( CDlgSearchReplace::RadioState::RS_HMCalc == ( CDlgSearchReplace::RadioState )m_iHMCalcOrIndSel )
		? CTADatabase::FilterSelection::ForHMCalc : CTADatabase::FilterSelection::ForIndAndBatchSel;

	iResult = TASApp.GetpTADB()->GetActuatorFailSafeValues( pActuatorTable, CDB_ControlProperties::CvCtrlType::eCvNU,
		(LPCTSTR)m_strPowerSupply, (LPCTSTR)m_strInputSignal, eFilterSelection );

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
}

void CDlgSRPageActuator::_FillComboDRPFct()
{
	CRank List;
	int iCount = 0;
	CTable *pActuatorTable = (CTable*)( TASApp.GetpTADB()->Get( _T("ACTUATOR_TAB") ).MP );
	ASSERT( NULL != pActuatorTable );

	CTADatabase::FilterSelection eFilterSelection = ( CDlgSearchReplace::RadioState::RS_HMCalc == (CDlgSearchReplace::RadioState)m_iHMCalcOrIndSel ) 
			? CTADatabase::FilterSelection::ForHMCalc : CTADatabase::FilterSelection::ForIndAndBatchSel;

	iCount = TASApp.GetpTADB()->GetActuatorDRPList( &List, pActuatorTable, CDB_ControlProperties::CvCtrlType::eCvNU, NULL, 
		(LPCTSTR)m_strPowerSupply, (LPCTSTR)m_strInputSignal, (true == m_bFailSafeFct)?1:0, eFilterSelection );
	
	if( iCount > 0 )
	{
		List.Transfer( &m_ComboDRPFct );
		
		if( iCount > 1 )
		{
			CString str = TASApp.LoadLocalizedString( IDS_COMBOTEXT_ALL_DRP );
			m_ComboDRPFct.InsertString( 0, str );
			m_ComboDRPFct.SetItemData( 0, CDB_ControlValve::DRPFunction::drpfAll );
		}
	}
	
	int iSelPos = 0;
	if( iCount > 1 && m_eDefaultReturnPosFct >= CDB_ControlValve::DRPFunction::drpfNone && m_eDefaultReturnPosFct < CDB_ControlValve::DRPFunction::drpfLast )
	{
		// If we have only one 1 item, this one is set at the position 0.
		// If we have more than one 1 item, "** All fail safe **" has been inserted at the position 0. Thus first valid item begins at 1.
		int i = ( 1 == m_ComboDRPFct.GetCount() ) ? 0 : 1;
		
		for( ; i < m_ComboDRPFct.GetCount(); i++ )
		{
            if( m_eDefaultReturnPosFct == (CDB_ControlValve::DRPFunction)m_ComboDRPFct.GetItemData( i ) )
			{
				iSelPos = i;
				break;
			}
		}
	}

	m_ComboDRPFct.SetCurSel( iSelPos );
	
	if( iCount < 2 )
	{
		m_ComboDRPFct.EnableWindow( FALSE );
	}
	else
	{
		m_ComboDRPFct.EnableWindow( TRUE );
	}
}
