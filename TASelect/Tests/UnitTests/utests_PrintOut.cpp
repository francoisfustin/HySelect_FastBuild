#include "stdafx.h"

#include "utests_base.h"
#include "utests_Printout.h"
#include "taselect.h"
#include "Mainfrm.h"
#include "TASelectDoc.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_Printout, CPPUNIT_TEST_CATEGORYNAME_OUTPUT );

void utests_Printout::setUp()
{
	utests_base::setUp();
	utests_Init::InitDefaultDbValues();
	CString OutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_OUTPUT ), CString( __FILE__ ) );

	if( false == OutPath.IsEmpty() )
	{
		DeleteFilesAndFolder( OutPath, _T("*.*") );
	}
}

void utests_Printout::tearDown()
{
	utests_base::tearDown();
	utests_Init::RestoreDefaultDbValues();
}

void utests_Printout::TestPrintOut()
{
	// List tsp file
	// Open tsp file
	// export selection to xlsx file
	// compare content with reference xlsx

	// Get Data folder
	CFileFind finder;
	CString DirPath =  utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_OUTPUT ), CString( __FILE__ ), true );
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

		try
		{

			if( finder.IsDots() )
			{
				continue;
			}

			CString strTsp = DirPath + finder.GetFileName();

			if( false == strTsp.IsEmpty() )
			{
				// File open
				// Reset flag modified.
				::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_OPENDOCUMENT, false, ( LPARAM )&strTsp );
				::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_OPENDOCUMENT, false, ( LPARAM )&strTsp );
				// Export to xlsx
				CString strXlsx = DirOutPath + finder.GetFileTitle() + _T(".xlsx");
				::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_EXPORTSELPTOXLSX, NULL, ( LPARAM )&strXlsx );
				::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_EXPORTSELPTOXLSX, NULL, ( LPARAM )&strXlsx );
			}

		}
		catch( CFileException * )
		{
		}
	}

	finder.Close();

	// For each xlsx file
	bWorking = finder.FindFile(DirPath + _T("*.xlsx"));

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		try
		{

			if (finder.IsDots())
			{
				continue;
			}

			// Compare with reference file.
			utests_Init::CompareOutAndReferenceXLSXFiles( CString( CPPUNIT_TEST_CATEGORYNAME_OUTPUT ), CString(__FILE__), finder.GetFileTitle() + _T(".xlsx") );
		}
		catch (CFileException *)
		{
		}
	}

	finder.Close();
}
