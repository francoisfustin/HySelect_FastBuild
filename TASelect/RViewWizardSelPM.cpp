#include "stdafx.h"


#include "TASelect.h"
#include "MainFrm.h"

#include "RViewWizardSelBase.h"
#include "SelectPM.h"
#include "ProductSelectionParameters.h"
#include "DlgLeftTabSelManager.h"

#include "DlgWizPMRightViewInputProjectType.h"
#include "DlgWizPMRightViewInputHeatTemp.h"
#include "DlgWizPMRightViewInputHeatTempAdv.h"
#include "DlgWizPMRightViewInputHeatPres.h"
#include "DlgWizPMRightViewInputHeatPresAdv.h"
#include "DlgWizPMRightViewInputHeatSystemVolumePower.h"
#include "DlgWizPMRightViewInputHeatSystemVolumePowerSWKI.h"
#include "DlgWizPMRightViewInputHeatGenerators.h"
#include "DlgWizPMRightViewInputHeatConsumers.h"
#include "DlgWizPMRightViewInputHeatPMType.h"
#include "DlgWizPMRightViewInputHeatPMType.h"
#include "DlgWizPMRightViewInputHeatRequ.h"
#include "DlgWizPMRightViewInputHeatRequ2.h"
#include "DlgWizPMRightViewInputCoolTemp.h"
#include "DlgWizPMRightViewInputCoolTempAdv.h"
#include "DlgWizPMRightViewInputCoolPres.h"
#include "DlgWizPMRightViewInputCoolPresAdv.h"
#include "DlgWizPMRightViewInputCoolSystemVolumePower.h"
#include "DlgWizPMRightViewInputCoolSystemVolumePowerSWKI.h"
#include "DlgWizPMRightViewInputCoolGenerators.h"
#include "DlgWizPMRightViewInputCoolConsumers.h"
#include "DlgWizPMRightViewInputCoolPMType.h"
#include "DlgWizPMRightViewInputCoolPMType.h"
#include "DlgWizPMRightViewInputCoolRequ.h"
#include "DlgWizPMRightViewInputCoolRequ2.h"
#include "DlgWizPMRightViewInputSolarTemp.h"
#include "DlgWizPMRightViewInputSolarTempAdv.h"
#include "DlgWizPMRightViewInputSolarPres.h"
#include "DlgWizPMRightViewInputSolarPresAdv.h"
#include "DlgWizPMRightViewInputSolarSystemVolumePower.h"
#include "DlgWizPMRightViewInputSolarSystemVolumePowerSWKI.h"
#include "DlgWizPMRightViewInputSolarSystemVolumePowerSWKIAdv.h"
#include "DlgWizPMRightViewInputSolarPMType.h"
#include "DlgWizPMRightViewInputSolarPMType.h"
#include "DlgWizPMRightViewInputSolarRequ.h"
#include "DlgWizPMRightViewInputSolarRequ2.h"
#include "RViewWizardSelPM.h"

#include "DlgWizPMRightViewInputBase.h"
#include "DlgWizPMRightViewPictureContainer.h"
#include "DlgWizPMRightViewTextContainer.h"


CRViewWizardSelPM *pRViewWizardSelPM = NULL;

CRViewWizardSelPM::CRViewWizardSelPM()
	:  CRViewWizardSelBase( CMainFrame::eRVWizardSelPM )
{
	m_pclWizardSelParams = NULL;
	m_pclDlgWizPM_RightViewInputBase = NULL;
	m_pclDlgWizPM_RVPictureContainer = NULL;
	m_pclDlgWizPM_RVTextContainer = NULL;
	pRViewWizardSelPM = this;

	ErrorStatusAndLink rErrorStatusAndLink;
	rErrorStatusAndLink.m_iErrorCode = 0;
	rErrorStatusAndLink.m_eMode = NAM_NoAdvancedMode;
	rErrorStatusAndLink.m_iNormalID = -1;
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::ProjectType] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Normal;
	rErrorStatusAndLink.m_iNormalID = -1;
	rErrorStatusAndLink.m_vecAdvIDList.push_back( CDlgWizardPM_RightViewInput_Base::HeatingTemperatureAdvance );
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::HeatingTemperature] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Advanced;
	rErrorStatusAndLink.m_iNormalID = CDlgWizardPM_RightViewInput_Base::HeatingTemperature;
	rErrorStatusAndLink.m_vecAdvIDList.clear();
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::HeatingTemperatureAdvance] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Normal;
	rErrorStatusAndLink.m_iNormalID = -1;
	rErrorStatusAndLink.m_vecAdvIDList.push_back( CDlgWizardPM_RightViewInput_Base::HeatingPressureAdvance );
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::HeatingPressure] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Advanced;
	rErrorStatusAndLink.m_iNormalID = CDlgWizardPM_RightViewInput_Base::HeatingPressure;
	rErrorStatusAndLink.m_vecAdvIDList.clear();
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::HeatingPressureAdvance] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Normal;
	rErrorStatusAndLink.m_iNormalID = -1;
	rErrorStatusAndLink.m_vecAdvIDList.push_back( CDlgWizardPM_RightViewInput_Base::HeatingGenerators );
	rErrorStatusAndLink.m_vecAdvIDList.push_back( CDlgWizardPM_RightViewInput_Base::HeatingConsumers );
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Advanced;
	rErrorStatusAndLink.m_iNormalID = CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower;
	rErrorStatusAndLink.m_vecAdvIDList.clear();
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::HeatingGenerators] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Advanced;
	rErrorStatusAndLink.m_iNormalID = -1;
	rErrorStatusAndLink.m_iNormalID = CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower;
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::HeatingConsumers] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_NoAdvancedMode;
	rErrorStatusAndLink.m_iNormalID = -1;
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::HeatingPressurisationMaintenanceType] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_NoAdvancedMode;
	rErrorStatusAndLink.m_iNormalID = -1;
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::HeatingRequirement] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_NoAdvancedMode;
	rErrorStatusAndLink.m_iNormalID = -1;
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::HeatingRequirement2] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Normal;
	rErrorStatusAndLink.m_iNormalID = -1;
	rErrorStatusAndLink.m_vecAdvIDList.push_back( CDlgWizardPM_RightViewInput_Base::CoolingTemperatureAdvance );
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::CoolingTemperature] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Advanced;
	rErrorStatusAndLink.m_iNormalID = CDlgWizardPM_RightViewInput_Base::CoolingTemperature;
	rErrorStatusAndLink.m_vecAdvIDList.clear();
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::CoolingTemperatureAdvance] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Normal;
	rErrorStatusAndLink.m_iNormalID = -1;
	rErrorStatusAndLink.m_vecAdvIDList.push_back( CDlgWizardPM_RightViewInput_Base::CoolingPressureAdvance );
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::CoolingPressure] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Advanced;
	rErrorStatusAndLink.m_iNormalID = CDlgWizardPM_RightViewInput_Base::CoolingPressure;
	rErrorStatusAndLink.m_vecAdvIDList.clear();
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::CoolingPressureAdvance] = rErrorStatusAndLink;
	
	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Normal;
	rErrorStatusAndLink.m_iNormalID = -1;
	rErrorStatusAndLink.m_vecAdvIDList.push_back( CDlgWizardPM_RightViewInput_Base::CoolingGenerators );
	rErrorStatusAndLink.m_vecAdvIDList.push_back( CDlgWizardPM_RightViewInput_Base::CoolingConsumers );
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::CoolingSystemVolumePower] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Advanced;
	rErrorStatusAndLink.m_iNormalID = CDlgWizardPM_RightViewInput_Base::CoolingSystemVolumePower;
	rErrorStatusAndLink.m_vecAdvIDList.clear();
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::CoolingGenerators] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Advanced;
	rErrorStatusAndLink.m_iNormalID = CDlgWizardPM_RightViewInput_Base::CoolingSystemVolumePower;
	rErrorStatusAndLink.m_vecAdvIDList.clear();
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::CoolingConsumers] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_NoAdvancedMode;
	rErrorStatusAndLink.m_iNormalID = -1;
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::CoolingPressurisationMaintenanceType] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_NoAdvancedMode;
	rErrorStatusAndLink.m_iNormalID = -1;
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::CoolingRequirement] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_NoAdvancedMode;
	rErrorStatusAndLink.m_iNormalID = -1;
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::CoolingRequirement2] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Normal;
	rErrorStatusAndLink.m_iNormalID = -1;
	rErrorStatusAndLink.m_vecAdvIDList.push_back( CDlgWizardPM_RightViewInput_Base::SolarTemperatureAdvance );
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::SolarTemperature] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Advanced;
	rErrorStatusAndLink.m_iNormalID = CDlgWizardPM_RightViewInput_Base::SolarTemperature;
	rErrorStatusAndLink.m_vecAdvIDList.clear();
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::SolarTemperatureAdvance] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Normal;
	rErrorStatusAndLink.m_iNormalID = -1;
	rErrorStatusAndLink.m_vecAdvIDList.push_back( CDlgWizardPM_RightViewInput_Base::SolarPressureAdvance );
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::SolarPressure] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Advanced;
	rErrorStatusAndLink.m_iNormalID = CDlgWizardPM_RightViewInput_Base::SolarPressure;
	rErrorStatusAndLink.m_vecAdvIDList.clear();
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::SolarPressureAdvance] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_NoAdvancedMode;
	rErrorStatusAndLink.m_iNormalID = -1;
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePower] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Normal;
	rErrorStatusAndLink.m_iNormalID = -1;
	rErrorStatusAndLink.m_vecAdvIDList.push_back( CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePowerSWKIAdvance );
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePowerSWKI] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_AdvancedMode_Advanced;
	rErrorStatusAndLink.m_iNormalID = CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePowerSWKI;
	rErrorStatusAndLink.m_vecAdvIDList.clear();
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePowerSWKIAdvance] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_NoAdvancedMode;
	rErrorStatusAndLink.m_iNormalID = -1;
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::SolarPressurisationMaintenanceType] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_NoAdvancedMode;
	rErrorStatusAndLink.m_iNormalID = -1;
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::SolarRequirement] = rErrorStatusAndLink;

	rErrorStatusAndLink.m_eMode = NAM_NoAdvancedMode;
	rErrorStatusAndLink.m_iNormalID = -1;
	m_mapInputValueDlgLinks[CDlgWizardPM_RightViewInput_Base::SolarRequirement2] = rErrorStatusAndLink;

	// List of all text.
	m_arTextIDSList.SetSize( CDlgWizardPM_RightViewInput_Base::LastElement );

	CArray<CTextIDValue> arTextIDSList;
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::Undefined].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::ProjectType].Copy( arTextIDSList );

	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_TEMP_TAZ1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_TEMP_TAZ2 ) );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::HeatingTemperature].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::SolarTemperature].Copy( arTextIDSList );

	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_HEATTEMP_SUPPLY1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_HEATTEMP_SUPPLY2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_HEATTEMP_RETURN1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_HEATTEMP_RETURN2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_HEATTEMP_MIN1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_HEATTEMP_MIN2, _U_TEMPERATURE, 5.0, _U_TEMPERATURE, -30.0 ) );
	// HYS-1054: Add text description for fill temperature for heating and solar system
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_TEMP_FILL1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_TEMP_FILL2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_TEMP_WATERCHAR1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_TEMP_WATERCHAR2 ) );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::HeatingTemperatureAdvance].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::SolarTemperatureAdvance].Copy( arTextIDSList );

	arTextIDSList.RemoveAll();
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PRESS_STATICHEIGHT1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PRESS_STATICHEIGHT2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PRESS_PZ1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PRESS_PZ2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PRESS_PSV1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PRESS_PSV2 ) );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::HeatingPressure].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::CoolingPressure].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::SolarPressure].Copy( arTextIDSList );

	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PRESS_PUMPHEAD1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PRESS_PUMPHEAD2 ) );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::HeatingPressureAdvance].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::CoolingPressureAdvance].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::SolarPressureAdvance].Copy( arTextIDSList );

	arTextIDSList.RemoveAll();
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_WATERVOLPOWER_VS1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_WATERVOLPOWER_VS2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_WATERVOLPOWER_POWER1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_WATERVOLPOWER_POWER2 ) );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::CoolingSystemVolumePower].Copy( arTextIDSList );

	arTextIDSList.RemoveAll();
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_HEATGENERATORS1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_HEATGENERATORS2 ) );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::HeatingGenerators].Copy( arTextIDSList );

	arTextIDSList.RemoveAll();
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_HEATCONSUMERS1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_HEATCONSUMERS2 ) );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::HeatingConsumers].Copy( arTextIDSList );

	arTextIDSList.RemoveAll();
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PMTYPE_PMTYPE1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PMTYPE_PMTYPE2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PMTYPE_WATERMAKEUPTYPE1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PMTYPE_WATERMAKEUPTYPE2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PMTYPE_WATERMAKEUPPN1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PMTYPE_WATERMAKEUPPN2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PMTYPE_WATERMAKEUPHARDNESS1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PMTYPE_WATERMAKEUPHARDNESS2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PMTYPE_DEGASSING1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_PMTYPE_DEGASSING2 ) );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::HeatingPressurisationMaintenanceType].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::CoolingPressurisationMaintenanceType].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::SolarPressurisationMaintenanceType].Copy( arTextIDSList );

	arTextIDSList.RemoveAll();
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::HeatingRequirement].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::HeatingRequirement2].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::CoolingRequirement].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::CoolingRequirement2].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::SolarRequirement].Copy( arTextIDSList );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::SolarRequirement2].Copy( arTextIDSList );

	arTextIDSList.RemoveAll();
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_TEMP_MAX1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_TEMP_MAX2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_COOLTEMP_SUPPLY1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_COOLTEMP_SUPPLY2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_COOLTEMP_RETURN1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_COOLTEMP_RETURN2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_TEMP_FILL1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_TEMP_FILL2 ) );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::CoolingTemperature].Copy( arTextIDSList );

	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_COOLTEMP_MIN1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_COOLTEMP_MIN2, _U_TEMPERATURE, -30.0 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_TEMP_WATERCHAR1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_TEMP_WATERCHAR2 ) );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::CoolingTemperatureAdvance].Copy( arTextIDSList );

	arTextIDSList.RemoveAll();
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_COLDGENERATORS1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_COLDGENERATORS2 ) );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::CoolingGenerators].Copy( arTextIDSList );

	arTextIDSList.RemoveAll();
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_COLDCONSUMERS1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_COLDCONSUMERS2 ) );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::CoolingConsumers].Copy( arTextIDSList );

	arTextIDSList.RemoveAll();
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_WATERVOLPOWER_VS1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_WATERVOLPOWER_VS2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_WATERVOLPOWER_VK1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_WATERVOLPOWER_VK2 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_WATERVOLPOWER_POWER1 ) );
	arTextIDSList.Add( CTextIDValue( IDS_DLGWIZARDPM_RVIEWTEXT_WATERVOLPOWER_POWER2 ) );
	m_arTextIDSList[CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePower].Copy( arTextIDSList );

	// List of all images.
	m_arImageIDBList.SetSize( CDlgWizardPM_RightViewInput_Base::LastElement );
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::Undefined] = -1;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::ProjectType] = -1;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::HeatingTemperature] = IDB_DLGWIZARDSELPM_HEATING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::HeatingTemperatureAdvance] = IDB_DLGWIZARDSELPM_HEATING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::HeatingPressure] = IDB_DLGWIZARDSELPM_HEATING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::HeatingPressureAdvance] = IDB_DLGWIZARDSELPM_HEATING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower] = IDB_DLGWIZARDSELPM_HEATING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::HeatingGenerators] = IDB_DLGWIZARDSELPM_HEATING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::HeatingConsumers] = IDB_DLGWIZARDSELPM_HEATING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::HeatingPressurisationMaintenanceType] = IDB_DLGWIZARDSELPM_HEATING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::HeatingRequirement] = IDB_DLGWIZARDSELPM_HEATING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::HeatingRequirement2] = IDB_DLGWIZARDSELPM_HEATING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::CoolingTemperature] = IDB_DLGWIZARDSELPM_COOLING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::CoolingTemperatureAdvance] = IDB_DLGWIZARDSELPM_COOLING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::CoolingPressure] = IDB_DLGWIZARDSELPM_COOLING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::CoolingPressureAdvance] = IDB_DLGWIZARDSELPM_COOLING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::CoolingSystemVolumePower] = IDB_DLGWIZARDSELPM_COOLING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::CoolingGenerators] = IDB_DLGWIZARDSELPM_COOLING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::CoolingConsumers] = IDB_DLGWIZARDSELPM_COOLING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::CoolingPressurisationMaintenanceType] = IDB_DLGWIZARDSELPM_COOLING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::CoolingRequirement] = IDB_DLGWIZARDSELPM_COOLING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::CoolingRequirement2] = IDB_DLGWIZARDSELPM_COOLING;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::SolarTemperature] = IDB_DLGWIZARDSELPM_SOLAR;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::SolarTemperatureAdvance] = IDB_DLGWIZARDSELPM_SOLAR;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::SolarPressure] = IDB_DLGWIZARDSELPM_SOLAR;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::SolarPressureAdvance] = IDB_DLGWIZARDSELPM_SOLAR;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePower] = IDB_DLGWIZARDSELPM_SOLAR;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePowerSWKI] = IDB_DLGWIZARDSELPM_SOLAR;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePowerSWKIAdvance] = IDB_DLGWIZARDSELPM_SOLAR;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::SolarPressurisationMaintenanceType] = IDB_DLGWIZARDSELPM_SOLAR;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::SolarRequirement] = IDB_DLGWIZARDSELPM_SOLAR;
	m_arImageIDBList[CDlgWizardPM_RightViewInput_Base::SolarRequirement2] = IDB_DLGWIZARDSELPM_SOLAR;
}

CRViewWizardSelPM::~CRViewWizardSelPM()
{
	if( NULL != m_pclDlgWizPM_RightViewInputBase )
	{
		if( NULL != m_pclDlgWizPM_RightViewInputBase->GetSafeHwnd() )
		{
			m_pclDlgWizPM_RightViewInputBase->DestroyWindow();
		}

		delete m_pclDlgWizPM_RightViewInputBase;
		m_pclDlgWizPM_RightViewInputBase = NULL;
	}

	if( NULL != m_pclDlgWizPM_RVPictureContainer )
	{
		if( NULL != m_pclDlgWizPM_RVPictureContainer->GetSafeHwnd() )
		{
			m_pclDlgWizPM_RVPictureContainer->DestroyWindow();
		}

		delete m_pclDlgWizPM_RVPictureContainer;
		m_pclDlgWizPM_RVPictureContainer = NULL;
	}
	
	if( NULL != m_pclDlgWizPM_RVTextContainer )
	{
		if( NULL != m_pclDlgWizPM_RVTextContainer->GetSafeHwnd() )
		{
			m_pclDlgWizPM_RVTextContainer->DestroyWindow();
		}

		delete m_pclDlgWizPM_RVTextContainer;
		m_pclDlgWizPM_RVTextContainer = NULL;
	}

	pRViewWizardSelPM = NULL;
}

void CRViewWizardSelPM::SetApplicationBackground( COLORREF cBackColor )
{
	m_pclDlgWizPM_RVTextContainer->SetApplicationBackground( cBackColor );
}

bool CRViewWizardSelPM::IsEmpty()
{
	// TODO
	return true;
}

void CRViewWizardSelPM::OnInputChange( int iErrorCode, int iErrorMaskNormal, int iErrorMaskAdvanced )
{
	if( NULL == m_pclDlgWizPM_RightViewInputBase )
	{
		// Must never happen!
		return;
	}

	// 'm_iErrorCode' allow us to keep which dialogs have an error. In this case when user comes back on it we can directly set 
	// the focus on the input where the error is. And also we can directly go in the advanced mode if the error is there.
	int iRightViewInputDialog = m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID();
	int iErrorSetMask = 0;
	int iErrorClearMask = 0;

	if( NAM_NoAdvancedMode == m_mapInputValueDlgLinks[iRightViewInputDialog].m_eMode
			|| NAM_AdvancedMode_Normal == m_mapInputValueDlgLinks[iRightViewInputDialog].m_eMode )
	{
		iErrorSetMask = iErrorMaskNormal;
		iErrorClearMask = ~iErrorMaskNormal;
	}
	else
	{
		iErrorSetMask = iErrorMaskAdvanced;
		iErrorClearMask = ~iErrorMaskAdvanced;
	}

	// Set to 0 concerned errors bits.
	m_mapInputValueDlgLinks[iRightViewInputDialog ].m_iErrorCode &= iErrorClearMask;

	// Now set the bits that are in error.
	m_mapInputValueDlgLinks[iRightViewInputDialog ].m_iErrorCode |= ( iErrorCode & iErrorSetMask );
	int iTotalError = m_mapInputValueDlgLinks[iRightViewInputDialog].m_iErrorCode;

	// If the current view is a normal or an advanced view, we need to clear/set corresponding error bits for the other view.

	if( NAM_AdvancedMode_Normal == m_mapInputValueDlgLinks[iRightViewInputDialog].m_eMode && 0 != (int)m_mapInputValueDlgLinks[iRightViewInputDialog].m_vecAdvIDList.size() )
	{
		// Check all the dialogs for the advanced mode corresponding to the current normal mode.
		iErrorSetMask = iErrorMaskAdvanced;
		iErrorClearMask = ~iErrorMaskAdvanced;

		for( auto &iter : m_mapInputValueDlgLinks[iRightViewInputDialog].m_vecAdvIDList )
		{
			m_mapInputValueDlgLinks[iter].m_iErrorCode &= iErrorClearMask;
			m_mapInputValueDlgLinks[iter].m_iErrorCode |= ( iErrorCode & iErrorSetMask );
			iTotalError |= m_mapInputValueDlgLinks[iter].m_iErrorCode;
		}
	}
	else if( NAM_AdvancedMode_Advanced == m_mapInputValueDlgLinks[iRightViewInputDialog].m_eMode && 0 != (int)m_mapInputValueDlgLinks[iRightViewInputDialog].m_iNormalID )
	{
		// Check the normal dialog corresponding to the current advanced mode.
		iErrorSetMask = iErrorMaskNormal;
		iErrorClearMask = ~iErrorMaskNormal;

		int iNormalID = m_mapInputValueDlgLinks[iRightViewInputDialog].m_iNormalID;
		m_mapInputValueDlgLinks[iNormalID].m_iErrorCode &= iErrorClearMask;
		m_mapInputValueDlgLinks[iNormalID].m_iErrorCode |= ( iErrorCode & iErrorSetMask );
		iTotalError |= m_mapInputValueDlgLinks[iNormalID].m_iErrorCode;
	}

	CRViewWizardSelBase::OnInputChange( iTotalError );
}

void CRViewWizardSelPM::FillPMInputUser()
{
	if( NULL != m_pclDlgWizPM_RightViewInputBase )
	{
		m_pclDlgWizPM_RightViewInputBase->FillPMInputUser();
	}
}

void CRViewWizardSelPM::ApplyPMInputUserUpdated( bool bWaterCharUpdated, bool bShowErrorMsg )
{
	if( NULL == m_pclDlgWizPM_RightViewInputBase )
	{
		// No current view opened, we exit.
		return;
	}

	m_pclDlgWizPM_RightViewInputBase->ApplyPMInputUserUpdated( bWaterCharUpdated, bShowErrorMsg );
}

BOOL CRViewWizardSelPM::Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext )
{
	// Create the form view.
	if( FALSE == CRViewWizardSelBase::Create( lpszClassName, lpszWindowName, dwStyle | WS_HSCROLL | WS_VSCROLL , rect, pParentWnd, nID, pContext ) )
	{
		return FALSE;
	}

	m_pclDlgWizPM_RVPictureContainer = new CDlgWizardPM_RightViewPictureContainer();
	
	if( NULL == m_pclDlgWizPM_RVPictureContainer )
	{
		ASSERTA_RETURN( FALSE );
	}
	
	if( FALSE == m_pclDlgWizPM_RVPictureContainer->Create( IDD_DLGWIZPM_RVIEWPICTURECONTAINER, this ) )
	{
		return FALSE;
	}

	m_pclDlgWizPM_RVTextContainer = new CDlgWizardPM_RightViewTextContainer();
	
	if( NULL == m_pclDlgWizPM_RVTextContainer )
	{
		ASSERTA_RETURN( FALSE );
	}

	if( FALSE == m_pclDlgWizPM_RVTextContainer->Create( IDD_DLGWIZPM_RVIEWTEXTCONTAINER, this ) )
	{
		return FALSE;
	}

	return TRUE;
}

void CRViewWizardSelPM::DetachCurrentInputDialog()
{
	// Because 'AttachNewInputDialog' is always called after this method, we can block redraw here and unblock it at the end
	// of the 'AttachNewInputDialog' to avoid blinking problem.
	SetRedraw( FALSE );

	if( NULL == m_pclDlgWizPM_RightViewInputBase )
	{
		// Except in this where we are at the result steps.
		SetRedraw( TRUE );
		return;
	}

	// Retrieve value from the dialog.
	m_pclDlgWizPM_RightViewInputBase->FillPMInputUser();

	if( NULL != m_pclDlgWizPM_RightViewInputBase->GetSafeHwnd() )
	{
		m_pclDlgWizPM_RightViewInputBase->DestroyWindow();
	}

	delete m_pclDlgWizPM_RightViewInputBase;
	m_pclDlgWizPM_RightViewInputBase = NULL;
}

void CRViewWizardSelPM::AttachNewInputDialog( CWizardSelPMParams *pclWizardSelParams, int iRightViewInputdialog, bool bBackEnable, bool bNextEnable )
{
	m_pclWizardSelParams = pclWizardSelParams;

	if( NULL != m_pclDlgWizPM_RightViewInputBase && iRightViewInputdialog == m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID() )
	{
		// Do nothing if it's already the same.
		SetRedraw( TRUE );
		return;
	}

	// Check first if we have already errors and focus on the good input value dialog (the one that has the error, for example the advanced mode).
	if( NULL != m_pclWizardSelParams &&	NAM_AdvancedMode_Normal == m_mapInputValueDlgLinks[iRightViewInputdialog].m_eMode 
			&& 0 == m_mapInputValueDlgLinks[iRightViewInputdialog].m_iErrorCode )
	{
		// Check all the dialogs for the advanced mode for the current step.
		for( auto &iter : m_mapInputValueDlgLinks[iRightViewInputdialog].m_vecAdvIDList )
		{
			if( 0 != m_mapInputValueDlgLinks[iter].m_iErrorCode )
			{
				// We found error in the advanced mode. We go in this mode.
				iRightViewInputdialog = iter;
				break;
			}
		}
	}

	// Check now special mode (For example, if user clicks on 'Water volume/Power' and if he has input detailed generators and/or consumers, we
	// go in this case directly in the advanced panel).
	if( NULL != m_pclWizardSelParams->m_pclSelectPMList && NULL != m_pclWizardSelParams->m_pclSelectPMList->GetpclPMInputUser()
			&& true == m_pclWizardSelParams->m_pclSelectPMList->GetpclPMInputUser()->IfSysVolExtDefExist() )
	{
		if( CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower == iRightViewInputdialog )
		{
			iRightViewInputdialog = CDlgWizardPM_RightViewInput_Base::HeatingGenerators;
		}
		else if( CDlgWizardPM_RightViewInput_Base::CoolingSystemVolumePower == iRightViewInputdialog )
		{
			iRightViewInputdialog = CDlgWizardPM_RightViewInput_Base::CoolingGenerators;
		}
	}
	
	UINT uiID = _CreateRightViewInputDialog( iRightViewInputdialog, &m_pclDlgWizPM_RightViewInputBase );
	
	if( NULL == m_pclDlgWizPM_RightViewInputBase )
	{
		SetRedraw( TRUE );
		return;
	}

	m_pclDlgWizPM_RightViewInputBase->Create( uiID, this );
	
	if( NULL == m_pclDlgWizPM_RightViewInputBase->GetSafeHwnd() )
	{
		delete m_pclDlgWizPM_RightViewInputBase;
		m_pclDlgWizPM_RightViewInputBase = NULL;
		SetRedraw( TRUE );
		return;
	}

	m_pclDlgWizPM_RightViewInputBase->EnableBackButton( bBackEnable );
	m_pclDlgWizPM_RightViewInputBase->EnableNextButton( bNextEnable );

	CRect rect;
	GetClientRect( &rect );
	int iNewWidth = ( rect.Width() <= RVIEWWIZARDSELPM_MINWIDTH ) ? RVIEWWIZARDSELPM_MINWIDTH - RVIEWWIZARDSELPM_TEXTPICTCONTAINER_WIDTH : rect.Width() - RVIEWWIZARDSELPM_TEXTPICTCONTAINER_WIDTH;
	int iNewHeight = ( rect.Height() <= RVIEWWIZARDSELPM_MINHEIGHT ) ? RVIEWWIZARDSELPM_MINHEIGHT : rect.Height();
	m_pclDlgWizPM_RightViewInputBase->SetWindowPos( NULL, 0, 0, iNewWidth, iNewHeight, SWP_NOZORDER );

	SetRedraw( TRUE );
	
	// Update text.
	// Remark: Must be called after 'SetRedraw( TRUE )'. Otherwise even if the dialog has created static controls to 
	// display text, the WM_PAINT message will not be sent. And the dialog will be only painted when resizing for example.
	UpdateText( iRightViewInputdialog );
	UpdatePicture( iRightViewInputdialog );

	m_pclDlgWizPM_RightViewInputBase->ShowWindow( SW_SHOW );
}

void CRViewWizardSelPM::UpdatePicture( int iRightViewInputdialog )
{
	m_pclDlgWizPM_RVPictureContainer->UpdatePicture( m_arImageIDBList[iRightViewInputdialog] );
}

void CRViewWizardSelPM::UpdateText( int iRightViewInputdialog )
{
	m_pclDlgWizPM_RVTextContainer->UpdateText( &m_arTextIDSList[iRightViewInputdialog] );
}

bool CRViewWizardSelPM::IsAtLeastOneError( int iRightViewInputID )
{
	bool bAtLeastOneError = false;
	int iErrorMaskNormal = 0;
	int iErrorMaskAdvanced = 0;
	int iErrorSetMask = 0;
	int iErrorClearMask = 0;
	int iErrorCode = 0;

	// First: Check the current view. Can be a view without advanced mode, a normal view or an advanced view.

	if( NULL != m_pclDlgWizPM_RightViewInputBase && iRightViewInputID == m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID() )
	{
		// The right view for which we want to check if there are error is the current one. We don't recreate it and we 
		// directly ask to the current dialog.
		iErrorCode = m_pclDlgWizPM_RightViewInputBase->IsAtLeastOneError( iErrorMaskNormal, iErrorMaskAdvanced );
	}
	else
	{
		// The right view for which we want to check if there are error is not the current one. We need to temporary
		// create it, ask if error and delete it.
		CDlgWizardPM_RightViewInput_Base *pDlg = NULL;
		_CreateRightViewInputDialog( iRightViewInputID, &pDlg );

		if( NULL == pDlg )
		{
			return false;
		}

		iErrorCode = pDlg->IsAtLeastOneError( iErrorMaskNormal, iErrorMaskAdvanced );
		delete pDlg;
	}

	// Don't change anything if there was any verification.
	if( -1 != iErrorMaskNormal && -1 != iErrorMaskAdvanced )
	{
		if( NAM_NoAdvancedMode == m_mapInputValueDlgLinks[iRightViewInputID].m_eMode
				|| NAM_AdvancedMode_Normal == m_mapInputValueDlgLinks[iRightViewInputID].m_eMode )
		{
			iErrorSetMask = iErrorMaskNormal;
			iErrorClearMask = ~iErrorMaskNormal;
		}
		else
		{
			iErrorSetMask = iErrorMaskAdvanced;
			iErrorClearMask = ~iErrorMaskAdvanced;
		}

		// Set to 0 concerned errors bits.
		m_mapInputValueDlgLinks[iRightViewInputID ].m_iErrorCode &= iErrorClearMask;

		// Now set the bits that are in error.
		m_mapInputValueDlgLinks[iRightViewInputID].m_iErrorCode |= ( iErrorCode & iErrorSetMask );
		bAtLeastOneError = ( 0 != iErrorCode ) ? true : false;
	}

	// Second: We verify the views that are linked with the first one:
	//          - If the view above has no advanced mode, we don't verify anything more.
	//          - If the view above is the normal mode, we check also errors for the advanced views (a).
	//          - If the view above is the advanced mode, we check also errors for the normal view (b).

	if( NAM_AdvancedMode_Normal == m_mapInputValueDlgLinks[iRightViewInputID].m_eMode && 0 != (int)m_mapInputValueDlgLinks[iRightViewInputID].m_vecAdvIDList.size() )
	{
		// (a).
		for( auto &iter : m_mapInputValueDlgLinks[iRightViewInputID].m_vecAdvIDList )
		{
			iErrorCode = 0;

			if( NULL != m_pclDlgWizPM_RightViewInputBase && iRightViewInputID == iter )
			{
				// Same remark as above.
				iErrorCode = m_pclDlgWizPM_RightViewInputBase->IsAtLeastOneError( iErrorMaskNormal, iErrorMaskAdvanced );
			}
			else
			{
				// Same remark as above.
				CDlgWizardPM_RightViewInput_Base *pDlg = NULL;
				_CreateRightViewInputDialog( iter, &pDlg );

				if( NULL == pDlg )
				{
					return false;
				}

				iErrorCode = pDlg->IsAtLeastOneError( iErrorMaskNormal, iErrorMaskAdvanced );
				delete pDlg;
			}

			// Don't change anything if there was any verification.
			if( -1 != iErrorMaskNormal && -1 != iErrorMaskAdvanced )
			{
				iErrorSetMask = iErrorMaskAdvanced;
				iErrorClearMask = ~iErrorMaskAdvanced;

				m_mapInputValueDlgLinks[iter].m_iErrorCode &= iErrorClearMask;
				m_mapInputValueDlgLinks[iter].m_iErrorCode |= ( iErrorCode & iErrorSetMask );
				bAtLeastOneError = ( 0 != iErrorCode ) ? true : false;
			}
		}
	}
	else if( NAM_AdvancedMode_Advanced == m_mapInputValueDlgLinks[iRightViewInputID].m_eMode && 0 != (int)m_mapInputValueDlgLinks[iRightViewInputID].m_iNormalID )
	{
		// (b).
		iErrorCode = 0;
		int iNormalID = m_mapInputValueDlgLinks[iRightViewInputID].m_iNormalID;

		if( NULL != m_pclDlgWizPM_RightViewInputBase && iRightViewInputID == iNormalID )
		{
			// Same remark as above.
			iErrorCode = m_pclDlgWizPM_RightViewInputBase->IsAtLeastOneError( iErrorMaskNormal, iErrorMaskAdvanced );
		}
		else
		{
			// Same remark as above.
			CDlgWizardPM_RightViewInput_Base *pDlg = NULL;
			_CreateRightViewInputDialog( iNormalID, &pDlg );

			if( NULL == pDlg )
			{
				return false;
			}

			iErrorCode = pDlg->IsAtLeastOneError( iErrorMaskNormal, iErrorMaskAdvanced );
			delete pDlg;
		}

		// Don't change anything if there was any verification.
		if( -1 != iErrorMaskNormal && -1 != iErrorMaskAdvanced )
		{
			iErrorSetMask = iErrorMaskNormal;
			iErrorClearMask = ~iErrorMaskNormal;

			m_mapInputValueDlgLinks[iNormalID].m_iErrorCode &= iErrorClearMask;
			m_mapInputValueDlgLinks[iNormalID].m_iErrorCode |= ( iErrorCode & iErrorSetMask );
			bAtLeastOneError = ( 0 != iErrorCode ) ? true : false;
		}
	}

	return bAtLeastOneError;
}

BEGIN_MESSAGE_MAP( CRViewWizardSelPM, CRViewWizardSelBase )
	ON_WM_SIZE()
	ON_MESSAGE( WM_USER_RIGHTVIEWWIZPM_BACKNEXT, OnButtonBackNextClicked )
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

void CRViewWizardSelPM::OnSize( UINT nType, int cx, int cy )
{
	CRViewWizardSelBase::OnSize( nType, cx, cy );

	if( NULL != m_pclDlgWizPM_RVPictureContainer && NULL != m_pclDlgWizPM_RVPictureContainer->GetSafeHwnd() 
			&& NULL != m_pclDlgWizPM_RVTextContainer && NULL != m_pclDlgWizPM_RVTextContainer->GetSafeHwnd() )
	{
		// Retrieve current position to take into account scrolling and setting the new position of containers.
		int nCurrentPosX = GetScrollPos( SBS_HORZ );
		int nCurrentPosY = GetScrollPos( SBS_VERT );

		// Text container.
		int iNewX = ( cx <= RVIEWWIZARDSELPM_MINWIDTH ) ? RVIEWWIZARDSELPM_MINWIDTH - RVIEWWIZARDSELPM_TEXTPICTCONTAINER_WIDTH : cx - RVIEWWIZARDSELPM_TEXTPICTCONTAINER_WIDTH;
		iNewX -= nCurrentPosX;
		int iNewY = -nCurrentPosY;

		int iNewHeight = ( cy <= RVIEWWIZARDSELPM_MINHEIGHT ) ? ( RVIEWWIZARDSELPM_MINHEIGHT >> 1 ) : ( cy >> 1 );
		m_pclDlgWizPM_RVTextContainer->SetWindowPos( NULL, iNewX, iNewY, RVIEWWIZARDSELPM_TEXTPICTCONTAINER_WIDTH, iNewHeight, SWP_NOZORDER );

		// Picture container.
		iNewY = ( cy <= RVIEWWIZARDSELPM_MINHEIGHT ) ? ( RVIEWWIZARDSELPM_MINHEIGHT >> 1 ): ( cy >> 1 );
		iNewY -= nCurrentPosY;

		m_pclDlgWizPM_RVPictureContainer->SetWindowPos( NULL, iNewX, iNewY, RVIEWWIZARDSELPM_TEXTPICTCONTAINER_WIDTH, iNewHeight, SWP_NOZORDER );
	}

	if( NULL != m_pclDlgWizPM_RightViewInputBase->GetSafeHwnd() )
	{
		int iNewWidth = ( cx <= RVIEWWIZARDSELPM_MINWIDTH ) ? RVIEWWIZARDSELPM_MINWIDTH - RVIEWWIZARDSELPM_TEXTPICTCONTAINER_WIDTH : cx - RVIEWWIZARDSELPM_TEXTPICTCONTAINER_WIDTH;
		int iNewHeight = ( cy <= RVIEWWIZARDSELPM_MINHEIGHT ) ? RVIEWWIZARDSELPM_MINHEIGHT : cy;
		m_pclDlgWizPM_RightViewInputBase->SetWindowPos( NULL, -1, -1, iNewWidth, iNewHeight, SWP_NOZORDER | SWP_NOMOVE );
	}

	if( cx < RVIEWWIZARDSELPM_MINWIDTH || cy < RVIEWWIZARDSELPM_MINHEIGHT )
	{
		SIZE sizeTotal;
		sizeTotal.cx = RVIEWWIZARDSELPM_MINWIDTH; 
		sizeTotal.cy = RVIEWWIZARDSELPM_MINHEIGHT;

		SetScrollSizes( MM_TEXT, sizeTotal );
	}
}

LRESULT CRViewWizardSelPM::OnButtonBackNextClicked( WPARAM wParam, LPARAM lParam )
{
	// Before to notify the 'CDlgWizardSelPM' class, check if we need to internally manage. 
	// This is the case for example with HeatGenerators and HeatConsumers advanced mode.
	// 0 is for the back button and 1 for the next button.

	if( ( CDlgWizardPM_RightViewInput_Base::HeatingPressure == m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID() 
			|| CDlgWizardPM_RightViewInput_Base::HeatingPressureAdvance == m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID() )
			&& WM_RVWPMBN_Next == wParam )
	{
		DetachCurrentInputDialog();

		// Need to know if it's 'CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower' or 'CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePowerSWKI'.
		// It's depending of the current norm.
		int iDlgWizePMRightViewInputID = m_pNotificationHandlers->OnRViewWizSelPMGetDlgWizePMRightViewInputID( CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower );

		// If user has defined a system volume, we directly go in this mode.
		if( true == m_pclWizardSelParams->m_pclSelectPMList->GetpclPMInputUser()->IfSysVolExtDefExist() )
		{
			AttachNewInputDialog( m_pclWizardSelParams, CDlgWizardPM_RightViewInput_Base::HeatingGenerators, true, true );
		}
		else
		{
			AttachNewInputDialog( m_pclWizardSelParams, iDlgWizePMRightViewInputID, true, true );
		}

		// Here we are changing the step, we need to notify 'CDlgWizardSelPM' to activate the good one.
		if( NULL != m_pNotificationHandlers )
		{
			m_pNotificationHandlers->OnRViewWizSelPMActivateStepWithCurrentRightView( iDlgWizePMRightViewInputID );
		}
	}
	else if( CDlgWizardPM_RightViewInput_Base::HeatingGenerators == m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID()
			&& WM_RVWPMBN_Next == wParam )
	{
		DetachCurrentInputDialog();
		AttachNewInputDialog( m_pclWizardSelParams, CDlgWizardPM_RightViewInput_Base::HeatingConsumers, true, true );
	}
	else if( CDlgWizardPM_RightViewInput_Base::HeatingConsumers == m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID()
			&& WM_RVWPMBN_Back == wParam )
	{
		DetachCurrentInputDialog();
		AttachNewInputDialog( m_pclWizardSelParams, CDlgWizardPM_RightViewInput_Base::HeatingGenerators, true, true );
	}
	else if( CDlgWizardPM_RightViewInput_Base::HeatingPressurisationMaintenanceType == m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID()
			&& WM_RVWPMBN_Back == wParam )
	{
		DetachCurrentInputDialog();

		// Need to know if it's 'CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower' or 'CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePowerSWKI'.
		// It's depending of the current norm.
		int iDlgWizePMRightViewInputID = m_pNotificationHandlers->OnRViewWizSelPMGetDlgWizePMRightViewInputID( CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower );

		// If user has defined a system volume, we go back in this mode.
		if( true == m_pclWizardSelParams->m_pclSelectPMList->GetpclPMInputUser()->IfSysVolExtDefExist() )
		{
			AttachNewInputDialog( m_pclWizardSelParams, CDlgWizardPM_RightViewInput_Base::HeatingConsumers, true, true );
		}
		else
		{
			AttachNewInputDialog( m_pclWizardSelParams, iDlgWizePMRightViewInputID, true, true );
		}

		// Here we are changing the step, we need to notify 'CDlgWizardSelPM' to activate the good one.
		if( NULL != m_pNotificationHandlers )
		{
			m_pNotificationHandlers->OnRViewWizSelPMActivateStepWithCurrentRightView( iDlgWizePMRightViewInputID );
		}
	}
	else if( ( CDlgWizardPM_RightViewInput_Base::CoolingPressure == m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID() 
				|| CDlgWizardPM_RightViewInput_Base::CoolingPressureAdvance == m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID() )
			&& WM_RVWPMBN_Next == wParam )
	{
		DetachCurrentInputDialog();

		// Need to know if it's 'CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower' or 'CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePowerSWKI'.
		// It's depending of the current norm.
		int iDlgWizePMRightViewInputID = m_pNotificationHandlers->OnRViewWizSelPMGetDlgWizePMRightViewInputID( CDlgWizardPM_RightViewInput_Base::CoolingSystemVolumePower );

		// If user has defined a system volume, we directly go in this mode.
		if( true == m_pclWizardSelParams->m_pclSelectPMList->GetpclPMInputUser()->IfSysVolExtDefExist() )
		{
			AttachNewInputDialog( m_pclWizardSelParams, CDlgWizardPM_RightViewInput_Base::CoolingGenerators, true, true );
		}
		else
		{
			AttachNewInputDialog( m_pclWizardSelParams, iDlgWizePMRightViewInputID, true, true );
		}

		// Here we are changing the step, we need to notify 'CDlgWizardSelPM' to activate the good one.
		if( NULL != m_pNotificationHandlers )
		{
			m_pNotificationHandlers->OnRViewWizSelPMActivateStepWithCurrentRightView( iDlgWizePMRightViewInputID );
		}
	}
	else if( CDlgWizardPM_RightViewInput_Base::CoolingGenerators == m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID()
			&& WM_RVWPMBN_Next == wParam )
	{
		DetachCurrentInputDialog();
		AttachNewInputDialog( m_pclWizardSelParams, CDlgWizardPM_RightViewInput_Base::CoolingConsumers, true, true );
	}
	else if( CDlgWizardPM_RightViewInput_Base::CoolingConsumers == m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID()
			&& WM_RVWPMBN_Back == wParam )
	{
		DetachCurrentInputDialog();
		AttachNewInputDialog( m_pclWizardSelParams, CDlgWizardPM_RightViewInput_Base::CoolingGenerators, true, true );
	}
	else if( CDlgWizardPM_RightViewInput_Base::CoolingPressurisationMaintenanceType == m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID()
			&&	WM_RVWPMBN_Back == wParam )
	{
		DetachCurrentInputDialog();

		// Need to know if it's 'CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower' or 'CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePowerSWKI'.
		// It's depending of the current norm.
		int iDlgWizePMRightViewInputID = m_pNotificationHandlers->OnRViewWizSelPMGetDlgWizePMRightViewInputID( CDlgWizardPM_RightViewInput_Base::CoolingSystemVolumePower );

		// If user has defined a system volume, we go back in this mode.
		if( true == m_pclWizardSelParams->m_pclSelectPMList->GetpclPMInputUser()->IfSysVolExtDefExist() )
		{
			AttachNewInputDialog( m_pclWizardSelParams, CDlgWizardPM_RightViewInput_Base::CoolingConsumers, true, true );
		}
		else
		{
			AttachNewInputDialog( m_pclWizardSelParams, iDlgWizePMRightViewInputID, true, true );
		}

		// Here we are changing the step, we need to notify 'CDlgWizardSelPM' to activate the good one.
		if( NULL != m_pNotificationHandlers )
		{
			m_pNotificationHandlers->OnRViewWizSelPMActivateStepWithCurrentRightView( iDlgWizePMRightViewInputID );
		}
	}
	else
	{
		CRViewWizardSelBase::OnButtonBackNextClicked( wParam, lParam );
	}

	return 0;
}

BOOL CRViewWizardSelPM::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
 	return CRViewWizardSelBase::OnMouseWheel( nFlags, zDelta, pt );
}

void CRViewWizardSelPM::OnInitialUpdate( )
{
	CRViewWizardSelBase::OnInitialUpdate();

	if( NULL != m_pclDlgWizPM_RVPictureContainer )
	{
		m_pclDlgWizPM_RVPictureContainer->ShowWindow( SW_SHOW );
	}
	
	if( NULL != m_pclDlgWizPM_RVTextContainer )
	{
		m_pclDlgWizPM_RVTextContainer->ShowWindow( SW_SHOW );
	}
}

void CRViewWizardSelPM::OnActivateView( BOOL bActivate, CView *pActivateView, CView *pDeactiveView )
{
	if( TRUE == bActivate )
	{
		UpdateText( m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID() );
		UpdatePicture( m_pclDlgWizPM_RightViewInputBase->GetRightViewInputID() );
	}
}

UINT CRViewWizardSelPM::_CreateRightViewInputDialog( int iRightViewInputdialog, CDlgWizardPM_RightViewInput_Base **ppclDlgWizPM_RightViewInputBase )
{
	if( NULL == ppclDlgWizPM_RightViewInputBase )
	{
		return CDlgWizardPM_RightViewInput_Base::Undefined;
	}

	UINT uiID = CDlgWizardPM_RightViewInput_Base::Undefined;

	switch( iRightViewInputdialog )
	{
		case CDlgWizardPM_RightViewInput_Base::ProjectType:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_ProjectType( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_PROJECTTYPE;
			break;

		case CDlgWizardPM_RightViewInput_Base::HeatingTemperature:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_HeatingTemperature( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATTEMP;
			break;

		case CDlgWizardPM_RightViewInput_Base::HeatingTemperatureAdvance:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_HeatingTemperatureAdvance( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATTEMPADV;
			break;

		case CDlgWizardPM_RightViewInput_Base::HeatingPressure:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_HeatingPressure( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATPRES;
			break;

		case CDlgWizardPM_RightViewInput_Base::HeatingPressureAdvance:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_HeatingPressureAdvance(m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATPRESADV;
			break;

		case CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePower:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_HeatingSystemVolumePower( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATSYSTEMVOLUMEPOWER;
			break;

		case CDlgWizardPM_RightViewInput_Base::HeatingSystemVolumePowerSWKI:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_HeatingSystemVolumePowerSWKI( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATSYSTEMVOLUMEPOWERSWKI;
			break;

		case CDlgWizardPM_RightViewInput_Base::HeatingGenerators:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_HeatingGenerators( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATGENERATORS;
			break;

		case CDlgWizardPM_RightViewInput_Base::HeatingConsumers:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_HeatingConsumers( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATCONSUMERS;
			break;

		case CDlgWizardPM_RightViewInput_Base::HeatingPressurisationMaintenanceType:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_HeatingPMType( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATPMTYPE;
			break;

		case CDlgWizardPM_RightViewInput_Base::HeatingRequirement:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_HeatingRequirement( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATREQU;
			break;

		case CDlgWizardPM_RightViewInput_Base::HeatingRequirement2:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_HeatingRequirement2( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATREQU2;
			break;

		case CDlgWizardPM_RightViewInput_Base::CoolingTemperature:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_CoolingTemperature( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLTEMP;
			break;

		case CDlgWizardPM_RightViewInput_Base::CoolingTemperatureAdvance:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_CoolingTemperatureAdvance( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLTEMPADV;
			break;

		case CDlgWizardPM_RightViewInput_Base::CoolingPressure:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_CoolingPressure( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLPRES;
			break;

		case CDlgWizardPM_RightViewInput_Base::CoolingPressureAdvance:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_CoolingPressureAdvance( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLPRESADV;
			break;

		case CDlgWizardPM_RightViewInput_Base::CoolingSystemVolumePower:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_CoolingSystemVolumePower( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLSYSTEMVOLUMEPOWER;
			break;

		case CDlgWizardPM_RightViewInput_Base::CoolingSystemVolumePowerSWKI:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_CoolingSystemVolumePowerSWKI( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLSYSTEMVOLUMEPOWERSWKI;
			break;

		case CDlgWizardPM_RightViewInput_Base::CoolingGenerators:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_CoolingGenerators( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLGENERATORS;
			break;

		case CDlgWizardPM_RightViewInput_Base::CoolingConsumers:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_CoolingConsumers( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLCONSUMERS;
			break;

		case CDlgWizardPM_RightViewInput_Base::CoolingPressurisationMaintenanceType:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_CoolingPMType( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLPMTYPE;
			break;

		case CDlgWizardPM_RightViewInput_Base::CoolingRequirement:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_CoolingRequirement( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLREQU;
			break;

		case CDlgWizardPM_RightViewInput_Base::CoolingRequirement2:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_CoolingRequirement2( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLREQU2;
			break;

		case CDlgWizardPM_RightViewInput_Base::SolarTemperature:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_SolarTemperature( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_SOLARTEMP;
			break;

		case CDlgWizardPM_RightViewInput_Base::SolarTemperatureAdvance:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_SolarTemperatureAdvance( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_SOLARTEMPADV;
			break;

		case CDlgWizardPM_RightViewInput_Base::SolarPressure:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_SolarPressure( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_SOLARPRES;
			break;

		case CDlgWizardPM_RightViewInput_Base::SolarPressureAdvance:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_SolarPressureAdvance( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_SOLARPRESADV;
			break;

		case CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePower:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_SolarSystemVolumePower( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_SOLARSYSTEMVOLUMEPOWER;
			break;

		case CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePowerSWKI:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKI( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_SOLARSYSTEMVOLUMEPOWERSWKI;
			break;

		case CDlgWizardPM_RightViewInput_Base::SolarSystemVolumePowerSWKIAdvance:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKIAdv( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_SOLARSYSTEMVOLUMEPOWERSWKIADV;
			break;

		case CDlgWizardPM_RightViewInput_Base::SolarPressurisationMaintenanceType:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_SolarPMType( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_SOLARPMTYPE;
			break;

		case CDlgWizardPM_RightViewInput_Base::SolarRequirement:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_SolarRequirement( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_SOLARREQU;
			break;

		case CDlgWizardPM_RightViewInput_Base::SolarRequirement2:
			*ppclDlgWizPM_RightViewInputBase = new CDlgWizardPM_RightViewInput_SolarRequirement2( m_pclWizardSelParams, this );
			uiID = IDD_DLGWIZPM_RIGHTVIEWINPUT_SOLARREQU2;
			break;
	}

	return uiID;
}
