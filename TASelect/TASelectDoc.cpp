//
// TASelectDoc.cpp : implementation of the CTASelectDoc class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <afxpriv.h> 
#include "global.h"
#include "Utilities.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "ModuleVersion.h"
#include "DlgTSPOpeningInfo.h"
#include "DlgOutput.h"
#include "DlgHMCompilationOutput.h"
#include "RViewSSelSS.h"
#include "DlgSelectionBase.h"
#include "DlgLeftTabSelManager.h"
#include "TASelectDoc.h"
#include "DlgTechParam.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTASelectDoc
CTASelectDoc *pTASelectDoc = NULL;

IMPLEMENT_DYNCREATE( CTASelectDoc, CDocument )

CTASelectDoc::CTASelectDoc()
{
	pTASelectDoc = this;
}

CTASelectDoc::~CTASelectDoc()
{
}

BEGIN_MESSAGE_MAP( CTASelectDoc, CDocument )
	ON_COMMAND( ID_APPMENU_OPEN, OnFileOpen )
	ON_COMMAND( ID_APPMENU_SAVE, OnFileSave )
	ON_COMMAND( ID_APPMENU_NEW, OnFileNew )
	ON_COMMAND( ID_APPMENU_SAVE_AS, OnFileSaveAs )
	ON_MESSAGE( WM_USER_CLOSEAPPLICATION, (LRESULT (CWnd::* )(WPARAM,LPARAM) )OnCloseApplication )
END_MESSAGE_MAP()

#ifdef _DEBUG
void CTASelectDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTASelectDoc::Dump( CDumpContext& dc ) const
{
	CDocument::Dump( dc );
}
#endif //_DEBUG

void CTASelectDoc::AutoSave()
{
	if( false == ( (CTASelectApp*)AfxGetApp() )->IsAutoSave() )
	{
		return;
	}
	
	CString str = TASApp.LoadLocalizedString( IDS_FILENONAME );
	CString	strInstallationDir = TASApp.GetStartDir();
	CString strProjectDir= GetPathName();
	CPath path( strProjectDir );

	// Keep only filename.
	strProjectDir = path.SplitPath( CPath::ePathFields::epfFName );
	
	CString UserFolder = TASApp.GetDocumentsFolderForDoc();
	CString strAutoSaveDir = UserFolder + CString( _HYSELECT_NAME_BCKSLASH ) + TASApp.GetTADBKey() + CString( _T("\\") ) + _PROJECTS_DIRECTORY + _AUTOSAVE_DIRECTORY;

	// Create AutoSave directory if needed.
	int iError = SHCreateDirectoryEx( NULL, (LPCTSTR)strAutoSaveDir, NULL );
	if( ERROR_SUCCESS == iError || ERROR_FILE_EXISTS == iError || ERROR_ALREADY_EXISTS == iError )
	{
		// Add filename.
		strAutoSaveDir += strProjectDir;
		CString strFileExtension = AUTOSAVINGEXTENSION;

		// If opened file is an Autosaved file...
		int iPos = strAutoSaveDir.Find( CString( AUTOSAVINGEXTENSION ) );
		if( iPos > 0 )
		{
			// Retrieve index if exist.
			int i = strAutoSaveDir.ReverseFind( '(' );
			int j = strAutoSaveDir.ReverseFind( ')' );
			int iVal = 1;
			CString iStr;
			if( i > iPos && i < j )
			{
				iStr = strAutoSaveDir.Mid( i + 1, j - i - 1 );
				iVal = _ttoi( (LPCTSTR)iStr );
				iVal++;
				
				// Remove '(xx)'.
				strAutoSaveDir.Delete( i, strAutoSaveDir.GetLength() - i );
			}
			iStr.Format( _T("(%d)"), iVal );
			strFileExtension = iStr;
		}

		CString strFilename = strAutoSaveDir + strFileExtension + _T(".tsp");
		FileSave((LPCTSTR) strFilename, true);
	}
}

void CTASelectDoc::SetTitle( LPCTSTR lpszTitle )
{
	CDocument::SetTitle( lpszTitle );
}

bool CTASelectDoc::CloseTASelect()
{
	// In case of NewFile take into account sseltab modification and store it into the registry 
	CString	str = TASApp.LoadLocalizedString( IDS_FILENONAME );

	if( NULL != pDlgLeftTabSelManager && GetPathName().Find( str ) >= 0 )								 //	Default name 
	{
		pDlgLeftTabSelManager->WriteAllSSelParamsToRegistry();
	}

	if( (TASApp.GetpTADS() )->IsModified() && false == TASApp.IsUnitTest() )
	{
		// Warning message file not saved.
		CString str;
		FormatString( str, AFXMSG_SAVECURRENTFILE, GetPathName() );
		int iReturn = AfxMessageBox( str, MB_YESNOCANCEL | MB_ICONQUESTION, 0 );
		if( IDCANCEL == iReturn )
			return false;
		if( IDYES == iReturn )
		{
			OnFileSave();
		}
		// HYS-1537 : Save parameters in registry even if the file is not saved
		// If the user close the application with a file open, and this file has been modified
		// Ssel Tab parameters will be replaced by the new ones.
		if( NULL != pDlgLeftTabSelManager )
		{
			pDlgLeftTabSelManager->WriteAllSSelParamsToRegistry();
		}
	}
	
	TASApp.GetpTADS()->Modified( false );
	return true;
}

BOOL CTASelectDoc::OnNewDocument()
{
	if( FALSE == CDocument::OnNewDocument() )
	{
		return FALSE;
	}
	
	g_bSBCSFile = false;
	
	// Reinitialization of the data structure.
	try
	{
		TASApp.GetpTADS()->Init();
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CTASelectDoc::OnNewDocument'."), __LINE__, __FILE__ );
		return FALSE;
	}
	
	// Reset flag modified, this is a new file.
	TASApp.GetpTADS()->Modified( false );

	// Load default file name.
	CString str = TASApp.LoadLocalizedString( IDS_FILENONAME );
	CString PrjDir = GetProjectDirectory();
	PrjDir = PrjDir + str;

	// Set current file to No Name.
	try
	{
		SetPathName( (LPCTSTR)PrjDir, FALSE );
	}
	catch(...){};
	// !!! Message to inform about creation of a new document 
	// !!! sent in the OnInitialUpdate() of the views

	return TRUE;
}

BOOL CTASelectDoc::OnOpenDocument( LPCTSTR lpszPathName )
{
	if( FALSE == CDocument::OnOpenDocument( lpszPathName ) )
	{
		return FALSE;
	}

	CTADatastruct *pTADS = TASApp.GetpTADS();
	CDlgHMCompilationOutput::CHMInterface clOutputInterface;
	clOutputInterface.ClearOutput();
	
	// If the application is already running...
	if( NULL != ::AfxGetApp()->m_pMainWnd )
	{
		// When the user DblClick on a tsp doc this test is not necessary.
		if( pTADS->IsModified() && false == TASApp.IsUnitTest() )
		{
			// Warning message file not saved.
			CString	str;
			FormatString( str, AFXMSG_SAVECURRENTFILE, GetPathName() ); 

			if( IDYES == AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION, 0 ) )
			{
				OnFileSave();
			}
		}
	}

	BOOL bReturn = FALSE;
	CString path = TASApp.FileOpen( NULL, lpszPathName );

	if( false == path.IsEmpty() )
	{
		// Reset flag modified.
		pTADS->Modified(false);
		
		// Reset the upper and lower limits for measurements.
		pMainFrame->ResetMeasurementsLimits();

		// Send a message new document is open; only if the applicationis already running, not for the *.tsp dblclick.
		if( ::AfxGetApp()->m_pMainWnd )
		{
			// Display a dialog box with information on the *.tsp file for deleted and not available valves.

			// HYS-1583: even in unit test mode we need to launch this dialog because there is a verification on the
			// individual selection and HM calc if exist. And we can have errors. For example, the PR 6000 Filter product 
			// has been removed from the database but not put in the deleted file. So when opening in debug mode there is
			// an ASSERT in the 'DCDlgTSPOpeningInfo' dialog. We need also this ASSERT when running unit test.
			CDlgTSPOpeningInfo dlg;
			dlg.Display( TASApp.IsUnitTest() );

			::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_NEWDOCUMENT );
			::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_NEWDOCUMENT );
		}

		bReturn = TRUE;
	}
	
	return bReturn;
}

void CTASelectDoc::OnCloseDocument() 
{
	CTADatastruct *pTADS = TASApp.GetpTADS();
	pTADS->CleanClipboard();
	AutoSave();

	if( 1 == pTADS->IsModified() && false == TASApp.IsUnitTest() )
	{
		// Warning message file not saved.
		CString str;
		FormatString( str, AFXMSG_SAVECURRENTFILE, GetPathName() );

		if( IDYES == AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION, 0 ) )
		{
			OnFileSave();
		}
	}
	
	CDocument::OnCloseDocument();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CTASelectDoc::OnFileOpen()
{
	CTADatastruct* pTADS = TASApp.GetpTADS();
	CDlgHMCompilationOutput::CHMInterface clOutputInterface;
	clOutputInterface.ClearOutput();

	// If current file is modified...
	if( pTADS->IsModified() )
	{
		// Warning message file not saved.
		CString	str;
		FormatString( str, AFXMSG_SAVECURRENTFILE, GetPathName() );

		if( IDYES == AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION, 0 ) )
		{
			OnFileSave();
		}
	}
	
	CString path = TASApp.FileOpen( TASApp.GetpTADS(), NULL );

	if( false == path.IsEmpty() )
	{
		// Reset flag modified.
		pTADS->Modified( false );
		
		// Reset the Upper and Lower limits for measurements.
		pMainFrame->ResetMeasurementsLimits();

		// Add current file to the list of recent files.
		SetPathName( (LPCTSTR)path, TRUE );
		
		// Display a dialog box with information on the *.tsp file for deleted and not available valves.
		
		// HYS-1583: even in unit test mode we need to launch this dialog because there is a verification on the
		// individual selection and HM calc if exist. And we can have errors. For example, the PR 6000 Filter product 
		// has been removed from the database but not put in the deleted file. So when opening in debug mode there is
		// an ASSERT in the 'DCDlgTSPOpeningInfo' dialog. We need also this ASSERT when running unit test.
		CDlgTSPOpeningInfo dlg;
		dlg.Display( TASApp.IsUnitTest() );

		::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_NEWDOCUMENT );
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_NEWDOCUMENT );
	}
}

void CTASelectDoc::OnFileSave()
{
	// Save TADS file with current name, if current name is default name call FileSaveAs.
	CString str = TASApp.LoadLocalizedString( IDS_FILENONAME );

	if( GetPathName().Find( str ) >= 0 )
	{
		OnFileSaveAs();
	}
	else if( g_bSBCSFile )
	{
		//	SBCS file saved as Unicode version.

		// Current file name.
		CString tmp = GetPathName();
		tmp = tmp.Right( tmp.GetLength() - tmp.ReverseFind( '\\' ) - 1 );

		CString str;
		FormatString( str, AFXMSG_SBCSTSPSAVETOUNICODE, tmp );
		AfxMessageBox( str, MB_OK | MB_ICONINFORMATION, 0 );
		OnFileSaveAs();
	}
	else
	{
		FileSave( ( LPCTSTR )GetPathName() );
	}
}

void CTASelectDoc::OnFileNew() 
{
	try
	{
		BeginWaitCursor();

		CTADatastruct *pTADS = TASApp.GetpTADS();
		CDlgHMCompilationOutput::CHMInterface clOutputInterface;
		clOutputInterface.ClearOutput();

		// If current file is modified...
		if( pTADS->IsModified() )
		{
			// Warning message file not saved.
			CString	str;
			FormatString( str, AFXMSG_SAVECURRENTFILE, GetPathName() );
			
			if( IDYES == AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION, 0 ) )
			{
				OnFileSave();
			}
		}

		// Write all technical parameter values in the registry.
		// HYS-1590: We extract the writing in registry from all the technical parameters dialogs to put 
		// it in only one method. For a future Jira card (HYS-1631) we will try to put all the writings in registry
		// in this method.
		pTADS->WriteAllDataInRegistry();

		// To be sure to preserve current user combo selection.
		if( NULL != pDlgLeftTabSelManager )
		{
			pDlgLeftTabSelManager->WriteAllSSelParamsToRegistry();
		}
		
		// Clear current doc.
		DeleteContents();	
		
		// HYS-1146: If the current product gallery is 6way valve, we have to keep changeOver application type.
		// This boolean allow to get correct changeOver initial values.
		bool bSwitchAppTypeAndWc = false;
		
		// HYS-1146
		if( ChangeOver == pTADS->GetpTechParams()->GetProductSelectionApplicationType() )
		{
			bSwitchAppTypeAndWc = true;
		}

		// Reinitialization of the data structure.
		TASApp.GetpTADS()->Init();
		
		// The init function change application type changeOver to heating we have to keep changeOver if the 
		// selected product gallery is 6way valve.
		if( true == bSwitchAppTypeAndWc )
		{
			pTADS->GetpTechParams()->SetProductSelectionApplicationType( ChangeOver );
			CWaterChar *pWC = pTADS->GetpWCForProductSelection()->GetpWCData();
			*pWC = *( pTADS->GetpTechParams()->GetDefaultISChangeOverWC( CoolingSide ) );
		}

		CString str = TASApp.LoadLocalizedString( IDS_FILENONAME );
		CString PrjDir = GetProjectDirectory();
		PrjDir = PrjDir + str;

		// Reset the upper and lower limits for measurements.
		pMainFrame->ResetMeasurementsLimits();

		// Reload pipe DB if temporary is used.
		if( false == TASApp.GetpPipeDB()->GetSaveDB() )
		{
			// Delete all temporary pipes.
			TASApp.GetpPipeDB()->DeletePipeTab();

			// Load pipes.db.
			TASApp.LoadPipesFromFile();

			// Import pipes from USERDB in the case it still contains PIPE_TAB.
			// Needed to upgrade from old versions.
			TASApp.GetpPipeDB()->CopyFrom( TASApp.GetpUserDB() );

			// Remove pipes from USERDB because only PIPEDB is used now.
			TASApp.GetpPipeDB()->DeletePipeTab( TASApp.GetpUserDB() );

			// Loop on Pipe series tables to introduce them in m_PIPEDB.
			TASApp.GetpPipeDB()->CopyFrom( TASApp.GetpTADB() );

			// Verify IDPTR of pipe stored in m_PIPEDB.
			TASApp.GetpPipeDB()->CheckPipeTab();
		}

		// Set current file to No Name blank file.
		SetPathName( PrjDir, FALSE );

		::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_NEWDOCUMENT );
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_NEWDOCUMENT );
		pTADS->Modified( false );

		EndWaitCursor();
	}
	catch( CHySelectException &clHySelectException )
	{
		EndWaitCursor();
		clHySelectException.AddMessage( _T("Error in 'CTASelectDoc::OnFileNew'."), __LINE__, __FILE__ );
		throw;
	}
}

void CTASelectDoc::OnFileSaveAs()
{
	CString PrjDir = GetProjectDirectory();

	// Load file filter.
	CString str = TASApp.LoadLocalizedString( IDS_OPENSAVEFILEFILTER );
	CFileDialog dlg( false, _T("tsp"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, ( LPCTSTR )str, NULL );
	dlg.m_ofn.lpstrInitialDir = ( LPCTSTR )PrjDir;

	if( IDOK == dlg.DoModal() )
	{
		// Save project directory if modified...
		if( dlg.GetPathName() != PrjDir )
		{
			CPath path( dlg.GetPathName() );
			CString str = path.SplitPath( ( CPath::ePathFields )( CPath::ePathFields::epfDrive + CPath::ePathFields::epfDir ) );
			AfxGetApp()->WriteProfileString( _T("Files" ), _T( "Project Directory"), str );
		}

		FileSave( ( LPCTSTR )dlg.GetPathName() );
		SetPathName( ( LPCTSTR )dlg.GetPathName(), TRUE );
		g_bSBCSFile = false;
	}
}

void CTASelectDoc::FileSave( LPCTSTR lpszPathName, bool bAutoSave )
{
	CTADatastruct *pTADS = TASApp.GetpTADS();
	
	BYTE bIsModified = pTADS->IsModified();

	// Update CDS with current Tab selection.
	if( NULL != pDlgLeftTabSelManager )
	{
		pDlgLeftTabSelManager->SaveAllSSelParamsToTADS();
	}

	if( true == bAutoSave )
	{
		// HYS-915: Don't make the auto save if the datastruct is not modified.
		if( 1 == bIsModified )	
		{	
			// Update CTADatastruct with current user pipes (Only those that are used in the project).
			pTADS->AddUsedUserPipesBeforeSavingProject();

			TRACE( _T("Autosave the TADS in binary file %s.\n"), lpszPathName );

			CTADatastruct *pTADSfoo = new CTADatastruct();

			if( NULL == pTADSfoo )
			{
				ASSERT_RETURN;
			}

			try
			{
				pTADS->DuplicateDataBaseTO( pTADSfoo );
			}
			catch( CHySelectException &clHySelectException )
			{
				clHySelectException.AddMessage( _T("Error in 'CTASelectDoc::FileSave'."), __LINE__, __FILE__ );
				ASSERT_RETURN;
			}

			pTADSfoo->CleanClipboard();
			pTADSfoo->VerifyDataSetIntegrity();

			try
			{
				pTADSfoo->SetVersion( CTADATASTRUCT_FORMAT_VERSION );
				std::ofstream outf( (LPCTSTR)lpszPathName, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc );
				pTADSfoo->Write( outf );
				outf.close();
			}
			catch (...)
			{
				TRACE( _T("Error during write of the TADS in binary file %s.\n"), lpszPathName );
				ASSERT( 0 );
			}

			pTADSfoo->MakeEmpty();
			delete pTADSfoo;

			// Remove PIPE_TAB from TADS to avoid non-synchronized state.
			TASApp.GetpPipeDB()->DeletePipeTab( pTADS );
		}
	}
	else
	{
		// Update CDS with current User Pipes.
		// 'true' to not copy the fixed pipes (The ones that are defined in the "Pipedb.txt").
		// We then copy only the user pipes.

		// HYS-1590: We need to save ONLY user pipes that are really used in the project.
		pTADS->AddUsedUserPipesBeforeSavingProject();

		bIsModified = 0;
		pTADS->CleanClipboard();

		TRACE( _T("Write the TADS in binary file %s.\n"), lpszPathName );
		
		// In debug mode, allow to verify if there is no orphans in 'CDataBase::m_DataSet'.
		pTADS->VerifyDataSetIntegrity();

		try
		{
			pTADS->SetVersion( CTADATASTRUCT_FORMAT_VERSION );
			std::ofstream outf( lpszPathName, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc );
			pTADS->Write( outf );
			outf.close();
		}
		catch (...)
		{
			TRACE( _T("Error during write of the TADS in binary file %s.\n"), lpszPathName );
			ASSERT( 0 );
		}

		// Remove PIPE_TAB from TADS to avoid non-synchronized state.
		TASApp.GetpPipeDB()->DeletePipeTab( pTADS );
	}

	// Reset flag modified.
	pTADS->Modified( bIsModified );

}

LRESULT CTASelectDoc::OnCloseApplication( WPARAM wParam, LPARAM lParam )
{
	return false;
}
