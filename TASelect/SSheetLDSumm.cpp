#include "stdafx.h"
#include "TASelect.h"
#include "Utilities.h"
#include "SSheetLDSumm.h"


CSSheetLDSumm::CSSheetLDSumm()
{
	m_lColn	= ColumnDescription::CD_Last - 1;
	m_lTitleRow	= 2;
	m_lHeaderRow = 4;
	m_dRowHeight = 12.75;
	m_fSheetHasData = false;
	m_ardColWidth.SetSize( ColumnDescription::CD_Last );
	m_dPageWidth = 0.0;
}

BOOL CSSheetLDSumm::Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID )
{
	if( FALSE == CSSheet::Create( dwStyle, rect, pParentWnd, nID ) ) 
		return FALSE;
	
	CSSheet::Init();

	// Set up main parameters not yet set in base class CSheet.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		
	return TRUE;
}

void CSSheetLDSumm::ResizeColumns( double dWidth )
{
	SetBool( SSB_REDRAW, FALSE );
	
	double dFactor = dWidth / m_dPageWidth;	
	dFactor = max( 1, dFactor );
	
	for( long lLoopColumn = ColumnDescription::CD_Name; lLoopColumn < ColumnDescription::CD_Last; lLoopColumn++ )
	{
		double dValue = m_ardColWidth[lLoopColumn];
		dValue *= dFactor;
		CSSheet::SetColWidth( lLoopColumn, dValue );
	}

	SetBool( SSB_REDRAW, TRUE );
	Invalidate();
}

// Fill SSheetLDSumm with summary of available logged data 
// Return true if there is at least one logged data.
bool CSSheetLDSumm::SetRedraw( bool fPrint )
{
	m_fSheetHasData = false;
	SetBool( SSB_REDRAW, FALSE );

	// Init the sheet.
	Init();

	// Disable the drag and drop function.
	SetBool( SSB_ALLOWDRAGDROP, FALSE );

	// In case it is a print, do not print the first white column.
	if( true == fPrint )
		ShowCol( ColumnDescription::CD_None, FALSE );

	// Count the number of logged data.
	long lCount = 0;
	CTable *pTab = TASApp.GetpTADS()->GetpLogDataTable();
	ASSERT( NULL != pTab );
	
	IDPTR LDIDPtr = _NULL_IDPTR;
	
	for( LDIDPtr = pTab->GetFirst(); _T('\0') != *LDIDPtr.ID; LDIDPtr = pTab->GetNext() )
	{
		if( true == LDIDPtr.MP->IsaTable() )
		{
			continue;
		}

		lCount++;
	}

	if( 0 == lCount )
	{
		return false;
	}
	
	m_fSheetHasData = true;
	SetMaxRows( 1 );

	// Determine the max number of rows.
	long lRowMax = lCount + 1;	// +1 for header
	
	SetMaxRows( lRowMax + 4 );	// +2 for first empty line and last empty line
	SetMaxCols( m_lColn );
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	FormatStaticText( -1, -1, -1, -1, _T("") );
	
	// Set Default Values for column width
	SetColWidth( ColumnDescription::CD_None, 2 );
	SetColWidth( ColumnDescription::CD_Name, 14 );
	SetColWidth( ColumnDescription::CD_Time, 14 );
	SetColWidth( ColumnDescription::CD_Duration, 12 );
	SetColWidth( ColumnDescription::CD_Plant, 15 );
	SetColWidth( ColumnDescription::CD_Module, 13 );
	SetColWidth( ColumnDescription::CD_ValveIndex, 9 );
	SetColWidth( ColumnDescription::CD_LogType, 25 );

	// Define the page width.
	CRect rect = GetSheetSizeInPixels( false );
	m_dPageWidth = rect.Width();

	// Format the header.
	InitHeader();

	//====================================
	// Fill and format the table.
	//====================================
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	
	long lRow = m_lHeaderRow;
	CString str;
	for( LDIDPtr = pTab->GetFirst(); '\0' != *LDIDPtr.ID; LDIDPtr = pTab->GetNext() )
	{
		// The table can have CDS_LoggedData, CDS_LogDataX and TMPLOGDATA_TAB.
		// Only the two first are CLog.
		if( true == LDIDPtr.MP->IsaTable() )
			continue;

		CLog *pLD = (CLog *)( LDIDPtr.MP );			ASSERT(NULL != pLD );
		if( NULL == pLD )
			continue;
		
		// Fill the row with data.
		FillRow( ++lRow, pLD );
		
		// Set the row's height.
		SetRowHeight( lRow, m_dRowHeight );
			
		// Draw border.
		SetCellBorder( ColumnDescription::CD_Name, lRow, ColumnDescription::CD_LogType, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	}

	SetBool( SSB_REDRAW, TRUE );

	return m_fSheetHasData;
}

void CSSheetLDSumm::InitHeader()
{
	CDC* pDC = GetDC();
	double dDeviceCaps = pDC->GetDeviceCaps( LOGPIXELSY );

	// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
	ReleaseDC(pDC);

	m_dRowHeight = 12.75 * dDeviceCaps / 96.0;		  //96 Dpi as reference.

	// Format the column heights.
	SetRowHeight( m_lTitleRow - 1, m_dRowHeight * 0.5 );		// First Empty line
	SetRowHeight( m_lTitleRow, m_dRowHeight * 2 );				// Title
	SetRowHeight( m_lTitleRow + 1, m_dRowHeight * 0.5 );		// Empty line
	SetRowHeight( m_lHeaderRow, m_dRowHeight * 1.6 );			// Header line

	// Format column headers.
	SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	
	// Set the text in the title.
	CString str = TASApp.LoadLocalizedString( IDS_LOGGEDDATAS );
	AddCellSpanW( ColumnDescription::CD_Name, m_lTitleRow, ColumnDescription::CD_Last - ColumnDescription::CD_Name, 1 );
	SetStaticText( ColumnDescription::CD_Name, m_lTitleRow, str );

	// Format the column headers.
	SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_WHITE );
	
	// Set the text of column headers
	str = TASApp.LoadLocalizedString( IDS_NAME );
	SetStaticText( ColumnDescription::CD_Name, m_lHeaderRow, str );
	str = TASApp.LoadLocalizedString( IDS_STARTINGTIME );
	SetStaticText( ColumnDescription::CD_Time, m_lHeaderRow, str );
	str = TASApp.LoadLocalizedString( IDS_DURATION );
	SetStaticText( ColumnDescription::CD_Duration, m_lHeaderRow, str );
	str = TASApp.LoadLocalizedString( IDS_PLANT );
	SetStaticText( ColumnDescription::CD_Plant, m_lHeaderRow, str );
	str = TASApp.LoadLocalizedString( IDS_MODULE );
	SetStaticText( ColumnDescription::CD_Module, m_lHeaderRow, str );
	str = TASApp.LoadLocalizedString( IDS_SHEETHDR_VALVEINDEX );
	SetStaticText( ColumnDescription::CD_ValveIndex, m_lHeaderRow, str );
	str = TASApp.LoadLocalizedString( IDS_LOGTYPE );
	SetStaticText( ColumnDescription::CD_LogType, m_lHeaderRow, str );

	// Draw border.
	SetCellBorder( ColumnDescription::CD_Name, m_lHeaderRow, ColumnDescription::CD_LogType, m_lHeaderRow, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_TOP, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	// Freeze row header.
	SetFreeze( 0, m_lHeaderRow );
}

int CSSheetLDSumm::FillRow( long lRow, CLog *pLD )
{
	if( NULL == pLD )
		return 0;
	
	long lLine = 2;
	CString str1, str2;

	// Write the name of the logged data.
	SetStaticText( ColumnDescription::CD_Name, lRow, pLD->GetName() );
	
	// Write starting time.
	CTimeUnic dtuBeg;
	pLD->GetDateTime( 0, dtuBeg );
	CString strDT = dtuBeg.Format( IDS_LOGDATETIME_FORMAT );
	SetStaticText( ColumnDescription::CD_Time, lRow, strDT );

	// Write duration.
	CTimeUnic dtuEnd;
	pLD->GetDateTime( pLD->GetLength() - 1, dtuEnd );
	CTimeSpan ts = dtuEnd - dtuBeg;  // Subtract 2 CTimes
	strDT.Empty(); 
	
	if( ts.GetDays() )
	{
		str2 = TASApp.LoadLocalizedString( IDS_DAYS );
	    strDT += _T("%D") + str2 + _T(" ");
	}
	
	if( ts.GetHours() )
	{
		str2 = TASApp.LoadLocalizedString( IDS_HOUR );
	    strDT += _T("%H") + str2 + _T(" ");
	}

	if( ts.GetMinutes() )
	{
		str2 = TASApp.LoadLocalizedString( IDS_MINUTE );
	    strDT += _T("%M") + str2 + _T(" ");
	}

	str2 = TASApp.LoadLocalizedString( IDS_SEC );
    strDT += _T("%S") + str2;
	SetStaticText( ColumnDescription::CD_Duration, lRow, ts.Format( strDT ) );

	SetStaticText( ColumnDescription::CD_Plant, lRow, pLD->GetSite() );
	SetStaticText( ColumnDescription::CD_Module, lRow, pLD->GetRef() );
	str1.Format( _T("%d"), pLD->GetValveIndex() );
	SetStaticText( ColumnDescription::CD_ValveIndex, lRow, str1 );

	// Write logging type.
	unsigned char ucMode = pLD->GetMode();
	switch( ucMode )
	{
		case CLog::LOGTYPE_DP:
		case CLog::LOGTYPE_DPTEMP:
			str1 = TASApp.LoadLocalizedString( IDS_PRESSUREDROP );
			break;

		case CLog::LOGTYPE_FLOW:
		case CLog::LOGTYPE_FLOWTEMP:
			str1 = TASApp.LoadLocalizedString( IDS_FLOW );
			break;

		case CLog::LOGTYPE_TEMP:
			str1 = TASApp.LoadLocalizedString( IDS_TEMPERATURE );
			break;

		case CLog::LOGTYPE_DT:
			str1 = TASApp.LoadLocalizedString( IDS_DIFFTEMPERATURE );
			break;

		case CLog::LOGTYPE_POWER:
			str1 = TASApp.LoadLocalizedString( IDS_POWER );
			break;
	}
	
	if( CLog::LOGTYPE_DPTEMP == ucMode || CLog::LOGTYPE_FLOWTEMP == ucMode )
	{
		str1 += _T(" / ");
		str2 = TASApp.LoadLocalizedString( IDS_TEMPERATURE );
		str1 += str2;
	}
	SetStaticText( ColumnDescription::CD_LogType, lRow, str1 );

	return lLine;
}

void CSSheetLDSumm::SetColWidth( long lColumn, double dSize )
{
	ASSERT( lColumn <= m_lColn );
	
	m_ardColWidth[lColumn] = dSize;
	CSSheet::SetColWidth( lColumn, dSize );
}
