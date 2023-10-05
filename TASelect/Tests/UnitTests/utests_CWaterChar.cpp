#include "stdafx.h"

#include "utests_base.h"
#include "utests_CWaterChar.h"
#include "DataBase.h"
#include "TASelect.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_CWaterChar, CPPUNIT_TEST_CATEGORYNAME_DATABASE );

void utests_CWaterChar::watercharConstructorTest()
{
	CWaterChar *pWC = new CWaterChar();
	CPPUNIT_ASSERT( pWC );

	CPPUNIT_ASSERT_DOUBLES_EQUAL( 20.0, pWC->GetTemp(), DOUBLE_DELTA_TOLERANCE );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, pWC->GetPcWeight(), DOUBLE_DELTA_TOLERANCE );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 998.2, pWC->GetDens(), DOUBLE_DELTA_TOLERANCE );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, pWC->GetTfreez(), DOUBLE_DELTA_TOLERANCE );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.004e-6, pWC->GetKinVisc(), DOUBLE_DELTA_TOLERANCE );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 4182.0, pWC->GetSpecifHeat(), DOUBLE_DELTA_TOLERANCE );
	CPPUNIT_ASSERT_EQUAL( false, pWC->GetCheckSpecifHeat() );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 150.0, pWC->GetpAdditChar()->GetMaxT( pWC->GetPcWeight() ), DOUBLE_DELTA_TOLERANCE );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 130807.65, pWC->GetVaporPressure( 125.0 ), DOUBLE_DELTA_TOLERANCE );

	CPPUNIT_ASSERT_NO_THROW( delete pWC );
}

void utests_CWaterChar::DataSheetExpensionWater()
{
	CWaterChar *pWC = new CWaterChar();
	CPPUNIT_ASSERT( pWC );

	CString strFileName = _T("DataSheetExpensionWater.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
	// Code to extract data
	double dMinTemp = 0;
	double dTestTemp[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 105, 110 };

	for( int d = 0; d < sizeof( dTestTemp ) / sizeof( double ); d++ )
	{
		CString str;
		str.Format( _T("%f; %f; %f\n"), dMinTemp, dTestTemp[d], pWC->GetExpansionCoeff( dMinTemp, dTestTemp[d] ) );
		str.Replace( _T("," ), _T( ".") );
		pOutf->WriteTxtLine( ( LPCTSTR )str );
	}

	CPPUNIT_ASSERT_NO_THROW( delete pWC );

	utests_Init::CloseTxtFile( pOutf );

	//Compare with reference file
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
}

void utests_CWaterChar::DataSheetExpensionMEG()
{
	CWaterChar *pWC = new CWaterChar();
	CPPUNIT_ASSERT( pWC );

	pWC->SetAdditFamID( _T("GLYCOL_ADDIT") );
	pWC->SetAdditID( _T("ETHYL_GLYC") );

	CString strFileName = _T("DataSheetExpensionMEG.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
	// Code to extract data
	double dDilution[] = { 30, 40, 50 };
	double dTestTemp[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 105, 110 };

	for( int e = 0; e < sizeof( dDilution ) / sizeof( double ); e++ )
	{
		for( int d = 0; d < sizeof( dTestTemp ) / sizeof( double ); d++ )
		{
			pWC->UpdateFluidData(dTestTemp[d], dDilution[e]);
			double tfreeze = pWC->GetTfreez();

			CString str;
			str.Format( _T("%f pct; %f; %f; %f\n"), dDilution[e], tfreeze, dTestTemp[d], pWC->GetExpansionCoeff( tfreeze, dTestTemp[d] ) );
			str.Replace( _T("," ), _T( ".") );
			pOutf->WriteTxtLine( ( LPCTSTR )str );
		}
	}

	CPPUNIT_ASSERT_NO_THROW( delete pWC );

	utests_Init::CloseTxtFile( pOutf );

	//Compare with reference file
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
}

void utests_CWaterChar::DataSheetExpensionMPG()
{
	CWaterChar *pWC = new CWaterChar();
	CPPUNIT_ASSERT( pWC );

	pWC->SetAdditFamID( _T("GLYCOL_ADDIT") );
	pWC->SetAdditID( _T("PROPYL_GLYC") );

	CString strFileName = _T("DataSheetExpensionMPG.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
	// Code to extract data
	double dDilution[] = { 30, 40, 50 };
	double dTestTemp[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 105, 110 };

	for( int e = 0; e < sizeof( dDilution ) / sizeof( double ); e++ )
	{
		for( int d = 0; d < sizeof( dTestTemp ) / sizeof( double ); d++ )
		{
			pWC->UpdateFluidData(dTestTemp[d], dDilution[e]);
			double tfreeze = pWC->GetTfreez();

			CString str;
			str.Format( _T("%f pct; %f; %f; %f\n"), dDilution[e], tfreeze, dTestTemp[d], pWC->GetExpansionCoeff( tfreeze, dTestTemp[d] ) );
			str.Replace( _T("," ), _T( ".") );
			pOutf->WriteTxtLine( ( LPCTSTR )str );
		}
	}

	CPPUNIT_ASSERT_NO_THROW( delete pWC );

	utests_Init::CloseTxtFile( pOutf );

	//Compare with reference file
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
}

void utests_CWaterChar::DataSheetVapourWater()
{
	CWaterChar *pWC = new CWaterChar();
	CPPUNIT_ASSERT( pWC );

	CString strFileName = _T("DataSheetVapourWater.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
	// Code to extract data
	double dMinTemp = 0;
	double dTestTemp[] = { 105, 110 };

	for( int d = 0; d < sizeof( dTestTemp ) / sizeof( double ); d++ )
	{
		CString str;
		str.Format( _T("%f; %f\n"), dTestTemp[d], pWC->GetVaporPressure( dTestTemp[d] ) );
		str.Replace( _T("," ), _T( ".") );
		pOutf->WriteTxtLine( ( LPCTSTR )str );
	}

	CPPUNIT_ASSERT_NO_THROW( delete pWC );

	utests_Init::CloseTxtFile( pOutf );

	//Compare with reference file
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
}

void utests_CWaterChar::DataSheetVapourMEG()
{
	CWaterChar *pWC = new CWaterChar();
	CPPUNIT_ASSERT( pWC );

	pWC->SetAdditFamID( _T("GLYCOL_ADDIT") );
	pWC->SetAdditID( _T("ETHYL_GLYC") );

	CString strFileName = _T("DataSheetVapourMEG.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
	// Code to extract data
	double dDilution[] = { 30, 40, 50 };
	double dTestTemp[] = { 105, 110 };

	for( int e = 0; e < sizeof( dDilution ) / sizeof( double ); e++ )
	{
		for( int d = 0; d < sizeof( dTestTemp ) / sizeof( double ); d++ )
		{
			pWC->UpdateFluidData(dTestTemp[d], dDilution[e]);

			CString str;
			str.Format( _T("%f pct; %f; %f\n"), dDilution[e], dTestTemp[d], pWC->GetVaporPressure( dTestTemp[d] ) );
			str.Replace( _T("," ), _T( ".") );
			pOutf->WriteTxtLine( ( LPCTSTR )str );
		}
	}

	CPPUNIT_ASSERT_NO_THROW( delete pWC );

	utests_Init::CloseTxtFile( pOutf );

	//Compare with reference file
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
}

void utests_CWaterChar::DataSheetVapourMPG()
{
	CWaterChar *pWC = new CWaterChar();
	CPPUNIT_ASSERT( pWC );

	pWC->SetAdditFamID( _T("GLYCOL_ADDIT") );
	pWC->SetAdditID( _T("PROPYL_GLYC") );

	CString strFileName = _T("DataSheetVapourMPG.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
	// Code to extract data
	double dDilution[] = { 30, 40, 50 };
	double dTestTemp[] = { 105, 110 };

	for( int e = 0; e < sizeof( dDilution ) / sizeof( double ); e++ )
	{
		for( int d = 0; d < sizeof( dTestTemp ) / sizeof( double ); d++ )
		{
			pWC->UpdateFluidData(dTestTemp[d], dDilution[e]);

			CString str;
			str.Format( _T("%f pct; %f; %f\n"), dDilution[e], dTestTemp[d], pWC->GetVaporPressure( dTestTemp[d] ) );
			str.Replace( _T("," ), _T( ".") );
			pOutf->WriteTxtLine( ( LPCTSTR )str );
		}
	}

	CPPUNIT_ASSERT_NO_THROW( delete pWC );

	utests_Init::CloseTxtFile( pOutf );

	//Compare with reference file
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
}

void utests_CWaterChar::DataSheetDensityWater()
{
	CWaterChar *pWC = new CWaterChar();
	CPPUNIT_ASSERT( pWC );

	CString strFileName = _T("DataSheetDensityWater.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
	// Code to extract data
	double dMinTemp = 0;
	double dTestTemp[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 105, 110 };

	for( int d = 0; d < sizeof( dTestTemp ) / sizeof( double ); d++ )
	{
		pWC->UpdateFluidData( dTestTemp[d] );

		CString str;
		str.Format( _T("%f; %f\n"), dTestTemp[d], pWC->GetDens() );
		str.Replace( _T("," ), _T( ".") );
		pOutf->WriteTxtLine( ( LPCTSTR )str );
	}

	CPPUNIT_ASSERT_NO_THROW( delete pWC );

	utests_Init::CloseTxtFile( pOutf );

	//Compare with reference file
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
}

void utests_CWaterChar::DataSheetDensityMEG()
{
	CWaterChar *pWC = new CWaterChar();
	CPPUNIT_ASSERT( pWC );

	pWC->SetAdditFamID( _T("GLYCOL_ADDIT") );
	pWC->SetAdditID( _T("ETHYL_GLYC") );

	CString strFileName = _T("DataSheetDensityMEG.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
	// Code to extract data
	double dDilution[] = { 30, 40, 50 };
	double dTestTemp[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 105, 110 };

	for( int e = 0; e < sizeof( dDilution ) / sizeof( double ); e++ )
	{
		pWC->UpdateFluidData( 0.0, dDilution[e]);
		dTestTemp[0] = pWC->GetTfreez();

		for( int d = 0; d < sizeof( dTestTemp ) / sizeof( double ); d++ )
		{
			pWC->UpdateFluidData( dTestTemp[d] );

			CString str;
			str.Format( _T("%f pct; %f; %f\n"), dDilution[e], dTestTemp[d], pWC->GetDens() );
			str.Replace( _T("," ), _T( ".") );
			pOutf->WriteTxtLine( ( LPCTSTR )str );
		}
	}

	CPPUNIT_ASSERT_NO_THROW( delete pWC );

	utests_Init::CloseTxtFile( pOutf );

	//Compare with reference file
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
}

void utests_CWaterChar::DataSheetDensityMPG()
{
	CWaterChar *pWC = new CWaterChar();
	CPPUNIT_ASSERT( pWC );

	pWC->SetAdditFamID( _T("GLYCOL_ADDIT") );
	pWC->SetAdditID( _T("PROPYL_GLYC") );

	CString strFileName = _T("DataSheetDensityMPG.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
	// Code to extract data
	double dDilution[] = { 30, 40, 50 };
	double dTestTemp[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 105, 110 };

	for( int e = 0; e < sizeof( dDilution ) / sizeof( double ); e++ )
	{
		pWC->UpdateFluidData( 0.0, dDilution[e] );
		dTestTemp[0] = pWC->GetTfreez();

		for( int d = 0; d < sizeof( dTestTemp ) / sizeof( double ); d++ )
		{
			pWC->UpdateFluidData( dTestTemp[d] );

			CString str;
			str.Format( _T("%f pct; %f; %f\n"), dDilution[e], dTestTemp[d], pWC->GetDens() );
			str.Replace( _T("," ), _T( ".") );
			pOutf->WriteTxtLine( ( LPCTSTR )str );
		}
	}

	CPPUNIT_ASSERT_NO_THROW( delete pWC );

	utests_Init::CloseTxtFile( pOutf );

	//Compare with reference file
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE ), CString( __FILE__ ), strFileName );
}
