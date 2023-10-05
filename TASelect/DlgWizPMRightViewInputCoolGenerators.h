#pragma once


#include "ExtStatic.h"
#include "ExtNumEdit.h"
#include "DialogExt.h"
#include "SelectPM.h"
#include "DlgWizPMRightViewInputBase.h"
#include "RViewDescription.h"

class CDlgWizardPM_RightViewInput_CoolingGenerators : public CDlgWizardPM_RightViewInput_Base
{

public:
	enum { IDD = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLGENERATORS };

	CDlgWizardPM_RightViewInput_CoolingGenerators( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent );
	~CDlgWizardPM_RightViewInput_CoolingGenerators();

	DECLARE_MESSAGE_MAP()

	// Overrides 'CDlgWizardPM_RightViewInput_Base' public pure virtual method.
	virtual void FillPMInputUser( void );

	BOOL PreTranslateMessage( MSG *pMsg );

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	virtual afx_msg void OnPaint();
	virtual afx_msg void OnSize( UINT nType, int cx, int cy );
	virtual afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	virtual afx_msg void OnBnClickedReset();
	virtual afx_msg void OnBnClickedResetAll();
	virtual afx_msg void OnBnClickedCancel();
	virtual afx_msg LRESULT OnComboDropCloseUp( WPARAM wParam, LPARAM lParam );
	virtual afx_msg LRESULT OnSSClick( WPARAM wParam, LPARAM lParam );
	virtual afx_msg LRESULT OnSSDblClick( WPARAM wParam, LPARAM lParam );
	virtual afx_msg LRESULT OnSSEditChange( WPARAM wParam, LPARAM lParam );
	// HYS-1164 : Add tooltip
	afx_msg LRESULT OnTextTipFetch( WPARAM wParam, LPARAM lParam );

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizardPM_RightViewInput_Base' protected virtual methods.
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	
// Private methods.
private:
	void _InitializeSSheet( void );
	void _UpdateStaticValue( void );

	void _RemoveColdGenerator( SS_CELLCOORD *plcCellCoord );

	void _AddColdGenerator( CSSheet *plcSSheet,
							CString pstrType = _T( "" ),
							double dCapacity = 0.0,
							double dContent = -1.0,
							bool bManual = false,
							double dTempSupply = -1.0,
							bool bManualTemp = false,
							long lQuantity = 1 ,
							bool bOnlyGUI = false );

	void _FillColdGeneratorCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *strSelect );
	
	void _FindGeneratorID( CSSheet *plcSSheet, int row, CString &strID );
	bool _IsCheckBoxChecked( CSSheet *pclSSheet, SS_COORD col, SS_COORD row );
	void _LoadColdGenerator();

	// Called to update internal container with all data about cold generators.
	void _UpdateColdGenerator();

	// Called when unit change and we need to update value in the sheet.
	void _UpdateColdGeneratorValues();

	bool _ButtonPlusGeneratorEnabled();
	void _CheckColdGeneratorColumnWidth( void );
	void _CheckColdGeneratorSheetHeight( void );

	void _VerifyResetButtons( void );

// Private variables.
private:
	bool m_bChangesDone;
	bool m_bGeneratorValid;

	CViewDescription m_ViewDescription;

	CSheetDescription *m_pclSDescGenerator;
	CSSheet *m_pclSSheetGenerator;
	CRect m_rectSheetInitialSize;

	std::map<int, CDB_StringID*> m_mapGeneratorType;
	int m_iColdGeneratorCount;

	CRect m_ColdGeneratorRectClient;

	enum SheetDescription
	{
		SD_ColdGenerator = 1,
	};

	enum ColumnDescription_ColdGenerator
	{
		CD_ColdGenerator_FirstColumn = 1,
		CD_ColdGenerator_Generator,
		CD_ColdGenerator_Capacity,
		CD_ColdGenerator_Content,
		CD_ColdGenerator_Manual,
		CD_ColdGenerator_SupplyTemp,
		CD_ColdGenerator_ManualTemp,
		CD_ColdGenerator_Quantity,
		CD_ColdGenerator_LastColumn = CD_ColdGenerator_Quantity
	};

	enum RowDescription_ColdGenerator
	{
		RD_ColdGenerator_ColName = 1,
		RD_ColdGenerator_UnitName,
		RD_ColdGenerator_FirstAvailRow
	};

	enum HeatGeneratorColumnWidth
	{
		CGCW_FirstColumn	= 2,
		CGCW_Generator		= 32,
		CGCW_Capacity		= 14,
		CGCW_Content		= 14,
		CGCW_Manual			= 3,
		CGCW_SupplyTemp		= 4,
		CGCW_ManualTemp		= 3,
		CGCW_Quantity		= 8
	};

	CExtStatic m_StaticTotalGeneratorColdCapacity;
	CExtStatic m_StaticTotalGeneratorContent;
	CExtStatic m_StaticTotalInstallation;
	CExtStatic m_StaticTotalContent;

	CStatic m_ButtonGoNormalMode;
	CRect m_ButtonGoNormalModePos;
	bool m_bGoToNormalModeMsgSent;

	bool m_bInitialized;
};
