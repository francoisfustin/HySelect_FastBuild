#include "stdafx.h"


#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "DlgWizPMRightViewInputHeatSystemVolumePowerSWKI.h"
#include "DlgIndSelPMSysVolHeating.h"
#include "RViewWizardSelBase.h"
#include "RViewWizardSelPM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, HeatingSystemVolumePower, IDD, pclParent )
{
	m_bGoToAdvModeMsgSent = false;
}

void CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::ApplyPMInputUserUpdated( bool bWaterCharUpdated, bool bShowErrorMsg )
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

	if( m_ExtEditStorageTankVolume.GetCurrentValSI() != pclPMInputUser->GetStorageTankVolume() )
	{
		m_dStorageTankVolumeSaved = pclPMInputUser->GetStorageTankVolume();
		m_ExtEditStorageTankVolume.SetCurrentValSI( m_dStorageTankVolumeSaved );
		m_ExtEditStorageTankVolume.Update();
		bAtLeastOneChange = true;
	}

	if( m_ExtEditStorageTankMaxTemp.GetCurrentValSI() != pclPMInputUser->GetStorageTankMaxTemp() )
	{
		m_dStorageTankMaxTempSaved = pclPMInputUser->GetStorageTankMaxTemp();
		m_ExtEditStorageTankMaxTemp.SetCurrentValSI( m_dStorageTankMaxTempSaved );
		m_ExtEditStorageTankMaxTemp.Update();
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

int CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::IsAtLeastOneError( int &iErrorMaskNormal, int &iErrorMaskAdvanced )
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

	if( pclPMInputUser->GetInstalledPower() <= 0.0 )
	{
		iError |= Error_Installation_InstalledPower;
	}

	if( pclPMInputUser->GetStorageTankMaxTemp() < m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetReturnTemperature() )
	{
		iError |= Error_Installation_StorageTankMaxTemp;
	}

	iErrorMaskNormal = Error_Installation_SystemVolume | Error_Installation_StorageTankMaxTemp | Error_Installation_InstalledPower;
	iErrorMaskAdvanced = 0;

	return iError;
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI, CDlgWizardPM_RightViewInput_Base )
	ON_WM_LBUTTONDOWN()
	ON_EN_SETFOCUS( IDC_EDIT_VS, OnEnSetFocusSystemVolume )
	ON_EN_SETFOCUS( IDC_EDIT_STORAGETANKVOLUME, OnEnSetFocusStorageTankVolume )
	ON_EN_SETFOCUS( IDC_EDIT_STORAGETANKMAXTEMP, OnEnSetFocusStorageTankMaxTemp )
	ON_EN_SETFOCUS( IDC_EDIT_QS, OnEnSetFocusInstalledPower )
	ON_EN_KILLFOCUS( IDC_EDIT_VS, OnKillFocusSystemVolume )
	ON_EN_KILLFOCUS( IDC_EDIT_STORAGETANKVOLUME, OnKillFocusStorageTankVolume )
	ON_EN_KILLFOCUS( IDC_EDIT_STORAGETANKMAXTEMP, OnKillFocusStorageTankMaxTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_QS, OnKillFocusInstalledPower )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardPM_RightViewInput_Base::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_STATIC_VS, m_StaticSystemVolume );
	DDX_Control( pDX, IDC_STATIC_STORAGETANKVOLUME, m_StaticStorageTankVolume );
	DDX_Control( pDX, IDC_STATIC_STORAGETANKMAXTEMP, m_StaticStorageTankMaxTemp );
	DDX_Control( pDX, IDC_STATIC_QS, m_StaticInstalledPower );
	DDX_Control( pDX, IDC_EDIT_VS, m_ExtEditSystemVolume );
	DDX_Control( pDX, IDC_EDIT_STORAGETANKVOLUME, m_ExtEditStorageTankVolume );
	DDX_Control( pDX, IDC_EDIT_STORAGETANKMAXTEMP, m_ExtEditStorageTankMaxTemp );
	DDX_Control( pDX, IDC_EDIT_QS, m_ExtEditInstalledPower );
	DDX_Control( pDX, IDC_STATIC_VS_UNIT, m_StaticSystemVolumeUnit );
	DDX_Control( pDX, IDC_STATIC_STORAGETANKVOLUMEUNIT, m_StaticStorageTankVolumeUnit );
	DDX_Control( pDX, IDC_STATIC_STORAGETANKMAXTEMPUNIT, m_StaticStorageTankMaxTempUnit );
	DDX_Control( pDX, IDC_STATIC_QS_UNIT, m_StaticInstalledPowerUnit );
	DDX_Control( pDX, IDC_STATIC_GOADVMODE2, m_ButtonGoAdvancedMode );
}

BOOL CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::OnInitDialog()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( FALSE );
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CDlgWizardPM_RightViewInput_Base::OnInitDialog();

	SetMainTitle( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATWATVOLPOWER_MAINTITLE ) );

	CString str;
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	//////////////////////////////////////////////////////////////////////////
	// EDITTEXT

	m_ExtEditSystemVolume.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditSystemVolume, _U_VOLUME );

	m_ExtEditStorageTankVolume.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditStorageTankVolume, _U_VOLUME );

	m_ExtEditStorageTankMaxTemp.SetCurrentValSI( m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetStorageTankMaxTemp() );
	InitNumericalEdit( &m_ExtEditStorageTankMaxTemp, _U_TEMPERATURE );

	m_ExtEditInstalledPower.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditInstalledPower, _U_TH_POWER );

	if( true == pclPMInputUser->IfSysVolExtDefExist() )
	{
		m_ExtEditSystemVolume.EnableWindow( FALSE );
		m_ExtEditInstalledPower.EnableWindow( FALSE );
	}
	
	//////////////////////////////////////////////////////////////////////////
	// STATIC

	// System volume [Vs]:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTHEATWATERVOLPOWER_SYSTEMVOLUME );
	m_StaticSystemVolume.SetTextAndToolTip( str );

	// System volume [Vs]: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ), name );
	SetDlgItemText( IDC_STATIC_VS_UNIT, name );

	// Storage tank volume [Vsto]:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTHEATWATERVOLPOWER_STORAGETANKVOLUME );
	m_StaticStorageTankVolume.SetTextAndToolTip( str );

	// Storage tank volume [Vsto]: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ), name );
	SetDlgItemText( IDC_STATIC_STORAGETANKVOLUMEUNIT, name );

	// Storage tank max. temp. [Tmax,vsto]:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTHEATWATERVOLPOWER_STORAGETANKMAXTEMP );
	m_StaticStorageTankMaxTemp.SetTextAndToolTip( str );

	// Storage tank max. temp. [Tmax,vsto]: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), name );
	SetDlgItemText( IDC_STATIC_STORAGETANKMAXTEMPUNIT, name );

	// Installed power [Qs]:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTHEATWATERVOLPOWER_INSTALLEDPOWER );
	m_StaticInstalledPower.SetTextAndToolTip( str );

	// Installed power [Qs]: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ), name );
	SetDlgItemText( IDC_STATIC_QS_UNIT, name );

	m_StaticSystemVolume.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticStorageTankVolume.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticStorageTankMaxTemp.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticInstalledPower.SetBackColor( RGB( 255, 255, 255 ) );

	m_ExtEditSystemVolume.ActiveSpecialValidation( true, this );
	m_ExtEditStorageTankVolume.ActiveSpecialValidation( true, this );
	m_ExtEditStorageTankMaxTemp.ActiveSpecialValidation( true, this );
	m_ExtEditInstalledPower.ActiveSpecialValidation( true, this );

	m_StaticSystemVolumeUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticStorageTankVolumeUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticStorageTankMaxTempUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticInstalledPowerUnit.SetBackColor( RGB( 255, 255, 255 ) );

	m_ButtonGoAdvancedMode.ShowWindow( SW_HIDE );
	m_ButtonGoAdvancedMode.GetWindowRect( &m_ButtonGoAdanvedModePos );
	ScreenToClient( &m_ButtonGoAdanvedModePos );

	ApplyPMInputUserUpdated();

	return TRUE;
}

void CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::OnPaint()
{
	CDlgWizardPM_RightViewInput_Base::OnPaint();

	DrawGoAdvancedNormalModeButtton( Draw_AdvancedMode, m_ButtonGoAdanvedModePos );
}

void CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	if( false == m_bGoToAdvModeMsgSent )
	{
		CRect rectClient;
		GetClientRect( &rectClient );
		CRect rectGoButton( 20, m_ButtonGoAdanvedModePos.top, rectClient.Width() - 20, m_ButtonGoAdanvedModePos.top + 20 );

		if( TRUE == rectGoButton.PtInRect( (POINT)point ) )
		{
			// Before to go in advance mode, we backup the system volume and the installed power to allow to retrieve
			// these values if user abort or reset in the advanced mode.
			pclPMInputUser->SetSystemVolumeBackup( m_ExtEditSystemVolume.GetCurrentValSI() );
			pclPMInputUser->SetInstalledPowerBackup( m_ExtEditInstalledPower.GetCurrentValSI() );

			pclPMInputUser->GetpHeatGeneratorListBackup()->CopyFrom( pclPMInputUser->GetHeatGeneratorList() );
			pclPMInputUser->GetpHeatConsumersListBackup()->CopyFrom( pclPMInputUser->GetHeatConsumersList() );
			pclPMInputUser->GetpPipeListBackup()->CopyFrom( pclPMInputUser->GetPipeList() );

			m_bGoToAdvModeMsgSent = true;
			m_pclParent->PostMessage( WM_USER_RIGHTVIEWWIZPM_SWITCHMODE, WPARAM( HeatingGenerators ), 0 );
		}
	}

	if( WS_DISABLED == ( m_ExtEditSystemVolume.GetStyle() & WS_DISABLED ) )
	{
		CRect rectWaterContent;
		m_ExtEditSystemVolume.GetWindowRect( &rectWaterContent );
		ScreenToClient( &rectWaterContent );

		CRect rectInstalledPower;
		m_ExtEditInstalledPower.GetWindowRect( &rectInstalledPower );
		ScreenToClient( &rectInstalledPower );

		if( TRUE == rectWaterContent.PtInRect( (POINT)point )
				|| TRUE == rectInstalledPower.PtInRect( (POINT)point ) )
		{
			CString str = TASApp.LoadLocalizedString( IDS_DLGWIZARDPM_RVIEWINPUTVALUE_SYSTEMVOLPOWERRESETALL );

			if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
			{
				pclPMInputUser->GetHeatGeneratorList()->Reset();
				pclPMInputUser->GetHeatConsumersList()->Reset();
				pclPMInputUser->GetPipeList()->Reset();

				m_ExtEditSystemVolume.EnableWindow( TRUE );
				m_ExtEditInstalledPower.EnableWindow( TRUE );

				if( TRUE == rectWaterContent.PtInRect( (POINT)point ) )
				{
					m_ExtEditSystemVolume.SetFocus();
				}
				else
				{
					m_ExtEditInstalledPower.SetFocus();
				}
			}
		}
	}

	CDlgWizardPM_RightViewInput_Base::OnLButtonDown( nFlags, point );
}

void CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::OnEnSetFocusSystemVolume()
{
	m_ExtEditSystemVolume.SetSel( 0, -1 );
	m_dSystemVolumeSaved = m_ExtEditSystemVolume.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::OnEnSetFocusStorageTankVolume()
{
	m_ExtEditStorageTankVolume.SetSel( 0, -1 );
	m_dStorageTankVolumeSaved = m_ExtEditStorageTankVolume.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::OnEnSetFocusStorageTankMaxTemp()
{
	m_ExtEditStorageTankMaxTemp.SetSel( 0, -1 );
	m_dStorageTankMaxTempSaved = m_ExtEditStorageTankMaxTemp.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::OnEnSetFocusInstalledPower()
{
	m_ExtEditInstalledPower.SetSel( 0, -1 );
	m_dInstalledPowerSaved = m_ExtEditSystemVolume.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::OnKillFocusSystemVolume()
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

	int iErrorCode = IsHeatingCoolingSystemVolumeOK( &m_ExtEditSystemVolume );

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

void CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::OnKillFocusStorageTankVolume()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Do nothing if no changed.
	if( m_dStorageTankVolumeSaved == m_ExtEditStorageTankVolume.GetCurrentValSI() )
	{
		return;
	}

	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetStorageTankVolume( m_ExtEditStorageTankVolume.GetCurrentValSI() );
}

void CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::OnKillFocusStorageTankMaxTemp()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Do nothing if no changed.
	if( m_dStorageTankMaxTempSaved == m_ExtEditStorageTankMaxTemp.GetCurrentValSI() )
	{
		return;
	}

	int iErrorCode = IsStorageTankMaxTempOK( &m_ExtEditStorageTankMaxTemp );

	// Do not save storage tank max. temperature if error.
	if( Error_Installation_StorageTankMaxTemp != ( Error_Installation_StorageTankMaxTemp & iErrorCode ) )
	{
		m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetStorageTankMaxTemp( m_ExtEditStorageTankMaxTemp.GetCurrentValSI() );
	}

	if( 0 == iErrorCode )
	{
		// Now, when we have an error in wizard, we don't save the value in the 'CPMInputUser' until the value is correct. If error exists and user
		// goes in one other step, the errors are cleared. 
		// We have only one case to manage here. If user input bad values in individual selection and goes in wizard mode, the values are
		// saved in this case in the 'CPMInputUser'. So, going to one other step doesn't clear the error. We need to keep the error red cross status
		// icon in the step button to force user to correct the bad values. And when these values are correct (iErrorCode = 0) we can clear
		// the error red cross icon and allow user to go in the results step.
		( (CRViewWizardSelPM *)m_pclParent )->OnInputChange( iErrorCode, Error_Installation_StorageTankMaxTemp );
	}
}

void CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::OnKillFocusInstalledPower()
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

	// Do not save isntalled power if error.
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

LRESULT CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgWizardPM_RightViewInput_Base::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	m_StaticSystemVolumeUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_ExtEditSystemVolume, _U_VOLUME );

	m_StaticStorageTankVolumeUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_ExtEditStorageTankVolume, _U_VOLUME );

	m_StaticStorageTankMaxTempUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_ExtEditStorageTankMaxTemp, _U_TEMPERATURE );

	m_StaticInstalledPowerUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TH_POWER ).c_str() );
	InitNumericalEdit( &m_ExtEditInstalledPower, _U_TH_POWER );

	return 0;
}

void CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::GetExtEditList( CArray<CExtNumEdit*> &arExtEditList )
{
	arExtEditList.Add( &m_ExtEditSystemVolume );
	arExtEditList.Add( &m_ExtEditStorageTankVolume );
	arExtEditList.Add( &m_ExtEditStorageTankMaxTemp );
	arExtEditList.Add( &m_ExtEditInstalledPower );
}

void CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI::_VerifyInputValues()
{
	IsHeatingCoolingSystemVolumeOK( &m_ExtEditSystemVolume );
	IsInstalledPowerOK( &m_ExtEditInstalledPower );
	IsStorageTankMaxTempOK( &m_ExtEditStorageTankMaxTemp );
}
