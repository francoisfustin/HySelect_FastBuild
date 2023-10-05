#include "stdafx.h"

#include "utests_base.h"
#include "utests_HydronicCalculation.h"
#include "TASelectDoc.h"
#include "DeletedProductHelper.h"
#include  "locale"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_HydronicCalculation, CPPUNIT_TEST_CATEGORYNAME_HYDRONICCALCULATION );

void utests_HydronicCalculation::setUp()
{
	utests_base::setUp();
	utests_Init::InitDefaultDbValues();
	CDimValue::AccessUDB()->SetDefaultUnit( _U_DIFFPRESS, _U_DIFFPRESS_PA );	// Pa
	CDimValue::AccessUDB()->SetDefaultUnit( _U_FLOW, _U_FLOW_L_H );	// l/h

	m_mapFolderToTest[L"CV"] = L"CV\\";
	m_mapFolderToTest[L"DPC"] = L"DpC\\";
	m_mapFolderToTest[L"2WInj"] = L"2WInj\\";
	m_mapFolderToTest[L"3WDiv"] = L"3WDiv\\";
	m_mapFolderToTest[L"3WInj"] = L"3WInj\\";
	m_mapFolderToTest[L"3WMix"] = L"3WMix\\";
//	m_mapFolderToTest[L"SmartControlValve"] = L"SmartControlValve\\";
	m_mapFolderToTest[L"Pending"] = L"Pending\\";
	m_mapFolderToTest[L"Other"] = L"Other\\";
	m_mapFolderToTest[L"AutoAdaptive"] = L"AutoAdaptive\\";

	for( std::map<CString, CString>::iterator it = m_mapFolderToTest.begin(); it != m_mapFolderToTest.end(); ++it )
	{
		CString OutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_HYDRONICCALCULATION ), CString( __FILE__ ) ) + it->second;
		// Create folder if doesn't exist
		WinSys::CreateFullDirectoryPath( OutPath );
	}
}

void utests_HydronicCalculation::tearDown()
{
	utests_base::tearDown();

	// Warning: when RestoreDefaultDbValues is called it change technical date and relaunch a calculus on the latest computed circuit
	// some error would be displayed but it will not impact unit tests
	utests_Init::RestoreDefaultDbValues();
	CDimValue::AccessUDB()->SetDefaultUnit( _U_DIFFPRESS, _U_DIFFPRESS_PA );	// Pa
	CDimValue::AccessUDB()->SetDefaultUnit( _U_FLOW, _U_FLOW_L_H );	// l/h
}

void utests_HydronicCalculation::TestHydraulicNetwork_CV()
{
	_TestHydraulicNetwork( m_mapFolderToTest[L"CV"] );
}

void utests_HydronicCalculation::TestHydraulicNetwork_DPC()
{
	_TestHydraulicNetwork( m_mapFolderToTest[L"DPC"] );
}

void utests_HydronicCalculation::TestHydraulicNetwork_2WInj()
{
	_TestHydraulicNetwork( m_mapFolderToTest[L"2WInj"] );
}

void utests_HydronicCalculation::TestHydraulicNetwork_3WInj()
{
	_TestHydraulicNetwork( m_mapFolderToTest[L"3WInj"] );
}

void utests_HydronicCalculation::TestHydraulicNetwork_3WDiv()
{
	_TestHydraulicNetwork( m_mapFolderToTest[L"3WDiv"] );
}

void utests_HydronicCalculation::TestHydraulicNetwork_3WMix()
{
	_TestHydraulicNetwork( m_mapFolderToTest[L"3WMix"] );
}

void utests_HydronicCalculation::TestHydraulicNetwork_SmartControlValve()
{
	_TestHydraulicNetwork( m_mapFolderToTest[L"SmartControlValve"] );
}

void utests_HydronicCalculation::TestHydraulicNetwork_Pending()
{
	_TestHydraulicNetwork( m_mapFolderToTest[L"Pending"] );
}

void utests_HydronicCalculation::TestHydraulicNetwork_Other()
{
	_TestHydraulicNetwork( m_mapFolderToTest[L"Other"] );
}

void utests_HydronicCalculation::TestHydraulicNetwork_AutoAdaptive()
{
	_TestHydraulicNetwork( m_mapFolderToTest[L"AutoAdaptive"] );
}

void utests_HydronicCalculation::_TestHydraulicNetwork( CString ExtPath )
{
	// List tsp file
	// Open tsp file
	// export selection to txt file
	// compare content with reference txt

	// Get Data folder
	CFileFind finder;
	CString DataPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_HYDRONICCALCULATION ), CString( __FILE__ ), true ) + ExtPath;
	CString OutPath = utests_Init::GetTxtFilePath( CString( CPPUNIT_TEST_CATEGORYNAME_HYDRONICCALCULATION ), CString( __FILE__ ), false ) + ExtPath;

	DeleteFilesAndFolder( OutPath, _T("*.*") );

	if( DataPath.ReverseFind( '\\' ) != DataPath.GetLength() - 1 )
	{
		DataPath += CString( _T("\\") );
	}

	// For each tsp file
	BOOL bWorking = finder.FindFile( DataPath + _T("*.tsp") );

	while( bWorking )
	{
		bWorking = finder.FindNextFile();

		try
		{
			if( finder.IsDots() )
			{
				continue;
			}

			CString strTsp = DataPath + finder.GetFileName();

			if( true == strTsp.IsEmpty() )
			{
				continue;
			}

			// File open
			// Reset flag modified.
			::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_OPENDOCUMENT, false, ( LPARAM )&strTsp );
			LRESULT lresult = ::AfxGetApp()->m_pMainWnd->SendMessage(WM_USER_OPENDOCUMENT, false, (LPARAM)&strTsp);
			
			if( 0 == lresult )
			{
				CTableHM *pTableHM = dynamic_cast<CTableHM *>(TASApp.GetpTADS()->GetpHydroModTable());

				if( NULL != pTableHM )
				{
					CDeletedProductHelper *pProductHelper = new CDeletedProductHelper();
					bool bDisplay = false;
					pProductHelper->VerifyHydroMod(pTableHM->GetIDPtr(), bDisplay);
					delete pProductHelper;
				}
			}
				
			// Export to txt.
			CString strTxt = OutPath + finder.GetFileTitle() + _T(".txt");
				
			// HYS-1191: force HM calc mode to 'true' otherwise the 'Freeze( false )' will have not effect.
			// Remark: when sending 'WM_USER_OPENDOCUMENT' we will get the 'CMainFrame::OnNewDocument()' method called.
			// And there, if activation code is expired or incorrect, the HMCalc mode will not be set.
			pMainFrame->SetHMCalcMode( true );

			pMainFrame->Freeze( false );
			pMainFrame->Freeze( true );
			pMainFrame->Freeze( false );

			_DropHMContent( strTxt, true );
		}
		catch( CFileException * )
		{
		}
	}

	// Compare now all the files outside the first loop to not block the process as soon as there is an
	// error on one file.
	string strAllFailed = "";
	string strAllErrors = "";
	bWorking = finder.FindFile( DataPath + _T("*.tsp") );

	while( TRUE == bWorking )
	{
		bWorking = finder.FindNextFile();

		if( TRUE == finder.IsDots() )
		{
			continue;
		}

		CString strTsp = DataPath + finder.GetFileName();
			
		if( true == strTsp.IsEmpty() )
		{
			continue;
		}

		CString strTxt = OutPath + finder.GetFileTitle() + _T(".txt");
		CString SrcFile = DataPath + finder.GetFileTitle() + _T(".txt");

		// Compare with reference file.
		if( false == IsFilesEqual( SrcFile, strTxt ) )
		{
			strAllFailed += finder.GetFileName() + "\r\n";
		}

		// Find error in HM Results.
		CString strToFind =  _T("ERROR");

		FILE *fb = _wfopen( SrcFile, _T("rb") );
		wchar_t tcLine[1024];
		CString strLine;
		string strMessage = " WARNING: An error is found in ";
		string strErrorMessage = "";

		if( NULL != fb )
		{
			while( NULL != fgetws( tcLine, 1024, fb ) )
			{
				strLine = tcLine;
				strLine.Trim( _T("\r\n") );
						
				if( strLine.Find( strToFind, 0 ) != -1 )
				{
					strErrorMessage += (string)(CStringA)finder.GetFileName()+ "\r\n";
				}
			}

			fclose( fb );
		}

		if( !strErrorMessage.empty() )
		{
			strAllErrors += strMessage + strErrorMessage + "\r\n";
		}
	}

	if( false == strAllFailed.empty() )
	{
		CPPUNIT_FAIL( strAllFailed + strAllErrors );
	}
}

void utests_HydronicCalculation::_DropHMContent( CString strfn, bool bClearFile )
{
	CFileTxt fw;

	try
	{
		int iReturn;
		UINT OpenFlag = CFile::modeCreate | CFile::modeReadWrite;

		if( false == bClearFile )
		{
			OpenFlag |= CFile::modeNoTruncate;
		}

		iReturn = fw.Open( strfn, OpenFlag );

		if( 0 == iReturn )
		{
			throw;
		}

		fw.SeekToEnd();

		// Write header.
		CString str;
		fw.WriteTxtLine( _T("\xFEFF") );
		fw.WriteTxtLine( _T("\n //Auto Generated test block\n") );

		CPath path( pTASelectDoc->GetPathName() );
		str.Format( _T("OpenTSPFile; %s\n"), path.GetFilename() );
		fw.WriteTxtLine( str );

		str.Format( _T("SetHMCalcMode; true\n") );
		fw.WriteTxtLine( str );

		str.Format( _T("HydroMod; Unfreeze\n") );
		fw.WriteTxtLine( str );

		_FillHM( &fw );

		str.Format( _T("Rem; --------------------->SUCCESS") );

		fw.WriteTxtLine( str );
		fw.Flush();
		fw.Close();
	}
	catch( CFileException *e )
	{
		TCHAR szCause[255];
		CString strFormatted;

		e->GetErrorMessage( szCause, 255 );
		strFormatted = _T("Error: ");
		strFormatted += szCause;
		AfxMessageBox( strFormatted );
	}
}

void utests_HydronicCalculation::_FillHM( CFileTxt *pfw, CTable *pTab )
{
	if( NULL == pTab )
	{
		pTab = TASApp.GetpTADS()->GetpHydroModTable();
	}

	// Order HM by position.
	std::map<int, CDS_HydroMod *> mapHM;
	std::map<int, CDS_HydroMod *>::iterator It;

	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( idptr.MP );
		mapHM[pHM->GetPos()] = pHM;
	}

	for( It = mapHM.begin(); It != mapHM.end(); ++It )
	{
		_WriteHMInfo( pfw, It->second );

		if( true == It->second->IsaModule() )
		{
			_FillHM( pfw, It->second );
		}
	}
}

void utests_HydronicCalculation::_WriteHMInfo( CFileTxt *pfw, CDS_HydroMod *pHM )
{
	CString strLineHeader;
	CString strLine, str;
	strLineHeader.Format( _T("%s; %s;"), (LPCTSTR)pHM->GetHMName(), (LPCTSTR)pHM->GetSchemeID() );

	// HM, TuDp.
	str = WriteCUDouble( _U_DIFFPRESS, pHM->GetpTermUnit()->GetDp(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" HM; TU-Dp;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// HM, TuQ.
	str = WriteCUDouble( _U_FLOW, pHM->GetpTermUnit()->GetQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" HM; TU-Q;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	if( true == pHM->IsInjectionCircuit() && NULL != dynamic_cast<CDS_HmInj *>( pHM ) )
	{
		// HM, Secondary side - Design supply temperature.
		CDS_HmInj *pHMInj = (CDS_HmInj *)( pHM );

		str = WriteCUDouble( _U_TEMPERATURE, pHMInj->GetDesignCircuitSupplySecondaryTemperature(), true, 3, 0, true, _U_TEMPERATURE_C );	// Temperature -> °C
		str.Replace( _T(","), _T(".") );
		strLine = strLineHeader + _T(" Injection design supply temperature;") + str + _T("\n");
		pfw->WriteTxtLine( strLine );

		// HM, Secondary side - Design return temperature.
		str = WriteCUDouble( _U_TEMPERATURE, pHMInj->GetDesignCircuitReturnSecondaryTemperature(), true, 3, 0, true, _U_TEMPERATURE_C );	// Temperature -> °C
		str.Replace( _T(","), _T(".") );
		strLine = strLineHeader + _T(" Injection design return temperature;") + str + _T("\n");
		pfw->WriteTxtLine( strLine );

		// HM, Secondary side - Computed return temperature.
		double dComputedTemperature = pHMInj->GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );
		str = WriteCUDouble( _U_TEMPERATURE, dComputedTemperature, true, 3, 0, true, _U_TEMPERATURE_C );	// Temperature -> °C
		str.Replace( _T(","), _T(".") );
		strLine = strLineHeader + _T(" Injection computed return temperature;") + str + _T("\n");
		pfw->WriteTxtLine( strLine );
	}

	// HM, Dp.
	str = WriteCUDouble( _U_DIFFPRESS, pHM->GetDp(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" HM; Dp;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// HM, Flow.
	str = WriteCUDouble( _U_FLOW, pHM->GetQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" HM; FLOW;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// HM, Havail.
	str = WriteCUDouble( _U_DIFFPRESS, pHM->GetHAvail(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" HM; HAvail;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	strLine = strLineHeader + _T(" ***ERROR*** Havail is < Dp;") + _T("\n");

	CDB_PIControlValve *pPICV = NULL;

	if( NULL != pHM->GetpCV() )
	{
		pPICV = dynamic_cast<CDB_PIControlValve *>( pHM->GetpCV()->GetCvIDPtr().MP );
	}

	if( NULL != pPICV )
	{
		// if( pHM->GetHAvail() < pHM->GetDp() )
		if( pHM->GetDp() - pHM->GetHAvail() > 2 )
		{
			pfw->WriteTxtLine( strLine );
		}
	}
	else if( abs( pHM->GetHAvail() - pHM->GetDp() ) > 2 )
	{
		pfw->WriteTxtLine( strLineHeader + _T(" ***ERROR*** Havail is > Dp + 2 ;") + _T("\n") );
	}

	if( true == pHM->IsBvExist( ( true ) ) )
	{
		_WriteBVInfo( pfw, strLineHeader + _T(" BVP;"), pHM->GetpBv() );
	}

	if( true == pHM->IsBvBypExist( ( true ) ) )
	{
		_WriteBVInfo( pfw, strLineHeader + _T(" BVB;"), pHM->GetpBypBv() );
	}

	if( true == pHM->IsBvSecExist( ( true ) ) )
	{
		_WriteBVInfo( pfw, strLineHeader + _T(" BVS;"), pHM->GetpSecBv() );
	}

	if( true == pHM->IsCvExist( ( true ) ) )
	{
		_WriteCVInfo( pfw, strLineHeader + _T(" CV;"), pHM->GetpCV() );
	}

	if( true == pHM->IsDpCExist( ( true ) ) )
	{
		_WriteDpCInfo( pfw, strLineHeader + _T(" DpC;"), pHM->GetpDpC() );
	}

	if( true == pHM->IsShutoffValveExist( CDS_HydroMod::eHMObj::eShutoffValveSupply, true ) )
	{
		_WriteShutOffValveInfo( pfw, strLineHeader + _T(" Shutoff valve;"), pHM->GetpShutoffValve( CDS_HydroMod::eHMObj::eShutoffValveSupply ) );
	}

	if( true == pHM->IsShutoffValveExist( CDS_HydroMod::eHMObj::eShutoffValveReturn, true ) )
	{
		_WriteShutOffValveInfo( pfw, strLineHeader + _T(" Shutoff valve;"), pHM->GetpShutoffValve( CDS_HydroMod::eHMObj::eShutoffValveReturn ) );
	}

	if( pHM->IsSmartControlValveExist( true ) )
	{
		_WriteSmartControlValveInfo( pfw, strLineHeader + _T(" Smart control valve;"), pHM->GetpSmartControlValve() );
	}

	_WritePipeInfo( pfw, strLineHeader + _T(" CIRCPIPE;"), pHM, CDS_HydroMod::eHMObj::eCircuitPrimaryPipe );
	_WritePipeInfo( pfw, strLineHeader + _T(" BYPASSPIPE;"), pHM, CDS_HydroMod::eHMObj::eCircuitBypassPrimaryPipe );
	_WritePipeInfo( pfw, strLineHeader + _T(" SECCIRCPIPE;"), pHM, CDS_HydroMod::eHMObj::eCircuitSecondaryPipe );
	_WritePipeInfo( pfw, strLineHeader + _T(" DISTSUPPIPE;"), pHM, CDS_HydroMod::eHMObj::eDistributionSupplyPipe );
	_WritePipeInfo( pfw, strLineHeader + _T(" DISTRETPIPE;"), pHM, CDS_HydroMod::eHMObj::eDistributionReturnPipe );
	_WritePipeAccessoryInfo( pfw, strLineHeader + _T(" CIRCACC;"), pHM, CDS_HydroMod::eHMObj::eCircuitPrimaryPipe );
	_WritePipeAccessoryInfo( pfw, strLineHeader + _T(" BYPASSACC;"), pHM, CDS_HydroMod::eHMObj::eCircuitBypassPrimaryPipe );
	_WritePipeAccessoryInfo( pfw, strLineHeader + _T(" SECCIRCACC;"), pHM, CDS_HydroMod::eHMObj::eCircuitSecondaryPipe );
	_WritePipeAccessoryInfo( pfw, strLineHeader + _T(" DISTSUPACC;"), pHM, CDS_HydroMod::eHMObj::eDistributionSupplyPipe );
	_WritePipeAccessoryInfo( pfw, strLineHeader + _T(" DISTRETPIPE;"), pHM, CDS_HydroMod::eHMObj::eDistributionReturnPipe );
}

void utests_HydronicCalculation::_WritePipeInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod *pHM, CDS_HydroMod::eHMObj ePipeLocation )
{
	if( NULL == pHM || NULL == pHM->IsPipeExist( ePipeLocation ) )
	{
		return;
	}

	CPipes *pHMPipe = pHM->GetpPipe( ePipeLocation );
	CString strLine, str;
	
	// Name.
	// 	str = pHMPipe->GetPipeSeries()->GetName();
	// 	strLine = strLineHeader + _T(" PipeNAME;") + str + _T("\n");
	// 	pfw->WriteTxtLine(strLine);
	
	// Dp.
	str = WriteCUDouble( _U_DIFFPRESS, pHMPipe->GetPipeDp(), true, 6, 0, true, _U_DIFFPRESS_PA ); // Dp -> Pascal
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Dp;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	if( true == pHMPipe->IsDiversityApplied() )
	{
		// Total flow.
		str = WriteCUDouble( _U_FLOW, pHMPipe->GetTotalQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
		str.Replace( _T(","), _T(".") );
		strLine = strLineHeader + _T(" Total flow;") + str + _T("\n");
		pfw->WriteTxtLine( strLine );
	}

	// Real flow (Different of total flow if diversity applied).
	str = WriteCUDouble( _U_FLOW, pHMPipe->GetRealQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Real flow;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// Supply temperature.
	CWaterChar *pclWaterChar = pHMPipe->GetpWaterChar( CAnchorPt::PipeLocation::PipeLocation_Supply );

	if( NULL != pclWaterChar )
	{
		str = WriteCUDouble( _U_TEMPERATURE, pclWaterChar->GetTemp(), true, 3, 0, true, _U_TEMPERATURE_C );	// Temperature -> °C
		str.Replace( _T(","), _T(".") );
		strLine = strLineHeader + _T(" Supply temperature;") + str + _T("\n");
		pfw->WriteTxtLine( strLine );

		str = pclWaterChar->GetAdditFamID();	// HYS-1882
		strLine = strLineHeader + _T(" Addit Fam ID;") + str + _T("\n");
		pfw->WriteTxtLine( strLine );

		str = pclWaterChar->GetAdditID();	// 
		strLine = strLineHeader + _T(" Addit ID;") + str + _T("\n");
		pfw->WriteTxtLine( strLine );

		str = WriteCUDouble( _U_PERCENT, pclWaterChar->GetPcWeight(), true, 3, 0, true );	// Temperature -> °C
		strLine = strLineHeader + _T( " Pc weight;" ) + str + _T( "\n" );
		pfw->WriteTxtLine( strLine );
	}
	
	// Return temperature.
	pclWaterChar = pHMPipe->GetpWaterChar( CAnchorPt::PipeLocation::PipeLocation_Return );

	if( NULL != pclWaterChar )
	{
		str = WriteCUDouble( _U_TEMPERATURE, pclWaterChar->GetTemp(), true, 3, 0, true, _U_TEMPERATURE_C );	// Temperature -> °C
		str.Replace( _T(","), _T(".") );
		strLine = strLineHeader + _T(" Return temperature;") + str + _T("\n");
		pfw->WriteTxtLine( strLine );

		str = pclWaterChar->GetAdditFamID();	// HYS-1882
		strLine = strLineHeader + _T( " Addit Fam ID;" ) + str + _T( "\n" );
		pfw->WriteTxtLine( strLine );

		str = pclWaterChar->GetAdditID();	// 
		strLine = strLineHeader + _T( " Addit ID;" ) + str + _T( "\n" );
		pfw->WriteTxtLine( strLine );

		str = WriteCUDouble( _U_PERCENT, pclWaterChar->GetPcWeight(), true, 3, 0, true );	// Temperature -> °C
		strLine = strLineHeader + _T( " Pc weight;" ) + str + _T( "\n" );
		pfw->WriteTxtLine( strLine );
	}
}

void utests_HydronicCalculation::_WritePipeAccessoryInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod *pHM, CDS_HydroMod::eHMObj ePipeLocation )
{
	if( NULL == pHM || NULL == pHM->IsPipeExist( ePipeLocation ) )
	{
		return;
	}

	CPipes *pHMPipe = pHM->GetpPipe( ePipeLocation );
	CString strLine, str;
	
	// Dp.
	str = WriteCUDouble( _U_DIFFPRESS, pHMPipe->GetSingularityTotalDp( true ), true, 6, 0, true, _U_DIFFPRESS_PA );	// Flow -> l/h
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Dp;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );
}

void utests_HydronicCalculation::_WriteDpCInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CDpC *pHMDpC )
{
	CString strLine, str;
	
	// Name.
	str = pHMDpC->GetpTAP()->GetName();
	strLine = strLineHeader + _T(" Name;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// ID.
	str = pHMDpC->GetIDPtr().ID;
	strLine = strLineHeader + _T(" ID;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );
	
	// Dp.
	str = WriteCUDouble( _U_DIFFPRESS, pHMDpC->GetDp(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Dp;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );
	
	// Flow.
	str = WriteCUDouble( _U_FLOW, pHMDpC->GetQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Flow;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// Presetting computed.
	str.Format( _T("%g"), pHMDpC->GetDpCSetting() );
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Presetting computed;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// Presetting displayed.
	str = pHMDpC->GetSettingStr( false );
	str.Remove( _T('*') );
	str.Replace( _T(","), _T(".") );

	CString strRounding = _T("-");

	if( NULL != dynamic_cast<CDB_Characteristic *>( pHMDpC->GetpTAP()->GetValveCharDataPointer() ) )
	{
		CDB_Characteristic *pChar = (CDB_Characteristic *)( pHMDpC->GetpTAP()->GetValveCharDataPointer() );
		strRounding.Format( _T("%g"), pChar->GetSettingRounding() );
	}
	
	strLine = strLineHeader + _T(" Presetting displayed;") + str + _T(" (rounding = ") + strRounding + _T(")\n");
	pfw->WriteTxtLine( strLine );
}

void utests_HydronicCalculation::_WriteBVInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CBV *pHMBV )
{
	CString strLine, str;
	
	// Name.
	str = pHMBV->GetpTAP()->GetName();
	strLine = strLineHeader + _T(" Name;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// ID.
	str = pHMBV->GetIDPtr().ID;
	strLine = strLineHeader + _T(" ID;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );
	
	// Dp.
	str = WriteCUDouble( _U_DIFFPRESS, pHMBV->GetDp(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Dp;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );
	
	// Flow.
	str = WriteCUDouble( _U_FLOW, pHMBV->GetQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Flow;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );
	
	// Presetting computed.
	str.Format( _T("%g"), pHMBV->GetSetting() );
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Presetting computed;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// Presetting displayed.
	str = pHMBV->GetSettingStr( false );
	str.Replace( _T(","), _T(".") );
	str.Remove( _T('*') );

	if( false == str.IsEmpty() )
	{
		CString strRounding = _T("-");
		
		if( NULL != pHMBV->GetpTAP()->GetValveCharacteristic() )
		{
			strRounding.Format( _T("%g"), pHMBV->GetpTAP()->GetValveCharacteristic()->GetSettingRounding() );
		}

		strLine = strLineHeader + _T(" Presetting displayed;") + str + _T(" (rounding = ") + strRounding + _T(")\n");
		pfw->WriteTxtLine( strLine );
	}
}

void utests_HydronicCalculation::_WriteCVInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CCv *pHMCV )
{
	CString strLine, str;

	// Name.
	if( true == pHMCV->IsTaCV() )
	{
		str = pHMCV->GetpTAP()->GetName();
		strLine = strLineHeader + _T(" Name;") + str + _T("\n");
		pfw->WriteTxtLine( strLine );

		// ID.
		str = pHMCV->GetCvIDPtr().ID;
		strLine = strLineHeader + _T(" ID;") + str + _T("\n");
		pfw->WriteTxtLine( strLine );

		// If characteristic exist, we can show presetting.
		if( NULL != pHMCV->GetpTAP()->GetValveCharacteristic() )
		{
			// Presetting computed.
			str.Format( _T("%g"), pHMCV->GetSetting() );
			str.Replace( _T(","), _T(".") );

			strLine = strLineHeader + _T(" Presetting computed;") + str + _T("\n");
			pfw->WriteTxtLine( strLine );

			// Presetting displayed.
			CString strRounding;
			strRounding.Format( _T("%g"), pHMCV->GetpTAP()->GetValveCharacteristic()->GetSettingRounding() );

			str = pHMCV->GetSettingStr( false );
			str.Remove( _T('*') );
			str.Replace( _T(","), _T(".") );

			strLine = strLineHeader + _T(" Presetting displayed;") + str + _T(" (rounding = ") + strRounding + _T(")\n");
			pfw->WriteTxtLine( strLine );
		}

		// Actuator.
		CDB_Actuator *pActr = dynamic_cast<CDB_Actuator *>( pHMCV->GetActrIDPtr().MP );

		if( NULL != pActr )
		{
			str = pActr->GetName();
			strLine = strLineHeader + _T(" CVACTR;") + str + _T("\n");
			pfw->WriteTxtLine( strLine );
		}
	}
	else
	{
		if( false == pHMCV->IsOn_Off() )
		{
			// Test Kvs < Kvsmax
			if( pHMCV->GetAuth() < 0.245 )
			{
				strLine = strLineHeader + _T(" ***ERROR*** Authority is < 0.25;") + _T("\n");
				pfw->WriteTxtLine( strLine );
			}

			str = WriteDouble( pHMCV->GetAuth(), 3, 2 );
			str.Replace( _T(","), _T(".") );
			strLine = strLineHeader + _T(" BetaMin;") + str + _T("\n");
			pfw->WriteTxtLine( strLine );
			
			str = WriteDouble( pHMCV->GetAuth( true ), 3, 2 );
			str.Replace( _T(","), _T(".") );
			strLine = strLineHeader + _T(" BetaDesign;") + str + _T("\n");
			pfw->WriteTxtLine( strLine );
		}
	}

	// Test Kvs < Kvsmax.
	if( true == pHMCV->IsTaCV() && true == pHMCV->IsPresettable() )
	{
		if( ( pHMCV->GetKv() - pHMCV->GetKvsmax() ) > 0.01 )
		{
			strLine = strLineHeader + _T(" ***ERROR*** Kv > Kvs max;") + _T("\n");
			pfw->WriteTxtLine( strLine );
		}

		// Kv.
		str = WriteDouble( pHMCV->GetKv(), 3, 2 );
		str.Replace( _T(","), _T(".") );
		strLine = strLineHeader + _T(" Kv;") + str + _T("\n");
		pfw->WriteTxtLine( strLine );
	}
	else
	{
		if( ( pHMCV->GetKvs() - pHMCV->GetKvsmax() ) > 0.01 )
		{
			strLine = strLineHeader + _T(" ***ERROR*** Kvs > Kvs max;") + _T("\n");
			pfw->WriteTxtLine( strLine );
		}

		// Kvs.
		str = WriteDouble( pHMCV->GetKvs(), 3, 2 );
		str.Replace( _T(","), _T(".") );
		strLine = strLineHeader + _T(" Kvs;") + str + _T("\n");
		pfw->WriteTxtLine( strLine );
	}

	// Kvs Max.
	str = WriteDouble( pHMCV->GetKvsmax(), 3, 2 );
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" KvsMax;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );
	
	// Dp.
	str = WriteCUDouble( _U_DIFFPRESS, pHMCV->GetDp(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Dp;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );
	
	// Flow.
	str = WriteCUDouble( _U_FLOW, pHMCV->GetQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Flow;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );
}

void utests_HydronicCalculation::_WriteShutOffValveInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CShutoffValve *pHMShutoffValve )
{
	CString strLine, str;
	
	// Name.
	str = pHMShutoffValve->GetpTAP()->GetName();
	strLine = strLineHeader + _T(" Name;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// ID.
	str = pHMShutoffValve->GetIDPtr().ID;
	strLine = strLineHeader + _T(" ID;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );
	
	// Dp.
	str = WriteCUDouble( _U_DIFFPRESS, pHMShutoffValve->GetDp(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Dp;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );
	
	// Flow.
	str = WriteCUDouble( _U_FLOW, pHMShutoffValve->GetQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Flow;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );
}

void utests_HydronicCalculation::_WriteSmartControlValveInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CSmartControlValve *pHMSmartControlValve )
{
	CString strLine, str;
	
	// Name.
	str = pHMSmartControlValve->GetpTAP()->GetName();
	strLine = strLineHeader + _T(" Name;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// ID.
	str = pHMSmartControlValve->GetIDPtr().ID;
	strLine = strLineHeader + _T(" ID;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// Control mode.
	CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( pHMSmartControlValve->GetIDPtr().MP );
	ASSERT( NULL != pclSmartControlValve );

	str = pclSmartControlValve->GetControlModeString( pHMSmartControlValve->GetControlMode() );
	strLine = strLineHeader + _T(" Control mode;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// Localization.
	str = pclSmartControlValve->GetLocalizationString( pHMSmartControlValve->GetLocalization() );
	strLine = strLineHeader + _T(" Localization;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// Dp.
	str = WriteCUDouble( _U_DIFFPRESS, pHMSmartControlValve->GetDp(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Dp;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	// Dp min.
	str = WriteCUDouble( _U_DIFFPRESS, pHMSmartControlValve->GetDpMin(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Dp min;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );
	
	// Flow.
	str = WriteCUDouble( _U_FLOW, pHMSmartControlValve->GetQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
	str.Replace( _T(","), _T(".") );
	strLine = strLineHeader + _T(" Flow;") + str + _T("\n");
	pfw->WriteTxtLine( strLine );

	if( SmartValveControlMode::SCVCM_Flow == pHMSmartControlValve->GetControlMode() )
	{
		// Flow max.
		str = WriteCUDouble( _U_FLOW, pHMSmartControlValve->GetFlowMax(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
		str.Replace( _T(","), _T(".") );
		strLine = strLineHeader + _T(" Flow;") + str + _T("\n");
		pfw->WriteTxtLine( strLine );
	}
	else if( SmartValveControlMode::SCVCM_Power == pHMSmartControlValve->GetControlMode() )
	{
		// Power max.
		str = WriteCUDouble( _U_TH_POWER, pHMSmartControlValve->GetPowerMax(), true, 6, 0, true, _U_TH_POWER_W );	// Power -> Watt
		str.Replace( _T(","), _T(".") );
		strLine = strLineHeader + _T(" Flow;") + str + _T("\n");
		pfw->WriteTxtLine( strLine );
	}
}
