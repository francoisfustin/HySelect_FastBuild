#pragma once


#include "SSheet.h"

class CSSheetLDSumm : public CSSheet
{
public:
	CSSheetLDSumm();

	// Overrides.
	BOOL Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );
	void ResizeColumns( double dWidth );
	bool SetRedraw( bool fPrint = false );
	bool IsSheetHasData( void ) { return m_fSheetHasData; }

// Protected methods.
protected:
	void InitHeader();
	int FillRow( long lRow, CLog *pLD );
	virtual void SetColWidth( long lColumn, double dSize );

// Private variables.
private:
	enum ColumnDescription
	{
		CD_None = 1, // Cell reference begin at 1
		CD_Name,
		CD_Time,
		CD_Duration,
		CD_Plant,
		CD_Module,
		CD_ValveIndex,
		CD_LogType,
		CD_Last
	};

	long	m_lColn;
	long	m_lTitleRow;
	long	m_lHeaderRow;
	double	m_dRowHeight;
	bool	m_fSheetHasData;
	CArray <double> m_ardColWidth;
	double	m_dPageWidth;
};