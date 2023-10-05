#pragma once


#include "RViewSSelSS.h"
#include "DlgIndSelBv.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES ALL VERSIONS

// Remark: 'CW' is for 'Column Width'.
// Version 2: 2016-10-19: 'CD_BalancingValve_CheckBox' added.
#define CW_RVIEWSSELBV_BALANCINGVALVE_VERSION		2
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINE ALL SHEET ID
//
// Remark: don't take enum 'SheetDescription' defined in .h. Because if for any reason we insert a new sheet, the following ID will
//         be increment and will no more correspond to the good one.

#define CW_RVIEWSSELBV_BALANCINGVALVE_SHEETID		1
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class CRViewSSelBv :public CRViewSSelSS
{
public:
	enum SheetDescription
	{
		SD_Valve = 1,
		SD_Accessory,
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

	CRViewSSelBv();
	virtual ~CRViewSSelBv();

	// Allow to retrieve the current balancing valve selected.
	CDB_TAProduct *GetCurrentBalancingValveSelected( void );

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
	virtual bool OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, TCHAR *pstrTipText, BOOL *pfShowTip );

	// End of overriding CMultiSpreadBase protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS protected pure virtual methods.
	
	// This method is called when user click on a product (and not on button or accessories).
	virtual bool OnClickProduct( CSheetDescription *pclSheetDescriptionBv, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );

	// This method is called when user click on a title group (to reinitialize default column width).
	virtual bool ResetColumnWidth( short nSheetDescriptionID );
	
	virtual bool IsSelectionReady( void );

	// End of overriding CRViewSSelSS protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to set the new cell description containing current balancing valve selected.
	void SetCurrentBalancingValveSelected( CCellDescriptionProduct *pclCDCurrentBalancingValveSelected );

// Private members
private:
	// Allow to fill valve sheet when user has clicked 'suggest'
	long _FillValveRows( CDS_SSelBv *pEditedProduct = NULL );

	// Allow to fill accessory sheet when user has chosen a valve
	void _FillAccessoryRows( );

	void _ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );
	void _WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );

// Private variables
private:
	CIndSelBVParams *m_pclIndSelBVParams;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRows;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupAccessory;
	CCDButtonShowAllPriorities *m_pCDBShowAllPriorities;
	vecCDCAccessoryList m_vecAccessoryList;
	long m_lBvSelectedRow;
};

extern CRViewSSelBv *pRViewSSelBv;
