#include "stdafx.h"

#include "utests_base.h"
#include "utests_VerifyArticleNumbers.h"
#include "DataBase.h"
#include "TASelect.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_VerifyArticleNumbers, CPPUNIT_TEST_CATEGORYNAME_DATABASE );

void utests_VerifyArticleNumbers::VerifyProductArticleNumbers()
{
	CDataList *pclDataSet;
	CString strFileName = _T("Articles.txt");
	
	// Data file is created every time.
	CFileTxt *pDataf = utests_Init::CreateOutTxtFile( utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), true ) + strFileName );
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE), CString( __FILE__ ), strFileName );
	
	bool bIsFailed = false;
	
	// Code to extract data.
	pclDataSet = GetpTADB()->GetpDataList();
	IDPTR IDPtr = _NULL_IDPTR;
	unsigned short pos;
	m_pvectArticleList = new vector<CString>();
	CPPUNIT_ASSERT( m_pvectArticleList );
	CPPUNIT_ASSERT( pclDataSet );

	_string *pOnRequest = GetpXmlStrTab()->GetIDSStr( _T("IDS_ON_REQUEST") );
	CString strOnRequest = _T("");

	if( NULL != pOnRequest )
	{
		strOnRequest = pOnRequest->c_str();
	}

	for( IDPtr = pclDataSet->GetFirst( pos ); NULL != IDPtr.MP; IDPtr = pclDataSet->GetNext( pos ) )
	{
		if( false == IDPtr.MP->IsSelectable(true) || ( ( NULL != dynamic_cast<CDB_Thing *>( IDPtr.MP ) )
				&& ( true == dynamic_cast<CDB_Thing *>( IDPtr.MP )->IsForHub() || true == dynamic_cast<CDB_Thing *>( IDPtr.MP )->IsForHubStation() ) ) )
		{
			continue;
		}

		try
		{
			IDPtr.MP->CrossVerifyID();
		}
		catch( CHySelectException &clHySelectException )
		{
			// To avoid compilation warning.
			clHySelectException.ClearMessages();

			continue;
		}

		if( NULL == dynamic_cast<CDB_Product *>( IDPtr.MP ) )
		{
			continue;
		}

		CString strArtNumTemp = IDPtr.MP->GetArtNum();

		if( false == strOnRequest.IsEmpty() )
		{
			if( 0 == strArtNumTemp.CompareNoCase( strOnRequest ) )
			{
				// Do not check for example when article number is "On request".
				continue;
			}
		}

		if( 0 == strArtNumTemp.CompareNoCase( _T("-") ) )
		{
			// Do not check for example when article number is for a part of set.
			continue;
		}

		CString strCleanedAN = _T( "" );
		StrCleanArticleNumber( strArtNumTemp, &strCleanedAN );
		vector<CString>::iterator it = find( m_pvectArticleList->begin(), m_pvectArticleList->end(), strCleanedAN );
		
		if( it != m_pvectArticleList->end() )
		{
			// This article number appears more than one time in the database ???
			CString strOut;
			strOut.Format( _T("Class name = %s ID = %s Reference = %s \n"), IDPtr.MP->GetClassNameW(), IDPtr.ID, strArtNumTemp );
			pOutf->WriteTxtLine( (LPCTSTR)strOut );
		}
		else
		{
			CString str;
			str.Format( _T("Class name = %s ID = %s Reference = %s \n"), IDPtr.MP->GetClassNameW(), IDPtr.ID, strArtNumTemp );
			pDataf->WriteTxtLine( (LPCTSTR)str );
			m_pvectArticleList->push_back( strCleanedAN );
		}
	}

	if( pOutf->GetLength() > 2 )
	{
		bIsFailed = true;
	}

	utests_Init::CloseTxtFile( pOutf );
	delete m_pvectArticleList;
	utests_Init::CloseTxtFile( pDataf );

	if( true == bIsFailed )
	{
		CPPUNIT_FAIL( "Incorrect references found \n" );
	}
}

