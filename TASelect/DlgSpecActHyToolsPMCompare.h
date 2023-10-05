#pragma once


#include "DialogExt.h"
#include "afxwin.h"
#include "ProductSelectionParameters.h"
#include "TUProductSelectionHelper.h"

class CTUProdSelLauncher;
class CDlgSpecActHyToolsPMCompare : public CDialogExt
{
public:
	enum { IDD = IDD_DLGSPECACT_HYTOOLSPMCOMPARE };

	CDlgSpecActHyToolsPMCompare( CWnd *pParent = NULL );
	virtual ~CDlgSpecActHyToolsPMCompare();
	// HYS 1024: unit test hystoolPMCompare
	void SetInputSelParam();
	void ExportResults( CString strCompleteFileName );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg void OnBnClickedOpenInputFileName();
	virtual afx_msg void OnBnClickedStartCompare();
	virtual afx_msg void OnBnClickedStartExport();
	virtual afx_msg void OnBnClickedClear();

// Private methods.
private:
	void _SetDefaultValues( ProjectType eProjectType );

	void _LaunchTestMain();
	bool _LaunchTestStatico( CString strLine );
	bool _LaunchTestCompresso( CString strLine );
	bool _LaunchTestTransfero( CString strLine );

	void _LaunchExport();
	bool _LaunchExportStatico( CString strLine );
	bool _LaunchExportCompresso( CString strLine );
	bool _LaunchExportTransfero( CString strLine );

	bool _ReadInputs( );
	void _WriteInputs( CString &strOutput );

	void _WriteToListCtrl( CString strMsg );

	CString _ConvertDouble2String( double dValue, CString strReference );
	bool _IsDoubleValueDifferent( double dValue1, double dValue2, double dPrecision = 1e-14 );

// Private variables.
private:
	enum InputFields
	{
		IF_Emtpy1 = 0,
		IF_RegionId,
		IF_RegionCode,
		IF_SafetyTempLimiter,
		IF_SupplyTemperature,
		IF_ReturnTemperature,
		IF_MaxTemperature,
		IF_MinTemperature,
		IF_FillTemperature,
		IF_Norm,
		IF_PressureVolumeIndexEnabled,
		IF_StaticHeight,
		IF_PZ,
		IF_PSV,
		IF_PressurisationSide,
		IF_PumpHead,
		IF_VolumeSystem,
		IF_SolarCollectorContent,
		IF_InstalledPower,
		IF_PressureMaintenanceEnabled,
		IF_Degassing,
		IF_WaterMakeUp,
		IF_StaticPressureOfWaterNetwork,
		IF_MakeUpWaterTemperature,
		IF_WaterHardness,
		IF_ExpansionVesselBag,
		IF_PreferTecboxOnTop,
		IF_CompressoVesselInternalCoating,
		IF_ExternalCompressedAir,
		IF_PumpDegassingCoolingInsulation,
		IF_RedundancyPumpCompressor,
		IF_RedundancyCompleteTecbox,
		IF_WaterMakeUpBreakTankRequired,
		IF_WaterMakeUpDutyAndStandby,
		IF_MaxWidth,
		IF_MaxHeight,
		IF_Last
	};

	enum StaticoFields
	{
		SF_VesselName = IF_Last,
		SF_VesselID,
		SF_VesselNumber,
		SF_InitialPressure,
		SF_FillingPressure,
		SF_WaterReserve,
		SF_IntermediateVesselName,
		SF_IntermediateVesselID,
		SF_DegassingName,
		SF_DegassingID,
		SF_ContractionVolume,
		SF_ExpansionCoefficent,
		SF_ExpansionVolume,
		SF_MinimumWaterReserve,
		SF_VaporOverPressure,
		SF_MinimumPSV,
		SF_MinimumInitialPressure,
		SF_MinimumPressure,
		SF_FinalPressure,
		SF_PressureFactor,
		SF_NominalVolume,
		SF_Last
	};

	enum CompressoFields
	{
		CF_CompressoName = IF_Last,
		CF_CompressoID,
		CF_CompressoVesselName,
		CF_CompressoVesselID,
		CF_CompressoSecondaryVesselName,
		CF_CompressoSecondaryVesselID,
		CF_CompressoSecondaryVesselNumber,
		CF_IntermediateVesselName,
		CF_IntermediateVesselID,
		CF_DegassingName,
		CF_DegassingID,
		CF_ExpansionCoefficient,
		CF_ExpansionVolume,
		CF_MinimumWaterReserve,
		CF_VaporOverPressure,
		CF_MinimumPressure,
		CF_MinimumPSV,
		CF_MinimumInitialPressure,
		CF_TargetPressure,
		CF_FinalPressure,
		CF_PressureFactor,
		CF_NominalVolume,
		CF_EqualisationFlow,
		CF_Last
	};

	enum TransferoFields
	{
		TF_TransferoName = IF_Last,
		TF_TransferoID,
		TF_TransferoVesselName,
		TF_TransferoVesselID,
		TF_TransferoSecondaryVesselName,
		TF_TransferoSecondaryVesselID,
		TF_TransferoSecondaryVesselNumber,
		TF_IntermediateVesselName,
		TF_IntermediateVesselID,
		TF_ExpansionCoefficient,
		TF_ExpansionVolume,
		TF_MinimumWaterReserve,
		TF_VaporOverPressure,
		TF_MinimumPressure,
		TF_MinimumPSV,
		TF_MinimumInitialPressure,
		TF_TargetPressure,
		TF_FinalPressure,
		TF_PressureFactor,
		TF_NominalVolume,
		TF_EqualisationFlow,
		TF_Last
	};

	CEdit m_InputFileName;
	CListBox m_List;
	CIndSelPMParams m_clIndSelPMParams;
	CTUProdSelFileHelper m_clInputFile;
	CTUProdSelFileHelper m_clOutputFile;
	CTUProdSelFileHelper m_clReportFile;
	std::map<CString, CString> m_mapHyToolsNorm2HySelect;
	std::vector<CString> m_vecStrings;
	int m_iLargestText;

	// Variable not in the 'PMWQSelectionPreferences'.
	bool m_bPreferTecboxOnTop;
	bool m_bPreferVesselBag;
};
