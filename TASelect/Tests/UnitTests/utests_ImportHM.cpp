#include "stdafx.h"

#include "utests_base.h"
#include "utests_ImportHM.h"
#include "TASelectDoc.h"
#include "DeletedProductHelper.h"
#include  "locale"
#include "HMTreeListCtrl.h"
#include "DlgInjectionError.h"
#include "DlgImportHM.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_ImportHM, CPPUNIT_TEST_CATEGORYNAME_IMPORTHYDRAULICCIRCUITS );

void utests_ImportHM::setUp()
{
	utests_base::setUp();
	utests_Init::InitDefaultDbValues();
	CDimValue::AccessUDB()->SetDefaultUnit( _U_DIFFPRESS, _U_DIFFPRESS_PA );	// Pa
	CDimValue::AccessUDB()->SetDefaultUnit( _U_FLOW, _U_FLOW_L_H );	// l/h
}

void utests_ImportHM::tearDown()
{
	utests_base::tearDown();

	// Warning: when RestoreDefaultDbValues is called it change technical date and relaunch a calculus on the latest computed circuit
	// some error would be displayed but it will not impact unit tests
	utests_Init::RestoreDefaultDbValues();
	CDimValue::AccessUDB()->SetDefaultUnit( _U_DIFFPRESS, _U_DIFFPRESS_PA );	// Pa
	CDimValue::AccessUDB()->SetDefaultUnit( _U_FLOW, _U_FLOW_L_H );	// l/h
}

void utests_ImportHM::ImportNewHMCalc()
{
	// Import new tsp file
	// Write result in txt file
	// compare content with reference txt

	// Get Data folder
	CFileFind finder;
	CString DataPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_IMPORTHYDRAULICCIRCUITS ), CString( __FILE__ ), true );
	CString OutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_IMPORTHYDRAULICCIRCUITS ), CString( __FILE__ ), false );

	DeleteFilesAndFolder( OutPath, _T( "*.*" ) );

	// Read input informations
	CString strInputFile = DataPath + _T( "ImportNewHMCalc.txt" );
	CStringArray ArrayParam;
	int nElt = 0;
	if( false == strInputFile.IsEmpty() )
	{
		if( false == _ReadInputInfos( &ArrayParam, nElt, strInputFile ) )
		{
			CPPUNIT_FAIL( "Incorrect input file " + (string)(CStringA)strInputFile );
		}
	}

	BOOL bWorking = finder.FindFile( DataPath + ArrayParam[0] );

	if( bWorking )
	{
		try
		{
			finder.FindNextFile();
			CString strTsp = DataPath + finder.GetFileName();

			if( false == strTsp.IsEmpty() )
			{
				// All out informations to verify except hydromod informations
				CString strOutPutInfos = CteEMPTY_STRING;

				CString strParamProj = DataPath + _T( "EmptyForParamOnly.tsp" );
				pTASelectDoc->BeginWaitCursor();

				// Close previous project
				pTASelectDoc->DeleteContents();	
				TASApp.GetpTADS()->Init();
				::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_OPENDOCUMENT, false, (LPARAM)&strParamProj );
				LRESULT lresult = ::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_OPENDOCUMENT, false, (LPARAM)&strParamProj );
				::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_NEWDOCUMENT );
				::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_NEWDOCUMENT );
				pTASelectDoc->EndWaitCursor();

				pMainFrame->SetHMCalcMode( true );
				strOutPutInfos += pMainFrame->IsHMCalcChecked() ? _T( "true" ) : _T( "false" );
				strOutPutInfos += _T( " / " );
				// Button status before import
				strOutPutInfos += pMainFrame->IsMenuHMImportEnable() ? _T( "true" ) : _T( "false" );
				strOutPutInfos += _T( " / " );

				strOutPutInfos += pMainFrame->IsRibbonHMImportEnable() ? _T( "true" ) : _T( "false" );
				strOutPutInfos += _T( " / " );

				// Import all the project by moving the root (look input information)
				ImportForUnitTest( strTsp, &ArrayParam, nElt  );

				// Button state after import
				strOutPutInfos += pMainFrame->IsMenuHMImportEnable() ? _T( "true" ) : _T( "false" );
				strOutPutInfos += _T( " / " );

				strOutPutInfos += pMainFrame->IsRibbonHMImportEnable() ? _T( "true" ) : _T( "false" );
				strOutPutInfos += _T( " / " );

				// Write the out txt file.
				CString SrcFile = DataPath + finder.GetFileTitle() + _T( ".txt" );
				CString strTxt = OutPath + finder.GetFileTitle() + _T( ".txt" );

				// Copy only input informations in the output file
				_WriteInputInfos( SrcFile, strTxt );
				// Wrtie output information
				_WriteImportNewHMCalcInfo( strTxt, strOutPutInfos, false, ArrayParam[0] );

				// Compare with reference file.
				utests_Init::CompareOutAndReferenceFiles2( SrcFile, strTxt, finder.GetFileName() );

				// Find error in HM Results
				CString strToFind = _T( "ERROR" );

				FILE* fb = _wfopen( SrcFile, _T( "rb" ) );
				wchar_t tcLine[1024];
				CString line;
				string message = " An error is found in ";
				string error_message = "";

				if( fb )
				{
					while( NULL != fgetws( tcLine, 1024, fb ) )
					{
						line = tcLine;
						line.Trim( _T( "\r\n" ) );

						if( line.Find( strToFind, 0 ) != -1 )
						{
							error_message += (string)(CStringA)finder.GetFileName() + "\r\n";
						}
					}

					fclose( fb );
				}

				if( !error_message.empty() )
				{
					CPPUNIT_FAIL( message + error_message );
				}
			}

		}
		catch( CFileException* )
		{
		}
	}
}

void utests_ImportHM::ImportNewNonHMCalc()
{
	// Import new tsp file with non HMCalc mode
	// Write result in txt file
	// compare content with reference txt

	// Get Data folder
	CFileFind finder;
	CString DataPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_IMPORTHYDRAULICCIRCUITS ), CString( __FILE__ ), true );
	CString OutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_IMPORTHYDRAULICCIRCUITS ), CString( __FILE__ ), false );

	// Read input informations
	CString strInputFile = DataPath + _T( "ImportNewNonHMCalc.txt" );
	CStringArray ArrayParam;
	int nElt = 0;
	if( false == strInputFile.IsEmpty() )
	{
		if( false == _ReadInputInfos( &ArrayParam, nElt, strInputFile ) )
		{
			CPPUNIT_FAIL( "Incorrect input file " + (string)(CStringA)strInputFile );
		}
	}

	BOOL bWorking = finder.FindFile( DataPath + ArrayParam[0] );

	if( bWorking )
	{
		try
		{
			finder.FindNextFile();
			CString strTsp = DataPath + finder.GetFileName();

			if( false == strTsp.IsEmpty() )
			{
				// information to verify except HM informations
				CString strOutPutInfos = CteEMPTY_STRING;

				// Close previous project
				pTASelectDoc->BeginWaitCursor();
				pTASelectDoc->DeleteContents();
				TASApp.GetpTADS()->Init();
				::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_NEWDOCUMENT );
				::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_NEWDOCUMENT );
				pTASelectDoc->EndWaitCursor();

				strOutPutInfos += pMainFrame->IsHMCalcChecked() ? _T( "true" ) : _T( "false" );
				strOutPutInfos += _T( " / " );

				// Button status before import
				strOutPutInfos += pMainFrame->IsMenuHMImportEnable() ? _T( "true" ) : _T( "false" );
				strOutPutInfos += _T( " / " );

				strOutPutInfos += pMainFrame->IsRibbonHMImportEnable() ? _T( "true" ) : _T( "false" );
				strOutPutInfos += _T( " / " );

				// Import all the project by moving the root ( see input informations )
				ImportForUnitTest( strTsp, &ArrayParam, nElt );

				// Button status after import
				strOutPutInfos += pMainFrame->IsMenuHMImportEnable() ? _T( "true" ) : _T( "false" );
				strOutPutInfos += _T( " / " );

				strOutPutInfos += pMainFrame->IsRibbonHMImportEnable() ? _T( "true" ) : _T( "false" );
				strOutPutInfos += _T( " / " );

				// Write the out txt file.
				CString strTxt = OutPath + finder.GetFileTitle() + _T( ".txt" );
				CString SrcFile = DataPath + finder.GetFileTitle() + _T( ".txt" );

				// Copy only input information in the output file
				_WriteInputInfos( SrcFile, strTxt );
				// Write output informations 
				_WriteImportNewNonHMCalcInfo( strTxt, strOutPutInfos, ArrayParam[0], false );

				// Compare with reference file.
				utests_Init::CompareOutAndReferenceFiles2( SrcFile, strTxt, finder.GetFileName() );

				// Fin error in HM Results
				CString strToFind = _T( "ERROR" );

				FILE* fb = _wfopen( SrcFile, _T( "rb" ) );
				wchar_t tcLine[1024];
				CString line;
				string message = " An error is found in ";
				string error_message = "";

				if( fb )
				{
					while( NULL != fgetws( tcLine, 1024, fb ) )
					{
						line = tcLine;
						line.Trim( _T( "\r\n" ) );

						if( line.Find( strToFind, 0 ) != -1 )
						{
							error_message += (string)(CStringA)finder.GetFileName() + "\r\n";
						}
					}

					fclose( fb );
				}

				if( !error_message.empty() )
				{
					CPPUNIT_FAIL( message + error_message );
				}
			}

		}
		catch( CFileException* )
		{
		}
	}
}

void utests_ImportHM::ImportExistingHMCalcToHMCalC()
{
	// Open existing tsp file with HMCalc mode
	// Import elements from another
	// Write result in txt file
	// compare content with reference txt

	// Get Data folder
	CFileFind finder;
	CFileFind finderFileToOpen;
	CString DataPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_IMPORTHYDRAULICCIRCUITS ), CString( __FILE__ ), true );
	CString OutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_IMPORTHYDRAULICCIRCUITS ), CString( __FILE__ ), false );

	// Read input informations
	CString strInputFile = DataPath + _T( "ImportExistingHMCalc.txt" );
	CStringArray ArrayParam;
	int nElt = 0;
	if( false == strInputFile.IsEmpty() )
	{
		if( false == _ReadInputInfos( &ArrayParam, nElt, strInputFile ) )
		{
			CPPUNIT_FAIL( "Incorrect input file " + (string)(CStringA)strInputFile );
		}
	}
	// file to import
	finder.FindFile( DataPath + ArrayParam[0] );
	// file to open
	BOOL bWorking = finderFileToOpen.FindFile( DataPath + _T( "ImportExistingHMCalc.tsp" ) );

	if( bWorking )
	{
		try
		{
			finder.FindNextFile();
			finderFileToOpen.FindNextFile();
			CString strTsp = DataPath + finder.GetFileName();
			CString strToOpen = DataPath + finderFileToOpen.GetFileName();
			if( false == strToOpen.IsEmpty() )
			{
				// Informations to verify except HM infos
				CString strOutPutInfos = CteEMPTY_STRING;
				// File open
				::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_OPENDOCUMENT, false, (LPARAM)&strToOpen );
				LRESULT lresult = ::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_OPENDOCUMENT, false, (LPARAM)&strToOpen );
				if( lresult == 0 )
				{
					CTableHM* pTableHM = dynamic_cast<CTableHM*>( TASApp.GetpTADS()->GetpHydroModTable() );

					if( NULL != pTableHM )
					{
						CDeletedProductHelper* pProductHelper = new CDeletedProductHelper();
						bool fDisplay = false;
						pProductHelper->VerifyHydroMod( pTableHM->GetIDPtr(), fDisplay );
						delete pProductHelper;
					}
				}
				if( false == strTsp.IsEmpty() )
				{
					strOutPutInfos += pMainFrame->IsHMCalcChecked() ? _T( "true" ) : _T( "false" );
					strOutPutInfos += _T( " / " );

					// Button status before unfreeze
					strOutPutInfos += pMainFrame->IsMenuHMImportEnable() ? _T( "true" ) : _T( "false" );
					strOutPutInfos += _T( " / " );

					strOutPutInfos += pMainFrame->IsRibbonHMImportEnable() ? _T( "true" ) : _T( "false" );
					strOutPutInfos += _T( " / " );

					// Unfreeze
					pMainFrame->Freeze( false );

					// Button status after unfreeze
					strOutPutInfos += pMainFrame->IsMenuHMImportEnable() ? _T( "true" ) : _T( "false" );
					strOutPutInfos += _T( " / " );

					strOutPutInfos += pMainFrame->IsRibbonHMImportEnable() ? _T( "true" ) : _T( "false" );
					strOutPutInfos += _T( " / " );

					// Import
					ImportForUnitTest( strTsp, &ArrayParam, nElt );

					// Button status after import
					strOutPutInfos += pMainFrame->IsMenuHMImportEnable() ? _T( "true" ) : _T( "false" );
					strOutPutInfos += _T( " / " );

					strOutPutInfos += pMainFrame->IsRibbonHMImportEnable() ? _T( "true" ) : _T( "false" );
					strOutPutInfos += _T( " / " );

					// Write the out txt file.
					CString strTxt = OutPath + finderFileToOpen.GetFileTitle() + _T( ".txt" );
					CString SrcFile = DataPath + finderFileToOpen.GetFileTitle() + _T( ".txt" );

					// Copy only the input informations in the output file
					_WriteInputInfos( SrcFile, strTxt );

					// Write output information
					_WriteImportExistingHMCalcInfo( strTxt, strOutPutInfos, false, ArrayParam[0] );

					// Compare with reference file.
					utests_Init::CompareOutAndReferenceFiles2( SrcFile, strTxt, finderFileToOpen.GetFileName() );

					// Fin error in HM Results
					CString strToFind = _T( "ERROR" );

					FILE* fb = _wfopen( SrcFile, _T( "rb" ) );
					wchar_t tcLine[1024];
					CString line;
					string message = " An error is found in ";
					string error_message = "";

					if( fb )
					{
						while( NULL != fgetws( tcLine, 1024, fb ) )
						{
							line = tcLine;
							line.Trim( _T( "\r\n" ) );

							if( line.Find( strToFind, 0 ) != -1 )
							{
								error_message += (string)(CStringA)finder.GetFileName() + "\r\n";
							}
						}

						fclose( fb );
					}

					if( !error_message.empty() )
					{
						CPPUNIT_FAIL( message + error_message );
					}
				}
			}

		}
		catch( CFileException* )
		{
		}
	}
}

void utests_ImportHM::ImportExistingNonHMCalcToNonHMCalc()
{
	// Open existing tsp file with NonHMCalc mode
	// Import elements from another
	// Write result in txt file
	// compare content with reference txt

	// Get Data folder
	CFileFind finder;
	CFileFind finderFileToOpen;
	CString DataPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_IMPORTHYDRAULICCIRCUITS ), CString( __FILE__ ), true );
	CString OutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_IMPORTHYDRAULICCIRCUITS ), CString( __FILE__ ), false );

	CString strInputFile = DataPath + _T( "ImportExistingNonHMCalc.txt" );
	CStringArray ArrayParam;
	int nElt = 0;
	if( false == strInputFile.IsEmpty() )
	{
		if( false == _ReadInputInfos( &ArrayParam, nElt, strInputFile ) )
		{
			CPPUNIT_FAIL( "Incorrect input file " + (string)(CStringA)strInputFile );
		}
	}

	finder.FindFile( DataPath + ArrayParam[0] );
	BOOL bWorking = finderFileToOpen.FindFile( DataPath + _T( "ImportExistingNonHMCalc.tsp" ) );

	if( bWorking )
	{
		try
		{
			finder.FindNextFile();
			finderFileToOpen.FindNextFile();
			CString strTsp = DataPath + finder.GetFileName();
			CString strToOpen = DataPath + finderFileToOpen.GetFileName();
			if( false == strToOpen.IsEmpty() )
			{
				// Information to verify except HM infos
				CString strOutPutInfos = CteEMPTY_STRING;
				// File open
				::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_OPENDOCUMENT, false, (LPARAM)&strToOpen );
				LRESULT lresult = ::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_OPENDOCUMENT, false, (LPARAM)&strToOpen );
				if( lresult == 0 )
				{
					CTableHM* pTableHM = dynamic_cast<CTableHM*>( TASApp.GetpTADS()->GetpHydroModTable() );

					if( NULL != pTableHM )
					{
						CDeletedProductHelper* pProductHelper = new CDeletedProductHelper();
						bool fDisplay = false;
						pProductHelper->VerifyHydroMod( pTableHM->GetIDPtr(), fDisplay );
						delete pProductHelper;
					}
				}
				if( false == strTsp.IsEmpty() )
				{
					strOutPutInfos += pMainFrame->IsHMCalcChecked() ? _T( "true" ) : _T( "false" );
					strOutPutInfos += _T( " / " );

					// Button status before import
					strOutPutInfos += pMainFrame->IsMenuHMImportEnable() ? _T( "true" ) : _T( "false" );
					strOutPutInfos += _T( " / " );

					strOutPutInfos += pMainFrame->IsRibbonHMImportEnable() ? _T( "true" ) : _T( "false" );
					strOutPutInfos += _T( " / " );

					// Import
					ImportForUnitTest( strTsp, &ArrayParam, nElt );

					// Button status after import
					strOutPutInfos += pMainFrame->IsMenuHMImportEnable() ? _T( "true" ) : _T( "false" );
					strOutPutInfos += _T( " / " );

					strOutPutInfos += pMainFrame->IsRibbonHMImportEnable() ? _T( "true" ) : _T( "false" );
					strOutPutInfos += _T( " / " );

					// Write the out txt file.
					CString strTxt = OutPath + finderFileToOpen.GetFileTitle() + _T( ".txt" );
					CString SrcFile = DataPath + finderFileToOpen.GetFileTitle() + _T( ".txt" );
					// Copy only input infos in the output file
					_WriteInputInfos( SrcFile, strTxt );
					// Write output informations
					_WriteImportNewNonHMCalcInfo( strTxt, strOutPutInfos, ArrayParam[0], false );

					// Compare with reference file.
					utests_Init::CompareOutAndReferenceFiles2( SrcFile, strTxt, finderFileToOpen.GetFileName() );

					// Fin error in HM Results
					CString strToFind = _T( "ERROR" );

					FILE* fb = _wfopen( SrcFile, _T( "rb" ) );
					wchar_t tcLine[1024];
					CString line;
					string message = " An error is found in ";
					string error_message = "";

					if( fb )
					{
						while( NULL != fgetws( tcLine, 1024, fb ) )
						{
							line = tcLine;
							line.Trim( _T( "\r\n" ) );

							if( line.Find( strToFind, 0 ) != -1 )
							{
								error_message += (string)(CStringA)finder.GetFileName() + "\r\n";
							}
						}

						fclose( fb );
					}

					if( !error_message.empty() )
					{
						CPPUNIT_FAIL( message + error_message );
					}
				}
			}

		}
		catch( CFileException* )
		{
		}
	}
}

void utests_ImportHM::ImportForUnitTest( CString strFileNameToImport, CStringArray* pArrayParam, int iArraySize )
{
	// We use the CDlgImportHM class to use some common functions ( ImportHMRecursively and OnBnClickedOpenProject )
	struct param
	{
		CString file;
		CStringArray* pArrayParam;
	};
	param *ImportParam = new param();
	ImportParam->file = strFileNameToImport;
	ImportParam->pArrayParam = pArrayParam;
	::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_IMPORT_UT, 0, (LPARAM)ImportParam );
	ImportParam->pArrayParam = NULL;
	delete ImportParam;
}

bool utests_ImportHM::_ReadInputInfos( CStringArray *pListStr, int &nElt, CString strInputFile )
{
	CString strLine = CteEMPTY_STRING;
	CString fileNameToImport = CteEMPTY_STRING;
	int iLineNumber = 0;
	if( false == strInputFile.IsEmpty() )
	{
		TCHAR buf[1024];
		memset( buf, 0, sizeof( buf ) );
		bool  bStop = false;
		CString strKey = CteEMPTY_STRING;
		CFileTxt* pOutf = new CFileTxt();
		BOOL ret = pOutf->Open( strInputFile, CFile::modeRead );
		if( FALSE == ret )
		{
			if( NULL != pOutf )
			{
				pOutf->Close();
				delete pOutf;
				pOutf = NULL;
			}
			return false;
		}
		while( ( false == bStop ) && ( pOutf->ReadTxtLine( buf, sizeof( buf ), &iLineNumber ) > 0 ) )
		{
			strLine = buf;
			int pos = 0;
			if( 0 == strLine.Compare( _T( "# Output informations." ) ) )
			{
				bStop = true;
				continue;
			}
			else if( 0 == strLine.Compare( _T( "# Input informations." ) ) )
			{
				continue;
			}
			else if( strLine[0] == '#' )
			{
				continue;
			}
			if( true == fileNameToImport.IsEmpty() )
			{
				pos = strLine.Find( _T( "=" ), pos );
				if( -1 != pos )
				{
					strKey = strLine.Tokenize( _T( "=" ), pos );
					strKey.Trim();
					pListStr->Add( strKey );
					fileNameToImport = strKey;
					nElt++;
					continue;
				}
				else
				{
					if( NULL != pOutf )
					{
						pOutf->Close();
						delete pOutf;
						pOutf = NULL;
					}
					return false;
				}
			}
			pos = 0;
			pos = strLine.Find( _T( "=>" ), pos );
			if( -1 != pos )
			{
				pListStr->Add( strLine );
				nElt++;
			}
		}

		if( NULL != pOutf )
		{
			pOutf->Close();
			delete pOutf;
			pOutf = NULL;
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool utests_ImportHM::_WriteInputInfos( CString strSrc, CString strDest )
{
	CString strLine = CteEMPTY_STRING;
	int iLineNumber = 0;
	if( false == strSrc.IsEmpty() && false == strDest.IsEmpty() )
	{
		TCHAR buf[1024];
		memset( buf, 0, sizeof( buf ) );
		bool  bStop = false;
		CString strKey = CteEMPTY_STRING;
		CFileTxt* pSrcf = new CFileTxt();
		CFileTxt* pOutf = new CFileTxt();
		BOOL retSrc = pSrcf->Open( strSrc, CFile::modeRead );
		BOOL retDest = pOutf->Open( strDest, CFile::modeCreate | CFile::modeWrite );
		if( FALSE == retSrc || FALSE == retDest )
		{
			if( NULL != pSrcf )
			{
				pSrcf->Close();
				delete pSrcf;
				pSrcf = NULL;
			}
			if( NULL != pOutf )
			{
				pOutf->Close();
				delete pOutf;
				pOutf = NULL;
			}
			return false;
		}
		pOutf->SeekToEnd();

		CString str;
		while( ( false == bStop ) && ( pSrcf->ReadTxtLine( buf, sizeof( buf ), &iLineNumber ) > 0 ) )
		{
			USES_CONVERSION;
			strLine = buf;
			int pos = 0;
			if( 0 == strLine.Compare( _T( "# Output informations." ) ) )
			{
				str = _T( "\r\n" ) + strLine;
				pOutf->Write( T2A( str.GetBuffer() ), str.GetLength() );
				bStop = true;
				continue;
			}
			// Write 
			strLine += _T( "\r\n" );
			pOutf->Write( T2A( strLine.GetBuffer() ), strLine.GetLength() );
		}
		if( NULL != pSrcf )
		{
			pSrcf->Close();
			delete pSrcf;
			pSrcf = NULL;
		}
		if( NULL != pOutf )
		{
			pOutf->Close();
			delete pOutf;
			pOutf = NULL;
		}
		return true;
	}
	return false;
}

void utests_ImportHM::_FillNonHMCalc( CFileTxt* pfw, CTable* pTab )
{
	if( NULL == pTab )
	{
		pTab = TASApp.GetpTADS()->GetpHydroModTable();
	}

	// Order HM by position.
	std::map<int, CDS_HydroMod*> mapHM;
	std::map<int, CDS_HydroMod*>::iterator It;

	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		CDS_HydroMod* pHM = dynamic_cast<CDS_HydroMod*>( idptr.MP );
		mapHM[pHM->GetPos()] = pHM;
	}
	CString str;
	for( It = mapHM.begin(); It != mapHM.end(); ++It )
	{
		USES_CONVERSION;
		str.Format( _T( "%s \r\n" ), It->second->GetHMName() );
		pfw->Write( T2A( str.GetBuffer() ), str.GetLength() );

		if( true == It->second->IsaModule() )
		{
			_FillNonHMCalc( pfw, It->second );
		}
	}
}

void utests_ImportHM::_WriteImportNewHMCalcInfo( CString strfn, CString strInfos, bool fClearFile, CString filename )
{
	CFileTxt fw;

	try
	{
		int ret;
		UINT OpenFlag = CFile::modeCreate | CFile::modeReadWrite | CFile::typeUnicode;

		if( false == fClearFile )
		{
			OpenFlag |= CFile::modeNoTruncate;
		}

		ret = fw.Open( strfn, OpenFlag );

		if( 0 == ret )
		{
			throw;
		}

		fw.SeekToEnd();
		CString str;
		USES_CONVERSION;
		if( true == fClearFile ) fw.WriteTxtLine( _T( "\xFEFF" ) );

		// Write visual information
		int nStr = 0;
		CStringArray StrArray;
		ParseString( strInfos, _T( "/" ), &nStr, &StrArray );

		str.Format( _T( "\r\n\r\nHMCalc mode = %s" ), StrArray[0] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "\r\nMenu button state before import = %s" ), StrArray[1] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "\r\nRibbon button state before import = %s" ), StrArray[2] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "\r\nMenu button state after import = %s" ), StrArray[3] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "\r\nRibbon button state after import = %s" ), StrArray[4] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str = _T( "\r\n\r\n// Import result \r\n" );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "Import result : %s in an empty project \r\n" ), filename );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		_FillHMCalc( &fw );

		str.Format( _T( "Rem; --------------------->SUCCESS" ) );

		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );
		fw.Flush();
		fw.Close();
	}
	catch( CFileException* e )
	{
		TCHAR szCause[255];
		CString strFormatted;

		e->GetErrorMessage( szCause, 255 );
		strFormatted = _T( "Error: " );
		strFormatted += szCause;
		AfxMessageBox( strFormatted );
	}
}

void utests_ImportHM::_WriteImportNewNonHMCalcInfo( CString strfn, CString strInfos, CString fileNameImported, bool fClearFile )
{
	CFileTxt fw;

	try
	{
		int ret;
		UINT OpenFlag = CFile::modeCreate | CFile::modeReadWrite | CFile::typeUnicode;

		if( false == fClearFile )
		{
			OpenFlag |= CFile::modeNoTruncate;
		}

		ret = fw.Open( strfn, OpenFlag );

		if( 0 == ret )
		{
			throw;
		}

		fw.SeekToEnd();

		//Write header.
		CString str;
		USES_CONVERSION;
		if (true == fClearFile) fw.WriteTxtLine( _T( "\xFEFF" ) );

		// Write visual information
		int nStr = 0;
		CStringArray StrArray;
		ParseString( strInfos, _T( "/" ), &nStr, &StrArray );
		str.Format( _T( "\r\n\r\nHMCalc mode = %s" ), StrArray[0] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "\r\nMenu button state before import = %s" ), StrArray[1] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "\r\nRibbon button state before import = %s" ), StrArray[2] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "\r\nMenu button state after import = %s" ), StrArray[3] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "\r\nRibbon button state after import = %s" ), StrArray[4] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str = _T( "\r\n\r\n// Import result \r\n" );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "Import result : %s imported\r\n" ), fileNameImported );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		_FillNonHMCalc( &fw );

		str = _T( "Rem; --------------------->SUCCESS" );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		fw.Flush();
		fw.Close();
	}
	catch( CFileException* e )
	{
		TCHAR szCause[255];
		CString strFormatted;

		e->GetErrorMessage( szCause, 255 );
		strFormatted = _T( "Error: " );
		strFormatted += szCause;
		AfxMessageBox( strFormatted );
	}
}

void utests_ImportHM::_WriteImportExistingHMCalcInfo( CString strfn, CString strInfos, bool fClearFile, CString filename )
{
	CFileTxt fw;

	try
	{
		int ret;
		UINT OpenFlag = CFile::modeCreate | CFile::modeReadWrite;

		if( false == fClearFile )
		{
			OpenFlag |= CFile::modeNoTruncate;
		}

		ret = fw.Open( strfn, OpenFlag );

		if( 0 == ret )
		{
			throw;
		}

		fw.SeekToEnd();

		//Write header.
		CString str;
		USES_CONVERSION;
		if( true == fClearFile ) fw.WriteTxtLine( _T( "\xFEFF" ) );

		// Write visual information
		int nStr = 0;
		CStringArray StrArray;
		ParseString( strInfos, _T( "/" ), &nStr, &StrArray );
		str.Format( _T( "\r\n\r\nHMCalc mode = %s" ), StrArray[0] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "\r\nMenu button state before unfreeze = %s" ), StrArray[1] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "\r\nRibbon button state before unfreeze = %s" ), StrArray[2] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "\r\nMenu button state after unfreeze = %s" ), StrArray[3] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "\r\nRibbon button state after unfreeze = %s" ), StrArray[4] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "\r\nMenu button state after import = %s" ), StrArray[5] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "\r\nRibbon button state after import = %s" ), StrArray[6] );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str = _T( "\r\n\r\n// Import result \r\n" );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		str.Format( _T( "Import result : %s in an existing project \r\n" ), filename );
		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );

		_FillHMCalc( &fw );

		str.Format( _T( "Rem; --------------------->SUCCESS" ) );

		fw.Write( T2A( str.GetBuffer() ), str.GetLength() );
		fw.Flush();
		fw.Close();
	}
	catch( CFileException* e )
	{
		TCHAR szCause[255];
		CString strFormatted;

		e->GetErrorMessage( szCause, 255 );
		strFormatted = _T( "Error: " );
		strFormatted += szCause;
		AfxMessageBox( strFormatted );
	}
}

void utests_ImportHM::_FillHMCalc( CFileTxt* pfw, CTable* pTab )
{
	if( NULL == pTab )
	{
		pTab = TASApp.GetpTADS()->GetpHydroModTable();
	}
	if( NULL == pfw )
	{
		return;
	}
	// Order HM by position.
	std::map<int, CDS_HydroMod*> mapHM;
	std::map<int, CDS_HydroMod*>::iterator It;

	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		CDS_HydroMod* pHM = dynamic_cast<CDS_HydroMod*>( idptr.MP );
		mapHM[pHM->GetPos()] = pHM;
	}

	for( It = mapHM.begin(); It != mapHM.end(); ++It )
	{
		_WriteHMInfo( pfw, It->second );

		if( true == It->second->IsaModule() )
		{
			_FillHMCalc( pfw, It->second );
		}
	}
}

void utests_ImportHM::_WriteHMInfo( CFileTxt* pfw, CDS_HydroMod* pHM )
{
	CString LineHeader;
	CString Line, str;
	if( NULL == pfw )
	{
		return;
	}
	LineHeader.Format( _T( "%s; %s; " ), pHM->GetHMName(), pHM->GetSchemeID() );

	// HM, TuDp.
	str = WriteCUDouble( _U_DIFFPRESS, pHM->GetpTermUnit()->GetDp(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " HM; TU-Dp;" ) + str + _T( "\r\n" );
	USES_CONVERSION;
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );

	// HM, TuQ.
	str = WriteCUDouble( _U_FLOW, pHM->GetpTermUnit()->GetQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " HM; TU-Q;" ) + str + _T( "\r\n" );
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );

	// HM, Dp.
	str = WriteCUDouble( _U_DIFFPRESS, pHM->GetDp(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " HM; Dp;" ) + str + _T( "\r\n" );
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );

	// HM, Flow.
	str = WriteCUDouble( _U_FLOW, pHM->GetQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " HM; FLOW;" ) + str + _T( "\r\n" );
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );

	// HM, Havail.
	str = WriteCUDouble( _U_DIFFPRESS, pHM->GetHAvail(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " HM; HAvail;" ) + str + _T( "\r\n" );
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );

	Line = LineHeader + _T( " ***ERROR*** Havail is < Dp;" ) + _T( "\r\n" );

	CDB_PIControlValve* pPICV = NULL;

	if( NULL != pHM->GetpCV() )
	{
		pPICV = dynamic_cast<CDB_PIControlValve*>( pHM->GetpCV()->GetCvIDPtr().MP );
	}

	if( NULL != pPICV )
	{
		// if( pHM->GetHAvail() < pHM->GetDp() )
		if( pHM->GetDp() - pHM->GetHAvail() > 2 )
		{
			pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
		}
	}
	else if( abs( pHM->GetHAvail() - pHM->GetDp() ) > 2 )
	{
		Line = LineHeader + _T( " ***ERROR*** Havail is > Dp + 2 ;" ) + _T( "\r\n" );
		pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
	}


	if( true == pHM->IsBvExist( ( true ) ) )
	{
		_WriteBVInfo( pfw, LineHeader + _T( " BVP;" ), pHM->GetpBv() );
	}

	if( true == pHM->IsBvBypExist( ( true ) ) )
	{
		_WriteBVInfo( pfw, LineHeader + _T( " BVB;" ), pHM->GetpBypBv() );
	}

	if( true == pHM->IsBvSecExist( ( true ) ) )
	{
		_WriteBVInfo( pfw, LineHeader + _T( " BVS;" ), pHM->GetpSecBv() );
	}

	if( true == pHM->IsCvExist( ( true ) ) )
	{
		_WriteCVInfo( pfw, LineHeader + _T( " CV;" ), pHM->GetpCV() );
	}

	if( true == pHM->IsDpCExist( ( true ) ) )
	{
		_WriteDpCInfo( pfw, LineHeader + _T( " DpC;" ), pHM->GetpDpC() );
	}

	_WritePipeInfo( pfw, LineHeader + _T( " CIRCPIPE;" ), pHM->GetpCircuitPrimaryPipe() );
	_WritePipeInfo( pfw, LineHeader + _T( " SECCIRCPIPE;" ), pHM->GetpCircuitSecondaryPipe() );
	_WritePipeInfo( pfw, LineHeader + _T( " DISTSUPPIPE;" ), pHM->GetpDistrSupplyPipe() );
	_WritePipeInfo( pfw, LineHeader + _T( " DISTRETPIPE;" ), pHM->GetpDistrReturnPipe() );
	_WritePipeAccessoryInfo( pfw, LineHeader + _T( " CIRCACC;" ), pHM->GetpCircuitPrimaryPipe() );
	_WritePipeAccessoryInfo( pfw, LineHeader + _T( " SECCIRCACC;" ), pHM->GetpCircuitSecondaryPipe() );
	_WritePipeAccessoryInfo( pfw, LineHeader + _T( " DISTSUPACC;" ), pHM->GetpDistrSupplyPipe() );
	_WritePipeAccessoryInfo( pfw, LineHeader + _T( " DISTRETPIPE;" ), pHM->GetpDistrReturnPipe() );
}

void utests_ImportHM::_WritePipeAccessoryInfo( CFileTxt* pfw, CString LineHeader, CPipes* pPipe )
{
	if( NULL == pPipe )
	{
		return;
	}

	CString Line, str;
	// Dp
	str = WriteCUDouble( _U_DIFFPRESS, pPipe->GetSingularityTotalDp( true ), true, 6, 0, true, _U_DIFFPRESS_PA );	// Flow -> l/h
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " Dp;" ) + str + _T( "\r\n" );
	USES_CONVERSION;
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
}

void utests_ImportHM::_WritePipeInfo( CFileTxt* pfw, CString LineHeader, CPipes* pPipe )
{
	if( NULL == pPipe )
	{
		return;
	}

	CString Line, str;
	// NAME
	// 	str = pPipe->GetPipeSeries()->GetName();
	// 	Line = LineHeader + _T(" PipeNAME;") + str + _T("\n");
	// 	pfw->WriteTxtLine(Line);
	// Dp
	str = WriteCUDouble( _U_DIFFPRESS, pPipe->GetPipeDp(), true, 6, 0, true, _U_DIFFPRESS_PA ); // Dp -> Pascal
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " Dp;" ) + str + _T( "\r\n" );
	USES_CONVERSION;
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
}

void utests_ImportHM::_WriteDpCInfo( CFileTxt* pfw, CString LineHeader, CDS_HydroMod::CDpC* pDpC )
{
	CString Line, str;
	// NAME
	str = pDpC->GetpTAP()->GetName();
	Line = LineHeader + _T( " DpCNAME;" ) + str + _T( "\r\n" );
	USES_CONVERSION;
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
	// Dp
	str = WriteCUDouble( _U_DIFFPRESS, pDpC->GetDp(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " Dp;" ) + str + _T( "\r\n" );
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
	// Flow
	str = WriteCUDouble( _U_FLOW, pDpC->GetQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " Flow;" ) + str + _T( "\r\n" );
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
	// Presetting
	str = pDpC->GetSettingStr( false );
	str.Remove( '*' );
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " PRESET;" ) + str + _T( "\r\n" );
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
}

void utests_ImportHM::_WriteBVInfo( CFileTxt* pfw, CString LineHeader, CDS_HydroMod::CBV* pBV )
{
	CString Line, str;
	// NAME
	str = pBV->GetpTAP()->GetName();
	Line = LineHeader + _T( " BVNAME;" ) + str + _T( "\r\n" );
	USES_CONVERSION;
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
	// Dp
	str = WriteCUDouble( _U_DIFFPRESS, pBV->GetDp(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " Dp;" ) + str + _T( "\r\n" );
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
	// Flow
	str = WriteCUDouble( _U_FLOW, pBV->GetQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " Flow;" ) + str + _T( "\r\n" );
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
	// Presetting
	str = pBV->GetSettingStr( false );
	str.Replace( _T( "," ), _T( "." ) );
	str.Remove( '*' );

	if( false == str.IsEmpty() )
	{
		Line = LineHeader + _T( " PRESET;" ) + str + _T( "\r\n" );
		pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
	}
}

void utests_ImportHM::_WriteCVInfo( CFileTxt* pfw, CString LineHeader, CDS_HydroMod::CCv* pCV )
{
	CString Line, str;
	USES_CONVERSION;
	// NAME
	if( true == pCV->IsTaCV() )
	{
		str = pCV->GetpTAP()->GetName();
		Line = LineHeader + _T( " CVNAME;" ) + str + _T( "\r\n" );
		pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
		// Presetting
		str = pCV->GetSettingStr( false );
		str.Remove( '*' );
		str.Replace( _T( "," ), _T( "." ) );

		if( false == str.IsEmpty() )
		{
			Line = LineHeader + _T( " PRESET;" ) + str + _T( "\r\n" );
			pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
		}

		// Actuator
		CDB_Actuator* pActr = dynamic_cast<CDB_Actuator*>( pCV->GetActrIDPtr().MP );

		if( NULL != pActr )
		{
			str = pActr->GetName();
			Line = LineHeader + _T( " CVACTR;" ) + str + _T( "\r\n" );
			pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
		}
	}
	else
	{
		if( false == pCV->IsOn_Off() )
		{
			// Test Kvs < Kvsmax
			if( pCV->GetAuth() < 0.245 )
			{
				Line = LineHeader + _T( " ***ERROR*** Authority is < 0.25;" ) + _T( "\r\n" );
				pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
			}

			str = WriteDouble( pCV->GetAuth(), 3, 2 );
			str.Replace( _T( "," ), _T( "." ) );
			Line = LineHeader + _T( " BetaMin;" ) + str + _T( "\r\n" );
			pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
			str = WriteDouble( pCV->GetAuth( true ), 3, 2 );
			str.Replace( _T( "," ), _T( "." ) );
			Line = LineHeader + _T( " BetaDesign;" ) + str + _T( "\r\n" );
			pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
		}
	}

	// Test Kvs < Kvsmax
	if( true == pCV->IsTaCV() && true == pCV->IsPresettable() )
	{
		if( ( pCV->GetKv() - pCV->GetKvsmax() ) > 0.01 )
		{
			Line = LineHeader + _T( " ***ERROR*** Kv > Kvs max;" ) + _T( "\r\n" );
			pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
		}

		//Kv
		str = WriteDouble( pCV->GetKv(), 3, 2 );
		str.Replace( _T( "," ), _T( "." ) );
		Line = LineHeader + _T( " Kv;" ) + str + _T( "\r\n" );
		pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
	}
	else
	{
		if( ( pCV->GetKvs() - pCV->GetKvsmax() ) > 0.01 )
		{
			Line = LineHeader + _T( " ***ERROR*** Kvs > Kvs max;" ) + _T( "\r\n" );
			pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
		}

		//Kvs
		str = WriteDouble( pCV->GetKvs(), 3, 2 );
		str.Replace( _T( "," ), _T( "." ) );
		Line = LineHeader + _T( " Kvs;" ) + str + _T( "\r\n" );
		pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
	}

	//Kvs Max
	str = WriteDouble( pCV->GetKvsmax(), 3, 2 );
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " KvsMax;" ) + str + _T( "\r\n" );
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
	// Dp
	str = WriteCUDouble( _U_DIFFPRESS, pCV->GetDp(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " Dp;" ) + str + _T( "\r\n" );
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
	// Flow
	str = WriteCUDouble( _U_FLOW, pCV->GetQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
	str.Replace( _T( "," ), _T( "." ) );
	Line = LineHeader + _T( " Flow;" ) + str + _T( "\r\n" );
	pfw->Write( T2A( Line.GetBuffer() ), Line.GetLength() );
}