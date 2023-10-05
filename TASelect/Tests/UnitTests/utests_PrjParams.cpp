#include "stdafx.h"

#include "utests_base.h"
#include "utests_PrjParams.h"
#include "taselect.h"
#include "DataStruct.h"
#include "Select.h"
#include "Excel_Tools.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_PrjParams, CPPUNIT_TEST_CATEGORYNAME_TECHNICALPARAMETERS );

void utests_PrjParams::setUp()
{
	utests_base::setUp();
	utests_Init::InitDefaultDbValues();

// 	CString OutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_TECHNICALPARAMETERS ), CString( __FILE__ ), _T( "" ) );
// 	if( false == OutPath.IsEmpty() )
// 		DeleteFilesAndFolder( OutPath, _T("*.*") );
	m_strList.RemoveAll();
	m_strList.Add( L"BvBlw65TypeID" );
	m_strList.Add( L"BvBlw65ConnectID" );
	m_strList.Add( L"BvBlw65VersionID" );
	m_strList.Add( L"BvBlw65PNID" );
	m_strList.Add( L"BvAbv50TypeID" );
	m_strList.Add( L"BvAbv50ConnectID" );
	m_strList.Add( L"BvAbv50VersionID" );
	m_strList.Add( L"BvAbv50PNID" );
	m_strList.Add( L"DpCBlw65TypeID" );
	m_strList.Add( L"DpCBlw65ConnectID" );
	m_strList.Add( L"DpCBlw65PNID" );
	m_strList.Add( L"DpCAbv50TypeID" );
	m_strList.Add( L"DpCAbv50ConnectID" );
	m_strList.Add( L"DpCAbv50PNID" );
	m_strList.Add( L"PipeCircSerieID" );
	m_strList.Add( L"PipeDistSupplySerieID" );
	m_strList.Add( L"CvBlw65TypeID" );
	m_strList.Add( L"CvBlw65ConnectID" );
	m_strList.Add( L"CvBlw65VersionID" );
	m_strList.Add( L"CvBlw65PNID" );
	m_strList.Add( L"CvAbv50TypeID" );
	m_strList.Add( L"CvAbv50ConnectID" );
	m_strList.Add( L"CvAbv50VersionID" );
	m_strList.Add( L"CvAbv50PNID" );
	m_strList.Add( L"BvBlw65FamilyID" );
	m_strList.Add( L"BvBlw65BdyMatID" );
	m_strList.Add( L"BvAbv50FamilyID" );
	m_strList.Add( L"BvAbv50BdyMatID" );
	m_strList.Add( L"DpCBlw65FamilyID" );
	m_strList.Add( L"DpCBlw65BdyMatID" );
	m_strList.Add( L"DpCBlw65VersionID" );
	m_strList.Add( L"DpCAbv50FamilyID" );
	m_strList.Add( L"DpCAbv50BdyMatID" );
	m_strList.Add( L"DpCAbv50VersionID" );
	m_strList.Add( L"BvBypBlw65FamilyID" );
	m_strList.Add( L"BvBypBlw65BdyMatID" );
	m_strList.Add( L"BvBypAbv50FamilyID" );
	m_strList.Add( L"BvBypAbv50BdyMatID" );
	m_strList.Add( L"CvBlw65FamilyID" );
	m_strList.Add( L"CvBlw65BdyMatID" );
	m_strList.Add( L"CvAbv50FamilyID" );
	m_strList.Add( L"CvAbv50BdyMatID" );
	m_strList.Add( L"PICvBlw65TypeID" );
	m_strList.Add( L"PICvBlw65FamilyID" );
	m_strList.Add( L"PICvBlw65BdyMatID" );
	m_strList.Add( L"PICvBlw65ConnectID" );
	m_strList.Add( L"PICvBlw65VersionID" );
	m_strList.Add( L"PICvBlw65PNID" );
	m_strList.Add( L"PICvAbv50TypeID" );
	m_strList.Add( L"PICvAbv50FamilyID" );
	m_strList.Add( L"PICvAbv50BdyMatID" );
	m_strList.Add( L"PICvAbv50ConnectID" );
	m_strList.Add( L"PICvAbv50VersionID" );
	m_strList.Add( L"PICvAbv50PNID" );
	m_strList.Add( L"PipeDistReturnSerieID" );
	m_strList.Add( L"BCvBlw65TypeID" );
	m_strList.Add( L"BCvBlw65FamilyID" );
	m_strList.Add( L"BCvBlw65BdyMatID" );
	m_strList.Add( L"BCvBlw65ConnectID" );
	m_strList.Add( L"BCvBlw65VersionID" );
	m_strList.Add( L"BCvBlw65PNID" );
	m_strList.Add( L"BCvAbv50TypeID" );
	m_strList.Add( L"BCvAbv50FamilyID" );
	m_strList.Add( L"BCvAbv50BdyMatID" );
	m_strList.Add( L"BCvAbv50ConnectID" );
	m_strList.Add( L"BCvAbv50VersionID" );
	m_strList.Add( L"BCvAbv50PNID" );
	m_strList.Add( L"ActuatorPowerSupplyID" );
	m_strList.Add( L"ActuatorInputSignalID" );
	m_strList.Add( L"SeparatorTypeID" );
	m_strList.Add( L"SeparatorFamilyID" );
	m_strList.Add( L"SeparatorConnectID" );
	m_strList.Add( L"SeparatorVersionID" );
	m_strList.Add( L"DpCBCValveBlw65TypeID" );
	m_strList.Add( L"DpCBCValveBlw65FamilyID" );
	m_strList.Add( L"DpCBCValveBlw65BdyMatID" );
	m_strList.Add( L"DpCBCValveBlw65ConnectID" );
	m_strList.Add( L"DpCBCValveBlw65VersionID" );
	m_strList.Add( L"DpCBCValveBlw65PNID" );
	m_strList.Add( L"DpCBCValveAbv50TypeID" );
	m_strList.Add( L"DpCBCValveAbv50FamilyID" );
	m_strList.Add( L"DpCBCValveAbv50BdyMatID" );
	m_strList.Add( L"DpCBCValveAbv50ConnectID" );
	m_strList.Add( L"DpCBCValveAbv50VersionID" );
	m_strList.Add( L"DpCBCValveAbv50PNID" );
	m_strList.Add( L"SmartControlValveBlw65TypeID" );
	m_strList.Add( L"SmartControlValveBlw65FamilyID" );
	m_strList.Add( L"SmartControlValveBlw65BdyMatID" );
	m_strList.Add( L"SmartControlValveBlw65ConnectID" );
	m_strList.Add( L"SmartControlValveBlw65VersionID" );
	m_strList.Add( L"SmartControlValveBlw65PNID" );
	m_strList.Add( L"SmartControlValveAbv50TypeID" );
	m_strList.Add( L"SmartControlValveAbv50FamilyID" );
	m_strList.Add( L"SmartControlValveAbv50BdyMatID" );
	m_strList.Add( L"SmartControlValveAbv50ConnectID" );
	m_strList.Add( L"SmartControlValveAbv50VersionID" );
	m_strList.Add( L"SmartControlValveAbv50PNID" );
	m_strList.Add( L"SmartDpCBlw65TypeID" );
	m_strList.Add( L"SmartDpCBlw65FamilyID" );
	m_strList.Add( L"SmartDpCBlw65BdyMatID" );
	m_strList.Add( L"SmartDpCBlw65ConnectID" );
	m_strList.Add( L"SmartDpCBlw65VersionID" );
	m_strList.Add( L"SmartDpCBlw65PNID" );
	m_strList.Add( L"SmartDpCAbv50TypeID" );
	m_strList.Add( L"SmartDpCAbv50FamilyID" );
	m_strList.Add( L"SmartDpCAbv50BdyMatID" );
	m_strList.Add( L"SmartDpCAbv50ConnectID" );
	m_strList.Add( L"SmartDpCAbv50VersionID" );
	m_strList.Add( L"SmartDpCAbv50PNID" );
	m_strList.Add( L"PipeByPassSerieID" );

	CPPUNIT_ASSERT_EQUAL( (int)m_strList.GetCount(), (int)CPrjParams::LASTID );
}

void utests_PrjParams::tearDown()
{
	utests_base::tearDown();
	utests_Init::RestoreDefaultDbValues();
}

void utests_PrjParams::TestClearOutputFolder()
{
}

void utests_PrjParams::TestPrjParams()
{
	CDS_ProjectParams *pProjectParam = TASApp.GetpTADS()->GetpProjectParams();
	CPPUNIT_ASSERT( NULL != pProjectParam );

	//CPrjParams *pPrjParams = pProjectParam->GetpHmCalcParams();
	CPrjParams *pPrjParams = new CPrjParams( NULL );
	pPrjParams->ResetPrjParams( true );
	CString strFileName = _T("PrjParams.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_TECHNICALPARAMETERS ), CString( __FILE__ ), strFileName );

	// Code to extract data
	int iMaxLenght = 25;
	for( int i = 0; i < CPrjParams::LASTID; i++ )
	{
		if( m_strList[i].GetLength()>iMaxLenght )
			iMaxLenght = m_strList[i].GetLength() + 1;
	}

	for( int i = 0; i < CPrjParams::LASTID; i++ )
	{
		CString str;
		CString strID = m_strList[i];
		CString strSpace = L"                         ";
		strID = strID + strSpace.Left( iMaxLenght - strID.GetLength() );
		str.Format( _T("%s: %s\n"), strID, pPrjParams->GetPrjParamID( (CPrjParams::_PrjParamsID)i ) );
		pOutf->WriteTxtLine( ( LPCTSTR )str );
	}
	utests_Init::CloseTxtFile( pOutf );

	//Compare with reference file
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_TECHNICALPARAMETERS ), CString( __FILE__ ), strFileName );

	delete pPrjParams;
}

void utests_PrjParams::TestReadWritePrjParams()
{
	// Test Write
	CPrjParams *pPrjParams = new CPrjParams( NULL );
	pPrjParams->ResetPrjParams( true );
	CString strFileName = _T("PrjParamsReadWrite.bin");
	std::fstream *pOutf = utests_Init::OpenBinFile( CString( CPPUNIT_TEST_CATEGORYNAME_TECHNICALPARAMETERS ), CString( __FILE__ ), strFileName, true );
	CPPUNIT_ASSERT( NULL != pOutf );
	pPrjParams->Write( (std::ofstream&)(*pOutf) );
	pOutf->close();
	delete pOutf;

	// Test Read
	CPrjParams *pPrjParamsSaved = new CPrjParams( NULL );
	pPrjParamsSaved->ResetPrjParams( true );
	std::fstream *pInpf = utests_Init::OpenBinFile( CString( CPPUNIT_TEST_CATEGORYNAME_TECHNICALPARAMETERS ), CString( __FILE__ ), strFileName, false );
	CPPUNIT_ASSERT( NULL != pInpf );
	pPrjParamsSaved->Read( ( std::ifstream& )( *pInpf ) );
	pInpf->close();
	delete pInpf;

	for( int i = 0; i < CPrjParams::LASTID; i++ )
	{
		const std::string str = Excel_Tools::ws2s((LPCTSTR)m_strList[i]);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(str , pPrjParams->GetPrjParamID( ( CPrjParams::_PrjParamsID ) i ), pPrjParamsSaved->GetPrjParamID( ( CPrjParams::_PrjParamsID ) i ) );
	}

	delete pPrjParams;
	delete pPrjParamsSaved;
}

