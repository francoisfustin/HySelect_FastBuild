#include "stdafx.h"


#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include "MainFrm.h"
#include "TASelect.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "ImportExportHMNodeDefinitions.h"
#include "ImportExportHM.h"
#include "ExportHMBase.h"

CExportHMBase::CExportHMBase()
{ 
	m_rMainNode.pnodeJson = NULL;
	// m_rMainNode.XML = null;
}

CExportHMBase::~CExportHMBase()
{
}

bool CExportHMBase::Export( CTable *pclTable, ProjectType eProjecType, CWaterChar clWaterChar, double dReturnTemperature, CString strFileName, CString &strErrorMsg )
{
	strErrorMsg = _T("");

	// First, copy all the hydraulic network from 'pclTable' to an intermediate structure.
	if( false == m_clImportExportHM.ConvertHMBeforeToExport( pclTable, eProjecType, clWaterChar, dReturnTemperature, strErrorMsg ) )
	{
		strErrorMsg = _T("Error when reading hydraulic circuit passed as argument.");
		return false;
	}

	// Now, use this internal structure to prepare a buffer with data to be written in file.
	NodeDefinition subNode;

	if( false == AddSubNodeObject( m_rMainNode, IMPORTEXPORTHM_NODE_HYDRAULIC_NETWORK, subNode, strErrorMsg ) )
	{
		strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HYDRAULIC_NETWORK );
		return false;
	}

	if( false == _WriteHydraulicNetwork( subNode, m_clImportExportHM.GetImportExportHMData()->m_clHydraulicNetwork, strErrorMsg ) )
	{
		return false;
	}

	// Copy now the 'm_rMainNode' containing all the info in the file.
	if( false == WriteFile( strFileName, strErrorMsg ) )
	{
		strErrorMsg = _T("Error when writing the file.");
		return false;
	}

	return true;
}

bool CExportHMBase::_WriteHydraulicNetwork( NodeDefinition &nodeHydraulicNetwork, CImportExportHMData::CHydraulicNetworkData &clHydraulicNetwork, CString &strErrorMsg )
{
	// Write version.
	if( false == WriteDoubleValue( nodeHydraulicNetwork, IMPORTEXPORTHM_NODE_HN_VERSION, strErrorMsg, clHydraulicNetwork.m_dVersion ) )
	{
		strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_VERSION );
		return false;
	}

	// Write direction.
	if( false == WriteStringValue( nodeHydraulicNetwork, IMPORTEXPORTHM_NODE_HN_DIRECTION, strErrorMsg, clHydraulicNetwork.m_strDirection ) )
	{
		strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_DIRECTION );
		return false;
	}

	// Write parameters.
	NodeDefinition subNodeParameters;

	if( false == AddSubNodeObject( nodeHydraulicNetwork, IMPORTEXPORTHM_NODE_HN_PARAMETERS, subNodeParameters, strErrorMsg ) )
	{
		strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_PARAMETERS );
		return false;
	}

	if( false == _WriteParameters( subNodeParameters, clHydraulicNetwork.m_clParameters, strErrorMsg ) )
	{
		return false;
	}

	// Write hydraulic circuit list.
	if( false == _WriteHydraulicCircuitList( nodeHydraulicNetwork, &clHydraulicNetwork.m_clHydraulicCircuitList, strErrorMsg ) )
	{
		return false;
	}

	return true;
}

bool CExportHMBase::_WriteParameters( NodeDefinition &nodeParameters, CImportExportHMData::CParametersData &clParameters, CString &strErrorMsg )
{
	// Write heating/cooling mode.
	CString strMode = ( ProjectType::Heating == clParameters.m_eMode ) ? IMPORTEXPORTHM_VALUE_HN_P_MODE_HEATING : IMPORTEXPORTHM_VALUE_HN_P_MODE_COOLING;

	if( false == WriteStringValue( nodeParameters, IMPORTEXPORTHM_NODE_HN_P_MODE, strErrorMsg, strMode ) )
	{
		strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_P_MODE );
		return false;
	}

	// Write water characteristic.
	NodeDefinition subNodeWaterChar;

	if( false == AddSubNodeObject( nodeParameters, IMPORTEXPORTHM_NODE_HN_P_FLUIDCHARACTERISTIC, subNodeWaterChar, strErrorMsg ) )
	{
		strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_P_FLUIDCHARACTERISTIC );
		return false;
	}

	if( false == _WriteFluidCharacteristic( subNodeWaterChar, clParameters.m_WC, clParameters.m_dReturnTemperature, strErrorMsg ) )
	{
		return false;
	}

	return true;
}

bool CExportHMBase::_WriteFluidCharacteristic( NodeDefinition &nodeFluidCharacteristic, CWaterChar &clWaterChar, double &dReturnTemperature, CString &strErrorMsg )
{
	// Write supply temperature.
	if( false == WriteDoubleValue( nodeFluidCharacteristic, IMPORTEXPORTHM_NODE_HN_P_FC_INSUPPLYTEMPERATURE, strErrorMsg, clWaterChar.GetTemp() ) )
	{
		strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_P_FC_INSUPPLYTEMPERATURE );
		return false;
	}

	// Write return temperature.
	if( false == WriteDoubleValue( nodeFluidCharacteristic, IMPORTEXPORTHM_NODE_HN_P_FC_INRETURNTEMPERATURE, strErrorMsg, dReturnTemperature ) )
	{
		strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_P_FC_INRETURNTEMPERATURE );
		return false;
	}

	// Write additive family ID.
	if( false == WriteStringValue( nodeFluidCharacteristic, IMPORTEXPORTHM_NODE_HN_P_FC_INADDITIVEFAMILY, strErrorMsg, clWaterChar.GetAdditFamID() ) )
	{
		strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_P_FC_INADDITIVEFAMILY );
		return false;
	}

	// Write additive name ID.
	if( false == WriteStringValue( nodeFluidCharacteristic, IMPORTEXPORTHM_NODE_HN_P_FC_INADDITIVENAME, strErrorMsg, clWaterChar.GetAdditID() ) )
	{
		strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_P_FC_INADDITIVENAME );
		return false;
	}

	// Write additive weight.
	if( false == WriteDoubleValue( nodeFluidCharacteristic, IMPORTEXPORTHM_NODE_HN_P_FC_INADDITIVEWEIGHT, strErrorMsg, clWaterChar.GetPcWeight() ) )
	{
		strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_P_FC_INADDITIVEWEIGHT );
		return false;
	}

	return true;
}

bool CExportHMBase::_WriteHydraulicCircuitList( NodeDefinition &nodeHydraulicNetwork, CImportExportHMData::CHydraulicCircuitListData *pclHydraulicCircuitList, CString &strErrorMsg )
{
	if( 0 == (int)pclHydraulicCircuitList->m_vecpHydraulicCircuitList.size() )
	{
		// No hydraulic circuit to export.
		return true;
	}

	// Add hydraulic circuit list node.
	NodeDefinition nodeHydraulicCircuitList;

	if( false == AddSubNodeArray( nodeHydraulicNetwork, IMPORTEXPORTHM_NODE_HN_HYDRAULICCIRCUITLIST, nodeHydraulicCircuitList, strErrorMsg ) )
	{
		strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HYDRAULICCIRCUITLIST );
		return false;
	}

	for( auto &pclHydraulicCircuit : pclHydraulicCircuitList->m_vecpHydraulicCircuitList )
	{
		NodeDefinition nodeHydraulicCircuit;

		if( false == AddSubNodeObject( nodeHydraulicCircuitList, IMPORTEXPORTHM_NODE_HN_HCL_HYDRAULIC_CIRCUIT, nodeHydraulicCircuit, strErrorMsg ) )
		{
			strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HYDRAULIC_CIRCUIT );
			return false;
		}

		// Write the name.
		if( false == WriteStringValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INNAME, strErrorMsg, pclHydraulicCircuit->m_strInName ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_INNAME );
			return false;
		}

		// Write the type.
		CString strInType = ( true == pclHydraulicCircuit->m_bInModule ) ? IMPORTEXPORTHM_VALUE_HN_HCL_HC_TYPE_MODULE : IMPORTEXPORTHM_VALUE_HN_HCL_HC_TYPE_CIRCUIT;

		if( false == WriteStringValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INTYPE, strErrorMsg, strInType ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_INTYPE );
			return false;
		}

		// Write the circuit type ID.
		if( false == WriteStringValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INCIRCUITTYPEID, strErrorMsg, pclHydraulicCircuit->m_pclCircuitScheme->GetIDPtr().ID ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_INCIRCUITTYPEID );
			return false;
		}

		// Write the position.
		if( false == WriteIntValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INPOSITION, strErrorMsg, pclHydraulicCircuit->m_iInPosition ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_INPOSITION );
			return false;
		}

		// Write the return mode.
		CString strReturnMode = ( CDS_HydroMod::ReturnType::Direct == pclHydraulicCircuit->m_eInReturnMode ) ? 
				IMPORTEXPORTHM_VALUE_HN_HCL_HC_RETURNMODE_DIRECT : IMPORTEXPORTHM_VALUE_HN_HCL_HC_RETURNMODE_REVERSE;

		if( false == WriteStringValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INRETURNMODE, strErrorMsg, strReturnMode ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_INPOSITION );
			return false;
		}

		// Write the design temperature at the inlet supply.
		if( false == WriteDoubleValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNINLETSUPPLY, strErrorMsg, pclHydraulicCircuit->m_dTemperatureDesignInletSupply ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNINLETSUPPLY );
			return false;
		}

		// Write the design temperature at the inlet return.
		if( false == WriteDoubleValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNINLETRETURN, strErrorMsg, pclHydraulicCircuit->m_dTemperatureDesignInletReturn ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNINLETRETURN );
			return false;
		}

		// Write the design temperature at the outlet supply.
		if( false == WriteDoubleValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNOUTLETSUPPLY, strErrorMsg, pclHydraulicCircuit->m_dTemperatureDesignOutletSupply ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNOUTLETSUPPLY );
			return false;
		}

		// Write the design temperature at the outlet return.
		if( false == WriteDoubleValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNOUTLETRETURN, strErrorMsg, pclHydraulicCircuit->m_dTemperatureDesignOutletReturn ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNOUTLETRETURN );
			return false;
		}

		// Write the terminal unit.
		if( false == _WriteTerminalUnitList( nodeHydraulicCircuit, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Write pipe list.
		if( false == _WritePipeList( nodeHydraulicCircuit, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Write balancing valve list.
		if( false == _WriteBalancingValveList( nodeHydraulicCircuit, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Write Dp controller valve list.
		if( false == _WriteDpControllerValveList( nodeHydraulicCircuit, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Write control valve list.
		if( false == _WriteControlValveList( nodeHydraulicCircuit, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Write shutoff valve list.
		if( false == _WriteShutoffValveList( nodeHydraulicCircuit, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Write pump.
		if( false == _WritePump( nodeHydraulicCircuit, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}
		
		// Check if the current hydraulic circuit is a module.
		if( true == pclHydraulicCircuit->m_bInModule && NULL != pclHydraulicCircuit->m_pclHydraulicChildrenCircuitList )
		{
			if( false == _WriteHydraulicCircuitList( nodeHydraulicCircuit, pclHydraulicCircuit->m_pclHydraulicChildrenCircuitList, strErrorMsg ) )
			{
				return false;
			}
		}
	}

	return true;
}

bool CExportHMBase::_WriteTerminalUnitList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	if( 0 == (int)pclHydraulicCircuit->m_vecpTerminalUnitList.size() )
	{
		// No terminal unit.
		return true;
	}

	// Add terminal unit list node.
	NodeDefinition nodeTerminalUnitList;

	if( false == AddSubNodeArray( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_TERMINALUNITLIST, nodeTerminalUnitList, strErrorMsg ) )
	{
		strErrorMsg.Format( _T("Can't create the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TERMINALUNITLIST );
		return false;
	}

	for( auto &pclTerminalUnitData : pclHydraulicCircuit->m_vecpTerminalUnitList )
	{
		NodeDefinition nodeTerminalUnit;

		if( false == AddSubNodeObject( nodeTerminalUnitList, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TERMINALUNIT, nodeTerminalUnit, strErrorMsg ) )
		{
			strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TERMINALUNIT );
			return false;
		}

		// Write the flow.
		NodeDefinition nodeTerminalUnitFlow;

		if( false == AddSubNodeObject( nodeTerminalUnit, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_INFLOW, nodeTerminalUnitFlow, strErrorMsg ) )
		{
			strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_INFLOW );
			return false;
		}

		if( CTermUnit::_QType::Q == pclTerminalUnitData->m_eFlowType )
		{
			// Write the flow type.
			if( false == WriteStringValue( nodeTerminalUnitFlow, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_TYPE, strErrorMsg, IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IF_TYPE_FLOW ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_TYPE );
				return false;
			}

			// Write the flow.
			if( false == WriteDoubleValue( nodeTerminalUnitFlow, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_FLOW, strErrorMsg, pclTerminalUnitData->m_dFlow ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_FLOW );
				return false;
			}
		}
		else
		{
			// Write the flow type.
			if( false == WriteStringValue( nodeTerminalUnitFlow, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_TYPE, strErrorMsg, IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IF_TYPE_POWERDT ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_TYPE );
				return false;
			}

			// Write the power/DT.
			if( false == WriteDoubleValue( nodeTerminalUnitFlow, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_POWER, strErrorMsg, pclTerminalUnitData->m_dPower ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_POWER );
				return false;
			}

			if( false == WriteDoubleValue( nodeTerminalUnitFlow, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_DT, strErrorMsg, pclTerminalUnitData->m_dDT ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_DT );
				return false;
			}
		}

		// Write the pressure drop.
		NodeDefinition nodeTerminalUnitPressureDrop;

		if( false == AddSubNodeObject( nodeTerminalUnit, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_INPRESSUREDROP, nodeTerminalUnitPressureDrop, strErrorMsg ) )
		{
			strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_INPRESSUREDROP );
			return false;
		}

		if( CDS_HydroMod::eDpType::Dp == pclTerminalUnitData->m_ePressureDropType )
		{
			// Write the pressure drop type.
			if( false == WriteStringValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_TYPE, strErrorMsg, IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_PRESSUREDROP ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_TYPE );
				return false;
			}

			// Write the pressure drop.
			if( false == WriteDoubleValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_PRESSUREDROP, strErrorMsg, pclTerminalUnitData->m_dPressureDrop ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_PRESSUREDROP );
				return false;
			}
		}
		else if( CDS_HydroMod::eDpType::QDpref == pclTerminalUnitData->m_ePressureDropType )
		{
			// Write the pressure drop type.
			if( false == WriteStringValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_TYPE, strErrorMsg, IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_QREFDPREF ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_TYPE );
				return false;
			}

			// Write the Qref/Dpref.
			if( false == WriteDoubleValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_FLOW_REF, strErrorMsg, pclTerminalUnitData->m_dFlowRef ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_FLOW_REF );
				return false;
			}

			if( false == WriteDoubleValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_PRESSURE_DROP_REF, strErrorMsg, pclTerminalUnitData->m_dPressureDropRef ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_PRESSURE_DROP_REF );
				return false;
			}
		}
		else if( CDS_HydroMod::eDpType::Kv == pclTerminalUnitData->m_ePressureDropType )
		{
			// Write the pressure drop type.
			if( false == WriteStringValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_TYPE, strErrorMsg, IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_KV ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_TYPE );
				return false;
			}

			// Write the kv.
			if( false == WriteDoubleValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_KV, strErrorMsg, pclTerminalUnitData->m_dKv ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_KV );
				return false;
			}
		}
		else
		{
			// Write the pressure drop type.
			if( false == WriteStringValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_TYPE, strErrorMsg, IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_CV ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_TYPE );
				return false;
			}

			// Write cv.
			if( false == WriteDoubleValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_CV, strErrorMsg, pclTerminalUnitData->m_dCv ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_CV );
				return false;
			}
		}
	}

	return true;
}

bool CExportHMBase::_WritePipeList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	if( 0 == (int)pclHydraulicCircuit->m_vecpPipeList.size() )
	{
		// No pipe to export.
		return true;
	}

	// Add pipe list node.
	NodeDefinition nodePileList;

	if( false == AddSubNodeArray( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_PIPELIST, nodePileList, strErrorMsg ) )
	{
		strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PIPELIST );
		return false;
	}

	for( auto &pclPipeData : pclHydraulicCircuit->m_vecpPipeList )
	{
		NodeDefinition nodePipe;

		if( false == AddSubNodeObject( nodePileList, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_PIPE, nodePipe, strErrorMsg ) )
		{
			strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_PIPE );
			return false;
		}

		// Write the pipe type.
		CString strPipeType = _T("");

		if( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == pclPipeData->m_eInType )
		{
			strPipeType = IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_DISTRIBUTIONSUPPLY;
		}
		else if( CDS_HydroMod::eHMObj::eDistributionReturnPipe == pclPipeData->m_eInType )
		{
			strPipeType = IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_DISTRIBUTIONRETURN;
		}
		else if( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe == pclPipeData->m_eInType )
		{
			strPipeType = IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_CIRCUITPRIMARY;
		}
		else
		{
			strPipeType = IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_CIRCUITSECONDARY;
		}

		if( false == WriteStringValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INTYPE, strErrorMsg, strPipeType ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INTYPE );
			return false;
		}

		// Write the pipe size ID.
		if( false == WriteStringValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INSIZEID, strErrorMsg, pclPipeData->m_strInSizeID ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INSIZEID );
			return false;
		}

		// Write the internal diameter.
		if( false == WriteDoubleValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_ININTERNALDIAMETER, strErrorMsg, pclPipeData->m_dInInternalDiameter ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_ININTERNALDIAMETER );
			return false;
		}

		// Write the roughness.
		if( false == WriteDoubleValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INROUGHNESS, strErrorMsg, pclPipeData->m_dInRoughness ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INROUGHNESS );
			return false;
		}

		// Write the length.
		if( false == WriteDoubleValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INLENGTH, strErrorMsg, pclPipeData->m_dInLength ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INLENGTH );
			return false;
		}

		// Write the supply temperature.
		if( false == WriteDoubleValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INTEMPERATURESUPPLY, strErrorMsg, pclPipeData->m_dSupplyTemperature ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INTEMPERATURESUPPLY );
			return false;
		}

		// Write the return temperature.
		if( false == WriteDoubleValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INTEMPERATURERETURN, strErrorMsg, pclPipeData->m_dReturnTemperature ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INTEMPERATURERETURN );
			return false;
		}

		// Write the lock.
		CString strIsLocked = ( true == pclPipeData->m_bInIsLocked ) ? _T("yes") : _T("no");
		
		if( false == WriteStringValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INLOCKED, strErrorMsg, strIsLocked ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INLOCKED );
			return false;
		}

		// Write pipe in accessory list.
		if( false == _WritePipeAccessoryList( nodePipe, pclPipeData, strErrorMsg ) )
		{
			return false;
		}
	}

	return true;
}

bool CExportHMBase::_WritePipeAccessoryList( NodeDefinition &nodePipeList, CImportExportHMData::CPipeData *pclPipeData, CString &strErrorMsg )
{
	if( 0 == (int)pclPipeData->m_vecpInPipeAccessoryList.size() )
	{
		// No pipe accessory to export.
		return true;
	}
	
	// Add pipe accessory list node.
	NodeDefinition nodePipeAccessoryList;

	if( false == AddSubNodeObject( nodePipeList, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_ACCESSORYLIST, nodePipeAccessoryList, strErrorMsg ) )
	{
		strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_ACCESSORYLIST );
		return false;
	}

	// Add in pipe accessory list node.
	NodeDefinition nodeInPipeAccessoryList;

	if( false == AddSubNodeArray( nodePipeAccessoryList, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_INACCESSORYLIST, nodeInPipeAccessoryList, strErrorMsg ) )
	{
		strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_INACCESSORYLIST );
		return false;
	}

	for( auto &pclPipeAccessoryData : pclPipeData->m_vecpInPipeAccessoryList )
	{
		NodeDefinition nodeInPipeAccessory;

		if( false == AddSubNodeObject( nodeInPipeAccessoryList, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_ACCESSORY, nodeInPipeAccessory, strErrorMsg ) )
		{
			strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_ACCESSORY );
			return false;
		}

		// Write the external ID.
		if( false == WriteStringValue( nodeInPipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_EXTERNALID, strErrorMsg, pclPipeAccessoryData->m_strInExternalID ) )
		{
		}

		// Write the pipe accessory type.
		CString strPipeAccessoryType = _T("");

		if( CDS_HydroMod::eDpType::None == pclPipeAccessoryData->m_eType )
		{
			strPipeAccessoryType = ( true == pclPipeAccessoryData->m_bIsElbow ) ? IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_ELBOW : IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_TEE;
		}
		else if( CDS_HydroMod::eDpType::dzeta == pclPipeAccessoryData->m_eType )
		{
			strPipeAccessoryType = IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_ZETA;
		}
		else if( CDS_HydroMod::eDpType::Dp == pclPipeAccessoryData->m_eType )
		{
			strPipeAccessoryType = IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_DP;
		}
		else if( CDS_HydroMod::eDpType::QDpref == pclPipeAccessoryData->m_eType )
		{
			strPipeAccessoryType = IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_QREFDPREF;
		}
		else if( CDS_HydroMod::eDpType::Kv == pclPipeAccessoryData->m_eType )
		{
			strPipeAccessoryType = IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_KV;
		}
		else if( CDS_HydroMod::eDpType::Cv == pclPipeAccessoryData->m_eType )
		{
			strPipeAccessoryType = IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_CV;
		}

		if( false == WriteStringValue( nodeInPipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE, strErrorMsg, strPipeAccessoryType ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE );
			return false;
		}

		// Write value.
		if( CDS_HydroMod::eDpType::dzeta == pclPipeAccessoryData->m_eType )
		{
			// Write the zeta value.
			if( false == WriteDoubleValue( nodeInPipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_ZETA, strErrorMsg, pclPipeAccessoryData->m_dZetaValue ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_ZETA );
				return false;
			}
		}
		else if( CDS_HydroMod::eDpType::Dp == pclPipeAccessoryData->m_eType )
		{
			// Write the pressure drop value.
			if( false == WriteDoubleValue( nodeInPipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_PRESSUREDROP, strErrorMsg, pclPipeAccessoryData->m_dPressureDrop ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_PRESSUREDROP );
				return false;
			}
		}
		else if( CDS_HydroMod::eDpType::QDpref == pclPipeAccessoryData->m_eType )
		{
			// Write the flow ref value.
			if( false == WriteDoubleValue( nodeInPipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_FLOWREF, strErrorMsg, pclPipeAccessoryData->m_dFlowRef ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_FLOWREF );
				return false;
			}

			// Write the pressure drop ref value.
			if( false == WriteDoubleValue( nodeInPipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_PRESSUREDROPREF, strErrorMsg, pclPipeAccessoryData->m_dPressureDropRef ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_PRESSUREDROPREF );
				return false;
			}
		}
		else if( CDS_HydroMod::eDpType::Kv == pclPipeAccessoryData->m_eType )
		{
			// Write the Kv value.
			if( false == WriteDoubleValue( nodeInPipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_KV, strErrorMsg, pclPipeAccessoryData->m_dKv ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_KV );
				return false;
			}
		}
		else if( CDS_HydroMod::eDpType::Cv == pclPipeAccessoryData->m_eType )
		{
			// Write the Cv value.
			if( false == WriteDoubleValue( nodeInPipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_CV, strErrorMsg, pclPipeAccessoryData->m_dCv ) )
			{
				strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_CV );
				return false;
			}
		}
	}

	return true;
}

bool CExportHMBase::_WriteBalancingValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	if( 0 == (int)pclHydraulicCircuit->m_vecpBalancingValveList.size() )
	{
		// No balancing valve to export.
		return true;
	}

	// Add balancing valve list node.
	NodeDefinition nodeBalancingValveList;

	if( false == AddSubNodeArray( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_BALANCINGVALVELIST, nodeBalancingValveList, strErrorMsg ) )
	{
		strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_BALANCINGVALVELIST );
		return false;
	}

	for( auto &pclBalancingValveData : pclHydraulicCircuit->m_vecpBalancingValveList )
	{
		NodeDefinition nodeBalancingValve;

		if( false == AddSubNodeObject( nodeBalancingValveList, IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BALANCINGVALVE, nodeBalancingValve, strErrorMsg ) )
		{
			strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BALANCINGVALVE );
			return false;
		}

		// Write the location.
		CString strInLocation = _T("");

		if( CDS_HydroMod::eHMObj::eBVprim == pclBalancingValveData->m_eInLocation )
		{
			strInLocation = IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_PRIMARY;
		}
		else if( CDS_HydroMod::eHMObj::eBVsec == pclBalancingValveData->m_eInLocation )
		{
			strInLocation = IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_SECONDARY;
		}
		else if( CDS_HydroMod::eHMObj::eBVbyp == pclBalancingValveData->m_eInLocation )
		{
			strInLocation = IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_BYPASS;
		}

		if( false == WriteStringValue( nodeBalancingValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INLOCATION, strErrorMsg, strInLocation ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INLOCATION );
			return false;
		}

		// Write the balancing valve size ID.
		if( false == WriteStringValue( nodeBalancingValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INSIZEID, strErrorMsg, pclBalancingValveData->m_strInSizeID ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INSIZEID );
			return false;
		}

		// Write the flow.
		if( false == WriteDoubleValue( nodeBalancingValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INFLOW, strErrorMsg, pclBalancingValveData->m_dFlow ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INFLOW );
			return false;
		}

		// Write the lock.
		CString strIsLocked = ( true == pclBalancingValveData->m_bInIsLocked ) ? _T("yes") : _T("no");

		if( false == WriteStringValue( nodeBalancingValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INLOCKED, strErrorMsg, strIsLocked ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INLOCKED );
			return false;
		}
	}

	return true;
}

bool CExportHMBase::_WriteDpControllerValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	if( 0 == (int)pclHydraulicCircuit->m_vecpDpControllerValveList.size() )
	{
		// No Dp controller to export.
		return true;
	}

	// Add Dp controller list node.
	NodeDefinition nodeDpControllerValveList;

	if( false == AddSubNodeArray( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCONTROLLERVALVELIST, nodeDpControllerValveList, strErrorMsg ) )
	{
		strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCONTROLLERVALVELIST );
		return false;
	}

	for( auto &pclDpControllerValveData : pclHydraulicCircuit->m_vecpDpControllerValveList )
	{
		NodeDefinition nodeDpControllerValve;

		if( false == AddSubNodeObject( nodeDpControllerValveList, IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCONTROLLERVALVE, nodeDpControllerValve, strErrorMsg ) )
		{
			strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCONTROLLERVALVE );
			return false;
		}

		// Write the external ID.
		if( false == WriteStringValue( nodeDpControllerValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INEXTERNALID, strErrorMsg, pclDpControllerValveData->m_strInExternalID ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INEXTERNALID );
			return false;
		}

		// Write the Dp controller valve size ID.
		if( false == WriteStringValue( nodeDpControllerValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INSIZEID, strErrorMsg, pclDpControllerValveData->m_strInSizeID ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INSIZEID );
			return false;
		}

		// Write the flow.
		if( false == WriteDoubleValue( nodeDpControllerValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INFLOW, strErrorMsg, pclDpControllerValveData->m_dFlow ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INFLOW );
			return false;
		}

		// Write the lock.
		CString strIsLocked = ( true == pclDpControllerValveData->m_bInIsLocked ) ? _T("yes") : _T("no");

		if( false == WriteStringValue( nodeDpControllerValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INLOCKED, strErrorMsg, strIsLocked ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INLOCKED );
			return false;
		}
	}

	return true;
}

bool CExportHMBase::_WriteControlValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	if( 0 == (int)pclHydraulicCircuit->m_vecpControlValveList.size() )
	{
		// No control valve to export.
		return true;
	}

	// Add control valve list node.
	NodeDefinition nodeControlValveList;

	if( false == AddSubNodeArray( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_CONTROLVALVELIST, nodeControlValveList, strErrorMsg ) )
	{
		strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_CONTROLVALVELIST );
		return false;
	}

	for( auto &pclControlValveData : pclHydraulicCircuit->m_vecpControlValveList )
	{
		NodeDefinition nodeControlValve;

		if( false == AddSubNodeObject( nodeControlValveList, IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CONTROLVALVE, nodeControlValve, strErrorMsg ) )
		{
			strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CONTROLVALVE );
			return false;
		}

		// Read the control type (Mandatory).
		CString strControlType = _T("");
			
		if( CDB_ControlProperties::CvCtrlType::eCvOnOff == pclControlValveData->m_eControlType )
		{
			strControlType = IMPORTEXPORTHM_VALUE_HN_HCL_HC_CVL_CV_INCONTROLTYPE_ONOFF;
		}
		else if( CDB_ControlProperties::CvCtrlType::eCv3point == pclControlValveData->m_eControlType )
		{
			strControlType = IMPORTEXPORTHM_VALUE_HN_HCL_HC_CVL_CV_INCONTROLTYPE_3POINTS;
		}
		else if( CDB_ControlProperties::CvCtrlType::eCvProportional == pclControlValveData->m_eControlType )
		{
			strControlType = IMPORTEXPORTHM_VALUE_HN_HCL_HC_CVL_CV_INCONTROLTYPE_PROPORTIONAL;
		}

		if( false == WriteStringValue( nodeControlValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INCONTROLTYPE, strErrorMsg, strControlType ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INCONTROLTYPE );
			return false;
		}

		// Write the control valve size ID.
		if( false == WriteStringValue( nodeControlValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INSIZEID, strErrorMsg, pclControlValveData->m_strInSizeID ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INSIZEID );
			return false;
		}

		// Write the flow.
		if( false == WriteDoubleValue( nodeControlValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INFLOW, strErrorMsg, pclControlValveData->m_dFlow ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INFLOW );
			return false;
		}

		// Write the lock.
		CString strIsLocked = ( true == pclControlValveData->m_bInIsLocked ) ? _T("yes") : _T("no");

		if( false == WriteStringValue( nodeControlValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INLOCKED, strErrorMsg, strIsLocked ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INLOCKED );
			return false;
		}
	}

	return true;
}

bool CExportHMBase::_WriteShutoffValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	if( 0 == (int)pclHydraulicCircuit->m_vecpShutoffValveList.size() )
	{
		// No shut-off valve to export.
		return true;
	}

	// Add shut-off valve list node.
	NodeDefinition nodeShutoffValveList;

	if( false == AddSubNodeArray( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_SHUTOFFVALVELIST, nodeShutoffValveList, strErrorMsg ) )
	{
		strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_SHUTOFFVALVELIST );
		return false;
	}

	for( auto &pclShutoffValveData : pclHydraulicCircuit->m_vecpShutoffValveList )
	{
		NodeDefinition nodeShutoffValve;

		if( false == AddSubNodeObject( nodeShutoffValveList, IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SHUTOFFVALVE, nodeShutoffValve, strErrorMsg ) )
		{
			strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SHUTOFFVALVE );
			return false;
		}

		// Write the shutoff valve size ID.
		if( false == WriteStringValue( nodeShutoffValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SV_INSIZEID, strErrorMsg, pclShutoffValveData->m_strInSizeID ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SV_INSIZEID );
			return false;
		}

		// Write the flow.
		if( false == WriteDoubleValue( nodeShutoffValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SV_INFLOW, strErrorMsg, pclShutoffValveData->m_dFlow ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SV_INFLOW );
			return false;
		}

		// Write the lock.
		CString strIsLocked = ( true == pclShutoffValveData->m_bInIsLocked ) ? _T("yes") : _T("no");

		if( false == WriteStringValue( nodeShutoffValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SV_INLOCKED, strErrorMsg, strIsLocked ) )
		{
			strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SV_INLOCKED );
			return false;
		}
	}

	return true;
}

bool CExportHMBase::_WritePump( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	if( false == (int)pclHydraulicCircuit->m_Pump.m_bExist )
	{
		// No pump to export.
		return true;
	}

	// Add control valve list node.
	NodeDefinition nodePump;

	if( false == AddSubNodeObject( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_PUMP, nodePump, strErrorMsg ) )
	{
		strErrorMsg.Format( _T("Can't add the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_PUMP );
		return false;
	}

	// Write Pump hmin.
	if( false == WriteDoubleValue( nodePump, IMPORTEXPORTHM_NODE_HN_HCL_HC_P_HMIN, strErrorMsg, pclHydraulicCircuit->m_Pump.m_dHMin ) )
	{
		strErrorMsg.Format( _T("Can't write the '%s' node."), IMPORTEXPORTHM_NODE_HN_HCL_HC_P_HMIN );
		return false;
	}

	return true;
}
