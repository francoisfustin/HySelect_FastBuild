#pragma once


#include "RViewSSelSS.h"
#include "DlgIndSelSmartControlValve.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES ALL VERSIONS

// Remark: 'CW' is for 'Column Width'.
#define CW_RVIEWSSELSMARTCONTROLVALVE_SMARTCONTROLVALVE_VERSION		2
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINE ALL SHEET ID
//
// Remark: don't take enum 'SheetDescription' defined in .h. Because if for any reason we insert a new sheet, the following ID will
//         be increment and will no more correspond to the good one.

#define CW_RVIEWSSELSMARTCONTROLVALVE_SMARTCONTROLVALVE_SHEETID		1
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class CRViewSSelSmartControlValve :public CRViewSSelSS
{
public:
	enum SheetDescription
	{
		SD_SmartControlValve = 1,
		SD_Accessory,
	};
	
	enum RowDescription_SmartControlValve
	{
		RD_SmartControlValve_FirstRow = 1,
		RD_SmartControlValve_GroupName,
		RD_SmartControlValve_ColName,
		RD_SmartControlValve_Unit,
		RD_SmartControlValve_FirstAvailRow
	};
	
	enum ColumnDescription_SmartControlValveData
	{
		CD_SmartControlValve_FirstColumn = 1,
		CD_SmartControlValve_CheckBox,
		CD_SmartControlValve_Name,
		CD_SmartControlValve_Material,
		CD_SmartControlValve_Connection,
		CD_SmartControlValve_PN,
		CD_SmartControlValve_Size,
		CD_SmartControlValve_Kvs,
		CD_SmartControlValve_Qnom,
		CD_SmartControlValve_DpMin,
		CD_SmartControlValve_DpMax,              // HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
		CD_SmartControlValve_TemperatureRange,
		CD_SmartControlValve_MoreInfo,
		CD_SmartControlValve_Separator,
		CD_SmartControlValve_PipeSize,
		CD_SmartControlValve_PipeLinDp,
		CD_SmartControlValve_PipeV,
		CD_SmartControlValve_Pointer,
		CD_SmartControlValve_LastColumn
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

	CRViewSSelSmartControlValve();
	virtual ~CRViewSSelSmartControlValve();

	// Allow to retrieve the current smart control valve valve selected.
	CDB_TAProduct *GetCurrentSmartControlValveSelected( void );

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
	virtual bool OnClickProduct( CSheetDescription *pclSheetDescriptionSmartControlValve, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );

	// This method is called when user click on a title group (to reinitialize default column width).
	virtual bool ResetColumnWidth( short nSheetDescriptionID );
	
	virtual bool IsSelectionReady( void );

	// End of overriding CRViewSSelSS protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to set the new cell description containing current smart control valve selected.
	void SetCurrentSmartControlValveSelected( CCellDescriptionProduct *pclCDCurrentSmartControlValveSelected );

// Private members
private:
	// Allow to fill valve sheet when user has clicked 'suggest'
	long _FillSmartControlValveRows( CDS_SSelSmartControlValve *pEditedProduct = NULL );

	// Allow to fill accessory sheet when user has chosen a valve
	void _FillAccessoryRows( );

	void _ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );
	void _WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );

// Private variables
private:
	CIndSelSmartControlValveParams *m_pclIndSelSmartControlValveParams;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRows;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupAccessory;
	CCDButtonShowAllPriorities *m_pCDBShowAllPriorities;
	vecCDCAccessoryList m_vecAccessoryList;
	long m_lSmartControlValveSelectedRow;
};

extern CRViewSSelSmartControlValve *pRViewSSelSmartControlValve;
