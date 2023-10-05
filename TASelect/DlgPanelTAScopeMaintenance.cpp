#include "stdafx.h"
#include "TASelect.h"
#include "DlgTreeList.h"
#include "DlgWizTAScope.h"
#include "DlgPanelTAScopeMaintenance.h"

CDlgPanelTAScopeMaintenance::CDlgPanelTAScopeMaintenance( CWnd* pParent )
	: CDlgWizard( CDlgPanelTAScopeMaintenance::IDD, pParent )
{
	m_pParent = NULL;
	m_nTimer = (UINT_PTR)0;
}

BEGIN_MESSAGE_MAP( CDlgPanelTAScopeMaintenance, CDlgWizard )
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED( IDC_BUTTONCLEANLOCALFILES, OnBnCleanLocalFiles )
	ON_BN_CLICKED( IDC_BUTTONCLEANTASCOPEFILES, OnBnCleanTAScopeFiles )
	ON_BN_CLICKED( IDC_BUTTONFORCEUPDATE, OnBnForceUpdate )
END_MESSAGE_MAP()

void CDlgPanelTAScopeMaintenance::DoDataExchange( CDataExchange* pDX )
{
	CDlgWizard::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICVERSION, m_StaticVersionNbr );
	DDX_Control( pDX, IDC_BUTTONCLEANLOCALFILES, m_ButtonDeleteLocFiles );
	DDX_Control( pDX, IDC_BUTTONCLEANTASCOPEFILES, m_ButtonDeleteTASCOPEFiles );
	DDX_Control( pDX, IDC_BUTTONFORCEUPDATE, m_ButtonForceUpdate );
}

BOOL CDlgPanelTAScopeMaintenance::OnInitDialog()
{
	CDlgWizard::OnInitDialog();
	m_pParent = (CDlgWizTAScope *)GetWizMan();

	// Set the white rectangle to the first drawn window.
	GetDlgItem( IDC_STATICRECTANGLE )->BringWindowToTop();
	m_brWhiteBrush.CreateSolidBrush( RGB( 255, 255, 255 ) );

	// Strings.
	GetDlgItem( IDC_EDITTITLE )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELMAINTENANCE_TITLE ) );
	GetDlgItem( IDC_STATICDETECTEDTASCOPE )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_VERSIONDETECTED ) );
	

	GetDlgItem( IDC_STATICINFOBTNCLEANLOCFILES )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_EDITINFOBTNCLEANLOCFILES ) );
	GetDlgItem( IDC_STATICINFOBTNCLEANTASCOPEFILES )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_EDITINFOBTNCLEANTASCOPEFILES ) );
	GetDlgItem( IDC_STATICINFOBTNFORCEFULLUPDATE )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_EDITINFOBTNFORCEFULLUPDATE ) );

	// 'Next' and 'Finish' buttons.
	m_pParent->ShowButtons( CWizardManager::WizButNext | CWizardManager::WizButBack, false );

	m_ButtonDeleteLocFiles.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGPANELMAINTENANCE_BUTTONCLEANLOCALFILES ) );
	m_ButtonDeleteTASCOPEFiles.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGPANELMAINTENANCE_BUTTONCLEANTASCOPEFILES ) );
	m_ButtonForceUpdate.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGPANELMAINTENANCE_BUTTONFORCEUPDATE ) );

	m_ButtonDeleteLocFiles.SetImage( IDB_COMM_TASCOPE_DELETE_BACKUP_48, IDB_COMM_TASCOPE_DELETE_BACKUP_48 );
	m_ButtonForceUpdate.SetImage( IDB_COMM_TASCOPE_RELOAD_48, IDB_COMM_TASCOPE_RELOAD_48 );
	m_ButtonDeleteTASCOPEFiles.SetImage( IDB_COMM_TASCOPE_CLEAN_48, IDB_COMM_TASCOPE_CLEAN_48 );

	m_ButtonDeleteTASCOPEFiles.EnableWindow( FALSE );
	m_ButtonForceUpdate.EnableWindow( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgPanelTAScopeMaintenance::OnTimer( UINT_PTR nIDEvent )
{
	CDlgWizard::OnTimer( nIDEvent );
	if( (UINT_PTR)0 == m_nTimer || m_nTimer != nIDEvent )
		return;

	CString str, str1;
	m_StaticVersionNbr.GetWindowText( str );
	if( str != m_pParent->GetpArCurrentVersion()->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvHH ) )
	{
		m_StaticVersionNbr.SetWindowText( m_pParent->GetpArCurrentVersion()->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvHH ) );
		m_pParent->ActivateRefresh( false );
		if( true == m_pParent->IsTAScopeAvailable() )
		{
			m_ButtonDeleteTASCOPEFiles.EnableWindow( TRUE );
			m_ButtonForceUpdate.EnableWindow( TRUE );
		}
		else
		{
			m_ButtonDeleteTASCOPEFiles.EnableWindow( FALSE );
			m_ButtonForceUpdate.EnableWindow( FALSE );
		}
		m_pParent->ActivateRefresh( true );
	}
}

HBRUSH CDlgPanelTAScopeMaintenance::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	HBRUSH hbr;

	if( pWnd->m_hWnd == GetDlgItem(IDC_EDITTITLE)->m_hWnd )
	{
		pDC->SetTextColor( RGB( 0, 0, 0 ) );
		pDC->SetBkColor( RGB( 255, 255, 255 ) );
		return (HBRUSH)m_brWhiteBrush;
	}
	
	hbr = CDlgWizard::OnCtlColor( pDC, pWnd, nCtlColor );
	return hbr;
}

void CDlgPanelTAScopeMaintenance::OnBnCleanLocalFiles()
{
	CString str = TASApp.LoadLocalizedString( IDS_DLGPANELMAINTENANCE_WARNINGCLEANLOCFILES );
	if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION ) )
	{
		// Delete content of TASCope data folder.
		DeleteFilesAndFolder( CTAScopeUtil::GetTAScopeDataFolder(), _T("*.*") );
		// Delete content of TASCope backup folder.
		DeleteFilesAndFolder( CTAScopeUtil::GetTAScopeBackupFolder(), _T("*.*") );
		// Delete content of TASCope maintenance folder.
		DeleteFilesAndFolder( CTAScopeUtil::GetTAScopeMaintenanceFolder(), _T("*.*") );
	}
}

void CDlgPanelTAScopeMaintenance::OnBnCleanTAScopeFiles()
{
	try
	{
		m_pParent->ActivateRefresh( false );
		m_pParent->WaitMilliSec( 200 );
		m_MetaData.Init();

		// Prepare and Load Metadata file .
		m_pParent->GetMetaData();

		CString fn = CTAScopeUtil::GetTAScopeMaintenanceFolder() + _T("\\") + METAFILE;
		CFileStatus fs;
		
		// Check if the file exists and get the status.
		if( FALSE == CFile::GetStatus( fn, fs ) )
		{
			return;
		}
	
		std::ifstream inpf( fn, std::ifstream::in | std::ifstream::binary );
		m_MetaData.Read( inpf );
		
		CDlgTreeList dlg;

		for( IDPTR IDPtr = m_MetaData.Access().GetFirst(); NULL != IDPtr.MP; IDPtr = m_MetaData.Access().GetNext() )
		{
			CDS_PlantX *pPlant = dynamic_cast<CDS_PlantX *>( IDPtr.MP );

			if( NULL != pPlant )
			{
				// Skip tadsx.tsc.
				CString fn = (CString)CStringA( pPlant->GetTSCFilename() );
				
				if( fn.Find( TADSXFILE ) > -1 )
				{
					continue;
				}
			
				CString str = pPlant->GetString( CDS_PlantX::Name );
				dlg.Add( str, fn );
			}	
		}
		
		dlg.Display( IDS_DLGPANELMAINTENANCE_WARNINGCLEANTASCOPEFILE );
		BeginWaitCursor();
		CArray<CString> strAr;
		CString ext = _T(".tsc");
		
		for( int i = 0; i < dlg.GetpStrDataArray()->GetCount(); i++ )
		{
			CString fn = dlg.GetpStrDataArray()->GetAt( i ).GetData();
			strAr.Add( fn );
			
			// Remove tsc associated bak file.
			int n = fn.Find( ext );
			
			if( n > 0 )
			{
				fn.Delete( n, ext.GetLength() );
				fn += _T(".bak");
				strAr.Add( fn );
			}
		}
		
		if (strAr.GetCount() > 0)
		{
			m_pParent->DeleteTASCOPEFile( &strAr );
			m_pParent->WaitMilliSec( 200 );
		}
		
		EndWaitCursor();
		
		// Are you sure you want to remove all loggings and quick measurements stored into the handheld?
		CString str = TASApp.LoadLocalizedString( IDS_DLGPANELMAINTENANCE_WARNINGCLEANQMANDLOG );
		
		if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION ) )
		{
			m_pParent->DeleteAllQMandLOG();
		}
		
		m_pParent->ActivateRefresh( true );
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgPanelTAScopeMaintenance::OnBnCleanTAScopeFiles'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgPanelTAScopeMaintenance::OnBnForceUpdate()
{
	// Force an update, check for update will simulate a eupdTASCOPEHftMissing only if the current HFT is present.
	CDlgWizTAScope::eTAScopeUpdate RetCode = m_pParent->CheckForUpdate( true );
	if( CDlgWizTAScope::eTAScopeUpdate::eupdTASCOPEHftMissing == RetCode )
	{
		m_pParent->JumpToPanel( CDlgWizTAScope::epanUpdate );
	}
	
	if( CDlgWizTAScope::eTAScopeUpdate::eupdLocalHftMissing == RetCode )
	{
		::AfxMessageBox( TASApp.LoadLocalizedString( IDS_DLGPANELMAINTENANCE_WARNINGMISSINGLOCALHFT ), MB_OK | MB_ICONEXCLAMATION );
	}
}

BOOL CDlgPanelTAScopeMaintenance::OnWizCancel()
{
	KillTimer( m_nTimer );
	m_nTimer = (UINT_PTR)0;
	return TRUE;
}

BOOL CDlgPanelTAScopeMaintenance::OnWizFinish()
{
	KillTimer( m_nTimer );
	m_nTimer = (UINT_PTR)0;
	m_pParent->JumpToPanel( CDlgWizTAScope::epanStart );
	return FALSE;
}

bool CDlgPanelTAScopeMaintenance::OnAfterActivate()
{
	// 'Next' and 'Finish' buttons.
	m_pParent->ShowButtons( CWizardManager::WizButNext | CWizardManager::WizButBack, false );
	m_nTimer = SetTimer( _TIMERID_DLGPANELTASCOPEMAINTENANCE, 1000, 0 );					// Retry after 5sec to verify connection
	return true;
}

int CDlgPanelTAScopeMaintenance::OnWizButFinishTextID()
{
	return IDS_DLGPANELMAINTENANCE_CANCEL;
}
