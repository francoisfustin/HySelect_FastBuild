#pragma once


#include "SSheet.h"

class CSSheetQuickMeas : public CSSheet
{
public:
	CSSheetQuickMeas();
	virtual ~CSSheetQuickMeas();
	
	BOOL Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );
	void ResizeColumns( double dWidth );
	bool SetRedraw( bool fExport = false, bool fPrint = false, CRank* pRankList = NULL );
	bool IsSheetHasData( void ) { return m_fSheetHasData; }

	// Spread Event are intercepted by the sheet's owner.
	void CellClicked( long lColumn, long lRow );	

	/**
	 * This function inserts a new line to add a note from user
	 * @author awa
	 * @param  (I) lColumn: current column
	 * @param  (I) lRow   : current row
	 * @remarks : Created by HYS658
	 */
	void AddNotes(long lColumn, long lRow);
	void AddDescriptionToNote(long lRow, CString strDescription);

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	// Returns 'true' if current method has displayed a tooltip.
	afx_msg LRESULT TextTipFetchQmSpread(WPARAM wParam, LPARAM lParam);

	void InitHeader( bool fExport = false, bool fPrint = false, CRank* pRankList = NULL );
	
	// Return the real row if a note is insert in current pHMX.
	long FillRow( long lRow, CDS_HydroModX *pHMX, bool fAttachedMeasurement = false );
	void SetWCinSubTitle();
	void SetMinMaxFlowDevInSubTitle();
	void HideEmptyColumns();
	void RedefineColWidth();
	void ResizeAndColorSubTitles();
	virtual void SetColWidth( long lColumn, double dSize );
	void SetGroupColor( COLORREF &Color );
	void CollapseAll( void );
	void ExpandAll( void );
	int SendQMToRow(CDS_HydroModX *pHM);

	/**
	 * This function shows a tooltip if mouseOver the cell Note (+)
	 * @author awa
	 * @remarks : Created by HYS658
	 */
	void OnTextTipFetch(long lColumn, long lRow, WORD* pwMultiLine, TCHAR* pstrTipText, BOOL* pfShowTip);
  
// Private variables.
private:
	enum RowDescription
	{
		RD_None = 1,		// Cell reference begin at 1
		RD_Title,
		RD_NbrQM,
		RD_SubTitle,
		RD_InfoSubTitle,
		RD_Info,
		RD_InfoUnits,
		RD_Last,
	};
	
	enum ColumnDescription
	{
		CD_None = 1,		// Cell reference begin at 1
		CD_Picture,
		CD_Note,
		CD_DateTime,
		CD_Reference,
		CD_ReferenceSep,
		CD_WaterChar,
		CD_Valve,
		CD_Setting,
		CD_Kv,
		CD_KvSignal,
		CD_KvSignalSep,
		CD_Dp,
		CD_DpSep,
		CD_DpFlow,
		CD_Flow,
		CD_DesignFlow,
		CD_FlowDeviation,
		CD_FlowDeviationSep,
		CD_TempHH,
		CD_Temp1DPS,
		CD_Temp2DPS,
		CD_DiffTemp,
		CD_DiffTempSep,
		CD_Power,
		CD_Last
	};

	long	m_lColn;
	long	m_lTitleRow;
	int		m_iNbrQM;
	double	m_dRowHeight;
	bool	m_fSheetHasData;
	CArray <double> m_arlColWidth;
	double	m_dPageWidth;
	double	m_dRViewWidth;

	bool	m_fHideCircuitSubTitle;
	bool	m_fHideDpSubTitle;
	bool	m_fHideFlowSubTitle;
	bool	m_fHideTempSubTitle;
	bool	m_fHidePowerSubTitle;
	
	std::vector <CDS_HydroModX*> m_vectorAlreadyPrinted;
	CArray <long> m_arlColumnExpandState;			// Contains columns in expanded state
};
