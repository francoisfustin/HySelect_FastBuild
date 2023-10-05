#pragma once


#include "DialogExt.h"
#include "ExtNumEdit.h"
#include "ExtStatic.h"
#include "SSheet.h"

class CDlgIndSelPMSysVolHeating : public CDialogExt
{
public:
	enum { IDD = IDD_DLGINDSELPMVOLSYSHEAT };

	CDlgIndSelPMSysVolHeating( CIndSelPMParams *pclIndSelPMParams, double dSupplyTemp, double dReturnTemp, CWnd *pParent = NULL );

	virtual ~CDlgIndSelPMSysVolHeating();

	BOOL PreTranslateMessage( MSG *pMsg );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedCheckGenProtect();
	afx_msg void OnBnClickedAddPipes();
	afx_msg void OnBnClickedResetAll();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnChangeBuffer();
	afx_msg LRESULT OnComboDropCloseUp( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSSClick( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSSEditChange( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSSLeaveCell( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSSDblClick( WPARAM wParam, LPARAM lParam );
	// HYS-1164 : Add tooltip
	afx_msg LRESULT OnTextTipFetch( WPARAM wParam, LPARAM lParam );

// Private methods.
private:
	void _InitializeSSheet( void );
	void _UpdateStaticValue( void );
	void _RemoveHeatConsumer( SS_CELLCOORD *plcCellCoord );
	void _RemoveHeatGenerator( SS_CELLCOORD *plcCellCoord );
	void _FillHeatGeneratorCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *ptcSelect );
	void _FillConsumersCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *ptcSelect );
	
	void _AddHeatConsumer( CSSheet *pclSheet, CString strType = _T(""), double dCapacity = 0.0, double dContent = -1.0, bool bManualContent = false, 
			double dSupplyTemp = 0.0, double dReturnTemp = 0.0, bool bManualTemp = false, bool bOnlyGUI = false );
	
	void _AddHeatGenerator( CSSheet *pclSheet, CString strType = _T(""), double dCapacity = 0.0, double dContent = -1.0, bool bManual = false, 
			bool bOnlyGUI = false );

	void _FindGeneratorID( CSSheet *pclSheet, int iRow, CString &strID );
	void _FindConsumerID( CSSheet *pclSheet, int iRow, CString &strID );
	bool _IsCheckBoxChecked( CSSheet *pclSheet, SS_COORD col, SS_COORD row );
	void _LoadHeatGenerator();
	void _LoadHeatConsumer();
	void _UpdateHeatGenerator();
	void _UpdateHeatConsumer();
	bool _ButtonPlusGeneratorEnabled();
	bool _ButtonPlusConsumerEnabled();
	void _CheckHeatGeneratorColumnWidth( void );
	void _CheckHeatConsumerColumnWidth( void );
	
// Private variables.
private:
	CIndSelPMParams *m_pclIndSelPMParams;
	bool m_bChangesDone;
	bool m_bBufferValid;
	bool m_bGeneratorValid;
	bool m_bConsumerValid;
	CViewDescription m_ViewDescription;

	// Keep a copy if user cancels.
	CHeatGeneratorList m_clHeatGeneratorListCopy;
	CHeatConsumersList m_clConsumersListCopy;
	CPipeList m_clPipeListCopy;

	CButton m_clHeatGeneratorProtect;
	CExtNumEdit m_clEditOtherBuffer;
	CExtNumEdit m_clEditAdditionalPiping;

	CExtStatic m_clStaticTotalGenHeatCapacity;
	CExtStatic m_clStaticTotalGenContent;
	CExtStatic m_clStaticTargetLinearDp;
	CExtStatic m_clStaticTotalConsHeatCapacity;
	CExtStatic m_clStaticTotalConsContent;
	CExtStatic m_clStaticTotalContent;
	CExtStatic m_clStaticTotalInstallation;

	CSheetDescription *m_pclSDescGenerator;
	CSheetDescription *m_pclSDescConsumer;
	CSSheet *m_pclSSheetGenerator;
	CSSheet *m_pclSSheetConsumer;

	std::map<int, CDB_StringID*> m_mapGeneratorType;
	std::map<int, CDB_StringID*> m_mapConsumerType;

	int m_iHeatGeneratorCount;
	int m_iConsumerCount;
	double m_dSupplyTemperature;
	double m_dReturnTemperature;

	CRect m_HeatGeneratorRectClient;
	CRect m_HeatConsumerRectClient;

	enum SheetDescription
	{
		SD_HeatGenerator = 1,
		SD_HeatConsumer
	};

	enum ColumnDescription_HeatGenerator
	{
		CD_HeatGenerator_FirstColumn = 1,
		CD_HeatGenerator_Generator,
		CD_HeatGenerator_Capacity,
		CD_HeatGenerator_Content,
		CD_HeatGenerator_ManualContent,
		CD_HeatGenerator_LastColumn = CD_HeatGenerator_ManualContent
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
};
