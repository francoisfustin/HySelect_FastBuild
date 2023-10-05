#include "stdafx.h"
#include "global.h"
#include "Utilities.h"
#include "TASelect.h"
#include "Hydronic.h"
#include "Select.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "HMPipes.h"

////////////////////////////////////////////////////////////////////

CTermUnit::CTermUnit( CDS_HydroMod *pHM )
{
	m_pParent = pHM;
	m_TermUnit.m_strDescription = _T("");
	m_bVirtual = false;
	m_TermUnit.m_eDpType = CDS_HydroMod::eDpType::Dp;
	m_TermUnit.m_eQType = _QType::Q;
	m_TermUnit.m_uDpVal.Dp = 1.0;
	m_TermUnit.m_uDpVal.sQDpRef.Dpref = 0.0;
	m_TermUnit.m_uDpVal.sQDpRef.Qref = 0.0;
	m_TermUnit.m_dQ = 0.0;
	m_TermUnit.m_dP = 0.0;
	m_TermUnit.m_dDT = 0.0;
	m_rDpValues.Reset();
	m_pTADS = TASApp.GetpTADS();
	m_dPartialQ = 0.0;
	m_dTotQwoDiversity = 0.0;
}

void CTermUnit::SetQType( enum _QType eQType )
{ 
	m_TermUnit.m_eQType = eQType;
}

void CTermUnit::SetQ( double dQ , bool bComputeHM )
{
	m_TermUnit.m_eQType = _QType::Q;

	if( dQ != DBL_MAX )
	{
		m_TermUnit.m_dQ = dQ;
	}

	if( dQ <= 0.0 )
	{
		return;
	}

	// Reset power.
	m_TermUnit.m_dP = 0.0;

	m_pParent->SetHMPrimAndSecFlow( dQ );

	// Update QwoDiversity if not a module.
	if( false == m_pParent->IsaModule() ) 
	{
		SetTotQwoDiversity( dQ );
	}

	if (true == bComputeHM)
	{
		m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceTUQ );
	}

	m_pParent->Modified();
}

void CTermUnit::SetPdT( double dP, double dDT, bool bComputeHM )
{
	SetPdTOnlyValues( dP, dDT );
	
	double dQ = GetQ();
	
	m_pParent->SetHMPrimAndSecFlow( dQ );
	
	// Update QwoDiversity if not a module.
	if( false == m_pParent->IsaModule() )
	{
		SetTotQwoDiversity( dQ );
	}
	
	// HYS-1872
	if( true == bComputeHM )
	{
		m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceTUQ );
	}

	m_pParent->Modified();
}

void CTermUnit::SetPdTOnlyValues( double dP, double dDT )
{
	m_TermUnit.m_eQType = _QType::PdT;

	if( DBL_MAX != dP )
	{
		m_TermUnit.m_dP = dP; 
	}
	
	if( DBL_MAX != dDT )
	{
		m_TermUnit.m_dDT = dDT; 
	}

	// Reset to 0 to force a new compute.
	m_TermUnit.m_dQ = 0.0;
}

void CTermUnit::SetDTFlowMode( double dDT, bool bComputeHM )
{
	m_TermUnit.m_eQType = _QType::Q;

	if( DBL_MAX != dDT )
	{
		m_TermUnit.m_dDT = dDT; 
	}

	if( 0 == dDT )
	{
		return;
	}

	// Reset to 0 to force a new compute.
	m_TermUnit.m_dP = 0.0;

	if( true == bComputeHM )
	{
		// for fluid characteristics.
		m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
	}

	m_pParent->Modified();
}

double CTermUnit::GetDp()
{
	// HYS-1716: To compute the pressure drop of the terminal unit, we need the temperature that is on the supply.
	CWaterChar *pclWaterChar = m_pParent->GetpOutWaterChar( CAnchorPt::PipeLocation_Supply );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	double dRho = pclWaterChar->GetDens();
	double dDp = 0.0;
	double dQ = GetQ();
	
	switch( m_TermUnit.m_eDpType )
	{
		case CDS_HydroMod::eDpType::Dp:
			dDp = m_TermUnit.m_uDpVal.Dp;	
			break;

		case CDS_HydroMod::eDpType::Kv:
			if( dQ > 0.0 && m_TermUnit.m_uDpVal.Kv > 0.0 )
				dDp = CalcDp( dQ, m_TermUnit.m_uDpVal.Kv, dRho );	
			break;

		case CDS_HydroMod::eDpType::Cv:
			if( dQ > 0.0 && m_TermUnit.m_uDpVal.Cv > 0.0 )
			{
				// HYS-1274: convert CV to KV before calculate Dp
				// Convert Cv To Kv.
				double dVal = m_TermUnit.m_uDpVal.Cv;
				CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
				if( NULL != pUnitDB )
				{
					UnitDesign_struct ud = pUnitDB->GetUnit( _C_KVCVCOEFF, 1 );				
					dVal = GetConvOf( ud ) * ( dVal + GetOffsetOf( ud ) );
				}
				dDp = CalcDp( dQ, dVal, dRho );
			}
			break;

		case CDS_HydroMod::eDpType::dzeta:
			if( dQ > 0.0 && m_TermUnit.m_uDpVal.sDzeta.dzeta > 0.0 && m_TermUnit.m_uDpVal.sDzeta.d > 0.0 )
			{
				double dKv = CalcKvFromDzeta( m_TermUnit.m_uDpVal.sDzeta.dzeta, m_TermUnit.m_uDpVal.sDzeta.d );
				dDp = CalcDp( dQ, dKv, dRho );	
			}
			break;

		case CDS_HydroMod::eDpType::QDpref:
			if( dQ > 0.0 && m_TermUnit.m_uDpVal.sQDpRef.Qref > 0.0 && m_TermUnit.m_uDpVal.sQDpRef.Dpref > 0.0 )
			{
				double dKvRef;
				dKvRef = CalcKv( m_TermUnit.m_uDpVal.sQDpRef.Qref, m_TermUnit.m_uDpVal.sQDpRef.Dpref, dRho );
				dDp = CalcDp( dQ, dKvRef, dRho );	
			}
			break;
	}
	return dDp;
}

double CTermUnit::GetDp( CDS_HydroMod::eDpType *peDpType, double *pdVal1, double *pdVal2 )
{
	if( NULL != peDpType )
	{
		*peDpType = m_TermUnit.m_eDpType;
	}

	switch( m_TermUnit.m_eDpType )
	{
		case CDS_HydroMod::eDpType::Dp:

			if( NULL != pdVal1 )
			{
				*pdVal1 = m_TermUnit.m_uDpVal.Dp;
			}

			break;

		case CDS_HydroMod::eDpType::Kv:

			if( NULL != pdVal1 )
			{
				*pdVal1 = m_TermUnit.m_uDpVal.Kv;
			}

			break;

		case CDS_HydroMod::eDpType::Cv:

			if( NULL != pdVal1 )
			{
				*pdVal1 = m_TermUnit.m_uDpVal.Cv;
			}

			break;

		case CDS_HydroMod::eDpType::dzeta:

			if( NULL != pdVal1 )
			{
				*pdVal1 = m_TermUnit.m_uDpVal.sDzeta.dzeta;
			}

			if( NULL != pdVal2 )
			{
				*pdVal2 = m_TermUnit.m_uDpVal.sDzeta.d;
			}

			break;

		case CDS_HydroMod::eDpType::QDpref:

			if( NULL != pdVal1 )
			{
				*pdVal1 = m_TermUnit.m_uDpVal.sQDpRef.Dpref;
			}

			if( NULL != pdVal2 )
			{
				*pdVal2 = m_TermUnit.m_uDpVal.sQDpRef.Qref;
			}

			break;

	}

	return GetDp();
}

double CTermUnit::GetKv()
{
	double dKv = 0.0;

	if( CDS_HydroMod::eDpType::Kv == m_TermUnit.m_eDpType )
	{
		dKv = m_TermUnit.m_uDpVal.Kv;
	}

	return dKv;
}

double CTermUnit::GetCv()
{
	double dCv = 0.0;

	if( CDS_HydroMod::eDpType::Cv == m_TermUnit.m_eDpType )
	{
		dCv = m_TermUnit.m_uDpVal.Cv;
	}

	return dCv;
}

double CTermUnit::GetDpRef()
{
	double dDpRef = 0.0;

	if( CDS_HydroMod::eDpType::QDpref == m_TermUnit.m_eDpType )
	{
		dDpRef = m_TermUnit.m_uDpVal.sQDpRef.Dpref;
	}

	return dDpRef;
}

double CTermUnit::GetQRef()
{
	double dQRef = 0.0;

	if( CDS_HydroMod::eDpType::QDpref == m_TermUnit.m_eDpType )
	{
		dQRef = m_TermUnit.m_uDpVal.sQDpRef.Qref;
	}

	return dQRef;
}

void CTermUnit::GetDpQRef( double &dDpRef, double &dQRef )
{
	dDpRef = m_TermUnit.m_uDpVal.sQDpRef.Dpref;
	dQRef = m_TermUnit.m_uDpVal.sQDpRef.Qref;
}

void CTermUnit::SetDp( CDS_HydroMod::eDpType eDpType, double dVal1, double dVal2, bool bCompute )
{
	m_TermUnit.m_eDpType = eDpType;

	switch( eDpType )
	{
		case CDS_HydroMod::eDpType::Dp:

			if( dVal1 != DBL_MAX )
			{
				m_TermUnit.m_uDpVal.Dp = dVal1;
			}

			break;

		case CDS_HydroMod::eDpType::Kv:

			if( dVal1 != DBL_MAX )
			{
				m_TermUnit.m_uDpVal.Kv = dVal1;	
			}

			m_TermUnit.m_uDpVal.sDzeta.d = 0.0;
			break;
		
		case CDS_HydroMod::eDpType::Cv:

			if( dVal1 != DBL_MAX )
			{
				m_TermUnit.m_uDpVal.Cv = dVal1;	
			}

			m_TermUnit.m_uDpVal.sDzeta.d = 0.0;
			break;
		
		case CDS_HydroMod::eDpType::dzeta:

			if( dVal1 != DBL_MAX )
			{
				m_TermUnit.m_uDpVal.sDzeta.dzeta = dVal1;
			}

			if( dVal2 != DBL_MAX )
			{
				m_TermUnit.m_uDpVal.sDzeta.d = dVal2;
			}

			break;
		
		case CDS_HydroMod::eDpType::QDpref:

			if( dVal1 != DBL_MAX )
			{
				m_TermUnit.m_uDpVal.sQDpRef.Dpref = dVal1;
			}

			if( dVal2 != DBL_MAX )
			{
				m_TermUnit.m_uDpVal.sQDpRef.Qref = dVal2;
			}

			break;
	}

	if (true == bCompute)
	{
		m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceTUDp );
	}

	m_pParent->Modified();
}

double CTermUnit::GetQ()
{
	double dFlow = 0.0;

	// HYS-1882: We should consider Qtype to update if it's necessary. See FIX HYS-1882.docx in Jira card for more details.
	if( ( m_TermUnit.m_dQ > 0.0  ) && ( m_TermUnit.m_dP <= 0 || _QType::Q == GetQType() ) )
	{
		dFlow = m_TermUnit.m_dQ;
	}
	else if( ( m_TermUnit.m_dP > 0.0 ) && ( m_TermUnit.m_dQ <= 0.0 || _QType::PdT == GetQType() ) )
	{
		// HYS-1746.
		CWaterChar clWaterCharIN;
		CWaterChar clWaterCharOUT;

		// Because the flow is computed with density (Rho) and specific heat (Cp) for both water characteristics at the
		// inlet and outlet of the terminal unit, we need to retrieve these characteristic from the pipes on which is 
		// located the terminal unit.
		if( false == m_pParent->GetWaterCharTU( clWaterCharIN, clWaterCharOUT ) )
		{
			ASSERTA_RETURN( 0.0 );
		}

		if( -273.15 == clWaterCharIN.GetTemp() || -273.15 == clWaterCharOUT.GetTemp() )
		{
			ASSERTA_RETURN( 0.0 );
		}

		// HYS-1833: If temperatures are the same, we return -1.0.
		if( clWaterCharIN.GetTemp() == clWaterCharOUT.GetTemp() )
		{
			return -1.0;
		}

		// HYS-1882: Do not update temperatures here.
				
		double dDenominator = clWaterCharIN.GetTemp() * clWaterCharIN.GetDens() * clWaterCharIN.GetSpecifHeat();
		dDenominator -= ( clWaterCharOUT.GetTemp() * clWaterCharOUT.GetDens() * clWaterCharOUT.GetSpecifHeat() );
		m_TermUnit.m_dQ = m_TermUnit.m_dP / abs( dDenominator );
		dFlow = m_TermUnit.m_dQ;
	}

	return dFlow;
}

double CTermUnit::GetP()
{
	double dPower = 0.0;
	
	// HYS-1882: We should consider Qtype to update if it's necessary. See FIX HYS-1882.docx in Jira card for more details.
	if( ( m_TermUnit.m_dP > 0.0 ) && ( m_TermUnit.m_dQ <= 0.0 || _QType::PdT == GetQType() ) )
	{
		dPower = m_TermUnit.m_dP;
	}
	else if( ( m_TermUnit.m_dQ > 0.0 ) && ( m_TermUnit.m_dP <= 0 || _QType::Q == GetQType() ) )
	{
		// HYS-1746.
		CWaterChar clWaterCharIN;
		CWaterChar clWaterCharOUT;

		// Because the power is computed with density (Rho) and specific heat (Cp) for both water characteristics at the
		// inlet and outlet of the terminal unit, we need to retrieve these characteristic from the pipes on which is 
		// located the terminal unit.
		if( false == m_pParent->GetWaterCharTU( clWaterCharIN, clWaterCharOUT ) )
		{
			ASSERTA_RETURN( 0.0 );
		}

		if( -273.15 == clWaterCharIN.GetTemp() || -273.15 == clWaterCharOUT.GetTemp() )
		{
			ASSERTA_RETURN( 0.0 );
		}

		// HYS-1882: Do not update temperature here.
				
		m_TermUnit.m_dP = clWaterCharIN.GetTemp() * clWaterCharIN.GetDens() * clWaterCharIN.GetSpecifHeat();
		m_TermUnit.m_dP -= ( clWaterCharOUT.GetTemp() * clWaterCharOUT.GetDens() * clWaterCharOUT.GetSpecifHeat() );
		m_TermUnit.m_dP *= m_TermUnit.m_dQ;
		m_TermUnit.m_dP = abs( m_TermUnit.m_dP );
		dPower = m_TermUnit.m_dP;
	}

	return dPower;
}

double CTermUnit::GetDT()
{
	if( m_TermUnit.m_eQType != _QType::PdT )
	{
		return 0.0;
	}

	if( 0.0 == m_TermUnit.m_dDT )
	{
		CWaterChar clWaterCharIN;
		CWaterChar clWaterCharOUT;

		// Retrieve the water characteristics from the inlet and outlet of the terminal unit.
		// Remark: 'CDS_HydroMod::GetWaterCharTU' will take care if it's an injection circuit or not to send back the
		//         good water characteristics.
		if( false == m_pParent->GetWaterCharTU( clWaterCharIN, clWaterCharOUT ) )
		{
			ASSERTA_RETURN( 0.0 );
		}

		if( -273.15 == clWaterCharIN.GetTemp() || -273.15 == clWaterCharOUT.GetTemp() )
		{
			ASSERTA_RETURN( 0.0 );
		}

		m_TermUnit.m_dDT = abs( clWaterCharIN.GetTemp() - clWaterCharOUT.GetTemp() );
	}

	return m_TermUnit.m_dDT;
}

double CTermUnit::GetDTFlowMode( bool bNeedsInitValue )
{
	if( m_TermUnit.m_eQType != _QType::Q )
	{
		return 0.0;
	}

	if( 0.0 == m_TermUnit.m_dDT && true == bNeedsInitValue )
	{
		CWaterChar clWaterCharIN;
		CWaterChar clWaterCharOUT;

		// Retrieve the water characteristics from the inlet and outlet of the terminal unit.
		// Remark: 'CDS_HydroMod::GetWaterCharTU' will take care if it's an injection circuit or not to send back the
		//         good water characteristics.
		if( false == m_pParent->GetWaterCharTU( clWaterCharIN, clWaterCharOUT ) )
		{
			ASSERTA_RETURN( 0.0 );
		}

		if( -273.15 == clWaterCharIN.GetTemp() || -273.15 == clWaterCharOUT.GetTemp() )
		{
			ASSERTA_RETURN( 0.0 );
		}

		m_TermUnit.m_dDT = abs( clWaterCharIN.GetTemp() - clWaterCharOUT.GetTemp() );
	}

	return m_TermUnit.m_dDT;
}

double CTermUnit::UpdateDT( double dDtToSet )
{
	double dDT = 0.0;

	if( 0 != dDtToSet )
	{
		m_TermUnit.m_dDT = dDtToSet;
		dDT = dDtToSet;
	}
	else
	{
		CWaterChar clWaterCharIN;
		CWaterChar clWaterCharOUT;

		// Retrieve the water characteristics from the inlet and outlet of the terminal unit.
		// Remark: 'CDS_HydroMod::GetWaterCharTU' will take care if it's an injection circuit or not to send back the
		//         good water characteristics.
		if( false == m_pParent->GetWaterCharTU( clWaterCharIN, clWaterCharOUT ) )
		{
			ASSERTA_RETURN( 0.0 );
		}

		if( -273.15 == clWaterCharIN.GetTemp() || -273.15 == clWaterCharOUT.GetTemp() )
		{
			ASSERTA_RETURN( 0.0 );
		}

		dDT = abs( clWaterCharIN.GetTemp() - clWaterCharOUT.GetTemp() );
	}

	return dDT;
}

double CTermUnit::DpToDpref()
{
	// HYS-1716: To compute the pressure drop reference of the terminal unit, we need the temperature that is on the supply.
	CWaterChar *pclWaterChar = m_pParent->GetpOutWaterChar( CAnchorPt::PipeLocation_Supply );

	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	double dRho = pclWaterChar->GetDens();
	double dDp = m_TermUnit.m_uDpVal.Dp;
	double dDpref = 0.0;
	double dQref = m_TermUnit.m_uDpVal.sQDpRef.Qref;
	double dQ = GetQ();
	
	if( dQ > 0.0 && dDp > 0.0 && dQref > 0.0 )
	{
		double dKv;
		dKv = CalcKv( dQ, dDp, dRho );
		dDpref = CalcDp( dQref, dKv, dRho );
	}
	
	return dDpref;
}

void CTermUnit::CopyTo( CTermUnit *pclDestTerminalUnit )
{
	m_TermUnit.CopyTo( pclDestTerminalUnit->GetTerminalUnitData() );
	pclDestTerminalUnit->SetDescription( m_TermUnit.m_strDescription );
	pclDestTerminalUnit->SetVirtual( IsVirtual() );
	pclDestTerminalUnit->SetTotQwoDiversity(m_dTotQwoDiversity);

	_DpVal* prWorkValues = pclDestTerminalUnit->GetpDpWorkStructure();
	CopyMemory( prWorkValues, &m_rDpValues, sizeof( _DpVal ) );
}

CString CTermUnit::GetFlowString()
{
	CString strFlowString = _T("");

	if( _QType::Q == m_TermUnit.m_eQType )
	{
		strFlowString = WriteCUDouble( _U_FLOW, GetQ() );
	}

	return strFlowString;
}

CString CTermUnit::GetPowerString()
{
	CString strPowerString = _T("");
	
	if( _QType::PdT == m_TermUnit.m_eQType )
	{
		strPowerString = WriteCUDouble( _U_TH_POWER, GetP() );
	}

	return strPowerString;
}

CString CTermUnit::GetDTString()
{
	CString strDTString = _T("");
	
	if( _QType::PdT == m_TermUnit.m_eQType )
	{
		strDTString = WriteCUDouble( _U_DIFFTEMP, GetDT() );
	}	
	else if( _QType::Q == m_TermUnit.m_eQType )
	{
		// HYS-1882
		strDTString = WriteCUDouble( _U_DIFFTEMP, GetDTFlowMode() );
	}

	return strDTString;
}

CString CTermUnit::GetDpString()
{
	CString strDpString = _T("");
	
	if( CDS_HydroMod::eDpType::Dp == m_TermUnit.m_eDpType )
	{
		strDpString = WriteCUDouble( _U_DIFFPRESS, GetDp() );
	}

	return strDpString;
}

CString CTermUnit::GetKvString()
{
	CString strKvString = _T("");
	
	if( CDS_HydroMod::eDpType::Kv == m_TermUnit.m_eDpType )
	{
		strKvString = WriteCUDouble( _C_KVCVCOEFF, GetKv() );
	}

	return strKvString;
}

CString CTermUnit::GetCvString()
{
	CString strCvString = _T("");
	
	if( CDS_HydroMod::eDpType::Cv == m_TermUnit.m_eDpType )
	{
		strCvString = WriteCUDouble( _C_KVCVCOEFF, GetCv() );
	}

	return strCvString;
}

CString CTermUnit::GetDpRefString()
{
	CString strDpRefString = _T("");
	
	if( CDS_HydroMod::eDpType::QDpref == m_TermUnit.m_eDpType )
	{
		strDpRefString = WriteCUDouble( _U_DIFFPRESS, GetDpRef() );
	}

	return strDpRefString;
}

CString CTermUnit::GetQRefString()
{
	CString strQRefString = _T("");
	
	if( CDS_HydroMod::eDpType::QDpref == m_TermUnit.m_eDpType )
	{
		strQRefString = WriteCUDouble( _U_FLOW, GetQRef() );
	}

	return strQRefString;
}

bool CTermUnit::IsCompletelyDefined()
{
	if( true == IsVirtual() )
	{
		return true;
	}
	
	if( GetQ() <= 0.0 )
	{
		return false;
	}
	
	// HYS-1882
	if( _QType::Q == GetQType() && true == m_pParent->IsDTFieldDisplayedInFlowMode() && GetDTFlowMode() <= 0 )
	{
		return false;
	}

	if( false == m_pParent->IsClass( CLASS( CDS_HmHub ) ) && false == m_pParent->IsClass( CLASS( CDS_HmHubStation ) ) )
	{
		if( GetDp() <= 0.0 )
		{
			return false;
		}
	}
	
	return true;
}

void CTermUnit::DropData( OUTSTREAM outf, int* piLevel )
{
	CString strTab = _T("");

	for( int i = *piLevel; i; i-- )
	{
		strTab += _T("\t");
	}

	CString str;
	str.Format( _T("m_dDT = %s\n"), WriteCUDouble( _U_DIFFTEMP, m_rDpValues.m_dDT, true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dTs = %s\n"), WriteCUDouble( _U_TEMPERATURE, m_rDpValues.m_dTs, true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dTr = %s\n"), WriteCUDouble( _U_TEMPERATURE, m_rDpValues.m_dTr, true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dKv = %s\n"), WriteCUDouble( _C_KVCVCOEFF, m_rDpValues.m_dKv ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dCv = %s\n"), WriteCUDouble( _C_KVCVCOEFF, m_rDpValues.m_dCv ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dQRef = %s\n"), WriteCUDouble( _U_FLOW, m_rDpValues.m_dQRef, true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dDpRef = %s\n"), WriteCUDouble( _U_DIFFPRESS, m_rDpValues.m_dDpRef, true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dDzeta = %s\n"), WriteDouble( m_rDpValues.m_dDzeta, 2, 2 ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dDiameter = %s\n"), WriteCUDouble( _U_DIAMETER, m_rDpValues.m_dDiameter, true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_pParent name = %s\n"), m_pParent->GetHMName() );
	WriteFormatedStringA2( outf, str, strTab );
}

#define CTERMUNIT_VERSION	5
// Version 4: HYS-1716: TsTr mode has been definitively deleted.
void CTermUnit::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CTERMUNIT_VERSION;
	outf.write( (char*)&Version, sizeof( Version ) );
	
	// Info.
	WriteString( outf, (LPCTSTR)m_TermUnit.m_strDescription );
	outf.write( (char*)&m_TermUnit.m_eDpType, sizeof( m_TermUnit.m_eDpType ) );
	outf.write( (char*)&m_TermUnit.m_eQType, sizeof( m_TermUnit.m_eQType ) );

	switch( m_TermUnit.m_eDpType )
	{
		case CDS_HydroMod::eDpType::Dp:
			outf.write( (char*)&m_TermUnit.m_uDpVal.Dp, sizeof( m_TermUnit.m_uDpVal.Dp ) );
			break;
		
		case CDS_HydroMod::eDpType::Kv:
			outf.write( (char*)&m_TermUnit.m_uDpVal.Kv, sizeof( m_TermUnit.m_uDpVal.Kv ) );
			break;
		
		case CDS_HydroMod::eDpType::Cv:
			outf.write( (char*)&m_TermUnit.m_uDpVal.Cv, sizeof( m_TermUnit.m_uDpVal.Cv ) );
			break;
		
		case CDS_HydroMod::eDpType::dzeta:
			outf.write( (char*)&m_TermUnit.m_uDpVal.sDzeta.dzeta, sizeof( m_TermUnit.m_uDpVal.sDzeta.dzeta ) );
			outf.write( (char*)&m_TermUnit.m_uDpVal.sDzeta.d, sizeof( m_TermUnit.m_uDpVal.sDzeta.d ) );
			break;
		
		case CDS_HydroMod::eDpType::QDpref:
			outf.write( (char*)&m_TermUnit.m_uDpVal.sQDpRef.Dpref, sizeof( m_TermUnit.m_uDpVal.sQDpRef.Dpref ) );
			outf.write( (char*)&m_TermUnit.m_uDpVal.sQDpRef.Qref, sizeof( m_TermUnit.m_uDpVal.sQDpRef.Qref ) );
			break;
	}
	
	switch( m_TermUnit.m_eQType )
	{
		case _QType::PdT:
			outf.write( (char*)&m_TermUnit.m_dP, sizeof( m_TermUnit.m_dP ) );
			outf.write( (char*)&m_TermUnit.m_dDT, sizeof( m_TermUnit.m_dDT ) );
			break;
		
		case _QType::Q:
			outf.write( (char*)&m_TermUnit.m_dQ, sizeof( m_TermUnit.m_dQ ) );
			outf.write( (char*)&m_TermUnit.m_dDT, sizeof( m_TermUnit.m_dDT ) );
			break;
	}

	outf.write( (char*)&m_bVirtual, sizeof( m_bVirtual ) );
	outf.write( (char*)&m_dTotQwoDiversity, sizeof( m_dTotQwoDiversity ) );
}

bool CTermUnit::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CTERMUNIT_VERSION;
	inpf.read( (char*)&Version, sizeof( Version ) );

	if( Version < 1 || Version > CTERMUNIT_VERSION )
	{
		return false;
	}
	
	// Info.
	TCHAR buf[CSTRING_LINE_SIZE_MAX];
	ReadString( inpf, buf, sizeof( buf ) );
	m_TermUnit.m_strDescription=buf;
	inpf.read( (char*)&m_TermUnit.m_eDpType, sizeof( m_TermUnit.m_eDpType ) );
	inpf.read( (char*)&m_TermUnit.m_eQType, sizeof( m_TermUnit.m_eQType ) );

	switch( m_TermUnit.m_eDpType )
	{
		case CDS_HydroMod::eDpType::Dp:
			inpf.read( (char*)&m_TermUnit.m_uDpVal.Dp, sizeof( m_TermUnit.m_uDpVal.Dp ) );

			if( Version > 1 )
			{
				break;	// break missing in version 1
			}

		case CDS_HydroMod::eDpType::Kv:
			inpf.read( (char*)&m_TermUnit.m_uDpVal.Kv, sizeof( m_TermUnit.m_uDpVal.Kv ) );

			if( Version > 1) 
			{
				break;
			}
		
		case CDS_HydroMod::eDpType::Cv:
			inpf.read( (char*)&m_TermUnit.m_uDpVal.Cv, sizeof( m_TermUnit.m_uDpVal.Cv ) );
			break;
		
		case CDS_HydroMod::eDpType::dzeta:
			inpf.read( (char*)&m_TermUnit.m_uDpVal.sDzeta.dzeta, sizeof( m_TermUnit.m_uDpVal.sDzeta.dzeta ) );
			inpf.read( (char*)&m_TermUnit.m_uDpVal.sDzeta.d, sizeof( m_TermUnit.m_uDpVal.sDzeta.d ) );
			break;
		
		case CDS_HydroMod::eDpType::QDpref:
			inpf.read( (char*)&m_TermUnit.m_uDpVal.sQDpRef.Dpref, sizeof( m_TermUnit.m_uDpVal.sQDpRef.Dpref ) );
			inpf.read( (char*)&m_TermUnit.m_uDpVal.sQDpRef.Qref, sizeof( m_TermUnit.m_uDpVal.sQDpRef.Qref ) );
			break;
	}
	
	switch( m_TermUnit.m_eQType )
	{
		case _QType::Q:
			inpf.read( (char*)&m_TermUnit.m_dQ, sizeof( m_TermUnit.m_dQ ) );
			if( Version > 4 )
			{
				inpf.read( (char*)&m_TermUnit.m_dDT, sizeof( m_TermUnit.m_dDT ) );
			}
			break;

		case _QType::PdT:
			inpf.read( (char*)&m_TermUnit.m_dP, sizeof( m_TermUnit.m_dP ) );
			inpf.read( (char*)&m_TermUnit.m_dDT, sizeof( m_TermUnit.m_dDT ) );
			break;

		// Keep only for old project.
		case _QType::PTsTr:

			if( Version < 4 )
			{
				inpf.read( (char*)&m_TermUnit.m_dP, sizeof( m_TermUnit.m_dP ) );

				double dTs;
				inpf.read( (char*)&dTs, sizeof( dTs ) );

				double dTr;
				inpf.read( (char*)&dTr, sizeof( dTr ) );

				m_TermUnit.m_dDT = abs( dTs - dTr );
			}
	}

	inpf.read( (char*)&m_bVirtual, sizeof( m_bVirtual ) );

	if( Version >= 3 )
	{
		inpf.read( (char*)&m_dTotQwoDiversity, sizeof( m_dTotQwoDiversity ) );
	}

	return true;
}

void CTermUnit::_STermUnit::CopyTo( CTermUnit::_STermUnit *pDest )
{
	if( NULL == pDest )
	{
		return;
	}

	pDest->m_strDescription = m_strDescription;
	pDest->m_eQType = m_eQType;
	pDest->m_dQ = m_dQ;
	pDest->m_dP = m_dP;
	pDest->m_dDT = m_dDT;
	pDest->m_eDpType = m_eDpType;
	pDest->m_uDpVal = m_uDpVal;
}