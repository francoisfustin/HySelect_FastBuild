#include "stdafx.h"

#include "utests_base.h"
#include "utests_LocalizedString.h"
#include "Resource.h"
#include "DataBase.h"
#include "Global.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_LocalizedString, CPPUNIT_TEST_CATEGORYNAME_DATABASE );

void utests_LocalizedString::checkParameterCountRC()
{
	checkParameterCount( _TASRC_XML_FILE, "RC" );
}

void utests_LocalizedString::checkParameterCountTips()
{
	checkParameterCount( _TASTIPS_XML_FILE, "Tips" );
}

void utests_LocalizedString::checkParameterCountST()
{
	checkParameterCount( _STRINGTAB_XML_FILE, "ST" );
}

void utests_LocalizedString::checkParameterCount( const char *xmlFile, const char *fileType )
{
	bool testSuccess = true;

	// parameter to check
	// HYS-966: Add "<" and ">"
	const wchar_t *checkList[] = { L"%1", L"%2", L"%3", L"%d", L"%s", L"<", L">" };
	const size_t checkCount = sizeof( checkList ) / sizeof( *checkList );

	// list of lang to check ("en" must be first)
	std::vector<std::wstring> vlangList;
	CXmlRcTab::GetLangList( std::string( xmlFile ), vlangList );
	const size_t langCount = vlangList.size();

	CXmlRcTab *XmlRcTab = new CXmlRcTab[langCount];

	char *errorInLang = new char[langCount];
	memset( errorInLang, 0, sizeof( char ) * langCount );

	// load string for every language
	for( unsigned int h = 0; h < langCount; h++ )
	{
		XmlRcTab[h].Init( std::string( xmlFile ), vlangList[h].c_str() );
	}

	int *checkCardinality = new int[langCount * checkCount];

	CString strTest;
	int foundPos = 0;

	CFileTxt *stringError[64];
	memset( stringError, 0, langCount * sizeof( CFileTxt * ) );
	CString stringSummary[64];

	for( unsigned int i = 0; i < langCount; i++ )
	{
		CString strFileName = vlangList[i].c_str();
		strFileName += "-LocalizedStringError";
		strFileName += fileType;
		strFileName += ".txt";
		stringError[i] = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
		stringSummary[i] = vlangList[i].c_str();
		stringSummary[i] += L"=";
	}

	for( int j = 0; j < XmlRcTab[0].GetIDCount(); j++ )
	{
		memset( checkCardinality, 0, sizeof( int ) *langCount * checkCount );

		for( unsigned int i = 0; i < langCount; i++ )
		{
			strTest = ( j ? XmlRcTab[i].GetNextIDSStr() : XmlRcTab[i].GetFirstIDSStr() );

			for( int k = 0; k < checkCount; k++ )
			{
				// count the number of match for every parameter possible
				foundPos = strTest.Find( checkList[k], foundPos );

				while( foundPos != -1 )
				{
					checkCardinality[( i * checkCount ) + k]++;
					foundPos++;
					foundPos = strTest.Find( checkList[k], foundPos );
				}

				foundPos = 0;
			}

			strTest.Empty();
		}

		for( unsigned int m = 1; m < langCount; m++ )
		{
			for( unsigned int n = 0; n < checkCount; n++ )
			{
				// check if the number of parameter is the same as in English
				if( checkCardinality[( m * checkCount ) + n] != checkCardinality[n] )
				{
					wchar_t idsnumber[256];
					swprintf( idsnumber, 256, L"%d", XmlRcTab[m].GetIDSByPos( j ) );
					testSuccess = false;
					stringSummary[m] += idsnumber;
					stringSummary[m] += L";";

					_string errorMsg;
					errorMsg = L"lang=";
					errorMsg += vlangList[m].c_str();
					errorMsg += L";IDS=";
					errorMsg += idsnumber;
					errorMsg += L";str=";
					errorMsg += XmlRcTab[m].GetIDSStrByPos( j ); 
					errorMsg += L"\n";

					stringError[m]->WriteTxtLine( errorMsg.c_str() );
					errorInLang[m] = 1;

					n = checkCount; // log wrong string only once
				}
			}
		}
	}

	delete[] XmlRcTab;
	delete[] checkCardinality;

	CString strFileSummaryName;
	strFileSummaryName = "ALL-LocalizedStringError";
	strFileSummaryName += fileType;
	strFileSummaryName += ".txt";

	CFileTxt *fileSummary = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileSummaryName );

	for( unsigned int i = 0; i < langCount; i++ )
	{
		utests_Init::CloseTxtFile( stringError[i] );
		fileSummary->WriteTxtLine( stringSummary[i] + L"\n" );

		if( 0 == errorInLang[i] )
		{
			CString fileName = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ) );
			fileName += vlangList[i].c_str();
			fileName += "-LocalizedStringError";
			fileName += fileType;
			fileName += ".txt";
			DeleteFile( fileName );
		}
	}

	utests_Init::CloseTxtFile( fileSummary );

	delete[] errorInLang;

	CPPUNIT_ASSERT_MESSAGE( "Difference in localization : see list in output file", true == testSuccess );
}
