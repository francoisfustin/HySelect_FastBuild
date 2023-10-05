#include "stdafx.h"
#include "MainFrm.h"
#include "TASelect.h"
#include "SelectPM.h"
#include "PMSelectionHelper.h"


CPMSelectionHelper::CPMSelectionHelper()
{
	m_bInitialized = false;
	m_eLastSelectionMode = LSM_NotYet;

	std::map<ProjectType, CSelectPMList *> mapPMList;
	mapPMList.insert( std::pair<ProjectType, CSelectPMList *>( ProjectType::Heating, &m_clIndividualSelectionHeatingSelectPMList) );
	mapPMList.insert( std::pair<ProjectType, CSelectPMList *>( ProjectType::Cooling, &m_clIndividualSelectionCoolingSelectPMList) );
	mapPMList.insert( std::pair<ProjectType, CSelectPMList *>( ProjectType::Solar, &m_clIndividualSelectionSolarSelectPMList) );
	m_mapSelectPMLists.insert( std::pair<ProductSelectionMode, std::map<ProjectType, CSelectPMList *>>( ProductSelectionMode::ProductSelectionMode_Individual, mapPMList ) );

	mapPMList.clear();
	mapPMList.insert( std::pair<ProjectType, CSelectPMList *>( ProjectType::Heating, &m_clWizardSelectionHeatingSelectPMList) );
	mapPMList.insert( std::pair<ProjectType, CSelectPMList *>( ProjectType::Cooling, &m_clWizardSelectionCoolingSelectPMList) );
	mapPMList.insert( std::pair<ProjectType, CSelectPMList *>( ProjectType::Solar, &m_clWizardSelectionSolarSelectPMList) );
	m_mapSelectPMLists.insert( std::pair<ProductSelectionMode, std::map<ProjectType, CSelectPMList *>>( ProductSelectionMode::ProductSelectionMode_Wizard, mapPMList ) );
}

CSelectPMList *CPMSelectionHelper::GetSelectPMList( ProductSelectionMode eProducSelectionMode )
{
	ProjectType eProjectType = TASApp.GetpTADS()->GetpTechParams()->GetProductSelectionApplicationType();
	return GetSelectPMList( eProducSelectionMode, eProjectType );
}

CSelectPMList *CPMSelectionHelper::GetSelectPMList( ProductSelectionMode eProducSelectionMode, ProjectType eProjectType )
{
	if( false == m_bInitialized )
	{
		ResetAll();
	}

	if( 0 == m_mapSelectPMLists.count( eProducSelectionMode ) 
			|| 0 == m_mapSelectPMLists.at( eProducSelectionMode ).count( eProjectType ) )
	{
		return NULL;
	}

	return m_mapSelectPMLists.at(eProducSelectionMode).at( eProjectType );
}

CPMInputUser *CPMSelectionHelper::GetPMInputUser( ProductSelectionMode eProducSelectionMode )
{
	CPMInputUser *pclPMInputUser = NULL;
	CSelectPMList *pclSelectPMList = GetSelectPMList( eProducSelectionMode );

	if( NULL != pclSelectPMList )
	{
		pclPMInputUser = pclSelectPMList->GetpclPMInputUser();
	}

	return pclPMInputUser;
}

CPMInputUser *CPMSelectionHelper::GetPMInputUser( ProductSelectionMode eProducSelectionMode, ProjectType eProjectType )
{
	if( false == m_bInitialized )
	{
		ResetAll();
	}

	CSelectPMList *pclSelectPMList = GetSelectPMList( eProducSelectionMode, eProjectType );
	CPMInputUser *pclPMInputUser = ( NULL != pclSelectPMList ) ? pclSelectPMList->GetpclPMInputUser() : NULL;
	return pclPMInputUser;
}

void CPMSelectionHelper::ResetAll()
{
	for( auto &iterSelectionMode : m_mapSelectPMLists )
	{
		for( auto &iterProjectType: iterSelectionMode.second )
		{
			Reset( iterSelectionMode.first, iterProjectType.first );

			iterProjectType.second->SetpTADB( TASApp.GetpTADB() );
			iterProjectType.second->SetpTADS( TASApp.GetpTADS() );

			if( NULL == iterProjectType.second->GetpclPMInputUser() )
			{
				ASSERT_RETURN;
			}
				
			iterProjectType.second->GetpclPMInputUser()->SetpTADB( TASApp.GetpTADB() );
			iterProjectType.second->GetpclPMInputUser()->SetpTADS( TASApp.GetpTADS() );
		}
	}

	m_bInitialized = true;
}

void CPMSelectionHelper::Reset( ProductSelectionMode eProducSelectionMode )
{
	ProjectType eProjectType = TASApp.GetpTADS()->GetpTechParams()->GetProductSelectionApplicationType();
	Reset( eProducSelectionMode, eProjectType );
}

void CPMSelectionHelper::Reset( ProductSelectionMode eProducSelectionMode, ProjectType eProjectType )
{
	if( 0 == m_mapSelectPMLists.count( eProducSelectionMode ) 
			|| 0 == m_mapSelectPMLists.at( eProducSelectionMode ).count( eProjectType) )
	{
		return;
	}

	CDS_TechnicalParameter *pTechParam = TASApp.GetpTADS()->GetpTechParams();
	ASSERT( NULL != pTechParam );

	if( NULL == pTechParam )
	{
		return;
	}

	CWaterChar *pWC = TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData();
	ASSERT( NULL != pWC );

	if( NULL == pWC )
	{
		return;
	}

	CPMInputUser *pclPMInputUser = m_mapSelectPMLists.at( eProducSelectionMode ).at( eProjectType )->GetpclPMInputUser();

	if( ProjectType::Heating == eProjectType )
	{
		pclPMInputUser->SetApplicationType( ProjectType::Heating );
		pclPMInputUser->SetPressureMaintenanceTypeID( _T("PMT_ALL_TAB") );
		pclPMInputUser->SetNormID( pTechParam->GetDefaultCalculationNorm().c_str() );
		pclPMInputUser->SetWaterMakeUpTypeID( _T("WMUP_TYPE_NONE") );
		pclPMInputUser->SetWaterMakeUpWaterTemp( pTechParam->GetDefaultWaterMakeUpTemp() );
		pclPMInputUser->SetFillTemperature( pTechParam->GetDefaultFillTemperature() );
		pclPMInputUser->SetWaterMakeUpNetworkPN( pTechParam->GetDefaultStaticPressureOfWaterNetwork() );
		pclPMInputUser->SetSafetyValveLocationChecked( BST_UNCHECKED );
		pclPMInputUser->SetSafetyValveLocation( 0.0 );
		pclPMInputUser->SetSafetyValveResponsePressure( pTechParam->GetDefaultSafetyPressureResponseValve() );
		pclPMInputUser->SetSafetyTempLimiter( pTechParam->GetDefaultTAZ() );
		pclPMInputUser->SetSupplyTemperature( pTechParam->GetDefaultISHeatingTps() );
		pclPMInputUser->SetReturnTemperature( pTechParam->GetDefaultISHeatingTpr() );
		pclPMInputUser->SetDegassingChecked( false );
		pclPMInputUser->SetDegassingPressureConnectPoint( 100000 );
		pclPMInputUser->SetStaticHeight( 0.0 );
		pclPMInputUser->SetPzChecked( BST_UNCHECKED );
		pclPMInputUser->SetPz( 0.0 );
		pclPMInputUser->SetSystemVolume( 5.0 );
		pclPMInputUser->SetInstalledPower( 10000.0 );
		pclPMInputUser->SetStorageTankVolume( 0.0 );
		pclPMInputUser->SetStorageTankMaxTemp( pTechParam->GetDefaultTAZ() ); 
		pclPMInputUser->SetPressOn( PressurON::poPumpSuction );
		pclPMInputUser->SetPumpHead( 0.0 );
		pclPMInputUser->SetMaxWidth( 0.0 );
		pclPMInputUser->SetMaxHeight( 0.0 );

		// In heating initialize minimum temperature with temperature defined in technical params or with the freezing point temperature.
		if( pTechParam->GetDefaultMinTempInHeating() > -273.0 && pTechParam->GetDefaultMinTempInHeating() > pWC->GetTfreez() )
		{
			pclPMInputUser->SetMinTemperature( pTechParam->GetDefaultMinTempInHeating() );
		}
		else
		{
			pclPMInputUser->SetMinTemperature( pWC->GetTfreez() );
		}

		// In case of heating/solar the max temperature for inlet of the degassing is the same as the return temperature.
		// Remark: Vento is always installed on the return pipe.
		pclPMInputUser->SetDegassingMaxTempConnectPoint( pTechParam->GetDefaultISHeatingTpr() );

		pclPMInputUser->SetWaterMakeUpWaterHardness( pTechParam->GetDefaultWaterHardness() );

		pclPMInputUser->GetHeatConsumersList()->Reset();
		pclPMInputUser->GetHeatGeneratorList()->Reset();
		pclPMInputUser->GetHeatingPipeList()->Reset();
	}
	else if( ProjectType::Cooling == eProjectType )
	{
		pclPMInputUser->SetApplicationType( ProjectType::Cooling );
		pclPMInputUser->SetPressureMaintenanceTypeID( _T("PMT_ALL_TAB") );
		pclPMInputUser->SetNormID( _T("") );
		pclPMInputUser->SetWaterMakeUpTypeID( _T("WMUP_TYPE_NONE") );
		pclPMInputUser->SetMaxTemperature( pTechParam->GetDefaultMaxTempInCooling() );
		pclPMInputUser->SetSupplyTemperature( pTechParam->GetDefaultISCoolingTps() );
		pclPMInputUser->SetReturnTemperature( pTechParam->GetDefaultISCoolingTpr() );
		pclPMInputUser->SetWaterMakeUpWaterTemp( pTechParam->GetDefaultFillTemperature() );
		pclPMInputUser->SetFillTemperature( pTechParam->GetDefaultFillTemperature() );
		pclPMInputUser->SetWaterMakeUpNetworkPN( pTechParam->GetDefaultStaticPressureOfWaterNetwork() );
		pclPMInputUser->SetWaterMakeUpWaterHardness( pTechParam->GetDefaultWaterHardness() );
		pclPMInputUser->SetSafetyValveLocationChecked( BST_UNCHECKED );
		pclPMInputUser->SetSafetyValveLocation( 0.0 );
		pclPMInputUser->SetSafetyValveResponsePressure( pTechParam->GetDefaultSafetyPressureResponseValve() );
		pclPMInputUser->SetDegassingChecked( false );
		pclPMInputUser->SetDegassingPressureConnectPoint( 100000 );
		pclPMInputUser->SetStaticHeight( 0.0 );
		pclPMInputUser->SetPzChecked( BST_UNCHECKED );
		pclPMInputUser->SetPz( 0.0 );
		pclPMInputUser->SetSystemVolume( 5.0 );
		pclPMInputUser->SetInstalledPower( 10000.0 );
		pclPMInputUser->SetStorageTankVolume( 0.0 );
		pclPMInputUser->SetStorageTankMaxTemp( pTechParam->GetDefaultTAZ() ); 
		pclPMInputUser->SetPressOn( PressurON::poPumpSuction );
		pclPMInputUser->SetPumpHead( 0.0 );
		pclPMInputUser->SetMaxWidth( 0.0 );
		pclPMInputUser->SetMaxHeight( 0.0 );

		// In case of cooling the max temperature for inlet of the degassing is the same as the return temperature.
		// Remark: Vento is always installed on the return pipe.
		pclPMInputUser->SetDegassingMaxTempConnectPoint( pclPMInputUser->GetReturnTemperature() );

		// HYS-1228: By default we check the 'Min. temp.' checkbox and set 5.0 °C.
		pclPMInputUser->SetMinTemperature( 5.0 );
		pclPMInputUser->SetCheckMinTemperature( true );

		pclPMInputUser->GetColdConsumersList()->Reset();
		pclPMInputUser->GetColdGeneratorList()->Reset();
		pclPMInputUser->GetCoolingPipeList()->Reset();
	}
	else if( ProjectType::Solar == eProjectType )
	{
		pclPMInputUser->SetApplicationType( ProjectType::Solar );
		pclPMInputUser->SetPressureMaintenanceTypeID( _T("PMT_ALL_TAB") );
		pclPMInputUser->SetNormID( _T( "" ) );
		pclPMInputUser->SetWaterMakeUpTypeID( _T("WMUP_TYPE_NONE") );
		pclPMInputUser->SetWaterMakeUpWaterTemp( pTechParam->GetDefaultWaterMakeUpTemp() );
		pclPMInputUser->SetWaterMakeUpNetworkPN( pTechParam->GetDefaultStaticPressureOfWaterNetwork() );
		pclPMInputUser->SetFillTemperature( pTechParam->GetDefaultFillTemperature() );
		pclPMInputUser->SetSafetyValveLocationChecked( BST_UNCHECKED );
		pclPMInputUser->SetSafetyValveLocation( 0.0 );
		pclPMInputUser->SetSafetyValveResponsePressure( pTechParam->GetDefaultSafetyPressureResponseValve() );
		pclPMInputUser->SetSafetyTempLimiter( pTechParam->GetDefaultTAZ() );
		pclPMInputUser->SetSupplyTemperature( pTechParam->GetDefaultISSolarTps() );
		pclPMInputUser->SetReturnTemperature( pTechParam->GetDefaultISSolarTpr() );
		pclPMInputUser->SetMinTemperature( pTechParam->GetDefaultMinTempInSolar() );
		pclPMInputUser->SetDegassingChecked( false );
		pclPMInputUser->SetDegassingPressureConnectPoint( 100000.0 );
		pclPMInputUser->SetStaticHeight( 0.0 );
		pclPMInputUser->SetPzChecked( BST_UNCHECKED );
		pclPMInputUser->SetPz( 0.0 );
		pclPMInputUser->SetSystemVolume( 0.166 );
		pclPMInputUser->SetSolarCollectorVolume( 0.016 );
		pclPMInputUser->SetSolarCollectorMultiplierFactorChecked( BST_UNCHECKED );
		pclPMInputUser->SetSolarCollectorMultiplierFactor( 2.0 );
		pclPMInputUser->SetInstalledPower( 10000.0 );
		pclPMInputUser->SetStorageTankVolume( 0.0 );
		pclPMInputUser->SetStorageTankMaxTemp( pTechParam->GetDefaultTAZ() ); 
		pclPMInputUser->SetPressOn( PressurON::poPumpSuction );
		pclPMInputUser->SetPumpHead( 0.0 );
		pclPMInputUser->SetMaxWidth( 0.0 );
		pclPMInputUser->SetMaxHeight( 0.0 );

		// In case of heating/solar the max temperature for inlet of the degassing is the same as the return temperature.
		// Remark: Vento is always installed on the return pipe.
		pclPMInputUser->SetDegassingMaxTempConnectPoint( pTechParam->GetDefaultISSolarTpr() );

		pclPMInputUser->SetWaterMakeUpWaterHardness( pTechParam->GetDefaultWaterHardness() );

		pclPMInputUser->GetHeatConsumersList()->Reset();
		pclPMInputUser->GetHeatGeneratorList()->Reset();
		pclPMInputUser->GetHeatingPipeList()->Reset();
	}
}

bool CPMSelectionHelper::VerifyPMWQPrefs( CProductSelelectionParameters * pclSelPMParams )
{
	// HYS-1537 : Can be use with wizard param
	CIndSelPMParams* pclIndSelPMparam = dynamic_cast<CIndSelPMParams*>( pclSelPMParams );
	CWizardSelPMParams* pclWizardSelPMparam = dynamic_cast<CWizardSelPMParams*>( pclSelPMParams );
	if( NULL == pclIndSelPMparam && NULL == pclWizardSelPMparam )
	{
		ASSERTA_RETURN( false );
	}

	CSelectPMList* pclSelectPMList = NULL;
	if( NULL != pclIndSelPMparam )
	{
		pclSelectPMList = pclIndSelPMparam->m_pclSelectPMList;
	}
	else
	{
		pclSelectPMList = pclWizardSelPMparam->m_pclSelectPMList;
	}

	if( NULL == pclSelectPMList || NULL == pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	if( false == m_clPMWQPrefsValidation.IsInitialized() )
	{
		_CheckDB( pclSelPMParams );
	}

	bool bReturn = false;
	CPMInputUser *pclPMInputUser = pclSelectPMList->GetpclPMInputUser();
	CPMWQPrefs *pclPMWQPrefs = pclPMInputUser->GetPMWQSelectionPreferences();

	CPMWQPrefs clPMWQPrefsCopy;
	clPMWQPrefsCopy.CopyFrom( pclPMWQPrefs );

	// pclPMWQPrefs->Reset();

	// Allow to save the state of button. If 'true' it means that the button has been already disabled and don't need
	// to be check at the end of this method.
	CPMWQPrefsValidation::PMWQPrefArray rPMWQPrefStates;
	ZeroMemory( &rPMWQPrefStates, sizeof( CPMWQPrefsValidation::PMWQPrefArray ) );

	// Check first if we are in heating, cooling or solar.
	ProjectType eProjectType = pclPMInputUser->GetApplicationType();

	if( ProjectType::Heating == eProjectType || ProjectType::Solar == eProjectType )
	{
		// In this case, we disable the cooling option for the pump and degassing and set the state to false by default.
		// HYS-1105: don't change the state of a button if it is disabled.
		pclPMWQPrefs->SetDisabled( CPMWQPrefs::PumpDegassingCoolingVersion, true );
		rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::PumpDegassingCoolingVersion] = true;
	}

	std::vector<int> vecToVerify;
	CString strWaterMakeUpID = pclPMInputUser->GetWaterMakeUpTypeID();
	bool bIsWaterMakeUpAvailable = ( 0 != strWaterMakeUpID.Compare( _T("WMUP_TYPE_NONE") ) ) ? true : false;

	if( CPMInputUser::MT_All == pclPMInputUser->GetPressureMaintenanceType() )
	{
		// HYS-1537 : Not to be verified in Solar
		if( Solar != pclPMInputUser->GetApplicationType() )
		{
			vecToVerify.push_back( CPMWQPrefsValidation::IV_Compresso );
			vecToVerify.push_back( CPMWQPrefsValidation::IV_Transfero );
			vecToVerify.push_back( CPMWQPrefsValidation::IV_TransferoTI );
		}
		if( BST_CHECKED == pclPMInputUser->GetDegassingChecked() )
		{
			vecToVerify.push_back( CPMWQPrefsValidation::IV_Vento );
		}

		// HYS-992: We don't disable anymore the 'PumpDegassingCoolingVersion' preference because we can now have Transfero TV
		//          with this option.

		if( true == bIsWaterMakeUpAvailable )
		{
			vecToVerify.push_back( CPMWQPrefsValidation::IV_Pleno );
		}
		else
		{
			pclPMWQPrefs->SetDisabled( CPMWQPrefs::WaterMakeupBreakTank, true );
			rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::WaterMakeupBreakTank] = true;

			pclPMWQPrefs->SetDisabled( CPMWQPrefs::WaterMakeupDutyStandBy, true );
			rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::WaterMakeupDutyStandBy] = true;
		}

		vecToVerify.push_back( CPMWQPrefsValidation::IV_Statico );
		vecToVerify.push_back( CPMWQPrefsValidation::IV_StaticoMembrane );
	}
	else if( CPMInputUser::MT_None == pclPMInputUser->GetPressureMaintenanceType() )
	{
		// Disable options regarding the Compresso selection.
		// HYS-1105: don't change the state of a button if it is disabled.
		pclPMWQPrefs->SetDisabled( CPMWQPrefs::CompressoInternalCoating, true );
		rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::CompressoInternalCoating] = true;

		// HYS-1105: don't change the state of a button if it is disabled.
		pclPMWQPrefs->SetDisabled( CPMWQPrefs::CompressoExternalAir, true );
		rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::CompressoExternalAir] = true;

		if( true == bIsWaterMakeUpAvailable )
		{
			vecToVerify.push_back( CPMWQPrefsValidation::IV_Pleno );
		}
		else
		{
			pclPMWQPrefs->SetDisabled( CPMWQPrefs::WaterMakeupBreakTank, true );
			rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::WaterMakeupBreakTank] = true;

			pclPMWQPrefs->SetDisabled( CPMWQPrefs::WaterMakeupDutyStandBy, true );
			rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::WaterMakeupDutyStandBy] = true;
		}

		if( BST_CHECKED == pclPMInputUser->GetDegassingChecked() )
		{
			vecToVerify.push_back( CPMWQPrefsValidation::IV_Vento );
		}
		else
		{
			pclPMWQPrefs->SetDisabled( CPMWQPrefs::PumpDegassingCoolingVersion, true );
			rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::PumpDegassingCoolingVersion] = true;
		}

		if( false == bIsWaterMakeUpAvailable && BST_UNCHECKED == pclPMInputUser->GetDegassingChecked() )
		{
			pclPMWQPrefs->SetDisabled( CPMWQPrefs::GeneralCombinedInOneDevice, true );
			rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::GeneralCombinedInOneDevice] = true;

			pclPMWQPrefs->SetDisabled( CPMWQPrefs::GeneralRedundancyPumpComp, true );
			rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::GeneralRedundancyPumpComp] = true;

			pclPMWQPrefs->SetDisabled( CPMWQPrefs::GeneralRedundancyTecBox, true );
			rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::GeneralRedundancyTecBox] = true;
		}
	}
	else if( CPMInputUser::MT_ExpansionVessel == pclPMInputUser->GetPressureMaintenanceType()
			|| CPMInputUser::MT_ExpansionVesselWithMembrane == pclPMInputUser->GetPressureMaintenanceType() )
	{
		// Disable options regarding the Compresso selection.
		// HYS-1105: don't change the state of a button if it is disabled.
		pclPMWQPrefs->SetDisabled( CPMWQPrefs::CompressoInternalCoating, true );
		rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::CompressoInternalCoating] = true;

		// HYS-1105: don't change the state of a button if it is disabled.
		pclPMWQPrefs->SetDisabled( CPMWQPrefs::CompressoExternalAir, true );
		rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::CompressoExternalAir] = true;

		if( CPMInputUser::MT_ExpansionVessel == pclPMInputUser->GetPressureMaintenanceType() )
		{
			vecToVerify.push_back( CPMWQPrefsValidation::IV_Statico );
		}
		else if( CPMInputUser::MT_ExpansionVesselWithMembrane == pclPMInputUser->GetPressureMaintenanceType() )
		{
			vecToVerify.push_back( CPMWQPrefsValidation::IV_StaticoMembrane );
		}

		if( true == bIsWaterMakeUpAvailable )
		{
			vecToVerify.push_back( CPMWQPrefsValidation::IV_Pleno );
		}
		else
		{
			pclPMWQPrefs->SetDisabled( CPMWQPrefs::WaterMakeupBreakTank, true );
			rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::WaterMakeupBreakTank] = true;

			pclPMWQPrefs->SetDisabled( CPMWQPrefs::WaterMakeupDutyStandBy, true );
			rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::WaterMakeupDutyStandBy] = true;
		}

		if( BST_CHECKED == pclPMInputUser->GetDegassingChecked() )
		{
			vecToVerify.push_back( CPMWQPrefsValidation::IV_Vento );
		}

		// HYS-992: We don't disable anymore the 'PumpDegassingCoolingVersion' preference because we can now have Transfero TV
		//          with this option.
	}
	else if( CPMInputUser::MT_WithCompressor == pclPMInputUser->GetPressureMaintenanceType() )
	{
		// Disable options regarding the Transfero selection.
		vecToVerify.push_back( CPMWQPrefsValidation::IV_Compresso );

		if( true == bIsWaterMakeUpAvailable )
		{
			vecToVerify.push_back( CPMWQPrefsValidation::IV_Pleno );
		}

		if( BST_CHECKED == pclPMInputUser->GetDegassingChecked() )
		{
			vecToVerify.push_back( CPMWQPrefsValidation::IV_Vento );
		}
	}
	else if( CPMInputUser::MT_WithPump == pclPMInputUser->GetPressureMaintenanceType() )
	{
		// Disable options regarding the Compresso selection.
		// HYS-1105: don't change the state of a button if it is disabled.
		pclPMWQPrefs->SetDisabled( CPMWQPrefs::CompressoInternalCoating, true );
		rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::CompressoInternalCoating] = true;

		// HYS-1105: don't change the state of a button if it is disabled.
		pclPMWQPrefs->SetDisabled( CPMWQPrefs::CompressoExternalAir, true );
		rPMWQPrefStates.m_arPMWQPrefs[CPMWQPrefs::CompressoExternalAir] = true;

		vecToVerify.push_back( CPMWQPrefsValidation::IV_Transfero );
		vecToVerify.push_back( CPMWQPrefsValidation::IV_TransferoTI );

		if( true == bIsWaterMakeUpAvailable )
		{
			vecToVerify.push_back( CPMWQPrefsValidation::IV_Pleno );
		}

		if( BST_CHECKED == pclPMInputUser->GetDegassingChecked() )
		{
			vecToVerify.push_back( CPMWQPrefsValidation::IV_Vento );
		}
	}

	if( 0 != (int)vecToVerify.size() )
	{
		int iFunctions = 0;

		if( BST_CHECKED == pclPMInputUser->GetDegassingChecked() )
		{
			iFunctions |= CPMWQPrefsValidation::DWFCT_Degassing;
		}

		if( true == bIsWaterMakeUpAvailable )
		{
			if( 0 == strWaterMakeUpID.Compare( _T("WMUP_TYPE_STD") ) )
			{
				iFunctions |= CPMWQPrefsValidation::DWFCT_WaterMakeUpStd;
			}
			else if( 0 == strWaterMakeUpID.Compare( _T("WMUP_TYPE_WITHSOFT" ) ) || 0 == strWaterMakeUpID.Compare( _T( "WMUP_TYPE_WITHDESAL") ) )
			{
				iFunctions |= CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal;
			}
		}

		// Loop on all options available and check for each TecBox to verify if there is a least one product that
		// exist for this option.
		for( int iLoopPref = CPMWQPrefs::PMWQP_First; iLoopPref < CPMWQPrefs::PMWQP_Last; iLoopPref++ )
		{
			bool bDisable = true;

			for( int iLoopTecBoxType = 0; iLoopTecBoxType < (int)vecToVerify.size(); iLoopTecBoxType++ )
			{
				if( CPMWQPrefs::GeneralCombinedInOneDevice == iLoopPref && CPMInputUser::MT_None != pclPMInputUser->GetPressureMaintenanceType() )
				{
					// If selection in a pressurisation system, the 'General combined in one device' option is only available for Compresso/Transfero.
					// Don't need to check thus the others functions.
					if( CPMWQPrefsValidation::IV_Vento == vecToVerify[iLoopTecBoxType] || CPMWQPrefsValidation::IV_Pleno == vecToVerify[iLoopTecBoxType] 
						|| CPMWQPrefsValidation::IV_Statico == vecToVerify[iLoopTecBoxType] || CPMWQPrefsValidation::IV_StaticoMembrane == vecToVerify[iLoopTecBoxType] )
					{
						continue;
					}
				}

				if( true == m_clPMWQPrefsValidation.GetValidateOptionFlag( (CPMWQPrefsValidation::IndexVector)vecToVerify[iLoopTecBoxType], iFunctions, (CPMWQPrefs::PMWQPreference)iLoopPref ) )
				{
					bDisable = false;
				}
			}

			if( false == rPMWQPrefStates.m_arPMWQPrefs[(CPMWQPrefs::PMWQPreference)iLoopPref] )
			{
				pclPMWQPrefs->SetDisabled( (CPMWQPrefs::PMWQPreference)iLoopPref, bDisable );

				// HYS-1105: don't change the state of a button if it is disabled. For example, if we are with compressor for the pressure maintenance type,
				// and we are going in the vessel pressure maintenance type, we disable the 'Internal coating' button. But we don't want that this button lost
				// its checked status. In this way and when we go back to the 'With compressor' maintenance type, we enable the button and its state is let to checked by default.
			}
		}

		// Special loop to check a button if it was previously enabled, checked and it is currently enabled.
		/*
		for( int iLoopPref = CPMWQPrefs::PMWQP_First; iLoopPref < CPMWQPrefs::PMWQP_Last; iLoopPref++ )
		{
			if( false == clPMWQPrefsCopy.IsDisabled( (CPMWQPrefs::PMWQPreference)iLoopPref )
					&& false == pclPMWQPrefs->IsDisabled( (CPMWQPrefs::PMWQPreference)iLoopPref ) 
					&& true == clPMWQPrefsCopy.IsChecked( (CPMWQPrefs::PMWQPreference)iLoopPref ) )
			{
				pclPMWQPrefs->SetChecked( (CPMWQPrefs::PMWQPreference)iLoopPref, true );
			}
		}
		*/

		bReturn = true;
	}

	return bReturn;
}

void CPMSelectionHelper::_CheckDB( CProductSelelectionParameters * pclSelPMParams )
{
	// HYS-1537 : to be also used with wizard param
	if( NULL == pclSelPMParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	m_clPMWQPrefsValidation.Init();

	_PrepareMapVentoAlone( pclSelPMParams );
	_PrepareMapPlenoAlone( pclSelPMParams );
	_PrepareMapPlenoTable( pclSelPMParams );

	// Check first the all TecBox.
	CTable *pTab = (CTable *)( pclSelPMParams->m_pTADB->Get( _T("TECBOX_TAB") ).MP );

	if( NULL == pTab )
	{
		return;
	}

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( IDPtr.MP );

		if( NULL == pclTecBox )
		{
			continue;
		}

		if( false == pclTecBox->IsSelectable( true ) )
		{
			continue;
		}

		CDB_TecBox::TecBoxType eTecBoxType = pclTecBox->GetTecBoxType();
		int iInternalDegassingFunction = 0;

		if( true == pclTecBox->IsFctDegassing() )
		{
			iInternalDegassingFunction |= CPMWQPrefsValidation::DWFCT_Degassing;
		}

		int iInternalWaterMakeUpFunction = 0;

		if( true == pclTecBox->IsFctWaterMakeUp() )
		{
			_CheckWaterMakeUpType( (CDB_Product*)pclTecBox, &iInternalWaterMakeUpFunction );
		}

		if( pclTecBox->GetNumberOfFunctions() > 1 )
		{
			_SetValidateOptionFlagHelper( (CDB_Product*)pclTecBox, iInternalDegassingFunction, iInternalWaterMakeUpFunction, CPMWQPrefs::GeneralCombinedInOneDevice, true );
		}

		// HYS-1121 : GeneralRedundancyPumpComp does not depend on water make-up or degassing
		if( CDB_TecBox::etbtCompresso == eTecBoxType )
		{
			if( pclTecBox->GetNbrPumpCompressor() > 1 )
			{
				// This option is always available for all type of TecBox.
				m_clPMWQPrefsValidation.SetValidateOptionFlag( CPMWQPrefsValidation::IV_Compresso, CPMWQPrefs::GeneralRedundancyPumpComp, true );
			}
			// HYS-1121 : CompressoExternalAir does not depend on water make-up or degassing
			if( true == pclTecBox->IsVariantExternalAir() )
			{
				m_clPMWQPrefsValidation.SetValidateOptionFlag( CPMWQPrefsValidation::IV_Compresso, CPMWQPrefs::CompressoExternalAir, true );
			}
		}
		if( ( CDB_TecBox::etbtTransferoTI == eTecBoxType ) && ( pclTecBox->GetNbrPumpCompressor() > 1 ) )
		{
			m_clPMWQPrefsValidation.SetValidateOptionFlag( CPMWQPrefsValidation::IV_TransferoTI, CPMWQPrefs::GeneralRedundancyPumpComp, true );
		}
		if( ( CDB_TecBox::etbtTransfero == eTecBoxType ) && ( pclTecBox->GetNbrPumpCompressor() > 1 ) )
		{
			m_clPMWQPrefsValidation.SetValidateOptionFlag( CPMWQPrefsValidation::IV_Transfero, CPMWQPrefs::GeneralRedundancyPumpComp, true );
		}

		// HYS-992: The 'DegassingCoolingVersion' option has been extended to Transfero (For example TV 6.1 EC).
		// And we want that this option be available even if the user has not checked the 'Degassing' option.
		if( true == pclTecBox->IsVariantCooling() && ( CDB_TecBox::etbtTransfero == pclTecBox->GetTecBoxType() 
				|| CDB_TecBox::etbtTransferoTI == pclTecBox->GetTecBoxType() ) )
		{
			// This is why we set '0' here for the degassing.
			_SetValidateOptionFlagHelper( (CDB_Product*)pclTecBox, 0, iInternalWaterMakeUpFunction, CPMWQPrefs::PumpDegassingCoolingVersion, true );

			// And we force to accept all water make-up functions (None, standard and soft/desal) if no integrated in the Transfero.
			if( 0 == iInternalWaterMakeUpFunction )
			{
				_SetValidateOptionFlagHelper( (CDB_Product*)pclTecBox, 0, CPMWQPrefsValidation::DWFCT_WaterMakeUpStd | CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal, 
						CPMWQPrefs::PumpDegassingCoolingVersion, true );
			}
		}

		if( 0 != iInternalDegassingFunction )
		{
			// If the degassing function is integrated in the TecBox, we directly check with flags.
			// It concerns Compresso, Transfero as well Vento.
			if( true == pclTecBox->IsVariantCooling() )
			{
				_SetValidateOptionFlagHelper( (CDB_Product*)pclTecBox, iInternalDegassingFunction, iInternalWaterMakeUpFunction, CPMWQPrefs::PumpDegassingCoolingVersion, true );

				// HYS-1094: And we force to accept all water make-up functions (None, standard and soft/desal) if no integrated in the Transfero.
				if( 0 == iInternalWaterMakeUpFunction )
				{
					_SetValidateOptionFlagHelper( (CDB_Product*)pclTecBox, iInternalDegassingFunction, CPMWQPrefsValidation::DWFCT_WaterMakeUpStd | CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal, 
							CPMWQPrefs::PumpDegassingCoolingVersion, true );
				}
			}
		}
		else
		{
			// If the degassing function in not integrated in the TecBox, we check with the Vento alone.
			// For degassing we have not a VentoID in the 'CDB_TecBox' object to link with a table of Vento.

			// Run on available Vento combinations.
			for( int iLoop = 0; iLoop < 2; iLoop++ )
			{
				int iTemp = ( 0 == iLoop ) ? 0 : CPMWQPrefsValidation::DWFCT_WaterMakeUpStd;

				if( 0 == m_mapVentoAlone.count( iTemp ) )
				{
					continue;
				}

				if( 1 == iLoop && ( CPMWQPrefsValidation::DWFCT_WaterMakeUpStd == ( CPMWQPrefsValidation::DWFCT_WaterMakeUpStd & iInternalWaterMakeUpFunction ) ) )
				{
					continue;
				}

				if( true == m_mapVentoAlone[iTemp].m_arPMWQPrefs[CPMWQPrefs::PumpDegassingCoolingVersion] )
				{
					m_clPMWQPrefsValidation.SetValidateOptionFlag( (CDB_Product*)pclTecBox, CPMWQPrefsValidation::DWFCT_Degassing | iTemp, CPMWQPrefs::PumpDegassingCoolingVersion, true );
				}
			}
		}

		if( 0 != iInternalWaterMakeUpFunction )
		{
			// If the water make-up function is integrated in the TecBox, we directly check with flags.
			// It concerns Compresso, Transfero as well Pleno.
			// HYS-1121 : these options will be active with or without degassing
			if( true == pclTecBox->IsVariantBreakTank() )
			{
				_SetValidateOptionFlagHelper( (CDB_Product*)pclTecBox, 0, iInternalWaterMakeUpFunction, CPMWQPrefs::WaterMakeupBreakTank, true );
				_SetValidateOptionFlagHelper( (CDB_Product*)pclTecBox, 1, iInternalWaterMakeUpFunction, CPMWQPrefs::WaterMakeupBreakTank, true );
			}

			if( pclTecBox->GetNbrPumpCompressor() > 1 )
			{
				_SetValidateOptionFlagHelper( (CDB_Product*)pclTecBox, 0, iInternalWaterMakeUpFunction, CPMWQPrefs::WaterMakeupDutyStandBy, true );
				_SetValidateOptionFlagHelper( (CDB_Product*)pclTecBox, 1, iInternalWaterMakeUpFunction, CPMWQPrefs::WaterMakeupDutyStandBy, true );
			}
		}
		else
		{
			// If the water make-up function in not integrated in the TecBox, we must check in a special way.
			// For water make-up we have a PlenoID in the 'CDB_TecBox' object to link with a table of Pleno.
			CTable *pclTable = dynamic_cast<CTable *>( (CData*)( pclTecBox->GetPlenoIDPtr().MP ) );

			if( NULL != pclTable && 0 != m_mapPlenoByTableID.count( CString( pclTable->GetIDPtr().ID ) ) )
			{
				CString strID = CString( pclTable->GetIDPtr().ID );

				// Run on available water make-up combinations.
				for( int iLoop = 0; iLoop < 2; iLoop++ )
				{
					int iTemp = ( 0 == iLoop ) ? CPMWQPrefsValidation::DWFCT_WaterMakeUpStd : CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal;

					if( 0 == m_mapPlenoByTableID[strID].count( iTemp ) )
					{
						continue;
					}

					if( true == m_mapPlenoByTableID[strID][iTemp].m_arPMWQPrefs[CPMWQPrefs::WaterMakeupBreakTank] )
					{
						_SetValidateOptionFlagHelper( (CDB_Product*)pclTecBox, iInternalDegassingFunction, iTemp, CPMWQPrefs::WaterMakeupBreakTank, true );
					}

					if( true == m_mapPlenoByTableID[strID][iTemp].m_arPMWQPrefs[CPMWQPrefs::WaterMakeupDutyStandBy] )
					{
						_SetValidateOptionFlagHelper( (CDB_Product*)pclTecBox, iInternalDegassingFunction, iTemp, CPMWQPrefs::WaterMakeupDutyStandBy, true );
					}
				}
			}
		}
	}

	// Check now all the vessels.
	pTab = (CTable *)( pclSelPMParams->m_pTADB->Get( _T("VSSL_TAB") ).MP );

	if( NULL == pTab )
	{
		return;
	}

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel *>( IDPtr.MP );

		if( NULL == pclVessel )
		{
			continue;
		}

		if( false == pclVessel->IsSelectable( true ) )
		{
			continue;
		}

		if( CDB_Vessel::ePMComp_Compresso == ( CDB_Vessel::ePMComp_Compresso & pclVessel->GetCompatibilityflags() ) &&
			CDB_Vessel::ePMComp_VsslIntCoating == ( CDB_Vessel::ePMComp_VsslIntCoating & pclVessel->GetCompatibilityflags() ) )
		{
			// Compresso vessel.
			m_clPMWQPrefsValidation.SetValidateOptionFlag( CPMWQPrefsValidation::IV_Compresso, CPMWQPrefs::CompressoInternalCoating, true );
			break;
		}
		else if( CDB_Vessel::ePMComp_Statico == ( CDB_Vessel::ePMComp_Statico & pclVessel->GetCompatibilityflags() ) )
		{
			// Statico bag and membrane.

			// Check with degassing (with Statico there is never integrated degassing device).
			// Run on available degassing combinations (alone or with a water make-up standard integrated).
			for( int iLoop = 0; iLoop < 2; iLoop++ )
			{
				int iTemp = ( 0 == iLoop ) ? 0 : CPMWQPrefsValidation::DWFCT_WaterMakeUpStd;

				if( 0 == m_mapVentoAlone.count( iTemp ) )
				{
					continue;
				}

				if( true == m_mapVentoAlone[iTemp].m_arPMWQPrefs[CPMWQPrefs::PumpDegassingCoolingVersion] )
				{
					m_clPMWQPrefsValidation.SetValidateOptionFlag( (CDB_Product*)pclVessel, CPMWQPrefsValidation::DWFCT_Degassing | iTemp, CPMWQPrefs::PumpDegassingCoolingVersion, true );
				}
			}

			// Check with water make-up (with Statico there is never integrated degassing device).
			CTable *pclTable = dynamic_cast<CTable *>( (CData*)( pclVessel->GetPlenoIDPtr().MP ) );

			if( NULL != pclTable && 0 != m_mapPlenoByTableID.count( CString( pclTable->GetIDPtr().ID ) ) )
			{
				CString strID = CString( pclTable->GetIDPtr().ID );

				// Run without and with degassing combinations.
				for( int iLoopDeg = 0; iLoopDeg < 2; iLoopDeg++ )
				{
					int iDegassing = ( 0 == iLoopDeg ) ? 0 : CPMWQPrefsValidation::DWFCT_Degassing;

					// Run on available water make-up combinations.
					for( int iLoopWMUp = 0; iLoopWMUp < 2; iLoopWMUp++ )
					{
						int iWaterMakeUp = ( 0 == iLoopWMUp ) ? CPMWQPrefsValidation::DWFCT_WaterMakeUpStd : CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal;

						if( 0 == m_mapPlenoByTableID[strID].count( iWaterMakeUp ) )
						{
							continue;
						}

						if( true == m_mapPlenoByTableID[strID][iWaterMakeUp].m_arPMWQPrefs[CPMWQPrefs::WaterMakeupBreakTank] )
						{
							m_clPMWQPrefsValidation.SetValidateOptionFlag( (CDB_Product*)pclVessel, iDegassing | iWaterMakeUp, CPMWQPrefs::WaterMakeupBreakTank, true );
						}

						if( true == m_mapPlenoByTableID[strID][iWaterMakeUp].m_arPMWQPrefs[CPMWQPrefs::WaterMakeupDutyStandBy] )
						{
							m_clPMWQPrefsValidation.SetValidateOptionFlag( (CDB_Product*)pclVessel, iDegassing | iWaterMakeUp, CPMWQPrefs::WaterMakeupDutyStandBy, true );
						}
					}
				}
			}
		}
	}

	// This option is always available for all type of TecBox.
	m_clPMWQPrefsValidation.SetValidateOptionFlag( CPMWQPrefsValidation::IV_Compresso, CPMWQPrefs::GeneralRedundancyTecBox, true );
	m_clPMWQPrefsValidation.SetValidateOptionFlag( CPMWQPrefsValidation::IV_Transfero, CPMWQPrefs::GeneralRedundancyTecBox, true );
	m_clPMWQPrefsValidation.SetValidateOptionFlag( CPMWQPrefsValidation::IV_TransferoTI, CPMWQPrefs::GeneralRedundancyTecBox, true );
	m_clPMWQPrefsValidation.SetValidateOptionFlag( CPMWQPrefsValidation::IV_Vento, CPMWQPrefs::GeneralRedundancyTecBox, true );
	m_clPMWQPrefsValidation.SetValidateOptionFlag( CPMWQPrefsValidation::IV_Pleno, CPMWQPrefs::GeneralRedundancyTecBox, true );

	// Check now option that can be copy in other combination.
	// For example: 'external air' option for the Compresso can be validated with degassing combination for example if this combination is not 
	//              a combined solution.
	if( true == m_clPMWQPrefsValidation.GetValidateOptionFlag( CPMWQPrefsValidation::IV_Compresso, CPMWQPrefsValidation::DWFCT_None, CPMWQPrefs::CompressoExternalAir ) )
	{
		m_clPMWQPrefsValidation.PopulateValidateOptionFlag( CPMWQPrefsValidation::IV_Compresso, CPMWQPrefs::CompressoExternalAir );
	}
}

void CPMSelectionHelper::_SetValidateOptionFlagHelper( CDB_Product *pclProduct, int iDegFunction, int iWMUpFunctions, CPMWQPrefs::PMWQPreference ePMWQPref, bool fSet )
{
	if( 0 == iWMUpFunctions )
	{
		m_clPMWQPrefsValidation.SetValidateOptionFlag( pclProduct, iDegFunction, ePMWQPref, fSet );
	}

	if( CPMWQPrefsValidation::DWFCT_WaterMakeUpStd == ( CPMWQPrefsValidation::DWFCT_WaterMakeUpStd & iWMUpFunctions ) )
	{
		m_clPMWQPrefsValidation.SetValidateOptionFlag( pclProduct, iDegFunction | CPMWQPrefsValidation::DWFCT_WaterMakeUpStd, ePMWQPref, fSet );
	}

	if( CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal == ( CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal & iWMUpFunctions ) )
	{
		m_clPMWQPrefsValidation.SetValidateOptionFlag( pclProduct, iDegFunction | CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal, ePMWQPref, fSet );
	}
}

void CPMSelectionHelper::_CheckWaterMakeUpType( CDB_Product *pclProduct, int *piFunctions )
{
	if( NULL == pclProduct || NULL == piFunctions )
	{
		return;
	}

	if( NULL != dynamic_cast<CDB_Vessel*>( pclProduct ) )
	{
		_CheckWMUpTable( pclProduct, piFunctions );
	}
	else if( NULL != dynamic_cast<CDB_TecBox*>( pclProduct ) )
	{
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox*>( pclProduct );

		if( CDB_TecBox::etbtPleno == pclTecBox->GetTecBoxType() && NULL != dynamic_cast<CDB_TBPlenoVento*>( pclTecBox ) )
		{
			CDB_TBPlenoVento *pclPleno = dynamic_cast<CDB_TBPlenoVento*>( pclTecBox );

			if( CDB_TBPlenoVento::ePMPR_RefillNone == pclPleno->GetRefillable() )
			{
				*piFunctions |= CPMWQPrefsValidation::DWFCT_WaterMakeUpStd;
			}
			else if ( CDB_TBPlenoVento::ePMPR_RefillMandatory == pclPleno->GetRefillable() )
			{
				*piFunctions |= CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal;
			}
			else
			{
				*piFunctions |= CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal | CPMWQPrefsValidation::DWFCT_WaterMakeUpStd;
			}
		}
		else if( CDB_TecBox::etbtVento == pclTecBox->GetTecBoxType() && NULL != dynamic_cast<CDB_TBPlenoVento*>( pclTecBox ) )
		{
			CDB_TBPlenoVento *pclVento = dynamic_cast<CDB_TBPlenoVento*>( pclTecBox );

			if( true == pclVento->IsFctWaterMakeUp() )
			{
				*piFunctions |= CPMWQPrefsValidation::DWFCT_WaterMakeUpStd;
			}
		}
		else if( CDB_TecBox::etbtCompresso == pclTecBox->GetTecBoxType() || CDB_TecBox::etbtTransfero == pclTecBox->GetTecBoxType() ||
			CDB_TecBox::etbtTransferoTI == pclTecBox->GetTecBoxType() )
		{
			if( true == pclTecBox->IsFctWaterMakeUp() )
			{
				*piFunctions |= CPMWQPrefsValidation::DWFCT_WaterMakeUpStd;
			}
			else
			{
				_CheckWMUpTable( pclProduct, piFunctions );
			}
		}
	}
}

void CPMSelectionHelper::_CheckWMUpTable( CDB_Product *pclProduct, int *piFunctions )
{
	if( NULL == pclProduct || NULL == dynamic_cast<CDB_Vessel*>( pclProduct )
			|| NULL == dynamic_cast<CDB_TecBox*>( pclProduct ) || NULL == piFunctions )
	{
		return;
	}

	CTable *pclTable = NULL;

	if( NULL != dynamic_cast<CDB_Vessel*>( pclProduct ) )
	{
		CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel*>( pclProduct );
		pclTable = dynamic_cast<CTable*>( (CData*)( pclVessel->GetPlenoIDPtr().MP ) );
	}
	else
	{
		CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox*>( pclProduct );
		pclTable = dynamic_cast<CTable*>( (CData*)( pclTecBox->GetPlenoIDPtr().MP ) );
	}

	if( NULL == pclTable || 0 == m_mapPlenoByTableID.count( CString( pclTable->GetIDPtr().ID ) ) )
	{
		return;
	}

	CString strID = CString( pclTable->GetIDPtr().ID );

	if( 0 != m_mapPlenoByTableID[strID].count( CPMWQPrefsValidation::DWFCT_WaterMakeUpStd ) )
	{
		*piFunctions |= CPMWQPrefsValidation::DWFCT_WaterMakeUpStd;
	}

	if( 0 != m_mapPlenoByTableID[strID].count( CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal ) )
	{
		*piFunctions |= CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal;
	}
}

void CPMSelectionHelper::_PrepareMapVentoAlone( CProductSelelectionParameters *pclSelPMParams )
{
	m_mapVentoAlone.clear();
	
	// HYS-1537 : To be also used with wizard param
	CTable *pTab = (CTable *)( pclSelPMParams->m_pTADB->Get( _T("TECBOX_TAB") ).MP );

	if( NULL == pTab )
	{
		return;
	}

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_TBPlenoVento *pclVento = dynamic_cast<CDB_TBPlenoVento *>( IDPtr.MP );

		if( NULL == pclVento )
		{
			continue;
		}

		if( false == pclVento->IsSelectable( true ) )
		{
			continue;
		}

		if( CDB_TecBox::etbtVento != pclVento->GetTecBoxType() )
		{
			continue;
		}

		int iVentoType = 0;

		if( true == pclVento->IsFctWaterMakeUp() )
		{
			iVentoType = CPMWQPrefsValidation::DWFCT_WaterMakeUpStd;
		}

		if( 0 == m_mapVentoAlone.count( iVentoType ) )
		{
			CPMWQPrefsValidation::PMWQPrefArray rPMWQPrefArray;
			ZeroMemory( &rPMWQPrefArray, sizeof( CPMWQPrefsValidation::PMWQPrefArray ) );
			m_mapVentoAlone[iVentoType] = rPMWQPrefArray;
		}

		if( true == pclVento->IsVariantCooling() )
		{
			m_mapVentoAlone[iVentoType].m_arPMWQPrefs[CPMWQPrefs::PumpDegassingCoolingVersion] = true;
		}

		if( CPMWQPrefsValidation::DWFCT_WaterMakeUpStd == iVentoType )
		{
			m_mapVentoAlone[iVentoType].m_arPMWQPrefs[CPMWQPrefs::GeneralCombinedInOneDevice] = true;

			if( true == pclVento->IsVariantBreakTank() )
			{
				m_mapVentoAlone[iVentoType].m_arPMWQPrefs[CPMWQPrefs::WaterMakeupBreakTank] = true;
			}

			if( pclVento->GetNbrPumpCompressor() > 1 )
			{
				m_mapVentoAlone[iVentoType].m_arPMWQPrefs[CPMWQPrefs::WaterMakeupDutyStandBy] = true;
			}
		}
	}
}

void CPMSelectionHelper::_PrepareMapPlenoAlone( CProductSelelectionParameters *pclSelPMParams )
{
	m_mapPlenoAlone.clear();

	// HYS-1537 : To be also used with wizard param
	CTable *pTab = (CTable *)( pclSelPMParams->m_pTADB->Get( _T("TECBOX_TAB") ).MP );

	if( NULL == pTab )
	{
		return;
	}

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_TBPlenoVento *pclPleno = dynamic_cast<CDB_TBPlenoVento *>( IDPtr.MP );

		if( NULL == pclPleno )
		{
			continue;
		}

		if( false == pclPleno->IsSelectable( true ) )
		{
			continue;
		}

		if( CDB_TecBox::etbtPleno != pclPleno->GetTecBoxType() )
		{
			continue;
		}

		int iPlenoType = ( CDB_TBPlenoVento::ePMPR_RefillNone == pclPleno->GetRefillable() ) ? CPMWQPrefsValidation::DWFCT_WaterMakeUpStd : CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal;

		if( 0 == m_mapPlenoAlone.count( iPlenoType ) )
		{
			CPMWQPrefsValidation::PMWQPrefArray rPMWQPrefArray;
			ZeroMemory( &rPMWQPrefArray, sizeof( CPMWQPrefsValidation::PMWQPrefArray ) );
			m_mapPlenoAlone[iPlenoType] = rPMWQPrefArray;
		}

		if( true == pclPleno->IsVariantBreakTank() )
		{
			m_mapPlenoAlone[iPlenoType].m_arPMWQPrefs[CPMWQPrefs::WaterMakeupBreakTank] = true;
		}

		if( pclPleno->GetNbrPumpCompressor() > 1 )
		{
			m_mapPlenoAlone[iPlenoType].m_arPMWQPrefs[CPMWQPrefs::WaterMakeupDutyStandBy] = true;
		}
	}
}

void CPMSelectionHelper::_PrepareMapPlenoTable( CProductSelelectionParameters *pclSelPMParams )
{
	m_mapPlenoByTableID.clear();

	// HYS-1537 : To be also used with wizard param
	for( int iLoopTable = 0; iLoopTable < 2; iLoopTable ++ )
	{
		CTable *pclPMProductTable = NULL;

		if( 0 == iLoopTable )
		{
			pclPMProductTable = (CTable *)( pclSelPMParams->m_pTADB->Get( _T("TECBOX_TAB") ).MP );
		}
		else
		{
			pclPMProductTable = (CTable *)( pclSelPMParams->m_pTADB->Get( _T("VSSL_TAB") ).MP );
		}

		if( NULL == pclPMProductTable )
		{
			return;
		}

		for( IDPTR PMProductIDPtr = pclPMProductTable->GetFirst(); _T('\0') != *PMProductIDPtr.ID; PMProductIDPtr = pclPMProductTable->GetNext() )
		{
			CTable *pclPlenoTable = NULL;

			if( 0 == iLoopTable )
			{
				CDB_TecBox *pclTecBox = dynamic_cast<CDB_TecBox *>( PMProductIDPtr.MP );

				if( NULL == pclTecBox || NULL == pclTecBox->GetPlenoIDPtr().MP )
				{
					continue;
				}

				if( false == pclTecBox->IsSelectable( true ) )
				{
					continue;
				}

				pclPlenoTable = dynamic_cast<CTable*>( (CData*)( pclTecBox->GetPlenoIDPtr().MP ) );
			}
			else
			{
				CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel *>( PMProductIDPtr.MP );

				if( NULL == pclVessel || NULL == pclVessel->GetPlenoIDPtr().MP )
				{
					continue;
				}

				if( false == pclVessel->IsSelectable( true ) )
				{
					continue;
				}

				pclPlenoTable = dynamic_cast<CTable*>( (CData*)( pclVessel->GetPlenoIDPtr().MP ) );
			}

			if( NULL == pclPlenoTable || 0 != m_mapPlenoByTableID.count( CString( pclPlenoTable->GetIDPtr().ID ) ) )
			{
				continue;
			}

			CString strTableID = CString( pclPlenoTable->GetIDPtr().ID );
			mapIntPrefArray mapPlenoStdAndSoftDesal;
			m_mapPlenoByTableID[strTableID] = mapPlenoStdAndSoftDesal;

			// For optimization.
			int iStop = 0;

			for( IDPTR PlenoIDPtr = pclPlenoTable->GetFirst(); _T('\0') != *PlenoIDPtr.ID; PlenoIDPtr = pclPlenoTable->GetNext() )
			{
				CDB_TBPlenoVento *pclPleno = dynamic_cast<CDB_TBPlenoVento *>( PlenoIDPtr.MP );

				if( NULL == pclPleno )
				{
					continue;
				}

				if( false == pclPleno->IsSelectable( true ) )
				{
					continue;
				}

				int iWMUpFunctions = ( CDB_TBPlenoVento::ePMPR_RefillNone == pclPleno->GetRefillable() ) ? CPMWQPrefsValidation::DWFCT_WaterMakeUpStd : CPMWQPrefsValidation::DWFCT_WaterMakeUpSoftDesal;

				if( 0 == m_mapPlenoByTableID[strTableID].count( iWMUpFunctions ) )
				{
					CPMWQPrefsValidation::PMWQPrefArray arPMWQPrefArray;
					ZeroMemory( &arPMWQPrefArray, sizeof( CPMWQPrefsValidation::PMWQPrefArray ) );
					m_mapPlenoByTableID[strTableID][iWMUpFunctions] = arPMWQPrefArray;
				}

				if( true == pclPleno->IsVariantBreakTank() )
				{
					m_mapPlenoByTableID[strTableID][iWMUpFunctions].m_arPMWQPrefs[CPMWQPrefs::WaterMakeupBreakTank] = true;

					iStop |= ( ( CPMWQPrefsValidation::DWFCT_WaterMakeUpStd == iWMUpFunctions ) ? 1 : 4 );
				}

				if( pclPleno->GetNbrPumpCompressor() > 1 )
				{
					m_mapPlenoByTableID[strTableID][iWMUpFunctions].m_arPMWQPrefs[CPMWQPrefs::WaterMakeupDutyStandBy] = true;
					iStop |= ( ( CPMWQPrefsValidation::DWFCT_WaterMakeUpStd == iWMUpFunctions ) ? 2 : 8 );
				}

				if( 15 == iStop )
				{
					break;
				}
			}
		}
	}
}
