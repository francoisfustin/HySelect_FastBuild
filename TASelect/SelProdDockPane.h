#pragma once


#include "HMHub.h"
class CSelProdDockPane : public CDockablePane
{
	DECLARE_DYNAMIC( CSelProdDockPane )

public:
	enum { IDD = IDD_SELPRODDOCKABLEPANE };

	CSelProdDockPane();
	virtual ~CSelProdDockPane();
	
	void SetActiveTab( CDB_PageSetup::enCheck eTabIndex );
	
	CMFCTabCtrl* GetMFCTabCtrl() { return &m_wndTabs; }
	void OnApplicationLook();

	void RedrawAll( bool fResetOrder = false, bool fPrint = false, bool fExport = false, bool fComeFromProductSelTab = false );
	bool IsInitialized() { return m_bInitialized; }

	CSelProdPageBase* GetpPage( CDB_PageSetup::enCheck ePage );
	CArticleGroupList* GetArticleList() { return &m_clArticleGroupList; }

	void SynchronizePageWithLeftTab( void );

	bool GoToPage( CDB_PageSetup::enCheck ePage );

protected:
	DECLARE_MESSAGE_MAP()
	virtual afx_msg void OnDestroy();
	virtual afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	virtual afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );
	virtual afx_msg void OnSize( UINT nType, int cx, int cy );
	virtual void OnAfterDock( CBasePane* pBar, LPCRECT lpRect, AFX_DOCK_METHOD dockMethod );
	virtual BOOL PreTranslateMessage( MSG* pMsg );

	virtual afx_msg void OnEditCopy();
	virtual afx_msg void OnEditCut();
	virtual afx_msg void OnEditPaste();
	
	virtual afx_msg void OnSSelDelete();	
	virtual afx_msg void OnSSelEdit();
	virtual afx_msg void OnSSelQuickEdit();
	virtual afx_msg void OnSSelGetTender();

	virtual afx_msg void OnSSelGetFullInfo();

	virtual afx_msg void OnUpdateEditCopy( CCmdUI* pCmdUI );
	virtual afx_msg void OnUpdateEditCut( CCmdUI* pCmdUI );
	virtual afx_msg void OnUpdateEditPaste( CCmdUI* pCmdUI );
	virtual afx_msg void OnUpdateGetTender( CCmdUI* pCmdUI );
	virtual afx_msg void OnUpdateMenuText( CCmdUI* pCmdUI );

	virtual afx_msg LRESULT OnChangeActiveTab( WPARAM wp, LPARAM lp );
	
	LRESULT OnUserSelectionChange( WPARAM wParam, LPARAM lParam );
	LRESULT OnSelProdEditProduct( WPARAM wParam, LPARAM lParam );

	// Just to allow container to call '_Reset' and '_Init'.
	void ResetInit( void ) { _Reset(); _Init(); }

// Protected variables.
protected:
	bool m_bInitialized;
	CMFCTabCtrl m_wndTabs;
	CDB_PageSetup::enCheck m_eTabIndex;
	CArray<CSelProdPageBase::HMvector> m_arHMmap;

	typedef std::map<CDB_PageSetup::enCheck, CSelProdPageBase*> mapSelProdPage;
	typedef mapSelProdPage::iterator mapSelProdPageIter;
	mapSelProdPage m_mapPageList;
	CArticleGroupList m_clArticleGroupList;
	CTADatastruct *m_pTADS;

// Private methods.
private:
	void _Reset();
	void _Init();

	void _ChangeTab( bool bShiftRight );

	// Scan the hydronic tree to extract and classify HM product. 
	void _ExtractHMInfos( CTable* pTable );

	CDB_PageSetup::enCheck _GetPageLinkedToLeftTab( void );
	CTable *_GetpTableLinkedToPage( CDB_PageSetup::enCheck ePage );

	void _DeleteTADSObj( bool fConfirm = false );

	enum CatalogType
	{
		CT_FullCat,
		CT_LightCat
	};
	void _DisplayRequestedDocSheet( CatalogType eCatalogType );

// Private variables.
private:
	struct CtxtMenuData
	{
		CtxtMenuData() { Clear(); }
		void Clear()
		{
			m_pclSSelectedValve = NULL;
			m_pclSSelectedPM = NULL;
			m_pclSSelectedActuator = NULL;
			m_pclSSelectedAccessory = NULL;
			m_pclSSelectedHUB = NULL;
			m_pclProduct = NULL;
			m_pclActuator = NULL;
		}
		CDS_SSel *m_pclSSelectedValve;
		CDS_SSelPMaint *m_pclSSelectedPM;
		CDS_Actuator *m_pclSSelectedActuator;
		CDS_Accessory *m_pclSSelectedAccessory;
		CDS_HmHub *m_pclSSelectedHUB;
		CDB_Product *m_pclProduct;
		CDB_Actuator *m_pclActuator;
		
	};
	CtxtMenuData m_rCtxtMenuData;
};

class CContainerBase : public CSelProdDockPane
{
public:
	CContainerBase( CWnd *pclParent );
	virtual ~CContainerBase();
	bool Init( void );

// Protected members.
protected:
	DECLARE_MESSAGE_MAP()
	virtual afx_msg void OnDestroy();
	bool PrepareMultiSheetInOne( bool fForPrint, bool fForExport );

protected:
	CWnd *m_pclParent;
	typedef std::map< CDB_PageSetup::enCheck, CSSheet* > mapPageSetupCSSheet;
	typedef mapPageSetupCSSheet::iterator mapPageSetupCSSheetIter;
	mapPageSetupCSSheet m_mapAllInOneSheetList;

private:
	void _ClearMap( void );
};

class CContainerForExcelExport : public CContainerBase
{
public:
	// HYS-436: For export all selected product in one sheet in Excel format
	enum RowDescription_Header
	{
		RD_Header_HMGroupName = 1,
		RD_Header_HMSubGroupName,
		RD_Header_ColName,
		RD_Header_FirstCirc
	};

	enum ColumnDescription_Primary
	{
		CD_Prim_Sub = 1,
		CD_Prim_Ref1,
		CD_Prim_Infos,
		CD_Prim_Ref2,
		CD_Prim_Product_Name,
		CD_Prim_Product_BodyMaterial,
		CD_Prim_Product_Size,
		CD_Prim_Product_Connection,
		CD_Prim_Product_Version,
		CD_Prim_Product_DpCDpLr,
		CD_Prim_Product_PnTminTmax,
		CD_Prim_Product_Location,							// HYS-1680: Smart control valve
		CD_Prim_Product_Kvs,								// Smart control valve
		CD_Prim_Product_Qnom,								// Smart control valve
		CD_Prim_Product_CableLengthToRemoteTempSensor,		// Smart control valve
		CD_Prim_Product_TASmartPowerSupply,		            // Smart control valve
		CD_Prim_Product_TASmartInputSignal,                 // Smart control valve
		CD_Prim_Product_TASmartOutputSignal,		        // Smart control valve
		CD_Prim_TechInfo_Flow,
		CD_Prim_TechInfo_PowerDt,
		CD_Prim_TechInfo_Dp,
		CD_Prim_TechInfo_ControlMode,						// HYS-1680
		CD_Prim_TechInfo_DpCDpL,
		CD_Prim_TechInfo_DpMin,
		CD_Prim_TechInfo_DpMax,
		CD_Prim_TechInfo_Setting,
		CD_Prim_TechInfo_DpCHmin,
		CD_Prim_TechInfo_DpCAuth,
		CD_Prim_ArticleNumber,
		CD_Prim_LocalArticleNumber,
		CD_Prim_Qty,
		CD_Second_Product_Name,
		CD_Second_Product_ActCloseOffDp,
		CD_Second_Product_ActActTime,
		CD_Second_Product_ActIP,
		CD_Second_Product_ActPowerSupply,
		CD_Second_Product_ActInputSignal,
		CD_Second_Product_ActOutputSignal,
		CD_Second_Product_ActRelayType,
		CD_Second_Product_ActFailSafe,
		CD_Second_Product_ActDRP,
		CD_Second_product_ActMaxTemp,
		CD_Second_Product_Size,
		CD_Second_Product_Connection,
		CD_Second_Product_Version,
		CD_Second_Product_PNTminTmax,
		CD_Second_TechInfo_Flow,
		CD_Second_TechInfo_PowerDt,
		CD_Second_TechInfo_Dp,
		CD_Second_TechInfo_Setting,
		CD_Second_ArticleNumber,
		CD_Second_LocalArticleNumber,
		CD_Second_Qty,
		CD_Mode,
		CD_Pointer
	};

	enum ValveType
	{
		CV,					
		DpC,					
		DpCBCV,
		BV,
		SV,
		Trv,
		TA6WayV,
		SmartControlValve,
		SmartDpC
	};

	// HYS-1071: For pressurisation and water quality
	enum RowDescription_Header_PMWQ
	{
		RD_Header_PMWQ_GroupName = 1,
		RD_Header_PMWQ_ColName,
		RD_Header_PMWQ_ColUnit,
		RD_Header_PMWQ_FirstCirc
	};

	enum ColumnDescription_PMWQ_Selection
	{
		CD_PMWQ_Sub = 1,
		CD_PMWQ_Ref1,
		CD_PMWQ_Ref2,
		CD_PMWQ_Qty,
		CD_PMWQ_ProductName,
		CD_PMWQ_ProductFunction,
		CD_PMWQ_ArticleNumber,
		CD_PMWQ_LocalArticleNumber,
		CD_PMWQ_Pointer
	};

	enum ColumnDescription_PMWQ_InputData
	{
		CD_PMWQ_IO_Sep = CD_PMWQ_Pointer + 1,
		CD_PMWQ_IO_AppType,
		CD_PMWQ_IO_FluidType,
		CD_PMWQ_IO_Norm,
		CD_PMWQ_IO_SystemVolume,
		CD_PMWQ_IO_SolarCollectorVolume,
		CD_PMWQ_IO_SolarCollectorMultiplierFactor,
		CD_PMWQ_IO_StorageTankVolume,
		CD_PMWQ_IO_StorageTankMaxTemp,
		CD_PMWQ_IO_InstPower,
		CD_PMWQ_IO_StatHeight,
		CD_PMWQ_IO_Pz,
		CD_PMWQ_IO_SafValvPress,
		CD_PMWQ_IO_SafTempLimit,
		CD_PMWQ_IO_MaxTemp,
		CD_PMWQ_IO_SupplyTemp,
		CD_PMWQ_IO_ReturnTemp,
		CD_PMWQ_IO_MinTemp,
		CD_PMWQ_IO_FillTemp,
		CD_PMWQ_IO_PressOn,
		CD_PMWQ_IO_PumpHead,
		CD_PMWQ_IO_MaxWidth,
		CD_PMWQ_IO_MaxHeight
	};

	enum ColumnDescription_PMWQ_OutputData
	{
		CD_PMWQ_IO_SystemExpCoeff = CD_PMWQ_IO_MaxHeight + 1,
		CD_PMWQ_IO_SystemExpVol,
		CD_PMWQ_IO_FactorX,
		CD_PMWQ_IO_SystemExpVolSWKI,
		CD_PMWQ_IO_StorageTankExpCoeff,
		CD_PMWQ_IO_StorageTankExpVolume,
		CD_PMWQ_IO_TotalExpVolume,
		CD_PMWQ_IO_SolarCollectorSecurityVolume,
		CD_PMWQ_IO_MinWResInExpansion,
		CD_PMWQ_IO_DegassingWaterReserve,
		CD_PMWQ_IO_VesselNetVolume,
		CD_PMWQ_IO_MinWResNotInExpansion,
		CD_PMWQ_IO_WaterRes,
		CD_PMWQ_IO_CtrVol,
		CD_PMWQ_IO_VapPress,
		CD_PMWQ_IO_MinPress,
		CD_PMWQ_IO_MinInitPress,
		CD_PMWQ_IO_InitPress,
		CD_PMWQ_IO_FillPress,
		CD_PMWQ_IO_TargetPress,
		CD_PMWQ_IO_FinalPress,
		CD_PMWQ_IO_SpeFlEqVol,
		CD_PMWQ_IO_NeFlEqVol,
		CD_PMWQ_IO_PressFact,
		CD_PMWQ_IO_NomVol,
		CD_PMWQ_IO_ExpPipe10,
		CD_PMWQ_IO_ExpPipe30,
		CD_PMWQ_IO_Pointer
	};

	enum ColumnDescription_PMWQ_SeparatorTechInfos
	{
		CD_PMWQ_SepTechInfo_Sep = CD_PMWQ_IO_Pointer + 1,
		CD_PMWQ_SepTechInfo_Flow,
		CD_PMWQ_SepTechInfo_PowerDt,
		CD_PMWQ_SepTechInfo_Dp
	};

	enum ColumnDescription_PMWQ_SafetyValveTechInfos
	{
		CD_PMWQ_SafValTechInfo_Sep = CD_PMWQ_SepTechInfo_Dp + 1,
		CD_PMWQ_SafValTechInfo_SetPress,
		CD_PMWQ_SafValTechInfo_HeatGenType,
		CD_PMWQ_SafValechInfo_MaxPower,
		CD_PMWQ_SafValechInfo_Power,
		CD_PMWQ_SafValechInfo_MaxCollec,
		CD_PMWQ_SafValechInfo_Collec,
		CD_PMWQ_LAST
	};

	enum RowDescription_LineSeparator
	{
		Individual_Sel,
		Direct_Sel,
		HMCalc_Sel,
		Pressurisation,
		SeparatorAirVent,
		SafetyValve
	};

	// HYS-1605: TA-Diagnostic and measurement
	enum RowDescription_Header_Meas
	{
		RD_Header_Meas_GroupName = 1,
		RD_Header_Meas_ColName,
		RD_Header_Meas_FirstCirc
	};

	enum ColumnDescription_Meas_Info
	{
		CD_Measurement_Sub = 1,
		CD_Measurement_DateTime,
		CD_Measurement_Reference,
		CD_Measurement_Description
	};

	enum ColumnDescription_Meas_Circuit
	{
		CD_Measurement_DescriptionSep = CD_Measurement_Description + 1,
		CD_Measurement_WaterChar,
		CD_Measurement_Valve,
		CD_Measurement_Presetting,
		CD_Measurement_Setting,
		CD_Measurement_Kv,
		CD_Measurement_KvSignal
	};

	enum ColumnDescription_Meas_Dp
	{
		CD_Measurement_KvSignalSep = CD_Measurement_KvSignal + 1,
		CD_Measurement_Dp,
		CD_Measurement_Dpl
	};

	enum ColumnDescription_Meas_Flow
	{
		CD_Measurement_DplSep = CD_Measurement_Dpl + 1,
		CD_Measurement_MeasureDp,
		CD_Measurement_DesignDp,
		CD_Measurement_MeasureFlow,
		CD_Measurement_DesignFlow,
		CD_Measurement_FlowDeviation
	};

	enum ColumnDescription_Meas_Temp
	{
		CD_Measurement_FlowDeviationSep = CD_Measurement_FlowDeviation + 1,
		CD_Measurement_TempHH,
		CD_Measurement_Temp1DPS,
		CD_Measurement_Temp2DPS,
		CD_Measurement_TempRef,
		CD_Measurement_DiffTemp,
		CD_Measurement_DesignDiffTemp,
		CD_Measurement_DiffTempDeviation
	};

	enum ColumnDescription_Meas_Power
	{
		CD_Measurement_DiffTempDeviationSep = CD_Measurement_DiffTempDeviation + 1,
		CD_Measurement_Power,
		CD_Measurement_DesignPower,
		CD_Measurement_PowerDeviation,
		CD_Measurement_Pointer
	};

	enum RowDescription_Header_Diag
	{
		RD_Header_Diag_GroupName = 1,
		RD_Header_Diag_ColName,
		RD_Header_Diag_FirstCirc
	};

	enum ColumnDescription_Diag_Info
	{
		CD_TADiagnostic_Sub = 1,
		CD_TADiagnostic_DateTime,
		CD_TADiagnostic_Reference,
		CD_TADiagnostic_Description
	};

	enum ColumnDescription_Diag_Circuit
	{
		CD_TADiagnostic_DescriptionSep = CD_TADiagnostic_Description + 1,
		CD_TADiagnostic_WaterChar,
		CD_TADiagnostic_Valve,
		CD_TADiagnostic_Presetting,
		CD_TADiagnostic_ComputedSetting,
		CD_TADiagnostic_Kv,
		CD_TADiagnostic_KvSignal,
		CD_TADiagnostic_DesignFlow
	};

	enum ColumnDescription_Diag_MeasData
	{
		CD_TADiagnostic_DesignFlowSep = CD_TADiagnostic_DesignFlow + 1,
		CD_TADiagnostic_Dp1,
		CD_TADiagnostic_Setting1,
		CD_TADiagnostic_Dp2,
		CD_TADiagnostic_MeasFlow,
		CD_TADiagnostic_FlowDeviation
	};

	enum ColumDescription_Diag_MeasDp
	{
		CD_TADiagnostic_FlowDeviationSep = CD_TADiagnostic_FlowDeviation + 1,
		CD_TADiagnostic_CircuitDpMeas,
		CD_TADiagnostic_CircuitDpDesign,
		CD_TADiagnostic_DistribDpMeas,
		CD_TADiagnostic_DistribDpDesign,
		CD_TADiagnostic_Pointer
	};

	CContainerForExcelExport( CWnd *pclParent );
	bool Export( CString strFileName );
	// HYS-1605: For export TA-Diagnostic and measurements
	bool ExportTADiagnostic( CString strFileName );
	void InitMeasurementSheet( CSSheet* pclSheet );
	bool InitExportMeasurements( CSSheet* pclSheet );
	void InitTADiagnosticSheet( CSSheet* pclSheet );
	bool InitExportTADiagnostic( CSSheet* pclSheet );
	void FillHMMeasurementRow( CDS_HydroMod *pHM, CSSheet *pclSheet, long *plRow );
	void FillOneHMMeasurementRow( CDS_HydroMod* pHM, CDB_TAProduct* pclProduct, CSSheet* pclSheet, long* plRow, bool bIsSecondary = false );
	void FillHMTADiagnosticRow( CDS_HydroMod *pHM, CSSheet *pclSheet, long *plRow );
	void FillOneHMTADiagnosticRow( CDS_HydroMod* pHM, CDB_TAProduct* pclProduct, CSSheet* pclSheet, int iNbChildren, long* plRow, bool bIsSecondary = false );
	// HYS-436: For export all selected product in one sheet in Excel format
	bool ExportInOneSheet( CString strFileName );
	// HYS-1605: Add lRow parameter to be generic for all export excel
	void SetOrResizeColumnWidth( CSSheet* pclSheet, long lCol, long lRow, double dMaxWidth, double *pdWidth );
	void SetAllColumnWidth( CSSheet* pclSheet, long lColStart, long lColEnd, long lRow, double dMaxWidth );
	void HideEmptyColumns( CSSheet *pclSheet, long lFirstRow, long lColStart = CD_Prim_Sub, long lColEnd = CD_Pointer );
	void InitProjectSheet( CSSheet* pclSheet );
	bool InitProjectResultExport( CSSheet* pclSheet );

	// HYS-1070: Browse pclProdTable, fill each row with an eType element 
	void FillProductSelection( CTable *pclProdTable, CSSheet *pclSheet, bool isFromDirSel, ValveType etype, long *plRow );
	void FillOneHMRowForBv( CDS_HydroMod *pHM, CDS_HydroMod::CBV *pclHMBase, CSSheet *pclSheet, CDB_RegulatingValve *pclBv, long lRow );
	void FillOneHMRow( CDS_HydroMod *pHM, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow, CString strBvPosition );
	void FillHMRow( CDS_HydroMod *pHM, CSSheet *pclSheet, long *plRow );
	void FillSelectionRow( CDS_SSel *pclSSel, CSSheet *pclSheet, ValveType etype, long *plRow );
	void FillSelectedBVRow( CDS_SSelBv *pclSSel, CSSheet *pclSheet, long *plRow );
	void FillSelectedCVRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow );
	void FillSelectedActrRow( CDS_Actuator *pclSSel, CSSheet *pclSheet, long *plRow );
	void FillSelectedDpCBCvRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow );
	void FillSelectedDpCRow( CDS_SSelDpC *pclSSel, CSSheet *pclSheet, long *plRow );
	void FillSelectedSVRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow );
	void FillSelectedTrvRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow );
	void FillSelected6WayValveRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow );
	void FillSelectedSmartControlValveRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow );
	void FillSelectedSmartDpCRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow );
	// HYS-2007: For DpSensor alone, with smart DpC in HMCalc or in selection.
	void FillSelectedDpSensorRow( CDS_SSel* pclSSel, CDB_Product* pclProduct, CSSheet* pclSheet, long* plRow );
	void FillSelectedReturnValveRow( CDS_SSel *pclSSel, CDB_TAProduct *pclProduct, CSSheet *pclSheet, long *plRow );
	void FillAccessories( CSSheet *pclSheet, long* plRow, CAccessoryList *pclAccessoryList, int iGroupQuantity, CString strInfos, CDS_SSel *pclSSel, bool bForSet = false, int iDistributedQty = 1 );
	// HYS-2007: Accessories in Dp Sensor set. 
	long FillAccessoryDpSensorSet( CSSheet* pclSheet, long lRow, CDB_Product* pAccessory, bool fWithArticleNumber, int iQuantity, CString strInfos, LPARAM lpParam = NULL );
	enum
	{
		SortType_Name,
		SortType_Key,
	};
	double FillAccessoriesHelper_GetSortTypeAndKey( CDB_Product *pclProduct, CDS_SSel *pclSSel, int &iSortType, double &dKey );
	void FillAccessoriesHelper_GetName( CDB_Product *pclProduct, CDS_SSel *pclSSel, CString &strName, CString &strDescription );

	void FillAccessoriesCtrlSet( CSSheet *pclSheet, long* plRow, CDS_SSelCtrl *pclSSelCtrl, CString strInfos, bool fForCtrl );
	void FillAccessoriesDpCBCV( CSSheet *pclSheet, long *plRow, CDS_SSelDpCBCV *pclSSelDpCBCV, CString strInfos, bool fForValve );

	// HYS-1071: For pressurisation and water quality
	void FillArtNumberCol( CSSheet* pclSheet, long lColumn, long lRow, CDB_Product* pclProduct, CString strArtNumberStr );
	void FillSetArtNumberCol( CSSheet* pclSheet, long lColumn, long lRow, CDB_Set* pclSet, CString strArtNumberStr );
	bool InitProjectPMWQExport( CSSheet* pclSheet );
	void InitPMWQProjectSheet( CSSheet* pclSheet );
	void FillSelectionPMRow( CDS_SSelPMaint *pclSSel, CSSheet *pclSheet, long *plRow );
	void FillDirSelRow( CDS_SSelPMaint * pclSSelPressMaint, CSSheet * pclSheet, long * plRow );
	void FillSelectedPlenoVentoRow( CDS_SSelPMaint * pclSSelPressMaint, CSSheet * pclSheet, long * plRow );
	void FillSelectedSeparatorRow( CDS_SSelAirVentSeparator *pclSSel, CSSheet *pclSheet, long *plRow );
	void FillSelectedSafetyValveRow( CDS_SSelSafetyValve *pclSSel, CSSheet *pclSheet, long *plRow );
	// HYS-1741: Fill accessories and services product rows
	void FillSelectedAccessServicesRow( CDS_SelPWQAccServices *pclSSel, CSSheet *pclSheet, long *plRow );
	void FillSelectedBlowTankRow( CDS_SSelSafetyValve *pclSSel, CSSheet *pclSheet, long *plRow );
	void FillPMInputDataRow( CDS_SSelPMaint *pclSSel, CSSheet *pclSheet, long lRow );
	void FillPMCalculatedDataRow( CDS_SSelPMaint *pclSSel, CSSheet *pclSheet, long lRow );
	void FillProductParametersRow( CData *pclProduct, CDS_SSelPMaint *pclSSelPM, CDS_SSelAirVentSeparator* pclSSelSep, CDS_SSelSafetyValve* pclSSelSafVal, CSSheet *pclSheet, long lRow );
	void AddLineSeparator( CSSheet *pclSheet, RowDescription_LineSeparator eLine, long *plRow );
	void FillAccessoriesPM( CSSheet * pclSheet, long *plRow, CAccessoryList * pclAccessoryList, int iGroupQuantity, int iDistributedQty );

private:
	CString m_selectionMode;
	vector<int> m_vectColumnShown{ CD_Prim_Product_Name, CD_Prim_Product_DpCDpLr, CD_Prim_TechInfo_Flow, CD_Prim_TechInfo_DpCDpL, CD_Prim_TechInfo_Setting, CD_Prim_ArticleNumber, CD_Prim_LocalArticleNumber,
		CD_Prim_Qty, CD_Second_Product_Name, CD_Second_TechInfo_Flow, CD_Second_TechInfo_Setting, CD_Second_ArticleNumber, CD_Second_LocalArticleNumber, CD_Second_Qty };

	// For pressurisation output.
	bool m_bAtLeastOneCooling;
	bool m_bAtLeastOneSolar;
	bool m_bAtLeastOneSWKINorm;
	bool m_bAtLeastOneNotSWKINorm;
	bool m_bAtLeastOneSWKINormWithStorageTank;
};

class CContainerForPrint : public CContainerBase
{
public:
	CContainerForPrint( CWnd *pclParent ) : CContainerBase( pclParent ) {};
	bool HasSomethingToPrint( CDB_PageSetup::enCheck ePage );
	bool BeginPrinting( void );
	CRect GetSheetSizeInPixels( CDB_PageSetup::enCheck ePage );
	void SetPageBreak( CDB_PageSetup::enCheck ePage, CDC *pDC, CRect rect, double dScaleRatio = 1.0 );
	void PrintPageCount( CDB_PageSetup::enCheck ePage, CDC *pDC, CRect& PrintRect, long& lPageCount );
	CSSheet* GetSheetToPrint( CDB_PageSetup::enCheck ePage );
};
