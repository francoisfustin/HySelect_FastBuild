#pragma once


#ifndef __AFXWIN_H__
#error "incluez 'stdafx.h' avant d'inclure ce fichier pour PCH"
#endif


#include "resource.h"       // main symbols
#include "Utilities.h"
#include "global.h"
#include "Units.h"
#include "Database.h"
#include "DataBObj.h"
#include "DataStruct.h"
#include "SetDpi.h"
#include "HydronicPic.h"
#include "EnBitmap.h"
#include "Select.h"
#include "RCImageManager.h"
#include "ssdllmfc.h"
#include "Tender.h"
#include "DocDb.h"
#include "PipeUserDatabase.h"
#include "HySelectException.h"

// CTASelectApp:
class CActivationWatcher;
class CTASelectApp : public CWinAppEx
{
	friend CActivationWatcher;

public:
	CTASelectApp();
	~CTASelectApp();

	CRCImageManager *GetpRCImageManager( )
	{
		return m_pclRCImageManager;
	}

	UINT GetAppLook()
	{
		return m_uiAppLook;
	}
	
	void SetAppLook( UINT uiAppLook )
	{
		m_uiAppLook = uiAppLook;
	}

	CTender& GetTender()
	{
		return m_Tender;
	}

	CDocDb& GetDocDb()
	{
		return m_DocDb;
	}

	CDocDb& GetDocDbEn()
	{
		return m_DocDbEn;
	}

	// DataBase access.
	CTADatabase *GetpTADB()
	{
		return m_pTADB;
	}

	// User DataBase access.
	CUserDatabase *GetpUserDB()
	{
		return m_pUSERDB;
	}

	// Pipe DataBase access.
	CPipeUserDatabase *GetpPipeDB()
	{
		return m_pPIPEDB;
	}

	// DataStruct access.
	CTADatastruct *GetpTADS()
	{
		return m_pTADS;
	}

	// UnitDataBase access.
	CUnitDatabase *GetpUnitDB()
	{
		return CDimValue::AccessUDB();
	}

	CString GetLocalArtNumberName()
	{
		return m_strNameLocalArtNumber;
	}

	// Return true if Local Art number is used.
	bool IsLocalArtNumberUsed()
	{
		return m_bLocalArtNumber;
	}

	// Return true if the TA-SCOPE data base files must keep the last update version.
	bool IsTAScopeFilesMaintain()
	{
		if( m_uiMaintainTAScopeFiles )
		{
			return true;
		}

		return false;
	}
	void SetTAScopeFilesMaintain( UINT ui )
	{
		m_uiMaintainTAScopeFiles = ui;
	}

	void SetTrvUsed( bool flag )
	{
		m_bTrvUsed = true;
	}

	bool IsTrvDisplayed() { return m_bTrvUsed; }
	bool IsDpCDisplayed() { return m_bDpCUsed; }
	bool IsHubDisplayed() { return m_bHubUsed; }
	bool IsCvDisplayed() { return m_bCvUsed; }
	bool IsPICvDisplayed() { return m_bPICvUsed; }
	bool IsBCvDisplayed() { return m_bBCvUsed; }

	// HYS-2110
	void SetDpCBCVUsed( bool flag ) { m_bDpCBCVUsed = flag; }
	bool IsDpCBCVDisplayed() { return m_bDpCBCVUsed; }
	bool Is6WayCVDisplayed() { return m_b6WayCVUsed; }
	bool IsShutOffValveDisplayed() { return m_bShutOffValveUsed; }
	bool IsAirVentSepDisplayed() { return m_bAirVentSepUsed; }
	bool IsPressureMaintenanceDisplayed() { return m_bPressureMaintenanceUsed; }
	bool IsSafetyValveDisplayed() { return m_bSafetyValveUsed; }
	bool IsSmartControlValveDisplayed() { return m_bSmartControlValveUsed; }
	bool IsSmartDpCDisplayed() { return m_bSmartDpUsed; }
	bool IsTapWaterControlDisplayed() { return m_bTapWaterControlUsed; }
	bool IsFloorHeatingControlDisplayed() { return m_bFloorHeatingControlUsed; }
	bool IsPMPriceIndexUsed() { return m_bPMPriceIndexUsed; }
	bool IsAutoUpdate() { return m_bAutoUpdate; }
	bool IsAutoSave() { return m_bAutoSave; }
	bool IsTenderText() { return m_bTenderText; }
	bool IsGAEBExport() { return m_bGAEBExport; }

	void SetAutoUpdateflag( bool bVal )
	{
		m_bAutoUpdate = bVal;
	}
	void SetAutoSaveflag( bool bVal )
	{
		m_bAutoSave = bVal;
	}
	void SetTenderText(bool bVal)
	{
		m_bTenderText = bVal;
	}
	void SetGAEBExport(bool bVal)
	{
		m_bGAEBExport = bVal;
	}
	void SetUnitTest( bool bVal )
	{
		m_fUnitTest = bVal;
	}
	void SetWizardMode(bool bVal)
	{
		m_bWizardMode = bVal;
	}
	bool IsLogRawData() const
	{
		return m_bLogRawData;
	}
	bool IsStartingWOInstal() const
	{
		return m_bStartingWOInstal;
	}
	bool IsTestInRelease() const
	{
		return m_fTestInRelease;
	}
	bool IsUnitTest() const
	{
		return m_fUnitTest;
	}
	bool IsWizardMode() const
	{
		return m_bWizardMode;
	}

	// Return CV family, could be Hora, Brazil, ...
	CString GetCvFamily()
	{
		return m_strCvFamily;
	}

	CString GetStartDir() const
	{
		return m_strStartDir;
	}

	// Return 'false' if there is an error.
	void LoadFlags();

	bool IsStarted()
	{
		return m_bAppStarted;
	}
	bool IsDebugComActif()
	{
		return m_bDebugCom;
	}
	void SetDebugCom( bool flag = true )
	{
		m_bDebugCom = flag;
	}

	HMODULE GetDLLHandle( const TCHAR *pIDstr );

	// Return 'true' or 'false' depending if the price is used into the application.
	bool IsPriceUsed()
	{
		return m_bPriceUsed;
	}
	bool IsHubPriceUsed()
	{
		return m_bHubPriceUsed;
	}

	// Return 'true' or 'false' depending if the user enter the hidden password.
	bool IsHidingPswSet()
	{
		return m_fHidingPswSet;
	}
	
	void SetHidingPsw( bool fFlag )
	{
		m_fHidingPswSet = fFlag;
	}

	// Workaround function: in HydronicCalc return TBV-CM authority based on Dp at full opening.
	bool IsTBVCMAuthOnFullDp()
	{
		return m_bTBVCMAuthOnFullDp;
	}

	// Return the factor between current font and standard font in %.
	int GetFontFactor( CWnd *pWnd );

	// Scan all TAProduct to compute SortingKey, used only into READ TADB.txt.
	// Store computed value into TADB.db, increase booting time of TACBX.
	void ComputeTAPSortingKey( CTable *pTab = NULL );

	bool IsTAArtNumberHidden()
	{
		if( true == IsLocalArtNumberUsed() )
		{
			return m_bHideTAArtNumber;
		}
		else
		{
			return false;
		}
	}

	// Language strings and localization information.
	CString GetTADBKey()
	{
		return m_strLanguageKey;
	}
	CString GetLanguage()
	{
		return m_strLanguage;
	}
	CString GetNextLanguage()
	{
		return m_strNextLanguage;
	}

	// Allow to save what is the choice of the user concerning the TASelect language for the next reboot.
	void SetNextLanguage( CString str )
	{
		m_strNextLanguage = str.MakeLower();
	}

	// Returns what is the language choose by user for documentation.
	CString GetDocLanguage( void );

	// Returns what is the language choose by user for documentation.
	void SetDocLanguage( CString strDocLanguage );

	CString LoadLocalizedString( UINT uiID );

	// Return a CString with the installation directory for the HyUpdate.
	CString GetHyUpdateInstDir()
	{
		return m_strHyUpdate_InstDir;
	}
	
	void StartProcess( CString strExeName, int iParamCount, ... );

	// Can be called when user explicitly chooses open file in main menu OR can be called by 'TASelectDoc::OnOpenDocument' when user
	// opens file by MRU list (or using drag & drop method).
	// 1st case called by 'CTASelectDoc::OnFileOpen' and 'CDlgImportHM::OnBnClickedButtonopenproj' methods.
	// 2nd case called by 'CTASelectDoc::OnOpenDocument' method.
	// In 1st case: 'lpszPathName' MUST be set to NULL -> We don't know yet the file name because we ruen the 'CFileDialog' dialog to allow user to search file to open.
	// In 2nd case: 'pTADS' MUST be set to NULL.
	CString FileOpen( CTADatastruct *pTADS, LPCTSTR lpszPathName );

	// Retrieve the number of characters accepted depending on the project params (TA-CBI compatible or not).
	WCHAR GetSiteNameMaxChar();
	WCHAR GetModuleNameMaxChar();

	// Display NewIn html page if needed, return true if it has been displayed.
	bool ShowNewInHtmlPage( bool fAppStarting = true );

	// Print methods.
	void LaunchPrintSetupDlg()
	{
		CWinAppEx::OnFilePrintSetup();
	}
	HGLOBAL GetHDevMode()
	{
		return m_hDevMode;
	}

	/// DLL access.
	HMODULE GetHMHubSchemes()
	{
		return m_hmHubSchemes;
	}
	HMODULE GetHMProdPics()
	{
		return m_hmProdPics;
	}

	CEvent *GetpInstanceEvent()
	{
		return m_pInstanceEvent;
	}
	CEvent *GetpShutdownEvent()
	{
		return m_pShutdownEvent;
	}

	// Pump Message method.
	static void PumpMessages();

	// Return true the local prod pic dll is loaded.
	bool IsLocalProdPicDllLoaded()
	{
		if( m_hmLocProdPics )
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void SetProdPicType( CProdPic::eProdPicType eType )
	{
		m_eProdPicType = eType;
	}

	CProdPic::eProdPicType GetProdPicType()
	{
		return m_eProdPicType;
	}

	// Return a CProPic object and fill the CEnBitmap object if it exist.
	// In case the Pictype or the LocSymboltype doesn't exist and you have choose that: the function retrieve the TASymbol pic.
	// You can verify what the function return through the CProPic object.
	CProdPic *GetpProdPic( LPCTSTR DescriptorID, CEnBitmap *pEnBmp = NULL );
	CDynCircSch *GetDynCircSch( LPCTSTR );

	// Verify the PX_DescriptorID for one Product exist in the DLL.
	bool VerifyPicExist( LPCTSTR DescriptorID );

	COLORREF GetBckColor()
	{
		return m_BckColor;
	}
	void SetBckColor( COLORREF colref )
	{
		m_BckColor = colref;
	}

	void OverridedSetRedraw( CWnd *pWnd, BOOL fSetRedraw );

	//////////////////////////////////////////////////////////////////////////////
	// MessageBox helpers
	LRESULT DisplayLocalizeMessageBox( WPARAM nIDPrompt, LPARAM nType = MB_OK)
	{
		return (::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_DISPLAYMSGBOX, nIDPrompt, nType ));
	}

	int AFXAPI AfxLocalizeMessageBox( UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = ( UINT ) - 1 )
	{
		CString str = LoadLocalizedString( nIDPrompt );
		return AfxMessageBox( ( LPCTSTR )str, nType, nIDHelp );
	}
	/////////////////////////////////////////////////////////////////////////////

	bool GetCompanyLogo( CEnBitmap *pBmp, CWnd *pWnd );

	//////////////////////////////////////////////////////////////
	// Debug Console
	// How to use it
	// ((CMyApp *)AfxGetApp())->StartDebugConsole(80,25,"essai.txt");
	// for(int i=0;i<5;i++)
	//	 ((CMyApp *)AfxGetApp())->DebugPrintf("Txt Ligne:%d",i);
	// Start trace debug
	bool StartDebugConsole( int iWidth, int iHeight, const TCHAR *pszfname = NULL );
	void CloseDebugConsole();
	// Line feed
	void DebugNewLine( void );

	// Use DebugPrintf like TRACE0, TRACE1, ... (The arguments are the same as printf)
	void DebugPrintf( CString s );
	//////////////////////////////////////////////////////////////

	virtual void PreLoadState();
	BOOL LoadPipesFromFile();

	CString GetDocumentsFolderForDoc() { return m_strDocumentsFolderForDoc; }
	CString GetDocumentsFolderForHelp() { return m_strDocumentsFolderForHelp; }
	CString GetDataPath();
	CString GetLogsPath();
	CString GetDocBasePath();
	CString GetProjectPath();
	CString GetAutoSavePath();

	virtual int Run();

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL PreTranslateMessage( MSG *pMsg );

	bool GenerateTASCopeDBX();
	bool ReadTADBTxT();

	// Create and insert tables: PAGE_SETUP_TAB, PAGE_FIELD_TAB, VARIOUS_TAB, GLOSSARY_TAB and RADNOMCOND_TAB in m_USERDB if they do not exist yet.
	// Create the PIPE_TAB table in m_USERDB if it does not exist yet and introduce in it all pipes from m_TADB.
	void PrepareUserDb();

	// Delete all fixed pipes and associated tables from m_USERDB.
	// Delete all fixed User Product and associated tables from m_USERDB.
	void CleanPipeUserDB();

	// Create compound products Body + Inlet + Outlet.
	void CreateCompoundTAP( CTable *pTab, LPCTSTR lpstrClass );

	// Generate *.txt files with information on the products's filtering.
	void CreateFilterViewTxT();
	void FillProductMap( CTable *pTab, std::multimap<int, CDB_TAProduct *> &MyMultiMap );


	// Private methods.
private:
	//void _DocDBFindMissingDBEntry( CFileTxt *pTxtFile, CString strDBLanguage, bool fMain = false, CTable *pTab = NULL, CList<CString> *pList = NULL );
	//void _DocDBFindMissingFiles( CFileTxt *pTxtFile, CString strDBLanguage, bool fMain = false, CTable *pTab = NULL, CList<CString> *pExistingIDList = NULL );
	void _ShowTipAtStartup( void );
	void _SilentUpdate();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Retrieve the directory where are saved the documentation files of the IMI products. </summary>
	///
	/// <remarks>	For the moment, this folder is always in 'c:\Users\UserName\Documents'. </remarks>
	///
	/// <returns>	The documentation directory. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void _PrepareDocumentsFolderForDoc();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Retrieve the directory where are saved the help files of the IMI products. </summary>
	///
	/// <remarks>	help files are the 'newin.htm' (in the root), the 'Qrs_none.htm', 'readme.htm' and 
	///             'HySelect user guide.pdf' (when available) for each country.
	///
	///             From HySelect v4.4.4.1, the new NSIS script writes help folder in: 
	///
	///                 - 'c:\Users\UserName\Documents' when installation is done for the current user.
	///                 - 'c:\Users\Public\Documents' when installation is done for all users. </remarks>
	///
	/// <returns>	The documentation directory. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void _PrepareDocumentsFolderForHelp( CString strAppPath );

// Protected variables.
protected:
	// HYS-1700: now if use double-clicks a .tsp file to open it, we will open when 'InitInstance' has finished.
	CString m_strFileToOpen;

	HMODULE m_hmHubSchemes;
	HMODULE m_hmLanguageRes;
	HMODULE m_hmProdPics;
	HMODULE m_hmLocProdPics;
	HINSTANCE m_hspread;
	ULONG_PTR m_gdiplusToken;

	CTADatabase *m_pTADB;
	CUserDatabase *m_pUSERDB;
	CPipeUserDatabase *m_pPIPEDB;
	CTADatastruct *m_pTADS;

	bool m_bDebugCom;
	bool m_bLogRawData;
	bool m_fTestInRelease;
	bool m_fUnitTest;
	bool m_bRegisterRunning;		// Program has been called with the "Register" argument by the DBBuilder.
	bool m_bWizardMode;
	bool m_bAutoUpdate;		// flag set to 1 when auto update procedure enabled
	bool m_fHidingPswSet;	// flag set to 1 when hiding password has been entered
	bool m_bPriceUsed;
	bool m_bHubPriceUsed;
	bool m_bAutoSave;
	bool m_bTenderText;
	bool m_bGAEBExport;
	bool m_bTrvUsed;
	bool m_bDpCUsed;
	bool m_bHubUsed;
	bool m_bCvUsed;
	bool m_bPICvUsed;
	bool m_bBCvUsed;
	bool m_bDpCBCVUsed;
	bool m_b6WayCVUsed;
	bool m_bShutOffValveUsed;
	bool m_bAirVentSepUsed;
	bool m_bPressureMaintenanceUsed;
	bool m_bPMPriceIndexUsed;
	bool m_bSafetyValveUsed;
	bool m_bSmartControlValveUsed;
	// HYS-1935: TA-Smart Dp - 02 - Add a new category for the ribbon
	bool m_bSmartDpUsed;
	bool m_bTapWaterControlUsed;
	bool m_bFloorHeatingControlUsed;
	bool m_bTBVCMAuthOnFullDp;
	bool m_bReadTADBTxt;
	bool m_bLocalArtNumber;
	bool m_bHideTAArtNumber;
	bool m_bAppStarted;
	bool m_bUpdateRegistryStartingFolder;		// Used to avoid to replacing registry installation folder, used for debugging for instance
	bool m_bStartingWOInstal;
	UINT m_uiMaintainTAScopeFiles;

	CString m_strLanguageKey;
	CString m_strLanguage;
	CString m_strNextLanguage;
	CString m_strDocLanguage;
	CString m_strNameLocalArtNumber;
	CString m_strStartDir;
	CString m_strCrossVerifyDocLangKey;
	CString	m_strHyUpdate_InstDir;
	CString m_strCvFamily;
	CString m_strDocumentsFolderForDoc;
	CString m_strDocumentsFolderForHelp;

	CTender m_Tender;
	CDocDb m_DocDb;
	CDocDb m_DocDbEn;

	CProdPic::eProdPicType m_eProdPicType;

	CEvent *m_pInstanceEvent;
	CEvent *m_pShutdownEvent;
	CActivationWatcher *m_pActivationWatch;

	HANDLE m_hStdOut;
	CStdioFile m_stdFileDebug;

	COLORREF m_BckColor;

	typedef std::map<int, IDPTR> mapQrs;
	mapQrs m_mQrsList;
	UINT  m_uiAppLook;

	CRCImageManager *m_pclRCImageManager;
};

extern CTASelectApp TASApp;
