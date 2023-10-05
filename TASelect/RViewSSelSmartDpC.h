#pragma once


#include "RViewSSelSS.h"
#include "DlgIndSelSmartDpC.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES ALL VERSIONS

// Remark: 'CW' is for 'Column Width'.
#define CW_RVIEWSSELSMARTVALVE_SMARTDPC_VERSION		1
#define CW_RVIEWSSELSMARTVALVE_DPSENSOR_VERSION		1
#define CW_RVIEWSSELSMARTVALVE_SETS_VERSION			1
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINE ALL SHEET ID
//
// Remark: don't take enum 'SheetDescription' defined in .h. Because if for any reason we insert a new sheet, the following ID will
//         be increment and will no more correspond to the good one.

#define CW_RVIEWSSELSMARTVALVE_SMARTDPC_SHEETID		1
#define CW_RVIEWSSELSMARTVALVE_DPSENSOR_SHEETID		2
#define CW_RVIEWSSELSMARTVALVE_SETS_SHEETID			3
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class CRViewSSelSmartDpC :public CRViewSSelSS
{
public:
	enum SheetDescription
	{
		SD_SmartDpC = 1,
		SD_SmartDpCAccessory,
		SD_DpSensor,
		SD_DpSensorAccessory,
		SD_Sets,
		SD_SetContent,
		SD_SmartDpCLast
	};

	enum RowDescription_SmartDpC
	{
		RD_SmartDpC_FirstRow = 1,
		RD_SmartDpC_GroupName,
		RD_SmartDpC_ColName,
		RD_SmartDpC_Unit,
		RD_SmartDpC_FirstAvailRow
	};

	enum ColumnDescription_SmartDpCData
	{
		CD_SmartDpC_FirstColumn = 1,
		CD_SmartDpC_CheckBox,
		CD_SmartDpC_Name,
		CD_SmartDpC_Material,
		CD_SmartDpC_Connection,
		CD_SmartDpC_PN,
		CD_SmartDpC_Size,
		CD_SmartDpC_Kvs,
		CD_SmartDpC_Qnom,
		CD_SmartDpC_DpMin,
		CD_SmartDpC_DpMax,
		CD_SmartDpC_TemperatureRange,
		CD_SmartDpC_MoreInfo,
		CD_SmartDpC_Separator,
		CD_SmartDpC_PipeSize,
		CD_SmartDpC_PipeLinDp,
		CD_SmartDpC_PipeV,
		CD_SmartDpC_Pointer,
		CD_SmartDpC_LastColumn
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

	enum RowDescription_DpSensor
	{
		RD_DpSensor_FirstRow = 1,
		RD_DpSensor_GroupName,
		RD_DpSensor_ColName,
		RD_DpSensor_Unit,
		RD_DpSensor_FirstAvailRow
	};

	enum ColumnDescription_DpSensor
	{
		CD_DpSensor_FirstColumn = 1,
		CD_DpSensor_Box,
		CD_DpSensor_CheckBox,
		CD_DpSensor_Name,
		CD_DpSensor_Dpl,
		CD_DpSensor_TemperatureRange,
		CD_DpSensor_Pointer,
		CD_DpSensor_LastColumn
	};

	enum RowDescription_Sets
	{
		RD_Sets_FirstRow = 1,
		RD_Sets_GroupName,
		RD_Sets_ColName,
		RD_Sets_FirstAvailRow
	};

	enum ColumnDescription_Sets
	{
		CD_Sets_FirstColumn = 1,
		CD_Sets_Box,
		CD_Sets_CheckBox,
		CD_Sets_Name,
		CD_Sets_Description,
		CD_Sets_Pointer,
		CD_Sets_LastColumn
	};

	CRViewSSelSmartDpC();
	virtual ~CRViewSSelSmartDpC();

	// Allow to retrieve the current smart differential pressure controller valve selected.
	CDB_TAProduct *GetCurrentSmartDpCSelected( void );
	
	// Allow to retrieve the current Dp sensor selected.
	CDB_DpSensor *GetCurrentDpSensorSelected( void );

	// Allow to retrieve the current set selected.
	CDB_Product *GetCurrentProductSetSelected( void );

	void OnExpandCollapeGroupButtonClicked( CCDButtonExpandCollapseGroup *pCDBExpandCollapseGroup, CSheetDescription *pclSSheetDescription );

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
	// Remark: called by 'CDlgIndSelSmartDpC::OnNewDocument'.
	virtual void OnNewDocument( CDS_IndSelParameter *pclIndSelParameter );

	// Allow to save some internal variables.
	// Remark: called by 'CDlgIndSelSmartDpC::SaveSelectionParameters'.
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
	virtual bool OnClickProduct( CSheetDescription *pclSheetDescriptionClicked, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );

	// This method is called when user click on a title group (to reinitialize default column width).
	virtual bool ResetColumnWidth( short nSheetDescriptionID );

	virtual bool IsSelectionReady( void );

	// End of overriding CRViewSSelSS protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to set the new cell description containing current smart differential pressure controller valve selected.
	void SetCurrentSmartDpCSelected( CCellDescriptionProduct *pclCDCurrentSmartDpCSelected );

	// Allow to set the new cell description containing current Dp sensor selected.
	void SetCurrentDpSensorSelected( CCellDescriptionProduct *pclCDCurrentDpSensorSelected );

	// Allow to set the new cell description containing current set selected.
	void SetCurrentSetSelected( CCellDescriptionProduct *pclCDCurrentSetSelected );

	// Private members
private:

	// Called by 'ClickProduct' when user clicks on a smart differential pressure controller.
	void _ClickOnSmartDpC( CSheetDescription *pclSheetDescriptionSmartDpC, CDB_SmartControlValve *pclSmartDpCClicked,
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );
	
	// Allow to fill valve sheet when user has clicked 'suggest'.
	long _FillSmartDpCRows( CDS_SSelSmartDpC *pEditedProduct = NULL );

	// Allow to fill accessory sheet when user has chosen a valve.
	void _FillAccessoryRows();

	// Called by 'ClickProduct' when user clicks on Dp sensor selected with a smart differential pressure controller.
	void _ClickOnDpSensor( CSheetDescription *pclSheetDescriptionDpSensor, CDB_DpSensor *pDpSensorClicked, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );
	void _FillDpSensorRows( CSelectedValve *pSelectedTAP );

	// Allow to fill accessory sheet when user has chosen a Dp sensor.
	void _FillDpSensorAccessoryRows();

	CDB_RuledTable *_GetSetContentRuledTabled( CDB_TAProduct *pclSmartDpC = NULL, CDB_Product *pclProductSet = NULL );
	int _GetSetContentAccessoryList( CRank *pclList, CDB_TAProduct *pclSmartDpC = NULL, CDB_Product *pclProductSet = NULL );
	CDB_DpSensor *_GetDpSensorInDpSensorSet( CDB_DpSensor *pclDpSensorSet );
	bool _IsDpSensorError( CDB_DpSensor *pclDpSensor );

	// Called by 'ClickProduct' when user clicks on sets selected with a smart differential pressure controller.
	void _ClickOnSets( CSheetDescription *pclSheetDescriptionSets, CDB_Product *pSetsClicked, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );
	void _FillSetsRows( CSelectedValve *pSelectedTAP );

	void _FillSetContentAccessoryRows();
	
	// Allow to retrieve row number where is the Dp sensor.
	long _GetRowOfEditedDpSensor( CSheetDescription *pclSheetDescriptionDpSensor, CDB_Product *pclEditedDpSensor );

	// Allow to retrieve row number where is the set.
	long _GetRowOfEditedSet( CSheetDescription *pclSheetDescriptionSets, CDB_Product *pclEditedProductSet );

	void _ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );
	void _WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );

	// Private variables
private:
	CIndSelSmartDpCParams *m_pclIndSelSmartDpCParams;

	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsSmartDpC;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsDpSensor;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsSets;

	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupSmartDpCAccessory;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupDpSensor;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupDpSensorRowAccessory;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupSets;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupConnectionSetAcc;

	CCDButtonShowAllPriorities *m_pCDBShowAllPriorities;
	vecCDCAccessoryList m_vecSmartDpCAccessoryList;
	vecCDCAccessoryList m_vecDpSensorAccessoryList;
	vecCDCAccessoryList m_vecSetContentAccessories;

	long m_lSmartDpCSelectedRow;
	long m_lDpSensorSelectedRow;
	long m_lSetSelectedRow;
};

extern CRViewSSelSmartDpC *pRViewSSelSmartDpC;
