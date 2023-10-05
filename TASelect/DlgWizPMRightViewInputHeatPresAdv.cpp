#include "stdafx.h"


#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "DlgWizPMRightViewInputHeatPresAdv.h"
#include "RViewWizardSelBase.h"
#include "RViewWizardSelPM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewInput_HeatingPressureAdvance::CDlgWizardPM_RightViewInput_HeatingPressureAdvance( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, HeatingPressureAdvance, IDD, pclParent )
{
	m_bGoToNormalModeMsgSent = false;
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::ApplyPMInputUserUpdated( bool bWaterCharUpdated, bool bShowErrorMsg )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	bool bAtLeastOneChange = false;

	if( m_ExtEditStaticHeight.GetCurrentValSI() != pclPMInputUser->GetStaticHeight() )
	{
		m_dStaticHeightSaved = pclPMInputUser->GetStaticHeight();
		m_ExtEditStaticHeight.SetCurrentValSI( m_dStaticHeightSaved );
		m_ExtEditStaticHeight.Update();
		bAtLeastOneChange = true;
	}

	if( m_BtnCheckPz.GetCheck() != pclPMInputUser->GetPzChecked() )
	{
		m_BtnCheckPz.SetCheck( pclPMInputUser->GetPzChecked() );
		bAtLeastOneChange = true;
	}

	if( m_ExtEditSafetyVRP.GetCurrentValSI() != pclPMInputUser->GetSafetyValveResponsePressure() )
	{
		m_dSafetyVRPSaved = pclPMInputUser->GetSafetyValveResponsePressure();
		m_ExtEditSafetyVRP.SetCurrentValSI( m_dSafetyVRPSaved );
		m_ExtEditSafetyVRP.Update();
		bAtLeastOneChange = true;
	}

	if( GetPressurOn( &m_ComboPressOn ) != pclPMInputUser->GetPressOn() )
	{
		SetPressurOn( &m_ComboPressOn, pclPMInputUser->GetPressOn() );
		bAtLeastOneChange = true;
	}
	
	// Apply pump head before calling 'OnCbnSelChangePressOn'.

	if( m_ExtEditPumpHead.GetCurrentValSI() != pclPMInputUser->GetPumpHead() )
	{
		m_dPumpHeadSaved = pclPMInputUser->GetPumpHead();
		m_ExtEditPumpHead.SetCurrentValSI( m_dPumpHeadSaved );
		m_ExtEditPumpHead.Update();
		bAtLeastOneChange = true;
	}

	OnCbnSelChangePressOn();

	// In this method we verify pz and PSV.
	OnBnClickedCheckPz();

	if( true == bAtLeastOneChange )
	{
		_VerifyInputValues();
	}
}

int CDlgWizardPM_RightViewInput_HeatingPressureAdvance::IsAtLeastOneError( int &iErrorMaskNormal, int &iErrorMaskAdvanced )
{
	// 'IsAtLeastOneError' is called without completely creating the dialog. We create only a new instance and call
	// the method (See in the '_CreateRightViewInputDialog' call in the 'CRViewWizardSelPM::IsAtLeastOneError 'method).
	// This is why here we can't absolutely not use variable as the 'CExtNumEdit' or 'CExtStatic'.

	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( 0 );
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	// Verify first 'pz'.
	double dPz = pclPMInputUser->GetPz();
	double dP0 = pclPMInputUser->GetMinimumPressure( false );
	int iError = 0;

	if( BST_CHECKED == pclPMInputUser->GetPzChecked() && dPz < dP0 )
	{
		iError |= Error_Pressure_Pz;
	}

	// Verify now 'PSV' is no error with 'pz'.
	double dPSVLimit = 0.0;

	if( false == pclPMInputUser->CheckPSV( &dPSVLimit ) )
	{
		iError |= Error_Pressure_SafetyVRP;
	}
	
	// Verify now pump head if no error with 'PSV'.
	if( PressurON::poPumpDischarge == pclPMInputUser->GetPressOn() && pclPMInputUser->GetPumpHead() <= 0.0 )
	{
		iError |= Error_Pressure_PumpHead;
	}

	iErrorMaskNormal = Error_Pressure_Pz | Error_Pressure_SafetyVRP;
	iErrorMaskAdvanced = Error_Pressure_Pz | Error_Pressure_SafetyVRP | Error_Pressure_PumpHead;

	return iError;
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_HeatingPressureAdvance, CDlgWizardPM_RightViewInput_Base )
	ON_WM_LBUTTONDOWN()

	ON_EN_SETFOCUS( IDC_EDIT_HST, OnEnSetFocusStaticHeight )
	ON_EN_SETFOCUS( IDC_EDIT_PZ2, OnEnSetFocusPz )
	ON_EN_SETFOCUS( IDC_EDIT_SVRP, OnEnSetFocusSafetyVRP )
	ON_EN_SETFOCUS( IDC_EDIT_PUMPHEAD, OnEnSetFocusPumpHead )

	ON_EN_CHANGE( IDC_EDIT_HST, OnEnChangeStaticHeight )

	ON_EN_KILLFOCUS( IDC_EDIT_HST, OnKillFocusStaticHeight )
	ON_EN_KILLFOCUS( IDC_EDIT_PZ2, OnKillFocusPz )
	ON_EN_KILLFOCUS( IDC_EDIT_SVRP, OnKillFocusSafetyVRP )
	ON_EN_KILLFOCUS( IDC_EDIT_PUMPHEAD, OnKillFocusPumpHead )

	ON_BN_CLICKED( IDC_CHECKPZ, OnBnClickedCheckPz )
	ON_CBN_SELCHANGE( IDC_COMBO_PRESSON, OnCbnSelChangePressOn )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardPM_RightViewInput_Base::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_STATIC_HST, m_StaticStaticHeight );
	DDX_Control( pDX, IDC_STATIC_P2, m_StaticP0 );
	DDX_Control( pDX, IDC_STATIC_PZ2, m_StaticPz);
	DDX_Control( pDX, IDC_STATIC_SVRP, m_StaticSafetyVRP );
	DDX_Control( pDX, IDC_STATIC_PSVMINIMUM3, m_StaticSafetyVRPMinimum );
	DDX_Control( pDX, IDC_STATIC_PRESSON, m_StaticComboPressOn );
	DDX_Control( pDX, IDC_STATIC_PUMPHEAD, m_StaticPumpHead );

	DDX_Control( pDX, IDC_CHECKPZ, m_BtnCheckPz );

	DDX_Control( pDX, IDC_EDIT_HST, m_ExtEditStaticHeight );
	DDX_Control( pDX, IDC_EDIT_PZ2, m_ExtEditPz );
	DDX_Control( pDX, IDC_EDIT_SVRP, m_ExtEditSafetyVRP );
	DDX_Control( pDX, IDC_EDIT_PUMPHEAD, m_ExtEditPumpHead );

	DDX_Control( pDX, IDC_STATIC_HST_UNIT, m_StaticStaticHeightUnit );
	DDX_Control( pDX, IDC_STATIC_PZUNIT, m_StaticPzUnit );
	DDX_Control( pDX, IDC_STATIC_SVRP_UNIT, m_StaticSafetyVRPUnit );
	DDX_Control( pDX, IDC_STATIC_PUMPHEAD_UNIT, m_StaticPumpHeadUnit );

	DDX_Control( pDX, IDC_COMBO_PRESSON, m_ComboPressOn );
	DDX_Control( pDX, IDC_STATIC_GONORMALMODE2, m_ButtonGoNormalMode );
}

BOOL CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnInitDialog()
{
	CDlgWizardPM_RightViewInput_Base::OnInitDialog();

	SetMainTitle( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATPRES_MAINTITLE ) );

	CString str;
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// To avoid any 'OnEnChange' event to be called.
	m_bOnEnChangeEnabled = false;

	//////////////////////////////////////////////////////////////////////////
	// EDITTEXT

	m_ExtEditStaticHeight.SetCurrentValSI( -1.0 );
	InitNumericalEdit( &m_ExtEditStaticHeight, _U_LENGTH );

	m_ExtEditPz.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditPz, _U_PRESSURE );

	m_ExtEditSafetyVRP.SetCurrentValSI( -1.0 );
	InitNumericalEdit( &m_ExtEditSafetyVRP, _U_PRESSURE );

	m_ExtEditPumpHead.SetCurrentValSI( -1.0 );
	InitNumericalEdit( &m_ExtEditPumpHead, _U_DIFFPRESS );

	//////////////////////////////////////////////////////////////////////////
	// COMBO

	FillComboPressOn( &m_ComboPressOn, PressurON::poLast );

	//////////////////////////////////////////////////////////////////////////
	// STATIC

	// Static height[Hst]:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTHEATPRESS_HST );
	m_StaticStaticHeight.SetTextAndToolTip( str );

	// Static height[Hst]: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ), name );
	SetDlgItemText( IDC_STATIC_HST_UNIT, name );

	// P0:
	m_StaticP0.SetWindowTextW( _T("") );

	// pz:
	str = TASApp.LoadLocalizedString( IDS_PM_PZFULL );
	CString str2 = TASApp.LoadLocalizedString( IDS_PM_TTPZ );
	m_StaticPz.SetTextAndToolTip( str, str2 );

	// pz: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ), name );
	SetDlgItemText( IDC_STATIC_PZUNIT, name );

	// Pz min checkbox.
	m_BtnCheckPz.SetCheck( BST_UNCHECKED );

	// Don't call 'OnBnClickedCheckPz' because this method save check state in 'm_pclWizardSelPMParams'.
	m_ExtEditPz.EnableWindow( FALSE );
	m_ExtEditPz.SetCurrentValSI( 0.0 );
	m_ExtEditPz.SetEmpty( true );
	m_StaticPzUnit.EnableWindow( FALSE );

	// Safety valve response pressure:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTHEATPRESS_SVRP );
	m_StaticSafetyVRP.SetTextAndToolTip( str );

	// Safety valve response pressure: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ), name );
	SetDlgItemText( IDC_STATIC_SVRP_UNIT, name );

	// Safety valve response pressure limit:
	m_StaticSafetyVRPMinimum.SetWindowTextW( _T("") );

	// Pressurisation on:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTHEATPRESS_PRESSON );
	m_StaticComboPressOn.SetTextAndToolTip( str );

	// Pump head:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTHEATPRESS_PUMPHEAD );
	m_StaticPumpHead.SetTextAndToolTip( str );

	// Pump head: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
	SetDlgItemText( IDC_STATIC_PUMPHEAD_UNIT, name );

	m_StaticStaticHeight.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticP0.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticPz.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSafetyVRP.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSafetyVRPMinimum.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticComboPressOn.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticPumpHead.SetBackColor( RGB( 255, 255, 255 ) );

	m_ExtEditStaticHeight.ActiveSpecialValidation( true, this );
	m_ExtEditPz.ActiveSpecialValidation( true, this );
	m_ExtEditSafetyVRP.ActiveSpecialValidation( true, this );
	m_ExtEditPumpHead.ActiveSpecialValidation( true, this );

	m_StaticStaticHeightUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticPzUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSafetyVRPUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticPumpHeadUnit.SetBackColor( RGB( 255, 255, 255 ) );

	m_ButtonGoNormalMode.ShowWindow( SW_HIDE );
	m_ButtonGoNormalMode.EnableWindow( FALSE );

	m_ButtonGoNormalMode.GetWindowRect( &m_ButtonGoNormalModePos );
	ScreenToClient( &m_ButtonGoNormalModePos );

	ApplyPMInputUserUpdated();

	m_bOnEnChangeEnabled = true;

	return TRUE;
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnPaint()
{
	CDlgWizardPM_RightViewInput_Base::OnPaint();

	DrawGoAdvancedNormalModeButtton( Draw_NormalMode, m_ButtonGoNormalModePos );
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( false == m_bGoToNormalModeMsgSent )
	{
		CRect rectClient;
		GetClientRect( &rectClient );
		CRect rectGoButton( 20, m_ButtonGoNormalModePos.top, rectClient.Width() - 20, m_ButtonGoNormalModePos.top + 20 );

		if( TRUE == rectGoButton.PtInRect( (POINT)point ) )
		{
			m_bGoToNormalModeMsgSent = true;
			m_pclParent->PostMessage( WM_USER_RIGHTVIEWWIZPM_SWITCHMODE, WPARAM( HeatingPressure ), 0 );
		}
	}

	CDlgWizardPM_RightViewInput_Base::OnLButtonDown( nFlags, point );
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnEnSetFocusStaticHeight()
{
	m_ExtEditStaticHeight.SetSel( 0, -1 );
	m_dStaticHeightSaved = m_ExtEditStaticHeight.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnEnSetFocusPz()
{
	m_ExtEditPz.SetSel( 0, -1 );
	m_dPzSaved = m_ExtEditPz.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnEnSetFocusSafetyVRP()
{
	m_ExtEditSafetyVRP.SetSel( 0, -1 );
	m_dSafetyVRPSaved = m_ExtEditSafetyVRP.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnEnSetFocusPumpHead()
{
	m_ExtEditPumpHead.SetSel( 0, -1 );
	m_dPumpHeadSaved = m_ExtEditPumpHead.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnEnChangeStaticHeight()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	if( false == m_bOnEnChangeEnabled )
	{
		return;
	}

	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetStaticHeight( m_ExtEditStaticHeight.GetCurrentValSI() );
	UpdateP0();
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnKillFocusStaticHeight()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	double dStaticHeight = m_ExtEditStaticHeight.GetCurrentValSI();

	// Do nothing if no change.
	if( m_dStaticHeightSaved == dStaticHeight )
	{
		return;
	}

	// Static height has an influence on p0. Thus we need to check pz input by user.
	int iErrorCode = ( false == CheckPz( &m_BtnCheckPz, &m_ExtEditPz ) ) ? Error_Pressure_Pz : 0;

	// Static height has an influence on the PSV.
	iErrorCode |= ( false == CheckSafetyVRP( &m_ExtEditSafetyVRP, &m_StaticSafetyVRPMinimum ) ) ? Error_Pressure_SafetyVRP : 0;

	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetStaticHeight( dStaticHeight );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPz( m_ExtEditPz.GetCurrentValSI() );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSafetyValveResponsePressure( m_ExtEditSafetyVRP.GetCurrentValSI() );

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Pressure_Pz | Error_Pressure_SafetyVRP, Error_Pressure_Pz | Error_Pressure_SafetyVRP );
	}
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnKillFocusPz()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	double dPz = m_ExtEditPz.GetCurrentValSI();

	// Do nothing if no change.
	if( m_dPzSaved == dPz )
	{
		return;
	}

	int iErrorCode = 0;

	if( true == CheckPz( &m_BtnCheckPz, &m_ExtEditPz ) )
	{
		// HYS-1350: we need to update internal variable before calling 'UpdateP0' and 'CheckSafetyVRP'.
		m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPz( dPz );

		UpdateP0();

		// Pz has an influence on the PSV.
		iErrorCode = ( false == CheckSafetyVRP( &m_ExtEditSafetyVRP, &m_StaticSafetyVRPMinimum ) ) ? Error_Pressure_SafetyVRP : 0;
	}
	else
	{
		// We take minimum pressure without taking account pz to be able to show the correct p0 when there is an error.
		double dP0 = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumPressure( false );

		CString str;
		FormatString( str, IDS_DLGWIZPM_RVIEWINPUTCOOLPRESS_P0, WriteCUDouble( _U_PRESSURE, dP0, true ) );
		m_StaticP0.SetWindowText( str );
		iErrorCode = Error_Pressure_Pz;
	}

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Pressure_Pz | Error_Pressure_SafetyVRP, Error_Pressure_Pz | Error_Pressure_SafetyVRP );
	}
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnKillFocusSafetyVRP()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	double dSafetyVRP = m_ExtEditSafetyVRP.GetCurrentValSI();

	// Do nothing if no change.
	if( m_dSafetyVRPSaved == dSafetyVRP )
	{
		return;
	}

	int iErrorCode = ( false == CheckSafetyVRP( &m_ExtEditSafetyVRP, &m_StaticSafetyVRPMinimum ) ) ? Error_Pressure_SafetyVRP : 0;
	
	// Do not save safety valve response pressure if error.
	if( Error_Pressure_SafetyVRP != ( Error_Pressure_SafetyVRP & iErrorCode ) )
	{
		m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSafetyValveResponsePressure( dSafetyVRP );
	}

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Pressure_SafetyVRP, Error_Pressure_SafetyVRP );
	}
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnKillFocusPumpHead()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Do nothing if no change.
	if( m_dPumpHeadSaved == m_ExtEditPumpHead.GetCurrentValSI() )
	{
		return;
	}

	int iErrorMaskNormal = 0;
	int iErrorMaskAdvance = Error_Pressure_PumpHead;
	int iErrorCode = IsPumpHeadOK( &m_ExtEditPumpHead, &m_ComboPressOn );

	if( 0 == iErrorCode )
	{
		UpdateP0();

		// Pump head has an influence on p0. Thus we need to check pz input by user.
		iErrorCode = ( false == CheckPz( &m_BtnCheckPz, &m_ExtEditPz ) ) ? Error_Pressure_Pz : 0;

		// Pump head has an influence on the PSV.
		iErrorCode |= ( false == CheckSafetyVRP( &m_ExtEditSafetyVRP, &m_StaticSafetyVRPMinimum ) ) ? Error_Pressure_SafetyVRP : 0;

		iErrorMaskNormal = Error_Pressure_Pz | Error_Pressure_SafetyVRP;
		iErrorMaskAdvance |= iErrorMaskNormal;
	}

	// Do not save pump head if error.
	if( Error_Pressure_PumpHead != ( Error_Pressure_PumpHead & iErrorCode ) )
	{
		m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPumpHead( m_ExtEditPumpHead.GetCurrentValSI() );
	}

	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPz( m_ExtEditPz.GetCurrentValSI() );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSafetyValveResponsePressure( m_ExtEditSafetyVRP.GetCurrentValSI() );

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, iErrorMaskNormal, iErrorMaskAdvance );
	}
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnBnClickedCheckPz()
{
	// By default reset error.
	m_ExtEditSafetyVRP.ResetDrawBorder();
	ResetToolTipAndErrorBorder( &m_ExtEditSafetyVRP );

	// After reset reload default values.
	InitToolTips( NULL );

	int iCheck = m_BtnCheckPz.GetCheck();
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPzChecked( iCheck );

	m_ExtEditPz.EnableWindow( ( BST_CHECKED == iCheck ) ? TRUE : FALSE );
	m_StaticPzUnit.EnableWindow( ( BST_CHECKED == iCheck ) ? TRUE : FALSE );

	if( BST_CHECKED == iCheck )
	{
		m_ExtEditPz.SetEmpty( false );
		m_ExtEditPz.SetCurrentValSI( m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumPressure() );
		m_ExtEditPz.Update();
		InitToolTips( &m_ExtEditPz );
	}
	else
	{
		m_ExtEditPz.SetCurrentValSI( 0.0 );
		m_ExtEditPz.SetEmpty( true );
	}

	UpdateP0();

	int iErrorCode = ( false == CheckPz( &m_BtnCheckPz, &m_ExtEditPz ) ) ? Error_Pressure_Pz : 0;
	iErrorCode |= ( false == CheckSafetyVRP( &m_ExtEditSafetyVRP, &m_StaticSafetyVRPMinimum ) ) ? Error_Pressure_SafetyVRP : 0;

	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPz( m_ExtEditPz.GetCurrentValSI() );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSafetyValveResponsePressure( m_ExtEditSafetyVRP.GetCurrentValSI() );

	// Do not save system volume if error.
	if( Error_Installation_SystemVolume != ( Error_Installation_SystemVolume & iErrorCode ) )
	{
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Pressure_Pz | Error_Pressure_SafetyVRP, Error_Pressure_Pz | Error_Pressure_SafetyVRP );
	}
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnCbnSelChangePressOn()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	ResetToolTipAndErrorBorder( &m_ExtEditPumpHead );

	PressurON ePressurON = GetPressurOn( &m_ComboPressOn );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPressOn( ePressurON );

	int iErrorCode = 0;
	int iErrorMaskNormal = 0;
	int iErrorMaskAdvance = Error_Pressure_PumpHead;

	if( PressurON::poPumpSuction == ePressurON )
	{
		m_StaticPumpHead.EnableWindow( FALSE );
		m_ExtEditPumpHead.EnableWindow( FALSE );
		m_ExtEditPumpHead.SetWindowTextW( _T("") );
		m_StaticPumpHeadUnit.EnableWindow( FALSE );
	}
	else
	{
		m_StaticPumpHead.EnableWindow( TRUE );
		m_ExtEditPumpHead.EnableWindow( TRUE );
		m_ExtEditPumpHead.SetCurrentValSI( pclPMInputUser->GetPumpHead() );
		m_ExtEditPumpHead.Update();
		m_StaticPumpHeadUnit.EnableWindow( TRUE );

		iErrorCode = IsPumpHeadOK( &m_ExtEditPumpHead, &m_ComboPressOn );
	}

	if( 0 == iErrorCode )
	{
		UpdateP0();

		// Pump discharge allow to the user to input a pump head. Pump head has an influence on p0. Thus we need to check pz input by user.
		iErrorCode = ( false == CheckPz( &m_BtnCheckPz, &m_ExtEditPz ) ) ? Error_Pressure_Pz : 0;

		// And pump head has also an influence on the PSV.
		iErrorCode |= ( false == CheckSafetyVRP( &m_ExtEditSafetyVRP, &m_StaticSafetyVRPMinimum ) ) ? Error_Pressure_SafetyVRP : 0;
		iErrorMaskNormal = Error_Pressure_Pz | Error_Pressure_SafetyVRP;
		iErrorMaskAdvance |= iErrorMaskNormal;
	}

	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPumpHead( m_ExtEditPumpHead.GetCurrentValSI() );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPz( m_ExtEditPz.GetCurrentValSI() );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSafetyValveResponsePressure( m_ExtEditSafetyVRP.GetCurrentValSI() );

	// Do not save system volume if error.
	if( Error_Installation_SystemVolume != ( Error_Installation_SystemVolume & iErrorCode ) )
	{
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, iErrorMaskNormal, iErrorMaskAdvance );
	}
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::UpdateP0()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList ||
			NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	double dP0 = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumPressure();

	CString str;
	FormatString( str, IDS_DLGWIZPM_RVIEWINPUTCOOLPRESS_P0, WriteCUDouble( _U_PRESSURE, dP0, true ) );
	m_StaticP0.SetWindowText( str );
}

LRESULT CDlgWizardPM_RightViewInput_HeatingPressureAdvance::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgWizardPM_RightViewInput_Base::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	m_StaticStaticHeightUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str() );
	InitNumericalEdit( &m_ExtEditStaticHeight, _U_LENGTH );

	m_StaticPzUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	InitNumericalEdit( &m_ExtEditPz, _U_PRESSURE );

	m_StaticSafetyVRPUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	InitNumericalEdit( &m_ExtEditSafetyVRP, _U_PRESSURE );

	m_StaticPumpHeadUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_DIFFPRESS ).c_str() );
	InitNumericalEdit( &m_ExtEditPumpHead, _U_DIFFPRESS );

	UpdateP0();
	CheckPz( &m_BtnCheckPz, &m_ExtEditPz );
	CheckSafetyVRP( &m_ExtEditSafetyVRP, &m_StaticSafetyVRPMinimum );

	return 0;
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::GetExtEditList( CArray<CExtNumEdit*> &arExtEditList )
{
	arExtEditList.Add( &m_ExtEditPz );
	arExtEditList.Add( &m_ExtEditSafetyVRP );
}

void CDlgWizardPM_RightViewInput_HeatingPressureAdvance::_VerifyInputValues()
{
	CheckPz( &m_BtnCheckPz, &m_ExtEditPz );
	CheckSafetyVRP( &m_ExtEditSafetyVRP, &m_StaticSafetyVRPMinimum );
	IsPumpHeadOK( &m_ExtEditPumpHead, &m_ComboPressOn );
}
