#pragma once

#include "RViewSSelCtrl.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES ALL VERSIONS

// Remark: 'CW' is for 'Column Width'.
// Version 3: 2016-10-19: 'CD_SupplyValve_CheckBox' added.
#define CW_RVIEWSSELTRV_SUPPLYVALVE_VERSION				3

// Version 2: 2016-10-19: 'CD_SupplyFlowLimited_CheckBox' added.
#define CW_RVIEWSSELTRV_SUPPLYFLOWLIMITED_VERSION		2

// Version 4: 2019-01-15: HYS-726: 'CD_SVThrmActuator_MaxTemp' added.
// Version 3: 2016-10-19: 'CD_SVThrmActuator_CheckBox' added.
#define CW_RVIEWSSELTRV_SVTHERMOACTUATOR_VERSION		4

// Version 4: 2019-01-15: HYS-726: 'CD_SVElecActuator_MaxTemp' added.
// Version 3: 2016-10-19: 'CD_SVElecActuator_CheckBox' added.
#define CW_RVIEWSSELTRV_SVELECTROACTUATOR_VERSION		4

// Version 2: 2016-10-19: 'CD_ReturnValve_CheckBox' added.
#define CW_RVIEWSSELTRV_RETURNVALVE_VERSION				2
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINE ALL SHEET ID
//
// Remark: don't take enum 'SheetDescription' defined in .h. Because if for any reason we insert a new sheet, the following ID will
//         be increment and will no more correspond to the good one.

#define CW_RVIEWSSELTRV_SHEETID_SUPPLYVALVE				1
#define CW_RVIEWSSELTRV_SHEETID_SUPPLYFLOWLIMITED		2
#define CW_RVIEWSSELTRV_SHEETID_SVTHERMOACTUATOR		5
#define CW_RVIEWSSELTRV_SHEETID_SVELECTROACTUATOR		6
#define CW_RVIEWSSELTRV_SHEETID_RETURNVALVE				7
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class CRViewSSelTrv :public CRViewSSelSS
{
public:
	enum SheetDescription
	{
		SD_First = 1,
		SD_SupplyValve = SD_First,
		SD_SupplyFlowLimited,
		SD_SVInsert,
		SD_SupplyValveAccessories,
		SD_SVThermoActuator,
		SD_SVElectroActuator,
		SD_SVActuatorAccessories,
		SD_ReturnValve,
		SD_ReturnValveAccessories,
		SD_Last
	};
	
	enum RowDescription_SupplyValve
	{
		RD_SupplyValve_FirstRow = 1,
		RD_SupplyValve_GroupName,
		RD_SupplyValve_ColName,
		RD_SupplyValve_Unit,
		RD_SupplyValve_FirstAvailRow
	};
	
	enum ColumnDescription_SupplyValveData
	{
		CD_SupplyValve_FirstColumn = 1,
		CD_SupplyValve_Box,
		CD_SupplyValve_CheckBox,
		CD_SupplyValve_Name,
		CD_SupplyValve_Connection,
		CD_SupplyValve_Version,
		CD_SupplyValve_PN,
		CD_SupplyValve_Size,
		CD_SupplyValve_Setting,
		CD_SupplyValve_Dp,
		CD_SupplyValve_DpFullOpening,
		CD_SupplyValve_Kv,
		CD_SupplyValve_TemperatureRange,
		CD_SupplyValve_Separator,
		CD_SupplyValve_PipeSize,
		CD_SupplyValve_PipeLinDp,
		CD_SupplyValve_PipeV,
		CD_SupplyValve_Pointer,
		CD_SupplyValve_LastColumn
	};

	enum ColumnDescription_SupplyFlowLimitedData
	{
		CD_SupplyFlowLimited_FirstColumn = 1,
		CD_SupplyFlowLimited_CheckBox,
		CD_SupplyFlowLimited_Name,
		CD_SupplyFlowLimited_Connection,
		CD_SupplyFlowLimited_Version,
		CD_SupplyFlowLimited_PN,
		CD_SupplyFlowLimited_Size,
		CD_SupplyFlowLimited_Setting,
		CD_SupplyFlowLimited_Dpmin,
		CD_SupplyFlowLimited_FlowRange,
		CD_SupplyFlowLimited_TemperatureRange,
		CD_SupplyFlowLimited_Separator,
		CD_SupplyFlowLimited_PipeSize,
		CD_SupplyFlowLimited_PipeLinDp,
		CD_SupplyFlowLimited_PipeV,
		CD_SupplyFlowLimited_Pointer,
		CD_SupplyFlowLimited_LastColumn
	};

	enum RowDescription_SVInsert
	{
		RD_SVInsert_FirstRow = 1,
		RD_SVInsert_GroupName,
		RD_SVInsert_DataRow
	};

	enum ColumnDescription_SVInsertData
	{
		CD_SVInsert_FirstColumn = 1,
		CD_SVInsert_Data,
		CD_SVInsert_LastColumn,
	};
	
	enum RowDescription_SupplyValveAccessoryHeader
	{
		RD_SupplyValveAccessory_FirstRow = 1,
		RD_SupplyValveAccessory_GroupName,
		RD_SupplyValveAccessory_FirstAvailRow
	};

	enum ColumDescription_SupplyValveAccessory
	{
		CD_SupplyValveAccessory_FirstColumn = 1,
		CD_SupplyValveAccessory_Left,
		CD_SupplyValveAccessory_Right,
		CD_SupplyValveAccessory_LastColumn
	};

	enum RowDescription_SupplyValveActuator
	{
		RD_SVActuator_FirstRow = 1,
		RD_SVActuator_GroupName,
		RD_SVActuator_ColName,
		RD_SVActuator_Unit,
		RD_SVActuator_FirstAvailRow
	};

	enum ColumnDescription_SVThermostaticActuator
	{
		CD_SVThrmActuator_FirstColumn = 1,
		CD_SVThrmActuator_Box,
		CD_SVThrmActuator_CheckBox,
		CD_SVThrmActuator_Name,
		CD_SVThrmActuator_Description,
		CD_SVThrmActuator_SettingRange,
		CD_SVThrmActuator_CapillaryLength,
		CD_SVThrmActuator_Hysteresis,
		CD_SVThrmActuator_FrostProtection,
		CD_SVThrmActuator_MaxTemp,
		CD_SVThrmActuator_Pointer,
		CD_SVThrmActuator_LastColumn
	};
	
	enum ColumnDescription_SVElectroActuator
	{
		CD_SVElecActuator_FirstColumn = 1,
		CD_SVElecActuator_Box,
		CD_SVElecActuator_CheckBox,
		CD_SVElecActuator_Name,
		CD_SVElecActuator_CloseOffValue,
		CD_SVElecActuator_ActuatingTime,
		CD_SVElecActuator_IP,
		CD_SVElecActuator_PowSupply,
		CD_SVElecActuator_InputSig,
		CD_SVElecActuator_OutputSig,
		CD_SVElecActuator_MaxTemp,
		CD_SVElecActuator_Pointer,
		CD_SVElecActuator_LastColumn
	};
	
	enum RowDescription_ReturnValve
	{
		RD_ReturnValve_FirstRow = 1,
		RD_ReturnValve_GroupName,
		RD_ReturnValve_ColName,
		RD_ReturnValve_Unit,
		RD_ReturnValve_FirstAvailRow
	};
	
	enum ColumnDescription_BalancingValveData
	{
		CD_ReturnValve_FirstColumn = 1,
		CD_ReturnValve_CheckBox,
		CD_ReturnValve_Name,
		CD_ReturnValve_Connection,
		CD_ReturnValve_Version,
		CD_ReturnValve_PN,
		CD_ReturnValve_Size,
		CD_ReturnValve_Preset,
		CD_ReturnValve_Dp,
		CD_ReturnValve_TemperatureRange,
		CD_ReturnValve_Separator,
		CD_ReturnValve_PipeSize,
		CD_ReturnValve_PipeLinDp,
		CD_ReturnValve_PipeV,
		CD_ReturnValve_Pointer,
		CD_ReturnValve_LastColumn
	};
	
	enum RowDescription_ReturnValveAccessoryHeader
	{
		RD_ReturnValveAccessory_FirstRow = 1,
		RD_ReturnValveAccessory_GroupName,
		RD_ReturnValveAccessory_FirstAvailRow
	};

	enum ColumDescription_ReturnValveAccessory
	{
		CD_ReturnValveAccessory_FirstColumn = 1,
		CD_ReturnValveAccessory_Left,
		CD_ReturnValveAccessory_Right,
		CD_ReturnValveAccessory_LastColumn
	};

	enum RowDescription_Accessory
	{
		RD_Accessory_FirstRow = 1,
		RD_Accessory_GroupName,
		RD_Accessory_FirstAvailRow
	};

	enum ColumDescription_Accessory
	{
		CD_Accessory_FirstColumn = 1,
		CD_Accessory_Left,
		CD_Accessory_Right,
		CD_Accessory_LastColumn
	};

	typedef struct _SuggestParams
	{
		bool m_bIsDifferentDpOnSVExist;				// Flag: true if different Dp on supply valve (thermostatic valve) are obtained.
		bool m_bIsDifferentDpOnRVExist;				// Flag: true if different Dp on return valve are obtained.
		double m_dMaxDpSV;							// Max obtained Dp on supply valve (thermostatic valve).
		double m_dMaxDpRV;							// Max obtained Dp on return valve.
		struct _SuggestParams() { m_bIsDifferentDpOnSVExist = false; m_bIsDifferentDpOnSVExist = false; m_dMaxDpSV = 0.0; m_dMaxDpRV = 0.0; }
	}SuggestParams;

	CRViewSSelTrv();
	virtual ~CRViewSSelTrv();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CMultiSpreadBase public virtual methods.
	virtual void Reset( void );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS public virtual methods.

	// Called by 'DlgIndSelXXX' the user click 'Suggest' button.
	// Param: 'pclProductSelectionParameters' contains all needed data.
	virtual void Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam = NULL );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS public pure virtual methods.
	
	// Called by 'DlgIndSelXXX' when user click 'Select' button.
	// param: 'pSelectedProductToFill' -> class where to save current user selection.
	virtual void FillInSelected( CDS_SelProd *pSelectedProductToFill );

	// End of overriding CRViewSSelSS public pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS public virtual methods.

	// Allow to initialize some internal variables.
	// Remark: Called by 'DlgIndSelTrv::OnNewDocument'.
	virtual void OnNewDocument( CDS_IndSelParameter *pclIndSelParameter );

	// Allow to save some internal variables.
	// Remark: called by 'CDlgIndSelDpC::SaveSelectionParameters'.
	virtual void SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter );

	// End of overriding CRViewSSelSS public virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to retrieve the current supply valve selected.
	CDB_ControlValve *GetCurrentSupplyValveSelected();

	// Allow to set the new cell description containing current supply valve selected.
	void SetCurrentSupplyValveSelected( CCellDescriptionProduct *pclCDCurrentSupplyValveSelected );

	// Called by 'CDlgIndSelTrv' when it needs to refresh actuator list with new user choice in combo.
	void UpdateSupplyValveActuators( void );

	// Allow to retrieve the current supply valve actuator selected.
	CDB_Actuator *GetCurrentSVActuatorSelected();

	// Allow to set the new cell description containing current supply valve selected.
	void SetCurrentSVActuatorSelected( CCellDescriptionProduct *pclCDCurrentSVActuatorSelected );

	// Allow to retrieve the current return valve selected.
	CDB_TAProduct *GetCurrentReturnValveSelected();

	// Allow to set the new cell description containing current return valve selected.
	void SetCurrentReturnValveSelected( CCellDescriptionProduct *pclCDCurrentReturnValveSelected );

	// Allow to retrieve the current supply valve actuator accessory list.
	void GetCurrentSVActuatorAccessoryList(CDB_Actuator *pclActuator, CRank* prkAccessoryList, CDB_RuledTableBase **pclRuledTable);

	void FillRowsAcc( bool bForCtrl, CRank *pclList, CDB_RuledTableBase *pclRuledTable );

// Protected members
protected:
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CMultiSpreadBase protected pure virtual methods.
	
	// Returns 'true' if current method has displayed a tooltip.
	virtual bool OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
			TCHAR *pstrTipText, BOOL *pbShowTip );
	
	// End of overriding CMultiSpreadBase protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS protected pure virtual methods.
	
	// This method is called when user click on a product (and not on button or accessories).
	virtual bool OnClickProduct( CSheetDescription *pclSheetDescription, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );

	// This method is called when user click on a title group (to reinitialize default column width).
	virtual bool ResetColumnWidth( short nSheetDescriptionID );

	virtual bool IsSelectionReady( void );

	// End of overriding CRViewSSelSS protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private members
private:
	void _Init( void );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Supply valve.
	
	// Called by 'ClickProduct' when user clicks on a supply valve.
	void _ClickOnSupplyValve( CSheetDescription *pclSheetDescriptionSupplyValve, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );
	
	// Allow to fill supply valve sheet when user clicks 'suggest'.
	// Param: 'pEditedSupplyValve' is passed as argument when we are in edition mode.
	// Return: if we are in edition mode, function returns row number where is the edited supply valve.
	long _FillSupplyValveRows( CDB_TAProduct *pEditedSupplyValve );

	// Allow '_FillSupplyValveRows' to init sheet and fill header.
	void _InitAndFillSupplyValveHeader( CSheetDescription *pclSheetDescriptionSupplyValve, CSSheet *pclSSheet );

	// Allow '_FillSupplyValveRows' to fill one row.
	// Param: 'pSelectedTAP' -> Supply valve to add in the list.
	// Param: 'lRow' -> row where to add the supply valve.
	// Param: 'pEditedTAP' -> Supply valve selected if we are in edition mode.
	// Returns: -1 if error, 0 if all is OK, and greater than 0 if current row is the edited one!
	long _FillOneSupplyValveRow( CSheetDescription *pclSheetDescriptionSupplyValve, CSSheet *pclSSheet, CSelectedValve *pSelectedTAP, 
			long lRow, CDB_TAProduct *pEditedSupplyValve = NULL );

	// Allow to fill supply flow limited control valve sheet when user clicks 'suggest'.
	// Param: 'pEditedSupplyValve' is passed as argument when we are in edition mode.
	// Return: if we are in edition mode, function returns row number where is the edited supply valve.
	long _FillSupplyFlowLimitedRows( CDB_TAProduct *pEditedSupplyValve );

	// Allow '_FillSupplyFlowLimitedRows' to init sheet and fill header.
	void _InitAndFillSupplyFlowLimitedHeader( CSheetDescription *pclSheetDescriptionSupplyValve, CSSheet *pclSSheet );

	// Allow '_FillSupplyFlowLimitedRows' to fill one row.
	// Param: 'pSelectedTAP' -> Supply valve to add in the list.
	// Param: 'lRow' -> row where to add the supply valve.
	// Param: 'pEditedTAP' -> Supply valve selected if we are in edition mode.
	// Returns: -1 if error, 0 if all is OK, and greater than 0 if current row is the edited one!
	long _FillOneSupplyFlowLimitedRow( CSheetDescription *pclSheetDescriptionSupplyValve, CSSheet *pclSSheet, CSelectedValve *pSelectedTAP, 
			long lRow, CDB_TAProduct* pEditedSupplyValve = NULL );

	// Allow to fill accessory sheet when user has chosen a supply valve.
	void _FillSupplyValveAccessoryRows( );

	// Allow to fill actuator sheet when user has chosen a supply valve.
	void _FillSupplyValveActuatorRows( void );
	void _FillSVThermostaticActuatorRows( CSheetDescription *pclSheetDescriptionActuator );
	void _FillSVElectroActuatorRows( CSheetDescription *pclSheetDescriptionActuator );

	// Allow to update 'Setting' and 'Dp' columns when user clicks on a return valve.
	// Remark: if 'dReturnValveDp' is set to -1, then we must clear all ( set "-").
	void _UpdateSupplyValveSettingDp( double dReturnValveDp );

	// Allow to create group for 'Other Insert' supply valve.
	void _CreateOtherInsertGroup( CDS_SSelRadSet *pSelectedTrv );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Supply valve actuator

	// Called by 'ClickProduct' when user clicks on an actuator.
	void _ClickOnSupplyValveActuator( CSheetDescription *pclSheetDescriptionActuator, CDB_Actuator *pActuatorClicked, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow, bool bExternalCall = true );

	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Return valve.

	// Called by 'ClickProduct' when user clicks on a return valve.
	// Param: 'bExternalCall' set to true when call comes from '_ClickOnSupplyValve' to differentiate with a call
	//        that directly comes from 'ClickProduct'.
	void _ClickOnReturnValve( CSheetDescription *pclSheetDescriptionReturnValve, CDB_RegulatingValve *pReturnValveClicked, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow, bool bExternalCall = true );

	// Allow to fill return valve sheet when user has chosen the supply valve.
	/// Param: 'pEditedReturnValve' is passed as argument when we are in edition mode.
	// Return: if we are in edition mode, function returns row number where is the edited return valve.
	long _FillReturnValveRows( CDB_TAProduct *pEditedReturnValve );

	// Allow '_FillReturnValveRows' to init sheet and fill header.
	void _InitAndFillReturnValveHeader( CSheetDescription *pclSheetDescriptionReturnValve, CSSheet *pclSSheet );

	// Allow '_FillReturnValveRows' to fill one row.
	// Param: 'pSelectedTAP' -> Return valve to add in the list.
	// Param: 'lRow' -> row where to add return valve.
	// Param: 'pEditedReturnValve' -> return valve selected if we are in edition mode.
	// Returns: -1 if error, 0 if all is OK, and greater than 0 if current row is the edited one!
	long _FillOneReturnValveRow( CSheetDescription *pclSheetDescriptionReturnValve, CSSheet *pclSSheet, CSelectedValve *pSelectedTAP, 
			long lRow, CDB_TAProduct *pEditedReturnValve = NULL );

	// Allow to fill accessory sheet when user has chosen a return valve.
	void _FillReturnValveAccessoryRows( );

	// Allow to update 'Setting' and 'Dp' columns when user clicks on a supply valve.
	// Remark: if 'dSupplyValveDp' is set to -1, then we must clear all ( set "-").
	void _UpdateReturnValveSettingDp( double dSupplyValveDp );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to retrieve row number where is the actuator.
	long _GetRowOfEditedSVActuator( CSheetDescription *pclSheetDescriptionSVActuator, CDB_Actuator *pActuator );

	// Allow to retrieve row number where is the return valve.
	long _GetRowOfEditedReturnValve( CSheetDescription *pclSheetDescriptionReturnValve, CDB_TAProduct* pEditedTAP );

	void _ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );
	void _WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );

// Private variables
private:
	CIndSelTRVParams *m_pclIndSelTRVParams;
	SuggestParams m_rSuggestParams;

	long m_lSupplyValveSelectedRow;
	long m_lSVActuatorSelectedRow;
	long m_lReturnValveSelectedRow;
	vecCDCAccessoryList m_vecSupplyValveAccessoryList;
	vecCDCAccessoryList m_vecSVActuatorAccessoryList;
	vecCDCAccessoryList m_vecReturnValveAccessoryList;

	bool m_bDiffDpSV;							// 'true' if in the selection there is supply valves with different pressure drop computed.
	bool m_bDiffDpRV;							// 'true' if in the selection there is return valves with different pressure drop computed.
	bool m_bMustFillSVSettingDp;				// 'true' if we must fill supply valve setting and Dp when user clicks on a return valve.
	bool m_bMustFillRVSettingDp;				// 'true' if we must fill return valve setting and Dp when user clicks on a supply valve.
	UINT m_uiSDActuatorID;						// Will contain SheetDescription ID in regards to current actuator type selected in DlgIndSelTrv.
	double m_dSVOtherInsertDp;
	
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsSupplyValve;
	CCDButtonShowAllPriorities *m_pCDBShowAllPrioritiesSupplyValve;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsSVActuator;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsReturnValve;
	CCDButtonShowAllPriorities *m_pCDBShowAllPrioritiesReturnValve;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupActuator;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseRowsSVActuatorAcc;
};

extern CRViewSSelTrv *pRViewSSelTrv;
