#include "stdafx.h"
#include <math.h>
#include <float.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "Hydronic.h"
#include "Utilities.h"
#include "ProductSelectionParameters.h"
#include "Select.h"

////////////////////////////////////////////////////////////////
//	CSelectPipe
////////////////////////////////////////////////////////////////
CSelectPipe::CSelectPipe( CProductSelelectionParameters *pclProductSelParams )
{
	m_pclProductSelParams = pclProductSelParams;
	Clean();
}

void CSelectPipe::Clean()
{
	m_strPipeID = _T("");
	m_dLinDp = 0.0;
	m_LinDpFlag = 0;
	m_dU = 0.0;
	m_UFlag = 0;
	m_TpFlag = 0;
	m_iPipeDNKey = 0;
}

void CSelectPipe::CopyFrom( CSelectPipe *pSelPipe )
{
	m_pclProductSelParams = pSelPipe->m_pclProductSelParams;
	m_strPipeID = pSelPipe->GetPipeID();
	m_dLinDp = pSelPipe->GetLinDp();				// Linear Dp
	m_LinDpFlag = pSelPipe->GetLinDpFlag();			// Linear Dp validity flag
	m_dU = pSelPipe->GetU();						// Water velocity
	m_UFlag = pSelPipe->GetUFlag();					// Water velocity validity flag
	m_TpFlag = pSelPipe->GetTpFlag();				// Temperature and pressure validity flag
	m_iPipeDNKey = pSelPipe->GetDNKey();
}

CDB_Pipe *CSelectPipe::GetpPipe()
{
	CDB_Pipe *pclPipe = NULL;

	if( false == m_strPipeID.IsEmpty() )
	{
		if(  NULL != m_pclProductSelParams )
		{
			pclPipe = dynamic_cast<CDB_Pipe *>( m_pclProductSelParams->m_pPipeDB->Get( m_strPipeID ).MP );
		}
		else
		{
			pclPipe = dynamic_cast<CDB_Pipe *>( TASApp.GetpPipeDB()->Get( m_strPipeID ).MP );
		}
	}

	return pclPipe;
}

#define CSELECTPIPE_VERSION		2
// Version 2: 2017-11-17 : 'm_iPipeDNKey' added.
void CSelectPipe::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CSELECTPIPE_VERSION;
	outf.write( (char*)&Version, sizeof( Version ) );
	
	// Infos.
	WriteString( outf, ( LPCTSTR )m_strPipeID );
	outf.write( (char*)&m_dLinDp, sizeof( m_dLinDp ) );
	outf.write( (char*)&m_LinDpFlag, sizeof( m_LinDpFlag ) );
	outf.write( (char*)&m_dU, sizeof( m_dU ) );
	outf.write( (char*)&m_UFlag, sizeof( m_UFlag ) );
	outf.write( (char*)&m_TpFlag, sizeof( m_TpFlag ) );

	// Version 2.
	outf.write( (char*)&m_iPipeDNKey, sizeof( m_iPipeDNKey ) );
}

void CSelectPipe::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version;
	inpf.read( (char*)&Version, sizeof( Version ) );

	if( Version < 1 || Version > CSELECTPIPE_VERSION )
	{
		return;
	}
	
	// Infos.
	if( false == ReadString( inpf, m_strPipeID ) )
	{
		return;
	}
	
	inpf.read( (char*)&m_dLinDp, sizeof( m_dLinDp ) );
	inpf.read( (char*)&m_LinDpFlag, sizeof( m_LinDpFlag ) );
	inpf.read( (char*)&m_dU, sizeof( m_dU ) );
	inpf.read( (char*)&m_UFlag, sizeof( m_UFlag ) );
	inpf.read( (char*)&m_TpFlag, sizeof( m_TpFlag ) );

	if( Version < 2 )
	{
		return;
	}

	// Version 2.
	inpf.read( (char*)&m_iPipeDNKey, sizeof( m_iPipeDNKey ) );
}

////////////////////////////////////////////////////////////////
//	CSelectPipeList
////////////////////////////////////////////////////////////////
CSelectPipeList::CSelectPipeList()
{
	m_pclBestPipe = NULL;
	Clean();
}

CSelectPipeList::~CSelectPipeList()
{
	if( NULL != m_pclBestPipe )
	{
		delete m_pclBestPipe;
	}
}

int CSelectPipeList::SelectPipes( CProductSelelectionParameters *pclProductSelectionParameters, double dFlow, bool bLimitToMinLinDpAndMinVelocity, 
		bool bBestPipeOnly, int iPipeSizeAbove, int iPipeSizeBelow, CWaterChar *pclWaterChar )
{
	if( NULL == pclProductSelectionParameters || NULL == pclProductSelectionParameters->m_pTADS	|| NULL == pclProductSelectionParameters->m_pTADS->GetpTechParams() 
			|| NULL == pclProductSelectionParameters->m_pTADB || NULL == pclProductSelectionParameters->m_pPipeDB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( dFlow <= 0.0 )
	{
		return 0;
	}

	// Look for pipe(s) with respect to the following criteria:
	//		- Maximum working t� (and pressure).
	//		- Minimum, target and maximum linear pressure drops.
	//		- Minimum, target and maximum velocities.
	// In case of no valid selection, remember the pipe which was the closest to the different criteria.

	CTable *pclPipeSeriesTable = dynamic_cast<CTable *>( pclProductSelectionParameters->m_pPipeDB->Get( pclProductSelectionParameters->m_strPipeSeriesID ).MP );

	if( NULL == pclPipeSeriesTable )
	{
		ASSERTA_RETURN( 0 );
	}

	if( NULL != m_pclBestPipe )
	{
		delete m_pclBestPipe;
	}

	m_pclBestPipe = new CSelectPipe( pclProductSelectionParameters );

	if( NULL == m_pclBestPipe )
	{
		ASSERTA_RETURN( 0 );
	}

	CDB_Pipe *pclPipeRequest = NULL;

	if( false == pclProductSelectionParameters->m_strPipeID.IsEmpty() )
	{
		pclPipeRequest = dynamic_cast<CDB_Pipe*>( pclProductSelectionParameters->m_pPipeDB->Get( pclProductSelectionParameters->m_strPipeID ).MP );

		if( NULL == pclPipeRequest )
		{
			ASSERTA_RETURN( 0 );
		}
	}
	
	// Clear previous pipe list.
	Clean();

	CDS_TechnicalParameter *pclTechParams = pclProductSelectionParameters->m_pTADS->GetpTechParams();
	double dTechLinDpMin = pclTechParams->GetPipeMinDp();
	double dTechLinDpMax = pclTechParams->GetPipeMaxDp();
	double dTechLinDpTarget = pclTechParams->GetPipeTargDp();
	double dTechUMin = pclTechParams->GetPipeMinVel();
	double dTechUMax = pclTechParams->GetPipeMaxVel();
	double dTechUTarget = pclTechParams->GetPipeTargVel();
	bool bVelocityTargetUsed = ( 0 == pclTechParams->GetPipeVtargUsed() ) ? false : true;

	// HYS-1147: we can now give one other fluid characteristic (It's to ease code for PIBCV selection with a 6-way valve
	// in the case of the OnOff control with PIBCV (We must compute two pipes, one for cooling and one for heating).
	double dRho = ( NULL == pclWaterChar ) ? pclProductSelectionParameters->m_WC.GetDens() : pclWaterChar->GetDens();
	double dNu = ( NULL == pclWaterChar ) ? pclProductSelectionParameters->m_WC.GetKinVisc() : pclWaterChar->GetKinVisc();
	double dTemperature = ( NULL == pclWaterChar ) ? pclProductSelectionParameters->m_WC.GetTemp() : pclWaterChar->GetTemp();

	CSelectPipe clPityPipe( pclProductSelectionParameters );
	CSelectPipe clBestPipe( pclProductSelectionParameters );
	double dPityValue = DBL_MAX;
	double dBestValue = DBL_MAX;
	
	// For each pipe.
	for( IDPTR IDPtr = pclPipeSeriesTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclPipeSeriesTable->GetNext() )
	{
		CDB_Pipe *pclPipe = (CDB_Pipe *)( IDPtr.MP );

		if( false == pclPipe->IsSelectable( true ) )
		{
			continue;
		}
		
		// If we have a specific pipe but it's not the same as the current one in the loop...
		if( NULL != pclPipeRequest && pclPipeRequest != pclPipe )
		{
			continue;
		}

		double dDiamater = pclPipe->GetIntDiameter();
		double dVelocity = dFlow / ( 0.785398163398 * dDiamater * dDiamater );
		double dLinDp = CalcPipeDp( dFlow, dRho, dNu, pclPipe->GetRoughness(), dDiamater );

		double dValue = pow( dLinDp - dTechLinDpTarget, 2 ) / pow( dTechLinDpTarget + 1.0e-6, 2 );
		
		if( true == bVelocityTargetUsed )
		{
			dValue += pow( dVelocity - dTechUTarget, 2 ) / pow( dTechUTarget + 1.0e-6, 2 );
		}

		// If all criteria met, add pipe to pre-selection list
		bool bPityCase = true;

		if( dLinDp <= dTechLinDpMax && dVelocity <= dTechUMax )
		{
			bPityCase = false;

			if( true == bLimitToMinLinDpAndMinVelocity && ( dLinDp < dTechLinDpMin || dVelocity < dTechUMin ) )
			{
				bPityCase = true;
			}
		}
		
		if( false == bPityCase )
		{
			CSelectPipe Pipe( pclProductSelectionParameters );
			Pipe.SetPipeID( pclPipe->GetIDPtr().ID );
			Pipe.SetLinDp( dLinDp );
			Pipe.SetU( dVelocity );
			Pipe.SetLinDpFlag( false );
			Pipe.SetUFlag( false );
			Pipe.SetTpFlag( dTemperature > ( (CDB_Pipe*)IDPtr.MP )->GetTmax() );
			Pipe.SetDNKey( ( (CDB_Pipe*)IDPtr.MP )->GetSizeKey( pclProductSelectionParameters->m_pTADB ) );
			
			// m_PipeMMap.insert( PipePair( pclPipe->GetSizeKey( pclProductSelectionParameters->m_pTADB ), Pipe ) );

			if( dValue < dBestValue )
			{
				dBestValue = dValue;
				
				// Copy current pipe to 'm_pclBestPipe'.
				Pipe.Copy( m_pclBestPipe );
			}
		}
		else
		{
			// Compute 'PityValue' and update if needed the pity pipe.
			// 'PityValue' means the best valve of the worst list.
			if( dValue < dPityValue )
			{
				dPityValue = dValue;
				clPityPipe.SetPipeID( pclPipe->GetIDPtr().ID );
				clPityPipe.SetLinDp( dLinDp );
				clPityPipe.SetU( dVelocity );
				clPityPipe.SetLinDpFlag( false );
				clPityPipe.SetUFlag( false );
				clPityPipe.SetTpFlag( dTemperature > ( (CDB_Pipe*)IDPtr.MP )->GetTmax() ); 
				clPityPipe.SetDNKey( ( (CDB_Pipe*)IDPtr.MP )->GetSizeKey( pclProductSelectionParameters->m_pTADB ) );
			}
		}
	}
	
	// ASSERT( 0 != m_PipeMMap.size() || DBL_MAX != dPityValue );

	if( DBL_MAX == dBestValue )
	{
		// If no pipe is pre-selected, insert the pity pipe in the pre-selection list.
		// m_PipeMMap.insert( PipePair( clPityPipe.GetpPipe()->GetSizeKey( pclProductSelectionParameters->m_pTADB ), clPityPipe ) );

		// Copy current pity pipe to 'm_pclBestPipe'.
		clPityPipe.Copy( m_pclBestPipe );
	}

	m_PipeMMap.insert( PipePair( m_pclBestPipe->GetpPipe()->GetSizeKey( pclProductSelectionParameters->m_pTADB ), *m_pclBestPipe ) );

	if( false == bBestPipeOnly && ( 0 != iPipeSizeAbove || 0 != iPipeSizeBelow ) )
	{
		for( IDPTR IDPtr = pclPipeSeriesTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclPipeSeriesTable->GetNext() )
		{
			CDB_Pipe *pclPipe = (CDB_Pipe *)( IDPtr.MP );

			if( pclPipe == m_pclBestPipe->GetpPipe() )
			{
				continue;
			}

			if( false == pclPipe->IsSelectable( true ) )
			{
				continue;
			}

			if(  ( pclPipe->GetSizeKey( pclProductSelectionParameters->m_pTADB ) <= ( m_pclBestPipe->GetDNKey() + iPipeSizeAbove ) &&
				   pclPipe->GetSizeKey( pclProductSelectionParameters->m_pTADB ) >= m_pclBestPipe->GetDNKey() ) 
			  || ( pclPipe->GetSizeKey( pclProductSelectionParameters->m_pTADB ) >= ( m_pclBestPipe->GetDNKey() - iPipeSizeBelow ) &&
				   pclPipe->GetSizeKey( pclProductSelectionParameters->m_pTADB ) < m_pclBestPipe->GetDNKey() ) )
			{
				double dDiamater = pclPipe->GetIntDiameter();
				double dVelocity = dFlow / ( 0.785398163398 * dDiamater * dDiamater );
				double dLinDp = CalcPipeDp( dFlow, dRho, dNu, pclPipe->GetRoughness(), dDiamater );

				CSelectPipe Pipe( pclProductSelectionParameters );
				Pipe.SetPipeID( pclPipe->GetIDPtr().ID );
				Pipe.SetLinDp( dLinDp );
				Pipe.SetU( dVelocity );
				Pipe.SetLinDpFlag( false );
				Pipe.SetUFlag( false );
				Pipe.SetTpFlag( dTemperature > ( (CDB_Pipe*)IDPtr.MP )->GetTmax() );
				Pipe.SetDNKey( ( (CDB_Pipe*)IDPtr.MP )->GetSizeKey( pclProductSelectionParameters->m_pTADB ) );
			
				m_PipeMMap.insert( PipePair( pclPipe->GetSizeKey( pclProductSelectionParameters->m_pTADB ), Pipe ) );
			}
		}
	}

	return m_PipeMMap.size();
}

CSelectPipe *CSelectPipeList::GetFirstSelectPipe()
{
	if( 0 == m_PipeMMap.size() )
	{
		return NULL;
	}

	m_Iter = m_PipeMMap.begin();

	CSelectPipe *pSelectPipe = &m_Iter->second;
	m_Iter++;

	return pSelectPipe;
}

CSelectPipe *CSelectPipeList::GetNextSelectPipe()
{
	if( 0 == m_PipeMMap.size() || m_PipeMMap.end() == m_Iter )
	{
		return NULL;
	}

	CSelectPipe *pSelectPipe = &m_Iter->second;
	m_Iter++;

	return pSelectPipe;
}

CSelectPipe *CSelectPipeList::GetBestSelectPipe( void )
{
	CSelectPipe *pclBestSelectPipe = NULL;

	if( m_PipeMMap.size() > 0 && NULL != m_pclBestPipe && NULL != m_pclBestPipe->GetpPipe() )
	{
		pclBestSelectPipe = m_pclBestPipe;
	}

	return pclBestSelectPipe;
}

CDB_Pipe *CSelectPipeList::GetBestPipe()
{
	CDB_Pipe *pclBestPipe = NULL;

	if( m_PipeMMap.size() > 0 && NULL != m_pclBestPipe && NULL != m_pclBestPipe->GetpPipe() )
	{
		pclBestPipe = m_pclBestPipe->GetpPipe();
	}

	return pclBestPipe;
}

bool CSelectPipeList::GetPipeSizeBound( IDPTR *pSmallestPipeIDPtr, IDPTR *pLargestPipeIDPtr )
{
	m_SmallestPipeIDPtr = _NULL_IDPTR;
	m_LargestPipeIDPtr = _NULL_IDPTR;

	PipeMMap::iterator cIter = m_PipeMMap.begin();
	int iBig = cIter->first;
	
	CDB_Pipe *pclPipe = cIter->second.GetpPipe();
	ASSERT( NULL != pclPipe );

	m_LargestPipeIDPtr = pclPipe->GetIDPtr();

	cIter = m_PipeMMap.end();
	--cIter;
	
	int iSmall = cIter->first;
	ASSERT( iBig >= iSmall );

	pclPipe = cIter->second.GetpPipe();
	ASSERT( NULL != pclPipe );

	m_SmallestPipeIDPtr = pclPipe->GetIDPtr();

	if( NULL != pSmallestPipeIDPtr )
	{
		*pSmallestPipeIDPtr = m_SmallestPipeIDPtr;
	}

	if( NULL != pLargestPipeIDPtr )
	{
		*pLargestPipeIDPtr = m_LargestPipeIDPtr;
	}

	return true;
}

void CSelectPipeList::GetMatchingPipe( int iSizeKey, CSelectPipe &selPipe )
{
	PipeMMap::const_iterator cIt;
	cIt = m_PipeMMap.lower_bound( iSizeKey );
	
	// Not found take the latest.
	if( cIt == m_PipeMMap.end() )
	{
		--cIt;
	}

	selPipe.CopyFrom( (CSelectPipe*)&cIt->second );
}

void CSelectPipeList::Clean()
{
	m_PipeMMap.clear();
	m_Iter = m_PipeMMap.end();
	m_SmallestPipeIDPtr = _NULL_IDPTR;
	m_LargestPipeIDPtr = _NULL_IDPTR;
}

void CSelectPipeList::CopyFrom( CSelectPipeList *pclSelectPipeList )
{
	if( NULL == pclSelectPipeList || NULL == pclSelectPipeList->GetpPipeMap() )
	{
		return;
	}

	PipeMMap *pmapPipe = pclSelectPipeList->GetpPipeMap();

	for( PipeMMap::iterator iter = pmapPipe->begin(); iter != pmapPipe->end(); iter++ )
	{
		CSelectPipe clSelectPipe( iter->second.GetProductSelParams() );
		clSelectPipe.Clean();
		clSelectPipe.CopyFrom( &iter->second );
		m_PipeMMap.insert( PipePair( iter->first, clSelectPipe ) );
	}

	m_SmallestPipeIDPtr = pclSelectPipeList->GetSmallestPipe();
	m_LargestPipeIDPtr = pclSelectPipeList->GetLargestPipe();

	if( NULL != m_pclBestPipe )
	{
		delete m_pclBestPipe;
		m_pclBestPipe = NULL;
	}
	
	if( NULL != pclSelectPipeList->GetBestSelectPipe() )
	{
		m_pclBestPipe = new CSelectPipe( pclSelectPipeList->GetBestSelectPipe()->GetProductSelParams() );

		if( NULL == m_pclBestPipe )
		{
			ASSERT_RETURN;
		}

		m_pclBestPipe->CopyFrom( pclSelectPipeList->GetBestSelectPipe() );
	}
}

//////////////////////////////////////////////////////////////////////////////////
//	Selected product base
//////////////////////////////////////////////////////////////////////////////////
CSelectedBase::CSelectedBase( SelectedBaseType eSelectedBaseType )
{
	m_eSelectedBaseType = eSelectedBaseType;
	m_IDPtr = _NULL_IDPTR;	// Valve IDPTR
	m_ulFlags = 0;
	m_dDp = 0.0;
	m_mapFlagsErrorCode.clear();
}

void CSelectedBase::SetFlag( Flags eFlag, bool bSet, int iErrorCode )
{
	unsigned long ulFlag = (unsigned long)eFlag;
	
	if( true == bSet )
	{
		// Set.
		m_ulFlags |= ulFlag;

		if( -1 != iErrorCode )
		{
			m_mapFlagsErrorCode[ eFlag ] = iErrorCode;
		}
	}	
	else
	{
		// Clear.
		ulFlag ^= 0xFFFF;
		m_ulFlags &= ulFlag;

		if( -1 != iErrorCode && m_mapFlagsErrorCode.count( eFlag ) > 0 )
		{
			m_mapFlagsErrorCode.erase( eFlag );
		}
	}
}

bool CSelectedBase::GetFlag( Flags eFlag, int *piErrorCode )
{
	bool bReturn = ( 0 == ( eFlag & m_ulFlags ) ) ? false : true ;
	
	if( NULL != piErrorCode && m_mapFlagsErrorCode.count( eFlag ) > 0 )
	{
		*piErrorCode = m_mapFlagsErrorCode[eFlag];
	}
	
	return bReturn;
}

int CSelectedBase::GetFlagError( Flags eFlag )
{
	int iError = -1;

	if( m_mapFlagsErrorCode.count( eFlag ) > 0 )
	{
		iError = m_mapFlagsErrorCode[eFlag];
	}

	return iError;
}

void CSelectedBase::CopyFrom( CSelectedBase *pFromSelectedProduct )
{
	if( NULL == pFromSelectedProduct || NULL == pFromSelectedProduct->GetpmapFlagsInt() )
	{
		return;
	}

	m_IDPtr = pFromSelectedProduct->GetProductIDPtr();
	m_ulFlags = pFromSelectedProduct->GetFlag( efAll );
	m_dDp = pFromSelectedProduct->GetDp();

	m_mapFlagsErrorCode.clear();
	mapFlagsInt *pmapFlagsInt = pFromSelectedProduct->GetpmapFlagsInt();

	for( mapFlagsIntIter iter = pmapFlagsInt->begin(); iter != pmapFlagsInt->end(); iter++ )
	{
		m_mapFlagsErrorCode[iter->first] = iter->second;
	}
}

//////////////////////////////////////////////////////////////////////////////////
//	Selected valve.
//////////////////////////////////////////////////////////////////////////////////
CSelectedValve::CSelectedValve()
	: CSelectedBase( CSelectedBase::SBT_Valve )
{
	m_dDpSignal = 0.0;
	m_dDpQuarterOpen = 0.0;
	m_dDpHalfOpen = 0.0;
	m_dDpFullOpen = 0.0;
	m_dh = 0.0;
	m_dBestDelta = 0.0;
	m_dDpMin = 0.0;
	m_dReqDpMv = 0.0;
	m_dHMin = 0.0;
}

void CSelectedValve::CopyFrom( CSelectedBase *pFromSelectedProduct )
{
	if( NULL == pFromSelectedProduct )
	{
		return;
	}

	CSelectedBase::CopyFrom( pFromSelectedProduct );

	CSelectedValve *pclSelectedValve = dynamic_cast<CSelectedValve *>( pFromSelectedProduct );

	if( NULL == pclSelectedValve )
	{
		return;
	}

	m_dDpSignal = pclSelectedValve->GetDpSignal();
	m_dDpQuarterOpen = pclSelectedValve->GetDpQuarterOpen();
	m_dDpHalfOpen = pclSelectedValve->GetDpHalfOpen();
	m_dDpFullOpen = pclSelectedValve->GetDpFullOpen();
	m_dh = pclSelectedValve->GetH();
	m_dBestDelta = pclSelectedValve->GetBestDelta();
	m_dDpMin = pclSelectedValve->GetDpMin();
	m_dReqDpMv = pclSelectedValve->GetRequiredDpMv();
	m_dHMin = pclSelectedValve->GetHMin();
}

//////////////////////////////////////////////////////////////////////////////////
//	Selected separator.
//////////////////////////////////////////////////////////////////////////////////
void CSelectedSeparator::CopyFrom( CSelectedBase *pFromSelectedProduct )
{
	if( NULL == pFromSelectedProduct )
	{
		return;
	}

	CSelectedBase::CopyFrom( pFromSelectedProduct );

	CSelectedSeparator *pclSelectedSeparator = dynamic_cast<CSelectedSeparator*>( pFromSelectedProduct );

	if( NULL == pclSelectedSeparator )
	{
		return;
	}

	// Nothing for the moment.
}

//////////////////////////////////////////////////////////////////////////////////
//	Selected safety valve.
//////////////////////////////////////////////////////////////////////////////////
CSelectedSafetyValve::CSelectedSafetyValve() 
		: CSelectedBase( CSelectedBase::SBT_SafetyValve )
{
	m_iQuantityNeeded = 0;
}

void CSelectedSafetyValve::CopyFrom( CSelectedBase *pFromSelectedProduct )
{
	if( NULL == pFromSelectedProduct )
	{
		return;
	}

	CSelectedBase::CopyFrom( pFromSelectedProduct );

	CSelectedSafetyValve *pclSelectedSafetyValve = dynamic_cast<CSelectedSafetyValve*>( pFromSelectedProduct );

	if( NULL == pclSelectedSafetyValve )
	{
		return;
	}

	m_iQuantityNeeded = pclSelectedSafetyValve->GetQuantityNeeded();
}

//////////////////////////////////////////////////////////////////////////////////
//	CSelectedProductList
//////////////////////////////////////////////////////////////////////////////////
CSelectedProductList::~CSelectedProductList()
{
	for( auto &iter : m_MMapKeyDecreasing )
	{
		if( NULL != iter.second)
		{
			delete iter.second;
		}
	}

	m_MMapKeyDecreasing.clear();
	m_PreselMMapKeyDecreasing.clear();
}

int CSelectedProductList::GetCount()
{
	return m_MMapKeyDecreasing.size();
}

void CSelectedProductList::CreateKeyAndInsertInMap( ProductMMapKeyDecreasing *pMap, CDB_TAProduct *pSourceProduct, CSelectedBase *pclProductToInsert )
{
	double dKey = 0.0;
	CTAPSortKey sKey( pSourceProduct );
	int iBits = 0;

	// Add 1 bit for priority.
	LONGLONG LL = ( true == pclProductToInsert->IsFlagSet( CSelectedBase::eNotPriority ) ? 0 : 1 );

	// Add size in the key (size take 5 bits).
	LONGLONG LL1 = sKey.GetPartialSortingKey( iBits, &CTAPSortKey::CTAPSKMask( CTAPSortKey::TAPSKM_All ), CTAPSortKey::TAPKO_Size, -1 );
	
	// Shift priority of 5 bits and add size key.
	LL <<= iBits;
	LL += LL1;
	
	// Add KVS after size...up to 7FFFH->15bits (32768) that means Kvs max = 3276,8 ...
	CDB_ControlValve *pCtrlValve = dynamic_cast<CDB_ControlValve*>( pSourceProduct );
	
	if( NULL != pCtrlValve )
	{
		// Case of control valve.
		// Remark: we multiply 'Kvs' by 10 for the first low value.
		LONGLONG llKvs = (LONGLONG)( pCtrlValve->GetKvs() * 10.0 );
		ASSERT( llKvs < 0x000FFFFF );

		LL <<= 20;
		LL += llKvs;
	}
	else if( true == pSourceProduct->IsKvSignalEquipped() && -1.0 != pSourceProduct->GetKvSignal() )
	{
		LONGLONG llKvs = (LONGLONG)( pSourceProduct->GetKvSignal() * 10.0 );
		ASSERT( llKvs < 0x000FFFFF );

		LL <<= 20;
		LL += llKvs;
	}
	else if( NULL != pSourceProduct->GetValveCharacteristic() && -1.0 != pSourceProduct->GetValveCharacteristic()->GetKvMax() )
	{
		LONGLONG llKvs = (LONGLONG)( pSourceProduct->GetValveCharacteristic()->GetKvMax() * 10.0 );
		ASSERT( llKvs < 0x000FFFFF );

		LL <<= 20;
		LL += llKvs;
	}
	
	// Add type (4 bits), connection (6 bits), body material (4 bits) and version (6 bits).
	// Remark: don't forget that 'ValvMMap' is a multimap sorted from greater to lower value. Because the first two keys are the priority and the size.
	// 1 is the greater priority while lower values are for lower priority. And we want to show valve beginning with the bigger size and going to the lower
	// size. BUT, for the type, connection, body material and version properties we want the same order as when filling combos in the 'DlgIndSelXXX'.
	// This is why for type, connection, body material and version we add the 'mskInverse' to inverse value..
	LL1 = sKey.GetPartialSortingKey( iBits, &CTAPSortKey::CTAPSKMask( CTAPSortKey::TAPSKM_All ), CTAPSortKey::TAPKO_Type | CTAPSortKey::TAPKO_Inverse, 
			CTAPSortKey::TAPKO_Conn | CTAPSortKey::TAPKO_Inverse, CTAPSortKey::TAPKO_BdyMat | CTAPSortKey::TAPKO_Inverse, CTAPSortKey::TAPKO_Vers | 
			CTAPSortKey::TAPKO_Inverse , -1 );

	LL <<= iBits;

	// We have now a total: 1+5+16+4+6+4+6 = 42 bits for an available space of 52 bits.
	
	LL += LL1;
	dKey = (double) LL;
		
	pMap->insert( ProductPair( dKey, pclProductToInsert ) );
}

void CSelectedProductList::Clean()
{
	m_clSelectPipeList.Clean();
	m_PreselMMapKeyDecreasing.clear();
}

void CSelectedProductList::CopyFrom( CSelectedProductList *pclSelectedProductList )
{
	if( NULL == pclSelectedProductList || NULL == pclSelectedProductList->GetSelectPipeList() )
	{
		return;
	}

	Clean();

	// Copy list of selected pipes.
	m_clSelectPipeList.CopyFrom( pclSelectedProductList->GetSelectPipeList() );
}

//////////////////////////////////////////////////////////////////////////////////
//	Base class for all individual selections.
//////////////////////////////////////////////////////////////////////////////////
CIndividualSelectBaseList::CIndividualSelectBaseList()
{
}

CIndividualSelectBaseList::~CIndividualSelectBaseList()
{
}

void CIndividualSelectBaseList::Clean()
{
	CSelectedProductList::Clean();

	for( auto &iter : m_MMapKeyDecreasing )
	{
		if( NULL != iter.second)
		{
			delete iter.second;
		}
	}

	m_MMapKeyDecreasing.clear();
}

void CIndividualSelectBaseList::CopyFrom( CSelectedProductList *pclSelectedProductList )
{
	// We can copy only if the objects are the same.
	if( NULL == pclSelectedProductList || NULL == dynamic_cast<CIndividualSelectBaseList *>( pclSelectedProductList ) )
	{
		return;
	}

	CSelectedProductList::CopyFrom( pclSelectedProductList );

	// For individual selection all the 'CSelectedProduct' objects are saved in the 'm_MMapKeyDecreasing' variable.
	for( auto &iter : pclSelectedProductList->m_MMapKeyDecreasing ) 
	{
		if( NULL == iter.second )
		{
			continue;
		}

		if( CSelectedBase::SBT_Valve == iter.second->GetSelectedBaseType() )
		{
			CSelectedValve *pclSelectedValve = new CSelectedValve();

			if( NULL == pclSelectedValve )
			{
				continue;
			}

			pclSelectedValve->CopyFrom( iter.second );
			m_MMapKeyDecreasing.insert( ProductPair( iter.first, pclSelectedValve ) );
		}
		else if( CSelectedBase::SBT_SafetyValve == iter.second->GetSelectedBaseType() )
		{
			CSelectedSafetyValve *pclSelectedSafetyValve = new CSelectedSafetyValve();

			if( NULL == pclSelectedSafetyValve )
			{
				continue;
			}

			pclSelectedSafetyValve->CopyFrom( iter.second );
			m_MMapKeyDecreasing.insert( ProductPair( iter.first, pclSelectedSafetyValve ) );
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////
//	Selected Balancing Valve container contain all balancing valve that match selection criteria
//////////////////////////////////////////////////////////////////////////////////
CSelectList::CSelectList() : CIndividualSelectBaseList()
{
	Clean();
}

CSelectList::~CSelectList()
{
	m_mapPriority.clear();
}

void CSelectList::Clean()
{
	CIndividualSelectBaseList::Clean();

	m_bForHub = false;
	m_bForHubStation = false;
	m_dPpn = 0.0;
	m_bForceFullOpening = false;
	m_pbDiffDpTrv = NULL;
	m_pdMaxDpTrv = NULL;
	m_dSelDpTot = 0.0;
	m_bUseWithThrmHead = false;
	m_eSelMvLoc = eMvLoc::MvLocSecondary;
	
	// Clean previous allocated memory
	m_mapPriority.clear();
}

int CSelectList::GetManBvList( CIndSelBVParams *pclIndSelBVParam, bool bForceFullOpening )
{
	if( NULL == pclIndSelBVParam || NULL == pclIndSelBVParam->m_pTADS || NULL == pclIndSelBVParam->m_pTADS->GetpTechParams() || NULL == pclIndSelBVParam->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	m_mapPriority.clear();

	return _GetList( pclIndSelBVParam, bForceFullOpening, false );
}

void CSelectList::AddTAP( CDB_TAProduct *pSourceProduct, CSelectedValve *pclSelectedTAP )
{
	if( NULL == pSourceProduct || NULL == pclSelectedTAP )
	{
		return;
	}
	
	CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pSourceProduct, pclSelectedTAP );
}

void CSelectList::CleanPreselectionList( CProductSelelectionParameters *pclProductSelectionParameters )
{
	// Filtering and cleaning operations
	// If a pre-selection list contains at least one fully valid selection,
	// all partially invalid selections (in temp. or min. opening)
	// are deleted.
	
	CDS_TechnicalParameter *pclTechParam = pclProductSelectionParameters->m_pTADS->GetpTechParams();
	m_cIter = m_PreselMMapKeyDecreasing.begin();

	while( m_cIter != m_PreselMMapKeyDecreasing.end() )
	{
		ProductMMapKeyDecreasing::const_iterator cit = m_cIter;
		++cit;
		
		if( 1 == pclTechParam->GetDpCHide() && true == ( (CSelectedValve *)m_cIter->second )->IsFlagSet( CSelectedBase::Flags::eValveDpCHide ) )
		{
			if( NULL != m_cIter->second )
			{
				delete m_cIter->second;
			}

			m_PreselMMapKeyDecreasing.erase( m_cIter );
		}

		m_cIter = cit;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CSelectList::_GetList( CIndSelBVParams *pclIndSelBVParam, bool bForceFullOpening, bool fForTrv )
{
	// Abort selection if pipe selection return nothing.
	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return 0;
	}

	// Get PN pressure.
	m_dPpn = 0.0;

	if( false == pclIndSelBVParam->m_strComboPNID.IsEmpty() )
	{
		m_dPpn = _tcstod( ( (CDB_StringID*)pclIndSelBVParam->m_pTADB->Get( pclIndSelBVParam->m_strComboPNID ).MP )->GetIDstr(), '\0' );
	}
	
	// Get water characteristics (T, P, rho and nu).
	bool bIsDpDefined = ( pclIndSelBVParam->m_dDp > 0.0 ) ? true : false;
	m_bForceFullOpening = bForceFullOpening;
	
	// Get smallest and biggest pipes.
	if( false == m_clSelectPipeList.GetPipeSizeBound() )
	{
		return 0;
	}

	CDB_Pipe *pSmallestPipe = m_clSelectPipeList.GetpSmallestPipe();
	
	if( NULL == pSmallestPipe )
	{
		return 0;
	}

	CDB_Pipe *pBiggestPipe = m_clSelectPipeList.GetpLargestPipe();

	if( NULL == pBiggestPipe )
	{
		return 0;
	}

	// Take into account SizeShiftAbove and SizeShiftBelow.
	// Setup variables to possibly enforce combined SizeShifts.
	// In case of Hub Selection don't use SizeShift from TechParam.
	int iSizeShiftUp;
	int iSizeShiftDown;
	CDS_TechnicalParameter *pclTechParams = pclIndSelBVParam->m_pTADS->GetpTechParams();

	if( true == m_bForHub || 99 == pclTechParams->GetSizeShiftAbove() ) // Combined SizeShift +3/-3 enforced
	{
		iSizeShiftUp = 3;
		iSizeShiftDown = -3;
	}
	else
	{
		iSizeShiftUp = pclTechParams->GetSizeShiftAbove();
		iSizeShiftDown = pclTechParams->GetSizeShiftBelow();
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclIndSelBVParam->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN = pclTableDN->GetBiggestDN();
	int iHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelBVParam->m_pTADB ) + iSizeShiftUp );
	int iLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelBVParam->m_pTADB ) + iSizeShiftDown );
	
	// In case of pipe with a size below DN15 force a selection up to DN15.
	int iDN15 = pclTableDN->GetSize( _T("DN_15") );

	// m_mapBest store for all PIPE size best valve value
	m_rBest.dBestDpAboveMin2 = DBL_MAX;
	m_rBest.dBestDpBelowMin = 0.0;
	m_rBest.dBestDp = 0.0;
	m_rBest.dBestDelta = DBL_MAX;

	_string str;
	LPARAM lpParam;

	for( BOOL bContinue = pclIndSelBVParam->m_BvList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelBVParam->m_BvList.GetNext( str, lpParam ) )
	{
		CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *)lpParam);
		ASSERT( NULL != pTAP );

		if( NULL == pTAP )
		{
			continue;
		}

		int iSizeKey = pTAP->GetSizeKey();

		// m_mapPriority store for each size the minimum priority level (most important)
		if( 0 == m_mapPriority.count( iSizeKey ) )
		{
			m_mapPriority[ iSizeKey ] = INT_MAX;
		}
	}

	bool bMinDpOk = false;

	// Do a loop on devices finding these with the correct type, connection, version, size, max pressure and temperature.
	bool bValidFound = false;
	CString strTypeID;
	
	for( BOOL bContinue = pclIndSelBVParam->m_BvList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelBVParam->m_BvList.GetNext( str, lpParam ) )
	{
		CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *)lpParam );
		ASSERT( NULL != pTAP );

		if( NULL == pTAP )
		{
			continue;
		}

		if( true == strTypeID.IsEmpty() )
		{
			strTypeID = pTAP->GetTypeID();
		}

		if( m_dPpn > 0.0 && false == pTAP->IsPNIncludedInPNRange( m_dPpn ) )
		{
			continue;
		}
		
		// If valve is not a return valve and the highest size is below DN15, we must accept DN15.
		if( strTypeID != _T("RVTYPE_RV") && iHighestSize < iDN15 )
		{
			if( pTAP->GetSizeKey() < iLowestSize || pTAP->GetSizeKey() > iDN15 )
			{
				continue;
			}
		}
		else
		{
			if( pTAP->GetSizeKey() < iLowestSize || pTAP->GetSizeKey() > iHighestSize )
			{
				continue;
			}
		}

		
		if( true == m_bForHub && false == pTAP->IsAvailable( m_bForHub ) )
		{
			continue;
		}

		if( true == m_bForHub && ( m_bForHubStation != pTAP->IsForHubStation() ) )
		{
			continue;
		}

		if( _T("RVTYPE_FO") == strTypeID )
		{
			_SelectFO( pclIndSelBVParam, pTAP, bValidFound );
		}
		else if( _T("RVTYPE_CS") == strTypeID )
		{
			_SelectCS( pclIndSelBVParam, pTAP, bValidFound );
		}
		else if( _T("RVTYPE_VV") == strTypeID )
		{
			_SelectVV( pclIndSelBVParam, pTAP, bValidFound );
		}
		else
		{
			_SelectBV( pclIndSelBVParam, pTAP, bValidFound );
		}
	}

	if( _T("RVTYPE_FO") == strTypeID )
	{
		if( true == bValidFound )
		{
			CleanPreselectionList( pclIndSelBVParam );
		}

		_ComputeBestFO( pclIndSelBVParam );
	}
	else if( _T("RVTYPE_CS") == strTypeID )
	{
		if( true == bValidFound )
		{
			CleanPreselectionList( pclIndSelBVParam );
		}

		_ComputeBestCS( pclIndSelBVParam );
	}
	else if( _T("RVTYPE_VV") == strTypeID )
	{
		if( true == bValidFound )
		{
			CleanPreselectionList( pclIndSelBVParam );
		}

		_ComputeBestVV( pclIndSelBVParam );
	}
	else
	{
		if( true == bValidFound )
		{
			CleanPreselectionList( pclIndSelBVParam );
		}

		_ComputeBestBV( pclIndSelBVParam );
	}

	return m_MMapKeyDecreasing.size();
}

bool CSelectList::_SelectBV( CIndSelBVParams *pclIndSelBVParam, CDB_TAProduct *pTAP, bool &bValidFound )
{
	bool bSettingCheck = true;
	CDB_RegulatingValve *pRV = dynamic_cast<CDB_RegulatingValve *>( pTAP );
	
	if( NULL == pRV )
	{
		return false;
	}

	//==========================================================================
	// Balancing valve treatment 
	// (i.e. RVTYPE_BV, RVTYPE_BVS, RVTYPE_DRV, RVTYPE_RV, RVTYPE_TBV,
	// RVTYPE_BVC, RVTYPE_TPVC, RVTYPE_MV)
	//==========================================================================

	if( 0 == IDcmp( pRV->GetIDPtr().ID, _T("RVTYPE_RV") ) || 0 == IDcmp( pRV->GetIDPtr().ID, _T("RVTYPE_RV_INSERT") ) )
	{
		bSettingCheck = false;
	}

	// Get Kv characteristic of the valve.
	CDB_ValveCharacteristic *pValveCharacteristic = pRV->GetValveCharacteristic();

	if( NULL == pValveCharacteristic )
	{
		return false;
	}

	// Compute the pressure drop for the valve fully, half and quarter opened.
	double dDpFullyOpen;
	double dDpHalfOpen;
	double dDpQuarterOpen;
	double dHMax = pValveCharacteristic->GetOpeningMax();
	
	if( -1.0 == dHMax )
	{
		return false;
	}
	
	double dRho = pclIndSelBVParam->m_WC.GetDens();
	double dNu = pclIndSelBVParam->m_WC.GetKinVisc();
	double dTemperature = pclIndSelBVParam->m_WC.GetTemp();

	if( false == pValveCharacteristic->GetValveDp( pclIndSelBVParam->m_dFlow, &dDpFullyOpen, dHMax, dRho, dNu ) )
	{
		return false;
	}

	double dHHalf = 0.5 * dHMax;

	if( false == pValveCharacteristic->GetValveDp( pclIndSelBVParam->m_dFlow, &dDpHalfOpen, dHHalf, dRho, dNu ) )
	{
		return false;
	}

	double dHQuarter = 0.25 * dHMax;
	
	if( false == pValveCharacteristic->GetValveDp( pclIndSelBVParam->m_dFlow, &dDpQuarterOpen, dHQuarter, dRho, dNu ) )
	{
		return false;
	}
	
	// Compute hand wheel setting.
	double dH = -1.0;
	double dDp = 0.0;
	bool bNotFound;
	int iNotFoundReason;
	bool bIsDpDefined = ( pclIndSelBVParam->m_dDp > 0.0 ) ? true : false;
	CDS_TechnicalParameter *pclTechParams = pclIndSelBVParam->m_pTADS->GetpTechParams();

	// If full opening of the valve is enforced...
	if( true == m_bForceFullOpening )
	{
		dH = dHMax;
		dDp = dDpFullyOpen;
		bNotFound = false;
	}
	else
	{
		// If user has specified a Dp that is bigger than Dp when valve is fully opened (it means that in closing valve we can
		// approach Dp chosen by user),
		// OR if user has not selected a Dp we have to be sure to get an opening in the range [0.25*h - h] that will give 3KPa.
		// If at 0.25*h valve has pressure drop below 3KPa, by opening pressure drop will decrease and then it's impossible to reach 3KPa.
		// If at 0.25*h valve has pressure drop above 3KPa (let's say 50KPa), if at full opening valve has pressure drop bigger than 3KPa (10KPa) 
		// it will be also impossible to reach 3KPa.
		if ( ( true == bIsDpDefined && pclIndSelBVParam->m_dDp >= dDpFullyOpen )
			|| ( false == bIsDpDefined && dDpQuarterOpen >= pclTechParams->GetValvMinDp() && dDpFullyOpen < pclTechParams->GetValvMinDp() ) )
		{
			dDp = ( true == bIsDpDefined ) ? pclIndSelBVParam->m_dDp : pclTechParams->GetValvMinDp();

			// Try to find a solution.
			bool bValid = false;
			bValid = pValveCharacteristic->GetValveOpening( pclIndSelBVParam->m_dFlow, dDp, &dH, dRho, dNu, ( eBool3::eb3True == pValveCharacteristic->IsDiscrete() ? 3 : 1 ) );

			// If it's not possible to have opening to reach required Dp...
			if( false == bValid )
			{
				// Take pressure drop available at the minimum recommenced setting of the valve (or minimal opening set in technical parameter if no
				// minimum recommenced setting is set for this valve).
				dH = pValveCharacteristic->GetMinRecSetting();

				if( dH <= 0.0 )
				{
					dH = pValveCharacteristic->GetOpeningMin();
				}

				if( -1.0 != dH )
				{
					if( false == pValveCharacteristic->GetValveDp( pclIndSelBVParam->m_dFlow, &dDp, dH, dRho, dNu ) )
					{
						return false;
					}
				}
			}
		}

		// If dh < 0, no solution was found.
		bNotFound = ( dH < 0.0 ) ? true : false;

		if( true == bNotFound )
		{
			// Try to be more specific for the reason why not found
			if( true == bIsDpDefined && pclIndSelBVParam->m_dDp < dDpFullyOpen )
			{
				iNotFoundReason = DpToReachTooLow;
				dH = dHMax;
				dDp = dDpFullyOpen;
			}
			else if( false == bIsDpDefined )
			{
				if( dDpQuarterOpen < pclTechParams->GetValvMinDp() )
				{
					iNotFoundReason = DpQOTooLow;
					dDp = pclTechParams->GetValvMinDp();
					
					bool bValid = pValveCharacteristic->GetValveOpening( pclIndSelBVParam->m_dFlow, dDp, &dH, dRho, dNu, ( eBool3::eb3True == pValveCharacteristic->IsDiscrete() ? 3 : 1 ) );
					
					if( false == bValid )
					{
						dH = pValveCharacteristic->GetMinRecSetting();

						if( dH <= 0.0 )
						{
							dH = pValveCharacteristic->GetOpeningMin();
						}

						if( -1.0 != dH )
						{
							if( false == pValveCharacteristic->GetValveDp( pclIndSelBVParam->m_dFlow, &dDp, dH, dRho, dNu ) )
							{
								return false;
							}
						}
					}
				}
				else if( dDpFullyOpen >= pclTechParams->GetValvMinDp() )
				{
					iNotFoundReason = DpFOTooHigh;
					dH = dHMax;
					dDp = dDpFullyOpen;
				}
			}
		}
		else if( eBool3::eb3True == pValveCharacteristic->IsDiscrete() )
		{
			// Recalculate the Dp for the setting as determined above.
			if( false == pValveCharacteristic->GetValveDp( pclIndSelBVParam->m_dFlow, &dDp, dH, dRho, dNu ) )
			{
				return false;
			}
		}
	}

	// Create a new entry in pre-selection list.
	// Fill the fields.
	CSelectedValve *pSelBV = new CSelectedValve();

	if( NULL == pSelBV )
	{
		ASSERTA_RETURN( false );
	}

	pSelBV->SetProductIDPtr( pRV->GetIDPtr() );
	pSelBV->SetDp( dDp );
	pSelBV->SetDpQuarterOpen( dDpQuarterOpen );
	pSelBV->SetDpHalfOpen( dDpHalfOpen );
	pSelBV->SetDpFullOpen( dDpFullyOpen );
	pSelBV->SetH( dH );

	pSelBV->SetFlag( CSelectedBase::Flags::eValveSetting, bSettingCheck && ( dH < pValveCharacteristic->GetMinRecSetting() ) );
	pSelBV->SetFlag( CSelectedBase::Flags::eValveMaxSetting, ( dH == dHMax ) ? true : false);

	// Set flag error to facilitate management with 'OnTextTipFetch' in RViewSSelBv class.
	if( true == bNotFound )
	{
		pSelBV->SetFlag( CSelectedBase::Flags::eDp, true, iNotFoundReason );
	}
	else if( dDp < pclTechParams->GetValvMinDp() )
	{
		pSelBV->SetFlag( CSelectedBase::Flags::eDp, true, DpBelowMinDp );
	}
	else if( dDp > pclTechParams->VerifyValvMaxDp( pTAP ) )
	{
		pSelBV->SetFlag( CSelectedBase::Flags::eDp, true, DpAboveMaxDp );
	}

	pSelBV->SetFlag( CSelectedBase::Flags::eValveFullODp, false == bIsDpDefined && dDpFullyOpen < pclTechParams->GetValvMinDp() );
	pSelBV->SetFlag( CSelectedBase::Flags::eValveHalfODp, false == bIsDpDefined && ( dDpHalfOpen > pclTechParams->VerifyValvMaxDp(pTAP) 
			|| dDpHalfOpen < pclTechParams->GetValvMinDp() ) );

	pSelBV->SetFlag( CSelectedBase::Flags::ePN, m_dPpn > pRV->GetPmaxmax() );
	pSelBV->SetFlag( CSelectedBase::Flags::eTemperature, dTemperature < pRV->GetTmin() || dTemperature > pRV->GetTmax() );

	pSelBV->SetFlag( CSelectedBase::eNotPriority, true );

	// Add the valve in the pre-selection list
	// Only if it's fully valid or if there is none fully valid valve already into this pre-selection list
	CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pTAP, pSelBV );
	
	bValidFound = ( true == bValidFound || false == pSelBV->IsFlagSet(CSelectedBase::Flags::eValveGlobal) );
	return true;
}

bool CSelectList::_SelectCS( CIndSelBVParams *pclIndSelBVParam, CDB_TAProduct *pTAP, bool &bValidFound )
{
	// Commissioning set treatment.

	// Cast the pointer on device.
	CDB_CommissioningSet *pCS = dynamic_cast<CDB_CommissioningSet *>( pTAP );
	ASSERT( NULL != pCS );

	if( NULL == pCS )
	{
		return false;
	}

	// Compute the signal pressure drop.
	double dRho = pclIndSelBVParam->m_WC.GetDens();
	double dNu = pclIndSelBVParam->m_WC.GetKinVisc();
	double dTemperature = pclIndSelBVParam->m_WC.GetTemp();
	double dKvSignal = pCS->GetKvSignal();
	
	if( -1.0 == dKvSignal )
	{
		return false;
	}
	
	double dSignal = CalcDp( pclIndSelBVParam->m_dFlow, dKvSignal, dRho );
		
	// Compute the orifice pressure drop.
	double dKv = pCS->GetKvFixO();
	double dDpFixO = 0.0;

	if( -1.0 != dKv )
	{
		dDpFixO = CalcDp( pclIndSelBVParam->m_dFlow, dKv, dRho );
	}

	// Get Kv characteristic of the DRV.
	CDB_ValveCharacteristic *pDevChar = pCS->GetDrvCurve();

	if( NULL == pDevChar )
	{
		return false;
	}

	// Compute the total pressure drop for the DRV fully open and the DRV half open.
	double dDpFullyOpen;
	double dDpHalfOpen;
	double dHMax = pDevChar->GetOpeningMax();
	
	if( -1.0 == dHMax )
	{
		return false;
	}
	
	if( false == pDevChar->GetValveDp( pclIndSelBVParam->m_dFlow, &dDpFullyOpen, dHMax, dRho, dNu ) )
	{
		return false;
	}

	dDpFullyOpen += dDpFixO;

	double dHHalf = 0.5 * dHMax;

	if( false == pDevChar->GetValveDp( pclIndSelBVParam->m_dFlow, &dDpHalfOpen, dHHalf, dRho, dNu ) )
	{
		return false;
	}

	dDpHalfOpen += dDpFixO;
	
	// Compute hand wheel setting.
	double dH = -1.0;
	double dDp;
	bool bNotFound;
	bool bIsDpDefined = ( pclIndSelBVParam->m_dDp > 0.0 ) ? true : false;
	
	if( true == m_bForceFullOpening )
	{
		// Full opening of the DRV is enforced.
		dH = dHMax;
		dDp = dDpFullyOpen;
		bNotFound = false;
	}
	else
	{
		// Selection process from flow and Dp if DpOK==true or from flow only if DpOK==false.
		if( true == bIsDpDefined )
		{
			// If the selected Dp is obtainable...
			if( pclIndSelBVParam->m_dDp >= dDpFullyOpen )
			{
				// Try to find a solution.
				bool bValid;
				dDp = pclIndSelBVParam->m_dDp - dDpFixO;
				bValid = pDevChar->GetValveOpening( pclIndSelBVParam->m_dFlow, dDp, &dH, dRho, dNu, 1 );

				if( false == bValid )
				{
					// The min Kv value was not small enough !
					dH = pDevChar->GetOpeningMin();

					if( -1.0 != dH )
					{
						if( false == pDevChar->GetValveDp( pclIndSelBVParam->m_dFlow, &dDp, dH, dRho, dNu ) )
						{
							return false;
						}
					}
				}

				dDp += dDpFixO;
			}
		}
		else
		{
			dH = dHMax;
			dDp = dDpFullyOpen;
		}

		// If h>=0, a solution was found
		bNotFound = ( dH < 0.0 ) ? true : false;

		if( true == bNotFound )
		{
			dH = dHMax;
			dDp = dDpFullyOpen;
		}
	}

	// Create a new entry in pre-selection list and fill the fields.
	CSelectedValve *pSelCS = new CSelectedValve();

	if( NULL == pSelCS )
	{
		ASSERTA_RETURN( false );
	}
	
	pSelCS->SetProductIDPtr( pCS->GetIDPtr() );

	pSelCS->SetDp( dDp );
	pSelCS->SetDpFullOpen( dDpFullyOpen );
	pSelCS->SetDpHalfOpen( dDpHalfOpen );
	pSelCS->SetH( dH );

	CDS_TechnicalParameter *pclTechParams = pclIndSelBVParam->m_pTADS->GetpTechParams();

	pSelCS->SetFlag( CSelectedBase::Flags::eValveSetting, false );
	pSelCS->SetFlag( CSelectedBase::Flags::eValveMaxSetting, ( dH == dHMax ) ? true : false );
	pSelCS->SetFlag( CSelectedBase::Flags::eDp, dSignal < pclTechParams->GetFoMinDp() || dSignal > pclTechParams->GetFoMaxDp() );
	pSelCS->SetFlag( CSelectedBase::Flags::eValveFullODp, bIsDpDefined && bNotFound );
	pSelCS->SetFlag( CSelectedBase::Flags::eValveHalfODp, false );	

	pSelCS->SetFlag( CSelectedBase::Flags::ePN, m_dPpn > pCS->GetPmaxmax() );
	pSelCS->SetFlag( CSelectedBase::Flags::eTemperature, dTemperature < pCS->GetTmin() || dTemperature > pCS->GetTmax() );
	pSelCS->SetFlag( CSelectedBase::Flags::eNotPriority, true );

	// Add the valve in the pre-selection list.
	// Only if it's fully valid or if there is none fully valid valve already into this pre-selection list.
	CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pTAP, pSelCS );

	bValidFound = bValidFound || !pSelCS->IsFlagSet( CSelectedBase::Flags::eValveGlobal );
	return true;
}

bool CSelectList::_SelectVV( CIndSelBVParams *pclIndSelBVParam, CDB_TAProduct *pTAP, bool &bValidFound )
{
	// Venturi balancing valve treatment.

	// Cast the pointer on device.
	CDB_VenturiValve *pVenturiValve = dynamic_cast<CDB_VenturiValve *>( pTAP );
	ASSERT( NULL != pVenturiValve );

	if( NULL == pVenturiValve )
	{
		return false;
	}
	
	// Compute the signal pressure drop.
	double dRho = pclIndSelBVParam->m_WC.GetDens();
	double dNu = pclIndSelBVParam->m_WC.GetKinVisc();
	double dTemperature = pclIndSelBVParam->m_WC.GetTemp();
	double dKvSignal = pVenturiValve->GetKvSignal();
	
	if( -1.0 == dKvSignal )
	{
		return false;
	}
	
	double dDpSignal = CalcDp( pclIndSelBVParam->m_dFlow, dKvSignal, dRho );
		
	// Get Kv characteristic of the VV.
	CDB_ValveCharacteristic *pDevChar = pVenturiValve->GetVvCurve();

	if( NULL == pDevChar )
	{
		return false;
	}

	// Compute the total pressure drop for the DRV fully open and the VV half open.
	double dDpFullyOpen;
	double dDpHalfOpen;
	double dHMax = pDevChar->GetOpeningMax();
	
	if( -1.0 == dHMax )
	{
		return false;
	}
	
	if( false == pDevChar->GetValveDp( pclIndSelBVParam->m_dFlow, &dDpFullyOpen, dHMax, dRho, dNu ) )
	{
		return false;
	}

	double dHHalf = 0.5 * dHMax;

	if( false == pDevChar->GetValveDp( pclIndSelBVParam->m_dFlow, &dDpHalfOpen, dHHalf, dRho, dNu ) )
	{
		return false;
	}
	
	// Compute hand wheel setting.
	double dH = -1.0;
	double dDp;
	bool bNotFound;
	bool bIsDpDefined = ( pclIndSelBVParam->m_dDp > 0.0 ) ? true : false;
	
	if( true == m_bForceFullOpening )
	{
		// Full opening of the VV is enforced.
		dH = dHMax;
		dDp = dDpFullyOpen;
		bNotFound = false;
	}
	else
	{
		// Selection process from flow and Dp if DpOK==true or from flow only if DpOK==false.
		if( true == bIsDpDefined )
		{
			// If the selected Dp is obtainable...
			if( pclIndSelBVParam->m_dDp >= dDpFullyOpen )
			{
				// Try to find a solution.
				dDp = pclIndSelBVParam->m_dDp;
				bool bValid = pDevChar->GetValveOpening( pclIndSelBVParam->m_dFlow, dDp, &dH, dRho, dNu, 1 );
				
				if( false == bValid )
				{
					// The min Kv value was not small enough !
					dH = pDevChar->GetOpeningMin();

					if( -1.0 != dH )
					{
						if( false == pDevChar->GetValveDp( pclIndSelBVParam->m_dFlow, &dDp, dH, dRho, dNu ) )
						{
							return false;
						}
					}
				}
			}
		}
		else
		{
			dH = dHMax;
			dDp = dDpFullyOpen;
		}

		// If h>=0, a solution was found.
		bNotFound = ( dH < 0.0 ) ? true : false;
		
		if( true == bNotFound )
		{
			dH = dHMax;
			dDp = dDpFullyOpen;
		}
	}

	// Create a new entry in pre-selection list and fill the fields.
	CSelectedValve *pSelVV = new CSelectedValve();
	
	if( NULL == pSelVV )
	{
		ASSERTA_RETURN( false );
	}
	
	pSelVV->SetProductIDPtr( pVenturiValve->GetIDPtr() );

	pSelVV->SetDp( dDp );
	pSelVV->SetDpSignal( dDpSignal );
	pSelVV->SetDpFullOpen( dDpFullyOpen );
	pSelVV->SetDpHalfOpen( dDpHalfOpen );
	pSelVV->SetH( dH );

	pSelVV->SetFlag( CSelectedBase::Flags::eValveSetting, false );
	pSelVV->SetFlag( CSelectedBase::Flags::eValveMaxSetting, ( dH == dHMax ) ? true : false );

	CDS_TechnicalParameter *pclTechParams = pclIndSelBVParam->m_pTADS->GetpTechParams();

	pSelVV->SetFlag( CSelectedBase::Flags::eDpSignal, dDpSignal < pclTechParams->GetVtriMinDp() || dDpSignal > pclTechParams->GetVtriMaxDp() );
	
	if( dDp < pclTechParams->GetVtriMinDp() )
	{
		pSelVV->SetFlag( CSelectedBase::Flags::eDp, true, DpBelowMinDp );
	}
	else if( dDp > pclTechParams->GetVtriMaxDp() )
	{
		pSelVV->SetFlag( CSelectedBase::Flags::eDp, true, DpAboveMaxDp );
	}

	pSelVV->SetFlag( CSelectedBase::Flags::eValveHalfODp, false );
	pSelVV->SetFlag( CSelectedBase::Flags::eValveFullODp, bIsDpDefined && bNotFound );
	pSelVV->SetFlag( CSelectedBase::Flags::ePN, m_dPpn > pVenturiValve->GetPmaxmax() );
	pSelVV->SetFlag( CSelectedBase::Flags::eTemperature, dTemperature < pVenturiValve->GetTmin() || dTemperature > pVenturiValve->GetTmax() );

	pSelVV->SetFlag( CSelectedBase::Flags::eNotPriority, true );

	// Add the valve in the pre-selection list.
	// Only if it's fully valid or if there is none fully valid valve already into this pre-selection list.
	CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pTAP, pSelVV );

	bValidFound = bValidFound || !pSelVV->IsFlagSet( CSelectedBase::Flags::eValveGlobal );
	return true;
}

bool CSelectList::_SelectFO( CIndSelBVParams *pclIndSelBVParam, CDB_TAProduct *pTAP, bool &bValidFound )
{
	//========================
	// Fixed orifice treatment
	//========================
	
	// Cast the pointer on device
	CDB_FixedOrifice *pFixedOrifice = dynamic_cast<CDB_FixedOrifice *>( pTAP );
	ASSERT( NULL != pFixedOrifice );

	if( NULL == pFixedOrifice )
	{
		return false;
	}
	
	// Compute the signal pressure drop.
	double dRho = pclIndSelBVParam->m_WC.GetDens();
	double dNu = pclIndSelBVParam->m_WC.GetKinVisc();
	double dTemperature = pclIndSelBVParam->m_WC.GetTemp();
	double dKvSignal = pFixedOrifice->GetKvSignal();
	
	if( -1.0 == dKvSignal )
	{
		return false;
	}
	
	double dDpSignal = CalcDp( pclIndSelBVParam->m_dFlow, dKvSignal, dRho );
		
	// Compute the orifice pressure drop.
	double dKv = pFixedOrifice->GetKv();
	double dDp;
	dDp = CalcDp( pclIndSelBVParam->m_dFlow, dKv, dRho );

	// Create a new entry in pre-selection list and fill the fields.
	CSelectedValve *pSelFO = new CSelectedValve();

	if( NULL == pSelFO )
	{
		ASSERTA_RETURN( false );
	}
	
	pSelFO->SetProductIDPtr( pFixedOrifice->GetIDPtr() );
	
	pSelFO->SetDp( dDp );
	pSelFO->SetDpSignal( dDpSignal );

	pSelFO->SetFlag( CSelectedBase::Flags::eValveSetting, false );
	pSelFO->SetFlag( CSelectedBase::Flags::eValveMaxSetting, false );

	CDS_TechnicalParameter *pclTechParams = pclIndSelBVParam->m_pTADS->GetpTechParams();
	pSelFO->SetFlag( CSelectedBase::Flags::eDpSignal, dDpSignal < pclTechParams->GetFoMinDp() || dDpSignal > pclTechParams->GetFoMaxDp() );
	
	if( dDp < pclTechParams->GetValvMinDp() )
	{
		pSelFO->SetFlag( CSelectedBase::Flags::eDp, true, DpBelowMinDp );
	}
	else if( dDp > pclTechParams->GetValvMaxDp() )
	{
		pSelFO->SetFlag( CSelectedBase::Flags::eDp, true, DpAboveMaxDp );
	}
	
	pSelFO->SetFlag( CSelectedBase::Flags::eValveFullODp, false );
	pSelFO->SetFlag( CSelectedBase::Flags::eValveHalfODp, false );

	pSelFO->SetFlag( CSelectedBase::Flags::ePN, m_dPpn > pFixedOrifice->GetPmaxmax() );
	pSelFO->SetFlag( CSelectedBase::Flags::eTemperature, dTemperature < pFixedOrifice->GetTmin() || dTemperature > pFixedOrifice->GetTmax() );
	pSelFO->SetFlag( CSelectedBase::Flags::eNotPriority, true );

	// Add the valve in the pre-selection list.
	// Only if it's fully valid or if there is none fully valid valve already into this pre-selection list.
	CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pTAP, pSelFO );
	
	bValidFound = bValidFound || !pSelFO->IsFlagSet( CSelectedBase::Flags::eValveGlobal );
	return true;
}

bool CSelectList::_SelectTrv( CIndSelBVParams *pclIndSelBVParam, CDB_TAProduct *pTAP, bool &bValidFound )
{
	CDB_ThermostaticValve *pTRV = dynamic_cast<CDB_ThermostaticValve *>( pTAP );
	ASSERT( NULL != pTRV );

	if( NULL == pTRV )
	{
		return false;
	}

	// Get Kv characteristic of the valve.
	CDB_ThermoCharacteristic *pThermoCharacteristic = pTRV->GetThermoCharacteristic();

	if( NULL == pThermoCharacteristic )
	{
		return false;
	}

	// Compute the pressure drop for the valve fully open and the valve half open.
	double dDpFullyOpen;
	double dHmax = pThermoCharacteristic->GetOpeningMax();
	
	if( -1.0 == dHmax )
	{
		return false;
	}
	
	// If thermostatic head is used, we take the Kv characteristic at 2DT Kelvin, otherwise we take KvS (iDeltaT = 0).
	int iDeltaT = ( true == m_bUseWithThrmHead ) ? 2 : 0;

	double dRho = pclIndSelBVParam->m_WC.GetDens();
	double dNu = pclIndSelBVParam->m_WC.GetKinVisc();
	double dTemperature = pclIndSelBVParam->m_WC.GetTemp();
	CDS_TechnicalParameter *pclTechParams = pclIndSelBVParam->m_pTADS->GetpTechParams();

	if( false == pThermoCharacteristic->GetValveDp( pclIndSelBVParam->m_dFlow, &dDpFullyOpen, dHmax, dRho, dNu, iDeltaT ) )
	{
		return false;
	}
	
	// Compute hand wheel setting.
	double dH = -1.0;
	double dDp = -1.0;
	bool bNotFound;
	bool bIsDpDefined = ( pclIndSelBVParam->m_dDp > 0.0 ) ? true : false;
	CString strProdTypeID = pTRV->GetTypeIDPtr().ID;

	// If full opening of the valve is enforced or valve is non-presettable...
	if( true == m_bForceFullOpening || _T("TRVTYPE_NOPRSET") == strProdTypeID || _T("TRVTYPE_INSERT_NOPRSET") == strProdTypeID )
	{
		dH = dHmax;
		dDp = dDpFullyOpen;
		bNotFound = ( bIsDpDefined && dDp > m_dSelDpTot ) ? true : false;
	}
	else 
	{
		// Selection process from flow and Dp if 'fDpOK' is set to 'true' or from flow only if fDpOK is set to 'false'.
		
		// If the Dp is specified or the Dp is not specified...
		if( ( true == bIsDpDefined && pclIndSelBVParam->m_dDp >= dDpFullyOpen) || false == bIsDpDefined )
		{
			if( true == bIsDpDefined )
			{
				dDp = pclIndSelBVParam->m_dDp;
			}
			else
			{
				dDp = pclTechParams->GetTrvMinDpRatio() * pclTechParams->GetTrvDefDpTot();
			}
			
			// Try to find a solution.
			bool bValid = false;
			
			// Remark: If valve opening is discrete, we use rounding case 4 (see 'CDB_ThermoCharacteristic::GetValveOpening()' comments in DataObj.h) otherwise
			//         we not apply rounding ( case 1).
			bValid = pThermoCharacteristic->GetValveOpening( pclIndSelBVParam->m_dFlow, dDp, &dH, dRho, dNu, ( eBool3::eb3True == pThermoCharacteristic->IsDiscrete() ) ? 4 : 1, iDeltaT );

			// If the min Kv value was not small enough...
			if( false == bValid )
			{
				dH = pThermoCharacteristic->GetMinRecSetting();

				if( dH <= 0.0 )
				{
					dH = pThermoCharacteristic->GetOpeningMin();
				}

				if( -1.0 != dH )
				{
					if( false == pThermoCharacteristic->GetValveDp( pclIndSelBVParam->m_dFlow, &dDp, dH, dRho, dNu, iDeltaT ) )
					{
						return false;
					}
				}
			}
		}

		// If dh<0, no solution was found...
		bNotFound = ( dH < 0.0 ) ? true : false;
		
		if( true == bNotFound )
		{
			dH = dHmax;
			dDp = dDpFullyOpen;
		}
		else if( eBool3::eb3True == pThermoCharacteristic->IsDiscrete() )
		{
			// Recalculate the Dp for the setting as determined above.
			if( false == pThermoCharacteristic->GetValveDp( pclIndSelBVParam->m_dFlow, &dDp, dH, dRho, dNu, iDeltaT ) )
			{
				return false;
			}
		}
	}

	// If the Dp is different from the other Trv's...
	if( *m_pdMaxDpTrv != -DBL_MAX )
	{
		if( *m_pdMaxDpTrv != dDp )
		{
			*m_pbDiffDpTrv = true;
			*m_pdMaxDpTrv = max( *m_pdMaxDpTrv, dDp );
		}
	}
	else
	{
		*m_pdMaxDpTrv = dDp;
	}

	// Create a new entry in pre-selection list and fill the fields.
	CSelectedValve *pSelTRV = new CSelectedValve();

	if( NULL == pSelTRV )
	{
		ASSERTA_RETURN( false );
	}

	pSelTRV->SetProductIDPtr( pTRV->GetIDPtr() );
	
	pSelTRV->SetFlag( CSelectedBase::Flags::eValveSetting, dH <= 0.0 ? true : false );
	pSelTRV->SetFlag( CSelectedBase::Flags::eValveMaxSetting, dH == dHmax ? true : false);

	pSelTRV->SetFlag( CSelectedBase::Flags::eDp, ( !m_bForceFullOpening && bIsDpDefined && ( bNotFound || dDp < ( pclTechParams->GetTrvMinDpRatio() * m_dSelDpTot ) ) ) ? 1 : 0 );	

	pSelTRV->SetFlag( CSelectedBase::Flags::eValveFullODp, !bIsDpDefined && dDpFullyOpen > m_dSelDpTot );	

	pSelTRV->SetFlag( CSelectedBase::Flags::eValveHalfODp, false );	

	pSelTRV->SetFlag( CSelectedBase::Flags::ePN, m_dPpn > pTRV->GetPmaxmax() );	
	pSelTRV->SetFlag( CSelectedBase::Flags::eTemperature, dTemperature < pTRV->GetTmin() || dTemperature > pTRV->GetTmax() );
	pSelTRV->SetFlag( CSelectedBase::Flags::eNotPriority, true );

	// Add the valve in the pre-selection list.
	// Only if it's fully valid or if there is none fully valid valve already into this pre-selection list!
	CTAPSortKey sKey( pTAP );
	double dKey = sKey.GetSortingKeyCustomAsDouble( &CTAPSortKey::CTAPSKMask( CTAPSortKey::TAPSKM_All ), CTAPSortKey::TAPKO_Type, CTAPSortKey::TAPKO_Size, CTAPSortKey::TAPKO_Fam, CTAPSortKey::TAPKO_Vers, -1 );
	m_MMapKeyDecreasing.insert( ProductPair( dKey, pSelTRV ) );

	bValidFound = bValidFound || !pSelTRV->IsFlagSet( CSelectedBase::Flags::eValveGlobal );
	return true;
}

// Scan all pre-selected valve and compute the best setting
void CSelectList::_ComputeBestBV( CIndSelBVParams *pclIndSelBVParam )
{
	CDS_TechnicalParameter *pclTechParams = pclIndSelBVParam->m_pTADS->GetpTechParams();
	bool bIsDpDefined = ( pclIndSelBVParam->m_dDp > 0.0 ) ? true : false;

	for( int iPhase = 0; iPhase < 3; iPhase++ )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedBase *pclSelectedProduct = m_cIter->second;

			if( NULL == pclSelectedProduct )
			{
				ASSERT( 0 );
				continue;
			}
			
			CSelectedValve *pclSelectedBalancingValve = dynamic_cast<CSelectedValve *>( pclSelectedProduct );

			if( NULL == pclSelectedBalancingValve )
			{
				ASSERT( 0 );
				continue;
			}
			
			CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct*>( pclSelectedBalancingValve->GetpData() );

			if( NULL == pTAP )
			{
				ASSERT( 0 );
				continue;
			}
			
			int iSizeKey = pTAP->GetSizeKey();

			switch (iPhase)
			{
				case 0:

					// Store highest priority for each product size.
					if( pTAP->GetPriorityLevel() < m_mapPriority[iSizeKey] )
					{
						m_mapPriority[iSizeKey] = pTAP->GetPriorityLevel();
					}
					break;

				case 1:
					
					// Ordering Key.
					if( m_mapPriority[iSizeKey] == pTAP->GetPriorityLevel() ) // Best product is only into high priority product
					{
						pclSelectedBalancingValve->SetFlag( CSelectedBase::eNotPriority, false );
						double dDeltaH = DBL_MAX;

						// Dp is unknown.
						if( false == bIsDpDefined )
						{
							if( pclSelectedBalancingValve->GetDpFullOpen() < pclTechParams->GetValvMinDp() )
							{
								m_rBest.dBestDpBelowMin = max( m_rBest.dBestDpBelowMin , pclTechParams->GetValvMinDp() );
							}

							if( pclSelectedBalancingValve->GetDpFullOpen() <= pclTechParams->GetValvMin2Dp() )
							{
								m_rBest.dBestDp = max( m_rBest.dBestDp , pclSelectedBalancingValve->GetDpFullOpen() );
							}
							else
							{
								m_rBest.dBestDpAboveMin2 = min( m_rBest.dBestDpAboveMin2 , pclSelectedBalancingValve->GetDpFullOpen() );
							}
						}
						else // Dp known - Best is closest to 75% of opening ...
						{
							if( m_mapPriority.count( iSizeKey ) > 0 )
							{
								CDB_ValveCharacteristic *pValveCharacteristic = (CDB_ValveCharacteristic *)pTAP->GetValveCharDataPointer();
								
								if( NULL != pValveCharacteristic )
								{
									double dhmax = pValveCharacteristic->GetOpeningMax();
									
									// We are looking for a DpC with a MV located on the secondary.
									// Dp has been specified due to the fact that the stabilized Dp doesn't reach the DpL min.
									// In this case we try to find the smallest valve.
									double dH = pclSelectedBalancingValve->GetH();

									if( m_eSelMvLoc == eMvLoc::MvLocSecondary )
									{
										dDeltaH = fabs( dH - ( dhmax ) );
									}
									else
									{
										dDeltaH = fabs( dH - ( 0.75 * dhmax ) );
									}

									pclSelectedBalancingValve->SetBestDelta( dDeltaH );

									// Valve are ordered from biggest to smallest
									if( dDeltaH >= 0 && pclSelectedBalancingValve->GetDp() == pclIndSelBVParam->m_dDp )
									{
										if( dDeltaH < m_rBest.dBestDelta )
										{
											m_rBest.dBestDelta = dDeltaH;
											m_rBest.pSelV = pclSelectedBalancingValve;
										}
										else if( m_rBest.dBestDelta != DBL_MAX  )
										{
											// Proposed product is smaller than best selected product
											// Specific treatment for STAF65-2 vs STAF80 and for settings included between 2.6 && 4.8.
											// iSizeKey ==  8	-->  STAF 65-2
											if ( iSizeKey ==  8 && iSizeKey < ( (CDB_TAProduct *)m_rBest.pSelV->GetpData())->GetSizeKey() )
											{
												if( dH > 2.6 && dH < 4.8 )
												{
													m_rBest.dBestDelta = dDeltaH;
													m_rBest.pSelV = pclSelectedBalancingValve;
												}
											}
										}
									}
								}
							}
						}
					}
					break;

				case 2:
					// Set flag best Dp.
					if( m_mapPriority[iSizeKey] == pTAP->GetPriorityLevel() ) // Best product is only into high priority product
					{
						if( false == bIsDpDefined )
						{
							// When dBestDp is not initialized, we have to use dBestDpBelowMin or dBestDpAboveMin2
							if( m_rBest.dBestDp == 0.0 )
							{
								if( m_rBest.dBestDpBelowMin != 0.0 )
								{
									m_rBest.dBestDp = m_rBest.dBestDpBelowMin;
								}
								else if( m_rBest.dBestDpAboveMin2 != DBL_MAX )
								{
									m_rBest.dBestDp = m_rBest.dBestDpAboveMin2;
								}
							}

							if( pclSelectedBalancingValve->GetDpFullOpen() == m_rBest.dBestDp )
							{
								pclSelectedBalancingValve->SetFlag( CSelectedBase::eBest, true );
							}
						}
						else
						{
							// Best valve is close to 75% of opening.
							if( pclSelectedBalancingValve->GetBestDelta() == m_rBest.dBestDelta )
							{
								pclSelectedBalancingValve->SetFlag( CSelectedBase::eBest, true );
							}
						}
					}

					// Transfer valve into the final list with good priority key
					CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pTAP, pclSelectedBalancingValve );

					break;
			}
		}
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();
}

void CSelectList::_ComputeBestCS( CIndSelBVParams *pclIndSelBVParam )
{
	CDS_TechnicalParameter *pclTechParams = pclIndSelBVParam->m_pTADS->GetpTechParams();
	bool bIsDpDefined = ( pclIndSelBVParam->m_dDp > 0.0 ) ? true : false;

	for( int iPhase = 0; iPhase < 3; iPhase++ )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedBase *pclSelectedProduct = m_cIter->second;

			if( NULL == pclSelectedProduct )
			{
				ASSERT( 0 );
				continue;
			}
			
			CSelectedValve *pclSelectedCommissioningSet = dynamic_cast<CSelectedValve *>( pclSelectedProduct );
			
			if (NULL == pclSelectedCommissioningSet)
			{
				ASSERT( 0 );
				continue;
			}

			CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct*>( pclSelectedCommissioningSet->GetpData() );
			
			if( NULL == pTAP )
			{
				ASSERT( 0 );
				continue;
			}
			
			int iSizeKey = pTAP->GetSizeKey();
			
			switch( iPhase )
			{
				case 0:

					// Store highest priority for each product size.
					if( pTAP->GetPriorityLevel() < m_mapPriority[iSizeKey] )
					{
						m_mapPriority[iSizeKey] = pTAP->GetPriorityLevel();
					}

					break;

				case 1:
					
					// Ordering Key
					if( m_mapPriority[iSizeKey] == pTAP->GetPriorityLevel() ) // Best product is only into high priority product
					{
						pclSelectedCommissioningSet->SetFlag( CSelectedBase::eNotPriority, false);
						double dDelta = DBL_MAX;
						
						// Root mean square, because the ideal Dp is proportional to the square of the arithmetic average of flow range.
						double dTargetSignal = pow( ( sqrt( pclTechParams->GetFoMinDp() ) + sqrt( pclTechParams->GetFoMaxDp() ) ), 2 ) / 4.0;  
						
						// Dp is unknown.
						if( false == bIsDpDefined )
						{
							dDelta = fabs( pclSelectedCommissioningSet->GetDpSignal() - dTargetSignal );

							if( dDelta < m_rBest.dBestDelta )
							{
								m_rBest.dBestDelta = dDelta;
								m_rBest.dBestDp = pclSelectedCommissioningSet->GetDpSignal();
							}
						}
						else
						{
							// Dp known - Best is closest to 75% of opening ...
							if( m_mapPriority.count( iSizeKey ) > 0 )
							{
								CDB_ValveCharacteristic *pValveCharacteristic = (CDB_ValveCharacteristic *)pTAP->GetValveCharDataPointer();

								if( NULL != pValveCharacteristic )
								{
									double dhmax = pValveCharacteristic->GetOpeningMax();
									dDelta = fabs( pclSelectedCommissioningSet->GetH() - ( 0.75 * dhmax ) );
									pclSelectedCommissioningSet->SetBestDelta( dDelta );
									
									if( dDelta >= 0.0 && dDelta < m_rBest.dBestDelta && pclSelectedCommissioningSet->GetDp() == pclIndSelBVParam->m_dDp )
									{
										m_rBest.dBestDelta = dDelta;
									}
								}
							}
						}
					}

					break;

				case 2:

					// Set flag best Dp.
					if( m_mapPriority[iSizeKey] == pTAP->GetPriorityLevel() ) // Best product is only into high priority product
					{
						if( false == bIsDpDefined )
						{
							// When dBestDp is not initialized, we have to use dBestDpBelowMin or dBestDpAboveMin2
							if( 0.0 == m_rBest.dBestDp )
							{
								if( m_rBest.dBestDpBelowMin != 0.0 )
								{
									m_rBest.dBestDp = m_rBest.dBestDpBelowMin;
								}
								else if( m_rBest.dBestDpAboveMin2 != DBL_MAX )
								{
									m_rBest.dBestDp = m_rBest.dBestDpAboveMin2;
								}
							}

							if( pclSelectedCommissioningSet->GetDp() == m_rBest.dBestDp )
							{
								pclSelectedCommissioningSet->SetFlag( CSelectedBase::eBest, true );
							}
						}
						else
						{
							// Best valve is close to 75% of opening.
							if( pclSelectedCommissioningSet->GetBestDelta() == m_rBest.dBestDelta )
							{
								pclSelectedCommissioningSet->SetFlag( CSelectedBase::eBest, true );
							}
						}
					}

					// Transfer valve into the final list with good priority key
					CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pTAP, pclSelectedCommissioningSet );

					break;
			}
		}
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();
}

void CSelectList::_ComputeBestVV( CIndSelBVParams *pclIndSelBVParam )
{
	CDS_TechnicalParameter *pclTechParams = pclIndSelBVParam->m_pTADS->GetpTechParams();
	bool bIsDpDefined = ( pclIndSelBVParam->m_dDp > 0.0 ) ? true : false;

	// It can happen that we have no valve with the dp required. In this case we will take the worse one.
	double dBestWorse = DBL_MAX;

	for( int iPhase = 0; iPhase < 3; iPhase++ )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedBase *pclSelectedProduct = m_cIter->second;
			
			if( NULL == pclSelectedProduct )
			{
				ASSERT( 0 );
				continue;
			}
			
			CSelectedValve *pclSelectedVenturiValve = dynamic_cast<CSelectedValve *>( pclSelectedProduct );
			
			if( NULL == pclSelectedVenturiValve )
			{
				ASSERT( 0 );
				continue;
			}
			
			CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct*>( pclSelectedVenturiValve->GetpData() );
			
			if( NULL == pTAP )
			{
				ASSERT( 0 );
				continue;
			}
			
			int iSizeKey = pTAP->GetSizeKey();

			switch( iPhase )
			{
				case 0:	
					
					// Store highest priority for each product size..
					if( pTAP->GetPriorityLevel() < m_mapPriority[iSizeKey] )
					{
						m_mapPriority[iSizeKey] = pTAP->GetPriorityLevel();
					}
					
					break;

				case 1:

					// Ordering key.
					if( m_mapPriority[iSizeKey] == pTAP->GetPriorityLevel() ) // Best product is only into high priority product
					{
						pclSelectedVenturiValve->SetFlag( CSelectedBase::eNotPriority, false );
						double dDelta = DBL_MAX;
						
						// From Jean-Christophe Carette on Slack (26th August 2020):
						// Target Dp needs to be seen as Dp corresponding to ideal KvSignal (for any given design flow).		
						// Ideal KvSignal is the one allowing to cover best flow range above and below the given design flow		
						// while staying between VTRI_MIN_DP and VTRI_MAX_DP.		
		
						// Target Dp is thus taken as the square of the arithmetic average of the upper and lower flows		
						// corresponding to VTRI_MAX_DP and VTRI_MIN_DP
						double dTargetSignal = pow( ( sqrt( pclTechParams->GetVtriMinDp() ) + sqrt( pclTechParams->GetVtriMaxDp() ) ), 2 ) / 4.0;
						
						// Dp is unknown.
						if( false == bIsDpDefined )
						{
							dDelta = fabs( pclSelectedVenturiValve->GetDpSignal() - dTargetSignal );
							
							// HYS-1436: We add also the condition that to be the best the dp signal must be greater than the venturi minimum dp (1 kPa) set in the localdb.txt.
							if( dDelta < m_rBest.dBestDelta && pclSelectedVenturiValve->GetDpSignal() > pclTechParams->GetVtriMinDp() )
							{
								m_rBest.dBestDelta = dDelta;
								m_rBest.dBestDp = pclSelectedVenturiValve->GetDpSignal();
							}
						}
						else // Dp known - Best is closest to 75% of opening ...
						{
							if( m_mapPriority.count( iSizeKey ) > 0 )
							{
								CDB_ValveCharacteristic *pValveCharacteristic = (CDB_ValveCharacteristic *)pTAP->GetValveCharDataPointer();
								
								if( NULL != pValveCharacteristic )
								{
									dDelta = fabs( pclSelectedVenturiValve->GetH() - ( 0.75 * pValveCharacteristic->GetOpeningMax() ) );
									pclSelectedVenturiValve->SetBestDelta( dDelta );

									if( dDelta < m_rBest.dBestDelta )
									{
										if( pclSelectedVenturiValve->GetDp() == pclIndSelBVParam->m_dDp )
										{
											// If Dp required is reached, we take this valve as the best...
											m_rBest.dBestDelta = dDelta;
										}
										else
										{
											// Dp required not reached, we take this one in case of there is no other valve that can
											// reach the dp. We will take the one that has its Dp at full opening the closest to the dp required.
											if( fabs( pclSelectedVenturiValve->GetDp() - pclIndSelBVParam->m_dDp ) < dBestWorse )
											{
												dBestWorse = fabs( pclSelectedVenturiValve->GetDp() - pclIndSelBVParam->m_dDp );
												pclSelectedVenturiValve->SetBestDelta( dBestWorse );
											}
										}
									}
								}
							}
						}
					}

					break;

				case 2:

					// Set flag best Dp.
					if( m_mapPriority[iSizeKey] == pTAP->GetPriorityLevel() ) // Best product is only into high priority product
					{
						if( false == bIsDpDefined )
						{
							// When dBestDp is not initialized, we have to use dBestDpBelowMin or dBestDpAboveMin2
							if( 0.0 == m_rBest.dBestDp )
							{
								if( m_rBest.dBestDpBelowMin != 0.0 )
								{
									m_rBest.dBestDp = m_rBest.dBestDpBelowMin;
								}
								else if( m_rBest.dBestDpAboveMin2 != DBL_MAX )
								{
									m_rBest.dBestDp = m_rBest.dBestDpAboveMin2;
								}
							}

							if( pclSelectedVenturiValve->GetDpSignal() == m_rBest.dBestDp )
							{
								pclSelectedVenturiValve->SetFlag( CSelectedBase::eBest, true );
							}
						}
						else
						{
							if( m_rBest.dBestDelta != DBL_MAX )
							{
								// A perfect match exist!
								if( pclSelectedVenturiValve->GetBestDelta() == m_rBest.dBestDelta )
								{
									pclSelectedVenturiValve->SetFlag( CSelectedBase::eBest, true );
								}
							}
							else
							{
								// We take the worse case.
								if( pclSelectedVenturiValve->GetBestDelta() == dBestWorse )
								{
									pclSelectedVenturiValve->SetFlag( CSelectedBase::eBest, true );
								}
							}
						}
					}

					// Transfer valve into the final list with good priority key.
					CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pTAP, pclSelectedVenturiValve );

					break;
			}
		}
	}
	
	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();
}

void CSelectList::_ComputeBestFO( CIndSelBVParams *pclIndSelBVParam )
{
	CDS_TechnicalParameter *pclTechParams = pclIndSelBVParam->m_pTADS->GetpTechParams();
	bool bIsDpDefined = ( pclIndSelBVParam->m_dDp > 0.0 ) ? true : false;

	for( int iPhase = 0; iPhase < 3; iPhase++ )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedBase *pclSelectedProduct = m_cIter->second;

			if( NULL == pclSelectedProduct )
			{
				ASSERT( 0 );
				continue;
			}

			CSelectedValve *pclSelectedFixedOrifice = dynamic_cast<CSelectedValve *>( pclSelectedProduct );

			if( NULL == pclSelectedFixedOrifice )
			{
				ASSERT( 0 );
				continue;
			}
			
			CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct*>(pclSelectedFixedOrifice->GetpData());

			if( NULL == pTAP )
			{
				ASSERT( 0 );
				continue;
			}
			
			int iSizeKey = pTAP->GetSizeKey();
			
			switch( iPhase )
			{
				case 0:
					
					// Store highest priority for each product size..
					if( pTAP->GetPriorityLevel() < m_mapPriority[iSizeKey] )
					{
						m_mapPriority[iSizeKey] = pTAP->GetPriorityLevel();
					}
					
					break;

				case 1:

					// Ordering Key.
					if( m_mapPriority[iSizeKey] == pTAP->GetPriorityLevel() ) // Best product is only into high priority product
					{
						pclSelectedFixedOrifice->SetFlag( CSelectedBase::eNotPriority, false );
						double dDelta = DBL_MAX;
					
						// From Jean-Christophe Carette on Slack (26th August 2020):
						// Target Dp needs to be seen as Dp corresponding to ideal KvSignal (for any given design flow).		
						// Ideal KvSignal is the one allowing to cover best flow range above and below the given design flow		
						// while staying between VTRI_MIN_DP and VTRI_MAX_DP.		
		
						// Target Dp is thus taken as the square of the arithmetic average of the upper and lower flows		
						// corresponding to VTRI_MAX_DP and VTRI_MIN_DP
						double dTargetDpSignal = pow( ( sqrt( pclTechParams->GetFoMinDp() ) + sqrt( pclTechParams->GetFoMaxDp() ) ), 2 ) / 4.0;  
						dDelta = fabs( pclSelectedFixedOrifice->GetDpSignal() - dTargetDpSignal );
						pclSelectedFixedOrifice->SetBestDelta( dDelta );

						if( dDelta < m_rBest.dBestDelta )
						{
							m_rBest.dBestDelta = dDelta;
							m_rBest.dBestDp = pclSelectedFixedOrifice->GetDpSignal();
						}
					}

					break;

				case 2:

					// Set flag best Dp.
					if( m_mapPriority[iSizeKey] == pTAP->GetPriorityLevel() ) // Best product is only into high priority product
					{
						if( pclSelectedFixedOrifice->GetBestDelta() == m_rBest.dBestDelta )
						{
							pclSelectedFixedOrifice->SetFlag( CSelectedBase::eBest, true );
						}
					}

					// Transfer valve into the final list with good priority key
					CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pTAP, pclSelectedFixedOrifice );
					break;
			}
		}
	}

	// Clean allocated memory
	m_PreselMMapKeyDecreasing.clear();
}

////////////////////////////////////////////////////////////////
//	CSelectCVList
////////////////////////////////////////////////////////////////
CSelectCVList::CSelectCVList() : CSelectCtrlList()
{
	m_dMinDpMinCv = DBL_MAX;
}

int CSelectCVList::SelectQ( CIndSelCVParams *pclIndSelCVParams, bool *pbSizeShiftProblem )
{
	return _Select( pclIndSelCVParams, pbSizeShiftProblem );
}

int CSelectCVList::SelectCvKvs( CIndSelCVParams *pclIndSelCVParams, bool *pbSizeShiftProblem )
{
	pclIndSelCVParams->m_dDp = 0.0;
	return _Select( pclIndSelCVParams, pbSizeShiftProblem );
}

int CSelectCVList::SelectCvDp( CIndSelCVParams *pclIndSelCVParams, bool *pbSizeShiftProblem )
{
	pclIndSelCVParams->m_dKvs = 0.0;
	return _Select( pclIndSelCVParams, pbSizeShiftProblem );
}

int CSelectCVList::_Select( CIndSelCVParams *pclIndSelCVParams, bool *pbSizeShiftProblem )
{
	if( NULL == pclIndSelCVParams || NULL == pclIndSelCVParams->m_pTADS || NULL == pclIndSelCVParams->m_pTADS->GetpTechParams() 
		|| NULL == pclIndSelCVParams->m_pTADB || NULL == pbSizeShiftProblem )
	{
		ASSERTA_RETURN( 0 );
	}

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return 0;
	}

	_string str;
	LPARAM lpParam;
	double dKey = 0;

	CDS_TechnicalParameter *pclTechParams = pclIndSelCVParams->m_pTADS->GetpTechParams();
	double dMaxCvDp = pclTechParams->GetCVMaxDispDp();
	double dMinCvDp = pclTechParams->GetCVMinDpOnoff();

	pclIndSelCVParams->m_CtrlList.GetFirst( str, lpParam );
	
	CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( (CData *)lpParam );
	ASSERT( NULL != pCV );

	if( CDB_ControlProperties::eCvProportional == pCV->GetCtrlProp()->GetCtrlType() )
	{
		dMinCvDp = pclTechParams->GetCVMinDpProp();
	}

	if( false == m_clSelectPipeList.GetPipeSizeBound() )
	{
		return 0;
	}

	CDB_Pipe *pSmallestPipe = m_clSelectPipeList.GetpSmallestPipe();

	if( NULL == pSmallestPipe )
	{
		return 0;
	}

	CDB_Pipe *pBiggestPipe = m_clSelectPipeList.GetpLargestPipe();

	if( NULL == pBiggestPipe )
	{
		return 0;
	}
	
	// Take into account SizeShiftAbove and SizeShiftBelow
	// Setup variables to possibly enforce combined SizeShifts
	// In case of Hub Selection don't use SizeShift from TechParam
	int iSizeShiftUp;
	int iSizeShiftDown;

	if( 99 == pclTechParams->GetSizeShiftAbove() ) // Combined SizeShift +3/-3 enforced
	{
		iSizeShiftUp = 3;
		iSizeShiftDown = -3;
	}
	else
	{
		iSizeShiftUp = pclTechParams->GetSizeShiftAbove();
		iSizeShiftDown = pclTechParams->GetSizeShiftBelow();
	}

	// If user has input a Kvs or Dp value, we must search control valves that are the best solution with this value.
	bool bThinSelection = ( 0.0 != pclIndSelCVParams->m_dKvs || 0.0 != pclIndSelCVParams->m_dDp ) ? true : false;
	
	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclIndSelCVParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN = pclTableDN->GetBiggestDN();
	int iHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelCVParams->m_pTADB ) + iSizeShiftUp );
	int iLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelCVParams->m_pTADB ) + iSizeShiftDown );

	// HYS-699: Now we check also with the extreme limits to avoid to get the message that we don't found because
	// a size shift problem when there is no possibility even with size above/below set to 4 (max.).
	int iMaxHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelCVParams->m_pTADB ) + pclTechParams->GetMaxSizeShift() );
	int iMaxLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelCVParams->m_pTADB ) - pclTechParams->GetMaxSizeShift() );
	int iDN15 = pclTableDN->GetSize( _T("DN_15") );
	
	if( iHighestSize < iDN15 )
	{
		iHighestSize = iDN15;
	}

	if( iMaxHighestSize < iDN15 )
	{
		iMaxHighestSize = iDN15;
	}

	double dKvs;
	double dDp;
	double dCvDporKvsValue;
	CArray<CSelectCvTypeLimits> ar;
	bool bNewTypeMustBeEntered;
	
	// Explication of the following code.
	// ----------------------------------
	// Run each valve, and for each type delimit the upper and lower boundaries 
	// for the Dp or the Kvs. Each time a valve is closer to the target value, correct 
	// the boundaries. So at the end, each type of valve will have as thin as possible 
	// the boundaries for an info (Dp or Kvs). After that we go to all the valves 
	// and add only ones that match the limits.
	// -------------------------------------------------------------------------------

	// Create a list with one control valve from each type (same connection, version etc)
	// and put braces between the control valve will be seen

	// Verify the list found at least one valve between the [Dp;Dp+50%] and one between [Dp-50%;Dp]
	// The same is true for Kvs. In that case show only valves in that range [Dp-50%;Dp+50%]
	double dUpperLimitKvsOrDp = 0.0;
	double dLowerLimitKvsOrDp = 0.0;
	bool bFindInUpperLimits = false;
	bool bFindInLowerLimits = false;
	const double DpKvsDeviation = 0.5; // Maximum Dp or Kvs deviation 

	// For thin selection extract categories
	if( true == bThinSelection )
	{	
		if( pclIndSelCVParams->m_dDp > 0.0 )
		{
			// Search in the range [ Dp - 50%; Dp + 50% ]
			dUpperLimitKvsOrDp = ( 1 + DpKvsDeviation) * pclIndSelCVParams->m_dDp;
			dLowerLimitKvsOrDp = DpKvsDeviation * pclIndSelCVParams->m_dDp;
		}
		else if( pclIndSelCVParams->m_dKvs > 0.0 )
		{
			// Search in the range [ Kvs - 50%; Kvs + 50% ]
			dUpperLimitKvsOrDp = (1 + DpKvsDeviation) * pclIndSelCVParams->m_dKvs;
			dLowerLimitKvsOrDp = DpKvsDeviation * pclIndSelCVParams->m_dKvs;
		}

		// Run all selected control valve.
		for( BOOL bContinue = pclIndSelCVParams->m_CtrlList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelCVParams->m_CtrlList.GetNext( str, lpParam ) )
		{
			double dTarget = -1.0;
			bNewTypeMustBeEntered = true;
			CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( (CData *)lpParam );
			ASSERT( NULL != pCV );
			
			// If control valve exceeds size limit we don't manage it!
			int iDN = pCV->GetSizeKey();

			if( iDN < iLowestSize || iDN > iHighestSize )
			{
				continue;
			}

			if( pclIndSelCVParams->m_dDp > 0.0 )
			{
				// Target to reach is the Dp input by user.
				dTarget = pclIndSelCVParams->m_dDp;
				
				// Compute Dp on the current control valve with flow and Kvs.
				dCvDporKvsValue = CalcDp( pclIndSelCVParams->m_dFlow, pCV->GetKvs(), pclIndSelCVParams->m_WC.GetDens() );
			}
			else if( pclIndSelCVParams->m_dKvs > 0.0 )
			{
				// Target to reach is the Kvs input by user.
				dTarget = pclIndSelCVParams->m_dKvs;
				
				// No need to compute, Kvs is given by the current control valve.
				dCvDporKvsValue = pCV->GetKvs();
			}
				
			// If at least one valve was found in the range [ Target; Target + 50% ]...
			if( dCvDporKvsValue > dTarget && dCvDporKvsValue <= dUpperLimitKvsOrDp )
			{
				bFindInUpperLimits = true;
			}
				
			// If at least one valve was found in the range [ Target - 50%; Target ]...
			if( dCvDporKvsValue < dTarget && dCvDporKvsValue >= dLowerLimitKvsOrDp )
			{
				bFindInLowerLimits = true;
			}

			// Run control valve already inserted in the array...
			for( int i = 0; i < ar.GetCount(); i++ )
			{
				CSelectCvTypeLimits Pointer = ar.GetAt( i );
					
				// If control valve in the array is the same type of the current selected control valve...
				if( ( pCV->GetBodyMaterialIDPtr().MP == Pointer.m_pCv->GetBodyMaterialIDPtr().MP ) &&
					( pCV->GetConnectIDPtr().MP == Pointer.m_pCv->GetConnectIDPtr().MP ) &&
					( pCV->GetVersionIDPtr().MP == Pointer.m_pCv->GetVersionIDPtr().MP ) &&
					( pCV->GetSizeKey() == Pointer.m_pCv->GetSizeKey() ) &&
					( pCV->GetPNIDPtr().MP == Pointer.m_pCv->GetPNIDPtr().MP ) &&
					( pCV->GetStroke() == Pointer.m_pCv->GetStroke() ) &&
					( pCV->GetRangeability() == Pointer.m_pCv->GetRangeability() ) )
				{
					// Don't add in the array but adapt if possible the boundaries.
					bNewTypeMustBeEntered = false;
						
					// If Kvs (or Dp) was found above the target and Kvs (or Dp) is below the current limit...
					if( dCvDporKvsValue > dTarget && dCvDporKvsValue < Pointer.GetLimitAbove() )
					{
						// Set the new limit.
						Pointer.SetLimitAbove( dCvDporKvsValue );
						ar.SetAt( i, Pointer );
					}
						
					// If Kvs (or Dp) was found below the target and Kvs (or Dp) is above the current limit...
					if( dCvDporKvsValue < dTarget && dCvDporKvsValue > Pointer.GetLimitBelow() )
					{
						Pointer.SetLimitBelow( dCvDporKvsValue );
						ar.SetAt( i, Pointer );
					}
				}
			}

			if( true == bNewTypeMustBeEntered )
			{
				CSelectCvTypeLimits Input;
				Input.m_pCv = pCV;

				if( dCvDporKvsValue > dTarget )
				{
					Input.SetLimitAbove( dCvDporKvsValue );
				}
				else if( dCvDporKvsValue < dTarget )
				{
					Input.SetLimitBelow( dCvDporKvsValue );
				}

				ar.Add( Input );
			}
		}
	}
	
	ProductMMapKeyDecreasing::const_iterator It1;

	bool bAtLeastOneFit = false;
	*pbSizeShiftProblem = false;

	// HYS - 1457: We show valves with Dp below the minimal Dp set in the technical parameters.
	
	for( BOOL bContinue = pclIndSelCVParams->m_CtrlList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelCVParams->m_CtrlList.GetNext( str, lpParam ) )
	{
		CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( (CData *)lpParam );
		ASSERT( NULL != pCV );
		
		// If user has not input Kvs or Dp, 'fThinSelection' is set to 'false' and then we can add the valve.
		bool bMustBeAdded = !bThinSelection;		
		
		dKvs = pCV->GetKvs();
		dDp = CalcDp( pclIndSelCVParams->m_dFlow, dKvs, pclIndSelCVParams->m_WC.GetDens() );

		// If pressure drop exceeds technical parameter limit for control valve...
		if( dDp > dMaxCvDp )
		{
			bMustBeAdded = false;
		}

		if( true == bThinSelection )
		{
			if( pclIndSelCVParams->m_dKvs > 0.0 )
			{
				dCvDporKvsValue = dKvs;
			}
			else if( pclIndSelCVParams->m_dDp > 0.0 )
			{
				dCvDporKvsValue = dDp;
			}

			// Try to find a valve into the same category
			for( int i = 0; i < ar.GetCount(); i++ )
			{
				CSelectCvTypeLimits CvLimits = ar.GetAt( i );

				if( ( pCV->GetBodyMaterialIDPtr().MP == CvLimits.m_pCv->GetBodyMaterialIDPtr().MP ) &&
					( pCV->GetConnectIDPtr().MP == CvLimits.m_pCv->GetConnectIDPtr().MP ) &&
					( pCV->GetVersionIDPtr().MP == CvLimits.m_pCv->GetVersionIDPtr().MP ) &&
					( pCV->GetSizeKey() == CvLimits.m_pCv->GetSizeKey() ) &&
					( pCV->GetPNIDPtr().MP == CvLimits.m_pCv->GetPNIDPtr().MP ) &&
					( pCV->GetStroke() == CvLimits.m_pCv->GetStroke() ) &&
					( pCV->GetRangeability() == CvLimits.m_pCv->GetRangeability() ) )
				{
					// If the item is right between the braces...
					if( ( dCvDporKvsValue <= CvLimits.GetLimitAbove() ) && ( dCvDporKvsValue >= CvLimits.GetLimitBelow() ) )
					{
						if( true == bFindInLowerLimits && true == bFindInUpperLimits )
						{
							// Show only valves between the upper and lower boundaries.
							if( dCvDporKvsValue >= dLowerLimitKvsOrDp && dCvDporKvsValue <= dUpperLimitKvsOrDp )
							{
								bMustBeAdded = true;
							}
						}
						else
						{
							bMustBeAdded = true;
						}
					}
				}
			}
		}

		if( false == bMustBeAdded )
		{
			pclIndSelCVParams->m_CtrlList.Delete();
			continue;
		}
		
		// If control valve exceeds size limit we don't kept it!
		int iDN = pCV->GetSizeKey();
		
		if( iDN < iLowestSize || iDN > iHighestSize )
		{
			pclIndSelCVParams->m_CtrlList.Delete();

			// HYS-699: If this product can be selected by increasing size above/below in technical parameters, we warn the user.
			if( iDN >= iMaxLowestSize && iDN <= iMaxHighestSize )
			{
				bAtLeastOneFit = true;
			}

			continue;
		}
		
		// 'm_mapPriority' stores for each size the minimum priority level (most important).
		if( 0 == m_mapPriority.count( pCV->GetSizeKey() ) )
		{
			m_mapPriority[pCV->GetSizeKey()] = INT_MAX;
		}

		CTAPSortKey sKey( (CDB_TAProduct*)pCV );
		int iBits = 0;
			
		LONGLONG LL = sKey.GetPartialSortingKey( iBits, &CTAPSortKey::CTAPSKMask( CTAPSortKey::TAPSKM_All ), CTAPSortKey::TAPKO_Size, 
				CTAPSortKey::TAPKO_Priority, CTAPSortKey::TAPKO_BdyMat | CTAPSortKey::TAPKO_Inverse, CTAPSortKey::TAPKO_Conn | CTAPSortKey::TAPKO_Inverse, 
				CTAPSortKey::TAPKO_Vers | CTAPSortKey::TAPKO_Inverse, CTAPSortKey::TAPKO_Pn | CTAPSortKey::TAPKO_Inverse, 
				CTAPSortKey::TAPKO_Fam | CTAPSortKey::TAPKO_Inverse, -1 );

		// Remark: we multiply 'Kvs' by 10 for the first decimal.
		unsigned int uiKvs = (unsigned int)( pCV->GetKvs() * 10 );
			
		if( uiKvs > 0x7FFF )
		{
			// Can happen with for example 'DR 16 EVSA/R DN400' valve that has a Kvs of 16350 (*10 = 163500)
			uiKvs = 0x7FFF;
		}

		LL <<= 15;
		LL += uiKvs;
		double dKey = (double)LL;

		CSelectedValve *pSelCV = new CSelectedValve();

		if( NULL != pSelCV )
		{
			pSelCV->SetProductIDPtr( pCV->GetIDPtr() );
			m_PreselMMapKeyDecreasing.insert( ProductPair( dKey, pSelCV ) );

			pSelCV->SetDp( dDp );
			pSelCV->SetFlag( CSelectedBase::eTemperature, pclIndSelCVParams->m_WC.GetTemp() < pCV->GetTmin() || pclIndSelCVParams->m_WC.GetTemp() > pCV->GetTmax() );
			pSelCV->SetFlag( CSelectedBase::Flags::eNotPriority, true );
		}
	}

	// Do a last loop to compute what is the lowest possible Dp value just equal or above the Dp value input by user...
	m_dMinDpMinCv = DBL_MAX;
	
	for( int iPhase = 0; iPhase < 2; iPhase++ )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); ++m_cIter)
		{
			CSelectedValve *pSelTap = (CSelectedValve *)m_cIter->second;
			ASSERT( NULL != pSelTap );

			if( NULL == pSelTap )
			{
				continue;
			}

			CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( pSelTap->GetProductIDPtr().MP );
			ASSERT( NULL != pCV );

			if( NULL == pCV ) 
			{
				continue;
			}

			int iSizeKey = pCV->GetSizeKey();

			switch( iPhase )
			{
				// Store highest priority for each product size.
				case 0:
					
					// If current selection has the lowest value for its size (lowest value means highest priority!)...
					if( pCV->GetPriorityLevel() < m_mapPriority[iSizeKey] )
					{
						m_mapPriority[iSizeKey] = pCV->GetPriorityLevel();
					}

					if( pclIndSelCVParams->m_dDp > 0.0 )
					{
						// Get the Dp value.
						double dDp = CalcDp( pclIndSelCVParams->m_dFlow, pCV->GetKvs(), pclIndSelCVParams->m_WC.GetDens() );

						// Calculate the min Dp equal or above the Dp input by user.
						if( ( dDp + 200 ) >= pclIndSelCVParams->m_dDp )
						{
							if( dDp < m_dMinDpMinCv )
							{
								m_dMinDpMinCv = dDp;
							}
						}
					}
					break;

				// Ordering key.
				case 1:
					{
						// Best product is only into high priority product.
						if( m_mapPriority[iSizeKey] == pCV->GetPriorityLevel() )
						{
							pSelTap->SetFlag( CSelectedBase::eNotPriority, false);
						}

						CTAPSortKey sKey( (CDB_TAProduct*)pCV );

						// Add 1 bit for priority.
						LONGLONG LL = ( true == pSelTap->IsFlagSet( CSelectedBase::eNotPriority ) ? 0 : 1 );

						int iBits = 0;

						LONGLONG LL1 = sKey.GetPartialSortingKey( iBits, &CTAPSortKey::CTAPSKMask( CTAPSortKey::TAPSKM_All ), CTAPSortKey::TAPKO_Size, 
								CTAPSortKey::TAPKO_Priority, CTAPSortKey::TAPKO_BdyMat | CTAPSortKey::TAPKO_Inverse, CTAPSortKey::TAPKO_Conn | CTAPSortKey::TAPKO_Inverse, 
								CTAPSortKey::TAPKO_Vers | CTAPSortKey::TAPKO_Inverse, CTAPSortKey::TAPKO_Pn | CTAPSortKey::TAPKO_Inverse, 
								CTAPSortKey::TAPKO_Fam | CTAPSortKey::TAPKO_Inverse, -1 );

						LL <<= iBits;
						LL += LL1;

						// Remark: we multiply 'Kvs' by 10 for the first decimal.
						unsigned int uiKvs = (unsigned int)( pCV->GetKvs() * 10 );

						if( uiKvs > 0x7FFF )
						{
							// Can happen with for example 'DR 16 EVSA/R DN400' valve that has a Kvs of 16350 (*10 = 163500)
							uiKvs = 0x7FFF;
						}

						LL <<= 15;
						LL += uiKvs;
						double dKey = (double)LL;

						m_MMapKeyDecreasing.insert( ProductPair( dKey, pSelTap ) );
					}
					break;
			}
		}
	}

	if( DBL_MAX == m_dMinDpMinCv )
	{
		m_dMinDpMinCv = pclIndSelCVParams->m_dDp;
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	// HYS-699: If this product can be selected by increasing size above/below in technical parameters, we warn the user.
	if( 0 == m_MMapKeyDecreasing.size() && true == bAtLeastOneFit )
	{
		*pbSizeShiftProblem = true;
	}

	return m_MMapKeyDecreasing.size();
}

////////////////////////////////////////////////////////////////
//	CSelectPICVList
////////////////////////////////////////////////////////////////
int CSelectPICVList::SelectQ( CIndSelPIBCVParams *pclIndSelPIBCVParams, bool *pbSizeShiftProblem )
{
	if( NULL == pclIndSelPIBCVParams || NULL == pclIndSelPIBCVParams->m_pTADS || NULL == pclIndSelPIBCVParams->m_pTADS->GetpTechParams()
		|| NULL == pclIndSelPIBCVParams->m_pTADB || NULL == pbSizeShiftProblem )
	{
		ASSERTA_RETURN( 0 );
	}

	*pbSizeShiftProblem = false;

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return 0;
	}
	
	// Variables.
	_string str;
	LPARAM lpParam;
	double dKey = 0.0;
	double dDp = 0.0;
	ProductMMapKeyDecreasing::iterator It, It1;
	
	if( false == m_clSelectPipeList.GetPipeSizeBound() )
	{
		return 0;
	}

	CDB_Pipe *pSmallestPipe = m_clSelectPipeList.GetpSmallestPipe();

	if( NULL == pSmallestPipe )
	{
		return 0;
	}

	CDB_Pipe *pBiggestPipe = m_clSelectPipeList.GetpLargestPipe();

	if( NULL == pBiggestPipe )
	{
		return 0;
	}
	
	// Take into account SizeShiftAbove and SizeShiftBelow
	// Setup variables to possibly enforce combined SizeShifts
	// In case of Hub Selection don't use SizeShift from TechParam
	int iSizeShiftUp;
	int iSizeShiftDown;
	CDS_TechnicalParameter *pclTechParams = pclIndSelPIBCVParams->m_pTADS->GetpTechParams();
	
	if( 99 == pclTechParams->GetSizeShiftAbove() ) // Combined SizeShift +3/-3 enforced
	{
		iSizeShiftUp = 3;
		iSizeShiftDown = -3;
	}
	else
	{
		iSizeShiftUp = pclTechParams->GetSizeShiftAbove();
		iSizeShiftDown = pclTechParams->GetSizeShiftBelow();
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclIndSelPIBCVParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN = pclTableDN->GetBiggestDN();
	int iHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelPIBCVParams->m_pTADB ) + iSizeShiftUp );
	int iLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelPIBCVParams->m_pTADB ) + iSizeShiftDown );

	int iMaxHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelPIBCVParams->m_pTADB ) + pclTechParams->GetMaxSizeShift() );
	int iMaxLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelPIBCVParams->m_pTADB ) - pclTechParams->GetMaxSizeShift() );

	int iDN15 = pclTableDN->GetSize( _T("DN_15") );
	
	if( iHighestSize < iDN15 )
	{
		iHighestSize = iDN15;
	}

	if( iMaxHighestSize < iDN15 )
	{
		iMaxHighestSize = iDN15;
	}

	// Prepare an array with a entry for each control valve size.
	for( BOOL bContinue = pclIndSelPIBCVParams->m_CtrlList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelPIBCVParams->m_CtrlList.GetNext( str, lpParam ) )
	{
		CDB_ControlValve *pControlValve = dynamic_cast<CDB_ControlValve *>( (CData *)lpParam );
		ASSERT( NULL != pControlValve );
		
		if( NULL == pControlValve )
		{
			continue;
		}

		int iSizeKey = pControlValve->GetSizeKey();

		// If entry doesn't yet exist, create it and set INT_MAX value. The good values will be set after below.
		if( 0 == m_mapPriority.count( iSizeKey ) )
		{
			m_mapPriority[ iSizeKey ] = INT_MAX;
		}
	}

	bool bAtLeastOneFit = false;
	*pbSizeShiftProblem = false;
	
	for( BOOL bContinue = pclIndSelPIBCVParams->m_CtrlList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelPIBCVParams->m_CtrlList.GetNext( str, lpParam ) )
	{
		CDB_PIControlValve *pPICV = dynamic_cast<CDB_PIControlValve *>( (CData *)lpParam );
		ASSERT( NULL != pPICV );

		if( NULL == pPICV )
		{
			continue;
		}
		
		// Calculate the Dpmin of Dp control part.
		dDp = pPICV->GetDpmin( pclIndSelPIBCVParams->m_dFlow, pclIndSelPIBCVParams->m_WC.GetDens() );

		if( -1.0 == dDp )
		{
			pclIndSelPIBCVParams->m_CtrlList.Delete();
			continue;
		}

		// Do not add valves with a Dp below the min Dp set into the Technical parameter box.
		if( 1 == pclTechParams->GetPICvHide() && dDp < pclTechParams->GetPICvMinDp() )
		{
			pclIndSelPIBCVParams->m_CtrlList.Delete();
			continue;
		}

		// Do not allow a higher flow than the max flow allowed for the valve.
		if( -1.0 == pPICV->GetQmax() || pclIndSelPIBCVParams->m_dFlow > pPICV->GetQmax() )
		{
			pclIndSelPIBCVParams->m_CtrlList.Delete();
			continue;
		}
		
		int iDN = pPICV->GetSizeKey();

		// Do a test to verify valves will exist with the maximum boundaries.
		// If not, the problem is not a sizeShift problem.
		if( iDN < iMaxLowestSize || iDN > iMaxHighestSize )
		{
			pclIndSelPIBCVParams->m_CtrlList.Delete();
			continue;
		}

		// Means that at least one PICV size is in the maximum boundaries [-4, +4]. If any PICV size is in the range [iLowestSize, iHighestSize],
		// 'Select' will return 0 but with 'SizeShiftProblem' set to 'true' we signal that it is possible to find PICV by changing above and below
		// size range and technical parameter.
		if( false == bAtLeastOneFit )
		{
			*pbSizeShiftProblem = true;
		}

		if( iDN >= iLowestSize && iDN <= iHighestSize )
		{
			// There is no problem with the size shift.
			*pbSizeShiftProblem = false;
			bAtLeastOneFit = true;

			// Retrieve curve characteristic.
			CDB_PICVCharacteristic *pclPICVCharacteristic = pPICV->GetPICVCharacteristic();
			ASSERT( NULL != pclPICVCharacteristic );

			if( NULL == pclPICVCharacteristic )
			{
				continue;
			}

			// Compute setting.
			double dPresetting = pPICV->GetPresetting( pclIndSelPIBCVParams->m_dFlow, pclIndSelPIBCVParams->m_WC.GetDens(), pclIndSelPIBCVParams->m_WC.GetKinVisc() );

			// HYS-1928: we don't allow valve that has a presetting to 0.
			if( dPresetting <= 0.0 )
			{
				continue;
			}

			// Check if current valve has a curve of stroke in function of the setting.
			bool bIsStrokeCurveDefined = false;

			if( pclPICVCharacteristic->IsStrokeCurveDefined() )
			{
				bIsStrokeCurveDefined = true;
			}

			// Retrieve the actuator group on the selected control valve.
			CTable *pclActuatorGroup = ( CTable * )( pPICV->GetActuatorGroupIDPtr().MP );

			if( NULL == pclActuatorGroup )
			{
				continue;
			}

			// Retrieve list of all actuators in this group.
			CRank rList( false );
			pclIndSelPIBCVParams->m_pTADB->GetActuator( &rList, pclActuatorGroup, L"", L"", -1, CDB_ElectroActuator::DefaultReturnPosition::Undefined, pclIndSelPIBCVParams->m_eFilterSelection );

			bool bCanInsert = false;
			CString str;
			LPARAM lparam;

			for( BOOL bContinue = rList.GetFirst( str, lparam ); TRUE == bContinue && false == bCanInsert; bContinue = rList.GetNext( str, lparam ) )
			{
				CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( (CData *)lparam );

				if( NULL == pclElectroActuator )
				{
					continue;
				}

				// HYS-1355: First of all verify if setting of the valve is below the minimum measurable setting. 
				// If the current actuator has not the possibility to set a minimum limited stroke, we don't accept this actuator.
				// Remark: Below this setting, there is too much error with the TA-Scope to read flow. So, it�s no recommended to work 
				// with it to apply a perfect balancing. Except if we can put an actuator for which its lift is controlled by electronic 
				// (Like TA-Slider).
				if( NULL != pclPICVCharacteristic && dPresetting < pclPICVCharacteristic->GetMinMeasSetting()
						&& CDB_Actuator::LimitedStrokeMode::LSM_Electronic != pclElectroActuator->GetLimitedStrokeMode() )
				{
					continue;
				}

				// HYS-1389: Second test: if valve has stroke characteristic in function of the setting AND if the actuator has
				// a mechanical limited stroke, we verify if the minimal limited stroke of the actuator is enough for the setting
				// of the valve. Example of EMO-TM (1mm limited stroke) with TA-Compact-P DN 10 setting 3 (lift < 1mm).
				if( NULL != pclPICVCharacteristic && true == bIsStrokeCurveDefined 
					&& CDB_Actuator::LimitedStrokeMode::LSM_No != pclElectroActuator->GetLimitedStrokeMode()
					&& -1.0 != pclElectroActuator->GetMinLimitedStroke()
					&& true == pclPICVCharacteristic->StrokeCharGiven()
					&& dPresetting < pclPICVCharacteristic->GetMinSettingWithActuator( pclElectroActuator ) )
				{
					continue;
				}

				bCanInsert = true;
			}

			if( false == bCanInsert )
			{
				continue;
			}

			CSelectedValve *pSelPICV = new CSelectedValve();

			if( NULL != pSelPICV )
			{
				pSelPICV->SetProductIDPtr( pPICV->GetIDPtr() );

				pSelPICV->SetFlag( CSelectedBase::eTemperature, pclIndSelPIBCVParams->m_WC.GetTemp() < pPICV->GetTmin() 
						|| pclIndSelPIBCVParams->m_WC.GetTemp() > pPICV->GetTmax() );

				pSelPICV->SetFlag( CSelectedBase::eNotPriority, true );
				CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pPICV, pSelPICV );
			}
		}
		else
		{
			pclIndSelPIBCVParams->m_CtrlList.Delete();
		}
	}

	if( m_PreselMMapKeyDecreasing.size() > 0 )
	{
		double dDpminbestvalve = DBL_MAX;
		int iBestDN = INT_MAX;
		double dBestPresetRatio = 0.0;
		double dAcceptedExtraDp = pclTechParams->GetPICVExtraThresholdDp(); // Accept an extra of 15kPa
		CDB_PIControlValve *pBestPICv = NULL;

		for( int iPhase = -1; iPhase < 2; iPhase++ )
		{
			for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
			{
				CSelectedBase *pclSelectedProduct = m_cIter->second;
				ASSERT( NULL != pclSelectedProduct );

				if( NULL == pclSelectedProduct )
				{
					continue;
				}

				CSelectedValve *pclSelectedPICv = dynamic_cast<CSelectedValve *>( pclSelectedProduct );
				ASSERT( NULL != pclSelectedPICv );

				if( NULL == pclSelectedPICv )
				{
					continue;
				}

				CDB_PIControlValve *pclPICv = dynamic_cast<CDB_PIControlValve*>( pclSelectedPICv->GetpData() );
				ASSERT( NULL != pclPICv );

				if( NULL == pclPICv )
				{
					continue;
				}

				int iCurrentDN = pclPICv->GetSizeKey();
				
				switch( iPhase )
				{
					case -1:

						// 'm_mapPriority' store for each size the minimum priority level (the most important).
						if( pclPICv->GetPriorityLevel() < m_mapPriority[iCurrentDN] )
						{
							m_mapPriority[iCurrentDN] = pclPICv->GetPriorityLevel();
						}

						break;

					// Store highest priority for each product size and check best valve.
					case 0:
						{
							// Get the Dp min value.
							double dDpMin = pclPICv->GetDpmin( pclIndSelPIBCVParams->m_dFlow, pclIndSelPIBCVParams->m_WC.GetDens() );

							if( -1.0 == dDpMin )
							{
								continue;
							}

							// Get the Dp min value.
							bool bNewBestExist = false;
		
							// (Best one = Smallest Dpmin amongst valves with smallest DN  with a threshold of AcceptedExtraDp if a larger valve has a lower Dpmin).
							// For the same size DpMin for NF is below Dpmin for LF Verify the opening, best valve is the one closest to its maximum opening.
							double dPresetting = pclPICv->GetPresetting( pclIndSelPIBCVParams->m_dFlow, pclIndSelPIBCVParams->m_WC.GetDens(), 
									pclIndSelPIBCVParams->m_WC.GetKinVisc() );

							double dMaxSetting = pclPICv->GetMaxSetting();

							if( -1.0 == dPresetting || -1.0 == dMaxSetting )
							{
								// No need to continue with this valve, we are not able to preset!
								continue;
							}

							pclSelectedPICv->SetH( dPresetting );
							pclSelectedPICv->SetDpMin( dDpMin );

							// Skip product with a lower commercial priority for selecting Best valve.
							if( m_mapPriority[iCurrentDN] != pclPICv->GetPriorityLevel() )
							{
								continue;
							}

							double dPresetRatio = dPresetting / dMaxSetting;
		
							// Specific treatment for TBV-CMP 3/4"HF and 1/2"HF same body as 25NF!
							// Assume analyzing object in decreasing size order.
							bool bValveSharingSameBody = false;

							if ( NULL != pBestPICv 
								&& 0 == IDcmp( pBestPICv->GetFamilyID(), _T("FAM_TBV-CMP") )
								&& 0 == IDcmp( pclPICv->GetFamilyID(), _T("FAM_TBV-CMP") ) )
							{
								bValveSharingSameBody = ( pclPICv->GetPICVCharacteristic() == pBestPICv->GetPICVCharacteristic() );
								bValveSharingSameBody |= ( dPresetting < pclPICv->GetPICVCharacteristic()->GetMinRecSetting() );
							}

							if( iCurrentDN == iBestDN )
							{
								if( dDpMin < ( dDpminbestvalve + dAcceptedExtraDp ) )
								{
									if( dPresetRatio > dBestPresetRatio )
									{
										bNewBestExist = true;
										dBestPresetRatio = dPresetRatio;
									}
								}
							}
							else if( ( iCurrentDN < iBestDN && dDpMin < ( dDpminbestvalve + dAcceptedExtraDp ) && false == bValveSharingSameBody ) ||
									 ( iCurrentDN > iBestDN && ( dDpMin + dAcceptedExtraDp ) < dDpminbestvalve ) )
							{
								bNewBestExist = true;
								dBestPresetRatio = dPresetRatio;
							}

							if( true == bNewBestExist )
							{
								iBestDN = iCurrentDN;
								dDpminbestvalve = dDpMin;
								pBestPICv = pclPICv;
							}
						}
						break;

					// Ordering Key.
					case 1:
						{
							// If current selection has the lowest priority level for its size (lowest level means highest priority!)...
							if( m_mapPriority[iCurrentDN] == pclPICv->GetPriorityLevel() )
							{
								pclSelectedPICv->SetFlag( CSelectedBase::eNotPriority, false );
							}

							// For the key we must reserve 8bits for the FC value (/1000). Because some valves (as TBV-CMP LF & NF or KTM 512 LF & NF) have
							// the 'Priority', 'Type', 'Size', 'Family', 'Body', 'Version' and 'PN' characteristics exactly the same. We can differentiate
							// LF and NF with the 'Fc value' from the PICV characteristic. We reserve 8bits: value max = 255 -> range 1000 � 250000 Pa for Fc).
							// We insert this value just after the family key. We must then reserve the space with a key with the same total bits (family).
							// After the key is done, we clear the bits reserved and replace them by the Fc value if exist.
							
							CTAPSortKey sKey( (CDB_TAProduct*)pclPICv );
							
							dKey = sKey.GetSortingKeyCustomAsDouble( &CTAPSortKey::CTAPSKMask( CTAPSortKey::TAPSKM_All ), CTAPSortKey::TAPKO_Priority | CTAPSortKey::TAPKO_Inverse, 
									CTAPSortKey::TAPKO_Type | CTAPSortKey::TAPKO_Inverse, CTAPSortKey::TAPKO_Size, CTAPSortKey::TAPKO_Fam | CTAPSortKey::TAPKO_Inverse, 
									CTAPSortKey::TAPKO_Fam, CTAPSortKey::TAPKO_BdyMat | CTAPSortKey::TAPKO_Inverse, CTAPSortKey::TAPKO_Conn | CTAPSortKey::TAPKO_Inverse, 
									CTAPSortKey::TAPKO_Vers | CTAPSortKey::TAPKO_Inverse, CTAPSortKey::TAPKO_Pn | CTAPSortKey::TAPKO_Inverse, -1 );

							int iFcShiftBit = sKey.GetNbrBitsKeyUsed( CTAPSortKey::TAPKO_BdyMat );
							iFcShiftBit += sKey.GetNbrBitsKeyUsed( CTAPSortKey::TAPKO_Conn );
							iFcShiftBit += sKey.GetNbrBitsKeyUsed( CTAPSortKey::TAPKO_Vers );
							iFcShiftBit += sKey.GetNbrBitsKeyUsed( CTAPSortKey::TAPKO_Pn );

							UINT64 ui64FcMask = 255 << iFcShiftBit;
							ui64FcMask = ~ui64FcMask;

							UINT64 ui64KeyValue = (UINT64)dKey;
							ui64KeyValue &= ui64FcMask;

							// Fc; size and family are not enough for instance TBV-CMP LF and NF.
							if( -1.0 != pclPICv->GetFc() )
							{
								UINT64 ui64FcValue = ( (UINT64)( pclPICv->GetFc() / 1000 ) & 0xFF ) << iFcShiftBit;
								ui64KeyValue += ui64FcValue;
							}

							dKey = (double)ui64KeyValue;

							m_MMapKeyDecreasing.insert( ProductPair( dKey, pclSelectedPICv ) );

							// Get the Dp min value.
							double dDpMin = pclSelectedPICv->GetDpMin();// pclPICv->GetDpmin( m_dFlow, m_pWC->GetDens() );

							if( -1.0 == dDpMin )
							{
								continue;
							}
		
							// Set the best valve in green.
							int iCurrentDN = pclPICv->GetSizeKey();

							if( -1 != iCurrentDN )
							{
								if( dDpminbestvalve == dDpMin && iCurrentDN == iBestDN )
								{
									pclSelectedPICv->SetFlag( CSelectedBase::eBest, true );
								}
							}
						}
						break;
				}
			}
		}
	}

	return m_MMapKeyDecreasing.size();
}

////////////////////////////////////////////////////////////////
//	CSelectBCVList
////////////////////////////////////////////////////////////////
CSelectBCVList::CSelectBCVList()
	: CSelectCtrlList()
{
	Clean();
}

void CSelectBCVList::Clean()
{
	CSelectCtrlList::Clean();
	m_dPpn = 0.0;
	m_bForceFullOpening = false;
}

// PROTECTED MEMBERS
int CSelectBCVList::SelectQ( CIndSelBCVParams *pclIndSelBCVParams, bool *pbSizeShiftProblem, bool bForceFullOpening )
{
	if( NULL == pclIndSelBCVParams || NULL == pclIndSelBCVParams->m_pTADS || NULL == pclIndSelBCVParams->m_pTADS->GetpTechParams()
		|| NULL == pclIndSelBCVParams->m_pTADB || NULL == pbSizeShiftProblem )
	{
		ASSERTA_RETURN( 0 );
	}

	m_bForceFullOpening = bForceFullOpening;

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return 0;
	}

	// Variables.
	_string str;
	LPARAM lparam;
	double dKey = 0.0;
	double dDp = 0.0;
	ProductMMapKeyDecreasing::iterator It, It1;
	bool bIsDpDefined = ( pclIndSelBCVParams->m_dDp > 0.0 ) ? true : false;

	if( false == m_clSelectPipeList.GetPipeSizeBound() )
	{
		return 0;
	}

	CDB_Pipe *pSmallestPipe = m_clSelectPipeList.GetpSmallestPipe();

	if( NULL == pSmallestPipe )
	{
		return 0;
	}

	CDB_Pipe *pBiggestPipe = m_clSelectPipeList.GetpLargestPipe();

	if( NULL == pBiggestPipe )
	{
		return 0;
	}

	// Take into account SizeShiftAbove and SizeShiftBelow
	// Setup variables to possibly enforce combined SizeShifts
	// In case of Hub Selection don't use SizeShift from TechParam
	int SizeShiftUp;
	int SizeShiftDown;
	CDS_TechnicalParameter *pclTechParams = pclIndSelBCVParams->m_pTADS->GetpTechParams();

	if( 99 == pclTechParams->GetSizeShiftAbove() ) // Combined SizeShift +3/-3 enforced
	{
		SizeShiftUp = 3;
		SizeShiftDown = -3;
	}
	else
	{
		SizeShiftUp = pclTechParams->GetSizeShiftAbove();
		SizeShiftDown = pclTechParams->GetSizeShiftBelow();
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclIndSelBCVParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN = pclTableDN->GetBiggestDN();
	int iHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelBCVParams->m_pTADB ) + SizeShiftUp );
	int iLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelBCVParams->m_pTADB ) + SizeShiftDown );

	// HYS-699: Now we check also with the extreme limits to avoid to get the message that we don't found because
	// a size shift problem when there is no possibility even with size above/below set to 4 (max.).
	int iMaxHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelBCVParams->m_pTADB ) + pclTechParams->GetMaxSizeShift() );
	int iMaxLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelBCVParams->m_pTADB ) - pclTechParams->GetMaxSizeShift() );

	// In case of pipe with a size below DN15 force a selection up to DN15 (first available size in regulating valve).
	int iDN15 = pclTableDN->GetSize( _T("DN_15") );

	if( iHighestSize < iDN15 )
	{
		iHighestSize = iDN15;
	}

	if( iMaxHighestSize < iDN15 )
	{
		iMaxHighestSize = iDN15;
	}

	bool bAtLeastOneFit = false;
	*pbSizeShiftProblem = false;

	m_rBest.dBestDpAboveMin2 = DBL_MAX;
	m_rBest.dBestDpBelowMin = 0.0;
	m_rBest.dBestDp = 0.0;
	m_rBest.dBestDelta = DBL_MAX;

	// Prepare an array with a entry for each control valve size.
	for( BOOL bContinue = pclIndSelBCVParams->m_CtrlList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = pclIndSelBCVParams->m_CtrlList.GetNext( str, lparam ) )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( (CData *)lparam );
		ASSERT( NULL != pclControlValve );

		if( NULL == pclControlValve )
		{
			continue;
		}

		int iSizeKey = pclControlValve->GetSizeKey();
		
		// If entry doesn't yet exist, create it and set INT_MAX value. The good values will be set after below.
		if( 0 == m_mapPriority.count( iSizeKey ) )
		{
			m_mapPriority[ iSizeKey ] = INT_MAX;
		}
	}

	bool bValidFound = false;
	double dValveMinDp = ( CDB_ControlProperties::eCvOnOff == pclIndSelBCVParams->m_eCvCtrlType ) ? pclTechParams->GetCVMinDpOnoff() : pclTechParams->GetCVMinDpProp();

	// When Dp is not specified we have to return to technical parameters to know what is the MinDp.
	if( false == bIsDpDefined )
	{
		pclIndSelBCVParams->m_dDp = dValveMinDp;
	}

	// Compute target KVS.
	double dTargetKvs = CalcKv( pclIndSelBCVParams->m_dFlow, pclIndSelBCVParams->m_dDp, pclIndSelBCVParams->m_WC.GetDens() );

	for( BOOL bContinue = pclIndSelBCVParams->m_CtrlList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = pclIndSelBCVParams->m_CtrlList.GetNext( str, lparam ) )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( (CData *)lparam );
		ASSERT( NULL != pclControlValve );

		if( NULL == pclControlValve )
		{
			pclIndSelBCVParams->m_CtrlList.Delete();
			continue;
		}

		if( pclControlValve->GetSizeKey() < iLowestSize || pclControlValve->GetSizeKey() > iHighestSize )
		{
			pclIndSelBCVParams->m_CtrlList.Delete();

			// HYS-699: If this product can be selected by increasing size above/below in technical parameters, we warn the user.
			if( pclControlValve->GetSizeKey() >= iMaxLowestSize && pclControlValve->GetSizeKey() <= iMaxHighestSize )
			{
				bAtLeastOneFit = true;
			}

			continue;
		}

		if( CDB_ControlProperties::CvCtrlType::eCvNU != pclIndSelBCVParams->m_eCvCtrlType )
		{
			if( NULL == pclControlValve->GetCtrlProp() || false == pclControlValve->GetCtrlProp()->CvCtrlTypeFits( pclIndSelBCVParams->m_eCvCtrlType ) )
			{
				pclIndSelBCVParams->m_CtrlList.Delete();
				continue;
			}
		}

		// Get Kv characteristic of the valve.
		CDB_ValveCharacteristic *pValveCharacteristic = pclControlValve->GetValveCharacteristic();

		if( NULL == pValveCharacteristic )
		{
			pclIndSelBCVParams->m_CtrlList.Delete();
			continue;
		}

		CSelectedValve *pSelBCV = new CSelectedValve();

		if( NULL == pSelBCV )
		{
			pclIndSelBCVParams->m_CtrlList.Delete();
			continue;
		}

		pSelBCV->SetProductIDPtr( pclControlValve->GetIDPtr() );

		// Prepare some variables.
		double dDpFullyOpen;
		double dDpHalfOpen;
		double dDpQuarterOpen;
		double dHMax = pValveCharacteristic->GetOpeningMax();

		if( -1.0 == dHMax )
		{
			pclIndSelBCVParams->m_CtrlList.Delete();
			continue;
		}

		if( false == pValveCharacteristic->GetValveDp( pclIndSelBCVParams->m_dFlow, &dDpFullyOpen, dHMax, pclIndSelBCVParams->m_WC.GetDens(),
			pclIndSelBCVParams->m_WC.GetKinVisc() ) )
		{
			pclIndSelBCVParams->m_CtrlList.Delete();
			continue;
		}

		double dHHalf = 0.5 * dHMax;

		if( false == pValveCharacteristic->GetValveDp( pclIndSelBCVParams->m_dFlow, &dDpHalfOpen, dHHalf, pclIndSelBCVParams->m_WC.GetDens(),
			pclIndSelBCVParams->m_WC.GetKinVisc() ) )
		{
			pclIndSelBCVParams->m_CtrlList.Delete();
			continue;
		}

		double dHQuarter = 0.25 * dHMax;

		if( false == pValveCharacteristic->GetValveDp( pclIndSelBCVParams->m_dFlow, &dDpQuarterOpen, dHQuarter, pclIndSelBCVParams->m_WC.GetDens(),
			pclIndSelBCVParams->m_WC.GetKinVisc() ) )
		{
			pclIndSelBCVParams->m_CtrlList.Delete();
			continue;
		}

		// Compute hand wheel setting.
		double dH = -1.0;
		double dDp = -1.0;
		bool bNotFound;

		if( true == m_bForceFullOpening )
		{
			dH = dHMax;
			dDp = dDpFullyOpen;
			bNotFound = false;
			pSelBCV->SetFlag( CSelectedBase::eValveMaxSetting, true );
		}
		else
		{
			// Selection process from flow and Dp if user has introduced a Dp value (fDpOK = true)
			// OR from flow only if no Dp.
			// Remark: If at 100% opening ( dDpFullyOpen ) we already have
			//         a Dp greater than Dp min, it is impossible to open more to reach good Dp min.
			//         
			if( ( true == bIsDpDefined && pclIndSelBCVParams->m_dDp >= dDpFullyOpen )
					|| ( false == bIsDpDefined && dDpFullyOpen < dValveMinDp ) )
			{
				// If user wants a specific Dp (fDpOp = true) we must take his value, otherwise we take the min Dp for the valve.
				dDp = ( true == bIsDpDefined ) ? pclIndSelBCVParams->m_dDp : dValveMinDp;

				// Try to find a solution.
				bool bValid = pValveCharacteristic->GetValveOpening( pclIndSelBCVParams->m_dFlow, dDp, &dH, pclIndSelBCVParams->m_WC.GetDens(),
						pclIndSelBCVParams->m_WC.GetKinVisc(), ( eBool3::eb3True == pValveCharacteristic->IsDiscrete() ) ? 3 : 1 );

				// The min Kv value was not small enough !
				if( false == bValid )
				{
					dH = pValveCharacteristic->GetMinRecSetting();

					if( dH <= 0.0 )
					{
						dH = pValveCharacteristic->GetOpeningMin();
					}

					if( -1.0 != dH )
					{
						if( false == pValveCharacteristic->GetValveDp( pclIndSelBCVParams->m_dFlow, &dDp, dH, pclIndSelBCVParams->m_WC.GetDens(),
							pclIndSelBCVParams->m_WC.GetKinVisc() ) )
						{
							pclIndSelBCVParams->m_CtrlList.Delete();
							continue;
						}
					}
				}
			}

			// If dh < 0, no solution was found.
			bNotFound = ( dH < 0.0 ) ? true : false;

			if( true == bNotFound )
			{
				dH = dHMax;
				dDp = dDpFullyOpen;
				pSelBCV->SetFlag( CSelectedBase::eValveMaxSetting, true );
			}
			else if( eBool3::eb3True == pValveCharacteristic->IsDiscrete() )
			{
				// Recalculate the Dp for the setting as determined above
				if( false == pValveCharacteristic->GetValveDp( pclIndSelBCVParams->m_dFlow, &dDp, dH, pclIndSelBCVParams->m_WC.GetDens(), pclIndSelBCVParams->m_WC.GetKinVisc() ) )
				{
					pclIndSelBCVParams->m_CtrlList.Delete();
					continue;
				}
			}
		}

		pSelBCV->SetFlag( CSelectedBase::Flags::eValveSetting, dH < pValveCharacteristic->GetMinRecSetting() );

		pSelBCV->SetFlag( CSelectedBase::Flags::eDp, dDp < dValveMinDp || dDp > pclTechParams->VerifyValvMaxDp( pclControlValve )
				|| ( true == bIsDpDefined && true == bNotFound ) );

		pSelBCV->SetFlag( CSelectedBase::Flags::eValveFullODp, dDpFullyOpen < dValveMinDp );

		pSelBCV->SetFlag( CSelectedBase::Flags::eValveHalfODp, ( dDpHalfOpen > pclTechParams->VerifyValvMaxDp( pclControlValve )
				|| dDpHalfOpen < dValveMinDp ) );

		pSelBCV->SetFlag( CSelectedBase::Flags::eTemperature, pclIndSelBCVParams->m_WC.GetTemp() < pclControlValve->GetTmin()
				|| pclIndSelBCVParams->m_WC.GetTemp() > pclControlValve->GetTmax() );

		pSelBCV->SetFlag( CSelectedBase::Flags::eNotPriority, true );

		pSelBCV->SetDp( dDp );
		pSelBCV->SetDpHalfOpen( dDpHalfOpen );
		pSelBCV->SetDpFullOpen( dDpFullyOpen );
		pSelBCV->SetH( dH );

		CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclControlValve, pSelBCV );

		bValidFound = ( bValidFound || !pSelBCV->IsFlagSet( CSelectedBase::Flags::eValveGlobal ) );
	}

	if( m_PreselMMapKeyDecreasing.size() > 0 )
	{
		// Phase 0: store highest priority
		// Phase 1: Ordering Key
		// Phase 2: Set flag best Dp
		for( int iPhase = 0; iPhase < 3; iPhase++ )
		{
			for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
			{
				CSelectedBase *pclSelectedProduct = m_cIter->second;
				ASSERT( NULL != pclSelectedProduct );

				if( NULL == pclSelectedProduct )
				{
					continue;
				}

				CSelectedValve *pclSelectedBCV = dynamic_cast<CSelectedValve *>( pclSelectedProduct );
				ASSERT( NULL != pclSelectedBCV );

				if( NULL == pclSelectedBCV )
				{
					continue;
				}

				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct*>( pclSelectedBCV->GetpData() );
				ASSERT( NULL != pTAP );

				if( NULL == pTAP )
				{
					continue;
				}

				int iSizeKey = pTAP->GetSizeKey();

				switch( iPhase )
				{
					// Store highest priority for each product size.
					case 0:

						// 'm_mapPriority' store for each size the minimum priority level (the most important).
						if( pTAP->GetPriorityLevel() < m_mapPriority[iSizeKey] )
						{
							m_mapPriority[iSizeKey] = pTAP->GetPriorityLevel();
						}

						break;

						// Ordering Key.
					case 1:

						// If current selection has the lowest priority level for its size (lowest level means highest priority!)...
						if( m_mapPriority[iSizeKey] == pTAP->GetPriorityLevel() )
						{
							CDB_RegulatingValve *pRV = static_cast<CDB_RegulatingValve *>( pTAP );

							if( NULL == pRV )
							{
								continue;
							}

							// We will choose biggest BCV (biggest Kvs) after we have the possibility to close it.
							double dKvs = pRV->GetKvs();

							if( -1.0 == dKvs )
							{
								continue;
							}

							pclSelectedBCV->SetFlag( CSelectedBase::eNotPriority, false );

							// We will target a Kvs bigger than needed to allow valve re-opening.
							// When Dp is not specified we will return to the minimum allowed Dp (10kPa) no need to reopen the valve
							double dTAFUSIONSecurityFactor = ( false == bIsDpDefined ) ? 1 : 0.8;

							// No security factor for TBV-CM.
							if( false == ( CDB_ControlProperties::eBCVType::ebcvtCharIndep == pRV->GetCtrlProp()->GetBCVType() ||
								CDB_ControlProperties::eBCVType::ebcvtLiftLimit == pRV->GetCtrlProp()->GetBCVType() ) )
							{
								dTAFUSIONSecurityFactor = 1;
							}

							bool bTestIfTheSmallestCurrentValveIsBetter = false;

							// Current valve is smaller than Best valve and the Dp at full opening is lower than Valve min Dp.
							if( false == bIsDpDefined )
							{
								double dValveOffsetDp = ( CDB_ControlProperties::eCvOnOff == pclIndSelBCVParams->m_eCvCtrlType ) ? pclTechParams->GetCVMinDpOnoff()
									: 5000;

								bTestIfTheSmallestCurrentValveIsBetter = ( dKvs < m_rBest.dBestKvs && pclSelectedBCV->GetDpFullOpen() <= ( dValveMinDp + dValveOffsetDp ) );
							}
							else
							{
								bTestIfTheSmallestCurrentValveIsBetter = ( dKvs < m_rBest.dBestKvs && pclSelectedBCV->GetDpFullOpen() <= dValveMinDp );
							}

							if( dTargetKvs <= ( dKvs * dTAFUSIONSecurityFactor ) || true == bTestIfTheSmallestCurrentValveIsBetter )
							{
								double dDelta = dKvs - dTargetKvs;

								if( dDelta < m_rBest.dBestDelta )
								{
									m_rBest.dBestDelta = dDelta;
									m_rBest.dBestKvs = dKvs;
									pclSelectedBCV->SetBestDelta( m_rBest.dBestDelta );
								}
							}
						}
						break;

					// Set flag best Dp.
					case 2:

						// Best product is only into high priority product.
						if( m_mapPriority[iSizeKey] == pTAP->GetPriorityLevel() )
						{
							if( pclSelectedBCV->GetBestDelta() == m_rBest.dBestDelta )
							{
								pclSelectedBCV->SetFlag( CSelectedBase::eBest, true );
							}
						}

						// Transfer valve into the final list with good priority key.
						CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pTAP, pclSelectedBCV );
						break;
				}
			}
		}
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	// HYS-699: If this product can be selected by increasing size above/below in technical parameters, we warn the user.
	if( 0 == m_MMapKeyDecreasing.size() && true == bAtLeastOneFit )
	{
		*pbSizeShiftProblem = true;
	}

	return m_MMapKeyDecreasing.size();
}

////////////////////////////////////////////////////////////////
//	CSelectSmartControlValveList
////////////////////////////////////////////////////////////////
CSelectSmartControlValveList::CSelectSmartControlValveList()
	: CIndividualSelectBaseList()
{
	Clean();
}

int CSelectSmartControlValveList::SelectSmartControlValve( CIndSelSmartControlValveParams *pclIndSelSmartControlValveParams, bool *pbSizeShiftProblem, bool &bBestFound )
{
	bBestFound = false;

	if( NULL == pclIndSelSmartControlValveParams || NULL == pclIndSelSmartControlValveParams->m_pTADS || NULL == pclIndSelSmartControlValveParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSelSmartControlValveParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return 0;
	}

	// Clean all previous data concerning smart control valve selection.
	// Remark: we don't call 'Clean()' because we won't delete the previous pipe selection. It allows us to do a new selection with same pipes.
	if( m_MMapKeyDecreasing.size() > 0 )
	{
		for( m_cIter = m_MMapKeyDecreasing.begin(); m_cIter != m_MMapKeyDecreasing.end(); m_cIter++ )
		{
			if( NULL != m_cIter->second)
			{
				delete m_cIter->second;
			}
		}
		
		m_MMapKeyDecreasing.clear();
	}

	m_PreselMMapKeyDecreasing.clear();
	m_mapPriority.clear();

	if( false == m_clSelectPipeList.GetPipeSizeBound() )
	{
		return 0;
	}

	CDB_Pipe *pSmallestPipe = m_clSelectPipeList.GetpSmallestPipe();

	if( NULL == pSmallestPipe )
	{
		return 0;
	}

	CDB_Pipe *pBiggestPipe = m_clSelectPipeList.GetpLargestPipe();

	if( NULL == pBiggestPipe )
	{
		return 0;
	}

	CDS_TechnicalParameter *pclTechParams = pclIndSelSmartControlValveParams->m_pTADS->GetpTechParams();

	// Setup variables to possibly enforce combined SizeShifts.
	// In case of Hub Selection don't use SizeShift from TechParam.
	if( 99 == pclTechParams->GetSizeShiftAbove() ) // Combined SizeShift +3/-3 enforced
	{
		m_iSizeShiftUp = 3;
		m_iSizeShiftDown = -3;
	}
	else
	{
		m_iSizeShiftUp = pclTechParams->GetSizeShiftAbove();
		m_iSizeShiftDown = pclTechParams->GetSizeShiftBelow();
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclIndSelSmartControlValveParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN = pclTableDN->GetBiggestDN();
	int iHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelSmartControlValveParams->m_pTADB ) + m_iSizeShiftUp );
	int iLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelSmartControlValveParams->m_pTADB ) + m_iSizeShiftDown );

	int iMaxHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelSmartControlValveParams->m_pTADB ) + pclTechParams->GetMaxSizeShift() );
	int iMaxLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelSmartControlValveParams->m_pTADB ) - pclTechParams->GetMaxSizeShift() );

	int iDN15 = pclTableDN->GetSize( _T("DN_15") );

	if( iHighestSize < iDN15 )
	{
		iHighestSize = iDN15;
	}

	if( iMaxHighestSize < iDN15 )
	{
		iMaxHighestSize = iDN15;
	}

	bool bAtLeastOneSizeFit = false;
	bool bAtLeastOneFlowFit = false;
	*pbSizeShiftProblem = true;
	double dDpMaxForBestSuggestion = pclIndSelSmartControlValveParams->m_pTADS->GetpTechParams()->GetSmartValveDpMaxForBestSuggestion();

	_string str;
	LPARAM lpParam;

	for( BOOL bContinue = pclIndSelSmartControlValveParams->m_SmartControlValveList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelSmartControlValveParams->m_SmartControlValveList.GetNext( str, lpParam ) )
	{
		CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( (CData *)lpParam );

		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		CDB_SmartValveCharacteristic *pclSmartValveCharacteristic = pclSmartControlValve->GetSmartValveCharacteristic();
		
		if( NULL == pclSmartValveCharacteristic )
		{
			pclIndSelSmartControlValveParams->m_SmartControlValveList.Delete();
			continue;
		}

		if( pclIndSelSmartControlValveParams->m_dFlow < pclSmartValveCharacteristic->GetMinAdjustableFlow() 
				|| pclIndSelSmartControlValveParams->m_dFlow > pclSmartValveCharacteristic->GetQnom() )
		{
			pclIndSelSmartControlValveParams->m_SmartControlValveList.Delete();
			continue;
		}

		bAtLeastOneFlowFit = true;

		// 'm_mapPriority' stores for each size the minimum priority level (most important).
		if( 0 == m_mapPriority.count( pclSmartControlValve->GetSizeKey() ) )
		{
			m_mapPriority[pclSmartControlValve->GetSizeKey()] = INT_MAX;
		}

		// Do a test to verify if valves exist within the maximum boundaries.
		// If not, the problem is not a sizeShift problem.
		if( pclSmartControlValve->GetSizeKey() < iMaxLowestSize || pclSmartControlValve->GetSizeKey() > iMaxHighestSize )
		{
			continue;
		}

		// Means that at least one smart control valve size is in the maximum boundaries [-4, +4]. If any smart control valve size is in the range 
		// [iLowestSize, iHighestSize], 'SelectSmartControlValve' will return 0 but with 'SizeShiftProblem' set to 'true' we signal that it is possible 
		// to find smart control valve by changing above and below size range and technical parameter.
		if( false == bAtLeastOneSizeFit )
		{
			*pbSizeShiftProblem = true;
		}

		if( pclSmartControlValve->GetSizeKey() < iLowestSize || pclSmartControlValve->GetSizeKey() > iHighestSize )
		{
			continue;
		}

		// There is no problem with the size shift.
		*pbSizeShiftProblem = false;
		bAtLeastOneSizeFit = true;

		// Create a new entry in pre-selection linked list.
		CSelectedValve *pclSelectedSmartControlValve = new CSelectedValve();
		ASSERT( NULL != pclSelectedSmartControlValve );

		if( NULL == pclSelectedSmartControlValve )
		{
			continue;
		}

		pclSelectedSmartControlValve->SetProductIDPtr( pclSmartControlValve->GetIDPtr() );

		double dDp = CalcDp( pclIndSelSmartControlValveParams->m_dFlow, pclSmartControlValve->GetKvs(), pclIndSelSmartControlValveParams->m_WC.GetDens() );
		pclSelectedSmartControlValve->SetDp( dDp );
		
		pclSelectedSmartControlValve->SetFlag( CSelectedBase::Flags::eNotPriority, true );
		CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclSmartControlValve, pclSelectedSmartControlValve );
	}

	// If no result inserted in the 'm_PreselMMapKeyDecreasing' map, we can leave now.
	if( 0 == m_PreselMMapKeyDecreasing.size() )
	{
		// If there is any valve with a good flow, it's not a size problem.
		if( false == bAtLeastOneFlowFit )
		{
			*pbSizeShiftProblem = false;
		}

		return 0;
	}

	double dBestDelta = DBL_MAX;

	for( int iPhase = 0; iPhase < 3; iPhase++ )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedValve *pSelTap = (CSelectedValve *)m_cIter->second;
			ASSERT( NULL != pSelTap );
			
			if( NULL == pSelTap )
			{
				continue;
			}

			CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( pSelTap->GetProductIDPtr().MP );
			ASSERT( NULL != pclSmartControlValve );

			if( NULL == pclSmartControlValve )
			{
				continue;
			}

			CDB_SmartValveCharacteristic *pclSmartValveCharacteristic = pclSmartControlValve->GetSmartValveCharacteristic();

			if( NULL == pclSmartValveCharacteristic )
			{
				continue;
			}

			int iSizeKey = pclSmartControlValve->GetSizeKey();

			switch( iPhase )
			{
				// Store highest priority for each product size.
				case 0:
					
					// If current selection has the lowest value for its size (lowest value means highest priority!)...
					if( pclSmartControlValve->GetPriorityLevel() < m_mapPriority[iSizeKey] )
					{
						m_mapPriority[iSizeKey] = pclSmartControlValve->GetPriorityLevel();
					}
					
					break;

				// Ordering key.
				case 1:
					
					// Best product is only into high priority product.
					if( m_mapPriority[iSizeKey] == pclSmartControlValve->GetPriorityLevel() )
					{
						pSelTap->SetFlag( CSelectedBase::eNotPriority, false);

						// Check the product that is closest of its nominal flow.
						if( pclSmartValveCharacteristic->GetQnom() - pclIndSelSmartControlValveParams->m_dFlow < dBestDelta )
						{
							if( pclSmartControlValve->GetDpMin( pclIndSelSmartControlValveParams->m_dFlow, pclIndSelSmartControlValveParams->m_WC.GetDens() ) <= dDpMaxForBestSuggestion )
							{
								dBestDelta = pclSmartValveCharacteristic->GetQnom() - pclIndSelSmartControlValveParams->m_dFlow;
							}
						}
					}
					break;

				case 2:
					
					// Set flag best flow (Best product is only into high priority product).
					if( m_mapPriority[iSizeKey] == pclSmartControlValve->GetPriorityLevel() )
					{
						if( DBL_MAX != dBestDelta && pclIndSelSmartControlValveParams->m_dFlow < pclSmartValveCharacteristic->GetQnom() 
								&& ( ( pclSmartValveCharacteristic->GetQnom() - pclIndSelSmartControlValveParams->m_dFlow ) == dBestDelta ) )
						{
							pSelTap->SetFlag( CSelectedBase::eBest, true );
							bBestFound = true;
						}
					}

					CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pclSmartControlValve, pSelTap );
					break;
			}
		}
	}
	
	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	return m_MMapKeyDecreasing.size();
}


////////////////////////////////////////////////////////////////
//	CSelectSmartDpCList
// HYS-1937: TA-Smart Dp - 04 - Individual selection: left tab.
////////////////////////////////////////////////////////////////
CSelectSmartDpCList::CSelectSmartDpCList()
	: CIndividualSelectBaseList()
{
	Clean();
}

void CSelectSmartDpCList::Clean()
{
	CIndividualSelectBaseList::Clean();

	m_DpSensorList.PurgeAll();
	m_iSizeShiftUp = 0;
	m_iSizeShiftDown = 0;
	m_dDpToStab = 0.0;
}

int CSelectSmartDpCList::SelectSmartDpC( CIndSelSmartDpCParams *pclIndSelSmartDpCParams, bool *pbSizeShiftProblem, bool &bBestFound )
{
	bBestFound = false;

	if( NULL == pclIndSelSmartDpCParams || NULL == pclIndSelSmartDpCParams->m_pTADS || NULL == pclIndSelSmartDpCParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSelSmartDpCParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return 0;
	}

	// Clean all previous data concerning smart Dp conntroller selection.
	// Remark: we don't call 'Clean()' because we won't delete the previous pipe selection. It allows us to do a new selection with same pipes.
	if( m_MMapKeyDecreasing.size() > 0 )
	{
		for( m_cIter = m_MMapKeyDecreasing.begin(); m_cIter != m_MMapKeyDecreasing.end(); m_cIter++ )
		{
			if( NULL != m_cIter->second)
			{
				delete m_cIter->second;
			}
		}
		
		m_MMapKeyDecreasing.clear();
	}

	m_PreselMMapKeyDecreasing.clear();
	m_mapPriority.clear();

	if( false == m_clSelectPipeList.GetPipeSizeBound() )
	{
		return 0;
	}

	CDB_Pipe *pSmallestPipe = m_clSelectPipeList.GetpSmallestPipe();

	if( NULL == pSmallestPipe )
	{
		return 0;
	}

	CDB_Pipe *pBiggestPipe = m_clSelectPipeList.GetpLargestPipe();

	if( NULL == pBiggestPipe )
	{
		return 0;
	}

	CDS_TechnicalParameter *pclTechParams = pclIndSelSmartDpCParams->m_pTADS->GetpTechParams();

	// Setup variables to possibly enforce combined SizeShifts.
	// In case of Hub Selection don't use SizeShift from TechParam.
	if( 99 == pclTechParams->GetSizeShiftAbove() ) // Combined SizeShift +3/-3 enforced
	{
		m_iSizeShiftUp = 3;
		m_iSizeShiftDown = -3;
	}
	else
	{
		m_iSizeShiftUp = pclTechParams->GetSizeShiftAbove();
		m_iSizeShiftDown = pclTechParams->GetSizeShiftBelow();
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>(pclIndSelSmartDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN = pclTableDN->GetBiggestDN();
	int iHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelSmartDpCParams->m_pTADB ) + m_iSizeShiftUp );
	int iLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelSmartDpCParams->m_pTADB ) + m_iSizeShiftDown );

	int iMaxHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelSmartDpCParams->m_pTADB ) + pclTechParams->GetMaxSizeShift() );
	int iMaxLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelSmartDpCParams->m_pTADB ) - pclTechParams->GetMaxSizeShift() );

	int iDN15 = pclTableDN->GetSize( _T("DN_15") );

	if( iHighestSize < iDN15 )
	{
		iHighestSize = iDN15;
	}

	if( iMaxHighestSize < iDN15 )
	{
		iMaxHighestSize = iDN15;
	}

	bool bAtLeastOneSizeFit = false;
	bool bAtLeastOneFlowFit = false;
	*pbSizeShiftProblem = true;

	// Dp stab on branch.
	m_bIsGroupDpbranchChecked = pclIndSelSmartDpCParams->m_bIsGroupDpbranchChecked;
	m_dDpToStab = pclIndSelSmartDpCParams->m_dDpBranch;
	double dDpMaxForBestSuggestion = pclIndSelSmartDpCParams->m_pTADS->GetpTechParams()->GetSmartValveDpMaxForBestSuggestion();

	_string str;
	LPARAM lpParam;

	for( BOOL bContinue = pclIndSelSmartDpCParams->m_SmartDpCList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelSmartDpCParams->m_SmartDpCList.GetNext( str, lpParam ) )
	{
		CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( (CData *)lpParam );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		CDB_SmartValveCharacteristic *pclSmartValveCharacteristic = pclSmartDpC->GetSmartValveCharacteristic();
		
		if( NULL == pclSmartValveCharacteristic )
		{
			pclIndSelSmartDpCParams->m_SmartDpCList.Delete();
			continue;
		}

		if( pclIndSelSmartDpCParams->m_dFlow < pclSmartValveCharacteristic->GetMinAdjustableFlow()
				|| pclIndSelSmartDpCParams->m_dFlow > pclSmartValveCharacteristic->GetQnom() )
		{
			pclIndSelSmartDpCParams->m_SmartDpCList.Delete();
			continue;
		}

		bAtLeastOneFlowFit = true;

		// 'm_mapPriority' stores for each size the minimum priority level (most important).
		if( 0 == m_mapPriority.count( pclSmartDpC->GetSizeKey() ) )
		{
			m_mapPriority[pclSmartDpC->GetSizeKey()] = INT_MAX;
		}

		// Do a test to verify if valves exist within the maximum boundaries.
		// If not, the problem is not a sizeShift problem.
		if( pclSmartDpC->GetSizeKey() < iMaxLowestSize || pclSmartDpC->GetSizeKey() > iMaxHighestSize )
		{
			continue;
		}

		// Means that at least one smart Dp conntroller size is in the maximum boundaries [-4, +4]. If any smart Dp conntroller size is in the range 
		// [iLowestSize, iHighestSize], 'SelectSmartDpC' will return 0 but with 'SizeShiftProblem' set to 'true' we signal that it is possible 
		// to find smart control valve by changing above and below size range and technical parameter.
		if( false == bAtLeastOneSizeFit )
		{
			*pbSizeShiftProblem = true;
		}

		if( pclSmartDpC->GetSizeKey() < iLowestSize || pclSmartDpC->GetSizeKey() > iHighestSize )
		{
			continue;
		}

		// There is no problem with the size shift.
		*pbSizeShiftProblem = false;
		bAtLeastOneSizeFit = true;

		// Create a new entry in pre-selection linked list.
		CSelectedValve *pclSelectedSmartDpC = new CSelectedValve();
		ASSERT( NULL != pclSelectedSmartDpC );

		if( NULL == pclSelectedSmartDpC )
		{
			continue;
		}

		pclSelectedSmartDpC->SetProductIDPtr( pclSmartDpC->GetIDPtr() );

		if( true == pclIndSelSmartDpCParams->m_bIsDpMaxChecked && pclIndSelSmartDpCParams->m_dDpMax > pclSmartDpC->GetDpmax() )
		{
			pclSelectedSmartDpC->SetFlag( CSelectedBase::Flags::eDp, true );
		}

		if( pclIndSelSmartDpCParams->m_WC.GetTemp() > pclSmartDpC->GetTmax()
				|| pclIndSelSmartDpCParams->m_WC.GetTemp() < pclSmartDpC->GetTmin() )
		{
			pclSelectedSmartDpC->SetFlag( CSelectedBase::Flags::eTemperature, true );
		}
		
		pclSelectedSmartDpC->SetFlag( CSelectedBase::Flags::eNotPriority, true );
		CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclSmartDpC, pclSelectedSmartDpC );
	}

	// If no result inserted in the 'm_PreselMMapKeyDecreasing' map, we can leave now.
	if( 0 == m_PreselMMapKeyDecreasing.size() )
	{
		// If there is any valve with a good flow, it's not a size problem.
		if( false == bAtLeastOneFlowFit )
		{
			*pbSizeShiftProblem = false;
		}

		return 0;
	}

	double dBestDelta = DBL_MAX;

	for( int iPhase = 0; iPhase < 3; iPhase++ )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedValve *pSelTap = (CSelectedValve *)m_cIter->second;
			ASSERT( NULL != pSelTap );
			
			if( NULL == pSelTap )
			{
				continue;
			}

			CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( pSelTap->GetProductIDPtr().MP );
			ASSERT( NULL != pclSmartDpC );

			if( NULL == pclSmartDpC )
			{
				continue;
			}

			CDB_SmartValveCharacteristic *pclSmartValveCharacteristic = pclSmartDpC->GetSmartValveCharacteristic();

			if( NULL == pclSmartValveCharacteristic )
			{
				continue;
			}

			int iSizeKey = pclSmartDpC->GetSizeKey();

			switch( iPhase )
			{
				// Store highest priority for each product size.
				case 0:
					
					// If current selection has the lowest value for its size (lowest value means highest priority!)...
					if( pclSmartDpC->GetPriorityLevel() < m_mapPriority[iSizeKey] )
					{
						m_mapPriority[iSizeKey] = pclSmartDpC->GetPriorityLevel();
					}
					
					break;

				// Ordering key.
				case 1:
					
					// Best product is only into high priority product.
					if( m_mapPriority[iSizeKey] == pclSmartDpC->GetPriorityLevel() )
					{
						pSelTap->SetFlag( CSelectedBase::eNotPriority, false);

						// Check the product that is closest of its nominal flow.
						if( pclSmartValveCharacteristic->GetQnom() - pclIndSelSmartDpCParams->m_dFlow < dBestDelta )
						{
							if( pclSmartDpC->GetDpMin( pclIndSelSmartDpCParams->m_dFlow, pclIndSelSmartDpCParams->m_WC.GetDens() ) <= dDpMaxForBestSuggestion )
							{
								dBestDelta = pclSmartValveCharacteristic->GetQnom() - pclIndSelSmartDpCParams->m_dFlow;
							}
						}
					}
					break;

				case 2:
					
					// Set flag best flow (Best product is only into high priority product).
					if( m_mapPriority[iSizeKey] == pclSmartDpC->GetPriorityLevel() )
					{
						if( DBL_MAX != dBestDelta
								&& ( ( pclSmartValveCharacteristic->GetQnom() - pclIndSelSmartDpCParams->m_dFlow ) == dBestDelta ) )
						{
							pSelTap->SetFlag( CSelectedBase::eBest, true );
							bBestFound = true;
						}
					}

					CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pclSmartDpC, pSelTap );
					break;
			}
		}
	}
	
	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	return m_MMapKeyDecreasing.size();
}

int CSelectSmartDpCList::SelectDpSensorSet( CIndSelSmartDpCParams *pclIndSelSmartDpCParams, std::set<CDB_Set *> *pSet, CSelectedValve *pSelSmartDpC )
{
	if( NULL == pclIndSelSmartDpCParams || NULL == pclIndSelSmartDpCParams->m_pTADS || NULL == pclIndSelSmartDpCParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSelSmartDpCParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( NULL == pSet || 0 == pSet->size() || NULL == pSelSmartDpC )
	{
		return 0;
	}

	if( NULL != GetDpSensorSelected() )
	{
		m_DpSensorList.PurgeAll();
	}

	// Add Dp sensor products linked to SmartDpC in a rank list.
	for( std::set<CDB_Set *>::iterator iter = pSet->begin(); iter != pSet->end(); ++iter )
	{
		CDB_Product *pclProductSet = dynamic_cast<CDB_Product *>( (*iter)->GetSecondIDPtr().MP );

		double dKey = (double)pclProductSet->GetSortingKey();

		if( NULL != dynamic_cast<CDB_DpSensor *>( pclProductSet ) )
		{
			CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclProductSet );

			ASSERT( ( pclDpSensor->GetMinMeasurableDp() / 1000.0 ) < 100.0 );
			dKey = dKey * 100.0 + ( pclDpSensor->GetMinMeasurableDp() / 1000.0 );
		}
		else
		{
			dKey += 1e9;
		}

		m_DpSensorList.Add( pclProductSet->GetName(), dKey, (LPARAM)( (*iter)->GetSecondIDPtr().MP) );
	}

	return m_DpSensorList.GetCount();
}

int CSelectSmartDpCList::SelectDpSensor( CIndSelSmartDpCParams *pclIndSelSmartDpCParams, CTable* pTableSensorGr, CSelectedValve *pSelSmartDpC )
{
	if( NULL == pclIndSelSmartDpCParams || NULL == pclIndSelSmartDpCParams->m_pTADS || NULL == pclIndSelSmartDpCParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSelSmartDpCParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( NULL == pSelSmartDpC )
	{
		return 0;
	}

	if( NULL != GetDpSensorSelected() )
	{
		m_DpSensorList.PurgeAll();
	}

	// Add Dp sensor products linked to SmartDpC in a rank list.
	for( IDPTR idptr = pTableSensorGr->GetFirst(); _T( '\0' ) != *idptr.ID; idptr = pTableSensorGr->GetNext() )
	{
		CDB_Product *pclProductSet = dynamic_cast<CDB_Product *>(idptr.MP );

		double dKey = (double)pclProductSet->GetSortingKey();

		if( NULL != dynamic_cast<CDB_DpSensor *>( pclProductSet ) )
		{
			CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclProductSet );

			ASSERT( ( pclDpSensor->GetMinMeasurableDp() / 1000.0 ) < 100.0 );
			dKey = dKey * 100.0 + ( pclDpSensor->GetMinMeasurableDp() / 1000.0 );
		}
		else
		{
			dKey += 1e9;
		}

		m_DpSensorList.Add( pclProductSet->GetName(), dKey, (LPARAM)(idptr.MP) );
	}

	return m_DpSensorList.GetCount();
}

////////////////////////////////////////////////////////////////
//	CSelectDpCList
////////////////////////////////////////////////////////////////
CSelectDpCList::CSelectDpCList()
	: CSelectList()
{
	m_pclIndSelBVParams = NULL;
	Clean();
}

CSelectDpCList::~CSelectDpCList()
{
	if( NULL != m_pclIndSelBVParams )
	{
		if( NULL != m_pclIndSelBVParams->m_pclSelectBvList )
		{
			delete m_pclIndSelBVParams->m_pclSelectBvList;
			m_pclIndSelBVParams->m_pclSelectBvList = NULL;
		}

		delete m_pclIndSelBVParams;
	}
}

void CSelectDpCList::Clean()
{
	CSelectList::Clean();

	if( NULL != m_pclIndSelBVParams )
	{
		if( NULL != m_pclIndSelBVParams->m_pclSelectBvList )
		{
			delete m_pclIndSelBVParams->m_pclSelectBvList;
		}

		delete m_pclIndSelBVParams;
	}

	m_pclIndSelBVParams = NULL;
	m_iSizeShiftUp = 0;
	m_iSizeShiftDown = 0;
	m_dDpToStab = 0.0;
	m_dMinRequiredDpOnMv = DBL_MAX;
}

// Pipe Selection must be done before DpC selection
int CSelectDpCList::SelectDpC( CIndSelDpCParams *pclIndSelDpCParams, bool &bValidFound, bool bTestDplmin )
{
	if( NULL == pclIndSelDpCParams || NULL == pclIndSelDpCParams->m_pTADS || NULL == pclIndSelDpCParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSelDpCParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return 0;
	}

	// Get pointers on required database tables
	CTable *pTab = (CTable*)( pclIndSelDpCParams->m_pTADB->Get( _T("DPCONTR_TAB") ).MP );

	if( NULL == pTab )
	{
		ASSERTA_RETURN( 0 );
	}

	if( false == m_clSelectPipeList.GetPipeSizeBound() )
	{
		return 0;
	}

	CDB_Pipe *pSmallestPipe = m_clSelectPipeList.GetpSmallestPipe();

	if( NULL == pSmallestPipe )
	{
		return 0;
	}

	CDB_Pipe *pBiggestPipe = m_clSelectPipeList.GetpLargestPipe();

	if( NULL == pBiggestPipe )
	{
		return 0;
	}

	CDS_TechnicalParameter *pclTechParams = pclIndSelDpCParams->m_pTADS->GetpTechParams();

	// Setup variables to possibly enforce combined SizeShifts
	// In case of Hub Selection don't use SizeShift from TechParam
	if( 99 == pclTechParams->GetSizeShiftAbove() ) // Combined SizeShift +3/-3 enforced
	{
		m_iSizeShiftUp = 3;
		m_iSizeShiftDown = -3;
	}
	else
	{
		m_iSizeShiftUp = pclTechParams->GetSizeShiftAbove();
		m_iSizeShiftDown = pclTechParams->GetSizeShiftBelow();
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclIndSelDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN = pclTableDN->GetBiggestDN();
	int iHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelDpCParams->m_pTADB ) + m_iSizeShiftUp );
	int iLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelDpCParams->m_pTADB ) + m_iSizeShiftDown );

	// Get PN pressure 
	double dPpn = 0.0;

	if( false == pclIndSelDpCParams->m_strComboPNID.IsEmpty() )
	{
		dPpn = _tcstod( ( (CDB_StringID*)( pclIndSelDpCParams->m_pTADB->Get( pclIndSelDpCParams->m_strComboPNID ).MP ) )->GetIDstr(), '\0' );
	}

	int iDN15 = pclTableDN->GetSize( _T("DN_15") );

	if( iHighestSize < iDN15 )
	{
		iHighestSize = iDN15;
	}
	
	m_rBest.dBestDpAboveMin2 = DBL_MAX;
	m_rBest.dBestDpBelowMin = 0.0;
	m_rBest.dBestDp = 0.0;
	m_rBest.dBestDelta = DBL_MAX;

	// Prepare an array with a entry for each Dp controller valve size.
	_string str;
	LPARAM lpParam;
	
	for( BOOL bContinue = pclIndSelDpCParams->m_DpCList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelDpCParams->m_DpCList.GetNext( str, lpParam ) )
	{
		CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( (CData *)lpParam );
		ASSERT( NULL != pclDpController );

		if( NULL == pclDpController )
		{
			continue;
		}

		int iSizeKey = pclDpController->GetSizeKey();
		
		// If entry doesn't yet exist, create it and set INT_MAX value. The good values will be set after below.
		if( 0 == m_mapPriority.count( iSizeKey ) )
		{
			m_mapPriority[ iSizeKey ] = INT_MAX;
		}
	}

	bValidFound = false;

	for( BOOL bContinue = pclIndSelDpCParams->m_DpCList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelDpCParams->m_DpCList.GetNext( str, lpParam ) )
	{
		CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( (CData *)lpParam );
		ASSERT( NULL != pclDpController );
		
		if( NULL == pclDpController )
		{
			continue;
		}

		// Get Kv characteristic of the valve.
		CDB_DpCCharacteristic *pclDpCCharacteristic = pclDpController->GetDpCCharacteristic();
		ASSERT( NULL != pclDpCCharacteristic );

		if( NULL == pclDpCCharacteristic )
		{
			continue;
		}

		if( false == pclDpController->IsSelectable( true ) )
		{
			continue;
		}

		if( 0.0 != dPpn && false == pclDpController->IsPNIncludedInPNRange( dPpn ) )
		{
			continue;
		}

		if( pclDpController->GetSizeKey() < iLowestSize || pclDpController->GetSizeKey() > iHighestSize )
		{
			continue;
		}

		// Find min and max Dpl.
		double dDplmin = pclDpController->GetDplmin();
		double dDplmax = pclDpController->GetDplmax();
		
		if( -1.0 == dDplmin || -1.0 == dDplmax )
		{
			continue;
		}

		// Initialize bounds on Dpl (to be adjusted depending on MvLoc).
		double dDplLowBound = dDplmin;
		double dDplHighBound = dDplmax;

		m_dDpToStab = 0.0;
		double dKvCv = 0.0;
		double dReqDpMv = pclTechParams->GetValvMinDp();
		int iDplTooSmall = 0;
		int iDplTooLarge = 0;

		if( true == pclIndSelDpCParams->m_bIsGroupDpbranchOrKvChecked )
		{
			// Calculate the DpToStab.
			if( eDpStab::DpStabOnCV == pclIndSelDpCParams->m_eDpStab )
			{
				// Dp stab on control valve.
				m_dDpToStab = CalcDp( pclIndSelDpCParams->m_dFlow, pclIndSelDpCParams->m_dKv, pclIndSelDpCParams->m_WC.GetDens() );
			}
			else
			{
				// Dp stab on branch.
				m_dDpToStab = pclIndSelDpCParams->m_dDpBranch;
			}

			// If DpC is on CV, we must be sure when lowering Dp min we don't change CV minimal authority!!
			if( eDpStab::DpStabOnCV == pclIndSelDpCParams->m_eDpStab )
			{
				if( true == pclDpCCharacteristic->IsProportionalBandAbsolute() )
				{
					dDplLowBound = pclTechParams->GetCVMinAuthor() * ( dDplLowBound + pclDpCCharacteristic->GetProportionalBand( m_dDpToStab ) );
				}
				else
				{
					dDplLowBound *= pclTechParams->GetCVMinAuthor() * ( 1.0 + pclDpCCharacteristic->GetProportionalBand( m_dDpToStab ) );
				}
			}
			
			dDplHighBound -= pclTechParams->GetValvMinDp();

			// 0 : NO problemo.
			// 1 : There is one problem but one alternative solution exist.
			// 2 : There is no solution.
			// Determine the Dpl validity.
			if( true == bTestDplmin )
			{
				if( m_dDpToStab < dDplmin )
				{
					// Bv at primary (Excluded from the stabilization), there is no way to change the dp to stabilize.
					// Bv at seconday (Included into the stabilization), the BV can increase the Dp to stabilize to reach the Dplmin.
					iDplTooSmall = ( eMvLoc::MvLocPrimary == pclIndSelDpCParams->m_eMvLoc ) ? 1 : 0;
				}
			}

			if( ( dDplLowBound != dDplmin ) && ( m_dDpToStab < dDplLowBound ) ) // No alternative.
			{
				iDplTooSmall = 2;
			}

			// Compute required Dp needed in valve to compensate.
			if( eMvLoc::MvLocSecondary == pclIndSelDpCParams->m_eMvLoc )
			{
				dReqDpMv = max( pclTechParams->GetValvMinDp(), dDplmin - m_dDpToStab );
			}

			if( m_dDpToStab > dDplHighBound )
			{
				// 'dDplHighBound' is the 'Dplmax' minus the valve min dp. (3 kPa).
				// Because we have the condition "m_dDpToStab > dDplmax" after, we can imagine that here we have "m_dDpToStab > dDplHighBound" 
				// AND "m_dDpToStab <= dDplmax". 
				// Let's take an example: BV at secondary side (Included in the stabilization) set  to its min. dp = 3 kPa. 
				// "dDplHighBound" = "Dplmax" - 3 = 17 kPa with a "Dplmax" = 20 kPa.
				// It means that a stabilization below 17 kPa is possible.
				// Now, let's take value of stabilization between "dDplHighBound" (17 kPa) and "Dplmax" (20 kPa) for example 18 kPa.
				// Dpl (18 kPa) + BV Dp (3 kPa) = 21 kPa -> With a dplmax at 20 kPa it's impossible.
				// EXCEPT if in this case, we put the BV at primary side. In this case Dpl becomes 18 kPa and is below the Dpmax.
				iDplTooLarge = ( eMvLoc::MvLocSecondary == pclIndSelDpCParams->m_eMvLoc ) ? 1 : 0;
			}

			if( m_dDpToStab > dDplmax )  // No alternative.
			{
				iDplTooLarge = 2;
			}
		}
					
		// Compute min and max q.
		// Qmin depends on DH-Dpl and the selection gives Hmin,
		// so:	qmin taken as qmin for DpToStab
		double dQMin = pclDpCCharacteristic->GetQmin( pclIndSelDpCParams->m_WC.GetDens(), ( true == pclIndSelDpCParams->m_bIsGroupDpbranchOrKvChecked ) 
				? m_dDpToStab : 0 );
		
		// Qmax depends on DH-Dpl and the selection gives Hmin,
		// so:	qmax taken as the max of qmax
		double dQMax = pclDpCCharacteristic->GetQmax( pclIndSelDpCParams->m_WC.GetDens() );

		double dDpmin = 0.0;
		bool bDpminFound = false;
		
		if( pclIndSelDpCParams->m_dFlow >= dQMin && pclIndSelDpCParams->m_dFlow <= dQMax )				// q larger than qmin AND q smaller than qmax
		{
			// Compute the minimum required differential pressure.
			pclDpCCharacteristic->DpCSolve( pclIndSelDpCParams->m_dFlow, &dDpmin, pclIndSelDpCParams->m_WC.GetDens() ); 
			bDpminFound = true;

			// Create a new entry in pre-selection linked list
			CSelectedValve *pSelDpC = new CSelectedValve();

			if( NULL == pSelDpC )
			{
				ASSERTA_RETURN( 0 );
			}

			pSelDpC->SetProductIDPtr( pclDpController->GetIDPtr() );

			// Fill the fields
			pSelDpC->SetDpMin( dDpmin );
			pSelDpC->SetRequiredDpMv( dReqDpMv );
			pSelDpC->SetFlag( CSelectedBase::Flags::eValveDpMin, ( pclIndSelDpCParams->m_dFlow > dQMax || false == bDpminFound ) );
			pSelDpC->SetFlag( CSelectedBase::Flags::eValveDpToSmall, ( 0 == iDplTooSmall ) ? false : true );
			pSelDpC->SetFlag( CSelectedBase::Flags::eValveDpToLarge, ( 0 == iDplTooLarge ) ? false : true );
			pSelDpC->SetFlag( CSelectedBase::Flags::eValveDpAlternative, ( 1 == iDplTooSmall || 1 == iDplTooLarge ) );
			pSelDpC->SetFlag( CSelectedBase::Flags::ePN, dPpn > pclDpController->GetPmaxmax() );

			pSelDpC->SetFlag( CSelectedBase::Flags::eTemperature, pclIndSelDpCParams->m_WC.GetTemp() < pclDpController->GetTmin() 
					|| pclIndSelDpCParams->m_WC.GetTemp() > pclDpController->GetTmax() );
			
			pSelDpC->SetFlag( CSelectedBase::Flags::eValveDpCHide, ( pclTechParams->GetDpCHide() && ( dDpmin < pclTechParams->GetDpCMinDp() ) ) );
			pSelDpC->SetFlag( CSelectedBase::Flags::eBest, false );
			pSelDpC->SetFlag( CSelectedBase::Flags::eNotPriority, true );

			// Add the valve in the pre-selection list
			// Only if it's fully valid or if there is none fully valid valve already into this pre-selection list
			if(	false == pSelDpC->IsFlagSet( CSelectedBase::Flags::eValveDpCHide ) )
			{
				CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclDpController, pSelDpC );
			}
			else
			{
				delete pSelDpC;
			}
			
			bValidFound = ( true == bValidFound || false == pSelDpC->IsFlagSet( CSelectedBase::Flags::eValveGlobal ) );
		}
	}

	if( true == bValidFound )
	{
		CleanPreselectionList( pclIndSelDpCParams );
	}
	
	sBest Best;
	double dTechParamDpCMinDp = pclTechParams->GetDpCMinDp();
	
	for( int iPhase = 0; iPhase < 3; iPhase++ )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedValve *pSelTap = (CSelectedValve *)m_cIter->second;
			ASSERT( NULL != pSelTap );

			if( NULL == pSelTap )
			{
				continue;
			}

			CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct*>( pSelTap->GetpData() );
			ASSERT( NULL != pTAP );

			if( NULL == pTAP )
			{
				continue;
			}

			int iSizeKey = pTAP->GetSizeKey();
			
			switch( iPhase )
			{
				// Store highest priority for each product size.
				case 0:	
			
					// 'm_mapPriority' store for each size the minimum priority level (the most important).
					if( pTAP->GetPriorityLevel() < m_mapPriority[iSizeKey] )
					{
						m_mapPriority[iSizeKey] = pTAP->GetPriorityLevel();
					}
					
					break;

				// Ordering Key.
				case 1:
					
					// If current selection has the lowest value for its size (lowest value means highest priority!)...
					if( m_mapPriority[iSizeKey] == pTAP->GetPriorityLevel() )
					{
						pSelTap->SetFlag( CSelectedBase::eNotPriority, false );
						
						// Determine BestDpmin - smallest Dpmin above DpCMinDp.
						if( pSelTap->GetDpMin() > dTechParamDpCMinDp )
						{
							Best.dBestDelta = min( pSelTap->GetDpMin(), Best.dBestDelta );
						}
					}
					
					// Transfer valve into the final list with good priority key.
					CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pTAP, pSelTap );
					
					break;

				case 2:

					if( pSelTap->GetDpMin() == Best.dBestDelta && false == pSelTap->GetFlag( CSelectedBase::eNotPriority ) )
					{
						pSelTap->SetFlag( CSelectedBase::Flags::eBest, true );
					}

					break;
			}
		}
	}
	
	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	return m_MMapKeyDecreasing.size();
}

int CSelectDpCList::SelectManBV( CIndSelDpCParams *pclIndSelDpCParams, CSelectedValve *pSelDpC )
{
	if( NULL == pclIndSelDpCParams || NULL == pclIndSelDpCParams->m_pTADS || NULL == pclIndSelDpCParams->m_pTADS->GetpTechParams()
		|| NULL == pclIndSelDpCParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( NULL == pSelDpC )
	{
		return 0;
	}

	CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( pSelDpC->GetpData() );

	if( NULL == pclDpController ) 
	{
		return -1;
	}
	
	if( NULL != m_pclIndSelBVParams )
	{
		if( NULL != m_pclIndSelBVParams->m_pclSelectBvList )
		{
			delete m_pclIndSelBVParams->m_pclSelectBvList;
			m_pclIndSelBVParams->m_pclSelectBvList = NULL;
		}

		delete m_pclIndSelBVParams;
		m_pclIndSelBVParams = NULL;
	}

	m_pclIndSelBVParams = new CIndSelBVParams();

	if( NULL == m_pclIndSelBVParams )
	{
		ASSERTA_RETURN( 0 );
	}

	m_pclIndSelBVParams->CopyFrom( pclIndSelDpCParams );
	m_pclIndSelBVParams->m_pclSelectBvList = new CSelectList();
	
	if( NULL == m_pclIndSelBVParams->m_pclSelectBvList || NULL == m_pclIndSelBVParams->m_pclSelectBvList->GetSelectPipeList() )
	{
		return 0;
	}
	
	m_pclIndSelBVParams->m_pclSelectBvList->GetSelectPipeList()->SelectPipes( pclIndSelDpCParams, pclIndSelDpCParams->m_dFlow );

	// Pre-select measuring valve.
	// Remark: Here the table is not a list of regulating valve objects but well a list of 'CDB_StringID' objects that enumerates
	//         list of all regulating valve families compatible with the Dp controller valves.
	CTable *pTab = (CTable*)( pclIndSelDpCParams->m_pTADB->Get( _T("MEASVALV_TAB") ).MP );

	if( NULL == pTab )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDNMin = 0;
	int iDNMax = INT_MAX;
	CDS_TechnicalParameter *pclTechParams = pclIndSelDpCParams->m_pTADS->GetpTechParams();

	// If the user want a MV selection using same DpC size, remove unneeded valve from BvList
	if( true == ( pclTechParams->GetDpCMvWithSameSizeOnly() != 0 ) ? true : false )
	{
		iDNMin = pclDpController->GetSizeKey();
		iDNMax = iDNMin;
	}
	
	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_StringID *pStrID = (CDB_StringID*)( IDPtr.MP );
		
		// Reject STAM
		CString strFamilyID = pStrID->GetIDstr();

		if( _T("FAM_STAM") == strFamilyID )
		{
			continue;
		}
		
		CDB_StringID *pStrFamilyID = (CDB_StringID*)( pclIndSelDpCParams->m_pTADB->Get( pStrID->GetIDstr() ).MP );
		CString strVersionID = pStrID->GetString();
		
		pclIndSelDpCParams->m_pTADB->GetBVList( &m_pclIndSelBVParams->m_BvList , pStrFamilyID->GetIDstr2(), pStrID->GetIDstr(), _T(""), _T(""), (LPCTSTR)strVersionID, 
				pclIndSelDpCParams->m_eFilterSelection, iDNMin, iDNMax );
	}
	
	_SearchBv( pclIndSelDpCParams, pclDpController, pSelDpC->GetRequiredDpMv() );

	return m_pclIndSelBVParams->m_pclSelectBvList->GetCount();
}

int CSelectDpCList::SelectManBVSet( CIndSelDpCParams *pclIndSelDpCParams, std::set<CDB_Set *> *pSet, CSelectedValve *pSelDpC )
{
	if( NULL == pclIndSelDpCParams || NULL == pclIndSelDpCParams->m_pTADS || NULL == pclIndSelDpCParams->m_pTADS->GetpTechParams()
		|| NULL == pclIndSelDpCParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( NULL == pSet || 0 == pSet->size() || NULL == pSelDpC )
	{
		return 0;
	}

	if( NULL != m_pclIndSelBVParams )
	{
		if( NULL != m_pclIndSelBVParams->m_pclSelectBvList )
		{
			delete m_pclIndSelBVParams->m_pclSelectBvList;
			m_pclIndSelBVParams->m_pclSelectBvList = NULL;
		}

		delete m_pclIndSelBVParams;
		m_pclIndSelBVParams = NULL;
	}

	m_pclIndSelBVParams = new CIndSelBVParams();

	if( NULL == m_pclIndSelBVParams )
	{
		ASSERTA_RETURN( 0 );
	}

	m_pclIndSelBVParams->CopyFrom( pclIndSelDpCParams );
	m_pclIndSelBVParams->m_pclSelectBvList = new CSelectList();

	if( NULL == m_pclIndSelBVParams->m_pclSelectBvList || NULL == m_pclIndSelBVParams->m_pclSelectBvList->GetSelectPipeList() )
	{
		ASSERTA_RETURN( 0 );
	}

	m_pclIndSelBVParams->m_pclSelectBvList->GetSelectPipeList()->SelectPipes( pclIndSelDpCParams, pclIndSelDpCParams->m_dFlow );

	// Add balancing valves linked to DpC in a rank list.
	for( std::set<CDB_Set*>::iterator iter = pSet->begin(); iter != pSet->end(); ++iter )
	{
		CDB_TAProduct *pTAProduct = dynamic_cast<CDB_TAProduct *>( (*iter)->GetSecondIDPtr().MP );
		
		if( NULL == pTAProduct )
		{
			continue;
		}

		double dKey = (double)pTAProduct->GetSortingKey();
		CData *pValveChar = pTAProduct->GetValveCharDataPointer();
		
		if( NULL != pValveChar )
		{
			m_pclIndSelBVParams->m_BvList.Add( pValveChar->GetIDPtr().ID, dKey, (LPARAM)( (*iter)->GetSecondIDPtr().MP ) );
		}
	}

	CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( (CData*)(*pSet->begin())->GetFirstIDPtr().MP );
	
	if( NULL == pclDpController ) 
	{
		return 0;
	}
	
	_SearchBv( pclIndSelDpCParams, pclDpController, pSelDpC->GetRequiredDpMv() );

	return m_pclIndSelBVParams->m_pclSelectBvList->GetCount();
}

CSelectList *CSelectDpCList::GetBvSelected()
{
	if( NULL == m_pclIndSelBVParams || NULL == m_pclIndSelBVParams->m_pclSelectBvList )
	{
		return NULL;
	}

	return m_pclIndSelBVParams->m_pclSelectBvList;
}

void CSelectDpCList::_SearchBv( CIndSelDpCParams *pclIndSelDpCParams, CDB_DpController *pclDpController, double dRequiredDpOnMv )
{
	if( NULL == m_pclIndSelBVParams || NULL == m_pclIndSelBVParams->m_pclSelectBvList )
	{
		ASSERT_RETURN;
	}

	CDS_TechnicalParameter *pclTechParams = pclIndSelDpCParams->m_pTADS->GetpTechParams();

	if( eMvLoc::MvLocSecondary == pclIndSelDpCParams->m_eMvLoc 
			&& pclDpController->GetDplmax() > ( m_dDpToStab + pclTechParams->GetValvMin2Dp() ) 
			&& m_dDpToStab > pclDpController->GetDplmin() )
	{
		dRequiredDpOnMv = 0.0;
	}
	
	// If Bv is in secondary and user has not set a Dp branch, we have no idea of the Bv max Dp. Thus we can launch
	// Bv selection with 'GetValvMinDp' criterion (in technical parameter).
	if( eMvLoc::MvLocSecondary == pclIndSelDpCParams->m_eMvLoc && 0.0 == m_dDpToStab )
	{
		dRequiredDpOnMv = 0.0;
	}
	
	// If Bv is locate on primary, we can launch Bv selection with 'GetValvMinDp' criterion (in technical parameter).
	if( eMvLoc::MvLocPrimary == pclIndSelDpCParams->m_eMvLoc )
	{
		dRequiredDpOnMv = 0.0;
	}

	m_pclIndSelBVParams->m_pclSelectBvList->SetMvLoc( pclIndSelDpCParams->m_eMvLoc );
	m_pclIndSelBVParams->m_dDp = dRequiredDpOnMv;

	m_pclIndSelBVParams->m_pclSelectBvList->GetManBvList( m_pclIndSelBVParams );

	for( ProductMMapKeyDecreasing::const_iterator cIter = m_pclIndSelBVParams->m_pclSelectBvList->GetpValveMap()->begin(); 
			cIter != m_pclIndSelBVParams->m_pclSelectBvList->GetpValveMap()->end(); )
	{
		if( NULL == cIter->second )
		{
			++cIter;
			continue;
		}

		CSelectedValve *pSelectedTAP = (CSelectedValve *)( cIter->second );
		
		if(	NULL == pSelectedTAP ) 
		{
			++cIter;
			continue;
		}
	
		// If Bv is locate on primary, we don't need to verify if its Dp is valid or not.
		// If Bv is locate on secondary, we need to verify if its Dp is not bigger than the max Dp the controller can control.
		// Ex: if 'DplMax' is 80KPa and 'm_DpToStab' is 75KPa, Dp for Bv can't be bigger than 5KPa!
		if( eMvLoc::MvLocSecondary == pclIndSelDpCParams->m_eMvLoc 
				&& pSelectedTAP->GetDp() > ( pclDpController->GetDplmax() - m_dDpToStab ) )
		{
			ProductMMapKeyDecreasing::const_iterator pNext = cIter;
			++pNext;
			delete cIter->second;
			m_pclIndSelBVParams->m_pclSelectBvList->GetpValveMap()->erase( cIter );
			cIter = pNext;
		}
		else
		{
			++cIter;
		}
	}
}

////////////////////////////////////////////////////////////////
//	CSelectDpCBCVList
////////////////////////////////////////////////////////////////
CSelectDpCBCVList::CSelectDpCBCVList()
	: CSelectCtrlList()
{
	m_pSelectedSvList = NULL;
	Clean();
}

CSelectDpCBCVList::~CSelectDpCBCVList()
{
	if( NULL != m_pSelectedSvList )
	{
		delete m_pSelectedSvList;
	}
}

void CSelectDpCBCVList::Clean()
{
	CSelectCtrlList::Clean();

	if( NULL != m_pSelectedSvList )
	{
		delete m_pSelectedSvList;
	}

	m_pSelectedSvList = NULL;

	m_iSizeShiftUp = 0;
	m_iSizeShiftDown = 0;
}

// Pipe Selection must be done before DpC selection
int CSelectDpCBCVList::SelectDpCBCV( CIndSelDpCBCVParams *pclIndSelDpCBCVParams, bool *pbValidFound, bool *pbSizeShiftProblem, bool bTestDplmin )
{
	if( NULL == pclIndSelDpCBCVParams || NULL == pclIndSelDpCBCVParams->m_pTADS || NULL == pclIndSelDpCBCVParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSelDpCBCVParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}
	
	if( NULL == pbValidFound || NULL == pbSizeShiftProblem )
	{
		return 0;
	}

	if( pclIndSelDpCBCVParams->m_dFlow <= 0.0 )
	{
		return 0;
	}

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return 0;
	}

	CTable *pTab = (CTable *)( pclIndSelDpCBCVParams->m_pTADB->Get( _T("DPCBALCTRLVALV_TAB") ).MP );

	if( NULL == pTab )
	{
		ASSERTA_RETURN( 0 );
	}

	if( false == m_clSelectPipeList.GetPipeSizeBound() )
	{
		return 0;
	}

	CDB_Pipe *pSmallestPipe = m_clSelectPipeList.GetpSmallestPipe();

	if( NULL == pSmallestPipe )
	{
		return 0;
	}

	CDB_Pipe *pBiggestPipe = m_clSelectPipeList.GetpLargestPipe();

	if( NULL == pBiggestPipe )
	{
		return 0;
	}
	
	CDS_TechnicalParameter *pclTechParams = pclIndSelDpCBCVParams->m_pTADS->GetpTechParams();

	// Setup variables to possibly enforce combined SizeShifts
	// In case of Hub Selection don't use SizeShift from TechParam
	if( 99 == pclTechParams->GetSizeShiftAbove() ) // Combined SizeShift +3/-3 enforced
	{
		m_iSizeShiftUp = 3;
		m_iSizeShiftDown = -3;
	}
	else
	{
		m_iSizeShiftUp = pclTechParams->GetSizeShiftAbove();
		m_iSizeShiftDown = pclTechParams->GetSizeShiftBelow();
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclIndSelDpCBCVParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN = pclTableDN->GetBiggestDN();
	int iHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelDpCBCVParams->m_pTADB ) + m_iSizeShiftUp );
	int iLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelDpCBCVParams->m_pTADB ) + m_iSizeShiftDown );

	int iMaxHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelDpCBCVParams->m_pTADB ) + pclTechParams->GetMaxSizeShift() );
	int iMaxLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelDpCBCVParams->m_pTADB ) - pclTechParams->GetMaxSizeShift() );

	// Get PN pressure.
	double dPpn = 0.0;

	if( false == pclIndSelDpCBCVParams->m_strComboPNID.IsEmpty() )
	{
		dPpn = _tcstod( ( (CDB_StringID*)( pclIndSelDpCBCVParams->m_pTADB->Get( pclIndSelDpCBCVParams->m_strComboPNID ).MP ) )->GetIDstr(), '\0' );
	}

	int iDN15 = pclTableDN->GetSize( _T("DN_15") );

	if( iHighestSize < iDN15 )
	{
		iHighestSize = iDN15;
	}

	if( iMaxHighestSize < iDN15 )
	{
		iMaxHighestSize = iDN15;
	}
	
	m_rBest.dBestDpAboveMin2 = DBL_MAX;
	m_rBest.dBestDpBelowMin = 0.0;
	m_rBest.dBestDp = 0.0;
	m_rBest.dBestDelta = DBL_MAX;

	// Prepare an array with a entry for each DpCBCV valve size.
	_string str;
	LPARAM lpParam;

	for( BOOL bContinue = pclIndSelDpCBCVParams->m_CtrlList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelDpCBCVParams->m_CtrlList.GetNext( str, lpParam ) )
	{
		CDB_DpCBCValve *pclDpCBCV = dynamic_cast<CDB_DpCBCValve *>( (CData *)lpParam );

		if( NULL == pclDpCBCV )
		{
			continue;
		}

		int iSizeKey = pclDpCBCV->GetSizeKey();

		// If entry doesn't yet exist, create it and set INT_MAX value. The good values will be set after below.
		if( 0 == m_mapPriority.count( iSizeKey ) )
		{
			m_mapPriority[ iSizeKey ] = INT_MAX;
		}
	}

	bool bAtLeastOneFit = false;
	*pbValidFound = false;

	for( BOOL bContinue = pclIndSelDpCBCVParams->m_CtrlList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelDpCBCVParams->m_CtrlList.GetNext( str, lpParam ) )
	{
		CDB_DpCBCValve *pclDpCBCV = dynamic_cast<CDB_DpCBCValve*>( (CData *)lpParam );
		ASSERT( NULL != pclDpCBCV );

		if( NULL == pclDpCBCV )
		{
			continue;
		}

		if( false == pclDpCBCV->IsSelectable( true ) )
		{
			continue;
		}

		if( 0.0 != dPpn && false == pclDpCBCV->IsPNIncludedInPNRange( dPpn ) )
		{
			continue;
		}

		// Do a test to verify valves will exist with the maximum boundaries.
		// If not, the problem is not a sizeShift problem.
		if( pclDpCBCV->GetSizeKey() < iMaxLowestSize || pclDpCBCV->GetSizeKey() > iMaxHighestSize )
		{
			continue;
		}

		// Means that at least one DpCBCV size is in the maximum boundaries [-4, +4]. If any DpCBCV size is in the range [iLowestSize, iHighestSize],
		// 'Select' will return 0 but with 'SizeShiftProblem' set to 'true' we signal that it is possible to find DpCBCV by changing above and below
		// size range and technical parameter.
		if( false == bAtLeastOneFit )
		{
			*pbSizeShiftProblem = true;
		}

		if( pclDpCBCV->GetSizeKey() < iLowestSize || pclDpCBCV->GetSizeKey() > iHighestSize )
		{
			continue;
		}

		// There is no problem with the size shift.
		*pbSizeShiftProblem = false;
		bAtLeastOneFit = true;

		// Get characteristic of the valve.
		CDB_DpCBCVCharacteristic *pclDpCBCVCharacteristic = pclDpCBCV->GetDpCBCVCharacteristic();
		ASSERT( NULL != pclDpCBCVCharacteristic );

		if( NULL == pclDpCBCVCharacteristic )
		{
			continue;
		}

		// Find min and max Dpl.
		double dDplmin = pclDpCBCV->GetDplmin();
		double dDplmax = pclDpCBCV->GetDplmax( pclIndSelDpCBCVParams->m_dFlow, pclIndSelDpCBCVParams->m_WC.GetDens() );
		
		if( -1.0 == dDplmin || -1.0 == dDplmax )
		{
			continue;
		}

		if( pclIndSelDpCBCVParams->m_dDpToStabilize < dDplmin )
		{
			continue;
		}

		if( pclIndSelDpCBCVParams->m_dDpToStabilize > dDplmax )
		{
			continue;
		}

		double dQMax = pclDpCBCVCharacteristic->GetQmax();
		double dDpmin = 0.0;
		bool bDpminFound = false;

		if( pclIndSelDpCBCVParams->m_dFlow <= dQMax )
		{
			// Compute the minimum required differential pressure.
			double dHMin = pclDpCBCV->GetHMin( pclIndSelDpCBCVParams->m_dFlow, pclIndSelDpCBCVParams->m_WC.GetDens() );
			double dSetting = pclDpCBCV->GetSetting( pclIndSelDpCBCVParams->m_dFlow, pclIndSelDpCBCVParams->m_WC.GetDens(), pclIndSelDpCBCVParams->m_dDpToStabilize );

			// Dp min can only be computed if we have Dpl defined.
			dDpmin = pclDpCBCV->GetDpmin( pclIndSelDpCBCVParams->m_dFlow, pclIndSelDpCBCVParams->m_WC.GetDens(), pclIndSelDpCBCVParams->m_dDpToStabilize );

			if( dDpmin > 0.0 )
			{
				bDpminFound = true;
			}

			// Create a new entry in pre-selection linked list
			CSelectedValve *pSelDpCBCV = new CSelectedValve();

			if( NULL == pSelDpCBCV )
			{
				return 0;
			}

			pSelDpCBCV->SetProductIDPtr( pclDpCBCV->GetIDPtr() );

			// Fill the fields.
			pSelDpCBCV->SetHMin( dHMin );
			pSelDpCBCV->SetH( dSetting );
			pSelDpCBCV->SetDpMin( dDpmin );
			pSelDpCBCV->SetFlag( CSelectedBase::Flags::eValveSetting, ( -1 == dSetting ) ? false : true );
			pSelDpCBCV->SetFlag( CSelectedBase::Flags::eValveDpMin, ( pclIndSelDpCBCVParams->m_dFlow > dQMax || false == bDpminFound ) );
			pSelDpCBCV->SetFlag( CSelectedBase::Flags::ePN, dPpn > pclDpCBCV->GetPmaxmax() );

			pSelDpCBCV->SetFlag( CSelectedBase::Flags::eTemperature, pclIndSelDpCBCVParams->m_WC.GetTemp() < pclDpCBCV->GetTmin() 
					|| pclIndSelDpCBCVParams->m_WC.GetTemp() > pclDpCBCV->GetTmax());
			
			pSelDpCBCV->SetFlag( CSelectedBase::Flags::eBest, false );
			pSelDpCBCV->SetFlag( CSelectedBase::Flags::eNotPriority, true );

			CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclDpCBCV, pSelDpCBCV );
			
			*pbValidFound = ( true == *pbValidFound || false == pSelDpCBCV->IsFlagSet( (CSelectedBase::Flags)( CSelectedBase::ePN | CSelectedBase::eTemperature 
					| CSelectedBase::eValveDpMin | CSelectedBase::eValveDpToSmall | CSelectedBase::eValveDpToLarge ) ) );
		}
	}

	if( true == *pbValidFound )
	{
		CleanPreselectionList( pclIndSelDpCBCVParams );
	}
	
	double dBestSetting = 0.0;
	CSelectedValve *pclBestValve = NULL;
	
	for( int iPhase = 0; iPhase < 3; iPhase++ )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedValve *pSelTap = (CSelectedValve *)m_cIter->second;
			ASSERT( NULL != pSelTap );

			if( NULL == pSelTap )
			{
				continue;
			}

			CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct*>( pSelTap->GetpData() );
			ASSERT( NULL != pTAP );

			if( NULL == pTAP )
			{
				continue;
			}

			int iSizeKey = pTAP->GetSizeKey();

			switch( iPhase )
			{
				// Store highest priority for each product size.
				case 0:	
					
					// 'm_mapPriority' store for each size the minimum priority level (the most important).
					if( pTAP->GetPriorityLevel() < m_mapPriority[iSizeKey] )
					{
						m_mapPriority[iSizeKey] = pTAP->GetPriorityLevel();
					}
					
					break;

				// Ordering Key.
				case 1:
					
					// If current selection has the lowest value for its size (lowest value means highest priority!)...
					if( m_mapPriority[iSizeKey] == pTAP->GetPriorityLevel() )
					{
						pSelTap->SetFlag( CSelectedBase::eNotPriority, false);

						// For the moment, the best one is the one with the setting opened at the max.
						if( pSelTap->GetH() > dBestSetting )
						{
							dBestSetting = pSelTap->GetH();
							pclBestValve = pSelTap;
						}
					}
					
					// Transfer valve into the final list with good priority key.
					CreateKeyAndInsertInMap(&m_MMapKeyDecreasing, pTAP, pSelTap );
					
					break;

				case 2:
					
					if( pSelTap == pclBestValve && false == pSelTap->GetFlag( CSelectedBase::eNotPriority ) )
					{
						pSelTap->SetFlag( CSelectedBase::Flags::eBest, true );
					}

					break;
			}
		}
	}
	
	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	return m_MMapKeyDecreasing.size();
}

int CSelectDpCBCVList::SelectShutoffValve( CIndSelDpCBCVParams *pclIndSelDpCBCVParams, CSelectedValve *pSelDpCBCV )
{
	if( NULL == pclIndSelDpCBCVParams || NULL == pclIndSelDpCBCVParams->m_pTADS || NULL == pclIndSelDpCBCVParams->m_pTADS->GetpTechParams()
		|| NULL == pclIndSelDpCBCVParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( NULL == pSelDpCBCV )
	{
		return 0;
	}

	CDB_DpCBCValve *pclDpCBCValve = dynamic_cast<CDB_DpCBCValve *>( pSelDpCBCV->GetpData() );

	if( NULL == pclDpCBCValve ) 
	{
		return -1;
	}
	
	if( NULL != m_pSelectedSvList )
	{
		delete m_pSelectedSvList;
		m_pSelectedSvList = NULL;
	}

	m_pSelectedSvList = new CSelectShutoffList();
	
	if( NULL == m_pSelectedSvList || NULL == m_pSelectedSvList->GetSelectPipeList() )
	{
		return 0;
	}

	CIndSelSVParams clIndSelSVParams;
	clIndSelSVParams.CopyFrom( pclIndSelDpCBCVParams );

	m_pSelectedSvList->GetSelectPipeList()->SelectPipes( &clIndSelSVParams, clIndSelSVParams.m_dFlow );

	clIndSelSVParams.m_pTADB->GetSvList( &clIndSelSVParams.m_SVList, _T("SHUTTYPE_GLOBE"), _T("FAM_STS*"), _T(""), _T(""), _T(""), 
			clIndSelSVParams.m_eFilterSelection );

	clIndSelSVParams.m_pTADB->GetSvList( &clIndSelSVParams.m_SVList, _T("SHUTTYPE_GLOBE"), _T("FAM_STS"), _T(""), _T(""), _T(""), 
			clIndSelSVParams.m_eFilterSelection );
	
	bool bSizeShiftProblem = false;
	bool bBestFound = false;
	m_pSelectedSvList->SelectShutoffValve( &clIndSelSVParams, &bSizeShiftProblem, bBestFound );
	
	return m_pSelectedSvList->GetCount();
}

////////////////////////////////////////////////////////////////
//	CSelectTrvList
////////////////////////////////////////////////////////////////
CSelectTrvList::CSelectTrvList()
	: CSelectList()
{
	m_pReturnValveList = NULL;
	Clean();
}

CSelectTrvList::~CSelectTrvList()
{
	if( NULL != m_pReturnValveList )
	{
		delete m_pReturnValveList;
		m_pReturnValveList = NULL;
	}
}

void CSelectTrvList::Clean()
{
	CSelectList::Clean();

	if( NULL != m_pReturnValveList )
	{
		delete m_pReturnValveList;
	}

	m_pReturnValveList = NULL;
	m_iSizeShiftUp = 0;
	m_iSizeShiftDown = 0;
	m_dSelDpSV = 0.0;
	m_dSelDpRV = 0.0;
	m_bDiffDpSV = true;
	m_dMaxDpSV = -DBL_MAX;
}

int CSelectTrvList::SelectSupplyValve( CIndSelTRVParams *pclIndSelTRVParams, double dDpRequiredOnSupplyValve, bool &bValidFound, 
		bool *pbSizeShiftProblem, bool bForceFullOpen )
{
	if( NULL == pclIndSelTRVParams || NULL == pclIndSelTRVParams->m_pTADS || NULL == pclIndSelTRVParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSelTRVParams->m_pTADB || NULL == pbSizeShiftProblem )
	{
		ASSERTA_RETURN( 0 );
	}

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return 0;
	}

	*pbSizeShiftProblem = false;

	// Get pointers on required database tables.
	CString strTableName = ( RadiatorValveType::RVT_Inserts == pclIndSelTRVParams->m_eValveType ) ? _T("TRINSERT_TAB") : _T("TRVALV_TAB");
	CTable *pTab = (CTable *)( pclIndSelTRVParams->m_pTADB->Get( strTableName ).MP );

	if( NULL == pTab )
	{
		ASSERTA_RETURN( 0 );
	}

	// Clean all previous data concerning supply valve selection.
	// Remark: we don't call 'Clean()' we won't delete 'm_pReturnValveList'. It allow to do a new selection with same pipes and same return valve.
	if( m_MMapKeyDecreasing.size() > 0 )
	{
		for( m_cIter = m_MMapKeyDecreasing.begin(); m_cIter != m_MMapKeyDecreasing.end(); m_cIter++ )
		{
			if( NULL != m_cIter->second)
			{
				delete m_cIter->second;
			}
		}

		m_MMapKeyDecreasing.clear();
	}

	m_PreselMMapKeyDecreasing.clear();

	if( false == m_clSelectPipeList.GetPipeSizeBound() )
	{
		return 0;
	}

	CDB_Pipe *pSmallestPipe = m_clSelectPipeList.GetpSmallestPipe();

	if( NULL == pSmallestPipe )
	{
		return 0;
	}

	CDB_Pipe *pBiggestPipe = m_clSelectPipeList.GetpLargestPipe();

	if( NULL == pBiggestPipe )
	{
		return 0;
	}

	CDS_TechnicalParameter *pclTechParams = pclIndSelTRVParams->m_pTADS->GetpTechParams();

	// Setup variables to possibly enforce combined SizeShifts.
	// In case of Hub Selection don't use SizeShift from TechParam.
	if( 99 == pclTechParams->GetSizeShiftAbove() ) // Combined SizeShift +3/-3 enforced
	{
		m_iSizeShiftUp = 3;
		m_iSizeShiftDown = -3;
	}
	else
	{
		m_iSizeShiftUp = pclTechParams->GetSizeShiftAbove();
		m_iSizeShiftDown = pclTechParams->GetSizeShiftBelow();
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclIndSelTRVParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN = pclTableDN->GetBiggestDN();
	int iHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelTRVParams->m_pTADB ) + m_iSizeShiftUp );
	int iLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelTRVParams->m_pTADB ) + m_iSizeShiftDown );

	// Get technical parameters.
	double dSVMinDpRatio = pclTechParams->GetTrvMinDpRatio();
	double dSVDefDpTot = pclTechParams->GetTrvDefDpTot();

	m_dMaxDpSV = -DBL_MAX;
	m_bDiffDpSV = false;

	// Prepare an array with a entry for each thermostatic valve size.
	_string str;
	LPARAM lpParam;

	for( BOOL bContinue = pclIndSelTRVParams->m_SupplyValveList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelTRVParams->m_SupplyValveList.GetNext( str, lpParam ) )
	{
		CDB_ThermostaticValve *pThermostaticValve = dynamic_cast<CDB_ThermostaticValve *>( (CData *)lpParam );

		if(  NULL == pThermostaticValve ) 
		{
			continue;
		}

		int iSizeKey = pThermostaticValve->GetSizeKey();
		
		// If entry doesn't yet exist, create it and set INT_MAX value. The good values will be set after below.
		if( 0 == m_mapPriority.count( iSizeKey ) )
		{
			m_mapPriority[ iSizeKey ] = INT_MAX;
		}
	}

	*pbSizeShiftProblem = true;
	bValidFound = false;

	for( BOOL bContinue = pclIndSelTRVParams->m_SupplyValveList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelTRVParams->m_SupplyValveList.GetNext( str, lpParam ) )
	{
		CDB_ThermostaticValve *pclThermostaticValve = dynamic_cast<CDB_ThermostaticValve *>( (CData *)lpParam );
		ASSERT( NULL != pclThermostaticValve );

		if( NULL == pclThermostaticValve )
		{
			continue;
		}

		// Get Kv characteristic of the valve.
		CDB_ThermoCharacteristic *pTrvCharacteristic = pclThermostaticValve->GetThermoCharacteristic();
		ASSERT( NULL != pTrvCharacteristic );

		if( NULL == pTrvCharacteristic )
		{
			continue;
		}

		if( false == pclThermostaticValve->IsSelectable( true ) )
		{
			continue;
		}

		if( pclThermostaticValve->GetSizeKey() < iLowestSize || pclThermostaticValve->GetSizeKey() > iHighestSize )
		{
			continue;
		}

		// At least one valve is in the size range.
		*pbSizeShiftProblem = false;

		// Compute the pressure drop for the valve fully open.
		double dDpFullyOpen;
		
		// If thermostatic head is used, we take the Kv characteristic at 2DT Kelvin, otherwise we take KvS (iDeltaT = 0).
		int iDeltaT = ( true == pclIndSelTRVParams->m_bIsThermostaticHead ) ? 2 : 0;

		if( -1.0 == pTrvCharacteristic->GetOpeningMax() )
		{
			continue;
		}
		
		if( false == pTrvCharacteristic->GetValveDp( pclIndSelTRVParams->m_dFlow, &dDpFullyOpen, pTrvCharacteristic->GetOpeningMax(), 
				pclIndSelTRVParams->m_WC.GetDens(), pclIndSelTRVParams->m_WC.GetKinVisc(), iDeltaT ) )
		{
			continue;
		}

		// Compute hand wheel setting.
		double dH = -1.0;
		double dDp = -1.0;
		bool bNotFound;

		// If full opening of the valve is enforced or valve is non-presettable...
		if( true == bForceFullOpen || pclThermostaticValve->GetTypeIDPtr().ID == _T("TRVTYPE_NOPRSET") 
				|| pclThermostaticValve->GetTypeIDPtr().ID == _T("TRVTYPE_INSERT_NOPRSET") )
		{
			dH = pTrvCharacteristic->GetOpeningMax();
			dDp = dDpFullyOpen;
			
			// If user has asked a precise pressure drop on both supply and return valves and if Dp fully opened on supply valve is bigger than
			// Dp required on it, than we must set that we have not found.
			// Remark: 'dSelDp' is the required Dp on supply valve!
			bNotFound = ( true == pclIndSelTRVParams->m_bDpEnabled && dDpRequiredOnSupplyValve > 0.0 && dDp >= dDpRequiredOnSupplyValve ) ? true : false;
		}
		else 
		{
			// Selection process from flow and Dp if user has introduced a Dp otherwise only from flow.

			// If user has specified a Dp that is bigger than Dp when valve is fully opened (it means that in closing valve we can
			// approach Dp chosen by user) or if user has not selected a Dp...
			if( ( true == pclIndSelTRVParams->m_bDpEnabled && dDpRequiredOnSupplyValve > dDpFullyOpen ) || false == pclIndSelTRVParams->m_bDpEnabled )
			{
				// Try to find a solution.
				bool bValid;
				dDp = ( true == pclIndSelTRVParams->m_bDpEnabled ) ? dDpRequiredOnSupplyValve : dSVMinDpRatio * dSVDefDpTot;
				
				// Remark: If valve opening is discrete, we use rounding case 4 (see 'CDB_ThermoCharacteristic::GetValveOpening()' comments in DataObj.h) otherwise
				//         we not apply rounding (case 1).
				bValid = pTrvCharacteristic->GetValveOpening( pclIndSelTRVParams->m_dFlow, dDp, &dH, pclIndSelTRVParams->m_WC.GetDens(), 
						pclIndSelTRVParams->m_WC.GetKinVisc(), ( eBool3::eb3True == pTrvCharacteristic->IsDiscrete() ) ? 4 : 1, iDeltaT );

				// If the min Kv value was not small enough...
				if( false == bValid )
				{
					// Take pressure drop available at the minimum recommenced setting of the valve.
					dH = pTrvCharacteristic->GetMinRecSetting();

					if( dH <= 0.0 )
					{
						dH = pTrvCharacteristic->GetOpeningMin();
					}

					if( -1.0 != dH )
					{
						if( false == pTrvCharacteristic->GetValveDp( pclIndSelTRVParams->m_dFlow, &dDp, dH, pclIndSelTRVParams->m_WC.GetDens(), 
								pclIndSelTRVParams->m_WC.GetKinVisc(), iDeltaT ) )
						{
							continue;
						}
					}
				}
			}

			// If dH < 0, no solution was found!
			bNotFound = ( dH < 0.0 ) ? true : false;

			if( true == bNotFound )
			{
				// Than the only solution is to take Dp when valve is fully opened.
				dH = pTrvCharacteristic->GetOpeningMax();
				dDp = dDpFullyOpen;
			}
			else if( eBool3::eb3True == pTrvCharacteristic->IsDiscrete() ) 
			{
				// Recalculate the Dp for the setting as determined above.
				// Remark: above we have computed opening to reach Dp wanted. But if valve is discrete that doesn't means at this opening we have exactly
				//         this Dp. Thus we have to compute Dp with known opening.
				if( false == pTrvCharacteristic->GetValveDp( pclIndSelTRVParams->m_dFlow, &dDp, dH, pclIndSelTRVParams->m_WC.GetDens(), 
						pclIndSelTRVParams->m_WC.GetKinVisc(), iDeltaT ) )
				{
					continue;
				}
			}
		}

		// Determine if the Dp is different from the other Trv's.
		if( m_dMaxDpSV != -DBL_MAX )
		{
			if( m_dMaxDpSV != dDp )
			{
				m_bDiffDpSV = true;
				m_dMaxDpSV = max( m_dMaxDpSV, dDp );
			}
		}
		else
		{
			m_dMaxDpSV = dDp;
		}

		// Create a new entry in pre-selection linked list.
		CSelectedValve *pSelTrv = new CSelectedValve();
		
		if( NULL == pSelTrv )
		{
			ASSERTA_RETURN( 0 );
		}

		pSelTrv->SetProductIDPtr( pclThermostaticValve->GetIDPtr() );
			
		// Fill some values.
		pSelTrv->SetH( dH );
		pSelTrv->SetDp( dDp );
		pSelTrv->SetDpFullOpen( dDpFullyOpen );

		// Set some flags.
		pSelTrv->SetFlag( CSelectedBase::Flags::eValveSetting, dH < pTrvCharacteristic->GetMinRecSetting() );
		pSelTrv->SetFlag( CSelectedBase::Flags::eValveMaxSetting, dH == pTrvCharacteristic->GetOpeningMax() );

		// Set an error code to facilitate management with 'OnTextTipFetch' in RViewSSelTrv.
		if( true == pclIndSelTRVParams->m_bDpEnabled )
		{
			if( true == bNotFound )
			{
				pSelTrv->SetFlag( CSelectedBase::Flags::eDp, true, DpNotFound );
			}
			else if( pclThermostaticValve->GetDpmax() != 0.0 && dDp > pclThermostaticValve->GetDpmax() )
			{
				pSelTrv->SetFlag( CSelectedBase::Flags::eDp, true, DpMaxReached );
			}
			else if( dDp < dSVMinDpRatio * pclIndSelTRVParams->m_dDp )
			{
				pSelTrv->SetFlag( CSelectedBase::Flags::eDp, true, DpAuthority );
			}
		}
		else
		{
			if( dDp < dSVMinDpRatio * dSVDefDpTot )
			{
				pSelTrv->SetFlag( CSelectedBase::Flags::eDp, true, DpAuthority );
			}
			
			if( pclThermostaticValve->GetDpmax() != 0.0 && dDp > pclThermostaticValve->GetDpmax() )
			{
				pSelTrv->SetFlag( CSelectedBase::Flags::eDp, true, DpMaxReached );
			}
		}

		pSelTrv->SetFlag( CSelectedBase::Flags::eValveFullODp, ( true == pclIndSelTRVParams->m_bDpEnabled && dDpFullyOpen > pclIndSelTRVParams->m_dDp ) );

		pSelTrv->SetFlag( CSelectedBase::Flags::eTemperature, pclIndSelTRVParams->m_WC.GetTemp() < pclThermostaticValve->GetTmin() 
				|| pclIndSelTRVParams->m_WC.GetTemp() > pclThermostaticValve->GetTmax() );
		
		pSelTrv->SetFlag( CSelectedBase::Flags::eBest, false );
		pSelTrv->SetFlag( CSelectedBase::Flags::eNotPriority, true );

		// Add the valve in the pre-selection list only if it's fully valid or if there is none fully valid valve already into this pre-selection list.
		CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclThermostaticValve, pSelTrv );
		bValidFound = ( true == bValidFound || false == pSelTrv->IsFlagSet( CSelectedBase::Flags::eValveGlobal ) );
	}

	if( true == bValidFound )
	{
		CleanPreselectionList( pclIndSelTRVParams );
	}
	
	sBest Best;
	
	for( int iPhase = 0; iPhase < 2; iPhase++ )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedValve *pSelTap = (CSelectedValve *)m_cIter->second;
			ASSERT( NULL != pSelTap );

			if( NULL == pSelTap )
			{
				continue;
			}

			CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pSelTap->GetpData() );
			ASSERT( NULL != pTAP );

			if( NULL == pTAP )
			{
				continue;
			}

			int iSizeKey = pTAP->GetSizeKey();
			
			switch( iPhase )
			{
				// Store highest priority for each product size.
				case 0:
					
					// If current selection has the lowest value for its size (lowest value means highest priority!)...
					if( pTAP->GetPriorityLevel() < m_mapPriority[iSizeKey] )
					{
						m_mapPriority[iSizeKey] = pTAP->GetPriorityLevel();
					}

					break;

				// Ordering key.
				case 1:
					
					// Best product is only into high priority product.
					if( m_mapPriority[iSizeKey] == pTAP->GetPriorityLevel() )
					{
						pSelTap->SetFlag( CSelectedBase::eNotPriority, false);
					}

					// Transfer valve into the final list with good priority key.
					CreateKeyAndInsertInMap(&m_MMapKeyDecreasing, pTAP, pSelTap );
					break;
			}
		}
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	m_dSelDpSV = dDpRequiredOnSupplyValve;

	return m_MMapKeyDecreasing.size();
}

int CSelectTrvList::SelectFlowLimitedControlValve( CIndSelTRVParams *pclIndSelTRVParams, bool &bValidFound, bool *pbSizeShiftProblem, 
		bool bForceFullOpen )
{
	if( NULL == pclIndSelTRVParams || NULL == pclIndSelTRVParams->m_pTADS || NULL == pclIndSelTRVParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSelTRVParams->m_pTADB || NULL == pbSizeShiftProblem )
	{
		ASSERTA_RETURN( 0 );
	}

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return 0;
	}

	// Get pointers on required database tables.
	// HYS-1305 : Add insert with automatic flow limiter
	CTable *pTab = NULL;
	if( _T( "TRVTYPE_INSERT_FL" ) != pclIndSelTRVParams->m_strComboSVInsertTypeID )
	{
		pTab = (CTable *)( pclIndSelTRVParams->m_pTADB->Get( _T( "FLCTRLVALV_TAB" ) ).MP );
	}
	else
	{
		pTab = (CTable *)( pclIndSelTRVParams->m_pTADB->Get( _T( "TRINSERT_TAB" ) ).MP );
	}

	if( NULL == pTab )
	{
		ASSERTA_RETURN( 0 );
	}

	// Clean all previous data concerning supply valve selection.
	// Remark: we don't call 'Clean()' we won't delete 'm_pReturnValveList'. It allow to do a new selection with same pipes and same return valve.
	if( m_MMapKeyDecreasing.size() > 0 )
	{
		for( m_cIter = m_MMapKeyDecreasing.begin(); m_cIter != m_MMapKeyDecreasing.end(); m_cIter++ )
		{
			if( NULL != m_cIter->second)
			{
				delete m_cIter->second;
			}
		}

		m_MMapKeyDecreasing.clear();
	}

	m_PreselMMapKeyDecreasing.clear();

	if( false == m_clSelectPipeList.GetPipeSizeBound() )
	{
		return 0;
	}

	CDB_Pipe *pSmallestPipe = m_clSelectPipeList.GetpSmallestPipe();

	if( NULL == pSmallestPipe )
	{
		return 0;
	}

	CDB_Pipe *pBiggestPipe = m_clSelectPipeList.GetpLargestPipe();

	if( NULL == pBiggestPipe )
	{
		return 0;
	}

	CDS_TechnicalParameter *pclTechParams = pclIndSelTRVParams->m_pTADS->GetpTechParams();

	// Setup variables to possibly enforce combined SizeShifts.
	// In case of Hub Selection don't use SizeShift from TechParam.
	if( 99 == pclTechParams->GetSizeShiftAbove() ) // Combined SizeShift +3/-3 enforced
	{
		m_iSizeShiftUp = 3;
		m_iSizeShiftDown = -3;
	}
	else
	{
		m_iSizeShiftUp = pclTechParams->GetSizeShiftAbove();
		m_iSizeShiftDown = pclTechParams->GetSizeShiftBelow();
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclIndSelTRVParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN = pclTableDN->GetBiggestDN();
	int iHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelTRVParams->m_pTADB ) + m_iSizeShiftUp );
	int iLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelTRVParams->m_pTADB ) + m_iSizeShiftDown );

	// HYS-699: Now we check also with the extreme limits to avoid to get the message that we don't found because
	// a size shift problem when there is no possibility even with size above/below set to 4 (max.).
	int iMaxHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelTRVParams->m_pTADB ) + pclTechParams->GetMaxSizeShift() );
	int iMaxLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelTRVParams->m_pTADB ) - pclTechParams->GetMaxSizeShift() );

	// Prepare an array with a entry for each flow limited control valve size.
	_string str;
	LPARAM lpParam;

	for( BOOL bContinue = pclIndSelTRVParams->m_SupplyValveList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelTRVParams->m_SupplyValveList.GetNext( str, lpParam ) )
	{
		CDB_FlowLimitedControlValve *pclFlowLimitedControlValve = dynamic_cast<CDB_FlowLimitedControlValve *>( (CData *)lpParam );

		if( NULL == pclFlowLimitedControlValve ) 
		{
			continue;
		}

		int iSizeKey = pclFlowLimitedControlValve->GetSizeKey();

		// If entry doesn't yet exist, create it and set INT_MAX value. The good values will be set after below.
		if( 0 == m_mapPriority.count( iSizeKey ) )
		{
			m_mapPriority[ iSizeKey ] = INT_MAX;
		}
	}


	bool bAtLeastOneFit = false;
	*pbSizeShiftProblem = false;

	bValidFound = false;

	for( BOOL bContinue = pclIndSelTRVParams->m_SupplyValveList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelTRVParams->m_SupplyValveList.GetNext( str, lpParam ) )
	{
		CDB_FlowLimitedControlValve *pclFlowLimitedControlValve = dynamic_cast<CDB_FlowLimitedControlValve *>( (CData *)lpParam );
		ASSERT( NULL != pclFlowLimitedControlValve );

		if( NULL == pclFlowLimitedControlValve )
		{
			continue;
		}

		// Get characteristic of the valve.
		CDB_FLCVCharacteristic *pFLCVCharacteristic = pclFlowLimitedControlValve->GetFLCVCharacteristic();
		ASSERT( NULL != pFLCVCharacteristic );

		if( NULL == pFLCVCharacteristic )
		{
			continue;
		}

		if( false == pclFlowLimitedControlValve->IsSelectable( true ) )
		{
			continue;
		}

		if( pclFlowLimitedControlValve->GetSizeKey() < iLowestSize || pclFlowLimitedControlValve->GetSizeKey() > iHighestSize )
		{
			// HYS-699: If this product can be selected by increasing size above/below in technical parameters, we warn the user.
			if( pclFlowLimitedControlValve->GetSizeKey() >= iMaxLowestSize && pclFlowLimitedControlValve->GetSizeKey() <= iMaxHighestSize )
			{
				bAtLeastOneFit = true;
			}

			continue;
		}

		// At least one valve is in the size range.
		*pbSizeShiftProblem = false;

		if( pclIndSelTRVParams->m_dFlow < pFLCVCharacteristic->GetQLFmin() || pclIndSelTRVParams->m_dFlow > pFLCVCharacteristic->GetQNFmax() )
		{
			continue;
		}

		// Create a new entry in pre-selection linked list.
		CSelectedValve *pSelTrv = new CSelectedValve();
		ASSERT( NULL != pSelTrv );

		if( NULL == pSelTrv )
		{
			continue;
		}
		
		pSelTrv->SetProductIDPtr( pclFlowLimitedControlValve->GetIDPtr() );
			
		// Fill some values.
		double dH = ( true == bForceFullOpen ) ? pFLCVCharacteristic->GetSettingMax() : pFLCVCharacteristic->GetSettingFromQ( pclIndSelTRVParams->m_dFlow );
		pSelTrv->SetH( dH );

		// Set some flags.
		pSelTrv->SetFlag( CSelectedBase::Flags::eValveMaxSetting, bForceFullOpen );
		
		pSelTrv->SetFlag( CSelectedBase::Flags::eTemperature, pclIndSelTRVParams->m_WC.GetTemp() < pclFlowLimitedControlValve->GetTmin() 
				|| pclIndSelTRVParams->m_WC.GetTemp() > pclFlowLimitedControlValve->GetTmax() );
		
		pSelTrv->SetFlag( CSelectedBase::Flags::eBest, false );
		pSelTrv->SetFlag( CSelectedBase::Flags::eNotPriority, true );

		// Add the valve in the pre-selection list only if it's fully valid or if there is none fully valid valve already into this pre-selection list.
		CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclFlowLimitedControlValve, pSelTrv );

		bValidFound = ( true == bValidFound || false == pSelTrv->IsFlagSet( CSelectedBase::Flags::eValveGlobal ) );
	}

	if( true == bValidFound )
	{
		CleanPreselectionList( pclIndSelTRVParams );
	}
	
	double dBestFlow = DBL_MAX;
	
	for( int iPhase = 0; iPhase < 3; iPhase++ )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedValve *pSelTap = (CSelectedValve *)m_cIter->second;
			ASSERT( NULL != pSelTap );

			if( NULL == pSelTap )
			{
				continue;
			}

			CDB_FlowLimitedControlValve *pclFlowLimitedControlValve = dynamic_cast<CDB_FlowLimitedControlValve*>( pSelTap->GetpData() );
			ASSERT( NULL != pclFlowLimitedControlValve );

			if( NULL == pclFlowLimitedControlValve )
			{
				continue;
			}

			int iSizeKey = pclFlowLimitedControlValve->GetSizeKey();
			
			switch( iPhase )
			{
				// Store highest priority for each product size.
				case 0:
					
					// If current selection has the lowest value for its size (lowest value means highest priority!)...
					if( pclFlowLimitedControlValve->GetPriorityLevel() < m_mapPriority[iSizeKey] )
					{
						m_mapPriority[iSizeKey] = pclFlowLimitedControlValve->GetPriorityLevel();
					}
					
					break;

				// Ordering key.
				case 1:
					
					// Best product is only into high priority product.
					if( m_mapPriority[iSizeKey] == pclFlowLimitedControlValve->GetPriorityLevel() )
					{
						pSelTap->SetFlag( CSelectedBase::eNotPriority, false );

						if( true == bForceFullOpen )
						{
							CDB_FLCVCharacteristic *pclFLCVCharacteristic = pclFlowLimitedControlValve->GetFLCVCharacteristic();
							
							if( NULL != pclFLCVCharacteristic )
							{
								double dFlow = pclFLCVCharacteristic->GetQ( pclFLCVCharacteristic->GetSettingMax() );
								
								if( abs( dFlow - pclIndSelTRVParams->m_dFlow ) < dBestFlow )
								{
									dBestFlow = abs( dFlow - pclIndSelTRVParams->m_dFlow );
								}
							}
						}
						else
						{
							dBestFlow = pclIndSelTRVParams->m_dFlow;
						}
					}
					
					// Transfer valve into the final list with good priority key.
					CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pclFlowLimitedControlValve, pSelTap );
					
					break;

				case 2:
					
					if( true == bForceFullOpen )
					{
						CDB_FLCVCharacteristic *pclFLCVCharacteristic = pclFlowLimitedControlValve->GetFLCVCharacteristic();
						
						if( NULL != pclFLCVCharacteristic )
						{
							double dFlow = pclFLCVCharacteristic->GetQ( pclFLCVCharacteristic->GetSettingMax() );

							if( dFlow == dBestFlow )
							{
								pSelTap->SetFlag( CSelectedBase::Flags::eBest, true );
							}
						}
					}
					else
					{
						pSelTap->SetFlag( CSelectedBase::Flags::eBest, true );
					}

					break;
			}
		}
	}
	
	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	// HYS-699: If this product can be selected by increasing size above/below in technical parameters, we warn the user.
	if( 0 == m_MMapKeyDecreasing.size() && true == bAtLeastOneFit )
	{
		*pbSizeShiftProblem = true;
	}

	return m_MMapKeyDecreasing.size();
}

int CSelectTrvList::SelectReturnValve( CIndSelTRVParams *pclIndSelTRVParams, CString strSelRVTypeID, CString strSelRVFamilyID, CString strSelRVConnectID, 
		CString strSelRVVersionID, double dDpRequiredOnReturnValve, bool bForceFullOpening )
{
	if( NULL == pclIndSelTRVParams || NULL == pclIndSelTRVParams->m_pTADS || NULL == pclIndSelTRVParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSelTRVParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return 0;
	}

	if( NULL != m_pReturnValveList )
	{
		delete m_pReturnValveList;
		m_pReturnValveList = NULL;
	}

	m_pReturnValveList = new CSelectList();

	if( NULL == m_pReturnValveList || NULL == m_pReturnValveList->GetSelectPipeList() )
	{
		return 0;
	}

	CIndSelBVParams clIndSelBVParams;
	clIndSelBVParams.CopyFrom( pclIndSelTRVParams );
	clIndSelBVParams.m_dDp = dDpRequiredOnReturnValve;

	// HYS-832: the 'CSelectPipeList::SelectPipes' method will internally create a 'CSelectPipe' object by passing as argument a pointer
	// on a 'CProductSelelectionParameters' object that is itself passed as first argument in 'SelectPipes'. If we pass 'clIndSelBVParams',
	// this object will be destroyed at the end of this method and than pointers in 'CSelectPipe' will be no more valid.
	// This is why here we need to pass the 'pclIndSelTRVParams' variable.
	// m_pReturnValveList->GetSelectPipeList()->SelectPipes( &clIndSelBVParams, clIndSelBVParams.m_dFlow );
	m_pReturnValveList->GetSelectPipeList()->SelectPipes( pclIndSelTRVParams, pclIndSelTRVParams->m_dFlow );

	// Preselect regulating valve.
	clIndSelBVParams.m_pTADB->GetBVList( &clIndSelBVParams.m_BvList, (LPCTSTR)strSelRVTypeID, (LPCTSTR)strSelRVFamilyID, _T(""), (LPCTSTR)strSelRVConnectID, 
			(LPCTSTR)strSelRVVersionID, clIndSelBVParams.m_eFilterSelection );

	m_pReturnValveList->GetManBvList( &clIndSelBVParams, bForceFullOpening );

	m_dSelDpRV = dDpRequiredOnReturnValve;

	return m_pReturnValveList->GetCount();
}

////////////////////////////////////////////////////////////////
//	CSelectSeparatorList
////////////////////////////////////////////////////////////////
CSelectSeparatorList::CSelectSeparatorList()
	: CIndividualSelectBaseList()
{
	Clean();
}

void CSelectSeparatorList::Clean()
{
	CIndividualSelectBaseList::Clean();
	m_iSizeShiftUp = 0;
	m_iSizeShiftDown = 0;
}

int CSelectSeparatorList::SelectSeparator( CIndSelSeparatorParams *pclIndSelSeparatorParams, bool *pbSizeShiftProblem, bool &bBestFound )
{
	if( NULL == pclIndSelSeparatorParams || NULL == pclIndSelSeparatorParams->m_pTADS || NULL == pclIndSelSeparatorParams->m_pTADS->GetpTechParams()
		|| NULL == pclIndSelSeparatorParams->m_pTADB || NULL == pbSizeShiftProblem )
	{
		ASSERTA_RETURN( 0 );
	}

	bBestFound = false;

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return 0;
	}

	// Get pointers on required database tables.
	CTable *pTab = (CTable*)( pclIndSelSeparatorParams->m_pTADB->Get( _T("SEPARATOR_TAB") ).MP );
	ASSERT( NULL != pTab );

	if( NULL == pTab )
	{
		return 0;
	}

	// Clean all previous data concerning separator selection.
	// Remark: we don't call 'Clean()' because we won't delete the previous pipe selection. It allows us to do a new selection with same pipes.
	if( m_MMapKeyDecreasing.size() > 0 )
	{
		for( m_cIter = m_MMapKeyDecreasing.begin(); m_cIter != m_MMapKeyDecreasing.end(); m_cIter++ )
		{
			if( NULL != m_cIter->second )
			{
				delete m_cIter->second;
			}
		}

		m_MMapKeyDecreasing.clear();
	}
	
	m_PreselMMapKeyDecreasing.clear();
	m_mapPriority.clear();

	if( false == m_clSelectPipeList.GetPipeSizeBound() )
	{
		return 0;
	}

	CDB_Pipe *pSmallestPipe = m_clSelectPipeList.GetpSmallestPipe();

	if( NULL == pSmallestPipe )
	{
		return 0;
	}

	CDB_Pipe *pBiggestPipe = m_clSelectPipeList.GetpLargestPipe();

	if( NULL == pBiggestPipe )
	{
		return 0;
	}

	CDS_TechnicalParameter *pclTechParams = pclIndSelSeparatorParams->m_pTADS->GetpTechParams();

	// Setup variables to possibly enforce combined SizeShifts.
	// In case of Hub Selection don't use SizeShift from TechParam.
	if( 99 == pclTechParams->GetSizeShiftAbove() ) // Combined SizeShift +3/-3 enforced
	{
		m_iSizeShiftUp = 3;
		m_iSizeShiftDown = -3;
	}
	else
	{
		m_iSizeShiftUp = pclTechParams->GetSizeShiftAbove();
		m_iSizeShiftDown = pclTechParams->GetSizeShiftBelow();
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclIndSelSeparatorParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN = pclTableDN->GetBiggestDN();
	int iHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelSeparatorParams->m_pTADB ) + m_iSizeShiftUp );
	int iLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelSeparatorParams->m_pTADB ) + m_iSizeShiftDown );

	// HYS-699: Now we check also with the extreme limits to avoid to get the message that we don't found because
	// a size shift problem when there is no possibility even with size above/below set to 4 (max.).
	int iMaxHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelSeparatorParams->m_pTADB ) + pclTechParams->GetMaxSizeShift() );
	int iMaxLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelSeparatorParams->m_pTADB ) - pclTechParams->GetMaxSizeShift() );

	int iDN15 = pclTableDN->GetSize( _T("DN_15") );
	
	if( iHighestSize < iDN15 )
	{
		iHighestSize = iDN15;
	}

	if( iMaxHighestSize < iDN15 )
	{
		iMaxHighestSize = iDN15;
	}

	bool bAtLeastOneFit = false;
	*pbSizeShiftProblem = false;

	_string str;
	LPARAM lpParam;
	
	for( BOOL bContinue = pclIndSelSeparatorParams->m_SeparatorList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelSeparatorParams->m_SeparatorList.GetNext( str, lpParam ) )
	{
		CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator *>( (CData *)lpParam );
		ASSERT( NULL != pclSeparator );

		if( NULL == pclSeparator )
		{
			continue;
		}

		// 'm_mapPriority' stores for each size the minimum priority level (most important).
		if( 0 == m_mapPriority.count( pclSeparator->GetSizeKey() ) )
		{
			m_mapPriority[pclSeparator->GetSizeKey()] = INT_MAX;
		}

		CDB_QDpCharacteristic *pclQDpCharacteristic = pclSeparator->GetQDpCharacteristic();
		
		if( NULL == pclQDpCharacteristic )
		{
			pclIndSelSeparatorParams->m_SeparatorList.Delete();

			continue;
		}

		if( pclSeparator->GetSizeKey() < iLowestSize || pclSeparator->GetSizeKey() > iHighestSize )
		{
			pclIndSelSeparatorParams->m_SeparatorList.Delete();

			// HYS-699: If this product can be selected by increasing size above/below in technical parameters, we warn the user.
			if( pclSeparator->GetSizeKey() >= iMaxLowestSize && pclSeparator->GetSizeKey() <= iMaxHighestSize )
			{
				bAtLeastOneFit = true;
			}

			continue;
		}

		if( pclIndSelSeparatorParams->m_dFlow < pclQDpCharacteristic->GetQmin() || pclIndSelSeparatorParams->m_dFlow > pclQDpCharacteristic->GetQmax() )
		{
			pclIndSelSeparatorParams->m_SeparatorList.Delete();
			continue;
		}

		// Create a new entry in pre-selection linked list.
		CSelectedSeparator *pSelSeparator = new CSelectedSeparator();
		ASSERT( NULL != pSelSeparator );

		if( NULL == pSelSeparator )
		{
			continue;
		}

		pSelSeparator->SetProductIDPtr( pclSeparator->GetIDPtr() );
		pSelSeparator->SetDp( pclQDpCharacteristic->GetDp( pclIndSelSeparatorParams->m_dFlow, pclIndSelSeparatorParams->m_WC.GetDens() ) );
		pSelSeparator->SetFlag( CSelectedBase::Flags::eNotPriority, true );

		CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclSeparator, pSelSeparator );
	}

	double dBestDelta = DBL_MAX;
	
	for( int iPhase = 0; iPhase < 3; iPhase++ )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedSeparator *pSelTap = (CSelectedSeparator *)m_cIter->second;
			ASSERT( NULL != pSelTap );

			if( NULL == pSelTap )
			{
				continue;
			}

			CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator *>( pSelTap->GetProductIDPtr().MP );
			ASSERT( NULL != pclSeparator );

			if( NULL == pclSeparator )
			{
				continue;
			}

			CDB_QDpCharacteristic *pclQDpCharacteristic = pclSeparator->GetQDpCharacteristic();

			if( NULL == pclQDpCharacteristic )
			{
				continue;
			}

			int iSizeKey = pclSeparator->GetSizeKey();

			switch( iPhase )
			{
				// Store highest priority for each product size.
				case 0:
					
					// If current selection has the lowest value for its size (lowest value means highest priority!)...
					if( pclSeparator->GetPriorityLevel() < m_mapPriority[iSizeKey] )
					{
						m_mapPriority[iSizeKey] = pclSeparator->GetPriorityLevel();
					}
					
					break;

				// Ordering key.
				case 1:
					
					// Best product is only into high priority product.
					if( m_mapPriority[iSizeKey] == pclSeparator->GetPriorityLevel() )
					{
						pSelTap->SetFlag( CSelectedBase::eNotPriority, false);

						// Check the product that is closest of its nominal flow.
						if( pclIndSelSeparatorParams->m_dFlow < pclQDpCharacteristic->GetQnom() && pclQDpCharacteristic->GetQnom() - pclIndSelSeparatorParams->m_dFlow < dBestDelta )
						{
							dBestDelta = pclQDpCharacteristic->GetQnom() - pclIndSelSeparatorParams->m_dFlow;
						}
					}
					
					break;

				case 2:
					
					// Set flag best flow (Best product is only into high priority product).
					if( m_mapPriority[iSizeKey] == pclSeparator->GetPriorityLevel() )
					{
						if( DBL_MAX != dBestDelta && pclIndSelSeparatorParams->m_dFlow < pclQDpCharacteristic->GetQnom() 
								&& ( ( pclQDpCharacteristic->GetQnom() - pclIndSelSeparatorParams->m_dFlow ) == dBestDelta ) )
						{
							pSelTap->SetFlag( CSelectedBase::eBest, true );
							bBestFound = true;
						}
					}
					
					CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pclSeparator, pSelTap );
					
					break;
			}
		}
	}
	
	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	// HYS-699: If this product can be selected by increasing size above/below in technical parameters, we warn the user.
	if( 0 == m_MMapKeyDecreasing.size() && true == bAtLeastOneFit )
	{
		*pbSizeShiftProblem = true;
	}

	return m_MMapKeyDecreasing.size();
}

////////////////////////////////////////////////////////////////
//	CSelectShutoffList
////////////////////////////////////////////////////////////////
CSelectShutoffList::CSelectShutoffList()
	: CIndividualSelectBaseList()
{
	Clean();
}

void CSelectShutoffList::Clean()
{
	CIndividualSelectBaseList::Clean();
	m_iSizeShiftUp = 0;
	m_iSizeShiftDown = 0;
}

int CSelectShutoffList::SelectShutoffValve( CIndSelSVParams *pclIndSelSVParams, bool *pbSizeShiftProblem, bool &bBestFound )
{
	bBestFound = false;

	if( NULL == pclIndSelSVParams || NULL == pclIndSelSVParams->m_pTADS || NULL == pclIndSelSVParams->m_pTADS->GetpTechParams()
		|| NULL == pclIndSelSVParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return 0;
	}

	// Get pointers on required database tables.
	CTable *pTab = (CTable*)( pclIndSelSVParams->m_pTADB->Get( _T("SHUTOFF_TAB") ).MP );
	
	if( NULL == pTab )
	{
		ASSERTA_RETURN( 0 );
	}

	// Clean all previous data concerning shut-off selection.
	// Remark: we don't call 'Clean()' because we won't delete the previous pipe selection. It allows us to do a new selection with same pipes.
	if( m_MMapKeyDecreasing.size() > 0 )
	{
		for( m_cIter = m_MMapKeyDecreasing.begin(); m_cIter != m_MMapKeyDecreasing.end(); m_cIter++ )
		{
			if( NULL != m_cIter->second)
			{
				delete m_cIter->second;
			}
		}
		
		m_MMapKeyDecreasing.clear();
	}

	m_PreselMMapKeyDecreasing.clear();
	m_mapPriority.clear();

	if( false == m_clSelectPipeList.GetPipeSizeBound() )
	{
		return 0;
	}

	CDB_Pipe *pSmallestPipe = m_clSelectPipeList.GetpSmallestPipe();

	if( NULL == pSmallestPipe )
	{
		return 0;
	}

	CDB_Pipe *pBiggestPipe = m_clSelectPipeList.GetpLargestPipe();

	if( NULL == pBiggestPipe )
	{
		return 0;
	}

	CDS_TechnicalParameter *pclTechParams = pclIndSelSVParams->m_pTADS->GetpTechParams();

	// Setup variables to possibly enforce combined SizeShifts.
	// In case of Hub Selection don't use SizeShift from TechParam.
	if( 99 == pclTechParams->GetSizeShiftAbove() ) // Combined SizeShift +3/-3 enforced
	{
		m_iSizeShiftUp = 3;
		m_iSizeShiftDown = -3;
	}
	else
	{
		m_iSizeShiftUp = pclTechParams->GetSizeShiftAbove();
		m_iSizeShiftDown = pclTechParams->GetSizeShiftBelow();
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclIndSelSVParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN = pclTableDN->GetBiggestDN();
	int iHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelSVParams->m_pTADB ) + m_iSizeShiftUp );
	int iLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelSVParams->m_pTADB ) + m_iSizeShiftDown );

	int iMaxHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSelSVParams->m_pTADB ) + pclTechParams->GetMaxSizeShift() );
	int iMaxLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSelSVParams->m_pTADB ) - pclTechParams->GetMaxSizeShift() );

	int iDN15 = pclTableDN->GetSize( _T("DN_15") );

	if( iHighestSize < iDN15 )
	{
		iHighestSize = iDN15;
	}

	if( iMaxHighestSize < iDN15 )
	{
		iMaxHighestSize = iDN15;
	}

	bool bAtLeastOneFit = false;
	*pbSizeShiftProblem = true;

	m_rBest.dBestDp = 0.0;
	m_rBest.dBestDelta = DBL_MAX;

	_string str;
	LPARAM lpParam;

	for( BOOL bContinue = pclIndSelSVParams->m_SVList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelSVParams->m_SVList.GetNext( str, lpParam ) )
	{
		CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve *>( (CData *)lpParam );
		ASSERT( NULL != pclShutoffValve );

		if( NULL == pclShutoffValve )
		{
			continue;
		}

		// Do not show the no drain sts if the shutoff valve is associated
		if ( (PSC_BC_CombinedDpCBalCtrlValve == pclIndSelSVParams->m_eProductSubCategory) &&
			 ( 0 != StringCompare(_T("VERS_DRAIN12"), pclShutoffValve->GetVersionID())) &&
			 ( 0 != StringCompare(_T("VERS_DRAIN34"), pclShutoffValve->GetVersionID())) )
		{
			continue;
		}

		// 'm_mapPriority' stores for each size the minimum priority level (most important).
		if( 0 == m_mapPriority.count( pclShutoffValve->GetSizeKey() ) )
		{
			m_mapPriority[pclShutoffValve->GetSizeKey()] = INT_MAX;
		}

		// Do a test to verify if valves exist within the maximum boundaries.
		// If not, the problem is not a sizeShift problem.
		if( pclShutoffValve->GetSizeKey() < iMaxLowestSize || pclShutoffValve->GetSizeKey() > iMaxHighestSize )
		{
			continue;
		}

		// Means that at least one shut-off valve size is in the maximum boundaries [-4, +4]. If any suht-off valve size is in the range 
		// [iLowestSize, iHighestSize], 'SelectShutoffValve' will return 0 but with 'SizeShiftProblem' set to 'true' we signal that it is possible 
		// to find shut-off valve by changing above and below size range and technical parameter.
		if( false == bAtLeastOneFit )
		{
			*pbSizeShiftProblem = true;
		}

		if( pclShutoffValve->GetSizeKey() < iLowestSize || pclShutoffValve->GetSizeKey() > iHighestSize )
		{
			continue;
		}

		// There is no problem with the size shift.
		*pbSizeShiftProblem = false;
		bAtLeastOneFit = true;

		CDB_ValveCharacteristic *pclValveCharacteristic = pclShutoffValve->GetValveCharacteristic();

		if( NULL == pclValveCharacteristic )
		{
			continue;
		}

		// Create a new entry in pre-selection linked list.
		CSelectedValve *pclSelectedShutoffValve = new CSelectedValve();
		ASSERT( NULL != pclSelectedShutoffValve );

		if( NULL == pclSelectedShutoffValve )
		{
			continue;
		}

		pclSelectedShutoffValve->SetProductIDPtr( pclShutoffValve->GetIDPtr() );

		pclSelectedShutoffValve->SetDp( pclValveCharacteristic->GetDpFullOpening( pclIndSelSVParams->m_dFlow, pclIndSelSVParams->m_WC.GetDens(), 
				pclIndSelSVParams->m_WC.GetKinVisc() ) );
		
		pclSelectedShutoffValve->SetFlag( CSelectedBase::Flags::eNotPriority, true );
		CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclShutoffValve, pclSelectedShutoffValve );
	}

	for( int iPhase = 0; iPhase < 3; iPhase++ )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedValve *pSelTap = (CSelectedValve *)m_cIter->second;
			ASSERT( NULL != pSelTap );
			
			if( NULL == pSelTap )
			{
				continue;
			}

			CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve *>( pSelTap->GetProductIDPtr().MP );
			ASSERT( NULL != pclShutoffValve );

			if( NULL == pclShutoffValve )
			{
				continue;
			}

			CDB_ValveCharacteristic *pclValveCharacteristic = pclShutoffValve->GetValveCharacteristic();

			if( NULL == pclValveCharacteristic )
			{
				continue;
			}

			int iSizeKey = pclShutoffValve->GetSizeKey();

			switch( iPhase )
			{
				// Store highest priority for each product size.
				case 0:
					
					// If current selection has the lowest value for its size (lowest value means highest priority!)...
					if( pclShutoffValve->GetPriorityLevel() < m_mapPriority[iSizeKey] )
					{
						m_mapPriority[iSizeKey] = pclShutoffValve->GetPriorityLevel();
					}
					
					break;

				// Ordering key.
				case 1:
					
					// Best product is only into high priority product.
					if( m_mapPriority[iSizeKey] == pclShutoffValve->GetPriorityLevel() )
					{
						pSelTap->SetFlag( CSelectedBase::eNotPriority, false);

						// Check the product that is closest of its nominal flow.
						double dDelta = fabs( pSelTap->GetDp() - pclTechParams->GetValvMinDp() );
						pSelTap->SetBestDelta( dDelta );
						
						if( dDelta < m_rBest.dBestDelta )
						{
							m_rBest.dBestDelta = dDelta;
							m_rBest.dBestDp = pSelTap->GetDp();
						}
					}
					break;

				case 2:
					
					// Set flag best flow (Best product is only into high priority product).
					if( m_mapPriority[iSizeKey] == pclShutoffValve->GetPriorityLevel() )
					{
						if( DBL_MAX != m_rBest.dBestDelta && m_rBest.dBestDp == pSelTap->GetDp() )
						{
							pSelTap->SetFlag( CSelectedBase::eBest, true );
							bBestFound = true;
						}
					}

					CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pclShutoffValve, pSelTap );
					break;
			}
		}
	}
	
	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	return m_MMapKeyDecreasing.size();
}

////////////////////////////////////////////////////////////////
//	CSelectSafetyValveList
////////////////////////////////////////////////////////////////
CSelectSafetyValveList::CSelectSafetyValveList()
	: CIndividualSelectBaseList()
{
	m_pclBlowTankList = NULL;
	Clean();
}

CSelectSafetyValveList::~CSelectSafetyValveList()
{
	if( NULL != m_pclBlowTankList )
	{
		delete m_pclBlowTankList;
		m_pclBlowTankList = NULL;
	}
}

void CSelectSafetyValveList::Clean()
{
	CIndividualSelectBaseList::Clean();

	if( NULL != m_pclBlowTankList )
	{
		delete m_pclBlowTankList;
		m_pclBlowTankList = NULL;
	}
}

int CSelectSafetyValveList::SelectSafetyValve( CIndSelSafetyValveParams *pclIndSelSafetyValveParams, bool &bBestFound )
{
	if( NULL == pclIndSelSafetyValveParams )
	{
		ASSERTA_RETURN( 0 );
	}

	int iResultCount = 0;

	if( Heating == pclIndSelSafetyValveParams->m_eSystemApplicationType )
	{
		iResultCount = _SelectSafetyValveHeating( pclIndSelSafetyValveParams, bBestFound );
	}
	else if( Cooling == pclIndSelSafetyValveParams->m_eSystemApplicationType )
	{
		iResultCount = _SelectSafetyValveCooling( pclIndSelSafetyValveParams, bBestFound );
	}
	else if( Solar == pclIndSelSafetyValveParams->m_eSystemApplicationType )
	{
		iResultCount = _SelectSafetyValveSolar( pclIndSelSafetyValveParams, bBestFound );
	}

	return iResultCount;
}

int CSelectSafetyValveList::SelectBlowTank( CIndSelSafetyValveParams *pclIndSelSafetyValveParams, CSelectedSafetyValve *pclSelectedSafetyValve )
{
	if( NULL == pclIndSelSafetyValveParams || NULL == pclIndSelSafetyValveParams->m_pTADS || NULL == pclIndSelSafetyValveParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSelSafetyValveParams->m_pTADB || NULL == pclSelectedSafetyValve 
			|| NULL == dynamic_cast<CDB_SafetyValveBase *>( pclSelectedSafetyValve->GetProductIDPtr().MP ) )
	{
		ASSERTA_RETURN( 0 );
	}

	CDB_SafetyValveBase *pclSafetyValveBase = dynamic_cast<CDB_SafetyValveBase *>( pclSelectedSafetyValve->GetProductIDPtr().MP );

	if( ProjectType::Heating != pclSafetyValveBase->GetApplicationCompatibility() )
	{
		// Blow tank is only for heating application type.
		return 0;
	}

	CDB_BlowTankAllocation *pclBlowTankAllocationTable = pclSafetyValveBase->GetBlowTankAllocationTable();

	if( NULL == pclBlowTankAllocationTable )
	{
		return 0;
	}

	CString strBlowTankID = pclBlowTankAllocationTable->GetBlowTank( pclSafetyValveBase, pclIndSelSafetyValveParams->m_dUserSetPressureChoice );

	if( true == strBlowTankID.IsEmpty() || NULL == dynamic_cast<CDB_BlowTank *>( pclIndSelSafetyValveParams->m_pTADB->Get( strBlowTankID ).MP ) )
	{
		return 0;
	}

	CDB_BlowTank *pclBlowTank = dynamic_cast<CDB_BlowTank *>( pclIndSelSafetyValveParams->m_pTADB->Get( strBlowTankID ).MP );

	if( NULL != m_pclBlowTankList )
	{
		delete m_pclBlowTankList;
		m_pclBlowTankList = NULL;
	}

	m_pclBlowTankList = new CSelectList();

	if( NULL == m_pclBlowTankList )
	{
		return 0;
	}

	CSelectedBase *pclSelectedBlowTank = new CSelectedBase( CSelectedBase::SBT_BlowTank );

	if( NULL == pclSelectedBlowTank )
	{
		delete m_pclBlowTankList;
		m_pclBlowTankList = NULL;
		return 0;
	}

	pclSelectedBlowTank->SetProductIDPtr( pclBlowTank->GetIDPtr() );
	pclSelectedBlowTank->SetFlag( CSelectedBase::Flags::eTemperature, pclIndSelSafetyValveParams->m_WC.GetTemp() < pclBlowTank->GetTmin() 
			|| pclIndSelSafetyValveParams->m_WC.GetTemp() > pclBlowTank->GetTmax() );
	pclSelectedBlowTank->SetFlag( CSelectedBase::Flags::eBest, true );
	pclSelectedBlowTank->SetFlag( CSelectedBase::Flags::eNotPriority, false );

	CreateKeyAndInsertInMap( &m_pclBlowTankList->m_MMapKeyDecreasing, pclBlowTank, pclSelectedBlowTank );

	return m_pclBlowTankList->GetCount();
}

void CSelectSafetyValveList::FillResultsInRankEx( CRankEx *pList )
{
	if( NULL == pList || 0 == (int)m_MMapKeyDecreasing.size() )
	{
		return;
	}

	pList->PurgeAll();

	for( CSelectedBase *pclLoopSelectedProduct = GetFirst<CSelectedBase>(); NULL != pclLoopSelectedProduct; pclLoopSelectedProduct = GetNext<CSelectedBase>() )
	{
		CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase *>( pclLoopSelectedProduct->GetpData() );
		pList->Add( pclSafetyValve->GetIDPtr().ID, pclSafetyValve->GetSortingKey(), (LPARAM)pclSafetyValve->GetIDPtr().MP );
	}
}

int CSelectSafetyValveList::_SelectSafetyValveHeating( CIndSelSafetyValveParams *pclIndSelSafetyValveParams, bool &bBestFound )
{
	if( NULL == pclIndSelSafetyValveParams || NULL == pclIndSelSafetyValveParams->m_pTADS || NULL == pclIndSelSafetyValveParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSelSafetyValveParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	bBestFound = false;

	// Get pointers on required database tables.
	CTable *pTab = (CTable*)( pclIndSelSafetyValveParams->m_pTADB->Get( _T("SAFETYVALVE_TAB") ).MP );
	ASSERT( NULL != pTab );

	if( NULL == pTab )
	{
		return 0;
	}

	// Clean all previous data concerning safety valve selection.
	Clean();

	CSelectedSafetyValve *pclBestSafetyValve = NULL;
	int iBestQuantityNeeded = INT_MAX;
	int iBestSizeKey = INT_MAX;
	double dBestSetPressure = DBL_MAX;
	bool bAtLeastOneFit = false;
	_string str;
	LPARAM lpParam;

	for( BOOL bContinue = pclIndSelSafetyValveParams->m_SafetyValveList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelSafetyValveParams->m_SafetyValveList.GetNext( str, lpParam ) )
	{
		CDB_SafetyValveHeating *pclSafetyValveHeating = dynamic_cast<CDB_SafetyValveHeating *>( (CData *)lpParam );
		ASSERT( NULL != pclSafetyValveHeating );

		if( NULL == pclSafetyValveHeating )
		{
			continue;
		}

		// Exclude those valves that have not the same set pressure.
		if( pclIndSelSafetyValveParams->m_dUserSetPressureChoice != 0.0 
				&& pclIndSelSafetyValveParams->m_dUserSetPressureChoice != pclSafetyValveHeating->GetSetPressure() )
		{
			pclIndSelSafetyValveParams->m_SafetyValveList.Delete();
			continue;
		}

		int iNbrSafetyValve = 0;
		double dPowerLimit = pclSafetyValveHeating->GetPowerLimit( pclIndSelSafetyValveParams->m_strSystemHeatGeneratorTypeID );

		if( 0.0 == dPowerLimit )
		{
			continue;
		}

		iNbrSafetyValve = (int)ceil( pclIndSelSafetyValveParams->m_dInstalledPower / dPowerLimit );

		if( iNbrSafetyValve < 1 || iNbrSafetyValve > pclIndSelSafetyValveParams->m_pTADS->GetpTechParams()->GetMaxSafetyValveInParallel() )
		{
			continue;
		}

		// HYS-1091: If more than one safety valve is used in parallel, the smaller unit must have a blow-of capacity at least 40% of the total.
		if( iNbrSafetyValve > 1 && dPowerLimit < ( 0.4 * pclIndSelSafetyValveParams->m_dInstalledPower ) )
		{
			continue;
		}

		// Create a new entry in pre-selection linked list.
		CSelectedSafetyValve *pclSelSafetyValve = new CSelectedSafetyValve();
		ASSERT( NULL != pclSelSafetyValve );

		if( NULL == pclSelSafetyValve )
		{
			continue;
		}

		pclSelSafetyValve->SetProductIDPtr( pclSafetyValveHeating->GetIDPtr() );
		pclSelSafetyValve->SetQuantityNeeded( iNbrSafetyValve );

		// First we take the safety valve for which we have the lowest needed number.
		// Second, if with the current safety valve we need the same number, we will take the lower one in term of size.
		// Third, if with the current safety valve we need the same number and it's the same size, we will take the lower one in term of set pressure.
		if( iNbrSafetyValve < iBestQuantityNeeded 
				|| ( iNbrSafetyValve == iBestQuantityNeeded && pclSafetyValveHeating->GetSizeKey() < iBestSizeKey ) 
				|| ( iNbrSafetyValve == iBestQuantityNeeded && pclSafetyValveHeating->GetSizeKey() == iBestSizeKey && pclSafetyValveHeating->GetSetPressure() < dBestSetPressure ) )
		{
			pclBestSafetyValve = pclSelSafetyValve;
			iBestSizeKey = pclSafetyValveHeating->GetSizeKey();
			dBestSetPressure = pclSafetyValveHeating->GetSetPressure();
			iBestQuantityNeeded = iNbrSafetyValve;
			bBestFound = true;
		}

		CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclSafetyValveHeating, pclSelSafetyValve );
	}

	if( NULL != pclBestSafetyValve )
	{
		pclBestSafetyValve->SetFlag( CSelectedBase::eBest, true );
	}

	for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
	{
		CSelectedSafetyValve *pSelTap = (CSelectedSafetyValve *)m_cIter->second;
		ASSERT( NULL != pSelTap );

		if( NULL == pSelTap )
		{
			continue;
		}

		CDB_SafetyValveHeating *pclSafetyValveHeating = dynamic_cast<CDB_SafetyValveHeating *>( pSelTap->GetProductIDPtr().MP );
		ASSERT( NULL != pclSafetyValveHeating );

		if( NULL == pclSafetyValveHeating )
		{
			delete pSelTap;
			continue;
		}

		int iSizeKey = pclSafetyValveHeating->GetSizeKey();

		// Accept only one size above the best.
		if( iSizeKey > iBestSizeKey + 1 )
		{
			delete pSelTap;
			continue;
		}

		CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pclSafetyValveHeating, pSelTap );
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	return m_MMapKeyDecreasing.size();
}

int CSelectSafetyValveList::_SelectSafetyValveCooling( CIndSelSafetyValveParams *pclIndSelSafetyValveParams, bool &bBestFound )
{
	if( NULL == pclIndSelSafetyValveParams || NULL == pclIndSelSafetyValveParams->m_pTADS || NULL == pclIndSelSafetyValveParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSelSafetyValveParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	bBestFound = false;

	// Get pointers on required database tables.
	CTable *pTab = (CTable*)( pclIndSelSafetyValveParams->m_pTADB->Get( _T("SAFETYVALVE_TAB") ).MP );
	ASSERT( NULL != pTab );

	if( NULL == pTab )
	{
		return 0;
	}

	// Clean all previous data concerning safety valve selection.
	Clean();

	CSelectedSafetyValve *pclBestSafetyValve = NULL;
	int iBestQuantityNeeded = INT_MAX;
	int iBestSizeKey = INT_MAX;
	double dBestSetPressure = DBL_MAX;
	bool bAtLeastOneFit = false;
	_string str;
	LPARAM lpParam;

	for( BOOL bContinue = pclIndSelSafetyValveParams->m_SafetyValveList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelSafetyValveParams->m_SafetyValveList.GetNext( str, lpParam ) )
	{
		CDB_SafetyValveCooling *pclSafetyValveCooling = dynamic_cast<CDB_SafetyValveCooling *>( (CData *)lpParam );
		ASSERT( NULL != pclSafetyValveCooling );

		if( NULL == pclSafetyValveCooling )
		{
			continue;
		}

		// Exclude those valves that have not the same set pressure.
		if( pclIndSelSafetyValveParams->m_dUserSetPressureChoice != 0.0
				&& pclIndSelSafetyValveParams->m_dUserSetPressureChoice != pclSafetyValveCooling->GetSetPressure() )
		{
			pclIndSelSafetyValveParams->m_SafetyValveList.Delete();
			continue;
		}

		int iNbrSafetyValve = 0;
		double dPowerLimit = pclSafetyValveCooling->GetPowerLimit( _T( "" ), pclIndSelSafetyValveParams->m_strNormID );

		if( 0.0 == dPowerLimit )
		{
			continue;
		}

		iNbrSafetyValve = (int)ceil( pclIndSelSafetyValveParams->m_dInstalledPower / dPowerLimit );

		if( iNbrSafetyValve < 1 || iNbrSafetyValve > pclIndSelSafetyValveParams->m_pTADS->GetpTechParams()->GetMaxSafetyValveInParallel() )
		{
			continue;
		}

		// HYS-1091: If more than one safety valve is used in parallel, the smaller unit must have a blow-of capacity at least 40% of the total.
		if( iNbrSafetyValve > 1 && dPowerLimit < ( 0.4 * pclIndSelSafetyValveParams->m_dInstalledPower ) )
		{
			continue;
		}

		// Create a new entry in pre-selection linked list.
		CSelectedSafetyValve *pclSelSafetyValve = new CSelectedSafetyValve();
		ASSERT( NULL != pclSelSafetyValve );

		if( NULL == pclSelSafetyValve )
		{
			continue;
		}

		pclSelSafetyValve->SetProductIDPtr( pclSafetyValveCooling->GetIDPtr() );
		pclSelSafetyValve->SetQuantityNeeded( iNbrSafetyValve );

		// First we take the safety valve for which we have the lowest needed number.
		// Second, if with the current safety valve we need the same number, we will take the lower one in term of size.
		// Third, if with the current safety valve we need the same number and it's the same size, we will take the lower one in term of set pressure.
		if( iNbrSafetyValve < iBestQuantityNeeded
			|| ( iNbrSafetyValve == iBestQuantityNeeded && pclSafetyValveCooling->GetSizeKey() < iBestSizeKey )
			|| ( iNbrSafetyValve == iBestQuantityNeeded && pclSafetyValveCooling->GetSizeKey() == iBestSizeKey && pclSafetyValveCooling->GetSetPressure() < dBestSetPressure ) )
		{
			pclBestSafetyValve = pclSelSafetyValve;
			iBestSizeKey = pclSafetyValveCooling->GetSizeKey();
			dBestSetPressure = pclSafetyValveCooling->GetSetPressure();
			iBestQuantityNeeded = iNbrSafetyValve;
			bBestFound = true;
		}

		CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclSafetyValveCooling, pclSelSafetyValve );
	}

	if( NULL != pclBestSafetyValve )
	{
		pclBestSafetyValve->SetFlag( CSelectedBase::eBest, true );
	}

	for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
	{
		CSelectedSafetyValve *pSelTap = (CSelectedSafetyValve *)m_cIter->second;
		ASSERT( NULL != pSelTap );

		if( NULL == pSelTap )
		{
			continue;
		}

		CDB_SafetyValveCooling *pclSafetyValveCooling = dynamic_cast<CDB_SafetyValveCooling *>( pSelTap->GetProductIDPtr().MP );
		ASSERT( NULL != pclSafetyValveCooling );

		if( NULL == pclSafetyValveCooling )
		{
			delete pSelTap;
			continue;
		}

		int iSizeKey = pclSafetyValveCooling->GetSizeKey();

		// Accept only one size above the best.
		if( iSizeKey > iBestSizeKey + 1 )
		{
			delete pSelTap;
			continue;
		}

		CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pclSafetyValveCooling, pSelTap );
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	return m_MMapKeyDecreasing.size();
}

int CSelectSafetyValveList::_SelectSafetyValveSolar( CIndSelSafetyValveParams *pclIndSelSafetyValveParams, bool &bBestFound )
{
	if( NULL == pclIndSelSafetyValveParams || NULL == pclIndSelSafetyValveParams->m_pTADS || NULL == pclIndSelSafetyValveParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSelSafetyValveParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	bBestFound = false;

	// Get pointers on required database tables.
	CTable *pTab = (CTable*)( pclIndSelSafetyValveParams->m_pTADB->Get( _T("SAFETYVALVE_TAB") ).MP );
	ASSERT( NULL != pTab );

	if( NULL == pTab )
	{
		return 0;
	}

	// Clean all previous data concerning safety valve selection.
	Clean();

	CSelectedSafetyValve *pclBestSafetyValve = NULL;
	int iBestQuantityNeeded = INT_MAX;
	int iBestSizeKey = INT_MAX;
	double dBestSetPressure = DBL_MAX;
	bool bAtLeastOneFit = false;
	_string str;
	LPARAM lpParam;

	for( BOOL bContinue = pclIndSelSafetyValveParams->m_SafetyValveList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSelSafetyValveParams->m_SafetyValveList.GetNext( str, lpParam ) )
	{
		CDB_SafetyValveSolar *pclSafetyValveSolar = dynamic_cast<CDB_SafetyValveSolar *>( (CData *)lpParam );
		ASSERT( NULL != pclSafetyValveSolar );

		if( NULL == pclSafetyValveSolar )
		{
			continue;
		}

		// Exclude those valves that have not the same set pressure.
		if( pclIndSelSafetyValveParams->m_dUserSetPressureChoice != 0.0
				&& pclIndSelSafetyValveParams->m_dUserSetPressureChoice != pclSafetyValveSolar->GetSetPressure() )
		{
			pclIndSelSafetyValveParams->m_SafetyValveList.Delete();
			continue;
		}

		int iNbrSafetyValve = 0;
		double dPowerLimit = pclSafetyValveSolar->GetPowerLimit();
		double dCollectorLimit = pclSafetyValveSolar->GetCollectorLimit();

		// Verify first the power limit.
		iNbrSafetyValve = (int)ceil( pclIndSelSafetyValveParams->m_dInstalledPower / dPowerLimit );

		if( iNbrSafetyValve < 1 || iNbrSafetyValve > pclIndSelSafetyValveParams->m_pTADS->GetpTechParams()->GetMaxSafetyValveInParallel() )
		{
			continue;
		}

		// Now verify the collector limit.
		if( pclIndSelSafetyValveParams->m_dInstalledCollector > dCollectorLimit )
		{
			continue;
		}

		// HYS-1091: If more than one safety valve is used in parallel, the smaller unit must have a blow-of capacity at least 40% of the total.
		if( iNbrSafetyValve > 1 && dPowerLimit < ( 0.4 * pclIndSelSafetyValveParams->m_dInstalledPower ) )
		{
			continue;
		}

		// Create a new entry in pre-selection linked list.
		CSelectedSafetyValve *pclSelSafetyValve = new CSelectedSafetyValve();
		ASSERT( NULL != pclSelSafetyValve );

		if( NULL == pclSelSafetyValve )
		{
			continue;
		}

		pclSelSafetyValve->SetProductIDPtr( pclSafetyValveSolar->GetIDPtr() );
		pclSelSafetyValve->SetQuantityNeeded( iNbrSafetyValve );

		// First we take the safety valve for which we have the lowest needed number.
		// Second, if with the current safety valve we need the same number, we will take the lower one in term of size.
		// Third, if with the current safety valve we need the same number and it's the same size, we will take the lower one in term of set pressure.
		if( iNbrSafetyValve < iBestQuantityNeeded
				|| ( iNbrSafetyValve == iBestQuantityNeeded && pclSafetyValveSolar->GetSizeKey() < iBestSizeKey )
				|| ( iNbrSafetyValve == iBestQuantityNeeded && pclSafetyValveSolar->GetSizeKey() == iBestSizeKey && pclSafetyValveSolar->GetSetPressure() < dBestSetPressure ) )
		{
			pclBestSafetyValve = pclSelSafetyValve;
			iBestSizeKey = pclSafetyValveSolar->GetSizeKey();
			dBestSetPressure = pclSafetyValveSolar->GetSetPressure();
			iBestQuantityNeeded = iNbrSafetyValve;
			bBestFound = true;
		}

		CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclSafetyValveSolar, pclSelSafetyValve );
	}

	if( NULL != pclBestSafetyValve )
	{
		pclBestSafetyValve->SetFlag( CSelectedBase::eBest, true );
	}

	for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
	{
		CSelectedSafetyValve *pSelTap = (CSelectedSafetyValve *)m_cIter->second;
		ASSERT( NULL != pSelTap );

		if( NULL == pSelTap )
		{
			continue;
		}

		CDB_SafetyValveSolar *pclSafetyValveSolar = dynamic_cast<CDB_SafetyValveSolar *>( pSelTap->GetProductIDPtr().MP );
		ASSERT( NULL != pclSafetyValveSolar );

		if( NULL == pclSafetyValveSolar )
		{
			delete pSelTap;
			continue;
		}

		int iSizeKey = pclSafetyValveSolar->GetSizeKey();

		// Accept only one size above the best.
		if( iSizeKey > iBestSizeKey + 1 )
		{
			delete pSelTap;
			continue;
		}

		CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pclSafetyValveSolar, pSelTap );
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	return m_MMapKeyDecreasing.size();
}

////////////////////////////////////////////////////////////////
//	CSelect6WayValveList
////////////////////////////////////////////////////////////////
CSelect6WayValveList::CSelect6WayValveList()
	: CSelectCtrlList()
{
	m_pSelectedPICVList1 = NULL;
	m_pSelectedPICVList2 = NULL;
	m_pSelectedBVList1 = NULL;
	m_pSelectedBVList2 = NULL;
	Clean();
}

CSelect6WayValveList::~CSelect6WayValveList()
{
	if( NULL != m_pSelectedPICVList1 )
	{
		delete m_pSelectedPICVList1;
		m_pSelectedPICVList1 = NULL;
	}

	if( NULL != m_pSelectedPICVList2 )
	{
		delete m_pSelectedPICVList2;
		m_pSelectedPICVList2 = NULL;
	}

	if( NULL != m_pSelectedBVList1 )
	{
		delete m_pSelectedBVList1;
		m_pSelectedBVList1 = NULL;
	}

	if( NULL != m_pSelectedBVList2 )
	{
		delete m_pSelectedBVList2;
		m_pSelectedBVList2 = NULL;
	}
}

void CSelect6WayValveList::Clean()
{
	CSelectCtrlList::Clean();

	if( NULL != m_pSelectedPICVList1 )
	{
		delete m_pSelectedPICVList1;
		m_pSelectedPICVList1 = NULL;
	}

	if( NULL != m_pSelectedPICVList2 )
	{
		delete m_pSelectedPICVList2;
		m_pSelectedPICVList2 = NULL;
	}

	if( NULL != m_pSelectedBVList1 )
	{
		delete m_pSelectedBVList1;
		m_pSelectedBVList1 = NULL;
	}

	if( NULL != m_pSelectedBVList2 )
	{
		delete m_pSelectedBVList2;
		m_pSelectedBVList2 = NULL;
	}

	m_iHighestSize = 0;
	m_iLowestSize = 0;
}

int CSelect6WayValveList::Select6WayValve( CIndSel6WayValveParams *pclIndSel6WayValveParams, bool *pbSizeShiftProblem )
{
	if( NULL == pclIndSel6WayValveParams || NULL == pclIndSel6WayValveParams->m_pTADS || NULL == pclIndSel6WayValveParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSel6WayValveParams->m_pTADB || NULL == pbSizeShiftProblem )
	{
		ASSERTA_RETURN( 0 );
	}

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return 0;
	}

	// Get pointers on required database tables.
	CTable *pTab = (CTable *)( pclIndSel6WayValveParams->m_pTADB->Get( _T("6WAYCTRLVALV_TAB") ).MP );

	if( NULL == pTab )
	{
		ASSERTA_RETURN( 0 );
	}

	if( false == m_clSelectPipeList.GetPipeSizeBound() )
	{
		return 0;
	}

	CDB_Pipe *pSmallestPipe = m_clSelectPipeList.GetpSmallestPipe();

	if( NULL == pSmallestPipe )
	{
		return 0;
	}

	CDB_Pipe *pBiggestPipe = m_clSelectPipeList.GetpLargestPipe();

	if( NULL == pBiggestPipe )
	{
		return 0;
	}

	CDS_TechnicalParameter *pclTechParams = pclIndSel6WayValveParams->m_pTADS->GetpTechParams();
	int iSizeShiftUp = 0;
	int iSizeShiftDown = 0;

	// Setup variables to possibly enforce combined SizeShifts
	if( 99 == pclTechParams->GetSizeShiftAbove() ) // Combined SizeShift +3/-3 enforced
	{
		iSizeShiftUp = 3;
		iSizeShiftDown = -3;
	}
	else
	{
		iSizeShiftUp = pclTechParams->GetSizeShiftAbove();
		iSizeShiftDown = pclTechParams->GetSizeShiftBelow();
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclIndSel6WayValveParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN = pclTableDN->GetBiggestDN();
	m_iHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSel6WayValveParams->m_pTADB ) + iSizeShiftUp );
	m_iLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSel6WayValveParams->m_pTADB ) + iSizeShiftDown );

	int iMaxHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclIndSel6WayValveParams->m_pTADB ) + pclTechParams->GetMaxSizeShift() );
	int iMaxLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclIndSel6WayValveParams->m_pTADB ) - pclTechParams->GetMaxSizeShift() );

	int iDN15 = pclTableDN->GetSize( _T("DN_15") );

	if( m_iHighestSize < iDN15 )
	{
		m_iHighestSize = iDN15;
	}

	if( iMaxHighestSize < iDN15 )
	{
		iMaxHighestSize = iDN15;
	}

	// Prepare an array with a entry for each 6-way valve size.
	_string str;
	LPARAM lpParam;
	
	for( BOOL bContinue = pclIndSel6WayValveParams->m_e6WayValveList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSel6WayValveParams->m_e6WayValveList.GetNext( str, lpParam ) )
	{
		CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( (CData *)lpParam );
		ASSERT( NULL != pcl6WayValve );

		if( NULL == pcl6WayValve )
		{
			continue;
		}

		int iSizeKey = pcl6WayValve->GetSizeKey();
		
		// If entry doesn't yet exist, create it and set INT_MAX value. The good values will be set after below.
		if( 0 == m_mapPriority.count( iSizeKey ) )
		{
			m_mapPriority[ iSizeKey ] = INT_MAX;
		}
	}

	double dBiggestFlow = 0.0;
	CWaterChar *pclBiggestFlowWaterChar = NULL;

	if( pclIndSel6WayValveParams->GetCoolingFlow() >= pclIndSel6WayValveParams->GetHeatingFlow() )
	{
		dBiggestFlow = pclIndSel6WayValveParams->GetCoolingFlow();
		pclBiggestFlowWaterChar = &pclIndSel6WayValveParams->GetCoolingWaterChar();
	}
	else
	{
		dBiggestFlow = pclIndSel6WayValveParams->GetHeatingFlow();
		pclBiggestFlowWaterChar = &pclIndSel6WayValveParams->GetHeatingWaterChar();
	}

	ASSERT( NULL != pclBiggestFlowWaterChar && dBiggestFlow > 0.0 );

	bool bAtLeastOneFit = false;
	*pbSizeShiftProblem = false;

	for( BOOL bContinue = pclIndSel6WayValveParams->m_e6WayValveList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pclIndSel6WayValveParams->m_e6WayValveList.GetNext( str, lpParam ) )
	{
		CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( (CData *)lpParam );
		ASSERT( NULL != pcl6WayValve );
		
		if( NULL == pcl6WayValve )
		{
			continue;
		}

		// Get Kvs of the valve.
		if( pcl6WayValve->GetKvs() <= 0.0 )
		{
			continue;
		}

		if( false == pcl6WayValve->IsSelectable( true ) )
		{
			continue;
		}

		int iDN = pcl6WayValve->GetSizeKey();

		// Do a test to verify valves will exist with the maximum boundaries.
		// If not, the problem is not a sizeShift problem.
		if( iDN < iMaxLowestSize || iDN > iMaxHighestSize )
		{
			pclIndSel6WayValveParams->m_e6WayValveList.Delete();
			continue;
		}

		// Means that at least one 6-way valve size is in the maximum boundaries [-4, +4]. If any 6-way valve size is in the range [iLowestSize, iHighestSize],
		// 'Select' will return 0 but with 'SizeShiftProblem' set to 'true' we signal that it is possible to find 6-way valve by changing above and below
		// size range and technical parameter.
		if( false == bAtLeastOneFit )
		{
			*pbSizeShiftProblem = true;
		}

		if( iDN >= m_iLowestSize && iDN <= m_iHighestSize )
		{
			// There is no problem with the size shift.
			*pbSizeShiftProblem = false;
			bAtLeastOneFit = true;

			double dQmax = pcl6WayValve->GetQmax();

			// For 6-way valve, we put a 'Qmax' value to be able to limit the choice with TA-Compact-P and TA-Modulator 
			// when the application type is EQM. For all other application type, we don't need this limitation. We can check
			// in regards to Kvs and Dpmax of the valve to get Qmax.
			if( e6Way_EQMControl != pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
			{
				// For density, we take the worst case that is in the higher temperature.
				double dRho = pclIndSel6WayValveParams->m_HeatingWC.GetDens();
				dQmax = CalcqT( pcl6WayValve->GetKvs(), pcl6WayValve->GetDpmax(), dRho );
			}

			if( dBiggestFlow > dQmax )
			{
				continue;
			}
		
			// Create a new entry in pre-selection linked list.
			CSelectedValve *pclSelected6WayValve = new CSelectedValve();

			if( NULL == pclSelected6WayValve )
			{
				ASSERTA_RETURN( 0 );
			}

			pclSelected6WayValve->SetProductIDPtr( pcl6WayValve->GetIDPtr() );

			// Fill the fields.
			double dMinHeatingTemp = pclIndSel6WayValveParams->GetHeatingWaterChar().GetTemp() - pclIndSel6WayValveParams->GetHeatingDT();
			double dMaxHeatingTemp = pclIndSel6WayValveParams->GetHeatingWaterChar().GetTemp();
			double dMinCoolingTemp = pclIndSel6WayValveParams->GetCoolingWaterChar().GetTemp();
			double dMaxCoolingTemp = pclIndSel6WayValveParams->GetCoolingWaterChar().GetTemp() + pclIndSel6WayValveParams->GetCoolingDT();

			// Check the temperature for the heating.
			pclSelected6WayValve->SetFlag( CSelectedBase::Flags::eTemperature, dMinHeatingTemp < pcl6WayValve->GetTmin() || dMaxHeatingTemp > pcl6WayValve->GetTmax() );

			// Check now the temperature for the cooling.
			pclSelected6WayValve->SetFlag( CSelectedBase::Flags::eTemperature, dMinCoolingTemp < pcl6WayValve->GetTmin() || dMaxCoolingTemp > pcl6WayValve->GetTmax() );
			
			pclSelected6WayValve->SetFlag( CSelectedBase::Flags::eBest, false );

			double dDp = CalcDp( dBiggestFlow, pcl6WayValve->GetKvs(), pclBiggestFlowWaterChar->GetDens() );
			pclSelected6WayValve->SetDp( dDp );

			// Add the valve in the pre-selection list.
			CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pcl6WayValve, pclSelected6WayValve );
		}
		else
		{
			pclIndSel6WayValveParams->m_e6WayValveList.Delete();
		}
	}
	
	if( m_PreselMMapKeyDecreasing.size() > 0 )
	{
		// Between two valves, we will take the one that has the pressure drop the closest to the Dp reference for 6-way valve.
		double dBestDelta = DBL_MAX;
		double dDpRefForBest6Way = pclTechParams->GetDpRefForBest6Way();
	
		// For the moment, we don't display results in regards to commercial priority (We have only 3 6-Way valves!!).
		for( int iPhase = 0; iPhase < 2; iPhase++ )
		{
			for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
			{
				CSelectedValve *pSelTap = (CSelectedValve *)m_cIter->second;
				ASSERT( NULL != pSelTap );

				if( NULL == pSelTap )
				{
					continue;
				}

				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct*>( pSelTap->GetpData() );
				ASSERT( NULL != pTAP );

				if( NULL == pTAP )
				{
					continue;
				}

				switch( iPhase )
				{
					// Check the lowest delta between pressure drop across the valve and the Dp reference value.
					case 0:	
						// Determine BestDpmin - smallest Dpmin above DpCMinDp.
						if( abs( pSelTap->GetDp() - dDpRefForBest6Way ) < dBestDelta )
						{
							dBestDelta = abs( pSelTap->GetDp() - dDpRefForBest6Way );
						}
					
						// Transfer valve into the final list with good priority key.
						CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pTAP, pSelTap );
					
						break;

					// Flag the best one.
					case 1:

						if( dBestDelta == abs( pSelTap->GetDp() - dDpRefForBest6Way ) )
						{
							pSelTap->SetFlag( CSelectedBase::Flags::eBest, true );
						}

						break;
				}
			}
		}
	
		// Clean allocated memory.
		m_PreselMMapKeyDecreasing.clear();
	}

	return m_MMapKeyDecreasing.size();
}

int CSelect6WayValveList::SelectPIBCValve( CIndSel6WayValveParams *pclIndSel6WayValveParams, CSelectedValve *pclSelected6WayValve, SideDefinition eSideDefinition, bool *pbSizeShiftProblem )
{
	if( NULL == pclIndSel6WayValveParams || NULL == pclIndSel6WayValveParams->m_pTADS || NULL == pclIndSel6WayValveParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSel6WayValveParams->m_pTADB || NULL == pbSizeShiftProblem )
	{
		ASSERTA_RETURN( 0 );
	}

	if( NULL == pclSelected6WayValve )
	{
		ASSERTA_RETURN( 0 );
	}

	if( e6Way_Alone == pclIndSel6WayValveParams->m_e6WayValveSelectionMode || e6Way_OnOffControlWithSTAD == pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		// No PIBCV in these both cases.
		ASSERTA_RETURN( 0 );
	}

	int iCount = 0;

	if( e6Way_EQMControl == pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		if( true == pclIndSel6WayValveParams->m_bOnlyForSet )
		{
			// HYS-1877: Select PIBCV for Set
			iCount = _SelectPIBCVSetEQMControl( pclIndSel6WayValveParams, pclSelected6WayValve, pbSizeShiftProblem );
		}
		else
		{
			iCount = _SelectPIBCValveEQMControl( pclIndSel6WayValveParams, pclSelected6WayValve, eSideDefinition, pbSizeShiftProblem );
		}
	}
	else 
	{
		iCount = _SelectPIBCValveOnOffControl( pclIndSel6WayValveParams, pclSelected6WayValve, eSideDefinition, pbSizeShiftProblem );
	}

	return iCount;
}

int CSelect6WayValveList::SelectBalancingValve( CIndSel6WayValveParams *pclIndSel6WayValveParams, CSelectedValve *pclSelected6WayValve, SideDefinition eSideDefinition, bool *pbSizeShiftProblem )
{
	if( NULL == pclIndSel6WayValveParams || NULL == pclIndSel6WayValveParams->m_pTADS || NULL == pclIndSel6WayValveParams->m_pTADS->GetpTechParams()
			|| NULL == pclIndSel6WayValveParams->m_pTADB || NULL == pbSizeShiftProblem )
	{
		ASSERTA_RETURN( 0 );
	}

	if( NULL == pclSelected6WayValve || NULL == dynamic_cast<CDB_6WayValve *>( pclSelected6WayValve->GetProductIDPtr().MP ) )
	{
		ASSERTA_RETURN( 0 );
	}

	CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( pclSelected6WayValve->GetProductIDPtr().MP );

	if( e6Way_Alone == pclIndSel6WayValveParams->m_e6WayValveSelectionMode && e6Way_EQMControl == pclIndSel6WayValveParams->m_e6WayValveSelectionMode
			&& e6Way_OnOffControlWithPIBCV == pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		// No PIBCV in these three cases.
		ASSERTA_RETURN( 0 );
	}

	// Check the side definition.
	if( BothSide == eSideDefinition )
	{
		// If we are working with application type B (Control via the actuator TA-MC106Y and the TA-6-way valve) or C (Control via the actuator TA-MC106Y and the 
		// TA-6-way valve), we have one valve for heating and one for cooling.
		ASSERTA_RETURN( 0 );
	}

	CSelectList *&pclSelectedBVList = ( CoolingSide == eSideDefinition ) ? m_pSelectedBVList2 : m_pSelectedBVList1;
	
	double dFlow = ( HeatingSide == eSideDefinition ) ? pclIndSel6WayValveParams->GetHeatingFlow() : pclIndSel6WayValveParams->GetCoolingFlow();
	CWaterChar *pclWaterChar = ( HeatingSide == eSideDefinition ) ? &pclIndSel6WayValveParams->GetHeatingWaterChar() : &pclIndSel6WayValveParams->GetCoolingWaterChar();
	ASSERT( dFlow > 0.0 && NULL != pclWaterChar );

	if( NULL != pclSelectedBVList )
	{
		delete pclSelectedBVList;
		pclSelectedBVList = NULL;
	}

	pclSelectedBVList = new CSelectList();

	if( NULL == pclSelectedBVList || NULL == pclSelectedBVList->GetSelectPipeList() )
	{
		return 0;
	}

	CIndSelBVParams clIndSelBVParams;

	// To have a proper initialization of the base variables (like 'm_pTADB') we need to copy from 'pclIndSel6WayValveParams'.
	clIndSelBVParams.CopyFrom( pclIndSel6WayValveParams );

	// But needed variables for balancing valve research are well in the 'pclIndSel6WayValveParams->m_clIndSelBVParams'.
	clIndSelBVParams.m_strComboTypeID = pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboTypeID;
	clIndSelBVParams.m_strComboFamilyID = pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboFamilyID;
	clIndSelBVParams.m_strComboMaterialID = pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboMaterialID;
	clIndSelBVParams.m_strComboConnectID = pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboConnectID;
	clIndSelBVParams.m_strComboVersionID = pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboVersionID;
	clIndSelBVParams.m_strComboPNID = pclIndSel6WayValveParams->m_clIndSelBVParams.m_strComboPNID;

	// Don't forget to put the right fluid characteristic before to launch the search.
	if( HeatingSide == eSideDefinition )
	{
		clIndSelBVParams.m_dFlow = pclIndSel6WayValveParams->GetHeatingFlow();
		clIndSelBVParams.m_dPower = pclIndSel6WayValveParams->GetHeatingPower();
		clIndSelBVParams.m_dDT = pclIndSel6WayValveParams->GetHeatingDT();
		clIndSelBVParams.m_WC = pclIndSel6WayValveParams->GetHeatingWaterChar();
	}
	else
	{
		clIndSelBVParams.m_dFlow = pclIndSel6WayValveParams->GetCoolingFlow();
		clIndSelBVParams.m_dPower = pclIndSel6WayValveParams->GetCoolingPower();
		clIndSelBVParams.m_dDT = pclIndSel6WayValveParams->GetCoolingDT();
		clIndSelBVParams.m_WC = pclIndSel6WayValveParams->GetCoolingWaterChar();
	}

	// HYS-832: the 'CSelectPipeList::SelectPipes' method will internally create a 'CSelectPipe' object by passing as argument a pointer
	// on a 'CProductSelelectionParameters' object that is itself passed as first argument in 'SelectPipes'. If we pass 'clIndSelBVParams',
	// this object will be destroyed at the end of this method and than pointers in 'CSelectPipe' will be no more valid.
	// This is why here we need to pass the 'pclIndSel6WayValveParams' variable.
	pclSelectedBVList->GetSelectPipeList()->SelectPipes( pclIndSel6WayValveParams, dFlow, pclWaterChar );

	// Preselect regulating valve.
	// HYS-1375 : Filter about Bv family must be checked 
	CString strBvFamilyID = clIndSelBVParams.m_strComboFamilyID;
	if( true == strBvFamilyID.IsEmpty() )
	{
		strBvFamilyID = pcl6WayValve->GetBvFamiliesGroupTableIDPtr().ID;
	}
	clIndSelBVParams.m_pTADB->GetBVFor6WayValveList( &clIndSelBVParams.m_BvList, (LPCTSTR)strBvFamilyID,
			(LPCTSTR)clIndSelBVParams.m_strComboMaterialID, (LPCTSTR)clIndSelBVParams.m_strComboConnectID, (LPCTSTR)clIndSelBVParams.m_strComboVersionID, 
			clIndSelBVParams.m_eFilterSelection );

	pclSelectedBVList->GetManBvList( &clIndSelBVParams );

	return pclSelectedBVList->GetCount();
}

bool CSelect6WayValveList::IsPIBCValveExist( SideDefinition eSideDefinition )
{
	bool bIsExist = false;

	switch( eSideDefinition )
	{
		case BothSide:
		case HeatingSide:

			if( NULL != m_pSelectedPICVList1 && m_pSelectedPICVList1->m_MMapKeyDecreasing.size() > 0 )
			{
				bIsExist = true;
			}

			break;

		case CoolingSide:

			if( NULL != m_pSelectedPICVList2 && m_pSelectedPICVList2->m_MMapKeyDecreasing.size() > 0 )
			{
				bIsExist = true;
			}

			break;
	}

	return bIsExist;
}

CSelectPICVList *CSelect6WayValveList::GetPICVList( SideDefinition eSideDefinition )
{
	CSelectPICVList *pclSelectPICVList = NULL;

	switch( eSideDefinition )
	{
		case BothSide:
		case HeatingSide:
			pclSelectPICVList = m_pSelectedPICVList1;
			break;

		case CoolingSide:
			pclSelectPICVList = m_pSelectedPICVList2;
			break;
	}

	return pclSelectPICVList;
}

bool CSelect6WayValveList::IsBalancingValveExist( SideDefinition eSideDefinition )
{
	bool bIsExist = false;

	switch( eSideDefinition )
	{
		case BothSide:
		case HeatingSide:

			if( NULL != m_pSelectedBVList1 && m_pSelectedBVList1->m_MMapKeyDecreasing.size() > 0 )
			{
				bIsExist = true;
			}

			break;

		case CoolingSide:

			if( NULL != m_pSelectedBVList2 && m_pSelectedBVList2->m_MMapKeyDecreasing.size() > 0 )
			{
				bIsExist = true;
			}

			break;
	}

	return bIsExist;
}

CSelectList *CSelect6WayValveList::GetBVList( SideDefinition eSideDefinition )
{
	CSelectList *pclSelecBVList = NULL;

	switch( eSideDefinition )
	{
		case BothSide:
		case HeatingSide:
			pclSelecBVList = m_pSelectedBVList1;
			break;

		case CoolingSide:
			pclSelecBVList = m_pSelectedBVList2;
			break;
	}

	return pclSelecBVList;
}


int CSelect6WayValveList::_SelectPIBCVSetEQMControl( CIndSel6WayValveParams* pclIndSel6WayValveParams, CSelectedValve* pclSelected6WayValve, bool* pbSizeShiftProblem )
{
	// Check the side definition.
	if( e6Way_EQMControl != pclIndSel6WayValveParams->m_e6WayValveSelectionMode || false == pclIndSel6WayValveParams->m_bOnlyForSet )
	{
		// If we are working with application type A (Control via the actuator TA-Slider 160 CO, TA-Slider 160 KNX R24 or TA-Slider 160 BACnet/Modbus CO and the
		// pressure independent control valve TA-Modulator) we have only one valve.
		ASSERTA_RETURN( 0 );
	}

	double dBiggestFlow = max( pclIndSel6WayValveParams->GetHeatingFlow(), pclIndSel6WayValveParams->GetCoolingFlow() );
	CWaterChar* pclBiggestFlowWaterChar = &pclIndSel6WayValveParams->GetCoolingWaterChar();
	ASSERT( dBiggestFlow > 0.0 && NULL != pclBiggestFlowWaterChar );

	if( NULL != m_pSelectedPICVList1 )
	{
		delete m_pSelectedPICVList1;
		m_pSelectedPICVList1 = NULL;
	}

	m_pSelectedPICVList1 = new CSelectPICVList();

	if( NULL == m_pSelectedPICVList1 )
	{
		ASSERTA_RETURN( 0 );
	}

	CDB_6WayValve* pcl6WayValve = dynamic_cast<CDB_6WayValve*>(pclSelected6WayValve->GetpData());

	if( NULL == pcl6WayValve || false == pcl6WayValve->IsPartOfaSet() )
	{
		ASSERTA_RETURN( 0 );
	}

	if( _NULL_IDPTR == pcl6WayValve->GetPicvGroupTableIDPtr() )
	{
		ASSERTA_RETURN( 0 );
	}

	CTable* pclCompatibleValveTable = dynamic_cast<CTable*>(pcl6WayValve->GetPicvGroupTableIDPtr().MP);

	if( NULL == pclCompatibleValveTable )
	{
		ASSERTA_RETURN( 0 );
	}

	// The pipe selection has been already done in the 'Select6WayValve' method.
	m_mapPriority.clear();
	for( IDPTR IDPtr = pclCompatibleValveTable->GetFirst(); _NULL_IDPTR != IDPtr; IDPtr = pclCompatibleValveTable->GetNext( IDPtr.MP ) )
	{
		CDB_PIControlValve* pclPIBCValve = dynamic_cast<CDB_PIControlValve*>((CData*)IDPtr.MP);
		ASSERT( NULL != pclPIBCValve );

		if( NULL == pclPIBCValve )
		{
			continue;
		}

		int iSizeKey = pclPIBCValve->GetSizeKey();

		// m_mapPriority store for each size the minimum priority level (most important)
		if( 0 == m_mapPriority.count( iSizeKey ) )
		{
			m_mapPriority[iSizeKey] = INT_MAX;
		}
	}

	CDB_6WayValveActuatorCompatibility* pcl6WayValveActrComp = dynamic_cast<CDB_6WayValveActuatorCompatibility*>(TASApp.GetpTADB()->Get( _T( "6WAY_ACTR_COMP" ) ).MP);
	ASSERT( NULL != pcl6WayValveActrComp );

	CDB_6WayValveActuatorCompatibility::ActuatorFilters* pActuatorFilters = pcl6WayValveActrComp->GetOneActutorFilters( pclIndSel6WayValveParams->m_e6WayValveSelectionMode );

	if( NULL == pActuatorFilters )
	{
		ASSERTA_RETURN( 0 );
	}

	// For now, there is only TA-Slider 160 for TA-Compact-P DN10/25 and TA-Modulator DN10/32.
	CDB_ElectroActuator* pclTASlider160 = dynamic_cast<CDB_ElectroActuator*>(TASApp.GetpTADB()->Get( _T( "SLIDER160-STD1M" ) ).MP);
	ASSERT( NULL != pclTASlider160 );

	CTableSet* pcl6WayValveTableSet = ((CDB_Product*)pcl6WayValve)->GetTableSet();
	if( NULL == pcl6WayValveTableSet )
	{
		return 0;
	}

	for( IDPTR idptr = pcl6WayValveTableSet->GetFirst(); _T( '\0' ) != *idptr.ID; idptr = pcl6WayValveTableSet->GetNext() )
	{
		CDB_Set* pTemp = dynamic_cast<CDB_Set*>(idptr.MP);
		if( NULL == pTemp || false == pTemp->IsSelectable( true ) )
		{
			continue;
		}

		if( 0 != IDcmp( pTemp->GetFirstIDPtr().ID, pcl6WayValve->GetIDPtr().ID ) )
		{
			continue;
		}

		// find the PIBCV
		CDB_Set* pclPibcvSet = (CDB_Set*)pclIndSel6WayValveParams->m_pTADB->Get( pTemp->GetNextID() ).MP;
		if( NULL == pclPibcvSet )
		{
			continue;
		}

		bool bPIBCVSet_found = false;

		CDB_PIControlValve* pclPIBCValve = dynamic_cast<CDB_PIControlValve*>(pclPibcvSet->GetFirstIDPtr().MP);

		if( NULL == pclPIBCValve || false == pclPIBCValve->IsSelectable( true ) )
		{
			continue;
		}

		if( pclPIBCValve->GetSizeKey() < m_iLowestSize || pclPIBCValve->GetSizeKey() > m_iHighestSize )
		{
			continue;
		}

		if( dBiggestFlow > pclPIBCValve->GetQmax() )
		{
			continue;
		}

		if( false == pclPIBCValve->IsPartOfaSet() )
		{
			continue;
		}

		// Verify values chosen in combo left tab.
		if( false == pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboFamilyID.IsEmpty()
			&& pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboFamilyID != pclPIBCValve->GetFamilyID() )
		{
			continue;
		}

		if( false == pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboMaterialID.IsEmpty()
			&& pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboMaterialID != pclPIBCValve->GetBodyMaterialID() )
		{
			continue;
		}

		if( false == pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboConnectID.IsEmpty()
			&& pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboConnectID != pclPIBCValve->GetConnectID() )
		{
			continue;
		}

		if( false == pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboVersionID.IsEmpty()
			&& pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboVersionID != pclPIBCValve->GetVersionID() )
		{
			continue;
		}

		if( false == pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboPNID.IsEmpty()
			&& 0 != StringCompare( pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboPNID, pclPIBCValve->GetPNIDPtr().ID ) )
		{
			continue;
		}

		double dMinSettingWithTASlider = pclPIBCValve->GetPICVCharacteristic()->GetMinSettingWithActuator( pclTASlider160 );

		// Verify now with the minimum setting that is possible with TA-Slider.
		// In EQM mode we use the same valve for cooling and heating.
		double dHCooling = pclPIBCValve->GetPresetting( pclIndSel6WayValveParams->GetCoolingFlow(), pclIndSel6WayValveParams->GetCoolingWaterChar().GetDens(),
														pclIndSel6WayValveParams->GetCoolingWaterChar().GetKinVisc() );

		if( dHCooling < dMinSettingWithTASlider )
		{
			continue;
		}

		double dHHeating = pclPIBCValve->GetPresetting( pclIndSel6WayValveParams->GetHeatingFlow(), pclIndSel6WayValveParams->GetHeatingWaterChar().GetDens(),
														pclIndSel6WayValveParams->GetHeatingWaterChar().GetKinVisc() );

		if( dHHeating < dMinSettingWithTASlider )
		{
			continue;
		}

		// Verify actuator
		CDB_ElectroActuator* pclElectroActuator = dynamic_cast<CDB_ElectroActuator*>(pclPibcvSet->GetSecondIDPtr().MP);
		if( NULL == pclElectroActuator || false == pclElectroActuator->IsSelectable( true ) )
		{
			continue;
		}

		// Verify family compatibility.
		if( pActuatorFilters->m_PIBCValveActuatorFamilyIDCompatibilityList.size() > 0 )
		{
			bool bFound = false;

			for( auto& iterFamilyIDPtr : pActuatorFilters->m_PIBCValveActuatorFamilyIDCompatibilityList )
			{
				if( 0 == _tcscmp( pclElectroActuator->GetFamilyID(), iterFamilyIDPtr.ID ) )
				{
					bFound = true;
					break;
				}
			}

			if( false == bFound )
			{
				continue;
			}
		}

		if( true == pclIndSel6WayValveParams->m_bCheckFastConnection && 0 != pclElectroActuator->GetRelayID().CompareNoCase( _T( "RELAY_CO" ) ) )
		{
			// If we want a fast electrical connection but the actuator isn't compatible...
			continue;
		}

		// Verify the value of input signal combo box
		if( false == pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strActuatorInputSignalID.IsEmpty() )
		{
			CTable* pSignalsTab = static_cast<CTable*>(TASApp.GetpTADB()->Get( _T( "SIG_TAB" ) ).MP);
			bool bActuator_found = false;
			for( int iLoop = 0; iLoop < (int)pclElectroActuator->GetNumOfInputSignalsIDptr(); iLoop++ )
			{
				IDPTR InputSignalIDPtr = pclElectroActuator->GetInputSignalsIDPtr( iLoop );

				if( NULL == InputSignalIDPtr.MP )
				{
					continue;
				}

				if( 0 == StringCompare( InputSignalIDPtr.ID, pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strActuatorInputSignalID ) )
				{
					bActuator_found = true;
					break;
				}
			}
			if( false == bActuator_found )
			{
				continue;
			}
		}

		// Create a new entry in pre-selection linked list.
		CSelectedValve* pclSelectedPIBCValve = new CSelectedValve();

		if( NULL == pclSelectedPIBCValve )
		{
			ASSERTA_RETURN( 0 );
		}

		pclSelectedPIBCValve->SetProductIDPtr( pclPIBCValve->GetIDPtr() );

		// Get the Dp min value.
		double dLowestFlow = 0.0;
		double dRhoOfTheLowestFlow = 0.0;

		if( pclIndSel6WayValveParams->GetHeatingFlow() < pclIndSel6WayValveParams->GetCoolingFlow() )
		{
			dLowestFlow = pclIndSel6WayValveParams->GetHeatingFlow();
			dRhoOfTheLowestFlow = pclIndSel6WayValveParams->GetHeatingWaterChar().GetDens();
		}
		else
		{
			dLowestFlow = pclIndSel6WayValveParams->GetCoolingFlow();
			dRhoOfTheLowestFlow = pclIndSel6WayValveParams->GetCoolingWaterChar().GetDens();
		}

		double dDpMin = pclPIBCValve->GetDpmin( dLowestFlow, dRhoOfTheLowestFlow );
		pclSelectedPIBCValve->SetDpMin( dDpMin );

		CDB_PICVCharacteristic* pclPIBCVCharacteristic = pclPIBCValve->GetPICVCharacteristic();

		if( NULL == pclPIBCVCharacteristic )
		{
			ASSERT( 0 );
			continue;
		}

		double dMinHeatingTemp = pclIndSel6WayValveParams->GetHeatingWaterChar().GetTemp() - pclIndSel6WayValveParams->GetHeatingDT();
		double dMaxHeatingTemp = pclIndSel6WayValveParams->GetHeatingWaterChar().GetTemp();
		double dMinCoolingTemp = pclIndSel6WayValveParams->GetCoolingWaterChar().GetTemp();
		double dMaxCoolingTemp = pclIndSel6WayValveParams->GetCoolingWaterChar().GetTemp() + pclIndSel6WayValveParams->GetCoolingDT();

		// Check the temperature for the heating.
		pclSelectedPIBCValve->SetFlag( CSelectedBase::Flags::eTemperature, dMinHeatingTemp < pclPIBCValve->GetTmin() || dMaxHeatingTemp > pclPIBCValve->GetTmax() );

		// Check now the temperature for the cooling.
		pclSelectedPIBCValve->SetFlag( CSelectedBase::Flags::eTemperature, dMinCoolingTemp < pclPIBCValve->GetTmin() || dMaxCoolingTemp > pclPIBCValve->GetTmax() );

		pclSelectedPIBCValve->SetFlag( CSelectedBase::Flags::eBest, false );

		bPIBCVSet_found = true;
		// Add the valve in the pre-selection list.
		CreateKeyAndInsertInMap( &m_pSelectedPICVList1->m_PreselMMapKeyDecreasing, pclPIBCValve, pclSelectedPIBCValve );

		// In case of one PIBCV per 6-way valve we don't neeed to continue.
		if( true == bPIBCVSet_found )
		{
			break;
		}
	}

	if( 0 == m_pSelectedPICVList1->m_PreselMMapKeyDecreasing.size() )
	{
		return 0;
	}

	// We have only one valve, it's the best.
	m_cIter = m_pSelectedPICVList1->m_PreselMMapKeyDecreasing.begin();

	CSelectedValve* pSelTap = (CSelectedValve*)m_cIter->second;
	ASSERT( NULL != pSelTap );

	if( NULL == pSelTap )
	{
		return 0;
	}

	CDB_TAProduct* pTAP = dynamic_cast<CDB_TAProduct*>(pSelTap->GetpData());
	ASSERT( NULL != pTAP );

	if( NULL == pTAP )
	{
		return 0;
	}

	pSelTap->SetFlag( CSelectedBase::Flags::eBest, true );
	CreateKeyAndInsertInMap( &m_pSelectedPICVList1->m_MMapKeyDecreasing, pTAP, pSelTap );

	// Clean allocated memory.
	m_pSelectedPICVList1->m_PreselMMapKeyDecreasing.clear();

	return m_pSelectedPICVList1->m_MMapKeyDecreasing.size();
}

int CSelect6WayValveList::_SelectPIBCValveEQMControl( CIndSel6WayValveParams *pclIndSel6WayValveParams, CSelectedValve *pclSelected6WayValve, SideDefinition eSideDefinition, bool *pbSizeShiftProblem )
{
	// Check the side definition.
	if( e6Way_EQMControl == pclIndSel6WayValveParams->m_e6WayValveSelectionMode && BothSide != eSideDefinition )
	{
		// If we are working with application type A (Control via the actuator TA-Slider 160 CO, TA-Slider 160 KNX R24 or TA-Slider 160 BACnet/Modbus CO and the
		// pressure independent control valve TA-Modulator) we have only one valve.
		ASSERTA_RETURN( 0 );
	}

	double dBiggestFlow = max( pclIndSel6WayValveParams->GetHeatingFlow(), pclIndSel6WayValveParams->GetCoolingFlow() );
	CWaterChar *pclBiggestFlowWaterChar = ( HeatingSide == eSideDefinition ) ? &pclIndSel6WayValveParams->GetHeatingWaterChar() : &pclIndSel6WayValveParams->GetCoolingWaterChar();
	ASSERT( dBiggestFlow > 0.0 && NULL != pclBiggestFlowWaterChar );

	if( NULL != m_pSelectedPICVList1 )
	{
		delete m_pSelectedPICVList1;
		m_pSelectedPICVList1 = NULL;
	}

	m_pSelectedPICVList1 = new CSelectPICVList();

	if( NULL == m_pSelectedPICVList1 )
	{
		ASSERTA_RETURN( 0 );
	}

	CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( pclSelected6WayValve->GetpData() );

	if( NULL == pcl6WayValve ) 
	{
		ASSERTA_RETURN( 0 );
	}

	if( _NULL_IDPTR == pcl6WayValve->GetPicvGroupTableIDPtr() )
	{
		ASSERTA_RETURN( 0 );
	}

	CTable *pclCompatibleValveTable  = dynamic_cast<CTable *>( pcl6WayValve->GetPicvGroupTableIDPtr().MP );

	if( NULL == pclCompatibleValveTable )
	{
		ASSERTA_RETURN( 0 );
	}

	// The pipe selection has been already done in the 'Select6WayValve' method.
	m_mapPriority.clear();

	for( IDPTR IDPtr = pclCompatibleValveTable->GetFirst(); _NULL_IDPTR != IDPtr; IDPtr = pclCompatibleValveTable->GetNext( IDPtr.MP ) )
	{
		CDB_PIControlValve *pclPIBCValve = dynamic_cast<CDB_PIControlValve *>( (CData *)IDPtr.MP );
		ASSERT( NULL != pclPIBCValve );

		if( NULL == pclPIBCValve )
		{
			continue;
		}

		int iSizeKey = pclPIBCValve->GetSizeKey();

		// m_mapPriority store for each size the minimum priority level (most important)
		if( 0 == m_mapPriority.count( iSizeKey ) )
		{
			m_mapPriority[ iSizeKey ] = INT_MAX;
		}
	}

	// HYS-1355: For now, there is only TA-Slider 160 for TA-Compact-P DN10/25 and TA-Modulator DN10/32.
	CDB_ElectroActuator *pclTASlider160 = dynamic_cast<CDB_ElectroActuator *>( TASApp.GetpTADB()->Get( _T("SLIDER160-STD1M") ).MP );
	ASSERT( NULL != pclTASlider160 );

	for( IDPTR IDPtr = pclCompatibleValveTable->GetFirst(); _NULL_IDPTR != IDPtr; IDPtr = pclCompatibleValveTable->GetNext( IDPtr.MP ) )
	{
		CDB_PIControlValve *pclPIBCValve = dynamic_cast<CDB_PIControlValve *>( (CData *)IDPtr.MP );
		ASSERT( NULL != pclPIBCValve );

		if( NULL == pclPIBCValve )
		{
			continue;
		}

		if( false == pclPIBCValve->IsSelectable( true ) )
		{
			continue;
		}

		if( pclPIBCValve->GetSizeKey() < m_iLowestSize || pclPIBCValve->GetSizeKey() > m_iHighestSize )
		{
			continue;
		}

		if( dBiggestFlow > pclPIBCValve->GetQmax() )
		{
			continue;
		}

		// HYS-1252: Verify values chosen in combo left tab.
		if( false == pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboFamilyID.IsEmpty() 
			&& pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboFamilyID != pclPIBCValve->GetFamilyID()  )
		{
			continue;
		}

		if( false == pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboMaterialID.IsEmpty()
			&& pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboMaterialID != pclPIBCValve->GetBodyMaterialID() )
		{
			continue;
		}
		
		if( false == pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboConnectID.IsEmpty() 
			&& pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboConnectID != pclPIBCValve->GetConnectID() )
		{
			continue;
		}
		
		if( false == pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboVersionID.IsEmpty()
			&& pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboVersionID != pclPIBCValve->GetVersionID() )
		{
			continue;
		}
		
		if( false == pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboPNID.IsEmpty()
				&& 0 != StringCompare( pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboPNID, pclPIBCValve->GetPNIDPtr().ID) )
		{
			continue;
		}

		double dMinSettingWithTASlider = pclPIBCValve->GetPICVCharacteristic()->GetMinSettingWithActuator( pclTASlider160 );

		// HYS-1355: Verify now with the minimum setting that is possible with TA-Slider.
		// In EQM mode we use the same valve for cooling and heating.
		double dHCooling = pclPIBCValve->GetPresetting( pclIndSel6WayValveParams->GetCoolingFlow(), pclIndSel6WayValveParams->GetCoolingWaterChar().GetDens(), 
				pclIndSel6WayValveParams->GetCoolingWaterChar().GetKinVisc() );

		if( dHCooling < dMinSettingWithTASlider )
		{
			continue;
		}

		double dHHeating = pclPIBCValve->GetPresetting( pclIndSel6WayValveParams->GetHeatingFlow(), pclIndSel6WayValveParams->GetHeatingWaterChar().GetDens(), 
				pclIndSel6WayValveParams->GetHeatingWaterChar().GetKinVisc() );

		if( dHHeating < dMinSettingWithTASlider )
		{
			continue;
		}

		// Create a new entry in pre-selection linked list.
		CSelectedValve *pclSelectedPIBCValve = new CSelectedValve();

		if( NULL == pclSelectedPIBCValve )
		{
			ASSERTA_RETURN( 0 );
		}

		pclSelectedPIBCValve->SetProductIDPtr( pclPIBCValve->GetIDPtr() );

		// Get the Dp min value.
		double dLowestFlow = 0.0;
		double dRhoOfTheLowestFlow = 0.0;
			
		if( pclIndSel6WayValveParams->GetHeatingFlow() < pclIndSel6WayValveParams->GetCoolingFlow() )
		{
			dLowestFlow = pclIndSel6WayValveParams->GetHeatingFlow();
			dRhoOfTheLowestFlow = pclIndSel6WayValveParams->GetHeatingWaterChar().GetDens();
		}
		else
		{
			dLowestFlow = pclIndSel6WayValveParams->GetCoolingFlow();
			dRhoOfTheLowestFlow = pclIndSel6WayValveParams->GetCoolingWaterChar().GetDens();
		}

		double dDpMin = pclPIBCValve->GetDpmin( dLowestFlow, dRhoOfTheLowestFlow );
		pclSelectedPIBCValve->SetDpMin( dDpMin );

		CDB_PICVCharacteristic *pclPIBCVCharacteristic = pclPIBCValve->GetPICVCharacteristic();

		if( NULL == pclPIBCVCharacteristic )
		{
			ASSERT( 0 );
			continue;
		}

		double dMinHeatingTemp = pclIndSel6WayValveParams->GetHeatingWaterChar().GetTemp() - pclIndSel6WayValveParams->GetHeatingDT();
		double dMaxHeatingTemp = pclIndSel6WayValveParams->GetHeatingWaterChar().GetTemp();
		double dMinCoolingTemp = pclIndSel6WayValveParams->GetCoolingWaterChar().GetTemp();
		double dMaxCoolingTemp = pclIndSel6WayValveParams->GetCoolingWaterChar().GetTemp() + pclIndSel6WayValveParams->GetCoolingDT();

		// Check the temperature for the heating.
		pclSelectedPIBCValve->SetFlag( CSelectedBase::Flags::eTemperature, dMinHeatingTemp < pclPIBCValve->GetTmin() || dMaxHeatingTemp > pclPIBCValve->GetTmax() );

		// Check now the temperature for the cooling.
		pclSelectedPIBCValve->SetFlag( CSelectedBase::Flags::eTemperature, dMinCoolingTemp < pclPIBCValve->GetTmin() || dMaxCoolingTemp > pclPIBCValve->GetTmax() );
			
		pclSelectedPIBCValve->SetFlag( CSelectedBase::Flags::eBest, false );

		// Add the valve in the pre-selection list.
		CreateKeyAndInsertInMap( &m_pSelectedPICVList1->m_PreselMMapKeyDecreasing, pclPIBCValve, pclSelectedPIBCValve );
	}

	if( 0 == m_pSelectedPICVList1->m_PreselMMapKeyDecreasing.size() )
	{
		return 0;
	}

	if( 1 == m_pSelectedPICVList1->m_PreselMMapKeyDecreasing.size() )
	{
		// We have only one valve, it's the best.
		m_cIter = m_pSelectedPICVList1->m_PreselMMapKeyDecreasing.begin();

		CSelectedValve *pSelTap = (CSelectedValve *)m_cIter->second;
		ASSERT( NULL != pSelTap );

		if( NULL == pSelTap )
		{
			return 0;
		}

		CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pSelTap->GetpData() );
		ASSERT( NULL != pTAP );

		if( NULL == pTAP )
		{
			return 0;
		}

		pSelTap->SetFlag( CSelectedBase::Flags::eBest, true );
		CreateKeyAndInsertInMap( &m_pSelectedPICVList1->m_MMapKeyDecreasing, pTAP, pSelTap );
	}
	else
	{
		// We need to discriminate between several solutions.
		// To discriminate between two valves, we take the sum of the settings between heating and cooling modes.
		// We take thus the setting for the heating mode with the heating flow input by the the user. 
		// And we add the setting for the cooling mode with the cooling flow fixed by the user.
		// And between 2 valves, we take the one that has this sum the highest.
		double dHighestSetting = 0.0;
		CSelectedValve *pclBest = NULL;

		for( m_cIter = m_pSelectedPICVList1->m_PreselMMapKeyDecreasing.begin(); m_cIter != m_pSelectedPICVList1->m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedValve *pclSelectedBalancingValve = dynamic_cast<CSelectedValve *>( m_cIter->second );

			if( NULL == pclSelectedBalancingValve )
			{
				ASSERT( 0 );
				continue;
			}
			
			CDB_PIControlValve *pclPIBControlValve = dynamic_cast<CDB_PIControlValve*>( pclSelectedBalancingValve->GetpData() );

			if( NULL == pclPIBControlValve )
			{
				ASSERT( 0 );
				continue;
			}

			CDB_PICVCharacteristic *pclPIBCVCharacteristic = pclPIBControlValve->GetPICVCharacteristic();

			if( NULL == pclPIBCVCharacteristic )
			{
				ASSERT( 0 );
				continue;
			}

			double dHeatingSetting = pclPIBCVCharacteristic->GetSettingFromQmax( pclIndSel6WayValveParams->GetHeatingFlow() );
			double dCoolingSetting = pclPIBCVCharacteristic->GetSettingFromQmax( pclIndSel6WayValveParams->GetCoolingFlow() );

			if( -1.0 == dHeatingSetting || -1.0 == dCoolingSetting )
			{
				ASSERT( 0 );
				continue;
			}

			if( dHeatingSetting + dCoolingSetting > dHighestSetting )
			{
				dHighestSetting = dHeatingSetting + dCoolingSetting;
				pclBest = pclSelectedBalancingValve;
			}

			CreateKeyAndInsertInMap( &m_pSelectedPICVList1->m_MMapKeyDecreasing, dynamic_cast<CDB_TAProduct*>( pclSelectedBalancingValve->GetpData() ), pclSelectedBalancingValve );
		}

		if( NULL != pclBest )
		{
			pclBest->SetFlag( CSelectedBase::Flags::eBest, true );
		}
	}

	// Clean allocated memory.
	m_pSelectedPICVList1->m_PreselMMapKeyDecreasing.clear();

	return m_pSelectedPICVList1->m_MMapKeyDecreasing.size();
}

int CSelect6WayValveList::_SelectPIBCValveOnOffControl( CIndSel6WayValveParams *pclIndSel6WayValveParams, CSelectedValve *pclSelected6WayValve, SideDefinition eSideDefinition, bool *pbSizeShiftProblem )
{
	if( e6Way_OnOffControlWithPIBCV == pclIndSel6WayValveParams->m_e6WayValveSelectionMode && BothSide == eSideDefinition )
	{
		// If we are working with application type B (Control via the actuator TA-MC106Y and the TA-6-way valve) or C (Control via the actuator TA-MC106Y and the 
		// TA-6-way valve), we have one valve for heating and one for cooling.
		ASSERTA_RETURN( 0 );
	}

	CSelectPICVList *&pclSelectedPIBCVList = ( HeatingSide == eSideDefinition ) ? m_pSelectedPICVList1 : m_pSelectedPICVList2;

	if( NULL != pclSelectedPIBCVList )
	{
		delete pclSelectedPIBCVList;
		pclSelectedPIBCVList = NULL;
	}

	pclSelectedPIBCVList = new CSelectPICVList();

	if( NULL == pclSelectedPIBCVList || NULL == pclSelectedPIBCVList->GetSelectPipeList() )
	{
		return 0;
	}

	CIndSelPIBCVParams clIndSelPIBCVParams;

	// To have a proper initialization of the base variables (like 'm_pTADB') we need to copy from 'pclIndSel6WayValveParams'.
	clIndSelPIBCVParams.CopyFrom( pclIndSel6WayValveParams );

	// But needed variables for PIBCV research are well in the 'pclIndSel6WayValveParams->m_clIndSelPIBCVParams'.
	clIndSelPIBCVParams.m_eCvCtrlType = pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eCvCtrlType;
	clIndSelPIBCVParams.m_strComboTypeID = pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboTypeID;
	clIndSelPIBCVParams.m_strComboFamilyID = pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboFamilyID;
	clIndSelPIBCVParams.m_strComboMaterialID = pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboMaterialID;
	clIndSelPIBCVParams.m_strComboConnectID = pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboConnectID;
	clIndSelPIBCVParams.m_strComboVersionID = pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboVersionID;
	clIndSelPIBCVParams.m_strComboPNID = pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strComboPNID;
	clIndSelPIBCVParams.m_strActuatorPowerSupplyID = pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strActuatorPowerSupplyID;
	clIndSelPIBCVParams.m_strActuatorInputSignalID = pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strActuatorInputSignalID;
	clIndSelPIBCVParams.m_iActuatorFailSafeFunction = pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_iActuatorFailSafeFunction;
	clIndSelPIBCVParams.m_eActuatorDRPFunction = pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_eActuatorDRPFunction;

	// Don't forget to put the right fluid characteristic before to launch the search.
	if( HeatingSide == eSideDefinition )
	{
		clIndSelPIBCVParams.m_dFlow = pclIndSel6WayValveParams->GetHeatingFlow();
		clIndSelPIBCVParams.m_dPower = pclIndSel6WayValveParams->GetHeatingPower();
		clIndSelPIBCVParams.m_dDT = pclIndSel6WayValveParams->GetHeatingDT();
		clIndSelPIBCVParams.m_WC = pclIndSel6WayValveParams->GetHeatingWaterChar();
	}
	else
	{
		clIndSelPIBCVParams.m_dFlow = pclIndSel6WayValveParams->GetCoolingFlow();
		clIndSelPIBCVParams.m_dPower = pclIndSel6WayValveParams->GetCoolingPower();
		clIndSelPIBCVParams.m_dDT = pclIndSel6WayValveParams->GetCoolingDT();
		clIndSelPIBCVParams.m_WC = pclIndSel6WayValveParams->GetCoolingWaterChar();
	}

	CWaterChar *pclWaterChar = ( HeatingSide == eSideDefinition ) ? &pclIndSel6WayValveParams->GetHeatingWaterChar() : &pclIndSel6WayValveParams->GetCoolingWaterChar();
	ASSERT( NULL != pclWaterChar );

	// HYS-832: the 'CSelectPipeList::SelectPipes' method will internally create a 'CSelectPipe' object by passing as argument a pointer
	// on a 'CProductSelelectionParameters' object that is itself passed as first argument in 'SelectPipes'. If we pass 'CIndSelPIBCVParams',
	// this object will be destroyed at the end of this method and than pointers in 'CSelectPipe' will be no more valid.
	// This is why here we need to pass the 'pclIndSel6WayValveParams' variable.
	pclSelectedPIBCVList->GetSelectPipeList()->SelectPipes( pclIndSel6WayValveParams, clIndSelPIBCVParams.m_dFlow, pclWaterChar );

	// Preselect regulating valve.
	clIndSelPIBCVParams.m_CtrlList.PurgeAll();

	int iValveCount = clIndSelPIBCVParams.m_pTADB->GetTaCVList(
			&clIndSelPIBCVParams.m_CtrlList,							// List where to saved
			CTADatabase::eForPiCv,										// Control valve target (cv, hmcv, picv or bcv)
			false, 														// 'true' returns as soon a result is found
			clIndSelPIBCVParams.m_eCV2W3W, 								// Set way number of valve
			(LPCTSTR)clIndSelPIBCVParams.m_strComboTypeID,
			(LPCTSTR)clIndSelPIBCVParams.m_strComboFamilyID, 
			(LPCTSTR)clIndSelPIBCVParams.m_strComboMaterialID,
			(LPCTSTR)clIndSelPIBCVParams.m_strComboConnectID,
			(LPCTSTR)clIndSelPIBCVParams.m_strComboVersionID,
			(LPCTSTR)clIndSelPIBCVParams.m_strComboPNID,
			CDB_ControlProperties::LastCVFUNC, 							// Set the control function (control only, presettable, ...)
			clIndSelPIBCVParams.m_eCvCtrlType,							// Set the control type (on/off, proportional, ...)
			clIndSelPIBCVParams.m_eFilterSelection,
			0,															// DNMin
			INT_MAX,													// DNMax
			false,														// 'true' if it's for hub station.
			NULL,														// 'pProd'.
			clIndSelPIBCVParams.m_bOnlyForSet );						// 'true' if it's only for a set.

	pclSelectedPIBCVList->SelectQ( &clIndSelPIBCVParams, pbSizeShiftProblem );
	
	return pclSelectedPIBCVList->GetCount();
}

//////////////////////////////////////////////////////////////////////////////////
//	Selected container for batch mode
//////////////////////////////////////////////////////////////////////////////////
CBatchSelectBaseList::CBatchSelectBaseList()
	: CSelectedProductList()
{
	Clean();
	m_iSizeShiftUp = 0;
	m_iSizeShiftDown = 0;
	m_pclBestProduct = NULL;
	m_pclBestPipe = NULL;
}

CBatchSelectBaseList::~CBatchSelectBaseList()
{
	Clean();
}

void CBatchSelectBaseList::CleanVecObjects()
{
	if( m_vecObjects.size() > 0 )
	{
		for( int iLoop = 0; iLoop < (int)m_vecObjects.size(); iLoop++ )
		{
			if( NULL != m_vecObjects[iLoop] )
			{
				delete m_vecObjects[iLoop];
			}
		}
	
		m_vecObjects.clear();
	}
}

void CBatchSelectBaseList::TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList )
{
	// Copy list of selected pipes.
	pclIndividualSelectList->GetSelectPipeList()->CopyFrom( GetSelectPipeList() );
}

void CBatchSelectBaseList::Clean()
{
	CSelectedProductList::Clean();

	// In batch selection, we save all 'CSelectedProduct' objects in the 'm_vecObjects' variable.
	// Thus we can clean this vector by deleting these objects and clear other maps.
	CleanVecObjects();
	m_MMapKeyDecreasing.clear();
	m_mmapProductList.clear();
}

void CBatchSelectBaseList::CopyFrom( CSelectedProductList *pclSelectedProductList )
{
	// We can copy only if the objects are the same.
	if( NULL == pclSelectedProductList || NULL == dynamic_cast<CBatchSelectBaseList *>( pclSelectedProductList ) )
	{
		return;
	}

	CSelectedProductList::CopyFrom( pclSelectedProductList );

	CBatchSelectBaseList *pclBatchSelectedProductList = dynamic_cast<CBatchSelectBaseList *>( pclSelectedProductList );

	// For batch selection all the 'CSelectedProduct' objects are saved in the 'm_MMapKeyDecreasing' variable.
	for( auto &iter : pclBatchSelectedProductList->m_MMapKeyDecreasing ) 
	{
		if( NULL == iter.second )
		{
			continue;
		}

		if( CSelectedBase::SBT_Valve == iter.second->GetSelectedBaseType() )
		{
			CSelectedValve *pclSelectedValve = new CSelectedValve();

			if( NULL == pclSelectedValve )
			{
				continue;
			}

			pclSelectedValve->CopyFrom( iter.second );
			m_MMapKeyDecreasing.insert( ProductPair( iter.first, pclSelectedValve ) );
		}
		else if( CSelectedBase::SBT_SafetyValve == iter.second->GetSelectedBaseType() )
		{
			CSelectedSafetyValve *pclSelectedSafetyValve = new CSelectedSafetyValve();

			if( NULL == pclSelectedSafetyValve )
			{
				continue;
			}

			pclSelectedSafetyValve->CopyFrom( iter.second );
			m_MMapKeyDecreasing.insert( ProductPair( iter.first, pclSelectedSafetyValve ) );
		}
	}
}

void CBatchSelectBaseList::BuildSortingKeys( CBatchSelectionParameters *pclBatchSelectionParameters, CTAPSortKey *psKeyTechParamBlw65, CTAPSortKey *psKeyTechParamAbv50 )
{	
	if( NULL == pclBatchSelectionParameters || NULL == psKeyTechParamBlw65 || NULL == psKeyTechParamAbv50 )
	{
		ASSERT_RETURN;
	}

	CDB_StringID *psidType = NULL;
	CDB_StringID *psidFam = NULL;
	CDB_StringID *psidBdy = NULL;
	CDB_StringID *psidConn = NULL;
	CDB_StringID *psidVers = NULL;
	CDB_StringID *psidPN = NULL;

	// Tech Param below DN65.
	psidType = (CDB_StringID *)( pclBatchSelectionParameters->m_pTADB->Get( (LPCTSTR)pclBatchSelectionParameters->m_strComboTypeBelow65ID ).MP );
	psidFam = (CDB_StringID *)( pclBatchSelectionParameters->m_pTADB->Get( (LPCTSTR)pclBatchSelectionParameters->m_strComboFamilyBelow65ID ).MP );
	psidBdy = (CDB_StringID *)( pclBatchSelectionParameters->m_pTADB->Get( (LPCTSTR)pclBatchSelectionParameters->m_strComboMaterialBelow65ID ).MP );
	psidConn = (CDB_StringID *)( pclBatchSelectionParameters->m_pTADB->Get( (LPCTSTR)pclBatchSelectionParameters->m_strComboConnectBelow65ID ).MP );
	psidVers = (CDB_StringID *)( pclBatchSelectionParameters->m_pTADB->Get( (LPCTSTR)pclBatchSelectionParameters->m_strComboVersionBelow65ID ).MP );
	psidPN = (CDB_StringID *)( pclBatchSelectionParameters->m_pTADB->Get( (LPCTSTR)pclBatchSelectionParameters->m_strComboPNBelow65ID ).MP );
	
	psKeyTechParamBlw65->Init( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );

	// Tech Param above DN50
	psidType = (CDB_StringID *)( pclBatchSelectionParameters->m_pTADB->Get( (LPCTSTR)pclBatchSelectionParameters->m_strComboTypeAbove50ID ).MP );
	psidFam = (CDB_StringID *)( pclBatchSelectionParameters->m_pTADB->Get( (LPCTSTR)pclBatchSelectionParameters->m_strComboFamilyAbove50ID ).MP );
	psidBdy = (CDB_StringID *)( pclBatchSelectionParameters->m_pTADB->Get( (LPCTSTR)pclBatchSelectionParameters->m_strComboMaterialAbove50ID ).MP );
	psidConn = (CDB_StringID *)( pclBatchSelectionParameters->m_pTADB->Get( (LPCTSTR)pclBatchSelectionParameters->m_strComboConnectAbove50ID ).MP );
	psidVers = (CDB_StringID *)( pclBatchSelectionParameters->m_pTADB->Get( (LPCTSTR)pclBatchSelectionParameters->m_strComboVersionAbove50ID ).MP );
	psidPN = (CDB_StringID *)( pclBatchSelectionParameters->m_pTADB->Get( (LPCTSTR)pclBatchSelectionParameters->m_strComboPNAbove50ID ).MP );
	
	psKeyTechParamAbv50->Init( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );
}

bool CBatchSelectBaseList::GetPipeSizeShift( CBatchSelectionParameters *pclBatchSelectionParameters, int &iHighestSize, int &iLowestSize )
{
	if( NULL == pclBatchSelectionParameters || NULL == pclBatchSelectionParameters->m_pTADS || NULL == pclBatchSelectionParameters->m_pTADS->GetpTechParams()
		|| NULL == pclBatchSelectionParameters->m_pTADB )
	{
		ASSERTA_RETURN( false );
	}

	// Verify pipe size shift.
	// Setup variables to possibly enforce combined SizeShifts.
	// In case of Hub Selection don't use SizeShift from TechParam.
	int iSizeShiftUp;
	int iSizeShiftDown;
	int iSizeShiftAbove = pclBatchSelectionParameters->m_pTADS->GetpTechParams()->GetSizeShiftAbove();
	int iSizeShiftBelow = pclBatchSelectionParameters->m_pTADS->GetpTechParams()->GetSizeShiftBelow();
	
	if( 99 == iSizeShiftAbove )  // Combined SizeShift +3/-3 enforced
	{
		iSizeShiftUp = 3;
		iSizeShiftDown = -3;
	}
	else
	{
		iSizeShiftUp = iSizeShiftAbove;
		iSizeShiftDown = iSizeShiftBelow;
	}
	
	// In case of pipe with a size below DN15 force a selection up to DN15.
	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclBatchSelectionParameters->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( false );
	}

	int iDN15 = pclTableDN->GetSize( _T("DN_15") );

	int iPipeDnKey = m_pclBestPipe->GetSizeKey( pclBatchSelectionParameters->m_pTADB );
	int iDN = pclTableDN->GetBiggestDN();
	iHighestSize = min( iPipeDnKey + iSizeShiftUp, iDN );

	if( iHighestSize < iDN15 )
	{
		iHighestSize = iDN15;
	}
	
	iLowestSize = max( 1, iPipeDnKey + iSizeShiftDown );

	return true;
}

//////////////////////////////////////////////////////////////////////////////////
// Selected control valve container base for batch mode for balancing and control
// valves and pressure independent balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////
CDB_Actuator *CBatchSelectCtrlListBase::SelectActuator( CBatchSelCtrlParamsBase *pclBatchSelCtrlParamsBase, CDB_ControlValve *pclControlValve, double dDp, int*  p_iReturn )
{
	if( NULL == pclBatchSelCtrlParamsBase || NULL == pclBatchSelCtrlParamsBase->m_pTADB )
	{
		ASSERTA_RETURN( NULL );
	}

	if( NULL == pclControlValve )
	{
		return NULL;
	}
	
	// Two loops: the first loop tries to find actuator belonging to a set. If not found, try all actuators.
	// Do not need to loop if control valve not belong to a set.
	int iLoopEnd = ( true == pclControlValve->IsPartOfaSet() ) ? 2 : 1;
	CDB_ElectroActuator *pclActuator = NULL;

	enum
	{
		LoopFirst = 0,
		LoopSet = LoopFirst,
		LoopAll,
		LoopLast
	};
	bool bStrict = true;
	// HYS-978: for the moment we don't prioritize sets.
	//for( int iLoop = LoopFirst; iLoop < LoopLast && NULL == pclActuator; iLoop++ )
	for( int iLoop = LoopAll; iLoop < LoopLast && NULL == pclActuator; iLoop++ )
	{
		if( LoopSet == iLoop && false == pclControlValve->IsPartOfaSet() )
		{
			// If we loop on set but the valve is not belonging to a set we bypass this loop.
			continue;
		}

		bool bWorkingForASet = ( LoopSet == iLoop ) ? true : false;

		// Actuator selection will be based:
		//   COC: base list.
		//   Second: Tech data.
		std::multimap<double, CDB_Actuator *> ActrFullMap;
		int iCount = _GetCOCCompliantActuatorList( pclBatchSelCtrlParamsBase, pclControlValve, dDp, &ActrFullMap, bWorkingForASet );
	
		if( 0 == iCount )
		{
			continue;
		}

		std::multimap<double, CDB_Actuator *> ActrPackMap;
		std::multimap<double, CDB_Actuator *>* pActrMap = NULL;
		
		if( true == bWorkingForASet )
		{
			if( 0 == pclControlValve->ExtractPackageCompliantActr( &ActrFullMap, &ActrPackMap ) )
			{
				continue;
			}

			pActrMap = &ActrPackMap;
		}
		else
		{
			pActrMap = &ActrFullMap;
		}

		// Transfer 'pActrMap'.
		CRank rkActrPreList( false );
		std::multimap< double, CDB_Actuator *>::iterator It;
		
		for( It = pActrMap->begin(); It != pActrMap->end(); ++It )
		{
			rkActrPreList.Add( ( It->second )->GetName(), It->first, ( LPARAM ) It->second );
		}

		// Extract actuator from the 'pclRankPreList' that match wanted 'Power supply', 'Input signal' and 'Fail safe' and save them in 'rkActrList'.
		CRank rkActrList( false );
		CRank *pclRankPreList = &rkActrPreList;

		// Do a loop to relax 'DRP', 'Fail safe', 'Input signal' and 'Power signal'.
		CDB_ElectroActuator::DefaultReturnPosition eActuatorDRP;
		bStrict = true;
		int iLoopRelax = 0;
		
		for( ; iLoopRelax < 5 && NULL == pclActuator; iLoopRelax++ )
		{
			switch( iLoopRelax )
			{
				// Strict.
				case 0:
					// In regards to the current user choice in the default return position combo box (None, closing, opening or configurable), the 'GetCompatibleActuatorDRP' method will return what
					// is the actuator Default return position function compatible (None, extending, retracting or configurable).
					if( CDB_ControlValve::drpfNone == pclBatchSelCtrlParamsBase->m_eActuatorDRPFunction )
					{
						eActuatorDRP = CDB_ElectroActuator::None;
					}
					else
					{
						eActuatorDRP = (CDB_ElectroActuator::DefaultReturnPosition)pclControlValve->GetCompatibleActuatorDRP( pclBatchSelCtrlParamsBase->m_eActuatorDRPFunction );
					}

					iCount = pclBatchSelCtrlParamsBase->m_pTADB->GetActuator( &rkActrList, (CRank*)pclRankPreList, (LPCTSTR)pclBatchSelCtrlParamsBase->m_strActuatorPowerSupplyID, 
							(LPCTSTR)pclBatchSelCtrlParamsBase->m_strActuatorInputSignalID, pclBatchSelCtrlParamsBase->m_iActuatorFailSafeFunction, eActuatorDRP, pclBatchSelCtrlParamsBase->m_eFilterSelection );

					break;

				// Relax DRP.
				// In regards to the current user choice in the DRP combo box (None, closing, opening or configurable), the 'GetCompatibleActuatorDRP' method will return what
				// is the actuator fail safe function compatible (None, extending, retracting or configurable).
				case 1:
					iCount = pclBatchSelCtrlParamsBase->m_pTADB->GetActuator( &rkActrList, (CRank*)pclRankPreList, (LPCTSTR)pclBatchSelCtrlParamsBase->m_strActuatorPowerSupplyID, 
							(LPCTSTR)pclBatchSelCtrlParamsBase->m_strActuatorInputSignalID, pclBatchSelCtrlParamsBase->m_iActuatorFailSafeFunction, CDB_ElectroActuator::DefaultReturnPosition::Undefined, pclBatchSelCtrlParamsBase->m_eFilterSelection );

					break;

				case 2:
					// Relax fail safe

					iCount = pclBatchSelCtrlParamsBase->m_pTADB->GetActuator( &rkActrList, (CRank*)pclRankPreList, (LPCTSTR)pclBatchSelCtrlParamsBase->m_strActuatorPowerSupplyID, 
							(LPCTSTR)pclBatchSelCtrlParamsBase->m_strActuatorInputSignalID, -1, CDB_ElectroActuator::DefaultReturnPosition::Undefined, pclBatchSelCtrlParamsBase->m_eFilterSelection );

					break;
				// Relax input signal.
				case 3:
					iCount = pclBatchSelCtrlParamsBase->m_pTADB->GetActuator( &rkActrList, (CRank*)pclRankPreList, (LPCTSTR)pclBatchSelCtrlParamsBase->m_strActuatorPowerSupplyID, 
							L"", -1, CDB_ElectroActuator::DefaultReturnPosition::Undefined, pclBatchSelCtrlParamsBase->m_eFilterSelection );
					
					break;

				// Relax power supply.
				case 4:
					iCount = pclBatchSelCtrlParamsBase->m_pTADB->GetActuator( &rkActrList, (CRank*)pclRankPreList, L"", L"", -1, CDB_ElectroActuator::DefaultReturnPosition::Undefined, 
							pclBatchSelCtrlParamsBase->m_eFilterSelection );

					break;
			}
		
			if( 0 == iCount )
			{
				continue;
			}

			CString str;
			LPARAM param;
			CDB_ElectroActuator *pclActuatorSaved = NULL;

			// Identify the smallest ones.
			double dBestKey = DBL_MAX;
			vector<CDB_ElectroActuator*> vectConfigurable;
			for( BOOL bContinue = rkActrList.GetFirst( str, param ); TRUE == bContinue; bContinue = rkActrList.GetNext( str, param ) )
			{
				pclActuator = (CDB_ElectroActuator *)param;
				
				// That allow us to retrieve the actuator with the lowest max force/torque AND take into account priority level
				// to take actuator with the lowest priority number (1 = higher priority; 10 = lower priority).
				double dKey = 1e9 * pclActuator->GetPriorityLevel();
				dKey += pclActuator->GetMaxForceTorque();
				
				if( dKey < dBestKey )
				{
					if( iLoopRelax <= 1 && eActuatorDRP != pclActuator->GetDefaultReturnPos() && CDB_ElectroActuator::DefaultReturnPosition::Configurable == pclActuator->GetDefaultReturnPos() )
					{
						vectConfigurable.push_back( pclActuator );
						continue;
					}
					dBestKey = dKey;
					pclActuatorSaved = pclActuator;
					pclBatchSelCtrlParamsBase->m_bActuatorSelectedAsSet = bWorkingForASet;
				}
			}

			if( NULL == pclActuatorSaved )
			{
				for( vector<CDB_ElectroActuator*>::iterator it =  vectConfigurable.begin(); it != vectConfigurable.end(); ++it )
				{
					pclActuator = *it;

					// That allow us to retrieve the actuator with the lowest max force/torque AND take into account priority level
					// to take actuator with the lowest priority number (1 = higher priority; 10 = lower priority).
					double dKey = 1e9 * pclActuator->GetPriorityLevel();
					dKey += pclActuator->GetMaxForceTorque();

					if( dKey < dBestKey )
					{
						dBestKey = dKey;
						pclActuatorSaved = pclActuator;
						pclBatchSelCtrlParamsBase->m_bActuatorSelectedAsSet = bWorkingForASet;
					}
				}
			}
			
			pclActuator = pclActuatorSaved;
			vectConfigurable.clear();
			
			if( 0 != iLoopRelax )
			{
				bStrict = false;
			}
		}
	}
	// Return flag
	if( NULL == pclActuator )
	{
		*p_iReturn = CBatchSelectBaseList::BatchReturn::BR_NotFound;
	}
	else if ( false == bStrict )
	{
		*p_iReturn = CBatchSelectBaseList::BatchReturn::BR_FoundAlernative;
	}
	else
	{
		*p_iReturn = CBatchSelectBaseList::BatchReturn::BR_FoundOneBest;
	}

	return pclActuator;
}

int CBatchSelectCtrlListBase::_GetCOCCompliantActuatorList( CBatchSelCtrlParamsBase *pclBatchSelCtrlParamsBase, CDB_ControlValve *pclControlValve, 
		double dDp, std::multimap< double, CDB_Actuator *> *pmmapActuatorList, bool bWorkingForASet )
{
	if( NULL == pclControlValve || NULL == pmmapActuatorList )
	{
		return 0;
	}
	
	if( NULL != pmmapActuatorList )
	{
		pmmapActuatorList->clear();
	}

	// Try to find some compliant actuators.
	CDB_CloseOffChar *pclCloseOffChar = (CDB_CloseOffChar*)( pclControlValve->GetCloseOffCharIDPtr().MP );
	double dMinCloseOffDp = dDp;

	std::multimap<double, CDB_Actuator*> mapActr;
	CRank rkList( false );

	if( pclBatchSelCtrlParamsBase->m_pTADB->GetActuator( &rkList, pclControlValve, bWorkingForASet, pclBatchSelCtrlParamsBase->m_eFilterSelection ) )
	{
		CString str;
		LPARAM lpItemData = 0;

		for( BOOL bContinue = rkList.GetFirst( str, lpItemData ); TRUE == bContinue; bContinue = rkList.GetNext( str, lpItemData ) )
		{
			CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( ( CData * ) lpItemData );

			if( NULL != pclActuator )
			{
				double dCloseOffDp = -1.0;

				if( NULL != pclCloseOffChar && CDB_CloseOffChar::CloseOffDp == pclCloseOffChar->GetLimitType() )
				{
					dCloseOffDp = pclCloseOffChar->GetCloseOffDp( pclActuator->GetMaxForceTorque() );
				}

				if( dCloseOffDp >= dMinCloseOffDp )
				{
					mapActr.insert( std::pair<double, CDB_Actuator *> ( dCloseOffDp, pclActuator ) );
				}
			}
		}
		
		// Not found keep strongest.
		if( 0 == mapActr.size() )
		{
			double dMaxCloseOffDp = -1.0;

			for( BOOL bContinue = rkList.GetFirst( str, lpItemData ); TRUE == bContinue; bContinue = rkList.GetNext( str, lpItemData ) )
			{
				CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( ( CData * ) lpItemData );
			
				if( NULL != pclActuator )
				{
					double dCloseOffDp = -1.0;

					if( NULL != pclCloseOffChar && CDB_CloseOffChar::CloseOffDp == pclCloseOffChar->GetLimitType() )
					{
						dCloseOffDp = pclCloseOffChar->GetCloseOffDp( pclActuator->GetMaxForceTorque() );
					}

					if( dCloseOffDp == dMaxCloseOffDp )
					{
						mapActr.insert( std::pair<double, CDB_Actuator *> ( dCloseOffDp, pclActuator ) );
					}

					if( dCloseOffDp > dMaxCloseOffDp )
					{
						mapActr.clear();
						dMaxCloseOffDp = dCloseOffDp;
						mapActr.insert( std::pair<double, CDB_Actuator *> ( dCloseOffDp, pclActuator ) );
					}
				}
			}
		}
	}

	rkList.PurgeAll();

	if( NULL != pmmapActuatorList )
	{
		*pmmapActuatorList = mapActr;
	}

	return mapActr.size();
}

//////////////////////////////////////////////////////////////////////////////////
//	Selected balancing & control valve container for batch mode
//////////////////////////////////////////////////////////////////////////////////
CBatchSelectBCVList::CBatchSelectBCVList()
	: CBatchSelectCtrlListBase()
{
}

void CBatchSelectBCVList::TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList )
{
	if( NULL == pclIndividualSelectList )
	{
		ASSERT_RETURN;
	}

	CBatchSelectBaseList::TransferResultsToIndividualSelectList( pclIndividualSelectList );

	// For BCV we use the 'm_MMapKeyDecreasing' variable.
	for( auto &iter : m_MMapKeyDecreasing )
	{
		if( NULL == iter.second )
		{
			continue;
		}

		if( CSelectedBase::SBT_Valve == iter.second->GetSelectedBaseType() )
		{
			CSelectedValve *pclSelectedValve = new CSelectedValve();

			if( NULL == pclSelectedValve )
			{
				continue;
			}

			pclSelectedValve->CopyFrom( iter.second );
			pclIndividualSelectList->m_MMapKeyDecreasing.insert( ProductPair( iter.first, pclSelectedValve ) );
		}
	}
}

int CBatchSelectBCVList::SelectBCV( CBatchSelBCVParams *pclBatchSelBCVParams, double dFlow, double dDp )
{
	if( NULL == pclBatchSelBCVParams || NULL == pclBatchSelBCVParams->m_pTADS || NULL == pclBatchSelBCVParams->m_pTADS->GetpTechParams()
		|| NULL == pclBatchSelBCVParams->m_pTADB )
	{
		ASSERTA_RETURN( BatchReturn::BR_BatchSelParamsNULL );
	}

	if( dFlow <= 0.0 )
	{
		return BatchReturn::BR_BadArguments;
	}

	m_pclBestProduct = NULL;

	// Only best pipe requested.
	m_clSelectPipeList.SelectPipes( pclBatchSelBCVParams, dFlow );
	m_pclBestPipe = m_clSelectPipeList.GetBestPipe();

	if( NULL == m_pclBestPipe && 1 == m_clSelectPipeList.GetPipeNumbers() )
	{
		m_pclBestPipe = m_clSelectPipeList.GetpPipeMap()->begin()->second.GetpPipe();
	}

	if( NULL == m_pclBestPipe )
	{
		return BatchReturn::BR_NoPipeFound;
	}

	CDS_TechnicalParameter *pTechParams = pclBatchSelBCVParams->m_pTADS->GetpTechParams();

	// Check Dp value.
	if( dDp <= 0.0 )
	{
		dDp = ( CDB_ControlProperties::eCvOnOff == pclBatchSelBCVParams->m_eCvCtrlType ) ? pTechParams->GetCVMinDpOnoff() : pTechParams->GetCVMinDpProp();
	}

	bool bAtLeastOneBestStrict = false;
	bool bAtLeastOneBestAlter = false;
	CSelectedBase *pclBestProductStrict = NULL;
	CSelectedBase *pclBestProductAlter = NULL;

	double dKvs = CalcKv( dFlow, dDp, pclBatchSelBCVParams->m_WC.GetDens() );

	if( true == _SelectBestTACV( pclBatchSelBCVParams, dKvs, dFlow, dDp ) )
	{
		// To facilitate use in the 'CDlgBatchSelBCv' class.
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedBase *pclSelectedProduct = m_cIter->second;
			ASSERT( NULL != pclSelectedProduct );

			if( NULL == pclSelectedProduct )
			{
				continue;
			}

			CSelectedValve *pclSelectedBCV = dynamic_cast<CSelectedValve *>( pclSelectedProduct );
			ASSERT( NULL != pclSelectedBCV );

			if( NULL == pclSelectedBCV )
			{
				continue;
			}

			if( true == pclSelectedBCV->GetFlag( CSelectedBase::eBest ) )
			{
				if( false == pclSelectedBCV->GetFlag( CSelectedBase::eValveBatchAlter ) )
				{
					bAtLeastOneBestStrict = true;
					pclBestProductStrict = pclSelectedBCV;
				}
				else
				{
					bAtLeastOneBestAlter = true;
					pclBestProductAlter = pclSelectedBCV;
				}
			}

			CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct*>( pclSelectedBCV->GetpData() );
			ASSERT( NULL != pTAP );

			if( NULL == pTAP )
			{
				continue;
			}
		
			// Transfer valve into the final list with good priority key.
			CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pTAP, pclSelectedBCV );
		}
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	// Set the return value.
	int iReturn;

	if( true == bAtLeastOneBestStrict && NULL != pclBestProductStrict )
	{
		m_pclBestProduct = pclBestProductStrict;
		iReturn = BatchReturn::BR_FoundOneBest;
	}
	else if( true == bAtLeastOneBestAlter && NULL != pclBestProductAlter )
	{
		m_pclBestProduct = pclBestProductAlter;
		iReturn = BatchReturn::BR_FoundOneBest | BatchReturn::BR_FoundAlernative;
	}
	else
	{
		iReturn = ( m_MMapKeyDecreasing.size() > 0 ) ? BatchReturn::BR_FoundButNotBest : BatchReturn::BR_NotFound;
	}

	return iReturn;
}

bool CBatchSelectBCVList::_SelectBestTACV( CBatchSelBCVParams *pclBatchSelBCVParams, double dTargetKvs, double dFlow, double dDpRequired )
{
	if( NULL == m_pclBestPipe )
	{
		return false;
	}

	int iHighestSize = 0;
	int iLowestSize = 0;

	if( false == GetPipeSizeShift( (CBatchSelectionParameters*)pclBatchSelBCVParams, iHighestSize, iLowestSize ) )
	{
		return false;
	}

	CTableDN *pclTableDN = (CTableDN *)( pclBatchSelBCVParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( false );
	}

	int iDN15 = pclTableDN->GetSize( _T("DN_15") );
	int iDN50 = pclTableDN->GetSize( _T("DN_50") );

	CDS_TechnicalParameter *pclTechParams = pclBatchSelBCVParams->m_pTADS->GetpTechParams();

	if( dDpRequired <= 0.0 )
	{
		dDpRequired = ( CDB_ControlProperties::eCvOnOff == pclBatchSelBCVParams->m_eCvCtrlType ) ? pclTechParams->GetCVMinDpOnoff() : pclTechParams->GetCVMinDpProp();
	}
	
	// Use the product type in regards to the selected pipe.
	bool bBelow65 = ( m_pclBestPipe->GetSizeKey( pclBatchSelBCVParams->m_pTADB ) <= iDN50 ) ? true : false;
	CString strType = ( true == bBelow65 ) ? pclBatchSelBCVParams->m_strComboTypeBelow65ID : pclBatchSelBCVParams->m_strComboTypeAbove50ID;

	CRankEx FullList;

	int iNum = pclBatchSelBCVParams->m_pTADB->GetTaCVList( &FullList, CTADatabase::eForBCv, false, pclBatchSelBCVParams->m_eCV2W3W, (LPCTSTR)strType, 
			L"", L"", L"", L"", L"", CDB_ControlProperties::LastCVFUNC, pclBatchSelBCVParams->m_eCvCtrlType, pclBatchSelBCVParams->m_eFilterSelection );

	if( 0 == iNum )
	{
		return false;
	}

	LPARAM lpParam;
	_string str;
	CRankEx RestrictiveList;

	// If user ask to be strict.
	if( true == pclBatchSelBCVParams->m_bIsCtrlTypeStrictChecked )
	{
		bool bAtLeastOneExist = false;

		for( BOOL bContinue = FullList.GetFirst( str, lpParam ); TRUE == bContinue && false == bAtLeastOneExist; bContinue = FullList.GetNext( str, lpParam ) )
		{
			CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( ( CData * ) lpParam );

			if( NULL == pCV )
			{
				continue;
			}

			if( CDB_ControlProperties::CvCtrlType::eCvOnOff == pclBatchSelBCVParams->m_eCvCtrlType 
				&& CDB_ControlProperties::eCTRLCHAR::NotCharacterized == pCV->GetCtrlProp()->GetCvCtrlChar() )
			{
				bAtLeastOneExist = true;
			}
			else if( ( CDB_ControlProperties::CvCtrlType::eCv3point == pclBatchSelBCVParams->m_eCvCtrlType 
					   || CDB_ControlProperties::CvCtrlType::eCvProportional == pclBatchSelBCVParams->m_eCvCtrlType ) 
					 && CDB_ControlProperties::eCTRLCHAR::NotCharacterized != pCV->GetCtrlProp()->GetCvCtrlChar() )
			{
				bAtLeastOneExist = true;
			}
		}

		if( true == bAtLeastOneExist )
		{
			for( BOOL bContinue = FullList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = FullList.GetNext( str, lpParam ) )
			{
				CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( ( CData * ) lpParam );

				if( NULL == pCV )
				{
					continue;
				}

				double dKey = pCV->GetSortingKey();
				bool bCanAdd = false;

				if( CDB_ControlProperties::CvCtrlType::eCvOnOff == pclBatchSelBCVParams->m_eCvCtrlType
						&& CDB_ControlProperties::eCTRLCHAR::NotCharacterized == pCV->GetCtrlProp()->GetCvCtrlChar() )
				{
					bCanAdd = true;
				}
				else if( ( CDB_ControlProperties::CvCtrlType::eCv3point == pclBatchSelBCVParams->m_eCvCtrlType 
						   || CDB_ControlProperties::CvCtrlType::eCvProportional == pclBatchSelBCVParams->m_eCvCtrlType ) 
						 && CDB_ControlProperties::eCTRLCHAR::NotCharacterized != pCV->GetCtrlProp()->GetCvCtrlChar() )
				{
					bCanAdd = true;
				}

				if( true == bCanAdd && false == RestrictiveList.GetaCopy( str, dKey, lpParam ) )
				{
					RestrictiveList.Add( pCV->GetName(), dKey, ( LPARAM )pCV );
				}
			}
		}
	}

	enum
	{
		LoopFirst = 0,
		LoopRestrictive = LoopFirst,
		LoopFullList,
		LoopDeleted,
		LoopLast
	};

	for( int iLoop = LoopFirst; iLoop < LoopLast; iLoop++ )
	{
		if( LoopRestrictive == iLoop && 0 == RestrictiveList.GetCount() )
		{
			// If there is no valve that strictly the same as the input user, we bypass this loop.
			continue;
		}

		if( LoopFullList == iLoop && true == pclBatchSelBCVParams->m_bIsCtrlTypeStrictChecked )
		{
			// If user asks to be precise with input, we bypass the full valve list.
		}

		if( LoopDeleted == iLoop && NULL != TASApp.GetpTADS()->GetpPersistData() && false == TASApp.GetpTADS()->GetpPersistData()->IsOldValveAreUsed() )
		{
			// If user doesn't want to show old valves, we bypass this step.
			continue;
		}

		CRankEx *pCurrentList = ( LoopRestrictive == iLoop ) ? &RestrictiveList : &FullList;

		// Extract valves according size-shift, keep at least one product size, valves are ordered by size.
		std::multimap<int, CDB_ControlValve *> mapCV;

		for( BOOL bContinue = pCurrentList->GetFirst( str, lpParam ); TRUE == bContinue; bContinue = pCurrentList->GetNext( str, lpParam ) )
		{
			CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( ( CData * ) lpParam );

			if( NULL == pCV )
			{
				continue;
			}

			if( LoopDeleted != iLoop && true == pCV->IsDeleted() )
			{
				// For restrictive and full list loop we don't take the deleted valves.
				continue;
			}

			int iProdSizeKey = pCV->GetSizeKey();
			mapCV.insert( std::make_pair( iProdSizeKey, pCV ) );
		}

		if( 0 == mapCV.size() )
		{
			continue;
		}

		std::multimap<int, CDB_ControlValve *>::iterator itMapCV;
		std::multimap<int, CDB_ControlValve *>::reverse_iterator ritMapCV;
		
		std::multimap<int, CDB_ControlValve *>::iterator itLow = mapCV.find( iLowestSize );
		std::multimap<int, CDB_ControlValve *>::iterator itUp = mapCV.find( iHighestSize );

		if( mapCV.end() == itLow && mapCV.end() == itUp )
		{
			// Valve with correct size not found, keep smallest or highest valve.
			itMapCV = mapCV.begin();

			if( itMapCV->first > iHighestSize )
			{
				// All valves size are above the highest size, keep smallest valve size.
				itLow = mapCV.upper_bound( itMapCV->first );
				mapCV.erase( itLow, mapCV.end() );
			}

			ritMapCV = mapCV.rbegin();

			if( ritMapCV->first < iLowestSize )
			{
				// All valves size are below the lowest size, keep highest valve size.
				itUp = mapCV.lower_bound( ritMapCV->first );
				mapCV.erase( mapCV.begin(), mapCV.end() );
			}
		}
		else
		{
			if( mapCV.end() != itLow && mapCV.begin() != itLow )
			{
				mapCV.erase( mapCV.begin(), itLow );
			}

			itUp = mapCV.upper_bound( iHighestSize );

			if( mapCV.end() != itUp )
			{
				mapCV.erase( itUp, mapCV.end() );
			}
		}

		if( 0 == mapCV.size() )
		{
			continue;
		}

		mmapKeyDoubleSelectedProduct mapBestDeltaKvs;
		mmapKeyDoubleSelectedProduct mapPoorDeltaKvs;

		double dMaxCvDpDefinedInTechParams = pclTechParams->GetCVMaxDpProp();

		double dValveMinDp = ( CDB_ControlProperties::CvCtrlType::eCvOnOff == pclBatchSelBCVParams->m_eCvCtrlType ) ? pclTechParams->GetCVMinDpOnoff() 
				: pclTechParams->GetCVMinDpProp();

		// Best valve is the smallest valve with the lowest Dpmin.
		// When we are working with valve with independent characteristics or lift limitation, we will take a bigger valve to have the possibility to close it.
		// Typically it's the TA-Fusion. TBV-CM have a balancing part and control part in series. Thus a change in the setting has an impact on the authority.
		
		// To only clean 'm_vecObjects' if it already contains objects.
		CleanVecObjects();

		for( itMapCV = mapCV.begin(); itMapCV != mapCV.end(); ++itMapCV )
		{
			CDB_ControlValve *pclBalancingControlValve = itMapCV->second;
			
			if( NULL == pclBalancingControlValve )
			{
				continue;
			}

			// To facilitate use in the 'CDlgBatchSelBCv' class.
			CSelectedValve *pSelBCV = new CSelectedValve();
			
			if( NULL == pSelBCV )
			{
				continue;
			}

			pSelBCV->SetProductIDPtr( pclBalancingControlValve->GetIDPtr() );
			pSelBCV->SetFlag( CSelectedBase::eValveBatchAlter, false );
			pSelBCV->SetFlag( CSelectedBase::Flags::eNotPriority, false );

			double dKvs = pclBalancingControlValve->GetKvs();
			double dDpFullyOpened = CalcDp( dFlow, dKvs, pclBatchSelBCVParams->m_WC.GetDens() );

			// No security factor for TBV-CM.
			double dTAFUSIONSecurityFactor = 1;
			bool bTAFusion = false;

			if(	CDB_ControlProperties::eBCVType::ebcvtCharIndep == pclBalancingControlValve->GetCtrlProp()->GetBCVType() 
					|| CDB_ControlProperties::eBCVType::ebcvtLiftLimit == pclBalancingControlValve->GetCtrlProp()->GetBCVType() ) 
			{
				// TA-FUSION
				// When valve at full opening is below ..10kPa don't target a smaller valve.
				dTAFUSIONSecurityFactor = ( dDpFullyOpened <= dValveMinDp ) ? 1 : 0.8;
				bTAFusion = true;
			}
			
			double dDeltaKvs = 0.0;

			// If 'dTAFUSIONSecurityFactor' is set to 0.8, we decrease the 'kvs'. It means at full opening, the flow will be lower than the valve
			// is smaller.
			double dKvsCorrected = dKvs * dTAFUSIONSecurityFactor;
			dDeltaKvs = abs( dTargetKvs - dKvsCorrected );
			
			if( dDpFullyOpened > dMaxCvDpDefinedInTechParams )
			{
				// Mark delta value with a constant value that report this selection at the end of the list.
				dDeltaKvs += 10000.0;
			}

			CDB_ValveCharacteristic *pclValveCharacteristic = pclBalancingControlValve->GetValveCharacteristic();
			double dH = -1.0;
			mmapKeyDoubleSelectedProduct::iterator mapIterWhereInserted;
			bool bInsertedInBest = true;

			if( dKvsCorrected >= dTargetKvs )
			{
				// We target a valve with a Kvs above the TargetKvs.
				// We can close the balancing & control valve to reach target value.
					
				// Normally, for TBV-CM valve we prefer to get a Kvs bigger than the targeted one.
				// Because in this case the change of the presetting has almost no impact on the authority.
				// But here in batch selection we don't take into account the authority and we try to have
				// the same behavior as in individual selection.

				// Verify if settings is above minimum recommended setting.
				pclValveCharacteristic->GetOpening( dTargetKvs, &dH );
					
				if( dH < pclValveCharacteristic->GetMinRecSetting() || dDpFullyOpened > dMaxCvDpDefinedInTechParams )
				{
					mapIterWhereInserted = mapPoorDeltaKvs.insert( std::make_pair( dDeltaKvs, pSelBCV ) );
					bInsertedInBest = false;
				}
				else
				{
					mapIterWhereInserted = mapBestDeltaKvs.insert( std::make_pair( dDeltaKvs, pSelBCV ) );
				}
			}
			else 
			{
				mapIterWhereInserted = mapPoorDeltaKvs.insert( std::make_pair( dDeltaKvs, pSelBCV ) );
				bInsertedInBest = false;
			}

			// HYS-825: Display DpFullyOpen and DpHalfOpen when editing batch result.
			double dHMax = pclValveCharacteristic->GetOpeningMax();

			if( dDpFullyOpened > 0.0 )
			{
				pSelBCV->SetDpFullOpen( dDpFullyOpened );
			}

			double dHHalf = 0.5 * dHMax;
			double dDpHalfOpen = -1.0;

			if( true == pclValveCharacteristic->GetValveDp( dFlow, &dDpHalfOpen, dHHalf, pclBatchSelBCVParams->m_WC.GetDens(),
					pclBatchSelBCVParams->m_WC.GetKinVisc() ) )
			{
				pSelBCV->SetDpHalfOpen( dDpHalfOpen );
			}

			// Selection process from flow and Dp if user has introduced a Dp value (fDpOK = true)
			// OR from flow only if no Dp.
			// Remark: If at 100% opening ( dDpFullyOpen ) we already have
			//         a Dp greater than Dp min, it is impossible to open more to reach good Dp min.
			double dDp = -1.0;

			if( ( dDpRequired > 0.0 && dDpRequired >= dDpFullyOpened )
					|| ( 0.0 == dDpRequired && dDpFullyOpened < dValveMinDp ) )
			{
				// If user wants a specific Dp (fDpOp = true) we must take his value, otherwise we take the min Dp for the valve.
				dDp = ( dDpRequired > 0.0 ) ? dDpRequired : dValveMinDp;

				// Try to find a solution.
				bool bValid = pclValveCharacteristic->GetValveOpening( dFlow, dDp, &dH, pclBatchSelBCVParams->m_WC.GetDens(),
						pclBatchSelBCVParams->m_WC.GetKinVisc(), ( eBool3::eb3True == pclValveCharacteristic->IsDiscrete() ) ? 3 : 1 );

				// The min Kv value was not small enough !
				if( false == bValid )
				{
					dH = pclValveCharacteristic->GetMinRecSetting();

					if( dH <= 0.0 )
					{
						dH = pclValveCharacteristic->GetOpeningMin();
					}

					if( -1.0 != dH )
					{
						if( false == pclValveCharacteristic->GetValveDp( dFlow, &dDp, dH, pclBatchSelBCVParams->m_WC.GetDens(),
								pclBatchSelBCVParams->m_WC.GetKinVisc() ) )
						{
							if( true == bInsertedInBest )
							{
								mapBestDeltaKvs.erase( mapIterWhereInserted );
							}
							else
							{
								mapPoorDeltaKvs.erase( mapIterWhereInserted );
							}

							delete pSelBCV;
							continue;
						}
					}
				}
			}

			// If dh < 0, no solution was found.
			bool bIsFound = ( dH < 0.0 ) ? false : true;

			if( false == bIsFound )
			{
				dH = dHMax;
				dDp = dDpFullyOpened;
				pSelBCV->SetFlag( CSelectedBase::eValveMaxSetting, true );
			}
			else if( eBool3::eb3True == pclValveCharacteristic->IsDiscrete() )
			{
				// Recalculate the Dp for the setting as determined above
				if( false == pclValveCharacteristic->GetValveDp( dFlow, &dDp, dH, pclBatchSelBCVParams->m_WC.GetDens(), pclBatchSelBCVParams->m_WC.GetKinVisc() ) )
				{
					if( true == bInsertedInBest )
					{
						mapBestDeltaKvs.erase( mapIterWhereInserted );
					}
					else
					{
						mapPoorDeltaKvs.erase( mapIterWhereInserted );
					}

					delete pSelBCV;
					continue;
				}
			}

			pSelBCV->SetH( dH );
			pSelBCV->SetDp( dDp );

			// To keep a trace of created object to well erase them when no more needed.
			m_vecObjects.push_back( pSelBCV );
		}

		mmapKeyDoubleSelectedProduct::iterator It = mapBestDeltaKvs.end();
		mmapKeyDoubleSelectedProduct::iterator ItBest = mapBestDeltaKvs.end();
	
		// Build sorting keys.
		CTAPSortKey sKeyTechParamBlw65;
		CTAPSortKey sKeyTechParamAbv50;
		BuildSortingKeys( pclBatchSelBCVParams, &sKeyTechParamBlw65 , &sKeyTechParamAbv50 );

		double dKeyCurrentTAP = 0.0;
		int iStartLoop = 0;
		int iEndLoop = 6;
		bool bFound = false;

		m_PreselMMapKeyDecreasing.clear();

		// For each map, we will determine the best and if the result is strictly matched the user choice or not.
		// HYS-882: to allow valves with size above/below, we accept to run the 2 loops.
		for( int iLoop = 0; iLoop < 2; iLoop++ )
		{
			// This loop give us the possibility to analyze two maps.
			// First Opening map, valves are big enough to satisfy requested Dp.
			// Second Dp map, valves are too small.
			mmapKeyDoubleSelectedProduct *pMap = NULL;

			switch( iLoop )
			{
				case 0:
					// Process mapDeltaOpeningTap.
					// analyze mapDeltaOPeningTAP, that contains all valves that can satisfy requested Dp.
					pMap = 	&mapBestDeltaKvs;
					break;

				case 1:
					// Process mapDeltaDpFOTAP.
					// Valve are too small to satisfy requested Dp.
					pMap = 	&mapPoorDeltaKvs;
					break;
			}

			// Try to find solution.
			if( pMap->size() > 0 )
			{
				int iUserPrefLoop = iStartLoop;

				for( ; iUserPrefLoop < iEndLoop; iUserPrefLoop++ )
				{
					// This loop give us the possibility to relax mask criteria one by one.
					// We will start with the most constraining key and if we don't found a product we will relax constrains one by one.
					CTAPSortKey::TAPSortKeyMask eMask = ( CTAPSortKey::TAPSortKeyMask )( CTAPSortKey::TAPSKM_All & ~CTAPSortKey::TAPSKM_PriorL 
							& ~CTAPSortKey::TAPSKM_Size );

					switch( iUserPrefLoop )
					{
						case 0:
							// No filter, we are strict.
							break;

						case 1:
							// Relax PN.
							eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
							break;

						case 2:
							// Relax PN and version.
							eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version );
							break;

						case 3:
							// Relax PN, version and connection.
							eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version 
									& ~CTAPSortKey::TAPSKM_Connect );
							break;

						case 4:
							// Relax PN, version, connection and body material.
							eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version 
									& ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Bdy );
							break;

						case 5:
							// Relax PN, version, connection, body material and family.
							eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version 
									& ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Bdy & ~CTAPSortKey::TAPSKM_Fam );
							break;
					}

					double dKeyTechParamBlw65 = sKeyTechParamBlw65.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
					double dKeyTechParamAbv50 = sKeyTechParamAbv50.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );

					// Previous valve doesn't exist, restart loop only on tech params.
					for( It = pMap->begin(); It != pMap->end(); ++It )
					{
						CDB_TAProduct *pTAP = (CDB_TAProduct*)( It->second->GetProductIDPtr().MP );
						int iSize = pTAP->GetSizeKey();
						double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
						bool bValveMatched = false;

						// Selected valve is not located on the same side of iDN50 border.
						// Use defined technical choice.
						if( iSize <= iDN50 )
						{
							if( dTAPKey == dKeyTechParamBlw65 )
							{
								bValveMatched = true;
							}
						}
						else
						{
							// Size > DN50
							if( dTAPKey == dKeyTechParamAbv50 )
							{
								bValveMatched = true;
							}
						}

						if( true == bValveMatched )
						{
							CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pTAP, It->second );

							if( false == bFound )
							{
								ItBest = It;
								bFound = true;
							}
												
							// Continue loop to save all valves in 'm_PreselMMap' that belongs to the same criterion.
						}
					}

					if( true == bFound )
					{
						break;
					}
				}

				if( true == bFound )
				{
					ItBest->second->SetFlag( CSelectedBase::eBest, true );

					if( 0 != iUserPrefLoop )
					{
						ItBest->second->SetFlag( CSelectedBase::eValveBatchAlter, true );
					}
				}
			}
		}

		if( true == bFound )
		{
			break;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////
//	Selected pressure & independent control valve container for batch mode
//////////////////////////////////////////////////////////////////////////////////
CBatchSelectPICvList::CBatchSelectPICvList()
	: CBatchSelectCtrlListBase()
{
}

void CBatchSelectPICvList::TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList )
{
	if( NULL == pclIndividualSelectList )
	{
		ASSERT_RETURN;
	}

	CBatchSelectBaseList::TransferResultsToIndividualSelectList( pclIndividualSelectList );

	// For PIBCV we use the 'm_MMapKeyDecreasing' variable.
	for( auto &iter : m_MMapKeyDecreasing )
	{
		if( NULL == iter.second )
		{
			continue;
		}

		if( CSelectedBase::SBT_Valve == iter.second->GetSelectedBaseType() )
		{
			CSelectedValve *pclSelectedValve = new CSelectedValve();

			if( NULL == pclSelectedValve )
			{
				continue;
			}

			pclSelectedValve->CopyFrom( iter.second );
			pclIndividualSelectList->m_MMapKeyDecreasing.insert( ProductPair( iter.first, pclSelectedValve ) );
		}
	}
}

int CBatchSelectPICvList::SelectPICv( CBatchSelPIBCVParams *pclBatchSelPIBCVParams, double dFlow )
{
	if( NULL == pclBatchSelPIBCVParams || NULL == pclBatchSelPIBCVParams->m_pTADS || NULL == pclBatchSelPIBCVParams->m_pTADS->GetpTechParams()
			|| NULL == pclBatchSelPIBCVParams->m_pTADB )
	{
		ASSERTA_RETURN( BatchReturn::BR_BatchSelParamsNULL );
	}

	if( dFlow <= 0.0 )
	{
		return BatchReturn::BR_BadArguments;
	}

	// Only best pipe requested.
	m_clSelectPipeList.SelectPipes( pclBatchSelPIBCVParams, dFlow );
	m_pclBestPipe = m_clSelectPipeList.GetBestPipe();

	if( NULL == m_pclBestPipe && 1 == m_clSelectPipeList.GetPipeNumbers() )
	{
		m_pclBestPipe = m_clSelectPipeList.GetpPipeMap()->begin()->second.GetpPipe();
	}

	if( NULL == m_pclBestPipe )
	{
		return BatchReturn::BR_NoPipeFound;
	}

	CDS_TechnicalParameter *pTechParams = pclBatchSelPIBCVParams->m_pTADS->GetpTechParams();

	bool bAtLeastOneBestStrict = false;
	bool bAtLeastOneBestAlter = false;
	CSelectedBase *pclBestProductStrict = NULL;
	CSelectedBase *pclBestProductAlter = NULL;

	if( true == _SelectBestPICV( pclBatchSelPIBCVParams, dFlow ) )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedBase *pclSelectedProduct = m_cIter->second;

			if( NULL == pclSelectedProduct )
			{
				ASSERT( 0 );
				continue;
			}

			CSelectedValve *pclSelectedPICv = dynamic_cast<CSelectedValve *>( pclSelectedProduct );

			if( NULL == pclSelectedPICv )
			{
				ASSERT( 0 );
				continue;
			}

			if( true == pclSelectedPICv->GetFlag( CSelectedBase::eBest ) )
			{
				if( false == pclSelectedPICv->GetFlag( CSelectedBase::eValveBatchAlter ) )
				{
					bAtLeastOneBestStrict = true;
					pclBestProductStrict = pclSelectedPICv;
				}
				else
				{
					bAtLeastOneBestAlter = true;
					pclBestProductAlter = pclSelectedPICv;
				}
			}

			CDB_PIControlValve *pclPIControlValve = dynamic_cast<CDB_PIControlValve*>( pclSelectedPICv->GetpData() );

			if( NULL == pclPIControlValve )
			{
				ASSERT( 0 );
				continue;
			}

			double dRho = pclBatchSelPIBCVParams->m_WC.GetDens();
			double dKinVisc = pclBatchSelPIBCVParams->m_WC.GetKinVisc();
			double dPresetting = pclPIControlValve->GetPresetting( dFlow, dRho, dKinVisc );
			pclSelectedPICv->SetH( dPresetting );

			// Transfer valve into the final list with good priority key.
			CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pclPIControlValve, pclSelectedPICv );
		}
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();
	
	// Set the return value.
	int iReturn;

	if( true == bAtLeastOneBestStrict && NULL != pclBestProductStrict )
	{
		m_pclBestProduct = pclBestProductStrict;
		iReturn = BatchReturn::BR_FoundOneBest;
	}
	else if( true == bAtLeastOneBestAlter && NULL != pclBestProductAlter )
	{
		m_pclBestProduct = pclBestProductAlter;
		iReturn = BatchReturn::BR_FoundOneBest | BatchReturn::BR_FoundAlernative;
	}
	else
	{
		iReturn = ( m_MMapKeyDecreasing.size() > 0 ) ? BatchReturn::BR_FoundButNotBest : BatchReturn::BR_NotFound;
	}

	return iReturn;
}

bool CBatchSelectPICvList::_SelectBestPICV( CBatchSelPIBCVParams *pclBatchSelPIBCVParams, double dFlow )
{
	if( NULL == pclBatchSelPIBCVParams || NULL == pclBatchSelPIBCVParams->m_pTADB || NULL == pclBatchSelPIBCVParams->m_pTADS )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == m_pclBestPipe )
	{
		return false;
	}

	if( false == m_clSelectPipeList.GetPipeSizeBound() )
	{
		return false;
	}

	CDB_Pipe *pSmallestPipe = m_clSelectPipeList.GetpSmallestPipe();

	if( NULL == pSmallestPipe )
	{
		return 0;
	}

	CDB_Pipe *pBiggestPipe = m_clSelectPipeList.GetpLargestPipe();

	if( NULL == pBiggestPipe )
	{
		return 0;
	}

	int iSizeShiftUp;
	int iSizeShiftDown;
	CDS_TechnicalParameter *pclTechParams = pclBatchSelPIBCVParams->m_pTADS->GetpTechParams();
	
	if( 99 == pclTechParams->GetSizeShiftAbove() ) // Combined SizeShift +3/-3 enforced
	{
		iSizeShiftUp = 3;
		iSizeShiftDown = -3;
	}
	else
	{
		iSizeShiftUp = pclTechParams->GetSizeShiftAbove();
		iSizeShiftDown = pclTechParams->GetSizeShiftBelow();
	}
	
	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclBatchSelPIBCVParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN = pclTableDN->GetBiggestDN();
	int iHighestSize = min( iDN, pBiggestPipe->GetSizeKey( pclBatchSelPIBCVParams->m_pTADB ) + iSizeShiftUp );
	int iLowestSize = max( 1, pSmallestPipe->GetSizeKey( pclBatchSelPIBCVParams->m_pTADB ) + iSizeShiftDown );

	int iDN50 = pclTableDN->GetSize( _T("DN_50") );
	int iDN15 = pclTableDN->GetSize( _T("DN_15") );
	
	if( iHighestSize < iDN15 )
	{
		iHighestSize = iDN15;
	}

	// Main loop scan all available PICV.
	// When Havail == 0.0 keep valve with minimum Fc, to minimize H needed.
	// When Havail > 0.0 keep valve with maximum Fc below Havail to maximalize Authority.
	CTable *pTab = dynamic_cast<CTable *>( pclBatchSelPIBCVParams->m_pTADB->Get( _T("PICTRLVALV_TAB") ).MP );

	if( NULL == pTab )
	{
		return false;
	}

	CTable *pclTableSchematic = (CTable *)( pclBatchSelPIBCVParams->m_pTADB->Get( L"CIRCSCHEME_TAB" ).MP );

	if( NULL == pclTableSchematic )
	{
		ASSERTA_RETURN( false );
	}

	mmapKeyDoubleSelectedProduct mapPICVFull;

	// To only clean 'm_vecObjects' if it already contains objects.
	CleanVecObjects();

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_PIControlValve *pPICV = dynamic_cast<CDB_PIControlValve *>( IDPtr.MP );

		if( NULL == pPICV )
		{
			ASSERT( 0 );
			continue;
		}

		if( false == pPICV->IsSelectable( true ) )
		{
			continue;
		}

		CDB_ControlProperties *pCtrlProp =  dynamic_cast<CDB_ControlProperties *>( pPICV->GetCtrlProp() );
		
		if( NULL == pCtrlProp )
		{
			continue;
		}

		if( NULL != pPICV->GetValveCharacteristic() )
		{
			double dPreset = pPICV->GetPresetting( dFlow, pclBatchSelPIBCVParams->m_WC.GetDens(), pclBatchSelPIBCVParams->m_WC.GetKinVisc() );
			// HYS-825: Presetting can be 0. The condition must be the default value -1
			if (dPreset == -1)
			{
				continue;
			}
		}

		// We can also here filtered type.
		if( ( pPICV->GetSizeKey() <= iDN50 && 0 != CString( pPICV->GetTypeID() ).Compare( pclBatchSelPIBCVParams->m_strComboTypeBelow65ID ) ) 
				|| ( pPICV->GetSizeKey() > iDN50 && 0 != CString( pPICV->GetTypeID() ).Compare( pclBatchSelPIBCVParams->m_strComboTypeAbove50ID ) ) )
		{
			continue;
		}

		// Verify ctrl properties.
		if( false == pCtrlProp->CvCtrlTypeFits( pclBatchSelPIBCVParams->m_eCvCtrlType ) )
		{
			continue;
		}

		// Check max flow.
		if( -1.0 == pPICV->GetQmax() || dFlow > pPICV->GetQmax() )
		{
			continue;
		}

		// To facilitate use in the 'CDlgBatchSelPICv' class.
		CSelectedValve *pSelPICV = new CSelectedValve();
		
		if( NULL == pSelPICV )
		{
			continue;
		}

		pSelPICV->SetProductIDPtr( pPICV->GetIDPtr() );
		pSelPICV->SetFlag( CSelectedBase::Flags::eNotPriority, false );

		double d64Key = pPICV->GetSortingKey();
		mapPICVFull.insert( std::make_pair( d64Key, pSelPICV ) );

		// To keep a trace of created object to well erase them when no more needed.
		m_vecObjects.push_back( pSelPICV );
	}

	// Key is the sorting key.
	mmapKeyDoubleSelectedProduct mapPICVRestrictive;
	mmapKeyDoubleSelectedProduct::iterator mapIt;

	// If user ask to be strict.
	if( true == pclBatchSelPIBCVParams->m_bIsCtrlTypeStrictChecked )
	{
		bool bAtLeastOneExist = false;

		for( mapIt = mapPICVFull.begin(); mapIt != mapPICVFull.end() && bAtLeastOneExist == false; ++mapIt )
		{
			CDB_ControlValve *pclControlValve = (CDB_ControlValve*)( mapIt->second->GetProductIDPtr().MP );

			if( CDB_ControlProperties::CvCtrlType::eCvOnOff == pclBatchSelPIBCVParams->m_eCvCtrlType 
				&& CDB_ControlProperties::eCTRLCHAR::NotCharacterized == pclControlValve->GetCtrlProp()->GetCvCtrlChar() )
			{
				bAtLeastOneExist = true;
			}
			else if( ( CDB_ControlProperties::CvCtrlType::eCv3point == pclBatchSelPIBCVParams->m_eCvCtrlType 
					   || CDB_ControlProperties::CvCtrlType::eCvProportional == pclBatchSelPIBCVParams->m_eCvCtrlType ) 
					 && CDB_ControlProperties::eCTRLCHAR::NotCharacterized != pclControlValve->GetCtrlProp()->GetCvCtrlChar() )
			{
				bAtLeastOneExist = true;
			}
		}

		if( true == bAtLeastOneExist )
		{
			for( mapIt = mapPICVFull.begin(); mapIt != mapPICVFull.end(); ++mapIt )
			{
				CDB_ControlValve *pclControlValve = (CDB_ControlValve*)( mapIt->second->GetProductIDPtr().MP );

				double d64Key = pclControlValve->GetSortingKey();
				bool bCanAdd = false;

				if( CDB_ControlProperties::CvCtrlType::eCvOnOff == pclBatchSelPIBCVParams->m_eCvCtrlType 
					&& CDB_ControlProperties::eCTRLCHAR::NotCharacterized == pclControlValve->GetCtrlProp()->GetCvCtrlChar() )
				{
					bCanAdd = true;
				}
				else if( ( CDB_ControlProperties::CvCtrlType::eCv3point == pclBatchSelPIBCVParams->m_eCvCtrlType
						   || CDB_ControlProperties::CvCtrlType::eCvProportional == pclBatchSelPIBCVParams->m_eCvCtrlType ) 
						 && CDB_ControlProperties::eCTRLCHAR::NotCharacterized != pclControlValve->GetCtrlProp()->GetCvCtrlChar() )
				{
					bCanAdd = true;
				}

				if( true == bCanAdd && 0 == mapPICVRestrictive.count( d64Key ) )
				{
					mapPICVRestrictive.insert( std::make_pair( d64Key, mapIt->second ) );
				}
			}
		}
	}

	for( int iLoop = 0; iLoop < 2; iLoop++ )
	{
		if( ( 0 == iLoop && 0 == mapPICVRestrictive.size() ) || ( 1 == iLoop && true == pclBatchSelPIBCVParams->m_bIsCtrlTypeStrictChecked ) )
		{
			continue;
		}

		mmapKeyDoubleSelectedProduct *pmapCurrentPICV = ( 0 == iLoop ) ? &mapPICVRestrictive : &mapPICVFull;

		bool bOneValveExist = false;
		int iSmallestValve = 0;
		int iHighestValve = 99999;
	
		// Adapt iLowestSize and iHigestSize to include at least one valve.
		for( mapIt = pmapCurrentPICV->begin(); mapIt != pmapCurrentPICV->end() && bOneValveExist == false; ++mapIt )
		{
			CDB_TAProduct *pclTAProduct = (CDB_TAProduct*)( mapIt->second->GetProductIDPtr().MP );

			int iProdSizeKey = pclTAProduct->GetSizeKey();
		
			if( iProdSizeKey < iHighestValve && iProdSizeKey > iHighestSize )
			{
				iHighestValve = iProdSizeKey;
			}
		
			if( iProdSizeKey > iSmallestValve && iProdSizeKey < iLowestSize )
			{
				iSmallestValve = iProdSizeKey;
			}
		
			if( iProdSizeKey >= iLowestSize && iProdSizeKey <= iHighestSize )
			{
				bOneValveExist = true;
			}
		}
	
		if( false == bOneValveExist )
		{
			if( iSmallestValve < iLowestSize )
			{
				iLowestSize = iSmallestValve;
			}
		
			if( iHighestValve > iHighestSize )
			{
				iHighestSize = iHighestValve;
			}
		}
		
		for( mapIt = pmapCurrentPICV->begin(); mapIt != pmapCurrentPICV->end(); )
		{
			CDB_PIControlValve *pclPIControlValve = (CDB_PIControlValve*)( mapIt->second->GetProductIDPtr().MP );

			int iProdSizeKey = pclPIControlValve->GetSizeKey();
		
			if( !( iProdSizeKey >= iLowestSize && iProdSizeKey <= iHighestSize ) )
			{
				if( mapIt == pmapCurrentPICV->begin() )
				{
					// First proposition is rejected
					// TODO: warning message in compilation Box
				}

				// Remove this selection.
				if( pmapCurrentPICV->size() > 1 )
				{
					mmapKeyDoubleSelectedProduct::iterator it = mapIt;
					++mapIt;
					
					// Don't delete the object here. Objects are saved in 'm_vecObjects' and there will be deleted when needed.

					pmapCurrentPICV->erase( it );
					continue;
				}
			}
			else
			{
				bOneValveExist = true;
			}

			++mapIt;
		}

		if( 0 == pmapCurrentPICV->size() )
		{
			continue;
		}

		// Build sorting keys.
		CTAPSortKey sKeyTechParamBlw65;
		CTAPSortKey sKeyTechParamAbv50;
		BuildSortingKeys( pclBatchSelPIBCVParams, &sKeyTechParamBlw65 , &sKeyTechParamAbv50 );

		bool bFound = false;
		int iUserPrefLoop = 0;

		for( ; iUserPrefLoop < 6 && false == bFound; iUserPrefLoop++ )
		{
			// This loop give us the possibility to relax mask criteria one by one.
			// We will start with the most constraining key and if we don't found a product we will relax constrains one by one.
			CTAPSortKey::TAPSortKeyMask eMask = ( CTAPSortKey::TAPSortKeyMask )( CTAPSortKey::TAPSKM_All & ~CTAPSortKey::TAPSKM_PriorL & ~CTAPSortKey::TAPSKM_Size );
			
			switch( iUserPrefLoop )
			{
				case 0:
					// No filter, we are strict.
					break;

				case 1:
					// Relax PN.
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
					break;

				case 2:
					// Relax PN and version.
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version );
					break;

				case 3:
					// Relax PN, version and connection.
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version & 
							~CTAPSortKey::TAPSKM_Connect );
					break;

				case 4:
					// Relax PN, version, connection and body material.
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version & 
							~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Bdy );
					break;

				case 5:
					// Relax PN, version, connection, body material and family.
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version & 
							~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Bdy & ~CTAPSortKey::TAPSKM_Fam );
					break;
			}

			double dKeyTechParamBlw65 = sKeyTechParamBlw65.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
			double dKeyTechParamAbv50 = sKeyTechParamAbv50.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
			mmapKeyDoubleSelectedProduct::iterator ItBest = pmapCurrentPICV->end();
			CDB_PIControlValve *pclBestPICV = NULL;
			double dBestRelPreSet = 0.0;
			double dBestDpMin = DBL_MAX;
			double dAcceptedExtraDp = pclTechParams->GetPICVExtraThresholdDp();

			for( mapIt = pmapCurrentPICV->begin(); mapIt != pmapCurrentPICV->end(); ++mapIt )
			{
				CDB_PIControlValve *pclPIControlValve = dynamic_cast<CDB_PIControlValve*>( mapIt->second->GetProductIDPtr().MP );

				bool bValveMatch = false;
				int iSize = pclPIControlValve->GetSizeKey();
				double dTAPKey = pclPIControlValve->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
			
				if( iSize <= iDN50 )
				{
					if( dTAPKey == dKeyTechParamBlw65 )
					{
						bValveMatch = true;
					}
				}
				else
				{
					// Size > DN50
					if( dTAPKey == dKeyTechParamAbv50 )
					{
						bValveMatch = true;
					}
				}

				if( false == bValveMatch )
				{
					continue;
				}

				double dDpMin = pclPIControlValve->GetDpmin( dFlow, pclBatchSelPIBCVParams->m_WC.GetDens() );
			
				if( -1.0 == dDpMin )
				{
					dDpMin = 0.0;
				}

				// Keep smallest Dpmin (FC) when Havail <= 0 / largest Dpmin below Havail when Havail >= 0.
				bool bNewBestExist = false;

				double dPresetting = pclPIControlValve->GetPresetting( dFlow, pclBatchSelPIBCVParams->m_WC.GetDens(), pclBatchSelPIBCVParams->m_WC.GetKinVisc() );
				double dMaxSetting = pclPIControlValve->GetMaxSetting();

				double dPresetRatio = 0.0;

				if( ItBest == pmapCurrentPICV->end() )
				{
					bNewBestExist = true;
				
					dPresetRatio = dPresetting / dMaxSetting;
				}
				else
				{
					// Specific treatment for TBV-CMP 3/4"HF and 1/2"HF same body as 25NF!
					// Assume analyzing object in decreasing size order
					bool bValveSharingSameBody = false;

					if( NULL != pclBestPICV && 0 == IDcmp( pclBestPICV->GetFamilyID(), _T("FAM_TBV-CMP") ) 
						&& 0 == IDcmp( pclPIControlValve->GetFamilyID(), _T("FAM_TBV-CMP") ) )
					{
						bValveSharingSameBody = ( pclPIControlValve->GetPICVCharacteristic() == pclBestPICV->GetPICVCharacteristic() );
						bValveSharingSameBody |= ( dPresetting < pclPIControlValve->GetPICVCharacteristic()->GetMinRecSetting() );
					}

					int iBestSize = ( (CDB_TAProduct*)( ItBest->second->GetProductIDPtr().MP ) )->GetSizeKey();
					int iSize = pclPIControlValve->GetSizeKey();

					// New valve is largest than best valve but its Dp(with extra Dp) is lower.
					// New valve is smallest than the best valve and its minDp is lower.
					if( iSize == iBestSize )
					{
						if( dDpMin < ( dBestDpMin + dAcceptedExtraDp ) )
						{
							if( -1.0 != dPresetting && -1.0 != dMaxSetting )
							{
								dPresetRatio = dPresetting / dMaxSetting;

								if( -1.0 != dPresetRatio && dPresetRatio > dBestRelPreSet )
								{
									bNewBestExist = true;
								}
							}
						}
					}
					else if( ( iSize < iBestSize && dDpMin < ( dBestDpMin + dAcceptedExtraDp ) && false == bValveSharingSameBody ) ||
							( iSize > iBestSize && ( dDpMin + dAcceptedExtraDp) < dBestDpMin ) )
					{
						bNewBestExist = true;
					}
				}

				if( true == bNewBestExist )
				{
					dBestDpMin = dDpMin;
					ItBest = mapIt;
					dBestRelPreSet = dPresetRatio;
					pclBestPICV = pclPIControlValve;
				}

				CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclPIControlValve, mapIt->second );
			}

			if( ItBest != pmapCurrentPICV->end() )
			{
				bFound = true;
				ItBest->second->SetFlag( CSelectedBase::eBest, true );

				if( 0 != iUserPrefLoop )
				{
					ItBest->second->SetFlag( CSelectedBase::eValveBatchAlter, true );
				}
			}
		}

		if( true == bFound )
		{
			break;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////
//	Selected balancing valve container for batch mode
//////////////////////////////////////////////////////////////////////////////////
CBatchSelectBvList::CBatchSelectBvList()
	: CBatchSelectBaseList()
{
}

void CBatchSelectBvList::TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList )
{
	if( NULL == pclIndividualSelectList )
	{
		ASSERT_RETURN;
	}

	CBatchSelectBaseList::TransferResultsToIndividualSelectList( pclIndividualSelectList );

	// For balancing valve we use the 'm_MMapKeyDecreasing' variable.
	for( auto &iter : m_MMapKeyDecreasing )
	{
		if( NULL == iter.second )
		{
			continue;
		}

		if( CSelectedBase::SBT_Valve == iter.second->GetSelectedBaseType() )
		{
			CSelectedValve *pclSelectedValve = new CSelectedValve();

			if( NULL == pclSelectedValve )
			{
				continue;
			}

			pclSelectedValve->CopyFrom( iter.second );
			pclIndividualSelectList->m_MMapKeyDecreasing.insert( ProductPair( iter.first, pclSelectedValve ) );
		}
	}
}

int CBatchSelectBvList::SelectBv( CBatchSelBVParams *pclBatchSelBVParams, double dFlow, double dDpRequired, CRankEx *pclBvList )
{
	if( NULL == pclBatchSelBVParams || NULL == pclBatchSelBVParams->m_pTADS || NULL == pclBatchSelBVParams->m_pTADS->GetpTechParams() 
			|| NULL == pclBatchSelBVParams->m_pTADB )
	{
		ASSERTA_RETURN( 0 );
	}

	if( dFlow <= 0.0 )
	{
		return BatchReturn::BR_BadArguments;
	}

	// Only best pipe requested.
	m_clSelectPipeList.SelectPipes( pclBatchSelBVParams, dFlow );
	m_pclBestPipe = m_clSelectPipeList.GetBestPipe();
	
	if( NULL == m_pclBestPipe && 1 == m_clSelectPipeList.GetPipeNumbers() )
	{
		m_pclBestPipe = m_clSelectPipeList.GetpPipeMap()->begin()->second.GetpPipe();
	}
	
	if( NULL == m_pclBestPipe )
	{
		return BatchReturn::BR_NoPipeFound;
	}

	bool bAtLeastOneBestStrict = false;
	bool bAtLeastOneBestAlter = false;
	CSelectedBase *pclBestProductStrict = NULL;
	CSelectedBase *pclBestProductAlter = NULL;

	if( true == _SelectBv( pclBatchSelBVParams, dFlow, dDpRequired, pclBvList ) )
	{
		// To facilitate use in the 'CDlgBatchSelBv' class.
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedBase *pclSelectedProduct = m_cIter->second;
			ASSERT( NULL != pclSelectedProduct );

			if( NULL == pclSelectedProduct )
			{
				continue;
			}

			CSelectedValve *pclSelectedBv = dynamic_cast<CSelectedValve *>( pclSelectedProduct );
			ASSERT( NULL != pclSelectedBv );

			if( NULL == pclSelectedBv )
			{
				continue;
			}

			if( true == pclSelectedBv->GetFlag( CSelectedBase::eBest ) )
			{
				if( false == pclSelectedBv->GetFlag( CSelectedBase::eValveBatchAlter ) )
				{
					bAtLeastOneBestStrict = true;
					pclBestProductStrict = pclSelectedBv;
				}
				else
				{
					bAtLeastOneBestAlter = true;
					pclBestProductAlter = pclSelectedBv;
				}
			}

			CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct*>( pclSelectedBv->GetpData() );
			ASSERT( NULL != pTAP );

			if( NULL == pTAP )
			{
				continue;
			}

			// Transfer valve into the final list with good priority key.
			CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pTAP, pclSelectedBv );
		}
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	// Set the return value.
	int iReturn;

	if( true == bAtLeastOneBestStrict && NULL != pclBestProductStrict )
	{
		m_pclBestProduct = pclBestProductStrict;
		iReturn = BatchReturn::BR_FoundOneBest;
	}
	else if( true == bAtLeastOneBestAlter && NULL != pclBestProductAlter )
	{
		m_pclBestProduct = pclBestProductAlter;
		iReturn = BatchReturn::BR_FoundOneBest | BatchReturn::BR_FoundAlernative;
	}
	else
	{
		iReturn = ( m_MMapKeyDecreasing.size() > 0 ) ? BatchReturn::BR_FoundButNotBest : BatchReturn::BR_NotFound;
	}

	return iReturn;
}

bool CBatchSelectBvList::_SelectBv( CBatchSelBVParams *pclBatchSelBVParams, double dFlow, double dDpRequired, CRankEx *pclBvList )
{
	if( NULL == m_pclBestPipe )
	{
		return false;
	}

	CDS_TechnicalParameter *pclTechParams = pclBatchSelBVParams->m_pTADS->GetpTechParams();

	// Take in count round problems.
	int iHighestSize = 0;
	int iLowestSize = 0;
	
	if( false == GetPipeSizeShift( (CBatchSelectionParameters*)pclBatchSelBVParams, iHighestSize, iLowestSize ) )
	{
		return false;
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclBatchSelBVParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN50 = pclTableDN->GetSize( _T("DN_50") );

	// Use the product type in regards to the selected pipe.
	int iDN = m_pclBestPipe->GetSizeKey( pclBatchSelBVParams->m_pTADB );
	CDB_StringID *pBVType;
	double dPN;
	
	if( iDN <= iDN50 )
	{
		pBVType = (CDB_StringID *)( pclBatchSelBVParams->m_pTADB->Get( (LPCTSTR)pclBatchSelBVParams->m_strComboTypeBelow65ID ).MP );
		dPN = _tcstod( ( (CDB_StringID *)( pclBatchSelBVParams->m_pTADB->Get( (LPCTSTR)pclBatchSelBVParams->m_strComboPNBelow65ID ).MP ) )->GetIDstr(), _T('\0') );
	}
	else
	{
		pBVType = (CDB_StringID *)( pclBatchSelBVParams->m_pTADB->Get( (LPCTSTR)pclBatchSelBVParams->m_strComboTypeAbove50ID ).MP );
		dPN = _tcstod( ( (CDB_StringID *)( pclBatchSelBVParams->m_pTADB->Get( (LPCTSTR)pclBatchSelBVParams->m_strComboPNAbove50ID ).MP ) )->GetIDstr(), _T('\0') );
	}
		
	CRankEx List;
	CRankEx *pclListToUse = &List;

	if( NULL == pclBvList )
	{
		// Establish valve list based on valve type and the best technical choice.
		pclBatchSelBVParams->m_pTADB->GetBVList( &List, pBVType->GetIDPtr().ID, L"", L"", L"", L"", pclBatchSelBVParams->m_eFilterSelection, 
				0, INT_MAX, NULL, false );
	}
	else
	{
		pclListToUse = pclBvList;
	}

	// To only clean the 'm_vecObjects' variable if it already contains all the 'CSelectedBase' objects.
	CleanVecObjects();

	LPARAM lparam;
	_string str;
	std::multimap<int, CDB_TAProduct *> mapBv;

	// Extract valves according SizeShift, keep at least one product size.
	for( BOOL bContinue = pclListToUse->GetFirst( str, lparam ); TRUE == bContinue; bContinue = pclListToUse->GetNext( str, lparam ) )
	{
		CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *)lparam );

		if( NULL == pTAP )
		{
			ASSERT( 0 );
			continue;
		}

		int iProdSizeKey = pTAP->GetSizeKey();
		mapBv.insert( std::make_pair( iProdSizeKey, pTAP ) );
	}

	if( 0 == mapBv.size() )
	{
		return false;
	}

	std::multimap<int, CDB_TAProduct *>::iterator itLow = mapBv.find( iLowestSize );
	std::multimap<int, CDB_TAProduct *>::iterator itUp = mapBv.find( iHighestSize );
	
	if( mapBv.end() == itLow && mapBv.end() == itUp )
	{
		// Valve with correct size not found, keep smallest or highest valve.
		std::multimap<int, CDB_TAProduct *>::iterator iter = mapBv.begin();
		
		if( iter->first > iHighestSize )
		{
			// All valves size are above the highest size, keep smallest valve size.
			itLow = mapBv.upper_bound( iter->first );
			mapBv.erase( itLow, mapBv.end() );
		}
		
		std::multimap<int, CDB_TAProduct *>::reverse_iterator rIter = mapBv.rbegin();
		
		if( rIter->first < iLowestSize )
		{
			// All valves size are below the lowest size, keep highest valve size.
			itUp = mapBv.lower_bound( rIter->first );
			mapBv.erase( mapBv.begin(), mapBv.end() );
		}
	}
	else
	{
		if( mapBv.end() != itLow && mapBv.begin() != itLow )
		{
			mapBv.erase( mapBv.begin(), itLow );
		}
		
		itUp = mapBv.upper_bound( iHighestSize );
		
		if( mapBv.end() != itUp )
		{
			mapBv.erase( itUp, mapBv.end() );
		}
	}

	if( 0 == mapBv.size() )
	{
		return false;
	}

	// To facilitate use in the 'CDlgBatchSelBv' class.
	for( auto &iter : mapBv )
	{
		CSelectedValve *pclSelectedValve = new CSelectedValve();

		if( NULL == pclSelectedValve )
		{
			ASSERT( 0 );
			continue;
		}

		pclSelectedValve->SetProductIDPtr( iter.second->GetIDPtr() );
		m_mmapProductList.insert( std::make_pair( iter.first, pclSelectedValve ) );

		// To keep a trace of created object to well erase them when no more needed.
		m_vecObjects.push_back( pclSelectedValve );
	}
	
	mmapKeyDoubleSelectedProduct mapDelta;

	if( 0 == _tcscmp( pBVType->GetIDPtr().ID, _T("RVTYPE_CS") ) )
	{
		double dSignalMax = pclTechParams->GetFoMaxDp();

		for( auto &iter : m_mmapProductList )
		{
			CDB_CommissioningSet *pCSValve = dynamic_cast<CDB_CommissioningSet *>( iter.second->GetProductIDPtr().MP );
			
			if( NULL == pCSValve )
			{
				ASSERT( 0 );
				continue;
			}

			int iProdSizeKey = iter.first;
			
			if( iProdSizeKey < iLowestSize || iProdSizeKey > iHighestSize )
			{
				continue;
			}

			double dDpFixO = 0.0;
			double dKv = pCSValve->GetKvFixO();
			
			if( -1.0 != dKv )
			{
				dDpFixO = CalcDp( dFlow, dKv, pclBatchSelBVParams->m_WC.GetDens() );
			}

			double dKvSignal = pCSValve->GetKvSignal();
			
			if( -1.0 == dKvSignal )
			{
				continue;
			}

			CSelectedValve *pclSelectedValve = dynamic_cast<CSelectedValve *>( iter.second );

			if( NULL == pclSelectedValve )
			{
				ASSERT( 0 );
				continue;
			}

			pclSelectedValve->SetDp( dDpRequired );
			pclSelectedValve->SetFlag( CSelectedBase::eValveBatchAlter, false );
			pclSelectedValve->SetFlag( CSelectedBase::Flags::eNotPriority, false );

			double dDpSignal = CalcDp( dFlow, dKvSignal, pclBatchSelBVParams->m_WC.GetDens() );
			dDpSignal += dDpFixO;
			pclSelectedValve->SetDpSignal( dDpSignal );

			if( dDpSignal <= dSignalMax )
			{
				double dDeltaSignal = dSignalMax - dDpSignal;	// Should be > 0
				mapDelta.insert( std::make_pair( dDeltaSignal, pclSelectedValve ) );
			}
			else
			{
				// Fully open valve cannot reach requested Dp.
				double dDeltaSignal = dDpSignal - dSignalMax;		// Should be > 0
				mapDelta.insert( std::make_pair( dDeltaSignal, pclSelectedValve ) );
			}
		}
	}
	else if( 0 == _tcscmp( pBVType->GetIDPtr().ID, _T("RVTYPE_FO") ) )
	{
		;
	}
	else if( 0 == _tcscmp( pBVType->GetIDPtr().ID, _T("RVTYPE_VV") ) )
	{
		// HYS-1436: we differentiate the venturi valve selection from the regulating valve.

		// From Jean-Christophe Carette on Slack (26th August 2020):
		// Target Dp needs to be seen as Dp corresponding to ideal KvSignal (for any given design flow).		
		// Ideal KvSignal is the one allowing to cover best flow range above and below the given design flow		
		// while staying between VTRI_MIN_DP and VTRI_MAX_DP.		
		
		// Target Dp is thus taken as the square of the arithmetic average of the upper and lower flows		
		// corresponding to VTRI_MAX_DP and VTRI_MIN_DP
		double dTargetSignal = pow( ( sqrt( pclTechParams->GetVtriMinDp() ) + sqrt( pclTechParams->GetVtriMaxDp() ) ), 2 ) / 4.0;

		// Compute the signal pressure drop.
		double dRho = pclBatchSelBVParams->m_WC.GetDens();
		double dNu = pclBatchSelBVParams->m_WC.GetKinVisc();
		double dTemperature = pclBatchSelBVParams->m_WC.GetTemp();
		double dBestDelta = DBL_MAX;
		std::pair<double, CSelectedBase *> pairBestVV( 0.0, NULL );

		// These two variables are used when dp required is defined.
		// It can happen that we have no valve with the dp required. In this case we will take the worse one.
		mmapKeyDoubleSelectedProduct mapTempDelta;
		double dBestWorse = DBL_MAX;

		for( auto &iter : m_mmapProductList )
		{
			CDB_VenturiValve *pclVenturiValve = dynamic_cast<CDB_VenturiValve *>( iter.second->GetProductIDPtr().MP );

			if( NULL == pclVenturiValve )
			{
				continue;
			}

			CDB_ValveCharacteristic *pValveChar = pclVenturiValve->GetValveCharacteristic();

			if( NULL == pValveChar )
			{
				continue;
			}

			double dKvSignal = pclVenturiValve->GetKvSignal();
	
			if( -1.0 == dKvSignal )
			{
				continue;
			}
	
			double dDpSignal = CalcDp( dFlow, dKvSignal, dRho );
		
			// Get Kv characteristic of the VV.
			CDB_ValveCharacteristic *pDevChar = pclVenturiValve->GetVvCurve();

			if( NULL == pDevChar )
			{
				continue;
			}

			// Compute the total pressure drop for the DRV fully open and the VV half open.
			double dDpFullyOpen;
			double dDpHalfOpen;
			double dHMax = pDevChar->GetOpeningMax();
	
			if( -1.0 == dHMax )
			{
				continue;
			}
	
			if( false == pDevChar->GetValveDp( dFlow, &dDpFullyOpen, dHMax, dRho, dNu ) )
			{
				continue;
			}

			double dHHalf = 0.5 * dHMax;

			if( false == pDevChar->GetValveDp( dFlow, &dDpHalfOpen, dHHalf, dRho, dNu ) )
			{
				continue;
			}
	
			// Compute hand wheel setting.
			double dH = -1.0;
			double dDp;
			bool bNotFound;
			bool bIsDpDefined = ( dDpRequired > 0.0 ) ? true : false;
	
			// Selection process from flow and Dp if DpOK==true or from flow only if DpOK==false.
			if( true == bIsDpDefined )
			{
				// If the selected Dp is obtainable...
				if( dDpRequired >= dDpFullyOpen )
				{
					// Try to find a solution.
					dDp = dDpRequired;
					bool bValid = pDevChar->GetValveOpening( dFlow, dDp, &dH, dRho, dNu, 1 );
				
					if( false == bValid )
					{
						// The min Kv value was not small enough !
						dH = pDevChar->GetOpeningMin();

						if( -1.0 != dH )
						{
							if( false == pDevChar->GetValveDp( dFlow, &dDp, dH, dRho, dNu ) )
							{
								return false;
							}
						}
					}
				}
			}
			else
			{
				dH = dHMax;
				dDp = dDpFullyOpen;
			}

			// If h>=0, a solution was found.
			bNotFound = ( dH < 0.0 ) ? true : false;
		
			if( true == bNotFound )
			{
				dH = dHMax;
				dDp = dDpFullyOpen;
			}

			CSelectedValve *pclSelectedVenturiValve = dynamic_cast<CSelectedValve *>( iter.second );

			if( NULL == pclSelectedVenturiValve )
			{
				ASSERT( 0 );
				continue;
			}
	
			pclSelectedVenturiValve->SetProductIDPtr( pclVenturiValve->GetIDPtr() );

			pclSelectedVenturiValve->SetDp( dDp );
			pclSelectedVenturiValve->SetDpSignal( dDpSignal );
			pclSelectedVenturiValve->SetDpFullOpen( dDpFullyOpen );
			pclSelectedVenturiValve->SetDpHalfOpen( dDpHalfOpen );
			pclSelectedVenturiValve->SetH( dH );

			pclSelectedVenturiValve->SetFlag( CSelectedBase::Flags::eValveSetting, false );
			pclSelectedVenturiValve->SetFlag( CSelectedBase::Flags::eValveMaxSetting, ( dH == dHMax ) ? true : false );

			CDS_TechnicalParameter *pclTechParams = pclBatchSelBVParams->m_pTADS->GetpTechParams();

			pclSelectedVenturiValve->SetFlag( CSelectedBase::Flags::eDpSignal, dDpSignal < pclTechParams->GetVtriMinDp() || dDpSignal > pclTechParams->GetVtriMaxDp() );
	
			if( dDp < pclTechParams->GetValvMinDp() )
			{
				pclSelectedVenturiValve->SetFlag( CSelectedBase::Flags::eDp, true, BR_DpBelowMinDp );
			}
			else if( dDp > pclTechParams->GetValvMaxDp() )
			{
				pclSelectedVenturiValve->SetFlag( CSelectedBase::Flags::eDp, true, BR_DpAboveMaxDp );
			}

			pclSelectedVenturiValve->SetFlag( CSelectedBase::Flags::eValveHalfODp, false );
			pclSelectedVenturiValve->SetFlag( CSelectedBase::Flags::eValveFullODp, bIsDpDefined && bNotFound );
			pclSelectedVenturiValve->SetFlag( CSelectedBase::Flags::ePN, dPN > pclVenturiValve->GetPmaxmax() );
			pclSelectedVenturiValve->SetFlag( CSelectedBase::Flags::eTemperature, dTemperature < pclVenturiValve->GetTmin() || dTemperature > pclVenturiValve->GetTmax() );

			pclSelectedVenturiValve->SetFlag( CSelectedBase::Flags::eNotPriority, true );

			double dDelta = 0.0;

			if( 0.0 == dDpRequired )
			{
				dDelta = abs( pclSelectedVenturiValve->GetDpSignal() - dTargetSignal );
				
				if( dDelta < dBestDelta && pclSelectedVenturiValve->GetDpSignal() > pclTechParams->GetVtriMinDp() )
				{
					dBestDelta = dDelta;
				}
				
				mapDelta.insert( std::make_pair( dDelta, pclSelectedVenturiValve ) );
			}
			else
			{
				// The best valve is the one that is the closest to 75% of opening.
				CDB_ValveCharacteristic *pValveCharacteristic = (CDB_ValveCharacteristic *)pclVenturiValve->GetValveCharDataPointer();
								
				if( NULL != pValveCharacteristic )
				{
					dDelta = fabs( pclSelectedVenturiValve->GetH() - ( 0.75 * pValveCharacteristic->GetOpeningMax() ) );
					pclSelectedVenturiValve->SetBestDelta( dDelta );

					if( dDelta < dBestDelta )
					{
						if( pclSelectedVenturiValve->GetDp() == dDpRequired )
						{
							// If Dp required is reached, we take this valve as the best...
							dBestDelta = dDelta;
							mapDelta.insert( std::make_pair( dDelta, pclSelectedVenturiValve ) );
						}
						else
						{
							// Dp required not reached, we take this one in case of there is no other valve that can
							// reach the dp. We will take the one that has its Dp at full opening the closest to the dp required.
							if( fabs( pclSelectedVenturiValve->GetDp() - dDpRequired ) < dBestWorse )
							{
								dBestWorse = fabs( pclSelectedVenturiValve->GetDp() - dDpRequired );
								pclSelectedVenturiValve->SetBestDelta( dBestWorse );
								mapTempDelta.insert( std::make_pair( dBestWorse, pclSelectedVenturiValve ) );
							}
						}
					}
				}
			}
		}

		if( dDpRequired > 0.0 && DBL_MAX == dBestDelta )
		{
			// If no perfect match was found we take the valves that are the closest to the dp required.
			mapDelta = mapTempDelta;
		}
	}
	else
	{
		bool bTargetDpUnknown = false;

		if( dDpRequired <= 0.0 )
		{
			dDpRequired = pclTechParams->GetValvMinDp();
			bTargetDpUnknown = true;
		}

		double dTargetInPercentOfOpening = 0.75;

		// For each valve found
		// Establish two ordered list
		//			Dp reached store quality of settings (setting to reach Dp minus optimum settings)
		//			Dp not reached, valve is fully open store Dp difference

		// Create a map that keep already computed characteristics
		// target of this map is to increase processing
		std::map <CDB_ValveCharacteristic *, std::pair<double, double>> mapValveChar;				// std::pair<dDpFO, dH>
		std::map <CDB_ValveCharacteristic *, std::pair<double, double>>::iterator itValveChar;
		CDB_ValveCharacteristic *pValveChar65 = NULL;

		for( auto &iter : m_mmapProductList )
		{
			CDB_TAProduct *pclBalancingValve = dynamic_cast<CDB_TAProduct*>( iter.second->GetProductIDPtr().MP );

			if( NULL == pclBalancingValve )
			{
				continue;
			}

			CDB_ValveCharacteristic *pValveChar = pclBalancingValve->GetValveCharacteristic();

			if( NULL == pValveChar )
			{
				continue;
			}

			double dMaxSetting = pValveChar->GetOpeningMax();

			if( -1.0 == dMaxSetting )
			{
				continue;
			}

			CSelectedValve *pclSelectedValve = dynamic_cast<CSelectedValve *>( iter.second );

			if( NULL == pclSelectedValve )
			{
				ASSERT( 0 );
				continue;
			}

			int iSizeKey = pclBalancingValve->GetSizeKey();

			// Specific treatment for STAF65-2 vs STAF80 and for settings included between 2.6 && 4.8.
			if( 8 == iSizeKey )	// STAF 65-2
			{
				pValveChar65 = pValveChar;
			}

			pclSelectedValve->SetFlag( CSelectedBase::Flags::eValveBatchAlter, false );
			pclSelectedValve->SetFlag( CSelectedBase::Flags::eNotPriority, false );

			// When the target Dp setting is known we try to find a valve with an opening of 75%
			double dTargetSetting = dMaxSetting;

			if( false == bTargetDpUnknown )
			{
				dTargetSetting *= dTargetInPercentOfOpening;
			}

			double dDpWithSettingFullyOpened = 0.0;
			itValveChar = mapValveChar.find( pValveChar );

			if( itValveChar != mapValveChar.end() )
			{
				dDpWithSettingFullyOpened = itValveChar->second.first;
			}
			else
			{
				dDpWithSettingFullyOpened = pValveChar->GetDpFullOpening( dFlow, pclBatchSelBVParams->m_WC.GetDens(), pclBatchSelBVParams->m_WC.GetKinVisc() );
			}

			double dSettingHalfOpened = 0.5 * pValveChar->GetOpeningMax();
			double dDpWithSettingHalfOpened = 0.0;
			pValveChar->GetValveDp( dFlow, &dDpWithSettingHalfOpened, dSettingHalfOpened, pclBatchSelBVParams->m_WC.GetDens(), pclBatchSelBVParams->m_WC.GetKinVisc() );

			if( true == pclBalancingValve->IsKvSignalEquipped() )	// Venturi valves
			{
				double dKvSignal = pclBalancingValve->GetKvSignal();

				if( -1.0 != dKvSignal )
				{
					double dSignal = CalcDp( dFlow, dKvSignal, pclBatchSelBVParams->m_WC.GetDens() );
					pclSelectedValve->SetDpSignal( dSignal );
				}
			}

			double dH = -1;
			bool bTest = false;

			if( true == bTargetDpUnknown )
			{
				// We don't know what is the target opening, we will accept all valves with a Dp@FO < ValvMin2Dp
				// normally m_dDP = ValveMinDp() < ValveMin2Dp
				bTest  = ( dDpRequired <= pclTechParams->GetValvMin2Dp() ) && ( dDpWithSettingFullyOpened <= pclTechParams->GetValvMin2Dp() );
			}
			else
			{
				bTest = ( dDpRequired > dDpWithSettingFullyOpened );
			}

			if( true == bTest )
			{
				// Valve Dp @ full opening is below ValvMin2Dp (6kPa)
				bool bflag = true;

				if( itValveChar != mapValveChar.end() )
				{
					dH = itValveChar->second.second;
				}
				else
				{
					bflag = pValveChar->GetValveOpening( dFlow, dDpRequired, &dH, pclBatchSelBVParams->m_WC.GetDens(), pclBatchSelBVParams->m_WC.GetKinVisc(), 
							( eBool3::eb3True == pValveChar->IsMultiTurn() ) ? 0 : 1 );
				}
				
				// Valve at full opening cannot reach requested Dp.
				double dDpToSave = dDpRequired;

				if( false == bflag )
				{
					dDpToSave = dDpWithSettingFullyOpened;
					dH = pValveChar->GetOpeningMax();
				}

				// Specific treatment for STAF65-2 vs STAF80 for settings included between 2.6 && 4.8.
				// For computing dDeltaSetting we align STAF80 curve to the STAF65-2 curve, selection based on size is done later.
				double dDeltaSetting = 0.0;

				if( 9 == iSizeKey && NULL != pValveChar65 && dH > 2.6 && dH < 4.8 )	// Size 65 & 80
				{
					double dH65 = mapValveChar[pValveChar65].second;
					dDeltaSetting = abs( dTargetSetting - dH65 );	
				}
				else
				{
					dDeltaSetting = abs( dTargetSetting - dH );	
				}

				mapDelta.insert( std::make_pair( dDeltaSetting, pclSelectedValve ) );
				
				// Update Main map with computed data.
				pclSelectedValve->SetDp( dDpToSave );
				pclSelectedValve->SetDpFullOpen( dDpWithSettingFullyOpened );
				pclSelectedValve->SetDpHalfOpen( dDpWithSettingHalfOpened );
				pclSelectedValve->SetH( dH );
			}
			else
			{
				// Fully open valve cannot reach requested Dp.
				double dDeltaDp = ( dDpWithSettingFullyOpened - dDpRequired ) * 100.0;		// Should be > 0
				mapDelta.insert( std::make_pair( dDeltaDp, pclSelectedValve ) );
				
				// Update Main map with computed data
				pclSelectedValve->SetDp( dDpWithSettingFullyOpened );
				pclSelectedValve->SetDpFullOpen( dDpWithSettingFullyOpened );
				pclSelectedValve->SetDpHalfOpen( dDpWithSettingHalfOpened );
				pclSelectedValve->SetH( dMaxSetting );
			}

			// Store new characteristic results.
			if( itValveChar == mapValveChar.end() )
			{
				mapValveChar[pValveChar] = std::pair<double, double> ( dDpWithSettingFullyOpened, dH );
			}
		}
	}

	mmapKeyDoubleSelectedProduct::iterator It = mapDelta.end();
	mmapKeyDoubleSelectedProduct::iterator ItBest = mapDelta.end();
	bool bBestSet = false;

	// Build sorting keys.
	CTAPSortKey sKeyTechParamBlw65;
	CTAPSortKey sKeyTechParamAbv50;
	BuildSortingKeys( pclBatchSelBVParams, &sKeyTechParamBlw65 , &sKeyTechParamAbv50 );

	double dKeyCurrentTAP = 0;

	// Try to find solution.
	if( mapDelta.size() > 0 )
	{
		for( int iLoop = 0; iLoop < 6; iLoop++ )
		{
			// This loop give us the possibility to relax mask criteria one by one.
			// We will start with the most constraining key and if we don't found a product we will relax constrains one by one.
			CTAPSortKey::TAPSortKeyMask eMask = ( CTAPSortKey::TAPSortKeyMask )( CTAPSortKey::TAPSKM_All & ~CTAPSortKey::TAPSKM_PriorL 
					& ~CTAPSortKey::TAPSKM_Size );

			m_PreselMMapKeyDecreasing.clear();

			switch( iLoop )
			{
				case 0:
					// Strict.
					break;
	
				case 1:
					// Relax PN.
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
					break;
					
				case 2:
					// Relax PN and Version.
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version );
					break;
					
				case 3:
					// Relax PN, version and connection.
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version 
							& ~CTAPSortKey::TAPSKM_Connect );
					break;
					
				case 4:
					// Relax PN, version, connection and body material.
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version 
							& ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Bdy );
					break;
					
				case 5:
					// Relax PN, version, connection, body material and family.
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version 
							& ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Bdy & ~CTAPSortKey::TAPSKM_Fam );
					break;
			}

			double dKeyTechParamBlw65 = sKeyTechParamBlw65.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
			double dKeyTechParamAbv50 = sKeyTechParamAbv50.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );

			// Previous valve doesn't exist, restart loop only on tech params.
			for( It = mapDelta.begin(); It != mapDelta.end(); ++It )
			{
				CDB_TAProduct *pTAP = (CDB_TAProduct*)( It->second->GetProductIDPtr().MP );
				int iSize = pTAP->GetSizeKey();
				double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
				bool bValveMatched = false;

				// Selected valve is not located on the same side of iDN50 border.
				// Use defined technical choice.
				if( iSize <= iDN50 )
				{
					if( dTAPKey == dKeyTechParamBlw65 )
					{
						bValveMatched = true;
					}
				}
				else
				{
					// Size > DN50
					if( dTAPKey == dKeyTechParamAbv50 )
					{
						bValveMatched = true;
					}
				}

				if( true == bValveMatched )
				{
					CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pTAP, It->second );

					if( false == bBestSet )
					{
						ItBest = It;
						bBestSet = true;
					}
												
					// Continue loop to save all valves in 'm_PreselMMap' that belongs to the same criterion.
				}
			}

			if( true == bBestSet )
			{
				ItBest->second->SetFlag( CSelectedBase::eBest, true );

				if( 0 != iLoop )
				{
					ItBest->second->SetFlag( CSelectedBase::eValveBatchAlter, true );
				}

				break;
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////
//	Selected separator for batch mode
//////////////////////////////////////////////////////////////////////////////////
CBatchSelectSeparatorList::CBatchSelectSeparatorList()
	: CBatchSelectBaseList()
{
	Clean();
}

void CBatchSelectSeparatorList::TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList )
{
	if( NULL == pclIndividualSelectList )
	{
		ASSERT_RETURN;
	}

	CBatchSelectBaseList::TransferResultsToIndividualSelectList( pclIndividualSelectList );

	// For separator we use the 'm_MMapKeyDecreasing' variable.
	for( auto &iter : m_MMapKeyDecreasing )
	{
		if( NULL == iter.second )
		{
			continue;
		}

		if( CSelectedBase::SBT_Separator == iter.second->GetSelectedBaseType() )
		{
			CSelectedSeparator *pclSelectedSeparator = new CSelectedSeparator();

			if( NULL == pclSelectedSeparator )
			{
				continue;
			}

			pclSelectedSeparator->CopyFrom( iter.second );
			pclIndividualSelectList->m_MMapKeyDecreasing.insert( ProductPair( iter.first, pclSelectedSeparator ) );
		}
	}
}

int CBatchSelectSeparatorList::SelectSeparator( CBatchSelSeparatorParams *pclBatchSelSeparatorParams, double dFlow )
{
	if( NULL == pclBatchSelSeparatorParams || NULL == pclBatchSelSeparatorParams->m_pTADS || NULL == pclBatchSelSeparatorParams->m_pTADS->GetpTechParams()
			|| NULL == pclBatchSelSeparatorParams->m_pTADB )
	{
		ASSERTA_RETURN( BatchReturn::BR_BatchSelParamsNULL );
	}

	if( dFlow <= 0.0 )
	{
		return BatchReturn::BR_BadArguments;
	}

	// Only best pipe requested.
	m_clSelectPipeList.SelectPipes( pclBatchSelSeparatorParams, dFlow );
	m_pclBestPipe = m_clSelectPipeList.GetBestPipe();

	if( NULL == m_pclBestPipe && 1 == m_clSelectPipeList.GetPipeNumbers() )
	{
		m_pclBestPipe = m_clSelectPipeList.GetpPipeMap()->begin()->second.GetpPipe();
	}
	
	if( NULL == m_pclBestPipe )
	{
		return BatchReturn::BR_NoPipeFound;
	}

	bool bAtLeastOneBestStrict = false;
	bool bAtLeastOneBestAlter = false;
	CSelectedBase *pclBestProductStrict = NULL;
	CSelectedBase *pclBestProductAlter = NULL;

	if( true == _SelectSeparator( pclBatchSelSeparatorParams, dFlow ) )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedValve *pSelTap = (CSelectedValve *)m_cIter->second;
			ASSERT( NULL != pSelTap );

			if( NULL == pSelTap )
			{
				continue;
			}

			CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator *>( pSelTap->GetProductIDPtr().MP );
			ASSERT( NULL != pclSeparator );
			
			if( NULL == pclSeparator )
			{
				continue;
			}

			if( true == pSelTap->GetFlag( CSelectedBase::eBest ) )
			{
				if( false == pSelTap->GetFlag( CSelectedBase::eValveBatchAlter ) )
				{
					bAtLeastOneBestStrict = true;
					pclBestProductStrict = pSelTap;
				}
				else
				{
					bAtLeastOneBestAlter = true;
					pclBestProductAlter = pSelTap;
				}
			}

			CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pclSeparator, pSelTap );
		}
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	// Set the return value.
	int iReturn;

	if( true == bAtLeastOneBestStrict && NULL != pclBestProductStrict )
	{
		m_pclBestProduct = pclBestProductStrict;
		iReturn = BatchReturn::BR_FoundOneBest;
	}
	else if( true == bAtLeastOneBestAlter && NULL != pclBestProductAlter )
	{
		m_pclBestProduct = pclBestProductAlter;
		iReturn = BatchReturn::BR_FoundOneBest | BatchReturn::BR_FoundAlernative;
	}
	else
	{
		iReturn = ( m_MMapKeyDecreasing.size() > 0 ) ? BatchReturn::BR_FoundButNotBest : BatchReturn::BR_NotFound;
	}

	return iReturn;
}

void CBatchSelectSeparatorList::Clean()
{
	m_bBestFound = false;
	m_bSizeShiftProblem = false;
	CBatchSelectBaseList::Clean();
}

bool CBatchSelectSeparatorList::_SelectSeparator( CBatchSelSeparatorParams *pclBatchSelSeparatorParams, double dFlow )
{
	// !!! REMARK: for the moment, it's exactly the same code as for the individual selection.
	
	m_bBestFound = false;

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return false;
	}

	// Get pointers on required database tables
	CTable *pTab = (CTable*)( pclBatchSelSeparatorParams->m_pTADB->Get( _T("SEPARATOR_TAB") ).MP );
	ASSERT( NULL != pTab );
	
	if( NULL == pTab )
	{
		return false;
	}

	int iHighestSize;
	int iLowestSize;

	if( false == GetPipeSizeShift( (CBatchSelectionParameters*)pclBatchSelSeparatorParams, iHighestSize, iLowestSize ) )
	{
		return false;
	}

	// Take all available products of the same type.
	CRankEx clSeparatorList;

	pclBatchSelSeparatorParams->m_pTADB->GetSeparatorList( &clSeparatorList, (LPCTSTR)pclBatchSelSeparatorParams->m_strComboTypeID, L"", L"", L"",
			pclBatchSelSeparatorParams->m_eFilterSelection );

	if( 0 == clSeparatorList.GetCount() )
	{
		return false;
	}

	bool bAtLeastOneFit = false;
	m_bSizeShiftProblem = false;
	mmapKeyDoubleSelectedProduct mmapSeparator;
	_string str;
	LPARAM lpParam;
	
	for( BOOL bContinue = clSeparatorList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = clSeparatorList.GetNext( str, lpParam ) )
	{
		CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator *>( (CData *)lpParam );
		ASSERT( NULL != pclSeparator );
		
		if( NULL == pclSeparator )
		{
			continue;
		}

		if( false == pclSeparator->IsSelectable( true ) )
		{
			continue;
		}

		CDB_QDpCharacteristic *pclQDpCharacteristic = pclSeparator->GetQDpCharacteristic();

		if( NULL == pclQDpCharacteristic )
		{
			clSeparatorList.Delete();
			continue;
		}

		if( dFlow < pclQDpCharacteristic->GetQmin() || dFlow > pclQDpCharacteristic->GetQmax() )
		{
			clSeparatorList.Delete();
			continue;
		}

		// Create a new entry in pre-selection linked list.
		CSelectedSeparator *pSelSeparator = new CSelectedSeparator();
		ASSERT( NULL != pSelSeparator );

		if( NULL == pSelSeparator )
		{
			continue;
		}

		pSelSeparator->SetProductIDPtr( pclSeparator->GetIDPtr() );
		pSelSeparator->SetDp( pclQDpCharacteristic->GetDp( dFlow, pclBatchSelSeparatorParams->m_WC.GetDens() ) );
		pSelSeparator->SetFlag( CSelectedBase::Flags::eNotPriority, false );

		double d64Key = pclSeparator->GetSortingKey();
		mmapSeparator.insert( std::make_pair( d64Key, pSelSeparator ) );
	}

	bool bOneValveExist = false;
	int iSmallestValve = 0;
	int iHighestValve = 99999;
	
	// Key is the sorting key.
	mmapKeyDoubleSelectedProduct::iterator mapIt;

	// Adapt 'dLowestSize' and 'iHigestSize' to include at least one separator.
	for( mapIt = mmapSeparator.begin(); mapIt != mmapSeparator.end() && bOneValveExist == false; ++mapIt )
	{
		CDB_TAProduct *pclTAProduct = (CDB_TAProduct*)( mapIt->second->GetProductIDPtr().MP );

		if( NULL == pclTAProduct )
		{
			continue;
		}

		int iProdSizeKey = pclTAProduct->GetSizeKey();
		
		if( iProdSizeKey < iHighestValve && iProdSizeKey > iHighestSize )
		{
			iHighestValve = iProdSizeKey;
		}

		if( iProdSizeKey > iSmallestValve && iProdSizeKey < iLowestSize )
		{
			iSmallestValve = iProdSizeKey;
		}
		
		if( iProdSizeKey >= iLowestSize && iProdSizeKey <= iHighestSize )
		{
			bOneValveExist = true;
		}
	}
	
	if( false == bOneValveExist )
	{
		if( iSmallestValve < iLowestSize )
		{
			iLowestSize = iSmallestValve;
		}
		
		if( iHighestValve > iHighestSize )
		{
			iHighestSize = iHighestValve;
		}
	}

	for( mapIt = mmapSeparator.begin(); mapIt != mmapSeparator.end(); )
	{
		CDB_Separator *pclSeparator = (CDB_Separator*)( mapIt->second->GetProductIDPtr().MP );

		if( NULL == pclSeparator )
		{
			continue;
		}

		int iProdSizeKey = pclSeparator->GetSizeKey();
		
		if( !( iProdSizeKey >= iLowestSize && iProdSizeKey <= iHighestSize ) )
		{
			if( mapIt == mmapSeparator.begin() )
			{
				// First proposition is rejected
				// TODO: warning message in compilation Box
			}

			// Remove this selection.
			if( mmapSeparator.size() > 1 )
			{
				mmapKeyDoubleSelectedProduct::iterator it = mapIt;
				++mapIt;
				delete it->second;
				mmapSeparator.erase( it );
				continue;
			}
			else
			{
				// To keep a trace of created object to well erase them when no more needed.
				m_vecObjects.push_back( mapIt->second );
			}
		}
		else
		{
			// To keep a trace of created object to well erase them when no more needed.
			m_vecObjects.push_back( mapIt->second );
			bOneValveExist = true;
		}

		++mapIt;
	}

	if( 0 == mmapSeparator.size() )
	{
		return false;
	}
	
	// Build sorting keys.
	// Remark: do not call 'BuildSortingKeys' that works with 'm_strComboXYZBelow65ID' and 'm_strCombXYZAbove50ID' variables.
	CTAPSortKey sKeyTechParam;
	CDB_StringID *psidType = (CDB_StringID *)( pclBatchSelSeparatorParams->m_pTADB->Get( (LPCTSTR)pclBatchSelSeparatorParams->m_strComboTypeID ).MP );
	CDB_StringID *psidFam = (CDB_StringID *)( pclBatchSelSeparatorParams->m_pTADB->Get( (LPCTSTR)pclBatchSelSeparatorParams->m_strComboFamilyID ).MP );
	CDB_StringID *psidConn = (CDB_StringID *)( pclBatchSelSeparatorParams->m_pTADB->Get( (LPCTSTR)pclBatchSelSeparatorParams->m_strComboConnectID ).MP );
	CDB_StringID *psidVers = (CDB_StringID *)( pclBatchSelSeparatorParams->m_pTADB->Get( (LPCTSTR)pclBatchSelSeparatorParams->m_strComboVersionID ).MP );
	
	sKeyTechParam.Init( psidType, NULL, psidFam, NULL, psidConn, psidVers, NULL, 0 );

	bool bFound = false;
	int iUserPrefLoop = 0;

	for( ; iUserPrefLoop < 4 && false == bFound; iUserPrefLoop++ )
	{
		// This loop give us the possibility to relax mask criteria one by one.
		// We will start with the most constraining key and if we don't found a product we will relax constrains one by one.
		CTAPSortKey::TAPSortKeyMask eMask = ( CTAPSortKey::TAPSortKeyMask )( CTAPSortKey::TAPSKM_All & ~CTAPSortKey::TAPSKM_PriorL 
				& ~CTAPSortKey::TAPSKM_Size & ~CTAPSortKey::TAPSKM_Bdy & ~CTAPSortKey::TAPSKM_PN );

		switch( iUserPrefLoop )
		{
			case 0:
				// No filter, we are strict.
				break;

			case 1:
				// Relax version.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Version );
				break;

			case 2:
				// Relax version and connection.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Version & ~CTAPSortKey::TAPSKM_Connect );
				break;

			case 3:
				// Relax version, connection and family.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Version & ~CTAPSortKey::TAPSKM_Connect 
						& ~CTAPSortKey::TAPSKM_Fam );
				break;
		}

		double dKeyTechParam = sKeyTechParam.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
		mmapKeyDoubleSelectedProduct::iterator ItBestBelowQNom = mmapSeparator.end();
		mmapKeyDoubleSelectedProduct::iterator ItBestAboveQNom = mmapSeparator.end();
		double dBestDeltaBelowQNom = DBL_MAX;
		double dBestDeltaAboveQNom = DBL_MAX;

		for( mapIt = mmapSeparator.begin(); mapIt != mmapSeparator.end(); ++mapIt )
		{
			CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator*>( mapIt->second->GetProductIDPtr().MP );
			
			if( NULL == pclSeparator )
			{
				continue;
			}

			bool bValveMatch = false;
			int iSize = pclSeparator->GetSizeKey();
			double dTAPKey = pclSeparator->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
			
			if( dTAPKey == dKeyTechParam )
			{
				bValveMatch = true;
			}
			
			if( false == bValveMatch )
			{
				continue;
			}

			CDB_QDpCharacteristic *pclQDpCharacteristic = pclSeparator->GetQDpCharacteristic();

			if( NULL == pclQDpCharacteristic )
			{
				continue;
			}

			// Check the product that is closest of its nominal flow.
			if( dFlow < pclQDpCharacteristic->GetQnom() && pclQDpCharacteristic->GetQnom() - dFlow < dBestDeltaBelowQNom )
			{
				dBestDeltaBelowQNom = pclQDpCharacteristic->GetQnom() - dFlow;
				ItBestBelowQNom = mapIt;
			}

			if( dFlow >= pclQDpCharacteristic->GetQnom() && dFlow < pclQDpCharacteristic->GetQmax() 
				&& dFlow - pclQDpCharacteristic->GetQnom() < dBestDeltaAboveQNom )
			{
				dBestDeltaAboveQNom = dFlow - pclQDpCharacteristic->GetQnom();
				ItBestAboveQNom = mapIt;
			}

			CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclSeparator, mapIt->second );
		}

		if( DBL_MAX != dBestDeltaBelowQNom )
		{
			bFound = true;
			ItBestBelowQNom->second->SetFlag( CSelectedBase::eBest, true );

			if( 0 != iUserPrefLoop )
			{
				ItBestBelowQNom->second->SetFlag( CSelectedBase::eValveBatchAlter, true );
			}
		}
		else if( DBL_MAX != dBestDeltaAboveQNom )
		{
			bFound = true;
			ItBestAboveQNom->second->SetFlag( CSelectedBase::eBest, true );

			if( 0 != iUserPrefLoop )
			{
				ItBestAboveQNom->second->SetFlag( CSelectedBase::eValveBatchAlter, true );
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////
//	Selected Dp controller valve container for batch mode
//////////////////////////////////////////////////////////////////////////////////
CBatchSelectDpCList::CBatchSelectDpCList()
	: CBatchSelectBaseList()
{
	m_pSelectedBatchBvList = NULL;
	Clean();
}

CBatchSelectDpCList::~CBatchSelectDpCList()
{
	if( NULL != m_pSelectedBatchBvList )
	{
		delete m_pSelectedBatchBvList;
	}
}

void CBatchSelectDpCList::Clean()
{
	CBatchSelectBaseList::Clean();

	if( NULL != m_pSelectedBatchBvList )
	{
		delete m_pSelectedBatchBvList;
	}

	m_pSelectedBatchBvList = NULL;
}

void CBatchSelectDpCList::TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList )
{
	if( NULL == pclIndividualSelectList )
	{
		ASSERT_RETURN;
	}

	CBatchSelectBaseList::TransferResultsToIndividualSelectList( pclIndividualSelectList );

	// For DpC we use the 'm_MMapKeyDecreasing' variable.
	for( auto &iter : m_MMapKeyDecreasing )
	{
		if( NULL == iter.second )
		{
			continue;
		}

		if( CSelectedBase::SBT_Valve == iter.second->GetSelectedBaseType() )
		{
			CSelectedValve *pclSelectedValve = new CSelectedValve();

			if( NULL == pclSelectedValve )
			{
				continue;
			}

			pclSelectedValve->CopyFrom( iter.second );
			pclIndividualSelectList->m_MMapKeyDecreasing.insert( ProductPair( iter.first, pclSelectedValve ) );
		}
	}
}

int CBatchSelectDpCList::SelectDpC( CBatchSelDpCParams *pclBatchSelDpCParams, double dFlow, double dDpBranch, double dKvs )
{
	if( NULL == pclBatchSelDpCParams || NULL == pclBatchSelDpCParams->m_pTADS || NULL == pclBatchSelDpCParams->m_pTADS->GetpTechParams()
			|| NULL == pclBatchSelDpCParams->m_pTADB )
	{
		ASSERTA_RETURN( BatchReturn::BR_BatchSelParamsNULL );
	}

	if( dFlow <= 0.0 )
	{
		return BatchReturn::BR_BadArguments;
	}

	// Only best pipe requested.
	m_clSelectPipeList.SelectPipes(pclBatchSelDpCParams, dFlow );
	m_pclBestPipe = m_clSelectPipeList.GetBestPipe();
	
	if( NULL == m_pclBestPipe && 1 == m_clSelectPipeList.GetPipeNumbers() )
	{
		m_pclBestPipe = m_clSelectPipeList.GetpPipeMap()->begin()->second.GetpPipe();
	}

	if( NULL == m_pclBestPipe )
	{
		return BatchReturn::BR_NoPipeFound;
	}

	// Save variables here becase 'SelectPipes' calls 'Clean'.
	m_pclBestProduct = NULL;

	bool bAtLeastOneBestStrict = false;
	bool bAtLeastOneBestAlter = false;
	CSelectedBase *pclBestProductStrict = NULL;
	CSelectedBase *pclBestProductAlter = NULL;

	if( true == _SelectBestDpC( pclBatchSelDpCParams, dFlow, dDpBranch, dKvs ) )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedBase *pclSelectedProduct = m_cIter->second;
			ASSERT( NULL != pclSelectedProduct );

			if( NULL == pclSelectedProduct )
			{
				continue;
			}

			CSelectedValve *pclSelectedDpC = dynamic_cast<CSelectedValve *>( pclSelectedProduct );
			ASSERT( NULL != pclSelectedDpC );

			if( NULL == pclSelectedDpC )
			{
				continue;
			}

			if( true == pclSelectedDpC->GetFlag( CSelectedBase::eBest ) )
			{
				if( false == pclSelectedDpC->GetFlag( CSelectedBase::eValveBatchAlter ) )
				{
					bAtLeastOneBestStrict = true;
					pclBestProductStrict = pclSelectedDpC;
				}
				else
				{
					bAtLeastOneBestAlter = true;
					pclBestProductAlter = pclSelectedDpC;
				}
			}

			CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedDpC->GetpData() );
			ASSERT( NULL != pTAP );

			if( NULL == pTAP )
			{
				continue;
			}
		
			// Transfer valve into the final list with good priority key.
			CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pTAP, pclSelectedDpC );
		}
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	// Set the return value.
	int iReturn;

	if( true == bAtLeastOneBestStrict && NULL != pclBestProductStrict )
	{
		m_pclBestProduct = pclBestProductStrict;
		iReturn = BatchReturn::BR_FoundOneBest;
	}
	else if( true == bAtLeastOneBestAlter && NULL != pclBestProductAlter )
	{
		m_pclBestProduct = pclBestProductAlter;
		iReturn = BatchReturn::BR_FoundOneBest | BatchReturn::BR_FoundAlernative;
	}
	else
	{
		iReturn = ( m_MMapKeyDecreasing.size() > 0 ) ? BatchReturn::BR_FoundButNotBest : BatchReturn::BR_NotFound;
	}

	if( NULL != m_pclBestProduct )
	{
		_SelectBestBv( pclBatchSelDpCParams, dFlow, dDpBranch, dKvs );
	}

	return iReturn;
}

bool CBatchSelectDpCList::_SelectBestDpC( CBatchSelDpCParams *pclBatchSelDpCParams, double dFlow, double dDpBranch, double dKvs )
{
	if( NULL == m_pclBestPipe )
	{
		return false;
	}

	double dDpToStab = _GetDpToStab( pclBatchSelDpCParams, dFlow, dDpBranch, dKvs, -1 );

	if( dDpToStab < 0.0 )
	{
		return false;
	}

	int iHighestSize = 0;
	int iLowestSize = 0;
	
	if( false == GetPipeSizeShift( pclBatchSelDpCParams, iHighestSize, iLowestSize ) )
	{
		return false;
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclBatchSelDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN50 = pclTableDN->GetSize( _T("DN_50") );

	// Establish DpC list based on DpCType and the best technical choice
	CRankEx List;
	pclBatchSelDpCParams->m_pTADB->GetDpCList( &List, pclBatchSelDpCParams->m_eDpCLoc, L"", L"", L"", L"", L"", L"", pclBatchSelDpCParams->m_eFilterSelection );

	CData *pData = NULL;
	_string str;
	
	m_PreselMMapKeyDecreasing.clear();

	// To only clean the 'm_vecObjects' variable if it already contains all the 'CSelectedBase' objects.
	CleanVecObjects();

	std::multimap<int, CDB_TAProduct*> mapTempDpC;

	// Extract valves according SizeShift, keep at least one product size.
	for( BOOL bContinue = List.GetFirstT<CData *>( str, pData ); TRUE == bContinue; bContinue = List.GetNextT<CData *>( str, pData ) )
	{
		CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( pData );

		if( NULL == pDpC )
		{
			continue;
		}

		CDB_DpCCharacteristic *pDpCChar = pDpC->GetDpCCharacteristic();

		if( NULL == pDpCChar )
		{
			continue;
		}

		double dQmin = pDpCChar->GetQmin( pclBatchSelDpCParams->m_WC.GetDens(), dDpToStab );
		double dQmax = pDpCChar->GetQmax( pclBatchSelDpCParams->m_WC.GetDens() );

		if( dFlow < dQmin || dFlow > dQmax )
		{
			continue;
		}

		// Verify DpL min/max.
		double dDplmin = pDpCChar->GetDplmin();
		double dDplmax = pDpCChar->GetDplmax();

		if( -1.0 == dDplmin || -1.0 == dDplmax )
		{
			continue;
		}

		// Add product.
		int iProdSizeKey = pDpC->GetSizeKey();
		mapTempDpC.insert( std::make_pair( iProdSizeKey, pDpC ) );
	}

	if( 0 == mapTempDpC.size() )
	{
		return false;
	}

	std::multimap<int, CDB_TAProduct *>::iterator itLow = mapTempDpC.find( iLowestSize );
	std::multimap<int, CDB_TAProduct *>::iterator itUp = mapTempDpC.find( iHighestSize );

	if( mapTempDpC.end() == itLow && mapTempDpC.end() == itUp )
	{
		// Valve with correct size not found, keep smallest or highest valve
		std::multimap<int, CDB_TAProduct*>::iterator itMapDpC = mapTempDpC.begin();

		if( itMapDpC->first > iHighestSize )
		{
			// All valves size are above the highest size, keep smallest valve size
			itLow = mapTempDpC.upper_bound( itMapDpC->first );
			mapTempDpC.erase( itLow, mapTempDpC.end() );
		}

		std::multimap<int, CDB_TAProduct*>::reverse_iterator rItmapDpC = mapTempDpC.rbegin();

		if( rItmapDpC->first < iLowestSize )
		{
			// All valves size are below the lowest size, keep highest valve size
			itUp = mapTempDpC.lower_bound( rItmapDpC->first );
			mapTempDpC.erase( mapTempDpC.begin(), mapTempDpC.end() );
		}
	}
	else
	{
		if( mapTempDpC.end() != itLow && mapTempDpC.begin() != itLow )
		{
			mapTempDpC.erase( mapTempDpC.begin(), itLow );
		}

		itUp = mapTempDpC.upper_bound( iHighestSize );

		if( mapTempDpC.end() != itUp )
		{
			mapTempDpC.erase( itUp, mapTempDpC.end() );
		}
	}

	if( 0 == mapTempDpC.size() )
	{
		return false;
	}

	// To facilitate use in the 'CDlgBatchSelDpC' class.
	for( auto &iter : mapTempDpC )
	{
		CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( iter.second );

		double dDpMin = -1.0;
		CDB_DpCCharacteristic *pDpCChar = pDpC->GetDpCCharacteristic();

		if( dDpToStab <= pDpCChar->GetDplmax() )
		{
			pDpCChar->DpCSolve( dFlow, &dDpMin, pclBatchSelDpCParams->m_WC.GetDens() );
		}

		CSelectedValve *pclSelectedDpC = new CSelectedValve();

		if( NULL == pclSelectedDpC )
		{
			ASSERT( 0 );
			continue;
		}

		pclSelectedDpC->SetProductIDPtr( pDpC->GetIDPtr() );
		pclSelectedDpC->SetFlag( CSelectedBase::Flags::eNotPriority, false );
		pclSelectedDpC->SetDpMin( dDpMin );

		m_mmapProductList.insert( std::make_pair( iter.first, pclSelectedDpC ) );

		// To keep a trace of created object to well erase them when no more needed.
		m_vecObjects.push_back( pclSelectedDpC );
	}


	// Full DpC list, start to order it.
	// Kvm pivot is 80% for all DpC.
	double dBestDpCKvmPivot = 0.80;
	CDS_TechnicalParameter *pclTechParams = pclBatchSelDpCParams->m_pTADS->GetpTechParams();

	// We don't use here the 'ProductMMapKeyDecreasing' map because the key is decreasing. Here we need to sort the
	// map with the lower Dp min to the bigger.
	std::multimap<double, CSelectedBase *> mapDpC;
	std::multimap<double, CSelectedBase *>::iterator It;
	std::multimap<double, CSelectedBase *>::iterator ItBest;

	// find DpC with BestDpCKvmPivot * Kvm close to the Kv(Havail; qd).
	for( auto &iter : m_mmapProductList )
	{
		CSelectedValve *pclSelectedValve = dynamic_cast<CSelectedValve *>( iter.second );

		if( NULL == pclSelectedValve )
		{
			continue;
		}

		CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( pclSelectedValve->GetProductIDPtr().MP );

		if( NULL == pDpC )
		{
			continue;
		}

		CDB_DpCCharacteristic *pDpCChar = pDpC->GetDpCCharacteristic();

		if( NULL == pDpCChar )
		{
			continue;
		}

		// HYS-882: we insert even if the Dp min is to low. It's to be able to show more choice when user
		// double-clicks on a result to go in the individual selection mode.
		double dKey = pclSelectedValve->GetDpMin();
		mapDpC.insert( std::make_pair( dKey, pclSelectedValve ) );

		// We select DpC with a Dpmin above technical parameter 'DpcMinDp'.
		if( pclSelectedValve->GetDpMin() >= pclTechParams->GetDpCMinDp() )
		{
			double dDpStab = _GetDpToStab( pclBatchSelDpCParams, dFlow, dDpBranch, dKvs, (int)eMvLoc::MvLocPrimary );

			if( dDpStab > pDpC->GetDplmax() )
			{
				continue;
			}

			if( dDpBranch > 0.0 && (int)eMvLoc::MvLocPrimary == pclBatchSelDpCParams->m_eMvLoc )
			{
				if( dDpStab < pDpC->GetDplmin() )
				{
					pclSelectedValve->SetFlag( CSelectedBase::eValveBatchDpCForceMvInSec, true );
				}
			}
		}
	}

	mapTempDpC.clear();

	// Build sorting keys.
	CTAPSortKey sKeyTechParamBlw65;
	CTAPSortKey sKeyTechParamAbv50;
	BuildSortingKeys( pclBatchSelDpCParams, &sKeyTechParamBlw65 , &sKeyTechParamAbv50 );

	bool bFound = false;
	double dKeyCurrentTAP = 0;
	int iUserPrefLoop = 0;
	for( ; iUserPrefLoop < 7; iUserPrefLoop++ )
	{
		// This loop give us the possibility to relax mask criteria one by one
		// we will start with the most constraining key and if we don't found a product we will relax constrains one by one
		CTAPSortKey::TAPSortKeyMask eMask = ( CTAPSortKey::TAPSortKeyMask )( CTAPSortKey::TAPSKM_All  & ~CTAPSortKey::TAPSKM_PriorL & ~CTAPSortKey::TAPSKM_Size
				& ~CTAPSortKey::TAPSKM_Fam );

		m_PreselMMapKeyDecreasing.clear();

		switch( iUserPrefLoop )
		{
			case 0:
				// All
				break;

			case 1:
				// Relax PN.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
				break;

			case 2:
				// Relax PN and connection.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN  & ~CTAPSortKey::TAPSKM_Connect );
				break;

			case 3:
				// Relax PN, connection and type.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN  & ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Type );
				break;

			case 4:
				// Relax PN, connection, type and body.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN  & ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Type
						& ~CTAPSortKey::TAPSKM_Bdy );
				break;

			case 5:
				// Relax PN, connection, type, body and version.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN  & ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Type
						& ~CTAPSortKey::TAPSKM_Bdy & ~CTAPSortKey::TAPSKM_Version );
				break;

			case 6:
				// Relax PN, connection, type, body, version and family.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN  & ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Type
						& ~CTAPSortKey::TAPSKM_Bdy & ~CTAPSortKey::TAPSKM_Version & ~CTAPSortKey::TAPSKM_Fam );
				break;
		}

		double dKeyTechParamBlw65 = sKeyTechParamBlw65.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
		double dKeyTechParamAbv50 = sKeyTechParamAbv50.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );

		for( It = mapDpC.begin(); It != mapDpC.end(); ++It )
		{
			if( NULL == It->second || NULL == It->second->GetProductIDPtr().MP )
			{
				continue;
			}

			CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( It->second->GetProductIDPtr().MP );

			if( NULL == pclDpController )
			{
				continue;
			}

			int iSize = pclDpController->GetSizeKey();
			double dTAPKey = pclDpController->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
			bool bValveMatched = false;

			// Selected valve is not located on the same side of iDN50 border.
			// Use defined technical choice
			if( iSize <= iDN50 )
			{
				if( dTAPKey == dKeyTechParamBlw65 )
				{
					bValveMatched = true;
				}
			}
			else
			{
				// Size > DN50.
				if( dTAPKey == dKeyTechParamAbv50 )
				{
					bValveMatched = true;
				}
			}

			if( true == bValveMatched )
			{
				CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclDpController, It->second );

				// HYS-882: Because we accept above to take also valves that have Dp min below the minimum set in the
				// technical parameters (To be able to show more choice when user double-clicks on a result to go in the
				// individual selection mode), we here filter to not set the best flag on these ones.
				// This is why the second condition.
				CSelectedValve *pclSelectedValve = (CSelectedValve *)It->second;

				if( false == bFound && pclSelectedValve->GetDpMin() >= pclTechParams->GetDpCMinDp() )
				{
					ItBest = It;
					bFound = true;
				}
												
				// Continue loop to save all valves in 'm_PreselMMap' that belongs to the same criterion.
			}
		}

		if( true == bFound )
		{
			// Abort loop and by this way we preserve the iterator.
			break;
		}
	}

	// If best valve found...
	if( true == bFound )
	{
		ItBest->second->SetFlag( CSelectedBase::eBest, true );

		if( 0 != iUserPrefLoop )
		{
			ItBest->second->SetFlag( CSelectedBase::eValveBatchAlter, true );
		}

		double dReqDpMv = pclBatchSelDpCParams->m_pTADS->GetpTechParams()->GetValvMinDp();

		if( eMvLoc::MvLocSecondary == pclBatchSelDpCParams->m_eMvLoc 
				|| true == ItBest->second->GetFlag( CSelectedBase::eValveBatchDpCForceMvInSec ) )
		{
			CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( ( (CSelectedValve*)ItBest->second )->GetProductIDPtr().MP );

			if( NULL == pclDpController )
			{
				ASSERTA_RETURN( false );
			}

			dReqDpMv = max( dReqDpMv, pclDpController->GetDplmin() - _GetDpToStab( pclBatchSelDpCParams, dFlow, dDpBranch, dKvs, (int)eMvLoc::MvLocPrimary ) );
			( (CSelectedValve*)ItBest->second )->SetRequiredDpMv( dReqDpMv );
		}
	}
	else
	{
		m_PreselMMapKeyDecreasing.clear();
	}

	return true;
}

bool CBatchSelectDpCList::_SelectBestBv( CBatchSelDpCParams *pclBatchSelDpCParams, double dFlow, double dDpBranch, double dKvs )
{
	CDB_DpController *pclDpController = dynamic_cast<CDB_DpController *>( m_pclBestProduct->GetProductIDPtr().MP );

	if( NULL == pclDpController )
	{
		return false;
	}

	if( NULL != m_pSelectedBatchBvList )
	{
		delete m_pSelectedBatchBvList;
		m_pSelectedBatchBvList = NULL;
	}
	
	m_pSelectedBatchBvList = new CBatchSelectBvList();

	if( NULL == m_pSelectedBatchBvList || NULL == m_pSelectedBatchBvList->GetSelectPipeList() )
	{
		return false;
	}

	m_pSelectedBatchBvList->GetSelectPipeList()->SelectPipes( &pclBatchSelDpCParams->m_clBatchSelBVParams, dFlow );

	// Pre-select regulating valve.
	CRankEx BvList;
	CTable *pTab = (CTable*)( pclBatchSelDpCParams->m_clBatchSelBVParams.m_pTADB->Get( _T("MEASVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	int iDNMin = 0;
	int iDNMax = INT_MAX;
	CDS_TechnicalParameter *pclTechParams = pclBatchSelDpCParams->m_clBatchSelBVParams.m_pTADS->GetpTechParams();

	// If the user want a regulating valve selection using same Dp controller size, remove unneeded valve from BvList
	if( true == ( pclTechParams->GetDpCMvWithSameSizeOnly() != 0 ) ? true : false )
	{
		iDNMin = pclDpController->GetSizeKey();
		iDNMax = iDNMin;
	}
	
	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_StringID *pStrID = (CDB_StringID*)( IDPtr.MP );
		
		// Reject STAM.
		CString strFamilyID = pStrID->GetIDstr();
		
		if( "FAM_STAM" == strFamilyID )
		{
			continue;
		}
		
		CDB_StringID *pStrFamilyID = (CDB_StringID*)( pclBatchSelDpCParams->m_clBatchSelBVParams.m_pTADB->Get( pStrID->GetIDstr() ).MP );
		CString str = pStrID->GetString();
		
		pclBatchSelDpCParams->m_clBatchSelBVParams.m_pTADB->GetBVList( &BvList, pStrFamilyID->GetIDstr2(), pStrID->GetIDstr(), _T(""), _T(""), (LPCTSTR)str, 
				pclBatchSelDpCParams->m_clBatchSelBVParams.m_eFilterSelection, iDNMin, iDNMax );
	}

	m_pSelectedBatchBvList->SelectBv( &pclBatchSelDpCParams->m_clBatchSelBVParams, dFlow, ( (CSelectedValve*)m_pclBestProduct )->GetRequiredDpMv(), &BvList );

	return true;
}

double CBatchSelectDpCList::_GetDpToStab( CBatchSelDpCParams *pclBatchSelDpCParams, double dFlow, double dDpBranch, double dKvs, int iForceMvLoc )
{
	double dDp = 0.0;

	if( eDpStab::DpStabOnBranch == pclBatchSelDpCParams->m_eDpStab )
	{
		dDp = dDpBranch;
	}	
	else
	{
		dDp = CalcDp( dFlow, dKvs, pclBatchSelDpCParams->m_WC.GetDens() );
	}

	// If measuring valve is on secondary...
	if( ( -1 == iForceMvLoc && eMvLoc::MvLocSecondary == pclBatchSelDpCParams->m_eMvLoc ) || eMvLoc::MvLocSecondary == iForceMvLoc )
	{
		dDp += pclBatchSelDpCParams->m_pTADS->GetpTechParams()->GetValvMinDp();
	}

	return dDp;
}

//////////////////////////////////////////////////////////////////////////////////
//	Selected smart control valve for batch mode
//////////////////////////////////////////////////////////////////////////////////
CBatchSelectSmartControlValveList::CBatchSelectSmartControlValveList()
	: CBatchSelectBaseList()
{
	Clean();
}

void CBatchSelectSmartControlValveList::TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList )
{
	if( NULL == pclIndividualSelectList )
	{
		ASSERT_RETURN;
	}

	CBatchSelectBaseList::TransferResultsToIndividualSelectList( pclIndividualSelectList );

	// For smart control valve we use the 'm_MMapKeyDecreasing' variable.
	for( auto &iter : m_MMapKeyDecreasing )
	{
		if( NULL == iter.second )
		{
			continue;
		}

		if( CSelectedBase::SBT_Valve == iter.second->GetSelectedBaseType() )
		{
			CSelectedValve *pclSelectedSmartControlValve = new CSelectedValve();

			if( NULL == pclSelectedSmartControlValve )
			{
				continue;
			}

			pclSelectedSmartControlValve->CopyFrom( iter.second );
			pclIndividualSelectList->m_MMapKeyDecreasing.insert( ProductPair( iter.first, pclSelectedSmartControlValve ) );
		}
	}
}

int CBatchSelectSmartControlValveList::SelectSmartControlValve( CBatchSelSmartControlValveParams *pclBatchSelSmartControlValveParams, double dFlow )
{
	if( NULL == pclBatchSelSmartControlValveParams || NULL == pclBatchSelSmartControlValveParams->m_pTADS || NULL == pclBatchSelSmartControlValveParams->m_pTADS->GetpTechParams()
			|| NULL == pclBatchSelSmartControlValveParams->m_pTADB )
	{
		ASSERTA_RETURN( BatchReturn::BR_BatchSelParamsNULL );
	}

	if( dFlow <= 0.0 )
	{
		return BatchReturn::BR_BadArguments;
	}

	// Only best pipe requested.
	m_clSelectPipeList.SelectPipes( pclBatchSelSmartControlValveParams, dFlow );
	m_pclBestPipe = m_clSelectPipeList.GetBestPipe();

	if( NULL == m_pclBestPipe && 1 == m_clSelectPipeList.GetPipeNumbers() )
	{
		m_pclBestPipe = m_clSelectPipeList.GetpPipeMap()->begin()->second.GetpPipe();
	}
	
	if( NULL == m_pclBestPipe )
	{
		return BatchReturn::BR_NoPipeFound;
	}

	bool bAtLeastOneBestStrict = false;
	bool bAtLeastOneBestAlter = false;
	CSelectedBase *pclBestProductStrict = NULL;
	CSelectedBase *pclBestProductAlter = NULL;

	if( true == _SelectSmartControlValve( pclBatchSelSmartControlValveParams, dFlow ) )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedValve *pSelTap = (CSelectedValve *)m_cIter->second;
			ASSERT( NULL != pSelTap );

			if( NULL == pSelTap )
			{
				continue;
			}

			CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( pSelTap->GetProductIDPtr().MP );
			ASSERT( NULL != pclSmartControlValve );
			
			if( NULL == pclSmartControlValve )
			{
				continue;
			}

			if( true == pSelTap->GetFlag( CSelectedBase::eBest ) )
			{
				if( false == pSelTap->GetFlag( CSelectedBase::eValveBatchAlter ) )
				{
					bAtLeastOneBestStrict = true;
					pclBestProductStrict = pSelTap;
				}
				else
				{
					bAtLeastOneBestAlter = true;
					pclBestProductAlter = pSelTap;
				}
			}

			CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pclSmartControlValve, pSelTap );
		}
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	// Set the return value.
	int iReturn;

	if( true == bAtLeastOneBestStrict && NULL != pclBestProductStrict )
	{
		m_pclBestProduct = pclBestProductStrict;
		iReturn = BatchReturn::BR_FoundOneBest;
	}
	else if( true == bAtLeastOneBestAlter && NULL != pclBestProductAlter )
	{
		m_pclBestProduct = pclBestProductAlter;
		iReturn = BatchReturn::BR_FoundOneBest | BatchReturn::BR_FoundAlernative;
	}
	else
	{
		iReturn = ( m_MMapKeyDecreasing.size() > 0 ) ? BatchReturn::BR_FoundButNotBest : BatchReturn::BR_NotFound;
	}

	return iReturn;
}

void CBatchSelectSmartControlValveList::Clean()
{
	m_bBestFound = false;
	m_bSizeShiftProblem = false;
	CBatchSelectBaseList::Clean();
}

bool CBatchSelectSmartControlValveList::_SelectSmartControlValve( CBatchSelSmartControlValveParams *pclBatchSelSmartControlValveParams, double dFlow )
{
	// !!! REMARK: for the moment, it's exactly the same code as for the individual selection.
	
	m_bBestFound = false;

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return false;
	}

	int iHighestSize;
	int iLowestSize;

	if( false == GetPipeSizeShift( (CBatchSelectionParameters*)pclBatchSelSmartControlValveParams, iHighestSize, iLowestSize ) )
	{
		return false;
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclBatchSelSmartControlValveParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( 0 );
	}

	int iDN50 = pclTableDN->GetSize( _T("DN_50") );

	// Take all available products of the same type.
	CRankEx clSmartControlValveList;

	pclBatchSelSmartControlValveParams->m_pTADB->GetSmartControlValveList( &clSmartControlValveList, _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), pclBatchSelSmartControlValveParams->m_eFilterSelection );

	if( 0 == clSmartControlValveList.GetCount() )
	{
		return false;
	}

	double dDpMaxForBestSuggestion = pclBatchSelSmartControlValveParams->m_pTADS->GetpTechParams()->GetSmartValveDpMaxForBestSuggestion();
	double dRho = pclBatchSelSmartControlValveParams->m_WC.GetDens();
	bool bAtLeastOneFit = false;
	m_bSizeShiftProblem = false;
	mmapKeyDoubleSelectedProduct mmapSmartControlValve;
	_string str;
	LPARAM lpParam;
	
	for( BOOL bContinue = clSmartControlValveList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = clSmartControlValveList.GetNext( str, lpParam ) )
	{
		CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( (CData *)lpParam );
				
		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		if( false == pclSmartControlValve->IsSelectable( true ) )
		{
			continue;
		}

		CDB_SmartValveCharacteristic *pclSmartValveCharacteristic = pclSmartControlValve->GetSmartValveCharacteristic();

		if( NULL == pclSmartValveCharacteristic )
		{
			clSmartControlValveList.Delete();
			continue;
		}

		if( dFlow < pclSmartValveCharacteristic->GetMinAdjustableFlow() || dFlow > pclSmartValveCharacteristic->GetQnom() )
		{
			clSmartControlValveList.Delete();
			continue;
		}

		// Create a new entry in pre-selection linked list.
		CSelectedValve *pSelSmartControlValve = new CSelectedValve();
		ASSERT( NULL != pSelSmartControlValve );

		if( NULL == pSelSmartControlValve )
		{
			continue;
		}

		double dDp = CalcDp( dFlow, pclSmartControlValve->GetKvs(), pclBatchSelSmartControlValveParams->m_WC.GetDens() );
		pSelSmartControlValve->SetDp( dDp );

		pSelSmartControlValve->SetProductIDPtr( pclSmartControlValve->GetIDPtr() );
		pSelSmartControlValve->SetFlag( CSelectedBase::Flags::eNotPriority, false );

		double d64Key = pclSmartControlValve->GetSortingKey();
		mmapSmartControlValve.insert( std::make_pair( d64Key, pSelSmartControlValve ) );
	}

	bool bOneSmartControlValveExist = false;
	int iSmallestValve = 0;
	int iHighestValve = 99999;
	
	// Key is the sorting key.
	mmapKeyDoubleSelectedProduct::iterator mapIt;

	// Adapt 'dLowestSize' and 'iHigestSize' to include at least one smart control valve.
	for( mapIt = mmapSmartControlValve.begin(); mapIt != mmapSmartControlValve.end() && bOneSmartControlValveExist == false; ++mapIt )
	{
		CDB_TAProduct *pclTAProduct = (CDB_TAProduct*)( mapIt->second->GetProductIDPtr().MP );

		if( NULL == pclTAProduct )
		{
			continue;
		}

		int iProdSizeKey = pclTAProduct->GetSizeKey();
		
		if( iProdSizeKey < iHighestValve && iProdSizeKey > iHighestSize )
		{
			iHighestValve = iProdSizeKey;
		}

		if( iProdSizeKey > iSmallestValve && iProdSizeKey < iLowestSize )
		{
			iSmallestValve = iProdSizeKey;
		}
		
		if( iProdSizeKey >= iLowestSize && iProdSizeKey <= iHighestSize )
		{
			bOneSmartControlValveExist = true;
		}
	}
	
	if( false == bOneSmartControlValveExist )
	{
		if( iSmallestValve < iLowestSize )
		{
			iLowestSize = iSmallestValve;
		}
		
		if( iHighestValve > iHighestSize )
		{
			iHighestSize = iHighestValve;
		}
	}

	for( mapIt = mmapSmartControlValve.begin(); mapIt != mmapSmartControlValve.end(); )
	{
		CDB_SmartControlValve *pclSmartControlValve = (CDB_SmartControlValve *)( mapIt->second->GetProductIDPtr().MP );

		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		int iProdSizeKey = pclSmartControlValve->GetSizeKey();
		
		if( !( iProdSizeKey >= iLowestSize && iProdSizeKey <= iHighestSize ) )
		{
			if( mapIt == mmapSmartControlValve.begin() )
			{
				// First proposition is rejected
				// TODO: warning message in compilation Box
			}

			// Remove this selection.
			if( mmapSmartControlValve.size() > 1 )
			{
				mmapKeyDoubleSelectedProduct::iterator it = mapIt;
				++mapIt;
				delete it->second;
				mmapSmartControlValve.erase( it );
				continue;
			}
			else
			{
				// To keep a trace of created object to well erase them when no more needed.
				m_vecObjects.push_back( mapIt->second );
			}
		}
		else
		{
			// To keep a trace of created object to well erase them when no more needed.
			m_vecObjects.push_back( mapIt->second );
			bOneSmartControlValveExist = true;
		}

		++mapIt;
	}

	if( 0 == mmapSmartControlValve.size() )
	{
		return false;
	}
	
	// Build sorting keys.
	CTAPSortKey sKeyTechParamBlw65;
	CTAPSortKey sKeyTechParamAbv50;
	BuildSortingKeys( pclBatchSelSmartControlValveParams, &sKeyTechParamBlw65 , &sKeyTechParamAbv50 );

	double dKeyCurrentTAP = 0;
	bool bFound = false;

	for( int iUserPrefLoop = 0; iUserPrefLoop < 4 && false == bFound; iUserPrefLoop++ )
	{
		// This loop give us the possibility to relax mask criteria one by one.
		// We will start with the most constraining key and if we don't found a product we will relax constrains one by one.
		CTAPSortKey::TAPSortKeyMask eMask = ( CTAPSortKey::TAPSortKeyMask )( CTAPSortKey::TAPSKM_All & ~CTAPSortKey::TAPSKM_PriorL 
				& ~CTAPSortKey::TAPSKM_Size & ~CTAPSortKey::TAPSKM_Type & ~CTAPSortKey::TAPSKM_Fam & ~CTAPSortKey::TAPSKM_Version );

		switch( iUserPrefLoop )
		{
			case 0:
				// No filter, we are strict.
				break;

			case 1:
				// Relax PN.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
				break;

			case 2:
				// Relax PN and connection.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Connect );
				break;

			case 3:
				// Relax PN, connection and body material.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Connect 
						& ~CTAPSortKey::TAPSKM_Bdy );
				break;
		}

		double dKeyTechParamBlw65 = sKeyTechParamBlw65.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
		double dKeyTechParamAbv50 = sKeyTechParamAbv50.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );

		mmapKeyDoubleSelectedProduct::iterator ItBestBelowQNom = mmapSmartControlValve.end();
		double dBestDeltaBelowQNom = DBL_MAX;

		for( mapIt = mmapSmartControlValve.begin(); mapIt != mmapSmartControlValve.end(); ++mapIt )
		{
			CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( mapIt->second->GetProductIDPtr().MP );
			
			if( NULL == pclSmartControlValve )
			{
				continue;
			}

			bool bValveMatch = false;
			int iSize = pclSmartControlValve->GetSizeKey();
			double dTAPKey = pclSmartControlValve->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
			
			// Selected valve is not located on the same side of iDN50 border.
			// Use defined technical choice.
			if( iSize <= iDN50 )
			{
				if( dTAPKey == dKeyTechParamBlw65 )
				{
					bValveMatch = true;
				}
			}
			else
			{
				// Size > DN50
				if( dTAPKey == dKeyTechParamAbv50 )
				{
					bValveMatch = true;
				}
			}
			
			if( false == bValveMatch )
			{
				continue;
			}

			CDB_SmartValveCharacteristic *pclSmartValveCharacteristic = pclSmartControlValve->GetSmartValveCharacteristic();

			if( NULL == pclSmartValveCharacteristic )
			{
				continue;
			}

			// Check the product that is closest of its nominal flow.
			if( pclSmartValveCharacteristic->GetQnom() - dFlow < dBestDeltaBelowQNom )
			{
				if( pclSmartControlValve->GetDpMin( dFlow, dRho ) <= dDpMaxForBestSuggestion )
				{
					dBestDeltaBelowQNom = pclSmartValveCharacteristic->GetQnom() - dFlow;
					ItBestBelowQNom = mapIt;
				}
			}

			CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclSmartControlValve, mapIt->second );
		}

		if( DBL_MAX != dBestDeltaBelowQNom )
		{
			bFound = true;
			ItBestBelowQNom->second->SetFlag( CSelectedBase::eBest, true );

			if( 0 != iUserPrefLoop )
			{
				ItBestBelowQNom->second->SetFlag( CSelectedBase::eValveBatchAlter, true );
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////
//	Selected smart differential pressure controller for batch mode
//////////////////////////////////////////////////////////////////////////////////
CBatchSelectSmartDpCList::CBatchSelectSmartDpCList()
	: CBatchSelectBaseList()
{
	Clean();
}

void CBatchSelectSmartDpCList::TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList )
{
	if( NULL == pclIndividualSelectList )
	{
		ASSERT_RETURN;
	}

	CBatchSelectBaseList::TransferResultsToIndividualSelectList( pclIndividualSelectList );

	// For smart differential pressure controller we use the 'm_MMapKeyDecreasing' variable.
	for( auto &iter : m_MMapKeyDecreasing )
	{
		if( NULL == iter.second )
		{
			continue;
		}

		if( CSelectedBase::SBT_Valve == iter.second->GetSelectedBaseType() )
		{
			CSelectedValve *pclSelectedSmartDpC = new CSelectedValve();

			if( NULL == pclSelectedSmartDpC )
			{
				continue;
			}

			pclSelectedSmartDpC->CopyFrom( iter.second );
			pclIndividualSelectList->m_MMapKeyDecreasing.insert( ProductPair( iter.first, pclSelectedSmartDpC ) );
		}
	}
}

int CBatchSelectSmartDpCList::SelectSmartDpC( CBatchSelSmartDpCParams *pclBatchSelSmartDpCParams, double dFlow )
{
	m_pclBestDpSensor = NULL;

	if( NULL == pclBatchSelSmartDpCParams )
	{
		ASSERTA_RETURN( BatchReturn::BR_BatchSelParamsNULL );
	}

	if( dFlow <= 0.0 )
	{
		return BatchReturn::BR_BadArguments;
	}

	// Only best pipe requested.
	m_clSelectPipeList.SelectPipes( pclBatchSelSmartDpCParams, dFlow );
	m_pclBestPipe = m_clSelectPipeList.GetBestPipe();

	if( NULL == m_pclBestPipe && 1 == m_clSelectPipeList.GetPipeNumbers() )
	{
		m_pclBestPipe = m_clSelectPipeList.GetpPipeMap()->begin()->second.GetpPipe();
	}
	
	if( NULL == m_pclBestPipe )
	{
		return BatchReturn::BR_NoPipeFound;
	}

	bool bAtLeastOneBestStrict = false;
	bool bAtLeastOneBestAlter = false;
	CSelectedBase *pclBestProductStrict = NULL;
	CSelectedBase *pclBestProductAlter = NULL;

	if( true == _SelectSmartDpC( pclBatchSelSmartDpCParams, dFlow ) )
	{
		for( m_cIter = m_PreselMMapKeyDecreasing.begin(); m_cIter != m_PreselMMapKeyDecreasing.end(); m_cIter++ )
		{
			CSelectedValve *pSelTap = (CSelectedValve *)m_cIter->second;

			if( NULL == pSelTap )
			{
				ASSERT_CONTINUE;
			}

			CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( pSelTap->GetProductIDPtr().MP );
			
			if( NULL == pclSmartDpC )
			{
				ASSERT_CONTINUE;
			}

			if( true == pSelTap->GetFlag( CSelectedBase::eBest ) )
			{
				if( false == pSelTap->GetFlag( CSelectedBase::eValveBatchAlter ) )
				{
					bAtLeastOneBestStrict = true;
					pclBestProductStrict = pSelTap;
				}
				else
				{
					bAtLeastOneBestAlter = true;
					pclBestProductAlter = pSelTap;
				}
			}

			CreateKeyAndInsertInMap( &m_MMapKeyDecreasing, pclSmartDpC, pSelTap );
		}
	}

	// Clean allocated memory.
	m_PreselMMapKeyDecreasing.clear();

	// Set the return value.
	int iReturn;

	if( true == bAtLeastOneBestStrict && NULL != pclBestProductStrict )
	{
		m_pclBestProduct = pclBestProductStrict;
		iReturn = BatchReturn::BR_FoundOneBest;
	}
	else if( true == bAtLeastOneBestAlter && NULL != pclBestProductAlter )
	{
		m_pclBestProduct = pclBestProductAlter;
		iReturn = BatchReturn::BR_FoundOneBest | BatchReturn::BR_FoundAlernative;
	}
	else
	{
		iReturn = ( m_MMapKeyDecreasing.size() > 0 ) ? BatchReturn::BR_FoundButNotBest : BatchReturn::BR_NotFound;
	}

	return iReturn;
}

CDB_DpSensor *CBatchSelectSmartDpCList::GetBestDpSensor( CBatchSelSmartDpCParams *pclBatchSelSmartDpCParams, double dDpBranch )
{
	CDB_DpSensor *pclBestDpSensor = NULL;

	if( NULL != (CDB_DpSensor*)m_pclBestDpSensor )
	{
		pclBestDpSensor = (CDB_DpSensor*)m_pclBestDpSensor;
	}
	else
	{
		pclBestDpSensor = _SelectBestDpSensor( pclBatchSelSmartDpCParams, dDpBranch, false );
	}

	return pclBestDpSensor;
}

CDB_Product *CBatchSelectSmartDpCList::GetBestDpSensorSet( CBatchSelSmartDpCParams *pclBatchSelSmartDpCParams, double dDpBranch )
{
	CDB_Product *pclBestDpSensor = NULL;

	if( NULL != m_pclBestDpSensor )
	{
		pclBestDpSensor = m_pclBestDpSensor;
	}
	else
	{
		pclBestDpSensor = _SelectBestDpSensor( pclBatchSelSmartDpCParams, dDpBranch, true );
	}

	return pclBestDpSensor;
}

void CBatchSelectSmartDpCList::Clean()
{
	m_bBestFound = false;
	m_bSizeShiftProblem = false;
	m_pclBestDpSensor = NULL;
	CBatchSelectBaseList::Clean();
}

bool CBatchSelectSmartDpCList::_SelectSmartDpC( CBatchSelSmartDpCParams *pclBatchSelSmartDpCParams, double dFlow )
{
	// !!! REMARK: for the moment, it's exactly the same code as for the hydraulic calculator.
	
	m_bBestFound = false;

	if( 0 == m_clSelectPipeList.GetPipeNumbers() )
	{
		return false;
	}

	int iHighestSize;
	int iLowestSize;

	if( false == GetPipeSizeShift( (CBatchSelectionParameters*)pclBatchSelSmartDpCParams, iHighestSize, iLowestSize ) )
	{
		return false;
	}

	CTableDN *pclTableDN = dynamic_cast<CTableDN *>( pclBatchSelSmartDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERTA_RETURN( false );
	}

	int iDN50 = pclTableDN->GetSize( _T("DN_50") );

	// Take all available products of the same type.
	CRankEx clSmartDpCList;

	pclBatchSelSmartDpCParams->m_pTADB->GetSmartDpCList( &clSmartDpCList, _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), pclBatchSelSmartDpCParams->m_eFilterSelection, 0, INT_MAX, 0, false, false );

	if( 0 == clSmartDpCList.GetCount() )
	{
		return false;
	}

	double dDpMaxForBestSuggestion = pclBatchSelSmartDpCParams->m_pTADS->GetpTechParams()->GetSmartValveDpMaxForBestSuggestion();
	double dRho = pclBatchSelSmartDpCParams->m_WC.GetDens();
	bool bAtLeastOneFit = false;
	m_bSizeShiftProblem = false;
	mmapKeyDoubleSelectedProduct mmapSmartDpC;
	_string str;
	LPARAM lpParam;
	
	for( BOOL bContinue = clSmartDpCList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = clSmartDpCList.GetNext( str, lpParam ) )
	{
		CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( (CData *)lpParam );
				
		if( NULL == pclSmartDpC )
		{
			continue;
		}

		if( false == pclSmartDpC->IsSelectable( true ) )
		{
			continue;
		}

		CDB_SmartValveCharacteristic *pclSmartDpCCharacteristic = pclSmartDpC->GetSmartValveCharacteristic();

		if( NULL == pclSmartDpCCharacteristic )
		{
			clSmartDpCList.Delete();
			continue;
		}

		if( dFlow < pclSmartDpCCharacteristic->GetMinAdjustableFlow() || dFlow > pclSmartDpCCharacteristic->GetQnom() )
		{
			clSmartDpCList.Delete();
			continue;
		}

		// Create a new entry in pre-selection linked list.
		CSelectedValve *pSelectedSmartDpC = new CSelectedValve();
		ASSERT( NULL != pSelectedSmartDpC );

		if( NULL == pSelectedSmartDpC )
		{
			continue;
		}

		double dDp = CalcDp( dFlow, pclSmartDpC->GetKvs(), pclBatchSelSmartDpCParams->m_WC.GetDens() );
		pSelectedSmartDpC->SetDp( dDp );

		pSelectedSmartDpC->SetProductIDPtr( pclSmartDpC->GetIDPtr() );
		pSelectedSmartDpC->SetFlag( CSelectedBase::Flags::eNotPriority, false );

		double d64Key = pclSmartDpC->GetSortingKey();
		mmapSmartDpC.insert( std::make_pair( d64Key, pSelectedSmartDpC ) );
	}

	bool bOneSmartDpCExist = false;
	int iSmallestValve = 0;
	int iHighestValve = 99999;
	
	// Key is the sorting key.
	mmapKeyDoubleSelectedProduct::iterator mapIt;

	// Adapt 'dLowestSize' and 'iHigestSize' to include at least one smart control valve.
	for( mapIt = mmapSmartDpC.begin(); mapIt != mmapSmartDpC.end() && bOneSmartDpCExist == false; ++mapIt )
	{
		CDB_TAProduct *pclTAProduct = (CDB_TAProduct *)( mapIt->second->GetProductIDPtr().MP );

		if( NULL == pclTAProduct )
		{
			continue;
		}

		int iProdSizeKey = pclTAProduct->GetSizeKey();
		
		if( iProdSizeKey < iHighestValve && iProdSizeKey > iHighestSize )
		{
			iHighestValve = iProdSizeKey;
		}

		if( iProdSizeKey > iSmallestValve && iProdSizeKey < iLowestSize )
		{
			iSmallestValve = iProdSizeKey;
		}
		
		if( iProdSizeKey >= iLowestSize && iProdSizeKey <= iHighestSize )
		{
			bOneSmartDpCExist = true;
		}
	}
	
	if( false == bOneSmartDpCExist )
	{
		if( iSmallestValve < iLowestSize )
		{
			iLowestSize = iSmallestValve;
		}
		
		if( iHighestValve > iHighestSize )
		{
			iHighestSize = iHighestValve;
		}
	}

	for( mapIt = mmapSmartDpC.begin(); mapIt != mmapSmartDpC.end(); )
	{
		CDB_SmartControlValve *pclSmartDpC = (CDB_SmartControlValve *)( mapIt->second->GetProductIDPtr().MP );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		int iProdSizeKey = pclSmartDpC->GetSizeKey();
		
		if( !( iProdSizeKey >= iLowestSize && iProdSizeKey <= iHighestSize ) )
		{
			if( mapIt == mmapSmartDpC.begin() )
			{
				// First proposition is rejected
				// TODO: warning message in compilation Box
			}

			// Remove this selection.
			if( mmapSmartDpC.size() > 1 )
			{
				mmapKeyDoubleSelectedProduct::iterator it = mapIt;
				++mapIt;
				delete it->second;
				mmapSmartDpC.erase( it );
				continue;
			}
			else
			{
				// To keep a trace of created object to well erase them when no more needed.
				m_vecObjects.push_back( mapIt->second );
			}
		}
		else
		{
			// To keep a trace of created object to well erase them when no more needed.
			m_vecObjects.push_back( mapIt->second );
			bOneSmartDpCExist = true;
		}

		++mapIt;
	}

	if( 0 == mmapSmartDpC.size() )
	{
		return false;
	}
	
	// Build sorting keys.
	CTAPSortKey sKeyTechParamBlw65;
	CTAPSortKey sKeyTechParamAbv50;
	BuildSortingKeys( pclBatchSelSmartDpCParams, &sKeyTechParamBlw65 , &sKeyTechParamAbv50 );

	double dKeyCurrentTAP = 0;
	bool bFound = false;

	for( int iUserPrefLoop = 0; iUserPrefLoop < 4 && false == bFound; iUserPrefLoop++ )
	{
		// This loop give us the possibility to relax mask criteria one by one.
		// We will start with the most constraining key and if we don't found a product we will relax constrains one by one.
		CTAPSortKey::TAPSortKeyMask eMask = ( CTAPSortKey::TAPSortKeyMask )( CTAPSortKey::TAPSKM_All & ~CTAPSortKey::TAPSKM_PriorL 
				& ~CTAPSortKey::TAPSKM_Size & ~CTAPSortKey::TAPSKM_Type & ~CTAPSortKey::TAPSKM_Fam & ~CTAPSortKey::TAPSKM_Version );

		switch( iUserPrefLoop )
		{
			case 0:
				// No filter, we are strict.
				break;

			case 1:
				// Relax PN.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
				break;

			case 2:
				// Relax PN and connection.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Connect );
				break;

			case 3:
				// Relax PN, connection and body material.
				eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Connect 
						& ~CTAPSortKey::TAPSKM_Bdy );
				break;
		}

		double dKeyTechParamBlw65 = sKeyTechParamBlw65.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
		double dKeyTechParamAbv50 = sKeyTechParamAbv50.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );

		mmapKeyDoubleSelectedProduct::iterator ItBestBelowQNom = mmapSmartDpC.end();
		double dBestDeltaBelowQNom = DBL_MAX;

		for( mapIt = mmapSmartDpC.begin(); mapIt != mmapSmartDpC.end(); ++mapIt )
		{
			CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( mapIt->second->GetProductIDPtr().MP );
			
			if( NULL == pclSmartDpC )
			{
				continue;
			}

			bool bValveMatch = false;
			int iSize = pclSmartDpC->GetSizeKey();
			double dTAPKey = pclSmartDpC->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
			
			// Selected valve is not located on the same side of iDN50 border.
			// Use defined technical choice.
			if( iSize <= iDN50 )
			{
				if( dTAPKey == dKeyTechParamBlw65 )
				{
					bValveMatch = true;
				}
			}
			else
			{
				// Size > DN50
				if( dTAPKey == dKeyTechParamAbv50 )
				{
					bValveMatch = true;
				}
			}
			
			if( false == bValveMatch )
			{
				continue;
			}

			CDB_SmartValveCharacteristic *pclSmartDpCCharacteristic = pclSmartDpC->GetSmartValveCharacteristic();

			if( NULL == pclSmartDpCCharacteristic )
			{
				continue;
			}

			// Check the product that is closest of its nominal flow.
			if( pclSmartDpCCharacteristic->GetQnom() - dFlow < dBestDeltaBelowQNom )
			{
				if( pclSmartDpC->GetDpMin( dFlow, dRho ) <= dDpMaxForBestSuggestion )
				{
					dBestDeltaBelowQNom = pclSmartDpCCharacteristic->GetQnom() - dFlow;
					ItBestBelowQNom = mapIt;
				}
			}

			CreateKeyAndInsertInMap( &m_PreselMMapKeyDecreasing, pclSmartDpC, mapIt->second );
		}

		if( DBL_MAX != dBestDeltaBelowQNom )
		{
			bFound = true;
			ItBestBelowQNom->second->SetFlag( CSelectedBase::eBest, true );

			if( 0 != iUserPrefLoop )
			{
				ItBestBelowQNom->second->SetFlag( CSelectedBase::eValveBatchAlter, true );
			}
		}
	}

	return true;
}

CDB_DpSensor *CBatchSelectSmartDpCList::_SelectBestDpSensor( CBatchSelSmartDpCParams *pclBatchSelSmartDpCParams, double dDpBranch, bool bForSet )
{
	if( NULL == pclBatchSelSmartDpCParams || NULL == m_pclBestProduct || NULL == dynamic_cast<CDB_SmartControlValve *>( m_pclBestProduct->GetProductIDPtr().MP ) )
	{
		return NULL;
	}

	CDB_SmartControlValve *pclSmartDpC = (CDB_SmartControlValve *)( m_pclBestProduct->GetProductIDPtr().MP );

	double dDpToStabilize = 0.0;

	if( true == pclBatchSelSmartDpCParams->m_bIsCheckboxDpBranchChecked && dDpBranch > 0.0 )
	{
		dDpToStabilize = dDpBranch;
	}

	// HYS-1992: No set for Smart DpC.
	CTable *pclTable = (CTable*)pclSmartDpC->GetDpSensorGroupIDPtr().MP;

	if( NULL == pclTable )
	{
		return NULL;
	}

	CRank ProductSetList;

	for( IDPTR IDPtr = pclTable->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pclTable->GetNext() )
	{
		CDB_Product *pclProductSet = dynamic_cast<CDB_Product *>(IDPtr.MP );

		double dKey = (double)pclProductSet->GetSortingKey();

		if( NULL != dynamic_cast<CDB_DpSensor *>( pclProductSet ) )
		{
			CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclProductSet );

			ASSERT( ( pclDpSensor->GetMinMeasurableDp() / 1000.0 ) < 100.0 );
			dKey = dKey * 100.0 + ( pclDpSensor->GetMinMeasurableDp() / 1000.0 );
		}
		else
		{
			dKey += 1e9;
		}

		ProductSetList.Add( pclProductSet->GetName(), dKey, (LPARAM)(IDPtr.MP) );
	}

	CString str;
	LPARAM lpItemData = 0;
	CDB_DpSensor *pclBest = NULL;
	CDB_DpSensor *pclMinMeasurableDp = NULL;
	double dBestDelta = DBL_MAX;

	for( BOOL bContinue = ProductSetList.GetFirst( str, lpItemData ); TRUE == bContinue; bContinue = ProductSetList.GetNext( str, lpItemData ) ) 
	{
		CDB_DpSensor *pclProductSet = dynamic_cast<CDB_DpSensor *>( (CData *)lpItemData );

		if( NULL == pclProductSet )
		{
			// It's a connection set, we don't take this one.
			continue;
		}

		// Keep the best of the worst in case of.
		if( NULL == pclMinMeasurableDp || pclProductSet->GetMinMeasurableDp() < pclMinMeasurableDp->GetMinMeasurableDp() )
		{
			pclMinMeasurableDp = pclProductSet;
		}

		if( dDpToStabilize < pclProductSet->GetMinMeasurableDp() )
		{
			continue;
		}

		if( dDpToStabilize - pclProductSet->GetMinMeasurableDp() < dBestDelta )
		{
			dBestDelta = dDpToStabilize - pclProductSet->GetMinMeasurableDp();
			pclBest = pclProductSet;
		}
	}

	if( NULL != pclBest )
	{
		m_pclBestDpSensor = (CDB_Product*)pclBest;
	}
	else if( NULL != pclMinMeasurableDp )
	{
		m_pclBestDpSensor = (CDB_Product*)pclMinMeasurableDp;
	}

	return (CDB_DpSensor*)m_pclBestDpSensor;
}