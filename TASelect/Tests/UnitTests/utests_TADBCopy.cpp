#include "stdafx.h"

#include "utests_base.h"
#include "utests_TADBCopy.h"
#include "TASelectDoc.h"
#include "DeletedProductHelper.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_TADBCopy, CPPUNIT_TEST_CATEGORYNAME_DATABASE );

void utests_TADBCopy::setUp()
{
	utests_base::setUp();
	utests_Init::InitDefaultDbValues();
}


void utests_TADBCopy::TestCopyOfTsp()
{
	// List tsp file
	// Open tsp file
	// Save a copy of tsp file
	// compare content with reference tsp

	// Get data folder.
	CFileFind finder;
	CString DataPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString(__FILE__), true );
	CString OutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString(__FILE__), false );

	DeleteFilesAndFolder( OutPath, _T("*.*") );

	if( DataPath.ReverseFind( _T('\\') ) != DataPath.GetLength() - 1 )
	{
		DataPath += CString( _T("\\") );
	}

	// For each tsp file
	BOOL bWorking = finder.FindFile( DataPath + _T("*.tsp") );

	while( TRUE == bWorking )
	{
		bWorking = finder.FindNextFile();

		try
		{

			if( TRUE == finder.IsDots() )
			{
				continue;
			}

			CString strTsp = DataPath + finder.GetFileName();

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
						bool bDisplay = false;
						pProductHelper->VerifyHydroMod( pTableHM->GetIDPtr(), bDisplay );
						delete pProductHelper;
					}
				}

				// 1 step rewrite them just to be sure that we are in line with current DB version.
				pTASelectDoc->FileSave( OutPath + finder.GetFileName(), false );
				
				// Call Autosave that will copy tsp and save it.
				CString strFN = OutPath + _GetCpyFileName( finder.GetFileName() );
				pTASelectDoc->FileSave( strFN, true );
			}

		}
		catch( CFileException * )
		{
		}
	}

	finder.Close();

	// For each -cpy.tsp file.
	bWorking = finder.FindFile( OutPath + _T("*-cpy.tsp") );

	while( TRUE == bWorking )
	{
		bWorking = finder.FindNextFile();

		try
		{

			if( TRUE == finder.IsDots() )
			{
				continue;
			}

			CString SrcFile = finder.GetFileName();
			SrcFile.Replace( L"-cpy.tsp", L".tsp" );
			SrcFile = OutPath + SrcFile;
			CString TrgFile = OutPath + finder.GetFileName();

			// Compare with reference file.
			utests_Init::CompareOutAndReferenceFiles2( SrcFile, TrgFile, finder.GetFileName() );
		}
		catch ( CFileException * )
		{
		}
	}

	finder.Close();
}


CString utests_TADBCopy::_GetCpyFileName(CString strFN)
{
	strFN.Delete(strFN.Find(L".tsp"), 4);
	strFN = strFN + L"-cpy.tsp";
	return strFN;
}