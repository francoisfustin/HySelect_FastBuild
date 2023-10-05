#include "stdafx.h"


#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "DlgWaterChar.h"
#include "DlgWizPMRightViewInputSolarTempAdv.h"
#include "DlgLeftTabSelManager.h"
#include "RViewWizardSelBase.h"
#include "RViewWizardSelPM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::CDlgWizardPM_RightViewInput_SolarTemperatureAdvance( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, SolarTemperatureAdvance, IDD, pclParent )
{
	m_bGoToNormalModeMsgSent = false;
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::ApplyPMInputUserUpdated( bool bWaterCharUpdated, bool bShowErrorMsg )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	bool bAtLeastOneChange = bWaterCharUpdated;

	if( GetNormID( &m_ComboNorm ) != pclPMInputUser->GetNormID() )
	{
		SetNormID( &m_ComboNorm, pclPMInputUser->GetNormID() );
		OnCbnSelChangeNorm();
		bAtLeastOneChange = true;
	}

	if( m_ExtEditSafetyTempLimiter.GetCurrentValSI() != pclPMInputUser->GetSafetyTempLimiter() )
	{
		m_dSafetyTempLimiterSaved = pclPMInputUser->GetSafetyTempLimiter();
		m_ExtEditSafetyTempLimiter.SetCurrentValSI( m_dSafetyTempLimiterSaved );
		m_ExtEditSafetyTempLimiter.Update();
		bAtLeastOneChange = true;
	}

	if( m_ExtEditSupplyTemp.GetCurrentValSI() != pclPMInputUser->GetSupplyTemperature() )
	{
		m_dSupplyTempSaved = pclPMInputUser->GetSupplyTemperature();
		m_ExtEditSupplyTemp.SetCurrentValSI( m_dSupplyTempSaved );
		m_ExtEditSupplyTemp.Update();
		bAtLeastOneChange = true;
	}

	if( m_ExtEditReturnTemp.GetCurrentValSI() != pclPMInputUser->GetReturnTemperature() )
	{
		m_dReturnTempSaved = pclPMInputUser->GetReturnTemperature();
		m_ExtEditReturnTemp.SetCurrentValSI( m_dReturnTempSaved );
		m_ExtEditReturnTemp.Update();
		bAtLeastOneChange = true;
	}

	if( m_ExtEditMinTemp.GetCurrentValSI() != pclPMInputUser->GetMinTemperature() )
	{
		m_dMinTempSaved = pclPMInputUser->GetMinTemperature();
		m_ExtEditMinTemp.SetCurrentValSI( m_dMinTempSaved );
		m_ExtEditMinTemp.Update();
		bAtLeastOneChange = true;
	}

	if( m_ExtEditFillTemp.GetCurrentValSI() != pclPMInputUser->GetFillTemperature() )
	{
		m_dFillTempSaved = pclPMInputUser->GetFillTemperature();
		m_ExtEditFillTemp.SetCurrentValSI( m_dFillTempSaved );
		m_ExtEditFillTemp.Update();
		bAtLeastOneChange = true;
	}

	// Verify also fluid characteristic.
	VerifyFluidCharacteristic( &m_ButtonWaterChar );

	if( true == bAtLeastOneChange )
	{
		_VerifyInputValues();
	}
}

int CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::IsAtLeastOneError( int &iErrorMaskNormal, int &iErrorMaskAdvanced )
{
	int iErrorCode = AreHeatingSolarTemperaturesOK();

	bool bEnable;
	CString strMsg;
	VerifyFluidCharacteristicHelper( bEnable, strMsg );

	if( false == bEnable )
	{
		iErrorCode |= Error_Temp_WaterChar;
	}

	iErrorMaskNormal = Error_Temp_SafetyTempLimiter;
	iErrorMaskAdvanced = Error_Temp_SafetyTempLimiter | Error_Temp_Supply | Error_Temp_Return | Error_Temp_Min | Error_Temp_Fill | Error_Temp_WaterChar;

	return iErrorCode;
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_SolarTemperatureAdvance, CDlgWizardPM_RightViewInput_Base )
	ON_WM_LBUTTONDOWN()

	ON_CBN_SELCHANGE( IDC_COMBO_NORM, OnCbnSelChangeNorm )

	ON_EN_SETFOCUS( IDC_EDIT_SFTYTEMP, OnEnSetFocusSafetyTempLimiter )
	ON_EN_SETFOCUS( IDC_EDIT_SUPPLYTEMP, OnEnSetFocusSupplyTemp )
	ON_EN_SETFOCUS( IDC_EDIT_RETURNTEMP, OnEnSetFocusReturnTemp )
	ON_EN_SETFOCUS( IDC_EDIT_MINTEMP, OnEnSetFocusMinTemp )
	ON_EN_SETFOCUS( IDC_EDIT_FILLTEMP, OnEnSetFocusFillTemp )

	ON_EN_KILLFOCUS( IDC_EDIT_SFTYTEMP, OnKillFocusEditSafetyTempLimiter )
	ON_EN_KILLFOCUS( IDC_EDIT_SUPPLYTEMP, OnKillFocusEditSupplyTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_RETURNTEMP, OnKillFocusEditReturnTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_MINTEMP, OnKillFocusEditMinTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_FILLTEMP, OnKillFocusEditFillTemp )

	ON_BN_CLICKED( IDC_BUTTON_WC, OnBnClickedWaterChar )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardPM_RightViewInput_Base::DoDataExchange( pDX );
	
	DDX_Control( pDX, IDC_STATIC_NORM, m_StaticNorm );
	DDX_Control( pDX, IDC_STATIC_SFTYTEMP, m_StaticSafetyTempLimiter );
	DDX_Control( pDX, IDC_STATIC_SUPPLYTEMP, m_StaticSupplyTemp );
	DDX_Control( pDX, IDC_STATIC_RETTEMP, m_StaticReturnTemp );
	DDX_Control( pDX, IDC_STATIC_MINTEMP, m_StaticMinTemp );
	DDX_Control( pDX, IDC_STATIC_WC, m_StaticWaterChar );
	DDX_Control( pDX, IDC_STATIC_FILLTEMP, m_StaticFillTemp );

	DDX_Control( pDX, IDC_COMBO_NORM, m_ComboNorm );
	DDX_Control( pDX, IDC_EDIT_SFTYTEMP, m_ExtEditSafetyTempLimiter );
	DDX_Control( pDX, IDC_EDIT_SUPPLYTEMP, m_ExtEditSupplyTemp );
	DDX_Control( pDX, IDC_EDIT_RETURNTEMP, m_ExtEditReturnTemp );
	DDX_Control( pDX, IDC_EDIT_MINTEMP, m_ExtEditMinTemp );
	DDX_Control( pDX, IDC_BUTTON_WC, m_ButtonWaterChar );
	DDX_Control( pDX, IDC_EDIT_FILLTEMP, m_ExtEditFillTemp );

	DDX_Control( pDX, IDC_STATIC_SFTYTEMP_UNIT, m_StaticSafetyTempLimiterUnit );
	DDX_Control( pDX, IDC_STATIC_SUPPLYTEMP_UNIT, m_StaticSupplyTempUnit );
	DDX_Control( pDX, IDC_STATIC_RETTEMP_UNIT, m_StaticReturnTempUnit );
	DDX_Control( pDX, IDC_STATIC_MINTEMP_UNIT, m_StaticMinTempUnit );
	DDX_Control( pDX, IDC_STATIC_FILLTEMP_UNIT_H, m_StaticFillTempUnit );

	DDX_Control( pDX, IDC_STATIC_GONORMALMODE, m_ButtonGoNormalMode );
}

BOOL CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnInitDialog()
{
	CDlgWizardPM_RightViewInput_Base::OnInitDialog();

	SetMainTitle( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWSOLARTEMP_MAINTITLE ) );

	CString str;
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	//////////////////////////////////////////////////////////////////////////
	// COMBO

	FillComboNorm( &m_ComboNorm, _T("") );

	//////////////////////////////////////////////////////////////////////////
	// BUTTON

	m_ButtonWaterChar.SetBitmap( ( HBITMAP )LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDB_H2OLIQUID ), IMAGE_BITMAP, 18, 18, LR_DEFAULTCOLOR ) );
	m_ButtonWaterChar.SetCheck( BST_UNCHECKED );

	//////////////////////////////////////////////////////////////////////////
	// EDITTEXT

	m_ExtEditSafetyTempLimiter.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditSafetyTempLimiter, _U_TEMPERATURE );

	m_ExtEditSupplyTemp.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditSupplyTemp, _U_TEMPERATURE );

	m_ExtEditReturnTemp.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditReturnTemp, _U_TEMPERATURE );

	m_ExtEditMinTemp.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditMinTemp, _U_TEMPERATURE );

	m_ExtEditFillTemp.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditFillTemp, _U_TEMPERATURE );

	//////////////////////////////////////////////////////////////////////////
	// STATIC

	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTHEATTEMP_NORM );
	m_StaticNorm.SetTextAndToolTip( str );

	// Safety temp. limiter [TAZ]:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTSOLARTEMP_TAZ );
	m_StaticSafetyTempLimiter.SetTextAndToolTip( str );

	// Safety temp. limiter [TAZ]: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), name );
	SetDlgItemText( IDC_STATIC_SFTYTEMP_UNIT, name );

	// Supply temperature:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTSOLARTEMP_SUPPLYTEMP );
	m_StaticSupplyTemp.SetTextAndToolTip( str );

	// Supply temperature: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), name );
	SetDlgItemText( IDC_STATIC_SUPPLYTEMP_UNIT, name );

	// Return temperature:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTSOLARTEMP_RETURNTEMP );
	m_StaticReturnTemp.SetTextAndToolTip( str );

	// Return temperature: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), name );
	SetDlgItemText( IDC_STATIC_RETTEMP_UNIT, name );

	// Min. temperature [tsmin]:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTSOLARTEMP_MINTEMP );
	m_StaticMinTemp.SetTextAndToolTip( str );

	// Min. temperature [tsmin]: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), name );
	SetDlgItemText( IDC_STATIC_MINTEMP_UNIT, name );

	// Fill temperature:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTCOOLTEMP_FILL );
	m_StaticFillTemp.SetTextAndToolTip( str );

	// Fill temperature: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), name );
	SetDlgItemText( IDC_STATIC_FILLTEMP_UNIT_H, name );

	// Fluid characteristics:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTSOLARTEMP_WC );
	m_StaticWaterChar.SetTextAndToolTip( str );

	m_StaticNorm.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSafetyTempLimiter.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSupplyTemp.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticReturnTemp.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticMinTemp.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticWaterChar.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticFillTemp.SetBackColor( RGB( 255, 255, 255 ) );

	m_ExtEditSafetyTempLimiter.ActiveSpecialValidation( true, this );
	m_ExtEditSupplyTemp.ActiveSpecialValidation( true, this );
	m_ExtEditReturnTemp.ActiveSpecialValidation( true, this );
	m_ExtEditMinTemp.ActiveSpecialValidation( true, this );
	m_ExtEditFillTemp.ActiveSpecialValidation( true, this );

	m_StaticSafetyTempLimiterUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSupplyTempUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticReturnTempUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticMinTempUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticFillTempUnit.SetBackColor( RGB( 255, 255, 255 ) );

	m_ButtonGoNormalMode.ShowWindow( SW_HIDE );
	m_ButtonGoNormalMode.EnableWindow( FALSE );

	m_ButtonGoNormalMode.GetWindowRect( &m_ButtonGoNormalModePos );
	ScreenToClient( &m_ButtonGoNormalModePos );

	ApplyPMInputUserUpdated();

	return TRUE;
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnPaint()
{
	CDlgWizardPM_RightViewInput_Base::OnPaint();

	DrawGoAdvancedNormalModeButtton( Draw_NormalMode, m_ButtonGoNormalModePos );
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( false == m_bGoToNormalModeMsgSent )
	{
		CRect rectClient;
		GetClientRect( &rectClient );
		CRect rectGoButton( 20, m_ButtonGoNormalModePos.top, rectClient.Width() - 20, m_ButtonGoNormalModePos.top + 20 );

		if( TRUE == rectGoButton.PtInRect( (POINT)point ) )
		{
			m_bGoToNormalModeMsgSent = true;
			m_pclParent->PostMessage( WM_USER_RIGHTVIEWWIZPM_SWITCHMODE, WPARAM( SolarTemperature ), 0 );
		}
	}

	CDlgWizardPM_RightViewInput_Base::OnLButtonDown( nFlags, point );
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnCbnSelChangeNorm()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetNormID( GetNormID( &m_ComboNorm ) );
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnEnSetFocusSafetyTempLimiter()
{
	m_ExtEditSafetyTempLimiter.SetSel( 0, -1 );
	m_dSafetyTempLimiterSaved = m_ExtEditSafetyTempLimiter.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnEnSetFocusSupplyTemp()
{
	m_ExtEditSupplyTemp.SetSel( 0, -1 );
	m_dSupplyTempSaved = m_ExtEditSupplyTemp.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnEnSetFocusReturnTemp()
{
	m_ExtEditReturnTemp.SetSel( 0, -1 );
	m_dReturnTempSaved = m_ExtEditReturnTemp.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnEnSetFocusMinTemp()
{
	m_ExtEditMinTemp.SetSel( 0, -1 );
	m_dMinTempSaved = m_ExtEditMinTemp.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnEnSetFocusFillTemp()
{
	m_ExtEditFillTemp.SetSel( 0, -1 );
	m_dFillTempSaved = m_ExtEditFillTemp.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnKillFocusEditSafetyTempLimiter()
{
	// Do nothing if no changed.
	if( m_dSafetyTempLimiterSaved == m_ExtEditSafetyTempLimiter.GetCurrentValSI() )
	{
		return;
	}

	// Check if errors.
	int iErrorCode = AreHeatingSolarTemperaturesOK( &m_ExtEditSafetyTempLimiter, &m_ExtEditSupplyTemp, &m_ExtEditReturnTemp, &m_ExtEditMinTemp, &m_ExtEditFillTemp );

	// Verify also fluid characteristic.
	// Remark: 'bEnable' is true if there is no error with the fluid characteristic.
	bool bEnable;
	CString strMsg;
	VerifyFluidCharacteristicHelper( bEnable, strMsg );

	if( false == bEnable )
	{
		iErrorCode |= Error_Temp_WaterChar;
	}

	// Update 'CPMInputUser' class and technical parameters if no error.
	UpdateOtherHeatingSolarTemperatures( iErrorCode, &m_ExtEditSafetyTempLimiter, &m_ExtEditSupplyTemp, &m_ExtEditReturnTemp, &m_ExtEditMinTemp, &m_ExtEditFillTemp );

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Temp_SafetyTempLimiter, 
				Error_Temp_SafetyTempLimiter | Error_Temp_Supply | Error_Temp_Return | Error_Temp_Min | Error_Temp_Fill | Error_Temp_WaterChar );
	}
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnKillFocusEditSupplyTemp()
{
	// Do nothing if no changed.
	if( m_dSupplyTempSaved == m_ExtEditSupplyTemp.GetCurrentValSI() )
	{
		return;
	}

	// Check if errors.
	int iErrorCode = AreHeatingSolarTemperaturesOK( &m_ExtEditSafetyTempLimiter, &m_ExtEditSupplyTemp, &m_ExtEditReturnTemp, &m_ExtEditMinTemp, &m_ExtEditFillTemp );

	// Verify also fluid characteristic.
	// Remark: 'bEnable' is true if there is no error with the fluid characteristic.
	bool bEnable;
	CString strMsg;
	VerifyFluidCharacteristicHelper( bEnable, strMsg );

	if( false == bEnable )
	{
		iErrorCode |= Error_Temp_WaterChar;
	}

	// Update 'CPMInputUser' class and technical parameters if no error.
	UpdateOtherHeatingSolarTemperatures( iErrorCode, &m_ExtEditSafetyTempLimiter, &m_ExtEditSupplyTemp, &m_ExtEditReturnTemp, &m_ExtEditMinTemp, &m_ExtEditFillTemp );

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Temp_SafetyTempLimiter, 
				Error_Temp_SafetyTempLimiter | Error_Temp_Supply | Error_Temp_Return | Error_Temp_Min | Error_Temp_Fill | Error_Temp_WaterChar );
	}
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnKillFocusEditReturnTemp()
{
	// Do nothing if no changed.
	if( m_dReturnTempSaved == m_ExtEditReturnTemp.GetCurrentValSI() )
	{
		return;
	}

	// Check if errors.
	int iErrorCode = AreHeatingSolarTemperaturesOK( &m_ExtEditSafetyTempLimiter, &m_ExtEditSupplyTemp, &m_ExtEditReturnTemp, &m_ExtEditMinTemp, &m_ExtEditFillTemp );

	// Verify also fluid characteristic.
	// Remark: 'bEnable' is true if there is no error with the fluid characteristic.
	bool bEnable;
	CString strMsg;
	VerifyFluidCharacteristicHelper( bEnable, strMsg );

	if( false == bEnable )
	{
		iErrorCode |= Error_Temp_WaterChar;
	}

	// Update 'CPMInputUser' class and technical parameters if no error.
	UpdateOtherHeatingSolarTemperatures( iErrorCode, &m_ExtEditSafetyTempLimiter, &m_ExtEditSupplyTemp, &m_ExtEditReturnTemp, &m_ExtEditMinTemp, &m_ExtEditFillTemp );

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Temp_SafetyTempLimiter, 
				Error_Temp_SafetyTempLimiter | Error_Temp_Supply | Error_Temp_Return | Error_Temp_Min | Error_Temp_Fill | Error_Temp_WaterChar );
	}
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnKillFocusEditMinTemp()
{
	// Do nothing if no changed.
	if( m_dMinTempSaved == m_ExtEditMinTemp.GetCurrentValSI() )
	{
		return;
	}

	// Check if errors.
	int iErrorCode = AreHeatingSolarTemperaturesOK( &m_ExtEditSafetyTempLimiter, &m_ExtEditSupplyTemp, &m_ExtEditReturnTemp, &m_ExtEditMinTemp, &m_ExtEditFillTemp );

	// Verify also fluid characteristic.
	// Remark: 'bEnable' is true if there is no error with the fluid characteristic.
	bool bEnable;
	CString strMsg;
	VerifyFluidCharacteristicHelper( bEnable, strMsg );

	if( false == bEnable )
	{
		iErrorCode |= Error_Temp_WaterChar;
	}

	// Update 'CPMInputUser' class and technical parameters if no error.
	UpdateOtherHeatingSolarTemperatures( iErrorCode, &m_ExtEditSafetyTempLimiter, &m_ExtEditSupplyTemp, &m_ExtEditReturnTemp, &m_ExtEditMinTemp, &m_ExtEditFillTemp );

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Temp_SafetyTempLimiter, 
				Error_Temp_SafetyTempLimiter | Error_Temp_Supply | Error_Temp_Return | Error_Temp_Min | Error_Temp_Fill | Error_Temp_WaterChar );
	}
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnKillFocusEditFillTemp()
{
	// Do nothing if no changed.
	if( m_dFillTempSaved == m_ExtEditFillTemp.GetCurrentValSI() )
	{
		return;
	}

	// Check if errors.
	int iErrorCode = AreHeatingSolarTemperaturesOK( &m_ExtEditSafetyTempLimiter, &m_ExtEditSupplyTemp, &m_ExtEditReturnTemp, &m_ExtEditMinTemp, &m_ExtEditFillTemp );

	// Verify also fluid characteristic.
	// Remark: 'bEnable' is true if there is no error with the fluid characteristic.
	bool bEnable;
	CString strMsg;
	VerifyFluidCharacteristicHelper( bEnable, strMsg );

	if( false == bEnable )
	{
		iErrorCode |= Error_Temp_WaterChar;
	}

	// Update 'CPMInputUser' class and technical parameters if no error.
	UpdateOtherHeatingSolarTemperatures( iErrorCode, &m_ExtEditSafetyTempLimiter, &m_ExtEditSupplyTemp, &m_ExtEditReturnTemp, &m_ExtEditMinTemp, &m_ExtEditFillTemp );

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Temp_SafetyTempLimiter, 
				Error_Temp_SafetyTempLimiter | Error_Temp_Supply | Error_Temp_Return | Error_Temp_Min | Error_Temp_Fill | Error_Temp_WaterChar );
	}
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnBnClickedWaterChar()
{
	CDlgWaterChar dlg( pDlgLeftTabSelManager->GetCurrentLeftTabDialog() );
	dlg.Display( NULL, CDlgWaterChar::DlgWaterChar_ForProductSelection );
}

LRESULT CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgWizardPM_RightViewInput_Base::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	m_StaticSafetyTempLimiterUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_ExtEditSafetyTempLimiter, _U_TEMPERATURE );

	m_StaticSupplyTempUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_ExtEditSupplyTemp, _U_TEMPERATURE );

	m_StaticReturnTempUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_ExtEditReturnTemp, _U_TEMPERATURE );

	m_StaticMinTempUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_ExtEditMinTemp, _U_TEMPERATURE );

	m_StaticFillTempUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_ExtEditFillTemp, _U_TEMPERATURE );

	return 0;
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::GetExtEditList( CArray<CExtNumEdit*> &arExtEditList )
{
	arExtEditList.Add( &m_ExtEditSafetyTempLimiter );
	arExtEditList.Add( &m_ExtEditSupplyTemp );
	arExtEditList.Add( &m_ExtEditReturnTemp );
	arExtEditList.Add( &m_ExtEditMinTemp );
	arExtEditList.Add( &m_ExtEditFillTemp );
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::InitToolTips( CWnd *pWnd )
{
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		CString TTstr;

		if( NULL == pWnd || pWnd == &m_ExtEditMinTemp )
		{
			TTstr = TASApp.LoadLocalizedString( IDS_PM_TTMINTEMP );

			double dFreezeTemp = m_pclWizardSelPMParams->m_WC.GetTfreez();
			CString str;
			FormatString( str, IDS_DLGINDSELPMPANELS_FREEZINGPOINT, WriteCUDouble( _U_TEMPERATURE, dFreezeTemp, true ) );
			TTstr = TTstr +  CString( L"\r\n" ) + str;

			m_ToolTip.AddToolWindow( &m_ExtEditMinTemp, TTstr );
		}
	}

	CDlgWizardPM_RightViewInput_Base::InitToolTips( pWnd );
}

void CDlgWizardPM_RightViewInput_SolarTemperatureAdvance::_VerifyInputValues()
{
	AreHeatingSolarTemperaturesOK( &m_ExtEditSafetyTempLimiter, &m_ExtEditSupplyTemp, &m_ExtEditReturnTemp, &m_ExtEditMinTemp, &m_ExtEditFillTemp );
}
