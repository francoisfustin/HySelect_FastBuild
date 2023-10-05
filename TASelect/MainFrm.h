// MainFrm.h : interface de la classe CMainFrame
//

#pragma once

#include "TASelect.h"
#include "MFCRibbonBarEx.h"
#include "MFCRibbonGalleryEx.h"
#include "MFCRibbonEditEx.h"
#include "DlgLTtabctrl.h"
#include "DlgOutput.h"
#include "DlgHMCompilationOutput.h"
#include "ToolsDockablePane.h"
#include "TestRunnerDlg.h"
#include "SelectPM.h"

#ifdef _DEBUG
#include "DlgSpecAct.h"
#endif

extern CRITICAL_SECTION CriticalSection; 

class CMFCRibbonComboBoxEx : public CMFCRibbonComboBox
{
	CArray<CString> m_ArString;
public:
	CMFCRibbonComboBoxEx( UINT nID, BOOL bHasEditBox = TRUE, int nWidth = -1, LPCTSTR lpszLabel = NULL, int nImage = -1) :
			 CMFCRibbonComboBox( nID, bHasEditBox, nWidth, lpszLabel, nImage) {}

	INT_PTR AddItemEx( LPCTSTR lpszItem, LPCTSTR lpszItemID );
	int FindItemID( LPCTSTR lpszItemID );
	CString GetItemID( int iIndex );
	int GetItemIndex( LPCTSTR lpszItem );
	void RemoveAllItems( void );
	BOOL DeleteItem( int iIndex );
	BOOL DeleteItem( DWORD_PTR dwData );
	BOOL DeleteItem( LPCTSTR lpszText );
	
	//DWORD_PTR GetItemData(int iIndex) const { return NULL; }

	CMFCRibbonComboBoxEx& operator = ( CMFCRibbonComboBoxEx& ComboBox );
};

/*!
CMainFrame Manage Ribbon Bar and all displayed views
CMainFrame::InitViews for loading differents views
*/
class CData;
class CDlgDiversityFactor;
class CDlgPrint;
class CMainFrame : public CFrameWndEx
{
protected:
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
	typedef enum RightViewList
	{
		eUndefined					= -1,
		eRVFirst					= 0,
		eActiveView					= eRVFirst,
		eRVInfo						= 1,
		eRVGeneral					= 2,
		eRVHMSumm					= 3,
		eRVProj						= 4,
		eRVHMCalc					= 5,
		eRVLDSumm					= 6,
		eRVLogData					= 7,
		eRVChart					= 8,
		eRVSelProd					= 9,
		eRVSSelBv					= 10,
		eRVSSelDpC					= 11,
		eRVSSelTrv					= 12,
		eRVSSelCv					= 13,
		eRVSSelPICv					= 14,
		eRVSSelBCv					= 15,
		eRVSSelHub					= 16,
		eRVQuickMeas				= 17,
		eRVSSelSeparator			= 18,
		eRVSSelPresureMaintenance	= 19,
		eRVStartPage				= 20,
		eRVSSelDpCBCV               = 21,
		eRVSSelSv					= 22,
		eRVWizardSelPM				= 23,
		eRVSSelSafetyValve			= 24,
		eRVSSel6WayValve			= 25,
		eRVSSelSmartControlValve	= 26,
		eRVSSelSmartDpController	= 27,   // HYS-1938: TA-Smart Dp - 05 - Individual selection: right view
		eRVLast                     = eRVSSelSmartDpController
	};

	enum eRibbonMenu
	{
		eRbnMNew = 0,
		eRbnMOpen,
		eRbnMSave,
		eRbnMSaveAs,
		eRbnMPrint,
		eRbnMPrintSetup,
		eRbnMPrintPreview,
		eRbnMExport,
		eRbnMExportExcel,
		eRbnMExportExcelOneSheet = eRbnMExportExcel,
		eRbnMExportHMToJsonFile,
		eRbnMExportChart,
		eRbnMExportLDList,
		eRbnMExportQM,
		eRbnMExportTADiagnostic = eRbnMExportQM,
		eRbnMExportPipes,
		eRbnMImport,
		eRbnMImportPipes,
		eRbnMImportHM,
		eRbnMImportHMFromJsonFile,
		eRbnMExit,
		eRbnMStartPage,
		eRbnMPMWizard,
		eRbnMLast
	};

	enum eRibbonCategory
	{
		eRbnHomeCat,
		eRbnProjectCat,
		eRbnToolsCat,
		eRbnCommunicationCat,
		eRbnSSelectionCat,
		eRbnLogDataCat,
		eRbnLastCat
	};

	enum eStripHome
	{
		eSHPanelDefUnits,
		eSHPanelPipeSeries,
		eSHPanelTechParam,
		eSHDevSizeAb,
		eSHDevSizeBl,
		eSHPdropTarg,
		eSHPdropMax,
		eSHWVelTarg,
		eSHWVelMax,
		eSHPanelOptions,
		eSHLanguageDoc,
		eSHUserRef,
		eSHPanelInfo,
		eSHReleaseNotes,
		eSHTip,
		eSHAbout,
		eSHManual,
		eSHTest,
		eSHTestUnit,
		eSHTestSpecialAcions,
		eSHLast
	};

	enum eStripProject
	{
		eSPPanelProjInfo,
		eSPHMCalc,
		eSPPanelProjParam,
		eSPSafetyFactor,
		eSPPanelProjTools,
		eSPImportHM,
		eSPSearchReplace,
		eSPDiversityFactor,
		eSPPanelWc,
		eSPTemp,
		eSPMeasPeriod,
		eSPStart,
		eSPEnd,
		eSPChangePeriod,
		eSPProjectType,
		eSPProjectCheck,
		eSPLast
	};

	enum eStripTools
	{
		eSTPanelHydronic,
		eSTViscCorr,
		eSTPdrop,
		eSTKvCalc,
		eSTCalculator,
		eSTPanelOther,
		eSTUnitConv,
		eSTTALink,
		eSTLast
	};

	enum eStripComm
	{
		eSCPanelDataTrans,
		eSCScopeComm,
		eSCCbiComm,
		eSCPanelUpdate,
		eSCProdDataBase,
		eSCProperties,
		eSCPanelDpSVisio,
		eSCDpSVisioComm,
		eSCLast
	};

	enum eStripLoggedData
	{
		eSLDPanelCurves,
		eSLDPanelZoom,
		eSLDZoomIn,
		eSLDZoomOut,
		eSLDUndoZoom,
		eSLD2DZoom,
		eSLDPanelChartOptions,
		eSLDTracking,
		eSLDMarkers,
		eSLDLegend,
		eSLDHorizAxis,
		eSLDExportChart,
		eSLDLast
	};

	enum PMWQSelectPrefsChangeOrigin
	{
		ChangeFromRibbonOneDevice,
		ChangeFromRibbonRedundancy,			// To delete after the combo pressure maintenance redundancy is removed from the ribbon.
		ChangeFromRibbonRedPumpComp,
		ChangeFromRibbonRedTecBox,
		ChangeFromRibbonIntBufferVessel,
		ChangeFromRibbonIntCoating,
		ChangeFromRibbonExternalAir,
		ChangeFromRibbonInsVaporSealed,
		ChangeFromRibbonBreakTank,
		ChangeFromRibbonDutyStdBy,
		ChangeFromDialog
	};

public:
	virtual ~CMainFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
	virtual BOOL OnShowPopupMenu( CMFCPopupMenu* pMenuPopup );

	void SetControlPane( bool fEnable = true ) { m_bViewControlPane = fEnable; }
	CRITICAL_SECTION *GetpCriticalSection() { return &CriticalSection; }
	
	//! View initialization
	void InitViews();
	
	/*! 
		View activation, return null ptr if fail.
		Category activation
	*/
	CView *ActiveFormView( RightViewList eRightView );
	RightViewList GetCurrentRightView( void );
	
	CView *GetSpecificView( RightViewList eRightView );
	
	/*!
		Get and Set pipe combo box
		If PipeSeries is Null return to the default pipe series
		If PipeSize is not fixed CData* is null, 
	*/
	void FillPipeSeriesCB();
	void FillProductSelPipeSeriesCB();
	void FillProductSelPipeSizeCB();
	void SetRibbonComboSelPipeSeries( CString strPipeSeriesID );
	void SetRibbonComboSelPipeSize( CString strPipeSizeID );
	void EnableRibbonComboSelPipeSeries( bool fEnable );
	void EnableRibbonComboSelPipeSize( bool fEnable );
	bool VerifyEditTemp( CDS_WaterCharacteristic* pWC, CMFCRibbonEditEx *pRbnEditEx, double dCurrentSITemp, bool fDisplayWarning = true );

	void SetUpperDateTime( COleDateTime time ) { m_tCurrentUpperTimeLimits = time; }
	void SetLowerDateTime( COleDateTime time ) { m_tCurrentLowerTimeLimits = time; }
	void SetMaxUpperDateTime( COleDateTime time ) { m_tMaxUpperTimeLimits = time; }
	void SetMaxLowerDateTime( COleDateTime time ) { m_tMaxLowerTimeLimits = time; }
	
	COleDateTime GetUpperDateTime() { return m_tCurrentUpperTimeLimits; }
	COleDateTime GetLowerDateTime() { return m_tCurrentLowerTimeLimits; }
	COleDateTime GetMaxUpperDateTime() { return m_tMaxUpperTimeLimits; }
	COleDateTime GetMaxLowerDateTime() { return m_tMaxLowerTimeLimits; }

	void SetMeasurementExist( bool bExist ) { m_bMeasExist = bExist; }
	// Reset the Upper and Lower limits for measurements.
	void ResetMeasurementsLimits();
	
	CMFCStatusBar *GetStatusBar() { return &m_wndStatusBar; }
	CMFCRibbonCategory *GetRibbonProjectCategory() { return m_pProjectCategory; }
	CMFCRibbonCategory *GetRibbonToolsCategory() { return m_pToolsCategory; }
	CMFCRibbonCategory *GetRibbonCommunicationCategory() { return m_pCommunicationCategory; }
	CMFCRibbonGalleryEx *GetRibbonSSelGallery() { return m_pGallery; }
	CString GetRibbonComboSelPipeSeriesID();
	CString GetRibbonComboSelPipeSizeID();
//	CMFCRibbonEditEx *GetRibbonProjSafetyFactor() { return m_pEBSafetyFactorProj; }
	CMFCRibbonEdit *GetStartLimitProj() { return m_pEBStartLimitProj; }
	CMFCRibbonEdit *GetEndLimitProj() { return m_pEBEndLimitProj; }
	CMFCRibbonCheckBox *GetRibbonCBLDT1() { return m_pCBLDT1; }
	CMFCRibbonCheckBox *GetRibbonCBLDT2() { return m_pCBLDT2; }

	void InitializeRibbon();
	void InitializeRibbonRichEditBoxes();
	void InitializeRibbonRichEditBoxesHome();
	void InitializeRibbonRichEditBoxesProject();
	void InitializeRibbonRichEditBoxesSSel();
	
	// Update the status bar text on the current CmdUI text and enable the Menu ID depending on 
	// the m_ArResIDMenuToGray variable.
	// If 'strToolTip' is not empty, we take it in place of the text related to the ID.
	void UpdateMenuToolTip( CCmdUI *pCmdUI, CString strToolTip = _T("") );
	
	// Add into a CArray the ID resource to gray.
	// Do nothing if the ResID already exist in the CArray.
	// If the uiResID = NULL All uiResID of the Menu will be active!
	void EnableMenuResID( UINT uiResID );
	
	// Remove from the CArray the ID resource to gray
	// Do nothing if the ResID didn't exist in the CArray
	void DisableMenuResID( UINT uiResID );
	
	// Function used to manage uiResID like a Menu ID (ID_TAB...)
	// The ResID contain the text and the ToolTip separate by a \n
	// By default this function will return the text of such an ID
	// without the ToolTip.
	// If you set the boolean to true, it will return the ToolTip instead
	CString GetSplitStringFromResID( UINT uiResID, bool fToolTip = false );

	void SetToolTipRibbonElement( CMFCRibbonBaseElement *pRbnBaseElement, UINT uiID );
	void SetToolTipRibbonElement( CMFCRibbonBaseElement *pRbnBaseElement, CString str );
	
	// Return state of HMCalc button
	bool IsHMCalcChecked( void ) { return m_fButHMCalc; }
	void SetHMCalcMode( bool fMode ) { m_fButHMCalc = fMode; }

	// Return state of the 'Freeze' button.
	bool IsFreezeChecked( void );
	void Freeze ( bool fFreeze );

	// Allow to show 'DlgDiversityFactor'.
	// Remark: It's simply to not give access to protected members 'OnRbnPTDF' from another classes.
	void ShowDlgDiversityFactor( void );
	
	// Called by 'CTASelectApp::InitInstance' to avoid problem with 'DockingManager.LoadState'.
	void CreateToolsDockablePane( void ) { _CreateToolsDockablePane(); }

	// Return: see 'DlgOutputErrorCode' for error code.
	CToolsDockablePane *GetpToolsDockablePane( void ) { return &m_ToolsDockablePane; }
	
	void ChangeRbnProjectApplicationType( ProjectType eProjectApplicationType );

	void ClickRibbonProductSelGallery( ProductSubCategory eProductSubCategory );
	void ClickRibbonProductSelectionMode( ProductSelectionMode eProductSelectionMode );
	void ChangeRbnProductSelectionApplicationType( ProjectType eProductSelectionApplicationType );
	
	// Just to not give free access to protected members.
	void ClickRibbonUpdateTASelect( void ) { OnRbnCUProdData(); }
	void UpdateRibbonWaterCharacteristic( int iForWhich ) { OnWaterChange( iForWhich ); }

	ProductSubCategory GetCurrentProductSubCategory( void ) { return m_eCurrentProductSubCategory; }

	ProductSelectionMode GetCurrentProductSelectionMode( void ) { return m_eCurrentProductSelectionMode; }
	void ChangeProductSelectionMode( ProductSelectionMode eProductSelectionMode );

	void UpdatePMWQSelectPrefsRibbon();

#ifdef _DEBUG
	bool EnableMacroRunning( bool fEnable );
	bool IsMacroRunning( bool& fIsRunning );
	bool WriteMacro( MSG* pMsg );
	// HYS-842: There is a global cache of type_info names and it is not cleared
	// This function allow to clear this cache.
	void clear_type_info_cache();
#endif

	void ExportSelPtoXLSX( CString strFn );
	// HYS-1128: Unit tests for export in one sheet
	void ExportSelPInOnetoXLSX( CString strFn );

	void PostMessageToDescendants( UINT message, WPARAM wParam = 0, LPARAM lParam = 0, BOOL bDeep = TRUE );

	void ShowStartPage( void );
	
	// HYS-1376 : To Get buttons status
	bool IsMenuHMImportEnable();
	bool IsRibbonHMImportEnable();

	bool IsHMCalcDebugPanelVisible();
	bool IsHMCalcDebugVerifyAutority();

protected:
	DECLARE_MESSAGE_MAP()

// 	afx_msg void OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized );
// 	afx_msg void OnActivateApp( BOOL bActive, DWORD dwThreadID );

	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnSizing( UINT fwSide, LPRECT pRect );

	afx_msg void OnClose();
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnEndSession( BOOL bEnding );
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	
	afx_msg void OnApplicationLook( UINT uiID );
	afx_msg void OnUpdateApplicationLook( CCmdUI *pCmdUI );
	
	afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnOpenDocument( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnExportSelPToXlsx( WPARAM wParam = 0, LPARAM lParam = 0 );
	// HYS-1128: Unit tests for export in one sheet
	afx_msg LRESULT OnExportSelPInOneToXlsx( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnDisplayMsgBox( WPARAM wParam = IDOK, LPARAM lParam = 0 );
	afx_msg void OnAppMenuPrint();					// Called when clicking on the print button in the quick access toolbar and in the application button.
	afx_msg void OnAppMenuSubPrint();				// Called when clicking in the print sub menu in the application button.
	afx_msg void OnAppMenuExportSelP();
	// HYS-436: Export all selected in one sheet 
	afx_msg void OnAppMenuExportSelInOne();
	afx_msg void OnAppMenuExportHMToJson();
	afx_msg void OnAppMenuExportChart();
	afx_msg void OnAppMenuExportLDList();
	afx_msg void OnAppMenuExportQM();
	// HYS-1605: Export TA-Diagnostic and measurements 
	afx_msg void OnAppMenuExportTADiagnoctic();
	afx_msg void OnAppMenuExportPipes();
	afx_msg void OnAppMenuImportPipes();
	afx_msg void OnAppMenuImportHM();
	afx_msg void OnAppMenuImportHMJson();
	afx_msg void OnAppMenuExit();
	afx_msg void OnAppMenuStartPage();
	afx_msg void OnAppMenuPMWizard();
	afx_msg void OnFilePrint();						// Called by print preview when clicking on the 'Print' button in the ribbon.
	
	afx_msg void OnRbnHDefUnits();					// Home\Default Units													-> 'Default units' expand button.
	afx_msg void OnRbnHDUFlow();					// Home\Default Units\Flow												-> 'Flow' combo.
	afx_msg void OnRbnHDUDiffPres();				// Home\Default Units\Differential Pressure								-> 'Differential pressure' combo.
	afx_msg void OnRbnHDUTemp();					// Home\Default Units\Temperature										-> 'Temperature' combo.
	afx_msg void OnRbnHPipeSeries();				// Home\Pipe Series														-> 'Pipe series' expand button.
	afx_msg void OnRbnHPSDefPipeSeries();			// Home\Pipe Series\Default Pipe Series									-> 'Default pipe series' combo.
	afx_msg void OnRbnHTechParam();					// Home\Technical Parameters											-> 'Technical parameters' expand button.
	afx_msg void OnRbnHTPDSSizeAbv();				// Home\Technical Parameters\Device Size\Size Above						-> 'Size above' spin button.
	afx_msg void OnRbnHTPDSSizeBLW();				// Home\Technical Parameters\Device Size\Size Below						-> 'Size below' spin button.
	afx_msg void OnRbnHTPPDTarget();				// Home\Technical Parameters\Pipe Pressure Drop\Target					-> 'Target' edit field.
	afx_msg void OnRbnHTPPDMax();					// Home\Technical Parameters\Pipe Pressure Drop\Max						-> 'Max' edit field.
	afx_msg void OnRbnHTPWVTarget();				// Home\Technical Parameters\Pipe Water Velocity\Target					-> 'Target' edit field.
	afx_msg void OnRbnHTPWVMax();					// Home\Technical Parameters\Pipe Water Velocity\Max					-> 'Max' edit field.
	afx_msg void OnRbnHOLang();						// Home\Options\Language												-> 'Language' combo.
	afx_msg void OnRbnHODocs();						// Home\Options\Docs													-> 'Product documentation' button.
	afx_msg void OnRbnHOAutoSav();					// Home\Options\Auto Save												-> 'Autosave recovery files' check box.
	afx_msg void OnRbnHOUserRef();					// Home\Options\User References											-> 'User references' button.
	afx_msg void OnRbnHINotes();					// Home\Information\Notes												-> 'Release notes' button.
	afx_msg void OnRbnHITipOfTheDay();				// Home\Information\Tip Of The Day										-> 'Tip of the day' button.
	afx_msg void OnRbnHIAbout();					// Home\Information\About												-> 'About HySelect' button.
	
	afx_msg void OnRbnPProjInfo();					// Project\Project Info													-> 'Project info' expand button.
	afx_msg void OnRbnPPIProjName();				// Project\Project Info\Project Name 									-> 'Project name' edit field.
	afx_msg void OnRbnPPICBICompatible();			// Project\Project Info\CBI Compatible									-> 'TA-CBI compatibility' check box.
	afx_msg void OnRbnPPIHMCalc();					// Project\Project Info\HM Calc											-> 'Hydonic circuit calculation' button.
	afx_msg void OnRbnPPPFreeze();					// Project\Project Parameters\Freeze									-> 'Freeze hydronic circuit calculation' check box.
	afx_msg void OnRbnPPPProjectType();				// Project\Project Parameters\Project Type								-> Project type Heating or cooulin combo box.
	afx_msg void OnRbnPPPHNAutoCheck();
// Remark: For Project\Tools\Import hydronic circuits we call 'OnAppMenuImportHM'.
	afx_msg void OnRbnPrjLaunchHNCheck();
	afx_msg void OnRbnPTSR();						// Project\Tools\Search and Replace										-> 'Search and Replace' button.
	afx_msg void OnRbnPTDF();						// Project\Tools\Diversity Factor										-> 'Diversity Factor' button.
	afx_msg void OnRbnPWaterChar();					// Project\Water Characteristic											-> 'Fluid Characteristic' expand button.
	afx_msg void OnRbnPWCTemp();					// Project\Water Characteristic\Temperature								-> 'Temperature' edit field.
	afx_msg void OnRbnPWCDT();						// Project\Water Characteristic\DT										-> 'DT' edit field.
	afx_msg void OnRbnPMPDateTime();				// Project\Measurement Period\Data Time									-> 'Choose period' button.
	afx_msg void OnRbnPDebugVerityAutority();		// Project\Debug\VerifyAutority											-> 'Verify autority' check box.

	afx_msg void OnRbnTHViscCor();					// Tools\Hydronic\Viscosity Correction									-> 'Viscosity correction' button.
	afx_msg void OnRbnTHPipePressDrop();			// Tools\Hydronic\Pipe Pressure Drop									-> 'Pipe pressure drop' button.
	afx_msg void OnRbnTHKvCalc();					// Tools\Hydronic\Kv Calculation										-> 'Kv values calculation' button.
	afx_msg void OnRbnTHCalculator();				// Tools\Hydronic\Calculator											-> 'Hydronic calculator' button.
	afx_msg void OnRbnTOUnitConv();					// Tools\Other\Unit Conversion											-> 'Unit conversion' button.
	afx_msg void OnRbnTOTALink();					// Tools\Other\TALink													-> 'TA Link' button.

	afx_msg void OnRbnCDTScopeComm();				// Communication\Data Transfer\Scope Communication						-> 'TA-SCOPE communication' button.
	afx_msg void OnRbnCDTCbiComm();					// Communication\Data Transfer\CBI Communication						-> 'TA-CBI communication' button.
	afx_msg void OnRbnCDTDpSVisioComm();			// Communication\Data Transfer\DpS-Visio Communication					-> 'DpS-Visio communication' button.
	afx_msg void OnRbnCUProdData();					// Communication\Update\Product Data									-> 'Update HySelect' button.
	afx_msg void OnRbnCUProperties();				// Communication\Update\Properties										-> 'Properties' button.
	afx_msg void OnRbnCUMaintainTAScope();			// Communication\Update\Maintain TAScope								-> 'TA-SCOPE update monitoring' check box.
	afx_msg void OnRbnCUTestHySelectUpdate();		// Communication\Update\Test HySelect update							-> 'Test HySelect update' button.

	afx_msg void OnRbnPSPTGallery();				// Product selection\Product Type\Gallery -> 'Product type' selection gallery.
	afx_msg void OnRbnPSSelectionModeIndividual();	// Product selection\Product\Selection\Selection mode\Individual -> 'Indivudal' button.
	afx_msg void OnRbnPSSelectionModeBatch();		// Product selection\Product\Selection\Selection mode\Batch -> 'Batch' button.
	afx_msg void OnRbnPSSelectionModeWizard();		// Product selection\Product\Selection\Selection mode\Wizard -> 'Wizard' button.
	afx_msg void OnRbnPSSelectionModeDirect();		// Product selection\Product\Selection\Selection mode\Direct -> 'Direct' button.
	afx_msg void OnRbnPSPPipeSeries();				// Product selection\Technical parameters\Pipes\Pipes Series -> 'Pipe series' combo.
	afx_msg void OnRbnPSPPipeSize();				// Product selection\Technical parameters\Pipes\Pipes Size -> 'Pipe size' combo.
	afx_msg void OnRbnPSTPDSSizeAbv();				// Product selection\Technical parameters\Device Size\Size Above -> 'Size above' spin button.
	afx_msg void OnRbnPSTPDSSizeBLW();				// Product selection\Technical parameters\Device Size\Size Below -> 'Size below' spin button.
	afx_msg void OnRbnPSTPPDTarget();				// Product selection\Technical parameters\Pipe pressure Drop\Target	-> 'Max' edit field.
	afx_msg void OnRbnPSTPPDMax();					// Product selection\Technical parameters\Pipe pressure Drop\Max -> 'Max' edit field.
	afx_msg void OnRbnPSTPWVTarget();				// Product selection\Technical parameters\Pipe Water Velocity\Target -> 'Target' edit field.
	afx_msg void OnRbnPSTPWVMax();					// Product selection\Technical parameters\Pipe Water Velocity\Max -> 'Max' edit field.
	afx_msg void OnRbnPSPMWQPrefsCombined();		// Product selection\Product\PM & WQ selection preferences\General - Combined function in one device.
	afx_msg void OnRbnPSPMWQPrefsRedPumpComp();		// Product selection\Product\PM & WQ selection preferences\General - Redundancy Pump/Compressor.
	afx_msg void OnRbnPSPMWQPrefsRedTecBox();		// Product selection\Product\PM & WQ selection preferences\General - Redundancy TecBox.
	afx_msg void OnRbnPSPMWQPrefsIntCoating();		// Product selection\Product\PM & WQ selection preferences\Compressor - Expansion vessel with internal coating.
	afx_msg void OnRbnPSPMWQPrefsExtAir();			// Product selection\Product\PM & WQ selection preferences\Compressor - With external compressed air.
	afx_msg void OnRbnPSPMWQPrefsInsVapSealed();	// Product selection\Product\PM & WQ selection preferences\Pump & Degassing - Cooling insulation with condensation water protection.
	afx_msg void OnRbnPSPMWQPrefsBreakTank();		// Product selection\Product\PM & WQ selection preferences\Water make-up - Break tank.
	afx_msg void OnRbnPSPMWQPrefsDtyStdBy();		// Product selection\Product\PM & WQ selection preferences\Water make-up - Duty and stand-by of make-up pumps.
	afx_msg void OnRbnPSAppplicationType();			// Product selection\Product\Application Type -> Application type Heating, cooling or solar combo box.
	afx_msg void OnRbnPSWaterChar();				// Product selection\Product\Water Characteristic -> 'Fluid Characteristic' expand button.
	afx_msg void OnRbnPSWCDT();						// Product selection\Product\Water Characteristic\DT -> 'DT' edit field.
	afx_msg void OnRbnPSWCTemp();					// Product selection\Product\Water Characteristic\Temperature -> 'Temperature' edit field.
	afx_msg void OnRbnPSToolsSR();					// Product selection\Product\Tools\Search & Replace -> 'Search & replace' button.
	afx_msg void OnRbnPSPMWQOneDevice();			// Product selection\Tech. parameters\PM & WQ selection preferences\Combined functions in one device
	afx_msg void OnRbnPSPMWQComboRedundancy();		// Product selection\Tech. parameters\PM & WQ selection preferences\Redundancy
	afx_msg void OnRbnPSPMWQSelectPrefs();			// Product selection\Tech. parameters\PM & WQ selection preferences
	afx_msg void OnRbnPSPMWQNbreMaxOfAdditionalVessels();		// Product selection\Tech. parameters\PM & WQ Technical parameters number of max additional vessels in parallel.
	afx_msg void OnRbnPSPMWQNbreMaxOfVento();		// Product selection\Tech. parameters\PM & WQ Technical parameters number of max Vento in parallel.

	afx_msg void OnRbnUsermanual();
	afx_msg void OnUpdateUsermanual( CCmdUI *pCmdUI );

	afx_msg void OnUpdateFilePrint( CCmdUI *pCmdUI );
	afx_msg void OnUpdateFileSubPrint( CCmdUI *pCmdUI );
	afx_msg void OnUpdateExport( CCmdUI *pCmdUI );
	afx_msg void OnUpdateFileExportSelp( CCmdUI *pCmdUI );
	afx_msg void OnUpdateFileExportChart( CCmdUI *pCmdUI );
	afx_msg void OnUpdateFileExportLdlist( CCmdUI *pCmdUI );
	afx_msg void OnUpdateFileExportQM( CCmdUI *pCmdUI );
	// HYS-1605: Export TA-Diagnostic and measurements 
	afx_msg void OnUpdateFileExportTADiagnostic( CCmdUI *pCmdUI );
	afx_msg void OnUpdateImport( CCmdUI *pCmdUI );
	afx_msg void OnUpdateMenuImportHM( CCmdUI *pCmdUI );			// HYS-1359: Import via menu
	afx_msg void OnUpdateMenuImportHMJson( CCmdUI *pCmdUI );

	afx_msg void OnUpdateLanguage( CCmdUI *pCmdUI );				// Home\Options\Language								-> 'Language' combo.
	afx_msg void OnUpdateActiveAutoSaving( CCmdUI *pCmdUI );		// Home\Options\Auto Save								-> 'Autosave recovery files' check box.

	afx_msg void OnUpdateCBICompatible( CCmdUI *pCmdUI );			// Project\Project Info\CBI Compatible					-> 'TA-CBI compatibility' check box.
	afx_msg void OnUpdateBtnHMCalc( CCmdUI *pCmdUI );				// Project\Project Info\HM Calc							-> 'Hydonic circuit calculation' button.
	afx_msg void OnUpdateFreezeHMCalc( CCmdUI *pCmdUI );			// Project\Project Parameters\Freeze					-> 'Freeze hydronic circuit calculation' check box.
	afx_msg void OnUpdateHNAutoCheck( CCmdUI *pCmdUI );
	afx_msg void OnUpdateProjectType( CCmdUI *pCmdUI );				// Project\Project Parameters\Project Type				-> Project type (heating/cooling) combo box.
	afx_msg void OnUpdateHMCalcDebugVerifyAutorityCheck( CCmdUI *pCmdUI );		// Project\Debug\Verify autority			-> 'Verify autority' check box.

	afx_msg void OnUpdateImportHM( CCmdUI *pCmdUI );				// Project\Tools\Import hydronic circuits				-> 'Import hydronic circuits' button.
	afx_msg void OnUpdateCheckHNCalculationBtn( CCmdUI *pCmdUI );
	afx_msg void OnUpdateSearchReplace( CCmdUI *pCmdUI );			// Project\Tools\Search and Replace						-> 'Search and Replace' button.
	afx_msg void OnUpdateDiversityFactor( CCmdUI *pCmdUI );			// Project\Tools\Diversity Factor						-> 'Diversity Factor' button.
	afx_msg void OnUpdateUpperBoundaries( CCmdUI *pCmdUI );			// Project\Measurement Period\Period Start				-> 'Start' edit field.
	afx_msg void OnUpdateLowerBoundaries( CCmdUI *pCmdUI );			// Project\Measurement Period\Period End				-> 'End' edit field.
	afx_msg void OnUpdateProjectDateTimeChange( CCmdUI *pCmdUI );	// Project\Measurement Period\Data Time					-> 'Choose period' button.
	afx_msg void OnUpdateProjectWaterChar( CCmdUI *pCmdUI );
	afx_msg void OnUpdateProjectWaterTemp( CCmdUI *pCmdUI );
	afx_msg void OnUpdateProjectWaterDT( CCmdUI *pCmdUI );
	afx_msg void OnUpdateISWaterDT( CCmdUI *pCmdUI );
	afx_msg void OnUpdateTASCOPEFILES( CCmdUI *pCmdUI );			// Communication\Data Transfer\Scope Communication		-> 'TA-SCOPE communication' button.
	afx_msg void OnUpdateUpdateCBI( CCmdUI *pCmdUI );				// Communication\Data Transfer\CBI Communication		-> 'TA-CBI communication' button.
	afx_msg void OnUpdateUpdateDpSVisio( CCmdUI *pCmdUI );			// Communication\DpS-Visio\DpS-Visio Communication		-> 'DpS-Visio communication' button.
	afx_msg void OnUpdateTestHySelectUpdate( CCmdUI *pCmdUI );			// Communication\Update\Test HySelect update		-> 'Test HySelect update' button.

	afx_msg void OnUpdateProductSelPipeseries( CCmdUI *pCmdUI );			// Product selection\Technical parameters\Pipes\Pipes Series -> 'Pipe series' combo.
	afx_msg void OnUpdateProductSelPipesize( CCmdUI *pCmdUI );				// Product selection\Technical parameters\Pipes\Pipes Size -> 'Pipe size' combo.
	afx_msg void OnUpdateProductSelSearchReplace( CCmdUI *pCmdUI );			// Product selection\Product\Tools\Search & replace -> 'Search & replace' button.
	
	afx_msg void OnUpdateProductSelIndividualSelection( CCmdUI *pCmdUI );	// Product selection\Product\Selection\SelectionMode\Individual -> 'Individual' button.
	afx_msg void OnUpdateProductSelBatchSelection( CCmdUI *pCmdUI );		// Product selection\Product\Selection\SelectionMode\Batch -> 'Batch' button.
	afx_msg void OnUpdateProductSelWizardSelection( CCmdUI *pCmdUI );		// Product selection\Product\Selection\SelectionMode\Wizard -> 'Wizard' button.
	afx_msg void OnUpdatePSApplicationType( CCmdUI *pCmdUI );               // Project\Fluid characteristics\Water char                -> 'Fluid characteristics' button.
	
	// These preferences are accessible in the 'Technical parameters' category for the pressure maintenance individual selection.
	afx_msg void OnUpdateProductSelPMWQSelectPrefs( CCmdUI *pCmdUI );			// Product selection\Technical parameters\PM & WQ selection preferences	-> The expand button
	afx_msg void OnUpdateProductSelPMWQOneDevice( CCmdUI *pCmdUI );				// Product selection\Technical parameters\PM & WQ selection preferences\Combined functions in on device	-> Checbox.
	afx_msg void OnUpdateProductSelPMWQRedundancy( CCmdUI *pCmdUI );			// Product selection\Technical parameters\PM & WQ selection preferences\Redundancy	-> Combo.

	// These preferences are accessible in the 'Product' category for the pressure maintenance individual selection.
	afx_msg void OnUpdateProductSelPMWQPrefsOneDevice( CCmdUI *pCmdUI );		// Product selection\Product\PM & WQ selection preferences\General - Combined functions in on device -> Button.
	afx_msg void OnUpdateProductSelPMWQPrefsRedPumpComp( CCmdUI *pCmdUI );		// Product selection\Product\PM & WQ selection preferences\General - Redundancy on pump/compressor -> Button.
	afx_msg void OnUpdateProductSelPMWQPrefsRedTecBox( CCmdUI *pCmdUI );		// Product selection\Product\PM & WQ selection preferences\General - Redundancy on TecBox -> Button.
	afx_msg void OnUpdateProductSelPMWQPrefsIntCoating( CCmdUI *pCmdUI );		// Product selection\Product\PM & WQ selection preferences\Compressor - Internal coating -> Button.
	afx_msg void OnUpdateProductSelPMWQPrefsExternalAir( CCmdUI *pCmdUI );		// Product selection\Product\PM & WQ selection preferences\Compressor - With external compressed air -> Button.
	afx_msg void OnUpdateProductSelPMWQPrefsCoolingInsulation( CCmdUI *pCmdUI );	// Product selection\Product\PM & WQ selection preferences\Pump & Degassing - Cooling insulation with condensation water protection -> Button.
	afx_msg void OnUpdateProductSelPMWQPrefsBreakTank( CCmdUI *pCmdUI );		// Product selection\Product\PM & WQ selection preferences\Water make-up - Break tank -> Button.
	afx_msg void OnUpdateProductSelPMWQPrefsDutyStdBy( CCmdUI *pCmdUI );		// Product selection\Product\PM & WQ selection preferences\Water make-up - Duty and stand-by of make-up pumps -> Button.

	afx_msg void OnCustomizeUserpassword();

#ifdef _DEBUG
	afx_msg void OnRbnTestsSpecialActions();							// This is the method called by accelerator key CTRL+SHIT+ALT+D (debug purpose).
	afx_msg void OnRbnTestsUnit();										// This is the method called by accelerator key CTRL+SHIT+ALT+U (Unit Tests purpose).
#endif

	afx_msg LRESULT OnPswChanged( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnProjectRefChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnUserPipeChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnDiversityFactorToClose( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnDialogPrintToClose( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnApplicationTypeChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg void OnWindowPosChanged( WINDOWPOS *lpwndpos );

#ifdef _DEBUG
	afx_msg LRESULT OnDialogDebugToClose( WPARAM wParam = 0, LPARAM lParam = 0 );
#endif
	// HYS-1376 : Called for unit test import process
	afx_msg LRESULT OnImportForUnitTest( WPARAM wParam, LPARAM lParam );

	afx_msg LPARAM OnRibbonCategoryChanged( WPARAM wParam = 0, LPARAM lParam = 0 );

// Private members.
public:
	void _CreateToolsDockablePane( void );

private:
	void _RbnTPSizeAbv( bool fFromSSel );
	void _RbnTPSizeBLW( bool fFromSSel );
	void _RbnTPPDTarget( bool fFromSSel );
	void _RbnTPPDMax( bool fFromSSel );
	void _RbnTPWVTarget( bool fFromSSel );
	void _RbnTPWVMax( bool fFromSSel );

	// Complete the gallery box with all image that can be displayed.
	void _FillProductSubCategoryGalleryBox();
	void _UpdateProductSubCategoryGalleryBox();

	void _FillRibbonUnitCB( CMFCRibbonComboBox *pCombo, ePHYSTYPE PhysType );
	void _FillRibbonLanguageCB( CMFCRibbonComboBox *pCombo );

	void _FillRibbonPMWQComboRedundancy( CMFCRibbonComboBox *pCombo );
	void _UpdateRibbonPMWQComboRedundancy( void );

	void _SetNewDateTimeValues();
	CString _GetManualFileName();

	void _PostMessageToDescendants( HWND hWnd, UINT message, WPARAM wParam = 0, LPARAM lParam = 0, BOOL bDeep = TRUE );

	// Called when user change the project application type in the ribbon (Heating/Cooling for HM Calc).
	// This method returns 'true' if the change has been well applied, otherwise it returns 'false'.
	bool _OnRbnProjectApplicationTypeChanged( bool bSendWaterChangeMessage = true );

	// Called when user change the product selection application type in the ribbon (Heating/Cooling/Solar for product selection).
	void _OnRbnProductSelectionApplicationTypeChanged( void );

	// When going to the "Product selections tools\Tech. parameters", we need to update the series combo in the ribbon with the
	// values in the CDS_XXXSelParameter.
	void _UpdatePipeCombosForProductSelection( void );

// Public variables.
public:
	CDlgLTtabctrl m_wndTabs;
	CArray	<CView *,CView *> m_arrayViews;			// Array containing all views for application.

	ProductSubCategory m_eCurrentProductSubCategory;
	ProductSelectionMode m_eCurrentProductSelectionMode;

// Protected variables.
protected:
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages m_GallerySelPressurisationAndWaterQuality;
	CMFCToolBarImages m_GallerySelBalancingAndControl;
	CMFCToolBarImages m_GallerySelThermostaticControl;
	
	CMFCStatusBar m_wndStatusBar;
	CMFCRibbonBarEx m_wndRibbonBar;

	CMFCRibbonCategory *m_pHomeCategory;
	CMFCRibbonCategory *m_pProjectCategory;
	CMFCRibbonCategory *m_pToolsCategory;
	CMFCRibbonCategory *m_pCommunicationCategory;

	// Contextual category "Product selection tools" - tab "Product".
	CMFCRibbonCategory *m_pProdSelToolsProductContextualCategory;

	// Contextual category "Product selection tools" - tab "Tech. parameters" for individual & batch selection mode except pressurisation.
	CMFCRibbonCategory *m_pProdSelToolsTechParamContextualCategory;
	
	// Contextual category "Product selection tools" - tab "Pressurisation preferences" for individual selection mode.
	CMFCRibbonCategory *m_pProdSelToolsPressurisationPrefContextualCategory;

	// Contextual category "Product selection tools" - tab "Tech. parameters" for wizard selection mode.
	CMFCRibbonCategory *m_pProdSelToolsPMWizardTechParamContextualCategory;

	CMFCRibbonCategory *m_pLogDataCategory;

	// MFC ribbon panel for 'Home' tab.
	CMFCRibbonPanel *m_pPanelHomeDefaultUnits;
	CMFCRibbonPanel *m_pPanelHomePipeSeries;
	CMFCRibbonPanel *m_pPanelHomeTechnicalParameters;
	CMFCRibbonPanel *m_pPanelHomeOptions;
	CMFCRibbonPanel *m_pPanelHomeInformation;
	CMFCRibbonPanel *m_pPanelHomeTest;
	
	// MFC ribbon panel for 'Project' tab.
	CMFCRibbonPanel *m_pPanelProjectProjectInfo;
	CMFCRibbonPanel *m_pPanelProjectParameters;
	CMFCRibbonPanel *m_pPanelProjectTools;
	CMFCRibbonPanel *m_pPanelProjectFluidCharacteristics;
	CMFCRibbonPanel *m_pPanelProjectMeasurementPeriod;
	CMFCRibbonPanel *m_pPanelProjectDebug;
	
	// MFC ribbon panel for 'Tools' tab.
	CMFCRibbonPanel *m_pPanelToolsHydronic;
	CMFCRibbonPanel *m_pPanelToolsOther;
	
	// MFC ribbon panel for 'Communication' tab.
	CMFCRibbonPanel *m_pPanelCommunicationDataTransfer;
	CMFCRibbonPanel *m_pPanelCommunicationDpSVisio;
	CMFCRibbonPanel *m_pPanelCommunicationUpdate;

	// MFC ribbon panel for 'Product selection' tab.
	CMFCRibbonPanel *m_pPanelProductSelProductType;
	CMFCRibbonPanel *m_pPanelApplicationParameters;
	CMFCRibbonPanel *m_pPanelProductSelPipe;
	CMFCRibbonPanel *m_pPanelProductSelTechnicalParameters;
	CMFCRibbonPanel *m_pPanelProductSelPMTechnicalParameters;
	CMFCRibbonPanel *m_pPanelProductSelPMWizardTechnicalParameters;
	CMFCRibbonPanel *m_pPanelProductSelFluidCharacteristics;
	CMFCRibbonPanel *m_pPanelProductSelTools;
	CMFCRibbonPanel *m_pPanelProductSelSelection;
	CMFCRibbonPanel *m_pPanelProductSelPMWQPrefs;				// In the 'Technical parameters' category.
	CMFCRibbonPanel *m_pPanelProductSelPMWQSelectionPrefs;		// In the 'Product' category.

	// MFC ribbon panel for 'Logged data' tab.
	CMFCRibbonPanel *m_pPanelLoggedDataDisplayHideCurves;
	CMFCRibbonPanel *m_pPanelLoggedDataZoom;
	CMFCRibbonPanel *m_pPanelLoggedDataChartOptions;

	CMFCRibbonGalleryEx *m_pGallery;
	
	//Technical parameters editors share same ID between Home ribbon and Product Selection Ribbon
	//All value modification (input or output) in one editor is automatically send to his brother
	CMFCRibbonEditEx *m_pEBSizeAboveHome;
	CMFCRibbonEditEx *m_pEBSizeBelowHome;
	CMFCRibbonEditEx *m_pEBPresTargetHome;
	CMFCRibbonEditEx *m_pEBPresMaxHome;
	CMFCRibbonEditEx *m_pEBVelTargetHome;
	CMFCRibbonEditEx *m_pEBVelMaxHome;

	CMFCRibbonEditEx *m_pEBSizeAboveSSel;
	CMFCRibbonEditEx *m_pEBSizeBelowSSel;
	CMFCRibbonEditEx *m_pEBPresTargetSSel;
	CMFCRibbonEditEx *m_pEBPresMaxSSel;
	CMFCRibbonEditEx *m_pEBVelTargetSSel;
	CMFCRibbonEditEx *m_pEBVelMaxSSel;

	CMFCRibbonEditEx *m_pEBMaxNbrOfAdditionalVesselsInParallelSSel;
	CMFCRibbonEditEx *m_pEBMaxNbrOfVentoSSel;
	CMFCRibbonEditEx *m_pEBMaxNbrOfVsslSSelPMWizard;
	CMFCRibbonEditEx *m_pEBMaxNbrOfVentoSSelPMWizard;

	CMFCRibbonEditEx *m_pEBProjName;
	CMFCRibbonCheckBox *m_pCBCBICompatible;
	CMFCRibbonComboBoxEx *m_pCBPipeSeries;
	CMFCRibbonComboBoxEx *m_pRibbonComboSelPipeSeries;
	CMFCRibbonComboBoxEx *m_pRibbonComboSelPipeSize;
	CMFCRibbonComboBoxEx *m_pCBProjectApplicationType;
	CMFCRibbonComboBoxEx *m_pCBProductSelectionApplicationType;
	
	CMFCRibbonLabel *m_pLBWaterCharInfo1Proj;
	CMFCRibbonLabel *m_pLBWaterCharInfo2Proj;
	CMFCRibbonEditEx *m_pEBWaterCharTempProj;
	CMFCRibbonEditEx *m_pEBWaterCharDTProj;

	CMFCRibbonLabel *m_pLBWaterCharInfo1ProductSel;
	CMFCRibbonLabel *m_pLBWaterCharInfo2SSel;
	CMFCRibbonEditEx *m_pEBWaterCharTempProductSel;
	CMFCRibbonEditEx *m_pEBWaterCharDTProductSel;
	CMFCRibbonEditEx *m_pEBWaterCharChangeOverTempProductSel;
	CMFCRibbonEditEx *m_pEBWaterCharChangeOverDTProductSel;
	CMFCRibbonCheckBox *m_pChBFreezeHMCalc;
	CMFCRibbonCheckBox *m_pChBHNAutoCheck;
	CMFCRibbonEdit *m_pEBStartLimitProj;
	CMFCRibbonEdit *m_pEBEndLimitProj;
	CMFCRibbonCheckBox *m_pChBVerifyAutority;
	CMFCRibbonCheckBox *m_pCBLDT1;
	CMFCRibbonCheckBox *m_pCBLDT2;
	CMFCRibbonComboBox *m_pPMWQSelecPrefsRedundancy;

	bool m_bViewHydronicStatus;
	bool m_bViewControlPane;
	bool m_fComboSelPipeSeriesEnabled;
	bool m_fComboSelPipeSizeEnabled;
	bool m_bMeasExist;
	bool m_fButHMCalc;

	// For debug purpose even in release mode :-)
	bool m_bHMCalcDebugPanelVisible;
	bool m_bHMCalcDebugVerifyAutority;

#ifdef _DEBUG
	CDlgSpecAct *m_pDlgDebug;
	TestRunnerModel *m_pTestRunnerModel;
	TestRunnerDlg *m_pTestRunnerDlg;
#endif

	COleDateTime m_tCurrentUpperTimeLimits;
	COleDateTime m_tCurrentLowerTimeLimits;
	COleDateTime m_tMaxUpperTimeLimits;
	COleDateTime m_tMaxLowerTimeLimits;
	CRITICAL_SECTION m_CriticalSection;

	// Variables use for context menu.
	CArray <UINT> m_ArResIDMenuToGray;

	CToolsDockablePane m_ToolsDockablePane;
	RightViewList m_eCurrentView;

	CDlgDiversityFactor *m_pDlgDiversityFactor;		// A modeless dialog box.
	UINT_PTR m_uiTimer;

	CDlgPrint *m_pDlgPrint;							// A modeless dialog box.

	bool m_fShown;						// When app is ready
	HWND m_HMainAppLostFocusNULL;		// Save the last object that has the focus when TASelect loses the focus (when user goes to other application).
	HWND m_HMainAppLostFocusDlg;		// Save the last object that has the focus when a dialog is opened.

	CSize m_clIconSizeProductSelectionGallery;
	bool m_arbSelectionModeAvailability[ProductSelectionMode::ProductSelectionMode_Last + 1][ProductSubCategory::PSC_Last + 1];

	// Tools dockable pane ID for the start page right view.
	int m_iStartPageToolsDockablePaneContextID;
	bool m_bIsMenuImportEnable; // Menu import status
	bool m_bIsRibbonImportEnable; // Ribbon import status
};

extern CMainFrame *pMainFrame;
