#include "stdafx.h"


#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "DlgWizPMRightViewInputSolarPres.h"
#include "RViewWizardSelBase.h"
#include "RViewWizardSelPM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgWizardPM_RightViewInput_SolarPressure::CDlgWizardPM_RightViewInput_SolarPressure( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, SolarPressure, IDD, pclParent )
{
	m_bGoToAdvModeMsgSent = false;
}

void CDlgWizardPM_RightViewInput_SolarPressure::ApplyPMInputUserUpdated( bool bWaterCharUpdated, bool bShowErrorMsg )
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

	// HYS-1083.
	if( m_BtnCheckpSVLocation.GetCheck() != pclPMInputUser->GetSafetyValveLocationChecked() )
	{
		m_BtnCheckpSVLocation.SetCheck( pclPMInputUser->GetSafetyValveLocationChecked() );
		OnBnClickedCheckpSVLocation();
		bAtLeastOneChange = true;
	}

	if( m_ExtEditSafetyVRP.GetCurrentValSI() != pclPMInputUser->GetSafetyValveResponsePressure() )
	{
		m_dSafetyVRPSaved = pclPMInputUser->GetSafetyValveResponsePressure();
		m_ExtEditSafetyVRP.SetCurrentValSI( m_dSafetyVRPSaved );
		m_ExtEditSafetyVRP.Update();
		bAtLeastOneChange = true;
	}

	// In this method we verify pz and PSV.
	OnBnClickedCheckPz();

	if( true == bAtLeastOneChange )
	{
		_VerifyInputValues();
	}
}

int CDlgWizardPM_RightViewInput_SolarPressure::IsAtLeastOneError( int &iErrorMaskNormal, int &iErrorMaskAdvanced )
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
	int iError = 0;
	double dSafetyValveLocation = pclPMInputUser->GetSafetyValveLocation();

	// Check the safety valve location.
	if( ( dSafetyValveLocation >= 0.0 && dSafetyValveLocation > pclPMInputUser->GetStaticHeight() )
			|| ( dSafetyValveLocation < 0.0 ) )
	{
		iError |= Error_Pressure_SVLocation;
	}

	// Verify first 'pz'.
	double dPz = pclPMInputUser->GetPz();
	double dP0 = pclPMInputUser->GetMinimumPressure( false );

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

	iErrorMaskNormal = Error_Pressure_Pz | Error_Pressure_SafetyVRP | Error_Pressure_SVLocation;
	iErrorMaskAdvanced = Error_Pressure_Pz | Error_Pressure_SafetyVRP | Error_Pressure_SVLocation | Error_Pressure_PumpHead;

	return iError;
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_SolarPressure, CDlgWizardPM_RightViewInput_Base )
	ON_WM_LBUTTONDOWN()

	ON_EN_SETFOCUS( IDC_EDIT_HST, OnEnSetFocusStaticHeight )
	ON_EN_SETFOCUS( IDC_EDIT_PZ2, OnEnSetFocusPz )
	ON_EN_SETFOCUS( IDC_EDIT_SVRP, OnEnSetFocusSafetyVRP )
	ON_EN_SETFOCUS( IDC_EDIT_PSVLOCATION, OnEnSetFocuspSVLocation )

	ON_EN_CHANGE( IDC_EDIT_HST, OnEnChangeStaticHeight )

	ON_EN_KILLFOCUS( IDC_EDIT_HST, OnKillFocusStaticHeight )
	ON_EN_KILLFOCUS( IDC_EDIT_PZ2, OnKillFocusPz )
	ON_EN_KILLFOCUS( IDC_EDIT_SVRP, OnKillFocusSafetyVRP )
	ON_EN_KILLFOCUS( IDC_EDIT_PSVLOCATION, OnKillFocuspSVLocation )

	ON_BN_CLICKED( IDC_CHECKPZ, OnBnClickedCheckPz )
	ON_BN_CLICKED( IDC_CHECKPSVLOCATION, OnBnClickedCheckpSVLocation )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_SolarPressure::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardPM_RightViewInput_Base::DoDataExchange( pDX );
	
	DDX_Control( pDX, IDC_STATIC_HST, m_StaticStaticHeight );
	DDX_Control( pDX, IDC_STATIC_P0, m_StaticP0 );
	DDX_Control( pDX, IDC_STATIC_PZ2, m_StaticPz);
	DDX_Control( pDX, IDC_STATIC_SVRP, m_StaticSafetyVRP );
	DDX_Control( pDX, IDC_STATIC_PSVMINIMUM, m_StaticSafetyVRPMinimum );

	DDX_Control( pDX, IDC_CHECKPZ, m_BtnCheckPz );

	DDX_Control( pDX, IDC_EDIT_HST, m_ExtEditStaticHeight );
	DDX_Control( pDX, IDC_EDIT_PZ2, m_ExtEditPz );
	DDX_Control( pDX, IDC_EDIT_SVRP, m_ExtEditSafetyVRP );
	DDX_Control( pDX, IDC_CHECKPSVLOCATION, m_BtnCheckpSVLocation );						// HYS-1083.
	DDX_Control( pDX, IDC_EDIT_PSVLOCATION, m_EditpSVLocation );							// HYS-1083.
	DDX_Control( pDX, IDC_STATIC_PSVLOCATION, m_StaticpSVLocation );						// HYS-1083.

	DDX_Control( pDX, IDC_STATIC_HST_UNIT, m_StaticStaticHeightUnit );
	DDX_Control( pDX, IDC_STATIC_PZUNIT, m_StaticPzUnit );
	DDX_Control( pDX, IDC_STATIC_SVRP_UNIT, m_StaticSafetyVRPUnit );
	DDX_Control( pDX, IDC_STATIC_STATICPSVLOCATIONUNIT, m_StaticSVLocationUnit );

	DDX_Control( pDX, IDC_STATIC_GOADVMODE, m_ButtonGoAdvancedMode );
}

BOOL CDlgWizardPM_RightViewInput_SolarPressure::OnInitDialog()
{
	CDlgWizardPM_RightViewInput_Base::OnInitDialog();

	SetMainTitle( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWSOLARPRES_MAINTITLE ) );

	CString str;
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// To avoid any 'OnEnChange' event to be called.
	m_bOnEnChangeEnabled = false;

	//////////////////////////////////////////////////////////////////////////
	// EDITTEXT

	m_ExtEditStaticHeight.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditStaticHeight, _U_LENGTH );

	m_ExtEditPz.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditPz, _U_PRESSURE );

	m_ExtEditSafetyVRP.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditSafetyVRP, _U_PRESSURE );

	m_EditpSVLocation.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_EditpSVLocation, _U_LENGTH );

	//////////////////////////////////////////////////////////////////////////
	// STATIC

	// Static height[Hst]:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTSOLARPRESS_HST );
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
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTSOLARPRESS_SVRP );
	m_StaticSafetyVRP.SetTextAndToolTip( str );

	// Safety valve response pressure: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ), name );
	SetDlgItemText( IDC_STATIC_SVRP_UNIT, name );

	// Safety valve response pressure limit:
	m_StaticSafetyVRPMinimum.SetWindowTextW( _T("") );

	// Safety valve location
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTSOLARPRESS_PSVLOCATION );
	m_StaticpSVLocation.SetTextAndToolTip( str );
	// Safety valve location unit
	GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ), name );
	SetDlgItemText( IDC_STATIC_STATICPSVLOCATIONUNIT, name );
	// PSV location checkbox.
	m_BtnCheckpSVLocation.SetCheck( BST_UNCHECKED );

	// Don't call 'OnBnClickedCheck...' because this method save check state in 'm_pclWizardSelPMParams'.
	m_EditpSVLocation.EnableWindow( FALSE );
	m_EditpSVLocation.SetCurrentValSI( 0.0 );
	m_EditpSVLocation.SetEmpty( true );
	m_StaticSVLocationUnit.EnableWindow( FALSE );

	m_StaticStaticHeight.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticP0.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticPz.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSafetyVRP.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSafetyVRPMinimum.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticpSVLocation.SetBackColor( RGB( 255, 255, 255 ) );

	m_ExtEditStaticHeight.ActiveSpecialValidation( true, this );
	m_ExtEditPz.ActiveSpecialValidation( true, this );
	m_ExtEditSafetyVRP.ActiveSpecialValidation( true, this );
	m_EditpSVLocation.ActiveSpecialValidation( true, this );

	m_StaticStaticHeightUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticPzUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSafetyVRPUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSVLocationUnit.SetBackColor( RGB( 255, 255, 255 ) );

	m_ButtonGoAdvancedMode.ShowWindow( SW_HIDE );
	m_ButtonGoAdvancedMode.EnableWindow( FALSE );

	m_ButtonGoAdvancedMode.GetWindowRect( &m_ButtonGoAdanvedModePos );
	ScreenToClient( &m_ButtonGoAdanvedModePos );

	ApplyPMInputUserUpdated();

	m_bOnEnChangeEnabled = true;

	return TRUE;
}

void CDlgWizardPM_RightViewInput_SolarPressure::OnPaint()
{
	CDlgWizardPM_RightViewInput_Base::OnPaint();

	DrawGoAdvancedNormalModeButtton( Draw_AdvancedMode, m_ButtonGoAdanvedModePos );
}

void CDlgWizardPM_RightViewInput_SolarPressure::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( false == m_bGoToAdvModeMsgSent )
	{
		CRect rectClient;
		GetClientRect( &rectClient );
		CRect rectGoButton( 20, m_ButtonGoAdanvedModePos.top, rectClient.Width() - 20, m_ButtonGoAdanvedModePos.top + 20 );

		if( TRUE == rectGoButton.PtInRect( (POINT)point ) )
		{
			m_bGoToAdvModeMsgSent = true;
			m_pclParent->PostMessage( WM_USER_RIGHTVIEWWIZPM_SWITCHMODE, WPARAM( SolarPressureAdvance ), 0 );
		}
	}

	CDlgWizardPM_RightViewInput_Base::OnLButtonDown( nFlags, point );
}

void CDlgWizardPM_RightViewInput_SolarPressure::OnEnSetFocusStaticHeight()
{
	m_ExtEditStaticHeight.SetSel( 0, -1 );
	m_dStaticHeightSaved = m_ExtEditStaticHeight.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_SolarPressure::OnEnSetFocusPz()
{
	m_ExtEditPz.SetSel( 0, -1 );
	m_dPzSaved = m_ExtEditPz.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_SolarPressure::OnEnSetFocusSafetyVRP()
{
	m_ExtEditSafetyVRP.SetSel( 0, -1 );
	m_dSafetyVRPSaved = m_ExtEditSafetyVRP.GetCurrentValSI();
}

// HYS-1083.
void CDlgWizardPM_RightViewInput_SolarPressure::OnEnSetFocuspSVLocation()
{
	m_EditpSVLocation.SetSel( 0, -1 );
	m_dpSVLocationSaved = m_EditpSVLocation.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_SolarPressure::OnEnChangeStaticHeight()
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

void CDlgWizardPM_RightViewInput_SolarPressure::OnKillFocusStaticHeight()
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

	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetStaticHeight( dStaticHeight );

	// Static height has an influence on p0. Thus we need to check pz input by user.
	int iErrorCode = ( false == CheckPz( &m_BtnCheckPz, &m_ExtEditPz ) ) ? Error_Pressure_Pz : 0;

	// Static height has an influence on the PSV.
	iErrorCode |= ( false == CheckSafetyVRP( &m_ExtEditSafetyVRP, &m_StaticSafetyVRPMinimum ) ) ? Error_Pressure_SafetyVRP : 0;
	// Static height has an influence on SV location error.
	iErrorCode |= ( false == IspSVLocationOK( &m_EditpSVLocation ) ) ? Error_Pressure_SVLocation : 0;

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
	( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Pressure_Pz | Error_Pressure_SafetyVRP | Error_Pressure_SVLocation, Error_Pressure_Pz | Error_Pressure_SafetyVRP | Error_Pressure_SVLocation );
	}
}

void CDlgWizardPM_RightViewInput_SolarPressure::OnKillFocusPz()
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

void CDlgWizardPM_RightViewInput_SolarPressure::OnKillFocusSafetyVRP()
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

	// HYS-1350: Do not save safety valve response pressure if error.
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

// HYS-1083.
void CDlgWizardPM_RightViewInput_SolarPressure::OnKillFocuspSVLocation()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList
		|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Do nothing if no change.
	if( m_dpSVLocationSaved == m_EditpSVLocation.GetCurrentValSI() )
	{
		return;
	}

	int iErrorCode = ( false == IspSVLocationOK( &m_EditpSVLocation ) ) ? Error_Pressure_SVLocation : 0;

	if( Error_Pressure_SVLocation != ( Error_Pressure_SVLocation & iErrorCode ) )
	{
		m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSafetyValveLocation( m_EditpSVLocation.GetCurrentValSI() );
	}
	if( 0 == iErrorCode )
	{
		( (CRViewWizardSelPM*)m_pclParent )->OnInputChange( iErrorCode, Error_Pressure_SVLocation, Error_Pressure_SVLocation );
	}
}

void CDlgWizardPM_RightViewInput_SolarPressure::OnBnClickedCheckPz()
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

void CDlgWizardPM_RightViewInput_SolarPressure::UpdateP0()
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

void CDlgWizardPM_RightViewInput_SolarPressure::OnBnClickedCheckpSVLocation()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList
		|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		return;
	}

	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSafetyValveLocationChecked( BST_CHECKED == m_BtnCheckpSVLocation.GetCheck() );

	if( BST_CHECKED == m_BtnCheckpSVLocation.GetCheck() )
	{
		m_EditpSVLocation.SetEmpty( false );
		m_EditpSVLocation.EnableWindow( TRUE );
		m_StaticSVLocationUnit.EnableWindow( TRUE );
		m_EditpSVLocation.SetCurrentValSI( m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSafetyValveLocation() );
		m_EditpSVLocation.Update();
		InitToolTips( &m_EditpSVLocation );
	}
	else
	{
		m_EditpSVLocation.ResetDrawBorder();
		ResetToolTipAndErrorBorder( &m_EditpSVLocation );
		m_EditpSVLocation.SetEmpty( true );
		m_EditpSVLocation.EnableWindow( FALSE );
		m_StaticSVLocationUnit.EnableWindow( FALSE );
		m_EditpSVLocation.SetCurrentValSI( 0.0 );
		m_EditpSVLocation.Update();

		m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSafetyValveLocation( 0.0 );

		int iErrorCode = ( false == IspSVLocationOK( &m_EditpSVLocation ) ) ? Error_Pressure_SVLocation : 0;
		( (CRViewWizardSelPM*)m_pclParent )->OnInputChange( iErrorCode, Error_Pressure_SVLocation, Error_Pressure_SVLocation );
	}
}

LRESULT CDlgWizardPM_RightViewInput_SolarPressure::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgWizardPM_RightViewInput_Base::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	m_StaticStaticHeightUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str() );
	InitNumericalEdit( &m_ExtEditStaticHeight, _U_LENGTH );

	m_StaticPzUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	InitNumericalEdit( &m_ExtEditPz, _U_PRESSURE );

	m_StaticSafetyVRPUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	InitNumericalEdit( &m_ExtEditSafetyVRP, _U_PRESSURE );

	m_StaticSVLocationUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str() );
	InitNumericalEdit( &m_EditpSVLocation, _U_LENGTH );
	m_EditpSVLocation.SetEditSign( CNumString::eBoth );

	UpdateP0();
	CheckPz( &m_BtnCheckPz, &m_ExtEditPz );
	CheckSafetyVRP( &m_ExtEditSafetyVRP, &m_StaticSafetyVRPMinimum );

	return 0;
}

void CDlgWizardPM_RightViewInput_SolarPressure::GetExtEditList( CArray<CExtNumEdit*> &arExtEditList )
{
	arExtEditList.Add( &m_ExtEditPz );
	arExtEditList.Add( &m_ExtEditSafetyVRP );
	arExtEditList.Add( &m_EditpSVLocation );
}

void CDlgWizardPM_RightViewInput_SolarPressure::_VerifyInputValues()
{
	CheckPz( &m_BtnCheckPz, &m_ExtEditPz );
	CheckSafetyVRP( &m_ExtEditSafetyVRP, &m_StaticSafetyVRPMinimum );
	// Check the safety valve location.
	IspSVLocationOK( &m_EditpSVLocation );
}
