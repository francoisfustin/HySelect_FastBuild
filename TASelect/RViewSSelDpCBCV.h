#pragma once


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES ALL VERSIONS

// Remark: 'CW' is for 'Column Width'.
// Version 2: 2016-10-19: 'CD_ShutoffValve_CheckBox' added.
#define CW_RVIEWSSELDPCBCV_SHUTOFFVALVE_VERSION		2
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINE ALL SHEET ID
//
// Remark: don't take enum 'SheetDescription' defined in .h. Because if for any reason we insert a new sheet, the following ID will
//         be increment and will no more correspond to the good one.
//
// Here because we are in an inherited class, we take the first available sheet ID after base class.
// Pay attention, if you have other inherited classes to not overlap these values.
#define CW_RVIEWSSELDPCBCV_SHEETID_SHUTOFFVALVE		CW_RVIEWSSELCTRL_SHEETID_FIRSTAVAILABLE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class CDlgInfoSSelDpCBCV;
class CRViewSSelDpCBCV : public CRViewSSelCtrl
{
public:
	enum SheetDescription
	{
		SD_DpCBCValveFirst = CRViewSSelCtrl::SheetDescription::SD_ControlValveLast,
		SD_ShutoffValve = SD_DpCBCValveFirst,
		SD_SVAccessory,
		SD_DpCBCValveLast,
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

	CRViewSSelDpCBCV();
	virtual ~CRViewSSelDpCBCV();

	// Allow to update CDlgInfoSSelDpCBCV.
	void UpdateDpCBCVPictureAndInfos( void );

	// Allow to update CDlgInfoSSelDpCBCV (update here only bitmap and infos concerning combined Dp controller, control and balancing valve).
	void UpdateSVInfos( void );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CMultiSpreadBase public virtual methods.
	virtual void Reset( void );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS public virtual methods.

	// Allow to initialize some internal variables.
	// Remark: Called by 'DlgIndSelXXX::OnNewDocument'.
	virtual void OnNewDocument( CDS_IndSelParameter *pclIndSelParameter );

	// Allow to save some internal variables.
	// Remark: called by 'CDlgIndSelXXX::SaveSelectionParameters'.
	virtual void SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter );

	// Called by 'DlgIndSelXXX' the user click 'Suggest' button.
	// Param: 'pclProductSelectionParameters' contains all needed data.
	virtual void Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam = NULL );

	// End of overriding CRViewSSelSS public virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS public pure virtual methods.

	// Called by 'DlgIndSelXXX' when user click 'Select' button.
	// param: 'pSelectedProductToFill' -> class where to save current user selection.
	virtual void FillInSelected( CDS_SelProd *pSelectedProductToFill );

	// End of overriding CRViewSSelSS public pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Protected members.
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


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelCltr protected pure virtual methods.

	// Param: 'fDowngradeActuatorFunctionality' set to 'true' if actuator with the '3 points' input signal type can work with the 'On/Off control type,
	//        and if actuator with the 'Proportional' input signal type can work with the 'On/Off' and the '3 points' control type.
	virtual bool IsActuatorFit( CDB_ElectroActuator *pclElectroActuator, CDB_ControlValve *pclSelectedControlValve, 
			bool bDowngradeActuatorFunctionality, bool bAcceptAllFailSafe );

	// End of overriding CRViewSSelCltr protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelCltr protected virtual methods.

	// Return row where is located edited control valve.
	virtual long FillControlValveRows( CDB_ControlValve *pEditedControlValve = NULL );
	virtual void GetAdapterList( CDB_ControlValve *pclSelectedControlValve, CRank *pclAdapterList, CDB_RuledTableBase **ppclRuledTable );
	virtual void GetSetAccessoryList( CDB_ControlValve *pclSelectedControlValve, CDB_Actuator *pclSeletedActuator, CRank *pclSetAccessoryList, CDB_RuledTableBase **ppclRuledTable );
	virtual void RemoveSheetDescriptions( UINT uiFromSheetDescriptionID );
	virtual void FillOtherProducts( CSelectedValve *pSelectedTAP );
	virtual void SuggestOtherProducts( CDS_SSelCtrl *pSelectedControlValve );
	// End of overriding CRViewSSelCltr protected virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Private members.
private:
	long _SetRemarksKTH( CSheetDescription *pclSheetDescriptionPICv, long lRow );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Shutoff valve.

	// Called by 'ClickProduct' when user clicks on a shutoff valve.
	bool _ClickOnShutoffValve( CSheetDescription *pclSheetDescriptionSv, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );
	
	// Allow to fill shutoff valve sheet when user clicks 'suggest'.
	// Param: 'pEditedSV' is passed as argument when we are in edition mode.
	void _FillShutoffValveRows( CSelectedValve *pSelectedTAPDpCBCV );

	// Allow '_InitAndFillSVHeader' to init sheet and fill header.
	void _InitAndFillSVHeader( CSheetDescription *pclSheetDescriptionSV, CSSheet *pclSSheet );

	// Allow '_FillOneSVRow' to fill one row.
	// Param: 'pSelectedTAP' -> Shutoff valve to add in the list.
	// Param: 'lRow' -> row where to add shutoff valve.
	long _FillOneSVRow( CSheetDescription *pclSheetDescriptionSV, CSSheet *pclSSheet, CSelectedValve *pSelectedTAP, long lRow );

	// Allow to fill accessory sheet when user has chosen a shutoff valve.
	void _FillSVAccessoryRows( );

	// Allow to retrieve the current shut-off valve selected.
	CDB_ShutoffValve *_GetCurrentShutoffValveSelected( void );

	// Allow to set the new cell description containing current shut-off valve selected.
	void _SetCurrentShutoffValveSelected( CCellDescriptionProduct *pclCDCurrentShutoffValveSelected );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to retrieve row number where is the shutoff valve.
	long _GetRowOfEditedSV( CSheetDescription *pclSheetDescriptionSV, CDB_TAProduct *pEditedTAP );

	void _ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );
	void _WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );

// Private variables.
private:
	CIndSelDpCBCVParams *m_pclIndSelDpCBCVParams;

	CDlgInfoSSelDpCBCV *m_pclDlgInfoSSelDpCBCV;
	CDB_RuledTableBase *m_pclAdapterRuledTable;
	long m_lSVSelectedRow;

	vecCDCAccessoryList m_vecSvAccessoryList;
	CCDButtonExpandCollapseRows *m_pCDBExpandCollapseRowsSv;
	CCDButtonExpandCollapseGroup *m_pCDBExpandCollapseGroupSvAccessory;
	CCDButtonShowAllPriorities *m_pCDBShowAllPrioritiesSv;
};

extern CRViewSSelDpCBCV *pRViewSSelDpCBCV;
