#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "HMInclude.h"
#include "DrawSet.h"
#include "DlgLeftTabBase.h"
#include "DlgLeftTabSelP.h"
#include "RViewDescription.h"
#include "SelProdArticle.h"
#include "SelProdPageBase.h"
#include "SelProdPageDiversityFactor.h"

// For TChart
#include "TeeChartDefines.h"
#include "axes.h"
#include "axis.h"
#include "axislabels.h"
#include "axislabelsitem.h"
#include "axistitle.h"
#include "brush.h"
#include "canvas.h"
#include "gradient.h"
#include "horizbarseries.h"
#include "legend.h"
#include "legenditem.h"
#include "legenditems.h"
#include "legendsymbol.h"
#include "marks.h"
#include "marksitem.h"
#include "pen.h"
#include "series.h"
#include "strings.h"
#include "teefont.h"
#include "titles.h"
#include "valuelist.h"
#include "valuelists.h"

CSelProdPageDiversityFactor::CSelProdPageDiversityFactor( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::DIVERSITYFACTOR, pclArticleGroupList )
{
	m_pclSelectionTableHM = NULL;
	m_hHandOpened = (HICON)INVALID_HANDLE_VALUE;
	m_pclSelectionTableHM = NULL;
	m_fCursorChanged = false;
}

CSelProdPageDiversityFactor::~CSelProdPageDiversityFactor( void )
{
	MM_UnregisterAllNotificationHandlers( this );

	if( NULL != m_TChartValves.GetSafeHwnd() )
	{
		m_TChartValves.DestroyWindow();
	}

	if( NULL != m_TChartPipes.GetSafeHwnd() )
	{
		m_TChartPipes.DestroyWindow();
	}
}

void CSelProdPageDiversityFactor::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageDiversityFactor::PreInit( HMvector &vecHMList )
{
	CTableHM *pclTable = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable());

	if( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	m_pclSelectionTableHM = ( '\0' != *pclTable->GetFirst().ID ) ? pclTable : NULL;
	return HasSomethingToDisplay();
}

bool CSelProdPageDiversityFactor::Init( bool fResetOrder, bool fPrint )
{
	if( false == HasSomethingToDisplay() )
		return false;

	// Call base class method to get a new 'Sheet'.
	CSheetDescription* pclSDDiversityFactor = CMultiSpreadBase::CreateSSheet( SD_DF );
	if( NULL == pclSDDiversityFactor || NULL == pclSDDiversityFactor->GetSSheetPointer() )
		return false;
	
	CSSheet* pclSheet = pclSDDiversityFactor->GetSSheetPointer();
	if( false == CSelProdPageBase::PrepareSheet( pclSDDiversityFactor, DFColumnDescription::CD_Pointer, fPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_DF );
		return false;
	}

	// Initialize column width.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSDDiversityFactor, DFColumnDescription::CD_HeaderCol, 2 );
		SetColWidth( pclSDDiversityFactor, DFColumnDescription::CD_FooterCol, 2 );
	}
	else
	{
		pclSheet->ShowCol( DFColumnDescription::CD_HeaderCol, FALSE );
		pclSheet->ShowCol( DFColumnDescription::CD_HeaderCol, FALSE );
	}

	m_fCursorChanged = false;
	m_hHandOpened = AfxGetApp()->LoadIcon( IDI_HANDCLICK );

	pclSheet->SetMaxRows( RD_Text_ColumnHeader );
	pclSheet->SetRowHeight( RD_Empty, m_dRowHeight * 0.5 );
	pclSheet->SetRowHeight( RD_Title, m_dRowHeight * 2 );
	pclSheet->SetRowHeight( RD_Empty2, m_dRowHeight * 0.5 );
	pclSheet->SetRowHeight( RD_DFDetails, m_dRowHeight * 1.5 );
	pclSheet->SetRowHeight( RD_DFClick, m_dRowHeight * 1.1 );
	pclSheet->SetRowHeight( RD_Empty3, m_dRowHeight * 1.5 );
	pclSheet->SetRowHeight( RD_Graphs, m_dRowHeight * 1.5 );
	pclSheet->SetRowHeight( RD_Empty4, m_dRowHeight * 1.5 );
	pclSheet->SetRowHeight( RD_Text_ColumnHeader, m_dRowHeight * 2 );

	if( false == _CreateTitleZone( pclSDDiversityFactor ) )
		return FALSE;
	
	// For the last column we need to compute to have the perfect match with RIGHTVIEWWIDTH.
	long lTotalWidth = 0;
	for( long lLoop = DFColumnDescription::CD_HeaderCol; lLoop < DFColumnDescription::CD_Pointer; lLoop++ )
		lTotalWidth += m_mapSheetInfos[pclSDDiversityFactor].m_mapColWidth[lLoop];
	if( lTotalWidth < RIGHTVIEWWIDTH )
		m_mapSheetInfos[pclSDDiversityFactor].m_mapColWidth[DFColumnDescription::CD_Pointer] = RIGHTVIEWWIDTH - lTotalWidth;
	else
		m_mapSheetInfos[pclSDDiversityFactor].m_mapColWidth[DFColumnDescription::CD_Pointer] = 0;
	
	if( false == _CreateGraphZone( pclSDDiversityFactor ) )
		return FALSE;
	
	_FillDFValvesGraph();
	_FillDFPipesGraph();
	_PutGraphInSheet( pclSDDiversityFactor );

	_InitDFTextHeader( pclSDDiversityFactor );
	_FillDFValvesText( pclSDDiversityFactor );
	_FillDFPipesText( pclSDDiversityFactor );

	// Move sheet to correct position.
	SetSheetSize();
	Invalidate();
	UpdateWindow();

	return true;
}

bool CSelProdPageDiversityFactor::HasSomethingToDisplay( void )
{
	if( NULL == m_pclSelectionTableHM || 0 == m_pclSelectionTableHM->GetItemCount() )
		return false;

	// If no diversity factor active...
	if( false == m_pclSelectionTableHM->IsDiversityExistSomewhere() )
		return false;

	return true;
}

BEGIN_MESSAGE_MAP( CSelProdPageDiversityFactor, CSelProdPageBase )
	ON_MESSAGE( WM_USER_MESSAGEDATA, MM_OnMessageReceive )
END_MESSAGE_MAP()

LRESULT CSelProdPageDiversityFactor::MM_OnMessageReceive( WPARAM wParam, LPARAM lParam )
{
	CSheetDescription* pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_DF );
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() ) 
	{
		// If no sheet description has not yet been created, pass the call to the base class.
		return CSelProdPageBase::MM_OnMessageReceive( wParam, lParam );
	}

	// Before read the message, make a copy in case we want to pass to the base class.
	WPARAM wMsgCopy = MM_CopyMessage( wParam );
	
	CMessageManager::CMessage* pclMessage = MM_ReadMessage( wParam );
	if( NULL == pclMessage )
	{
		// If it's impossible to read message, it's not needed to call the base class.
		MM_DestroyMessage( wMsgCopy );
		return 0;
	}
	
	if( CMessageManager::ClassID::CID_CSSheet != (CMessageManager::ClassID)pclMessage->GetClassID() )
	{
		// Here we want to receive only message concerned by 'SSheet'. If it's not the case, we pass the call to the base class.
		return CSelProdPageBase::MM_OnMessageReceive( wMsgCopy, lParam );
	}

	switch( (CMessageManager::SSheetMessageType)pclMessage->GetMessageType() )
	{
		case CMessageManager::SSheetMessageType::SSheetMST_MouseEvent:
			{
				CMessageManager::CSSheetMEMsg* pclSShetMEMsg = dynamic_cast<CMessageManager::CSSheetMEMsg*>( pclMessage );
				if( NULL == pclSShetMEMsg )
				{
					// If it's impossible to read message, it's not needed to call the base class.
					delete pclMessage;
					MM_DestroyMessage( wMsgCopy );
					return 0;
				}

				SSheetMEParams* prParams = pclSShetMEMsg->GetParams();
				if( NULL == prParams->m_hSheet || prParams->m_hSheet != pclSheetDescription->GetSSheetPointer()->GetSafeHwnd() )
				{
					// If it's impossible to read message, it's not needed to call the base class.
					delete pclMessage;
					MM_DestroyMessage( wMsgCopy );
					return 0;
				}

				switch( prParams->m_eMouseEventType )
				{
					case CMessageManager::SSheetMET::MET_LButtonDown:
						_OnLButtonDown( prParams->m_lColumn, prParams->m_lRow );
						break;

					case CMessageManager::SSheetMET::MET_MouseMove:
						_OnMouseMove( prParams->m_lColumn, prParams->m_lRow );
						break;
				}
			}
			break;
	}

	// In all cases, let the base class to also catch this message.
	delete pclMessage;
	return CSelProdPageBase::MM_OnMessageReceive( wMsgCopy, lParam );
}

void CSelProdPageDiversityFactor::_OnMouseMove( long lMouseMoveCol, long lMouseMoveRow )
{
	CSheetDescription* pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_DF );
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() ) 
		return;

	if( false == m_fCursorChanged && lMouseMoveCol >= DFColumnDescription::CD_ValveNumberBefore && lMouseMoveCol <= DFColumnDescription::CD_ValveNumberAfter && RD_DFClick == lMouseMoveRow )
	{
		pclSheetDescription->GetSSheetPointer()->SetCursor( SS_CURSORTYPE_DEFAULT, m_hHandOpened );
		m_fCursorChanged = true;
	}
	else if( true == m_fCursorChanged && ( lMouseMoveCol < DFColumnDescription::CD_ValveNumberBefore || lMouseMoveCol > DFColumnDescription::CD_ValveNumberAfter || RD_DFClick != lMouseMoveRow ) )
	{
		pclSheetDescription->GetSSheetPointer()->SetCursor( SS_CURSORTYPE_DEFAULT, SS_CURSOR_ARROW );
		m_fCursorChanged = false;
	}
}

void CSelProdPageDiversityFactor::_OnLButtonDown( long lLButtonDownCol, long lLButtonDownRow )
{
	if( lLButtonDownCol >= DFColumnDescription::CD_ValveNumberBefore && lLButtonDownCol <= DFColumnDescription::CD_ValveNumberAfter && RD_DFClick == lLButtonDownRow )
	{
		pMainFrame->ShowDlgDiversityFactor();
	}
}

bool CSelProdPageDiversityFactor::_CreateTitleZone( CSheetDescription* pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
		return false;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();

	SetColWidth( pclSheetDescription, DFColumnDescription::CD_HeaderCol, 2 );
	SetColWidth( pclSheetDescription, DFColumnDescription::CD_ValveNumberBefore, 6 );
	SetColWidth( pclSheetDescription, DFColumnDescription::CD_ValveNameBefore, 30 );
	SetColWidth( pclSheetDescription, DFColumnDescription::CD_ValveSeparator, 2 );
	SetColWidth( pclSheetDescription, DFColumnDescription::CD_ValveNumberAfter, 6 );
	SetColWidth( pclSheetDescription, DFColumnDescription::CD_ValveNameAfter, 30 );
	SetColWidth( pclSheetDescription, DFColumnDescription::CD_Separator, 2 );
	SetColWidth( pclSheetDescription, DFColumnDescription::CD_PipeNumberBefore, 6 );
	SetColWidth( pclSheetDescription, DFColumnDescription::CD_PipeNameBefore, 30 );
	SetColWidth( pclSheetDescription, DFColumnDescription::CD_PipeSeparator, 2 );
	SetColWidth( pclSheetDescription, DFColumnDescription::CD_PipeNumberAfter, 6 );
	SetColWidth( pclSheetDescription, DFColumnDescription::CD_PipeNameAfter, 30 );
	SetColWidth( pclSheetDescription, DFColumnDescription::CD_FooterCol, 2 );
	SetColWidth( pclSheetDescription, DFColumnDescription::CD_Pointer, 0 );

	// Page title.
	CString str = TASApp.LoadLocalizedString( IDS_RVIEWSELP_TITLE_DIVERSITYFACTOR );
	pclSheet->AddCellSpan( DFColumnDescription::CD_ValveNumberBefore, RD_Title, DFColumnDescription::CD_FooterCol - DFColumnDescription::CD_ValveNumberBefore, 1 );
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	pclSheet->SetStaticText( DFColumnDescription::CD_ValveNumberBefore, RD_Title, str );

	// Diversity factor.
	CTableHM::vecHydroMod vecModuleList;
	double dDF;
	m_pclSelectionTableHM->GetDFModuleList( NULL, vecModuleList, dDF );
	if( (int)vecModuleList.size() > 0 )
	{
		str = TASApp.LoadLocalizedString( IDS_RVIEWSELP_DF_TITLECURRENTDF );

		// Add to a map to sort.
		std::map<CString, bool> mapModuleList;
		for( int iLoop = 0; iLoop < (int)vecModuleList.size(); iLoop++ )
			mapModuleList[CString(vecModuleList[iLoop]->GetHMName())] = false;
			
		int iMin = min( 3, vecModuleList.size() );

		str.Append( _T(" ") );
		int iLoop = 0;
		for( std::map<CString, bool>::iterator iter = mapModuleList.begin(); iLoop < iMin ; ++iter, ++iLoop )
		{
			if( iLoop < ( iMin - 1 ) )
				str.AppendFormat( _T("%s; "), iter->first );
			else
			{
				if( (int)vecModuleList.size() > iMin )
					str.AppendFormat( _T("%s;..."), iter->first );
				else
					str.AppendFormat( _T("%s."), iter->first );
			}
		}
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_RVIEWSELP_DF_TITLECURRENTNODF );
	}

	// Set on which module has been applied the diversity factor.
	pclSheet->AddCellSpan( DFColumnDescription::CD_ValveNumberBefore, RD_DFDetails, DFColumnDescription::CD_FooterCol - DFColumnDescription::CD_ValveNumberBefore, 1 );
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::SubTitle );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	pclSheet->SetStaticText( DFColumnDescription::CD_ValveNumberBefore, RD_DFDetails, str );

	// Set the link to open the diversity factor dialog.
	pclSheet->AddCellSpan( DFColumnDescription::CD_ValveNumberBefore, RD_DFClick, DFColumnDescription::CD_ValveNameAfter - DFColumnDescription::CD_ValveNumberBefore, 1 );
	str.Format( _T("(%s)"), TASApp.LoadLocalizedString( IDS_RVIEWSELP_DF_TITLEGODFDLG ) );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)9 );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontUnderline, (LPARAM)TRUE );
	pclSheet->SetStaticText( DFColumnDescription::CD_ValveNumberBefore, RD_DFClick, str );

	// Fill the rest of the line with the same color as the link.
	pclSheet->AddCellSpan( DFColumnDescription::CD_ValveNameAfter, RD_DFClick, DFColumnDescription::CD_FooterCol - DFColumnDescription::CD_ValveNameAfter, 1 );
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::SubTitle );
	pclSheet->SetStaticText( DFColumnDescription::CD_ValveNameAfter, RD_DFClick, _T("") );

	// Activate SSheet notification handlers for left button mouse and mouse move.
	MM_RegisterNotificationHandler( this,	CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonDown | 
											CMessageManager::SSheetNHFlags::SSheetNHF_MouseMove );
	return true;
}

bool CSelProdPageDiversityFactor::_CreateGraphZone( CSheetDescription* pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
		return false;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CRect rectTitleSheet = pclSheet->GetSheetSizeInPixels();

	// Destroy if already exist.
	if( m_TChartValves.GetSafeHwnd() != NULL )
	{
		m_TChartValves.DestroyWindow();
		m_TChartValves.ClearChart();
	}

	if( m_TChartPipes.GetSafeHwnd() != NULL )
	{
		m_TChartPipes.DestroyWindow();
		m_TChartPipes.ClearChart();
	}

	// Create TChart for diversity factor valve graph.
	CRect rect = CRect( 50, rectTitleSheet.bottom, 450, rectTitleSheet.bottom + 350 );

	// Remark: first parameter is set to NULL because TEEchart activeX doesn't support the property DISPID_CAPTION and DISPID_TEXT to set
	//         the window text. In debug mode we have a COleException message printed in the output and we don't want it any more.
	// Remark: don't set 'pclSheet' as the parent. Because when 'OnPaint' is called, this is first the 'COleContainer' that will be called
	//         (See the 'CWnd::OnPaint' method) and after the 'Default()' method that will give the hand to the 'SpreadSheetProc' method
	//         (in the 'SS_MAIN.C' file). I don't know why but TChart will validate all the client area and when the 'SS_Paint' method
	//         (in the 'SS_DRAW.C' file) will call the 'fpBeginPaint' method (Macro for '::BeginPaint'), there is no invalidate area to
	//         repaint. And thus 'pclSheet' is never drawn.
	if( FALSE == m_TChartValves.Create( NULL, ( ( pclSheet->GetStyle() | WS_CHILD ) & ~WS_VISIBLE ), rect, this, IDC_TCHARTDFVALVES ) )
	{
		return false;
	}

	// Create TChart for diversity factor pipe graph.
	CRect rect2( 500, rectTitleSheet.bottom, 900, rectTitleSheet.bottom + 350 );

	if( FALSE == m_TChartPipes.Create( NULL, ( ( pclSheet->GetStyle() | WS_CHILD ) & ~WS_VISIBLE ), rect2, this, IDC_TCHARTDFPIPES ) )
	{
		return false;
	}

	// Add two horizontal bar series.
	m_TChartValves.AddSeries( scHorizBar );
	m_TChartValves.AddSeries( scHorizBar );
	m_TChartPipes.AddSeries( scHorizBar );
	m_TChartPipes.AddSeries( scHorizBar );

	pclSheet->AddCellSpan( DFColumnDescription::CD_ValveNumberBefore, RD_Graphs, DFColumnDescription::CD_Separator - DFColumnDescription::CD_ValveNumberBefore, 1 );
	pclSheet->AddCellSpan( DFColumnDescription::CD_PipeNumberBefore, RD_Graphs, DFColumnDescription::CD_FooterCol - DFColumnDescription::CD_PipeNumberBefore, 1 );

	return true;
}

void CSelProdPageDiversityFactor::_FillDFValvesGraph( void )
{
	// To be sure.
	m_mapValvesGraph.clear();

	// Scan all the project to extract needed data.
	std::vector<CTable*> vecTreeList;
	int iCurrentDeep = 0;
	bool fCanStop = false;
	bool fIsDiversityExist = m_pclSelectionTableHM->IsDiversityExistSomewhere();
	CTable *pTable = (CTable*)m_pclSelectionTableHM;
	
	CDS_HydroMod* pHM = dynamic_cast<CDS_HydroMod*>( (CData *)( pTable->GetFirst().MP ) );
	do 
	{
		// If hydromod exist...
		if( NULL != pHM )
		{
			for( int iLoop = 0; iLoop < 5; iLoop++ )
			{
				CDB_TAProduct* pTAProduct = NULL;
				CDB_TAProduct* pTADFProduct = NULL;
				switch(iLoop )
				{
					case 0:
						if( NULL != pHM->GetpBv() )
						{
							pTAProduct = dynamic_cast<CDB_TAProduct*>( pHM->GetpBv()->GetIDPtr().MP );
							pTADFProduct = dynamic_cast<CDB_TAProduct*>( pHM->GetpBv()->GetDiversityIDPtr().MP );
						}
						break;

					case 1:
						if( NULL != pHM->GetpCV() )
						{
							pTAProduct = dynamic_cast<CDB_TAProduct*>( pHM->GetpCV()->GetCvIDPtr().MP );
							pTADFProduct = dynamic_cast<CDB_TAProduct*>( pHM->GetpCV()->GetDiversityIDPtr().MP );
						}
						break;

					case 2:
						if( NULL != pHM->GetpDpC() )
						{
							pTAProduct = dynamic_cast<CDB_TAProduct*>( pHM->GetpDpC()->GetIDPtr().MP );
							pTADFProduct = dynamic_cast<CDB_TAProduct*>( pHM->GetpDpC()->GetDiversityIDPtr().MP );
						}
						break;

					case 3:
						if( NULL != pHM->GetpBypBv() )
						{
							pTAProduct = dynamic_cast<CDB_TAProduct*>( pHM->GetpBypBv()->GetIDPtr().MP );
							pTADFProduct = dynamic_cast<CDB_TAProduct*>( pHM->GetpBypBv()->GetDiversityIDPtr().MP );
						}
						break;

					case 4:
						if( NULL != pHM->GetpSecBv() )
						{
							pTAProduct = dynamic_cast<CDB_TAProduct*>( pHM->GetpSecBv()->GetIDPtr().MP );
							pTADFProduct = dynamic_cast<CDB_TAProduct*>( pHM->GetpSecBv()->GetDiversityIDPtr().MP );
						}
						break;

				}
				
				int iSizeBefore = -1;
				int iSizeAfter = -1;
				CString strSizeBefore( _T("") );
				CString strSizeAfter( _T("") );
				if( true == fIsDiversityExist )
				{
					if( NULL != pTADFProduct )
					{
						iSizeBefore = pTADFProduct->GetSizeKey();
						strSizeBefore = pTADFProduct->GetSize();
					}

					if( NULL != pTAProduct )
					{
						iSizeAfter = pTAProduct->GetSizeKey();
						strSizeAfter = pTAProduct->GetSize();
					}
				}
				else
				{
					if( NULL != pTAProduct )
					{
						iSizeBefore = pTAProduct->GetSizeKey();
						iSizeAfter = pTAProduct->GetSizeKey();
						strSizeBefore = pTAProduct->GetSize();
						strSizeAfter = pTAProduct->GetSize();
					}
				}

				// Set before entry for graph.
				if( iSizeBefore != -1 )
				{
					if( 0 == m_mapValvesGraph.count( iSizeBefore ) )
						m_mapValvesGraph[iSizeBefore][false] = 1;
					else
					{
						if( 0 == m_mapValvesGraph[iSizeBefore].count( false ) )
							m_mapValvesGraph[iSizeBefore][false] = 1;
						else
							m_mapValvesGraph[iSizeBefore][false]++;
					}

					// Add relation link between size and string for optimization later.
					if( 0 == m_mapSizeString.count( iSizeBefore ) && false == strSizeBefore.IsEmpty() )
						m_mapSizeString[iSizeBefore] = strSizeBefore;
				}

				// Set after entry for graph.
				if( iSizeAfter != -1 )
				{
					if( 0 == m_mapValvesGraph.count( iSizeAfter ) )
						m_mapValvesGraph[iSizeAfter][true] = 1;
					else
					{
						if( 0 == m_mapValvesGraph[iSizeAfter].count( true ) )
							m_mapValvesGraph[iSizeAfter][true] = 1;
						else
							m_mapValvesGraph[iSizeAfter][true]++;
					}

					// Add relation link between size and string for optimization later.
					if( 0 == m_mapSizeString.count( iSizeAfter ) && false == strSizeAfter.IsEmpty() )
						m_mapSizeString[iSizeAfter] = strSizeAfter;
				}

				// Set before/after for text.
				if( iSizeBefore != -1 && iSizeAfter != -1 )
				{
					if( 0 == m_mapValvesText.count( iSizeBefore ) )
						m_mapValvesText[iSizeBefore][iSizeAfter] = 1;
					else
					{
						if( 0 == m_mapValvesText[iSizeBefore].count( iSizeAfter ) )
							m_mapValvesText[iSizeBefore][iSizeAfter] = 1;
						else
							m_mapValvesText[iSizeBefore][iSizeAfter]++;
					}
				}
			}
		}

		if( NULL == pHM )
		{
			if( vecTreeList.size() > 0 )
			{
				iCurrentDeep--;
				pTable = vecTreeList.back();
				pHM = dynamic_cast<CDS_HydroMod*>( (CData *)( pTable->GetNext().MP ) );
				vecTreeList.pop_back();
			}
			else
				fCanStop = true;
		}
		else if( NULL != pHM->GetFirst().MP )
		{
			iCurrentDeep++;
			vecTreeList.push_back( pTable );
			pTable = (CTable*)pHM;
			pHM = dynamic_cast<CDS_HydroMod*>( (CData*)( pTable->GetFirst().MP ) );
		}
		else
			pHM = dynamic_cast<CDS_HydroMod*>( (CData *)( pTable->GetNext().MP ) );
	}while( false == fCanStop );

	// Adjust TChart height in regards to number of points.
	CRect rect;
	m_TChartValves.GetClientRect( &rect );
	if( m_mapValvesGraph.size() <= 7 )
		rect.bottom = rect.top + 350;
	else
		rect.bottom = rect.top + m_mapValvesGraph.size() * 50;
	m_TChartValves.SetWindowPos( NULL, -1, -1, rect.Width(), rect.Height(), SWP_NOMOVE | SWP_NOZORDER );

	// Fill graph.
	int i = 1;
	for( mapValveGraphIter iterGraph = m_mapValvesGraph.begin(); iterGraph!= m_mapValvesGraph.end(); ++iterGraph, ++i )
	{
		// 1st param: number of valves of this name AFTER applying diversity factor.
		// 2nd param: index of the point on the graph.
		// 3rd param: name of the size of the valve.
		m_TChartValves.Series( 0 ).AddXY( iterGraph->second[true], i, m_mapSizeString[iterGraph->first], RGB( 0, 0, 255 ) );
		m_TChartValves.Series( 0 ).SetPointColor( i - 1, _IMI_GRAY_XXLIGHT );

		// 1st param: number of valves of this name BEFORE applying diversity factor.
		m_TChartValves.Series( 1 ).AddXY( iterGraph->second[false], i, m_mapSizeString[iterGraph->first], RGB( 0, 0, 255 ) );
		m_TChartValves.Series( 1 ).SetPointColor( i - 1, _TAH_ORANGE_XLIGHT );
	}

	m_TChartValves.Series( 0 ).GetMarks().SetVisible( FALSE );
	m_TChartValves.Series( 1 ).GetMarks().SetVisible( FALSE );

	m_TChartValves.Series( 0 ).GetAsHorizBar().SetMultiBar( mbSide );
	m_TChartValves.Series( 1 ).GetAsHorizBar().SetMultiBar( mbSide );

	m_TChartValves.Series( 0 ).GetAsHorizBar().SetAutoBarSize( TRUE );
	m_TChartValves.Series( 1 ).GetAsHorizBar().SetAutoBarSize( TRUE );

	m_TChartValves.Series( 0 ).GetAsHorizBar().SetBarStyle( bsRectGradient );
	CGradient gradient = m_TChartValves.Series( 0 ).GetAsHorizBar().GetGradient();
	gradient.SetStartColor( _IMI_GRAY_LIGHT );
	gradient.SetMidColor( _IMI_GRAY_XLIGHT );
	gradient.SetEndColor( _IMI_GRAY_XXLIGHT );
	gradient.SetDirection( gdFromTopLeft );
	gradient.SetVisible( TRUE );
 
	m_TChartValves.Series( 1 ).GetAsHorizBar().SetBarStyle( bsRectGradient );
	CGradient gradient2 = m_TChartValves.Series( 1 ).GetAsHorizBar().GetGradient();
	gradient2.SetStartColor( _TAH_ORANGE_MED );
	gradient2.SetMidColor( _TAH_ORANGE_LIGHT );
	gradient2.SetEndColor( _TAH_ORANGE_XLIGHT );
	gradient2.SetDirection( gdFromTopLeft );
	gradient2.SetVisible( TRUE );

	m_TChartValves.Series( 0 ).SetColor( _IMI_GRAY_LIGHT );
	m_TChartValves.Series( 1 ).SetColor( _TAH_ORANGE_MED );
	
	m_TChartValves.GetAxis().GetBottom().SetAutomaticMaximum( TRUE );
	m_TChartValves.GetAxis().GetBottom().SetIncrement( 1 );
	m_TChartValves.GetAxis().GetBottom().GetTitle().SetCaption( TASApp.LoadLocalizedString( IDS_RVIEWSELP_DF_VALVEXAXISCAPTION ) );
	
	CString str = TASApp.LoadLocalizedString( IDS_RVIEWSELP_DF_CHARTLEGENDWITHDF );
	m_TChartValves.Series( 0 ).SetTitle( str );
	str = TASApp.LoadLocalizedString( IDS_RVIEWSELP_DF_CHARTLEGENDWITHOUTDF );
	m_TChartValves.Series( 1 ).SetTitle( str );

	// Change top title.
	m_TChartValves.GetHeader().GetText().Clear();
	str = TASApp.LoadLocalizedString( IDS_RVIEWSELP_DF_CHARTVALVETITLE );
	m_TChartValves.GetHeader().GetText().SetText( str );
	m_TChartValves.GetHeader().GetFont().SetBold( TRUE );
	m_TChartValves.GetHeader().GetFont().SetColor( _IMI_GRAY_LIGHT );
	m_TChartValves.GetHeader().GetFont().SetHeight( 12 );
	m_TChartValves.GetHeader().GetFont().SetName( _T("Verdana") );
}

void CSelProdPageDiversityFactor::_FillDFPipesGraph( void )
{
	// To be sure.
	m_mapPipesGraph.clear();

	// Scan all the project to extract needed data.
	std::vector<CTable*> vecTreeList;
	int iCurrentDeep = 0;
	bool fCanStop = false;
	CTable *pTable = (CTable*)m_pclSelectionTableHM;
	
	CDS_HydroMod* pHM = dynamic_cast<CDS_HydroMod*>( (CData *)( pTable->GetFirst().MP ) );
	do 
	{
		// If hydromod exist...
		// Remark: why 'iCurrentDeep'? Because no distribution pipe in a root module!!
		if( NULL != pHM && iCurrentDeep > 0 )
		{
			bool fIsDiversityExist = pHM->IsDiversityExist();

			for( int iLoop = 0; iLoop < 2; iLoop++ )
			{
				CDB_Pipe* pCDBPipe = NULL;
				CDB_Pipe* pCDBDFPipe = NULL;
				CPipes* pPipe = NULL;
				
				if( 0 == iLoop )
				{
					if( NULL != pHM->GetpDistrSupplyPipe() )
					{
						pCDBPipe = dynamic_cast<CDB_Pipe*>( pHM->GetpDistrSupplyPipe()->GetIDPtr().MP );
						pCDBDFPipe = dynamic_cast<CDB_Pipe*>( pHM->GetpDistrSupplyPipe()->GetDiversityIDPtr().MP );
						pPipe = pHM->GetpDistrSupplyPipe();
					}
				}
				else
				{
					if( CDS_HydroMod::ReturnType::Reverse != pHM->GetReturnType() || NULL == pHM->GetpDistrReturnPipe() )
						continue;

					pCDBPipe = dynamic_cast<CDB_Pipe*>( pHM->GetpDistrReturnPipe()->GetIDPtr().MP );
					pCDBDFPipe = dynamic_cast<CDB_Pipe*>( pHM->GetpDistrReturnPipe()->GetDiversityIDPtr().MP );
					pPipe = pHM->GetpDistrReturnPipe();
				}

				int iSizeBefore = -1;
				int iSizeAfter = -1;
				CString strSizeBefore( _T("") );
				CString strSizeAfter( _T("") );
				if( true == fIsDiversityExist )
				{
					if( NULL != pCDBDFPipe )
					{
						iSizeBefore = pCDBDFPipe->GetSizeKey( TASApp.GetpTADB() );
						strSizeBefore = pCDBDFPipe->GetSize( TASApp.GetpTADB() );
					}
					if( NULL != pCDBPipe )
					{
						iSizeAfter = pCDBPipe->GetSizeKey( TASApp.GetpTADB() );
						strSizeAfter = pCDBPipe->GetSize( TASApp.GetpTADB() );
					}
				}
				else
				{
					if( NULL != pCDBPipe )
					{
						iSizeBefore = pCDBPipe->GetSizeKey( TASApp.GetpTADB() );
						iSizeAfter = pCDBPipe->GetSizeKey( TASApp.GetpTADB() );
						strSizeBefore = pCDBPipe->GetSize( TASApp.GetpTADB() );
						strSizeAfter = pCDBPipe->GetSize( TASApp.GetpTADB() );
					}
				}

				// Set before entry for graph.
				if( iSizeBefore != -1 )
				{
					if( 0 == m_mapPipesGraph.count( iSizeBefore ) )
						m_mapPipesGraph[iSizeBefore][false] = pPipe->GetLength();
					else
					{
						if( 0 == m_mapPipesGraph[iSizeBefore].count( false ) )
							m_mapPipesGraph[iSizeBefore][false] = pPipe->GetLength();
						else
							m_mapPipesGraph[iSizeBefore][false] += pPipe->GetLength();
					}

					// Add relation link between size and string for optimization later.
					if( 0 == m_mapSizeString.count( iSizeBefore ) && false == strSizeBefore.IsEmpty() )
						m_mapSizeString[iSizeBefore] = strSizeBefore;
				}

				// Set after entry for graph.
				if( iSizeAfter != -1 )
				{
					if( 0 == m_mapPipesGraph.count( iSizeAfter ) )
						m_mapPipesGraph[iSizeAfter][true] = pPipe->GetLength();
					else
					{
						if( 0 == m_mapPipesGraph[iSizeAfter].count( true ) )
							m_mapPipesGraph[iSizeAfter][true] = pPipe->GetLength();
						else
							m_mapPipesGraph[iSizeAfter][true] += pPipe->GetLength();
					}

					// Add relation link between size and string for optimization later.
					if( 0 == m_mapSizeString.count( iSizeAfter ) && false == strSizeAfter.IsEmpty() )
						m_mapSizeString[iSizeAfter] = strSizeAfter;
				}

				// Set before/after for text.
				if( iSizeBefore != -1 && iSizeAfter != -1 )
				{
					if( 0 == m_mapPipesText.count( iSizeBefore ) )
						m_mapPipesText[iSizeBefore][iSizeAfter] = pPipe->GetLength();
					else
					{
						if( 0 == m_mapPipesText[iSizeBefore].count( iSizeAfter ) )
							m_mapPipesText[iSizeBefore][iSizeAfter] = pPipe->GetLength();
						else
							m_mapPipesText[iSizeBefore][iSizeAfter] += pPipe->GetLength();
					}
				}
			}
		}

		if( NULL == pHM )
		{
			if( vecTreeList.size() > 0 )
			{
				iCurrentDeep--;
				pTable = vecTreeList.back();
				pHM = dynamic_cast<CDS_HydroMod*>( (CData *)( pTable->GetNext().MP ) );
				vecTreeList.pop_back();
			}
			else
				fCanStop = true;
		}
		else if( NULL != pHM->GetFirst().MP )
		{
			iCurrentDeep++;
			vecTreeList.push_back( pTable );
			pTable = (CTable*)pHM;
			pHM = dynamic_cast<CDS_HydroMod*>( (CData*)( pTable->GetFirst().MP ) );
		}
		else
			pHM = dynamic_cast<CDS_HydroMod*>( (CData *)( pTable->GetNext().MP ) );

	}while( false == fCanStop );

	// Adjust TChart height in regards to number of points.
	CRect rect;
	m_TChartPipes.GetClientRect( &rect );
	if( m_mapPipesGraph.size() <= 7 )
		rect.bottom = rect.top + 350;
	else
		rect.bottom = rect.top + m_mapPipesGraph.size() * 50;
	m_TChartPipes.SetWindowPos( NULL, -1, -1, rect.Width(), rect.Height(), SWP_NOMOVE | SWP_NOZORDER );

	// Fill graph.
	int i = 1;
	for( mapPipeGraphIter iterGraph = m_mapPipesGraph.begin(); iterGraph!= m_mapPipesGraph.end(); ++iterGraph, ++i )
	{
		// 1st param: length of pipes of this name AFTER applying diversity factor.
		// 2nd param: index of the point on the graph.
		// 3rd param: name of the pipe.
		m_TChartPipes.Series( 0 ).AddXY( iterGraph->second[true], i, m_mapSizeString[iterGraph->first], RGB( 0, 0, 255 ) );
		m_TChartPipes.Series( 0 ).SetPointColor( i - 1, _IMI_GRAY_XXLIGHT );
		
		// 1st param: length of pipes of this name BEFORE applying diversity factor.
		m_TChartPipes.Series( 1 ).AddXY( iterGraph->second[false], i, m_mapSizeString[iterGraph->first], RGB( 0, 0, 255 ) );
		m_TChartPipes.Series( 1 ).SetPointColor( i - 1, _TAH_ORANGE_XLIGHT );
	}

	m_TChartPipes.Series( 0 ).GetMarks().SetVisible( FALSE );
	m_TChartPipes.Series( 1 ).GetMarks().SetVisible( FALSE );

	m_TChartPipes.Series( 0 ).GetAsHorizBar().SetMultiBar( mbSide );
	m_TChartPipes.Series( 1 ).GetAsHorizBar().SetMultiBar( mbSide );

	m_TChartPipes.Series( 0 ).GetAsHorizBar().SetAutoBarSize( TRUE );
	m_TChartPipes.Series( 1 ).GetAsHorizBar().SetAutoBarSize( TRUE );

	m_TChartPipes.Series( 0 ).GetAsHorizBar().SetBarStyle( bsRectGradient );
	CGradient gradient = m_TChartPipes.Series( 0 ).GetAsHorizBar().GetGradient();
	gradient.SetStartColor( _IMI_GRAY_LIGHT );
	gradient.SetMidColor( _IMI_GRAY_XLIGHT );
	gradient.SetEndColor( _IMI_GRAY_XXLIGHT );
	gradient.SetDirection( gdFromTopLeft );
	gradient.SetVisible( TRUE );
 
	m_TChartPipes.Series( 1 ).GetAsHorizBar().SetBarStyle( bsRectGradient );
	CGradient gradient2 = m_TChartPipes.Series( 1 ).GetAsHorizBar().GetGradient();
	gradient2.SetStartColor( _TAH_ORANGE_MED );
	gradient2.SetMidColor( _TAH_ORANGE_LIGHT );
	gradient2.SetEndColor( _TAH_ORANGE_XLIGHT );
	gradient2.SetDirection( gdFromTopLeft );
	gradient2.SetVisible( TRUE );

	m_TChartPipes.Series( 0 ).SetColor( _IMI_GRAY_LIGHT );
	m_TChartPipes.Series( 1 ).SetColor( _TAH_ORANGE_MED );
	
	m_TChartPipes.GetAxis().GetBottom().SetAutomaticMaximum( TRUE );
	m_TChartPipes.GetAxis().GetBottom().SetIncrement( 1 );

	CString str;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	FormatString( str, IDS_RVIEWSELP_DF_PIPEXAXISCAPTION, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	m_TChartPipes.GetAxis().GetBottom().GetTitle().SetCaption( str );
	
	str = TASApp.LoadLocalizedString( IDS_RVIEWSELP_DF_CHARTLEGENDWITHDF );
	m_TChartPipes.Series( 0 ).SetTitle( str );
	str = TASApp.LoadLocalizedString( IDS_RVIEWSELP_DF_CHARTLEGENDWITHOUTDF );
	m_TChartPipes.Series( 1 ).SetTitle( str );

	// Change top title.
	m_TChartPipes.GetHeader().GetText().Clear();
	str = TASApp.LoadLocalizedString( IDS_RVIEWSELP_DF_CHARTPIPETITLE );
	m_TChartPipes.GetHeader().GetText().SetText( str );
	m_TChartPipes.GetHeader().GetFont().SetBold( TRUE );
	m_TChartPipes.GetHeader().GetFont().SetColor( _IMI_GRAY_LIGHT );
	m_TChartPipes.GetHeader().GetFont().SetHeight( 12 );
	m_TChartPipes.GetHeader().GetFont().SetName( _T("Verdana") );
}

void CSelProdPageDiversityFactor::_PutGraphInSheet( CSheetDescription* pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
		return;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CRect rectGraphValves;
	m_TChartValves.GetClientRect( &rectGraphValves );
	CRect rectGraphPipes;
	m_TChartPipes.GetClientRect( &rectGraphPipes );
	int iHeight = max( rectGraphValves.Height(), rectGraphPipes.Height() );

	// Set the height.
	pclSheet->SetRowHeightInPixels( RD_Graphs, iHeight );

	// Get width available.
	CRect rectSheetValves = pclSheet->GetSelectionCoordInPixels( CRect( DFColumnDescription::CD_ValveNumberBefore, RD_Graphs, DFColumnDescription::CD_ValveNameAfter, RD_Graphs ) );
	CRect rectSheetPipes = pclSheet->GetSelectionCoordInPixels( CRect( DFColumnDescription::CD_PipeNumberBefore, RD_Graphs, DFColumnDescription::CD_PipeNameAfter, RD_Graphs ) );

	// Set position and resize.
	m_TChartValves.SetWindowPos( NULL, rectSheetValves.left, rectSheetValves.top, rectSheetValves.Width(), iHeight, SWP_NOZORDER | SWP_NOACTIVATE );
	m_TChartPipes.SetWindowPos( NULL, rectSheetPipes.left, rectSheetPipes.top, rectSheetPipes.Width(), iHeight, SWP_NOZORDER | SWP_NOACTIVATE );

	// Retrieve a BMP copy in the clipboard of the valves graph.
	m_TChartValves.GetExport().CopyToClipboardBitmap();
	if( FALSE == IsClipboardFormatAvailable( CF_BITMAP ) )
		return;
	::OpenClipboard( NULL );
	HGLOBAL hBitmap = ::GetClipboardData( CF_BITMAP );
	if( hBitmap != NULL )
	{
		// Copy bitmap in members variables.
		if( m_clChartValvesBmp.GetSafeHandle() != NULL )
			m_clChartValvesBmp.DeleteObject();
		m_clChartValvesBmp.CopyImage( (HBITMAP)hBitmap );

		// Set the bitmap in spread.
		pclSheet->SetPictureCellWithHandle( (HBITMAP)m_clChartValvesBmp, DFColumnDescription::CD_ValveNumberBefore, RD_Graphs, false );
	}
	::CloseClipboard();

	// Retrieve a BMP copy in the clipboard of the valves graph.
	m_TChartPipes.GetExport().CopyToClipboardBitmap();
	::OpenClipboard( NULL );
	hBitmap = ::GetClipboardData( CF_BITMAP );
	if( hBitmap != NULL )
	{
		// Copy bitmap in members variables.
		if( m_clChartPipesBmp.GetSafeHandle() != NULL )
			m_clChartPipesBmp.DeleteObject();
		m_clChartPipesBmp.CopyImage( (HBITMAP)hBitmap );

		// Set the bitmap in spread.
		pclSheet->SetPictureCellWithHandle( (HBITMAP)m_clChartPipesBmp, DFColumnDescription::CD_PipeNumberBefore, RD_Graphs, false );
	}
	::CloseClipboard();
}

void CSelProdPageDiversityFactor::_InitDFTextHeader( CSheetDescription* pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
		return;
	
	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();

	// Add a black line before column title.
	pclSheet->SetCellBorder( DFColumnDescription::CD_ValveNumberBefore, RD_Empty4, DFColumnDescription::CD_ValveNameAfter, RD_Empty4, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( DFColumnDescription::CD_PipeNumberBefore, RD_Empty4, DFColumnDescription::CD_PipeNameAfter, RD_Empty4, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Set the background to _TAH_ORANGE.
	pclSheet->SetBackColor( DFColumnDescription::CD_ValveNumberBefore, RD_Text_ColumnHeader, DFColumnDescription::CD_ValveNameAfter, RD_Text_ColumnHeader, _TAH_ORANGE );
	pclSheet->SetBackColor( DFColumnDescription::CD_PipeNumberBefore, RD_Text_ColumnHeader, DFColumnDescription::CD_PipeNameAfter, RD_Text_ColumnHeader, _TAH_ORANGE );

	CString str;
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_ORANGE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	str = TASApp.LoadLocalizedString( IDS_RVIEWSELP_DF_TEXTHDRVALVEBEFORE );
	pclSheet->SetStaticText( DFColumnDescription::CD_ValveNameBefore, RD_Text_ColumnHeader, str );
	str = TASApp.LoadLocalizedString( IDS_RVIEWSELP_DF_TEXTHDRVALVEAFTER );
	pclSheet->SetStaticText( DFColumnDescription::CD_ValveNameAfter, RD_Text_ColumnHeader, str );
	str = TASApp.LoadLocalizedString( IDS_RVIEWSELP_DF_TEXTHDRPIPEBEFORE );
	pclSheet->SetStaticText( DFColumnDescription::CD_PipeNameBefore, RD_Text_ColumnHeader, str );
	str = TASApp.LoadLocalizedString( IDS_RVIEWSELP_DF_TEXTHDRPIPEAFTER );
	pclSheet->SetStaticText( DFColumnDescription::CD_PipeNameAfter, RD_Text_ColumnHeader, str );

	// Add a black line under column titles.
	pclSheet->SetCellBorder( DFColumnDescription::CD_ValveNumberBefore, RD_Text_ColumnHeader, DFColumnDescription::CD_ValveNameAfter, RD_Text_ColumnHeader, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( DFColumnDescription::CD_PipeNumberBefore, RD_Text_ColumnHeader, DFColumnDescription::CD_PipeNameAfter, RD_Text_ColumnHeader, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
}

void CSelProdPageDiversityFactor::_FillDFValvesText( CSheetDescription* pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
		return;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	long lRow = RD_Text_FirstData;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// Fill data.
	int i = 1;
	CString str;
	for( mapTextValveRevIter iterText = m_mapValvesText.rbegin(); iterText!= m_mapValvesText.rend(); ++iterText, ++i )
	{
		for( mapValveCounterIter iterCount = iterText->second.begin(); iterCount != iterText->second.end(); ++iterCount )
		{
			_SetLastRow( pclSheetDescription, lRow );

			// Number of valve changes.
			str.Format( _T("%d"), iterCount->second );
			pclSheet->SetStaticText( DFColumnDescription::CD_ValveNumberBefore, lRow, str );

			// Name of valves before.
			pclSheet->SetStaticText( DFColumnDescription::CD_ValveNameBefore, lRow, m_mapSizeString[iterText->first] );

			// Set icon.
			pclSheet->SetPictureCellWithID( IDI_ARROWYELLOWRIGHT, DFColumnDescription::CD_ValveSeparator, lRow, CSSheet::PictureCellType::Icon );

			// Number of valve changes.
			str.Format( _T("%d"), iterCount->second );
			pclSheet->SetStaticText( DFColumnDescription::CD_ValveNumberAfter, lRow, str );

			// Name of valves after.
			pclSheet->SetStaticText( DFColumnDescription::CD_ValveNameAfter, lRow, m_mapSizeString[iterCount->first] );

			// If sizes are not the same
			if( iterText->first != iterCount->first )
			{
				pclSheet->SetBackColor( DFColumnDescription::CD_ValveNumberBefore, lRow, DFColumnDescription::CD_ValveNameAfter, lRow, _IMI_GRAY_XLIGHT );
				pclSheet->SetFontBold( DFColumnDescription::CD_ValveNumberBefore, lRow, DFColumnDescription::CD_ValveNameAfter, lRow, TRUE );
			}

			lRow++;
		}
	}
}

void CSelProdPageDiversityFactor::_FillDFPipesText( CSheetDescription* pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
		return;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	long lRow = RD_Text_FirstData;

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// Fill text.
	int i = 1;
	CString str;
	for( mapTextPipeRevIter iterText = m_mapPipesText.rbegin(); iterText!= m_mapPipesText.rend(); ++iterText, ++i )
	{
		for( mapPipeCounterIter iterCount = iterText->second.begin(); iterCount != iterText->second.end(); ++iterCount )
		{
			_SetLastRow( pclSheetDescription, lRow );

			// Number of pipe changes.
			pclSheet->SetStaticText( DFColumnDescription::CD_PipeNumberBefore, lRow, WriteCUDouble( _U_LENGTH, iterCount->second, true ) );

			// Name of pipes before.
			pclSheet->SetStaticText( DFColumnDescription::CD_PipeNameBefore, lRow, m_mapSizeString[iterText->first] );

			// Set icon.
			pclSheet->SetPictureCellWithID( IDI_ARROWYELLOWRIGHT, DFColumnDescription::CD_PipeSeparator, lRow, CSSheet::PictureCellType::Icon );

			// Number of pipe changes.
			pclSheet->SetStaticText( DFColumnDescription::CD_PipeNumberAfter, lRow, WriteCUDouble( _U_LENGTH, iterCount->second, true ) );

			// Name of pipes after.
			pclSheet->SetStaticText( DFColumnDescription::CD_PipeNameAfter, lRow, m_mapSizeString[iterCount->first] );

			// If sizes are not the same.
			if( iterText->first != iterCount->first )
			{
				pclSheet->SetBackColor( DFColumnDescription::CD_PipeNumberBefore, lRow, DFColumnDescription::CD_PipeNameAfter, lRow, _IMI_GRAY_XLIGHT );
				pclSheet->SetFontBold( DFColumnDescription::CD_PipeNumberBefore, lRow, DFColumnDescription::CD_PipeNameAfter, lRow, TRUE );
			}

			lRow++;
		}
	}

	// Add a black line under the last data.
	lRow = pclSheet->GetMaxRows();
	pclSheet->SetCellBorder( DFColumnDescription::CD_ValveNumberBefore, lRow, DFColumnDescription::CD_ValveNameAfter, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSheet->SetCellBorder( DFColumnDescription::CD_PipeNumberBefore, lRow, DFColumnDescription::CD_PipeNameAfter, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
}

void CSelProdPageDiversityFactor::_SetLastRow( CSheetDescription* pclSheetDescription, long lLastRow )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || lLastRow <= 0 )
		return;

	// Extent number of rows if needed.
	long lMaxRows = pclSheetDescription->GetSSheetPointer()->GetMaxRows();
	if( lLastRow > lMaxRows ) 
		pclSheetDescription->GetSSheetPointer()->SetMaxRows( lLastRow );
}
