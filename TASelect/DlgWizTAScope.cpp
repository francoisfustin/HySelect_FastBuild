#include "stdafx.h"
#include "TASelect.h"
#include "utilities.h"
#include "FileTableMng.h"
#include "crc32.h"
#include "DlgWizTAScope.h"

int CDlgWizTAScope::m_iComIndex = 0;

CDlgWizTAScope::CDlgWizTAScope( CWnd* pParent )
	: CWizardManager( IDS_WIZTASCOPE_TITLE, CDlgWizTAScope::IDD )
{
	m_pDlgProgressBar = NULL;
	m_fComPortBusy = false;
	m_fRefreshVersion = true;
	m_fInterruptRequested = false;
	m_iComNumber = 0;
	m_fBootMode = false;
	m_pCom = NULL;
	m_arData.RemoveAll();
	m_dProgress = 0.0;
	m_iComIndex = 0;
	m_fTAScopeOffLineForced = false;
	m_CurrentVersion.RemoveAll();
	m_uiDSversion = 0;
	m_DpsComStatus = ServiceManager::eDPSComStatus::eDPcsNotConnected;
	m_nTimer = (UINT_PTR)0;
	m_pSFT = NULL; // TA-SCOPE file table into the PC
	m_pHFT = NULL; // TA-SCOPE file table into the TASCOPE
	m_pFT  = NULL; // file table with new files that should be transfered into the TA-SCOPE
	m_eHHWirelessModuleType = WirelessModuleType::eWirelessModuleUnknown;
	m_bNewDpSVisioFirmware = false;
	ClearSoftwareVersion();
	Add( &m_DlgPanelStart, epanStart );
	Add( &m_DlgPanelDownload, epanDownLoad );
	Add( &m_DlgPanelUpload, epanUpLoad );
	Add( &m_DlgPanelMaintenance, epanMaintenance );
	Add( &m_DlgPanelUpdate, epanUpdate );
}

CDlgWizTAScope::~CDlgWizTAScope()
{
	CleanHFTs();
}

CFileTable *CDlgWizTAScope::GetHftTable()
{
	if( NULL != m_pFT && m_pFT->GetCount() > 0 )
	{
		return m_pFT;
	}
	
	if( NULL != m_pSFT && m_pSFT->GetCount() > 0 )
	{
		return m_pSFT;
	}
	
	return NULL;
}

void CDlgWizTAScope::CreateProgressBar( ProgressBarType eProgressBarType )
{
	DeleteProgressBar();

	m_eProgressBarType = eProgressBarType;
	int iIDS = ( PBT_Download == m_eProgressBarType ) ? IDS_WIZTASCOPE_TITLEPROGRESSBARDOWNLOAD : IDS_WIZTASCOPE_TITLEPROGRESSBARUPLOAD;

	CString str = TASApp.LoadLocalizedString( iIDS );
	m_pDlgProgressBar = new CDlgProgressBar( this, str );
	m_dProgress = 0.0;
	m_nTimer = SetTimer( _TIMERID_DLGWIZTASCOPE, 100, 0 );					// Update progress Bar each 100ms
	m_pDlgProgressBar->Display();
}

void CDlgWizTAScope::DeleteProgressBar()
{
	KillTimer( m_nTimer  );
	m_nTimer = (UINT_PTR)0;
	
	if( NULL != m_pDlgProgressBar )
	{
		delete m_pDlgProgressBar;
	}

	m_dProgress = 0.0;
	m_pDlgProgressBar = NULL;
}

int CDlgWizTAScope::FoundTAScopePortCom()
{
	if( NULL == this->GetSafeHwnd() )
	{
		return false;
	}
	
	// Try to find the port where is connected the CBI.
	int i, iMin, iMax;
	
	if( m_iComIndex > MAXCOMNUMBER )
	{
		m_iComIndex = 1;
	}
	
	iMin = m_iComIndex;
	m_iComIndex += 32;
	iMax = m_iComIndex;//MAXCOMNUMBER;
	ServiceManager::eComStatus eFound = ServiceManager::eComStatus::ecsUndef;
	m_iComNumber = 0;

	CStringArray ComPorts;

	CSerialPort::EnumerateComPortUsingRegistry( ComPorts );

	for( int j = 0; j < ComPorts.GetSize() && eFound != ServiceManager::eComStatus::ecsOKFinish; j++ )
	{
		CString str = ComPorts.GetAt( j );
		TRACE( _T("\r\n%s"), (LPCTSTR) str );
		str.Delete( 0, 3 );		// Remove "COM"
		i = _wtoi( (LPCTSTR)str );
		
		bool fRet = PortOpen( i );

		if( true == fRet )
		{
			eFound = ServiceManager::service_GET_TIME( *m_pCom , &m_arData );
		
			if( ServiceManager::eComStatus::ecsOKFinish == eFound )
			{
				m_iComNumber = i;
			}
			else
			{	
				ErrorCode errCode = ServiceManager::service_GET_LASTERRORCODE();				
				
				if( ErrorCode::UNSUPPORTED == errCode || ErrorCode::UNSUPPORTED_BOOT == errCode )
				{
					m_iComNumber = i;
					m_fBootMode = true;
					// Force out.
					eFound = ServiceManager::eComStatus::ecsOKFinish;
				}
			}

			PortClose();
		}

		WaitMilliSec( 100 );
	}

	return ( eFound == ServiceManager::eComStatus::ecsOKFinish );
}

bool CDlgWizTAScope::IsTAScopeAvailable()
{
	if( true == IsComPortBuzy() )
	{
		return false;
	}

	if( m_iComNumber > 0 )
	{
		bool fRet;
		fRet = PortOpen( m_iComNumber );
		m_fBootMode = false;

		if( true == fRet )
		{
			ServiceManager::eComStatus bFound = ServiceManager::service_RUN_GOTOMAINMENU( *m_pCom );

			if( ServiceManager::eComStatus::ecsOKFinish == bFound )
			{
				bFound = ServiceManager::service_GET_TIME( *m_pCom , &m_arData );
			}
			
			PortClose();
				
			if( ServiceManager::eComStatus::ecsOKFinish != bFound ) 
			{
				ErrorCode errCode = ServiceManager::service_GET_LASTERRORCODE();				
				
				if( ErrorCode::UNSUPPORTED == errCode || ErrorCode::UNSUPPORTED_BOOT == errCode ) 
				{
					m_fBootMode = true;
					return true;
				}
				
				m_iComNumber = 0;
				return false;
			}
			else
			{
				return true;
			}
		}
	}

	PortClose();
	return false;
}

double CDlgWizTAScope::GetVersionAsDouble( eSoftwareVersion esvIndex )
{
	CString strVer = GetpArCurrentVersion()->GetAt( esvIndex );
	double dVer = m_pFT->VersionStringToDouble( strVer );
	return dVer;
}

void CDlgWizTAScope::ClearSoftwareVersion()
{
	m_CurrentVersion.SetSize( eswLast );
	CString str( _T("") );

	for( int i = 0; i < eswLast; i++ )
	{
		m_CurrentVersion.SetAt( (eSoftwareVersion)i, str );
	}
}

// TODO how to return a value that could be checked with HFT
bool CDlgWizTAScope::RequestSoftwareVersion()
{
	if( false == IsTAScopeAvailable() )	// Initialized ?
	{
		return false;
	}
	
	if( false == PortOpen() )
	{
		return false;
	}
	
	ServiceManager::eComStatus ecsRet = ServiceManager::service_GET_SOFTWARE_VERSIONS( *m_pCom, &m_arData );

	ClearSoftwareVersion();

	if( ServiceManager::ecsOKFinish != ecsRet )
	{
		PortClose();
		return false;
	}
	
	CString str;
	str.Format( _T("%d.%d.%d.%d"), m_arData[3], m_arData[2], m_arData[1], m_arData[0] );
	m_CurrentVersion.SetAt( eswvHH, str );
	
	if( m_arData[4] > 0 )
	{
		// When the bit 7 is set to 1 TA-Scope has a Dresden radio. Otherwise it's an old TA-Scope with Atmel radio.
		// In the case of the Dresden radio, we remove the bit 7.
		m_eHHWirelessModuleType = ( 0x80 == ( m_arData[4] & 0x80 ) ) ? WirelessModuleType::eHandheldWirelessDresden : WirelessModuleType::eHandheldWirelessAtmel;

		// Save wireless version without modification to facilitate working on the HFT.
		str.Format( _T("%d.%d"), m_arData[4] >> 4, m_arData[4] & 0x0F );
		m_CurrentVersion.SetAt( eswvWireless, str );

		if( WirelessModuleType::eHandheldWirelessDresden == m_eHHWirelessModuleType )
		{
			// Save wireless version without bit 7 to facilitate display.
			str.Format( _T("%d.%d"), ( m_arData[4] & 0x70 ) >> 4, m_arData[4] & 0x0F );
			m_CurrentVersion.SetAt( eswvWirelessDresden, str );
		}
	}
	else
	{
		m_eHHWirelessModuleType = WirelessModuleType::eWirelessModuleUnknown;
		m_CurrentVersion.SetAt( eswvWireless, _T("-") );
		m_CurrentVersion.SetAt( eswvWirelessDresden, _T("-") );
	}
	
	str = _T("-");
	
	if( m_arData[8] )
	{
		str.Format( _T("%d.%d.0"), m_arData[8] >> 4, m_arData[8] & 0x0F );
	}

	m_CurrentVersion.SetAt( eswvDPS, str );
	
	// Sanity
	m_uiDSversion = 0;

	if( m_arData.GetCount() >= 15 )
	{
		str.Format( _T("%d.%d.%d.%d" ), m_arData[15], m_arData[14], m_arData[13], m_arData[12] );
		m_CurrentVersion.SetAt( eswvDB, str );
		m_arData[m_arData.GetCount() - 1] = 0;
		const int iStartDBKeyVersion = 16;
		
		if( m_arData.GetCount() > iStartDBKeyVersion )
		{
			str.Format( _T("%s"), (TCHAR *)&m_arData[16] );
			m_CurrentVersion.SetAt( eswvDBKey, str );
			const int iStartDSVersion = 26;
			
			if( m_arData.GetCount() > iStartDSVersion )
			{
				for( int i = iStartDSVersion + 3; i >= iStartDSVersion; i-- )
				{
					m_uiDSversion <<= 8;
					m_uiDSversion += m_arData[i];
				}
			}
		}
	}
	else
	{
		str = _T("-");
		m_CurrentVersion.SetAt( eswvDB, str );
		m_CurrentVersion.SetAt( eswvDBKey, str );
	}
	
	// Ask for DPS connection status.
	ecsRet = ServiceManager::service_GET_COMSTATUS( *m_pCom, &m_arData );
	PortClose();
	
	if( ServiceManager::ecsError == ecsRet )
	{
		ErrorCode er = ServiceManager::service_GET_LASTERRORCODE();
		m_DpsComStatus = ServiceManager::eDPSComStatus::eDPcsNotServiceNotSupported;
	}
	else
	{
		m_DpsComStatus = (ServiceManager::eDPSComStatus)m_arData[0];
	}
	
	// DPS is not connected until it's not done by a cable.
	if( m_DpsComStatus != ServiceManager::eDPSComStatus::eDPcsWired )
	{
		str = _T("-");
		m_CurrentVersion.SetAt( eswvDPS, str );
	}

	return true;
}

bool CDlgWizTAScope::GetMetaData()
{
	if( false == IsTAScopeAvailable() )	// Initialized ?
	{
		return false;
	}

	if( false == PortOpen() )
	{
		return false;
	}
	
	// Ask to generate MetaData.
	ServiceManager::eComStatus ecsRet = ServiceManager::service_RUN_GENERATE_METADATA( *m_pCom );
	PortClose();
	
	if( ServiceManager::ecsOKFinish != ecsRet )
	{
		return false;
	}

	// Remove 'old' metadata file from Maintenance folder.
	CString strfileName = CTAScopeUtil::GetTAScopeMaintenanceFolder() + _T("\\") + METAFILE;
	RemoveFile( strfileName );
	
	// Download MetaData File.
	bool fRet = ReadFile( CString( METAFILE ), strfileName );
	return fRet;
}

bool CDlgWizTAScope::GetTADSFile()
{
	CString strFileName = CTAScopeUtil::GetTAScopeDataFolder() + _T("\\") + TADSXFILE;
	RemoveFile( strFileName );
	
	// Download Tadsx file.
	bool fRet = ReadFile( CString( TADSXFILE ), strFileName, false );
	return fRet;
}

bool CDlgWizTAScope::RebootDevice( eSoftwareVersion eDevice )
{
	if( false == IsTAScopeAvailable() )	// Initialized ?
	{
		return false;
	}

	if( false == PortOpen() )
	{
		return false;
	}
	
	ServiceManager::eComStatus ecsRet = ServiceManager::ecsUndef;

	switch( eDevice )
	{
		case eswvHH:
			ecsRet = ServiceManager::service_RUN_REBOOT( *GetComPort() );
			break;
		
		case eswvWireless:
			ecsRet = ServiceManager::service_RUN_REBOOT_ZBHH( *GetComPort() );
			break;
		
		case eswvDPS:
			ecsRet = ServiceManager::service_RUN_SENSOR_REBOOT( *GetComPort() );
			break;
		
		default:
			break;
	}
	
	PortClose();

	if( ServiceManager::ecsOKFinish != ecsRet )
	{
		return false;
	}
	
	return true;
}

void CDlgWizTAScope::WaitMilliSec( unsigned int uiDelay )
{
	DWORD dwTick = GetTickCount();
	
	while( ( GetTickCount() - dwTick ) < uiDelay )
	{
		TASApp.PumpMessages();
	}
}

bool CDlgWizTAScope::UpdateZBsw(  WirelessModuleType eWhichWirelessModule )
{
	if( false == IsTAScopeAvailable() )	// Initialized ?
	{
		return false;
	}
	
	if( false == PortOpen() )
	{
		return false;
	}

	string fn;
	ServiceManager::eComStatus ecsRet = ServiceManager::ecsUndef;
	
	switch( eWhichWirelessModule )
	{
		case eHandheldWirelessAtmel:
			fn = "handheld.srec";
			ecsRet = ServiceManager::service_RUN_UPDATE_ZBHH( *m_pCom, fn,  &m_dProgress );
			break;

		case eHandheldWirelessDresden:
			fn = "hh_dresden.srec";
			ecsRet = ServiceManager::service_RUN_UPDATE_ZBHH( *m_pCom, fn,  &m_dProgress );
			break;

		case eSensor1Wireless:
			fn = "sensorunit.srec";
			ecsRet = ServiceManager::service_RUN_SENSOR_UPDATE( *m_pCom, fn,  &m_dProgress );

		case eWirelessModuleUnknown:
		default:
			ecsRet = ServiceManager::ecsOKFinish;
			break;
	}
	
	PortClose();
	
	if( ServiceManager::ecsOKFinish != ecsRet )
	{
		return false;
	}

	return true;
}

bool CDlgWizTAScope::GetFileList( CArray<CString> *parList, CArray<UINT32> *parSize )
{
	if( false == IsTAScopeAvailable() )	// Initialized ?
	{
		return false;
	}
	
	if( false == PortOpen() )
	{
		return false;
	}

	ServiceManager::eComStatus ecsRet = ServiceManager::service_F_LIST( *m_pCom, parList, parSize );

	PortClose();
	
	if( ServiceManager::ecsOKFinish == ecsRet )
	{
		return true;
	}

	return false;
}

bool CDlgWizTAScope::DischargeCurrentProject()
{
	if( false == IsTAScopeAvailable() )	// Initialized ?
	{
		return false;
	}
	
	if( false == PortOpen() )
	{
		return false;
	}

	// Discharge the current project.
	ServiceManager::eComStatus ecsRet = ServiceManager::service_F_SNAPSHOT( *m_pCom );
	
	if( ServiceManager::ecsOKFinish == ecsRet )
	{
		ecsRet = ServiceManager::service_RUN_DISCHARGEPROJECT( *m_pCom );
	}

	PortClose();
	
	if( ServiceManager::ecsOKFinish == ecsRet )
	{
		return true;
	}
	
	return false;
}

bool CDlgWizTAScope::DeleteAllQMandLOG()
{
	if( false == IsTAScopeAvailable() )	// Initialized ?
	{
		return false;
	}
	
	if( false == PortOpen() )
	{
		return false;
	}

	ServiceManager::eComStatus ecsRet = ServiceManager::service_RUN_CLEANMAINDATASTRUCT( *m_pCom );

	PortClose();
	
	if( ServiceManager::ecsOKFinish == ecsRet )
	{
		return true;
	}
	
	return false;
}

bool CDlgWizTAScope::DeleteTASCOPEFile( CArray<CString> *parList )
{
	if( false == IsTAScopeAvailable() )	// Initialized ?
	{
		return false;
	}
	
	if( false == PortOpen() )
	{
		return false;
	}

	ServiceManager::eComStatus ecsRet = ServiceManager::eComStatus::ecsOKFinish;
	
	for( int i = 0; i < parList->GetCount() && ServiceManager::ecsOKFinish == ecsRet; i++ )
	{
		if( false == parList->GetAt(i).IsEmpty() )	
		{
			CStringA fnA = CStringA( parList->GetAt( i ) );
			int iPos = fnA.ReverseFind('\\');
			
			if( iPos > -1 )
			{
				fnA.Delete( 0, iPos );
			}

			string fn = (LPCSTR)fnA;	
			ecsRet = ServiceManager::service_F_DELETE( *m_pCom, fn );
		}
	}

	PortClose();
	
	if( ServiceManager::ecsOKFinish == ecsRet )
	{
		return true;
	}

	return false;
}

bool CDlgWizTAScope::DeleteTASCOPEFile( CString fn )
{
	CArray<CString> arFn;
	arFn.Add( fn );
	
	// Remove tsc associated bak file.
	CString ext = _T(".tsc");
	int n = fn.Find( ext );

	if( n > 0 )
	{
		fn.Delete( n, ext.GetLength() );
		fn += _T(".bak");
		arFn.Add( fn );
	}

	return DeleteTASCOPEFile( &arFn );
}

bool CDlgWizTAScope::DeleteTASCOPEFile( const char * pfn )
{
	CString strFn = (CString)CStringA( pfn );
	return DeleteTASCOPEFile( strFn );
}

bool CDlgWizTAScope::BackupTASCOPEFiles( bool fAll )
{
	// Backup tadsx.tsc file and other files if requested.
	// The idea is to have a backup of TADSX in case of...
	__time32_t t;
	t = _time32( NULL );
	
	tm TM;
	_gmtime32_s( &TM, &t );
	CString FolderName;
	FolderName.Format( _T("\\%02d%02d%02d-%02d%02d%02d" ), TM.tm_year - 100, TM.tm_mon + 1, TM.tm_mday, TM.tm_hour, TM.tm_min, TM.tm_sec );
	
	// Create a dated backup folder.
	CString strTSCBackcupPath = CTAScopeUtil::GetTAScopeBackupFolder() + FolderName;
	int iError = SHCreateDirectoryEx( NULL, (LPCTSTR)strTSCBackcupPath, NULL );
	
	if( !( ERROR_SUCCESS == iError  || ERROR_FILE_EXISTS == iError || ERROR_ALREADY_EXISTS == iError ) )
	{
		return false;
	}
	
	// TADSX file.
	bool fRet = GetTADSFile();
	CString strFileName = CTAScopeUtil::GetTAScopeDataFolder() + _T("\\") + TADSXFILE;
	CString strNewFileName = strTSCBackcupPath + _T("\\") + TADSXFILE;
	FileMoveCopy( strFileName, strNewFileName, true );
	
	// Other files.
	// Get file list keep only *.tsc
	if( true == fAll )
	{
		CArray<CString> FileList;
		CArray<UINT32>	FileSize;
		fRet = GetFileList( &FileList, &FileSize );
		double dNbrFiles = 1;	//tadsx.tsc
		double dCount = 1;		//tadsx.tsc	
	
		for( int i = 0; i < FileList.GetCount(); i++ )
		{
			if( FileList.GetAt( i ).Find( _T(".tsc") ) > 0 )
			{
				dNbrFiles++;
			}

			if( FileList.GetAt( i ).Find( _T(".bak") ) > 0 )
			{
				dNbrFiles++;
			}
		}

		if( dNbrFiles > 1 )
		{
			CreateProgressBar( ProgressBarType::PBT_Download );

			for( int i = 0; i < FileList.GetCount(); i++ )
			{
				if( FileList.GetAt( i ).Find( _T(".tsc") ) > 0 || FileList.GetAt( i ).Find( _T(".bak") ) > 0 )
				{
					SetProgressValue( dCount / dNbrFiles * 100 );
					strFileName = strTSCBackcupPath + _T("\\") + FileList.GetAt( i );
					ReadFile( FileList.GetAt( i ), strFileName, false );
					dCount++;
				}
			}
			DeleteProgressBar();
		}
	}

	return true;
}

bool CDlgWizTAScope::ReadFile( CString strSrcFn, CString strTrgFn, bool fUseProgressBar )
{
	if( false == IsTAScopeAvailable() )	// Initialized ?
	{
		return false;
	}

	if( false == PortOpen() )
	{
		return false;
	}

	// Start progress bar.
	if( true == fUseProgressBar )
	{
		CreateProgressBar( ProgressBarType::PBT_Download );
	}
	
	string fnsrc = (LPCSTR)( (CStringA)strSrcFn );
	ServiceManager::eComStatus ecsRet = ServiceManager::service_F_READ( *m_pCom, fnsrc, &strTrgFn, ( true == fUseProgressBar ) ? &m_dProgress : NULL );

	if( true == fUseProgressBar )
	{
		DeleteProgressBar();
	}
	
	if( ServiceManager::ecsOKFinish != ecsRet )
	{
		PortClose();
		return false;
	}

	PortClose();
	
	return true;
}

bool CDlgWizTAScope::WriteFile( CString strSrcFn, bool fSecure, bool fUseProgressBar )
{
	if( false == IsTAScopeAvailable() )	// Initialized ?
	{
		return false;
	}
	
	if( false == PortOpen() )
	{
		return false;
	}

	// Start progress bar.
	if( true == fUseProgressBar )
	{
		CreateProgressBar( ProgressBarType::PBT_Upload );
	}
	
	string fnsrc = (LPCSTR)( (CStringA)strSrcFn );
	ServiceManager::eComStatus ecsRet = ServiceManager::service_F_WRITE( *m_pCom, fnsrc, fSecure, &m_dProgress );

	if( true == fUseProgressBar )
	{
		DeleteProgressBar();
	}

	if( ServiceManager::ecsOKFinish != ecsRet )
	{
		PortClose();
		return false;
	}

	PortClose();
	return true;
}

CDlgWizTAScope::eTAScopeUpdate CDlgWizTAScope::CheckForUpdate( bool fForce )
{
	// Clean HFT variables.
	CleanHFTs();

	m_pSFT = new CFileTable(); // TA-SCOPE file table into the PC
	m_pHFT = new CFileTable(); // TA-SCOPE file table into the TASCOPE
	m_pFT = new CFileTable();  // file table with new files that should be transfered into the TA-SCOPE

	// Check if the HFT-TASCOPE exist.
	CFileStatus status;

	// Ex: "c:\Users\FFustin\Documents\HySelect\tascope\int\hft-tascope.txt".
	if( FALSE == CFile::GetStatus( CTAScopeUtil::GetTAScopeLocHFTPath(), status ) )
	{
		return CDlgWizTAScope::eTAScopeUpdate::eupdLocalHftMissing;			// Local HFT doesn't exist ---> No Update possible
	}
	else
	{
		// Load server(TAS) hft.
		m_pSFT->ReadFile( (wchar_t*)(LPCTSTR)CTAScopeUtil::GetTAScopeLocHFTPath() );
		
		// Verify files integrity.
		CList<CString,CString&> errorList;
		CheckHFTIntegrity( m_pSFT, &errorList );			// Verify CRC32 for each file
		
		if( errorList.GetCount() > 0 )
		{
			return CDlgWizTAScope::eTAScopeUpdate::eupdIncorrectCRC;
		}

	}
	
	// HFT exist into the maintenance folder remove it.
	// Remark: HFT in "c:\Users\FFustin\Documents\HySelect\tascope\int\" is the available one to be sent to the TA-Scope.
	//         HFT in "c:\Users\FFustin\Documents\HySelect\tascope\int\Maintenance" is the one that is in the TA-Scope.
	// Here, we will remove the HFT that is in the "Maintenance" folder and will download the one for the TA-Scope currently connected.
	CString MaintHFTPath = CTAScopeUtil::GetTAScopeMaintHFTPath();
	RemoveFile( MaintHFTPath );

	// When we are in boot mode propose to update all.
	if( true == IsInBootMode() )
	{
		return CDlgWizTAScope::eTAScopeUpdate::eupdUpdateAvailable;
	}

	// Test if HFT exist into the TA-SCOPE, download it if exist.
	// Ex: Read HFT file from TA-Scope and transfer it into "c:\Users\FFustin\Documents\HySelect\tascope\int\Maintenance\hft-tascope.txt".
	if( true == fForce || false == ReadFile( _TASCOPE_HFT, CTAScopeUtil::GetTAScopeMaintHFTPath(), false ) )
	{
		return CDlgWizTAScope::eTAScopeUpdate::eupdTASCOPEHftMissing;		// TA-SCOPE HFT doesn't exist --> Update needed
	}
	
	m_pHFT->ReadFile( (wchar_t*)(LPCTSTR)MaintHFTPath );

	double dTAScopeDBVer4digits = 0;
	
	// Update TA-SCOPE HFT with real version of TA-SCOPE and DPS.
	// Remark: "m_CurrentVersion" contains real version of HH, DB, Wireless from the TA-Scope currently connected and read
	//         by the "RequestSoftwareVersion" method.
	double dAppVers = m_pHFT->VersionStringToDouble( m_CurrentVersion.GetAt( CDlgWizTAScope::eSoftwareVersion::eswvHH ) );
	
	// If TA-Scope is connected and we had successfully read software versions.
	if( false == m_CurrentVersion.GetAt( CDlgWizTAScope::eSoftwareVersion::eswvHH ).IsEmpty() )
	{
		if( true == m_pHFT->FindTargetWoPath( _T("app.bin") ) )
		{
			m_pHFT->SetVersion( dAppVers );
		}
		
		if( eHandheldWirelessAtmel == m_eHHWirelessModuleType && true == m_pHFT->FindTargetWoPath( _T("handheld.srec") ) )
		{
			m_pHFT->SetVersion( m_pHFT->VersionStringToDouble( m_CurrentVersion.GetAt( CDlgWizTAScope::eSoftwareVersion::eswvWireless ) ) );
		}

		if( eHandheldWirelessDresden == m_eHHWirelessModuleType && true == m_pHFT->FindTargetWoPath( _T("hh_dresden.srec") ) )
		{
			// 'eswvWireless' contains wireless version without modification in case of the Dresden.
			m_pHFT->SetVersion( m_pHFT->VersionStringToDouble( m_CurrentVersion.GetAt( CDlgWizTAScope::eSoftwareVersion::eswvWireless ) ) );
		}
		
		if( true == m_pHFT->FindTargetWoPath( _T("tadb.xdb") ) )
		{
			// A version is encoded like "4.4.8.5".
			// The 'VersionStringToDouble' will convert it in "404.0805'.
			dTAScopeDBVer4digits = m_pHFT->VersionStringToDouble( m_CurrentVersion.GetAt( CDlgWizTAScope::eSoftwareVersion::eswvDB ) );
			
			// We will compare only the first 3 upper digits; 4th digit is reserved for HySelect DB modification.
			// 404.0805 * 100 = 40408.05 -> cast with int it becomes 40408 and divided now by 100 we have 404.08.
			// In this way we have well the DB version in double without the last digit.
			double dTAScopeDBVersion = ( (double)( (int)( dTAScopeDBVer4digits * 100 ) ) ) / 100;
			m_pHFT->SetVersion( dTAScopeDBVersion );

			// We do the same for the "tadb.xdb" file version that are to be sent to the TA-Scope.
			m_pSFT->FindTargetWoPath( _T("tadb.xdb") );
			double dTasDBVersion = m_pSFT->GetVersion();
			dTasDBVersion = ( (double)( (int)( dTasDBVersion * 100 ) ) ) / 100;
			m_pSFT->SetVersion( dTasDBVersion );

			CDB_MultiString *pMS = dynamic_cast<CDB_MultiString *>( TASApp.GetpTADB()->Get( _T("TADB_VERSION") ).MP );
			
			if( NULL != pMS )
			{
				CString str = pMS->SearchSubString( _T("TadbKey") ).c_str();
				str.MakeLower();
				CString strDBTASCOPE = m_CurrentVersion.GetAt( CDlgWizTAScope::eSoftwareVersion::eswvDBKey );
				strDBTASCOPE.MakeLower();
				
				// Different Database ! force upload.
				if( str != strDBTASCOPE )
				{
					m_pHFT->SetVersion( 0 );
				}
			}
		}
		
		// DPS connected?
		if( m_CurrentVersion.GetAt( CDlgWizTAScope::eSoftwareVersion::eswvDPS ) != _T("-") )
		{
			if( true == m_pHFT->FindTargetWoPath( _T("SensorUnit.srec") ) )
			{
				m_pHFT->SetVersion( m_pHFT->VersionStringToDouble( m_CurrentVersion.GetAt( CDlgWizTAScope::eSoftwareVersion::eswvDPS ) ) );
			}
		}
	}

	// No update needed when the TA-SCOPE run a newer version.
	m_pSFT->FindTargetWoPath( _T("app.bin") );
	
	if( dAppVers > m_pSFT->GetVersion() )
	{
		return CDlgWizTAScope::eTAScopeUpdate::eupdNoUpdateNeeded;
	}

	m_pSFT->CompareVersionFileTable( m_pHFT, m_pFT, dAppVers );
	
	// Restore 4 digits for TA-SCOPE DB version.
	m_pHFT->FindTargetWoPath( _T("tadb.xdb") );
	m_pHFT->SetVersion( dTAScopeDBVer4digits );

	// Check if there is a difference between dps2.dfu version. If it's the case we need to warn user after the update
	// of the TA-Scope.
	if( true == m_pSFT->FindTargetWoPath( _T("dps2.dfu") ) )
	{
		if( false == m_pHFT->FindTargetWoPath( _T("dps2.dfu") ) || 
			m_pSFT->GetVersion() > m_pHFT->GetVersion() )
		{
			m_bNewDpSVisioFirmware = true;
		}
	}
	
	// One difference was detected update will be available.
	if( m_pFT->GetCount() > 0 )
	{
		return CDlgWizTAScope::eTAScopeUpdate::eupdUpdateAvailable;
	}

	return CDlgWizTAScope::eTAScopeUpdate::eupdNoUpdateNeeded;
}

bool CDlgWizTAScope::CheckHFTIntegrity( CFileTable *pSFT, CList<CString,CString&> *errorList )
{
	if( NULL == pSFT )
	{
		return false;
	}
	
	// Variables.
	bool fRet = true;
	CFileStatus fs;
	bool fContinue;

	for( fContinue = pSFT->GetFirst(); false == fContinue; fContinue = pSFT->GetNext() )  //get the list in SFT.
	{
		CString Target = pSFT->GetTarget();  
		Target = Target.MakeLower();
		CString TargetPath = CTAScopeUtil::CompleteTargetPath( Target );

		if( TRUE == CFile::GetStatus( TargetPath, fs ) ) // check if the file exists and get the status.
		{
			CCRC32 Crc32;

			if( Crc32.Get_CRC( TargetPath ) != pSFT->GetCRC32() )  //check the CRC of the file.
			{
				errorList->AddTail( TargetPath );  // the file CRC is incorrect.
				fRet = false;
			}
		}
		else
		{
			errorList->AddTail( TargetPath );  //The status information for the specified file can not be successfully obtained.
			fRet = false;
		}

	}

	return fRet;
}

BEGIN_MESSAGE_MAP( CDlgWizTAScope, CWizardManager )
	ON_WM_TIMER()
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CDlgWizTAScope::DoDataExchange( CDataExchange* pDX )
{
	CWizardManager::DoDataExchange( pDX );
}

BOOL CDlgWizTAScope::OnInitDialog()
{
	CWizardManager::OnInitDialog();
	return TRUE; 
}

void CDlgWizTAScope::OnTimer( UINT_PTR nIDEvent )
{
	CWizardManager::OnTimer( nIDEvent );

	if( (UINT_PTR)0 == m_nTimer || m_nTimer != nIDEvent )
	{
		return;
	}

	if( NULL != m_pDlgProgressBar && NULL != m_pDlgProgressBar->GetSafeHwnd() )
	{
		int iIDS = ( PBT_Download == m_eProgressBarType ) ? IDS_TASCOPE_WIZARD_COLLECTINGDATA : IDS_TASCOPE_WIZARD_SENDINGDATA;
		CString str = TASApp.LoadLocalizedString( iIDS );
		m_pDlgProgressBar->UpdateProgress( (int)m_dProgress, (LPCTSTR)str );
	}
}

void CDlgWizTAScope::OnSize( UINT nType, int cx, int cy )
{
	CWizardManager::OnSize( nType, cx, cy );
}

bool CDlgWizTAScope::OnWizFinish()
{
	PortClose();
	return true;
}

bool CDlgWizTAScope::PortOpen( int i )
{
	if( NULL == this->GetSafeHwnd() )
	{
		return false;
	}
	
	if( true == IsComPortBuzy() )
	{
		return false;
	}
	
	if( i > 0 )
	{
		m_iComNumber = i;
	}
	
	if( NULL != m_pCom )
	{
		PortClose();
	}
	
	SetComPortBuzy( true );
	m_pCom = new CSerialPort;

	if( NULL != m_pCom )
	{
		bool fRet = m_pCom->PortOpen( m_iComNumber, COMSPEED, NOPARITY, 8, ONESTOPBIT, (CWnd*)this );

		if( true == fRet )
		{
			return fRet;
		}
	}

	PortClose();
	return false;
}

void CDlgWizTAScope::PortClose()
{
	if( NULL != m_pCom )
	{
		m_pCom->PortClose();
		delete m_pCom;
		m_pCom = NULL;
		SetComPortBuzy( false );
	}
}

void CDlgWizTAScope::CleanHFTs()
{
	if( NULL != m_pSFT )
	{
		delete m_pSFT;
	}
	
	if( NULL != m_pHFT )
	{
		delete m_pHFT;
	}
	
	if( NULL != m_pFT )
	{
		delete m_pFT;
	}
	
	m_pSFT = NULL; // TA-SCOPE file table into the PC
	m_pHFT = NULL; // TA-SCOPE file table into the TASCOPE
	m_pFT  = NULL; // file table with new files that should be transfered into the TA-SCOPE
}
