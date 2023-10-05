#include "stdafx.h"
#include "TASelect.h"
#include "Utilities.h"
#include "HydroMod.h"
#include "SSheetHMSumm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSSheetHMSumm::CSSheetHMSumm()
{
	m_lColn = ColumnDescription::CD_Last - 1;
	m_lTitleRow	= 2;
	m_lHeaderRow = 4;
	m_dRowHeight = 12.75;
	m_fSheetHasData = false;
	m_ColWidth.SetSize( ColumnDescription::CD_Last );
	m_dPageWidth = 0.0;
	m_pclSSheetToDraw = NULL;
}

BOOL CSSheetHMSumm::Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID )
{
	if( FALSE == CSSheet::Create( dwStyle, rect, pParentWnd, nID ) ) 
		return FALSE;
	
	m_pclSSheetToDraw = this;

	return TRUE;
}

void CSSheetHMSumm::ResizeColumns( double dWidth )
{
	m_pclSSheetToDraw->SetBool( SSB_REDRAW, FALSE );

	double dFactor = dWidth / m_dPageWidth;
	dFactor = max( 1, dFactor );
	for( long lLoopColumn = ColumnDescription::CD_Name; lLoopColumn < ColumnDescription::CD_Last; lLoopColumn++)
	{
		double dValue = m_ColWidth[lLoopColumn];
		dValue *= dFactor;
		m_pclSSheetToDraw->SetColWidth( lLoopColumn, dValue );
	}

	m_pclSSheetToDraw->SetBool( SSB_REDRAW, TRUE );
	Invalidate();
}

bool CSSheetHMSumm::SetRedraw( CSSheet* pclSheet, bool fPrint )
{
	if( NULL != pclSheet )
		m_pclSSheetToDraw = pclSheet;

	m_fSheetHasData = false;
	m_pclSSheetToDraw->SetBool( SSB_REDRAW, FALSE );

	// Init the sheet.
	m_pclSSheetToDraw->Init();

	// Disable the drag and drop function.
	m_pclSSheetToDraw->SetBool( SSB_ALLOWDRAGDROP, FALSE );

	// All cells are static by default and filled with _T("").
	m_pclSSheetToDraw->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pclSSheetToDraw->FormatStaticText( -1, -1, -1, -1, _T("") );
	
	// In case it is a print, do not print the first white column.
	if( true == fPrint )
		m_pclSSheetToDraw->ShowCol( ColumnDescription::CD_None, FALSE );

	// Count the number of HM.
	CTable *pTab = TASApp.GetpTADS()->GetpHydroModTable();
	ASSERT( NULL != pTab );

	int iCount = TASApp.GetpTADS()->GetHMCount( pTab );

	m_pclSSheetToDraw->SetMaxRows( 1 );

	// Determine the max row number.
	int iRowMax = iCount + 2;	// +2 for header
	
	// ...
	m_pclSSheetToDraw->SetMaxRows( iRowMax + 3 );	// +3 for the Title
	m_pclSSheetToDraw->SetMaxCols( m_lColn );
	
	// Set default values for column width.
	_SetColWidth( ColumnDescription::CD_None, 2 );
	_SetColWidth( ColumnDescription::CD_Name, 20 );
	_SetColWidth( ColumnDescription::CD_Description, 32 );
	_SetColWidth( ColumnDescription::CD_ParentModule, 20 );
	_SetColWidth( ColumnDescription::CD_ValveNumber, 10 );
	_SetColWidth( ColumnDescription::CD_TotalFlow, 10 );

	// Define the page width.
	CRect rect = m_pclSSheetToDraw->GetSheetSizeInPixels( false );
	m_dPageWidth = rect.Width();

	// Format the headers.
	_InitHeader();

	if( 0 == iCount )
	{
		m_pclSSheetToDraw->SetBool( SSB_REDRAW, TRUE );
		return m_fSheetHasData;
	}

	m_fSheetHasData = true;

	//====================================
	// Fill and format the table.
	//====================================
	m_pclSSheetToDraw->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	m_pclSSheetToDraw->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	m_pclSSheetToDraw->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_VLIGHTGRAY );

	long lRow = m_lHeaderRow + 1;
	_ScanAndFillRow( &lRow, pTab );

	m_pclSSheetToDraw->SetSheetName( GetSheet(), TASApp.LoadLocalizedString( IDS_SHEETHMSUMM_SHEETNAME ) );
	
	m_pclSSheetToDraw->SetBool( SSB_REDRAW, TRUE );

	return m_fSheetHasData;
}

void CSSheetHMSumm::_InitHeader()
{
	// Variables.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();			ASSERT( NULL != pUnitDB );
	CDC* pDC = GetDC();
	double dDeviceCaps = pDC->GetDeviceCaps( LOGPIXELSY );

	// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
	ReleaseDC( pDC );

	m_dRowHeight = 12.75 * dDeviceCaps / 96.0;		  //96 Dpi as reference.
	TCHAR unitname[_MAXCHARS];
	CString str;	

	// Format the column heights.
	m_pclSSheetToDraw->SetRowHeight( m_lTitleRow - 1, m_dRowHeight * 0.5 );		// First Empty line
	m_pclSSheetToDraw->SetRowHeight( m_lTitleRow, m_dRowHeight * 2 );				// Title
	m_pclSSheetToDraw->SetRowHeight( m_lTitleRow + 1, m_dRowHeight * 0.5 );		// Empty line
	m_pclSSheetToDraw->SetRowHeight( m_lHeaderRow, m_dRowHeight * 0.7 );			// Header line
	m_pclSSheetToDraw->SetRowHeight( m_lHeaderRow + 1, m_dRowHeight * 0.7 );		// Header line units

	// Format column headers.
	m_pclSSheetToDraw->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	
	// Set the text in the title.
	str = TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK );
	m_pclSSheetToDraw->AddCellSpanW( ColumnDescription::CD_Name, m_lTitleRow, ColumnDescription::CD_Last - ColumnDescription::CD_Name, 1 );
	m_pclSSheetToDraw->SetStaticText( ColumnDescription::CD_Name, m_lTitleRow, str );

	// Format the column headers.
	m_pclSSheetToDraw->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	m_pclSSheetToDraw->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _WHITE );
	
	// Set the text of column headers.
	str = TASApp.LoadLocalizedString( IDS_MODULENAME );
	m_pclSSheetToDraw->SetStaticText( ColumnDescription::CD_Name, m_lHeaderRow, str );
	m_pclSSheetToDraw->AddCellSpanW( ColumnDescription::CD_Name, m_lHeaderRow, 1, 2 );
	
	str = TASApp.LoadLocalizedString( IDS_MODULEDESCRIPTION );
	m_pclSSheetToDraw->SetStaticText( ColumnDescription::CD_Description, m_lHeaderRow, str );
	m_pclSSheetToDraw->AddCellSpanW( ColumnDescription::CD_Description, m_lHeaderRow, 1, 2 );

	str = TASApp.LoadLocalizedString( IDS_PARENTMODULE );
	m_pclSSheetToDraw->SetStaticText( ColumnDescription::CD_ParentModule, m_lHeaderRow, str );
	m_pclSSheetToDraw->AddCellSpanW( ColumnDescription::CD_ParentModule, m_lHeaderRow, 1, 2 );

	str = TASApp.LoadLocalizedString( IDS_NBROFVALVES );
	m_pclSSheetToDraw->SetStaticText( ColumnDescription::CD_ValveNumber, m_lHeaderRow, str );
	m_pclSSheetToDraw->AddCellSpanW( ColumnDescription::CD_ValveNumber, m_lHeaderRow, 1, 2 );

	str = TASApp.LoadLocalizedString( IDS_FLOW );
	m_pclSSheetToDraw->SetStaticText( ColumnDescription::CD_TotalFlow, m_lHeaderRow, str );	
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), unitname );
	str = (CString)unitname;
	m_pclSSheetToDraw->SetStaticText( ColumnDescription::CD_TotalFlow, m_lHeaderRow + 1, str );

	// Draw border.
	m_pclSSheetToDraw->SetCellBorder( ColumnDescription::CD_Name, m_lHeaderRow, ColumnDescription::CD_Last - 1, m_lHeaderRow, true, SS_BORDERTYPE_TOP, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	m_pclSSheetToDraw->SetCellBorder( ColumnDescription::CD_Name, m_lHeaderRow + 1, ColumnDescription::CD_Last - 1, m_lHeaderRow + 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	// Freeze row header.
	m_pclSSheetToDraw->SetFreeze( 0, m_lHeaderRow + 1 );
}

long CSSheetHMSumm::_FillRow( long lRow, CDS_HydroMod *pHM )
{
	if (NULL == pHM )
		return 0;
	
	long lLine = 2;
	CString str1, str2;

	// Write the name of the HM.
	m_pclSSheetToDraw->SetStaticText( ColumnDescription::CD_Name, lRow , pHM->GetHMName() );
	
	// Write description.
	str1 = pHM->GetDescription();
	if( true == str1.IsEmpty() )
		m_pclSSheetToDraw->SetStaticText( ColumnDescription::CD_Description, lRow, _T("-") );
	else
		m_pclSSheetToDraw->SetStaticText( ColumnDescription::CD_Description, lRow, pHM->GetDescription() );

	// Write parent module.
	CTable* pTab = pHM->GetIDPtr().PP;
	if( 0 == pTab->IsClass( CLASS( CTableHM ) ) )
	{
		CDS_HydroMod* pPHM = (CDS_HydroMod*)( pHM->GetIDPtr().PP );
		m_pclSSheetToDraw->SetStaticText( ColumnDescription::CD_ParentModule, lRow, pPHM->GetHMName() );
	}
	else
		m_pclSSheetToDraw->SetStaticText( ColumnDescription::CD_ParentModule, lRow, _T("-") );

	// Write number of valves
	// Use CDS_HydroMod::GetValveCount() 
	// because the number returned by GetCount() is actually the total number 
	// of children HM including those that do not have a defined PV
	str1.Format( _T("%d"), pHM->GetValveCount(true) );
	m_pclSSheetToDraw->SetStaticText( ColumnDescription::CD_ValveNumber, lRow, str1 );

	// Write total flow
	// !!!!!!!!! TODO: check if Getqd covers all cases (wrt valve definition)
	m_pclSSheetToDraw->SetStaticText( ColumnDescription::CD_TotalFlow, lRow, WriteDouble( CDimValue::SItoCU( _U_FLOW, pHM->GetQDesign() ), 3, 0, 0 ) );

	return lLine;
}

void CSSheetHMSumm::_ScanAndFillRow( long *plRow, CTable *pHM )
{
	if( NULL == pHM )
		return;
	
	CRank SortList;
	for( IDPTR IDPtrChild = pHM->GetFirst(); *IDPtrChild.ID; IDPtrChild = pHM->GetNext( IDPtrChild.MP ) )
	{
		CDS_HydroMod *pChild = (CDS_HydroMod *)IDPtrChild.MP;
		// Fill a CRank list to sort items 
		SortList.Add( _T(""), pChild->GetPos(), (long)pChild );
	}
	
	// Insert sorted items into the tree.
	CString str;
	LPARAM itemdata;
	for( BOOL fContinue = SortList.GetFirst( str, itemdata ); TRUE == fContinue; fContinue = SortList.GetNext( str, itemdata ) )
	{
		CDS_HydroMod *pChild = (CDS_HydroMod *)itemdata;
		if( true == pChild->IsaModule() )
		{
			// Alternate the background color between gray and white.
			if( m_pclSSheetToDraw->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) == _VLIGHTGRAY )
				m_pclSSheetToDraw->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
			else
				m_pclSSheetToDraw->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_VLIGHTGRAY );

			// FillRow and format.
			long lLine = _FillRow( ++(*plRow), pChild );
			
			// Set the row's height.
			m_pclSSheetToDraw->SetRowHeight( *plRow, m_dRowHeight * 0.70 );
				
			// Draw border.
			m_pclSSheetToDraw->SetCellBorder( ColumnDescription::CD_Name, *plRow, ColumnDescription::CD_Last - 1, *plRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

			// Scan all child for this branch....
			_ScanAndFillRow( plRow, pChild );
		}
	}
}

void CSSheetHMSumm::_SetColWidth( long lColumn, double dSize )
{
	ASSERT( lColumn <= m_lColn );
	m_ColWidth[lColumn] = dSize;
	m_pclSSheetToDraw->SetColWidth( lColumn, dSize );
}
