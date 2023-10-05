#pragma once


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES ALL VERSIONS

// Remark: 'CW' is for 'Column Width'.
// Version 2: 'CD_ShutoffValve_CheckBox' added.
#define CW_RVIEWSSELSV_SHUTOFFVALVE_VERSION			2

// Version 4: 2020-09-14: HYS-1458 Rename fail-safe column title and content
// Version 3: 2019-09-09: HYS-1079: Add CD_Actuator_DefaultReturnPos
// Version 2: 2019-01-15: HYS-726: CD_Actuator_MaxTemp added
// Version 1: 2018-08-14: all the group added.
#define CW_RVIEWSSELSV_ACTUATOR_VERSION				4
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINE ALL SHEET ID
//
// Remark: don't take enum 'SheetDescription' defined in .h. Because if for any reason we insert a new sheet, the following ID will
//         be increment and will no more correspond to the good one.

#define CW_RVIEWSSELSV_SHEETID_SHUTOFFVALVE			1
#define CW_RVIEWSSELSV_SHEETID_ACTUATOR				2
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class CRViewSSelSv :public CRViewSSelSS
{
public:
	enum SheetDescription
	{
		SD_ShutOffValve = 1,
		SD_ShutOffValveAccessory,
		SD_ShutOffValveActuator,
		SD_ShutOffValveActuatorAccessory,
		SD_ShutOffValveLast,
	};
	
	enum RowDescription_ShutoffValve
	{
		RD_ShutoffValve_FirstRow = 1,
		RD_ShutoffValve_GroupName,
		RD_ShutoffValve_ColName,
		RD_ShutoffValve_Unit,
		RD_ShutoffValve_FirstAvailRow
	};
	
	enum ColumnDescription_ShutoffValveData
	{
		CD_ShutoffValve_FirstColumn = 1,
		CD_ShutoffValve_Box,
		CD_ShutoffValve_CheckBox,
		CD_ShutoffValve_Name,
		CD_ShutoffValve_Material,
		CD_ShutoffValve_Connection,
		CD_ShutoffValve_Version,
		CD_ShutoffValve_PN,
		CD_ShutoffValve_Size,
		CD_ShutoffValve_Dp,
		CD_ShutoffValve_TemperatureRange,
		CD_ShutoffValve_Separator,
		CD_ShutoffValve_PipeSize,
		CD_ShutoffValve_PipeLinDp,
		CD_ShutoffValve_PipeV,
		CD_ShutoffValve_Pointer,
		CD_ShutoffValve_LastColumn
	};
	
	enum RowDescription_AccessoryHeader
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

	enum RowDescription_Actuator
	{
		RD_Actuator_FirstRow = 1,
		RD_Actuator_GroupName,
		RD_Actuator_ColName,
		RD_Actuator_Unit,
		RD_Actuator_FirstAvailRow
	};

	enum ColumnDescription_Actuator
	{
		CD_Actuator_FirstColumn = 1,
		CD_Actuator_Box,
		CD_Actuator_CheckBox,
		CD_Actuator_Name,
		CD_Actuator_CloseOffValue,
		CD_Actuator_ActuatingTime,
		CD_Actuator_IP,
		CD_Actuator_PowSupply,
		CD_Actuator_InputSig,
		CD_Actuator_OutputSig,
		CD_Actuator_RelayType,
		CD_Actuator_FailSafe,
		CD_Actuator_DefaultReturnPos,
		CD_Actuator_MaxTemp,
		CD_Actuator_Pointer,
		CD_Actuator_LastColumn
	};
	
	CRViewSSelSv();
	virtual ~CRViewSSelSv();

	// Allow to retrieve the current shut-off valve selected.
	CDB_TAProduct *GetCurrentShutOffValveSelected( void );

	// Allow to retrieve the current actuator selected.
	CDB_Actuator *GetCurrentActuatorSelected( void );

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
	// Remark: called by 'CDlgIndSelBv::OnNewDocument'.
	virtual void OnNewDocument( CDS_IndSelParameter *pclIndSelParameter );

	// Allow to save some internal variables.
	// Remark: called by 'CDlgIndSelBv::SaveSelectionParameters'.
	virtual void SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter );

	// End of overriding CRViewSSelSS public virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
	virtual bool OnClickProduct( CSheetDescription *pclSheetDescriptionSv, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );

	// This method is called when user click on a title group (to reinitialize default column width).
	virtual bool ResetColumnWidth( short nSheetDescriptionID );
	
	virtual bool IsSelectionReady( void );

	// End of overriding CRViewSSelSS protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to set the new cell description containing current shut-off valve selected.
	void SetCurrentShutoffValveSelected( CCellDescriptionProduct *pclCDCurrentShutoffValveSelected );

	// Allow to set the new cell description containing current actuator selected.
	void SetCurrentActuatorSelected( CCellDescriptionProduct *pclCDCurrentActuatorSelected );

// Private members
private:
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Shut-off valve.

	// Called by 'OnClickProduct' when user clicks on a control valve.
	void _ClickOnShutOffValve( CSheetDescription *pclSheetDescriptionSv, CDB_ShutoffValve *pShutOffValveClicked, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );

	// Allow to fill valve sheet when user has clicked 'suggest'.
	long _FillShutOffValveRows( CDS_SSelSv *pEditedProduct = NULL );

	// Allow to fill accessory sheet when user has chosen a valve.
	void _FillShutOffAccessoryRows( );

	// Allow to fill actuator sheet when user has chosen the shut-off valve.
	void _FillShutOffActuatorRows( void );

	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Actuator.

	// Called by 'OnClickProduct' when user clicks on an actuator.
	// Param: 'bExternalCall' set to true when call comes from '_ClickOnShutOffValve' to differentiate with a call
	//        that directly comes from 'ClickProduct'.
	void _ClickOnActuator( CSheetDescription *pclSheetDescriptionActuator, CDB_Actuator *pActuatorClicked, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow, bool bExternalCall = true );

	// Allow to retrieve actuators corresponding to the current shut-off valve.
	void _GetActuatorList( CDB_ShutoffValve *pclSelectedShutOffValve, CRank *pclActuatorList );

	void _GetActuatorAccessoryList( CDB_Actuator *pclSeletedActuator, CRank *pclActuatorAccessoryList, CDB_RuledTableBase **ppclRuledTable );

	// Allow to fill accessory sheet when user has chosen an actuator.
	void _FillActuatorAccessoryRows( void );

	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void _FillRowsAcc( int iTitleID, bool bForCtrl, CRank *pclList, CDB_RuledTableBase *pclRuledTable );

	// Allow to retrieve row number where is the actuator.
	long _GetRowOfEditedActuator( CSheetDescription *pclSheetDescriptionActuator, CDB_Actuator *pActuator );

	void _ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );
	void _WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );

// Private variables
private:
	CIndSelSVParams *m_pclIndSelSVParams;
	long m_lShutOffValveSelectedRow;
	long m_lActuatorSelectedRow;
	vecCDCAccessoryList m_vecShutOffValveAccessoryList;
	vecCDCAccessoryList m_vecActuatorAccessories;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRows;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupAccessory;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsActuator;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupActuatorAcc;
	CCDButtonShowAllPriorities *m_pCDBShowAllPriorities;
};

extern CRViewSSelSv *pRViewSSelSv;
