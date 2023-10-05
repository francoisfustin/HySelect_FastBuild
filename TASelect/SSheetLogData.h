//
// SSheetLogData.h: interface for the CSSheetLogData class.
//
//////////////////////////////////////////////////////////////////////
#pragma once


#include "SSheet.h"

class CSSheetLogData : public CSSheet
{
private:
	enum MainPartRowDescription_Header
	{
		RD_Header_BeforeTitle = 1,
		RD_Header_Title,
		RD_Header_AfterTitle,
	};
	
	enum FirstPartRowDescription_Header
	{
		RD_Header_Plant = RD_Header_AfterTitle,
		RD_Header_Module,
		RD_Header_ValveIndex,
		RD_Header_Valve,
		RD_Header_ValveName,
		RD_Header_LogType,
		RD_Header_EmptyLine,
		RD_Header_LastLine = RD_Header_EmptyLine,
	};
	
	enum SecondPartRowDescription_Header
	{
		RD_Header_StartingTime = RD_Header_Plant,
		RD_Header_EndingTime = RD_Header_Module,
		RD_Header_PointsNumber = RD_Header_ValveIndex,
		RD_Header_DtBetweenPoints = RD_Header_Valve,
		RD_Header_LowestSampleValue = RD_Header_ValveName,
		RD_Header_HighestSampleValue = RD_Header_LogType
	};

	// Columns description
	enum ColumnDescription_Data
	{
		CD_Data_None = 1,			// Cell reference begin at 1
		CD_Data_Index,				// Index
		CD_Data_Date,				// Date
		CD_Data_Time,				// Time
		CD_Data_Power,				// Power
		CD_Data_Flow,				// Flow
		CD_Data_Dp,					// Pressure Drop
		CD_Data_DT,					// Delta Temperature
		CD_Data_Temp1,				// Temperature 1
		CD_Data_Temp2,				// Temperature 2
		CD_Data_Last
	};

	enum GroupDescription
	{
		GD_Header = 1,
		GD_Data
	};

public:
	CSSheetLogData();
	virtual ~CSSheetLogData();

// Overrides
	BOOL Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );
	void ResizeColumns( double dWidth ) {}
	bool SetRedraw( CLog *pLD, bool fPrint = false );

protected:
	void FormatHeader( CLog *pLD, bool fPrint );
	void FormatSummaryHeader( CLog *pLD );
	void InitDataColumnWidth( CLog *pLD, bool fPrint, double dFactor = 1.0 );
	void FormatData( CLog *pLD, bool fPrint );
	void SetBlocAsStatic( long lFromColumn, long lFromRow, long lToColumn, long lToRow, COLORREF color );
	
	// Find the column cell that match the most to the selected
	// value input. Can have value between 0 and 1.
	// Ex : 0.50 --> Will find the middle cell
	// Ex : 0.75 --> Will find the cell at 3/4
	long FindColumnCell( double dPercentage );
	
	long FillRow( long lRow, CLog *pLD, int iIndex );

// Private variables
private:
 	double	m_dRowHeight;
	double	m_dPageWidth;
};