#pragma once


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES ALL VERSIONS

// Remark: 'CW' is for 'Column Width'.
// Version 1: 2018-11-20: created.
#define CW_RVIEWSSELSAFETYVALVE_SAFETYVALVE_VERSION			1

// Version 1: 2019-05-24: created.
#define CW_RVIEWSSELSAFETYVALVE_BLOWTANK_VERSION			1
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINE ALL SHEET ID
//
// Remark: don't take enum 'SheetDescription' defined in .h. Because if for any reason we insert a new sheet, the following ID will
//         be increment and will no more correspond to the good one.

#define CW_RVIEWSSELSAFETYVALVE_SHEETID_SAFETYVALVE			1

#define CW_RVIEWSSELSAFETYVALVE_SHEETID_BLOWTANK			2
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class CRViewSSelSafetyValve :public CRViewSSelSS
{
public:
	enum SheetDescription
	{
		SD_SafetyValve = 1,
		SD_SafetyValveAccessory,
		SD_BlowTank,
		SD_BlowTankAccessory
	};
	
	enum RowDescription_SafetyValve
	{
		RD_SafetyValve_FirstRow = 1,
		RD_SafetyValve_GroupName,
		RD_SafetyValve_ColName,
		RD_SafetyValve_Unit,
		RD_SafetyValve_FirstAvailRow
	};
	
	enum ColumnDescription_SafetyValve
	{
		CD_SafetyValve_FirstColumn = 1,
		CD_SafetyValve_CheckBox,
		CD_SafetyValve_Name,
		CD_SafetyValve_Number,
		CD_SafetyValve_Material,
		CD_SafetyValve_SetPressure,
		CD_SafetyValve_MediumName,
		CD_SafetyValve_Version,
		CD_SafetyValve_LiftingMechanism,
		CD_SafetyValve_Connection,
		CD_SafetyValve_Size,
		CD_SafetyValve_Power,
		CD_SafetyValve_Power100PC,
		CD_SafetyValve_TempRange,
		CD_SafetyValve_Pointer,
		CD_SafetyValve_LastColumn
	};

	enum RowDescription_SafetyValveAccessoryHeader
	{
		RD_SafetyValveAccessory_FirstRow = 1,
		RD_SafetyValveAccessory_GroupName,
		RD_SafetyValveAccessory_FirstAvailRow
	};

	enum ColumDescription_SafetyValveAccessory
	{
		CD_SafetyValveAccessory_FirstColumn = 1,
		CD_SafetyValveAccessory_Left,
		CD_SafetyValveAccessory_Right,
		CD_SafetyValveAccessory_LastColumn
	};

	enum RowDescription_BlowTank
	{
		RD_BlowTank_FirstRow = 1,
		RD_BlowTank_GroupName,
		RD_BlowTank_ColName,
		RD_BlowTank_Unit,
		RD_BlowTank_FirstAvailRow
	};
	
	enum ColumnDescription_BlowTank
	{
		CD_BlowTank_FirstColumn = 1,
		CD_BlowTank_CheckBox,
		CD_BlowTank_Name,
		CD_BlowTank_Number,
		CD_BlowTank_PS,
		CD_BlowTank_MediumName,
		CD_BlowTank_Material,
		CD_BlowTank_Size,
		CD_BlowTank_TempRange,
		CD_BlowTank_Pointer,
		CD_BlowTank_LastColumn
	};

	enum RowDescription_BlowTankAccessoryHeader
	{
		RD_BlowTankAccessory_FirstRow = 1,
		RD_BlowTankAccessory_GroupName,
		RD_BlowTankAccessory_FirstAvailRow
	};

	enum ColumDescription_BlowTankAccessory
	{
		CD_BlowTankAccessory_FirstColumn = 1,
		CD_BlowTankAccessory_Left,
		CD_BlowTankAccessory_Right,
		CD_BlowTankAccessory_LastColumn
	};
	
	CRViewSSelSafetyValve();
	virtual ~CRViewSSelSafetyValve();

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
	// Remark: called by 'CDlgIndSelSeparator::OnNewDocument'.
	virtual void OnNewDocument( CDS_IndSelParameter *pclIndSelParameter );

	// Allow to save some internal variables.
	// Remark: called by 'CDlgIndSelSeparator::SaveSelectionParameters'.
	virtual void SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter );

	// End of overriding CRViewSSelSS public virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to retrieve the current balancing valve selected.
	CDB_TAProduct *GetCurrentSafetyValveSelected( void );
	
	// Allow to retrieve the current blow tank selected.
	CDB_TAProduct *GetCurrentBlowTankSelected();

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
	virtual bool OnClickProduct( CSheetDescription *pclSheetDescriptionSafetyValve, CCellDescriptionProduct *pclCellDescriptionProduct, 
			long lColumn, long lRow );

	// This method is called when user click on a title group (to reinitialize default column width).
	virtual bool ResetColumnWidth( short nSheetDescriptionID );
	
	virtual bool IsSelectionReady( void );

	// End of overriding CRViewSSelSS protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private members
private:
	void _Init( void );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Safety valves.

	// Called by 'ClickProduct' when user clicks on a safety valve.
	void _ClickOnSafetyValve( CSheetDescription *pclSheetDescriptionSafetyValve, CDB_SafetyValveBase *pclSafetyValveClicked, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );

	// Allow to fill valve sheet when user has clicked 'suggest'
	long _FillSafetyValveRows( CDS_SSelSafetyValve *pEditedProduct = NULL );

	// Allow to fill accessory sheet when user has chosen a safety valve.
	void _FillSafetyValveAccessoryRows( );

	// Allow to set the new cell description containing current safety valve selected.
	void _SetCurrentSafetyValveSelected( CCellDescriptionProduct *pclCDCurrentSafetyValveSelected );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Blow tank device.

	// Called by 'ClickProduct' when user clicks on a blow tank device.
	// Param: 'bExternalCall' set to true when call comes from '_ClickOnSafetyValve' to differentiate with a call
	//        that directly comes from 'ClickProduct'.
	void _ClickOnBlowTank( CSheetDescription *pclSheetDescriptionBlowTank, CDB_BlowTank *pclBlowTank, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow, bool bExternalCall = true );

	// Allow '_InitAndFillBlowTankHeader' to init sheet and fill header.
	void _InitAndFillBlowTankHeader( CSheetDescription *pclSheetDescriptionBlowTank, CSSheet *pclSSheet );

	// Allow to fill blow tank sheet when user clicks 'suggest'.
	// Param: 'pSelectedTAPSafetyValve' is passed as argument when we are in edition mode.
	void _FillBlowTankRows( CSelectedSafetyValve *pSelectedTAPSafetyValve );

	// Allow '_FillOneBlowTankRow' to fill one row.
	// Param: 'pSelectedTAP' -> Blow tank to add in the list.
	// Param: 'lRow' -> row where to add blow tank.
	long _FillOneBlowTankRow( CSheetDescription *pclSheetDescriptionBlowTank, CSSheet *pclSSheet, CSelectedBase *pSelectedTAP, long lRow );

	// Allow to fill accessory sheet when user has chosen a blow tank.
	void _FillBlowTankAccessoryRows( );

	// Allow to retrieve the current blow tank selected.
	CDB_TAProduct *_GetCurrentBlowTankSelected( void );

	// Allow to set the new cell description containing current blow tank selected.
	void _SetCurrentBlowTankSelected( CCellDescriptionProduct *pclCDCurrentBlowTankSelected );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to retrieve row number where is the blow tank.
	long _GetRowOfEditedBlowTank( CSheetDescription *pclSheetDescriptionBlowTank, CDB_TAProduct *pEditedTAP );

	void _ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );
	void _WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );

// Private variables.
private:
	CIndSelSafetyValveParams *m_pclIndSelSafetyValveParams;
	long m_lSafetyValveSelectedRow;
	long m_lBlowTankSelectedRow;
	vecCDCAccessoryList m_vecSafetyValveAccessoryList;
	vecCDCAccessoryList m_vecBlowTankAccessoryList;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsSafetyValve;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupSafetyValveAccessory;
	CCDButtonShowAllPriorities *m_pCDBShowAllPrioritiesSafetyValve;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsBlowTank;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupBlowTankAccessory;
	CCDButtonShowAllPriorities *m_pCDBShowAllPrioritiesBlowTank;
};

extern CRViewSSelSafetyValve *pRViewSSelSafetyValve;
