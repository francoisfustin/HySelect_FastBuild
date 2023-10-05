#pragma once


#include "DialogExt.h"
#include "ExtNumEdit.h"
#include "ExtStatic.h"

class CDlgIndSelAdditionalPiping : public CDialogExt
{
public:
	enum { IDD = IDD_DLGINDSELADDPIPES };

	CDlgIndSelAdditionalPiping( CProductSelelectionParameters *pclProductSelParams, CPipeList *pclPipeList, CWnd *pParent = NULL );
	virtual ~CDlgIndSelAdditionalPiping();

	BOOL PreTranslateMessage( MSG *pMsg );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedAddPipes();
	afx_msg void OnBnClickedResetAll();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg LRESULT OnSSClick( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSSEditChange( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnComboDropCloseUp( WPARAM wParam, LPARAM lParam );

// Private methods.
private:
	void _UpdateStaticValue( void );
	void _InitializeSSheet();
	bool _ButtonPlusPipeEnabled();
	bool _IsCheckBoxChecked( CSSheet *pclSSheet, SS_COORD col, SS_COORD row );
	void _AddPipe( CSSheet *pclSSheet, CString strSerieID = _T("" ), CString strPipeID = _T( ""), double dLength = 0.0, bool bOnlyGUI = false );
	void _UpdatePipeList();
	void _LoadPipeList();
	CString _FillPipeSeriesCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *ptcSerieSelect = _T( "" ) );
	CString _FillPipeSizesCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *ptcPipeSerie = _T("" ), const TCHAR *ptcSelect = _T( "") );
	void _RemovePipe( SS_CELLCOORD *plcCellCoord );
	void _FindSerieID( CSSheet *pclSheet, int iRow, CString &strID );
	void _FindSizeID( CSSheet *pclSheet, int iRow, CString &strID );
	void _CheckPipeListColumnWidth( void );

	// Private variables.
private:
	CProductSelelectionParameters *m_pclProductSelParams;
	bool m_bChangesDone;
	bool m_bLengthValid;
	CPipeList *m_pclPipeList;

	// Keep a copy if user cancels.
	CPipeList m_clPipeListCopy;

	CExtStatic m_clStaticTotalWaterVolumeText;
	CExtStatic m_clStaticTotalWaterVolumeValue;

	int m_iPipeCount;
	CSSheet *m_pclSSheet;
	CViewDescription m_ViewDescription;
	CSheetDescription *m_pclSDescription;

	std::map<CString, CTable *> m_mapPipeSeriesType;

	CRect m_PipeListRectClient;

	enum SheetDescription
	{
		SD_PipeList = 1
	};

	enum ColumnDescription_PipeList
	{
		CD_PipeList_FirstColumn = 1,
		CD_PipeList_PipeSeries,
		CD_PipeList_Size,
		CD_PipeList_Length,
		CD_PipeList_LastColumn = CD_PipeList_Length
	};

	enum RowDescription_PipeList
	{
		RD_PipeList_ColName = 1,
		RD_PipeList_UnitName,
		RD_PipeList_FirstAvailRow
	};

	enum PipeListColumnWidth
	{
		PLCW_FirstColumn	= 2,
		PLCW_PipeSeries		= 32,
		PLCW_Size			= 10,
		PLCW_Length			= 10
	};
};
