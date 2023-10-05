#include "stdafx.h"

#include "utests_base.h"
#include "utests_ExpansionPipeSize.h"
#include "taselect.h"
#include "MainFrm.h"
#include "DataStruct.h"
#include "SelectPM.h"
#include "DlgLeftTabSelManager.h"
#include "TUProductSelectionHelper.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_ExpansionPipeSize, CPPUNIT_TEST_CATEGORYNAME_PRESSURISATION );

void utests_ExpansionPipeSize::setUp()
{
	utests_base::setUp();
	utests_Init::InitDefaultDbValues();
}

void utests_ExpansionPipeSize::tearDown()
{
	utests_base::tearDown();
	utests_Init::RestoreDefaultDbValues();
}

void utests_ExpansionPipeSize::TestExpansionPipeSize()
{
	CPMInputUser *pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser(ProductSelectionMode::ProductSelectionMode_Individual, ProjectType::Heating );
	CPPUNIT_ASSERT( NULL != pclPMInputUser );

	CString strFileName = _T("ExpansionPipeSize.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_PRESSURISATION ), CString( __FILE__ ), strFileName );

	pOutf->WriteTxtLine( _T("Type|name|Application type|Norm|Pipe length|HST|Power|Pipe ID\n" ) );

	// Test expansion pipe for expansion vessel.
	CTable *pclVesselTable = dynamic_cast<CTable *>( TASApp.GetpTADB()->Get( _T("VSSL_TAB") ).MP );
	CPPUNIT_ASSERT( NULL != pclVesselTable );

	// Take the first vessel.
	CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel *>( pclVesselTable->GetFirst().MP );
	CPPUNIT_ASSERT( NULL != pclVessel );

	// Test first for heating and EN 12828 norm.
	pclPMInputUser->SetApplicationType( ProjectType::Heating );
	pclPMInputUser->SetNormID( _T("PM_NORM_EN12828") );
	
	CString strHeader; 
	strHeader.Format( _T("Expansion vessel|%s|Heating|EN 12828|10|10|"), pclVessel->GetName() );

	CString str;
	CString strLine;
	double dInstalledPower = 0.0;
	IDPTR ExpansionPipeIDPtr = _NULL_IDPTR;

	do 
	{
		pclPMInputUser->SetInstalledPower( dInstalledPower );
		ExpansionPipeIDPtr = pclPMInputUser->GetExpansionPipeSizeIDPtr( pclVessel, 0.0 );

		strLine = strHeader;
		str.Format( _T("%f|%s\n"), dInstalledPower, ExpansionPipeIDPtr.ID );
		strLine += str;
		pOutf->WriteTxtLine( strLine );

		dInstalledPower += 0.5e6;

	}while( dInstalledPower < 16.0e6 );

	// Test for heating and SWKI HE301-01 norm.
	pclPMInputUser->SetNormID( _T("PM_NORM_SWKI_HE301_01") );
	strHeader.Format( _T("Expansion vessel|%s|Heating|SWKI HE301-01|10|10|"), pclVessel->GetName() );

	dInstalledPower = 0.0;

	do 
	{
		pclPMInputUser->SetInstalledPower( dInstalledPower );
		ExpansionPipeIDPtr = pclPMInputUser->GetExpansionPipeSizeIDPtr( pclVessel, 0.0 );

		strLine = strHeader;
		str.Format( _T("%f|%s\n"), dInstalledPower, ExpansionPipeIDPtr.ID );
		strLine += str;
		pOutf->WriteTxtLine( strLine );

		dInstalledPower += 0.15e6;

	}while( dInstalledPower < 9.6e6 );

	// Test for cooling.
	pclPMInputUser->SetApplicationType( ProjectType::Cooling );
	strHeader.Format( _T("Expansion vessel|%s|Cooling||10|10|"), pclVessel->GetName() );

	dInstalledPower = 0.0;

	do 
	{
		pclPMInputUser->SetInstalledPower( dInstalledPower );
		ExpansionPipeIDPtr = pclPMInputUser->GetExpansionPipeSizeIDPtr( pclVessel, 0.0 );

		strLine = strHeader;
		str.Format( _T("%f|%s\n"), dInstalledPower, ExpansionPipeIDPtr.ID );
		strLine += str;
		pOutf->WriteTxtLine( strLine );

		dInstalledPower += 0.8e6;

	}while( dInstalledPower < 25.2e6 );

	// Test now all Transfero TV Connect.
	CTUProdSelFileHelper clTUProdSelFileHelper;
	CTable *pclTecBoxTable = (CTable *)( TASApp.GetpTADB()->Get( _T("TECBOX_TAB") ).MP );
	CPPUNIT_ASSERT( NULL != pclTecBoxTable );

	// Sort Transfero.
	// Key= 4/6/8/10/14 + 1/2 pumps + without H/with H.
	//      4 bits + 1 bits + 1bits
	std::map<UCHAR, CDB_TecBox *> mapTransferoTVSorted;

	for( IDPTR IDPtr = pclTecBoxTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclTecBoxTable->GetNext( IDPtr.MP ) )
	{
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( IDPtr.MP );
		CPPUNIT_ASSERT( NULL != pclTecBox );

		if( 0 != CString( pclTecBox->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TV_C") ) )
		{
			continue;
		}

		CString strID = pclTecBox->GetIDPtr().ID;
		std::vector<CString> vecStrings;

		clTUProdSelFileHelper.SplitOneLineMultiValues( strID, vecStrings, _T("_"), false );
		CPPUNIT_ASSERT( 4 == vecStrings.size() );

		UCHAR ucKey = _wtoi( vecStrings.at( 2 ) ) << 2;
		ucKey += ( 1 == pclTecBox->GetNbrPumpCompressor() ) ? 0 : 2;
		ucKey += ( CDB_TecBox::ePMVariant_HighFlowTemp == ( CDB_TecBox::ePMVariant_HighFlowTemp & pclTecBox->GetTecBoxVariant() ) ) ? 1 : 0;

		if( 0 != mapTransferoTVSorted.count( ucKey ) )
		{
			continue;
		}

		mapTransferoTVSorted.insert( std::pair<UCHAR, CDB_TecBox *>( ucKey, pclTecBox ) );
	}

	pclPMInputUser->SetApplicationType( ProjectType::Heating );
	pclPMInputUser->SetNormID( _T("PM_NORM_EN12828") );
	pclPMInputUser->SetInstalledPower( 10000.0 );

	for( auto &iter : mapTransferoTVSorted )
	{
		for( int iPipeLoop = 0; iPipeLoop < 2; iPipeLoop++ )
		{
			int iPipeLength = ( 0 == iPipeLoop ) ? 10 : 30;
			strHeader.Format( _T("Transfero TV Connect|%s|Heating|EN 12828|%i|"), iter.second->GetName(), iPipeLength );

			for( int iHST = 10; iHST < 100; iHST+= 2 )
			{
				pclPMInputUser->SetStaticHeight( (double)iHST );
				ExpansionPipeIDPtr = pclPMInputUser->GetExpansionPipeSizeIDPtr( iter.second, (double)iPipeLength );

				strLine = strHeader;
				str.Format( _T("%i|10000.0|%s\n"), iHST, ExpansionPipeIDPtr.ID );
				strLine += str;
				pOutf->WriteTxtLine( strLine );
			}
		}
	}

	// Test now all Transfero TVI Connect.

	// Sort Transfero TVI Connect.
	// Key= power (max 6800)
	std::map<UINT16, CDB_TecBox *> mapTransferoTVISorted;

	for( IDPTR IDPtr = pclTecBoxTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclTecBoxTable->GetNext( IDPtr.MP ) )
	{
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( IDPtr.MP );
		CPPUNIT_ASSERT( NULL != pclTecBox );

		if( 0 != CString( pclTecBox->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TVI_C") ) )
		{
			continue;
		}

		UINT16 ucKey = (UINT16)pclTecBox->GetPower();

		if( 0 != mapTransferoTVISorted.count( ucKey ) )
		{
			continue;
		}

		mapTransferoTVISorted.insert( std::pair<UINT16, CDB_TecBox *>( ucKey, pclTecBox ) );
	}

	for( auto &iter : mapTransferoTVISorted )
	{
		for( int iPipeLoop = 0; iPipeLoop < 2; iPipeLoop++ )
		{
			int iPipeLength = ( 0 == iPipeLoop ) ? 10 : 30;
			strHeader.Format( _T("Transfero TVI Connect|%s|Heating|EN 12828|%i|"), iter.second->GetName(), iPipeLength );

			for( int iHST = 70; iHST < 200; iHST+= 2 )
			{
				pclPMInputUser->SetStaticHeight( (double)iHST );
				ExpansionPipeIDPtr = pclPMInputUser->GetExpansionPipeSizeIDPtr( iter.second, (double)iPipeLength );

				strLine = strHeader;
				str.Format( _T("%i|10000.0|%s\n"), iHST, ExpansionPipeIDPtr.ID );
				strLine += str;
				pOutf->WriteTxtLine( strLine );
			}
		}
	}

	// Test now all Transfero TI.

	// Sort Transfero TI Connect.
	// Take one from TI ..0.2, TI ..1.2, TI ..2.2, TI .. 3.2
	std::map<UCHAR, CDB_TecBox *> mapTransferoTISorted;

	for( IDPTR IDPtr = pclTecBoxTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclTecBoxTable->GetNext( IDPtr.MP ) )
	{
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( IDPtr.MP );
		CPPUNIT_ASSERT( NULL != pclTecBox );

		if( 0 != CString( pclTecBox->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TIC") ) )
		{
			continue;
		}

		CString strID = pclTecBox->GetIDPtr().ID;
		std::vector<CString> vecStrings;

		clTUProdSelFileHelper.SplitOneLineMultiValues( strID, vecStrings, _T("_"), false );
		CPPUNIT_ASSERT( 4 == vecStrings.size() );

		UCHAR ucKey = vecStrings.at( 2 ).GetAt( vecStrings.at( 2 ).GetLength() - 1 ) - 48;

		if( 0 != mapTransferoTISorted.count( ucKey ) )
		{
			continue;
		}

		mapTransferoTISorted.insert( std::pair<UCHAR, CDB_TecBox *>( ucKey, pclTecBox ) );
	}

	pclPMInputUser->SetStaticHeight( 10.0 );

	for( auto &iter : mapTransferoTISorted )
	{
		for( int iPipeLoop = 0; iPipeLoop < 2; iPipeLoop++ )
		{
			int iPipeLength = ( 0 == iPipeLoop ) ? 10 : 30;
			strHeader.Format( _T("Transfero TI|%s|Heating|EN 12828|%i|10.0|10000.0"), iter.second->GetName(), iPipeLength );

			ExpansionPipeIDPtr = pclPMInputUser->GetExpansionPipeSizeIDPtr( iter.second, (double)iPipeLength );

			strLine = strHeader;
			str.Format( _T("|%s\n"), ExpansionPipeIDPtr.ID );
			strLine += str;
			pOutf->WriteTxtLine( strLine );
		}
	}

	utests_Init::CloseTxtFile( pOutf );

	// Compare with reference file.
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_PRESSURISATION ), CString( __FILE__ ), strFileName );
}
