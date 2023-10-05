#pragma once


#include "SSheet.h"

class CSSheetHMSumm : public CSSheet
{
public:
	CSSheetHMSumm();
	
	BOOL Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );
	void ResizeColumns( double dWidth );

	// New feature: it's possible now to ask to 'SSheetHMSumm' to draw in an other sheet than itself. To do that, just define
	// 'pclSheet' parameter.
	bool SetRedraw( CSSheet* pclSheet = NULL, bool fPrint = false );
	
	bool IsSheetHasData( void ) { return m_fSheetHasData; }
		
// Private methods.
private:
	void _InitHeader();
	long _FillRow( long lRow, CDS_HydroMod *pHM );

	//	Recursively sweep over modules and call FillRow().
	void _ScanAndFillRow( long *plRow, CTable *pHM );

	void _SetColWidth( long lColumn, double dSize );

// Private variables.
protected:
	enum ColumnDescription
	{
		CD_None =1,		// Cell reference begin at 1
		CD_Name,
		CD_Description,
		CD_ParentModule,
		CD_ValveNumber,
		CD_TotalFlow,
		CD_Last
	};

	long	m_lColn;
	long	m_lTitleRow;
	long	m_lHeaderRow;
	double	m_dRowHeight;
	bool	m_fSheetHasData;
	CArray <double> m_ColWidth;
	double	m_dPageWidth;
	CSSheet* m_pclSSheetToDraw;
};
