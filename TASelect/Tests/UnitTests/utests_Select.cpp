#include "stdafx.h"

#include "utests_base.h"
#include "utests_Select.h"
#include "taselect.h"
#include "MainFrm.h"
#include "DataStruct.h"
#include "Select.h"
#include "ProductSelectionParameters.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_SelectPipeList, CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION );

void utests_SelectPipeList::setUp()
{
	utests_base::setUp();
	utests_Init::InitDefaultDbValues();

	CString OutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ) );

	if( false == OutPath.IsEmpty() )
	{
		DeleteFilesAndFolder( OutPath, _T("*.*") );
	}
}

void utests_SelectPipeList::tearDown()
{
	utests_base::tearDown();
	utests_Init::RestoreDefaultDbValues();
}

void utests_SelectPipeList::TestPipeList()
{
	CWaterChar clWC;
	utests_Init::InitFluidWater( &clWC );
	
	CSelectPipeList clSelectPipeList;

	// Access to pipe series table
	CTable *pPipeTab = static_cast<CTable *>( TASApp.GetpPipeDB()->Get( _T("STEEL_GEN") ).MP );
	CPPUNIT_ASSERT( NULL != pPipeTab );

	CProductSelelectionParameters clCProductSelelectionParameters( TASApp.GetpTADB(), TASApp.GetpTADS(), TASApp.GetpUserDB(),
			TASApp.GetpPipeDB(), &clWC, pPipeTab->GetIDPtr().ID, _T("") );

	const int iNbreOfTest = 5000;
	CString strPipeID;
	CString strFileName = _T("PipeList.txt");
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ), strFileName );

	// Code to extract data.
	double dMinValue = 1000;
	double dMaxValue = 5000000;
	double dIncrement = 1000;

	for( double d = dMinValue; d < dMaxValue; d += dIncrement )
	{
		double dFlow = d / 1000 / 3600;		// To SI
		int iCount = clSelectPipeList.SelectPipes( &clCProductSelelectionParameters, dFlow );
		CSelectPipe *pSelectPipe = clSelectPipeList.GetBestSelectPipe();

		if( NULL == pSelectPipe || NULL == pSelectPipe->GetpPipe() )
		{
			continue;
		}

		CDB_Pipe *pclPipe = pSelectPipe->GetpPipe();

		if( 0 == strPipeID.Compare( pclPipe->GetIDPtr().ID ) )
		{
			continue;
		}

		strPipeID = pclPipe->GetIDPtr().ID;

		CString str;
		str.Format( _T("%s; %s; %s; %s\n"), pclPipe->GetIDPtr().ID, CString( WriteDouble( pSelectPipe->GetLinDp(), 12 ) ),
					CString( WriteDouble( pSelectPipe->GetU(), 12 ) ), CString( WriteDouble( dFlow, 12 ) ) );
		
		str.Replace( _T("," ), _T( ".") );
		pOutf->WriteTxtLine( ( LPCTSTR )str );
	}
	
	utests_Init::CloseTxtFile( pOutf );

	//Compare with reference file
	 utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ), strFileName );
}


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_SelectPIBCVList, "Select" );

void utests_SelectPIBCVList::setUp()
{
	utests_Init::InitDefaultDbValues();
}

void utests_SelectPIBCVList::tearDown()
{
	utests_Init::RestoreDefaultDbValues();
}

void utests_SelectPIBCVList::TestPIBCVList()
{
	CWaterChar clWC;
	utests_Init::InitFluidWater( &clWC );
	
	// Access to pipe series table.
	CTable *pPipeTab = static_cast<CTable *>( TASApp.GetpPipeDB()->Get( _T("STEEL_GEN") ).MP );
	CPPUNIT_ASSERT( NULL != pPipeTab );

	CIndSelPIBCVParams clIndSelPIBCVParams( TASApp.GetpTADB(), TASApp.GetpTADS(), TASApp.GetpUserDB(), TASApp.GetpPipeDB(),
			&clWC, pPipeTab->GetIDPtr().ID, _T("") );

	CRankEx PIBCVListSave;
	
	int iCount = GetpTADB()->GetTaCVList( 
			&clIndSelPIBCVParams.m_CtrlList,					// List where to saved
			CTADatabase::eForPiCv,								// Control valve target (cv, hmcv, picv or bcv)
			false, 												// 'true' returns as soon a result is found
			CDB_ControlProperties::CV2W,		 				// Set way number of valve
			_T( "" ),											// Type ID
			_T( "" ),											// Family ID
			_T( "" ),											// Body material ID
			_T( "" ),											// Connection ID
			_T( "" ),											// Version ID
			_T( "" ),											// PN ID
			CDB_ControlProperties::LastCVFUNC, 					// Set the control function (control only, presettable, ...)
			CDB_ControlProperties::eCvProportional,				// Set the control type (on/off, proportional, ...)
			CTADatabase::FilterSelection::ForIndAndBatchSel,
			0,													// DNMin
			INT_MAX,											// DNMax
			false,												// 'true' if it's for hub station.
			NULL,												// 'pProd'.
			false												// 'true' if it's only for a set.
			);
	
	// Save valve list;
	PIBCVListSave = clIndSelPIBCVParams.m_CtrlList;
	CString strFileName = _T("PIBCVList.txt");
	
	// Code to extract data.
	CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ), strFileName );
	_string strID;
	CDB_PIControlValve *pPIBCV = NULL;

	for( BOOL bContinue = clIndSelPIBCVParams.m_CtrlList.GetFirstT<CDB_PIControlValve *>( strID, pPIBCV ); FALSE != bContinue; 
			bContinue = clIndSelPIBCVParams.m_CtrlList.GetNextT<CDB_PIControlValve *>( strID, pPIBCV ) )
	{
		pOutf->WriteTxtLine( (LPCTSTR)pPIBCV->GetIDPtr().ID );
		pOutf->WriteTxtLine( _T("\n") );
	}
	
	utests_Init::CloseTxtFile( pOutf );

	// Compare with reference file.
	utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ), strFileName );

	CString FileNameHeader = _T("PIBCV_%s.txt");
	double dMinValue = 1000;
	double dMaxValue = 75000;
	double dIncrement = 1000;

	for( double d = dMinValue; d < dMaxValue; d += dIncrement )
	{
		clIndSelPIBCVParams.m_dFlow = d / 1000.0 / 3600.0;		// To SI

		clIndSelPIBCVParams.m_pclSelectPIBCVList = new CSelectPICVList();
		clIndSelPIBCVParams.m_pclSelectPIBCVList->GetSelectPipeList()->SelectPipes( &clIndSelPIBCVParams, clIndSelPIBCVParams.m_dFlow );
		
		// Recall saved list.
		clIndSelPIBCVParams.m_CtrlList = PIBCVListSave;
		bool bSizeShiftProblem = false;
		int iDevFound = clIndSelPIBCVParams.m_pclSelectPIBCVList->SelectQ( &clIndSelPIBCVParams, &bSizeShiftProblem );
		
		CString strFilename;
		strFilename.Format( FileNameHeader, WriteDouble( d, 0 ) );
		CFileTxt *pOutf = utests_Init::CreateOutTxtFile( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString(__FILE__), strFilename );
		
		// Code to extract data.
		for( CSelectedValve *pclSelectedPICv = clIndSelPIBCVParams.m_pclSelectPIBCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedPICv; 
				pclSelectedPICv = clIndSelPIBCVParams.m_pclSelectPIBCVList->GetNext<CSelectedValve>() )
		{
			CString str;

			str.Format( _T("%s; %s; %s; %s"), pclSelectedPICv->GetProductIDPtr().ID, CString( WriteDouble( pclSelectedPICv->GetDpMin(), 12 ) ),
					CString( WriteDouble( pclSelectedPICv->GetH(), 12 ) ), CString( WriteDouble( pclSelectedPICv->GetFlag( CSelectedBase::efAll ), 0 ) ) );
			
			str.Replace( _T("," ), _T( ".") );
			pOutf->WriteTxtLine( ( LPCTSTR )str );
			pOutf->WriteTxtLine( _T("\n") );
		}
		
		utests_Init::CloseTxtFile( pOutf );
		delete clIndSelPIBCVParams.m_pclSelectPIBCVList;
		clIndSelPIBCVParams.m_pclSelectPIBCVList = NULL;
	}
	
	// Test Validity by file comparison.
	for( double d = dMinValue; d < dMaxValue; d += dIncrement )
	{
		CString strFileName;
		strFileName.Format( FileNameHeader, WriteDouble( d, 0 ) );
		utests_Init::CompareOutAndReferenceFiles( CString( CPPUNIT_TEST_CATEGORYNAME_PRODUCTSELECTION ), CString( __FILE__ ), strFileName );
	}
}
