#include "stdafx.h"
#include <psapi.h>


#include <ctime>
#include <vector>
#include "MainFrm.h"
#include "TASelect.h"
#include "Database.h"
#include "DataBObj.h"
#include "DataStruct.h"
#include "utilities.h"
#include "HydroMod.h"
#include "SelectPM.h"
#include "EnBitmapPatchWork.h"
#include "DlgHydronicSchView.h"
#include "TabDlgSpecActDev.h"
#include "SSheetPanelCirc2.h"
#include "DlgDebugSearchDBObject.h"
#include "DlgSpecActHyToolsPMCompare.h"
#include "DlgSpecActReformatFilterTabFiles.h"
#include "DlgSpecActComputeCRC.h"
#include "DlgSpecialActionExtractArticleWithoutPrice.h"

#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

CTabDlgSpecActDev::CTabDlgSpecActDev( CWnd* pParent )
	: CDialogExt( CTabDlgSpecActDev::IDD, pParent )
{
	SetBackgroundColor( _WHITE_DLGBOX );

	m_vecComboInfos.push_back( CComboInfo( CString( _T("Extract pipe series") ), &CTabDlgSpecActDev::ComboFctExtractPipeSeries ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("Test hydronic schemes") ), &CTabDlgSpecActDev::ComboFctTestHydronicScheme ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("Generate TAP-txt") ), &CTabDlgSpecActDev::ComboFctGenerateTAPTxt ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("Extract all article numbers") ), &CTabDlgSpecActDev::ComboFctExtractAllArticleNumbers ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("Extract PM in SQL Lite DB") ), &CTabDlgSpecActDev::ComboFctPMExtractToSQL ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("Show 'Hydronic schematic view' dialog") ), &CTabDlgSpecActDev::ComboFctShowHydronicSchView ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("Search database object") ), &CTabDlgSpecActDev::ComboFctButtonSearchDBObject ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("HyTools PM comparaison") ), &CTabDlgSpecActDev::ComboFctHyToolsPMCompare ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("Rearrange 'Filter tabs' file") ), &CTabDlgSpecActDev::ComboFctReformatFilterTabFiles ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("Extract all PM products") ), &CTabDlgSpecActDev::ComboFctExtractAllPMProduct ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("Mark all products as deleted") ), &CTabDlgSpecActDev::ComboFctMarkAllProductAsDeleted ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("Mark all products as undeleted") ), &CTabDlgSpecActDev::ComboFctMarkAllProductAsUndeleted ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("Test 6-way valve selection") ), &CTabDlgSpecActDev::ComboFctTest6WayValveSelection ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("Test STAV valve selection") ), &CTabDlgSpecActDev::ComboFctTestSTAVSelection ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("Compute CRC on a file") ), &CTabDlgSpecActDev::ComboFctComputeCRC ) );
	m_vecComboInfos.push_back( CComboInfo( CString( _T("Extract article without price") ), &CTabDlgSpecActDev::ComboFctExtractArticleWithoutPrice ) );
}

BEGIN_MESSAGE_MAP( CTabDlgSpecActDev, CDialogExt )
	ON_BN_CLICKED( IDC_BUTTONRUNFCT, &OnBnClickedRun )
	ON_BN_CLICKED( IDC_BUTTONCLEAR, &OnBnClickedClear )
END_MESSAGE_MAP()

void CTabDlgSpecActDev::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOFCT, m_clComboFunctions );
	DDX_Control( pDX, IDC_LISTOUTPUT, m_List );
	DDX_Control( pDX, IDC_BUTTONCLEAR, m_clButtonClear );
}

BOOL CTabDlgSpecActDev::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	int iCount = 0;
	for( auto &iter : m_vecComboInfos )
	{
		int iPos = m_clComboFunctions.AddString( iter.m_strComboText );
		m_clComboFunctions.SetItemData( iPos, iCount++ );
	}

	m_clComboFunctions.SetCurSel( 0 );
	m_List.ModifyStyle( LBS_SORT, 0 );

	return TRUE;
}

void CTabDlgSpecActDev::OnBnClickedRun()
{
	int iPos = m_clComboFunctions.GetCurSel();
	(this->*(m_vecComboInfos.at( m_clComboFunctions.GetItemData( iPos ) ).m_pFunction))();
}

void CTabDlgSpecActDev::OnBnClickedClear()
{
	m_List.ResetContent();
}

void CTabDlgSpecActDev::ComboFctExtractPipeSeries()
{
	CString str = _T("Exported file name (*.txt)|*.txt||");
	CFileDialog dlg( false, _T("txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXTENSIONDIFFERENT, (LPCTSTR)str, NULL );
	CString PrjDir = GetProjectDirectory();

	dlg.m_ofn.lpstrInitialDir = (LPCTSTR)PrjDir;

	if( IDOK == dlg.DoModal() )
	{
		BeginWaitCursor();
	
		// Switch in CBXMode.
		CFileTxt outf;
		outf.Open( dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite );
		
		// Unicode.
		BYTE tmpbuf[] = { 0xff, 0xfe };
		outf.Write( tmpbuf, sizeof( tmpbuf ) );

		_ExtractPipeSeries( &outf );

		_ExtractPipes( &outf );

		outf.Close();

		CString str = dlg.GetPathName() + CString( _T("\r\nCreated") ); 

		EndWaitCursor();

		AfxMessageBox( str, MB_OK | MB_ICONEXCLAMATION, 0 );
	}
}

void CTabDlgSpecActDev::ComboFctTestHydronicScheme()
{
	CSSheetPanelCirc2 *pSPC2 = new CSSheetPanelCirc2;
	IDPTR idptr;

	if( false == pSPC2->TestHydronicSchemesBoxPosition( idptr ) )
	{
		CString str;
		str.Format( _T("Fail on %s"), idptr.ID );
		AfxMessageBox( str );
	}
	
	delete pSPC2;
}

void CTabDlgSpecActDev::ComboFctGenerateTAPTxt()
{
//	_ExtractKvVsOpeningForMVValves();
	_ExtractFlowAndDpmin();
//	_ExtractKvVsOpeningForTAFUSIONC();
}

void CTabDlgSpecActDev::ComboFctExtractAllArticleNumbers()
{
	// TASApp.GetpTADB()->ExportAllArticleNumber( _T("c:\\temp\\artnbr.txt") );
	_ExtractAllArticleNumbersAndVVS();
}

void CTabDlgSpecActDev::ComboFctPMExtractToSQL()
{
	sqlite3 *pDb;
	int iReturnCode;
	CString str;

/*	CTime t = CTime::GetCurrentTime();
	CStringA DbFN = (CStringA)t.Format( L"%Y%m%d-%H%M%S" );
	DbFN = DbFN + "PMTable.db";
*/

	iReturnCode = sqlite3_open( "HyAppDB.sqlite", &pDb );

	if( 0 != iReturnCode ) 
	{
		str.Format( _T("%d;Can't open database: %s"), stderr, sqlite3_errmsg( pDb ) );
		AfxMessageBox( str, MB_OK | MB_ICONSTOP );
	}
	else
	{
		iReturnCode = _ExtractVessels(pDb); 
		iReturnCode = _ExtractTecBoxes(pDb);
		sqlite3_close( pDb );
	}

	AfxMessageBox( _T("SQLite Pressurisation DB updated"), MB_OK );
}

void CTabDlgSpecActDev::ComboFctShowHydronicSchView()
{
	CDlgHydronicSchView clDlgHydronicSchView;
	clDlgHydronicSchView.DoModal();
}

void CTabDlgSpecActDev::ComboFctButtonSearchDBObject()
{
	CDlgDebugSearchDBObject clDebugSearchObject;
	clDebugSearchObject.DoModal();
}

void CTabDlgSpecActDev::ComboFctHyToolsPMCompare()
{
	CDlgSpecActHyToolsPMCompare clDlgSpecActHyToolsPMCompare;
	clDlgSpecActHyToolsPMCompare.DoModal();
}

void CTabDlgSpecActDev::ComboFctReformatFilterTabFiles()
{
	CDlgSpecActReformatFilterTabFiles clDlgSpecActReformatFilterTabFiles;
	clDlgSpecActReformatFilterTabFiles.DoModal();
}

void CTabDlgSpecActDev::ComboFctExtractAllPMProduct()
{
	m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), _T("Extract is starting...") ) );

	std::vector<CString> vecTables;

	vecTables.push_back( _T("VSSL_TAB" ) );
	vecTables.push_back( _T("TECBOX_TAB" ) );
	vecTables.push_back( _T("PLENOREFILL_TAB" ) );
	vecTables.push_back( _T("AIRVENT_TAB" ) );
	vecTables.push_back( _T("SEPARATOR_TAB" ) );
	
	std::map<CString, CDB_Product*> mapAllObjects;

	for( int iLoop = 0; iLoop < (int)vecTables.size(); iLoop++ )
	{
		IDPTR TableIDPtr = TASApp.GetpTADB()->Get( vecTables[iLoop] );

		if( NULL == TableIDPtr.MP )
		{
			CString str;
			str.Format( _T("Table %s not in database!!\n"), vecTables[iLoop] );
			OutputDebugString( str );
			ASSERT( 0 );
			continue;
		}

		CTable *pTable = (CTable*)( TableIDPtr.MP );

		for( IDPTR ProductIDPtr = pTable->GetFirst(); _T('\0') != *ProductIDPtr.ID; ProductIDPtr = pTable->GetNext( ProductIDPtr.MP ) )
		{
			CData *pclData = dynamic_cast<CData*>( ProductIDPtr.MP );

			if( NULL == pclData || true == pclData->IsDeleted() )
			{
				continue;
			}

			CDB_Product *pclProduct = dynamic_cast<CDB_Product*>( ProductIDPtr.MP );

			if( NULL == pclProduct )
			{
				ASSERT( 0 );
				continue;
			}

			mapAllObjects.insert( std::pair<CString, CDB_Product*>( pclProduct->GetArtNum(), pclProduct ) );

			// Check accessories.
			if( _NULL_IDPTR != pclProduct->GetAccessoriesGroupIDPtr() )
			{
				CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclProduct->GetAccessoriesGroupIDPtr().MP );

				if( NULL == pclRuledTable )
				{
					ASSERT( 0 );
					continue;
				}

				for( IDPTR AccessoryIDPtr = pclRuledTable->GetFirst(); _T('\0') != *AccessoryIDPtr.ID; AccessoryIDPtr = pclRuledTable->GetNext() )
				{
					CDB_Product *pActAcc = dynamic_cast<CDB_Product *>( AccessoryIDPtr.MP );

					if( NULL == pActAcc || false == pActAcc->IsAnAccessory() )
					{
						ASSERT( 0 );
						continue;
					}

					if( 0 == mapAllObjects.count( pActAcc->GetArtNum() ) )
					{
						mapAllObjects.insert( std::pair<CString, CDB_Product*>( pActAcc->GetArtNum(), pActAcc ) );
					}
				}
			}
		}
	}

	FILE *pFile = _wfopen( _T("c:\\temp\\PMproduct_list.txt" ), _T("wt") );
	fwprintf( pFile, _T("Article number;Name;Availability;Hidden\n" ) );

	for( auto &iter : mapAllObjects )
	{
		CString strArticleNumber = iter.second->GetArtNum();
		CString strName = iter.second->GetName();
		int iAvailability = ( true == iter.second->IsAvailable() ) ? 1 : 0;
		int iHidden = ( true == iter.second->IsHidden() ) ? 1 : 0;

		fwprintf( pFile, _T("%s;%s;%i;%i\n"), (LPCTSTR)strArticleNumber, (LPCTSTR)strName, iAvailability, iHidden );
	}

	fclose( pFile );

	m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), _T("Extract finished.") ) );
}

void CTabDlgSpecActDev::ComboFctMarkAllProductAsDeleted()
{
	CDataList* pclDataList = TASApp.GetpTADB()->GetpDataList();

	IDPTR ObjectIDPtr = pclDataList->GetFirst();

	while( _NULL_IDPTR != ObjectIDPtr )
	{
		((CData*)ObjectIDPtr.MP)->DeleteArticle();
		ObjectIDPtr = pclDataList->GetNext();
	}

	m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), _T("All objects are marked as deleted.") ) );
}

void CTabDlgSpecActDev::ComboFctMarkAllProductAsUndeleted()
{
	CDataList* pclDataList = TASApp.GetpTADB()->GetpDataList();

	IDPTR ObjectIDPtr = pclDataList->GetFirst();

	while( _NULL_IDPTR != ObjectIDPtr )
	{
		((CData*)ObjectIDPtr.MP)->SetDeleted( false );
		ObjectIDPtr = pclDataList->GetNext();
	}

	m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), _T("All objects are marked as undeleted.") ) );
}

void CTabDlgSpecActDev::ComboFctTest6WayValveSelection()
{
	CIndSel6WayValveParams clIndSel6WayValveParams( TASApp.GetpTADB(), TASApp.GetpTADS(), TASApp.GetpUserDB(), TASApp.GetpPipeDB(), TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData() );
	clIndSel6WayValveParams.m_strPipeSeriesID = pMainFrame->GetRibbonComboSelPipeSeriesID();
	clIndSel6WayValveParams.m_strPipeID = pMainFrame->GetRibbonComboSelPipeSizeID();
	clIndSel6WayValveParams.m_dFlow = 150.0 / 3600000.0;
	clIndSel6WayValveParams.m_WC.SetTemp( 75.0 );
	clIndSel6WayValveParams.m_dCoolingFlow = 600.0 / 3600000.0;
	clIndSel6WayValveParams.m_CoolingWC.SetTemp( 6.0 );
	clIndSel6WayValveParams.m_e6WayValveSelectionMode = e6Way_EQMControl;

	clIndSel6WayValveParams.m_e6WayValveList.PurgeAll();

	clIndSel6WayValveParams.m_pTADB->Get6WayValveList( &clIndSel6WayValveParams.m_e6WayValveList, (LPCTSTR)clIndSel6WayValveParams.m_strComboFamilyID, 
			(LPCTSTR)clIndSel6WayValveParams.m_strComboVersionID, clIndSel6WayValveParams.m_eFilterSelection );

	if( NULL != clIndSel6WayValveParams.m_pclSelect6WayValveList )
	{
		delete clIndSel6WayValveParams.m_pclSelect6WayValveList;
	}

	clIndSel6WayValveParams.m_pclSelect6WayValveList = new CSelect6WayValveList();

	if( NULL == clIndSel6WayValveParams.m_pclSelect6WayValveList || NULL == clIndSel6WayValveParams.m_pclSelect6WayValveList->GetSelectPipeList() )
	{
		return;
	}

	double dBiggestFlow = max( clIndSel6WayValveParams.m_dFlow, clIndSel6WayValveParams.m_dCoolingFlow );
	clIndSel6WayValveParams.m_pclSelect6WayValveList->GetSelectPipeList()->SelectPipes( &clIndSel6WayValveParams, dBiggestFlow );

	bool bSizeShiftProblem = false;
	int iDevFound = clIndSel6WayValveParams.m_pclSelect6WayValveList->Select6WayValve( &clIndSel6WayValveParams, &bSizeShiftProblem );

	if( 0 == iDevFound )
	{
		return;
	}

	CSelectedValve *pclBestSelected6WayValve = NULL;

	for( CSelectedValve *pclSelectedValve = clIndSel6WayValveParams.m_pclSelect6WayValveList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
			pclSelectedValve = clIndSel6WayValveParams.m_pclSelect6WayValveList->GetNext<CSelectedValve>() )
	{
		if( true == pclSelectedValve->GetFlag( CSelectedBase::eBest ) )
		{
			pclBestSelected6WayValve = pclSelectedValve;
			break;
		}
	}

	if( NULL == pclBestSelected6WayValve )
	{
		return;
	}

	iDevFound = clIndSel6WayValveParams.m_pclSelect6WayValveList->SelectPIBCValve( &clIndSel6WayValveParams, pclBestSelected6WayValve, BothSide, &bSizeShiftProblem );

	if( 0 == iDevFound )
	{
		return;
	}

	CSelectPICVList *pclSelectPICVList = clIndSel6WayValveParams.m_pclSelect6WayValveList->GetPICVList( BothSide );

	if( NULL == pclSelectPICVList )
	{
		ASSERT_RETURN;
	}

	CSelectedValve *pclBestSelectedPIBCVValve = NULL;

	for( CSelectedValve *pclSelectedValve = pclSelectPICVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
			pclSelectedValve = pclSelectPICVList->GetNext<CSelectedValve>() )
	{
		if( true == pclSelectedValve->GetFlag( CSelectedBase::eBest ) )
		{
			pclBestSelectedPIBCVValve = pclSelectedValve;
			break;
		}
	}
}

void CTabDlgSpecActDev::ComboFctTestSTAVSelection()
{
	CIndSelBVParams clIndSelBVParams( TASApp.GetpTADB(), TASApp.GetpTADS(), TASApp.GetpUserDB(), TASApp.GetpPipeDB(), TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData() );
	clIndSelBVParams.m_strPipeSeriesID = pMainFrame->GetRibbonComboSelPipeSeriesID();
	clIndSelBVParams.m_strPipeID = pMainFrame->GetRibbonComboSelPipeSizeID();
	clIndSelBVParams.m_dFlow = 0.0;
	clIndSelBVParams.m_dDp = 6000.0;
	clIndSelBVParams.m_WC.SetTemp( 75.0 );
	
	clIndSelBVParams.m_strComboTypeID = _T("RVTYPE_VV");
	clIndSelBVParams.m_BvList.PurgeAll();
	clIndSelBVParams.m_pTADB->GetBVList( &clIndSelBVParams.m_BvList, _T("RVTYPE_VV"), _T("") , _T(""), _T(""), _T("") );

	CBatchSelBVParams clBatchSelBVParams( TASApp.GetpTADB(), TASApp.GetpTADS(), TASApp.GetpUserDB(), TASApp.GetpPipeDB(), TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData() );
	clBatchSelBVParams.m_strPipeSeriesID = pMainFrame->GetRibbonComboSelPipeSeriesID();
	clBatchSelBVParams.m_strPipeID = pMainFrame->GetRibbonComboSelPipeSizeID();
	clBatchSelBVParams.m_WC.SetTemp( 75.0 );
	CDlgBatchSelBase::BSRowParameters *pclRowParameters = new CDlgBatchSelBase::BSRowParameters();

	if( NULL == pclRowParameters )
	{
		return;
	}
			
	pclRowParameters->m_dDp = 6000.0;

	clBatchSelBVParams.m_strComboTypeBelow65ID = _T("RVTYPE_VV");
	clBatchSelBVParams.m_strComboPNBelow65ID = _T("PN_4");
	clBatchSelBVParams.m_strComboTypeAbove50ID = _T("RVTYPE_VV");
	clBatchSelBVParams.m_strComboPNAbove50ID = _T("PN_4");

	m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), _T("Starting...") ) );
	FILE *pFile = _tfopen( _T("c:\\temp\\STAV_ind_sel.txt"), _T("w") );
	double dFlow = 0.01; // l/s

	for( int i = 0; i < 2000; i++ )
	{
		// Individual selection.
		if( NULL != clIndSelBVParams.m_pclSelectBvList )
		{
			delete clIndSelBVParams.m_pclSelectBvList;
			clIndSelBVParams.m_pclSelectBvList = NULL;
		}

		clIndSelBVParams.m_pclSelectBvList = new CSelectList();

		if( NULL == clIndSelBVParams.m_pclSelectBvList || NULL == clIndSelBVParams.m_pclSelectBvList->GetSelectPipeList() )
		{
			return;
		}

		double dFlowSI = dFlow / 1000.0;
		clIndSelBVParams.m_dFlow = dFlowSI;
		clIndSelBVParams.m_pclSelectBvList->GetSelectPipeList()->SelectPipes( &clIndSelBVParams, dFlowSI );

		bool bSizeShiftProblem = false;
		int iDevFound = clIndSelBVParams.m_pclSelectBvList->GetManBvList( &clIndSelBVParams );
		
		bool bByPass = false;

		if( 0 == iDevFound )
		{
			fwprintf( pFile, _T("Flow: %f; Not found"), dFlow );
			bByPass = true;
		}

		CSelectedValve *pclBestSelectedSTAV = NULL;
		if( false == bByPass )
		{
			for( CSelectedValve *pclSelectedValve = clIndSelBVParams.m_pclSelectBvList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
					pclSelectedValve = clIndSelBVParams.m_pclSelectBvList->GetNext<CSelectedValve>() )
			{
				if( true == pclSelectedValve->GetFlag( CSelectedBase::eBest ) )
				{
					pclBestSelectedSTAV = pclSelectedValve;
					break;
				}
			}
		}

		bByPass = false;
		
		if( NULL == pclBestSelectedSTAV )
		{
			fwprintf( pFile, _T("Flow: %f; Not found"), dFlow );
			bByPass = true;
		}
		
		if( false == bByPass )
		{
			CDB_VenturiValve *pclBest = (CDB_VenturiValve *)pclBestSelectedSTAV->GetpData();
			CString strSetting = _T("-");
			
			if( NULL != pclBest->GetValveCharacteristic() && -1.0 != pclBestSelectedSTAV->GetH() )
			{
				strSetting = pclBest->GetValveCharacteristic()->GetSettingString( pclBestSelectedSTAV->GetH() );
			}

			_ftprintf( pFile, _T("Flow: %f; %s; %s"), dFlow, pclBest->GetName(), (LPCTSTR)strSetting );
		}

		// Batch selection.
		if( NULL != clBatchSelBVParams.m_pclBatchBVList )
		{
			delete clBatchSelBVParams.m_pclBatchBVList;
			clBatchSelBVParams.m_pclBatchBVList = NULL;
		}

		clBatchSelBVParams.m_pclBatchBVList = new CBatchSelectBvList();

		if( NULL == clBatchSelBVParams.m_pclBatchBVList || NULL == clBatchSelBVParams.m_pclBatchBVList->GetSelectPipeList() )
		{
			return;
		}

		pclRowParameters->m_pclBatchResults = clBatchSelBVParams.m_pclBatchBVList;
		pclRowParameters->m_dFlow = dFlowSI;


		pclRowParameters->m_pclSelectedProduct = NULL;
		int iReturnCode = clBatchSelBVParams.m_pclBatchBVList->SelectBv( &clBatchSelBVParams, pclRowParameters->m_dFlow, pclRowParameters->m_dDp );

		bool bAlternative = ( CBatchSelectBaseList::BR_FoundAlernative == ( iReturnCode & CBatchSelectBaseList::BR_FoundAlernative ) );

		if( ( CBatchSelectBaseList::BR_BadArguments == ( iReturnCode & CBatchSelectBaseList::BR_BadArguments ) ) 
				|| ( CBatchSelectBaseList::BR_NoPipeFound == ( iReturnCode & CBatchSelectBaseList::BR_NoPipeFound ) ) 
				|| ( CBatchSelectBaseList::BR_NotFound == ( iReturnCode & CBatchSelectBaseList::BR_NotFound ) ) )
		{
			pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindNoSolution;
		}
		else if( CBatchSelectBaseList::BR_FoundOneBest == ( iReturnCode & CBatchSelectBaseList::BR_FoundOneBest ) )
		{
			pclRowParameters->m_pclSelectedProduct = clBatchSelBVParams.m_pclBatchBVList->GetBestProduct();

			if( false == bAlternative )
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
			}
			else
			{
				pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
			}
		}
		else if( CBatchSelectBaseList::BR_FoundButNotBest == ( iReturnCode & CBatchSelectBaseList::BR_FoundButNotBest ) )
		{
			// Verify if there is only one product. In that case, we take this one by default.
			if( 1 == clBatchSelBVParams.m_pclBatchBVList->GetCount() )
			{
				pclRowParameters->m_pclSelectedProduct = clBatchSelBVParams.m_pclBatchBVList->GetFirst<CSelectedBase>();

				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolution;
				}
				else
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindOneSolutionAlter;
				}
			}
			else
			{
				if( false == bAlternative )
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutions;
				}
				else
				{
					pclRowParameters->m_eStatus = CDlgBatchSelBase::BS_RS_FindSolutionsAlter;
				}
			}
		}

		if( CDlgBatchSelBase::BS_RS_FindSolutions == pclRowParameters->m_eStatus )
		{
			fwprintf( pFile, _T("; Found more that one solution!\n") );
			dFlow += 0.001;
			continue;
		}
		else if( CDlgBatchSelBase::BS_RS_FindNoSolution == pclRowParameters->m_eStatus )
		{
			fwprintf( pFile, _T("; Not found!\n") );
			dFlow += 0.001;
			continue;
		}
		else if( CDlgBatchSelBase::BS_RS_FindSolutionsAlter == pclRowParameters->m_eStatus )
		{
			fwprintf( pFile, _T("; Found more that one solution with alternative!\n") );
			dFlow += 0.001;
			continue;
		}

		if( NULL == pclRowParameters->m_pclSelectedProduct 
				|| NULL == dynamic_cast<CSelectedValve *>( pclRowParameters->m_pclSelectedProduct )
				|| NULL == dynamic_cast<CDB_TAProduct *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP ) )
		{
			return;
		}

		CDB_VenturiValve *pclBest = dynamic_cast<CDB_VenturiValve *>( pclRowParameters->m_pclSelectedProduct->GetProductIDPtr().MP );
		CString strSetting = _T("-");
			
		if( NULL != pclBest->GetValveCharacteristic() && -1.0 != pclBestSelectedSTAV->GetH() )
		{
			strSetting = pclBest->GetValveCharacteristic()->GetSettingString( pclBestSelectedSTAV->GetH() );
		}

		_ftprintf( pFile, _T("; %s; %s\n"), pclBest->GetName(), (LPCTSTR)strSetting );

		dFlow += 0.001;
	}

	fclose( pFile );
	
	m_List.SetCurSel( m_List.InsertString( m_List.GetCount(), _T("Finished!") ) );

	if( NULL != pclRowParameters )
	{
		delete pclRowParameters;
		pclRowParameters = NULL;
	}

	if( NULL != clIndSelBVParams.m_pclSelectBvList )
	{
		delete clIndSelBVParams.m_pclSelectBvList;
		clIndSelBVParams.m_pclSelectBvList = NULL;
	}

	if( NULL != clBatchSelBVParams.m_pclBatchBVList )
	{
		delete clBatchSelBVParams.m_pclBatchBVList;
		clBatchSelBVParams.m_pclBatchBVList = NULL;
	}
}

void CTabDlgSpecActDev::ComboFctComputeCRC()
{
	CDlgSpecActComputeCRC clDlgSpecActComputeCRC;
	clDlgSpecActComputeCRC.DoModal();
}

void CTabDlgSpecActDev::ComboFctExtractArticleWithoutPrice()
{
	CDlgSpecialActionExtractArticleWithoutPrice clDlgSpecialActionExtractArticleWithoutPrice;
	clDlgSpecialActionExtractArticleWithoutPrice.DoModal();
}

void CTabDlgSpecActDev::_ExtractPipeSeries( CFileTxt *pOutf, CTable *pTab )
{
	if( NULL == pTab )
	{
		pTab = TASApp.GetpPipeDB()->GetPipeTab();

		if( NULL == pTab )
		{
			ASSERT_RETURN;
		}

		CString str = _T("PipeSeries_id;PipeSeriesNameST;PipeSeriesRoughness");
		pOutf->WriteTxtLine( (LPCTSTR)str );
		pOutf->WriteTxtLine( _T("\r\n") );
	}
	
	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		CTable *pTabSeries = (CTable *)( idptr.MP );

		if( NULL == pTabSeries )
		{
			ASSERT_RETURN;
		}

		if( true == pTabSeries->GetFirst().MP->IsaTable() )
		{
			_ExtractPipeSeries( pOutf, (CTable *)( pTabSeries->GetFirst().MP ) );
		}
		else
		{
			CDB_Pipe *pPipe = (CDB_Pipe *)( pTabSeries->GetFirst().MP );
			CString strRoughness = WriteDouble( pPipe->GetRoughness(), 12 );
			CString str = CString( pTabSeries->GetIDPtr().ID ) + CString( _T(";") ) + CString( pTabSeries->GetNameIDS() ) + CString( _T(";") ) + strRoughness;
			pOutf->WriteTxtLine( (LPCTSTR)str );
			pOutf->WriteTxtLine( _T("\r\n") );
		}
	}
}

void CTabDlgSpecActDev::_ExtractPipes( CFileTxt *pOutf, CTable *pTab )
{
	if( NULL == pTab )
	{
		pTab = TASApp.GetpPipeDB()->GetPipeTab();
		
		if( NULL == pTab )
		{
			ASSERT_RETURN;
		}

		CString str = _T("Pipe_id;PipeSeries_id;Size_id;PipeName;PipeIntDiam");
		pOutf->WriteTxtLine((LPCTSTR)str);
		pOutf->WriteTxtLine(_T("\r\n") );
	}

	std::map< double, CDB_Pipe * > mapPipe;
	std::map< double, CDB_Pipe * >::iterator It;
	
	for( IDPTR idptrSeries = pTab->GetFirst(); NULL != idptrSeries.MP; idptrSeries = pTab->GetNext(idptrSeries.MP) )
	{
		CTable *pTabSeries = (CTable *)(idptrSeries.MP);

		if (NULL == pTabSeries)
		{
			ASSERT_RETURN;
		}
	
		mapPipe.clear();

		for( IDPTR idptrPipe = pTabSeries->GetFirst(); NULL != idptrPipe.MP; idptrPipe = pTabSeries->GetNext(idptrPipe.MP) )
		{
			if( true == idptrPipe.MP->IsaTable() )
			{
				_ExtractPipes( pOutf, (CTable *)pTabSeries->GetFirst().MP );
			}
			else
			{
				CDB_Pipe *pPipe = dynamic_cast<CDB_Pipe *>( idptrPipe.MP );
				
				if( NULL == pPipe )
				{
					ASSERT( 0 );
					continue;
				}
				
				mapPipe[pPipe->GetIntDiameter()] = pPipe;
			}
		}

		if( mapPipe.size()>0 )
		{
			for (It = mapPipe.begin(); It != mapPipe.end(); It++)
			{
				CDB_Pipe *pPipe = It->second;
				if (NULL == pPipe)
				{
					ASSERT(0);
					continue;
				}
				CString strIntDiam = WriteDouble(pPipe->GetIntDiameter(),12);
				CString str = CString(pPipe->GetIDPtr().ID) + CString( _T(";")) + CString(idptrSeries.ID) + CString( _T(";")) + pPipe->GetSizeID() + CString( _T(";")) +  CString(pPipe->GetName()) + CString( _T(";")) + strIntDiam;
				pOutf->WriteTxtLine((LPCTSTR)str);
				pOutf->WriteTxtLine(_T("\r\n") );
			}
		}
		pOutf->WriteTxtLine(_T("\r\n") );
	}
}

void CTabDlgSpecActDev::_ExtractCDBTAP()
{
	BeginWaitCursor();
	CFileTxt fw( _T("DropTAP.txt"), CFile::modeCreate | CFile::modeReadWrite );
	CDataList *pDL = TASApp.GetpTADB()->GetpDataList();

	for( IDPTR IDPtr = pDL->GetFirst(); IDPtr.MP != NULL; IDPtr = pDL->GetNext() )
	{
		CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct*>( IDPtr.MP );
		
		if( NULL != pTAP )
		{
			CString str = CString( _T("----------------------------------------------\r\n") );
			fw.WriteTxtLine( str );
			str = IDPtr.ID + CString( _T("\r\n") );
			fw.WriteTxtLine( str );
			str = pTAP->GetName() + CString( _T("\r\n") );
			fw.WriteTxtLine( str );
			str = pTAP->GetBodyMaterialID() + CString( _T("\r\n") );
			fw.WriteTxtLine( str );
			str = pTAP->GetFamilyID() + CString( _T("\r\n") );
			fw.WriteTxtLine( str );
			str = pTAP->GetTypeID() + CString( _T("\r\n") );
			fw.WriteTxtLine( str );
			str.Format(_T("%d\r\n"), pTAP->GetPriorityLevel());
			fw.WriteTxtLine( str );
			str = pTAP->GetComment() + CString( _T("\r\n") );
			fw.WriteTxtLine( str );
			str = pTAP->GetProdPicID() + CString( _T("\r\n") );
			fw.WriteTxtLine( str );
			str = pTAP->GetAccessoriesGroupIDPtr().ID + CString( _T("\r\n") );
			fw.WriteTxtLine( str );

			switch( pTAP->GetPartOfaSet() )
			{
				case ePartOfaSetNo:
					str = pTAP->GetProdPicID() + CString( _T("ePartOfaSetNo\r\n") );
					break;

				case ePartOfaSetYes:
					str = pTAP->GetProdPicID() + CString( _T("ePartOfaSetYes\r\n") );
					break;

				case ePartOfaSetYesBreakOk:
					str = pTAP->GetProdPicID() + CString( _T("ePartOfaSetYesBreakOk\r\n") );
					break;

				case ePartOfaSetYesBreakOnMsg:
					str = pTAP->GetProdPicID() + CString( _T("ePartOfaSetYesBreakOnMsg\r\n") );
					break;

				case ePartOfaSetLast:
					str = pTAP->GetProdPicID() + CString( _T("ePartOfaSetLast\r\n") );
					break;

				default:
					str = pTAP->GetProdPicID() + CString( _T("UNKNOWN\r\n") );

			}

			fw.WriteTxtLine( str );

			str = pTAP->GetProdPicID() + CString( _T("\r\n") );
			fw.WriteTxtLine( str );
		}
	}

	fw.Flush();
	fw.Close();
	EndWaitCursor();
}

void CTabDlgSpecActDev::_TestArticleNumberDuplication()
{
	CString str = _T("Duplicate Art. Number file name (*.txt)|*.txt||");
	CFileDialog dlg( false, _T("txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXTENSIONDIFFERENT, (LPCTSTR)str, NULL );
	CString PrjDir = GetProjectDirectory();

	dlg.m_ofn.lpstrInitialDir = (LPCTSTR)PrjDir;
	
	if( IDOK == dlg.DoModal() )
	{
		BeginWaitCursor();
		CFileTxt outf;
		outf.Open( dlg.GetPathName(), CFile::modeCreate|CFile::modeWrite );
	
		// Unicode.
		BYTE tmpbuf[] = { 0xff, 0xfe };
		outf.Write( tmpbuf, sizeof( tmpbuf ) );

		std::map<_string, std::vector<CDB_Set *>> mmap;
		std::map<_string, std::vector<CDB_Set *>>::iterator it;
		
		CStringArray ar;
		ar.Add(_T("BCVACTSET_TAB") );
		ar.Add(_T("PICVACTSET_TAB") );
		
		for( int i = 0 ; i < ar.GetSize(); i++ )
		{
			CTable *pTab = (CTable *)( TASApp.GetpTADB()->Get(ar[i]).MP );

			for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
			{
				CDB_Set *pSet = dynamic_cast<CDB_Set*>( idptr.MP );

				if( NULL == pSet )
				{
					continue;
				}

				if( 0 == mmap.count( (LPCTSTR)pSet->GetReference() ) )
				{
					std::vector<CDB_Set *> v;
					v.push_back( pSet );
					mmap[(LPCTSTR)pSet->GetReference()] = v;
				}
				else
				{
					mmap[(LPCTSTR)pSet->GetReference()].push_back( pSet );
				}
			}
		}

		for( it = mmap.begin(); it != mmap.end(); it++ )
		{
			//if ( it->second.size() > 1 ) 
			{
				CString str = it->first.c_str() + CString( _T(" : ") );
				
				for( int i = 0; i < (int)it->second.size(); i++ )
				{
					str = str + CString( it->second[i]->GetIDPtr().ID ) + CString( _T("; ") );
				}

				str = str + CString( _T("\r\n") );
				outf.WriteTxtLine( str );
			}
		}

		outf.Close();

		CString str = dlg.GetPathName() + CString( _T("\r\nCreated") );

		EndWaitCursor();

		AfxMessageBox( str, MB_OK | MB_ICONEXCLAMATION, 0 );
	}
}

void CTabDlgSpecActDev::_ExtractUniversalValves( void )
{
	IDPTR idptr = TASApp.GetpTADB()->Get( _T("UNIVERSALVALV_TAB" ) );
	CTable *pTab = (CTable *)( idptr.MP );

	for( idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext() )
	{
		int i = 0;
		i++;
	}

	pTab = (CTable *)( pTab->GetFirst().MP );

	for( idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext() )
	{
		int i = 0;
		i++;
	}
}

void CTabDlgSpecActDev::_ExtractKvVsOpeningForMVValves( void )
{
	///////////////////////////////////////
	/// Code for extracting Kv vs Opening for STAD, STAF, ... families
	///////////////////////////////////////
	CDS_WaterCharacteristic *pWC = TASApp.GetpTADS()->GetpWCForProductSelection();

	BeginWaitCursor();

	CFileTxt fw( _T("STADstar.txt"), CFile::modeCreate | CFile::modeReadWrite );
	
	// For each valve.
	CTable *pTab = (CTable *)( TASApp.GetpTADB()->Get( _T("REGVALV_TAB") ).MP );
	
	std::set<wstring> setFamID;
	setFamID.insert(L"FAM_STAD*");
	//setFamID.insert(L"FAM_STAD");
	//setFamID.insert(L"FAM_STAD-C");
	//setFamID.insert(L"FAM_STAD-R");
	//setFamID.insert(L"FAM_STAF");
	//setFamID.insert(L"FAM_STAF-R");
	//setFamID.insert(L"FAM_STAG");
	//setFamID.insert(L"FAM_TBV");
	//setFamID.insert(L"STAF-SG");

	std::set<wstring>::iterator ItFam;
	
	for( ItFam = setFamID.begin(); ItFam != setFamID.end(); ItFam++ )
	{
		CRankEx List;
		TASApp.GetpTADB()->GetBVList( &List, L"", ItFam->c_str(), L"", L"", L"" );
		wstring str;
		LPARAM lparam;
		std::set<wstring> setChar;
		
		for( bool bContinue = List.GetFirst(str, lparam); bContinue == true; bContinue = List.GetNext( str, lparam ) )
		{
			CDB_RegulatingValve *pBV = dynamic_cast<CDB_RegulatingValve*>( (CData *)lparam );

			if( NULL == pBV )
			{
				continue;
			}

//			if( false == ( IDcmp( pBV->GetIDPtr().ID, _T("STAF-SG350A_BV") ) == 0 || IDcmp(pBV->GetIDPtr().ID, _T("STAF-SG400A_BV") ) == 0) )
//			{
//				continue;
//			}

			if( false == pBV->IsSelectable(true) )
			{
				continue;
			}

	 		std::set<wstring>::iterator ItFam = setFamID.find( pBV->GetFamilyID() );

	 		if( ItFam == setFamID.end() )
			{
	 			continue;
			}

			CDB_ValveCharacteristic *pChar = dynamic_cast<CDB_ValveCharacteristic*>( pBV->GetValveCharDataPointer() );

			if( NULL == pChar )
			{
				continue;
			}

			std::set<wstring>::iterator It = setChar.find( pChar->GetIDPtr().ID );

			if( It != setChar.end() )
			{
				continue;
			}

			setChar.insert( pChar->GetIDPtr().ID );

			fw.WriteTxtLine( pBV->GetName() );
			fw.Write( _T("\r\n"), sizeof( TCHAR ) * 2 );

			CString str = CString( _T("h; Kv\r\n") ) ;
			fw.WriteTxtLine( str );

			for( double dH = 0.0; dH <= pChar->GetOpeningMax(); dH += 0.005 )
			{
				double dKv = pChar->GetKv( dH );
				CString strh = WriteDouble( dH, 4, 2 );
				CString strKv = WriteDouble( dKv, 10, 4 );
				CString str = strh + CString( _T(";") ) + strKv + CString( _T("\r\n") );
				fw.WriteTxtLine( str );
			}

			fw.WriteTxtLine( _T("-------------------------------------------------------------\r\n") );
		}
	}

	fw.Flush();
	fw.Close();					
	EndWaitCursor();
}

void CTabDlgSpecActDev::_ExtractFlowAndDpmin( void )
{
	CStringArray IDList;

 	//IDList.Add( _T("COMPACT_P_10A") );
 	//IDList.Add( _T("COMPACT_P_15A") );
 	//IDList.Add( _T("COMPACT_P_20A") );
 	//IDList.Add( _T("COMPACT_P_25A") );
 	//IDList.Add( _T("COMPACT_P_32A") );

	//IDList.Add( _T("TA_MODULATOR_15A") );
	//IDList.Add( _T("TA_MODULATOR_20A") );
	//IDList.Add( _T("TA_MODULATOR_25A") );
	//IDList.Add( _T("TA_MODULATOR_32A") );
	IDList.Add( _T("TA_MODULATOR_40A") );
	IDList.Add( _T("TA_MODULATOR_50A") );
	IDList.Add( _T("TA_MODULATOR_65A") );
	IDList.Add( _T("TA_MODULATOR_80A") );
	IDList.Add( _T("TA_MODULATOR_100NFA") );
	IDList.Add( _T("TA_MODULATOR_100HFA") );
	IDList.Add( _T("TA_MODULATOR_125NFA") );
	IDList.Add( _T("TA_MODULATOR_125HFA") );
	IDList.Add( _T("TA_MODULATOR_150NFA") );
	IDList.Add( _T("TA_MODULATOR_150HFA") );
// 	IDList.Add( _T("TBV-CMP_15LFA") );
// 	IDList.Add( _T("TBV-CMP_15NFA") );
// 	IDList.Add( _T("TBV-CMP_20NFA") );
// 	IDList.Add( _T("TBV-CMP_25NFA") );

// 	IDList.Add( _T("FUSION_P32A") );
// 	IDList.Add( _T("FUSION_P40A") );
// 	IDList.Add( _T("FUSION_P50A") );
//  IDList.Add( _T("FUSION_P65_2A") );
//  IDList.Add( _T("FUSION_P80_2A") );
// 	IDList.Add( _T("FUSION_P100A") );
// 	IDList.Add( _T("FUSION_P125A") );
// 	IDList.Add( _T("FUSION_P150A") );

// 	IDList.Add( _T("KTCM512_15LF") );        
// 	IDList.Add( _T("KTCM512_15NF") );        
// 	IDList.Add( _T("KTCM512_15HF") );        
// 	IDList.Add( _T("KTCM512_20LFA") );       
// 	IDList.Add( _T("KTCM512_20LFB") );       
// 	IDList.Add( _T("KTCM512_20NFA") );       
// 	IDList.Add( _T("KTCM512_20NFB") );       
// 	IDList.Add( _T("KTCM512_20HFA") );       
// 	IDList.Add( _T("KTCM512_20HFB") );       
	
	CDS_WaterCharacteristic *pWC = TASApp.GetpTADS()->GetpWCForProductSelection();

	BeginWaitCursor();

	CFileTxt fw( _T("Modulator.txt"), CFile::modeCreate | CFile::modeReadWrite );
	
	// For each valve.
	CTable *pTab = (CTable *)( TASApp.GetpTADB()->Get( _T("PICTRLVALV_TAB") ).MP );
	std::set<CDB_PICVCharacteristic*> setChar;

//	for (IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext(idptr.MP) )
	for( int i = 0; i < IDList.GetCount(); i++ )
	{
//		CString strID = idptr.ID;
//
// 		if( 0 != strID.Find( _T("KTM512_") ) )
//		{
// 			continue;
//		}
	
		IDPTR idptr = TASApp.GetpTADB()->Get( IDList[i] );
		CDB_PIControlValve *pCV = dynamic_cast<CDB_PIControlValve*>( idptr.MP );

		if( NULL == pCV )
		{
			continue;
		}
		
		if( false == pCV->IsSelectable(true) )
		{
			continue;
		}

		CDB_PICVCharacteristic *pChar = dynamic_cast<CDB_PICVCharacteristic*>( pCV->GetValveCharDataPointer() );
		
		if( NULL == pChar )
		{
			continue;
		}
		
		std::set<CDB_PICVCharacteristic *>::iterator It = setChar.find( pChar );

		if( It != setChar.end() )
		{
			continue;
		}
		
		setChar.insert( pChar );

		fw.WriteTxtLine( pCV->GetName() );
		fw.Write( _T("\r\n" ), sizeof( TCHAR ) * 2 );

		CString str = _T("Setting;Stroke [m];Flow [m3/s];Dpmin [Pa]\r\n");
		fw.WriteTxtLine( str );
		double dStep = ( pChar->GetQmaxMax() - pChar->GetQmax( 1 ) ) / 100.0;

//		h=f(q) & dpmin = f(h)
//
// 		for( double dq = pChar->GetQmax( 1 ); dq <= pChar->GetQmaxMax(); dq += dStep )
// 		{
// 			double dh = pChar->GetSettingFromQmax( dq );
// 			double dpmin = pChar->GetDpmin( dh );
// 			CString strdq = WriteDouble( dq, 10, 4 );
// 			CString strh = WriteDouble( dh, 4, 2 );
// 			CString strdpmin = WriteDouble( dpmin, 10, 0 );
// 			CString str = strdq + CString( _T(";") ) + strh + CString( _T(";") ) + strdpmin + CString( _T("\r\n") );
// 			fw.WriteTxtLine( str );
// 		}

		// q=f(h) & dpmin=f(h)
		for( double dH = 0.0; dH <= pChar->GetSettingMax(); dH += 0.5 )
		{
			double dStroke = pChar->GetStroke( dH );
			double dFlow = pChar->GetQmax( dH );
			double dpmin = pCV->GetDpmin( dFlow, pWC->GetDens() ); //pChar->GetDpmin(dh);
			CString strSetting = WriteDouble( dH, 4, 2 );
			CString strStroke = WriteDouble( dStroke, 4, 2 );
			CString strFlow = WriteCUDouble( _U_FLOW, dFlow );
			CString strDpMin = WriteCUDouble( _U_DIFFPRESS, dpmin );
			CString str = strSetting + CString( _T(";") ) + strStroke + CString( _T(";") ) + strFlow + CString( _T(";") ) + strDpMin + CString( _T("\r\n") );
			fw.WriteTxtLine( str );
		}
	}

	fw.Flush();
	fw.Close();					
	EndWaitCursor();
}

void CTabDlgSpecActDev::_ExtractKvVsOpeningForTAFUSIONC( void )
{
	CStringArray IDList;

	IDList.Add( _T("TBV-C15NFA_TV") );
	IDList.Add( _T("TBV-C20NFA_TV") );
	IDList.Add( _T("TBV-C25NFE_TV") );

	IDList.Add( _T("FUSION_C32A") );
	IDList.Add( _T("FUSION_C40A") );
	IDList.Add( _T("FUSION_C50A") );
	IDList.Add( _T("FUSION_C65A") );
	IDList.Add( _T("FUSION_C80A") );
	IDList.Add( _T("FUSION_C100A") );
	IDList.Add( _T("FUSION_C125A") );
	IDList.Add( _T("FUSION_C150A") );

	BeginWaitCursor();

	CFileTxt fw( _T("TBV-C.txt"), CFile::modeCreate | CFile::modeReadWrite );
	
	// For each valve.
	for( int i = 0; i < IDList.GetCount(); i++ )
	{
		CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve*>( TASApp.GetpTADB()->Get( IDList[i]).MP );
		
		if( NULL == pCV )
		{
			continue;
		}

		fw.WriteTxtLine( pCV->GetName() );
		fw.Write( _T("\r\n"), sizeof( TCHAR ) * 2 );

		CString str = CString( _T("h; Kv\r\n") );
		fw.WriteTxtLine( str );
		
		CDB_ValveCharacteristic *pChar = dynamic_cast<CDB_ValveCharacteristic*>( pCV->GetValveCharDataPointer() );
		
		if( NULL == pChar )
		{
			continue;
		}

		for( double dH = 0.5; dH <= pChar->GetOpeningMax(); dH += 0.05 )
		{
			double dKv = pChar->GetKv( dH );
			CString strh = WriteDouble( dH, 4, 2 );
			CString strKv = WriteDouble( dKv, 10, 4 );
			CString str = strh + CString( _T(";") ) + strKv + CString( _T("\r\n") );
			fw.WriteTxtLine( str );
		}

		fw.WriteTxtLine( _T("--------------\r\n") );
	}

	fw.Flush();
	fw.Close();					
	EndWaitCursor();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pressurisation vessels/Tecbox exportation code 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CTabDlgSpecActDev::cbFillVesselFamiliesMap(void *pthis, int argc, char **argv, char **azColName)
{
	int i;
	ASSERT(NULL != pthis);

	if (NULL == pthis)
	{
		return -1;
	}

	CTable *pTab = ((CTabDlgSpecActDev *)pthis)->m_pTab;

	if (NULL == pthis || NULL == pTab)
	{
		return -1;
	}

	// Scan table and fill map accordingly.
	for (i = 0; i < argc; i++)
	{
		CStringA strDbg;

		if (0 == strcmp("VesselFamilyNameST", azColName[i]))
		{
			CStringA str = (char*)argv[i];
			int iUnderscrorePos = str.ReverseFind('_');

			if (iUnderscrorePos <= 0)
			{
				return -1;
			}

			CString strID = L"VSSLFAM" + CString(str.Mid(iUnderscrorePos));

			if (NULL != pTab->Get(strID).MP)
			{
				int iPK = StrToIntA(argv[0]);
				((CTabDlgSpecActDev *)pthis)->m_mapPKVesselFamilyName.Add(strID, iPK);
				//				strDbg.Format("Added: %s = %s", strID, argv[i] ? argv[i] : "NULL");
			}
			else
			{
				//				strDbg.Format("Family not found in HySelect DB: %s = %s", strID, argv[i] ? argv[i] : "NULL");
			}
			//			TASApp.DebugPrintf(CString(strDbg));
		}
	}

	printf("\n");
	return 0;
}

int CTabDlgSpecActDev::cbFillTecboxFamiliesMap(void *pthis, int argc, char **argv, char **azColName)
{
	ASSERT(NULL != pthis);

	if (NULL == pthis)
	{
		return -1;
	}

	// m_pTab should be initialized with TECBOXFAM_TAB MP
	CTable *pTab = ((CTabDlgSpecActDev *)pthis)->m_pTab;

	if (NULL == pthis || NULL == pTab)
	{
		return -1;
	}

	CTemplateMap < CString, CString> mapFamPKID;
	mapFamPKID.Add(L"TecboxFamilyName.CX_Connect", L"TBXFAM_COMPR_CX_CONNECT");				
	mapFamPKID.Add(L"TecboxFamilyName.C_Connect", L"TBXFAM_COMPR_CONNECT");
	mapFamPKID.Add(L"TecboxFamilyName.Simply_Compresso", L"TBXFAM_COMPR_SIMP");
	mapFamPKID.Add(L"TecboxFamilyName.TV_Connect", L"TBXFAM_TRANSFERO_TV_C");
	mapFamPKID.Add(L"TecboxFamilyName.TVI_Connect", L"TBXFAM_TRANSFERO_TVI_C");
	mapFamPKID.Add(L"TecboxFamilyName.V_Connect", L"TBXFAM_VENTO_CONNECT");
	mapFamPKID.Add(L"TecboxFamilyName.V_EcoEfficient", L"TBXFAM_VENTO_V");
	
	// Scan table and fill map accordingly.
	CString str = (CString)((char*)argv[2]);	// Column TecboxfamilyNameST
	CString strID;
	if (true == mapFamPKID.Get(str, strID))
	{
		if (NULL != pTab->Get(strID).MP)
		{
			int iPK = StrToIntA(argv[0]);
			((CTabDlgSpecActDev *)pthis)->m_mapPKTecboxFamilyName.Add(strID, iPK);
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
	return 0;
}

int CTabDlgSpecActDev::cbFillVesselMap(void *pthis, int argc, char **argv, char **azColName)
{
	int i;
	ASSERT(NULL != pthis);

	if (NULL == pthis)
	{
		return -1;
	}

	CTable *pTab = ((CTabDlgSpecActDev *)pthis)->m_pTab;

	if (NULL == pthis || NULL == pTab)
	{
		return -1;
	}

	// Scan table and fill map accordingly.
	for (i = 0; i < argc; i++)
	{
		CStringA strDbg;

		if (0 == strcmp("Vessel_HYSid", azColName[i]))
		{
			CString HYSid = CString(argv[i]);
			int iPK = StrToIntA(argv[0]);
			((CTabDlgSpecActDev *)pthis)->m_mapPKVessel.Add(HYSid, iPK);
			// 			strDbg.Format("Added: %s = %s", HYSid, argv[0] ? argv[i] : "NULL");
			// 			TASApp.DebugPrintf(CString(strDbg));
			break;
		}
	}

	printf("\n");
	return 0;
}

int CTabDlgSpecActDev::cbFillTecBoxesMap(void *pthis, int argc, char **argv, char **azColName)
{
	int i;
	ASSERT(NULL != pthis);

	if (NULL == pthis)
	{
		return -1;
	}

	CTable *pTab = ((CTabDlgSpecActDev *)pthis)->m_pTab;

	if (NULL == pthis || NULL == pTab)
	{
		return -1;
	}

	// Scan table and fill map accordingly.
	for (i = 0; i < argc; i++)
	{
		CStringA strDbg;

		if (0 == strcmp("Tecbox_HYSid", azColName[i]))
		{
			CString HYSid = CString(argv[i]);
			int iPK = StrToIntA(argv[0]);
			((CTabDlgSpecActDev *)pthis)->m_mapPKTecbox.Add(HYSid, iPK);
			break;
		}
	}
	return 0;
}

// Replace in str all occurrences of "from" to "to"
void CTabDlgSpecActDev::_ReplaceAll(std::string &str, const std::string& from, const std::string& to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
}

void CTabDlgSpecActDev::_PrepareInsertQuery(std::string &str)
{
	int ColNbr = 0;
	size_t start_pos = 0;
	std::string to = "";
	std::string strColNbr = ")values(";
	while ((start_pos = str.find("#", start_pos)) != std::string::npos)
	{
		++ColNbr;
		size_t comma_pos = str.find(',', start_pos);
		if (comma_pos == std::string::npos)
		{
			comma_pos = str.length();
			strColNbr = strColNbr + "?" + std::to_string(ColNbr) ;
		}
		else
		{
			strColNbr = strColNbr + "?" + std::to_string(ColNbr) + ",";
		}
		str.replace(start_pos, comma_pos - start_pos, "");
		start_pos++;
	}
	str = str + strColNbr;
}

bool CTabDlgSpecActDev::SortVessels(CDB_Vessel *pVssl1, CDB_Vessel *pVssl2)
{
	double dVN1 = pVssl1->GetNominalVolume();
	double dVN2 = pVssl2->GetNominalVolume();

	if (dVN1 < dVN2)
	{
		return true;
	}
	else if (dVN1 > dVN2)
	{
		return false;
	}
	else // dVN1==dVN2
	{
		double dP1 = pVssl1->GetPmaxmax();
		double dP2 = pVssl2->GetPmaxmax();
		return (dP1 < dP2);
	}
}

bool CTabDlgSpecActDev::SortTecboxes(CDB_TecBox *pTB1, CDB_TecBox *pTB2)
{
	double dPmax1 = pTB1->GetPmaxmax();
	double dPmax2 = pTB2->GetPmaxmax();
	if (dPmax1 < dPmax2)
	{
		return true;
	}
	else if (dPmax1 > dPmax2)
	{
		return false;
	}
	else // dPmax1==dPmax2
	{
		double dP1 = pTB1->GetPower();
		double dP2 = pTB2->GetPower();

		if (dP1 < dP2)
		{
			return true;
		}
		else if (dP1 > dP2)
		{
			return false;
		}
		else // dP1==dP2
		{
			double dW1 = pTB1->GetWeight();
			double dW2 = pTB2->GetWeight();
			return (dP1 < dP2);
		}
	}
}

void CTabDlgSpecActDev::_CreateSQLTable( sqlite3 *pdb, char *sql )
{
	char *zErrMsg = 0;
	int iReturnCode;
	CString str;
	
	/* Execute SQL statement */
	iReturnCode = sqlite3_exec( pdb, sql, NULL, 0, &zErrMsg );

	if( SQLITE_OK != iReturnCode )
	{
		CString strSQL = (CString)zErrMsg;
		str.Format( _T("SQL error: %s"), strSQL );
		AfxMessageBox( str, MB_OK | MB_ICONSTOP );
		sqlite3_free( zErrMsg );
	}
}

void CTabDlgSpecActDev::_ExtractAllArticleNumbersAndVVS()
{
	CFile cFile;

	if( 0 == cFile.Open( _T("c:\\temp\\artnbr.txt"), CFile::modeCreate | CFile::modeWrite ) )
	{
		return;
	}

	for( IDPTR IDPtr = TASApp.GetpTADB()->GetpDataList()->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = TASApp.GetpTADB()->GetpDataList()->GetNext() )
	{
		USES_CONVERSION;

		CDB_Product *pclProduct = dynamic_cast<CDB_Product *>( (CData *)( IDPtr.MP ) );

		if( NULL == pclProduct )
		{
			continue;
		}

		if( true == pclProduct->IsDeleted() )
		{
			continue;
		}

		CString strArticleNumber = pclProduct->GetArtNum();
		strArticleNumber.Trim();

		if( true == strArticleNumber.IsEmpty() )
		{
			continue;
		}

		CString strLAN = LocArtNumTab.GetLocalArticleNumber( pclProduct->GetArtNum() );
		strLAN.Trim();

		CString str = strArticleNumber + _T(";") + strLAN + _T(";") + pclProduct->GetName() + _T("\r\n");
		cFile.Write( T2A( str.GetBuffer() ), str.GetLength() );
	}

	cFile.Close();
}

void CTabDlgSpecActDev::_SQLiteError( sqlite3 *pSQLD, CString strErr )
{
	sqlite3_close( pSQLD );

	CString str;
	str.Format( _T("SQL error: %s"), strErr );
	AfxMessageBox( str, MB_OK | MB_ICONSTOP );

}

CStringA CTabDlgSpecActDev::_NormalizeName(CStringA strA)
{
	strA.Replace(' ', '_');
	strA.Replace('-', '_');
	return strA;
}

CStringA CTabDlgSpecActDev::_GetInstallationLayout( int iInstallationLayout )
{
	if( CDB_Product::eilWallHanged & ( iInstallationLayout & CDB_Product::eilWallHanged ) ) 
	{
		return CStringA("WallHanged");
	}
	else if( CDB_Product::eilVesselTop & ( iInstallationLayout & CDB_Product::eilVesselTop ) ) 
	{
		return CStringA("VesselTop");
	}
	else if( CDB_Product::eilFloorStanding & ( iInstallationLayout & CDB_Product::eilFloorStanding ) ) 
	{
		return CStringA("FloorStanding");
	}
	else if( CDB_Product::eilFloorStandingNextTo & ( iInstallationLayout & CDB_Product::eilFloorStandingNextTo ) ) 
	{
		return CStringA("FloorStandingNextTo");
	}
	else if( CDB_Product::eilFloorStandingInFront & ( iInstallationLayout & CDB_Product::eilFloorStandingInFront ) ) 
	{
		return CStringA("FloorStandingInFront");
	}
	else if( CDB_Product::eilIntegrated & ( iInstallationLayout & CDB_Product::eilIntegrated ) ) 
	{
		return CStringA("Integrated");
	}
	else if( CDB_Product::eiUndefined & ( iInstallationLayout & CDB_Product::eiUndefined ) ) 
	{
		return CStringA("");
	}
	else
	{
		return CStringA("");
	}
}

/*
int CTabDlgSpecActDev::_GetConnection(CDB_Connect *pConnect)
{
	/*	
	Connection_id	ConnectionNameST	ConnectionSortKey
			1	sr_Connection_ThreadISO228_Female_G	1
			2	sr_Connection_ThreadISO228_Male_G	2
			3	sr_Connection_ThreadISO7 - 1_Female_Rp	3
			4	sr_Connection_ThreadISO7 - 1_Female_Rc	4
			5	sr_Connection_ThreadISO7 - 1_Male_R	5
	*/
/*	int iSqlPK = _wtoi(pConnect->GetIDstr2());
	return iSqlPK;
}
*/

int CTabDlgSpecActDev::_GetConnectionSizes(sqlite3 *pDb, CDB_Vessel* pVssl )
{
	sqlite3_stmt *pStmt = NULL;
	char *zErrMsg = 0;
	std::string SqlQuery;

/*
ConnectionSize_id	Connection_id	ConnectionSizeNameST	ConnectionSizeSortKey
	1	1	sr_ConnectionSize_Female_G_1-16	1
	2	1	sr_ConnectionSize_Female_G_1-8	2
	3	1	sr_ConnectionSize_Female_G_1-4	3
	4	1	sr_ConnectionSize_Female_G_3-8	4
	5	1	sr_ConnectionSize_Female_G_1-2	5
	6	1	sr_ConnectionSize_Female_G_3-4	6
	7	1	sr_ConnectionSize_Female_G_1	7
	8	1	sr_ConnectionSize_Female_G_1_1-4	8
	9	1	sr_ConnectionSize_Female_G_1_1-2	9
	10	1	sr_ConnectionSize_Female_G_2	10
	11	1	sr_ConnectionSize_Female_G_2_1-2	11
	12	1	sr_ConnectionSize_Female_G_3	12
	13	1	sr_ConnectionSize_Female_G_4	13
*/

	CDB_StringID *pStrID = (CDB_StringID *)pVssl->GetSizeIDPtr().MP;
	CStringA str = (CStringA)(pStrID->GetIDstr2());
	str.Replace('/', '_');
	str.Replace(' ', '_');
	str.Replace('\"', 'i');

	// Try to identify connection type
	// based on IDPTR.ID
	CDB_Connect *pConnect = (CDB_Connect *)pVssl->GetConnectIDPtr().MP;
	SqlQuery = "SELECT * FROM ConnectionSizes WHERE Connection_id = ";
	SqlQuery = SqlQuery + (char*)pConnect->GetIDstr2();

	if (0 == wcscmp(pConnect->GetIDPtr().ID, _T("CONN_THR_F_G")) )
	{
		str = "Female_G_" + str;
	}
	else if (0 == wcscmp(pConnect->GetIDPtr().ID, _T("CONN_THR_M_G") ) )
	{
		str = "Male_G_" + str;
	}
	else if (0 == wcscmp(pConnect->GetIDPtr().ID, _T("CONN_THR_F_RP") ) )
	{
		str = "Female_Rp_" + str;
	}
	else if (0 == wcscmp(pConnect->GetIDPtr().ID, _T("CONN_THR_F_RC") ) )
	{
		str = "Female_Rc_" + str;
	}
	else if (0 == wcscmp(pConnect->GetIDPtr().ID, _T("CONN_THR_M_R") ) )
	{
		str = "Male_R_" + str;
	}
	else if (0 == wcscmp(pConnect->GetIDPtr().ID, _T("CONN_FLANGE") ) )
	{
		str = (CStringA)(pVssl->GetSizeIDPtr().ID);
		str = ".EN1092_1_" + str;
	}
	else
	{
		DebugBreak();
	}

	// Identify PK in ConnectionSizes table that correspond to the connection type and size
	sqlite3_prepare_v2(pDb, SqlQuery.c_str(), -1, &pStmt, NULL);

	int iPK = 0;
	while (sqlite3_step(pStmt) == SQLITE_ROW)
	{
		CStringA strSizeName = CStringA(sqlite3_column_text(pStmt, 2));		// ConnectionSizeNameST
		if (-1 != strSizeName.Find(str))
		{
			iPK = sqlite3_column_int( pStmt, 0 );							// ConnectionSize_id
			break;
		}
	}
	
	sqlite3_finalize(pStmt);

	return iPK;
}

bool CTabDlgSpecActDev::_ExtractVessels( sqlite3 *pDb )
{
	sqlite3_stmt *pStmt = NULL;
	char *zErrMsg = 0;
	int iReturnCode;
	char *pSqlQuery;
	int i = 0;
	CString str;
	CStringA strA;

	// Table definition '#' character is mandatory, it's used internally to prepare SQlitequeries
	std::string strVesselColumns =
		"[Vessel_id]#				INT PRIMARY KEY		NOT NULL,\n" \
		"[Vessel_HYSid]#			TEXT				NOT NULL,\n" \
		"[VesselFamily_id]#         INT					NOT NULL,\n" \
		"[VesselNameST]#			TEXT				NOT NULL,\n" \
		"[VesselPicture]#			TEXT						,\n" \
		"[VesselArticleNo]#			TEXT				NOT NULL,\n" \
		"[VesselVN]#				REAL				NOT NULL,\n" \
		"[VesselPmax]#				REAL				NOT NULL,\n" \
		"[VesselPSch]#				REAL						,\n" \
		"[VesselTmin]#				REAL				NOT NULL,\n" \
		"[VesselTmax]#				REAL				NOT NULL,\n" \
		"[VesselFactoryP0]#			REAL				NOT NULL,\n" \
		"[VesselHeight]#			REAL				NOT NULL,\n" \
		"[VesselDiameter]#			REAL				NOT NULL,\n" \
		"[VesselWeight]#			REAL				NOT NULL,\n" \
		"[VesselConnectionSize_id]#	INT					NOT NULL,\n" \
		"[VesselInstallationCode]#	TEXT				NOT NULL,\n" \
		"[VesselAcceptTecboxOnTop]#	INT					NOT NULL,\n" \
		"[VesselInternallyCoated]#	INT					NOT NULL,\n" \
		"[VesselBetaOnly]#			INT					NOT NULL,\n" \
		"[VesselDeleted]#			INT					NOT NULL";

	std::string strPrim_SecVesselColumns =
		"[PrimaryVesselSecondaryVessel_id]#		INT PRIMARY KEY     NOT NULL,\n"\
		"[PrimaryVessel_id]#					INT					NOT NULL,\n"\
		"[SecondaryVessel_id]#					INT				    NOT NULL";


	// Fill cross references map.
	// Read VesselFamilies and fill m_mapPKVesselFamilyName that give a link between VesselFamily_id PK and VesselFamily IDPTR
	m_pTab = (CTable *)( TASApp.GetpTADB()->Get( _T("VSSLFAM_TAB") ).MP );
	ASSERT( NULL != m_pTab );
	
	if( NULL == m_pTab )
	{
		AfxMessageBox( _T("Missing VSSFAM_TAB"), MB_OK | MB_ICONSTOP );
		return false;
	}

	// Extract VesselFamily_id.
	pSqlQuery = "SELECT * from VesselFamilies";
	iReturnCode = sqlite3_exec( pDb, pSqlQuery, this->cbFillVesselFamiliesMap, this, &zErrMsg );
	if( SQLITE_OK != iReturnCode )
	{
		// Table doesn't exist.
		CString str = (CString)zErrMsg;
		if (NULL != zErrMsg)
		{
			sqlite3_free(zErrMsg);
		}
		_SQLiteError( pDb, str );
		return false;
	}

	// Extract Vessel_id and Vessel_HYSid.
	m_pTab = (CTable *)( TASApp.GetpTADB()->Get( _T("VSSL_TAB") ).MP );
	ASSERT( NULL != m_pTab );

	if( NULL == m_pTab )
	{
		AfxMessageBox( _T("Missing VSSL_TAB"), MB_OK | MB_ICONSTOP );
		return false;
	}

	int iMaxPK = 0;
	pSqlQuery = "SELECT * from Vessels";
	iReturnCode = sqlite3_exec( pDb, pSqlQuery, this->cbFillVesselMap, this, &zErrMsg );
	
	if( SQLITE_OK == iReturnCode )
	{
		// If Table doesn't exist no problem we will create it from scratch.
		// If table exist extract PK of vessel to use it when we redo the table.
		// Extract max PK.
		CString strHYSid;
		int iPK;
		bool bContinue = m_mapPKVessel.GetFirst( strHYSid, iPK );

		while( true == bContinue )
		{
			if( iPK > iMaxPK )
			{
				iMaxPK = iPK;
			}

			bContinue = m_mapPKVessel.GetNext( strHYSid, iPK );
		}
	}

	// Drop Vessels table
	pSqlQuery = "DROP TABLE IF EXISTS Vessels";
	iReturnCode = sqlite3_exec( pDb, pSqlQuery, NULL, NULL, &zErrMsg );
	
	// Create new Vessel Table.
	std::string strCol = strVesselColumns;
	_ReplaceAll(strCol, "#", " ");
	std::string strSqlQuery = "CREATE TABLE IF NOT EXISTS [Vessels](\n" + strCol + ")";
	pSqlQuery = (char*) strSqlQuery.c_str();
	_CreateSQLTable( pDb, pSqlQuery );

	// Add vessels from HySelect into the SQLite vessel table.
	m_pTab = (CTable *)( TASApp.GetpTADB()->Get( _T("VSSL_TAB") ).MP );	
	ASSERT( NULL != m_pTab );

	// Prepare INSERT into Vessels query
	strCol = strVesselColumns;
	_PrepareInsertQuery(strCol);
	strSqlQuery = "INSERT INTO Vessels (" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();

	CString strFamilyID;
	int iVesselFamIndex = 1;
	m_mapPKPrimVslIDSecVsl.Clear();
		
	while( true == m_mapPKVesselFamilyName.GetSecond( strFamilyID, iVesselFamIndex ) )
	{
		std::vector<CDB_Vessel *> arrVssl;

		// Sort vessels based on VN.
		for( IDPTR IDPtr = m_pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_pTab->GetNext() )
		{
			CDB_Vessel *pVessel = (CDB_Vessel *)( IDPtr.MP );

			if( strFamilyID != pVessel->GetFamilyID() )
			{
				continue;
			}

			if( true == pVessel->IsDeleted() )
			{
				continue;
			}

			arrVssl.push_back( pVessel );
		}

		std::sort( arrVssl.begin(), arrVssl.end(), CTabDlgSpecActDev::SortVessels );

		// Enter vessels in SQLiteDB.
		int j = 0;

		for( unsigned j = 0; j < arrVssl.size(); j++ )
		{
			CDB_Vessel *pVessel = (CDB_Vessel *)arrVssl[j];
			IDPTR idptr = pVessel->GetIDPtr();
			int iPKConnectSizeid = _GetConnectionSizes(pDb, pVessel);
			int iPK = 0;
			unsigned int uiField = 1;

			if( true == m_mapPKVessel.Get( idptr.ID, iPK ) )
			{
				i = iPK;
			}
			else //Not found
			{
				iMaxPK++;
				i = iMaxPK;
			}
				
			// Fill link between primary and secondary vessel.
			if( NULL != pVessel->GetSecVsslIDPtr().MP )
			{
				m_mapPKPrimVslIDSecVsl.Add( i, pVessel->GetSecVsslIDPtr().ID );
			}

			iReturnCode = sqlite3_prepare_v2( pDb, pSqlQuery, -1, &pStmt, NULL );
			sqlite3_bind_int( pStmt, uiField++, i );

			strA = CStringA( idptr.ID );
			sqlite3_bind_text( pStmt, uiField++, (LPCSTR)strA, -1, SQLITE_STATIC );

			sqlite3_bind_int( pStmt, uiField++, iVesselFamIndex );

			strA = _NormalizeName(CStringA("sr_VesselName_") + CStringA( pVessel->GetName() ) );
			sqlite3_bind_text( pStmt, uiField++, (LPCSTR)strA, -1, SQLITE_STATIC );

			strA = pVessel->GetProdPicID();
			strA.MakeLower();
			int iPos = strA.Find("px_");
			if (iPos > -1)
			{
				strA.Delete(iPos, 3);
			}
			if (0 != strA.Compare("no_id"))
			{
				strA.Remove('_');
				sqlite3_bind_text(pStmt, uiField, (LPCSTR)strA, -1, SQLITE_STATIC);
			}
			uiField++;

			strA = CStringA( pVessel->GetArtNum() );
			sqlite3_bind_text( pStmt, uiField++, (LPCSTR)strA, -1, SQLITE_STATIC );

			sqlite3_bind_double( pStmt, uiField++, pVessel->GetNominalVolume() );
			sqlite3_bind_double( pStmt, uiField++, pVessel->GetPmaxmax() );
			if (0 != pVessel->GetPSch())
			{
				sqlite3_bind_double(pStmt, uiField, pVessel->GetPSch());
			}
			uiField++;
			sqlite3_bind_double(pStmt, uiField++, pVessel->GetTmin());
			sqlite3_bind_double( pStmt, uiField++, pVessel->GetTmax() );
			sqlite3_bind_double( pStmt, uiField++, pVessel->GetFactoryP0() );
			sqlite3_bind_double( pStmt, uiField++, pVessel->GetHeight() );
			sqlite3_bind_double( pStmt, uiField++, pVessel->GetDiameter() );
			sqlite3_bind_double( pStmt, uiField++, pVessel->GetWeight() );
			
			sqlite3_bind_int(pStmt, uiField++, iPKConnectSizeid);

			/* VesselInstallationCode can take the following values :
			•	WallHanged
			•	FloorStanding
			•	Integrated
			*/
			strA = _GetInstallationLayout(pVessel->GetInstallationLayout());
			sqlite3_bind_text( pStmt, uiField++, (LPCSTR)strA, -1, SQLITE_STATIC );

			int ifoo = ( false == pVessel->IsAcceptTecBoxOnTop() ) ? 0 : 1;
			sqlite3_bind_int( pStmt, uiField++, ifoo );
			
			ifoo = pVessel->IsCompatible( CDB_Vessel::ePMComp_VsslIntCoating );
			sqlite3_bind_int( pStmt, uiField++, ifoo );


			// BetaOnly
			ifoo = 0;					
			sqlite3_bind_int(pStmt, uiField++, ifoo);

			ifoo = pVessel->IsDeleted();
			sqlite3_bind_int( pStmt, uiField++, ifoo );

			iReturnCode = sqlite3_step( pStmt );

			if( SQLITE_DONE != iReturnCode )
			{
				CString str = (CString)zErrMsg;
				if (NULL != zErrMsg)
				{
					sqlite3_free(zErrMsg);
				}
				_SQLiteError(pDb, str);
				return false;
			}

			sqlite3_finalize( pStmt );
		}

		++iVesselFamIndex;
	};

	m_mapPKVessel.Clear();
		
	// Extract Vessel_id and Vessel_HYSid according new filling.
	pSqlQuery = "SELECT * from Vessels";
	iReturnCode = sqlite3_exec( pDb, pSqlQuery, this->cbFillVesselMap, this, &zErrMsg );
		
	if( SQLITE_OK != iReturnCode )
	{
		CString str = (CString)zErrMsg;
		if (NULL != zErrMsg)
		{
			sqlite3_free(zErrMsg);
		}
		_SQLiteError(pDb, str);
		return false;
	}

	// We define a relational table PrimaryVessels_SecondaryVessels between primary and secondary vessels.
	// Drop PrimaryVessels_SecondaryVessels table.
	pSqlQuery = "DROP TABLE IF EXISTS PrimaryVessels_SecondaryVessels";
	iReturnCode = sqlite3_exec( pDb, pSqlQuery, NULL, NULL, &zErrMsg );
	pStmt = NULL;
		
	// Create new PrimaryVessels_SecondaryVessels Table.
	 strCol = strPrim_SecVesselColumns;
	_ReplaceAll(strCol, "#", " ");
	strSqlQuery = "CREATE TABLE IF NOT EXISTS [PrimaryVessels_SecondaryVessels](\n" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();
	_CreateSQLTable(pDb, pSqlQuery);

	// Fill it.
	// Prepare INSERT into Vessels query
	strCol = strPrim_SecVesselColumns;
	_PrepareInsertQuery(strCol);
	strSqlQuery = "INSERT INTO PrimaryVessels_SecondaryVessels (" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();

	i = 0;
	int iPK;
	CString strHysSecVsl;
	bool bContinue = m_mapPKPrimVslIDSecVsl.GetFirst( iPK, strHysSecVsl );
		
	while( true == bContinue )
	{
		int iPKsec = 0;
			
		if( true == m_mapPKVessel.Get( strHysSecVsl, iPKsec ) )
		{
			++i;
			iReturnCode = sqlite3_prepare_v2( pDb, pSqlQuery, -1, &pStmt, NULL );
				
			if( SQLITE_OK != iReturnCode )
			{
				CString str = (CString)zErrMsg;
				if (NULL != zErrMsg)
				{
					sqlite3_free(zErrMsg);
				}
				_SQLiteError(pDb, str);
				return false;
			}

			sqlite3_bind_int( pStmt, 1, i );
			sqlite3_bind_int( pStmt, 2, iPK );
			sqlite3_bind_int( pStmt, 3, iPKsec );
			iReturnCode = sqlite3_step( pStmt );
				
			if( SQLITE_DONE != iReturnCode )
			{
				CString str = (CString)zErrMsg;
				if (NULL != zErrMsg)
				{
					sqlite3_free(zErrMsg);
				}
				_SQLiteError(pDb, str);
				return false;
			}

			sqlite3_finalize( pStmt );
		}
		else
		{
			CString str = _T("Missing secondary vessels in TADB") + strHysSecVsl;
			AfxMessageBox( str, MB_OK | MB_ICONSTOP );
		}

		bContinue = m_mapPKPrimVslIDSecVsl.GetNext( iPK, strHysSecVsl );
	}

	return true;
}

bool CTabDlgSpecActDev::_ExtractTecBoxes( sqlite3 *pDb )
{
	sqlite3_stmt *pStmt = NULL;
	char *zErrMsg = 0;
	int iReturnCode;
	char *pSqlQuery;
	int i = 0;
	CString str;
	CStringA strA;

	std::string strTecboxesColumns =
		"[Tecbox_id]#								INT	PRIMARY KEY	NOT NULL,\n" \
		"[Tecbox_HYSid]#							TEXT			NOT NULL,\n" \
		"[TecboxFamily_id]#							INT				NOT NULL,\n" \
		"[TecboxChar_id]#							INT						,\n" \
		"[MinVesselVolumeChar_id]#					INT						,\n" \
		"[TecboxNameST]#							TEXT			NOT NULL,\n" \
		"[TecboxPicture]#							TEXT					,\n" \
		"[TecboxArticleNo]#							TEXT			NOT NULL,\n" \
		"[TecboxIP]#								INT				NOT NULL,\n" \
		"[TecboxPowerConsumption]#					REAL			NOT NULL,\n" \
		"[TecboxSoundPressureLevel]#				REAL			NOT NULL,\n" \
		"[TecboxPumpCompressorCount]#				INT				NOT NULL,\n" \
		"[TecboxMaxSystemPressure]#					REAL			NOT NULL,\n" \
		"[TecboxMaxSystemVolume]#					REAL					,\n" \
		"[TecboxTmin]#								REAL			NOT NULL,\n" \
		"[TecboxTmax]#								REAL			NOT NULL,\n" \
		"[TecboxInstallationCode]#					TEXT			NOT NULL,\n" \
		"[TecboxHeight]#							REAL			NOT NULL,\n" \
		"[TecboxWidth]#								REAL			NOT NULL,\n" \
		"[TecboxDepth]#								REAL			NOT NULL,\n" \
		"[TecboxWeight]#							REAL			NOT NULL,\n" \
		"[TecboxFunctionCompressor]#				INT				NOT NULL,\n" \
		"[TecboxFunctionPump]#						INT 			NOT NULL,\n" \
		"[TecboxFunctionDegassing]#					INT 			NOT NULL,\n" \
		"[TecboxFunctionWaterMakeUp]#				INT 			NOT NULL,\n" \
		"[TecboxVariantCooling]#					INT 			NOT NULL,\n" \
		"[TecboxVariantBreakTank]#					INT 			NOT NULL,\n" \
		"[TecboxVariantExternalAir]#				INT 			NOT NULL,\n" \
		"[TecboxVariantHighFlowTemp]#				INT 			NOT NULL,\n" \
		"[TecboxIntegratedPrimaryVessel_id]#		INT						,\n" \
		"[TecboxMaxSecondaryVessel]#				INT						,\n" \
		"[TecboxBetaOnly]#							INT				NOT NULL,\n" \
		"[TecboxDeleted]#							INT 			NOT NULL";

	std::string strTecboxChars =
		"[TecboxChar_id]#						INT PRIMARY KEY     NOT NULL,\n"\
		"[TecboxCharNameST]#					TEXT				NOT NULL,\n"\
		"[TecboxCharWorkingPmax]#				REAL				NOT NULL,\n"\
		"[TecboxCharWorkingPmin]#				REAL			    NOT NULL";

	std::string strTecboxCharData =
		"[TecboxCharData_id]#					INT PRIMARY KEY     NOT NULL,\n"\
		"[TecboxChar_id]#						INT				    NOT NULL,\n"\
		"[qN]#									REAL				NOT NULL,\n"\
		"[pman_compr-pump]#						REAL				,\n"\
		"[pman_valve]#							REAL				,\n"\
		"[pxmax]#								REAL				,\n"\
		"[px1]#									REAL				,\n"\
		"[px2]#									REAL				   ";

	std::string strMinVesselVolumeChars =
		"[MinVesselVolumeChar_id]#				INT PRIMARY KEY     NOT NULL\n";

	std::string strMinVesselVolumeCharData =
		"[MinVesselVolumeCharData_id]#			INT PRIMARY KEY     NOT NULL,\n"\
		"[MinVesselVolumeChar_id]#				INT				    NOT NULL,\n"\
		"[pressure]#							REAL				NOT NULL,\n"\
		"[minVesselVolume]#						REAL				NOT NULL\n";

	std::string strTecboxes_BufferVessels =
		"[TecboxBufferVessel_id]#		INT PRIMARY KEY     NOT NULL,\n"\
		"[Tecbox_id]#					INT					NOT NULL,\n"\
		"[Vessel_id]#					INT				    NOT NULL";

	std::string strTecboxes_PowerSupplies =
		"[TecboxPowerSupply_id]#		INT PRIMARY KEY     NOT NULL,\n"\
		"[Tecbox_id]#					INT					NOT NULL,\n"\
		"[PowerSupply_id]#				INT				    NOT NULL";

	std::string strTecboxes_Frequencies =
		"[TecboxFrequency_id]#		INT PRIMARY KEY     NOT NULL,\n"\
		"[Tecbox_id]#				INT					NOT NULL,\n"\
		"[Frequency_id]#			INT				    NOT NULL";

	//////////////////////////////////////////////////////////////////
	// Fill cross references map.
	// Read TecBoxFamilies and fill m_mapPKTecboxFamilyName that give a link between TecboxFamily_id PK and TecboxFamily IDPTR
	m_pTab = (CTable *)(TASApp.GetpTADB()->Get(_T("TECBOXFAM_TAB")).MP);
	ASSERT(NULL != m_pTab);

	if (NULL == m_pTab)
	{
		AfxMessageBox(_T("Missing TECBOXFAM_TAB"), MB_OK | MB_ICONSTOP);
		return false;
	}

	// Extract TecboxFamily_id.
	pSqlQuery = "SELECT * from TecboxFamilies";
	iReturnCode = sqlite3_exec(pDb, pSqlQuery, this->cbFillTecboxFamiliesMap, this, &zErrMsg);

	if (SQLITE_OK != iReturnCode)
	{
		// Table doesn't exist.
		CString str = (CString)zErrMsg;
		if (NULL != zErrMsg)
		{
			sqlite3_free(zErrMsg);
		}
		_SQLiteError(pDb, str);
		return false;
	}
	//////////////////////////////////////////////////////////////////
	// Fill cross reference map with all curves needed
	m_pTab = (CTable *)(TASApp.GetpTADB()->Get(_T("TECBOX_TAB")).MP);
	ASSERT(NULL != m_pTab);
	CTemplateMap < CDB_TBCurve*, CDB_TecBox* > mapTBtoTBCurve;
	CTemplateMap < CDB_TBPlenoVento*, int> mapTBVentoPK;
	CTemplateMap < CDB_TBPlenoVento*, CDB_TBPlenoVento*> mapTBVentoVentoRef;
	CString strFamilyID;
	int iTecBoxFamIndex = 1;
	for (bool fCont = m_mapPKTecboxFamilyName.GetFirst(strFamilyID, iTecBoxFamIndex); true == fCont; fCont = m_mapPKTecboxFamilyName.GetNext(strFamilyID, iTecBoxFamIndex))
	{
		for (IDPTR IDPtr = m_pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_pTab->GetNext(IDPtr.MP))
		{
			CDB_TecBox *pTecbox = (CDB_TecBox *)(IDPtr.MP);

			if (strFamilyID != pTecbox->GetFamilyID())
			{
				continue;
			}

			if (true == pTecbox->IsDeleted())
			{
				continue;
			}

			if (NULL != pTecbox->GetPumpComprCurveIDPtr().MP)
			{
				mapTBtoTBCurve.Add((CDB_TBCurve*)pTecbox->GetPumpComprCurveIDPtr().MP, pTecbox);
			}
			else
			{
				CDB_TBPlenoVento *pVento = dynamic_cast<CDB_TBPlenoVento *>(pTecbox);
				if (NULL != pVento)
				{
					mapTBVentoPK.Add(pVento, 0);
					// try to find existing vento with same values
					CDB_TBPlenoVento *pVento1, *pVentoRef = NULL;
					bool bContinue = mapTBVentoVentoRef.GetFirst(pVento1, pVentoRef);
					bool bFound = false;
					while (true == bContinue && false == bFound)
					{
						if (pVento->GetWorkingPressRangeMax() == pVentoRef->GetWorkingPressRangeMax() &&
							pVento->GetWorkingPressRangeMin() == pVentoRef->GetWorkingPressRangeMin())
						{
							mapTBVentoVentoRef.Add(pVento, pVentoRef);
							bFound = true;
						}
						bContinue = mapTBVentoVentoRef.GetNext(pVento1, pVentoRef);
					}
					if (false == bFound)
					{
						// Add a new entry as reference
						mapTBVentoVentoRef.Add(pVento, pVento);
					}
				}
			}
		}
	}

	m_mapPKVessel.Clear();
	// Extract Vessel_id and Vessel_HYSid according new filling.
	pSqlQuery = "SELECT * from Vessels";
	iReturnCode = sqlite3_exec(pDb, pSqlQuery, this->cbFillVesselMap, this, &zErrMsg);

	if (SQLITE_OK != iReturnCode)
	{
		CString str = (CString)zErrMsg;
		if (NULL != zErrMsg)
		{
			sqlite3_free(zErrMsg);
		}
		_SQLiteError(pDb, str);
		return false;
	}
	//////////////////////////////////////////////////////////////////
	// Tecbox_Char and Tecbox_CharData
	//
	// Drop existing table if exist
	pSqlQuery = "DROP TABLE IF EXISTS [TecboxChars]";
	iReturnCode = sqlite3_exec(pDb, pSqlQuery, NULL, NULL, &zErrMsg);

	pSqlQuery = "DROP TABLE IF EXISTS [TecboxCharData]";
	iReturnCode = sqlite3_exec(pDb, pSqlQuery, NULL, NULL, &zErrMsg);

	// Create new TecboxCharData Table.
	std::string strCol = strTecboxCharData;
	_ReplaceAll(strCol, "#", "");
	std::string strSqlQuery = "CREATE TABLE IF NOT EXISTS [TecboxCharData](\n" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();
	_CreateSQLTable(pDb, pSqlQuery);

	//Retrieve all curves
	CTable *pTab = (CTable *)(TASApp.GetpTADB()->Get(_T("TBPUMPCOMPRESSCURVE_TAB")).MP);

	// Create an array with CDB_TBCurve * and TecboxChar_id
	CTemplateMap < int, CDB_TBCurve*> mapTBCurve;

	// Prepare INSERT into TecboxCharData query
	strCol = strTecboxCharData;
	_PrepareInsertQuery(strCol);
	strSqlQuery = "INSERT INTO [TecboxCharData] (" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();

	int iTBCharKey = 1;
	int iTBCharDataKey = 1;
	int iMinVesselVolumeChar_id = 0;
	for (IDPTR idptrTBC = pTab->GetFirst(); NULL != idptrTBC.MP; idptrTBC = pTab->GetNext())
	{
		CDB_TBCurve * pTBCurve = (CDB_TBCurve *) idptrTBC.MP;
		// Is curve needed?
		if (false == mapTBtoTBCurve.IsExist(pTBCurve))
		{
			continue;
		}
		CDB_TecBox *pTecBox = NULL;
		mapTBtoTBCurve.Get(pTBCurve, pTecBox);
		//Fill all data
		CCurveFitter* pCurve = pTBCurve->GetpCurve();
		for (int i = 0; i < pCurve->GetSize(); ++i)
		{
			// Fill table TecboxChars Table
			unsigned int uiField = 1;

			// Primary Key
			iReturnCode = sqlite3_prepare_v2(pDb, pSqlQuery, -1, &pStmt, NULL);
			iReturnCode = sqlite3_bind_int(pStmt, uiField++, iTBCharDataKey++);
		
			//TecboxChar_id
			iReturnCode = sqlite3_bind_int(pStmt, uiField++, iTBCharKey);

			// Each Line; qN
			iReturnCode = sqlite3_bind_double(pStmt, uiField++, pCurve->GetPointX(i) );

			
			if (true == pTecBox->IsVariantExternalAir()) // Compresso CX
			{
				// Skip 2 first fields
				uiField += 2;
				// pour cx f0 = pxmax, f1 = px1 f2 = px2
				// f(0)
				iReturnCode = sqlite3_bind_double(pStmt, uiField++, pCurve->GetPointF(i, 0));
				// f(1)
				iReturnCode = sqlite3_bind_double(pStmt, uiField++, pCurve->GetPointF(i, 1));

				if (pCurve->GetFuncNumber() > 2) // CX 2 or 3 curve 
				{
					iReturnCode = sqlite3_bind_double(pStmt, uiField++, pCurve->GetPointF(i, 2));
				}
			}
			else
			{
				// f(0)
				iReturnCode = sqlite3_bind_double(pStmt, uiField++, pCurve->GetPointF(i, 0));
				// f(1)
				if (pCurve->GetFuncNumber() > 1) 
				{
					iReturnCode = sqlite3_bind_double(pStmt, uiField++, pCurve->GetPointF(i, 1));
				}
			}

			iReturnCode = sqlite3_step(pStmt);
			
			sqlite3_finalize(pStmt);
		}
		mapTBCurve.Add(iTBCharKey++ , pTBCurve);
	}
	// Create new TecboxChars Table.
	strCol = strTecboxChars;
	_ReplaceAll(strCol, "#", "");
	strSqlQuery = "CREATE TABLE IF NOT EXISTS [TecboxChars](\n" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();
	_CreateSQLTable(pDb, pSqlQuery);

	// Prepare INSERT into TecboxCharData query
	strCol = strTecboxChars;
	_PrepareInsertQuery(strCol);
	strSqlQuery = "INSERT INTO [TecboxChars] (" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();

	// Fill it 
	CDB_TBCurve * pTBCurve = NULL;
	bool bContinue = mapTBCurve.GetFirst( iTBCharKey , pTBCurve );
	int iMaxPK = 0;
	while (true == bContinue)
	{
		unsigned int uiField = 1;
		// Primary Key
		iReturnCode = sqlite3_prepare_v2(pDb, pSqlQuery, -1, &pStmt, NULL);
		sqlite3_bind_int(pStmt, uiField++, iTBCharKey);
		iMaxPK = max( iMaxPK, iTBCharKey);
		// Curve name
		strA = _NormalizeName( CStringA("sr_TBCurve_") + CStringA(pTBCurve->GetCurveName() ) );
		sqlite3_bind_text(pStmt, uiField++, (LPCSTR)strA, -1, SQLITE_STATIC);

		sqlite3_bind_double(pStmt, uiField++, pTBCurve->GetPmax());

		sqlite3_bind_double(pStmt, uiField++, pTBCurve->GetPmin());

		iReturnCode = sqlite3_step(pStmt);

		bContinue = mapTBCurve.GetNext(iTBCharKey, pTBCurve);

		sqlite3_finalize(pStmt);
	}

	++iMaxPK;
	CDB_TBPlenoVento *pVento = NULL;
	bContinue = mapTBVentoPK.GetFirst(pVento, iTBCharKey);
	// Specific for PlenoVento
	while (true == bContinue)
	{
		unsigned int uiField = 1;
		// Try to find an entry with same data
		CDB_TBPlenoVento *pVentoRef = NULL;
		mapTBVentoVentoRef.Get(pVento, pVentoRef);
		// In SQLite we will add only Vento Ref
		if (pVento == pVentoRef)
		{
			// Primary Key
			iReturnCode = sqlite3_prepare_v2(pDb, pSqlQuery, -1, &pStmt, NULL);
			sqlite3_bind_int(pStmt, uiField++, iMaxPK);
			mapTBVentoPK.Add(pVento, iMaxPK++);

			// Curve name
			strA = _NormalizeName(CStringA("sr_Vento_") + CStringA(pVento->GetName()));
			sqlite3_bind_text(pStmt, uiField++, (LPCSTR)strA, -1, SQLITE_STATIC);

			sqlite3_bind_double(pStmt, uiField++, pVento->GetWorkingPressRangeMax());

			sqlite3_bind_double(pStmt, uiField++, pVento->GetWorkingPressRangeMin());

			iReturnCode = sqlite3_step(pStmt);

			sqlite3_finalize(pStmt);
		}
		bContinue = mapTBVentoPK.GetNext(pVento, iTBCharKey);
	}
	//////////////////////////////////////////////////////////////////
	// MinVesselVolumeChars table and MinVesselVolumeCharData
	//
	// Drop tables if exist
	pSqlQuery = "DROP TABLE IF EXISTS [MinVesselVolumeCharData]";
	iReturnCode = sqlite3_exec(pDb, pSqlQuery, NULL, NULL, &zErrMsg);

	pSqlQuery = "DROP TABLE IF EXISTS [MinVesselVolumeChars]";
	iReturnCode = sqlite3_exec(pDb, pSqlQuery, NULL, NULL, &zErrMsg);

	// Create new MinVesselVolumeChars table and MinVesselVolumeCharData.
	strCol = strMinVesselVolumeCharData;
	_ReplaceAll(strCol, "#", "");
	strSqlQuery = "CREATE TABLE IF NOT EXISTS [MinVesselVolumeCharData](\n" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();
	_CreateSQLTable(pDb, pSqlQuery);

	strCol = strMinVesselVolumeChars;
	_ReplaceAll(strCol, "#", "");
	strSqlQuery = "CREATE TABLE IF NOT EXISTS [MinVesselVolumeChars](\n" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();
	_CreateSQLTable(pDb, pSqlQuery);

	// Create list of vento with MinVolume
	// List of vento are already stored in a mapTBVentoPK
	CTemplateMap < CDB_TBPlenoVento*, CDB_TBPlenoVento*> mapTBMinVolVentoVentoRef;

	pVento = NULL;
	iTBCharKey = 0;
	bContinue = mapTBVentoPK.GetFirst(pVento, iTBCharKey);
	// Specific for PlenoVento
	while (true == bContinue)
	{
		CDB_TBPlenoVento *pVentoRef = NULL;
		CDB_TBPlenoVento *pVento1 = NULL;
		CArray<CDB_TBPlenoVento::_VesselVolumeLimit> *parrVentoVolumeLimit, *parrVentoRefVolumeLimit;
		parrVentoVolumeLimit = pVento->GetMinimumVesselVolumeArray();
		//try to find existing pVentoRef 
		bool bContVentoRefMap = mapTBMinVolVentoVentoRef.GetFirst(pVento1, pVentoRef);
		bool begual = true;
		while ( true == bContVentoRefMap )
		{
			parrVentoRefVolumeLimit = pVentoRef->GetMinimumVesselVolumeArray();
			if (parrVentoRefVolumeLimit->GetSize() == parrVentoVolumeLimit->GetSize())
			{
				for (int i = 0; i < parrVentoRefVolumeLimit->GetSize() ; ++i)
				{
					if (parrVentoRefVolumeLimit->GetAt(i).m_dMinimumVolume != parrVentoVolumeLimit->GetAt(i).m_dMinimumVolume)
					{
						begual = false;
						break;
					}
					if (parrVentoRefVolumeLimit->GetAt(i).m_dPmax != parrVentoVolumeLimit->GetAt(i).m_dPmax)
					{
						begual = false;
						break;
					}
					else if (-1 == parrVentoVolumeLimit->GetAt(i).m_dPmax ) // When pmax == -1 we have to test pmax of device
					{
						if ( pVento->GetPmaxmax() != pVentoRef->GetPmaxmax() )
						{
							begual = false;
							break;
						}
					}
					if (parrVentoRefVolumeLimit->GetAt(i).m_dPmin != parrVentoVolumeLimit->GetAt(i).m_dPmin)
					{
						begual = false;
						break;
					}
				}
				// Found matching
				break;
			}
			bContVentoRefMap = mapTBMinVolVentoVentoRef.GetNext(pVento1, pVentoRef);
			if (false == bContVentoRefMap)
			{
				begual = false;
			}
		}
		if ( true == begual && NULL != pVentoRef )
		{
			mapTBMinVolVentoVentoRef.Add(pVento, pVentoRef);
		}
		else
		{
			// New ref
			mapTBMinVolVentoVentoRef.Add(pVento, pVento);
		}

		bContinue = mapTBVentoPK.GetNext(pVento, iTBCharKey);
	}
	//////////////////////////////////////////////////////////////////
	// Fill MinVesselVolumeChars table
	//
	CTemplateMap < CDB_TBPlenoVento*, int> mapTBMinVolVentoRef_PK;
	strCol = strMinVesselVolumeChars;
	_PrepareInsertQuery(strCol);
	strSqlQuery = "INSERT INTO [MinVesselVolumeChars] (" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();

	CDB_TBPlenoVento *pVentoRef = NULL;
	bContinue = mapTBMinVolVentoVentoRef.GetFirst(pVento, pVentoRef);
	iMaxPK = 0;
	while (true == bContinue)
	{
		// Test if already added
		bool bSkipIt = mapTBMinVolVentoRef_PK.IsExist(pVentoRef);
		// Add one entry for each pVentoRef
		if (false == bSkipIt)
		{
			++iMaxPK;
			unsigned int uiField = 1;
			// Save pVentoRef and PK into the map
			mapTBMinVolVentoRef_PK.Add(pVentoRef, iMaxPK);
			// Primary Key
			iReturnCode = sqlite3_prepare_v2(pDb, pSqlQuery, -1, &pStmt, NULL);
			sqlite3_bind_int(pStmt, uiField++, iMaxPK);
			iReturnCode = sqlite3_step(pStmt);
			sqlite3_finalize(pStmt);
		}
		bContinue = mapTBMinVolVentoVentoRef.GetNext(pVento, pVentoRef);
	}
	//////////////////////////////////////////////////////////////////
	// Fill MinVesselVolumeCharData table
	//
	strCol = strMinVesselVolumeCharData;
	_PrepareInsertQuery(strCol);
	strSqlQuery = "INSERT INTO [MinVesselVolumeCharData] (" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();

	pVentoRef = NULL;
	int iVentoRefPK = 0;
	CArray< CDB_TBPlenoVento *> arrVentoRefAdded;
	bContinue = mapTBMinVolVentoVentoRef.GetFirst(pVento, pVentoRef);
	iMaxPK = 0;
	while (true == bContinue)
	{
		bool bSkipIt = false;
		for (int i = 0; i < arrVentoRefAdded.GetCount() && false == bSkipIt; ++i)
		{
			if (arrVentoRefAdded.GetAt(i) == pVentoRef)
			{
				bSkipIt = true;
			}
		}
		if (false ==  bSkipIt)
		{
			arrVentoRefAdded.Add(pVentoRef);
			CArray<CDB_TBPlenoVento::_VesselVolumeLimit> *parrVentoRefVolumeLimit;
			mapTBMinVolVentoRef_PK.Get(pVentoRef, iVentoRefPK);
			parrVentoRefVolumeLimit = pVentoRef->GetMinimumVesselVolumeArray();
			for (int i = 0; i < parrVentoRefVolumeLimit->GetSize(); ++i)
			{
				// Add one entry for each pVentoRef
				++iMaxPK;
				unsigned int uiField = 1;
				// Primary Key
				iReturnCode = sqlite3_prepare_v2(pDb, pSqlQuery, -1, &pStmt, NULL);
				sqlite3_bind_int(pStmt, uiField++, iMaxPK);

				// MinVesselVolumeChar_id PK
				sqlite3_bind_int(pStmt, uiField++, iVentoRefPK);

				sqlite3_bind_double(pStmt, uiField++, parrVentoRefVolumeLimit->GetAt(i).m_dPmin);

				sqlite3_bind_double(pStmt, uiField++, parrVentoRefVolumeLimit->GetAt(i).m_dMinimumVolume);

				iReturnCode = sqlite3_step(pStmt);

				sqlite3_finalize(pStmt);
			}

			// Add pmax
			int i = parrVentoRefVolumeLimit->GetSize()-1;
			if (i>=0 && -1 == parrVentoRefVolumeLimit->GetAt(i).m_dPmax)
			{
				++iMaxPK;
				unsigned int uiField = 1;
				// Primary Key
				iReturnCode = sqlite3_prepare_v2(pDb, pSqlQuery, -1, &pStmt, NULL);
				sqlite3_bind_int(pStmt, uiField++, iMaxPK);

				sqlite3_bind_int(pStmt, uiField++, iVentoRefPK);

				sqlite3_bind_double(pStmt, uiField++, pVentoRef->GetPmaxmax());

				sqlite3_bind_double(pStmt, uiField++, parrVentoRefVolumeLimit->GetAt(i).m_dMinimumVolume);

				iReturnCode = sqlite3_step(pStmt);

				sqlite3_finalize(pStmt);
			}
		}

		bContinue = mapTBMinVolVentoVentoRef.GetNext(pVento, pVentoRef);
	}

	//////////////////////////////////////////////////////////////////
	// Tecbox table
	//
	// Extract Tecbox_id and Tecbox_HYSid.
	m_pTab = (CTable *)(TASApp.GetpTADB()->Get(_T("TECBOX_TAB")).MP);
	ASSERT(NULL != m_pTab);

	if (NULL == m_pTab)
	{
		AfxMessageBox(_T("Missing TECBOX_TAB"), MB_OK | MB_ICONSTOP);
		return false;
	}

	iMaxPK = 0;
	pSqlQuery = "SELECT * from Tecboxes";
	iReturnCode = sqlite3_exec(pDb, pSqlQuery, this->cbFillTecBoxesMap, this, &zErrMsg);

	if (SQLITE_OK == iReturnCode)
	{
		// If Table doesn't exist no problem we will create it from scratch.
		// If table exist extract PK of Tecbox to use it when we redo the table.
		// Extract max PK.
		CString strHYSid;
		int iPK;
		bool bContinue = m_mapPKTecbox.GetFirst(strHYSid, iPK);

		while (true == bContinue)
		{
			if (iPK > iMaxPK)
			{
				iMaxPK = iPK;
			}

			bContinue = m_mapPKTecbox.GetNext(strHYSid, iPK);
		}
	}

	// Drop tecboxes table
	pSqlQuery = "DROP TABLE IF EXISTS [Tecboxes]";
	iReturnCode = sqlite3_exec(pDb, pSqlQuery, NULL, NULL, &zErrMsg);

	// Create new Tecbox Table.
	strCol = strTecboxesColumns;
	_ReplaceAll(strCol, "#", "");
	strSqlQuery = "CREATE TABLE IF NOT EXISTS [Tecboxes](\n" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();
	_CreateSQLTable(pDb, pSqlQuery);

	// Add Tecbox from HySelect into the SQLite Tecbox table.
	m_pTab = (CTable *)(TASApp.GetpTADB()->Get(_T("TECBOX_TAB")).MP);
	ASSERT(NULL != m_pTab);

	// Prepare INSERT into Tecbox query
	strCol = strTecboxesColumns;
	_PrepareInsertQuery(strCol);
	strSqlQuery = "INSERT INTO [Tecboxes] (" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();

	iTecBoxFamIndex = 1;
	while (true == m_mapPKTecboxFamilyName.GetSecond(strFamilyID, iTecBoxFamIndex))
	{
		std::vector<CDB_TecBox *> arrTecbox;

		// Sort Tecbox based on ....Power/Weight TODO
		for (IDPTR IDPtr = m_pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_pTab->GetNext())
		{
			CDB_TecBox *pTecbox = (CDB_TecBox *)(IDPtr.MP);

			if (strFamilyID != pTecbox->GetFamilyID())
			{
				continue;
			}

			if (true == pTecbox->IsDeleted())
			{
				continue;
			}

			arrTecbox.push_back(pTecbox);
		}

		std::sort(arrTecbox.begin(), arrTecbox.end(), CTabDlgSpecActDev::SortTecboxes);

		// Enter Tecbox in SQLiteDB.
		int j = 0;

		for (unsigned j = 0; j < arrTecbox.size(); j++)
		{
			CDB_TecBox *pTecbox = (CDB_TecBox *)arrTecbox[j];
			IDPTR idptr = pTecbox->GetIDPtr();
			int iPK = 0;

			if (true == m_mapPKTecbox.Get(idptr.ID, iPK))
			{
				i = iPK;
			}
			else //Not found
			{
				iMaxPK++;
				i = iMaxPK;
			}
			unsigned int uiField = 1;
			// Primary Key
			iReturnCode = sqlite3_prepare_v2(pDb, pSqlQuery, -1, &pStmt, NULL);
			sqlite3_bind_int(pStmt, uiField++, i);
			
			// HYSid
			strA = CStringA(idptr.ID);
			sqlite3_bind_text(pStmt, uiField++, (LPCSTR)strA, -1, SQLITE_STATIC);
			
			// Family_id
			sqlite3_bind_int(pStmt, uiField++, iTecBoxFamIndex);

			// char_id 
			iTBCharKey = 0;
			pTBCurve = (CDB_TBCurve*) pTecbox->GetPumpComprCurveIDPtr().MP;
			//Vento?
			CDB_TBPlenoVento *pVento = dynamic_cast<CDB_TBPlenoVento*>(pTecbox);
			if (NULL != pTBCurve)
			{
				mapTBCurve.GetSecond(iTBCharKey, pTBCurve);
			}
			else
			{
				CDB_TBPlenoVento *pVentoRef = pVento;
				if (NULL != pVento)
				{
					mapTBVentoVentoRef.Get(pVento, pVentoRef);
					mapTBVentoPK.Get(pVentoRef, iTBCharKey);
				}
			}
			if (0 != iTBCharKey)
			{
				sqlite3_bind_int(pStmt, uiField, iTBCharKey);
			}
			uiField++;

			//MinVesselVolumeChar_id
			if (NULL != pVento)
			{
				CDB_TBPlenoVento *pVentoRef = NULL;
				mapTBMinVolVentoVentoRef.Get(pVento, pVentoRef);
				int iMinVesselVolumeChar_id = 0;
				mapTBMinVolVentoRef_PK.Get(pVentoRef, iMinVesselVolumeChar_id);
				if (0 != iMinVesselVolumeChar_id)
				{
					iReturnCode = sqlite3_bind_int(pStmt, uiField, iMinVesselVolumeChar_id);
				}
			}
			uiField++;

			// Tecbox name
			strA = _NormalizeName(CStringA("sr_TecboxName_") + CStringA(pTecbox->GetName()));
			sqlite3_bind_text(pStmt, uiField++, (LPCSTR)strA, -1, SQLITE_STATIC);

			// Picture file name
			strA = pTecbox->GetProdPicID();
			strA.MakeLower();
			int iPos = strA.Find("px_");
			if (iPos > -1)
			{
				strA.Delete(iPos, 3);
			}
			if (0 != strA.Compare("no_id"))
			{
				strA.Remove('_');
				sqlite3_bind_text(pStmt, uiField, (LPCSTR)strA, -1, SQLITE_STATIC);
			}
			uiField++;
			// Tecbox article number
			strA = CStringA(pTecbox->GetArtNum());
			sqlite3_bind_text(pStmt, uiField++, (LPCSTR)strA, -1, SQLITE_STATIC);

			// Tecbox IP
			strA = CStringA(pTecbox->GetIPxx());
			sqlite3_bind_text(pStmt, uiField++, (LPCSTR)strA, -1, SQLITE_STATIC);

			// Tecbox Power consumption
			sqlite3_bind_double(pStmt, uiField++, pTecbox->GetPower());
			
			sqlite3_bind_double(pStmt, uiField++, pTecbox->GetSoundPressureLevel());

			sqlite3_bind_int(pStmt, uiField++, pTecbox->GetNbrPumpCompressor());

			sqlite3_bind_double(pStmt, uiField++, pTecbox->GetPmaxmax());

			//TecboxMaxSystemVolume
			CDB_TBPlenoVento *pPleno = dynamic_cast<CDB_TBPlenoVento *>(pTecbox);
			if (NULL != pPleno)
			{
				sqlite3_bind_double(pStmt, uiField, pPleno->GetSystemVolume());
			}
			uiField++;

			sqlite3_bind_double(pStmt, uiField++, pTecbox->GetTmin());
 
			sqlite3_bind_double(pStmt, uiField++, pTecbox->GetTmax());

			// Tecbox Installation code
			strA = _GetInstallationLayout(pTecbox->GetInstallationLayout());
			sqlite3_bind_text(pStmt, uiField++, (LPCSTR)strA, -1, SQLITE_STATIC);

			sqlite3_bind_double(pStmt, uiField++, pTecbox->GetHeight());
			sqlite3_bind_double(pStmt, uiField++, pTecbox->GetWidth());
			sqlite3_bind_double(pStmt, uiField++, pTecbox->GetDepth());
			sqlite3_bind_double(pStmt, uiField++, pTecbox->GetWeight());

			// Tecbox functions (-1/0/1)  TODO why -1
			sqlite3_bind_int(pStmt, uiField++, pTecbox->IsFctCompressor() ? 1 : pTecbox->IsFctExcludedCompressor() ? -1 : 0);
			sqlite3_bind_int(pStmt, uiField++, pTecbox->IsFctPump() ? 1 : pTecbox->IsFctExcludedPump() ? -1 : 0);
			sqlite3_bind_int(pStmt, uiField++, pTecbox->IsFctDegassing() ? 1 : 0);
			sqlite3_bind_int(pStmt, uiField++, pTecbox->IsFctWaterMakeUp() ? 1 : 0);

			// Tecbox variant (0/1)
			sqlite3_bind_int(pStmt, uiField++, pTecbox->IsVariantCooling());
			sqlite3_bind_int(pStmt, uiField++, pTecbox->IsVariantBreakTank());
			sqlite3_bind_int(pStmt, uiField++, pTecbox->IsVariantExternalAir());
			sqlite3_bind_int(pStmt, uiField++, pTecbox->IsVariantHighFlowTemperature());
			IDPTR idptrIV = pTecbox->GetIntegratedVesselIDPtr();
			if (NULL != idptrIV.MP && CDB_TecBox::ePMVariant_IntegratedPrimaryVessel == (CDB_TecBox::ePMVariant_IntegratedPrimaryVessel & pTecbox->GetTecBoxVariant()))
			{
				int iPKVssl = 0;
				m_mapPKVessel.Get (idptrIV.ID, iPKVssl);
				sqlite3_bind_int(pStmt, uiField++, iPKVssl);
			}
			else
			{
				uiField++;
			}
			//TecboxMaxSecondaryVessel
			int iMaxSecondaryVesssels = pTecbox->GetMaximumSecondaryVessels();
			if ( iMaxSecondaryVesssels > -1)
			{
				sqlite3_bind_int(pStmt, uiField++, iMaxSecondaryVesssels);
			}
			else
			{
				uiField++;
			}
			// BetaOnly
			int ifoo = 0;
			sqlite3_bind_int(pStmt, uiField++, ifoo);

			ifoo = pTecbox->IsDeleted();
			sqlite3_bind_int(pStmt, uiField++, ifoo);

			iReturnCode = sqlite3_step(pStmt);

			if (SQLITE_DONE != iReturnCode)
			{
				CString str = (CString)zErrMsg;
				if (NULL != zErrMsg)
				{
					sqlite3_free(zErrMsg);
				}
				_SQLiteError(pDb, str);
				return false;
			}

			sqlite3_finalize(pStmt);

		}

		++iTecBoxFamIndex;
	};

	////////////////////////////////////////////////////
	// For each tecbox verify if a buffer vessel exist
	// if yes add relation in the buffer vessel table
	////////////////////////////////////////////////////
	m_mapPKTecbox.Clear();
	// Extract tecbox_id and tecbox_HYSid according new filling.
	pSqlQuery = "SELECT * from Tecboxes";
	iReturnCode = sqlite3_exec(pDb, pSqlQuery, this->cbFillTecBoxesMap, this, &zErrMsg);
	if (SQLITE_OK != iReturnCode)
	{
		CString str = (CString)zErrMsg;
		if (NULL != zErrMsg)
		{
			sqlite3_free(zErrMsg);
		}
		_SQLiteError(pDb, str);
		return false;
	}

	// Drop previous buffer vessel table and create a new ones
	// Drop Tecboxes_BufferVessels table
	pSqlQuery = "DROP TABLE IF EXISTS [Tecboxes_BufferVessels]";
	iReturnCode = sqlite3_exec(pDb, pSqlQuery, NULL, NULL, &zErrMsg);

	// Create new Tecboxes_BufferVessels Table.
	strCol = strTecboxes_BufferVessels;
	_ReplaceAll(strCol, "#", "");
	strSqlQuery = "CREATE TABLE IF NOT EXISTS [Tecboxes_BufferVessels](\n" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();
	_CreateSQLTable(pDb, pSqlQuery);

	// Prepare INSERT into Tecboxes_BufferVessels query
	strCol = strTecboxes_BufferVessels;
	_PrepareInsertQuery(strCol);
	strSqlQuery = "INSERT INTO [Tecboxes_BufferVessels] (" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();

	CString strHYSid;
	int iPKTecbox = 0;
	bContinue = m_mapPKTecbox.GetFirst(strHYSid, iPKTecbox);

	int iKey = 1;
	while (true == bContinue)
	{
		IDPTR idptrTB = m_pTab->Get(strHYSid);
		CDB_TecBox *pTecbox = (CDB_TecBox *)idptrTB.MP;

		if ( CDB_TecBox::ePMVariant_IntegratedPrimaryVessel != (CDB_TecBox::ePMVariant_IntegratedPrimaryVessel & pTecbox->GetTecBoxVariant() ) )
		{
			CSelectPMList PMList;
			std::map<int, CSelectedTransfero::BufferVesselData> mapBufVssl;
			PMList.SetpTADB(TASApp.GetpTADB());
			PMList.SetpTADS(TASApp.GetpTADS());
			PMList.GetTransferoBufferVesselList(pTecbox, &mapBufVssl);

			for (auto &iter : mapBufVssl)
			{
				CDB_Vessel * pVssl = iter.second.m_pclVessel;
				if (NULL != pVssl)
				{
					iReturnCode = sqlite3_prepare_v2(pDb, pSqlQuery, -1, &pStmt, NULL);

					if (SQLITE_OK != iReturnCode)
					{
						CString str = (CString)zErrMsg;
						if (NULL != zErrMsg)
						{
							sqlite3_free(zErrMsg);
						}
						_SQLiteError(pDb, str);
						return false;
					}
					int iPKVssl = 0;
					m_mapPKVessel.Get(pVssl->GetIDPtr().ID, iPKVssl);
					sqlite3_bind_int(pStmt, 1, iKey++);
					sqlite3_bind_int(pStmt, 2, iPKTecbox);
					sqlite3_bind_int(pStmt, 3, iPKVssl);
					iReturnCode = sqlite3_step(pStmt);

					if (SQLITE_DONE != iReturnCode)
					{
						CString str = (CString)zErrMsg;
						if (NULL != zErrMsg)
						{
							sqlite3_free(zErrMsg);
						}
						_SQLiteError(pDb, str);
						return false;
					}

					sqlite3_finalize(pStmt);
				}
			}
		}

		bContinue = m_mapPKTecbox.GetNext(strHYSid, iPKTecbox);
	}

	// Drop Tecboxes_PowerSupplies table
	pSqlQuery = "DROP TABLE IF EXISTS [Tecboxes_PowerSupplies]";
	iReturnCode = sqlite3_exec(pDb, pSqlQuery, NULL, NULL, &zErrMsg);

	// Create new Tecboxes_PowerSupplies Table.
	strCol = strTecboxes_PowerSupplies;
	_ReplaceAll(strCol, "#", "");
	strSqlQuery = "CREATE TABLE IF NOT EXISTS [Tecboxes_PowerSupplies](\n" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();
	_CreateSQLTable(pDb, pSqlQuery);

	// Prepare INSERT into Tecboxes_PowerSupplies query
	strCol = strTecboxes_PowerSupplies;
	_PrepareInsertQuery(strCol);
	strSqlQuery = "INSERT INTO [Tecboxes_PowerSupplies] (" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();

	iPKTecbox = 0;
	bContinue = m_mapPKTecbox.GetFirst(strHYSid, iPKTecbox);

	iKey = 1;
	while (true == bContinue)
	{
		IDPTR idptrTB = m_pTab->Get(strHYSid);
		CDB_TecBox *pTecbox = (CDB_TecBox *)idptrTB.MP;

		for (int i = 0; i < (int)pTecbox->GetNumOfPowerSupplyIDptr(); ++i)
		{
			IDPTR idptrPS = pTecbox->GetPowerSupplyIDPtr(i);
			CDB_StringID *pstrID = (CDB_StringID *)idptrPS.MP;
			int iPKPS = _wtoi(pstrID->GetIDstr2());
			iReturnCode = sqlite3_prepare_v2(pDb, pSqlQuery, -1, &pStmt, NULL);
			if (SQLITE_OK != iReturnCode)
			{
				CString str = (CString)zErrMsg;
				if (NULL != zErrMsg)
				{
					sqlite3_free(zErrMsg);
				}
				_SQLiteError(pDb, str);
				return false;
			}
			sqlite3_bind_int(pStmt, 1, iKey++);
			sqlite3_bind_int(pStmt, 2, iPKTecbox);
			sqlite3_bind_int(pStmt, 3, iPKPS);
			iReturnCode = sqlite3_step(pStmt);

			if (SQLITE_DONE != iReturnCode)
			{
				CString str = (CString)zErrMsg;
				if (NULL != zErrMsg)
				{
					sqlite3_free(zErrMsg);
				}
				_SQLiteError(pDb, str);
				return false;
			}

			sqlite3_finalize(pStmt);
		}
		bContinue = m_mapPKTecbox.GetNext(strHYSid, iPKTecbox);
	}

	// Drop Tecboxes_PowerFrequencies table
	pSqlQuery = "DROP TABLE IF EXISTS [Tecboxes_PowerFrequencies]";
	iReturnCode = sqlite3_exec(pDb, pSqlQuery, NULL, NULL, &zErrMsg);

	// Create new Tecboxes_PowerFrequencies Table.
	strCol = strTecboxes_Frequencies;
	_ReplaceAll(strCol, "#", "");
	strSqlQuery = "CREATE TABLE IF NOT EXISTS [Tecboxes_PowerFrequencies](\n" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();
	_CreateSQLTable(pDb, pSqlQuery);

	// Prepare INSERT into Tecboxes_PowerFrequencies query
	strCol = strTecboxes_Frequencies;
	_PrepareInsertQuery(strCol);
	strSqlQuery = "INSERT INTO [Tecboxes_PowerFrequencies] (" + strCol + ")";
	pSqlQuery = (char*)strSqlQuery.c_str();

	iPKTecbox = 0;
	bContinue = m_mapPKTecbox.GetFirst(strHYSid, iPKTecbox);

	iKey = 1;
	while (true == bContinue)
	{
		IDPTR idptrTB = m_pTab->Get(strHYSid);
		CDB_TecBox *pTecbox = (CDB_TecBox *)idptrTB.MP;

		for (int i = 0; i < (int)pTecbox->GetNumOfPowerSupplyIDptr(); ++i)
		{
			IDPTR idptrHZ = pTecbox->GetFrequencyIDPtr();
			CDB_StringID *pstrID = (CDB_StringID *)idptrHZ.MP;
			int iPKHZ = _wtoi(pstrID->GetIDstr2());
			iReturnCode = sqlite3_prepare_v2(pDb, pSqlQuery, -1, &pStmt, NULL);
			if (SQLITE_OK != iReturnCode)
			{
				CString str = (CString)zErrMsg;
				if (NULL != zErrMsg)
				{
					sqlite3_free(zErrMsg);
				}
				_SQLiteError(pDb, str);
				return false;
			}
			sqlite3_bind_int(pStmt, 1, iKey++);
			sqlite3_bind_int(pStmt, 2, iPKTecbox);
			sqlite3_bind_int(pStmt, 3, iPKHZ);
			iReturnCode = sqlite3_step(pStmt);

			if (SQLITE_DONE != iReturnCode)
			{
				CString str = (CString)zErrMsg;
				if (NULL != zErrMsg)
				{
					sqlite3_free(zErrMsg);
				}
				_SQLiteError(pDb, str);
				return false;
			}
			sqlite3_finalize(pStmt);
		}
		bContinue = m_mapPKTecbox.GetNext(strHYSid, iPKTecbox);
	}
	   	  
	m_mapPKTecbox.Clear();
	mapTBCurve.Clear();
	return true;
}
