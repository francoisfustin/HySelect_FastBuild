#include "stdafx.h"


#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "DlgWizPMRightViewInputHeatPMType.h"
#include "DlgLeftTabSelManager.h"
#include "RViewWizardSelBase.h"
#include "RViewWizardSelPM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewInput_HeatingPMType::CDlgWizardPM_RightViewInput_HeatingPMType( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, HeatingPressurisationMaintenanceType, IDD, pclParent )
{
}

void CDlgWizardPM_RightViewInput_HeatingPMType::ApplyPMInputUserUpdated( bool bWaterCharUpdated, bool bShowErrorMsg )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	bool bAtLeastOneChange = false;

	if( 0 != CString( m_ComboPMType.GetCBCurSelIDPtr().ID ).Compare( pclPMInputUser->GetPressureMaintenanceTypeID() ) )
	{
		m_ComboPMType.SetCurSelWithID( pclPMInputUser->GetPressureMaintenanceTypeID() );
		bAtLeastOneChange = true;
	}

	if( 0 != CString( m_ComboWaterMakeUpType.GetCBCurSelIDPtr().ID ).Compare( pclPMInputUser->GetWaterMakeUpTypeID() ) )
	{
		m_ComboWaterMakeUpType.SetCurSelWithID( pclPMInputUser->GetWaterMakeUpTypeID() );
		bAtLeastOneChange = true;
	}

	// Apply all these following values before calling 'OnCbnSelChangeWaterMakeUpType'.

	if( m_ExtEditWaterMakeUpNetworkPN.GetCurrentValSI() != pclPMInputUser->GetWaterMakeUpNetworkPN() )
	{
		m_dWaterMakeUpNetworkPNSaved = pclPMInputUser->GetWaterMakeUpNetworkPN();
		m_ExtEditWaterMakeUpNetworkPN.SetCurrentValSI( m_dWaterMakeUpNetworkPNSaved );
		m_ExtEditWaterMakeUpNetworkPN.Update();
		bAtLeastOneChange = true;
	}

	if( m_ExtEditWaterHardness.GetCurrentValSI() != pclPMInputUser->GetWaterMakeUpWaterHardness() )
	{
		m_ExtEditWaterHardness.SetCurrentValSI( pclPMInputUser->GetWaterMakeUpWaterHardness() );
		m_ExtEditWaterHardness.Update();
		bAtLeastOneChange = true;
	}

	OnCbnSelChangeWaterMakeUpType();

	if( m_clButtonDegassing.GetCheck() != pclPMInputUser->GetDegassingChecked() )
	{
		m_clButtonDegassing.SetCheck( pclPMInputUser->GetDegassingChecked() );
		bAtLeastOneChange = true;
	}

	if( true == bAtLeastOneChange )
	{
		_VerifyInputValues();
	}
}

int CDlgWizardPM_RightViewInput_HeatingPMType::IsAtLeastOneError( int &iErrorMaskNormal, int &iErrorMaskAdvanced )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( 0 );
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	int iError = 0;

	if( pclPMInputUser->GetWaterMakeUpNetworkPN() <= 0.0 )
	{
		iError |= Error_Pressure_WaterMakeUpNetworkPN;
	}

	iErrorMaskNormal = Error_Pressure_WaterMakeUpNetworkPN;
	iErrorMaskAdvanced = 0;

	return iError;
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_HeatingPMType, CDlgWizardPM_RightViewInput_Base )
	ON_CBN_SELCHANGE( IDC_COMBO_PMT, OnCbnSelChangePressureMaintenanceType )
	ON_CBN_SELCHANGE( IDC_COMBO_WMU, OnCbnSelChangeWaterMakeUpType )

	ON_EN_SETFOCUS( IDC_WMEDITNETWORKPN, OnEnSetFocusWaterMakeUpNetworkPN )
	ON_EN_SETFOCUS( IDC_EDIT_WATERHARDNESS, OnEnSetFocusWaterHardness )

	ON_EN_KILLFOCUS( IDC_WMEDITNETWORKPN, OnKillFocusWaterMakeUpNetworkPN )
	ON_EN_KILLFOCUS( IDC_EDIT_WATERHARDNESS, OnKillFocusEditWaterHardness )

	ON_BN_CLICKED( IDC_DEGASSING, OnBnClickedDegassing )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_HeatingPMType::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardPM_RightViewInput_Base::DoDataExchange( pDX );
	
	DDX_Control( pDX, IDC_COMBO_PMT, m_ComboPMType );
	DDX_Control( pDX, IDC_COMBO_WMU, m_ComboWaterMakeUpType );

	DDX_Control( pDX, IDC_STATIC_PMT, m_StaticPMType );
	DDX_Control( pDX, IDC_STATIC_WMU, m_StaticWaterMakeUpType );
	DDX_Control( pDX, IDC_STATICWMNETWORKPN, m_StaticWaterMakeUpNetworkPN );
	DDX_Control( pDX, IDC_STATIC_WATERHARDNESS, m_StaticWaterHardness );

	DDX_Control( pDX, IDC_WMEDITNETWORKPN, m_ExtEditWaterMakeUpNetworkPN );
	DDX_Control( pDX, IDC_EDIT_WATERHARDNESS, m_ExtEditWaterHardness );

	DDX_Control( pDX, IDC_STATICWMNETWORKPNUNIT, m_StaticWaterMakeUpNetworkPNUnit );
	DDX_Control( pDX, IDC_STATIC_WATERHARDUNIT, m_StaticWaterHardnessUnit );

	DDX_Control( pDX, IDC_DEGASSING, m_clButtonDegassing );
}

BOOL CDlgWizardPM_RightViewInput_HeatingPMType::OnInitDialog()
{
	CDlgWizardPM_RightViewInput_Base::OnInitDialog();

	SetMainTitle( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATPMTYPE_MAINTITLE ) );

	CString str;
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	//////////////////////////////////////////////////////////////////////////
	// COMBO

	// 'true' to exclude 'None'.
	FillComboPressMaintType( &m_ComboPMType, _T(""), true );
	FillComboWaterMakeUpType( &m_ComboWaterMakeUpType, _T("") );

	//////////////////////////////////////////////////////////////////////////
	// EDITTEXT

	// Values will be filled when calling 'OnCbnSelChangeWaterMakeUpType' at the end of this method.
	m_ExtEditWaterMakeUpNetworkPN.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditWaterMakeUpNetworkPN, _U_PRESSURE );

	m_ExtEditWaterHardness.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditWaterHardness, _U_WATERHARDNESS );


	//////////////////////////////////////////////////////////////////////////
	// BUTTON

	// Degassing
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTPMTYPE_DEGASSING );
	m_clButtonDegassing.SetWindowText( str );
	m_clButtonDegassing.SetCheck( BST_UNCHECKED );
	m_clButtonDegassing.SetBackColor( RGB( 255, 255, 255 ) );

	//////////////////////////////////////////////////////////////////////////
	// STATIC

	// Pressure maintenance type:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTPMTYPE_PMT );
	m_StaticPMType.SetTextAndToolTip( str );

	// Water make-up type:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTPMTYPE_WMU );
	m_StaticWaterMakeUpType.SetTextAndToolTip( str );

	// Static pressure of water network:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTPMTYPE_WATERMAKEUPPN );
	m_StaticWaterMakeUpNetworkPN.SetTextAndToolTip( str );

	// Static pressure of water network: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ), name );
	SetDlgItemText( IDC_STATICWMNETWORKPNUNIT, name );

	// Water hardness:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTPMTYPE_WATERMAKEUPWATERHARDNESS );
	m_StaticWaterHardness.SetTextAndToolTip( str );

	// Water hardness: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_WATERHARDNESS ), name );
	SetDlgItemText( IDC_STATIC_WATERHARDUNIT, name );

	m_StaticPMType.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticWaterMakeUpType.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticWaterMakeUpNetworkPN.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticWaterHardness.SetBackColor( RGB( 255, 255, 255 ) );

	m_ExtEditWaterMakeUpNetworkPN.ActiveSpecialValidation( true, this );
	m_ExtEditWaterHardness.ActiveSpecialValidation( true, this );

	m_StaticWaterMakeUpNetworkPNUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticWaterHardnessUnit.SetBackColor( RGB( 255, 255, 255 ) );

	ApplyPMInputUserUpdated();

	return TRUE;
}

void CDlgWizardPM_RightViewInput_HeatingPMType::OnCbnSelChangePressureMaintenanceType()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	pclPMInputUser->SetPressureMaintenanceTypeID( m_ComboPMType.GetCBCurSelIDPtr().ID );
    // HYS-1537 : Update PM preferences
	if( true == pDlgLeftTabSelManager->GetPMSelectionHelper()->VerifyPMWQPrefs( m_pclWizardSelPMParams ) )
	{
		pMainFrame->UpdatePMWQSelectPrefsRibbon();
	}
}


void CDlgWizardPM_RightViewInput_HeatingPMType::OnCbnSelChangeWaterMakeUpType()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	pclPMInputUser->SetWaterMakeUpTypeID( m_ComboWaterMakeUpType.GetCBCurSelIDPtr().ID );

	bool bWaterMakeUpAvailable = ( 0 != pclPMInputUser->GetWaterMakeUpTypeID().Compare( _T("WMUP_TYPE_NONE") ) ) ? true : false;

	if( false == bWaterMakeUpAvailable )
	{
		m_StaticWaterMakeUpNetworkPN.EnableWindow( FALSE );
		m_ExtEditWaterMakeUpNetworkPN.EnableWindow( FALSE );
		m_ExtEditWaterMakeUpNetworkPN.SetWindowTextW( _T("") );
		m_StaticWaterMakeUpNetworkPNUnit.EnableWindow( FALSE );

		m_StaticWaterHardness.EnableWindow( FALSE );
		m_ExtEditWaterHardness.EnableWindow( FALSE );
		m_ExtEditWaterHardness.SetWindowTextW( _T("") );
		m_StaticWaterHardnessUnit.EnableWindow( FALSE );
	}
	else
	{
		m_StaticWaterMakeUpNetworkPN.EnableWindow( TRUE );
		m_ExtEditWaterMakeUpNetworkPN.EnableWindow( TRUE );
		m_ExtEditWaterMakeUpNetworkPN.SetWindowTextW( _T("") );
		m_ExtEditWaterMakeUpNetworkPN.SetCurrentValSI( pclPMInputUser->GetWaterMakeUpNetworkPN() );
		m_ExtEditWaterMakeUpNetworkPN.Update();
		m_StaticWaterMakeUpNetworkPNUnit.EnableWindow( TRUE );

		m_StaticWaterHardness.EnableWindow( TRUE );
		m_ExtEditWaterHardness.EnableWindow( TRUE );
		m_ExtEditWaterHardness.SetWindowTextW( _T("") );
		m_ExtEditWaterHardness.SetCurrentValSI( pclPMInputUser->GetWaterMakeUpWaterHardness() );
		m_ExtEditWaterHardness.Update();
		m_StaticWaterHardnessUnit.EnableWindow( TRUE );
	}
	// HYS-1537 : Update PM preferences
	if( true == pDlgLeftTabSelManager->GetPMSelectionHelper()->VerifyPMWQPrefs( m_pclWizardSelPMParams ) )
	{
		pMainFrame->UpdatePMWQSelectPrefsRibbon();
	}
}

void CDlgWizardPM_RightViewInput_HeatingPMType::OnEnSetFocusWaterMakeUpNetworkPN()
{
	m_ExtEditWaterMakeUpNetworkPN.SetSel( 0, -1 );
}

void CDlgWizardPM_RightViewInput_HeatingPMType::OnEnSetFocusWaterHardness()
{
	m_ExtEditWaterHardness.SetSel( 0, -1 );
}

void CDlgWizardPM_RightViewInput_HeatingPMType::OnKillFocusWaterMakeUpNetworkPN()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Do nothing if no changed.
	if( m_dWaterMakeUpNetworkPNSaved == m_ExtEditWaterMakeUpNetworkPN.GetCurrentValSI() )
	{
		return;
	}

	int iErrorCode = IsWaterMakeUpNetworkPNOK( &m_ExtEditWaterMakeUpNetworkPN );

	// HYS-1350: Do not save water make-up network PN if error.
	if( Error_Pressure_WaterMakeUpNetworkPN != ( Error_Pressure_WaterMakeUpNetworkPN & iErrorCode ) )
	{
		m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpNetworkPN( m_ExtEditWaterMakeUpNetworkPN.GetCurrentValSI() );
	}

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Pressure_WaterMakeUpNetworkPN );
	}
}

void CDlgWizardPM_RightViewInput_HeatingPMType::OnKillFocusEditWaterHardness()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	if( false == m_ExtEditWaterHardness.IsDrawBorderActivated() )
	{
		m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpWaterHardness( m_ExtEditWaterHardness.GetCurrentValSI() );
	}
}

void CDlgWizardPM_RightViewInput_HeatingPMType::OnBnClickedDegassing()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetDegassingChecked( m_clButtonDegassing.GetCheck() );
	// HYS-1537 : Update PM preferences
	if( true == pDlgLeftTabSelManager->GetPMSelectionHelper()->VerifyPMWQPrefs( m_pclWizardSelPMParams ) )
	{
		pMainFrame->UpdatePMWQSelectPrefsRibbon();
	}
}

LRESULT CDlgWizardPM_RightViewInput_HeatingPMType::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgWizardPM_RightViewInput_Base::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	m_StaticWaterMakeUpNetworkPNUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	InitNumericalEdit( &m_ExtEditWaterMakeUpNetworkPN, _U_PRESSURE );

	m_StaticWaterHardnessUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_WATERHARDNESS ).c_str() );
	InitNumericalEdit( &m_ExtEditWaterHardness, _U_WATERHARDNESS );

	return 0;
}

void CDlgWizardPM_RightViewInput_HeatingPMType::GetExtEditList( CArray<CExtNumEdit*> &arExtEditList )
{
	arExtEditList.Add( &m_ExtEditWaterMakeUpNetworkPN );
}

void CDlgWizardPM_RightViewInput_HeatingPMType::_VerifyInputValues()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	if( true == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->IsWaterMakeUpExist() )
	{
		IsWaterMakeUpNetworkPNOK( &m_ExtEditWaterMakeUpNetworkPN );
	}
}
