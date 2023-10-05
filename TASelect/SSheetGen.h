#pragma once


#include "SSheet.h"

class CSSheetGen : public CSSheet
{
private:
	// Column Size
	enum enum_ColumnWidth
	{
		ecw_None =1,		// Cell reference begin at 1
		ecw_Description,
		ecw_Last
	};
	
public:
	CSSheetGen();
	virtual ~CSSheetGen() {}
	
	BOOL Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );
	void ResizeColumns(double dWidth);
	bool SetRedraw();	

// Private methods.
protected:
	void _FormatHeader();
	
	// Format and fill information about the Hydraulic Network.
	void _FormatHN();
	
	// Format and fill information about the Logged Data.
	void _FormatLD();

	// Format and fill information about the Quick Measurement.
	void _FormatQM();

	// Count the number of hydronic modules.
	int _CountNbrHydroMod( CTable *pTab = NULL );

	// Count the number of logged data.
	int _CountNbrLoggedData();

	// Count the number of quick measurements.
	int _CountNbrQuickMeas();

	void _SetColWidth( int iCol, double dSize );

// Private variables.
private:
	int m_iColn;
	int m_iTitleRow;
	int m_iHeaderHNRow;
	int m_iHeaderLDRow;
	int m_iHeaderQMRow;
	int m_iCurRow;
	double m_dRowHeight;
	CArray <double> m_arColWidth;
	double m_dPageWidth;
};