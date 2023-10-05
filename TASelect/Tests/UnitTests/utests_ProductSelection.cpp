#include "stdafx.h"


#ifdef _DEBUG

#include "mainfrm.h"
#include "ProductSelectionParameters.h"
#include "TUProductSelectionHelper.h"
#include "utests_base.h"
#include "utests_ProductSelection.h"
#include "TASelectDoc.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_ProductSelection, CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION );

void utests_ProductSelection::setUp()
{
	utests_base::setUp();
	utests_Init::InitDefaultDbValues();
}

void utests_ProductSelection::tearDown()
{
	utests_base::tearDown();
	utests_Init::RestoreDefaultDbValues();
}

void utests_ProductSelection::IndividualSelectionBCV()
{
	_ExecuteTest( _T("TestIndSelBCV.txt") );
}

void utests_ProductSelection::IndividualSelectionBV()
{
	_ExecuteTest( _T("TestIndSelBV.txt") );
}

void utests_ProductSelection::IndividualSelectionCV()
{
	_ExecuteTest( _T("TestIndSelCV.txt") );
}

void utests_ProductSelection::IndividualSelectionDpC()
{
	_ExecuteTest( _T("TestIndSelDpC.txt") );
}

void utests_ProductSelection::IndividualSelectionDpCBCV()
{
	_ExecuteTest( _T("TestIndSelDpCBCV.txt") );
}

void utests_ProductSelection::IndividualSelectionPIBCV()
{
	_ExecuteTest( _T("TestIndSelPIBCV.txt") );
}

void utests_ProductSelection::IndividualSelectionPM()
{
	_ExecuteTest( _T("TestIndSelPM.txt") );
}

void utests_ProductSelection::IndividualSelectionSafetyValve()
{
	_ExecuteTest( _T("TestIndSelSafetyValve.txt") );
}

void utests_ProductSelection::IndividualSelectionSeparator()
{
	if( 0 == StringCompare( TASApp.GetTADBKey(), _T("US") ) ) 
	{
		return;
	}

	_ExecuteTest( _T("TestIndSelSeparator.txt") );
}

void utests_ProductSelection::IndividualSelectionShutoffValve()
{
	_ExecuteTest( _T("TestIndSelShutoffValve.txt") );
}

void utests_ProductSelection::IndividualSelectionTRV()
{
	if( 0 == StringCompare( TASApp.GetTADBKey(), _T("US") ) )
	{
		return;
	}
	_ExecuteTest( _T("TestIndSelTRV.txt") );
}

void utests_ProductSelection::IndividualSelection6WayValve()
{
	if( 0 == StringCompare( TASApp.GetTADBKey(), _T( "US" ) ) )
	{
		return;
	}
	_ExecuteTest( _T("TestIndSel6WayValves.txt") );
}

void utests_ProductSelection::IndividualSelectionSmartControlValve()
{
	_ExecuteTest( _T("TestIndSelSmartControlValve.txt") );
}

void utests_ProductSelection::BatchSelectionBCV()
{
	_ExecuteTest( _T("TestBatchSelBCV.txt") );
}

void utests_ProductSelection::BatchSelectionBV()
{
	_ExecuteTest( _T("TestBatchSelBV.txt") );
}

void utests_ProductSelection::BatchSelectionDpC()
{
	_ExecuteTest( _T("TestBatchSelDpC.txt") );
}

void utests_ProductSelection::BatchSelectionPIBCV()
{
	_ExecuteTest( _T("TestBatchSelPIBCV.txt") );
}

void utests_ProductSelection::BatchSelectionSeparator()
{
	if( 0 != StringCompare( TASApp.GetTADBKey(), _T("US") ) )
	{
		_ExecuteTest( _T("TestBatchSelSeparator.txt") );
	}
}

void utests_ProductSelection::BatchSelectionSmartControlValve()
{
	_ExecuteTest( _T("TestBatchSelSmartControlValve.txt") );
}

void utests_ProductSelection::TestProductSelection()
{
	// List txt file.
	// Open txt file.
	// export selection to txt file.
	// compare content with reference txt.

	// Get data folder.
	CFileFind finder;
	CString DirPath =  utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ), true );

	if( DirPath.ReverseFind( '\\' ) != DirPath.GetLength() - 1 )
	{
		DirPath += CString( _T("\\") );
	}

	CString OutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ) );
	if( false == OutPath.IsEmpty() )
	{
		DeleteFilesAndFolder( OutPath, _T( "*.*" ) );
	}
	// For each input file.
	BOOL bWorking = finder.FindFile( DirPath + _T("*.txt") );

	while( TRUE == bWorking )
	{
		bWorking = finder.FindNextFile();

		try
		{

			if( TRUE == finder.IsDots() )
			{
				continue;
			}

			CString strInputFile = DirPath + finder.GetFileName();

			if( false == strInputFile.IsEmpty() )
			{
				m_clTUProdSelLauncher.SetTADB( TASApp.GetpTADB() );
				m_clTUProdSelLauncher.SetTADS( TASApp.GetpTADS() );
				m_clTUProdSelLauncher.SetUserDB( TASApp.GetpUserDB() );
				m_clTUProdSelLauncher.SetPipeDB( TASApp.GetpPipeDB() );

				// Execute test with input file.
				CString strErrorMessage;
				
				if( 0 != m_clTUProdSelLauncher.ExecuteTest( strInputFile, strErrorMessage ) )
				{
					strInputFile += _T("\r\n");
					strErrorMessage.Insert( 0, strInputFile );

					USES_CONVERSION;
					std::string strTemp = CT2A( strErrorMessage );
					CPPUNIT_FAIL( strTemp );
				}
			}

		}
		catch( CFileException * )
		{
		}
	}

	finder.Close();
}

void utests_ProductSelection::_ExecuteTest( CString strInputFile )
{
	if( false == strInputFile.IsEmpty() )
	{
		CString DataPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ), true );
		CString OutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ), false );

		m_clTUProdSelLauncher.SetTADB( TASApp.GetpTADB() );
		m_clTUProdSelLauncher.SetTADS( TASApp.GetpTADS() );
		m_clTUProdSelLauncher.SetUserDB( TASApp.GetpUserDB() );
		m_clTUProdSelLauncher.SetPipeDB( TASApp.GetpPipeDB() );

		// HYS-1192: force locale info to be in English - United States to have the same between all computers.
		LCID CurrentLocale = GetThreadLocale();

		// Prefer the 'SetThreadLocale' function instead of the '_tsetlocale' function. Because the first function will set the DEFAULT
		// regional settings corresponding to the LCID and not the one that the user can change in the Windows settings.
		SetThreadLocale( 0x0409 );

		CString strInputFileFullPath = DataPath + _T("\\") + strInputFile;
		CString strOutputFileFullPath = OutPath + _T("\\") + strInputFile;

		CString strErrorMessage;
		m_clTUProdSelLauncher.LoadTestAndDropResults( strInputFileFullPath, strOutputFileFullPath, strErrorMessage );

		// Restore regional settings.
		SetThreadLocale( CurrentLocale );

		// Compare with reference file.
		utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ), strInputFile );
	}
	else
	{
		CString strError;
		strError.Format( _T("The '%s' file doesn't exist."), strInputFile );
		USES_CONVERSION;
		std::string strTemp = CT2A( strError );
		CPPUNIT_ASSERT_MESSAGE( strTemp, 0 );
	}
}

#endif
