#pragma once


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES ALL VERSIONS

// Remark: 'CW' is for 'Column Width'.
// Version 3: 2021-04-01: HYS-1585: Change 'CD_Separator_Name' to well shown Ferro Cleaner name.
// Version 2: 2016-10-19: 'CD_Separator_CheckBox' added.
#define CW_RVIEWSSELSEPARATOR_SEPARATOR_VERSION			3
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINE ALL SHEET ID
//
// Remark: don't take enum 'SheetDescription' defined in .h. Because if for any reason we insert a new sheet, the following ID will
//         be increment and will no more correspond to the good one.

#define CW_RVIEWSSELSEPARATOR_SHEETID_SEPARATOR			1
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class CRViewSSelSeparator :public CRViewSSelSS
{
public:
	enum SheetDescription
	{
		SD_Separator = 1,
		SD_Accessory,
	};
	
	enum RowDescription_Separator
	{
		RD_Separator_FirstRow = 1,
		RD_Separator_GroupName,
		RD_Separator_ColName,
		RD_Separator_Unit,
		RD_Separator_FirstAvailRow
	};
	
	enum ColumnDescription_Separator
	{
		CD_Separator_FirstColumn = 1,
		CD_Separator_CheckBox,
		CD_Separator_Name,
		CD_Separator_Type,
		CD_Separator_Material,
		CD_Separator_Connection,
		CD_Separator_Version,
		CD_Separator_PN,
		CD_Separator_Size,
		CD_Separator_Dp,
		CD_Separator_TemperatureRange,
		CD_Separator_Separator,
		CD_Separator_PipeSize,
		CD_Separator_PipeLinDp,
		CD_Separator_PipeV,
		CD_Separator_Pointer,
		CD_Separator_LastColumn
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
	
	CRViewSSelSeparator();
	virtual ~CRViewSSelSeparator();

	// Allow to retrieve the current balancing valve selected.
	CDB_TAProduct *GetCurrentSeparatorSelected( void );

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
	virtual bool OnClickProduct( CSheetDescription *pclSheetDescriptionSeparator, CCellDescriptionProduct *pclCellDescriptionProduct, 
			long lColumn, long lRow );

	// This method is called when user click on a title group (to reinitialize default column width).
	virtual bool ResetColumnWidth( short nSheetDescriptionID );
	
	virtual bool IsSelectionReady( void );

	// End of overriding CRViewSSelSS protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to set the new cell description containing current separator selected.
	void SetCurrentSeparatorSelected( CCellDescriptionProduct *pclCDCurrentSeparatorSelected );

// Private members
private:
	// Allow to fill valve sheet when user has clicked 'suggest'
	long _FillSeparatorRows( CDS_SSelAirVentSeparator *pEditedProduct = NULL );

	// Allow to fill accessory sheet when user has chosen a separator.
	void _FillAccessoryRows( );

	void _ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );
	void _WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );

// Private variables.
private:
	CIndSelSeparatorParams *m_pclIndSelSeparatorParams;
	long m_lCtrlSelectedRow;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRows;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupAccessory;
	CCDButtonShowAllPriorities *m_pCDBShowAllPriorities;
	vecCDCAccessoryList m_vecAccessoryList;
};

extern CRViewSSelSeparator *pRViewSSelSeparator;
