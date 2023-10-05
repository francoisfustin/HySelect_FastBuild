#include "stdafx.h"

#include <objbase.h>  
#include <msxml6.h>
#include <regex>
#include "UTests_Common.h"
#include "taselect.h"
#include "datastruct.h"

// Because I use regular expression and compiler doesn't recognize escape sequences.
#pragma warning( disable: 4129 )

// *********************************************************************
// To Attach a Category to the root use :
// CPPUNIT_REGISTRY_ADD_TO_DEFAULT("Category");
//
// To declare Sub Category use :
// CPPUNIT_REGISTRY_ADD( "SubCategory", "Category" );
//
// To register a test class in a Category use :
// CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(utests_myclass, "SubCategory");
//
// Root
// L Category
//   L SubCategory
//     L utest_myclass
//       L utest_myclass::my_test
//       L utest_myclass::other_test
//
// *********************************************************************

CPPUNIT_REGISTRY_ADD_TO_DEFAULT( CPPUNIT_TEST_CATEGORYNAME_ACTUATORS );
CPPUNIT_REGISTRY_ADD_TO_DEFAULT( CPPUNIT_TEST_CATEGORYNAME_DATABASE );
CPPUNIT_REGISTRY_ADD_TO_DEFAULT( CPPUNIT_TEST_CATEGORYNAME_HYDRONICCALCULATION );
CPPUNIT_REGISTRY_ADD_TO_DEFAULT( CPPUNIT_TEST_CATEGORYNAME_IMPORTHYDRAULICCIRCUITS );
CPPUNIT_REGISTRY_ADD_TO_DEFAULT( CPPUNIT_TEST_CATEGORYNAME_OUTPUT );
CPPUNIT_REGISTRY_ADD_TO_DEFAULT( CPPUNIT_TEST_CATEGORYNAME_PRESSURISATION );
CPPUNIT_REGISTRY_ADD_TO_DEFAULT( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION );
CPPUNIT_REGISTRY_ADD_TO_DEFAULT( CPPUNIT_TEST_CATEGORYNAME_TECHNICALPARAMETERS );

ISAXXMLReader *utests_Init::m_pSAXReader = NULL;

bool utests_Init::InitFluidWater( CWaterChar *pWC, double dTemp /*= 75 */ )
{
	if( NULL == pWC )
	{
		return false;
	}

	pWC->SetAdditFamID( _T("WATER_ADDIT") );
	pWC->SetAdditID( _T("WATER") );
	pWC->SetTemp( dTemp );
	CPPUNIT_ASSERT_EQUAL( true, pWC->UpdateFluidData( dTemp, 0 ) );
	CPPUNIT_ASSERT_EQUAL( CWaterChar::efrcFluidOk, pWC->CheckFluidData( dTemp ) );

	return true;
}

bool utests_Init::InitFluidGlycol( CWaterChar *pWC, double dTemp /*= -10 */ )
{
	if( NULL == pWC )
	{
		return false;
	}

	pWC->SetAdditFamID( _T("GLYCOL_ADDIT") );
	pWC->SetAdditID( _T("ETHYL_GLYC") );
	pWC->SetPcWeight( 40 );
	pWC->SetTemp( dTemp );
	CPPUNIT_ASSERT_EQUAL( true, pWC->UpdateFluidData( dTemp, pWC->GetPcWeight() ) );
	CPPUNIT_ASSERT_EQUAL( CWaterChar::efrcFluidOk, pWC->CheckFluidData( dTemp ) );

	return true;
}

void utests_Init::InitDefaultDbValues()
{
	CString strDataLocalizedPath( CString( __FILE__ ) );
	CPath path( strDataLocalizedPath );
	strDataLocalizedPath = path.SplitPath( CPath::epfDir ) + CString( _T( "Data\\") + TASApp.GetTADBKey().MakeUpper() );

	CFileFind dirFinder;
	
	if( FALSE == dirFinder.FindFile( strDataLocalizedPath ) )
	{
		CString strMessage;
		strMessage.Format( _T("There is no test for the '%s' country."), TASApp.GetTADBKey().MakeUpper() );
		USES_CONVERSION;
		std::string strTemp = CT2A( strMessage );
		CPPUNIT_ASSERT_MESSAGE( strTemp, 0 );
	}

	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
	pTechP->Init();
	TASApp.GetpUnitDB()->ResetToSI();

	// Send message that unit have been changed.
	AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_UNITCHANGE );
}

void utests_Init::RestoreDefaultDbValues()
{
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
	pTechP->Init();
	//TASApp.GetpTADS()->ReInitTechAndSSelParam();

	// Send message that parameters have been changed.
	::AfxGetApp()->m_pMainWnd->SendMessage(WM_USER_TECHPARCHANGE);
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants(WM_USER_TECHPARCHANGE);
}

CFileTxt *utests_Init::CreateOutTxtFile( CString strCategoryName, CString strCurrentTestClass, CString strCurrentTestFile )
{
	CString strNewPath = _CreateFullDirPath( strCategoryName, strCurrentTestClass, strCurrentTestFile );

	if( true == strNewPath.IsEmpty() )
	{
		return NULL;
	}

	return CreateOutTxtFile( strNewPath );
}

CFileTxt *utests_Init::CreateOutTxtFile( CString strNewPath )
{
	// Code to extract data
	CFileTxt *pOutf = new CFileTxt();
	pOutf->Open( strNewPath, CFile::modeCreate | CFile::modeWrite );
	//Unicode
	BYTE tmpbuf[] = { 0xff, 0xfe };
	pOutf->Write( tmpbuf, sizeof( tmpbuf ) );

	return pOutf;
}

CString utests_Init::GetTxtFilePath( CString strCategoryName, CString strCurrentTestClass, bool bReference )
{
	CPath path( strCurrentTestClass );
	CString strNewPath( path.GetPath() );
	CString strLocal = TASApp.GetTADBKey().MakeUpper() + _T("\\");

	CString strFolder = ( true == bReference ) ? CString( _T( "Data\\" + strLocal + strCategoryName ) ) : CString( _T( "Out\\" + strLocal + strCategoryName ) );
	strFolder += _T("\\");

	strNewPath += strFolder + path.SplitPath( CPath::epfFName ) + CString( _T("\\") );
	bool bRet = WinSys::CreateFullDirectoryPath( strNewPath );

	if( false == bRet )
	{
		return _T( "" );
	}

	return strNewPath;
}

void utests_Init::CloseTxtFile( CFileTxt *pOutf )
{
	if( NULL == pOutf )
	{
		return;
	}

	pOutf->Close();
	delete pOutf;
}

std::fstream *utests_Init::OpenBinFile( CString strCategoryName, CString strCurrentTestClass, CString strCurrentTestFile, bool bWrite )
{
	CString NewPath = _CreateFullDirPath( strCategoryName, strCurrentTestClass, strCurrentTestFile );

	if( true == NewPath.IsEmpty() )
	{
		return NULL;
	}

	std::fstream *pfs = new std::fstream();

	if( true == bWrite )
	{
		pfs->open( NewPath, std::fstream::out | std::fstream::binary | std::fstream::trunc );
	}
	else
	{
		pfs->open( NewPath, std::fstream::in | std::fstream::binary );
	}

	if( ( pfs->rdstate() & std::ofstream::failbit ) )
	{
		return NULL;
	}

	return pfs;
}


void utests_Init::CloseBinFile( std::fstream *pfs )
{
	pfs->close();
	delete( pfs );
}

void utests_Init::CompareOutAndReferenceFiles( CString strCategoryName, CString strCurrentTestClass, CString strCurrentTestFile )
{
	//Compare with reference file
	CString RefFile = utests_Init::GetTxtFilePath( strCategoryName, strCurrentTestClass, true ) + strCurrentTestFile;
	CString NewFile = utests_Init::GetTxtFilePath( strCategoryName, strCurrentTestClass, false ) + strCurrentTestFile;

	if( false == IsFilesEqual( NewFile, RefFile ) )
	{
		CStringA astr = ( CStringA )NewFile;
		CPPUNIT_FAIL( ( string )astr );
	}
}

void utests_Init::CompareOutAndReferenceFiles2( CString strRefFile, CString strOutFile, CString strCurrentTestFile )
{
	if( false == IsFilesEqual( strOutFile, strRefFile ) )
	{
		CStringA astr = (CStringA)strCurrentTestFile;
		CPPUNIT_FAIL((string)astr);
	}
}

void utests_Init::CompareOutAndReferenceXLSXFiles( CString strCategoryName, CString strCurrentTestClass, CString strCurrentTestFile )
{
	CoInitialize( NULL );
	m_pSAXReader = NULL;

	HRESULT hr = CoCreateInstance( __uuidof( SAXXMLReader60 ), NULL,  CLSCTX_ALL, __uuidof(ISAXXMLReader), (void **)&m_pSAXReader);
	CPPUNIT_ASSERT( !FAILED( hr ) );

	CSAXErrorHandlerImpl clSAXErrorHandlerImpl;

	hr = m_pSAXReader->putErrorHandler( &clSAXErrorHandlerImpl );
	CPPUNIT_ASSERT( !FAILED( hr ) );

	CZipArchive zipReference;
	CZipArchive zipToCompare;

	CString strReferenceFileName = utests_Init::GetTxtFilePath( strCategoryName, strCurrentTestClass, true ) + strCurrentTestFile;
	CString strToCompareFileName = utests_Init::GetTxtFilePath( strCategoryName, strCurrentTestClass, false ) + strCurrentTestFile;

	// Reference file.
	zipReference.Open( strReferenceFileName, CZipArchive::zipOpenReadOnly );

	CZipCentralDir::Info zipCentralDirInfo;
	zipReference.GetCentralDirInfo( zipCentralDirInfo );
	std::vector<ZIP_INDEX_TYPE> vecRefIndexSheets;
	std::vector<CString> vecRefNameSheets;
	ZIP_INDEX_TYPE zipIndexRefShareStrings = ZIP_FILE_INDEX_NOT_FOUND;

	for( WORD wIndexLoop = 0; wIndexLoop < zipCentralDirInfo.m_uEntriesNumber; wIndexLoop++ )
	{
		CZipFileHeader zipFileHeader;
		zipReference.GetFileInfo( zipFileHeader, wIndexLoop );
		CZipString zipString = zipFileHeader.GetFileName();
		
		wregex rx( _T("xl\\\\worksheets\\\\.+\\.xml") );
		
		if( true == regex_match( zipString, rx, regex_constants::match_default ) )
		{
			vecRefIndexSheets.push_back( wIndexLoop );
			vecRefNameSheets.push_back( CString( zipString ) );
		}
		else if( 0 == zipString.compare( _T("xl\\sharedStrings.xml") ) )
		{
			zipIndexRefShareStrings = wIndexLoop;
		}
	}

	// Read references sheet if exist.
	std::map<int, CRelationshipsXMLReader> mapRefSheetRelationships;

	for( WORD wIndexLoop = 0; wIndexLoop < zipCentralDirInfo.m_uEntriesNumber; wIndexLoop++ )
	{
		CZipFileHeader zipFileHeader;
		zipReference.GetFileInfo( zipFileHeader, wIndexLoop );
		CZipString zipString = zipFileHeader.GetFileName();
		
		// In the "xl\worksheets\_rels\" folder.
		wregex rx( _T("xl\\\\worksheets\\\\\.\\\\.+\\.xml") );
		
		if( true == regex_match( zipString, rx, regex_constants::match_default ) )
		{
			// Search if this reference correspond to an existing sheet.
			for( int iLoopNameSheet = 0; iLoopNameSheet < (int)vecRefNameSheets.size(); iLoopNameSheet++ )
			{
				if( 0 == vecRefNameSheets[iLoopNameSheet].CompareNoCase( CString( zipString ) ) )
				{
					CRelationshipsXMLReader clRelationshipsXMLReader;
					_ReadXMLFile( zipReference, wIndexLoop, clRelationshipsXMLReader );
					mapRefSheetRelationships[wIndexLoop] = clRelationshipsXMLReader;
				}
			}
			
			vecRefIndexSheets.push_back( wIndexLoop );
		}
	}

	// To compare file.
	zipToCompare.Open( strToCompareFileName, CZipArchive::zipOpenReadOnly );

	zipToCompare.GetCentralDirInfo( zipCentralDirInfo );
	std::vector<ZIP_INDEX_TYPE> vecToCompIndexSheets;
	std::vector<CString> vecToCompNameSheets;
	ZIP_INDEX_TYPE zipIndexToCompShareStrings = ZIP_FILE_INDEX_NOT_FOUND;

	for( WORD wIndexLoop = 0; wIndexLoop < zipCentralDirInfo.m_uEntriesNumber; wIndexLoop++ )
	{
		CZipFileHeader zipFileHeader;
		zipToCompare.GetFileInfo( zipFileHeader, wIndexLoop );
		CZipString zipString = zipFileHeader.GetFileName();
		
		wregex rx( _T("xl\\\\worksheets\\\\.+\\.xml") );
		
		if( true == regex_match( zipString, rx, regex_constants::match_default ) )
		{
			vecToCompIndexSheets.push_back( wIndexLoop );
			vecToCompNameSheets.push_back( CString( zipString ) );
		}
		else if( 0 == zipString.compare( _T("xl\\sharedStrings.xml") ) )
		{
			zipIndexToCompShareStrings = wIndexLoop;
		}
	}

	// Read references sheet if exist.
	std::map<int, CRelationshipsXMLReader> mapToCompSheetRelationships;

	for( WORD wIndexLoop = 0; wIndexLoop < zipCentralDirInfo.m_uEntriesNumber; wIndexLoop++ )
	{
		CZipFileHeader zipFileHeader;
		zipToCompare.GetFileInfo( zipFileHeader, wIndexLoop );
		CZipString zipString = zipFileHeader.GetFileName();
		
		// In the "xl\worksheets\_rels\" folder.
		wregex rx( _T("xl\\\\worksheets\\\\\.\\\\.+\\.xml") );
		
		if( true == regex_match( zipString, rx, regex_constants::match_default ) )
		{
			// Search if this reference correspond to an existing sheet.
			for( int iLoopNameSheet = 0; iLoopNameSheet < (int)vecToCompNameSheets.size(); iLoopNameSheet++ )
			{
				if( 0 == vecToCompNameSheets[iLoopNameSheet].CompareNoCase( CString( zipString ) ) )
				{
					CRelationshipsXMLReader clRelationshipsXMLReader;
					_ReadXMLFile( zipToCompare, wIndexLoop, clRelationshipsXMLReader );
					mapToCompSheetRelationships[wIndexLoop] = clRelationshipsXMLReader;
				}
			}
			
			vecRefIndexSheets.push_back( wIndexLoop );
		}
	}

	CPPUNIT_ASSERT( (int)vecRefIndexSheets.size() == (int)vecToCompIndexSheets.size() );

	// Compare reference to the sheets if they exist.
	_CompareRelationships( mapRefSheetRelationships, mapToCompSheetRelationships );
	
	// Compare share strings if they exist.
	CPPUNIT_ASSERT( zipIndexRefShareStrings == zipIndexToCompShareStrings );
	CShareStringsXMLReader clRefShareStringsXMLReader;
	CShareStringsXMLReader clToCompShareStringsXMLReader;

	if( ZIP_FILE_INDEX_NOT_FOUND != zipIndexRefShareStrings )
	{
		// Read share strings from the reference file.
		_ReadXMLFile( zipReference, zipIndexRefShareStrings, clRefShareStringsXMLReader );

		// Read share strings from the file to compare.
		_ReadXMLFile( zipToCompare, zipIndexToCompShareStrings, clToCompShareStringsXMLReader );

		// Compare.
		int iRefStringsNbrs = clRefShareStringsXMLReader.GetStringNbr();
		int iToCompStringsNbrs = clToCompShareStringsXMLReader.GetStringNbr();
		CPPUNIT_ASSERT( iRefStringsNbrs == iToCompStringsNbrs );

		for( int iLoopStr = 0; iLoopStr < iRefStringsNbrs; iLoopStr++ )
		{
			CString strRefStr = clRefShareStringsXMLReader.GetString( iLoopStr );
			CString strToCompStr = clToCompShareStringsXMLReader.GetString( iLoopStr );
			CPPUNIT_ASSERT( 0 == strRefStr.Compare( strToCompStr ) );
		}
	}

	// Compare sheets in the workbook.
	for( int iLoopSheet = 0; iLoopSheet < (int)vecRefIndexSheets.size(); iLoopSheet++ )
	{
		CString strRefSheetTempFileName;
		_ExtractXMLToTempFile( zipReference, vecRefIndexSheets[iLoopSheet], strRefSheetTempFileName );

		CString strToCompSheetTempFileName;
		_ExtractXMLToTempFile( zipToCompare, vecRefIndexSheets[iLoopSheet], strToCompSheetTempFileName );

		_Compare2XMLFile( strRefSheetTempFileName, strToCompSheetTempFileName, vecRefNameSheets[iLoopSheet], strCurrentTestFile );
	}

	m_pSAXReader->Release();
	CoUninitialize();
}

void utests_Init::_ReadXMLFile( CZipArchive &zipArchive, WORD wIndex, CSAXContentHandlerImpl &clSAXContentHandlerImpl )
{
	CString strTempFileName;
	_ExtractXMLToTempFile( zipArchive, wIndex, strTempFileName );

	HRESULT hr = m_pSAXReader->putContentHandler( &clSAXContentHandlerImpl );
	CPPUNIT_ASSERT( !FAILED( hr ) );

	CString strURLPath = CString( _T("file:///") ) + CString( strTempFileName );
	hr = m_pSAXReader->parseURL( (LPCTSTR)strURLPath );
	CPPUNIT_ASSERT( !FAILED( hr ) );

	DeleteFile( strTempFileName );
}

void utests_Init::_ExtractXMLToTempFile( CZipArchive &zipArchive, WORD wIndex, CString &strTempFileName )
{
	// Read share strings from the reference file.
	TCHAR lpPathBuffer[512];
	DWORD dwRetVal = GetTempPath( 512, lpPathBuffer );
	CPPUNIT_ASSERT( dwRetVal <= 512 && ( 0 != dwRetVal ) );

	TCHAR szTempName[512];  
	UINT uRetVal = GetTempFileName( lpPathBuffer, TEXT("REF"), 0, szTempName );
	CPPUNIT_ASSERT( 0 != uRetVal );

	bool bExtractResult = zipArchive.ExtractFile( wIndex, lpPathBuffer, false, szTempName );
	CPPUNIT_ASSERT( true == bExtractResult );

	strTempFileName = CString( szTempName );
}

void utests_Init::_CompareRelationships( std::map<int, CRelationshipsXMLReader> &mapReference, std::map<int, CRelationshipsXMLReader> &mapToCompare )
{
	CPPUNIT_ASSERT( (int)mapReference.size() == (int)mapToCompare.size() );
	
	std::map<int, CRelationshipsXMLReader>::iterator iterReference = mapReference.begin();
	std::map<int, CRelationshipsXMLReader>::iterator iterToCompare = mapToCompare.begin();

	for( ; iterReference != mapReference.end(); iterReference++, iterToCompare++ )
	{
		CPPUNIT_ASSERT( iterReference->first == iterToCompare->first );

		CRelationshipsXMLReader::RelationshipsAttributes rRelshipsAttributesRef = iterReference->second.GetRelationships( iterReference->first );
		CRelationshipsXMLReader::RelationshipsAttributes rRelshipsAttributesToComp = iterToCompare->second.GetRelationships( iterToCompare->first );

		CPPUNIT_ASSERT( 0 == rRelshipsAttributesRef.m_strID.CompareNoCase( rRelshipsAttributesToComp.m_strID ) );
		CPPUNIT_ASSERT( 0 == rRelshipsAttributesRef.m_strType.CompareNoCase( rRelshipsAttributesToComp.m_strType ) );
		CPPUNIT_ASSERT( 0 == rRelshipsAttributesRef.m_strTarget.CompareNoCase( rRelshipsAttributesToComp.m_strTarget ) );
	}
}

void utests_Init::_Compare2XMLFile( CString strRefTempFileName, CString strToCompTempFileName, CString strFileNameInXML, CString strXMLFileName )
{
	CThreadXMLSheetComparator cThreadRefXMLSheetComparator;
	CThreadXMLSheetComparator cThreadToCompXMLSheetComparator;

	CEvent clRefReadyToBeComparedEvent( FALSE, TRUE );
	CEvent clRefStopEvent( FALSE, TRUE );
	CEvent clToCompReadyToBeComparedEvent( FALSE, TRUE );
	CEvent clToCompStopEvent( FALSE, TRUE );

	clRefReadyToBeComparedEvent.ResetEvent();
	clRefStopEvent.ResetEvent();
	clToCompReadyToBeComparedEvent.ResetEvent();
	clToCompStopEvent.ResetEvent();

	cThreadRefXMLSheetComparator.Start( strRefTempFileName, &clRefReadyToBeComparedEvent, &clRefStopEvent );
	cThreadToCompXMLSheetComparator.Start( strToCompTempFileName, &clToCompReadyToBeComparedEvent, &clToCompStopEvent );

	bool bFinished = false;

	HANDLE arhEvent[4];
	arhEvent[0] = clRefReadyToBeComparedEvent.m_hObject;
	arhEvent[1] = clRefStopEvent.m_hObject;
	arhEvent[2] = clToCompReadyToBeComparedEvent.m_hObject;
	arhEvent[3] = clToCompStopEvent.m_hObject;

	bool bRefReadyToBeCompared = false;
	bool bRefFinished = false;
	bool bToCompReadyToBeCompared = false;
	bool bToCompFinished = false;
	bool bStopAll = false;

	while( false == bFinished )
	{
		DWORD dwReturn = WaitForMultipleObjects( 4, arhEvent, FALSE, 100 );

		switch( dwReturn )
		{
			// Reference file gets an element ready to be compared.
			case WAIT_OBJECT_0:
				bRefReadyToBeCompared = true;
				clRefReadyToBeComparedEvent.ResetEvent();
				break;

			// Reference file has finished or stopped.
			case WAIT_OBJECT_0 + 1:
				bRefFinished = true;

				// Must reset otherwise 'WaitForMultipleObjects' will never check the next handles.
				clRefStopEvent.ResetEvent();
				break;

			// File to compare gets an element ready to be compared.
			case WAIT_OBJECT_0 + 2:
				bToCompReadyToBeCompared = true;
				clToCompReadyToBeComparedEvent.ResetEvent();
				break;

			// File to compare has finished or stopped.
			case WAIT_OBJECT_0 + 3:
				bToCompFinished = true;
				clToCompStopEvent.ResetEvent();
				break;
		}

		if( true == bRefFinished && true == bToCompFinished )
		{
			bFinished = true;

			CPPUNIT_ASSERT( CThreadXMLSheetComparator::ES_FinishedError != cThreadRefXMLSheetComparator.GetCurrentState() );
			CPPUNIT_ASSERT( CThreadXMLSheetComparator::ES_FinishedError != cThreadToCompXMLSheetComparator.GetCurrentState() );
		}
		else if( true == bRefFinished && CThreadXMLSheetComparator::ES_FinishedError == cThreadRefXMLSheetComparator.GetCurrentState() )
		{
			// If XML reference encounters an error we must finish the other thread.
			cThreadToCompXMLSheetComparator.Stop();
		}
		else if( true == bToCompFinished && CThreadXMLSheetComparator::ES_FinishedError == cThreadToCompXMLSheetComparator.GetCurrentState() )
		{
			// If the XML to compare encounters an error we must finish the other thread.
			cThreadRefXMLSheetComparator.Stop();
		}
		else if( true == bRefReadyToBeCompared && true == bToCompReadyToBeCompared )
		{
			bool bResult = cThreadRefXMLSheetComparator.CompareElements( &cThreadToCompXMLSheetComparator );
			
			if( true == bResult )
			{
				bRefReadyToBeCompared = false;
				bToCompReadyToBeCompared = false;

				// Restart the SAX parser.
				cThreadRefXMLSheetComparator.Continue();
				cThreadToCompXMLSheetComparator.Continue();
			}
			else
			{
				CXMLElementComparator *pRefXMLEleComp = cThreadRefXMLSheetComparator.GetXMLElementComparator();
				CXMLElementComparator *pToCompXMLEleComp = cThreadToCompXMLSheetComparator.GetXMLElementComparator();

				bFinished = true;
				CString strErrorMsg;

				strErrorMsg.Format( _T("XML file: %s; file in xml: %s; Ref: type: %s; element: %s "), strXMLFileName, strFileNameInXML, 
						pRefXMLEleComp->GetElementTypeStr(), pRefXMLEleComp->GetElement() );

				strErrorMsg.Format( _T("%sComp: type: %s; element: %s "), strErrorMsg, pToCompXMLEleComp->GetElementTypeStr(), 
						pToCompXMLEleComp->GetElement() );

				USES_CONVERSION;
				std::string strTemp = CT2A( strErrorMsg );

				// Reset the event because when thread stops, it set this event and thanks to that
				// we can check.
				clRefStopEvent.ResetEvent();
				clToCompStopEvent.ResetEvent();

				cThreadRefXMLSheetComparator.Stop();
				cThreadToCompXMLSheetComparator.Stop();
				
				// Wait that the two thread has well stopped.
				arhEvent[0] = clRefStopEvent.m_hObject;
				arhEvent[1] = clToCompStopEvent.m_hObject;

				bool bFirstThreadStopped = false;
				bool bSecondThreadStopped = false;
				bool bTreadsFinished = false;

				while( false == bTreadsFinished )
				{
					dwReturn = WaitForMultipleObjects( 2, arhEvent, FALSE, 100 );

					switch( dwReturn )
					{
						// First thread has stopped.
						case WAIT_OBJECT_0:
							bFirstThreadStopped = true;
							clRefStopEvent.ResetEvent();
							break;

						// Second thread has stopped.
						case WAIT_OBJECT_0 + 1:
							bSecondThreadStopped = true;
							clToCompStopEvent.ResetEvent();
							break;
					}

					if( true == bFirstThreadStopped && true == bSecondThreadStopped )
					{
						bTreadsFinished = true;
					}
				}
				
				CPPUNIT_FAIL( strTemp );
			}
		}
	}
}

CString utests_Init::_CreateFullDirPath( CString strCategoryName, CString strCurrentClassName, CString strCurrentTestFile )
{
	CString NewPath = GetTxtFilePath( strCategoryName, strCurrentClassName, false ) + strCurrentTestFile;

	if( true == NewPath.IsEmpty() )
	{
		return NewPath;
	}

	bool bRet = WinSys::CreateFullDirectoryPath( NewPath );

	if( false == bRet )
	{
		return L"";
	}

	return NewPath;
}

#define BUFFER_SIZE 0x100000 // 1 Mb

bool IsFilesEqual( CString lFilePath, CString rFilePath )
{
	std::ifstream lFile( lFilePath, std::ifstream::in | std::ifstream::binary );
	std::ifstream rFile( rFilePath, std::ifstream::in | std::ifstream::binary );

	if( !lFile.is_open() || !rFile.is_open() )
	{
		return false;
	}

	char *lBuffer = new char[BUFFER_SIZE]();
	char *rBuffer = new char[BUFFER_SIZE]();

	do
	{
		lFile.read( lBuffer, BUFFER_SIZE );
		rFile.read( rBuffer, BUFFER_SIZE );

		if( std::memcmp( lBuffer, rBuffer, BUFFER_SIZE ) != 0 )
		{
			delete[] lBuffer;
			delete[] rBuffer;
			return false;
		}
	}
	while( lFile.good() || rFile.good() );

	delete[] lBuffer;
	delete[] rBuffer;
	return true;
}

HRESULT STDMETHODCALLTYPE CSAXErrorHandlerImpl::error( ISAXLocator *pLocator, const wchar_t *pwchErrorMessage, HRESULT hrErrorCode )
{
	CPPUNIT_ASSERT( 0 );
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CSAXErrorHandlerImpl::fatalError( ISAXLocator *pLocator, const wchar_t *pwchErrorMessage, HRESULT hrErrorCode )
{
	CPPUNIT_ASSERT( 0 );
	return S_OK;
}

void CSAXContentHandlerImpl::RawName2CString( const wchar_t __RPC_FAR *pwchRawName, int cchRawName )
{
	cchRawName = ( cchRawName > 999 ) ? 999 : cchRawName;
	wcsncpy_s( m_tcValue, pwchRawName, cchRawName );
}

CShareStringsXMLReader::CShareStringsXMLReader()
{
	m_bSIStarted = false;
	m_bTStarted = false;
}

CString CShareStringsXMLReader::GetString( int iIndex )
{
	CString str = _T("");
	int iNbrStrings = (int)m_vecShareStrings.size();

	if( iIndex < 0 || iIndex > ( iNbrStrings - 1 ) )
	{
		return str;
	}

	return m_vecShareStrings[iIndex];
}

HRESULT STDMETHODCALLTYPE CShareStringsXMLReader::startElement( const wchar_t __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri, 
	const wchar_t __RPC_FAR *pwchLocalName, int cchLocalName, const wchar_t __RPC_FAR *pwchRawName, int cchRawName, ISAXAttributes __RPC_FAR *pAttributes )
{
	RawName2CString( pwchRawName, cchRawName );
	CString strValue( m_tcValue );

	if( 0 == strValue.CompareNoCase( _T("si") ) )
	{
		if( false == m_bSIStarted )
		{
			m_bSIStarted = true;
			m_bTStarted = false;
		}
		else
		{
			// Error in the XML FILE.
			CPPUNIT_ASSERT( 0 );
		}
	}
	else if( 0 == strValue.CompareNoCase( _T("t") ) )
	{
		if( true == m_bSIStarted && false == m_bTStarted )
		{
			m_bTStarted = true;
		}
		else if( ( true == m_bSIStarted && true == m_bSIStarted ) || false == m_bSIStarted )
		{
			// Error in the XML FILE.
			CPPUNIT_ASSERT( 0 );
		}
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CShareStringsXMLReader::endElement( const wchar_t __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri, 
	const wchar_t __RPC_FAR *pwchLocalName, int cchLocalName, const wchar_t __RPC_FAR *pwchRawName, int cchRawName )
{
	RawName2CString( pwchRawName, cchRawName );
	CString strValue( m_tcValue );

	if( 0 == strValue.CompareNoCase( _T("si") ) )
	{
		if( true == m_bSIStarted && false == m_bTStarted )
		{
			m_bSIStarted = false;
		}
		else if( ( true == m_bSIStarted && true == m_bTStarted ) || false == m_bSIStarted )
		{
			// Error in the XML FILE.
			CPPUNIT_ASSERT( 0 );
		}
	}
	else if( 0 == strValue.CompareNoCase( _T("t") ) )
	{
		if( true == m_bTStarted )
		{
			m_bTStarted = false;
		}
		else
		{
			// Error in the XML FILE.
			CPPUNIT_ASSERT( 0 );
		}
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CShareStringsXMLReader::characters( const wchar_t __RPC_FAR *pwchChars, int cchChars )
{
	if( true == m_bSIStarted && true == m_bTStarted )
	{
		RawName2CString( pwchChars, cchChars);
		CString strValue( m_tcValue );
		m_vecShareStrings.push_back( strValue );
	}

	return S_OK;
}

CRelationshipsXMLReader::CRelationshipsXMLReader()
{
}

CRelationshipsXMLReader::~CRelationshipsXMLReader()
{
	m_vecRelationships.clear();
}

CRelationshipsXMLReader::RelationshipsAttributes CRelationshipsXMLReader::GetRelationships( int iIndex )
{
	RelationshipsAttributes rRelationshipsAttributes;

	// TODO

	return rRelationshipsAttributes;
}

HRESULT STDMETHODCALLTYPE CRelationshipsXMLReader::startElement( const wchar_t __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri, 
	const wchar_t __RPC_FAR *pwchLocalName, int cchLocalName, const wchar_t __RPC_FAR *pwchRawName, int cchRawName, ISAXAttributes __RPC_FAR *pAttributes )
{
	// TODO
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CRelationshipsXMLReader::endElement( const wchar_t __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri, 
	const wchar_t __RPC_FAR *pwchLocalName, int cchLocalName, const wchar_t __RPC_FAR *pwchRawName, int cchRawName )
{
	// TODO
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CRelationshipsXMLReader::characters( const wchar_t __RPC_FAR *pwchChars, int cchChars )
{
	// TODO
	return S_OK;
}

CXMLElementComparator::CXMLElementComparator( wchar_t *tcElement, XMLElementType eXMLElementType )
{ 
	wcscpy( m_tcValue, tcElement );
	m_eXMLElementType = eXMLElementType; 
}

CString CXMLElementComparator::GetElementTypeStr()
{
	CString strElementTypeStr = _T("");

	if( XMLType_StartElement == m_eXMLElementType )
	{
		strElementTypeStr = "Start element";
	}
	else if( XMLType_EndElement == m_eXMLElementType )
	{
		strElementTypeStr = "End element";
	}
	else if( XMLType_Character == m_eXMLElementType )
	{
		strElementTypeStr = "Character element";
	}
	else
	{
		strElementTypeStr = "Unknown";
	}

	return strElementTypeStr;
}

void CXMLElementComparator::RawName2CString( const wchar_t __RPC_FAR *pwchRawName, int cchRawName )
{
	cchRawName = ( cchRawName > 999 ) ? 999 : cchRawName;
	wcsncpy_s( m_tcValue, pwchRawName, cchRawName );
}

CXMLStartElementComparator::CXMLStartElementComparator( wchar_t *tcElement, ISAXAttributes *pStartElementAttributes )
	:CXMLElementComparator( tcElement, CXMLElementComparator::XMLType_StartElement )
{
	m_pStartElementAttributes = pStartElementAttributes;
}

bool CXMLStartElementComparator::IsTheSame( CXMLElementComparator *pToCompare )
{
	if( NULL == pToCompare || CXMLElementComparator::XMLType_StartElement != pToCompare->GetElementType() )
	{
		return false;
	}

	CXMLStartElementComparator *pStartElementToCompare = (CXMLStartElementComparator *)pToCompare;

	int iResult = wcscmp( m_tcValue, pStartElementToCompare->GetStartElement() );

	if( 0 != iResult )
	{
		return false;
	}

	ISAXAttributes *pAttributesToCompare = pStartElementToCompare->GetISAXAttributes();

	if( NULL == m_pStartElementAttributes || NULL == pAttributesToCompare )
	{
		return false;
	}

	int iRefLength;
	m_pStartElementAttributes->getLength( &iRefLength );

	int iToCompLength;
	pAttributesToCompare->getLength( &iToCompLength );

	if( iRefLength != iToCompLength )
	{
		return false;
	}

	for( int iLoopAttribute = 0; iLoopAttribute < iRefLength; iLoopAttribute++ )
	{
		const wchar_t *pwchUri = NULL;
		int icchUri = 0;
		const wchar_t *pwchLocalName = NULL;
		int icchLocalName = 0;
		const wchar_t *pwchQName = NULL;
		int icchQName = 0;

		// Read first attribute from the reference.
		HRESULT hr = m_pStartElementAttributes->getName( iLoopAttribute, &pwchUri, &icchUri, &pwchLocalName, &icchLocalName, &pwchQName, &icchQName );

		if( FAILED( hr ) )
		{
			return false;
		}

		// Try to find same attribute in the one to compare.
		int iIndex = -1;
		hr = pAttributesToCompare->getIndexFromQName( pwchQName, icchQName, &iIndex );

		if( FAILED( hr ) || -1 == iIndex )
		{
			return false;
		}

		// Read value of the current attribute in the reference.
		const wchar_t *pwchRefValue = NULL;
		int cchRefValue = 0;
		hr = m_pStartElementAttributes->getValue( iLoopAttribute, &pwchRefValue, &cchRefValue );

		if( FAILED( hr ) )
		{
			return false;
		}

		// Read value of the current attribute in the one to compare.
		const wchar_t *pwchToCompValue = NULL;
		int cchToCompValue = 0;
		hr = m_pStartElementAttributes->getValue( iIndex, &pwchToCompValue, &cchToCompValue );

		if( FAILED( hr ) )
		{
			return false;
		}
		
		TCHAR tcRefValue[1000];
		RawName2CString( pwchRefValue, cchRefValue );
		wcscpy( tcRefValue, m_tcValue );

		TCHAR tcCompValue[1000];
		RawName2CString( pwchToCompValue, cchToCompValue );
		wcscpy( tcCompValue, m_tcValue );

		if( 0 != wcscmp( tcRefValue, tcCompValue ) )
		{
			return false;
		}
	}

	return true;
}

CXMLEndElementComparator::CXMLEndElementComparator( wchar_t *tcElement )
	:CXMLElementComparator( tcElement, CXMLElementComparator::XMLType_EndElement )
{
}

bool CXMLEndElementComparator::IsTheSame( CXMLElementComparator *pToCompare )
{
	if( NULL == pToCompare || CXMLElementComparator::XMLType_EndElement != pToCompare->GetElementType() )
	{
		return false;
	}

	CXMLEndElementComparator *pEndElementToCompare = (CXMLEndElementComparator *)pToCompare;

	int iResult = wcscmp( m_tcValue, pEndElementToCompare->GetEndElement() );

	if( 0 != iResult )
	{
		return false;
	}

	return true;
}

CXMLCharacterElementComparator::CXMLCharacterElementComparator( wchar_t *tcElement )
	:CXMLElementComparator( tcElement, CXMLElementComparator::XMLType_Character )
{
}

bool CXMLCharacterElementComparator::IsTheSame( CXMLElementComparator *pToCompare )
{
	if( NULL == pToCompare || CXMLElementComparator::XMLType_Character != pToCompare->GetElementType() )
	{
		return false;
	}

	CXMLCharacterElementComparator *pCharacterToCompare = (CXMLCharacterElementComparator *)pToCompare;

	int iResult = wcscmp( m_tcValue, pCharacterToCompare->GetCharacter() );

	if( 0 != iResult )
	{
		return false;
	}

	return true;
}

CThreadXMLSheetComparator::CThreadXMLSheetComparator()
{
	InitializeCriticalSection( &m_CriticalSection );
	m_pThread = NULL;
	m_pclThreadContinueEvent = NULL;
	m_pclThreadStopEvent = NULL;
	m_iEngineState = ES_Nothing;
	m_pSAXReader = NULL;
	m_pXMLElementComparator = NULL;
}

CThreadXMLSheetComparator::~CThreadXMLSheetComparator()
{
	if( NULL != m_pclThreadContinueEvent )
	{
		delete m_pclThreadContinueEvent;
	}

	if( NULL != m_pclThreadStopEvent )
	{
		delete m_pclThreadStopEvent;
	}

	DeleteCriticalSection( &m_CriticalSection );
}

void CThreadXMLSheetComparator::Start( CString strFileName, CEvent *pclCallerReadyToBeCompared, CEvent *pclCallerStop )
{
	if( NULL != m_pThread || true == strFileName.IsEmpty() || NULL == pclCallerReadyToBeCompared || NULL == pclCallerStop )
	{
		return;
	}

	m_szTempXMLFilename = strFileName;
	m_pclCallerReadyToBeCompared = pclCallerReadyToBeCompared;
	m_pclCallerStop = pclCallerStop;

	// Create an event to continue the parsing when needed.
	m_pclThreadContinueEvent = new CEvent( FALSE, TRUE );

	if( NULL == m_pclThreadContinueEvent )
	{
		return;
	}

	m_pclThreadContinueEvent->ResetEvent();

	// Create an event to stop the parsing when needed.
	m_pclThreadStopEvent = new CEvent( FALSE, TRUE );

	if( NULL == m_pclThreadStopEvent )
	{
		return;
	}

	m_pclThreadStopEvent->ResetEvent();

	m_pThread = AfxBeginThread( &ThreadXMLSheetParser, ( LPVOID )this );
}

void CThreadXMLSheetComparator::Continue()
{
	m_pclThreadContinueEvent->SetEvent();
}

void CThreadXMLSheetComparator::Stop()
{
	if( NULL == m_pThread )
	{
		return;
	}

	m_pclThreadStopEvent->SetEvent();
}
	
bool CThreadXMLSheetComparator::CompareElements( CThreadXMLSheetComparator *pToCompThreadXMLSheetComparator )
{
	if( NULL == pToCompThreadXMLSheetComparator )
	{
		return false;
	}

	if( m_iEngineState != pToCompThreadXMLSheetComparator->GetCurrentState() )
	{
		return false;
	}

	CXMLElementComparator *pToCompXMLElementComparator = pToCompThreadXMLSheetComparator->GetXMLElementComparator();

	if( NULL == m_pXMLElementComparator || NULL == pToCompXMLElementComparator )
	{
		return false;
	}

	return m_pXMLElementComparator->IsTheSame( pToCompXMLElementComparator );
}

int CThreadXMLSheetComparator::GetCurrentState()
{
	EnterCriticalSection( &m_CriticalSection );
	int iEngineState = m_iEngineState;
	LeaveCriticalSection( &m_CriticalSection );

	return iEngineState;
}

CString CThreadXMLSheetComparator::GetCurrentStateStr()
{
	EnterCriticalSection( &m_CriticalSection );
	int iEngineState = m_iEngineState;
	LeaveCriticalSection( &m_CriticalSection );

	CString strState = _T("");

	if( ES_Parsing == ( ES_Parsing & iEngineState ) )
	{
		strState += _T("ES_Parsing");
	}
	
	if( ES_StartElement == ( ES_StartElement & iEngineState ) )
	{
		strState += ( true == strState.IsEmpty() ) ? _T("ES_StartElement") : _T("|ES_StartElement");
	}

	if( ES_Character == ( ES_Character & iEngineState ) )
	{
		strState += ( true == strState.IsEmpty() ) ? _T("ES_Character") : _T("|ES_Character");
	}

	if( ES_EndElement == ( ES_EndElement & iEngineState ) )
	{
		strState += ( true == strState.IsEmpty() ) ? _T("ES_EndElement") : _T("|ES_EndElement");
	}

	if( ES_FinishedOK == ( ES_FinishedOK & iEngineState ) )
	{
		strState += ( true == strState.IsEmpty() ) ? _T("ES_FinishedOK") : _T("|ES_FinishedOK");
	}

	if( ES_FinishedError == ( ES_FinishedError & iEngineState ) )
	{
		strState += ( true == strState.IsEmpty() ) ? _T("ES_FinishedError") : _T("|ES_FinishedError");
	}

	if( ES_WaitComparaison == ( ES_WaitComparaison & iEngineState ) )
	{
		strState += ( true == strState.IsEmpty() ) ? _T("ES_WaitComparaison") : _T("|ES_WaitComparaison");
	}

	if( ES_Stop == ( ES_Stop & iEngineState ) )
	{
		strState += ( true == strState.IsEmpty() ) ? _T("ES_Stop") : _T("|ES_Stop");
	}

	return strState;
}

UINT CThreadXMLSheetComparator::ThreadXMLSheetParser( LPVOID pParam )
{
	CThreadXMLSheetComparator *pclThreadXMLSheetComparator = (CThreadXMLSheetComparator *)pParam;

	if( NULL == pclThreadXMLSheetComparator )
	{
		return -1;
	}

	CoInitialize( NULL );
	pclThreadXMLSheetComparator->m_pSAXReader = NULL;

	HRESULT hr = CoCreateInstance( __uuidof( SAXXMLReader60 ), NULL,  CLSCTX_ALL, __uuidof(ISAXXMLReader), (void **)&pclThreadXMLSheetComparator->m_pSAXReader);
	
	if( FAILED( hr ) )
	{
		return -1;
	}

	hr = pclThreadXMLSheetComparator->m_pSAXReader->putErrorHandler( &pclThreadXMLSheetComparator->m_clSAXErrorHandlerImpl );
	
	if( FAILED( hr ) )
	{
		return -1;
	}

	hr = pclThreadXMLSheetComparator->m_pSAXReader->putContentHandler( pclThreadXMLSheetComparator );
	
	if( FAILED( hr ) )
	{
		return -1;
	}

	CString strURLPath = CString( _T("file:///") ) + CString( pclThreadXMLSheetComparator->m_szTempXMLFilename );
	
	EnterCriticalSection( &pclThreadXMLSheetComparator->m_CriticalSection );
	pclThreadXMLSheetComparator->m_iEngineState = ES_Parsing;
	LeaveCriticalSection( &pclThreadXMLSheetComparator->m_CriticalSection );

	hr = pclThreadXMLSheetComparator->m_pSAXReader->parseURL( (LPCTSTR)strURLPath );

	DeleteFile( pclThreadXMLSheetComparator->m_szTempXMLFilename );

	pclThreadXMLSheetComparator->m_pSAXReader->Release();
	CoUninitialize();

	EnterCriticalSection( &pclThreadXMLSheetComparator->m_CriticalSection );
	pclThreadXMLSheetComparator->m_iEngineState = FAILED( hr ) ? ES_FinishedError : ES_FinishedOK;
	LeaveCriticalSection( &pclThreadXMLSheetComparator->m_CriticalSection );

	pclThreadXMLSheetComparator->m_pclCallerStop->SetEvent();

	return 0;
}

HRESULT STDMETHODCALLTYPE CThreadXMLSheetComparator::startElement( const wchar_t __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri, 
	const wchar_t __RPC_FAR *pwchLocalName,	int cchLocalName, const wchar_t __RPC_FAR *pwchRawName, int cchRawName, ISAXAttributes __RPC_FAR *pAttributes )
{
	EnterCriticalSection( &m_CriticalSection );
	int iEngineState = m_iEngineState;
	LeaveCriticalSection( &m_CriticalSection );

	if( ES_Stop == iEngineState )
	{
		// Do not manage anymore when the thread and parser must be stopped.
		return E_FAIL;
	}

	if( NULL != m_pXMLElementComparator )
	{
		delete m_pXMLElementComparator;
		m_pXMLElementComparator = NULL;
	}

	RawName2CString( pwchRawName, cchRawName );
	m_pXMLElementComparator = new CXMLStartElementComparator( m_tcValue, pAttributes );

	if( NULL == m_pXMLElementComparator )
	{
		EnterCriticalSection( &m_CriticalSection );
		m_iEngineState = ES_Stop;
		LeaveCriticalSection( &m_CriticalSection );

		m_pclCallerStop->SetEvent();
		return E_FAIL;
	}

	EnterCriticalSection( &m_CriticalSection );
	m_iEngineState = ES_WaitComparaison | ES_StartElement;
	LeaveCriticalSection( &m_CriticalSection );

	// Signal to the caller that an element is ready to be compared with the other thread.
	m_pclCallerReadyToBeCompared->SetEvent();

	// Doesn't leave this method until comparison with other XML parser has been done.
	_WaitComparaison();

	EnterCriticalSection( &m_CriticalSection );
	iEngineState = m_iEngineState;

	if( NULL != m_pXMLElementComparator )
	{
		delete m_pXMLElementComparator;
		m_pXMLElementComparator = NULL;
	}

	LeaveCriticalSection( &m_CriticalSection );

	if( ES_Stop == iEngineState )
	{
		return E_FAIL;
	}

	EnterCriticalSection( &m_CriticalSection );
	m_iEngineState = ES_Parsing;
	LeaveCriticalSection( &m_CriticalSection );

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CThreadXMLSheetComparator::endElement( const wchar_t __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri,
	const wchar_t __RPC_FAR *pwchLocalName, int cchLocalName, const wchar_t __RPC_FAR *pwchRawName, int cchRawName )
{
	EnterCriticalSection( &m_CriticalSection );
	int iEngineState = m_iEngineState;
	LeaveCriticalSection( &m_CriticalSection );

	if( ES_Stop == iEngineState )
	{
		// Do not manage anymore when the thread and parser must be stopped.
		return E_FAIL;
	}

	if( NULL != m_pXMLElementComparator )
	{
		delete m_pXMLElementComparator;
		m_pXMLElementComparator = NULL;
	}

	RawName2CString( pwchRawName, cchRawName );
	m_pXMLElementComparator = new CXMLEndElementComparator( m_tcValue );

	if( NULL == m_pXMLElementComparator )
	{
		EnterCriticalSection( &m_CriticalSection );
		m_iEngineState = ES_Stop;
		LeaveCriticalSection( &m_CriticalSection );

		m_pclCallerStop->SetEvent();
		return E_FAIL;
	}

	EnterCriticalSection( &m_CriticalSection );
	m_iEngineState = ES_WaitComparaison | ES_EndElement;
	LeaveCriticalSection( &m_CriticalSection );

	// Signal to the caller that an element is ready to be compared with the other thread.
	m_pclCallerReadyToBeCompared->SetEvent();

	// Doesn't leave this method until comparison with other XML parser has been done.
	_WaitComparaison();

	EnterCriticalSection( &m_CriticalSection );
	iEngineState = m_iEngineState;

	if( NULL != m_pXMLElementComparator )
	{
		delete m_pXMLElementComparator;
		m_pXMLElementComparator = NULL;
	}

	LeaveCriticalSection( &m_CriticalSection );

	if( ES_Stop == iEngineState )
	{
		return E_FAIL;
	}
	
	EnterCriticalSection( &m_CriticalSection );
	m_iEngineState = ES_Parsing;
	LeaveCriticalSection( &m_CriticalSection );

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CThreadXMLSheetComparator::characters( const wchar_t __RPC_FAR *pwchChars, int cchChars )
{
	EnterCriticalSection( &m_CriticalSection );
	int iEngineState = m_iEngineState;
	LeaveCriticalSection( &m_CriticalSection );

	if( ES_Stop == iEngineState )
	{
		// Do not manage anymore when the thread and parser must be stopped.
		return E_FAIL;
	}

	if( NULL != m_pXMLElementComparator )
	{
		delete m_pXMLElementComparator;
		m_pXMLElementComparator = NULL;
	}

	RawName2CString( pwchChars, cchChars );
	m_pXMLElementComparator = new CXMLCharacterElementComparator( m_tcValue );

	if( NULL == m_pXMLElementComparator )
	{
		EnterCriticalSection( &m_CriticalSection );
		m_iEngineState = ES_Stop;
		LeaveCriticalSection( &m_CriticalSection );

		m_pclCallerStop->SetEvent();
		return E_FAIL;
	}

	EnterCriticalSection( &m_CriticalSection );
	m_iEngineState = ES_WaitComparaison | ES_Character;
	LeaveCriticalSection( &m_CriticalSection );

	// Signal to the caller that an element is ready to be compared with the other thread.
	m_pclCallerReadyToBeCompared->SetEvent();

	// Doesn't leave this method until comparison with other XML parser has been done.
	_WaitComparaison();

	EnterCriticalSection( &m_CriticalSection );
	iEngineState = m_iEngineState;

	if( NULL != m_pXMLElementComparator )
	{
		delete m_pXMLElementComparator;
		m_pXMLElementComparator = NULL;
	}

	LeaveCriticalSection( &m_CriticalSection );

	if( ES_Stop == iEngineState )
	{
		return E_FAIL;
	}

	EnterCriticalSection( &m_CriticalSection );
	m_iEngineState = ES_Parsing;
	LeaveCriticalSection( &m_CriticalSection );

	return S_OK;
}

void CThreadXMLSheetComparator::_WaitComparaison()
{
	bool bStop = false;
	
	HANDLE arhEvent[2];
	arhEvent[0] = m_pclThreadStopEvent->m_hObject;
	arhEvent[1] = m_pclThreadContinueEvent->m_hObject;

	// Debug.
	CString strOutput;
	
	// While we are in this method, parser is blocked thus no chance to have other call.
	// This is why we can set the 'm_bMustStop' variable without critical section.

	while( false == bStop )
	{
		DWORD dwReturn = WaitForMultipleObjects( 2, arhEvent, FALSE, 100 );

		switch( dwReturn )
		{
			// Receive a stop because there is a problem when comparing two XML.
			case WAIT_OBJECT_0:
				EnterCriticalSection( &m_CriticalSection );
				m_iEngineState = ES_Stop;
				LeaveCriticalSection( &m_CriticalSection );
				bStop = true;
				break;

			// Comparison is finished and OK. We can continue the SAX parser.
			case WAIT_OBJECT_0 + 1:
				EnterCriticalSection( &m_CriticalSection );
				m_pclThreadContinueEvent->ResetEvent();
				LeaveCriticalSection( &m_CriticalSection );
				bStop = true;
				break;

			// Timeout.
			case WAIT_TIMEOUT:
				break;
		}
	}
}
