#include "stdafx.h"
#include "TASelect.h"
#include "DlgSelectActuator.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC( CDlgSelectActuator, CDialogEx )

CDlgSelectActuator::CDlgSelectActuator( CTADatabase::FilterSelection eFilterSelection, bool bStartForCombo, CWnd *pParent )
	: CDialogEx( CDlgSelectActuator::IDD, pParent )
{
	m_eFilterSelection = eFilterSelection;
	m_DlgPos = CRect( 0, 0, 0, 0 );
	m_pHM = NULL;
	m_bStartForCombo = bStartForCombo;
	m_lRow = 0;
	m_lCol = 0;
	m_pMainSpreadSheet = dynamic_cast<CSSheet *>( pParent );
	m_bOpen = false;
	m_pTechParams = NULL;
	m_nTimer = (UINT_PTR)0;
	m_bDisableActivateMessage = false;
}

void CDlgSelectActuator::OpenDlgSelectActr( CDS_HydroMod *pHM, long lCol, long lRow )
{
	m_pHM = pHM;	 
	m_lCol = lCol;
	m_lRow = lRow;

	if( true == m_bOpen )
	{
		PostMessage( WM_USER_CLOSESPREADCOMBOBOX );
		return;
	}

	m_bOpen = true;

	if( NULL != m_pMainSpreadSheet )
	{
		CRect rect = m_pMainSpreadSheet->GetCellCoordInPixel( m_lCol, m_lRow );
		m_pMainSpreadSheet->ClientToScreen( &rect );
		m_DlgPos = rect;
	}

	m_pTechParams = m_pHM->GetpPrjParam()->GetpHmCalcParams();

	if( 0 == GetSafeHwnd() )
	{
		CDialogEx::Create( IDD );
	}
	else
	{
		_Refresh();
	}

	ShowWindow( SW_SHOW );
}

int CDlgSelectActuator::InitCompliantActuatorList()
{
	if( NULL == m_pHM )
	{
		return 0;
	}

	if( NULL == m_pHM->GetpCV() )
	{
		return 0;
	}

	CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( m_pHM->GetpCV()->GetCvIDPtr().MP );

	if( NULL == pCV )
	{
		return 0;
	}

	// Remark: 'InitCompliantActuatorList' is called only when we are not working with set.
	m_pHM->GetpCV()->GetCOCCompliantActuatorList( &m_ActrFullList, false );

	return m_ActrFullList.size();
}

void CDlgSelectActuator::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECKPACKAGE, m_CheckPackage );
	DDX_Control( pDX, IDC_COMBOPOWERSUPPLY, m_CBPowerSupply );
	DDX_Control( pDX, IDC_COMBOINPUTSIGNAL, m_CBInputSignal );
	DDX_Control( pDX, IDC_CHECKFAILSAFE, m_CheckboxFailSafe );
	DDX_Control( pDX, IDC_COMBODRP, m_CBDefaultReturnPosition );
	DDX_Control( pDX, IDC_COMBOACTUATOR, m_CBActuator );
	DDX_Control( pDX, IDC_EDITSELECTEDACTUATOR, m_EditSelectedActuator );
	DDX_Control( pDX, IDC_GROUPBOX, m_GroupBox );
}

BEGIN_MESSAGE_MAP( CDlgSelectActuator, CDialogEx )
	ON_COMMAND( IDC_GROUPBOX, OnBnClickedGroupBox )
	ON_BN_CLICKED( IDC_CHECKPACKAGE, OnClickedCheckPackage )
	ON_CBN_SELCHANGE( IDC_COMBOPOWERSUPPLY, OnComboSelChangePowerSupply )
	ON_CBN_SELCHANGE( IDC_COMBOINPUTSIGNAL, OnComboSelChangeInputSignal )
	ON_BN_CLICKED( IDC_CHECKFAILSAFE, OnBnClickedCheckFailSafe )
	ON_CBN_SELCHANGE( IDC_COMBODRP, OnComboSelChangeDRP )
	ON_CBN_SELCHANGE( IDC_COMBOACTUATOR, OnComboSelChangeActuator )
	ON_BN_CLICKED( IDOK, OnBnClickedOk )
	ON_WM_ACTIVATE()
	ON_WM_TIMER()
	ON_MESSAGE( WM_USER_CLOSESPREADCOMBOBOX, OnCloseCB )
END_MESSAGE_MAP()

BOOL CDlgSelectActuator::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_GroupBox.SetCheckBoxStyle( BS_AUTOCHECKBOX );

	// String initialization
	CString str = TASApp.LoadLocalizedString( IDS_DLGSELECTACTUATOR_CAPTION );
	this->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGSELECTACTUATOR_CHECKPACKAGE );
	GetDlgItem( IDC_CHECKPACKAGE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGSELECTACTUATOR_STATICPOWERSUPPLY );
	GetDlgItem( IDC_STATICPOWERSUPPLY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGSELECTACTUATOR_STATICINPUTSIGNAL );
	GetDlgItem( IDC_STATICINPUTSIGNAL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGSELECTACTUATOR_STATICFAILSAFE );
	GetDlgItem( IDC_CHECKFAILSAFE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGSSEL_DEFAULTRETURNPOS );
	GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_ACTUATOR );
	GetDlgItem( IDC_STATICACTUATOR )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGSELECTACTUATOR_GROUPBOX );
	GetDlgItem( IDC_GROUPBOX )->SetWindowText( str );

	m_ToolTip.Create( this, TTS_NOPREFIX );
	m_ToolTip.SetDelayTime( TTDT_AUTOPOP, 5000 );

	if( eBool3::eb3True == m_pHM->GetpCV()->GetActrSelectionAllowed() )
	{
		m_GroupBox.SetCheck( TRUE );
	}
	else
	{
		m_GroupBox.SetCheck( FALSE );
		OnBnClickedGroupBox();
	}

	if( eBool3::eb3True == m_pHM->GetpCV()->GetActrSelectedAsaPackage() )
	{
		m_CheckPackage.SetCheck( BST_CHECKED );
	}
	else
	{
		m_CheckPackage.SetCheck( BST_UNCHECKED );
	}
	
	m_CheckBoxStatus = -1;
	_Refresh();

	return TRUE;
}

LRESULT CDlgSelectActuator::OnCloseCB( WPARAM wParam, LPARAM lParam )
{
	EndDialog( TRUE );
	m_bOpen = false;

	if( NULL != m_pMainSpreadSheet )
	{
		m_pMainSpreadSheet->SetActiveCell( m_lCol,m_lRow );
		m_pMainSpreadSheet->CloseDialogSCB( this, true );
	}

	DestroyWindow();
	return true;
}

void CDlgSelectActuator::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	if( NULL != m_hWnd && true == m_bStartForCombo )
	{
		//First get the current Window Styles
		LONG lStyle = GetWindowLong( m_hWnd, GWL_STYLE );

		lStyle &= ~WS_SYSMENU;       //SYSMENU to remove a close button
		lStyle &= ~WS_BORDER;        

		// Now set the modified window style.
		SetWindowLong( m_hWnd, GWL_STYLE, lStyle );
	}

	CDialogEx::PreSubclassWindow();
}

BOOL CDlgSelectActuator::PreTranslateMessage( MSG *pMsg )
{
	if( WM_MOUSEMOVE == pMsg->message )
	{ 
		// Pass a mouse message to a tool tip control for processing 
		if( NULL !=  m_ToolTip.GetSafeHwnd() )
		{
			m_ToolTip.RelayEvent( pMsg );
		}
	}

	return CDialogEx::PreTranslateMessage( pMsg );
}

void CDlgSelectActuator::OnBnClickedGroupBox()
{
	if( TRUE == m_GroupBox.GetCheck() )
	{
		if( NULL != m_pHM->GetpCV() )
		{
			m_pHM->GetpCV()->SetActrSelectionAllowed(eBool3::eb3True);
			m_pHM->GetpCV()->SelectActuator();
		}
	}

	_Refresh();
}

void CDlgSelectActuator::OnClickedCheckPackage()
{
	if( NULL == m_pHM || NULL == m_pHM->GetpCV() || NULL == dynamic_cast<CDB_ControlValve *>( m_pHM->GetpCV()->GetCvIDPtr().MP ) )
	{
		return;
	}

	if( BST_UNCHECKED == m_CheckPackage.GetCheck() )	
	{
		// Verify first if we can break the set.
		CDB_ControlValve *pclControlValve = (CDB_ControlValve *)( m_pHM->GetpCV()->GetCvIDPtr().MP );

		// Remark: if the set is not breakable, the '_FillCheckBoxPackage' method will disable the checkbox with a tooltip to explain.

		if( ePartOfaSetYesBreakOnMsg == pclControlValve->GetPartOfaSet() )
		{
			// Breakable with a message.
			CString str = TASApp.LoadLocalizedString( AFXMSG_PRODUCTSETBREAK );
			
			// When the dialog box message is displayed, the 'OnActivate' method of this dialog is called with the 'WA_INACTIVE' state.
			// And we don't want in this case that this method close the dialog.
			m_bDisableActivateMessage = true;

			if( IDNO == MessageBox( str, NULL, MB_YESNO | MB_ICONQUESTION ) )
			{
				m_CheckPackage.SetCheck( BST_CHECKED );
				m_bDisableActivateMessage = false;
				return;
			}

			m_bDisableActivateMessage = false;
		}

		// Specify now that the control valve is chosen without a set.
		m_pHM->GetpCV()->SetCVSelectedAsaPackage( eb3False );

		_Refresh();
	}
	else
	{
		// Remark: if no set is available, the '_FillCheckBoxPackage' method will disable the checkbox with a tooltip to explain.

		// Specify now that the control valve is chosen with a set.
		m_pHM->GetpCV()->SetCVSelectedAsaPackage( eb3True );
	}

	_FillCBPowerSupply();
}

void CDlgSelectActuator::OnComboSelChangePowerSupply()
{
	_FillCBInputSignal();
}

void CDlgSelectActuator::OnComboSelChangeInputSignal()
{
	_SetCheckFailSafeFct();
}

void CDlgSelectActuator::OnBnClickedCheckFailSafe()
{
	m_CheckBoxStatus = m_CheckboxFailSafe.GetCheck();
	if( BST_CHECKED == m_CheckboxFailSafe.GetCheck() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGSSEL_FAILSAFEPOSITION );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
	else
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGSSEL_DEFAULTRETURNPOS );
		GetDlgItem( IDC_STATICDRP )->SetWindowText( str );
	}
	_FillCBDefaultReturnPos();
}

void CDlgSelectActuator::OnComboSelChangeDRP()
{
	_FillCBActuator();
}

void CDlgSelectActuator::OnComboSelChangeActuator()
{
	m_EditSelectedActuator.Clear();

	if( NULL == m_pHM )
	{
		return;
	}

	if( NULL == m_pHM->GetpCV() )
	{
		return;
	}

	CDB_ControlValve *pCVDB = dynamic_cast<CDB_ControlValve *>( m_pHM->GetpCV()->GetCvIDPtr().MP );

	if( NULL == pCVDB )
	{
		return;
	}

	if( m_CBActuator.GetCurSel() < 0 )
	{
		return;
	}

	CDB_ElectroActuator *pActr = dynamic_cast<CDB_ElectroActuator *>( (CData *)m_CBActuator.GetItemData( m_CBActuator.GetCurSel() ) );
	// HYS-941: Thermostatic actuators management added
	if( NULL == pActr )
	{
		CDB_ThermostaticActuator *pActr = dynamic_cast<CDB_ThermostaticActuator *>( (CData *)m_CBActuator.GetItemData( m_CBActuator.GetCurSel() ) );
		if ( NULL == pActr )
		{
			return;
		}

		CString str = CString( pActr->GetName() ) + _T("\r\n");
		str += CString( _T("\r\n") );

		CString strDescription = pActr->GetComment();

		if( false == strDescription.IsEmpty() )
		{
			str += strDescription + _T("\r\n");
		}
	
		// HYS-951: Min setting can be 0.
		if( pActr->GetMinSetting() >= 0 && pActr->GetMaxSetting() > 0 )
		{
			CString str2;
			CString str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_SETTINGRANGE );
			CString strMin = WriteCUDouble( _U_TEMPERATURE, pActr->GetMinSetting(), false );
			CString strMax = WriteCUDouble( _U_TEMPERATURE, pActr->GetMaxSetting(), true );
			str2.Format( _T("%s: [%s-%s]"), str1, strMin, strMax );
			str += str2;
			str += CString( _T("\r\n") );
		}

		if( pActr->GetCapillaryLength() > 0 )
		{
			CString str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_CAPILLENGTH );
			str1 += CString( _T(": ") ) + WriteCUDouble( _U_LENGTH, pActr->GetCapillaryLength(), true );
			str += str1;
			str += CString( _T("\r\n") );
		}

		if( pActr->GetHysteresis() > 0 )
		{
			CString str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_HYSTERESIS );
			str1 += CString( _T(": ") ) + WriteCUDouble( _U_DIFFTEMP, pActr->GetHysteresis(), true );
			str += str1;
			str += CString( _T("\r\n") );
		}

		if( pActr->GetFrostProtection() > 0 )
		{
			CString str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_FROSTPROTECT );
			str1 += CString( _T(": ") ) + WriteCUDouble( _U_TEMPERATURE, pActr->GetFrostProtection(), true );
			str += str1;
		}
		m_EditSelectedActuator.SetWindowText( str );
	}
	else
	{

		CString str = CString( pActr->GetName() ) + _T("\r\n");
		str += CString( _T("\r\n") );

		CString strDescription = pActr->GetComment();

		if( false == strDescription.IsEmpty() )
		{
			str += strDescription + _T("\r\n");
		}

		CString str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_IPXXAUTO );
		str1 += CString( _T(" : ") ) + pActr->GetIPxxAuto();

		if( false == pActr->GetIPxxManual().IsEmpty() )
		{
			str1 += _T("(" ) + pActr->GetIPxxManual() + _T( ")");
		}

		str += str1;
		str += CString( _T("\r\n") );

		str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_POWSUP );
		str1 += CString( _T(" : ") ) + pActr->GetPowerSupplyStr();
		str += str1;
		str += CString( _T("\r\n") );

		str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_INPUTSIG );
		str1 += CString( _T(" : ") ) + pActr->GetInOutSignalsStr( true );
		str += str1;
		str += CString( _T("\r\n") );

		str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_OUTPUTSIG );

		if( CString( _T( "" ) ) == pActr->GetInOutSignalsStr( false ) )
		{
			str1 += CString( _T(" :    -") );
		}
		else
		{
			str1 += CString( _T(" : ") ) + pActr->GetInOutSignalsStr( false );
		}

		str += str1;
		str += CString( _T("\r\n") );

		str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_RELAYTYPE );
		str1 += CString( _T(" : ") ) + pActr->GetRelayStr();
		str += str1;
		str += CString( _T("\r\n") );

		str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_FAILSAFE );

		if( pActr->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
		{
			str1 += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
		}
		else if( pActr->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
		{
			str1 += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
		}
		else
		{
			str1 += CString( _T( " : " ) ) + TASApp.LoadLocalizedString( IDS_NO );
		}

		str += str1;
		str += CString( _T("\r\n") );

		if( m_CheckboxFailSafe.GetCheck() == BST_CHECKED )
		{
			str1 = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_FSP ) + _T( " " );
		}
		else
		{
			str1 = TASApp.LoadLocalizedString( IDS_DLGSRACTUATOR_ACT_DRP ) + _T( " " );
		}

		if( CString( _T( "" ) ) == pActr->GetDefaultReturnPosStr( pActr->GetDefaultReturnPos() ).c_str() )
		{
			str1 += CString( _T(" -") );
		}
		else
		{
			str1 += pActr->GetDefaultReturnPosStr( pActr->GetDefaultReturnPos() ).c_str();
		}

		str += str1;
		str += CString( _T("\r\n") );

		str1 = _T( "" );
		CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( pCVDB->GetCloseOffCharIDPtr().MP );

		if( NULL != pCloseOffChar )
		{
			double dCloseOffValue = 0.0;

			if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
			{
				str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_CLOSEOFFDP );
				dCloseOffValue = pCloseOffChar->GetCloseOffDp( pActr->GetMaxForceTorque() );

				if( dCloseOffValue > 0.0 )
				{
					str1 += CString( _T(" : ") ) + WriteCUDouble( _U_DIFFPRESS, dCloseOffValue, true );
				}
			}
			else
			{
				str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACT_MAXINLETPRESSURE );
				dCloseOffValue = pCloseOffChar->GetMaxInletPressure( pActr->GetMaxForceTorque() );

				if( dCloseOffValue > 0.0 )
				{
					str1 += CString( _T(" : ") ) + WriteCUDouble( _U_PRESSURE, dCloseOffValue, true );
				}
			}
		}

		str += str1 + _T("\r\n");
		str1 = TASApp.LoadLocalizedString( IDS_DLGDIRSEL_ACTUATING_TIME ) + CString( _T(" : ") ) + pActr->GetActuatingTimesStr( pCVDB->GetStroke(), true );

		str += str1;
		m_EditSelectedActuator.SetWindowText( str );
	}
}

void CDlgSelectActuator::OnBnClickedOk()
{
	if( NULL == m_pHM )
	{
		return;
	}

	if( NULL == m_pHM->GetpCV() )
	{
		return;
	}

	if( BST_UNCHECKED == m_GroupBox.GetCheck() )
	{
		m_pHM->GetpCV()->SetActrSelectionAllowed( eBool3::eb3False );
		m_pHM->GetpCV()->RemoveActuator();
		CDialogEx::OnOK();

		return;
	}
	else
	{
		m_pHM->GetpCV()->SetActrSelectionAllowed( eBool3::eb3True );
	}

	CDB_ControlValve *pCVDB = dynamic_cast<CDB_ControlValve *>(m_pHM->GetpCV()->GetCvIDPtr().MP);

	if( NULL == pCVDB )
	{
		return;
	}

	if( m_CBActuator.GetCurSel() < 0 )
	{
		return;
	}

	// HYS-941: Thermostatic actuators management added
	CDB_Actuator *pActr = dynamic_cast<CDB_Actuator *>( (CData *)m_CBActuator.GetItemData( m_CBActuator.GetCurSel() ) );
	CDB_ElectroActuator *pElecActr = dynamic_cast<CDB_ElectroActuator *>( (CData *)m_CBActuator.GetItemData( m_CBActuator.GetCurSel() ) );

	if( ( NULL == pActr ) )
	{
		return;
	}

	if( m_CBPowerSupply.GetCurSel() < 0 || m_CBInputSignal.GetCurSel() < 0 )
	{
		return;
	}

	m_pHM->GetpCV()->SetActrIDPtr( pActr->GetIDPtr() );


	if( NULL != pElecActr )
	{
		IDPTR IdptrPowerSupply = ( (CData *)m_CBPowerSupply.GetItemData( m_CBPowerSupply.GetCurSel() ) )->GetIDPtr();
		m_pHM->GetpCV()->SetActrSelectedVoltageIDPtr( IdptrPowerSupply );

		IDPTR IdptrInputSignal = ( (CData *)m_CBInputSignal.GetItemData( m_CBInputSignal.GetCurSel() ) )->GetIDPtr();
		m_pHM->GetpCV()->SetActrSelectedSignalIDPtr( IdptrInputSignal );
	}
	
	// Adapter needed?
	CDB_Product *pAdapt = pCVDB->GetMatchingAdapter( pActr );

	if( NULL != pAdapt && true == pAdapt->IsAnAccessory() )
	{
		m_pHM->GetpCV()->SetActrAdaptIDPtr( pAdapt->GetIDPtr() );
	}
	else
	{
		m_pHM->GetpCV()->SetActrAdaptIDPtr( _NULL_IDPTR );
	}

	m_pHM->GetpCV()->SetActrSelectedAsaPackage( ( TRUE == m_CheckPackage.GetCheck() ) ? eb3True : eb3False );
	CDialogEx::OnOK();
}

void CDlgSelectActuator::OnActivate( UINT nState, CWnd *pWndOther, BOOL bMinimized )
{
	CDialogEx::OnActivate(nState, pWndOther, bMinimized);

	if( false == m_bDisableActivateMessage )
	{
		if( WA_ACTIVE == nState )
		{
			m_nTimer = SetTimer( _TIMERID_DLGSELECTACTUATOR, 1, NULL );
		}

		if( WA_INACTIVE == nState )
		{
			if( (UINT_PTR)0 == m_nTimer)
			{
				PostMessage( WM_USER_CLOSESPREADCOMBOBOX );
			}
		}
	}
}

void CDlgSelectActuator::OnTimer( UINT_PTR nIDEvent )
{
	CDialogEx::OnTimer( nIDEvent );

	if( (UINT_PTR)0 == m_nTimer || m_nTimer != nIDEvent )
	{
		return;
	}

	KillTimer( m_nTimer );
	m_nTimer = (UINT_PTR)0;

	if( true == m_bStartForCombo && NULL != m_pMainSpreadSheet )
	{
		CRect rectDlgMainSSheet;
		GetWindowRect(rectDlgMainSSheet);

		CRect WndRect;
		m_pMainSpreadSheet->GetDesktopWindow()->GetWindowRect(WndRect);

		CRect BtnRect;
		GetDlgItem(IDOK)->GetWindowRect(BtnRect);
		rectDlgMainSSheet.bottom = BtnRect.bottom + 9;

		if( m_DlgPos != CRect(0, 0, 0, 0 ) )
		{
			CPoint DlgPos;
			DlgPos.x = m_DlgPos.left;
			DlgPos.y = m_DlgPos.bottom;

			if( ( m_DlgPos.left + rectDlgMainSSheet.Width() ) > WndRect.right )
			{
				DlgPos.x = m_DlgPos.right-rectDlgMainSSheet.Width();
			}

			::SetWindowPos( m_hWnd, HWND_TOPMOST, DlgPos.x, DlgPos.y, rectDlgMainSSheet.Width(), rectDlgMainSSheet.Height(), SWP_NOCOPYBITS );
		}
	}
}

void CDlgSelectActuator::_Refresh()
{
	if( NULL == m_pHM || NULL == m_pHM->GetpCV() || NULL == dynamic_cast<CDB_ControlValve*>( m_pHM->GetpCV()->GetCvIDPtr().MP ) )
	{
		return;
	}

	if( eb3True != m_pHM->GetpCV()->GetCVSelectedAsaPackage() )
	{
		// Search compatible actuators only when we are not working with set.
		InitCompliantActuatorList();
	}

	_FillCheckBoxPackage();
	_FillCBPowerSupply();

	if( FALSE == m_GroupBox.GetCheck() )
	{
		m_CheckPackage.EnableWindow( FALSE );
		m_CBActuator.EnableWindow( FALSE );
		m_CBDefaultReturnPosition.EnableWindow( FALSE );
		m_CBInputSignal.EnableWindow( FALSE );
		m_CBPowerSupply.EnableWindow( FALSE );
		m_CheckboxFailSafe.EnableWindow( FALSE );
	}
}

void CDlgSelectActuator::_FillCheckBoxPackage()
{
	if( NULL == m_pHM || NULL == m_pHM->GetpCV() || NULL == dynamic_cast<CDB_ControlValve *>( m_pHM->GetpCV()->GetCvIDPtr().MP ) )
	{
		return;
	}

	CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( m_pHM->GetpCV()->GetCvIDPtr().MP );

	if( 0 != m_ToolTip.GetToolCount() )
	{
		m_ToolTip.DelTool( &m_CheckPackage );
	}

	m_CheckPackage.SetCheck( BST_UNCHECKED );
	m_CheckPackage.EnableWindow( FALSE );

	m_ActrPackageCompliantList.clear();

	// Here, HySelect has already selected the valve. We know thus what has been selected in regards to the 'Select valve and actuator set'
	// defined in the technical parameters.
	//   1) If user doesn't want set, the variable 'CDS_HydroMod::CCv:m_bCVSelectedAsaPackage' is set to 'eb3False'. We must then check also
	//      if valve belongs to a set. If 'yes' than we can enable the checkbox. If no, the checkbox is disable.
	//   2) If user wants valve and actuator set, the valve found can belong to a set. In this case 
	//      the 'CDS_HydroMod::CCv:m_bCVSelectedAsaPackage' variable is set to 'eb3True'. And user can break the set ONLY if the control valve
	//      can be sold separately.
	//   3) If user wants valve and actuator set, but no valve has been found the 'CDS_HydroMod::CCv:m_bCVSelectedAsaPackage' 
	//      variable is reset to 'false'.
	//
	//        | CV part of set | Check state | Check value | Remark                             | Combo power supply, input signal, fail-safe, drp and actuator.
	// -------+----------------+-------------+--------------------------------------------------+-----------------------------------------------------
	// Case 1 | No             | Disable     | Unchecked   | Add tooltip on checkbox to explain | Adapt in regards to the available actuators and enabled.
	//      1'| Yes            | Enable      | Unchecked   |                                    | Adapt in regards to the available actuators and enabled.
	// Case 2 | Yes            | Disable     | Checked     | Add tooltip on checkbox to explain | Adapt in regards to the unique actuator belonging to the set and disabled.
	//      2'| YesBreakable   | Enable      | Checked     |                                    | Adapt in regards to the unique actuator belonging to the set and disabled.
	// Case 3 | No             | Disable     | Unchecked   | Add tooltip on checkbox to explain | Adapt in regards to the available actuators and enabled.

//	if( false == m_pHM->GetpPrjParam()->GetpHmCalcParams()->GetActuatorSelectedByPackage() )  
//	{
		if( eb3False == m_pHM->GetpCV()->GetCVSelectedAsaPackage() )
		{
			if( false == pclControlValve->IsPartOfaSet() )
			{
				// Case 1.
				m_CheckPackage.SetCheck( BST_UNCHECKED );
				m_CheckPackage.EnableWindow( FALSE );

				// There is no set available containing an actuator and this product.
				CString str = TASApp.LoadLocalizedString( IDS_DLGSELECTACTUATOR_CANTBESOLDBYSET );
				m_ToolTip.AddToolWindow( &m_CheckPackage, str );
			}
			else
			{
				// Case 1'.
				m_CheckPackage.SetCheck( BST_UNCHECKED );
				m_CheckPackage.EnableWindow( TRUE );
			}
		}
// 	}
// 	else
// 	{
		if( eb3True == m_pHM->GetpCV()->GetCVSelectedAsaPackage() )
		{
			if( ePartOfaSetYes == pclControlValve->GetPartOfaSet() )
			{
				// Case 2.
				m_CheckPackage.SetCheck( BST_CHECKED );
				m_CheckPackage.EnableWindow( FALSE );

				// This product and actuator set can't be sold separately.
				CString str = TASApp.LoadLocalizedString( IDS_DLGSELECTACTUATOR_CANTBESOLDSEPARATELY );
				m_ToolTip.AddToolWindow( &m_CheckPackage, str );
			}
			else if( ePartOfaSetYesBreakOk == pclControlValve->GetPartOfaSet() || ePartOfaSetYesBreakOnMsg == pclControlValve->GetPartOfaSet() )
			{
				// Case 2'.
				m_CheckPackage.SetCheck( BST_CHECKED );
				m_CheckPackage.EnableWindow( TRUE );
			}
		}
// 		else
// 		{
// 			// Case 3.
// 			m_CheckPackage.SetCheck( BST_UNCHECKED );
// 			m_CheckPackage.EnableWindow( FALSE );
// 
// 			// There is no set available containing an actuator and this product.
// 			CString str = TASApp.LoadLocalizedString( IDS_DLGSELECTACTUATOR_CANTBESOLDBYSET );
// 			m_ToolTip.AddToolWindow( &m_CheckPackage, str );
// 		}
//	}
}

void CDlgSelectActuator::_FillCBPowerSupply()
{
	if( NULL == m_pHM || NULL == m_pHM->GetpCV() || NULL == dynamic_cast<CDB_ControlValve*>( m_pHM->GetpCV()->GetCvIDPtr().MP ) )
	{
		return;
	}

	if( eb3False == m_pHM->GetpCV()->GetCVSelectedAsaPackage() )
	{
		// Take the power supplies from all the actuators compatible with the current control valve.
		CRank rkPowerSupplyList, rkList;
	
		// Transfer selected list to a rank list.
		_MaptoRankList( &m_ActrFullList, &rkList );
	
		// Extract list of power supply.
		GetpTADB()->GetActuatorPowerSupplyList( &rkPowerSupplyList, &rkList, m_pHM->GetCvCtrlType(), m_eFilterSelection );
	
		// Transfer to the combo
		rkPowerSupplyList.Transfer( &m_CBPowerSupply );
	}
	else
	{
		// Take only the power supplies from the actuator belonging to the set.
		m_CBPowerSupply.ResetContent();

		if( NULL != dynamic_cast<CDB_ElectroActuator *>( m_pHM->GetpCV()->GetActrIDPtr().MP ) )
		{
			CDB_ElectroActuator *pclElectroActuator = (CDB_ElectroActuator *)( m_pHM->GetpCV()->GetActrIDPtr().MP );

			for( int iLoop = 0; iLoop < ( int )pclElectroActuator->GetNumOfPowerSupplyIDptr(); iLoop++ )
			{
				IDPTR PowerSupplyIDPtr = pclElectroActuator->GetPowerSupplyIDPtr( iLoop );
				CDB_StringID *pclActrPowerSupply = dynamic_cast<CDB_StringID *>( PowerSupplyIDPtr.MP );

				int iItem = m_CBPowerSupply.AddString( pclActrPowerSupply->GetString() );
				m_CBPowerSupply.SetItemData( iItem, (LPARAM)(void *)PowerSupplyIDPtr.MP );
			}
		}
	}
	
	if( m_CBPowerSupply.GetCount() > 1 )
	{
		m_CBPowerSupply.EnableWindow( TRUE );
			
		// Try to match with previous selection.
		m_CBPowerSupply.SetCurSel( 0 );
		int iPos = -1;

		if( NULL != m_pHM->GetpCV()->GetActrIDPtr().MP )
		{
			iPos = _FindCBLParam( &m_CBPowerSupply, (DWORD_PTR)m_pHM->GetpCV()->GetActrSelectedVoltageIDPtr().MP );
		}

		if( iPos < 0 )	// Return to Tech params
		{
			iPos = _FindCBLParam( &m_CBPowerSupply, (DWORD_PTR)m_pTechParams->GetPrjParamIDPtr( CPrjParams::ActuatorPowerSupplyID ).MP ); 
		}

		if( iPos >= 0 )
		{
			m_CBPowerSupply.SetCurSel( iPos );
		}
	}
	else
	{
		m_CBPowerSupply.SetCurSel( 0 );
		m_CBPowerSupply.EnableWindow( FALSE );
	}
			
	OnComboSelChangePowerSupply();
}

void CDlgSelectActuator::_FillCBInputSignal()
{
	if( NULL == m_pHM || NULL == m_pHM->GetpCV() || NULL == dynamic_cast<CDB_ControlValve*>( m_pHM->GetpCV()->GetCvIDPtr().MP ) )
	{
		return;
	}

	if( eb3False == m_pHM->GetpCV()->GetCVSelectedAsaPackage() )
	{
		// Take the input signals from all the actuators compatible with the current control valve.

		// Current selected power supply.
		CDB_StringID *pStrIDPowerSupply = NULL;

		if( m_CBPowerSupply.GetCurSel() > -1 )
		{
			pStrIDPowerSupply = dynamic_cast<CDB_StringID *>( (CData *)m_CBPowerSupply.GetItemData( m_CBPowerSupply.GetCurSel() ) );
		}
		else 
		{
			m_CBInputSignal.ResetContent();
			m_CBInputSignal.EnableWindow( FALSE );
			m_CheckboxFailSafe.SetCheck( BST_UNCHECKED );
			m_CheckboxFailSafe.EnableWindow( FALSE );
			m_CBDefaultReturnPosition.ResetContent();
			m_CBDefaultReturnPosition.EnableWindow( FALSE );
			m_CBActuator.ResetContent();
			m_CBActuator.EnableWindow( FALSE );
			GetDlgItem( IDOK )->EnableWindow( FALSE );
			return;
		}

		CRank rkInputSignalList, rkList;
		_MaptoRankList( &m_ActrFullList, &rkList );
	
		// Extract list of input signal.
		GetpTADB()->GetActuatorInputSignalList( &rkInputSignalList, &rkList, m_pHM->GetCvCtrlType(), pStrIDPowerSupply->GetIDPtr().ID, m_eFilterSelection );

		rkInputSignalList.Transfer( &m_CBInputSignal );
	}
	else
	{
		// Take only the input signals from the actuator belonging to the set.
		m_CBInputSignal.ResetContent();

		if( NULL != dynamic_cast<CDB_ElectroActuator *>( m_pHM->GetpCV()->GetActrIDPtr().MP ) )
		{
			CDB_ElectroActuator *pclElectroActuator = (CDB_ElectroActuator *)( m_pHM->GetpCV()->GetActrIDPtr().MP );

			for( int iLoop = 0; iLoop < ( int )pclElectroActuator->GetNumOfInputSignalsIDptr(); iLoop++ )
			{
				IDPTR InputSignalIDPtr = pclElectroActuator->GetInputSignalsIDPtr( iLoop );
				CDB_StringID *pclActrInputSignal = dynamic_cast<CDB_StringID *>( InputSignalIDPtr.MP );

				CString str;
				FormatString( str, pclActrInputSignal->GetString(), L"" );
				int iItem = m_CBInputSignal.AddString( str );
				m_CBInputSignal.SetItemData( iItem, (LPARAM)(void *)InputSignalIDPtr.MP );
			}
		}
	}

	if( m_CBInputSignal.GetCount() > 1 )
	{
		m_CBInputSignal.EnableWindow(TRUE);

		// Try to match with previous selection
		m_CBInputSignal.SetCurSel( 0 );
		int iPos = -1;

		if( NULL != m_pHM->GetpCV()->GetActrIDPtr().MP )
		{
			iPos = _FindCBLParam( &m_CBInputSignal, (DWORD_PTR)m_pHM->GetpCV()->GetActrSelectedSignalIDPtr().MP );
		}

		if( iPos < 0 )	// Return to Tech params
		{
			iPos = _FindCBLParam( &m_CBInputSignal, (DWORD_PTR)m_pTechParams->GetPrjParamIDPtr( CPrjParams::ActuatorInputSignalID ).MP ); 
		}

		if( iPos >= 0 )
		{
			m_CBInputSignal.SetCurSel( iPos );
		}
	}
	else
	{
		m_CBInputSignal.EnableWindow( FALSE );

		if( 1 == m_CBInputSignal.GetCount() )
		{
			m_CBInputSignal.SetCurSel( 0 );
		}
	}
	
	OnComboSelChangeInputSignal();
}

void CDlgSelectActuator::_SetCheckFailSafeFct()
{
	if( NULL == m_pHM || NULL == m_pHM->GetpCV() || NULL == dynamic_cast<CDB_ControlValve *>( m_pHM->GetpCV()->GetCvIDPtr().MP ) )
	{
		return;
	}

	if( eb3False == m_pHM->GetpCV()->GetCVSelectedAsaPackage() )
	{
		// Take the fail safes from all the actuators compatible with the current control valve.
		
		CDB_ControlValve *pCV = (CDB_ControlValve *)( m_pHM->GetpCV()->GetCvIDPtr().MP );

		CRank rkList;
		_MaptoRankList( &m_ActrFullList, &rkList );

		// Current selected 'Power supply'.
		CDB_StringID *pStrIDPowerSupply = NULL;

		if( m_CBPowerSupply.GetCurSel() > -1 )
		{
			pStrIDPowerSupply = dynamic_cast<CDB_StringID *>( (CData *)m_CBPowerSupply.GetItemData( m_CBPowerSupply.GetCurSel() ) );
		}

		// Current selected 'Input Signal'.
		CDB_StringID *pStrInputSignal = NULL;

		if( m_CBInputSignal.GetCurSel() > -1 )
		{
			pStrInputSignal = dynamic_cast<CDB_StringID*>( (CData *)m_CBInputSignal.GetItemData( m_CBInputSignal.GetCurSel() ) );
		}

		if( NULL == pStrIDPowerSupply || NULL == pStrInputSignal )
		{
			// Checkbox unchecked and disabled
			m_CheckboxFailSafe.SetCheck( BST_UNCHECKED );
			m_CheckboxFailSafe.EnableWindow( FALSE );
			m_CBDefaultReturnPosition.ResetContent();
			m_CBDefaultReturnPosition.EnableWindow( FALSE );
			m_CBActuator.ResetContent();
			m_CBActuator.EnableWindow( FALSE );
			GetDlgItem( IDOK )->EnableWindow( FALSE );
			return;
		}

		m_CheckBoxStatus = GetpTADB()->GetActuatorFailSafeValues( &rkList, m_pHM->GetCvCtrlType(), pStrIDPowerSupply->GetIDPtr().ID, pStrInputSignal->GetIDPtr().ID, m_eFilterSelection );

		if( CTADatabase::FailSafeCheckStatus::eFirst == m_CheckBoxStatus )
		{
			// Checkbox unchecked and disabled.
			m_CheckboxFailSafe.SetCheck( BST_UNCHECKED );
			m_CheckboxFailSafe.EnableWindow( FALSE );
		}
		else if( CTADatabase::FailSafeCheckStatus::eOnlyWithoutFailSafe == m_CheckBoxStatus )
		{
			// Checkbox unchecked and enable.
			m_CheckboxFailSafe.SetCheck( BST_UNCHECKED );
			m_CheckboxFailSafe.EnableWindow( FALSE );
		}
		else if( CTADatabase::FailSafeCheckStatus::eOnlyWithFailSafe == m_CheckBoxStatus )
		{
			// Checkbox checked and enable.
			m_CheckboxFailSafe.SetCheck( BST_CHECKED );
			m_CheckboxFailSafe.EnableWindow( FALSE );
		}
		else if( CTADatabase::FailSafeCheckStatus::eBothFailSafe == m_CheckBoxStatus )
		{
			// Checkbox checked and enable.
			m_CheckboxFailSafe.SetCheck( BST_UNCHECKED );
			m_CheckboxFailSafe.EnableWindow( TRUE );
		}

		if( NULL != dynamic_cast<CDB_ElectroActuator *>( m_pHM->GetpCV()->GetActrIDPtr().MP ) )
		{
			CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( m_pHM->GetpCV()->GetActrIDPtr().MP );

			ASSERT( -1 != (int)pclElectroActuator->GetFailSafe() );

			if( CDB_ElectroActuator::FailSafeType::eFSTypeElectronic <= pclElectroActuator->GetFailSafe() 
				&& CDB_ElectroActuator::FailSafeType::eFSTypeLast > pclElectroActuator->GetFailSafe() )
			{
				m_CheckboxFailSafe.SetCheck( BST_CHECKED );
			}
			else if ( 0 == (int)pclElectroActuator->GetFailSafe() )
			{
				m_CheckboxFailSafe.SetCheck( BST_UNCHECKED );
			}
		}
	}
	else
	{
		// Take only the fail safe status from the actuator belonging to the set.

		// If a set, we can't modify the checkbox.
		m_CheckboxFailSafe.EnableWindow( FALSE );
		
		if( NULL != dynamic_cast<CDB_ElectroActuator *>( m_pHM->GetpCV()->GetActrIDPtr().MP ) )
		{
			CDB_ElectroActuator *pclElectroActuator = (CDB_ElectroActuator *)( m_pHM->GetpCV()->GetActrIDPtr().MP );

			if( CDB_ElectroActuator::FailSafeType::eFSTypeElectronic <= pclElectroActuator->GetFailSafe() 
				&& CDB_ElectroActuator::FailSafeType::eFSTypeLast > pclElectroActuator->GetFailSafe() )
			{
				m_CheckboxFailSafe.SetCheck( BST_CHECKED );
			}
			else if ( 0 == (int)pclElectroActuator->GetFailSafe() )
			{
				m_CheckboxFailSafe.SetCheck( BST_UNCHECKED );
			}
		}
	}

	OnBnClickedCheckFailSafe();
}

void CDlgSelectActuator::_FillCBDefaultReturnPos()
{
	if( NULL == m_pHM || NULL == m_pHM->GetpCV() || NULL == dynamic_cast<CDB_ControlValve *>( m_pHM->GetpCV()->GetCvIDPtr().MP ) )
	{
		return;
	}

	CDB_ControlValve *pclControlValve = (CDB_ControlValve *)( m_pHM->GetpCV()->GetCvIDPtr().MP );

	if( eb3False == m_pHM->GetpCV()->GetCVSelectedAsaPackage() )
	{
		// Take the default return positions from all the actuators compatible with the current control valve.

		// Extract list of 'Default return position'.
		CRank rkFailSafeList, rkList;
		_MaptoRankList( &m_ActrFullList, &rkList );

		// Current selected 'Power supply'.
		CDB_StringID *pStrIDPowerSupply = NULL;

		if( m_CBPowerSupply.GetCurSel() > -1 )
		{
			pStrIDPowerSupply = dynamic_cast<CDB_StringID *>( (CData *)m_CBPowerSupply.GetItemData( m_CBPowerSupply.GetCurSel() ) );
		}
	
		// Current selected 'Input Signal'.
		CDB_StringID *pStrInputSignal= NULL;

		if( m_CBInputSignal.GetCurSel() > -1 )
		{
			pStrInputSignal = dynamic_cast<CDB_StringID*>( (CData *)m_CBInputSignal.GetItemData( m_CBInputSignal.GetCurSel() ) );
		}

		if( NULL == pStrIDPowerSupply || NULL == pStrInputSignal || -1 == m_CheckBoxStatus )
		{
			m_CBDefaultReturnPosition.ResetContent();
			m_CBDefaultReturnPosition.EnableWindow( FALSE );
			m_CBActuator.ResetContent();
			m_CBActuator.EnableWindow( FALSE );
			GetDlgItem( IDOK )->EnableWindow( FALSE );
			return;
		}
		
		ASSERT( -1 != m_CheckboxFailSafe.GetCheck() );
		int iFailSafe =  m_CheckboxFailSafe.GetCheck();
		
		GetpTADB()->GetActuatorDRPList( &rkFailSafeList, &rkList, m_pHM->GetCvCtrlType(), pclControlValve, pStrIDPowerSupply->GetIDPtr().ID, pStrInputSignal->GetIDPtr().ID, iFailSafe, m_eFilterSelection );

		rkFailSafeList.Transfer( &m_CBDefaultReturnPosition );
	}
	else
	{
		// Take only the default return position from the actuator belonging to the set.
		m_CBDefaultReturnPosition.ResetContent();

		if( NULL != dynamic_cast<CDB_ElectroActuator *>( m_pHM->GetpCV()->GetActrIDPtr().MP ) )
		{
			CDB_ElectroActuator *pclElectroActuator = (CDB_ElectroActuator *)( m_pHM->GetpCV()->GetActrIDPtr().MP );

			int iKey = pclControlValve->GetCompatibleDRPFunction( ( int )pclElectroActuator->GetDefaultReturnPos() );

			int iItem = m_CBDefaultReturnPosition.AddString( GetpTADB()->GetDRPFunctionStr( ( CDB_ControlValve::DRPFunction ) iKey ) );
			m_CBDefaultReturnPosition.SetItemData( iItem, (LPARAM)(void *)iKey );
		}
	}

	if( m_CBDefaultReturnPosition.GetCount() > 1 )
	{
		m_CBDefaultReturnPosition.EnableWindow( TRUE );
		
		// Try to match with previous selection.
		m_CBDefaultReturnPosition.SetCurSel( 0 );
		int iPos = -1;
		
		if( NULL != m_pHM->GetpCV()->GetActrIDPtr().MP )
		{
			CDB_ElectroActuator *pActr = dynamic_cast<CDB_ElectroActuator *>( m_pHM->GetpCV()->GetActrIDPtr().MP );
			if( NULL != pActr )
			{
				for( int i = 0; i < m_CBDefaultReturnPosition.GetCount() && iPos < 0; i++ )
				{
					if( ( CDB_ControlValve::DRPFunction )(pclControlValve->GetCompatibleDRPFunction( pActr->GetDefaultReturnPos() ) ) == (CDB_ControlValve::DRPFunction) m_CBDefaultReturnPosition.GetItemData( i ) )
					{
						iPos = i;
						break;
					}
				}
			}
		}
		
		// Return to Tech params.
		if( iPos < 0 )
		{
			for( int i = 0; i < m_CBDefaultReturnPosition.GetCount() && iPos < 0; i++ )
			{
				if( m_pTechParams->GetActuatorDRPFct() == ( CDB_ControlValve::DRPFunction ) m_CBDefaultReturnPosition.GetItemData( i ) )
				{
					iPos = i;
					break;
				}
			}
		}
		
		if( iPos >= 0 )
		{
			m_CBDefaultReturnPosition.SetCurSel( iPos );
		}
	}
	else
	{
		m_CBDefaultReturnPosition.EnableWindow( FALSE );

		if( 1 == m_CBDefaultReturnPosition.GetCount() )
		{
			m_CBDefaultReturnPosition.SetCurSel( 0 );
		}
	}

	OnComboSelChangeDRP();
}

void CDlgSelectActuator::_FillCBActuator()
{
	if( NULL == m_pHM || NULL == m_pHM->GetpCV() || NULL == dynamic_cast<CDB_ControlValve*>( m_pHM->GetpCV()->GetCvIDPtr().MP ) )
	{
		return;
	}

	if( eb3False == m_pHM->GetpCV()->GetCVSelectedAsaPackage() )
	{
		// Take the actuators compatible with the current control valve.

		CRank rkActrList, rkList;
		_MaptoRankList( &m_ActrFullList, &rkList );

		// Current selected 'Power Supply'.
		CDB_StringID *pStrIDPowerSupply = NULL;

		if( m_CBPowerSupply.GetCurSel() > -1 )
		{
			pStrIDPowerSupply = dynamic_cast<CDB_StringID *>( (CData *)m_CBPowerSupply.GetItemData( m_CBPowerSupply.GetCurSel() ) );
		}
	
		// Current selected 'Input Signal'.
		CDB_StringID *pStrInputSignal= NULL;

		if( m_CBInputSignal.GetCurSel() > -1 )
		{
			pStrInputSignal = dynamic_cast<CDB_StringID *>( (CData *)m_CBInputSignal.GetItemData( m_CBInputSignal.GetCurSel() ) );
		}
	
		// Current selected 'Fail safe function'.
		CDB_ControlValve::DRPFunction eDRPFct = CDB_ControlValve::DRPFunction::drpfLast;

		if( m_CBDefaultReturnPosition.GetCurSel() > -1 )
		{
			eDRPFct = (CDB_ControlValve::DRPFunction)m_CBDefaultReturnPosition.GetItemData( m_CBDefaultReturnPosition.GetCurSel() );
		}
	
		if( NULL != pStrIDPowerSupply && NULL != pStrInputSignal && CDB_ControlValve::DRPFunction::drpfLast != eDRPFct && m_CheckBoxStatus != -1)
		{
			CDB_ControlValve *pclControlValve = (CDB_ControlValve *)( m_pHM->GetpCV()->GetCvIDPtr().MP );
			int iFailsafe = ( BST_CHECKED == m_CheckboxFailSafe.GetCheck() ) ? 1 : 0;
			CDB_ElectroActuator::DefaultReturnPosition eActuatorDRP = (CDB_ElectroActuator::DefaultReturnPosition)pclControlValve->GetCompatibleActuatorDRP( eDRPFct );
			GetpTADB()->GetActuator( &rkActrList, &rkList, pStrIDPowerSupply->GetIDPtr().ID, pStrInputSignal->GetIDPtr().ID, iFailsafe, eActuatorDRP, m_eFilterSelection );

			// HYS-941: Replace the call to 'rkActrList.Transfer' by this code below to show the actuator name and not the ID.
			m_CBActuator.ResetContent();
			CString str;
			LPARAM itemdata;

			for( BOOL bContinue = rkActrList.GetFirst( str, itemdata ); TRUE == bContinue; bContinue = rkActrList.GetNext( str, itemdata ) )
			{
				CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( (CData *)itemdata );

				if( NULL == pclActuator )
				{
					ASSERT( 0 );
					continue;
				}

				CString strName = pclActuator->GetName();

				// If thermostatic actuator we add also the article number to differentiate.
				if( NULL != dynamic_cast<CDB_ThermostaticActuator *>( pclActuator ) )
				{
					strName += _T(" [" ) + CString( pclActuator->GetArtNum() ) + _T( "]");
				}

				int iItem = m_CBActuator.AddString( strName );
				m_CBActuator.SetItemData( iItem, itemdata );
			}
		}
	}
	else
	{
		// Take the actuator belonging to the set.
		m_CBActuator.ResetContent();

		if( NULL != dynamic_cast<CDB_ElectroActuator *>( m_pHM->GetpCV()->GetActrIDPtr().MP ) )
		{
			CDB_ElectroActuator *pclActuator = (CDB_ElectroActuator *)( m_pHM->GetpCV()->GetActrIDPtr().MP );

			int iItem = m_CBActuator.AddString( pclActuator->GetName() );
			m_CBActuator.SetItemData( iItem, (LPARAM)(void *)pclActuator );
		}
	}
	
	if( m_CBActuator.GetCount() > 1 )
	{
		m_CBActuator.EnableWindow( TRUE );
		GetDlgItem( IDOK )->EnableWindow( true );
		// Try to match with previous selection.
		m_CBActuator.SetCurSel( 0 );
		int iPos = -1;
		
		if( NULL != m_pHM->GetpCV()->GetActrIDPtr().MP )
		{
			iPos = _FindCBLParam( &m_CBActuator, (DWORD_PTR)m_pHM->GetpCV()->GetActrIDPtr().MP );
		}
		else	// Return to Tech params
		{
			iPos = 0;
		}

		if( iPos >= 0 )
		{
			m_CBActuator.SetCurSel( iPos );
		}
	}
	else
	{
		m_CBActuator.EnableWindow( FALSE );

		if( 1 == m_CBActuator.GetCount() )
		{
			m_CBActuator.SetCurSel( 0 );
			GetDlgItem( IDOK )->EnableWindow( true );
		}
		else
		{
			GetDlgItem( IDOK )->EnableWindow( false );
		}
	}

	OnComboSelChangeActuator();
}

void  CDlgSelectActuator::_MaptoRankList( std::multimap< double, CDB_Actuator *> *pActrMap, CRank *pRkList)
{
	std::multimap< double, CDB_Actuator *>::iterator It;

	if( 0 == pActrMap->size() )
	{
		return;
	}

	for( It = pActrMap->begin(); It != pActrMap->end(); It++ )
	{
		pRkList->Add( (It->second)->GetName(), It->first, (LPARAM)It->second );
	}
}

int CDlgSelectActuator::_FindCBLParam( CComboBox *pCB, DWORD_PTR DataMP )
{
	if( NULL == pCB )
	{
		return -1;
	}

	int iPos = -1;
	
	for( int i = 0; i < pCB->GetCount() && iPos < 0; i++ )
	{
		if( DataMP == pCB->GetItemData( i ) )
		{
			iPos = i;
		}
	}

	return iPos;
}
