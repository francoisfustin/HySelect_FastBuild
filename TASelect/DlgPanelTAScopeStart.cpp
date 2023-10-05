#include "stdafx.h"
#include "TASelect.h"
#include "HMInclude.h"

#ifdef DEBUG
#include "DlgSpecAct.h"
#endif

#include "DlgWizTAScope.h"
#include "DlgPanelTAScopeStart.h"

CDlgPanelTAScopeStart::CDlgPanelTAScopeStart( CWnd *pParent )
	: CDlgWizard( CDlgPanelTAScopeStart::IDD, pParent )
{
	m_pParent = NULL;
	m_fTADSHNEmpty = false;
	m_fConnectionEstablished = false;
	m_strPreviousDpsVersion = _T( "" );
	m_strPreviousHHVersion = _T( "" );
	m_nTimer = ( UINT_PTR )0;
}

void CDlgPanelTAScopeStart::SetBtnInfoText( int iIDInfo, int iIDstr )
{
	SetBtnInfoText( iIDInfo, TASApp.LoadLocalizedString( iIDstr ) );
}

void CDlgPanelTAScopeStart::SetBtnInfoText( int iIDInfo, CString strText )
{
	if( NULL != GetDlgItem( iIDInfo ) )
	{
		GetDlgItem( iIDInfo )->ShowWindow( !strText.IsEmpty() );
		GetDlgItem( iIDInfo )->SetWindowText( strText );
	}
}

BEGIN_MESSAGE_MAP( CDlgPanelTAScopeStart, CDlgWizard )
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED( IDC_BUTDOWNLOAD, OnBnClickedDownLoad )
	ON_BN_CLICKED( IDC_BUTUPLOAD, OnBnClickedUpLoad )
	ON_BN_CLICKED( IDC_BUTMAINTENANCE, OnBnClickedMaintenance )
	ON_NOTIFY( NM_CLICK, IDC_SYSLINK2, OnNMClickSyslink2 )
	ON_BN_CLICKED( IDC_CHECKOFFLINE, OnClickedCheckoffline )
END_MESSAGE_MAP()

void CDlgPanelTAScopeStart::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizard::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICVERSION, m_StaticVersionNbr );
	DDX_Control( pDX, IDC_STATICIMGTASCOPE, m_ImgTASCOPEHeader );
	DDX_Control( pDX, IDC_STATICLEFTBANNER, m_LeftBanner );
	DDX_Control( pDX, IDC_BUTDOWNLOAD, m_ButDownLoad );
	DDX_Control( pDX, IDC_BUTUPLOAD, m_ButUpLoad );
	DDX_Control( pDX, IDC_BUTMAINTENANCE, m_ButMaintenance );
	DDX_Control( pDX, IDC_STATICFLAG, m_StaticFlag );
	DDX_Control( pDX, IDC_SYSLINK2, m_UpdateAvailable );
	DDX_Control( pDX, IDC_CHECKOFFLINE, m_CheckOffLine );
}

BOOL CDlgPanelTAScopeStart::OnInitDialog()
{
	CDlgWizard::OnInitDialog();
	m_pParent = ( CDlgWizTAScope * )GetWizMan();

	// Set the white rectangle to the first drawn window.
	GetDlgItem( IDC_STATICRECTANGLE )->BringWindowToTop();
	m_brWhiteBrush.CreateSolidBrush( RGB( 255, 255, 255 ) );
	m_brBtnFaceBrush.CreateSolidBrush( GetSysColor( COLOR_BTNFACE ) );
	GetDlgItem( IDC_EDITTITLE )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_WELCOMETITLE ) );

	// Hide header icon.
	m_ImgTASCOPEHeader.ShowWindow( SW_HIDE );

	// Initialize strings.
	GetDlgItem( IDC_STATICSELECTTODO )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_SELECTOPERATION ) );
	GetDlgItem( IDC_STATICDETECTEDTASCOPE )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_VERSIONDETECTED ) );

	// Initialize update flag BMP.
	m_StaticFlag.SetImageID( IDB_FLAG_RED );
	m_StaticFlag.ShowWindow( SW_HIDE );
	m_UpdateAvailable.SetWindowText( _T("<a>" ) + TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_UPDATEAVAILABLE ) + _T( "</a>") );
	m_UpdateAvailable.ShowWindow( SW_HIDE );

	m_fConnectionEstablished = false;
	m_strPreviousDpsVersion = L"-";
	m_strPreviousHHVersion = L"-";

	m_CheckOffLine.ShowWindow( FALSE );

#ifdef DEBUG
	// Developer only.
	m_CheckOffLine.SetCheck( FALSE );
	CDlgSpecAct dlg;

	if( TRUE == dlg.CheckFile() )
	{
		m_CheckOffLine.ShowWindow( TRUE );
	}
#endif

	// Initialize buttons.
	m_ButDownLoad.SetImage( IDB_COMM_TASCOPE_DOWNLOAD_48, IDB_COMM_TASCOPE_DOWNLOAD_48 );

	// Use the application menu font at the button text font.
	m_ButDownLoad.EnableMenuFont();

	// Use the current Windows theme to draw the button borders.
	m_ButDownLoad.EnableWindowsTheming( TRUE );

	m_ButDownLoad.SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_BUTTONDOWNLOAD ) );

	m_ButUpLoad.SetImage( IDB_COMM_TASCOPE_UPLOAD_48, IDB_COMM_TASCOPE_UPLOAD_48 );
	m_ButUpLoad.SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_BUTTONUPLOAD ) );
	m_ButMaintenance.SetImage( IDB_COMM_TASCOPE_MAINT_48, IDB_COMM_TASCOPE_MAINT_48 );
	m_ButMaintenance.SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_BUTTONMAINTENANCE ) );

	// Check if something is available for uploading.
	m_fTADSHNEmpty = true;
	CArray<CTable *> aTab;
	CTable *pTab = TASApp.GetpTADS()->GetpHydroModTable();
	aTab.Add( pTab );

	pTab = TASApp.GetpTADS()->GetpHUBSelectionTable();
	aTab.Add( pTab );

	for( int j = 0; j < aTab.GetCount() && m_fTADSHNEmpty; j++ )
	{
		pTab = aTab[j];
		ASSERT( pTab );

		if( NULL != pTab &&  NULL != dynamic_cast<CDS_HydroMod *>( pTab->GetFirst().MP ) )
		{
			m_fTADSHNEmpty = false;
		}
	}

	// Button will be enabled when the communication is established.
	m_ButDownLoad.EnableWindow( FALSE );
	m_ButUpLoad.EnableWindow( FALSE );
	m_ButMaintenance.EnableWindow( FALSE );

	// Clear all BtnInfos.
	SetBtnInfoText( IDC_STATICINFOBTNDOWNLOAD, _T( "" ) );
	SetBtnInfoText( IDC_STATICINFOBTNUPLOAD, _T( "" ) );
	SetBtnInfoText( IDC_STATICINFOBTNMAINTENANCE, _T( "" ) );

	m_nTimer = SetTimer( _TIMERID_DLGPANELTASCOPESTART, 500, 0 );					// Let the time to initialize screen
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgPanelTAScopeStart::OnTimer( UINT_PTR nIDEvent )
{
	CDlgWizard::OnTimer( nIDEvent );

	if( ( UINT_PTR )0 == m_nTimer || m_nTimer != nIDEvent )
	{
		return;
	}

	if( true == m_pParent->IsComPortBuzy() )
	{
		return;
	}

	KillTimer( m_nTimer );
	m_nTimer = ( UINT_PTR )0;

	if( true == m_pParent->GetRefreshVersion() )
	{
		// Check connection every timer event (1 sec).
		if( !UpdateVersionNumber() )
		{
			m_pParent->ClearSoftwareVersion();
			m_pParent->FoundTAScopePortCom();
		}
	}

	m_nTimer = SetTimer( _TIMERID_DLGPANELTASCOPESTART, 1000, 0 );					// Retry after 1sec to verify connection
}

HBRUSH CDlgPanelTAScopeStart::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
{
	HBRUSH hbr;

	if( pWnd->m_hWnd == GetDlgItem( IDC_EDITTITLE )->m_hWnd )
	{
		pDC->SetTextColor( RGB( 0, 0, 0 ) );
		pDC->SetBkColor( RGB( 255, 255, 255 ) );
		return ( HBRUSH )m_brWhiteBrush;
	}

	hbr = CDlgWizard::OnCtlColor( pDC, pWnd, nCtlColor );
	return hbr;
}

void CDlgPanelTAScopeStart::OnBnClickedDownLoad()
{
	m_pParent->JumpToPanel( CDlgWizTAScope::epanDownLoad );
}

void CDlgPanelTAScopeStart::OnBnClickedUpLoad()
{
	m_pParent->JumpToPanel( CDlgWizTAScope::epanUpLoad );
}

void CDlgPanelTAScopeStart::OnBnClickedMaintenance()
{
	m_pParent->JumpToPanel( CDlgWizTAScope::epanMaintenance );
}

void CDlgPanelTAScopeStart::OnNMClickSyslink2( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( true == m_pParent->IsTAScopeAvailable() )
	{
		m_pParent->JumpToPanel( CDlgWizTAScope::epanUpdate );
	}

	if( NULL != pResult )
	{
		*pResult = 0;
	}
}

void CDlgPanelTAScopeStart::OnClickedCheckoffline()
{
	bool fFlag = ( TRUE == m_CheckOffLine.GetCheck() ) ? true : false;
	m_pParent->SetComForcedOffLine( fFlag );
	// Force a refresh of Upload/Download buttons.
	UpdateVersionNumber();
}

BOOL CDlgPanelTAScopeStart::OnWizNext()
{
	return FALSE;
}

bool CDlgPanelTAScopeStart::OnAfterActivate()
{
	// 'Next' and 'Finish' buttons.
	m_pParent->ShowButtons( CWizardManager::WizButNext, false );

	// Just to have the focus on something and to be able to intercept the [ESCAPE] in the 'CDlgWizard' base class.
	m_CheckOffLine.SetFocus();

	return true;
}

int CDlgPanelTAScopeStart::OnWizButFinishTextID()
{
	return IDS_TASCOPE_PANELSTART_BUTTONCLOSE;
}

bool CDlgPanelTAScopeStart::UpdateVersionNumber()
{
	CString str;
	bool fRet = false;

	// TA-SCOPE is there.
	fRet = m_pParent->IsTAScopeAvailable();

	if( true == fRet && false == m_pParent->IsInBootMode() )
	{
		fRet = m_pParent->RequestSoftwareVersion();
	}

	// If TAScope communication is OK...
	if( true == fRet )
	{
		bool fDownLoadHFTNeeded = false;

		if( m_pParent->GetpArCurrentVersion()->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvHH ) != m_strPreviousHHVersion )
		{
			m_pParent->DischargeCurrentProject();
			fDownLoadHFTNeeded = true;
		}

		m_strPreviousHHVersion = m_pParent->GetpArCurrentVersion()->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvHH );

		// Verify DpS status only one time.
		if( m_pParent->GetpArCurrentVersion()->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvDPS ) != m_strPreviousDpsVersion )
		{
			fDownLoadHFTNeeded = true;
		}

		if( false == fDownLoadHFTNeeded )
		{
			return true;
		}

		m_strPreviousDpsVersion = m_pParent->GetpArCurrentVersion()->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvDPS );
		m_StaticVersionNbr.SetWindowText( m_pParent->GetpArCurrentVersion()->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvHH ) );

		// Check if an update is available.
		GetDlgItem( IDC_STATICUPDATECOMMENT )->ShowWindow( SW_HIDE );
		m_pParent->ActivateRefresh( false );

		CDlgWizTAScope::eTAScopeUpdate HFTstatus;
		HFTstatus = m_pParent->CheckForUpdate();
		m_pParent->ActivateRefresh( true );

		if( CDlgWizTAScope::eTAScopeUpdate::eupdUpdateAvailable == HFTstatus )
		{
			m_StaticFlag.SetImageID( IDB_FLAG_RED );
			m_StaticFlag.ShowWindow( SW_SHOW );

			// <a> balise for blue effect.
			m_UpdateAvailable.SetWindowText( _T("<a>" ) + TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_UPDATEAVAILABLE ) + _T( "</a>") );
			m_UpdateAvailable.ShowWindow( SW_SHOW );

			// Disable upload possibility if needed when an update is available.
			_ActivateButtonsDownloadUpload( true );

			if( true == m_pParent->IsInBootMode() )
			{
				m_StaticVersionNbr.SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_BOOTMODE ) );
				return true;
			}
		}
		else if( CDlgWizTAScope::eTAScopeUpdate::eupdNoUpdateNeeded == HFTstatus )
		{
			_ActivateButtonsDownloadUpload( true );
			m_StaticFlag.ShowWindow( SW_HIDE );
			m_UpdateAvailable.ShowWindow( SW_HIDE );
		}
		else
		{
			m_StaticFlag.SetImageID( IDB_WARNING16 );
			m_StaticFlag.ShowWindow( SW_SHOW );
			m_UpdateAvailable.ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICUPDATECOMMENT )->ShowWindow( SW_HIDE );

			CString str;

			switch( HFTstatus )
			{
				case CDlgWizTAScope::eTAScopeUpdate::eupdTASCOPEHftMissing:
					GetDlgItem( IDC_STATICUPDATECOMMENT )->SetWindowText( TASApp.LoadLocalizedString( IDS_DLGPANELSTART_WARNINGMISSINGTASCOPEHFT ) );
					GetDlgItem( IDC_STATICUPDATECOMMENT )->ShowWindow( SW_SHOW );
					break;

				case CDlgWizTAScope::eTAScopeUpdate::eupdLocalHftMissing:
					GetDlgItem( IDC_STATICUPDATECOMMENT )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_NOLOCALSFT ) );
					GetDlgItem( IDC_STATICUPDATECOMMENT )->ShowWindow( SW_SHOW );
					break;

				case CDlgWizTAScope::eTAScopeUpdate::eupdIncorrectCRC:
					GetDlgItem( IDC_STATICUPDATECOMMENT )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_INCORRECTSFTCRC ) );
					GetDlgItem( IDC_STATICUPDATECOMMENT )->ShowWindow( SW_SHOW );
					break;

				default:
				case CDlgWizTAScope::eTAScopeUpdate::eupdFail:
					m_StaticFlag.ShowWindow( SW_HIDE );
					GetDlgItem( IDC_STATICUPDATECOMMENT )->ShowWindow( SW_HIDE );
					break;
			}

			_ActivateButtonsDownloadUpload( true );
		}

		m_fConnectionEstablished = true;
		return true;
	}
	else
	{
		// No TAScope communication available disable fields.
		m_fConnectionEstablished = false;
		m_strPreviousDpsVersion = L"-";
		m_strPreviousHHVersion = L"-";

		// Disable buttons.
		_ActivateButtonsDownloadUpload( false );
		m_StaticVersionNbr.SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_NOCONNECTED ) );

		m_StaticFlag.ShowWindow( SW_HIDE );
		m_UpdateAvailable.ShowWindow( SW_HIDE );

		return false;
	}

	return false;
}

void CDlgPanelTAScopeStart::_ActivateButtonsDownloadUpload( bool fEnable )
{
	if( true == m_pParent->IsComForcedOffLine() )
	{
		m_ButDownLoad.EnableWindow( TRUE );
		m_ButUpLoad.EnableWindow( TRUE );
		m_ButMaintenance.EnableWindow( TRUE );
		return;
	}

	if( false == fEnable || true == m_pParent->IsInBootMode() )
	{
		m_ButDownLoad.EnableWindow( FALSE );
		m_ButUpLoad.EnableWindow( FALSE );
		m_ButMaintenance.EnableWindow( FALSE );
		SetBtnInfoText( IDC_STATICINFOBTNDOWNLOAD, _T( "" ) );
		SetBtnInfoText( IDC_STATICINFOBTNUPLOAD, _T( "" ) );
		SetBtnInfoText( IDC_STATICINFOBTNMAINTENANCE, _T( "" ) );
	}
	else
	{
		unsigned int uiTASDSXVersion = _ttoi( CTADATASTRUCTX_FORMAT_VERSION );

		if( uiTASDSXVersion < m_pParent->GetTAScopeDSVersion() )
		{
			// Download is possible if the current HySelect datastruct version is greater or egual to the TA-SCOPE datastruct version.
			m_ButDownLoad.EnableWindow( FALSE );
			SetBtnInfoText( IDC_STATICINFOBTNDOWNLOAD, IDS_TASCOPE_PANELSTART_INFOBTNDOWNLOAD_PROCEEDTASUPDATEFIRST );
		}
		else
		{
			m_ButDownLoad.EnableWindow( TRUE );
			SetBtnInfoText( IDC_STATICINFOBTNDOWNLOAD, _T( "" ) );

			// Upload is possible if the TASCOPE datastruct version is greater or equal to the HySelect DS version.
			if( uiTASDSXVersion > m_pParent->GetTAScopeDSVersion() )
			{
				m_ButUpLoad.EnableWindow( FALSE );
				SetBtnInfoText( IDC_STATICINFOBTNUPLOAD, IDS_TASCOPE_PANELSTART_INFOBTNUPLOAD_PROCEEDUPDATEFIRST );
			}
			else	// Datastruct matchings
			{
				// Something to Upload?
				if( false == m_fTADSHNEmpty )
				{
					m_ButUpLoad.EnableWindow( TRUE );
				}
				else
				{
					m_ButUpLoad.EnableWindow( FALSE );
				}

				SetBtnInfoText( IDC_STATICINFOBTNDOWNLOAD, _T( "" ) );
			}

			m_ButMaintenance.EnableWindow( TRUE );
		}
	}
}
