#include "stdafx.h"
#include "TASelect.h"
#include "FileTableMng.h"
#include "DlgWizTAScope.h"
#include "DlgPanelTAScopeUpdate.h"

using namespace TASCOPE;

CDlgPanelTAScopeUpdate::CDlgPanelTAScopeUpdate( CWnd *pParent )
	: CDlgWizard( CDlgPanelTAScopeUpdate::IDD, pParent )
{
	m_pParent = NULL;
	m_dTotalSize = 0.0;
	m_dAlreadyUplploaded = 0.0;
	m_dAlreadyUploadedCateg = 0.0;
	m_fUploadInCourse = false;
	m_fUploadHFT = false;
	m_fRefreshingHFTNeeded = false;
	m_iCurrentCategory = -1;
	m_eUpdateZBHH = eProcessingZBUpdate::No;
	m_eUpdateZBDPS = eProcessingZBUpdate::No;
	m_nTimer = ( UINT_PTR )0;
}

BEGIN_MESSAGE_MAP( CDlgPanelTAScopeUpdate, CDlgWizard )
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

void CDlgPanelTAScopeUpdate::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizard::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_FILELIST, m_FileList );
	DDX_Control( pDX, IDC_PROGRESS, m_Progress );
	DDX_Control( pDX, IDC_PROGRESSALL, m_ProgressAll );
	DDX_Control( pDX, IDC_EDITINFOS, m_EditInfos );
	DDX_Control( pDX, IDC_STATICBMPLINK, m_ConnectionPicture );
	DDX_Control( pDX, IDC_STATICVERSION, m_GroupVersion );
	DDX_Control( pDX, IDC_STATICSFTVERSION, m_GroupSFTVersion );
	DDX_Control( pDX, IDC_STATICGBINFOS, m_GroupGBInfo );
}

BOOL CDlgPanelTAScopeUpdate::OnInitDialog()
{
	CDlgWizard::OnInitDialog();
	m_pParent = ( CDlgWizTAScope * )GetWizMan();

	m_fUploadInCourse = false;

	// Set the white rectangle to the first drawn window.
	GetDlgItem( IDC_STATICRECTANGLE )->BringWindowToTop();
	m_brWhiteBrush.CreateSolidBrush( RGB( 255, 255, 255 ) );

	// Connection picture.
	m_Bmp.LoadImage( GIF_CONNECT_TO_PC, L"GIF", 0, GetSysColor( COLOR_BTNFACE ) );
	m_ConnectionPicture.SetBitmap( ( HBITMAP )m_Bmp.GetSafeHandle() );

	// Title.
	GetDlgItem( IDC_EDITTITLE )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_TITLE ) );

	// Filling strings.
	m_GroupVersion.SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_STATICVERSION ) );
	GetDlgItem( IDC_EDITWARNING )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_CONNECTIONWARNING ) );
	m_GroupSFTVersion.SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_STATICSFTVERSION ) );
	GetDlgItem( IDC_CHECKBACKUP )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_CHECKBACKUP ) );
	( ( CButton * )GetDlgItem( IDC_CHECKBACKUP ) )->SetCheck( BST_CHECKED );

	// Fill Edit.
	m_SmallImageList.Create( IDB_TASCOPEUPDATEICONS, 16, 1, _BLACK );
	m_FileList.SetImageList( &m_SmallImageList, LVS_REPORT );
	m_FileList.ModifyStyle( LVS_TYPEMASK, LVS_REPORT & LVS_TYPEMASK );

	CString str = TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_ACTION );
	m_FileList.InsertColumn( 0, str, LVCFMT_LEFT, 290 );
	str = TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_STATUS );
	m_FileList.InsertColumn( 1, str, LVCFMT_RIGHT, 100 );

	m_nTimer = SetTimer( _TIMERID_DLGPANELTASCOPEUPDATE, 100, 0 );			// Update Version/progress Bar each 100ms

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgPanelTAScopeUpdate::OnTimer( UINT_PTR nIDEvent )
{
	CDlgWizard::OnTimer( nIDEvent );

	if( ( UINT_PTR )0 == m_nTimer || m_nTimer != nIDEvent )
	{
		return;
	}

	if( true == m_fUploadInCourse )
	{
		// Updating in course.
		BeginWaitCursor();

		if( true == m_fUploadHFT )
		{
			m_Progress.SetPos( 0 );
			return;
		}

		double dVal = m_pParent->GetProgressValue();

		// Special processing for updating ZB processor.
		if( eProcessingZBUpdate::InCourse == m_eUpdateZBDPS || eProcessingZBUpdate::InCourse == m_eUpdateZBHH )
		{
			if( dVal > 100.0 )
			{
				m_pParent->SetProgressValue( 0.0 );
			}

			m_Progress.SetPos( ( short )dVal );
		}
		else
		{
			CString str;
			CategoryItem Item = m_ArCateg.GetAt( m_iCurrentCategory );
			double d = ( dVal * m_pParent->GetHftTable()->GetSize() / 100 + m_dAlreadyUploadedCateg ) / Item.TotalFileSize * 100.0;

			if( d > 100.0 )
			{
				d = 100.0;
			}

			m_Progress.SetPos( ( short )d );
			str.Format( _T("%3.1f/%3.1f kB"), d * Item.TotalFileSize / 102400.0, ( Item.TotalFileSize / 1024.0 ) );
			m_FileList.SetItemText( m_iCurrentCategory, 1, ( LPCTSTR )str );

			// Compute AllProgress.
			double dAllVal = ( dVal * m_pParent->GetHftTable()->GetSize() / 100.0 + m_dAlreadyUplploaded ) / m_dTotalSize * 100.0;
			m_ProgressAll.SetPos( ( short )dAllVal );
		}
	}
	else
	{
		CString str;
		CString str1;
		
		// Ask for version refreshing.
		m_pParent->RequestSoftwareVersion();
		CArray<CString, CString> *pAr = m_pParent->GetpArCurrentVersion();

		if( true == pAr->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvHH ).IsEmpty() && false == m_pParent->IsInBootMode() )	// Not connected
		{
			m_pParent->DisableButtons( CWizardManager::enum_WizBut::WizButNext );

			str1 = CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_NOCONNECTED );
			FormatString( str, IDS_TASCOPE_PANELUPDATE_STATICHHVERSION, str );
			GetDlgItem( IDC_STATICHH )->GetWindowText( str1 );

			// Update fields when its needed.
			if( str1 != str )
			{
				GetDlgItem( IDC_STATICHH )->SetWindowText( str );
				FormatString( str, IDS_TASCOPE_PANELUPDATE_STATICDBVERSION, _T("-") );
				GetDlgItem( IDC_STATICDB )->SetWindowText( str );
				FormatString( str, IDS_TASCOPE_PANELUPDATE_STATICZBHHVERSION, _T("-") );
				GetDlgItem( IDC_STATICZBHH )->SetWindowText( str );
				FormatString( str, IDS_TASCOPE_PANELUPDATE_STATICDPSVERSION, _T("-") );
				GetDlgItem( IDC_STATICDPS )->SetWindowText( str );
			}
		}
		else
		{
			m_pParent->EnableButtons( CWizardManager::enum_WizBut::WizButNext );

			CString str1;
			GetDlgItem( IDC_STATICHH )->GetWindowText( str1 );
			FormatString( str, IDS_TASCOPE_PANELUPDATE_STATICHHVERSION, pAr->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvHH ) );

			// Update fields on when needed.
			if( str1 != str )
			{
				GetDlgItem( IDC_STATICHH )->SetWindowText( str );
				FormatString( str, IDS_TASCOPE_PANELUPDATE_STATICDBVERSION, pAr->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvDB ) );
				int pos = str.ReverseFind( '.' );
				str.Delete( pos, str.GetLength() - pos );
				str = str + _T("-") + pAr->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvDBKey );
				GetDlgItem( IDC_STATICDB )->SetWindowText( str );
			}

			// HH ZigBee can be refreshed later, update it independently.
			CString strWirelessVersion =_T("");

			if( CDlgWizTAScope::eHandheldWirelessAtmel == m_pParent->GetHHWirelessModuleType() )
			{
				strWirelessVersion = pAr->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvWireless );

				if( false == strWirelessVersion.IsEmpty() )
				{
					strWirelessVersion += _T("-AT");
				}
			}
			else if( CDlgWizTAScope::eHandheldWirelessDresden == m_pParent->GetHHWirelessModuleType() )
			{
				// Dresden wireless version with modification (bit 7 cleared) is saved in this other variable.
				strWirelessVersion = pAr->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvWirelessDresden );
				
				if( false == strWirelessVersion.IsEmpty() )
				{
					strWirelessVersion += _T("-DR");
				}
			}

			FormatString( str, IDS_TASCOPE_PANELUPDATE_STATICZBHHVERSION, strWirelessVersion );
			GetDlgItem( IDC_STATICZBHH )->GetWindowText( str1 );

			if( str != str1 )
			{
				GetDlgItem( IDC_STATICZBHH )->SetWindowText( str );
			}

			// Update DPS field independently.
			GetDlgItem( IDC_STATICDPS )->GetWindowText( str1 );

			if( _T("-") == pAr->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvDPS ) )	// Not Connected
			{
				str1 = CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_NOCONNECTED );
				FormatString( str, IDS_TASCOPE_PANELUPDATE_STATICDPSVERSION, str1 );
				GetDlgItem( IDC_STATICHH )->GetWindowText( str1 );

				// Update fields when its needed.
				if( str1 != str )
				{
					GetDlgItem( IDC_STATICDPS )->SetWindowText( str );
				}
			}
			else	// DPS Connected
			{
				FormatString( str, IDS_TASCOPE_PANELUPDATE_STATICDPSVERSION, pAr->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvDPS ) );

				if( str1 != str )
				{
					GetDlgItem( IDC_STATICDPS )->SetWindowText( str );
				}
			}
		}
	}
}

HBRUSH CDlgPanelTAScopeUpdate::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor )
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

BOOL CDlgPanelTAScopeUpdate::OnWizCancel()
{
	BOOL fReturn = TRUE;

	if( true == m_fUploadInCourse )
	{
		// If download is running and user doesn't want to cancel operation, 'OnWizNext' returns 'FALSE'. But the methods also returns 'FALSE' is
		// user stops download. We must thus check 'm_fUploadInCourse'.
		OnWizNext();
		fReturn = ( true == m_fUploadInCourse ) ? FALSE : TRUE;
	}
	else
	{
		KillTimer( m_nTimer );
		m_nTimer = ( UINT_PTR )0;
	}

	return fReturn;
}

BOOL CDlgPanelTAScopeUpdate::OnWizNext()
{
	// 'Update' ('m_fUploadInCourse' set to 'false') or 'Cancel update' ('m_fUploadInCourse' set to 'true') button.
	m_pParent->ActivateRefresh( false );

	// By default we kill the timer to have no interference with this method.
	KillTimer( m_nTimer );
	m_nTimer = ( UINT_PTR )0;

	BOOL fReturn = TRUE;

	if( false == m_fUploadInCourse )
	{
		m_pParent->SetButtonText( CWizardManager::WizButNext, IDS_TASCOPE_PANELUPDATE_CANCELUPDATE );
		m_fUploadInCourse = true;
		m_pParent->DisableButtons( CWizardManager::enum_WizBut::WizButFinish );
		InitFileList();
		fReturn = m_pParent->BackupTASCOPEFiles( ( ( CButton * )GetDlgItem( IDC_CHECKBACKUP ) )->GetCheck() ? true : false );
	}
	else
	{
		fReturn = FALSE;
	}

	if( FALSE == fReturn )
	{
		// Stop uploading?
		if( IDYES != AfxMessageBox( IDS_TASCOPE_PANELUPDATE_AFXWARN_CANCELUPDATE, MB_YESNO | MB_ICONSTOP ) )
		{
			// We must relaunch the timer.
			m_nTimer = SetTimer( _TIMERID_DLGPANELTASCOPEUPDATE, 100, 0 );			// Update Version/progress Bar each 100ms
			return FALSE;
		}

		// Yes.
		TASCOPE::ServiceManager::StopCurrentService();
		m_pParent->SetButtonText( CWizardManager::enum_WizBut::WizButNext, IDS_TASCOPE_PANELUPDATE_UPDATE );
		m_pParent->EnableButtons( CWizardManager::enum_WizBut::WizButFinish );
		m_fUploadInCourse = false;
		m_nTimer = SetTimer( _TIMERID_DLGPANELTASCOPEUPDATE, 100, 0 );			// Update Version/progress Bar each 100ms
		return FALSE;
	}

	m_nTimer = SetTimer( _TIMERID_DLGPANELTASCOPEUPDATE, 100, 0 );			// Update Version/progress Bar each 100ms

	m_ProgressAll.ShowWindow( SW_SHOW );
	m_EditInfos.SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_INFO_UPLOADINGFILES ) );
	m_eUpdateZBHH = eProcessingZBUpdate::No;
	m_eUpdateZBDPS = eProcessingZBUpdate::No;
	m_fUploadHFT = false;
	m_fRefreshingHFTNeeded = true;  //After updating process it's necessary to refresh the HFT table

	// Start uploading...
	LV_ITEM lvi;
	CFileStatus fs;

	// Loop on all items stored into the FileTable.
	bool fSuccess = true;
	CFileTable *pSFT = m_pParent->GetHftTable();

	if( NULL != pSFT )
	{
		lvi.mask = LVIF_IMAGE;
		lvi.iSubItem = 0;
		int iItem = 0;
		bool bStatus = true;
		m_Progress.SetRange( 0, 100 );
		m_ProgressAll.SetRange( 0, 100 );
		m_dAlreadyUplploaded = 0;

		// Loop on all categories (Handheld app, DB, DpS app, others).
		for( int i = 0; i < m_ArCateg.GetCount() && true == m_fUploadInCourse; i++ )
		{
			m_iCurrentCategory = i;
			m_dAlreadyUploadedCateg = 0.0;

			// Update list control.
			lvi.iItem = i;
			lvi.iImage = eUpdateIcon::Incourse;
			m_FileList.SetItem( &lvi );

			// Update Progress.
			m_Progress.SetPos( 0 );

			// Process each file included into the category.
			CategoryItem Item = m_ArCateg.GetAt( i );

			// Latest Item Upgrading firmware.
			if( IDS_TASCOPE_PANELUPDATE_CATEGFIRMWAREUPDATE == Item.IDS )
			{
				break;
			}

			for( UINT j = 0; j < Item.arCount && true == fSuccess && true == m_fUploadInCourse; j++ )
			{
				pSFT->SetActivePosition( Item.arPos[j] );

				CString srcFn = pSFT->GetTarget();
				srcFn = srcFn.MakeLower();
				CString trgFn = srcFn;
				int iPos = trgFn.ReverseFind( '\\' );

				if( iPos > 0 )
				{
					trgFn.Delete( 0, iPos + 1 );
				}

				CString strSrcPath = CTAScopeUtil::CompleteTargetPath( srcFn );

				if( FALSE == CFile::GetStatus( strSrcPath, fs ) )							// check if the file exists and get the status.
				{
					continue;
				}

				// Specific processing for ZigBit files.
				bool fSecure = true;

				if( trgFn.Find( _T("handheld.srec") ) > -1 )
				{
					if( true == m_pParent->IsInBootMode() )
					{
						// If TA-Scope is in boot mode we have any way to retrieve the wireless brand (Atmel or Dresden) and version.
						// By default, in boot mode, we don't upload wireless firmware.
						continue;
					}

					if( CDlgWizTAScope::eHandheldWirelessAtmel != m_pParent->GetHHWirelessModuleType() )
					{
						// Do not take into account this file if the TA-Scope has an Dresden or unknown wireless version.
						continue;
					}

					// Verify that the current Hh software is able to manage new update.
					// If not report the upgrade after the Hh rebooting.
					if( pSFT->GetminAppVersion() <= m_pParent->GetVersionAsDouble( CDlgWizTAScope::eswvHH ) )
					{
						m_eUpdateZBHH = eProcessingZBUpdate::Requested;
						fSecure = false;
					}
					else
					{
						m_eUpdateZBHH = eProcessingZBUpdate::Reported;
					}


				}

				if( trgFn.Find( _T("hh_dresden.srec") ) > -1 )
				{
					if( true == m_pParent->IsInBootMode() )
					{
						// If TA-Scope is in boot mode we have any way to retrieve the wireless brand (Atmel or Dresden) and version.
						// By default, in boot mode, we don't upload wireless firmware.
						continue;
					}

					if( CDlgWizTAScope::eHandheldWirelessDresden != m_pParent->GetHHWirelessModuleType() )
					{
						// Do not take into account this file if the TA-Scope has an Atmel or unknown wireless version.
						continue;
					}

					// Verify that the current Hh software is able to manage new update.
					// If not report the upgrade after the Hh rebooting.
					if( pSFT->GetminAppVersion() <= m_pParent->GetVersionAsDouble( CDlgWizTAScope::eswvHH ) )
					{
						m_eUpdateZBHH = eProcessingZBUpdate::Requested;
						fSecure = false;
					}
					else
					{
						m_eUpdateZBHH = eProcessingZBUpdate::Reported;
					}
				}

				if( trgFn.Find( _T("sensorunit.srec") ) > -1 )
				{
					if( true == m_pParent->IsInBootMode() )
					{
						// If TA-Scope is in boot mode we have any way to retrieve the sensor information.
						// By default, in boot mode, we don't upload sensor firmware.
						continue;
					}

					// Verify that the current Hh software is able to manage new update.
					// If not report the upgrade after the Hh rebooting.
					if( pSFT->GetminAppVersion() <= m_pParent->GetVersionAsDouble( CDlgWizTAScope::eswvHH ) )
					{
						m_eUpdateZBDPS = eProcessingZBUpdate::Requested;
						fSecure = false;
					}
					else
					{
						m_eUpdateZBDPS = eProcessingZBUpdate::Reported;
					}

					if( true == m_pParent->IsInBootMode() )
					{
						continue;
					}
				}

				if( trgFn.Find( _T("dps2.dfu") ) > -1 )
				{
					// We don't upload this file in the TA-Scope. 
					// For DpS-Visio it is HySelct that apply the update and not the TA-Scope as it was 
					// the case before with the first DpS version.
					continue;
				}

				if( trgFn.Find( _T(".scb") ) > -1 )
				{
					fSecure = false;
				}

				// Service_F_ADD_CRC will modify the file !!! CRC added at the end of file.
				// Doing that the next integrity test will fail.
				// To escape to this copy, the file into the before adding CRC.
				CString strTempPath = CTAScopeUtil::GetTAScopeMaintenanceFolder() + _T("\\") + trgFn;
				FileMoveCopy( strSrcPath, strTempPath, false );

				if( true == fSecure )
				{
					ServiceManager::eComStatus ecsRet = ServiceManager::service_F_ADD_CRC( strTempPath );
				}

				if( false == m_pParent->WriteFile( strTempPath, fSecure, false ) )
				{
					fSuccess = false;
				}

				// Added to let the time for TA-SCOPE moving from dummy to target.
				m_pParent->WaitMilliSec( 500 );

				m_dAlreadyUplploaded += pSFT->GetSize();
				m_dAlreadyUploadedCateg += pSFT->GetSize();

				// Remove temporary file.
				RemoveFile( strTempPath );
			}

			CString str;

			if( true == fSuccess )
			{
				lvi.iImage = eUpdateIcon::Success;
				m_FileList.SetItem( &lvi );
				str = TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_DONE );
			}
			else
			{
				// TODO CDlgPanelTAScopeUpdate::OnBnClickedButtonupdate() what should be the reaction if the transfer fail?
				lvi.iImage = eUpdateIcon::Failed;
				m_FileList.SetItem( &lvi );
				bStatus =  false;
			}

			m_Progress.SetPos( 100 );
			m_Progress.Invalidate();	// Force progress bar refresh
			m_FileList.SetItemText( i, 1, ( LPCTSTR )str );
		}
	}

	if( true == fSuccess )
	{
		// Uploading HFT table into the handheld.
		m_fUploadHFT = true;
		CFileStatus status;

		if( TRUE == CFile::GetStatus( CTAScopeUtil::GetTAScopeLocHFTPath(), status ) )
		{
			// Local HFT should exist, already been verified into the CDlgWizTAScope::CheckForUpdate().
			if( false == m_pParent->WriteFile( CTAScopeUtil::GetTAScopeLocHFTPath(), false, false ) )
			{
				fSuccess = false;
			}
		}

		m_fUploadHFT = false;

		if( false == m_pParent->IsInBootMode() )
		{
			if( eProcessingZBUpdate::Reported == m_eUpdateZBDPS || eProcessingZBUpdate::Reported == m_eUpdateZBHH )
			{
				AfxMessageBox( IDS_TASCOPE_PANELUPDATE_UPDATINGWIRELESSDELAYED, MB_OK | MB_ICONEXCLAMATION );
			}

			if( eProcessingZBUpdate::Requested == m_eUpdateZBDPS || eProcessingZBUpdate::Requested == m_eUpdateZBHH )
			{
				m_pParent->DisableButtons( CWizardManager::WizButNext );
				CString str = TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_INCOURSE );
				m_FileList.SetItemText( m_iCurrentCategory, 1, ( LPCTSTR )str );
			}

			// Update Dps and ZB if needed.
			if( eProcessingZBUpdate::Requested == m_eUpdateZBDPS )
			{
				m_ProgressAll.ShowWindow( SW_HIDE );
				m_EditInfos.SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_INFO_UPDATINGDPS ) );

				// Ask for version refreshing.
				m_pParent->RequestSoftwareVersion();

				// Test if DPS is online TODO what about wireless connection.
				bool fOnline = ( m_pParent->GetpArCurrentVersion()->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvDPS ) != _T("-") ) ? true : false;
				bool fUserCancelOperation = false;

				while( false == fOnline )
				{
					if( IDCANCEL != AfxMessageBox( IDS_TASCOPE_PANELUPDATE_AFXDPSOFFLINE, MB_RETRYCANCEL | MB_ICONEXCLAMATION ) )
					{
						// Ask for version refreshing.
						m_pParent->RequestSoftwareVersion();
						fOnline = ( m_pParent->GetpArCurrentVersion()->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvDPS ) != _T("-") ) ? true : false;

						if( true == fOnline )
						{
							// Update Dps version field.
							CString str;
							FormatString( str, IDS_TASCOPE_PANELUPDATE_STATICDPSVERSION, m_pParent->GetpArCurrentVersion()->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvDPS ) );
							GetDlgItem( IDC_STATICDPS )->SetWindowText( str );
						}
					}
					else
					{
						fUserCancelOperation = true;
						break;
					}
				};

				m_Progress.SetPos( 0 );

				// Try to update except if the user canceled.
				if( false == fUserCancelOperation )
				{
					m_eUpdateZBDPS = eProcessingZBUpdate::InCourse;

					if( m_pParent->UpdateZBsw( CDlgWizTAScope::eSensor1Wireless ) )
					{
						m_eUpdateZBDPS = eProcessingZBUpdate::FinishOk;
					}
					else
					{
						// TODO CDlgPanelTAScopeUpdate::OnBnClickedButtonupdate() how to manage failure during transmission to the DPS?
						m_eUpdateZBDPS = eProcessingZBUpdate::FinishFail;
						/*
							if (AfxMessageBox(IDS_TASCOPE_PANELUPDATE_AFXDPSOFFLINE,MB_RETRYCANCEL|MB_ICONEXCLAMATION)==IDCANCEL)
								break;
						*/
					}
				}
			}

			if( eProcessingZBUpdate::Requested == m_eUpdateZBHH )
			{
				m_Progress.SetPos( 0 );
				m_eUpdateZBHH = eProcessingZBUpdate::InCourse;
				m_EditInfos.SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_INFO_UPDATINGWIRELESS ) );
				m_pParent->WaitMilliSec( 1000 );

				if( true == m_pParent->UpdateZBsw( m_pParent->GetHHWirelessModuleType() ) )
				{
					fSuccess = true;
					m_eUpdateZBHH = eProcessingZBUpdate::FinishOk;
				}
				else
				{
					fSuccess = false;
					m_eUpdateZBHH = eProcessingZBUpdate::FinishFail;
				}

				KillTimer( m_nTimer );
				m_nTimer = ( UINT_PTR )0;
				m_pParent->WaitMilliSec( 3000 );
			}

			CString str = TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_DONE );

			if( eProcessingZBUpdate::FinishFail == m_eUpdateZBHH || eProcessingZBUpdate::FinishFail == m_eUpdateZBDPS )
			{
				// TODO what should be the reaction if the transfer fail?
				str = TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_FAIL );
				lvi.iImage = eUpdateIcon::Failed;
				m_FileList.SetItem( &lvi );
			}
			else
			{
				lvi.iImage = eUpdateIcon::Success;
				m_FileList.SetItem( &lvi );
			}

			m_Progress.SetPos( 100 );
			m_Progress.Invalidate();	// Force progress bar refresh

			m_FileList.SetItemText( m_iCurrentCategory, 1, ( LPCTSTR )str );
		}

		// Clean all version fields.
		_CleanVersionField();

		// Stop refreshing until TASCOPE reboot.
		KillTimer( m_nTimer );
		m_nTimer = ( UINT_PTR )0;
		m_pParent->WaitMilliSec( 500 );

		// Ask for rebooting only if we are not in boot mode (this mode doesn't accept the command!).
		if( false == m_pParent->IsInBootMode() )
		{
			m_EditInfos.SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_INFO_REBOOTING ) );
			m_pParent->RebootDevice( CDlgWizTAScope::eSoftwareVersion::eswvHH );
			m_pParent->WaitMilliSec( 10000 );
		}
	}

	if( true == fSuccess )
	{
		m_EditInfos.SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_INFO_UPDATINGSUCCESS ) );
	}
	else if( true == m_fUploadInCourse )
	{
		m_EditInfos.SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_INFO_UPDATINGFAIL ) );
	}
	else
	{
		m_EditInfos.SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_INFO_UPDATINGABORTED ) );
	}

	m_fUploadInCourse = false;

	m_pParent->EnableButtons( CWizardManager::enum_WizBut::WizButNext );
	m_pParent->ShowButtons( CWizardManager::enum_WizBut::WizButNext, SW_HIDE );
	m_pParent->SetButtonText( CWizardManager::enum_WizBut::WizButNext, IDS_TASCOPE_PANELUPDATE_UPDATE );

	m_pParent->ActivateRefresh( true );
	EndWaitCursor();
	m_Progress.SetPos( 0 );
	m_ProgressAll.SetPos( 0 );

	if( eProcessingZBUpdate::Reported == m_eUpdateZBDPS || eProcessingZBUpdate::Reported == m_eUpdateZBHH )
	{
		OnAfterActivate();
	}
	else
	{
		m_pParent->EnableButtons( CWizardManager::enum_WizBut::WizButFinish );

		if( true == m_pParent->IsInBootMode() )
		{
			AfxMessageBox( IDS_TASCOPE_PANELUPDATE_MANUALLYREBOOT, MB_OK | MB_ICONINFORMATION, 0 );
		}
		else if( true == m_pParent->IsNewDpSVisioFirmware() )
		{
			AfxMessageBox( IDS_TASCOPE_PANELUPDATE_NEWDPSVISIOFIRMWARE, MB_OK | MB_ICONINFORMATION, 0 );
		}
	}

	return FALSE;
}

BOOL CDlgPanelTAScopeUpdate::OnWizFinish()
{
	KillTimer( m_nTimer );
	m_nTimer = ( UINT_PTR )0;
	// Force software version.
	m_pParent->ClearSoftwareVersion();
	m_pParent->JumpToPanel( CDlgWizTAScope::epanStart );
	return FALSE;
}

bool CDlgPanelTAScopeUpdate::OnActivate()
{
	return TRUE;
}

bool CDlgPanelTAScopeUpdate::OnAfterActivate()
{
	// 'Next' and 'Finish' buttons.
	m_pParent->ShowButtons( CWizardManager::WizButBack, false );
	m_pParent->SetButtonText( CWizardManager::WizButNext, IDS_TASCOPE_PANELUPDATE_UPDATE );

	m_fRefreshingHFTNeeded = false;
	InitFileList();
	m_pParent->ShowButtons( CWizardManager::WizButNext, true );
	_CleanVersionField();

	m_nTimer = SetTimer( _TIMERID_DLGPANELTASCOPEUPDATE, 100, 0 );			// Update Version/progress Bar each 100ms
	return true;
}

bool CDlgPanelTAScopeUpdate::InitFileList()
{
	int nIndex;
	LV_ITEM lvi;
	m_FileList.DeleteAllItems();

	// Add the first line in the action list.
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvi.pszText = _T( "" );
	lvi.iImage = eUpdateIcon::Wait;
	lvi.stateMask = LVIS_STATEIMAGEMASK;
	lvi.state = INDEXTOSTATEIMAGEMASK( 0 );
	lvi.iSubItem = 0;
	m_dTotalSize = 0;
	m_ArCateg.RemoveAll();
	m_EditInfos.SetWindowText( _T( "" ) );
	bool fZBShouldbeUpdated = false;

	// Loop on all items stored into the FileTable.
	CFileTable *pSFT = m_pParent->GetHftTable();

	if( NULL != pSFT )
	{
		for( bool bIsEnd = pSFT->GetFirst(); false == bIsEnd; bIsEnd = pSFT->GetNext() )  //get the list in SFT.
		{
			CString str = pSFT->GetTarget();
			str.MakeLower();
			int pos = str.ReverseFind( '\\' );

			if( pos > 0 )
			{
				str.Delete( 0, pos + 1 );
			}

			// Fill category list.
			// Handheld Software
			if( _T("app.bin" ) == str || _T( "handheld.srec" ) == str || _T( "hh_dresden.srec") == str )
			{
				if( _T("handheld.srec") == str || _T("hh_dresden.srec") == str )
				{
					if( true == m_pParent->IsInBootMode() )
					{
						// If TA-Scope is in boot mode we have any way to retrieve the wireless brand (Atmel or Dresden) and version.
						// By default, in boot mode, we don't upload wireless firmware.
						continue;
					}

					if( CDlgWizTAScope::eWirelessModuleUnknown == m_pParent->GetHHWirelessModuleType() )
					{
						// Don't know the wireless module type.
						continue;
					}

					// First check if handheld wireless firmware corresponds to the TA-Scope wireless module.
					if( ( _T("handheld.srec") == str && CDlgWizTAScope::eHandheldWirelessAtmel != m_pParent->GetHHWirelessModuleType() ) ||
						( _T("hh_dresden.srec") == str && CDlgWizTAScope::eHandheldWirelessDresden != m_pParent->GetHHWirelessModuleType() ) )
					{
						continue;
					}

					AddCategoryItem( IDS_TASCOPE_PANELUPDATE_CATEGHHAPP, pSFT );

					if( false == m_pParent->IsInBootMode() )
					{
						fZBShouldbeUpdated = true;
					}

					CString strVersion = _T("");
					double dWirelessVersion = pSFT->GetVersion();

					if( _T("hh_dresden.srec") == str )
					{
						dWirelessVersion -= 800;
					}

					FormatString( strVersion, IDS_TASCOPE_PANELUPDATE_STATICZBHHVERSION, pSFT->VersionDoubleToString( dWirelessVersion, FILETABLE_VERSIONDIGITNBR_HH_WIRELESS ) );

					if( _T("handheld.srec") == str )
					{
						strVersion += _T("-AT");
					}
					else
					{
						strVersion += _T("-DR");
					}

					GetDlgItem( IDC_STATICSFTZBHH )->SetWindowText( strVersion );
				}
				else
				{
					AddCategoryItem( IDS_TASCOPE_PANELUPDATE_CATEGHHAPP, pSFT );

					CString strVersion;
					FormatString( strVersion, IDS_TASCOPE_PANELUPDATE_STATICHHVERSION, pSFT->VersionDoubleToString( pSFT->GetVersion(), FILETABLE_VERSIONDIGITNBR_HH_APP ) );
					GetDlgItem( IDC_STATICSFTHH )->SetWindowText( strVersion );
				}

			}
			else if( _T("sensorunit.srec") == str )
			{
				if( true == m_pParent->IsInBootMode() )
				{
					// If TA-Scope is in boot mode we have any way to retrieve the wireless brand (Atmel or Dresden) and version.
					// By default, in boot mode, we don't upload wireless firmware.
					continue;
				}

				// DPS software.
				AddCategoryItem( IDS_TASCOPE_PANELUPDATE_CATEGDPSAPP, pSFT );

				if( false == m_pParent->IsInBootMode() )
				{
					fZBShouldbeUpdated = true;
				}

				CString strVersion;
				FormatString( strVersion, IDS_TASCOPE_PANELUPDATE_STATICDPSVERSION, pSFT->VersionDoubleToString( pSFT->GetVersion(), FILETABLE_VERSIONDIGITNBR_DPS_APP ) );
				GetDlgItem( IDC_STATICSFTDPS )->SetWindowText( strVersion );
			}
			else if( _T("tadb.xdb" ) == str || _T( "tadbstx.xml") == str )
			{
				// Database.
				AddCategoryItem( IDS_TASCOPE_PANELUPDATE_CATEGDB, pSFT );

				if( _T("tadb.xdb") == str )
				{
					CString strVersion;
					FormatString( strVersion, IDS_TASCOPE_PANELUPDATE_STATICDBVERSION, pSFT->VersionDoubleToString( pSFT->GetVersion(), FILETABLE_VERSIONDIGITNBR_HH_DB ) );
					strVersion = strVersion + CString( _T("-") ) + TASApp.GetTADBKey();
					GetDlgItem( IDC_STATICSFTDB )->SetWindowText( strVersion );
				}
			}
			else if( _T("dps2.dfu") == str )
			{
				// Do nothing with this file.
			}
			else
			{
				// Others.
				AddCategoryItem( IDS_TASCOPE_PANELUPDATE_CATEGOTHER, pSFT );
			}
		}

		// Added at the end.
		if( true == fZBShouldbeUpdated )
		{
			AddCategoryItem( IDS_TASCOPE_PANELUPDATE_CATEGFIRMWAREUPDATE, pSFT );
		}

		// Ordering Category list
		/*
			0 IDS_TASCOPE_PANELUPDATE_CATEGHHAPP
			1 IDS_TASCOPE_PANELUPDATE_CATEGDB
			2 IDS_TASCOPE_PANELUPDATE_CATEGDPSAPP
			3 IDS_TASCOPE_PANELUPDATE_CATEGOTHER
			4 IDS_TASCOPE_PANELUPDATE_CATEGFIRMWAREUPDATE
		*/
		int OrderList[] = { IDS_TASCOPE_PANELUPDATE_CATEGHHAPP,
							IDS_TASCOPE_PANELUPDATE_CATEGDB,
							IDS_TASCOPE_PANELUPDATE_CATEGDPSAPP,
							IDS_TASCOPE_PANELUPDATE_CATEGOTHER,
							IDS_TASCOPE_PANELUPDATE_CATEGFIRMWAREUPDATE,
							-1
						  };
		CArray<CategoryItem> tempAr;

		for( int j = 0; OrderList[j] > 0; j++ )
		{
			for( int i = 0; i < m_ArCateg.GetCount(); i++ )
			{
				if( m_ArCateg[i].IDS == OrderList[j] )
				{
					tempAr.Add( m_ArCateg[i] );
					break;
				}
			}
		}

		m_ArCateg.RemoveAll();
		m_ArCateg.Copy( tempAr );

		// Fill the category list, proceed after all file uploading.
		for( int i = 0; i < m_ArCateg.GetCount(); i++ )
		{
			CategoryItem Item = m_ArCateg.GetAt( i );
			lvi.iItem = i;
			CString str = TASApp.LoadLocalizedString( Item.IDS );
			lvi.pszText = ( LPWSTR )( LPCTSTR )str;
			nIndex = m_FileList.InsertItem( &lvi );
			str = TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPDATE_WAITING );
			m_FileList.SetItemText( nIndex, 1, ( LPCTSTR )str );
			m_dTotalSize += Item.TotalFileSize;
		}
	}

	return true;
}

void CDlgPanelTAScopeUpdate::AddCategoryItem( UINT uiIDS, CFileTable *pSFT )
{
	bool fFound = false;

	// Verify category ID existence.
	int i = 0;
	int iFoundPos = 0;

	for( ; i < m_ArCateg.GetCount() && false == fFound; i++ )
	{
		if( m_ArCateg.GetAt( i ).IDS == uiIDS )
		{
			fFound = true;
			iFoundPos = i;
		}
	}

	if( true == fFound )
	{
		CategoryItem &Item = m_ArCateg.ElementAt( iFoundPos );

		// Category already exist add the file.
		Item = m_ArCateg.GetAt( iFoundPos );

		if( NULL != pSFT )
		{
			Item.arPos[Item.arCount] = pSFT->GetActivePosition();
			Item.TotalFileSize += pSFT->GetSize();
		}
		else
		{
			Item.arPos[Item.arCount] = NULL;
		}

		Item.arCount++;
	}
	else
	{
		// Not found add it.
		CategoryItem Item;
		Item.IDS = uiIDS;
		Item.arCount = 1;

		if( NULL != pSFT )
		{
			Item.arPos[0] = pSFT->GetActivePosition();
			Item.TotalFileSize = pSFT->GetSize();
		}
		else
		{
			Item.arPos[0] = NULL;
			Item.TotalFileSize = 0;
		}

		m_ArCateg.Add( Item );
	}
}

void CDlgPanelTAScopeUpdate::_CleanVersionField()
{
	CString str, str1;
	m_pParent->ClearSoftwareVersion();
	FormatString( str, IDS_TASCOPE_PANELUPDATE_STATICHHVERSION, _T( "" ) );
	GetDlgItem( IDC_STATICHH )->SetWindowText( str );
	FormatString( str, IDS_TASCOPE_PANELUPDATE_STATICDBVERSION, _T( "" ) );
	GetDlgItem( IDC_STATICDB )->SetWindowText( str );
	FormatString( str, IDS_TASCOPE_PANELUPDATE_STATICZBHHVERSION, _T( "" ) );
	GetDlgItem( IDC_STATICZBHH )->SetWindowText( str );
	str1 = CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_NOCONNECTED );
	FormatString( str, IDS_TASCOPE_PANELUPDATE_STATICDPSVERSION, str1 );
	GetDlgItem( IDC_STATICDPS )->SetWindowText( str );
}
