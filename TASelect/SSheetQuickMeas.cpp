#include "stdafx.h"
#include <algorithm>
#include "TASelect.h"
#include "Utilities.h"
#include "MainFrm.h"
#include "DlgQuickMeasNote.h"
#include "SSheetQuickMeas.h"
#include "Drawset.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSSheetQuickMeas::CSSheetQuickMeas()
{
	m_lColn = ColumnDescription::CD_Last - 1;
	m_lTitleRow	= 2;
	m_iNbrQM = 0;
	m_dRowHeight = 12.75;
	m_fSheetHasData = false;
	m_arlColWidth.SetSize( ColumnDescription::CD_Last );
	m_dPageWidth = 0.0;
	m_dRViewWidth = 0.0;
	m_fHideCircuitSubTitle = false;
	m_fHideDpSubTitle = false;
	m_fHideFlowSubTitle = false;
	m_fHideTempSubTitle = false;
	m_fHidePowerSubTitle = false;
}

CSSheetQuickMeas::~CSSheetQuickMeas()
{
	m_arlColumnExpandState.RemoveAll();
}

BOOL CSSheetQuickMeas::Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID )
{
	if( FALSE == CSSheet::Create( dwStyle, rect, pParentWnd, nID ) ) 
		return FALSE;
	
	CSSheet::Init();

	// Set up main parameters not yet set in base class CSheet.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
	SetBool( SSB_ALLOWDRAGDROP, FALSE );
	FormatStaticText( -1, -1, -1, -1, _T("") );
	
	SetGridType( SS_GRID_HORIZONTAL | SS_GRID_VERTICAL );
	
	return TRUE;
}

void CSSheetQuickMeas::ResizeColumns( double dWidth )
{
	SetBool( SSB_REDRAW, FALSE );
	
	m_dRViewWidth = dWidth;
	double dFactor = dWidth / m_dPageWidth;	
	dFactor = max( 1, dFactor );
	
	for( long lLoopColumn = ColumnDescription::CD_None + 1; lLoopColumn < ColumnDescription::CD_Last; lLoopColumn++ )
	{
		double dValue = m_arlColWidth[lLoopColumn];
		dValue *= dFactor;
		CSSheet::SetColWidth( lLoopColumn, dValue );
	}
	
	SetBool( SSB_REDRAW, TRUE );
	Invalidate();
}

int CSSheetQuickMeas::SendQMToRow( CDS_HydroModX *pHMX )
{
	long lRow = GetMaxRows();
	int iCount = 0;
	long lRowInit = lRow;
	while( NULL != pHMX )
	{
		if( m_vectorAlreadyPrinted.end() != std::find( m_vectorAlreadyPrinted.begin(), m_vectorAlreadyPrinted.end(), pHMX ) )
			break;
		
		// Verify the date with the limits.
		__time32_t time32 = pHMX->GetMeasDateTime();
		tm dt;

		if( 0 != _gmtime32_s( &dt, &time32 ) )
		{
			ZeroMemory( &dt, sizeof( dt ) );
			dt.tm_year = 70;
			dt.tm_mday = 1;
		}

		COleDateTime tim( dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec );

		// Do not show measurements outside the limits.
		if( tim < pMainFrame->GetLowerDateTime() || tim > pMainFrame->GetUpperDateTime() )
		{
			break;
		}

		// FillRow and format.
		SetMaxRows( ++lRow );
		lRow = FillRow( lRow, pHMX, ( iCount > 0 ) ? true : false );
		SetCellBorder(ColumnDescription::CD_Picture, lRow, ColumnDescription::CD_Note, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY);
		m_vectorAlreadyPrinted.push_back( pHMX );
		
		CDS_QMTable *pTab = dynamic_cast<CDS_QMTable*>( pHMX->GetIDPtr().PP );
		if( NULL != pTab )
		{
			pHMX = pTab->GetAssociatedHM( pHMX );
		}
		iCount++;
	}
	if (iCount > 1)
	{
		// Display link Picture when needed.
		// Span cell with previous row associated.
		int iNbRow = lRow - lRowInit;
		AddCellSpan(ColumnDescription::CD_Picture, lRowInit + 1, 1, iNbRow);
		SetPictureCellWithID(IDB_QMLINK, ColumnDescription::CD_Picture, lRowInit+1, CSSheet::PictureCellType::Bitmap);
	}
	return iCount;
}

// Fill SheetQuickMeas
// Return true if there is at least one quick measurement.
bool CSSheetQuickMeas::SetRedraw( bool fExport, bool fPrint, CRank* pRankList )
{
	m_fSheetHasData = false;
	SetBool( SSB_REDRAW, FALSE );

	// Init the sheet.
	Init();

	m_vectorAlreadyPrinted.clear();

	// Disable the drag and drop function.
	SetBool( SSB_ALLOWDRAGDROP, FALSE );
	
	// In case it is a print, do not print the first white column.
	if( true == fPrint )
		ShowCol( ColumnDescription::CD_None, FALSE );

	// Get the QUICKMEAS_TAB.
	CTable *pTab = TASApp.GetpTADS()->GetpQuickMeasureTable();

	if( NULL == pTab )
	{
		SetBool( SSB_REDRAW, TRUE );
		return m_fSheetHasData;
	}

	// Count the number of QM.
	m_iNbrQM = pTab->GetItemCount();

	if( 0 == m_iNbrQM )
	{
		SetBool( SSB_REDRAW, TRUE );
		return m_fSheetHasData;
	}

	m_fSheetHasData = true;
	BeginWaitCursor();

	// Determine the max nbr. of rows.
	long lRowMax =  RowDescription::RD_Last;	// Add header
	SetMaxRows( lRowMax );
	SetMaxCols( m_lColn );
	
	// Format the headers.
	InitHeader( fExport, fPrint, pRankList );

	//====================================
	// Fill and format the table.
	//====================================
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	long lRow = RowDescription::RD_Last;

	if( true == fPrint && NULL != pRankList && pRankList->GetCount() > 0 )
	{
		// Variables.
		LPARAM lparam = NULL;
		CString str;

		for( BOOL fContinue = pRankList->GetFirst( str, lparam ); TRUE == fContinue; fContinue = pRankList->GetNext( str, lparam ) )
		{
			CDS_HydroModX *pHMX = NULL;
			pHMX = (CDS_HydroModX *)lparam;

			if( pHMX != NULL)
			{
				SendQMToRow(pHMX);
			}
		}
	}
	else
	{
		for( IDPTR IDPtr = pTab->GetFirst(); NULL != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDS_HydroModX *pHMX = NULL;
			pHMX = (CDS_HydroModX *)( IDPtr.MP );
			
			SendQMToRow(pHMX);
		}
	}
	lRow = GetMaxRows();

	SetCellBorder( ColumnDescription::CD_Picture, lRow , ColumnDescription::CD_Power, lRow , true, SS_BORDERTYPE_BOTTOM );

	// Set Water Char information in the Measured values.
	// Sub Title if all Quick Meas have the same parameters.
	SetWCinSubTitle();

	// Set Min, Max flow deviation in the SubTitle.
	SetMinMaxFlowDevInSubTitle();

	// Hide empty columns.
	HideEmptyColumns();
	RedefineColWidth();

	// Define the page width.
	m_dPageWidth = GetSheetSizeInPixels(false).Width();

	if( true == fPrint || true == fExport )
	{
		for( long lLoopColumn = ColumnDescription::CD_DateTime; lLoopColumn < ColumnDescription::CD_Last; lLoopColumn++ )
		{
			if( TRUE == IsColHidden( lLoopColumn ) )
				ShowCol( lLoopColumn, TRUE );
		}
		ShowCol(ColumnDescription::CD_Note, FALSE);
		SetWCinSubTitle();
		HideEmptyColumns();
		RedefineColWidth();
	}
	ResizeAndColorSubTitles();

	// Check now if we can hide some Expand/Collapse column button or not.
	VerifyExpandCollapseColumnButtons( CRect( ColumnDescription::CD_DateTime, RowDescription::RD_Last, ColumnDescription::CD_Power, lRow ), RowDescription::RD_InfoSubTitle );
	
	if( false == fPrint )
	{
		// Check previous state and change Expand/Collapse group to match it.
		for( int i = 0; i < m_arlColumnExpandState.GetCount(); i++ )
		{
			long lColumn = m_arlColumnExpandState.GetAt( i );
			ChangeExpandCollapseColumnButtonState( lColumn, RowDescription::RD_InfoSubTitle, RowDescription::RD_Last, lRow, CSSheet::ExpandCollapseColumnAction::ECCA_ToExpand, false );
		}
	
		// Run all columns and check state of all Expand/Collapse column buttons to eventually change state of main Expand/Collapse column button.
		// See CSSheet.h for more details about parameters.
		VerifyMainExpandCollapseButton( ColumnDescription::CD_None, RowDescription::RD_InfoSubTitle, RowDescription::RD_InfoSubTitle, ColumnDescription::CD_DateTime, ColumnDescription::CD_Last );
	}
	
	EndWaitCursor();

	SetBool( SSB_REDRAW, TRUE );

	return m_fSheetHasData;
}

void CSSheetQuickMeas::CellClicked( long lColumn, long lRow )
{
	// Add the column Note 
	if ((lRow != RowDescription::RD_InfoSubTitle)
		&& (lColumn != ColumnDescription::CD_Note))
	{
		return;
	}

	BeginWaitCursor();
	SetBool( SSB_REDRAW, FALSE );

	bool fRedraw = false;

	// Add a Note
	if (lColumn == ColumnDescription::CD_Note)
	{
		AddNotes(lColumn, lRow);
	}

	// If click is on a main Expand/Collapse button...
	if( true == IsMainExpandCollapseButton( lColumn, lRow ) )
	{
		fRedraw = true;

		// Change state of all child Expand/Collapse column buttons.
		int iCurrentState = GetMainExpandCollapseColumnState( lColumn, lRow );
		switch( iCurrentState )
		{
			case CSSheet::ExpandCollapseColumnState::ECCS_Collapsed:
				ExpandAll();
				break;
			case CSSheet::ExpandCollapseColumnState::ECCS_Expanded:
				CollapseAll();
				break;
			case CSSheet::ExpandCollapseColumnState::ECCS_Invalid:
				fRedraw = false;
				break;
		}

		// Change state of main Expand/Collapse column button.
		if( true == fRedraw )
			ChangeMainExpandCollapseButtonState( lColumn, lRow, CSSheet::ExpandCollapseColumnAction::ECCA_Toggle );
	}
	else if( true == IsExpandCollapseColumnButton( lColumn, lRow ) )
	{
		// Change state of button.
		ChangeExpandCollapseColumnButtonState( lColumn, lRow, RowDescription::RD_Last, GetMaxRows(), ExpandCollapseColumnAction::ECCA_Toggle );

		// If collapse or extend has been executed on a group ...
		HideEmptyColumns();

		// Run all columns and check state of all Expand/Collapse column buttons to eventually change state of main Expand/Collapse column button.
		// See CSSheet.h for more details about parameters.
		VerifyMainExpandCollapseButton( ColumnDescription::CD_None, RowDescription::RD_InfoSubTitle, RowDescription::RD_InfoSubTitle, ColumnDescription::CD_DateTime, ColumnDescription::CD_Last );

		m_arlColumnExpandState.RemoveAll();
		for( long lLoopCol = ColumnDescription::CD_DateTime; lLoopCol < ColumnDescription::CD_Last; lLoopCol++ )
		{
			if( CSSheet::ExpandCollapseColumnState::ECCS_Expanded == GetExpandCollapseColumnState( lLoopCol, RowDescription::RD_InfoSubTitle ) )
				m_arlColumnExpandState.Add( lLoopCol );
		}

		// The sheet must be redrawn.
		fRedraw = true;
	}

	if( true == fRedraw )
	{
		RedefineColWidth();

		// Redefine the page width.
		m_dPageWidth = GetSheetSizeInPixels( false ).Width();

		// Resize the col.
		ResizeColumns( m_dRViewWidth );

		// Invalidate the sheet.
		Invalidate();
	}
	
	SetBool( SSB_REDRAW, TRUE );
	EndWaitCursor();
}

void CSSheetQuickMeas::InitHeader( bool bExport, bool bPrint, CRank* pRankList )
{
	// Variables.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	ASSERT( NULL != pUnitDB );

	CDC *pDC = GetDC();
	double dDeviceCaps = pDC->GetDeviceCaps( LOGPIXELSY );

	// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
	ReleaseDC( pDC );

	m_dRowHeight = 12.75 * dDeviceCaps / 96.0;		  //96 Dpi as reference.
	TCHAR unitname[_MAXCHARS];
	CString str;	

	// Format the rows's height.
	SetRowHeight( RowDescription::RD_None, m_dRowHeight * 0.5 );
	SetRowHeight( RowDescription::RD_Title, m_dRowHeight * 2 );
	SetRowHeight( RowDescription::RD_NbrQM, m_dRowHeight * 1.5 );
	SetRowHeight( RowDescription::RD_SubTitle, m_dRowHeight * 1.7 );
	SetRowHeight( RowDescription::RD_InfoSubTitle,m_dRowHeight );
	SetRowHeight( RowDescription::RD_Info, m_dRowHeight * 2 );
	SetRowHeight( RowDescription::RD_InfoUnits, m_dRowHeight * 1.2 );

	// Format column headers.
	SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	
	// Set the text in the title.
	str = TASApp.LoadLocalizedString( IDS_QUICKMEASUREMENTS );
	AddCellSpanW( ColumnDescription::CD_None + 1, RowDescription::RD_Title, ColumnDescription::CD_Last - ColumnDescription::CD_None - 1, 1 );
	SetStaticText( ColumnDescription::CD_None + 1, RowDescription::RD_Title, str );

	// Format the number of quick measurements.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	if( true == bPrint && 0 != pRankList->GetCount() )
		str.Format( _T("%d"), pRankList->GetCount() );
	else
		str.Format( _T("%d"), m_iNbrQM );
	str += _T(" ");
	str += TASApp.LoadLocalizedString( IDS_QUICKMEASUREMENTS );
	AddCellSpanW( ColumnDescription::CD_None + 1, RowDescription::RD_NbrQM, ColumnDescription::CD_Last - ColumnDescription::CD_None - 1, 1 );
	SetStaticText( ColumnDescription::CD_None + 1, RowDescription::RD_NbrQM, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Format the subtitles.
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	// Set the text in the subtitle.
	str = TASApp.LoadLocalizedString( IDS_SHEETQM_INFO );
	AddCellSpanW( ColumnDescription::CD_None + 1, RowDescription::RD_SubTitle, ColumnDescription::CD_ReferenceSep - ColumnDescription::CD_None - 1, 2 );
	SetStaticText( ColumnDescription::CD_None + 1, RowDescription::RD_SubTitle, str );

	str = TASApp.LoadLocalizedString( IDS_SHEETQM_CIRCUIT );
	AddCellSpanW( ColumnDescription::CD_WaterChar, RowDescription::RD_SubTitle, ColumnDescription::CD_KvSignalSep - ColumnDescription::CD_WaterChar, 2 );
	SetStaticText( ColumnDescription::CD_WaterChar, RowDescription::RD_SubTitle, str );
	
	str = TASApp.LoadLocalizedString( IDS_SHEETQM_DIFFPRESSURE );
	AddCellSpanW( ColumnDescription::CD_Dp, RowDescription::RD_SubTitle, ColumnDescription::CD_DpSep - ColumnDescription::CD_Dp, 2 );
	SetStaticText( ColumnDescription::CD_Dp, RowDescription::RD_SubTitle, str );
	
	str = TASApp.LoadLocalizedString( IDS_FLOW );
	AddCellSpanW( ColumnDescription::CD_DpFlow, RowDescription::RD_SubTitle, ColumnDescription::CD_FlowDeviationSep - ColumnDescription::CD_DpFlow, 2 );
	SetStaticText( ColumnDescription::CD_DpFlow, RowDescription::RD_SubTitle, str );
	
	str = TASApp.LoadLocalizedString( IDS_TEMPERATURE );
	AddCellSpanW( ColumnDescription::CD_TempHH, RowDescription::RD_SubTitle, ColumnDescription::CD_DiffTempSep - ColumnDescription::CD_TempHH, 2 );
	SetStaticText( ColumnDescription::CD_TempHH, RowDescription::RD_SubTitle, str );
	
	str = TASApp.LoadLocalizedString( IDS_POWER );
	AddCellSpanW( ColumnDescription::CD_Power, RowDescription::RD_SubTitle, ColumnDescription::CD_Last-ColumnDescription::CD_Power, 2 );
	SetStaticText( ColumnDescription::CD_Power, RowDescription::RD_SubTitle, str );

	// Format the column headers.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	
	// Set the text of column headers.
	str = TASApp.LoadLocalizedString( IDS_SHEETQM_DATETIME );
	SetStaticText( ColumnDescription::CD_DateTime, RowDescription::RD_Info, str );

	str = TASApp.LoadLocalizedString( IDS_SHEETQM_REFERENCE );
	SetStaticText( ColumnDescription::CD_Reference, RowDescription::RD_Info, str );

	str = TASApp.LoadLocalizedString( IDS_SHEETQM_WATERCHAR );
	SetStaticText( ColumnDescription::CD_WaterChar, RowDescription::RD_Info, str );

	str = TASApp.LoadLocalizedString( IDS_SHEETQM_VALVE );
	SetStaticText( ColumnDescription::CD_Valve, RowDescription::RD_Info, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	str = TASApp.LoadLocalizedString( IDS_SHEETQM_VALVETYPE );
	SetStaticText( ColumnDescription::CD_Valve, RowDescription::RD_InfoUnits, str );

	str = TASApp.LoadLocalizedString( IDS_SHEETQM_SETTING );
	SetStaticText( ColumnDescription::CD_Setting, RowDescription::RD_Info, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	str = TASApp.LoadLocalizedString(IDS_SHEETHDR_TURNSPOS );
	SetStaticText( ColumnDescription::CD_Setting, RowDescription::RD_InfoUnits, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		str = TASApp.LoadLocalizedString( IDS_KV );
	else
		str = TASApp.LoadLocalizedString( IDS_CV );
	SetStaticText( ColumnDescription::CD_Kv, RowDescription::RD_Info, str );

	if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		str = TASApp.LoadLocalizedString( IDS_KVSIGNAL );
	else
		str = TASApp.LoadLocalizedString( IDS_CVSIGNAL );
	SetStaticText( ColumnDescription::CD_KvSignal, RowDescription::RD_Info, str );

	str = TASApp.LoadLocalizedString( IDS_SHEETQM_MEASDP );
	SetStaticText( ColumnDescription::CD_Dp, RowDescription::RD_Info, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	str = (CString)unitname;
	SetStaticText( ColumnDescription::CD_Dp, RowDescription::RD_InfoUnits, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	str = TASApp.LoadLocalizedString( IDS_SHEETQM_MEASDP );
	SetStaticText( ColumnDescription::CD_DpFlow, RowDescription::RD_Info, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	str = (CString)unitname;
	SetStaticText( ColumnDescription::CD_DpFlow, RowDescription::RD_InfoUnits, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	str = TASApp.LoadLocalizedString( IDS_SHEETQM_MEASFLOW );
	SetStaticText( ColumnDescription::CD_Flow, RowDescription::RD_Info, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), unitname );
	str = (CString)unitname;
	SetStaticText( ColumnDescription::CD_Flow, RowDescription::RD_InfoUnits, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	str = TASApp.LoadLocalizedString( IDS_SHEETQM_DESIGNFLOW );
	SetStaticText( ColumnDescription::CD_DesignFlow, RowDescription::RD_Info, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), unitname );
	str = (CString)unitname;
	SetStaticText( ColumnDescription::CD_DesignFlow, RowDescription::RD_InfoUnits, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	
	str = TASApp.LoadLocalizedString( IDS_SHEETQM_FLOWDEV );
	SetStaticText( ColumnDescription::CD_FlowDeviation, RowDescription::RD_Info, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	str = _T("%");
	SetStaticText( ColumnDescription::CD_FlowDeviation, RowDescription::RD_InfoUnits, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	str = TASApp.LoadLocalizedString( IDS_SHEETQM_MEASTEMP );
	SetStaticText( ColumnDescription::CD_TempHH, RowDescription::RD_Info, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), unitname );
	str = (CString)unitname;
	SetStaticText( ColumnDescription::CD_TempHH, RowDescription::RD_InfoUnits, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	
	str = TASApp.LoadLocalizedString( IDS_SHEETQM_T1DT );
	SetStaticText( ColumnDescription::CD_Temp1DPS, RowDescription::RD_Info, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), unitname );
	str = (CString)unitname;
	SetStaticText( ColumnDescription::CD_Temp1DPS, RowDescription::RD_InfoUnits, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	str = TASApp.LoadLocalizedString( IDS_SHEETQM_T2DT );
	SetStaticText( ColumnDescription::CD_Temp2DPS, RowDescription::RD_Info, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), unitname );
	str = (CString)unitname;
	SetStaticText( ColumnDescription::CD_Temp2DPS, RowDescription::RD_InfoUnits, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	str = TASApp.LoadLocalizedString( IDS_SHEETQM_MEASDT );
	SetStaticText( ColumnDescription::CD_DiffTemp, RowDescription::RD_Info, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFTEMP ), unitname );
	str = (CString)unitname;
	SetStaticText( ColumnDescription::CD_DiffTemp, RowDescription::RD_InfoUnits, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	str = TASApp.LoadLocalizedString( IDS_SHEETQM_MEASPOWER );
	SetStaticText( ColumnDescription::CD_Power, RowDescription::RD_Info, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ), unitname );
	str = (CString)unitname;
	SetStaticText( ColumnDescription::CD_Power, RowDescription::RD_InfoUnits, str );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE);
	
	// Draw border.
	SetCellBorder( ColumnDescription::CD_Picture, RowDescription::RD_InfoUnits, ColumnDescription::CD_Reference, RowDescription::RD_InfoUnits, true, SS_BORDERTYPE_BOTTOM );
	SetCellBorder( ColumnDescription::CD_WaterChar, RowDescription::RD_InfoUnits, ColumnDescription::CD_KvSignal, RowDescription::RD_InfoUnits, true, SS_BORDERTYPE_BOTTOM );
	SetCellBorder( ColumnDescription::CD_Dp, RowDescription::RD_InfoUnits, ColumnDescription::CD_Dp, RowDescription::RD_InfoUnits, true, SS_BORDERTYPE_BOTTOM );
	SetCellBorder( ColumnDescription::CD_DpFlow, RowDescription::RD_InfoUnits, ColumnDescription::CD_FlowDeviation, RowDescription::RD_InfoUnits, true, SS_BORDERTYPE_BOTTOM );
	SetCellBorder( ColumnDescription::CD_TempHH, RowDescription::RD_InfoUnits, ColumnDescription::CD_DiffTemp, RowDescription::RD_InfoUnits, true, SS_BORDERTYPE_BOTTOM );
	SetCellBorder( ColumnDescription::CD_Power, RowDescription::RD_InfoUnits, ColumnDescription::CD_Power, RowDescription::RD_InfoUnits, true, SS_BORDERTYPE_BOTTOM );

	if( false == bPrint && false == bExport )
	{
		// Create a Expand/Collapse column button. Button is placed to 'ColumnDescription::CD_KvSignalSep' column and 'RowDescription::RD_InfoSubTitle' row.
		// And column range managed by button is from 'ColumnDescription::CD_WaterChar' to 'ColumnDescription::CD_KvSignal'.
		CreateExpandCollapseColumnButton( ColumnDescription::CD_KvSignalSep, RowDescription::RD_InfoSubTitle, ColumnDescription::CD_WaterChar, ColumnDescription::CD_KvSignal, GetMaxRows() );
		
		// Set that column range from 'ColumnDescription::CD_WaterChar' to 'ColumnDescription::CD_Setting' CAN'T BE collapsed.
		SetFlagCanBeCollapsed( ColumnDescription::CD_WaterChar, ColumnDescription::CD_Setting, false );
		
		// Set that column 'ColumnDescription::CD_KvSignal' CAN'T BE collapsed.
		// Finally, just 'ColumnDescription::CD_Kv' can be collapsed in this group.
		SetFlagCanBeCollapsed( ColumnDescription::CD_KvSignal, ColumnDescription::CD_KvSignal, false );
		
		// Collapse this group.
		ChangeExpandCollapseColumnButtonState( ColumnDescription::CD_KvSignalSep, RowDescription::RD_InfoSubTitle, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse, false );

		CreateMainExpandCollapseButton( ColumnDescription::CD_None, RowDescription::RD_InfoSubTitle );
	}
	
	// Freeze row header.
	SetFreeze( 0, RowDescription::RD_InfoUnits );
}

long CSSheetQuickMeas::FillRow( long lRow, CDS_HydroModX *pHMX, bool fAttachedMeasurement )
{
	long lReturn = lRow;
	if( NULL == pHMX )
		return lReturn;

	// Variables.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();			ASSERT( NULL != pUnitDB );
	unsigned int uiQMType = pHMX->GetQMType();
	CString str, str1, str2;
	double dDesignFlow;

	// Get the current valve.
	CDS_HydroModX::CBase *pValve = pHMX->GetpBV( CDS_HydroModX::InPrimary );
	if( NULL == pValve )
		pValve = pHMX->GetpCv();

	// Write DateTime.
	__time32_t t = pHMX->GetMeasDateTime();
	CTimeUnic dtu( (__time64_t)t );
	str = dtu.FormatGmt( IDS_DATE_FORMAT ) + CString( _T(" ") ) + dtu.FormatGmt( IDS_TIME_FORMAT );
	// Align the DateTime text to the left to be the same with the Note row
	SetTextPatternProperty(CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT);
	SetStaticText( ColumnDescription::CD_DateTime, lRow, str );

	// Add Note column and insert the + image 
	SetStaticText(ColumnDescription::CD_Note, lRow, _T(""));
	SetPictureCellWithID(IDI_SPLUS, ColumnDescription::CD_Note, lRow, CSSheet::PictureCellType::Icon);
	SetCellProperty(ColumnDescription::CD_Note, lRow, CSSheet::_SSCellProperty::CellCantLeftClick, false);
	SetTextTip(SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK);
	SetTextPatternProperty(CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER);

	// Write the reference.
	SetStaticText( ColumnDescription::CD_Reference, lRow, pHMX->GetHMName()->c_str() );

	// Write information depending on the QMtype.
	if( true == pHMX->IsQMType( CDS_HydroModX::eQMtype::QMPower ) )
	{
		// Write power.
		SetStaticText( ColumnDescription::CD_Power, lRow, pHMX->GetQMstrPower().c_str() );
	}

	if( true == pHMX->IsQMType( CDS_HydroModX::eQMtype::QMPower ) || true == pHMX->IsQMType( CDS_HydroModX::eQMtype::QMFlow ) )
	{
		CDB_TAProduct *pTAP = NULL;
		CDB_AlienRegValve *pAlienValve = NULL;

		if( NULL != pValve )
		{
			if( CDS_HydroModX::TAmode == pValve->GetMode() )
				pTAP = dynamic_cast<CDB_TAProduct *>( pValve->GetValveIDPtr().MP );
			else if( CDS_HydroModX::ALIENmode == pValve->GetMode() )
				pAlienValve = dynamic_cast<CDB_AlienRegValve *>( pValve->GetValveIDPtr().MP );
		}

		// Write water characteristic.
		CDS_WaterCharacteristic* pWC = dynamic_cast<CDS_WaterCharacteristic*>( pHMX->GetWaterCharIDPtr().MP );
		/* ASSERT( NULL != pWC ); */

		if( NULL != pWC )
		{
			pWC->BuildWaterStrings( str1, str2 );
			str1 += _T(" ");
			str1 += str2;
			if( CDS_HydroModX::eDTS::InternalReference == pHMX->GetWaterTempRef() )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			}
			SetStaticText( ColumnDescription::CD_WaterChar, lRow, str1 );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		}

		// Write valve.
		if( NULL != pValve )
		{
			if( CDS_HydroModX::eMode::TAmode == pValve->GetMode() || NULL != pAlienValve )
				SetStaticText( ColumnDescription::CD_Valve, lRow, pValve->GetValveName() );
			else if( CDS_HydroModX::eMode::Kvmode == pValve->GetMode() || (CDS_HydroModX::eMode::ALIENmode == pValve->GetMode() &&  NULL == pAlienValve) )
			{
				CDS_HydroModX::CBV *pBv = dynamic_cast<CDS_HydroModX::CBV *>( pValve );
				if( NULL != pBv )
				{
					if( pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
						str = TASApp.LoadLocalizedString( IDS_CV );
					else
						str = TASApp.LoadLocalizedString( IDS_KV );
					str += L" = ";
					str += WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, pBv->GetKv() ), 3, 0 );

					SetStaticText( ColumnDescription::CD_Valve, lRow, str );
				}
			}
		}

		// Write setting.
		if( CDS_HydroModX::eMode::TAmode == pValve->GetMode() || CDS_HydroModX::eMode::ALIENmode == pValve->GetMode() )
		{
			if( NULL != pTAP )
			{
				SetStaticText( ColumnDescription::CD_Setting, lRow, pHMX->GetQMstrOpening().c_str() );
			}
			else if (NULL != pAlienValve )
			{
				SetStaticText( ColumnDescription::CD_Setting, lRow, pHMX->GetQMstrOpening().c_str() );
			}
		}

		// Write Kv.
		if( CDS_HydroModX::eMode::TAmode == pValve->GetMode() )
		{
			if( NULL != pTAP )
			{
				CDB_ValveCharacteristic *pChar = pTAP->GetValveCharacteristic();
				if( NULL != pChar )
				{
					double dKv = pChar->GetKv( pValve->GetCurOpening() );
					SetStaticText( ColumnDescription::CD_Kv, lRow, ( -1.0 != dKv ) ? WriteCUDouble( _C_KVCVCOEFF, dKv ) : TASApp.LoadLocalizedString( IDS_ERROR ) );
				}
			}
		}
		else if( CDS_HydroModX::eMode::Kvmode == pValve->GetMode() || CDS_HydroModX::eMode::ALIENmode == pValve->GetMode() )
		{
			CDS_HydroModX::CBV *pBv = dynamic_cast<CDS_HydroModX::CBV *>( pValve );
			if( NULL != pBv )
			{
				SetStaticText( ColumnDescription::CD_Kv, lRow, WriteCUDouble( _C_KVCVCOEFF, pBv->GetKv() ) );
			}
		}

		// Do not hide the Kv column if a valve is define in Kv mode.
		if( CDS_HydroModX::Kvmode == pValve->GetMode() || CDS_HydroModX::eMode::ALIENmode == pValve->GetMode() )
		{
			SetFlagCanBeCollapsed( ColumnDescription::CD_Kv, ColumnDescription::CD_Kv, false );
		}

		// Write Kv Signal.
		if( NULL != pTAP )
		{
			if( true == pTAP->IsKvSignalEquipped() && -1.0 != pTAP->GetKvSignal() )
			{
				SetStaticText( ColumnDescription::CD_KvSignal, lRow, WriteCUDouble( _C_KVCVCOEFF, pTAP->GetKvSignal() ) );
			}
		}
		else if ( NULL != pAlienValve )
		{
			if( true == pAlienValve->IsKvSignalEquipped() && -1.0 != pAlienValve->GetKvSignal() )
			{
				SetStaticText( ColumnDescription::CD_KvSignal, lRow, WriteCUDouble( _C_KVCVCOEFF, pAlienValve->GetKvSignal() ) );
			}
		}
		
		// Write the Dp.
		SetStaticText( ColumnDescription::CD_DpFlow, lRow, pHMX->GetQMstrDp().c_str() );

		// Write the flow.
		if( pHMX->GetMeasFlow() != 0.0)
			SetStaticText( ColumnDescription::CD_Flow, lRow, pHMX->GetQMstrQ().c_str() );

		// Write the design flow.
		if ( pHMX->GetDesignFlow() != 0.0)
			SetStaticText( ColumnDescription::CD_DesignFlow, lRow, WriteCUDouble( _U_FLOW, pHMX->GetDesignFlow() ) );

		// Write the flow deviation.
		if( pHMX->GetMeasFlow() != 0.0 && pHMX->GetDesignFlow() != 0.0 )
		{
			dDesignFlow = pHMX->GetDesignFlow();
			if( NULL != pValve )
			{
				str = WriteDouble( pValve->GetFlowRatio() - 100, 3, 0 );
				
				// Special treatment for "0.0".
				if( 0 == IDcmp( str, L"-0.0" ) || 0 == IDcmp( str, L"0.0" ) )
					str = L"0";

				SetStaticText( ColumnDescription::CD_FlowDeviation, lRow, str );
			}
		}
	}

	int k = 0;
	for( int j = CDS_HydroModX::DTSonHH; j >= CDS_HydroModX::DTS2onDPS; j--, k++ )
	{
		double dT = pHMX->GetWaterTemp( (CDS_HydroModX::eDTS) j );
		if( dT > -273.15 )
		{
			str = WriteCUDouble( _U_TEMPERATURE, dT );
			if( j == pHMX->GetWaterTempRef() && ( true == pHMX->IsQMType( CDS_HydroModX::eQMtype::QMPower ) || true == pHMX->IsQMType( CDS_HydroModX::eQMtype::QMFlow ) ) )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			}
		}
		else 
			str = _T("");

		SetStaticText( ColumnDescription::CD_TempHH + k, lRow, str );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	}

	bool fDisplayDT = ( pHMX->GetDTSensor( 0 ) > -1 && pHMX->GetWaterTemp( pHMX->GetDTSensor( 0 ) ) > -273.15 &&
						pHMX->GetDTSensor( 1 ) > -1 && pHMX->GetWaterTemp( pHMX->GetDTSensor( 1 ) ) > -273.15);

	if( true == fDisplayDT )
	{
		// Write Meas. DT.
		SetStaticText( ColumnDescription::CD_DiffTemp, lRow, pHMX->GetQMstrDT().c_str() );
	}

	if( true == pHMX->IsQMType( CDS_HydroModX::eQMtype::QMDp )  || true == pHMX->IsQMType( CDS_HydroModX::eQMtype::QMFlow ))
	{
		SetStaticText( ColumnDescription::CD_Dp, lRow, pHMX->GetQMstrDp().c_str() );
	}

	// Set the row's height.
	SetRowHeight( lRow, m_dRowHeight /** 1.5*/ );

	// Save the cell parameter
	SetCellParam(ColumnDescription::CD_Note, lRow, ColumnDescription::CD_Last, lRow, (long)pHMX);

	// HYS-864: Add the description of pHMX to the note column
	AddDescriptionToNote(lRow, pHMX->GetDescription()->c_str());
	pHMX->SetDescription(L"");

	// If a note is registered add the Note
	if ((pHMX != NULL) && (!pHMX->GetQMNote().empty()))
	{
		SetMaxRows(lRow + 1);
		AddCellSpanW(ColumnDescription::CD_Note + 1, lRow + 1, ColumnDescription::CD_Last - 1, 1);
		SetTextPattern(CSSheet::_SSTextPattern::UserStaticColumn);
		SetTextPatternProperty(CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE);
		SetTextPatternProperty(CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE);
		SetTextPatternProperty(CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT);
		SetBorderRange(ColumnDescription::CD_Picture, lRow, ColumnDescription::CD_Last - 1, lRow, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_NONE, SPREAD_COLOR_NONE);
		// Draw border.
		SetCellBorder(ColumnDescription::CD_Note, lRow + 1, ColumnDescription::CD_Reference, lRow +1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY);
		SetCellBorder(ColumnDescription::CD_WaterChar, lRow + 1, ColumnDescription::CD_KvSignal, lRow + 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY);
		SetCellBorder(ColumnDescription::CD_Dp, lRow + 1, ColumnDescription::CD_Dp, lRow + 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY);
		SetCellBorder(ColumnDescription::CD_DpFlow, lRow + 1, ColumnDescription::CD_FlowDeviation, lRow + 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY);
		SetCellBorder(ColumnDescription::CD_TempHH, lRow + 1, ColumnDescription::CD_DiffTemp, lRow + 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY);
		SetCellBorder(ColumnDescription::CD_Power, lRow + 1, ColumnDescription::CD_Power, lRow + 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY);
		SetStaticText(ColumnDescription::CD_Note + 1, lRow + 1, pHMX->GetQMNote().c_str());
		lReturn++;
	}
	else
	{
		// Draw border.
		SetCellBorder(ColumnDescription::CD_Note, lRow, ColumnDescription::CD_Reference, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY);
		SetCellBorder(ColumnDescription::CD_WaterChar, lRow, ColumnDescription::CD_KvSignal, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY);
		SetCellBorder(ColumnDescription::CD_Dp, lRow, ColumnDescription::CD_Dp, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY);
		SetCellBorder(ColumnDescription::CD_DpFlow, lRow, ColumnDescription::CD_FlowDeviation, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY);
		SetCellBorder(ColumnDescription::CD_TempHH, lRow, ColumnDescription::CD_DiffTemp, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY);
		SetCellBorder(ColumnDescription::CD_Power, lRow, ColumnDescription::CD_Power, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY);
	}

	return lReturn;
}

void CSSheetQuickMeas::SetWCinSubTitle()
{
	// Get the QUICKMEAS_TAB.
	CTable *pTab = TASApp.GetpTADS()->GetpQuickMeasureTable();

	if( NULL == pTab )
	{
		return;
	}

	// Get the first water characteristic and set it as reference.
	IDPTR IDPtr = pTab->GetFirst();
	CDS_HydroModX *pHMX = dynamic_cast<CDS_HydroModX *>( IDPtr.MP );

	if( NULL == pHMX )
	{
		return;
	}

	// Save the characteristic as 'pWaterChar'.
	CDS_WaterCharacteristic *pWc = dynamic_cast<CDS_WaterCharacteristic*>( pHMX->GetWaterCharIDPtr().MP );
	
	if( NULL == pWc )
	{
		return;
	}

	CWaterChar *pWaterChar = pWc->GetpWCData();

	// Do the loop on all the table and compare each water characteristic.
	bool fSameChar = true;
	
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDS_HydroModX *pHMX = NULL;
		pHMX = (CDS_HydroModX *)( IDPtr.MP );
		
		if( NULL == pHMX )
		{
			continue;
		}

		// Get the water characteristics for the current quick measurement.
		CDS_WaterCharacteristic *pWC = dynamic_cast<CDS_WaterCharacteristic*>( pHMX->GetWaterCharIDPtr().MP );
		ASSERT( NULL != pWC );

		if( 0 == pWC->GetpWCData()->Compare( *pWaterChar, true ) )
		{
			fSameChar = false;
			break;
		}
	}

	// Verify the boolean to set the water characteristic in the subtitle and hide the water characteristic column.
	if( true == fSameChar )
	{
		// Create the strings.
		CString str, str1, str2;
		pWc->BuildWaterStrings( str1, str2 );
		str1 += _T("; ");
		str1 += str2;

		// Redefine the pattern.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
		COLORREF BackColor = GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor );
		SetGroupColor( BackColor );
		
		// Because 'SetTextPattern' is called again in 'SetGroupColor', we must set again word wrap property here.
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
		
		// Set the info into the subtitle.
		str = TASApp.LoadLocalizedString( IDS_SHEETQM_CIRCUIT );
		str += ( _T("\n") + str1 );
		SetStaticText( ColumnDescription::CD_WaterChar, RowDescription::RD_SubTitle, str );
	}
	
	// Resize the valve column to avoid subtitle reading problems.
	SetColWidth( ColumnDescription::CD_WaterChar, 12 + ( fSameChar * 12 ) );

	// Show/Hide the water characteristic column.
	SetColumnAlwaysHidden( ColumnDescription::CD_WaterChar, ColumnDescription::CD_WaterChar, fSameChar );
}

void CSSheetQuickMeas::SetMinMaxFlowDevInSubTitle()
{
	// Get the QUICKMEAS_TAB.
	CTable *pTab = TASApp.GetpTADS()->GetpQuickMeasureTable();

	if( NULL == pTab )
	{
		return;
	}

	// Variables.
	CString str, str1, str2;
	double dMinRatio = 9999;
	double dMaxRatio = -9999;
	bool fSetFlowDev = false;

	// Do the loop on all the table and find the highest and lowest values for the flow deviation.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDS_HydroModX *pHMX = NULL;
		pHMX = (CDS_HydroModX *)( IDPtr.MP );

		if( NULL == pHMX )
		{
			continue;
		}

		// Get the current valve.
		CDS_HydroModX::CBase *pValve = pHMX->GetpBV( CDS_HydroModX::InPrimary );

		if( NULL == pValve )
		{
			pValve = pHMX->GetpCv();
		}

		if( CDS_HydroModX::eQMtype::QMFlow == pHMX->GetQMType() || CDS_HydroModX::eQMtype::QMPower == pHMX->GetQMType() )
		{
			double dFlowRatio = 0.0;

			if( NULL != pValve )
			{
				dFlowRatio = pValve->GetFlowRatio();

				if( 0.0 != dFlowRatio )
				{
					// Set the min ratio.
					if( dFlowRatio < dMinRatio )
					{
						dMinRatio = dFlowRatio;
					}
					
					// Set the max ratio.
					if( dFlowRatio > dMaxRatio )
					{
						dMaxRatio = dFlowRatio;
					}
					
					// The 'min max' text must be displayed.
					fSetFlowDev = true;
				}
			}
		}
	}
	
	// Set the text.
	if( true == fSetFlowDev )
	{
		str1.Format( _T("%f"), dMinRatio - 100.0 );
		int iNegValue = str1.Find('-');
		int iPos = str1.Find('.');
		int i = 0;
		
		if( iPos <= 3 + iNegValue )
		{
			i = 2;					
		}

		str1 = str1.Left( iPos +i );

		// Special treatment for "0.0".
		if( 0 == IDcmp( str1, L"-0.0" ) || 0 == IDcmp( str1, L"0.0" ) )
		{
			str1 = L"0";
		}
		
		// Test if there is one or two values.
		if( dMinRatio != dMaxRatio )
		{
			str2.Format( _T("%f"), dMaxRatio - 100.0 );
			int iNegValue = str2.Find('-');
			int iPos = str2.Find('.');
			int i = 0;
			
			if( iPos <= 3 + iNegValue )
			{
				i = 2;					
			}

			str2 = str2.Left( iPos + i );

			// Special treatment for "0.0".
			if( 0 == IDcmp( str2, L"-0.0" ) || 0 == IDcmp( str2, L"0.0" ) )
			{
				str2 = L"0";
			}

			str1 += _T("% / ");
			str1 += str2;
		}

		str1 += _T("%");

		// Redefine the pattern.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, ( LPARAM )TRUE );

		str = TASApp.LoadLocalizedString( IDS_FLOW );
		str += _T("\r\n") + str1;
		SetStaticText( ColumnDescription::CD_DpFlow, RowDescription::RD_SubTitle, str );
	}
}


void CSSheetQuickMeas::HideEmptyColumns()
{
	long lFromColumn, lToColumn, lFromRow;
	
	lFromColumn = ColumnDescription::CD_None + 1;
	lToColumn = ColumnDescription::CD_Last - 1;
	lFromRow = RowDescription::RD_Last;

	for( long lLoopColumn = lFromColumn; lLoopColumn <= lToColumn; lLoopColumn++ )
	{
		// Do not collapse.
		if( ColumnDescription::CD_Picture == lLoopColumn ||
			ColumnDescription::CD_ReferenceSep == lLoopColumn ||
			ColumnDescription::CD_KvSignalSep == lLoopColumn ||
			ColumnDescription::CD_DpSep == lLoopColumn ||
			ColumnDescription::CD_FlowDeviationSep == lLoopColumn ||
			ColumnDescription::CD_DiffTempSep == lLoopColumn )
			continue;

		// If column is already hidden, continue with the next column.
		if( TRUE == IsColHidden( lLoopColumn ) )
			continue;

		bool fColEmpty = true;
		for( long lLoopRow = lFromRow; lLoopRow <= GetMaxRows() && true == fColEmpty; lLoopRow++ )
		{	
			fColEmpty = GetCellText( lLoopColumn, lLoopRow ).IsEmpty();
		}
		ShowCol( lLoopColumn, ( true == fColEmpty ) ? FALSE : TRUE );
		
		// Do once again a loop and set the Flag Hidden for all cells that are really empty
		// ( ColHidden) is not considered as an empty column.
		fColEmpty = true;
		for( long lLoopRow = lFromRow; lLoopRow <= GetMaxRows() && true == fColEmpty; lLoopRow++ )
		{
			fColEmpty = GetCellText( lLoopColumn, lLoopRow ).IsEmpty();
		}
		if( true == fColEmpty )
			SetFlagAlwaysHidden( lLoopColumn, lLoopColumn, true );
	}
}

void CSSheetQuickMeas::RedefineColWidth()
{
	// Set Default values for column width.
	double dValueWidth = 8.0;
	double dTextWidth = 24.0;
	double dLightTextWidth = 12.0;

	SetColWidth( ColumnDescription::CD_None, 2.0 );
	SetColWidth( ColumnDescription::CD_Picture, 3.0 );
	SetColWidth( ColumnDescription::CD_Note, 1.0 );
	SetColWidth( ColumnDescription::CD_DateTime, dLightTextWidth );
	SetColWidth( ColumnDescription::CD_Reference, dValueWidth );
	SetColWidth( ColumnDescription::CD_ReferenceSep, .8 );
	SetColWidth( ColumnDescription::CD_WaterChar, dTextWidth * .8 );
	SetColWidth( ColumnDescription::CD_Valve, dLightTextWidth );
	SetColWidth( ColumnDescription::CD_Setting, dValueWidth );
	SetColWidth( ColumnDescription::CD_Kv, dValueWidth );
	SetColWidth( ColumnDescription::CD_KvSignal, dValueWidth );
	SetColWidth( ColumnDescription::CD_KvSignalSep, .8 );
	SetColWidth( ColumnDescription::CD_Dp, dValueWidth );
	SetColWidth( ColumnDescription::CD_DpSep, .8 );
	SetColWidth( ColumnDescription::CD_DpFlow, dValueWidth );
	SetColWidth( ColumnDescription::CD_Flow, dValueWidth );
	SetColWidth( ColumnDescription::CD_DesignFlow, dValueWidth );
	SetColWidth( ColumnDescription::CD_FlowDeviation, dValueWidth );
	SetColWidth( ColumnDescription::CD_FlowDeviationSep, .8 );
	SetColWidth( ColumnDescription::CD_TempHH, dValueWidth );
	SetColWidth( ColumnDescription::CD_Temp1DPS, dValueWidth );
	SetColWidth( ColumnDescription::CD_Temp2DPS, dValueWidth );
	SetColWidth( ColumnDescription::CD_DiffTemp, dValueWidth );
	SetColWidth( ColumnDescription::CD_DiffTempSep, .8 );
	SetColWidth( ColumnDescription::CD_Power, dValueWidth );

	// Do a loop on all column and reset the col width if the column is not shown.
	for( long lLoopColumn = ColumnDescription::CD_None; lLoopColumn < ColumnDescription::CD_Last; lLoopColumn++ )
	{
		// This flag is set for all cells that are hidden.
		if( TRUE == IsColHidden( lLoopColumn ) )
			SetColWidth( lLoopColumn, 0.0 );
	}

	// Special treatment to hide the separator depending to the hidden group.
	m_fHideCircuitSubTitle = true;
	m_fHideDpSubTitle = true;
	m_fHideFlowSubTitle = true;
	m_fHideTempSubTitle = true;
	m_fHidePowerSubTitle = true;

	// Circuit.
	for( long lLoopColumn = ColumnDescription::CD_WaterChar; lLoopColumn < ColumnDescription::CD_KvSignalSep; lLoopColumn++ )
	{
		if( false == IsColHidden( lLoopColumn ) )
			m_fHideCircuitSubTitle = false;
	}
	
	// Dp.
	if( false == IsColHidden( ColumnDescription::CD_Dp ) )
		m_fHideDpSubTitle = false;
	
	// Flow.
	for( long lLoopColumn = ColumnDescription::CD_DpFlow; lLoopColumn < ColumnDescription::CD_FlowDeviationSep; lLoopColumn++ )
	{
		if( false == IsColHidden( lLoopColumn ) )
			m_fHideFlowSubTitle = false;
	}
	
	// Temperature.
	for( long lLoopColumn = ColumnDescription::CD_TempHH; lLoopColumn < ColumnDescription::CD_DiffTempSep; lLoopColumn++ )
	{
		if( false == IsColHidden( lLoopColumn ) )
			m_fHideTempSubTitle = false;
	}

	// Define the power status to resize the subtitle.
	if( false == IsColHidden( ColumnDescription::CD_Power ) )
		m_fHidePowerSubTitle = false;

	// Define the separator column width.
	if( true == m_fHideCircuitSubTitle )
		SetColWidth( ColumnDescription::CD_KvSignalSep, 0.0 );
	if( true == m_fHideDpSubTitle )
		SetColWidth( ColumnDescription::CD_DpSep, 0.0 );
	if( true == m_fHideFlowSubTitle )
		SetColWidth( ColumnDescription::CD_FlowDeviationSep, 0.0 );
	if( true == m_fHideTempSubTitle )
		SetColWidth( ColumnDescription::CD_DiffTempSep, 0.0 );
}

void CSSheetQuickMeas::ResizeAndColorSubTitles()
{
	if( false == m_fHideDpSubTitle && true == m_fHideFlowSubTitle && true == m_fHideTempSubTitle && true == m_fHidePowerSubTitle )
	{	
		// Resize the Dp Sub-Title if it is alone.
		AddCellSpanW( ColumnDescription::CD_Dp, RowDescription::RD_SubTitle, ColumnDescription::CD_DpSep - ColumnDescription::CD_Dp + 1, 2 );
	}
	else if( false == m_fHideFlowSubTitle && true == m_fHideTempSubTitle && true == m_fHidePowerSubTitle )
	{
		// Resize the Flow Sub-Title if it is alone.
		AddCellSpanW( ColumnDescription::CD_DpFlow, RowDescription::RD_SubTitle, ColumnDescription::CD_FlowDeviationSep - ColumnDescription::CD_DpFlow + 1, 2 );
	}

	// Define variables for the background color.
	SetTextPattern( TitleGroup );
	COLORREF BackColor = GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor );
	SetGroupColor( BackColor );
	
	// Set the color for the circuit.
	if( false == m_fHideCircuitSubTitle )
	{
		SetBackColor( ColumnDescription::CD_WaterChar, RowDescription::RD_SubTitle, BackColor );
		SetGroupColor( BackColor );
	}

	// Set the color for the Dp.
	if( false == m_fHideDpSubTitle )
	{
		SetBackColor( ColumnDescription::CD_Dp, RowDescription::RD_SubTitle, BackColor );
		SetGroupColor( BackColor );
	}

	// Set the color for the flow.
	if( false == m_fHideFlowSubTitle )
	{
		SetBackColor( ColumnDescription::CD_DpFlow, RowDescription::RD_SubTitle, BackColor );
		SetGroupColor( BackColor );
	}

	// Set the color for the temperature.
	if( false == m_fHideTempSubTitle )
	{
		SetBackColor( ColumnDescription::CD_TempHH, RowDescription::RD_SubTitle, BackColor );
		SetGroupColor( BackColor );
	}

	// Set the color for the power.
	if( false == m_fHidePowerSubTitle )
	{
		SetBackColor( ColumnDescription::CD_Power, RowDescription::RD_SubTitle, BackColor );
		SetGroupColor( BackColor );
	}
}

void CSSheetQuickMeas::SetColWidth( long lColumn, double dSize )
{
	ASSERT( lColumn <= m_lColn );
	m_arlColWidth[lColumn] = dSize;
	CSSheet::SetColWidth( lColumn, dSize );
}

void CSSheetQuickMeas::SetGroupColor( COLORREF &Color )
{
	SetTextPattern( TitleGroup );
	if( _IMI_TITLE_GROUP2 != Color )
	{
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
	}
	Color = GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor );
}

void CSSheetQuickMeas::CollapseAll()
{
	m_arlColumnExpandState.RemoveAll();
	for( long lLoopColumn = ColumnDescription::CD_DateTime; lLoopColumn < ColumnDescription::CD_Last; lLoopColumn++ )
	{
		if( CSSheet::ExpandCollapseColumnState::ECCS_Expanded == GetExpandCollapseColumnState( lLoopColumn, RowDescription::RD_InfoSubTitle ) )
		{
			ChangeExpandCollapseColumnButtonState( lLoopColumn, RowDescription::RD_InfoSubTitle, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
		}
	}
}

void CSSheetQuickMeas::ExpandAll()
{
	m_arlColumnExpandState.RemoveAll();
	for( long lLoopColumn = ColumnDescription::CD_DateTime; lLoopColumn < ColumnDescription::CD_Last; lLoopColumn++ )
	{
		if( CSSheet::ExpandCollapseColumnState::ECCS_Collapsed == GetExpandCollapseColumnState( lLoopColumn, RowDescription::RD_InfoSubTitle ) )
		{
			ChangeExpandCollapseColumnButtonState( lLoopColumn, RowDescription::RD_InfoSubTitle, RowDescription::RD_Last, GetMaxRows(), CSSheet::ExpandCollapseColumnAction::ECCA_ToExpand );
			m_arlColumnExpandState.Add( lLoopColumn );
		}
	}
}

void CSSheetQuickMeas::AddDescriptionToNote(long lRow, CString strDescription)
{
	CDS_HydroModX *pHMX = NULL;
	CString strSheetNote = _T("");
	pHMX = (CDS_HydroModX *)GetCellParam(ColumnDescription::CD_Note, lRow);
	
	if (NULL == pHMX)
		return;

	if (0 != StringCompare(strDescription, _T("")))
	{
		CString strNote = ((!pHMX->GetQMNote().empty()) ? pHMX->GetQMNote().c_str() : _T(""));
		if (0 != StringCompare(strNote, _T("")))
		{
			strNote += _T("\r\n");
		}
		strNote += strDescription;
		FormatString(strSheetNote, IDS_SHEETQM_NOTE, strNote);
		pHMX->SetQMNote((wstring)(strSheetNote));
	}
}

void CSSheetQuickMeas::AddNotes(long lColumn, long lRow)
{
	CDS_HydroModX *pHMX = NULL;
	long lnRow = GetMaxRows();
	CString strNote = _T("");
	CString strSheetNote = _T("");
	SS_CELLTYPE img;
	bool bIsAnImage = false;
	//bHasNoNote = true if this is the first note 
	bool bHasNoNote = true;
	DlgQuickMeasNote* pDlgQMNote = NULL;
	// if the cell is a picture
	bIsAnImage = IsCellIsAPicture(lColumn, lRow, img);
	if (lRow < lnRow)
	{
		bHasNoNote = IsCellIsAPicture(lColumn, lRow + 1, img);
	}

	if (!bHasNoNote && bIsAnImage)
	{
		pDlgQMNote = new DlgQuickMeasNote(GetCellText(lColumn + 1, lRow + 1), this);
	}
	else
	{
		pDlgQMNote = new DlgQuickMeasNote(this);
	}
	// check the picture because the row that contains new note has no picture so the left click has no effect
	if (bIsAnImage)
	{
		if (IDOK == pDlgQMNote->DoModal())
		{
			strNote = pDlgQMNote->GetTextNote();
			FormatString(strSheetNote, IDS_SHEETQM_NOTE, strNote);

			if (0 != StringCompare(strNote, _T("")))
			{
				//put the note in the row
				pHMX = (CDS_HydroModX *)GetCellParam(lColumn, lRow);
				pHMX->SetQMNote((wstring)(strSheetNote));
			}
			else
			{
				// remove the note
				pHMX = (CDS_HydroModX *)GetCellParam(lColumn, lRow);
				pHMX->SetQMNote(_T(""));
			}

			SetRedraw();
			CRect rect;
			GetParent()->GetClientRect(&rect);
			int	iWidth = rect.Width();
			ResizeColumns( max(iWidth, RIGHTVIEWWIDTH) );
		}
	}
	delete pDlgQMNote;
}

BEGIN_MESSAGE_MAP(CSSheetQuickMeas, CSSheet)
	// Spread DLL messages
	ON_MESSAGE(SSM_TEXTTIPFETCH, TextTipFetchQmSpread)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Spread DLL message handlers.
/////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CSSheetQuickMeas::TextTipFetchQmSpread(WPARAM wParam, LPARAM lParam)
{
	SS_TEXTTIPFETCH* pTextTipFetch = (SS_TEXTTIPFETCH *)lParam;

	// If hText is not empty, spread takes hText in place of szText!
	pTextTipFetch->hText = NULL;
	OnTextTipFetch(pTextTipFetch->Col, pTextTipFetch->Row, &pTextTipFetch->wMultiLine, pTextTipFetch->szText, &pTextTipFetch->fShow);
	return 0;
}

void CSSheetQuickMeas::OnTextTipFetch(long lColumn, long lRow, WORD* pwMultiLine, TCHAR* pstrTipText, BOOL* pfShowTip)
{
	*pfShowTip = false;
	*pwMultiLine = SS_TT_MULTILINE_AUTO;
	SS_CELLTYPE img;
	bool bHasAnImage = true;
	CString str = TASApp.LoadLocalizedString( IDS_SHEETQM_TIP );
	bHasAnImage = IsCellIsAPicture(lColumn, lRow, img);
	
	if ((lColumn == ColumnDescription::CD_Note) && (bHasAnImage))
	{
		if (false == str.IsEmpty())
		{
			wcsncpy_s(pstrTipText, SS_TT_TEXTMAX, (LPCTSTR)str, SS_TT_TEXTMAX);
			*pfShowTip = true;
		}
	}
}