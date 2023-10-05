#include "stdafx.h"


#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "DlgWizPMRightViewInputSolarSystemVolumePowerSWKI.h"
#include "RViewWizardSelBase.h"
#include "RViewWizardSelPM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, SolarSystemVolumePower, IDD, pclParent )
{
	m_bGoToAdvModeMsgSent = false;
}

void CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::ApplyPMInputUserUpdated( bool bWaterCharUpdated, bool bShowErrorMsg )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	bool bAtLeastOneChange = false;

	if( m_ExtEditSystemVolume.GetCurrentValSI() != pclPMInputUser->GetSystemVolume() )
	{
		m_dSystemVolumeSaved = pclPMInputUser->GetSystemVolume();
		m_ExtEditSystemVolume.SetCurrentValSI( m_dSystemVolumeSaved );
		m_ExtEditSystemVolume.Update();
		bAtLeastOneChange = true;
	}

	if( m_ExtEditSolarContent.GetCurrentValSI() != pclPMInputUser->GetSolarCollectorVolume() )
	{
		m_ExtEditSolarContent.SetCurrentValSI( pclPMInputUser->GetSolarCollectorVolume() );
		m_ExtEditSolarContent.Update();
		bAtLeastOneChange = true;
	}

	if( m_ExtEditInstalledPower.GetCurrentValSI() != pclPMInputUser->GetInstalledPower() )
	{
		m_dInstalledPowerSaved = pclPMInputUser->GetInstalledPower();
		m_ExtEditInstalledPower.SetCurrentValSI( m_dInstalledPowerSaved );
		m_ExtEditInstalledPower.Update();
		bAtLeastOneChange = true;
	}

	if( true == bAtLeastOneChange )
	{
		_VerifyInputValues();
	}
}

int CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::IsAtLeastOneError( int &iErrorMaskNormal, int &iErrorMaskAdvanced )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( 0 );
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	int iError = 0;

	if( pclPMInputUser->GetSystemVolume() <= 0.0 )
	{
		iError |= Error_Installation_SystemVolume;
	}

	if( pclPMInputUser->GetSystemVolume() <= pclPMInputUser->GetSolarCollectorVolume() )
	{
		iError |= ( Error_Installation_SystemVolume | Error_Installation_SolarContent );
	}

	if( pclPMInputUser->GetInstalledPower() <= 0.0 )
	{
		iError |= Error_Installation_InstalledPower;
	}

	iErrorMaskNormal = Error_Installation_SystemVolume | Error_Installation_SolarContent | Error_Installation_InstalledPower;
	iErrorMaskAdvanced = Error_Installation_SystemVolume | Error_Installation_SolarContent | Error_Installation_InstalledPower | Error_Installation_StorageTankMaxTemp ;

	return iError;
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI, CDlgWizardPM_RightViewInput_Base )
	ON_EN_SETFOCUS( IDC_EDIT_VS, OnEnSetFocusSystemVolume )
	ON_EN_SETFOCUS( IDC_EDIT_VK, OnEnSetFocusSolarCollector )
	ON_EN_SETFOCUS( IDC_EDIT_QS, OnEnSetFocusInstalledPower )
	ON_EN_KILLFOCUS( IDC_EDIT_VS, OnKillFocusSystemVolume )
	ON_EN_KILLFOCUS( IDC_EDIT_VK, OnKillFocusSolarCollector )
	ON_EN_KILLFOCUS( IDC_EDIT_QS, OnKillFocusInstallPower )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardPM_RightViewInput_Base::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_STATIC_VS, m_StaticSystemVolume );
	DDX_Control( pDX, IDC_STATIC_VK, m_StaticSolarContent );
	DDX_Control( pDX, IDC_STATIC_QS, m_StaticInstalledPower );
	DDX_Control( pDX, IDC_EDIT_VS, m_ExtEditSystemVolume );
	DDX_Control( pDX, IDC_EDIT_VK, m_ExtEditSolarContent );
	DDX_Control( pDX, IDC_EDIT_QS, m_ExtEditInstalledPower );
	DDX_Control( pDX, IDC_STATIC_VS_UNIT, m_StaticSystemVolumeUnit );
	DDX_Control( pDX, IDC_STATIC_VK_UNIT, m_StaticSolarContentUnit );
	DDX_Control( pDX, IDC_STATIC_QS_UNIT, m_StaticInstalledPowerUnit );
	DDX_Control( pDX, IDC_STATIC_GOADVMODE, m_ButtonGoAdvancedMode );
}

BOOL CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::OnInitDialog()
{
	CDlgWizardPM_RightViewInput_Base::OnInitDialog();

	SetMainTitle( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWSOLARWATVOLPOWER_MAINTITLE ) );

	CString str;
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	//////////////////////////////////////////////////////////////////////////
	// EDITTEXT

	m_ExtEditSystemVolume.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditSystemVolume, _U_VOLUME );

	m_ExtEditSolarContent.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditSolarContent, _U_VOLUME );

	m_ExtEditInstalledPower.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditInstalledPower, _U_TH_POWER );

	//////////////////////////////////////////////////////////////////////////
	// STATIC

	// Water content [Vs]:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTSOLARWATERVOLPOWER_SYSTEMVOLUME );
	m_StaticSystemVolume.SetTextAndToolTip( str );

	// Water content [Vs]: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ), name );
	SetDlgItemText( IDC_STATIC_VS_UNIT, name );

	// Solar content [Vk]:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTSOLARWATERVOLPOWER_SOLARCONTENT );
	m_StaticSolarContent.SetTextAndToolTip( str );

	// Solar content [Vk]: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ), name );
	SetDlgItemText( IDC_STATIC_VK_UNIT, name );

// Installed power [Qs]:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTSOLARWATERVOLPOWER_INSTALLEDPOWER );
	m_StaticInstalledPower.SetTextAndToolTip( str );

	// Installed power [Qs]: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ), name );
	SetDlgItemText( IDC_STATIC_QS_UNIT, name );

	m_StaticSystemVolume.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSolarContent.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticInstalledPower.SetBackColor( RGB( 255, 255, 255 ) );

	m_ExtEditSystemVolume.ActiveSpecialValidation( true, this );
	m_ExtEditSolarContent.ActiveSpecialValidation( true, this );
	m_ExtEditInstalledPower.ActiveSpecialValidation( true, this );

	m_StaticSystemVolumeUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSolarContentUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticInstalledPowerUnit.SetBackColor( RGB( 255, 255, 255 ) );

	m_ButtonGoAdvancedMode.ShowWindow( SW_HIDE );
	m_ButtonGoAdvancedMode.GetWindowRect( &m_ButtonGoAdanvedModePos );
	ScreenToClient( &m_ButtonGoAdanvedModePos );

	ApplyPMInputUserUpdated();

	return TRUE;
}

void CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::OnPaint()
{
	CDlgWizardPM_RightViewInput_Base::OnPaint();

	DrawGoAdvancedNormalModeButtton( Draw_AdvancedMode, m_ButtonGoAdanvedModePos );
}

void CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( false == m_bGoToAdvModeMsgSent )
	{
		CRect rectClient;
		GetClientRect( &rectClient );
		CRect rectGoButton( 20, m_ButtonGoAdanvedModePos.top, rectClient.Width() - 20, m_ButtonGoAdanvedModePos.top + 20 );

		if( TRUE == rectGoButton.PtInRect( (POINT)point ) )
		{
			m_bGoToAdvModeMsgSent = true;
			m_pclParent->PostMessage( WM_USER_RIGHTVIEWWIZPM_SWITCHMODE, WPARAM( SolarSystemVolumePowerSWKIAdvance ), 0 );
		}
	}

	CDlgWizardPM_RightViewInput_Base::OnLButtonDown( nFlags, point );
}

void CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::OnEnSetFocusSystemVolume()
{
	m_ExtEditSystemVolume.SetSel( 0, -1 );
	m_dSystemVolumeSaved = m_ExtEditSystemVolume.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::OnEnSetFocusSolarCollector()
{
	m_ExtEditSolarContent.SetSel( 0, -1 );
	m_dSolarContentSaved = m_ExtEditSystemVolume.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::OnEnSetFocusInstalledPower()
{
	m_ExtEditInstalledPower.SetSel( 0, -1 );
	m_dInstalledPowerSaved = m_ExtEditSystemVolume.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::OnKillFocusSystemVolume()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Do nothing if no changed.
	if( m_dSystemVolumeSaved == m_ExtEditSystemVolume.GetCurrentValSI() )
	{
		return;
	}

	int iErrorCode = IsSolarSystemVolumeOK( &m_ExtEditSystemVolume, &m_ExtEditSolarContent );

	// Do not save system volume if error.
	if( Error_Installation_SystemVolume != ( Error_Installation_SystemVolume & iErrorCode ) )
	{
		m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSystemVolume( m_ExtEditSystemVolume.GetCurrentValSI() );
	}

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Installation_SystemVolume );
	}
}

void CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::OnKillFocusSolarCollector()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Do nothing if no changed.
	if( m_dSolarContentSaved == m_ExtEditSolarContent.GetCurrentValSI() )
	{
		return;
	}

	int iErrorCode = IsSolarSolarContentOK( &m_ExtEditSolarContent, &m_ExtEditSystemVolume );

	// Do not save solar collector volume if error.
	if( Error_Installation_SolarContent != ( Error_Installation_SolarContent & iErrorCode ) )
	{
		m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSolarCollectorVolume( m_ExtEditSolarContent.GetCurrentValSI() );
	}

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Installation_SystemVolume | Error_Installation_SolarContent );
	}
}

void CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::OnKillFocusInstallPower()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Do nothing if no changed.
	if( m_dInstalledPowerSaved == m_ExtEditInstalledPower.GetCurrentValSI() )
	{
		return;
	}

	// Check if error.
	int iErrorCode = IsInstalledPowerOK( &m_ExtEditInstalledPower );

	// Do not save installed power if error.
	if( Error_Installation_InstalledPower != ( iErrorCode & Error_Installation_InstalledPower ) )
	{
		m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetInstalledPower( m_ExtEditInstalledPower.GetCurrentValSI() );
	}

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Installation_InstalledPower );
	}
}

LRESULT CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgWizardPM_RightViewInput_Base::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	m_StaticSystemVolumeUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_ExtEditSystemVolume, _U_VOLUME );

	m_StaticSolarContentUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_ExtEditSolarContent, _U_VOLUME );

	m_StaticInstalledPowerUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TH_POWER ).c_str() );
	InitNumericalEdit( &m_ExtEditInstalledPower, _U_TH_POWER );

	return 0;
}

void CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::GetExtEditList( CArray<CExtNumEdit*> &arExtEditList )
{
	arExtEditList.Add( &m_ExtEditSystemVolume );
	arExtEditList.Add( &m_ExtEditSolarContent );
	arExtEditList.Add( &m_ExtEditInstalledPower );
}

void CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::InitToolTips( CWnd *pWnd )
{
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		CString TTstr;

		if( NULL == pWnd || pWnd == &m_ExtEditSolarContent )
		{
			TTstr = TASApp.LoadLocalizedString( IDS_PM_TTSOLARCONTENT );
			m_ToolTip.AddToolWindow( &m_ExtEditSolarContent, TTstr );
		}
	}

	CDlgWizardPM_RightViewInput_Base::InitToolTips( pWnd );
}

void CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI::_VerifyInputValues()
{
	IsInstalledPowerOK( &m_ExtEditInstalledPower );
	IsSolarSystemVolumeOK( &m_ExtEditSystemVolume, &m_ExtEditSolarContent );
}
