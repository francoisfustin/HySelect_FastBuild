#include "stdafx.h"

#include "utests_base.h"
#include "utests_CDBPageField.h"
#include "DataBObj.h"
#include "TASelect.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_CDBPageField, CPPUNIT_TEST_CATEGORYNAME_DATABASE );

void utests_CDBPageField::WriteCompletePageField()
{
	IDPTR testIDPtr;

	try
	{
		TASApp.GetpUserDB()->CreateObject( testIDPtr, CLASS( CDB_PageField ), _T("UNITTEST") );
	}
	catch( CHySelectException &clHySelectException )
	{
		// To avoid compilation warning.
		clHySelectException.ClearMessages();
		CPPUNIT_ASSERT( 1 );
	}

	CDB_PageField *testpf = static_cast<CDB_PageField *>( testIDPtr.MP );
	testpf->SetPredefStyle( PAGESTYLECOMPLETE );

	CString lastDbPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE), CString( __FILE__ ), false ) + CString( "lastPageField.db" );
	std::ofstream outpf( ( LPCTSTR )lastDbPath, std::ofstream::out | std::ofstream::binary );
	CPPUNIT_ASSERT( !( outpf.rdstate() & std::ofstream::failbit ) );
	testpf->PublicWrite( outpf );
	outpf.close();

	CPPUNIT_ASSERT( TASApp.GetpUserDB()->DeleteObject( testIDPtr ) );
}

void utests_CDBPageField::ConstructorTest()
{
	IDPTR testIDPtr;
	
	try
	{
		TASApp.GetpUserDB()->CreateObject( testIDPtr, CLASS( CDB_PageField ), _T("UNITTEST") );
	}
	catch( CHySelectException &clHySelectException )
	{
		// To avoid compilation warning.
		clHySelectException.ClearMessages();
		CPPUNIT_ASSERT( 1 );
	}

	CDB_PageField *testpf = static_cast<CDB_PageField *>( testIDPtr.MP );
	testpf->SetPredefStyle( 0 );

	for( int i = 0; i < PAGEFIELD_SIZE; ++i )
	{
		CPPUNIT_ASSERT( testpf->GetField( static_cast<PageField_enum>( i ) ) == false );
	}

	CPPUNIT_ASSERT( TASApp.GetpUserDB()->DeleteObject( testIDPtr ) );

}

void utests_CDBPageField::ReadyToTest()
{
	IDPTR idtest = TASApp.GetpUserDB()->Get( _T("UNITTEST") );
	CPPUNIT_ASSERT_MESSAGE( "User Db not clean", idtest == _NULL_IDPTR );
}

void utests_CDBPageField::Version13Test()
{
	ReadyToTest();

	IDPTR testIDPtr;
	TASApp.GetpUserDB()->CreateObject( testIDPtr, CLASS( CDB_PageField ), _T("UNITTEST") );
	CDB_PageField *testpf = static_cast<CDB_PageField *>( testIDPtr.MP );
	testpf->SetPredefStyle( 0 );


	CString v13DbPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE), CString( __FILE__ ), true ) + CString( "version13.db" );
	std::ifstream inpf( ( LPCTSTR )v13DbPath, std::ifstream::in | std::ifstream::binary );
	CPPUNIT_ASSERT( !( inpf.rdstate() & std::ifstream::failbit ) );
	CPPUNIT_ASSERT( testpf->PublicRead( inpf ) );
	inpf.close();

	CPPUNIT_ASSERT( testpf->GetField( epfNONE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfFIRSTREF ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSECONDREF ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFOPRODUCT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFOPRODUCTNAME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFOPRODUCTDPP ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFOPRODUCTSIZE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFOPRODUCTCONNECTION ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFOPRODUCTVERSION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFOPRODUCTPN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTTECHINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTTECHINFOFLOW ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTTECHINFODP ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCTNAME ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCTCONNECTION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCTNOMINALVOLUME ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCTMAXPRESSURE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCTTEMPRANGE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCTWEIGHT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCTMAXWEIGHT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfTECHBOXINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfTECHBOXINFOPRODUCT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfTECHBOXINFOPRODUCTNAME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfTECHBOXINFOPRODUCTPS ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfTECHBOXINFOPRODUCTPOWER ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfTECHBOXINFOPRODUCTSUPPLYVOLTAGE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERMAKEUPINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERMAKEUPINFOPRODUCT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERMAKEUPINFOPRODUCTNAME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERMAKEUPINFOPRODUCTFUNCTIONS ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERMAKEUPINFOPRODUCTCAPACITY ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERMAKEUPINFOPRODUCTHEIGHT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERMAKEUPINFOPRODUCTWEIGHT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfBVINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfBVINFOPRODUCT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfBVINFOPRODUCTNAME ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfBVINFOPRODUCTSIZE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfBVINFOPRODUCTCONNECTION ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfBVINFOPRODUCTVERSION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfBVINFOPRODUCTPN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfBVTECHINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfBVTECHINFOFLOW ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfBVTECHINFODP ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfBVTECHINFOSETTING ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFOPRODUCT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFOPRODUCTNAME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFOPRODUCTSIZE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFOPRODUCTCONNECTION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFOPRODUCTVERSION ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFOPRODUCTDPLRANGE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFOPRODUCTPN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFOFLOW ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFO2NDDP ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFOHMIN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFOCONNECTSCHEME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFOAUTHORITY ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFOSETTING ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7INFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7INFOPRODUCT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7INFOPRODUCTNAME ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7INFOPRODUCTSIZE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7INFOPRODUCTCONNECTION ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7INFOPRODUCTVERSION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7INFOPRODUCTPN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFOFLOW ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFODP ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFOSETTING ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFODPTOT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFOCONNECTSCHEME ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFODPMIN ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFOFLOWRANGE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7RADINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7RADINFOREQPOW ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7RADINFOINSTPOW ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7RADINFOTROOM ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7RADINFOTSUPPLY ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7RADINFODELTAT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDSINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDSINFOPRODUCT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDSINFOPRODUCTNAME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDSINFOPRODUCTSIZE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDSINFOPRODUCTCONNECTION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDSINFOPRODUCTVERSION ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDSINFOPRODUCTPN ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfPDINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfPDINFOPRODUCT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfPDINFOPRODUCTNAME ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfPDINFOPRODUCTSIZE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfARTICLE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfPIPEINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfPIPENAME ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfPIPESIZE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfPIPEWATERU ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfPIPEDP ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfQUANTITY ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSALEINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSALEUNITPRICE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSALETOTALPRICE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfREMARK ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFODPMIN ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDSTECHINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfPDTECHINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfNU1 ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfNU2 ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFOPRODUCT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFOPRODUCTNAME ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFOPRODUCTKV ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFOPRODUCTBDYMATERIAL ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFOPRODUCTCONNECTION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFOPRODUCTVERSION ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFOPRODUCTPN ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfCVTECHINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfCVTECHINFOFLOW ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfCVTECHINFODP ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfCVTECHINFOSETTING ) == false );

	CPPUNIT_ASSERT( TASApp.GetpUserDB()->DeleteObject( testIDPtr ) );
}

void utests_CDBPageField::Version14Test()
{
	ReadyToTest();

	IDPTR testIDPtr;
	TASApp.GetpUserDB()->CreateObject( testIDPtr, CLASS( CDB_PageField ), _T("UNITTEST") );
	CDB_PageField *testpf = static_cast<CDB_PageField *>( testIDPtr.MP );

	testpf->SetPredefStyle( 0 );


	CString v13DbPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_DATABASE), CString( __FILE__ ), true ) + CString( "version14.db" );
	std::ifstream inpf( ( LPCTSTR )v13DbPath, std::ifstream::in | std::ifstream::binary );
	CPPUNIT_ASSERT( !( inpf.rdstate() & std::ifstream::failbit ) );
	CPPUNIT_ASSERT( testpf->PublicRead( inpf ) );
	inpf.close();

	CPPUNIT_ASSERT( testpf->GetField( epfNONE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfFIRSTREF ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSECONDREF ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFOPRODUCT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFOPRODUCTNAME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFOPRODUCTDPP ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFOPRODUCTSIZE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFOPRODUCTCONNECTION ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFOPRODUCTVERSION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTINFOPRODUCTPN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTTECHINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTTECHINFOFLOW ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSEPARATORAIRVENTTECHINFODP ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCTNAME ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCTCONNECTION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCTNOMINALVOLUME ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCTMAXPRESSURE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCTTEMPRANGE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCTWEIGHT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSTATICOINFOPRODUCTMAXWEIGHT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfTECHBOXINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfTECHBOXINFOPRODUCT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfTECHBOXINFOPRODUCTNAME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfTECHBOXINFOPRODUCTPS ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfTECHBOXINFOPRODUCTPOWER ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfTECHBOXINFOPRODUCTSUPPLYVOLTAGE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERMAKEUPINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERMAKEUPINFOPRODUCT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERMAKEUPINFOPRODUCTNAME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERMAKEUPINFOPRODUCTFUNCTIONS ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERMAKEUPINFOPRODUCTCAPACITY ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERMAKEUPINFOPRODUCTHEIGHT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfWATERMAKEUPINFOPRODUCTWEIGHT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfBVINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfBVINFOPRODUCT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfBVINFOPRODUCTNAME ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfBVINFOPRODUCTSIZE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfBVINFOPRODUCTCONNECTION ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfBVINFOPRODUCTVERSION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfBVINFOPRODUCTPN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfBVTECHINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfBVTECHINFOFLOW ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfBVTECHINFODP ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfBVTECHINFOSETTING ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFOPRODUCT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFOPRODUCTNAME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFOPRODUCTSIZE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFOPRODUCTCONNECTION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFOPRODUCTVERSION ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFOPRODUCTDPLRANGE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCINFOPRODUCTPN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFOFLOW ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFO2NDDP ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFOHMIN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFOCONNECTSCHEME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFOAUTHORITY ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFOSETTING ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCBCVINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCBCVINFOPRODUCT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCBCVINFOPRODUCTNAME ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCBCVINFOPRODUCTSIZE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCBCVINFOPRODUCTCONNECTION ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCBCVINFOPRODUCTVERSION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCBCVINFOPRODUCTDPLRANGE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCBCVINFOPRODUCTPN ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCBCVTECHINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCBCVTECHINFOFLOW ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCBCVTECHINFODPMIN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCBCVTECHINFOSETTING ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCBCVTECHINFOHMIN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7INFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7INFOPRODUCT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7INFOPRODUCTNAME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7INFOPRODUCTSIZE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7INFOPRODUCTCONNECTION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7INFOPRODUCTVERSION ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7INFOPRODUCTPN ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFOFLOW ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFODP ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFOSETTING ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFODPTOT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFOCONNECTSCHEME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFODPMIN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7TECHINFOFLOWRANGE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7RADINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7RADINFOREQPOW ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7RADINFOINSTPOW ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7RADINFOTROOM ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7RADINFOTSUPPLY ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfRD7RADINFODELTAT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDSINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDSINFOPRODUCT ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDSINFOPRODUCTNAME ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDSINFOPRODUCTSIZE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDSINFOPRODUCTCONNECTION ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDSINFOPRODUCTVERSION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDSINFOPRODUCTPN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfPDINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfPDINFOPRODUCT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfPDINFOPRODUCTNAME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfPDINFOPRODUCTSIZE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfARTICLE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfPIPEINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfPIPENAME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfPIPESIZE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfPIPEWATERU ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfPIPEDP ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfQUANTITY ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSALEINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfSALEUNITPRICE ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfSALETOTALPRICE ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfREMARK ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfDPCTECHINFODPMIN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfDSTECHINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfPDTECHINFO ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfNU1 ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfNU2 ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFOPRODUCT ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFOPRODUCTNAME ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFOPRODUCTKV ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFOPRODUCTBDYMATERIAL ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFOPRODUCTCONNECTION ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFOPRODUCTVERSION ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfCVINFOPRODUCTPN ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfCVTECHINFO ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfCVTECHINFOFLOW ) == true );
	CPPUNIT_ASSERT( testpf->GetField( epfCVTECHINFODP ) == false );
	CPPUNIT_ASSERT( testpf->GetField( epfCVTECHINFOSETTING ) == true );

	CPPUNIT_ASSERT( TASApp.GetpUserDB()->DeleteObject( testIDPtr ) );
}

