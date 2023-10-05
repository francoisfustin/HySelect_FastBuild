#include "stdafx.h"


#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include "MainFrm.h"
#include "TASelect.h"
#include "Hydromod.h"
#include "TerminalUnit.h"
#include "HMPipes.h"
#include "HMPump.h"
#include "ImportExportHMNodeDefinitions.h"
#include "ImportExportHM.h"
#include "ExportHMBase.h"
#include "ImportHMBase.h"

bool CImportExportHM::ConvertHMBeforeToExport( CTable *pclTable, ProjectType eProjecType, CWaterChar clWaterChar, double dReturnTemperature, CString strErrorMsg )
{
	if( NULL == pclTable )
	{
		return false;
	}

	m_clImportExportHMData.m_clHydraulicNetwork.m_dVersion = IMPORTEXPORTHM_VERSION;
	m_clImportExportHMData.m_clHydraulicNetwork.m_strDirection = EXPORT_HN_DIRECTION;
	m_clImportExportHMData.m_clHydraulicNetwork.m_clParameters.m_eMode = eProjecType;
	m_clImportExportHMData.m_clHydraulicNetwork.m_clParameters.m_WC = clWaterChar;
	m_clImportExportHMData.m_clHydraulicNetwork.m_clParameters.m_dReturnTemperature = dReturnTemperature;
	m_clImportExportHMData.m_clHydraulicNetwork.m_clParameters.m_pclHydraulicNetworkData = &m_clImportExportHMData.m_clHydraulicNetwork;
	m_clImportExportHMData.m_clHydraulicNetwork.m_clHydraulicCircuitList.m_pclHydraulicNetworkData = &m_clImportExportHMData.m_clHydraulicNetwork;
	m_clImportExportHMData.m_clHydraulicNetwork.m_clHydraulicCircuitList.m_pclHydraulicCircuitParent = NULL;

	if( false == _ReadHydraulicCircuitList( pclTable, &m_clImportExportHMData.m_clHydraulicNetwork.m_clHydraulicCircuitList, strErrorMsg ) )
	{
		return false;
	}

	return true;
}

bool CImportExportHM::CreateHMAfterImport( CTable *pclTable, CString strErrorMsg )
{
	return true;
}

bool CImportExportHM::_ReadHydraulicCircuitList( CTable *pclTable, CImportExportHMData::CHydraulicCircuitListData *pclHydraulicCircuitList, CString &strErrorMsg )
{
	IDPTR IDPtr = pclTable->GetFirst();

	if( NULL == IDPtr.MP || _T('\0') == *IDPtr.ID )
	{
		strErrorMsg = _T("At least one circuit must be defined.");
		return false;
	}

	// For each hydromod.
	for(; NULL != IDPtr.MP; IDPtr = pclTable->GetNext( IDPtr.MP ) )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( IDPtr.MP );

		if( NULL == pHM )
		{
			strErrorMsg.Format( _T("Internal error: Object '%s' is not a 'CDS_HydroMod' object."), IDPtr.ID );
			return false;
		}

		CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit = new CImportExportHMData::CHydraulicCircuitData();
		pclHydraulicCircuit->m_pclHydraulicCircuitParent = pclHydraulicCircuitList->m_pclHydraulicCircuitParent;
		pclHydraulicCircuit->m_pclHydraulicNetworkData = pclHydraulicCircuitList->m_pclHydraulicNetworkData;

		pclHydraulicCircuit->m_strInName = pHM->GetHMName();
		pclHydraulicCircuit->m_bInModule = pHM->IsaModule();
		pclHydraulicCircuit->m_pclCircuitScheme = pHM->GetpSch();
		pclHydraulicCircuit->m_iInPosition = pHM->GetPos();
		pclHydraulicCircuit->m_eInReturnMode = pHM->GetReturnType();
		pHM->GetDesignTemperature( pclHydraulicCircuit->m_dTemperatureDesignInletSupply, NULL, CAnchorPt::PipeLocation_Supply );
		pHM->GetDesignTemperature( pclHydraulicCircuit->m_dTemperatureDesignInletReturn, NULL, CAnchorPt::PipeLocation_Return );

		if( true == pHM->IsInjectionCircuit() )
		{
			CDS_HmInj *pclHmInj = dynamic_cast<CDS_HmInj *>( pHM );

			if( NULL == pclHmInj )
			{
				strErrorMsg.Format( _T("Internal error: Object '%s' is not a 'CDS_HmInj' object."), IDPtr.ID );
				return false;
			}

			pclHydraulicCircuit->m_dTemperatureDesignOutletSupply = pclHmInj->GetDesignCircuitSupplySecondaryTemperature();
			pclHydraulicCircuit->m_dTemperatureDesignOutletReturn = pclHmInj->GetDesignCircuitReturnSecondaryTemperature();
		}
		
		// Read the terminal unit.
		if( false == _ReadTerminalUnitList( pHM, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Read pipe list (Optional for the root, mandatory for others).
		if( false == _ReadPipeList( pHM, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Read balancing valve list.
		if( false == _ReadBalancingValveList( pHM, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Read Dp controller valve list.
		if( false == _ReadDpControllerValveList( pHM, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Read control valve list.
		if( false == _ReadControlValveList( pHM, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Read shutoff valve list.
		if( false == _ReadShutoffValveList( pHM, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Read pump.
		if( false == _ReadPump( pHM, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}
		
		pclHydraulicCircuitList->m_vecpHydraulicCircuitList.push_back( pclHydraulicCircuit );

		// Check if the current hydraulic circuit is a module.
		if( true == pclHydraulicCircuit->m_bInModule && NULL != pHM->GetFirstHMChild() )
		{
			pclHydraulicCircuit->m_pclHydraulicChildrenCircuitList = new CImportExportHMData::CHydraulicCircuitListData();
			pclHydraulicCircuit->m_pclHydraulicChildrenCircuitList->m_pclHydraulicCircuitParent = pclHydraulicCircuitList;
			pclHydraulicCircuit->m_pclHydraulicChildrenCircuitList->m_pclHydraulicNetworkData = pclHydraulicCircuitList->m_pclHydraulicNetworkData;

			if( false == _ReadHydraulicCircuitList( pHM, pclHydraulicCircuit->m_pclHydraulicChildrenCircuitList, strErrorMsg ) )
			{
				return false;
			}
		}
	}

	return true;
}

bool CImportExportHM::_ReadTerminalUnitList( CDS_HydroMod *pHM, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	if( false == pclHydraulicCircuit->m_bInModule )
	{
		if( NULL == pHM->GetpTermUnit() )
		{
			strErrorMsg = _T("Terminal unit must be defined for a hydraulic circuit of type 'circuit'.");
			return false;
		}
	}

	CImportExportHMData::CTerminalUnitData *pclTerminalUnitData = new CImportExportHMData::CTerminalUnitData();

	// Flow.
	pclTerminalUnitData->m_eFlowType = pHM->GetpTermUnit()->GetQType();

	if( CTermUnit::_QType::Q == pclTerminalUnitData->m_eFlowType )
	{
		pclTerminalUnitData->m_dFlow = pHM->GetpTermUnit()->GetQ();
	}
	else
	{
		pclTerminalUnitData->m_dPower = pHM->GetpTermUnit()->GetP();
		pclTerminalUnitData->m_dDT = pHM->GetpTermUnit()->GetDT();
	}

	// Dp.
	pclTerminalUnitData->m_ePressureDropType = pHM->GetpTermUnit()->GetDpType();

	if( CDS_HydroMod::eDpType::Dp == pclTerminalUnitData->m_ePressureDropType )
	{
		pclTerminalUnitData->m_dPressureDrop = pHM->GetpTermUnit()->GetDp();
	}
	else if( CDS_HydroMod::eDpType::QDpref == pclTerminalUnitData->m_ePressureDropType )
	{
		pclTerminalUnitData->m_dFlowRef = pHM->GetpTermUnit()->GetQRef();
		pclTerminalUnitData->m_dPressureDropRef = pHM->GetpTermUnit()->GetDpRef();
	}
	else if( CDS_HydroMod::eDpType::Kv == pclTerminalUnitData->m_ePressureDropType )
	{
		pclTerminalUnitData->m_dKv = pHM->GetpTermUnit()->GetKv();
	}
	else if( CDS_HydroMod::eDpType::Cv == pclTerminalUnitData->m_ePressureDropType )
	{
		pclTerminalUnitData->m_dCv = pHM->GetpTermUnit()->GetCv();
	}

	pclTerminalUnitData->m_pclHydraulicCircuitParent = pclHydraulicCircuit;
	pclHydraulicCircuit->m_vecpTerminalUnitList.push_back( pclTerminalUnitData );

	return true;
}

bool CImportExportHM::_ReadPipeList( CDS_HydroMod *pHM, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	for( int iLoopPipeType = 0; iLoopPipeType < 4; iLoopPipeType++ )
	{
		CPipes *pclHMPipes = NULL;

		if( 0 == iLoopPipeType )
		{
			pclHMPipes = pHM->GetpDistrSupplyPipe();
		}
		else if( 1 == iLoopPipeType )
		{
			pclHMPipes = pHM->GetpDistrReturnPipe();
		}
		else if( 2 == iLoopPipeType )
		{
			pclHMPipes = pHM->GetpCircuitPrimaryPipe();
		}
		else if( 3 == iLoopPipeType )
		{
			pclHMPipes = pHM->GetpCircuitSecondaryPipe();
		}

		if( NULL == pclHMPipes || NULL == pclHMPipes->GetIDPtr().MP || _T('\0') == pclHMPipes->GetIDPtr().ID )
		{
			continue;
		}

		CDB_Pipe *pclPipe = dynamic_cast<CDB_Pipe *>( pclHMPipes->GetIDPtr().MP );

		if( NULL == pclPipe )
		{
			strErrorMsg = _T("The object in HM pipes is not a 'CDB_Pipe' object.");
			return false;
		}

		CImportExportHMData::CPipeData *pclPipeData = new CImportExportHMData::CPipeData();

		pclPipeData->m_eInType = pclHMPipes->GetLocate();
		pclPipeData->m_strInSizeID = pclPipe->GetSizeID();
		pclPipeData->m_dInInternalDiameter = pclPipe->GetIntDiameter();
		pclPipeData->m_dInRoughness = pclPipe->GetRoughness();
		pclPipeData->m_dInLength = pclHMPipes->GetLength();
		pclPipeData->m_dSupplyTemperature = pclHMPipes->GetTemperature( CAnchorPt::PipeLocation_Supply );
		pclPipeData->m_dReturnTemperature = pclHMPipes->GetTemperature( CAnchorPt::PipeLocation_Return );
		
		pclPipeData->m_bInIsLocked = pclHMPipes->IsLocked();

		if( pclHMPipes->GetSingularityCount() > 0 )
		{
			if( false == _ReadPipeAccessoryList( pclHMPipes, pclPipeData, strErrorMsg ) )
			{
				return false;
			}
		}

		pclPipeData->m_pclHydraulicCircuitParent = pclHydraulicCircuit;
		pclHydraulicCircuit->m_vecpPipeList.push_back( pclPipeData );
	}

	return true;
}

bool CImportExportHM::_ReadPipeAccessoryList( CPipes *pclHMPipes, CImportExportHMData::CPipeData *pclPipeData, CString &strErrorMsg )
{
	for( int iLoopPipeAccessory = 0; iLoopPipeAccessory < pclHMPipes->GetSingularityCount(); iLoopPipeAccessory++ )
	{
		CPipes::CSingularity clSingularity;

		if( false == pclHMPipes->GetSingularity( iLoopPipeAccessory, &clSingularity ) )
		{
			strErrorMsg.Format( _T("Can't retrieve pipe accessory (index: %i) on the current pipe."), iLoopPipeAccessory );
			return false;
		}

		if( CDS_HydroMod::eDpType::None == clSingularity.m_eDpType && NULL == clSingularity.GetSingulIDPtr().MP || _T('\0') == clSingularity.GetSingulIDPtr().ID )
		{
			strErrorMsg.Format( _T("Pipe accessory found (index: %i) but not defined."), iLoopPipeAccessory );
			return false;
		}

		CImportExportHMData::CPipeAccessoryData *pclPipeAccessoryData = new CImportExportHMData::CPipeAccessoryData();

		pclPipeAccessoryData->m_eType = clSingularity.m_eDpType;

		if( CDS_HydroMod::eDpType::None == pclPipeAccessoryData->m_eType )
		{
			pclPipeAccessoryData->m_bIsElbow = ( 0 == CString( clSingularity.GetSingulIDPtr().ID ).Compare( _T("ELBOW_90") ) ) ? true : false;
		}
		else if( CDS_HydroMod::eDpType::dzeta == pclPipeAccessoryData->m_eType )
		{
			pclPipeAccessoryData->m_dZetaValue = clSingularity.m_uDpVal.Kv;
		}
		else if( CDS_HydroMod::eDpType::Dp == pclPipeAccessoryData->m_eType )
		{
			pclPipeAccessoryData->m_dPressureDrop = clSingularity.m_uDpVal.Dp;
		}
		else if( CDS_HydroMod::eDpType::QDpref == pclPipeAccessoryData->m_eType )
		{
			pclPipeAccessoryData->m_dFlowRef = clSingularity.m_uDpVal.sQDpRef.Qref;
			pclPipeAccessoryData->m_dPressureDropRef = clSingularity.m_uDpVal.sQDpRef.Dpref;
		}
		else if( CDS_HydroMod::eDpType::Kv == pclPipeAccessoryData->m_eType )
		{
			pclPipeAccessoryData->m_dPressureDrop = clSingularity.m_uDpVal.Kv;
		}
		else if( CDS_HydroMod::eDpType::Cv == pclPipeAccessoryData->m_eType )
		{
			pclPipeAccessoryData->m_dPressureDrop = clSingularity.m_uDpVal.Cv;
		}

		pclPipeAccessoryData->m_pclPipeParent = pclPipeData;
		pclPipeData->m_vecpInPipeAccessoryList.push_back( pclPipeAccessoryData );
	}

	return true;
}
	     	
bool CImportExportHM::_ReadBalancingValveList( CDS_HydroMod *pHM, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	for( int iLoopBalancingValve = 0; iLoopBalancingValve < 3; iLoopBalancingValve++ )
	{
		CDS_HydroMod::CBV *pclHMBalancingValve = NULL;

		if( 0 == iLoopBalancingValve )
		{
			pclHMBalancingValve = pHM->GetpBv();
		}
		else if( 0 == iLoopBalancingValve )
		{
			pclHMBalancingValve = pHM->GetpSecBv();
		}
		else
		{
			pclHMBalancingValve = pHM->GetpBypBv();
		}

		if( NULL == pclHMBalancingValve || NULL == pclHMBalancingValve->GetIDPtr().MP || _T('\0') == pclHMBalancingValve->GetIDPtr().ID )
		{
			continue;
		}

		CDB_RegulatingValve *pclBalancingValve = dynamic_cast<CDB_RegulatingValve *>( pclHMBalancingValve->GetIDPtr().MP );

		if( NULL == pclBalancingValve )
		{
			strErrorMsg = _T("The object in 'CDS_HydroMod::CBV' is not a 'CDB_RegulatingValve' object.");
			return false;
		}

		CImportExportHMData::CBalancingValveData *pclBalancingValveData = new CImportExportHMData::CBalancingValveData();

		pclBalancingValveData->m_eInLocation = pclHMBalancingValve->GetHMObjectType();
		pclBalancingValveData->m_strInSizeID = pclBalancingValve->GetIDPtr().ID;
		pclBalancingValveData->m_dFlow = pclHMBalancingValve->GetQ();
		pclBalancingValveData->m_bInIsLocked = pHM->IsLocked( pclBalancingValveData->m_eInLocation );

		pclBalancingValveData->m_pclHydraulicCircuitParent = pclHydraulicCircuit;
		pclHydraulicCircuit->m_vecpBalancingValveList.push_back( pclBalancingValveData );
	}

	return true;
}

bool CImportExportHM::_ReadDpControllerValveList( CDS_HydroMod *pHM, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	CDS_HydroMod::CDpC *pclHMDpController = pHM->GetpDpC();

	if( NULL == pclHMDpController || NULL == pclHMDpController->GetIDPtr().MP || _T('\0') == pclHMDpController->GetIDPtr().ID )
	{
		return true;
	}

	CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( pclHMDpController->GetIDPtr().MP );

	if( NULL == pclDpController )
	{
		strErrorMsg = _T("The object in 'CDS_HydroMod::CDpC' is not a 'CDB_DpController' object.");
		return false;
	}

	CImportExportHMData::CDpControllerValveData *pclDpControllerValveData = new CImportExportHMData::CDpControllerValveData();

	pclDpControllerValveData->m_strInSizeID = pclDpController->GetSizeID();
	pclDpControllerValveData->m_dFlow = pclHMDpController->GetQ();
	pclDpControllerValveData->m_bInIsLocked = pHM->IsLocked( CDS_HydroMod::eHMObj::eDpC );

	pclDpControllerValveData->m_pclHydraulicCircuitParent = pclHydraulicCircuit;
	pclHydraulicCircuit->m_vecpDpControllerValveList.push_back( pclDpControllerValveData );

	return true;
}

bool CImportExportHM::_ReadControlValveList( CDS_HydroMod *pHM, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	CDS_HydroMod::CCv *pclHMControlValve = pHM->GetpCV();

	if( NULL == pclHMControlValve || NULL == pclHMControlValve->GetCvIDPtr().MP || _T('\0') == pclHMControlValve->GetCvIDPtr().ID )
	{
		return true;
	}

	CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( pclHMControlValve->GetCvIDPtr().MP );

	if( NULL == pclControlValve )
	{
		strErrorMsg = _T("The object in 'CDS_HydroMod::CCv' is not a 'CDB_ControlValve' object.");
		return false;
	}

	CImportExportHMData::CControlValveData *pclControlValveData = new CImportExportHMData::CControlValveData();

	pclControlValveData->m_eControlType = pclHMControlValve->GetCtrlType();
	pclControlValveData->m_strInSizeID = pclControlValve->GetSizeID();
	pclControlValveData->m_dFlow = pclHMControlValve->GetQ();
	pclControlValveData->m_bInIsLocked = pHM->IsLocked( CDS_HydroMod::eHMObj::eCV );

	pclControlValveData->m_pclHydraulicCircuitParent = pclHydraulicCircuit;
	pclHydraulicCircuit->m_vecpControlValveList.push_back( pclControlValveData );

	return true;
}

bool CImportExportHM::_ReadShutoffValveList( CDS_HydroMod *pHM, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	for( int iLoopShutoffValve = 0; iLoopShutoffValve < 2; iLoopShutoffValve++ )
	{
		CDS_HydroMod::CShutoffValve *pclHMShutoffValve = NULL;
		
		CDS_HydroMod::eHMObj eSVType = ( 0 == iLoopShutoffValve ) ? CDS_HydroMod::eHMObj::eShutoffValveSupply : CDS_HydroMod::eHMObj::eShutoffValveReturn;
		pclHMShutoffValve = pHM->GetpShutoffValve( eSVType );

		if( NULL == pclHMShutoffValve || NULL == pclHMShutoffValve->GetIDPtr().MP || _T('\0') == pclHMShutoffValve->GetIDPtr().ID )
		{
			continue;
		}

		CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve *>( pclHMShutoffValve->GetIDPtr().MP );

		if( NULL == pclShutoffValve )
		{
			strErrorMsg = _T("The object in 'CDS_HydroMod::CSV' is not a 'CDB_ShutoffValve' object.");
			return false;
		}
		
		CImportExportHMData::CShutoffValveData *pclShutoffValveData = new CImportExportHMData::CShutoffValveData();

		pclShutoffValveData->m_strInSizeID = pclShutoffValve->GetSizeID();
		pclShutoffValveData->m_dFlow = pclHMShutoffValve->GetQ();
		pclShutoffValveData->m_bInIsLocked = pHM->IsLocked( eSVType );

		pclShutoffValveData->m_pclHydraulicCircuitParent = pclHydraulicCircuit;
		pclHydraulicCircuit->m_vecpShutoffValveList.push_back( pclShutoffValveData );
	}

	return true;
}

bool CImportExportHM::_ReadPump( CDS_HydroMod *pHM, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	pclHydraulicCircuit->m_Pump.m_bExist = false;
	CPump *pclHMPump = pHM->GetpPump();

	if( NULL == pclHMPump )
	{
		return true;
	}

	pclHydraulicCircuit->m_Pump.m_bExist = true;
	pclHydraulicCircuit->m_Pump.m_dHMin = pclHMPump->GetHmin();

	return true;
}
