#include "stdafx.h"

#include "utests_base.h"
#include "utests_TenderText.h"
#include "DataBase.h"
#include "TASelect.h"
#include "Tender.h"
#include "CsvParser.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_TenderText, CPPUNIT_TEST_CATEGORYNAME_DATABASE );

bool test_predicate_digit( char c )
{
	return !isdigit( c );
}

void utests_TenderText::CheckIndexEncodingSupport()
{
	CString zipPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE), CString( __FILE__ ), true ) + CString( "encoding.zip" );
	CTender tender;
	tender.SetZipPath( ( const TCHAR * )zipPath );

	_string strTender;

	// rtf file
	tender.GetTenderTxt( L"12345", strTender );
	CPPUNIT_ASSERT( _string( L"12345" ) == strTender );

	// txt file
	tender.GetTenderTxt( L"67890", strTender );
	CPPUNIT_ASSERT( _string( L"67890" ) == strTender );

	// use index.csv to concatenate rtf and txt
	tender.GetTenderTxt( L"1234567890", strTender );
	CPPUNIT_ASSERT( _string( L"12345\r\n67890" ) == strTender );

}

void utests_TenderText::CheckCharset()
{
	CDataList *pDL = TASApp.GetpTADB()->GetpDataList();
	size_t iArticleCount = 0;

	for( IDPTR IDPtr = pDL->GetFirst(); NULL != IDPtr.MP; IDPtr = pDL->GetNext() )
	{
		CDB_Product *pclProduct = dynamic_cast<CDB_Product *>( IDPtr.MP );

		if( NULL == pclProduct )
		{
			continue;
		}

		if( true == pclProduct->IsSelectable(true) )
		{
			iArticleCount++;
			const TCHAR *tcArticleNumber = pclProduct->GetBodyArtNum();
			size_t iArtNumLen = wcslen( tcArticleNumber );

			if( 0 == iArtNumLen )
			{
				CString str;
				str.Format( _T("%s -> article number empty!"), pclProduct->GetIDPtr().ID );
				USES_CONVERSION;
				CPPUNIT_FAIL( W2A( str ) );
			}

			for( unsigned int i = 0; i < iArtNumLen; ++i )
			{
				if( !( isdigit( tcArticleNumber[i] )
					   || tcArticleNumber[i] == _T('-')
					   || tcArticleNumber[i] == _T('_')
					   || tcArticleNumber[i] == _T(' ')
					   || tcArticleNumber[i] == _T('E')
					   || tcArticleNumber[i] == _T('S')
					   || tcArticleNumber[i] == _T('F')
					   || tcArticleNumber[i] == _T('.')
					   || tcArticleNumber[i] == _T('&')
					   || tcArticleNumber[i] == _T(' ') ) )
				{
					CString str;
					str.Format( _T("%s -> not a valid character in article number"), pclProduct->GetIDPtr().ID );
					//USES_CONVERSION;
					//CPPUNIT_FAIL( W2A( str ) );
				}
			}
		}
		else if( true == pclProduct->IsDeleted() )
		{
			const TCHAR *tcArticleNumber = pclProduct->GetBodyArtNum();
			size_t iArtNumLen = wcslen(tcArticleNumber);

			if( 0 == iArtNumLen )
			{
				CString str;
				str.Format( _T("%s -> article number empty!"), pclProduct->GetIDPtr().ID );
				USES_CONVERSION;
				CPPUNIT_FAIL( W2A( str ) );
			}

			for( unsigned int i = 0; i < iArtNumLen; ++i )
			{
				if( !(tcArticleNumber[i] == _T('-')
					|| tcArticleNumber[i] == _T('_')
					|| tcArticleNumber[i] == _T(' ')
					|| tcArticleNumber[i] == _T('*')
					|| tcArticleNumber[i] == _T('.')
					|| tcArticleNumber[i] == _T('&')
					|| tcArticleNumber[i] == _T('\"')
					|| tcArticleNumber[i] == _T(' ') ) )
				{
					CString str;
					str.Format( _T("%s -> not a valid character in deleted article number"), pclProduct->GetIDPtr().ID );
					USES_CONVERSION;
					CPPUNIT_FAIL( W2A( str ) );
				}
			}
		}
	}
}

void utests_TenderText::CheckTenderMissing()
{
	// TODO Change the input path with the path used in production
	CString zipPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE), CString( __FILE__ ), true ) + CString( "tender.zip" );
	CTender tender;
	tender.SetZipPath( ( const TCHAR * )zipPath );
	std::vector<_string> artList = tender.GetArtNumVector();
	_string tenderTxt;

	CFileTxt *txtMissing = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE), CString( __FILE__ ), CString( "TenderMissing.txt" ) );
	CFileTxt *txtFound = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE), CString( __FILE__ ), CString( "TenderFound.txt" ) );

	for( unsigned int i = 0; i < artList.size(); ++i )
	{
		tender.GetTenderTxt( artList.at( i ), tenderTxt );

		if( tenderTxt.empty() )
		{
			txtMissing->WriteTxtLine( artList.at( i ).c_str() );
			txtMissing->WriteTxtLine( _T("\n") );
		}
		else
		{
			txtFound->WriteTxtLine( artList.at( i ).c_str() );
			txtFound->WriteTxtLine( _T("\n") );
		}
	}

	utests_Init::CloseTxtFile( txtMissing );
	utests_Init::CloseTxtFile( txtFound );
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), CString( "TenderMissingDE.txt" ) );
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), CString( "TenderFound.txt" ) );

}

void utests_TenderText::CheckTenderNotUsed()
{
	// TODO Change the input path with the path used in production
	CString zipPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE), CString( __FILE__ ), true ) + CString( "tender.zip" );
	CTender tender;
	tender.SetZipPath( ( const TCHAR * )zipPath );
	tender.PublicOpenZip();
	std::vector<_string> artList = tender.GetArtNumVector();
	std::vector<_string> tenderList = tender.GetArtNumTenderVector();
	std::vector<int> tenderUsed;

	for( unsigned int i = 0; i < artList.size(); ++i )
	{
		int index = tender.GetTenderID( artList[i] );

		if( index >= 0 )
		{
			tenderUsed.push_back( index );
		}
	}

	// sort and remove doublon;
	std::sort( tenderUsed.begin(), tenderUsed.end() );
	auto last = std::unique( tenderUsed.begin(), tenderUsed.end() );
	// change the physical size to fit the real size
	tenderUsed.erase( last, tenderUsed.end() );
	CFileTxt *txtNotUsed = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE), CString( __FILE__ ), CString( "TenderNotUsed.txt" ) );

	for( unsigned int i = 0; i < tenderList.size(); ++i )
	{
		if( ( tenderUsed.empty() || i != tenderUsed[0] ) && false == tenderList[i].empty() )
		{
			txtNotUsed->WriteTxtLine( tenderList[i].c_str() );
			txtNotUsed->WriteTxtLine( _T("\n") );
		}

		if( false == tenderUsed.empty() && i == tenderUsed[0] )
		{
			tenderUsed.erase( tenderUsed.begin() );
		}
	}

	utests_Init::CloseTxtFile( txtNotUsed );
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), CString( "TenderNotUsed.txt" ) );
}

void utests_TenderText::RemoveUnusedTender()
{
	CString zipPathIn = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE), CString( __FILE__ ), true ) + CString( "tender.zip" );
	CString zipPathOut = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE), CString( __FILE__ ), false ) + CString( "tender.zip" );

	CopyFile( zipPathIn, zipPathOut, false );
	CTender tender;
	tender.SetZipPath( zipPathOut );
	tender.RemoveUnusedTender();
}

void utests_TenderText::CsvFormatRule1()
{
	CsvParser *csvparser = CsvParser_new_from_string( L"aaa;bbb;ccc\r\nzzz;yyy;xxx\r\n", ";", 0 );
	CsvRow *row = CsvParser_getRow( csvparser );
	TCHAR **rowFields = CsvParser_getFields( row );

	CPPUNIT_ASSERT( row->numOfFields_ == 3 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[0], L"aaa" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[1], L"bbb" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[2], L"ccc" ) == 0 );

	CsvParser_destroy_row( row );
	row = CsvParser_getRow( csvparser );
	rowFields = CsvParser_getFields( row );

	CPPUNIT_ASSERT( row->numOfFields_ == 3 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[0], L"zzz" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[1], L"yyy" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[2], L"xxx" ) == 0 );

	CsvParser_destroy_row( row );
	CsvParser_destroy( csvparser );
}

void utests_TenderText::CsvFormatRule2()
{
	CsvParser *csvparser = CsvParser_new_from_string( L"aaa;bbb;ccc\r\nzzz;yyy;xxx", ";", 0 );
	CsvRow *row = CsvParser_getRow( csvparser );
	TCHAR **rowFields = CsvParser_getFields( row );

	CPPUNIT_ASSERT( row->numOfFields_ == 3 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[0], L"aaa" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[1], L"bbb" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[2], L"ccc" ) == 0 );

	CsvParser_destroy_row( row );
	row = CsvParser_getRow( csvparser );
	rowFields = CsvParser_getFields( row );

	CPPUNIT_ASSERT( row->numOfFields_ == 3 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[0], L"zzz" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[1], L"yyy" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[2], L"xxx" ) == 0 );

	CsvParser_destroy_row( row );
	CsvParser_destroy( csvparser );
}

void utests_TenderText::CsvFormatRule5()
{
	CsvParser *csvparser = CsvParser_new_from_string( L"\"aaa\";\"bbb\";\"ccc\"\r\nzzz;yyy;xxx", ";", 0 );
	CsvRow *row = CsvParser_getRow( csvparser );
	TCHAR **rowFields = CsvParser_getFields( row );

	CPPUNIT_ASSERT( row->numOfFields_ == 3 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[0], L"aaa" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[1], L"bbb" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[2], L"ccc" ) == 0 );

	CsvParser_destroy_row( row );
	row = CsvParser_getRow( csvparser );
	rowFields = CsvParser_getFields( row );

	CPPUNIT_ASSERT( row->numOfFields_ == 3 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[0], L"zzz" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[1], L"yyy" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[2], L"xxx" ) == 0 );

	CsvParser_destroy_row( row );
	CsvParser_destroy( csvparser );
}

void utests_TenderText::CsvFormatRule6()
{
	CsvParser *csvparser = CsvParser_new_from_string( L"\"a;aa\";\"b\r\nbb\";\"ccc\"\r\nzzz;yyy;xxx", ";", 0 );
	CsvRow *row = CsvParser_getRow( csvparser );
	TCHAR **rowFields = CsvParser_getFields( row );

	CPPUNIT_ASSERT( row->numOfFields_ == 3 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[0], L"a;aa" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[1], L"b\r\nbb" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[2], L"ccc" ) == 0 );

	CsvParser_destroy_row( row );
	row = CsvParser_getRow( csvparser );
	rowFields = CsvParser_getFields( row );

	CPPUNIT_ASSERT( row->numOfFields_ == 3 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[0], L"zzz" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[1], L"yyy" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[2], L"xxx" ) == 0 );

	CsvParser_destroy_row( row );
	CsvParser_destroy( csvparser );
}

void utests_TenderText::CsvFormatRule7()
{
	CsvParser *csvparser = CsvParser_new_from_string( L"\"aaa\";\"b\"\"bb\";\"ccc\"\r\nzzz;yyy;xxx", ";", 0 );
	CsvRow *row = CsvParser_getRow( csvparser );
	TCHAR **rowFields = CsvParser_getFields( row );

	CPPUNIT_ASSERT( row->numOfFields_ == 3 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[0], L"aaa" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[1], L"b\"bb" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[2], L"ccc" ) == 0 );

	CsvParser_destroy_row( row );
	row = CsvParser_getRow( csvparser );
	rowFields = CsvParser_getFields( row );

	CPPUNIT_ASSERT( row->numOfFields_ == 3 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[0], L"zzz" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[1], L"yyy" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[2], L"xxx" ) == 0 );

	CsvParser_destroy_row( row );
	CsvParser_destroy( csvparser );
}

void utests_TenderText::CsvFormatEmpty1()
{
	CsvParser *csvparser = CsvParser_new_from_string( L"\"aaa\";\"bbb\";\"\"", ";", 0 );
	CsvRow *row = CsvParser_getRow( csvparser );
	TCHAR **rowFields = CsvParser_getFields( row );

	CPPUNIT_ASSERT( row->numOfFields_ == 3 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[0], L"aaa" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[1], L"bbb" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[2], L"" ) == 0 );

	CsvParser_destroy_row( row );
	row = CsvParser_getRow( csvparser );
	CPPUNIT_ASSERT_MESSAGE( "Found row after end of csv", row == NULL );
	CsvParser_destroy( csvparser );
}

void utests_TenderText::CsvFormatEmpty2()
{
	CsvParser *csvparser = CsvParser_new_from_string( L"\"aaa\";\"bbb\";\"\"\r\n", ";", 0 );
	CsvRow *row = CsvParser_getRow( csvparser );
	TCHAR **rowFields = CsvParser_getFields( row );

	CPPUNIT_ASSERT( row->numOfFields_ == 3 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[0], L"aaa" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[1], L"bbb" ) == 0 );
	CPPUNIT_ASSERT( _tcscmp( rowFields[2], L"" ) == 0 );

	CsvParser_destroy_row( row );
	row = CsvParser_getRow( csvparser );
	CPPUNIT_ASSERT_MESSAGE( "Found row after end of csv", row == NULL );
	CsvParser_destroy( csvparser );
}