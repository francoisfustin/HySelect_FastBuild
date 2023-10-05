#include "StdAfx.h"

#include "MainFrm.h"
#include "HMInclude.h"
#include "Select.h"
#include "ProductSelectionParameters.h"

////////////////////////////////////////////////////////////////////
CPipes::CPipes( CDS_HydroMod *pHM, CDS_HydroMod::eHMObj eLocated )
{
	m_pParent = pHM;
	m_IDPtr = _NULL_IDPTR;					// Pipe IDPtr.
	m_DiversityIDPtr = _NULL_IDPTR;			// Pipe Diversity IDPtr (pipe saved before applying diversity factor).
	m_IDPtrBestPipe = _NULL_IDPTR;			// Best pipe IDPtr
	m_dLength = 0.0;						// Pipe length.
	m_pPipeSerie = NULL;
	m_dLinDp = 0.0;
	m_dRealQ = 0.0;
	m_dPartialPendingQ = 0.0;
	m_dTotalQ = 0.0;
	m_dVelocity = 0.0;
	m_pTADS = TASApp.GetpTADS();
	m_eLocated = eLocated;
	m_bIsDiversityApplied = false;			// By default no diversity was applied on this pipe.
	m_eReturnType = pHM->GetReturnType();	// See comments in '.h' for this variable.
}

CPipes::~CPipes()
{
	RemoveAllSingularities();
}

void CPipes::Copy( CPipes *pHMpipe, bool fResetDiversityFactor )
{
	pHMpipe->SetIDPtr( m_IDPtr,false );
	pHMpipe->SetLength( GetLength(), false );
	pHMpipe->SetRealQ( GetRealQ() );
	pHMpipe->SetLocate( GetLocate() );
	pHMpipe->SetBestPipeIDPtr( GetBestPipeIDPtr() );
	pHMpipe->SetDiversityAppliedFlag( IsDiversityApplied() );
	pHMpipe->SetDiversityIDPtr( ( true == fResetDiversityFactor ) ? _NULL_IDPTR : GetDiversityIDPtr() );
	pHMpipe->SetTotalQ( GetTotalQ() );
	pHMpipe->SetPartialPendingQ( GetPartialPendingQ() );
	pHMpipe->SetLinDp( GetLinDp() );
	pHMpipe->SetVelocity( GetVelocity() );
	pHMpipe->SetWaterChar( GetpWaterChar( CAnchorPt::PipeLocation_Supply ), CAnchorPt::PipeLocation_Supply );
	pHMpipe->SetWaterChar( GetpWaterChar( CAnchorPt::PipeLocation_Return ), CAnchorPt::PipeLocation_Return );

	pHMpipe->RemoveAllSingularities();

	for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
	{
		pHMpipe->_CopySingularity( iter->second );
	}
}

bool CPipes::Compare( CPipes *pPipes )
{
	if( m_IDPtr != pPipes->GetIDPtr() )
	{
		return false;
	}

	if( m_DiversityIDPtr != pPipes->GetDiversityIDPtr() )
	{
		return false;
	}

	if( m_pPipeSerie != pPipes->GetPipeSeries() )
	{
		return false;
	}

	if( m_IDPtrBestPipe != pPipes->GetBestPipeIDPtr() )
	{
		return false;
	}

	if( m_dLength != pPipes->GetLength() )
	{
		return false;
	}

	if( m_dRealQ != pPipes->GetRealQ() )
	{
		return false;
	}

	if( m_dTotalQ != pPipes->GetTotalQ() )
	{
		return false;
	}

	if( m_dPartialPendingQ != pPipes->GetPartialPendingQ() )
	{
		return false;
	}

	if( m_dLinDp != pPipes->GetLinDp() )
	{
		return false;
	}

	if( m_dVelocity != pPipes->GetVelocity() )
	{
		return false;
	}

	if( m_eLocated != pPipes->GetLocate() )
	{
		return false;
	}

	if( m_bIsDiversityApplied != pPipes->IsDiversityApplied() )
	{
		return false;
	}
	
	if( m_clSupplyWaterChar != *pPipes->GetpWaterChar( CAnchorPt::PipeLocation_Supply ) )
	{
		return false;
	}

	if( m_clReturnWaterChar != *pPipes->GetpWaterChar( CAnchorPt::PipeLocation_Return ) )
	{
		return false;
	}
	
	std::vector<CSingularity *> vecSingularities1;
	std::vector<CSingularity *> vecSingularities2;
	GetSingularityList( &vecSingularities1, false);
	pPipes->GetSingularityList( &vecSingularities2, false);
	
	if( vecSingularities1.size() != vecSingularities2.size() )
	{
		ClearSingularityVector( &vecSingularities1 );
		ClearSingularityVector( &vecSingularities2 );
		return false;
	}
	
	bool bFound = true;

	for( int iLoop = 0; iLoop < (int)vecSingularities1.size() && true == bFound; iLoop++ )
	{
		if( vecSingularities1[iLoop]->m_nID != vecSingularities2[iLoop]->m_nID )
		{
			bFound = false;
		}
		else if( vecSingularities1[iLoop]->m_IDPtr != vecSingularities2[iLoop]->m_IDPtr )
		{
			bFound = false;
		}
		else if( 0 != vecSingularities1[iLoop]->GetDescription().Compare( vecSingularities2[iLoop]->GetDescription() ) )
		{
			bFound = false;
		}
		else if( vecSingularities1[iLoop]->m_eDpType != vecSingularities2[iLoop]->m_eDpType )
		{
			bFound = false;
		}
		else if( 0 != memcmp( &vecSingularities1[iLoop]->m_uDpVal, &vecSingularities2[iLoop]->m_uDpVal, sizeof( CDS_HydroMod::_uDpVal ) ) )
		{
			bFound = false;
		}
	}

	ClearSingularityVector( &vecSingularities1 );
	ClearSingularityVector( &vecSingularities2 );

	return bFound;
}

IDPTR CPipes::GetIDPtr()
{
	m_IDPtr.DB = TASApp.GetpPipeDB();

	if( _T('\0') != *m_IDPtr.ID && NULL != m_IDPtr.DB )
	{
		Extend( &m_IDPtr );
	}
	else
	{
		m_IDPtr = _NULL_IDPTR;
	}

	m_pPipeSerie = (CTable *)( m_IDPtr.PP );

	return m_IDPtr;
}

CTable *CPipes::GetPipeSeries()
{ 
	if( NULL != GetIDPtr().PP )
	{
		return GetIDPtr().PP;
	}
	else
	{
		return NULL;
	}
}

IDPTR CPipes::GetDiversityIDPtr()
{
	m_DiversityIDPtr.DB = TASApp.GetpPipeDB();

	if( _T('\0') != *m_DiversityIDPtr.ID && NULL != m_DiversityIDPtr.DB )
	{
		Extend( &m_DiversityIDPtr );
		return m_DiversityIDPtr;
	}
	else
	{
		return _NULL_IDPTR;
	}
}

IDPTR CPipes::GetBestPipeIDPtr()
{ 
	m_IDPtrBestPipe.DB = TASApp.GetpPipeDB();

	if( _T('\0') != *m_IDPtrBestPipe.ID && NULL != m_IDPtrBestPipe.DB )
	{
		Extend( &m_IDPtrBestPipe );
		return m_IDPtrBestPipe;
	}
	else
	{
		return _NULL_IDPTR;
	}
}

double CPipes::GetPipeDp()
{
	GetLinDp();
	return ( m_dLinDp * GetLength() * m_pParent->GetpPrjParam()->GetSafetyFactor() );
}

double CPipes::GetLinDp()
{
	// Retrieve 'Linear pressure drop' and 'Velocity'.
	if( NULL != dynamic_cast<CDB_Pipe *>( (CData *)GetIDPtr().MP ) && GetRealQ() > 0.0 )
	{
		CDB_Pipe *pclPipe = (CDB_Pipe *)( (CData *)GetIDPtr().MP );

		if( CDS_HydroMod::ReturnType::Reverse == m_eReturnType && 
				( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == m_eLocated || CDS_HydroMod::eHMObj::eDistributionSupplyPipe == m_eLocated ) )
		{
			// If we are in a reverse return mode, we take the water characteristic of the corresponding pipe.
			// Here it's not an average.
			if( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == m_eLocated )
			{
				m_dLinDp = pclPipe->GetLinearDp( GetRealQ(), m_clSupplyWaterChar.GetDens(), m_clSupplyWaterChar.GetKinVisc() );
			}
			else
			{
				m_dLinDp = pclPipe->GetLinearDp( GetRealQ(), m_clReturnWaterChar.GetDens(), m_clReturnWaterChar.GetKinVisc() );
			}
		}
		else
		{
			// If we are in a direct return mode, we do an average between supply and return.

			// HYS-1716: For a future Jira card (HYS-1729) we will find a way to show details about the different linear dp.
			// We add the supply pipe.
			m_dLinDp = pclPipe->GetLinearDp( GetRealQ(), m_clSupplyWaterChar.GetDens(), m_clSupplyWaterChar.GetKinVisc() );
			
			// We add the return pipe.
			m_dLinDp += pclPipe->GetLinearDp( GetRealQ(), m_clReturnWaterChar.GetDens(), m_clReturnWaterChar.GetKinVisc() );
			m_dLinDp /= 2.0;
		}

		m_dVelocity = pclPipe->GetVelocity( GetRealQ() );
	}
	else
	{
		m_dLinDp = 0.0;
		m_dVelocity = 0.0;
	}

	return m_dLinDp;
}

double CPipes::GetVelocity()
{
	// Update local velocity variable with GetlinDp method.
	GetLinDp();
	return m_dVelocity;
}

double CPipes::GetVelocity( double dQ )
{
	double dVelocity = 0.0;
	
	// Retrieve 'Velocity'.
	if( NULL != dynamic_cast<CDB_Pipe *>( (CData *)GetIDPtr().MP ) && dQ > 0.0 )
	{
		CDB_Pipe *pclPipe = (CDB_Pipe *)( (CData *)GetIDPtr().MP );
		dVelocity = pclPipe->GetVelocity( dQ );
	}

	return dVelocity;
}

CWaterChar *CPipes::GetpWaterChar( CAnchorPt::PipeLocation ePipeLocation )
{
	CWaterChar *pclWaterChar = NULL;

	// HYS-1734: For 3-way dividing circuit use supply water char.
	if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
	{
		pclWaterChar = &m_clSupplyWaterChar;
	}
	else if( CAnchorPt::PipeLocation::PipeLocation_ByPass == ePipeLocation )
	{
		if( CAnchorPt::CircuitSide::CircuitSide_Primary == m_pParent->GetBypassPipeSide() )
		{
			pclWaterChar = &m_clSupplyWaterChar;
		}
		else if( CAnchorPt::CircuitSide::CircuitSide_Secondary == m_pParent->GetBypassPipeSide() )
		{
			pclWaterChar = &m_clReturnWaterChar;
		}
	}
	else
	{
		pclWaterChar = &m_clReturnWaterChar;
	}

	return pclWaterChar;
}

double CPipes::GetTemperature( CAnchorPt::PipeLocation ePipeLocation )
{
	double dTemperature = -273.15;

	// HYS-1734: For 3-way dividing circuit use supply temperature.
	if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
	{
		dTemperature = m_clSupplyWaterChar.GetTemp();
	}
	else if( CAnchorPt::PipeLocation::PipeLocation_ByPass == ePipeLocation )
	{
		if( CAnchorPt::CircuitSide::CircuitSide_Primary == m_pParent->GetBypassPipeSide() )
		{
			dTemperature = m_clSupplyWaterChar.GetTemp();
		}
		else if( CAnchorPt::CircuitSide::CircuitSide_Secondary == m_pParent->GetBypassPipeSide() )
		{
			dTemperature = m_clReturnWaterChar.GetTemp();
		}
	}
	else
	{
		dTemperature = m_clReturnWaterChar.GetTemp();
	}

	return dTemperature;
}

void CPipes::SetPipeSeries( CTable *pSerieTab, bool fSignal )
{
	if( GetIDPtr().PP == pSerieTab )
	{
		return;
	}

	if( pSerieTab == TASApp.GetpPipeDB()->GetPipeTab() )
	{
		pSerieTab = 0;
	}

	ASSERT( NULL != pSerieTab );

	if( NULL == pSerieTab )
	{
		return;
	}

	// Select a pipe in this series to fix IDPTR.
	IDPTR idptr = pSerieTab->GetFirst();
	
	// Set IDPtr without signaling.
	SetIDPtr( idptr, false );
	m_pParent->SetLock( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe, false, false );

	// Select BestPipe in this series.
	if( true == fSignal )
	{
		m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::ecePipeSerie, (LPARAM)this );
	}

	m_pParent->Modified();
}

void CPipes::SetIDPtr( IDPTR idptr, bool bSignal )
{
	if( _tcscmp( m_IDPtr.ID, idptr.ID) != 0 )
	{
		m_IDPtr = idptr;

		if( _T('\0') != *idptr.ID )
		{
			if( _T('\0') != *m_IDPtr.ID && NULL != m_IDPtr.DB )
			{
				Extend( &m_IDPtr );
			}
		}

		m_pPipeSerie = (CTable *)( m_IDPtr.PP );
		m_dLinDp = 0.0;						// Force LinDp actualization in function GetPipeDp.
		m_pParent->Modified();
	}

	if( true == bSignal )
	{
		m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::ecePipeSize, (LPARAM)this );
	}
}

void CPipes::SetDiversityAppliedFlag( bool bIsDiversityApplied )
{
	// First check if pipe is well a distribution. Diversity factor can be applied only on this kind of pipe.
	if( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == m_eLocated || CDS_HydroMod::eHMObj::eDistributionReturnPipe == m_eLocated )
	{
		m_bIsDiversityApplied = bIsDiversityApplied;
	}
}

void CPipes::SetLength( double dLength, bool bSignal )
{
	if( m_dLength != dLength )
	{
		m_dLength = dLength;

		if( true == bSignal )
		{
			m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::ecePipeLength, (LPARAM)this );
		}

		m_pParent->Modified();
	}
}

IDPTR CPipes::SelectBestPipe( double dQ )
{
	if( true == m_pParent->IsaModule() && ( CDS_HydroMod::eHMObj::eDistributionSupplyPipe != m_eLocated ) 
			&& ( CDS_HydroMod::eHMObj::eDistributionReturnPipe != m_eLocated ) )
	{
		// If circuit pipe is on the primary side and there is no secondary side (no 2way/3way-injection), we can reset because in this case there is
		// no circuit pipe for a module.
		bool bResetPipeSeries = ( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe == m_eLocated && NULL != m_pParent->GetpSchcat() && false == m_pParent->GetpSchcat()->IsSecondarySideExist() );

		// If circuit pipe is on the secondary side, we reset because for module there is never circuit pipe on the secondary side.
		bResetPipeSeries |= ( CDS_HydroMod::eHMObj::eCircuitSecondaryPipe == m_eLocated );

		// If bypass pipe is on the primary side we can reset because in this case there is no bypass pipe for a module.
		bResetPipeSeries |= ( CDS_HydroMod::eHMObj::eCircuitBypassPrimaryPipe == m_eLocated && CAnchorPt::CircuitSide::CircuitSide_Primary == m_pParent->GetBypassPipeSide() );

		if( false == IsLocked() && true == bResetPipeSeries )
		{
			// In case of a module we don't need to fix pipe series for un-existing circuit pipe.
			// Circuit pipe doesn't exist in a module length = 0! but it's used to size accessories.
			// So we we need to keep the largest possible choice of pipe we force standard pipe series that allow us a
			// selection up to DN 1200.
			CTable *pPipeSerieTab = (CTable *)( TASApp.GetpPipeDB()->Get( _T("STEEL_GEN") ).MP );
			SetPipeSeries( pPipeSerieTab );
		}
	}

	// If series doesn't exist yet...
	if( NULL == GetPipeSeries() )
	{
		// Pipe series doesn't exist yet, user read a file containing module without flow.
		// Select default circuit pipe series if q flowing is greater than 0.
		CString strPipeID;

		if( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == m_eLocated )
		{
			strPipeID = m_pParent->GetpPrjParam()->GetpHmCalcParams()->GetPrjParamID( CPrjParams::PipeDistSupplySerieID );
		}
		else if( CDS_HydroMod::eHMObj::eDistributionReturnPipe == m_eLocated )
		{
			strPipeID = m_pParent->GetpPrjParam()->GetpHmCalcParams()->GetPrjParamID( CPrjParams::PipeDistReturnSerieID );
		}
		else if( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe == m_eLocated || CDS_HydroMod::eHMObj::eCircuitSecondaryPipe == m_eLocated )
		{
			strPipeID = m_pParent->GetpPrjParam()->GetpHmCalcParams()->GetPrjParamID( CPrjParams::PipeCircSerieID );
		}
		else if( CDS_HydroMod::eHMObj::eCircuitBypassPrimaryPipe == m_eLocated )
		{
			strPipeID = m_pParent->GetpPrjParam()->GetpHmCalcParams()->GetPrjParamID( CPrjParams::PipeByPassSerieID );
		}

		if( true == strPipeID.IsEmpty() )
		{
			strPipeID = m_pParent->GetpTechParam()->GetDefaultPipeSerieIDPtr().ID;
		}

		if( false == strPipeID.IsEmpty() )
		{
			CTable *pTab = TASApp.GetpPipeDB()->GetPipeTab();
			CTable *pPipeSerieTab = (CTable *)( pTab->Get( (LPCTSTR)strPipeID ).MP );

			if( NULL != pPipeSerieTab )
			{
				// Not selectable, use default pipe series.
				if( false == pPipeSerieTab->IsSelectable( true ) )
				{
					pPipeSerieTab = (CTable *)( m_pParent->GetpTechParam()->GetDefaultPipeSerieIDPtr().MP );
				}

				if( NULL != pPipeSerieTab )
				{
					SetPipeSeries( pPipeSerieTab );
				}
			}
		}
	}

	// If series exist take it...
	// GetPipeSerie force updating of 'm_pPipeSerie', it's needed because when the user call
	// 'CDialogCustomPipe', pipes memory pointer moves.
	if( NULL != GetPipeSeries() )
	{
		if( dQ <= 0.0 )
		{
			return _NULL_IDPTR;
		}
		
		m_dLinDp = 0.0;
		
		// HYS-1716: For the moment we have not difference between the supply and the return for the primary and secondary circuits pipes
		// and the distribution pipe in direct return mode. We thus don't know which water characteristic to use.
		CWaterChar *pclWaterChar = NULL;

		if( CDS_HydroMod::ReturnType::Direct == m_eReturnType )
		{
			// In direct return mode, we have only one pipe for supply and return primary and secondary circuit pipes and the distribution
			// pipe. In this case, we take the water characteristic of the pipe that has the biggest density.
			pclWaterChar = ( m_clSupplyWaterChar.GetDens() >= m_clReturnWaterChar.GetDens() ) ? &m_clSupplyWaterChar : &m_clReturnWaterChar;
		}
		else
		{
			// In reverse mode we will take the water char corresponding to the current pipe.
			pclWaterChar = ( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == GetLocate() ) ? &m_clSupplyWaterChar : &m_clReturnWaterChar;
		}

		CProductSelelectionParameters clProductSelectionParameters( TASApp.GetpTADB(), TASApp.GetpTADS(), TASApp.GetpUserDB(),
				TASApp.GetpPipeDB(), pclWaterChar, m_pPipeSerie->GetIDPtr().ID );

		CSelectPipeList clSelectPipeList;
		clSelectPipeList.SelectPipes( &clProductSelectionParameters, dQ, false );

		m_IDPtrBestPipe = _NULL_IDPTR;

		if( NULL != clSelectPipeList.GetBestPipe() )
		{
			m_IDPtrBestPipe = clSelectPipeList.GetBestPipe()->GetIDPtr();
		}
		
		// If pipe exist verify Lock flag...
		if( NULL != GetIDPtr().MP )
		{
			if( true == IsLocked() )
			{
				return GetIDPtr();
			}
		}

		return m_IDPtrBestPipe;
	}

	return _NULL_IDPTR;
}

void CPipes::UpdateAllPipesIDPtr()
{
	if( _T('\0') != m_IDPtr.ID[0] )
	{
		m_IDPtr = TASApp.GetpPipeDB()->Get( m_IDPtr.ID );
		ASSERT( NULL != m_IDPtr.MP );

		m_pPipeSerie = (CTable *)( m_IDPtr.PP );
	}

	if( _T('\0') != m_IDPtrBestPipe.ID[0] )
	{
		m_IDPtrBestPipe = TASApp.GetpPipeDB()->Get( m_IDPtrBestPipe.ID );
		ASSERT( NULL != m_IDPtrBestPipe.MP );
	}

	if( _T('\0') != m_DiversityIDPtr.ID[0] )
	{
		m_DiversityIDPtr = TASApp.GetpPipeDB()->Get( m_DiversityIDPtr.ID );
		ASSERT( NULL != m_DiversityIDPtr.MP );
	}

	for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
	{
		if( CDS_HydroMod::eDpType::Pipelength == iter->second->m_eDpType 
				&& _T('\0') != iter->second->m_IDPtr.ID[0] )
		{
			iter->second->m_IDPtr = TASApp.GetpPipeDB()->Get( iter->second->m_IDPtr.ID );
			ASSERT( NULL != iter->second->m_IDPtr.MP );
		}
	}
}

bool CPipes::IsPipeUsed( IDPTR IDPtrPipeSerie, IDPTR IDPtrPipeSize )
{
	ASSERT( NULL != IDPtrPipeSerie.MP );

	if( _NULL_IDPTR != IDPtrPipeSize )
	{
		if( IDPtrPipeSize.MP == GetIDPtr().MP )
		{
			return true;
		}
		
		// Verify if the pipe size is used in the current pipe singularities.
		for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
		{
			if( iter->second->m_eDpType != CDS_HydroMod::eDpType::Pipelength )
			{
				continue;
			}

			if( IDPtrPipeSize.MP != iter->second->GetSingulIDPtr().MP )
			{
				continue;
			}

			return true;
		}
	}
	else
	{
		// Verify if the pipe series is used in the current pipe.
		if( IDPtrPipeSerie.MP == GetIDPtr().PP )
		{
			return true;
		}

		// Verify if the pipe series is used in the current pipe singularities.
		for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
		{
			if( iter->second->m_eDpType != CDS_HydroMod::eDpType::Pipelength )
			{
				continue;
			}

			if( IDPtrPipeSerie.MP != iter->second->GetSingulIDPtr().PP )
			{
				continue;
			}

			return true;
		}
	}

	return false;
}

bool CPipes::IsCompletelyDefined()
{
	if( NULL != GetIDPtr().MP )				// safe IDPtr, checked by Extend in GetIDPtr()
	{
		return true;
	}

	return false;
}

bool CPipes::IsBestPipe( CData *pData )
{	
	return ( pData == m_IDPtrBestPipe.MP ) ? true : false;
}

bool CPipes::IsLocked()
{
	bool bIsLocked = false;

	switch( GetLocate() )
	{
		case CDS_HydroMod::eHMObj::eCircuitPrimaryPipe:
			bIsLocked = m_pParent->IsLocked( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe );
			break;

		case CDS_HydroMod::eHMObj::eCircuitBypassPrimaryPipe:
			bIsLocked = m_pParent->IsLocked( CDS_HydroMod::eHMObj::eCircuitBypassPrimaryPipe );
			break;

		case CDS_HydroMod::eHMObj::eCircuitSecondaryPipe:
			bIsLocked = m_pParent->IsLocked( CDS_HydroMod::eHMObj::eCircuitSecondaryPipe );
			break;

		case CDS_HydroMod::eHMObj::eDistributionSupplyPipe:
			bIsLocked = m_pParent->IsLocked( CDS_HydroMod::eHMObj::eDistributionSupplyPipe );
			break;

		case CDS_HydroMod::eHMObj::eDistributionReturnPipe:
			bIsLocked = m_pParent->IsLocked( CDS_HydroMod::eHMObj::eDistributionReturnPipe );
			break;

		default:
			ASSERT( 0 );
			break;
	}

	return bIsLocked;
}

void CPipes::SetLock( bool bFlag, bool bResizeNow /*= true*/ )
{
	switch( GetLocate() )
	{
		case CDS_HydroMod::eHMObj::eCircuitPrimaryPipe:
			m_pParent->SetLock( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe, bFlag, bResizeNow );
			break;

		case CDS_HydroMod::eHMObj::eCircuitBypassPrimaryPipe:
			m_pParent->SetLock( CDS_HydroMod::eHMObj::eCircuitBypassPrimaryPipe, bFlag, bResizeNow );
			break;

		case CDS_HydroMod::eHMObj::eCircuitSecondaryPipe:
			m_pParent->SetLock( CDS_HydroMod::eHMObj::eCircuitSecondaryPipe, bFlag, bResizeNow );
			break;

		case CDS_HydroMod::eHMObj::eDistributionSupplyPipe:
			m_pParent->SetLock( CDS_HydroMod::eHMObj::eDistributionSupplyPipe, bFlag, bResizeNow );
			break;

		case CDS_HydroMod::eHMObj::eDistributionReturnPipe:
			m_pParent->SetLock( CDS_HydroMod::eHMObj::eDistributionReturnPipe, bFlag, bResizeNow );
			break;

		default:
			ASSERT( 0 );
			break;
	}
}

double CPipes::GetConnectDp( CDS_HydroMod *pNextHM )
{
	double dDp = 0.0;

	if( GetRealQ() <= 0.0 )
	{
		return dDp;
	}

	CSingularity Singularity;

	// Connection singularities are always stored with 'SINGULARITY_CONNECT_ID'.
	if( false == GetSingularity( SINGULARITY_CONNECT_ID, &Singularity ) )
	{
		return dDp;
	}
	
	CDB_Singularity *pSingularity = (CDB_Singularity *)( Singularity.GetSingulIDPtr().MP );

	if( NULL == pSingularity )
	{
		ASSERTA_RETURN( dDp );
	}

	if( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe == GetLocate() || CDS_HydroMod::eHMObj::eCircuitSecondaryPipe == GetLocate() )
	{
		// Dp for circuit pipe, singularity must be Elbow or Tee.
		double dVa = GetVelocity();
		double dVb = m_pParent->GetpDistrSupplyPipe()->GetVelocity();

		if( 0.0 == dVb )
		{
			return 0.0;
		}
		
		double dRatio = dVa / dVb;
		
		// No ratio needed when singularity is constant.
		if( CDB_Singularity::eXType::Constant == pSingularity->GetXType() )
		{
			dRatio = 1;
		}
		
		// HYS-1716: We compute here the Dp for the flow that is going from the distribution tee (Or elbow) to the circuit.
		// It concerns the fluid that are in the supply circuit pipe at the primary side (That is normally the same as the
		// supply distribution pipe). We need thus the water characteristic of the supply circuit pipe at the primary side (m_clSupplyWaterChar).
		dDp = CalcDpFromDzeta( pSingularity->GetDzeta( dRatio ), dVa, m_clSupplyWaterChar.GetDens() );

		if( CDS_HydroMod::ReturnType::Reverse == m_eReturnType && CDS_HydroMod::eHMObj::eCircuitPrimaryPipe == GetLocate() )
		{
			// In REVERSE return mode, connection singularities are not the same for input and output:
			//
			//   --T-----T-----E
			//     |     |     |
			//     E-----T-----T--
			//
			//   i = 1     : Tee for input and Elbow for output.
			//   1 < i < n : Tee for input and output.
			//   i = n     : Elbow for input and Tee for output.
			//
			// Now, we have 2 'CPipes' objects for distribution supply and return pipe but NOT for circuit pipe.
			// How to know what to use for connection in circuit pipe?
			// For distribution pipes we have:
			//
			//   i = 1     : dp = TEE;  dr = NONE
			//   1 < i < n : dp = TEE;  dr = TEE
			//   i = n     : dp = NONE; dr = TEE
			//
			// We can thus deduce from these values:
			//
			//   dpS  | dpR -> cpIn | cpOut
			//   -----+------+------+------
			//   Tee  | None |  Tee | Elbow
			//   Tee  | Tee  |  Tee | Tee
			//   None | Tee  |  Tee | Tee

			CPipes *pDistrReturnPipe = m_pParent->GetpDistrReturnPipe();
			ASSERT( NULL != pDistrReturnPipe );

			// It's normally impossible to have this case because each circuit MUST have a distribution pipe.
			if( NULL == pDistrReturnPipe )
			{
				return ( 2 * dDp );
			}

			// Dp for circuit pipe, singularity must be Elbow or Tee.
			double dVa = GetVelocity();
			double dVbp = pDistrReturnPipe->GetVelocity();

			if( 0.0 == dVbp )
			{
				return ( 2 * dDp );
			}

			// Connection singularity are always stored with SINGULARITY_CONNECT_ID.
			CSingularity DistrReturnSingularity;

			if( false == pDistrReturnPipe->GetSingularity( SINGULARITY_CONNECT_ID, &DistrReturnSingularity ) )
			{
				ASSERTA_RETURN( 2 * dDp );
			}

			CDB_Singularity *pDistrReturnSingularity = (CDB_Singularity *)( DistrReturnSingularity.GetSingulIDPtr().MP );
			ASSERT( NULL != pDistrReturnSingularity );

			if( NULL == pDistrReturnSingularity )
			{
				return ( 2 * dDp );
			}

			CTable *pSingularityTable = (CTable*)( TASApp.GetpTADB()->Get( _T("SINGULAR_TAB") ).MP );
			ASSERT( NULL != pSingularityTable );

			if( NULL == pSingularityTable )
			{
				return ( 2 * dDp );
			}

			CDB_Singularity *pCircuitOutletSingularity = NULL;

			if( 0 == _tcscmp( pDistrReturnSingularity->GetIDPtr().ID, _T("SINGUL_NONE") ) )
			{
				pCircuitOutletSingularity = (CDB_Singularity *)( pSingularityTable->Get( _T("ELBOW_90") ).MP );
			}
			else if( 0 == _tcscmp( pDistrReturnSingularity->GetIDPtr().ID, _T("TEE_DZB") ) )
			{
				pCircuitOutletSingularity = (CDB_Singularity *)( pSingularityTable->Get( _T("TEE_DZA") ).MP );
			}
			else
			{
				// ???
			}

			if ( NULL != pCircuitOutletSingularity)
			{
				double dRatio = dVa / dVbp;

				// HYS-1716: Same remark as above: we compute here the Dp for the flow that is going from the circuit tee (Or elbow)
				// to the return distribution pipe.
				// It concerns the fluid that are in the return circuit pipe at the primary side.
				// We need thus the water characteristic of the return circuit pipe at the primary side (m_clReturnWaterChar).
				dDp += CalcDpFromDzeta( pCircuitOutletSingularity->GetDzeta( dRatio ), dVa, m_clReturnWaterChar.GetDens() );
			}
		}
		else
		{
			// In DIRECT return mode, connection singularities are the same for input and output:
			//
			//   --T-----T-----
			//     |     |     |
			//   --T-----T-----

			// HYS-1716. But now we have a difference for the temperature that are in the return circuit pipe at the primary side.
			// So we need also to compute the Dp for this case.
			// Remark: Because we are in direct reverse mode, there is only one pipe for both supply and return distribution pipe.
			// Here we take thus the velocity of the distribution supply pipe but we compute the pressure drop with the return temperature.
			double dVa = GetVelocity();
			double dVb = m_pParent->GetpDistrSupplyPipe()->GetVelocity();

			double dRatio = dVa / dVb;
		
			// No ratio needed when singularity is constant.
			if( CDB_Singularity::eXType::Constant == pSingularity->GetXType() )
			{
				dRatio = 1;
			}

			dDp += CalcDpFromDzeta( pSingularity->GetDzeta( dRatio ), dVa, m_clReturnWaterChar.GetDens() );
		}
	}
	else if( CDS_HydroMod::eHMObj::eCircuitBypassPrimaryPipe == GetLocate() && NULL != m_pParent->GetpSch() 
			&& eDpStab::DpStabOnBVBypass ==  m_pParent->GetpSch()->GetDpStab() )
	{
		// For the moment we accept this case only for the auto-adapting variable flow decoupling circuit.

		// Dp for circuit pipe, singularity must be Elbow or Tee.
		double dVa = GetVelocity();
		double dVb = m_pParent->GetpCircuitPrimaryPipe()->GetVelocity();

		if( 0.0 == dVb )
		{
			return 0.0;
		}
		
		double dRatio = dVa / dVb;
		
		// No ratio needed when singularity is constant.
		if( CDB_Singularity::eXType::Constant == pSingularity->GetXType() )
		{
			dRatio = 1;
		}
		
		// The temperature in the bypass is the same as the circuit supply pipe at the primary side.
		dDp = CalcDpFromDzeta( pSingularity->GetDzeta( dRatio ), dVa, m_clSupplyWaterChar.GetDens() );

	}
	else 
	{
		// REMARK: before we had only on 'CPipes' object for distribution pipe in a 'CDS_HydroMod' object. Since we have the possibility to work
		//         in direct or reverse return mode, we have 1 'CPipe' object for distribution pipe in the direct return mode. If we
		//         are in a reverse return mode, we have 2 'CPipe' objects: 1 for distribution supply pipe and 1 for distribution return pipe.
		//         'Dp' must not be multiplied by 2 for this last case.
		if( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == GetLocate() || CDS_HydroMod::eHMObj::eDistributionReturnPipe == GetLocate() )
		{
			// for a Tee, compute Dp at the outside of Tee, it will be used for next circuit.
			if( 0 == _tcscmp( _T("TEE_DZB"), Singularity.GetSingulIDPtr().ID ) )
			{
				double dVb = GetVelocity();
				
				// Compute flow after t division to the circuit.
				// Remark: - 'GetRealQ' sends the Q as it has been computed taking in count the fact that module can have one diversity factor applied
				//           or one (or more) pending circuits.
				//         - Before we subtracted 'm_pParent->GetQ()' to 'GetRealQ()'. Why is it no more correct? 'm_pParent' is the CDS_Hydromod to which
				//           distribution pipe belongs to. 'CDS_Hydromod::GetQ()' sends back ONLY the Q design as it has been input by the user.
				//           If parent is a pending circuit, flow at the input of the TEE is the same at the output because there is no flow in the circuit.
				//           It's the reason why here, we need to take real Q in the circuit pipe.
				// double dQMax = GetRealQ() - m_pParent->GetQ();
				double dQ;

				if( NULL != m_pParent->GetpCircuitPrimaryPipe() )
				{
					dQ = GetRealQ() - m_pParent->GetpCircuitPrimaryPipe()->GetRealQ();
				}
				else
				{
					// If we not have distribution, there is no flow!!
					dQ = GetRealQ();
				}

				if( dQ <= 0 )
				{
					return 0.0;
				}
				
				if( _T('\0') == *GetIDPtr().ID )
				{
					return 0.0;
				}

				// Compute speed for tee itself.
				double dVbp = GetVelocity( dQ );
				double dRatio = dVbp / dVb;

				// HYS-1716: We compute here the Dp for the flow that is going from the distribution tee to the next distribution pipe.
				if( CDS_HydroMod::ReturnType::Direct == m_eReturnType )
				{
					// If we are in the direct return mode, we have only on pipe for the supply and the return.
					// So, the dp accross the tee will be for the both supply and return.
					dDp = CalcDpFromDzeta( pSingularity->GetDzeta( dRatio ), dVbp, m_clSupplyWaterChar.GetDens() );
					dDp += CalcDpFromDzeta( pSingularity->GetDzeta( dRatio ), dVbp, m_clReturnWaterChar.GetDens() );
				}
				else
				{
					// If we are in the reverse return mode, we have one pipe for supply and one for return.
					// Here we must send only dp accross 1 Tee.
					if( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == GetLocate() )
					{
						dDp = CalcDpFromDzeta( pSingularity->GetDzeta( dRatio ), dVbp, m_clSupplyWaterChar.GetDens() );
					}
					else
					{
						dDp = CalcDpFromDzeta( pSingularity->GetDzeta( dRatio ), dVbp, m_clReturnWaterChar.GetDens() );
					}
				}
				
				// We must now compute the restriction (Or expansion) in regards to the next distribution pipe.
				double dDpRestriction = 0.0;

				if( NULL == pNextHM )
				{
					CTable *pTab = (CTable *)m_pParent->GetIDPtr().PP;

					if( NULL != pTab )
					{
						// We need NEXT circuit to compute a Tee for a distribution supply.
						if( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == GetLocate() )
						{
							int iPos = m_pParent->GetPos() + 1;
							CTable *pTab = (CTable *)m_pParent->GetIDPtr().PP;
							
							if( NULL != pTab )
							{
								for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
								{
									CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( idptr.MP );
									
									if( NULL != pHM )
									{
										if( pHM->GetPos() == iPos )
										{
											pNextHM = pHM;
											break;
										}
									}
								}
							}
						}
						else
						{
							// We need PREVIOUS circuit to compute a Tee for a distribution return.
							int iPos = m_pParent->GetPos() - 1;
							
							if( iPos >= 0 )
							{
								CTable *pTab = (CTable *)m_pParent->GetIDPtr().PP;
								
								if( NULL != pTab )
								{
									for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
									{
									
										CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( idptr.MP );
										
										if( NULL != pHM )
										{
											if( pHM->GetPos() == iPos )
											{
												pNextHM = pHM;
												break;
											}
										}
									}
								}
							}
						}
					}
				}
				
				if( NULL != pNextHM && NULL != GetIDPtr().MP )
				{
					CPipes *pNextPipe = NULL;

					// If current pipe is a distribution supply...
					if( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == m_eLocated )
					{
						pNextPipe = pNextHM->GetpDistrSupplyPipe();
					}
					else if( CDS_HydroMod::eHMObj::eDistributionReturnPipe == m_eLocated )
					{
						pNextPipe = pNextHM->GetpDistrReturnPipe();
					}

					if( NULL != pNextPipe && NULL != pNextPipe->GetIDPtr().MP )
					{
						double Sb = ( (CDB_Pipe *)GetIDPtr().MP )->GetIntDiameter();
						Sb *= Sb * M_PI_4;
						double Sa = ( (CDB_Pipe *)( pNextPipe->GetIDPtr().MP ) )->GetIntDiameter();
						Sa *= Sa * M_PI_4;
						
						if( Sb != Sa )
						{
							// Pipe around Tee are different, restriction/expansion exists we have to compute two cases.

							// In the equation below we have to consider highest speed (smallest pipe).
							if( Sb > Sa )
							{
								dVbp = pNextPipe->GetVelocity();
							}
							else
							{
								dVbp = GetVelocity();
							}

							if( CDS_HydroMod::ReturnType::Direct == m_eReturnType )
							{
								// In direct return mode, we compute restriction for the supply and the return connection singularity.
								
								// Speed is higher with a smallest flow we have a restriction ! 
								// compute surface ratio
								// ______
								//       |_____		Sa/Sb	Za = 0.34 *(1-Sa/Sb)
								//   Sb		Sa		RP book p45	
								//  --->  _____		Idel'Cik p82/83 (use speed vbp)
								// ______|			
								//					
								double dDzeta = 0.34 * ( 1 - Sa / Sb );

								// HYS-1716: in direct return mode, we are with the supply distribution pipe (m_clSupplyWaterChar).
								dDpRestriction = CalcDpFromDzeta( dDzeta, dVbp, m_clSupplyWaterChar.GetDens() );

								// Speed is higher with a smallest flow we have a restriction ! 
								// compute surface ratio
								// ______
								//       |_____		Sa/Sb	Za = (1-Sa/Sb)�
								//   Sb		Sa		RP book p45	
								//   <--- _____		Idel'Cik p82/83 (use speed vbp)
								// ______|	
								//					
								dDzeta = 1 - Sa / Sb;
								dDzeta *= dDzeta;

								// HYS-1716: the water charateristic concerned is well the return distribution pipe. 
								dDpRestriction += CalcDpFromDzeta( dDzeta, dVbp, m_clReturnWaterChar.GetDens() );
							}
							else
							{
								// In reverse return mode, we compute only connection in regards to the current location of the pipe.
								
								if( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == m_eLocated )
								{
									// Speed is higher with a smallest flow we have a restriction ! 
									// compute surface ratio
									// ______
									//       |_____		Sa/Sb	Za = 0.34 *(1-Sa/Sb)
									//   Sb		Sa		RP book p45	
									//  --->  _____		Idel'Cik p82/83 (use speed vbp)
									// ______|			
									//					
									double dDzeta = 0.34 * ( 1 - Sa / Sb );
									dDpRestriction = CalcDpFromDzeta( dDzeta, dVbp, m_clSupplyWaterChar.GetDens() );
								}
								else if( CDS_HydroMod::eHMObj::eDistributionReturnPipe == m_eLocated )
								{
									// Speed is higher with a smallest flow we have a restriction ! 
									// compute surface ratio
									// ______
									//       |_____		Sa/Sb	Za = (1-Sa/Sb)�
									//   Sb		Sa		RP book p45	
									//   <--- _____		Idel'Cik p82/83 (use speed vbp)
									// ______|	
									//					
									double dDzeta = 1 - Sa / Sb;
									dDzeta *= dDzeta;
									dDpRestriction = CalcDpFromDzeta( dDzeta, dVbp, m_clReturnWaterChar.GetDens() );
								}
							}
						}
						
						dDp += dDpRestriction;
					}
				}
			}
		}
	}
	
	return dDp;
}

void CPipes::GetConnectDescription( CString &strDescription, bool bWithConnectionSuffix )
{
	// See comments in .h for more details about this method.

	CSingularity Singularity;
	strDescription = _T("-");

	if( false == GetSingularity( SINGULARITY_CONNECT_ID, &Singularity ) )
	{
		return;
	}
	
	if( CDS_HydroMod::ReturnType::Direct == m_pParent->GetReturnType() )
	{
		if( _tcscmp( Singularity.GetSingulIDPtr().ID, _T("SINGUL_NONE") ) != 0 )
		{
			CString strTemp = CString( _T("2 ") ) + Singularity.GetDescription(); 

			if( true == bWithConnectionSuffix )
			{
				FormatString( strDescription, IDS_HMPIPE_FORCONNECTION, strTemp );
			}
			else
			{
				strDescription = strTemp;
			}
		}
	}
	else
	{
		switch( m_eLocated )
		{
			case CDS_HydroMod::eHMObj::eCircuitPrimaryPipe:
				{
					CTable *pSingularityTable = (CTable*)( TASApp.GetpTADB()->Get( _T("SINGULAR_TAB") ).MP );

					if( NULL == pSingularityTable )
					{
						ASSERT_RETURN;
					}

					CPipes *pDistrReturnPipe = m_pParent->GetpDistrReturnPipe();

					if( NULL == pDistrReturnPipe )
					{
						ASSERT_RETURN;
					}

					CSingularity DistrReturnSingularity;
					
					if( false == pDistrReturnPipe->GetSingularity( SINGULARITY_CONNECT_ID, &DistrReturnSingularity ) )
					{
						return;
					}

					CDB_Singularity *pDistrReturnSingularity = (CDB_Singularity *)( DistrReturnSingularity.GetSingulIDPtr().MP );
					
					if(	NULL == pDistrReturnSingularity )
					{
						ASSERT_RETURN;
					}

					// To understand how we choose the correct connection singularity for circuit pipe outlet, see the comments in 
					// the 'GetConnectDp' method.
					CDB_Singularity* pSingularityOut = NULL;

					if( 0 == _tcscmp( pDistrReturnSingularity->GetIDPtr().ID, _T("SINGUL_NONE") ) )
					{
						pSingularityOut = (CDB_Singularity*)( pSingularityTable->Get( _T("ELBOW_90") ).MP );
					}
					else if( 0 == _tcscmp( pDistrReturnSingularity->GetIDPtr().ID, _T("TEE_DZB") ) )
					{
						pSingularityOut = (CDB_Singularity*)( pSingularityTable->Get( _T("TEE_DZA") ).MP );
					}

					CString strTemp = Singularity.GetDescription() + _T("/") + pSingularityOut->GetDescription();

					if( true == bWithConnectionSuffix )
					{
						FormatString( strDescription, IDS_HMPIPE_FORCONNECTION, strTemp );
					}
					else
					{
						strDescription = strTemp;
					}
				}
				break;

			case CDS_HydroMod::eHMObj::eCircuitSecondaryPipe:
			case CDS_HydroMod::eHMObj::eDistributionSupplyPipe:
			case CDS_HydroMod::eHMObj::eDistributionReturnPipe:
				strDescription = Singularity.GetDescription();
				break;
		}
	}
}

eBool3 CPipes::CheckLinDpMinMax()
{
	if( GetLinDp() < m_pParent->GetpTechParam()->GetPipeMinDp()
			|| GetLinDp() > m_pParent->GetpTechParam()->GetPipeMaxDp() )
	{
		return eb3False;
	}

	return eb3True;
}

eBool3 CPipes::CheckVelocityMinMax()
{
	if( GetVelocity() < m_pParent->GetpTechParam()->GetPipeMinVel()
			|| GetVelocity() > m_pParent->GetpTechParam()->GetPipeMaxVel() )
	{
		return eb3False;
	}

	return eb3True;
}

void CPipes::CheckPipeConnection( CString strID )
{
	CSingularity Singul;

	if( true == GetSingularity( SINGULARITY_CONNECT_ID, &Singul ) )
	{
		// Singul is OK.
		if( strID == CString( Singul.GetSingulIDPtr().ID) )
		{
			return;
		}

		RemoveSingularity( SINGULARITY_CONNECT_ID );
	}
	
	_AddSingularity( SINGULARITY_CONNECT_ID, strID );
}

short CPipes::AddAccessorySingularity( CString strDescription, CDS_HydroMod::eDpType eSingulDpType, double dValue1, double dValue2 )
{
	short nID = _GetFirstFreeID();

	if( SINGULARITY_NOMOREAVAILABLE == nID )
	{
		return -1;
	}

	CSingularity *pSingularity = new CSingularity;

	if( NULL == pSingularity )
	{
		ASSERTA_RETURN( -1 );
	}
	
	pSingularity->m_nID = nID;
	pSingularity->m_strDescription = strDescription;
	pSingularity->m_IDPtr = _NULL_IDPTR;
	pSingularity->m_eDpType = eSingulDpType;
	
	switch( eSingulDpType )
	{
		case CDS_HydroMod::eDpType::Dp:
			pSingularity->m_uDpVal.Dp = dValue1;
			break;
		
		case CDS_HydroMod::eDpType::Kv:
			pSingularity->m_uDpVal.Kv = dValue1;
			break;
		
		case CDS_HydroMod::eDpType::Cv:
			pSingularity->m_uDpVal.Cv = dValue1;	
			break;
		
		case CDS_HydroMod::eDpType::dzeta:
			pSingularity->m_uDpVal.sDzeta.dzeta = dValue1;
			pSingularity->m_uDpVal.sDzeta.d = dValue2;
			break;
		
		case CDS_HydroMod::eDpType::QDpref:
			pSingularity->m_uDpVal.sQDpRef.Dpref = dValue1;
			pSingularity->m_uDpVal.sQDpRef.Qref = dValue2;
			break;
	}

	m_mapSingularityList[nID] = pSingularity;
	m_pParent->Modified();

	return pSingularity->m_nID;
}

void CPipes::AddAccessorySingularities( std::vector<CSingularity *> *pvecSingularities )
{
	if( NULL == pvecSingularities || 0 == (int)pvecSingularities->size() )
	{
		return;
	}

	for( std::vector<CSingularity *>::iterator iter = pvecSingularities->begin(); iter != pvecSingularities->end(); ++iter )
	{
		CSingularity *pSingularity = new CSingularity;

		if( NULL == pSingularity )
		{
			ASSERT_CONTINUE;
		}
		
		short nID = _GetFirstFreeID();

		if( SINGULARITY_NOMOREAVAILABLE == nID )
		{
			break;
		}

		pSingularity->m_nID = nID;
		pSingularity->m_IDPtr = (*iter)->m_IDPtr;
		pSingularity->m_strDescription = (*iter)->m_strDescription;
		pSingularity->m_eDpType = (*iter)->m_eDpType;
		pSingularity->m_uDpVal = (*iter)->m_uDpVal;
		m_mapSingularityList[nID] = pSingularity;
	}
}

short CPipes::AddPipeSingularity( CString strDescription, IDPTR PipeIDPtr, double dLength )
{
	// Find first free place.
	// Remark: search from 'SINGULARITY_FIRSTACCESSORY_ID' because 'SINGULARITY_CONNECT_ID' is reserved for the connection singularity.
	short nID = _GetFirstFreeID();

	if( SINGULARITY_NOMOREAVAILABLE == nID )
	{
		return -1;
	}

	CSingularity *pSingularity = new CSingularity;

	if( NULL == pSingularity )
	{
		ASSERTA_RETURN( -1 );
	}
	
	pSingularity->m_nID = nID;
	pSingularity->m_IDPtr = PipeIDPtr;
	pSingularity->m_strDescription = strDescription;
	pSingularity->m_eDpType = CDS_HydroMod::eDpType::Pipelength;
	pSingularity->m_uDpVal.Length = dLength;
	m_mapSingularityList[nID] = pSingularity;
	m_pParent->Modified();

	return pSingularity->m_nID;
}

short CPipes::AddConnectionSingularity( LPCTSTR SingulID )
{
	// Check if not already exist.
	if( m_mapSingularityList.count( SINGULARITY_CONNECT_ID ) != 0 )
	{
		return -1;
	}

	return _AddSingularity( SINGULARITY_CONNECT_ID, SingulID );
}

void CPipes::RemoveSingularity( short nID )
{
	if( 0 == (int)m_mapSingularityList.size() || 0 == m_mapSingularityList.count( nID ) )
	{
		return;
	}

	if( NULL != m_mapSingularityList[nID] )
	{
		delete m_mapSingularityList[nID];
	}

	m_mapSingularityList.erase( nID );
}

void CPipes::RemoveAllSingularities( bool bExceptConnection )
{
	if( m_mapSingularityList.size() > 0 )
	{
		SingularityIter iter = m_mapSingularityList.begin();

		while( iter != m_mapSingularityList.end() )
		{
			if( NULL != iter->second && ( false == bExceptConnection || SINGULARITY_CONNECT_ID != iter->second->m_nID ) )
			{
				SingularityIter nextiter = iter;
				++nextiter;
				delete iter->second;
				m_mapSingularityList.erase( iter );
				iter = nextiter;
			}
			else
			{
				++iter;
			}
		}
	}
}

CString CPipes::GetSingularityTypeString( CSingularity *pSingularity )
{
	CString str = _T("");

	switch( pSingularity->m_eDpType )
	{
		case CDS_HydroMod::eDpType::Kv:
			str = TASApp.LoadLocalizedString( IDS_PIPES_ACCKV );
			break;

		case CDS_HydroMod::eDpType::Cv:
			str = TASApp.LoadLocalizedString( IDS_PIPES_ACCCV );
			break;

		case CDS_HydroMod::eDpType::dzeta:
			str = TASApp.LoadLocalizedString( IDS_PIPES_ACCZETA );
			break;

		case CDS_HydroMod::eDpType::QDpref:
			str = TASApp.LoadLocalizedString( IDS_PIPES_ACCQREFDPREF );
			break;

		case CDS_HydroMod::eDpType::Pipelength:
			str = TASApp.LoadLocalizedString( IDS_PIPES_ACCPIPELENGTH );
			break;
	}
	
	return str;
}

CString CPipes::GetSingularitiesDescriptionShort( bool bWithConnection, bool bWithConnectionSuffix )
{
	// First, test if we are on a root module.
	// Remark: no singularity (connection or/and accessory) on a root module.
	if( true == m_pParent->IsaModule() && 0 == m_pParent->GetLevel() )
	{
		return _T("");
	}
	
	// If there is no singularity...
	CString strResult( _T("-") );

	if( 0 == (int)m_mapSingularityList.size() )
	{
		return strResult;
	}

	// Verify if a connection exists.
	bool bConnectionExist = false;
	bool bConnectionSingulNone = true;

	if( m_mapSingularityList.count( SINGULARITY_CONNECT_ID ) > 0 )
	{
		bConnectionExist = true;

		if( _tcscmp( m_mapSingularityList[SINGULARITY_CONNECT_ID]->GetSingulIDPtr().ID, _T("SINGUL_NONE") ) != 0 )
		{
			bConnectionSingulNone = false;
		}
	}

	int iSingularityCount = m_mapSingularityList.size();
	int iSingularityToExport = iSingularityCount;

	if( true == bWithConnection )
	{
		if( true == bConnectionExist && true == bConnectionSingulNone )
		{
			iSingularityToExport--;
		}
	}
	else
	{
		if( true == bConnectionExist )
		{
			iSingularityToExport--;
		}
	}

	// If there is some singularity to export...
	if( iSingularityToExport > 0 )
	{
		// Special case: if pipe is a circuit and is part of a module (not a root, level > 0) and not in injection,
		//               there is only the connection singularity possible.
		if( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe == m_eLocated && true == m_pParent->IsaModule() 
				&& NULL != m_pParent->GetpSchcat() && false == m_pParent->GetpSchcat()->IsSecondarySideExist() )
		{
			// If there is a connection and defined...
			if( true == bWithConnection && true == bConnectionExist && false == bConnectionSingulNone )
			{
				GetConnectDescription( strResult, bWithConnectionSuffix );
			}
		}
		else
		{
			// If only one we can show its name.
			if( 1 == iSingularityToExport )
			{
				// If we must export connection and the one on the pipe is available and defined...
				if( true == bWithConnection && true == bConnectionExist && false == bConnectionSingulNone )
				{
					GetConnectDescription( strResult, bWithConnectionSuffix );
				}
				else
				{
					// Take the accessory singularity.
					for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
					{
						if( iter->second->m_nID >= SINGULARITY_FIRSTACCESSORY_ID )
						{
							strResult = iter->second->GetDescription();
							
							// If user has not input description...
							if( true == strResult.IsEmpty() )
							{
								strResult = GetSingularityTypeString( iter->second );
							}
							break;
						}
					}
				}
			}
			else
			{
				// If more than one singularity, we show only the number.
			
				// Do a correction about connection singularity. We have only one entry in the map (with ID = 0) for
				// 2 connections for the direct return mode. In reverse return mode, each distribution pipe (supply and return)
				// has its own connection.
				if( true == bWithConnection && true == bConnectionExist && false == bConnectionSingulNone
						&& !( ( CDS_HydroMod::eHMObj::eDistributionReturnPipe == m_eLocated || CDS_HydroMod::eHMObj::eDistributionSupplyPipe == m_eLocated ) 
							&& CDS_HydroMod::ReturnType::Reverse == m_pParent->GetReturnType() ) )
				{
					iSingularityToExport++;
				}

				CString str;
				str.Format( _T("%d"), iSingularityToExport );
				FormatString( strResult, IDS_HMPIPE_SINGULNUMBER, (LPCTSTR)str );
			}
		}
	}
	return strResult;
}

CString CPipes::GetSingularitiesInputDataShort()
{
	// First, test if we are on a root module.
	// Remark: no singularity (connection or/and accessory) on a root module.
	CString strResult( _T("") );

	if( true == m_pParent->IsaModule() && 0 == m_pParent->GetLevel() )
	{
		return strResult;
	}

	// If there is no singularity...
	if( 0 == (int)m_mapSingularityList.size() )
	{
		return strResult;
	}
	
	// If a connection exists, we can't export input data for accessory singularity.
	bool bConnectionExist = false;
	bool bConnectionSingulNone = true;

	if( m_mapSingularityList.count( SINGULARITY_CONNECT_ID ) > 0 )
	{
		bConnectionExist = true;

		if( _tcscmp( m_mapSingularityList[SINGULARITY_CONNECT_ID]->GetSingulIDPtr().ID, _T("SINGUL_NONE") ) != 0 )
		{
			bConnectionSingulNone = false;
		}
	}

	if( true == bConnectionExist && false == bConnectionSingulNone )
	{
		return strResult;
	}

	int iAccessoryCount = m_mapSingularityList.size();

	if( true == bConnectionSingulNone )
	{
		iAccessoryCount--;
	}

	// If there one singularity to export...
	if( 1 == iAccessoryCount )
	{
		// Take the accessory singularity.
		for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
		{
			if( iter->second->m_nID >= SINGULARITY_FIRSTACCESSORY_ID )
			{
				strResult = _FormatSingularityStringHelper( iter->second, false, true, false, false );
				break;
			}
		}
	}
	
	return strResult;
}

CString CPipes::GetSingularityListFull( bool bDescription, bool bInputData, bool bDp, bool bWithConnection, int iLimitOutput )
{
	// First, test if we are on a root module.
	// Remark: no singularity (connection or/and accessory) on a root module.
	if( true == m_pParent->IsaModule() && 0 == m_pParent->GetLevel() )
	{
		return _T("");
	}

	// If there is no singularity...
	CString strResult( TASApp.LoadLocalizedString( IDS_HMPIPE_NOSINGULARITY ) );

	if( 0 == (int)m_mapSingularityList.size() )
	{
		return strResult;
	}
	
	// Verify if a connection exists.
	bool bConnectionExist = false;
	bool bConnectionSingulNone = true;

	if( m_mapSingularityList.count( SINGULARITY_CONNECT_ID ) > 0 )
	{
		bConnectionExist = true;

		if( _tcscmp( m_mapSingularityList[SINGULARITY_CONNECT_ID]->GetSingulIDPtr().ID, _T("SINGUL_NONE") ) != 0 )
		{
			bConnectionSingulNone = false;
		}
	}

	int iSingularityCount = m_mapSingularityList.size();
	int iSingularityToExport = iSingularityCount;

	if( true == bWithConnection )
	{
		if( true == bConnectionExist && true == bConnectionSingulNone )
		{
			iSingularityToExport--;
		}
	}
	else
	{
		if( true == bConnectionExist )
		{
			iSingularityToExport--;
		}
	}

	// If there are some singularities to export...
	if( iSingularityToExport > 0 )
	{
		if( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe == m_eLocated && true == m_pParent->IsaModule() 
				&& NULL != m_pParent->GetpSchcat() && false == m_pParent->GetpSchcat()->IsSecondarySideExist() )
		{
			// Special case: if pipe is a circuit and is part of a module (not a root, level > 0), and not in injection,
			// there is only the connection singularity possible.

			// If there is a connection and defined...
			if( true == bWithConnection && true == bConnectionExist && false == bConnectionSingulNone )
			{
				strResult = _FormatSingularityStringHelper( m_mapSingularityList[SINGULARITY_CONNECT_ID], bDescription, bInputData, bDp, false );
			}
		}
		else
		{
			// If only one we can show its name.
			if( 1 == iSingularityToExport )
			{
				// If we must export connection and the one in pipe is available and defined...
				if( true == bWithConnection && true == bConnectionExist && false == bConnectionSingulNone )
				{
					strResult = _FormatSingularityStringHelper( m_mapSingularityList[SINGULARITY_CONNECT_ID], bDescription, bInputData, bDp, false );
				}
				else
				{
					// Take the accessory singularity.
					for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
					{
						if( iter->second->m_nID >= SINGULARITY_FIRSTACCESSORY_ID )
						{
							strResult = _FormatSingularityStringHelper( iter->second, bDescription, bInputData, bDp, false );
							break;
						}
					}
				}
			}
			else
			{
				// If more than one singularity, we show all in a list format starting each one with a bullet.
				strResult = _T("");
				int iExportedCount = 0;

				for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
				{
					bool bExport = false;

					if( SINGULARITY_CONNECT_ID == iter->second->m_nID && true == bWithConnection && false == bConnectionSingulNone )
					{
						bExport = true;
					}
					else if( iter->second->m_nID >= SINGULARITY_FIRSTACCESSORY_ID )
					{
						bExport = true;
					}

					if( true == bExport )
					{
						if( false == strResult.IsEmpty() )
						{
							strResult += CString( _T("\r\n") );
						}

						CString strSingularity = _FormatSingularityStringHelper( iter->second, bDescription, bInputData, bDp, true );
						strResult += strSingularity;
						iExportedCount++;
						
						if( iLimitOutput > 1 && iExportedCount == iLimitOutput )
						{
							strResult += CString( _T("\r\n...") );
							break;
						}
					}
				}

			}
		}
	}
	
	return strResult;
}

bool CPipes::GetSingularityDetails( CSingularity *pSingularity, CString *pstrDescription, CString *pstrInputData, CString *pstrDp )
{
	if( pstrDescription != NULL )
	{
		*pstrDescription = _T("");
	}

	if( pstrInputData != NULL )
	{
		*pstrInputData = _T("");
	}

	if( pstrDp != NULL )
	{
		*pstrDp = _T("");
	}

	if( NULL == pSingularity )
	{
		return false;
	}

	if( SINGULARITY_CONNECT_ID == pSingularity->m_nID && 0 == _tcscmp( pSingularity->GetSingulIDPtr().ID, _T("SINGUL_NONE") ) )
	{
		return false;
	}

	if( pstrDescription != NULL )
	{
		*pstrDescription = _GetSingularityDescription( pSingularity );
	}

	if( pstrInputData != NULL )
	{
		*pstrInputData = _GetSingularityInputData( pSingularity );
	}

	if( pstrDp != NULL )
	{
		*pstrDp = _GetSingularityDp( pSingularity );
	}
	
	return true;
}

bool CPipes::GetSingularity( short nID, CSingularity *pSingularity )
{
	if( 0 == (int)m_mapSingularityList.size() || 0 == m_mapSingularityList.count( nID ) || NULL == pSingularity )
	{
		return false;
	}

	pSingularity->m_nID = nID;
	pSingularity->m_IDPtr = m_mapSingularityList[nID]->m_IDPtr;
	pSingularity->m_strDescription = m_mapSingularityList[nID]->m_strDescription;
	pSingularity->m_eDpType = m_mapSingularityList[nID]->m_eDpType;
	pSingularity->m_uDpVal = m_mapSingularityList[nID]->m_uDpVal;
	
	return true;
}

int CPipes::GetSingularityCount()
{ 
	return (int)m_mapSingularityList.size();
}

double CPipes::GetSingularityDp( CSingularity *pSingularity, CDS_HydroMod *pNextHM )
{
	if( NULL == pSingularity )
	{
		return 0.0;
	}
	
	// 'm_nID' is set to '0' when it's the special singularity for connection (tee or elbow). In this case we directly give the hand
	// to 'GetConnectDp'.
	if( SINGULARITY_CONNECT_ID == pSingularity->m_nID )
	{
		return GetConnectDp( pNextHM );
	}

	double dQ = GetRealQ();

	if( 0.0 == dQ )
	{
		return 0.0;
	}

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	ASSERT( NULL != pUnitDB );

	double dDp = 0.0;

	// HYS-1716: For a future card (HYS-1728), it will be allowed to the user to choose supply or return pipe.
	double dRho = m_clSupplyWaterChar.GetDens();
	
	if( _T('\0') != *pSingularity->GetSingulIDPtr().ID && pSingularity->m_eDpType != CDS_HydroMod::eDpType::Pipelength )
	{
		//TODO CPipes::GetSingulDp() take in count of Singularities from TADB
		//CDB_Singularity *pSingul = dynamic_cast<CDB_Singularity*>(Singul.GetSingulIDPtr().MP);
	}
	else
	{
		CDS_HydroMod::eDpType SingulDpType = pSingularity->m_eDpType;
		double dVal1 = 0.0;
		double dVal2 = 0.0;
		dDp = 0.0;

		switch( SingulDpType )
		{
			case CDS_HydroMod::eDpType::Dp:
				dVal1 = pSingularity->m_uDpVal.Dp;	
				dDp = dVal1;
				break;
		
			case CDS_HydroMod::eDpType::Kv:
				dVal1 = pSingularity->m_uDpVal.Kv;
				dDp = CalcDp( dQ, dVal1, dRho );
				break;
	
			case CDS_HydroMod::eDpType::Cv:
				{
					dVal1 = pSingularity->m_uDpVal.Cv;
					if( 0.0 == dVal1 )
						return 0.0;
					// Convert Cv To Kv.
					UnitDesign_struct ud = pUnitDB->GetUnit( _C_KVCVCOEFF, 1 );
					dVal1 = GetConvOf( ud ) * ( dVal1 + GetOffsetOf( ud ) );
					dDp = CalcDp( dQ, dVal1, dRho );
				}
				break;

			case CDS_HydroMod::eDpType::dzeta:
				{
					dVal1 = pSingularity->m_uDpVal.sDzeta.dzeta;
					CDB_Pipe *ppipe = (CDB_Pipe *)GetIDPtr().MP;
					if( NULL != ppipe )
					{
						double dDi = ppipe->GetIntDiameter();
						double dKv = CalcKvFromDzeta( dVal1, dDi );
						double dQ = GetRealQ();
						dDp = CalcDp( dQ, dKv, dRho );
					}
				}
				break;

			case CDS_HydroMod::eDpType::Pipelength:
				{
					double dLinDp = 0.0;
					CDB_Pipe *pPipe = NULL;

					if( _NULL_IDPTR == pSingularity->m_IDPtr )
					{
						dLinDp = GetLinDp();
					}
					else
					{
						pPipe = dynamic_cast<CDB_Pipe*>( pSingularity->GetSingulIDPtr().MP );

						// HYS-1716: For a future card (HYS-1728), it will be allowed to the user to choose supply or return pipe.
						dLinDp = pPipe->GetLinearDp( GetRealQ(), m_clSupplyWaterChar.GetDens(), m_clSupplyWaterChar.GetKinVisc() );
					}

					dDp = dLinDp * pSingularity->m_uDpVal.Length * m_pParent->GetpPrjParam()->GetSafetyFactor();
				}
				break;

			case CDS_HydroMod::eDpType::QDpref:
				{
					dVal1 = pSingularity->m_uDpVal.sQDpRef.Dpref;
					dVal2 = pSingularity->m_uDpVal.sQDpRef.Qref;
					double dKvRef = 0.0;
					dKvRef = CalcKv( dVal2, dVal1, dRho );
					dDp = CalcDp( dQ, dKvRef, dRho );
				}
				break;
		}
	}
	
	if( dDp < 0.0 )
	{
		dDp = 0.0;
	}

	return dDp;
}

double CPipes::GetSingularityTotalDp( bool bWithConnection, CDS_HydroMod *pNextHM )
{
	if( 0 == (int)m_mapSingularityList.size() )
	{
		return 0.0;
	}
	
	double dQ = GetRealQ();

	if( dQ < 0.0 )
	{
		return 0.0;
	}

	double dTotalDp = 0.0;
	double dDp = 0.0;


	for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
	{
		// Connection singularity are computed by CHydroMod::GetConnectDp().Skip it if not requested.
		if( SINGULARITY_CONNECT_ID == iter->first && false == bWithConnection )
			continue;
		
		dDp = GetSingularityDp( iter->second, pNextHM );
		dTotalDp += dDp;
	}

	if( dTotalDp < 0.0 )
	{
		dTotalDp = 0.0;
	}
	
	return dTotalDp;
}

int CPipes::GetFirstSingularity( CSingularity *pSingularity )
{
	if( NULL == pSingularity || 0 == (int)m_mapSingularityList.size() )
	{
		return -1;
	}
	
	pSingularity->m_nID = m_mapSingularityList.begin()->second->m_nID;
	pSingularity->m_IDPtr = m_mapSingularityList.begin()->second->m_IDPtr;
	pSingularity->m_strDescription = m_mapSingularityList.begin()->second->m_strDescription;
	pSingularity->m_eDpType = m_mapSingularityList.begin()->second->m_eDpType;
	pSingularity->m_uDpVal = m_mapSingularityList.begin()->second->m_uDpVal;
	
	return 1;
}

int CPipes::GetNextSingularity( int iPos, CSingularity *pSingularity )
{
	if( NULL == pSingularity || iPos < 1 || iPos >= (int)m_mapSingularityList.size() )
	{
		return -1;
	}
	
	SingularityIter iter = m_mapSingularityList.begin();
	int iPosToFind = iPos;

	while( iPosToFind > 0 )
	{
		++iter;
		iPosToFind--;
	}
	
	pSingularity->m_nID = iter->second->m_nID;
	pSingularity->m_IDPtr = iter->second->m_IDPtr;
	pSingularity->m_strDescription = iter->second->m_strDescription;
	pSingularity->m_eDpType = iter->second->m_eDpType;
	pSingularity->m_uDpVal = iter->second->m_uDpVal;
	
	return ( iPos + 1 );
}

void CPipes::GetSingularityList( std::vector<CSingularity *> *pVecSingularities, bool bExceptConnect )
{
	if( NULL == pVecSingularities )
	{
		return;
	}

	pVecSingularities->clear();

	if( m_mapSingularityList.size() > 0 )
	{
		for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
		{
			if( true == bExceptConnect && SINGULARITY_CONNECT_ID == iter->first )
			{
				continue;
			}

			CSingularity *pSingularity = new CSingularity;

			if( NULL == pSingularity )
			{
				ASSERT_CONTINUE;
			}

			pSingularity->m_nID = iter->second->m_nID;
			pSingularity->m_IDPtr = iter->second->m_IDPtr;
			pSingularity->m_strDescription = iter->second->m_strDescription;
			pSingularity->m_eDpType = iter->second->m_eDpType;
			pSingularity->m_uDpVal = iter->second->m_uDpVal;
			pVecSingularities->push_back( pSingularity );
		}
	}
}

void CPipes::ClearSingularityVector( std::vector<CSingularity *> *pvecSingularities )
{
	if( NULL == pvecSingularities || 0 == (int)pvecSingularities->size() )
	{
		return;
	}

	for( int iLoop = 0; iLoop < (int)pvecSingularities->size(); iLoop++ )
	{
		if( NULL != pvecSingularities->at( iLoop ) )
		{
			delete pvecSingularities->at( iLoop );
		}
	}
	
	pvecSingularities->clear();
}

void CPipes::FormatSingularityString( CString &FSstr, CSingularity *pSingularity, bool bDescription, bool bInputData, bool bDp )
{
	FSstr.Empty();

	if( pSingularity != NULL )
	{
		FSstr = _FormatSingularityStringHelper( pSingularity, bDescription, bInputData, bDp, false );
	}
	else
	{
		// Determine number of accessories to display.
		// Remark: it's just to know if we must show 'strBullet' or not.
		int iRealCount = 0;

		for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
		{
			if( _tcscmp( iter->second->GetSingulIDPtr().ID, _T("SINGUL_NONE") ) != 0 )
			{
				iRealCount++;
			}
		}
	
		for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
		{
			if( false == FSstr.IsEmpty() )
			{
				FSstr += CString( _T("\r\n") );
			}

			CString strSingularity = _FormatSingularityStringHelper( iter->second, bDescription, bInputData, bDp, ( iRealCount > 1 ) ? true : false );
			FSstr += strSingularity;

		}

		if( true == FSstr.IsEmpty() )
		{
			FSstr = TASApp.LoadLocalizedString( IDS_HMCALC_NOSINGULARITY );
		}
	}
}

void CPipes::CopyAllSingularities( CPipes *pclDestPipe )
{
	if( NULL == pclDestPipe || 0 == (int)m_mapSingularityList.size() )
	{
		return;
	}

	for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
	{
		// If it's a connection and we can copy it, we must verify also if pipes location are the same!
		if(	iter->first >= SINGULARITY_FIRSTACCESSORY_ID )
		{
			pclDestPipe->_CopySingularity( iter->second );
		}
	}
}

int CPipes::WriteAllSingularitiesToBuffer( BYTE *pbBuffer, bool bExceptConnect )
{
	int iTotalSize = 0;
	short nNumberOfSingularities = (short)m_mapSingularityList.size();

	if( ( true == bExceptConnect && nNumberOfSingularities > 1 ) || ( false == bExceptConnect && nNumberOfSingularities > 0 ) )
	{
		if( true == bExceptConnect )
		{
			nNumberOfSingularities--;
		}

		// int: size of data in buffer; short = number of singularities;
		iTotalSize = sizeof( int ) + sizeof( short);

		// Add size of all singularities.
		iTotalSize += ( nNumberOfSingularities * ( sizeof( short) + sizeof( IDPTR ) + sizeof( CDS_HydroMod::eDpType ) + sizeof( CDS_HydroMod::_uDpVal ) ) );
		
		for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
		{
			if( true == bExceptConnect && SINGULARITY_CONNECT_ID == iter->first )
			{
				continue;
			}
				
			// CString is in TCHAR (because project is compiled in Unicode) and we want size in byte.
			iTotalSize += sizeof( int );
			iTotalSize += ( ( iter->second->m_strDescription.GetLength() + 1 ) * 2 );
		}
	}
	
	if( NULL != pbBuffer && 0 != iTotalSize )
	{
		BYTE *pbDest = pbBuffer;
		
		// Write total size of buffer.
		memcpy_s( pbDest, sizeof( int ), &iTotalSize, sizeof( int ) );
		pbDest += sizeof( int );
		
		// Write number of singularities.
		memcpy_s( pbDest, sizeof( short ), &nNumberOfSingularities, sizeof( short ) );
		pbDest += sizeof( short );
		
		for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
		{
			if( true == bExceptConnect && SINGULARITY_CONNECT_ID == iter->first )
			{
				continue;
			}

			memcpy_s( pbDest, sizeof( short ), &iter->second->m_nID, sizeof( short ) );
			pbDest += sizeof( short );
			
			// Write IDPTR class variables.
			memcpy_s( pbDest, ( _ID_LENGTH + 1) * 2, &iter->second->m_IDPtr.ID, ( _ID_LENGTH + 1) * 2 );
			pbDest += ( _ID_LENGTH + 1) * 2;
			memcpy_s( pbDest, sizeof( void* ), &iter->second->m_IDPtr.MP, sizeof( void* ) );
			pbDest += sizeof( void* );
			memcpy_s( pbDest, sizeof( void* ), &iter->second->m_IDPtr.DB, sizeof( void* ) );
			pbDest += sizeof( void* );
			memcpy_s( pbDest, sizeof( void* ), &iter->second->m_IDPtr.PP, sizeof( void* ) );
			pbDest += sizeof( void* );
			
			// Write description (size + string).
			int iLen = ( iter->second->m_strDescription.GetLength() + 1 ) * 2;
			memcpy_s( pbDest, sizeof( int ), &iLen, sizeof( int ) );
			pbDest += sizeof( int );
			memcpy_s( pbDest, iLen, (LPCTSTR)iter->second->m_strDescription, iLen );
			pbDest += iLen;
			
			memcpy_s( pbDest, sizeof( CDS_HydroMod::eDpType ), &iter->second->m_eDpType, sizeof( CDS_HydroMod::eDpType ) );
			pbDest += sizeof( CDS_HydroMod::eDpType );
			
			memcpy_s( pbDest, sizeof( CDS_HydroMod::_uDpVal ), &iter->second->m_uDpVal, sizeof( CDS_HydroMod::_uDpVal ) );
			pbDest += sizeof( CDS_HydroMod::_uDpVal );
		}
	}
	
	return iTotalSize;
}

int CPipes::ReadAllSingularitiesFromBuffer( BYTE *pbBuffer )
{
	if( NULL == pbBuffer )
	{
		return -1;
	}
	
	// Read the total size of the buffer.
	int iTotalSize;
	BYTE *pbSrc = pbBuffer;
	memcpy_s( &iTotalSize, sizeof( int ), pbSrc, sizeof( int ) );
	pbSrc += sizeof( int );

	if( 0 == iTotalSize )
	{
		return -1;
	}

	// Read number of singularities.
	short nNumberOfSingularities;
	memcpy_s( &nNumberOfSingularities, sizeof( short ), pbSrc, sizeof( short ) );
	pbSrc += sizeof( short );

	if( 0 == nNumberOfSingularities )
	{
		return -1;
	}
	
	for( short nLoop = 0; nLoop < nNumberOfSingularities; nLoop++ )
	{
		CSingularity *pSingularity = new CSingularity;

		if( NULL == pSingularity )
		{
			ASSERT_CONTINUE;
		}

		memcpy_s( &pSingularity->m_nID, sizeof( short ), pbSrc, sizeof( short ) );
		pbSrc += sizeof( short );

		// Read IDPTR class variables.
		memcpy_s( &pSingularity->m_IDPtr.ID, ( _ID_LENGTH + 1) * 2, pbSrc, ( _ID_LENGTH + 1) * 2 );
		pbSrc += ( _ID_LENGTH + 1) * 2;
		memcpy_s( &pSingularity->m_IDPtr.MP, sizeof( void* ), pbSrc, sizeof( void* ) );
		pbSrc += sizeof( void* );
		memcpy_s( &pSingularity->m_IDPtr.DB, sizeof( void* ), pbSrc, sizeof( void* ) );
		pbSrc += sizeof( void* );
		memcpy_s( &pSingularity->m_IDPtr.PP, sizeof( void* ), pbSrc, sizeof( void* ) );
		pbSrc += sizeof( void* );

		// Read description (size + string).
		int iLen;
		memcpy_s( &iLen, sizeof( int ), pbSrc, sizeof( int ) );
		pbSrc += sizeof( int );
		TCHAR *ptcDescription = new TCHAR[iLen];

		if( NULL == ptcDescription )
		{
			delete pSingularity;
			return -1;
		}
		
		memcpy_s( ptcDescription, iLen, pbSrc, iLen );
		pbSrc += iLen;
		pSingularity->m_strDescription = ptcDescription;

		memcpy_s( &pSingularity->m_eDpType, sizeof( CDS_HydroMod::eDpType ), pbSrc, sizeof( CDS_HydroMod::eDpType ) );
		pbSrc += sizeof( CDS_HydroMod::eDpType );

		memcpy_s( &pSingularity->m_uDpVal, sizeof( CDS_HydroMod::_uDpVal ), pbSrc, sizeof( CDS_HydroMod::_uDpVal ) );
		pbSrc += sizeof( CDS_HydroMod::_uDpVal );

		short nID = _GetFirstFreeID();
		
		if( SINGULARITY_NOMOREAVAILABLE == nID )
		{
			delete pSingularity;
			break;
		}
		
		pSingularity->m_nID = nID;
		m_mapSingularityList[nID] = pSingularity;
	}

	return iTotalSize;
}

void CPipes::DropData( OUTSTREAM outf, int *piLevel )
{
	CString strTab = _T("");

	for( int i = *piLevel; i; i-- )
	{
		strTab += _T("\t");
	}

	CString str;
	str.Format( _T("m_IDPtr.ID = %s\n"), GetIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_bIsDiversityApplied = %s\n"), ( true == m_bIsDiversityApplied ) ? _T("true") : _T("false") );
	str.Format( _T("m_DiversityIDPtr.ID = %s\n"), GetDiversityIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	
	if( m_pPipeSerie != NULL )
	{
		str.Format( _T("m_pPipeSerie ID = %s\n"), m_pPipeSerie->GetIDPtr().ID );
	}
	else
	{
		str.Format( _T("m_pPipeSerie is not defined\n") );
	}

	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_IDPtrBestPipe.ID = %s\n"), GetBestPipeIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Length = %s\n"), WriteCUDouble( _U_LENGTH, GetLength(), true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_RealQ = %s\n"), WriteCUDouble( _U_FLOW, GetRealQ(), true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_TotalQ = %s\n"), WriteCUDouble( _U_FLOW, GetTotalQ(), true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_PartialPendingQ = %s\n"), WriteCUDouble( _U_FLOW, GetPartialPendingQ(), true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_LinDp = %s\n"), WriteCUDouble( _U_DIFFPRESS, GetLinDp(), true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Velocity = %s\n"), WriteCUDouble( _U_VELOCITY, GetVelocity(), true ) );
	WriteFormatedStringA2( outf, str, strTab );

	str = _T("m_Located = ");

	switch( GetLocate() )
	{
		case CDS_HydroMod::eHMObj::eNone:
			str.Append( _T("eNone\n") );
			break;

		case CDS_HydroMod::eHMObj::eCircuitPrimaryPipe:
			str.Append( _T("eCircuitPrimaryPipe\n") );
			break;

		case CDS_HydroMod::eHMObj::eCircuitBypassPrimaryPipe:
			str.Append( _T("eCircuitBypassPrimaryPipe\n") );
			break;

		case CDS_HydroMod::eHMObj::eCircuitSecondaryPipe:
			str.Append( _T("eCircuitSecondaryPipe\n") );
			break;

		case CDS_HydroMod::eHMObj::eDistributionSupplyPipe:
			str.Append( _T("eDistributionSupplyPipe\n") );
			break;

		case CDS_HydroMod::eHMObj::eDistributionReturnPipe:
			str.Append( _T("eDistributionReturnPipe\n") );
			break;

		case CDS_HydroMod::eHMObj::eBVprim:
			str.Append( _T("eBVprim\n") );
			break;

		case CDS_HydroMod::eHMObj::eBVbyp:
			str.Append( _T("eBVbyp\n") );
			break;

		case CDS_HydroMod::eHMObj::eBVsec:
			str.Append( _T("eBVsec\n") );
			break;

		case CDS_HydroMod::eHMObj::eDpC:
			str.Append( _T("eDpC\n") );
			break;

		case CDS_HydroMod::eHMObj::eCV:
			str.Append( _T("eCV\n") );
			break;

		case CDS_HydroMod::eHMObj::eShutoffValveSupply:
			str.Append( _T("eShutoffValveSupply\n") );
			break;

		case CDS_HydroMod::eHMObj::eShutoffValveReturn:
			str.Append( _T("eShutoffValveReturn\n") );
			break;

		case CDS_HydroMod::eHMObj::eALL:
			str.Append( _T("eALL\n") );
			break;
	}
	
	WriteFormatedStringA2( outf, str, strTab );

	WriteFormatedStringA2( outf, _T("Singularity list\n"), strTab );

	if( 0 == (int)m_mapSingularityList.size() )
	{
		WriteFormatedStringA2( outf, _T("\tThere is no singularity"), strTab );
	}
	else
	{
		for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
		{
			CString strSingularity;
			FormatSingularityString( strSingularity, iter->second, true, true );
			str.Format( _T("\t%s"), strSingularity );
			WriteFormatedStringA2( outf, str, strTab );
		}
	
	}
	WriteFormatedStringA2( outf, _T("\n"), strTab );
}

IDPTR CPipes::CSingularity::GetSingulIDPtr()
{
	if( _T('\0') == *m_IDPtr.ID )
	{
		m_IDPtr = _NULL_IDPTR;
		return m_IDPtr;
	}
	
	if( CDS_HydroMod::eDpType::Pipelength == m_eDpType )
	{
		IDPTR idptr;

		try
		{
			idptr = ( (CData *)( m_IDPtr.MP ) )->GetIDPtr();
		}
		catch(...){}

		if( idptr.MP != m_IDPtr.MP )
		{
			// Try to restore IDPTR.
			m_IDPtr = TASApp.GetpPipeDB()->Get( m_IDPtr.ID );
		}
	}

	return m_IDPtr;
}

CString CPipes::CSingularity::GetDescription()
{
	if( NULL == GetpXmlStrTab() )
	{
		return _T("");
	}

	_string *pStr = GetpXmlStrTab()->GetIDSStr( (LPCTSTR)m_strDescription );

	if( NULL != pStr )
	{
		return pStr->c_str();
	}
	else
	{
		return m_strDescription;
	}
}

#define CPIPES_VERSION	4
// Version 4: HYS-1716: we add water characteristic directly in the pipe.
void CPipes::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CPIPES_VERSION;
	outf.write( (char *)&Version, sizeof( Version ) );
	
	// Info.
	WriteString( outf, m_IDPtr.ID );
	outf.write( (char *)&m_dLength, sizeof( m_dLength ) );
	outf.write( (char *)&m_dRealQ, sizeof( m_dRealQ ) );
	outf.write( (char *)&m_eLocated, sizeof( m_eLocated ) );
	WriteString( outf, m_IDPtrBestPipe.ID );

	// Singularities.
	int iCount = (int)m_mapSingularityList.size();
	outf.write( (char *)&iCount, sizeof( iCount ) );

	for( SingularityIter iter = m_mapSingularityList.begin(); iter != m_mapSingularityList.end(); ++iter )
	{
		CSingularity *pSingularity = iter->second;

		if( NULL == pSingularity )
		{
			continue;
		}

		outf.write( (char *)&pSingularity->m_nID, sizeof( pSingularity->m_nID ) );
		WriteString( outf, (LPCTSTR)pSingularity->m_strDescription );
		outf.write( (char *)&pSingularity->m_eDpType, sizeof( pSingularity->m_eDpType ) );
		
		switch( pSingularity->m_eDpType )
		{
			case CDS_HydroMod::eDpType::Dp:			// union type
			case CDS_HydroMod::eDpType::Kv:
			case CDS_HydroMod::eDpType::Cv:
			case CDS_HydroMod::eDpType::Pipelength:
				outf.write( (char *)&pSingularity->m_uDpVal.Cv, sizeof( pSingularity->m_uDpVal.Cv ) );
				break;

			case CDS_HydroMod::eDpType::dzeta:			// union type
			case CDS_HydroMod::eDpType::QDpref:
				outf.write( (char *)&pSingularity->m_uDpVal.sQDpRef.Dpref, sizeof( pSingularity->m_uDpVal.sQDpRef.Dpref ) );
				outf.write( (char *)&pSingularity->m_uDpVal.sQDpRef.Qref, sizeof( pSingularity->m_uDpVal.sQDpRef.Qref ) );
				break;
			
			default:
				break;
		}
		
		WriteString( outf, (LPCTSTR)pSingularity->GetSingulIDPtr().ID );
	}

	// Remark: No need to write 'm_Locate' because this variable is set when CHMPipes is created.

	// Version 2.
	WriteString( outf, m_DiversityIDPtr.ID );
	outf.write( (char *)&m_dTotalQ, sizeof( m_dTotalQ ) );

	// Version 3.
	outf.write( (char *)&m_bIsDiversityApplied, sizeof( m_bIsDiversityApplied ) );

	// Version 4: HYS-1716: now the water characteristic (And so temperatures) for a circuit are set in the pipes.
	m_clSupplyWaterChar.Write( outf );
	m_clReturnWaterChar.Write( outf );
}

bool CPipes::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CPIPES_VERSION;
	inpf.read( (char *)&Version, sizeof( Version ) );

	if( Version < 1 || Version > CPIPES_VERSION )
	{
		return false;
	}
	
	// Info.
	ReadString( inpf, m_IDPtr.ID, sizeof( m_IDPtr.ID ) );
	bool bPipeIDExist = ( _T('\0') != *m_IDPtr.ID ) ? true : false;
	
	// If Pipe ID exist and pipe is not available in the USER DB set general flag PipeUnavailable.
	if( true == bPipeIDExist )
	{
		IDPTR PipeIDPtr = TASApp.GetpPipeDB()->Get( m_IDPtr.ID );
		
		// TODO AL CPipes::Read() implement a user function to solve missing pipe series
		// Developer code implemented to force a 'standard' pipe in case of missing pipe series
		// remove // in from line loading pipe steel... 
		if( NULL != PipeIDPtr.MP && NULL != PipeIDPtr.PP )
		{
			m_IDPtr = PipeIDPtr;

			// Some pipes could be hidden by the user.
			// When we read a new tsp file remove this.
			if( true == PipeIDPtr.PP->IsHidden() )
			{
				PipeIDPtr.PP->SetHidden( false );
			}

			if( true == PipeIDPtr.MP->IsHidden() )
			{
				PipeIDPtr.MP->SetHidden( false );
			}
		}
	}
	
	if( NULL != m_IDPtr.MP && true == m_IDPtr.MP->IsaTable() )
	{
		m_IDPtr = ( (CTable *)m_IDPtr.MP )->GetFirst();
	}

	m_dLinDp = 0;
	
	// Init pointer on pipe series.
	m_pPipeSerie = m_IDPtr.PP;
	
	// Pipe length.
	inpf.read( (char *)&m_dLength, sizeof( m_dLength ) );
	
	// Pipe Q.
	inpf.read( (char *)&m_dRealQ, sizeof( m_dRealQ ) );
	inpf.read( (char *)&m_eLocated, sizeof( m_eLocated ) );

	// Best pipe ID.
	ReadString( inpf, m_IDPtrBestPipe.ID, sizeof( m_IDPtrBestPipe.ID ) );
	m_IDPtrBestPipe = TASApp.GetpPipeDB()->Get( m_IDPtrBestPipe.ID );

	if( NULL != m_IDPtrBestPipe.MP && true == m_IDPtrBestPipe.MP->IsaTable() )
	{
		m_IDPtrBestPipe = ( (CTable *)m_IDPtrBestPipe.MP )->GetFirst();
	}

	// Singularity.
	RemoveAllSingularities();
	int iCount;
	inpf.read( (char *)&iCount, sizeof( iCount ) );

	for( int i = 0; i < iCount; i++ )
	{
		CSingularity *pSingularity = new CSingularity;

		if( NULL == pSingularity )
		{
			ASSERT_CONTINUE;
		}

		pSingularity->m_IDPtr = _NULL_IDPTR;
		pSingularity->m_nID = 0;
		inpf.read( (char *)&pSingularity->m_nID, sizeof( pSingularity->m_nID ) );
		
		// HYS-853: when one singularity is deleted, assign its key.
		if( pSingularity->m_nID != 0 )
		{
			pSingularity->m_nID = _GetFirstFreeID();
		}

		TCHAR tcBuffer[CSTRING_LINE_SIZE_MAX];
		ReadString( inpf, tcBuffer, sizeof( tcBuffer ) );
		pSingularity->m_strDescription = (CString)tcBuffer;
		
		inpf.read( (char *)&pSingularity->m_eDpType, sizeof( pSingularity->m_eDpType ) );

		switch( pSingularity->m_eDpType )
		{
			case CDS_HydroMod::eDpType::Dp:				// union type
			case CDS_HydroMod::eDpType::Kv:
			case CDS_HydroMod::eDpType::Cv:
			case CDS_HydroMod::eDpType::Pipelength:

				inpf.read( (char *)&pSingularity->m_uDpVal.Cv, sizeof( pSingularity->m_uDpVal.Cv ) );
				break;
			
			case CDS_HydroMod::eDpType::dzeta:			// union type
			case CDS_HydroMod::eDpType::QDpref:

				inpf.read( (char *)&pSingularity->m_uDpVal.sQDpRef.Dpref, sizeof( pSingularity->m_uDpVal.sQDpRef.Dpref ) );
				inpf.read( (char *)&pSingularity->m_uDpVal.sQDpRef.Qref, sizeof( pSingularity->m_uDpVal.sQDpRef.Qref ) );
				
				break;

			default:
				break;
		}
		
		ReadString( inpf, tcBuffer, sizeof( tcBuffer ) );

		if( _T('\0') != *tcBuffer )
		{
			switch( pSingularity->m_eDpType )
			{
				case CDS_HydroMod::eDpType::Pipelength:
					pSingularity->m_IDPtr = TASApp.GetpPipeDB()->Get( tcBuffer );
					ASSERT( _T('\0') != *pSingularity->m_IDPtr.ID );
					break;
				
				default:
					pSingularity->m_IDPtr = m_pParent->m_pSingulTab->Get( tcBuffer );
					ASSERT( _T('\0') != *pSingularity->GetSingulIDPtr().ID );
					break;
			}
		}
		
		m_mapSingularityList[pSingularity->m_nID] = pSingularity;
	}

	if( Version >= 2 )
	{
		// Read pipe ID before diversity fact has been applied.
		ReadString( inpf, m_DiversityIDPtr.ID, sizeof( m_DiversityIDPtr.ID ) );

		bPipeIDExist = ( _T('\0') != *m_DiversityIDPtr.ID ) ? true : false;
	
		// If Pipe ID exist and pipe is not available in the USER DB set general flag PipeUnavailable.
		if( true == bPipeIDExist )
		{
			m_DiversityIDPtr = TASApp.GetpPipeDB()->Get( m_DiversityIDPtr.ID );

			if( NULL != m_DiversityIDPtr.MP )
			{
				// Some pipes could be hidden by the user.
				// When we read a new tsp file remove this.
				if( true == m_DiversityIDPtr.PP->IsHidden() )
				{	
					m_DiversityIDPtr.PP->SetHidden( false );
				}

				if( true == m_DiversityIDPtr.MP->IsHidden() )
				{
					m_DiversityIDPtr.MP->SetHidden( false );
				}
			}
		}
	
		if( NULL != m_DiversityIDPtr.MP && true == m_DiversityIDPtr.MP->IsaTable() )
		{
			m_DiversityIDPtr = ( (CTable *)m_DiversityIDPtr.MP )->GetFirst();
		}

		inpf.read( (char *)&m_dTotalQ, sizeof( m_dTotalQ ) );
	}

	if( Version >= 3 )
	{
		inpf.read( (char *)&m_bIsDiversityApplied, sizeof( m_bIsDiversityApplied ) );
	}
	
	if( Version < 4 )
	{
		return true;
	}

	// Version 4: HYS-1716: now the water characteristic (And so temperatures) for a circuit are set in the pipes.
	m_clSupplyWaterChar.Read( inpf );
	m_clReturnWaterChar.Read( inpf );

	return true;
}

short CPipes::_GetFirstFreeID()
{
	// Find first free place.
	// Remark: search from 'SINGULARITY_FIRSTACCESSORY_ID' because 'SINGULARITY_CONNECT_ID' is reserved for the connection singularity.
	CSingularity Singularity;
	short nID;

	for( nID = SINGULARITY_FIRSTACCESSORY_ID; nID < SINGULARITY_MAXACCESSORY; nID++ )
	{
		if( false == GetSingularity( nID, &Singularity ) )
		{
			break;
		}
	}
	
	if( SINGULARITY_MAXACCESSORY == nID )
	{
		nID = SINGULARITY_NOMOREAVAILABLE;
	}

	return nID;
}

short CPipes::_AddSingularity( short nID, LPCTSTR SingulID )
{
	CDB_Singularity *pSingul = (CDB_Singularity *)( m_pParent->m_pSingulTab->Get( SingulID ).MP );

	if( NULL == pSingul )
	{
		ASSERTA_RETURN( -1 );
	}

	CSingularity *pSingularity = new CSingularity;
	
	if( NULL == pSingularity )
	{
		ASSERTA_RETURN( -1 );
	}
	
	pSingularity->m_nID = nID;
	pSingularity->m_IDPtr = pSingul->GetIDPtr();
	pSingularity->m_strDescription = pSingul->GetDescription();
	pSingularity->m_eDpType = CDS_HydroMod::eDpType::None;
	memset( &pSingularity->m_uDpVal.Dp, 0, sizeof( pSingularity->m_uDpVal ) );
	m_mapSingularityList[pSingularity->m_nID] = pSingularity;
	m_pParent->Modified();

	return pSingularity->m_nID;
}

bool CPipes::_CopySingularity( CSingularity *pSrcSingularity )
{
	short nID = 0;

	if( 0 != pSrcSingularity->m_nID )
	{
		nID = _GetFirstFreeID();
	}

	if( SINGULARITY_NOMOREAVAILABLE == nID )
	{
		return false;
	}

	CSingularity *pDstSingularity = new CSingularity;

	if( NULL == pDstSingularity )
	{
		ASSERTA_RETURN( false );
	}

	pDstSingularity->m_nID = nID;
	pDstSingularity->m_strDescription = pSrcSingularity->m_strDescription;
	pDstSingularity->m_IDPtr = pSrcSingularity->GetSingulIDPtr();
	pDstSingularity->m_eDpType = pSrcSingularity->m_eDpType;

	switch( pDstSingularity->m_eDpType )
	{
		case CDS_HydroMod::eDpType::Dp:
			pDstSingularity->m_uDpVal.Dp = pSrcSingularity->m_uDpVal.Dp;
			break;
		
		case CDS_HydroMod::eDpType::Kv:
			pDstSingularity->m_uDpVal.Kv = pSrcSingularity->m_uDpVal.Kv;
			break;
		
		case CDS_HydroMod::eDpType::Cv:
			pDstSingularity->m_uDpVal.Cv = pSrcSingularity->m_uDpVal.Cv;	
			break;
		
		case CDS_HydroMod::eDpType::dzeta:
			pDstSingularity->m_uDpVal.sDzeta.dzeta = pSrcSingularity->m_uDpVal.sDzeta.dzeta;
			pDstSingularity->m_uDpVal.sDzeta.d = pSrcSingularity->m_uDpVal.sDzeta.d;
			break;
		
		case CDS_HydroMod::eDpType::QDpref:
			pDstSingularity->m_uDpVal.sQDpRef.Dpref = pSrcSingularity->m_uDpVal.sQDpRef.Dpref;
			pDstSingularity->m_uDpVal.sQDpRef.Qref = pSrcSingularity->m_uDpVal.sQDpRef.Qref;
			break;
		
		case CDS_HydroMod::eDpType::Pipelength:
			pDstSingularity->m_uDpVal.Length = pSrcSingularity->m_uDpVal.Length;
			break;
	}

	m_mapSingularityList[pDstSingularity->m_nID] = pDstSingularity;
	m_pParent->Modified();
	
	return true;
}

CString CPipes::_FormatSingularityStringHelper( CSingularity *pSingularity, bool bDescription, bool bInputData, bool bDp, bool bWithBullet )
{
	CString strResult( _T("") );

	if( NULL == pSingularity )
	{
		return strResult;
	}
	
	if( 0 == _tcscmp( pSingularity->GetSingulIDPtr().ID, _T("SINGUL_NONE") ) )
	{
		return strResult;
	}

	CString strBullet = ( true == bWithBullet ) ? CString( _T("-> ") ) : CString( _T("") );
	CString strSeparator = CString( _T(" ; ") );

	// If we must show description...
	if( true == bDescription )
	{
		CString strDescription = _GetSingularityDescription( pSingularity );

		if( false == strDescription.IsEmpty() )
		{
			strResult = strBullet + strDescription;
		}
	}

	// If we must show input data...
	// Remark: it's only possible for an accessory singularity.
	if( true == bInputData )
	{
		CString strInputData = _GetSingularityInputData( pSingularity );

		if( false == strInputData.IsEmpty() )
		{
			// If no description has been wrote before...
			if( true == strResult.IsEmpty() )
			{
				strResult = strBullet + strInputData;
			}
			else
			{
				strResult += strSeparator + strInputData;
			}
		}
	}
		
	// If we must show pressure drop and this one is valid...
	if( true == bDp )
	{
		CString strDp = _GetSingularityDp( pSingularity );

		if( false == strDp.IsEmpty() )
		{
			if( true == strResult.IsEmpty() )
			{
				strResult = strBullet + strDp;
			}
			else
			{
				strResult += strSeparator + strDp;
			}
		}
	}
	
	return strResult;
}

CString CPipes::_GetSingularityDescription( CSingularity *pSingularity )
{
	CString strResult( _T("") );
	
	// If it's a connection singularity...
	if( SINGULARITY_CONNECT_ID == pSingularity->m_nID )
	{
		// Retrieve the correct description (see comments in .h to this method).
		CString strDescription;
		GetConnectDescription( strDescription );

		if( strDescription.Compare( _T("-") ) != 0 )
		{
			FormatString( strResult, IDS_HMPIPE_FORCONNECTION, strDescription );
		}
	}
	else 
	{
		if( false == pSingularity->GetDescription().IsEmpty() )
		{
			strResult = pSingularity->GetDescription();
		}
		
		// If user has not input description...
		if( true == strResult.IsEmpty() )
		{
			strResult = GetSingularityTypeString( pSingularity );
		}
	}

	return strResult;
}

CString CPipes::_GetSingularityInputData( CSingularity *pSingularity )
{
	CString strResult( _T("") );

	if( pSingularity->m_nID >= SINGULARITY_FIRSTACCESSORY_ID )
	{
		CString str, str1, str2;
		CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
		ASSERT( NULL != pUnitDB );

		double dValue = 0.0;

		switch( pSingularity->m_eDpType )
		{
			default:
			case CDS_HydroMod::eDpType::None:
				break;

			case CDS_HydroMod::eDpType::Dp:
				dValue = pSingularity->m_uDpVal.Dp;
				str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_DP );
				str1 = str + WriteCUDouble( _U_DIFFPRESS, dValue ) + GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str();
				break;

			case CDS_HydroMod::eDpType::Cv:
				dValue = pSingularity->m_uDpVal.Cv;
				str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_CV );
				str1 = str + WriteDouble( dValue, 2, 0, true );
				break;

			case CDS_HydroMod::eDpType::Kv:
				dValue = pSingularity->m_uDpVal.Kv;
				str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_KV );
				str1 = str + WriteDouble( dValue, 2, 0, true );
				break;

			case CDS_HydroMod::eDpType::dzeta:
				dValue = pSingularity->m_uDpVal.sDzeta.dzeta;
				str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_ZETA );
				str1 = str + WriteCUDouble( _C_KVCVCOEFF, dValue );
				break;

			case CDS_HydroMod::eDpType::QDpref:
				dValue = pSingularity->m_uDpVal.sQDpRef.Qref;
				str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_QREF );
				str1 = str + WriteCUDouble( _U_FLOW, dValue, true );
				str = str1;
				dValue = pSingularity->m_uDpVal.sQDpRef.Dpref;
				str2 = CString( _T(" ") ) + TASApp.LoadLocalizedString( IDS_DLGACCESSORY_DPREF );
				str1 = str + str2 + WriteCUDouble( _U_DIFFPRESS, dValue, true );
				break;

			case CDS_HydroMod::eDpType::Pipelength:
				{
					dValue = pSingularity->m_uDpVal.Length;
					CDB_Pipe *pDBPipe = NULL;
					CTable *pTab = NULL;

					// Pipe series.
					if( _NULL_IDPTR == pSingularity->GetSingulIDPtr() )
					{
						pTab = dynamic_cast<CTable*>( GetIDPtr().PP );
					}
					else
					{
						pTab = dynamic_cast<CTable*>( pSingularity->GetSingulIDPtr().PP );
					}

					if( NULL != pTab )
					{
						str1 = pTab->GetName();
					}

					pDBPipe = NULL;

					// Pipe size.
					if( _NULL_IDPTR == pSingularity->GetSingulIDPtr() )
					{
						pDBPipe = dynamic_cast<CDB_Pipe*>( GetIDPtr().MP );
					}
					else
					{
						pDBPipe = dynamic_cast<CDB_Pipe*>( pSingularity->GetSingulIDPtr().MP );
					}

					if( NULL != pDBPipe )
					{
						str1 += pDBPipe->GetName();
					}

					str1 += _T(" ");
					str = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_LENGTH );
					str1 += str + WriteCUDouble( _U_LENGTH, dValue, true );
				}
				
				break;
		}

		// If there is some valid input data...
		if( false == str1.IsEmpty() )
		{
			strResult = str1;
		}
	}
	
	return strResult;
}

CString CPipes::_GetSingularityDp( CSingularity *pSingularity )
{
	CString strResult( _T("") );

	if( GetSingularityDp( pSingularity ) > 0.0 )
	{
		CString str1 = TASApp.LoadLocalizedString( IDS_DLGACCESSORY_DP );
		strResult = str1 + WriteCUDouble( _U_DIFFPRESS, GetSingularityDp( pSingularity ), true );
	}
	
	return strResult;
}

void CPipes::SetRealQ( double dRealQ )
{
	if( m_dRealQ != dRealQ )
	{ 
		m_dRealQ = dRealQ; 
		m_dLinDp = 0.0; 
		m_pParent->Modified(); 
	} 
}

void CPipes::SetTotalQ( double dTotalQ )
{
	 m_dTotalQ = dTotalQ;
}

void CPipes::SetWaterChar( CWaterChar *pclWaterChar, CAnchorPt::PipeLocation ePipeLocation )
{
	if( NULL == pclWaterChar )
	{
		ASSERT_RETURN;
	}

	// We do here well a copy of 'pclWaterChar', we do not keep a pointer on it !!
	if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
	{
		m_clSupplyWaterChar = *pclWaterChar;
	}
	else if( CAnchorPt::PipeLocation::PipeLocation_ByPass == ePipeLocation )
	{
		if( CAnchorPt::CircuitSide::CircuitSide_Primary == m_pParent->GetBypassPipeSide() )
		{
			m_clSupplyWaterChar = *pclWaterChar;
		}
		else if( CAnchorPt::CircuitSide::CircuitSide_Secondary == m_pParent->GetBypassPipeSide() )
		{
			m_clReturnWaterChar = *pclWaterChar;
		}
	}
	else if( CAnchorPt::PipeLocation_Return == ePipeLocation )
	{
		m_clReturnWaterChar = *pclWaterChar;
	}
	else
	{
		ASSERT( 0 );
	}
}

void CPipes::SetTemperature( double dTemperature, CAnchorPt::PipeLocation ePipeLocation )
{
	if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
	{
		m_clSupplyWaterChar.UpdateFluidData( dTemperature );
	}
	else if( CAnchorPt::PipeLocation::PipeLocation_ByPass == ePipeLocation )
	{
		if( CAnchorPt::CircuitSide::CircuitSide_Primary == m_pParent->GetBypassPipeSide() )
		{
			m_clSupplyWaterChar.UpdateFluidData( dTemperature );
		}
		else if( CAnchorPt::CircuitSide::CircuitSide_Secondary == m_pParent->GetBypassPipeSide() )
		{
			m_clReturnWaterChar.UpdateFluidData( dTemperature );
		}
	}
	else if( CAnchorPt::PipeLocation_Return == ePipeLocation )
	{
		m_clReturnWaterChar.UpdateFluidData( dTemperature );
	}
	else
	{
		ASSERT( 0 );
	}
}

void CPipes::_FreeBuffer( CMemFile *pclMemFile )
{
	
	if( NULL == pclMemFile )
	{
		return;
	}

	BYTE *pBuffer = pclMemFile->Detach();

	if( NULL != pBuffer )
	{
		free( pBuffer );
	}

}

CWaterChar *CPipes::_GetCurrentWaterChar()
{
	return NULL;
}

bool CPipes::_IsParentHMScheme3Wdividing()
{
	if( NULL == GetpParent() || NULL == GetpParent()->GetpSchcat() )
	{
		return false;
	}

	if( GetpParent()->GetpSchcat()->Get3WType() == CDB_CircSchemeCateg::e3wTypeDividing )
	{
		return true;
	}

	return false;
}
