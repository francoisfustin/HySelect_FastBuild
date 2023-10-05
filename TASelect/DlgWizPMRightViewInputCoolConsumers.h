#pragma once


#include "ExtStatic.h"
#include "ExtNumEdit.h"
#include "DialogExt.h"
#include "SelectPM.h"
#include "DlgWizPMRightViewInputBase.h"
#include "RViewDescription.h"

class CDlgWizardPM_RightViewInput_CoolingConsumers : public CDlgWizardPM_RightViewInput_Base
{

public:
	enum { IDD = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLCONSUMERS };

	CDlgWizardPM_RightViewInput_CoolingConsumers( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent );
	~CDlgWizardPM_RightViewInput_CoolingConsumers();

	// Overrides 'CDlgWizardPM_RightViewInput_Base' public pure virtual method.
	virtual void FillPMInputUser( void );

	BOOL PreTranslateMessage( MSG *pMsg );

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
	virtual afx_msg LRESULT OnComboDropCloseUp( WPARAM wParam, LPARAM lParam );
	virtual afx_msg LRESULT OnSSClick( WPARAM wParam, LPARAM lParam );
	virtual afx_msg LRESULT OnSSDblClick( WPARAM wParam, LPARAM lParam );
	virtual afx_msg LRESULT OnSSEditChange( WPARAM wParam, LPARAM lParam );
	    // HYS-1164 : Add tooltip
	afx_msg LRESULT OnTextTipFetch( WPARAM wParam, LPARAM lParam );
	afx_msg void OnEnChangeMaxAirInputTempToAHU();

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizardPM_RightViewInput_Base' protected virtual methods.
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 );

	//////////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	void _InitializeSSheet( void );
	void _UpdateStaticValue( void );
	void _RemoveColdConsumer( SS_CELLCOORD *plcCellCoord);
	void _FillConsumersCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *strSelect );
	
	void _AddColdConsumer( CSSheet *plcSSheet,
						   CString pstrType = _T( "" ),
						   double dCapacity = 0.0,
						   double dContent = -1.0,
						   bool bManualContent = false,
						   double dSupplyTemp = 0.0,
						   double dReturnTemp = 0.0,
						   bool bManualTemp = false,
						   bool bOnlyGUI = false,
						   double dRoomTemp = 0.0 ,
						   long lQuantity = 1,
						   double dSurface = 0.0 );
	
	void _FindConsumerID( CSSheet *plcSSheet, int row, CString &strID );
	void _LoadColdConsumer();
	
	// Called to update internal container with all data about cold consumers.
	void _UpdateColdConsumer();

	// Called when unit change and we need to update value in the sheet.
	void _UpdateColdConsumerValues();
	
	bool _ButtonPlusConsumerEnabled();
	void _CheckColdConsumerColumnWidth( void );
	void _CheckColdConsumerSheetHeight( void );
	bool _IsCheckBoxChecked( CSSheet *pclSSheet, SS_COORD col, SS_COORD row );

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

	CRect m_ColdConsumerRectClient;

	enum SheetDescription
	{
		SD_ColdConsumer = 1,
	};
		
	enum ColumnDescription_ColdConsumer
	{
		CD_ColdConsumer_FirstColumn = 1,
		CD_ColdConsumer_Consumer,
		CD_ColdConsumer_Capacity,
		CD_ColdConsumer_Surface,
		CD_ColdConsumer_Content,
		CD_ColdConsumer_ManualContent,
		CD_ColdConsumer_SupplyTemp,
		CD_ColdConsumer_ReturnTemp,
		CD_ColdConsumer_ManualTemp,
		CD_ColdConsumer_Troom,
		CD_ColdConsumer_Quantity,
		CD_ColdConsumer_LastColumn = CD_ColdConsumer_Quantity
	};

	enum RowDescription_ColdConsumer
	{
		RD_ColdConsumer_ColName = 1,
		RD_ColdConsumer_UnitName,
		RD_ColdConsumer_FirstAvailRow
	};

	enum ColdConsumerColumnWidth
	{
		CCCW_First			= 2,
		CCCW_Consumer		= 22,
		CCCW_Capacity		= 14,
		CCCW_Content		= 14,
		CCCW_ManualContent	= 3,
		CCCW_SupplyTemp		= 4,
		CCCW_ReturnTemp		= 4,
		CCCW_ManualTemp		= 3,
		CCCW_Troom			= 6,
		CCCW_Quantity		= 8,
		CCCW_Surface		= 6
	};
	
	CExtStatic m_StaticTotalConsumerColdCapacity;
	CExtStatic m_StaticTotalConsumerContent;
	CExtStatic m_StaticOtherVolume;
	CExtStatic m_StaticAdditionalPipe;
	CExtStatic m_StaticTargetLinearDp;
	CExtStatic m_StaticTotalInstallation;
	CExtStatic m_StaticTotalContent;
	CExtStatic m_clStaticMaxAirInputTempToAHU;
	CExtStatic m_StaticMaxAirInputTempToAHUUnit;

	CExtNumEdit m_clEditMaxAirInputTempForAHU;
	CExtNumEdit m_ExtEditOtherVolume;
	CExtNumEdit m_ExtEditAdditionalPipe;

	CExtStatic m_StaticOtherVolumeUnit;
	CExtStatic m_StaticAdditionalPipeUnit;

	CStatic m_ButtonGoNormalMode;
	CRect m_ButtonGoNormalModePos;
	bool m_bGoToNormalModeMsgSent;

	bool m_bInitialized;
};
