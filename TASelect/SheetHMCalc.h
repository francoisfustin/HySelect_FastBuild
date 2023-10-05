#pragma once


#include "HydroMod.h" 
#include "SSheet.h"
#include "RViewDescription.h"
#include "MessageManager.h"

// Set the maximum of accessories that we can display in a tooltip for the pipe.
#define HMCALC_ACCDISPMAX			6

class CDS_HydroMod;
class CDlgComboBoxHM; 
class CDlgSelectActuator;
class CSheetHMCalc : public CSSheet
{
public:
	enum SheetDescription
	{
		SUndefined = -1,
		SFirst = 1,
		SDistribution = SFirst,
		SCircuit,
		SCircuitInj,
		SMeasurement,
		STADiagnostic,
		SLast = STADiagnostic
	};

	enum RowDescription_Header
	{
		RD_Header_GroupName = 1,
		RD_Header_ButDlg,
		RD_Header_ColName,
		RD_Header_Unit,
		RD_Header_Picture,
		RD_Header_FirstCirc
	};

	enum ColumnDescription_Distribution
	{
		CD_Distribution_Sub = 1,
		CD_Distribution_Pos,
		CD_Distribution_Name,
		CD_Distribution_Desc,
		CD_Distribution_Type,
		CD_Distribution_Q,
		CD_Distribution_H,
		CD_Distribution_SupplyPicture,
		CD_Distribution_SupplyPipeSeries,
		CD_Distribution_SupplyPipeSize,
		CD_Distribution_SupplyPipeLength,
		CD_Distribution_SupplyPipeDp,
		CD_Distribution_SupplyPipeTotalQ,
		CD_Distribution_SupplyDiversityFactor,
		CD_Distribution_SupplyPipeLinDp,
		CD_Distribution_SupplyPipeVelocity,
		CD_Distribution_SupplyPipeSep,
		CD_Distribution_SupplyAccDesc,
		CD_Distribution_SupplyAccData,
		CD_Distribution_SupplyAccDp,
		CD_Distribution_ReturnPicture,
		CD_Distribution_ReturnPipeSeries,
		CD_Distribution_ReturnPipeSize,
		CD_Distribution_ReturnPipeLength,
		CD_Distribution_ReturnPipeDp,
		CD_Distribution_ReturnPipeTotalQ,
		CD_Distribution_ReturnDiversityFactor,
		CD_Distribution_ReturnPipeLinDp,
		CD_Distribution_ReturnPipeVelocity,
		CD_Distribution_ReturnPipeSep,
		CD_Distribution_ReturnAccDesc,
		CD_Distribution_ReturnAccData,
		CD_Distribution_ReturnAccDp,
		CD_Distribution_Pointer
	};

	// HYS-1676: Add Smart control valve to Sheet HM Calc
	enum ColumnDescription_Circuit
	{
		CD_Circuit_Sub = 1,
		CD_Circuit_Pos,
		CD_Circuit_Name,
		CD_Circuit_Desc,
		CD_Circuit_DescSep,
		CD_Circuit_UnitDesc,
		CD_Circuit_UnitQ,
		CD_Circuit_UnitP,
		CD_Circuit_UnitDT,
		CD_Circuit_UnitDp,
		CD_Circuit_UnitQref,
		CD_Circuit_UnitDpref,
		CD_Circuit_UnitKv,
		CD_Circuit_UnitCv,
		CD_Circuit_UnitSep,
		CD_Circuit_PumpHUser,
		CD_Circuit_PumpHMin,
		CD_Circuit_PumpSep,
		CD_Circuit_CVName,
		CD_Circuit_CVDesc,
		CD_Circuit_CVKvsMax,
		CD_Circuit_CVKvs,
		CD_Circuit_CVSet,
		CD_Circuit_CVDp,
		CD_Circuit_CVDpMin,
		CD_Circuit_CVAuth,
		CD_Circuit_CVDesignAuth,
		CD_Circuit_CVSep,
		CD_Circuit_ActuatorName,
		CD_Circuit_ActuatorPowerSupply,
		CD_Circuit_ActuatorInputSignal,
		CD_Circuit_ActuatorOutputSignal,
		CD_Circuit_ActuatorRelayType,
		CD_Circuit_ActuatorFailSafe,
		CD_Circuit_ActuatorDRP,
		CD_Circuit_ActuatorSep,
		CD_Circuit_SmartValveName,
		CD_Circuit_SmartValveDp,
		CD_Circuit_SmartValveDpMin,
		CD_Circuit_SmartValveControlMode,
		CD_Circuit_SmartValveLocation,
		CD_Circuit_SmartValveKvs,
		CD_Circuit_SmartValveMoreInfo,
		CD_Circuit_SmartValveProductSet,
		CD_Circuit_SmartValveDpSensor,
		CD_Circuit_SmartValveDpl,
		CD_Circuit_SmartValveSep,
		CD_Circuit_BvName,
		CD_Circuit_BvSet,
		CD_Circuit_BvDp,
		CD_Circuit_BvDpSig,
		CD_Circuit_BvDpMin,
		CD_Circuit_BvSep,
		CD_Circuit_SvPict,
		CD_Circuit_SvName,
		CD_Circuit_SvDp,
		CD_Circuit_SvSep,
		CD_Circuit_DpCName,
		CD_Circuit_DpCDpLr,
		CD_Circuit_DpCSet,
		CD_Circuit_DpCDp,
		CD_Circuit_DpCDpl,
		CD_Circuit_DpCDpMin,
		CD_Circuit_DpCSep,
		CD_Circuit_BvBypPict,
		CD_Circuit_BvBypName,
		CD_Circuit_BvBypFlow,
		CD_Circuit_BvBypSet,
		CD_Circuit_BvBypSep,
		CD_Circuit_PipeSerie,
		CD_Circuit_PipeSize,
		CD_Circuit_PipeL,
		CD_Circuit_PipeDp,
		CD_Circuit_PipeLinDp,
		CD_Circuit_PipeV,
		CD_Circuit_PipeSep,
		CD_Circuit_PipeAccDesc,
		CD_Circuit_PipeAccData,
		CD_Circuit_PipeAccDp,
		CD_Circuit_PipeAccSep,
		CD_Circuit_Pointer
	};

	// HYS-1202: Add UnitDesc, P, DT, Qref, Dpref, Kv and Cv columnss
	enum ColumnDescription_Circuit2WInjSecSide
	{
		CD_CircuitInjSecSide_Sub = 1,
		CD_CircuitInjSecSide_Pos,
		CD_CircuitInjSecSide_Name,
		CD_CircuitInjSecSide_Desc,
		CD_CircuitInjSecSide_DescSep,
		CD_CircuitInjSecSide_UnitDesc,
		CD_CircuitInjSecSide_Q,
		CD_CircuitInjSecSide_P,
		CD_CircuitInjSecSide_DT,
		CD_CircuitInjSecSide_Dp,
		CD_CircuitInjSecSide_Qref,
		CD_CircuitInjSecSide_Dpref,
		CD_CircuitInjSecSide_Kv,
		CD_CircuitInjSecSide_Cv,
		CD_CircuitInjSecSide_Sep,
		CD_CircuitInjSecSide_PumpH,
		CD_CircuitInjSecSide_PumpHMin,
		CD_CircuitInjSecSide_PumpHAvailable,		// HYS-1930: For auto-adaptive with decoupling bypass hydraulic circuit for example.
		CD_CircuitInjSecSide_PumpSep,
		CD_CircuitInjSecSide_TempTsp,
		CD_CircuitInjSecSide_TempTss,
		CD_CircuitInjSecSide_TempTrs,
		CD_CircuitInjSecSide_TempSep,
		CD_CircuitInjSecSide_CVName,
		CD_CircuitInjSecSide_CVDesc,
		CD_CircuitInjSecSide_CVKvsMax,
		CD_CircuitInjSecSide_CVKvs,
		CD_CircuitInjSecSide_CVSet,
		CD_CircuitInjSecSide_CVDp,
		CD_CircuitInjSecSide_CVDpMin,
		CD_CircuitInjSecSide_CVAuth,
		CD_CircuitInjSecSide_CVDesignAuth,
		CD_CircuitInjSecSide_CVSep,
		CD_CircuitInjSecSide_ActuatorName,
		CD_CircuitInjSecSide_ActuatorPowerSupply,
		CD_CircuitInjSecSide_ActuatorInputSignal,
		CD_CircuitInjSecSide_ActuatorOutputSignal,
		CD_CircuitInjSecSide_ActuatorRelayType,
		CD_CircuitInjSecSide_ActuatorFailSafe,
		CD_CircuitInjSecSide_ActuatorDRP,
		CD_CircuitInjSecSide_ActuatorSep,
		CD_CircuitInjSecSide_BvName,
		CD_CircuitInjSecSide_BvSet,
		CD_CircuitInjSecSide_BvDp,
		CD_CircuitInjSecSide_BvDpSig,
		CD_CircuitInjSecSide_BvDpMin,
		CD_CircuitInjSecSide_BvSep,
		CD_CircuitInjSecSide_PipeSerie,
		CD_CircuitInjSecSide_PipeSize,
		CD_CircuitInjSecSide_PipeL,
		CD_CircuitInjSecSide_PipeDp,
		CD_CircuitInjSecSide_PipeLinDp,
		CD_CircuitInjSecSide_PipeV,
		CD_CircuitInjSecSide_PipeSep,
		CD_CircuitInjSecSide_AccDesc,
		CD_CircuitInjSecSide_AccData,
		CD_CircuitInjSecSide_AccDp,
		CD_CircuitInjSecSide_Pointer
	};

	enum ColumnDescription_Measurement
	{
		CD_Measurement_Sub = 1,
		CD_Measurement_Pos,
		CD_Measurement_DateTime,
		CD_Measurement_Reference,
		CD_Measurement_Description,
		CD_Measurement_DescriptionSep,
		CD_Measurement_WaterChar,
		CD_Measurement_Valve,
		CD_Measurement_Presetting,
		CD_Measurement_Setting,
		CD_Measurement_Kv,
		CD_Measurement_KvSignal,
		CD_Measurement_KvSignalSep,
		CD_Measurement_Dp,
		CD_Measurement_Dpl,
		CD_Measurement_DplSep,
		CD_Measurement_MeasureDp,
		CD_Measurement_DesignDp,
		CD_Measurement_MeasureFlow,
		CD_Measurement_DesignFlow,
		CD_Measurement_FlowDeviation,
		CD_Measurement_FlowDeviationSep,
		CD_Measurement_TempHH,
		CD_Measurement_Temp1DPS,
		CD_Measurement_Temp2DPS,
		CD_Measurement_TempRef,
		CD_Measurement_DiffTemp,
		CD_Measurement_DesignDiffTemp,
		CD_Measurement_DiffTempDeviation,
		CD_Measurement_DiffTempDeviationSep,
		CD_Measurement_Power,
		CD_Measurement_DesignPower,
		CD_Measurement_PowerDeviation,
		CD_Measurement_Pointer
	};

	enum ColumnDescription_TADiagnostic
	{
		CD_TADiagnostic_Sub = 1,
		CD_TADiagnostic_Pos,
		CD_TADiagnostic_DateTime,
		CD_TADiagnostic_Reference,
		CD_TADiagnostic_Description,
		CD_TADiagnostic_DescriptionSep,
		CD_TADiagnostic_WaterChar,
		CD_TADiagnostic_Valve,
		CD_TADiagnostic_Presetting,
		CD_TADiagnostic_ComputedSetting,
		CD_TADiagnostic_Setting,
		CD_TADiagnostic_Kv,
		CD_TADiagnostic_KvSignal,
		CD_TADiagnostic_DesignFlow,
		CD_TADiagnostic_DesignFlowSep,
		CD_TADiagnostic_Dp1,
		CD_TADiagnostic_Setting1,
		CD_TADiagnostic_Dp2,
		CD_TADiagnostic_MeasFlow,
		CD_TADiagnostic_FlowDeviation,
		CD_TADiagnostic_FlowDeviationSep,
		CD_TADiagnostic_CircuitDpMeas,
		CD_TADiagnostic_CircuitDpDesign,
		CD_TADiagnostic_Picture,
		CD_TADiagnostic_DistribDpMeas,
		CD_TADiagnostic_DistribDpDesign,
		CD_TADiagnostic_Pointer
	};
	
	CSheetHMCalc();
	virtual ~CSheetHMCalc();

	BOOL Create( DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID );
	void Reset( bool bAll = false );

	// Set to 'true' when exporting recursively all the hydraulic network. In this case, first sheet will be the
	// HM summary, 2-5 the first hydromod, 6-9 the second, etc. (See in the 'CDlgLeftTabProject::OnFltMenuExportRecursive' method).
	void ReserveFirstSheet() { m_bReserverFirstSheet = true; }

	// Param: 'iTabOffset' is used when excel exporting to set the correct tab position.
	void Init( CDS_HydroMod *pHM, bool bPrinting = false, int iTabOffset = 0, bool bForExport = false );

	// Allows to tell to SheetHMCalc to save its current state (current sheet, current line, last edited cell, ...).
	void SaveState( void );

	// Allow to restore if a previous state has been saved.
	void RestoreState( void );

	// Returns the object type under the cursor or NULL if there is no object.
	CDS_HydroMod::eHMObj GetHMObj( long lColumn, long lRow );

	// Returns 'true' if there is a least one object in the column.
	// If 'peHMObjInColumn' is not NULL and object exist in the column, this variable will contain the type of the object.
	bool IsHMObjInColumn( long lColumn, CDS_HydroMod::eHMObj *peHMObjInColumn = NULL );

	CDS_HydroMod *GetpHM( long lRow );
	CDS_HydroMod *GetpParentHM() { return m_pHM; }
	CDS_HydroMod *GetSelectedHM();

	// Allow to retrieve a list of all HM that are visible.
	void GetAllHMVisible( std::vector<CDS_HydroMod *> *pvecAllHM );

	// Returns 'true' if hydromod has been well found and selected.
	bool SelectRow( CDS_HydroMod *pHM );
	void EditHMRow();	
	void DeleteHMRow();
		
	bool IsMeasurementExist( CDS_HydroMod *pHM );
	bool IsInjectionCircuitExist( CDS_HydroMod *pHM );

	// Check if at column and row there is 
	bool IsEditAccAvailable( long lColumn, long lRow, CPipes *&pPipes );
	
	// Get the current sheet position in regards to the current 'm_iTabOffset'.
	// Remark: it's typically for export recursive feature. Suppose we have for example a module (*A) with different circuits and one of the child
	//         is a sub module (*A.1). The first sheets exported for *A will be 1 (SDistribution), 2 (SCircuit), 3 (SCircuitInj), 4 (SMeasurement) and
	//         5 (STADiagnostic). For *A.1, we have to start to 6. In this case 'm_iTabOffset' is set to 1. And this method will return for example
	//         6 for the distribution sheet.
	int GetSheetNumber( SheetDescription eSheet );

	// It's the opposite one of the 'GetSheetNumber'. We give a sheet number and we have the corresponding sheet description.
	SheetDescription GetSheetDescription( int iSheetNumber );
	
	// Called by 'RViewHMCalc' when the user presses TAB or SHIT+TAB key.
	// Remark: Do nothing if we are in printing or exporting mode.
	void ChangeSheet( bool bShiftRight );

	// Allow to get hydromod that is under mouse pointer.
	// Param: 'point' is the mouse x, y in SCREEN coordinate.
	// Param: 'pclColCoord' and 'plRowCoord' will contain the column and row corresponding to the 'point'.
	// Return: pointer on the hydromod if exist or NULL.
	// Remark: returns NULL if we are in printing or exporting mode.
	CDS_HydroMod *GetHMUnderMouse( CPoint point, long *plColCoord = NULL, long *plRowCoord = NULL );

	// Allow to get CDB_TAProduct that is under mouse pointer.
	// Param: 'point' is the mouse x, y in SCREEN coordinate.
	// Return: TAProduct if exist or NULL.
	// Remark: returns NULL if we are in printing or exporting mode.
	CDB_TAProduct *GetTAProductUnderMouse( CPoint point );

	// Allow to retrieve list of all column widths of all visible sheet.
	// Remark: Do nothing if we are in printing or exporting mode.
	void GetAllColumnWidth( mapMapShortLong &mapSSheetColumnWidth );

	// Allow to set list of all column widths of all visible sheet.
	// Remark: Do nothing if we are in printing or exporting mode.
	void SetAllColumnWidth( mapMapShortLong &mapSSheetColumnWidth );

	// Allow to know if user is being typing in a text or numeric field.
	// Remark: returns 'false' if we are in printing or exporting mode.
	bool IsEditModeActive( void );
	
	// Overriding virtual public methods of 'CSSheet'.
	virtual void CloseDialogSCB( CDialogEx *pDlgSCB, bool bModified );

	// 'SheetHMCalc' will automatically gray lines that contains this list of hydromod.
	void SetFlagCut( std::vector<CDS_HydroMod*>* pvecHydromod );

	// Allow to reset all lines displayed in cut mode.
	// Returns 'true' is at least one item as been changed in 'SheetHMCalc'.
	bool ResetFlagCut( void );
	bool IsFlagCutSet( void );
	bool IsObjectInCutMode( CData *pObject );

	//////////////////////////////////////////////////////////////////////////////////
	// Spread Event are intercepted by the sheet's Owner (CRViewHMCalc)
	void ComboDropDown( long lColumn, long lRow );
	void DragDropBlock( long lOldColumnFrom, long lOldRowFrom, long lOldColumnTo, long lOldRowTo, long lNewColumFrom, long lNewRowFrom, long lNewColumnTo, long lNewRowTo, BOOL *pbCancel );
	void EditChange( long lColumn, long lRow );
	void LeaveCell( long lOldColumn, long lOldRow, long lNewColumn, long lNewRow, BOOL *pbCancel );
	void PrepareforExport( void );
	void SheetChanged( short nOldSheet, short nNewSheet );
	void TextTipFetch( long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, TCHAR *pstrTipText, BOOL *pbShowTip );
	void TopLeftChangeFpspread( long lNewLeft, long lNewTop );
	//
	//////////////////////////////////////////////////////////////////////////////////

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	// 'MessageManager' message handler.
	afx_msg LRESULT MM_OnMessageReceive( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT ComboSelChangeFpspread( WPARAM wParam, LPARAM lParam );

// Private methods
private:
	void _OnMouseMove( long lColumn, long lRow );
	void _OnLButtonUp( long lColumn, long lRow );
	void _OnCellClicked( long lColumn, long lRow );
	void _OnCellDblClick( long lColumn, long lRow );
	void _OnRightClick( long lColumn, long lRow );

	void _InitDistributionSheet( CDS_HydroMod *pHM );
	void _InitCircuitsSheet( CDS_HydroMod *pHM );
	void _InitCircuitInjSheet( CDS_HydroMod *pHM );
	void _InitMeasurementSheet( CDS_HydroMod *pHM );
	void _InitTADiagnosticSheet( CDS_HydroMod *pHM );

	void _FillDistributionSheet();
	void _FillCircuitsSheet();
	void _FillCircuitInjSheet();
	void _FillMeasurementSheet();
	void _FillTADiagnosticSheet();

	// Called when cell content change.
	bool _SaveAndCompute( long lColumn, long lRow, bool bComputeAll = true );

	// Returns 'true' if drag & drop operation is available for this cell.
	// 'Available' just means that the cell contains value that can be drag & drop on other cells.
	bool _IsDragDropAvailable( long lColumn, long lRow );

	// Returns 'true' if drag & drop operation is possible for this cell.
	// 'Possible' means that we can drag the value in the cell at position lColumn & lRow.
	bool _IsDragDropPossible( CDS_HydroMod *pclHMSource, CDS_HydroMod *pclHMDest, long lColumnDest, long lRowDest );

	// Allow to manage drag & drop.
	// Returns 'true' if a change has been done.
	bool _ApplyDragAndDrop( CDS_HydroMod *pclHMSource, CDS_HydroMod *pclHMDest, long lColumnDest, long lRowDest );

	bool _SetCurrentSheet( int iSheet );

	bool _SelectRow( long lRow );

	// If column has no data and can be hide then do it.
	void _HideEmptyColumns();

	bool _ArrowKeyDown( long lFromRow );
	bool _ArrowKeyUp( long lFromRow );
	
	void _SetGroupColor( COLORREF &Color );

	void _RefreshMainCollapseExpandButton();

	// Allow to expand all groups.
	void _ExpandAll();

	// Allow to collapse all groups.
	void _CollapseAll();

	bool _CheckValidity( double dVal );

	void _SetStaticText( long lColumn, long lRow, int iTextID );
	void _SetStaticText( long lColumn, long lRow, CString strText );

	// Try to set the water char in the subtitle if all info in wc are the same.
	void _SetWaterCharSubTitle( int iColumnPos );

	// Try to set the Min, Max flow deviation in the SubTitle if possible.
	void _SetMinMaxFlowDevInSubTitle( int iColumnPos );

	// Returns 'true' if we must reset the HM tree.
	bool _SetHMName( long lRow );

	void _DrawCheckbox( long lColumn, long lRow, CDS_HydroMod::eUseProduct eState );
	void _RescanResizeColumnInfo( int iSSheetNumber );
	bool _IsRowSelectable( long lRow );
	void _HideSuccessiveSeparator( long lFromCol, long lToCol, long lRow, UINT uiSheetDescriptionID );
	void _RestoreGroupStateAndScrolling( void );

	void _AddChildrenInCutList( CDS_HydroMod *pclParentHM );
	
	// Allow to prepare foreground and background color in regards to the current hydromod is in 'cut' or 'normal' mode.
	// Returns 'true' is current line is in 'cut' mode.
	bool _PrepareColors( CDS_HydroMod *pHM );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
	// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
	// This method is called when user click on a title group (or when column width are not yet initialized).
	bool _ResetColumnWidth( UINT uiSheetDescriptionID );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void _ChangeBv( CDS_HydroMod *pHM, CDS_HydroMod::CBV *pBV, CDS_HydroMod::CBV *pSrcBv );

	// To manage params with combo box and errors.
	void _ClearAllCombos( SheetDescription eSheetDescription );
	void _ClearOneLineCombos( SheetDescription eSheetDescription, long lRow );
	void _FormatComboList( SheetDescription eSheetDescription, long lColumn, long lRow, long lStyle );
	void _AddComboBoxParam( SheetDescription eSheetDescription, long lColumn, long lRow, LPARAM lpParam );
	LPARAM _GetComboBoxParam( SheetDescription eSheetDescription, long lColumn, long lRow, int iIndex );
	void _VerifyComboBoxError();

protected:
	bool m_bInitialized;
	std::vector<long> m_arSheetGroupExpandedList[SheetDescription::SLast + 1];	// Contains columns in expand state for each sheet.
	CPoint m_arptLeftTop[SheetDescription::SLast + 1];
	bool m_bPrinting;
	bool m_bExporting;
	bool m_bReserverFirstSheet;
	long m_lMSSelectedRow;
	short m_nCurrentSheet;
	long m_lLastEditedRow;
	long m_lLastEditedCol;
	CString m_strCellText;
	double m_dCellDouble;
	bool m_bCellModified;
	LPARAM m_lpCellParam;
	bool m_bComboEditMode;

	CDlgComboBoxHM *m_pSCB;
	CDlgSelectActuator *m_pHMSelectActuator;

	int m_iTabOffSet;				// Set an offset to interpret children modules. Main module=0, the first child=1,scd=2,...

	CTable *m_pDistributionSupplyPipeTable;
	CTable *m_pDistributionReturnPipeTable;
	CTable *m_pPipeCircTab;
	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;
	CDS_HydroMod *m_pHM;
	bool m_bAtLeastOneReverseMode;

	CViewDescription m_ViewDescription;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
	// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
	std::map<int, CResizingColumnInfo*> m_mapResizingColumnInfos;
	mapMapShortLong		m_mapSSheetColumnWidth;	   
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

private:
	typedef struct _SavedState
	{
		bool m_bIsSaved;
		std::vector<CDS_HydroMod*> m_vecSelectedHM;
		short m_nCurrentSheet;
		long m_lLastEditedCol;
		long m_lLastEditedRow;
		CString m_strCellText;
		double m_dCellDouble;
		bool m_bCellModified;
		LPARAM m_lpCellParam;
		std::vector<long> m_arSheetGroupExpandedList[SheetDescription::SLast + 1];
		CPoint m_arptLeftTop[SheetDescription::SLast + 1];

		struct _SavedState()
		{
			m_bIsSaved = false;
			m_vecSelectedHM.clear();
			m_nCurrentSheet = 0;
			m_lLastEditedCol = 0;
			m_lLastEditedRow = 0;
			m_strCellText = _T("");
			m_dCellDouble = 0.0;
			m_bCellModified = false;
			m_lpCellParam = (LPARAM)0;
			for( int i = 0; i < SheetDescription::SLast + 1; i++ ) m_arSheetGroupExpandedList[i].clear();
			for( int i = 0; i < SheetDescription::SLast + 1; i++ ) m_arptLeftTop[i].SetPoint( 0, 0 );
		}
	}SavedState;
	SavedState m_rSavedState;

	std::vector<CDS_HydroMod*> m_vecCutRow;
	typedef struct _FillColors
	{
		COLORREF m_EditBkg;
		COLORREF m_DiversityBkg;
		COLORREF m_ParentModuleBkg;
		COLORREF m_ModuleCircuitBkg;
		COLORREF m_ModuleCircuitBkg2nd;			//Secondary
		COLORREF m_NormalFore;
		COLORREF m_IndexFore;
		COLORREF m_ErrorFore;
		COLORREF m_NotTAFore;
		struct _FillColors()
		{
			Reset();
		}
		void Reset()
		{
			m_EditBkg = (COLORREF)0;
			m_DiversityBkg = (COLORREF)0;
			m_ParentModuleBkg = (COLORREF)0;
			m_ModuleCircuitBkg = (COLORREF)0;
			m_ModuleCircuitBkg2nd = (COLORREF)0;
			m_NormalFore = (COLORREF)0;
			m_IndexFore = (COLORREF)0;
			m_ErrorFore = (COLORREF)0;
			m_NotTAFore = (COLORREF)0;
		}
	}FillColors;
	FillColors m_rFillColors;

	typedef struct _DragDropInfo
	{
		struct _DragDropInfo()
		{
			Reset();
			m_hHandOpen = NULL;
			m_hHandClosed = NULL;
			m_hHandForbidden = NULL;
			m_mapFrozenColsWidth.clear();
			m_mapFrozenRowsWidth.clear();
		};

		void Reset()
		{
			m_bDragDropModeEnabled = false;
			m_bDragDropRunning = false;
			m_lStartCol = 0;
			m_lStartRow = 0;
			m_rectStartCell = CRect( 0 );
			m_pHMSource = NULL;
		}

		bool m_bDragDropModeEnabled;
		bool m_bDragDropRunning;
		long m_lStartCol;
		long m_lStartRow;
		std::map<int, long> m_mapFrozenColsWidth;
		std::map<int, long> m_mapFrozenRowsWidth;
		CRect m_rectStartCell;
		CDS_HydroMod *m_pHMSource;
		HICON m_hHandOpen;
		HICON m_hHandClosed;
		HICON m_hHandForbidden;
	}DragDropInfo;
	DragDropInfo m_DragDropInfo;

	// To manage content of a TSpread combo box (There is no way in native function to set a param for each line of a combo).
	class classcomp { public: bool operator() ( const CPoint &pt1, const CPoint &pt2) const { if( pt1.x == pt2.x ) { return ( pt1.y > pt2.y ); } return ( pt1.x < pt2.x ); } };
	typedef std::vector<LPARAM> vecparam;
	typedef std::map<CPoint, vecparam, classcomp> mappointparam;

	// In sheet description, we can have a list of combo box defined by their position (CPoint).
	// And for each combo box, we can have a list or parameters links to each combo line.
	std::map<SheetDescription, mappointparam> m_mapComboBoxList;

	typedef std::pair<CPoint, vecparam> pairptvecparam;
	typedef std::pair<SheetDescription, mappointparam> pairsheetdescriptionmapptparam;
};
