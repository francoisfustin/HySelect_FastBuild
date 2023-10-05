#include "stdafx.h"


#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "DlgWizPMRightViewInputHeatTemp.h"
#include "RViewWizardSelBase.h"
#include "RViewWizardSelPM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewInput_HeatingTemperature::CDlgWizardPM_RightViewInput_HeatingTemperature( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, HeatingTemperature, IDD, pclParent )
{
	m_bGoToAdvModeMsgSent = false;
}

void CDlgWizardPM_RightViewInput_HeatingTemperature::ApplyPMInputUserUpdated( bool bWaterCharUpdated, bool bShowErrorMsg )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	bool bAtLeastOneChange = bWaterCharUpdated;

	if( m_ExtEditSafetyTempLimiter.GetCurrentValSI() != pclPMInputUser->GetSafetyTempLimiter() )
	{
		m_dSafetyTempLimiterSaved = pclPMInputUser->GetSafetyTempLimiter();
		m_ExtEditSafetyTempLimiter.SetCurrentValSI( m_dSafetyTempLimiterSaved );
		m_ExtEditSafetyTempLimiter.Update();
		bAtLeastOneChange = true;
	}

	// Must be done when safety temperature limiter is filled.
	if( GetNormID( &m_ComboNorm ) != pclPMInputUser->GetNormID() )
	{
		SetNormID( &m_ComboNorm, pclPMInputUser->GetNormID() );
		OnCbnSelChangeNorm();
		bAtLeastOneChange = true;
	}

	if( true == bAtLeastOneChange )
	{
		_VerifyInputValues( bShowErrorMsg );
	}
}

int CDlgWizardPM_RightViewInput_HeatingTemperature::IsAtLeastOneError( int &iErrorMaskNormal, int &iErrorMaskAdvanced )
{
	int iErrorCode = IsHeatingSolarSafetyTempLimiterOK();

	iErrorMaskNormal = Error_Temp_SafetyTempLimiter;
	iErrorMaskAdvanced = Error_Temp_SafetyTempLimiter | Error_Temp_Supply | Error_Temp_Return | Error_Temp_Min | Error_Temp_Fill | Error_Temp_WaterChar;

	return iErrorCode;
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_HeatingTemperature, CDlgWizardPM_RightViewInput_Base )
	ON_WM_LBUTTONDOWN()
	ON_CBN_SELCHANGE( IDC_COMBO_NORM, OnCbnSelChangeNorm )
	ON_EN_SETFOCUS( IDC_EDIT_SFTYTEMP, OnEnSetFocusSafetyTempLimiter )
	ON_EN_KILLFOCUS( IDC_EDIT_SFTYTEMP, OnKillFocusEditSafetyTempLimiter )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_HeatingTemperature::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardPM_RightViewInput_Base::DoDataExchange( pDX );
	
	DDX_Control( pDX, IDC_STATIC_NORM, m_StaticNorm );
	DDX_Control( pDX, IDC_STATIC_SFTYTEMP, m_StaticSafetyTempLimiter );

	DDX_Control( pDX, IDC_COMBO_NORM, m_ComboNorm );
	DDX_Control( pDX, IDC_EDIT_SFTYTEMP, m_ExtEditSafetyTempLimiter );

	DDX_Control( pDX, IDC_STATIC_SFTYTEMP_UNIT, m_StaticSafetyTempLimiterUnit );
	DDX_Control( pDX, IDC_STATIC_GOADVMODE, m_ButtonGoAdvancedMode );
}

BOOL CDlgWizardPM_RightViewInput_HeatingTemperature::OnInitDialog()
{
	CDlgWizardPM_RightViewInput_Base::OnInitDialog();

	SetMainTitle( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATTEMP_MAINTITLE ) );

	CString str;
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	//////////////////////////////////////////////////////////////////////////
	// COMBO

	FillComboNorm( &m_ComboNorm, _T("") );

	//////////////////////////////////////////////////////////////////////////
	// EDITTEXT

	m_ExtEditSafetyTempLimiter.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditSafetyTempLimiter, _U_TEMPERATURE );
	
	//////////////////////////////////////////////////////////////////////////
	// STATIC

	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTHEATTEMP_NORM );
	m_StaticNorm.SetTextAndToolTip( str );

	// Safety temp. limiter [TAZ]:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTHEATTEMP_TAZ );
	m_StaticSafetyTempLimiter.SetTextAndToolTip( str );

	// Safety temp. limiter [TAZ]: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), name );
	SetDlgItemText( IDC_STATIC_SFTYTEMP_UNIT, name );

	m_StaticNorm.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSafetyTempLimiter.SetBackColor( RGB( 255, 255, 255 ) );

	m_ExtEditSafetyTempLimiter.ActiveSpecialValidation( true, this );
	m_StaticSafetyTempLimiterUnit.SetBackColor( RGB( 255, 255, 255 ) );

	m_ButtonGoAdvancedMode.ShowWindow( SW_HIDE );
	m_ButtonGoAdvancedMode.GetWindowRect( &m_ButtonGoAdanvedModePos );
	ScreenToClient( &m_ButtonGoAdanvedModePos );

	ApplyPMInputUserUpdated( false, false );

	return TRUE;
}

void CDlgWizardPM_RightViewInput_HeatingTemperature::OnPaint()
{
	CDlgWizardPM_RightViewInput_Base::OnPaint();

	DrawGoAdvancedNormalModeButtton( Draw_AdvancedMode, m_ButtonGoAdanvedModePos );
}

void CDlgWizardPM_RightViewInput_HeatingTemperature::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( false == m_bGoToAdvModeMsgSent )
	{
		CRect rectClient;
		GetClientRect( &rectClient );
		CRect rectGoButton( 20, m_ButtonGoAdanvedModePos.top, rectClient.Width() - 20, m_ButtonGoAdanvedModePos.top + 20 );

		if( TRUE == rectGoButton.PtInRect( (POINT)point ) )
		{
			m_bGoToAdvModeMsgSent = true;
			m_pclParent->PostMessage( WM_USER_RIGHTVIEWWIZPM_SWITCHMODE, WPARAM( HeatingTemperatureAdvance ), 0 );
		}
	}

	CDlgWizardPM_RightViewInput_Base::OnLButtonDown( nFlags, point );
}

void CDlgWizardPM_RightViewInput_HeatingTemperature::OnCbnSelChangeNorm()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetNormID( GetNormID( &m_ComboNorm ) );

	int iErrorCode = IsHeatingSolarSafetyTempLimiterOK( &m_ExtEditSafetyTempLimiter, &m_ComboNorm );

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Temp_SafetyTempLimiter, Error_Temp_SafetyTempLimiter | Error_Temp_Supply );
	}
}

void CDlgWizardPM_RightViewInput_HeatingTemperature::OnEnSetFocusSafetyTempLimiter()
{
	m_ExtEditSafetyTempLimiter.SetSel( 0, -1 );
	m_dSafetyTempLimiterSaved = m_ExtEditSafetyTempLimiter.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_HeatingTemperature::OnKillFocusEditSafetyTempLimiter()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	// Do nothing if no changed.
	if( m_dSafetyTempLimiterSaved == m_ExtEditSafetyTempLimiter.GetCurrentValSI() )
	{
		return;
	}

	// Check if errors.
	int iErrorCode = IsHeatingSolarSafetyTempLimiterOK( &m_ExtEditSafetyTempLimiter, &m_ComboNorm );

	// Update 'CPMInputUser' class and technical parameters if no error.
	UpdateOtherHeatingSolarTemperatures( iErrorCode, &m_ExtEditSafetyTempLimiter );
	
	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Temp_SafetyTempLimiter, Error_Temp_SafetyTempLimiter | Error_Temp_Supply );
	}
}

LRESULT CDlgWizardPM_RightViewInput_HeatingTemperature::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgWizardPM_RightViewInput_Base::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	m_StaticSafetyTempLimiterUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_ExtEditSafetyTempLimiter, _U_TEMPERATURE );

	return 0;
}

void CDlgWizardPM_RightViewInput_HeatingTemperature::GetExtEditList( CArray<CExtNumEdit*> &arExtEditList )
{
	arExtEditList.Add( &m_ExtEditSafetyTempLimiter );
}

void CDlgWizardPM_RightViewInput_HeatingTemperature::_VerifyInputValues( bool bShowErrorMsg )
{
	IsHeatingSolarSafetyTempLimiterOK( &m_ExtEditSafetyTempLimiter, &m_ComboNorm, bShowErrorMsg );
}
