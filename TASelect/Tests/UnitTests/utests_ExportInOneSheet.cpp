#include "stdafx.h"

#include "utests_base.h"
#include "utests_ExportInOneSheet.h"
#include "taselect.h"
#include "Mainfrm.h"
#include "TASelectDoc.h"
#include "DeletedProductHelper.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_ExportInOneSheet, CPPUNIT_TEST_CATEGORYNAME_OUTPUT );

void utests_ExportInOneSheet::setUp()
{
	utests_base::setUp();
	utests_Init::InitDefaultDbValues();
	CString OutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_OUTPUT ), CString( __FILE__ ) );

	if( false == OutPath.IsEmpty() )
	{
		DeleteFilesAndFolder( OutPath, _T("*.*") );
	}
}

void utests_ExportInOneSheet::tearDown()
{
	utests_base::tearDown();
	utests_Init::RestoreDefaultDbValues();
}

void utests_ExportInOneSheet::TestExportInOneSheet()
{
	// List tsp file
	// Open tsp file
	// export selection in one sheet to xlsx file
	// compare content with reference xlsx

	// Get Data folder
	CFileFind finder;
	CString DirPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_OUTPUT ), CString( __FILE__ ), true );
	CString DirOutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_OUTPUT ), CString( __FILE__ ), false );

	// Delete previous result
	DeleteFilesAndFolder( DirOutPath, _T("*.xlsx") );

	if( DirPath.ReverseFind( '\\' ) != DirPath.GetLength() - 1 )
	{
		DirPath += CString( _T("\\") );
	}

	// For each tsp file
	BOOL bWorking = finder.FindFile( DirPath + _T("*.tsp") );

	while( bWorking )
	{
		bWorking = finder.FindNextFile();
		CString strTsp;

		try
		{

			if( finder.IsDots() )
			{
				continue;
			}

			strTsp = DirPath + finder.GetFileName();

			if( false == strTsp.IsEmpty() )
			{
				// File open
				// Reset flag modified.
				::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_OPENDOCUMENT, false, (LPARAM)&strTsp );
				LRESULT lresult = ::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_OPENDOCUMENT, false, (LPARAM)&strTsp );

				if( 0 == lresult )
				{
					CTableHM *pTableHM = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );

					if( NULL != pTableHM )
					{
						CDeletedProductHelper *pProductHelper = new CDeletedProductHelper();
						bool fDisplay = false;
						pProductHelper->VerifyHydroMod( pTableHM->GetIDPtr(), fDisplay );
						delete pProductHelper;

						// HYS-1191: force HM calc mode to 'true' otherwise the 'Freeze( false )' will have not effect.
						// Remark: when sending 'WM_USER_OPENDOCUMENT' we will get the 'CMainFrame::OnNewDocument()' method called.
						// And there, if activation code is expired or incorrect, the HMCalc mode will not be set.
						pMainFrame->SetHMCalcMode( true );
						pMainFrame->Freeze( false );
					}
				}

				// Export to xlsx.
				CString strXlsx = DirOutPath + finder.GetFileTitle() + _T(".xlsx");
				::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_EXPORTSELPINONETOXLSX, NULL, (LPARAM)&strXlsx );
				::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_EXPORTSELPINONETOXLSX, NULL, (LPARAM)&strXlsx );
			}

		}
		catch( CFileException *)
		{
			/*CString ErrorMsg = _T("Failed to export ") + strTsp;
			CPPUNIT_MESSAGE_H( ErrorMsg );*/// Can't be tested for now
		}
	}

	finder.Close();

	// For each xlsx file
	bWorking = finder.FindFile( DirPath + _T("*.xlsx") );

	while( bWorking )
	{
		bWorking = finder.FindNextFile();

		try
		{

			if( finder.IsDots() )
			{
				continue;
			}

			// Compare with reference file.
			utests_Init::CompareOutAndReferenceXLSXFiles( CString( CPPUNIT_TEST_CATEGORYNAME_OUTPUT ), CString( __FILE__ ), finder.GetFileTitle() + _T(".xlsx") );
		}
		catch( CFileException * )
		{
			/*CString ErrorMsg = _T("Something is different between ") + CString( __FILE__ ) + _T(" and ") + finder.GetFileTitle();
			CPPUNIT_MESSAGE_H( ErrorMsg );*/ // Can't be tested for now
		}
	}

	finder.Close();
}
