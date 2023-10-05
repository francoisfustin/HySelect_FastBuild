#include "stdafx.h"


#include "MainFrm.h"
#include "TASelectDoc.h"
#include "TASelectView.h"
#include "MFCVisualManagerOffice2007Ex.h"
#include "RestartAPI.h"
#include "DataStruct.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "SelectPM.h"

#include "HMTreeDroptarget.h"
#include "HMTreeListCtrl.h"

#include "DlgLeftTabProject.h"
#include "DlgLeftTabSelManager.h"
#include "DlgInfoSSelDpC.h"
#include "DpsVisioIncludes.h"

#include "SSheetLDSumm.h"
#include "SSheetLogData.h"
#include "SSheetPanelCirc2.h"
#include "SSheetQuickMeas.h"

#include "SelProdPageBase.h"
#include "SelProdDockPane.h"
#include "RViewGen.h"
#include "RViewChart.h"
#include "RViewProj.h"
#include "RViewHMCalc.h"
#include "RViewQuickMeas.h"
#include "RViewHMSumm.h"
#include "RViewInfo.h"
#include "RViewLDSumm.h"
#include "RViewLogData.h"
#include "RViewSelProd.h"
#include "RViewSSelSS.h"
#include "RViewSSelCtrl.h"
#include "RViewSSelTrv.h"
#include "RViewSSelHub.h"
#include "RViewSSelBv.h"
#include "RViewSSelBCV.h"
#include "RViewSSelCV.h"
#include "RViewSSelDpC.h"
#include "RViewSSelPICV.h"
#include "RViewSSelSeparator.h"
#include "RViewSSelPM.h"
#include "RViewSSelDpCBCV.h"
#include "RViewSSelSv.h"
#include "RViewWizardSelBase.h"
#include "RViewWizardSelPM.h"
#include "RViewStartPage.h"
#include "RViewSSelSafetyValve.h"
#include "RViewSSel6WayValve.h"
#include "RViewSSelSmartControlValve.h"
#include "RViewSSelSmartDpC.h"

#include "DlgSelectionBase.h"
#include "DlgInfoSSelDpC.h"
#include "DlgDefUnits.h"
#include "DlgDocs.h"
#include "DlgTechParam.h"
#include "DlgCustPipe.h"
#include "DlgViscCorr.h"
#include "DlgDirSel.h"
#include "DlgWaterChar.h"
#include "DlgCOWaterChar.h"
#include "DlgRef.h"
#include "DlgPrint.h"
#include "DlgUnitConv.h"
#include "DlgRibbonDateTime.h"
#include "DlgWizTAScope.h"
#include "DlgInjectionError.h"
#include "DlgImportHM.h"
#include "DlgAboutBox.h"
#include "DlgPipeDp.h"
#include "DlgKvSeries.h"

#ifdef DEBUG
#include "TabDlgSpecActDev.h"
#include "TabDlgSpecActUser.h"
#include "TabDlgSpecActTesting.h"
#include "TabDlgSpecActTesting2.h"
#include "DlgSpecAct.h"
#endif

#include "DlgTip.h"
#include "DlgPassword.h"
#include "DlgUserRef.h"
#include "DlgGateway.h"
#include "DlgSearchReplace.h"
#include "DlgRegister.h"
#include "DlgHydroCalc.h"
#include "DlgTALink.h"
#include "DlgDiversityFactor.h"
#include "DlgIndSelPM.h"
#include "DlgPMWQSelectionPrefs.h"

#include "ImportExportHMNodeDefinitions.h"
#include "ImportExportHM.h"
#include "ImportHMBase.h"
#include "ImportHMFromJson.h"
#include "ImportHMFromXML.h"
#include "ExportHMBase.h"
#include "ExportHMToJson.h"

#include "FileTableMng.h"
#include "TAScopeUtil.h"
#include "MFCRibbonButtonEx.h"

#ifdef _DEBUG
#include "..\UnitTests\cppunit\include\cppunit\extensions\TestFactoryRegistry.h"
#include "..\UnitTests\cppunit\include\cppunit\ui\text\TestRunner.h"
#include "..\UnitTests\cppunit\include\cppunit\XmlOutputter.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CTASelectApp TASApp;
CMainFrame *pMainFrame = NULL;
CRITICAL_SECTION CriticalSection;

#define USEACTIVATIONCODE

IMPLEMENT_DYNCREATE( CMainFrame, CFrameWndEx )

BEGIN_MESSAGE_MAP( CMainFrame, CFrameWndEx )
	// 	ON_WM_ACTIVATE()
	// 	ON_WM_ACTIVATEAPP()

	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_QUERYENDSESSION()
	ON_WM_ENDSESSION()
	ON_WM_TIMER()
	ON_WM_WINDOWPOSCHANGED()

	ON_COMMAND_RANGE( ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook )
	ON_UPDATE_COMMAND_UI_RANGE( ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook )

	ON_MESSAGE( WM_USER_NEWDOCUMENT, OnNewDocument )
	ON_MESSAGE( WM_USER_OPENDOCUMENT, OnOpenDocument )
	ON_MESSAGE( WM_USER_EXPORTSELPTOXLSX, OnExportSelPToXlsx )
	// HYS-1128: Unit tests for export in one sheet
	ON_MESSAGE( WM_USER_EXPORTSELPINONETOXLSX, OnExportSelPInOneToXlsx )
	ON_MESSAGE( WM_USER_DISPLAYMSGBOX, OnDisplayMsgBox )
	// HYS-1376: Unit tests import process
	ON_MESSAGE( WM_USER_IMPORT_UT, OnImportForUnitTest )

	ON_REGISTERED_MESSAGE( AFX_WM_ON_CHANGE_RIBBON_CATEGORY, OnRibbonCategoryChanged )

	ON_COMMAND( ID_APPMENU_SUBPRINT, OnAppMenuSubPrint )
	ON_COMMAND( ID_APPMENU_PRINT, OnAppMenuPrint )
	ON_COMMAND( ID_APPMENU_EXPORT_SELP, OnAppMenuExportSelP )
	ON_COMMAND( ID_APPMENU_EXPORT_SELECTEDPRODUCTSINONESHEET, OnAppMenuExportSelInOne )
	ON_COMMAND( ID_APPMENU_EXPORT_HM_JSON, OnAppMenuExportHMToJson )
	ON_COMMAND( ID_APPMENU_EXPORT_CHART, OnAppMenuExportChart )
	ON_COMMAND( ID_APPMENU_EXPORT_LDLIST, OnAppMenuExportLDList )
	ON_COMMAND( ID_APPMENU_EXPORT_QM, OnAppMenuExportQM )
	// HYS-1605: Export TA-Diagnostic and measurements 
	ON_COMMAND( ID_APPMENU_EXPORT_TADIAGNOSTIC, OnAppMenuExportTADiagnoctic )
	ON_COMMAND( ID_APPMENU_EXPORT_PIPES, OnAppMenuExportPipes )
	ON_COMMAND( ID_APPMENU_IMPORT_PIPES, OnAppMenuImportPipes )
	ON_COMMAND( ID_APPMENU_IMPORT_HM, OnAppMenuImportHM )
	ON_COMMAND( ID_APPMENU_IMPORT_HM_JSON, OnAppMenuImportHMJson )
	ON_COMMAND( ID_APPMENU_EXIT, OnAppMenuExit )
	ON_COMMAND( ID_FILE_PRINT, OnFilePrint )
	ON_COMMAND( ID_APPMENU_START_PAGE, OnAppMenuStartPage )
	ON_COMMAND( ID_APPMENU_PM_WIZARD, OnAppMenuPMWizard )

	ON_COMMAND( ID_RBN_H_DEFUNITS, OnRbnHDefUnits )						// Home\Default Units													-> 'Default units' expand button.
	ON_COMMAND( ID_RBN_H_DU_FLOW, OnRbnHDUFlow )						// Home\Default Units\Flow												-> 'Flow' combo.
	ON_COMMAND( ID_RBN_H_DU_DIFFPRESS, OnRbnHDUDiffPres )				// Home\Default Units\Differential Pressure								-> 'Differential pressure' combo.
	ON_COMMAND( ID_RBN_H_DU_TEMP, OnRbnHDUTemp )						// Home\Default Units\Temperature										-> 'Temperature' combo.
	ON_COMMAND( ID_RBN_H_PIPESERIES, OnRbnHPipeSeries )					// Home\Pipe Series														-> 'Pipe series' expand button.
	ON_COMMAND( ID_RBN_H_PS_DEFPIPESERIES, OnRbnHPSDefPipeSeries )		// Home\Pipe Series\Default Pipe Series									-> 'Default pipe series' combo.
	ON_COMMAND( ID_RBN_H_TECHPARAM, OnRbnHTechParam )					// Home\Technical Parameters											-> 'Technical parameters' expand button.
	ON_COMMAND( ID_RBN_H_TP_SIZEABV, OnRbnHTPDSSizeAbv )				// Home\Technical Parameters\Device Size\Size Above						-> 'Size above' spin button.
	ON_COMMAND( ID_RBN_H_TP_SIZEBLW, OnRbnHTPDSSizeBLW )				// Home\Technical Parameters\Device Size\Size Below						-> 'Size below' spin button.
	ON_COMMAND( ID_RBN_H_TP_PD_TARGET, OnRbnHTPPDTarget )				// Home\Technical Parameters\Pipe Pressure Drop\Target					-> 'Target' edit field.
	ON_COMMAND( ID_RBN_H_TP_PD_MAX, OnRbnHTPPDMax )						// Home\Technical Parameters\Pipe Pressure Drop\Max						-> 'Max' edit field.
	ON_COMMAND( ID_RBN_H_TP_WV_TARGET, OnRbnHTPWVTarget )				// Home\Technical Parameters\Pipe Water Velocity\Target					-> 'Target' edit field.
	ON_COMMAND( ID_RBN_H_TP_WV_MAX, OnRbnHTPWVMax )						// Home\Technical Parameters\Pipe Water Velocity\Max					-> 'Max' edit field.
	ON_COMMAND( ID_RBN_H_O_LANG, OnRbnHOLang )							// Home\Options\Language												-> 'Language' combo.
	ON_COMMAND( ID_RBN_H_O_DOCS, OnRbnHODocs )							// Home\Options\Docs													-> 'Product documentation' button.
	ON_COMMAND( ID_RBN_H_O_AUTOSAV, OnRbnHOAutoSav )					// Home\Options\Auto Save												-> 'Autosave recovery files' check box.
	ON_COMMAND( ID_RBN_H_O_USERREF, OnRbnHOUserRef )					// Home\Options\User References											-> 'User references' button.
	ON_COMMAND( ID_RBN_H_I_NOTES, OnRbnHINotes )						// Home\Information\Notes												-> 'Release notes' button.
	ON_COMMAND( ID_RBN_H_I_TIPOFTHEDAY, OnRbnHITipOfTheDay )			// Home\Information\Tip Of The Day										-> 'Tip of the day' button.
	ON_COMMAND( ID_RBN_H_I_ABOUT, OnRbnHIAbout )						// Home\Information\About												-> 'About HySelect' button.
	ON_COMMAND( ID_RBN_USERMANUAL, OnRbnUsermanual )

#ifdef _DEBUG
	ON_COMMAND( ID_RBN_TEST_UNIT, OnRbnTestsUnit )						// Home\Tests\Unit
	ON_COMMAND( ID_RBN_TEST_SPECIALACTIONS, OnRbnTestsSpecialActions )	// Home\Tests\Special actions
#endif

	ON_UPDATE_COMMAND_UI( ID_RBN_USERMANUAL, OnUpdateUsermanual )

	ON_COMMAND( ID_RBN_P_PROJINFO, OnRbnPProjInfo )						// Project\Project Info													-> 'Project info' expand button.
	ON_COMMAND( ID_RBN_P_PI_PROJNAME, OnRbnPPIProjName )				// Project\Project Info\Project Name 									-> 'Project name' edit field.
	ON_COMMAND( ID_RBN_P_PI_CBICOMPATIBLE, OnRbnPPICBICompatible )		// Project\Project Info\CBI Compatible									-> 'TA-CBI compatibility' check box.
	ON_COMMAND( ID_RBN_P_PI_HMCALC, OnRbnPPIHMCalc )					// Project\Project Info\HM Calc											-> 'Hydonic circuit calculation' button.
	ON_COMMAND( ID_RBN_P_PP_FREEZE, OnRbnPPPFreeze )					// Project\Project Parameters\Freeze									-> 'Freeze hydronic circuit calculation' check box.
	ON_COMMAND( ID_RBN_P_PP_HNAUTOCHECK, OnRbnPPPHNAutoCheck )			// Project\Project Parameters\AutoCheck									-> 'Freeze hydronic circuit calculation' check box.
	ON_COMMAND( ID_RBN_P_PP_PROJECTTYPE, OnRbnPPPProjectType )			// Project\Project Parameters\Project Type								-> Project type Heating or cooulin combo box.
	ON_COMMAND( ID_RBN_P_DEBUG_VERIFYAUTORITY, OnRbnPDebugVerityAutority )	// Project\Debug\Verify autority									-> 'Verify autority' button.

	ON_COMMAND( ID_RBN_P_PT_IMPORT_HM, OnAppMenuImportHM )				// Project\Tools\Import hydronic circuits								-> 'Import hydronic circuits' button.
	ON_COMMAND( ID_RBN_P_PT_HNCHECK, OnRbnPrjLaunchHNCheck )			// Project\Tools\check hydronic circuits calculation
	ON_COMMAND( ID_RBN_P_PT_SR, OnRbnPTSR )								// Project\Tools\Search and Replace										-> 'Search and Replace' button.
	ON_COMMAND( ID_RBN_P_PT_DF, OnRbnPTDF )								// Project\Tools\Diversity Factor										-> 'Diversity Factor' button.
	ON_COMMAND( ID_RBN_P_WC_TEMP, OnRbnPWCTemp )						// Project\Water Characteristic Temp									-> 'Fluid Characteristic'
	ON_COMMAND( ID_RBN_P_WATERCHAR, OnRbnPWaterChar )					// Project\Water Characteristic\Temperature								-> 'Temperature' edit field.
	ON_COMMAND( ID_RBN_P_WC_DT, OnRbnPWCDT )							// Project\Water Characteristic\Default DT								-> 'DefaultDT' edit field.
	ON_COMMAND( ID_RBN_P_MP_DATETIME, OnRbnPMPDateTime )				// Project\Measurement Period\Data Time									-> 'Choose period' button.

	ON_COMMAND( ID_RBN_T_H_VISCCOR, OnRbnTHViscCor )					// Tools\Hydronic\Viscosity Correction									-> 'Viscosity correction' button.
	ON_COMMAND( ID_RBN_T_H_PIPEPRESSDROP, OnRbnTHPipePressDrop )		// Tools\Hydronic\Pipe Pressure Drop									-> 'Pipe pressure drop' button.
	ON_COMMAND( ID_RBN_T_H_KVCALC, OnRbnTHKvCalc )						// Tools\Hydronic\Kv Calculation										-> 'Kv values calculation' button.
	ON_COMMAND( ID_RBN_T_H_CALCULATOR, OnRbnTHCalculator )				// Tools\Hydronic\Calculator											-> 'Hydronic calculator' button.
	ON_COMMAND( ID_RBN_T_O_UNITCONV, OnRbnTOUnitConv )					// Tools\Other\Unit Conversion											-> 'Unit conversion' button.
	ON_COMMAND( ID_RBN_T_O_TALINK, OnRbnTOTALink )						// Tools\Other\TALink													-> 'TA Link' button.

	ON_COMMAND( ID_RBN_C_DT_SCOPECOMM, OnRbnCDTScopeComm )					// Communication\Data Transfer\Scope Communication					-> 'TA-SCOPE communication' button.
	ON_COMMAND( ID_RBN_C_DT_CBICOMM, OnRbnCDTCbiComm )						// Communication\Data Transfer\CBI Communication					-> 'TA-CBI communication' button.
	ON_COMMAND( ID_RBN_C_DT_DPSVISIOCOMM, OnRbnCDTDpSVisioComm )			// Communication\DpS-Visio\DpS-Visio Communication					-> 'DpS-Vision communication' button.
	ON_COMMAND( ID_RBN_C_U_PRODDATA, OnRbnCUProdData )						// Communication\Update\Product Data								-> 'Update HySelect' button.
	ON_COMMAND( ID_RBN_C_U_PROPERTIES, OnRbnCUProperties )					// Communication\Update\Properties									-> 'Properties' button.
	ON_COMMAND( ID_RBN_C_U_MAINTAINTASCOPE, OnRbnCUMaintainTAScope )		// Communication\Update\Maintain TAScope							-> 'TA-SCOPE update monitoring' check box.
	ON_COMMAND( ID_RBN_C_U_TESTHYSELECTUPDATE, OnRbnCUTestHySelectUpdate )	// Communication\Update\Maintain TAScope							-> 'Test HySelect update' button.

	ON_COMMAND( ID_RBN_PS_PT_GALLERY, OnRbnPSPTGallery )				// Product selection\Product Type\Gallery							-> 'Product type' selection gallery.
	ON_COMMAND( ID_RBN_PS_SELECTIONMODE_INDIVIDUAL, OnRbnPSSelectionModeIndividual )	// Product selection\Product\Selection\Individual, batch, wizard & direct selection\Individual -> 'Indivudal' button.
	ON_COMMAND( ID_RBN_PS_SELECTIONMODE_BATCH, OnRbnPSSelectionModeBatch )				// Product selection\Product\Selection\Individual, batch, wizard & direct selection\Batch -> 'Batch' button.
	ON_COMMAND( ID_RBN_PS_SELECTIONMODE_WIZARD, OnRbnPSSelectionModeWizard )			// Product selection\Product\Selection\Individual, batch, wizard & direct selection\Wizard -> 'Wizard' button.
	ON_COMMAND( ID_RBN_PS_SELECTIONMODE_DIRECT, OnRbnPSSelectionModeDirect )			// Product selection\Product\Selection\Individual, batch, wizard & direct selection\Direct -> 'Direct' button.
	ON_COMMAND( ID_RBN_PS_PIPE, OnRbnHPipeSeries )						// Product selection\Pipes
	ON_COMMAND( ID_RBN_PS_P_PIPESERIES, OnRbnPSPPipeSeries )			// Product selection\Pipes\Pipes Series								-> 'Pipe series' combo.
	ON_COMMAND( ID_RBN_PS_P_PIPESIZE, OnRbnPSPPipeSize )				// Product selection\Pipes\Pipes Size								-> 'Pipe size' combo.
	ON_COMMAND( ID_RBN_SSEL_TP_SIZEABV, OnRbnPSTPDSSizeAbv )			// Product selection\Technical parameters\Device Size\Size Above		-> 'Size above' spin button.
	ON_COMMAND( ID_RBN_SSEL_TP_SIZEBLW, OnRbnPSTPDSSizeBLW )			// Product selection\Technical parameters\Device Size\Size Below		-> 'Size below' spin button.
	ON_COMMAND( ID_RBN_SSEL_TP_PD_TARGET, OnRbnPSTPPDTarget )			// Product selection\Technical parameters\Pipe pressure Drop\Target	-> 'Max' edit field.
	ON_COMMAND( ID_RBN_SSEL_TP_PD_MAX, OnRbnPSTPPDMax )					// Product selection\Technical parameters\Pipe pressure Drop\Max		-> 'Max' edit field.
	ON_COMMAND( ID_RBN_SSEL_TP_WV_TARGET, OnRbnPSTPWVTarget )			// Product selection\Technical parameters\Pipe Water Velocity\Target	-> 'Target' edit field.
	ON_COMMAND( ID_RBN_SSEL_TP_WV_MAX, OnRbnPSTPWVMax )					// Product selection\Technical parameters\Pipe Water Velocity\Max	-> 'Max' edit field.
	ON_COMMAND( ID_RBH_PS_PMWQ_COMBINEDFCT, OnRbnPSPMWQPrefsCombined )			// Product selection\Product\PM & WQ selection preferences\General - Combined function in one device.
	ON_COMMAND( ID_RBH_PS_PMWQ_REDPUMPCOMPR, OnRbnPSPMWQPrefsRedPumpComp )		// Product selection\Product\PM & WQ selection preferences\General - Redundancy Pump/Compressor.
	ON_COMMAND( ID_RBH_PS_PMWQ_REDTECBOX, OnRbnPSPMWQPrefsRedTecBox )			// Product selection\Product\PM & WQ selection preferences\General - Redundancy TecBox.
	ON_COMMAND( ID_RBH_PS_PMWQ_EXPVSSLINTCOAT, OnRbnPSPMWQPrefsIntCoating )		// Product selection\Product\PM & WQ selection preferences\Compressor - Expansion vessel with internal coating.
	ON_COMMAND( ID_RBH_PS_PMWQ_EXTCOMPRAIR, OnRbnPSPMWQPrefsExtAir )			// Product selection\Product\PM & WQ selection preferences\Compressor - With external compressed air.
	ON_COMMAND( ID_RBH_PS_PMWQ_INSVAPORSEALED, OnRbnPSPMWQPrefsInsVapSealed )	// Product selection\Product\PM & WQ selection preferences\Pump & Degassing - Cooling insulation with condensation water protection.
	ON_COMMAND( ID_RBH_PS_PMWQ_BREAKTANK, OnRbnPSPMWQPrefsBreakTank )			// Product selection\Product\PM & WQ selection preferences\Water make-up - Break tank.
	ON_COMMAND( ID_RBH_PS_PMWQ_DUTYSTANDBY, OnRbnPSPMWQPrefsDtyStdBy )			// Product selection\Product\PM & WQ selection preferences\Water make-up - Duty and stand-by of make-up pumps.
	ON_COMMAND( ID_RBN_PS_APPLITYPE, OnRbnPSAppplicationType )					// Product selection\Product\Application type -> Application type 'Heating/cooling/Solar' combo box.
	ON_COMMAND( ID_RBN_PS_WC_TEMP, OnRbnPSWCTemp )								// Product selection\Product\Water Characteristic\Temperature -> 'Temperature' edit field.
	ON_COMMAND(	ID_RBN_PS_WC_DT, OnRbnPSWCDT )									// Product selection\Product\Water Characteristic\Default DT -> 'DefaultDT' edit field.
	ON_COMMAND( ID_RBN_PS_WATERCHAR, OnRbnPSWaterChar )							// Product selection\Product\Water Characteristic -> 'Fluid Characteristic' expand button.
	ON_COMMAND( ID_RBN_PS_T_SR, OnRbnPSToolsSR )								// Product selection\Product\Tools\Search & Replace -> 'Search & replace' button.
	ON_COMMAND( ID_RBN_PS_PMWQ_ONEDEVICE, OnRbnPSPMWQOneDevice )				// Product selection\Technical parameters\PM & WQ selection preferences\Combined functions in one device
	ON_COMMAND( ID_RBN_PS_PMWQ_COMBOREDDCY, OnRbnPSPMWQComboRedundancy )		// Product selection\Technical parameters\PM & WQ selection preferences\Redundancy
	ON_COMMAND( ID_RBN_PS_PMWQSELECTPREFS, OnRbnPSPMWQSelectPrefs )				// Product selection\Technical parameters\PM & WQ selection preferences
	ON_COMMAND( ID_RBN_PS_PMWQMAXNBROFADDITIONALVESSELSINPARALLEL, OnRbnPSPMWQNbreMaxOfAdditionalVessels ) // Product selection\Technical parameters\PM & WQ Technical parameters number of max vessel
	ON_COMMAND( ID_RBN_PS_PMWQMAXNBROFVENTOINPARALLEL, OnRbnPSPMWQNbreMaxOfVento ) // Product selection\Technical parameters\PM & WQ Technical parameters number of max vento

	ON_UPDATE_COMMAND_UI( ID_APPMENU_PRINT, OnUpdateFilePrint )
	ON_UPDATE_COMMAND_UI( ID_APPMENU_SUBPRINT, OnUpdateFileSubPrint )
	ON_UPDATE_COMMAND_UI( ID_APPMENU_EXPORT, OnUpdateExport )
	ON_UPDATE_COMMAND_UI( ID_APPMENU_EXPORT_SELP, OnUpdateFileExportSelp )
	ON_UPDATE_COMMAND_UI( ID_APPMENU_EXPORT_SELECTEDPRODUCTSINONESHEET, OnUpdateFileExportSelp )
	ON_UPDATE_COMMAND_UI( ID_APPMENU_EXPORT_CHART, OnUpdateFileExportChart )
	ON_UPDATE_COMMAND_UI( ID_APPMENU_EXPORT_LDLIST, OnUpdateFileExportLdlist )
	ON_UPDATE_COMMAND_UI( ID_APPMENU_EXPORT_QM, OnUpdateFileExportQM )
	// HYS-1605: Availability of Export TA-Diagnostic menu
	ON_UPDATE_COMMAND_UI( ID_APPMENU_EXPORT_TADIAGNOSTIC, OnUpdateFileExportTADiagnostic )
	ON_UPDATE_COMMAND_UI( ID_APPMENU_IMPORT, OnUpdateImport )
	ON_UPDATE_COMMAND_UI( ID_APPMENU_IMPORT_HM, OnUpdateMenuImportHM )
	ON_UPDATE_COMMAND_UI( ID_APPMENU_IMPORT_HM_JSON, OnUpdateMenuImportHMJson )


	ON_UPDATE_COMMAND_UI( ID_RBN_H_O_LANG, OnUpdateLanguage )						// Home\Options\Language								-> 'Language' combo.
	ON_UPDATE_COMMAND_UI( ID_RBN_H_O_AUTOSAV, OnUpdateActiveAutoSaving )			// Home\Options\Auto Save								-> 'Autosave recovery files' check box.

	ON_UPDATE_COMMAND_UI( ID_RBN_P_PI_CBICOMPATIBLE, OnUpdateCBICompatible )		// Project\Project Info\CBI Compatible					-> 'TA-CBI compatibility' check box.
	ON_UPDATE_COMMAND_UI( ID_RBN_P_PI_HMCALC, OnUpdateBtnHMCalc )					// Project\Project Info\HM Calc							-> 'Hydonic circuit calculation' button.
	ON_UPDATE_COMMAND_UI( ID_RBN_P_PP_FREEZE, OnUpdateFreezeHMCalc )				// Project\Project Parameters\Freeze					-> 'Freeze hydronic circuit calculation' check box.
	ON_UPDATE_COMMAND_UI( ID_RBN_P_PP_HNAUTOCHECK, OnUpdateHNAutoCheck )			// Project\Project Parameters\AutoCheck
	ON_UPDATE_COMMAND_UI( ID_RBN_P_PP_PROJECTTYPE, OnUpdateProjectType )			// Project\Project Parameters\Project Type				-> Project type (heating/cooling) combo box.
	ON_UPDATE_COMMAND_UI( ID_RBN_P_DEBUG_VERIFYAUTORITY, OnUpdateHMCalcDebugVerifyAutorityCheck ) // Project\Debug\Verify autority			-> 'Verify autority' check box.
	ON_UPDATE_COMMAND_UI( ID_RBN_P_PT_IMPORT_HM, OnUpdateImportHM )					// Project\Tools\Import hydronic circuits				-> 'Import hydronic circuits' button.
	ON_UPDATE_COMMAND_UI( ID_RBN_P_PT_HNCHECK, OnUpdateCheckHNCalculationBtn )		// Project\Tools\Check hydronic circuits
	ON_UPDATE_COMMAND_UI( ID_RBN_P_PT_SR, OnUpdateSearchReplace )					// Project\Tools\Search and Replace						-> 'Search and Replace' button.
	ON_UPDATE_COMMAND_UI( ID_RBN_P_PT_DF, OnUpdateDiversityFactor )					// Project\Tools\Diversity Factor						-> 'Diversity Factor' button.
	ON_UPDATE_COMMAND_UI( ID_RBN_P_MP_START, OnUpdateUpperBoundaries )				// Project\Measurement Period\Period Start				-> 'Start' edit field.
	ON_UPDATE_COMMAND_UI( ID_RBN_P_MP_END, OnUpdateLowerBoundaries )				// Project\Measurement Period\Period End				-> 'End' edit field.
	ON_UPDATE_COMMAND_UI( ID_RBN_P_MP_DATETIME, OnUpdateProjectDateTimeChange )		// Project\Measurement Period\Data Time					-> 'Choose period' button.
	ON_UPDATE_COMMAND_UI( ID_RBN_P_WC_TEMP, OnUpdateProjectWaterTemp )				// Project\Fluid characteristics\Temperature			-> 'Temperature' edit field.
	ON_UPDATE_COMMAND_UI( ID_RBN_P_WC_DT, OnUpdateProjectWaterDT )					// Project\Fluid characteristics\Default DT				-> 'Default DT' edit field.
	ON_UPDATE_COMMAND_UI( ID_RBN_P_WATERCHAR, OnUpdateProjectWaterChar )			// Project\Fluid characteristics\Water char				-> 'Fluid characteristics' button.

	ON_UPDATE_COMMAND_UI( ID_RBN_C_U_MAINTAINTASCOPE, OnUpdateTASCOPEFILES )			// Communication\Data Transfer\TA-Scope Communication	-> 'TA-SCOPE communication' button.
	ON_UPDATE_COMMAND_UI( ID_RBN_C_DT_CBICOMM, OnUpdateUpdateCBI )						// Communication\Data Transfer\CBI Communication		-> 'TA-CBI communication' button.
	ON_UPDATE_COMMAND_UI( ID_RBN_C_DT_DPSVISIOCOMM, OnUpdateUpdateDpSVisio )			// Communication\DpS-Visio\DpS-Visio Communication		-> 'DpS-Visio communication' button.
	ON_UPDATE_COMMAND_UI( ID_RBN_C_U_TESTHYSELECTUPDATE, OnUpdateTestHySelectUpdate )	// Communication\Update\Test HySelect update			-> 'Test HySelect update' button.

	ON_UPDATE_COMMAND_UI( ID_RBN_PS_WC_DT, OnUpdateISWaterDT )						// 

	ON_UPDATE_COMMAND_UI( ID_RBN_PS_SELECTIONMODE_INDIVIDUAL, OnUpdateProductSelIndividualSelection )	// Product selection tools\Product\Direct, Batch & Wizard Selection\Individual -> 'Individual' button.
	ON_UPDATE_COMMAND_UI( ID_RBN_PS_SELECTIONMODE_BATCH, OnUpdateProductSelBatchSelection )				// Product selection tools\Product\Direct, Batch & Wizard Selection\Batch -> 'Batch' button.
	ON_UPDATE_COMMAND_UI( ID_RBN_PS_SELECTIONMODE_WIZARD, OnUpdateProductSelWizardSelection )			// Product selection tools\Product\Direct, Batch & Wizard Selection\Wizard -> 'Wizard' button.
	ON_UPDATE_COMMAND_UI( ID_RBN_PS_APPLITYPE, OnUpdatePSApplicationType )                                // Product selection\Product\Application type -> Application type 'Heating/cooling/Solar' combo box.
	ON_UPDATE_COMMAND_UI( ID_RBN_PS_T_SR, OnUpdateProductSelSearchReplace )					// Product selection tools\Product\Tools\Search & Replace -> 'Search & replace' button.

	ON_UPDATE_COMMAND_UI( ID_RBN_PS_P_PIPESERIES, OnUpdateProductSelPipeseries )			// Product selection tools\Tech. parameters\Pipes\Pipes Series -> 'Pipe series' combo.
	ON_UPDATE_COMMAND_UI( ID_RBN_PS_P_PIPESIZE, OnUpdateProductSelPipesize )				// Product selection tools\Tech. parameters\Pipes\Pipes Size -> 'Pipe size' combo.

	ON_UPDATE_COMMAND_UI( ID_RBN_PS_PMWQ_ONEDEVICE, OnUpdateProductSelPMWQOneDevice )		// Product selection tools\Pressurisation preferences\PM & WQ selection preferences\Combined functions in on device -> Checbox.
	ON_UPDATE_COMMAND_UI( ID_RBN_PS_PMWQ_COMBOREDDCY, OnUpdateProductSelPMWQRedundancy )	// Product selection tools\Pressurisation preferences\PM & WQ selection preferences\Redundancy -> Combo.
	ON_UPDATE_COMMAND_UI( ID_RBN_PS_PMWQSELECTPREFS, OnUpdateProductSelPMWQSelectPrefs )	// Product selection tools\Pressurisation preferences\PM & WQ selection preferences -> The expand button
	
	ON_UPDATE_COMMAND_UI( ID_RBH_PS_PMWQ_COMBINEDFCT, OnUpdateProductSelPMWQPrefsOneDevice )			// Product selection tools\Pressurisation preferences\Pressure maintenance\General - Combined functions in on device -> Button.
	ON_UPDATE_COMMAND_UI( ID_RBH_PS_PMWQ_REDPUMPCOMPR, OnUpdateProductSelPMWQPrefsRedPumpComp )			// Product selection tools\Pressurisation preferences\Pressure maintenance\General - Redundancy on pump/compressor -> Button.
	ON_UPDATE_COMMAND_UI( ID_RBH_PS_PMWQ_REDTECBOX, OnUpdateProductSelPMWQPrefsRedTecBox )				// Product selection tools\Pressurisation preferences\Pressure maintenance\General - Redundancy on TecBox -> Button.
	ON_UPDATE_COMMAND_UI( ID_RBH_PS_PMWQ_EXPVSSLINTCOAT, OnUpdateProductSelPMWQPrefsIntCoating )		// Product selection tools\Pressurisation preferences\Pressure maintenance\Compressor - Internal coating -> Button.
	ON_UPDATE_COMMAND_UI( ID_RBH_PS_PMWQ_EXTCOMPRAIR, OnUpdateProductSelPMWQPrefsExternalAir )			// Product selection tools\Pressurisation preferences\Pressure maintenance\Compressor - With external compressed air -> Button.
	ON_UPDATE_COMMAND_UI( ID_RBH_PS_PMWQ_INSVAPORSEALED, OnUpdateProductSelPMWQPrefsCoolingInsulation )	// Product selection tools\Pressurisation preferences\Pressure maintenance\Pump & Degassing - Cooling insulation with condensation water protection -> Button.
	ON_UPDATE_COMMAND_UI( ID_RBH_PS_PMWQ_BREAKTANK, OnUpdateProductSelPMWQPrefsBreakTank )				// Product selection tools\Pressurisation preferences\Pressure maintenance\Water make-up - Break tank -> Button.
	ON_UPDATE_COMMAND_UI( ID_RBH_PS_PMWQ_DUTYSTANDBY, OnUpdateProductSelPMWQPrefsDutyStdBy )			// Product selection tools\Pressurisation preferences\Pressure maintenance\Water make-up - Duty and stand-by of make-up pumps -> Button.

	ON_COMMAND( ID_CUSTOMIZE_USERPASSWORD, OnCustomizeUserpassword )

#ifdef _DEBUG
	ON_COMMAND( ID_HELP_DEBUG, OnRbnTestsSpecialActions )
	ON_COMMAND( ID_HELP_UNIT, OnRbnTestsUnit )
#endif

	ON_MESSAGE( WM_USER_UPDATEPRODUCTDATABASE, ( LRESULT( CWnd::* )( WPARAM, LPARAM ) )OnRbnCUProdData )
	ON_MESSAGE( WM_USER_WATERCHANGE, OnWaterChange )
	ON_MESSAGE( WM_USER_PSWCHANGED, OnPswChanged )
	ON_MESSAGE( WM_USER_PROJECTREFCHANGE, OnProjectRefChange )
	ON_MESSAGE( WM_USER_PIPECHANGE, OnUserPipeChange )
	ON_MESSAGE( WM_USER_TECHPARCHANGE, OnTechParChange )
	ON_MESSAGE( WM_USER_DIVERSITYFACTORTOCLOSE, OnDiversityFactorToClose )
	ON_MESSAGE( WM_USER_DLGPRINTTOCLOSE, OnDialogPrintToClose )
	ON_MESSAGE( WM_USER_APPLICATIONTYPECHANGE, OnApplicationTypeChange )

#ifdef _DEBUG
	ON_MESSAGE( WM_USER_DLGDEBUGCLOSE, OnDialogDebugToClose )
#endif

END_MESSAGE_MAP()

INT_PTR CMFCRibbonComboBoxEx::AddItemEx( LPCTSTR lpszItem, LPCTSTR lpszItemID )
{
	m_ArString.Add( lpszItemID );
	return AddItem( lpszItem );
}

int CMFCRibbonComboBoxEx::FindItemID( LPCTSTR lpszItemID )
{
	for( int i = 0; i < m_ArString.GetCount(); i++ )
	{
		if( 0 == m_ArString.GetAt( i ).Compare( lpszItemID ) )
		{
			return i;
		}
	}

	return -1;
}

CString CMFCRibbonComboBoxEx::GetItemID( int iIndex )
{
	if( iIndex < m_ArString.GetCount() )
	{
		return m_ArString.GetAt( iIndex );
	}
	else
	{
		return _T( "" );
	}
}

int CMFCRibbonComboBoxEx::GetItemIndex( LPCTSTR lpszItem )
{
	int iCount = GetCount();
	int iIndex = 0;
	int iReturn = -1;
	while( iIndex < iCount )
	{
		if( 0 == StringCompare( GetItem( iIndex ), lpszItem ) )
		{
			iReturn = iIndex;
			break;
		}
		iIndex++;
	}
	return iReturn;
}

void CMFCRibbonComboBoxEx::RemoveAllItems( void )
{
	m_ArString.RemoveAll();
	CMFCRibbonComboBox::RemoveAllItems();
}

BOOL CMFCRibbonComboBoxEx::DeleteItem( int iIndex )
{
	BOOL fReturnValue = FALSE;

	if( iIndex < m_ArString.GetCount() )
	{
		m_ArString.RemoveAt( iIndex );
		CMFCRibbonComboBox::DeleteItem( iIndex );
		fReturnValue = TRUE;
	}
	else
	{
		ASSERT( 0 );
	}

	return fReturnValue;
}

BOOL CMFCRibbonComboBoxEx::DeleteItem( DWORD_PTR dwData )
{
	ASSERT( 0 );
	return false;
}

BOOL CMFCRibbonComboBoxEx::DeleteItem( LPCTSTR lpszText )
{
	ASSERT( 0 );
	return false;
}

CMFCRibbonComboBoxEx &CMFCRibbonComboBoxEx::operator=( CMFCRibbonComboBoxEx &ComboBox )
{
	RemoveAllItems();

	for( int iLoop = 0; iLoop < ComboBox.GetCount(); iLoop++ )
	{
		AddItemEx( ComboBox.GetItem( iLoop ), ComboBox.GetItemID( iLoop ) );
	}

	return *this;
}

CMainFrame::CMainFrame()
{
	pMainFrame = this;
	TASApp.SetAppLook( TASApp.GetInt( L"ApplicationLook", ID_VIEW_APPLOOK_OFF_2007_BLUE ) );
	m_bViewHydronicStatus = true;
	m_bViewControlPane = true;
	m_fComboSelPipeSeriesEnabled = true;
	m_fComboSelPipeSizeEnabled = true;
	m_bMeasExist = false;
	m_fButHMCalc = false;
	m_pHomeCategory = NULL;
	m_pProjectCategory = NULL;
	m_pToolsCategory = NULL;
	m_pCommunicationCategory = NULL;
	m_pProdSelToolsProductContextualCategory = NULL;
	m_pProdSelToolsTechParamContextualCategory = NULL;
	m_pProdSelToolsPressurisationPrefContextualCategory = NULL;
	m_pProdSelToolsPMWizardTechParamContextualCategory = NULL;
	m_pLogDataCategory = NULL;
	m_pPanelHomeDefaultUnits = NULL;
	m_pPanelHomePipeSeries = NULL;
	m_pPanelHomeTechnicalParameters = NULL;
	m_pPanelProductSelTechnicalParameters = NULL;
	m_pPanelProductSelPMTechnicalParameters = NULL;
	m_pPanelProductSelPMWizardTechnicalParameters = NULL;
	m_pPanelHomeOptions = NULL;
	m_pPanelHomeInformation = NULL;
	m_pPanelProjectProjectInfo = NULL;
	m_pPanelProjectFluidCharacteristics = NULL;
	m_pPanelProjectParameters = NULL;
	m_pPanelProjectMeasurementPeriod = NULL;
	m_pPanelProjectDebug = NULL;
	m_pPanelToolsHydronic = NULL;
	m_pPanelToolsOther = NULL;
	m_pPanelCommunicationDataTransfer = NULL;
	m_pPanelCommunicationDpSVisio = NULL;
	m_pPanelCommunicationUpdate = NULL;
	m_pPanelProductSelProductType = NULL;
	m_pPanelProductSelFluidCharacteristics = NULL;
	m_pPanelProductSelTools = NULL;
	m_pPanelProductSelSelection = NULL;
	m_pPanelProductSelPMWQPrefs = NULL;
	m_pPanelProductSelPMWQSelectionPrefs = NULL;
	m_pPanelLoggedDataDisplayHideCurves = NULL;
	m_pPanelLoggedDataChartOptions = NULL;
	m_pPanelLoggedDataZoom = NULL;
	m_pGallery = NULL;
	m_pEBSizeAboveHome = NULL;
	m_pEBSizeBelowHome = NULL;
	m_pEBPresTargetHome = NULL;
	m_pEBPresMaxHome = NULL;
	m_pEBVelTargetHome = NULL;
	m_pEBVelMaxHome = NULL;
	m_pEBMaxNbrOfAdditionalVesselsInParallelSSel = NULL;
	m_pEBMaxNbrOfVentoSSel = NULL;
	m_pEBMaxNbrOfVsslSSelPMWizard = NULL;
	m_pEBMaxNbrOfVentoSSelPMWizard = NULL;
	m_pEBSizeAboveSSel = NULL;
	m_pEBSizeBelowSSel = NULL;
	m_pEBPresTargetSSel = NULL;
	m_pEBPresMaxSSel = NULL;
	m_pEBVelTargetSSel = NULL;
	m_pEBVelMaxSSel = NULL;
	m_pEBProjName = NULL;
	m_pCBCBICompatible = NULL;
	m_pCBPipeSeries = NULL;
	m_pRibbonComboSelPipeSeries = NULL;
	m_pRibbonComboSelPipeSize = NULL;
	m_pPanelProductSelPipe = NULL;
	m_pLBWaterCharInfo1Proj = NULL;
	m_pLBWaterCharInfo2Proj = NULL;
	m_pEBWaterCharTempProj = NULL;
	m_pEBWaterCharDTProj = NULL;
	m_pLBWaterCharInfo1ProductSel = NULL;
	m_pLBWaterCharInfo2SSel = NULL;
	m_pEBWaterCharTempProductSel = NULL;
	m_pEBWaterCharDTProductSel = NULL;
	m_pEBWaterCharChangeOverTempProductSel = NULL;
	m_pEBWaterCharChangeOverDTProductSel = NULL;
	m_pChBFreezeHMCalc = NULL;
	m_pChBHNAutoCheck = NULL;
	m_pEBStartLimitProj = NULL;
	m_pEBEndLimitProj = NULL;
	m_pChBVerifyAutority = NULL;
	m_pCBLDT1 = NULL;
	m_pCBLDT2 = NULL;
	m_pPMWQSelecPrefsRedundancy = NULL;
	m_tCurrentUpperTimeLimits = COleDateTime();
	m_tCurrentLowerTimeLimits = COleDateTime();
	m_tMaxUpperTimeLimits = COleDateTime();
	m_tMaxLowerTimeLimits = COleDateTime();
	m_ArResIDMenuToGray.RemoveAll();
	InitializeCriticalSection( &m_CriticalSection );
	m_eCurrentView = RightViewList::eUndefined;
	m_pDlgDiversityFactor = NULL;
	m_pCBProjectApplicationType = NULL;
	m_pCBProductSelectionApplicationType = NULL;
	m_uiTimer = ( UINT_PTR )0;
	m_pDlgPrint = NULL;
	m_fShown = false;
	m_HMainAppLostFocusNULL = NULL;
	m_HMainAppLostFocusDlg = NULL;
	m_eCurrentProductSubCategory = ProductSubCategory::PSC_Undefined;
	m_eCurrentProductSelectionMode = ProductSelectionMode_Individual;

#ifdef _DEBUG
	m_pDlgDebug = NULL;
	m_pTestRunnerModel = NULL;
	m_pTestRunnerDlg = NULL;
#endif

	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_Undefined] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_PM_AirVent] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_PM_Separator] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_PM_ExpansionVessel] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_PM_TecBox] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_PM_SofteningAndDesalination] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_PM_SafetyValve] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_BC_RegulatingValve] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_BC_DpController] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_BC_BalAndCtrlValve] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_BC_PressureIndepCtrlValve] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_BC_CombinedDpCBalCtrlValve] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_BC_ControlValve] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_BC_SmartControlValve] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_BC_SmartDpC] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_BC_6WayControlValve] = true; // HYS-1149
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_BC_HUB] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_TC_ThermostaticValve] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_TC_FloorHeatingControl] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_TC_TapWaterControl] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Individual][PSC_ESC_ShutoffValve] = true;

	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_Undefined] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_PM_AirVent] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_PM_Separator] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_PM_ExpansionVessel] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_PM_TecBox] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_PM_SofteningAndDesalination] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_PM_SafetyValve] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_BC_RegulatingValve] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_BC_DpController] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_BC_BalAndCtrlValve] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_BC_PressureIndepCtrlValve] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_BC_CombinedDpCBalCtrlValve] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_BC_ControlValve] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_BC_SmartControlValve] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_BC_SmartDpC] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_BC_6WayControlValve] = false; // HYS-1149
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_BC_HUB] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_TC_ThermostaticValve] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_TC_FloorHeatingControl] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_TC_TapWaterControl] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Batch][PSC_ESC_ShutoffValve] = false;

	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_Undefined] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_PM_AirVent] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_PM_Separator] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_PM_ExpansionVessel] = true;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_PM_TecBox] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_PM_SofteningAndDesalination] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_PM_SafetyValve] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_BC_RegulatingValve] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_BC_DpController] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_BC_BalAndCtrlValve] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_BC_PressureIndepCtrlValve] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_BC_CombinedDpCBalCtrlValve] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_BC_ControlValve] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_BC_SmartControlValve] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_BC_SmartDpC] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_BC_6WayControlValve] = false; // HYS-1149
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_BC_HUB] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_TC_ThermostaticValve] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_TC_FloorHeatingControl] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_TC_TapWaterControl] = false;
	m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_ESC_ShutoffValve] = false;

	m_iStartPageToolsDockablePaneContextID = -1;
	// HYS-1376 : Import unit test
	m_bIsMenuImportEnable = false;
	m_bIsRibbonImportEnable = false;

	// For HMCalc debug purpose even in release mode.
	m_bHMCalcDebugPanelVisible = false;
	m_bHMCalcDebugVerifyAutority = true;
}

CMainFrame::~CMainFrame()
{
	if( NULL != m_pDlgDiversityFactor )
	{
		if( NULL != m_pDlgDiversityFactor->GetSafeHwnd() )
		{
			m_pDlgDiversityFactor->DestroyWindow();
		}

		delete m_pDlgDiversityFactor;
		m_pDlgDiversityFactor = NULL;
	}

	if( NULL != m_pDlgPrint )
	{
		if( NULL != m_pDlgPrint->GetSafeHwnd() )
		{
			m_pDlgPrint->DestroyWindow();
		}

		delete m_pDlgPrint;
		m_pDlgPrint = NULL;
	}

#ifdef _DEBUG
	if( NULL != m_pDlgDebug )
	{
		if( NULL != m_pDlgDebug->GetSafeHwnd() )
		{
			m_pDlgDebug->DestroyWindow();
		}

		delete m_pDlgDebug;
		m_pDlgDebug = NULL;
	}

	if( NULL != m_pTestRunnerDlg )
	{
		delete m_pTestRunnerModel;
		m_pTestRunnerModel = NULL;
	}

	if( NULL != m_pTestRunnerDlg )
	{
		delete m_pTestRunnerDlg;
		m_pTestRunnerDlg = NULL;
	}
	clear_type_info_cache();
#endif

	pMainFrame = NULL;
}


#ifdef _DEBUG

void CMainFrame::clear_type_info_cache()
{
	//clear the cache __type_info_root_node
	while (auto entry = InterlockedPopEntrySList(reinterpret_cast<PSLIST_HEADER>(&__type_info_root_node)))
	{
		free(entry);
		entry = NULL;
	}
}

void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump( CDumpContext &dc ) const
{
	CFrameWndEx::Dump( dc );
}
#endif //_DEBUG

BOOL CMainFrame::PreCreateWindow( CREATESTRUCT &cs )
{
	return CFrameWndEx::PreCreateWindow( cs );
}

BOOL CMainFrame::OnShowPopupMenu( CMFCPopupMenu *pMenuPopup )
{
	CFrameWndEx::OnShowPopupMenu( pMenuPopup );

	BOOL fReturn = FALSE;

	if( NULL != pMenuPopup )
	{
		CWnd *pWnd = pMenuPopup->GetMessageWnd();
		fReturn = ( 0 == ::SendMessage( pWnd->GetSafeHwnd(), WM_USER_SHOWPOPUPMENU, ( WPARAM )pMenuPopup, 0 ) ) ? TRUE : FALSE;
	}

	return fReturn;
}

void CMainFrame::InitViews()
{
	CView *pActiveView = GetActiveView();

	// The table with all different initialized views.
	// BE carefully order to add is fixed by 'RightViewList'.
	m_arrayViews.Add( pActiveView );			// Main view
	m_arrayViews.Add( new CRViewInfo );
	m_arrayViews.Add( new CRViewGen );
	m_arrayViews.Add( new CRViewHMSumm );
	m_arrayViews.Add( new CRViewProj );
	m_arrayViews.Add( new CRViewHMCalc );
	m_arrayViews.Add( new CRViewLDSumm );
	m_arrayViews.Add( new CRViewLogData );
	m_arrayViews.Add( new CRViewChart );
	m_arrayViews.Add( new CRViewSelProd );
	m_arrayViews.Add( new CRViewSSelBv );
	m_arrayViews.Add( new CRViewSSelDpC );
	m_arrayViews.Add( new CRViewSSelTrv );
	m_arrayViews.Add( new CRViewSSelCv );
	m_arrayViews.Add( new CRViewSSelPICv );
	m_arrayViews.Add( new CRViewSSelBCv );
	m_arrayViews.Add( new CRViewSSelHub );
	m_arrayViews.Add( new CRViewQuickMeas );
	m_arrayViews.Add( new CRViewSSelSeparator );
	m_arrayViews.Add( new CRViewSSelPM );
	m_arrayViews.Add( new CRViewStartPage );
	m_arrayViews.Add( new CRViewSSelDpCBCV );
	m_arrayViews.Add( new CRViewSSelSv );
	m_arrayViews.Add( new CRViewWizardSelPM );
	m_arrayViews.Add( new CRViewSSelSafetyValve );
	m_arrayViews.Add( new CRViewSSel6WayValve );
	m_arrayViews.Add( new CRViewSSelSmartControlValve );
	m_arrayViews.Add( new CRViewSSelSmartDpC );  // HYS-1938: TA-Smart Dp - 05 - Individual selection: right view

	CDocument *pCurrentDoc = GetActiveDocument();

	// Initialize CCreateContext to specify where will be located the new view.
	CCreateContext newContext;
	newContext.m_pNewViewClass = NULL;
	newContext.m_pNewDocTemplate = NULL;
	newContext.m_pLastView = NULL;
	newContext.m_pCurrentFrame = NULL;
	newContext.m_pCurrentDoc = pCurrentDoc;

	// Gets resized later.
	CRect rect( 0, 0, 0, 0 );

	for( int iView = 1; iView < m_arrayViews.GetCount(); iView++ )
	{
		// View will persist during all application live.
		// Will be destroyed when the application will be closed.
		m_arrayViews[iView]->Create( NULL, NULL, ( AFX_WS_DEFAULT_VIEW & ~WS_VISIBLE ),	rect, this,	AFX_IDW_PANE_FIRST + iView, &newContext );
	}
}

CView *CMainFrame::ActiveFormView( RightViewList eRightView )
{
	// Exchange activated view with the selected ones.
	if( eRightView < RightViewList::eRVFirst || eRightView > RightViewList::eRVLast || m_arrayViews.GetSize() <= eRightView )
	{
		return NULL;
	}

	m_eCurrentView = eRightView;
	CView *pActiveView = GetActiveView();
	CView *pNewView = m_arrayViews[(int)eRightView];

	if( pActiveView == pNewView )
	{
		return pActiveView;
	}

	// Swap view ID.
	UINT idActive = ::GetWindowLong( pActiveView->m_hWnd, GWL_ID );
	UINT idNew = ::GetWindowLong( pNewView->m_hWnd, GWL_ID );

	::SetWindowLong( pActiveView->m_hWnd, GWL_ID, idNew );
	::SetWindowLong( pNewView->m_hWnd, GWL_ID, idActive );

	// Display new view and hide previous.
	pActiveView->ShowWindow( SW_HIDE );
	pActiveView->EnableWindow( FALSE );
	pNewView->ShowWindow( SW_SHOW );
	pNewView->EnableWindow( TRUE );

	// Second argument will force a call to 'OnActivateView' for the new right view.
	SetActiveView( pNewView, TRUE );

	RecalcLayout();
	pNewView->Invalidate();

	// Activate Ribbon category.
	switch( eRightView )
	{
		case RightViewList::eRVInfo:
			m_wndRibbonBar.ShowContextCategories( ID_PRODUCTSEL_CATEGORY_BASE, FALSE );
			m_wndRibbonBar.ShowContextCategories( ID_LOGDATA_CATEGORY, FALSE );
			break;

		case RightViewList::eRVGeneral:
		case RightViewList::eRVHMSumm:
		case RightViewList::eRVProj:
		case RightViewList::eRVHMCalc:
		case RightViewList::eRVLDSumm:
		case RightViewList::eRVLogData:
			m_wndRibbonBar.ShowContextCategories( ID_PRODUCTSEL_CATEGORY_BASE, FALSE );
			m_wndRibbonBar.ShowContextCategories( ID_LOGDATA_CATEGORY, FALSE );
			break;

		case RightViewList::eRVChart:
			m_wndRibbonBar.ShowContextCategories( ID_LOGDATA_CATEGORY, TRUE );
			m_wndRibbonBar.SetActiveCategory( m_pLogDataCategory );
			m_wndRibbonBar.ShowContextCategories( ID_PRODUCTSEL_CATEGORY_BASE, FALSE );
			break;

		case RightViewList::eRVSelProd:
			m_wndRibbonBar.ShowContextCategories( ID_PRODUCTSEL_CATEGORY_BASE, FALSE );
			m_wndRibbonBar.ShowContextCategories( ID_LOGDATA_CATEGORY, FALSE );
			break;

		case RightViewList::eRVSSelBv:
		case RightViewList::eRVSSelDpC:
		case RightViewList::eRVSSelTrv:
		case RightViewList::eRVSSelCv:
		case RightViewList::eRVSSelPICv:
		case RightViewList::eRVSSelDpCBCV:
		case RightViewList::eRVSSelBCv:
		case RightViewList::eRVSSelHub:
		case RightViewList::eRVSSelSeparator:
		case RightViewList::eRVSSelSv:
		case RightViewList::eRVSSelSafetyValve:
		case RightViewList::eRVSSel6WayValve:
		case RightViewList::eRVSSelSmartControlValve:
		case RightViewList::eRVSSelSmartDpController:  // HYS-1938: TA-Smart Dp - 05 - Individual selection: right view
		{
			// Show the 'Product' panel under the 'Product selection tools' contextual category.
			m_wndRibbonBar.ShowContextCategories( ID_PRODUCTSEL_CATEGORY_BASE, TRUE );
			m_wndRibbonBar.SetActiveCategory( m_pProdSelToolsProductContextualCategory );

			// Show the 'Tech. parameters' panel under the 'Product selection tools' contextual category.
			// Remark: this panel is visible for all products in individual selection mode except pressurisation.
			int iCateg = m_wndRibbonBar.FindCategoryIndexByData( ID_SSEL_CATEGORY_BALCTRL );
			m_wndRibbonBar.ShowCategory( iCateg, TRUE );
			
			// Hide the 'Pressurisation prefs.' panel under the 'Product selection tools' contextual category.
			// Remark: this panel is visible for pressurisation in individual selection mode.
			iCateg = m_wndRibbonBar.FindCategoryIndexByData( ID_PRODUCTSEL_CATEGORY_PRESSMAINT );
			m_wndRibbonBar.ShowCategory( iCateg, FALSE );

			// Hide the 'Tech. parameters' panel under the 'Product selection tools' contextual category.
			// Remark: this panel is visible for pressurisation in wizard selection mode.
			iCateg = m_wndRibbonBar.FindCategoryIndexByData( ID_PRODUCTSEL_CATEGORY_PRESSMAINTWIZARD );
			m_wndRibbonBar.ShowCategory( iCateg, FALSE );
			
			m_wndRibbonBar.ShowContextCategories( ID_LOGDATA_CATEGORY, FALSE );

			break;
		}

		case RightViewList::eRVSSelPresureMaintenance:
		{
			// Show the 'Product' panel under the 'Product selection tools' contextual category.
			m_wndRibbonBar.ShowContextCategories( ID_PRODUCTSEL_CATEGORY_BASE, TRUE );
			m_wndRibbonBar.SetActiveCategory( m_pProdSelToolsProductContextualCategory );

			// Hide the 'Tech. parameters' panel under the 'Product selection tools' contextual category.
			// Remark: this panel is visible for all products in individual selection mode except pressurisation.
			int iCateg = m_wndRibbonBar.FindCategoryIndexByData( ID_SSEL_CATEGORY_BALCTRL );
			m_wndRibbonBar.ShowCategory( iCateg, FALSE );
			
			// Show the 'Pressurisation prefs.' panel under the 'Product selection tools' contextual category.
			// Remark: this panel is visible for pressurisation in individual selection mode.
			iCateg = m_wndRibbonBar.FindCategoryIndexByData( ID_PRODUCTSEL_CATEGORY_PRESSMAINT );
			m_wndRibbonBar.ShowCategory( iCateg, TRUE );

			// Hide the 'Tech. parameters' panel under the 'Product selection tools' contextual category.
			// Remark: this panel is visible for pressurisation in wizard selection mode.
			iCateg = m_wndRibbonBar.FindCategoryIndexByData( ID_PRODUCTSEL_CATEGORY_PRESSMAINTWIZARD );
			m_wndRibbonBar.ShowCategory( iCateg, FALSE );
			
			m_wndRibbonBar.ShowContextCategories( ID_LOGDATA_CATEGORY, FALSE );
			break;
		}

		case RightViewList::eRVWizardSelPM:
		{
			// Show the 'Product' panel under the 'Product selection tools' contextual category.
			m_wndRibbonBar.ShowContextCategories( ID_PRODUCTSEL_CATEGORY_BASE, TRUE );
			m_wndRibbonBar.SetActiveCategory( m_pProdSelToolsProductContextualCategory );

			// Hide the 'Tech. parameters' panel under the 'Product selection tools' contextual category.
			// Remark: this panel is visible for all products in individual selection mode except pressurisation.
			int iCateg = m_wndRibbonBar.FindCategoryIndexByData( ID_SSEL_CATEGORY_BALCTRL );
			m_wndRibbonBar.ShowCategory( iCateg, FALSE );
			
			// Hide the 'Pressurisation prefs.' panel under the 'Product selection tools' contextual category.
			// Remark: this panel is visible for pressurisation in individual selection mode.
			iCateg = m_wndRibbonBar.FindCategoryIndexByData( ID_PRODUCTSEL_CATEGORY_PRESSMAINT );
			m_wndRibbonBar.ShowCategory( iCateg, FALSE );

			// Show the 'Tech. parameters' panel under the 'Product selection tools' contextual category.
			// Remark: this panel is visible for pressurisation in wizard selection mode.
			iCateg = m_wndRibbonBar.FindCategoryIndexByData( ID_PRODUCTSEL_CATEGORY_PRESSMAINTWIZARD );
			m_wndRibbonBar.ShowCategory( iCateg, TRUE );
			
			m_wndRibbonBar.ShowContextCategories( ID_LOGDATA_CATEGORY, FALSE );
			break;
		}

		case RightViewList::eRVQuickMeas:
			m_wndRibbonBar.ShowContextCategories( ID_PRODUCTSEL_CATEGORY_BASE, FALSE );
			m_wndRibbonBar.ShowContextCategories( ID_LOGDATA_CATEGORY, FALSE );

		default:
			break;
	}

	// Refresh the Ribbon Bar.
	m_wndRibbonBar.ForceRecalcLayout();

	return pNewView;
}

CMainFrame::RightViewList CMainFrame::GetCurrentRightView( void )
{
	return m_eCurrentView;
}

CView *CMainFrame::GetSpecificView( CMainFrame::RightViewList eRightView )
{
	// Exchange activated view with the selected ones.
	if( eRightView < RightViewList::eRVFirst || eRightView > RightViewList::eRVLast || m_arrayViews.GetSize() <= eRightView )
	{
		return NULL;
	}

	return m_arrayViews[(int)eRightView];
}

void CMainFrame::FillPipeSeriesCB()
{
	// Variables.
	CTable *pTab = TASApp.GetpPipeDB()->GetPipeTab();

	// Verify initialize variables.
	if( NULL == pTab )
	{
		ASSERT_RETURN;
	}

	CDS_TechnicalParameter *pTechParam = TASApp.GetpTADS()->GetpTechParams();

	if( NULL == pTechParam )
	{
		ASSERT_RETURN;
	}

	// Remove all items.
	m_pCBPipeSeries->RemoveAllItems();

	CRank rkList;
	CString str;
	LPARAM lparam;
	
	// Create a rank sort list.
	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CTable ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		if( false == ( (CTable *)IDPtr.MP )->IsAvailable() )
		{
			continue;
		}

		if( true == ( (CTable *)IDPtr.MP )->IsDeleted() )
		{
			continue;
		}

		str = ( (CTable *)IDPtr.MP )->GetName();
		lparam = (LPARAM)IDPtr.MP;
		rkList.AddStrSort( str, 0, lparam, false );
	}

	// Insert all elements in the rank list into the combo box.
	int iCurPipe = 0; // 0 the default position to set if no default pipe is found
	int i = 0;
	IDPTR FirstElemCombo = _NULL_IDPTR;
	LPCTSTR lpDefaultPipes = pTechParam->GetDefaultPipeSerieID();

	// HYS-1499 : bValueFound tells us if the lpDefaultPipes is in the combo
	bool bValueFound = false;

	for( BOOL bContinue = rkList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rkList.GetNext( str, lparam ) )
	{
		CTable *pTabPipe = (CTable *)lparam;

		if( 0 == pTabPipe->GetItemCount( CLASS( CDB_Pipe ) ) )
		{
			continue;
		}

		if( true == pTabPipe->IsHidden() )
		{
			continue;
		}

		if( 0 == i )
		{
			FirstElemCombo = pTabPipe->GetIDPtr();
		}

		m_pCBPipeSeries->AddItemEx( (LPCTSTR)str, pTabPipe->GetIDPtr().ID );

		if( 0 == IDcmp( pTabPipe->GetIDPtr().ID, lpDefaultPipes ) )
		{
			iCurPipe = i;
			bValueFound = true;
		}

		i++;
	}

	// HYS-1499 : The current defaultpipeserie is not displayed in the combo. Change it.
	if( false == bValueFound )
	{
		pTechParam->SetDefaultPipeSerieID( FirstElemCombo.ID );
	}
	
	m_pCBPipeSeries->SelectItem( iCurPipe );
}

void CMainFrame::FillProductSelPipeSeriesCB()
{
	if( NULL == m_pRibbonComboSelPipeSeries || NULL == m_pCBPipeSeries || 0 == m_pCBPipeSeries->GetCount() )
	{
		return;
	}

	*m_pRibbonComboSelPipeSeries = *m_pCBPipeSeries;
	m_pRibbonComboSelPipeSeries->SelectItem( m_pCBPipeSeries->GetCurSel() );
}

void CMainFrame::FillProductSelPipeSizeCB()
{
	if( NULL == m_pRibbonComboSelPipeSeries )
	{
		return;
	}

	m_pRibbonComboSelPipeSize->RemoveAllItems();

	if( 0 == m_pRibbonComboSelPipeSeries->GetCount() )
	{
		ASSERT_RETURN;
	}

	// Find the correct Pipe series table in table "PIPE_TAB".
	int iPos = m_pRibbonComboSelPipeSeries->GetCurSel();

	if( iPos < 0 )
	{
		return;
	}

	CTable *pTab = dynamic_cast<CTable *>( TASApp.GetpPipeDB()->Get( m_pRibbonComboSelPipeSeries->GetItemID( iPos ) ).MP );

	if( NULL == pTab )
	{
		return;
	}

	// Fill the Pipe size combo box according to the selected pipe series.
	CRank List;

	for( IDPTR IDPtr = pTab->GetFirst(); *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		if( ( (CDB_Pipe *)IDPtr.MP )->IsSelectable( true ) )
		{
			List.Add( ( (CDB_Pipe *)IDPtr.MP )->GetName(), ( (CDB_Pipe *)IDPtr.MP )->GetIntDiameter(), (LPARAM)IDPtr.MP );
		}
	}

	// Add first all sizes.
	CString str = TASApp.LoadLocalizedString( IDS_COMBOTEXT_ALL_SIZES );
	m_pRibbonComboSelPipeSize->AddItemEx( str, _T( "" ) );
	LPARAM lparam;

	for( BOOL fContinue = List.GetFirst( str, lparam ); TRUE == fContinue; fContinue = List.GetNext( str, lparam ) )
	{
		CData *pData = (CData *)lparam;
		m_pRibbonComboSelPipeSize->AddItemEx( str, pData->GetIDPtr().ID );
	}

	if( m_pRibbonComboSelPipeSize->GetCount() > 0 )
	{
		m_pRibbonComboSelPipeSize->SelectItem( 0 );
	}
}

void CMainFrame::SetRibbonComboSelPipeSeries( CString strPipeSeriesID )
{
	if( NULL == m_pRibbonComboSelPipeSeries )
	{
		return;
	}

	int i = ( false == strPipeSeriesID.IsEmpty() ) ? m_pRibbonComboSelPipeSeries->FindItemID( strPipeSeriesID ) : 0;
	m_pRibbonComboSelPipeSeries->SelectItem( max( i, 0 ) );
	FillProductSelPipeSizeCB();
}

void CMainFrame::SetRibbonComboSelPipeSize( CString strPipeSizeID )
{
	if( NULL == m_pRibbonComboSelPipeSize )
	{
		return;
	}

	int i = ( false == strPipeSizeID.IsEmpty() ) ? m_pRibbonComboSelPipeSize->FindItemID( strPipeSizeID ) : 0;

	if( -1 != i )
	{
		m_pRibbonComboSelPipeSize->SelectItem( i );
	}
}

void CMainFrame::EnableRibbonComboSelPipeSeries( bool fEnable )
{
	m_fComboSelPipeSeriesEnabled = fEnable;
}

void CMainFrame::EnableRibbonComboSelPipeSize( bool fEnable )
{
	m_fComboSelPipeSizeEnabled = fEnable;
}

bool CMainFrame::VerifyEditTemp( CDS_WaterCharacteristic *pWC, CMFCRibbonEditEx *pRbnEditEx, double dCurrentSITemp, bool fDisplayWarning )
{
	EnterCriticalSection( &m_CriticalSection );
	bool fRet = false;

	// If value below the absolute zero do nothing because it is internally treated in the CMFCRibbonEditEx.
	if( dCurrentSITemp < -273.15 )
	{
		LeaveCriticalSection( &m_CriticalSection );
		return false;
	}

	// HYS-1194 : Do not compare with the freezing point when we are working with any other fluid.
	// In this case the feezing point is not known. 
	if( 0 == _tcscmp( pWC->GetAdditFamID(), _T( "OTHER_ADDIT" ) ) )
	{
		LeaveCriticalSection( &m_CriticalSection );
		return true;
	}

	// Get the freezing temperature.
	double dTempFreez = pWC->GetTfreez();

	// In case the fluid is in a solid phase...
	if( dCurrentSITemp <= dTempFreez )
	{
		if( true == fDisplayWarning )
		{
			CString str;
			FormatString( str, AFXMSG_BELOW_TFREEZ, WriteCUDouble( _U_TEMPERATURE, dTempFreez, true ) );
			AfxMessageBox( (LPCTSTR)str, MB_OK, -1 );

			CMFCRibbonRichEditCtrlNum *pRichEditCtrl = pRbnEditEx->GetNumericalEdit();
			pRichEditCtrl->SetFocus();
		}
	}
	else
	{
		// In case the fluid is in a liquid phase

		// Verify the temperature is not above the max Temp.
		CString AdditCharID = ( (CDB_StringID *)( TASApp.GetpTADB()->Get( pWC->GetAdditID() ).MP ) )->GetIDstr();
		CDB_AdditCharacteristic *pAdditChar = (CDB_AdditCharacteristic *)( TASApp.GetpTADB()->Get( AdditCharID ).MP );
		ASSERT( pAdditChar );
		double dDens = pWC->GetDens();
		double dKinVisc = pWC->GetKinVisc();
		double dSpecHead = pWC->GetSpecifHeat();
		double dVaporPressure = 0.0;

		if( SurfValReturn_enum::SV_ABOVEYMAX == pAdditChar->GetAdditProp( pWC->GetPcWeight(), dCurrentSITemp, &dDens, &dKinVisc, &dSpecHead, &dVaporPressure ) )
		{
			if( true == fDisplayWarning )
			{
				CString str;
				FormatString( str, AFXMSG_ABOVE_MAXTEMP, WriteCUDouble( _U_TEMPERATURE, pAdditChar->GetMaxT( pWC->GetPcWeight() ), true ) );
				AfxMessageBox( (LPCTSTR)str, MB_OK, -1 );

				CMFCRibbonRichEditCtrlNum *pRichEditCtrl = pRbnEditEx->GetNumericalEdit();
				pRichEditCtrl->SetFocus();
			}
		}
		else
		{
			fRet = true;
		}
	}

	LeaveCriticalSection( &m_CriticalSection );
	return fRet;
}

void CMainFrame::ResetMeasurementsLimits()
{
	SetMaxUpperDateTime( COleDateTime() );
	SetMaxLowerDateTime( COleDateTime( 9999, 1, 1, 1, 1, 1 ) );

	// Define the upper and lower limits allowed for the Piping tab (containing all CDS_HydroMod).
	CTableHM *pPipingTab = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );

	if( NULL != pPipingTab && pPipingTab->GetItemCount() > 0 )
	{
		COleDateTime clLowerTime;
		COleDateTime clUpperTime;
		pPipingTab->GetLowerUpperDataTime( clLowerTime, clUpperTime );

		if( GetMaxLowerDateTime() > clLowerTime )
		{
			SetMaxLowerDateTime( clLowerTime );
		}

		if( GetMaxUpperDateTime() < clUpperTime )
		{
			SetMaxUpperDateTime( clUpperTime );
		}
	}

	// Define the upper and lower limits allowed for the quick measurements.
	CDS_QMTable *pQMTab = dynamic_cast<CDS_QMTable *>( TASApp.GetpTADS()->GetpQuickMeasureTable() );

	if( NULL != pQMTab && pQMTab->GetItemCount() > 0 )
	{
		COleDateTime clLowerTime;
		COleDateTime clUpperTime;
		pQMTab->GetLowerUpperDataTime( clLowerTime, clUpperTime );

		if( GetMaxLowerDateTime() > clLowerTime )
		{
			SetMaxLowerDateTime( clLowerTime );
		}

		if( GetMaxUpperDateTime() < clUpperTime )
		{
			SetMaxUpperDateTime( clUpperTime );
		}
	}

	if( NULL != GetStartLimitProj() )
	{
		GetStartLimitProj()->SetEditText( L"" );
	}

	if( NULL != GetEndLimitProj() )
	{
		GetEndLimitProj()->SetEditText( L"" );
	}

	SetUpperDateTime( GetMaxUpperDateTime() );
	SetLowerDateTime( GetMaxLowerDateTime() );
	SetMeasurementExist( ( GetMaxUpperDateTime() != COleDateTime() ) ? true : false );
}

CString CMainFrame::GetRibbonComboSelPipeSeriesID()
{
	CData *pclData = NULL;
	int iPos = m_pRibbonComboSelPipeSeries->GetCurSel();

	if( iPos > -1 )
	{
		pclData = (CData *)( TASApp.GetpPipeDB()->Get( m_pRibbonComboSelPipeSeries->GetItemID( iPos ) ).MP );
	}

	CString strID = _T("");

	if( NULL != pclData )
	{
		strID = pclData->GetIDPtr().ID;
	}

	return strID;
}

CString CMainFrame::GetRibbonComboSelPipeSizeID()
{
	CData *pclData = NULL;
	int iPos = m_pRibbonComboSelPipeSize->GetCurSel();

	if( iPos > -1 )
	{
		pclData = (CData *)( TASApp.GetpPipeDB()->Get( m_pRibbonComboSelPipeSize->GetItemID( iPos ) ).MP );
	}

	CString strID = _T("");

	if( NULL != pclData )
	{
		strID = pclData->GetIDPtr().ID;
	}

	return strID;
}

void CMainFrame::InitializeRibbon()
{
	//-------------------------------------------------
	// INIT MAIN BUTTON
	m_MainButton.SetImage( IDB_MAIN );

	//-------------------------------------------------
	// MAIN PANEL
	m_wndRibbonBar.SetApplicationButton( &m_MainButton, CSize( 45, 45 ) );

	CMFCRibbonMainPanel *pMainPanel = m_wndRibbonBar.AddMainCategory( L"", IDB_STRIP_APPMENU_16, IDB_STRIP_APPMENU_32 );

	CMFCRibbonButton *pBtnNew = new CMFCRibbonButton( ID_APPMENU_NEW, TASApp.LoadLocalizedString( IDS_APPMENU_NEW ), eRbnMNew, eRbnMNew );
	CMFCRibbonButton *pBtnOpen = new CMFCRibbonButton( ID_APPMENU_OPEN, TASApp.LoadLocalizedString( IDS_APPMENU_OPEN ), eRbnMOpen, eRbnMOpen );
	CMFCRibbonButton *pBtnSave = new CMFCRibbonButton( ID_APPMENU_SAVE, TASApp.LoadLocalizedString( IDS_APPMENU_SAVE ), eRbnMSave, eRbnMSave );
	CMFCRibbonButton *pBtnSaveAs = new CMFCRibbonButton( ID_APPMENU_SAVE_AS, TASApp.LoadLocalizedString( IDS_APPMENU_SAVE_AS ), eRbnMSaveAs, eRbnMSaveAs );
	CMFCRibbonButton *pBtnStartPage = new CMFCRibbonButton( ID_APPMENU_START_PAGE, TASApp.LoadLocalizedString( IDS_APPMENU_START_PAGE ), eRbnMStartPage, eRbnMStartPage, TRUE);
	CMFCRibbonButton *pBtnPMWizard = new CMFCRibbonButton( ID_APPMENU_PM_WIZARD, TASApp.LoadLocalizedString( IDS_APPMENU_PM_WIZARD ), eRbnMPMWizard, eRbnMPMWizard, TRUE);

	CMFCRibbonButton *pBtnPrint = new CMFCRibbonButton( ID_APPMENU_PRINT, TASApp.LoadLocalizedString( IDS_APPMENU_PRINT ), eRbnMPrint, eRbnMPrint );
	CMFCRibbonLabel  *pBtnPrintLabel = new CMFCRibbonLabel( TASApp.LoadLocalizedString( IDS_APPMENU_PRINT ) );
	CMFCRibbonButton *pBtnSubPrint = new CMFCRibbonButton( ID_APPMENU_SUBPRINT, TASApp.LoadLocalizedString( IDS_APPMENU_PRINT ), eRbnMPrint, eRbnMPrint, TRUE );
	CMFCRibbonButton *pBtnSubPrintSetup = new CMFCRibbonButton( ID_APPMENU_PRINT_SETUP, TASApp.LoadLocalizedString( IDS_APPMENU_PRINT_SETUP ), eRbnMPrintSetup,	eRbnMPrintSetup, TRUE );

	CMFCRibbonButton *pBtnExportData = new CMFCRibbonButton( ID_APPMENU_EXPORT, TASApp.LoadLocalizedString( IDS_APPMENU_EXPORT ), eRbnMExport, eRbnMExport );
	
	// HYS-1605: Commented because of an issue with subitems. The last was not entirely visible
	//CMFCRibbonLabel  *pBtnExportLabel = new CMFCRibbonLabel( TASApp.LoadLocalizedString( IDS_APPMENU_EXPORT ) );
	CMFCRibbonButton *pBtnSubExportSelectedProcuts = new CMFCRibbonButton( ID_APPMENU_EXPORT_SELP, TASApp.LoadLocalizedString( IDS_APPMENU_EXPORT_SELP ), eRbnMExportExcel, eRbnMExportExcel, TRUE );
	
	// HYS-436: Export all selected in one sheet.
	CMFCRibbonButton *pBtnSubExportSelectedInOneSheet = new CMFCRibbonButton( ID_APPMENU_EXPORT_SELECTEDPRODUCTSINONESHEET, TASApp.LoadLocalizedString( IDS_APPMENU_EXPORT_INONESHEET ), eRbnMExportExcelOneSheet, eRbnMExportExcelOneSheet, TRUE );

#if defined(_DEBUG) && defined(_JSON_FEATURE_ENABLED)
	CMFCRibbonButton *pBtnSubExportHydronicCircuitsToJsonFile = new CMFCRibbonButton( ID_APPMENU_EXPORT_HM_JSON, TASApp.LoadLocalizedString( IDS_APPMENU_EXPORT_HM_JSON ), eRbnMExportHMToJsonFile, eRbnMExportHMToJsonFile, TRUE );
#endif

	CMFCRibbonButton *pBtnSubExportLoggedDataChart = new CMFCRibbonButton( ID_APPMENU_EXPORT_CHART, TASApp.LoadLocalizedString( IDS_APPMENU_EXPORT_CHART ), eRbnMExportChart, eRbnMExportChart, TRUE );
	CMFCRibbonButton *pBtnSubExportLoggedDataList = new CMFCRibbonButton( ID_APPMENU_EXPORT_LDLIST, TASApp.LoadLocalizedString( IDS_APPMENU_EXPORT_LDLIST ), eRbnMExportLDList, eRbnMExportLDList, TRUE );
	CMFCRibbonButton *pBtnSubExportQuickMeasurements = new CMFCRibbonButton( ID_APPMENU_EXPORT_QM, TASApp.LoadLocalizedString( IDS_APPMENU_EXPORT_QM ), eRbnMExportQM, eRbnMExportQM, TRUE );
	
	// HYS-1605: Export TA-Diagnostic.
	CMFCRibbonButton* pBtnSubExportTADiagnostic = new CMFCRibbonButton( ID_APPMENU_EXPORT_TADIAGNOSTIC, TASApp.LoadLocalizedString( IDS_APPMENU_EXPORT_TADIAGNOSTIC ), eRbnMExportTADiagnostic, eRbnMExportTADiagnostic, TRUE );
	CMFCRibbonButton *pBtnSubExportUserDefinedPipes = new CMFCRibbonButton( ID_APPMENU_EXPORT_PIPES, TASApp.LoadLocalizedString( IDS_APPMENU_EXPORT_PIPES ), eRbnMExportPipes, eRbnMExportPipes, TRUE );

	CMFCRibbonButton *pBtnImportData = new CMFCRibbonButton( ID_APPMENU_IMPORT, TASApp.LoadLocalizedString( IDS_APPMENU_IMPORT ), eRbnMImport, eRbnMImport, TRUE );
	CMFCRibbonButton *pBtnImportLabel = new CMFCRibbonLabel( TASApp.LoadLocalizedString( IDS_APPMENU_IMPORT ) );
	CMFCRibbonButton *pBtnSubImportUserDefinedPipes = new CMFCRibbonButton( ID_APPMENU_IMPORT_PIPES, TASApp.LoadLocalizedString( IDS_APPMENU_IMPORT_PIPES ), eRbnMImportPipes, eRbnMImportPipes, TRUE );
	CMFCRibbonButton *pBtnSubImportHydronicCircuits = new CMFCRibbonButton( ID_APPMENU_IMPORT_HM, TASApp.LoadLocalizedString( IDS_APPMENU_IMPORT_HM ), eRbnMImportHM, eRbnMImportHM, TRUE );

#if defined(_DEBUG) && defined(_JSON_FEATURE_ENABLED)
	CMFCRibbonButton *pBtnSubImportHydronicCircuitsFromJsonFile = new CMFCRibbonButton( ID_APPMENU_IMPORT_HM_JSON, TASApp.LoadLocalizedString( IDS_APPMENU_IMPORT_HM_JSON ), eRbnMImportHMFromJsonFile, eRbnMImportHMFromJsonFile, TRUE );
#endif

	SetToolTipRibbonElement( pBtnNew, IDS_APPMENU_NEW_TT );
	SetToolTipRibbonElement( pBtnOpen, IDS_APPMENU_OPEN_TT );
	SetToolTipRibbonElement( pBtnSave, IDS_APPMENU_SAVE_TT );
	SetToolTipRibbonElement( pBtnSaveAs, IDS_APPMENU_SAVE_AS_TT );

	SetToolTipRibbonElement( pBtnPMWizard, IDS_APPMENU_PM_WIZARD_HM_TT );

	SetToolTipRibbonElement( pBtnSubPrint, IDS_APPMENU_PRINT_TT );
	SetToolTipRibbonElement( pBtnSubPrintSetup, IDS_APPMENU_PRINT_SETUP_TT );

	SetToolTipRibbonElement( pBtnSubExportSelectedProcuts, IDS_APPMENU_EXPORT_SELP_TT );
	
	// HYS-436: Export all selected in one sheet.
	SetToolTipRibbonElement( pBtnSubExportSelectedInOneSheet, IDS_APPMENU_EXPORT_INONESHEET_TT );

#if defined(_DEBUG) && defined(_JSON_FEATURE_ENABLED)
	SetToolTipRibbonElement( pBtnSubExportHydronicCircuitsToJsonFile, IDS_APPMENU_EXPORT_HM_JSON_TT );
#endif

	SetToolTipRibbonElement( pBtnSubExportLoggedDataChart, IDS_APPMENU_EXPORT_CHART_TT );
	SetToolTipRibbonElement( pBtnSubExportLoggedDataList, IDS_APPMENU_EXPORT_LDLIST_TT );
	SetToolTipRibbonElement( pBtnSubExportQuickMeasurements, IDS_APPMENU_EXPORT_QM_TT );
	
	// HYS-1505: Export TA-Diagnostic and measurements.
	pBtnSubExportTADiagnostic->SetDescription( TASApp.LoadLocalizedString( IDS_APPMENU_EXPORT_TADIAGNOSTIC_TT ) );
	SetToolTipRibbonElement( pBtnSubExportUserDefinedPipes, IDS_APPMENU_EXPORT_PIPES_TT );

	SetToolTipRibbonElement( pBtnSubImportUserDefinedPipes, IDS_APPMENU_IMPORT_PIPES_TT );
	SetToolTipRibbonElement( pBtnSubImportHydronicCircuits, IDS_APPMENU_IMPORT_HM_TT );

#if defined(_DEBUG) && defined(_JSON_FEATURE_ENABLED)
	SetToolTipRibbonElement( pBtnSubImportHydronicCircuitsFromJsonFile, IDS_APPMENU_IMPORT_HM_JSON_TT );
#endif

	pMainPanel->Add( pBtnNew );
	pMainPanel->Add( pBtnOpen );
	pMainPanel->Add( pBtnSave );
	pMainPanel->Add( pBtnSaveAs );
	pMainPanel->Add( new CMFCRibbonSeparator( TRUE ) );
	pMainPanel->Add( pBtnStartPage );
	pMainPanel->Add( new CMFCRibbonSeparator( TRUE ) );
	pMainPanel->Add( pBtnPMWizard );
	pMainPanel->Add( new CMFCRibbonSeparator( TRUE ) );

	pMainPanel->Add( pBtnPrint );
	pBtnPrint->AddSubItem( pBtnPrintLabel );
	pBtnPrint->AddSubItem( pBtnSubPrint );
	pBtnPrint->AddSubItem( pBtnSubPrintSetup );

	pMainPanel->Add( pBtnExportData );
	pBtnExportData->AddSubItem( pBtnSubExportSelectedProcuts );

	// HYS-436: Export all selected in one sheet.
	pBtnExportData->AddSubItem( pBtnSubExportSelectedInOneSheet );

#if defined(_DEBUG) && defined(_JSON_FEATURE_ENABLED)
	pBtnExportData->AddSubItem( pBtnSubExportHydronicCircuitsToJsonFile );
#endif

	pBtnExportData->AddSubItem( pBtnSubExportLoggedDataChart );
	pBtnExportData->AddSubItem( pBtnSubExportLoggedDataList );
	pBtnExportData->AddSubItem( pBtnSubExportQuickMeasurements );
	
	// HYS-1605: Export TA-Diagnostic and measurements.
	pBtnExportData->AddSubItem( pBtnSubExportTADiagnostic );
	pBtnExportData->AddSubItem( pBtnSubExportUserDefinedPipes );

	pMainPanel->Add( pBtnImportData );
	pBtnImportData->AddSubItem( pBtnImportLabel );
	pBtnImportData->AddSubItem( pBtnSubImportUserDefinedPipes );
	pBtnImportData->AddSubItem( pBtnSubImportHydronicCircuits );

#if defined(_DEBUG) && defined(_JSON_FEATURE_ENABLED)
	pBtnImportData->AddSubItem( pBtnSubImportHydronicCircuitsFromJsonFile );
#endif

	pMainPanel->AddRecentFilesList( TASApp.LoadLocalizedString( IDS_RBN_RECENTDOCS ) );
	pMainPanel->AddToBottom( new CMFCRibbonMainPanelButton( ID_APPMENU_EXIT, TASApp.LoadLocalizedString( IDS_APPMENU_EXIT ), eRbnMExit ) );

	//-------------------------------------------------
	// HOME GROUP
	m_pHomeCategory = m_wndRibbonBar.AddCategory( TASApp.LoadLocalizedString( IDS_RBN_HOME ), IDB_STRIP_HOME_16, IDB_STRIP_HOME_32 );

	// Default units.
	m_pPanelHomeDefaultUnits = m_pHomeCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_H_DEFUNITS ),
							   m_pHomeCategory->GetSmallImages().ExtractIcon( eSHPanelDefUnits ) );
	SetToolTipRibbonElement( &m_pPanelHomeDefaultUnits->GetLaunchButton(), IDS_RBN_H_DEFUNITS_TT );
	{
		CMFCRibbonComboBox *pComboFlow = new CMFCRibbonComboBox( ID_RBN_H_DU_FLOW, 0, -1, TASApp.LoadLocalizedString( IDS_RBN_H_DU_FLOW ) );
		CMFCRibbonComboBox *pComboDp = new CMFCRibbonComboBox( ID_RBN_H_DU_DIFFPRESS, 0, -1, TASApp.LoadLocalizedString( IDS_RBN_H_DU_DIFFPRESS ) );
		CMFCRibbonComboBox *pComboTemp = new CMFCRibbonComboBox( ID_RBN_H_DU_TEMP, 0, -1, TASApp.LoadLocalizedString( IDS_RBN_H_DU_TEMP ) );

		SetToolTipRibbonElement( pComboFlow, IDS_RBN_H_DU_FLOW_TT );
		SetToolTipRibbonElement( pComboDp, IDS_RBN_H_DU_DIFFPRESS_TT );
		SetToolTipRibbonElement( pComboTemp, IDS_RBN_H_DU_TEMP_TT );

		_FillRibbonUnitCB( pComboFlow, _U_FLOW );
		_FillRibbonUnitCB( pComboDp, _U_DIFFPRESS );
		_FillRibbonUnitCB( pComboTemp, _U_TEMPERATURE );

		m_pPanelHomeDefaultUnits->Add( pComboFlow );
		m_pPanelHomeDefaultUnits->Add( pComboDp );
		m_pPanelHomeDefaultUnits->Add( pComboTemp );

		m_pPanelHomeDefaultUnits->SetJustifyColumns();
		m_pPanelHomeDefaultUnits->EnableLaunchButton( ID_RBN_H_DEFUNITS );
	}

	// Pipes series Group Box.
	m_pPanelHomePipeSeries = m_pHomeCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_H_PIPESERIES ),
							 m_pHomeCategory->GetSmallImages().ExtractIcon( eSHPanelPipeSeries ) );
	SetToolTipRibbonElement( &m_pPanelHomePipeSeries->GetLaunchButton(), IDS_RBN_H_PIPESERIES_TT );
	{
		m_pCBPipeSeries = new CMFCRibbonComboBoxEx( ID_RBN_H_PS_DEFPIPESERIES, 0, 160 );
		SetToolTipRibbonElement( m_pCBPipeSeries, IDS_RBN_H_PS_DEFPIPESERIES_TT );

		FillPipeSeriesCB();

		m_pPanelHomePipeSeries->Add( new CMFCRibbonLabel( TASApp.LoadLocalizedString( IDS_RBN_H_PS_DEFPIPESERIES ) ) );
		m_pPanelHomePipeSeries->Add( m_pCBPipeSeries );

		m_pPanelHomePipeSeries->SetJustifyColumns();
		m_pPanelHomePipeSeries->EnableLaunchButton( ID_RBN_H_PIPESERIES );
	}

	// Technical parameters group box.
	m_pPanelHomeTechnicalParameters = m_pHomeCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_H_TECHPARAM ),
									  m_pHomeCategory->GetSmallImages().ExtractIcon( eSHPanelTechParam ) );
	SetToolTipRibbonElement( &m_pPanelHomeTechnicalParameters->GetLaunchButton(), IDS_RBN_H_TECHPARAM_TT );
	{
		CDS_TechnicalParameter *pTechP = (CDS_TechnicalParameter *)( TASApp.GetpTADB()->Get( L"PARAM_TECH" ).MP );

		// Instantiate new MFC ribbon edit.
		m_pEBSizeAboveHome = new CMFCRibbonEditEx( ID_RBN_H_TP_SIZEABV, 30, TASApp.LoadLocalizedString( IDS_RBN_H_TP_SIZEABV ), eSHDevSizeAb );
		m_pEBSizeBelowHome = new CMFCRibbonEditEx( ID_RBN_H_TP_SIZEBLW, 30, TASApp.LoadLocalizedString( IDS_RBN_H_TP_SIZEBLW ), eSHDevSizeBl );
		m_pEBPresTargetHome = new CMFCRibbonEditEx( ID_RBN_H_TP_PD_TARGET, 95, TASApp.LoadLocalizedString( IDS_RBN_H_TP_PD_TARGET ), eSHPdropTarg );
		m_pEBPresMaxHome = new CMFCRibbonEditEx( ID_RBN_H_TP_PD_MAX, 95, TASApp.LoadLocalizedString( IDS_RBN_H_TP_PD_MAX ), eSHPdropMax );
		m_pEBVelTargetHome = new CMFCRibbonEditEx( ID_RBN_H_TP_WV_TARGET, 65, TASApp.LoadLocalizedString( IDS_RBN_H_TP_WV_TARGET ), eSHWVelTarg );
		m_pEBVelMaxHome = new CMFCRibbonEditEx( ID_RBN_H_TP_WV_MAX, 65, TASApp.LoadLocalizedString( IDS_RBN_H_TP_WV_MAX ), eSHWVelMax );

		m_pEBSizeAboveHome->EnableSpinButtons( 0, pTechP->GetMaxSizeShift() );
		m_pEBSizeBelowHome->EnableSpinButtons( 0, pTechP->GetMaxSizeShift() );

		// Create it.
		m_pEBSizeAboveHome->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pEBSizeBelowHome->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pEBPresTargetHome->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pEBPresMaxHome->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pEBVelTargetHome->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pEBVelMaxHome->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );

		SetToolTipRibbonElement( m_pEBSizeAboveHome, IDS_RBN_H_TP_SIZEABV_TT );
		SetToolTipRibbonElement( m_pEBSizeBelowHome, IDS_RBN_H_TP_SIZEBLW_TT );
		SetToolTipRibbonElement( m_pEBPresTargetHome, IDS_RBN_H_TP_PD_TARGET_TT );
		SetToolTipRibbonElement( m_pEBPresMaxHome, IDS_RBN_H_TP_PD_MAX_TT );
		SetToolTipRibbonElement( m_pEBVelTargetHome, IDS_RBN_H_TP_WV_TARGET_TT );
		SetToolTipRibbonElement( m_pEBVelMaxHome, IDS_RBN_H_TP_WV_MAX_TT );

		m_pPanelHomeTechnicalParameters->Add( new CMFCRibbonLabel( TASApp.LoadLocalizedString( IDS_RBN_H_TP_DEVSIZE ) ) );
		m_pPanelHomeTechnicalParameters->Add( m_pEBSizeAboveHome );
		m_pPanelHomeTechnicalParameters->Add( m_pEBSizeBelowHome );
		m_pPanelHomeTechnicalParameters->AddSeparator();
		m_pPanelHomeTechnicalParameters->Add( new CMFCRibbonLabel( TASApp.LoadLocalizedString( IDS_RBN_H_TP_PIPEPRESSDROP ) ) );
		m_pPanelHomeTechnicalParameters->Add( m_pEBPresTargetHome );
		m_pPanelHomeTechnicalParameters->Add( m_pEBPresMaxHome );
		m_pPanelHomeTechnicalParameters->AddSeparator();
		m_pPanelHomeTechnicalParameters->Add( new CMFCRibbonLabel( TASApp.LoadLocalizedString( IDS_RBN_H_TP_PIPEWTVEL ) ) );
		m_pPanelHomeTechnicalParameters->Add( m_pEBVelTargetHome );
		m_pPanelHomeTechnicalParameters->Add( m_pEBVelMaxHome );

		m_pPanelHomeTechnicalParameters->SetJustifyColumns();
		m_pPanelHomeTechnicalParameters->EnableLaunchButton( ID_RBN_H_TECHPARAM );
	}

	// Option group box.
	m_pPanelHomeOptions = m_pHomeCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_H_OPTIONS ),
						  m_pHomeCategory->GetSmallImages().ExtractIcon( eSHPanelOptions ) );
	{
		CMFCRibbonComboBox *pComboLang = new CMFCRibbonComboBox( ID_RBN_H_O_LANG, 0, -1, TASApp.LoadLocalizedString( IDS_RBN_H_O_LANG ) );
		CMFCRibbonButton *pBtnDocs = new CMFCRibbonButton( ID_RBN_H_O_DOCS, TASApp.LoadLocalizedString( IDS_RBN_H_O_DOCS ), eSHLanguageDoc );
		CMFCRibbonButton *pBtnAutoSave = new CMFCRibbonCheckBox( ID_RBN_H_O_AUTOSAV, TASApp.LoadLocalizedString( IDS_RBN_H_O_AUTOSAV ) );
		CMFCRibbonButton *pBtnUserRef = new CMFCRibbonButton( ID_RBN_H_O_USERREF, TASApp.LoadLocalizedString( IDS_RBN_H_O_USERREF ), eSHUserRef, eSHUserRef );
		SetToolTipRibbonElement( pComboLang, IDS_RBN_H_O_LANG_TT );
		SetToolTipRibbonElement( pBtnDocs, IDS_RBN_H_O_DOCS_TT );
		SetToolTipRibbonElement( pBtnAutoSave, IDS_RBN_H_O_AUTOSAV_TT );
		SetToolTipRibbonElement( pBtnUserRef, IDS_RBN_H_O_USERREF_TT );

		_FillRibbonLanguageCB( pComboLang );

		m_pPanelHomeOptions->Add( pComboLang );
		m_pPanelHomeOptions->Add( pBtnDocs );
		m_pPanelHomeOptions->Add( pBtnAutoSave );
		m_pPanelHomeOptions->Add( pBtnUserRef );

		m_pPanelHomeOptions->SetJustifyColumns();
	}

	// Information group box.
	m_pPanelHomeInformation = m_pHomeCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_H_INFO ),
							  m_pHomeCategory->GetSmallImages().ExtractIcon( eSHPanelInfo ) );
	{
		CMFCRibbonButton *pBReleaseNotes = new CMFCRibbonButton( ID_RBN_H_I_NOTES, TASApp.LoadLocalizedString( IDS_RBN_H_I_NOTES ), eSHReleaseNotes, eSHReleaseNotes );
		CMFCRibbonButton *pBTipOfTheDay = new CMFCRibbonButton( ID_RBN_H_I_TIPOFTHEDAY, TASApp.LoadLocalizedString( IDS_RBN_H_I_TIPOFTHEDAY ), eSHTip, eSHTip );
		CMFCRibbonButton *pBAbout = new CMFCRibbonButton( ID_RBN_H_I_ABOUT, TASApp.LoadLocalizedString( IDS_RBN_H_I_ABOUT ), eSHAbout, eSHAbout );
		CMFCRibbonButton *pBManuel = new CMFCRibbonButton( ID_RBN_USERMANUAL, TASApp.LoadLocalizedString( IDS_RBN_H_I_USERMANUAL ), eSHManual, eSHManual );
		SetToolTipRibbonElement( pBReleaseNotes, IDS_RBN_H_I_NOTES_TT );
		SetToolTipRibbonElement( pBTipOfTheDay, IDS_RBN_H_I_TIPOFTHEDAY_TT );
		SetToolTipRibbonElement( pBAbout, IDS_RBN_H_I_ABOUT_TT );
		SetToolTipRibbonElement( pBManuel, ID_RBN_USERMANUAL );

		m_pPanelHomeInformation->Add( pBManuel );
		m_pPanelHomeInformation->Add( pBReleaseNotes );
		m_pPanelHomeInformation->Add( pBTipOfTheDay );
		m_pPanelHomeInformation->Add( pBAbout );

		m_pPanelHomeInformation->SetJustifyColumns();
	}

	bool bDisplayPanelHomeTest = TASApp.IsTestInRelease();

#ifdef DEBUG
	bDisplayPanelHomeTest = true;
#endif

	if( true == bDisplayPanelHomeTest )
	{
		m_pPanelHomeTest = m_pHomeCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_HOME_TEST ), m_pHomeCategory->GetSmallImages().ExtractIcon( eSHTest ) );
		{
			CMFCRibbonButton *pUnitTest = new CMFCRibbonButton( ID_RBN_TEST_UNIT, TASApp.LoadLocalizedString( IDS_RBN_HOME_UNITTEST ), eSHTestUnit, eSHTestUnit );
			CMFCRibbonButton *pTestSpecialActions = new CMFCRibbonButton( ID_RBN_TEST_SPECIALACTIONS, TASApp.LoadLocalizedString( IDS_RBN_HOME_SPECIALACTIONS ), eSHTestSpecialAcions, eSHTestSpecialAcions );
			m_pPanelHomeTest->Add( pUnitTest );
			m_pPanelHomeTest->Add( pTestSpecialActions );
			m_pPanelHomeTest->SetJustifyColumns();
		}
	}

	//-------------------------------------------------
	// PROJECT SELECTION GROUP.
	m_pProjectCategory = m_wndRibbonBar.AddCategory( TASApp.LoadLocalizedString( IDS_RBN_PROJECT ), IDB_STRIP_PROJECT_16, IDB_STRIP_PROJECT_32 );

	// Hydronic panel.
	m_pPanelProjectProjectInfo = m_pProjectCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_P_PROJINFO ),
			m_pProjectCategory->GetSmallImages().ExtractIcon( eSPPanelProjInfo ) );

	SetToolTipRibbonElement( &m_pPanelProjectProjectInfo->GetLaunchButton(), IDS_RBN_P_PROJINFO_TT );
	{
		m_pEBProjName = new CMFCRibbonEditEx( ID_RBN_P_PI_PROJNAME, 150, L"", -1, CMFCRibbonEditEx::eTypeRichEditCtrl::eCBIRestrEditCtrl );
		m_pCBCBICompatible = new CMFCRibbonCheckBox( ID_RBN_P_PI_CBICOMPATIBLE, TASApp.LoadLocalizedString( IDS_RBN_P_PI_CBICOMPATIBLE ) );
		CMFCRibbonButton *pBtnHMCalc = new CMFCRibbonButton( ID_RBN_P_PI_HMCALC, TASApp.LoadLocalizedString( IDS_RBN_P_PI_HMCALC ), eSPHMCalc, eSPHMCalc );

		SetToolTipRibbonElement( m_pEBProjName, IDS_RBN_P_PROJINFO_TT );
		SetToolTipRibbonElement( m_pCBCBICompatible, IDS_RBN_P_PI_CBICOMPATIBLE_TT );
		SetToolTipRibbonElement( pBtnHMCalc, IDS_RBN_P_PI_HMCALC_TT );

		m_pPanelProjectProjectInfo->Add( new CMFCRibbonLabel( TASApp.LoadLocalizedString( IDS_RBN_P_PI_PROJNAME ) ) );
		m_pPanelProjectProjectInfo->Add( m_pEBProjName );
		m_pPanelProjectProjectInfo->Add( m_pCBCBICompatible );
		m_pPanelProjectProjectInfo->AddSeparator();
		m_pPanelProjectProjectInfo->Add( pBtnHMCalc );

		m_pPanelProjectProjectInfo->SetJustifyColumns();
		m_pPanelProjectProjectInfo->EnableLaunchButton( ID_RBN_P_PROJINFO );
	}

	// Project parameters group box.
	m_pPanelProjectParameters = m_pProjectCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_P_PROJPARAM ),
			m_pProjectCategory->GetSmallImages().ExtractIcon( eSPPanelProjParam ) );

	{
		m_pChBFreezeHMCalc = new CMFCRibbonCheckBox( ID_RBN_P_PP_FREEZE, TASApp.LoadLocalizedString( IDS_RBN_P_PP_FREEZE ) );
		m_pChBHNAutoCheck = new CMFCRibbonCheckBox( ID_RBN_P_PP_HNAUTOCHECK, TASApp.LoadLocalizedString( IDS_RBN_P_PP_HNAUTOCHECK ) );
		m_pCBProjectApplicationType = new CMFCRibbonComboBoxEx( ID_RBN_P_PP_PROJECTTYPE, 0, -1, TASApp.LoadLocalizedString( IDS_RBN_P_PP_PROJECTTYPE ), eSPProjectType );

		SetToolTipRibbonElement( m_pChBFreezeHMCalc, IDS_RBN_P_PP_FREEZE_TT );
		SetToolTipRibbonElement( m_pChBHNAutoCheck, IDS_RBN_P_PP_HNAUTOCHECK_TT );
		SetToolTipRibbonElement( m_pCBProjectApplicationType, IDS_RBN_P_PP_PROJECTTYPE_TT );

		m_pCBProjectApplicationType->RemoveAllItems();
		m_pCBProjectApplicationType->AddItem( (LPCTSTR)TASApp.LoadLocalizedString( IDS_RBN_PRJTYPE_HEATING ), Heating );
		m_pCBProjectApplicationType->AddItem( (LPCTSTR)TASApp.LoadLocalizedString( IDS_RBN_PRJTYPE_COOLING ), Cooling );
		
		CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
		ProjectType eProjectApplicationType = pTechP->GetProjectApplicationType();

		if( m_pCBProjectApplicationType->GetCount() > 0 )
		{
			m_pCBProjectApplicationType->SelectItem( (DWORD_PTR)eProjectApplicationType );
		}


		m_pPanelProjectParameters->Add( m_pChBFreezeHMCalc );
		m_pPanelProjectParameters->Add( m_pChBHNAutoCheck );
		m_pPanelProjectParameters->Add( m_pCBProjectApplicationType );

		m_pPanelProjectParameters->SetJustifyColumns();
	}

	// Project tools group box.
	m_pPanelProjectTools = m_pProjectCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_P_PROJTOOLS ),
			m_pProjectCategory->GetSmallImages().ExtractIcon( eSPPanelProjTools ) );

	{
		CMFCRibbonButton *pBtnProjectCheck = new CMFCRibbonButton( ID_RBN_P_PT_HNCHECK, TASApp.LoadLocalizedString( IDS_RBN_P_PT_HNCHECK ), eSPProjectCheck,
				eSPProjectCheck );
		CMFCRibbonButton *pBtnImportHM = new CMFCRibbonButton( ID_RBN_P_PT_IMPORT_HM, TASApp.LoadLocalizedString( IDS_RBN_P_PT_IMPORT_HM ), eSPImportHM, eSPImportHM );
		CMFCRibbonButton *pBtnSearchReplace = new CMFCRibbonButton( ID_RBN_P_PT_SR, TASApp.LoadLocalizedString( IDS_RBN_P_PT_SR ), eSPSearchReplace, eSPSearchReplace );
		CMFCRibbonButton *pBtnDiversityFactor = new CMFCRibbonButton( ID_RBN_P_PT_DF, TASApp.LoadLocalizedString( IDS_RBN_P_PT_DF ), eSPDiversityFactor,
				eSPDiversityFactor );

		//SetToolTipRibbonElement( pBtnProjectCheck, IDS_RBN_P_PT_IMPORT_HM_TT );
		SetToolTipRibbonElement( pBtnImportHM, IDS_RBN_P_PT_IMPORT_HM_TT );
		SetToolTipRibbonElement( pBtnSearchReplace, IDS_RBN_P_PT_SR_TT );
		SetToolTipRibbonElement( pBtnDiversityFactor, IDS_RBN_P_PT_DF_TT );

		m_pPanelProjectTools->Add( pBtnProjectCheck );
		m_pPanelProjectTools->Add( pBtnImportHM );
		m_pPanelProjectTools->Add( pBtnSearchReplace );
		m_pPanelProjectTools->Add( pBtnDiversityFactor );

		m_pPanelProjectTools->SetJustifyColumns();
	}

	// Panel water characteristics.
	m_pPanelProjectFluidCharacteristics = m_pProjectCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_P_WATERCHAR ),
			m_pProjectCategory->GetSmallImages().ExtractIcon( eSPPanelWc ) );

	// ! The IDS is correct it is just also used in the product selection. IDS_RBN_P_WATERCHAR_TT will not be used
	SetToolTipRibbonElement( &m_pPanelProjectFluidCharacteristics->GetLaunchButton(), IDS_RBN_PS_WATERCHAR_TT );
	{
		// Build new water characteristic strings.
		CString str1, str2, str3;
		TASApp.GetpTADS()->GetpWCForProject()->BuildWaterStringsRibbonBar( str1, str2, str3 );
		CString str = str1;

		if( false == str2.IsEmpty() )
		{
			str = str1 + _T("; ") + str2;
		}

		m_pLBWaterCharInfo1Proj = new CMFCRibbonLabel( str );
		m_pEBWaterCharTempProj = new CMFCRibbonEditEx( ID_RBN_P_WC_TEMP, 65, TASApp.LoadLocalizedString( IDS_RBN_P_WC_TEMP ), eSPTemp );
		m_pEBWaterCharDTProj = new CMFCRibbonEditEx( ID_RBN_P_WC_DT, 65, TASApp.LoadLocalizedString( IDS_RBN_P_WC_DT ) );

		m_pEBWaterCharTempProj->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pEBWaterCharDTProj->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );

		SetToolTipRibbonElement( m_pEBWaterCharTempProj, IDS_RBN_P_WC_TEMP_TT );
		SetToolTipRibbonElement( m_pEBWaterCharDTProj, IDS_RBN_PS_WC_DT_TT );

		m_pPanelProjectFluidCharacteristics->Add( m_pLBWaterCharInfo1Proj );
		m_pPanelProjectFluidCharacteristics->Add( m_pEBWaterCharTempProj );
		m_pPanelProjectFluidCharacteristics->Add( m_pEBWaterCharDTProj );

		m_pPanelProjectFluidCharacteristics->SetJustifyColumns();
		m_pPanelProjectFluidCharacteristics->SetCenterColumnVert();
		m_pPanelProjectFluidCharacteristics->EnableLaunchButton( ID_RBN_P_WATERCHAR );
	}

	// Measure Date & Time Group Box.
	m_pPanelProjectMeasurementPeriod = m_pProjectCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_P_MEASPERIOD ),
			m_pProjectCategory->GetSmallImages().ExtractIcon( eSPMeasPeriod ) );
	{
		CMFCRibbonLabel *pLBDateTimeBound = new CMFCRibbonLabel( TASApp.LoadLocalizedString( IDS_RBN_P_MP_LIMITS ) );
		m_pEBStartLimitProj = new CMFCRibbonEdit( ID_RBN_P_MP_START, 115, TASApp.LoadLocalizedString( IDS_RBN_P_MP_START ), eSPStart );
		m_pEBEndLimitProj = new CMFCRibbonEdit( ID_RBN_P_MP_END, 115, TASApp.LoadLocalizedString( IDS_RBN_P_MP_END ), eSPEnd );
		CMFCRibbonButton *pDateTime = new CMFCRibbonButton( ID_RBN_P_MP_DATETIME, TASApp.LoadLocalizedString( IDS_RBN_P_MP_CHANGELIMITS ), eSPChangePeriod,
				eSPChangePeriod );

		SetToolTipRibbonElement( m_pEBStartLimitProj, IDS_RBN_P_MP_START_TT );
		SetToolTipRibbonElement( m_pEBEndLimitProj, IDS_RBN_P_MP_END_TT );
		SetToolTipRibbonElement( pDateTime, IDS_RBN_P_MP_CHANGELIMITS_TT );

		m_pPanelProjectMeasurementPeriod->Add( pLBDateTimeBound );
		m_pPanelProjectMeasurementPeriod->Add( m_pEBStartLimitProj );
		m_pPanelProjectMeasurementPeriod->Add( m_pEBEndLimitProj );
		m_pPanelProjectMeasurementPeriod->AddSeparator();
		m_pPanelProjectMeasurementPeriod->Add( pDateTime );

		m_pPanelProjectMeasurementPeriod->SetJustifyColumns();
	}

	//-------------------------------------------------
	// TOOLS SELECTION GROUP.
	m_pToolsCategory = m_wndRibbonBar.AddCategory( TASApp.LoadLocalizedString( IDS_RBN_TOOLS ), IDB_STRIP_TOOLS_16, IDB_STRIP_TOOLS_32 );

	// Hydronic panel.
	m_pPanelToolsHydronic = m_pToolsCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_T_HYDRONIC ),
							m_pToolsCategory->GetSmallImages().ExtractIcon( eSTPanelHydronic ) );
	{
		CMFCRibbonButton *pRibbonViscCorButton = new CMFCRibbonButton( ID_RBN_T_H_VISCCOR, TASApp.LoadLocalizedString( IDS_RBN_T_H_VISCCOR ), eSTViscCorr,
				eSTViscCorr );
		CMFCRibbonButton *pRibbonPipePressDropButton = new CMFCRibbonButton( ID_RBN_T_H_PIPEPRESSDROP, TASApp.LoadLocalizedString( IDS_RBN_T_H_PIPEPRESSDROP ),
				eSTPdrop, eSTPdrop );
		CMFCRibbonButton *pRibbonKvCalcButton = new CMFCRibbonButton( ID_RBN_T_H_KVCALC, TASApp.LoadLocalizedString( IDS_RBN_T_H_KVCALC ), eSTKvCalc, eSTKvCalc );
		CMFCRibbonButton *pRibbonCalculatorButton = new CMFCRibbonButton( ID_RBN_T_H_CALCULATOR, TASApp.LoadLocalizedString( IDS_RBN_T_H_CALCULATOR ), eSTCalculator,
				eSTCalculator );
		SetToolTipRibbonElement( pRibbonViscCorButton, IDS_RBN_T_H_VISCCOR_TT );
		SetToolTipRibbonElement( pRibbonPipePressDropButton, IDS_RBN_T_H_PIPEPRESSDROP_TT );
		SetToolTipRibbonElement( pRibbonKvCalcButton, IDS_RBN_T_H_KVCALC_TT );
		SetToolTipRibbonElement( pRibbonCalculatorButton, IDS_RBN_T_H_CALCULATOR_TT );

		m_pPanelToolsHydronic->Add( pRibbonViscCorButton );
		m_pPanelToolsHydronic->Add( pRibbonPipePressDropButton );
		m_pPanelToolsHydronic->Add( pRibbonKvCalcButton );
		m_pPanelToolsHydronic->Add( pRibbonCalculatorButton );

		m_pPanelToolsHydronic->SetJustifyColumns();
	}

	// Other panel.
	m_pPanelToolsOther = m_pToolsCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_T_OTHER ),
						 m_pToolsCategory->GetSmallImages().ExtractIcon( eSTPanelOther ) );
	{
		CMFCRibbonButton *pRibbonUnitConvButton = new CMFCRibbonButton( ID_RBN_T_O_UNITCONV, TASApp.LoadLocalizedString( IDS_RBN_T_O_UNITCONV ), eSTUnitConv,
				eSTUnitConv );
		CMFCRibbonButton *pRibbonTALinkButton = new CMFCRibbonButton( ID_RBN_T_O_TALINK, TASApp.LoadLocalizedString( IDS_RBN_T_O_TALINK ), eSTTALink, eSTTALink );
		SetToolTipRibbonElement( pRibbonUnitConvButton, IDS_RBN_T_O_UNITCONV_TT );
		SetToolTipRibbonElement( pRibbonTALinkButton, IDS_RBN_T_O_TALINK_TT );
		m_pPanelToolsOther->Add( pRibbonUnitConvButton );
		m_pPanelToolsOther->Add( pRibbonTALinkButton );
		m_pPanelToolsOther->SetJustifyColumns();
	}

	//-------------------------------------------------
	// COMMUNICATION SELECTION GROUP.
	m_pCommunicationCategory = m_wndRibbonBar.AddCategory( TASApp.LoadLocalizedString( IDS_RBN_COMM ), IDB_STRIP_COMM_16, IDB_STRIP_COMM_32 );

	CString strTAScopeVersion = TASApp.LoadLocalizedString( IDS_UNKNOWN );
	CString strDpSVisioVersion = TASApp.LoadLocalizedString( IDS_UNKNOWN );

	// Data transfer panel.
	m_pPanelCommunicationDataTransfer = m_pCommunicationCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_C_DATATRANS ),
										m_pCommunicationCategory->GetSmallImages().ExtractIcon( eSCPanelDataTrans ) );
	{
		CMFCRibbonButton *pBScopeComm = new CMFCRibbonButton( ID_RBN_C_DT_SCOPECOMM, TASApp.LoadLocalizedString( IDS_RBN_C_DT_SCOPECOMM ), eSCScopeComm, eSCScopeComm );
		CMFCRibbonButton *pBCBIComm = new CMFCRibbonButton( ID_RBN_C_DT_CBICOMM, TASApp.LoadLocalizedString( IDS_RBN_C_DT_CBICOMM ), eSCCbiComm, eSCCbiComm );
		
		CFileStatus status;
		CFileTable *pSFT = new CFileTable();

		if( TRUE == CFile::GetStatus( CTAScopeUtil::GetTAScopeLocHFTPath(), status ) )
		{
			pSFT->ReadFile( ( wchar_t * )(LPCTSTR)CTAScopeUtil::GetTAScopeLocHFTPath() );

			if( true == pSFT->FindTargetWoPath( L"app.bin" ) )
			{
				strTAScopeVersion = pSFT->VersionDoubleToString( pSFT->GetVersion(), FILETABLE_VERSIONDIGITNBR_HH_APP );
			}

			if( true == pSFT->FindTargetWoPath( L"dps2.dfu" ) )
			{
				strDpSVisioVersion = pSFT->VersionDoubleToString( pSFT->GetVersion(), FILETABLE_VERSIONDIGITNBR_DPS_APP );
			}
		}

		if( NULL != pSFT )
		{
			delete pSFT;
		}

		CString strTAScopeTT;
		FormatString( strTAScopeTT, IDS_RBN_C_DT_SCOPECOMM_TT, strTAScopeVersion );

		SetToolTipRibbonElement( pBScopeComm, strTAScopeTT );
		SetToolTipRibbonElement( pBCBIComm, IDS_RBN_C_DT_CBICOMM_TT );
		
		m_pPanelCommunicationDataTransfer->Add( pBScopeComm );
		m_pPanelCommunicationDataTransfer->Add( pBCBIComm );

		m_pPanelCommunicationDataTransfer->SetJustifyColumns();
	}

	// DpS-Visio update.
	m_pPanelCommunicationDpSVisio = m_pCommunicationCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_C_DPSVISIO ),
		m_pCommunicationCategory->GetSmallImages().ExtractIcon( eSCPanelDpSVisio ) );
	{
		CMFCRibbonButton *pBDpSVisioComm = new CMFCRibbonButton( ID_RBN_C_DT_DPSVISIOCOMM, TASApp.LoadLocalizedString( IDS_RBN_C_DT_DPSVISIOCOMM ), eSCDpSVisioComm, eSCDpSVisioComm );
		
		CString strDpSVisioTT;
		FormatString( strDpSVisioTT, IDS_RBN_C_DT_DPSVISIOCOMM_TT, strDpSVisioVersion );

		SetToolTipRibbonElement( pBDpSVisioComm, strDpSVisioTT );

		m_pPanelCommunicationDpSVisio->Add( pBDpSVisioComm );

		m_pPanelCommunicationDpSVisio->SetJustifyColumns();
	}

	// Update panel.
	m_pPanelCommunicationUpdate = m_pCommunicationCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_C_UPDATE ),
								  m_pCommunicationCategory->GetSmallImages().ExtractIcon( eSCPanelUpdate ) );
	{
		CMFCRibbonButton *pBProdData = new CMFCRibbonButton( ID_RBN_C_U_PRODDATA, TASApp.LoadLocalizedString( IDS_RBN_C_U_PRODDATA ), eSCProdDataBase, eSCProdDataBase );
		CMFCRibbonButton *pBProp = new CMFCRibbonButton( ID_RBN_C_U_PROPERTIES, TASApp.LoadLocalizedString( IDS_RBN_C_U_PROPERTIES ), eSCProperties, eSCProperties );
		CMFCRibbonButton *pBtnMaintainTASCOPE = new CMFCRibbonCheckBox( ID_RBN_C_U_MAINTAINTASCOPE, TASApp.LoadLocalizedString( IDS_RBN_C_U_MAINTAINTASCOPE ) );

		// For debug purpose.
		CMFCRibbonButtonEx *pBtnTestHySelectUpdate = new CMFCRibbonButtonEx( ID_RBN_C_U_TESTHYSELECTUPDATE, _T("Test HySelect update" ) );
		pBtnTestHySelectUpdate->SetVisible( FALSE );
		
		SetToolTipRibbonElement( pBProdData, IDS_RBN_C_U_PRODDATA_TT );
		SetToolTipRibbonElement( pBProp, IDS_RBN_C_U_PROPERTIES_TT );
		SetToolTipRibbonElement( pBtnMaintainTASCOPE, IDS_RBN_C_U_MAINTAINTASCOPE_TT );

		m_pPanelCommunicationUpdate->Add( pBProdData );
		m_pPanelCommunicationUpdate->Add( pBProp );
		m_pPanelCommunicationUpdate->AddSeparator();
		m_pPanelCommunicationUpdate->Add( pBtnMaintainTASCOPE );
		m_pPanelCommunicationUpdate->Add( pBtnTestHySelectUpdate );

		m_pPanelCommunicationUpdate->SetJustifyColumns();
	}

	//-------------------------------------------------------------------------------------------------------------
	// 'Product selection tools' contextual category.
	//   -> 'Product' tab.
	//-------------------------------------------------------------------------------------------------------------
	m_pProdSelToolsProductContextualCategory = m_wndRibbonBar.AddContextCategory( TASApp.LoadLocalizedString( IDS_RBN_PRODUCTSEL ), TASApp.LoadLocalizedString( IDS_RBN_PRODUCTSEL_CONTEXT ),
					  ID_PRODUCTSEL_CATEGORY_BASE, AFX_CategoryColor_Orange, IDB_STRIP_PRODUCTSEL_16, IDB_STRIP_PRODUCTSEL_32 );

	// Gallery box.
	m_pPanelProductSelProductType = m_pProdSelToolsProductContextualCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_PS_PRODTYPE ),
								m_pProdSelToolsProductContextualCategory->GetSmallImages().ExtractIcon( CRCImageManager::ILSPS_PanelProdType ) );
	{
		m_pGallery = new CMFCRibbonGalleryEx( ID_RBN_PS_PT_GALLERY, TASApp.LoadLocalizedString( IDS_RBN_PS_PT_GALLERY ), 0, 0 );
		SetToolTipRibbonElement( m_pGallery, IDS_RBN_PS_PT_GALLERY_TT );
		_FillProductSubCategoryGalleryBox();
		m_pPanelProductSelProductType->Add( m_pGallery );
		m_pPanelProductSelProductType->SetJustifyColumns();
	}

	// Panel selection (Individual, Batch, Wizard & Direct).
	m_pPanelProductSelSelection = m_pProdSelToolsProductContextualCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_PS_INDBATCHWIZARDDIRECTSEL ),
							  m_pProdSelToolsProductContextualCategory->GetSmallImages().ExtractIcon( CRCImageManager::ILSPS_PanelSel ) );
	{
		CMFCRibbonButton *pRibbonIndividualSelButton = new CMFCRibbonButton( ID_RBN_PS_SELECTIONMODE_INDIVIDUAL, TASApp.LoadLocalizedString( IDS_RBN_PS_SELECTIONMODE_INDIVIDUAL ),
				CRCImageManager::ILSPS_Individual, CRCImageManager::ILSPS_Individual );
		SetToolTipRibbonElement( pRibbonIndividualSelButton, IDS_RBN_PS_SELECTIONMODE_INDIVIDUAL_TT );
		m_pPanelProductSelSelection->Add( pRibbonIndividualSelButton );

		CMFCRibbonButton *pRibbonBatchSelButton = new CMFCRibbonButton( ID_RBN_PS_SELECTIONMODE_BATCH, TASApp.LoadLocalizedString( IDS_RBN_PS_SELECTIONMODE_BATCH ),
				CRCImageManager::ILSPS_Batch, CRCImageManager::ILSPS_Batch );
		SetToolTipRibbonElement( pRibbonBatchSelButton, IDS_RBN_PS_SELECTIONMODE_BATCH_TT );
		m_pPanelProductSelSelection->Add( pRibbonBatchSelButton );

		CMFCRibbonButton *pRibbonWizardSelButton = new CMFCRibbonButton( ID_RBN_PS_SELECTIONMODE_WIZARD, TASApp.LoadLocalizedString( IDS_RBN_PS_SELECTIONMODE_WIZARD ),
				CRCImageManager::ILSPS_Wizard, CRCImageManager::ILSPS_Wizard );
		SetToolTipRibbonElement( pRibbonWizardSelButton, IDS_RBN_PS_SELECTIONMODE_WIZARD_TT );
		m_pPanelProductSelSelection->Add( pRibbonWizardSelButton );

		CMFCRibbonButton *pRibbonDirSelButton = new CMFCRibbonButton( ID_RBN_PS_SELECTIONMODE_DIRECT, TASApp.LoadLocalizedString( IDS_RBN_PS_SELECTIONMODE_DIRECT ),
				CRCImageManager::ILSPS_Direct, CRCImageManager::ILSPS_Direct );
		SetToolTipRibbonElement( pRibbonDirSelButton, IDS_RBN_PS_SELECTIONMODE_DIRECT_TT );
		m_pPanelProductSelSelection->Add( pRibbonDirSelButton );


		m_pPanelProductSelSelection->SetCenterColumnVert( TRUE );
		m_pPanelProductSelSelection->SetJustifyColumns();
	}

	// 'Application parameters' panel.
	m_pPanelApplicationParameters = m_pProdSelToolsProductContextualCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_PS_APPLIPARAM ),
									m_pProdSelToolsProductContextualCategory->GetSmallImages().ExtractIcon( CRCImageManager::ILSPS_ApplicationType ) );
	{
		m_pCBProductSelectionApplicationType = new CMFCRibbonComboBoxEx( ID_RBN_PS_APPLITYPE, 0, -1, TASApp.LoadLocalizedString( IDS_RBN_PS_APPLI ),
				CRCImageManager::ILSPS_ApplicationType );

		SetToolTipRibbonElement( m_pCBProductSelectionApplicationType, IDS_RBN_PS_APPLITYPE_TT );

		CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
		ProjectType PrjType = pTechP->GetProductSelectionApplicationType();
		m_pCBProductSelectionApplicationType->RemoveAllItems();
		int iSelectedItem = 0;

		if( Heating == ( pTechP->GetAvailablePMPanels() & Heating ) )
		{
			int iPos = m_pCBProductSelectionApplicationType->AddItem( (LPCTSTR)TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( Heating ) ), Heating );

			if( Heating == PrjType )
			{
				iSelectedItem = iPos;
			}
			m_pCBProductSelectionApplicationType->AddItemEx( (LPCTSTR)TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( Heating ) ),
				(LPCTSTR)TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( Heating ) ) );
		}

		if( Cooling == ( pTechP->GetAvailablePMPanels() & Cooling ) )
		{
			int iPos = m_pCBProductSelectionApplicationType->AddItem( (LPCTSTR)TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( Cooling ) ), Cooling );

			if( Cooling == PrjType )
			{
				iSelectedItem = iPos;
			}
			m_pCBProductSelectionApplicationType->AddItemEx( (LPCTSTR)TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( Cooling ) ),
				(LPCTSTR)TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( Cooling ) ) );
		}

		if( Solar == ( pTechP->GetAvailablePMPanels() & Solar ) )
		{
			int iPos = m_pCBProductSelectionApplicationType->AddItem( (LPCTSTR)TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( Solar ) ), Solar );

			if( Solar == PrjType )
			{
				iSelectedItem = iPos;
			}
			m_pCBProductSelectionApplicationType->AddItemEx( (LPCTSTR)TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( Solar ) ),
				(LPCTSTR)TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( Solar ) ) );
		}

		m_pCBProductSelectionApplicationType->SelectItem( iSelectedItem );

		m_pPanelApplicationParameters->Add( m_pCBProductSelectionApplicationType );

		m_pPanelApplicationParameters->SetJustifyColumns();
	}

	// 'Fluid Characteristics' panel.
	m_pPanelProductSelFluidCharacteristics = m_pProdSelToolsProductContextualCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_PS_WATERCHAR ),
										 m_pProdSelToolsProductContextualCategory->GetSmallImages().ExtractIcon( CRCImageManager::ILSPS_PanelWc ) );
	SetToolTipRibbonElement( &m_pPanelProductSelFluidCharacteristics->GetLaunchButton(), IDS_RBN_PS_WATERCHAR_TT );
	{
		// Build new water characteristic strings.
		CString str1, str2, str3;
		TASApp.GetpTADS()->GetpWCForProductSelection()->BuildWaterStringsRibbonBar( str1, str2, str3 );
		CString str = str1;

		if( false == str2.IsEmpty() )
		{
			str = str1 + _T("; ") + str2;
		}

		m_pLBWaterCharInfo1ProductSel = new CMFCRibbonLabel( str );
		m_pEBWaterCharTempProductSel = new CMFCRibbonEditEx( ID_RBN_PS_WC_TEMP, 65, TASApp.LoadLocalizedString( IDS_RBN_PS_WC_TEMP ), CRCImageManager::ILSPS_Temp );
		m_pEBWaterCharDTProductSel = new CMFCRibbonEditEx( ID_RBN_PS_WC_DT, 65, TASApp.LoadLocalizedString( IDS_RBN_PS_WC_DT ) );

		m_pEBWaterCharTempProductSel->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pEBWaterCharDTProductSel->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );

		SetToolTipRibbonElement( m_pEBWaterCharTempProductSel, IDS_RBN_PS_WC_TEMP_TT );
		SetToolTipRibbonElement( m_pEBWaterCharDTProductSel, IDS_RBN_PS_WC_DTEMP_TT );

		m_pPanelProductSelFluidCharacteristics->Add( m_pLBWaterCharInfo1ProductSel );
		m_pPanelProductSelFluidCharacteristics->Add( m_pEBWaterCharTempProductSel );
		m_pPanelProductSelFluidCharacteristics->Add( m_pEBWaterCharDTProductSel );

		m_pPanelProductSelFluidCharacteristics->SetJustifyColumns();
		m_pPanelProductSelFluidCharacteristics->EnableLaunchButton( ID_RBN_PS_WATERCHAR );
	}

	// Product selection tools Group Box.
	// Remark: It's fully intentional that we extract icon from 'm_pProjectCatergory'. This icon appears when user decrease width of the application.
	//         Here we well need the 'Tools' icon from the project category.
	m_pPanelProductSelTools = m_pProdSelToolsProductContextualCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_PS_TOOLS ),
						  m_pProjectCategory->GetSmallImages().ExtractIcon( eSPPanelProjTools ) );
	{
		CMFCRibbonButton *pBtnSearchReplace = new CMFCRibbonButton( ID_RBN_PS_T_SR, TASApp.LoadLocalizedString( IDS_RBN_PS_T_SR ), CRCImageManager::ILSPS_SearchReplace,
				CRCImageManager::ILSPS_SearchReplace );

		SetToolTipRibbonElement( pBtnSearchReplace, IDS_RBN_PS_T_SR_TT );
		m_pPanelProductSelTools->Add( pBtnSearchReplace );
		m_pPanelProductSelTools->SetJustifyColumns();
	}
	
	//-------------------------------------------------------------------------------------------------------------
	// 'Product selection tools' contextual category.
	//   -> 'Tech. parameters' tab for individual & batch selection mode except pressurisation.
	//-------------------------------------------------------------------------------------------------------------

	m_pProdSelToolsTechParamContextualCategory = m_wndRibbonBar.AddContextCategory( TASApp.LoadLocalizedString( IDS_RBN_PRODUCSEL_TECHPARAMINDSEL ),
							 TASApp.LoadLocalizedString( IDS_RBN_PRODUCTSEL_CONTEXT ), ID_PRODUCTSEL_CATEGORY_BASE, AFX_CategoryColor_Orange, IDB_STRIP_PRODUCTSEL_16, IDB_STRIP_PRODUCTSEL_32 );
	m_pProdSelToolsTechParamContextualCategory->SetData( ID_SSEL_CATEGORY_BALCTRL );
	
	// Panel Pipes.
	m_pPanelProductSelPipe = m_pProdSelToolsTechParamContextualCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_PS_PIPE ),
						 m_pProdSelToolsTechParamContextualCategory->GetSmallImages().ExtractIcon( CRCImageManager::ILSPS_PanelPipe ) );
	
	SetToolTipRibbonElement( &m_pPanelProductSelPipe->GetLaunchButton(), IDS_RBN_PS_PIPE_TT );
	{
		// Determine the width in pixel of the combo box
		int iCBWidth = 150;

		// Fill Combo Pipe Series for SSEL
		m_pRibbonComboSelPipeSeries = new CMFCRibbonComboBoxEx( ID_RBN_PS_P_PIPESERIES, 0, iCBWidth, TASApp.LoadLocalizedString( IDS_RBN_PS_P_PIPESERIES ) );
		m_pRibbonComboSelPipeSize = new CMFCRibbonComboBoxEx( ID_RBN_PS_P_PIPESIZE, 0, iCBWidth, TASApp.LoadLocalizedString( IDS_RBN_PS_P_PIPESIZE ) );

		SetToolTipRibbonElement( m_pRibbonComboSelPipeSeries, IDS_RBN_PS_P_PIPESERIES_TT );
		SetToolTipRibbonElement( m_pRibbonComboSelPipeSize, IDS_RBN_PS_P_PIPESIZE_TT );

		// Fill single selection combo with same values as default combo values.
		FillProductSelPipeSeriesCB();
		// Fill size in regards to current series in previous combo.
		FillProductSelPipeSizeCB();

		m_pPanelProductSelPipe->Add( m_pRibbonComboSelPipeSeries );
		m_pPanelProductSelPipe->Add( m_pRibbonComboSelPipeSize );

		m_pPanelProductSelPipe->SetJustifyColumns();
	}

	// Tech Params Group Box.
	// Technical parameters editors share same ID between 'Home' ribbon and 'Product selection' ribbon.
	// All value modification (input or output) in one editor is automatically send to his brother.
	m_pPanelProductSelTechnicalParameters = m_pProdSelToolsTechParamContextualCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_H_TECHPARAM ),
										m_pProdSelToolsTechParamContextualCategory->GetSmallImages().ExtractIcon( CRCImageManager::ILSPS_PanelTechParam ) );
	SetToolTipRibbonElement( &m_pPanelProductSelTechnicalParameters->GetLaunchButton(), IDS_RBN_H_TECHPARAM_TT );
	{
		CDS_TechnicalParameter *pTechP = (CDS_TechnicalParameter *)( TASApp.GetpTADB()->Get( L"PARAM_TECH" ).MP );

		// Instantiate MFC ribbon edit.
		m_pEBSizeAboveSSel = new CMFCRibbonEditEx( ID_RBN_SSEL_TP_SIZEABV, 30, TASApp.LoadLocalizedString( IDS_RBN_H_TP_SIZEABV ), CRCImageManager::ILSPS_DevSizeAb );
		m_pEBSizeBelowSSel = new CMFCRibbonEditEx( ID_RBN_SSEL_TP_SIZEBLW, 30, TASApp.LoadLocalizedString( IDS_RBN_H_TP_SIZEBLW ), CRCImageManager::ILSPS_DevSizeBl );
		m_pEBPresTargetSSel = new CMFCRibbonEditEx( ID_RBN_SSEL_TP_PD_TARGET, 95, TASApp.LoadLocalizedString( IDS_RBN_H_TP_PD_TARGET ),
				CRCImageManager::ILSPS_PdropTarg );
		m_pEBPresMaxSSel = new CMFCRibbonEditEx( ID_RBN_SSEL_TP_PD_MAX, 95, TASApp.LoadLocalizedString( IDS_RBN_H_TP_PD_MAX ), CRCImageManager::ILSPS_PdropMax );
		m_pEBVelTargetSSel = new CMFCRibbonEditEx( ID_RBN_SSEL_TP_WV_TARGET, 65, TASApp.LoadLocalizedString( IDS_RBN_H_TP_WV_TARGET ),
				CRCImageManager::ILSPS_WVelTarg );
		m_pEBVelMaxSSel = new CMFCRibbonEditEx( ID_RBN_SSEL_TP_WV_MAX, 65, TASApp.LoadLocalizedString( IDS_RBN_H_TP_WV_MAX ), CRCImageManager::ILSPS_WVelMax );

		m_pEBSizeAboveSSel->EnableSpinButtons( 0, pTechP->GetMaxSizeShift() );
		m_pEBSizeBelowSSel->EnableSpinButtons( 0, pTechP->GetMaxSizeShift() );

		// Create it.
		m_pEBSizeAboveSSel->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pEBSizeBelowSSel->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pEBPresTargetSSel->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pEBPresMaxSSel->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pEBVelTargetSSel->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pEBVelMaxSSel->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );

		SetToolTipRibbonElement( m_pEBSizeAboveSSel, IDS_RBN_H_TP_SIZEABV_TT );
		SetToolTipRibbonElement( m_pEBSizeBelowSSel, IDS_RBN_H_TP_SIZEBLW_TT );
		SetToolTipRibbonElement( m_pEBPresTargetSSel, IDS_RBN_H_TP_PD_TARGET_TT );
		SetToolTipRibbonElement( m_pEBPresMaxSSel, IDS_RBN_H_TP_PD_MAX_TT );
		SetToolTipRibbonElement( m_pEBVelTargetSSel, IDS_RBN_H_TP_WV_TARGET_TT );
		SetToolTipRibbonElement( m_pEBVelMaxSSel, IDS_RBN_H_TP_WV_MAX_TT );

		m_pPanelProductSelTechnicalParameters->Add( new CMFCRibbonLabel( TASApp.LoadLocalizedString( IDS_RBN_H_TP_DEVSIZE ) ) );
		m_pPanelProductSelTechnicalParameters->Add( m_pEBSizeAboveSSel );
		m_pPanelProductSelTechnicalParameters->Add( m_pEBSizeBelowSSel );
		m_pPanelProductSelTechnicalParameters->AddSeparator();
		m_pPanelProductSelTechnicalParameters->Add( new CMFCRibbonLabel( TASApp.LoadLocalizedString( IDS_RBN_H_TP_PIPEPRESSDROP ) ) );
		m_pPanelProductSelTechnicalParameters->Add( m_pEBPresTargetSSel );
		m_pPanelProductSelTechnicalParameters->Add( m_pEBPresMaxSSel );
		m_pPanelProductSelTechnicalParameters->AddSeparator();
		m_pPanelProductSelTechnicalParameters->Add( new CMFCRibbonLabel( TASApp.LoadLocalizedString( IDS_RBN_H_TP_PIPEWTVEL ) ) );
		m_pPanelProductSelTechnicalParameters->Add( m_pEBVelTargetSSel );
		m_pPanelProductSelTechnicalParameters->Add( m_pEBVelMaxSSel );

		m_pPanelProductSelTechnicalParameters->SetJustifyColumns();
		m_pPanelProductSelTechnicalParameters->EnableLaunchButton( ID_RBN_H_TECHPARAM );
	}

	//-------------------------------------------------------------------------------------------------------------
	// 'Product selection tools' contextual category.
	//   -> 'Pressurisation preferences' tab for individual pressurisation maintenante selection.
	//-------------------------------------------------------------------------------------------------------------

	m_pProdSelToolsPressurisationPrefContextualCategory = m_wndRibbonBar.AddContextCategory( TASApp.LoadLocalizedString( IDS_RBN_PRODUCTSEL_PRESSPREFSINDSEL ),
								 TASApp.LoadLocalizedString( IDS_RBN_PRODUCTSEL_CONTEXT ), ID_PRODUCTSEL_CATEGORY_BASE, AFX_CategoryColor_Orange, IDB_STRIP_PRODUCTSEL_16, IDB_STRIP_PRODUCTSEL_32 );
	m_pProdSelToolsPressurisationPrefContextualCategory->SetData( ID_PRODUCTSEL_CATEGORY_PRESSMAINT );
	
	// Panel Pressurisation maintenance & Water quality preferences.
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_PMWQPrefs );

	if( NULL != pclImgListButton )
	{
		m_pPanelProductSelPMWQPrefs = m_pProdSelToolsPressurisationPrefContextualCategory->AddPanel( TASApp.LoadLocalizedString( IDS_COMBOSSELPRESSUREMAINTENANCE ),
								  m_pProdSelToolsPressurisationPrefContextualCategory->GetSmallImages().ExtractIcon( CRCImageManager::ILSPS_PMWQSelectionPrefs ) );

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// General options.

		// Combined button.
		CMFCRibbonButtonsGroup *pRibbonPMWQPRefsButtonGroup = new CMFCRibbonButtonsGroup();
		CMFCRibbonButton *pRibbonPMWQPrefsButton = new CMFCRibbonButton( ID_RBH_PS_PMWQ_COMBINEDFCT, _T( "" ),
				( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_Combined ) );
		SetToolTipRibbonElement( pRibbonPMWQPrefsButton, IDS_RBN_PS_PWMQ_COMBINEDFCT_TT );
		pRibbonPMWQPRefsButtonGroup->AddButton( pRibbonPMWQPrefsButton );

		// Redundancy Pump/Compressor.
		pRibbonPMWQPrefsButton = new CMFCRibbonButton( ID_RBH_PS_PMWQ_REDPUMPCOMPR, _T( "" ),
				( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_RedundPumpCompr ) );
		SetToolTipRibbonElement( pRibbonPMWQPrefsButton, IDS_RBN_PS_PMWQ_REDPUMPCOMPR_TT );
		pRibbonPMWQPRefsButtonGroup->AddButton( pRibbonPMWQPrefsButton );

		// Redundancy TecBox.
		pRibbonPMWQPrefsButton = new CMFCRibbonButton( ID_RBH_PS_PMWQ_REDTECBOX, _T( "" ),
				( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_RedundTecBox ) );
		SetToolTipRibbonElement( pRibbonPMWQPrefsButton, IDS_RBN_PS_PMWQ_REDTECBOX_TT );
		pRibbonPMWQPRefsButtonGroup->AddButton( pRibbonPMWQPrefsButton );
		m_pPanelProductSelPMWQPrefs->Add( pRibbonPMWQPRefsButtonGroup );
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		// HYS-866: No more need of the 'Integrated buffer vessel' option.


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Compressor options.

		// Expansion vessel with internal coating.
		pRibbonPMWQPRefsButtonGroup = new CMFCRibbonButtonsGroup();
		pRibbonPMWQPrefsButton = new CMFCRibbonButton( ID_RBH_PS_PMWQ_EXPVSSLINTCOAT, _T( "" ),
				( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_InternalCoating ) );
		SetToolTipRibbonElement( pRibbonPMWQPrefsButton, IDS_RBN_PS_PMWQ_EXPVSSLINTCOAT_TT );
		pRibbonPMWQPRefsButtonGroup->AddButton( pRibbonPMWQPrefsButton );

		// With external compressed air.
		pRibbonPMWQPrefsButton = new CMFCRibbonButton( ID_RBH_PS_PMWQ_EXTCOMPRAIR, _T( "" ),
				( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_ExternalAir ) );
		SetToolTipRibbonElement( pRibbonPMWQPrefsButton, IDS_RBN_PS_PMWQ_EXTCOMPRAIR_TT );
		pRibbonPMWQPRefsButtonGroup->AddButton( pRibbonPMWQPrefsButton );
		m_pPanelProductSelPMWQPrefs->Add( pRibbonPMWQPRefsButtonGroup );
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// HYS-984: Pump & degassing options.

		// Insulated and vapor sealed for cooling.
		pRibbonPMWQPRefsButtonGroup = new CMFCRibbonButtonsGroup();
		pRibbonPMWQPrefsButton = new CMFCRibbonButton( ID_RBH_PS_PMWQ_INSVAPORSEALED, _T( "" ),
			( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_InsulatedVaporSealing ) );
		SetToolTipRibbonElement( pRibbonPMWQPrefsButton, IDS_RBN_PS_PMWQ_INSVAPORSEALED_TT );
		pRibbonPMWQPRefsButtonGroup->AddButton( pRibbonPMWQPrefsButton );

		m_pPanelProductSelPMWQPrefs->Add( pRibbonPMWQPRefsButtonGroup );
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Degassing options.
		// HYS-984: 'Isolated and vapor sealed for cooling' option is now merged with pump.

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Water make-up options.

		// Break tank.
		pRibbonPMWQPRefsButtonGroup = new CMFCRibbonButtonsGroup();
		pRibbonPMWQPrefsButton = new CMFCRibbonButton( ID_RBH_PS_PMWQ_BREAKTANK, _T( "" ),
				( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_BreakTank ) );
		SetToolTipRibbonElement( pRibbonPMWQPrefsButton, IDS_RBN_PS_PMWQ_BREAKTANK_TT );
		pRibbonPMWQPRefsButtonGroup->AddButton( pRibbonPMWQPrefsButton );

		// Duty and stand-by of make-up pumps.
		pRibbonPMWQPrefsButton = new CMFCRibbonButton( ID_RBH_PS_PMWQ_DUTYSTANDBY, _T( "" ),
				( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_DutyStandBy ) );
		SetToolTipRibbonElement( pRibbonPMWQPrefsButton, IDS_RBN_PS_PMWQ_DUTYSTANDBY_TT );
		pRibbonPMWQPRefsButtonGroup->AddButton( pRibbonPMWQPrefsButton );
		m_pPanelProductSelPMWQPrefs->Add( pRibbonPMWQPRefsButtonGroup );
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		m_pPanelProductSelPMWQPrefs->SetCenterColumnVert( TRUE );
	}

	// Panel selection (Pressurization & Water Quality selection preferences).
	m_pPanelProductSelPMWQSelectionPrefs = m_pProdSelToolsPressurisationPrefContextualCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_PS_PMWQSELECTIONPREFS ),
									   m_pProdSelToolsPressurisationPrefContextualCategory->GetSmallImages().ExtractIcon( CRCImageManager::ILSPS_PMWQSelectionPrefs ) );

	SetToolTipRibbonElement( &m_pPanelProductSelPMWQSelectionPrefs->GetLaunchButton(), IDS_RBN_PS_PMWQSELECTPREFS_TT );
	{
		CMFCRibbonCheckBox *pCBProductSelPMWQOneDevice = new CMFCRibbonCheckBox( ID_RBN_PS_PMWQ_ONEDEVICE, TASApp.LoadLocalizedString( IDS_RBN_PS_PMWQ_ONEDEVICE ) );
		m_pPanelProductSelPMWQSelectionPrefs->Add( pCBProductSelPMWQOneDevice );

		m_pPMWQSelecPrefsRedundancy = new CMFCRibbonComboBox( ID_RBN_PS_PMWQ_COMBOREDDCY, 0, -1, TASApp.LoadLocalizedString( IDS_RBN_PS_PMWQ_COMBOREDDCY ) );
		SetToolTipRibbonElement( m_pPMWQSelecPrefsRedundancy, IDS_RBN_PS_PMWQ_REDUNDANCY_TT );
		_FillRibbonPMWQComboRedundancy( m_pPMWQSelecPrefsRedundancy );
		m_pPanelProductSelPMWQSelectionPrefs->Add( m_pPMWQSelecPrefsRedundancy );

		m_pPanelProductSelPMWQSelectionPrefs->SetJustifyColumns();
		m_pPanelProductSelPMWQSelectionPrefs->EnableLaunchButton( ID_RBN_PS_PMWQSELECTPREFS );
	}

	// Panel technical parameters.
	m_pPanelProductSelPMTechnicalParameters = m_pProdSelToolsPressurisationPrefContextualCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_H_TECHPARAM ),
										  m_pProdSelToolsPressurisationPrefContextualCategory->GetSmallImages().ExtractIcon( CRCImageManager::ILSPS_PanelTechParam ) );
	SetToolTipRibbonElement( &m_pPanelProductSelPMTechnicalParameters->GetLaunchButton(), IDS_RBN_H_TECHPARAM_TT );
	{
		CDS_TechnicalParameter *pTechP = (CDS_TechnicalParameter *)( TASApp.GetpTADB()->Get( L"PARAM_TECH" ).MP );
		// 		CMFCRibbonCheckBox *pCBSplitVssl = new CMFCRibbonCheckBox( ID_RBN_PS_PMCBSPLITVSSL, TASApp.LoadLocalizedString( IDS_RIBBONTECHPARAMPM_PRESSVOLLIMIT ) );
		// 		m_pPanelProductSelPMTechnicalParameters->Add( pCBSplitVssl );
		// 		m_pPanelProductSelPMTechnicalParameters->SetJustifyColumns();

		m_pEBMaxNbrOfAdditionalVesselsInParallelSSel = new CMFCRibbonEditEx( ID_RBN_PS_PMWQMAXNBROFADDITIONALVESSELSINPARALLEL, 30, TASApp.LoadLocalizedString( IDS_RIBBONTECHPARAMPM_MAXADDITIONALVESSELSINPARALLEL ) );
		m_pEBMaxNbrOfAdditionalVesselsInParallelSSel->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pPanelProductSelPMTechnicalParameters->Add( m_pEBMaxNbrOfAdditionalVesselsInParallelSSel );
		m_pPanelProductSelPMTechnicalParameters->EnableLaunchButton( ID_RBN_H_TECHPARAM );

		m_pEBMaxNbrOfVentoSSel = new CMFCRibbonEditEx( ID_RBN_PS_PMWQMAXNBROFVENTOINPARALLEL, 30, TASApp.LoadLocalizedString( IDS_RIBBONTECHPARAMPM_MAXVENTO ) );
		m_pEBMaxNbrOfVentoSSel->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pPanelProductSelPMTechnicalParameters->Add( m_pEBMaxNbrOfVentoSSel );
		m_pPanelProductSelPMTechnicalParameters->EnableLaunchButton( ID_RBN_H_TECHPARAM );

		m_pPanelProductSelPMTechnicalParameters->SetJustifyColumns();
	}

	//-------------------------------------------------------------------------------------------------------------
	// 'Product selection tools' contextual category.
	//   -> 'Tech. parameters' tab for pressurisation wizard selection mode.
	//-------------------------------------------------------------------------------------------------------------

	m_pProdSelToolsPMWizardTechParamContextualCategory = m_wndRibbonBar.AddContextCategory( TASApp.LoadLocalizedString( IDS_RBN_PRODUCTSEL_PRESSPREFSWIZSEL ),
								 TASApp.LoadLocalizedString( IDS_RBN_PRODUCTSEL_CONTEXT ), ID_PRODUCTSEL_CATEGORY_BASE, AFX_CategoryColor_Orange, IDB_STRIP_PRODUCTSEL_16, IDB_STRIP_PRODUCTSEL_32 );
	m_pProdSelToolsPMWizardTechParamContextualCategory->SetData( ID_PRODUCTSEL_CATEGORY_PRESSMAINTWIZARD );

	// Panel technical parameters.
	m_pPanelProductSelPMWizardTechnicalParameters = m_pProdSelToolsPMWizardTechParamContextualCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_H_TECHPARAM ),
										  m_pProdSelToolsPMWizardTechParamContextualCategory->GetSmallImages().ExtractIcon( CRCImageManager::ILSPS_PanelTechParam ) );
	SetToolTipRibbonElement( &m_pPanelProductSelPMWizardTechnicalParameters->GetLaunchButton(), IDS_RBN_H_TECHPARAM_TT );
	{
		m_pEBMaxNbrOfVsslSSelPMWizard = new CMFCRibbonEditEx( ID_RBN_PS_PMWQMAXNBROFADDITIONALVESSELSINPARALLEL, 30, TASApp.LoadLocalizedString( IDS_RIBBONTECHPARAMPM_MAXADDITIONALVESSELSINPARALLEL ) );
		m_pEBMaxNbrOfVsslSSelPMWizard->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pPanelProductSelPMWizardTechnicalParameters->Add( m_pEBMaxNbrOfVsslSSelPMWizard );
		m_pPanelProductSelPMWizardTechnicalParameters->EnableLaunchButton( ID_RBN_H_TECHPARAM );

		m_pEBMaxNbrOfVentoSSelPMWizard = new CMFCRibbonEditEx( ID_RBN_PS_PMWQMAXNBROFVENTOINPARALLEL, 30, TASApp.LoadLocalizedString( IDS_RIBBONTECHPARAMPM_MAXVENTO ) );
		m_pEBMaxNbrOfVentoSSelPMWizard->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
		m_pPanelProductSelPMWizardTechnicalParameters->Add( m_pEBMaxNbrOfVentoSSelPMWizard );
		m_pPanelProductSelPMWizardTechnicalParameters->EnableLaunchButton( ID_RBN_H_TECHPARAM );

		m_pPanelProductSelPMWizardTechnicalParameters->SetJustifyColumns();
	}

	//-------------------------------------------------
	// LOGGED DATA SELECTION GROUP.
	m_pLogDataCategory = m_wndRibbonBar.AddContextCategory( TASApp.LoadLocalizedString( IDS_RBN_LOGDATA ), TASApp.LoadLocalizedString( IDS_RBN_LOGDATA_CONTEXT ),
						 ID_LOGDATA_CATEGORY, AFX_CategoryColor_Orange, IDB_STRIP_LOGGEDDATA_16, IDB_STRIP_LOGGEDDATA_32 );

	// Panel Curves.
	m_pPanelLoggedDataChartOptions = m_pLogDataCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_LD_CURVES ),
									 m_pLogDataCategory->GetSmallImages().ExtractIcon( eSLDPanelCurves ) );
	{
		CMFCRibbonCheckBox *pCbLDFlow = new CMFCRibbonCheckBox( ID_RBN_LD_C_FLOW, TASApp.LoadLocalizedString( IDS_RBN_LD_C_FLOW ) );
		CMFCRibbonCheckBox *pCbLDDp = new CMFCRibbonCheckBox( ID_RBN_LD_C_DP, TASApp.LoadLocalizedString( IDS_RBN_LD_C_DP ) );
		m_pCBLDT1 = new CMFCRibbonCheckBox( ID_RBN_LD_C_T1, TASApp.LoadLocalizedString( IDS_RBN_LD_C_T1 ) );
		m_pCBLDT2 = new CMFCRibbonCheckBox( ID_RBN_LD_C_T2, TASApp.LoadLocalizedString( IDS_RBN_LD_C_T2 ) );
		CMFCRibbonCheckBox *pCbLDDT = new CMFCRibbonCheckBox( ID_RBN_LD_C_DT, TASApp.LoadLocalizedString( IDS_RBN_LD_C_DT ) );
		CMFCRibbonCheckBox *pCbLDP = new CMFCRibbonCheckBox( ID_RBN_LD_C_POWER, TASApp.LoadLocalizedString( IDS_RBN_LD_C_POWER ) );

		SetToolTipRibbonElement( pCbLDFlow, IDS_RBN_LD_C_FLOW_TT );
		SetToolTipRibbonElement( pCbLDDp, IDS_RBN_LD_C_DP_TT );
		SetToolTipRibbonElement( m_pCBLDT1, IDS_RBN_LD_C_T1_TT );
		SetToolTipRibbonElement( m_pCBLDT2, IDS_RBN_LD_C_T2_TT );
		SetToolTipRibbonElement( pCbLDDT, IDS_RBN_LD_C_DT_TT );
		SetToolTipRibbonElement( pCbLDP, IDS_RBN_LD_C_POWER_TT );

		m_pPanelLoggedDataChartOptions->Add( pCbLDFlow );
		m_pPanelLoggedDataChartOptions->Add( m_pCBLDT1 );
		m_pPanelLoggedDataChartOptions->Add( pCbLDDT );
		m_pPanelLoggedDataChartOptions->Add( pCbLDDp );
		m_pPanelLoggedDataChartOptions->Add( m_pCBLDT2 );
		m_pPanelLoggedDataChartOptions->Add( pCbLDP );

		m_pPanelLoggedDataChartOptions->SetJustifyColumns();
	}

	// Panel Zoom.
	m_pPanelLoggedDataZoom = m_pLogDataCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_LD_ZOOM ),
							 m_pLogDataCategory->GetSmallImages().ExtractIcon( eSLDPanelZoom ) );
	{
		CMFCRibbonButton *pRibbonZoomInButton = new CMFCRibbonButton( ID_RBN_LD_Z_IN, TASApp.LoadLocalizedString( IDS_RBN_LD_Z_IN ), eSLDZoomIn, eSLDZoomIn );
		CMFCRibbonButton *pRibbonZoomOutButton = new CMFCRibbonButton( ID_RBN_LD_Z_OUT, TASApp.LoadLocalizedString( IDS_RBN_LD_Z_OUT ), eSLDZoomOut, eSLDZoomOut );
		CMFCRibbonButton *pRibbonZoomUndoButton = new CMFCRibbonButton( ID_RBN_LD_Z_UNDO, TASApp.LoadLocalizedString( IDS_RBN_LD_Z_UNDO ), eSLDUndoZoom, eSLDUndoZoom );
		CMFCRibbonButton *pRibbon2DZoomgButton = new CMFCRibbonButton( ID_RBN_LD_Z_2D, TASApp.LoadLocalizedString( IDS_RBN_LD_Z_2D ), eSLD2DZoom, eSLD2DZoom );

		SetToolTipRibbonElement( pRibbonZoomInButton, IDS_RBN_LD_Z_IN_TT );
		SetToolTipRibbonElement( pRibbonZoomOutButton, IDS_RBN_LD_Z_OUT_TT );
		SetToolTipRibbonElement( pRibbonZoomUndoButton, IDS_RBN_LD_Z_UNDO_TT );
		SetToolTipRibbonElement( pRibbon2DZoomgButton, IDS_RBN_LD_Z_2D_TT );

		m_pPanelLoggedDataZoom->Add( pRibbonZoomInButton );
		m_pPanelLoggedDataZoom->Add( pRibbonZoomOutButton );
		m_pPanelLoggedDataZoom->Add( pRibbonZoomUndoButton );
		m_pPanelLoggedDataZoom->Add( pRibbon2DZoomgButton );

		m_pPanelLoggedDataZoom->SetJustifyColumns();
	}

	// Panel Chart options.
	m_pPanelLoggedDataDisplayHideCurves = m_pLogDataCategory->AddPanel( TASApp.LoadLocalizedString( IDS_RBN_LD_CHARTOPTIONS ),
										  m_pLogDataCategory->GetSmallImages().ExtractIcon( eSLDPanelChartOptions ) );
	{
		CMFCRibbonButton *pRibbonTrackingButton = new CMFCRibbonButton( ID_RBN_LD_CO_TRACKING, TASApp.LoadLocalizedString( IDS_RBN_LD_CO_TRACKING ), eSLDTracking,
				eSLDTracking );
		CMFCRibbonButton *pRibbonDisplaySymbols = new CMFCRibbonButton( ID_RBN_LD_CO_MARKERS, TASApp.LoadLocalizedString( IDS_RBN_LD_CO_MARKERS ), eSLDMarkers,
				eSLDMarkers );
		CMFCRibbonButton *pRibbonLegendButton = new CMFCRibbonButton( ID_RBN_LD_CO_LEGEND, TASApp.LoadLocalizedString( IDS_RBN_LD_CO_LEGEND ), eSLDLegend, eSLDLegend );
		CMFCRibbonButton *pRibbonHAxisButton = new CMFCRibbonButton( ID_RBN_LD_CO_DATEAXIS, TASApp.LoadLocalizedString( IDS_RBN_LD_CO_DATEAXIS ), eSLDHorizAxis,
				eSLDHorizAxis );
		CMFCRibbonButton *pRibbonExpChartButton = new CMFCRibbonButton( ID_RBN_LD_CO_EXPORT, TASApp.LoadLocalizedString( IDS_RBN_LD_CO_EXPORT ), eSLDExportChart,
				eSLDExportChart );

		SetToolTipRibbonElement( pRibbonTrackingButton, IDS_RBN_LD_CO_TRACKING_TT );
		SetToolTipRibbonElement( pRibbonDisplaySymbols, IDS_RBN_LD_CO_MARKERS_TT );
		SetToolTipRibbonElement( pRibbonLegendButton, IDS_RBN_LD_CO_LEGEND_TT );
		SetToolTipRibbonElement( pRibbonHAxisButton, IDS_RBN_LD_CO_DATEAXIS_TT );
		SetToolTipRibbonElement( pRibbonExpChartButton, IDS_RBN_LD_CO_EXPORT_TT );

		m_pPanelLoggedDataDisplayHideCurves->Add( pRibbonTrackingButton );
		m_pPanelLoggedDataDisplayHideCurves->AddSeparator();
		m_pPanelLoggedDataDisplayHideCurves->Add( pRibbonDisplaySymbols );
		m_pPanelLoggedDataDisplayHideCurves->Add( pRibbonLegendButton );
		m_pPanelLoggedDataDisplayHideCurves->Add( pRibbonHAxisButton );
		m_pPanelLoggedDataDisplayHideCurves->AddSeparator();
		m_pPanelLoggedDataDisplayHideCurves->Add( pRibbonExpChartButton );

		m_pPanelLoggedDataDisplayHideCurves->SetJustifyColumns();
	}

	//-------------------------------------------------
	// RIGHT VIEW Buttons.
	CMFCToolBarImages Image16;
	Image16.Load( IDB_STRIP_HOME_16 );

	CMFCRibbonButton *pVisualStyleButton = new CMFCRibbonButton( -1, TASApp.LoadLocalizedString( IDS_RBN_STYLE ) );

	pVisualStyleButton->SetMenu( IDR_THEME_MENU, FALSE, TRUE );
	SetToolTipRibbonElement( pVisualStyleButton, IDS_RBN_STYLE_TT );

	m_wndRibbonBar.AddToTabs( pVisualStyleButton );

	//-------------------------------------------------
	// QUICK ACCESS TOOLBAR.
	CList<UINT, UINT> lstQATCmds;
	{
		lstQATCmds.AddTail( ID_APPMENU_NEW );
		lstQATCmds.AddTail( ID_APPMENU_OPEN );
		lstQATCmds.AddTail( ID_APPMENU_SAVE );
		lstQATCmds.AddTail( ID_APPMENU_PRINT );
	}

	m_wndRibbonBar.SetQuickAccessCommands( lstQATCmds );

	CMFCRibbonQuickAccessToolBarDefaultState *qaToolBarState = new CMFCRibbonQuickAccessToolBarDefaultState();
	qaToolBarState->AddCommand( ID_APPMENU_NEW, TRUE );
	qaToolBarState->AddCommand( ID_APPMENU_OPEN, TRUE );
	qaToolBarState->AddCommand( ID_APPMENU_SAVE, TRUE );
	qaToolBarState->AddCommand( ID_APPMENU_PRINT, TRUE );
	m_wndRibbonBar.SetQuickAccessDefaultState( *qaToolBarState );
	delete qaToolBarState;

	m_wndRibbonBar.EnableKeyTips( FALSE );
}

void CMainFrame::InitializeRibbonRichEditBoxes()
{
	InitializeRibbonRichEditBoxesHome();
	InitializeRibbonRichEditBoxesProject();
	InitializeRibbonRichEditBoxesSSel();
}

void CMainFrame::InitializeRibbonRichEditBoxesHome()
{
	// Init variables.
	CString str;

	// Create the Technical parameter pointer.
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();

	// Define 'm_pEBSizeAboveHome'.
	m_pEBSizeAboveHome->SetPhysicalTypeEdit( _U_NODIM );
	m_pEBSizeAboveHome->SetUnitsUsed( false );
	m_pEBSizeAboveHome->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBSizeAboveHome->SetEditType( CNumString::eEditType::eINT );
	m_pEBSizeAboveHome->SetMaxIntValue( pTechP->GetMaxSizeShift() );
	m_pEBSizeAboveHome->SetCurrentValSI( pTechP->GetSizeShiftAbove() );

	// Define 'm_pEBSizeBelowHome'.
	m_pEBSizeBelowHome->SetPhysicalTypeEdit( _U_NODIM );
	m_pEBSizeBelowHome->SetUnitsUsed( false );
	m_pEBSizeBelowHome->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBSizeBelowHome->SetEditType( CNumString::eEditType::eINT );
	m_pEBSizeBelowHome->SetMaxIntValue( pTechP->GetMaxSizeShift() );
	m_pEBSizeBelowHome->SetCurrentValSI( abs( pTechP->GetSizeShiftBelow() ) );

	// Define 'm_pEBPresTargetHome'.
	m_pEBPresTargetHome->SetPhysicalTypeEdit( _U_LINPRESSDROP );
	m_pEBPresTargetHome->SetUnitsUsed( TRUE );
	m_pEBPresTargetHome->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBPresTargetHome->SetCurrentValSI( pTechP->GetPipeTargDp() );

	// Define 'm_pEBPresMaxHome'.
	m_pEBPresMaxHome->SetPhysicalTypeEdit( _U_LINPRESSDROP );
	m_pEBPresMaxHome->SetUnitsUsed( TRUE );
	m_pEBPresMaxHome->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBPresMaxHome->SetCurrentValSI( pTechP->GetPipeMaxDp() );
	
	// Define 'm_pEBVelTargetHome'.
	m_pEBVelTargetHome->SetPhysicalTypeEdit( _U_VELOCITY );
	m_pEBVelTargetHome->SetUnitsUsed( TRUE );
	m_pEBVelTargetHome->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBVelTargetHome->SetCurrentValSI( pTechP->GetPipeTargVel() );

	// Define 'm_pEBVelMaxHome'.
	m_pEBVelMaxHome->SetPhysicalTypeEdit( _U_VELOCITY );
	m_pEBVelMaxHome->SetUnitsUsed( TRUE );
	m_pEBVelMaxHome->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBVelMaxHome->SetCurrentValSI( pTechP->GetPipeMaxVel() );

	m_wndRibbonBar.ForceRecalcLayout();
}

void CMainFrame::InitializeRibbonRichEditBoxesProject()
{
	// Build new water characteristic strings.
	CDS_WaterCharacteristic *pWC = TASApp.GetpTADS()->GetpWCForProject();

	if( NULL == pWC )
	{
		return;
	}

	// Init variables.
	CString str1, str2, str3;
	pWC->BuildWaterStringsRibbonBar( str1, str2, str3 );

	CString str = str1;

	if( false == str2.IsEmpty() )
	{
		str = str1 + _T("; ") + str2;
	}

	m_pLBWaterCharInfo1Proj->SetText( str );

	// Define 'm_pEBWaterCharTempProj'.
	m_pEBWaterCharTempProj->SetPhysicalTypeEdit( _U_TEMPERATURE );
	m_pEBWaterCharTempProj->SetEditForTemperature();
	m_pEBWaterCharTempProj->SetUnitsUsed( TRUE );
	m_pEBWaterCharTempProj->SetEditSign( CNumString::eEditSign::eBoth );
	m_pEBWaterCharTempProj->SetMinDblValue( -273.15 );
	m_pEBWaterCharTempProj->SetEditTextEx( str3 );
	m_pEBWaterCharTempProj->SetCurrentValSI( pWC->GetTemp() );

	// Define 'm_pEBWaterCharDTProj'.
	m_pEBWaterCharDTProj->SetPhysicalTypeEdit( _U_DIFFTEMP );
	m_pEBWaterCharDTProj->SetUnitsUsed( TRUE );
	m_pEBWaterCharDTProj->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBWaterCharDTProj->SetMinDblValue( 0 );
	m_pEBWaterCharDTProj->SetCurrentValSI( TASApp.GetpTADS()->GetpTechParams()->GetDefaultPrjDT() );

	m_wndRibbonBar.ForceRecalcLayout();
}

void CMainFrame::InitializeRibbonRichEditBoxesSSel()
{
	// Init variables.
	CString str1, str2, str3;

	// Build new water characteristic strings.
	CDS_WaterCharacteristic *pWC = TASApp.GetpTADS()->GetpWCForProductSelection();

	if( NULL == pWC )
	{
		return;
	}

	pWC->BuildWaterStringsRibbonBar( str1, str2, str3 );

	CString str = str1;

	if( false == str2.IsEmpty() )
	{
		str = str1 + _T("; ") + str2;
	}

	m_pLBWaterCharInfo1ProductSel->SetText( str );
	str1.Empty();
	str2.Empty();
	pWC->BuildTempChangeOverStringsRibbonBar( str1, str2 );
	
	// Define temperature field for product selection.
	if( NULL != m_pEBWaterCharTempProductSel )
	{
		m_pEBWaterCharTempProductSel->SetPhysicalTypeEdit( _U_TEMPERATURE );
		m_pEBWaterCharTempProductSel->SetEditForTemperature();
		m_pEBWaterCharTempProductSel->SetUnitsUsed( TRUE );
		m_pEBWaterCharTempProductSel->SetEditSign( CNumString::eEditSign::eBoth );
		m_pEBWaterCharTempProductSel->SetMinDblValue( -273.15 );
		m_pEBWaterCharTempProductSel->SetCurrentValSI( pWC->GetTemp() );
	}
	else
	{
		m_pEBWaterCharChangeOverTempProductSel->SetEditText( str1 );
	}

	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();

	// Define 'm_pEBSizeAboveSSel'.
	m_pEBSizeAboveSSel->SetPhysicalTypeEdit( _U_NODIM );
	m_pEBSizeAboveSSel->SetUnitsUsed( false );
	m_pEBSizeAboveSSel->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBSizeAboveSSel->SetEditType( CNumString::eEditType::eINT );
	m_pEBSizeAboveSSel->SetMaxIntValue( pTechP->GetMaxSizeShift() );
	m_pEBSizeAboveSSel->SetCurrentValSI( pTechP->GetSizeShiftAbove() );

	// Define 'm_pEBSizeBelowSSel'.
	m_pEBSizeBelowSSel->SetPhysicalTypeEdit( _U_NODIM );
	m_pEBSizeBelowSSel->SetUnitsUsed( false );
	m_pEBSizeBelowSSel->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBSizeBelowSSel->SetEditType( CNumString::eEditType::eINT );
	m_pEBSizeBelowSSel->SetMaxIntValue( pTechP->GetMaxSizeShift() );
	m_pEBSizeBelowSSel->SetCurrentValSI( abs( pTechP->GetSizeShiftBelow() ) );

	// Define 'm_pEBPresTargetSSel'.
	m_pEBPresTargetSSel->SetPhysicalTypeEdit( _U_LINPRESSDROP );
	m_pEBPresTargetSSel->SetUnitsUsed( TRUE );
	m_pEBPresTargetSSel->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBPresTargetSSel->SetCurrentValSI( pTechP->GetPipeTargDp() );

	// Define 'm_pEBPresMaxSSel'.
	m_pEBPresMaxSSel->SetPhysicalTypeEdit( _U_LINPRESSDROP );
	m_pEBPresMaxSSel->SetUnitsUsed( TRUE );
	m_pEBPresMaxSSel->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBPresMaxSSel->SetCurrentValSI( pTechP->GetPipeMaxDp() );

	// Define 'm_pEBVelTargetSSel'.
	m_pEBVelTargetSSel->SetPhysicalTypeEdit( _U_VELOCITY );
	m_pEBVelTargetSSel->SetUnitsUsed( TRUE );
	m_pEBVelTargetSSel->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBVelTargetSSel->SetCurrentValSI( pTechP->GetPipeTargVel() );

	// Define 'm_pEBVelMaxSSel'.
	m_pEBVelMaxSSel->SetPhysicalTypeEdit( _U_VELOCITY );
	m_pEBVelMaxSSel->SetUnitsUsed( TRUE );
	m_pEBVelMaxSSel->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBVelMaxSSel->SetCurrentValSI( pTechP->GetPipeMaxVel() );

	// Define 'm_pEBWaterCharDTSSel'.
	if( NULL != m_pEBWaterCharDTProductSel )
	{
		m_pEBWaterCharDTProductSel->SetPhysicalTypeEdit( _U_DIFFTEMP );
		m_pEBWaterCharDTProductSel->SetUnitsUsed( TRUE );
		m_pEBWaterCharDTProductSel->SetEditSign( CNumString::eEditSign::ePositive );
		m_pEBWaterCharDTProductSel->SetMinDblValue( 0 );
		m_pEBWaterCharDTProductSel->SetCurrentValSI( TASApp.GetpTADS()->GetpTechParams()->GetDefaultISDT() );
	}
	else
	{
		m_pEBWaterCharChangeOverDTProductSel->SetEditText( str2 );
	}

	m_pEBMaxNbrOfAdditionalVesselsInParallelSSel->SetPhysicalTypeEdit( _U_NODIM );
	m_pEBMaxNbrOfAdditionalVesselsInParallelSSel->SetUnitsUsed( false );
	m_pEBMaxNbrOfAdditionalVesselsInParallelSSel->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBMaxNbrOfAdditionalVesselsInParallelSSel->SetEditType( CNumString::eEditType::eINT );
	m_pEBMaxNbrOfAdditionalVesselsInParallelSSel->SetMinIntValue( 0 );
	m_pEBMaxNbrOfAdditionalVesselsInParallelSSel->SetMaxIntValue( 10 );
	m_pEBMaxNbrOfAdditionalVesselsInParallelSSel->SetCurrentValSI( TASApp.GetpTADS()->GetpTechParams()->GetMaxNumberOfAdditionalVesselsInParallel() );

	m_pEBMaxNbrOfVentoSSel->SetPhysicalTypeEdit( _U_NODIM );
	m_pEBMaxNbrOfVentoSSel->SetUnitsUsed( false );
	m_pEBMaxNbrOfVentoSSel->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBMaxNbrOfVentoSSel->SetEditType( CNumString::eEditType::eINT );
	m_pEBMaxNbrOfVentoSSel->SetMinIntValue( 1 );
	m_pEBMaxNbrOfVentoSSel->SetMaxIntValue( 10 );
	m_pEBMaxNbrOfVentoSSel->SetCurrentValSI( TASApp.GetpTADS()->GetpTechParams()->GetMaxNumberOfVentoInParallel() );

	m_pEBMaxNbrOfVsslSSelPMWizard->SetPhysicalTypeEdit( _U_NODIM );
	m_pEBMaxNbrOfVsslSSelPMWizard->SetUnitsUsed( false );
	m_pEBMaxNbrOfVsslSSelPMWizard->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBMaxNbrOfVsslSSelPMWizard->SetEditType( CNumString::eEditType::eINT );
	m_pEBMaxNbrOfVsslSSelPMWizard->SetMinIntValue( 0 );
	m_pEBMaxNbrOfVsslSSelPMWizard->SetMaxIntValue( 10 );
	m_pEBMaxNbrOfVsslSSelPMWizard->SetCurrentValSI( TASApp.GetpTADS()->GetpTechParams()->GetMaxNumberOfAdditionalVesselsInParallel() );

	m_pEBMaxNbrOfVentoSSelPMWizard->SetPhysicalTypeEdit( _U_NODIM );
	m_pEBMaxNbrOfVentoSSelPMWizard->SetUnitsUsed( false );
	m_pEBMaxNbrOfVentoSSelPMWizard->SetEditSign( CNumString::eEditSign::ePositive );
	m_pEBMaxNbrOfVentoSSelPMWizard->SetEditType( CNumString::eEditType::eINT );
	m_pEBMaxNbrOfVentoSSelPMWizard->SetMinIntValue( 1 );
	m_pEBMaxNbrOfVentoSSelPMWizard->SetMaxIntValue( 10 );
	m_pEBMaxNbrOfVentoSSelPMWizard->SetCurrentValSI( TASApp.GetpTADS()->GetpTechParams()->GetMaxNumberOfVentoInParallel() );

	m_wndRibbonBar.ForceRecalcLayout();
}

void CMainFrame::UpdateMenuToolTip( CCmdUI *pCmdUI, CString strToolTip )
{
	if( NULL == pCmdUI )
	{
		return;
	}

	// Variables.
	CPoint ptCursor( 0, 0 );
	UINT uiResID = NULL;

	// Get the owner Popup Menu Bar.
	CMFCPopupMenuBar *pPopupMenuBar = dynamic_cast<CMFCPopupMenuBar *>( pCmdUI->m_pOther );

	if( NULL == pPopupMenuBar )
	{
		return;
	}

	// Get the current sibling uiResID.
	::GetCursorPos( &ptCursor );
	pPopupMenuBar->ScreenToClient( &ptCursor );
	int nIndex = pPopupMenuBar->HitTest( ptCursor );

	if( -1 != nIndex )
	{
		uiResID = pPopupMenuBar->GetItemID( nIndex );
	}

	// Verify the Button equal the current CCmd ID.
	if( pCmdUI->m_nID == uiResID )
	{
		// Set the Localized text to display into the status bar.
		if( true == strToolTip.IsEmpty() )
		{
			pMainFrame->GetStatusBar()->SetPaneText( NULL, GetSplitStringFromResID( pCmdUI->m_nID, TRUE ) );
		}
		else
		{
			pMainFrame->GetStatusBar()->SetPaneText( NULL, strToolTip );
		}
	}

	// Verify the Current ResID must be grayed.
	bool fGrayed = false;

	for( int i = 0; i <= m_ArResIDMenuToGray.GetUpperBound(); i++ )
	{
		UINT uiCurResID = m_ArResIDMenuToGray.GetAt( i );

		if( uiCurResID == pCmdUI->m_nID )
		{
			fGrayed = true;
			break;
		}
	}

	pCmdUI->Enable( !fGrayed );
}

void CMainFrame::EnableMenuResID( UINT uiResID )
{
	// If no 'uiResID' was defined, it removes all item in the CArray.
	if( 0 == uiResID )
	{
		m_ArResIDMenuToGray.RemoveAll();
		return;
	}

	// Variable.
	int iPos = -1;

	// Loop on the CArray and try to find the current 'uiResID'.
	for( int i = 0; i <= m_ArResIDMenuToGray.GetUpperBound(); i++ )
	{
		UINT uiCurResID = m_ArResIDMenuToGray.GetAt( i );

		if( uiCurResID == uiResID )
		{
			iPos = i;
			break;
		}
	}

	// Remove the ResID.
	if( iPos != -1 )
	{
		m_ArResIDMenuToGray.RemoveAt( iPos );
	}
}

void CMainFrame::DisableMenuResID( UINT uiResID )
{
	// Variable.
	bool fResIDExist = false;

	// Loop on the CArray and try to find the current 'uiResID'.
	for( int i = 0; i <= m_ArResIDMenuToGray.GetUpperBound(); i++ )
	{
		UINT uiCurResID = m_ArResIDMenuToGray.GetAt( i );

		if( uiCurResID == uiResID )
		{
			fResIDExist = true;
			break;
		}
	}

	// Add the 'uiResID'.
	if( false == fResIDExist )
	{
		m_ArResIDMenuToGray.Add( uiResID );
	}
}

CString CMainFrame::GetSplitStringFromResID( UINT uiResID, bool fToolTip )
{
	if( 0 == uiResID )
	{
		return L"";
	}

	CString str1, str2;
	int pos = -1;

	// Load the current string.
	str1 = TASApp.LoadLocalizedString( uiResID );
	str2 = str1;

	// Find the position of the last "\n".
	pos = str1.ReverseFind( '\n' );

	// Split the string in two.
	if( pos != -1 )
	{
		// Remark: 'pos + 1' to eliminate the '\n'
		str2 = str1.Right( str1.GetLength() - ( pos + 1 ) );
		str1 = str1.Left( pos );
	}

	// Return the wanted part of string.
	if( true == fToolTip )
	{
		return str1;
	}

	return str2;
}

void CMainFrame::SetToolTipRibbonElement( CMFCRibbonBaseElement *pRbnBaseElement, UINT uiID )
{
	if( NULL == pRbnBaseElement || 0 == uiID )
	{
		return;
	}

	// Load the current string.
	SetToolTipRibbonElement( pRbnBaseElement, TASApp.LoadLocalizedString( uiID ) );
}

void CMainFrame::SetToolTipRibbonElement( CMFCRibbonBaseElement *pRbnBaseElement, CString str )
{
	int pos = -1;
	CString str2 = str;

	// Find the position of the last "\n".
	for( int i = str.Find( L"\n" ); i != -1; i = str.Find( L"\n", pos + 2 ) )
	{
		pos = i;
	}

	// Split the string in two.
	// One part for the header of the tooltip and one part for the description.
	if( pos != -1 )
	{
		str2 = str.Right( str.GetLength() - pos );
		str = str.Left( pos );
	}

	// Fill the tooltip with the header.
	pRbnBaseElement->SetToolTipText( (LPCTSTR)str2 );

	// Fill the tooltip with the description.
	if( pos != -1 )
	{
		pRbnBaseElement->SetDescription( (LPCTSTR)str );
	}
}

bool CMainFrame::IsFreezeChecked( void )
{
	if( NULL == m_pChBFreezeHMCalc )
	{
		return false;
	}

	return ( TRUE == m_pChBFreezeHMCalc->IsChecked() ) ? true : false;
}

void CMainFrame::Freeze( bool fFreeze )
{
	if( false == IsHMCalcChecked() )
	{
		return;
	}

	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( fFreeze == pPrjParam->IsFreezed() )
	{
		return;
	}

	OnRbnPPPFreeze();
}

void CMainFrame::ShowDlgDiversityFactor( void )
{
	OnRbnPTDF();
}

void CMainFrame::ChangeRbnProjectApplicationType( ProjectType eProjectApplicationType )
{
	if( NULL == m_pCBProjectApplicationType )
	{
		return;
	}

	for( int i = 0; i < m_pCBProjectApplicationType->GetCount(); ++i )
	{
		if( (int)m_pCBProjectApplicationType->GetItemData( i ) == (int)eProjectApplicationType )
		{
			m_pCBProjectApplicationType->SelectItem( i );
			break;
		}
	}

	_OnRbnProjectApplicationTypeChanged();
}

void CMainFrame::ClickRibbonProductSelGallery( ProductSubCategory eProductSubCategory )
{
	// Select the right item in the gallery box.
	int iIndex = -1; 

	for( int i = CRCImageManager::ImgListStripSelCategory::ILSSC_First; i < CRCImageManager::ImgListStripSelCategory::ILSSC_Last && -1 == iIndex; i++ )
	{
		if( eProductSubCategory == ( ProductSubCategory )pMainFrame->GetRibbonSSelGallery()->GetLparamFromImageIndex( i ) )
		{
			iIndex = i;
		}
	}

	if( -1 != iIndex )
	{
		GetRibbonSSelGallery()->SelectItem( iIndex );
		OnRbnPSPTGallery();
	}
}

void CMainFrame::ClickRibbonProductSelectionMode( ProductSelectionMode eProductSelectionMode )
{
	switch( eProductSelectionMode )
	{
		case ProductSelectionMode::ProductSelectionMode_Individual:
			OnRbnPSSelectionModeIndividual();
			break;

		case ProductSelectionMode::ProductSelectionMode_Batch:
			OnRbnPSSelectionModeBatch();
			break;

		case ProductSelectionMode::ProductSelectionMode_Wizard:
			OnRbnPSSelectionModeWizard();
			break;
	}
}

void CMainFrame::ChangeRbnProductSelectionApplicationType( ProjectType eProductSelectionApplicationType )
{
	if( NULL == m_pCBProductSelectionApplicationType )
	{
		return;
	}

	for( int i = 0; i < m_pCBProductSelectionApplicationType->GetCount(); ++i )
	{
		if( (int)m_pCBProductSelectionApplicationType->GetItemData( i ) == (int)eProductSelectionApplicationType )
		{
			m_pCBProductSelectionApplicationType->SelectItem( i );
			break;
		}
	}
	_OnRbnProductSelectionApplicationTypeChanged();
}

void CMainFrame::ChangeProductSelectionMode( ProductSelectionMode eProductSelectionMode )
{
	m_eCurrentProductSelectionMode = eProductSelectionMode;

	if( NULL != pDlgLeftTabSelManager )
	{
		pDlgLeftTabSelManager->UpdateLeftTabDialog( m_eCurrentProductSubCategory, m_eCurrentProductSelectionMode );
	}

	_UpdateProductSubCategoryGalleryBox();
}

void CMainFrame::UpdatePMWQSelectPrefsRibbon()
{
	m_wndRibbonBar.ForceRecalcLayout();
}

#ifdef _DEBUG
bool CMainFrame::EnableMacroRunning( bool fEnable )
{
	bool fReturn = false;

	if( NULL != m_pDlgDebug || NULL != m_pDlgDebug->GetSafeHwnd() )
	{
		CTabDlgSpecActTesting2 *pTabDebug = m_pDlgDebug->GetpTabTesting2();

		if( NULL != pTabDebug )
		{
			pTabDebug->EnableMacroRunning( fEnable );
			fReturn = true;
		}
	}

	return fReturn;
}

bool CMainFrame::IsMacroRunning( bool &fIsRunning )
{
	bool fReturn = false;

	if( NULL != m_pDlgDebug || NULL != m_pDlgDebug->GetSafeHwnd() )
	{
		CTabDlgSpecActTesting2 *pTabDebug = m_pDlgDebug->GetpTabTesting2();

		if( NULL != pTabDebug )
		{
			fIsRunning = pTabDebug->IsMacroRunning();
			fReturn = true;
		}
	}

	return fReturn;
}

bool CMainFrame::WriteMacro( MSG *pMsg )
{
	bool fReturn = false;

	if( NULL != m_pDlgDebug || NULL != m_pDlgDebug->GetSafeHwnd() )
	{
		CTabDlgSpecActTesting2 *pTabDebug = m_pDlgDebug->GetpTabTesting2();

		if( NULL != pTabDebug )
		{
			pTabDebug->WriteMacro( pMsg );
			fReturn = true;
		}
	}

	return fReturn;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* TO AMELIORATE HYS-1251
// Remarks about these two following methods.
//
// 1st case: When TASelect loses the focus (if user goes to other application) three messages are sent to the 'CFrameWnd': 'OnActivateTopLevel', 'OnActivate'
//           and 'OnActivateApp'. And when gains the focus, it's in this order: 'OnActivateApp', 'OnActivateTopLevel' and 'OnActivate'.
//           What we want is to be able to not lost the object that had the focus before TASelect lost focus. By default, these three methods will reset the focus
//           on the active view. If we are for example in the BCV individual selection but the focus is in the left tab dialog on the 'Flow' edit control, when
//           TASelect regains the focus, we don't want that the right view has the focus but well the 'Flow' edit control. This is why we save the current focus
//           (m_HMainAppLostFocusNULL) in the 'OnActivateApp' (this is the last method called) when 'bActive' is 0 (0 signals that application becomes inactive).
//           To reset the focus, we can use this variable saved BUT NOT in the 'OnActivateApp'. Because both 'OnActivateTopLevel' and 'OnActivate' methods are called
//           after and will internally call the 'OnActivateView' that reset the focus on the active view. The best place to do it is in the 'OnActivate' (the last
//           method called when TASelect gains focus). We first call 'CFrameWndEx::OnActivate' to let framework do all its job and after that we set the saved focus.
//
// 2nd case: Each time a dialog is opened, the 'OnActivate' is called with 'nState' set to '0'. In this case, 'pWndOther' is the pointer on this dialog. In this case
//           we save the HWND that has the focus in 'm_HMainAppLostFocusDlg'. When the dialog is closed, the 'OnActivate' is called with 'nState' set to 1 or 2 and
//           'pWndOther' is the pointer on this dialog. In this condition, we can reset the focus saved in 'm_HMainAppLostFocusDlg'.
//
// 3rd case: If a dialog is opened and user goes outside TASelect, ONLY 'OnActivateApp' is called. And when TASelect regains focus, ALSO ONLY 'OnActivateApp' is
//           called. In this particular case, we can't reset the focus with
//

void CMainFrame::OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized )
{
#ifdef DEBUG
	CString str;
	str.Format( _T("OnActivate start. nState: %i; pWndOther: 0x%08x; bMinimized: %i\n"), nState, pWndOther, bMinimized );
	OutputDebugString( str );
#endif

	// We have also a special case.
	if( 0 == nState && NULL != pWndOther )
	{
		m_HMainAppLostFocusDlg = GetFocus()->GetSafeHwnd();
	}

	CFrameWndEx::OnActivate( nState, pWndOther, bMinimized );

	// Two cases: 1) user comes back to TASelect (by clicking on it or with TAB key for example), 'nState' is different of 0 and 'pWndOther' is NULL. In this case
	//               we reset the focus with 'm_HMainAppLostFocusNULL'.
	//            2) we close the dialog, 'nState' is different of 0 and 'pWndOther' is the pointer on this dialog. In this case, we reset the focus with
	//               'm_HMainAppLostFocusDlg'.
	if( 0 != nState )
	{
		if( NULL == pWndOther && NULL != m_HMainAppLostFocusNULL && NULL != CWnd::FromHandle( m_HMainAppLostFocusNULL ) )
		{
			CWnd::FromHandle( m_HMainAppLostFocusNULL )->SetFocus();
			m_HMainAppLostFocusNULL = NULL;
		}
		else if( NULL != pWndOther && NULL != m_HMainAppLostFocusDlg && NULL != CWnd::FromHandle( m_HMainAppLostFocusDlg ) )
		{
			CWnd::FromHandle( m_HMainAppLostFocusDlg )->SetFocus();
			m_HMainAppLostFocusDlg = NULL;
		}
	}

#ifdef DEBUG
	OutputDebugString( _T("OnActivate end.\n\n") );
#endif
}

void CMainFrame::OnActivateApp( BOOL bActive, DWORD dwThreadID )
{
#ifdef DEBUG
	CString str;
	str.Format( _T("OnActivateApp start. bActive: %i; dwThreadID: 0x%08x; bMinimized: %i\n"), bActive, dwThreadID );
	OutputDebugString( str );
#endif

	if( FALSE == bActive )
	{
		m_HMainAppLostFocusNULL = GetFocus()->GetSafeHwnd();
	}

	CFrameWndEx::OnActivateApp( bActive, dwThreadID );

#ifdef DEBUG
	OutputDebugString( _T("OnActivateApp end.\n\n") );
#endif
}
*/

int CMainFrame::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( -1 == CFrameWndEx::OnCreate( lpCreateStruct ) )
	{
		return -1;
	}

	// Set the visual manager and style based on persisted value.
	OnApplicationLook( TASApp.GetAppLook() );

	m_wndRibbonBar.Create( this );
	InitializeRibbon();

	// Initialize parameters with CMFCRibbonEditEx after the controls are created.
	// To initialize all control you MUST set each Category as active... Otherwise it will not work.
	m_wndRibbonBar.ShowContextCategories( ID_PRODUCTSEL_CATEGORY_BASE );
	m_wndRibbonBar.ShowContextCategories( ID_LOGDATA_CATEGORY );

	for( int i = 0; i < eRbnLastCat; i++ )
	{
		CMFCRibbonCategory *pCateg = m_wndRibbonBar.GetCategory( i );

		if( TRUE == pCateg->IsVisible() )
		{
			m_wndRibbonBar.SetActiveCategory( pCateg );
		}
	}

	m_wndRibbonBar.HideAllContextCategories();
	InitializeRibbonRichEditBoxes();
	m_wndRibbonBar.SetActiveCategory( m_pHomeCategory );

	// Create the Status bar.
	if( FALSE == m_wndStatusBar.Create( this, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM, AFX_IDW_STATUS_BAR ) )
	{
		TRACE0( "Failed to create status bar\n" );
		return -1;      // fail to create
	}

	// Define the indicators to display on the status bar.
	// Special treatment for indicators because there is a display bug when the ID_INDICATOR_* are not define in the RC file!
	static UINT indicators[] =
	{
		ID_SEPARATOR,           // status line indicator (used for the popup menu)
		ID_SEPARATOR,
		ID_SEPARATOR,
		ID_SEPARATOR
	};
	m_wndStatusBar.SetIndicators( indicators, sizeof( indicators ) / sizeof( UINT ) );
	int cxWidth = 30;

	// Define the Text according to the indicators and redefine the good indicators.
	m_wndStatusBar.SetPaneText( 1, TASApp.LoadLocalizedString( IDS_INDICATOR_CAPS ) );
	m_wndStatusBar.SetPaneInfo( 1, ID_INDICATOR_CAPS, SBPS_NORMAL, cxWidth );
	m_wndStatusBar.SetPaneText( 2, TASApp.LoadLocalizedString( IDS_INDICATOR_NUM ) );
	m_wndStatusBar.SetPaneInfo( 2, ID_INDICATOR_NUM, SBPS_NORMAL, cxWidth );
	m_wndStatusBar.SetPaneText( 3, TASApp.LoadLocalizedString( IDS_INDICATOR_SCRL ) );
	m_wndStatusBar.SetPaneInfo( 3, ID_INDICATOR_SCRL, SBPS_NORMAL, cxWidth );

	// Create the Wnd Tabs that will be displayed on the left of the application.
	if( FALSE == m_wndTabs.Create( TASApp.LoadLocalizedString( IDS_LEFTTAB ), this, CRect( 0, 0, 275, 0 ),
								   TRUE /* Has gripper */, 1000,
								   WS_CHILD | WS_VISIBLE | CBRS_LEFT | CBRS_FLOAT_MULTI,
								   AFX_CBRS_REGULAR_TABS, AFX_CBRS_RESIZE | AFX_CBRS_AUTOHIDE ) )
	{
		TRACE0( "Failed to create output bar\n" );
		return -1;      // fail to create
	}

	m_wndTabs.SetActiveTab( CMyMFCTabCtrl::etiProj );
	// Attach the panel to the frame.
	DockPane( &m_wndTabs );

	// Enable Visual Studio 2005 style docking window behavior.
	CDockingManager::SetDockingMode( DT_SMART );

	// Enable Visual Studio 2005 style docking window auto-hide behavior.
	EnableAutoHidePanes( CBRS_ALIGN_ANY );

	// Set AutoSaveTimer.
	m_uiTimer = SetTimer( _TIMERID_MAIN, AUTOSAVETIMER, NULL );

	m_wndRibbonBar.ForceRecalcLayout();

	return 0;
}

void CMainFrame::OnSize( UINT nType, int cx, int cy )
{
	m_ToolsDockablePane.BlockSavingDockSize( true );
	CFrameWndEx::OnSize( nType, cx, cy );
	
	if( m_ToolsDockablePane.GetSafeHwnd() != NULL )
	{
		m_ToolsDockablePane.OnMainFrameSize( nType, cx, cy );
	}
	
	m_ToolsDockablePane.BlockSavingDockSize( false );

	SendMessageToDescendants( WM_USER_MAINFRAMERESIZE );
}

void CMainFrame::OnSizing( UINT fwSide, LPRECT pRect )
{
	m_ToolsDockablePane.BlockSavingDockSize( true );
	CFrameWndEx::OnSizing( fwSide, pRect );

	if( m_ToolsDockablePane.GetSafeHwnd() != NULL )
	{
		m_ToolsDockablePane.OnMainFrameSizing( fwSide, pRect );
	}

	m_ToolsDockablePane.BlockSavingDockSize( false );
}

void CMainFrame::OnClose()
{
	CMyPreviewViewEx *pView = dynamic_cast<CMyPreviewViewEx *>( GetActiveView() );

	if( NULL != pView )
	{
		SendMessage( WM_COMMAND, AFX_ID_PREVIEW_CLOSE, 0 );
	}
	else
	{
		if( true == pTASelectDoc->CloseTASelect() )
		{
			CFrameWndEx::OnClose();
		}
	}
}

BOOL CMainFrame::OnQueryEndSession()
{
	if( FALSE == CFrameWndEx::OnQueryEndSession() )
	{
		return FALSE;
	}

	pTASelectDoc->CloseTASelect();

	return TRUE;
}

void CMainFrame::OnEndSession( BOOL bEnding )
{
	CFrameWndEx::OnEndSession( bEnding );
	pTASelectDoc->CloseTASelect();
}

void CMainFrame::OnTimer( UINT_PTR nIDEvent )
{
	CFrameWndEx::OnTimer( nIDEvent );

	if( ( UINT_PTR )0 == m_uiTimer || m_uiTimer != nIDEvent )
	{
		return;
	}

	if( NULL != pTASelectDoc )
	{
		pTASelectDoc->AutoSave();
	}
}

void CMainFrame::OnApplicationLook( UINT uiID )
{
	CWaitCursor wait;

	TASApp.SetAppLook( uiID );

	switch( TASApp.GetAppLook() )
	{
		case ID_VIEW_APPLOOK_WIN_2000:
			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManager ) );
			m_wndRibbonBar.SetWindows7Look( FALSE );
			break;

		case ID_VIEW_APPLOOK_OFF_XP:
			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerOfficeXP ) );
			m_wndRibbonBar.SetWindows7Look( FALSE );
			break;

		case ID_VIEW_APPLOOK_WIN_XP:
			CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerWindows ) );
			m_wndRibbonBar.SetWindows7Look( FALSE );
			break;

		case ID_VIEW_APPLOOK_OFF_2003:
			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerOffice2003 ) );
			CDockingManager::SetDockingMode( DT_SMART );
			m_wndRibbonBar.SetWindows7Look( FALSE );
			break;

		case ID_VIEW_APPLOOK_VS_2005:
			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerVS2005 ) );
			CDockingManager::SetDockingMode( DT_SMART );
			m_wndRibbonBar.SetWindows7Look( FALSE );
			break;

		case ID_VIEW_APPLOOK_VS_2008:
			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerVS2008 ) );
			CDockingManager::SetDockingMode( DT_SMART );
			m_wndRibbonBar.SetWindows7Look( FALSE );
			break;

		case ID_VIEW_APPLOOK_WINDOWS_7:
			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerWindows7 ) );
			CDockingManager::SetDockingMode( DT_SMART );
			m_wndRibbonBar.SetWindows7Look( TRUE );
			break;

		default:
			switch( TASApp.GetAppLook() )
			{
				case ID_VIEW_APPLOOK_OFF_2007_BLUE:
					CMFCVisualManagerOffice2007::SetStyle( CMFCVisualManagerOffice2007::Office2007_LunaBlue );
					break;

				case ID_VIEW_APPLOOK_OFF_2007_BLACK:
					CMFCVisualManagerOffice2007::SetStyle( CMFCVisualManagerOffice2007::Office2007_ObsidianBlack );
					break;

				case ID_VIEW_APPLOOK_OFF_2007_SILVER:
					CMFCVisualManagerOffice2007::SetStyle( CMFCVisualManagerOffice2007::Office2007_Silver );
					break;

				case ID_VIEW_APPLOOK_OFF_2007_AQUA:
					CMFCVisualManagerOffice2007::SetStyle( CMFCVisualManagerOffice2007::Office2007_Aqua );
					break;
			}

			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerOffice2007 ) );
			CDockingManager::SetDockingMode( DT_SMART );
			m_wndRibbonBar.SetWindows7Look( FALSE );
	}

	// 	m_wndOutput.UpdateFonts();
	//
	// 	// Force to keep the afx resource from the DLL when the user change the visual style
	// 	// Bug known by MFC...
	// 	CMFCVisualManagerOffice2007Ex::SetAutoFreeRes( FALSE );
	//
	// 	switch( TASApp.GetAppLook() )
	// 	{
	// 		case ID_VIEW_APPLOOK_WIN_2000:
	// 			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManager ) );
	// 			break;
	//
	// 		case ID_VIEW_APPLOOK_OFF_XP:
	// 			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerOfficeXP ) );
	// 			break;
	//
	// 		case ID_VIEW_APPLOOK_WIN_XP:
	// 			CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
	// 			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerWindows ) );
	// 			break;
	//
	// 		case ID_VIEW_APPLOOK_OFF_2003:
	// 			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerOffice2003 ) );
	// 			CDockingManager::SetDockingMode( DT_SMART );
	// 			break;
	//
	// 		case ID_VIEW_APPLOOK_VS_2005:
	// 			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerVS2005));
	// 			CDockingManager::SetDockingMode(DT_SMART);
	// 			break;
	//
	// 		default:
	// 			switch( TASApp.GetAppLook() )
	// 			{
	// 				case ID_VIEW_APPLOOK_OFF_2007_BLUE:
	// 					CMFCVisualManagerOffice2007::SetStyle( CMFCVisualManagerOffice2007::Office2007_LunaBlue );
	// 					break;
	//
	// 				case ID_VIEW_APPLOOK_OFF_2007_BLACK:
	// 					CMFCVisualManagerOffice2007::SetStyle( CMFCVisualManagerOffice2007::Office2007_ObsidianBlack );
	// 					break;
	//
	// 				case ID_VIEW_APPLOOK_OFF_2007_SILVER:
	// 					CMFCVisualManagerOffice2007::SetStyle( CMFCVisualManagerOffice2007::Office2007_Silver );
	// 					break;
	//
	// 				case ID_VIEW_APPLOOK_OFF_2007_AQUA:
	// 					CMFCVisualManagerOffice2007::SetStyle( CMFCVisualManagerOffice2007::Office2007_Aqua );
	// 					break;
	// 			}
	//
	// 			CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerOffice2007 ) );
	// 			CDockingManager::SetDockingMode( DT_SMART );
	// 	}
	//
	// Retrieve interface color On look change.
	COLORREF clrDark;
	COLORREF clrBlack;
	COLORREF clrHighlight;
	COLORREF clrFace;
	COLORREF clrDarkShadow;
	COLORREF clrLight;
	CBrush *pbrFace = NULL;
	CBrush *pbrBlack = NULL;
	CMFCVisualManager::GetInstance()->GetTabFrameColors( m_wndTabs.GetMFCTabCtrl(), clrDark, clrBlack, clrHighlight, clrFace, clrDarkShadow, clrLight, pbrFace,
			pbrBlack );
	TASApp.SetBckColor( clrDarkShadow );

	RedrawWindow( NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE );

	TASApp.WriteInt( L"ApplicationLook", TASApp.GetAppLook() );
	this->m_wndTabs.OnApplicationLook();

	for( int i = 0; i < m_arrayViews.GetCount(); i++ )
	{
		CFormViewEx *pForm = dynamic_cast<CFormViewEx *>( m_arrayViews.GetAt( i ) );

		if( NULL != pForm )
		{
			pForm->OnApplicationLook( uiID );
		}
	}
}

void CMainFrame::OnUpdateApplicationLook( CCmdUI *pCmdUI )
{
	pCmdUI->SetRadio( TASApp.GetAppLook() == pCmdUI->m_nID );
}

LRESULT CMainFrame::OnDisplayMsgBox( WPARAM wParam, LPARAM lParam )
{
	return TASApp.AfxLocalizeMessageBox( wParam, lParam );
}

LRESULT CMainFrame::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	// Variables.
	CDS_ProjectRef *pPrjRef = TASApp.GetpTADS()->GetpProjectRef();
	ASSERT( NULL != pPrjRef );

	// Set the string in the Project Reference.
	m_pEBProjName->SetEditText( (CString)pPrjRef->GetString( CDS_ProjectRef::Name ) );

	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();

	// Change project type in the ribbon for project (Heating or cooling).
	ProjectType eProjectApplicationType = pTechP->GetProjectApplicationType();

	// Set technical parameters to 'InvalidProjectType' to force a refresh.
	pTechP->SetProjectApplicationType( ProjectType::InvalidProjectType	);
	ChangeRbnProjectApplicationType( eProjectApplicationType );
	
	// HYS-1334 : Combo application type is not updated when opening project
	if( NULL != m_pCBProductSelectionApplicationType )
	{
		int iCurSel = m_pCBProductSelectionApplicationType->GetCurSel();

		if( iCurSel >= 0 )
		{
			ProjectType eComboProductSelectionApplicationType = (ProjectType)m_pCBProductSelectionApplicationType->GetItemData( iCurSel );

			if( pTechP->GetProductSelectionApplicationType() != eComboProductSelectionApplicationType )
			{
				// update combo application type for selection
				for( int i = 0; i < m_pCBProductSelectionApplicationType->GetCount(); ++i )
				{
					if( (int)m_pCBProductSelectionApplicationType->GetItemData( i ) == (int)(pTechP->GetProductSelectionApplicationType()) )
					{
						m_pCBProductSelectionApplicationType->SelectItem( i );
						break;
					}
				}
			}
		}
	}

	// HYS-1590: If we are already in the "Product selections tools\Tech. parameters" ribbon category when opening a project, the pipe combos
	// are not updated (These ones are updated when this ribbon category is selected).
	_UpdatePipeCombosForProductSelection();

	// To force a refresh of the ribbon.
	InitializeRibbonRichEditBoxes();

	// Pay attention we have now two flags: one in the 'CDS_HydroMod' class and one in the 'CDS_PrjParams'. The first one (the old one)
	// tell us if the project has been converted in hydronic calculation mode. In the old version, the project was converted either
	// intentionally by the user (when clicking on the HM calc button in the ribbon) or when user sent his project to the TAScope.
	// Now, we have a second variable in 'CDS_PrjParams' that tells us if user has intentionally pressed the HM calc button to convert
	// his project from the CBI mode to the hydronic calculation mode.
	// Why we need that? To not automatically going into the hydronic calculation mode when opening a tsp project that has the 'HMCalcMode'
	// flag set to 'true'. If this an old project, we stay in the CBI mode. If an user has built his project in the CBI mode and sent it
	// to the TAScope, he doesn't want to go in hydronic calculation mode when importing back his project in TASelect.
	if( true == TASApp.GetpTADS()->IsOldProject() )
	{
		if( false == TASApp.GetpTADS()->IsHMCalcMode() )
		{
			SetHMCalcMode( false );
		}
		else
		{
			// Don't change HM calculation button.
			// Set the new flag to 'true' to specify this project has been converted in hydronic calculation mode.
			TASApp.GetpTADS()->SetUserForcedHMCalcModeFlag( true );
		}
	}
	else
	{
		// Check if current HySelect is activated before to go in HM calculation mode if needed.
		CDlgRegister DlgReg;
		int nDay = DlgReg.Init();

		if( nDay > 0 )
		{
			// Set the HM calculation button in regards to the new flag.
			SetHMCalcMode( TASApp.GetpTADS()->GetUserForcedHMCalcModeFlag() );
		}
		else
		{
			SetHMCalcMode( false );
		}
	}

	// Is something in Piping Tab?
	CTable *pPipTab = TASApp.GetpTADS()->GetpHydroModTable();
	ASSERT( NULL != pPipTab );

	CDS_ProjectParams *pProjectParams = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pProjectParams );

	if( '\0' != *pPipTab->GetFirst().ID )
	{
		pProjectParams->SetFreeze( true );
	}
	else
	{
		pProjectParams->SetFreeze( false );
	}

	CView *pView = GetActiveView();

	for( int nView = 0; nView < m_arrayViews.GetCount(); nView++ )
	{
		if( pView != m_arrayViews[nView] )
		{
			m_arrayViews[nView]->ShowWindow( SW_HIDE );
			m_arrayViews[nView]->EnableWindow( TRUE );
		}
	}

	if( NULL != m_pDlgDiversityFactor )
	{
		if( m_pDlgDiversityFactor->GetSafeHwnd() != NULL )
		{
			m_pDlgDiversityFactor->DestroyWindow();
		}

		delete m_pDlgDiversityFactor;
		m_pDlgDiversityFactor = NULL;
	}

	// If we are in debug mode (CTRL+SHIFT+P).
	if( true == TASApp.GetpTADB()->IsDebugPsw() && false == m_bHMCalcDebugPanelVisible )
	{
		m_pPanelProjectDebug = m_pProjectCategory->AddPanel( _T("Debug") );
		m_pChBVerifyAutority = new CMFCRibbonCheckBox( ID_RBN_P_DEBUG_VERIFYAUTORITY, _T("Verify autority") );
		m_pPanelProjectDebug->Add( m_pChBVerifyAutority );
		m_pPanelProjectDebug->SetJustifyColumns();
		m_bHMCalcDebugPanelVisible = true;
	}

	return 0;
}

LRESULT CMainFrame::OnOpenDocument( WPARAM wParam, LPARAM lParam )
{
	bool checkIfModified = ( wParam ? true : false );

	if( false == checkIfModified )
	{
		CTADatastruct *pTADS = TASApp.GetpTADS();
		pTADS->Modified( false );
	}

	CString *file1 = ( CString * )lParam;
	LPCTSTR szTemp = (LPCTSTR)( *file1 );
	pTASelectDoc->OnOpenDocument( szTemp );
	pTASelectDoc->SetPathName( szTemp, FALSE );
	return 0;
}

LRESULT CMainFrame::OnExportSelPToXlsx(WPARAM wParam, LPARAM lParam)
{
	CString *file1 = (CString *)lParam;
	ExportSelPtoXLSX(*file1);
	return 0;
}

LRESULT CMainFrame::OnExportSelPInOneToXlsx(WPARAM wParam, LPARAM lParam)
{
	CString *file1 = (CString *)lParam;
	ExportSelPInOnetoXLSX(*file1);
	return 0;
}

LRESULT CMainFrame::OnImportForUnitTest( WPARAM wParam, LPARAM lParam )
{
	struct param
	{
		CString file;
		CStringArray* pArrayParam;
	};
	struct param* pParam = (struct param*)lParam;
	CString strFileNameToImport = pParam->file;
	CStringArray* pArrayParam = pParam->pArrayParam;
	int iArraySize = pArrayParam->GetSize();
	BeginWaitCursor();
	
	CDlgImportHM dlg( strFileNameToImport, pArrayParam, iArraySize );

	if( FALSE == dlg.Create( CDlgImportHM::IDD ) )
	{
		return 0;
	}
	else
	{
		dlg.AddItemsUnitTests();
	}
	EndWaitCursor();
	return 0;
}

LPARAM CMainFrame::OnRibbonCategoryChanged( WPARAM wParam, LPARAM lParam )
{
	_UpdatePipeCombosForProductSelection();
	return 0;
}

void CMainFrame::OnAppMenuPrint()
{
	if( false == TASApp.GetpTADS()->IsSomethingToPrint() )
	{
		return;
	}

	if( NULL == m_pDlgPrint )
	{
		m_pDlgPrint = new CDlgPrint();

		if( NULL == m_pDlgPrint )
		{
			return;
		}

		if( FALSE == m_pDlgPrint->Create( CDlgPrint::IDD, this ) )
		{
			delete m_pDlgPrint;
			m_pDlgPrint = NULL;
			return;
		}
	}

	m_pDlgPrint->ShowWindow( SW_SHOW );
	m_pDlgPrint->SetFocus();
}

void CMainFrame::OnAppMenuSubPrint()
{
	if( NULL == m_pDlgPrint )
	{
		m_pDlgPrint = new CDlgPrint();

		if( NULL == m_pDlgPrint )
		{
			return;
		}

		if( FALSE == m_pDlgPrint->Create( CDlgPrint::IDD, this ) )
		{
			delete m_pDlgPrint;
			m_pDlgPrint = NULL;
			return;
		}
	}

	m_pDlgPrint->ShowWindow( SW_SHOW );
	m_pDlgPrint->SetFocus();
}

void CMainFrame::OnAppMenuExportSelP()
{
	if( NULL != pRViewSelProd )
	{
		pRViewSelProd->OnFileExportSelp();
	}
}

void CMainFrame::OnAppMenuExportSelInOne()
{
	if( NULL != pRViewSelProd )
	{
		pRViewSelProd->OnFileExportSelInOneSheet();
	}
}

void CMainFrame::OnAppMenuExportHMToJson()
{
	CExportHMToJson clExportHMToJson;
	CString strErrorMsg; 

	CTADatastruct *pclTADatastruct = TASApp.GetpTADS();
	CDS_TechnicalParameter *pclTechParams = TASApp.GetpTADS()->GetpTechParams();

	if( false == clExportHMToJson.Export( pclTADatastruct->GetpHydroModTable(), pclTechParams->GetProjectApplicationType(),
			*pclTADatastruct->GetpWCForProject()->GetpWCData(), pclTechParams->GetDefaultPrjTpr(), _T("c:\\temp\\export_json.txt"), strErrorMsg ) )
	{
		CString strMessage;
		strMessage.Format( _T("Export error.\r\n%s"), strErrorMsg );
		AfxMessageBox( strMessage, MB_OK );
	}
	else
	{
		AfxMessageBox( _T("Export OK!"), MB_OK );
	}
}

void CMainFrame::ExportSelPtoXLSX( CString strFn )
{
	if( NULL != pRViewSelProd )
	{
		pRViewSelProd->OnFileExportSelp(strFn);
	}
}

void CMainFrame::ExportSelPInOnetoXLSX( CString strFn )
{
	if( NULL != pRViewSelProd )
	{
		pRViewSelProd->OnFileExportSelInOneSheet(strFn);
	}
}

void CMainFrame::PostMessageToDescendants( UINT message, WPARAM wParam, LPARAM lParam, BOOL bDeep )
{
	_PostMessageToDescendants( m_hWnd, message, wParam, lParam, bDeep );
}

void CMainFrame::ShowStartPage()
{
	if( NULL == m_ToolsDockablePane.GetSafeHwnd() )
	{
		ASSERT_RETURN;
	}

	if( NULL != pDlgLTtabctrl )
	{
		pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiProj );
	}

	GetRibbonBar()->SetActiveCategory( GetRibbonProjectCategory() );
	ActiveFormView( RightViewList::eRVStartPage );

	if( -1 == m_iStartPageToolsDockablePaneContextID )
	{
		CToolsDockablePane::ContextParameters *pclContext = m_ToolsDockablePane.RegisterContext();

		if( NULL == pclContext )
		{
			ASSERT_RETURN;
		}

		pclContext->m_bIsPaneVisible = false;
		pclContext->m_bIsPaneAutoHide = FALSE;
		pclContext->m_bIsGripperEnabled = FALSE;
		pclContext->m_bIsPaneFullScreen = false;
		pclContext->m_bIsContextMenuEnabled = false;
		pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_HMCompilation] = false;
		pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = false;
		pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = false;
		pclContext->m_iLinkedRightView = CMainFrame::RightViewList::eRVStartPage;
		m_iStartPageToolsDockablePaneContextID = pclContext->m_iContextID;
	}

	m_ToolsDockablePane.RestoreContext( m_iStartPageToolsDockablePaneContextID );
}

bool CMainFrame::IsMenuHMImportEnable()
{
	if( NULL != TASApp.GetpTADS()->GetpHydroModTable() )
	{
		CTable* pTable = TASApp.GetpTADS()->GetpHydroModTable();
		CDS_HydroMod* pChild = (CDS_HydroMod*)( pTable->GetFirst().MP );
		if( NULL != pChild && true == IsHMCalcChecked() && true == pChild->GetpPrjParam()->IsFreezed() )
		{
			m_bIsMenuImportEnable = false;
		}
		else
		{
			m_bIsMenuImportEnable = true;
		}
	}
	else
	{
		m_bIsMenuImportEnable = true;
	}

	return m_bIsMenuImportEnable;
}

bool CMainFrame::IsRibbonHMImportEnable()
{
	if( NULL != TASApp.GetpTADS()->GetpHydroModTable() )
	{
		CTable* pTable = TASApp.GetpTADS()->GetpHydroModTable();
		CDS_HydroMod* pChild = (CDS_HydroMod*)( pTable->GetFirst().MP );
		if( NULL != pChild && true == IsHMCalcChecked() && true == pChild->GetpPrjParam()->IsFreezed() )
		{
			m_bIsRibbonImportEnable = false;
		}
		else
		{
			m_bIsRibbonImportEnable = true;
		}
	}
	else
	{
		m_bIsRibbonImportEnable = true;
	}
	return m_bIsRibbonImportEnable;
}

bool CMainFrame::IsHMCalcDebugPanelVisible()
{
	return m_bHMCalcDebugPanelVisible;
}

bool CMainFrame::IsHMCalcDebugVerifyAutority()
{
	return m_bHMCalcDebugVerifyAutority;
}

void CMainFrame::OnAppMenuExportChart()
{
	if( NULL != pRViewChart )
	{
		pRViewChart->CallOnFileExportChart();
	}
}

void CMainFrame::OnAppMenuExportLDList()
{
	if( NULL != pRViewLogData )
	{
		pRViewLogData->CallOnFileExportLdlist();
	}
}

void CMainFrame::OnAppMenuExportQM()
{
	if( NULL != pRViewQuickMeas )
	{
		pRViewQuickMeas->CallOnFileExportQM();
	}
}

void CMainFrame::OnAppMenuExportTADiagnoctic()
{
	if( NULL != pRViewSelProd )
	{
		pRViewSelProd->OnFileExportTADiagnostic();
	}
}

void CMainFrame::OnAppMenuExportPipes()
{
	CString str;
	str = TASApp.LoadLocalizedString( IDS_FILE_EXPORT_PIPESERIES );
	CFileDialog dlg( false, L"exp", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXTENSIONDIFFERENT, (LPCTSTR)str, NULL );
	CString PrjDir = GetProjectDirectory();

	dlg.m_ofn.lpstrInitialDir = (LPCTSTR)PrjDir;

	if( IDOK == dlg.DoModal() )
	{
		// Save Project directory if modified.
		if( dlg.GetPathName() != PrjDir )
		{
			CPath path( dlg.GetPathName() );
			CString str = path.SplitPath( ( CPath::ePathFields )( CPath::ePathFields::epfDrive + CPath::ePathFields::epfDir ) );
			AfxGetApp()->WriteProfileString( L"Files", L"Project Directory", str );
		}

		// Create exportation DB.
		CPipeUserDatabase ExpDB;
		ExpDB.Export( TASApp.GetpPipeDB() );

		// Drop ExpDB on disk.
		TRACE( L"Export user pipes into exp file %s.\n", ( TCHAR * )(LPCTSTR)dlg.GetPathName() );
		std::ofstream outf( dlg.GetPathName(), std::ofstream::out | std::ofstream::binary | std::ofstream::trunc );
		ASSERT( !( outf.rdstate() & std::ofstream::failbit ) );
		ExpDB.Write( outf );
		outf.close();
	}
}

void CMainFrame::OnAppMenuImportPipes()
{
	try
	{
		CString str;
		str = TASApp.LoadLocalizedString( IDS_FILE_IMPORT_PIPESERIES );
		CFileDialog dlg( true, L"exp", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, (LPCTSTR)str, NULL );
		CString PrjDir = GetProjectDirectory();

		dlg.m_ofn.lpstrInitialDir = (LPCTSTR)PrjDir;

		if( IDOK == dlg.DoModal() )
		{
			// HYS-703: A double click goes like this: mouse down, mouse up, double click, mouse up.The dialog disappears between the double click message
			// and the second mouse up message. The control receives the mouse up message and treats it as a full click, we need to stop it.
			// Avoid that the double-click on the CFileDialog sends the WM_LBUTTONUP message to the next window.
			MSG msg;
			PeekMessage( &msg, 0, WM_LBUTTONUP, WM_LBUTTONUP, PM_REMOVE );

			// Save Project directory if modified.
			CPath path( dlg.GetPathName() );
			str = path.SplitPath( ( CPath::ePathFields )( CPath::ePathFields::epfDrive + CPath::ePathFields::epfDir ) );

			if( str != PrjDir )
			{
				AfxGetApp()->WriteProfileString( L"Files", L"Project Directory", str );
			}

			CPipeUserDatabase ExpDB;
			std::ifstream inpf( dlg.GetPathName(), std::ifstream::in | std::ifstream::binary );
			ASSERT( !( inpf.rdstate() & std::ifstream::failbit ) );

			ExpDB.Read( inpf );
			inpf.close();

			if( false == ExpDB.CheckImportValidity() )
			{
				FormatString( str, AFXMSG_IMPORT_FAULT, dlg.GetFileName() );
				AfxMessageBox( str );
				return;
			}

			// HYS-1590: Old ID form "{AAAA}_GEN_ID" is now converted to "AAAA_UPIPS_OLD" or "AAAA_UPIPE_OLD".
			// See more details in the Jira card.
			ExpDB.RenameOldPipeIDs();

			// The imported file is correct, introduce all pipe series and pipes into the USERDB.
			// First to verify if the imported pipes are not already present in the USERDB.
			// Second to import pipes.
			if( false == TASApp.GetpPipeDB()->FindDuplicate( &ExpDB ) )
			{
				TASApp.GetpPipeDB()->CopyFrom( &ExpDB );

				// Fill the ribbon bar with the new list of pipes. 
				// For project.
				FillPipeSeriesCB();

				// For product selection.
				FillProductSelPipeSeriesCB();

				// Send message to notify that the pipe database has been modified.
				::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_PIPECHANGE, ( WPARAM )WM_UPC_ForProject );
			}
			else
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_PIPESCANTBEIMPORTED );
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CMainFrame::OnAppMenuImportPipes'."), __LINE__, __FILE__ );
		throw;
	}
}

void CMainFrame::OnAppMenuImportHM()
{
	CDlgImportHM dlg;
	dlg.DoModal();
}

void CMainFrame::OnAppMenuImportHMJson()
{
	CString strFileName = _T("");
	CString str = TASApp.LoadLocalizedString( IDS_OPENSAVEJSONFILEFILTER );
	CFileDialog dlg( true, _T("json"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, (LPCTSTR)str, NULL );
	CString PrjDir = GetProjectDirectory();

	CString strReturn = _T( "" );
	dlg.m_ofn.lpstrInitialDir = (LPCTSTR)PrjDir;

	if( IDOK == dlg.DoModal() )
	{
		// HYS-703: A double click goes like this: mouse down, mouse up, double click, mouse up.The dialog disappears between the double click message
		// and the second mouse up message. The control receives the mouse up message and treats it as a full click, we need to stop it.
		// Avoid that the double-click on the CFileDialog sends the WM_LBUTTONUP message to the next window.
		MSG msg;
		PeekMessage( &msg, 0, WM_LBUTTONUP, WM_LBUTTONUP, PM_REMOVE );

		strFileName = dlg.GetPathName();
	}

	if( true == strFileName.IsEmpty() )
	{
		return;
	}
		
	// Try to read the .json file.
	CString strErrorMsg;
	CImportHMFromJson clImportHMFromJson;
		
	if( false == clImportHMFromJson.Import( strFileName, strErrorMsg ) )
	{
		::AfxMessageBox( strErrorMsg );
		return;
	}
}

void CMainFrame::OnRbnPrjLaunchHNCheck()
{
	TASApp.GetpTADS()->VerifyAllRootModules();
}

void CMainFrame::OnAppMenuExit()
{
	OnClose();
}

void CMainFrame::OnAppMenuStartPage()
{
	ShowStartPage();
}

void CMainFrame::OnAppMenuPMWizard()
{
	if( NULL != pDlgLTtabctrl )
	{
		pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
		pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_PM_ExpansionVessel );
		pMainFrame->ClickRibbonProductSelectionMode( ProductSelectionMode::ProductSelectionMode_Wizard );
	}
}

void CMainFrame::OnFilePrint()
{
	if( NULL == m_pDlgPrint || NULL == m_pDlgPrint->GetSafeHwnd() )
	{
		return;
	}

	// When print preview is active and user clicks on the 'Print' button in the ribbon, a 'AFX_ID_PRINT_PREVIEW' message is sent to the current
	// 'CPrintPreview' class. This one will close the print preview and send a 'ID_FILE_PRINT' command to the main frame. We thus intercept this
	// command to call the appropriate method in the dialog print.
	m_pDlgPrint->Print_PrintPrev( false );
}

void CMainFrame::OnRbnHDefUnits()
{
	CDlgDefUnits dlg;

	if( IDOK == dlg.Display( CDimValue::AccessUDB() ) )
	{
		// Variable.
		CMFCRibbonComboBox *pCombo = NULL;
		int nSel = -1;

		// Change the flow combo box in the ribbon bar.
		nSel = CDimValue::AccessUDB()->GetDefaultUnitIndex( _U_FLOW );

		if( nSel != CB_ERR )
		{
			pCombo = DYNAMIC_DOWNCAST( CMFCRibbonComboBox, m_wndRibbonBar.FindByID( ID_RBN_H_DU_FLOW ) );

			if( NULL == pCombo )
			{
				return;
			}

			pCombo->SelectItem( nSel );
		}

		// Change the Dp combo box in the ribbon bar.
		nSel = CDimValue::AccessUDB()->GetDefaultUnitIndex( _U_DIFFPRESS );

		if( nSel != CB_ERR )
		{
			pCombo = DYNAMIC_DOWNCAST( CMFCRibbonComboBox, m_wndRibbonBar.FindByID( ID_RBN_H_DU_DIFFPRESS ) );

			if( NULL == pCombo )
			{
				return;
			}

			pCombo->SelectItem( nSel );
		}

		// Change the temp combo box in the ribbon bar.
		nSel = CDimValue::AccessUDB()->GetDefaultUnitIndex( _U_TEMPERATURE );

		if( nSel != CB_ERR )
		{
			pCombo = DYNAMIC_DOWNCAST( CMFCRibbonComboBox, m_wndRibbonBar.FindByID( ID_RBN_H_DU_TEMP ) );

			if( NULL == pCombo )
			{
				return;
			}

			pCombo->SelectItem( nSel );
		}

		// Refresh water characteristics in the ribbon for project and product selection.
		OnWaterChange( WMUserWaterCharWParam::WM_UWC_WP_ForAll, WMUserWaterCharLParam::WM_UWC_LWP_Change | WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver );

		// Send message that unit have been changed.
		AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_UNITCHANGE );
	}
}

void CMainFrame::OnRbnHDUFlow()
{
	// Set the focus on the MainFrame to be sure the killfocus function of other windows have been done properly.
	this->SetFocus();

	CMFCRibbonComboBox *pCombo = DYNAMIC_DOWNCAST( CMFCRibbonComboBox, m_wndRibbonBar.FindByID( ID_RBN_H_DU_FLOW ) );

	if( NULL == pCombo )
	{
		return;
	}

	int nSel = pCombo->GetCurSel();

	if( nSel != CB_ERR )
	{
		CDimValue::AccessUDB()->SetDefaultUnit( _U_FLOW, nSel );
		AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_UNITCHANGE );
	}
}

void CMainFrame::OnRbnHDUDiffPres()
{
	// Set the focus on the MainFrame to be sure the killfocus function of other windows have been done properly.
	this->SetFocus();

	CMFCRibbonComboBox *pCombo = DYNAMIC_DOWNCAST( CMFCRibbonComboBox, m_wndRibbonBar.FindByID( ID_RBN_H_DU_DIFFPRESS ) );

	if( NULL == pCombo )
	{
		return;
	}

	int nSel = pCombo->GetCurSel();

	if( CB_ERR == nSel )
	{
		return;
	}

	CDimValue::AccessUDB()->SetDefaultUnit( _U_DIFFPRESS, nSel );
	AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_UNITCHANGE );
}

void CMainFrame::OnRbnHDUTemp()
{
	// Set the focus on the MainFrame to be sure the killfocus function of other windows have been done properly.
	this->SetFocus();

	CMFCRibbonComboBox *pCombo = DYNAMIC_DOWNCAST( CMFCRibbonComboBox, m_wndRibbonBar.FindByID( ID_RBN_H_DU_TEMP ) );

	if( NULL == pCombo )
	{
		return;
	}

	int nSel = pCombo->GetCurSel();

	if( nSel != CB_ERR )
	{
		CDimValue::AccessUDB()->SetDefaultUnit( _U_TEMPERATURE, nSel );
		AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_UNITCHANGE );

		// Refresh water characteristics in the ribbon.
		OnWaterChange( WMUserWaterCharWParam::WM_UWC_WP_ForAll, WMUserWaterCharLParam::WM_UWC_LWP_Change );
	}
}

void CMainFrame::OnRbnHPipeSeries()
{
	CDlgCustPipe dlg;
	dlg.Display();
}

void CMainFrame::OnRbnHPSDefPipeSeries()
{
	if( NULL == m_pCBPipeSeries )
	{
		return;
	}

	int nSel = m_pCBPipeSeries->GetCurSel();

	if( CB_ERR == nSel )
	{
		return;
	}

	// Get the Technical parameters.
	CDS_TechnicalParameter *pTechParam = TASApp.GetpTADS()->GetpTechParams();

	if( NULL == pTechParam )
	{
		return;
	}

	// Get the current pipe series tab.
	CTable *plcPipeSeriesTable = (CTable *)( TASApp.GetpPipeDB()->Get( m_pCBPipeSeries->GetItemID( nSel ) ).MP );

	if( NULL == plcPipeSeriesTable )
	{
		return;
	}

	// Get the first pipe in the current series.
	CString strID = plcPipeSeriesTable->GetIDPtr().ID;
	CDB_Pipe *pPipe = (CDB_Pipe *)( plcPipeSeriesTable->GetFirst().MP );

	if( NULL == pPipe )
	{
		return;
	}

	// Verify the pipe is not a user pipe, the pipe is selectable and is different than the current default pipes serie.
	if( false == pPipe->IsFixed() || true == plcPipeSeriesTable->IsHidden() || strID == pTechParam->GetDefaultPipeSerieID() )
	{
		return;
	}

	// Set the new default pipe serie.
	pTechParam->SetDefaultPipeSerieID( plcPipeSeriesTable->GetIDPtr().ID );

	// Set the new default pipe series for the current project too.
	CPrjParams *pPrjParams = TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pPrjParams );

	pPrjParams->SetPrjParamID( CPrjParams::PipeCircSerieID, (LPCTSTR)strID );
	pPrjParams->SetPrjParamID( CPrjParams::PipeByPassSerieID, (LPCTSTR)strID );
	pPrjParams->SetPrjParamID( CPrjParams::PipeDistSupplySerieID, (LPCTSTR)strID );
	pPrjParams->SetPrjParamID( CPrjParams::PipeDistReturnSerieID, (LPCTSTR)strID );

	// Write the pipes in the registry and send a message that pipe change.
	AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_PIPECHANGE, ( WPARAM )WMUserPipeChange::WM_UPC_ForProject );
	AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PIPECHANGE, ( WPARAM )WMUserPipeChange::WM_UPC_ForProject );
}

void CMainFrame::OnRbnHTechParam()
{
	CDlgTechParam dlg;
	dlg.DoModal();
}

void CMainFrame::OnRbnHTPDSSizeAbv()
{
	_RbnTPSizeAbv( false );
}

void CMainFrame::OnRbnHTPDSSizeBLW()
{
	_RbnTPSizeBLW( false );
}

void CMainFrame::OnRbnHTPPDTarget()
{
	_RbnTPPDTarget( false );
}

void CMainFrame::OnRbnHTPPDMax()
{
	_RbnTPPDMax( false );
}

void CMainFrame::OnRbnHTPWVTarget()
{
	_RbnTPWVTarget( false );
}

void CMainFrame::OnRbnHTPWVMax()
{
	_RbnTPWVMax( false );
}

void CMainFrame::OnRbnHOLang()
{
	// Get the selected language.
	CMFCRibbonComboBox *pCombo = DYNAMIC_DOWNCAST( CMFCRibbonComboBox, m_wndRibbonBar.FindByID( ID_RBN_H_O_LANG ) );

	if( NULL == pCombo )
	{
		return;
	}

	int nSel = pCombo->GetCurSel();

	if( nSel != CB_ERR )
	{
		// Recuperate the trigram value.
		DWORD_PTR pDWORD = pCombo->GetItemData( nSel );
		LPCTSTR lpctstr = (LPCTSTR)pDWORD;
		CString str = lpctstr;

		// In case the user select the same language, do nothing.
		CString NextLang = TASApp.GetNextLanguage();

		if( 0 == IDcmp( NextLang, str ) )
		{
			return;
		}

		// Save it into a variable that will be save on the registry at the ExitInstance() function.
		TASApp.SetNextLanguage( str );

		// Ask to close the application.
		if( IDYES == TASApp.AfxLocalizeMessageBox( AFXMSG_RESTARTFORLANGSWITCH, MB_YESNO ) )
		{
			if( FALSE == RA_ActivateRestartProcess() )
			{
				// Handle restart error here
				return;
			}

			OnClose();
		}
	}
}

void CMainFrame::OnRbnHODocs()
{
	CDlgDocs dlgDocs;
	dlgDocs.Display();
}

void CMainFrame::OnRbnHOAutoSav()
{
	TASApp.SetAutoSaveflag( !TASApp.IsAutoSave() );
	TASApp.WriteProfileInt( PREFERENCE, AUTOSAVING, TASApp.IsAutoSave() );
}

void CMainFrame::OnRbnHOUserRef()
{
	CDlgUserRef dlg;
	dlg.Display();
}

void CMainFrame::OnRbnHINotes()
{
	TASApp.ShowNewInHtmlPage( false );
}

void CMainFrame::OnRbnHITipOfTheDay()
{
	CDlgTip dlg;
	dlg.DoModal();
}

void CMainFrame::OnRbnHIAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}

void CMainFrame::OnRbnPProjInfo()
{
	CDlgRef dlg;
	dlg.Display();
}

void CMainFrame::OnRbnPPIProjName()
{
	// Variable.
	CDS_ProjectRef *pPrjRef = TASApp.GetpTADS()->GetpProjectRef();
	ASSERT( pPrjRef );

	// Set the string in the Project Reference.
	pPrjRef->SetString( CDS_ProjectRef::Name, (LPCTSTR)m_pEBProjName->GetEditText() );

	// Set a message Project References have changed.
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PROJECTREFCHANGE );
}

void CMainFrame::OnRbnPPICBICompatible()
{
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	CTableHM *pTab = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
	int iHMCount = 0;

	if( NULL != pTab )
	{
		iHMCount = pTab->GetItemCount();
	}

	if( false == pPrjParam->IsCompatibleCBI() && iHMCount > 0 )
	{
		CString str = TASApp.LoadLocalizedString( AFX_MSG_TACBI_COMPATIBILITY );

		if( IDNO == AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION, 0 ) )
		{
			return;
		}
	}

	pPrjParam->SetCompatibleCBI( false == pPrjParam->IsCompatibleCBI() );

	if( true == pPrjParam->IsCompatibleCBI() )
	{
		CTableHM *pTab = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );

		if( NULL != pTab )
		{
			TASApp.GetpTADS()->RenameBranch( pTab );
		}

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, (LPARAM)NULL );
		}
	}
}

void CMainFrame::OnRbnPPIHMCalc()
{
	// Define the button to HMCalc mode or not.
	if( true == IsHMCalcChecked() )
	{
		SetHMCalcMode( false );
	}
	else
	{
		#ifdef USEACTIVATIONCODE
		CDlgRegister DlgReg;
		int nDay = DlgReg.Init();

		if( nDay <= 0 )
		{
			DlgReg.DoModal();
		}
		else if( nDay < ACTIVATIONWARNINGDAYS )
		{
			CString str;
			TCHAR buf[10];
			_stprintf_s( buf, SIZEOFINTCHAR( buf ), L"%d", nDay );
			FormatString( str, IDS_ACTIVATIONCODEWILLEXPIRE, buf );

			if( IDYES == AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION, 0 ) )
			{
				DlgReg.DoModal();
			}
		}

		if( DlgReg.Init() <= 0 )
		{
			SetHMCalcMode( false );
			return;
		}

		#endif

		// Verify that all valves are defined.
		if( true == TASApp.GetpTADS()->IsOneValveNotDefine() )
		{
			CString str = TASApp.LoadLocalizedString( AFXMSG_ONE_VALVE_PARTIALLYDEF );
			AfxMessageBox( (LPCTSTR)str );
			SetHMCalcMode( false );
			return;
		}

		// Pay attention we have now two flags: one in the 'CDS_HydroMod' class and one in the 'CDS_PrjParams'. The first one (the old one)
		// tell us if the project has been converted in hydronic calculation mode. In the old version, the project was converted either
		// intentionally by the user (when clicking on the HM calc button in the ribbon) or when user sent his project to the TAScope.
		// Now, we have a second variable in 'CDS_PrjParams' that tells us if user has intentionally pressed the HM calc button to convert
		// his project from the CBI mode to the hydronic calculation mode.

		// If project is old and has never been converted to hydrocalc mode OR if project is new and it has not been intentionally converted in
		// hydrocalc mode...
		if( TASApp.GetpTADS()->GetpHydroModTable()->GetItemCount() > 0 )
		{
			if( ( true == TASApp.GetpTADS()->IsOldProject() && eb3False == TASApp.GetpTADS()->IsHMCalcMode( NULL, true ) ) 
					|| ( false == TASApp.GetpTADS()->IsOldProject() && false == TASApp.GetpTADS()->GetUserForcedHMCalcModeFlag() ) )
			{
				// Send a warning message that the HydroMod will be converted in a hydronic circuit calculation mode.
				CString str = TASApp.LoadLocalizedString( AFXMSG_CONVERT_IN_HMCALC_MODE );

				if( IDYES != ::AfxMessageBox( (LPCTSTR)str, MB_YESNO | MB_ICONEXCLAMATION ) )
				{
					SetHMCalcMode( false );
					return;
				}
				else
				{
					BeginWaitCursor();
					TASApp.GetpTADS()->CompleteProjForHydroCalcMode( NULL, true );
					TASApp.GetpTADS()->ComputeAllInstallation();
					EndWaitCursor();
				}
			}
			else
			{
				// In case at least one module comes from the HMCalc mode, all other modules will be converted into HMCalc mode!!
				BeginWaitCursor();
				TASApp.GetpTADS()->CompleteProjForHydroCalcMode( NULL, false );
				TASApp.GetpTADS()->ComputeAllInstallation();
				EndWaitCursor();
			}
		}

		SetHMCalcMode( true );

		// Set that user has intentionally forced to go in the hydronic calculation mode.
		TASApp.GetpTADS()->SetUserForcedHMCalcModeFlag( true );
	}

	if( NULL != pDlgLeftTabProject )
	{
		pDlgLeftTabProject->ResetTree();
	}

	// If diversity factor dialog is created, change its internal state.
	if( NULL != m_pDlgDiversityFactor && NULL != m_pDlgDiversityFactor->GetSafeHwnd() )
	{
		m_pDlgDiversityFactor->CheckInternalState();
	}
}

void CMainFrame::OnRbnPPPFreeze()
{
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == pPrjParam->IsFreezed() )
	{
		pPrjParam->SetFreeze( false );
		BeginWaitCursor();

		// Compute only if we are in the hydronic circuit calculation mode.
		if( true == IsHMCalcChecked() )
		{
			TASApp.GetpTADS()->ComputeAllInstallation();
		}

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->ResetTree();
		}

		// Verify if the RViewSelProd is currently active.
		// In that case refresh the view to update values.
		if( NULL != pRViewSelProd && TRUE == pRViewSelProd->IsWindowVisible() )
		{
			pRViewSelProd->RedrawRightView();
		}

		EndWaitCursor();
	}
	else
	{
		pPrjParam->SetFreeze( TRUE );
		BeginWaitCursor();

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->ResetTree();
		}

		EndWaitCursor();
	}

	// If diversity factor dialog is created, change its internal state.
	if( NULL != m_pDlgDiversityFactor && NULL != m_pDlgDiversityFactor->GetSafeHwnd() )
	{
		m_pDlgDiversityFactor->CheckInternalState();
	}
}

void CMainFrame::OnRbnPPPHNAutoCheck()
{
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == pPrjParam->GetHNAutomaticCheck() )
	{
		pPrjParam->SetHNAutomaticCheck( false );
	}
	else
	{
		pPrjParam->SetHNAutomaticCheck( true );
	}
}

void CMainFrame::OnRbnPPPProjectType()
{
	// HYS-1716: Before to apply, we will verify if there are already injection circuits and check if temperatures are OK.

	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERT_RETURN;
	}

	// Save previous value.
	ProjectType ePreviousProjectType = TASApp.GetpTADS()->GetpTechParams()->GetProjectApplicationType();
	
	// We call this method with 'false' parameter to not call the 'OnWaterChange' method.
	// So, we have all supply temperature, delta T and water characteristic variables set. It's easier for us to verify injection circuits with that just after.
	if( true == _OnRbnProjectApplicationTypeChanged( false ) )
	{
		// HYS-1716: Now we can verify if there is at least one injection hydraulic circuit that have its design temperatures in error
		// with this user change.
		std::multimap<CDS_HydroMod *, CTable *> mmapInjectionHydraulicCircuitWithTemperatureError;
		std::vector<CDS_HydroMod *> vecAllInjectionCircuitsWithTemperatureError;
		CTableHM *pclHydraulicNetwork = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
		ASSERT( NULL != pclHydraulicNetwork );

		CDlgInjectionError::ReturnCode eDlgInjectionErrorReturnCode = CDlgInjectionError::ReturnCode::Undefined;

		if( true == pclHydraulicNetwork->IsInjectionCircuitTemperatureErrorForEditOperation( NULL, TASApp.GetpTADS()->GetpTechParams()->GetDefaultPrjTps(),
				&mmapInjectionHydraulicCircuitWithTemperatureError, &vecAllInjectionCircuitsWithTemperatureError, false, true ) )
		{
			// Show the dialog to ask user if he wants to apply this new temperature without changing all the children in errors, or if wants to apply and 
			// automatically correct the errors or if he wants to cancel.
			CDlgInjectionError DlgInjectionError( &vecAllInjectionCircuitsWithTemperatureError );
			eDlgInjectionErrorReturnCode = (CDlgInjectionError::ReturnCode)DlgInjectionError.DoModal();

			if( CDlgInjectionError::ReturnCode::Cancel == eDlgInjectionErrorReturnCode )
			{
				// Restore previous value.
				if( NULL == m_pCBProjectApplicationType )
				{
					ASSERT_RETURN;
				}

				m_pCBProjectApplicationType->SelectItem( (int)ePreviousProjectType );
				_OnRbnProjectApplicationTypeChanged( false );

				return;
			}

			// The correction will be applied after the change of the temperature.
		}

		// HYS-1716: if there are errors and these ones must be corrected (Choice validated by user in the 'CDlgInjectionError' dialog above).
		if( CDlgInjectionError::ReturnCode::ApplyWithCorrection == eDlgInjectionErrorReturnCode )
		{
			pclHydraulicNetwork->CorrectAllInjectionCircuits( &mmapInjectionHydraulicCircuitWithTemperatureError, true );
		}

		// HYS-1716: Now that the application type has been changed, we need to run all chidren injection circuit to update their
		// primary flows.
		// Remark: here we pass NULL as argument because we need to check all the hydraulic network.
		pclHydraulicNetwork->CorrectAllPrimaryFlow();

		OnWaterChange( WMUserWaterCharWParam::WM_UWC_WP_ForProject, WMUserWaterCharLParam::WM_UWC_LWP_Change );
		::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_WATERCHANGE, ( WPARAM )WMUserWaterCharWParam::WM_UWC_WP_ForProject, WMUserWaterCharLParam::WM_UWC_LWP_Change );
	}
}

void CMainFrame::OnRbnPTSR()
{
	CDlgSearchReplace Dlg( CDlgSearchReplace::OpenFromWhere::OFW_RibbonProject );
	Dlg.DoModal();
}

void CMainFrame::OnRbnPTDF()
{
	if( NULL == m_pDlgDiversityFactor )
	{
		m_pDlgDiversityFactor = new CDlgDiversityFactor();

		if( NULL == m_pDlgDiversityFactor )
		{
			return;
		}

		if( 0 == m_pDlgDiversityFactor->Create( IDD_DLGDIVERSITYFACTOR, this ) )
		{
			delete m_pDlgDiversityFactor;
			m_pDlgDiversityFactor = NULL;
			return;
		}
	}

	m_pDlgDiversityFactor->ShowWindow( SW_SHOW );
	m_pDlgDiversityFactor->SetFocus();
}

void CMainFrame::OnRbnPWaterChar()
{
	// Init variables.
	CDS_WaterCharacteristic *pWC = TASApp.GetpTADS()->GetpWCForProject();

	if( NULL == pWC )
	{
		return;
	}

	// Get the temperature value.
	double dValueSI = m_pEBWaterCharTempProj->GetCurrentValSI();

	SetFocus();

	// Verify the temperature input.
	if( true == VerifyEditTemp( pWC, m_pEBWaterCharTempProj, dValueSI, false ) )
	{
		// Set temp to the water characteristics.
		pWC->UpdateFluidData( dValueSI );

		// Display the dialog.
		// Call Dialog water characteristic from the project.
		CDlgWaterChar dlg;
		dlg.Display( NULL, CDlgWaterChar::DlgWaterChar_ForHMCalc );
	}
}

void CMainFrame::OnRbnPSWCDT()
{
	if( NULL == TASApp.GetpTADS()->GetpWCForProductSelection() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERT_RETURN;
	}

	// Get the Temp value.
	double dVal;
	LPCTSTR lpstr = m_pEBWaterCharDTProductSel->GetEditText();
	ReadCUDouble( _U_DIFFTEMP, (CString)lpstr, &dVal );

	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();

	if( dVal == pTech->GetDefaultISDT() )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	CDS_WaterCharacteristic *pWC = TASApp.GetpTADS()->GetpWCForProductSelection();
	CString AdditCharID = ( (CDB_StringID *)( TASApp.GetpTADB()->Get( pWC->GetAdditID() ).MP ) )->GetIDstr();
	CDB_AdditCharacteristic *pAdditChar = (CDB_AdditCharacteristic *)( TASApp.GetpTADB()->Get( AdditCharID ).MP );

	if( NULL == pAdditChar )
	{
		ASSERT_RETURN;
	}

	// HYS-1194 : Do not compare with the freezing or max. temp. point when we are working with any other fluid.
	// In this case these values are unknown. 
	if( 0 != _tcscmp( pWC->GetAdditFamID(), _T( "OTHER_ADDIT" ) ) )
	{
		if( Cooling == pTech->GetProductSelectionApplicationType() )
		{
			double dMaxTemp = pAdditChar->GetMaxT( pWC->GetPcWeight() );

			if( ( pWC->GetTemp() + dVal ) > dMaxTemp )
			{
				CString str;
				FormatString( str, AFXMSG_DTABOVE_TMAX, WriteCUDouble( _U_TEMPERATURE, dMaxTemp, true ) );
				AfxMessageBox( (LPCTSTR)str, MB_OK, -1 );

				CMFCRibbonRichEditCtrlNum *pRichEditCtrl = m_pEBWaterCharDTProductSel->GetNumericalEdit();
				pRichEditCtrl->SetFocus();

				// Restore previous value.
				m_pEBWaterCharDTProductSel->SetCurrentValSI( pTech->GetDefaultISDT() );

				return;
			}
		}
		else
		{
			if( ( pWC->GetTemp() - dVal ) <= pWC->GetTfreez() )
			{
				CString str;
				FormatString( str, AFXMSG_DTBELOW_TFREEZ, WriteCUDouble( _U_TEMPERATURE, pWC->GetTfreez(), true ) );
				AfxMessageBox( (LPCTSTR)str, MB_OK, -1 );

				CMFCRibbonRichEditCtrlNum *pRichEditCtrl = m_pEBWaterCharDTProductSel->GetNumericalEdit();
				pRichEditCtrl->SetFocus();

				// Restore previous value.
				m_pEBWaterCharDTProductSel->SetCurrentValSI( pTech->GetDefaultISDT() );

				return;
			}
		}
	}
	pTech->SetDefaultISDT( dVal );

	// Send message to inform about modification of water characteristics.
	// HYS-1473 : When the change is made via the ribbon we have to set LPARAM as changed.
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_WATERCHANGE, ( WPARAM )WMUserWaterCharWParam::WM_UWC_WP_ForProductSel, 
		WMUserWaterCharLParam::WM_UWC_LWP_Change );
	
	// Do not call "OnWaterChange" in this case.
}

void CMainFrame::OnRbnPWCDT()
{
	if( NULL == TASApp.GetpTADS()->GetpWCForProject() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERT_RETURN;
	}

	// Get the Temp value.
	double dDT;
	LPCTSTR lpstr = m_pEBWaterCharDTProj->GetEditText();
	ReadCUDouble( _U_DIFFTEMP, (CString)lpstr, &dDT );

	CDS_TechnicalParameter *pTech =  TASApp.GetpTADS()->GetpTechParams();

	if( dDT == pTech->GetDefaultPrjDT() )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	CDS_WaterCharacteristic *pWC = TASApp.GetpTADS()->GetpWCForProject();
	CString AdditCharID = ( (CDB_StringID *)( TASApp.GetpTADB()->Get( pWC->GetAdditID() ).MP ) )->GetIDstr();
	CDB_AdditCharacteristic *pAdditChar = (CDB_AdditCharacteristic *)( TASApp.GetpTADB()->Get( AdditCharID ).MP );

	if( NULL == pAdditChar )
	{
		ASSERT_RETURN;
	}

	// HYS-1194 : Do not compare with the freezing or max. temp. point when we are working with any other fluid.
	// In this case these values are unknown. 
	if( 0 != _tcscmp( pWC->GetAdditFamID(), _T( "OTHER_ADDIT" ) ) )
	{
		if( Cooling == pTech->GetProjectApplicationType() )
		{
			double dMaxTemp = pAdditChar->GetMaxT( pWC->GetPcWeight() );

			if( ( pWC->GetTemp() + dDT ) > dMaxTemp )
			{
				CString str;
				FormatString( str, AFXMSG_DTABOVE_TMAX, WriteCUDouble( _U_TEMPERATURE, dMaxTemp, true ) );
				AfxMessageBox( (LPCTSTR)str, MB_OK, -1 );

				CMFCRibbonRichEditCtrlNum *pRichEditCtrl = m_pEBWaterCharDTProj->GetNumericalEdit();
				pRichEditCtrl->SetFocus();

				// Restore previous value.
				m_pEBWaterCharDTProj->SetCurrentValSI( pTech->GetDefaultPrjDT() );

				return;
			}
		}
		else
		{
			if( ( pWC->GetTemp() - dDT ) <= pWC->GetTfreez() )
			{
				CString str;
				FormatString( str, AFXMSG_DTBELOW_TFREEZ, WriteCUDouble( _U_TEMPERATURE, pWC->GetTfreez(), true ) );
				AfxMessageBox( (LPCTSTR)str, MB_OK, -1 );

				CMFCRibbonRichEditCtrlNum *pRichEditCtrl = m_pEBWaterCharDTProj->GetNumericalEdit();
				pRichEditCtrl->SetFocus();

				// Restore previous value.
				m_pEBWaterCharDTProj->SetCurrentValSI( pTech->GetDefaultPrjDT() );

				return;
			}
		}
	}

	pTech->SetDefaultPrjDT( dDT );

	// Send message to inform about modification of water characteristics.
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_WATERCHANGE, ( WPARAM )WMUserWaterCharWParam::WM_UWC_WP_ForProject, 
		WMUserWaterCharLParam::WM_UWC_LWP_Change );
	OnWaterChange( WMUserWaterCharWParam::WM_UWC_WP_ForProject, WMUserWaterCharLParam::WM_UWC_LWP_Change );
}

void CMainFrame::OnRbnPWCTemp()
{
	if( NULL == TASApp.GetpTADS()->GetpWCForProject() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERT_RETURN;
	}

	// Init variables.
	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();
	CDS_WaterCharacteristic *pWC = TASApp.GetpTADS()->GetpWCForProject();

	// Get the temperature value.
	double dSupplyTemperature = m_pEBWaterCharTempProj->GetCurrentValSI();

	if( dSupplyTemperature == pTech->GetDefaultPrjTps() )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	// Verify first the validity of the temperature input.
	if( false == VerifyEditTemp( pWC, m_pEBWaterCharTempProj, dSupplyTemperature ) )
	{
		// Restore previous value.
		m_pEBWaterCharTempProj->SetCurrentValSI( pTech->GetDefaultPrjTps() );
		
		return;
	}

	// Now verify with DT.
	double dDeltaT = m_pEBWaterCharDTProj->GetCurrentValSI();

	CString AdditCharID = ( (CDB_StringID *)( TASApp.GetpTADB()->Get( pWC->GetAdditID() ).MP ) )->GetIDstr();
	CDB_AdditCharacteristic *pAdditChar = (CDB_AdditCharacteristic *)( TASApp.GetpTADB()->Get( AdditCharID ).MP );

	if( NULL == pAdditChar )
	{
		ASSERT_RETURN;
	}

	// HYS-1194 : Do not compare with the freezing or max. temp. point when we are working with any other fluid.
	// In this case these values are unknown. 
	if( 0 != _tcscmp( pWC->GetAdditFamID(), _T( "OTHER_ADDIT" ) ) )
	{
		if( Cooling == pTech->GetProjectApplicationType() )
		{
			double dMaxTemp = pAdditChar->GetMaxT( pWC->GetPcWeight() );

			if( ( dSupplyTemperature + dDeltaT ) > dMaxTemp )
			{
				CString str;
				FormatString( str, AFXMSG_TPLUSDTABOVE_TMAX, WriteCUDouble( _U_TEMPERATURE, dMaxTemp, true ) );
				AfxMessageBox( (LPCTSTR)str, MB_OK, -1 );

				// Restore previous value.
				m_pEBWaterCharTempProj->SetCurrentValSI( pTech->GetDefaultPrjTps() );

				CMFCRibbonRichEditCtrlNum *pRichEditCtrl = m_pEBWaterCharTempProj->GetNumericalEdit();
				pRichEditCtrl->SetFocus();

				return;
			}
		}
		else
		{
			if( ( dSupplyTemperature - dDeltaT ) <= pWC->GetTfreez() )
			{
				CString str;
				FormatString( str, AFXMSG_TMINUSDTBELOW_TFREEZ, WriteCUDouble( _U_TEMPERATURE, pWC->GetTfreez(), true ) );
				AfxMessageBox( (LPCTSTR)str, MB_OK, -1 );

				// Restore previous value.
				m_pEBWaterCharTempProj->SetCurrentValSI( pTech->GetDefaultPrjTps() );

				CMFCRibbonRichEditCtrlNum *pRichEditCtrl = m_pEBWaterCharTempProj->GetNumericalEdit();
				pRichEditCtrl->SetFocus();

				return;
			}
		}
	}

	// HYS-1716: Now we can verify if there is at least one injection hydraulic circuit that have its design temperatures in error
	// with this user change.
	std::multimap<CDS_HydroMod *, CTable *> mmapInjectionHydraulicCircuitWithTemperatureError;
	std::vector<CDS_HydroMod *> vecAllInjectionCircuitsWithTemperatureError;
	CTableHM *pclHydraulicNetwork = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
	ASSERT( NULL != pclHydraulicNetwork );

	CDlgInjectionError::ReturnCode eDlgInjectionErrorReturnCode = CDlgInjectionError::ReturnCode::Undefined;

	if( true == pclHydraulicNetwork->IsInjectionCircuitTemperatureErrorForEditOperation( NULL, dSupplyTemperature,
			&mmapInjectionHydraulicCircuitWithTemperatureError, &vecAllInjectionCircuitsWithTemperatureError ) )
	{
		// Show the dialog to ask user if he wants to apply this new temperature without changing all the children in errors, or if wants to apply and 
		// automatically correct the errors or if he wants to cancel.
		CDlgInjectionError DlgInjectionError( &vecAllInjectionCircuitsWithTemperatureError );
		eDlgInjectionErrorReturnCode = (CDlgInjectionError::ReturnCode)DlgInjectionError.DoModal();

		if( CDlgInjectionError::ReturnCode::Cancel == eDlgInjectionErrorReturnCode )
		{
			// Restore previous value.
			m_pEBWaterCharTempProj->SetCurrentValSI( pTech->GetDefaultPrjTps() );

			CMFCRibbonRichEditCtrlNum *pRichEditCtrl = m_pEBWaterCharTempProj->GetNumericalEdit();
			pRichEditCtrl->SetFocus();

			return;
		}

		// The correction will be applied after the change of the temperature.
	}
	
	// Save the default temperature.
	pTech->SetDefaultPrjTps( dSupplyTemperature );
		
	// Set temp to the water characteristics.
	pWC->UpdateFluidData( dSupplyTemperature );

	// HYS-1716: if there are errors and these ones must be corrected (Choice validated by user in the 'CDlgInjectionError' dialog above).
	if( CDlgInjectionError::ReturnCode::ApplyWithCorrection == eDlgInjectionErrorReturnCode )
	{
		pclHydraulicNetwork->CorrectAllInjectionCircuits( &mmapInjectionHydraulicCircuitWithTemperatureError );
	}

	// Now we must pass all the concerned injection circuits to force a recomputation of the primary flow.
	// Remark: the compute all installation will be done in the 'CDlgLeftTabProject' when receiving the 'WM_USER_WATERCHANGE' message just sent after.
	// Remark: here we don't pass argument because we want that all flows for the installation be recomputed.
	pclHydraulicNetwork->CorrectAllPrimaryFlow();

	// Send message to inform about modification of water characteristics.
	// HYS-1473 : When the change is made via the ribbon we have to set LPARAM as changed.
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_WATERCHANGE, ( WPARAM )WMUserWaterCharWParam::WM_UWC_WP_ForProject, 
		WMUserWaterCharLParam::WM_UWC_LWP_Change );

	// HYS-1275: Update temperature in the Rview
	if( NULL != pDlgLeftTabProject )
	{
		pDlgLeftTabProject->ResetTree();
	}
}

void CMainFrame::OnRbnPMPDateTime()
{
	CDlgRibbonDateTime dlg;
	int nRet = dlg.DoModal();

	// Set the new values from the DateTimePicker Dialog box.
	if( IDOK == nRet )
	{
		_SetNewDateTimeValues();

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->RefreshRightView();
		}
	}
}

void CMainFrame::OnRbnPDebugVerityAutority()
{
	m_bHMCalcDebugVerifyAutority = !m_bHMCalcDebugVerifyAutority;

	BeginWaitCursor();

	// Compute only if we are in the hydronic circuit calculation mode.
	if( true == IsHMCalcChecked() )
	{
		TASApp.GetpTADS()->ComputeAllInstallation();
	}

	if( NULL != pDlgLeftTabProject )
	{
		pDlgLeftTabProject->ResetTree();
	}

	// Verify if the RViewSelProd is currently active.
	// In that case refresh the view to update values.
	if( NULL != pRViewSelProd && TRUE == pRViewSelProd->IsWindowVisible() )
	{
		pRViewSelProd->RedrawRightView();
	}

	EndWaitCursor();
}

void CMainFrame::OnRbnTHViscCor()
{
	CDlgViscCorr dlg;
	dlg.Display();
}

void CMainFrame::OnRbnTHPipePressDrop()
{
	CDlgPipeDp dlg;
	dlg.Display();
	dlg.DoModal();
}

void CMainFrame::OnRbnTHKvCalc()
{
	CDlgKvSeries dlg;
	dlg.Display();
}

void CMainFrame::OnRbnTHCalculator()
{
	CDlgHydroCalc dlg;
	dlg.DoModal();
}

void CMainFrame::OnRbnTOUnitConv()
{
	CDlgUnitConv dlg;
	dlg.Display();
}

void CMainFrame::OnRbnTOTALink()
{
	CDlgTALink dlg;
	dlg.DoModal();
}

void CMainFrame::OnRbnCDTScopeComm()
{
	// Send message to notify the display of the project tab and the selection of the root item to avoid refreshing problems.
	if( NULL != pDlgLTtabctrl )
	{
		pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiProj );
	}

	if( NULL != pDlgLeftTabProject )
	{
		pDlgLeftTabProject->SelectRootItem();
	}

	CDlgWizTAScope dlgWiz;
	dlgWiz.DisplayWizard();

	BeginWaitCursor();

	if( NULL != pDlgLeftTabProject )
	{
		pDlgLeftTabProject->ResetTree();
	}

	EndWaitCursor();
}

void CMainFrame::OnRbnCDTCbiComm()
{
	if( NULL != pDlgLeftTabProject )
	{
		pDlgLeftTabProject->OnButtoncbi();
	}
}

void CMainFrame::OnRbnCDTDpSVisioComm()
{
	CDlgDpSVisio clDlgDpsVisio( this );
	clDlgDpsVisio.DoModal();
}

void CMainFrame::OnRbnCUProdData()
{
	if( true == TASApp.GetHyUpdateInstDir().IsEmpty() && false == TASApp.IsStartingWOInstal() )
	{
		return;
	}

	CString HyUpdate;
	HyUpdate = TASApp.GetHyUpdateInstDir() + L"HyUpdate.exe";
	CString strParam;

	if( true == TASApp.IsAutoUpdate() )
	{
		strParam += CString( L"*Auto" );
		TASApp.SetAutoUpdateflag( false );
		TASApp.StartProcess( HyUpdate, 2, ::AfxGetAppName(), L"*Auto" );
	}

	TASApp.StartProcess( HyUpdate, 1, ::AfxGetAppName() );
}

void CMainFrame::OnRbnCUProperties()
{
	CDlgGateway	dlg;
	dlg.Display();
}

void CMainFrame::OnRbnCUMaintainTAScope()
{
	if( true == TASApp.IsTAScopeFilesMaintain() )
	{
		TASApp.SetTAScopeFilesMaintain( 0 );
	}
	else
	{
		TASApp.SetTAScopeFilesMaintain( 1 );
	}

	TASApp.WriteProfileInt( L"TA-SCOPE", L"Maintain files", TASApp.IsTAScopeFilesMaintain() );
}

void CMainFrame::OnRbnCUTestHySelectUpdate()
{
	if( false == TASApp.GetpTADB()->IsDebugPsw() )
	{
		return;
	}

	CString HyUpdate = TASApp.GetHyUpdateInstDir() + L"HyUpdate.exe";
	TASApp.StartProcess( HyUpdate, 2, ::AfxGetAppName(), L"*Test" );
}

void CMainFrame::OnRbnPSPTGallery()
{
	// Get the number to the selected image.
	int iSelectedItem = m_pGallery->GetSelectedItem();

	// Get the LPARAM of the corresponding item.
	m_eCurrentProductSubCategory = (ProductSubCategory)m_pGallery->GetLparamFromImageIndex( iSelectedItem );

	// Show the correct 'CDlgIndSelxxx' dialog in the left tab.
	RightViewList eRightViewToSelect = RightViewList::eUndefined;

	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();

	if( NULL != pDlgLeftTabSelManager )
	{
		BYTE bModified = TASApp.GetpTADS()->IsModified();
		
		// If we left changeOver application type, delete it from the combo application type.
		if( ChangeOver == pTechP->GetProductSelectionApplicationType() )
		{
			int iPos = m_pCBProductSelectionApplicationType->GetItemIndex( (LPCTSTR)TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( ChangeOver ) ) );
			
			if( -1 != iPos )
			{
				m_pCBProductSelectionApplicationType->DeleteItem( iPos );
			}
			
			pMainFrame->ChangeRbnProductSelectionApplicationType( ProjectType::Heating );
		}
		
		TASApp.GetpTADS()->Modified( bModified );
		
		// Update left tab with the new selected.
		eRightViewToSelect = pDlgLeftTabSelManager->UpdateLeftTabDialog( m_eCurrentProductSubCategory, m_eCurrentProductSelectionMode );

		// If the selected product is 6way control valve.
		bModified = TASApp.GetpTADS()->IsModified();
		
		if( ProductSubCategory::PSC_BC_6WayControlValve == m_eCurrentProductSubCategory )
		{
			// Add changeOver application type.
			int iPos = m_pCBProductSelectionApplicationType->GetItemIndex( (LPCTSTR)TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( ChangeOver ) ) );
			
			if( -1 == iPos )
			{
				m_pCBProductSelectionApplicationType->AddItem( (LPCTSTR)TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( ChangeOver ) ), ChangeOver );
				m_pCBProductSelectionApplicationType->AddItemEx( (LPCTSTR)TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( ChangeOver ) ),
					(LPCTSTR)TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( ChangeOver ) ) );
				m_pPanelApplicationParameters->SetJustifyColumns();
			}
			
			pMainFrame->ChangeRbnProductSelectionApplicationType( ProjectType::ChangeOver );
		}
		
		TASApp.GetpTADS()->Modified( bModified );
	}

	// Active the correct right view.
	ActiveFormView( eRightViewToSelect );
}

void CMainFrame::OnRbnPSPPipeSeries()
{
	if( NULL == m_pRibbonComboSelPipeSeries )
	{
		return;
	}

	int nSel = m_pRibbonComboSelPipeSeries->GetCurSel();

	if( nSel != CB_ERR )
	{
		FillProductSelPipeSizeCB();
		AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PIPECHANGE, ( WPARAM )WMUserPipeChange::WM_UPC_ForProductSelection );
	}
}

void CMainFrame::OnRbnPSPPipeSize()
{
	AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PIPECHANGE, ( WPARAM )WMUserPipeChange::WM_UPC_ForProductSelection );
}

void CMainFrame::OnRbnPSTPDSSizeAbv()
{
	_RbnTPSizeAbv( true );
}

void CMainFrame::OnRbnPSTPDSSizeBLW()
{
	_RbnTPSizeBLW( true );
}

void CMainFrame::OnRbnPSTPPDTarget()
{
	_RbnTPPDTarget( true );
}

void CMainFrame::OnRbnPSTPPDMax()
{
	_RbnTPPDMax( true );
}

void CMainFrame::OnRbnPSTPWVTarget()
{
	_RbnTPWVTarget( true );
}

void CMainFrame::OnRbnPSTPWVMax()
{
	_RbnTPWVMax( true );
}

void CMainFrame::OnRbnPSPMWQPrefsCombined()
{
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL == pclPMInputUser || NULL == pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		return;
	}

	bool fNewValue = !pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice );
	pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::GeneralCombinedInOneDevice, fNewValue );

	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PMWQSELECTIONPREFSCHANGE, ( WPARAM )ChangeFromRibbonOneDevice, (LPARAM)fNewValue );
}

void CMainFrame::OnRbnPSPMWQPrefsRedPumpComp()
{
	// PAY ATTENTION: 2015-08-12: the combo 'm_pPMWQSelecPrefsRedundancy' contained in first position (0-indexed) the 'PM_REDDCY_NONE' value,
	//                in second 'PM_REDDCY_PUMPCOMP', third 'PM_REDDCY_TECBOX' and finally 'PM_REDDCY_ALL'.
	//                If we make any change in this combo 'OnRbnPSPMWQPrefsRedPumpComp' and 'OnRbnPSPMWQPrefsRedTecBox' must be of course
	//                updated.
	//                TODO: a best way to make a link between this two methods.

	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL == pclPMInputUser || NULL == pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		return;
	}

	bool bRedundancyPumpCompr = !pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyPumpComp );
	pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::GeneralRedundancyPumpComp, bRedundancyPumpCompr );

	// _UpdateRibbonPMWQComboRedundancy();

	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PMWQSELECTIONPREFSCHANGE, ( WPARAM )ChangeFromRibbonRedPumpComp, bRedundancyPumpCompr );
}

void CMainFrame::OnRbnPSPMWQPrefsRedTecBox()
{
	// PAY ATTENTION: 2015-08-12: the combo 'm_pPMWQSelecPrefsRedundancy' contained in first position (0-indexed) the 'PM_REDDCY_NONE' value,
	//                in second 'PM_REDDCY_PUMPCOMP', third 'PM_REDDCY_TECBOX' and finally 'PM_REDDCY_ALL'.
	//                If we make any change in this combo 'OnRbnPSPMWQPrefsRedPumpComp' and 'OnRbnPSPMWQPrefsRedTecBox' must be of course
	//                updated.
	//                TODO: a best way to make a link between this two methods.

	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL == pclPMInputUser || NULL == pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		return;
	}

	bool bRedundancyTecBox = !pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox );
	pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::GeneralRedundancyTecBox, bRedundancyTecBox );

	// _UpdateRibbonPMWQComboRedundancy();

	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PMWQSELECTIONPREFSCHANGE, ( WPARAM )ChangeFromRibbonRedTecBox, bRedundancyTecBox );
}

void CMainFrame::OnRbnPSPMWQPrefsIntCoating()
{
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL == pclPMInputUser || NULL == pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		return;
	}

	bool fNewValue = !pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::CompressoInternalCoating );
	pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::CompressoInternalCoating, fNewValue );

	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PMWQSELECTIONPREFSCHANGE, ( WPARAM )ChangeFromRibbonIntCoating, (LPARAM)fNewValue );
}

void CMainFrame::OnRbnPSPMWQPrefsExtAir()
{
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL == pclPMInputUser || NULL == pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		return;
	}

	bool fNewValue = !pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::CompressoExternalAir );
	pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::CompressoExternalAir, fNewValue );

	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PMWQSELECTIONPREFSCHANGE, ( WPARAM )ChangeFromRibbonExternalAir, (LPARAM)fNewValue );
}

void CMainFrame::OnRbnPSPMWQPrefsInsVapSealed()
{
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL == pclPMInputUser || NULL == pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		return;
	}

	bool fNewValue = !pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::PumpDegassingCoolingVersion );
	pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::PumpDegassingCoolingVersion, fNewValue );

	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PMWQSELECTIONPREFSCHANGE, ( WPARAM )ChangeFromRibbonInsVaporSealed, (LPARAM)fNewValue );
}

void CMainFrame::OnRbnPSPMWQPrefsBreakTank()
{
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL == pclPMInputUser || NULL == pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		return;
	}

	bool fNewValue = !pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::WaterMakeupBreakTank );
	pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::WaterMakeupBreakTank, fNewValue );

	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PMWQSELECTIONPREFSCHANGE, ( WPARAM )ChangeFromRibbonBreakTank, (LPARAM)fNewValue );
}

void CMainFrame::OnRbnPSPMWQPrefsDtyStdBy()
{
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL == pclPMInputUser || NULL == pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		return;
	}

	bool fNewValue = !pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::WaterMakeupDutyStandBy );
	pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::WaterMakeupDutyStandBy, fNewValue );

	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PMWQSELECTIONPREFSCHANGE, ( WPARAM )ChangeFromRibbonDutyStdBy, (LPARAM)fNewValue );
}

void CMainFrame::OnRbnPSAppplicationType()
{
	_OnRbnProductSelectionApplicationTypeChanged();
}

void CMainFrame::OnRbnPSWaterChar()
{
	// Init variables.
	CDS_WaterCharacteristic *pWC = TASApp.GetpTADS()->GetpWCForProductSelection();

	if( NULL == pWC )
	{
		return;
	}

	// Get the temperature value.
	double dValueSI = 0.0;
	
	if( NULL != m_pEBWaterCharTempProductSel )
	{
		dValueSI = m_pEBWaterCharTempProductSel->GetCurrentValSI();
	}

	SetFocus();

	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();

	// Verify the temperature input.
	if( ChangeOver == pTechP->GetProductSelectionApplicationType() )
	{
		CDlgCOWaterChar::INotificationHandler *pclDlgWaterCharNotification = NULL;

		if( pMainFrame->GetRibbonBar()->GetActiveCategory()->GetContextID() == ID_PRODUCTSEL_CATEGORY_BASE )
		{
			pclDlgWaterCharNotification = pDlgLeftTabSelManager->GetCurrentLeftTabDialog();
		}
		// Display the dialog.
		CDlgCOWaterChar dlg( NULL, pclDlgWaterCharNotification );
		dlg.Display();
	}
	else if( true == VerifyEditTemp( pWC, m_pEBWaterCharTempProductSel, dValueSI, false ) )
	{
		// Set temp to the water characteristics.
		pWC->UpdateFluidData( dValueSI );

		CDlgWaterChar::INotificationHandler *pclDlgWaterCharNotification = NULL;

		if( pMainFrame->GetRibbonBar()->GetActiveCategory()->GetContextID() == ID_PRODUCTSEL_CATEGORY_BASE )
		{
			pclDlgWaterCharNotification = pDlgLeftTabSelManager->GetCurrentLeftTabDialog();
		}

		// Display the dialog.
		CDlgWaterChar dlg( pclDlgWaterCharNotification );
		dlg.Display();
	}
}

void CMainFrame::OnRbnPSWCTemp()
{
	if( NULL == TASApp.GetpTADS()->GetpTechParams() || NULL == TASApp.GetpTADS()->GetpWCForProductSelection() 
			|| NULL == TASApp.GetpTADS()->GetpHydroModTable() )
	{
		ASSERT_RETURN;
	}

	// Init variables.
	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();
	CDS_WaterCharacteristic *pWC = TASApp.GetpTADS()->GetpWCForProductSelection();

	// Get the temperature value.
	double dSupplyTemperature = m_pEBWaterCharTempProductSel->GetCurrentValSI();

	if( dSupplyTemperature == pTech->GetDefaultISTps() )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	// First verify the validity of the temperature input.
	if( false == VerifyEditTemp( pWC, m_pEBWaterCharTempProductSel, dSupplyTemperature ) )
	{
		// Restore previous value.
		m_pEBWaterCharTempProductSel->SetCurrentValSI( pTech->GetDefaultISTps() );

		return;
	}

	// Now verify with DT.
	double dDeltaT = m_pEBWaterCharDTProductSel->GetCurrentValSI();
	CString AdditCharID = ( (CDB_StringID *)( TASApp.GetpTADB()->Get( pWC->GetAdditID() ).MP ) )->GetIDstr();
	CDB_AdditCharacteristic *pAdditChar = (CDB_AdditCharacteristic *)( TASApp.GetpTADB()->Get( AdditCharID ).MP );

	if( NULL == pAdditChar )
	{
		ASSERT_RETURN;
	}

	// HYS-1194 : Do not compare with the freezing or max. temp. point when we are working with any other fluid.
	// In this case these values are unknown. 
	if( 0 != _tcscmp( pWC->GetAdditFamID(), _T( "OTHER_ADDIT" ) ) )
	{
		if( Cooling == pTech->GetProductSelectionApplicationType() )
		{
			double dMaxTemp = pAdditChar->GetMaxT( pWC->GetPcWeight() );

			if( ( dSupplyTemperature + dDeltaT ) > dMaxTemp )
			{
				CString str;
				FormatString( str, AFXMSG_TPLUSDTABOVE_TMAX, WriteCUDouble( _U_TEMPERATURE, dMaxTemp, true ) );
				AfxMessageBox( (LPCTSTR)str, MB_OK, -1 );

				CMFCRibbonRichEditCtrlNum *pRichEditCtrl = m_pEBWaterCharTempProductSel->GetNumericalEdit();
				pRichEditCtrl->SetFocus();

				// Restore previous value.
				m_pEBWaterCharTempProductSel->SetCurrentValSI( pTech->GetDefaultISTps() );

				return;
			}
		}
		else
		{
			if( ( dSupplyTemperature - dDeltaT ) <= pWC->GetTfreez() )
			{
				CString str;
				FormatString( str, AFXMSG_TMINUSDTBELOW_TFREEZ, WriteCUDouble( _U_TEMPERATURE, pWC->GetTfreez(), true ) );
				AfxMessageBox( (LPCTSTR)str, MB_OK, -1 );

				CMFCRibbonRichEditCtrlNum *pRichEditCtrl = m_pEBWaterCharTempProductSel->GetNumericalEdit();
				pRichEditCtrl->SetFocus();

				// Restore previous value.
				m_pEBWaterCharTempProductSel->SetCurrentValSI( pTech->GetDefaultISTps() );

				return;
			}
		}
	}
	
	// Save the default temperature.
	pTech->SetDefaultISTps( dSupplyTemperature );

	// Set temp to the water characteristics.
	pWC->UpdateFluidData( dSupplyTemperature );

	// Send message to inform about modification of water characteristics.
	// HYS-1473 : When the change is made via the ribbon we have to set LPARAM as changed.
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_WATERCHANGE, ( WPARAM )WMUserWaterCharWParam::WM_UWC_WP_ForProductSel, 
		WMUserWaterCharLParam::WM_UWC_LWP_Change );

	// Do not call "OnWaterChange" in this case.
}

void CMainFrame::OnRbnPSToolsSR()
{
	CDlgSearchReplace Dlg( CDlgSearchReplace::OpenFromWhere::OFW_RibbonProductSel );
	Dlg.DoModal();
}

void CMainFrame::OnRbnPSSelectionModeDirect()
{
	CDlgDirSel dlg;
	dlg.DoModal();
}

void CMainFrame::OnRbnPSSelectionModeIndividual()
{
	// Send message to inform about change of the button state.
	m_eCurrentProductSelectionMode = ProductSelectionMode_Individual;

	// Show the correct 'CDlgIndSelxxx' dialog in the left tab.
	RightViewList eRightViewToSelect = RightViewList::eUndefined;

	if( NULL != pDlgLeftTabSelManager )
	{
		eRightViewToSelect = pDlgLeftTabSelManager->UpdateLeftTabDialog( m_eCurrentProductSubCategory, m_eCurrentProductSelectionMode );
	}

	// Active the correct right view.
	ActiveFormView( eRightViewToSelect );

	_UpdateProductSubCategoryGalleryBox();
}

void CMainFrame::OnRbnPSSelectionModeBatch()
{
	// Send message to inform about change of the button state.
	m_eCurrentProductSelectionMode = ProductSelectionMode_Batch;

	// Show the correct 'CDlgIndSelxxx' dialog in the left tab.
	RightViewList eRightViewToSelect = RightViewList::eUndefined;

	if( NULL != pDlgLeftTabSelManager )
	{
		eRightViewToSelect = pDlgLeftTabSelManager->UpdateLeftTabDialog( m_eCurrentProductSubCategory, m_eCurrentProductSelectionMode );
	}

	// Active the correct right view.
	ActiveFormView( eRightViewToSelect );

	_UpdateProductSubCategoryGalleryBox();
}

void CMainFrame::OnRbnPSSelectionModeWizard()
{
	// Send message to inform about change of the button state.
	m_eCurrentProductSelectionMode = ProductSelectionMode_Wizard;

	// Show the correct 'CDlgIndSelxxx' dialog in the left tab.
	RightViewList eRightViewToSelect = RightViewList::eUndefined;

	if( NULL != pDlgLeftTabSelManager )
	{
		eRightViewToSelect = pDlgLeftTabSelManager->UpdateLeftTabDialog( m_eCurrentProductSubCategory, m_eCurrentProductSelectionMode );
	}

	// Active the correct right view.
	ActiveFormView( eRightViewToSelect );

	_UpdateProductSubCategoryGalleryBox();
}

void CMainFrame::OnRbnPSPMWQOneDevice()
{
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL == pclPMInputUser || NULL == pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		return;
	}

	pclPMInputUser->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::GeneralCombinedInOneDevice, !pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice ) );

	// Send message to inform about change of the button state.
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PMWQSELECTIONPREFSCHANGE, ( WPARAM )ChangeFromRibbonOneDevice,
			(LPARAM)pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice ) );
}

void CMainFrame::OnRbnPSPMWQComboRedundancy()
{
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL == pclPMInputUser || NULL == pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		return;
	}

	// Send message to inform about change of the button state.
	LPARAM lParam = 0;

	if( NULL != m_pPMWQSelecPrefsRedundancy )
	{
		lParam = m_pPMWQSelecPrefsRedundancy->GetItemData( m_pPMWQSelecPrefsRedundancy->GetCurSel() );
	}

	CData *pclData = dynamic_cast<CData *>( (CData *)lParam );
	ASSERT( NULL != pclData );

	if( NULL != pclData )
	{
		pclPMInputUser->GetPMWQSelectionPreferences()->SetRedundancy( (wchar_t*)&pclData->GetIDPtr().ID[0] );
	}

	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PMWQSELECTIONPREFSCHANGE, ( WPARAM )ChangeFromRibbonRedundancy, lParam );
}

void CMainFrame::OnRbnPSPMWQSelectPrefs()
{
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL == pclPMInputUser || NULL == pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		return;
	}

	CDlgPMWQSelectionPrefs dlg( pclPMInputUser->GetPMWQSelectionPreferences(), pclPMInputUser->GetpTADB() );
	dlg.DoModal();

	// For a combo CCmdUI is not enough to be able to update it. We must do it here.
	CString strComboID = pclPMInputUser->GetPMWQSelectionPreferences()->GetRedundancy();
	CMFCRibbonComboBox *pCombo = DYNAMIC_DOWNCAST( CMFCRibbonComboBox, m_wndRibbonBar.FindByID( ID_RBN_PS_PMWQ_COMBOREDDCY ) );

	if( NULL != pCombo )
	{
		CDB_StringID *pclStringID = dynamic_cast<CDB_StringID *>( (CData *)( TASApp.GetpTADB()->Get( strComboID ).MP ) );

		if( NULL != pclStringID )
		{
			int iSel = pCombo->FindItem( pclStringID->GetString() );

			if( -1 == iSel )
			{
				iSel = 0;
			}

			pCombo->SelectItem( iSel );
		}
	}

	if( true == dlg.IsChangesOperate() )
	{
		::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PMWQSELECTIONPREFSCHANGE, ( WPARAM )ChangeFromDialog );
	}
}

void CMainFrame::OnRbnPSPMWQNbreMaxOfAdditionalVessels()
{
	// Set the new value into the Technical parameters and the registry
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
	int iNbreOfVssl = (int)m_pEBMaxNbrOfAdditionalVesselsInParallelSSel->GetCurrentValSI();
	pTechP->SetMaxNumberOfAdditionalVesselsInParallel( iNbreOfVssl );

	::AfxGetApp()->WriteProfileInt( REGISTRYSECTIONNAME_DLGTECHPARAM, L"MaxNumberOfVesselsInParallel", iNbreOfVssl );

	// Set a message Tech params have changed
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_TECHPARCHANGE );
}

void CMainFrame::OnRbnPSPMWQNbreMaxOfVento()
{
	// Set the new value into the Technical parameters and the registry
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
	int iNbreOfVento = (int)m_pEBMaxNbrOfVentoSSel->GetCurrentValSI();
	pTechP->SetMaxNumberOfVentoInParallel( iNbreOfVento );

	::AfxGetApp()->WriteProfileInt( REGISTRYSECTIONNAME_DLGTECHPARAM, L"MaxNumberOfVentoInParallel", iNbreOfVento );

	// Set a message Tech params have changed
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_TECHPARCHANGE );
}

void CMainFrame::OnRbnUsermanual()
{
	SHELLEXECUTEINFO ShExecInfo;

	// Find the User Manual filename.
	CString Filename = _GetManualFileName();
	BOOL fFind = ( FALSE == Filename.IsEmpty() );

	if( TRUE == fFind )
	{
		//CString Filename = ( (CDB_MultiString*)TASApp.GetpTADB()->Get( L"HELPFILE_PARAM" ).MP)->GetString( 0 );

		// Fill-in the SHELLEXECUTEINFO structure.
		ShExecInfo.cbSize = sizeof( SHELLEXECUTEINFO );
		ShExecInfo.fMask = NULL;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = L"open";
		ShExecInfo.lpFile = Filename;
		ShExecInfo.lpParameters = NULL;
		ShExecInfo.lpDirectory = NULL;
		ShExecInfo.nShow = SW_SHOWNORMAL;
		ShExecInfo.hInstApp = NULL;

		ShellExecuteEx( &ShExecInfo );

		if( ERROR_NO_ASSOCIATION == GetLastError() )
		{
			TASApp.AfxLocalizeMessageBox( AFXMSG_NOACROBATREADER );
		}
		else if( ERROR_FILE_NOT_FOUND == GetLastError() )
		{
			CString str;
			FormatString( str, AFXMSG_NOUSERMANUAL, Filename );
			::AfxMessageBox( str );
		}
	}
}

void CMainFrame::OnUpdateFilePrint( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( TRUE );
}

void CMainFrame::OnUpdateFileSubPrint( CCmdUI *pCmdUI )
{
	if( true == TASApp.GetpTADS()->IsSomethingToPrint() )
	{
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnUpdateExport( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( TRUE );
}

void CMainFrame::OnUpdateFileExportSelp( CCmdUI *pCmdUI )
{
	if( true == TASApp.GetpTADS()->IsSomethingToPrint() )
	{
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnUpdateFileExportChart( CCmdUI *pCmdUI )
{
	if( NULL != pDlgLeftTabProject )
	{
		if( CMainFrame::RightViewList::eRVChart == pDlgLeftTabProject->GetCurrentView() )
		{
			pCmdUI->Enable( TRUE );
			return;
		}
	}

	pCmdUI->Enable( FALSE );
}

void CMainFrame::OnUpdateFileExportLdlist( CCmdUI *pCmdUI )
{
	if( NULL != pDlgLeftTabProject )
	{
		if( CMainFrame::RightViewList::eRVLogData == pDlgLeftTabProject->GetCurrentView() )
		{
			pCmdUI->Enable( TRUE );
			return;
		}
	}

	pCmdUI->Enable( FALSE );
}

void CMainFrame::OnUpdateFileExportQM( CCmdUI *pCmdUI )
{
	CTable *pTab = TASApp.GetpTADS()->GetpQuickMeasureTable();
	ASSERT( NULL != pTab );

	if( NULL != pTab && pTab->GetItemCount() > 0 )
	{
		pCmdUI->Enable( TRUE );
		return;
	}

	pCmdUI->Enable( FALSE );
}

void CMainFrame::OnUpdateFileExportTADiagnostic( CCmdUI* pCmdUI )
{
	if( NULL != TASApp.GetpTADS()->GetpHydroModTable() )
	{
		CTable* pTable = TASApp.GetpTADS()->GetpHydroModTable();
		bool bMeasFound = false;
		for( IDPTR IDPtr = pTable->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pTable->GetNext() )
		{		
			CDS_HydroMod* pRoot = (CDS_HydroMod*)( IDPtr.MP );

			if( NULL != pRoot && true == pRoot->IsMeasurementExistsInProject( pRoot ) )
			{
				bMeasFound = true;
				break;
			}
		}

		if( true == bMeasFound )
		{
			pCmdUI->Enable( TRUE );
		}
		else
		{
			pCmdUI->Enable( FALSE );
		}
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnUpdateImport( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( TRUE );
}

void CMainFrame::OnUpdateMenuImportHM( CCmdUI *pCmdUI )
{
	if( NULL != TASApp.GetpTADS()->GetpHydroModTable() )
	{
		CTable* pTable = TASApp.GetpTADS()->GetpHydroModTable();
		CDS_HydroMod* pChild = (CDS_HydroMod*)( pTable->GetFirst().MP );
		if( NULL != pChild && true == IsHMCalcChecked() && true == pChild->GetpPrjParam()->IsFreezed() )
		{
			pCmdUI->Enable( FALSE );
		}
		else
		{
			pCmdUI->Enable( TRUE );
		}
	}
	else
	{
		pCmdUI->Enable( TRUE );
	}
}

void CMainFrame::OnUpdateMenuImportHMJson( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( TRUE );
}

void CMainFrame::OnUpdateLanguage( CCmdUI *pCmdUI )
{
	CMFCRibbonComboBox *pCombo = DYNAMIC_DOWNCAST( CMFCRibbonComboBox, m_wndRibbonBar.FindByID( ID_RBN_H_O_LANG ) );

	if( NULL == pCombo )
	{
		return;
	}

	if( pCombo->GetCount() > 1 )
	{
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnUpdateUsermanual( CCmdUI *pCmdUI )
{
	BOOL fFind = ( FALSE == _GetManualFileName().IsEmpty() );
	pCmdUI->Enable( fFind );
}

void CMainFrame::OnUpdateActiveAutoSaving( CCmdUI *pCmdUI )
{
	pCmdUI->SetCheck( TASApp.IsAutoSave() );
}

void CMainFrame::OnUpdateCBICompatible( CCmdUI *pCmdUI )
{
	// Set the Check value.
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );
	pCmdUI->SetCheck( pPrjParam->IsCompatibleCBI() );
}

void CMainFrame::OnUpdateBtnHMCalc( CCmdUI *pCmdUI )
{
	pCmdUI->SetCheck( IsHMCalcChecked() );
}

void CMainFrame::OnUpdateFreezeHMCalc( CCmdUI *pCmdUI )
{
	// Set the Check value.
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );
	pCmdUI->SetCheck( pPrjParam->IsFreezed() );
	pCmdUI->Enable( ( true == IsHMCalcChecked() ) ? TRUE : FALSE );
}

void CMainFrame::OnUpdateHNAutoCheck( CCmdUI *pCmdUI )
{
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == IsHMCalcChecked() && false == pPrjParam->IsFreezed() )
	{
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}

	// Set the Check value.
	pCmdUI->SetCheck( pPrjParam->GetHNAutomaticCheck() );
}

void CMainFrame::OnUpdateProjectType( CCmdUI *pCmdUI )
{
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == IsHMCalcChecked() && false == pPrjParam->IsFreezed() )
	{
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnUpdateHMCalcDebugVerifyAutorityCheck( CCmdUI *pCmdUI )
{
	pCmdUI->SetCheck( ( true == m_bHMCalcDebugVerifyAutority ) ? BST_CHECKED : BST_UNCHECKED );
}

// if modified, modify also IsRibbonHMImportEnable
void CMainFrame::OnUpdateImportHM( CCmdUI *pCmdUI )
{
	if( NULL != TASApp.GetpTADS()->GetpHydroModTable() )
	{
		CTable* pTable = TASApp.GetpTADS()->GetpHydroModTable();
		CDS_HydroMod* pChild = (CDS_HydroMod*)( pTable->GetFirst().MP );
		if( NULL != pChild && true == IsHMCalcChecked() && true == pChild->GetpPrjParam()->IsFreezed() )
		{
			pCmdUI->Enable( FALSE );
		}
		else
		{
			pCmdUI->Enable( TRUE );
		}
	}
	else
	{
		pCmdUI->Enable( TRUE );
	}
}

void CMainFrame::OnUpdateProjectWaterChar( CCmdUI *pCmdUI )
{
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == IsHMCalcChecked() && true == pPrjParam->IsFreezed() )
	{
		pCmdUI->Enable( FALSE );
	}
	else
	{
		pCmdUI->Enable( TRUE );
	}
}

void CMainFrame::OnUpdateProjectWaterTemp( CCmdUI *pCmdUI )
{
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == IsHMCalcChecked() && true == pPrjParam->IsFreezed() )
	{
		pCmdUI->Enable( FALSE );
	}
	else
	{
		pCmdUI->Enable( TRUE );
	}
}


void CMainFrame::OnUpdateISWaterDT( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( TRUE );
}

void CMainFrame::OnUpdateProjectWaterDT( CCmdUI *pCmdUI )
{
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == IsHMCalcChecked() && true == pPrjParam->IsFreezed() )
	{
		pCmdUI->Enable( FALSE );
	}
	else
	{
		pCmdUI->Enable( TRUE );
	}
}

void CMainFrame::OnUpdateCheckHNCalculationBtn( CCmdUI *pCmdUI )
{
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == IsHMCalcChecked() && false == pPrjParam->IsFreezed() )
	{
		BOOL fRet = TRUE;
		
		CTableHM *pTableHM = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
		ASSERT( NULL != pTableHM );
		
		fRet &= ( ( NULL == pTableHM || 0 == pTableHM->GetItemCount() ) ? FALSE : TRUE );
		pCmdUI->Enable( fRet );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnUpdateSearchReplace( CCmdUI *pCmdUI )
{
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	// At now, it's possible to call 'Search and replace' on individual selections.
	if( ( true == IsHMCalcChecked() && false == pPrjParam->IsFreezed() ) ||
		true == TASApp.GetpTADS()->IsIndividualSelectionExist() )
	{
		BOOL fRet = TRUE;

		if( false == TASApp.GetpTADS()->IsIndividualSelectionExist() )
		{
			CTableHM *pTableHM = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
			ASSERT( NULL != pTableHM );
			
			fRet &= ( ( NULL == pTableHM || 0 == pTableHM->GetItemCount() ) ? FALSE : TRUE );
		}

		pCmdUI->Enable( fRet );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnUpdateDiversityFactor( CCmdUI *pCmdUI )
{
	// By default we always enable the possibly to click on this button. If user opens a project in which before he has changed some diversity
	// factor coefficient, he must be able to see what is has applied.
	// The only case where we don't enable this button is when there is no project.

	CTableHM *pTableHM = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
	ASSERT( pTableHM );
	pCmdUI->Enable( ( NULL == pTableHM || 0 == pTableHM->GetItemCount() ) ? FALSE : TRUE );
}

void CMainFrame::OnUpdateUpperBoundaries( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( FALSE );
}

void CMainFrame::OnUpdateLowerBoundaries( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( FALSE );
}

void CMainFrame::OnUpdateProjectDateTimeChange( CCmdUI *pCmdUI )
{
	if( GetActiveView() == m_arrayViews[eRVHMCalc] )
	{
		if( true == m_bMeasExist )
		{
			pCmdUI->Enable( TRUE );
		}
		else
		{
			pCmdUI->Enable( FALSE );
		}
	}
	else if( GetActiveView() == m_arrayViews[eRVQuickMeas] )
	{
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnUpdateTASCOPEFILES( CCmdUI *pCmdUI )
{
	pCmdUI->SetCheck( true == TASApp.IsTAScopeFilesMaintain() );
}

void CMainFrame::OnUpdateUpdateCBI( CCmdUI *pCmdUI )
{
	// Disable the component if the project is not CBI compatible.
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );
	pCmdUI->Enable( pPrjParam->IsCompatibleCBI() );
}

void CMainFrame::OnUpdateUpdateDpSVisio( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( TRUE );
}

void CMainFrame::OnUpdateTestHySelectUpdate( CCmdUI *pCmdUI )
{
	if( true == TASApp.GetpTADB()->IsDebugPsw() )
	{
		CMFCRibbonButton *pButton = DYNAMIC_DOWNCAST( CMFCRibbonButton, m_wndRibbonBar.FindByID( pCmdUI->m_nID ) );
		
		if( FALSE == pButton->IsVisible() )
		{
			pButton->SetVisible( TRUE );
			m_wndRibbonBar.ForceRecalcLayout();
		}
	}
}

void CMainFrame::OnUpdateProductSelPipeseries( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( m_fComboSelPipeSeriesEnabled );
}

void CMainFrame::OnUpdateProductSelPipesize( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( m_fComboSelPipeSizeEnabled );
}

void CMainFrame::OnUpdateProductSelSearchReplace( CCmdUI *pCmdUI )
{
	OnUpdateSearchReplace( pCmdUI );
}

void CMainFrame::OnUpdateProductSelIndividualSelection( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( m_arbSelectionModeAvailability[ProductSelectionMode_Individual][m_eCurrentProductSubCategory] );
	pCmdUI->SetCheck( ( ProductSelectionMode_Individual == m_eCurrentProductSelectionMode ) ? BST_CHECKED : BST_UNCHECKED );
}

void CMainFrame::OnUpdateProductSelBatchSelection( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( m_arbSelectionModeAvailability[ProductSelectionMode_Batch][m_eCurrentProductSubCategory] );
	pCmdUI->SetCheck( ( ProductSelectionMode_Batch == m_eCurrentProductSelectionMode ) ? BST_CHECKED : BST_UNCHECKED );
}

void CMainFrame::OnUpdateProductSelWizardSelection( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][m_eCurrentProductSubCategory] );
	pCmdUI->SetCheck( ( ProductSelectionMode_Wizard == m_eCurrentProductSelectionMode ) ? BST_CHECKED : BST_UNCHECKED );
}

void CMainFrame::OnUpdatePSApplicationType( CCmdUI *pCmdUI )
{
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
	ASSERT( NULL != pTechP );

	if( ChangeOver == pTechP->GetProductSelectionApplicationType() )
	{
		pCmdUI->Enable( FALSE );
	}
	else
	{
		pCmdUI->Enable( TRUE );
	}
}

void CMainFrame::OnUpdateProductSelPMWQSelectPrefs( CCmdUI *pCmdUI )
{
	BOOL fEnable = ( ProductSubCategory::PSC_PM_ExpansionVessel == m_eCurrentProductSubCategory ) ? TRUE : FALSE;
	pCmdUI->Enable( fEnable );
}

void CMainFrame::OnUpdateProductSelPMWQOneDevice( CCmdUI *pCmdUI )
{
	// Called by framework to refresh the check box in the 'Technical parameters' category for the 
	// pressure maintenance individual selection.

	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	// HYS-1537 : We modify only if the pref is enabled
	if( NULL != pclPMInputUser && NULL != pclPMInputUser->GetPMWQSelectionPreferences() 
		&& false == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralCombinedInOneDevice ) )
	{
		pCmdUI->SetCheck( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice ) ) ? TRUE : FALSE );
		pCmdUI->Enable( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralCombinedInOneDevice ) ) ? FALSE : TRUE );
	}
	else
	{
		pCmdUI->SetCheck( FALSE );
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnUpdateProductSelPMWQRedundancy( CCmdUI *pCmdUI )
{
	// Called by framework to refresh the check box in the 'Technical parameters' category for the 
	// pressure maintenance individual selection.
	BOOL fEnable = ( ProductSubCategory::PSC_PM_ExpansionVessel == m_eCurrentProductSubCategory ) ? TRUE : FALSE;
	pCmdUI->Enable( fEnable );
}

void CMainFrame::OnUpdateProductSelPMWQPrefsOneDevice( CCmdUI *pCmdUI )
{
	// Called by framework to refresh the button in the 'Pressurisation preferences' category for the 
	// pressure maintenance individual selection.
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL != pclPMInputUser && NULL != pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		pCmdUI->SetCheck( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice ) ) ? TRUE : FALSE );
		pCmdUI->Enable( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralCombinedInOneDevice ) ) ? FALSE : TRUE );
	}
	else
	{
		pCmdUI->SetCheck( FALSE );
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnUpdateProductSelPMWQPrefsRedPumpComp( CCmdUI *pCmdUI )
{
	// Called by framework to refresh the button in the 'Pressurisation preferences' category for the 
	// pressure maintenance individual selection.
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL != pclPMInputUser && NULL != pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		pCmdUI->SetCheck( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyPumpComp ) ) ? TRUE : FALSE );
		pCmdUI->Enable( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralRedundancyPumpComp ) ) ? FALSE : TRUE );
	}
	else
	{
		pCmdUI->SetCheck( FALSE );
		pCmdUI->Enable( FALSE );
	}

	// Update here the combo.
	_UpdateRibbonPMWQComboRedundancy();
}

void CMainFrame::OnUpdateProductSelPMWQPrefsRedTecBox( CCmdUI *pCmdUI )
{
	// Called by framework to refresh the button in the 'Pressurisation preferences' category for the 
	// pressure maintenance individual selection.
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL != pclPMInputUser && NULL != pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		pCmdUI->SetCheck( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox ) ) ? TRUE : FALSE );
		pCmdUI->Enable( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox ) ) ? FALSE : TRUE );
	}
	else
	{
		pCmdUI->SetCheck( FALSE );
		pCmdUI->Enable( FALSE );
	}

	// Update here the combo.
	_UpdateRibbonPMWQComboRedundancy();
}

void CMainFrame::OnUpdateProductSelPMWQPrefsIntCoating( CCmdUI *pCmdUI )
{
	// Called by framework to refresh the button in the 'Pressurisation preferences' category for the 
	// pressure maintenance individual selection.
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL != pclPMInputUser && NULL != pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		pCmdUI->SetCheck( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::CompressoInternalCoating ) ) ? TRUE : FALSE );
		pCmdUI->Enable( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::CompressoInternalCoating ) ) ? FALSE : TRUE );
	}
	else
	{
		pCmdUI->SetCheck( FALSE );
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnUpdateProductSelPMWQPrefsExternalAir( CCmdUI *pCmdUI )
{
	// Called by framework to refresh the button in the 'Pressurisation preferences' category for the 
	// pressure maintenance individual selection.
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL != pclPMInputUser && NULL != pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		pCmdUI->SetCheck( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::CompressoExternalAir ) ) ? TRUE : FALSE );
		pCmdUI->Enable( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::CompressoExternalAir ) ) ? FALSE : TRUE );
	}
	else
	{
		pCmdUI->SetCheck( FALSE );
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnUpdateProductSelPMWQPrefsCoolingInsulation( CCmdUI *pCmdUI )
{
	// Called by framework to refresh the button in the 'Pressurisation preferences' category for the 
	// pressure maintenance individual selection.
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL != pclPMInputUser && NULL != pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		pCmdUI->SetCheck( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::PumpDegassingCoolingVersion ) ) ? TRUE : FALSE );
		pCmdUI->Enable( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::PumpDegassingCoolingVersion ) ) ? FALSE : TRUE );
	}
	else
	{
		pCmdUI->SetCheck( FALSE );
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnUpdateProductSelPMWQPrefsBreakTank( CCmdUI *pCmdUI )
{
	// Called by framework to refresh the button in the 'Pressurisation preferences' category for the 
	// pressure maintenance individual selection.
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL != pclPMInputUser && NULL != pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		pCmdUI->SetCheck( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::WaterMakeupBreakTank ) ) ? TRUE : FALSE );
		pCmdUI->Enable( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::WaterMakeupBreakTank ) ) ? FALSE : TRUE );
	}
	else
	{
		pCmdUI->SetCheck( FALSE );
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnUpdateProductSelPMWQPrefsDutyStdBy( CCmdUI *pCmdUI )
{
	// Called by framework to refresh the button in the 'Pressurisation preferences' category for the 
	// pressure maintenance individual selection.
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL != pclPMInputUser && NULL != pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		pCmdUI->SetCheck( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::WaterMakeupDutyStandBy ) ) ? TRUE : FALSE );
		pCmdUI->Enable( ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::WaterMakeupDutyStandBy ) ) ? FALSE : TRUE );
	}
	else
	{
		pCmdUI->SetCheck( FALSE );
		pCmdUI->Enable( FALSE );
	}
}

void CMainFrame::OnCustomizeUserpassword()
{
	CDlgPassword dlg;
	dlg.DoModal();
}

#ifdef _DEBUG
void CMainFrame::OnRbnTestsSpecialActions()
{
	if( NULL == m_pDlgDebug )
	{
		m_pDlgDebug = new CDlgSpecAct();

		if( NULL == m_pDlgDebug )
		{
			return;
		}

		if( 0 == m_pDlgDebug->Create( IDD_DLGSPECACT, this ) )
		{
			delete m_pDlgDebug;
			m_pDlgDebug = NULL;
			return;
		}
	}

	m_pDlgDebug->ShowWindow( SW_SHOW );
	m_pDlgDebug->SetFocus();
}

void CMainFrame::OnRbnTestsUnit()
{
	if( NULL != m_pTestRunnerModel )
	{
		delete m_pTestRunnerModel;
		m_pTestRunnerModel = NULL;
	}

	m_pTestRunnerModel = new TestRunnerModel();

	if( NULL != m_pTestRunnerDlg )
	{
		delete m_pTestRunnerDlg;
		m_pTestRunnerDlg = NULL;
	}

	m_pTestRunnerDlg = new TestRunnerDlg( m_pTestRunnerModel, CPP_UNIT_TEST_RUNNER_IDD_DIALOG_TESTRUNNER, this );

	if( m_pTestRunnerDlg->CheckFile() )
	{
		TASApp.SetUnitTest( true );
		m_pTestRunnerDlg->DoModal();
		TASApp.SetUnitTest( false );
		//m_pTestRunnerDlg->Create( CPP_UNIT_TEST_RUNNER_IDD_DIALOG_TESTRUNNER, this );
	}
}
#endif


LRESULT CMainFrame::OnPswChanged( WPARAM wParam, LPARAM lParam )
{
	// Get the Number to the selected image.
	int iSelectedItem = m_pGallery->GetSelectedItem();

	// Get the LPARAM of the corresponding item.
	iSelectedItem = (int)m_pGallery->GetLparamFromImageIndex( iSelectedItem );

	// Reset the product selection Gallery Box.
	_FillProductSubCategoryGalleryBox();

	// Get the item corresponding to the LPARAM.
	iSelectedItem = m_pGallery->GetImageIndexFromLparam( iSelectedItem );

	// Set back the last selected number.
	m_pGallery->SelectItem( iSelectedItem );

	// Refresh the layout.
	m_wndRibbonBar.ForceRecalcLayout();

	return 0;
}

LRESULT CMainFrame::OnProjectRefChange( WPARAM wParam, LPARAM lParam )
{
	m_pEBProjName->SetEditText( (CString)TASApp.GetpTADS()->GetpProjectRef()->GetString( CDS_ProjectRef::Name ) );
	return 0;
}

LRESULT CMainFrame::OnUserPipeChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != ( WMUserPipeChange )wParam )
	{
		return 0;
	}

	int nSel = m_pCBPipeSeries->GetCurSel();

	if( -1 != nSel )
	{
		// Change combo pipe series for product selection.
		m_pRibbonComboSelPipeSeries->SelectItem( nSel );
		OnRbnPSPPipeSeries();
	}

	return 0;
}

LRESULT CMainFrame::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	InitializeRibbonRichEditBoxes();
	return 0;
}

LRESULT CMainFrame::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserWaterCharWParam::WM_UWC_WP_ForTools == wParam )
	{
		return 0;
	}

	CString str1, str2, str3;
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
	CDS_WaterCharacteristic *pWC = NULL;

	if( WMUserWaterCharWParam::WM_UWC_WP_ForProject == ( WMUserWaterCharWParam::WM_UWC_WP_ForProject & wParam ) )
	{
		pWC = TASApp.GetpTADS()->GetpWCForProject();

		// Fill strings in the ribbon 'Project' tab.
		pWC->BuildWaterStringsRibbonBar( str1, str2, str3 );
		
		CString str = str1;

		if( false == str2.IsEmpty() )
		{
			str = str1 + _T("; ") + str2;
		}

		m_pLBWaterCharInfo1Proj->SetText( str );
		m_pEBWaterCharTempProj->SetEditTextEx( str3 );

		m_pEBWaterCharDTProj->SetCurrentValSI( pTechP->GetDefaultPrjDT() );

		// HYS-1041: Disable Wizard selection mode.
		if( ( NULL != pWC ) && ( 0 != StringCompare( pWC->GetAdditFamID(), _T("GLYCOL_ADDIT") ) ) &&
			( 0 != StringCompare( pWC->GetAdditFamID(), _T("WATER_ADDIT") ) ) )
		{
			m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_PM_ExpansionVessel] = false;
			m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_PM_AirVent] = false;
		}
		else
		{
			m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_PM_ExpansionVessel] = true;
			m_arbSelectionModeAvailability[ProductSelectionMode_Wizard][PSC_PM_AirVent] = true;
		}
	}
	
	// Intentionally no 'else' because we can have 'WMUserWaterChar::WM_UMC_ForAll' in 'wParam'.

	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel == ( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel & wParam ) )
	{
		pWC = TASApp.GetpTADS()->GetpWCForProductSelection();

		// Fill strings in the ribbon 'Product selection' tab.
		pWC->BuildWaterStringsRibbonBar( str1, str2, str3 );
		CString str = str1;

		if( false == str2.IsEmpty() )
		{
			str = str1 + _T("; ") + str2;
		}

		m_pLBWaterCharInfo1ProductSel->SetText( str );
		if( ChangeOver == pTechP->GetProductSelectionApplicationType() )
		{
			TASApp.GetpTADS()->GetpWCForProductSelection()->BuildTempChangeOverStringsRibbonBar( str1, str2 );
			int iIndex = m_pPanelProductSelFluidCharacteristics->GetIndex( m_pEBWaterCharTempProductSel );
			if( -1 != iIndex )
			{
				if( NULL == m_pEBWaterCharChangeOverTempProductSel )
				{
					// HYS-1693: To correctly update the ribbon we should specify the type of the edittext. By default 
					// it considers that it is a numerical edit
					m_pEBWaterCharChangeOverTempProductSel = new CMFCRibbonEditEx( ID_RBN_PS_WCCO_TEMP, 100, TASApp.LoadLocalizedString( IDS_RBN_PS_WC_TEMP ),
						CRCImageManager::ILSPS_Temp, CMFCRibbonEditEx::eTypeRichEditCtrl::eLastEditCtrl );
					m_pEBWaterCharChangeOverTempProductSel->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_DISABLED );
				}

				m_pPanelProductSelFluidCharacteristics->Remove( iIndex, TRUE );
				m_pPanelProductSelFluidCharacteristics->Insert( m_pEBWaterCharChangeOverTempProductSel, iIndex );
				m_pEBWaterCharTempProductSel = NULL;
			}

			iIndex = m_pPanelProductSelFluidCharacteristics->GetIndex( m_pEBWaterCharDTProductSel );
			if( -1 != iIndex )
			{
				if( NULL == m_pEBWaterCharChangeOverDTProductSel )
				{
					// HYS-1693: To correctly update the ribbon we should specify the type of the edittext. By default 
					// it considers that it is a numerical edit
					m_pEBWaterCharChangeOverDTProductSel = new CMFCRibbonEditEx( ID_RBN_PS_WCCO_DT, 100, TASApp.LoadLocalizedString( IDS_RBN_PS_WC_DT ), -1,
						CMFCRibbonEditEx::eTypeRichEditCtrl::eLastEditCtrl );
					m_pEBWaterCharChangeOverDTProductSel->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_DISABLED );
				}

				m_pPanelProductSelFluidCharacteristics->Remove( iIndex, TRUE );
				m_pPanelProductSelFluidCharacteristics->Insert( m_pEBWaterCharChangeOverDTProductSel, iIndex );
				m_pEBWaterCharDTProductSel = NULL;
			}
			m_pEBWaterCharChangeOverTempProductSel->SetEditText( str1 );
			m_pEBWaterCharChangeOverDTProductSel->SetEditText( str2 );
		}
		else
		{
			int iIndex = m_pPanelProductSelFluidCharacteristics->GetIndex( m_pEBWaterCharChangeOverTempProductSel );
			if( -1 != iIndex )
			{
				if( NULL == m_pEBWaterCharTempProductSel )
				{
					m_pEBWaterCharTempProductSel = new CMFCRibbonEditEx( ID_RBN_PS_WC_TEMP, 65, TASApp.LoadLocalizedString( IDS_RBN_PS_WC_TEMP ), CRCImageManager::ILSPS_Temp );

					m_pEBWaterCharTempProductSel->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );
					SetToolTipRibbonElement( m_pEBWaterCharTempProductSel, IDS_RBN_PS_WC_TEMP_TT );
				}

				m_pPanelProductSelFluidCharacteristics->Remove( iIndex, TRUE );
				m_pPanelProductSelFluidCharacteristics->Insert( m_pEBWaterCharTempProductSel, iIndex );
				m_pEBWaterCharChangeOverTempProductSel = NULL;

			}
			iIndex = m_pPanelProductSelFluidCharacteristics->GetIndex( m_pEBWaterCharChangeOverDTProductSel );
			if( -1 != iIndex )
			{
				if( NULL == m_pEBWaterCharDTProductSel )
				{
					m_pEBWaterCharDTProductSel = new CMFCRibbonEditEx( ID_RBN_PS_WC_DT, 65, TASApp.LoadLocalizedString( IDS_RBN_PS_WC_DT ) );

					m_pEBWaterCharDTProductSel->CreateEdit( &m_wndRibbonBar, WS_CHILD | WS_TABSTOP | ES_WANTRETURN | ES_AUTOHSCROLL );

					SetToolTipRibbonElement( m_pEBWaterCharDTProductSel, IDS_RBN_PS_WC_DTEMP_TT );
				}

				m_pPanelProductSelFluidCharacteristics->Remove( iIndex, TRUE );
				m_pPanelProductSelFluidCharacteristics->Insert( m_pEBWaterCharDTProductSel, iIndex );
				m_pEBWaterCharChangeOverDTProductSel = NULL;
			}
			m_pEBWaterCharTempProductSel->SetEditText( str3 );
			m_pEBWaterCharDTProductSel->SetCurrentValSI( pTechP->GetDefaultISDT() );
			InitializeRibbonRichEditBoxesSSel();
		}
		m_pPanelProductSelFluidCharacteristics->SetJustifyColumns();
	}

	// Refresh the ribbon bar.
	m_wndRibbonBar.ForceRecalcLayout();

	return 0;
}

LRESULT CMainFrame::OnDiversityFactorToClose( WPARAM wParam, LPARAM lParam )
{
	if( NULL != m_pDlgDiversityFactor )
	{
		if( NULL != m_pDlgDiversityFactor->GetSafeHwnd() )
		{
			m_pDlgDiversityFactor->DestroyWindow();
		}

		delete m_pDlgDiversityFactor;
		m_pDlgDiversityFactor = NULL;
	}

	return 0;
}

LRESULT CMainFrame::OnDialogPrintToClose( WPARAM wParam, LPARAM lParam )
{
	if( NULL != m_pDlgPrint )
	{
		if( NULL != m_pDlgPrint->GetSafeHwnd() )
		{
			m_pDlgPrint->DestroyWindow();
		}

		delete m_pDlgPrint;
		m_pDlgPrint = NULL;
	}

	return 0;
}

#ifdef _DEBUG
LRESULT CMainFrame::OnDialogDebugToClose( WPARAM wParam, LPARAM lParam )
{
	if( NULL != m_pDlgDebug )
	{
		if( NULL != m_pDlgDebug->GetSafeHwnd() )
		{
			m_pDlgDebug->DestroyWindow();
		}

		delete m_pDlgDebug;
		m_pDlgDebug = NULL;
	}

	return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::_CreateToolsDockablePane( void )
{
	// Create.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TOOLSDOCKABLEPANEOUTPUT_CAPTION );

	// We remove the 'WS_VISIBLE' flag to not show the dockable pane by default.
	if( FALSE == m_ToolsDockablePane.Create( str, this, CRect( 0, 0, 600, 200 ), TRUE, CToolsDockablePane::IDD,
			WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_ALIGN_BOTTOM | CBRS_FLOAT_MULTI,
			AFX_CBRS_REGULAR_TABS, AFX_CBRS_RESIZE | AFX_CBRS_AUTOHIDE ) )
	{
		TRACE0( "Failed to create Output window\n" );
		return;
	}

	// Set icon.
	// 	HICON hOutputBarIcon = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE( bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	// 	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	// This method allows to set on which side dockable pane can be docked (CBRS_ALIGN_LEFT, CBRS_ALIGN_TOP, CBRS_ALIGN_RIGHT, CBRS_ALIGN_BOTTOM
	// or CBRS_ALIGN_ANY).
	m_ToolsDockablePane.EnableDocking( CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM );

	// By default, when a pane is in auto hide mode and is hidden, if there is some focusing, activating in mainframe, the dockable pane shows and hides after
	// a little delay. If we set 'm_bHideInAutoHideMode' to TRUE the dockable pane stays always hidden. Only the user can show it.
	m_ToolsDockablePane.m_bHideInAutoHideMode = TRUE;

	DockPane( &m_ToolsDockablePane );
}

void CMainFrame::_RbnTPSizeAbv( bool fFromSSel )
{
	// Set the new value into the Technical parameters and the registry
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
	CMFCRibbonEditEx *pEdit = ( true == fFromSSel ) ? m_pEBSizeAboveSSel : m_pEBSizeAboveHome;
	int iSize = (int)pEdit->GetCurrentValSI();
	pTechP->SetSizeShiftAbove( iSize );
	CString str;
	str.Format( L"%i", iSize );
	m_pEBSizeAboveHome->SetEditTextEx( str, TRUE );
	m_pEBSizeAboveSSel->SetEditTextEx( str, TRUE );

	// Set a message Tech params have changed
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_TECHPARCHANGE );
}

void CMainFrame::_RbnTPSizeBLW( bool fFromSSel )
{
	// Set the new value into the Technical parameters and the registry
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
	CMFCRibbonEditEx *pEdit = ( true == fFromSSel ) ? m_pEBSizeBelowSSel : m_pEBSizeBelowHome;
	int iSize = (int)pEdit->GetCurrentValSI();
	pTechP->SetSizeShiftBelow( -iSize );
	CString str;
	str.Format( L"%i", abs( iSize ) );
	m_pEBSizeBelowHome->SetEditTextEx( str, TRUE );
	m_pEBSizeBelowSSel->SetEditTextEx( str, TRUE );

	// Set a message Tech params have changed
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_TECHPARCHANGE );
}

void CMainFrame::_RbnTPPDTarget( bool fFromSSel )
{
	// Set the new value into the Technical parameters and the registry
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
	CMFCRibbonEditEx *pEdit = ( true == fFromSSel ) ? m_pEBPresTargetSSel : m_pEBPresTargetHome;
	double dTarg = pEdit->GetCurrentValSI();
	m_pEBPresTargetSSel->SetEditTextEx( pEdit->GetCurrentString() );
	m_pEBPresTargetHome->SetEditTextEx( pEdit->GetCurrentString() );
	pTechP->SetPipeTargDp( dTarg );

	// Verify the Target is not higher than the Max value
	// If it is the case, change the Max value as equal to
	// the target value
	double dMax = m_pEBPresMaxHome->GetCurrentValSI();

	if( dTarg > dMax )
	{
		m_pEBPresMaxHome->SetEditTextEx( pEdit->GetCurrentString() );
		m_pEBPresMaxSSel->SetEditTextEx( pEdit->GetCurrentString() );
		pTechP->SetPipeMaxDp( dTarg );
	}

	// Set a message Tech params have changed
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_TECHPARCHANGE );
}

void CMainFrame::_RbnTPPDMax( bool fFromSSel )
{
	// Set the new value into the Technical parameters and the registry
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
	CMFCRibbonEditEx *pEdit = ( true == fFromSSel ) ? m_pEBPresMaxSSel : m_pEBPresMaxHome;
	double dMax = pEdit->GetCurrentValSI();
	m_pEBPresMaxSSel->SetEditTextEx( pEdit->GetCurrentString() );
	m_pEBPresMaxHome->SetEditTextEx( pEdit->GetCurrentString() );

	pTechP->SetPipeMaxDp( dMax );

	// Verify the Max is not lower than the Target value
	// If it is the case, change the Target value as equal to
	// the Max value
	double dTarg = m_pEBPresTargetHome->GetCurrentValSI();

	if( dMax < dTarg )
	{
		m_pEBPresTargetHome->SetEditTextEx( pEdit->GetCurrentString() );
		m_pEBPresTargetSSel->SetEditTextEx( pEdit->GetCurrentString() );
		pTechP->SetPipeTargDp( dMax );
	}

	// Set a message Tech params have changed
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_TECHPARCHANGE );
}

void CMainFrame::_RbnTPWVTarget( bool fFromSSel )
{
	// Set the new value into the Technical parameters and the registry
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
	CMFCRibbonEditEx *pEdit = ( true == fFromSSel ) ? m_pEBVelTargetSSel : m_pEBVelTargetHome;
	double dTarg = pEdit->GetCurrentValSI();
	m_pEBVelTargetSSel->SetEditTextEx( pEdit->GetCurrentString() );
	m_pEBVelTargetHome->SetEditTextEx( pEdit->GetCurrentString() );
	pTechP->SetPipeTargVel( dTarg );

	// Verify the Target is not higher than the Max value
	// If it is the case, change the Max value as equal to
	// the target value
	double dMax = pTechP->GetPipeMaxVel();
	double dMin = pTechP->GetPipeMinVel();

	if( dTarg < dMin )
	{
		// HYS-1878: 3- Set dTarg to dMin
		dTarg = dMin;
		pEdit->SetCurrentValSI( dTarg );
		m_pEBVelTargetSSel->SetEditTextEx( pEdit->GetCurrentString() );
		m_pEBVelTargetHome->SetEditTextEx( pEdit->GetCurrentString() );
		pTechP->SetPipeTargVel( dTarg );
	}

	if( dTarg > dMax )
	{
		m_pEBVelMaxHome->SetEditTextEx( pEdit->GetCurrentString() );
		m_pEBVelMaxSSel->SetEditTextEx( pEdit->GetCurrentString() );
		pTechP->SetPipeMaxVel( dTarg );
	}

	// Set a message Tech params have changed
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_TECHPARCHANGE );
}

void CMainFrame::_RbnTPWVMax( bool fFromSSel )
{
	// Set the new value into the Technical parameters and the registry
	CDS_TechnicalParameter *pTechP = TASApp.GetpTADS()->GetpTechParams();
	CMFCRibbonEditEx *pEdit = ( true == fFromSSel ) ? m_pEBVelMaxSSel : m_pEBVelMaxHome;
	double dMax = pEdit->GetCurrentValSI();
	m_pEBVelMaxSSel->SetEditTextEx( pEdit->GetCurrentString() );
	m_pEBVelMaxHome->SetEditTextEx( pEdit->GetCurrentString() );

	pTechP->SetPipeMaxVel( dMax );

	// Verify the Target is not higher than the Max value
	// If it is the case, change the target value as equal to
	// the max value
	double dTarg = pTechP->GetPipeTargVel();
	double dMin = pTechP->GetPipeMinVel();

	if( dMin > dMax )
	{
		// HYS-1878: 3- Set max velocity to min value
		dMax = dMin;
		pEdit->SetCurrentValSI( dMax );
		m_pEBVelMaxSSel->SetEditTextEx( pEdit->GetCurrentString() );
		m_pEBVelMaxHome->SetEditTextEx( pEdit->GetCurrentString() );
		pTechP->SetPipeMaxVel( dMax );
	}

	if( dTarg > dMax )
	{
		m_pEBVelTargetSSel->SetEditTextEx( pEdit->GetCurrentString() );
		m_pEBVelTargetHome->SetEditTextEx( pEdit->GetCurrentString() );
		pTechP->SetPipeTargVel( dMax );
	}

	// Set a message Tech params have changed
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_TECHPARCHANGE );
}

void CMainFrame::_FillProductSubCategoryGalleryBox()
{
	CRCImageManager *pcRCIManager = TASApp.GetpRCImageManager();

	if( NULL == pcRCIManager )
	{
		return;
	}

	// Clear the gallery.
	m_pGallery->Clear();
	m_pGallery->ClearLparamArray();

	m_GallerySelPressurisationAndWaterQuality.Clear();
	m_GallerySelBalancingAndControl.Clear();
	m_GallerySelThermostaticControl.Clear();

	// Load the image list.
	CImageList *pclImgListStripSelCateg = pcRCIManager->GetImageList( CRCImageManager::ILN_StripSelectionCategory );
	ASSERT( NULL != pclImgListStripSelCateg );

	if( NULL == pclImgListStripSelCateg )
	{
		return;
	}

	IMAGEINFO rImageInfo;

	if( FALSE == pclImgListStripSelCateg->GetImageInfo( CRCImageManager::ILSSC_BalancingValve, &rImageInfo ) )
	{
		return;
	}

	CBitmap *pclFirstBitmap = CBitmap::FromHandle( rImageInfo.hbmImage );

	if( NULL == pclFirstBitmap )
	{
		return;
	}

	m_clIconSizeProductSelectionGallery = CSize( rImageInfo.rcImage.right, rImageInfo.rcImage.bottom );
	m_GallerySelPressurisationAndWaterQuality.SetImageSize( m_clIconSizeProductSelectionGallery );
	m_GallerySelBalancingAndControl.SetImageSize( m_clIconSizeProductSelectionGallery );
	m_GallerySelThermostaticControl.SetImageSize( m_clIconSizeProductSelectionGallery );

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pressurization and water quality group.
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int iNbrImages = 0;
	bool bEnabled;
	int iIcon;

	// If we can display air vents & separators...
	if( true == TASApp.IsAirVentSepDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_PM_Separator];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_Separator : CRCImageManager::ILSSC_SeparatorGrayed;

		m_GallerySelPressurisationAndWaterQuality.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_PM_Separator );
		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELSEPARATOR ) );
		m_eCurrentProductSubCategory = ProductSubCategory::PSC_PM_Separator;
	}

	if( true == TASApp.IsPressureMaintenanceDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_PM_ExpansionVessel];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_PressureMaintenance : CRCImageManager::ILSSC_PressureMaintenanceGrayed;

		m_GallerySelPressurisationAndWaterQuality.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_PM_ExpansionVessel );
		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELPRESSUREMAINTENANCE ) );

		if( ProductSubCategory::PSC_Undefined == m_eCurrentProductSubCategory )
		{
			m_eCurrentProductSubCategory = ProductSubCategory::PSC_PM_ExpansionVessel;
		}
	}

	if( true == TASApp.IsSafetyValveDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_PM_SafetyValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_SafetyValve : CRCImageManager::ILSSC_SafetyValveGrayed;

		m_GallerySelPressurisationAndWaterQuality.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_PM_SafetyValve );
		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELSAFETYVALVE ) );

		if( ProductSubCategory::PSC_Undefined == m_eCurrentProductSubCategory )
		{
			m_eCurrentProductSubCategory = ProductSubCategory::PSC_PM_SafetyValve;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Balancing and control group.
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Balancing valve.
	bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_RegulatingValve];
	iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_BalancingValve : CRCImageManager::ILSSC_BalancingValveGrayed;

	m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

	m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_BC_RegulatingValve );
	m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
	m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELBV ) );

	if( ProductSubCategory::PSC_Undefined == m_eCurrentProductSubCategory )
	{
		m_eCurrentProductSubCategory = ProductSubCategory::PSC_BC_RegulatingValve;
	}

	// If we can display Dp controller...
	if( true == TASApp.IsDpCDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_DpController];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_DpController : CRCImageManager::ILSSC_DpControllerGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_BC_DpController );
		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELDPC ) );
	}

	// If we can display balancing and control valve.
	if( true == TASApp.IsBCvDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_BalAndCtrlValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_BalancingControlValve : CRCImageManager::ILSSC_BalancingControlValveGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_BC_BalAndCtrlValve );
		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELBCV ) );
	}

	// If we can display pressure and independent control valve.
	if( true == TASApp.IsPICvDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_PressureIndepCtrlValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_PressIndepControlValve : CRCImageManager::ILSSC_PressIndepControlValveGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_BC_PressureIndepCtrlValve );
		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELPICV ) );
	}

	// If we can display combined Dp controller, balancing and control valve.

	// HYS-2110: By disabling TA-Compact-Dp for US, no more valves available for DpCBCV type so we have to hide the type. 
	CRankEx list;
	int iValveCpt = TASApp.GetpTADB()->GetTaCVTypeList( &list, CTADatabase::eForDpCBCV );

	if( 0 >= iValveCpt )
	{
		TASApp.SetDpCBCVUsed( false );
		m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve] = false;
	}

	if( true == TASApp.IsDpCBCVDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_DpCBCV : CRCImageManager::ILSSC_DpCBCVGrayed;

		m_GallerySelBalancingAndControl.AddIcon(pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve );
		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELDPCBCV ) );
	}

	// HYS-1149: If we can display 6Way control valve.
	if( true == TASApp.Is6WayCVDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_6WayControlValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_6WayValve : CRCImageManager::ILSSC_6WayValveGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_BC_6WayControlValve );
		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSEL6WAY ) );
	}

	// If we can display control valve...
	if( true == TASApp.IsCvDisplayed() )
	{
		if( 0 == TASApp.GetCvFamily().CompareNoCase( L"Brazil" ) )
		{
			bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_ControlValve];
			iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_ControlValveBrazil : CRCImageManager::ILSSC_ControlValveBrazilGrayed;

			m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

			m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_BC_ControlValve );
			m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
			m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELCV ) );
		}
		else
		{
			bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_ControlValve];
			iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_ControlValveHora : CRCImageManager::ILSSC_ControlValveHoraGrayed;

			m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

			m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_BC_ControlValve );
			m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
			m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELCV ) );
		}
	}

	// If we can display smart control valve...
	if( true == TASApp.IsSmartControlValveDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_SmartControlValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_SmartControlValve : CRCImageManager::ILSSC_SmartControlValveGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_BC_SmartControlValve );
		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELSMARTCONTROLVALVE ) );
	}

	// If we can display smart differential pressure controller.
	if( true == TASApp.IsSmartDpCDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_SmartDpC];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_SmartDp : CRCImageManager::ILSSC_SmartDpGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_BC_SmartDpC );
		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELSMARTDPC ) );
	}

	// If we can display shut-off valve.
	if( true == TASApp.IsShutOffValveDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_ESC_ShutoffValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_ShutOffValve : CRCImageManager::ILSSC_ShutOffValveGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_ESC_ShutoffValve );
		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELSV ) );
	}

	// If we can display HUB...
	if( true == TASApp.IsHubDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_HUB];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_Hub : CRCImageManager::ILSSC_HubGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_BC_HUB );
		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELTAHUB ) );
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Thermostatic control group.
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// If we can display thermostatic valve.
	if( true == TASApp.IsTrvDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_TC_ThermostaticValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_ThermostaticValve : CRCImageManager::ILSSC_ThermostaticValveGrayed;

		m_GallerySelThermostaticControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetLparamToImageIndex( iNbrImages, (LPARAM)ProductSubCategory::PSC_TC_ThermostaticValve );
		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELTRV ) );
	}

	m_pGallery->ShowXIcons( iNbrImages );

	// Create all groups.
	if( true == TASApp.IsAirVentSepDisplayed() || true == TASApp.IsPressureMaintenanceDisplayed() )
	{
		m_pGallery->AddGroup( TASApp.LoadLocalizedString( IDS_RBN_PS_PT_G_PWQ ), m_GallerySelPressurisationAndWaterQuality );
	}

	if( true == TASApp.IsCvDisplayed() || true == TASApp.IsBCvDisplayed() || true == TASApp.IsPICvDisplayed() )
	{
		m_pGallery->AddGroup( TASApp.LoadLocalizedString( IDS_RBN_PS_PT_G_BC ), m_GallerySelBalancingAndControl );
	}

	if( true == TASApp.IsTrvDisplayed() )
	{
		m_pGallery->AddGroup( TASApp.LoadLocalizedString( IDS_RBN_PS_PT_G_TC ), m_GallerySelThermostaticControl );
	}
}

void CMainFrame::_UpdateProductSubCategoryGalleryBox()
{
	CRCImageManager *pcRCIManager = TASApp.GetpRCImageManager();

	if( NULL == pcRCIManager )
	{
		return;
	}

	// Load the image list.
	CImageList *pclImgListStripSelCateg = pcRCIManager->GetImageList( CRCImageManager::ILN_StripSelectionCategory );
	ASSERT( NULL != pclImgListStripSelCateg );

	if( NULL == pclImgListStripSelCateg )
	{
		return;
	}

	// Clear the gallery.
	m_pGallery->Clear();

	m_GallerySelPressurisationAndWaterQuality.Clear();
	m_GallerySelBalancingAndControl.Clear();
	m_GallerySelThermostaticControl.Clear();

	m_GallerySelPressurisationAndWaterQuality.SetImageSize( m_clIconSizeProductSelectionGallery );
	m_GallerySelBalancingAndControl.SetImageSize( m_clIconSizeProductSelectionGallery );
	m_GallerySelThermostaticControl.SetImageSize( m_clIconSizeProductSelectionGallery );

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pressurization and water quality group.
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int iNbrImages = 0;
	bool bEnabled;
	int iIcon;

	// If we can display air vents & separators...
	if( true == TASApp.IsAirVentSepDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_PM_Separator];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_Separator : CRCImageManager::ILSSC_SeparatorGrayed;

		m_GallerySelPressurisationAndWaterQuality.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELSEPARATOR ) );
	}

	if( true == TASApp.IsPressureMaintenanceDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_PM_ExpansionVessel];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_PressureMaintenance : CRCImageManager::ILSSC_PressureMaintenanceGrayed;

		m_GallerySelPressurisationAndWaterQuality.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELPRESSUREMAINTENANCE ) );
	}

	// If we can display safety valves...
	if( true == TASApp.IsSafetyValveDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_PM_SafetyValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_SafetyValve : CRCImageManager::ILSSC_SafetyValveGrayed;

		m_GallerySelPressurisationAndWaterQuality.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELSAFETYVALVE ) );
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Balancing and control group.
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Balancing valve.
	bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_RegulatingValve];
	iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_BalancingValve : CRCImageManager::ILSSC_BalancingValveGrayed;

	m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

	m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
	m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELBV ) );

	// If we can display Dp controller...
	if( true == TASApp.IsDpCDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_DpController];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_DpController : CRCImageManager::ILSSC_DpControllerGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELDPC ) );
	}

	// If we can display balancing and control valve.
	if( true == TASApp.IsBCvDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_BalAndCtrlValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_BalancingControlValve : CRCImageManager::ILSSC_BalancingControlValveGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELBCV ) );
	}

	// If we can display pressure and independent control valve.
	if( true == TASApp.IsPICvDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_PressureIndepCtrlValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_PressIndepControlValve : CRCImageManager::ILSSC_PressIndepControlValveGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELPICV ) );
	}

	// If we can display combined Dp controller, balancing and control valve.
	if( true == TASApp.IsDpCBCVDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_DpCBCV : CRCImageManager::ILSSC_DpCBCVGrayed;

		m_GallerySelBalancingAndControl.AddIcon(pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELDPCBCV ) );
	}

	// HYS-1149: If we can display 6-way control valve.
	if( true == TASApp.Is6WayCVDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_6WayControlValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_6WayValve : CRCImageManager::ILSSC_6WayValveGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSEL6WAY ) );
	}

	// If we can display control valve...
	if( true == TASApp.IsCvDisplayed() )
	{
		if( 0 == TASApp.GetCvFamily().CompareNoCase( L"Brazil" ) )
		{
			bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_ControlValve];
			iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_ControlValveBrazil : CRCImageManager::ILSSC_ControlValveBrazilGrayed;

			m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

			m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
			m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELCV ) );
		}
		else
		{
			bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_ControlValve];
			iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_ControlValveHora : CRCImageManager::ILSSC_ControlValveHoraGrayed;

			m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

			m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
			m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELCV ) );
		}
	}

	// If we can display smart control valve.
	if( true == TASApp.IsSmartControlValveDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_SmartControlValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_SmartControlValve : CRCImageManager::ILSSC_SmartControlValveGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELSMARTCONTROLVALVE ) );
	}
	
	// If we can display smart differential pressure controller.
	if( true == TASApp.IsSmartDpCDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_SmartDpC];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_SmartDp : CRCImageManager::ILSSC_SmartDpGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELSMARTDPC ) );
	}

	// If we can display shut-off valve.
	if( true == TASApp.IsShutOffValveDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_ESC_ShutoffValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_ShutOffValve : CRCImageManager::ILSSC_ShutOffValveGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELSV ) );
	}

	// If we can display HUB...
	if( true == TASApp.IsHubDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_BC_HUB];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_Hub : CRCImageManager::ILSSC_HubGrayed;

		m_GallerySelBalancingAndControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELTAHUB ) );
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Thermostatic control group.
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// If we can display thermostatic valve.
	if( true == TASApp.IsTrvDisplayed() )
	{
		bEnabled = m_arbSelectionModeAvailability[m_eCurrentProductSelectionMode][ProductSubCategory::PSC_TC_ThermostaticValve];
		iIcon = ( true == bEnabled ) ? CRCImageManager::ILSSC_ThermostaticValve : CRCImageManager::ILSSC_ThermostaticValveGrayed;

		m_GallerySelThermostaticControl.AddIcon( pclImgListStripSelCateg->ExtractIcon( iIcon ), TRUE );

		m_pGallery->SetEnabledIcon( iNbrImages, bEnabled );
		m_pGallery->SetItemToolTip( iNbrImages++, TASApp.LoadLocalizedString( IDS_COMBOSSELTRV ) );
	}


	// Create all groups.
	if( true == TASApp.IsAirVentSepDisplayed() || true == TASApp.IsPressureMaintenanceDisplayed() )
	{
		m_pGallery->AddGroup( TASApp.LoadLocalizedString( IDS_RBN_PS_PT_G_PWQ ), m_GallerySelPressurisationAndWaterQuality );
	}

	if( true == TASApp.IsCvDisplayed() || true == TASApp.IsBCvDisplayed() || true == TASApp.IsPICvDisplayed() )
	{
		m_pGallery->AddGroup( TASApp.LoadLocalizedString( IDS_RBN_PS_PT_G_BC ), m_GallerySelBalancingAndControl );
	}

	if( true == TASApp.IsTrvDisplayed() )
	{
		m_pGallery->AddGroup( TASApp.LoadLocalizedString( IDS_RBN_PS_PT_G_TC ), m_GallerySelThermostaticControl );
	}

	m_wndRibbonBar.ForceRecalcLayout();
}

void CMainFrame::_FillRibbonUnitCB( CMFCRibbonComboBox *pCombo, ePHYSTYPE PhysType )
{
	pCombo->RemoveAllItems();

	for( int i = 0 ; i < CDimValue::AccessUDB()->GetLength( PhysType ); i++ )
	{
		_string name;
		name = GetNameOf( CDimValue::AccessUDB()->GetUnit( PhysType, i ) );
		pCombo->AddItem( name.c_str(), PhysType );
	}

	pCombo->SelectItem( CDimValue::AccessUDB()->GetDefaultUnitIndex( PhysType ) );
}

void CMainFrame::_FillRibbonLanguageCB( CMFCRibbonComboBox *pCombo )
{
	// Init variables.
	std::map<int, CData *> Map;
	std::map<int, CData *>::iterator it;
	int iCurLang = 0;

	// Remove all items.
	pCombo->RemoveAllItems();

	// Get the map with all languages.
	CTableOrdered *pTab = ( CTableOrdered * )TASApp.GetpTADB()->Get( L"LANG_TAB" ).MP;
	ASSERT( pTab );

	// Order the list.
	for( IDPTR IDPtr = pTab->GetFirst(); *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		Map.insert( std::pair<int, CData *>( _ttoi( ( ( CDB_StringID * )IDPtr.MP )->GetIDstr() ), IDPtr.MP ) );
	}

	// Fill the combo with all languages.
	for( it = Map.begin(); it != Map.end(); it++ )
	{
		LPCTSTR lpLang = ( ( CDB_StringID * )it->second )->GetString();
		LPCTSTR lpTriGram = ( ( CDB_StringID * )it->second )->GetIDstr2();
		pCombo->AddItem( lpLang, (DWORD_PTR)lpTriGram );
	}

	// Do a loop to find the current language.
	for( int i = 0; i < pCombo->GetCount(); i++ )
	{
		DWORD_PTR pDWORD = pCombo->GetItemData( i );
		CString str = TASApp.GetLanguage();

		if( 0 == str.CompareNoCase( (LPCTSTR)pDWORD ) )
		{
			iCurLang = i;
		}
	}

	// Select the current language.
	pCombo->SelectItem( iCurLang );
}

void CMainFrame::_FillRibbonPMWQComboRedundancy( CMFCRibbonComboBox *pCombo )
{
	CTable *pTable = dynamic_cast<CTable *>( (CData *)( TASApp.GetpTADB()->Get( _T("PMREDUNDANCY_TAB") ).MP ) );
	ASSERT( NULL != pTable );

	if( NULL == pTable )
	{
		return;
	}

	pCombo->RemoveAllItems();

	// 1st of all, sort in regards to the index set in the 'CDB_StringID' object.
	std::map<int, CDB_StringID *> mapItems;

	for( IDPTR IDPtr = pTable->GetFirst(); _NULL_IDPTR != IDPtr; IDPtr = pTable->GetNext() )
	{
		CDB_StringID *pclStringID = dynamic_cast<CDB_StringID *>( (CData *)IDPtr.MP );
		ASSERT( NULL != pclStringID );

		if( NULL == pclStringID )
		{
			continue;
		}

		mapItems.insert( std::pair<int, CDB_StringID*>( _wtoi( pclStringID->GetIDstr() ), pclStringID ) );
	}

	// 2nd insert items in the combo.
	for( auto &iter : mapItems )
	{
		pCombo->AddItem( iter.second->GetString(), (DWORD_PTR)( iter.second->GetIDPtr().MP ) );
	}

	// TODO: Set the correct selection in regards to the user choice.
	pCombo->SelectItem( 0 );
}

void CMainFrame::_UpdateRibbonPMWQComboRedundancy()
{
	CPMInputUser *pclPMInputUser = NULL;

	if( NULL != pDlgLeftTabSelManager )
	{
		pclPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( GetCurrentProductSelectionMode() );
	}

	if( NULL == pclPMInputUser || NULL == pclPMInputUser->GetPMWQSelectionPreferences() )
	{
		return;
	}

	// HYS-1537 : The preference on which we base the combo values should be enable
	bool bRedundancyPumpCompr = pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyPumpComp );
	bool bRedundancyPumpComprDisabled = pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralRedundancyPumpComp );
	bool bRedundancyTecBox = pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox );
	bool bRedundancyTecBoxDisabled = pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox );
	int iSelectItem = ( true == bRedundancyPumpCompr && false == bRedundancyPumpComprDisabled ) ? 1 : 0;
	iSelectItem += ( true == bRedundancyTecBox && false == bRedundancyTecBoxDisabled ) ? 2 : 0;

	m_pPMWQSelecPrefsRedundancy->SelectItem( iSelectItem );
}

void CMainFrame::_SetNewDateTimeValues()
{
	// Set the Start limit.
	m_pEBStartLimitProj->SetEditText( m_tCurrentLowerTimeLimits.Format() );

	// Set the End limit.
	m_pEBEndLimitProj->SetEditText( m_tCurrentUpperTimeLimits.Format() );
}

CString CMainFrame::_GetManualFileName()
{
	CString HelpFolder;
	CString DocumentFolder = TASApp.GetDocumentsFolderForHelp();

	// Find the Readme file according to the language.
	HelpFolder = DocumentFolder + CString( _HYSELECT_NAME_BCKSLASH ) + CString( _HELP_DIRECTORY );

	CFileStatus fs;
	CString DefaultEnExt = _T("en");
	CString CurrentSelectLanguage = TASApp.GetLanguage();
	CString Filename = HelpFolder +  _T("\\" ) + CurrentSelectLanguage +  _T( "\\") + _HYSELECTUSERGUIDE;
	BOOL fFind = ( TRUE == CFile::GetStatus( Filename, fs ) );

	if( FALSE == fFind )
	{
		Filename = HelpFolder +  _T("\\" ) + DefaultEnExt +  _T( "\\") + _HYSELECTUSERGUIDE;
		fFind = ( TRUE == CFile::GetStatus( Filename, fs ) );
	}

	if( FALSE == fFind )
	{
		return CString( _T( "" ) );
	}

	return Filename;
}

void CMainFrame::_PostMessageToDescendants( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL bDeep )
{
	// It's the copy of 'CWnd::SendMessageToDescendants' to allow to post instead of send.

	// walk through HWNDs to avoid creating temporary CWnd objects
	// unless we need to call this function recursively
	for (HWND hWndChild = ::GetTopWindow(hWnd); hWndChild != NULL; hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		// send message with Windows SendMessage API
		::PostMessage(hWndChild, message, wParam, lParam);
		
		if (bDeep && ::GetTopWindow(hWndChild) != NULL)
		{
			// send to child windows after parent
			_PostMessageToDescendants(hWndChild, message, wParam, lParam, bDeep );
		}
	}
}

LRESULT CMainFrame::OnApplicationTypeChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pCBProductSelectionApplicationType )
	{
		return 0;
	}

	for( int i = 0; i < m_pCBProductSelectionApplicationType->GetCount(); ++i )
	{
		if( (int)m_pCBProductSelectionApplicationType->GetItemData( i ) == (int)wParam )
		{
			m_pCBProductSelectionApplicationType->SelectItem( i );
			break;
		}
	}

	_OnRbnProductSelectionApplicationTypeChanged();
	return 0;
}

void CMainFrame::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CFrameWndEx::OnWindowPosChanged(lpwndpos);

	if ((lpwndpos->flags & SWP_SHOWWINDOW) && !m_fShown)
	{
		m_fShown = true;

		if (true == TASApp.IsUnitTest())
		{
#ifdef _DEBUG
			// Get the top level suite from the registry
			CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

			// Create Path for the outputfile
			CPath path(_T(__FILE__));
			CString outputFilePath(path.GetPath() + _T("Tests\\UnitTests\\Out\\cunit_result.xml"));
			bool bRet = WinSys::CreateFullDirectoryPath(outputFilePath);

			CppUnit::TextUi::TestRunner runner;
			std::ofstream outputFile(outputFilePath);

			CppUnit::XmlOutputter *outputter = new CppUnit::XmlOutputter(&runner.result(), outputFile);
			runner.setOutputter(outputter);
			runner.addTest(suite);
			bool wasSucessful = runner.run();
			outputFile.close();
			PostMessage( WM_CLOSE );
#endif
		}

	}
}

bool CMainFrame::_OnRbnProjectApplicationTypeChanged( bool bSendWaterChangeMessage )
{
	CDS_TechnicalParameter *pTechnicalParameters = TASApp.GetpTADS()->GetpTechParams();

	if( NULL == pTechnicalParameters )
	{
		ASSERTA_RETURN( false );
	}

	ProjectType ePrjTypeInTechParam = pTechnicalParameters->GetProjectApplicationType();

	if( NULL == m_pCBProjectApplicationType )
	{
		ASSERTA_RETURN( false );
	}

	int iCurSel = m_pCBProjectApplicationType->GetCurSel();

	if( iCurSel < 0 )
	{
		ASSERTA_RETURN( false );
	}

	ProjectType ePrjTypSelected = (ProjectType)m_pCBProjectApplicationType->GetItemData( iCurSel );

	if( ePrjTypeInTechParam == ePrjTypSelected )
	{
		// Nothing changed. Do nothing.
		return false;
	}

	pTechnicalParameters->SetProjectApplicationType( (ProjectType)ePrjTypSelected );

	// Update PROJ_WC
	// Overwrite temperature and defaultDT with values coming from technical parameters
	CDS_WaterCharacteristic *pWC = TASApp.GetpTADS()->GetpWCForProject();

	if( NULL == pWC )
	{
		ASSERTA_RETURN( false );
	}

	IDPTR AdditIDPtr = TASApp.GetpTADB()->Get( pTechnicalParameters->GetDefaultPrjAdditID() );

	pWC->SetAdditFamID( AdditIDPtr.PP->GetIDPtr().ID );
	pWC->SetAdditID( AdditIDPtr.ID );

	double dPcWeight = pTechnicalParameters->GetDefaultPrjPcWeight();
	pWC->SetPcWeight( dPcWeight );

	double dDefaultProjectTps = pTechnicalParameters->GetDefaultPrjTps();	
	
	// To get a correct freeze temperature, we must at least one time call 'UpdateFluidData'.
	pWC->SetTemp( dDefaultProjectTps );
	pWC->UpdateFluidData( dDefaultProjectTps );
	double dTFreeze = pWC->GetTfreez();

	double dDT = pTechnicalParameters->GetDefaultPrjDT();

	// HYS-1092: can do this test only if we have a water characteristic for the current media.
	if( NULL != pWC->GetpAdditChar() )
	{
		double dTMax = pWC->GetpAdditChar()->GetMaxT( pWC->GetPcWeight() );

		// Verify that saved temperatures match TFreeze / TMax of current fluid.
		if( Cooling == ePrjTypSelected )
		{
			dDefaultProjectTps = min( dDefaultProjectTps, ( dTMax - dDT ) );
			dDefaultProjectTps = max( dDefaultProjectTps, dTFreeze );
		}
		else // Heating.
		{
			dDefaultProjectTps = min( dDefaultProjectTps, dTMax );
			dDefaultProjectTps = max( dDefaultProjectTps, (dTFreeze + dDT) );
		}
	}
	
	// HYS-1334 : Synchronize temperature between WC and Tech param.
	if( pWC->GetTemp() != dDefaultProjectTps )
	{
		pWC->SetTemp( dDefaultProjectTps );
		pWC->UpdateFluidData( dDefaultProjectTps );
		
		// Update technical paramaters.
		pTechnicalParameters->SetDefaultPrjTps( dDefaultProjectTps );
	}

	// Send message to inform about modification of water characteristics.
	if( true == bSendWaterChangeMessage )
	{
		OnWaterChange( WMUserWaterCharWParam::WM_UWC_WP_ForProject, WMUserWaterCharLParam::WM_UWC_LWP_Change );
	}

	return true;
}

void CMainFrame::_OnRbnProductSelectionApplicationTypeChanged()
{
	CDS_TechnicalParameter *pTechnicalParameters = TASApp.GetpTADS()->GetpTechParams();

	if( NULL == pTechnicalParameters )
	{
		return;
	}

	ProjectType eCurrentProductSelectionApplicationType = pTechnicalParameters->GetProductSelectionApplicationType();

	if( NULL == m_pCBProductSelectionApplicationType )
	{
		return;
	}
	
	int iCurSel = m_pCBProductSelectionApplicationType->GetCurSel();

	if( iCurSel < 0 )
	{
		return;
	}

	ProjectType eComboProductSelectionApplicationType = (ProjectType)m_pCBProductSelectionApplicationType->GetItemData( iCurSel );

	if( eCurrentProductSelectionApplicationType == eComboProductSelectionApplicationType )
	{
		// Nothing changed. Do nothing.
		return;
	}

	pTechnicalParameters->SetProductSelectionApplicationType( (ProjectType)eComboProductSelectionApplicationType );

	CDS_WaterCharacteristic *pWC = TASApp.GetpTADS()->GetpWCForProductSelection();

	if( NULL == pWC )
	{
		return;
	}

	IDPTR AdditIDPtr = TASApp.GetpTADB()->Get( pTechnicalParameters->GetDefaultISAdditID() );

	pWC->SetAdditFamID( AdditIDPtr.PP->GetIDPtr().ID );
	pWC->SetAdditID( AdditIDPtr.ID );

	double dPcWeight = pTechnicalParameters->GetDefaultISPcWeight();
	pWC->SetPcWeight( dPcWeight );
	
	double dDefaultISTps = pTechnicalParameters->GetDefaultISTps();

	if( ChangeOver == pTechnicalParameters->GetProductSelectionApplicationType() )
	{
		if( NULL != pWC->GetpWCData()->GetSecondWaterChar() )
		{
			pWC->GetpWCData()->GetSecondWaterChar()->SetTemp( pTechnicalParameters->GetDefaultISTps( HeatingSide ) );
		}
		else
		{
			pWC->GetpWCData()->SetIsForChangeOver( true );
			pWC->GetpWCData()->GetSecondWaterChar()->SetTemp( pTechnicalParameters->GetDefaultISTps( HeatingSide ) );
		}
		pWC->GetpWCData()->GetSecondWaterChar()->UpdateFluidData( pTechnicalParameters->GetDefaultISTps( HeatingSide ) );
	}
	else
	{
		// HYS-1622: We are not in changeOver selection we have to set m_bIsForChangeOver and 	CWaterChar *GetSecondWaterChar() { return m_pclSecondWaterChar; }
		if( NULL != pWC->GetpWCData()->GetSecondWaterChar() || true == pWC->GetpWCData()->GetIsForChangeOver() )
		{
			pWC->GetpWCData()->SetIsForChangeOver( false );
		}
	}

	// To get a correct freeze temperature, we must at least one time call 'UpdateFluidData'.
	pWC->SetTemp( dDefaultISTps );
	pWC->UpdateFluidData( dDefaultISTps );
	double dTFreeze = pWC->GetTfreez();

	double dDT = pTechnicalParameters->GetDefaultISDT();

	// HYS-1092: can do this test only if we have a water characteristic for the current media.
	if( NULL != pWC->GetpAdditChar() )
	{
		double dTMax = pWC->GetpAdditChar()->GetMaxT( pWC->GetPcWeight() );
		
		// Verify that saved temperatures match TFreeze / TMax of current fluid
		if( Cooling == eComboProductSelectionApplicationType )
		{
			dDefaultISTps = min( dDefaultISTps, ( dTMax - dDT ) );
			dDefaultISTps = max( dDefaultISTps, dTFreeze );
		}
		else // Solar || Heating 
		{
			dDefaultISTps = min( dDefaultISTps, dTMax );
			dDefaultISTps = max( dDefaultISTps, (dTFreeze + dDT) );
		}
	}
	// HYS-1334 : Synchronize temperature between WC and Tech param
	if( pWC->GetTemp() != dDefaultISTps )
	{
		pWC->SetTemp( dDefaultISTps );
		pWC->UpdateFluidData( dDefaultISTps );
		// update technical paramaters
		pTechnicalParameters->SetDefaultISTps( dDefaultISTps );
	}

	// HYS-1230: If user has chosen 'Other fluid' we have to retrieve the density, kinematic viscosity and specific heat he has input.
	if( 0 == CString( AdditIDPtr.ID ).Compare( _T("OTHER_FLUID") ) )
	{
		pWC->SetDens( pTechnicalParameters->GetDefaultISWC()->GetDens() );
		pWC->SetKinVisc( pTechnicalParameters->GetDefaultISWC()->GetKinVisc() );

		if( true == pTechnicalParameters->GetDefaultISWC()->GetCheckSpecifHeat() )
		{
			pWC->SetCheckSpecifHeat( true );
			pWC->SetSpecifHeat( pTechnicalParameters->GetDefaultISWC()->GetSpecifHeat() );
		}
	}

	OnWaterChange( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel, WMUserWaterCharLParam::WM_UWC_LWP_Change | WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver );

	if( NULL != pDlgLeftTabSelManager && (int)eCurrentProductSelectionApplicationType != eComboProductSelectionApplicationType )
	{
		pDlgLeftTabSelManager->ChangeApplicationType( eComboProductSelectionApplicationType );
	}
}

void CMainFrame::_UpdatePipeCombosForProductSelection()
{
	if( NULL == pDlgLeftTabSelManager )
	{
		return;
	}

	if( m_pProdSelToolsTechParamContextualCategory == m_wndRibbonBar.GetActiveCategory() && NULL != pDlgLeftTabSelManager->GetCurrentLeftTabDialog() 
			&& NULL != pDlgLeftTabSelManager->GetCurrentLeftTabDialog()->GetProductSelectParameters() )
	{
		CProductSelelectionParameters *pclProductSelectionParameters = pDlgLeftTabSelManager->GetCurrentLeftTabDialog()->GetProductSelectParameters();

		switch( GetCurrentProductSelectionMode() )
		{
			case ProductSelectionMode::ProductSelectionMode_Individual:
				{
					CDS_IndSelParameter *pclCDSIndSelParameters = pclProductSelectionParameters->m_pTADS->GetpIndSelParameter();

					if( NULL != pclCDSIndSelParameters )
					{
						SetRibbonComboSelPipeSeries( pclCDSIndSelParameters->GetPipeSeriesID( pclProductSelectionParameters ) );
						SetRibbonComboSelPipeSize( pclCDSIndSelParameters->GetPipeSizeID() );
					}
				}
				break;

			case ProductSelectionMode::ProductSelectionMode_Batch:
				{
					CDS_BatchSelParameter *pclCDSBatchSelParameters = pclProductSelectionParameters->m_pTADS->GetpBatchSelParameter();

					if( NULL != pclCDSBatchSelParameters )
					{
						SetRibbonComboSelPipeSeries( pclCDSBatchSelParameters->GetPipeSeriesID( pclProductSelectionParameters ) );
						SetRibbonComboSelPipeSize( pclCDSBatchSelParameters->GetPipeSizeID() );
					}
				}
				break;

			case ProductSelectionMode::ProductSelectionMode_Wizard:
				{
					CDS_WizardSelParameter *pclCDSWizardSelParameters = pclProductSelectionParameters->m_pTADS->GetpWizardSelParameter();

					if( NULL != pclCDSWizardSelParameters )
					{
						SetRibbonComboSelPipeSeries( pclCDSWizardSelParameters->GetPipeSeriesID( pclProductSelectionParameters ) );
						SetRibbonComboSelPipeSize( pclCDSWizardSelParameters->GetPipeSizeID() );
					}
				}
				break;
		}
	}
}
