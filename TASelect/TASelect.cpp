#include "stdafx.h"
#include "afxwinappex.h"
#include <VersionHelpers.h>
#include "stringex.h"
#include "ActWatch.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "TASelectDoc.h"
#include "TASelectView.h"
#include <windows.h>
#include "RestartAPI.h"
#include "Gdiplus.h"

#include "DlgLeftTabInfo.h"
#include "DlgLeftTabProject.h"

#include "DlgAboutBox.h"
#include "DlgGateway.h"
#include "DlgDocs.h"
#include "DlgHtmlView.h"
#include "DlgTip.h"
#include "DlgTechParam.h"
#include "RegAccess.h"
#include "SplashWindow.h"
#include "STDFU.h"

#include <AtlBase.h>
#include <AtlConv.h>
#include "DlgLeftTabSelManager.h"

#include "HySelectExceptionHandlers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// The one and only CPhoenixApp object.
CTASelectApp TASApp;
#ifndef _DEBUG
CHySelectExceptionHandler clHySelectExceptionHandler( &TASApp );
#endif

CTASelectApp::CTASelectApp()
{
	m_strFileToOpen = _T("");
	m_hmHubSchemes = ( HMODULE )0;
	m_hmLanguageRes = ( HMODULE )0;
	m_hmProdPics = ( HMODULE )0;
	m_hmLocProdPics = ( HMODULE )0;
	m_hspread = ( HMODULE )0;
	m_pTADB = NULL;
	m_pUSERDB = NULL;
	m_pTADS = NULL;
	m_bDebugCom = false;
	m_bLogRawData = false;
	m_fTestInRelease = false;
	m_fUnitTest = false;
	m_bRegisterRunning = false;
	m_bWizardMode = false;
	m_bAutoUpdate = false;
	m_fHidingPswSet = false;
	m_bPriceUsed = false;
	m_bHubPriceUsed = false;
	m_bAutoSave = false;
	m_bTrvUsed = false;
	m_bDpCUsed = false;
	m_bHubUsed = false;
	m_bCvUsed = false;
	m_bPICvUsed = false;
	m_bBCvUsed = false;
	m_bDpCBCVUsed = false;
	m_b6WayCVUsed = false;	// HYS-1150
	m_bShutOffValveUsed = false;
	m_bAirVentSepUsed = false;
	m_bPressureMaintenanceUsed = false;
	m_bPMPriceIndexUsed = false;
	m_bSafetyValveUsed = false;
	m_bSmartControlValveUsed = false;
	m_bSmartDpUsed = false;             // HYS-1935: TA-Smart Dp - 02 - Add a new category for the ribbon
	m_bTapWaterControlUsed = false;		// HYS-1258
	m_bFloorHeatingControlUsed = false;
	m_bTBVCMAuthOnFullDp = false;
	m_bReadTADBTxt = false;
	m_bLocalArtNumber = false;
	m_bHideTAArtNumber = false;
	m_bAppStarted = false;
	m_bUpdateRegistryStartingFolder = false;
	m_bStartingWOInstal = false;
	m_uiMaintainTAScopeFiles = 0;
	m_strLanguageKey = _T( "" );
	m_strLanguage = _T( "" );
	m_strNextLanguage = _T( "" );
	m_strDocLanguage = _T( "" );
	m_strNameLocalArtNumber = _T( "" );
	m_strStartDir = _T( "" );
	m_strCrossVerifyDocLangKey = _T( "" );
	m_strHyUpdate_InstDir = _T( "" );
	m_strCvFamily = _T( "" );
	m_strDocumentsFolderForDoc = _T("");
	m_strDocumentsFolderForHelp = _T("");
	m_eProdPicType = CProdPic::LastPicType;
	m_pInstanceEvent = NULL;
	m_pShutdownEvent = NULL;
	m_pActivationWatch = NULL;
	m_hStdOut = ( HANDLE )0;
	m_BckColor = 0;
	m_mQrsList.clear();
	m_uiAppLook = 0;
	m_pclRCImageManager = NULL;
}

CTASelectApp::~CTASelectApp()
{
	//Debug Console
	#ifdef _DEBUG
	if( m_stdFileDebug.m_pStream != NULL )
	{
		m_stdFileDebug.Close();
	}

	if( NULL != m_hStdOut )
	{
		FreeConsole();
		//CloseHandle( m_hStdOut );
	}

	#endif

	// If we've not shutdown, do it!
	if( m_pShutdownEvent != NULL )
	{
		m_pShutdownEvent->SetEvent();

		if( m_pActivationWatch != NULL )
		{
			::WaitForSingleObject( m_pActivationWatch->m_hThread, INFINITE );
		}

		delete m_pShutdownEvent;
		m_pShutdownEvent = NULL;
	}

	// Kill the instance handle, too.
	if( m_pInstanceEvent != NULL )
	{
		m_pInstanceEvent->Unlock();
		delete m_pInstanceEvent;
		m_pInstanceEvent = NULL;
	}

	if( NULL != m_pclRCImageManager )
	{
		delete m_pclRCImageManager;
	}
}

BEGIN_MESSAGE_MAP( CTASelectApp, CWinAppEx )
	ON_COMMAND( ID_APPMENU_NEW, OnFileNew )
	ON_COMMAND( ID_APPMENU_OPEN, OnFileOpen )
	ON_COMMAND( ID_APPMENU_PRINT_SETUP, OnFilePrintSetup )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CTASelectApp::LoadFlags()
{
	try
	{
		// Set flags PriceUsed, DpcUsed and TrvUsed.
		CDB_MultiString *pMStr = ( CDB_MultiString * )( GetpTADB()->Get( _T("TADB_VERSION") ).MP );

		if( NULL == pMStr )
		{
			HYSELECT_THROW( _T("Can't retrieve the object 'TADB_VERSION' from the current database.") );
		}

		CString str = pMStr->SearchSubString( _T("Price Used") ).c_str();
		str.MakeLower();
		m_bPriceUsed = false;

		if( _T("true") == str )
		{
			m_bPriceUsed = true;
		}

		str = pMStr->SearchSubString( _T("Hub Price Used") ).c_str();
		str.MakeLower();
		m_bHubPriceUsed = false;

		if( _T("true") == str )
		{
			m_bHubPriceUsed = true;
		}

		str = pMStr->SearchSubString( _T("DpC Used") ).c_str();
		str.MakeLower();
		m_bDpCUsed = false;

		if( _T("true") == str )
		{
			m_bDpCUsed = true;
		}

		str = pMStr->SearchSubString( _T("Cv Used") ).c_str();
		str.MakeLower();
		m_bCvUsed = false;

		if( _T("true") == str )
		{
			m_bCvUsed = true;
		}

		str = pMStr->SearchSubString( _T("Cv Family") ).c_str();
		str.MakeLower();
		m_strCvFamily = str;

		str = pMStr->SearchSubString( _T("PICv Used") ).c_str();
		str.MakeLower();
		m_bPICvUsed = false;

		if( _T("true") == str )
		{
			m_bPICvUsed = true;
		}

		str = pMStr->SearchSubString( _T("BCV Used") ).c_str();
		str.MakeLower();
		m_bBCvUsed = false;

		if( _T("true") == str )
		{
			m_bBCvUsed = true;
		}

		str = pMStr->SearchSubString(_T("DpCBCV Used")).c_str();
		str.MakeLower();
		m_bDpCBCVUsed = false;

		if (_T("true") == str)
		{
			m_bDpCBCVUsed = true;
		}
		
		// HYS-1150
		str = pMStr->SearchSubString(_T("6WayCV Used")).c_str();
		str.MakeLower();
		m_b6WayCVUsed = false;

		if (_T("true") == str)
		{
			m_b6WayCVUsed = true;
		}

		str = pMStr->SearchSubString(_T("ShutOffV Used")).c_str();
		str.MakeLower();
		m_bShutOffValveUsed = false;

		if (_T("true") == str)
		{
			m_bShutOffValveUsed = true;
		}

		str = pMStr->SearchSubString( _T("AirVSep Used") ).c_str();
		str.MakeLower();
		m_bAirVentSepUsed = false;

		if( _T("true") == str )
		{
			m_bAirVentSepUsed = true;
		}

		str = pMStr->SearchSubString( _T("PressMaint Used") ).c_str();
		str.MakeLower();
		m_bPressureMaintenanceUsed = false;

		if( _T("true") == str )
		{
			m_bPressureMaintenanceUsed = true;
		}

		str = pMStr->SearchSubString( _T("PM Price Index Used") ).c_str();
		str.MakeLower();
		m_bPMPriceIndexUsed = false;

		if( _T("true") == str )
		{
			m_bPMPriceIndexUsed = true;
		}

		str = pMStr->SearchSubString( _T("Safety valve Used") ).c_str();
		str.MakeLower();
		m_bSafetyValveUsed = false;

		if( _T("true") == str )
		{
			m_bSafetyValveUsed = true;
		}

		str = pMStr->SearchSubString( _T("Smart control valve Used") ).c_str();
		str.MakeLower();
		m_bSmartControlValveUsed = false;

		if( _T("true") == str )
		{
			m_bSmartControlValveUsed = true;
		}

		// HYS-1935: TA-Smart Dp - 02 - Add a new category for the ribbon
		str = pMStr->SearchSubString( _T("Smart Dp Used") ).c_str();
		str.MakeLower();
		m_bSmartDpUsed = false;

		if( _T("true") == str )
		{
			m_bSmartDpUsed = true;
		}

		str = pMStr->SearchSubString( _T("Hub Used") ).c_str();
		str.MakeLower();
		m_bHubUsed = false;

		if( _T("true") == str )
		{
			m_bHubUsed = true;
			m_hmHubSchemes = LoadLibrary( _HUB_SCHEMES_DLL );

			if( NULL == m_hmHubSchemes )
			{
				if( m_hmHubSchemes )
				{
					FreeLibrary( m_hmHubSchemes );
				}

				throw CHySelectException( TASApp.LoadLocalizedString( AFXMSG_DLLLOADINGFAILED ), __LINE__, __FILE__ );
			}
		}

		str = pMStr->SearchSubString( _T("Trv Used") ).c_str();
		str.MakeLower();
		m_bTrvUsed = false;

		if( _T("true") == str )
		{
			m_bTrvUsed = true;
		}

		// HYS-1571: floor heating control added.
		str = pMStr->SearchSubString( _T("Floor heating control used") ).c_str();
		str.MakeLower();
		m_bFloorHeatingControlUsed = false;

		if( _T("true") == str )
		{
			m_bFloorHeatingControlUsed = true;
		}

		// HYS-1258: tap water control added.
		str = pMStr->SearchSubString( _T("Tap water control used") ).c_str();
		str.MakeLower();
		m_bTapWaterControlUsed = false;

		if( _T("true") == str )
		{
			m_bTapWaterControlUsed = true;
		}

		// Set flags Local Article Number, Hide TA Article number and get Local Article number prefix.
		str = pMStr->SearchSubString( _T("LAN Used") ).c_str();
		str.MakeLower();
		m_bLocalArtNumber = false;

		if( _T("true") == str )
		{
			m_bLocalArtNumber = true;
		}

		str = pMStr->SearchSubString( _T("HideTaArt") ).c_str();
		str.MakeLower();
		m_bHideTAArtNumber = false;

		if( _T("true") == str )
		{
			m_bHideTAArtNumber = true;
		}

		m_strNameLocalArtNumber = pMStr->SearchSubString( _T("LocPrefix") ).c_str();

		// 'VERSION_LANGUAGE' is defined in 'localdb_txt' and contains a list of language versions encoded with a few characters.
		// (ex: for Belgium we have "nl", "fr" and "en").
		CDB_MultiString *pMStrLng = ( CDB_MultiString * )( GetpTADB()->Get( _T("VERSION_LANGUAGE") ).MP );
		CString CurLang = pMStrLng->GetString( 0 );

		// Try to read in registry the "Language" key in "Language" section. If not exists, take 'CurLang'.
		m_strLanguage = GetProfileString( _T("Language" ), _T( "Language"), ( LPCTSTR )CurLang );
		m_strLanguage.MakeLower();

		// HYS-1142: Verify if the current language exist yet.
		CTableOrdered *pTab = (CTableOrdered *)( TASApp.GetpTADB()->Get( _T("LANG_TAB") ).MP );
		ASSERT( NULL != pTab );

		bool bFound = false;

		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID && false == bFound; IDPtr = pTab->GetNext() )
		{
			if( 0 == CString( ( (CDB_StringID *)IDPtr.MP )->GetIDstr2() ).CompareNoCase( m_strLanguage ) )
			{
				bFound = true;
			}
		}

		if( false == bFound )
		{
			m_strLanguage = _T("en");
		}

		// Save "Language" in registry.
		WriteProfileString( _T("Language" ), _T( "Language"), ( LPCTSTR )m_strLanguage );

		// At now, 'm_Language' and 'm_NextLanguage' are the same.
		SetNextLanguage( m_strLanguage );

		// Try to read in registry the "Documentation" key in "Language" section. If not exists, set to "".
		m_strDocLanguage = GetProfileString( _T("Language" ), _T( "Documentation" ), _T( "") );
		m_strDocLanguage.MakeLower();

		// HYS-1142: Verify if the current language exist yet.
		bFound = false;

		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID && false == bFound; IDPtr = pTab->GetNext() )
		{
			if( 0 == CString( ( (CDB_StringID *)IDPtr.MP )->GetIDstr2() ).CompareNoCase( m_strDocLanguage ) )
			{
				bFound = true;
			}
		}

		if( false == bFound )
		{
			m_strDocLanguage = _T("en");
		}

		// Save "Language documentation" in registry.
		WriteProfileString( _T("Language" ), _T( "Documentation"), ( LPCTSTR )m_strDocLanguage );

		str = pMStr->SearchSubString( _T("AutoSaving") ).c_str();
		str.MakeLower();
		m_bAutoSave = false;

		if( _T("true") == str )
		{
			m_bAutoSave = true;
		}

		str = pMStr->SearchSubString(_T("TenderText")).c_str();
		str.MakeLower();
		m_bTenderText = false;

		if (_T("true") == str)
		{
			m_bTenderText = true;
		}

		str = pMStr->SearchSubString(_T("GAEBExport")).c_str();
		str.MakeLower();
		m_bGAEBExport = false;

		if (_T("true") == str)
		{
			m_bGAEBExport = true;
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CTASelectApp::LoadFlags'."), __LINE__, __FILE__ );
		throw;
	}
}

HMODULE CTASelectApp::GetDLLHandle( const TCHAR *pIDstr )
{
	if( NULL == pIDstr )
	{
		return NULL;
	}

	if( 0 == _tcscmp( pIDstr, _HUB_SCHEMES_DLL ) )
	{
		return m_hmHubSchemes;
	}

	return NULL;
}

int CTASelectApp::GetFontFactor( CWnd *pWnd )
{
	CDC *pDC = pWnd->GetDC();
	int prvMode = pDC->SetMapMode( MM_TWIPS );

	TEXTMETRIC tm;
	pDC->GetTextMetrics( &tm );
	pDC->SetMapMode( prvMode );
	int iFactor = ( ( tm.tmDigitizedAspectX - 96 ) * 100 ) / 96 + 100;
	pWnd->ReleaseDC( pDC );
	return iFactor;
}

void CTASelectApp::ComputeTAPSortingKey( CTable *pTab )
{
	HYSELECT_TRY
	{
		if( NULL == pTab )
		{
			pTab = &m_pTADB->Access();
		}

		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("'pTab' is null.") );
		}

		// Loop on the Table.
		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// If the table content a chilTable, do the recursive function...
			if( true == IDPtr.MP->IsaTable() )
			{
				ComputeTAPSortingKey( (CTable *)IDPtr.MP );
			}
			else
			{
				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );

				if( NULL != pTAP )
				{
					pTAP->GetSortingKey();
				}
			}
		}
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CTASelectApp::ComputeTAPSortingKey' method.") )
}

CString CTASelectApp::GetDocLanguage( void )
{
	// If not yet defined...
	if( true == m_strDocLanguage.IsEmpty() )
	{
		m_strDocLanguage = TASApp.GetLanguage();
	}

	return m_strDocLanguage;
}

void CTASelectApp::SetDocLanguage( CString strDocLanguage )
{
	m_strDocLanguage = strDocLanguage;
	WriteProfileString( _T("Language" ), _T( "Documentation"), ( LPCTSTR )strDocLanguage );
}

CString CTASelectApp::LoadLocalizedString( UINT uiID )
{
	// Variable.
	CString str = L"";

	// Try to get the string from the xml resource.
	WCHAR *pStr = GetpXmlRcTab()->GetIDSStr( uiID );

	if( NULL != pStr )
	{
		str = pStr;
	}
	else
	{
		// In case it doesn't work, retrieve the resource string.
		str.LoadString( uiID );
	}

	return str;
}

void CTASelectApp::StartProcess( CString strExeName, int iParamCount, ... )
{
	CString strParam;
	va_list list;
	va_start( list, iParamCount );

	for( int nArg = 0; nArg < iParamCount; nArg++ )
	{
		if( nArg > 0 )
		{
			strParam += CString( _T(" ") );
		}

		strParam += va_arg( list, TCHAR * );
	}

	va_end( list );

	CoInitializeEx( NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE );

	SHELLEXECUTEINFO ShExecInfo;

	// Fill-in the SHELLEXECUTEINFO structure.
	memset( &ShExecInfo, 0, sizeof( SHELLEXECUTEINFO ) );
	ShExecInfo.cbSize = sizeof( SHELLEXECUTEINFO );
	ShExecInfo.fMask = NULL;
	ShExecInfo.hwnd = NULL;

	// Try to open database.
	CString fn = TASApp.GetStartDir() + _SUBLANGUAGE_FILENAME;
	BOOL bAdmin = WinSys::IsUserAdmin();

	if( WinSys::ElevationNeeded( ( LPTSTR )( LPCTSTR )fn ) )
	{
		ShExecInfo.lpVerb =  _T("runas");
	}
	else
	{
		ShExecInfo.lpVerb = NULL;
	}

	ShExecInfo.lpFile = strExeName;
	ShExecInfo.lpParameters = strParam;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOWNORMAL;
	ShExecInfo.hInstApp = NULL;

	CFileStatus fs;

	if( TRUE == CFile::GetStatus( ( LPCTSTR )strExeName, fs ) )
	{
		ShellExecuteEx( &ShExecInfo );
	}
}

CString CTASelectApp::FileOpen( CTADatastruct *pTADS, LPCTSTR lpszPathName )
{
	// Can be called when user explicitly chooses open file in main menu OR can be called by 'TASelectDoc::OnOpenDocument' when user
	// opens file by MRU list (or using drag & drop method).
	// 1st case called by 'CTASelectDoc::OnFileOpen' and 'CDlgImportHM::OnBnClickedButtonopenproj' methods.
	// 2nd case called by 'CTASelectDoc::OnOpenDocument' method.
	// In 1st case: 'lpszPathName' MUST be set to NULL -> We don't know yet the file name because we ruen the 'CFileDialog' dialog to allow user to search file to open.
	// In 2nd case: 'pTADS' MUST be set to NULL.

	CString strPathName = _T( "" );

	// HYS-1376 : The case when both the filename and datastruct are given.
	if( NULL != pTADS && NULL == lpszPathName )
	{
		// Call comes from 'OnFileOpen'.

		// Get Current directory, this directory is updated when phoenix load TADB.DB see phoenix.cpp InitInstance().
		CString str = TASApp.LoadLocalizedString( IDS_OPENSAVEFILEFILTER );
		CFileDialog dlg( true, _T("tsp"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, ( LPCTSTR )str, NULL );
		CString PrjDir = GetProjectDirectory();

		CString strReturn = _T( "" );
		dlg.m_ofn.lpstrInitialDir = ( LPCTSTR )PrjDir;

		if( IDOK == dlg.DoModal() )
		{
			strPathName = dlg.GetPathName();
			CPath path( strPathName );
			CString strSplitted = path.SplitPath( ( CPath::ePathFields )( CPath::ePathFields::epfDrive + CPath::ePathFields::epfDir ) );

			// Save project directory if modified.
			if( strSplitted != PrjDir )
			{
				AfxGetApp()->WriteProfileString( _T("Files" ), _T( "Project Directory"), str );
			}
		}
	}
	else
	{
		// Call comes from 'OnOpenDocument'.
		if( NULL != lpszPathName )
		{
			strPathName = lpszPathName;
		}
		
		if( NULL == pTADS )
		{
			pTADS = TASApp.GetpTADS();
		}
	}

	// HYS-1146 review by 1334: After openning file the application type and the water char in datastruct keep project values.
	// This boolean allow to get the initial values. For example, we are in changeOver with 6-way valve selection and open
	// a project. After openning, we are alaways in 6-way valve selection but application type is not good any more
	// HYS-1380: Complete this solution with the opposite: We are in heating mode and open a change-over project. The product selection
	// application type must be heating and not change-over. The read process replace the current with the file application type
	ProjectType eOldSelectionApplicationType = ProjectType::InvalidProjectType;

	if( false == strPathName.IsEmpty() )
	{
		// Check .tsp file.
		TRACE( _T("Check the validity of %s.\n"), (TCHAR *)( LPCTSTR )strPathName );
		std::ifstream inpf( strPathName, std::ifstream::in | std::ifstream::binary );
		int rCode;

		if( true == inpf.is_open() )
		{
			rCode = CDataBase::CheckDBFile( inpf );
			inpf.close();
		}
		else
		{
			rCode = CDataBase::CheckDBFileRetCode::FileNotFound;
		}

		// If something wrong...
		if( rCode < 0 )
		{
			CString str1;
			CString str = strPathName;
			str = str.Right( str.GetLength() - str.ReverseFind( '\\' ) - 1 );

			switch( rCode )
			{
				// Not a correct file.
				case CDataBase::CheckDBFileRetCode::Invalid:
					FormatString( str1, AFXMSG_NOTACORRECTFILE, str );
					break;

				// Format Version is not correct.
				case CDataBase::CheckDBFileRetCode::TooRecent:
					FormatString( str1, AFXMSG_TSPFORMATNEWERTHANTAS2, str );
					break;

				case CDataBase::CheckDBFileRetCode::NotOkUnicode:
					FormatString( str1, AFXMSG_UNICODETSPOPENFROMSBCS, str );
					break;

				case CDataBase::CheckDBFileRetCode::FileNotFound:
					FormatString( str1, AFXMSG_FILENOTFOUND, str );
					break;
			}

			::AfxMessageBox( str1 );
			
			// Set to NULL to avoid 'CTASelectDoc::OnFileOpen()' to continue with this file.
			strPathName = _T( "" );
		}
		else
		{
			if( CDataBase::CheckDBFileRetCode::OkSbcs == rCode )
			{
				CString	str;
				CString tmp = strPathName;
				tmp = tmp.Right( tmp.GetLength() - tmp.ReverseFind( '\\' ) - 1 );
				FormatString( str, AFXMSG_SBCSTSPOPENFROMUNICODE, tmp );
				::AfxMessageBox( str, MB_OK | MB_ICONINFORMATION, 0 );
			}

			// CDocument clear.
			pTASelectDoc->DeleteContents();

			// Read .tsp file.
			TRACE( _T("Read the TADS in binary file %s.\n"), (TCHAR *)( LPCTSTR )strPathName );
			std::ifstream inpf( strPathName, std::ifstream::in | std::ifstream::binary );

			// HYS-1146 : before reading the file we save the current project type.
			// HYS-1368 : fileOpen function can be called with pTADS->GetpTechParams() null.
			if( NULL != pTADS->GetpTechParams() )
			{
				// HYS-1380: Keep old application type.
				eOldSelectionApplicationType = pTADS->GetpTechParams()->GetProductSelectionApplicationType();
			}

			// Read TADS.
			try
			{
				pTADS->Read( inpf );
			}
			catch( CHySelectException &clHySelectException )
			{
				// We don't need here the messages. And here we call the 'ClearMessages' method to avoid a compilation warning.
				clHySelectException.ClearMessages();

				inpf.close();

				CString str1, str;
				CPath path( strPathName );
				str = path.SplitPath( ( CPath::ePathFields )( CPath::ePathFields::epfFName + CPath::ePathFields::epfExt ) );
				FormatString( str1, AFXMSG_NOTACORRECTFILE, str );
				::AfxMessageBox( str1 );
				pTADS->Modified( false );
				OnFileNew();

				// We intentionally call 'return' instead of 'throw' because we want here that HySelect recreate an empty document.
				return L"";
			}

			inpf.close();

			// Add the missing parts in TADS structure if necessary.
			pTADS->CreateTSPProjTable();

			// Verify Position of each circuit in his parent.
			pTADS->VerifyAndCorrectPosition();

			pTADS->VerifyAndCleanEmptySSelObj();

			// FF: 2014-03-27: A bug has been introduced in previous version before 4.2.3.4 concerning saving of the 'CDS_BatchSelParameter' in
			//                 a '.tsp' file. We need to verify if data in 'CDS_BatchSelParameter' are relevant.
			pTADS->VerifyBatchSelParameter();

			// FF: 2015-02-26: Sometimes the measurements done in hydronic network and imported from the TA Scope were bad (negative values). 
			// Before the correction (reset by a default value) it was possible to import and directly save to a TSP file. In this case, 
			// we need to correct the bad values.
			pTADS->VerifyMeasTaBalDateTime();
			
			// FF: 2015-02-26: Sometimes the quick measurements imported from the TA Scope were bad (negative values). Before the correction (reset
			// by a default value) it was possible to import and directly save to a TSP file. In this case, we need to correct the bad values.
			pTADS->VerifyQuickMeasurements();

			// HYS-1857: From HySelect v4.5.0.1 until v4.5.0.6 (Included) an error has been introduced concerning BV at secondary side for 3-way mixing circuits.
			// It was set in the "tadb.txt" file that the BV was optional but it's no true. So, we need to correct now all projects where user
			// has intentionally removed the BV.
			pTADS->VerifyAllBVSecForAll3WayCircuits();

			// FF: 2018-05-31: before datastruct v28 (before HySelect v4.4.4.9), we have not a perfect match between temperatures set in the 
			// 'CDS_TechnicalParameters' and the temperatures set in the 'PROJ_WC' and 'PARAM_WC' object. It happened for example when you work
			// on a hydraulic network and change the supply temperature in the ribbon. Temperature was saved in the 'PROJ_WC object but not in the
			// 'CDS_TechnicalParameters'. If you open the project with the actual version, it's the data from the 'CDS_TechnicalParameters' that 
			// overwrite the 'PROJ_WC' and 'PARAM_WC'. And because that we lost the real temperature that user had set in the ribbon for his project.
			// To summarize: if datastruct version <= v27, copy 'PROJC_WC'/'PARAM_WC' to 'CDS_TechnicalParameter' (It concerns the supply temperature).
			pTADS->VerifyWaterCharAndTechnicalParameters( eOldSelectionApplicationType );

			// HYS-1207: Update WaterChar of injection circuits.
			// pTADS->RefreshAllSecondaryWCForHMCalc();

			// If project is old and has never been converted to hydrocalc mode OR if project is new and it has not been intentionally converted in
			// hydrocalc mode...
			if( ( true == pTADS->IsOldProject() && eb3True == pTADS->IsHMCalcMode( NULL, false ) )
					|| ( false == pTADS->IsOldProject() && true == pTADS->GetUserForcedHMCalcModeFlag() ) )
			{
				// HYS-1716: No more need of the 'CTADatastruct::RefreshAllSecondaryWCForHMCalc' method.
				// When opening a project we run it to update all the water characteristics and temperatures for all pipes.
				// HYS-1882: Add a boolean to determinate if we run the project from the root or from the deepest circuit.
				// The value is false to run from the root module. It should be the case when opening a file because
				// we need that the parent WC are up to date before managed child circuit.
				pTADS->VerifyProjectAllWaterCharacteristics( false );
			}
			
			// Needed to update working variables, HM is frozen, no alteration of saved variables.
			pTADS->ComputeAllInstallation();
		}
	}

	return strPathName;
}

WCHAR CTASelectApp::GetSiteNameMaxChar()
{
	CDS_ProjectParams *pPrjParam = GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == pPrjParam->IsCompatibleCBI() )
	{
		return CBIMAXSITENAMECHAR;
	}

	return SCOPEMAXSITENAMECHAR;
}

WCHAR CTASelectApp::GetModuleNameMaxChar()
{
	CDS_ProjectParams *pPrjParam = GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == pPrjParam->IsCompatibleCBI() )
	{
		return CBIMAXMODULENAMECHAR;
	}

	return SCOPEMAXMODULENAMECHAR;
}

bool CTASelectApp::ShowNewInHtmlPage( bool fAppStarting )
{
	// Get information about Maintaining TAScope files.
	if( true == fAppStarting )
	{
		int iDisplayNewIn = GetProfileInt( HYUPDATE, SHOWNEWIN, 1 );

		if( 0 == iDisplayNewIn )
		{
			return false;
		}
	}

	CString strFilename;
	CString DocumentFolder = TASApp.GetDocumentsFolderForHelp();

	// Find the Readme file according to the language.
	strFilename = DocumentFolder + CString( _HYSELECT_NAME_BCKSLASH ) + CString( _HELP_DIRECTORY );
	strFilename += CString( L"NewIn.htm" );

	CFileFind finder;
	BOOL fFound = finder.FindFile( strFilename );

	if( TRUE == fFound )
	{
		// If the file exist, we display it.
		CDlgHtmlView dlg;
		dlg.Display( strFilename, IDS_DLGNEWIN_TITLE, ( true == fAppStarting ) ? IDS_DLGNEWIN_INFO : 0 );

		// And we reset the registry key to not show at each starting of HySelect.
		WriteProfileInt( HYUPDATE, SHOWNEWIN, 0 );
		return true;
	}

	return false;
}

void CTASelectApp::PumpMessages()
{
	// Handle dialog messages.
	MSG msg;

	while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
	{
		if( !AfxGetApp()->PreTranslateMessage( &msg ) )
		{
			::TranslateMessage( &msg );
			::DispatchMessage( &msg );
		}

		AfxGetApp()->OnIdle( 0 );   // updates user interface.
		AfxGetApp()->OnIdle( 1 );   // frees temporary objects.
	}
}

CProdPic *CTASelectApp::GetpProdPic( LPCTSTR DescriptorID, CEnBitmap *pEnBmp )
{
	// Create the function GetpPic that comes from the HydronicPic Dll or the localized Dll.
	pGetProdPic GetpPic;

	// If the local dll is loaded and the user have choosen to see his localized symbol load the function in the localized dll.
	if( true == IsLocalProdPicDllLoaded() && CProdPic::PicLocSymb == m_eProdPicType )
	{
		GetpPic = ( pGetProdPic )GetProcAddress( m_hmLocProdPics, "GetLocProdPic" );
	}
	else
	{
		GetpPic = ( pGetProdPic )GetProcAddress( m_hmProdPics, "GetProdPic" );
	}

	// Create the ProdPic pointer.
	CProdPic *pProdPic = GetpPic( DescriptorID, m_eProdPicType );

	// If the PicType doesn't exist, load the TASymbolType pic instead...
	bool fPicExist = true;

	if( NULL == pProdPic )
	{
		fPicExist = false;
		GetpPic = ( pGetProdPic )GetProcAddress( m_hmProdPics, "GetProdPic" );
		pProdPic = GetpPic( DescriptorID, CProdPic::PicTASymb );
	}

	if( NULL == pProdPic )
	{
		return NULL;
	}

	// If the pointer exist load the image...
	if( NULL != pEnBmp )
	{
		if( true == fPicExist && IsLocalProdPicDllLoaded() && CProdPic::PicLocSymb == m_eProdPicType )
		{
			pEnBmp->LoadImage( pProdPic->GetResID(), _T("GIF"), m_hmLocProdPics );
		}
		else
		{
			pEnBmp->LoadImage( pProdPic->GetResID(), _T("GIF"), m_hmProdPics );
		}
	}

	return pProdPic;
}

CDynCircSch *CTASelectApp::GetDynCircSch( LPCTSTR DescriptorID )
{
	// Create the function GetpScheme that comes from the HydronicPic Dll.
	pGetDynCircSch GetpScheme;

	// load the function in the HydronicPic Dll.
	GetpScheme = ( pGetDynCircSch )GetProcAddress( m_hmProdPics, "GetDynCircSch" );

	// Create the ProdPic pointer.
	CDynCircSch *pCirScheme = GetpScheme( DescriptorID );

	if( NULL == pCirScheme )
	{
		ASSERT( 0 );
	}

	return pCirScheme;
}

bool CTASelectApp::VerifyPicExist( LPCTSTR DescriptorID )
{
	pVerifyImgFound pVerif;
	pVerif = ( pVerifyImgFound )GetProcAddress( m_hmProdPics, "VerifyImgFound" );
	return pVerif( DescriptorID );
}

void CTASelectApp::OverridedSetRedraw( CWnd *pWnd, BOOL fSetRedraw )
{
	// Pay attention: CWnd::SetRedraw(TRUE) on an invisible object may cause it to become visible. And you don't want it !!!
	//	if( TRUE == pWnd->IsWindowVisible() )
	pWnd->SetRedraw( fSetRedraw );
}

bool CTASelectApp::StartDebugConsole( int iWidth, int iHeight, const TCHAR *pszfname )
{
	//#ifdef _DEBUG
	if( NULL != pszfname && NULL == m_stdFileDebug.m_pStream )
	{
		m_stdFileDebug.Open( pszfname, CFile::modeCreate | CFile::modeWrite | CFile::typeText );
	}

	if( NULL != m_hStdOut )
	{
		return false;
	}

	AllocConsole();
	SetConsoleTitle( _T("Debug Window") );
	m_hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );

	COORD co = { (SHORT)iWidth, (SHORT)iHeight };
	SetConsoleScreenBufferSize( m_hStdOut, co );

	co.X = 0;
	co.Y = 0;
	SetConsoleCursorPosition( m_hStdOut, co );
	return true;
	//#endif
	return false;
}

void CTASelectApp::CloseDebugConsole()
{
	if( m_stdFileDebug.m_pStream != NULL )
	{
		m_stdFileDebug.Close();
	}
}

void CTASelectApp::DebugNewLine( void )
{

	//#ifdef _DEBUG
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

	if( FALSE == GetConsoleScreenBufferInfo( m_hStdOut, &csbiInfo ) )
	{
		return;
	}

	csbiInfo.dwCursorPosition.X = 0;

	// If it is the last line in the screen buffer, scroll the buffer up...
	if( ( csbiInfo.dwSize.Y - 1 ) == csbiInfo.dwCursorPosition.Y )
	{
		SMALL_RECT srctWindow;

		if( csbiInfo.srWindow.Top > 0 )
		{
			srctWindow.Top = -1;		// Move top up by one row.
			srctWindow.Bottom = -1;		// Move bottom up by one row
			srctWindow.Left = 0;		// No change.
			srctWindow.Right = 0;		// No change.

			if( FALSE == SetConsoleWindowInfo(
					m_hStdOut,				// Screen buffer handle.
					FALSE,					// Deltas, not absolute.
					&srctWindow ) )			// Specifies new location.
			{
				return;
			}
		}
	}
	else
	{
		// Otherwise, advance the cursor to the next line.
		csbiInfo.dwCursorPosition.Y += 1;
	}

	if( FALSE == SetConsoleCursorPosition( m_hStdOut, csbiInfo.dwCursorPosition ) )
	{
		return;
	}

	//#endif
}

void CTASelectApp::DebugPrintf( CString s )
{
	//#ifdef _DEBUG
	DWORD dwCharsWritten;

	if( NULL != m_hStdOut )
	{
		DebugNewLine();
		WriteConsole( m_hStdOut, s, s.GetLength(), &dwCharsWritten, NULL );
	}

	if( m_stdFileDebug.m_pStream != NULL )
	{
		CString str = s;
		str += _T("\r\n");
		m_stdFileDebug.WriteString( str );
	}

	//#endif
}

int CTASelectApp::Run()
{
#ifndef _DEBUG
	// Initialize the handlers to manage all exceptions.
	clHySelectExceptionHandler.SetProcessExceptionHandlers();
	clHySelectExceptionHandler.SetThreadExceptionHandlers();

	try
	{
		return CWinAppEx::Run();
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectExceptionHandler.HyselectExceptionHandler( clHySelectException );	
	}
#else
	return CWinAppEx::Run();
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CTASelectApp::PreLoadState()
{
	GetContextMenuManager()->AddMenu( _T("IDR_FLTMENU_BATCHSEL"), IDR_FLTMENU_BATCHSEL );
}

BOOL CTASelectApp::InitInstance()
{
	try
	{
		//TODO: call AfxInitRichEdit2() to initialize richedit2 library.
		// In case the application restart itself after a language switch, verify the previous instance is finished.
		if( TRUE == RA_CheckForRestartProcessStart() ) 
		{
			RA_WaitForPreviousProcessFinish();
		}

		if( FALSE == InitializeCriticalSectionAndSpinCount( &CriticalSection, 0x80000400 ) )
		{
			HYSELECT_THROW( _T("Internal error: Can't initialize 'CriticalSection'.") );
		}

#ifndef DEBUG

		// Prevent multiple instances of HySelect
		m_pInstanceEvent = new CEvent( FALSE, FALSE, SYNCHOBJ );
		DWORD dwError = ::GetLastError();

		if( NULL == m_pInstanceEvent )
		{
			HYSELECT_THROW( _T("Internal error: Can't create the 'm_pInstanceEvent' event.") );
		}

		if( ERROR_ALREADY_EXISTS == dwError )
		{
			HYSELECT_THROW( _T("HySelect is already running.") );
		}

		m_pShutdownEvent = new CEvent();

		if( NULL == m_pShutdownEvent )
		{
			HYSELECT_THROW( _T("Internal error: Can't create the 'm_pShutdownEvent' event.") );
		}
		
		m_pActivationWatch = ( CActivationWatcher * )AfxBeginThread( RUNTIME_CLASS( CActivationWatcher ) );

		if( NULL == m_pShutdownEvent )
		{
			HYSELECT_THROW( _T("Internal error: Can't start the 'm_pActivationWatch' thread.") );
		}
		
#endif

		// Statical instantiate have some compilation problems.
		m_pTADB = new CTADatabase();

		if( NULL == m_pTADB )
		{
			HYSELECT_THROW( _T("Internal error: Can't instantiate the 'm_pTADB' CTADatabase object.") );
		}

		m_pUSERDB = new CUserDatabase();

		if( NULL == m_pUSERDB )
		{
			HYSELECT_THROW( _T("Internal error: Can't instantiate the 'm_pUSERDB' CUserDatabase object.") );
		}

		m_pPIPEDB = new CPipeUserDatabase();

		if (NULL == m_pPIPEDB)
		{
			HYSELECT_THROW( _T("Internal error: Can't instantiate the 'm_pPIPEDB' CPipeUserDatabase object.") );
		}

		m_pTADS = new CTADatastruct();

		if( NULL == m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: Can't instantiate the 'm_pTADS' CTADatastruct object.") );
		}

		m_bDebugCom = false;
		m_bLogRawData = false;
		m_bUpdateRegistryStartingFolder = true;

		// Catching starting parameters.
		CStringArray StartingParameters;

		for( int i = 0; i < __argc; i++ )
		{
			CString str = CString( __targv[i] );
			StartingParameters.Add( str );
		}

		if( StartingParameters.GetCount() > 1 )
		{
			if( _T("Register") == StartingParameters[1] )
			{
				// We know that HySelect is called by DBBuilder to build databse. If there is an error with the "pipe.db" or "user.db" files
				// we don't show a message but only an error and we exit.
				m_bRegisterRunning = true;
			}
		}

		// Get the directory of application.
		// Needed for starting HySelect from a tsp file.
		CString sAppDir = __targv[0];
		CPath *pPath = new CPath( sAppDir );
		sAppDir = pPath->SplitPath( ( CPath::ePathFields )( CPath::epfDrive + CPath::epfDir ) );
		delete pPath;

		CFileStatus fs;
		m_bStartingWOInstal = false;

		if( TRUE == CFile::GetStatus( _T("HySelect.ini"), fs ) )
		{
			m_bStartingWOInstal = true;
		}

		// In development (debug/release) mode don't change the default folder.
		// Except if a specific folder is specified...
		sAppDir.MakeLower();

		if( sAppDir.Find( _T("taselect\\debug" ) ) < 0 && sAppDir.Find( _T( "taselect\\release") ) < 0 )
		{
			_tchdir( ( LPCTSTR )sAppDir );
		}
		else
		{
			m_bUpdateRegistryStartingFolder = false;
		}

		_PrepareDocumentsFolderForDoc();
		_PrepareDocumentsFolderForHelp( sAppDir );

		///////////////////////////////////////////////////////////////
		// Compose a new application name that depends on language
		char buf[10];
		memset( buf, 0, sizeof( buf ) );

		if( TRUE == CFile::GetStatus( _SUBLANGUAGE_FILENAME, fs ) )
		{
			CFile inpf( _SUBLANGUAGE_FILENAME, CFile::modeRead | CFile::typeBinary );
			inpf.Read( buf, sizeof( buf ) );
			inpf.Close();
		}
		else
		{
			HYSELECT_THROW( _T("'TADB.key' file is missing in the HySelect installation folder.") );
		}

		// Skip unused character.
		m_strLanguageKey = ( CString )&buf[0];
		m_strLanguageKey.TrimRight();
		m_strLanguageKey.TrimLeft();

		if( true == m_strLanguageKey.IsEmpty() )
		{
			CString str;
			str.Format( _T("Internal error: %s"), TASApp.LoadLocalizedString( IDS_CHECK_SUBLANGKEY ) );
			throw CHySelectException( str, __LINE__, __FILE__ );
		}

		// Parse command line for standard shell commands, DDE, file open.
		CCommandLineInfo cmdInfo;
		ParseCommandLine( cmdInfo );

		/////////////////////////////////////////////////////////////////
		// Change the registry key under which settings are stored.
		CString tmp = CRegAccess::GetTASRegistryKey();
		SetRegistryKey( ( LPCTSTR )tmp );

		CRegAccess::VerifyAndRepairQATRegistryEntry();

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Init the project directory, if doesn't exist.
		CString ProjDir( GetProfileString( _T("Files" ), _T( "Project Directory" ), _T( "") ) );

		if( true == ProjDir.IsEmpty() )
		{
			AfxGetApp()->WriteProfileString( _T("Files" ), _T( "Project Directory"), _PROJECTS_DIRECTORY );
		}

		// Get information about Maintaining TAScope files.
		m_uiMaintainTAScopeFiles = GetProfileInt( _T("TA-SCOPE" ), _T( "Maintain files"), 1 );
		WriteProfileInt( L"TA-SCOPE", L"Maintain files", m_uiMaintainTAScopeFiles );

		// Get information about the ProdPicType.
		UINT uiPicType = GetProfileInt( L"DialogTechParam", L"Valve image type", CProdPic::Pic );
		SetProdPicType( ( CProdPic::eProdPicType )uiPicType );

		// Retrieve the installation directory.
		TCHAR szDir[MAX_PATH] = _T( "" );

		if( 0 == ::GetCurrentDirectory( sizeof( szDir ) - 1, szDir ) )
		{
			// ::GetLastError()
		}

		m_strStartDir = CString( szDir ) + _T("\\");
		m_strStartDir.MakeLower();

		// JEFF VANZETTEN modification, set flag m_TBVCMAuthCheat to true if file "TBVCMAUTH.bin" exist.
		// If flag is true TBV-CM authority is computed on the full Cv Dp and not only on control part Dp.
		CFileFind finder;
		BOOL bFound = finder.FindFile( m_strStartDir + _T("TbvCMAuth.bin") );
		m_bTBVCMAuthOnFullDp = ( TRUE == bFound ) ? true : false;

		/////////////////////////////////////////////////////////////////
		// Load Graphical resources DLLs
		m_hmHubSchemes = NULL;
		m_hmProdPics = NULL;
		m_hmLocProdPics = NULL;
		
		m_hmProdPics = LoadLibrary( _HYDRO_PICS );
		DWORD dw = GetLastError();

		if( NULL == m_hmProdPics )
		{
			TASApp.AfxLocalizeMessageBox( AFXMSG_DLLLOADINGFAILED, MB_OK | MB_ICONSTOP );

			if( NULL != m_hmProdPics )
			{
				FreeLibrary( m_hmProdPics );
			}

			if( NULL != m_hmLocProdPics )
			{
				FreeLibrary( m_hmLocProdPics );
			}

			throw CHySelectException( TASApp.LoadLocalizedString( AFXMSG_DLLLOADINGFAILED ), __LINE__, __FILE__ );
		}

		/////////////////////////////////////////////////////////////////
		// Read the database.
		// If the text file is available read it instead of the binary file.
		// Try to read it from the working directory first. If it is not possible,
		// try from the installation dir.
		m_bReadTADBTxt = false;

		if( true == ReadTADBTxT() )
		{
			m_bReadTADBTxt = true;

			// When we create TADB from TXT files we will also create DB for TAScope.
			GenerateTASCopeDBX();
		}
		else if( TRUE == CFile::GetStatus( _DATABASE_BIN_FILE, fs ) )
		{
			std::ifstream inpf( _DATABASE_BIN_FILE, std::ifstream::in | std::ifstream::binary );
		
			if( std::ifstream::failbit == ( inpf.rdstate() & std::ifstream::failbit ) )
			{
				HYSELECT_THROW( _T("Can't open the '%s' file."), _DATABASE_BIN_FILE );
			}
		
			m_pTADB->Read( inpf );
			inpf.close();

			// Everything has worked.
			// The current directory is thus the application directory !
			TCHAR dir[MAX_PATH];
			int iLength = GetCurrentDirectory( MAX_PATH, dir );

			if( iLength > 0 )
			{
				_tcsncpy_s( dir + iLength, MAX_PATH - iLength - 1, _T("\\"), MAX_PATH - iLength - 2 );
				m_strStartDir = dir;

				if( true == m_bUpdateRegistryStartingFolder )
				{
					WriteProfileString( _T("Files" ), _T( "Installation Directory"), dir );
				}
			}
		}
		else if( TRUE == CFile::GetStatus( m_strStartDir + _DATABASE_BIN_FILE, fs ) )
		{
			CString strFile = m_strStartDir + _DATABASE_BIN_FILE;
			std::ifstream inpf( strFile, std::ifstream::in | std::ifstream::binary );
		
			if( std::ifstream::failbit == ( inpf.rdstate() & std::ifstream::failbit ) )
			{
				HYSELECT_THROW( _T("Can't open the '%s' file."), strFile );
			}
		
			m_pTADB->Read( inpf );
			inpf.close();
		}
		else
		{
			m_pTADB->Modified( FALSE );
			throw CHySelectException( TASApp.LoadLocalizedString( AFXMSG_FAIL_OPEN_DBFILE ), __LINE__, __FILE__ );
		}

		CString strUSERDBPath = TASApp.GetDataPath();

		// Create UserData directory if needed.
		int iError = SHCreateDirectoryEx( NULL, ( LPCTSTR )strUSERDBPath, NULL );

		if( ERROR_SUCCESS == iError || ERROR_FILE_EXISTS == iError || ERROR_ALREADY_EXISTS == iError )
		{
			// If User.db doesn't exist in this folder copy from installation dir if possible.
			CString UserDB = strUSERDBPath + CString( _T("user.db") );

			if( TRUE == CFile::GetStatus( UserDB, fs ) )
			{
				std::ifstream inpf( ( LPCTSTR )UserDB, std::ifstream::in | std::ifstream::binary );

				if( std::ifstream::failbit == ( inpf.rdstate() & std::ifstream::failbit ) )
				{
					HYSELECT_THROW( _T("Can't open the '%s' file."), UserDB );
				}
				
				short nVersion;
				short nUserDbVersion;
				inpf.read( ( char * )&nUserDbVersion, sizeof( nUserDbVersion ) );
				inpf.close();

				_string str = _string( ( ( CDB_MultiString * )GetpTADB()->Get( _T("TADB_VERSION") ).MP )->GetString( 0 ) );
				RemoveTChar( &str, '.' );
				from_string( str, nVersion );

				// If it's ok...
				try
				{
					// If compatible database version of the current HySelect installed is bigger or equal to the user database version...
					if( nVersion >= nUserDbVersion )
					{
						inpf.open( ( LPCTSTR )UserDB, std::ifstream::in | std::ifstream::binary );
						
						if( std::ifstream::failbit == ( inpf.rdstate() & std::ifstream::failbit ) )
						{
							HYSELECT_THROW( _T("Can't open the '%s' file."), UserDB );
						}

						m_pUSERDB->Read( inpf );
						inpf.close();
					}
					else
					{
						HYSELECT_THROW( _T("User database version (%i) can't be bigger than the supported database version (%i) of the current HySelect installed."), nUserDbVersion, nVersion );
					}
				}
				catch( CHySelectException &clHySelectException )
				{
					// We don't need here the messages. And here we call the 'ClearMessages' method to avoid a compilation warning.
					clHySelectException.ClearMessages();

					// Corrupted USER.db ?????
					int iError = IDOK;

					if( false == m_bRegisterRunning )
					{
						iError = AfxMessageBox( TASApp.LoadLocalizedString( IDS_ERRORCORRUPTEDUSERDB ), MB_OKCANCEL );
					}

					if( IDOK == iError )
					{
						m_pUSERDB->Init();

						// Take a copy of corrupted file ".bad" extension.
						CString FileSave = UserDB + _T(".bad");
						FileMoveCopy( UserDB, FileSave, true );

						if( false == m_bRegisterRunning && FALSE == RA_ActivateRestartProcess() )
						{
							// Handle restart error here.
							return FALSE;
						}
					}

					// We intentionally call 'return' instead of 'throw' because we want here that HySelect restart with the new pipe database file.
					return FALSE;
				}
			}
			else if( CFile::GetStatus( _USER_DATABASE_BIN_FILE, fs ) )
			{
				std::ifstream inpf( _USER_DATABASE_BIN_FILE, std::ifstream::in | std::ifstream::binary );

				if( std::ifstream::failbit == ( inpf.rdstate() & std::ifstream::failbit ) )
				{
					HYSELECT_THROW( _T("Can't open the '%s' file."), _USER_DATABASE_BIN_FILE );
				}

				m_pUSERDB->Read( inpf );
				inpf.close();
			}
			else if( CFile::GetStatus( m_strStartDir + _USER_DATABASE_BIN_FILE, fs ) )
			{
				CString strFile = m_strStartDir + _USER_DATABASE_BIN_FILE;
				std::ifstream inpf( strFile, std::ifstream::in | std::ifstream::binary );

				if( std::ifstream::failbit == ( inpf.rdstate() & std::ifstream::failbit ) )
				{
					HYSELECT_THROW( _T("Can't open the '%s' file."), strFile );
				}

				m_pUSERDB->Read( inpf );
				inpf.close();
			}

			if( FALSE == LoadPipesFromFile() )
			{
				// We intentionally call 'return' instead of 'throw' because we want here that HySelect restart with the new pipe database file
				// if there is an error with the file.
				return FALSE;
			}

		}

		// Recover default units from registry or from localized defaults units if the registry doesn't exist yet.
		CTADatabase *pTADB = ( ( CTASelectApp * )AfxGetApp() )->m_pTADB;
		CDB_DefaultUnits *pLocalUnits = ( CDB_DefaultUnits * )( pTADB->Get( _T("DEFAULT_UNITS") ).MP );
		CDimValue dv;
		CUnitDatabase *pDB = dv.AccessUDB();

		for( int i = 0; i < _UNITS_NUMBER_OF; i++ )
		{
			pDB->SetDefaultUnit( i, ( int )GetProfileInt( _T("Default Units"), _UNIT_STRING_ID[i], pLocalUnits->GetLocalizedDefaultUnits( i ) ) );
		}

		LoadFlags();

		// Read TadbST.xml.
		// Stringtab is a resource text file that contains all strings for TADB.TXT.
		if( TRUE == CFile::GetStatus( _T( _STRINGTAB_XML_FILE ), fs ) )
		{
			_string lang = ( LPCTSTR )GetLanguage();
			XmlStrTab.Init( std::string( _STRINGTAB_XML_FILE ), lang );
		}

		// Read TasRC.xml.
		// TasRC is a resource text file that contains all strings from the TASelect.rc.
		if( TRUE == CFile::GetStatus( _T( _TASRC_XML_FILE ), fs ) )
		{
			_string lang = ( LPCTSTR )GetLanguage();
			XmlRcTab.Init( std::string( _TASRC_XML_FILE ), lang );
		}

		// Read TasTips.xml.
		// TasTIP is a resource text file that contains all strings for "the tips of the day".
		if( TRUE == CFile::GetStatus( _T( _TASTIPS_XML_FILE ), fs ) )
		{
			_string lang = ( LPCTSTR )GetLanguage();
			XmlTipTab.Init( std::string( _TASTIPS_XML_FILE ), lang );
		}

		// Recover Currency code from registry.
		// HYS-1138: do not read anymore currency from registry. We can then delete this one.
		tmp = CRegAccess::GetTASRegistryKey();
		CString tmpKey = _T("Software\\") + tmp + _T("\\HySelect");
		HKEY hKey;
		LONG lResult = RegOpenKeyEx( HKEY_CURRENT_USER, tmpKey, 0, KEY_READ, &hKey );

		if( ERROR_SUCCESS == lResult )
		{
			DelRegTree( hKey, _T("Currency") );
		}

		// Correct check box for AU and GB version.
		//AU and GB version replaced by en version since 4.1.1.0.
		if( 0 == m_strLanguageKey.CompareNoCase( _T("AU") ) )
		{
			int iCheck = GetProfileInt( _T("Documentation" ), _T( "en-AU"), 0 );

			if( 0 != iCheck )
			{
				::AfxGetApp()->WriteProfileInt( _T("Documentation" ), _T( "en"), 1 );
				// Only one times clear old flag.
				::AfxGetApp()->WriteProfileInt( _T("Documentation" ), _T( "en-AU"), 0 );
			}
		}
		else if( 0 == m_strLanguageKey.CompareNoCase( _T("GB") ) )
		{
			int iCheck = ( BOOL )::AfxGetApp()->GetProfileInt( _T("Documentation" ), _T( "en-GB"), 0 );

			if( 0 != iCheck )
			{
				::AfxGetApp()->WriteProfileInt( _T("Documentation" ), _T( "en"), 1 );
				// Only one times clear old flag.
				::AfxGetApp()->WriteProfileInt( _T("Documentation" ), _T( "en-GB"), 0 );
			}
		}

		// *** Initialize the docdb class  ***
		// ***********************************
		CString DocumentFolder = TASApp.GetDocumentsFolderForDoc();
		CString DocDbPath;
		DocDbPath.Format( _DOCDB_CSV_FILE, TASApp.GetDocLanguage() );
		m_DocDb.SetCsvPath( DocumentFolder + CString( _HYSELECT_NAME_BCKSLASH ) + DocDbPath );

		// HYS-1057.
		if( 0 != GetDocLanguage().CompareNoCase( _T("en") ) )
		{
			m_DocDbEn.SetCsvPathEn( DocumentFolder + CString( _HYSELECT_NAME_BCKSLASH ) + _T("doc\\en\\docdb.csv") );
		}

		// Create some tables (PAGE_SETUP_TAB, PAGE_FIELD_TAB, VARIOUS_TAB, GLOSSARY_TAB and RADNOMCOND_TAB).
		// And fill m_PIPEDB with pipes that are in m_USERDB (For old HySelect versions) and from the HySelect database.
		PrepareUserDb();

		// First initialization needed before Ribbon bar initialization.
		GetpTADS()->Init();

		m_bAppStarted = true;

		// InitCommonControlsEx() needed for Windows�XP for Visual style with ComCtl32.dll version�6.
		INITCOMMONCONTROLSEX InitCtrls;
		InitCtrls.dwSize = sizeof( InitCtrls );

		// For including commons controls.
		InitCtrls.dwICC = ICC_WIN95_CLASSES;

		if( FALSE == InitCommonControlsEx( &InitCtrls ) )
		{
			HYSELECT_THROW( _T("Internal error: Can't initialize the common control DLL.") );
		}

		if( FALSE == CWinAppEx::InitInstance() )
		{
			HYSELECT_THROW( _T("Internal error: Can't instantiate the application. ") );
		}

		if( FALSE == AfxOleInit() )
		{
			throw CHySelectException( TASApp.LoadLocalizedString( IDS_OLE_INIT_FAILED ), __LINE__, __FILE__ );
		}

		EnableLoadWindowPlacement( FALSE );

		/////////////////////////////////////////////////////////////////////////
		// Standard initialization
		// If you are not using these features and wish to reduce the size
		//  of your final executable, you should remove from the following
		//  the specific initialization routines you do not need.

		LoadStdProfileSettings();  // Load standard INI file options

		if( FALSE == InitContextMenuManager() )
		{
			HYSELECT_THROW( _T("Internal error: Can't initialize the 'CContextMenuManager' object.") );
		}

		if( FALSE == InitKeyboardManager() )
		{
			HYSELECT_THROW( _T("Internal error: Can't initialize the 'CKeyboardManager' object.") );
		}

		if( FALSE == InitTooltipManager() )
		{
			HYSELECT_THROW( _T("Internal error: Can't initialize the 'CToolTipManager' object.") );
		}

		CMFCToolTipInfo ttParams;
		ttParams.m_bVislManagerTheme = TRUE;
		TASApp.GetTooltipManager()->SetTooltipParams( AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS( CMFCToolTipCtrl ), &ttParams );

		CSingleDocTemplate *pDocTemplate;
		pDocTemplate = new CSingleDocTemplate(
			IDR_MAINFRAME,
			RUNTIME_CLASS( CTASelectDoc ),
			RUNTIME_CLASS( CMainFrame ),
			RUNTIME_CLASS( CTASelectView ) );

		if( NULL == pDocTemplate )
		{
			HYSELECT_THROW( _T("Internal error: Can't instantiate the 'CSingleDocTemplate' object.") );
		}

		AddDocTemplate( pDocTemplate );

		// Enable DDE Execute open.
		EnableShellOpen();
		RegisterShellFileTypes( TRUE );

		// Update registry with ftp parameters.
		m_bAutoUpdate = false;
		CString strMsg;
		CDB_MultiString *pmsFtpServerParams = ( CDB_MultiString * )( m_pTADB->Get( _T("FTPSERVER_PARAM") ).MP );

		if( FALSE == WriteProfileString( HYUPDATE, FTPSERVER, pmsFtpServerParams->GetString( 0 ) ) )
		{
			DWORD dwError = GetLastError();
			strMsg.Format( _T("Internal error: Error when writing in registry (Code: 0x%08X"), dwError );
			throw CHySelectException( strMsg, __LINE__, __FILE__ );
		}

		if( FALSE == WriteProfileString( HYUPDATE, FTPLOGIN, pmsFtpServerParams->GetString( 1 ) ) )
		{
			DWORD dwError = GetLastError();
			strMsg.Format( _T("Internal error: Error when writing in registry (Code: 0x%08X"), dwError );
			throw CHySelectException( strMsg, __LINE__, __FILE__ );
		}

		if( FALSE == WriteProfileString( HYUPDATE, FTPSERVERPSW, pmsFtpServerParams->GetString( 2 ) ) )
		{
			DWORD dwError = GetLastError();
			strMsg.Format( _T("Internal error: Error when writing in registry (Code: 0x%08X"), dwError );
			throw CHySelectException( strMsg, __LINE__, __FILE__ );
		}

		if( FALSE == WriteProfileString( HYUPDATE, HTTPSERVER, pmsFtpServerParams->GetString( 3 ) ) )
		{
			DWORD dwError = GetLastError();
			strMsg.Format( _T("Internal error: Error when writing in registry (Code: 0x%08X"), dwError );
			throw CHySelectException( strMsg, __LINE__, __FILE__ );
		}

		if( FALSE == WriteProfileString( HYUPDATE, HTTPLOGIN, pmsFtpServerParams->GetString( 4 ) ) )
		{
			DWORD dwError = GetLastError();
			strMsg.Format( _T("Internal error: Error when writing in registry (Code: 0x%08X"), dwError );
			throw CHySelectException( strMsg, __LINE__, __FILE__ );
		}

		if( FALSE == WriteProfileString( HYUPDATE, HTTPSERVERPSW, pmsFtpServerParams->GetString( 5 ) ) )
		{
			DWORD dwError = GetLastError();
			strMsg.Format( _T("Internal error: Error when writing in registry (Code: 0x%08X"), dwError );
			throw CHySelectException( strMsg, __LINE__, __FILE__ );
		}

		// Test Windows system compatibility before 6 (Vista) access to "tahc_software"
		CString strSubFolder = pmsFtpServerParams->GetString( 6 );

		if( TRUE == IsWindowsVistaOrGreater() )
		{
			// If a sub-folder is declared for supporting Windows version 6 and above use it.
			CString strSubFolderWinV6 = pmsFtpServerParams->GetString( 7 );

			if( false == strSubFolderWinV6.IsEmpty() )
			{
				strSubFolder = strSubFolderWinV6;
			}
		}

		if( FALSE == WriteProfileString( HYUPDATE, WEBSERVERDIR, strSubFolder ) )
		{
			return false;
		}

		CDB_MultiString *pmsFtpGatewayParams = ( CDB_MultiString * )( m_pUSERDB->Get( _T("FTPGATEWAY_PAR") ).MP );

		if( pmsFtpGatewayParams->GetNumofLines() < CDlgGateway::egwLast )
		{
			// Old Version of FTPGATEWAY_PAR, reset all.
			pmsFtpGatewayParams->DeleteAllLines();
			pmsFtpGatewayParams->SetString( CDlgGateway::egwFtpAutoUpdate, _T("True") );
			pmsFtpGatewayParams->SetString( CDlgGateway::egwUsed, _T("False") );
			pmsFtpGatewayParams->SetString( CDlgGateway::egwFtpAdd, _T( "" ) );
			pmsFtpGatewayParams->SetString( CDlgGateway::egwFtpPsw, _T( "" ) );
			pmsFtpGatewayParams->SetString( CDlgGateway::egwFtpPort, _T("21") );
			pmsFtpGatewayParams->SetString( CDlgGateway::egwHttpAdd, _T( "" ) );
			pmsFtpGatewayParams->SetString( CDlgGateway::egwHttpPsw, _T( "" ) );
			pmsFtpGatewayParams->SetString( CDlgGateway::egwHttpPort, _T("80") );
		}

		if( FALSE == WriteProfileInt( HYUPDATE, GATEWAYUSED, _tcsicmp( pmsFtpGatewayParams->GetString( CDlgGateway::egwUsed ), _T("true") ) ? 0 : 1 ) )
		{
			DWORD dwError = GetLastError();
			strMsg.Format( _T("Internal error: Error when writing in registry (Code: 0x%08X"), dwError );
			throw CHySelectException( strMsg, __LINE__, __FILE__ );
		}

		if( FALSE == WriteProfileString( HYUPDATE, FTPGATEWAY, pmsFtpGatewayParams->GetString( CDlgGateway::egwFtpAdd ) ) )
		{
			DWORD dwError = GetLastError();
			strMsg.Format( _T("Internal error: Error when writing in registry (Code: 0x%08X"), dwError );
			throw CHySelectException( strMsg, __LINE__, __FILE__ );
		}

		if( FALSE == WriteProfileString( HYUPDATE, FTPGATEWAYPSW, pmsFtpGatewayParams->GetString( CDlgGateway::egwFtpPsw ) ) )
		{
			DWORD dwError = GetLastError();
			strMsg.Format( _T("Internal error: Error when writing in registry (Code: 0x%08X"), dwError );
			throw CHySelectException( strMsg, __LINE__, __FILE__ );
		}

		if( FALSE == WriteProfileInt( HYUPDATE, FTPPORT,
									  *pmsFtpGatewayParams->GetString( CDlgGateway::egwFtpPort ) ?  _ttoi( pmsFtpGatewayParams->GetString( CDlgGateway::egwFtpPort ) ) : 21 ) )
		{
			DWORD dwError = GetLastError();
			strMsg.Format( _T("Internal error: Error when writing in registry (Code: 0x%08X"), dwError );
			throw CHySelectException( strMsg, __LINE__, __FILE__ );
		}

		if( FALSE == WriteProfileString( HYUPDATE, HTTPGATEWAY, pmsFtpGatewayParams->GetString( CDlgGateway::egwHttpAdd ) ) )
		{
			DWORD dwError = GetLastError();
			strMsg.Format( _T("Internal error: Error when writing in registry (Code: 0x%08X"), dwError );
			throw CHySelectException( strMsg, __LINE__, __FILE__ );
		}

		if( FALSE == WriteProfileString( HYUPDATE, HTTPGATEWAYPSW, pmsFtpGatewayParams->GetString( CDlgGateway::egwHttpPsw ) ) )
		{
			DWORD dwError = GetLastError();
			strMsg.Format( _T("Internal error: Error when writing in registry (Code: 0x%08X"), dwError );
			throw CHySelectException( strMsg, __LINE__, __FILE__ );
		}

		if( FALSE == WriteProfileInt( HYUPDATE, HTTPPORT,
									  *pmsFtpGatewayParams->GetString( CDlgGateway::egwHttpPort ) ? _ttoi( pmsFtpGatewayParams->GetString( CDlgGateway::egwHttpPort ) ) : 80 ) )
		{
			DWORD dwError = GetLastError();
			strMsg.Format( _T("Internal error: Error when writing in registry (Code: 0x%08X"), dwError );
			throw CHySelectException( strMsg, __LINE__, __FILE__ );
		}

		// Auto Update flag must be modified by HyUpdate, so update TADS with value stored in registry if this value exist.
		UINT uiAutoUpdateFlag = GetProfileInt( HYUPDATE, AUTOUPDATE, 0xFFFF );

		// If 'Auto Update' not yet defined, write default value 1...
		if( 0xFFFF == uiAutoUpdateFlag )
		{
			if( 0 == _tcsicmp( pmsFtpGatewayParams->GetString( CDlgGateway::egwFtpAutoUpdate ), _T("true") ) )
			{
				uiAutoUpdateFlag = 1;
			}
			else if( 0 == _tcsicmp( pmsFtpGatewayParams->GetString( CDlgGateway::egwFtpAutoUpdate ), _T("fixedtime") ) )
			{
				uiAutoUpdateFlag = 2;
			}
			else
			{
				uiAutoUpdateFlag = 0;
			}

			if( FALSE == WriteProfileInt( HYUPDATE, AUTOUPDATE, uiAutoUpdateFlag ) )
			{
				DWORD dwError = GetLastError();
				strMsg.Format( _T("Internal error: Error when writing in registry (Code: 0x%08X"), dwError );
				throw CHySelectException( strMsg, __LINE__, __FILE__ );
			}
		}

		if( 1 == uiAutoUpdateFlag )
		{
			pmsFtpGatewayParams->SetString( CDlgGateway::egwFtpAutoUpdate, _T("true") );
		}
		else if( 0 == uiAutoUpdateFlag )
		{
			pmsFtpGatewayParams->SetString( CDlgGateway::egwFtpAutoUpdate, _T("false") );
		}
		else
		{
			pmsFtpGatewayParams->SetString( CDlgGateway::egwFtpAutoUpdate, _T("fixedtime") );
		}

		// Instantiate the resource image manager.
		m_pclRCImageManager = new CRCImageManager();

		if( NULL == m_pclRCImageManager )
		{
			HYSELECT_THROW( _T("Internal error: 'm_pclRCImageManager' instantiation error.") );
		}

		////////////////////////////////////////////////////////////////////////////
		// Retrieve the HyUpdate installation directory
		m_strHyUpdate_InstDir.Empty();

		// In release mode '_TAS_NAME' is 'HySelect'.
		// The directory of installation is named as 'c:\Program Files (x86)\IMI Hydronic Engineering\HySelect-INT' for example.
		CString Hyselectname = _HYSELECT_NAME;
		Hyselectname.MakeLower();
		m_strStartDir.MakeLower();

		int iTasPos = m_strStartDir.Find( Hyselectname );

		if( iTasPos > 0 )
		{
			// To retrieve update directory, we remove the 'HySelect-INT' part to keep only 'c:\Program Files (x86)\IMI Hydronic Engineering\'.
			CString rootpath = m_strStartDir.Left( iTasPos );

			// The HyUpdate directory is 'c:\Program Files (x86)\IMI Hydronic Engineering\HyUpdate-INT\' for our example.
			m_strHyUpdate_InstDir = rootpath + HYUPDATE + _T("-" ) + m_strLanguageKey + _T( "\\");
		}

		bool bInstallDoc = false;

		// [0] application name
		// [1]...[n] parameters
		if( StartingParameters.GetCount() > 1 )
		{
			// When command line contain register just update information into the registry and close the application.
			if( _T("Register") == StartingParameters[1] )
			{
				return FALSE;
			}
			else if( _T("rawdata") == StartingParameters[1] )
			{
				m_bLogRawData = true;
				cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
				cmdInfo.m_strFileName = _T( "" );
			}
			else if( _T("test") == StartingParameters[1] )
			{
				m_fTestInRelease = true;
				cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
				cmdInfo.m_strFileName = _T( "" );
			}
			else if (_T("unit") == StartingParameters[1])
			{
				m_fUnitTest = true;
				cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
				cmdInfo.m_strFileName = _T("");
			}
			else if (_T("wizard") == StartingParameters[1])
			{
				m_bWizardMode = true;
				cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
				cmdInfo.m_strFileName = _T("");
			}
			else if( _T("FilterView") == StartingParameters[1] )
			{
				CreateFilterViewTxT();
				return FALSE;
			}
			else if( _T("DocInstall") == StartingParameters[1] )
			{
				bInstallDoc = true;
			}
/*			else if( _T("CrossVerifyDocDB") == StartingParameters[1] && StartingParameters.GetCount() > 2 )
			{
				CString strDBLanguage = StartingParameters[2];
				_DocDBFindMissingFiles( NULL, strDBLanguage, true );
				_DocDBFindMissingDBEntry( NULL, strDBLanguage, true );
				return FALSE;
			}
*/		}

		if( false == bInstallDoc && false == m_bLogRawData )
		{
			// Verify command line for validity of file.
			if( false == cmdInfo.m_strFileName.IsEmpty() )
			{
				std::ifstream inpf( cmdInfo.m_strFileName, std::ifstream::in | std::ifstream::binary );
				ASSERT( !( inpf.rdstate() & std::ifstream::failbit ) );
				int rCode = CDataBase::CheckDBFile( inpf );
				inpf.close();

				// If something wrong...
				if( rCode < 0 )
				{
					CString str;
					str.Format( _T("%s, %d"), cmdInfo.m_strFileName, rCode );

					if( -1 == rCode )		// Not a correct file
					{
						FormatString( str, AFXMSG_NOTACORRECTFILE, cmdInfo.m_strFileName );
					}
					else if( -2 == rCode )	// Format Version is not correct
					{
						FormatString( str, AFXMSG_TSPFORMATNEWERTHANTAS2, cmdInfo.m_strFileName );
					}

					::AfxMessageBox( str );
					cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
					cmdInfo.m_strFileName = _T( "" );
				}
			}

			/*#ifndef DEBUG
				// Hide splash screen
				Sleep(1000);
				CSplashWindow::HideSplashScreen();
			#endif*/
		}
		else
		{
			cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
			cmdInfo.m_strFileName = _T( "" );
		}

		// HYS-1700: We don't process shell command if it's to open a file (Double-click on a '.tsp' file in the Windows explorer to open it).
		// In this case, we save the file name, we force the shell command to "FileNew" and we will open the file at the end of this method.
		if( CCommandLineInfo::FileOpen == cmdInfo.m_nShellCommand )
		{
			m_strFileToOpen = cmdInfo.m_strFileName;
			cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
			cmdInfo.m_strFileName = _T("");
		}

		if( FALSE == ProcessShellCommand( cmdInfo ) )
		{
			HYSELECT_THROW( _T("Internal error: Error when calling the 'ProcessShellCommand'.") );
		}

		if( true == bInstallDoc )
		{
			// How to hide Main window?
			m_pMainWnd->MoveWindow( 10000, 10000, 10000, 10000 );

			CDlgDocs dlg;
			dlg.Display( true );
			return FALSE;
		}

		// Retrieve previous position, if doesn't exit place HySelect on the primary screen.
		if( FALSE == ReloadWindowPlacement( static_cast<CFrameWnd *>( m_pMainWnd ) ) )
		{
			// Get the width in pixel of the primary screen.
			int CxPhysPrimScreen = GetSystemMetrics( SM_CXSCREEN );
			int CyPhysPrimScreen = GetSystemMetrics( SM_CYSCREEN );

			// The MainFrame will be sized at 5/6 of the primary screen.
			int x = CxPhysPrimScreen / 12;
			int y = CyPhysPrimScreen / 12;
			CxPhysPrimScreen = CxPhysPrimScreen * 5 / 6;
			CyPhysPrimScreen = CyPhysPrimScreen * 5 / 6;
			m_pMainWnd->ShowWindow( m_nCmdShow );
			m_pMainWnd->MoveWindow( x, y, CxPhysPrimScreen, CyPhysPrimScreen );
		}

		// If the window is placed on a screen that is currently deactivated, center the application into the main active screen.
		HMONITOR hMonitor = NULL;
		CRect rect;
		m_pMainWnd->GetWindowRect( &rect );
		hMonitor = MonitorFromRect( &rect, MONITOR_DEFAULTTONULL );

		if( NULL == hMonitor )
		{
			m_pMainWnd->CenterWindow();
		}


		// Enable drag/drop open.
		m_pMainWnd->DragAcceptFiles();
		// Send message to inform about creation of a new document.
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_NEWDOCUMENT );
		::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_NEWDOCUMENT );

		// Update AutoSaving Menu.
		uiAutoUpdateFlag = GetProfileInt( PREFERENCE, AUTOSAVING, m_bAutoSave );
		m_bAutoSave = ( uiAutoUpdateFlag > 0 ) ? 1 : 0;

		/////////////////////////////////////////////////////////////////////////
		// Check if a HyUpdate.new exist in the current directory (after an internet upgrade...)
		// in this case move this file in the HyUpdate installation directory.
		bool bHyUpdateNewExist = false;

		if( false == m_strHyUpdate_InstDir.IsEmpty() || true == TASApp.IsStartingWOInstal() )
		{
			CFileFind finder;
			CString strSource, strTarget;
			strSource = m_strStartDir + _T("HyUpdate.new");

			if( TRUE == finder.FindFile( strSource ) )
			{
				bHyUpdateNewExist = true;
			}

			if( true == bHyUpdateNewExist )
			{
				// Check if user is administrator.
				CString SrcFile = CString( _T("\"" ) ) + strSource + CString( _T( "\"") );
				strTarget = CString( _T("\"" ) ) + m_strHyUpdate_InstDir + _T( "HyUpdate.exe" ) + CString( _T( "\"") );
				StartProcess( _T("MoveFile.exe" ), 3, ( LPCTSTR )SrcFile, ( LPCTSTR )strTarget, _T( "true") );
			}

			strSource = m_strStartDir + _T("StartHyUpdate.new");

			if( TRUE == finder.FindFile( strSource ) )
			{
				// Copy 'StartHyUpdate.new' in the 'HyUpdate' folder.
				CString SrcFile = CString( _T("\"" ) ) + strSource + CString( _T( "\"") );
				strTarget = CString( _T("\"" ) ) + m_strHyUpdate_InstDir + _T( "StartHyUpdate.exe" ) + CString( _T( "\"") );
				StartProcess( _T("MoveFile.exe" ), 3, ( LPCTSTR )SrcFile, ( LPCTSTR )strTarget, _T( "true") );
			}
		}

		// New In Page.
		bool bUpdateDone = ShowNewInHtmlPage();

		// Don't launch HyUpdate if a new version of HyUpdate has been detected, or if a update process just finished.
		// We must wait time needed to move this new version to the HyUpdate folder.
		if( false == bHyUpdateNewExist && false == bUpdateDone )
		{
			// Check current Date and time with last date and time Update launch an update if delta date >= 2 month and if AutoUpdate flag is set.
			COleDateTime tdate = COleDateTime::GetCurrentTime();
			CString str = GetProfileString( HYUPDATE, LASTUPDATE, _T( "" ) );
			COleDateTime lastupdate;

			// If different of 'Never propose'...
			if( _tcsicmp( pmsFtpGatewayParams->GetString( CDlgGateway::egwFtpAutoUpdate ), _T("false") ) )
			{
				// If silent mode...
				if( 0 == _tcsicmp( pmsFtpGatewayParams->GetString( CDlgGateway::egwFtpAutoUpdate ), _T("true") ) )
				{
					_SilentUpdate();
				}
				else
				{
					if( false == lastupdate.ParseDateTime( ( LPCTSTR )str, VAR_DATEVALUEONLY ) )
					{
						// last update doesn't contain any value, update it with current Date Time.
						m_bAutoUpdate = true;

						// If 'AutoUpdate flag' is set, do it...
						if( 0 == _tcsicmp( pmsFtpGatewayParams->GetString( CDlgGateway::egwFtpAutoUpdate ), _T("fixedtime") ) )
						{
							pMainFrame->ClickRibbonUpdateTASelect();
						}
					}
					else
					{
						// More than 60 days.

						// AutoUpdate flag is set, do it...
						if( 0 == _tcsicmp( pmsFtpGatewayParams->GetString( CDlgGateway::egwFtpAutoUpdate ), _T("fixedtime") ) )
						{
							COleDateTimeSpan ts = tdate - lastupdate;

							if( ts.GetDays() > 60 )
							{
								m_bAutoUpdate = true;
								pMainFrame->ClickRibbonUpdateTASelect();
							}
						}
					}
				}
			}
		}

		// Used to  generate (in debug) a Pipe txt drop to build TAPocket DB
		//ExtractPipesForTAPocket();
		// CG: This line inserted for 'Tip of the Day' component.
		_ShowTipAtStartup();

		if( NULL != pMainFrame )
		{
			pMainFrame->CreateToolsDockablePane();
		}

		// *** Initialize the tender class ***
		// ***********************************
		m_Tender.SetZipPath(m_strStartDir + _TENDER_ZIP_FILE);

		
		if( true == m_bWizardMode )
		{
			// Switch to Wizard PM.
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
				pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_PM_ExpansionVessel );
				pMainFrame->ClickRibbonProductSelectionMode( ProductSelectionMode::ProductSelectionMode_Wizard );
			}
		}
		else
		{
			// Load Start Page
			CMainFrame *pFrame = static_cast<CMainFrame*>( AfxGetMainWnd() );
			pFrame->ActiveFormView( CMainFrame::eRVStartPage );
		}

		// Global initialization of DFU part.
		STDFU_Init();

		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		Gdiplus::GdiplusStartup( &m_gdiplusToken, &gdiplusStartupInput, NULL );

		// HYS-1700: If user has double-clicked on a '.tsp' file to open it, we now open the file
		// when 'InitInstance' is finished.
		if( false == m_strFileToOpen.IsEmpty() )
		{
			pMainFrame->PostMessage( WM_USER_OPENDOCUMENT, false, (LPARAM)&m_strFileToOpen );
		}

		return TRUE;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CTASelectApp::InitInstance'."), __LINE__, __FILE__ );
#ifndef DEBUG
		throw;
#else
		AfxDebugBreak();
#endif

		return FALSE;
	}
}

BOOL CTASelectApp::LoadPipesFromFile()
{
	try
	{
		CFileStatus fs;

		// *** Pipe DB Loading ***
		// If Pipes.db doesn't exist in this folder copy from installation dir if possible.
		CString strPipeDB = TASApp.GetDataPath() + CString( _T("pipes.db") );

		if( TRUE == CFile::GetStatus( strPipeDB, fs ) )
		{
			std::ifstream inpf( (LPCTSTR)strPipeDB, std::ifstream::in | std::ifstream::binary );
			short nVersion, nPipeDbVersion;
			inpf.read( (char *)&nPipeDbVersion, sizeof( nPipeDbVersion ) );
			inpf.close();

			_string str = _string( ( (CDB_MultiString *)GetpTADB()->Get( _T("TADB_VERSION") ).MP )->GetString( 0 ) );
			RemoveTChar( &str, '.' );
			from_string( str, nVersion );

			// If it's ok...
			if( nVersion >= nPipeDbVersion )
			{
				// Take it if exist.
				inpf.open( (LPCTSTR)strPipeDB, std::ifstream::in | std::ifstream::binary );

				if( std::ifstream::failbit == ( inpf.rdstate() & std::ifstream::failbit ) )
				{
					HYSELECT_THROW( _T("Can't open the '%s' file."), strPipeDB );
				}

				m_pPIPEDB->Read(inpf);
				inpf.close();
			}
			else
			{
				// Corrupted PIPES.db ?????
				int iError = IDOK;

				if( false == m_bRegisterRunning )
				{
					iError = AfxMessageBox( TASApp.LoadLocalizedString( IDS_ERRORCORRUPTEDUSERDB ), MB_OKCANCEL );
				}

				if( IDOK == iError )
				{
					m_pPIPEDB->Init();

					// Copy corrupted file to "pipes.db.bad" extension.
					CString strFileSave = strPipeDB + _T(".bad");

					// 'true' to delete the source.
					FileMoveCopy( strPipeDB, strFileSave, true );

					// Try to restart HySelect.
					if( false == m_bRegisterRunning && FALSE == RA_ActivateRestartProcess() )
					{
						// Handle restart error here.
						HYSELECT_THROW( _T("Internal error: Can't restart HySelect after reinitialization of the pipe database file.") );
					}
				}

				// Intentionally call 'return' instead of 'throw' because we want here that HySelect restart with the new pipe database file.
				return FALSE;
			}
		}
		else if( TRUE == CFile::GetStatus( _PIPES_DATABASE_BIN_FILE, fs ) )
		{
			std::ifstream inpf( _PIPES_DATABASE_BIN_FILE, std::ifstream::in | std::ifstream::binary );

			if( std::ifstream::failbit == ( inpf.rdstate() & std::ifstream::failbit ) )
			{
				HYSELECT_THROW( _T("Can't open the '%s' file."), _PIPES_DATABASE_BIN_FILE );
			}

			m_pPIPEDB->Read( inpf );
			inpf.close();
		}
		else if( TRUE == CFile::GetStatus( m_strStartDir + _PIPES_DATABASE_BIN_FILE, fs ) )
		{
			CString strFile = m_strStartDir + _PIPES_DATABASE_BIN_FILE;
			std::ifstream inpf( strFile, std::ifstream::in | std::ifstream::binary );

			if( std::ifstream::failbit == ( inpf.rdstate() & std::ifstream::failbit ) )
			{
				HYSELECT_THROW( _T("Can't open the '%s' file."), strFile );
			}

			m_pPIPEDB->Read( inpf );
			inpf.close();
		}

		// HYS-935: Because in previous HySelect version we had a bug with the "Autosave" feature, it can happen
		// that some 'pipes.db' contains fixed pipes (The ones that come from CTADatastruct, tadb.db). 
		// We need to clean these pipes.
		m_pPIPEDB->CleanPipesDB();

		return TRUE;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CTASelectApp::LoadPipesFromFile'."), __LINE__, __FILE__ );
		throw;
	}
}

CString CTASelectApp::GetDataPath()
{
	CString strDataPath = m_strDocumentsFolderForDoc + CString( _HYSELECT_NAME_BCKSLASH ) + m_strLanguageKey + CString( _T("\\") + CString( _DATA_DIRECTORY ) );
	return strDataPath;
}

CString CTASelectApp::GetLogsPath()
{
	CString strLogsPath = m_strDocumentsFolderForDoc + CString( _HYSELECT_NAME_BCKSLASH ) + m_strLanguageKey + CString( _T("\\") + CString( _LOGS_DIRECTORY ) );
	return strLogsPath;
}

CString CTASelectApp::GetDocBasePath()
{
	CString strDocumentPath = m_strDocumentsFolderForDoc + CString( _HYSELECT_NAME_BCKSLASH ) + CString( _DOC_DIRECTORY );
	return strDocumentPath;
}

CString CTASelectApp::GetProjectPath()
{
	CString strProjectPath = m_strDocumentsFolderForDoc + CString( _HYSELECT_NAME_BCKSLASH ) + m_strLanguageKey + CString( _T("\\") ) + _PROJECTS_DIRECTORY;
	return strProjectPath;
}

CString CTASelectApp::GetAutoSavePath()
{
	CString strAutoSavePath = m_strDocumentsFolderForDoc + CString( _HYSELECT_NAME_BCKSLASH ) + m_strLanguageKey + CString( _T("\\") ) + _PROJECTS_DIRECTORY + _AUTOSAVE_DIRECTORY;
	return strAutoSavePath;
}

int CTASelectApp::ExitInstance()
{
	// Release resources used by the critical section object.
	DeleteCriticalSection( &CriticalSection );

	if( true == m_bAppStarted )
	{
		// Retrieve the installation directory.
		const CString InstDir( TASApp.GetStartDir() );
		CFileStatus fs;
		BOOL fWorkingDirIsOK = CFile::GetStatus( _DATABASE_TEXT_FILE, fs ) || CFile::GetStatus( _DATABASE_BIN_FILE, fs );

		// TADB.DB is generated only in release mode with Txt files!!!!
		// Problem was the CDB_DOC object that is modified when running.
		// see CDB_Doc::ReadText()

#ifndef _DEBUG
		// Never Re-write a binary database!
		if( true == m_bReadTADBTxt )
		{
			// Write the database.
			// If the working directory is valid, write it there.
			// If not, write it in the installation dir.
			if( 1 == m_pTADB->IsModified() )
			{
				TRACE( _T("Write the database in binary file %s.\n"), _DATABASE_BIN_FILE );

				if( TRUE == fWorkingDirIsOK )
				{
					std::ofstream outf( _DATABASE_BIN_FILE, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc );
					ASSERT( !( outf.rdstate() & std::ofstream::failbit ) );
					m_pTADB->Write( outf );
					outf.close();
				}
				else
				{
					std::ofstream outf( InstDir + _DATABASE_BIN_FILE, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc );
					ASSERT( !( outf.rdstate() & std::ofstream::failbit ) );
					m_pTADB->Write( outf );
					outf.close();
				}
			}
		}
#endif

#ifndef _DEBUG
		// Write the user database.
		if( m_pUSERDB->IsModified() > 0 )
		{
			TRACE( _T("Write the User database in binary file %s.\n"), _USER_DATABASE_BIN_FILE );
			CString UserDBFolder = TASApp.GetDocumentsFolderForDoc();
			CString strUSERDBPath = UserDBFolder + CString( _HYSELECT_NAME_BCKSLASH ) + m_strLanguageKey + CString( _T("\\") + CString( _DATA_DIRECTORY ) );

			// Create UserData directory if needed
			int iError = SHCreateDirectoryEx( NULL, ( LPCTSTR )strUSERDBPath, NULL );

			if( ERROR_SUCCESS == iError || ERROR_FILE_EXISTS == iError || ERROR_ALREADY_EXISTS == iError )
			{
				CString UserDB = strUSERDBPath + CString( _T("user.db") );
				std::ofstream outf( UserDB, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc );
				ASSERT( !( outf.rdstate() & std::ofstream::failbit ) );

				if( !( outf.rdstate() & std::ofstream::failbit ) )
				{
					m_pUSERDB->Write( outf );
					outf.close();
				}
			}
		}
#endif

		// Delete all fixed pipes and associated tables from the pipe user database.
		CleanPipeUserDB();

#ifndef _DEBUG
		// Write the pipe database.
		if (m_pPIPEDB->IsModified() > 0 && true == m_pPIPEDB->GetSaveDB())
		{
			TRACE(_T("Write the Pipe database in binary file %s.\n"), _PIPES_DATABASE_BIN_FILE);
			CString PipeDBFolder = TASApp.GetDocumentsFolderForDoc();
			CString strPIPEDBPath = PipeDBFolder + CString(_HYSELECT_NAME_BCKSLASH) + m_strLanguageKey + CString(_T("\\") + CString(_DATA_DIRECTORY));

			// Create UserData directory if needed
			int iError = SHCreateDirectoryEx(NULL, (LPCTSTR)strPIPEDBPath, NULL);

			if (ERROR_SUCCESS == iError || ERROR_FILE_EXISTS == iError || ERROR_ALREADY_EXISTS == iError)
			{
				CString PipesDB = strPIPEDBPath + CString(_T("pipes.db"));
				std::ofstream outf(PipesDB, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
				ASSERT(!(outf.rdstate() & std::ofstream::failbit));

				if (!(outf.rdstate() & std::ofstream::failbit))
				{
					m_pPIPEDB->Write(outf);
					outf.close();
				}
			}
		}
#endif

		// Write all technical parameter values in the registry.
		// HYS-1590: We extract the writing in registry from all the technical parameters dialogs to put 
		// it in only one method. For a future Jira card (HYS-1631) we will try to put all the writings in registry
		// in this method.
		TASApp.GetpTADS()->WriteAllDataInRegistry();

		// Write default units to registry and delete the unit database.
		CUnitDatabase *pDB = CDimValue::AccessUDB();

		for( int i = 0; i < _UNITS_NUMBER_OF; i++ )
		{
			WriteProfileInt( _T("Default Units"), _UNIT_STRING_ID[i], pDB->GetDefaultUnitIndex( i ) );
		}

		delete pDB;

		// Save the language that user wants.
		// Remark: if user wants to change default language, we save it in 'm_NextLanguage' and when rebooting, if 'm_Language' and 'm_NextLanguage'
		//         are different then we can apply change.
		WriteProfileString( _T("Language" ), _T( "Language"), ( LPCTSTR )GetNextLanguage() );

		// Save the documentation language by default or choose by user.
		WriteProfileString( _T("Language" ), _T( "Documentation"), ( LPCTSTR )GetDocLanguage() );
	}

	// Release Resource DLL.
	HMODULE hDll = AfxGetResourceHandle();

	if( hDll != AfxGetInstanceHandle() )
	{
		AfxSetResourceHandle( AfxGetInstanceHandle() );
		FreeLibrary( hDll );
	}

	if( m_hmHubSchemes )
	{
		FreeLibrary( m_hmHubSchemes );
	}

	if( m_hmProdPics )
	{
		FreeLibrary( m_hmProdPics );
	}

	if( m_hmLocProdPics )
	{
		FreeLibrary( m_hmLocProdPics );
	}

	if( m_hspread )
	{
		FreeLibrary( m_hspread );
	}

	delete m_pTADB;
	delete m_pUSERDB;
	delete m_pPIPEDB;
	delete m_pTADS;

	//Show to the console the memory
	// leaks only in debug mode
	//#ifdef _DEBUG
	//	DumpUnfreed();
	//#endif

	int nRes = CWinAppEx::ExitInstance();

	RA_DoRestartProcessFinish();

	STDFU_DeInit();

	Gdiplus::GdiplusShutdown( m_gdiplusToken );

	return nRes;
}

BOOL CTASelectApp::PreTranslateMessage( MSG *pMsg )
{
#ifdef DEBUG
	bool fIsRunning;
	CMainFrame *pMainFrame = static_cast<CMainFrame *>( AfxGetMainWnd() );

	if( NULL != pMainFrame && NULL != pMainFrame->GetSafeHwnd() && true == pMainFrame->IsMacroRunning( fIsRunning ) && true == fIsRunning )
	{
		MSG *pMsg = AfxGetCurrentMessage();

		if( WM_LBUTTONDOWN == pMsg->message || WM_LBUTTONDBLCLK == pMsg->message || WM_LBUTTONUP == pMsg->message || WM_RBUTTONDOWN == pMsg->message ||
			WM_RBUTTONUP == pMsg->message || WM_KEYDOWN == pMsg->message || WM_KEYUP == pMsg->message )
		{
			pMainFrame->WriteMacro( pMsg );
		}
	}
#endif

	return CWinAppEx::PreTranslateMessage( pMsg );
}

bool CTASelectApp::GenerateTASCopeDBX()
{
	bool fAutoSave = TASApp.IsAutoSave();
	TASApp.SetAutoSaveflag( false );

	// Switch in CBXMode.
	g_bWriteTACBXMode = true;
	const CString InstDir( TASApp.GetStartDir() );
	CString strTAScopeXDB = CString( InstDir + _DATA_DIRECTORY + _T("TADB.xdb") );
	RemoveFile( strTAScopeXDB );
	std::ofstream outf( strTAScopeXDB, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc );

	if( ( outf.rdstate() & std::ofstream::failbit ) )
	{
		return false;
	}

	TASApp.GetpTADB()->Write( outf );
	outf.close();
	g_bWriteTACBXMode = false;
	TASApp.SetAutoSaveflag( fAutoSave );
	return true;
}

bool CTASelectApp::ReadTADBTxT()
{
	try
	{
		CFileStatus fs;

		if( TRUE == CFile::GetStatus( _DATABASE_TEXT_FILE, fs ) )
		{
			// If TADB3.txt exist, delete the TADB3.DB file first.
			const CString InstDir( TASApp.GetStartDir() );
			RemoveFile( CString( InstDir + _DATABASE_BIN_FILE ) );

			// Read Redefinetab.txt.
			// Redefinition table allowing to modify the name, family, Qrs and CBI type of CDB_TAProduct objects identified by their ID.
			if( TRUE == CFile::GetStatus( _REDEFINETAB_TEXT_FILE, fs ) )
			{
				CFileTxt inpf( _REDEFINETAB_TEXT_FILE, CFile::modeRead | CFile::typeBinary );
				RedefineTab.ReadFieldTab( inpf );
				inpf.Close();
			}

			// Read Filtertab.txt.
			// Filtration table allowing to delete, hide, make n.a., etc. products or groups of products.
			if( TRUE == CFile::GetStatus( _FILTERTAB_TEXT_FILE, fs ) )
			{
				CFileTxt inpf( _FILTERTAB_TEXT_FILE, CFile::modeRead | CFile::typeBinary );

				HYSELECT_TRY
				{
					FilterTab.ReadFieldTab( inpf );
				}
				HYSELECT_CATCH_ARG( clHySelectException, _T("Can't read database text file '%s'."), _FILTERTAB_TEXT_FILE )

				inpf.Close();
			}

			// Read Tadb.txt.
			std::ifstream inpf( _DATABASE_TEXT_FILE, std::ifstream::in | std::ifstream::binary );

			if( std::ifstream::failbit == ( inpf.rdstate() & std::ifstream::failbit ) )
			{
				HYSELECT_THROW( _T("Can't access to the file '%s'."), _DATABASE_TEXT_FILE );
			}
			
			HYSELECT_TRY
			{
				m_pTADB->ReadText( inpf );
			}
			HYSELECT_CATCH_ARG( clHySelectException, _T("Can't read database text file '%s'."), _DATABASE_TEXT_FILE )
			
			inpf.close();

			CStringArray arFiles;
			arFiles.Add( _PIPEDB_TEXT_FILE );
			arFiles.Add(_TADB_ACC_TEXT_FILE);
			arFiles.Add( _TADB_REGV_TEXT_FILE );
			arFiles.Add( _TADB_DPC_TEXT_FILE );
			arFiles.Add( _TADB_STADSTAR_TXT_FILE ); // at this position due to MV declaration that are added in MVtable declared in DpC file
			arFiles.Add( _TADB_CV_TEXT_FILE );
			arFiles.Add( _TADB_TRV_TEXT_FILE);
			arFiles.Add( _TADB_BCV_TEXT_FILE );
			arFiles.Add( _TADB_WQ_TEXT_FILE );
			arFiles.Add( _TADB_PM_TEXT_FILE );
			arFiles.Add( _TADB_SV_TEXT_FILE );
			arFiles.Add( _TADB_SAFETYVALVE_TEXT_FILE );
			arFiles.Add( _TADB_SMARTVALVE_TEXT_FILE );
			arFiles.Add( _TADB_STAFSTAR_TXT_FILE );
			arFiles.Add( _TADB_HUB_TEXT_FILE );
			arFiles.Add( _TADB_DELETED_PROD );
			arFiles.Add( _TADB_ACT_TEXT_FILE );
			arFiles.Add( _LOCALDB_TEXT_FILE );
			//arFiles.Add( _DOCDB_TEXT_FILE );
			arFiles.Add( _PRICEDB_TEXT_FILE );

			CFileFind finder;
			BOOL bFound = finder.FindFile( CString( L"DATA\\UNIVALV_" ) + CString( L"*.txt" ) );

			while( TRUE == bFound )
			{
				bFound = finder.FindNextFile();

				if( TRUE == finder.IsDots() )
				{
					continue;
				}

				if( TRUE == finder.IsDirectory() )
				{
					continue;
				}

				arFiles.Add( finder.GetFilePath() );
			}

			for( int iLoopFile = 0; iLoopFile < arFiles.GetCount(); iLoopFile++ )
			{
				if( TRUE == CFile::GetStatus( ( LPCTSTR )arFiles[iLoopFile], fs ) )
				{
					TRACE( _T("\n%s"), arFiles[iLoopFile] );
					inpf.open( ( LPCTSTR )arFiles[iLoopFile], std::ifstream::in | std::ifstream::binary );

					if( std::ifstream::failbit == ( inpf.rdstate() & std::ifstream::failbit ) )
					{
						HYSELECT_THROW( _T("Can't access to the file '%s'."), arFiles[iLoopFile] );
					}

					bool bReadingObjectFromDeletedFile = ( 0 == CString( _TADB_DELETED_PROD ).Compare( arFiles[iLoopFile] ) ) ? true : false;

					HYSELECT_TRY
					{
						m_pTADB->Append( inpf, 0, bReadingObjectFromDeletedFile );
					}
					HYSELECT_CATCH_ARG( clHySelectException, _T("Can't read database text file '%s'."), arFiles[iLoopFile] )

					inpf.close();
				}
				else if( 0 != arFiles[iLoopFile].Compare( _PRICEDB_TEXT_FILE ) )
				{
					// Price database file is optional. This is why here if we can't access a file and it's not the price database file, it's an error.
					HYSELECT_THROW( _T("Can't get status of the file '%s'."), arFiles[iLoopFile] );
				}
			}

			m_pTADB->RemoveAllEnumDefinition();

			// Read LanTab.txt.
			// "Local Article Number table" contains all local article number in front of TA Article number.
			if( TRUE == CFile::GetStatus( _LOCARTNUMTAB_TEXT_FILE, fs ) )
			{
				CFileTxt clFileText( _LOCARTNUMTAB_TEXT_FILE, CFile::modeRead | CFile::typeBinary );

				HYSELECT_TRY
				{
					LocArtNumTab.ReadFile( clFileText );
				}
				HYSELECT_CATCH_ARG( clHySelectException, _T("Can't read database text file '%s'."), _LOCARTNUMTAB_TEXT_FILE )

				clFileText.Close();
			}

			// Create compound products Body + Inlet + Outlet for DpControllers.
			CreateCompoundTAP( (CTable *)( m_pTADB->Get( _T("DPCONTR_TAB") ).MP ), CLASS( CDB_DpController ) );

			// Create compound products Body + Inlet + Outlet for differential pressure relief valves.
			CreateCompoundTAP( (CTable *)( m_pTADB->Get( _T("DPCONTR_TAB") ).MP ), CLASS( CDB_DpReliefValve ) );

			// Create compound products Body + Inlet + Outlet for PIControlValves.
			CreateCompoundTAP( (CTable *)( m_pTADB->Get( _T("PICTRLVALV_TAB") ).MP ), CLASS( CDB_PIControlValve ) );

			// Filtration must be applied after CreateCompoundTAP.
			m_pTADB->ApplyFilters();

			// HYS-1697: Compute sorting keys after having applied filters.
			// Compute SortingKey.
			ComputeTAPSortingKey();

			// Cross verify reading.
			m_pTADB->CrossVerifyID();

			// Everything has worked.
			// The current directory is thus the application directory !
			TCHAR tcDir[MAX_PATH];
			int iLength = GetCurrentDirectory( MAX_PATH, tcDir );

			if( iLength > 0 )
			{
				_tcsncpy_s( tcDir + iLength, MAX_PATH - iLength - 1, _T("\\"), MAX_PATH - iLength - 2 );

				//m_strStartDir = dir;
				if( true == m_bUpdateRegistryStartingFolder )
				{
					WriteProfileString( _T("Files" ), _T( "Installation Directory"), tcDir );
				}
			}

			return true;
		}

		return false;
	}
	HYSELECT_CATCH( clHySelectException, _T("Error in 'CTASelectApp::ReadTADBTxT'.") )
}

void CTASelectApp::PrepareUserDb()
{
	try
	{
		CString	str;

		// Create and insert the PAGE_SETUP_TAB table in m_USERDB if it does not exist yet.
		CTable *pPageSetupTab;

		if( _T('\0') == *( (*m_pUSERDB)().Get( _T("PAGE_SETUP_TAB") ).ID ) )
		{
			IDPTR IDPtrTab;
			m_pUSERDB->CreateObject( IDPtrTab, CLASS( CTable ), _T("PAGE_SETUP_TAB") );

			pPageSetupTab = static_cast<CTable *>( IDPtrTab.MP );
			pPageSetupTab->SetName( _T("Page Setup") );
		
			(*m_pUSERDB)().Insert( IDPtrTab );
		}

		// Create and insert the PAGE_FIELD_TAB table in m_USERDB if it does not exist yet.
		CTable *pPageFieldTab;

		if( _T('\0') == *( (*m_pUSERDB)().Get( _T("PAGE_FIELD_TAB") ).ID ) )
		{
			IDPTR IDPtrTab;
			m_pUSERDB->CreateObject( IDPtrTab, CLASS( CTable ), _T("PAGE_FIELD_TAB") );

			pPageFieldTab = (CTable *)( IDPtrTab.MP );
			pPageFieldTab->SetTrueParent( true );
			pPageFieldTab->SetName( _T("Page Field") );

			(*m_pUSERDB)().Insert( IDPtrTab );

			// Build all predefined styles.

			// Style Complete.
			IDPTR StyleIDPtr;
			m_pUSERDB->CreateObject( StyleIDPtr, CLASS( CDB_PageField ), _T("COMPLETE") );
			pPageFieldTab->Insert( StyleIDPtr );

			CDB_PageField *pPageField = ( CDB_PageField * )( StyleIDPtr.MP );
			str = LoadLocalizedString( IDS_PAGEFIELD_COMPLETE );
			pPageField->SetString( (TCHAR *)( LPCTSTR )str );
			pPageField->SetPredefStyle( PAGESTYLECOMPLETE );
			pPageField->Lock( IDPtrTab );

			// Style Technical Long.
			if( true == ( ( CTASelectApp * )AfxGetApp() )->IsPriceUsed() )
			{
				m_pUSERDB->CreateObject( StyleIDPtr, CLASS( CDB_PageField ), _T("TECHNICAL_LONG") );
				pPageFieldTab->Insert( StyleIDPtr );

				pPageField = ( CDB_PageField * )( StyleIDPtr.MP );
				str = LoadLocalizedString( IDS_PAGEFIELD_TECH );
				pPageField->SetString( (TCHAR *)( LPCTSTR )str );
				pPageField->SetPredefStyle( PAGESTYLETECHNICAL );
				pPageField->Lock( IDPtrTab );
			}

			// Style Technical short.
			m_pUSERDB->CreateObject( StyleIDPtr, CLASS( CDB_PageField ), _T("TECHNICAL_SHORT") );
			pPageFieldTab->Insert( StyleIDPtr );

			pPageField = ( CDB_PageField * )( StyleIDPtr.MP );
			str = LoadLocalizedString( IDS_PAGEFIELD_STECH );
			pPageField->SetString( (TCHAR *)( LPCTSTR )str );
			pPageField->SetPredefStyle( PAGESTYLETECHNICALSHORT );
			pPageField->Lock( IDPtrTab );

			// Style order long.
			if( true == ( (CTASelectApp *)AfxGetApp() )->IsPriceUsed() )
			{
				m_pUSERDB->CreateObject( StyleIDPtr, CLASS( CDB_PageField ), _T("ORDER") );
				pPageFieldTab->Insert( StyleIDPtr );
			
				pPageField = (CDB_PageField *)( StyleIDPtr.MP );
				str = LoadLocalizedString( IDS_PAGEFIELD_ORDER );
				pPageField->SetString( (TCHAR *)( LPCTSTR )str );
				pPageField->SetPredefStyle( PAGESTYLEORDER );
				pPageField->Lock( IDPtrTab );
			}
		}

		// Create and insert the VARIOUS_TAB table in m_USERDB if it does not exist yet.
		CTable *pVariousTab;

		if( _T('\0') == *( (*m_pUSERDB)().Get( _T("VARIOUS_TAB") ).ID ) )
		{
			IDPTR IDPtr;
			m_pUSERDB->CreateObject( IDPtr, CLASS( CTable ), _T("VARIOUS_TAB") );

			pVariousTab = (CTable *)( IDPtr.MP );
			pVariousTab->SetName( _T("Various parameters") );
		
			(*m_pUSERDB)().Insert( IDPtr );

			// Create a multiString with gateway parameters:
			// line 0 Gateway used or not
			// line 1 Gateway FTP address
			// line 2 Gateway FTP password
			// line 3 Gateway FTP port number
			// line 4 AutoUpdate flag
			// line 5 Gateway HTTP address
			// line 6 Gateway HTTP password
			// line 7 Gateway HTTP port number

			IDPTR msIDPtr;
			m_pUSERDB->CreateObject( msIDPtr, CLASS( CDB_MultiString ), _T("FTPGATEWAY_PAR") );
		
			CDB_MultiString *pMS = ( CDB_MultiString * )( msIDPtr.MP );

			pMS->SetString( CDlgGateway::egwFtpAutoUpdate, _T("true") );
			pMS->SetString( CDlgGateway::egwUsed, _T("false") );
			pMS->SetString( CDlgGateway::egwFtpAdd, _T("") );
			pMS->SetString( CDlgGateway::egwFtpPsw, _T("") );
			pMS->SetString( CDlgGateway::egwFtpPort, _T("21") );
			pMS->SetString( CDlgGateway::egwHttpAdd, _T("") );
			pMS->SetString( CDlgGateway::egwHttpPsw, _T("") );
			pMS->SetString( CDlgGateway::egwHttpPort, _T("80") );

			pVariousTab->Insert( msIDPtr );

			// Create User Reference Entry.
			IDPTR urIDPtr;
			m_pUSERDB->CreateObject( urIDPtr, CLASS( CDS_UserRef ), _T("USER_REF") );
			pVariousTab->Insert( urIDPtr );
		}

		// Create and insert the GLOSSARY_TAB table in m_USERDB if it does not exist yet.
		CTable *pGlossaryTab;

		if( _T('\0') == *( (*m_pUSERDB)().Get( _T("GLOSSARY_TAB") ).ID ) )
		{
			IDPTR IDPtr;
			m_pUSERDB->CreateObject( IDPtr, CLASS( CTable ), _T("GLOSSARY_TAB") );

			pGlossaryTab = (CTable *)( IDPtr.MP );
			pGlossaryTab->SetName( _T("Glossary") );
		
			(*m_pUSERDB)().Insert( IDPtr );
		}

		// Create and insert the RADNOMCOND_TAB table in m_USERDB if it does not exist yet.
		CTable *pRadNomCondTab;

		if( _T('\0') == *( (*m_pUSERDB)().Get( _T("RADNOMCOND_TAB") ).ID ) )
		{
			IDPTR IDPtr;
			m_pUSERDB->CreateObject( IDPtr, CLASS( CTable ), _T("RADNOMCOND_TAB") );
			pRadNomCondTab = (CTable *)( IDPtr.MP );
			pRadNomCondTab->SetName( _T("Radiator nominal conditions") );

			(*m_pUSERDB)().Insert( IDPtr );

			// Get a pointer to RADNOMCOND_TAB table from m_TADB.
			CTable *pTaRadNomCondTab = (CTable *)( (*m_pTADB)().Get( _T("RADNOMCOND_TAB") ).MP );
			ASSERT( NULL != pTaRadNomCondTab );

			// Loop on existing CDB_RadNomCond in TA radiator nom. Cond. table and introduce them into USERDB.
			for( IDPTR RNCIDPtrTA = pTaRadNomCondTab->GetFirst( CLASS( CDB_RadNomCond ) ); *RNCIDPtrTA.ID; RNCIDPtrTA = pTaRadNomCondTab->GetNext() )
			{
				IDPTR RNCIDPtr;
				m_pUSERDB->CreateObject( RNCIDPtr, CLASS( CDB_RadNomCond ), RNCIDPtrTA.ID );
				pRadNomCondTab->Insert( RNCIDPtr );
				( (CDB_RadNomCond *)RNCIDPtrTA.MP )->Copy( (CDB_RadNomCond *)RNCIDPtr.MP );
				( (CDB_RadNomCond *)RNCIDPtr.MP )->SetFix( true );
			}
		}

		// HYS:291: Allow to use temporarily the user db pipe embedded in the tsp file.
		// Import pipes from USERDB in the case it still contains PIPE_TAB.
		// Needed to upgrade from old versions.
		TASApp.GetpPipeDB()->CopyFrom( m_pUSERDB );

		// Remove pipes from USERDB because only PIPEDB is used now.
		TASApp.GetpPipeDB()->DeletePipeTab( m_pUSERDB );

		// Loop on Pipe series tables to introduce them in m_PIPEDB.
		TASApp.GetpPipeDB()->CopyFrom( m_pTADB );

		// Verify IDPTR of pipe stored in m_PIPEDB.
		TASApp.GetpPipeDB()->CheckPipeTab();
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CTASelectApp::PrepareUserDb'."), __LINE__, __FILE__ );
		throw;
	}
}

void CTASelectApp::CleanPipeUserDB()
{
	TASApp.GetpPipeDB()->RemoveFixed();
}

void CTASelectApp::CreateCompoundTAP( CTable *pTab, LPCTSTR lpstrClass )
{
	try
	{
		// Scan all TA product from one category to find product body without fixed connection.
		// Create new TA product based on the product body and available CDB_Component

		// Create a new temporary Table to store new composed TA products.
		IDPTR IDPtrTmpTab;
		m_pTADB->CreateObject( IDPtrTmpTab, CLASS( CTable ), _T("TMPTABLE") );
		pTab->Insert( IDPtrTmpTab );

		int iNumberOfValves = 0;
		IDPTR IDPtr = _NULL_IDPTR;

		for( IDPtr = pTab->GetFirst( lpstrClass ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			if( false == ( (CDB_TAProduct *)( IDPtr.MP ) )->IsConnTabUsed() )
			{
				continue;
			}

			CDB_TAProduct *pTAP = (CDB_TAProduct *)( IDPtr.MP );
			CDB_RuledTable *pRTIn = dynamic_cast<CDB_RuledTable *>( GetpTADB()->Get( pTAP->GetConnectID() ).MP );
		
			if( NULL == pRTIn )
			{
				HYSELECT_THROW( _T("Object '%s' uses component table but the input component ruled table is not defined."), IDPtr.ID );
			}
		
			CDB_RuledTable *pRTOut = dynamic_cast<CDB_RuledTable *>( GetpTADB()->Get( pTAP->GetConn2ID() ).MP );

			// For each input component find matching output component.
			IDPTR IDPtrCompIn;

			for( IDPtrCompIn = pRTIn->GetFirst(); _T('\0') != *IDPtrCompIn.ID; IDPtrCompIn = pRTIn->GetNext( IDPtrCompIn.MP ) )
			{
				IDPTR IDPtrCompOut;
				CDB_Component *pCompIn = dynamic_cast<CDB_Component *>( IDPtrCompIn.MP );
			
				if( NULL == pCompIn )
				{
					HYSELECT_THROW( _T("The object '%s' in the component table '%s' is not a 'CDB_Component' object."), IDPtrCompIn.ID, pTAP->GetConnectID() );
				}

				if( NULL != pRTOut )
				{
					for( IDPtrCompOut = pRTOut->GetFirst(); _T('\0') != *IDPtrCompOut.ID; IDPtrCompOut = pRTOut->GetNext() )
					{
						CDB_Component *pCompOut = dynamic_cast<CDB_Component *>( IDPtrCompOut.MP );

						if( NULL == pCompOut )
						{
							HYSELECT_THROW( _T("The object '%s' in the component table '%s' is not a 'CDB_Component' object."), IDPtrCompOut, pTAP->GetConn2ID() );
						}

						if( 0 == _tcscmp( pCompIn->GetConnectID(), pCompOut->GetConnectID() ) && 0 == _tcscmp( pCompIn->GetSizeID(), pCompOut->GetSizeID() ) )
						{
							// Inlet and Outlet are found, create new composed product body+inlet+outlet ID of new object.
							CString str = CString( IDPtr.ID ) + CString( pCompIn->GetIDExt() ) + CString( pCompOut->GetIDExt() );

							IDPTR IDPtrNew;
							m_pTADB->CreateObject( IDPtrNew, lpstrClass, ( LPCTSTR )str );
							( (CTable *)IDPtrTmpTab.MP )->Insert( IDPtrNew );
							IDPtr.MP->Copy( IDPtrNew.MP );

							// Update ConnectID.
							( (CDB_TAProduct *)IDPtrNew.MP )->SetConnectID( pCompIn->GetIDPtr().ID );
							( (CDB_TAProduct *)IDPtrNew.MP )->SetConn2ID( pCompOut->GetIDPtr().ID );

							// Update SizeID.
							( (CDB_TAProduct *)IDPtrNew.MP )->SetSizeID( pCompIn->GetSizeID() );

							// Keep Body Article Number, composed article number is done in TAProduct::GetArtNumb().

							// Update Size, availability, ... of new TA product
							( (CDB_TAProduct *)IDPtrNew.MP )->SetAvailable( pTAP->IsAvailable() && pCompIn->IsAvailable() && pCompOut->IsAvailable() );
							( (CDB_TAProduct *)IDPtrNew.MP )->SetDeleted( pTAP->IsDeleted() || pCompIn->IsDeleted() || pCompOut->IsDeleted() );
							( (CDB_TAProduct *)IDPtrNew.MP )->SetHidden( pTAP->IsHidden() || pCompIn->IsHidden() || pCompOut->IsHidden() );
							( (CDB_TAProduct *)IDPtrNew.MP )->Fix();
							( (CDB_TAProduct *)IDPtrNew.MP )->SetInt1( pTAP->GetInt1() && pCompIn->GetInt1() && pCompOut->GetInt1() );
						
							iNumberOfValves++;
						}
					}
				}
				else
				{
					// Inlet found, create new composed product body+inlet ID of new object.
					CString str = CString( IDPtr.ID ) + CString( pCompIn->GetIDExt() );

					IDPTR IDPtrNew;
					m_pTADB->CreateObject( IDPtrNew, lpstrClass, ( LPCTSTR )str );
					( (CTable *)IDPtrTmpTab.MP )->Insert( IDPtrNew );
					IDPtr.MP->Copy( IDPtrNew.MP );

					// Update ConnectID.
					( (CDB_TAProduct *)IDPtrNew.MP )->SetConnectID( pCompIn->GetIDPtr().ID );
					( (CDB_TAProduct *)IDPtrNew.MP )->SetConn2ID( _NO_ID );

					// Update SizeID.
					( (CDB_TAProduct *)IDPtrNew.MP )->SetSizeID( pCompIn->GetSizeID() );

					// Keep Body Article Number, composed article number is done in TAProduct::GetArtNumb()

					// Update Size, availability, ... of new TA product.
					( (CDB_TAProduct *)IDPtrNew.MP )->SetAvailable( pTAP->IsAvailable() && pCompIn->IsAvailable() );
					( (CDB_TAProduct *)IDPtrNew.MP )->SetDeleted( pTAP->IsDeleted() || pCompIn->IsDeleted() );
					( (CDB_TAProduct *)IDPtrNew.MP )->SetHidden( pTAP->IsHidden() || pCompIn->IsHidden() );
					( (CDB_TAProduct *)IDPtrNew.MP )->Fix();
					( (CDB_TAProduct *)IDPtrNew.MP )->SetInt1( pTAP->GetInt1() && pCompIn->GetInt1() );
				
					iNumberOfValves++;
				}
			}
		}

		TRACE( _T("\n %i TA product Created in temporary table"), iNumberOfValves );

		// Clean TADB from TA product body without Inlet/Outlet.
		iNumberOfValves = 0;

		for( IDPtr = pTab->GetFirst( lpstrClass ); _T('\0') != *IDPtr.ID; )
		{
			if( false == ( (CDB_TAProduct *)IDPtr.MP )->IsConnTabUsed() )
			{
				IDPtr = pTab->GetNext();
				continue;
			}

			// Unlock Characteristic.
			if( true == IDPtr.MP->IsClass( CLASS( CDB_DpController ) ) && NULL != ( (CDB_DpController *)IDPtr.MP )->GetDpCCharacteristic() )
			{
				( (CDB_DpController *)IDPtr.MP )->GetDpCCharacteristic()->Unlock( IDPtr );
			}
			else
			{
				if( NULL != ( (CDB_TAProduct *)IDPtr.MP )->GetValveCharacteristic() )
				{
					( (CDB_TAProduct *)IDPtr.MP )->GetValveCharacteristic()->Unlock( IDPtr );
				}
			}

			IDPtr.MP->Unlock( pTab->GetIDPtr() );
			IDPTR IDPtrNext = pTab->GetNext();
			pTab->Remove( IDPtr );
			m_pTADB->DeleteObject( IDPtr );
			IDPtr = IDPtrNext;
			iNumberOfValves++;
		}

		TRACE( _T("\n %i TA product body removed"), iNumberOfValves );

		// Add new composed objects in TA product Tab.
		iNumberOfValves = 0;

		for( IDPtr = ( (CTable *)IDPtrTmpTab.MP )->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = ( (CTable *)IDPtrTmpTab.MP )->GetNext() )
		{
			( (CTable *)IDPtrTmpTab.MP )->Remove( IDPtr );
			pTab->Insert( IDPtr );
			IDPtr.MP->Lock( pTab->GetIDPtr() );
			iNumberOfValves++;
		}

		TRACE( _T("\n %i TA product moved and locked in TADB\n"), iNumberOfValves );

		// Remove Temporary Tab.
		pTab->Remove( IDPtrTmpTab );
		m_pTADB->DeleteObject( IDPtrTmpTab );
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CTASelectApp::CreateCompoundTAP'."), __LINE__, __FILE__ );
		throw;
	}
}

void CTASelectApp::CreateFilterViewTxT()
{
	BeginWaitCursor();

	// Variables.
	CFileTxt f;
	CString strTxtLine;
	int iDN = 0;
	int i = 0;
	int iMapPos = 0;
	bool fNewValve = true;
	CTable *pTab = NULL;
	IDPTR IDPtr = _NULL_IDPTR;
	CString strTitle = L"Table;NameID[From-To];TypeID;FamilyID;ConnectionID;VersionID;PN;SizeID[From-To];ProdPicID;d.;a.;h.;p.";
	std::multimap<int, CDB_TAProduct *> MyMultiMap;
	std::multimap<int, CDB_TAProduct *>::iterator it;
	CDB_TAProduct *pTAP;

	f.Open( L"FilterView.txt", CFile::modeCreate | CFile::modeWrite );

	f.SeekToEnd();
	f.WriteTxtLine( strTitle );

	// Create the table with all valves.
	pTab = (CTable *)( m_pTADB->Get( L"ROOT_TABLE" ).MP );
	ASSERT( pTab );

	// First level of ROOT Table.
	for( IDPtr = pTab->GetFirst(); *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CTable *pSubTable = dynamic_cast<CTable *>( IDPtr.MP );

		if( NULL == pSubTable )
		{
			continue;
		}

		FillProductMap( pSubTable, MyMultiMap );
	}

	// Find the information about the size and name boundaries for a same key value.
	i = 1;
	int iSizeMin = 99;
	int iSizeMax = 1;
	CString SizeMin, SizeMax, NameMin, NameMax;
	SizeMin = SizeMax = ( (CDB_TAProduct *)MyMultiMap.begin()->second )->GetSizeID();
	NameMin = NameMax = ( (CDB_TAProduct *)MyMultiMap.begin()->second )->GetName();

	for( it = MyMultiMap.begin() ; it != MyMultiMap.end(); it++ )
	{
		if( i == it->first )
		{
			pTAP = it->second;

			if( pTAP->GetSizeKey() < iSizeMin )
			{
				iSizeMin = pTAP->GetSizeKey();
				SizeMin = pTAP->GetSizeID();
				NameMin = pTAP->GetName();
			}

			if( pTAP->GetSizeKey() > iSizeMax )
			{
				iSizeMax = pTAP->GetSizeKey();
				SizeMax = pTAP->GetSizeID();
				NameMax = pTAP->GetName();
			}

			continue;
		}

		CDB_Component *pComp = dynamic_cast<CDB_Component *>( pTAP );

		if( NULL != pComp )
		{
			CTable *pTabCmp = dynamic_cast<CTable *>( pTAP->GetIDPtr().PP );
			ASSERT( pTabCmp );
			ASSERT( !IDcmp( pTabCmp->GetIDPtr().ID, L"COMPONENT_TAB" ) );
		}

		CString TabName = ( (CTable *)pTAP->GetIDPtr().PP )->GetName();

		// Write informations into the file.
		strTxtLine.Format(	L"\n%s;[%s - %s];%s;%s;%s;%s;%s;[%s - %s];%s;%i;%i;%i;%i",
							TabName,
							NameMin,
							NameMax,
							pTAP->GetTypeID(),
							pTAP->GetFamilyID(),
							pTAP->GetConnectID(),
							pTAP->GetVersionID(),
							pTAP->GetPN().c_str(),
							SizeMin,
							SizeMax,
							( NULL != dynamic_cast<CDB_TAProduct *>( pTAP ) ) ? pTAP->GetProdPicID() : L"",
							pTAP->IsDeleted(),
							pTAP->IsAvailable(),
							pTAP->IsHidden(),
							pTAP->GetPriorityLevel() );

		f.WriteTxtLine( ( LPCTSTR )strTxtLine );
		pTAP = it->second;
		iSizeMin = 99;
		iSizeMax = 1;
		SizeMin = SizeMax = pTAP->GetSizeID();
		NameMin = NameMax = pTAP->GetName();

		if( pTAP->GetSizeKey() < iSizeMin )
		{
			iSizeMin = pTAP->GetSizeKey();
			SizeMin = pTAP->GetSizeID();
			NameMin = pTAP->GetName();
		}

		if( pTAP->GetSizeKey() > iSizeMax )
		{
			iSizeMax = pTAP->GetSizeKey();
			SizeMax = pTAP->GetSizeID();
			NameMax = pTAP->GetName();
		}

		i++;
	}

	f.Close();
	MyMultiMap.clear();
	EndWaitCursor();
}

void CTASelectApp::FillProductMap( CTable *pTab, std::multimap<int, CDB_TAProduct *> &MyMultiMap )
{
	if( NULL == pTab )
	{
		return;
	}

	int i = MyMultiMap.size();
	int iMapPos = 0;
	bool fNewValve = true;
	std::multimap<int, CDB_TAProduct *>::iterator it;


	// Do a loop on all valves and classify them into a multimap.
	// Each valve only change by the name and the size will have the same key.
	for( IDPTR IDPtr = pTab->GetFirst(); *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );

		if( NULL == pTAP )
		{
			continue;
		}

		for( it = MyMultiMap.begin() ; it != MyMultiMap.end(); it++ )
		{
			CDB_TAProduct *pTAPMMap = ( (CDB_TAProduct *)it->second );

			if( 0 == IDcmp( pTAP->GetTypeID(), pTAPMMap->GetTypeID() ) &&
				0 == IDcmp( pTAP->GetFamilyID(), pTAPMMap->GetFamilyID() ) &&
				0 == IDcmp( pTAP->GetConnectID(), pTAPMMap->GetConnectID() ) &&
				0 == IDcmp( pTAP->GetVersionID(), pTAPMMap->GetVersionID() ) &&
				0 == IDcmp( pTAP->GetPN().c_str(), pTAPMMap->GetPN().c_str() ) &&
				( pTAP->IsDeleted() == pTAPMMap->IsDeleted() ) &&
				( pTAP->IsAvailable() == pTAPMMap->IsAvailable() ) &&
				( pTAP->IsHidden() == pTAPMMap->IsHidden() ) &&
				( pTAP->GetPriorityLevel() == pTAPMMap->GetPriorityLevel() ) )
			{
				fNewValve = false;
				iMapPos = it->first;
			}
		}

		if( true == fNewValve )
		{
			MyMultiMap.insert( std::pair<int, CDB_TAProduct *>( ++i, pTAP ) );
		}
		else
		{
			MyMultiMap.insert( std::pair<int, CDB_TAProduct *>( iMapPos, pTAP ) );
		}

		fNewValve = true;
	}
}

void CTASelectApp::_ShowTipAtStartup( void )
{
	// CG: This function added by 'Tip of the Day' component.

	CCommandLineInfo cmdInfo;
	ParseCommandLine( cmdInfo );

	if( TRUE == cmdInfo.m_bShowSplash )
	{
		CDlgTip dlg;

		if( TRUE == dlg.CanShowToopTip() )
		{
			dlg.DoModal();
		}
	}
}

#define SILENTUPDATEDAYS 0
void CTASelectApp::_SilentUpdate()
{
	CString str;
	COleDateTime tdate = COleDateTime::GetCurrentTime();
	str = GetProfileString( HYUPDATE, LASTUPDATE, _T( "" ) );

	COleDateTime silentupdate;
	bool fNoDate = false;

	if( !silentupdate.ParseDateTime( ( LPCTSTR )str, VAR_DATEVALUEONLY ) )
	{
		fNoDate = true;
		silentupdate = COleDateTime::GetCurrentTime();
	}

	COleDateTimeSpan ts = tdate - silentupdate;

	if( SILENTUPDATEDAYS == 0 || ( ts.GetDays() > SILENTUPDATEDAYS ) || fNoDate )
	{
		if( true == m_strHyUpdate_InstDir.IsEmpty() && false == TASApp.IsStartingWOInstal() )
		{
			return;
		}

		CString HyUpdate;
		HyUpdate = m_strHyUpdate_InstDir + _T("HyUpdate.exe");
		StartProcess( HyUpdate, 2, ::AfxGetAppName(), _T("*Silent") );
	}
}

bool CTASelectApp::GetCompanyLogo( CEnBitmap *pBmp, CWnd *pWnd )
{
	if( NULL == pBmp )
		return false;
	// Create an offset to take into account the new TabMFC.
	if( NULL == pWnd )
		return false;
	CDC* pDC = pWnd->GetDC();
	int DeviceCaps = pDC->GetDeviceCaps( LOGPIXELSY );
	// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
	pWnd->ReleaseDC( pDC );

	if (DeviceCaps <= 96)
		pBmp->LoadBitmap( IDB_LOGOTA_SCREEN );
 	else if (DeviceCaps >= 144)
 		pBmp->LoadBitmap( IDB_LOGOTA_SCREEN_150 );
 	else
 		pBmp->LoadBitmap( IDB_LOGOTA_SCREEN_125 );

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CTASelectApp::_PrepareDocumentsFolderForDoc()
{
	m_strDocumentsFolderForDoc = _T("");

	// TODO: remove when HyUpdate will be changed. See comments in the '_PrepareDocumentsFolderForHelp' method.
	m_strDocumentsFolderForHelp = _T("");

	// When starting from an 'USB' kit or a folder without real installation document folder is located
	// into the sub folder documents.
	if( true == m_bStartingWOInstal )
	{
		m_strDocumentsFolderForDoc = m_strStartDir + CString( _T("documents") );
		SHCreateDirectoryEx( 0, ( LPCTSTR )m_strDocumentsFolderForDoc, NULL );
		return;
	}

	PWSTR pszPath = NULL;
	HRESULT hr = SHGetKnownFolderPath( FOLDERID_Documents, 0, NULL, &pszPath );

	if( SUCCEEDED( hr ) )
	{
		m_strDocumentsFolderForDoc = pszPath;

		// TODO: remove when HyUpdate will be changed. See comments in the '_PrepareDocumentsFolderForHelp' method.
		m_strDocumentsFolderForHelp = pszPath;

		CoTaskMemFree( pszPath );
	}
}

void CTASelectApp::_PrepareDocumentsFolderForHelp( CString strAppPath )
{
	// !!!!!!! Can't release that for HySelect v4.4.4.1. Because the actual version of HyUpdate always copy help
	// !!!!!!! files in the 'c:\Users\UserName' (current user) instead of the 'c:\User\Public' folder when the HySelect
	// !!!!!!! was installed for all users.

	/*
	m_strDocumentsFolderForHelp = _T("");

	if( -1 != strAppPath.Find( _T("taselect\\debug" ) ) || -1 != strAppPath.Find( _T( "taselect\\release") ) )
	{
		// If we are running on development, we take the current user documents folder.
		
		// Default path: %USERPROFILES%\Documents.
		PWSTR pszPath = NULL;
		HRESULT hr = SHGetKnownFolderPath( FOLDERID_Documents, 0, NULL, &pszPath );

		if( !SUCCEEDED( hr ) )
		{
			return;
		}

		m_strDocumentsFolderForHelp = pszPath;
		CoTaskMemFree( pszPath );
	}

	// When starting from an 'USB' kit or a folder without real installation document folder is located
	// into the sub folder documents.
	if( true == m_bStartingWOInstal )
	{
		m_strDocumentsFolderForHelp = m_strStartDir + CString( _T("documents") );
		SHCreateDirectoryEx( 0, ( LPCTSTR )m_strDocumentsFolderForHelp, NULL );
		return;
	}

	// Default path: %SystemDrive%\Program files (or %SystemDrive%\Program Files (x86) on 64 bits machine).
	PWSTR pszPath = NULL;
	HRESULT hr = SHGetKnownFolderPath( FOLDERID_ProgramFilesX86, 0, NULL, &pszPath );

	if( !SUCCEEDED( hr ) )
	{
		return;
	}

	CString strProgramFiles = pszPath;
	strProgramFiles.MakeLower();
	CoTaskMemFree( pszPath );

	if( -1 != strAppPath.Find( strProgramFiles ) )
	{
		// HySelect is launched from 'Program Files'. It means it's an installation that has been done for
		// all users. And then in this case the documents folder for help files is in the public.
			
		// Default path: %PUBLIC%\Documents.
		PWSTR pszPath = NULL;
		hr = SHGetKnownFolderPath( FOLDERID_PublicDocuments, 0, NULL, &pszPath );

		if( !SUCCEEDED( hr ) )
		{
			return;
		}

		m_strDocumentsFolderForHelp = pszPath;
		CoTaskMemFree( pszPath );
		return;
	}

	// Default path: %USERPROFILE%\AppData\Local.
	pszPath = NULL;
	hr = SHGetKnownFolderPath( FOLDERID_LocalAppData, 0, NULL, &pszPath );

	if( !SUCCEEDED( hr ) )
	{
		return;
	}

	CString strLocalAppData = pszPath;
	strLocalAppData.MakeLower();
	CoTaskMemFree( pszPath );

	if( -1 != strAppPath.Find( strLocalAppData ) )
	{
		// HySelect is launched from 'LocalAppData'. It means it's an installation that has been done for
		// the current user. And then in this case the documents folder for help files is in the user.
				
		// Default path: %USERPROFILES%\Documents.
		PWSTR pszPath = NULL;
		hr = SHGetKnownFolderPath( FOLDERID_Documents, 0, NULL, &pszPath );

		if( !SUCCEEDED( hr ) )
		{
			return;
		}

		m_strDocumentsFolderForHelp = pszPath;
		CoTaskMemFree( pszPath );
	}
	*/
}
