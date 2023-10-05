#include "stdafx.h"


#include "mainfrm.h"
#include "taselectdoc.h"
#include "HMInclude.h"
#include "HiPerfTimer.h"
#include "DlgLeftTabProject.h"
#include "DlgWizTAScope.h"
#include "DlgPanelTAScopeDownload.h"
#include "TabDlgSpecActTesting.h"
#include "DeletedProductHelper.h"
#include "UnitTests/utests_base.h"
#include "UnitTests/utests_HydronicCalculation.h"

#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;


CTabDlgSpecActTesting::CTabDlgSpecActTesting( CWnd *pParent )
	: CDialogExt(CTabDlgSpecActTesting::IDD, pParent)
{
	SetBackgroundColor(_WHITE_DLGBOX);
}

BEGIN_MESSAGE_MAP(CTabDlgSpecActTesting, CDialogExt)
	ON_BN_CLICKED(IDC_BUTTONOPEN, &CTabDlgSpecActTesting::OnBnClickedBrowse)
	ON_BN_CLICKED(IDC_BUTTONSTART, &CTabDlgSpecActTesting::OnBnClickedStart)
	ON_BN_CLICKED(IDC_BUTTONSTOP, &CTabDlgSpecActTesting::OnBnClickedStop)
	ON_BN_CLICKED(IDC_BUTTONSNAPSHOOT, &CTabDlgSpecActTesting::OnBnClickedHMSnapshoot)
	ON_BN_CLICKED(IDC_BUTTONBATCHSNAPSHOOT, &CTabDlgSpecActTesting::OnBnClickedBatchSnapshoot)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CTabDlgSpecActTesting::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITFILENAME, m_EditFileName );
	DDX_Control( pDX, IDC_LIST, m_List );
}

BOOL CTabDlgSpecActTesting::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	CString str = TASApp.GetProfileString( _T("Testing"), _T("Filename"), _T("") );
	m_EditFileName.SetWindowText( str );
	m_bRun = false;

	return TRUE;
}

void CTabDlgSpecActTesting::OnBnClickedBrowse()
{
	CString str;
	m_EditFileName.GetWindowText(str);
	CString strFilter = _T("Batch files (*.txt)|*.txt||");
	CFileDialog dlg( TRUE, _T("txt"), str, OFN_EXTENSIONDIFFERENT, (LPCTSTR)strFilter, NULL, 0, FALSE );

	if( IDOK == dlg.DoModal() )
	{
		m_EditFileName.SetWindowText( dlg.GetPathName() );
		TASApp.WriteProfileString( _T("Testing"), _T("Filename"), (LPCTSTR)dlg.GetPathName() );
	}
}

void CTabDlgSpecActTesting::OnBnClickedStart()
{
	m_bRun = false;
	
	// Verify file.
	CString strfn;
	m_EditFileName.GetWindowText( strfn );
	_ProcessBatch( strfn );
}

void CTabDlgSpecActTesting::OnBnClickedStop()
{
	m_bRun = false;
}

void CTabDlgSpecActTesting::OnBnClickedHMSnapshoot()
{
	CString str;
	m_EditFileName.GetWindowText( str );
	CString strFilter=_T("Batch files (*.txt)|*.txt||");
	CFileDialog dlg( FALSE, _T("txt"), str, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, (LPCTSTR)strFilter, NULL, 0, FALSE );

	if( IDOK == dlg.DoModal() )
	{
		BeginWaitCursor();
		CString strfn = dlg.GetPathName( );
		_DropHMContent( strfn, true );
		EndWaitCursor();
	}
}

void CTabDlgSpecActTesting::OnBnClickedBatchSnapshoot()
{
	CString str;
	m_EditFileName.GetWindowText(str);
	CString strFilter = _T("Batch files (*.txt)|*.txt||");
	CFileDialog dlg( TRUE, _T("txt"), str, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, (LPCTSTR)strFilter, NULL, 0, FALSE );

	if( IDOK == dlg.DoModal() )
	{
		BeginWaitCursor();
		CString strfn = dlg.GetPathName( );
		_ProcessBatch( strfn );
		EndWaitCursor();
	}
}

void CTabDlgSpecActTesting::OnSize( UINT nType, int cx, int cy )
{
	CDialogExt::OnSize(nType, cx, cy);
	CRect rect;
	if (NULL != m_List.GetSafeHwnd())
	{
		m_List.GetWindowRect(&rect);
		ScreenToClient(rect);
		m_List.SetWindowPos(NULL, rect.left, rect.top, cx-rect.left, cy-rect.top, SWP_NOZORDER);
	}
	// TODO: Add your message handler code here
}

void CTabDlgSpecActTesting::_ProcessBatch( CString strfn )
{
	CFileTxt inpf;
	m_arBatch.RemoveAll();
	m_List.ResetContent();
	CHiPerfTimer HiPerfTimer;
	HiPerfTimer.Start();

	try
	{
		bool bContinueWithBatch = false;

		do
		{
			int iReturn = inpf.Open( strfn, CFile::modeRead );

			if( 0 == iReturn )
			{
				// Not a valid file.
				CString str;
				str.Format( _T("Not a valid file [%s], is it Unicode?"), strfn );
				m_List.InsertString( 0, str );
				return;
			}

			CPath path( strfn );
			m_RootFolder = path.GetPath();
			
			// Read text lines and store it.
			int iLineCount = 0;
			TCHAR tcBuffer[1024];
			memset( tcBuffer, 0, sizeof( tcBuffer ) );
			m_mapStr.clear();

			while( inpf.ReadTxtLine( tcBuffer, sizeof( tcBuffer ), &iLineCount ) > 0 )
			{
				CString strLine = tcBuffer;
				strLine.TrimLeft();
				strLine.TrimRight();

				if( true == strLine.IsEmpty() )
				{
					continue;
				}

				m_mapStr.insert( std::make_pair(iLineCount, strLine) );
			}
			
			inpf.Close();
			
			if( 0 == iLineCount )
			{
				// Not a valid file.
				CString str;
				str.Format( _T("Not a valid file, is it Unicode?") );
				m_List.InsertString( 0, str );
				return;
			}
			
			m_bRun = true;

			_Parse();

			if( m_arBatch.GetCount() > 0 )
			{
				strfn = m_arBatch[0];
				m_arBatch.RemoveAt( 0 );
				bContinueWithBatch = true && m_bRun;
			}
			else
			{
				bContinueWithBatch = false;
			}

		}while( true == bContinueWithBatch );

		HiPerfTimer.Stop();
		int iTime = (int)( HiPerfTimer.Duration() / 1000.0 );

		CString str;
		str.Format(_T("TotalTime(ms): %d"), iTime);
		m_List.InsertString( 0, str );
	}
	catch( CFileException *e )
	{
		TCHAR tcCause[255];
		CString strFormatted;

		e->GetErrorMessage( tcCause, 255 );
		strFormatted = _T("Error: ");
		strFormatted += tcCause;
		AfxMessageBox(strFormatted);
	}
}

void CTabDlgSpecActTesting::_Parse()
{
	std::map<int, CString>::iterator It;
	bool bRet = true;

	for( It = m_mapStr.begin(); It != m_mapStr.end() && true == bRet && true == m_bRun; It++ )
	{
		m_arStr.RemoveAll();
		CString strToken;
		int iPos = 0;
		strToken = It->second.Tokenize( _T(";"), iPos );
	
		while( strToken != _T("") )
		{
			strToken.TrimLeft();
			strToken.TrimRight();

			if( -1 == strToken.Find( _T(".tsp"), 0 ) && -1 == strToken.Find( _T(".txt"), 0 ) )
			{
				strToken.MakeUpper();
			}

			m_arStr.Add( strToken );
			strToken = It->second.Tokenize( _T(";"), iPos );
		}
		
		CString strRet;
		bRet = _InterpretCmd( &strRet );
		CString str;

		if( true == bRet )
		{
			if( 0 == m_arStr[0].Compare( _T("OPENTSPFILE") ) || 0 == m_arStr[0].Compare( _T("OPENTSCFILE") ) || 0 == m_arStr[0].Compare( _T("REM") ) )
			{
				str.Format( _T("%4d: %s"), It->first, It->second );
				m_List.InsertString( 0, str );
			}
		}
		else
		{
			str.Format(_T("%4d: [FAIL: %s] -> %s"), It->first, strRet, It->second);
			m_List.InsertString( 0, str );
			m_bRun = false;
		}
		
		MSG msg;

		while( ::PeekMessage( &msg, GetSafeHwnd(), 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		} 
	}
	
	if( false == m_bRun )
	{
		m_List.InsertString( 0, _T("Process aborted") );
	}
}

bool CTabDlgSpecActTesting::_InterpretCmd( CString *pstrReturn )
{
	*pstrReturn = _T("");

	if( 0 == m_arStr.GetSize() )
	{
		return false;
	}

	// Save current strPath.
	CString strfn;
	m_EditFileName.GetWindowText( strfn );

	if( 0 == m_arStr[0].Compare( _T("ADDBATCHFILE") ) )
	{
		///////////////////////
		// BATCHFILE
		///////////////////////

		if( m_arStr.GetSize() < 2 )
		{
			*pstrReturn = _T("Missing arguments");
			return false;
		}
		
		CString strTsp = m_RootFolder + m_arStr[1];
		m_arBatch.Add( strTsp );
		*pstrReturn = _T("");
		return true;
	}
	else if( 0 == m_arStr[0].Compare( _T("OPENTSPFILE") ) )
	{
		///////////////////////
		// OPENTSPFILE
		///////////////////////

		if( m_arStr.GetSize() < 2 )
		{
			*pstrReturn = _T("Missing arguments");
			return false;
		}
		
		CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
		pTechP->Init();
		TASApp.GetpUnitDB()->ResetToSI();

		// Send message that unit have been changed.
		AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_UNITCHANGE );
		CDimValue::AccessUDB()->SetDefaultUnit( _U_DIFFPRESS, _U_DIFFPRESS_PA );	// Pa
		CDimValue::AccessUDB()->SetDefaultUnit( _U_FLOW, _U_FLOW_L_H );	// l/h

		CString strTsp = m_RootFolder + m_arStr[1];
		CTADatastruct *pTADS = TASApp.GetpTADS();
		CString strPath = TASApp.FileOpen( NULL, strTsp );
		
		if( false == strPath.IsEmpty() )
		{
			// Reset flag modified.
			pTADS->Modified( false );
			::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_NEWDOCUMENT );
			LRESULT lresult = ::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_NEWDOCUMENT );
			pTASelectDoc->SetPathName( strTsp, FALSE );
			
			if( 0 == lresult )
			{
				CTableHM *pTableHM = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );

				if( NULL != pTableHM )
				{
					CDeletedProductHelper *pProductHelper = new CDeletedProductHelper();
					bool bDisplay = false;
					pProductHelper->VerifyHydroMod( pTableHM->GetIDPtr(), bDisplay );
					delete pProductHelper;
				}
			}

			pMainFrame->SetHMCalcMode( true );
			
			pMainFrame->Freeze( false );
			pMainFrame->Freeze( true );
			pMainFrame->Freeze( false );

			return true;
		}
		
		*pstrReturn = _T("Cannot open file");
		return false;
	}
	else if( 0 == m_arStr[0].Compare( _T("OPENTSCFILE") ) )
	{
		///////////////////////
		// OPENTSCFILE
		///////////////////////

		if( m_arStr.GetSize() < 2 )
		{
			*pstrReturn = _T("Missing arguments");
			return false;
		}

		CString strTsc = m_RootFolder + m_arStr[1];
		CDlgPanelTAScopeDownload dlg;
		
		if( false == dlg.DownLoadHMX( true, strTsc ) )
		{
			*pstrReturn = _T("Error when importing tsc file.");
			return false;
		}
		
		CTADatastruct *pTADS = TASApp.GetpTADS();
		pTADS->Modified( false );
		
		// Reset the upper and lower limits for measurements.
		pMainFrame->ResetMeasurementsLimits();

		// UnFreeze the project, reset the tree and pass in Non-HMCalc mode.
		pMainFrame->SetHMCalcMode( false );
		
		CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
		ASSERT( NULL != pPrjParam );
		
		pPrjParam->SetFreeze( false );
		
		BeginWaitCursor();
		
		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->ResetTree();
		}

		EndWaitCursor();

		return true;
	}
	else if( 0 == m_arStr[0].Compare( _T("CLOSEFILE") ) )
	{
		///////////////////////
		//  CLOSEFILE
		///////////////////////
	}
	else if( 0 == m_arStr[0].Compare( _T("DROPHMCONTENT") ) )
	{
		///////////////////////
		//  DROPHMCONTENT
		///////////////////////

		if( m_arStr.GetSize() < 3 )
		{
			*pstrReturn = _T("Missing arguments");
			return false;
		}
		
		CString strfn = m_RootFolder + m_arStr[1];
		_DropHMContent( strfn, ( 0 != m_arStr[2].Compare( _T("APPEND") ) ) );
		return true;
	}
	else if( 0 == m_arStr[0].Compare( _T("HYDROMOD") ) )
	{
		///////////////////////
		// HYDROMOD
		///////////////////////
		return _ProcessHydroMod( pstrReturn );
	}
	else if( 0 == m_arStr[0].Compare( _T("SETHMCALCMODE") ) )
	{
		if( 0 == m_arStr[1].Compare( _T("TRUE") ) )
		{
			pMainFrame->SetHMCalcMode( true );
		}
		else
		{
			pMainFrame->SetHMCalcMode( false );
		}

		return true;
	}
	else if( 0 == m_arStr[0].Compare( _T("REM") ) )
	{
		*pstrReturn = _T("");
		return true;
	}

	*pstrReturn = _T("Unknown command");
	return false;
}

void CTabDlgSpecActTesting::_DropHMContent( CString strfn, bool bClearFile )
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
		TCHAR tcCause[255];
		CString strFormatted;

		e->GetErrorMessage( tcCause, 255 );
		strFormatted = _T("Error: ");
		strFormatted += tcCause;
		AfxMessageBox( strFormatted );
	}
}

void CTabDlgSpecActTesting::_FillHM( CFileTxt *pfw, CTable *pTab )
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
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>(idptr.MP);
		mapHM[ pHM->GetPos() ] = pHM;
	}

	for( It = mapHM.begin(); It != mapHM.end(); It++ )
	{
		_WriteHMInfo( pfw, It->second );
		
		if( true == It->second->IsaModule() )
		{
			_FillHM( pfw, It->second );
		}
	}
}

void CTabDlgSpecActTesting::_WriteHMInfo( CFileTxt *pfw, CDS_HydroMod *pHM )
{
	CString strLineHeader;
	CString strLine, str;
	strLineHeader.Format( _T("%s; %s;"), (LPCTSTR)pHM->GetHMName(), (LPCTSTR)pHM->GetSchemeID() );

	// HM, TuDp.
	str = _T( "" );
	if( NULL != pHM->GetpTermUnit() )
	{
		str = WriteCUDouble( _U_DIFFPRESS, pHM->GetpTermUnit()->GetDp(), true, 6, 0, true, _U_DIFFPRESS_PA );
		str.Replace( _T( "," ), _T( "." ) );
	}
	strLine = strLineHeader + _T(" HM; TU-Dp;" ) + str + _T( "\n");
	pfw->WriteTxtLine( strLine );

	// HM, TuQ.
	str = _T( "" );
	if( NULL != pHM->GetpTermUnit() )
	{
		str = WriteCUDouble( _U_FLOW, pHM->GetpTermUnit()->GetQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
		str.Replace( _T( "," ), _T( "." ) );
	}
	strLine = strLineHeader + _T(" HM; TU-Q;" ) + str + _T( "\n");
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
	str.Replace( _T("," ), _T( ".") );
	strLine = strLineHeader + _T(" HM; Dp;" ) + str + _T( "\n");
	pfw->WriteTxtLine( strLine );

	// HM, Flow.
	str = WriteCUDouble( _U_FLOW, pHM->GetQ(), true, 6, 0, true, _U_FLOW_L_H );	// Flow -> l/h
	str.Replace( _T("," ), _T( ".") );
	strLine = strLineHeader + _T(" HM; FLOW;" ) + str + _T( "\n");
	pfw->WriteTxtLine( strLine );

	// HM, Havail.
	str = WriteCUDouble( _U_DIFFPRESS, pHM->GetHAvail(), true, 6, 0, true, _U_DIFFPRESS_PA );
	str.Replace( _T("," ), _T( ".") );
	strLine = strLineHeader + _T(" HM; HAvail;" ) + str + _T( "\n");
	pfw->WriteTxtLine( strLine );

	strLine = strLineHeader + _T(" ***ERROR*** Havail is < Dp;" ) + _T( "\n");

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
		pfw->WriteTxtLine( strLine );
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

void CTabDlgSpecActTesting::_WritePipeInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod *pHM, CDS_HydroMod::eHMObj ePipeLocation )
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

void CTabDlgSpecActTesting::_WritePipeAccessoryInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod *pHM, CDS_HydroMod::eHMObj ePipeLocation )
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

void CTabDlgSpecActTesting::_WriteDpCInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CDpC *pHMDpC )
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

void CTabDlgSpecActTesting::_WriteBVInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CBV *pHMBV )
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

void CTabDlgSpecActTesting::_WriteCVInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CCv *pHMCV )
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

void CTabDlgSpecActTesting::_WriteShutOffValveInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CShutoffValve *pHMShutoffValve )
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

void CTabDlgSpecActTesting::_WriteSmartControlValveInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CSmartControlValve *pHMSmartControlValve )
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

bool CTabDlgSpecActTesting::_ProcessHydroMod( CString *pRetStr )
{
	if( m_arStr.GetSize() < 2 )
	{
		*pRetStr = _T("Missing arguments");
		return false;
	}
	
	if( 0 == m_arStr[1].Compare( _T("UNFREEZE") ) )
	{
		pMainFrame->Freeze(false);
		return true;
	}
	else if( 0 == m_arStr[1].Compare( _T("FREEZE") ) )
	{
		pMainFrame->Freeze(true);
		return true;
	}
	else if( 0 == m_arStr[1].Compare( _T("HMNAME") ) )
	{
		if( m_arStr.GetSize() < 3 )
		{
			*pRetStr = _T("Missing arguments");
			return false;
		}

		CTableHM *pTab = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );

		if( NULL == pTab )
		{
			return false;
		}

		CDS_HydroMod *pHM = TASApp.GetpTADS()->FindHydroMod( m_arStr[2], pTab, NULL, CDatastruct::Both );

		if( NULL == pHM )
		{
			return false;
		}

		if( 0 == m_arStr[3].Compare( _T("CHECKVALUE") ) )
		{
			if( m_arStr.GetSize() < 7 )
			{
				*pRetStr = _T("Missing arguments");
				return false;
			}
			
			if( 0 == m_arStr[4].Compare( _T("HM") ) )
			{
				if( 0 == m_arStr[5].Compare( _T("DP") ) )
				{
					return _ProcessCheckValue( pRetStr, pHM->GetDp() );
				}
				else if( 0 == m_arStr[5].Compare( _T("FLOW") ) )
				{
					double d = CDimValue::SItoCU( _U_FLOW, pHM->GetQ() );
					return _ProcessCheckValue( pRetStr, d );
				}
				else if( 0 == m_arStr[5].Compare( _T("SECFLOW") ) )
				{
					double d = CDimValue::SItoCU( _U_FLOW, pHM->GetpTermUnit()->GetQ() );
					return _ProcessCheckValue( pRetStr, d );
				}
			}
			else if( 0 == m_arStr[4].Compare( _T("CV") ) )
			{
				if( NULL == pHM->GetpCV() )
				{
					*pRetStr = _T("CV doesn't exist");
					return false;
				}

				if( 0 == m_arStr[5].Compare( _T("DP") ) )
				{
					return _ProcessCheckValue( pRetStr, pHM->GetpCV()->GetDp() );
				}

				if( 0 == m_arStr[5].Compare(_T ("DPMIN") ) )
				{
					return _ProcessCheckValue( pRetStr, pHM->GetpCV()->GetDpMin() );
				}
				else if( 0 == m_arStr[5].Compare( _T("FLOW") ) )
				{
					double d = CDimValue::SItoCU( _U_FLOW, pHM->GetpCV()->GetQ() );
					return _ProcessCheckValue( pRetStr, d );
				}
				else if( 0 == m_arStr[5].Compare( _T("BETAMIN") ) )
				{
					return _ProcessCheckValue( pRetStr, pHM->GetpCV()->GetAuth() );
				}
				else if( 0 == m_arStr[5].Compare( _T("BETADESIGN") ) )
				{
					return _ProcessCheckValue( pRetStr, pHM->GetpCV()->GetAuth(true) );
				}
				else if( 0 == m_arStr[5].Compare( _T("PRESET") ) )
				{
					CString strPreset = pHM->GetpCV()->GetSettingStr();
					strPreset.Replace( _T("," ), _T( ".") );
					double dPreset = _ttof( strPreset );
					return _ProcessCheckValue( pRetStr, dPreset );
				}
				else if( 0 == m_arStr[5].Compare( _T("KVS") ) )
				{
					return _ProcessCheckValue( pRetStr, pHM->GetpCV()->GetKvs() );
				}
				else if( 0 == m_arStr[5].Compare( _T("KVSMAX") ) )
				{
					return _ProcessCheckValue( pRetStr, pHM->GetpCV()->GetKvsmax() );
				}
				else if( 0 == m_arStr[5].Compare( _T("CVNAME") ) )
				{
					CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct*>(pHM->GetpCV()->GetCvIDPtr().MP );

					if( NULL == pTAP )
					{
						*pRetStr = _T("CV-NAME doesn't exist");
						return false;
					}
					
					if( 0 == m_arStr[6].CompareNoCase( pTAP->GetName() ) )
					{
						return true;
					}
					else
					{
						*pRetStr = _T("CV-NAME doesn't match");
						return false;
					}
				}
				else if( 0 == m_arStr[5].Compare( _T("CVACTR") ) )
				{
					CDB_Actuator *pActr = dynamic_cast<CDB_Actuator*>(pHM->GetpCV()->GetActrIDPtr().MP );
					
					if( NULL == pActr )
					{
						*pRetStr = _T("Actuator doesn't exist");
						return false;
					}
					
					if( 0 == m_arStr[6].CompareNoCase( pActr->GetName() ) )
					{
						return true;
					}
					else
					{
						*pRetStr = _T("Actuator NAME doesn't match");
						return false;
					}
				}

			}
			else if( 0 == m_arStr[4].Compare( _T("BVP") ) )
			{
				if( NULL == pHM->GetpBv() )
				{
					*pRetStr = _T("BVP doesn't exist");
					return false;
				}
				
				if( 0 == m_arStr[5].Compare( _T("DP") ) )
				{
					return _ProcessCheckValue( pRetStr, pHM->GetpBv()->GetDp() );
				}
				else if( 0 == m_arStr[5].Compare( _T("FLOW") ) )
				{
					double d = CDimValue::SItoCU( _U_FLOW, pHM->GetpBv()->GetQ() );
					return _ProcessCheckValue( pRetStr, d );
				}
				else if( 0 == m_arStr[5].Compare( _T("PRESET") ) )
				{
					CString strPreset = pHM->GetpBv()->GetSettingStr();
					strPreset.Replace( _T("," ), _T( ".") );
					double dPreset = _ttof( strPreset );
					return _ProcessCheckValue( pRetStr, dPreset );
				}
				else if( 0 == m_arStr[5].Compare( _T("BVNAME") ) )
				{
					CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct*>(pHM->GetpBv()->GetIDPtr().MP );
					
					if( NULL == pTAP )
					{
						*pRetStr = _T("BV-NAME doesn't exist");
						return false;
					}

					if( 0 == m_arStr[6].CompareNoCase( pTAP->GetName() ) )
					{
						return true;
					}
					else
					{
						*pRetStr = _T("BV-NAME doesn't match");
						return false;
					}
				}

			}
			else if( 0 == m_arStr[4].Compare( _T("BVB") ) )
			{
				if( NULL == pHM->GetpBypBv() )
				{
					*pRetStr = _T("BVB doesn't exist");
					return false;
				}

				if( 0 == m_arStr[5].Compare( _T("DP") ) )
				{
					return _ProcessCheckValue( pRetStr, pHM->GetpBypBv()->GetDp() );
				}
				else if( 0 == m_arStr[5].Compare( _T("FLOW") ) )
				{
					double d = CDimValue::SItoCU( _U_FLOW, pHM->GetpBypBv()->GetQ() );
					return _ProcessCheckValue( pRetStr, d );
				}
				else if( 0 == m_arStr[5].Compare( _T("PRESET") ) )
				{
					CString strPreset = pHM->GetpBypBv()->GetSettingStr();
					strPreset.Replace( _T("," ), _T( ".") );
					double dPreset = _ttof( strPreset );
					return _ProcessCheckValue( pRetStr, dPreset );
				}
			}
			else if( 0 == m_arStr[4].Compare( _T("BVS") ) )
			{
				if( NULL == pHM->GetpSecBv() )
				{
					*pRetStr = _T("BVS doesn't exist");
					return false;
				}

				if( 0 == m_arStr[5].Compare( _T("DP") ) )
				{
					return _ProcessCheckValue( pRetStr, pHM->GetpSecBv()->GetDp() );
				}
				else if( 0 == m_arStr[5].Compare( _T("FLOW") ) )
				{
					double d = CDimValue::SItoCU( _U_FLOW, pHM->GetpSecBv()->GetQ() );
					return _ProcessCheckValue( pRetStr, d );
				}
				else if( 0 == m_arStr[5].Compare( _T("PRESET") ) )
				{
					CString strPreset = pHM->GetpSecBv()->GetSettingStr();
					strPreset.Replace( _T("," ), _T( ".") );
					double dPreset = _ttof( strPreset );
					return _ProcessCheckValue( pRetStr, dPreset );
				}
			}
			else if( 0 == m_arStr[4].Compare( _T("DPC") ) )
			{
				if( NULL == pHM->GetpDpC() )
				{
					*pRetStr = _T("DpC doesn't exist");
					return false;
				}

				if( 0 == m_arStr[5].Compare( _T("DP") ) )
				{
					return _ProcessCheckValue( pRetStr, pHM->GetpDpC()->GetDp() );
				}
				else if( 0 == m_arStr[5].Compare( _T("FLOW") ) )
				{
					double d = CDimValue::SItoCU( _U_FLOW, pHM->GetpDpC()->GetQ() );
					return _ProcessCheckValue( pRetStr, d );
				}
				else if( 0 == m_arStr[5].Compare( _T("PRESET") ) )
				{
					CString strPreset = pHM->GetpDpC()->GetSettingStr();
					strPreset.Replace( _T("," ), _T( ".") );
					double dPreset = _ttof( strPreset );
					return _ProcessCheckValue( pRetStr, dPreset );
				}
			}

			if( 0 == m_arStr[4].Compare( _T("CIRCPIPE") ) )
			{
				if( 0 == m_arStr[5].Compare( _T("DP") ) )
				{
					CPipes *pPipe = pHM->GetpCircuitPrimaryPipe();
					
					if( NULL == pPipe )
					{
						*pRetStr = _T("Circuit pipe doesn't exist");
						return false;
					}

					return _ProcessCheckValue( pRetStr, pPipe->GetPipeDp() );
				}
			}

			if( 0 == m_arStr[4].Compare( _T("SECCIRCPIPE") ) )
			{
				if( 0 == m_arStr[5].Compare( _T("DP") ) )
				{
					CPipes *pPipe = pHM->GetpCircuitSecondaryPipe();
					
					if( NULL == pPipe )
					{
						*pRetStr = _T("Secondary circuit pipe doesn't exist");
						return false;
					}

					return _ProcessCheckValue( pRetStr, pPipe->GetPipeDp() );
				}
			}

			if( 0 == m_arStr[4].Compare( _T("DISTSUPPIPE") ) )
			{
				if( 0 == m_arStr[5].Compare( _T("DP") ) )
				{
					CPipes *pPipe = pHM->GetpDistrSupplyPipe();
					
					if( NULL == pPipe )
					{
						*pRetStr = _T("Distribution supply pipe doesn't exist");
						return false;
					}
					
					return _ProcessCheckValue( pRetStr, pPipe->GetPipeDp() );
				}
			}

			if( 0 == m_arStr[4].Compare( _T("DISTRETPIPE") ) )
			{
				if( 0 == m_arStr[5].Compare( _T("DP") ) )
				{
					CPipes *pPipe = pHM->GetpDistrReturnPipe();
					
					if( NULL == pPipe )
					{
						*pRetStr = _T("Distribution return pipe doesn't exist");
						return false;
					}

					return _ProcessCheckValue( pRetStr, pPipe->GetPipeDp() );
				}
			}

			if( 0 == m_arStr[4].Compare( _T("CIRCACC") ) )
			{
				if( 0 == m_arStr[5].Compare( _T("DP") ) )
				{
					CPipes *pPipe = pHM->GetpCircuitPrimaryPipe();
					
					if( NULL == pPipe )
					{
						*pRetStr = _T("Circuit pipe doesn't exist");
						return false;
					}

					return _ProcessCheckValue( pRetStr, pPipe->GetSingularityTotalDp(true) );
				}
			}

			if( 0 == m_arStr[4].Compare( _T("SECCIRCACC") ) )
			{
				if( 0 == m_arStr[5].Compare( _T("DP") ) )
				{
					CPipes *pPipe = pHM->GetpCircuitSecondaryPipe();
					
					if( NULL == pPipe )
					{
						*pRetStr = _T("Secondary circuit pipe doesn't exist");
						return false;
					}

					return _ProcessCheckValue( pRetStr, pPipe->GetSingularityTotalDp(true) );
				}
			}

			if( 0 == m_arStr[4].Compare( _T("DISTSUPACC") ) )
			{
				if( 0 == m_arStr[5].Compare( _T("DP") ) )
				{
					CPipes *pPipe = pHM->GetpDistrSupplyPipe();

					if( NULL == pPipe )
					{
						*pRetStr = _T("Distribution supply pipe doesn't exist");
						return false;
					}

					return _ProcessCheckValue( pRetStr, pPipe->GetSingularityTotalDp(true) );
				}
			}
			
			if( 0 == m_arStr[4].Compare( _T("DISTRETPIPE") ) )
			{
				if( 0 == m_arStr[5].Compare( _T("DP") ) )
				{
					CPipes *pPipe = pHM->GetpDistrReturnPipe();
					
					if( NULL == pPipe )
					{
						*pRetStr = _T("Distribution return pipe doesn't exist");
						return false;
					}
					
					return _ProcessCheckValue( pRetStr, pPipe->GetSingularityTotalDp(true) );
				}
			}

		}

		return true;
	}

	*pRetStr = _T("Unknown command");

	return false;
}

bool CTabDlgSpecActTesting::_ProcessCheckValue( CString *pRetStr, double dHMValue )
{
	double dVal;

	if( RD_OK != ReadDouble( m_arStr[6], &dVal ) )
	{
		*pRetStr = _T("Invalid number");
		return false;
	}

	if( 0 == m_arStr[5].Compare( _T("DP") ) )
	{
		double dErr = fabs( dHMValue - dVal );

		// 1 Pascal
		if( dErr > 1 )
		{
			*pRetStr = _T("Dp value doesn't match");
			return false;
		}
	}
	else if( 0 == m_arStr[5].Compare( _T("FLOW") ) )
	{
		double dErr = fabs( dHMValue - dVal );

		// l/h
		if( dErr > 1 )
		{
			*pRetStr = _T("Flow value doesn't match");
			return false;
		}
	}
	else if( 0 == m_arStr[5].Compare( _T("SECFLOW") ) )
	{
		double dErr = fabs( dHMValue - dVal );
		
		// l/h
		if( dErr > 1 )
		{
			*pRetStr = _T("Secondary flow value doesn't match");
			return false;
		}
	}
	else if( 0 == m_arStr[5].Compare( _T("BETAMIN") ) )
	{
		double dErr = fabs( dHMValue - dVal );
		
		if( dErr > 0.01 )
		{
			*pRetStr = _T("BetaMin value doesn't match");
			return false;
		}
	}
	else if( 0 == m_arStr[5].Compare( _T("PRESET") ) )
	{
		double dErr = fabs( dHMValue - dVal );
		
		if( dErr > 0.01 )
		{
			*pRetStr = _T("Presetting value doesn't match");
			return false;
		}
	}
	else if( 0 == m_arStr[5].Compare( _T("KVS") ) )
	{
		double dErr = fabs( dHMValue - dVal );
		
		if( dErr > 0.1 )
		{
			*pRetStr = _T("Kvs value doesn't match");
			return false;
		}
	}
	else if( 0 == m_arStr[5].Compare( _T("KVSMAX") ) )
	{
		double dErr = fabs( dHMValue - dVal );
		
		if( dErr > 0.1 )
		{
			*pRetStr = _T("KvsMax value doesn't match");
			return false;
		}
	}
	
	return true;
}

