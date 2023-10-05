#pragma once



class CRCImageManager
{
public:
	typedef enum ImageListName
	{
		ILN_Undefined = -1,
		ILN_6WayValveTabCltr = 0,
		ILN_Button,
		ILN_CheckBox,
		ILN_CustomPipe,
		ILN_GroupBox,
		ILN_GroupBoxGrayed,
		ILN_HMTree,
		ILN_InfoTree,
		ILN_Print,
		ILN_ProjectTree,
		ILN_StripComm,
		ILN_StripHome,
		ILN_StripIndivSel,
		ILN_StripLoggedData,
		ILN_StripProject,
		ILN_StripSelectionCategory,
		ILN_StripTools,
		ILN_PMWQPrefs,
		ILN_Last
	};
		
	typedef enum ImageSize
	{
		IS_Undefined	= -1,
		IS_Size16		= 0,
		IS_Size32		= 1,
		IS_Size48		= 2
	};

	// 'IDB_IMGLIST_6WAYVALVETABCTRL'.
	typedef enum ImgList6WayValveTabCtrl
	{
		IL6WV_6WayValve = 0,
		IL6WV_PIBCv,
		IL6WV_Bv,
		IL6WV_Last
	};

	// 'IDB_IMGLIST_BUTTON'.
	typedef enum ImgListButton
	{
		ILB_First = 0,
		ILB_AlignLeft = ILB_First,
		ILB_AlignCenter,
		ILB_AlignRight,
		ILB_Font,
		ILB_EditHeaderFooter,
		ILB_Save,
		ILB_SavePageSetup,
		ILB_SaveRemark,
		ILB_SaveStyle,
		ILB_PrintSetup,
		ILB_Apply,
		ILB_CollapseAllTree,
		ILB_ExpandAllTree,
		ILB_SelectAll,
		ILB_UnselectAll,
		ILB_CompRad,
		ILB_Calc1,
		ILB_Calc2,
		ILB_CopyQDp,
		ILB_SaveTempCond,
		ILB_EditPrjInfo,
		ILB_SetUserRef,
		ILB_HMTree,
		ILB_CrossHMTree,
		ILB_EraseLogData,
		ILB_EraseSavedData,
		ILB_AdditRecomended,
		ILB_ModifyWater,
		ILB_TrashBin,
		ILB_InfoCannotSend,
		ILB_Copy,
		ILB_Last
	};

	// 'IDB_IMGLST_CHECKBOX1616'.
	typedef enum ImgListCheckBox
	{
		ILCB_First = 0,
		ILCB_NotSelected = ILCB_First,
		ILCB_Selected,
		ILCB_NotSelectedDisable,
		ILCB_SelectedDisable,
		ILCB_Image4,
		ILCB_Image5,
		ILCB_Dot1,
		ILCB_Dot2,
		ILCB_Last
	};

	// 'IDB_IMGLIST_CUSTPIPE'.
	typedef enum ImgListCustPipe
	{
		ILCP_First = 0,
		ILCP_LockUp = ILCP_First,
		ILCP_LockDown,
		ILCP_UnLockUp,
		ILCP_UnLockDown,
		ILCP_SetAsDefault,
		ILCP_Last
	};

	// 'IDB_IMGLIST_GROUPBOX'.
	typedef enum ImgListGroupBox
	{
		ILGB_First = 0,
		ILGB_Valve = ILGB_First,
		ILGB_DpC,
		ILGB_Q,
		ILGB_DP,
		ILGB_KV,
		ILGB_Pen,
		ILGB_Pipe,
		ILGB_Hub,
		ILGB_Rad,
		ILGB_RadCond,
		ILGB_Water,
		ILGB_H2OAddit,
		ILGB_H2OPt,		
		ILGB_Table,
		ILGB_PostIt,
		ILGB_Note,
		ILGB_Info,
		ILGB_Key,
		ILGB_Ruler1,
		ILGB_Ruler2,
		ILGB_ThermoH,
		ILGB_Actuator,
		ILGB_Separator,
		ILGB_SafetyValve,
		ILGB_Heating,
		ILGB_Cooling,
		ILGB_Last
	};

	// 'IDB_IMGLIST_GROUPBOX_GRAYED'.
	typedef enum ImgListGroupBoxGrayed
	{
		ILGBG_First = 0,
		ILGBG_Valve = ILGBG_First,
		ILGBG_DpC,
		ILGBG_Q,
		ILGBG_DP,
		ILGBG_KV,
		ILGBG_Pen,
		ILGBG_Pipe,
		ILGBG_Hub,
		ILGBG_Rad,
		ILGBG_RadCond,
		ILGBG_Water,
		ILGBG_H2OAddit,
		ILGBG_H2OPt,		
		ILGBG_Table,
		ILGBG_PostIt,
		ILGBG_Note,
		ILGBG_Info,
		ILGBG_Key,
		ILGBG_Ruler1,
		ILGBG_Ruler2,
		ILGBG_ThermoH,
		ILGBG_Actuator,
		ILGBG_Separator,
		ILGBG_SafetyValve,
		ILGBG_Heating,
		ILGBG_Cooling,
		ILGBG_Last
	};

	// 'IDB_IMGLST_HMTREE'.
	typedef enum ImgListHMTree
	{
		ILHM_First = 0,
		ILHM_HydronicNetwork = ILHM_First,
		ILHM_HydronicNetworkSelected,
		ILHM_HydronicCircuit,
		ILHM_HydronicCircuitSelected,
		ILHM_TerminalUnit,
		ILHM_TerminalUnitSelected,
		ILHM_HUBBranch,
		ILHM_HUBBranchSelected,
		ILHM_OpenedBox,
		ILHM_ClosedBox,
		ILHM_OverlayExclamation,
		ILHM_OverlayCrossMark,
		ILHM_OverlayMissingPartnerValve,
		ILHM_OverlayWhiteCross,
		ILHM_OverlayClosedBox,
		ILHM_PadLockYellow,
		ILHM_PadLockOrange,
		ILHM_PadLockRed,
		ILHM_Last
	};

	// 'IDB_LVINFOTREE'.
	typedef enum ImgListInfoTree
	{
		ILIT_First = 0,
		ILIT_BookClosed = ILIT_First,
		ILIT_BookOpened,
		ILIT_QuickReferenceSheetNormal,
		ILIT_QuickReferenceSheetSelected,
		ILIT_FullCatalogNormal,
		ILIT_FullCatalogSelected,
		ILIT_Last
	};

	// 'IDB_IMGLIST_PMWQPREFS'.
	typedef enum ImgListPMWQPrefs
	{
		ILPMWQP_First = 0,
		ILPMWQP_Combined = ILPMWQP_First,
		ILPMWQP_RedundPumpCompr,
		ILPMWQP_RedundTecBox,
		ILPMWQP_InternalCoating,
		ILPMWQP_ExternalAir,
		ILPMWQP_InsulatedVaporSealing,
		ILPMWQP_BreakTank,
		ILPMWQP_DutyStandBy,
		ILPMWQP_CombinedSelected,
		ILPMWQP_RedundPumpComprSelected,
		ILPMWQP_RedundTecBoxSelected,
		ILPMWQP_InternalCoatingSelected,
		ILPMWQP_ExternalAirSelected,
		ILPMWQP_InsulatedVaporSealingSelected,
		ILPMWQP_BreakTankSelected,
		ILPMWQP_DutyStandBySelected,
		ILPMWQP_Last
	};

	// 'IDB_IMGLST_PRINT'.
	typedef enum ImgListPrint
	{
		ILP_First = 0,
		ILP_BalancingValve = ILP_First,
		ILP_DpController,
		ILP_ThermostaticValve,
		ILP_HUB,
		ILP_Image4,
		ILP_PartiallyDefined,
		ILP_ArticleList,
		ILP_PipeList,
		ILP_ControlValve,
		ILP_AirVentSeparator,
		ILP_PressureMaintenance,
		ILP_TenderText,
		ILP_ShutoffValve,
		ILP_DpcBCValve,
		ILP_SafetyValve,
		ILP_6WayValveValve,
		ILP_TapWaterControl,
		ILP_SmartControlValve,
		ILP_SmartDpC,
		ILP_FloorHeatingControl,
		ILP_Last
	};

	// 'IDB_LVPROJTREE'.
	typedef enum ImgListProjectTree
	{
		ILPT_First = 0,
		ILPT_Root = ILPT_First,
		ILPT_RootSelected,
		ILPT_Image2,
		ILPT_Image3,
		ILPT_HydronicNetwork,
		ILPT_HydronicNetworkSelected,
		ILPT_HydronicCircuit,
		ILPT_HydronicCircuitSelected,
		ILPT_Image8,
		ILPT_Image9,
		ILPT_LogBranch,
		ILPT_LogBranchSelected,
		ILPT_LogChart,
		ILPT_LogChartSelected,
		ILPT_Image14,
		ILPT_Image15,
		ILPT_LogData,
		ILPT_LogDataSelected,
		ILPT_Image18,
		ILPT_Image19,
		ILPT_QuickMeasurementBranch,
		ILPT_QuickMeasurementBranchSelected,
		ILPT_OverlayExclamation,
		ILPT_OverlayCrossMark,
		ILPT_OverlayPartnerValve,
		ILPT_TerminalUnit,
		ILPT_TerminalUnitSelected,
		ILPT_OverlayEditCircuit,
		ILPT_OverlayNewCircuit,
		ILPT_HUBBranch,
		ILPT_HUBBranchSelected,
		ILPT_Last
	};

	// 'IDB_STRIP_COMM_16' and 'IDB_STRIP_COMM_32'.
	typedef enum ImgListStripComm
	{
		ILSC_First = 0,
		ILSC_PanelDataTrans = ILSC_First,
		ILSC_ScopeComm,
		ILSC_biComm,
		ILSC_PanelUpdate,
		ILSC_ProdDataBase,
		ILSC_Properties,
		ILSC_PanelDpSVisio,
		ILSC_DpSVisio,
		ILSC_Last
	};

	// 'IDB_STRIP_HOME_16' and 'IDB_STRIP_HOME_32'.
	typedef enum ImgListStripHome
	{
		ILSH_First = 0,
		ILSH_PanelDefUnits = ILSH_First,
		ILSH_PanelPipeSeries,
		ILSH_PanelTechParam,
		ILSH_DevSizeAb,
		ILSH_DevSizeBl,
		ILSH_PdropTarg,
		ILSH_PdropMax,
		ILSH_WVelTarg,
		ILSH_WVelMax,
		ILSH_PanelOptions,
		ILSH_LanguageDoc,
		ILSH_UserRef,
		ILSH_PanelInfo,
		ILSH_ReleaseNotes,
		ILSH_Tip,
		ILSH_About,
		ILSH_Manual,
		ILSH_Last
	};

	// 'IDB_STRIP_PRODUCTSEL_16' and 'IDB_STRIP_PRODUCTSEL_32'.
	typedef enum ImgListStripProductSel
	{
		ILSPS_First = 0,
		ILSPS_PanelProdType = ILSPS_First,
		ILSPS_PanelPipe,
		ILSPS_PanelTechParam,
		ILSPS_DevSizeAb,
		ILSPS_DevSizeBl,
		ILSPS_PdropTarg,
		ILSPS_PdropMax,
		ILSPS_WVelTarg,
		ILSPS_WVelMax,
		ILSPS_PanelWc,
		ILSPS_Temp,
		ILSPS_PanelSel,
		ILSPS_Direct,
		ILSPS_Batch,
		ILSPS_SearchReplace,
		ILSPS_ApplicationType,
		ILSPS_PMWQSelectionPrefs,
		ILSPS_Wizard,
		ILSPS_Individual,
		ILSPS_Last
	};

	// 'IDB_STRIP_LOGGEDDATA_16' and 'IDB_STRIP_LOGGEDDATA_32'.
	typedef enum ImgListStripLoggedData
	{
		ILSLD_First = 0,
		ILSLD_PanelCurves = ILSLD_First,
		ILSLD_PanelZoom,
		ILSLD_ZoomIn,
		ILSLD_ZoomOut,
		ILSLD_UndoZoom,
		ILSLD_Zoom,
		ILSLD_PanelChartOptions,
		ILSLD_Tracking,
		ILSLD_Markers,
		ILSLD_Legend,
		ILSLD_HorizAxis,
		ILSLD_ExportChart,
		ILSLD_Last
	};

	// 'IDB_STRIP_PROJECT_16' and 'IDB_STRIP_PROJECT_32'.
	typedef enum ImgListStripProject
	{
		ILSP_First = 0,
		ILSP_PanelProjInfo = ILSP_First,
		ILSP_HMCalc,
		ILSP_PanelProjParam,
		ILSP_SafetyFactor,
		ILSP_PanelProjTools,
		ILSP_ImportHM,
		ILSP_SearchReplace,
		ILSP_DiversityFactor,
		ILSP_PanelWc,
		ILSP_Temp,
		ILSP_MeasPeriod,
		ILSP_Start,
		ILSP_End,
		ILSP_ChangePeriod,
		ILSP_ProjectType,
		ILSP_ProjectCheck,
		ILSP_Last
	};

	// 'IDB_STRIP_SINGLESELCATEG'.
	// HYS-1149: Add 6way valve
	// HYS-1935: TA-Smart Dp - 02 - Add a new category for the ribbon
	typedef enum ImgListStripSelCategory
	{
		ILSSC_First = 0,
		ILSSC_BalancingValve = ILSSC_First,
		ILSSC_DpController,
		ILSSC_ThermostaticValve,
		ILSSC_BalancingControlValve,
		ILSSC_ControlValveHora,
		ILSSC_ControlValveBrazil,
		ILSSC_PressIndepControlValve,
		ILSSC_Hub,
		ILSSC_Separator,
		ILSSC_PressureMaintenance,
		ILSSC_DpCBCV,
		ILSSC_ShutOffValve,
		ILSSC_SafetyValve,
		ILSSC_6WayValve,
		ILSSC_SmartControlValve,
		ILSSC_SmartDp,
		ILSSC_BalancingValveGrayed,
		ILSSC_DpControllerGrayed,
		ILSSC_ThermostaticValveGrayed,
		ILSSC_BalancingControlValveGrayed,
		ILSSC_ControlValveHoraGrayed,
		ILSSC_ControlValveBrazilGrayed,
		ILSSC_PressIndepControlValveGrayed,
		ILSSC_HubGrayed,
		ILSSC_SeparatorGrayed,
		ILSSC_PressureMaintenanceGrayed,
		ILSSC_DpCBCVGrayed,
		ILSSC_ShutOffValveGrayed,
		ILSSC_SafetyValveGrayed,
		ILSSC_6WayValveGrayed,
		ILSSC_SmartControlValveGrayed,
		ILSSC_SmartDpGrayed,
		ILSSC_Last
	};

	// 'IDB_STRIP_TOOLS_16' and 'IDB_STRIP_TOOLS_32'.
	typedef enum ImgListStripTools
	{
		ILST_First = 0,
		ILST_PanelHydronic = ILST_First,
		ILST_ViscCorr,
		ILST_Pdrop,
		ILST_KvCalc,
		ILST_Calculator,
		ILST_PanelOther,
		ILST_UnitConv,
		ILST_TALink,
		ILST_Last
	};

	CRCImageManager();
	virtual ~CRCImageManager();

	// Allow to retrieve a list loading when TASelect has started.
	CImageList *GetImageList( ImageListName eImageListName );

	// Allow to retrieve a copy of a existing list.
	// Param: 'bShiftForStatus' set to true to transform 0-indexed list to 1-indexed list. It's a special case for
	//        'CTreeCtrl' class when setting image list with the flag 'TVSIL_STATE'.
	// Remark: it's up to the caller to delete 'CImageList' when it no more need it.
	CImageList *GetImageListCopy( ImageListName eImageListName, bool bShiftForStatus = false );

// Private methods.
private:
	void _Init( void );
	CImageList *_CreateImageList( short nIndex, bool bShiftForStatus = false );

// Private variables.
private:
	typedef struct _ImageInfos
	{
		int m_cx;
		int m_cy;
		UINT m_nFlags;
		int m_nInitial;
		int m_nGrow;
		COLORREF m_crMask;
	}ImageInfos;

	typedef struct _ImageDefinition
	{
		ImageListName m_eImageListName;
		int m_iBitmapID;
		ImageSize m_eImageSize;
		ImageInfos m_rImageInfos;
	}ImageDefinition;

	static ImageDefinition m_arImageList[];

	typedef std::map<ImageListName, CImageList*> mapEnumImageList;
	typedef mapEnumImageList::iterator mapEnumImageListIter;
	typedef std::map<ImageListName, short> mapEnumShort;
	typedef mapEnumShort::iterator mapEnumShortIter;
	mapEnumImageList m_mapImageContainer;
	mapEnumShort m_mapImageIndex;
};
