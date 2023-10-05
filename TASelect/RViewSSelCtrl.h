#pragma once


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES ALL VERSIONS

// Remark: 'CW' is for 'Column Width'.
// Version 5: HYS-1199: 2019-10-24: 'CD_ControlValve_Fc' removed.
// Version 4: 2016-10-19: 'CD_ControlValve_CheckBox' added.
// Version 3: 2015-11-18: 'CD_ControlValve_DplRange' added.
// Version 2: 'CD_ControlValve_DpMax' added.
#define CW_RVIEWSSELCTRL_CTRLVALVE_VERSION			5

// Version 7: 2020-09-14: HYS-1458 Rename fail-safe column title and content
// Version 6: 2019-08-30: HYS-1079 Add column 'CD_Actuator_DefaultReturnPos'
// Version 5: 2019-01-15: HYS-726:'CD_Actuator_MaxTemp' added.
// Version 4: 2018-08-08: 'CD_Actuator_RelayType' added.
// Version 3: 2016-10-19: 'CD_Actuator_MaxInletPressure' added.
#define CW_RVIEWSSELCTRL_ACTUATOR_VERSION			7
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINE ALL SHEET ID
//
// Remark: don't take enum 'SheetDescription' defined in .h. Because if for any reason we insert a new sheet, the following ID will
//         be increment and will no more correspond to the good one.

#define CW_RVIEWSSELCTRL_SHEETID_CTRLVALVE			1
#define CW_RVIEWSSELCTRL_SHEETID_ACTUATOR			4	// Why not '2'? Because previous version was already written with sheetdescription ID that was 4.
#define CW_RVIEWSSELCTRL_SHEETID_FIRSTAVAILABLE		( CW_RVIEWSSELCTRL_SHEETID_ACTUATOR + 1 )		// For inherited class.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class CRViewSSelCtrl :public CRViewSSelSS
{
public:
	enum SheetDescription
	{
		SD_ControlValveFirst = 1,
		SD_ControlValve = SD_ControlValveFirst,
		SD_ControlValveAccessory,
		SD_ControlValveAdapter,
		SD_Actuator,
		SD_ActuatorAccessory,
		SD_CtrlActuatorAccessorySet,
		SD_ControlValveLast
	};

	enum RowDescription_ControlValve
	{
		RD_ControlValve_FirstRow = 1,
		RD_ControlValve_GroupName,
		RD_ControlValve_ColName,
		RD_ControlValve_Unit,
		RD_ControlValve_FirstAvailRow
	};

	enum ColumnDescription_ControlValve
	{
		CD_ControlValve_FirstColumn = 1,
		CD_ControlValve_Box,
		CD_ControlValve_CheckBox,
		CD_ControlValve_Name,
		CD_ControlValve_Material,
		CD_ControlValve_Connection,
		CD_ControlValve_Version,
		CD_ControlValve_Size,
		CD_ControlValve_PN,
		CD_ControlValve_Rangeability,
		CD_ControlValve_LeakageRate,
		CD_ControlValve_Stroke,
		CD_ControlValve_ImgCharacteristic,
		CD_ControlValve_ImgSeparator,
		CD_ControlValve_ImgPushClose,
		CD_ControlValve_Kvs,
		CD_ControlValve_Preset,
		CD_ControlValve_Dp,
		CD_ControlValve_DpFullOpening,
		CD_ControlValve_DpHalfOpening,
		CD_ControlValve_DplRange,
		CD_ControlValve_DpMax,
		CD_ControlValve_TemperatureRange,
		CD_ControlValve_Separator,
		CD_ControlValve_PipeSize,
		CD_ControlValve_PipeLinDp,
		CD_ControlValve_PipeV,
		CD_ControlValve_Pointer,
		CD_ControlValve_LastColumn
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

	CRViewSSelCtrl( CMainFrame::RightViewList eRightViewID );
	virtual ~CRViewSSelCtrl( void ) {}

	// Allow to retrieve the current control valve selected.
	CDB_ControlValve *GetCurrentControlValveSelected( void );

	// Allow to retrieve the 'CSelectedValve' object of the current control valve selected.
	CSelectedValve *GetCurrentSelectedValveObject( void );

	// Allow to retrieve the current actuator selected.
	CDB_Actuator *GetCurrentActuatorSelected( void );

	// Allow to retrieve the first adapter selected.
	CDB_Product *GetFirstAdapterSelected( void );

	// Allow to retrieve the next adapter selected.
	CDB_Product *GetNextAdapterSelected( void );

	/**
	 * This function is implemented to management Expand or Collapse cells for Actuator sheet on individual selection
	 * @author awa
	 * @param  (I) pCDBExpandCollapseGroup: a pointer on button
	 * @param  (I) pclSSheetDescription   : current sheet description
	 * @remarks : Created by HYS-821
	 */
	void OnExpandCollapeGroupButtonClicked(CCDButtonExpandCollapseGroup *pCDBExpandCollapseGroup, CSheetDescription *pclSSheetDescription);

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
	// Remark: Called by 'DlgIndSelXXX::OnNewDocument'.
	virtual void OnNewDocument( CDS_IndSelParameter *pclIndSelParameter );

	// Allow to save some internal variables.
	// Remark: called by 'CDlgIndSelXXX::SaveSelectionParameters'.
	virtual void SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter );

	// End of overriding CRViewSSelSS public virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Protected members
protected:	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS protected virtual methods.
	
	// Returns 'true' if current method has displayed a tooltip.
	virtual bool OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
			TCHAR *pstrTipText, BOOL *pbShowTip );
	
	// This method is called to gray incompatible adapter (at now only for BCV and PICV).
	// When use select a control valve, it's possible that there is some TA actuators available for it. But it's also possible to put actuator from other
	// manufacturer. To do that we just need an adapter between control valve and actuator. So when user select a control valve, we show all adapters and also
	// TA actuators. If user click on an adapter, we need to check if this one is compatible with displayed TA actuators or not. If it's not the case then
	// we grayed all actuators that are not compatible. In the same way if user click on an TA actuator, we have to run list of all displayed adapters to check
	// which ones can be used with control valve. Then all incompatible adapters will be also grayed.
	// Param: 'pAccessory' -> pointer on the current accessory (or adapter) concerned by the call.
	// Param: 'fCheck' -> set if accessory is selected or not.
	virtual void GrayUncompatibleAccessoriesCheckbox( CDB_Product *pclAccessory, bool bCheck );
	
	// HYS-1108: This method is called to gray incompatible accessories for cross tables.
	// When use select a control valve, it's possible that there is some actuators available for it. But it's also possible to check actuator that does not
	// contain the accessory. In this case the crossing accessory should not be displayed for control valve. it is grayed out and disabled.
	// Param: 'pclCurrentActuatorSelected' -> pointer on the current actuator concerned by the call. Can be NULL
	// Param: 'fCheck' -> set if accessory is selected or not.
	virtual void GrayUncompatibleCrossingAccessories( CDB_Actuator *pclCurrentActuatorSelected, bool bCheck );
	
	///////////////////////////////////////////////////////////////////////////////////
	// HYS-2031
	// In the database to manage excluded products we do as follow:
	//   * In Cv accessory table (CDB_RuledTable where stem heater is listed),
	//     in the accessory line, we add exlusion sign before adding excluded product.
	//     Stem heater \ !Adapter.
	//    
	//   * In actuator adapter table(CDB_RuledTable where the adapter is listed),
	//     in the adapter line, we add exclusion sign before adding excluded product.
	//     Adapter \ !Stem heater
	// 
	// We look the two tables to manage exclusion
	// /////////////////////////////////////////////////////////////////////////////////
	// This function is created to manage adapter that is included in one product accessory (Stem heater).
	// The adapter is grayed out and unchecked when the stem heater is selected.
	// This is called when an action is on pclAccessory that is the stem heater.
	virtual void GrayOtherExcludedOrUncompatibleProduct( CDB_Product *pclAccessory, bool bChecked );

	// Following user actions it can happens that a sheet disappears. Because we alternate color for the titles,
	// we have to update the title in regards to these new changes.
	virtual void UpdateTitleBackgroundColor( CSheetDescription *pclSheetDescription );
	
	// End of overriding CRViewSSelSS protected virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS protected pure virtual methods.
	
	// This method is called when user click on a product (and not on button or accessories).
	virtual bool OnClickProduct( CSheetDescription *pclSheetDescription, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );

	// This method is called when user click on a title group (to reinitialize default column width).
	virtual bool ResetColumnWidth( short nSheetDescriptionID );
	
	// End of overriding CRViewSSelSS protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to set the new cell description containing current control valve selected.
	void SetCurrentControlValveSelected( CCellDescriptionProduct *pclCDCurrentControlValveSelected );

	// Allow to set the new cell description containing current actuator selected.
	void SetCurrentActuatorSelected( CCellDescriptionProduct *pclCDCurrentActuatorSelected );

	bool IsSelectionReady( void );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// This method must be declared in inherited classes.

	// Allow to know what is the control valve type for the selection.
	virtual bool IsActuatorFit( CDB_ElectroActuator *pclElectroActuator, CDB_ControlValve *pclSelectedControlValve, bool fDowngradeActuatorFunctionality, bool fAcceptAllFailSafe ) = 0;
	// End of pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// These next methods can be overriding in inherited classes for their own purpose.
	
	// Return row where is located edited control valve.
	virtual long FillControlValveRows( CDB_ControlValve *pclSelectedControlValve = NULL );
	virtual void GetAdapterList( CDB_ControlValve *pclSelectedControlValve, CRank *pclAdapterList, CDB_RuledTableBase **ppclRuledTable );
	virtual void GetActuatorAccessoryList( CDB_Actuator *pclSeletedActuator, CRank *pclActuatorAccessoryList, CDB_RuledTableBase **ppclRuledTable );
	virtual void GetSetAccessoryList( CDB_ControlValve *pclSelectedControlValve, CDB_Actuator *pclSeletedActuator, CRank *pclSetAccessoryList, CDB_RuledTableBase **ppclRuledTable );

	// This method is called by the '_ClickOnControlValve' and '_ClickOnActuator' methods when they need to remove sheets that are below them.
	// Remark: typically it is needed for 'RViewSSelDpCBCV' inherited class. If user chooses a TA-COMPACT-DP and selects a corresponding actuator,
	//         we don't want that shutoff valves disappear when user clicks to unselect the actuator.
	virtual void RemoveSheetDescriptions( UINT uiFromSheetDescriptionID );

	// This method is called when user clicks on a control valve. The method '_ClickOnControlValve' fills the sheets in regards to 
	// the current suggested valves, adapters and actuators. And before to exit, the method calls inherited class to allow them to 
	// show other products if needed.
	// Remark: typically it is the case of the 'RViewSSelDpCBCV' inherited class where we need to show shutoff valve when 
	//         user clicks on one control valve.
	virtual void FillOtherProducts( CSelectedValve *pSelectedTAP ) {}

	// This method is called when user edit a previous selection. The method 'Suggest' selects control valve, adapter and actuator.
	// And before to exit the method calls inherited class to allow them to select other products if needed.
	// Remark: typically it is the case of the 'RViewSSelDpCBCV' inherited class where we need to select shutoff valve and its
	//         accessories if it exist in the selection.
	virtual void SuggestOtherProducts( CDS_SSelCtrl *pSelectedControlValve ) {}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	bool m_bDowngradeActuatorFunctionality;
	
// Private members
private:
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Control valve.

	// Called by 'ClickProduct' when user clicks on a control valve.
	void _ClickOnControlValve( CSheetDescription *pclSheetDescriptionCv, CDB_ControlValve *pControlValveClicked, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );
	
	// Allow to fill accessory sheet when user has chosen a control valve.
	void _FillCtrlAccessoryRows( void );

	// Allow to fill adapter sheet when user has chosen a control valve.
	void _FillCtrlAdapterRows( void );

	// Allow to fill actuator sheet when user has chosen the control valve.
	void _FillCtrlActuatorRows( void );

	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Actuator.

	// Called by 'ClickProduct' when user clicks on an actuator.
	// Param: 'fExternalCall' set to true when call comes from '_ClickOnControlValve' to differentiate with a call
	//        that directly comes from 'ClickProduct'.
	void _ClickOnActuator( CSheetDescription *pclSheetDescriptionActuator, CDB_Actuator *pActuatorClicked, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow, bool bExternalCall = true );

	// Allow to fill accessory sheet when user has chosen an actuator.
	void _FillActuatorAccessoryRows( void );

	// Allow to retrieve actuators corresponding to the current control valve.
	void _GetActuatorList( CSelectedValve *pclSelectedValveObject, CRank *pclActuatorList );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void _FillRowsAcc( int iTitleID, bool bForCtrl, CRank *pclList, CDB_RuledTableBase *pclRuledTable );

	// Allow to retrieve row number where is the actuator.
	long _GetRowOfEditedActuator( CSheetDescription *pclSheetDescriptionActuator, CDB_Actuator *pActuator );

	// This method is called when user select an actuator to check what is the compatible adapter for it.
	void _SelectActuatorAdapter( void );

	void _ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );
	void _WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );

// Private variables
private:
	CIndSelCtrlParamsBase *m_pclIndSelCtrlParams;
	long m_lCtrlSelectedRow;
	long m_lActuatorSelectedRow;
	vecCDCAccessoryList m_vecControlValveAccessories;
	vecCDCAccessoryList m_vecControlValveAdapter;
	vecCDCAccessoryList m_vecActuatorAccessories;
	vecCDCAccessoryList m_vecCtrlActuatorSetAccessories;
	vecCDCAccessoryListIter m_vecAdapterIter;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsCtrl;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupCtrlAcc;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupAdapter;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsActuator;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupActuatorAcc;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupCVSetAcc;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupActuator;
	CDB_Product *m_pCurrentAdapter;
};
