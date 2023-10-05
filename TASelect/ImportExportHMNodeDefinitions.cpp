#include "stdafx.h"

#include "Global.h"
#include "TASelect.h"
#include "Hydromod.h"
#include "TerminalUnit.h"
#include "ImportExportHMNodeDefinitions.h"


void CImportExportHMData::CPipeData::Reset()
{
	m_strInExternalID = _T("");
	m_eInType = CDS_HydroMod::eHMObj::eNone;
	m_strInSizeID = _T("");
	m_dInInternalDiameter = 0.0; 
	m_dInRoughness = 0.0;
	m_dInLength = 0.0;
	m_dSupplyTemperature = -273.15;
	m_dReturnTemperature = -273.15;
	m_bInIsLocked = false;
	m_pclHydraulicCircuitParent = NULL;

	for( auto &iter : m_vecpInPipeAccessoryList )
	{
		if( NULL != iter )
		{
			delete iter;
		}
	}

	m_vecpInPipeAccessoryList.clear();
}

void CImportExportHMData::CHydraulicCircuitData::Reset()
{
	m_strInName = _T("");
	m_bInModule = false;
	m_pclCircuitScheme = NULL;
	m_iInPosition = -1;
	m_eInReturnMode = CDS_HydroMod::ReturnType::LastReturnType;
	m_dTemperatureDesignInletSupply = -273.15;
	m_dTemperatureDesignInletReturn = -273.15;
	m_dTemperatureDesignOutletSupply = -273.15;
	m_dTemperatureDesignOutletReturn = -273.15;
	m_pclHydraulicCircuitParent = NULL;
	m_pclHydraulicNetworkData = NULL;
	
	for( auto &iter : m_vecpTerminalUnitList )
	{
		if( NULL != iter )
		{
			delete iter;
		}
	}

	m_vecpTerminalUnitList.clear();

	for( auto &iter : m_vecpPipeList )
	{
		if( NULL != iter )
		{
			delete iter;
		}
	}

	m_vecpPipeList.clear();

	for( auto &iter : m_vecpBalancingValveList )
	{
		if( NULL != iter )
		{
			delete iter;
		}
	}

	m_vecpBalancingValveList.clear();

	for( auto &iter : m_vecpDpControllerValveList )
	{
		if( NULL != iter )
		{
			delete iter;
		}
	}

	m_vecpDpControllerValveList.clear();

	for( auto &iter : m_vecpControlValveList )
	{
		if( NULL != iter )
		{
			delete iter;
		}
	}

	m_vecpControlValveList.clear();

	for( auto &iter : m_vecpShutoffValveList )
	{
		if( NULL != iter )
		{
			delete iter;
		}
	}

	m_vecpShutoffValveList.clear();

	if( NULL != m_pclHydraulicChildrenCircuitList )
	{
		delete m_pclHydraulicChildrenCircuitList;
		m_pclHydraulicChildrenCircuitList = NULL;
	}
}

void CImportExportHMData::CHydraulicCircuitListData::Reset()
{
	m_pclHydraulicCircuitParent = NULL;
	m_pclHydraulicNetworkData = NULL;

	for( auto &iter : m_vecpHydraulicCircuitList )
	{
		if( NULL != iter )
		{
			delete iter;
		}
	}

	m_vecpHydraulicCircuitList.clear();
}
