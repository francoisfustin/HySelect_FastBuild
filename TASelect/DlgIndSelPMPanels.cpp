#include "stdafx.h"
#include "afxdialogex.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "SelectPM.h"

#include "RViewSSelSS.h"
#include "DlgLeftTabSelManager.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelPM.h"

#include "DlgIndSelPMSysVolHeating.h"
#include "DlgIndSelPMSysVolCooling.h"

#include "DlgSolarContentMultiplierFactor.h"
#include "DlgStorageTankMaxTemperature.h"

#include "DlgIndSelPMPanels.h"

CDlgIndSelPMPanelBase::CDlgIndSelPMPanelBase( CIndSelPMParams *pclIndSelParams, PanelType ePanelType, UINT nID, CWnd *pParent )
	: CDialogExt( nID, pParent ), CDlgSelectionTabHelper( this )
{
	m_pclIndSelPMParams = pclIndSelParams;
	m_ePanelType = ePanelType;
	m_pclInterface = NULL;
	m_bOnEnChangeEnabled = true;
	m_dDegassingMaxTempSaved = 0.0;
	m_dWaterMakeUpNetworkPNSaved = 0.0;
}

void CDlgIndSelPMPanelBase::EnableDegassing( bool bEnable, bool bWaterMakupChecked )
{
	m_GroupDegassing.EnableWindow( bEnable, true );
	m_EditDegassingMaxTemp.ResetDrawBorder();

	if( ePanelNoPressMaint != GetPanelType() )
	{
		// For heating, cooling and solar the max temperature point of the degassing inlet is the same as the max temperature
		// of the system. In this case we disabled edition on this field.
		m_EditDegassingMaxTemp.EnableWindow( FALSE );

		m_GroupDegassing.ShowWindow( bEnable );

		if( true == bEnable )
		{
			m_GroupWaterMakeUp.MoveGroupBox( m_OrgPosGroupWaterMakeUp );
		}
		else
		{
			m_GroupWaterMakeUp.MoveGroupBox( m_OrgPosGroupDegassing );
		}

		// For heating, cooling and solar panel, when we remove the 'Degassing' or/and 'Water make-up' group, we resize the client area
		// to match the new size.
		CRect rectNewClient = m_OrgClient;

		// Remove the 'Degassing' group height if it is not visible.
		if( false == bEnable )
		{
			rectNewClient.bottom -= ( m_OrgPosGroupWaterMakeUp.top - m_OrgPosGroupDegassing.top );
		}

		// Remove the 'Water Make-Up' group height if it is not visible.
		if( false == bWaterMakupChecked )
		{
			rectNewClient.bottom -= ( m_OrgClient.bottom - m_OrgPosGroupWaterMakeUp.top );
		}

		SetWindowPos( NULL, -1, -1, rectNewClient.Width(), rectNewClient.Height(), SWP_NOMOVE | SWP_NOACTIVATE );

		Invalidate();
		UpdateWindow();
	}
}

void CDlgIndSelPMPanelBase::EnableWaterMakeUp( bool bEnable, bool bDegassingChecked )
{
	m_GroupWaterMakeUp.EnableWindow( bEnable, true );
	m_EditWaterMakeUpNetworkPN.ResetDrawBorder();
	if( ePanelNoPressMaint == GetPanelType() )
	{
		m_EditWaterMakeUpWaterTemp.ResetDrawBorder();
	}

	if( ePanelNoPressMaint != GetPanelType() )
	{
		m_GroupWaterMakeUp.ShowWindow( bEnable );

		// When Water make-up is enabled without degassing, move water make-up group at the degassing position
		if( true == bEnable )
		{
			if( true == bDegassingChecked )
			{
				m_GroupWaterMakeUp.MoveGroupBox( m_OrgPosGroupWaterMakeUp );
			}
			else
			{
				m_GroupWaterMakeUp.MoveGroupBox( m_OrgPosGroupDegassing );
			}
		}

		// For heating, cooling and solar panel, when we remove the 'Degassing' or/and 'Water make-up' group, we resize the client area
		// to match the new size.
		CRect rectNewClient = m_OrgClient;

		// Remove the 'Degassing' group height if it is not visible.
		if( false == bDegassingChecked )
		{
			rectNewClient.bottom -= ( m_OrgPosGroupWaterMakeUp.top - m_OrgPosGroupDegassing.top );
		}

		// Remove the 'Water Make-Up' group height if it is not visible.
		if( false == bEnable )
		{
			rectNewClient.bottom -= ( m_OrgClient.bottom - m_OrgPosGroupWaterMakeUp.top );
		}

		SetWindowPos( NULL, -1, -1, rectNewClient.Width(), rectNewClient.Height(), SWP_NOMOVE | SWP_NOACTIVATE );

		Invalidate();
		UpdateWindow();
	}
}

bool CDlgIndSelPMPanelBase::ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser )
	{
		return false;
	}

	m_GroupDegassing.EnableWindow( ( BST_CHECKED == pclPMInputUser->GetDegassingChecked() ) ? true : false , true );

	m_EditDegassingMaxTemp.SetCurrentValSI( pclPMInputUser->GetDegassingMaxTempConnectPoint() );
	m_EditDegassingMaxTemp.Update();

	m_GroupWaterMakeUp.EnableWindow( pclPMInputUser->IsWaterMakeUpExist(), true );

	m_EditWaterMakeUpNetworkPN.SetCurrentValSI( pclPMInputUser->GetWaterMakeUpNetworkPN() );
	m_EditWaterMakeUpNetworkPN.Update();

	return true;
}

bool CDlgIndSelPMPanelBase::VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser )
	{
		return false;
	}

	IsDegassingMaxTempOK( true );
	IsWaterMakeUpTempOK( true );

	ResetToolTipsErrorBorder( &m_EditWaterMakeUpNetworkPN );
	m_EditWaterMakeUpNetworkPN.ResetDrawBorder();

	CString msg;

	for( std::vector<std::pair<int, int>>::iterator iterError = m_vecErrorList.begin(); iterError != m_vecErrorList.end();
		 iterError++ )
	{
		if( CPMInputUser::ePM_InputEmpty == iterError->first )
		{
			if( CPMInputUser::RACIE_WaterMakeUpPSN == iterError->second )
			{
				msg = _T("\r\n - ") + TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORWATERMAKEUPPSN );
				m_mapErrorMsg[MEID_WaterMakeUpPSN] = msg;

				ResetToolTipsErrorBorder( &m_EditWaterMakeUpNetworkPN );
				msg = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORWATERMAKEUPPSN );
				m_ToolTip.AddToolWindow( &m_EditWaterMakeUpNetworkPN, msg );
				m_EditWaterMakeUpNetworkPN.SetDrawBorder( true, _RED );
			}
		}
	}

	if( 0 == ( int )m_mapErrorMsg.size() )
	{
		return true;
	}

	msg = TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_INVALIDINPUT );

	for( std::map<int, CString>::iterator iter = m_mapErrorMsg.begin(); iter != m_mapErrorMsg.end(); iter++ )
	{
		msg += iter->second;
	}

	Invalidate();
	UpdateWindow();

	if( true == bAllowShowErrorMessage )
	{
		AfxMessageBox( msg );
	}

	return false;
}

void CDlgIndSelPMPanelBase::OnNewDocument()
{
	ResetToolTipsErrorBorder();
	InitToolTips();

	// To avoid any 'OnEnChange' event to be called.
	m_bOnEnChangeEnabled = false;

	// Don't call 'OnUnitChange' because in this method we call a 'FillPMInputUser' and 'ApplyPMInputUser' and this will change status of some
	// control that we don't need when 'OnNewDocument' is called.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	GetDlgItem( IDC_STATICDEGTEMPUNIT )->SetWindowText( pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditDegassingMaxTemp, _U_TEMPERATURE );

	GetDlgItem( IDC_STATICWMNETWORKPNUNIT )->SetWindowText( pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	InitNumericalEdit( &m_EditWaterMakeUpNetworkPN, _U_PRESSURE );

	m_bOnEnChangeEnabled = true;
}

bool CDlgIndSelPMPanelBase::LeftTabKillFocus( bool fNext )
{
	// !!! Does not intentionally call the base class !!!

	CDlgIndSelPressureMaintenance::InterfacePMPanels *pInterface = ( CDlgIndSelPressureMaintenance::InterfacePMPanels * )
			m_pclInterface;

	if( NULL == pInterface )
	{
		return false;
	}

	bool fReturn = false;

	if( true == fNext )
	{
		// TAB -> must set the focus on the 'CDlgInSelPMPanels' and set the focus on the first control.
		CRViewSSelSS *pclRViewSSelSS = DLGINDSELPMINTERFACE_GETLINKEDRIGHTVIEW( pInterface );

		if( NULL != pclRViewSSelSS && TRUE == pclRViewSSelSS->IsWindowVisible() && false == pclRViewSSelSS->IsEmpty() )
		{
			// Set the focus on the right view.
			pclRViewSSelSS->SetFocus();
			// Set focus on the appropriate group in the right view.
			pclRViewSSelSS->SetFocusW( true );
			fReturn = true;
		}
		else
		{
			// Focus must be set on the first control of the parent 'CDlgIndSelPressureMaintenance'.
			pInterface->m_pclParent->SetFocus();
			pInterface->m_pclParent->SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere::First );
			fReturn = true;
		}
	}
	else
	{
		// SHIFT + TAB -> go to the top control only if right view is not empty.
		pInterface->m_pclParent->SetFocus();
		pInterface->m_pclParent->SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere::Last );
		fReturn = true;
	}

	return fReturn;
}

void CDlgIndSelPMPanelBase::OnSpecialValidation( CWnd *pWnd, int iVirtualKey )
{
	CDlgIndSelPressureMaintenance::InterfacePMPanels *pInterface = (CDlgIndSelPressureMaintenance::InterfacePMPanels *)m_pclInterface;

	if( NULL == pInterface )
	{
		return;
	}

	if( NULL == pWnd )
	{
		return;
	}

	if( VK_DOWN == iVirtualKey )
	{
		CWnd *pWndNextFocus = GetNextDlgTabItem( pWnd, FALSE );

		if( NULL != pWndNextFocus )
		{
			CRect rectCurrent;
			pWnd->GetWindowRect( &rectCurrent );
			ScreenToClient( &rectCurrent );
			CRect rectNext;
			pWndNextFocus->GetWindowRect( &rectNext );
			ScreenToClient( &rectNext );

			if( rectNext.bottom > rectCurrent.top )
			{
				pWndNextFocus->SetFocus();
			}
		}
	}
	else if( VK_UP == iVirtualKey )
	{
		CWnd *pWndPrevFocus = GetNextDlgTabItem( pWnd, TRUE );

		if( NULL != pWndPrevFocus )
		{
			CRect rectCurrent;
			pWnd->GetWindowRect( &rectCurrent );
			ScreenToClient( &rectCurrent );
			CRect rectPrev;
			pWndPrevFocus->GetWindowRect( &rectPrev );
			ScreenToClient( &rectPrev );

			if( rectPrev.top < rectCurrent.bottom )
			{
				pWndPrevFocus->SetFocus();
			}
		}
	}
	else if( VK_RETURN == iVirtualKey )
	{
		pInterface->m_pclParent->ClickOnSuggestButton();
	}
}

void CDlgIndSelPMPanelBase::EnablePMSelection( bool Enable )
{
	m_EditDegassingMaxTemp.ActiveSpecialValidation( Enable, this );
	m_EditWaterMakeUpNetworkPN.ActiveSpecialValidation( Enable, this );
	Invalidate();
	UpdateWindow();
}

BEGIN_MESSAGE_MAP( CDlgIndSelPMPanelBase, CDialogExt )
	ON_MESSAGE( WM_USER_PIPECHANGE, OnPipeChange )
	ON_MESSAGE( WM_USER_UNITCHANGE, OnUnitChange )
	ON_MESSAGE( WM_USER_WATERCHANGE, OnWaterChange )

	ON_EN_CHANGE( IDC_DEGEDITTEMP, OnEnChangeEdit )
	ON_EN_CHANGE( IDC_WMEDITNETWORKPN, OnEnChangeEdit )

	ON_EN_SETFOCUS( IDC_DEGEDITTEMP, OnEnSetFocusDegassingMaxTemp )
	ON_EN_SETFOCUS( IDC_WMEDITNETWORKPN, OnEnSetFocusWaterMakeUpNetworkPN )

	ON_EN_KILLFOCUS( IDC_DEGEDITTEMP, OnKillFocusDegassingMaxTemp )
	ON_EN_KILLFOCUS( IDC_WMEDITNETWORKPN, OnKillFocusWaterMakeUpNetworkPN )
END_MESSAGE_MAP()

void CDlgIndSelPMPanelBase::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_GROUPDEGASSING, m_GroupDegassing );
	DDX_Control( pDX, IDC_DEGEDITTEMP, m_EditDegassingMaxTemp );
	DDX_Control( pDX, IDC_GROUPWATERMAKEUP, m_GroupWaterMakeUp );
	DDX_Control( pDX, IDC_WMEDITNETWORKPN, m_EditWaterMakeUpNetworkPN );
}

BOOL CDlgIndSelPMPanelBase::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	GetDlgItem( IDC_STATICDEGMAXTEMPCONNECTPOINT )->SetWindowText( TASApp.LoadLocalizedString( IDS_PM_DEGMAXTEMPCONNECT ) );
	GetDlgItem( IDC_STATICWMNETWORKPN )->SetWindowText( TASApp.LoadLocalizedString( IDS_PM_WMNETWORKPN ) );

	m_GroupDegassing.SetWindowText( TASApp.LoadLocalizedString( IDS_PM_DEGASSINGGROUPTITLE ) );
	m_GroupDegassing.SetInOffice2007Mainframe( true );
	m_GroupDegassing.SaveChildList();

	m_GroupWaterMakeUp.SetWindowText( TASApp.LoadLocalizedString( IDS_PM_WATERMAKEUPGROUPTITLE ) );
	m_GroupWaterMakeUp.SetInOffice2007Mainframe( true );
	m_GroupWaterMakeUp.SaveChildList();

	// Disable the 'OnNewDocument' handler to not interfere with this dialog.
	m_EditDegassingMaxTemp.SetOnNewDocumentHandler( false );
	m_EditWaterMakeUpNetworkPN.SetOnNewDocumentHandler( false );
	m_EditWaterMakeUpWaterTemp.SetOnNewDocumentHandler( false );

	CExtNumEdit m_EditWaterMakeUpWaterTemp;
	m_EditDegassingMaxTemp.ActiveSpecialValidation( true, this );
	m_EditWaterMakeUpNetworkPN.ActiveSpecialValidation( true, this );

	m_ToolTip.Create( this, TTS_NOPREFIX );
	m_ToolTip.SetDelayTime( TTDT_AUTOPOP, 5000 );
	InitToolTips();

	GetClientRect( &m_OrgClient );
	m_GroupDegassing.GetWindowRect( &m_OrgPosGroupDegassing );
	ScreenToClient( m_OrgPosGroupDegassing );
	
	m_GroupWaterMakeUp.GetWindowRect( &m_OrgPosGroupWaterMakeUp );
	ScreenToClient( m_OrgPosGroupWaterMakeUp );

	return TRUE;
}

BOOL CDlgIndSelPMPanelBase::PreTranslateMessage( MSG *pMsg )
{
	if( TRUE == CDlgSelectionTabHelper::OnPreTranslateMessage( pMsg ) )
	{
		return TRUE;
	}

	return CDialogExt::PreTranslateMessage( pMsg );
}

LRESULT CDlgIndSelPMPanelBase::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	GetDlgItem( IDC_STATICDEGTEMPUNIT )->SetWindowText( pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditDegassingMaxTemp, _U_TEMPERATURE );

	GetDlgItem( IDC_STATICWMNETWORKPNUNIT )->SetWindowText( pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	InitNumericalEdit( &m_EditWaterMakeUpNetworkPN, _U_PRESSURE );

	return 0;
}

void CDlgIndSelPMPanelBase::OnEnChangeEdit()
{
	if( NULL == m_pclInterface || false == m_bOnEnChangeEnabled )
	{
		return;
	}

	ClearRightSheet();
}

void CDlgIndSelPMPanelBase::OnEnSetFocusDegassingMaxTemp()
{
	m_dDegassingMaxTempSaved = m_EditDegassingMaxTemp.GetCurrentValSI();
	m_EditDegassingMaxTemp.SetSel( 0, -1 );
}

void CDlgIndSelPMPanelBase::OnEnSetFocusWaterMakeUpNetworkPN()
{
	m_dWaterMakeUpNetworkPNSaved = m_EditWaterMakeUpNetworkPN.GetCurrentValSI();
	m_EditWaterMakeUpNetworkPN.SetSel( 0, -1 );
}

void CDlgIndSelPMPanelBase::OnEnSetFocusWaterMakeUpWaterTemp()
{
	m_dWaterMakeUpTempSaved = m_EditWaterMakeUpWaterTemp.GetCurrentValSI();
	m_EditWaterMakeUpWaterTemp.SetSel( 0, -1 );
}

void CDlgIndSelPMPanelBase::OnKillFocusDegassingMaxTemp()
{
	// Do nothing if no changed.
	if( m_dDegassingMaxTempSaved == m_EditDegassingMaxTemp.GetCurrentValSI() )
	{
		return;
	}

	if( true == IsDegassingMaxTempOK() )
	{
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetDegassingMaxTempConnectPoint( m_EditDegassingMaxTemp.GetCurrentValSI() );
	}
}

void CDlgIndSelPMPanelBase::OnKillFocusWaterMakeUpNetworkPN()
{
	// Do nothing if no changed.
	if( m_dWaterMakeUpNetworkPNSaved == m_EditWaterMakeUpNetworkPN.GetCurrentValSI() )
	{
		return;
	}

	ResetToolTipsErrorBorder( &m_EditWaterMakeUpNetworkPN );
	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpNetworkPN( m_EditWaterMakeUpNetworkPN.GetCurrentValSI() );
}

void CDlgIndSelPMPanelBase::OnKillFocusWaterMakeUpWaterTemp()
{
	// Do nothing if no changed.
	if( m_dWaterMakeUpTempSaved == m_EditWaterMakeUpWaterTemp.GetCurrentValSI() )
	{
		return;
	}

	if( true == IsWaterMakeUpTempOK() )
	{
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpWaterTemp( m_EditWaterMakeUpWaterTemp.GetCurrentValSI() );
	}
}

void CDlgIndSelPMPanelBase::InitNumericalEdit( CExtNumEdit *pEdit, ePHYSTYPE phystype )
{
	if( NULL != pEdit )
	{
		pEdit->SetPhysicalType( phystype );
		pEdit->SetEditSign( CNumString::ePositive );
		pEdit->SetMinDblValue( 0.0 );

		if( _U_TEMPERATURE == phystype )
		{
			pEdit->SetEditSign( CNumString::eBoth );
			pEdit->SetMinDblValue( -273.15 );
		}

		pEdit->Update();
	}
}

void CDlgIndSelPMPanelBase::ClearRightSheet()
{
	// Clear the right sheet.
	CDlgIndSelPressureMaintenance::InterfacePMPanels *pInterface = ( CDlgIndSelPressureMaintenance::InterfacePMPanels * )
			m_pclInterface;

	DLGINDSELPMINTERFACE_CLEARALL( pInterface );

	// Invalidate select button.
	pInterface->m_pclParent->EnableSelectButton( false );
}

void CDlgIndSelPMPanelBase::ResetToolTipsErrorBorder( CWnd *pWnd, bool bToolTips, bool bErrorBorder )
{
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		// Tooltips that are informations on a field.
		// No in base.

		// Tooltips that are errors on a field.
		if( NULL == pWnd || pWnd == &m_EditDegassingMaxTemp )
		{
			if( true == bToolTips )
			{
				m_ToolTip.DelTool( &m_EditDegassingMaxTemp );
			}

			if( true == bErrorBorder )
			{
				m_EditDegassingMaxTemp.ResetDrawBorder();
			}
		}

		if( NULL == pWnd || pWnd == &m_EditWaterMakeUpNetworkPN )
		{
			if( true == bToolTips )
			{
				m_ToolTip.DelTool( &m_EditWaterMakeUpNetworkPN );
			}

			if( true == bErrorBorder )
			{
				m_EditWaterMakeUpNetworkPN.ResetDrawBorder();
			}
		}

		m_ToolTip.SetMaxTipWidth( 1000 );
	}
}

bool CDlgIndSelPMPanelBase::IsFluidTemperatureOK( CExtNumEdit *pclCurrentEdit, CString strPrefix, bool fInsertInErrorMap,
		MapErrorID eMapErrorID )
{
	if( NULL == pclCurrentEdit )
	{
		return true;
	}

	bool fFluidOK = true;
	CWaterChar *pWC = &m_pclIndSelPMParams->m_WC;
	double dCurrentTemp = pclCurrentEdit->GetCurrentValSI();
	double dSavedTemp = dCurrentTemp;
	CWaterChar::eFluidRetCode RetCode = pWC->CheckFluidData( dCurrentTemp );

	// Remark: If fluid is unknown (It means user has introduced himself density, kinetic viscosity and specific heat), we can't
	//         verify.
	if( CWaterChar::efrcFluidOk != RetCode && CWaterChar::efrcFluidUnknown != RetCode )
	{
		CString strToolTip;

		if( CWaterChar::efrcTemperatureTooLow == RetCode )
		{
			CString strFreezePtTemp = WriteCUDouble( _U_TEMPERATURE, dCurrentTemp, true, -1, 2 );
			CString strFreezingPointMsg;
			FormatString( strFreezingPointMsg, IDS_DLGINDSELPMPANELS_TTERRORFREEZINGPOINT, strFreezePtTemp );
			CString strSecondStr = CString( WriteCUDouble( _U_TEMPERATURE, dSavedTemp, true, -1, 2 ) ) + _T(" <= ") + strFreezingPointMsg;
			FormatString( strToolTip, IDS_DLGINDSELPMPANELS_TTERRORFLUIDDATATEMPTOOLOW, strPrefix, strSecondStr );
		}
		else if( CWaterChar::efrcTempTooHigh == RetCode )
		{
			CString strTemp = CString( WriteCUDouble( _U_TEMPERATURE, dSavedTemp, true, -1, 2 ) )
							  + _T(" > ") + CString( WriteCUDouble( _U_TEMPERATURE, dCurrentTemp, true, -1, 2 ) );
			FormatString( strToolTip, IDS_DLGINDSELPMPANELS_TTERRORFLUIDDATATEMPTOOHIGH, strPrefix, strTemp );
		}
		else if( CWaterChar::efrcAdditiveTooHigh == RetCode )
		{
			strToolTip = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORFLUIDDATATEMPADDTOOHIGH );
		}
		
		// HYS-838: We have to reset before adding tooltip.
		ResetToolTipsErrorBorder( pclCurrentEdit );
		m_ToolTip.AddToolWindow( pclCurrentEdit, strToolTip );
		pclCurrentEdit->SetDrawBorder( true, _RED );

		if( true == fInsertInErrorMap )
		{
			m_mapErrorMsg[eMapErrorID] = _T("\r\n - ") + strToolTip;
		}

		fFluidOK = false;
	}

	return fFluidOK;
}

bool CDlgIndSelPMPanelBase::IsDegassingMaxTempOK( bool fInsertInErrorMap )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	if( BST_UNCHECKED == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetDegassingChecked() )
	{
		return true;
	}

	m_EditDegassingMaxTemp.ResetDrawBorder();
	ResetToolTipsErrorBorder( &m_EditDegassingMaxTemp );

	CString strPrefix = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORDEGMAXTEMP );
	return IsFluidTemperatureOK( &m_EditDegassingMaxTemp, strPrefix, fInsertInErrorMap, MEID_DegassingMaxTemp );
}

bool CDlgIndSelPMPanelBase::IsWaterMakeUpTempOK( bool fInsertInErrorMap )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	if( false == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->IsWaterMakeUpExist() )
	{
		return true;
	}

	CString strPrefix = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORMAXWATERTEMP );
	return IsFluidTemperatureOK( &m_EditWaterMakeUpWaterTemp, strPrefix, fInsertInErrorMap, MEID_WaterMakeUpMaxTemp );
}

CDlgIndSelPMPanelHCSBase::CDlgIndSelPMPanelHCSBase( CIndSelPMParams *pclIndSelParams, PanelType ePanelType, UINT nID, CWnd *pParent )
	: CDlgIndSelPMPanelBase( pclIndSelParams, ePanelType, nID, pParent )
{
	m_dStaticHeightSaved = 0.0;
	m_dPzSaved = 0.0;
	m_dSystemVolumeSaved = 0.0;
	m_dInstalledPowerSaved = 0.0;
	m_dpSVLocationSaved = 0.0;
	m_dSafetyValveResponsePressureSaved = 0.0;
	m_dSupplyTempSaved = 0.0;
	m_dReturnTempSaved = 0.0;
	m_dMinTempSaved = 0.0;
	m_dFillTempSaved = 0.0;
	m_dPumpHeadSaved = 0.0;
	m_dMaxWidthSaved = 0.0;
	m_dMaxHeightSaved = 0.0;
}

void CDlgIndSelPMPanelHCSBase::SaveSelectionParameters()
{
	CDlgIndSelPMPanelBase::SaveSelectionParameters();
	CDS_IndSelParameter *pclIndSelParameter = m_pclIndSelPMParams->m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	pclIndSelParameter->SetPMPressOn( GetPressurOn() );
}

bool CDlgIndSelPMPanelHCSBase::ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser || false == CDlgIndSelPMPanelBase::ApplyPMInputUser( pclPMInputUser, bAllowShowErrorMessage ) )
	{
		return false;
	}
	
	// HYS-1054: In Heating, Cooling, Solar mode the m_EditDegassingMaxTemp stays disabled
	m_EditDegassingMaxTemp.EnableWindow( FALSE );
	m_EditStaticHeight.SetCurrentValSI( pclPMInputUser->GetStaticHeight() );
	m_EditStaticHeight.Update();

	m_BtnCheckPz.SetCheck( pclPMInputUser->GetPzChecked() );

	m_EditWaterContent.SetCurrentValSI( pclPMInputUser->GetSystemVolume() );
	m_EditWaterContent.Update();

	m_EditInstalledPower.SetCurrentValSI( pclPMInputUser->GetInstalledPower() );
	m_EditInstalledPower.Update();

	UpdateVolumeSystemStatus( false == pclPMInputUser->IfSysVolExtDefExist(), false == pclPMInputUser->IfInstPowExtDefExist() );

	// HYS-1083.
	m_BtnCheckpSVLocation.SetCheck( pclPMInputUser->GetSafetyValveLocationChecked() );
	OnBnClickedCheckpSVLocation();

	m_EditSafetyValveResponsePressure.SetCurrentValSI( pclPMInputUser->GetSafetyValveResponsePressure() );
	m_EditSafetyValveResponsePressure.Update();

	m_EditSupplyTemp.SetCurrentValSI( pclPMInputUser->GetSupplyTemperature() );
	m_EditSupplyTemp.Update();

	m_EditReturnTemp.SetCurrentValSI( pclPMInputUser->GetReturnTemperature() );
	m_EditReturnTemp.Update();

	if( Heating == pclPMInputUser->GetApplicationType() || Solar == pclPMInputUser->GetApplicationType() )
	{
		// HYS-1054: Update m_EditDegassingMaxTemp with m_EditReturnTemp
		pclPMInputUser->SetDegassingMaxTempConnectPoint( m_EditReturnTemp.GetCurrentValSI() );
		m_EditDegassingMaxTemp.SetCurrentValSI( pclPMInputUser->GetDegassingMaxTempConnectPoint() );
		m_EditDegassingMaxTemp.Update();
	}
	
	// HYS-1054: Update make-up water temperature with filling temperature
	m_EditFillTemp.SetCurrentValSI( pclPMInputUser->GetFillTemperature() );
	m_EditFillTemp.Update();
	pclPMInputUser->SetWaterMakeUpWaterTemp( m_EditFillTemp.GetCurrentValSI() );

	FillComboPressOn( pclPMInputUser->GetPressOn() );
	OnCbnSelChangePressOn();

	m_EditPumpHead.SetCurrentValSI( pclPMInputUser->GetPumpHead() );
	m_EditPumpHead.Update();

	m_EditMaxWidth.SetCurrentValSI( pclPMInputUser->GetMaxWidth() );
	m_EditMaxWidth.Update();

	m_EditMaxHeight.SetCurrentValSI( pclPMInputUser->GetMaxHeight() );
	m_EditMaxHeight.Update();

	// In this method we verify pz and PSV.
	OnBnClickedCheckPz();

	return true;
}

bool CDlgIndSelPMPanelHCSBase::VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser )
	{
		return false;
	}

	// Check the safety valve location.
	IspSVLocationOK( &m_EditpSVLocation, true );

	CString msg;
	bool bMinTempAlreadySet = false;

	ResetToolTipsErrorBorder( &m_EditPz );
	m_EditPz.ResetDrawBorder();
	
	ResetToolTipsErrorBorder( &m_EditWaterContent );
	m_EditWaterContent.ResetDrawBorder();
	
	//ResetToolTipsErrorBorder( &m_EditInstalledPower );
	//m_EditInstalledPower.ResetDrawBorder();
	
	ResetToolTipsErrorBorder( &m_EditPumpHead );
	m_EditPumpHead.ResetDrawBorder();
	
	ResetToolTipsErrorBorder( &m_EditSafetyValveResponsePressure );
	m_EditSafetyValveResponsePressure.ResetDrawBorder();

	for( std::vector<std::pair<int, int>>::iterator iterError = m_vecErrorList.begin(); iterError != m_vecErrorList.end(); iterError++ )
	{
		if( CPMInputUser::ePM_InputEmpty == iterError->first )
		{
			if( CPMInputUser::RACIE_WaterContent == iterError->second )
			{
				msg = _T("\r\n - ") + TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSYSTEMVOLUME );
				m_mapErrorMsg[MEID_WaterContent] = msg;

				ResetToolTipsErrorBorder( &m_EditWaterContent );
				msg = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSYSTEMVOLUME );
				m_ToolTip.AddToolWindow( &m_EditWaterContent, msg );
				m_EditWaterContent.SetDrawBorder( true, _RED );
				continue;
			}

			/*
			if( CPMInputUser::RACIE_InstalledPower == iterError->second )
			{
				msg = _T("\r\n - ") + TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORPOWER );
				m_mapErrorMsg[MEID_InstalledPower] = msg;

				ResetToolTipsErrorBorder( &m_EditInstalledPower );
				msg = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORPOWER );
				m_ToolTip.AddToolWindow( &m_EditInstalledPower, msg );
				m_EditInstalledPower.SetDrawBorder( true, _RED );
			}
			*/

			if( CPMInputUser::RACIE_PumpHead == iterError->second )
			{
				msg = _T("\r\n - ") + TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORPUMPHEAD );
				m_mapErrorMsg[MEID_PumpHead] = msg;

				ResetToolTipsErrorBorder( &m_EditPumpHead );
				msg = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORPUMPHEAD );
				m_ToolTip.AddToolWindow( &m_EditPumpHead, msg );
				m_EditPumpHead.SetDrawBorder( true, _RED );
			}
		}

		if( CPMInputUser::ePM_InvalidPSV == iterError->first )
		{
			// Take action depending current pressurisation maintenance type and errors.
			// We take what the user has written to avoid rounding this value with error.
			// Example: user input: 5,726; value computed: 5,73 -> If we use 'WriteCUDouble' for the use inpur, we will have 
			//          in the error message "5,73 < 5,73".
			TCHAR tcPSVInput[256];
			m_EditSafetyValveResponsePressure.GetWindowText( tcPSVInput, 256 );

			CString strPSVInput = tcPSVInput;
			CUnitDatabase *pclUnitDB = CDimValue::AccessUDB();
			strPSVInput += _T(" ");
			strPSVInput += CString( GetNameOf( pclUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );

			double dPSVLimit = 0.0;
			pclPMInputUser->CheckPSV( &dPSVLimit );
			CString strPSVLimit = WriteCUDouble( _U_PRESSURE, dPSVLimit, true, -1, 2 );

			CString msgPart2;
			FormatString( msgPart2, IDS_DLGINDSELPMPANELS_TTERRORPSV, strPSVInput, strPSVLimit );
			msg = _T("\r\n - ") + msgPart2;
			m_mapErrorMsg[MEID_SafetyPressureValve] = msg;

			ResetToolTipsErrorBorder( &m_EditSafetyValveResponsePressure );
			m_ToolTip.AddToolWindow( &m_EditSafetyValveResponsePressure, msgPart2 );
			m_EditSafetyValveResponsePressure.SetDrawBorder( true, _RED );
			continue;
		}
	}

	return CDlgIndSelPMPanelBase::VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

void CDlgIndSelPMPanelHCSBase::SetPz( double dPz )
{
	m_EditPz.SetCurrentValSI( dPz );
	m_EditPz.Update();
}

void CDlgIndSelPMPanelHCSBase::SetPressurOn( PressurON ePressurON )
{
	int iSel = m_ComboPressOn.FindItemDataPtr( (int)ePressurON );

	if( iSel >= 0 )
	{
		m_ComboPressOn.SetCurSel( iSel );

		if( PressurON::poPumpSuction == GetPressurOn() )
		{
			m_EditPumpHead.EnableWindow( FALSE );
			m_EditPumpHead.ResetDrawBorder();
			m_EditPz.EnableWindow( FALSE );
		}
		else
		{
			m_EditPumpHead.EnableWindow( TRUE );
			m_EditPz.EnableWindow( TRUE );
		}
	}
}

void CDlgIndSelPMPanelHCSBase::SetPumpHead( double dPumpHead )
{
	m_EditPumpHead.SetCurrentValSI( dPumpHead );
	m_EditPumpHead.Update();
}

void CDlgIndSelPMPanelHCSBase::SetMaxWidth( double dMaxWidth )
{
	m_EditMaxWidth.SetCurrentValSI( dMaxWidth );
	m_EditMaxWidth.Update();
}

void CDlgIndSelPMPanelHCSBase::SetMaxHeight( double dMaxHeight )
{
	m_EditMaxHeight.SetCurrentValSI( dMaxHeight );
	m_EditMaxHeight.Update();
}

PressurON CDlgIndSelPMPanelHCSBase::GetPressurOn( void )
{
	PressurON ePressurON = PressurON::poPumpSuction;		// By default in case of error
	int iCur = m_ComboPressOn.GetCurSel();

	if( iCur >= 0 )
	{
        ePressurON = (PressurON)m_ComboPressOn.GetItemData( iCur );

		if( ePressurON < PressurON::poPumpSuction || ePressurON >= PressurON::poLast )
		{
			ASSERT( 0 );
			ePressurON = PressurON::poPumpSuction;
		}
	}

	return ePressurON;
}

void CDlgIndSelPMPanelHCSBase::OnPMTypeChanged( CPMInputUser *pclPMInputUser )
{
	CDlgIndSelPMPanelBase::OnPMTypeChanged( pclPMInputUser );

	// Pressure maintenance type has an influence on the PSV.
	CheckSafetyVRP();
}

void CDlgIndSelPMPanelHCSBase::OnNewDocument()
{
	CDlgIndSelPMPanelBase::OnNewDocument();

	// To avoid any 'OnEnChange' event to be called.
	m_bOnEnChangeEnabled = false;

	// Don't call 'OnUnitChange' because in this method we call a 'FillPMInputUser' and 'ApplyPMInputUser' and this will change status of some
	// control that we don't need when 'OnNewDocument' is called.
	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();
	GetDlgItem( IDC_STATIC_ISTATICHEIGHTUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str() );
	InitNumericalEdit( &m_EditStaticHeight, _U_LENGTH );

	UpdateP0();

	GetDlgItem( IDC_STATIC_PZUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	InitNumericalEdit( &m_EditPz, _U_PRESSURE );

	GetDlgItem( IDC_STATIC_WATERCONTENTUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_EditWaterContent, _U_VOLUME );

	GetDlgItem( IDC_STATIC_INSTALLPOWERUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TH_POWER ).c_str() );
	InitNumericalEdit( &m_EditInstalledPower, _U_TH_POWER );

	UpdateVolumeSystemStatus( TRUE, TRUE );

	// HYS-1083.
	GetDlgItem( IDC_STATIC_STATICPSVLOCATIONUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str() );
	InitNumericalEdit( &m_EditpSVLocation, _U_NODIM );
	m_EditpSVLocation.SetEditSign( CNumString::eBoth );
	m_EditpSVLocation.SetMinDblValue( -DBL_MAX );

	GetDlgItem( IDC_STATIC_STATICSVRPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	InitNumericalEdit( &m_EditSafetyValveResponsePressure, _U_PRESSURE );

	GetDlgItem( IDC_STATIC_SUPPLYTEMPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditSupplyTemp, _U_TEMPERATURE );

	GetDlgItem( IDC_STATIC_RETURNTEMPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditReturnTemp, _U_TEMPERATURE );

	GetDlgItem( IDC_STATIC_MINTEMPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditMinTemp, _U_TEMPERATURE );

	GetDlgItem( IDC_STATIC_FILLTEMPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditFillTemp, _U_TEMPERATURE );

	GetDlgItem( IDC_STATIC_PUMPHEADUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_DIFFPRESS ).c_str() );
	InitNumericalEdit( &m_EditPumpHead, _U_DIFFPRESS );

	GetDlgItem( IDC_STATIC_MAXWIDTHUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str() );
	InitNumericalEdit( &m_EditMaxWidth, _U_LENGTH );

	GetDlgItem( IDC_STATIC_MAXHEIGHTUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str() );
	InitNumericalEdit( &m_EditMaxHeight, _U_LENGTH );

	m_bOnEnChangeEnabled = true;
}

void CDlgIndSelPMPanelHCSBase::EnablePMSelection( bool Enable )
{
	CDlgIndSelPMPanelBase::EnablePMSelection( Enable );
	m_EditStaticHeight.ActiveSpecialValidation( Enable, this );
	m_EditPz.ActiveSpecialValidation( Enable, this );
	m_EditWaterContent.ActiveSpecialValidation( Enable, this );
	m_EditInstalledPower.ActiveSpecialValidation( Enable, this );
	m_EditpSVLocation.ActiveSpecialValidation( Enable, this );		// HYS-1083.
	m_EditSafetyValveResponsePressure.ActiveSpecialValidation( Enable, this );
	m_EditSupplyTemp.ActiveSpecialValidation( Enable, this );
	m_EditReturnTemp.ActiveSpecialValidation( Enable, this );
	m_EditMinTemp.ActiveSpecialValidation( Enable, this );
	m_EditFillTemp.ActiveSpecialValidation( Enable, this );
	m_EditPumpHead.ActiveSpecialValidation( Enable, this );
	m_EditMaxWidth.ActiveSpecialValidation( Enable, this );
	m_EditMaxHeight.ActiveSpecialValidation( Enable, this );
	Invalidate();
	UpdateWindow();
}

BEGIN_MESSAGE_MAP( CDlgIndSelPMPanelHCSBase, CDlgIndSelPMPanelBase )
	ON_CBN_SELCHANGE( IDC_COMBO_PRESSON, OnCbnSelChangePressOn )

	ON_EN_CHANGE( IDC_EDIT_STATICHEIGHT, OnEnChangeStaticHeight )
	ON_EN_CHANGE( IDC_EDIT_PZ, OnEnChangeEdit )
	ON_EN_CHANGE( IDC_EDIT_WATERCONTENT, OnEnChangeEdit )
	ON_EN_CHANGE( IDC_EDIT_INSTALLPOWER, OnEnChangeInstalledPower )
	ON_EN_CHANGE( IDC_EDIT_PSVLOCATION, OnEnChangeEdit )						// HYS-1083.
	ON_EN_CHANGE( IDC_EDIT_SAFETYVRP, OnEnChangeEdit )	
	ON_EN_CHANGE( IDC_EDIT_SUPPLYTEMP, OnEnChangeEdit )
	ON_EN_CHANGE( IDC_EDIT_RETURNTEMP, OnEnChangeEdit )
	ON_EN_CHANGE( IDC_EDIT_MINTEMP, OnEnChangeEdit )
	ON_EN_CHANGE( IDC_EDIT_FILLTEMP, OnEnChangeEdit )
	ON_EN_CHANGE( IDC_EDIT_PUMPHEAD, OnEnChangePumpHead )
	ON_EN_CHANGE( IDC_EDIT_MAXWIDTH, OnEnChangeEdit )
	ON_EN_CHANGE( IDC_EDIT_MAXHEIGHT, OnEnChangeEdit )

	ON_EN_SETFOCUS( IDC_EDIT_STATICHEIGHT, OnEnSetFocusStaticHeight )
	ON_EN_SETFOCUS( IDC_EDIT_PZ, OnEnSetFocusPz )
	ON_EN_SETFOCUS( IDC_EDIT_WATERCONTENT, OnEnSetFocusSystemVolume )
	ON_EN_SETFOCUS( IDC_EDIT_INSTALLPOWER, OnEnSetFocusInstalledPower )
	ON_EN_SETFOCUS( IDC_EDIT_PSVLOCATION, OnEnSetFocuspSVLocation )				// HYS-1083.
	ON_EN_SETFOCUS( IDC_EDIT_SAFETYVRP, OnEnSetFocusSafetyVRP )
	ON_EN_SETFOCUS( IDC_EDIT_SUPPLYTEMP, OnEnSetFocusSupplyTemp )
	ON_EN_SETFOCUS( IDC_EDIT_RETURNTEMP, OnEnSetFocusReturnTemp )
	ON_EN_SETFOCUS( IDC_EDIT_MINTEMP, OnEnSetFocusMinTemp )
	ON_EN_SETFOCUS( IDC_EDIT_FILLTEMP, OnEnSetFocusFillTemp )
	ON_EN_SETFOCUS( IDC_EDIT_PUMPHEAD, OnEnSetFocusPumpHead )
	ON_EN_SETFOCUS( IDC_EDIT_MAXWIDTH, OnEnSetFocusMaxWidth )
	ON_EN_SETFOCUS( IDC_EDIT_MAXHEIGHT, OnEnSetFocusMaxHeight )

	ON_EN_KILLFOCUS( IDC_EDIT_STATICHEIGHT, OnKillFocusStaticHeight )
	ON_EN_KILLFOCUS( IDC_EDIT_PZ, OnKillFocusPz )
	ON_EN_KILLFOCUS( IDC_EDIT_WATERCONTENT, OnKillFocusSystemVolume )
	ON_EN_KILLFOCUS( IDC_EDIT_INSTALLPOWER, OnKillFocusInstalledPower )
	ON_EN_KILLFOCUS( IDC_EDIT_PSVLOCATION, OnKillFocuspSVLocation )				// HYS-1083.
	ON_EN_KILLFOCUS( IDC_EDIT_SAFETYVRP, OnKillFocusSafetyVRP )
	ON_EN_KILLFOCUS( IDC_EDIT_PUMPHEAD, OnKillFocusPumpHead )
	ON_EN_KILLFOCUS(IDC_EDIT_MAXWIDTH, OnKillFocusMaxWidth )
	ON_EN_KILLFOCUS(IDC_EDIT_MAXHEIGHT, OnKillFocusMaxHeight )

	ON_BN_CLICKED( IDC_CHECKPZ, OnBnClickedCheckPz )
	ON_BN_CLICKED( IDC_CHECKPSVLOCATION, OnBnClickedCheckpSVLocation )			// HYS-1083.
END_MESSAGE_MAP()

void CDlgIndSelPMPanelHCSBase::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelPMPanelBase::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_STATIC_STATICHEIGHT, m_StaticStaticHeight );
	DDX_Control( pDX, IDC_STATIC_P0, m_StaticP0 );
	DDX_Control( pDX, IDC_STATIC_PZ, m_StaticPz);
	DDX_Control( pDX, IDC_STATIC_WATERCONTENT, m_StaticSystemVolume );
	DDX_Control( pDX, IDC_STATIC_INSTALLPOWER, m_StaticInstalledPower );
	DDX_Control( pDX, IDC_STATIC_PSVLOCATION, m_StaticpSVLocation );						// HYS-1083.
	DDX_Control( pDX, IDC_STATIC_SAFETYRESPVALVPRESS, m_StaticSafetyValveResponsePressure );
	DDX_Control( pDX, IDC_STATIC_MINIMUMPSV, m_StaticMinPSV );								// HYS-1083.
	DDX_Control( pDX, IDC_STATIC_SUPPLYTEMP, m_StaticSupplyTemp );
	DDX_Control( pDX, IDC_STATIC_RETURNTEMP, m_StaticReturnTemp );
	DDX_Control( pDX, IDC_STATIC_MINTEMP, m_StaticMinTemp );
	DDX_Control( pDX, IDC_STATIC_FILLTEMP, m_StaticFillTemp );
	DDX_Control( pDX, IDC_STATIC_PRESSUON, m_StaticPressureOn );
	DDX_Control( pDX, IDC_STATIC_PUMPHEAD, m_StaticPumpHead );
	DDX_Control( pDX, IDC_STATIC_MAXWIDTH, m_StaticMaxWidth);
	DDX_Control( pDX, IDC_STATIC_MAXHEIGHT, m_StaticMaxHeight );
	DDX_Control( pDX, IDC_BUTTON_WATERCONTENT, m_BtnWaterContent );
	DDX_Control( pDX, IDC_CHECKPZ, m_BtnCheckPz );
	DDX_Control( pDX, IDC_EDIT_PZ, m_EditPz );
	DDX_Control( pDX, IDC_EDIT_STATICHEIGHT, m_EditStaticHeight );
	DDX_Control( pDX, IDC_EDIT_WATERCONTENT, m_EditWaterContent );
	DDX_Control( pDX, IDC_EDIT_INSTALLPOWER, m_EditInstalledPower );
	DDX_Control( pDX, IDC_CHECKPSVLOCATION, m_BtnCheckpSVLocation );						// HYS-1083.
	DDX_Control( pDX, IDC_EDIT_PSVLOCATION, m_EditpSVLocation );							// HYS-1083.
	DDX_Control( pDX, IDC_EDIT_SAFETYVRP, m_EditSafetyValveResponsePressure );
	DDX_Control( pDX, IDC_EDIT_SUPPLYTEMP, m_EditSupplyTemp );
	DDX_Control( pDX, IDC_EDIT_RETURNTEMP, m_EditReturnTemp );
	DDX_Control( pDX, IDC_EDIT_MINTEMP, m_EditMinTemp );
	DDX_Control( pDX, IDC_EDIT_FILLTEMP, m_EditFillTemp );
	DDX_Control( pDX, IDC_COMBO_PRESSON, m_ComboPressOn );
	DDX_Control( pDX, IDC_EDIT_PUMPHEAD, m_EditPumpHead );
	DDX_Control( pDX, IDC_EDIT_MAXWIDTH, m_EditMaxWidth );
	DDX_Control( pDX, IDC_EDIT_MAXHEIGHT, m_EditMaxHeight );
}

BOOL CDlgIndSelPMPanelHCSBase::OnInitDialog()
{
	CDlgIndSelPMPanelBase::OnInitDialog();

	m_StaticStaticHeight.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_STATICHEIGHT ) );
	m_StaticPz.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_PZ ), TASApp.LoadLocalizedString( IDS_PM_TTPZ ) );
	m_StaticSystemVolume.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_WATERCONTENT ) );
	m_StaticInstalledPower.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_INSTALLPOWER ) );
	m_StaticpSVLocation.SetWindowTextW( TASApp.LoadLocalizedString( IDS_PM_PSVLOCATION ) );			// HYS-1083.
	m_StaticpSVLocation.SetToolTip( TASApp.LoadLocalizedString( IDS_PM_TTPSVLOCATION ) );			// HYS-1083.
	m_StaticSafetyValveResponsePressure.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_SAFETYRESPVALVPRESS ) );
	m_StaticSupplyTemp.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_SUPPLYTEMP ) );
	m_StaticReturnTemp.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_RETURNTEMP ) );
	m_StaticMinTemp.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_MINTEMP ) );
	m_StaticFillTemp.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_FILLTEMP ) );
	m_StaticPressureOn.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_PRESSUON ) );
	m_StaticPumpHead.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_PUMPHEAD ) );
	m_StaticMaxWidth.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_MAXWIDTH ) );
	m_StaticMaxHeight.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_MAXHEIGHT ) );

	// Disable the 'OnNewDocument' handler to not interfere with this dialog.
	m_EditStaticHeight.SetOnNewDocumentHandler( false );
	m_EditPz.SetOnNewDocumentHandler( false );
	m_EditWaterContent.SetOnNewDocumentHandler( false );
	m_EditInstalledPower.SetOnNewDocumentHandler( false );
	m_EditSafetyValveResponsePressure.SetOnNewDocumentHandler( false );
	m_EditpSVLocation.SetOnNewDocumentHandler( false );
	m_EditSupplyTemp.SetOnNewDocumentHandler( false );
	m_EditReturnTemp.SetOnNewDocumentHandler( false );
	m_EditMinTemp.SetOnNewDocumentHandler( false );
	m_EditFillTemp.SetOnNewDocumentHandler( false );
	m_EditPumpHead.SetOnNewDocumentHandler( false );
	m_EditMaxWidth.SetOnNewDocumentHandler( false );
	m_EditMaxHeight.SetOnNewDocumentHandler( false );

	m_EditStaticHeight.ActiveSpecialValidation( true, this );
	m_EditPz.ActiveSpecialValidation( true, this );
	m_EditWaterContent.ActiveSpecialValidation( true, this );
	m_EditInstalledPower.ActiveSpecialValidation( true, this );
	m_EditSafetyValveResponsePressure.ActiveSpecialValidation( true, this );
	m_EditpSVLocation.ActiveSpecialValidation( true, this );					// HYS-1083.
	m_EditSupplyTemp.ActiveSpecialValidation( true, this );
	m_EditReturnTemp.ActiveSpecialValidation( true, this );
	m_EditMinTemp.ActiveSpecialValidation( true, this );
	m_EditFillTemp.ActiveSpecialValidation( true, this );
	m_EditPumpHead.ActiveSpecialValidation( true, this );
	m_EditMaxWidth.ActiveSpecialValidation( true, this );
	m_EditMaxHeight.ActiveSpecialValidation( true, this );

	// HYS-1083: Distance between the vessel and the safety pressure valve.\r\nSet negative value if pSV is placed lower than the vessel or positive if higher.
	CString TTstr = TASApp.LoadLocalizedString( IDS_PM_TTPSVLOCATION );
	m_ToolTip.AddToolWindow( &m_BtnCheckpSVLocation, TTstr );

	return TRUE;
}

LRESULT CDlgIndSelPMPanelHCSBase::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList )
	{
		// Not yet ready!
		return 0;
	}

	CDlgIndSelPMPanelBase::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();
	GetDlgItem( IDC_STATIC_ISTATICHEIGHTUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str() );
	InitNumericalEdit( &m_EditStaticHeight, _U_LENGTH );

	GetDlgItem( IDC_STATIC_PZUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	InitNumericalEdit( &m_EditPz, _U_PRESSURE );

	GetDlgItem( IDC_STATIC_WATERCONTENTUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_EditWaterContent, _U_VOLUME );

	GetDlgItem( IDC_STATIC_INSTALLPOWERUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TH_POWER ).c_str() );
	InitNumericalEdit( &m_EditInstalledPower, _U_TH_POWER );
	
	// HYS-1083.
	GetDlgItem( IDC_STATIC_STATICPSVLOCATIONUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str() );
	InitNumericalEdit( &m_EditpSVLocation, _U_LENGTH );
	m_EditpSVLocation.SetEditSign( CNumString::eBoth );

	GetDlgItem( IDC_STATIC_STATICSVRPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	InitNumericalEdit( &m_EditSafetyValveResponsePressure, _U_PRESSURE );

	GetDlgItem( IDC_STATIC_SUPPLYTEMPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditSupplyTemp, _U_TEMPERATURE );

	GetDlgItem( IDC_STATIC_RETURNTEMPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditReturnTemp, _U_TEMPERATURE );

	GetDlgItem( IDC_STATIC_MINTEMPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditMinTemp, _U_TEMPERATURE );

	GetDlgItem( IDC_STATIC_FILLTEMPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditFillTemp, _U_TEMPERATURE );

	GetDlgItem( IDC_STATIC_PUMPHEADUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_DIFFPRESS ).c_str() );
	InitNumericalEdit( &m_EditPumpHead, _U_DIFFPRESS );

	GetDlgItem( IDC_STATIC_MAXWIDTHUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str() );
	InitNumericalEdit( &m_EditMaxWidth, _U_LENGTH );

	GetDlgItem( IDC_STATIC_MAXHEIGHTUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str() );
	InitNumericalEdit( &m_EditMaxHeight, _U_LENGTH );

	UpdateP0();
	CheckPz();
	CheckSafetyVRP();

	return 0;
}

LRESULT CDlgIndSelPMPanelHCSBase::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( (WMUserWaterCharWParam)wParam & WM_UWC_WP_ForProductSel )
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChange == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		// Not yet ready!
		return 0;
	}

	CDlgIndSelPMPanelBase::OnWaterChange();

	UpdateP0();

	// Density of the media has an influence on p0. Thus we need to check pz input by user.
	CheckPz();
	
	// Density of the media has an influence on the PSV.
	CheckSafetyVRP();

	return 0;
}

void CDlgIndSelPMPanelHCSBase::OnCbnSelChangePressOn()
{
	if( PressurON::poPumpSuction == GetPressurOn() )
	{
		m_EditPumpHead.EnableWindow( FALSE );
		m_EditPumpHead.ResetDrawBorder();
	}
	else
	{
		m_EditPumpHead.EnableWindow( TRUE );
	}

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPressOn( GetPressurOn() );

	UpdateP0();

	// Clear the right sheet.
	if( GetFocus() == &m_ComboPressOn )
	{
		ClearRightSheet();
	}

	// Pump discharge allow to the user to input a pump head. Pump head has an influence on p0. Thus we need to check pz input by user.
	CheckPz();

	// And pump head has also an influence on the PSV.
	CheckSafetyVRP();
}

void CDlgIndSelPMPanelHCSBase::OnEnSetFocusStaticHeight()
{
	m_EditStaticHeight.SetSel( 0, -1 );
	m_dStaticHeightSaved = m_EditStaticHeight.GetCurrentValSI();
}

void CDlgIndSelPMPanelHCSBase::OnEnSetFocusPz()
{
	m_EditPz.SetSel( 0, -1 );
	m_dPzSaved = m_EditPz.GetCurrentValSI();
}

void CDlgIndSelPMPanelHCSBase::OnEnSetFocusSystemVolume()
{
	m_EditWaterContent.SetSel( 0, -1 );
	m_dSystemVolumeSaved = m_EditWaterContent.GetCurrentValSI();
}

void CDlgIndSelPMPanelHCSBase::OnEnSetFocusInstalledPower()
{
	m_EditInstalledPower.SetSel( 0, -1 );
	m_dInstalledPowerSaved = m_EditInstalledPower.GetCurrentValSI();
}

// HYS-1083.
void CDlgIndSelPMPanelHCSBase::OnEnSetFocuspSVLocation()
{
	m_EditpSVLocation.SetSel( 0, -1 );
	m_dpSVLocationSaved = m_EditpSVLocation.GetCurrentValSI();
}

void CDlgIndSelPMPanelHCSBase::OnEnSetFocusSafetyVRP()
{
	m_EditSafetyValveResponsePressure.SetSel( 0, -1 );
	m_dSafetyValveResponsePressureSaved = m_EditSafetyValveResponsePressure.GetCurrentValSI();
}

void CDlgIndSelPMPanelHCSBase::OnEnSetFocusSupplyTemp()
{
	m_EditSupplyTemp.SetSel( 0, -1 );
	m_dSupplyTempSaved = m_EditSupplyTemp.GetCurrentValSI();
}

void CDlgIndSelPMPanelHCSBase::OnEnSetFocusReturnTemp()
{
	m_EditReturnTemp.SetSel( 0, -1 );
	m_dReturnTempSaved = m_EditReturnTemp.GetCurrentValSI();
}

void CDlgIndSelPMPanelHCSBase::OnEnSetFocusMinTemp()
{
	m_EditMinTemp.SetSel( 0, -1 );
	m_dMinTempSaved = m_EditMinTemp.GetCurrentValSI();
}

void CDlgIndSelPMPanelHCSBase::OnEnSetFocusFillTemp()
{
	m_EditFillTemp.SetSel( 0, -1 );
	m_dFillTempSaved = m_EditFillTemp.GetCurrentValSI();
}

void CDlgIndSelPMPanelHCSBase::OnEnSetFocusPumpHead()
{
	m_EditPumpHead.SetSel( 0, -1 );
	m_dPumpHeadSaved = m_EditPumpHead.GetCurrentValSI();
}

void CDlgIndSelPMPanelHCSBase::OnEnSetFocusMaxWidth()
{
	m_EditMaxWidth.SetSel( 0, -1 );
	m_dMaxWidthSaved = m_EditMaxWidth.GetCurrentValSI();
}

void CDlgIndSelPMPanelHCSBase::OnEnSetFocusMaxHeight()
{
	m_EditMaxHeight.SetSel( 0, -1 );
	m_dMaxHeightSaved = m_EditMaxHeight.GetCurrentValSI();
}

void CDlgIndSelPMPanelHCSBase::OnEnChangeStaticHeight()
{
	// HYS-829: value is not changed if the window is not visible for the current mode 
	if( false == m_bOnEnChangeEnabled  || false == IsWindowVisible() )
	{
		return;
	}

	CDlgIndSelPMPanelBase::OnEnChangeEdit();
	
	// HYS 829: If the update concern another application type don't modify the current Height
	if( m_pclIndSelPMParams->m_eApplicationType == GetPanelType() )
	{
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetStaticHeight( m_EditStaticHeight.GetCurrentValSI() );
	}
	
	UpdateP0();
}

void CDlgIndSelPMPanelHCSBase::OnEnChangeInstalledPower()
{
	// HYS-829: value is not changed if the window is not visible for the current mode 
	if( false == m_bOnEnChangeEnabled || false == IsWindowVisible() )
	{
		return;
	}

	CDlgIndSelPMPanelBase::OnEnChangeEdit();

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetInstalledPower( m_EditInstalledPower.GetCurrentValSI() );
	UpdateP0();
}

void CDlgIndSelPMPanelHCSBase::OnEnChangePumpHead()
{
	// HYS-829: value is not changed if the window is not visible for the current mode 
	if( false == m_bOnEnChangeEnabled || false == IsWindowVisible() )
	{
		return;
	}

	CDlgIndSelPMPanelBase::OnEnChangeEdit();

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPumpHead( m_EditPumpHead.GetCurrentValSI() );
	UpdateP0();
}

void CDlgIndSelPMPanelHCSBase::OnKillFocusStaticHeight()
{
	double dStaticHeight = m_EditStaticHeight.GetCurrentValSI();

	// Do nothing if no change.
	if( m_dStaticHeightSaved == dStaticHeight )
	{
		return;
	}

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetStaticHeight( dStaticHeight );
	// Static height has an influence on p0. Thus we need to check pz input by user.
	CheckPz();

	// Static height has an influence on the PSV.
	CheckSafetyVRP();
	// static height has an influence on Savety valve location error detection.
	IspSVLocationOK( &m_EditpSVLocation );
}

void CDlgIndSelPMPanelHCSBase::OnKillFocusPz()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList ||
			NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	double dPz = m_EditPz.GetCurrentValSI();

	// Do nothing if no change.
	if( m_dPzSaved == dPz )
	{
		return;
	}

	if( true == CheckPz() )
	{
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPz( dPz );
		UpdateP0();

		// pz has an influence on the PSV.
		CheckSafetyVRP();
	}
	else
	{
		// We take minimum pressure without taking account pz to be able to show the correct p0 when there is an error.
		double dP0 = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumPressure( false );

		CString str;
		FormatString( str, IDS_DLGINDSELPMPANELS_P0, WriteCUDouble( _U_PRESSURE, dP0, true ) );
		m_StaticP0.SetWindowText( str );
	}
}

void CDlgIndSelPMPanelHCSBase::OnKillFocusSystemVolume()
{
	// Do nothing if no change.
	if( m_dSystemVolumeSaved == m_EditWaterContent.GetCurrentValSI() )
	{
		return;
	}

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSystemVolume( m_EditWaterContent.GetCurrentValSI() );
}

void CDlgIndSelPMPanelHCSBase::OnKillFocusInstalledPower()
{
	// Do nothing if no change.
	if( m_dInstalledPowerSaved == m_EditInstalledPower.GetCurrentValSI() )
	{
		return;
	}

	ResetToolTipsErrorBorder( &m_EditInstalledPower );

	// Installed power has an influence on the PSV.
	CheckSafetyVRP();

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetInstalledPower( m_EditInstalledPower.GetCurrentValSI() );
}

// HYS-1083.
void CDlgIndSelPMPanelHCSBase::OnKillFocuspSVLocation()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Do nothing if no change.
	if( m_dpSVLocationSaved == m_EditpSVLocation.GetCurrentValSI() )
	{
		return;
	}

	UINT uiError = IspSVLocationOK( &m_EditpSVLocation );

	if( MEID_SupplyTemperature != ( MEID_SupplyTemperature & uiError ) )
	{
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSafetyValveLocation( m_EditpSVLocation.GetCurrentValSI() );
	}
}

void CDlgIndSelPMPanelHCSBase::OnKillFocusSafetyVRP()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList ||
			NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	double dSafetyVRP = m_EditSafetyValveResponsePressure.GetCurrentValSI();

	// Do nothing if no change.
	if( m_dSafetyValveResponsePressureSaved == dSafetyVRP )
	{
		return;
	}

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSafetyValveResponsePressure( dSafetyVRP );
	CheckSafetyVRP();
}

void CDlgIndSelPMPanelHCSBase::OnKillFocusPumpHead()
{
	// Do nothing if no change.
	if( m_dPumpHeadSaved == m_EditPumpHead.GetCurrentValSI() )
	{
		return;
	}

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPumpHead( m_EditPumpHead.GetCurrentValSI() );

	UpdateP0();

	// Pump head has an influence on p0. Thus we need to check pz input by user.
	CheckPz();

	// Pump head has an influence on the PSV.
	CheckSafetyVRP();
}

void CDlgIndSelPMPanelHCSBase::OnKillFocusMaxWidth()
{
	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetMaxWidth( m_EditMaxWidth.GetCurrentValSI() );
}

void CDlgIndSelPMPanelHCSBase::OnKillFocusMaxHeight()
{
	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetMaxHeight( m_EditMaxHeight.GetCurrentValSI() );
}

void CDlgIndSelPMPanelHCSBase::OnBnClickedCheckPz()
{
	ResetToolTipsErrorBorder( &m_EditPz );

	int iCheck = m_BtnCheckPz.GetCheck();
	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPzChecked( iCheck );

	m_EditPz.EnableWindow( ( BST_CHECKED == iCheck ) ? TRUE : FALSE );
	GetDlgItem( IDC_STATIC_PZUNIT )->EnableWindow( ( BST_CHECKED == iCheck ) ? TRUE : FALSE );

	if( BST_CHECKED == iCheck )
	{
		m_EditPz.SetEmpty( false );

		// By default, we set the minimum pressure.
		m_EditPz.SetCurrentValSI( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumPressure() );
		m_EditPz.Update();
		InitToolTips( &m_EditPz );
	}
	else
	{
		m_EditPz.SetCurrentValSI( 0.0 );
		m_EditPz.SetEmpty( true );
	}

	UpdateP0();
	CheckPz();
	CheckSafetyVRP();
}

void CDlgIndSelPMPanelHCSBase::OnBnClickedCheckpSVLocation()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		return;
	}

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSafetyValveLocationChecked( ( BST_CHECKED == m_BtnCheckpSVLocation.GetCheck() ) ? true : false );

	if( BST_CHECKED == m_BtnCheckpSVLocation.GetCheck() )
	{
		m_EditpSVLocation.EnableWindow( TRUE );
		m_EditpSVLocation.SetCurrentValSI( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSafetyValveLocation() );
		m_EditpSVLocation.Update();
	}
	else
	{
		// Reset error when unchecked
		ResetToolTipsErrorBorder( &m_EditpSVLocation );
		m_EditpSVLocation.EnableWindow( FALSE );
		m_EditpSVLocation.SetCurrentValSI( 0.0 );
		m_EditpSVLocation.Update();

		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSafetyValveLocation( 0.0 );
	}
}

void CDlgIndSelPMPanelHCSBase::ResetToolTipsErrorBorder(CWnd *pWnd, bool fToolTips, bool fErrorBorder)
{
	CExtNumEdit *pHelper[] = { &m_EditMaxHeight, &m_EditMaxWidth, &m_EditPz, &m_EditMinTemp, &m_EditInstalledPower, &m_EditWaterContent, 
			&m_EditSafetyValveResponsePressure, &m_EditpSVLocation, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditFillTemp, NULL };

	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		int iLoop = 0;
		CExtNumEdit *pNextEdit = ( NULL != (CExtNumEdit *)pWnd ) ? (CExtNumEdit *)pWnd : pHelper[iLoop];

		do
		{
			if( NULL == pWnd || pWnd == pNextEdit )
			{
				if( true == fToolTips )
				{
					m_ToolTip.DelTool( pNextEdit );
				}

				if( true == fErrorBorder )
				{
					pNextEdit->ResetDrawBorder();
				}
			}

			pNextEdit = ( NULL != pWnd ) ? NULL : pHelper[++iLoop];
		}
		while( NULL != pNextEdit );
	}

	CDlgIndSelPMPanelBase::ResetToolTipsErrorBorder( pWnd );
}

void CDlgIndSelPMPanelHCSBase::InitToolTips( CWnd *pWnd )
{
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		CString TTstr;

		if( NULL == pWnd || pWnd == &m_EditPz )
		{
			ResetToolTipsErrorBorder( &m_EditPz );
			TTstr = TASApp.LoadLocalizedString( IDS_PM_TTPZ );
			m_ToolTip.AddToolWindow( &m_EditPz, TTstr );
		}

		if( NULL == pWnd || pWnd == &m_EditMaxHeight )
		{
			ResetToolTipsErrorBorder( &m_EditMaxHeight );
			TTstr = TASApp.LoadLocalizedString( IDS_PM_TTMAXHEIGHT );
			m_ToolTip.AddToolWindow( &m_EditMaxHeight, TTstr );
		}

		if( NULL == pWnd || pWnd == &m_EditMaxWidth )
		{
			ResetToolTipsErrorBorder( &m_EditMaxWidth );
			TTstr = TASApp.LoadLocalizedString( IDS_PM_TTMAXDIAMETER );
			m_ToolTip.AddToolWindow( &m_EditMaxWidth, TTstr );
		}

		if( NULL == pWnd || pWnd == &m_EditMinTemp )
		{
			TTstr = TASApp.LoadLocalizedString( IDS_PM_TTMINTEMP );
			ResetToolTipsErrorBorder( &m_EditMinTemp );
			CWaterChar *pWC = &m_pclIndSelPMParams->m_WC;
			ASSERT( NULL != pWC );

			if( NULL == pWC )
			{
				return;
			}

			double dFreezeTemp = pWC->GetTfreez();
			CString str;
			FormatString( str, IDS_DLGINDSELPMPANELS_FREEZINGPOINT, WriteCUDouble( _U_TEMPERATURE, dFreezeTemp, true ) );
			TTstr = TTstr +  CString( L"\r\n" ) + str;

			m_ToolTip.AddToolWindow( &m_EditMinTemp, TTstr );
		}
	}

	CDlgIndSelPMPanelBase::InitToolTips( pWnd );
}

void CDlgIndSelPMPanelHCSBase::UpdateP0()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	double dP0 = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumPressure();

	CString str;
	FormatString( str, IDS_DLGINDSELPMPANELS_P0, WriteCUDouble( _U_PRESSURE, dP0, true ) );
	m_StaticP0.SetWindowText( str );
	m_StaticP0.Invalidate();
	m_StaticP0.UpdateWindow();
}

bool CDlgIndSelPMPanelHCSBase::CheckPz()
{
	if( BST_UNCHECKED == m_BtnCheckPz.GetCheck() )
	{
		return false;
	}

	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList ||
			NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	ResetToolTipsErrorBorder( &m_EditPz );

	bool bReturn = true;
	double dPz = m_EditPz.GetCurrentValSI();
	double dP0 = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumPressure( false );

	if( dPz < dP0 )
	{
		CString strPz = WriteCUDouble( _U_PRESSURE, dPz, false );
		CString strP0 = WriteCUDouble( _U_PRESSURE, dP0, false );

		CString msg;
		FormatString( msg, IDS_DLGINDSELPMPANELS_TTERRORPZ, strPz, strP0 );
		m_ToolTip.AddToolWindow( &m_EditPz, msg );
		m_EditPz.SetDrawBorder( true, _RED );
		bReturn = false;
	}
	else
	{
		InitToolTips( &m_EditPz );
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetPz( dPz );
	}

	return bReturn;
}

bool CDlgIndSelPMPanelHCSBase::CheckSafetyVRP()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList ||
			NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	ResetToolTipsErrorBorder( &m_EditSafetyValveResponsePressure );
	
	bool bReturn = true;
	double dSafetyVRP = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSafetyValveResponsePressure();
	double dPSVLimit = 0.0;

	// Check what is the pSV limit.
	bool bpSVCorrect = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->CheckPSV( &dPSVLimit );
	CString strPSVLimit = WriteCUDouble( _U_PRESSURE, dPSVLimit, true, -1, 2 );

	// Update PSV limit static text.
	CString msg = _T(">= ") + strPSVLimit;
	GetDlgItem( IDC_STATIC_MINIMUMPSV )->SetWindowTextW( msg );

	if( false == bpSVCorrect )
	{
		// We take what the user has written to avoid rounding this value with error.
		// Example: user input: 5,726; value computed: 5,73 -> If we use 'WriteCUDouble' for the use input, we will have 
		//          in the error message "5,73 < 5,73".
		TCHAR tcPSVInput[256];
		m_EditSafetyValveResponsePressure.GetWindowText( tcPSVInput, 256 );

		CString msg;
		FormatString( msg, IDS_DLGINDSELPMPANELS_TTERRORPSV, tcPSVInput, strPSVLimit );

		m_ToolTip.AddToolWindow( &m_EditSafetyValveResponsePressure, msg );
		m_EditSafetyValveResponsePressure.SetDrawBorder( true, _RED );
		bReturn = false;
	}
	else
	{
		InitToolTips( &m_EditSafetyValveResponsePressure );
	}

	return bReturn;
}

void CDlgIndSelPMPanelHCSBase::FillComboPressOn( PressurON ePressOn )
{
	CDS_TechnicalParameter *pTechParam = m_pclIndSelPMParams->m_pTADS->GetpTechParams();

	if( NULL == pTechParam )
	{
		return;
	}
	
	CRankEx PressurOnList;
	m_ComboPressOn.ResetContent();
	CRankEx rkList;

	rkList.Add( ( LPCTSTR )TASApp.LoadLocalizedString( pTechParam->GetPressurONIDS( PressurON::poPumpDischarge ) ),
				PressurON::poPumpDischarge, PressurON::poPumpDischarge );
	
	rkList.Add( ( LPCTSTR )TASApp.LoadLocalizedString( pTechParam->GetPressurONIDS( PressurON::poPumpSuction ) ),
				PressurON::poPumpSuction, PressurON::poPumpSuction );

	rkList.Transfer( &m_ComboPressOn );

	int iSelPos = 0;

	for( int i = 0; i < m_ComboPressOn.GetCount(); i++ )
	{
        if( ePressOn == (PressurON)m_ComboPressOn.GetItemData( i ) )
		{
			iSelPos = i;
			break;
		}
	}

	m_ComboPressOn.SetCurSel( iSelPos );

	if( m_ComboPressOn.GetCount() <= 1 )
	{
		m_ComboPressOn.EnableWindow( false );
	}
	else
	{
		m_ComboPressOn.EnableWindow( true );
	}
}

UINT CDlgIndSelPMPanelHCSBase::AreHeatingSolarTemperaturesOK( CExtNumEdit *pclSftyEdit, CExtNumEdit *pclSupplyEdit,
		CExtNumEdit *pclReturnEdit, CExtNumEdit *pclMinEdit, CExtNumEdit *pclFillEdit, bool bInsertInErrorMap )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( MEID_Undefined );
	}

	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERTA_RETURN( MEID_Undefined );
	}

	UINT uiErrorFlag = MEID_Undefined;

	if( NULL == pclSftyEdit || NULL == pclSupplyEdit || NULL == pclReturnEdit || NULL == pclMinEdit || NULL == pclFillEdit )
	{
		return uiErrorFlag;
	}

	// By default reset all errors.
	pclSftyEdit->ResetDrawBorder();
	pclSupplyEdit->ResetDrawBorder();
	pclReturnEdit->ResetDrawBorder();
	pclMinEdit->ResetDrawBorder();
	pclFillEdit->ResetDrawBorder();
	ResetToolTipsErrorBorder( pclSftyEdit );
	ResetToolTipsErrorBorder( pclSupplyEdit );
	ResetToolTipsErrorBorder( pclReturnEdit );
	ResetToolTipsErrorBorder( pclMinEdit );
	ResetToolTipsErrorBorder( pclFillEdit );

	// After reset reload default values.
	InitToolTips( NULL );

	CString strSfty = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSAFETYTEMP );
	CString strSupply = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSUPPLYTEMP );
	CString strReturn = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORRETURNTEMP );
	CString strMin = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORMINTEMP );
	CString strFill = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORFILLTEMP );
	CString strSftyLowerCase = strSfty;
	strSftyLowerCase.MakeLower();
	CString strSupplyLowerCase = strSupply;
	strSupplyLowerCase.MakeLower();
	CString strReturnLowerCase = strReturn;
	strReturnLowerCase.MakeLower();
	CString strMinLowerCase = strMin;
	strMinLowerCase.MakeLower();
	CString strFillLowerCase = strFill;
	strFillLowerCase.MakeLower();

	// Before check temperatures between each others, we check first if temperature are valid.
	bool bSftyBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclSftyEdit, strSfty, bInsertInErrorMap, MEID_SafetyTempLimiter ) )
	{
		bSftyBadFluidError = true;
		// HYS-838: To return the error flag and not update values
		uiErrorFlag |= MEID_SafetyTempLimiter;
	}

	bool bSupplyBadfluidError = false;

	if( false == IsFluidTemperatureOK( pclSupplyEdit, strSupply, bInsertInErrorMap, MEID_SupplyTemperature ) )
	{
		bSupplyBadfluidError = true;
	}

	bool bReturnBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclReturnEdit, strReturn, bInsertInErrorMap, MEID_ReturnTemperature ) )
	{
		bReturnBadFluidError = true;
		// HYS-838: To return the error flag and not update values
		uiErrorFlag |= MEID_ReturnTemperature;
	}

	bool bMinBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclMinEdit, strMin, bInsertInErrorMap, MEID_MinTemperature ) )
	{
		bMinBadFluidError = true;
		uiErrorFlag |= MEID_MinTemperature;
	}

	bool bFillBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclFillEdit, strFill, bInsertInErrorMap, MEID_FillTemperature ) )
	{
		bFillBadFluidError = true;
	}

	// Check now all temperatures.
	double dSafetyTempLimiter = pclSftyEdit->GetCurrentValSI();
	double dSupplyTemperature = pclSupplyEdit->GetCurrentValSI();
	double dReturnTemperature = pclReturnEdit->GetCurrentValSI();
	double dMinTemperature = pclMinEdit->GetCurrentValSI();
	double dFillTemperature = pclFillEdit->GetCurrentValSI();

	CString strTTSfty( _T( "" ) );
	CString strTTSupply( _T( "" ) );
	CString strTTReturn( _T( "" ) );
	CString strTTMin( _T( "" ) );
	CString strTTFill( _T( "" ) );
	
	// In first step, check temperatures that must be not to high for EN12828 norm.
	if( _T("PM_NORM_EN12828" ) == CString( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetNormID() ) )
	{
		double dSupplyTempLimit = TASApp.GetpTADS()->GetpTechParams()->GetFluidTempLimitEN12953();
		double dReturnTempLimit = dSupplyTempLimit;
		double dTAZLimit = TASApp.GetpTADS()->GetpTechParams()->GetSafetyTempLimiterLimitEN12953();

		if( dSupplyTemperature > dSupplyTempLimit )
		{
			if( false == strTTSupply.IsEmpty() )
			{
				strTTSupply += _T("\r\n");
			}

			CString strTemp;
			CString strSupplyTempLimit = WriteCUDouble( _U_TEMPERATURE, dSupplyTempLimit, true );
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERROREN12953, strSupply, strSupplyTempLimit );
			strTTSupply += strTemp;
		}

		if( dReturnTemperature > dReturnTempLimit )
		{
			if( false == strTTReturn.IsEmpty() )
			{
				strTTReturn += _T("\r\n");
			}

			CString strTemp;
			CString strReturnTempLimit = WriteCUDouble( _U_TEMPERATURE, dReturnTempLimit, true );
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERROREN12953, strReturn, strReturnTempLimit );
			strTTReturn += strTemp;
		}

		if( dSafetyTempLimiter > dTAZLimit )
		{
			if( false == strTTSfty.IsEmpty() )
			{
				strTTSfty += _T("\r\n");
			}

			CString strTemp;
			CString strTAZLimit = WriteCUDouble( _U_TEMPERATURE, dTAZLimit, true );
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERROREN12953, strSfty, strTAZLimit );
			strTTSfty += strTemp;
		}
	}

	if( dSafetyTempLimiter < dSupplyTemperature )
	{
		if( false == bSftyBadFluidError )
		{
			FormatString( strTTSfty, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strSfty, strSupplyLowerCase );
		}

		if( false == bSupplyBadfluidError )
		{
			FormatString( strTTSupply, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strSupply, strSftyLowerCase );
		}
	}

	if( dSupplyTemperature < dReturnTemperature )
	{
		if( false == bSupplyBadfluidError )
		{
			if( false == strTTSupply.IsEmpty() )
			{
				strTTSupply += _T("\r\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strSupply, strReturnLowerCase );
			strTTSupply += strTemp;
		}

		if( false == bReturnBadFluidError )
		{
			FormatString( strTTReturn, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strReturn, strSupplyLowerCase );
		}
	}

	if( dReturnTemperature < dMinTemperature )
	{
		if( false == bReturnBadFluidError )
		{
			if( false == strTTReturn.IsEmpty() )
			{
				strTTReturn += _T("\r\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strReturn, strMinLowerCase );
			strTTReturn += strTemp;
		}

		if( false == bMinBadFluidError )
		{
			FormatString( strTTMin, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strMin, strReturnLowerCase );
		}
	}

	if( dFillTemperature > dReturnTemperature )
	{
		if( false == bFillBadFluidError )
		{
			FormatString( strTTFill, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strFill, strReturnLowerCase );
		}

		if( false == bSftyBadFluidError )
		{
			if( false == strTTReturn.IsEmpty() )
			{
				strTTReturn += _T("\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strReturn, strFillLowerCase );
			strTTReturn += strTemp;
		}
	}

	if( dFillTemperature < dMinTemperature )
	{
		if( false == bFillBadFluidError )
		{
			FormatString( strTTFill, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strFill, strMinLowerCase );
		}

		if( false == bMinBadFluidError )
		{
			if( false == strTTMin.IsEmpty() )
			{
				strTTMin += _T("\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strMin, strFillLowerCase );
			strTTMin += strTemp;
		}
	}

	if( false == strTTSfty.IsEmpty() )
	{
		ResetToolTipsErrorBorder( pclSftyEdit );
		m_ToolTip.AddToolWindow( pclSftyEdit, strTTSfty );
		pclSftyEdit->SetDrawBorder( true, _RED );
		uiErrorFlag |= MEID_SafetyTempLimiter;

		if( true == bInsertInErrorMap )
		{
			m_mapErrorMsg[MEID_SafetyTempLimiter] = _T("\r\n - ") + CString( strTTSfty );
		}
	}

	if( false == strTTSupply.IsEmpty() )
	{
		ResetToolTipsErrorBorder( pclSupplyEdit );
		m_ToolTip.AddToolWindow( pclSupplyEdit, strTTSupply );
		pclSupplyEdit->SetDrawBorder( true, _RED );
		uiErrorFlag |= MEID_SupplyTemperature;

		if( true == bInsertInErrorMap )
		{
			m_mapErrorMsg[MEID_SupplyTemperature] = _T("\r\n - ") + strTTSupply;
		}
	}

	if( false == strTTReturn.IsEmpty() )
	{
		ResetToolTipsErrorBorder( pclReturnEdit );
		m_ToolTip.AddToolWindow( pclReturnEdit, strTTReturn );
		pclReturnEdit->SetDrawBorder( true, _RED );
		uiErrorFlag |= MEID_ReturnTemperature;

		if( true == bInsertInErrorMap )
		{
			m_mapErrorMsg[MEID_ReturnTemperature] = _T("\r\n - ") + strTTReturn;
		}
	}

	if( false == strTTMin.IsEmpty() )
	{
		ResetToolTipsErrorBorder( pclMinEdit );
		m_ToolTip.AddToolWindow( pclMinEdit, strTTMin );
		pclMinEdit->SetDrawBorder( true, _RED );
		uiErrorFlag |= MEID_MinTemperature;

		if( true == bInsertInErrorMap )
		{
			m_mapErrorMsg[MEID_MinTemperature] = _T("\r\n - ") + strTTMin;
		}
	}

	if( false == strTTFill.IsEmpty() )
	{
		ResetToolTipsErrorBorder( pclFillEdit );
		m_ToolTip.AddToolWindow( pclFillEdit, strTTFill );
		pclFillEdit->SetDrawBorder( true, _RED );

		uiErrorFlag |= MEID_FillTemperature;

		if( true == bInsertInErrorMap )
		{
			m_mapErrorMsg[MEID_FillTemperature] = _T("\r\n - ") + strTTFill;
		}
	}

	return uiErrorFlag;
}

UINT CDlgIndSelPMPanelHCSBase::AreCoolingTemperaturesOK( CExtNumEdit *pclMaxEdit, CExtNumEdit *pclSupplyEdit,
		CExtNumEdit *pclReturnEdit, CExtNumEdit *pclMinEdit, INT iCheckMinTemp, CExtNumEdit *pclFillEdit, bool bInsertInErrorMap )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( MEID_Undefined );
	}

	UINT uiErrorFlag = MEID_Undefined;

	if( NULL == pclMaxEdit || NULL == pclSupplyEdit || NULL == pclReturnEdit || NULL == pclMinEdit || NULL == pclFillEdit )
	{
		return uiErrorFlag;
	}

	// By default, reset all error on temperature fields.
	pclMaxEdit->ResetDrawBorder();
	pclSupplyEdit->ResetDrawBorder();
	pclReturnEdit->ResetDrawBorder();
	pclMinEdit->ResetDrawBorder();
	pclFillEdit->ResetDrawBorder();
	
	ResetToolTipsErrorBorder( pclMaxEdit );
	ResetToolTipsErrorBorder( pclSupplyEdit );
	ResetToolTipsErrorBorder( pclReturnEdit );
	ResetToolTipsErrorBorder( pclMinEdit );
	ResetToolTipsErrorBorder( pclFillEdit );

	// After reset reload default values.
	InitToolTips( NULL );

	CString strMax = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORMAXTEMP );
	CString strSupply = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSUPPLYTEMP );
	CString strReturn = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORRETURNTEMP );
	CString strMin = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORMINTEMP );
	CString strFill = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORFILLTEMP );
	CString strMaxLowerCase = strMax;
	strMaxLowerCase.MakeLower();
	CString strSupplyLowerCase = strSupply;
	strSupplyLowerCase.MakeLower();
	CString strReturnLowerCase = strReturn;
	strReturnLowerCase.MakeLower();
	CString strMinLowerCase = strMin;
	strMinLowerCase.MakeLower();
	CString strFillLowerCase = strFill;
	strFillLowerCase.MakeLower();

	// Before check temperatures between each others, we check first if temperature are valid.
	bool bMaxBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclMaxEdit, strMax, bInsertInErrorMap, MEID_MaxTemperature ) )
	{
		bMaxBadFluidError = true;
		uiErrorFlag |= MEID_MaxTemperature;
	}

	bool bSupplyBadfluidError = false;

	if( false == IsFluidTemperatureOK( pclSupplyEdit, strSupply, bInsertInErrorMap, MEID_SupplyTemperature ) )
	{
		bSupplyBadfluidError = true;
		// HYS-838: To return the error flag and not update values
		uiErrorFlag |= MEID_SupplyTemperature;
	}

	bool bReturnBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclReturnEdit, strReturn, bInsertInErrorMap, MEID_ReturnTemperature ) )
	{
		bReturnBadFluidError = true;
	}

	bool bMinBadFluidError = false;

	if( BST_CHECKED == iCheckMinTemp
		&& false == IsFluidTemperatureOK( pclMinEdit, strMin, bInsertInErrorMap, MEID_MinTemperature ) )
	{
		bMinBadFluidError = true;
		uiErrorFlag |= MEID_MinTemperature;
	}

	bool bFillBadFluidError = false;

	if( false == IsFluidTemperatureOK( pclFillEdit, strFill, bInsertInErrorMap, MEID_FillTemperature ) )
	{
		bFillBadFluidError = true;
	}

	/*
	// Special case for cooling. The fill temperature and water make-up temperature are in fact the same!
	// Remark: in this case we force 'fInsertInErrorMap' to 'false'. Because in this case, if there is an error,
	//         this one is already inserted in the error map by the previous call (with fill temperature).
	if( true == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->IsWaterMakeUpExist() && ePanelCooling == GetPanelType()
			&& false == IsFluidTemperatureOK( &m_EditWaterMakeUpWaterTemp, strFill, false, MEID_FillTemperature ) )
	{
		bAllIsOK = false;
	}
	*/

	// Check now all temperatures.
	double dMaxTemperature = pclMaxEdit->GetCurrentValSI();
	double dSupplyTemperature = pclSupplyEdit->GetCurrentValSI();
	double dReturnTemperature = pclReturnEdit->GetCurrentValSI();
	double dMinTemperature = pclMinEdit->GetCurrentValSI();
	double dFillTemperature = pclFillEdit->GetCurrentValSI();

	CString strTTMax( _T( "" ) );
	CString strTTSupply( _T( "" ) );
	CString strTTReturn( _T( "" ) );
	CString strTTMin( _T( "" ) );
	CString strTTFill( _T( "" ) );

	if( dMaxTemperature < dReturnTemperature )
	{
		if( false == bMaxBadFluidError )
		{
			FormatString( strTTMax, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strMax, strReturnLowerCase );
		}

		if( false == bReturnBadFluidError )
		{
			FormatString( strTTReturn, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strReturn, strMaxLowerCase );
		}
	}

	if( dReturnTemperature < dSupplyTemperature )
	{
		if( false == bReturnBadFluidError )
		{
			if( false == strTTReturn.IsEmpty() )
			{
				strTTReturn += _T("\r\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strReturn, strSupplyLowerCase );
			strTTReturn += strTemp;
		}

		if( false == bSupplyBadfluidError )
		{
			FormatString( strTTSupply, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strSupply, strReturnLowerCase );
		}
	}

	if( BST_CHECKED == iCheckMinTemp && dMinTemperature > dSupplyTemperature )
	{
		if( false == bMinBadFluidError )
		{
			FormatString( strTTMin, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strMin, strSupplyLowerCase );
		}

		if( false == bSupplyBadfluidError )
		{
			if( false == strTTSupply.IsEmpty() )
			{
				strTTSupply += _T("\r\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strSupply, strMinLowerCase );
			strTTSupply += strTemp;
		}
	}

	if( dFillTemperature > dMaxTemperature )
	{
		if( false == bFillBadFluidError )
		{
			FormatString( strTTFill, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strFill, strMaxLowerCase );
		}

		if( false == bMaxBadFluidError )
		{
			if( false == strTTMax.IsEmpty() )
			{
				strTTMax += _T("\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strMax, strFillLowerCase );
			strTTMax += strTemp;
		}
	}

	if( dFillTemperature < dMinTemperature )
	{
		if( false == bFillBadFluidError )
		{
			FormatString( strTTFill, IDS_DLGINDSELPMPANELS_TTERRORTEMPLOWER, strFill, strMinLowerCase );
		}

		if( BST_CHECKED == iCheckMinTemp && false == bReturnBadFluidError )
		{
			if( false == strTTMin.IsEmpty() )
			{
				strTTMin += _T("\n");
			}

			CString strTemp;
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERRORTEMPGREATER, strMin, strFillLowerCase );
			strTTMin += strTemp;
		}
	}

	if( false == strTTMax.IsEmpty() )
	{
		ResetToolTipsErrorBorder( pclMaxEdit );
		m_ToolTip.AddToolWindow( pclMaxEdit, strTTMax );
		pclMaxEdit->SetDrawBorder( true, _RED );
		uiErrorFlag |= MEID_MaxTemperature;

		if( true == bInsertInErrorMap )
		{
			m_mapErrorMsg[MEID_MaxTemperature] = _T("\r\n - ") + strTTMax;
		}
	}

	if( false == strTTSupply.IsEmpty() )
	{
		ResetToolTipsErrorBorder( pclSupplyEdit );
		m_ToolTip.AddToolWindow( pclSupplyEdit, strTTSupply );
		pclSupplyEdit->SetDrawBorder( true, _RED );
		uiErrorFlag |= MEID_SupplyTemperature;

		if( true == bInsertInErrorMap )
		{
			m_mapErrorMsg[MEID_SupplyTemperature] = _T("\r\n - ") + strTTSupply;
		}
	}

	if( false == strTTReturn.IsEmpty() )
	{
		ResetToolTipsErrorBorder( pclReturnEdit );
		m_ToolTip.AddToolWindow( pclReturnEdit, strTTReturn );
		pclReturnEdit->SetDrawBorder( true, _RED );
		uiErrorFlag |= MEID_ReturnTemperature;

		if( true == bInsertInErrorMap )
		{
			m_mapErrorMsg[MEID_ReturnTemperature] = _T("\r\n - ") + strTTReturn;
		}
	}

	if( false == strTTMin.IsEmpty() )
	{
		ResetToolTipsErrorBorder( pclMinEdit );
		m_ToolTip.AddToolWindow( pclMinEdit, strTTMin );
		pclMinEdit->SetDrawBorder( true, _RED );
		uiErrorFlag |= MEID_MinTemperature;

		if( true == bInsertInErrorMap )
		{
			m_mapErrorMsg[MEID_MinTemperature] = _T("\r\n - ") + strTTMin;
		}
	}

	if( false == strTTFill.IsEmpty() )
	{
		ResetToolTipsErrorBorder( pclFillEdit );
		m_ToolTip.AddToolWindow( pclFillEdit, strTTFill );
		pclFillEdit->SetDrawBorder( true, _RED );

		uiErrorFlag |= MEID_FillTemperature;

		if( true == bInsertInErrorMap )
		{
			m_mapErrorMsg[MEID_FillTemperature] = _T("\r\n - ") + strTTFill;
		}
	}

	return uiErrorFlag;
}

UINT CDlgIndSelPMPanelHCSBase::IsInstalledPowerOK( CExtNumEdit *pclInstalledPower, bool bInsertInErrorMap )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( MEID_Undefined );
	}

	UINT uiErrorFlag = MEID_Undefined;

	if( NULL == pclInstalledPower )
	{
		return uiErrorFlag;
	}

	// By default reset the error.
	pclInstalledPower->ResetDrawBorder();
	ResetToolTipsErrorBorder( pclInstalledPower );

	// After reset reload default values.
	// Call InitToolTips with specific control to avoid rest of min. temp. control tooltip
	InitToolTips( pclInstalledPower );

	CPMInputUser *pclPMInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) && pclPMInputUser->GetInstalledPower() <= 0 )
	{
		m_mapErrorMsg[MEID_InstalledPower] = _T("\r\n - ") + TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORPOWER );
		m_ToolTip.AddToolWindow( pclInstalledPower, TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORPOWER ) );
		pclInstalledPower->SetDrawBorder( true, _RED );
		uiErrorFlag |= MEID_InstalledPower;
	}

	return uiErrorFlag;
}

UINT CDlgIndSelPMPanelHCSBase::IspSVLocationOK( CExtNumEdit *pclpSVLocation, bool bInsertInErrorMap )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( MEID_Undefined );
	}

	UINT uiErrorFlag = MEID_Undefined;

	if( NULL == pclpSVLocation )
	{
		return uiErrorFlag;
	}

	// By default reset the error.
	pclpSVLocation->ResetDrawBorder();
	ResetToolTipsErrorBorder( pclpSVLocation );

	// After reset reload default values.
	// Call InitToolTips with specific control to avoid rest of min. temp. control tooltip
	InitToolTips( pclpSVLocation );

	CPMInputUser *pclPMInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	double dpSVLocation = pclpSVLocation->GetCurrentValSI();

	CString strToolTip( _T("") );

	if( dpSVLocation >= 0.0 && dpSVLocation > pclPMInputUser->GetStaticHeight() )
	{
		// The safety valve can't be placed in a higher point that the static height defined (%1 > %2).
		CString strpSVLocation = WriteCUDouble( _U_LENGTH, dpSVLocation, true );
		CString strStaticHeight = WriteCUDouble( _U_LENGTH, pclPMInputUser->GetStaticHeight(), true );
		FormatString( strToolTip, IDS_DLGINDSELPMPANELS_TTERRORPSVLOCATIONTOHIGH, strpSVLocation, strStaticHeight );
	}
	else if( dpSVLocation < 0.0 )
	{
		// See the 'PM - Max height between vessel and pSV.docx' documentation for explanation.
		double dHLimit = pclPMInputUser->GetSafetyValveResponsePressure() - pclPMInputUser->GetMinimumRequiredPSVRaw( pclPMInputUser->GetPressureMaintenanceType() );
		dHLimit /= ( pclPMInputUser->GetpWC()->GetDens() * 9.81 );

		if( abs( dpSVLocation ) > dHLimit )
		{
			// The safety valve is placed too low. The actual limit is %1.
			CString strHLimit = WriteCUDouble( _U_LENGTH, dHLimit, true );
			FormatString( strToolTip, IDS_DLGINDSELPMPANELS_TTERRORPSVLOCATIONTOLOW, strHLimit );
		}
	}

	if( false == strToolTip.IsEmpty() )
	{
		m_ToolTip.AddToolWindow( pclpSVLocation, strToolTip );
		pclpSVLocation->SetDrawBorder( true, _RED );
		uiErrorFlag |= MEID_PSVLocation;

		if( true == bInsertInErrorMap )
		{
			m_mapErrorMsg[MEID_PSVLocation] = _T("\r\n - ") + strToolTip;
		}
	}

	return uiErrorFlag;
}

void CDlgIndSelPMPanelHCSBase::UpdateOtherHeatingSolarTemperatures( UINT uiError, CExtNumEdit *pclSftyEdit, CExtNumEdit *pclSupplyEdit, CExtNumEdit *pclReturnEdit, CExtNumEdit *pclMinEdit, CExtNumEdit *pclFillEdit )
{
	if( NULL == pclSftyEdit || NULL == pclSupplyEdit || NULL == pclReturnEdit || NULL == pclMinEdit || NULL == pclFillEdit )
	{
		return;
	}

	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclIndSelPMParams->m_pTADS || NULL == m_pclIndSelPMParams->m_pTADS->GetpTechParams() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	CDS_TechnicalParameter *pTechParam = m_pclIndSelPMParams->m_pTADS->GetpTechParams();

	bool bUpdateRibbon = false;
	pclPMInputUser->SetSafetyTempLimiter( pclSftyEdit->GetCurrentValSI() );

	if( MEID_SafetyTempLimiter != ( MEID_SafetyTempLimiter & uiError ) )
	{
		pTechParam->SetDefaultTAZ( pclSftyEdit->GetCurrentValSI() );
	}

	pclPMInputUser->SetSupplyTemperature( pclSupplyEdit->GetCurrentValSI() );
		
	m_pclIndSelPMParams->m_WC.SetTemp( pclSupplyEdit->GetCurrentValSI() );
	m_pclIndSelPMParams->m_WC.UpdateFluidData( m_pclIndSelPMParams->m_WC.GetTemp() );
	*m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetpWC() = m_pclIndSelPMParams->m_WC;

	if( MEID_SupplyTemperature != ( MEID_SupplyTemperature & uiError ) )
	{
		// Copy also in the current water char object from the 'CTADataStruct' database. Because 'CMainFrame::OnWaterChange' will be called because
		// the 'WM_USER_WATERCHANGE' message sent just after. And this method will update the ribbon bar not with values from the technical 
		// parameters but well with the current water char object.
		*( TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData() ) = m_pclIndSelPMParams->m_WC;

		pTechParam->SetDefaultISTps( pclSupplyEdit->GetCurrentValSI() );
		bUpdateRibbon = true;
	}

	// HYS-1350: Do not save return temperature if error.
	if( MEID_ReturnTemperature != ( MEID_ReturnTemperature & uiError ) )
	{
		pclPMInputUser->SetReturnTemperature( pclReturnEdit->GetCurrentValSI() );
	}

	if( MEID_SupplyTemperature != ( MEID_SupplyTemperature & uiError ) && MEID_ReturnTemperature != ( MEID_ReturnTemperature & uiError ) )
	{
		pTechParam->SetDefaultISDT( pclSupplyEdit->GetCurrentValSI() - pclReturnEdit->GetCurrentValSI() );
		bUpdateRibbon = true;
	}

	// HYS-1350: Do not save min. temperature when error.
	if( MEID_MinTemperature != ( MEID_MinTemperature & uiError ) )
	{
		pclPMInputUser->SetMinTemperature( pclMinEdit->GetCurrentValSI() );
	}

	// HYS-1350: Do not save fill temperature when error.
	if( MEID_FillTemperature != ( MEID_FillTemperature & uiError ) )
	{
		pclPMInputUser->SetFillTemperature( pclFillEdit->GetCurrentValSI() );
	}

	if( true == bUpdateRibbon )
	{
		// First message is sent to the mainframe to allow it to change the ribbon value.
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_WATERCHANGE, ( WPARAM )WMUserWaterCharWParam::WM_UWC_WP_ForProductSel );
	}
}

void CDlgIndSelPMPanelHCSBase::UpdateOtherCoolingTemperatures( UINT uiError, CExtNumEdit *pclMaxEdit, CExtNumEdit *pclSupplyEdit, CExtNumEdit *pclReturnEdit, CExtNumEdit *pclMinEdit, 
		INT iCheckMinTemp, CExtNumEdit *pclFillEdit )
{
	if( NULL == pclMaxEdit || NULL == pclSupplyEdit || NULL == pclReturnEdit || NULL == pclMinEdit || NULL == pclFillEdit )
	{
		return;
	}

	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclIndSelPMParams->m_pTADS || NULL == m_pclIndSelPMParams->m_pTADS->GetpTechParams() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	CDS_TechnicalParameter *pTechParam = m_pclIndSelPMParams->m_pTADS->GetpTechParams();
	bool bUpdateRibbon = false;

	pclPMInputUser->SetMaxTemperature( pclMaxEdit->GetCurrentValSI() );
	pclPMInputUser->SetSupplyTemperature( pclSupplyEdit->GetCurrentValSI() );

	m_pclIndSelPMParams->m_WC.SetTemp( pclSupplyEdit->GetCurrentValSI() );
	m_pclIndSelPMParams->m_WC.UpdateFluidData( m_pclIndSelPMParams->m_WC.GetTemp() );

	if( MEID_SupplyTemperature != ( MEID_SupplyTemperature & uiError ) )
	{
		// Copy also in the current water char object from the 'CTADataStruct' database. Because 'CMainFrame::OnWaterChange' will be called because
		// the 'WM_USER_WATERCHANGE' message sent just after. And this method will update the ribbon bar not with values from the technical 
		// parameters but well with the current water char object.
		*( TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData() ) = m_pclIndSelPMParams->m_WC;

		pTechParam->SetDefaultISTps( pclSupplyEdit->GetCurrentValSI() );
		bUpdateRibbon = true;
	}

	// HYS-1350: Do not save return temperature if error.
	if( MEID_ReturnTemperature != ( MEID_ReturnTemperature & uiError ) )
	{
		pclPMInputUser->SetReturnTemperature( pclReturnEdit->GetCurrentValSI() );
	}

	if( MEID_SupplyTemperature != ( MEID_SupplyTemperature & uiError ) && MEID_ReturnTemperature != ( MEID_ReturnTemperature & uiError ) )
	{
		pTechParam->SetDefaultISDT( pclReturnEdit->GetCurrentValSI() - pclSupplyEdit->GetCurrentValSI() );
		bUpdateRibbon = true;
	}

	// HYS-1350: Do not save min. temperature when error.
	if( MEID_MinTemperature != ( MEID_MinTemperature & uiError ) )
	{
		pclPMInputUser->SetMinTemperature( pclMinEdit->GetCurrentValSI() );
	}

	// HYS-1350: Do not save fill temperature when error.
	if( MEID_FillTemperature != ( MEID_FillTemperature & uiError ) )
	{
		pclPMInputUser->SetFillTemperature( pclFillEdit->GetCurrentValSI() );
	}

	if( true == bUpdateRibbon )
	{
		// Message is sent to the mainframe to allow it to change the ribbon value.
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_WATERCHANGE, ( WPARAM )WMUserWaterCharWParam::WM_UWC_WP_ForProductSel );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelHeatingBase dialog
CDlgIndSelPMPanelHeatingBase::CDlgIndSelPMPanelHeatingBase( CIndSelPMParams *pclIndSelParams, PanelType ePanelType, UINT nID, CWnd *pParent )
	: CDlgIndSelPMPanelHCSBase( pclIndSelParams, ePanelType, nID, pParent )
{
	m_dSafetyTempLimiterSaved = 0.0;
	m_dWaterHardnessSaved = 0.0;
}

void CDlgIndSelPMPanelHeatingBase::OnNormChanged()
{
	AreHeatingSolarTemperaturesOK( &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );
	ClearRightSheet();

	// Norm has an influence on the PSV.
	CheckSafetyVRP();

	// HYS-1407: Norm has an influence on the p0.
	UpdateP0();
}

bool CDlgIndSelPMPanelHeatingBase::ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser || false == CDlgIndSelPMPanelHCSBase::ApplyPMInputUser( pclPMInputUser, bAllowShowErrorMessage ) )
	{
		return false;
	}

	m_EditSafetyTLim.SetCurrentValSI( pclPMInputUser->GetSafetyTempLimiter() );
	m_EditSafetyTLim.Update();

	// HYS-1350: forgot to initialize also this field.
	m_EditMinTemp.SetCurrentValSI( pclPMInputUser->GetMinTemperature() );
	m_EditMinTemp.Update();

	m_EditWaterHardness.SetCurrentValSI( pclPMInputUser->GetWaterMakeUpWaterHardness() );
	m_EditWaterHardness.Update();

	return true;
}

bool CDlgIndSelPMPanelHeatingBase::VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser )
	{
		return false;
	}

	// Check temperature fields.
	AreHeatingSolarTemperaturesOK( &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp, true );

	pclPMInputUser->CheckInputData( &m_vecErrorList );
	return CDlgIndSelPMPanelHCSBase::VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

void CDlgIndSelPMPanelHeatingBase::OnNewDocument()
{
	CDlgIndSelPMPanelHCSBase::OnNewDocument();

	m_EditInstalledPower.SetReadOnly( FALSE );
	m_EditInstalledPower.EnableWindow( TRUE );

	CButton *pclButton = (CButton *)GetDlgItem( IDC_BUTTON_WATERCONTENT );

	if( NULL != pclButton && NULL != pclButton->GetSafeHwnd() )
	{
		pclButton->SetIcon( (HICON)LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDI_VOLUME_ICON ), IMAGE_ICON, 
				18, 18, LR_DEFAULTCOLOR ) );
	}

	// To avoid any 'OnEnChange' event to be called.
	m_bOnEnChangeEnabled = false;

	// Don't call 'OnUnitChange' because in this method we call a 'FillPMInputUser' and 'ApplyPMInputUser' and this will change status of some
	// control that we don't need when 'OnNewDocument' is called.
	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	GetDlgItem( IDC_STATIC_SAFETYTEMPLIMUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditSafetyTLim, _U_TEMPERATURE );

	GetDlgItem( IDC_STATIC_WATERHARDUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_WATERHARDNESS ).c_str() );
	InitNumericalEdit( &m_EditWaterHardness, _U_WATERHARDNESS );

	m_bOnEnChangeEnabled = true;
}

void CDlgIndSelPMPanelHeatingBase::EnablePMSelection( bool Enable )
{
	CDlgIndSelPMPanelHCSBase::EnablePMSelection( Enable );
	m_EditWaterHardness.ActiveSpecialValidation( Enable, this );
	m_EditSafetyTLim.ActiveSpecialValidation( Enable, this );
	Invalidate();
	UpdateWindow();
}

BEGIN_MESSAGE_MAP( CDlgIndSelPMPanelHeatingBase, CDlgIndSelPMPanelHCSBase )
	ON_BN_CLICKED( IDC_BUTTON_WATERCONTENT, OnBnClickedWaterContent )

	ON_EN_CHANGE( IDC_EDIT_SAFETYTEMPLIM, OnEnChangeSafetyTempLimiter )
	ON_EN_CHANGE( IDC_EDIT_WATERHARDNESS, OnEnChangeEdit )

	ON_EN_SETFOCUS( IDC_EDIT_SAFETYTEMPLIM, OnEnSetFocusSafetyTempLimiter )
	ON_EN_SETFOCUS( IDC_EDIT_WATERHARDNESS, OnEnSetFocusWaterHardness )

	ON_EN_KILLFOCUS( IDC_EDIT_SAFETYTEMPLIM, OnKillFocusEditSafetyTempLimiter )
	ON_EN_KILLFOCUS( IDC_EDIT_SUPPLYTEMP, OnKillFocusEditSupplyTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_RETURNTEMP, OnKillFocusEditReturnTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_MINTEMP, OnKillFocusEditMinTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_FILLTEMP, OnKillFocusEditFillTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_WATERHARDNESS, OnKillFocusEditWaterHardness )
END_MESSAGE_MAP()

void CDlgIndSelPMPanelHeatingBase::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelPMPanelHCSBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATIC_SAFETYTEMPLIM, m_StaticSafetyTLim );
	DDX_Control( pDX, IDC_EDIT_SAFETYTEMPLIM, m_EditSafetyTLim );
	DDX_Control( pDX, IDC_EDIT_WATERHARDNESS, m_EditWaterHardness );
}

BOOL CDlgIndSelPMPanelHeatingBase::OnInitDialog()
{
	CDlgIndSelPMPanelHCSBase::OnInitDialog();

	// Disable the 'OnNewDocument' handler to not interfere with this dialog.
	m_EditSafetyTLim.SetOnNewDocumentHandler( false );
	m_EditWaterHardness.SetOnNewDocumentHandler( false );

	m_StaticSafetyTLim.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PMHEATING_SAFETYTEMPLIM ) );
	m_EditSafetyTLim.ActiveSpecialValidation( true, this );

	GetDlgItem( IDC_STATIC_WATERHARDNESS )->SetWindowText( TASApp.LoadLocalizedString( IDS_PM_WATERHARDNESS ) );
	m_EditWaterHardness.ActiveSpecialValidation( true, this );

	return TRUE;
}

LRESULT CDlgIndSelPMPanelHeatingBase::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList )
	{
		// Not yet ready!
		return 0;
	}

	CDlgIndSelPMPanelHCSBase::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	GetDlgItem( IDC_STATIC_SAFETYTEMPLIMUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditSafetyTLim, _U_TEMPERATURE );

	GetDlgItem( IDC_STATIC_WATERHARDUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_WATERHARDNESS ).c_str() );
	InitNumericalEdit( &m_EditWaterHardness, _U_WATERHARDNESS );

	return 0;
}

void CDlgIndSelPMPanelHeatingBase::OnBnClickedWaterContent()
{
	double dSupplyTemp = m_EditSupplyTemp.GetCurrentValSI();
	double dReturnTemp = m_EditReturnTemp.GetCurrentValSI();

	CDlgIndSelPMSysVolHeating dlg( m_pclIndSelPMParams, dSupplyTemp, dReturnTemp, this );

	if( IDOK == dlg.DoModal() )
	{
		_UpdateInstalledPowerWaterContent();		
	}
}

void CDlgIndSelPMPanelHeatingBase::OnEnSetFocusSafetyTempLimiter()
{
	m_EditSafetyTLim.SetSel( 0, -1 );
	m_dSafetyTempLimiterSaved = m_EditSafetyTLim.GetCurrentValSI();
}

void CDlgIndSelPMPanelHeatingBase::OnEnSetFocusWaterHardness()
{
	m_EditWaterHardness.SetSel( 0, -1 );
	m_dWaterHardnessSaved = m_EditWaterHardness.GetCurrentValSI();
}

void CDlgIndSelPMPanelHeatingBase::OnEnChangeSafetyTempLimiter()
{
	// HYS-829: value is not changed if the window is not visible for the current mode 
	if( NULL == m_pclInterface || false == m_bOnEnChangeEnabled || false == IsWindowVisible() )
	{
		return;
	}

	CDlgIndSelPMPanelHCSBase::OnEnChangeEdit();

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSafetyTempLimiter( m_EditSafetyTLim.GetCurrentValSI() );
	UpdateP0();
}

void CDlgIndSelPMPanelHeatingBase::OnKillFocusEditSafetyTempLimiter()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pTADS || NULL == m_pclIndSelPMParams->m_pTADS->GetpTechParams() 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CDlgIndSelPressureMaintenance::InterfacePMPanels *pInterface = (CDlgIndSelPressureMaintenance::InterfacePMPanels *)m_pclInterface;

	if( NULL == pInterface )
	{
		return;
	}

	// Do nothing if no change.
	if( m_dSafetyTempLimiterSaved == m_EditSafetyTLim.GetCurrentValSI() )
	{
		return;
	}

	// Verify if we must change to EN12953 norm.
	if( _T("PM_NORM_EN12828" ) == CString( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetNormID() ) )
	{
		double dSafetyTempLimiterLimit = m_pclIndSelPMParams->m_pTADS->GetpTechParams()->GetSafetyTempLimiterLimitEN12953();

		if( m_EditSafetyTLim.GetCurrentValSI() > dSafetyTempLimiterLimit )
		{
			CString strTemp;
			CString strSafetyTempLimiter = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSAFETYTEMP );
			CString strSafetyTempLimiterLimit = WriteCUDouble( _U_TEMPERATURE, dSafetyTempLimiterLimit, true );
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERROREN12953, strSafetyTempLimiter, strSafetyTempLimiterLimit );
			strTemp += _T("\r\n\r\n") + TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_SWITCHEN12953 );
			
			if( IDYES == MessageBox( strTemp, _T(""), MB_YESNO | MB_ICONEXCLAMATION ) )
			{
				pInterface->m_pclParent->SetNorm( _T("PM_NORM_EN12953") );
			}
		}
	}

	// Verify temperatures.
	UINT uiError = AreHeatingSolarTemperaturesOK( &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	if( MEID_SafetyTempLimiter != ( MEID_SafetyTempLimiter & uiError ) )
	{
		InitToolTips( &m_EditSafetyTLim );
	}

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherHeatingSolarTemperatures( uiError, &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	UpdateP0();

	// Safety temperature limiter has an influence on p0. Thus we need to check pz input by user.
	CheckPz();

	// Safety temperature limiter has an influence on the PSV.
	CheckSafetyVRP();
}

void CDlgIndSelPMPanelHeatingBase::OnKillFocusEditSupplyTemp()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pTADS || NULL == m_pclIndSelPMParams->m_pTADS->GetpTechParams() 
			|| NULL ==  m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CDlgIndSelPressureMaintenance::InterfacePMPanels *pInterface = (CDlgIndSelPressureMaintenance::InterfacePMPanels *)m_pclInterface;

	if( NULL == pInterface )
	{
		return;
	}

	// Do nothing if no changed.
	if( m_dSupplyTempSaved == m_EditSupplyTemp.GetCurrentValSI() )
	{
		return;
	}

	// Verify if we must change to EN12953 norm.
	if( _T("PM_NORM_EN12828" ) == CString( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetNormID() ) )
	{
		double dSupplyTempLimit = m_pclIndSelPMParams->m_pTADS->GetpTechParams()->GetFluidTempLimitEN12953();

		if( m_EditSupplyTemp.GetCurrentValSI() > dSupplyTempLimit )
		{
			CString strTemp;
			CString strSupply = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSUPPLYTEMP );
			CString strSupplyTempLimit = WriteCUDouble( _U_TEMPERATURE, dSupplyTempLimit, true );
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERROREN12953, strSupply, strSupplyTempLimit );
			strTemp += _T("\r\n\r\n") + TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_SWITCHEN12953 );
			
			if( IDYES == MessageBox( strTemp, _T(""), MB_YESNO | MB_ICONEXCLAMATION ) )
			{
				pInterface->m_pclParent->SetNorm( _T("PM_NORM_EN12953") );
			}
		}
	}
	
	// Verify temperatures.
	UINT uiError = AreHeatingSolarTemperaturesOK( &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherHeatingSolarTemperatures( uiError, &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	// Update the static water content.
	_UpdateInstalledPowerWaterContent();
}

void CDlgIndSelPMPanelHeatingBase::OnKillFocusEditReturnTemp()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pTADS || NULL == m_pclIndSelPMParams->m_pTADS->GetpTechParams() 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CDlgIndSelPressureMaintenance::InterfacePMPanels *pInterface = (CDlgIndSelPressureMaintenance::InterfacePMPanels *)m_pclInterface;

	if( NULL == pInterface )
	{
		return;
	}

	// Do nothing if no changed.
	if( m_dReturnTempSaved == m_EditReturnTemp.GetCurrentValSI() )
	{
		return;
	}
	
	// Verify if we must change to EN12953 norm.
	if( _T("PM_NORM_EN12828" ) == CString( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetNormID() ) )
	{
		double dReturnTempLimit = m_pclIndSelPMParams->m_pTADS->GetpTechParams()->GetFluidTempLimitEN12953();

		if( m_EditReturnTemp.GetCurrentValSI() > dReturnTempLimit )
		{
			CString strTemp;
			CString strReturn = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORRETURNTEMP );
			CString strReturnTempLimit = WriteCUDouble( _U_TEMPERATURE, dReturnTempLimit, true );
			FormatString( strTemp, IDS_DLGINDSELPMPANELS_TTERROREN12953, strReturn, strReturnTempLimit );
			strTemp += _T("\r\n\r\n") + TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_SWITCHEN12953 );
			
			if( IDYES == MessageBox( strTemp, _T(""), MB_YESNO | MB_ICONEXCLAMATION ) )
			{
				pInterface->m_pclParent->SetNorm( _T("PM_NORM_EN12953") );
			}
		}
	}

	// Verify temperatures.
	UINT uiError = AreHeatingSolarTemperaturesOK( &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	if( MEID_ReturnTemperature != ( MEID_ReturnTemperature & uiError ) )
	{
		InitToolTips( &m_EditReturnTemp );
	}

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherHeatingSolarTemperatures( uiError, &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	// Update the static water content.
	_UpdateInstalledPowerWaterContent();

	// HYS-1054: In heating and solar mode set the m_EditDegassingMaxTemp value with m_EditReturnTemp value
	m_EditDegassingMaxTemp.SetCurrentValSI( m_EditReturnTemp.GetCurrentValSI() );
	m_EditDegassingMaxTemp.Update();
}

void CDlgIndSelPMPanelHeatingBase::OnKillFocusEditMinTemp()
{
	// Do nothing if no change.
	if( m_dMinTempSaved == m_EditMinTemp.GetCurrentValSI() )
	{
		return;
	}
	
	// Verify temperatures.
	UINT uiError = AreHeatingSolarTemperaturesOK( &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	if( MEID_MinTemperature != ( MEID_MinTemperature & uiError ) )
	{
		// If no more error with the minimum temperature, we reset the tooltip to the default value.
		InitToolTips( &m_EditMinTemp );
	}

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherHeatingSolarTemperatures( uiError, &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	// HYS-1135: The minimum temperature has an influence on the minimum pressure.
	UpdateP0();
}

void CDlgIndSelPMPanelHeatingBase::OnKillFocusEditFillTemp()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Do nothing if no changed.
	if( m_dFillTempSaved == m_EditFillTemp.GetCurrentValSI() )
	{
		return;
	}

	// Verify temperatures.
	UINT uiError = AreHeatingSolarTemperaturesOK( &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	if( MEID_FillTemperature != ( MEID_FillTemperature & uiError ) )
	{
		InitToolTips( &m_EditFillTemp );
	}

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherHeatingSolarTemperatures( uiError, &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpWaterTemp( m_EditFillTemp.GetCurrentValSI() );
}

void CDlgIndSelPMPanelHeatingBase::OnKillFocusEditWaterHardness()
{
	// Do nothing if no change.
	if( m_dWaterHardnessSaved == m_EditWaterHardness.GetCurrentValSI() )
	{
		return;
	}

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpWaterHardness( m_EditWaterHardness.GetCurrentValSI() );
}

void CDlgIndSelPMPanelHeatingBase::ResetToolTipsErrorBorder( CWnd *pWnd, bool bToolTips, bool bErrorBorder )
{
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		// Tooltips that are informations on a field.
		// No in heating.

		// Tooltips that are errors on a field.
		if( NULL == pWnd || pWnd == &m_EditSafetyTLim )
		{
			if( true == bToolTips )
			{
				m_ToolTip.DelTool( &m_EditSafetyTLim );
			}

			if( true == bErrorBorder )
			{
				m_EditSafetyTLim.ResetDrawBorder();
			}
		}
	}

	CDlgIndSelPMPanelHCSBase::ResetToolTipsErrorBorder( pWnd );
}

void CDlgIndSelPMPanelHeatingBase::UpdateVolumeSystemStatus( BOOL bEnableSysVol, BOOL bEnableInstPower )
{
	m_EditWaterContent.SetReadOnly( !bEnableSysVol );
	m_EditWaterContent.EnableWindow( bEnableSysVol );

	m_EditInstalledPower.SetReadOnly( !bEnableInstPower );
	m_EditInstalledPower.EnableWindow( bEnableInstPower );

	CButton *pclButton = (CButton *)GetDlgItem( IDC_BUTTON_WATERCONTENT );

	if( NULL != pclButton && NULL != pclButton->GetSafeHwnd() )
	{
		if( TRUE == bEnableInstPower && TRUE == bEnableSysVol )
		{
			pclButton->SetIcon( (HICON)LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDI_VOLUME_ICON ), IMAGE_ICON, 18, 18,
					LR_DEFAULTCOLOR ) );
		}
		else
		{
			pclButton->SetIcon( (HICON)LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDI_VOLUMEOK_ICON ), IMAGE_ICON, 18, 18,
					LR_DEFAULTCOLOR ) );
		}
	}
}

void CDlgIndSelPMPanelHeatingBase::_UpdateInstalledPowerWaterContent()
{
	double dTotalHeatWaterContent = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetTotalHeatWaterContent();

	if( 0.0 != dTotalHeatWaterContent )
	{
		m_EditWaterContent.SetCurrentValSI( dTotalHeatWaterContent );
		m_EditWaterContent.Update();
	}

	double dTotalHeatInstalledPower = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetTotalHeatInstalledPower();

	if( 0.0 != dTotalHeatInstalledPower )
	{
		m_EditInstalledPower.SetCurrentValSI( dTotalHeatInstalledPower );
		m_EditInstalledPower.Update();
	}

	UpdateVolumeSystemStatus( ( 0.0 == dTotalHeatWaterContent ) ? TRUE : FALSE, ( 0.0 == dTotalHeatInstalledPower ) ? TRUE : FALSE );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelHeating dialog
CDlgIndSelPMPanelHeating::CDlgIndSelPMPanelHeating( CIndSelPMParams *pclIndSelParams, CWnd *pParent )
	: CDlgIndSelPMPanelHeatingBase( pclIndSelParams, PanelType::ePanelHeating, CDlgIndSelPMPanelHeating::IDD, pParent )
{
}

bool CDlgIndSelPMPanelHeating::ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser || false == CDlgIndSelPMPanelHeatingBase::ApplyPMInputUser( pclPMInputUser, bAllowShowErrorMessage ) )
	{
		return false;
	}

	return VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

bool CDlgIndSelPMPanelHeating::VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser )
	{
		return false;
	}

	m_vecErrorList.clear();
	m_mapErrorMsg.clear();

	return CDlgIndSelPMPanelHeatingBase::VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelHeatingSWKI dialog
CDlgIndSelPMPanelHeatingSWKI::CDlgIndSelPMPanelHeatingSWKI( CIndSelPMParams *pclIndSelParams, CWnd *pParent )
	: CDlgIndSelPMPanelHeatingBase( pclIndSelParams, PanelType::ePanelHeatingSWKI, CDlgIndSelPMPanelHeatingSWKI::IDD, pParent )
{
	m_dStorageTankVolumeSaved = 0.0;
}

bool CDlgIndSelPMPanelHeatingSWKI::ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser || false == CDlgIndSelPMPanelHeatingBase::ApplyPMInputUser( pclPMInputUser, bAllowShowErrorMessage ) )
	{
		return false;
	}

	CString strToolTip;
	FormatString( strToolTip, IDS_DLGINDSELPMPANELS_STORAGETANKMAXTEMPTT, WriteCUDouble( _U_TEMPERATURE, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetStorageTankMaxTemp(), true ) );
	m_ToolTip.AddToolWindow( &m_BtnStorageTankMaxTemp, strToolTip );

	m_EditStorageTankVolume.SetCurrentValSI( pclPMInputUser->GetStorageTankVolume() );
	m_EditStorageTankVolume.Update();

	return VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

bool CDlgIndSelPMPanelHeatingSWKI::VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser )
	{
		return false;
	}

	m_vecErrorList.clear();
	m_mapErrorMsg.clear();

	// Check installed power.
	IsInstalledPowerOK( &m_EditInstalledPower, true );

	return CDlgIndSelPMPanelHeatingBase::VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

void CDlgIndSelPMPanelHeatingSWKI::OnNewDocument()
{
	CDlgIndSelPMPanelHeatingBase::OnNewDocument();

	m_EditStorageTankVolume.SetReadOnly( FALSE );
	m_EditStorageTankVolume.EnableWindow( TRUE );

	m_BtnStorageTankMaxTemp.SetIcon( (HICON)LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDI_STORAGETANKMAXTEMP ), IMAGE_ICON, 18, 18, LR_DEFAULTCOLOR ) );

	// To avoid any 'OnEnChange' event to be called.
	m_bOnEnChangeEnabled = false;

	// Don't call 'OnUnitChange' because in this method we call a 'FillPMInputUser' and 'ApplyPMInputUser' and this will change status of some
	// control that we don't need when 'OnNewDocument' is called.
	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	GetDlgItem( IDC_STATIC_STORAGETANKVOLUMEUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_EditStorageTankVolume, _U_VOLUME );

	m_bOnEnChangeEnabled = true;
}

void CDlgIndSelPMPanelHeatingSWKI::EnablePMSelection( bool Enable )
{
	CDlgIndSelPMPanelHeatingBase::EnablePMSelection( Enable );
	m_EditStorageTankVolume.ActiveSpecialValidation( Enable, this );
	Invalidate();
	UpdateWindow();
}

BEGIN_MESSAGE_MAP( CDlgIndSelPMPanelHeatingSWKI, CDlgIndSelPMPanelHeatingBase )
	ON_BN_CLICKED( IDC_BUTTON_STORAGETANKMAXTEMP, OnBnClickedStorageTankMaxTemp )
	ON_EN_SETFOCUS( IDC_EDIT_STORAGETANKVOLUME, OnEnSetFocusStorageTankVolume )
	ON_EN_KILLFOCUS( IDC_EDIT_STORAGETANKVOLUME, OnKillFocusStorageTankVolume )
END_MESSAGE_MAP()

void CDlgIndSelPMPanelHeatingSWKI::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelPMPanelHeatingBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_BUTTON_STORAGETANKMAXTEMP, m_BtnStorageTankMaxTemp );
	DDX_Control( pDX, IDC_STATIC_STORAGETANKVOLUME, m_StaticStorageTankVolume );
	DDX_Control( pDX, IDC_EDIT_STORAGETANKVOLUME, m_EditStorageTankVolume );
}

BOOL CDlgIndSelPMPanelHeatingSWKI::OnInitDialog()
{
	CDlgIndSelPMPanelHeatingBase::OnInitDialog();

	// Disable the 'OnNewDocument' handler to not interfere with this dialog.
	m_EditStorageTankVolume.SetOnNewDocumentHandler( false );

	// Change 'Water content' by 'System volume' for SWKI HE301-01.
	m_StaticSystemVolume.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_SYSTEMVOLUME ) );

	m_StaticStorageTankVolume.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PMHEATINGSWKI_STORAGETANKVOLUME ) );
	m_EditStorageTankVolume.ActiveSpecialValidation( true, this );

	return TRUE;
}

LRESULT CDlgIndSelPMPanelHeatingSWKI::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList )
	{
		// Not yet ready!
		return 0;
	}

	CDlgIndSelPMPanelHeatingBase::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	GetDlgItem( IDC_STATIC_STORAGETANKVOLUMEUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_EditStorageTankVolume, _U_VOLUME );

	return 0;
}

void CDlgIndSelPMPanelHeatingSWKI::OnBnClickedStorageTankMaxTemp()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		return;
	}

	CDlgStorageTankMaxTemperature dlg( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser(), this );
	dlg.DoModal();

	m_ToolTip.DelTool( &m_BtnStorageTankMaxTemp );
	CString strToolTip;
	FormatString( strToolTip, IDS_DLGINDSELPMPANELS_STORAGETANKMAXTEMPTT, WriteCUDouble( _U_TEMPERATURE, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetStorageTankMaxTemp(), true ) );
	m_ToolTip.AddToolWindow( &m_BtnStorageTankMaxTemp, strToolTip );
}

void CDlgIndSelPMPanelHeatingSWKI::OnEnSetFocusStorageTankVolume()
{
	m_EditStorageTankVolume.SetSel( 0, -1 );
	m_dStorageTankVolumeSaved = m_EditStorageTankVolume.GetCurrentValSI();
}

void CDlgIndSelPMPanelHeatingSWKI::OnKillFocusStorageTankVolume()
{
	// Do nothing if no change.
	if( m_dStorageTankVolumeSaved == m_EditStorageTankVolume.GetCurrentValSI() )
	{
		return;
	}

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetStorageTankVolume( m_EditStorageTankVolume.GetCurrentValSI() );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelCoolingBase dialog
CDlgIndSelPMPanelCoolingBase::CDlgIndSelPMPanelCoolingBase( CIndSelPMParams *pclIndSelParams, PanelType ePanelType, UINT nID, CWnd *pParent )
	: CDlgIndSelPMPanelHCSBase( pclIndSelParams, ePanelType, nID, pParent )
{
	m_dMaxTempSaved = 0.0;
	m_dFillTempSaved = 0.0;
}

void CDlgIndSelPMPanelCoolingBase::OnNormChanged()
{
	AreCoolingTemperaturesOK( &m_EditMaxTemp, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, m_clCheckMinTemp.GetCheck(), &m_EditFillTemp );
	ClearRightSheet();

	// Norm has an influence on the PSV.
	CheckSafetyVRP();

	// HYS-1407: Norm has an influence on the p0.
	UpdateP0();
}

bool CDlgIndSelPMPanelCoolingBase::ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser || false == CDlgIndSelPMPanelHCSBase::ApplyPMInputUser( pclPMInputUser, bAllowShowErrorMessage ) )
	{
		return false;
	}

	m_EditMaxTemp.SetCurrentValSI( pclPMInputUser->GetMaxTemperature() );
	m_EditMaxTemp.Update();
	
	// HYS-1054: Apply the current value of m_EditMaxTemp to m_EditDegassingMaxTemp
	pclPMInputUser->SetDegassingMaxTempConnectPoint( m_EditMaxTemp.GetCurrentValSI() );
	m_EditDegassingMaxTemp.SetCurrentValSI( pclPMInputUser->GetDegassingMaxTempConnectPoint() );
	m_EditDegassingMaxTemp.Update();

	m_clCheckMinTemp.SetCheck( ( true == pclPMInputUser->GetCheckMinTemperature() ) ? BST_CHECKED : BST_UNCHECKED );
	_UpdateMinTemperature();

	return true;
}

bool CDlgIndSelPMPanelCoolingBase::VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser )
	{
		return false;
	}

	// Check temperature fields.
	AreCoolingTemperaturesOK( &m_EditMaxTemp, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, m_clCheckMinTemp.GetCheck(), &m_EditFillTemp, true );

	pclPMInputUser->CheckInputData( &m_vecErrorList );
	return CDlgIndSelPMPanelHCSBase::VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

void CDlgIndSelPMPanelCoolingBase::OnNewDocument()
{
	CDlgIndSelPMPanelHCSBase::OnNewDocument();

	// To avoid any 'OnEnChange' event to be called.
	m_bOnEnChangeEnabled = false;

	// Don't call 'OnUnitChange' because in this method we call a 'FillPMInputUser' and 'ApplyPMInputUser' and this will change status of some
	// control that we don't need when 'OnNewDocument' is called.
	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	GetDlgItem( IDC_STATIC_MAXTEMPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditMaxTemp, _U_TEMPERATURE );

	GetDlgItem( IDC_STATIC_MINTEMPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditMinTemp, _U_TEMPERATURE );

	m_bOnEnChangeEnabled = true;
}

void CDlgIndSelPMPanelCoolingBase::EnablePMSelection( bool Enable )
{
	CDlgIndSelPMPanelHCSBase::EnablePMSelection( Enable );
	m_EditMaxTemp.ActiveSpecialValidation( Enable, this );
	m_EditMinTemp.ActiveSpecialValidation( Enable, this );
	Invalidate();
	UpdateWindow();
}

BEGIN_MESSAGE_MAP( CDlgIndSelPMPanelCoolingBase, CDlgIndSelPMPanelHCSBase )
	ON_EN_CHANGE( IDC_EDIT_MAXTEMP, OnEnChangeEdit )

	ON_BN_CLICKED( IDC_CHECKMINTEMP, OnBnClickedCheckMinTemp )
	ON_BN_CLICKED( IDC_BUTTON_WATERCONTENT, OnBnClickedWaterContent )

	ON_EN_SETFOCUS( IDC_EDIT_MAXTEMP, OnEnSetFocusMaxTemp )

	ON_EN_KILLFOCUS( IDC_EDIT_MAXTEMP, OnKillFocusEditMaxTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_SUPPLYTEMP, OnKillFocusEditSupplyTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_RETURNTEMP, OnKillFocusEditReturnTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_MINTEMP, OnKillFocusEditMinTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_FILLTEMP, OnKillFocusEditFillTemp )
END_MESSAGE_MAP()

void CDlgIndSelPMPanelCoolingBase::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelPMPanelHCSBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDIT_MAXTEMP, m_EditMaxTemp );
	DDX_Control( pDX, IDC_CHECKMINTEMP, m_clCheckMinTemp );
}

BOOL CDlgIndSelPMPanelCoolingBase::OnInitDialog()
{
	CDlgIndSelPMPanelHCSBase::OnInitDialog();

	CButton *bt = ( CButton * )GetDlgItem( IDC_BUTTON_WATERCONTENT );

	if( bt && bt->GetSafeHwnd() )
	{
		bt->SetIcon( ( HICON )LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDI_VOLUME_ICON ), IMAGE_ICON, 18, 18,
										 LR_DEFAULTCOLOR ) );
	}

	GetDlgItem( IDC_STATIC_MAXTEMP )->SetWindowText( TASApp.LoadLocalizedString( IDS_PM_MAXTEMP ) );
	GetDlgItem( IDC_STATIC_MINTEMP )->SetWindowText( TASApp.LoadLocalizedString( IDS_PM_MINTEMP ) );

	// Disable the 'OnNewDocument' handler to not interfere with this dialog.
	m_EditMaxTemp.SetOnNewDocumentHandler( false );

	m_EditMaxTemp.ActiveSpecialValidation( true, this );
	m_EditMinTemp.ActiveSpecialValidation( true, this );

	return TRUE;
}

LRESULT CDlgIndSelPMPanelCoolingBase::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList )
	{
		// Not yet ready!
		return 0;
	}

	CDlgIndSelPMPanelHCSBase::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	GetDlgItem( IDC_STATIC_MAXTEMPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditMaxTemp, _U_TEMPERATURE );

	GetDlgItem( IDC_STATIC_MINTEMPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditMinTemp, _U_TEMPERATURE );

	return 0;
}

void CDlgIndSelPMPanelCoolingBase::OnBnClickedCheckMinTemp()
{
	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetCheckMinTemperature( ( BST_CHECKED == m_clCheckMinTemp.GetCheck() ) ? true : false );

	if( BST_CHECKED == m_clCheckMinTemp.GetCheck() )
	{
		m_EditMinTemp.EnableWindow( TRUE );
		
		// HYS-1128: When user check the 'Min. temp.' checkbox we set 5.0 °C by default.
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetMinTemperature( 5.0 );
	}
	else
	{
		// HYS-1128: we now take always the freezing point when checkbox is not checked.
		double dMinTemperature = m_pclIndSelPMParams->m_WC.GetTfreez();

		m_EditMinTemp.EnableWindow( FALSE );
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetMinTemperature( dMinTemperature );
	}
	// HYS-1554 : Avoid to update min temp on this way if the panel is not the current one because edit fields will not
	// have the good values.
	bool bCheckMinTempNeeded = true;
	if( ePanelCoolingSWKI == GetPanelType() && PN_SWKIHE301_01 != m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetNorm() )
	{
		bCheckMinTempNeeded = false;
	}
	if( m_pclIndSelPMParams->m_eApplicationType == Cooling && true == bCheckMinTempNeeded )
	{
		UINT uiError = _UpdateMinTemperature();

		// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
		UpdateOtherCoolingTemperatures( uiError, &m_EditMaxTemp, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, m_clCheckMinTemp.GetCheck(), &m_EditFillTemp );
	}
}

void CDlgIndSelPMPanelCoolingBase::OnBnClickedWaterContent()
{
	double dSupplyTemp = m_EditSupplyTemp.GetCurrentValSI();
	double dReturnTemp = m_EditReturnTemp.GetCurrentValSI();
	CDlgIndSelPMSysVolCooling dlg( m_pclIndSelPMParams, dSupplyTemp, dReturnTemp, this );

	if( IDOK == dlg.DoModal() )
	{
		_UpdateInstalledPowerWaterContent();
	}
}

void CDlgIndSelPMPanelCoolingBase::OnEnSetFocusMaxTemp()
{
	m_EditMaxTemp.SetSel( 0, -1 );
	m_dMaxTempSaved = m_EditMaxTemp.GetCurrentValSI();
}

void CDlgIndSelPMPanelCoolingBase::OnKillFocusEditMaxTemp()
{
	// Do nothing if no change.
	if( m_dMaxTempSaved == m_EditMaxTemp.GetCurrentValSI() )
	{
		return;
	}

	// Verify temperatures.
	UINT uiError = AreCoolingTemperaturesOK( &m_EditMaxTemp, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, m_clCheckMinTemp.GetCheck(), &m_EditFillTemp );

	if( MEID_MaxTemperature != ( MEID_MaxTemperature & uiError ) )
	{
		InitToolTips( &m_EditMaxTemp );
	}

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherCoolingTemperatures( uiError, &m_EditMaxTemp, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, m_clCheckMinTemp.GetCheck(), &m_EditFillTemp );

	// Update the static water content.
	_UpdateInstalledPowerWaterContent();

	// HYS-1054: Update m_EditDegassingMaxTemp with current m_EditMaxTemp value
	m_EditDegassingMaxTemp.SetCurrentValSI( m_EditMaxTemp.GetCurrentValSI() );
	m_EditDegassingMaxTemp.Update();
}

void CDlgIndSelPMPanelCoolingBase::OnKillFocusEditSupplyTemp()
{
	// Do nothing if no changed.
	if( m_dSupplyTempSaved == m_EditSupplyTemp.GetCurrentValSI() )
	{
		return;
	}

	// Verify temperatures.
	UINT uiError = AreCoolingTemperaturesOK( &m_EditMaxTemp, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, m_clCheckMinTemp.GetCheck(), &m_EditFillTemp );

	if( MEID_SupplyTemperature != ( MEID_SupplyTemperature & uiError ) )
	{
		InitToolTips( &m_EditSupplyTemp );
	}

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherCoolingTemperatures( uiError, &m_EditMaxTemp, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, m_clCheckMinTemp.GetCheck(), &m_EditFillTemp );

	// Update the static water content.
	_UpdateInstalledPowerWaterContent();
}

void CDlgIndSelPMPanelCoolingBase::OnKillFocusEditReturnTemp()
{
	// Do nothing if no changed.
	if( m_dReturnTempSaved == m_EditReturnTemp.GetCurrentValSI() )
	{
		return;
	}

	// Verify temperatures.
	UINT uiError = AreCoolingTemperaturesOK( &m_EditMaxTemp, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, m_clCheckMinTemp.GetCheck(), &m_EditFillTemp );

	if( MEID_ReturnTemperature != ( MEID_ReturnTemperature & uiError ) )
	{
		InitToolTips( &m_EditReturnTemp );
		
		m_EditDegassingMaxTemp.SetCurrentValSI( m_EditMaxTemp.GetCurrentValSI() );
		m_EditDegassingMaxTemp.Update();
	}

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherCoolingTemperatures( uiError, &m_EditMaxTemp, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, m_clCheckMinTemp.GetCheck(), &m_EditFillTemp );

	// Update the static water content.
	_UpdateInstalledPowerWaterContent();
}

void CDlgIndSelPMPanelCoolingBase::OnKillFocusEditMinTemp()
{
	// Do nothing if no change.
	if( m_dMinTempSaved == m_EditMinTemp.GetCurrentValSI() )
	{
		return;
	}

	// Verify temperatures.
	UINT uiError = AreCoolingTemperaturesOK( &m_EditMaxTemp, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, m_clCheckMinTemp.GetCheck(), &m_EditFillTemp );

	if( MEID_MinTemperature != ( MEID_MinTemperature & uiError ) )
	{
		InitToolTips( &m_EditMinTemp );
	}

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherCoolingTemperatures( uiError, &m_EditMaxTemp, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, m_clCheckMinTemp.GetCheck(), &m_EditFillTemp );

	// HYS-1135: The minimum temperature has an influence on the minimum pressure.
	UpdateP0();
}

void CDlgIndSelPMPanelCoolingBase::OnKillFocusEditFillTemp()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Do nothing if no changed.
	if( m_dFillTempSaved == m_EditFillTemp.GetCurrentValSI() )
	{
		return;
	}

	// Verify temperatures.
	UINT uiError = AreCoolingTemperaturesOK( &m_EditMaxTemp, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, m_clCheckMinTemp.GetCheck(), &m_EditFillTemp );

	if( MEID_FillTemperature != ( MEID_FillTemperature & uiError ) )
	{
		InitToolTips( &m_EditFillTemp );
	}

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherCoolingTemperatures( uiError, &m_EditMaxTemp, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, m_clCheckMinTemp.GetCheck(), &m_EditFillTemp );
	// HYS-1054: WM temperature field is no more available for Cooling mode, but its value is kept and aqual to fill temp 
	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpWaterTemp( m_EditFillTemp.GetCurrentValSI() );
}

void CDlgIndSelPMPanelCoolingBase::ResetToolTipsErrorBorder( CWnd *pWnd, bool fToolTips, bool fErrorBorder )
{
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		// Tooltips that are information on a field.
		if( NULL == pWnd || pWnd == &m_EditMaxTemp )
		{
			if( true == fToolTips )
			{
				m_ToolTip.DelTool( &m_EditMaxTemp );
			}

			if( true == fErrorBorder )
			{
				m_EditMaxTemp.ResetDrawBorder();
			}
		}

		// Tooltips that are information or error.
		if (NULL == pWnd || pWnd == &m_EditMinTemp)
		{
			if (true == fToolTips)
			{
				m_ToolTip.DelTool(&m_EditMinTemp);
			}

			if (true == fErrorBorder)
			{
				m_EditMinTemp.ResetDrawBorder();
			}
		}
	}

	CDlgIndSelPMPanelHCSBase::ResetToolTipsErrorBorder( pWnd );
}

void CDlgIndSelPMPanelCoolingBase::InitToolTips( CWnd *pWnd )
{
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		CString TTstr;

		if( NULL == pWnd || pWnd == &m_EditMaxTemp )
		{
			ResetToolTipsErrorBorder( &m_EditMaxTemp );
			TTstr = TASApp.LoadLocalizedString( IDS_PM_TTMAXTEMP );
			m_ToolTip.AddToolWindow( &m_EditMaxTemp, TTstr );
		}

	}

	CDlgIndSelPMPanelHCSBase::InitToolTips( pWnd );
}

void CDlgIndSelPMPanelCoolingBase::UpdateVolumeSystemStatus( BOOL bEnableSysVol, BOOL bEnableInstPower )
{
	m_EditWaterContent.SetReadOnly( !bEnableSysVol );
	m_EditWaterContent.EnableWindow( bEnableSysVol );

	m_EditInstalledPower.SetReadOnly( !bEnableInstPower );
	m_EditInstalledPower.EnableWindow( bEnableInstPower );

	CButton *bt = (CButton *)GetDlgItem( IDC_BUTTON_WATERCONTENT );

	if( bt && bt->GetSafeHwnd() )
	{
		if( TRUE == bEnableInstPower && TRUE == bEnableSysVol )
		{
			bt->SetIcon( (HICON)LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDI_VOLUME_ICON ), IMAGE_ICON, 18, 18,
					LR_DEFAULTCOLOR ) );
		}
		else
		{
			bt->SetIcon( (HICON)LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDI_VOLUMEOK_ICON ), IMAGE_ICON, 18, 18,
					LR_DEFAULTCOLOR ) );
		}
	}
}

void CDlgIndSelPMPanelCoolingBase::_UpdateInstalledPowerWaterContent()
{
	double dTotalColdWaterContent = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetTotalColdWaterContent();

	if( 0.0 != dTotalColdWaterContent )
	{
		m_EditWaterContent.SetCurrentValSI( dTotalColdWaterContent );
		m_EditWaterContent.Update();
	}

	double dTotalColdInstalledPower = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetTotalColdInstalledPower();

	if( 0.0 != dTotalColdInstalledPower )
	{
		m_EditInstalledPower.SetCurrentValSI( dTotalColdInstalledPower );
		m_EditInstalledPower.Update();
	}

	UpdateVolumeSystemStatus( ( 0.0 == dTotalColdWaterContent ) ? TRUE : FALSE, ( 0.0 == dTotalColdInstalledPower ) ? TRUE : FALSE );
}

UINT CDlgIndSelPMPanelCoolingBase::_UpdateMinTemperature()
{
	m_EditMinTemp.SetCurrentValSI( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinTemperature() );
	m_EditMinTemp.Update();

	UINT uiError = AreCoolingTemperaturesOK( &m_EditMaxTemp, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, m_clCheckMinTemp.GetCheck(), &m_EditFillTemp );
	return uiError;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelCooling dialog
CDlgIndSelPMPanelCooling::CDlgIndSelPMPanelCooling( CIndSelPMParams *pclIndSelParams, CWnd *pParent )
	: CDlgIndSelPMPanelCoolingBase( pclIndSelParams, PanelType::ePanelCooling, CDlgIndSelPMPanelCooling::IDD, pParent )
{
}

bool CDlgIndSelPMPanelCooling::ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser || false == CDlgIndSelPMPanelCoolingBase::ApplyPMInputUser( pclPMInputUser, bAllowShowErrorMessage ) )
	{
		return false;
	}

	return VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

bool CDlgIndSelPMPanelCooling::VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser )
	{
		return false;
	}

	m_vecErrorList.clear();
	m_mapErrorMsg.clear();

	return CDlgIndSelPMPanelCoolingBase::VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelCoolingSWKI dialog
CDlgIndSelPMPanelCoolingSWKI::CDlgIndSelPMPanelCoolingSWKI( CIndSelPMParams *pclIndSelParams, CWnd *pParent )
	: CDlgIndSelPMPanelCoolingBase( pclIndSelParams, PanelType::ePanelCoolingSWKI, CDlgIndSelPMPanelCoolingSWKI::IDD, pParent )
{
	m_dStorageTankVolumeSaved = 0.0;
}

bool CDlgIndSelPMPanelCoolingSWKI::ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser || false == CDlgIndSelPMPanelCoolingBase::ApplyPMInputUser( pclPMInputUser, bAllowShowErrorMessage ) )
	{
		return false;
	}

	CString strToolTip;
	FormatString( strToolTip, IDS_DLGINDSELPMPANELS_STORAGETANKMAXTEMPTT, WriteCUDouble( _U_TEMPERATURE, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetStorageTankMaxTemp(), true ) );
	m_ToolTip.AddToolWindow( &m_BtnStorageTankMaxTemp, strToolTip );

	m_EditStorageTankVolume.SetCurrentValSI( pclPMInputUser->GetStorageTankVolume() );
	m_EditStorageTankVolume.Update();

	return VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

bool CDlgIndSelPMPanelCoolingSWKI::VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser )
	{
		return false;
	}

	m_vecErrorList.clear();
	m_mapErrorMsg.clear();

	// Check installed power.
	IsInstalledPowerOK( &m_EditInstalledPower, true );

	return CDlgIndSelPMPanelCoolingBase::VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

void CDlgIndSelPMPanelCoolingSWKI::OnNewDocument()
{
	CDlgIndSelPMPanelCoolingBase::OnNewDocument();

	m_EditStorageTankVolume.SetReadOnly( FALSE );
	m_EditStorageTankVolume.EnableWindow( TRUE );

	m_BtnStorageTankMaxTemp.SetIcon( (HICON)LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDI_STORAGETANKMAXTEMP ), IMAGE_ICON, 
			18, 18, LR_DEFAULTCOLOR ) );

	// To avoid any 'OnEnChange' event to be called.
	m_bOnEnChangeEnabled = false;

	// Don't call 'OnUnitChange' because in this method we call a 'FillPMInputUser' and 'ApplyPMInputUser' and this will change status of some
	// control that we don't need when 'OnNewDocument' is called.
	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	GetDlgItem( IDC_STATIC_STORAGETANKVOLUMEUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_EditStorageTankVolume, _U_VOLUME );

	m_bOnEnChangeEnabled = true;
}

void CDlgIndSelPMPanelCoolingSWKI::EnablePMSelection( bool Enable )
{
	CDlgIndSelPMPanelCoolingBase::EnablePMSelection( Enable );
	m_EditStorageTankVolume.ActiveSpecialValidation( Enable, this );
	Invalidate();
	UpdateWindow();
}

BEGIN_MESSAGE_MAP( CDlgIndSelPMPanelCoolingSWKI, CDlgIndSelPMPanelCoolingBase )
	ON_BN_CLICKED( IDC_BUTTON_STORAGETANKMAXTEMP, OnBnClickedStorageTankMaxTemp )
	ON_EN_SETFOCUS( IDC_EDIT_STORAGETANKVOLUME, OnEnSetFocusStorageTankVolume )
	ON_EN_KILLFOCUS( IDC_EDIT_STORAGETANKVOLUME, OnKillFocusStorageTankVolume )
END_MESSAGE_MAP()

void CDlgIndSelPMPanelCoolingSWKI::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelPMPanelCoolingBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_BUTTON_STORAGETANKMAXTEMP, m_BtnStorageTankMaxTemp );
	DDX_Control( pDX, IDC_STATIC_STORAGETANKVOLUME, m_StaticStorageTankVolume );
	DDX_Control( pDX, IDC_EDIT_STORAGETANKVOLUME, m_EditStorageTankVolume );
}

BOOL CDlgIndSelPMPanelCoolingSWKI::OnInitDialog()
{
	CDlgIndSelPMPanelCoolingBase::OnInitDialog();

	// Disable the 'OnNewDocument' handler to not interfere with this dialog.
	m_EditStorageTankVolume.SetOnNewDocumentHandler( false );

	// Change 'Water content' by 'System volume' for SWKI HE301-01.
	m_StaticSystemVolume.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_SYSTEMVOLUME ) );

	m_StaticStorageTankVolume.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PMHEATINGSWKI_STORAGETANKVOLUME ) );
	m_EditStorageTankVolume.ActiveSpecialValidation( true, this );

	return TRUE;
}

LRESULT CDlgIndSelPMPanelCoolingSWKI::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList )
	{
		// Not yet ready!
		return 0;
	}

	CDlgIndSelPMPanelCoolingBase::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	GetDlgItem( IDC_STATIC_STORAGETANKVOLUMEUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_EditStorageTankVolume, _U_VOLUME );

	return 0;
}

void CDlgIndSelPMPanelCoolingSWKI::OnBnClickedStorageTankMaxTemp()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		return;
	}

	CDlgStorageTankMaxTemperature dlg( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser(), this );
	dlg.DoModal();

	m_ToolTip.DelTool( &m_BtnStorageTankMaxTemp );
	CString strToolTip;
	FormatString( strToolTip, IDS_DLGINDSELPMPANELS_STORAGETANKMAXTEMPTT, WriteCUDouble( _U_TEMPERATURE, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetStorageTankMaxTemp(), true ) );
	m_ToolTip.AddToolWindow( &m_BtnStorageTankMaxTemp, strToolTip );
}

void CDlgIndSelPMPanelCoolingSWKI::OnEnSetFocusStorageTankVolume()
{
	m_EditStorageTankVolume.SetSel( 0, -1 );
	m_dStorageTankVolumeSaved = m_EditStorageTankVolume.GetCurrentValSI();
}

void CDlgIndSelPMPanelCoolingSWKI::OnKillFocusStorageTankVolume()
{
	// Do nothing if no change.
	if( m_dStorageTankVolumeSaved == m_EditStorageTankVolume.GetCurrentValSI() )
	{
		return;
	}

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetStorageTankVolume( m_EditStorageTankVolume.GetCurrentValSI() );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelSolarBase dialog
CDlgIndSelPMPanelSolarBase::CDlgIndSelPMPanelSolarBase( CIndSelPMParams *pclIndSelParams, PanelType ePanelType, UINT nID, CWnd *pParent )
	: CDlgIndSelPMPanelHCSBase( pclIndSelParams, ePanelType, nID, pParent )
{
	m_dSolarContentSaved = 0.0;
	m_dSafetyTempLimiterSaved = 0.0;
}

void CDlgIndSelPMPanelSolarBase::OnNormChanged()
{
	AreHeatingSolarTemperaturesOK( &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );
	ClearRightSheet();

	// Norm has an influence on the PSV.
	CheckSafetyVRP();

	// HYS-1407: Norm has an influence on the p0.
	UpdateP0();
}

bool CDlgIndSelPMPanelSolarBase::ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser || false == CDlgIndSelPMPanelHCSBase::ApplyPMInputUser( pclPMInputUser, bAllowShowErrorMessage ) )
	{
		return false;
	}

	m_EditSolarContent.SetCurrentValSI( pclPMInputUser->GetSolarCollectorVolume() );
	m_EditSolarContent.Update();

	// HYS-1350: forgot to initialize also this field.
	m_EditMinTemp.SetCurrentValSI( pclPMInputUser->GetMinTemperature() );
	m_EditMinTemp.Update();

	m_EditSafetyTLim.SetCurrentValSI( pclPMInputUser->GetSafetyTempLimiter() );
	m_EditSafetyTLim.Update();

	return true;
}

bool CDlgIndSelPMPanelSolarBase::VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser )
	{
		return false;
	}

	// Check solar content.

	// By default reset error.
	m_EditSolarContent.ResetDrawBorder();
	ResetToolTipsErrorBorder( &m_EditSolarContent );

	// After reset reload default values.
	InitToolTips( NULL );

	if( m_EditSolarContent.GetCurrentValSI() >= m_EditWaterContent.GetCurrentValSI() )
	{
		CString strToolTip;
		CString strTemp = CString( WriteCUDouble( _U_VOLUME, m_EditSolarContent.GetCurrentValSI(), true, -1, 2 ) )
							  + _T(" >= ") + CString( WriteCUDouble( _U_VOLUME, m_EditWaterContent.GetCurrentValSI(), true, -1, 2 ) );
		FormatString( strToolTip, IDS_DLGINDSELPMPANELS_TTERRORSOLARCONTENTTOHIGH, strTemp );

		m_ToolTip.AddToolWindow( &m_EditSolarContent, strToolTip );
		m_EditSolarContent.SetDrawBorder( true, _RED );
	}

	// Check temperature fields.
	AreHeatingSolarTemperaturesOK( &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp, true );

	pclPMInputUser->CheckInputData( &m_vecErrorList );

	CString msg;

	for( std::vector<std::pair<int, int>>::iterator iterError = m_vecErrorList.begin(); iterError != m_vecErrorList.end();
		 iterError++ )
	{
		if( CPMInputUser::ePM_InputEmpty == iterError->first && CPMInputUser::RACIE_SolarContent == iterError->second )
		{
			msg = _T("\r\n - ") + TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSOLARCONTENT );
			m_mapErrorMsg[MEID_SolarContent] = msg;

			ResetToolTipsErrorBorder( &m_EditSolarContent );
			msg = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSOLARCONTENT );
			m_ToolTip.AddToolWindow( &m_EditSolarContent, msg );
			m_EditSolarContent.SetDrawBorder( true, _RED );
			continue;
		}
	}

	return CDlgIndSelPMPanelHCSBase::VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

void CDlgIndSelPMPanelSolarBase::OnNewDocument()
{
	CDlgIndSelPMPanelHCSBase::OnNewDocument();

	// To avoid any 'OnEnChange' event to be called.
	m_bOnEnChangeEnabled = false;

	// Don't call 'OnUnitChange' because in this method we call a 'FillPMInputUser' and 'ApplyPMInputUser' and this will change status of some
	// control that we don't need when 'OnNewDocument' is called.
	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();
	GetDlgItem( IDC_STATIC_SOLARCONTENTUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_EditSolarContent, _U_VOLUME );

	GetDlgItem( IDC_STATIC_SAFETYTEMPLIMUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditSafetyTLim, _U_TEMPERATURE );

	m_bOnEnChangeEnabled = true;
}

void CDlgIndSelPMPanelSolarBase::EnablePMSelection( bool Enable )
{
	CDlgIndSelPMPanelHCSBase::EnablePMSelection( Enable );
	m_EditSolarContent.ActiveSpecialValidation( Enable, this );
	m_EditSafetyTLim.ActiveSpecialValidation( Enable, this );
	Invalidate();
	UpdateWindow();
}

BEGIN_MESSAGE_MAP( CDlgIndSelPMPanelSolarBase, CDlgIndSelPMPanelHCSBase )
	ON_EN_CHANGE( IDC_EDIT_SOLARCONTENT, OnEnChangeEdit )
	ON_EN_CHANGE( IDC_EDIT_SAFETYTEMPLIM, OnEnChangeEdit )
	ON_EN_CHANGE( IDC_EDIT_SUPPLYTEMP, OnEnChangeEdit )
	ON_EN_CHANGE( IDC_EDIT_RETURNTEMP, OnEnChangeEdit )

	ON_EN_SETFOCUS( IDC_EDIT_SOLARCONTENT, OnEnSetFocusSolarCollector )
	ON_EN_SETFOCUS( IDC_EDIT_SAFETYTEMPLIM, OnEnSetFocusSafetyTempLimiter )

	ON_EN_KILLFOCUS( IDC_EDIT_SOLARCONTENT, OnKillFocusSolarCollector )
	ON_EN_KILLFOCUS( IDC_EDIT_SAFETYTEMPLIM, OnKillFocusEditSafetyTempLimiter )
	ON_EN_KILLFOCUS( IDC_EDIT_SUPPLYTEMP, OnKillFocusEditSupplyTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_RETURNTEMP, OnKillFocusEditReturnTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_MINTEMP, OnKillFocusEditMinTemp )
	ON_EN_KILLFOCUS( IDC_EDIT_FILLTEMP, OnKillFocusEditFillTemp )
END_MESSAGE_MAP()

void CDlgIndSelPMPanelSolarBase::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelPMPanelHCSBase::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_SOLARCONTENT, m_EditSolarContent);
	DDX_Control(pDX, IDC_EDIT_SAFETYTEMPLIM, m_EditSafetyTLim);
}

BOOL CDlgIndSelPMPanelSolarBase::OnInitDialog()
{
	CDlgIndSelPMPanelHCSBase::OnInitDialog();

	GetDlgItem( IDC_BUTTON_WATERCONTENT )->EnableWindow( FALSE );
	GetDlgItem( IDC_BUTTON_WATERCONTENT )->ShowWindow( SW_HIDE );

	GetDlgItem( IDC_STATIC_SOLARCONTENT )->SetWindowText( TASApp.LoadLocalizedString( IDS_PM_SOLARCONTENT ) );
	GetDlgItem( IDC_STATIC_SAFETYTEMPLIM )->SetWindowText( TASApp.LoadLocalizedString( IDS_PMSOLAR_SAFETYTEMPLIM ) );

	// Disable the 'OnNewDocument' handler to not interfere with this dialog.
	m_EditSolarContent.SetOnNewDocumentHandler( false );
	m_EditSafetyTLim.SetOnNewDocumentHandler( false );

	m_EditSolarContent.ActiveSpecialValidation( true, this );
	m_EditSafetyTLim.ActiveSpecialValidation( true, this );

	return TRUE;
}

LRESULT CDlgIndSelPMPanelSolarBase::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList )
	{
		// Not yet ready!
		return 0;
	}

	CDlgIndSelPMPanelHCSBase::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();
	GetDlgItem( IDC_STATIC_SOLARCONTENTUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_EditSolarContent, _U_VOLUME );

	GetDlgItem( IDC_STATIC_SAFETYTEMPLIMUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditSafetyTLim, _U_TEMPERATURE );

	return 0;
}

void CDlgIndSelPMPanelSolarBase::OnEnSetFocusSolarCollector()
{
	m_EditSolarContent.SetSel( 0, -1 );
	m_dSolarContentSaved = m_EditSolarContent.GetCurrentValSI();
}

void CDlgIndSelPMPanelSolarBase::OnEnSetFocusSafetyTempLimiter()
{
	m_EditSafetyTLim.SetSel( 0, -1 );
	m_dSafetyTempLimiterSaved = m_EditSafetyTLim.GetCurrentValSI();
}

void CDlgIndSelPMPanelSolarBase::OnEnChangeSafetyTempLimiter()
{
	// HYS-829: value is not changed if the window is not visible for the current mode 
	if( NULL == m_pclInterface || false == m_bOnEnChangeEnabled || false == IsWindowVisible() )
	{
		return;
	}

	CDlgIndSelPMPanelHCSBase::OnEnChangeEdit();

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSafetyTempLimiter( m_EditSafetyTLim.GetCurrentValSI() );
	UpdateP0();
}

void CDlgIndSelPMPanelSolarBase::OnKillFocusSolarCollector()
{
	// Do nothing if no change.
	if( m_dSolarContentSaved == m_EditSolarContent.GetCurrentValSI() )
	{
		return;
	}

	m_EditSolarContent.ResetDrawBorder();
	ResetToolTipsErrorBorder( &m_EditSolarContent );

	// After reset reload default values.
	InitToolTips( NULL );

	if( m_EditSolarContent.GetCurrentValSI() >= m_EditWaterContent.GetCurrentValSI() )
	{
		CString strToolTip;
		CString strTemp = CString( WriteCUDouble( _U_VOLUME, m_EditSolarContent.GetCurrentValSI(), true, -1, 2 ) )
							  + _T(" >= ") + CString( WriteCUDouble( _U_VOLUME, m_EditWaterContent.GetCurrentValSI(), true, -1, 2 ) );
		FormatString( strToolTip, IDS_DLGINDSELPMPANELS_TTERRORSOLARCONTENTTOHIGH, strTemp );

		m_ToolTip.AddToolWindow( &m_EditSolarContent, strToolTip );
		m_EditSolarContent.SetDrawBorder( true, _RED );
	}
	else
	{
		InitToolTips( &m_EditSolarContent );
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSolarCollectorVolume( m_EditSolarContent.GetCurrentValSI() );
	}
}

void CDlgIndSelPMPanelSolarBase::OnKillFocusEditSafetyTempLimiter()
{
	// Do nothing if no change.
	if( m_dSafetyTempLimiterSaved == m_EditSafetyTLim.GetCurrentValSI() )
	{
		return;
	}

	// Verify temperatures.
	UINT uiError = AreHeatingSolarTemperaturesOK( &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	if( MEID_SafetyTempLimiter != ( MEID_SafetyTempLimiter & uiError ) )
	{
		InitToolTips( &m_EditSafetyTLim );
	}

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherHeatingSolarTemperatures( uiError, &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	UpdateP0();

	// Safety temperature limiter has an influence on p0. Thus we need to check pz input by user.
	CheckPz();

	// Safety temperature limiter has an influence on the PSV.
	CheckSafetyVRP();
}

void CDlgIndSelPMPanelSolarBase::OnKillFocusEditSupplyTemp()
{
	// Do nothing if no changed.
	if( m_dSupplyTempSaved == m_EditSupplyTemp.GetCurrentValSI() )
	{
		return;
	}

	// Verify temperatures.
	UINT uiError = AreHeatingSolarTemperaturesOK( &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherHeatingSolarTemperatures( uiError, &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );
}

void CDlgIndSelPMPanelSolarBase::OnKillFocusEditReturnTemp()
{
	// Do nothing if no changed.
	if( m_dReturnTempSaved == m_EditReturnTemp.GetCurrentValSI() )
	{
		return;
	}

	// Verify temperatures.
	UINT uiError = AreHeatingSolarTemperaturesOK( &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	if( MEID_ReturnTemperature != ( MEID_ReturnTemperature & uiError ) )
	{
		InitToolTips( &m_EditReturnTemp );
	}

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherHeatingSolarTemperatures( uiError, &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	m_EditDegassingMaxTemp.SetCurrentValSI( m_EditReturnTemp.GetCurrentValSI() );
	m_EditDegassingMaxTemp.Update();
}

void CDlgIndSelPMPanelSolarBase::OnKillFocusEditMinTemp()
{
	// Do nothing if no changed.
	if( m_dMinTempSaved == m_EditMinTemp.GetCurrentValSI() )
	{
		return;
	}

	// Verify temperatures.
	UINT uiError = AreHeatingSolarTemperaturesOK( &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	if( MEID_MinTemperature != ( MEID_MinTemperature & uiError ) )
	{
		// If no more error with the minimum temperature, we reset the tooltip to the default value.
		InitToolTips( &m_EditMinTemp );
	}

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherHeatingSolarTemperatures( uiError, &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	// HYS-1135: The minimum temperature has an influence on the minimum pressure.
	UpdateP0();
}

void CDlgIndSelPMPanelSolarBase::OnKillFocusEditFillTemp()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Do nothing if no changed.
	if( m_dFillTempSaved == m_EditFillTemp.GetCurrentValSI() )
	{
		return;
	}

	// Verify temperatures.
	UINT uiError = AreHeatingSolarTemperaturesOK( &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	if( MEID_FillTemperature != ( MEID_FillTemperature & uiError ) )
	{
		InitToolTips( &m_EditFillTemp );
	}

	// Verify also if other temperatures are yet in error or not following the change of the minimum temperature.
	UpdateOtherHeatingSolarTemperatures( uiError, &m_EditSafetyTLim, &m_EditSupplyTemp, &m_EditReturnTemp, &m_EditMinTemp, &m_EditFillTemp );

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpWaterTemp( m_EditFillTemp.GetCurrentValSI() );
}

void CDlgIndSelPMPanelSolarBase::ResetToolTipsErrorBorder( CWnd *pWnd, bool fToolTips, bool fErrorBorder )
{
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		// Tooltips that are informations on a field.
		if( NULL == pWnd || pWnd == &m_EditSolarContent )
		{
			if( true == fToolTips )
			{
				m_ToolTip.DelTool( &m_EditSolarContent );
			}

			if( true == fErrorBorder )
			{
				m_EditSolarContent.ResetDrawBorder();
			}
		}

		// Tooltips that are errors on a field.
		if( NULL == pWnd || pWnd == &m_EditSafetyTLim )
		{
			if( true == fToolTips )
			{
				m_ToolTip.DelTool( &m_EditSafetyTLim );
			}

			if( true == fErrorBorder )
			{
				m_EditSafetyTLim.ResetDrawBorder();
			}
		}
	}

	CDlgIndSelPMPanelHCSBase::ResetToolTipsErrorBorder( pWnd );
}

void CDlgIndSelPMPanelSolarBase::InitToolTips( CWnd* pWnd )
{
	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		CString TTstr;

		if( NULL == pWnd || pWnd == &m_EditSolarContent )
		{
			ResetToolTipsErrorBorder( &m_EditSolarContent );
			TTstr = TASApp.LoadLocalizedString( IDS_PM_TTSOLARCONTENT );
			m_ToolTip.AddToolWindow( &m_EditSolarContent, TTstr );
		}

	}

	CDlgIndSelPMPanelHCSBase::InitToolTips( pWnd );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelSolar dialog
CDlgIndSelPMPanelSolar::CDlgIndSelPMPanelSolar( CIndSelPMParams *pclIndSelParams, CWnd *pParent )
	: CDlgIndSelPMPanelSolarBase( pclIndSelParams, PanelType::ePanelSolar, CDlgIndSelPMPanelSolar::IDD, pParent )
{
}

bool CDlgIndSelPMPanelSolar::ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser || false == CDlgIndSelPMPanelSolarBase::ApplyPMInputUser( pclPMInputUser, bAllowShowErrorMessage ) )
	{
		return false;
	}

	return VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

bool CDlgIndSelPMPanelSolar::VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser )
	{
		return false;
	}

	m_vecErrorList.clear();
	m_mapErrorMsg.clear();

	return CDlgIndSelPMPanelSolarBase::VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelSolarSWKI dialog
CDlgIndSelPMPanelSolarSWKI::CDlgIndSelPMPanelSolarSWKI( CIndSelPMParams *pclIndSelParams, CWnd *pParent )
	: CDlgIndSelPMPanelSolarBase( pclIndSelParams, PanelType::ePanelSolarSWKI, CDlgIndSelPMPanelSolarSWKI::IDD, pParent )
{
	m_dStorageTankVolumeSaved = 0.0;
}

bool CDlgIndSelPMPanelSolarSWKI::ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser || false == CDlgIndSelPMPanelSolarBase::ApplyPMInputUser( pclPMInputUser, bAllowShowErrorMessage ) )
	{
		return false;
	}

	CString strToolTip;
	FormatString( strToolTip, IDS_DLGINDSELPMPANELS_SOLARCONTENTEXPFACTORTT, WriteDouble( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSolarCollectorMultiplierFactor(), 2, 1 ) );
	m_ToolTip.AddToolWindow( &m_BtnCheckSolarContentMultiplierFactor, strToolTip );

	FormatString( strToolTip, IDS_DLGINDSELPMPANELS_STORAGETANKMAXTEMPTT, WriteCUDouble( _U_TEMPERATURE, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetStorageTankMaxTemp(), true ) );
	m_ToolTip.AddToolWindow( &m_BtnStorageTankMaxTemp, strToolTip );

	m_EditStorageTankVolume.SetCurrentValSI( pclPMInputUser->GetStorageTankVolume() );
	m_EditStorageTankVolume.Update();

	return VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

bool CDlgIndSelPMPanelSolarSWKI::VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser )
	{
		return false;
	}

	m_vecErrorList.clear();
	m_mapErrorMsg.clear();

	// Check installed power.
	IsInstalledPowerOK( &m_EditInstalledPower, true );

	return CDlgIndSelPMPanelSolarBase::VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

void CDlgIndSelPMPanelSolarSWKI::OnNewDocument()
{
	CDlgIndSelPMPanelSolarBase::OnNewDocument();

	m_BtnCheckSolarContentMultiplierFactor.SetIcon( (HICON)LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDI_SOLARCONTENTEXPCOEFF ), IMAGE_ICON, 18, 18, LR_DEFAULTCOLOR ) );

	m_EditStorageTankVolume.SetReadOnly( FALSE );
	m_EditStorageTankVolume.EnableWindow( TRUE );

	m_BtnStorageTankMaxTemp.SetIcon( (HICON)LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDI_STORAGETANKMAXTEMP ), IMAGE_ICON, 18, 18, LR_DEFAULTCOLOR ) );

	// To avoid any 'OnEnChange' event to be called.
	m_bOnEnChangeEnabled = false;

	// Don't call 'OnUnitChange' because in this method we call a 'FillPMInputUser' and 'ApplyPMInputUser' and this will change status of some
	// control that we don't need when 'OnNewDocument' is called.
	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	GetDlgItem( IDC_STATIC_STORAGETANKVOLUMEUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_EditStorageTankVolume, _U_VOLUME );

	m_bOnEnChangeEnabled = true;
}

void CDlgIndSelPMPanelSolarSWKI::EnablePMSelection( bool Enable )
{
	CDlgIndSelPMPanelSolarBase::EnablePMSelection( Enable );
	
	m_EditStorageTankVolume.ActiveSpecialValidation( Enable, this );
	
	Invalidate();
	UpdateWindow();
}

BEGIN_MESSAGE_MAP( CDlgIndSelPMPanelSolarSWKI, CDlgIndSelPMPanelSolarBase )
	ON_BN_CLICKED( IDC_BUTTON_SOLARCONTENTMULTIPLIERFACTOR, OnBnClickedSolarContentMultiplierFactor )
	ON_BN_CLICKED( IDC_BUTTON_STORAGETANKMAXTEMP, OnBnClickedStorageTankMaxTemp )
	ON_EN_SETFOCUS( IDC_EDIT_STORAGETANKVOLUME, OnEnSetFocusStorageTankVolume )
	ON_EN_KILLFOCUS( IDC_EDIT_STORAGETANKVOLUME, OnKillFocusStorageTankVolume )
END_MESSAGE_MAP()

void CDlgIndSelPMPanelSolarSWKI::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelPMPanelSolarBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_BUTTON_SOLARCONTENTMULTIPLIERFACTOR, m_BtnCheckSolarContentMultiplierFactor );
	DDX_Control( pDX, IDC_BUTTON_STORAGETANKMAXTEMP, m_BtnStorageTankMaxTemp );
	DDX_Control( pDX, IDC_STATIC_STORAGETANKVOLUME, m_StaticStorageTankVolume );
	DDX_Control( pDX, IDC_EDIT_STORAGETANKVOLUME, m_EditStorageTankVolume );
}

BOOL CDlgIndSelPMPanelSolarSWKI::OnInitDialog()
{
	CDlgIndSelPMPanelSolarBase::OnInitDialog();

	// Disable the 'OnNewDocument' handler to not interfere with this dialog.
	m_EditStorageTankVolume.SetOnNewDocumentHandler( false );

	// Change 'Water content' by 'System volume' for SWKI HE301-01.
	m_StaticSystemVolume.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PM_SYSTEMVOLUME ) );

	m_StaticStorageTankVolume.SetTextAndToolTip( TASApp.LoadLocalizedString( IDS_PMHEATINGSWKI_STORAGETANKVOLUME ) );
	m_EditStorageTankVolume.ActiveSpecialValidation( true, this );

	return TRUE;
}

LRESULT CDlgIndSelPMPanelSolarSWKI::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList )
	{
		// Not yet ready!
		return 0;
	}

	CDlgIndSelPMPanelSolarBase::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	GetDlgItem( IDC_STATIC_STORAGETANKVOLUMEUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_EditStorageTankVolume, _U_VOLUME );

	return 0;
}

void CDlgIndSelPMPanelSolarSWKI::OnBnClickedSolarContentMultiplierFactor()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		return;
	}

	CDlgSolarContentMultiplierFactor dlg( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser(), this );
	dlg.DoModal();

	m_ToolTip.DelTool( &m_BtnCheckSolarContentMultiplierFactor );
	CString strToolTip = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_SOLARCONTENTEXPFACTORTT );
	FormatString( strToolTip, IDS_DLGINDSELPMPANELS_SOLARCONTENTEXPFACTORTT, WriteDouble( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSolarCollectorMultiplierFactor(), 2, 1 ) );
	m_ToolTip.AddToolWindow( &m_BtnCheckSolarContentMultiplierFactor, strToolTip );
}

void CDlgIndSelPMPanelSolarSWKI::OnBnClickedStorageTankMaxTemp()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		return;
	}

	CDlgStorageTankMaxTemperature dlg( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser(), this );
	dlg.DoModal();

	m_ToolTip.DelTool( &m_BtnStorageTankMaxTemp );
	CString strToolTip;
	FormatString( strToolTip, IDS_DLGINDSELPMPANELS_STORAGETANKMAXTEMPTT, WriteCUDouble( _U_TEMPERATURE, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetStorageTankMaxTemp(), true ) );
	m_ToolTip.AddToolWindow( &m_BtnStorageTankMaxTemp, strToolTip );
}

void CDlgIndSelPMPanelSolarSWKI::OnEnSetFocusStorageTankVolume()
{
	m_EditStorageTankVolume.SetSel( 0, -1 );
	m_dStorageTankVolumeSaved = m_EditStorageTankVolume.GetCurrentValSI();
}

void CDlgIndSelPMPanelSolarSWKI::OnKillFocusStorageTankVolume()
{
	// Do nothing if no change.
	if( m_dStorageTankVolumeSaved == m_EditStorageTankVolume.GetCurrentValSI() )
	{
		return;
	}

	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetStorageTankVolume( m_EditStorageTankVolume.GetCurrentValSI() );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMNo dialog
CDlgIndSelPMNo::CDlgIndSelPMNo( CIndSelPMParams *pclIndSelParams, CWnd *pParent )
	: CDlgIndSelPMPanelBase( pclIndSelParams, PanelType::ePanelNoPressMaint, CDlgIndSelPMNo::IDD, pParent )
{
	m_dDegassingPressureConnectPointSaved = 0.0;
	m_dSystemVolSaved = 0.0;
	m_dWaterHardnessSaved = 0.0;
	m_dWaterMakeUpTempSaved = 0.0;
}

bool CDlgIndSelPMNo::ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser || false == CDlgIndSelPMPanelBase::ApplyPMInputUser( pclPMInputUser, bAllowShowErrorMessage ) )
	{
		return false;
	}

	m_EditDegassingPressure.SetCurrentValSI( pclPMInputUser->GetDegassingPressureConnectPoint() );
	m_EditDegassingPressure.Update();

	m_EditSystemVol.SetCurrentValSI( pclPMInputUser->GetSystemVolume() );
	m_EditSystemVol.Update();

	m_EditWaterHardness.SetCurrentValSI( pclPMInputUser->GetWaterMakeUpWaterHardness() );
	m_EditWaterHardness.Update();

	// HYS-1054: WM temperature is only available for PM type None
	m_EditWaterMakeUpWaterTemp.SetCurrentValSI( pclPMInputUser->GetWaterMakeUpWaterTemp() );
	m_EditWaterMakeUpWaterTemp.Update();

	return VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

bool CDlgIndSelPMNo::VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage )
{
	if( NULL == pclPMInputUser )
	{
		return false;
	}

	m_vecErrorList.clear();
	m_mapErrorMsg.clear();
	pclPMInputUser->CheckInputData( &m_vecErrorList );

	ResetToolTipsErrorBorder( &m_EditSystemVol );
	m_EditSystemVol.ResetDrawBorder();

	CString msg;

	for( std::vector<std::pair<int, int>>::iterator iterError = m_vecErrorList.begin(); iterError != m_vecErrorList.end();
		 iterError++ )
	{
		if( CPMInputUser::ePM_InputEmpty == iterError->first && CPMInputUser::RACIE_WaterContent == iterError->second )
		{
			msg = _T("\r\n - ") + TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSYSTEMVOLUME );
			m_mapErrorMsg[MEID_WaterContent] = msg;

			ResetToolTipsErrorBorder( &m_EditSystemVol );
			msg = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTERRORSYSTEMVOLUME );
			m_ToolTip.AddToolWindow( &m_EditSystemVol, msg );
			m_EditSystemVol.SetDrawBorder( true, _RED );
			continue;
		}
	}

	return CDlgIndSelPMPanelBase::VerifyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
}

void CDlgIndSelPMNo::OnPMTypeChanged( CPMInputUser *pclPMInputUser )
{
	CDlgIndSelPMPanelBase::OnPMTypeChanged( pclPMInputUser );

	m_EditDegassingPressure.SetCurrentValSI( pclPMInputUser->GetDegassingPressureConnectPoint() );
	m_EditDegassingPressure.Update();

	m_EditSystemVol.SetCurrentValSI( pclPMInputUser->GetSystemVolume() );
	m_EditSystemVol.Update();

	m_EditWaterHardness.SetCurrentValSI( pclPMInputUser->GetWaterMakeUpWaterHardness() );
	m_EditWaterHardness.Update();
}

void CDlgIndSelPMNo::OnNewDocument()
{
	CDlgIndSelPMPanelBase::OnNewDocument();

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// To avoid any 'OnEnChange' event to be called.
	m_bOnEnChangeEnabled = false;

	GetDlgItem( IDC_STATICDEGPRESSUNIT )->SetWindowText( pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	InitNumericalEdit( &m_EditDegassingPressure, _U_PRESSURE );

	GetDlgItem( IDC_STATICSYSTEMVOLUNIT )->SetWindowText( pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_EditSystemVol, _U_VOLUME );

	GetDlgItem( IDC_STATICWATERHARDUNIT )->SetWindowText( pUnitDB->GetNameOfDefaultUnit( _U_WATERHARDNESS ).c_str() );
	InitNumericalEdit( &m_EditWaterHardness, _U_WATERHARDNESS );

	// HYS-1054: WM temperature is only available for PM type None
	GetDlgItem( IDC_STATICWMTEMPUNIT )->SetWindowText( pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditWaterMakeUpWaterTemp, _U_TEMPERATURE );

	m_bOnEnChangeEnabled = false;
}

void CDlgIndSelPMNo::EnableDegassing( bool bEnable, bool bWaterMakupChecked )
{
	CDlgIndSelPMPanelBase::EnableDegassing( bEnable, bWaterMakupChecked );

	if( true == bEnable )
	{
		// Static pressure in the system at the point where the vacuum degasser will be connected.
		CString strTT = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_TTDEGPRESSCONNECTPOINT );
		m_ToolTipOther.AddToolWindow( GetDlgItem( IDC_STATICDEGPRESSCONNECTPOINT ), strTT );
	}
	else
	{
		m_ToolTipOther.DelTool( GetDlgItem( IDC_STATICDEGPRESSCONNECTPOINT ) );
	}
}

void CDlgIndSelPMNo::EnablePMSelection( bool Enable )
{
	CDlgIndSelPMPanelBase::EnablePMSelection( Enable );
	m_EditDegassingPressure.ActiveSpecialValidation( Enable, this );
	m_EditSystemVol.ActiveSpecialValidation( Enable, this );
	m_EditWaterHardness.ActiveSpecialValidation( Enable, this );
	m_EditWaterMakeUpWaterTemp.ActiveSpecialValidation( Enable, this );
	Invalidate();
	UpdateWindow();
}

BEGIN_MESSAGE_MAP( CDlgIndSelPMNo, CDlgIndSelPMPanelBase )
	ON_EN_CHANGE( IDC_DEGEDITPRESS, OnEnChangeEdit )
	ON_EN_CHANGE( IDC_EDITSYSTEMVOL, OnEnChangeEdit )
	ON_EN_CHANGE( IDC_EDITWATERHARDNESS, OnEnChangeEdit )
	ON_EN_CHANGE( IDC_WMEDITTEMP, OnEnChangeEdit )

	ON_EN_SETFOCUS( IDC_DEGEDITPRESS, OnEnSetFocusDegPress )
	ON_EN_SETFOCUS( IDC_EDITSYSTEMVOL, OnEnSetFocusSystemVolume )
	ON_EN_SETFOCUS( IDC_EDITWATERHARDNESS, OnEnSetFocusWaterHardness )
	ON_EN_SETFOCUS( IDC_WMEDITTEMP, OnEnSetFocusWaterMakeUpWaterTemp )

	ON_EN_KILLFOCUS(IDC_DEGEDITPRESS, OnKillFocusEditDegPress )
	ON_EN_KILLFOCUS(IDC_EDITSYSTEMVOL, OnKillFocusEditSystemVolume )
	ON_EN_KILLFOCUS(IDC_EDITWATERHARDNESS, OnKillFocusEditWaterHardness )
	ON_EN_KILLFOCUS( IDC_WMEDITTEMP, OnKillFocusWaterMakeUpWaterTemp )
END_MESSAGE_MAP()

void CDlgIndSelPMNo::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelPMPanelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_DEGEDITPRESS, m_EditDegassingPressure );
	DDX_Control( pDX, IDC_EDITSYSTEMVOL, m_EditSystemVol );
	DDX_Control( pDX, IDC_EDITWATERHARDNESS, m_EditWaterHardness );
	DDX_Control( pDX, IDC_WMEDITTEMP, m_EditWaterMakeUpWaterTemp );
}

BOOL CDlgIndSelPMNo::OnInitDialog()
{
	CDlgIndSelPMPanelBase::OnInitDialog();

	GetDlgItem( IDC_STATICDEGPRESSCONNECTPOINT )->SetWindowText( TASApp.LoadLocalizedString( IDS_PM_DEGPRESSURECONNECT ) );
	GetDlgItem( IDC_STATICSYSTEMVOL )->SetWindowText( TASApp.LoadLocalizedString( IDS_PM_WATERCONTENT ) );
	GetDlgItem( IDC_STATICWATERHARDNESS )->SetWindowText( TASApp.LoadLocalizedString( IDS_PM_WATERHARDNESS ) );
	// HYS-1054: WM temperature is only available for PM type None
	GetDlgItem( IDC_STATICWMTEMP )->SetWindowText( TASApp.LoadLocalizedString( IDS_PM_WMWATERTEMP ) );

	// Disable the 'OnNewDocument' handler to not interfere with this dialog.
	m_EditDegassingPressure.SetOnNewDocumentHandler( false );
	m_EditSystemVol.SetOnNewDocumentHandler( false );
	m_EditWaterHardness.SetOnNewDocumentHandler( false );

	m_EditDegassingPressure.ActiveSpecialValidation( true, this );
	m_EditSystemVol.ActiveSpecialValidation( true, this );
	m_EditWaterHardness.ActiveSpecialValidation( true, this );
	// HYS-1054: WM temperature is only available for PM type None
	m_EditWaterMakeUpWaterTemp.ActiveSpecialValidation( true, this );

	m_ToolTipOther.Create( this, TTS_NOPREFIX );

	return TRUE;
}

LRESULT CDlgIndSelPMNo::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList )
	{
		// Not yet ready!
		return 0;
	}

	CDlgIndSelPMPanelBase::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	GetDlgItem( IDC_STATICDEGPRESSUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	InitNumericalEdit( &m_EditDegassingPressure, _U_PRESSURE );

	GetDlgItem( IDC_STATICSYSTEMVOLUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	InitNumericalEdit( &m_EditSystemVol, _U_VOLUME );

	GetDlgItem( IDC_STATICWATERHARDUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_WATERHARDNESS ).c_str() );
	InitNumericalEdit( &m_EditWaterHardness, _U_WATERHARDNESS );

	// HYS-1054: WM temperature is only available for PM type None
	GetDlgItem( IDC_STATICWMTEMPUNIT )->SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	InitNumericalEdit( &m_EditWaterMakeUpWaterTemp, _U_TEMPERATURE );

	return 0;
}

void CDlgIndSelPMNo::OnEnSetFocusSystemVolume()
{
	m_dSystemVolSaved = m_EditSystemVol.GetCurrentValSI();
	m_EditSystemVol.SetSel( 0, -1 );
}

void CDlgIndSelPMNo::OnEnSetFocusDegPress()
{
	m_dDegassingPressureConnectPointSaved = m_EditDegassingPressure.GetCurrentValSI();
	m_EditDegassingPressure.SetSel( 0, -1 );
}

void CDlgIndSelPMNo::OnEnSetFocusWaterHardness()
{
	m_dWaterHardnessSaved = m_EditWaterHardness.GetCurrentValSI();
	m_EditWaterHardness.SetSel( 0, -1 );
}

void CDlgIndSelPMNo::OnKillFocusEditSystemVolume()
{
	// Do nothing if no changed.
	if (m_dSystemVolSaved == m_EditSystemVol.GetCurrentValSI())
	{
		return;
	}

	ResetToolTipsErrorBorder(&m_EditSystemVol);
	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetSystemVolume( m_EditSystemVol.GetCurrentValSI() );
}

void CDlgIndSelPMNo::OnKillFocusEditDegPress()
{
	// Do nothing if no changed.
	if (m_dDegassingPressureConnectPointSaved == m_EditDegassingPressure.GetCurrentValSI())
	{
		return;
	}

	ResetToolTipsErrorBorder(&m_EditDegassingPressure);
	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetDegassingPressureConnectPoint( m_EditDegassingPressure.GetCurrentValSI() );
}

void CDlgIndSelPMNo::OnKillFocusEditWaterHardness()
{
	// Do nothing if no changed.
	if (m_dWaterHardnessSaved == m_EditWaterHardness.GetCurrentValSI())
	{
		return;
	}

	ResetToolTipsErrorBorder(&m_EditWaterHardness);
	m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpWaterHardness( m_EditWaterHardness.GetCurrentValSI() );
}
