#pragma once



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES ALL VERSIONS

// Remark: 'CW' is for 'Column Width'.
// Version 4: 2016-10-19: 'CD_DpController_CheckBox' added.
// Version 3: 'CD_DpController_Preset' added.
// Version 2: 'CD_DpController_DpMax' added.
#define CW_RVIEWSSELDPC_DPCONTROLLER_VERSION				4

// Version 2: 2016-10-19: 'CD_BalancingValve_CheckBox' added.
#define CW_RVIEWSSELDPC_BALANCINGVALVE_VERSION				2
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINE ALL SHEET ID
//
// Remark: don't take enum 'SheetDescription' defined in .h. Because if for any reason we insert a new sheet, the following ID will
//         be increment and will no more correspond to the good one.

#define CW_RVIEWSSELDPC_DPCONTROLLER_SHEETID				1

#define CW_RVIEWSSELDPC_BALANCINGVALVE_SHEETID				2
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRViewSSelDpC :public CRViewSSelSS
{
public:
	enum SheetDescription
	{
		SD_First = 1,
		SD_DpController = SD_First,
		SD_DpCAccessories,
		SD_BalancingValve,
		SD_BvAccessories,
		SD_Last
	};
	
	enum RowDescription_DpController
	{
		RD_DpController_FirstRow = 1,
		RD_DpController_GroupName,
		RD_DpController_ColName,
		RD_DpController_Unit,
		RD_DpController_FirstAvailRow
	};
	
	enum ColumnDescription_DpControllerData
	{
		CD_DpController_FirstColumn = 1,
		CD_DpController_Box,
		CD_DpController_CheckBox,
		CD_DpController_Name,
		CD_DpController_Material,
		CD_DpController_Connection,
		CD_DpController_Version,
		CD_DpController_PN,
		CD_DpController_Size,
		CD_DpController_Preset,
		CD_DpController_DpMin,
		CD_DpController_DplRange,
		CD_DpController_DpMax,
		CD_DpController_TemperatureRange,
		CD_DpController_Separator,
		CD_DpController_PipeSize,
		CD_DpController_PipeLinDp,
		CD_DpController_PipeV,
		CD_DpController_Pointer,
		CD_DpController_LastColumn
	};
	
	enum RowDescription_DpCAccessoryHeader
	{
		RD_DpCAccessory_FirstRow = 1,
		RD_DpCAccessory_GroupName,
		RD_DpCAccessory_FirstAvailRow
	};

	enum ColumDescription_DpCAccessory
	{
		CD_DpCAccessory_FirstColumn = 1,
		CD_DpCAccessory_Left,
		CD_DpCAccessory_Right,
		CD_DpCAccessory_LastColumn
	};

	enum RowDescription_BalancingValve
	{
		RD_BalancingValve_FirstRow = 1,
		RD_BalancingValve_GroupName,
		RD_BalancingValve_ColName,
		RD_BalancingValve_Unit,
		RD_BalancingValve_FirstAvailRow
	};
	
	enum ColumnDescription_BalancingValveData
	{
		CD_BalancingValve_FirstColumn = 1,
		CD_BalancingValve_Box,
		CD_BalancingValve_CheckBox,
		CD_BalancingValve_Name,
		CD_BalancingValve_Material,
		CD_BalancingValve_Connection,
		CD_BalancingValve_Version,
		CD_BalancingValve_PN,
		CD_BalancingValve_Size,
		CD_BalancingValve_Preset,
		CD_BalancingValve_DpSignal,
		CD_BalancingValve_Dp,
		CD_BalancingValve_DpFullOpening,
		CD_BalancingValve_DpHalfOpening,
		CD_BalancingValve_TemperatureRange,
		CD_BalancingValve_Separator,
		CD_BalancingValve_PipeSize,
		CD_BalancingValve_PipeLinDp,
		CD_BalancingValve_PipeV,
		CD_BalancingValve_Pointer,
		CD_BalancingValve_LastColumn
	};
	
	enum RowDescription_BVAccessoryHeader
	{
		RD_BVAccessory_FirstRow = 1,
		RD_BVAccessory_GroupName,
		RD_BVAccessory_FirstAvailRow
	};

	enum ColumDescription_BVAccessory
	{
		CD_BVAccessory_FirstColumn = 1,
		CD_BVAccessory_Left,
		CD_BVAccessory_Right,
		CD_BVAccessory_LastColumn
	};

	CRViewSSelDpC();
	virtual ~CRViewSSelDpC();

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
	// Remark: Called by 'CDlgIndSelDpC::OnNewDocument'.
	virtual void OnNewDocument( CDS_IndSelParameter *pclIndSelParameter );

	// Allow to save some internal variables.
	// Remark: called by 'CDlgIndSelDpC::SaveSelectionParameters'.
	virtual void SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter );

	// End of overriding CRViewSSelSS public virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to retrieve the current Dp controller selected.
	CDB_DpController *GetCurrentDpControllerSelected();

	// Allow to set the new cell description containing current Dp controller selected.
	void SetCurrentDpControllerSelected( CCellDescriptionProduct *pclCDCurrentDpControllerSelected );

	// Allow to retrieve the current balancing valve selected.
	CDB_TAProduct *GetCurrentBalancingValveSelected();

	// Allow to set the new cell description containing current balancing valve selected.
	void SetCurrentBalancingValveSelected( CCellDescriptionProduct *pclCDCurrentBalancingValveSelected );

	// Allow to update CDlgInfoSSelDpC (update here only bitmap and infos concerning Dp controller).
	void UpdateDpCPictureAndInfos( void );

	// Allow to update CDlgInfoSSelDpC (update here only infos concerning balancing valve).
	void UpdateBvInfos( void );

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


// Private members
private:
	void _Init( void );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Dp Controller
	
	// Called by 'ClickProduct' when user clicks on a Dp controller.
	void _ClickOnDpController( CSheetDescription *pclSheetDescriptionDpC, CDB_DpController *pDpControllerClicked, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );
	
	// Allow to fill DpC sheet when user clicks 'suggest'.
	// Param: 'pEditedDpC' is passed as argument when we are in edition mode.
	long _FillDpControllerRows( CDS_SSelDpC *pEditedDpC = NULL );

	// Allow '_FillDpControllerRows' to init sheet and fill header.
	void _InitAndFillDpCHeader( CSheetDescription *pclSheetDescriptionDpC, CSSheet *pclSSheet );

	// Allow '_FillDpControllerRows' to fill one row.
	// Param: 'pSelectedTAP' -> Dp controller to add in the list.
	// Param: 'lRow' -> row where to add Dp controller.
	// Param: 'pEditedTAP' -> Dp controller selected if we are in edition mode.
	// Returns: -1 if error, 0 if all is OK, and greater than 0 if current row is the edited one!
	long _FillOneDpCRow( CSheetDescription *pclSheetDescriptionDpC, CSSheet *pclSSheet, CSelectedValve *pSelectedTAP, long lRow, 
			CDB_TAProduct *pEditedTAP = NULL );

	void _UpdateDpCPresetColumn( bool bReset );

	// Allow to fill accessory sheet when user has chosen a Dp controller.
	void _FillDpCAccessoryRows( );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Balancing valve

	// Called by 'ClickProduct' when user clicks on a balancing valve.
	// Param: 'bExternalCall' set to true when call comes from '_ClickOnDpController' to differentiate with a call
	//        that directly comes from 'ClickProduct'.
	void _ClickOnBalancingValve( CSheetDescription *pclSheetDescriptionBv, CDB_RegulatingValve *pBalancingValve, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow, bool bExternalCall = true );

	// Allow to fill balancing valve sheet when user has chosen the DpController.
	// Param: 'pSelectedTAPDpC' -> current Dp controller selected.
	void _FillBalancingValveRows( CSelectedValve *pSelectedTAPDpC );

	// Allow '_FillBalancingValveRows' to init sheet and fill header.
	void _InitAndFillBvHeader( CSheetDescription *pclSheetDescriptionBv, CSSheet *pclSSheet );

	// Allow '_FillBalancingValveRows' to fill one row.
	// Param: 'pSelectedTAP' -> Balancing valve to add in the list.
	// Param: 'lRow' -> row where to add Balancing valve.
	long _FillOneBvRow( CSheetDescription *pclSheetDescriptionBv, CSSheet *pclSSheet, CSelectedValve *pSelectedTAP, long lRow );

	// Allow to fill accessory sheet when user has chosen a valve.
	void _FillBvAccessoryRows( );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to retrieve row number where is the balancing valve.
	long _GetRowOfEditedBv( CSheetDescription *pclSheetDescriptionBv, CDB_TAProduct *pEditedTAP );

	void _ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );
	void _WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );

	// HYS-1382: Check automatically the crossing accessories for STAP15-50 and STAF15-50.
	void _SelectAutoCrossingAccessories( CDB_RegulatingValve* pBalancingValve, bool bCheck );
// Private variables.
private:
	CIndSelDpCParams *m_pclIndSelDpCParams;
	CDlgInfoSSelDpC *m_pclDlgInfoSSelDpC;
	long m_lDpCSelectedRow;
	long m_lBvSelectedRow;
	vecCDCAccessoryList m_vecDpCAccessoryList;			// Accessories on Dp Controller
	vecCDCAccessoryList m_vecBvAccessoryList;			// Accessories on Balancing Valve
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsDpC;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupDpCAccessory;
	CCDButtonShowAllPriorities *m_pCDBShowAllPrioritiesDpC;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsBv;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupBvAccessory;
	CCDButtonShowAllPriorities *m_pCDBShowAllPrioritiesBv;
};

extern CRViewSSelDpC *pRViewSSelDpC;
