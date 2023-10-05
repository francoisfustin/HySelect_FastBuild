#include "stdafx.h"


#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "DlgWizPMRightViewInputCoolSystemVolumePower.h"
#include "RViewWizardSelBase.h"
#include "RViewWizardSelPM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewInput_CoolingSystemVolumePower::CDlgWizardPM_RightViewInput_CoolingSystemVolumePower( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, CoolingSystemVolumePower, IDD, pclParent )
{
	m_bGoToAdvModeMsgSent = false;
}

void CDlgWizardPM_RightViewInput_CoolingSystemVolumePower::ApplyPMInputUserUpdated( bool bWaterCharUpdated, bool bShowErrorMsg )
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

int CDlgWizardPM_RightViewInput_CoolingSystemVolumePower::IsAtLeastOneError( int &iErrorMaskNormal, int &iErrorMaskAdvanced )
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

	iErrorMaskNormal = Error_Installation_SystemVolume;
	iErrorMaskAdvanced = 0;

	return iError;
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_CoolingSystemVolumePower, CDlgWizardPM_RightViewInput_Base )
	ON_WM_LBUTTONDOWN()
	ON_EN_SETFOCUS( IDC_EDIT_VS, OnEnSetFocusSystemVolume )
	ON_EN_SETFOCUS( IDC_EDIT_QS, OnEnSetFocusInstalledPower )
	ON_EN_KILLFOCUS( IDC_EDIT_VS, OnKillFocusSystemVolume )
	ON_EN_KILLFOCUS( IDC_EDIT_QS, OnKillFocusInstalledPower )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_CoolingSystemVolumePower::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardPM_RightViewInput_Base::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_STATIC_VS, m_StaticSystemVolume );
	DDX_Control( pDX, IDC_STATIC_QS, m_StaticInstalledPower );
	DDX_Control( pDX, IDC_EDIT_VS, m_ExtEditSystemVolume );
	DDX_Control( pDX, IDC_EDIT_QS, m_ExtEditInstalledPower );
	DDX_Control( pDX, IDC_STATIC_VS_UNIT, m_StaticSystemVolumeUnit );
	DDX_Control( pDX, IDC_STATIC_QS_UNIT, m_StaticInstalledPowerUnit );
	DDX_Control( pDX, IDC_STATIC_GOADVMODE2, m_ButtonGoAdvancedMode );
}

BOOL CDlgWizardPM_RightViewInput_CoolingSystemVolumePower::OnInitDialog()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( FALSE );
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CDlgWizardPM_RightViewInput_Base::OnInitDialog();

	SetMainTitle( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWCOOLWATVOLPOWER_MAINTITLE ) );

	CString str;
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	//////////////////////////////////////////////////////////////////////////
	// EDITTEXT

	m_ExtEditSystemVolume.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditSystemVolume, _U_VOLUME );

	m_ExtEditInstalledPower.SetCurrentValSI( 0.0 );
	InitNumericalEdit( &m_ExtEditInstalledPower, _U_TH_POWER );

	if( true == pclPMInputUser->IfSysVolExtDefExist() )
	{
		m_ExtEditSystemVolume.EnableWindow( FALSE );
		m_ExtEditInstalledPower.EnableWindow( FALSE );
	}
	
	//////////////////////////////////////////////////////////////////////////
	// STATIC

	// Water content [Vs]:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTCOOLWATERVOLPOWER_SYSTEMVOLUME );
	m_StaticSystemVolume.SetTextAndToolTip( str );

	// Water content [Vs]: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ), name );
	SetDlgItemText( IDC_STATIC_VS_UNIT, name );

	// Installed power [Qs]:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTCOOLWATERVOLPOWER_INSTALLEDPOWER );
	m_StaticInstalledPower.SetTextAndToolTip( str );

	// Installed power [Qs]: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ), name );
	SetDlgItemText( IDC_STATIC_QS_UNIT, name );

	m_StaticSystemVolume.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticInstalledPower.SetBackColor( RGB( 255, 255, 255 ) );

	m_ExtEditSystemVolume.ActiveSpecialValidation( true, this );
	m_ExtEditInstalledPower.ActiveSpecialValidation( true, this );

	m_StaticSystemVolumeUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticInstalledPowerUnit.SetBackColor( RGB( 255, 255, 255 ) );

	m_ButtonGoAdvancedMode.ShowWindow( SW_HIDE );
	m_ButtonGoAdvancedMode.GetWindowRect( &m_ButtonGoAdanvedModePos );
	ScreenToClient( &m_ButtonGoAdanvedModePos );

	ApplyPMInputUserUpdated();

	return TRUE;
}

void CDlgWizardPM_RightViewInput_CoolingSystemVolumePower::OnPaint()
{
	CDlgWizardPM_RightViewInput_Base::OnPaint();

	DrawGoAdvancedNormalModeButtton( Draw_AdvancedMode, m_ButtonGoAdanvedModePos );
}

void CDlgWizardPM_RightViewInput_CoolingSystemVolumePower::OnLButtonDown( UINT nFlags, CPoint point )
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

			pclPMInputUser->GetpColdGeneratorListBackup()->CopyFrom( pclPMInputUser->GetColdGeneratorList() );
			pclPMInputUser->GetpColdConsumersListBackup()->CopyFrom( pclPMInputUser->GetColdConsumersList() );
			pclPMInputUser->GetpPipeListBackup()->CopyFrom( pclPMInputUser->GetPipeList() );

			m_bGoToAdvModeMsgSent = true;
			m_pclParent->PostMessage( WM_USER_RIGHTVIEWWIZPM_SWITCHMODE, WPARAM( CoolingGenerators ), 0 );
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

		if( TRUE == rectWaterContent.PtInRect( (POINT)point ) || 
			TRUE == rectInstalledPower.PtInRect( (POINT)point ) )
		{
			CString str = TASApp.LoadLocalizedString( IDS_DLGWIZARDPM_RVIEWINPUTVALUE_SYSTEMVOLPOWERRESETALL );

			if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
			{
				pclPMInputUser->GetColdGeneratorList()->Reset();
				pclPMInputUser->GetColdConsumersList()->Reset();
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

void CDlgWizardPM_RightViewInput_CoolingSystemVolumePower::OnEnSetFocusSystemVolume()
{
	m_ExtEditSystemVolume.SetSel( 0, -1 );
	m_dSystemVolumeSaved = m_ExtEditSystemVolume.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_CoolingSystemVolumePower::OnEnSetFocusInstalledPower()
{
	m_ExtEditInstalledPower.SetSel( 0, -1 );
	m_dInstalledPowerSaved = m_ExtEditSystemVolume.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_CoolingSystemVolumePower::OnKillFocusSystemVolume()
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

void CDlgWizardPM_RightViewInput_CoolingSystemVolumePower::OnKillFocusInstalledPower()
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

	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetInstalledPower( m_ExtEditInstalledPower.GetCurrentValSI() );
}

LRESULT CDlgWizardPM_RightViewInput_CoolingSystemVolumePower::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgWizardPM_RightViewInput_Base::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	m_StaticSystemVolumeUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_ExtEditSystemVolume, _U_VOLUME );

	m_StaticInstalledPowerUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TH_POWER ).c_str() );
	InitNumericalEdit( &m_ExtEditInstalledPower, _U_TH_POWER );

	return 0;
}

void CDlgWizardPM_RightViewInput_CoolingSystemVolumePower::GetExtEditList( CArray<CExtNumEdit*> &arExtEditList )
{
	arExtEditList.Add( &m_ExtEditSystemVolume );
	arExtEditList.Add( &m_ExtEditInstalledPower );
}

void CDlgWizardPM_RightViewInput_CoolingSystemVolumePower::_VerifyInputValues()
{
	IsHeatingCoolingSystemVolumeOK( &m_ExtEditSystemVolume );
}
