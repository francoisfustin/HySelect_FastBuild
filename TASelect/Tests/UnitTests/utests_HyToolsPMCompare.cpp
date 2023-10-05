#include "stdafx.h"

#include "utests_base.h"
#include "mainfrm.h"
#include "utests_HytoolsPMCompare.h"
#include "DlgSpecActHyToolsPMCompare.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_HytoolsPMCompare, CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION );

void utests_HytoolsPMCompare::VerifyResults( CString strFileNameIn, CString strFileNameTemp, CString strFileNameOut )
{
	CString DataPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ), true );
	bool isUnixFormat = true;
	wifstream fin( DataPath + strFileNameIn, std::ifstream::binary );
	if( fin.is_open() )
	{
		unsigned short ch = fin.get();

		while( !fin.eof() )
		{
			if( ch == '\r' )
			{
				isUnixFormat = false;
				break;
			}
			ch = fin.get();
		}

		if( isUnixFormat == true )
		{
			ofstream fou( DataPath + strFileNameTemp, std::ifstream::out );

			fin.clear();
			fin.seekg( 0, std::ios::beg );
			ch = fin.get();
			while( !fin.eof() )
			{
				fou.put( (char)ch );
				ch = fin.get();
			}
			fin.clear();
			fin.close();
			fou.close();
			DeleteFile( DataPath + strFileNameIn );
			rename( (CStringA)( DataPath + strFileNameTemp ), (CStringA)( DataPath + strFileNameIn ) );
		}
	}

	CString OutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ), false );
	CDlgSpecActHyToolsPMCompare* Dlg = new CDlgSpecActHyToolsPMCompare();

	Dlg->SetInputSelParam();
	Dlg->ExportResults( DataPath + strFileNameIn );
	DeleteFile( OutPath + strFileNameIn );
	bool result = MoveFile( DataPath + strFileNameOut, OutPath + strFileNameIn );
	if ( true == result )
		utests_Init::CompareOutAndReferenceFiles2( DataPath + strFileNameIn, OutPath + strFileNameIn, strFileNameIn );
	delete Dlg;
}

void utests_HytoolsPMCompare::TestHyToolsPMCompare()
{
	// List txt file
	// Open txt file
	// verify results HyTools and HySelect

	// Get Data folder
	CFileFind finder;
	CString DirPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ), true );
	CString DirOutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ), false );

	// Delete previous result
	DeleteFilesAndFolder( DirOutPath, _T("*.txt") );

	if( DirPath.ReverseFind( '\\' ) != DirPath.GetLength() - 1 )
	{
		DirPath += CString( _T("\\") );
	}

	// For each txt file
	BOOL bWorking = finder.FindFile( DirPath + _T("*.txt") );

	while( bWorking )
	{
		bWorking = finder.FindNextFile();

		try
		{

			if( finder.IsDots() )
			{
				continue;
			}

			CString strTxtFile = DirPath + finder.GetFileName();

			if( false == strTxtFile.IsEmpty() )
			{
				CString strTemp = _T("file_temp.txt");
				CString strOut = finder.GetFileName();
				strOut.Insert( strOut.ReverseFind( _T( '.' ) ), _T("-OUT") );
				VerifyResults( finder.GetFileName(), strTemp, strOut );
			}

		}
		catch( CFileException * )
		{
		}
	}
}


