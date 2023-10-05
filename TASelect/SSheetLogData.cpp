//
// SSheetLogData.cpp: implementation of the CSSheetLogData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TASelect.h"
#include "Utilities.h"

#include "SSheetLogData.h"
#include "DrawSet.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSSheetLogData::CSSheetLogData()
{
	m_dRowHeight = 12.75;
	m_dPageWidth = 0.0;
}

CSSheetLogData::~CSSheetLogData()
{
}

//////////////////////////////////////////////////////////////////////
// CSSheetLogData member functions

BOOL CSSheetLogData::Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID )
{
	if( !CSSheet::Create( dwStyle, rect, pParentWnd, nID ) ) 
		return FALSE;

	CSSheet::Init();
	
	// Set up main parameters not yet set in base class CSSheet
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
	SetBool( SSB_ALLOWDRAGDROP, FALSE );
	
	return TRUE;
}

void CSSheetLogData::FormatHeader( CLog *pLD, bool fPrint )
{
	// Variables
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();				ASSERT( pUnitDB );

	SetRowHeight( RD_Header_BeforeTitle, m_dRowHeight * 0.5 );				// First Empty line
	SetRowHeight( RD_Header_Title, m_dRowHeight * 2 );						// Title
	SetRowHeight( RD_Header_AfterTitle, m_dRowHeight * 0.5 );				// Empty line
	SetRowHeight( RD_Header_Plant, m_dRowHeight * 0.7 );					// Plant for Summary
	SetRowHeight( RD_Header_Module, m_dRowHeight * 0.7 );					// Module for Summary
	SetRowHeight( RD_Header_ValveIndex, m_dRowHeight * 0.7 );				// Valve index for Summary
	SetRowHeight( RD_Header_Valve, m_dRowHeight * 0.7 );					// Valve for Summary
	SetRowHeight( RD_Header_ValveName, m_dRowHeight * 0.7 );				// Valve name for Summary
	SetRowHeight( RD_Header_LogType, m_dRowHeight * 0.9 );					// Logging type for Summary 0.9 to give some more space below
	SetRowHeight( RD_Header_EmptyLine, m_dRowHeight * 0.5 );				// Empty line

	// Format Title
	SetTextPattern( CSSheet::_SSTextPattern::TitlePage );
	
	// Set the text in the Title
	CString str = pLD->GetName();
	str += ( _T(": ") + TASApp.LoadLocalizedString( IDS_DATALIST) );
	AddCellSpanW( CD_Data_Index, RD_Header_Title, CD_Data_Last - CD_Data_Index, 1 );
	SetStaticText( CD_Data_Index, RD_Header_Title, str );
}

void CSSheetLogData::FormatSummaryHeader( CLog *pLD )
{
	// Variables
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	CString str, str1;
	CString stmp;
	double dVal;	

	// Modify TextPattern
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Find the column cell that is closed to the middle of the sheet
 	long lCell14 = FindColumnCell( 0.30 );
 	long lCellMiddle = FindColumnCell( 0.62 );
 	long lCell34 = FindColumnCell( 0.85 );

	// Title Plant
	str = TASApp.LoadLocalizedString( IDS_PLANT );
	str += _T(":");
	SetStaticText( CD_Data_Index, RD_Header_Plant, str );
	AddCellSpanW( CD_Data_Index, RD_Header_Plant, lCell14 - CD_Data_Index, 1 );

	// Title Module
	str = TASApp.LoadLocalizedString( IDS_MODULE );
	str += _T(":");
	SetStaticText( CD_Data_Index, RD_Header_Module, str );
	AddCellSpanW( CD_Data_Index, RD_Header_Module, lCell14 - CD_Data_Index, 1 );

	// Title Valve Index
	str = TASApp.LoadLocalizedString( IDS_VALVEINDEX );
	str += _T(":");
 	SetStaticText( CD_Data_Index, RD_Header_ValveIndex, str );
 	AddCellSpanW( CD_Data_Index, RD_Header_ValveIndex, lCell14 - CD_Data_Index, 1 );

	// Title Valve
	str = TASApp.LoadLocalizedString( IDS_VALVE );
	str += _T(":");
 	SetStaticText( CD_Data_Index, RD_Header_Valve, str );
 	AddCellSpanW( CD_Data_Index, RD_Header_Valve, lCell14 - CD_Data_Index, 1 );

	// Title Valve name
	//str = TASApp.LoadLocalizedString( IDS_VALVENAME );
	//str += _T(":");
 //	SetStaticText( CD_Data_Index, RD_Header_ValveName, str );
 //	AddCellSpanW( CD_Data_Index, RD_Header_ValveName, lCell14 - CD_Data_Index, 1 );

	// Title Logging type
	str = TASApp.LoadLocalizedString( IDS_LOGTYPE );
	str += _T(":");
 	SetStaticText( CD_Data_Index, RD_Header_LogType, str );
 	AddCellSpanW( CD_Data_Index, RD_Header_LogType, lCell14 - CD_Data_Index, 1 );

	// Title Starting time
	str = TASApp.LoadLocalizedString( IDS_STARTINGTIME );
	str += _T(":");
 	SetStaticText( lCellMiddle, RD_Header_StartingTime, str );
 	AddCellSpanW( lCellMiddle, RD_Header_StartingTime, lCell34 - lCellMiddle, 1 );

	// Title Ending time
	str = TASApp.LoadLocalizedString( IDS_ENDINGTIME );
	str += _T(":");
 	SetStaticText( lCellMiddle, RD_Header_EndingTime, str );
 	AddCellSpanW( lCellMiddle, RD_Header_EndingTime, lCell34 - lCellMiddle, 1 );

	// Title Nbr. of points
	str = TASApp.LoadLocalizedString( IDS_NBROFPOINTS );
	str += _T(":");
 	SetStaticText( lCellMiddle, RD_Header_PointsNumber, str );
 	AddCellSpanW( lCellMiddle, RD_Header_PointsNumber, lCell34 - lCellMiddle, 1 );

	// Title DT between points
	str = TASApp.LoadLocalizedString( IDS_DTBETWPTS );
	str += _T(":");
 	SetStaticText( lCellMiddle, RD_Header_DtBetweenPoints, str );
 	AddCellSpanW( lCellMiddle, RD_Header_DtBetweenPoints, lCell34 - lCellMiddle, 1 );

	// Title Lowest sampled value
	str = TASApp.LoadLocalizedString( IDS_LOWSAMPLEDVAL );
	str += _T(":");
 	SetStaticText( lCellMiddle, RD_Header_LowestSampleValue, str );
 	AddCellSpanW( lCellMiddle, RD_Header_LowestSampleValue, lCell34 - lCellMiddle, 1 );

	// Title Highest sampled value
	str = TASApp.LoadLocalizedString( IDS_HIGHSAMPLEDVAL );
	str += _T(":");
 	SetStaticText( lCellMiddle, RD_Header_HighestSampleValue, str );
 	AddCellSpanW( lCellMiddle, RD_Header_HighestSampleValue, lCell34 - lCellMiddle, 1 );

	// Modify TextPattern
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Info Plant
	str = pLD->GetSite();
	SetStaticText( lCell14, RD_Header_Plant, str );
	AddCellSpanW( lCell14, RD_Header_Plant, lCellMiddle - lCell14, 1 );

	// Info Module
	str = pLD->GetRef();
 	SetStaticText( lCell14, RD_Header_Module, str );
 	AddCellSpanW( lCell14, RD_Header_Module, lCellMiddle - lCell14, 1 );

	// Info Valve index
	str.Format( _T("%d"), pLD->GetValveIndex() );
 	SetStaticText( lCell14, RD_Header_ValveIndex, str );
 	AddCellSpanW( lCell14, RD_Header_ValveIndex, lCellMiddle - lCell14, 1 );

	// Info Valve
	str = pLD->GetValveName();
 	SetStaticText( lCell14, RD_Header_Valve, str );
 	AddCellSpanW( lCell14, RD_Header_Valve, lCellMiddle - lCell14, 1 );

	// Info Valve name
	//str = pLD->GetValveName();
 //	SetStaticText( lCell14, RD_Header_ValveName, str );
 //	AddCellSpanW( lCell14, RD_Header_ValveName, lCellMiddle - lCell14, 1 );

	// Info Logging type
	unsigned char ucMode = pLD->GetMode();
	switch( ucMode )
	{
		case CLog::LOGTYPE_DP:
		case CLog::LOGTYPE_DPTEMP:
			str = TASApp.LoadLocalizedString( IDS_PRESSUREDROP );
			break;

		case CLog::LOGTYPE_FLOW:
		case CLog::LOGTYPE_FLOWTEMP:
			str = TASApp.LoadLocalizedString( IDS_FLOW );
			break;

		case CLog::LOGTYPE_TEMP:
			str = TASApp.LoadLocalizedString( IDS_TEMPERATURE );
			break;

		case CLog::LOGTYPE_DT:
			str = TASApp.LoadLocalizedString( IDS_CHART_DIFFTEMP );
			break;

		case CLog::LOGTYPE_POWER:
			str = TASApp.LoadLocalizedString( IDS_POWER );
			break;
	}

	if( CLog::LOGTYPE_DPTEMP == ucMode || CLog::LOGTYPE_FLOWTEMP == ucMode )
	{
		str += _T(" / ");
		str1 = TASApp.LoadLocalizedString( IDS_TEMPERATURE );
		str += str1;
	}
	
 	SetStaticText( lCell14, RD_Header_LogType, str );
 	AddCellSpanW( lCell14, RD_Header_LogType, lCellMiddle - lCell14, 1 );	
	
	// Info Starting time
	CTimeUnic dtu;
	pLD->GetDateTime( 0, dtu );
	str = dtu.Format( IDS_LOGDATETIME_FORMAT );
 	SetStaticText( lCell34, RD_Header_StartingTime, str );
 	AddCellSpanW( lCell34, RD_Header_StartingTime, CD_Data_Last - lCell34, 1 );

	// Info Ending time
	pLD->GetDateTime( pLD->GetLength() - 1, dtu ); 
	str = dtu.Format( IDS_LOGDATETIME_FORMAT );
 	SetStaticText( lCell34, RD_Header_EndingTime, str );
 	AddCellSpanW( lCell34, RD_Header_EndingTime, CD_Data_Last - lCell34, 1 );

	// Info Nbr. of points
	str.Format( _T("%d"), pLD->GetLength() );
 	SetStaticText( lCell34, RD_Header_PointsNumber, str );
 	AddCellSpanW( lCell34, RD_Header_PointsNumber, CD_Data_Last - lCell34, 1 );

	// Info DT between points
	str.Format( _T("%d"), pLD->GetInterval() );
	stmp = TASApp.LoadLocalizedString( IDS_SEC );
	str += _T(" ") + stmp;
 	SetStaticText( lCell34, RD_Header_DtBetweenPoints, str );
 	AddCellSpanW( lCell34, RD_Header_DtBetweenPoints, CD_Data_Last - lCell34, 1 );

	// Info Lowest sampled value
	switch( pLD->GetMode() )
	{
		// Pressure drop
		case CLog::LOGTYPE_DP:
			if( false == pLD->GetLowestValue( CLog::ect_Dp, dVal ) )
				dVal = 0;
			str = WriteCUDouble( _U_DIFFPRESS, dVal, true );
			break;
		
		// Flow
		case CLog::LOGTYPE_FLOW:
			if( false == pLD->GetLowestValue( CLog::ect_Flow, dVal ) )
				dVal = 0;
			str = WriteCUDouble( _U_FLOW, dVal, true );
			str += _T("/");

			if( false == pLD->GetLowestValue( CLog::ect_Dp, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_DIFFPRESS, dVal, true );
			break;

		// Temperature
		case CLog::LOGTYPE_TEMP:
			if( true == pLD->IsT1SensorUsed() )
			{
				// T1
				if( false == pLD->GetLowestValue( CLog::ect_Temp1, dVal ) )
					dVal = 0; 
			}
			else
			{
				// T2
				if( false == pLD->GetLowestValue( CLog::ect_Temp2, dVal ) )
					dVal = 0;
			}
			str = WriteCUDouble( _U_TEMPERATURE, dVal, true );
			break;
		
		// Pressure drop and temperature
		case CLog::LOGTYPE_DPTEMP:
			if( false == pLD->GetLowestValue( CLog::ect_Dp, dVal ) )
				dVal = 0;
			str = WriteCUDouble( _U_DIFFPRESS, dVal, true );
			str += _T("/");
			
			if( true == pLD->IsT1SensorUsed() )
			{
				// T1
				if( false == pLD->GetLowestValue( CLog::ect_Temp1, dVal ) )
					dVal = 0;
			}
			else
			{
				// T2
				if( false == pLD->GetLowestValue( CLog::ect_Temp2, dVal ) )
					dVal = 0;
			}
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			break;
		
		// Flow and temperature
		case CLog::LOGTYPE_FLOWTEMP:
			if( false == pLD->GetLowestValue( CLog::ect_Flow, dVal ) )
				dVal = 0;
			str = WriteCUDouble( _U_FLOW, dVal, true );
			str += _T("/");

			if( true == pLD->IsT1SensorUsed() )
			{
				// T1
				if( false == pLD->GetLowestValue( CLog::ect_Temp1, dVal ) )
					dVal = 0;
			}
			else
			{
				// T2
				if( false == pLD->GetLowestValue( CLog::ect_Temp2, dVal ) )
					dVal = 0;
			}
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			str += _T("/");

			if( false == pLD->GetLowestValue( CLog::ect_Dp, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_DIFFPRESS, dVal, true );
			break;
		
		// DT
		case CLog::LOGTYPE_DT:
			if( false == pLD->GetLowestValue( CLog::ect_DeltaT, dVal ) )
				dVal = 0;
			str = WriteCUDouble( _U_DIFFTEMP, dVal, true );
			str += _T("/");

			if( false == pLD->GetLowestValue( CLog::ect_Temp1, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			str += _T("/");
			
			if( false == pLD->GetLowestValue( CLog::ect_Temp2, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			break;
		
		// Power
		case CLog::LOGTYPE_POWER:
			if( false == pLD->GetLowestValue( CLog::ect_Power, dVal ) )
				dVal = 0;
			str = WriteCUDouble( _U_TH_POWER, dVal, true );
			str += _T("/");

			if( false == pLD->GetLowestValue( CLog::ect_Flow, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_FLOW, dVal, true );
			str += _T("/");

			if( false == pLD->GetLowestValue( CLog::ect_Dp, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_DIFFPRESS, dVal, true );
			str += _T("/");

			if( false == pLD->GetLowestValue( CLog::ect_DeltaT, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_DIFFTEMP, dVal, true );
			str += _T("/");

			if( false == pLD->GetLowestValue( CLog::ect_Temp1, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			str += _T("/");

			if( false == pLD->GetLowestValue( CLog::ect_Temp2, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			break;
	}
	
 	SetStaticText( lCell34, RD_Header_LowestSampleValue, str );
 	AddCellSpanW( lCell34, RD_Header_LowestSampleValue, CD_Data_Last - lCell34, 1 );

	// Info highest sampled value
	switch (pLD->GetMode())
	{
		// Pressure drop
		case CLog::LOGTYPE_DP:
			if( false == pLD->GetHighestValue( CLog::ect_Dp, dVal ) )
				dVal = 0;
			str = WriteCUDouble( _U_DIFFPRESS, dVal, true );
			break;

		// Flow
		case CLog::LOGTYPE_FLOW:
			if( false == pLD->GetHighestValue( CLog::ect_Flow, dVal ) )
				dVal = 0;
			str = WriteCUDouble( _U_FLOW, dVal, true );
			str += _T("/");

			if( false == pLD->GetHighestValue( CLog::ect_Dp, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_DIFFPRESS, dVal, true );
			break;
		
		// Temperature
		case CLog::LOGTYPE_TEMP:
			if( true == pLD->IsT1SensorUsed() )
			{
				// T1
				if( false == pLD->GetHighestValue( CLog::ect_Temp1, dVal ) )
					dVal = 0;
			}
			else
			{
				// T2
				if( false == pLD->GetHighestValue( CLog::ect_Temp2, dVal ) )
					dVal = 0;
			}
			str = WriteCUDouble( _U_TEMPERATURE, dVal, true );
			break;
		
		// Pressure drop and temperature
		case CLog::LOGTYPE_DPTEMP:
			if( false == pLD->GetHighestValue( CLog::ect_Dp, dVal ) )
				dVal = 0;
			str = WriteCUDouble( _U_DIFFPRESS, dVal, true );
			str += _T("/");

			if( true == pLD->IsT1SensorUsed() )
			{
				// T1
				if( false == pLD->GetHighestValue( CLog::ect_Temp1, dVal ) )
					dVal = 0;
			}
			else
			{
				// T2
				if( false == pLD->GetHighestValue( CLog::ect_Temp2, dVal ) )
					dVal = 0;
			}
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			break;
		
		// Flow and temperature
		case CLog::LOGTYPE_FLOWTEMP:
			if( false == pLD->GetHighestValue( CLog::ect_Flow, dVal ) )
				dVal = 0;
			str = WriteCUDouble( _U_FLOW, dVal, true );
			str += _T("/");

			if( true == pLD->IsT1SensorUsed() )
			{
				// T1
				if( false == pLD->GetHighestValue( CLog::ect_Temp1, dVal ) )
					dVal = 0;
			}
			else
			{
				// T2
				if( false == pLD->GetHighestValue( CLog::ect_Temp2, dVal ) )
					dVal = 0;
			}
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			str += _T("/");

			if( false == pLD->GetHighestValue( CLog::ect_Dp, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_DIFFPRESS, dVal, true );
			break;
		
		// DT
		case CLog::LOGTYPE_DT:
			if( false == pLD->GetHighestValue( CLog::ect_DeltaT, dVal ) )
				dVal = 0;
			str = WriteCUDouble( _U_DIFFTEMP, dVal, true );
			str += _T("/");

			if( false == pLD->GetHighestValue( CLog::ect_Temp1, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			str += _T("/");

			if( false == pLD->GetHighestValue( CLog::ect_Temp2, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			break;
		
		// Power
		case CLog::LOGTYPE_POWER:
			if( false == pLD->GetHighestValue( CLog::ect_Power, dVal ) )
				dVal = 0;
			str = WriteCUDouble( _U_TH_POWER, dVal, true );
			str += _T("/");

			if( false == pLD->GetHighestValue( CLog::ect_Flow, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_FLOW, dVal, true );
			str += _T("/");

			if( false == pLD->GetHighestValue( CLog::ect_Dp, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_DIFFPRESS, dVal, true );
			str += _T("/");

			if( false == pLD->GetHighestValue( CLog::ect_DeltaT, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_DIFFTEMP, dVal, true );
			str += _T("/");

			if( false == pLD->GetHighestValue( CLog::ect_Temp1, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			str += _T("/");

			if( false == pLD->GetHighestValue( CLog::ect_Temp2, dVal ) )
				dVal = 0;
			str += WriteCUDouble( _U_TEMPERATURE, dVal, true );
			break;
	}
	
 	SetStaticText( lCell34, RD_Header_HighestSampleValue, str );
 	AddCellSpanW( lCell34, RD_Header_HighestSampleValue, CD_Data_Last - lCell34, 1 );

	// Draw border
	SetCellBorder( CD_Data_Index, RD_Header_LogType, CD_Data_Last - 1, RD_Header_LogType, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
}

void CSSheetLogData::InitDataColumnWidth( CLog *pLD, bool fPrint, double dFactor )
{
	SetColWidth( CD_Data_None, 2 * dFactor );
	SetColWidth( CD_Data_Index, 12 * dFactor );
	SetColWidth( CD_Data_Date, 12 * dFactor );
	SetColWidth( CD_Data_Time, 13 * dFactor );
	SetColWidth( CD_Data_Power, 13 * dFactor );
	SetColWidth( CD_Data_Flow, 13 * dFactor );
	SetColWidth( CD_Data_Dp, 13 * dFactor );
	SetColWidth( CD_Data_DT, 13 * dFactor );
	SetColWidth( CD_Data_Temp1, 13 * dFactor );
	SetColWidth( CD_Data_Temp2, 13 * dFactor );
	SetColWidth( CD_Data_Last, 2 * dFactor );
	ShowCol( CD_Data_Last, FALSE );
	
	// In Case it is a print, do not print the first white column.
	// Resize the rows with data to put more data on a single sheet.
	if( true == fPrint )
		ShowCol( CD_Data_None, FALSE );

	// Hide unnecessary column depending on the logging mode
	switch( pLD->GetMode() )
	{
		case CLog::LOGTYPE_DP:
			ShowCol( CD_Data_Power, FALSE );
			ShowCol( CD_Data_Flow, FALSE );
			ShowCol( CD_Data_DT, FALSE );
			ShowCol( CD_Data_Temp1, FALSE );
			ShowCol( CD_Data_Temp2, FALSE );
			break;
		
		case CLog::LOGTYPE_FLOW:
			ShowCol( CD_Data_Power, FALSE );
			if( CLog::TOOL_TASCOPE != pLD->ToolUsedForMeasure() )
				ShowCol( CD_Data_Dp, FALSE );
			ShowCol( CD_Data_DT, FALSE );
			ShowCol( CD_Data_Temp1, FALSE );
			ShowCol( CD_Data_Temp2, FALSE );
			break;
		
		case CLog::LOGTYPE_TEMP:
			ShowCol( CD_Data_Power, FALSE );
			ShowCol( CD_Data_Flow, FALSE );
			ShowCol( CD_Data_Dp, FALSE );
			ShowCol( CD_Data_DT, FALSE );

			if( CLog::TOOL_TASCOPE == pLD->ToolUsedForMeasure() )
			{
				if( true == pLD->IsT1SensorUsed() )
					ShowCol( CD_Data_Temp2, FALSE );
				else
					ShowCol( CD_Data_Temp1, FALSE );
			}
			else
				ShowCol( CD_Data_Temp2, FALSE );
			break;
		
		case CLog::LOGTYPE_DPTEMP:
			ShowCol( CD_Data_Power, FALSE );
			ShowCol( CD_Data_Flow, FALSE );
			ShowCol( CD_Data_DT, FALSE );

			if( CLog::TOOL_TASCOPE == pLD->ToolUsedForMeasure() )
			{
				if( true == pLD->IsT1SensorUsed() )
					ShowCol( CD_Data_Temp2, FALSE );
				else
					ShowCol( CD_Data_Temp1, FALSE );
			}
			else
				ShowCol( CD_Data_Temp2, FALSE );
			break;

		case CLog::LOGTYPE_FLOWTEMP:
			ShowCol( CD_Data_Power, FALSE );
			if( CLog::TOOL_TASCOPE != pLD->ToolUsedForMeasure() )
				ShowCol( CD_Data_Dp, FALSE );
			ShowCol( CD_Data_DT, FALSE );
			if( CLog::TOOL_TASCOPE == pLD->ToolUsedForMeasure() )
			{
				if( true == pLD->IsT1SensorUsed() )
					ShowCol( CD_Data_Temp2, FALSE );
				else
					ShowCol( CD_Data_Temp1, FALSE );
			}
			else
				ShowCol( CD_Data_Temp2, FALSE );
			break;
		
		case CLog::LOGTYPE_DT:
			ShowCol( CD_Data_Power, FALSE );
			ShowCol( CD_Data_Flow, FALSE );
			ShowCol( CD_Data_Dp, FALSE );
			break;

		case CLog::LOGTYPE_POWER:
			break;
	}
}

void CSSheetLogData::FormatData( CLog *pLD, bool fPrint )
{
	// Format the column headers
	SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_GRAY_MED );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)( SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER ) );

	// Set the text of column headers
	TCHAR unitname[_MAXCHARS];
	
	long lDataTitleRow = RD_Header_LastLine + 1;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();				ASSERT( pUnitDB );

	CString str = TASApp.LoadLocalizedString( IDS_INDEX );
	AddCellSpanW( CD_Data_Index, lDataTitleRow, 1, 2 );
	SetStaticText( CD_Data_Index, lDataTitleRow, str );


	str = TASApp.LoadLocalizedString( IDS_DATE );
	AddCellSpanW( CD_Data_Date, lDataTitleRow, 1, 2 );
	SetStaticText( CD_Data_Date, lDataTitleRow, str );

	str = TASApp.LoadLocalizedString( IDS_TIME );
	AddCellSpanW( CD_Data_Time, lDataTitleRow, 1, 2 );
	SetStaticText( CD_Data_Time, lDataTitleRow, str );

	// Power
	if( FALSE == IsColHidden( CD_Data_Power ) )
	{
		// Name
		str = TASApp.LoadLocalizedString( IDS_POWER );
		AddCellSpanW( CD_Data_Power, lDataTitleRow, CD_Data_Flow - CD_Data_Power, 1 );
		SetStaticText( CD_Data_Power, lDataTitleRow, str );
	
		// Unit
		GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ), unitname );
		str = (CString)unitname;
		AddCellSpanW( CD_Data_Power, lDataTitleRow + 1, CD_Data_Flow - CD_Data_Power, 1 );
		SetStaticText( CD_Data_Power, lDataTitleRow + 1, str );
	}

	// Flow
	if( FALSE == IsColHidden( CD_Data_Flow ) )
	{
		// Name
		str = TASApp.LoadLocalizedString( IDS_FLOW );
		AddCellSpanW( CD_Data_Flow, lDataTitleRow, CD_Data_Dp - CD_Data_Flow, 1 );
		SetStaticText( CD_Data_Flow, lDataTitleRow, str );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), unitname );
		
		// Unit
		str = (CString)unitname;
		AddCellSpanW( CD_Data_Flow, lDataTitleRow + 1, CD_Data_Dp - CD_Data_Flow, 1 );
		SetStaticText( CD_Data_Flow, lDataTitleRow + 1, str );
	}

	// Dp
	if( FALSE == IsColHidden( CD_Data_Dp ) )
	{
		// Name
		str = TASApp.LoadLocalizedString( IDS_PRESSUREDROP );
		AddCellSpanW( CD_Data_Dp, lDataTitleRow, CD_Data_DT - CD_Data_Dp, 1 );
		SetStaticText( CD_Data_Dp, lDataTitleRow, str );
		
		// Unit
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
		str = (CString)unitname;
		AddCellSpanW( CD_Data_Dp, lDataTitleRow + 1, CD_Data_DT - CD_Data_Dp, 1 );
		SetStaticText( CD_Data_Dp, lDataTitleRow + 1, str );
	}

	// Delta T
	if( FALSE == IsColHidden( CD_Data_DT ) )
	{
		// Name
		str = TASApp.LoadLocalizedString( IDS_DIFFTEMPERATURE );
		AddCellSpanW( CD_Data_DT, lDataTitleRow, CD_Data_Temp1 - CD_Data_DT, 1 );
		SetStaticText( CD_Data_DT, lDataTitleRow, str );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFTEMP ), unitname );
	
		// Unit
		str = (CString)unitname;
		AddCellSpanW( CD_Data_DT, lDataTitleRow + 1, CD_Data_Temp1 - CD_Data_DT, 1 );
		SetStaticText( CD_Data_DT, lDataTitleRow + 1, str );
	}

	// Temperature sensor 1
	if( FALSE == IsColHidden( CD_Data_Temp1 ) )
	{
		// Name
		// If TASCOPE...
		if( CLog::TOOL_TASCOPE == pLD->ToolUsedForMeasure() )
			str = TASApp.LoadLocalizedString( IDS_CHART_T1 );
		else
			str = TASApp.LoadLocalizedString( IDS_TEMPERATURE );
		AddCellSpanW( CD_Data_Temp1, lDataTitleRow, CD_Data_Temp2 - CD_Data_Temp1, 1 );
		SetStaticText( CD_Data_Temp1, lDataTitleRow, str );
	
		// Unit
		GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), unitname );
		str = (CString)unitname;
		AddCellSpanW( CD_Data_Temp1, lDataTitleRow + 1, CD_Data_Temp2 - CD_Data_Temp1, 1 );
		SetStaticText( CD_Data_Temp1, lDataTitleRow + 1, str );
	}

	// Temperature sensor 2
	if( FALSE == IsColHidden( CD_Data_Temp2 ) )
	{
		// Name
		str = TASApp.LoadLocalizedString( IDS_CHART_T2 );
		AddCellSpanW( CD_Data_Temp2, lDataTitleRow, CD_Data_Last - CD_Data_Temp2, 1 );
		SetStaticText( CD_Data_Temp2, lDataTitleRow, str );
	
		// Unit
		GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), unitname );
		str = (CString)unitname;
		AddCellSpanW( CD_Data_Temp2, lDataTitleRow + 1, CD_Data_Last - CD_Data_Temp2, 1 );
		SetStaticText( CD_Data_Temp2, lDataTitleRow + 1, str );
	}

	// Draw border
	SetCellBorder(	CD_Data_Index, lDataTitleRow + 1, 
					CD_Data_Last - 1, lDataTitleRow + 1, 
					true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	// Freeze Row Header
	SetFreeze( 0, lDataTitleRow + 1 );
}

long CSSheetLogData::FindColumnCell( double dPercentage )
{
	// Verify the value enter match
	if (dPercentage < 0.0 || dPercentage > 1.0)
	{
		ASSERT(0);
		return 0;		
	}

	// Variables
	double dTotalWidth = 0.0;
	double dWantedWidth = 0.0;
	double dVal = 0.0;
	long lReturn = 0;
	
	// Get the total width
	for( long lLoopColumn = CD_Data_None + 1; lLoopColumn < CD_Data_Last; lLoopColumn++ )
		dTotalWidth += GetColWidthW( lLoopColumn );

	// Set the wanted width
	dWantedWidth = dTotalWidth * dPercentage;

	// Find the closed cell to the middle
	for( long lLoopColumn = CD_Data_None + 1; lLoopColumn < CD_Data_Last; lLoopColumn++ )
	{
		dVal += GetColWidthW( lLoopColumn );
		if( dVal > dWantedWidth - 1 )
		{
			// Add an offset of 1
			// To let the column resize correctly
			lReturn = lLoopColumn;
			break;
		}
	}
	
	return lReturn;
}

long CSSheetLogData::FillRow( long lRow, CLog *pLD, int iIndex )
{
	if( false == pLD )
		return 0;
	
	long lLine = 1;
	CString str;

	// Write Dp, flow and temperature
	double dValue;
	
	switch( pLD->GetMode() )
	{
		case CLog::LOGTYPE_DP:
			// Dp
			( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Dp, dValue ) : pLD->GetNext( CLog::ect_Dp, dValue );
			str = (CString)WriteCUDouble( _U_DIFFPRESS, dValue );
			SetCellText( CD_Data_Dp, lRow, str);
			break;
		
		case CLog::LOGTYPE_FLOW:
			// Flow
			( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Flow, dValue ) : pLD->GetNext( CLog::ect_Flow, dValue );
			str = (CString)WriteCUDouble( _U_FLOW, dValue );
			SetCellText( CD_Data_Flow, lRow, str );

			// Dp in case of TAScope instrument
			if( CLog::TOOL_TASCOPE == pLD->ToolUsedForMeasure() )
			{
				( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Dp, dValue ) : pLD->GetNext( CLog::ect_Dp, dValue );
				str = (CString)WriteCUDouble( _U_DIFFPRESS, dValue );
				SetCellText( CD_Data_Dp, lRow, str );
			}
			break;

		case CLog::LOGTYPE_TEMP:
			// Temperature
			if( CLog::TOOL_TASCOPE == pLD->ToolUsedForMeasure() )
			{
				// TAScope has two temperature sensor available.

				if( true == pLD->IsT1SensorUsed() )
				{
					// Case of probe 1 used.
					( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Temp1, dValue ) : pLD->GetNext( CLog::ect_Temp1, dValue );
					str = (CString)WriteCUDouble( _U_TEMPERATURE, dValue );
					SetCellText( CD_Data_Temp1, lRow, str );
				}
				else
				{
					// Case of probe 2 used.
					( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Temp2, dValue ) : pLD->GetNext( CLog::ect_Temp2, dValue );
					str = (CString)WriteCUDouble( _U_TEMPERATURE, dValue );
					SetCellText( CD_Data_Temp2, lRow, str );
				}
			}
			else
			{
				// If from CBI, only one temperature sensor available
				( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Temp1, dValue ) : pLD->GetNext( CLog::ect_Temp1, dValue );
				str = (CString)WriteCUDouble( _U_TEMPERATURE, dValue );
				SetCellText( CD_Data_Temp1, lRow, str );
			}
			break;
		
		case CLog::LOGTYPE_DPTEMP:
			// Dp
			( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Dp, dValue ) : pLD->GetNext( CLog::ect_Dp, dValue );
			str = (CString)WriteCUDouble( _U_DIFFPRESS, dValue );
			SetCellText( CD_Data_Dp, lRow, str );
			
			// Temperature
			if( CLog::TOOL_TASCOPE == pLD->ToolUsedForMeasure() )
			{
				// TAScope has two temperature sensors available.

				if( true == pLD->IsT1SensorUsed() )
				{
					// Case of probe 1 used.
					( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Temp1, dValue ) : pLD->GetNext( CLog::ect_Temp1, dValue );
					str = (CString)WriteCUDouble( _U_TEMPERATURE, dValue );
					SetCellText( CD_Data_Temp1, lRow, str );
				}
				else
				{
					// Case of probe 2 used.
					( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Temp2, dValue ) : pLD->GetNext( CLog::ect_Temp2, dValue );
					str = (CString)WriteCUDouble( _U_TEMPERATURE, dValue );
					SetCellText( CD_Data_Temp2, lRow, str );
				}
			}
			else
			{
				// If from CBI, only one temperature sensors available
				( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Temp1, dValue ) : pLD->GetNext( CLog::ect_Temp1, dValue );
				str = (CString)WriteCUDouble( _U_TEMPERATURE, dValue );
				SetCellText( CD_Data_Temp1, lRow, str );
			}
			break;

		case CLog::LOGTYPE_FLOWTEMP:
			// Flow
			( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Flow, dValue ) : pLD->GetNext( CLog::ect_Flow, dValue );
			str = (CString)WriteCUDouble( _U_FLOW, dValue );
			SetCellText( CD_Data_Flow, lRow, str );

			// Dp in case of TAScope instrument
			if( CLog::TOOL_TASCOPE == pLD->ToolUsedForMeasure() )
			{
				( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Dp, dValue ) : pLD->GetNext( CLog::ect_Dp, dValue );
				str = (CString)WriteCUDouble( _U_DIFFPRESS, dValue );
				SetCellText( CD_Data_Dp, lRow, str );
			}

			// Temperature
			if( CLog::TOOL_TASCOPE == pLD->ToolUsedForMeasure() )
			{
				// TAScope has two temperature sensors available.

				if( true == pLD->IsT1SensorUsed() )
				{
					// Case of probe 1 used.
					( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Temp1, dValue ) : pLD->GetNext( CLog::ect_Temp1, dValue );
					str = (CString)WriteCUDouble( _U_TEMPERATURE, dValue );
					SetCellText( CD_Data_Temp1, lRow, str );
				}
				else
				{
					// Case of probe 2 used.
					( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Temp2, dValue ) : pLD->GetNext( CLog::ect_Temp2, dValue );
					str = (CString)WriteCUDouble( _U_TEMPERATURE, dValue );
					SetCellText( CD_Data_Temp2, lRow, str );
				}
			}
			else
			{
				// If from CBI, only one temperature sensors available
				( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Temp1, dValue ) : pLD->GetNext( CLog::ect_Temp1, dValue );
				str = (CString)WriteCUDouble( _U_TEMPERATURE, dValue );
				SetCellText( CD_Data_Temp1, lRow, str );
			}
			break;
		
		case CLog::LOGTYPE_DT:
			// Delta T
			( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_DeltaT, dValue ) : pLD->GetNext( CLog::ect_DeltaT, dValue );
			str = (CString)WriteCUDouble( _U_DIFFTEMP, dValue );
			SetCellText( CD_Data_DT, lRow, str );
			
			// Temperature 1
			( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Temp1, dValue ) : pLD->GetNext( CLog::ect_Temp1, dValue );
			str = (CString)WriteCUDouble( _U_TEMPERATURE, dValue );
			SetCellText( CD_Data_Temp1, lRow, str );

			// Temperature 2
			( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Temp2, dValue ) : pLD->GetNext( CLog::ect_Temp2, dValue );
			str = (CString)WriteCUDouble( _U_TEMPERATURE, dValue );
			SetCellText( CD_Data_Temp2, lRow, str );
			
			break;

		case CLog::LOGTYPE_POWER:
			// Power
			( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Power, dValue ) : pLD->GetNext( CLog::ect_Power, dValue );
			str = (CString)WriteCUDouble( _U_TH_POWER, dValue );
			SetCellText( CD_Data_Power, lRow, str );

			// Flow
			( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Flow, dValue ) : pLD->GetNext( CLog::ect_Flow, dValue );
			str = (CString)WriteCUDouble( _U_FLOW, dValue );
			SetCellText( CD_Data_Flow, lRow, str );
			
			// Dp
			( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Dp, dValue ) : pLD->GetNext( CLog::ect_Dp, dValue );
			str = (CString)WriteCUDouble( _U_DIFFPRESS, dValue );
			SetCellText( CD_Data_Dp, lRow, str );
			
			// Delta T
			( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_DeltaT, dValue ) : pLD->GetNext( CLog::ect_DeltaT, dValue );
			str = (CString)WriteCUDouble( _U_DIFFTEMP, dValue );
			SetCellText( CD_Data_DT, lRow, str );

			// Temperature 1
			( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Temp1, dValue ) : pLD->GetNext( CLog::ect_Temp1, dValue );
			str = (CString)WriteCUDouble( _U_TEMPERATURE, dValue );
			SetCellText( CD_Data_Temp1, lRow, str );

			// Temperature 2
			( 0 == iIndex ) ? pLD->GetFirst( CLog::ect_Temp2, dValue ) : pLD->GetNext( CLog::ect_Temp2, dValue );
			str = (CString)WriteCUDouble( _U_TEMPERATURE, dValue );
			SetCellText( CD_Data_Temp2, lRow, str );
			break;
	}

	// Write the index of the sample
	str.Format( _T("%d"), iIndex + 1 );
	SetCellText( CD_Data_Index, lRow, str );
	
	// Write Date and Time
	CTimeUnic dtu;
	pLD->GetDateTime( iIndex, dtu );
 	SetCellText( CD_Data_Date, lRow, dtu.Format( IDS_DATE_FORMAT ) );
 	SetCellText( CD_Data_Time, lRow, dtu.Format( IDS_TIME_FORMAT ) );
		
	return lLine;
}

// Fill SSheetLogData with logged data 
// Return true if there is at least one value.
bool CSSheetLogData::SetRedraw( CLog *pLD, bool fPrint )
{
	SetBool( SSB_REDRAW, FALSE );

	Init();
	if( false == pLD )
		return false;

	long lCount = pLD->GetLength();
	if( 0 == lCount )
		return false;

	BeginWaitCursor();

	// Determine the max nbr. of rows
	long lRowMax = lCount;
	
	SetMaxRows( lRowMax + RD_Header_LastLine + 2 );
	SetMaxCols( CD_Data_Last - 1 );

	// First of all, we initialize columns for data because we need total width of data to adapt header at the same width.
	InitDataColumnWidth( pLD, fPrint );

	// Reformat data columns if total width is lower than RIGHTVIEWWIDTH
	CRect rectSheetSize = GetSheetSizeInPixels( false );
	m_dPageWidth = rectSheetSize.Width();
	if( m_dPageWidth < RIGHTVIEWWIDTH )
	{
		double dFactor = (double)RIGHTVIEWWIDTH / m_dPageWidth;
		InitDataColumnWidth( pLD, fPrint, dFactor );
	}

	// Format the data columns
	FormatData( pLD, fPrint );

	// Format the header
	FormatHeader( pLD, fPrint );

	// Format the Summary header
	FormatSummaryHeader( pLD );

	// Define some default values to avoid function that are time consuming. Important for a log with 40000 samples!!
	// Use simple SetText function instead of SetStaticText
//	SetBlocAsStatic( m_SheetLogDataDescription[GD_Data][CD_Data_None] + 1, RD_Header_LastLine + 2, m_SheetLogDataDescription[GD_Data][CD_Data_Last] - 1, GetMaxRows(), _WHITE );
	
	// Fill and format the table
	long lRow = RD_Header_LastLine + 2;
	SetMaxRows( lRow + lCount );
	for( int iIndex = 0; iIndex < lCount; iIndex++ )
	{
		// Fill the row with data
		FillRow( ++lRow, pLD, iIndex );
				
		// Set the row's height
		SetRowHeight( lRow, m_dRowHeight );
			
		// Draw border
		SetCellBorder( CD_Data_Index, lRow, CD_Data_Last - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	}

	EndWaitCursor();
	SetBool( SSB_REDRAW, TRUE );

	return true;
}

// Define some default values to avoid function that are time consuming. Important for a log with 40000 samples!!
// Use simple SetText function instead of SetStaticText
void CSSheetLogData::SetBlocAsStatic( long lFromColumn, long lFromRow, long lToColumn, long lToRow, COLORREF color )
{
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)color );

	SS_CELLTYPE rCellType;
	SetTypeStaticText( &rCellType, GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign) | 
									GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleVerticalAlign) );
	SetCellTypeRange( lFromColumn, lFromRow, lToColumn, lToRow, &rCellType );

	SetColorRange( lFromColumn, lFromRow, lToColumn, lToRow, color,	GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor) );
}
