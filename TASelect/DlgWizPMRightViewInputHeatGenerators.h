#pragma once


#include "ExtStatic.h"
#include "ExtNumEdit.h"
#include "DialogExt.h"
#include "SelectPM.h"
#include "DlgWizPMRightViewInputBase.h"
#include "RViewDescription.h"

class CDlgWizardPM_RightViewInput_HeatingGenerators : public CDlgWizardPM_RightViewInput_Base
{

public:
	enum { IDD = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATGENERATORS };

	CDlgWizardPM_RightViewInput_HeatingGenerators( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent );
	~CDlgWizardPM_RightViewInput_HeatingGenerators();

	DECLARE_MESSAGE_MAP()

	// Overrides 'CDlgWizardPM_RightViewInput_Base' public pure virtual method.
	virtual void FillPMInputUser( void );

	BOOL PreTranslateMessage( MSG* pMsg );

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	virtual afx_msg void OnPaint();
	virtual afx_msg void OnSize( UINT nType, int cx, int cy );
	virtual afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	virtual afx_msg void OnBnClickedCheckGenProtect();
	virtual afx_msg void OnBnClickedReset();
	virtual afx_msg void OnBnClickedResetAll();
	virtual afx_msg void OnBnClickedCancel();
	virtual afx_msg LRESULT OnComboDropCloseUp(WPARAM wParam, LPARAM lParam);
	virtual afx_msg LRESULT OnSSClick(WPARAM wParam, LPARAM lParam);
	virtual afx_msg LRESULT OnSSDblClick(WPARAM wParam, LPARAM lParam);
	virtual afx_msg LRESULT OnSSEditChange(WPARAM wParam, LPARAM lParam);

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizardPM_RightViewInput_Base' protected virtual methods.
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	
// Private methods.
private:
	void _InitializeSSheet( void );
	void _UpdateStaticValue( void );
	void _RemoveHeatGenerator( SS_CELLCOORD *plcCellCoord );
	void _AddHeatGenerator(CSSheet * plcSSheet, CString pstrType = _T(""), double dCapacity = 0.0, double dContent = -1.0, bool bManual = false, bool bOnlyGUI = false);
	void _FillHeatGeneratorCombo( CSSheet* pclSheet, long lCol, long lRow, const TCHAR* strSelect );
	void _FindGeneratorID(CSSheet * plcSSheet, int row, CString &strID);
	bool _IsCheckBoxChecked(CSSheet * pclSSheet, SS_COORD col, SS_COORD row);
	void _LoadHeatGenerator();

	// Called to update internal container with all data about heat generators.
	void _UpdateHeatGenerator();

	// Called when unit change and we need to update value in the sheet.
	void _UpdateHeatGeneratorValues();

	bool _ButtonPlusGeneratorEnabled();
	void _CheckHeatGeneratorColumnWidth( void );
	void _CheckHeatGeneratorSheetHeight( void );

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
	int m_iHeatGeneratorCount;

	CRect m_HeatGeneratorRectClient;

	enum SheetDescription
	{
		SD_HeatGenerator = 1,
	};

	enum ColumnDescription_HeatGenerator
	{
		CD_HeatGenerator_FirstColumn = 1,
		CD_HeatGenerator_Generator,
		CD_HeatGenerator_Capacity,
		CD_HeatGenerator_Content,
		CD_HeatGenerator_Manual,
		CD_HeatGenerator_LastColumn = CD_HeatGenerator_Manual
	};

	enum RowDescription_HeatGenerator
	{
		RD_HeatGenerator_ColName = 1,
		RD_HeatGenerator_UnitName,
		RD_HeatGenerator_FirstAvailRow
	};

	enum HeatGeneratorColumnWidth
	{
		HGCW_FirstColumn	= 2,
		HGCW_Generator		= 32,
		HGCW_Capacity		= 16,
		HGCW_Content		= 16,
		HGCW_Manual			= 3
	};

	CExtStatic m_StaticTotalGeneratorHeatCapacity;
	CExtStatic m_StaticTotalGeneratorContent;
	CExtStatic m_StaticTotalInstallation;
	CExtStatic m_StaticTotalContent;

	CButton m_ButtonHeatGeneratorProtect;

	CStatic m_ButtonGoNormalMode;
	CRect m_ButtonGoNormalModePos;
	bool m_bGoToNormalModeMsgSent;

	bool m_bInitialized;
};
