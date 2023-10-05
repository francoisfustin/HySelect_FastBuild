#pragma once


#include "ExtStatic.h"
#include "ExtNumEdit.h"
#include "DialogExt.h"
#include "SelectPM.h"
#include "DlgWizPMRightViewInputBase.h"
#include "RViewDescription.h"

class CDlgWizardPM_RightViewInput_HeatingConsumers : public CDlgWizardPM_RightViewInput_Base
{

public:
	enum { IDD = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATCONSUMERS };

	CDlgWizardPM_RightViewInput_HeatingConsumers( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent );
	~CDlgWizardPM_RightViewInput_HeatingConsumers();

	// Overrides 'CDlgWizardPM_RightViewInput_Base' public pure virtual method.
	virtual void FillPMInputUser( void );

	BOOL PreTranslateMessage( MSG* pMsg );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	virtual afx_msg void OnPaint();
	virtual afx_msg void OnSize( UINT nType, int cx, int cy );
	virtual afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	virtual afx_msg void OnKillFocusOtherVolume();
	virtual afx_msg void OnBnClickedAddPipes();
	virtual afx_msg void OnBnClickedReset();
	virtual afx_msg void OnBnClickedResetAll();
	virtual afx_msg void OnBnClickedCancel();
	virtual afx_msg LRESULT OnComboDropCloseUp(WPARAM wParam, LPARAM lParam);
	virtual afx_msg LRESULT OnSSClick(WPARAM wParam, LPARAM lParam);
	virtual afx_msg LRESULT OnSSDblClick(WPARAM wParam, LPARAM lParam);
	virtual afx_msg LRESULT OnSSEditChange(WPARAM wParam, LPARAM lParam);
	// HYS-1164 : Add tooltip
	afx_msg LRESULT OnTextTipFetch( WPARAM wParam, LPARAM lParam );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizardPM_RightViewInput_Base' protected virtual methods.
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 );

	//////////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	void _InitializeSSheet( void );
	void _UpdateStaticValue( void );
	void _RemoveHeatConsumer(SS_CELLCOORD * plcCellCoord);
	void _FillConsumersCombo( CSSheet* pclSheet, long lCol, long lRow, const TCHAR* strSelect );
	void _AddHeatConsumer(CSSheet * plcSSheet, CString pstrType = _T(""), double dCapacity = 0.0, double dContent = -1.0, bool bManualContent = false, double dSupplyTemp = 0.0, double dReturnTemp = 0.0, bool bManualTemp = false, bool bOnlyGUI = false);
	void _FindConsumerID(CSSheet * plcSSheet, int row, CString &strID);
	void _LoadHeatConsumer();

	// Called to update internal container with all data about heat consumers.
	void _UpdateHeatConsumer();

	// Called when unit change and we need to update value in the sheet.
	void _UpdateHeatConsumerValues();

	bool _ButtonPlusConsumerEnabled();
	void _CheckHeatConsumerColumnWidth( void );
	void _CheckHeatConsumerSheetHeight( void );
	bool _IsCheckBoxChecked( CSSheet * pclSSheet, SS_COORD col, SS_COORD row );

	void _VerifyResetButtons( void );

// Private variables.
private:
	bool m_bChangesDone;
	bool m_bConsumerValid;
	bool m_bBufferValid;

	CViewDescription m_ViewDescription;

	CSheetDescription *m_pclSDescConsumer;
	CSSheet *m_pclSSheetConsumer;
	CRect m_rectSheetInitialSize;

	std::map<int, CDB_StringID*> m_mapConsumerType;
	int m_iConsumerCount;

	CRect m_HeatConsumerRectClient;

	enum SheetDescription
	{
		SD_HeatConsumer = 1,
	};
		
	enum ColumnDescription_HeatConsumer
	{
		CD_HeatConsumer_FirstColumn = 1,
		CD_HeatConsumer_Consumer,
		CD_HeatConsumer_Capacity,
		CD_HeatConsumer_Content,
		CD_HeatConsumer_ManualContent,
		CD_HeatConsumer_SupplyTemp,
		CD_HeatConsumer_ReturnTemp,
		CD_HeatConsumer_ManualTemp,
		CD_HeatConsumer_LastColumn = CD_HeatConsumer_ManualTemp
	};

	enum RowDescription_HeatConsumer
	{
		RD_HeatConsumer_ColName = 1,
		RD_HeatConsumer_UnitName,
		RD_HeatConsumer_FirstAvailRow
	};

	enum HeatConsumerColumnWidth
	{
		HCCW_FirstColumn	= 2,
		HCCW_Consumer		= 20,
		HCCW_Capacity		= 16,
		HCCW_Content		= 16,
		HCCW_ManualContent	= 3,
		HCCW_SupplyTemp		= 13,
		HCCW_ReturnTemp		= 13,
		HCCW_ManualTemp		= 3
	};

	CExtStatic m_StaticTotalConsumerHeatCapacity;
	CExtStatic m_StaticTotalConsumerContent;
	CExtStatic m_StaticOtherVolume;
	CExtStatic m_StaticAdditionalPipe;
	CExtStatic m_StaticTargetLinearDp;
	CExtStatic m_StaticTotalInstallation;
	CExtStatic m_StaticTotalContent;

	CExtNumEdit m_ExtEditOtherVolume;
	CExtNumEdit m_ExtEditAdditionalPipe;

	CExtStatic m_StaticOtherVolumeUnit;
	CExtStatic m_StaticAdditionalPipeUnit;

	CStatic m_ButtonGoNormalMode;
	CRect m_ButtonGoNormalModePos;
	bool m_bGoToNormalModeMsgSent;

	bool m_bInitialized;
};
