#include "stdafx.h"
#include "TASelect.h"
#include "HydroMod.h"
#include "SSheetGen.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSSheetGen::CSSheetGen()
{
	m_arColWidth.SetSize( ecw_Last );
	m_iColn			= ecw_Last - 1;
	m_iTitleRow		= 2;
	m_dRowHeight	= 12.75;
	m_dPageWidth		= 0.0;
	m_iHeaderHNRow	= m_iTitleRow + 2;
	m_iHeaderLDRow	= m_iHeaderHNRow + 3;
	m_iHeaderQMRow	= m_iHeaderLDRow + 3;
	m_iCurRow		= 1;
}

BOOL CSSheetGen::Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID )
{
	if( FALSE == CSSheet::Create( dwStyle, rect, pParentWnd, nID ) ) 
		return FALSE;
	
	CSSheet::Init();

	// Set up main parameters not yet set in base class CSheet.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
	SetBool( SSB_ALLOWDRAGDROP, FALSE );
	
	return TRUE;
}

void CSSheetGen::ResizeColumns( double dWidth )
{
	SetBool( SSB_REDRAW, FALSE );
	
	double dFactor = dWidth / m_dPageWidth;
	dFactor = max( 1, dFactor );
	for( int iLoopColumn = ecw_Description; iLoopColumn < ecw_Last; iLoopColumn++ )
	{
		double dValue = m_arColWidth[iLoopColumn];
		dValue *= dFactor;
		CSSheet::SetColWidth( iLoopColumn, dValue );
	}
	SetBool( SSB_REDRAW, TRUE );
	Invalidate();
}

bool CSSheetGen::SetRedraw()
{
	SetBool( SSB_REDRAW, FALSE );

	// Variables.
	long lMaxRow = 0;

	// Define the number of rows needed for the 'Title'.
	lMaxRow += 3;

	// Define the number of rows needed for the 'Hydronic Network' information.
	lMaxRow += 4;

	// Define the number of rows needed for the 'Logged Data' information.
	lMaxRow += 4;

	// Define the number of rows needed for the 'Quick Measurement' information.
	lMaxRow += 3;

	SetMaxRows( lMaxRow );

	// Define the number of columns needed.
	SetMaxCols( m_iColn );

	// Define the columns' width.
	_SetColWidth( 0, 0 );
	_SetColWidth( ecw_None, 2 );
	_SetColWidth( ecw_Description, 10 );

	// To be sure that all cells are set in static mode by default.
	CSSheet::FormatStaticText( -1, -1, -1, -1, _T("") );

	// Define the page width.
	CRect rect = GetSheetSizeInPixels( false );
	m_dPageWidth = rect.Width();
	m_dPageWidth *= 0.83;

	// Format the header.
	_FormatHeader();

	// Format and fill information for hydronic network, logged data and quick measurement.
	_FormatHN();
	_FormatLD();
	_FormatQM();

	SetBool( SSB_REDRAW, TRUE );

	return true;
}

void CSSheetGen::_FormatHeader()
{
	// Variables.
	CDC* pDC = GetDC();
	double DeviceCaps = pDC->GetDeviceCaps( LOGPIXELSY );
	
	// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
	ReleaseDC( pDC );
	
	m_dRowHeight = 12.75 * DeviceCaps / 96.0;		  //96 Dpi as reference.
	CString str;	

	// Format the column heights.
	SetRowHeight( m_iTitleRow - 1, m_dRowHeight * 0.5 );	// First Empty line
	SetRowHeight( m_iTitleRow, m_dRowHeight * 2 );			// Title
	SetRowHeight( m_iTitleRow + 1, m_dRowHeight * 0.5 );	// Empty line

	// Format the column headers.
	SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_TITLE_MAIN );
	
	// Set the text in the title.
	CDS_ProjectRef *pPrjRef = TASApp.GetpTADS()->GetpProjectRef();
	ASSERT( NULL != pPrjRef );

	str = pPrjRef->GetString( CDS_ProjectRef::Name );
	AddCellSpanW( ecw_None + 1, m_iTitleRow, ecw_Last - ( ecw_None + 1 ), 1 );
	SetStaticText( ecw_Description, m_iTitleRow, str );
}

void CSSheetGen::_FormatHN()
{
	// Variables.
	CString str;

	// Define the header background.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_TITLE_SECOND );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_TAH_WHITE );

	// Draw border.
	SetCellBorder( ecw_None + 1, m_iHeaderHNRow - 1, ecw_Last - 1, m_iHeaderHNRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	SetCellBorder( ecw_None + 1, m_iHeaderHNRow, ecw_Last - 1, m_iHeaderHNRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	// Set the text for the header.
	str = TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK );
	SetStaticText( ecw_Description, m_iHeaderHNRow, str );

	// Define the information background.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// The first line after the header.
	m_iCurRow = m_iHeaderHNRow + 2;

	// Set the number of modules	.
	int iCount = _CountNbrHydroMod();
	str.Format( TASApp.LoadLocalizedString( IDS_SHEETGEN_NBRMODULES ), iCount );
	SetStaticText( ecw_Description, m_iCurRow++, str );

	// Increment the row to have a white line.
	m_iCurRow++;
}

void CSSheetGen::_FormatLD()
{
	// Variables.
	CString str;

	// Redefine the 'm_iHeaderLDRow' position taking into account the number of lines needed for the hydraulic network.
	m_iHeaderLDRow = m_iCurRow;

	// Define the header background.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_TITLE_SECOND );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_TAH_WHITE );

	// Draw border.
	SetCellBorder( ecw_None + 1, m_iHeaderLDRow - 1, ecw_Last - 1, m_iHeaderLDRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	SetCellBorder( ecw_None + 1, m_iHeaderLDRow, ecw_Last - 1, m_iHeaderLDRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	// Set the text for the header.
	str = TASApp.LoadLocalizedString( IDS_LOGGEDDATA );
	SetStaticText( ecw_Description, m_iHeaderLDRow, str );

	// Define the information background.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// The first line after the header.
	m_iCurRow = m_iHeaderLDRow + 2;

	// Set the number of logged data.
	int iCount = _CountNbrLoggedData();
	str.Format( TASApp.LoadLocalizedString( IDS_SHEETGEN_NBRLOGDATA ), iCount );
	SetStaticText( ecw_Description, m_iCurRow++, str );

	// Increment the row to have a white line.
	m_iCurRow++;
}

void CSSheetGen::_FormatQM()
{
	// Variables.
	CString str;
	
	// Redefine the 'm_iHeaderLDRow' position taking into account the number of lines needed for the hydraulic network and
	// the logged data.
	m_iHeaderQMRow = m_iCurRow;

	// Define the header background.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_TITLE_SECOND );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_TAH_WHITE );

	// Draw border.
	SetCellBorder( ecw_None + 1, m_iHeaderQMRow - 1, ecw_Last - 1, m_iHeaderQMRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	SetCellBorder( ecw_None + 1, m_iHeaderQMRow, ecw_Last - 1, m_iHeaderQMRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	// Set the text for the header.
	str = TASApp.LoadLocalizedString( IDS_SHEETGEN_QUICKMEAS );
	SetStaticText( ecw_Description, m_iHeaderQMRow, str );

	// Define the information background.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// The first line after the header.
	m_iCurRow = m_iHeaderQMRow + 2;

	// Set the number of quick measurements.
	int iCount = _CountNbrQuickMeas();
	str.Format( TASApp.LoadLocalizedString( IDS_SHEETGEN_NBRQUICKMEAS ), iCount );
	SetStaticText( ecw_Description, m_iCurRow++, str );
}

int CSSheetGen::_CountNbrHydroMod( CTable *pTab )
{
	// Variable.
	int iCount = 0;

	if( NULL == pTab )
	{
		pTab = TASApp.GetpTADS()->GetpHydroModTable();
		
		if( 0 == pTab->GetItemCount() )
		{
			return 0;
		}
	}

	ASSERT( NULL != pTab );
	
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( IDPtr.MP );

		if( NULL == pHM )
		{
			return 0;
		}

		if( true == pHM->IsaModule() )
		{
			iCount++;
			iCount += _CountNbrHydroMod( pHM );
		}
	}

	return iCount;
}

int CSSheetGen::_CountNbrLoggedData()
{
	// Variable.
	int iCount = 0;

	// Get the table with all logged data.
	CTable *pTab = TASApp.GetpTADS()->GetpLogDataTable();
	ASSERT( NULL != pTab );

	if( NULL == pTab )
	{
		return 0;
	}

	// Do a loop on the table.
	for( IDPTR LDIDPtr = pTab->GetFirst(); _T('\0') != *LDIDPtr.ID; LDIDPtr = pTab->GetNext() )
	{
		if( true == LDIDPtr.MP->IsaTable() )
		{
			continue;
		}

		iCount++;
	}

	return iCount;
}

int CSSheetGen::_CountNbrQuickMeas()
{
	// Get the table with all quick measurements.
	CTable *pTab = TASApp.GetpTADS()->GetpQuickMeasureTable();
	ASSERT( NULL != pTab );

	if( NULL == pTab )
	{
		return 0;
	}

	return pTab->GetItemCount();
}

void CSSheetGen::_SetColWidth( int iCol, double dSize )
{
	ASSERT( iCol <= m_iColn );
	m_arColWidth[iCol] = dSize;
	CSSheet::SetColWidth( iCol, dSize );
}
