#pragma once



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES ALL VERSIONS

// Remark: 'CW' is for 'Column Width'.
// Version 1: 2019-10-15: Creation.
#define CW_RVIEWSSEL6WAYVALVE_6WAYVALVE_VERSION							1

// Version 1: 2019-10-15: Creation.
#define CW_RVIEWSSEL6WAYVALVE_6WAYVALVE_ACTUATOR_VERSION				1

// Version 1: 2019-10-15: Creation.
#define CW_RVIEWSSEL6WAYVALVE_PIBCVALVEBOTHSIDE_VERSION					1
#define CW_RVIEWSSEL6WAYVALVE_PIBCVALVECOOLIGSIDE_VERSION				1
#define CW_RVIEWSSEL6WAYVALVE_PIBCVALVEHEATINGSIDE_VERSION				1

// Version 1: 2019-10-15: Creation.
#define CW_RVIEWSSEL6WAYVALVE_PIBCVALVEBOTHSIDE_ACTUATOR_VERSION		1
#define CW_RVIEWSSEL6WAYVALVE_PIBCVALVECOOLINSQIDE_ACTUATOR_VERSION		1
#define CW_RVIEWSSEL6WAYVALVE_PIBCVALVEHEATINGSIDE_ACTUATOR_VERSION		1

// Version 1: 2019-10-15: Creation.
#define CW_RVIEWSSEL6WAYVALVE_BALANCINGVALVECOOLINGSIDE_VERSION			1
#define CW_RVIEWSSEL6WAYVALVE_BALANCINGVALVEHEATINGSIDE_VERSION			1
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINE ALL SHEET ID
//
// Remark: don't take enum 'SheetDescription' defined in .h. Because if for any reason we insert a new sheet, the following ID will
//         be increment and will no more correspond to the good one.

#define CW_RVIEWSSELDPC_SHEETID_6WAYVALVE								1
#define CW_RVIEWSSELDPC_SHEETID_6WAYVALVE_ACTUATOR						2
#define CW_RVIEWSSELDPC_SHEETID_PIBCVALVEBOTHSIDE						3
#define CW_RVIEWSSELDPC_SHEETID_PIBCVALVECOOLINGSIDE					4
#define CW_RVIEWSSELDPC_SHEETID_PIBCVALVEHEATINGSIDE					5
#define CW_RVIEWSSELDPC_SHEETID_PIBCVALVEBOTHSIDE_ACTUATOR				6
#define CW_RVIEWSSELDPC_SHEETID_PIBCVALVECOOLINGSIDE_ACTUATOR			7
#define CW_RVIEWSSELDPC_SHEETID_PIBCVALVEHEATINGSIDE_ACTUATOR			8
#define CW_RVIEWSSELDPC_SHEETID_BALANCINGVALVECOOLINGSIDE				9
#define CW_RVIEWSSELDPC_SHEETID_BALANCINGVALVEHEATINGSIDE				10
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRViewSSel6WayValve :public CRViewSSelSS
{
public:
	enum
	{
		SD_TypeValve					= 0x0001,
		SD_TypeValveAccessories			= 0x0002,
		SD_TypeValveActuator			= 0x0004,
		SD_TypeValveActuatorAccessories	= 0x0008,
		SD_TypeLast						= SD_TypeValveActuatorAccessories,
		SD_Family6WayValve				= 0x0100,
		SD_FamilyPIBCValve				= 0x0200,
		SD_FamilyBalancingValve			= 0x0400,
	};

	enum
	{
		SD_SheetTypeMask				= 0x00FF,
		SD_SheetFamilyMask				= 0x0F00,
		SD_SheetSideDefMask				= 0xF000,
	};
		
	#define SHEETSIDEDEFSHIFT								12
	#define GETSHEETSIDE( SheetID )							( (SideDefinition)( ( SheetID & SD_SheetSideDefMask ) >> SHEETSIDEDEFSHIFT ) )
	#define GETSHEETTYPE( SheetID )							( SheetID & SD_SheetTypeMask )
	#define GETSHEETFAMILY( SheetID )						( SheetID & SD_SheetFamilyMask )

	enum SheetDescription
	{
		SD_6WayValve										= ( ( BothSide << SHEETSIDEDEFSHIFT ) + SD_Family6WayValve + SD_TypeValve ),
		SD_6WayValveAccessories								= ( ( BothSide << SHEETSIDEDEFSHIFT ) + SD_Family6WayValve + SD_TypeValveAccessories ),
		SD_6WayValveActuator								= ( ( BothSide << SHEETSIDEDEFSHIFT ) + SD_Family6WayValve + SD_TypeValveActuator ),
		SD_6WayValveActuatorAccessories						= ( ( BothSide << SHEETSIDEDEFSHIFT ) + SD_Family6WayValve + SD_TypeValveActuatorAccessories ),
		
		// Case for application type A: Control via the actuator TA-Slider 160 CO, TA-Slider 160
		// KNX R24 or TA-Slider 160 BACnet/Modbus CO and the pressure independent control valve TA-Modulator.
		SD_PIBCValveBothSide								= ( ( BothSide << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValve ),
		SD_PIBCValveAccessoriesBothSide						= ( ( BothSide << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValveAccessories ),
		SD_PIBCValveActuatorBothSide						= ( ( BothSide << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValveActuator ),
		SD_PIBCValveActuatorAccessoriesBothSide				= ( ( BothSide << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValveActuatorAccessories ),

		// Case for application type B: Control via the actuator TA-MC106Y and the TA-6-way valve.
		// Cooling side.
		SD_PIBCValveCoolingSide								= ( ( CoolingSide << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValve ),
		SD_PIBCValveAccessoriesCoolingSide					= ( ( CoolingSide << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValveAccessories ),
		SD_PIBCValveActuatorCoolingSide						= ( ( CoolingSide << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValveActuator ),
		SD_PIBCValveActuatorAccessoriesCoolingSide			= ( ( CoolingSide << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValveActuatorAccessories ),

		// Heating side.
		SD_PIBCValveHeatingSide								= ( ( HeatingSide << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValve ),
		SD_PIBCValveAccessoriesHeatingSide					= ( ( HeatingSide << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValveAccessories ),
		SD_PIBCValveActuatorHeatingSide						= ( ( HeatingSide << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValveActuator ),
		SD_PIBCValveActuatorAccessoriesHeatingSide			= ( ( HeatingSide << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValveActuatorAccessories ),

		// Case for application type C: Control via the actuator TA-MC106Y and the TA-6-way valve.
		// Cooling side.
		SD_BalancingValveCoolingSide						= ( ( CoolingSide << SHEETSIDEDEFSHIFT ) + SD_FamilyBalancingValve + SD_TypeValve ),
		SD_BalancingValveAccessoriesCoolingSide				= ( ( CoolingSide << SHEETSIDEDEFSHIFT ) + SD_FamilyBalancingValve + SD_TypeValveAccessories ),

		// Heating side.
		SD_BalancingValveHeatingSide						= ( ( HeatingSide << SHEETSIDEDEFSHIFT ) + SD_FamilyBalancingValve + SD_TypeValve ),
		SD_BalancingValveAccessoriesHeatingSide				= ( ( HeatingSide << SHEETSIDEDEFSHIFT ) + SD_FamilyBalancingValve + SD_TypeValveAccessories ),

		SD_Last
	};

	enum SheetDescriptionHelper
	{
		SD_PIBCValve,
		SD_PIBCValveAccessories,
		SD_PIBCValveActuator,
		SD_PIBCValveActuatorAccessories,
		SD_BalancingValve,
		SD_BalancingValveAccessories
	};
	
	enum RowDescription_6WayValve
	{
		RD_6WayValve_FirstRow = 1,
		RD_6WayValve_GroupName,
		RD_6WayValve_ColName,
		RD_6WayValve_Unit,
		RD_6WayValve_FirstAvailRow
	};
	
	// HYS-1884: One column connection. Delete Connect. to TU and Connect. to C/H and replace it by Connection.
	// HYS-1877: Add column for set box picture.
	enum ColumnDescription_6WayValveData
	{
		CD_6WayValve_FirstColumn = 1,
		CD_6WayValve_Box,
		CD_6WayValve_CheckBox,
		CD_6WayValve_Name,
		CD_6WayValve_Material,
		CD_6WayValve_Connection,
		CD_6WayValve_Version,
		CD_6WayValve_PN,
		CD_6WayValve_Size,
		CD_6WayValve_DpCooling,
		CD_6WayValve_DpHeating,
		CD_6WayValve_TemperatureRange,
		CD_6WayValve_Pointer,
		CD_6WayValve_LastColumn
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

	enum RowDescription_PIBCValve
	{
		RD_PIBCValve_FirstRow = 1,
		RD_PIBCValve_GroupName,
		RD_PIBCValve_ColName,
		RD_PIBCValve_Unit,
		RD_PIBCValve_FirstAvailRow
	};
	
	enum ColumnDescription_PIBCVData
	{
		CD_PIBCV_FirstColumn = 1,
		CD_PIBCV_Box,
		CD_PIBCV_CheckBox,
		CD_PIBCV_Name,
		CD_PIBCV_Material,
		CD_PIBCV_Connection,
		CD_PIBCV_Version,
		CD_PIBCV_Size,
		CD_PIBCV_PN,
		CD_PIBCV_Rangeability,
		CD_PIBCV_LeakageRate,
		CD_PIBCV_Stroke,
		CD_PIBCV_ImgCharacteristic,
		CD_PIBCV_ImgSeparator,
		CD_PIBCV_ImgPushClose,
		CD_PIBCV_Preset,
		CD_PIBCV_DpMinCooling,
		CD_PIBCV_DpMinHeating,
		CD_PIBCV_DpMax,
		CD_PIBCV_TemperatureRange,
		CD_PIBCV_Separator,
		CD_PIBCV_PipeSize,
		CD_PIBCV_PipeLinDp,
		CD_PIBCV_PipeV,
		CD_PIBCV_Pointer,
		CD_PIBCV_LastColumn
	};

	enum RowDescription_BalancingValve
	{
		RD_BalancingValve_FirstRow = 1,
		RD_BalancingValve_GroupName,
		RD_BalancingValve_ColName,
		RD_BalancingValve_Unit,
		RD_BalancingValve_FirstAvailRow
	};

	enum ColumnDescription_BVData
	{
		CD_BV_FirstColumn = 1,
		CD_BV_CheckBox,
		CD_BV_Name,
		CD_BV_Material,
		CD_BV_Connection,
		CD_BV_Version,
		CD_BV_PN,
		CD_BV_Size,
		CD_BV_Preset,
		CD_BV_DpSignal,
		CD_BV_Dp,
		CD_BV_DpFullOpening,
		CD_BV_DpHalfOpening,
		CD_BV_TemperatureRange,
		CD_BV_Separator,
		CD_BV_PipeSize,
		CD_BV_PipeLinDp,
		CD_BV_PipeV,
		CD_BV_Pointer,
		CD_BV_LastColumn
	};

	CRViewSSel6WayValve();
	virtual ~CRViewSSel6WayValve();

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
	virtual void FillInSelected( CDS_SelProd *pclSelectedProductToFill );

	// End of overriding CRViewSSelSS public pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS public virtual methods.

	// Allow to initialize some internal variables.
	// Remark: Called by 'CDlgIndSelDpC::OnNewDocument'.
	virtual void OnNewDocument( CDS_IndSelParameter *pclIndSelParameter );

	// Allow to save some internal variables.
	// Remark: called by 'CDlgIndSelDpC::SaveSelectionParameters'.
	virtual void SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter );

	// End of overriding CRViewSSelSS public virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to retrieve the current 6-way valve selected.
	CDB_6WayValve *GetCurrent6WayValveSelected();

	// Allow to retrieve the current actuator selected with the 6-way valve.
	CDB_Actuator *GetCurrent6WayValveActuatorSelected();

	// Allow to retrieve the current pressure independent balancing & control valve selected.
	// 'eSideDefinition' tells if we need the valve on the heating, cooling (Application type B & C) or both side (Application type A).
	CDB_TAProduct *GetCurrentPIBCValveSelected( SideDefinition eSideDefinition );

	// Allow to retrieve the current actuator selected with the pressure independent balancing & control valve.
	// 'eSideDefinition' tells if we need the valve on the heating, cooling (Application type B & C) or both side (Application type A).
	CDB_Actuator *GetCurrentPIBCValveActuatorSelected( SideDefinition eSideDefinition );

	// Allow to retrieve the current balancing valve selected.
	// 'eSideDefinition' tells if we need the valve on the heating, cooling (Application type B & C) or both side (Application type A).
	CDB_TAProduct *GetCurrentBalancingValveSelected( SideDefinition eSideDefinition );

// Protected members
protected:
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CMultiSpreadBase protected virtual methods.
	
	// Returns 'true' if current method has displayed a tooltip.
	virtual bool OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
			TCHAR *pstrTipText, BOOL *pbShowTip );

	// End of overriding CMultiSpreadBase protected virtual methods.
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

	// Allow to set the new cell description containing current 6-way valve selected.
	void SetCurrent6WayValveSelected( CCellDescriptionProduct *pclCDCurrent6WayValveSelected );

	// Allow to set the new cell description containing current actuator selected with the 6-way valve.
	void SetCurrent6WayValveActuatorSelected( CCellDescriptionProduct *pclCDCurrent6WayValveActuatorSelected );

	// Allow to set the new cell description containing the current pressure independent balancing & control valve selected.
	// 'eSideDefinition' tells if we set the valve on the heating, cooling (Application type B & C) or both side (Application type A).
	void SetCurrentPIBCValveSelected( CCellDescriptionProduct *pclCDCurrentPIBCValveSelected, SideDefinition eSideDefinition );

	// Allow to set the new cell description containing the current actuator selected with the pressure independent balancing & control valve.
	// 'eSideDefinition' tells if we set the valve on the heating, cooling (Application type B & C) or both side (Application type A).
	void SetCurrentPIBCValveActuatorSelected( CCellDescriptionProduct *pclCDCurrentPIBCValveActuatorSelected, SideDefinition eSideDefinition );

	// Allow to set the new cell description containing the current balancing valve selected.
	// 'eSideDefinition' tells if we set the valve on the heating, cooling (Application type B & C) or both side (Application type A).
	void SetCurrentBalancingValveSelected( CCellDescriptionProduct *pclCDCurrentBalancingValveSelected, SideDefinition eSideDefinition );

	// HYS-1877: Alow to change the box image state. Closed or Open box.
	void ChangeBoxStateFor6WValveSet( int iButtonState, bool bApplyChange, SideDefinition eSideDefinition, long lRow, CSheetDescription* pclCurrentSheetDescription );

// Private members
private:
	void _Init( void );

	void _SuggestPIBCVHelper( SideDefinition eSideDefinition, CDS_SSel6WayValve *pclSelected6WayValve );
	void _SuggestBVHelper( SideDefinition eSideDefinition, CDS_SSel6WayValve *pclSelected6WayValve );

	void _FillInSelectedPIBCVHelper( SideDefinition eSideDefinition, CDS_SSel6WayValve *pclSelected6WayValveToFill, bool bIsEditionMode );
	void _FillInSelectedBVHelper( SideDefinition eSideDefinition, CDS_SSel6WayValve *pclSelected6WayValveToFill, bool bIsEditionMode );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 6-way valve.
	
	// Called by 'ClickProduct' when user clicks on a 6-way valve.
	void _ClickOn6WayValve( CSheetDescription *pclSheetDescription6WayValve, CDB_6WayValve *pcl6WayValveClicked, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );
	
	// Allow to fill 6-way valve sheet when user clicks 'suggest'.
	// Param: 'pEdited6WayValve' is passed as argument when we are in edition mode.
	long _Fill6WayValveRows( CDS_SSel6WayValve *pEdited6WayValve = NULL );

	// Allow '_Fill6WayValveRows' to init sheet and fill header.
	void _InitAndFill6WayValveHeader( CSheetDescription *pclSheetDescription6WayValve, CSSheet *pclSSheet );

	// Allow '_Fill6WayValveRows' to fill one row.
	// Param: 'pSelectedTAP' -> 6-way valve to add in the list.
	// Param: 'lRow' -> row where to add the 6-way valve.
	// Param: 'pEditedTAP' -> 6-way valve selected if we are in edition mode.
	// Returns: -1 if error, 0 if all is OK, and greater than 0 if current row is the edited one!
	long _FillOne6WayValveRow( CSheetDescription *pclSheetDescription6WayValve, CSSheet *pclSSheet, CSelectedValve *pSelectedTAP, long lRow, 
			CDB_TAProduct *pEditedTAP = NULL );

	void _Fill6WayValveAccessories();

	void _FillErrorMessages( CSheetDescription *pclSheetDescription, long lRow, CString strMsg );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Actuator for 6-way valve.

	void _Fill6WayValveActuators();

	// Called by 'ClickProduct' when user clicks on an actuator selected with a 6-way valve.
	void _ClickOn6WayValveActuator( CSheetDescription *pclSheetDescription6WayValveActuator, CDB_Actuator *p6WayValveActuatorClicked, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );

	void _Fill6WayValveActuatorAccessories();

	// Allow to retrieve actuators corresponding to the current 6-way valve.
	void _Get6WayValveActuatorList( CDB_6WayValve *pclSelected6WayValve, CRank *pclActuatorList );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pressure independent balancing & control valve.

	// Verify if both flow can get results.
	bool _VerifyFlows( CSelectedValve *pSelected6WayValve );

	// Called by 'ClickProduct' when user clicks on a pressure independent balancing & control valve.
	void _ClickOnPIBCValve( CSheetDescription *pclSheetDescriptionPIBCValve, CDB_PIControlValve *pclPIBCValve, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );

	// These three following methods are to fill the pressure independent balancing & control valves.
	// 'eSideDefinition' tells if we fill the valve on the heating, cooling (Application type B & C) or both side (Application type A).
	void _FillPIBCValveRows( CSelectedValve *pSelected6WayValve, SideDefinition eSideDefinition );

	// Allow '_FillPIBCValveRows' to init sheet and fill header.
	void _InitAndFillPIBCValveHeader( CSheetDescription *pclSheetDescriptionPIBCValve, CSSheet *pclSSheet, SideDefinition eSideDefinition );

	// Allow '_FillPIBCValveRows' to fill one row.
	// Param: 'pclSelectedPICValve' -> Pressure independent balancing & control valve to add in the list.
	// Param: 'lRow' -> row where to add the pressure independent balancing & control valve.
	long _FillOnePIBCValveRow( CSheetDescription *pclSheetDescriptionPIBCValve, CSSheet *pclSSheet, CSelectedValve *pclSelectedPICValve, long lRow );

	void _FillPIBCValveAccessories( SideDefinition eSideDefinition );
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Actuator for pressure independent balancing & control valve.

	// Called by 'ClickProduct' when user clicks on an actuator selected with a pressure independent balancing & control valve.
	// 'eSideDefinition' tells if we fill the valve on the heating, cooling (Application type B & C) or both side (Application type A).
	void _FillPIBCValveActuators( SideDefinition eSideDefinition );

	void _ClickOnPIBCValveActuator( CSheetDescription *pclSheetDescriptionPIBCValveActuator, CDB_Actuator *pclPIBCValveActuatorClicked, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );

	// 'eSideDefinition' tells if we fill the valve on the heating, cooling (Application type B & C) or both side (Application type A).
	void _FillPIBCValveActuatorAccessories( SideDefinition eSideDefinition );

	// Allow to retrieve actuators corresponding to the current pressure independent balancing & control valve.
	// 'eSideDefinition' tells if we fill the valve on the heating, cooling (Application type B & C) or both side (Application type A).
	// HYS-1877: Consider set selection.
	void _GetPIBCValveActuatorList( CDB_PIControlValve *pclSelectedPIBCValve, CRank *pclActuatorList, SideDefinition eSideDefinition, bool bForSet = false );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Balancing valve.

	// Called by 'ClickProduct' when user clicks on a control balancing valve.
	void _ClickOnBalancingValve( CSheetDescription *pclSheetDescriptionBalancingValve, CDB_RegulatingValve *pclReturnBalancingValve, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );

	// 'eSideDefinition' tells if we fill the valve on the heating, cooling (Application type B & C) or both side (Application type A).
	void _FillBalancingValveRows( CSelectedValve *pSelected6WayValve, SideDefinition eSideDefinition );

	// Allow '_FillBalancingValveRows' to init sheet and fill header.
	void _InitAndFillBalancingValveHeader( CSheetDescription *pclSheetDescriptionBalancingValve, CSSheet *pclSSheet, SideDefinition eSideDefinition );

	// Allow '_FillBalancingValveRows' to fill one row.
	// Param: 'pSelectedBalancingValve' -> Balancing valve to add in the list.
	// Param: 'lRow' -> row where to add the balancing valve.
	long _FillOneBalancingValveRow( CSheetDescription *pclSheetDescriptionBv, CSSheet *pclSSheet, CSelectedValve *pSelectedBalancingValve, long lRow );

	void _FillBalancingValveAccessories( SideDefinition eSideDefinition );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow when suggesting, to verify if there is only one product. In that case, automatically click on it and eventually on
	// actuator if there is also only one.
	void _VerifyOneProductAndClick( SideDefinition eSideDefinition, int iSheetFamily, int iSheetType );

	void _ClickOnProductClearHelper( int iSheetFamily, SideDefinition eSideDefinition );

	void _RemoveAllSheetAfter( SideDefinition eSideDefinition, int iSheetFamily, int iSheetType );

	// 'bIsMain' is to differentiate between '6-way valve' and 'Pressure indep. balancing & control valve' titles (For the EQMControl case).
	COLORREF _GetGroupColor( bool bIsMain, SideDefinition eSideDefinition, int iSheetType );

	// Allow to know what is the control valve type for the selection.
	bool _IsActuatorFitForPIBCV( CDB_ElectroActuator *pclElectroActuator, CDB_ControlValve *pclSelectedControlValve, 
			bool bDowngradeActuatorFunctionality, bool bAcceptAllFailSafe );

	// Allow to retrieve a list of accessories linked to an actuator.
	void _GetActuatorAccessoryList( CDB_Actuator *pclSeletedActuator, CRank *pclActuatorAccessoryList, CDB_RuledTableBase **ppclRuledTable );

	// Allow to retrieve row number where is the pressure independent balancing & control valve.
	long _GetRowOfEditedPIBCValve( CSheetDescription *pclSheetDescriptionPIBCValve, CDB_TAProduct *pEditedTAP );

	// Allow to retrieve row number where is the actuator.
	long _GetRowOfEditedActuator( CSheetDescription *pclSheetDescriptionActuator, CDB_Actuator *pActuator );

	// Allow to retrieve row number where is the balancing valve.
	long _GetRowOfEditedBValve( CSheetDescription *pclSheetDescriptionBalancingValve, CDB_TAProduct *pEditedTAP );

	// Allow to fill accessory sheet.
	void _FillAccessoryRows( SideDefinition eSideDefinition, int iSheetFamily, int iSheetType, CRank *pclList, CDB_RuledTableBase *pclRuledTable );

	void _ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );
	void _WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );

// Private variables.
private:
	CIndSel6WayValveParams *m_pclIndSel6WayValveParams;

	struct ClassVariables
	{
		bool m_bDowngradeValveActuatorFunctionality;
		long m_lValveSelectedRow;
		long m_lValveActuatorSelectedRow;
		vecCDCAccessoryList m_vecValveAccessoryList;
		vecCDCAccessoryList m_vecValveActuatorAccessoryList;
		CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsValve;
		CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupValveAccessory;
		CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupValveActuator;
		CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsValveActuator;
		CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupValveActuatorAccessory;
		CCDButtonShowAllPriorities *m_pCDBShowAllPrioritiesValve;
	};

	std::map<int, std::map<SideDefinition, ClassVariables>> m_mapVariables;

	#define SETMAPVARIABLE( SheetFamily, SheetSide, Variable, Value )			{ if( 0 != m_mapVariables.count( SheetFamily) && 0 != m_mapVariables[SheetFamily].count( SheetSide) ) \
																						m_mapVariables[SheetFamily][SheetSide].Variable = Value; }

	#define SETMAPVARIABLEALLSIDES( SheetFamily, Variable, Value )				{ SETMAPVARIABLE( SheetFamily, BothSide, Variable, Value ); SETMAPVARIABLE( SheetFamily, HeatingSide, Variable, Value ); \
																						SETMAPVARIABLE( SheetFamily, CoolingSide, Variable, Value ); }

	#define CLEARMAPVARIABLEVECTOR( SheetFamily, SheetSide, Variable )			{ if( 0 != m_mapVariables.count( SheetFamily) && 0 != m_mapVariables[SheetFamily].count( SheetSide) ) \
																						m_mapVariables[SheetFamily][SheetSide].Variable.clear(); }

	#define CLEARMAPVARIABLEVECTORALLSIDES( SheetFamily, Variable )				{ CLEARMAPVARIABLEVECTOR( SheetFamily, BothSide, Variable ); CLEARMAPVARIABLEVECTOR( SheetFamily, HeatingSide, Variable ); \
																						CLEARMAPVARIABLEVECTOR( SheetFamily, CoolingSide, Variable ); }
};

extern CRViewSSel6WayValve *pRViewSSel6WayValve;
