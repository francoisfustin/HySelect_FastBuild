#pragma once


#include "DialogExt.h"
#include "ExtNumEdit.h"
#include "ExtStatic.h"
#include "SSheet.h"

class CDlgIndSelPMSysVolCooling : public CDialogExt
{
public:
	enum { IDD = IDD_DLGINDSELPMVOLSYSCOLD };

	CDlgIndSelPMSysVolCooling( CIndSelPMParams *pclIndSelPMParams, double dSupplyTemp, double dReturnTemp, CWnd *pParent = NULL );
	virtual ~CDlgIndSelPMSysVolCooling();

	BOOL PreTranslateMessage( MSG *pMsg );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedAddPipes();
	afx_msg void OnBnClickedResetAll();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeMaxAirInputTempToAHU();
	afx_msg void OnChangeBuffer();
	afx_msg LRESULT OnComboDropCloseUp( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSSClick( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSSEditChange( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSSDblClick( WPARAM wParam, LPARAM lParam );
	// HYS-1164 : Add tooltip
	afx_msg LRESULT OnTextTipFetch( WPARAM wParam, LPARAM lParam );

// Private methods.
private:
	void _InitializeSSheet( void );
	void _UpdateStaticValue( void );
	void _RemoveColdConsumer( SS_CELLCOORD *plcCellCoord );
	void _RemoveColdGenerator( SS_CELLCOORD *plcCellCoord );
	void _FillColdGeneratorCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *ptcSelect );
	void _FillConsumersCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *ptcSelect );
	
	void _AddColdConsumer( CSSheet *pclSheet, CString strType = _T( "" ), double dCapacity = 0.0, double dContent = -1.0, bool bManualContent = false,
			double dSupplyTemp = 0.0, double dReturnTemp = 0.0, bool bManualTemp = false, bool bOnlyGUI = false, double dRoomTemp = 0.0,
			long lQuantity = 1, double dSurface = 0.0 );

	void _AddColdGenerator( CSSheet *pclSheet, CString strType = _T( "" ), double dCapacity = 0.0, double dContent = -1.0, bool bManual = false, 
			double dTempSupply = -1.0, bool bManualTemp = false, long lQuantity = 1 , bool bOnlyGUI = false );

	void _FindGeneratorID( CSSheet *pclSheet, int iRow, CString &strID );
	void _FindConsumerID( CSSheet *pclSheet, int iRow, CString &strID );
	bool _IsCheckBoxChecked( CSSheet *pclSheet, SS_COORD col, SS_COORD row );
	void _LoadColdGenerator();
	void _LoadColdConsumer();
	void _UpdateColdGenerator();
	void _UpdateColdConsumer();
	bool _ButtonPlusGeneratorEnabled();
	bool _ButtonPlusConsumerEnabled();
	void _CheckColdGeneratorColumnWidth( void );
	void _CheckColdConsumerColumnWidth( void );
	
// Private variables.
private:
	CIndSelPMParams *m_pclIndSelPMParams;
	bool m_bChangesDone;
	bool m_bBufferValid;
	bool m_bGeneratorValid;
	bool m_bConsumerValid;
	CViewDescription m_ViewDescription;

	// Keep a copy if user cancels.
	CColdGeneratorList m_clColdGeneratorListCopy;
	CColdConsumersList m_clConsumersListCopy;
	CPipeList m_clPipeListCopy;

	CExtNumEdit m_clEditMaxAirInputTempForAHU;
	CExtNumEdit m_clEditOtherBuffer;
	CExtNumEdit m_clEditAdditionalPiping;

	CExtStatic m_clStaticMaxAirInputTempToAHU;
	CExtStatic m_clStaticTotalGenColdCapacity;
	CExtStatic m_clStaticTotalGenContent;
	CExtStatic m_clStaticTargetLinearDp;
	CExtStatic m_clStaticTotalConsColdCapacity;
	CExtStatic m_clStaticTotalConsContent;
	CExtStatic m_clStaticTotalContent;
	CExtStatic m_clStaticTotalInstallation;

	CSheetDescription *m_pclSDescGenerator;
	CSheetDescription *m_pclSDescConsumer;
	CSSheet *m_pclSSheetGenerator;
	CSSheet *m_pclSSheetConsumer;

	std::map<int, CDB_StringID *> m_mapGeneratorType;
	std::map<int, CDB_StringID *> m_mapConsumerType;

	int m_iColdGeneratorCount;
	int m_iConsumerCount;
	double m_dSupplyTemperature;
	double m_dReturnTemperature;

	CRect m_ColdGeneratorRectClient;
	CRect m_ColdConsumerRectClient;

	enum SheetDescription
	{
		SD_ColdGenerator = 1,
		SD_ColdConsumer
	};

	enum RowDescription_ColdGenerator
	{
		RD_ColdGenerator_ColName = 1,
		RD_ColdGenerator_UnitName,
		RD_ColdGenerator_FirstAvailRow
	};

	enum ColumnDescription_ColdGenerator
	{
		CD_ColdGenerator_FirstColumn = 1,
		CD_ColdGenerator_Generator,
		CD_ColdGenerator_Capacity,
		CD_ColdGenerator_Content,
		CD_ColdGenerator_ManualContent,
		CD_ColdGenerator_SupplyTemp,
		CD_ColdGenerator_ManualTemp,
		CD_ColdGenerator_Quantity,
		CD_ColdGenerator_LastColumn = CD_ColdGenerator_Quantity
	};

	enum RowDescription_ColdConsumer
	{
		RD_ColdConsumer_ColName = 1,
		RD_ColdConsumer_UnitName,
		RD_ColdConsumer_FirstAvailRow
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

	enum ColdGeneratorColumnWidth
	{
		CGCW_First			= 2,
		CGCW_Generator		= 32,
		CGCW_Capacity		= 14,
		CGCW_Content		= 14,
		CGCW_Manual			= 3,
		CGCW_SupplyTemp		= 4,
		CGCW_ManualTemp		= 3,
		CGCW_Quantity		= 8
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
};
