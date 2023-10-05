
//
// HydroMod.cpp: implementation of the CDS_HydroMod class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <map>
#include "MainFrm.h"
#include "HMInclude.h"
#include "HiPerfTimer.h"
#include "DlgOutput.h"
#include "DlgHMCompilationOutput.h"
//#define CDSHM_CAL

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DATA( CDS_HydroMod )
IMPLEMENT_DATA( CDS_Hm2W )
IMPLEMENT_DATA( CDS_Hm2WInj )
IMPLEMENT_DATA( CDS_Hm3W )
IMPLEMENT_DATA( CDS_Hm3WInj )
IMPLEMENT_DATA( CDS_HmHub )
IMPLEMENT_DATA( CDS_HmHubStation )

// Let this variable global.
CDS_HydroMod_DebugLog m_clHydroModDebugLog;

CDS_HydroMod::CDS_HydroMod( CDataBase *pDataBase, LPCTSTR tcsID )
	: CTable( pDataBase, tcsID )
{
	m_pSingulTab = static_cast<CTable *>( TASApp.GetpTADB()->Get( _T("SINGULAR_TAB") ).MP );
	ASSERT( NULL != m_pSingulTab );

	m_pclTableDN = static_cast<CTableDN *>( TASApp.GetpTADB()->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != m_pclTableDN );

	_tcsncpy_s( m_HMName, SIZEOFINTCHAR( m_HMName ), _T(""), SIZEOFINTCHAR( m_HMName ) - 1 );
	m_strDescription = _T("");
	m_iLevel = -1;
	m_usUniqueID = 0;
	m_iPosition = -1;
	m_bModule = true;
	m_bPending = false;
	m_strDeprecatedSelThingID = _T("");
	_tcsncpy_s( m_tcCBIType, SIZEOFINTCHAR( m_tcCBIType ), _T(""), SIZEOFINTCHAR( m_tcCBIType ) - 1 );
	_tcsncpy_s( m_tcCBISize, SIZEOFINTCHAR( m_tcCBISize ), _T(""), SIZEOFINTCHAR( m_tcCBISize ) - 1 );
	m_dQDesign = 0.0;
	m_dDpDesign = 0.0;
	m_dPresetting = -1.0;
	m_dDiversityFactor = -1;	// Remark: -1.0 is the value to tell that there is not yet diversity factor applied.
	m_eVDescrType = ( enum_VDescriptionType ) 0;
	m_dKvCv = 0.0;
	m_eTreatment = ett_None;
	m_SchemeIDPtr = _NULL_IDPTR;
	m_strRemL1 = _T("");
	m_iRemarkIndex = 0;					// Remark Index used for visualisation; computed each time the table is displayed
	m_strComment = _T("");
	m_bIsCircuitIndex = false;
	m_dHAvail = 0.0;
	m_dHPressInt = 0.0;
	m_usLock = 0;
	m_eReturnType = ReturnType::Direct;
	m_bCheck = false;
	m_bHMCalcMode = false;
	m_bConstantFlow = false;
	m_pCircuitPrimaryPipe = NULL;
	m_pCircuitSecondaryPipe = NULL;
	m_pCircuitBypassPipe = NULL;
	m_pDistrSupplyPipe = NULL;
	m_pDistrReturnPipe = NULL;
	m_CBIValveIDPtr = _NULL_IDPTR;
	m_pDpC = NULL;
	m_pCv = NULL;
	m_pBv = NULL;
	m_pSecBv = NULL;
	m_pBypBv = NULL;
	m_pShutoffValveSupply = NULL;
	m_pShutoffValveReturn = NULL;
	m_pSmartControlValve = NULL;
	m_pSmartDpC = NULL;
	m_pTermUnit = NULL;
	m_pPump = NULL;
	m_bComputeAllInCourse = false;
	m_iDN50 = m_pclTableDN->GetSize( _T("DN_50") );
	m_iDN15 = m_pclTableDN->GetSize( _T("DN_15") );
	m_lpTreatment = NULL;
	m_uCurrentMeasDataIndex = 0;
	m_ucModuleStatus = 0;
	
	m_pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != m_pPrjParam );

	m_pTechParam = TASApp.GetpTADS()->GetpTechParams();
	ASSERT( NULL != m_pTechParam );

	m_MeasDataDateTimeVector.clear();
	m_bComputeAllowed = true;
	m_mapErrorMessages.clear();

	m_ComputeHMState = eceNone;
	m_ComputeHMParam = 0;
}

CDS_HydroMod::~CDS_HydroMod()
{
	DeletePipes();
	DeleteBv( &m_pBv );
	DeleteBv( &m_pBypBv );
	DeleteBv( &m_pSecBv );
	DeleteSV( &m_pShutoffValveSupply );
	DeleteSV( &m_pShutoffValveReturn );
	DeleteSmartControlValve();
	DeleteSmartDpC();
	DeleteDpC();
	DeleteTermUnit();
	DeleteCv();
	DeleteMeasData();
	DeletePump();
}

CDS_HydroMod *CDS_HydroMod::ImportHydroMod( CDS_HydroMod *pHM, int iNewIndex )
{
	try
	{
		if( NULL == pHM )
		{
			return NULL;
		}

		IDPTR newIDPtr;
		TASApp.GetpTADS()->CreateObject( newIDPtr, pHM->GetClassName() );
	
		CDS_HydroMod *pNewHM = dynamic_cast<CDS_HydroMod *>( newIDPtr.MP );
		TASApp.GetpTADS()->VerifyNewPos( this, iNewIndex );
		
		Insert( newIDPtr );
	
		pNewHM->SetSchemeIDPtr( pHM->GetSchemeIDPtr() );
		pHM->Copy( pNewHM );

		if(true == IsClass( CLASS( CTableHM ) ) )
		{
			pNewHM->SetLevel( 0 );
		}
		else
		{
			pNewHM->SetLevel( GetLevel() + 1 );
		}

		pNewHM->SetPos( iNewIndex );
		pNewHM->RenameMe( true );
		pNewHM->ComputeHM( eceResize );

		return pNewHM;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDS_HydroMod::ImportHydroMod'."), __LINE__, __FILE__ );
		throw;
	}
}

bool CDS_HydroMod::IsPipeUsed( IDPTR IDPtrPipeSerie, IDPTR IDPtrPipeSize )
{
	bool bReturn = false;

	if( NULL != GetpDistrSupplyPipe() )
	{
		if( true == GetpDistrSupplyPipe()->IsPipeUsed( IDPtrPipeSerie, IDPtrPipeSize ) )
		{
			bReturn = true;
		}
	}

	if( false == bReturn && NULL != GetpDistrReturnPipe() )
	{
		if( true == GetpDistrReturnPipe()->IsPipeUsed( IDPtrPipeSerie, IDPtrPipeSize ) )
		{
			bReturn = true;
		}
	}

	if( false == bReturn && NULL != GetpCircuitPrimaryPipe() )
	{
		if( true == GetpCircuitPrimaryPipe()->IsPipeUsed( IDPtrPipeSerie, IDPtrPipeSize ) )
		{
			bReturn = true;
		}
	}

	if( false == bReturn && NULL != GetpCircuitSecondaryPipe() )
	{
		if( true == GetpCircuitSecondaryPipe()->IsPipeUsed( IDPtrPipeSerie, IDPtrPipeSize ) )
		{
			bReturn = true;
		}
	}

	if( false == bReturn && NULL != GetpCircuitBypassPipe() )
	{
		if( true == GetpCircuitBypassPipe()->IsPipeUsed( IDPtrPipeSerie, IDPtrPipeSize ) )
		{
			bReturn = true;
		}
	}

	return bReturn;
}

// pHM Target
// bValveDataOnly == false doesn't copy HM name, position, ...
// bCreateLinkedDevice create HM associated devices as pipe, Bv, DpC, Cv, ..
// bToClipboard Selected product will be copied into the clipboard
// bResetDiversityFactor is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the
// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
void CDS_HydroMod::Copy( CDS_HydroMod *pHMTarget, bool bValveDataOnly, bool bCreateLinkedDevice, bool bToClipboard, bool bResetDiversityFactor )
{
	if( NULL == dynamic_cast<CDS_HydroMod *>( pHMTarget ) )
	{
		ASSERT( 0 );
		return;
	}

	pHMTarget->EnableComputeHM( false );
	pHMTarget->SetLock( m_usLock );

	//
	// Copy data as module flag in this place, this flag is needed to set up flow in terminal unit
	if( false == bValveDataOnly )
	{
		pHMTarget->SetHMName( GetHMName() );
		pHMTarget->SetDescription( GetDescription() );
		pHMTarget->SetLevel( GetLevel() );
		pHMTarget->SetUid( m_usUniqueID );
		pHMTarget->SetPos( GetPos() );
		pHMTarget->SetFlagModule( IsaModule() );
		pHMTarget->SetQDesign(m_dQDesign);
	}

	if( true == bResetDiversityFactor )
	{
		pHMTarget->ResetDiversityFactor();
	}
	else
	{
		pHMTarget->SetDiversityFactor( GetDiversityFactor() );
	}

	// SetReturnType will create necessary pipes
	pHMTarget->SetReturnType( GetReturnType() );

	// Create pipe without singularity, it will be added by the copy function.
	if( true == bCreateLinkedDevice )
	{
		pHMTarget->CreatePipes( false );
	}

	if( NULL != pHMTarget->GetpCircuitPrimaryPipe() && NULL != m_pCircuitPrimaryPipe )
	{
		m_pCircuitPrimaryPipe->Copy( pHMTarget->GetpCircuitPrimaryPipe(), bResetDiversityFactor );
	}

	if( NULL != pHMTarget->GetpCircuitBypassPipe() && NULL != m_pCircuitBypassPipe )
	{
		m_pCircuitBypassPipe->Copy( pHMTarget->GetpCircuitBypassPipe(), bResetDiversityFactor );
	}

	if( NULL != pHMTarget->GetpCircuitSecondaryPipe() && NULL != m_pCircuitSecondaryPipe )
	{
		m_pCircuitSecondaryPipe->Copy( pHMTarget->GetpCircuitSecondaryPipe(), bResetDiversityFactor );
	}

	if( NULL != pHMTarget->GetpDistrSupplyPipe() && NULL != m_pDistrSupplyPipe )
	{
		m_pDistrSupplyPipe->Copy( pHMTarget->GetpDistrSupplyPipe(), bResetDiversityFactor );
	}

	if( NULL != pHMTarget->GetpDistrReturnPipe() && NULL != m_pDistrReturnPipe )
	{
		m_pDistrReturnPipe->Copy( pHMTarget->GetpDistrReturnPipe(), bResetDiversityFactor );
	}

	if( true == IsDpCExist() )
	{
		if( true == bCreateLinkedDevice )
		{
			pHMTarget->CreateDpC();
			if( true == IsBvExist() )
			{
				pHMTarget->CreateBv( CDS_HydroMod::eHMObj::eBVprim );
			}
		}

		if( NULL != pHMTarget->GetpDpC() )
		{
			m_pDpC->Copy( pHMTarget->GetpDpC(), bResetDiversityFactor );
		}
	}

	if( true == IsBvExist() )
	{
		if( NULL == IsDpCExist() )
		{
			if( true == bCreateLinkedDevice )
			{
				pHMTarget->CreateBv( CDS_HydroMod::eHMObj::eBVprim );
			}
		}

		if( NULL != pHMTarget->GetpBv() )
		{
			m_pBv->Copy( pHMTarget->GetpBv(), bResetDiversityFactor );
		}
	}

	if( true == IsBvBypExist() )
	{
		pHMTarget->CreateBv( CDS_HydroMod::eHMObj::eBVbyp );

		if( NULL != pHMTarget->GetpBypBv() )
		{
			m_pBypBv->Copy( pHMTarget->GetpBypBv(), bResetDiversityFactor );
		}
	}

	if( true == IsBvSecExist() )
	{
		if( true == bCreateLinkedDevice )
		{
			pHMTarget->CreateBv( CDS_HydroMod::eHMObj::eBVsec );
		}

		if( NULL != pHMTarget->GetpSecBv() )
		{
			m_pSecBv->Copy( pHMTarget->GetpSecBv(), bResetDiversityFactor );
		}
	}

	if( true == IsPumpExist() )
	{
		if( true == bCreateLinkedDevice )
		{
			pHMTarget->CreatePump();
		}

		if( NULL != pHMTarget->GetpPump() )
		{
			m_pPump->Copy( pHMTarget->GetpPump() );
		}
	}

	if( true == IsCvExist() )
	{
		if( true == bCreateLinkedDevice )
		{
			pHMTarget->CreateCv( GetpCV()->GetCvLocate() );

			// Create a linked TACV if needed.
			if( true == GetpCV()->IsTaCV() )
			{
				pHMTarget->GetpCV()->SetTACv( true );
				// Set CV IDptr temporary.  It will be used into CreateLinkedCV() to switch between TA Product type.
				pHMTarget->GetpCV()->SetCvIDPtr( GetpCV()->GetCvIDPtr() );
			}
		}

		if( NULL != pHMTarget->GetpCV() )
		{
			m_pCv->Copy( pHMTarget->GetpCV(), bResetDiversityFactor );
		}
	}

	if( true == IsShutoffValveExist( eHMObj::eShutoffValveSupply ) )
	{
		if( true == bCreateLinkedDevice )
		{
			pHMTarget->CreateSV( eShutoffValveSupply );
		}

		if( NULL != pHMTarget->GetpShutoffValve( eHMObj::eShutoffValveSupply ) )
		{
			m_pShutoffValveSupply->Copy( pHMTarget->GetpShutoffValve( eHMObj::eShutoffValveSupply ), bResetDiversityFactor );
		}
	}

	if( true == IsShutoffValveExist( eHMObj::eShutoffValveReturn ) )
	{
		if( true == bCreateLinkedDevice )
		{
			pHMTarget->CreateSV( eShutoffValveReturn );
		}

		if( NULL != pHMTarget->GetpShutoffValve( eHMObj::eShutoffValveReturn ) )
		{
			m_pShutoffValveReturn->Copy( pHMTarget->GetpShutoffValve( eHMObj::eShutoffValveReturn ), bResetDiversityFactor );
		}
	}

	if( true == IsSmartControlValveExist() )
	{
		if( true == bCreateLinkedDevice )
		{
			pHMTarget->CreateSmartControlValve( GetpSmartControlValve()->GetLocalization() );
		}

		if( NULL != pHMTarget->GetpSmartControlValve() )
		{
			m_pSmartControlValve->Copy( pHMTarget->GetpSmartControlValve(), bResetDiversityFactor );
		}
	}

	if( true == IsSmartDpCExist() )
	{
		if( true == bCreateLinkedDevice )
		{
			pHMTarget->CreateSmartDpC( GetpSmartDpC()->GetLocalization() );
		}

		if( NULL != pHMTarget->GetpSmartDpC() )
		{
			m_pSmartDpC->Copy( pHMTarget->GetpSmartDpC(), bResetDiversityFactor );
		}
	}

	if( true == IsTermUnitExist() )
	{
		if( true == bCreateLinkedDevice )
		{
			pHMTarget->CreateTermUnit();
		}

		if( NULL != pHMTarget->GetpTermUnit() )
		{
			m_pTermUnit->CopyTo( pHMTarget->GetpTermUnit() );

			// HYS-1766: Copy necessary terminal unit values into the valves.
			if( CTermUnit::_QType::Q == pHMTarget->GetpTermUnit()->GetQType() )
			{
				m_pTermUnit->SetQ( pHMTarget->GetpTermUnit()->GetTerminalUnitData()->m_dQ, false );
			}
			else if( CTermUnit::_QType::PdT == pHMTarget->GetpTermUnit()->GetQType() )
			{
				// HYS-1872: No computeHM during this copy process.
				m_pTermUnit->SetPdT( pHMTarget->GetpTermUnit()->GetTerminalUnitData()->m_dP, pHMTarget->GetpTermUnit()->GetTerminalUnitData()->m_dDT, false );
			}
		}
	}

	pHMTarget->SetCBIType( GetCBIType() );
	pHMTarget->SetCBISize( GetCBISize() );
	pHMTarget->SetCBIValveID( GetCBIValveID() );
	pHMTarget->SetQDesign( GetQDesign() );
	pHMTarget->SetDpDesign( GetDpDesign() );
	pHMTarget->SetFlagCircuitIndex( IsCircuitIndex() );
	pHMTarget->SetPresetting( GetPresetting() );
	pHMTarget->SetVDescrType( GetVDescrType() );
	pHMTarget->SetKvCv( GetKvCv() );
	pHMTarget->SetTreatment( GetTreatment() );
	pHMTarget->SetTreatmentLparam( GetTreatmentLparam() );
	//Doesn't copy Scheme it is fixed by the hydromod creation
	pHMTarget->SetRemL1( GetRemL1() );

	pHMTarget->SetPending( GetPending() );
	pHMTarget->SetHMCalcMode( IsHMCalcMode() );

	pHMTarget->m_dHAvail = GetHAvail( CAnchorPt::CircuitSide_Primary );
	pHMTarget->m_dHPressInt = m_dHPressInt;

	int iSize = m_MeasDataDateTimeVector.size();

	for( int i = 0; i < iSize; i++ )
	{
		CMeasData *pMeasData = pHMTarget->CreateMeasData();

		if (NULL == pMeasData)
		{
			ASSERT( NULL != pMeasData );
			continue;
		}

		GetpMeasData( i )->Copy( pMeasData );
	}
}

void CDS_HydroMod::Copy( CData *pTo )
{

	( (CDS_HydroMod *)pTo )->SetSchemeIDPtr( GetSchemeIDPtr() );
	CDS_HydroMod::Copy( (CDS_HydroMod *)pTo, false, true, false, false );
}

bool CDS_HydroMod::IsAvailableForTASCOPE()
{
	bool bRet = false;
	CompleteHMForHydroCalcMode( GetParent() );

	// Balancing Type.
	CDB_CircuitScheme *pSch = GetpSch();

	if( NULL == pSch )
	{
		return false;
	}

	// HYS-1750: Add Pending circuit for terminal unit.
	if( 0 != IDcmp( pSch->GetIDPtr().ID, L"SP" ) && 0 != IDcmp( pSch->GetIDPtr().ID, L"PDG" ) 
		&& 0 != IDcmp( pSch->GetIDPtr().ID, L"PUMP" ) && 0 != IDcmp( pSch->GetIDPtr().ID, L"PUMP_BV" ) )
	{
		// Test Kv-Mode if the circuit is not a SP (straight pipe).
		if( 0 == *GetCBIValveID() )
		{
			if( GetVDescrType() == edt_KvCv )
			{
				return true;
			}
		}
	}

	bRet = true;
	return bRet;
}

bool CDS_HydroMod::AnalyzeVSPDpSensorPosition( CDS_HydroMod **pHMMostRequesting, double &dRequestedDp )
{
	// Return true if applicable.
	// Establish the list of DpC circuits.
	// Could be recursive when module is a simple pipe.
	for( IDPTR idptr = GetFirst(); NULL != idptr.MP; idptr = GetNext( idptr.MP ) )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( idptr.MP );

		if( NULL == pHM->GetpSch() )
		{
			return false;
		}

		if( pHM->GetpSch()->GetDpCType() == CDB_CircuitScheme::eDpCTypeDPC	|| pHM->GetpSch()->GetDpCType() == CDB_CircuitScheme::eDpCTypePICV
				|| pHM->GetpSch()->GetDpCType() == CDB_CircuitScheme::eDpCTypeDPCBCV )
		{
			double dDp = pHM->GetDp( true );

			if( dRequestedDp < dDp )
			{
				dRequestedDp = dDp;
				( *pHMMostRequesting ) = pHM;
			}
		}
		else if( true == pHM->IsaModule() && NULL != pHM->GetpSchcat() && true == pHM->GetpSchcat()->IsStraight() )
		{
			// Analyze recursively.
			if( false == pHM->AnalyzeVSPDpSensorPosition( pHMMostRequesting, dRequestedDp ) )
			{
				return false;
			}
		}
		else
		{
			// Not compliant circuit.
			return false;
		}
	}

	if( NULL == ( *pHMMostRequesting ) )
	{
		return false;
	}

	return true;
}

void CDS_HydroMod::CleanAndPrepareHM( CDS_HydroModX *pHMX, CTable *pclParent, int iLevel, bool bImportInExistingHM )
{
	CDB_CircuitScheme *pCircuitSchemeHMX = dynamic_cast<CDB_CircuitScheme *>( pHMX->GetSchemeIDPtr().MP );

	if( NULL == pCircuitSchemeHMX )
	{
		ASSERT_RETURN;
	}

	if( true == bImportInExistingHM )
	{
		// In case we import HydroModX into an existent HydroMod, we have to verify if some valves in HydroMod
		// can be deleted.
		// Example: User creates a 2 way control circuit in HySelect and send it to the TA-Scope. In TA-Scope, user
		// changes the circuit to a distribution circuit (no more control valve is needed). When importing the project
		// in the existing one in HySelect, we have thus to delete the control valve.
		CDB_CircuitScheme *pCircuitSchemeHM = dynamic_cast<CDB_CircuitScheme *>( GetSchemeIDPtr().MP );

		if( NULL == pCircuitSchemeHM )
		{
			ASSERT_RETURN;
		}

		// By default we control all the anchoring points.
		bool arbExistingAnchorPointHMX[CAnchorPt::eFunc::eFuncLast + 1] = { 0 };

		for( int i = 0; i < pCircuitSchemeHMX->GetAnchorPtListSize(); i++ )
		{
			arbExistingAnchorPointHMX[pCircuitSchemeHMX->GetAnchorPtFunc( i )] = true;
		}

		for( int i = 0; i < CAnchorPt::eFunc::eFuncLast; i++ )
		{
			switch( i )
			{
				case CAnchorPt::eFunc::BV_P:

					if( false == arbExistingAnchorPointHMX[i] && NULL != m_pBv )
					{
						DeleteBv( &m_pBv );
					}
					else if( true == arbExistingAnchorPointHMX[1] && NULL == m_pBv )
					{
						CreateBv( eBVprim );
					}

					break;

				case CAnchorPt::eFunc::BV_Byp:

					if( false == arbExistingAnchorPointHMX[i] && NULL != m_pBypBv )
					{
						DeleteBv( &m_pBypBv );
					}
					else if( true == arbExistingAnchorPointHMX[i] && NULL == m_pBypBv )
					{
						CreateBv( eBVbyp );
					}

					break;

				case CAnchorPt::eFunc::BV_S:

					if( false == arbExistingAnchorPointHMX[i] && NULL != m_pSecBv )
					{
						DeleteBv( &m_pSecBv );
					}
					else if( true == arbExistingAnchorPointHMX[i] && NULL == m_pSecBv )
					{
						CreateBv( eBVsec );
					}

					break;

				case CAnchorPt::eFunc::ControlValve:

					if( false == arbExistingAnchorPointHMX[i] && NULL != m_pCv )
					{
						if( false == arbExistingAnchorPointHMX[CAnchorPt::eFunc::PICV] && false == arbExistingAnchorPointHMX[CAnchorPt::eFunc::DPCBCV] )
						{
							DeleteCv();
						}
					}
					else if( true == arbExistingAnchorPointHMX[i] && NULL == m_pCv )
					{
						CreateCv( pCircuitSchemeHMX->GetCvLoc() );
					}

					break;

				case CAnchorPt::eFunc::DpC:

					if( false == arbExistingAnchorPointHMX[i] && NULL != m_pDpC )
					{
						DeleteDpC();
					}
					else if( true == arbExistingAnchorPointHMX[i] && NULL == m_pDpC )
					{
						CreateDpC();
					}

					break;

				case CAnchorPt::eFunc::PICV:

					if( false == arbExistingAnchorPointHMX[i] && NULL != m_pCv )
					{
						if( false == arbExistingAnchorPointHMX[CAnchorPt::eFunc::ControlValve] && false == arbExistingAnchorPointHMX[CAnchorPt::eFunc::DPCBCV] )
						{
							DeleteCv();
						}
					}
					else if( true == arbExistingAnchorPointHMX[i] && NULL == m_pCv )
					{
						CreateCv( pCircuitSchemeHMX->GetCvLoc() );
					}

					break;

				case CAnchorPt::eFunc::PUMP:

					if( false == arbExistingAnchorPointHMX[i] && NULL != m_pPump )
					{
						DeletePump();
					}
					else if( true == arbExistingAnchorPointHMX[i] && NULL == m_pPump )
					{
						CreatePump();
					}

					break;

				case CAnchorPt::eFunc::ShutoffValve:

					if( false == arbExistingAnchorPointHMX[i] && NULL != m_pShutoffValveSupply )
					{
						DeleteSV( &m_pShutoffValveSupply );
					}
					else if( true == arbExistingAnchorPointHMX[i] && NULL == m_pShutoffValveSupply )
					{
						CreateSV( eShutoffValveSupply );
					}

					if( false == arbExistingAnchorPointHMX[i] && NULL != m_pShutoffValveReturn )
					{
						DeleteSV( &m_pShutoffValveReturn );
					}
					else if( true == arbExistingAnchorPointHMX[i] && NULL == m_pShutoffValveReturn )
					{
						CreateSV( eShutoffValveReturn );
					}

					break;

				case CAnchorPt::eFunc::DPCBCV:

					if( false == arbExistingAnchorPointHMX[i] && NULL != m_pCv )
					{
						if( false == arbExistingAnchorPointHMX[CAnchorPt::eFunc::ControlValve] && false == arbExistingAnchorPointHMX[CAnchorPt::eFunc::PICV] )
						{
							DeleteCv();
						}
					}
					else if( true == arbExistingAnchorPointHMX[i] && NULL == m_pCv )
					{
						CreateCv( pCircuitSchemeHMX->GetCvLoc() );
					}

					break;

				case CAnchorPt::eFunc::SmartControlValve:
					if( false == arbExistingAnchorPointHMX[i] && NULL != m_pSmartControlValve )
					{
						if( false == arbExistingAnchorPointHMX[CAnchorPt::eFunc::SmartControlValve] )
						{
							DeleteSmartControlValve();
						}
					}
					else if( true == arbExistingAnchorPointHMX[i] && NULL == m_pSmartControlValve )
					{
						CreateSmartControlValve( pCircuitSchemeHMX->GetSmartControlValveLoc() );
					}

					break;

				case CAnchorPt::eFunc::SmartDpC:
					if( false == arbExistingAnchorPointHMX[i] && NULL != m_pSmartDpC )
					{
						if( false == arbExistingAnchorPointHMX[CAnchorPt::eFunc::SmartDpC] )
						{
							DeleteSmartDpC();
						}
					}
					else if( true == arbExistingAnchorPointHMX[i] && NULL == m_pSmartDpC )
					{
						CreateSmartDpC( pCircuitSchemeHMX->GetSmartDpCLoc() );
					}

					break;
			}
		}
	}
	else
	{
		// In the 'CTableHM::LoadFromHMXTable' method we call the 'CreateNewHM' method if project from TA-Scope is imported in a new
		// HySelect empty project. This method create all needed valve in regards to the circuit scheme. If a valve is
		// Here we have to clean created valves in the 'CreateNewHM' method that don't exist in the imported TA-Scope project.
		// Example: If we create a 'CDS_Hm2WInj' object we directly have a shutoff valve created if exist in the circuit scheme.
		// But this valve can be optional. In the 'CreateNewHM' we can't know if this valve exist or not in the TA-Scope project to import.
		// Here we can verify if shutoff valve exist or not in the TA-Scope project. If not we have thus to delete it.
		for( int i = 0; i < pCircuitSchemeHMX->GetAnchorPtListSize(); i++ )
		{
			CAnchorPt::eFunc AnchorPuntFct = pCircuitSchemeHMX->GetAnchorPtFunc( i );

			if( true == pCircuitSchemeHMX->IsAnchorPtOptional( AnchorPuntFct ) )
			{
				switch( AnchorPuntFct )
				{
					case CAnchorPt::eFunc::BV_P:

						if( NULL == pHMX->GetpBV( CDS_HydroModX::InPrimary ) && NULL != GetpBv() )
						{
							DeleteBv( &m_pBv );
						}

						break;

					case CAnchorPt::eFunc::BV_Byp:

						if( NULL == pHMX->GetpBV( CDS_HydroModX::InBypass ) && NULL != GetpBypBv() )
						{
							DeleteBv( &m_pBypBv );
						}

						break;

					case CAnchorPt::eFunc::BV_S:

						if( NULL == pHMX->GetpBV( CDS_HydroModX::InSecondary ) && NULL != GetpSecBv() )
						{
							DeleteBv( &m_pSecBv );
						}

						break;

					case CAnchorPt::eFunc::ControlValve:
					case CAnchorPt::eFunc::PICV:
					case CAnchorPt::eFunc::DPCBCV:

						if( NULL == pHMX->GetpCv() && NULL != GetpCV() )
						{
							DeleteCv();
						}

						break;

					case CAnchorPt::eFunc::DpC:

						if( NULL == pHMX->GetpDpC() && NULL != GetpDpC() )
						{
							DeleteDpC();
						}

						break;

					case CAnchorPt::eFunc::ShutoffValve:

						// In the case of the optional shutoff valve, in HySelect we always keep a 'CDS_HydroMod::ShutoffValve' object
						// to switch between user choice (added or not). This is why here we can't delete.

						break;

					case CAnchorPt::eFunc::SmartControlValve:

						if( NULL == pHMX->GetpSmartControlValve() && NULL != GetpSmartControlValve() )
						{
							DeleteSmartControlValve();
						}

						break;

					case CAnchorPt::eFunc::SmartDpC:

						if( NULL == pHMX->GetpSmartDpC() && NULL != GetpSmartDpC() )
						{
							DeleteSmartDpC();
						}

						break;
				}
			}
		}
	}

	// Define the HMName.
	// In case the HMX is an orphan, do not change the name if it's already the same with a '*'.
	// Define the level and the position according.
	if( pHMX->GetHMXStatus() & CDS_HydroModX::eHMStatusFlags::sfOrphan )
	{
		// Verify if the module is a new one.
		// In that case, the module name is L"".
		if( GetHMName() != _T("") )
		{
			CString HMName = GetHMName();

			if( 0 == HMName.Find( _T('*') ) )
			{
				// An orphan module do not have "*", so we compare the name without the "*".
				HMName = HMName.Right( HMName.GetLength() - 1 );

				if( HMName != pHMX->GetHMName()->c_str() )
				{
					SetHMName( pHMX->GetHMName()->c_str() );
				}
				else
				{
					// Same Name, do nothing
				}
			}
			else
			{
				SetHMName( pHMX->GetHMName()->c_str() );
			}
		}
		else
		{
			CString strHMXName = _T("*");
			strHMXName += pHMX->GetHMName()->c_str();
			SetHMName( strHMXName );
			SetLevel( iLevel );
			SetPos( pHMX->GetPosition() );
		}
	}
	else
	{
		SetHMName( pHMX->GetHMName()->c_str() );
		SetLevel( iLevel );
		SetPos( pHMX->GetPosition() );
	}

	// Define the description, the scheme, the uid.
	SetDescription( pHMX->GetDescription()->c_str() );

	CDS_ProjectParams *pPrjParams = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParams );

	if( NULL == pPrjParams )
	{
		return;
	}

	if( pHMX->GetUid() != 0 )
	{
		SetUid( pHMX->GetUid() );
	}
	else if( NULL != pPrjParams )
	{
		SetUid( pPrjParams->GetNextUid() );
	}

	SetFlagModule( pHMX->IsaModule() );
	SetSchemeID( pCircuitSchemeHMX->GetIDPtr().ID );
	SetFlagCircuitIndex( pHMX->IsCircuitIndex() );

	// Define the pipes.
	// Recuperate/Update pipes depending on the following scheme :
	// Pipes in HySelect		Pipes in TA Scope		Comment
	// ---------------------------------------------------------------
	//			1						1				Keep pipes from HySelect
	//			1						0				Keep pipes from HySelect
	//			0						1				Create new pipes in HySelect + copy pipes from TA-Scope (if possible)
	//			0						0				Create new pipes in HySelect
	CPipes *pDistSupplyPipes = NULL;
	CPipes *pDistReturnPipes = NULL;
	CPipes *pCircPipes = NULL;
	CPipes *pSecondaryCircuitPipes = NULL;
	CreatePipes();

	// Define the water characteristics in the pipes.
	InitAllWaterCharWithParent( pclParent );

	// Try to recover pipes from TA-SCOPE; if doesn't exist take the default pipe from Tech Param.
	pDistSupplyPipes = GetpDistrSupplyPipe();

	if( NULL != pDistSupplyPipes )
	{
		// Verify if a pipe series already exist, in that case, keep it.
		if( NULL != pDistSupplyPipes->GetPipeSeries() )
		{
			pDistSupplyPipes->SetRealQ( GetQ() );
		}
		else
		{
			// Otherwise try to recuperate the pipe series from the TA-SCOPE or create one series depending on preferences.

			CHMXPipe *pHMXPipeDistributionSupply = pHMX->GetpPipe( CHMXPipe::epipeOnDistributionSupply );
			IDPTR IDPtrPipe = _NULL_IDPTR;

			if( NULL != pHMXPipeDistributionSupply && NULL != pHMXPipeDistributionSupply->GetpPipe() )
			{
				IDPtrPipe = TASApp.GetpPipeDB()->Get( pHMXPipeDistributionSupply->GetpPipe()->GetIDPtr().ID );
			}

			if( _T('\0') == *IDPtrPipe.ID )
			{
				CTable *pPipeSeries = (CTable *) pPrjParams->GetpHmCalcParams()->GetPrjParamIDPtr( CPrjParams::PipeDistSupplySerieID ).MP;

				if( NULL != pPipeSeries )
				{
					IDPtrPipe = pPipeSeries->GetFirst();
				}
			}

			// Sanity test.
			ASSERT( IDPtrPipe.DB == TASApp.GetpPipeDB() );

			pDistSupplyPipes->SetIDPtr( IDPtrPipe );

			if( NULL != pHMXPipeDistributionSupply )
			{
				pDistSupplyPipes->SetLength( pHMXPipeDistributionSupply->GetLength() );
				pDistSupplyPipes->SetRealQ( pHMXPipeDistributionSupply->GetFlow() );
			}
			else
			{
				pDistSupplyPipes->SetRealQ( GetQ() );
			}
		}
	}

	pDistReturnPipes = GetpDistrReturnPipe();

	if( NULL != pDistReturnPipes )
	{
		// Verify if a pipe series already exist, in that case, keep it.
		if( NULL != pDistReturnPipes->GetPipeSeries() )
		{
			pDistReturnPipes->SetRealQ( GetQ() );
		}
		else
		{
			// Otherwise try to recuperate the pipe series from the TA-SCOPE or create one series depending on preferences.

			CHMXPipe *pHMXPipeDistributionReturn = pHMX->GetpPipe( CHMXPipe::epipeOnDistributionReturn );
			IDPTR IDPtrPipe = _NULL_IDPTR;

			if( NULL != pHMXPipeDistributionReturn && NULL != pHMXPipeDistributionReturn->GetpPipe() )
			{
				IDPtrPipe = TASApp.GetpPipeDB()->Get( pHMXPipeDistributionReturn->GetpPipe()->GetIDPtr().ID );
			}

			if( _T('\0') == *IDPtrPipe.ID )
			{
				IDPtrPipe = ( (CTable *) pPrjParams->GetpHmCalcParams()->GetPrjParamIDPtr( CPrjParams::PipeDistReturnSerieID ).MP )->GetFirst();
			}

			// Sanity test.
			ASSERT( IDPtrPipe.DB == TASApp.GetpPipeDB() );

			pDistReturnPipes->SetIDPtr( IDPtrPipe );

			if( NULL != pHMXPipeDistributionReturn )
			{
				pDistReturnPipes->SetLength( pHMXPipeDistributionReturn->GetLength() );
				pDistReturnPipes->SetRealQ( pHMXPipeDistributionReturn->GetFlow() );
			}
			else
			{
				pDistReturnPipes->SetRealQ( GetQ() );
			}
		}
	}

	pCircPipes = GetpCircuitPrimaryPipe();

	if( NULL != pCircPipes )
	{
		// Verify if a pipe series already exist, in that case, keep it.
		if( NULL != pCircPipes->GetPipeSeries() )
		{
			pCircPipes->SetRealQ( GetQ() );
		}
		else
		{
			// Otherwise try to recuperate the pipe series from the TA-SCOPE or create one series depending on preferences.

			CHMXPipe *pHMXPipeCircuit = pHMX->GetpPipe( CHMXPipe::epipeOnCircuit );
			IDPTR IDPtrPipe = _NULL_IDPTR;

			if( NULL != pHMXPipeCircuit && NULL != pHMXPipeCircuit->GetpPipe() )
			{
				IDPtrPipe = TASApp.GetpPipeDB()->Get( pHMXPipeCircuit->GetpPipe()->GetIDPtr().ID );
			}

			if( _T('\0') == *IDPtrPipe.ID )
			{
				IDPtrPipe = ( (CTable *) pPrjParams->GetpHmCalcParams()->GetPrjParamIDPtr( CPrjParams::PipeCircSerieID ).MP )->GetFirst();
			}

			// Sanity test.
			ASSERT( IDPtrPipe.DB == TASApp.GetpPipeDB() );

			pCircPipes->SetIDPtr( IDPtrPipe );

			if( NULL != pHMXPipeCircuit )
			{
				pCircPipes->SetLength( pHMXPipeCircuit->GetLength() );
				pCircPipes->SetRealQ( pHMXPipeCircuit->GetFlow() );
			}
			else
			{
				pCircPipes->SetRealQ( GetQ() );
			}
		}
	}

	pSecondaryCircuitPipes = GetpCircuitSecondaryPipe();

	if( NULL != pSecondaryCircuitPipes )
	{
		// Verify if a pipe series already exist, in that case, keep it.
		if( NULL != pSecondaryCircuitPipes->GetPipeSeries() )
		{
			pSecondaryCircuitPipes->SetRealQ( GetQ() );
		}
		else
		{
			// Otherwise try to recuperate the pipe series from the TA-SCOPE or create one series depending on preferences.

			CHMXPipe *pHMXPipeSecondaryCircuit = pHMX->GetpPipe( CHMXPipe::epipeOnSecondaryCircuit );
			IDPTR IDPtrPipe = _NULL_IDPTR;

			if( NULL != pHMXPipeSecondaryCircuit && NULL != pHMXPipeSecondaryCircuit->GetpPipe() )
			{
				IDPtrPipe = TASApp.GetpPipeDB()->Get( pHMXPipeSecondaryCircuit->GetpPipe()->GetIDPtr().ID );
			}

			if( _T('\0') == *IDPtrPipe.ID )
			{
				IDPtrPipe = ( (CTable *)pPrjParams->GetpHmCalcParams()->GetPrjParamIDPtr( CPrjParams::PipeCircSerieID ).MP )->GetFirst();
			}

			// Sanity test.
			ASSERT( IDPtrPipe.DB == TASApp.GetpPipeDB() );

			pSecondaryCircuitPipes->SetIDPtr( IDPtrPipe );

			if( NULL != pHMXPipeSecondaryCircuit )
			{
				pSecondaryCircuitPipes->SetLength( pHMXPipeSecondaryCircuit->GetLength() );
				pSecondaryCircuitPipes->SetRealQ( pHMXPipeSecondaryCircuit->GetFlow() );
			}
			else
			{
				pSecondaryCircuitPipes->SetRealQ( GetQ() );
			}
		}
	}

	// Define the terminal unit.
	CreateTermUnit();
	GetpTermUnit()->SetQ( GetQ() );
	GetpTermUnit()->SetVirtual( pHMX->IsaModule() );
}

void CDS_HydroMod::TransferMeasFromHMX( CDS_HydroModX *pHMX )
{
	// Proceed for Primary and secondary side
	CDS_HydroModX::eLocate locate = CDS_HydroModX::InPrimary;

	for( int k = 0; k < 2; k++ )
	{
		if( k == 1 )
		{
			locate = CDS_HydroModX::InSecondary;
		}

		CDS_HydroModX::CMeasData *pMDx = pHMX->GetpMeasData( locate );

		if( NULL == pMDx )
		{
			continue;
		}

		CDS_HydroModX::CBase *pMeasValve = pHMX->GetpMeasValve( locate );
		// Define Bv and Cv valves from the HMX.
		CDS_HydroModX::CBV *pHMXBv = pHMX->GetpBV( locate );
		CDS_HydroModX::CCv *pHMXCv = pHMX->GetpCv();

		// Set the m_CurrentMeasDataIndex.
		SetCurrentMeasDataIndex( 0 );

		// Set the m_MeasDataDateTimeVector.
		if( 0 != pMDx->GetMeasDateTime() || true == pHMX->IsTABalMeasCompleted( locate )
				|| CDS_HydroModX::etabmTABalPlusStarted == pHMX->GetTABalanceMode() || CDS_HydroModX::etabmTABalPlus == pHMX->GetTABalanceMode() )
		{
			// Run all measured data in current HydroMod (TASelect) to verify if measure in HydroModX (TAScope) is already exist.
			for( unsigned int i = 0; i < GetMeasDataSize(); i++ )
			{
				CMeasData *pMeasData = GetpMeasData( i );

				if( pMeasData->GetMeasDateTime() == pMDx->GetMeasDateTime( ) &&  pMeasData->GetTABalDateTime() == pMDx->GetTABalDateTime( ) )
				{
					return;
				}
			}

			CMeasData *pMeasData = CreateMeasData();

			if( NULL == pMeasData )
			{
				return;
			}

			pMeasData->SetMeasDateTime( pMDx->GetMeasDateTime() );
			pMeasData->SetTABalDateTime( pMDx->GetTABalDateTime() );
			pMeasData->SetInstrument( CDS_HydroMod::eInstrumentUsed::TASCOPE );
			pMeasData->SetQMType( pMDx->GetQMType() );
			pMeasData->SetLocate( locate );

			// Set values for the Bv/Cv or PiCv in the CMeasData class...
			if( NULL != pMeasValve && pHMXCv == pMeasValve )
			{
				pMeasData->SetMode( pHMXCv->GetMode() );
				pMeasData->SetTAPID( pHMXCv->GetValveIDPtr().ID );
				pMeasData->SetCurOpening( pHMXCv->GetCurOpening() );
				pMeasData->SetDesignQ( pHMXCv->GetDesignFlow() );
				pMeasData->SetDesignOpening( pHMXCv->GetDesignOpening() );

				// TA-Diagnostic values.
				pMeasData->SetTaBalOpening_1( pHMXCv->GetTABalOpening_1() );
				pMeasData->SetTaBalMeasDp_1( pHMXCv->GetTABalMeasDp_1() );
				pMeasData->SetTaBalOpeningResult( pHMXCv->GetTABalResultOpening() );
			}
			else if( NULL != pMeasValve && pHMXBv == pMeasValve )
			{
				pMeasData->SetMode( pHMXBv->GetMode() );
				pMeasData->SetTAPID( pHMXBv->GetValveIDPtr().ID );
				pMeasData->SetKv( pHMXBv->GetKv() );
				pMeasData->SetCurOpening( pHMXBv->GetCurOpening() );
				pMeasData->SetDesignQ( pHMXBv->GetDesignFlow() );
				pMeasData->SetDesignOpening( pHMXBv->GetDesignOpening() );

				// To improved when it will be implemented :
				// m_dDesignDT, m_dDesignPower, m_dDesignDp

				pMeasData->SetTaBalOpening_1( pHMXBv->GetTABalOpening_1() );
				pMeasData->SetTaBalMeasDp_1( pHMXBv->GetTABalMeasDp_1() );
				pMeasData->SetTaBalOpeningResult( pHMXBv->GetTABalResultOpening() );
				pMeasData->SetDpPVTABal( pHMX->GetTABalMeasDp_2( locate ) );
			}

			// HYS-1716: We put the right water characteristic regarding the location of the valve measured.
			// Remark: water characteristics are alredy set because we called the "CDS_HydroMod::CleanAndPrepareHM" and
			// the "CDS_HmInj::TransferOtherInfoFromHMX" methods just before calling this method.
			if( CDS_HydroModX::InPrimary == locate )
			{
				pMeasData->CopyWCData( GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary ) );
			}
			else
			{
				pMeasData->CopyWCData( GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary ) );
			}

			pMeasData->SetMeasDp( pMDx->GetMeasDp() );
			pMeasData->SetMeasDpL( pMDx->GetMeasDpL() );

			for( int i = CDS_HydroModX::eDTS::DTSUndef + 1; i < CDS_HydroModX::eDTS::LastDTS; i++ )
			{
				pMeasData->SetT( ( CDS_HydroModX::eDTS ) i, pHMX->GetWaterTemp( i, locate ) );
			}

			pMeasData->SetUsedDTSensor( 0, pHMX->GetDTSensor( 0, locate ) );
			pMeasData->SetUsedDTSensor( 1, pHMX->GetDTSensor( 1, locate ) );
			pMeasData->SetFlagBalanced( pHMX->GetTABalanceMode( ) );
			pMeasData->SetFlagTaBalMeasCompleted( pHMX->IsTABalMeasCompleted( locate ) );

			// Redefine the water char temperature if real temp was measured.
			CDS_HydroModX::eDTS edts = pHMX->GetWaterTempRef( locate );

			if( edts != CDS_HydroModX::eDTS::InternalReference )
			{
				double dRealTemp = pHMX->GetWaterTemp( ( int ) edts, locate );
				pMeasData->GetpWC()->UpdateFluidData( dRealTemp );
			}

			// Retrieve distribution and circuit Dp
			pMeasData->SetTABalDistribDp( pHMX->GetTaBalPipeDp( locate ) );
			pMeasData->SetTABalCircDp( pHMX->GetTaBalUnitDp( locate ) );

			// Retrieve distribution and circuit pipes and terminal unit.
			pMeasData->SetpDistributionPipe( pHMX->GetpPipe( CHMXPipe::epipeOnDistributionSupply ) );
			pMeasData->SetpCircuitPipe( pHMX->GetpPipe( CHMXPipe::epipeOnCircuit ) );
			pMeasData->SetpTerminalUnit( pHMX->GetpTU() );
			pMeasData->SetTaBalMeasDp_2( pHMX->GetTABalMeasDp_2( locate ) );
		}
	}
}

void CDS_HydroMod::TransferValvesInfoFromHMX( CDS_HydroModX *pHMX, int iHMObjToCheck )
{
	m_bComputeAllowed = false;

	// Variables.
	CDS_HydroModX::CDpC *pHMXDpC = pHMX->GetpDpC();
	CDS_HydroModX::CBV *pHMXBv = pHMX->GetpBV( CDS_HydroModX::eLocate::InPrimary );
	CDS_HydroModX::CBV *pHMXBvSec = pHMX->GetpBV( CDS_HydroModX::eLocate::InSecondary );
	CDS_HydroModX::CCv *pHMXCv = pHMX->GetpCv();
	CDS_HydroModX::CShutoffValve *pHMXSvSupply = pHMX->GetpShutoffValve( ShutoffValveLocSupply );
	CDS_HydroModX::CShutoffValve *pHMXSvReturn = pHMX->GetpShutoffValve( ShutoffValveLocReturn );
	CDS_HydroModX::CSmartControlValve *pHMXSmartControlValve = pHMX->GetpSmartControlValve();
	CDS_HydroModX::CSmartDpC *pHMXSmartDpC = pHMX->GetpSmartDpC();

	// Define the DpC valve if a pHMXDpC exist.
	if( ( eHMObj::eDpC == ( iHMObjToCheck & eHMObj::eDpC ) ) && NULL != pHMXDpC )
	{
		CDS_HydroMod::CDpC *pDpC = GetpDpC();

		if( NULL != pDpC )
		{
			pDpC->SetIDPtr( pHMXDpC->GetValveIDPtr() );
			pDpC->SetBestDpCIDPtr( pHMXDpC->GetValveIDPtr() );
			pDpC->SetMvLoc( pHMXDpC->GetMvLoc() );
			pDpC->SetMvLocLocked( false );
		}
	}

	bool bCBIValueSet = false;

	// Define the Bv valve if a pHMXBv exist.
	if( NULL != pHMXBv )
	{
		if( eHMObj::eBVprim == ( iHMObjToCheck & eHMObj::eBVprim ) )
		{
			// Complete information for the newly created balancing valve.
			CDS_HydroMod::CBV *pBv = GetpBv();

			if( NULL != pBv )
			{
				pBv->SetIDPtr( pHMXBv->GetValveIDPtr() );
				pBv->SetSetting( pHMXBv->GetDesignOpening() );
				pBv->SetLocate( CDS_HydroMod::eHMObj::eBVprim );
			}
		}

		// Additional information that goes directly to the HM For CBI compatibility...
		CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( TASApp.GetpTADB()->Get( pHMXBv->GetValveIDPtr().ID ).MP );

		if( NULL != pTAP )
		{
			// Product exist for CBI?
			if( *pTAP->GetCBIType() )
			{
				SetCBIValveID( pTAP->GetIDPtr().ID );
			}
		}

		// For CBI compatibility.
		// We have 6 balancing types in TA-Scope: BV, BVCV, DPC, PICV, DPC_CV and DPC_BCV (see "CDS_HydroModX::eHMXBalType" in
		// the "DataStruct.h" file). In case of BVCV, PICV, DPC_CV and DPC_BCV we have no balancing valve (saved in 'pHMXBv').
		// In that case we can set Q, Dp and presetting we this valve.
		// For all other cases, it must be done below for the control valve.
		SetQDesign( pHMXBv->GetDesignFlow() );

		CWaterChar *pclWaterChar = GetpWaterChar( eHMObj::eBVprim );
		ASSERT( NULL != pclWaterChar );

		if( NULL != pclWaterChar )
		{
			SetDpDesign( CalcDp( pHMXBv->GetDesignFlow(), pHMXBv->GetDesignOpening(), pclWaterChar->GetDens() ) );
			SetPresetting( pHMXBv->GetDesignOpening() );
		}
		
		bCBIValueSet = true;

		if( CDS_HydroModX::eMode::Kvmode == pHMXBv->GetMode() )
		{
			SetVDescrType( enum_VDescriptionType::edt_KvCv );
			SetKvCv( pHMXBv->GetKv() );
		}
		else if( CDS_HydroModX::eMode::TAmode == pHMXBv->GetMode() )
		{
			SetVDescrType( enum_VDescriptionType::edt_TADBValve );
		}
		else
		{
			ASSERT( 0 );
		}
	}

	// Define the Ctrl valve if a pHMXCv exist.
	bool bComputeCV = false;

	if( eHMObj::eCV == ( iHMObjToCheck & eHMObj::eCV ) )
	{
		if( NULL != pHMXCv )
		{
			CWaterChar *pclWaterChar = GetpWaterChar( eHMObj::eCV );
			ASSERT( NULL != pclWaterChar );

			// Complete information for the newly created control valve.
			CDS_HydroMod::CCv *pHMCv = GetpCV();

			if( NULL != pHMCv )
			{
				if( CDS_HydroModX::Kvmode == pHMXCv->GetMode() )
				{
					pHMCv->SetTACv( false );
					CDS_TechnicalParameter *pTechParam = TASApp.GetpTADS()->GetpTechParams();
					ASSERT( NULL != pTechParam );

					if( NULL != pTechParam && NULL != pclWaterChar )
					{
						pHMCv->SetKvs( CalcKv( pHMXCv->GetDesignFlow(), pTechParam->GetCVMinDpProp(), pclWaterChar->GetDens() ) );
					}
				}
				else if( CDS_HydroModX::TAmode == pHMXCv->GetMode() )
				{
					pHMCv->SetTACv( true );
				}

				pHMCv->SetCvLocate( pHMXCv->GetCvLocate() );
				pHMCv->SetCvIDPtr( pHMXCv->GetValveIDPtr() );
				pHMCv->SetSetting( pHMXCv->GetDesignOpening() );
				pHMCv->SetQ( pHMXCv->GetDesignFlow(), false );

				CDB_ControlValve *pCvDB = dynamic_cast<CDB_ControlValve *>( pHMCv->GetCvIDPtr().MP );

				if( NULL != pCvDB )
				{
					CDB_ControlProperties *pCtrlProp = pCvDB->GetCtrlProp();

					if( NULL != pCtrlProp )
					{
						pHMCv->SetCtrlType( pCtrlProp->GetCtrlType() );
					}
				}
			}

			// Additional information that goes directly to the HM.

			if( false == bCBIValueSet )
			{
				SetQDesign( pHMXCv->GetDesignFlow() );
				SetPresetting( pHMXCv->GetDesignOpening() );

				if( NULL != pclWaterChar )
				{
					SetDpDesign( CalcDp( pHMXCv->GetDesignFlow(), pHMXCv->GetDesignOpening(), pclWaterChar->GetDens() ) );
				}

				if( CDS_HydroModX::eMode::Kvmode == pHMXCv->GetMode() )
				{
					SetVDescrType( enum_VDescriptionType::edt_KvCv );
				}
				else if( CDS_HydroModX::eMode::TAmode == pHMXCv->GetMode() )
				{
					SetVDescrType( enum_VDescriptionType::edt_TADBValve );
				}
				else
				{
					ASSERT( 0 );
				}
			}
		}
		else if( NULL != GetpCV() )
		{
			bComputeCV = true;
		}
	}

	if( ( eHMObj::eShutoffValveSupply == ( iHMObjToCheck & eHMObj::eShutoffValveSupply ) ) && NULL != pHMXSvSupply )
	{
		// Complete information for the newly created shut-off valve.
		CDS_HydroMod::CShutoffValve *pHMShutoffValve = GetpShutoffValve( eShutoffValveSupply );

		if( NULL != pHMShutoffValve )
		{
			pHMShutoffValve->SetIDPtr( pHMXSvSupply->GetValveIDPtr() );

			// TODO: check if there will be other data to set here !!!
		}
	}

	if( ( eHMObj::eShutoffValveReturn == ( iHMObjToCheck & eHMObj::eShutoffValveReturn ) ) && NULL != pHMXSvReturn )
	{
		// Complete information for the newly created shut-off valve.
		CDS_HydroMod::CShutoffValve *pHMShutoffValve = GetpShutoffValve( eShutoffValveReturn );

		if( NULL != pHMShutoffValve )
		{
			pHMShutoffValve->SetIDPtr( pHMXSvReturn->GetValveIDPtr() );

			// TODO: check if there will be other data to set here !!!
		}
	}

	if( ( eHMObj::eSmartControlValve == ( iHMObjToCheck & eHMObj::eSmartControlValve ) ) && NULL != pHMXSmartControlValve )
	{
		// Complete information for the newly created smart control valve.
		CDS_HydroMod::CSmartControlValve *pHMSmartControlValve = GetpSmartControlValve();

		if( NULL != pHMSmartControlValve )
		{
			pHMSmartControlValve->SetIDPtr( pHMXSmartControlValve->GetValveIDPtr() );

			// TODO: check if there will be other data to set here !!!
		}
	}

	if( ( eHMObj::eSmartDpC == ( iHMObjToCheck & eHMObj::eSmartDpC ) ) && NULL != pHMXSmartDpC )
	{
		// Complete information for the newly created smart differential pressure controller.
		CDS_HydroMod::CSmartDpC *pHMSmartDpC = GetpSmartDpC();

		if( NULL != pHMSmartDpC )
		{
			pHMSmartDpC->SetIDPtr( pHMXSmartDpC->GetValveIDPtr() );

			// TODO: check if there will be other data to set here !!!
		}
	}

	if( NULL != GetpSchcat() && true == GetpSchcat()->IsInjection() )
	{
		if( eHMObj::eBVsec == ( iHMObjToCheck & eHMObj::eBVsec ) )
		{
			if( NULL != pHMX->GetpBV( CDS_HydroModX::InSecondary ) )
			{
				CDS_HydroMod::CBV *pBvSec = GetpSecBv();

				if( NULL != pBvSec )
				{
					CDS_HydroModX::CBV *pHMXBvSec = pHMX->GetpBV( CDS_HydroModX::InSecondary );
					pBvSec->SetIDPtr( pHMXBvSec->GetValveIDPtr() );
					pBvSec->SetSetting( pHMXBvSec->GetDesignOpening() );
					pBvSec->SetLocate( CDS_HydroMod::eHMObj::eBVsec );
				}
			}
			else if( NULL != GetpSecBv() )
			{
				// If no secondary balancing valve in TA-Scope, we must delete secondary valve if exist in HySelect.
				// Remark: In 'CTableHM::LoadFromHMXTable' method we call 'CreateNewHM'. This method will create all needed valves in regards to
				//         the circuit scheme. If it's a injection circuit 'CDS_HmInj::Init' method is called and it creates the secondary valve
				//         by default.
				DeleteBv( GetppSecBv() );
			}
		}

		CDS_HmInj *pHMInj = dynamic_cast<CDS_HmInj *>( this );

		if( NULL != pHMInj )
		{
			if( NULL != pHMInj->GetpTermUnit() )
			{
				pHMInj->GetpTermUnit()->SetQ( pHMX->GetDesignFlow( CDS_HydroModX::InSecondary ), false );
			}
		}
	}
	else if( NULL != GetpTermUnit() )
	{
		GetpTermUnit()->SetQ( pHMX->GetDesignFlow( CDS_HydroModX::InPrimary ), false );
	}
	else
	{
		ASSERT( 0 );
	}

	if( eHMObj::eBVbyp == ( iHMObjToCheck & eHMObj::eBVbyp ) )
	{
		if( NULL != pHMX->GetpBV( CDS_HydroModX::InBypass ) )
		{
			CDS_HydroMod::CBV *pBvByp = GetpBypBv();

			if( NULL != pBvByp )
			{
				CDS_HydroModX::CBV *pHMXBvByp = pHMX->GetpBV( CDS_HydroModX::InBypass );
				pBvByp->SetIDPtr( pHMXBvByp->GetValveIDPtr() );
				pBvByp->SetSetting( pHMXBvByp->GetDesignOpening() );
				pBvByp->SetLocate( CDS_HydroMod::eHMObj::eBVbyp );
			}
		}
		else if( NULL != GetpBypBv() )
		{
			// If no bypass balancing valve in TA-Scope, we must delete bypass balancing valve if exist in HySelect.
			// Remark: In 'CTableHM::LoadFromHMXTable' method we call 'CreateNewHM'. This method will create all needed valves in regards to
			//         the circuit scheme. If it's a 3 way dividing circuit 'CDS_Hm3W::Init' method is called and it creates the bypass valve
			//         by default.
			DeleteBv( GetppBypBv() );
		}
	}

	// Must be done after having updated Q.
	if( true == bComputeCV )
	{
		// TA-Scope doesn't create control valve except if this one is combined with a balancing or/and a differential pressure controller part.
		// Combined valve can be TBV-C, TBV-CM, TA-FUSION-C, TA-FUSION-P, TA-COMPACT-DP, ...
		// For example: Add a "2-way control circuit" circuit type with "Manual balancing" balancing type, TA-Scope will create the balancing valve
		// but not the control valve.
		// When importing in HySelect, 'CTableHM::LoadFromHMXTable' method will call 'CreateNewHM'. This last method checks circuit scheme and creates
		// corresponding valves included ones that are not created in TA-Scope as it is the case in the example with the control valve.
		// To let HySelect to choose the best control valve, we have to unlock this valve.
		// All valves have been locked in the 'CDS_HydroMod::LoadFromHMX' calling method. This is why we unlock here.
		SetLock( CDS_HydroMod::eHMObj::eCV, false, false, true );
		EnableComputeHM( true );
		ComputeHM( eceCV );
		EnableComputeHM( false );
	}

	m_bComputeAllowed = true;

	m_bComputeAllInCourse = false;
}

bool CDS_HydroMod::LoadFromHMX( CDS_HydroModX *pHMX, CTable *pclParent, int iLevel, bool bImportInExistingHM )
{
	if( NULL == pHMX )
	{
		return false;
	}

	// Deny the possibility to compute the module during the transfer process.
	EnableComputeHM( false );

	// Delete nonexistent valves, define water characteristics, name, position, scheme, pipes, TU, etc.
	CleanAndPrepareHM( pHMX, pclParent, iLevel, bImportInExistingHM );

	// Lock all by default.
	SetLock( CDS_HydroMod::eHMObj::eALL, true, false, true );

	if( false == bImportInExistingHM )
	{
		// If it's a new circuit that we import, we must unlock pipe to allow HySelect to size them.
		// Remark: Because when creating a network in TA-Scope there is no pipe and by default when
		// importing in HySelect the pipe size is set to DN 10.
		SetLock( CDS_HydroMod::eHMObj::eAllPipes, false, false, true );
		// 		EnableComputeHM( true );
		// 		ComputeHM( ecePipeSize );
		// 		EnableComputeHM( false );
	}

	// Allow to inherited class (ie 'CDS_Hm2W', 'CDS_Hm3W' and so on) to fill specific data.
	// Remark: the call of this method must be done before 'TransferMeasFromHMX'. Because for example 'CDS_HmInj' computes
	//         primary flow and needs secondary and return temperatures that are loaded when calling 'TransferOtherInfoFromHMX'.
	TransferOtherInfoFromHMX( pHMX );

	// Transfer all information in valves.
	TransferValvesInfoFromHMX( pHMX );

	// Transfer all measurements into corresponding valves.
	TransferMeasFromHMX( pHMX );

	// Allow the possibility to compute the module.
	EnableComputeHM( true );

	return true;
}

// Refactoring old scheme ID
// Scheme ID before 27/05/2010 was 59 and differentiated according type _TA; _516; _50
// due to the implementation of DynCirchScheme this differentiation will be done directly by the DynCirchScheme engine.
// This function is used to convert SchemeID stored in tsp files before 27/05/2010
CString CDS_HydroMod::ConvertOldSchemeID( TCHAR *pBuf )
{
	CString str = pBuf;

	if( true == str.IsEmpty() )
	{
		return str;
	}

	// Be care full string order in the array is important.
	CStringArray arStr2Remove;
	arStr2Remove.Add( _T("B_TA") );
	arStr2Remove.Add( _T("_TA") );
	arStr2Remove.Add( _T("B_516") );
	arStr2Remove.Add( _T("_516") );
	arStr2Remove.Add( _T("B_50") );
	arStr2Remove.Add( _T("_50") );
	arStr2Remove.Add( _T("_50") );

	for( int i = 0; i < arStr2Remove.GetCount(); i++ )
	{
		int iPos = str.Find( arStr2Remove[i] );

		if( -1 != iPos )
		{
			str.Delete( iPos, arStr2Remove[i].GetLength() );
			break;	// abort the loop string as been found
		}
	}

	int iPos = str.Find( _T("3WDU_") );

	if( -1 != iPos )
	{
		str.Delete( 3 );
	}

	#ifdef DEBUG

	if( str == L"3WINJ_MIX_DECBYP1" || str == L"3WINJ_MIX_DECBYP2" )
	{
		str = str + L"_BV";
	}

	// Verify that new object exist into the database
	IDPTR idptr = TASApp.GetpTADB()->Get( (LPCTSTR) str );
	ASSERT( NULL != idptr.MP );
	#endif
	return str;
}

#define CDS_HYDROMOD_VERSION	12
// Versuin 12: HYS-1930: Add bypass pipe.
// Version 11: HYS-1959: we add smart differential pressure controller.
// Version 10: HYS-1716: The water characteristics are now set in the pipes.
// Version 9: 2021-06-11: HYS-1677: we add smart control valve.
void CDS_HydroMod::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_VERSION;
	WriteData<>( outf, Version );

	// Base class.
	CTable::Write( outf );

	// Infos.
	WriteString( outf, GetHMName() );
	TCHAR buf[CSTRING_LINE_SIZE_MAX];
	_tcsncpy_s( buf, SIZEOFINTCHAR( buf ), (LPCTSTR) m_strDescription, CSTRING_LINE_SIZE_MAX - 1 );
	WriteString( outf, buf );

	WriteData<>( outf, m_iLevel );
	WriteData<>( outf, m_usUniqueID );
	WriteData<>( outf, m_iPosition );
	WriteData<>( outf, m_bModule );

	// Version 5.
	WriteString( outf, _T("") );						// Deprecated string
	WriteString( outf, m_CBIValveIDPtr.ID );

	// Version 5.
	WriteString( outf, m_tcCBIType );
	WriteString( outf, m_tcCBISize );
	WriteData<>( outf, m_dQDesign );
	WriteData<>( outf, m_dDpDesign );
	WriteData<>( outf, m_dPresetting );

	WriteData<>( outf, m_eVDescrType );
	WriteData<>( outf, m_dKvCv );
	WriteString( outf, m_SchemeIDPtr.ID );
	WriteString( outf, m_strRemL1 );
	WriteData<>( outf, m_dHAvail );
	WriteData<>( outf, m_bIsCircuitIndex );
	WriteData<>( outf, m_dHPressInt );
	WriteData<>( outf, m_usLock );
	
	bool bFlag = ( NULL != m_pCircuitPrimaryPipe ) ? true : false;
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pDistrSupplyPipe->Write( outf );
		m_pCircuitPrimaryPipe->Write( outf );
	}

	bFlag = ( NULL != m_pBv ) ? true : false;
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pBv->Write( outf );
	}

	bFlag = ( NULL != m_pBypBv ) ? true : false;
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pBypBv->Write( outf );
	}

	bFlag = ( NULL != m_pSecBv ) ? true : false;
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pSecBv->Write( outf );
	}

	bFlag = ( NULL != m_pDpC ) ? true : false;
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pDpC->Write( outf );
	}

	bFlag = ( NULL != m_pTermUnit ) ? true : false;
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pTermUnit->Write( outf );
	}

	bFlag = ( NULL != m_pCv ) ? true : false;
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pCv->Write( outf );
	}

	bFlag = ( NULL != m_pShutoffValveSupply ) ? true : false;
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pShutoffValveSupply->Write( outf );
	}

	bFlag = ( NULL != m_pShutoffValveReturn ) ? true : false;
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pShutoffValveReturn->Write( outf );
	}

	// Version 10: the water characteristics are now set in the pipes.
	// WriteString( outf, m_WaterCharID );
	
	int iSize = m_MeasDataDateTimeVector.size();
	WriteData<>( outf, iSize );

	for( int i = 0; i < iSize; i++ )
	{
		CMeasData *pMeasData = GetpMeasData( i );
		ASSERT( pMeasData );
		pMeasData->Write( outf );
	}

	WriteData<>( outf, m_bHMCalcMode );
	WriteData<>( outf, m_dDiversityFactor );

	bFlag = IsPumpExist();
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pPump->Write( outf );
	}

	bFlag = ( NULL != m_pCircuitSecondaryPipe ) ? true : false;
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pCircuitSecondaryPipe->Write( outf );
	}

	// Version 10: the temperatures are now set in the pipes.
	// WriteData<>( outf, m_dSupplyTemp );
	// WriteData<>( outf, m_dReturnTemp );

	// Version 8.
	WriteData<>( outf, m_eReturnType );

	bFlag = ( NULL != m_pDistrReturnPipe ) ? true : false;
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pDistrReturnPipe->Write( outf );
	}

	// Version 9.
	bFlag = ( NULL != m_pSmartControlValve ) ? true : false;
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pSmartControlValve->Write( outf );
	}

	// Version 11.
	bFlag = ( NULL != m_pSmartDpC ) ? true : false;
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pSmartDpC->Write( outf );
	}

	// Version 12.
	bFlag = ( NULL != m_pCircuitBypassPipe ) ? true : false;
	WriteData<>( outf, bFlag );

	if( true == bFlag )
	{
		m_pCircuitBypassPipe->Write( outf );
	}
}

bool CDS_HydroMod::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_VERSION;
	ReadData<>( inpf, Version );

	if( Version < 1 || Version > CDS_HYDROMOD_VERSION )
	{
		return false;
	}

	// Base class.
	if( false == CTable::Read( inpf ) )
	{
		return false;
	}

	// Infos.
	bool bFlag;
	ReadString( inpf, m_HMName, sizeof( m_HMName ) );
	ReadString( inpf, m_strDescription );
	ReadData<>( inpf, m_iLevel );
	ReadData<>( inpf, m_usUniqueID );
	ReadData<>( inpf, m_iPosition );
	ReadData<>( inpf, m_bModule );
	
	TCHAR buf[CSTRING_LINE_SIZE_MAX];
	ReadString( inpf, buf, sizeof( buf ) );
	m_strDeprecatedSelThingID = buf;

	if( Version >= 5 )
	{
		ReadString( inpf, buf, sizeof( buf ) );
		SetCBIValveID( buf );
	}

	// We have to keep a copy of CBI valve, original'ones will be erased during the BV reading
	IDPTR IdptrValveCBI = GetCBIValveIDPtr();
	ReadString( inpf, m_tcCBIType, sizeof( m_tcCBIType ) );
	ReadString( inpf, m_tcCBISize, sizeof( m_tcCBISize ) );
	ReadData<>( inpf, m_dQDesign );
	ReadData<>( inpf, m_dDpDesign );
	ReadData<>( inpf, m_dPresetting );

	if( Version < 4 )
	{
		double dSettingTABal1, dQTABal1, dDpTABal1, dDpTABal2, dDpPvTABal, dSettingf, dQf, dDpf;
		ReadData<>( inpf, dSettingTABal1 );
		ReadData<>( inpf, dQTABal1 );
		ReadData<>( inpf, dDpTABal1 );
		ReadData<>( inpf, dDpTABal2 );
		ReadData<>( inpf, dDpPvTABal );
		ReadData<>( inpf, dSettingf );
		ReadData<>( inpf, dQf );
		ReadData<>( inpf, dDpf );

		if( dSettingTABal1 > 0.0 || dQTABal1 > 0.0 || dDpTABal1 > 0.0 || dDpTABal2 > 0.0 || dDpPvTABal > 0.0 || dSettingf > 0.0 || dQf > 0.0 || dDpf > 0.0 )
		{
			CMeasData *pMD = CreateMeasData();			// Old CBI measurement 01/01/1970

			if( NULL != pMD )
			{
				pMD->SetTaBalOpening_1( dSettingTABal1 );
				pMD->SetTaBalMeasDp_1( dDpTABal1 );
				pMD->SetTaBalMeasDp_2( dDpTABal2 );
				pMD->SetTaBalOpeningResult( dSettingf );
				pMD->SetDpPVTABal( dDpPvTABal );
				pMD->Setqf( dQf );
				pMD->SetqTABal1( dQTABal1 );
				pMD->SetMeasDp( dDpf );
				pMD->SetDesignQ( m_dQDesign );

				for( int iCurDTS = CDS_HydroModX::eDTS::DTS2onDPS; iCurDTS < CDS_HydroModX::eDTS::LastDTS; iCurDTS++ )
				{
					pMD->SetT( ( CDS_HydroModX::eDTS ) iCurDTS, -273.15 );
				}

				if( dQf > 0.0 )
				{
					pMD->SetQMType( CDS_HydroModX::eQMtype::QMFlow );
				}
				else if( 0.0 == dQf && dDpf > 0.0 )
				{
					pMD->SetQMType( CDS_HydroModX::eQMtype::QMDp );
				}
				else
				{
					pMD->SetQMType( CDS_HydroModX::eQMtype::QMundef );
				}
			}
		}
	}

	ReadData<>( inpf, m_eVDescrType );
	ReadData<>( inpf, m_dKvCv );

	// HYS-2025: We need to know if pipe are created for test condition on version 12.
	bool bIsPipesCreated = false;
	if( Version > 1 )
	{
		ReadString( inpf, buf, sizeof( buf ) );
		// Convert the old SchemeID with the new one.
		CString str = ConvertOldSchemeID( buf );
		SetSchemeID( str );
		ReadString( inpf, buf, sizeof( buf ) );
		m_strRemL1 = buf;
		ReadData<>( inpf, m_dHAvail );
		ReadData<>( inpf, m_bIsCircuitIndex );
		ReadData<>( inpf, m_dHPressInt );
		ReadData<>( inpf, m_usLock );

		ReadData<>( inpf, bFlag );

		if( true == bFlag )
		{
			CreatePipes();

			bIsPipesCreated = true;

			if( false == m_pDistrSupplyPipe->Read( inpf ) )
			{
				return false;
			}

			if( false == m_pCircuitPrimaryPipe->Read( inpf ) )
			{
				return false;
			}
		}

		ReadData<>( inpf, bFlag );

		if( true == bFlag )
		{
			CreateBv( CDS_HydroMod::eHMObj::eBVprim );

			if( false == m_pBv->Read( inpf ) )
			{
				return false;
			}
		}

		// HYS-1732: now that he have read BV map and the 'CDB_CircuitScheme' ID, we can correct.
		if( 0 == GetSchemeID().Compare( _T("PUMP") ) )
		{
			if( NULL != GetpBv() )
			{
				SetSchemeID( _T("PUMP_BV") );
			}
		}

		ReadData<>( inpf, bFlag );

		if( true == bFlag )
		{
			CreateBv( CDS_HydroMod::eHMObj::eBVbyp );

			if( false == m_pBypBv->Read( inpf ) )
			{
				return false;
			}
		}

		ReadData<>( inpf, bFlag );

		if( true == bFlag )
		{
			CreateBv( CDS_HydroMod::eHMObj::eBVsec );

			if( false == m_pSecBv->Read( inpf ) )
			{
				return false;
			}
		}

		ReadData<>( inpf, bFlag );

		if( true == bFlag )
		{
			CreateDpC();

			if( false == m_pDpC->Read( inpf ) )
			{
				return false;
			}
		}

		ReadData<>( inpf, bFlag );

		if( true == bFlag )
		{
			CreateTermUnit();

			if( false == m_pTermUnit->Read( inpf ) )
			{
				return false;
			}
		}

		ReadData<>( inpf, bFlag );

		if( true == bFlag )
		{
			// The cv location will be override by the calling to the 'Read' method below.
			CreateCv( CvLocNone );

			if( false == m_pCv->Read( inpf ) )
			{
				return false;
			}
		}
	}

	if( Version > 2 )
	{
		bool bFlag;
		ReadData<>( inpf, bFlag );

		if( true == bFlag )
		{
			CreateSV( eHMObj::eShutoffValveSupply );

			if( false == GetpShutoffValve( eHMObj::eShutoffValveSupply )->Read( inpf ) )
			{
				return false;
			}
		}

		ReadData<>( inpf, bFlag );

		if( true == bFlag )
		{
			CreateSV( eHMObj::eShutoffValveReturn );

			if( false == GetpShutoffValve( eHMObj::eShutoffValveReturn )->Read( inpf ) )
			{
				return false;
			}
		}

		if( Version < 10 )
		{
			// HYS-1716: Don't use anymore here. Water characteristics are now set in pipes and be
			// initialize when computing the network.
			CString strWaterCharID;
			ReadString( inpf, strWaterCharID );
		}
	}

	// Restore BV in CBI container
	m_CBIValveIDPtr = IdptrValveCBI;

	// Previously the HMCalc mode were define by the TermUnit's existence !!
	// Do the test only here because the Terminal Unit must be read first !!
	if( Version < 4 )
	{
		m_bHMCalcMode = IsTermUnitExist();
	}

	if( Version > 3 )
	{
		int iSize = 0;
		ReadData<>( inpf, iSize );

		for( ; iSize > 0; iSize-- )
		{
			CMeasData *pMeasData = CreateMeasData();
			ASSERT( pMeasData );

			if( false == pMeasData->Read( inpf ) )
			{
				return false;
			}
		}

		ReadData<>( inpf, m_bHMCalcMode );
	}

	if( Version > 5 )
	{
		ReadData<>( inpf, m_dDiversityFactor );
		ReadData<>( inpf, bFlag );

		if( true == bFlag )
		{
			CreatePump();

			if( false == m_pPump->Read( inpf ) )
			{
				return false;
			}
		}
	}

	if( Version > 6 )
	{
		ReadData<>( inpf, bFlag );

		if( true == bFlag )
		{
			// HYS-706 following code that create temporary secondary pipe was created to fix an issue introduced with 3way mixing ciruit.

			bool bToBeDeleted = false;

			if( NULL == m_pCircuitSecondaryPipe )
			{
				m_pCircuitSecondaryPipe = new CPipes( this, eHMObj::eCircuitSecondaryPipe );
				bToBeDeleted = true;
			}

			if( false == m_pCircuitSecondaryPipe->Read( inpf ) )
			{
				return false;
			}

			if( true == bToBeDeleted )
			{
				delete m_pCircuitSecondaryPipe;
				m_pCircuitSecondaryPipe = NULL;
			}
		}

		if( Version < 10 )
		{
			// Version 10: the temperatures before this version were not in the pipes.
			// And for the 'CDS_HydroMod' we done nothing with them !!
			double dDummy = 0.0;
			ReadData<>( inpf, dDummy );
			ReadData<>( inpf, dDummy );
		}
	}

	if( Version > 7 )
	{
		ReadData<>( inpf, m_eReturnType );

		if( ReturnType::Reverse == m_eReturnType )
		{
			// The 'CreatePipes' method has been called just before. But we did not have the return mode yet.
			// So the distribution return pipe has not been created even if it is a reverse return mode.
			m_pDistrReturnPipe = new CPipes( this, eHMObj::eDistributionReturnPipe );

			// Must also change other pipes already read.
			if( NULL != m_pCircuitPrimaryPipe )
			{
				m_pCircuitPrimaryPipe->ChangeReturnType( ReturnType::Reverse );
			}

			if( NULL != m_pDistrSupplyPipe )
			{
				m_pDistrSupplyPipe->ChangeReturnType( ReturnType::Reverse );
			}

			if( NULL != m_pCircuitSecondaryPipe )
			{
				m_pCircuitSecondaryPipe->ChangeReturnType( ReturnType::Reverse );
			}

			if( NULL != m_pCircuitBypassPipe )
			{
				m_pCircuitBypassPipe->ChangeReturnType( ReturnType::Reverse );
			}
		}

		ReadData<>( inpf, bFlag );

		if( true == bFlag )
		{
			// 'm_pDistrReturnPipe' is created above when calling 'CreatePipes'.
			if( false == m_pDistrReturnPipe->Read( inpf ) )
			{
				return false;
			}
		}
	}

	if( Version > 8 )
	{
		ReadData<>( inpf, bFlag );

		if( true == bFlag )
		{
			CreateSmartControlValve( SmartValveLocalization::SmartValveLocSupply );

			if( false == GetpSmartControlValve()->Read( inpf ) )
			{
				return false;
			}
		}
	}

	if( Version > 10 )
	{
		ReadData<>( inpf, bFlag );

		if( true == bFlag )
		{
			CreateSmartDpC( SmartValveLocalization::SmartValveLocSupply );

			if( false == GetpSmartDpC()->Read( inpf ) )
			{
				return false;
			}
		}
	}

	if( Version > 11 )
	{
		ReadData<>( inpf, bFlag );
		ASSERT( false == bFlag || NULL != m_pCircuitBypassPipe );

		if( true == bFlag && NULL != m_pCircuitBypassPipe )
		{
			if( false == m_pCircuitBypassPipe->Read( inpf ) )
			{
				return false;
			}
		}
		// HYS-2025: We delete the optional bypass Bv in 3-way mixing circuit in DB. We do not read m_pCircuitBypassPipe for this circuit.
		// After that we remark that old projects with 3-way mixing circuit no longer open in HySelect. The HYS-2025 is commited in 4.5.1.4. 
		// We should manage old projects with Bv bypass on 3-way mixing circuit when we open them.
		else if( Version == 12 && true == bFlag && _ttoi((TASApp.GetpTADS()->GetDBFileHeader()->GetAppVersionStr())) < 4514 
			&& true == bIsPipesCreated && (NULL != GetpSchcat() && GetpSchcat()->Get3WType() == CDB_CircSchemeCateg::e3wTypeMixing) )
		{
			if (NULL == m_pCircuitBypassPipe)
			{
				m_pCircuitBypassPipe = new CPipes(this, eHMObj::eCircuitBypassPrimaryPipe);
			}

			// For bypass, it's well from Vb -> Va so TEE_DZA.
			m_pCircuitBypassPipe->AddConnectionSingularity(_T("TEE_DZA"));
			m_pCircuitBypassPipe->Read(inpf);
			if (NULL != m_pCircuitBypassPipe)
			{
				delete m_pCircuitBypassPipe;
			}

			m_pCircuitBypassPipe = NULL;
		}
	}

	// Pipe verification, remove circuit pipe if exist
	SetFlagModule( IsaModule() );

	return true;
}

CPipes *CDS_HydroMod::GetpPipe( eHMObj eProductLocation )
{
	CPipes *pclPipe = NULL;
	CAnchorPt::AnchorPtFunc *pAnchorPtFunc = NULL;

	CDB_CircuitScheme *pclCircuitSheme = GetpSch();

	if( NULL == pclCircuitSheme )
	{
		ASSERTA_RETURN( NULL );
	}

	switch( eProductLocation )
	{
		case eCircuitPrimaryPipe:

			if( NULL == GetpCircuitPrimaryPipe() )
			{
				ASSERTA_RETURN( NULL );
			}
			
			pclPipe = GetpCircuitPrimaryPipe();
			break;

		case eCircuitBypassPrimaryPipe:

			if( NULL == GetpCircuitBypassPipe() )
			{
				ASSERTA_RETURN( NULL );
			}

			pclPipe = GetpCircuitBypassPipe();
			break;

		case eCircuitSecondaryPipe:

			if( NULL == GetpCircuitSecondaryPipe() )
			{
				ASSERTA_RETURN( NULL );
			}

			pclPipe = GetpCircuitSecondaryPipe();
			break;

		case eDistributionSupplyPipe:

			if( NULL == GetpDistrSupplyPipe() )
			{
				ASSERTA_RETURN( NULL );
			}

			pclPipe = GetpDistrSupplyPipe();
			break;

		case eBVprim:

			if( NULL == GetpCircuitPrimaryPipe() )
			{
				ASSERTA_RETURN( NULL );
			}
			
			pclPipe = GetpCircuitPrimaryPipe();
			break;

		case eBVbyp:

			if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() )
			{
				// HYS-1930: auto-adapting variable flow decoupling circuit.
				if( NULL == GetpCircuitBypassPipe() )
				{
					ASSERTA_RETURN( NULL );
				}
			
				pclPipe = GetpCircuitBypassPipe();
			}
			else
			{
				pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::BV_Byp );

				if( CAnchorPt::CircuitSide_Primary == pAnchorPtFunc->m_eCircuitSide )
				{
					if( NULL == GetpCircuitPrimaryPipe() )
					{
						ASSERTA_RETURN( NULL );
					}
			
					pclPipe = GetpCircuitPrimaryPipe();
				}
				else
				{
					if( NULL == GetpCircuitSecondaryPipe() )
					{
						ASSERTA_RETURN( NULL );
					}

					pclPipe = GetpCircuitSecondaryPipe();
				}
			}
			
			break;

		case eBVsec:

			if( NULL == GetpCircuitSecondaryPipe() )
			{
				ASSERTA_RETURN( NULL );
			}

			pclPipe = GetpCircuitSecondaryPipe();
			break;

		case eDpC:
			pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::DpC );

			if( CAnchorPt::CircuitSide_Primary == pAnchorPtFunc->m_eCircuitSide )
			{
				if( NULL == GetpCircuitPrimaryPipe() )
				{
					ASSERTA_RETURN( NULL );
				}
			
				pclPipe = GetpCircuitPrimaryPipe();
			}
			else
			{
				if( NULL == GetpCircuitSecondaryPipe() )
				{
					ASSERTA_RETURN( NULL );
				}

				pclPipe = GetpCircuitSecondaryPipe();
			}

			break;

		case eCV:

			if( true == pclCircuitSheme->IsAnchorPtExist( CAnchorPt::eFunc::PICV ) )
			{
				pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::PICV );
			}
			else if( true == pclCircuitSheme->IsAnchorPtExist( CAnchorPt::eFunc::DPCBCV ) )
			{
				pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::DPCBCV );
			}
			else if( true == pclCircuitSheme->IsAnchorPtExist( CAnchorPt::eFunc::ControlValve ) )
			{
				pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::ControlValve );
			}

			if( CAnchorPt::CircuitSide_Primary == pAnchorPtFunc->m_eCircuitSide )
			{
				if( NULL == GetpCircuitPrimaryPipe() )
				{
					ASSERTA_RETURN( NULL );
				}
			
				pclPipe = GetpCircuitPrimaryPipe();
			}
			else
			{
				if( NULL == GetpCircuitSecondaryPipe() )
				{
					ASSERTA_RETURN( NULL );
				}

				pclPipe = GetpCircuitSecondaryPipe();
			}

			break;

		case eShutoffValveSupply:
		case eShutoffValveReturn:
			pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::ShutoffValve );

			if( CAnchorPt::CircuitSide_Primary == pAnchorPtFunc->m_eCircuitSide )
			{
				if( NULL == GetpCircuitPrimaryPipe() )
				{
					ASSERTA_RETURN( NULL );
				}
			
				pclPipe = GetpCircuitPrimaryPipe();
			}
			else
			{
				if( NULL == GetpCircuitSecondaryPipe() )
				{
					ASSERTA_RETURN( NULL );
				}

				pclPipe = GetpCircuitSecondaryPipe();
			}

			break;

		case eDistributionReturnPipe:
			if( NULL == GetpDistrReturnPipe() )
			{
				ASSERTA_RETURN( NULL );
			}

			pclPipe = GetpDistrReturnPipe();
				break;

		case eSmartControlValve:
			pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::SmartControlValve );

			if( CAnchorPt::CircuitSide_Primary == pAnchorPtFunc->m_eCircuitSide )
			{
				if( NULL == GetpCircuitPrimaryPipe() )
				{
					ASSERTA_RETURN( NULL );
				}
			
				pclPipe = GetpCircuitPrimaryPipe();
			}
			else
			{
				if( NULL == GetpCircuitSecondaryPipe() )
				{
					ASSERTA_RETURN( NULL );
				}

				pclPipe = GetpCircuitSecondaryPipe();
			}

			break;

		case eSmartDpC:
			pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::SmartDpC );

			if( CAnchorPt::CircuitSide_Primary == pAnchorPtFunc->m_eCircuitSide )
			{
				if( NULL == GetpCircuitPrimaryPipe() )
				{
					ASSERTA_RETURN( NULL );
				}
			
				pclPipe = GetpCircuitPrimaryPipe();
			}
			else
			{
				if( NULL == GetpCircuitSecondaryPipe() )
				{
					ASSERTA_RETURN( NULL );
				}

				pclPipe = GetpCircuitSecondaryPipe();
			}

			break;
	}

	return pclPipe;
}

CAnchorPt::CircuitSide CDS_HydroMod::GetBypassPipeSide()
{
	if( NULL == GetpSchcat() || NULL == GetpSch() )
	{
		return CAnchorPt::CircuitSide::CircuitSide_Undefined;
	}

	CAnchorPt::CircuitSide eCircuitSide = CAnchorPt::CircuitSide::CircuitSide_Undefined;

	if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() )
	{
		// Auto-adapting variable flow decoupling circuit.
		eCircuitSide = CAnchorPt::CircuitSide::CircuitSide_Primary;
	}
	else if( true == GetpSchcat()->Is2W() && true == GetpSchcat()->IsInjection() )
	{
		// All the 2-way injection circuits.
		eCircuitSide = CAnchorPt::CircuitSide::CircuitSide_Secondary;
	}
	else if( true == GetpSchcat()->Is3W() )
	{
		if( false == GetpSchcat()->IsInjection() )
		{
			// All the 3-way dividing circuits.
			eCircuitSide = CAnchorPt::CircuitSide::CircuitSide_Primary;
		}
		else
		{
			// All the 3-way injection or mixing circuit depend of the control valve location.
			if( true == GetpSch()->IsAnchorPtExist( CAnchorPt::eFunc::ControlValve ) )
			{
				CAnchorPt::AnchorPtFunc *pAnchorPtFunc = GetpSch()->GetAnchorPtDetails( CAnchorPt::eFunc::ControlValve );
				
				if( NULL != pAnchorPtFunc )
				{
					eCircuitSide = pAnchorPtFunc->m_eCircuitSide;
				}
			}
		}
	}

	return eCircuitSide;
}

void CDS_HydroMod::UpdateAllPipesIDPtr()
{
	if( NULL != m_pCircuitPrimaryPipe )
	{
		m_pCircuitPrimaryPipe->UpdateAllPipesIDPtr();
	}

	if( NULL != m_pDistrSupplyPipe )
	{
		m_pDistrSupplyPipe->UpdateAllPipesIDPtr();
	}

	if( NULL != m_pDistrReturnPipe )
	{
		m_pDistrReturnPipe->UpdateAllPipesIDPtr();
	}

	if( NULL != m_pCircuitSecondaryPipe )
	{
		m_pCircuitSecondaryPipe->UpdateAllPipesIDPtr();
	}

	if( NULL != m_pCircuitBypassPipe )
	{
		m_pCircuitBypassPipe->UpdateAllPipesIDPtr();
	}
}

CDS_HydroMod *CDS_HydroMod::GetParent( void )
{
	CDS_HydroMod *pParent = NULL;

	if( NULL != GetIDPtr().PP )
	{
		pParent = dynamic_cast<CDS_HydroMod *>( (CData *)GetIDPtr().PP );
	}

	return pParent;
}

CDS_HydroMod::CMeasData *CDS_HydroMod::GetpMeasData( unsigned uiIndex )
{
	if( uiIndex >= m_MeasDataDateTimeVector.size() )
	{
		return NULL;
	}

	return m_MeasDataDateTimeVector[uiIndex];
}

CDS_HydroMod::CMeasData *CDS_HydroMod::GetpMeasDataForCBI()		 // Return CMeasData* for CBI NULL if doesn't exist
{
	CDS_HydroMod::CMeasData *pMD = NULL;

	for( unsigned i = 0; i < m_MeasDataDateTimeVector.size(); i++ )
	{
		pMD = GetpMeasData( i );

		if( CDS_HydroMod::eInstrumentUsed::TACBI == pMD->GetInstrument() && 0 == pMD->GetMeasDateTime() )
		{
			return pMD;
		}
	}

	return NULL;
}

bool CDS_HydroMod::IsMeasurementExistsInProject( CDS_HydroMod *pRootModule )
{
	bool bReturn = false;

	if( NULL == pRootModule )
	{
		return false;
	}

	// Do a loop on the children of the module to see if Measurement exist
	for( IDPTR IDPtr = pRootModule->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pRootModule->GetNext() )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)( IDPtr.MP );

		if( NULL != pHM )
		{
			if( 0 != pHM->GetMeasDataSize() )
			{
				bReturn = true;
				break;
			}
		}
		
		if( true == pHM->IsaModule() )
		{
			bReturn = IsMeasurementExistsInProject( pHM );
			
			if( true == bReturn )
			{
				break;
			}
		}
	}

	return bReturn;
}

double CDS_HydroMod::CMeasData::GetFlow( double dOpening, double dDp )
{
	if( dDp <= 0.0 )
	{
		return 0.0;
	}

	double dKv = 0.0;

	// Get Water Characteristics.
	CWaterChar *pWC = GetpWC();

	if( NULL == pWC )
	{
		return 0.0;    // Not initialized!
	}

	double dRho = pWC->GetDens();
	double dNu = pWC->GetKinVisc();

	if( CDS_HydroModX::eMode::TAmode == GetMode() )
	{
		CDB_TAProduct *pTAP = GetpTAP();

		// If valve not found...
		if( NULL == pTAP )
		{
			return 0.0;
		}

		if( true == pTAP->IsKvSignalEquipped() )
		{
			// TODO Use GetValvQ from CDB_FixOChar....
			dKv = pTAP->GetKvSignal();
		}
		else
		{
			CDB_ValveCharacteristic *pValveCharacteristic = pTAP->GetValveCharacteristic();

			if( NULL == pValveCharacteristic )
			{
				return 0.0;
			}

			if( dOpening <= 0.0 )
			{
				return 0.0;
			}

			double dQ = 0.0;

			if( true == pValveCharacteristic->GetValveQ( &dQ, dDp, dOpening, dRho, dNu ) )
			{
				return dQ;
			}
			else
			{
				return 0.0;
			}
		}
	}
	else
	{
		dKv = GetKv();
	}

	if( dKv <= 0.0 )
	{
		return 0.0;
	}

	double dQ = CalcqT( dKv, dDp, dRho );
	return dQ;
}

double CDS_HydroMod::CMeasData::GetDTRatio()
{
	if( GetDesignDT() > 0.0 )
	{
		double DT = abs( GetT( m_UsedDTSensor[0] ) - GetT( m_UsedDTSensor[1] ) );
		return ( DT / GetDesignDT() * 100.0 );
	}

	return 0.0;
}

double CDS_HydroMod::CMeasData::GetPowerRatio()
{
	if( GetDesignPower() > 0.0 )
	{
		return ( GetPower() / GetDesignPower() * 100.0 );
	}

	return 0.0;
}

double CDS_HydroMod::CMeasData::GetPower()
{
	double dT1 = GetT( m_UsedDTSensor[0] );
	double dT2 = GetT( m_UsedDTSensor[1] );
	double dDT = abs( dT1 - dT2 );

	if( dDT < 0.0 )
	{
		return 0.0;
	}

	CWaterChar *pWC = GetpWC();

	if( NULL == pWC )
	{
		return 0.0;
	}

	double dRho = pWC->GetDens();
	double dcp = pWC->GetSpecifHeat();
	double dPower = CalcP( GetMeasFlow(), dDT, dRho, dcp );
	return dPower;
}

_string CDS_HydroMod::CMeasData::GetstrCurOpening()
{
	_string str = _T("-");
	CDB_TAProduct *pTAP = GetpTAP();

	if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
	{
		str = pTAP->GetValveCharacteristic()->GetSettingString( GetCurOpening() );
	}

	return str;
}

_string	CDS_HydroMod::CMeasData::GetstrDesignOpening()
{
	_string str = _T("-");
	CDB_TAProduct *pTAP = GetpTAP();

	if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
	{
		str = pTAP->GetValveCharacteristic()->GetSettingString( GetDesignOpening() );
	}

	return str;
}

_string	CDS_HydroMod::CMeasData::GetstrKv()
{
	double dKv = 0.0;

	if( CDS_HydroModX::TAmode == GetMode() )
	{
		CDB_TAProduct *pTAp = GetpTAP();

		if( NULL == pTAp )
		{
			return _T("");
		}

		CDB_ValveCharacteristic *pValveCharacteristic = pTAp->GetValveCharacteristic();

		if( NULL == pValveCharacteristic )
		{
			return _T("");
		}

		dKv = pValveCharacteristic->GetKv( GetCurOpening() );

		if( -1.0 == dKv )
		{
			return _T("");
		}
	}
	else
	{
		dKv = GetKv();
	}

	return WriteCUDouble( _C_KVCVCOEFF, dKv );
}

_string	CDS_HydroMod::CMeasData::GetstrMeasDp()
{
	return WriteCUDouble( _U_DIFFPRESS, GetMeasDp() );
}

_string	CDS_HydroMod::CMeasData::GetstrMeasQ()
{
	return WriteCUDouble( _U_FLOW, GetMeasFlow() );
}

_string	CDS_HydroMod::CMeasData::GetstrDesignQ()
{
	return WriteCUDouble( _U_FLOW, GetDesignQ() );
}

_string	CDS_HydroMod::CMeasData::GetstrT( CDS_HydroModX::eDTS dts )
{
	return WriteCUDouble( _U_TEMPERATURE, GetT( dts ) );
}

_string CDS_HydroMod::CMeasData::GetstrDT( CDS_HydroModX::eDTS dts1, CDS_HydroModX::eDTS dts2 )
{
	double dT1 = GetT( dts1 );
	double dT2 = GetT( dts2 );

	return WriteCUDouble( _U_DIFFTEMP, abs( dT1 - dT2 ) );
}

_string CDS_HydroMod::CMeasData::GetstrDesignDT()
{
	return WriteCUDouble( _U_DIFFTEMP, GetDesignDT() ) ;
}

_string CDS_HydroMod::CMeasData::GetstrPower()
{
	return WriteCUDouble( _U_TH_POWER, GetPower() );
}

_string CDS_HydroMod::CMeasData::GetstrDesignPower()
{
	return WriteCUDouble( _U_DIFFTEMP, GetDesignPower() );
}

double CDS_HydroMod::CMeasData::GetMeasFlow()
{
	// Special treatment for CBI.
	if( CDS_HydroMod::eInstrumentUsed::TACBI == GetInstrument() )
	{
		return Getqf();
	}

	return GetFlow( GetCurOpening(), GetMeasDp() );
}

double CDS_HydroMod::CMeasData::GetCurOpening()
{
	// Special treatment for CBI.
	if( CDS_HydroMod::eInstrumentUsed::TACBI == GetInstrument() )
	{
		return GetTaBalOpeningResult();
	}

	return m_dCurOpening;
}

void CDS_HydroMod::CMeasData::SetpDistributionPipe( CHMXPipe *pDistributionPipe )
{
	if( NULL != m_pDistributionPipe )
	{
		delete m_pDistributionPipe;
	}

	m_pDistributionPipe = new CHMXPipe( GetpWC(), CHMXPipe::epipeOnDistributionSupply );

	if( NULL != m_pDistributionPipe && NULL != pDistributionPipe )
	{
		pDistributionPipe->Copy( m_pDistributionPipe );
	}
}

void CDS_HydroMod::CMeasData::SetpCircuitPipe( CHMXPipe *pCircuitPipe )
{
	if( NULL != m_pCircuitPipe )
	{
		delete m_pCircuitPipe;
	}

	m_pCircuitPipe = new CHMXPipe( GetpWC(), CHMXPipe::epipeOnCircuit );

	if( NULL != m_pCircuitPipe && NULL != pCircuitPipe )
	{
		pCircuitPipe->Copy( m_pCircuitPipe );
	}
}

void CDS_HydroMod::CMeasData::SetpTerminalUnit( CHMXTU *pTerminalUnit )
{
	if( NULL != m_pTerminalUnit )
	{
		delete m_pTerminalUnit;
	}

	m_pTerminalUnit = new CHMXTU();

	if( NULL != m_pTerminalUnit && NULL != pTerminalUnit )
	{
		pTerminalUnit->Copy( m_pTerminalUnit );
	}
}

void CDS_HydroMod::CMeasData::DropData( OUTSTREAM outf, int *piLevel )
{
	CString strTab = _T("");

	for( int i = *piLevel; i; i-- )
	{
		strTab += _T("\t");
	}

	CString str;
	char buff[30];
	struct tm dataandtime;
	_localtime32_s( &dataandtime, &m_MeasDateAndTime );
	asctime_s( buff, sizeof( buff ), &dataandtime );
	str.Format( _T("m_MeasDateAndTime = %s\n"), CString( buff ) );
	WriteFormatedStringA2( outf, str, strTab );
	_localtime32_s( &dataandtime, &m_TABalDateAndTime );
	asctime_s( buff, sizeof( buff ), &dataandtime );
	str.Format( _T("m_TABalDateAndTime = %s\n"), CString( buff ) );
	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_Instrument = ") );

	switch( GetInstrument() )
	{
		case eInstrumentUsed::TACBI:
			str.Append( _T("TACBI\n") );
			break;

		case eInstrumentUsed::TASCOPE:
			str.Append( _T("TASCOPE\n") );
			break;
	}

	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_QMType = ") );

	switch( GetQMType() )
	{
		case CDS_HydroModX::eQMtype::QMundef:
			str.Append( _T("QMundef\n") );
			break;

		case CDS_HydroModX::eQMtype::QMDp:
			str.Append( _T("QMDp\n") );
			break;

		case CDS_HydroModX::eQMtype::QMFlow:
			str.Append( _T("QMFlow\n") );
			break;

		case CDS_HydroModX::eQMtype::QMTemp:
			str.Append( _T("QMTemp\n") );
			break;

		case CDS_HydroModX::eQMtype::QMPower:
			str.Append( _T("QMPower\n") );
			break;

		case CDS_HydroModX::eQMtype::QMDpl:
			str.Append( _T("QMDpl\n") );
			break;
	}

	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_Locate = ") );

	switch( GetPipeLocation() )
	{
		case CDS_HydroModX::eLocate::undef:
			str.Append( _T("undef\n") );
			break;

		case CDS_HydroModX::eLocate::InPrimary:
			str.Append( _T("InPrimary\n") );
			break;

		case CDS_HydroModX::eLocate::InSecondary:
			str.Append( _T("InSecondary\n") );
			break;

		case CDS_HydroModX::eLocate::InBypass:
			str.Append( _T("InBypass\n") );
			break;
	}

	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_Mode = ") );

	switch( GetMode() )
	{
		case CDS_HydroModX::eMode::TAmode:
			str.Append( _T("TAmode\n") );
			break;

		case CDS_HydroModX::eMode::Kvmode:
			str.Append( _T("Kvmode\n") );
			break;
	}

	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_TAPID = %s\n"), m_TAPID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dKv = %s\n"), WriteCUDouble( _C_KVCVCOEFF, GetKv() ) );
	WriteFormatedStringA2( outf, str, strTab );

	CString str1, str2, str3;
	m_WC.BuildWaterStringsRibbonBar( str1, str2, str3 );
	str.Format( _T("Water characteristic: %s; %s; %s\n"), str1, str2, str3 );
	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_dMeasDp = %s\n"), WriteCUDouble( _U_DIFFPRESS, GetMeasDp(), true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dMeasDpl = %s\n"), WriteCUDouble( _U_DIFFPRESS, GetMeasDpL(), true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dCurOpening = %s\n"), GetstrCurOpening() );
	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_dDT = DTSonHH: %s"), GetstrT( CDS_HydroModX::eDTS::DTSonHH ) );
	str.AppendFormat( _T("; DTS1onDPS: %s"), GetstrT( CDS_HydroModX::eDTS::DTS1onDPS ) );
	str.AppendFormat( _T("; DTS2onDPS: %s"), GetstrT( CDS_HydroModX::eDTS::DTS2onDPS ) );
	str.AppendFormat( _T("; DTSRef: %s\n"), GetstrT( CDS_HydroModX::eDTS::DTSRef ) );
	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_UsedDTSensor = 1: ") );

	switch( GetUsedDTSensor( 0 ) )
	{
		case CDS_HydroModX::eDTS::DTSUndef:
			str.Append( _T("undefined") );
			break;

		case CDS_HydroModX::eDTS::DTS2onDPS:
			str.Append( _T("DTS2onDPS") );
			break;

		case CDS_HydroModX::eDTS::DTS1onDPS:
			str.Append( _T("DTS1onDPS") );
			break;

		case CDS_HydroModX::eDTS::DTSonHH:
			str.Append( _T("DTSonHH") );
			break;

		default:
			str.Append( _T("invalid") );
			break;
	}

	switch( GetUsedDTSensor( 1 ) )
	{
		case CDS_HydroModX::eDTS::DTSUndef:
			str.Append( _T("; 2: undefined\n") );
			break;

		case CDS_HydroModX::eDTS::DTS2onDPS:
			str.Append( _T("; 2: DTS2onDPS\n") );
			break;

		case CDS_HydroModX::eDTS::DTS1onDPS:
			str.Append( _T("; 2: DTS1onDPS\n") );
			break;

		case CDS_HydroModX::eDTS::DTSonHH:
			str.Append( _T("; 2: DTSonHH\n") );
			break;

		default:
			str.Append( _T("; 2: invalid\n") );
			break;
	}

	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_dDesignQ = %s\n"), GetstrDesignQ() );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dDesignOpening = %s\n"), GetstrDesignOpening() );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dDesignDT = %s\n"), GetstrDesignDT() );
	WriteFormatedStringA2( outf, str, strTab );

	str = _T("m_eBalanced = ");

	switch( GetFlagBalanced() )
	{
		case CDS_HydroModX::eTABalanceMethod::etabmNoBalancing:
			str.Append( _T("etabmNoBalancing\n") );
			break;

		case CDS_HydroModX::eTABalanceMethod::etabmTABalPlus:
			str.Append( _T("etabmTABalPlus\n") );
			break;

		case CDS_HydroModX::eTABalanceMethod::etabmTABalWireless:
			str.Append( _T("etabmTABalWireless\n") );
			break;

		case CDS_HydroModX::eTABalanceMethod::etabmTABalPlusStarted:
			str.Append( _T("etabmTABalPlusStarted\n") );
			break;

		case CDS_HydroModX::eTABalanceMethod::etabmTABalWirelessStarted:
			str.Append( _T("etabmTABalWirelessStarted\n") );
			break;

		case CDS_HydroModX::eTABalanceMethod::etabmForced:
			str.Append( _T("etabmForced\n") );
			break;
	}

	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_bTaBalMeasCompleted = %s\n" ), ( true == GetFlagTaBalMeasCompleted() ) ? _T("true") : _T( "false") );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dTaBalOpening_1 = %s\n"), WriteDouble( GetTaBalOpening_1(), 2, 2 ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dTaBalMeasDp_1 = %s\n"), WriteCUDouble( _U_DIFFPRESS, GetTaBalMeasDp_1() ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dTaBalMeasDp_2 = %s\n"), WriteCUDouble( _U_DIFFPRESS, GetTaBalMeasDp_2() ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dTaBalOpeningResult = %s\n"), WriteDouble( GetTaBalOpeningResult(), 2, 2 ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_DpPVTABal = %s\n"), WriteCUDouble( _U_DIFFPRESS, GetDpPVTABal() ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dTABalDistribDp = %s\n"), WriteCUDouble( _U_DIFFPRESS, GetDistribDpComputed() ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dTABalCircuitDp = %s\n"), WriteCUDouble( _U_DIFFPRESS, GetCircDpComputed() ) );
	WriteFormatedStringA2( outf, str, strTab );

	if( NULL != m_pDistributionPipe && NULL != m_pDistributionPipe->GetpPipe() )
	{
		WriteFormatedStringA2( outf, _T("Distribution pipe = "), strTab );
		str.AppendFormat( _T("ID: %s"), m_pDistributionPipe->GetpPipe()->GetIDPtr().ID );
		str.AppendFormat( _T("; Location: ") );

		switch( m_pDistributionPipe->GetPipeLoc() )
		{
			case CHMXPipe::ePipeLoc::epipeOnDistributionSupply:
				str.AppendFormat( _T("epipeOnDistribution") );
				break;

			case CHMXPipe::ePipeLoc::epipeOnCircuit:
				str.AppendFormat( _T("epipeOnCircuit") );
				break;
		}

		str.AppendFormat( _T("; RLin: %f"), m_pDistributionPipe->GetRLin() );
		str.AppendFormat( _T("; KvFix: %s"), WriteCUDouble( _C_KVCVCOEFF, m_pDistributionPipe->GetKvFix() ) );
		str.AppendFormat( _T("; Length: %s"), WriteCUDouble( _U_LENGTH, m_pDistributionPipe->GetLength(), true ) );
		str.AppendFormat( _T("; Flow: %s"), WriteCUDouble( _U_FLOW, m_pDistributionPipe->GetFlow(), true ) );

		if( m_pDistributionPipe->GetpParentHMX() != NULL )
		{
			str.AppendFormat( _T("; Parent: %s"), m_pDistributionPipe->GetpParentHMX()->GetHMName() );
		}
		else
		{
			str.AppendFormat( _T("; Parent: unknown") );
		}

		WriteFormatedStringA2( outf, str, strTab );
		WriteFormatedStringA2( outf, _T("\n"), strTab );
	}
	else
	{
		WriteFormatedStringA2( outf, _T("Distribution pipe is not defined\n"), strTab );
	}

	if( NULL != m_pCircuitPipe && NULL != m_pCircuitPipe->GetpPipe() )
	{
		WriteFormatedStringA2( outf, _T("Circuit pipe = "), strTab );
		str.AppendFormat( _T("ID: %s"), m_pCircuitPipe->GetpPipe()->GetIDPtr().ID );
		str.AppendFormat( _T("; Location: ") );

		switch( m_pCircuitPipe->GetPipeLoc() )
		{
			case CHMXPipe::ePipeLoc::epipeOnDistributionSupply:
				str.AppendFormat( _T("epipeOnDistribution") );
				break;

			case CHMXPipe::ePipeLoc::epipeOnCircuit:
				str.AppendFormat( _T("epipeOnCircuit") );
				break;
		}

		str.AppendFormat( _T("; RLin: %f"), m_pCircuitPipe->GetRLin() );
		str.AppendFormat( _T("; KvFix: %s"), WriteCUDouble( _C_KVCVCOEFF, m_pCircuitPipe->GetKvFix() ) );
		str.AppendFormat( _T("; Length: %s"), WriteCUDouble( _U_LENGTH, m_pCircuitPipe->GetLength(), true ) );
		str.AppendFormat( _T("; Flow: %s"), WriteCUDouble( _U_FLOW, m_pCircuitPipe->GetFlow(), true ) );

		if( m_pCircuitPipe->GetpParentHMX() != NULL )
		{
			str.AppendFormat( _T("; Parent: %s"), m_pCircuitPipe->GetpParentHMX()->GetHMName() );
		}
		else
		{
			str.AppendFormat( _T("; Parent: unknown") );
		}

		WriteFormatedStringA2( outf, str, strTab );
		WriteFormatedStringA2( outf, _T("\n"), strTab );
	}
	else
	{
		WriteFormatedStringA2( outf, _T("Circuit pipe is not defined\n"), strTab );
	}

	if( m_pTerminalUnit != NULL )
	{
		WriteFormatedStringA2( outf, _T("Terminal unit = "), strTab );
		str.AppendFormat( _T("Dp: %s"), WriteCUDouble( _U_DIFFPRESS, m_pTerminalUnit->GetDp(), true ) );
		str.AppendFormat( _T("; Design flow: %s"), WriteCUDouble( _U_FLOW, m_pTerminalUnit->GetDesignFlow(), true ) );

		if( m_pTerminalUnit->GetpParentHMX() != NULL )
		{
			str.AppendFormat( _T("; Parent: %s"), m_pTerminalUnit->GetpParentHMX()->GetHMName() );
		}
		else
		{
			str.AppendFormat( _T("; Parent: unknown") );
		}

		WriteFormatedStringA2( outf, str, strTab );
		WriteFormatedStringA2( outf, _T("\n"), strTab );
	}
	else
	{
		WriteFormatedStringA2( outf, _T("Terminal unit is not defined\n"), strTab );
	}

	str.Format( _T("m_qf (CBI specific) = %s\n"), WriteCUDouble( _U_FLOW, Getqf(), true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_qTABal1 (CBI specific) = %s\n"), WriteCUDouble( _U_FLOW, GetqTABal1(), true ) );
	WriteFormatedStringA2( outf, str, strTab );

	WriteFormatedStringA2( outf, _T("\n"), strTab );
}

// Remark: if circuit is injection, the inherited "Init" method will force the "bCreateBV" to true.
void CDS_HydroMod::Init( IDPTR SchIDPtr, CTable *pclParent, bool bCreateBV )
{
	SetSchemeIDPtr( SchIDPtr );
	CreatePipes();

	CDB_CircuitScheme *pSch = static_cast<CDB_CircuitScheme *>( SchIDPtr.MP );

	if( NULL == pSch )
	{
		ASSERT( 0 );
		return;
	}

	// HYS-1579: We should allow adding pump without balancing valve
	if( true == pSch->IsAnchorPtExist( CAnchorPt::eFunc::PUMP ) )
	{
		CreatePump();
	}

	if( true == bCreateBV )
	{
		if( true == pSch->IsAnchorPtExist( CAnchorPt::BV_P ) /*&& false == pSch->IsAnchorPtOptional( CAnchorPt::BV_P ) */ )
		{
			CreateBv( CDS_HydroMod::eHMObj::eBVprim );
		}

		if( true == pSch->IsAnchorPtExist( CAnchorPt::DpC ) && false == pSch->IsAnchorPtOptional( CAnchorPt::DpC ) )
		{
			CreateDpC();
		}

		// Creation of other valves is done in inherited classes.
	}

	if( eDpStab::DpStabOnBVBypass == pSch->GetDpStab() )
	{
		// HYS-1930: Auto-adapting variable flow decoupling circuit
		CreateBv( CDS_HydroMod::eHMObj::eBVbyp );
		ASSERT( NULL != GetpBypBv() );

		if( NULL == GetpDpC() )
		{
			ASSERT_RETURN;
		}

		// No choice for this circuit, the stabilized differential pressure include the BV in the bypass.
		GetpDpC()->SetMvLoc( eMvLoc::MvLocSecondary );
		GetpDpC()->SetMvLocLocked( true );
	}
	
	if( true == pSch->IsAnchorPtExist( CAnchorPt::SmartDpC ) )
	{
		CreateSmartDpC( pSch->GetSmartDpCLoc() );
		ASSERT( NULL != GetpSmartDpC() );

		// For the moment, we have only set.
		//GetpSmartDpC()->SetSelectedAsaPackage( eBool3::eb3True );
	}

	// HYS-1716: Initialiaze all water characteristics for all pipes.
	InitAllWaterCharWithParent( pclParent );
}

CTable *CDS_HydroMod::GetOwnerTable()
{
	CTable *pTab = GetIDPtr().PP;

	while( NULL != pTab && false == pTab->IsClass( CLASS( CTableHM ) ) )
	{
		pTab = pTab->GetIDPtr().PP;
	}

	ASSERT( NULL != pTab );
	return pTab;
}

bool CDS_HydroMod::GetBalancingIndex( double *pdBalIndex )
{
	int iCount = 0;
	double dSumMeas = 0.0;
	double dSumDesign = 0.0;
	IDPTR IDPtr = _NULL_IDPTR;

	for( IDPtr = GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = GetNext() )
	{
		CMeasData *pMS;
		CDS_HydroMod *pHM = static_cast< CDS_HydroMod * >( IDPtr.MP );

		// If measures exist...
		if( pHM->GetMeasDataSize() > 0 )
		{
			pMS = pHM->GetpMeasData( pHM->GetCurrentMeasDataIndex() );

			// Verify the type of instrument used.
			// The flow will not be found in the same variable!
			double dQf = 0.0;

			if( CDS_HydroMod::eInstrumentUsed::TASCOPE == pMS->GetInstrument() )
			{
				if( 0 == pMS->GetMeasDateTime() )
				{
					continue;
				}

				//dQf = pMS->GetFlow( pMS->GetTaBalOpeningResult(), pMS->GetMeasDp() );
				dQf = pMS->GetMeasFlow();
			}
			else if( CDS_HydroMod::eInstrumentUsed::TACBI == pMS->GetInstrument() )
			{
				dQf = pMS->Getqf();    // Computed field, save time
			}

			if( NULL != pMS && dQf != 0.0 && pMS->GetDesignQ() != 0.0 )
			{
				iCount++;
				dSumMeas += dQf;
				dSumDesign += pMS->GetDesignQ();
			}
		}
	}

	// The balancing index has no meaning when there is only one valve that is measured in the module.
	if( iCount <= 1 )
	{
		return false;
	}

	double dDelta = dSumMeas / dSumDesign;
	double dTmp;

	*pdBalIndex = 0.0;

	for( IDPtr = GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = GetNext() )
	{
		CMeasData *pMS;
		CDS_HydroMod *pHM = static_cast< CDS_HydroMod * >( IDPtr.MP );

		// If measures exist...
		if( pHM->GetMeasDataSize() > 0 )
		{
			pMS = pHM->GetpMeasData( pHM->GetCurrentMeasDataIndex() );

			// Verify the type of instrument used.
			// The flow will not be found in the same variable!
			double dQf = 0.0;

			if( CDS_HydroMod::eInstrumentUsed::TASCOPE == pMS->GetInstrument() )
			{
				if( 0 == pMS->GetMeasDateTime() )
				{
					continue;
				}

				//dQf = pMS->GetFlow( pMS->GetTaBalOpeningResult(), pMS->GetMeasDp() );
				dQf = pMS->GetMeasFlow();
			}
			else if( CDS_HydroMod::eInstrumentUsed::TACBI == pMS->GetInstrument() )
			{
				dQf = pMS->Getqf();    // Computed field, save time
			}

			if( NULL != pMS && dQf != 0.0 && pMS->GetDesignQ() != 0.0 )
			{
				dTmp = dQf / ( dDelta * pMS->GetDesignQ() ) - 1.0;
				*pdBalIndex += ( dTmp * dTmp );
			}
		}
	}

	*pdBalIndex = sqrt( *pdBalIndex / iCount );

	return true;
}

CString CDS_HydroMod::RenameMe( bool bForceAuto )
{
	if( true == bForceAuto || true == GetHMName().IsEmpty() || GetHMName().GetAt( 0 ) == _T('*') )
	{
		SetHMName( _T("*") );

		if( GetLevel() <= 0 )
		{
			CString str;
			TASApp.GetpTADS()->ComposeRMName( str, this, GetPos() );
			SetHMName( (LPCTSTR) str );
		}
		else
		{
			// Found main owner table.
			CTable *pTab = GetIDPtr().PP;

			while( false == pTab->IsClass( CLASS( CTableHM ) ) )
			{
				pTab = (CTable *)pTab->GetIDPtr().PP;

				if( NULL == pTab )
				{
					break;
				}
			}

			TASApp.GetpTADS()->ComposeValveName( GetIDPtr(), pTab );
		}
	}

	return GetHMName();
}

int CDS_HydroMod::GetCount( bool bRecursive, bool bOnlyModule, int iCount )
{
	for( IDPTR IDPtr = GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = GetNext() )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( IDPtr.MP );

		if( NULL == pHM )
		{
			continue;
		}

		if( false == bOnlyModule || true == pHM->IsaModule() )
		{
			iCount++;
		}

		if( true == bRecursive && true == pHM->IsaModule() )
		{
			iCount = pHM->GetCount( bRecursive, bOnlyModule, iCount );
		}
	}

	return iCount;
}

int CDS_HydroMod::GetValveCount( bool bRecursive )
{
	int iCount = 0;

	for( IDPTR IDPtr = GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = GetNext() )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( IDPtr.MP );

		if( NULL == pHM )
		{
			continue;
		}

		int iLocalCount = 0;

		if( NULL != pHM->GetpBv() )
		{
			iLocalCount++;
		}

		if( NULL != pHM->GetpBypBv() )
		{
			iLocalCount++;
		}

		if( NULL != pHM->GetpDpC() )
		{
			iLocalCount++;
		}

		if( NULL != pHM->GetpCV() && true == pHM->GetpCV()->IsTaCV() )
		{
			iLocalCount++;
		}

		if( NULL != pHM->GetpShutoffValve( eHMObj::eShutoffValveSupply ) )
		{
			iLocalCount++;
		}

		if( NULL != pHM->GetpShutoffValve( eHMObj::eShutoffValveReturn ) )
		{
			iLocalCount++;
		}

		if( NULL != pHM->GetpSmartControlValve() )
		{
			iLocalCount++;
		}

		if( NULL != pHM->GetpSmartDpC() )
		{
			iLocalCount++;
		}

		if( iLocalCount == 0 )
		{
			// Test CBI valve.
			if( NULL != pHM->GetCBIValveIDPtr().MP )
			{
				iLocalCount++;
			}
		}

		iCount += iLocalCount;

		if( true == bRecursive && true == pHM->IsaModule() )
		{
			iCount += pHM->GetValveCount( bRecursive );
		}
	}

	return iCount;
}

bool CDS_HydroMod::HasaValve()
{
	if( edt_TADBValve == m_eVDescrType )
	{
		return ( GetTADBValveIDPtr().MP != NULL );
	}

	if( ( edt_CBISizeValve == m_eVDescrType || edt_CBISizeInchValve == m_eVDescrType ) && _T('\0') != *GetCBIType() )
	{
		return true;
	}

	if( edt_KvCv == m_eVDescrType && GetKvCv() > 0.0 )
	{
		return true;
	}

	return false;
}

void CDS_HydroMod::SetReturnType( ReturnType eReturnType )
{
	m_eReturnType = eReturnType;

	// Set the flags in the pipes.
	if( NULL != m_pCircuitPrimaryPipe )
	{
		m_pCircuitPrimaryPipe->ChangeReturnType( eReturnType );
	}

	if( NULL != m_pDistrSupplyPipe )
	{
		m_pDistrSupplyPipe->ChangeReturnType( eReturnType );
	}

	if( NULL != m_pDistrReturnPipe )
	{
		if( ReturnType::Direct == eReturnType )
		{
			delete m_pDistrReturnPipe;
			m_pDistrReturnPipe = NULL;
		}
		else
		{
			m_pDistrReturnPipe->ChangeReturnType( eReturnType );
		}
	}
	else if( ReturnType::Reverse == eReturnType )
	{
		CreatePipes( true, eHMObj::eDistributionReturnPipe );
		InitAllWaterCharWithParent( GetParent(), eHMObj::eDistributionReturnPipe );
	}

	if( NULL != m_pCircuitSecondaryPipe )
	{
		m_pCircuitSecondaryPipe->ChangeReturnType( eReturnType );
	}

	if( NULL != m_pCircuitBypassPipe )
	{
		m_pCircuitBypassPipe->ChangeReturnType( eReturnType );
	}
}

void CDS_HydroMod::SetWaterChar( CWaterChar *pclWaterChar, CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	if( NULL == pclWaterChar )
	{
		ASSERT_RETURN;
	}

	if( CAnchorPt::PipeType_Distribution == ePipeType )
	{
		if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
		{
			if( NULL == m_pDistrSupplyPipe )
			{
				ASSERT_RETURN;
			}

			m_pDistrSupplyPipe->SetWaterChar( pclWaterChar, CAnchorPt::PipeLocation_Supply );
		}
		else
		{
			if( ReturnType::Direct == GetReturnType() )
			{
				if( NULL == m_pDistrSupplyPipe )
				{
					ASSERT_RETURN;
				}

				// In case of direct return mode, the return temperature is also in the distribution supply pipe.
				m_pDistrSupplyPipe->SetWaterChar( pclWaterChar, CAnchorPt::PipeLocation_Return );
			}
			else
			{
				if( NULL == m_pDistrReturnPipe )
				{
					ASSERT_RETURN;
				}

				// In case of reverse return mode, the return temperature is in the distribution return pipe.
				m_pDistrReturnPipe->SetWaterChar( pclWaterChar, CAnchorPt::PipeLocation_Return );
			}
		}
	}
	else if( CAnchorPt::PipeType_Circuit == ePipeType )
	{
		if( CAnchorPt::CircuitSide_Primary == eCircuitSide )
		{
			if( CAnchorPt::PipeLocation::PipeLocation_Supply == ePipeLocation || CAnchorPt::PipeLocation::PipeLocation_Return == ePipeLocation )
			{
				if( NULL == m_pCircuitPrimaryPipe )
				{
					ASSERT_RETURN;
				}

				m_pCircuitPrimaryPipe->SetWaterChar( pclWaterChar, ePipeLocation );
			}
			else if( CAnchorPt::PipeLocation::PipeLocation_ByPass == ePipeLocation )
			{
				if( NULL == m_pCircuitBypassPipe )
				{
					ASSERT_RETURN;
				}

				m_pCircuitBypassPipe->SetWaterChar( pclWaterChar, ePipeLocation );
			}
		}
		else
		{
			// Secondary is threated in the 'CDS_HmInj'. 
			// We must not arrive here in this case!
			ASSERT_RETURN;
		}
	}
}

void CDS_HydroMod::SetWaterChar( CWaterChar *pclWaterChar )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	if( NULL != m_pDistrSupplyPipe )
	{
		m_pDistrSupplyPipe->SetWaterChar( pclWaterChar, CAnchorPt::PipeLocation_Supply );

		if( ReturnType::Direct == m_eReturnType && NULL != m_pDistrReturnPipe )
		{
			m_pDistrSupplyPipe->SetWaterChar( pclWaterChar, CAnchorPt::PipeLocation_Return );
		}
	}

	if( NULL != m_pDistrReturnPipe )
	{
		m_pDistrReturnPipe->SetWaterChar( pclWaterChar, CAnchorPt::PipeLocation_Return );
	}

	if( NULL != m_pCircuitPrimaryPipe )
	{
		m_pCircuitPrimaryPipe->SetWaterChar( pclWaterChar, CAnchorPt::PipeLocation_Supply );
		m_pCircuitPrimaryPipe->SetWaterChar( pclWaterChar, CAnchorPt::PipeLocation_Return );
	}

	if( NULL != m_pCircuitBypassPipe )
	{
		m_pCircuitBypassPipe->SetWaterChar( pclWaterChar, CAnchorPt::PipeLocation_ByPass );
	}
}

void CDS_HydroMod::SetTemperature( double dTemperature, CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	if( CAnchorPt::PipeType_Distribution == ePipeType )
	{
		if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
		{
			if( NULL == m_pDistrSupplyPipe )
			{
				ASSERT_RETURN;
			}

			m_pDistrSupplyPipe->SetTemperature( dTemperature, CAnchorPt::PipeLocation_Supply );
		}
		else
		{
			if( ReturnType::Direct == GetReturnType() )
			{
				if( NULL == m_pDistrSupplyPipe )
				{
					ASSERT_RETURN;
				}

				// In case of direct return mode, the return temperature is also in the distribution supply pipe.
				m_pDistrSupplyPipe->SetTemperature( dTemperature, CAnchorPt::PipeLocation_Return );
			}
			else
			{
				if( NULL == m_pDistrReturnPipe )
				{
					ASSERT_RETURN;
				}

				// In case of reverse return mode, the return temperature is in the distribution return pipe.
				m_pDistrReturnPipe->SetTemperature( dTemperature, CAnchorPt::PipeLocation_Return );
			}
		}
	}
	else if( CAnchorPt::PipeType_Circuit == ePipeType )
	{
		if( CAnchorPt::CircuitSide_Primary == eCircuitSide )
		{
			if( CAnchorPt::PipeLocation::PipeLocation_Supply == ePipeLocation || CAnchorPt::PipeLocation::PipeLocation_Return == ePipeLocation )
			{
				if( NULL == m_pCircuitPrimaryPipe )
				{
					ASSERT_RETURN;
				}

				m_pCircuitPrimaryPipe->SetTemperature( dTemperature, ePipeLocation );
			}
			else if( CAnchorPt::PipeLocation::PipeLocation_ByPass == ePipeLocation )
			{
				if( NULL == m_pCircuitBypassPipe )
				{
					ASSERT_RETURN
				}

				m_pCircuitBypassPipe->SetTemperature( dTemperature, ePipeLocation );
			}
		}
		else if( CDB_CircSchemeCateg::e3WType::e3wTypeMixing == GetpSchcat()->Get3WType() )
		{
			// Temperatures for secondardy side of a 3-way mixing are managed here and not in the 'CDS_HmInj' inherited class.
			if( NULL == m_pCircuitSecondaryPipe )
			{
				ASSERT_RETURN;
			}

			m_pCircuitSecondaryPipe->SetTemperature( dTemperature, ePipeLocation );
		}
		else
		{
			// Secondary is threated in the 'CDS_HmInj'. 
			// We must not arrive here in this case!
			ASSERT_RETURN;
		}
	}
}

enum enum_ValveInCBI CDS_HydroMod::ValveIsInCBI( CString &strCBIVersion, int iCBICompat )
{
	CTADatastruct *pTADS = TASApp.GetpTADS();
	CTADatabase *pTADB = GetpTADB();

	// Valve determined by his KvCv, always compatible.
	if( edt_KvCv == m_eVDescrType && GetKvCv() > 0.0 )
	{
		return evi_ValvePresent;
	}

	// Partially defined product try to find matching valve in TADB.
	if( ( edt_CBISizeValve == m_eVDescrType || edt_CBISizeInchValve == m_eVDescrType ) && *GetCBIType() )
	{
		CRank TAPList;

		if( true == pTADB->GetTAPListFromCBIDef( &TAPList, m_tcCBIType, m_tcCBISize ) )
		{
			CString str;
			LPARAM pTAP;

			// At least one valve compatible.
			TAPList.GetFirst( str, pTAP );

			if( strCBIVersion.CompareNoCase( ( ( CDB_TAProduct * ) pTAP )->GetCBIVersion() ) >= 0
					&& iCBICompat >= ( ( CDB_TAProduct * ) pTAP )->GetCBICompatibilityIndex() )
			{
				return evi_ValvePresent;
			}

			while( TRUE == TAPList.GetNext( str, pTAP ) )
			{
				if( strCBIVersion.CompareNoCase( ( ( CDB_TAProduct * ) pTAP )->GetCBIVersion() ) >= 0
						&& iCBICompat >= ( ( CDB_TAProduct * ) pTAP )->GetCBICompatibilityIndex() )
				{
					return evi_ValvePresent;
				}
			}
		}
		else		// Valve not present in the TADB
		{
			return evi_Undetermined;
		}
	}

	// Valve from TADB; check the compatibility
	if( edt_TADBValve == m_eVDescrType )
	{
		CDB_TAProduct *pTAP = static_cast<CDB_TAProduct *>( GetTADBValveIDPtr().MP );

		if( NULL != pTAP )
		{
			// Check compatibility.
			if( strCBIVersion.CompareNoCase( pTAP->GetCBIVersion() ) >= 0 && iCBICompat >= pTAP->GetCBICompatibilityIndex() )
			{
				return evi_ValvePresent;
			}
		}
	}

	return evi_ValveNotPresent;
}

void CDS_HydroMod::DeleteTermUnit()
{
	if( NULL != m_pTermUnit )
	{
		delete m_pTermUnit;
	}

	m_pTermUnit = NULL;
}

void CDS_HydroMod::DeletePump()
{
	if( NULL != m_pPump )
	{
		delete m_pPump;
	}

	m_pPump = NULL;
}

void CDS_HydroMod::DeletePipes( int iWhichPipe )
{
	if( eHMObj::eCircuitPrimaryPipe == ( iWhichPipe & eHMObj::eCircuitPrimaryPipe ) )
	{
		if( NULL != m_pCircuitPrimaryPipe )
		{
			delete m_pCircuitPrimaryPipe;
		}

		m_pCircuitPrimaryPipe = NULL;
	}

	if( eHMObj::eDistributionSupplyPipe == ( iWhichPipe & eHMObj::eDistributionSupplyPipe ) )
	{
		if( NULL != m_pDistrSupplyPipe )
		{
			delete m_pDistrSupplyPipe;
		}

		m_pDistrSupplyPipe = NULL;
	}

	if( eHMObj::eDistributionReturnPipe == ( iWhichPipe & eHMObj::eDistributionReturnPipe ) )
	{
		if( NULL != m_pDistrReturnPipe )
		{
			delete m_pDistrReturnPipe;
		}

		m_pDistrReturnPipe = NULL;
	}

	if( eHMObj::eCircuitSecondaryPipe == ( iWhichPipe & eHMObj::eCircuitSecondaryPipe ) )
	{
		if( NULL != m_pCircuitSecondaryPipe )
		{
			delete m_pCircuitSecondaryPipe;
		}

		m_pCircuitSecondaryPipe = NULL;
	}

	if( eHMObj::eCircuitBypassPrimaryPipe == ( iWhichPipe & eHMObj::eCircuitBypassPrimaryPipe ) )
	{
		if( NULL != m_pCircuitBypassPipe )
		{
			delete m_pCircuitBypassPipe;
		}

		m_pCircuitBypassPipe = NULL;
	}
}

void CDS_HydroMod::DeleteMeasData()
{
	for( unsigned i = 0; i < m_MeasDataDateTimeVector.size(); i++ )
	{
		delete m_MeasDataDateTimeVector[i];
	}
}

void CDS_HydroMod::DeleteBv( CBV **pBv )
{
	if( NULL != *pBv )
	{
		if( eHMObj::eBVprim == ( *pBv )->GetHMObjectType() )
		{
			SetCBIValveID( _T("") );
		}

		delete *pBv;
	}

	*pBv = NULL;
}

void CDS_HydroMod::DeleteDpC()
{
	if( NULL != m_pDpC )
	{
		delete m_pDpC;
	}

	m_pDpC = NULL;
}

void CDS_HydroMod::DeleteCv()
{
	if( NULL != m_pCv )
	{
		delete m_pCv;
	}

	m_pCv = NULL;
}

void CDS_HydroMod::DeleteSmartControlValve()
{
	if( NULL != m_pSmartControlValve )
	{
		delete m_pSmartControlValve;
	}

	m_pSmartControlValve = NULL;
}

void CDS_HydroMod::DeleteSmartDpC()
{
	if( NULL != m_pSmartDpC )
	{
		delete m_pSmartDpC;
	}

	m_pSmartDpC = NULL;
}

void CDS_HydroMod::DeleteSV( CShutoffValve **pSV )
{
	if( NULL != *pSV )
	{
		delete *pSV;
	}

	*pSV = NULL;
}

bool CDS_HydroMod::CreateTermUnit()
{
	if( NULL != m_pTermUnit )
	{
		return false;
	}

	try
	{
		m_pTermUnit = new CTermUnit( this );
	}
	catch( ... )
	{
		return false;
	}

	return true;
}

bool CDS_HydroMod::CreatePump()
{
	if( NULL != m_pPump )
	{
		return false;
	}

	try
	{
		m_pPump = new CPump( this );
	}
	catch( ... )
	{
		return false;
	}

	return true;
}

// This function will complete the hydronic module to be readable in HMCalc mode.
// This function is created typically when the user create an hydronic module
// from the CBI or in non hydrocalc mode.
bool CDS_HydroMod::CompleteHMForHydroCalcMode( CTable *pclParent, bool bLock /*= true*/ )
{
	// Variables.
	CTable *pTabCircSch = static_cast<CTable *>( TASApp.GetpTADB()->Get( _T("CIRCSCHEME_TAB") ).MP );
	ASSERT( NULL != pTabCircSch );

	IDPTR SchIDPtr = _NULL_IDPTR;

	if( true == IsHMCalcMode() )
	{
		// Code to correct missing selected valve into old ".tsp" files.
		if( NULL != GetpBv() && NULL == GetpBv()->GetIDPtr().MP && NULL != GetpBv()->GetBestBvIDPtr().MP )
		{
			GetpBv()->SetIDPtr( GetpBv()->GetBestBvIDPtr() );
		}

		if( NULL != GetpCV() && true == GetpCV()->IsTaCV() && NULL == GetpCV()->GetCvIDPtr().MP && NULL != GetpCV()->GetBestCvIDPtr().MP )
		{
			GetpCV()->SetCvIDPtr( GetpCV()->GetBestCvIDPtr() );
		}

		if( NULL != GetpDpC() && NULL == GetpDpC()->GetIDPtr().MP && NULL != GetpDpC()->GetBestDpCIDPtr().MP )
		{
			GetpDpC()->SetIDPtr( GetpDpC()->GetBestDpCIDPtr() );
		}

		return true;
	}

	// Remark: Before we had the possibility to add a balancing valve or a balancing & control valve.
	//         Now we have also the combined Dp controller, control and balancing valve.

	// Set a default scheme with no valve.
	if( NULL == GetSchemeIDPtr().MP && NULL == GetCBIValveIDPtr().MP )
	{
		// HYS-1750: Staight pipe circuit in only for module. If we have a terminal unit the default circuit without valve should be Pending circuit.
		if( true == IsaModule() )
		{
			SetSchemeID( _T( "SP" ) );
		}
		else
		{
			SetSchemeID( _T( "PDG" ) );
		}

		if( edt_KvCv == GetVDescrType() || NULL != GetpBv() )
		{
			SetSchemeID( _T("DC_BV") );
		}
		else if( true == IsCvExist() )
		{
			CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( GetpCV()->GetCvIDPtr().MP ) ;

			if( NULL != pCV && true == pCV->IsaBCV() )
			{
				SetSchemeID( _T("2WD_BV3") );
			}
		}
	}

	// Create the default pipes.
	CreatePipes();

	// Initialize all the water characteristics in the pipes.
	InitAllWaterCharWithParent( pclParent );

	// If pipe exist verify that they could be selected.
	CArray <CTable *> arTab;

	if( NULL != GetpCircuitPrimaryPipe() )
	{
		arTab.Add( GetpCircuitPrimaryPipe()->GetPipeSeries() );
	}

	if( NULL != GetpDistrSupplyPipe() )
	{
		arTab.Add( GetpDistrSupplyPipe()->GetPipeSeries() );
	}

	if( NULL != GetpDistrReturnPipe() )
	{
		arTab.Add( GetpDistrReturnPipe()->GetPipeSeries() );
	}

	for( int i = 0; i < arTab.GetCount(); i++ )
	{
		CTable *pTab = arTab[i];

		if( NULL != pTab )
		{
			for( IDPTR IDPtr = pTab->GetFirst(); NULL != IDPtr.MP; IDPtr = pTab->GetNext() )
			{
				CDB_Pipe *pPipe = static_cast<CDB_Pipe *>( IDPtr.MP );
				pPipe->SetHidden( false );
			}

			pTab->SetHidden( false );
		}
	}

	// Create the terminal unit.
	CreateTermUnit();
	GetpTermUnit()->SetQ( GetQDesign() );

	// Set the terminal unit visible or not.
	GetpTermUnit()->SetVirtual( IsaModule() );
	SetHMCalcMode( true );
	bool bValveExist = ( NULL != GetCBIValveIDPtr().MP ) ? true : false;

	if( false == bValveExist )
	{
		return false;
	}

	// Remark: Before we had the possibility to add a balancing valve or a balancing & control valve.
	//         Now we have also the combined Dp controller, control and balancing valve (ie: TA-COMPACT-DP).

	// We try first to dynamically cast to 'CDB_DpCBCValve' that is inherited from 'CDB_ControlValve'.
	CDB_DpCBCValve *pDBDpCBCValve = dynamic_cast<CDB_DpCBCValve *>( GetCBIValveIDPtr().MP );

	CDB_ControlValve *pDBCv = NULL;

	if( NULL == pDBDpCBCValve )
	{
		pDBCv = dynamic_cast<CDB_ControlValve *>( GetCBIValveIDPtr().MP );
	}

	CDB_RegulatingValve *pDBBV = NULL;

	if( NULL == pDBDpCBCValve && NULL == pDBCv )
	{
		pDBBV = dynamic_cast<CDB_RegulatingValve *>( GetCBIValveIDPtr().MP );
	}

	CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( GetCBIValveIDPtr().MP );

	// Define the scheme with the current valve.
	CTable *pTab = static_cast<CTable *>( TASApp.GetpTADB()->Get( _T("CIRCSCHEME_TAB") ).MP );
	ASSERT( pTab );

	// If scheme doesn't yet exist we need to determine it in regards to the current valves created in the hydromod.
	if( NULL != pTab && NULL == GetSchemeIDPtr().MP )
	{
		if( NULL != pDBBV )
		{
			// Set the scheme.
			SetSchemeID( _T("DC_BV") );
		}
		else if( NULL != pDBCv )
		{
			CDB_PIControlValve *pPICV = dynamic_cast<CDB_PIControlValve *>( pDBCv );
			CDB_ControlProperties *pCtrlProp = pDBCv->GetCtrlProp();

			if( NULL != pCtrlProp )
			{
				// Do a loop on all scheme.
				for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
				{
					CDB_CircuitScheme *pSch = static_cast<CDB_CircuitScheme *>( IDPtr.MP );

					// Consider only distribution schemes.
					// Control Valve exist keep only 2ways circuit.
					if( 0 != IDcmp( pSch->GetSchemeCategID(), _T("SCHCATEG_2WD") ) /*&& 0 != IDcmp( pSch->GetSchemeCategID(), _T("SCHCATEG_2W_INJ") )*/ )
					{
						continue;
					}

					if( false == pCtrlProp->CtrlParamFits( ( (CDB_CircSchemeCateg *)pSch->GetSchemeCategIDPtr().MP )->Get2W3W(), pSch->GetCvFunc() ) )
					{
						continue;
					}

					if( NULL != pPICV )
					{
						if( CDB_CircuitScheme::eDpCType::eDpCTypePICV != pSch->GetDpCType() )
						{
							continue;
						}
					}
					else
					{
						if( CDB_CircuitScheme::eDpCType::eDpCTypePICV == pSch->GetDpCType() )
						{
							continue;
						}
					}

					SetSchemeID( IDPtr.ID );
					break;
				}
			}
		}
		else if( NULL != pDBDpCBCValve )
		{
			// For the moment we have only one circuit scheme for the combined Dp controller, control and balancing valves.
			SetSchemeID( _T("2WD_DPCBCV") );
		}
		else if( NULL != pclSmartControlValve )
		{
			if( 0 == CString( pclSmartControlValve->GetTypeID() ).Compare( _T("SMARTCTRLVALVETYPE") ) )
			{
				// We take the only solution that is the 2-way control circuit with the smart control valve on the supply by default.
				SetSchemeID( _T("2WD_SMART1") );
			}
			else if( 0 == CString( pclSmartControlValve->GetTypeID() ).Compare( _T("SMARTDPCTYPE") ) )
			{
				// We take the only solution that is the distribution circuit with the smart differential pressure controller on the supply by default.
				SetSchemeID( _T("DC_SMARTDP1") );
			}
			else
			{
				// Must not happen.
				ASSERT( 0 );
			}
		}
		else if( true == IsaModule() )
		{
			SetSchemeID( _T("SP") );
		}
		else
		{
			SetSchemeID( _T( "PDG" ) );
		}
	}

	// Is the selected valve is a control valve or a combined Dp controller, control and balancing valve.
	if( ( NULL != pDBCv || NULL != pDBDpCBCValve ) && NULL != GetpSch() )
	{
		// If exist remove BV (user probably change from BV to CV)...
		if( NULL != GetpBv() )
		{
			DeleteBv( GetppBv() );
		}

		CreateCv( GetpSch()->GetCvLoc() );
		CDS_HydroMod::CCv *pHMCv = GetpCV();

		if( NULL != pHMCv )
		{
			CDB_ControlValve *pclControlValve = ( NULL != pDBCv ) ? pDBCv : pDBDpCBCValve;

			// Set valve.
			this->m_CBIValveIDPtr = pclControlValve->GetIDPtr();
			pHMCv->SetCvIDPtr( pclControlValve->GetIDPtr() );
			pHMCv->SetBestCvIDPtr( pclControlValve->GetIDPtr() );
			pHMCv->SetTACv( true );
			CDB_ControlProperties *pCtrlProp = pclControlValve->GetCtrlProp();

			if( NULL != pCtrlProp )
			{
				pHMCv->SetCtrlType( pCtrlProp->GetCtrlType() );
			}

			// Set the Flow for the valve.
			pHMCv->SetQ( GetQDesign() );

			// Recuperate the define opening or set to the MAXIMUM opening.
			if( GetPresetting() != 0.0 )
			{
				pHMCv->SetSetting( GetPresetting() );
			}
			else
			{
				CDB_Characteristic *pCharacteristic = pclControlValve->GetValveCharacteristic();

				if( NULL != pCharacteristic )
				{
					double dOpeningMax = pCharacteristic->GetOpeningMax();

					if( -1.0 != dOpeningMax )
					{
						pHMCv->SetSetting( dOpeningMax );
					}
				}
			}

			if( true == bLock )
			{
				SetLock( CDS_HydroMod::eHMObj::eCV, true, false, true );
			}
		}
	}
	else if( NULL != pDBBV )	// Is it a balancing valve ?
	{
		// If exist remove CV (user probably change from CV to BV)...
		if( NULL != GetpCV() )
		{
			DeleteCv();
		}

		// Create the Bv.
		CreateBv( CDS_HydroMod::eHMObj::eBVprim );
		CDS_HydroMod::CBV *pHMBv = GetpBv();

		if( NULL != pHMBv )
		{
			// Set valve.
			this->m_CBIValveIDPtr = pDBBV->GetIDPtr();
			pHMBv->SetIDPtr( pDBBV->GetIDPtr()/*GetCBIValveIDPtr()*/ );
			pHMBv->SetBestBvIDPtr( pDBBV->GetIDPtr() );

			// Set the Flow for the valve.
			pHMBv->SetQ( GetQDesign() );

			// Set a default value for the Dp of the valve.
			pHMBv->SetDp( 0.0 );

			// Recuperate the define opening or set to the MAXIMUM opening.
			if( GetPresetting() != 0.0 )
			{
				pHMBv->SetSetting( GetPresetting() );
			}
			else
			{
				CDB_Characteristic *pCharacteristic = pDBBV->GetValveCharacteristic();

				if( NULL != pCharacteristic )
				{
					double dOpeningMax = pCharacteristic->GetOpeningMax();

					if( -1.0 != dOpeningMax )
					{
						pHMBv->SetSetting( dOpeningMax );
					}
				}
			}

			if( true == bLock )
			{
				SetLock( CDS_HydroMod::eHMObj::eBVprim, true, false, true );
			}
		}
	}
	else if( NULL != pclSmartControlValve )
	{
		if( 0 == CString( pclSmartControlValve->GetTypeID() ).Compare( _T("SMARTCTRLVALVETYPE") ) )
		{
			CreateSmartControlValve( SmartValveLocalization::SmartValveLocSupply );
			CDS_HydroMod::CSmartControlValve *pHMSmartControlValve = GetpSmartControlValve();

			if( NULL != pHMSmartControlValve )
			{
				// Set valve.
				this->m_CBIValveIDPtr = pclSmartControlValve->GetIDPtr();
				pHMSmartControlValve->SetIDPtr( pclSmartControlValve->GetIDPtr() );
				pHMSmartControlValve->SetBestIDPtr( pclSmartControlValve->GetIDPtr() );

				// Set the flow for the valve.
				pHMSmartControlValve->SetQ( GetQDesign() );

				if( true == bLock )
				{
					SetLock( CDS_HydroMod::eHMObj::eSmartControlValve, true, false, true );
				}
			}
		}
		else if( 0 == CString( pclSmartControlValve->GetTypeID() ).Compare( _T("SMARTDPCTYPE") ) )
		{
			CreateSmartDpC( SmartValveLocalization::SmartValveLocSupply );
			CDS_HydroMod::CSmartDpC *pHMSmartDpC = GetpSmartDpC();

			if( NULL != pHMSmartDpC )
			{
				// Set valve.
				this->m_CBIValveIDPtr = pclSmartControlValve->GetIDPtr();
				pHMSmartDpC->SetIDPtr( pclSmartControlValve->GetIDPtr() );
				pHMSmartDpC->SetBestIDPtr( pclSmartControlValve->GetIDPtr() );

				// Set the flow for the valve.
				pHMSmartDpC->SetQ( GetQDesign() );

				if( true == bLock )
				{
					SetLock( CDS_HydroMod::eHMObj::eSmartDpC, true, false, true );
				}
			}
		}
		else
		{
			// Must not happen.
			ASSERT( 0 );
		}
	}

	return true;
}

bool CDS_HydroMod::CreatePipes( bool bWithSingularity, int iWhichPipe )
{
	try
	{
		if( eHMObj::eCircuitPrimaryPipe == ( iWhichPipe & eHMObj::eCircuitPrimaryPipe ) )
		{
			if( NULL == m_pCircuitPrimaryPipe )
			{
				m_pCircuitPrimaryPipe = new CPipes( this, eHMObj::eCircuitPrimaryPipe );
			}

			if( true == bWithSingularity )
			{
				m_pCircuitPrimaryPipe->AddConnectionSingularity( _T("ELBOW_90") );
			}
		}

		if( eHMObj::eDistributionSupplyPipe == ( iWhichPipe & eHMObj::eDistributionSupplyPipe ) )
		{
			if( NULL == m_pDistrSupplyPipe )
			{
				m_pDistrSupplyPipe = new CPipes( this, eHMObj::eDistributionSupplyPipe );
			}

			// Remark: If connexion singularity already exists, we overwrite it with new the value.
			if( true == bWithSingularity )
			{
				m_pDistrSupplyPipe->AddConnectionSingularity( _T("SINGUL_NONE") );
			}
		}

		if( eHMObj::eDistributionReturnPipe == ( iWhichPipe & eHMObj::eDistributionReturnPipe ) && ReturnType::Reverse == m_eReturnType )
		{
			if( NULL == m_pDistrReturnPipe )
			{
				m_pDistrReturnPipe = new CPipes( this, eHMObj::eDistributionReturnPipe );
			}

			// Remark: If connexion singularity already exists, we overwrite it with new the value.
			if( true == bWithSingularity )
			{
				m_pDistrReturnPipe->AddConnectionSingularity( _T("SINGUL_NONE") );
			}
		}

		if( eHMObj::eCircuitSecondaryPipe == ( iWhichPipe & eHMObj::eCircuitSecondaryPipe ) && GetpSchcat() != NULL && true == GetpSchcat()->IsInjection() )
		{
			if( NULL == m_pCircuitSecondaryPipe )
			{
				m_pCircuitSecondaryPipe = new CPipes( this, eHMObj::eCircuitSecondaryPipe );
			}

			// Remark: If connexion singularity already exists, we overwrite it with new the value.
			if( true == bWithSingularity )
			{
				m_pCircuitSecondaryPipe->AddConnectionSingularity( _T("SINGUL_NONE") );
			}
		}

		if( eHMObj::eCircuitBypassPrimaryPipe == ( iWhichPipe & eHMObj::eCircuitBypassPrimaryPipe ) && GetpSchcat() != NULL && true == GetpSch()->IsAnchorPtExist( CAnchorPt::eFunc::BV_Byp ) )
		{
			if( NULL == m_pCircuitBypassPipe )
			{
				m_pCircuitBypassPipe = new CPipes( this, eHMObj::eCircuitBypassPrimaryPipe );
			}

			if( true == bWithSingularity )
			{
				// For bypass, it's well from Vb -> Va so TEE_DZA.
				m_pCircuitBypassPipe->AddConnectionSingularity( _T("TEE_DZA") );
			}
		}
	}
	catch( ... )
	{
		ASSERT( 0 );
		return false;
	}

	return true;
}

bool CDS_HydroMod::CreateBv( eHMObj eBvLocated )
{
	CBV **pBv;

	switch( eBvLocated )
	{
		case eHMObj::eBVprim:
			pBv = &m_pBv;
			break;

		case eHMObj::eBVsec:
			pBv = &m_pSecBv;
			break;

		case eHMObj::eBVbyp:
			pBv = &m_pBypBv;
			break;

		default:
			ASSERT( 0 );
			break;
	}

	if( NULL != *pBv )
	{
		return false;
	}

	try
	{
		*pBv = new CBV( this, eBvLocated );
	}
	catch( ... )
	{
		*pBv = NULL;
		return false;
	}

	return true;
}

bool CDS_HydroMod::CreateDpC()
{
	if( NULL != m_pDpC )
	{
		return false;
	}

	try
	{
		m_pDpC = new CDpC( this );
	}
	catch( ... )
	{
		return false;
	}

	return true;
}

CDS_HydroMod::CMeasData *CDS_HydroMod::CreateMeasData()
{
	try
	{
		CMeasData *pMeasData = new CMeasData( this );
		m_MeasDataDateTimeVector.push_back( pMeasData );
		return pMeasData;
	}
	catch( ... )
	{
		return NULL;
	}

	return NULL;
}

bool CDS_HydroMod::CreateCv( CvLocation eCvLocated )
{
	if( NULL != m_pCv )
	{
		return false;
	}

	try
	{
		m_pCv = new CCv( this, eCvLocated );
	}
	catch( ... )
	{
		ASSERT( 0 );
		return false;
	}

	return true;
}

bool CDS_HydroMod::CreateSmartControlValve( SmartValveLocalization eSmartValveLocalization )
{
	if( NULL != m_pSmartControlValve )
	{
		return false;
	}

	try
	{
		m_pSmartControlValve = new CSmartControlValve( this );
		m_pSmartControlValve->SetLocalization( eSmartValveLocalization );
	}
	catch( ... )
	{
		ASSERT( 0 );
		return false;
	}

	return true;
}

bool CDS_HydroMod::CreateSmartDpC( SmartValveLocalization eSmartValveLocalization )
{
	if( NULL != m_pSmartDpC )
	{
		return false;
	}

	try
	{
		m_pSmartDpC = new CSmartDpC( this );
		m_pSmartDpC->SetLocalization( eSmartValveLocalization );
	}
	catch( ... )
	{
		ASSERT( 0 );
		return false;
	}

	return true;
}

bool CDS_HydroMod::CreateSV( eHMObj eSVLocated )
{
	CShutoffValve **pSV;

	switch( eSVLocated )
	{
		case eHMObj::eShutoffValveSupply:
			pSV = &m_pShutoffValveSupply;
			break;

		case eHMObj::eShutoffValveReturn:
			pSV = &m_pShutoffValveReturn;
			break;

		default:
			ASSERT( 0 );
			break;
	}

	if( NULL != *pSV )
	{
		return false;
	}

	try
	{
		*pSV = new CShutoffValve( this, eSVLocated );
	}
	catch( ... )
	{
		*pSV = NULL;
		return false;
	}

	return true;
}

void CDS_HydroMod::SetFlagModule( bool bModule )
{
	if( m_bModule != bModule )
	{
		m_bModule = bModule;
		Modified();
	}

	// Terminal Unit is virtual when circuit is a module.
	if( true == IsTermUnitExist() )
	{
		GetpTermUnit()->SetVirtual( ( true == m_bModule ) ? true : false );
	}

	// Remove pipe length and singularity if existing, useful when changing a circuit to a module.
	if( 0 == GetLevel() )
	{
		if( NULL != GetpDistrSupplyPipe() )
		{
			SetLock( eDistributionSupplyPipe, false, false, true );
			GetpDistrSupplyPipe()->SetLength( 0.0 );
			GetpDistrSupplyPipe()->RemoveAllSingularities();
		}

		if( NULL != GetpDistrReturnPipe() )
		{
			SetLock( eDistributionReturnPipe, false, false, true );
			GetpDistrReturnPipe()->SetLength( 0.0 );
			GetpDistrReturnPipe()->RemoveAllSingularities();
		}
	}

	if( true == m_bModule && NULL != GetpSchcat() )
	{
		// Circuit pipes don't exist for module, except when module is injection BUT not for the root.
		if( NULL != GetpCircuitPrimaryPipe() && ( false == GetpSchcat()->IsSecondarySideExist() || 0 == GetLevel() ) )		// Level 0 for RootModule
		{
			SetLock( eCircuitPrimaryPipe, false, false, true );
			GetpCircuitPrimaryPipe()->SetLength( 0.0 );
			GetpCircuitPrimaryPipe()->RemoveAllSingularities();
		}

		//  Bypass pipes don't exist for module, except when bypass is at the primary side BUT not for the root.
		if( NULL != GetpCircuitBypassPipe() && ( CAnchorPt::CircuitSide::CircuitSide_Secondary == GetBypassPipeSide() || 0 == GetLevel() ) )		// Level 0 for RootModule
		{
			SetLock( eCircuitBypassPrimaryPipe, false, false, true );
			GetpCircuitBypassPipe()->SetLength( 0.0 );
			GetpCircuitBypassPipe()->RemoveAllSingularities();
		}

		// Secondary pipes exist for module when injection BUT not for the root.
		if( NULL != GetpCircuitSecondaryPipe() && ( true == GetpSchcat()->IsSecondarySideExist() || 0 == GetLevel() ) )		// Level 0 for RootModule
		{
			SetLock( eCircuitSecondaryPipe, false, false, true );
			GetpCircuitSecondaryPipe()->SetLength( 0.0 );
			GetpCircuitSecondaryPipe()->RemoveAllSingularities();
		}
	}
}

bool CDS_HydroMod::CheckIfTempAreValid( int *piErrors, std::vector<CString> *pvecStrErrors )
{
	// HYS-1882: Verify return temperature.
	if( NULL != GetpTermUnit() && ( false == IsDTFieldDisplayedInFlowMode() && CTermUnit::_QType::PdT != GetpTermUnit()->GetQType() ) )
	{
		return true;
	}

	ProjectType eProjectApplicationType = TASApp.GetpTADS()->GetpTechParams()->GetProjectApplicationType();

	double dPrimarySupplyTemp = GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Supply,
			CAnchorPt::CircuitSide::CircuitSide_Primary );
	
	double dPrimaryReturnTemp = GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Return,
			CAnchorPt::CircuitSide::CircuitSide_Primary );

	CString strSupplyTemp = WriteCUDouble( _U_TEMPERATURE, dPrimarySupplyTemp, true );
	CString strReturnTemp = WriteCUDouble( _U_TEMPERATURE, dPrimaryReturnTemp, true );
	int iErrors = 0;

	if( dPrimarySupplyTemp > -273.15 && dPrimaryReturnTemp > -273.15 )
	{
		CString str = _T( "" );

		if( dPrimarySupplyTemp == dPrimaryReturnTemp )
		{
			iErrors |= TE_TemperatureEquals;

			//if( NULL != pvecStrErrors )
			//{
			//	// "Secondary supply temperature 'tss' (%1) can not be equal to the secondary return temperature 'trs' (%2)."
			//	FormatString( str, IDS_INJCIRC_TEMPERROR_CANTBEEQUAL, strSupplyTemp, strReturnTemp );
			//	pvecStrErrors->push_back( str );
			//}
		}

		if( eProjectApplicationType == Heating )
		{
			if( dPrimaryReturnTemp > dPrimarySupplyTemp )
			{
				iErrors |= TE_Heating_TrpGreaterThanTsp;

				//if( NULL != pvecStrErrors )
				//{
				//	// Heating system: supply secondary temperature 'tss' (%1) can not be lower than the return secondary temperature 'trs' (%2).
				//	FormatString( str, IDS_INJCIRC_TEMPERROR_HEATING_TRSCANTBEGREATERTHANTSP, strReturnTemp, strSupplyTemp );
				//	pvecStrErrors->push_back( str );
				//}
			}

		}
		else
		{

			if( dPrimaryReturnTemp < dPrimarySupplyTemp )
			{
				iErrors |= TE_Cooling_TrpLowerThanTsp;

				//if( NULL != pvecStrErrors )
				//{
				//	// Cooling system: supply secondary temperature 'tss' (%1) can not be lower than the supply primary temperature 'tsp' (%2).
				//	FormatString( str, IDS_INJCIRC_TEMPERROR_COOLING_TRSCANTBELOWERTHANTSP, strReturnTemp, strSupplyTemp );
				//	pvecStrErrors->push_back( str );
				//}
			}
		}

		// HYS-1748: Consider freezing temperature.
		double dFreezingPoint = -273.15;
		CWaterChar* pclWaterChar = TASApp.GetpTADS()->GetpWCForProject()->GetpWCData();

		if( NULL != pclWaterChar )
		{
			dFreezingPoint = pclWaterChar->GetTfreez();
		}

		if( dFreezingPoint != -273.15 && (TE_TemperatureEquals != (TE_TemperatureEquals & iErrors))
			&& (TE_Heating_TrpGreaterThanTsp != (TE_Heating_TrpGreaterThanTsp & iErrors))
			&& (TE_Cooling_TrpLowerThanTsp != (TE_Cooling_TrpLowerThanTsp & iErrors)) )
		{
			CString strFreezingPoint = WriteCUDouble( _U_TEMPERATURE, dFreezingPoint, true );

			if( dPrimaryReturnTemp <= dFreezingPoint )
			{
				iErrors |= TE_ReturnTemp_FreezingPoint;

				//if( NULL != pvecStrErrors )
				//{
				//	// Return secondary temperature 'trs' (%1) must be greater than the freezing point temperature (%2).
				//	FormatString( str, IDS_INJCIRC_TEMPERROR_TRSBELOWOREQUALTOFREEZINGPOINT, strReturnTemp, strFreezingPoint );
				//	pvecStrErrors->push_back( str );
				//}
			}
		}
		if( NULL != piErrors )
		{
			*piErrors = iErrors;
		}

		return ( TE_OK == iErrors ) ? true : false;
	}

	return false;
}

bool CDS_HydroMod::IsCompletelyDefined( int iHMObjToCheck )
{
	if( eHMObj::eCircuitPrimaryPipe == ( iHMObjToCheck & eHMObj::eCircuitPrimaryPipe ) )
	{
		if( NULL != GetpCircuitPrimaryPipe() && false == GetpCircuitPrimaryPipe()->IsCompletelyDefined() )
		{
			return false;
		}
	}

	if( eHMObj::eCircuitBypassPrimaryPipe == ( iHMObjToCheck & eHMObj::eCircuitBypassPrimaryPipe ) )
	{
		if( NULL != GetpCircuitBypassPipe() && false == GetpCircuitBypassPipe()->IsCompletelyDefined() )
		{
			return false;
		}
	}

	if( eHMObj::eCircuitSecondaryPipe == ( iHMObjToCheck & eHMObj::eCircuitSecondaryPipe ) )
	{
		if( NULL != GetpCircuitSecondaryPipe() && false == GetpCircuitSecondaryPipe()->IsCompletelyDefined() )
		{
			return false;
		}
	}

	if( eHMObj::eDistributionSupplyPipe == ( iHMObjToCheck & eHMObj::eDistributionSupplyPipe ) )
	{
		if( NULL != GetpDistrSupplyPipe() && false == GetpDistrSupplyPipe()->IsCompletelyDefined() )
		{
			return false;
		}
	}

	if( eHMObj::eDistributionReturnPipe == ( iHMObjToCheck & eHMObj::eDistributionReturnPipe ) )
	{
		if( ReturnType::Reverse == GetReturnType() && NULL != GetpDistrReturnPipe() &&  false == GetpDistrReturnPipe()->IsCompletelyDefined() )
		{
			return false;
		}
	}

	if( true == IsTermUnitExist() && false == GetpTermUnit()->IsCompletelyDefined() )
	{
		return false;
	}

	if( eHMObj::eBVprim == ( iHMObjToCheck & eHMObj::eBVprim ) )
	{
		if( true == IsBvExist() && false == GetpBv()->IsCompletelyDefined() )
		{
			return false;
		}
	}

	if( eHMObj::eBVbyp == ( iHMObjToCheck & eHMObj::eBVbyp ) )
	{
		if( true == IsBvBypExist() && false == GetpBypBv()->IsCompletelyDefined() )
		{
			return false;
		}
	}

	if( eHMObj::eBVsec == ( iHMObjToCheck & eHMObj::eBVsec ) )
	{
		if( true == IsBvSecExist() && false == GetpSecBv()->IsCompletelyDefined() )
		{
			return false;
		}
	}

	if( eHMObj::eDpC == ( iHMObjToCheck & eHMObj::eDpC ) )
	{
		if( true == IsDpCExist() && false == GetpDpC()->IsCompletelyDefined() )
		{
			return false;
		}
	}

	if( eHMObj::eCV == ( iHMObjToCheck & eHMObj::eCV ) )
	{
		if( true == IsCvExist() && false == GetpCV()->IsCompletelyDefined() )
		{
			return false;
		}
	}

	if( eHMObj::eShutoffValveSupply == ( iHMObjToCheck & eHMObj::eShutoffValveSupply ) )
	{
		if( NULL != GetpShutoffValve( eHMObj::eShutoffValveSupply ) && false == GetpShutoffValve( eHMObj::eShutoffValveSupply )->IsCompletelyDefined() )
		{
			return false;
		}
	}

	if( eHMObj::eShutoffValveReturn == ( iHMObjToCheck & eHMObj::eShutoffValveReturn ) )
	{
		if( NULL != GetpShutoffValve( eHMObj::eShutoffValveReturn ) && false == GetpShutoffValve( eHMObj::eShutoffValveReturn )->IsCompletelyDefined() )
		{
			return false;
		}
	}

	if( eHMObj::eSmartControlValve == ( iHMObjToCheck & eHMObj::eSmartControlValve ) )
	{
		if( true == IsSmartControlValveExist() && false == GetpSmartControlValve()->IsCompletelyDefined() )
		{
			return false;
		}
	}

	if( eHMObj::eSmartDpC == ( iHMObjToCheck & eHMObj::eSmartDpC ) )
	{
		if( true == IsSmartDpCExist() && false == GetpSmartDpC()->IsCompletelyDefined() )
		{
			return false;
		}
	}

	return true;
}

bool CDS_HydroMod::IsBvExist( bool bAndSelected )
{
	if( NULL != m_pBv && ( false == bAndSelected || NULL != m_pBv->GetIDPtr().MP ) )
	{
		return  true;
	}

	return false;
}

bool CDS_HydroMod::IsBvBypExist( bool bAndSelected )
{
	if( NULL != m_pBypBv && ( false == bAndSelected || NULL != m_pBypBv->GetIDPtr().MP ) )
	{
		return true;
	}

	return false;
}

bool CDS_HydroMod::IsBvSecExist( bool bAndSelected )
{
	if( NULL != m_pSecBv && ( false == bAndSelected || NULL != m_pSecBv->GetIDPtr().MP ) )
	{
		return true;
	}

	return false;
}

bool CDS_HydroMod::IsDpCExist( bool bAndSelected )
{
	if( NULL != m_pDpC && ( false == bAndSelected || NULL != m_pDpC->GetIDPtr().MP ) )
	{
		return true;
	}

	return false;
}

bool CDS_HydroMod::IsTermUnitExist()
{
	return ( NULL != m_pTermUnit ) ? true : false;
}

bool CDS_HydroMod::IsPressureInterfaceExist()
{
	if( NULL != GetpSchcat() )
	{
		if( true == GetpSchcat()->IsSecondarySideExist() )
		{
			return true;
		}

		if( true == IsDpCExist() && CDS_HydroMod::ePressIntType::pitDpC == GetPressIntType()
				&& NULL != GetpSch() && DpStabOnBranch == GetpSch()->GetDpStab() )
		{
			return true;
		}

	}

	return false;
}

bool CDS_HydroMod::IsPumpExist()
{
	return ( NULL != m_pPump ) ? true : false;
}

bool CDS_HydroMod::IsCvExist( bool bAndSelected )
{
	if( NULL == m_pCv )
	{
		return false;
	}

	if( true == m_pCv->IsTaCV() && true == bAndSelected && NULL == m_pCv->GetCvIDPtr().MP )
	{
		// In case we are in TA valve mode and we want verify if valve is selected and this one is NULL, we return false.
		return false;
	}

	// We are in TA valve mode but we don't want verify if valve is selected or we are in Kv mode (CvIDPtr in this case can be NULL).
	return true;
}

bool CDS_HydroMod::IsShutoffValveExist( eHMObj eLocated, bool bAndSelected )
{
	if( eShutoffValveSupply != eLocated && eShutoffValveReturn != eLocated )
	{
		return false;
	}

	if( eShutoffValveSupply == eLocated && NULL != m_pShutoffValveSupply && ( false == bAndSelected || NULL != m_pShutoffValveSupply->GetIDPtr().MP ) )
	{
		return true;
	}
	else if( eShutoffValveReturn == eLocated && NULL != m_pShutoffValveReturn && ( false == bAndSelected || NULL != m_pShutoffValveReturn->GetIDPtr().MP ) )
	{
		return true;
	}

	return false;
}

bool CDS_HydroMod::IsShutoffValveExist( bool bAndSelected )
{
	bool bExist = false;

	if( true == IsShutoffValveExist( eHMObj::eShutoffValveSupply, bAndSelected ) || true == IsShutoffValveExist( eHMObj::eShutoffValveReturn, bAndSelected ) )
	{
		bExist = true;
	}

	return bExist;
}

bool CDS_HydroMod::IsSmartControlValveExist( bool bAndSelected )
{
	if( NULL != m_pSmartControlValve && ( false == bAndSelected || NULL != m_pSmartControlValve->GetIDPtr().MP ) )
	{
		return true;
	}

	return false;
}

bool CDS_HydroMod::IsSmartDpCExist( bool bAndSelected )
{
	if( NULL != m_pSmartDpC && ( false == bAndSelected || NULL != m_pSmartDpC->GetIDPtr().MP ) )
	{
		return true;
	}

	return false;
}

bool CDS_HydroMod::IsPipeExist( eHMObj ePipeLocation, bool bAndSelected )
{
	bool bIsExist = false;

	switch( ePipeLocation) 
	{
		case eHMObj::eCircuitPrimaryPipe:
			bIsExist |= ( NULL != GetpCircuitPrimaryPipe() ) ? true : false;
			bIsExist &= ( true == bAndSelected && NULL == GetpCircuitPrimaryPipe()->GetIDPtr().MP ) ? false : true;
			break;

		case eHMObj::eCircuitBypassPrimaryPipe:
			bIsExist |= ( NULL != GetpCircuitBypassPipe() ) ? true : false;
			bIsExist &= ( true == bAndSelected && NULL == GetpCircuitBypassPipe()->GetIDPtr().MP ) ? false : true;
			break;

		case eHMObj::eCircuitSecondaryPipe:
			bIsExist |= ( NULL != GetpCircuitSecondaryPipe() ) ? true : false;
			bIsExist &= ( true == bAndSelected && NULL == GetpCircuitSecondaryPipe()->GetIDPtr().MP ) ? false : true;
			break;

		case eHMObj::eDistributionSupplyPipe:
			bIsExist |= ( NULL != GetpDistrSupplyPipe() ) ? true : false;
			bIsExist &= ( true == bAndSelected && NULL == GetpDistrSupplyPipe()->GetIDPtr().MP ) ? false : true;
			break;

		case eHMObj::eDistributionReturnPipe:
			bIsExist |= ( NULL != GetpDistrReturnPipe() ) ? true : false;
			bIsExist &= ( true == bAndSelected && NULL == GetpDistrReturnPipe()->GetIDPtr().MP ) ? false : true;
			break;
	}

	return bIsExist;
}

bool CDS_HydroMod::IsEmpty( void )
{
	bool bIsExist = IsBvExist( true ) || IsBvSecExist( true ) || IsBvBypExist( true ) || IsCvExist( true ) || IsDpCExist( true ) ||
			IsShutoffValveExist( eHMObj::eShutoffValveSupply, true ) || IsShutoffValveExist( eHMObj::eShutoffValveReturn, true );

	bIsExist |= ( NULL != GetpCircuitPrimaryPipe() );
	bIsExist |= ( NULL != GetpDistrSupplyPipe() );
	bIsExist |= ( NULL != GetpDistrReturnPipe() );
	bIsExist |= ( NULL != GetpCircuitSecondaryPipe() );

	return !bIsExist;
}

// HYS-484: Remove all isCompletelyDefined calls
bool CDS_HydroMod::IsLocked( eHMObj eObj )
{
	if( false == IsForHub() && true == GetpPrjParam()->IsFreezed() )
	{
		return true;
	}

	bool bSomethingIsLocked = false;

	switch( eObj )
	{
		case eHMObj::eALL:

			if( true == IsBvExist() )
			{
				// Primary balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				bSomethingIsLocked = true;

				if( NULL == GetpBv()->GetIDPtr().MP )
				{
					// The primary balancing valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eBVprim ) != eHMObj::eBVprim )
				{
					// The primary balancing valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			if( false == bSomethingIsLocked && true == IsBvSecExist() )
			{
				// Secondary balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				bSomethingIsLocked = true;

				if( NULL == GetpSecBv()->GetIDPtr().MP )
				{
					// The secondary balancing valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eBVsec ) != eHMObj::eBVsec )
				{
					// The secondary balancing valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			if( false == bSomethingIsLocked && true == IsBvBypExist() )
			{
				// Bypass balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				bSomethingIsLocked = true;

				if( NULL == GetpBypBv()->GetIDPtr().MP )
				{
					// The bypass secondary balancing valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eBVbyp ) != eHMObj::eBVbyp )
				{
					// The bypass secondary balancing valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			if( false == bSomethingIsLocked && NULL != GetpCircuitPrimaryPipe() )
			{
				bSomethingIsLocked = true;

				if( ( m_usLock & eHMObj::eCircuitPrimaryPipe ) != eHMObj::eCircuitPrimaryPipe )
				{
					bSomethingIsLocked = false;
				}
			}

			if( false == bSomethingIsLocked && NULL != GetpCircuitBypassPipe() )
			{
				bSomethingIsLocked = true;

				if( ( m_usLock & eHMObj::eCircuitBypassPrimaryPipe ) != eHMObj::eCircuitBypassPrimaryPipe )
				{
					bSomethingIsLocked = false;
				}
			}

			if( false == bSomethingIsLocked && NULL != GetpDistrSupplyPipe() )
			{
				bSomethingIsLocked = true;

				if( ( m_usLock & eHMObj::eDistributionSupplyPipe ) != eHMObj::eDistributionSupplyPipe )
				{
					bSomethingIsLocked = false;
				}
			}

			if( false == bSomethingIsLocked && ReturnType::Reverse == m_eReturnType &&  NULL != GetpDistrReturnPipe() )
			{
				bSomethingIsLocked = true;

				if( ( m_usLock & eHMObj::eDistributionReturnPipe ) != eHMObj::eDistributionReturnPipe )
				{
					bSomethingIsLocked = false;
				}
			}

			if( false == bSomethingIsLocked && GetpCircuitSecondaryPipe() &&  NULL != GetpCircuitSecondaryPipe()->GetLength() )
			{
				bSomethingIsLocked = true;

				if( ( m_usLock & eHMObj::eCircuitSecondaryPipe ) != eHMObj::eCircuitSecondaryPipe )
				{
					bSomethingIsLocked = false;
				}
			}

			if( false == bSomethingIsLocked && true == IsCvExist() )
			{
				// Control valve exists in the circuit scheme and there is a 'CDS_Hydromod::CCv' created.
				bSomethingIsLocked = true;

				if( (true == GetpCV()->IsTaCV() &&  NULL == GetpCV()->GetCvIDPtr().MP ) || ( false == GetpCV()->IsTaCV() && GetpCV()->GetKvs() <= 0 ) )
				{
					// The control valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eCV ) != eHMObj::eCV )
				{
					// The control valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			if( false == bSomethingIsLocked && true == IsDpCExist() )
			{
				// Differential pressure controller valve exists in the circuit scheme and there is a 'CDS_Hydromod::CDpC' created.
				bSomethingIsLocked = true;

				if( NULL == GetpDpC()->GetIDPtr().MP )
				{
					// The differential pressure controller valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eDpC ) != eHMObj::eDpC )
				{
					// The differential pressure controller valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			if( false == bSomethingIsLocked && true == IsShutoffValveExist( eHMObj::eShutoffValveSupply ) )
			{
				// Supply shutoff valve exists in the circuit scheme and there is a 'CDS_Hydromod::CShutoffValve' created.
				bSomethingIsLocked = true;

				if( NULL == GetpShutoffValve( eHMObj::eShutoffValveSupply )->GetIDPtr().MP )
				{
					// The supply shutoff valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eShutoffValveSupply ) != eHMObj::eShutoffValveSupply )
				{
					// The supply shutoff valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			if( false == bSomethingIsLocked && true == IsShutoffValveExist( eHMObj::eShutoffValveReturn ) )
			{
				// Return shutoff valve exists in the circuit scheme and there is a 'CDS_Hydromod::CShutoffValve' created.
				bSomethingIsLocked = true;

				if( NULL == GetpShutoffValve( eHMObj::eShutoffValveReturn )->GetIDPtr().MP )
				{
					// The return shutoff valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eShutoffValveReturn ) != eHMObj::eShutoffValveReturn )
				{
					// The return shutoff valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			if( false == bSomethingIsLocked && true == IsSmartControlValveExist() )
			{
				// Primary smart control valve exists in the circuit scheme and there is a 'CDS_Hydromod::CSmartControlValve' created.
				bSomethingIsLocked = true;

				if( NULL == GetpSmartControlValve()->GetIDPtr().MP )
				{
					// The primary smart control valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eSmartControlValve ) != eHMObj::eSmartControlValve )
				{
					// The primary smart control valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			if( false == bSomethingIsLocked && true == IsSmartDpCExist() )
			{
				// Primary smart differential pressure controller exists in the circuit scheme and there is a 'CDS_Hydromod::CSmartDpC' created.
				bSomethingIsLocked = true;

				if( NULL == GetpSmartDpC()->GetIDPtr().MP )
				{
					// The primary smart differential pressure controller is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eSmartDpC ) != eHMObj::eSmartDpC )
				{
					// The primary smart differential pressure controller is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			break;

		case eHMObj::eBVprim:

			if( true == IsBvExist() )
			{
				// Primary balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				bSomethingIsLocked = true;

				if( NULL == GetpBv()->GetIDPtr().MP )
				{
					// The primary balancing valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eBVprim ) != eHMObj::eBVprim )
				{
					// The primary balancing valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			break;

		case eHMObj::eBVsec:

			if( true == IsBvSecExist() )
			{
				// Secondary balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				bSomethingIsLocked = true;

				if( NULL == GetpSecBv()->GetIDPtr().MP )
				{
					// The secondary balancing valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eBVsec ) != eHMObj::eBVsec )
				{
					// The secondary balancing valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			break;

		case eHMObj::eBVbyp:

			if( true == IsBvBypExist() )
			{
				// Bypass balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				bSomethingIsLocked = true;

				if( NULL == GetpBypBv()->GetIDPtr().MP )
				{
					// The bypass balancing valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eBVbyp ) != eHMObj::eBVbyp )
				{
					// The bypass balancing valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			break;

		case eHMObj::eCircuitPrimaryPipe:

			if( NULL != GetpCircuitPrimaryPipe() )
			{
				bSomethingIsLocked = true;

				if( ( m_usLock & eObj ) != eHMObj::eCircuitPrimaryPipe )
				{
					bSomethingIsLocked = false;
				}
			}

			break;

		case eHMObj::eCircuitBypassPrimaryPipe:

			if( NULL != GetpCircuitBypassPipe() )
			{
				bSomethingIsLocked = true;

				if( ( m_usLock & eObj ) != eHMObj::eCircuitBypassPrimaryPipe )
				{
					bSomethingIsLocked = false;
				}
			}

			break;

		case eHMObj::eDistributionSupplyPipe:

			if( NULL != GetpDistrSupplyPipe() )
			{
				bSomethingIsLocked = true;

				if( ( m_usLock & eObj ) != eHMObj::eDistributionSupplyPipe )
				{
					bSomethingIsLocked = false;
				}
			}

			break;

		case eHMObj::eDistributionReturnPipe:

			if( ReturnType::Reverse == m_eReturnType && NULL != GetpDistrReturnPipe() )
			{
				bSomethingIsLocked = true;

				if( ( m_usLock & eObj ) != eHMObj::eDistributionReturnPipe )
				{
					bSomethingIsLocked = false;
				}
			}

			break;

		case eHMObj::eCircuitSecondaryPipe:

			if( NULL != GetpCircuitSecondaryPipe() )
			{
				bSomethingIsLocked = true;

				if( ( m_usLock & eObj ) != eHMObj::eCircuitSecondaryPipe )
				{
					bSomethingIsLocked = false;
				}
			}

			break;

		case eHMObj::eCV:

			if( true == IsCvExist() )
			{
				// Control valve exists in the circuit scheme and there is a 'CDS_Hydromod::CCv' created.
				bSomethingIsLocked = true;

				if( ( true == GetpCV()->IsTaCV() && NULL == GetpCV()->GetCvIDPtr().MP ) || ( false == GetpCV()->IsTaCV() && GetpCV()->GetKvs() <= 0 ) )
				{
					// The control valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eCV ) != eHMObj::eCV )
				{
					// The control valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			break;

		case eHMObj::eDpC:

			if( true == IsDpCExist() )
			{
				// Differential pressure controller valve exists in the circuit scheme and there is a 'CDS_Hydromod::CDpC' created.
				bSomethingIsLocked = true;

				if( NULL == GetpDpC()->GetIDPtr().MP )
				{
					// The differential pressure controller valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eDpC ) != eHMObj::eDpC )
				{
					// The differential pressure controller valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			break;

		case eHMObj::eShutoffValveSupply:

			if( true == IsShutoffValveExist( eHMObj::eShutoffValveSupply ) )
			{
				// Supply shutoff valve exists in the circuit scheme and there is a 'CDS_Hydromod::CShutoffValve' created.
				bSomethingIsLocked = true;

				if( NULL == GetpShutoffValve( eHMObj::eShutoffValveSupply )->GetIDPtr().MP )
				{
					// The supply shutoff valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eShutoffValveSupply ) != eHMObj::eShutoffValveSupply )
				{
					// The supply shutoff valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			break;

		case eHMObj::eShutoffValveReturn:

			if( true == IsShutoffValveExist( eHMObj::eShutoffValveReturn ) )
			{
				// Return shutoff valve exists in the circuit scheme and there is a 'CDS_Hydromod::CShutoffValve' created.
				bSomethingIsLocked = true;

				if( NULL == GetpShutoffValve( eHMObj::eShutoffValveReturn )->GetIDPtr().MP )
				{
					// The return shutoff valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eShutoffValveReturn ) != eHMObj::eShutoffValveReturn )
				{
					// The return shutoff valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			break;

		case eHMObj::eSmartControlValve:

			if( true == IsSmartControlValveExist() )
			{
				// Smart control valve exists in the circuit scheme and there is a 'CDS_Hydromod::CSmartControlValve' created.
				bSomethingIsLocked = true;

				if( NULL == GetpSmartControlValve()->GetIDPtr().MP )
				{
					// The smart control valve is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eSmartControlValve ) != eHMObj::eSmartControlValve )
				{
					// The smart control valve is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			break;

		case eHMObj::eSmartDpC:

			if( true == IsSmartDpCExist() )
			{
				// Smart differential pressure controller exists in the circuit scheme and there is a 'CDS_Hydromod::CSmartDifferrentialPressureController' created.
				bSomethingIsLocked = true;

				if( NULL == GetpSmartDpC()->GetIDPtr().MP )
				{
					// The smart differential pressure controller is not completely defined, we return 'false'.
					bSomethingIsLocked = false;
				}
				else if( ( m_usLock & eHMObj::eSmartDpC ) != eHMObj::eSmartDpC )
				{
					// The smart differential pressure controller is defined and it is not locked.
					bSomethingIsLocked = false;
				}
			}

			break;

		default:
			ASSERT( 0 );
			break;
	}

	return bSomethingIsLocked;
}

bool CDS_HydroMod::IsAtLeastOneObjectLocked( bool bExcludeCPForModule )
{
	bool bAtLeastOneObjectLocked = IsLocked( eHMObj::eALL );

	if( true == IsBvExist() &&  NULL != GetpBv()->GetIDPtr().MP )
	{
		if( eHMObj::eBVprim == ( m_usLock & eHMObj::eBVprim ) )
		{
			bAtLeastOneObjectLocked = true;
		}
	}

	if( false == bAtLeastOneObjectLocked && true == IsBvSecExist() && NULL != GetpSecBv()->GetIDPtr().MP )
	{
		if( eHMObj::eBVsec == ( m_usLock & eHMObj::eBVsec ) )
		{
			bAtLeastOneObjectLocked = true;
		}
	}

	if( false == bAtLeastOneObjectLocked && true == IsBvBypExist() && NULL != GetpBypBv()->GetIDPtr().MP )
	{
		if( eHMObj::eBVbyp == ( m_usLock & eHMObj::eBVbyp ) )
		{
			bAtLeastOneObjectLocked = true;
		}
	}

	if( false == bAtLeastOneObjectLocked && NULL != GetpCircuitPrimaryPipe() )
	{
		if( true == bExcludeCPForModule && true == IsaModule() && NULL != GetpSchcat() && false == GetpSchcat()->IsSecondarySideExist()
				&& NULL != GetFirstHMChild() && NULL != GetFirstHMChild()->GetpDistrSupplyPipe() )
		{
			// If we must exclude circuit primary pipe for module, we are on a module without secondary side and there is already a 
			// child defined, we don't check the lock status.
		}
		else if( eHMObj::eCircuitPrimaryPipe == ( m_usLock & eHMObj::eCircuitPrimaryPipe ) )
		{
			bAtLeastOneObjectLocked = true;
		}
	}

	if( false == bAtLeastOneObjectLocked && NULL != GetpCircuitBypassPipe() )
	{
		if( true == bExcludeCPForModule && true == IsaModule() && NULL != GetpSchcat() && CAnchorPt::CircuitSide::CircuitSide_Primary == GetBypassPipeSide()
				&& NULL != GetFirstHMChild() && NULL != GetFirstHMChild()->GetpDistrSupplyPipe() )
		{
			// If we must exclude circuit primary pipe for module, we are on a module with the bypass pipe at the primary side and there is already a 
			// child defined, we don't check the lock status.
		}
		else if( eHMObj::eCircuitBypassPrimaryPipe == ( m_usLock & eHMObj::eCircuitBypassPrimaryPipe ) )
		{
			bAtLeastOneObjectLocked = true;
		}
	}

	if( false == bAtLeastOneObjectLocked && NULL != GetpDistrSupplyPipe() )
	{
		if( eHMObj::eDistributionSupplyPipe == ( m_usLock & eHMObj::eDistributionSupplyPipe ) )
		{
			bAtLeastOneObjectLocked = true;
		}
	}

	if( false == bAtLeastOneObjectLocked && ReturnType::Reverse == m_eReturnType && NULL != GetpDistrReturnPipe() )
	{
		if( eHMObj::eDistributionReturnPipe == ( m_usLock & eHMObj::eDistributionReturnPipe ) )
		{
			bAtLeastOneObjectLocked = true;
		}
	}

	if( false == bAtLeastOneObjectLocked &&  NULL != GetpCircuitSecondaryPipe() && NULL != GetpCircuitSecondaryPipe() )
	{
		if( true == bExcludeCPForModule && true == IsaModule() && NULL != GetpSchcat() && true == GetpSchcat()->IsSecondarySideExist()
			&& NULL != GetFirstHMChild() && NULL != GetFirstHMChild()->GetpDistrSupplyPipe() )
		{
			// Do nothing
		}
		else if( eHMObj::eCircuitSecondaryPipe == ( m_usLock & eHMObj::eCircuitSecondaryPipe ) )
		{
			bAtLeastOneObjectLocked = true;
		}
	}

	if( false == bAtLeastOneObjectLocked && true == IsCvExist() && 
		( ( true == GetpCV()->IsTaCV() && NULL != GetpCV()->GetCvIDPtr().MP ) || ( false == GetpCV()->IsTaCV() && GetpCV()->GetKvs() > 0 ) ) )
	{
		if( eHMObj::eCV == ( m_usLock & eHMObj::eCV ) )
		{
			bAtLeastOneObjectLocked = true;
		}
	}

	if( false == bAtLeastOneObjectLocked && true == IsDpCExist() && NULL != GetpDpC()->GetIDPtr().MP )
	{
		if( eHMObj::eDpC == ( m_usLock & eHMObj::eDpC ) )
		{
			bAtLeastOneObjectLocked = true;
		}
	}

	if( false == bAtLeastOneObjectLocked && true == IsShutoffValveExist( eHMObj::eShutoffValveSupply ) && NULL != GetpShutoffValve( eHMObj::eShutoffValveSupply )->GetIDPtr().MP )
	{
		if( eHMObj::eShutoffValveSupply == ( m_usLock & eHMObj::eShutoffValveSupply ) )
		{
			bAtLeastOneObjectLocked = true;
		}
	}

	if( false == bAtLeastOneObjectLocked && true == IsShutoffValveExist( eHMObj::eShutoffValveReturn ) && NULL != GetpShutoffValve( eHMObj::eShutoffValveReturn )->GetIDPtr().MP )
	{
		if( eHMObj::eShutoffValveReturn == ( m_usLock & eHMObj::eShutoffValveReturn ) )
		{
			bAtLeastOneObjectLocked = true;
		}
	}

	if( false == bAtLeastOneObjectLocked && true == IsSmartControlValveExist() && NULL != GetpSmartControlValve()->GetIDPtr().MP )
	{
		if( eHMObj::eSmartControlValve == ( m_usLock & eHMObj::eSmartControlValve ) )
		{
			bAtLeastOneObjectLocked = true;
		}
	}

	if( false == bAtLeastOneObjectLocked && true == IsSmartDpCExist() && NULL != GetpSmartDpC()->GetIDPtr().MP )
	{
		if( eHMObj::eSmartDpC == ( m_usLock & eHMObj::eSmartDpC ) )
		{
			bAtLeastOneObjectLocked = true;
		}
	}

	return bAtLeastOneObjectLocked;
}

bool CDS_HydroMod::IsAtLeastOneObjectUnlocked( bool bExcludeCPForModule )
{
	bool bAtLeastOneObjectUnlocked = false;

	if( true == IsBvExist() && NULL != GetpBv()->GetIDPtr().MP )
	{
		if( eHMObj::eBVprim != ( m_usLock & eHMObj::eBVprim ) )
		{
			bAtLeastOneObjectUnlocked = true;
		}
	}

	if( false == bAtLeastOneObjectUnlocked && true == IsBvSecExist() && NULL != GetpSecBv()->GetIDPtr().MP )
	{
		if( eHMObj::eBVsec != ( m_usLock & eHMObj::eBVsec ) )
		{
			bAtLeastOneObjectUnlocked = true;
		}
	}

	if( false == bAtLeastOneObjectUnlocked && true == IsBvBypExist() && NULL != GetpBypBv()->GetIDPtr().MP )
	{
		if( eHMObj::eBVbyp != ( m_usLock & eHMObj::eBVbyp ) )
		{
			bAtLeastOneObjectUnlocked = true;
		}
	}

	if( NULL != GetpCircuitPrimaryPipe() && false == bAtLeastOneObjectUnlocked )
	{
		if( true == bExcludeCPForModule && true == IsaModule() && NULL != GetpSchcat() && false == GetpSchcat()->IsSecondarySideExist()
				&& NULL != GetFirstHMChild() && NULL != GetFirstHMChild()->GetpDistrSupplyPipe() )
		{
			// If we must exclude circuit primary pipe for module, we are on a module without secondary side and there is already a 
			// child defined, we don't check the lock status.
		}
		else if( eHMObj::eCircuitPrimaryPipe != ( m_usLock & eHMObj::eCircuitPrimaryPipe ) )
		{
			bAtLeastOneObjectUnlocked = true;
		}
	}

	if( NULL != GetpCircuitBypassPipe() && false == bAtLeastOneObjectUnlocked )
	{
		if( true == bExcludeCPForModule && true == IsaModule() && NULL != GetpSchcat() && CAnchorPt::CircuitSide::CircuitSide_Primary == GetBypassPipeSide()
				&& NULL != GetFirstHMChild() && NULL != GetFirstHMChild()->GetpDistrSupplyPipe() )
		{
			// If we must exclude circuit primary pipe for module, we are on a module with the bypass pipe at the primary side and there is already a 
			// child defined, we don't check the lock status.
		}
		else if( eHMObj::eCircuitBypassPrimaryPipe != ( m_usLock & eHMObj::eCircuitBypassPrimaryPipe ) )
		{
			bAtLeastOneObjectUnlocked = true;
		}
	}

	if( NULL != GetpDistrSupplyPipe() && false == bAtLeastOneObjectUnlocked )
	{
		if( eHMObj::eDistributionSupplyPipe != ( m_usLock & eHMObj::eDistributionSupplyPipe ) )
		{
			bAtLeastOneObjectUnlocked = true;
		}
	}

	if( NULL != GetpDistrReturnPipe() && false == bAtLeastOneObjectUnlocked && ReturnType::Reverse == m_eReturnType )
	{
		if( eHMObj::eDistributionReturnPipe != ( m_usLock & eHMObj::eDistributionReturnPipe ) )
		{
			bAtLeastOneObjectUnlocked = true;
		}
	}

	if( NULL != GetpCircuitSecondaryPipe() && false == bAtLeastOneObjectUnlocked )
	{
		if( true == bExcludeCPForModule && true == IsaModule() && NULL != GetpSchcat() && true == GetpSchcat()->IsSecondarySideExist()
			&& NULL != GetFirstHMChild() && NULL != GetFirstHMChild()->GetpDistrSupplyPipe() )
		{
			// Do nothing
		}
		else if( eHMObj::eCircuitSecondaryPipe != ( m_usLock & eHMObj::eCircuitSecondaryPipe ) )
		{
			bAtLeastOneObjectUnlocked = true;
		}
	}

	if( false == bAtLeastOneObjectUnlocked && true == IsCvExist() &&
		( ( true == GetpCV()->IsTaCV() && NULL != GetpCV()->GetCvIDPtr().MP ) || ( false == GetpCV()->IsTaCV() && GetpCV()->GetKvs() > 0 ) ) )
	{
		if( eHMObj::eCV != ( m_usLock & eHMObj::eCV ) )
		{
			bAtLeastOneObjectUnlocked = true;
		}
	}

	if( false == bAtLeastOneObjectUnlocked && true == IsDpCExist() && NULL != GetpDpC()->GetIDPtr().MP )
	{
		if( eHMObj::eDpC != ( m_usLock & eHMObj::eDpC ) )
		{
			bAtLeastOneObjectUnlocked = true;
		}
	}

	if( false == bAtLeastOneObjectUnlocked && true == IsShutoffValveExist( eHMObj::eShutoffValveSupply ) && NULL != GetpShutoffValve( eHMObj::eShutoffValveSupply )->GetIDPtr().MP )
	{
		if( eHMObj::eShutoffValveSupply != ( m_usLock & eHMObj::eShutoffValveSupply ) )
		{
			bAtLeastOneObjectUnlocked = true;
		}
	}

	if( false == bAtLeastOneObjectUnlocked && true == IsShutoffValveExist( eHMObj::eShutoffValveReturn ) && NULL != GetpShutoffValve( eHMObj::eShutoffValveReturn )->GetIDPtr().MP )
	{
		if( eHMObj::eShutoffValveReturn != ( m_usLock & eHMObj::eShutoffValveReturn ) )
		{
			bAtLeastOneObjectUnlocked = true;
		}
	}

	if( false == bAtLeastOneObjectUnlocked && true == IsSmartControlValveExist() && NULL != GetpSmartControlValve()->GetIDPtr().MP )
	{
		if( eHMObj::eSmartControlValve != ( m_usLock & eHMObj::eSmartControlValve ) )
		{
			bAtLeastOneObjectUnlocked = true;
		}
	}

	if( false == bAtLeastOneObjectUnlocked && true == IsSmartDpCExist() && NULL != GetpSmartDpC()->GetIDPtr().MP )
	{
		if( eHMObj::eSmartDpC != ( m_usLock & eHMObj::eSmartDpC ) )
		{
			bAtLeastOneObjectUnlocked = true;
		}
	}

	return bAtLeastOneObjectUnlocked;
}

bool CDS_HydroMod::IsAtLeastOneObjectLockedRecursive( bool bExcludeRoot, bool bExcludeCPForModule )
{
	if( false == bExcludeRoot && true == IsAtLeastOneObjectLocked( bExcludeCPForModule ) )
	{
		return true;
	}

	bool bAtLeastOneObjectLocked = false;

	for( IDPTR idptr = GetFirst(); NULL != idptr.MP; idptr = GetNext( idptr.MP ) )
	{
		CDS_HydroMod *pclChildHM = dynamic_cast<CDS_HydroMod *>( idptr.MP );

		if( NULL == pclChildHM )
		{
			continue;
		}

		if( true == pclChildHM->IsaModule() )
		{
			bAtLeastOneObjectLocked = pclChildHM->IsAtLeastOneObjectLockedRecursive( false, bExcludeCPForModule );
		}
		else
		{
			bAtLeastOneObjectLocked = pclChildHM->IsAtLeastOneObjectLocked( bExcludeCPForModule );
		}

		if( true == bAtLeastOneObjectLocked )
		{
			break;
		}
	}

	return bAtLeastOneObjectLocked;
}

bool CDS_HydroMod::IsAtLeastOneObjectUnlockedRecursive( bool bExcludeRoot, bool bExcludeCPForModule )
{
	if( false == bExcludeRoot && true == IsAtLeastOneObjectUnlocked( bExcludeCPForModule ) )
	{
		return true;
	}

	bool bAtLeastOneObjectUnLocked = false;

	for( IDPTR idptr = GetFirst(); NULL != idptr.MP; idptr = GetNext( idptr.MP ) )
	{
		CDS_HydroMod *pclChildHM = dynamic_cast<CDS_HydroMod *>( idptr.MP );

		if( NULL == pclChildHM )
		{
			continue;
		}

		if( true == pclChildHM->IsaModule() )
		{
			bAtLeastOneObjectUnLocked = pclChildHM->IsAtLeastOneObjectUnlockedRecursive( false, bExcludeCPForModule );
		}
		else
		{
			bAtLeastOneObjectUnLocked = pclChildHM->IsAtLeastOneObjectUnlocked( bExcludeCPForModule );
		}

		if( true == bAtLeastOneObjectUnLocked )
		{
			break;
		}
	}

	return bAtLeastOneObjectUnLocked;
}

eBool3 CDS_HydroMod::IsLockedTriState( eHMObj eObj )
{
	if( NULL == IsForHub() && true == GetpPrjParam()->IsFreezed() )
	{
		return eBool3::eb3False;
	}

	eBool3 eSomethingIsLocked = eBool3::eb3Undef;

	switch( eObj )
	{
		case eHMObj::eALL:

			if( true == IsBvExist() )
			{
				// Primary balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpBv()->IsCompletelyDefined() )
				{
					// The primary balancing valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eBVprim ) != eHMObj::eBVprim )
				{
					// The primary balancing valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			if( eBool3::eb3True != eSomethingIsLocked && true == IsBvSecExist() )
			{
				// Secondary balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpSecBv()->IsCompletelyDefined() )
				{
					// The secondary balancing valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eBVsec ) != eHMObj::eBVsec )
				{
					// The secondary balancing valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			if( eBool3::eb3True != eSomethingIsLocked && true == IsBvBypExist() )
			{
				// Bypass balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpBypBv()->IsCompletelyDefined() )
				{
					// The bypass balancing valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eBVbyp ) != eHMObj::eBVbyp )
				{
					// The bypass balancing valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			if( eBool3::eb3True != eSomethingIsLocked && NULL != GetpCircuitPrimaryPipe() )
			{
				eSomethingIsLocked = eBool3::eb3True;

				if( ( m_usLock & eHMObj::eCircuitPrimaryPipe ) != eHMObj::eCircuitPrimaryPipe )
				{
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			if( eBool3::eb3True != eSomethingIsLocked && NULL != GetpCircuitBypassPipe() )
			{
				eSomethingIsLocked = eBool3::eb3True;

				if( ( m_usLock & eHMObj::eCircuitBypassPrimaryPipe ) != eHMObj::eCircuitBypassPrimaryPipe )
				{
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			if( eBool3::eb3True != eSomethingIsLocked && NULL != GetpDistrSupplyPipe() )
			{
				eSomethingIsLocked = eBool3::eb3True;

				if( ( m_usLock & eHMObj::eDistributionSupplyPipe ) != eHMObj::eDistributionSupplyPipe )
				{
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			if( eBool3::eb3True != eSomethingIsLocked && NULL != GetpDistrReturnPipe() && ReturnType::Reverse == m_eReturnType )
			{
				eSomethingIsLocked = eBool3::eb3True;

				if( ( m_usLock & eHMObj::eDistributionReturnPipe ) != eHMObj::eDistributionReturnPipe )
				{
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			if( eBool3::eb3True != eSomethingIsLocked && NULL != GetpCircuitSecondaryPipe() )
			{
				eSomethingIsLocked = eBool3::eb3True;

				if( ( m_usLock & eHMObj::eCircuitSecondaryPipe ) != eHMObj::eCircuitSecondaryPipe )
				{
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			if( eBool3::eb3True != eSomethingIsLocked && true == IsCvExist() )
			{
				// Control valve exists in the circuit scheme and there is a 'CDS_Hydromod::CCv' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpCV()->IsCompletelyDefined() )
				{
					// The control valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eCV ) != eHMObj::eCV )
				{
					// The control valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			if( eBool3::eb3True != eSomethingIsLocked && true == IsDpCExist() )
			{
				// Differential pressure controller valve exists in the circuit scheme and there is a 'CDS_Hydromod::CDpC' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpDpC()->IsCompletelyDefined() )
				{
					// The differential pressure controller valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eDpC ) != eHMObj::eDpC )
				{
					// The differential pressure controller valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			if( eBool3::eb3True != eSomethingIsLocked && true == IsShutoffValveExist( eHMObj::eShutoffValveSupply ) )
			{
				// Supply shutoff valve exists in the circuit scheme and there is a 'CDS_Hydromod::CShutoffValve' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpShutoffValve( eHMObj::eShutoffValveSupply )->IsCompletelyDefined() )
				{
					// The supply shutoff valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eShutoffValveSupply ) != eHMObj::eShutoffValveSupply )
				{
					// The supply shutoff valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			if( eBool3::eb3True != eSomethingIsLocked && true == IsShutoffValveExist( eHMObj::eShutoffValveReturn ) )
			{
				// Return shutoff valve exists in the circuit scheme and there is a 'CDS_Hydromod::CShutoffValve' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpShutoffValve( eHMObj::eShutoffValveReturn )->IsCompletelyDefined() )
				{
					// The return shutoff valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eShutoffValveReturn ) != eHMObj::eShutoffValveReturn )
				{
					// The return shutoff valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			if( eBool3::eb3True != eSomethingIsLocked && true == IsSmartControlValveExist() )
			{
				// Smart control valve exists in the circuit scheme and there is a 'CDS_Hydromod::CSmartControlValve' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpSmartControlValve()->IsCompletelyDefined() )
				{
					// The smart control valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eSmartControlValve ) != eHMObj::eSmartControlValve )
				{
					// The smart control valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			if( eBool3::eb3True != eSomethingIsLocked && true == IsSmartDpCExist() )
			{
				// Smart differential pressure controller exists in the circuit scheme and there is a 'CDS_Hydromod::CSmartDpC' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpSmartDpC()->IsCompletelyDefined() )
				{
					// The smart differential pressure controller is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eSmartDpC ) != eHMObj::eSmartDpC )
				{
					// The smart differential pressure controller is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		case eHMObj::eBVprim:

			if( true == IsBvExist() )
			{
				// Primary balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpBv()->IsCompletelyDefined() )
				{
					// The primary balancing valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eBVprim ) != eHMObj::eBVprim )
				{
					// The primary balancing valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		case eHMObj::eBVsec:

			if( true == IsBvSecExist() )
			{
				// Secondary balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpSecBv()->IsCompletelyDefined() )
				{
					// The secondary balancing valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eBVsec ) != eHMObj::eBVsec )
				{
					// The secondary balancing valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		case eHMObj::eBVbyp:

			if( true == IsBvBypExist() )
			{
				// Bypass balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpBypBv()->IsCompletelyDefined() )
				{
					// The bypass secondary balancing valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eBVbyp ) != eHMObj::eBVbyp )
				{
					// The bypass secondary balancing valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		case eHMObj::eCircuitPrimaryPipe:

			if( NULL != GetpCircuitPrimaryPipe() )
			{
				eSomethingIsLocked = eBool3::eb3True;

				if( ( m_usLock & eObj ) != eHMObj::eCircuitPrimaryPipe )
				{
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		case eHMObj::eCircuitBypassPrimaryPipe:

			if( NULL != GetpCircuitBypassPipe() )
			{
				eSomethingIsLocked = eBool3::eb3True;

				if( ( m_usLock & eObj ) != eHMObj::eCircuitBypassPrimaryPipe )
				{
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		case eHMObj::eDistributionSupplyPipe:

			if( NULL != GetpDistrSupplyPipe() )
			{
				eSomethingIsLocked = eBool3::eb3True;

				if( ( m_usLock & eObj ) != eHMObj::eDistributionSupplyPipe )
				{
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		case eHMObj::eDistributionReturnPipe:

			if( NULL != GetpDistrReturnPipe() )
			{
				eSomethingIsLocked = eBool3::eb3True;

				if( ( m_usLock & eObj ) != eHMObj::eDistributionReturnPipe )
				{
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		case eHMObj::eCircuitSecondaryPipe:

			if( NULL != GetpCircuitSecondaryPipe() )
			{
				eSomethingIsLocked = eBool3::eb3True;

				if( ( m_usLock & eObj ) != eHMObj::eCircuitSecondaryPipe )
				{
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		case eHMObj::eCV:

			if( true == IsCvExist() )
			{
				// Control valve exists in the circuit scheme and there is a 'CDS_Hydromod::CCv' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpCV()->IsCompletelyDefined() )
				{
					// The control valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eCV ) != eHMObj::eCV )
				{
					// The control valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		case eHMObj::eDpC:

			if( true == IsDpCExist() )
			{
				// Differential pressure controller valve exists in the circuit scheme and there is a 'CDS_Hydromod::CDpC' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpDpC()->IsCompletelyDefined() )
				{
					// The differential pressure controller valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eDpC ) != eHMObj::eDpC )
				{
					// The differential pressure controller valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		case eHMObj::eShutoffValveSupply:

			if( true == IsShutoffValveExist( eHMObj::eShutoffValveSupply ) )
			{
				// Supply shutoff valve exists in the circuit scheme and there is a 'CDS_Hydromod::CShutoffValve' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpShutoffValve( eHMObj::eShutoffValveSupply )->IsCompletelyDefined() )
				{
					// The supply shutoff valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eShutoffValveSupply ) != eHMObj::eShutoffValveSupply )
				{
					// The supply shutoff valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		case eHMObj::eShutoffValveReturn:

			if( true == IsShutoffValveExist( eHMObj::eShutoffValveReturn ) )
			{
				// Return shutoff valve exists in the circuit scheme and there is a 'CDS_Hydromod::CShutoffValve' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpShutoffValve( eHMObj::eShutoffValveReturn )->IsCompletelyDefined() )
				{
					// The return shutoff valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eShutoffValveReturn ) != eHMObj::eShutoffValveReturn )
				{
					// The return shutoff valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		case eHMObj::eSmartControlValve:

			if( true == IsSmartControlValveExist() )
			{
				// Smart control valve exists in the circuit scheme and there is a 'CDS_Hydromod::CSmartControlValve' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpSmartControlValve()->IsCompletelyDefined() )
				{
					// The smart control valve is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eSmartControlValve ) != eHMObj::eSmartControlValve )
				{
					// The smart control valve is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		case eHMObj::eSmartDpC:

			if( true == IsSmartDpCExist() )
			{
				// Smart differential pressure controller exists in the circuit scheme and there is a 'CDS_Hydromod::CSmartDpC' created.
				eSomethingIsLocked = eBool3::eb3True;

				if( false == GetpSmartDpC()->IsCompletelyDefined() )
				{
					// The smart differential pressure controller is not completely defined, we return 'false'.
					eSomethingIsLocked = eBool3::eb3False;
				}
				else if( ( m_usLock & eHMObj::eSmartDpC ) != eHMObj::eSmartDpC )
				{
					// The smart differential pressure controller is defined and it is not locked.
					eSomethingIsLocked = eBool3::eb3False;
				}
			}

			break;

		default:
			// No need of ASSERT because it's possible to call this method with no object. Thus we need to return 'eb3Undef'.
			break;
	}

	return eSomethingIsLocked;
}

bool CDS_HydroMod::IsUnLocked( eHMObj eObj )
{
	bool bSomethingUnlocked = false;

	switch( eObj )
	{
		case eHMObj::eALL:

			if( true == IsBvExist() )
			{
				// Primary balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				bSomethingUnlocked = true;

				if( NULL == GetpBv()->GetIDPtr().MP )
				{
					// The primary balancing valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eBVprim == ( m_usLock & eHMObj::eBVprim ) )
				{
					// The primary balancing valve is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			if( false == bSomethingUnlocked && true == IsBvSecExist() )
			{
				// Secondary balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				bSomethingUnlocked = true;

				if( NULL == GetpSecBv()->GetIDPtr().MP )
				{
					// The secondary balancing valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eBVsec == ( m_usLock & eHMObj::eBVsec ) )
				{
					// The secondary balancing valve is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			if( false == bSomethingUnlocked && true == IsBvBypExist() )
			{
				// Bypass balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				bSomethingUnlocked = true;

				if( NULL == GetpBypBv()->GetIDPtr().MP )
				{
					// The bypass secondary balancing valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eBVbyp == ( m_usLock & eHMObj::eBVbyp ) )
				{
					// The bypass secondary balancing valve is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			if( false == bSomethingUnlocked && NULL != GetpCircuitPrimaryPipe() )
			{
				bSomethingUnlocked = true;

				if( eHMObj::eCircuitPrimaryPipe == ( m_usLock & eHMObj::eCircuitPrimaryPipe ) )
				{
					bSomethingUnlocked = false;
				}
			}

			if( false == bSomethingUnlocked && NULL != GetpCircuitBypassPipe() )
			{
				bSomethingUnlocked = true;

				if( eHMObj::eCircuitBypassPrimaryPipe == ( m_usLock & eHMObj::eCircuitBypassPrimaryPipe ) )
				{
					bSomethingUnlocked = false;
				}
			}

			if( false == bSomethingUnlocked && NULL != GetpDistrSupplyPipe() )
			{
				bSomethingUnlocked = true;

				if( eHMObj::eDistributionSupplyPipe == ( m_usLock & eHMObj::eDistributionSupplyPipe ) )
				{
					bSomethingUnlocked = false;
				}
			}

			if( false == bSomethingUnlocked && ReturnType::Reverse == m_eReturnType &&  NULL != GetpDistrReturnPipe() )
			{
				bSomethingUnlocked = true;

				if( eHMObj::eDistributionReturnPipe == ( m_usLock & eHMObj::eDistributionReturnPipe ) )
				{
					bSomethingUnlocked = false;
				}
			}

			if( false == bSomethingUnlocked && NULL != GetpCircuitSecondaryPipe() &&  NULL != GetpCircuitSecondaryPipe()->GetLength() )
			{
				bSomethingUnlocked = true;

				if( eHMObj::eCircuitSecondaryPipe == ( m_usLock & eHMObj::eCircuitSecondaryPipe ) )
				{
					bSomethingUnlocked = false;
				}
			}

			if( false == bSomethingUnlocked && true == IsCvExist() )
			{
				// Control valve exists in the circuit scheme and there is a 'CDS_Hydromod::CCv' created.
				bSomethingUnlocked = true;

				if( ( true == GetpCV()->IsTaCV() && NULL == GetpCV()->GetCvIDPtr().MP ) || ( false == GetpCV()->IsTaCV() && GetpCV()->GetKvs() <= 0 ) )
				{
					// The control valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eCV == ( m_usLock & eHMObj::eCV ) )
				{
					// The control valve is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			if( false == bSomethingUnlocked && true == IsDpCExist() )
			{
				// Differential pressure controller valve exists in the circuit scheme and there is a 'CDS_Hydromod::CDpC' created.
				bSomethingUnlocked = true;

				if( NULL == GetpDpC()->GetIDPtr().MP )
				{
					// The differential pressure controller valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eDpC == ( m_usLock & eHMObj::eDpC ) )
				{
					// The differential pressure controller valve is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			if( false == bSomethingUnlocked && true == IsShutoffValveExist( eHMObj::eShutoffValveSupply ) )
			{
				// Supply shutoff valve exists in the circuit scheme and there is a 'CDS_Hydromod::CShutoffValve' created.
				bSomethingUnlocked = true;

				if( NULL == GetpShutoffValve( eHMObj::eShutoffValveSupply )->GetIDPtr().MP )
				{
					// The supply shutoff valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eShutoffValveSupply == ( m_usLock & eHMObj::eShutoffValveSupply ) )
				{
					// The supply shutoff valve is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			if( false == bSomethingUnlocked && true == IsShutoffValveExist( eHMObj::eShutoffValveReturn ) )
			{
				// Return shutoff valve exists in the circuit scheme and there is a 'CDS_Hydromod::CShutoffValve' created.
				bSomethingUnlocked = true;

				if( NULL == GetpShutoffValve( eHMObj::eShutoffValveReturn )->GetIDPtr().MP )
				{
					// The return shutoff valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eShutoffValveReturn == ( m_usLock & eHMObj::eShutoffValveReturn ) )
				{
					// The return shutoff valve is defined and it is not locked.
					bSomethingUnlocked = false;
				}
			}

			if( false == bSomethingUnlocked && true == IsSmartControlValveExist() )
			{
				// Smart control valve exists in the circuit scheme and there is a 'CDS_Hydromod::CSmartControlValve' created.
				bSomethingUnlocked = true;

				if( NULL == GetpSmartControlValve()->GetIDPtr().MP )
				{
					// The smart control valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eSmartControlValve == ( m_usLock & eHMObj::eSmartControlValve ) )
				{
					// The smart control valve is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			if( false == bSomethingUnlocked && true == IsSmartDpCExist() )
			{
				// Smart differential pressure controller exists in the circuit scheme and there is a 'CDS_Hydromod::CSmartDpC' created.
				bSomethingUnlocked = true;

				if( NULL == GetpSmartDpC()->GetIDPtr().MP )
				{
					// The smart differential pressure controller is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eSmartDpC == ( m_usLock & eHMObj::eSmartDpC ) )
				{
					// The smart differential pressure controller is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			break;

		case eHMObj::eBVprim:

			if( true == IsBvExist() )
			{
				// Primary balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				bSomethingUnlocked = true;

				if( NULL == GetpBv()->GetIDPtr().MP )
				{
					// The primary balancing valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eBVprim == ( m_usLock & eHMObj::eBVprim ) )
				{
					// The primary balancing valve is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			break;

		case eHMObj::eBVsec:

			if( true == IsBvSecExist() )
			{
				// Secondary balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				bSomethingUnlocked = true;

				if( NULL == GetpSecBv()->GetIDPtr().MP )
				{
					// The secondary balancing valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eBVsec == ( m_usLock & eHMObj::eBVsec ) )
				{
					// The secondary balancing valve is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			break;

		case eHMObj::eBVbyp:

			if( true == IsBvBypExist() )
			{
				// Bypass balancing valve exists in the circuit scheme and there is a 'CDS_Hydromod::CBV' created.
				bSomethingUnlocked = true;

				if( NULL == GetpBypBv()->GetIDPtr().MP )
				{
					// The bypass balancing valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eBVbyp == ( m_usLock & eHMObj::eBVbyp ) )
				{
					// The bypass secondary balancing valve is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			break;

		case eHMObj::eCircuitPrimaryPipe:

			if( NULL != GetpCircuitPrimaryPipe() )
			{
				bSomethingUnlocked = true;

				if( eHMObj::eCircuitPrimaryPipe == ( m_usLock & eHMObj::eCircuitPrimaryPipe ) )
				{
					bSomethingUnlocked = false;
				}
			}

			break;

		case eHMObj::eCircuitBypassPrimaryPipe:

			if( NULL != GetpCircuitBypassPipe() )
			{
				bSomethingUnlocked = true;

				if( eHMObj::eCircuitBypassPrimaryPipe == ( m_usLock & eHMObj::eCircuitBypassPrimaryPipe ) )
				{
					bSomethingUnlocked = false;
				}
			}

			break;

		case eHMObj::eDistributionSupplyPipe:

			if( NULL != GetpDistrSupplyPipe() )
			{
				bSomethingUnlocked = true;

				if( eHMObj::eDistributionSupplyPipe == ( m_usLock & eHMObj::eDistributionSupplyPipe ) )
				{
					bSomethingUnlocked = false;
				}
			}

			break;

		case eHMObj::eDistributionReturnPipe:

			if( ReturnType::Reverse == m_eReturnType &&  NULL != GetpDistrReturnPipe() )
			{
				bSomethingUnlocked = true;

				if( eHMObj::eDistributionReturnPipe == ( m_usLock & eHMObj::eDistributionReturnPipe ) )
				{
					bSomethingUnlocked = false;
				}
			}

			break;

		case eHMObj::eCircuitSecondaryPipe:

			if( NULL != GetpCircuitSecondaryPipe() &&  NULL != GetpCircuitSecondaryPipe()->GetLength() )
			{
				bSomethingUnlocked = true;

				if( eHMObj::eCircuitSecondaryPipe == ( m_usLock & eHMObj::eCircuitSecondaryPipe ) )
				{
					bSomethingUnlocked = false;
				}
			}

			break;

		case eHMObj::eCV:

			if( true == IsCvExist() )
			{
				// Control valve exists in the circuit scheme and there is a 'CDS_Hydromod::CCv' created.
				bSomethingUnlocked = true;

				if( ( true == GetpCV()->IsTaCV() && NULL == GetpCV()->GetCvIDPtr().MP ) || ( false == GetpCV()->IsTaCV() && GetpCV()->GetKvs() <= 0 ) )
				{
					// The control valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eCV == ( m_usLock & eHMObj::eCV ) )
				{
					// The control valve is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			break;

		case eHMObj::eDpC:

			if( true == IsDpCExist() )
			{
				// Differential pressure controller valve exists in the circuit scheme and there is a 'CDS_Hydromod::CDpC' created.
				bSomethingUnlocked = true;

				if( NULL == GetpDpC()->GetIDPtr().MP )
				{
					// The differential pressure controller valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eDpC == ( m_usLock & eHMObj::eDpC ) )
				{
					// The differential pressure controller valve is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			break;

		case eHMObj::eShutoffValveSupply:

			if( true == IsShutoffValveExist( eHMObj::eShutoffValveSupply ) )
			{
				// Supply shutoff valve exists in the circuit scheme and there is a 'CDS_Hydromod::CShutoffValve' created.
				bSomethingUnlocked = true;

				if( NULL == GetpShutoffValve( eHMObj::eShutoffValveSupply )->GetIDPtr().MP )
				{
					// The supply shutoff valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eShutoffValveSupply == ( m_usLock & eHMObj::eShutoffValveSupply ) )
				{
					// The supply shutoff valve is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			break;

		case eHMObj::eShutoffValveReturn:

			if( true == IsShutoffValveExist( eHMObj::eShutoffValveReturn ) )
			{
				// Return shutoff valve exists in the circuit scheme and there is a 'CDS_Hydromod::CShutoffValve' created.
				bSomethingUnlocked = true;

				if( NULL == GetpShutoffValve( eHMObj::eShutoffValveReturn )->GetIDPtr().MP )
				{
					// The return shutoff valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eShutoffValveReturn == ( m_usLock & eHMObj::eShutoffValveReturn ) )
				{
					// The return shutoff valve is defined and it is not locked.
					bSomethingUnlocked = false;
				}
			}

			break;

		case eHMObj::eSmartControlValve:

			if( true == IsSmartControlValveExist() )
			{
				// Smart control valve exists in the circuit scheme and there is a 'CDS_Hydromod::CSmartControlValve' created.
				bSomethingUnlocked = true;

				if( NULL == GetpSmartControlValve()->GetIDPtr().MP )
				{
					// The smart control valve is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eSmartControlValve == ( m_usLock & eHMObj::eSmartControlValve ) )
				{
					// The smart control valve is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			break;

		case eHMObj::eSmartDpC:

			if( true == IsSmartDpCExist() )
			{
				// Smart differential pressure controller exists in the circuit scheme and there is a 'CDS_Hydromod::CSmartDpC' created.
				bSomethingUnlocked = true;

				if( NULL == GetpSmartDpC()->GetIDPtr().MP )
				{
					// The smart differential pressure controller is not completely defined, we return 'false'.
					bSomethingUnlocked = false;
				}
				else if( eHMObj::eSmartDpC == ( m_usLock & eHMObj::eSmartDpC ) )
				{
					// The smart differential pressure controller is defined and it is locked.
					bSomethingUnlocked = false;
				}
			}

			break;

		default:
			ASSERT( 0 );
			break;
	}

	return bSomethingUnlocked;
}

void CDS_HydroMod::SetLock( eHMObj eObj, bool bFlag, bool bResizeNow, bool bDontCareFreezed )
{
	if( false == bDontCareFreezed && false == IsForHub() && true == GetpPrjParam()->IsFreezed() )
	{
		return;
	}

	// Particular case. Current CDS_HydroMod is a module with at least one child and we want to lock/unlock circuit (or secondary)
	// pipe of the current module, we don't allow.
	if( true == IsaModule() && NULL != GetpSchcat() )
	{
		if( eHMObj::eALL == eObj ||
				( false == GetpSchcat()->IsSecondarySideExist() && ( eHMObj::eCircuitPrimaryPipe == ( eObj & eHMObj::eCircuitPrimaryPipe ) ) ) ||
				( true == GetpSchcat()->IsSecondarySideExist() && ( eHMObj::eCircuitSecondaryPipe == ( eObj & eHMObj::eCircuitSecondaryPipe ) ) ) )
		{
			CDS_HydroMod *pclFirstChildHM = GetFirstHMChild();

			if( NULL != pclFirstChildHM && NULL != pclFirstChildHM->GetpDistrSupplyPipe() )
			{
				int iObj = ( int )eObj;

				if( false == GetpSchcat()->IsSecondarySideExist() )
				{
					iObj &= ~eHMObj::eCircuitPrimaryPipe;

					if( NULL != GetpCircuitBypassPipe() && CAnchorPt::CircuitSide::CircuitSide_Primary == GetBypassPipeSide() )
					{
						iObj &= ~eHMObj::eCircuitBypassPrimaryPipe;
					}
				}
				else
				{
					iObj &= ~eHMObj::eCircuitSecondaryPipe;

					if( NULL != GetpCircuitBypassPipe() && CAnchorPt::CircuitSide::CircuitSide_Secondary == GetBypassPipeSide() )
					{
						iObj &= ~eHMObj::eCircuitBypassPrimaryPipe;
					}
				}

				eObj = ( eHMObj )iObj;

				if( eHMObj::eNone == eObj )
				{
					return;
				}
			}
		}
	}

	if( true == bFlag )
	{
		m_usLock |= eObj;
	}
	else
	{
		m_usLock &= ~eObj;
	}

	// Particular case. If current CDS_HydroMod is the first child of a module and we lock its distribution pipe,
	// the circuit pipe (or secondary) of the module must also be locked.
	if( eHMObj::eALL == eObj || ( eHMObj::eDistributionSupplyPipe == ( eObj & eHMObj::eDistributionSupplyPipe ) ) )
	{
		CDS_HydroMod *pclParentHM = GetParent();

		if( NULL != pclParentHM && this == pclParentHM->GetFirstHMChild() )
		{
			if( NULL != pclParentHM->GetpCircuitSecondaryPipe() )
			{
				if( pclParentHM->GetpCircuitSecondaryPipe()->IsLocked() != bFlag )
				{
					pclParentHM->GetpCircuitSecondaryPipe()->SetLock( bFlag );
				}
			}
			else if( NULL != pclParentHM->GetpCircuitPrimaryPipe() )
			{
				if( pclParentHM->GetpCircuitPrimaryPipe()->IsLocked() != bFlag )
				{
					pclParentHM->GetpCircuitPrimaryPipe()->SetLock( bFlag );
				}
			}
		}
	}

	// Resize only if unlock and bool bResizeNow set to true.
	if( false == bFlag && true == bResizeNow )
	{
		ComputeHM( eceResize );
	}
}

void CDS_HydroMod::SetRecursiveLock( eHMObj eObj, bool bFlag, bool bResizeNow, bool bDontCareFreezed )
{
	SetLock( eObj, bFlag, bResizeNow, bDontCareFreezed );

	for( IDPTR IDPtr = GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = GetNext( IDPtr.MP ) )
	{
		CDS_HydroMod *pHMChild = dynamic_cast<CDS_HydroMod *>( IDPtr.MP );
		ASSERT( NULL != pHMChild );

		if( NULL != pHMChild )
		{
			pHMChild->SetRecursiveLock( eObj, bFlag, bResizeNow, bDontCareFreezed );
		}
	}
}

void CDS_HydroMod::SetCBIValveID( LPCTSTR tcsCBIValveID )
{
	if( _T('\0') == *tcsCBIValveID )
	{
		m_CBIValveIDPtr = _NULL_IDPTR;
		Modified();
	}
	else if( 0 != _tcscmp( tcsCBIValveID, m_CBIValveIDPtr.ID ) )
	{
		m_CBIValveIDPtr = TASApp.GetpTADB()->Get( tcsCBIValveID );
		ASSERT( _T('\0') != *m_CBIValveIDPtr.ID );
		
		Modified();
	}
}

IDPTR CDS_HydroMod::GetCBIValveIDPtr()
{
	if( _T('\0') != *m_CBIValveIDPtr.ID && m_CBIValveIDPtr.DB )
	{
		Extend( &m_CBIValveIDPtr );
		return m_CBIValveIDPtr;
	}

	return _NULL_IDPTR;
}

CDB_Pipe *CDS_HydroMod::GetPipeSizeShift( int &iHighestSize, int &iLowestSize, CAnchorPt::CircuitSide eCircuitSide, CAnchorPt::PipeLocation ePipeLocation )
{
	// Verify Pipe Size shift.
	// Setup variables to possibly enforce combined SizeShifts.
	// In case of Hub Selection don't use SizeShift from TechParam.
	int iSizeShiftUp;
	int iSizeShiftDown;
	int iSizeShiftAbove = GetpTechParam()->GetSizeShiftAbove();
	int iSizeShiftBelow = GetpTechParam()->GetSizeShiftBelow();

	if( true == IsForHub() || 99 == iSizeShiftAbove )  // Combined SizeShift +3/-3 enforced
	{
		iSizeShiftUp = 3;
		iSizeShiftDown = -3;
	}
	else
	{
		iSizeShiftUp = iSizeShiftAbove;
		iSizeShiftDown = iSizeShiftBelow;
	}

	CDB_Pipe *pPipe = NULL;

	if( CAnchorPt::CircuitSide_Primary == eCircuitSide )
	{
		if( CAnchorPt::PipeLocation::PipeLocation_Undefined == ePipeLocation || CAnchorPt::PipeLocation::PipeLocation_Supply == ePipeLocation 
				|| CAnchorPt::PipeLocation::PipeLocation_Return == ePipeLocation )
		{
			if( NULL != GetpCircuitPrimaryPipe() )
			{
				pPipe = dynamic_cast<CDB_Pipe *>( GetpCircuitPrimaryPipe()->GetIDPtr().MP );
			}
			else
			{
				ASSERTA_RETURN( NULL );
			}
		}
		else if( CAnchorPt::PipeLocation::PipeLocation_ByPass == ePipeLocation )
		{
			if( NULL != GetpCircuitBypassPipe() )
			{
				pPipe = dynamic_cast<CDB_Pipe *>( GetpCircuitBypassPipe()->GetIDPtr().MP );
			}
			else
			{
				ASSERTA_RETURN( NULL );
			}
		}
	}
	else if( CAnchorPt::CircuitSide_Secondary == eCircuitSide )
	{
		if( CAnchorPt::PipeLocation::PipeLocation_Undefined == ePipeLocation || CAnchorPt::PipeLocation::PipeLocation_Supply == ePipeLocation 
				|| CAnchorPt::PipeLocation::PipeLocation_Return == ePipeLocation )
		{
			if( NULL != GetpCircuitSecondaryPipe() )
			{
				pPipe = dynamic_cast<CDB_Pipe *>( GetpCircuitSecondaryPipe()->GetIDPtr().MP );
			}
			else
			{
				ASSERTA_RETURN( NULL );
			}
		}
		else if( CAnchorPt::PipeLocation::PipeLocation_ByPass == ePipeLocation )
		{
			if( NULL != GetpCircuitBypassPipe() )
			{
				pPipe = dynamic_cast<CDB_Pipe *>( GetpCircuitBypassPipe()->GetIDPtr().MP );
			}
			else
			{
				ASSERTA_RETURN( NULL );
			}
		}
	}
	else
	{
		ASSERTA_RETURN( NULL );
	}

	if( NULL == pPipe )
	{
		return false;
	}

	int iPipeDnKey = pPipe->GetSizeKey( TASApp.GetpTADB() );
	int iDN = m_pclTableDN->GetBiggestDN();
	iHighestSize = min( iPipeDnKey + iSizeShiftUp, iDN );

	// In case of pipe with a size below DN15 force a selection up to DN15.
	if( iHighestSize < m_iDN15 )
	{
		iHighestSize = m_iDN15;
	}

	iLowestSize = max( 1, iPipeDnKey + iSizeShiftDown );

	return pPipe;
}

void CDS_HydroMod::SetRemarkIndex( int iRemarkIndex )
{
	if( m_iRemarkIndex != iRemarkIndex )
	{
		m_iRemarkIndex = iRemarkIndex;
		Modified();
	}
}

void CDS_HydroMod::SetSchemeID( CString strSchemID )
{
	if( _T('\0') == *strSchemID )
	{
		return;
	}

	if( _tcscmp( strSchemID, m_SchemeIDPtr.ID ) )
	{
		IDPTR IDPtr = TASApp.GetpTADB()->Get( strSchemID );
		ASSERT( _T('\0') != *IDPtr.ID );

		if( _T('\0') != *IDPtr.ID )
		{
			SetSchemeIDPtr( IDPtr );
		}
		else
		{
			ASSERT( 0 );
		}
	}
}

void CDS_HydroMod::SetSchemeIDPtr( IDPTR SchemeIDPtr )
{
	m_SchemeIDPtr = SchemeIDPtr;
}

// Find correct SchemeIDPtr in function of selected device
void CDS_HydroMod::ResetSchemeIDPtr()
{
	CTable *pTab = static_cast<CTable *>( TASApp.GetpTADB()->Get( _T("CIRCSCHEME_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Current scheme category.
	CDB_CircSchemeCateg *pSchCateg = GetpSchcat();

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_CircuitScheme *pSch = static_cast<CDB_CircuitScheme *>( IDPtr.MP );

		if( NULL == pSch )
		{
			continue;
		}

		if( NULL != pSchCateg && pSchCateg != static_cast<CDB_CircSchemeCateg *>( pSch->GetSchemeCategIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSchCateg && ( (CDB_CircSchemeCateg *)pSch->GetSchemeCategIDPtr().MP )->Get2W3W() != pSchCateg->Get2W3W() )
		{
			continue;
		}

		if( NULL != GetpSch() && pSch->GetBalType() != GetpSch()->GetBalType() )
		{
			continue;
		}

		if( NULL != GetpSch() && pSch->GetTermUnit() != GetpSch()->GetTermUnit() )
		{
			continue;
		}

		//AB+/A/B both should be true or false
		// continue if ->	Dpc exist and anchoring point doesn't exist
		//				OR	DpC doesn't exist and anchoring point exist
		if( ( NULL != GetpDpC() ) != pSch->IsAnchorPtExist( CAnchorPt::DpC ) )
		{
			continue;
		}

		if( ( NULL == GetpBv() && true == pSch->IsAnchorPtExist( CAnchorPt::BV_P ) && false == pSch->IsAnchorPtOptional( CAnchorPt::BV_P ) )
				|| ( NULL != GetpBv() && false == pSch->IsAnchorPtExist( CAnchorPt::BV_P ) ) )
		{
			continue;
		}

		if( ( NULL == GetpBypBv() && true == pSch->IsAnchorPtExist( CAnchorPt::BV_Byp ) && false == pSch->IsAnchorPtOptional( CAnchorPt::BV_Byp ) )
				|| ( NULL != GetpBypBv() && false == pSch->IsAnchorPtExist( CAnchorPt::BV_Byp ) ) )
		{
			continue;
		}

		if( ( NULL == GetpSecBv() && true == pSch->IsAnchorPtExist( CAnchorPt::BV_S ) && false == pSch->IsAnchorPtOptional( CAnchorPt::BV_S ) )
				|| ( NULL != GetpSecBv() && false == pSch->IsAnchorPtExist( CAnchorPt::BV_S ) ) )
		{
			continue;
		}

		if( NULL != GetpCV() && eb3True == GetpCV()->IsPICV() && false == pSch->IsAnchorPtExist( CAnchorPt::PICV ) )
		{
			// If we have a PICV in the hydraulic network but there is no PICV in the schematic, it's not the good one.
			continue;
		}

		if( NULL != GetpCV() && eb3True == GetpCV()->IsDpCBCV() && false == pSch->IsAnchorPtExist( CAnchorPt::DPCBCV ) )
		{
			// If we have a DpCBCV in the hydraulic network but there is no DpCBCV in the schematic, it's not the good one.
			continue;
		}

		if( NULL != GetpCV() && eb3False == GetpCV()->IsPICV() && eb3False == GetpCV()->IsDpCBCV() && false == pSch->IsAnchorPtExist( CAnchorPt::ControlValve ) )
		{
			// If we have a normal control valve (not a PICV and not a DpCBCV) in the hydraulic network but there is no control valve in the schematic,
			// it's not the good one.
			continue;
		}

		if( NULL == GetpCV() && ( true == pSch->IsAnchorPtExist( CAnchorPt::ControlValve ) || true == pSch->IsAnchorPtExist( CAnchorPt::PICV )
				|| true == pSch->IsAnchorPtExist( CAnchorPt::DPCBCV ) ) )
		{
			continue;
		}

		if( NULL != GetpSch() && pSch->GetCvFunc() != GetpSch()->GetCvFunc() )
		{
			continue;
		}

		if( true == IsDpCExist() )
		{
			if( pSch->GetMvLoc() != GetpDpC()->GetMvLoc() )
			{
				continue;
			}
		}

		if( NULL != GetpSch() && pSch->GetDpStab() != GetpSch()->GetDpStab() )
		{
			continue;
		}

		if( true == IsCvExist() && NULL != GetpSch() && pSch->GetCvLoc() != GetpSch()->GetCvLoc() )
		{
			continue;
		}

		if( NULL != GetpShutoffValve( eShutoffValveSupply ) && ( false == pSch->IsAnchorPtExist( CAnchorPt::ShutoffValve ) || ( NULL != GetpSch() && ShutoffValveLocSupply != GetpSch()->GetShutoffValveLoc() ) ) )
		{
			continue;
		}

		if( NULL == GetpShutoffValve( eShutoffValveSupply ) && true == pSch->IsAnchorPtExist( CAnchorPt::ShutoffValve ) && NULL != GetpSch() && ShutoffValveLocSupply == GetpSch()->GetShutoffValveLoc() )
		{
			continue;
		}

		if( NULL != GetpShutoffValve( eShutoffValveReturn ) && ( false == pSch->IsAnchorPtExist( CAnchorPt::ShutoffValve ) || ( NULL != GetpSch() && ShutoffValveLocReturn != GetpSch()->GetShutoffValveLoc() ) ) )
		{
			continue;
		}

		if( NULL == GetpShutoffValve( eShutoffValveReturn ) && true == pSch->IsAnchorPtExist( CAnchorPt::ShutoffValve ) && NULL != GetpSch() && ShutoffValveLocReturn == GetpSch()->GetShutoffValveLoc() )
		{
			continue;
		}

		if( NULL != GetpSmartControlValve() && ( false == pSch->IsAnchorPtExist( CAnchorPt::SmartControlValve ) || GetpSmartControlValve()->GetLocalization() != pSch->GetSmartControlValveLoc() ) )
		{
			continue;
		}

		if( NULL == GetpSmartControlValve() && true == pSch->IsAnchorPtExist( CAnchorPt::SmartControlValve ) )
		{
			continue;
		}

		if( NULL != GetpSmartDpC() && ( false == pSch->IsAnchorPtExist( CAnchorPt::SmartDpC ) || GetpSmartDpC()->GetLocalization() != pSch->GetSmartDpCLoc() ) )
		{
			continue;
		}

		if( NULL == GetpSmartDpC() && true == pSch->IsAnchorPtExist( CAnchorPt::SmartDpC ) )
		{
			continue;
		}

		SetSchemeID( IDPtr.ID );
		break;
	}
}

void CDS_HydroMod::SetDescription( CString strDescription )
{
	if( strDescription != m_strDescription )
	{
		m_strDescription = strDescription;
		Modified();
	}
}

CDB_CircSchemeCateg *CDS_HydroMod::GetpSchcat()
{
	CDB_CircSchemeCateg *pReturn = ( GetpSch() != NULL ) ? (CDB_CircSchemeCateg *)GetpSch()->GetSchemeCategIDPtr().MP : NULL;
	return pReturn;
}

// compute flow for the current distribution pipe,
// take in count flow for terminal unit and flow for sibling items with a greater index
double CDS_HydroMod::GetDistrTotQ()
{
	double Q = GetQ();
	CTable *pTab = static_cast<CTable *>( GetIDPtr().PP );

	// Test if we reached Hydronic table
	if( true == pTab->IsClass( CLASS( CTableHM ) ) )
	{
		return Q;
	}

	else
	{
		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDS_HydroMod *pHM = static_cast< CDS_HydroMod * >( IDPtr.MP );

			if( pHM->GetPos() <= GetPos() )
			{
				continue;
			}

			Q += pHM->GetQ();
		}
	}

	return Q;
}

// HYS-709: Return corrected HMin, CV min Dp is replaced by real CV Dp
double CDS_HydroMod::GetHminForUserDisplay()
{
	double dHmin = 0.0;

	if( GetpSchcat() != NULL && true == GetpSchcat()->IsPump() )
	{
		dHmin = GetpPump()->GetHmin();
	}
	else
	{
		// HYS-1704: As the comment said above, we have to use the real CV Dp. I had a look for HYS-709 
		// and I think it was an oversight.
		dHmin = GetDp( true );
		
		// Why second condition? In the case of the DpC stabilizes the control valve, the pressure drop on the control valve 
		// is already computed in the stabilized pressure (DpL) of the DpC.
		if( true == IsCvExist( true ) && false == IsDpCExist() )
		{
			// HYS-1757: We must also exclude the 3-way mixing with decoupling bypass. Because when calling 'GetDp' with 'true' to have the minimum,
			// for this circuit we never return the minimum pressure drop accross the control valve but well the real pressure drop that is near of
			// 3 kPa in this case (Or equal to 3 kPa if we are working in Kvs without Reynard series).
			if( eb3True == GetpCV()->IsCVLocInPrimary() && NULL != GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixing != GetpSchcat()->Get3WType() 
					&& CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp != GetpSchcat()->Get3WType() )
			{
				if( false == GetpCV()->IsTaCV() || false == GetpCV()->IsPresettable() )
				{
					// We remove Dpmin of the control valve added when calling the 'GetDp( true )' method above.
					dHmin -= GetpCV()->GetDpMin();

					// And we add now the pressure drop on the control valve as it is computed.
					dHmin += GetpCV()->GetDp();
				}
			}
		}

		if( dHmin <= 0.0 )
		{
			return 0.0;
		}
	}

	return dHmin;
}

double CDS_HydroMod::GetDp( bool bDpMin )
{
	if( NULL == GetpTermUnit() || NULL == GetpCircuitPrimaryPipe() )
	{
		return 0;
	}

	double dTUDp = GetDpOnOutOfPrimSide(); //Set to 0 when Injection circuit
	double dCircuitPrimaryPipeDp = 0.0;
	double dCircuitPrimaryPipeSingularityDp = 0.0;

	// TODO PENDING : test "false == IsPending()" should be reviewed when we deliver a circuit with a Pending flag see IsPending function
	if( ( false == IsaModule() || ( NULL != GetpSchcat() && true == GetpSchcat()->IsSecondarySideExist() ) ) && false == IsPending() )
	{
		dCircuitPrimaryPipeDp = GetpCircuitPrimaryPipe()->GetPipeDp();
		dCircuitPrimaryPipeSingularityDp = GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true );
	}

	double dCircuitBypassPipeDp = 0.0;
	double dCircuitBypassPipeSingularityDp = 0.0;

	/* HYS-1930 Unfortunately, we don't show at that moment the bypass pipe box in the SSheetPanelCirc2.
	   If we take the decision to show it, we have to do for all the circtuits.
	   And because we show any information about pressure loss in the bypass pipe, we don't take into account this value.
	   Otherwise user can't manually verify Hmin value for example.
	if( NULL != GetpSchcat() && eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() && NULL != GetpCircuitBypassPipe() )
	{
		// Auto-adapting variable flow decoupling circuit.
		dCircuitBypassPipeDp = GetpCircuitBypassPipe()->GetPipeDp();
		dCircuitBypassPipeSingularityDp = GetpCircuitBypassPipe()->GetSingularityTotalDp( true );
	}
	*/

	double dBVDp = 0.0;
	double dDpCDp = 0.0;
	double dDpCDpl = 0.0;
	double dDpCv = 0.0;
	double dSVSupplyDp = 0.0;
	double dSVReturnDp = 0.0;
	double dSmartControlValveDp = 0.0;
	double dSmartDpCDp = 0.0;

	if( true == IsShutoffValveExist( eHMObj::eShutoffValveSupply, true ) )
	{
		dSVSupplyDp = GetpShutoffValve( eHMObj::eShutoffValveSupply )->GetDp();
	}

	if( true == IsShutoffValveExist( eHMObj::eShutoffValveReturn, true ) )
	{
		dSVReturnDp = GetpShutoffValve( eHMObj::eShutoffValveReturn )->GetDp();
	}

	if( true == IsBvExist( true ) )
	{
		if( true == bDpMin /*&& CDB_CircSchemeCateg::e3wTypeMixing != GetpSchcat()->Get3WType()*/ ) //< in case of 3WMixing Dp taken by BV is a mirror of CV Dp
		{
			dBVDp = GetpBv()->GetDpMin();
		}
		else
		{
			dBVDp = GetpBv()->GetDp();
		}
	}

	if( true == IsDpCExist( true ) )
	{
		dDpCDp = ( true == bDpMin ) ? GetpDpC()->GetDpmin() : GetpDpC()->GetDp();
		dDpCDpl = max( GetpDpC()->GetDpToStab( eMvLoc::MvLocLast, bDpMin ), GetpDpC()->GetDplmin() );

		// If measuring valve is located on secondary Dp of this measuring valve is already in dDpCDp.
		if( eMvLoc::MvLocSecondary == GetpDpC()->GetMvLoc() )
		{
			dBVDp = 0.0;
		}

		// In case of branch stabilization TuDp is already in dDpCDp.
		if( NULL != GetpSch() && eDpStab::DpStabOnBranch == GetpSch()->GetDpStab() )
		{
			dTUDp = 0.0;
		}
	}

	// If a Dp controller stabilizes the pressure drop accross the control valve, we don't need to add the pressure drop of the control valve.
	if( true == IsCvExist( true ) && false == IsDpCExist() )
	{
		// We exclude 3-way mixing circuits for which the control valve is at the primary side but we do not want to manage here.
		if( eb3True == GetpCV()->IsCVLocInPrimary() && NULL != GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixing != GetpSchcat()->Get3WType() )
		{
			// If we are with a 3-way mixing circuit with decoupling bypass and a control valve at the primary side, the control valve has an 
			// authority close to 1. So, when this circuit is computed, we already have the good control valve with its pressure drop.
			// When calling the "_AdjustPumpHminForModulatingCV" method, this circuit will not be added to the vector of circuit to verify.
			// In this case, we need the true pressure drop accross the control valve in any case. Otherwise, the H available at the entry of the
			// circuit will never be updated.
			if( CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == GetpSchcat()->Get3WType() )
			{
				dDpCv = GetpCV()->GetDp();
			}
			else
			{
				if( true == bDpMin )
				{
					dDpCv = GetpCV()->GetDpMin();
				}
				else
				{
					dDpCv = GetpCV()->GetDp();
				}
			}
		}
	}

	if( true == IsSmartControlValveExist( true ) )
	{
		if( true == bDpMin )
		{
			dSmartControlValveDp = GetpSmartControlValve()->GetDpMin();
		}
		else
		{
			dSmartControlValveDp = GetpSmartControlValve()->GetDp();
		}
	}

	if( true == IsSmartDpCExist( true ) )
	{
		if( true == bDpMin )
		{
			dSmartDpCDp = GetpSmartDpC()->GetDpMin();
		}
		else
		{
			dSmartDpCDp = GetpSmartDpC()->GetDp();
		}
	}

	double dDpTot = dSVReturnDp + dSVSupplyDp + dTUDp + dCircuitPrimaryPipeDp + dCircuitPrimaryPipeSingularityDp + dCircuitBypassPipeDp 
			+ dCircuitBypassPipeSingularityDp + dBVDp + dDpCDp + dDpCDpl + dDpCv + dSmartControlValveDp + dSmartDpCDp;

	if( true == IsPending() )
	{
		dDpTot = m_dHAvail;
	}

	return dDpTot;
}

void CDS_HydroMod::ResizeBalDevForH( double dHAvail, bool bForPending, CAnchorPt::CircuitSide eCircuitSide, bool *pbHasHAvailSecChanged )
{
	if( eCircuitSide != CAnchorPt::CircuitSide::CircuitSide_Primary && eCircuitSide != CAnchorPt::CircuitSide::CircuitSide_Secondary )
	{
		ASSERT_RETURN;
	}

	if( NULL != pbHasHAvailSecChanged )
	{
		*pbHasHAvailSecChanged = false;
	}

	unsigned int uiLockStatus = m_usLock;

	if( true == bForPending )
	{
		SetLock( eALL );

		if( true == IsaModule() && GetQ() != GetpTermUnit()->GetPartialPendingQ() )
		{
			GetpTermUnit()->SetQ( GetpTermUnit()->GetPartialPendingQ(), false );
		}
	}

	if( dHAvail <= 0.0 )
	{
		if( NULL != GetpPump() )
		{
			// We reset pump only if we are sure that pump is located on primary side.
			if( CAnchorPt::CircuitSide::CircuitSide_Primary == eCircuitSide && NULL != GetpSchcat() && false == GetpSchcat()->IsSecondarySideExist() )
			{
				GetpPump()->SetHmin( 0 );
			}
		}
	}

	// Differential pressure available changes at the entry of the circuit. We must adjust the balancing device of this circuit.
	// Pay attention: a "Balancing" device is not necessarily a balancing valve. It's the device (Product, valve) that 
	// can be adjusted to manage the difference of pressure which arrives on the circuit.
	// By priority:
	//   1) Dp controller: This valve is either protecting what it is behind it from any variation of differential pressure at 
	//      the entry of the circuit or is protecting the control valve just behind it. Thus, if a variation occurs, this valve
	//      can be adapted to manage it.
	//   2) Balancing valve: Without a Dp controller, if a balancing valve exists, it is this one that will manage this change.
	// 	    But if a control valve is also there, we will try to size and adjust this one by letting the balancing valve to its minimum.
	//   3) Control valve: without DpC and BV, we can close or open the control valve to take the difference.
	//      Pay attention that we are talking about a differential pressure variation at the entry of the circuit, so at the
	//      primary side. We can adjust or resize the control valve ONLY if this one is located at the primary side !
	//   4) Smart control valve and smart differential pressure controller: This valve is working alone without any other Dp controller, 
	//		balancing or control valve. Thus, it is this valve that will manage the change of differential pressure.

	// If we have a shutoff valve we must deduce the differential pressure drop on it from the H available at the entry
	// of the circuit.
	if( dHAvail > 0.0 )
	{
		if( true == IsShutoffValveExist( CDS_HydroMod::eHMObj::eShutoffValveReturn, true ) )
		{
			dHAvail -= GetpShutoffValve( CDS_HydroMod::eHMObj::eShutoffValveReturn )->GetDp();
		}

		if( true == IsShutoffValveExist( CDS_HydroMod::eHMObj::eShutoffValveSupply, true ) )
		{
			dHAvail -= GetpShutoffValve( CDS_HydroMod::eHMObj::eShutoffValveSupply )->GetDp();
		}
	}

	if( true == IsDpCExist() && CAnchorPt::CircuitSide::CircuitSide_Primary == eCircuitSide )
	{
		// Case 1 (See above).

		if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() )
		{
			// HYS-1930: auto-adapting variable flow decoupling circuit.

			ClearErrorMessage( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_SecondaryPump );

			// If H available is different from 0, we adapt primary and secondary side for several scenarii.
			if( dHAvail > 0.0 )
			{
				// For this case we have to consider the H available in the bypass that is in this case the H available at the entry
				// of the hydraulic circuit minus the pressure drop accross the Dp controller (Hbypass) -> It's the stabilized differential pressure !!

				// The exceed of pressure at the entry of the hydraulic circuit can be taken either by closing the Dp controller or by increasing
				// the stabilized differential pressure.
				// We take the second option. Because if we increase the Dpl, it's a Dp that can help to decrease the pump head at the secondary side.
				// (That is not the case if we take the exceed in the Dp controller itself).

				// When increasing the stabilized differential pressure, we can have 2 cases:
				//  1) The dpl range of the current Dp controller can manage.
				//  2) The dplmax is not enough -> We need to check one other Dp controller with a bigger Dpl range.
				//     In this case, we have two cases yet:
				//		a) Dp controller found, change it -> it's ok.
				//      b) Dp controller not found -> Set to Dplmax and preset the Dp controller to take the difference (Havail - Dplmax).

				// We first initialize these variables needed if the Dp controller is changed.
				double dBvsecDpMin = 0.0;

				if( NULL != GetpSecBv() )
				{
					GetpSecBv()->ResizeBv( 0.0 );
				}

				if( NULL != GetpSecBv() )
				{
					// Will return either 3 kPa or the minimal pressure drop at full opening.
					dBvsecDpMin = GetpSecBv()->GetDpMin();
				}

				double dTUDp = GetpTermUnit()->GetDp();
				double dSecPipesDp = 0.0;
					
				if( NULL != GetpCircuitSecondaryPipe() )
				{
					dSecPipesDp += GetpCircuitSecondaryPipe()->GetPipeDp();
					dSecPipesDp += GetpCircuitSecondaryPipe()->GetSingularityTotalDp( true );
				}

				// Now we continue with the minimal pressure drop that the Dp controller needs to work.
				double dDpCDpmin = GetpDpC()->GetDpmin();
				double dDpCDplmin = GetpDpC()->GetDplmin();
				double dDpCDplmax = GetpDpC()->GetDplmax();
				double dHbypass = fabs( dHAvail - dDpCDpmin );

				if( NULL != GetpCircuitPrimaryPipe() )
				{
					dHbypass -= GetpCircuitPrimaryPipe()->GetPipeDp();
					dHbypass -= GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true );
				}

				double dHAvailSec = GetpPump()->GetHAvail() + dHbypass;

				/* HYS-1930 Unfortunately, we don't show at that moment the bypass pipe box in the SSheetPanelCirc2.
					If we take the decision to show it, we have to do for all the circtuits.
					And because we don't show information about pressure loss in the bypass pipe, we don't take into account this value.
					Otherwise, user can't manually verify Hmin value for example.
				if( NULL != GetpCircuitBypassPipe() )
				{
					dHbypass -= GetpCircuitBypassPipe()->GetPipeDp();
					dHbypass -= GetpCircuitBypassPipe()->GetSingularityTotalDp( true );
				}
				*/

				// Set the Bv bypass to the desired stabilized differential pressure.
				GetpBypBv()->ResizeBv( dHbypass );

				// 2) Check if the Dpl range of the current Dp controller is enough.
				if( dHbypass > dDpCDplmax )
				{
					// Try to find a good range OR to keep Dpl max with the exceed of pressure taken by
					// the Dp controller itself.
					// GetpDpC()->SelectBestDpC( dHbypass, false, true );
					GetpDpC()->SelectBestDpC( dHAvail, false, true );

					// By changing the Dp controller, we can have a different minimum pressure drop required for it to work.
					double dNewDpCDpmin = GetpDpC()->GetDpmin();

					if( dNewDpCDpmin != dDpCDpmin )
					{
						dHbypass = fabs( dHAvail - dNewDpCDpmin );

						if( NULL != GetpCircuitPrimaryPipe() )
						{
							dHbypass -= GetpCircuitPrimaryPipe()->GetPipeDp();
							dHbypass -= GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true );
						}
					}
							
					if( dHbypass > GetpDpC()->GetDplmax() )
					{
						// We don't find a Dp controller with a good Dpl max, we limit the Hbypass to the Dpl max.
						dHbypass = GetpDpC()->GetDplmax();
					}

					// Set the Bv bypass to the desired stabilized differential pressure.
					GetpBypBv()->ResizeBv( dHbypass );

					// Check if the minmum differential pressure drop to stabilize possible by the new Dp controller is
					// different. In this case, we need to change Hmin.
					double dNewDpCDplmin = GetpDpC()->GetDplmin();

					if( dNewDpCDplmin != dDpCDplmin )
					{
						GetpPump()->SetHmin( dTUDp + dBvsecDpMin - dNewDpCDplmin );
					}
				}

				// Now that the Dp controller is fixed and we know the H available in the bypass, we can decide
				// how to help the pump at the secondary side.
				// We have 3 cases:
				// Remark: HT is the H required by children (Tertiary part). We include the BV at the secondary part.
				//	1) HT > Hbypass: we can have two cases:
				//		a) HT - Hbypass < Pump Hmin min (Tech param = 25 kPa) -> Pump Hmin = 25 kPa; BVsec += 25 - (HT - Hbypass)
				//		b) HT - Hbypass >= Pump Hmin min (25 kPa)             -> Pump Hmin = HT - Hbypass
				//  2) HT <= Hbypass: We can delete the pump at the secondary side -> BVsec += (Hbypass - HT)

				// -------+----*----P---*-------*
				//        |    ↑        ↑       ↑
				//   +---Bvb   | Hb     | HT    | Havailable
				//   |    |    ↓        ↓       ↓
				// --DpC--+----*--------*--Bvs--*

				double dPumpHmin = GetpPump()->GetHmin();
				bool bPumpHUserDefined = ( GetpPump()->GetHpump() > 0.0 ) ? true : false;
				double dPumpHUser = GetpPump()->GetHpump();
				double dPumpHAvail = GetpPump()->GetHAvail();

				double dHTrequired = dTUDp + dSecPipesDp + dBvsecDpMin;

				// Check the cases.
				double dDefaultSecondaryPumpHMin = m_pTechParam->GetDefaultSecondaryPumpHMin();
				double dExceedOfPressure = 0.0;
				double dNewPumpHmin = 0.0;

				if( dHTrequired > dHbypass )
				{
					// Case 1.
					if( dHTrequired - dHbypass < dDefaultSecondaryPumpHMin )
					{
						// Case 1.a.
						dNewPumpHmin = dDefaultSecondaryPumpHMin;
						dExceedOfPressure = dDefaultSecondaryPumpHMin - ( dHTrequired - dHbypass );

						CString str;
						CString strHavailable = WriteCUDouble( _U_DIFFPRESS, dHTrequired, true );
						CString strHbypass = WriteCUDouble( _U_DIFFPRESS, dHbypass, true );
						CString strDefaultSecondaryPumpHMin = WriteCUDouble( _U_DIFFPRESS, dDefaultSecondaryPumpHMin, true );

						// The computed H required by children (%1) minus the H in the bypass (%2) is lower than\r\n
						// the default sec. pump Hmin set in the technical parameters (%3). This is why the pump Hmin\r\n
						// is set to %4 and the balancing valve at the secondary side takes the difference.
						FormatString( str, IDS_HYDROMOD_SECPUMPHMINBLOCKEDTODEFAULTTECHPARAMS, strHavailable, strHbypass, strDefaultSecondaryPumpHMin, strDefaultSecondaryPumpHMin );
						m_mapErrorMessages.insert( std::pair<CDS_HydroMod::eValidityFlags, CString>( CDS_HydroMod::eValidityFlags::evfSecPumpHMinTechParam, str ) );
					}
					else
					{
						// Case 1.b.
						dNewPumpHmin = ( dHTrequired - dHbypass );
					}
				}
				else if( dHTrequired <= dHbypass )
				{
					// Case 2.
					dNewPumpHmin = 0.0;
					dExceedOfPressure = ( dHbypass - dHTrequired );

					CString str;
					CString strHbypass = WriteCUDouble( _U_DIFFPRESS, dHbypass, true );

					// Thanks to the H in the bypass (%1) which is quite large, we no longer need the pump on the secondary side.
					FormatString( str, IDS_HYDROMOD_SECPUMPNOLONGERNEEDED, strHbypass );
					m_mapErrorMessages.insert( std::pair<CDS_HydroMod::eValidityFlags, CString>( CDS_HydroMod::eValidityFlags::evfSecPumpNotNeeded, str ) );
				}

				if( dNewPumpHmin != dPumpHmin )
				{
					GetpPump()->SetHmin( dNewPumpHmin );
				}

				if( NULL != GetpSecBv() && dExceedOfPressure > 0.0 )
				{
					double dBvDpToSet = dBvsecDpMin + dExceedOfPressure;
					GetpSecBv()->ResizeBv( dBvDpToSet );

					double dNewBvsecDpMin = GetpSecBv()->GetDpMin();
					dBvsecDpMin = dNewBvsecDpMin;

					if( dNewBvsecDpMin > dBvDpToSet )
					{
						// When resizing the BV at the secondary side, we can get the case where the size is changed and then
						// that the Dp min is bigger that the 'dBvDpToSet' value.
						double dNewPumpHMin = GetpPump()->GetHmin() + ( dNewBvsecDpMin - dBvDpToSet );
						dPumpHmin = dNewPumpHMin;
						GetpPump()->SetHmin( dNewPumpHMin );
					}
				}

				if( true == bPumpHUserDefined && GetpPump()->GetHpump() < GetpPump()->GetHmin() )
				{
					GetpPump()->SetHpump( GetpPump()->GetHmin() );
				}

				double dNewDpCDpmin = GetpDpC()->GetDpmin();
				double dNewHbypass = fabs( dHAvail - dDpCDpmin );
				double dNewHAvailSec = GetpPump()->GetHAvail() + dNewHbypass;

				if( NULL != pbHasHAvailSecChanged && dNewHAvailSec != dHAvailSec )
				{
					*pbHasHAvailSecChanged = true;
				}
			}
			else
			{
				// We are going to the pump.
				GetpBypBv()->ResizeBv( 0.0 );

				// To be sure that the selection does not take the same valve as the previous one, we invalidate 
				// the previous selection.
				GetpDpC()->InvalidateSelection();
				GetpDpC()->SelectBestDpC( 0.0, true );
			}
		}
		else
		{
			// All other hydraulic circuits.

			if( dHAvail > 0.0 )
			{
				// We must deduce the pressure drop in the circuit pipe.
				// Circuit pipe doesn't exist for a module. Or it exists if there is a secondary side.
				if( false == IsaModule() || ( NULL != GetpSchcat() && true == GetpSchcat()->IsSecondarySideExist() ) )
				{
					dHAvail -= ( GetpCircuitPrimaryPipe()->GetPipeDp() + GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true ) );
				}
			}

			// Resize DpC, if Havail if high enough and if Mv is located on primary selected first Mv with a Dp100% > 3kPa.
			m_bComputeAllowed = false;
			GetpDpC()->ResizeDpC( dHAvail );
			m_bComputeAllowed = true;
		}
	}
	else if( true == IsBvExist() )
	{
		// Case 2 (See above).

		if( dHAvail > 0.0 )
		{
			// We must deduce the pressure drop in the circuit pipe.
			// Circuit pipe doesn't exist for a module. Or it exists if there is a secondary side.
			if( false == IsaModule() || ( NULL != GetpSchcat() && true == GetpSchcat()->IsSecondarySideExist() ) )
			{
				dHAvail -= ( GetpCircuitPrimaryPipe()->GetPipeDp() + GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true ) );
			}

			// We deduce now the pressure drop accross the terminal unit.
			// For injection circuit, this method will return 0.
			dHAvail -= GetDpOnOutOfPrimSide();
		}

		bool bResize = true;

		if( true == IsCvExist() && eb3True == GetpCV()->IsCVLocInPrimary() )
		{
			if( dHAvail <= 0.0 )
			{
				// Resize CV to a minimum if it's not a 3-way mixing circuit.
				if( NULL != GetpSchcat() && GetpSchcat()->Get3WType() != CDB_CircSchemeCateg::e3wTypeMixing )
				{
					GetpCV()->SizeControlValveForBetaMin( 0.0 );
				}
			}
			else
			{
				// If we have a pressure independant balancing & control valve or a Dp combined, balancing & control valve,
				// it is this valve that will take the change of the available differential pressure at the entry of the circuit.
				if( eb3True == GetpCV()->IsPICV() || eb3True == GetpCV()->IsDpCBCV() )
				{
					GetpCV()->SizePIBCVorDpCBCV( dHAvail - GetpBv()->GetDp() );

					// Keep Bv unchanged and let it to its minimum.
					bResize = false;
				}
			}

			// In case of 3W mixing circuit primary Bv is resized in the 'SetHAvail' method.
			if( NULL != GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixing == GetpSchcat()->Get3WType() )
			{
				bResize = false;
			}
		}

		if( true == bResize )
		{
			m_bComputeAllowed = false;
			GetpBv()->ResizeBv( dHAvail );
			m_bComputeAllowed = true;
		}
	}
	else if( true == IsCvExist() && eb3True == GetpCV()->IsCVLocInPrimary() )
	{
		// Case 3 (See above).

		if( dHAvail > 0.0 )
		{
			// We must deduce the pressure drop in the circuit pipe.
			// Circuit pipe doesn't exist for a module. Or it exists if there is a secondary side.
			if( false == IsaModule() || ( NULL != GetpSchcat() && true == GetpSchcat()->IsSecondarySideExist() ) )
			{
				dHAvail -= ( GetpCircuitPrimaryPipe()->GetPipeDp() + GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true ) );
			}

			// We deduce now the pressure drop accross the terminal unit.
			// For injection circuit, this method will return 0.
			dHAvail -= GetDpOnOutOfPrimSide();

			if( eb3True == GetpCV()->IsPICV() || eb3True == GetpCV()->IsDpCBCV() )
			{
				// If we have a pressure independant balancing & control valve or a Dp combined, balancing & control valve,
				// it is this valve that will take the change of the available differential pressure at the entry of the circuit.
				GetpCV()->SizePIBCVorDpCBCV( dHAvail );
			}
			else
			{
				if( NULL != GetpCV()->GetpCV() )
				{
					if( CDB_ControlProperties::eBCVType::ebcvtKvSeries == GetpCV()->GetpCV()->GetCtrlProp()->GetBCVType() )
					{
						// For TBV-CM we close the balancing part in series with the control part without changing the valve size.
						GetpCV()->ResizePresetCV( dHAvail, false );
					}
					else
					{
						// For lift limitation and characteristic independent we can resize the device.
						GetpCV()->ResizePresetCV( dHAvail, true );
					}
				}
			}
		}
		else
		{
			// Select valve.
			GetpCV()->SizeControlValveForBetaMin( 0.0 );
		}
	}
	else if( true == IsSmartControlValveExist() )
	{
		// Case 4 (See above).

		if( dHAvail > 0.0 )
		{
			// Circuit pipe doesn't exist for a module. Or it exists if there is a secondary side.
			if( false == IsaModule() || ( NULL != GetpSchcat() && true == GetpSchcat()->IsSecondarySideExist() ) )
			{
				dHAvail -= ( GetpCircuitPrimaryPipe()->GetPipeDp() + GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true ) );
			}

			// We deduce now the pressure drop accross the terminal unit.
			// For injection circuit, this method will return 0.
			dHAvail -= GetDpOnOutOfPrimSide();
		}

		m_bComputeAllowed = false;
		GetpSmartControlValve()->ResizeSmartControlValve( dHAvail );
		m_bComputeAllowed = true;
	}
	else if( true == IsSmartDpCExist() )
	{
		// Case 4 (See above).

		if( dHAvail > 0.0 )
		{
			// Circuit pipe doesn't exist for a module.
			if( false == IsaModule() )
			{
				dHAvail -= ( GetpCircuitPrimaryPipe()->GetPipeDp() + GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true ) );
			}

			// We deduce now the pressure drop accross the terminal unit.
			// For injection circuit, this method will return 0.
			dHAvail -= GetDpOnOutOfPrimSide();
		}

		m_bComputeAllowed = false;
		GetpSmartDpC()->ResizeValve( dHAvail );
		m_bComputeAllowed = true;
	}
	else
	{
		// Straight pipe no balancing device available.
		if( true == IsaModule() && dHAvail > 0.0 )
		{
			// Circuit pipe doesn't exist for a module.
			if( false == IsaModule() || ( NULL != GetpSchcat() && true == GetpSchcat()->IsSecondarySideExist() ) )
			{
				dHAvail -= ( GetpCircuitPrimaryPipe()->GetPipeDp() + GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true ) );
			}

			GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Dp, dHAvail );
		}
	}

	if( true == bForPending )
	{
		// Restore locking state.
		SetLock( uiLockStatus );
	}
}

void CDS_HydroMod::ResizePumpSecFollowingCVAuthorityChanges( CDS_HydroMod *pHM )
{
	if( NULL == pHM )
	{
		return;
	}

	if( false == pHM->IsaModule() )
	{
		return;
	}

	if( NULL == pHM->GetpSchcat() )
	{
		return;
	}

	if( eDpStab::DpStabOnBVBypass != pHM->GetpSch()->GetDpStab() )
	{
		return;
	}

	double dHbypass = pHM->GetpBypBv()->GetDp();
	double dPumpHmin = pHM->GetpPump()->GetHmin();
	bool bPumpHUserDefined = ( pHM->GetpPump()->GetHpump() > 0.0 ) ? true : false;
	double dPumpHUser = pHM->GetpPump()->GetHpump();
	double dPumpHavail = pHM->GetpPump()->GetHAvail();
	double dBvSecDp = pHM->GetpSecBv()->GetDp();
	double dSecPipesDp = 0.0;
					
	if( NULL != pHM->GetpCircuitSecondaryPipe() )
	{
		dSecPipesDp += pHM->GetpCircuitSecondaryPipe()->GetPipeDp();
		dSecPipesDp += pHM->GetpCircuitSecondaryPipe()->GetSingularityTotalDp( true );
	}

	double dOldTUDp = dHbypass + dPumpHavail - dSecPipesDp - dBvSecDp;
	double dNewTUDp = pHM->GetpTermUnit()->GetDp();
	
	/*
	if( dNewTUDp <= dOldTUDp )
	{
		return;
	}
	*/

	double dDefaultSecondaryPumpHMin = pHM->GetpTechParam()->GetDefaultSecondaryPumpHMin();
	double dNewPumpHmin = 0.0;

	if( dNewTUDp > dHbypass )
	{
		// Case 1.
		if( dNewTUDp - dHbypass < dDefaultSecondaryPumpHMin )
		{
			// Case 1.a.
			dNewPumpHmin = dDefaultSecondaryPumpHMin;
		}
		else
		{
			// Case 1.b.
			dNewPumpHmin = ( dNewTUDp - dHbypass );
		}
	}
	else if( dNewTUDp <= dHbypass )
	{
		// Case 2.
		dNewPumpHmin = 0.0;
	}

	pHM->GetpPump()->SetHmin( dNewPumpHmin );
	double dNewPumpHavail = pHM->GetpPump()->GetHAvail();

	double dNewBvSecDp = dHbypass + dNewPumpHavail - dNewTUDp;
	pHM->GetpSecBv()->ResizeBv( max( dNewBvSecDp, pHM->GetpTechParam()->GetValvMinDp() ) );

	double dNewBvSecDpMin = pHM->GetpSecBv()->GetDpMin();
	double dBilan = dHbypass + dNewPumpHmin - dNewTUDp - dNewBvSecDpMin;

	if( dBilan < 0.0 )
	{
		pHM->GetpPump()->SetHmin( dNewPumpHmin + abs( dBilan ) );
	}

	if( true == bPumpHUserDefined && pHM->GetpPump()->GetHpump() < pHM->GetpPump()->GetHmin() )
	{
		pHM->GetpPump()->SetHpump( pHM->GetpPump()->GetHmin() );
	}
}

double CDS_HydroMod::GetQ( CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide )
{
	double dQ = 0.0;

	if( CAnchorPt::PipeType_Distribution == ePipeType )
	{
		if( CAnchorPt::PipeLocation_Supply == ePipeLocation || ReturnType::Direct == GetReturnType() )
		{
			// If we ask for the supply OR the return but we are in the direct return mode (Same pipe for supply and return in this case).
			if( NULL == m_pDistrSupplyPipe )
			{
				ASSERTA_RETURN( 0.0 );
			}

			dQ = m_pDistrSupplyPipe->GetRealQ();
		}
		else
		{
			// We are in reverse mode and we ask the flow for the distribution return pipe.
			if( NULL == m_pDistrReturnPipe )
			{
				ASSERTA_RETURN( 0.0 );
			}

			dQ = m_pDistrReturnPipe->GetRealQ();
		}
	}
	else if( CAnchorPt::PipeType_Circuit == ePipeType )
	{
		if( CAnchorPt::CircuitSide_Primary == eCircuitSide )
		{
			if( CAnchorPt::PipeLocation::PipeLocation_Supply == ePipeLocation || CAnchorPt::PipeLocation::PipeLocation_Return == ePipeLocation )
			{
				if( NULL == m_pCircuitPrimaryPipe )
				{
					ASSERTA_RETURN( 0.0 );
				}

				dQ = m_pCircuitPrimaryPipe->GetRealQ();
			}
			else if( CAnchorPt::PipeLocation::PipeLocation_ByPass == ePipeLocation )
			{
				if( NULL == m_pCircuitBypassPipe )
				{
					ASSERTA_RETURN( 0.0 );
				}

				dQ = m_pCircuitBypassPipe->GetRealQ();
			}
		}
		else
		{
			// Secondary is threated in the 'CDS_HmInj'. 
			// We must not arrive here in this case!
			ASSERTA_RETURN( 0.0 );
		}
	}

	return dQ;
}

double CDS_HydroMod::GetHMPrimaryQ( )
{
	return GetQDesign();
}

void CDS_HydroMod::SetHMPrimAndSecFlow( double dQ )
{
	if( dQ < 0 || NULL == GetpSch() )
	{
		return;
	}

	// Update design flow(m_qd) for CBI compatibility.
	SetQDesign( dQ );

	// Update Q for CircuitPipe.
	if( NULL != GetpCircuitPrimaryPipe() )
	{
		GetpCircuitPrimaryPipe()->SetRealQ( dQ );
	}

	if( NULL != GetpCircuitBypassPipe() )
	{
		if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() )
		{
			// HYS-1930: auto-adapting variable flow decoupling circuit.

			// In this case, the flow for the bypass must be 1% of the secondary flow.
			// The Q passed as argument is either the Q of the terminal unit OR if injection circuit, it's already the Q computed for the primary side.
			// In the case of auto-adapting variable flow decoupling circuit, the primary flow must be just 1% bigger than the secondary flow.
			// Thus, the Q passed as argument is already multiply by 1,01. To retrieve the secondary flow, we just need to divide by 1,01 and we take 1% of this value.
			//  dQ                          dQ         dQ
			// ---- * 0.01 -> Qbypass = ----------- = ----- -> EX= Qsec = 300 l/h -> dQ = 300 * 1.01 = 303 -> Qbypass = 303/101 = 3 -> 3 is well 1% of the secondary Q!
			// 1.01                      1.01 * 100    101
			GetpCircuitBypassPipe()->SetRealQ( dQ / 101.0 );
		}
		else
		{
			GetpCircuitBypassPipe()->SetRealQ( dQ );
		}
	}

	// Set Q for primary balancing valve.
	if( NULL != GetpBv() )
	{
		GetpBv()->SetQ( dQ );
	}

	if( NULL != GetpBypBv() )
	{
		if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() )
		{
			// HYS-1930: auto-adapting variable flow decoupling circuit.

			// In this case, the flow for the bypass must be 1% of the primary flow.
			GetpBypBv()->SetQ( dQ * 0.01 );
		}
		else
		{
			GetpBypBv()->SetQ( dQ );
		}
	}

	if( NULL != GetpDpC() )
	{
		GetpDpC()->SetQ( dQ );
	}

	if( NULL != GetpCV() )
	{
		GetpCV()->SetQ( dQ, true );
	}

	if( NULL != GetpShutoffValve( eHMObj::eShutoffValveSupply ) )
	{
		GetpShutoffValve( eHMObj::eShutoffValveSupply )->SetQ( dQ );
	}

	if( NULL != GetpShutoffValve( eHMObj::eShutoffValveReturn ) )
	{
		GetpShutoffValve( eHMObj::eShutoffValveReturn )->SetQ( dQ );
	}

	if( NULL != GetpSmartControlValve() )
	{
		GetpSmartControlValve()->SetQ( dQ );
	}

	if( NULL != GetpSmartDpC() )
	{
		GetpSmartDpC()->SetQ( dQ );
	}
}

void CDS_HydroMod::ComputeHM( eComputeHMEvent ComputeHMEvent, LPARAM pObj )
{
	if( false == IsHMCalcMode() )
	{
		return;
	}

	// Cleaning operation that remove un-allowed pipes
	SetFlagModule( IsaModule() );

	// During HydroMod copy Compute HydroMod is deactivated.
	if( false == m_bComputeAllowed )
	{
		return;
	}

	// Sanity tests.
	if( m_ComputeHMState > eceLAST || m_ComputeHMState < eceNone )
	{
		ASSERT( 0 );
		m_ComputeHMState = eceNone;
	}

	// If the engine is already running discard new event...
	if( m_ComputeHMState != eceNone && m_ComputeHMState != eceLAST )
	{
		return;
	}

	if( GetQ() <= 0 )
	{
		m_ComputeHMState = eceNone;
	}

	//SendEventToConsole( ComputeHMEvent );
	_SetComputeHMEngine( ComputeHMEvent, pObj );

	bool bAllShouldBeSized = false;

	while( m_ComputeHMState != eceNone && m_ComputeHMState != eceLAST )
	{
		switch( m_ComputeHMState )
		{
			// Not computed event.
			case eceTUDp:
			case eceBypBvAdded:
			case eceShutoffValveAdded:
			case ecePipeLength:
				_SetComputeHMEngine( eceLAST );
				break;

			case eceResize:
			case eceTUQ:
				bAllShouldBeSized = true;
				_SetComputeHMEngine( ecePipeSerie );

				break;

			// Pipe.
			case ecePipeSerie:
			case ecePipeSize:

				_ProcessCircuitPipes();
				_ProcessBypassPipe();

				// Pipe Size changed -> DN min and DN max change, select a new type of valve
				_SetComputeHMEngine( eceBVType );

				break;

			///////////////////////////////////////////////////////
			// BV
			case eceBVType:
			case eceBVConnect:
			case eceBVVers:
			case eceBVPN:
			case eceBV:
			{
				CBV *pBvParam = NULL;

				if( NULL != m_ComputeHMParam )
				{
					pBvParam = (CBV *)m_ComputeHMParam;
				}

				if( NULL != GetpBv() 
						&& ( NULL == pBvParam || eHMObj::eBVprim == pBvParam->GetHMObjectType() || eHMObj::eBVsec == pBvParam->GetHMObjectType() ) )
				{
					CBV *pBv = GetpBv();
					m_bComputeAllowed = false;

					if( NULL != GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixing != GetpSchcat()->Get3WType() )
					{
						if( eHMObj::eBVprim == pBv->GetHMObjectType() )
						{
							pBv->ResizeBv( 0.0 );
						}
						else if( eHMObj::eBVsec == pBv->GetHMObjectType() && false == IsaModule() )
						{
							pBv->ResizeBv( 0.0 );
						}
					}

					m_bComputeAllowed = true;
				}

				if( NULL != GetpBypBv() 
						&& ( NULL == pBvParam || eHMObj::eBVbyp == pBvParam->GetHMObjectType() ) )
				{
					CBV *pBv = GetpBypBv();
					double dDp = 0.0;

					// 3Ways circuits, bypass valve takes same Dp as TU, should we test that we are in 3ways?
					if( true == GetpSchcat()->Is3W() )
					{
						dDp = GetDpOnOutOfPrimSide();
					}

					m_bComputeAllowed = false;
					pBv->ResizeBv( dDp );
					m_bComputeAllowed = true;
				}

				if( true == bAllShouldBeSized )
				{
					_SetComputeHMEngine( eceCV );
				}
				else
				{
					_SetComputeHMEngine( eceLAST );
				}
			}
			break;

			///////////////////////////////////////////////////////
			// CV
			case eceCV:
			case eceCVVers:
			case eceCVConnect:
			case eceCVBdy:
			case eceCVPN:

				if( NULL != GetpCV() && eb3True == GetpCV()->IsCVLocInPrimary() && NULL != GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixing != GetpSchcat()->Get3WType() )
				{
					GetpCV()->SizeControlValveForBetaMin( 0.0 );
				}

				if( true == bAllShouldBeSized )
				{
					_SetComputeHMEngine( eceDpCType );
				}
				else
				{
					_SetComputeHMEngine( eceLAST );
				}

				break;

			///////////////////////////////////////////////////////
			// DpC
			case eceDpCType:
			case eceDpCConnect:
			case eceDpCPN:
			case eceDpC:

				if( NULL != GetpDpC() )
				{
					if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() )
					{
						// HYS-1930: auto-adapting variable flow decoupling circuit.
						// Try to have the Dp controller with the minimum Dplmin.
						// GetpDpC()->SelectBestDpC( 0.0, true );
						GetpDpC()->SelectBestDpC();
					}
					else
					{
						GetpDpC()->SelectBestDpC();
					}
				}

				// Shutoff valve resizing is asked by 'CDS_Hm2W' when needed. Thus we bypass shutoff valve and
				// we directly go to the smart control valve.
				_SetComputeHMEngine( eceSmartControlValve );

				break;

			///////////////////////////////////////////////////////
			// ShutOffValves
			case eceShutOffValves:
			case eceShutoffValveType:
			case eceShutoffValveConnect:
			case eceShutoffValveVersion:
			case eceShutoffValvePN:
				{
					// Shutoff valve resizing is asked by 'CDS_Hm2W' when needed.
					CShutoffValve *pShutoffValve = ( CShutoffValve * )m_ComputeHMParam;

					if( NULL != pShutoffValve )
					{
						m_bComputeAllowed = false;

						pShutoffValve->ResizeShutoffValve();

						m_bComputeAllowed = true;
					}

					_SetComputeHMEngine( eceLAST );
				}
				break;

			case eceSmartControlValve:
			case eceSmartControlValveBodyMaterial:
			case eceSmartControlValveConnect:
			case eceSmartControlValvePN:

				if( NULL != GetpSmartControlValve() )
				{
					GetpSmartControlValve()->ResizeSmartControlValve( 0.0 );
				}

				_SetComputeHMEngine( eceSmartDpC );

				break;

			case eceSmartDpC:
			case eceSmartDpCBodyMaterial:
			case eceSmartDpCConnect:
			case eceSmartDpCPN:

				if( NULL != GetpSmartDpC() )
				{
					GetpSmartDpC()->ResizeValve( 0.0 );
				}

				_SetComputeHMEngine( eceLAST );

				break;

			default:
				ASSERT( 0 );
				_SetComputeHMEngine( eceNone, 0 );
				break;
		}
	}
}

CDS_HydroMod::ePressIntType CDS_HydroMod::GetPressIntType()
{
	if( true == IsDpCExist() && _T('\0') != *GetSchemeIDPtr().ID && NULL != GetpSch() && CDB_CircuitScheme::eBALTYPE::DPC == GetpSch()->GetBalType() )
	{
		// Verify if DpC stabilize the branch.
		if( DpStabOnBranch == GetpSch()->GetDpStab() )
		{
			return CDS_HydroMod::ePressIntType::pitDpC;
		}
	}

	if( true == IsCvExist() && _T('\0') != *GetSchemeIDPtr().ID && NULL != GetpSch() && CDB_CircuitScheme::eDpCType::eDpCTypeDPCBCV == GetpSch()->GetDpCType() )
	{
		// Case of combined Dp controller, control and balancing valve.
		return CDS_HydroMod::ePressIntType::pitDpC;
	}

	if( true == IsSmartDpCExist() )
	{
		// Case of the smart differential pressure controller.
		return CDS_HydroMod::ePressIntType::pitDpC;
	}

	if( NULL != GetpSch() && eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() )
	{
		// Auto-adapting variable flow decoupling circuit.
		return CDS_HydroMod::ePressIntType::pitDpC;
	}

	return CDS_HydroMod::ePressIntType::pitStatic;
}

// Compute Dp along a path to the pressure interface; used to CV sizing.
// We take into account all pipe dp, balancing device dpmin and CV dp.
// Remark: called only by the "CDS_HydroMod::CCv::SizeControlValveForBetaMin" method. We are thus sure that we
//         have all the time a control valve in the current hydraulic circuit.
double CDS_HydroMod::_GetHreqPressInt()
{
	CDS_HydroMod *pPIHM = NULL;
	CDS_HydroMod *pHM = this;
	double dDpAlongThePath  = 0.0;
	bool bWeAreWorkingOnPrimarySideOfCurrentCiruit = ( eb3True == pHM->GetpCV()->IsCVLocInPrimary() );

	// BE CAREFULL Hreq at the pump doesn't contains CV Dp.
	if( NULL != pHM->GetpCV() )
	{
		if( true == bWeAreWorkingOnPrimarySideOfCurrentCiruit )
		{
			pPIHM = GetpPressIntHM();
		}
		else
		{
			// When CV is localized on secondary the pressure interface is the circuit itself.
			// We are working with the secondary.
			pPIHM = this;
		}

		// In case of 3way injection circuit CV dp is covered by secondary pump.
		if( NULL != GetpSchcat() && GetpSchcat()->Get3WType() != CDB_CircSchemeCateg::e3wTypeMixing )
		{
			dDpAlongThePath -= pHM->GetpCV()->GetDp();
		}
	}

	// Add Dp along the distribution pipes
	if( this != pPIHM )
	{
		CDS_HydroMod *pLoopHM = pHM;

		do
		{
			dDpAlongThePath += pLoopHM->GetHreq( this );

			if( false == pLoopHM->GetIDPtr().PP->IsClass( CLASS( CTableHM ) ) )
			{
				pLoopHM = static_cast<CDS_HydroMod *>( pLoopHM->GetIDPtr().PP );
			}
		}
		while( pLoopHM != pPIHM );
	}
	else
	{
		// Pressure interface is the current circuit.
		if( true == bWeAreWorkingOnPrimarySideOfCurrentCiruit )
		{
			dDpAlongThePath += GetDpOnOutOfPrimSide();
			double dPipeDp = 0;
			double dSingularDp = 0.0;

			// When circuit is stabilized by a Dp controller, we consider circuit Dp outside of stabilized Dp.
			// Second condition is to check combined Dp controller, control and balancing valve (TA-COMPACT-DP).
			// Third condition is for the smart differential pressure controller.
			if( NULL == GetpDpC() && eb3False == GetpCV()->IsDpCBCV() && NULL == GetppSmartDpC() )
			{
				dPipeDp = GetpCircuitPrimaryPipe()->GetPipeDp();
				dSingularDp = GetpCircuitPrimaryPipe()->GetSingularityTotalDp( false );
			}

			dDpAlongThePath += dPipeDp + dSingularDp;
		}
		else
		{
			// Working on secondary side
			dDpAlongThePath += pHM->GetpTermUnit()->GetDp();
			double dPipeDp = GetpCircuitSecondaryPipe()->GetPipeDp();
			double dSingularDp = GetpCircuitSecondaryPipe()->GetSingularityTotalDp( true );
			dDpAlongThePath += dPipeDp + dSingularDp;
		}

		// Done, we are on the pressure interface
		//return dDpAlongThePath;
	}

	// We reach pressure interface
	if( true == bWeAreWorkingOnPrimarySideOfCurrentCiruit )
	{
		if( ( pPIHM->GetpSchcat() != NULL && false == pPIHM->GetpSchcat()->IsSecondarySideExist() ) || pPIHM == pHM )
		{
			// Pressure interface reached add internal components.
			switch( pPIHM->GetPressIntType() )
			{
				// Static balancing, Dp of this HM must be added and pipe Dp to the pump.
				case CDS_HydroMod::ePressIntType::pitStatic:
				{
					if( true == pPIHM->IsDpCExist( true ) && NULL != pPIHM->GetpSch() && DpStabOnCV == pPIHM->GetpSch()->GetDpStab() )
					{
						dDpAlongThePath += pPIHM->GetpDpC()->GetDpmin();
						dDpAlongThePath += pPIHM->GetpDpC()->GetDpL();

						if( true == pPIHM->IsBvExist( true ) && eMvLoc::MvLocPrimary == pPIHM->GetpDpC()->GetMvLoc() )
						{
							dDpAlongThePath += pPIHM->GetpBv()->GetDpMin();
						}
					}
					else
					{
						if( true == pPIHM->IsCvExist( true ) && eb3True == pPIHM->GetpCV()->IsCVLocInPrimary() )
						{
							dDpAlongThePath  += pPIHM->GetpCV()->GetDp();
						}

						if( true == pPIHM->IsBvExist( true ) )
						{
							dDpAlongThePath  += pPIHM->GetpBv()->GetDpMin();
						}
					}
				}
				break;

				// DpC on branch.
				case CDS_HydroMod::ePressIntType::pitDpC:
				{
					// We add this condition to exclude combined Dp controller, control and balancing valve (TA-COMPACT-DP that
					// is CDB_CircuitScheme::eDpCType::eDpCTypeDPCBCV).
					if( NULL != pPIHM->GetpSch() && CDB_CircuitScheme::eDpCType::eDpCTypeDPC == pPIHM->GetpSch()->GetDpCType() )
					{
						if( true == pPIHM->IsCvExist( true ) && eb3True == pPIHM->GetpCV()->IsCVLocInPrimary() )
						{
							dDpAlongThePath += pPIHM->GetpCV()->GetDp();
						}

						if( true == pPIHM->IsBvExist( true ) )
						{
							// Take in count BV only if it's located on secondary.
							CDB_CircuitScheme *pSch = pPIHM->GetpSch();

							if( eMvLoc::MvLocSecondary == pSch->GetMvLoc() )
							{
								dDpAlongThePath += pPIHM->GetpBv()->GetDpMin();
							}
						}
						else if( true == pPIHM->IsBvBypExist( true ) )
						{
							// HYS-1930: auto-adapting variable flow decoupling circuit.
							// Take in count BV only if it's located on secondary.
							CDB_CircuitScheme *pSch = pPIHM->GetpSch();

							if( eMvLoc::MvLocSecondary == pSch->GetMvLoc() )
							{
								dDpAlongThePath += max( pPIHM->GetpBypBv()->GetDpMin(), pPIHM->GetpDpC()->GetDplmin() );
							}
						}
					}

					break;
				}
			}
		}
		// Pressure interface is an injection circuit, we are on the secondary
		else
		{
			// Pump secondary side of Injection circuit, add BV DP if BV exist.
			if( true == pPIHM->IsBvSecExist( true ) )
			{
				dDpAlongThePath += pPIHM->GetpSecBv()->GetDpMin();
			}

			// CV on secondary side.
			if( true == pPIHM->IsCvExist( true ) && ( eb3False == pPIHM->GetpCV()->IsCVLocInPrimary()
					|| ( NULL != pPIHM->GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixing == pPIHM->GetpSchcat()->Get3WType() ) ) )
			{
				dDpAlongThePath += pPIHM->GetpCV()->GetDp();
			}
		}
	}
	// We are working on the secondary side of the current injection circuit.
	else
	{
		// Injection circuit
		// Pump secondary side of Injection circuit, add BV DP if BV exist
		if( true == pPIHM->IsBvSecExist( true ) )
		{
			dDpAlongThePath += pPIHM->GetpSecBv()->GetDpMin();
		}

		// CV on secondary side
		if( true == pPIHM->IsCvExist( true ) && eb3False == pPIHM->GetpCV()->IsCVLocInPrimary() )
		{
			dDpAlongThePath += pPIHM->GetpCV()->GetDp();
		}
	}

	return dDpAlongThePath;
}

double CDS_HydroMod::GetHreq( CDS_HydroMod *pHMOrg )
{
	CTable *pTab = (CTable *) GetIDPtr().PP;
	double dHRequired = 0.0;

	if( _T('\0') != *pTab->GetFirst().ID )
	{
		// Sort children in function of their position (index), descending order.
		// if Pos for pHM is n
		//	  Pos for pNextHM is n+1
		//	  Pos for pPrevHM is n-1

		CRank HMList;
		CString str;

		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			HMList.Add( str, -1 * ( static_cast<CDS_HydroMod *>( IDPtr.MP ) )->GetPos(), (LPARAM)IDPtr.MP );
		}

		CDS_HydroMod *pHM = NULL;
		CDS_HydroMod *pNextHM = NULL;

		// Follows pipes and add pipes dp.
		CDS_HydroMod *pPrevHM = NULL;

		for( BOOL bContinue = HMList.GetFirstT<CDS_HydroMod *>( str, pHM ); TRUE == bContinue; )
		{
			CPipes *pDistributionPipe = NULL;

			if( NULL != pPrevHM )
			{
				pHM = pPrevHM;
			}

			// Splitted outside the loop to increase processing speed of GetConnectDp; GetSingulTotDp.
			bContinue = HMList.GetNextT<CDS_HydroMod *>( str, pPrevHM );

			// At HM position, take in count passage through the HM.
			if( pHM->GetPos() == GetPos() )
			{
				// If we have a CV on the way, we have to take into account real CV dp
				double d = pHM->GetDp( true );

				// Why second condition? In the case of the DpC stabilizes the control valve, the pressure drop on the control valve 
				// is already computed in the stabilized pressure (DpL) of the DpC.
				if( true == pHM->IsCvExist( true ) && false == pHM->IsDpCExist() )
				{
					// HYS-1757: We must also exclude the 3-way mixing with decoupling bypass. Because when calling 'GetDp' with 'true' to have the minimum,
					// for this circuit we never return the minimum pressure drop accross the control valve but well the real pressure drop that is near of
					// 3 kPa in this case (Or equal to 3 kPa if we are working in Kvs without Reynard series).
					if( eb3True == pHM->GetpCV()->IsCVLocInPrimary() && NULL != pHM->GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixing != pHM->GetpSchcat()->Get3WType() 
							&& CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp != pHM->GetpSchcat()->Get3WType() )
					{
						if( false == pHM->GetpCV()->IsTaCV() || false == pHM->GetpCV()->IsPresettable() )
						{
							// We remove Dpmin of the control valve added when calling the 'GetDp( true )' method above.
							d -= pHM->GetpCV()->GetDpMin();

							// And we add now the pressure drop on the control valve as it is computed.
							d += pHM->GetpCV()->GetDp();
						}
					}
				}

				// For the original module we have to take into account full Dp
				if( true == pHM->IsaModule() && pHM != pHMOrg )
				{
					dHRequired += d - pHM->GetDpOnOutOfPrimSide();
				}
				else
				{
					dHRequired += d;
				}
			}

			if( pHM->GetReturnType() == CDS_HydroMod::ReturnType::Direct )
			{
				// Don't care about circuit with a position greater than current position (index).
				// Ex: if parent module *A has 4 children *A.1, *A.2, *A.3 and *A.4 and we call 'GetHreq' on module *A.2, we don't
				//     take *A.3 and *A.4.
				if( pHM->GetPos() > GetPos() )
				{
					continue;
				}

				pDistributionPipe = pHM->GetpDistrSupplyPipe();
			}
			else	// Reverse Return
			{
				if( pHM->GetPos() > GetPos() )
				{
					pDistributionPipe = pHM->GetpDistrReturnPipe();
				}
				else if( pHM->GetPos() < GetPos() )
				{
					pDistributionPipe = pHM->GetpDistrSupplyPipe();
				}
				else		// GetPos() == Pos we have to take into account Distribution Supply and Return, for supply prevHM doesn't exist yet
				{
					pDistributionPipe = pHM->GetpDistrSupplyPipe();
					dHRequired += pDistributionPipe->GetPipeDp() + pDistributionPipe->GetSingularityTotalDp( false, pPrevHM );
					pDistributionPipe = pHM->GetpDistrReturnPipe();
				}
			}

			// Distribution.
			// Remark: Elbow connection singularity is take in count ONLY for circuit and not distribution.
			if( NULL != pNextHM )
			{
				// If a next circuit exists (in fact, if a circuit follows the current one!), we add pressure drop on the
				// connection singularity (tee) with this next distribution pipe.
				dHRequired += pDistributionPipe->GetConnectDp( pNextHM );
			}

			// We add pressure drop on all singularities except the connection one (false).
			dHRequired += pDistributionPipe->GetPipeDp() + pDistributionPipe->GetSingularityTotalDp( false, pPrevHM );
			pNextHM = pHM;
		}
	}

	return dHRequired;
}

void CDS_HydroMod::_VerifyPumpHeadUser( CDS_HydroMod *pclHydroMod, bool *pbHasHuserChanged )
{
	if( NULL == pclHydroMod )
	{
		return;
	}

	if( NULL != pbHasHuserChanged )
	{
		*pbHasHuserChanged = false;
	}

	if( false == pclHydroMod->IsInjectionCircuit() )
	{
		return;
	}

	if( NULL == pclHydroMod->GetpPump() )
	{
		return;
	}

	bool bPumpHUserDefined = ( pclHydroMod->GetpPump()->GetHpump() > 0.0 ) ? true : false;

	if( false == bPumpHUserDefined )
	{
		return;
	}

	double dPumpHUser = pclHydroMod->GetpPump()->GetHpump();

	if( dPumpHUser <= 0.0 )
	{
		return;
	}

	double dPumpHmin = pclHydroMod->GetpPump()->GetHmin();

	if( dPumpHUser <= dPumpHmin )
	{
		dPumpHUser = dPumpHmin;
		pclHydroMod->GetpPump()->SetHpump( dPumpHUser, false );
	}
	else
	{
		if( eDpStab::DpStabOnBVBypass == pclHydroMod->GetpSch()->GetDpStab() && true == pclHydroMod->GetpBypBv()->IsCompletelyDefined() )
		{
			// HYS-1930: auto-adapting variable flow decoupling circuit.

			double dHbypass = pclHydroMod->GetpBypBv()->GetDp();
			double dTUDp = pclHydroMod->GetpTermUnit()->GetDp();
			double dSecPipesDp = 0.0;
					
			if( NULL != pclHydroMod->GetpCircuitSecondaryPipe() )
			{
				dSecPipesDp += pclHydroMod->GetpCircuitSecondaryPipe()->GetPipeDp();
				dSecPipesDp += pclHydroMod->GetpCircuitSecondaryPipe()->GetSingularityTotalDp( true );
			}

			double dBvDpToSet = dHbypass + dPumpHUser - dTUDp - dSecPipesDp;
			pclHydroMod->GetpSecBv()->ResizeBv( dBvDpToSet );

			double dNewBvsDpMin = pclHydroMod->GetpSecBv()->GetDpMin();

			if( dNewBvsDpMin > dBvDpToSet )
			{
				// When resizing the BV at the secondary side, we can get the case where the size is changed and then
				// that the Dp min is bigger that the 'dBvDpToSet' value.
				double dNewPumpHMin = pclHydroMod->GetpPump()->GetHmin() + ( dNewBvsDpMin - dBvDpToSet );
				dPumpHmin = dNewPumpHMin;
				pclHydroMod->GetpPump()->SetHmin( dNewPumpHMin );

				dPumpHUser = dNewPumpHMin;
				pclHydroMod->GetpPump()->SetHpump( dPumpHUser, false );

				if( NULL != pbHasHuserChanged )
				{
					*pbHasHuserChanged = true;
				}
			}
		}
	}
}

// Test code used to verify Ascending and descending phase,
//#define TEST_AVAILABLE_H_DURING_BOTH_PHASES
#ifdef TEST_AVAILABLE_H_DURING_BOTH_PHASES
CMap<CDS_HydroMod *, CDS_HydroMod *, double, double> testMap( 16 );
#endif
// Distribute H available to children module, and resize balancing device
// This function is called after the pump sizing
void CDS_HydroMod::_DistributeHAvailable( CTable *pPHM, double dHAvail, bool bResizeForPendingCircuit )
{
	CRank HMList;
	CString str;

	// Sort children ascending order 1 to n.
	std::map<int, CDS_HydroMod *> mapHMList;
	std::map<int, CDS_HydroMod *>::iterator mapHMListIter;

	for( IDPTR IDPtr = pPHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pPHM->GetNext() )
	{
		mapHMList[static_cast<CDS_HydroMod *>( IDPtr.MP )->GetPos()] = static_cast<CDS_HydroMod *>( IDPtr.MP );
	}
	
	// Havail contains H at the top of distribution pipe.
	CDS_HydroMod *pHM = NULL;
	CDS_HydroMod *pHMReverse = NULL;
	CDS_HydroMod *pPrevHM = NULL;

	if( 0 == mapHMList.size() )
	{
		return;
	}

	// All siblings MUST HAVE the same return type.
	ReturnType eReturnType = mapHMList.begin()->second->GetReturnType();

	switch( eReturnType )
	{
		case ReturnType::Direct:
			for( mapHMListIter = mapHMList.begin(); mapHMListIter != mapHMList.end(); ++mapHMListIter )
			{
				pHM = static_cast<CDS_HydroMod *>( mapHMListIter->second );

				// Remove pressure drop in distribution pipe.
				if( NULL != pPrevHM )
				{
					// Dp in connection from previous hydraulic circuit to the current one.
					double dConnectDp = pPrevHM->GetpDistrSupplyPipe()->GetConnectDp( pHM );

					if( ReturnType::Reverse == pPrevHM->GetReturnType() && NULL != pPrevHM->GetpDistrReturnPipe() )
					{
						dConnectDp += pPrevHM->GetpDistrReturnPipe()->GetConnectDp( pHM );
					}

					dHAvail = dHAvail - dConnectDp;
				}

				// GetSingulTotDp( false ) because we take only singularities that are not a connection.
				dHAvail = dHAvail - ( pHM->GetpDistrSupplyPipe()->GetPipeDp() + pHM->GetpDistrSupplyPipe()->GetSingularityTotalDp( false ) );

				if( ReturnType::Reverse == pHM->GetReturnType() && NULL != pHM->GetpDistrReturnPipe() )
				{
					dHAvail = dHAvail - ( pHM->GetpDistrReturnPipe()->GetPipeDp() + pHM->GetpDistrReturnPipe()->GetSingularityTotalDp( false ) );
				}

				bool bHasPumpHminChangedInSetHavail = false;
				pHM->SetHavail( dHAvail, CAnchorPt::CircuitSide_Primary, false, &bHasPumpHminChangedInSetHavail );

				bool bHasPumpHminChangedInResizeBalDevForH = false;
				pHM->ResizeBalDevForH( max( 0.0, dHAvail ), bResizeForPendingCircuit, CAnchorPt::CircuitSide::CircuitSide_Primary, &bHasPumpHminChangedInResizeBalDevForH );

				// HYS-1707: Because we reset the BV secondary valve to its minimum (See the 'CDS_HydroMod::_ComputeAllToThePump' method), we need 
				// to verify if it's OK in case of user has defined a Pump head higher that the Pump Hmin.
				// HYS-1757: Last condition "false == pHM->IsaModule()" added because we effectively set to 0 but only for circuit.
				if( NULL != pHM->GetpSchcat() && true == pHM->GetpSchcat()->IsSecondarySideExist() && false == pHM->IsaModule() )
				{
					if( eDpStab::DpStabOnBVBypass == pHM->GetpSch()->GetDpStab() )
					{
						// HYS-1930: auto-adapting variable flow decoupling circuit.
						// In this case, we do nothing. All has been completed when calling "ResizeBalDevForH" above !!
					}
					else
					{
						// We voluntary set -1.0 to well manage the resizing in the inherited method.
						pHM->ResizeBalDevForH( -1.0, bResizeForPendingCircuit, CAnchorPt::CircuitSide::CircuitSide_Secondary, &bHasPumpHminChangedInResizeBalDevForH );
					}
				}

				// Now, if we are with a 3-way mixing or an auto-adaptive flow with decoupling bypass circuit, we verify the pump
				// head input by user.
				bool bHasPumpHUserChanged = false;
				_VerifyPumpHeadUser( pHM, &bHasPumpHUserChanged );

				if( true == pHM->IsaModule() && 
					( true == bHasPumpHminChangedInSetHavail || true == bHasPumpHminChangedInResizeBalDevForH || true == bHasPumpHUserChanged ) )
				{
					// If we are with a module and the pump head at the secondary side has changed, we add this circuit to verify for the
					// control valve authoriy of the children.

					CTableHM *pPipingTab = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
					pPipingTab->FillHMToReviewForChildrenAuthority( pHM );
				}

				pPrevHM = pHM;

				// If it's a module, continue to distribute the Available H
				// If it's a injection circuit or a pending circuit stop here
				if( true == pHM->IsaModule() && ( pHM->GetpSchcat() != NULL && false == pHM->GetpSchcat()->IsSecondarySideExist() ) && false == pHM->IsPending() )
				{
					// pHM->GetpTermUnit()->GetDp() has been updated during the ResizeBalDevForH() process
					_DistributeHAvailable( pHM, pHM->GetDpOnOutOfPrimSide(), bResizeForPendingCircuit );
				}
			}

			break;

		case ReturnType::Reverse:
		{
			// 1st pass: precompute the pressure drop in distribution supply and return pipe.
			std::vector<double> vecDistSupplyTotal;
			std::vector<double> vecDistReturnTotal;
			std::map<int, CDS_HydroMod *>::iterator mapHMListIterNext;
			std::map<int, CDS_HydroMod *>::reverse_iterator mapHMListRevIter;
			std::map<int, CDS_HydroMod *>::reverse_iterator mapHMListRevIterPrev;

			double dDpSupplyTotal = 0.0;
			double dDpReturnTotal = 0.0;

			CDS_HydroMod *pHMNext;
			CDS_HydroMod *pHMReversePrev;

			mapHMListRevIter = mapHMList.rbegin();

			for( mapHMListIter = mapHMList.begin(); mapHMListIter != mapHMList.end(); ++mapHMListIter )
			{
				pHM = static_cast<CDS_HydroMod *>( mapHMListIter->second );
				pHMReverse = static_cast<CDS_HydroMod *>( mapHMListRevIter->second );

				dDpSupplyTotal += pHM->GetpDistrSupplyPipe()->GetPipeDp();
				dDpSupplyTotal += pHM->GetpDistrSupplyPipe()->GetSingularityTotalDp( false );

				dDpReturnTotal += pHMReverse->GetpDistrReturnPipe()->GetPipeDp();
				dDpReturnTotal += pHMReverse->GetpDistrReturnPipe()->GetSingularityTotalDp( false );

				vecDistSupplyTotal.push_back( dDpSupplyTotal );
				vecDistReturnTotal.push_back( dDpReturnTotal );

				pHMNext = NULL;
				pHMReversePrev = NULL;

				if( mapHMListIter != mapHMList.end() )
				{
					mapHMListIterNext = mapHMListIter;
					++mapHMListIterNext;

					if( mapHMListIterNext != mapHMList.end() )
					{
						pHMNext = mapHMListIterNext->second;
						mapHMListRevIterPrev = mapHMListRevIter;
						++mapHMListRevIterPrev;
						pHMReversePrev = mapHMListRevIterPrev->second;
					}
				}

				dDpSupplyTotal += pHM->GetpDistrSupplyPipe()->GetConnectDp( pHMNext );
				dDpReturnTotal += pHMReverse->GetpDistrReturnPipe()->GetConnectDp( pHMReversePrev );

				++mapHMListRevIter;
			}

			// 2nd pass: we can now compute HAvail on all circuits.
			int iLoop = 0;
			int iLoopRev = ( int ) vecDistSupplyTotal.size() - 1;

			for( mapHMListIter = mapHMList.begin(); mapHMListIter != mapHMList.end(); ++mapHMListIter )
			{
				pHM = static_cast<CDS_HydroMod *>( mapHMListIter->second );

				double dHAvailCircuit = dHAvail - ( vecDistSupplyTotal[iLoop] + vecDistReturnTotal[iLoopRev] );

				pHM->SetHavail( dHAvailCircuit, CAnchorPt::CircuitSide_Primary );
				pHM->ResizeBalDevForH( max( 0.0, dHAvailCircuit ), bResizeForPendingCircuit, CAnchorPt::CircuitSide::CircuitSide_Primary );

				// If it's a module, continue to distribute the Available H.
				// If it's a injection circuit or a pending circuit stop here.
				if( true == pHM->IsaModule() && ( pHM->GetpSchcat() != NULL && false == pHM->GetpSchcat()->IsSecondarySideExist() ) && false == pHM->IsPending() )
				{
					// pHM->GetpTermUnit()->GetDp() has been updated during the ResizeBalDevForH() process.
					_DistributeHAvailable( pHM, pHM->GetDpOnOutOfPrimSide(), bResizeForPendingCircuit );
				}

				iLoopRev--;
				iLoop++;
			}
		}
		break;
	}
}

// Scan all circuit included into the ordered list and compute H needed at the entry of the module
void CDS_HydroMod::_UpdateDp( std::map<int, CDS_HydroMod *> *pmapChildren, bool bResizeForPendingCircuit, double *pdHUpStream, double *pdHNeedCurrent,
		bool bResizeToFullOpening )
{
	if( NULL == pmapChildren || NULL == pdHUpStream || NULL == pdHNeedCurrent )
	{
		ASSERT_RETURN;
	}

	std::map<int, CDS_HydroMod *>::iterator mapChildrenIter;
	std::map<int, CDS_HydroMod *>::iterator mapChildrenIterNext;

	// H in the entry of distribution pipe of this HM.
	*pdHUpStream = 0.0;
	CDS_HydroMod *pIndexHM = NULL;
	CDS_HydroMod *pChildHM = NULL;
	CDS_HydroMod *pPrevHM = NULL;

	// These 3 next variables are used only for the reverse mode.
	*pdHNeedCurrent = 0.0;
	double dHNeedTemp = 0.0;
	double dDpDistributionReturnPipeTotal = 0.0;

	for( mapChildrenIter = pmapChildren->begin(); mapChildrenIter != pmapChildren->end(); ++mapChildrenIter )
	{
		pChildHM = mapChildrenIter->second;

		// Compute available H for this module.
		if( NULL != pPrevHM )
		{
			// Previous circuit exist, HUpStream must be Updated with Dp from Tee connection.
			// We have to use pPrevHM because GetConnectDp should be used with a parameter 'NextCircuit' that takes
			// a distribution flow lower than the distribution flow in current circuit.

			// We are computing here Dp in tee in front of circuit n; Havailable before this operation
			// is the Dp available at the entry of distribution pipe of circuit n+1.
			double d = pChildHM->GetpDistrSupplyPipe()->GetConnectDp( pPrevHM );
			*pdHUpStream += d;

			// If we are in reverse return mode...
			if( ReturnType::Reverse == pChildHM->GetReturnType() /*eReturnType*/ && NULL != pChildHM->GetpDistrReturnPipe() )
			{
				// We remove pressure drop on the distribution return pipe and all of its singularities (without connection).
				double d = pChildHM->GetpDistrReturnPipe()->GetSingularityTotalDp( false );
				*pdHUpStream -= d;
				*pdHUpStream -= pChildHM->GetpDistrReturnPipe()->GetPipeDp();
			}
		}

		double dHNeededAtTheCircuitInput = 0.0;

		// We have to reset HAvail because pclChildHM->GetDp() use this value for Dp controller.
		// Third parameter 'true' to specify that we are going to the pump.
		pChildHM->SetHavail( dHNeededAtTheCircuitInput, CAnchorPt::CircuitSide_Primary, true );

		// 2016-12-19: when we are going to the pump, we need the Dp min!!
		dHNeededAtTheCircuitInput = pChildHM->GetDp( bResizeToFullOpening );

		// Why second condition? In the case of the DpC stabilizes the control valve, the pressure drop on the control valve 
		// is already computed in the stabilized pressure (DpL) of the DpC.
		if( true == bResizeToFullOpening && true == pChildHM->IsCvExist( true ) && false == pChildHM->IsDpCExist() )
		{
			// HYS-1757: We must also exclude the 3-way mixing with decoupling bypass. Because when calling 'GetDp' with 'true' to have the minimum,
			// for this circuit we never return the minimum pressure drop accross the control valve but well the real pressure drop that is near of
			// 3 kPa in this case (Or equal to 3 kPa if we are working in Kvs without Reynard series).
			if( eb3True == pChildHM->GetpCV()->IsCVLocInPrimary() && NULL != pChildHM->GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixing != pChildHM->GetpSchcat()->Get3WType() 
					&& CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp != pChildHM->GetpSchcat()->Get3WType() )
			{
				if( false == pChildHM->GetpCV()->IsTaCV() || false == pChildHM->GetpCV()->IsPresettable() )
				{
					// We remove Dpmin of the control valve added when calling the 'GetDp( true )' method above.
					dHNeededAtTheCircuitInput -= pChildHM->GetpCV()->GetDpMin();

					// And we add now the pressure drop on the control valve as it is computed.
					dHNeededAtTheCircuitInput += pChildHM->GetpCV()->GetDp();
				}
			}
		}

		// By default clear flag index.
		pChildHM->SetFlagCircuitIndex( false );

		// If this circuit need less Dp than the previous one...
		if( dHNeededAtTheCircuitInput < *pdHUpStream )
		{
			// HYS-1930: no need to call 'pChildHM->SetHavail' now, it will be set later when calling the '_UpdateHAvailAccordingTUDp' method.
			// pChildHM->SetHavail( *pdHUpStream, CAnchorPt::CircuitSide_Primary );
			dHNeededAtTheCircuitInput = *pdHUpStream;
		}
		else
		{
			// H needed for this circuit is higher than H available. We found a new index circuit.
			
			// HYS-1930: no need to call 'pChildHM->SetHavail' now, it will be set later when calling the '_UpdateHAvailAccordingTUDp' method.
			// pChildHM->SetHavail( dHNeededAtTheCircuitInput, CAnchorPt::CircuitSide_Primary );

			if( false == pChildHM->IsPending() )
			{
				if( NULL != pIndexHM )
				{
					pIndexHM->SetFlagCircuitIndex( false );
				}

				pChildHM->SetFlagCircuitIndex( true );
				pIndexHM = pChildHM;
			}
		}

		#ifdef TEST_AVAILABLE_H_DURING_BOTH_PHASES
		testMap[pChildHM] = pChildHM->GetHAvail( Primary );
		#endif

		// H needed at the HM (circuit itself + distribution pipe) entry is the sum of:
		//    - Circuit Dp including TU, Balancing devices, circuit pipe and accessories.
		//    - Distribution pipe Dp.
		//    - Accessories on distribution pipes.
		//

		// GetSingulTotDp(false) doesn't take into account Dp on tee used to connect pPrevHM
		
		// HYS-1930: we keep the value computed above 'dHNeededAtTheCircuitInput'.
		// *pdHUpStream = pChildHM->GetHAvail( CAnchorPt::CircuitSide_Primary ) + pChildHM->GetpDistrSupplyPipe()->GetPipeDp() + pChildHM->GetpDistrSupplyPipe()->GetSingularityTotalDp( false );
		*pdHUpStream = dHNeededAtTheCircuitInput + pChildHM->GetpDistrSupplyPipe()->GetPipeDp() + pChildHM->GetpDistrSupplyPipe()->GetSingularityTotalDp( false );

		if( ReturnType::Reverse ==  pChildHM->GetReturnType() && NULL != pChildHM->GetpDistrReturnPipe() )
		{
			// In direct return mode, '*pdHUpStream' will allow us to determine the H needed at the entry of the module.
			// In reverse return mode', this variable only allow us to determine the HAvail on each circuit.
			// To retrieve H needed, we must to do it in an another way.

			// Get the total pressure drop on distribution return pipe from the end to the outlet of current circuit
			// and add singularities on this distribution return pipe except the connection (false).
			double dReturnPipeDp = pChildHM->GetpDistrReturnPipe()->GetPipeDp() + pChildHM->GetpDistrReturnPipe()->GetSingularityTotalDp( false );
			dDpDistributionReturnPipeTotal += dReturnPipeDp;

			double dSupplyPipeDp = pChildHM->GetpDistrSupplyPipe()->GetPipeDp() + pChildHM->GetpDistrSupplyPipe()->GetSingularityTotalDp( false );

			// HYS-1930: we keep the value computed above 'dHNeededAtTheCircuitInput'.
			// dHNeedTemp = pChildHM->GetHAvail( CAnchorPt::CircuitSide_Primary ) + dSupplyPipeDp;
			dHNeedTemp = dHNeededAtTheCircuitInput + dSupplyPipeDp;
			
			dHNeedTemp += dDpDistributionReturnPipeTotal;

			if( dHNeedTemp > *pdHNeedCurrent )
			{
				// This temporary circuit becomes the one that has the bigger need.
				*pdHNeedCurrent = dHNeedTemp;
			}
			else
			{
				// Update H need of current circuit.

				// Add pressure drop on the current distribution supply pipe and all of its singularities except the connection one.
				*pdHNeedCurrent += dSupplyPipeDp;
				// Add pressure drop in the connection singularity between current circuit (n) and the previous one (n+1).
				*pdHNeedCurrent += pChildHM->GetpDistrSupplyPipe()->GetConnectDp( pPrevHM );
			}

			// For the next circuit, we need to add the pressure drop in the connection singularity between current circuit (n) and
			// the previous one (n-1).
			mapChildrenIterNext = mapChildrenIter;
			++mapChildrenIterNext;

			// We do that only if of course there is a circuit at the n-1 position.
			if( mapChildrenIterNext != pmapChildren->end() )
			{
				double dDpConnect = pChildHM->GetpDistrReturnPipe()->GetConnectDp( mapChildrenIterNext->second );
				dDpDistributionReturnPipeTotal += dDpConnect;
				*pdHUpStream -= dDpConnect;
			}
		}

		pPrevHM = pChildHM;
	}
}

// After CV modification (Beta min computation)  H needed for circuits and Index valve position change, we have to recompute Dp in branch and index position
void CDS_HydroMod::_AdjustDpAfterCVResizing( CDS_HydroMod *pOrgHM )
{
	CDS_HydroMod *pHM = this;

	// Scan all children, go deeper into HM struct.
	for( IDPTR IDPtr = pHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pHM->GetNext( IDPtr.MP ) )
	{
		CDS_HydroMod *pcHM = static_cast<CDS_HydroMod *>( IDPtr.MP );

		// Compute children, only when current module is not a module with a bypass.
		if( true == pcHM->IsaModule() && false == pcHM->IsPressureInterfaceExist() )
		{
			pcHM->_AdjustDpAfterCVResizing( pOrgHM );
		}
	}

	// Pressure interface is a module...
	if( true == pHM->IsaModule() )
	{
		// When pHM is a child module or original hydromod w/o shortcut bypass.
		if( false == pHM->IsPressureInterfaceExist() || pHM == pOrgHM )
		{
			ReturnType eReturnType = ReturnType::LastReturnType;
			std::map<int, CDS_HydroMod *> mapChildren;

			// Sort children in function of their position, starting from the latest circuit in the module and going to
			// the first circuit (circuit n to circuit 1).
			for( IDPTR IDPtr = pHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pHM->GetNext() )
			{
				CDS_HydroMod *pcHM = dynamic_cast<CDS_HydroMod *>( IDPtr.MP );

				if( NULL == pcHM )
				{
					continue;
				}

				// Save pos-hydromod list.
				mapChildren[-1 * pcHM->GetPos()] = pcHM;
				eReturnType = pcHM->GetReturnType();
			}

			double dHUpStream = 0;
			double dHNeedCurrent = 0;
			// pHM->_UpdateDp( &mapChildren, false, &dHUpStream, &dHNeedCurrent, false );
			pHM->_UpdateDp( &mapChildren, false, &dHUpStream, &dHNeedCurrent, true );

			// Clean all maps.
			mapChildren.clear();

			// Update Terminal unit Dp for the module.
			if( ReturnType::Direct == eReturnType )
			{
				pHM->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Dp, dHUpStream, 0, false );
			}
			else
			{
				pHM->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Dp, dHNeedCurrent, 0, false );
			}
		}
	}
}

void CDS_HydroMod::_SetPipeDiversityFlag( CDS_HydroMod *pStartHM, bool bDiversityFlag, CDS_HydroMod *pHM )
{
	if( NULL == pStartHM )
	{
		return;
	}

	// Visit all children.
	// Remark: - if 'pHM' is NULL it's the first pass. In this case we set 'Diversity Applied' flag in distribution pipes of
	//           all children of 'pStartHM'.
	//         - In next passes, we set the 'Diversity Applied' flag in distribution pipes only for modules that are
	//           positioned before 'pHM'.
	for( IDPTR IDPtr = pStartHM->GetFirst(); IDPtr.MP != NULL; IDPtr = pStartHM->GetNext( IDPtr.MP ) )
	{
		CDS_HydroMod *pChildHM = static_cast<CDS_HydroMod *>( IDPtr.MP );

		if( NULL == pChildHM )
		{
			continue;
		}

		if( NULL == pHM || ( pChildHM->GetPos() <= pHM->GetPos() ) )
		{
			if( pChildHM->GetpDistrSupplyPipe() != NULL )
			{
				pChildHM->GetpDistrSupplyPipe()->SetDiversityAppliedFlag( bDiversityFlag );
			}

			if( ReturnType::Reverse == pChildHM->GetReturnType() && pChildHM->GetpDistrReturnPipe() != NULL )
			{
				pChildHM->GetpDistrReturnPipe()->SetDiversityAppliedFlag( bDiversityFlag );
			}
		}
	}

	// If parent exists...
	if( pStartHM->GetLevel() > 0 && pStartHM->GetIDPtr().PP != NULL )
	{
		_SetPipeDiversityFlag( static_cast<CDS_HydroMod *>( pStartHM->GetIDPtr().PP ), bDiversityFlag, pStartHM );
	}
}

void CDS_HydroMod::_UpdateQDp( bool bResize, bool bResizeForPendingCircuit, bool OnlyUpdateHNeededAtTheCircuitInterface )
{
	if( false == IsaModule() )
	{
		ASSERT_RETURN;
	}

	ASSERT( true == GetpTermUnit()->IsVirtual() );

	// Because 'TotalQ', 'RealQ' and 'PartialQ' partly allow to update flow in different distribution pipe, if we are working
	// in a reverse return mode, we need to distinguish between distribution supply and return pipe.
	double dQSupplyTotal = 0.0;				// Will contain total Q in the module.
	double dQSupplyPartial = 0.0;			// Without pending circuit.
	double dQSupplyTotalwoDiversity = 0.0;
	double dQReturnTotal = 0.0;				// Will contain total Q in the module.
	double dQReturnPartial = 0.0;			// Without pending circuit.
	double dQReturnTotalwoDiversity = 0.0;

	// By default.
	ReturnType eReturnType = ReturnType::LastReturnType;

	CTableHM *pPipingTab = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
	ASSERT( NULL != pPipingTab );

	// Module must include at least one circuit
	if( _T('\0') != GetFirst().ID )
	{
		// Will contain list of all children modules from the deeper to the first one (circuit n to 1).
		std::map<int, CDS_HydroMod *> mapChildren;
		std::map<int, CDS_HydroMod *>::iterator mapChildrenIter;
		std::map<int, CDS_HydroMod *>::reverse_iterator mapChildrenReverseIter;

		// Will contain list of all circuit module and their flow from the lower to the bigger.
		std::multimap<double, CDS_HydroMod *> mmapQ;
		std::multimap<double, CDS_HydroMod *>::reverse_iterator ritQ;

		// Allow to save the lock state for each child between case 0 and 1 (see below cases) when we are resizing for pending circuit.
		std::map<CDS_HydroMod *, unsigned int> mapLockState;

		double dTotalQWithDiversityWanted = 0.0;

		if( true == bResizeForPendingCircuit )
		{
			mapLockState[this] = GetLock();
			SetLock( eALL );
		}

		// HYS-1784: We can't keep anymore only one loop to fill the "mapChildren" and "mmapQ" maps. Why ?
		// Temperatures information for pipes are refreshed by calling the "VerifyWaterCharacteristic" method.
		// To be able to refresh temperature, we need the next circuit of the current one if it exists.
		// In this first loop, we fill the "mapChildren" with their right order. At this moment, we don't have next circuit.
		// The call to the "VerifyWaterCharacteristic" method must be done after.
		// And, because we don't have yet temperature information, we can't compute flow.
		// The fill of the "mmapQ" map must also be done later.

		// Sort children in function of their position, starting from the latest circuit in the module and going to
		// the first circuit (circuit n to circuit 1).
		for( IDPTR IDPtr = GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = GetNext( IDPtr.MP ) )
		{
			CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( IDPtr.MP );

			if( NULL == pHM )
			{
				continue;
			}

			// Memorize the lock status for current child if we are resizing for pending circuit. Because when computing for pending
			// circuit, we MUST NOT change any size of pipe and devices. Then we lock all. After that, we must return lock status to
			// its initial state.
			if( true == bResizeForPendingCircuit )
			{
				mapLockState[pHM] = pHM->GetLock();
				pHM->SetLock( eALL );
			}

			// Save pos-hydromod list.
			mapChildren[-1 * pHM->GetPos()] = pHM;

			// Force Q updating
			// In case of of shortcutbypass, circuit was proceesed in a previous loop, skip it
			// Return type MUST BE always the same for all siblings.
			if( eReturnType == LastReturnType )
			{
				eReturnType = pHM->GetReturnType();
			}
			else
			{
				ASSERT( eReturnType == pHM->GetReturnType() );

				if( eReturnType != pHM->GetReturnType() )
				{
					// HYS-1359 : The element is not compatible regarding to the return mode.
					pHM->SetReturnType( eReturnType );
				}
			}
		}

		// HYS-1784: Now update temperatures in pipes.
		CDS_HydroMod *pclChildHM = NULL;
		CDS_HydroMod *pclPreviousChildHM = NULL;

		for( mapChildrenIter = mapChildren.begin(); mapChildrenIter != mapChildren.end(); ++mapChildrenIter )
		{
			pclChildHM = mapChildrenIter->second;
			pclChildHM->VerifyWaterCharacteristic( NULL, pclPreviousChildHM );
			pclPreviousChildHM = pclChildHM;
		}

		// HYS-1784: Now fill the "mmapQ" map.
		for( mapChildrenIter = mapChildren.begin(); mapChildrenIter != mapChildren.end(); ++mapChildrenIter )
		{
			pclChildHM = mapChildrenIter->second;
			
			// Force Q updating.
			double dQ = pclChildHM->GetpTermUnit()->GetQ();

			// In case of of shortcutbypass, circuit was proceesed in a previous loop, skip it.
			// HYS-1785: if the current hydraulic circuit is a pressure interface but not a module, we force the update of the primary side.
			if( false == pclChildHM->IsPressureInterfaceExist() || false == pclChildHM->IsaModule() )
			{
				if( false == bResizeForPendingCircuit )
				{
					pclChildHM->SetHMPrimAndSecFlow( dQ );

					if( true == bResize )
					{
						pclChildHM->ComputeHM( eComputeHMEvent::eceResize );
					}
				}
			}

			// Save flow-hydromod list.
			dTotalQWithDiversityWanted += pclChildHM->GetHMPrimaryQ();
			mmapQ.insert( pair<double, CDS_HydroMod *> ( pclChildHM->GetHMPrimaryQ(), pclChildHM ) );
		}

		double dTotalQWithDiversityAdapted = dTotalQWithDiversityWanted;

		// Compute max flow according diversity factor.
		// Remark: -1.0 is the value to tell that there is not yet diversity factor applied.
		if( GetDiversityFactor() > 0.0 && GetDiversityFactor() < 1.0 )
		{
			pPipingTab->SetDiversityExist( true );
			_SetPipeDiversityFlag( this, true );

			// Apply diversity in the second phase of computing (See 'ComputeAll').
			if( true == pPipingTab->GetDiversityActive() )
			{
				dTotalQWithDiversityWanted *= GetDiversityFactor();
			}

			// In mmapQ circuits have been ordered by flow should it be by power?
			// Compute Maximized flow.
			dTotalQWithDiversityAdapted = 0.0;

			for( ritQ = mmapQ.rbegin(); ritQ != mmapQ.rend() && dTotalQWithDiversityAdapted < dTotalQWithDiversityWanted ; ++ritQ )
			{
				dTotalQWithDiversityAdapted += ritQ->first;
			}
		}

		double dHUpStream;

		// These 3 next variables are used only for the reverse mode.
		double dHNeedCurrent = 0.0;
		double dHNeedTemp = 0.0;
		double dDpDistributionReturnPipeTotal = 0.0;

		// 3 Steps:
		//
		// 	1) Set good connection for each pipe.
		//
		//  2) We set the flow for each pipe. In direct mode, we go from the deeper to the first circuit add we add flow of each
		//      Terminal Unit. With a limitation if diversity factor is active. In reverse mode, we exactly do the same for the distribution
		//      supply pipe. For distribution return pipe, we do the same but in the other direction, from the first circuit to the deeper.
		//
		//  3) Now that we have all flow in pipes, we can compute H needed in the input of this module.


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// First step: set the good connection for each pipes.
		// Remark: no need to be done again if we are resizing for pending circuit.
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if( false == bResizeForPendingCircuit )
		{
			for( mapChildrenIter = mapChildren.begin(); mapChildrenIter != mapChildren.end(); ++mapChildrenIter )
			{
				pclChildHM = mapChildrenIter->second;
				_VerifyConnection( pclChildHM, this );
			}

			// HYS-1761: We verify also root module to remove singularities on pipes that can come from a copy/paste operation.
			if( 0 == this->GetLevel() )
			{
				_VerifyConnection( this, NULL );
			}
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Second step: set the good flow in each pipe.
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		for( mapChildrenIter = mapChildren.begin(); mapChildrenIter != mapChildren.end(); ++mapChildrenIter )
		{
			pclChildHM = mapChildrenIter->second;

			// Reset flag index.
			pclChildHM->SetFlagCircuitIndex( false );

			// If resizing needed launch a complete re-suggestion on each circuit.

			// Get flow of the circuit and add it to the total flow; needed to determine flow in distribution pipe.
			dQSupplyTotal = dQSupplyTotal + pclChildHM->GetHMPrimaryQ();

			// When the hydromod is a module where the diversity is acting; GetHMPrimaryQ() will return the real flow and not the total flow w/o diversity.
			// This the reason why we need to use an another variable to store total flow without diversity.
			dQSupplyTotalwoDiversity += pclChildHM->GetTotQwoDiversity();

			if( false == pclChildHM->IsPending() )
			{
				// In case of a module w/o decoupling bypass PartialPending has been updated previously, we scan HM starting from deepest module
				if( false == pclChildHM->IsaModule() )
				{
					// Partial pending Q is a working variable, initialized to 0.
					// We update this variable to the requested flow here
					pclChildHM->GetpTermUnit()->SetPartialPendingQ( pclChildHM->GetpTermUnit()->GetQ() );
				}

				// We use virtual function CDS_HydroMod::GetPartialPendingQ() that take into account injection circuit if needed.
				dQSupplyPartial = dQSupplyPartial + pclChildHM->GetPartialPendingQ();
			}
			else
			{
				// We don't take into account flow when the circuit is pending.
				pclChildHM->GetpTermUnit()->SetPartialPendingQ( 0.0 );
				// dQSupplyPartial = dQSupplyPartial + 0.0 ;
			}

			if( false == bResizeForPendingCircuit )
			{
				// Diversity, maximize the flow in distribution supply pipe to the diversity max flow adapted.
				double dQ = min( dQSupplyTotal, dTotalQWithDiversityAdapted );
				pclChildHM->GetpDistrSupplyPipe()->SetRealQ( dQ );
				pclChildHM->GetpDistrSupplyPipe()->SetTotalQ( dQSupplyTotalwoDiversity );
				pclChildHM->GetpDistrSupplyPipe()->SetPartialPendingQ( dQSupplyPartial );
				IDPTR idptr = pclChildHM->GetpDistrSupplyPipe()->SelectBestPipe( dQ );

				if( false == pclChildHM->GetpDistrSupplyPipe()->IsLocked() )
				{
					pclChildHM->GetpDistrSupplyPipe()->SetIDPtr( idptr, false );
				}

				// Circuit Pipe.
				// Remark: When the circuit is pending, flow in circuit pipe should be set to 0, it will be done later w/o resizing pipe.
				pclChildHM->GetpCircuitPrimaryPipe()->SetTotalQ( pclChildHM->GetQ() );
				pclChildHM->GetpCircuitPrimaryPipe()->SetRealQ( pclChildHM->GetQ() );
				idptr = pclChildHM->GetpCircuitPrimaryPipe()->SelectBestPipe( pclChildHM->GetQ() );

				if( false == pclChildHM->GetpCircuitPrimaryPipe()->IsLocked() )
				{
					pclChildHM->GetpCircuitPrimaryPipe()->SetIDPtr( idptr, false );
				}
			}
			else
			{
				pclChildHM->GetpDistrSupplyPipe()->SetRealQ( pclChildHM->GetpDistrSupplyPipe()->GetPartialPendingQ() );

				if( true == pclChildHM->IsPending() )
				{
					// When the circuit is pending flow in circuit pipe should be set to 0.
					pclChildHM->GetpCircuitPrimaryPipe()->SetRealQ( 0.0 );
				}
			}

			pclPreviousChildHM = pclChildHM;
		}

		// If we are in reverse return mode...
		if( ReturnType::Reverse == eReturnType )
		{
			// Compute all flows in distribution return pipe going from first circuit to the deeper.
			for( mapChildrenReverseIter = mapChildren.rbegin(); mapChildrenReverseIter != mapChildren.rend(); ++mapChildrenReverseIter )
			{
				pclChildHM = mapChildrenReverseIter->second;

				// Get flow of the circuit and add it to the total flow; needed to determine flow in distribution pipe.
				dQReturnTotal = dQReturnTotal + pclChildHM->GetHMPrimaryQ();

				// When the hydromod is a module where the diversity is acting; GetQ() will return the real flow and not the total flow w/o diversity.
				// This the reason why we need to use an another variable to store total flow without diversity.
				if( true == pclChildHM->IsaModule() )
				{
					dQReturnTotalwoDiversity += pclChildHM->GetTotQwoDiversity();
				}
				else
				{
					dQReturnTotalwoDiversity += pclChildHM->GetHMPrimaryQ();
				}

				dQReturnPartial = dQReturnPartial + pclChildHM->GetPartialPendingQ();

				if( false == bResizeForPendingCircuit )
				{
					// Diversity, maximize the flow in distribution return pipe to the diversity max flow adapted.
					double dQ = min( dQReturnTotal, dTotalQWithDiversityAdapted );
					pclChildHM->GetpDistrReturnPipe()->SetRealQ( dQ );

					pclChildHM->GetpDistrReturnPipe()->SetTotalQ( dQReturnTotalwoDiversity );

					pclChildHM->GetpDistrReturnPipe()->SetPartialPendingQ( dQReturnPartial );
					IDPTR idptr = pclChildHM->GetpDistrReturnPipe()->SelectBestPipe( dQ );

					if( false == pclChildHM->GetpDistrReturnPipe()->IsLocked() )
					{
						pclChildHM->GetpDistrReturnPipe()->SetIDPtr( idptr, false );
					}
				}
				else
				{
					pclChildHM->GetpDistrReturnPipe()->SetRealQ( pclChildHM->GetpDistrReturnPipe()->GetPartialPendingQ() );
				}
			}
		}

		// HYS-1882: If distribution return pipe real Q is change we should update distribution return temperature.
		// It occure refresh problem. When we change the flow in sheetHMcalc, Distribution return temp is not well calculated.
		// We have to edit one circuit to update it.
		pclPreviousChildHM = NULL;
		for( mapChildrenIter = mapChildren.begin(); mapChildrenIter != mapChildren.end(); ++mapChildrenIter )
		{
			pclChildHM = mapChildrenIter->second;
			pclChildHM->VerifyWaterCharacteristic( NULL, pclPreviousChildHM );
			pclPreviousChildHM = pclChildHM;
		}

		// HYS-1882: If distribution return pipe real Q is change we should update distribution return temperature.
		// It occure refresh problem. When we change the flow in sheetHMcalc, Distribution return temp is not well calculated.
		// We have to edit one circuit to update it.
		pclPreviousChildHM = NULL;
		for( mapChildrenIter = mapChildren.begin(); mapChildrenIter != mapChildren.end(); ++mapChildrenIter )
		{
			pclChildHM = mapChildrenIter->second;
			pclChildHM->VerifyWaterCharacteristic( NULL, pclPreviousChildHM );
			pclPreviousChildHM = pclChildHM;
		}

		// HYS-1716.
		VerifyWaterCharacteristic( ( true == IsDesignTemperatureInterface() ) ? this : NULL );

		///////////////////////////////////////////////////////////////////////////
		// STEP 3 ---> Compute HAvail on each circuit and H needed for the module.
		///////////////////////////////////////////////////////////////////////////
		_UpdateDp( &mapChildren, bResizeForPendingCircuit, &dHUpStream, &dHNeedCurrent, true );

		// Clean all maps.
		mapChildren.clear();
		mmapQ.clear();

		// Compute completely this branch before signaling to the parent.
		// Qtot contains total flow for this module, pHM is child at the first position.
		// Update virtual Terminal Unit with Qtot and H needed for children.
		double dQ = min( dQSupplyPartial, dTotalQWithDiversityAdapted );
		GetpTermUnit()->SetPartialPendingQ( dQ );

		if( true == bResizeForPendingCircuit )
		{
			// HYS-1755: we force the refresh even if it's the same flow.
			// We already had old unit tests with primary flows for hydraulic circuit with secondary side that were not well refreshed.
			// For example, in the HYS-1716 Jira card, we review the formula to compute the primary flow. In this case, secondary flow in the virtual terminal
			// unit of the secondary side of a module doesn't change. But we need a refresh.
			GetpTermUnit()->SetQ( dQ, false );
		}
		else
		{
			double dQ = min( dQSupplyTotal, dTotalQWithDiversityAdapted );
			GetpTermUnit()->SetTotQwoDiversity( dQSupplyTotal );

			// HYS_1755: same remark as above.
			GetpTermUnit()->SetQ( dQ, false );
		}

		// Update Terminal unit Dp for the module
		if( ReturnType::Direct == eReturnType )
		{
			GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Dp, dHUpStream, 0, false );
		}
		else
		{
			GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Dp, dHNeedCurrent, 0, false );
		}

		// Resize module according new flow, new Dp in virtual terminal unit.
		// HYS-810: ComputeHM have to be done with 2wInjCircuit
		// HYS-810 review: 3WInj is taking into account
		// if( NULL != GetpSchcat() && ( false == GetpSchcat()->IsSecondarySideExist() || GetpSchcat()->Get3WType() != CDB_CircSchemeCateg::e3wTypeMixDecByp ) )
		// {
			ComputeHM( eComputeHMEvent::eceResize );
		// }

		// Now we can unlock
		if( true == bResizeForPendingCircuit )
		{
			for( std::map<CDS_HydroMod *, unsigned int>::iterator iter = mapLockState.begin(); iter != mapLockState.end(); ++iter )
			{
				iter->first->SetLock( iter->second );
			}

			mapLockState.clear();
		}
	}
	else
	{
		// TODO Warn the user
		// Invalidate Balancing device.
		if( NULL != GetpBv() )
		{
			GetpBv()->InvalidateSelection();
		}

		if( NULL != GetpBypBv() )
		{
			GetpBypBv()->InvalidateSelection();
		}

		if( NULL != GetpSecBv() )
		{
			GetpSecBv()->InvalidateSelection();
		}

		if( NULL != GetpDpC() )
		{
			GetpDpC()->InvalidateSelection();
		}

		if( NULL != GetpCV() )
		{
			GetpCV()->InvalidateSelection();
		}

		if( NULL != GetpShutoffValve( eHMObj::eShutoffValveSupply ) )
		{
			GetpShutoffValve( eHMObj::eShutoffValveSupply )->InvalidateSelection();
		}

		if( NULL != GetpShutoffValve( eHMObj::eShutoffValveReturn ) )
		{
			GetpShutoffValve( eHMObj::eShutoffValveReturn )->InvalidateSelection();
		}

		if( NULL != GetpSmartControlValve() )
		{
			GetpSmartControlValve()->InvalidateSelection();
		}

		if( NULL != GetpSmartDpC() )
		{
			GetpSmartDpC()->InvalidateSelection();
		}

		// Empty Module erase Qtot et Dp for Terminal unit.
		GetpTermUnit()->SetQ( 0, false );
		GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Dp, 0, false );
	}
}

CDS_HydroMod *CDS_HydroMod::GetpPressIntHM( std::vector<CDS_HydroMod *> *pHMListOntheWay )
{
	CDS_HydroMod *pHM = this;

	// Stop when main table, pump or DpC  or smart differential pressure controller reached.
	// We have to start on the current circuit to proceed DpC on branch
	while( NULL != pHM && pHM != pHM->GetIDPtr().PP )
	{
		// Verify if DpC or smart differential pressure controller stabilizes the branch.
		if( CDS_HydroMod::ePressIntType::pitDpC == pHM->GetPressIntType() )
		{
			break;
		}

		// Stop when we reach secondary of an injection circuit.
		if( pHM != this && pHM->GetpSchcat() != NULL && true == pHM->GetpSchcat()->IsSecondarySideExist() )
		{
			break;
		}

		// Stop when main table reached.
		if( true == pHM->GetIDPtr().PP->IsClass( CLASS( CTableHM ) ) || true == pHM->GetIDPtr().PP->IsClass( CLASS( CTable ) ) )
		{
			break;
		}

		// Take parent.
		pHM = dynamic_cast<CDS_HydroMod *>( pHM->GetIDPtr().PP );

		if( NULL != pHMListOntheWay )
		{
			pHMListOntheWay->push_back( pHM );
		}
	}

	if( NULL == pHM )
	{
		ASSERTA_RETURN( this );
	}

	return pHM;
}

CDS_HydroMod *CDS_HydroMod::GetpDesignTemperatureInterface()
{
	CTable *pclTable = this;
	bool bStop = false;

	// Stop when we reach the main table or an injection circuit (But not 3-way mixing circuit).
	do
	{
		// Take parent.
		pclTable = (CTable *)( pclTable->GetIDPtr().PP );

		if( NULL != pclTable )
		{
			if( true == pclTable->GetIDPtr().MP->IsClass( CLASS( CTableHM ) ) || true == pclTable->GetIDPtr().MP->IsClass( CLASS( CTable ) ) )
			{
				// Stop if we are on the piping table.
				// Why 'pHM->GetIDPtr().MP->IsClass' and not directly 'pHM->IsClass'? Because 'pHM' is always casted as a 'CDS_HydroMod'.
				// But 'MP' pointer is casted on a 'CData' and 'IsClass' will send back the correct result.
				
				// In this case we return the root module and not its parent table.
				pclTable = this;
				bStop = true;
			}
			else if( NULL != dynamic_cast<CDS_HydroMod *>( pclTable ) )
			{
				CDS_HydroMod *pHM = (CDS_HydroMod *)( pclTable );

				// Remark: Here we can't call the "IsDesignTemperatureInterface" method to know if the 'pHM' current hydraulic
				// circuit is a design temperature interface. Because we need to not test in this case when the 'pHM' is the
				// current hydraulic circuit. This allows to know what is the interface for design temperature on an hydraulic
				// circuit that is itself a interface for design temperatures for other child hydraulic circuits.
				
				if( 0 == pHM->GetLevel() )
				{
					// Stop if we are on the root.
					bStop = true;
				}
				else if( pHM != this && pHM->GetpSchcat() != NULL && true == pHM->GetpSchcat()->IsSecondarySideExist() 
						&& CDB_CircSchemeCateg::e3WType::e3wTypeMixing != pHM->GetpSchcat()->Get3WType() )
				{
					// Stop when we reach secondary of an injection circuit (That is not a 3-way mixing circuit).
					bStop = true;
				}
			}
		}
	}while( false == bStop && NULL != pclTable );

	return dynamic_cast<CDS_HydroMod *>( pclTable );
}

bool CDS_HydroMod::IsDesignTemperatureInterface()
{
	bool bIsPressureInterfaceForDesignTemperature = false;

	if( 0 == GetLevel() )
	{
		// Stop if we are on the root.
		bIsPressureInterfaceForDesignTemperature = true;
	}
	else if( GetpSchcat() != NULL && true == GetpSchcat()->IsSecondarySideExist() && CDB_CircSchemeCateg::e3WType::e3wTypeMixing != GetpSchcat()->Get3WType() )
	{
		// Stop when we reach secondary of an injection circuit (That is not a 3-way mixing circuit).
		bIsPressureInterfaceForDesignTemperature = true;
	}

	return bIsPressureInterfaceForDesignTemperature;
}

CDS_HydroMod *CDS_HydroMod::GetpRootHM()
{
	CDS_HydroMod *pHM = this;

	while( NULL != pHM && pHM != pHM->GetIDPtr().PP )
	{
		// Stop when main table reached.
		if( true == pHM->GetIDPtr().PP->IsClass( CLASS( CTableHM ) ) || true == pHM->GetIDPtr().PP->IsClass( CLASS( CTable ) ) )
		{
			break;
		}

		// Take parent.
		pHM = dynamic_cast<CDS_HydroMod *>( pHM->GetIDPtr().PP );
	}

	if( NULL == pHM )
	{
		ASSERT( 0 );
		return this;
	}

	return pHM;
}

void CDS_HydroMod::SetDiversityFactor( double dDiversityFactor )
{
	if( dDiversityFactor > 0.0 && dDiversityFactor <= 1.0 )
	{
		m_dDiversityFactor = dDiversityFactor;
	}
	else
	{
		ResetDiversityFactor();
	}
}

void CDS_HydroMod::ResetDiversityFactor( void )
{
	m_dDiversityFactor = -1;
}

void CDS_HydroMod::CleanAllDiversityInfos( CDS_HydroMod *pHM )
{
	if( NULL == pHM )
	{
		return;
	}

	for( IDPTR IDPtr = pHM->GetFirst(); IDPtr.MP != NULL; IDPtr = pHM->GetNext( IDPtr.MP ) )
	{
		CDS_HydroMod *pCHM = static_cast< CDS_HydroMod * >( IDPtr.MP );

		if( pCHM != NULL )
		{
			CleanAllDiversityInfos( pCHM );
		}
	}

	pHM->ResetDiversityFactor();

	if( pHM->GetpDistrSupplyPipe() != NULL )
	{
		pHM->GetpDistrSupplyPipe()->SetDiversityAppliedFlag( false );
	}
	else if( ReturnType::Reverse == pHM->GetReturnType() && pHM->GetpDistrReturnPipe() != NULL )
	{
		pHM->GetpDistrReturnPipe()->SetDiversityAppliedFlag( false );
	}
}

void CDS_HydroMod::SaveDiversityValvesPipes( CDS_HydroMod *pHM )
{
	if( true == pHM->IsaModule() )
	{
		for( IDPTR idptr = pHM->GetFirst(); NULL != idptr.MP; idptr = pHM->GetNext( idptr.MP ) )
		{
			CDS_HydroMod *pCHM = dynamic_cast<CDS_HydroMod *>( idptr.MP );

			if( NULL != pCHM )
			{
				SaveDiversityValvesPipes( pCHM );
			}
		}
	}

	if( NULL != pHM->GetpBv() )
	{
		pHM->GetpBv()->SetDiversityIDPtr( pHM->GetpBv()->GetIDPtr() );
	}

	if( NULL != pHM->GetpBypBv() )
	{
		pHM->GetpBypBv()->SetDiversityIDPtr( pHM->GetpBypBv()->GetIDPtr() );
	}

	if( NULL != pHM->GetpSecBv() )
	{
		pHM->GetpSecBv()->SetDiversityIDPtr( pHM->GetpSecBv()->GetIDPtr() );
	}

	if( NULL != pHM->GetpDpC() )
	{
		pHM->GetpDpC()->SetDiversityIDPtr( pHM->GetpDpC()->GetIDPtr() );
	}

	if( NULL != pHM->GetpCV() )
	{
		pHM->GetpCV()->SetDiversityIDPtr( pHM->GetpCV()->GetCvIDPtr() );
	}

	if( NULL != pHM->GetpShutoffValve( eShutoffValveSupply ) )
	{
		pHM->GetpShutoffValve( eShutoffValveSupply )->SetDiversityIDPtr( pHM->GetpShutoffValve( eShutoffValveSupply )->GetIDPtr() );
	}

	if( NULL != pHM->GetpShutoffValve( eShutoffValveReturn ) )
	{
		pHM->GetpShutoffValve( eShutoffValveReturn )->SetDiversityIDPtr( pHM->GetpShutoffValve( eShutoffValveReturn )->GetIDPtr() );
	}

	if( NULL != pHM->GetpSmartControlValve() )
	{
		pHM->GetpSmartControlValve()->SetDiversityIDPtr( pHM->GetpSmartControlValve()->GetIDPtr() );
	}

	if( NULL != pHM->GetpSmartDpC() )
	{
		pHM->GetpSmartDpC()->SetDiversityIDPtr( pHM->GetpSmartDpC()->GetIDPtr() );
	}

	if( NULL != pHM->GetpDistrSupplyPipe() )
	{
		pHM->GetpDistrSupplyPipe()->SetDiversityIDPtr( pHM->GetpDistrSupplyPipe()->GetIDPtr() );
	}

	if( ReturnType::Reverse == pHM->GetReturnType() && NULL != pHM->GetpDistrReturnPipe() )
	{
		pHM->GetpDistrReturnPipe()->SetDiversityIDPtr( pHM->GetpDistrReturnPipe()->GetIDPtr() );
	}
}

void CDS_HydroMod::ComputeAll( bool bResize )
{
	CTableHM *pPipingTab = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );

	if( NULL == pPipingTab )
	{
		ASSERT_RETURN;
	}

	m_mapErrorMessages.clear();
	CDS_HydroMod *pRootHM = GetpRootHM();

	// Establish the list of modules containing a short cut bypass (as a Pump or Injection circuit).
	pPipingTab->GetpPressureInterfaceMap()->clear();
	pPipingTab->FillPressureInterfaceMap( pRootHM );

	// HYS-1777: Clear 3way mixing circuit vector
	pPipingTab->GetpVecHMToReviewChildrenAuthority()->clear();
	// Root module is added to this map.
	pPipingTab->GetpPressureInterfaceMap()->insert( make_pair( INT_MIN, pRootHM ) );

	if( false == IsHMCalcMode() )
	{
		return;
	}

	// Scan all pressure interfaces and update flag constant flow.
	for( auto &iter : *pPipingTab->GetpPressureInterfaceMap() )
	{
		CDS_HydroMod *pHM = iter.second;

		// Update m_bConstantFlow variable for the branch.
		bool bConstantFlow = pPipingTab->CheckIsConstantFlow( pHM );
		pHM->SetFlagConstantFlow( bConstantFlow );
	}

	// In case the current HydroMod is a Hub, do not take into account the freeze function.
	CDS_HmHubStation *pHmHubtStation = dynamic_cast<CDS_HmHubStation *>( this );
	CDS_HmHub *pHmHub = dynamic_cast<CDS_HmHub *>( this );

	if( NULL == pHmHubtStation && NULL == pHmHub )
	{
		if( true == GetpPrjParam()->IsFreezed() )
		{
			return;
		}
	}

	if( true == pRootHM->m_bComputeAllInCourse )
	{
		return;
	}
	else
	{
		pRootHM->m_bComputeAllInCourse = true;
	}

	// Root module is always Index!
	pRootHM->SetFlagCircuitIndex( true );

	// Interpret the root level to have the same pipes as it is in the first level with the first module.
	CDS_HydroMod *pcHM = NULL;

	#ifdef TEST_AVAILABLE_H_DURING_BOTH_PHASES
	testMap.RemoveAll();
	#endif

	// Proceed in two steps:
	//  1 - Force diversity acting to false, compute all the installation.
	//  2 - If diversity have been detected during previous phase, save selected valve
	//	  - relaunch computing with diversity acting set to true;
	pPipingTab->SetDiversityActive( false );

	for( int LoopOnDiversity = 0; LoopOnDiversity < 2; LoopOnDiversity++ )
	{
		if( 0 != LoopOnDiversity )
		{
			// If no diversity exist cancel the loop.
			if( false == pPipingTab->IsDiversityExist() )
			{
				break;
			}

			// Activate diversity and relaunch compute.
			pPipingTab->SetDiversityActive( true );
		}

		// Reset diversity factor flag.
		pPipingTab->SetDiversityExist( false );

		_ComputeAllPressureInterface( pPipingTab, bResize, ( 0 != LoopOnDiversity ), false );
	}

	// Post processing operation
	// Pending circuits;
	_ComputeAllPressureInterface( pPipingTab, false, false, true );

	// Actuator sizing;
	// Resize actuator, taking into account available head
	for( auto &iter : *pPipingTab->GetpPressureInterfaceMap() )
	{
		std::multimap<int, CDS_HydroMod * >CVList;
		std::multimap<int, CDS_HydroMod * >::iterator ItCVList;
		CDS_HydroMod *pHM = iter.second;

		// Store in ModCVList all circuit with CV.
		// HYS-1868: call this function from the main module.
		pHM->FillVectorWithModulatingCV( pHM, &CVList, true );

		for( ItCVList = CVList.begin(); ItCVList != CVList.end(); ++ItCVList )
		{
			CDS_HydroMod *pCVHM = dynamic_cast<CDS_HydroMod *>( ItCVList->second );
			
			// HYS-485 : We should not select an actuator if the CV is locked.
			if( NULL != pCVHM->GetpCV() && true == pCVHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) 
					&& 0 != pCVHM->GetpCV()->GetCvIDPtr().ID )
			{
				continue;
			}

			pCVHM->GetpCV()->SelectActuator();
		}
	}

	pRootHM->m_bComputeAllInCourse = false;

	// When CheckAllModules is set to true Calls comes from ComputeAllInstallation.
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == pPrjParam->GetHNAutomaticCheck() || true == TASApp.GetpTADS()->GetCheckAllModules() )
	{
		CDlgHMCompilationOutput::CHMInterface clOutputInterface;

		if( false == TASApp.GetpTADS()->GetCheckAllModules() )
		{
			clOutputInterface.ClearOutput();
			clOutputInterface.BlockOutput();
		}

		clOutputInterface.FillAndSendMessage( IDS_INFOMSG_HM_ANALYZESTARTED, CDlgHMCompilationOutput::Comment, NULL, L"", pRootHM->GetHMName() );
		TASApp.GetpTADS()->VerifyInstallation( pRootHM );
		clOutputInterface.FillAndSendMessage( IDS_INFOMSG_HM_ANALYZEFINISHED, CDlgHMCompilationOutput::Comment, NULL, L"", pRootHM->GetHMName() );

		if( false == TASApp.GetpTADS()->GetCheckAllModules() )
		{
			clOutputInterface.ReleaseOutput();
		}
	}
}

void CDS_HydroMod::_ComputeAllToThePump( bool bResize, bool bResizeForPendingCircuit, CDS_HydroMod *pOrgHM )
{
	CDS_HydroMod *pHM = this;

	// Open balancing devices at 3 kPa.
	pHM->ResizeBalDevForH( 0.0, bResizeForPendingCircuit, CAnchorPt::CircuitSide::CircuitSide_Secondary );

	// Scan all children, go deeper into HM struct.
	for( IDPTR IDPtr = pHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pHM->GetNext( IDPtr.MP ) )
	{
		CDS_HydroMod *pcHM = static_cast<CDS_HydroMod *>( IDPtr.MP );

		// Compute children, only when current module is not a module with a bypass.
		if( true == pcHM->IsaModule() )
		{
			// It's a module, go deeper.
			if( false == pcHM->IsPressureInterfaceExist() )
			{
				pcHM->_ComputeAllToThePump( bResize, bResizeForPendingCircuit, pOrgHM );
			}
		}
		else if( NULL != pcHM->GetpSchcat() && true == pcHM->GetpSchcat()->IsSecondarySideExist() )
		{
			// HYS-1707: Terminal unit with secondary side Bv (deleted) are not updated in the unfreeze process. 
			// Temperature for secondary in this case is now considered.
			pcHM->ResizeBalDevForH( 0.0, bResizeForPendingCircuit, CAnchorPt::CircuitSide::CircuitSide_Secondary );
		}
		
		// By default when going to the pump reset HAvail.
		pcHM->m_dHAvail = 0.0;

		// Open balancing devices at 3 kPa.
		pcHM->ResizeBalDevForH( 0.0, bResizeForPendingCircuit, CAnchorPt::CircuitSide::CircuitSide_Primary );
	}

	// Pressure interface is a module...
	if( true == pHM->IsaModule() )
	{
		// When pHM is a child module of original hydromod w/o shortcut bypass.
		if( false == pHM->IsPressureInterfaceExist() || pHM == pOrgHM )
		{
			// Refresh flow in pipes and Update Q for virtual TermUnit.
			// Resize balancing device for 3Kpa.
			pHM->_UpdateQDp( bResize, bResizeForPendingCircuit, false );
		}
	}
	else
	{
		// Pressure interface with a terminal unit.
		if( NULL != GetpDpC() )
		{
			GetpDpC()->SelectBestDpC();
		}
		else if( NULL != GetpSmartDpC() )
		{
			GetpSmartDpC()->SelectBest();
		}

		// If CV exist, resize it... Done inside _UpdateQDp() but what about if we don't pass trough 2w in root module for instance?
		if( NULL != GetpCV() )
		{
			if( 0.0 == pHM->GetpCV()->GetQ() )
			{
				return;
			}

			pHM->GetpCV()->SizeControlValveForBetaMin( 0.0 );
		}
	}
}

void CDS_HydroMod::_ComputeAllPressureInterface( CTableHM *pPipingTab, bool bResize, bool bForDiversity, bool bForPending )
{
	// Sanity, when we are processing pending phase, we will not resize circuit or proceed diversity, these was done previously
	if( true == bForPending )
	{
		bResize = false;
		bForDiversity = false;
	}

	CDS_HydroMod *pRootHM = GetpRootHM();

	// Compute each module with a short cut bypass, start from the deeper and finish with the root module.
	std::multimap<int, CDS_HydroMod *>::reverse_iterator rit = pPipingTab->GetpPressureInterfaceMap()->rbegin();

	// HYS-1777: Get the pointer on 3-way mixing circuit vector.
	std::vector<CDS_HydroMod*>* pvecHMToReviewChildrenAuthority = pPipingTab->GetpVecHMToReviewChildrenAuthority();
	std::vector<CDS_HydroMod*>::iterator it3wayMix;

	bool bStop = false;

	// HYS-1777: This loop contains 2 iterations: The first concerns PressureInterfaceMap with rit. The second concerns the
	// 3-way mixing circuit vector with it3wayMix.
	// The second iteration when i == 1 is for 3-way mixing circuit modules only, we go through the vector and for each
	// pHM (as 3-way mixing circuit module) we perform the first loop algorithm from _AdjustPumpHminForModulatingCV
	// until the end of the loop. This allows to review authority of all CV on the 3-way mixing children with the good Hmin pump.
	// NOTE: The 3-way mixing circuit modules are examined in both the first iteration and the second iteration.
	for( int i = 0; i < 2; i++ )
	{
		if( 1 == i && pvecHMToReviewChildrenAuthority->size() > 0 )
		{
			// HYS-1777: We reset bStop to go on 3-way mixing vector.
			bStop = false;
			it3wayMix = pvecHMToReviewChildrenAuthority->begin();
		}
		
		while( false == bStop )
		{
			CDS_HydroMod *pHM = NULL;

			if( 0 == i )
			{
				// For Pressure interface map
				pHM = rit->second;
				++rit;

				if( rit == pPipingTab->GetpPressureInterfaceMap()->rend() )
				{
					bStop = true;
				}
			}
			else
			{
				// For 3-way mixing circuit vector
				pHM = *it3wayMix;
				it3wayMix++;

				if( it3wayMix == pvecHMToReviewChildrenAuthority->end() )
				{
					bStop = true;
				}
			}

			// HYS-1777: This part is only for the iteration with pressure interface map.
			if( 0 == i )
			{
				// When we are computing second loop on diversity first save computed valves in previous stage.
				if( true == bForDiversity )
				{
					SaveDiversityValvesPipes( pHM );
				}

				if( true == bForPending )
				{
					// If pending circuit doesn't exist .....
					if( pHM->GetpTermUnit()->GetPartialPendingQ() >= pHM->GetpTermUnit()->GetQ() )
					{
						continue;
					}
				}
				else
				{
					// Reset Pump head
					if( pRootHM == pHM )
					{
						// Third parameter 'true' to specify that we are going to the pump.
						pHM->SetHavail( 0.0, CAnchorPt::CircuitSide_Primary, true );
					}

					pHM->ResetHavailRecursively();
				}

				// Open balancing devices at 3 kPa
				if( pRootHM == pHM )
				{
					pHM->ResizeBalDevForH( 0.0, bForPending, CAnchorPt::CircuitSide::CircuitSide_Primary );
				}

				// Start from deeper circuit and establish Hmin and qtot requested at the pump.
				pHM->_ComputeAllToThePump( bResize, bForPending, pHM );

				// Update H available for the pressure interface; needed for circuit with a cv into root module.
				_UpdateHAvailAccordingTUDp( pHM );
			}

			bool bChildrenWithCV = false;

			// HYS-1930: is it possible now to bypass this step for debug purpose.
			if( false == pMainFrame->IsHMCalcDebugPanelVisible() || true == pMainFrame->IsHMCalcDebugVerifyAutority() )
			{
				// For each modulating control valve, verify/increase pump head to satisfy minimum authority.
				// This method returns "false" if there is no children with a control valve.
				bChildrenWithCV = pHM->_AdjustPumpHminForModulatingCV();

				if( true == bChildrenWithCV )
				{
					// All virtual module TU must be updated for taking into account increasing of requested CV Dp.

					// If 'pHM' is at the root and the CV is at the primary side, we don't need to call the "_AdjustDpAfterCVResizing" method.
					// if( 0 != pHM->GetLevel() || ( NULL != pHM->IsCvExist( true ) && eb3False == pHM->GetpCV()->IsCVLocInPrimary() ) )
					// {
						pHM->_AdjustDpAfterCVResizing( pHM );
					// }
				}
			}

			if( false == bChildrenWithCV && 1 == i )
			{
				// HYS-1777: if we go through the 3-way mixing circuit vector we do nothing if the 3-way has no child with a CV. 
				continue;
			}

			// Update H available for the pressure interface; needed for circuit with a cv into root module.
			// HYS-1930: If we are in the second loop to verify the authority, we don't call this method if we are with
			// the auto-adaptive flow with decoupling bypass circuit. Because we don't want anymore that the primary side be changed.
			if( 0 == i || eDpStab::DpStabOnBVBypass != pHM->GetpSch()->GetDpStab() )
			{
				_UpdateHAvailAccordingTUDp( pHM );
			}

			// Before distributing Havailable we have to update virtual terminal unit Dp of pressure interface
			if( true == pHM->IsaModule() )
			{
				// HYS-1930: If we are in the second loop to verify the authority, we don't call this method if we are with
				// the auto-adaptive flow with decoupling bypass circuit. Because we don't want anymore that the primary side be changed.
				if( 0 == i || eDpStab::DpStabOnBVBypass != pHM->GetpSch()->GetDpStab() )
				{
					pHM->_UpdateTUDpAccordingHavailable();
				}
			}

			// Resize
			if( pHM == pRootHM )
			{
				// If the pressure interface is the root module, we must resize balancing device in the primary side.
				// Because there will be no more PI at higher level to manage it.
				
				// HYS-1930: If we are in the second loop to verify the authority, we don't call this method if we are with
				// the auto-adaptive flow with decoupling bypass circuit. Because we don't want anymore that the primary side be changed.
				if( 0 == i || eDpStab::DpStabOnBVBypass != pHM->GetpSch()->GetDpStab() )
				{
					pHM->ResizeBalDevForH( max( 0.0, pHM->GetHAvail( CAnchorPt::CircuitSide_Primary ) ), bForPending, CAnchorPt::CircuitSide::CircuitSide_Primary );
				}
			}

			if( NULL != pHM->GetpSchcat() && true == pHM->GetpSchcat()->IsSecondarySideExist() && pHM->GetpSchcat()->Get3WType() != CDB_CircSchemeCateg::e3wTypeMixing )
			{
				// HYS-1930: If we are in the second loop to verify the authority, we don't call this method if we are with
				// the auto-adaptive flow with decoupling bypass circuit. Because we want to manage seconrady side in another way.
				if( 0 == i || eDpStab::DpStabOnBVBypass != pHM->GetpSch()->GetDpStab() )
				{
					pHM->ResizeBalDevForH( max( 0.0, pHM->GetHAvail( CAnchorPt::CircuitSide_Secondary ) ), bForPending, CAnchorPt::CircuitSide::CircuitSide_Secondary );
				}
				else
				{
					// HYS-1930: auto-adapting variable flow decoupling circuit.
					pHM->ResizePumpSecFollowingCVAuthorityChanges( pHM );
				}
			}

			double dDpTU = pHM->GetpTermUnit()->GetDp();
			pHM->_DistributeHAvailable( pHM, dDpTU, bForPending );

			if( pHM == pRootHM && eDpStab::DpStabOnBVBypass == pHM->GetpSch()->GetDpStab() )
			{
				// HYS-1930: auto-adapting variable flow decoupling circuit.
				// We verify pump head input by user only when all have been computed. Than when H available at the entry of
				// the circuit is fixed. This is why it is done in the "_DistributeHAvailable" method.
				// But for a root module, this method is never called. This is why we need here to manually call
				// the "_VerifyPumpHeadUser" method.
				pHM->_VerifyPumpHeadUser( pHM );
			}
			
			if( 1 == i )
			{
				pvecHMToReviewChildrenAuthority->clear();
			}
		}
	}
}

void CDS_HydroMod::FillVectorWithModulatingCV( CDS_HydroMod *pHM, std::multimap<int, CDS_HydroMod *> *pMMap, bool bForActuatorSelection /*= false */ )
{
	// Recursive function, caller (this) is the MainHM
	// We consider only CV located in primary side in case of children and on secondary side, if exist on the pressure interface.
	// This method is called starting from a pressure interface.
	if( NULL == pHM || NULL == pHM->GetpSchcat() )
	{
		ASSERT_RETURN;
	}

	// HYS-1757: Note about the 3-way mixing circuits.
	// What are the cases where it is not needed to test authority of the control valve ?
	// 
	// If 'pHM' IS NOT the pressure interface:
	// 
	//		1a) 3-way mixing with only a BV: this circuit is auto-managed (See 'CDS_Hm3WInj::ComputeHM' and 'CDS_HydroMod::SetHavail'). No need to verify authority.
	//		2a) 3-way mixing with a DpC on branch: same case of 1).
	//		3a) 3-way mixing with decoupling bypass, CV at the SECONDARY side: not concerned in this case because the CV is at the SECONDARY side.
	//		4a) 3-way mixing with decoupling bypass, CV at the PRIMARY side, primary CONSTANT flow: authority is always close 1. No need to verify.
	//		5a) 3-way mixing with decoupling bypass, CV at the PRIMARY side, primary VARIABLE flow: the auhority must be calculated!
	//		6a) 3-way mixing with decoupling bypass, CV at the SECONDARY side, DpC on branch: not concerned in this case because the CV is at the SECONDARY side.
	//		7a) 3-way mixing with decoupling bypass, CV at the PRIMARY side, DpC on branch, primary CONSTANT flow: this circuit is auto-managed. No need to verify authority.
	//		8a) 3-way mixing with decoupling bypass, CV at the PRIMARY side, DpC on branch, primary VARIABLE flow: this circuit is auto-managed. No need to verify authority.
	//
	// If 'pHM' IS the pressure interface:
	// 
	//		1b) 3-way mixing with only a BV: not concerned in this case because the CV is at the PRIMARY side.
	//		2b) 3-way mixing with a DpC on branch: not concerned in this case because the CV is at the PRIMARY side.
	//		3b) 3-way mixing with decoupling bypass, CV at the SECONDARY side: authority is always close to 1. No need to verify.
	//		4b) 3-way mixing with decoupling bypass, CV at the PRIMARY side, primary CONSTANT flow: not concerned in this case because the CV is at the PRIMARY side.
	//		5b) 3-way mixing with decoupling bypass, CV at the PRIMARY side, primary VARIABLE flow: not concerned in this case because the CV is at the PRIMARY side.
	//		6b) 3-way mixing with decoupling bypass, CV at the SECONDARY side, DpC on branch: authority is always close to 1. No need to verify.
	//		7b) 3-way mixing with decoupling bypass, CV at the PRIMARY side, DpC on branch, primary CONSTANT flow: not concerned in this case because the CV is at the PRIMARY side.
	//		8b) 3-way mixing with decoupling bypass, CV at the PRIMARY side, DpC on branch, primary VARIABLE flow: not concerned in this case because the CV is at the PRIMARY side.
	
	bool bAddIt = false;

	// (this != pHM) we are working with children, CV must be located on the primary side.
	// (this == pHM) we are working with main HM, if the secondary side exists, CV must be located on the secondary side.
	bool bTest = pHM->IsCvExist( true );

	if( this != pHM )
	{
		// It's a child, we consider CV on the primary side.
		bTest = bTest && ( eb3True == pHM->GetpCV()->IsCVLocInPrimary() );

		// Do a verification for the 3-way mixing circuit with decoupling bypass.
		if( true == bTest )
		{
			if( CDB_CircSchemeCateg::e3WType::e3wTypeMixing == pHM->GetpSchcat()->Get3WType() )
			{
				// Case 1a) & 2a): these circuits are auto-managed. No need to verify authority.
				bTest = false;
			}
			else if( CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == pHM->GetpSchcat()->Get3WType() )
			{
				if( CDB_CircuitScheme::eDpCType::eDpCTypeDPC == pHM->GetpSch()->GetDpCType() )
				{
					// Case 7a) & 8a): these circuits are auto-managed. No need to verify authority.
					bTest = false;
				}
				else
				{
					// An additional information has been given by Bjarne Andreasen in his webinar on hydronic control circuit - Part 3.
					// (See power point page 8). If control valve is at the primary side and the flow distribution is variable the 
					// authority must be calculated.

					if( true == pHM->GetpPressIntHM()->IsConstantFlow() )
					{
						// Case 4a).
						bTest = false;
					}
				}
			}
		}
	}
	else
	{
		// It's the module, we consider CV on the secondary side.
		// 
		// HYS-1754: In reality, if we are on the pressure interface, effectively we consider the control valve only
		// if this one is at the secondary side. Because if the control valve is at the primary side, it's the parent pressure interface
		// of the current one that will manage it. EXCEPT when we are on the root module (Level 0). No one will manage control valve
		// that is at the primary side. So effectively, we have two cases:
		//	1) We are not on the root module (Level > 0) -> we accept ONLY module with CV at the secondary side. There is only one 
		//     module with a CV at the secondary side: 3-way mixing with decoupling bypass. And, as described above, for this case the 
		//     authority is always near 1. And then, for the case 1, we don't add circuit.
		//  2) We are on the root moduel (Level = 0) -> we accept ALL module with CV at the primary side.

		if( pHM->GetLevel() > 0 )
		{
			// Case 1).
			bTest = false;
		}
		else
		{
			// Case 2).
			bTest = bTest && ( eb3True == pHM->GetpCV()->IsCVLocInPrimary() );
		}

		/*
		if( true == pHM->GetpSchcat()->IsSecondarySideExist() )
		{
			// If a secondary side, we take the control valve only if this one is located at the primary side.
			bTest = bTest && ( eb3False == pHM->GetpCV()->IsCVLocInPrimary() );

			if( true == bTest && eb3False == pHM->GetpCV()->IsCVLocInPrimary() )
			{
				if( CDB_CircSchemeCateg::e3wTypeMixDecByp == pHM->GetpSchcat()->Get3WType() )
				{
					// Case 3b) & 6b).
					bTest = false;
				}
			}
		}
		else
		{
			// Main HM w/o secondary side CV must be on the primary side.
			bTest = bTest && ( eb3True == pHM->GetpCV()->IsCVLocInPrimary() );
		}
		*/
	}

	if( true == bTest )
	{
		// When we are not working for actuator selection, only modulating CV will be added in the list.
		bAddIt = ( false == pHM->GetpCV()->IsOn_Off() );

		if( true == bForActuatorSelection )
		{
			CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( pHM->GetpCV()->GetCvIDPtr().MP );

			if( NULL != pCV )
			{
				bAddIt = ( NULL != pCV->GetActuatorGroupIDPtr().MP ) ? true : false;
			}
		}
		else
		{
			if( true == pHM->GetpSchcat()->Is3W() && CDB_CircSchemeCateg::e3wTypeMixing == pHM->GetpSchcat()->Get3WType() )
			{
				// In 3W mixing CV is sized according HAvail
				bAddIt = false;
			}
		}
	}

	if( true == bAddIt )
	{
		int iPos = -1 * ( pHM->GetLevel() * 10000 + pHM->GetPos() + pHM->IsCircuitIndex() * 1000000 );
		pMMap->insert( std::pair<int, CDS_HydroMod *>( iPos, pHM ) );
	}

	// Exploring children.
	// Added only when we are not on a injection circuit; secondary of injection circuits will be added later.
	// If we are inspecting MainHM (first call of recursion) we will enter in this module
	if( true == pHM->IsaModule() && ( false == pHM->IsPressureInterfaceExist() || this == pHM ) )
	{
		for( IDPTR IDPtr = pHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pHM->GetNext( IDPtr.MP ) )
		{
			CDS_HydroMod *pcHM = static_cast<CDS_HydroMod *>( IDPtr.MP );
			FillVectorWithModulatingCV( pcHM, pMMap, bForActuatorSelection );
		}
	}
}

// Return false when pump head increase, request a new loop.
bool CDS_HydroMod::_ResizeCv()
{
	// This method is only called by the '_AdjustPumpHminForModulatingCV' when we need to verify for each circuit with a
	// control valve that the authority is good.
	//
	// Remember: When we are working in the design condition, all the circuits around the current one are working in a normal
	// way. So, we have a differential pressure available at the entry of the circuit that is almost the one computed when designing.
	// The design authority is given by DpCV/(DpCV+Ha) and must be greater or equal to 0.25.
	
	// Now we have the worst case that is when all the circuits around the current one for example are shut down. So, at the entry
	// of the current circuit, we can have almost all the root pump head (We have yet pressure drop in pipes and singularities).
	// The minimum authority is given by DpCV/(DpCv+PumpH) and must be greater or equal to 0.25.

	if( false == IsCvExist( true ) )
	{
		return true;
	}

	if( 0.0 == GetpCV()->GetQ() )
	{
		return true;
	}

	// On-Off valves are not resized for minimum authority.
	if( true == GetpCV()->IsOn_Off() )
	{
		return true;
	}

	// Skip pressure independent control valve and combined Dp controller, control and balancing valve (ie: TA-COMPACT-DP).
	if( eb3True == GetpCV()->IsPICV() || eb3True == GetpCV()->IsDpCBCV() )
	{
		return true;
	}

	// If CV is resized there is a modification of HMin, 'CheckAndResizeCV' must be relaunched from the pump.
	// Don't abort the loop but continue to resize with the new Hmin.
	std::vector<CDS_HydroMod *> HMListOntheWay;
	CDS_HydroMod *pPIHM = NULL;

	if( eb3True == GetpCV()->IsCVLocInPrimary() )
	{
		pPIHM = GetpPressIntHM( &HMListOntheWay );
	}
	else
	{
		pPIHM = this;
	}

	double dHPressInt = 0.0;

	// If we are on the pressure interface.
	if( this == pPIHM )
	{
		if( pPIHM->GetpSchcat() != NULL && true == pPIHM->GetpSchcat()->IsSecondarySideExist() )
		{
			if( eb3True == GetpCV()->IsCVLocInPrimary() )
			{
				if( CDS_HydroMod::ePressIntType::pitDpC == pPIHM->GetPressIntType() )
				{
					dHPressInt = max( pPIHM->GetpDpC()->GetDpToStab(), pPIHM->GetpDpC()->GetDplmin() );
				}
				else
				{
					// Injection circuit not stabilized by DpC.
					dHPressInt = pPIHM->GetHAvail( CAnchorPt::CircuitSide_Primary, false );
				}
			}
			else
			{
				// HYS-1757: In case of 3-way mixing circuit with decoupling bypass and the control valve at the secondary
				// side, whatever if secondary side has a constant or a variable flow, the authority is always near 1.
				if( CDB_ControlProperties::CV2W3W::CV3W == pPIHM->GetpSchcat()->Get2W3W() && CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == pPIHM->GetpSchcat()->Get3WType() )
				{
					dHPressInt = 0.0;
				}
				else
				{
					dHPressInt = pPIHM->GetHAvail( CAnchorPt::CircuitSide_Secondary, false );
				}
			}
		}
		else
		{
			if( CDS_HydroMod::ePressIntType::pitDpC == pPIHM->GetPressIntType() )
			{
				dHPressInt = max( pPIHM->GetpDpC()->GetDpToStab(), pPIHM->GetpDpC()->GetDplmin() );
			}
			else
			{
				dHPressInt = pPIHM->GetHAvail( CAnchorPt::CircuitSide_Primary, false );
			}
		}
	}
	else
	{
		dHPressInt = pPIHM->GetHAvail( CAnchorPt::CircuitSide_Secondary, false );
	}

	double dHPressIntSave = dHPressInt;

	// Size control valve will return H needed at pressure interface to satisfy minimum authority
	double dNewHAtPressureInt = GetpCV()->SizeControlValveForBetaMin( dHPressInt );

	// TODO to clarify in some circumstance dHmin greater but very close to GetPressIntH (delta == 0.0000000000004)
	// in this situation we have a dead lock. workaround test only 5 first decimals.
	// How to reproduce it: *A = BV, *A.1 = 2w BV+prop CV defined by the Kvs value, q=3600l/h Dp = 10kPa
	// Kvs max = 11.4, Kvs = 10; change manually Kvs to 11.4 and unlock
	double dTest = floor( ( dNewHAtPressureInt - dHPressIntSave ) /* 10000/**/ );

	if( dTest <= 100.0 )
	{
		return true;
	}

	// At this point we will request a pump head increase --> new loop

	// Control valve is located on the pressure interface itself,
	// We are on the root module, we have to update Virtual root pump.
	if( pPIHM == this )
	{
		// For non-injection circuit, if DpC exist update it.
		// HYS-1369 : Look if GetpDpC() exists.
		if( NULL != pPIHM->GetpDpC() &&  NULL != pPIHM->GetpSchcat() && false == pPIHM->GetpSchcat()->IsSecondarySideExist() 
				&& CDS_HydroMod::ePressIntType::pitDpC == pPIHM->GetPressIntType() && NULL != pPIHM->GetpSch() && DpStabOnBranch == pPIHM->GetpSch()->GetDpStab() )
		{
			pPIHM->_UpdateDPCDp( this, dHPressIntSave, dNewHAtPressureInt );
		}
		else
		{
			// Depending CV localization, we have to update primary or secondary side.
			pPIHM->SetHavail( dNewHAtPressureInt, ( eb3True == pPIHM->GetpCV()->IsCVLocInPrimary() ) ? CAnchorPt::CircuitSide_Primary : CAnchorPt::CircuitSide_Secondary );
		}
	}
	else
	{
		// For non-injection circuit, if DpC exist update it.
		// HYS-1369 : Look if GetpDpC() exists.
		if( NULL != pPIHM->GetpDpC() && NULL != pPIHM->GetpSchcat() && false == pPIHM->GetpSchcat()->IsSecondarySideExist() 
				&& CDS_HydroMod::ePressIntType::pitDpC == pPIHM->GetPressIntType() && NULL != pPIHM->GetpSch() && DpStabOnBranch == pPIHM->GetpSch()->GetDpStab() )
		{
			pPIHM->_UpdateDPCDp( this, dHPressIntSave, dNewHAtPressureInt );
		}
		else
		{
			// We are on a child circuit, we must update the secondary of PI
			pPIHM->SetHavail( dNewHAtPressureInt, ( NULL != pPIHM->GetpSchcat() && true == pPIHM->GetpSchcat()->IsSecondarySideExist() ) ? CAnchorPt::CircuitSide_Secondary : CAnchorPt::CircuitSide_Primary );
		}
	}

	return false;
}

//"file://C:\\DocTA\\Software\\TAS Project\\TASelect-Sln\\Doc\\2WSizing.pdf"
bool CDS_HydroMod::_AdjustPumpHminForModulatingCV()
{
	// This method is called by the '_ComputeAllPressureInterface' method when we have computed the H needed at the pump.
	// Now, for each pressure interface, we need to verify if all circuits with control valve have a good authority.
	// See the '_ResizeCv' method for more details.

	CDS_HydroMod *pHM = this;
	bool bRestartFromThePumpNeeded = false;
	std::multimap<int, CDS_HydroMod *> ModCVList;

	// Store in ModCVList all circuits with modulating CV.
	pHM->FillVectorWithModulatingCV( pHM, &ModCVList, false );

	if( ModCVList.size() == 0 )
	{
		return false;
	}

	int iCount = 0;
	bool bRestartLoop = true;

	while( true == bRestartLoop || iCount < 2 )
	{
		bRestartLoop = false;

		// Resize each modulating CV, if needed resizing of CV will increase Hmin at pressure interface.
		for( auto rIt = ModCVList.rbegin(); rIt != ModCVList.rend(); ++rIt )
		{
			CDS_HydroMod *pHM = rIt->second;

			// _ResizeCv() return false when pump head increase, meaning a new loop is requested.
			if( false == pHM->_ResizeCv() )
			{
				bRestartLoop = true;
			}
		}

		iCount++;

		if( iCount > 100 )  ///Stop  infinite loop
		{
			ASSERT( 0 );
			break;
		}
	}

	ModCVList.clear();
	return true;
}

bool CDS_HydroMod::SolvePartDef()
{
	CRank rkl( false );

	TASApp.GetpTADB()->GetTAPListFromCBIDef( &rkl, GetCBIType(), GetCBISize() );

	// Products are sorted by Family and version, take the first one.
	bool bSet = false;

	if( rkl.GetCount() > 0 )
	{
		// Save ID of partially defined product.
		if( NULL != GetpBv() || ( NULL != GetpCV() && true == GetpCV()->IsTaCV() ) )
		{
			CString str;
			LPARAM lparam = NULL;
			rkl.GetFirst( str, lparam );

			if( lparam )
			{
				GetpPrjParam()->SetFreeze( true );

				if( ett_Added == GetTreatment() )
				{
					GetpTermUnit()->SetQ( GetQDesign() );
				}

				if( NULL != GetpBv() )
				{
					SetLock( eHMObj::eBVprim, true, false, true );
					GetpBv()->SetQ( GetQ() );
					GetpBv()->ForceBVSelection( ( (CDB_TAProduct *)lparam )->GetIDPtr() );
				}
				else if( NULL != GetpCV() )
				{
					SetLock( eHMObj::eCV, true, false, true );
					GetpCV()->SetQ( GetQ() );
					GetpCV()->ForceCVSelection( ( (CDB_TAProduct *)lparam )->GetIDPtr() );
				}

				bSet = true;
				SetVDescrType( edt_TADBValve );
			}
		}
	}

	rkl.PurgeAll();
	return bSet;
}

bool CDS_HydroMod::IsDTFieldDisplayedInFlowMode()
{
	bool bReturn = false;

	if( NULL == GetpTermUnit() )
	{
		return bReturn;
	}

	if( true == IsaModule() || CTermUnit::_QType::PdT == GetpTermUnit()->GetQType() )
	{
		return bReturn;
	}

	// We look if we have a parent that is a temperature interface
	if( ( NULL != GetpDesignTemperatureInterface() && GetpRootHM() != GetpDesignTemperatureInterface() )
			|| true == GetpDesignTemperatureInterface()->IsInjectionCircuit() )
	{
		// HYS-1930: we add this additional condition. Because for the auto-adapting variable flow decoupling circuit,
		// the supply temperature at the secondary side is the same as the primary one.
		if( eDpStab::DpStabOnBVBypass != GetpDesignTemperatureInterface()->GetpSch()->GetDpStab() )
		{
			// Show DT with Qtype = flow
			bReturn = true;
		}
	}
	else if( true == IsInjectionCircuit() )
	{
		// HYS-1930: we add this additional condition. Because for the auto-adapting variable flow decoupling circuit,
		// the supply temperature at the secondary side is the same as the primary one.
		if( eDpStab::DpStabOnBVBypass != GetpSch()->GetDpStab() )
		{
			bReturn = true;
		}
	}

	return bReturn;
}

void CDS_HydroMod::DropData( OUTSTREAM outf, int *piLevel )
{
	CString strTab = _T("");

	for( int i = *piLevel; i; i-- )
	{
		strTab += _T("\t");
	}

	CString str;
	str.Format( _T("HMName = %s\n"), m_HMName );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("Description = %s\n"), m_strDescription );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("Level = %u\n"), m_iLevel );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("uid = %u\n"), m_usUniqueID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("Pos = %u\n"), m_iPosition );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("bModule = %s\n" ), ( true == m_bModule ) ? _T("true") : _T( "false") );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_DeprecatedSelThingID = %s\n"), m_strDeprecatedSelThingID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("CBIType = %s\n"), m_tcCBIType );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("CBISize = %s\n"), m_tcCBISize );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_qd = %s\n"), WriteCUDouble( _U_FLOW, m_dQDesign, true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("Dpd = %s\n"), WriteCUDouble( _U_DIFFPRESS, m_dDpDesign, true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Havail = %s\n"), WriteCUDouble( _U_DIFFPRESS, m_dHAvail, true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("Presetting = %s\n"), WriteDouble( m_dPresetting, 2, 2 ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dDiversityFactor = %s\n"), WriteDouble( m_dDiversityFactor, 2, 2 ) );
	WriteFormatedStringA2( outf, str, strTab );

	str = _T("VDescrType = ");

	switch( m_eVDescrType )
	{
		case edt_TADBValve:
			str.Append( _T("edt_TADBValve\n") );
			break;

		case edt_CBISizeValve:
			str.Append( _T("edt_CBISizeValve\n") );
			break;

		case edt_CBISizeInchValve:
			str.Append( _T("edt_CBISizeInchValve\n") );
			break;

		case edt_KvCv:
			str.Append( _T("edt_KvCv\n") );
			break;
	}

	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("KvCv = %s\n"), WriteCUDouble( _C_KVCVCOEFF, m_dKvCv ) );
	WriteFormatedStringA2( outf, str, strTab );

	str = _T("Treatment = ");

	switch( m_eTreatment )
	{
		case ett_None:
			str.Append( _T("ett_None\n") );
			break;

		case ett_Delete:
			str.Append( _T("ett_Delete\n") );
			break;

		case ett_Kept:
			str.Append( _T("ett_Kept\n") );
			break;

		case ett_Added:
			str.Append( _T("ett_Added\n") );
			break;

		case ett_Moved:
			str.Append( _T("ett_Moved\n") );
			break;

		case ett_Reinsert:
			str.Append( _T("ett_Reinsert\n") );
			break;

		case ett_ValveModified:
			str.Append( _T("ett_ValveModified\n") );
			break;
	}

	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_SchemeIDPtr.ID = %s\n"), m_SchemeIDPtr.ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("RemL1 = %s\n"), m_strRemL1 );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_bCirIndex = %s\n" ), ( true == m_bIsCircuitIndex ) ? _T("true") : _T( "false") );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_HPressInt = %s\n"), WriteCUDouble( _U_PRESSURE, m_dHPressInt, true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Lock = %u\n"), m_usLock );
	WriteFormatedStringA2( outf, str, strTab );

	if( m_pDistrSupplyPipe != NULL )
	{
		WriteFormatedStringA2( outf, _T("Distribution pipe:\n"), strTab );
		( *piLevel )++;
		m_pDistrSupplyPipe->DropData( outf, piLevel );
		( *piLevel )--;
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no distribution pipe\n"), strTab );
	}

	if( m_pDistrReturnPipe != NULL )
	{
		WriteFormatedStringA2( outf, _T("Distribution returnn pipe:\n"), strTab );
		( *piLevel )++;
		m_pDistrReturnPipe->DropData( outf, piLevel );
		( *piLevel )--;
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no distribution returnn pipe\n"), strTab );
	}

	if( m_pCircuitPrimaryPipe != NULL )
	{
		WriteFormatedStringA2( outf, _T("Circuit pipe:\n"), strTab );
		( *piLevel )++;
		m_pCircuitPrimaryPipe->DropData( outf, piLevel );
		( *piLevel )--;
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no circuit pipe\n"), strTab );
	}

	if( m_pCircuitBypassPipe != NULL )
	{
		WriteFormatedStringA2( outf, _T("Bypass pipe:\n"), strTab );
		( *piLevel )++;
		m_pCircuitBypassPipe->DropData( outf, piLevel );
		( *piLevel )--;
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no bypass pipe\n"), strTab );
	}

	if( m_pCircuitSecondaryPipe != NULL )
	{
		WriteFormatedStringA2( outf, _T("Secondary pipe:\n"), strTab );
		( *piLevel )++;
		m_pCircuitSecondaryPipe->DropData( outf, piLevel );
		( *piLevel )--;
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no secondary pipe\n"), strTab );
	}

	str.Format( _T("m_CBIValveIDPtr.ID = %s\n"), m_CBIValveIDPtr.ID );
	WriteFormatedStringA2( outf, str, strTab );

	if( m_pDpC != NULL )
	{
		WriteFormatedStringA2( outf, _T("Differencial pressure controler:\n"), strTab );
		( *piLevel )++;
		m_pDpC->DropData( outf, piLevel );
		( *piLevel )--;
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no differencial pressure controler\n"), strTab );
	}

	if( m_pCv != NULL )
	{
		WriteFormatedStringA2( outf, _T("Control valve:\n"), strTab );
		( *piLevel )++;
		m_pCv->DropData( outf, piLevel );
		( *piLevel )--;
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no control valve\n"), strTab );
	}

	if( m_pBv != NULL )
	{
		WriteFormatedStringA2( outf, _T("Balancing valve:\n"), strTab );
		( *piLevel )++;
		m_pBv->DropData( outf, piLevel );
		( *piLevel )--;
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no balancing valve\n"), strTab );
	}

	if( m_pBypBv != NULL )
	{
		WriteFormatedStringA2( outf, _T("Bypass balancing valve:\n"), strTab );
		( *piLevel )++;
		m_pBypBv->DropData( outf, piLevel );
		( *piLevel )--;
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no bypass balancing valve\n"), strTab );
	}

	if( m_pSecBv != NULL )
	{
		WriteFormatedStringA2( outf, _T("Balancing valve on secondary:\n"), strTab );
		( *piLevel )++;
		m_pSecBv->DropData( outf, piLevel );
		( *piLevel )--;
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no balancing valve on secondary\n"), strTab );
	}

	if( m_pShutoffValveSupply != NULL )
	{
		WriteFormatedStringA2( outf, _T("Shut-off valve on supply:\n"), strTab );
		( *piLevel )++;
		m_pShutoffValveSupply->DropData( outf, piLevel );
		( *piLevel )--;
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no shut-off valve on supply\n"), strTab );
	}

	if( m_pShutoffValveReturn != NULL )
	{
		WriteFormatedStringA2( outf, _T("Shut-off valve on return:\n"), strTab );
		( *piLevel )++;
		m_pShutoffValveReturn->DropData( outf, piLevel );
		( *piLevel )--;
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no shut-off valve on return\n"), strTab );
	}

	if( m_pTermUnit != NULL )
	{
		WriteFormatedStringA2( outf, _T("Terminal unit:\n"), strTab );
		( *piLevel )++;
		m_pTermUnit->DropData( outf, piLevel );
		( *piLevel )--;
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no terminal unit\n"), strTab );
	}

	if( m_pPump != NULL )
	{
		WriteFormatedStringA2( outf, _T("Pump:\n"), strTab );
		( *piLevel )++;
		m_pPump->DropData( outf, piLevel );
		( *piLevel )--;
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no pump\n"), strTab );
	}

	str.Format( _T("m_bComputeAllInCourse = %s\n" ), ( true == m_bComputeAllInCourse ) ? _T("true") : _T( "false") );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_DN50 = %u\n"), m_iDN50 );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_DN15 = %u\n"), m_iDN15 );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_CurrentMeasDataIndex = %u\n"), m_uCurrentMeasDataIndex );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_ModuleStatus = %u\n"), m_ucModuleStatus );
	WriteFormatedStringA2( outf, str, strTab );

	if( m_MeasDataDateTimeVector.size() > 0 )
	{
		WriteFormatedStringA2( outf, _T("Measuring data:\n"), strTab );

		for( int iLoop = 0; iLoop < ( int ) m_MeasDataDateTimeVector.size(); iLoop++ )
		{
			( *piLevel )++;
			m_MeasDataDateTimeVector[iLoop]->DropData( outf, piLevel );
			( *piLevel )--;
		}
	}
	else
	{
		WriteFormatedStringA2( outf, _T("There is no measuring data\n"), strTab );
	}

	WriteFormatedStringA2( outf, _T("\n"), strTab );

	if( true == IsaModule() )
	{
		CDS_HydroMod *pHMChild = dynamic_cast<CDS_HydroMod *>( GetFirst().MP );

		if( NULL != pHMChild )
		{
			( *piLevel )++;
			pHMChild->DropData( outf, piLevel );
			( *piLevel )--;
		}
	}
}

CDB_ControlProperties::CvCtrlType CDS_HydroMod::GetCvCtrlType()
{
	// Modification done for HYS-663 IsCvExist( false ) in place of IsCvExist( true )
	if( true == IsCvExist( false ) )
	{
		return GetpCV()->GetCtrlType();
	}

	return CDB_ControlProperties::CvCtrlType::eCvNU;
}

void CDS_HydroMod::ResizeShutOffValves()
{
	// Remark: this method is called only by 'HMHub'.
	if( NULL != GetpShutoffValve( eHMObj::eShutoffValveSupply ) )
	{
		GetpShutoffValve( eHMObj::eShutoffValveSupply )->ResizeShutoffValve();
	}

	if( NULL != GetpShutoffValve( eHMObj::eShutoffValveReturn ) )
	{
		GetpShutoffValve( eHMObj::eShutoffValveReturn )->ResizeShutoffValve();
	}
}

// Return IDPTR of TADB valve measured into the CBI
IDPTR CDS_HydroMod::GetTADBValveIDPtr()
{
	return GetCBIValveIDPtr();
}

int CDS_HydroMod::CheckValidity()
{
	int iReturn = 0;

	// First step: Check error/warning messages on valves.

	if( NULL != GetpBv() )
	{
		iReturn += GetpBv()->CheckValidity();
	}

	if( NULL != GetpBypBv() )
	{
		iReturn += GetpBypBv()->CheckValidity();
	}

	if( NULL != GetpSecBv() )
	{
		iReturn += GetpSecBv()->CheckValidity();
	}

	if( NULL != GetpDpC() )
	{
		iReturn += GetpDpC()->CheckValidity();
	}

	if( NULL != GetpCV() )
	{
		iReturn += GetpCV()->CheckValidity();
	}

	if( NULL != GetpSmartControlValve() )
	{
		iReturn += GetpSmartControlValve()->CheckValidity();
	}

	if( NULL != GetpSmartDpC() )
	{
		iReturn += GetpSmartDpC()->CheckValidity();
	}

	// Second step: Check error/warning messages on the hydraulic circuit itself.

	if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() )
	{
		// HYS-1930: auto-adapting variable flow with decoupling bypass circuit.

		if( m_mapErrorMessages.count( CDS_HydroMod::eValidityFlags::evfSecPumpHMinTechParam ) != 0 )
		{
			iReturn += CDS_HydroMod::eValidityFlags::evfSecPumpHMinTechParam;
		}

		if( m_mapErrorMessages.count( CDS_HydroMod::eValidityFlags::evfSecPumpNotNeeded ) != 0 )
		{
			iReturn += CDS_HydroMod::eValidityFlags::evfSecPumpNotNeeded;
		}
	}

	return iReturn;
}

bool CDS_HydroMod::IsPending()
{
	if( NULL != GetpSchcat() )
	{
		if( true == GetpSchcat()->IsPending() )
		{
			return true;
		}
	}

	return false;
}

bool CDS_HydroMod::IsDiversityExist()
{
	bool bReturn = false;

	if( GetDiversityFactor() != -1.0 && GetDiversityFactor() < 1.0 )
	{
		bReturn = true;
	}
	else
	{
		if( m_pDistrSupplyPipe != NULL && true == m_pDistrSupplyPipe->IsDiversityApplied() )
		{
			bReturn = true;
		}
		else if( ReturnType::Reverse == m_eReturnType && m_pDistrReturnPipe != NULL && true == m_pDistrReturnPipe->IsDiversityApplied() )
		{
			bReturn = true;
		}
	}

	return bReturn;
}

double CDS_HydroMod::GetDpOnOutOfPrimSide()
{
	if( NULL != GetpTermUnit() )
	{
		return GetpTermUnit()->GetDp();
	}

	return 0.0;
}

void CDS_HydroMod::SendEventToConsole( eComputeHMEvent event )
{
	CString str;

	switch( event )
	{
		case eceNone:
			str = _T("eceNone");
			break;

		case eceResize:
			str = _T("eceResize");
			break;

		case eceTUQ:
			str = _T("eceTUQ");
			break;

		case eceTUDp:
			str = _T("eceTUDp");
			break;

		case ecePipeSerie:
			str = _T("ecePipeSerie");
			break;

		case ecePipeSize:
			str = _T("ecePipeSize");
			break;

		case ecePipeLength:
			str = _T("ecePipeLength");
			break;

		case eceBV:
			str = _T("eceBV");
			break;

		case eceBVType:
			str = _T("eceBVType");
			break;

		case eceBVConnect:
			str = _T("eceBVConnect");
			break;

		case eceBVVers:
			str = _T("eceBVVers");
			break;

		case eceBVPN:
			str = _T("eceBVPN");
			break;

		case eceDpC:
			str = _T("eceDpC");
			break;

		case eceDpCType:
			str = _T("eceDpCType");
			break;

		case eceDpCConnect:
			str = _T("eceDpCConnect");
			break;

		case eceDpCPN:
			str = _T("eceDpCPN");
			break;

		case eceCVKvs:
			str = _T("eceCVKvs");
			break;

		case eceCV:
			str = _T("eceCV");
			break;

		case eceCVConnect:
			str = _T("eceCVConnect");
			break;

		case eceCVVers:
			str = _T("eceCVVers");
			break;

		case eceCVBdy:
			str = _T("eceCVBdy");
			break;

		case eceCVPN:
			str = _T("eceCVPN");
			break;

		case eceBypBvAdded:
			str = _T("eceBypBvAdded");
			break;

		case eceShutOffValves:
			str = _T("eceShutOffValves");
			break;

		case eceShutoffValveType:
			str = _T("eceShutoffValveType");
			break;

		case eceShutoffValveConnect:
			str = _T("eceShutoffValveConnect");
			break;

		case eceShutoffValveVersion:
			str = _T("eceShutoffValveVersion");
			break;

		case eceShutoffValvePN:
			str = _T("eceShutoffValvePN");
			break;

		case eceSmartControlValve:
			str = _T("eceSmartControlValve");
			break;

		case eceSmartControlValveBodyMaterial:
			str = _T("eceSmartControlValveBodyMaterial");
			break;

		case eceSmartControlValveConnect:
			str = _T("eceSmartControlValveConnect");
			break;

		case eceSmartControlValvePN:
			str = _T("eceSmartControlValvePN");
			break;

		case eceSmartDpC:
			str = _T("eceSmartDpC");
			break;

		case eceSmartDpCBodyMaterial:
			str = _T("eceSmartDpCBodyMaterial");
			break;

		case eceSmartDpCConnect:
			str = _T("eceSmartDpCConnect");
			break;

		case eceSmartDpCPN:
			str = _T("eceSmartDpCPN");
			break;

		default:
			str = _T("Default!");
			break;
	}

	CString strClass;
	strClass = this->GetClassNameW();
	CString strOut = strClass + CString( _T(":\tEvent: ") ) + str;
}

// Used exclusively for pressure interfaces.
// It means also that this method is called only when we are going to the pump.
void CDS_HydroMod::_UpdateHAvailAccordingTUDp( CDS_HydroMod *pHM )
{
	if( NULL == pHM->GetpSchcat() )
	{
		ASSERT_RETURN;
	}

	if( CDB_ControlProperties::CV2W3W::CV3W == pHM->GetpSchcat()->Get2W3W() && CDB_CircSchemeCateg::e3WType::e3wTypeMixing == pHM->GetpSchcat()->Get3WType()
			&& eDpStab::DpStabOnBranch == pHM->GetpSch()->GetDpStab() )
	{
		// 3-way mixing is a special injection circuit that manages itself the H available.
		return;
	}

	m_bComputeAllowed = false;

	// Working on Inj. circuit means that we have to update secondary side.
	// primary side has been updated during the AdjustCV.
	if( true == pHM->GetpSchcat()->IsSecondarySideExist() )
	{
		pHM->AdjustPumpHmin();

		// Pressure interface with DpC on branch.
		if( true == pHM->IsDpCExist() && CDS_HydroMod::ePressIntType::pitDpC == pHM->GetPressIntType()
				&& NULL != pHM->GetpSch() && DpStabOnBranch == pHM->GetpSch()->GetDpStab() )
		{
			// TO CLARIFY: Why do we resize to 1.0 ? 
			pHM->GetpDpC()->ResizeDpC( 1 );

			// TO CLARIFY: Why do we call 'GetDp' without parameter -> bDpMin = false while we are at this stage going
			// to the pump
			// Third parameter 'true' to specify that we are going to the pump.
			pHM->SetHavail( pHM->GetDp(), CAnchorPt::CircuitSide_Primary, true );
		}
		else if( true == pHM->IsDpCExist() && CDS_HydroMod::ePressIntType::pitDpC == pHM->GetPressIntType()
				&& NULL != pHM->GetpSch() && DpStabOnBVBypass == pHM->GetpSch()->GetDpStab() )
		{
			// Third parameter 'true' to specify that we are going to the pump.
			pHM->SetHavail( pHM->GetDp( true ), CAnchorPt::CircuitSide_Primary, true );
		}
	}

	// 0 == level means that we are on the root module !
	if( false == pHM->GetpSchcat()->IsSecondarySideExist() || 0 == pHM->GetLevel() )
	{
		// Third parameter 'true' to specify that we are going to the pump.
		pHM->SetHavail( pHM->GetDp(), CAnchorPt::CircuitSide_Primary, true );
	}

	m_bComputeAllowed = true;
}

// Used exclusively for pressure interfaces.
void CDS_HydroMod::_UpdateTUDpAccordingHavailable()
{
	m_bComputeAllowed = false;

	// If a pump exist, it can be the one in the root with or without a partner valve or it can be any other injection
	// circuits (2-way injection, 3-way injection or 3-way injection with decoupling bypasss circuits).
	if( NULL != GetpPump() )
	{
		if( eDpStab::DpStabOnBVBypass != GetpSch()->GetDpStab() && CDB_CircSchemeCateg::e3wTypeMixing != GetpSchcat()->Get3WType() )
		{
			double dTUDp = 0.0;
			double dPumpHAvail = GetpPump()->GetHAvail();
			double dCVDp = 0.0;

			if( GetpSchcat() != NULL && true == GetpSchcat()->IsSecondarySideExist() )
			{
				// It's a pump for a 2-way injection, 3-way injection or 3-way injection with decoupling bypass circuits.
				double dBvDpSec = 0.0;

				// HYS-1690: The 3-way mixing circuit has its CV on the primary side but we consider the Dp of the CV.
				if( NULL != GetpCV() && ( eb3False == GetpCV()->IsCVLocInPrimary() /*|| CDB_CircSchemeCateg::e3wTypeMixing == GetpSchcat()->Get3WType() */ ) )
				{
					dCVDp = GetpCV()->GetDp();
				}

				if( NULL != GetpSecBv() && NULL != GetpSecBv()->GetIDPtr().MP )
				{
					if( GetpPump()->GetHpump() > GetpPump()->GetHmin() )
					{
						// Bv should be adjusted to take the excess of differential pressure.
						double dDpToTake = 0.0;

						// Remark: Hpump and Hmin already contain respectively the Dp BVsec and the Dpmin BVsec.
						// HPump - HminPump = Dp BVsec - Dpmin BVsec = Excess of differential pressure.
						// Dp BVsec = HPump - HminPump + Dpmin BVsec.
						dDpToTake = max( 0, GetpPump()->GetHpump() - GetpPump()->GetHmin() + GetpSecBv()->GetDpMin() );

						// Before resizing the balancing valve, we must take its Dpmin.
						double dOldBvSecDpMin = GetpSecBv()->GetDpMin();

						GetpSecBv()->ResizeBv( dDpToTake );
						dBvDpSec = GetpSecBv()->GetDp();

						// Now we have to verify if Dpmin of the balancing valve has not increased.
						double dNewBvSecDpMin = GetpSecBv()->GetDpMin();

						if( dNewBvSecDpMin != dOldBvSecDpMin )
						{
							// Hmin must change.
							GetpPump()->SetHmin( GetpPump()->GetHmin() + abs( dNewBvSecDpMin - dOldBvSecDpMin ) );

							// By chance, if user has already defined a pump head, the maximum that we can reach by increasing
							// the Hmin will never be above the H available (See doc for demonstration).
						}
					}
					else
					{
						dBvDpSec = GetpSecBv()->GetDp();
					}
				}

				/*
				double dHbypass = 0.0;

				if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() && NULL != GetpBypBv() && GetpBypBv()->IsCompletelyDefined() )
				{
					// HYS-1930: auto-adapting variable flow decoupling circuit.
					// In this case, we must not forget that the stabilisez differential pressure in the bypass helps the secondary pump.
					dHbypass = GetpBypBv()->GetDp();
				}
				*/

				dTUDp = dPumpHAvail - dCVDp - dBvDpSec;		//+ dHbypass;
			}
			else if( NULL != GetpBv() && NULL != GetpBv()->GetIDPtr().MP )
			{
				// A circuit with a pump without secondary side is only the root circuit.
				// This one can have partner valve or not.
				double dBvDpPrim = 0.0;

				// Bv should be adjusted to take the excess of differential pressure.
				// See remark above.
				double dDpToTake = max( 0, GetpPump()->GetHAvail() - ( GetpPump()->GetHmin() - GetpBv()->GetDpMin() ) );
				GetpBv()->ResizeBv( dDpToTake );
				dBvDpPrim = GetpBv()->GetDp();

				dTUDp = dPumpHAvail - dBvDpPrim;
			}

			GetpTermUnit()->SetDp( dTUDp, false );
		}
	}
	else
	{
		if( CDS_HydroMod::ePressIntType::pitStatic == GetPressIntType() )
		{
			double dCircuitDp = GetDp() - GetpTermUnit()->GetDp();
			double dTUDp = m_dHAvail - dCircuitDp;
			GetpTermUnit()->SetDp( dTUDp, false );
		}
		else if( CDS_HydroMod::ePressIntType::pitDpC == GetPressIntType() )
		{
			// TUDp should be up-to-date see SetHAvail()!
		}
	}

	m_bComputeAllowed = true;
}

/**
 *	@brief Return Havail for the ciruit
 *	@param CircuiSide primary or secondary side of the circuit,
 *			when secondary side it return H available at the outside of the circuit (for instance injection module, DpC stabilazed module)
 *	@param bHmin return minimum value
 */
double CDS_HydroMod::GetHAvail( CAnchorPt::CircuitSide eCircuitSide, bool bHmin )
{
	double dDp = 0;

	if( CAnchorPt::CircuitSide_Primary == eCircuitSide )
	{
		dDp = m_dHAvail;

		// Pump at primary pipe can be only for the root module (Level = 0) with a pump.
		if( NULL != GetpPump() && NULL != GetpSchcat() && false == GetpSchcat()->IsSecondarySideExist() && CDB_CircSchemeCateg::e3wTypeMixing != GetpSchcat()->Get3WType() )
		{
			if( true == bHmin )
			{
				dDp = GetpPump()->GetHmin( );
			}
			else
			{
				dDp = GetpPump()->GetHAvail();
			}
		}
	}
	else
	{
		// Retrieve the H available for the secondary side (What is delivered to the children).

		if( NULL != GetpPump() )
		{
			if( true == bHmin )
			{
				dDp = GetpPump()->GetHmin( );
			}
			else
			{
				dDp = GetpPump()->GetHAvail();
			}

			if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() )
			{
				// HYS-1930: auto-adapting variable flow decoupling circuit.
				// The stabilized differential pressure in the bypass helps the pump.
				if( NULL != GetpBypBv() )
				{
					dDp += GetpBypBv()->GetDp();
				}
			}
		}
		else
		{
			if( CDS_HydroMod::ePressIntType::pitStatic == GetPressIntType() )
			{
				dDp = m_dHAvail;
			}
			else if( CDS_HydroMod::ePressIntType::pitDpC == GetPressIntType() )
			{
				double dHavail = GetpTermUnit()->GetDp();

				// If Bv exist and is located inside the stabilized pressure we have to add it.
				if( true == IsBvExist( true ) )
				{
					// Take in count BV only if it's located on secondary.
					CDB_CircuitScheme *pSch = GetpSch();

					if( NULL != pSch && MvLocSecondary == pSch->GetMvLoc() )
					{
						dHavail += GetpBv()->GetDp();
					}
				}

				if( true == IsCvExist( true ) )
				{
					dHavail += GetpCV()->GetDp();
				}

				dDp = dHavail;
			}
		}
	}

	return dDp;
}

void CDS_HydroMod::ResetHavailRecursively( )
{
	for( IDPTR idptr = this->GetFirst(); NULL != idptr.MP; idptr = this->GetNext( idptr.MP ) )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod * >( idptr.MP );

		if( NULL != pHM )
		{
			// Third parameter 'true' to specify that we are going to the pump.
			pHM->SetHavail( 0.0, CAnchorPt::CircuitSide_Primary, true );
		}

		if( true == pHM->IsaModule() && NULL != pHM->GetpSchcat() && false == pHM->GetpSchcat()->IsSecondarySideExist() )
		{
			pHM->ResetHavailRecursively( );
		}
	}
}

bool CDS_HydroMod::_UpdateDPCDp( CDS_HydroMod *pHM, double dHPressIntSaved, double dNewHAtPressureInt )
{
	// DPC
	// If a DPC exist, we have to update TU Dp and HAvail.
	if( CDS_HydroMod::ePressIntType::pitDpC == GetPressIntType() && NULL != GetpSch() && DpStabOnBranch == GetpSch()->GetDpStab() )
	{
		// Increase TU Dp.
		double dDpIncrease = dNewHAtPressureInt - dHPressIntSaved;
		double dTUDp = GetpTermUnit()->GetDp();

		// We will update TUdp only if we are not working on the pressure interface.
		if( this != pHM )
		{
			GetpTermUnit()->SetDp( dTUDp + dDpIncrease );
		}
		
		// HYS-1369 : Look if GetpDpC() exists.
		if( NULL != GetpDpC() )
		{
			CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( GetpDpC()->GetpTAP() );

			if( NULL != pDpC )
			{
				if( dNewHAtPressureInt > pDpC->GetDplmax() )
				{
					// Try to resize pDpC to take into account this new value of dHavail.
					// Absolutely needed when resizing CV ...
					// PB could be modified (DA50-100 (50-150) PB = 20kPa; DA50-100 (100-400) PB = 100kPa).
					GetpDpC()->ResizeDpC( 0 );
				}

				// Increase HAvailable
				m_dHAvail = GetDp();
			}
		}

		Modified();
		return true;
	}

	return false;
}

// Set value of m_HAvail
// return true when the TUDp has been updated.
bool CDS_HydroMod::SetHavail( double dHAvail, CAnchorPt::CircuitSide eCircuitSide, bool bGoingToPump, bool *pbHasHAvailSecChanged )
{
	// Sanity test
	if( NULL != GetpSchcat() && false == GetpSchcat()->IsSecondarySideExist() && CAnchorPt::CircuitSide_Secondary == eCircuitSide )
	{
		eCircuitSide = CAnchorPt::CircuitSide_Primary;
		ASSERT( 0 );
	}

	if( NULL != pbHasHAvailSecChanged )
	{
		*pbHasHAvailSecChanged = false;
	}

	// When primary side we have to update m_HAvail;
	// This case occurs when we are going to the pump.
	if( CAnchorPt::CircuitSide_Primary == eCircuitSide )
	{
		if( m_dHAvail != dHAvail )
		{
			m_dHAvail = dHAvail;
			Modified();
		}

		if( NULL != GetpPump() && NULL != GetpSchcat() )
		{
			// If there is a pump in this circuit we will update Pump Hmin.
			if( false == GetpSchcat()->IsSecondarySideExist() && CDB_CircSchemeCateg::e3wTypeMixing != GetpSchcat()->Get3WType() )
			{
				// Remark: The only case here for a circuit with a pump and without secondary side is the root module with a pump
				GetpPump()->SetHmin( dHAvail );

				if( true == IsaModule() )
				{
					return true;
				}
			}
			else if( true == GetpSchcat()->Is3W() && CDB_CircSchemeCateg::e3wTypeMixing == GetpSchcat()->Get3WType() && dHAvail > 0.0 )
			{
				// Code here is only for the 3-way mixing circuit that is very particular.

				if( false == IsDpCExist( true ) || NULL == GetpSch() || DpStabOnBranch != GetpSch()->GetDpStab() )
				{
					// Pressure drop on the control valve should be sized based on H available.
					double dOldDpCV = GetpCV()->GetDp();
					GetpCV()->Size3WCvCstFlow( dHAvail, false );

					// Resize the balancing valve at the primary side to take into account same Dp as CV and HAvail.
					// It's possible that the new pressure drop across the control valve will be different than requested due to compliance with Reynard series.
					double dNewDpCV = GetpCV()->GetDp();
					double dDpBv = dNewDpCV;

					if( NULL != GetpCircuitPrimaryPipe() )
					{
						dDpBv -= GetpCircuitPrimaryPipe()->GetPipeDp();
						dDpBv -= GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true );
					}
							
					GetpBv()->ResizeBv( dDpBv );
					
					// We need to update "m_dHAvail" because the value can be higher due to the control valve.
					m_dHAvail = dNewDpCV;
				}
				else
				{
					// To no change the code in the "SelectBestDpC" method, we need to size BV first. Because in this method, if we don't want a Dp controller
					// with the lowest Dplmin, we call the "GetDpToStab" method that returns pressure drop accross BV in the case of a 3-way mixing circuit.
					double dDpCv = GetpCV()->GetDp();
					GetpBv()->ResizeBv( dDpCv );

					// We invalidate selection. Otherwise the "SelectBestDpC" method will try to match a Dp controller with what was previously selected.
					GetpDpC()->InvalidateSelection();

					// Now we can size the DpC that will take the pressure drop of the BV as differential pressure to stabilize.
					GetpDpC()->SelectBestDpC( 0.0 );

					// If 'm_dHAvail' = 0  -> We size the circuit for its minimum. So we can update 'm_dHAvail'.
					// If 'm_dHAvail' != 0 -> We size the circuit also for its minimum. But we don't change the value. The difference between Hmin and Havail
					// will bne taken by the Dp controller.
					if( 0.0 == m_dHAvail )
					{
						// 'dDpCv' here is the same as Dpl.
						m_dHAvail = GetpDpC()->GetDpmin() + dDpCv;

						if( NULL != GetpCircuitPrimaryPipe() )
						{
							m_dHAvail += GetpCircuitPrimaryPipe()->GetPipeDp();
							m_dHAvail += GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true );
						}
					}
				}

				// Before checking the secondary side, we resize the BV to a minimum.
				GetpSecBv()->ResizeBv( 0.0 );

				// Verify now pump Hmin on the secondary side.
				double dDpTU = GetpTermUnit()->GetDp();
				double dDpBVsmin = GetpSecBv()->GetDpMin();
				double dDpCV = GetpCV()->GetDp();

				double dOldHPumpMin = GetpPump()->GetHmin();
				double dDpHPump = GetpPump()->GetHpump();

				// Any change in the pressure drop of the control valve is taken by the secondary pump.
				// So no need to change TU and to impact children.
				double dNewHPumpMin = dDpTU + dDpBVsmin + dDpCV;

				if( dNewHPumpMin != dOldHPumpMin )
				{
					GetpPump()->SetHmin( dNewHPumpMin );

					if( NULL != pbHasHAvailSecChanged )
					{
						*pbHasHAvailSecChanged = true;
					}
				}

				// Bv should be adjusted to take the excess of differential pressure.
				double dDpToTake = 0.0;

				// Remark: Hpump and Hmin already contain respectively the Dp BVsec and the Dpmin BVsec.
				// HPump - HminPump = Dp BVsec - Dpmin BVsec = Excess of differential pressure.
				// Dp BVsec = HPump - HminPump + Dpmin BVsec.
				dDpToTake = max( 0, GetpPump()->GetHAvail() - GetpPump()->GetHmin() + GetpSecBv()->GetDpMin() );

				// Now we resize BV to be aligned with Havail et Hmin of the pump if changes have been applied.
				double dOldBvsDpMin = GetpSecBv()->GetDpMin();
				GetpSecBv()->ResizeBv( dDpToTake );

				double dNewBvsDpMin = GetpSecBv()->GetDpMin();

				if( dNewBvsDpMin != dOldBvsDpMin )
				{
					// When resizing the BV at the secondary side, we can get the case where the size is changed and then
					// that the Dp min also change.
					GetpPump()->SetHmin( GetpPump()->GetHmin() + ( dNewBvsDpMin - dOldBvsDpMin ) );
				}

				if( true == IsaModule() )
				{
					return true;
				}
			}
		}
	}
	else // Secondary
	{
		// Pump exist --> Pump circuit or 2wInj
		if( NULL != GetpPump() )
		{
			if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() && NULL != GetpBypBv() )
			{
				GetpPump()->SetHmin( max( 0.0, dHAvail - GetpBypBv()->GetDp() ) );
			}
			else
			{
				GetpPump()->SetHmin( dHAvail );
			}

			if( true == IsaModule() )
			{
				return true;
			}
		}
		else // Secondary side withou Pump  == not working
		{
			ASSERT( 0 );
		}
	}

	return false;
}

void CDS_HydroMod::_VerifyConnection( CDS_HydroMod *pHM, CDS_HydroMod *pParentHM )
{
	if( NULL == pHM || NULL == pHM->GetpSchcat() || NULL == pHM->GetpSch() )
	{
		return;
	}

	// (n), (1<i<n) and (1) are the position of the circuit in the module.
	// t is for a TEE connection.
	// e is for a ELBOW connection.
	//
	// CONNECTION SINGULARITY FOR CIRCUIT
	//
	// DC
	//    *     *                                                 Last        |  Middle     | First
	//    |     |                                                 (n)         |  (1<i<n)    | (1)
	//    |  (B)t-----                                            ------------+-------------+-----------
	//    |     | (1)                    Circuit pipe (Primary)   2*Elbow(A)  | 2*Tee-Za(B) | 2*Tee-Za(B)
	// (B)t---- | ----
	//    |     |
	//    |  (A)e-----
	//    |       (n)
	// (A)e-----------
	//
	// 2-way injection.
	//    *     *                                                 Last        |  Middle     | First
	//    |     |    (C)                                          (n)         |  (1<i<n)    | (1)
	//    |  (B)t-----t--Pump---                                  ------------+-------------+-----------
	//    |     |     |      (1)                  Primary pipe    2*Elbow(A)  | 2*Tee-Za(B) | 2*Tee-Za(B)
	// (B)t---- | ----t---------                Secondary pipe    2*Tee-Zb(C) | 2*Tee-Zb(C) | 2*Tee-Zb(C)
	//    |     |                                                 2*Tee-Za(C) | 2*Tee-Za(C) | 2*Tee-Za(C)
	//    |  (A)e-----t--Pump---                   Bypass pipe    Not defined for this case (We have only for auto-adapting variable flow decoupling circuit).
	//    |           |      (n)
	// (A)e-----------t---------
	//               (C)
	// 
	// 3-way dividing.
	//    *     *                                                 Last        |  Middle     | First
	//    |     |    (C)                                          (n)         |  (1<i<n)    | (1)
	//    |  (B)t-----t--Pump---                                  ------------+-------------+-----------
	//    |     |     |      (1)                  Primary pipe    2*Elbow(A)  | 2*Tee-Za(B) | 2*Tee-Za(B)
	// (B)t---- | ----CV--------                                  1*Tee-Zb(C) | 1*Tee-Zb(C) | 1*Tee-Zb(C)
	//    |     |    (D)                                          1*CV(D)     | 1*CV(D)     | 1*CV(D)     (Count only one time at Kvs)
	//    |  (A)e-----t--Pump---				                  1*Tee-Za(C) | 1*Tee-Za(C) | 1*Tee-Za(C)
	//    |           |      (n)                   Bypass pipe    Not defined for this case (We have only for auto-adapting variable flow decoupling circuit).
	// (A)e-----------t---------
	//               (C)
	// 
	// 3-way mixing simple.
	//    *     *                                                 Last        |  Middle     | First
	//    |     |    (D)                                          (n)         |  (1<i<n)    | (1)
	//    |  (B)t----CV--Pump---                                  ------------+-------------+-----------
	//    |     |     |      (1)                  Primary pipe    2*Elbow(A)  | 2*Tee-Za(B) | 2*Tee-Za(B)
	// (B)t---- | ----t---------                                  1*Tee-Zb(C) | 1*Tee-Zb(C) | 1*Tee-Zb(C)
	//    |     |    (C)                                          1*CV(D)     | 1*CV(D)     | 1*CV(D)     (Count only one time at Kvs)
	//    |  (A)e-----t--Pump---				                  1*Tee-Za(C) | 1*Tee-Za(C) | 1*Tee-Za(C)
	//    |           |      (n)                   Bypass pipe    Not defined for this case (We have only for auto-adapting variable flow decoupling circuit).
	// (A)e-----------t---------
	//               (C)
	// 
	// 3-way mixing circuit with decoupling bypass (CV at primary side)
	//    *     *                                                 Last        |  Middle     | First
	//    |     |    (C)   (E)                                    (n)         |  (1<i<n)    | (1)
	//    |  (B)t-----t-----t--Pump---                            ------------+-------------+-----------
	//    |     |     |     |  (1)                Primary pipe    2*Elbow(A)  | 2*Tee-Za(B) | 2*Tee-Za(B)
	// (B)t---- | ---CV-----t---------                            1*Tee-Zb(C) | 1*Tee-Zb(C) | 1*Tee-Zb(C)
	//    |     |    (D)                                          1*CV(D)     | 1*CV(D)     | 1*CV(D)     (Count only one time at Kvs)
	//    |  (A)e-----t-----t--Pump---          Secondary pipe    2*Tee-Zb(E) | 2*Tee-Zb(E) | 2*Tee-Zb(E)
	//    |           |     |  (n)                                2*Tee-Za(E) | 2*Tee-Zb(E) | 2*Tee-Zb(E) (Include tee on the bypass at secondary side)
	// (A)e----------CV-----t---------                            1*Tee-Za(C) | 1*Tee-Za(C) | 1*Tee-Za(C) (For the moment we have only bypass pipe defined that is the primary one!)
	//               (D)   (E)                     Bypass pipe    Not defined for this case (We have only for auto-adapting variable flow decoupling circuit).
	//
	// 3-way mixing circuit with decoupling bypass (CV at secondary side)
	//    *     *                                                 Last        |  Middle     | First
	//    |     |    (C)   (D)                                    (n)         |  (1<i<n)    | (1)
	//    |  (B)t-----t----CV--Pump---                            ------------+-------------+-----------
	//    |     |     |     |  (1)                Primary pipe    2*Elbow(A)  | 2*Tee-Za(B) | 2*Tee-Za(B)
	// (B)t---- | ----t-----t---------          Secondary pipe    2*Tee-Zb(C) | 2*Tee-Zb(C) | 2*Tee-Zb(C)
	//    |     |          (D)                                    2*Tee-Za(C) | 2*Tee-Zb(C) | 2*Tee-Zb(C) (Include tee on the bypass at primary side)
	//    |  (A)e-----t----CV--Pump---                            1*CV(D)     | 1*CV(D)     | 1*CV(D)     (Count only one time at Kvs)
	//    |           |     |  (n)                                1*Tee-Zb(E) | 1*Tee-Zb(E) | 1*Tee-Zb(E) (Include tee on the bypass at secondary side)
	// (A)e-----------t-----t---------                            1*Tee-Za(CE | 1*Tee-Za(E) | 1*Tee-Za(C)
	//               (C)   (E)                     Bypass pipe    Not defined for this case (We have only for auto-adapting variable flow decoupling circuit).
    //
 	// Auto-adapting variable flow decoupling circuit
	//    *     *                                                 Last        |  Middle     | First
	//    |     |    (C)                                          (n)         |  (1<i<n)    | (1)
	//    |  (B)t-----t--Pump---                                  ------------+-------------+-----------
	//    |     |     |      (1)                  Primary pipe    2*Elbow(A)  | 2*Tee-Za(B) | 2*Tee-Za(B)
	// (B)t---- | ----t---------                Secondary pipe    2*Tee-Zb(C) | 2*Tee-Zb(C) | 2*Tee-Zb(C)
	//    |     |                                  Bypass pipe    2*Tee-Za(C) | 2*Tee-Za(C) | 2*Tee-Za(C)
	//    |  (A)e-----t--Pump---
	//    |           |      (n)
	// (A)e-----------t---------
	//               (C)
	//
	// CONNECTION SINGULARITY FOR DISTRIBUTION
	//
	// Direct return mode.
	//    *     *                                                 Last      |  Middle   | First
	//    |     |                                                 (n)       |  (1<i<n)  | (1)
	//    |  (A)t-----                                            ----------+-----------+-----------
	//    |     | (1)                Distribution pipe (Supply)   None      | Tee-Zb(A) | Tee-Zb(A)
	// (A)t---- | ----
	//    |     |
	//    |     e-----
	//    |       (n)
	//    e-----------
	//
	// Reverse return mode.
	//    *        *                                              Last      |  Middle   | First
	//    |        |                                              (n)       |  (1<i<n)  | (1)
	//    |     (B)t-----                                         ----------+-----------+-----------
	//    |        | (1)              Distribution supply pipe    None      | Tee-Zb(A) | Tee-Zb(B)
	//    |   e--- | ----             Distribution return pipe    Tee-Zb(C) | Tee-Zb(D) | None
	//    |   |    |
	//    |   | (A)t-----
	//    |   |    | (1<i<n)
	//    |(D)t--- | ----
	//    |   |    |
	//    |   |    e-----
	//    |   |      (n)
	//    |(C)t----------
	//    |   |
	//    +---+

	if( false == pHM->IsForHub() )
	{
		// If we are not on a root module
		if( NULL != pParentHM )
		{
			// If we are on the last circuit...
			if( pHM->GetPos() == pParentHM->GetCount() )
			{
				// Circuit pipe (primary) has an Elbow 90° connection.
				pHM->GetpCircuitPrimaryPipe()->CheckPipeConnection( _T("ELBOW_90") );

				// Distribution pipe (supply) has no connection.
				pHM->GetpDistrSupplyPipe()->CheckPipeConnection( _T("SINGUL_NONE") );

				// If we are in reverse return mode...
				if( ReturnType::Reverse == pHM->GetReturnType() && NULL != pHM->GetpDistrReturnPipe() )
				{
					// Distribution pipe (return) has an Tee connection (with dzeta b).
					pHM->GetpDistrReturnPipe()->CheckPipeConnection( _T("TEE_DZB") );
				}
			}
			else if( pHM->GetPos() > 1 &&  pHM->GetPos() < pParentHM->GetCount() )
			{
				// We are between the last and the first circuit.

				// Circuit pipe (primary) has an Tee connection (with dzeta a).
				pHM->GetpCircuitPrimaryPipe()->CheckPipeConnection( _T("TEE_DZA") );

				// Distribution pipe (supply) has an Tee connection (with dzeta b).
				pHM->GetpDistrSupplyPipe()->CheckPipeConnection( _T("TEE_DZB") );

				// If we are in reverse return mode...
				if( ReturnType::Reverse == pHM->GetReturnType() && NULL != pHM->GetpDistrReturnPipe() )
				{
					// Distribution pipe (return) has an Tee connection (with dzeta b).
					pHM->GetpDistrReturnPipe()->CheckPipeConnection( _T("TEE_DZB") );
				}
			}
			else
			{
				// We are on the first circuit of the module.

				// Circuit pipe (primary) has an Tee connection (with dzeta a).
				pHM->GetpCircuitPrimaryPipe()->CheckPipeConnection( _T("TEE_DZA") );

				// Distribution pipe (supply) has an Tee connection (with dzeta b).
				pHM->GetpDistrSupplyPipe()->CheckPipeConnection( _T("TEE_DZB") );

				// If we are in reverse return mode...
				if( ReturnType::Reverse == pHM->GetReturnType() && NULL != pHM->GetpDistrReturnPipe() )
				{
					// There is no connection for distribution return pipe for the first circuit.
					pHM->GetpDistrReturnPipe()->CheckPipeConnection( _T("SINGUL_NONE") );
				}
			}

			if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() && NULL != pHM->GetpCircuitBypassPipe() 
					&& NULL != pHM->GetpCircuitSecondaryPipe()  )
			{
				// For auto-adapting variable flow decoupling circuit.
				pHM->GetpCircuitBypassPipe()->CheckPipeConnection( _T("TEE_DZA") );
				pHM->GetpCircuitSecondaryPipe()->CheckPipeConnection( _T("TEE_DZB") );
			}
		}
		else
		{
			// HYS-1761: On a root module we clear all singularities.

			pHM->GetpCircuitPrimaryPipe()->RemoveAllSingularities();
			pHM->GetpDistrSupplyPipe()->RemoveAllSingularities();

			// If we are in reverse return mode...
			if( ReturnType::Reverse == pHM->GetReturnType() && NULL != pHM->GetpDistrReturnPipe() )
			{
				pHM->GetpDistrReturnPipe()->RemoveAllSingularities();
			}

			// Is injection...
			if( pHM->GetpSchcat() != NULL && true == pHM->GetpSchcat()->IsInjection() && pHM->GetpCircuitSecondaryPipe() != NULL )
			{
				pHM->GetpCircuitSecondaryPipe()->RemoveAllSingularities();
			}

			if( NULL != pHM->GetpCircuitBypassPipe() )
			{
				pHM->GetpCircuitBypassPipe()->RemoveAllSingularities();
			}
		}
	}
	else
	{
		// Specific for HUB.
		CDS_HmHub *pHub = NULL;

		if( true == pHM->IsClass( CLASS( CDS_HmHub ) ) )
		{
			pHub = static_cast<CDS_HmHub *>( pHM );
		}
		else
		{
			pHub = static_cast<CDS_HmHub *>( pHM->GetIDPtr().PP );
		}

		// Accessories Elbow are only present in loop details selection.
		if( NULL != pHub && CDS_HmHub::SelMode::LoopDetails == pHub->GetSelMode() )
		{
			pHM->GetpCircuitPrimaryPipe()->CheckPipeConnection( _T("ELBOW_90") );
		}
		else
		{
			pHM->GetpCircuitPrimaryPipe()->CheckPipeConnection( _T("SINGUL_NONE") );
		}

		pHM->GetpDistrSupplyPipe()->CheckPipeConnection( _T("SINGUL_NONE") );
	}
}

bool CDS_HydroMod::BreakOnHMName( CString HMName, bool bBreakOnFailure /*= true*/ )
{
	bool bRet = false;
	
#ifdef DEBUG
	if( true == WildcardMatch( GetHMName(), HMName ) )
	{
		bRet = true;

		if( true == bBreakOnFailure )
		{
			DebugBreak();
		}
	}
#endif

	return bRet;
}

void CDS_HydroMod::InitAllWaterCharWithParent( CTable *pclParent, eHMObj eWhichPipe )
{
	// HYS-1716: Set the water characteristic in the different pipes in regards to its design temperature interface.
	// For initialization step, see the document in "Doc\Hydraulic network\How are managed temperatures in pipes.docx".
	// We do the same when importing a project from a TA-Scope.

	CWaterChar clSupplyWaterChar;
	CWaterChar clReturnWaterChar;
	CDS_HydroMod *pclDesignTemperatureInterfaceHM = NULL;

	// Because this method is mainly called when building a network, this current hydraulic circuit is not yet inserted
	// in its parent table. Because that we can't call the 'GetDesignWaterChar' method to retrieve what is the water
	// characteristic from the parent design temperature interface that drives this current hydraulic circuit.
	// Hopefully, we have the 'pclParent' in which the current hydraulic circuit will be inserted. 'pclParent' can be
	// a design temperature interface or not. If it's not the case, we can retrieve what is the parent design temperature
	// of 'pclParent' because the 'pclParent' is already inserted in the network.

	if( NULL != dynamic_cast<CDS_HydroMod *>( pclParent ) )
	{
		pclDesignTemperatureInterfaceHM = (CDS_HydroMod *)pclParent;

		if( false == pclDesignTemperatureInterfaceHM->IsDesignTemperatureInterface() )
		{
			pclDesignTemperatureInterfaceHM = pclDesignTemperatureInterfaceHM->GetpDesignTemperatureInterface();
		}

		if( NULL == pclDesignTemperatureInterfaceHM )
		{
			ASSERT_RETURN;
		}
	}

	if( NULL != pclDesignTemperatureInterfaceHM && true == pclDesignTemperatureInterfaceHM->IsInjectionCircuit() )
	{
		// Water characteristic are the ones that we can find at the output of this design temperature interface.
		clSupplyWaterChar = *pclDesignTemperatureInterfaceHM->GetpOutWaterChar( CAnchorPt::PipeLocation_Supply );
		clReturnWaterChar = *pclDesignTemperatureInterfaceHM->GetpOutWaterChar( CAnchorPt::PipeLocation_Return );

		CDS_HmInj *pclDesignTemperatureInterfaceHMInj = dynamic_cast<CDS_HmInj *>( pclDesignTemperatureInterfaceHM );
		
		if( NULL == pclDesignTemperatureInterfaceHMInj )
		{
			ASSERT_RETURN;
		}

		// But for the temperature, we must take the designed one.
		double dSupplyTemperature = pclDesignTemperatureInterfaceHMInj->GetDesignCircuitSupplySecondaryTemperature();
		double dReturnTemperature = pclDesignTemperatureInterfaceHMInj->GetDesignCircuitReturnSecondaryTemperature();

		if( -273.15 == dSupplyTemperature || -273.15 == dReturnTemperature )
		{
			ASSERT_RETURN;
		}

		clSupplyWaterChar.UpdateFluidData( dSupplyTemperature );
		clReturnWaterChar.UpdateFluidData( dReturnTemperature );
	}
	else
	{
		// The parent is not a hydromod, we need to retrieve temperatures in the general project parameters.
		ASSERT( NULL != TASApp.GetpTADS()->GetpWCForProject() && NULL != TASApp.GetpTADS()->GetpWCForProject()->GetpWCData() );

		clSupplyWaterChar = *TASApp.GetpTADS()->GetpWCForProject()->GetpWCData();
		
		// Return water characteristic is the same as the supply. We just need to update the temperature.
		clReturnWaterChar = clSupplyWaterChar;
		clReturnWaterChar.UpdateFluidData( TASApp.GetpTADS()->GetpTechParams()->GetDefaultPrjTpr() );
	}
	
	if( eHMObj::eDistributionSupplyPipe == ( eHMObj::eDistributionSupplyPipe & eWhichPipe ) )
	{
		// The ditribution supply water characteristic is exactly the same as the one coming from the parent.
		SetWaterChar( &clSupplyWaterChar, CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Supply );
	}

	if( eHMObj::eCircuitPrimaryPipe == ( eHMObj::eCircuitPrimaryPipe & eWhichPipe ) )
	{
		// The circuit supply temperature at the primary side has the same temperature as the distribution supply temperature.
		SetWaterChar( &clSupplyWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );

		// The circuit return temperature has the same temperature as the designed one by user for the return temperature
		// of the parent. When going to the pump, the return computed temperature on the parent can be different that the
		// one enter by the user. But here, our circuit is designed to run at the temperatures given by the user.
		SetWaterChar( &clReturnWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );
	}

	if( eHMObj::eCircuitBypassPrimaryPipe == ( eHMObj::eCircuitBypassPrimaryPipe & eWhichPipe ) )
	{
		if( NULL != GetpCircuitBypassPipe() )
		{
			if( CAnchorPt::CircuitSide::CircuitSide_Primary == GetBypassPipeSide() )
			{
				// If bypass pipe is at the primary side, we take the same water characteristic and temperature of the circuit supply at the primary side.
				SetWaterChar( &clSupplyWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_ByPass, CAnchorPt::CircuitSide_Primary );
			}
			else if( CAnchorPt::CircuitSide::CircuitSide_Secondary == GetBypassPipeSide() )
			{
				// If bypass pipe is at the secondary side, we take the same water characteristic and temperature of the circuit return at the secondary side.
				SetWaterChar( &clReturnWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_ByPass, CAnchorPt::CircuitSide_Secondary );
			}
		}
	}

	if( eHMObj::eDistributionReturnPipe == ( eHMObj::eDistributionReturnPipe & eWhichPipe ) )
	{
		// Distribution return water characteristic is for the moment the same as the return circuit at the primary side.
		// The return temperature will be calculated later.
		SetWaterChar( &clReturnWaterChar, CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );
	}
}

std::vector<CString> CDS_HydroMod::GetErrorMessageList( CDS_HydroMod::ErrorWarningMessageFlags eErrorWarningMessageFlags )
{
	// Remark: "m_mapErrorMessages" is filled during the "ComputeAll" process.

	std::vector<CString> vecAllMessages;

	// HYS-1930: Implemented only at that moment only for the auto-adaptive with decoupling bypass hydraulic circuit.

	if( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_SecondaryPump == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_SecondaryPump )
			|| CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All ) )
	{
		if( m_mapErrorMessages.count( CDS_HydroMod::eValidityFlags::evfSecPumpHMinTechParam ) > 0 )
		{
			vecAllMessages.push_back( m_mapErrorMessages.at( CDS_HydroMod::eValidityFlags::evfSecPumpHMinTechParam ) );
		}
		
		if( m_mapErrorMessages.count( CDS_HydroMod::eValidityFlags::evfSecPumpNotNeeded ) > 0 )
		{
			vecAllMessages.push_back( m_mapErrorMessages.at( CDS_HydroMod::eValidityFlags::evfSecPumpNotNeeded ) );
		}
	}

	return vecAllMessages;
}

CString CDS_HydroMod::GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags eErrorWarningMessageFlags, CString strPrefix, bool bReturnLine )
{
	return ConvertVecStringToString( GetErrorMessageList( eErrorWarningMessageFlags ), strPrefix, bReturnLine );
}

void CDS_HydroMod::ClearErrorMessage( CDS_HydroMod::ErrorWarningMessageFlags eErrorWarningMessageFlags )
{
	if( EWMF_All == eErrorWarningMessageFlags )
	{
		m_mapErrorMessages.clear();
	}
	else
	{
		std::vector<int> vecFlagToClean;

		switch( eErrorWarningMessageFlags )
		{
			case EWMF_Temp:
				vecFlagToClean.push_back( evfTempTooHigh );
				vecFlagToClean.push_back( evfTempTooLow );
				break;

			case EWMF_Flow:
				vecFlagToClean.push_back( evfFlowTooLow );
				vecFlagToClean.push_back( evfFlowTooHigh );
				break;

			case EWMF_Dp:
				vecFlagToClean.push_back( evfDpTooHigh );
				vecFlagToClean.push_back( evfDpTooLow );
				break;

			case EWMF_DpSignal:
				vecFlagToClean.push_back( evfDpSignalTooHigh );
				vecFlagToClean.push_back( evfDpSignalTooLow );
				break;

			case EWMF_MaxDp:
				vecFlagToClean.push_back( evfMaxDpTooHigh );
				break;

			case EWMF_Dpl:
				vecFlagToClean.push_back( evfDplTooHigh );
				vecFlagToClean.push_back( evfDplTooLow );
				break;

			case EWMF_Power:
				vecFlagToClean.push_back( evfPowerTooLow );
				vecFlagToClean.push_back( evfPowerTooHigh );
				break;

			case EWMF_SecondaryPump:
				vecFlagToClean.push_back( evfSecPumpHMinTechParam );
				vecFlagToClean.push_back( evfSecPumpNotNeeded );
				break;
		}

		for( auto &iter : vecFlagToClean )
		{
			std::map<CDS_HydroMod::eValidityFlags, CString>::iterator iterErrorWarning = m_mapErrorMessages.find( (CDS_HydroMod::eValidityFlags)iter );

			if( iterErrorWarning != m_mapErrorMessages.end() )
			{
				m_mapErrorMessages.erase( iterErrorWarning );
			}
		}
	}
}

CDS_HydroMod *CDS_HydroMod::FindIndexCircuit()
{
	if( false == this->IsaModule() )
	{
		return NULL;
	}

	// Scan all children.
	CDS_HydroMod *pHM = NULL;
	bool bFound = false;

	for( IDPTR idptr = GetFirst(); NULL != idptr.MP && bFound == false; idptr = GetNext( idptr.MP ) )
	{
		pHM = dynamic_cast<CDS_HydroMod *>( idptr.MP );

		if( NULL == pHM )
		{
			continue;
		}

		bFound = pHM->IsCircuitIndex();
	}

	if( false == bFound )
	{
		return NULL;
	}

	if( true == pHM->IsaModule() && NULL != pHM->GetpSchcat() && false == pHM->GetpSchcat()->IsActiveCircuit() )
	{
		pHM = pHM->FindIndexCircuit();
	}

	return pHM;
}

CWaterChar *CDS_HydroMod::GetpWaterChar( CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	CWaterChar *pclWaterChar = NULL;

	if( CAnchorPt::PipeType_Distribution == ePipeType )
	{
		if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
		{
			if( NULL == m_pDistrSupplyPipe )
			{
				ASSERTA_RETURN( NULL );
			}

			pclWaterChar = m_pDistrSupplyPipe->GetpWaterChar( CAnchorPt::PipeLocation_Supply );
		}
		else
		{
			if( ReturnType::Direct == GetReturnType() )
			{
				if( NULL == m_pDistrSupplyPipe )
				{
					ASSERTA_RETURN( NULL );
				}

				// In case of direct return mode, the return temperature is also in the distribution supply pipe.
				pclWaterChar = m_pDistrSupplyPipe->GetpWaterChar( CAnchorPt::PipeLocation_Return );
			}
			else
			{
				if( NULL == m_pDistrReturnPipe )
				{
					ASSERTA_RETURN( NULL );
				}

				// In case of reverse return mode, the return temperature is in the distribution return pipe.
				pclWaterChar = m_pDistrReturnPipe->GetpWaterChar( CAnchorPt::PipeLocation_Return );
			}
		}
	}
	else if( CAnchorPt::PipeType_Circuit == ePipeType )
	{
		if( CAnchorPt::CircuitSide_Primary == eCircuitSide )
		{
			if( CAnchorPt::PipeLocation::PipeLocation_Supply == ePipeLocation || CAnchorPt::PipeLocation::PipeLocation_Return == ePipeLocation )
			{
				if( NULL == m_pCircuitPrimaryPipe )
				{
					ASSERTA_RETURN( NULL );
				}

				pclWaterChar = m_pCircuitPrimaryPipe->GetpWaterChar( ePipeLocation );
			}
			else if( CAnchorPt::PipeLocation::PipeLocation_ByPass == ePipeLocation )
			{
				if( NULL == m_pCircuitBypassPipe )
				{
					ASSERTA_RETURN( NULL );
				}

				pclWaterChar = m_pCircuitBypassPipe->GetpWaterChar( ePipeLocation );
			}
		}
		else
		{
			// Secondary is threated in the 'CDS_HmInj'. 
			// We must not arrive here in this case!
			ASSERTA_RETURN( NULL );
		}
	}

	return pclWaterChar;
}

CWaterChar *CDS_HydroMod::GetpWaterChar( eHMObj eProductLocation )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	CDB_CircuitScheme *pclCircuitSheme = GetpSch();

	if( NULL == pclCircuitSheme )
	{
		ASSERTA_RETURN( NULL );
	}

	CAnchorPt::AnchorPtFunc *pAnchorPtFunc = NULL;

	switch( eProductLocation )
	{
		case eCircuitPrimaryPipe:
		case eCircuitSecondaryPipe:
		case eDistributionSupplyPipe:
		case eCircuitBypassPrimaryPipe:
			// This method can not be called for pipes. For pipe use the approppriate 'GetpWaterChar' method.
			ASSERT( 0 );
			break;

		case eBVprim:
			pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::BV_P );
			break;

		case eBVbyp:
			pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::BV_Byp );
			break;

		case eBVsec:
			pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::BV_S );
			break;

		case eDpC:
			pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::DpC );
			break;

		case eCV:

			if( true == pclCircuitSheme->IsAnchorPtExist( CAnchorPt::eFunc::PICV ) )
			{
				pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::PICV );
			}
			else if( true == pclCircuitSheme->IsAnchorPtExist( CAnchorPt::eFunc::DPCBCV ) )
			{
				pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::DPCBCV );
			}
			else if( true == pclCircuitSheme->IsAnchorPtExist( CAnchorPt::eFunc::ControlValve ) )
			{
				pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::ControlValve );
			}

			break;

		case eShutoffValveSupply:
		case eShutoffValveReturn:
			pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::ShutoffValve );
			break;

		case eDistributionReturnPipe:
			// This method can not be called for pipes. For pipe use the approppriate 'GetpWaterChar' method.
			ASSERT( 0 );
			break;

		case eSmartControlValve:
			pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::SmartControlValve );
			break;

		case eSmartDpC:
			pAnchorPtFunc = pclCircuitSheme->GetAnchorPtDetails( CAnchorPt::eFunc::SmartDpC );
			break;
	}

	if( NULL == pAnchorPtFunc )
	{
		ASSERTA_RETURN( NULL );
	}

	return GetpWaterChar( pAnchorPtFunc->m_ePipeType, pAnchorPtFunc->m_ePipeLocation, pAnchorPtFunc->m_eCircuitSide );
}

bool CDS_HydroMod::GetWaterCharTU( CWaterChar &clWaterCharIN, CWaterChar &clWaterCharOUT )
{
	CWaterChar *pclWaterCharIN = NULL;
	CWaterChar *pclWaterCharOUT = NULL;

	if( false == IsInjectionCircuit() )
	{
		pclWaterCharIN = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
		pclWaterCharOUT = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );
	}
	else
	{
		pclWaterCharIN = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary );
		pclWaterCharOUT = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );
	}

	if( NULL == pclWaterCharIN || NULL == pclWaterCharOUT )
	{
		ASSERTA_RETURN( false );
	}

	clWaterCharIN = *pclWaterCharIN;
	clWaterCharOUT = *pclWaterCharOUT;

	return true;
}

bool CDS_HydroMod::GetDesignWaterChar( CWaterChar &clWaterChar, CDS_HydroMod *pclDesignTemperatureInterfaceHM, CAnchorPt::PipeLocation ePipeLocation )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.

	if( NULL == pclDesignTemperatureInterfaceHM || false == pclDesignTemperatureInterfaceHM->IsDesignTemperatureInterface() )
	{
		// If design temperature interface is not given as argument (Or if pointer is not a design temperature), we search 
		// what is the design temperature interface that drives the water characteristic of the circuit pipes at the 
		// primary side of the current hydraulic circuit.
		pclDesignTemperatureInterfaceHM = GetpDesignTemperatureInterface();
	}

	// We have two cases:
	//	(1) Design temperature interface is an injection: the water characteristic and temperature are the DESIGN ones.
	//	(2) Design temperatureinterface is the root or NULL: the water characteristic and temperature are those defined in the ribbon.

	double dTemperature = -273.15;

	// Remark: the 'IsInjectionCircuit' does not consider 3-way mixing circuit as injection.
	if( NULL != pclDesignTemperatureInterfaceHM && true == pclDesignTemperatureInterfaceHM->IsInjectionCircuit() )
	{
		CWaterChar *pclWaterChar = pclDesignTemperatureInterfaceHM->GetpOutWaterChar( ePipeLocation );

		if( NULL == pclWaterChar )
		{
			ASSERTA_RETURN( false );
		}

		clWaterChar = *pclWaterChar;

		CDS_HmInj *pclPressureInterfaceHMInj = dynamic_cast<CDS_HmInj *>( pclDesignTemperatureInterfaceHM );

		if( NULL == pclPressureInterfaceHMInj )
		{
			ASSERTA_RETURN( false );
		}

		// Case (1): the design temperature interface is an injection circuit.
		if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
		{
			dTemperature = pclPressureInterfaceHMInj->GetDesignCircuitSupplySecondaryTemperature();
		}
		else
		{
			dTemperature = pclPressureInterfaceHMInj->GetDesignCircuitReturnSecondaryTemperature();
		}
	}
	else
	{
		if( NULL == TASApp.GetpTADS()->GetpWCForProject() || NULL == TASApp.GetpTADS()->GetpWCForProject()->GetpWCData() )
		{
			ASSERTA_RETURN( false );
		}
		
		clWaterChar = *TASApp.GetpTADS()->GetpWCForProject()->GetpWCData();

		if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
		{
			dTemperature = TASApp.GetpTADS()->GetpTechParams()->GetDefaultPrjTps();
		}
		else
		{
			dTemperature = TASApp.GetpTADS()->GetpTechParams()->GetDefaultPrjTpr();
		}
	}

	if( -273.15 == dTemperature )
	{
		ASSERTA_RETURN( false );
	}

	clWaterChar.UpdateFluidData( dTemperature );
	return true;
}

CWaterChar *CDS_HydroMod::GetpInWaterChar( CAnchorPt::PipeLocation ePipeLocation )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	CWaterChar *pclWaterChar = NULL;

	if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
	{
		// The input supply water characteristic is considered to be the water characteristic of the fluid arriving in the
		// supply pipe of the circuit on the primary side of the circuit (That is the same as the water characteristic in the
		// distribution supply pipe).
		// See the "How are managed temperatures in pipes.docx" document in the "Doc" folder of the solution for more details.
		pclWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
	}
	else
	{
		// The input return water characteristic is considered to be the water characteristic of the fluid leaving the
		// return pipe of the circuit on the primary side of the circuit.
		// See the "How are managed temperatures in pipes.docx" document in the "Doc" folder of the solution for more details.
		pclWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );
	}

	return pclWaterChar;
}

CWaterChar *CDS_HydroMod::GetpOutWaterChar( CAnchorPt::PipeLocation ePipeLocation )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	CWaterChar *pclWaterChar = NULL;

	if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
	{
		// This method is called by a children to know what is the output supply water characteristic leaving out
		// the current hydraulic circuit.
		// If this method is called it's because we ARE NOT with an injection circuit.
		// In this case we are thus with a module WITHOUT secondary side.
		// So, the output supply water characteristic is considered to be the water characteristic of the fluid leaving the
		// supply pipe of the circuit at the primary side.
		pclWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
	}
	else
	{
		// This method is called by a children to know what is the output return water characteristic arriving in
		// the current hydraulic circuit.
		// If this method is called it's because we ARE NOT with an injection circuit.
		// In this case we are thus with a module WITHOUT secondary side.
		// So, the output return water characteristic is considered to be the water characteristic of the fluid arriving in the
		// return pipe of the circuit at the secondary side.
		pclWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );
	}

	return pclWaterChar;
}

double CDS_HydroMod::GetTemperature( CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	double dTemperature = -273.15;

	if( CAnchorPt::PipeType_Distribution == ePipeType )
	{
		if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
		{
			if( NULL == m_pDistrSupplyPipe )
			{
				ASSERTA_RETURN( -273.15 );
			}

			dTemperature = m_pDistrSupplyPipe->GetTemperature( CAnchorPt::PipeLocation_Supply );
		}
		else
		{
			if( ReturnType::Direct == GetReturnType() )
			{
				if( NULL == m_pDistrSupplyPipe )
				{
					ASSERTA_RETURN( -273.15 );
				}

				// In case of direct return mode, the return temperature is also in the distribution supply pipe.
				dTemperature = m_pDistrSupplyPipe->GetTemperature( CAnchorPt::PipeLocation_Return );
			}
			else
			{
				if( NULL == m_pDistrReturnPipe )
				{
					ASSERTA_RETURN( -273.15 );
				}

				// In case of reverse return mode, the return temperature is in the distribution return pipe.
				dTemperature = m_pDistrReturnPipe->GetTemperature( CAnchorPt::PipeLocation_Return );
			}
		}
	}
	else if( CAnchorPt::PipeType_Circuit == ePipeType )
	{
		if( CAnchorPt::CircuitSide_Primary == eCircuitSide )
		{
			if( CAnchorPt::PipeLocation::PipeLocation_Supply == ePipeLocation || CAnchorPt::PipeLocation::PipeLocation_Return == ePipeLocation )
			{
				if( NULL == m_pCircuitPrimaryPipe )
				{
					ASSERTA_RETURN( -273.15 );
				}

				dTemperature = m_pCircuitPrimaryPipe->GetTemperature( ePipeLocation );
			}
			else if( CAnchorPt::PipeLocation::PipeLocation_ByPass == ePipeLocation )
			{
				if( NULL == m_pCircuitBypassPipe )
				{
					ASSERTA_RETURN( -273.15 );
				}

				dTemperature = m_pCircuitBypassPipe->GetTemperature( ePipeLocation );
			}
		}
		else if( CAnchorPt::CircuitSide_Secondary == eCircuitSide )
		{
			// Secondary is threated in the 'CDS_HmInj'. 
			// We must not arrive here in this case!
			ASSERTA_RETURN( NULL );
		}
	}

	return dTemperature;
}

bool CDS_HydroMod::GetDesignTemperature( double &dTemperature, CDS_HydroMod *pclDesignTemperatureInterfaceHM, CAnchorPt::PipeLocation ePipeLocation )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.

	if( NULL == pclDesignTemperatureInterfaceHM || false == pclDesignTemperatureInterfaceHM->IsDesignTemperatureInterface() )
	{
		// If design temperature interface is not given as argument (Or if pointer is not a design temperature), we search 
		// what is the design temperature interface that drives the water characteristic of the circuit pipes at the 
		// primary side of the current hydraulic circuit.
		pclDesignTemperatureInterfaceHM = GetpDesignTemperatureInterface();
	}

	// We have two cases:
	//	(1) Design temperature interface is an injection: the water characteristic and temperature are the DESIGN ones.
	//	(2) Design temperature interface is the root or NULL: the water characteristic and temperature are those defined in the ribbon.

	dTemperature = -273.15;

	// Remark: the 'IsInjectionCircuit' does not consider 3-way mixing circuit as injection.
	if( NULL != pclDesignTemperatureInterfaceHM && true == pclDesignTemperatureInterfaceHM->IsInjectionCircuit() )
	{
		// Case (1): the design temperature interface is an injection circuit.
		if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
		{
			dTemperature = pclDesignTemperatureInterfaceHM->GetDesignCircuitSupplySecondaryTemperature();
		}
		else
		{
			dTemperature = pclDesignTemperatureInterfaceHM->GetDesignCircuitReturnSecondaryTemperature();
		}
	}
	else
	{
		if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
		{
			dTemperature = TASApp.GetpTADS()->GetpTechParams()->GetDefaultPrjTps();
		}
		else
		{
			dTemperature = TASApp.GetpTADS()->GetpTechParams()->GetDefaultPrjTpr();
		}
	}

	if( -273.15 == dTemperature )
	{
		ASSERTA_RETURN( false );
	}

	return true;
}

double CDS_HydroMod::GetInTemperature( CAnchorPt::PipeLocation ePipeLocation )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	double dInTemperature = -273.15;

	if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
	{
		// The input supply temperature is considered to be the temperature of the fluid arriving in the
		// supply pipe of the circuit on the primary side of the circuit (tcsp that is the same as the tds).
		// See the "How are managed temperatures in pipes.docx" document in the "Doc" folder of the solution for more details.
		dInTemperature = GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
	}
	else
	{
		// The return temperature is considered to be the temperature of the fluid leaving the
		// return pipe of the circuit on the primary side of the circuit (tcrp).
		// See the "How are managed temperatures in pipes.docx" document in the "Doc" folder of the solution for more details.
		dInTemperature = GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );
	}

	return dInTemperature;
}

double CDS_HydroMod::GetOutTemperature( CAnchorPt::PipeLocation ePipeLocation )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	double dOutTemperature = -273.15;

	if( CAnchorPt::PipeLocation_Supply == ePipeLocation )
	{
		// This method is called by a children to know what is the output supply temperature leaving out
		// the current hydraulic circuit.
		// If this method is called it's because we ARE NOT with an injection circuit.
		// In this case we are thus with a module WITHOUT secondary side.
		// So, the output supply temperature is considered to be the temperature of the fluid leaving the
		// supply pipe of the circuit at the primary side.
		dOutTemperature = GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
	}
	else
	{
		// This method is called by a children to know what is the output return temperature arriving in
		// the current hydraulic circuit.
		// If this method is called it's because we ARE NOT with an injection circuit.
		// In this case we are thus with a module WITHOUT secondary side.
		// So, the output return temperature is considered to be the temperature of the fluid arriving in the
		// return pipe of the circuit at the primary side.
		// ATTENTION! There is a difference betwen the 'GetOutTemperature' and 'GetDesignTemperature' methods for the return temperature.
		// The first method will return the COMPUTED return temperature. While the second method will return the DESIGN tempeature
		// (The temperature set by the user by example for a secondary side of an injection circuit.
		dOutTemperature = GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );
	}

	return dOutTemperature;
}

double CDS_HydroMod::GetPower( CAnchorPt::CircuitSide eCircuitSide )
{
	if( CAnchorPt::CircuitSide_Primary != eCircuitSide )
	{
		ASSERTA_RETURN( 0.0 );
	}

	if( GetQ() <= 0.0 )
	{
		return 0.0;
	}

	CWaterChar *pclCircuitSupplyPrimaryWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, eCircuitSide );

	if( NULL == pclCircuitSupplyPrimaryWaterChar || 273.15 == pclCircuitSupplyPrimaryWaterChar->GetTemp() )
	{
		ASSERTA_RETURN( 0.0 );
	}

	CWaterChar *pclCircuitReturnPrimaryWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, eCircuitSide );

	if( NULL == pclCircuitReturnPrimaryWaterChar || 273.15 == pclCircuitReturnPrimaryWaterChar->GetTemp() )
	{
		ASSERTA_RETURN( 0.0 );
	}

	double dPower = pclCircuitSupplyPrimaryWaterChar->GetTemp() * pclCircuitSupplyPrimaryWaterChar->GetDens() * pclCircuitSupplyPrimaryWaterChar->GetSpecifHeat();
	dPower -= ( pclCircuitReturnPrimaryWaterChar->GetTemp() * pclCircuitReturnPrimaryWaterChar->GetDens() * pclCircuitReturnPrimaryWaterChar->GetSpecifHeat() );
	dPower *= GetQ();

	return abs( dPower );
}

void CDS_HydroMod::DistributeWaterCharacteristics( CDS_HydroMod *pclDesignTemperatureInterfaceHM )
{
	if( NULL == pclDesignTemperatureInterfaceHM || NULL == TASApp.GetpTADS()->GetpWCForProject() 
			|| NULL == TASApp.GetpTADS()->GetpWCForProject()->GetpWCData() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERT_RETURN;
	}

	// If current circuit is not in HM calc mode, we don't verify.
	if( false == IsHMCalcMode() )
	{
		return;
	}

	// This function runs all the project from the root and for each circuit checks if water characteristics must be updated.
	// For each temperature interface, we check if it must be updated or not. Child circuit supply temperatures are updated accordingly to their
	// temperature interface. Return temperature will be updated when performing the computation.

	if( true == IsaModule() )
	{
		if( NULL == GetFirstHMChild() )
		{
			return;
		}

		// To compute the return temperatures in the distributions pipes we need to do that:
		//		-> Direct return mode: going from the last circuit to the first one -> descending order (false).
		//		-> Reverse return mode: going from the first circuit to the last one -> ascendind order (true).
		bool bAscendingOrder = ( ReturnType::Direct == GetFirstHMChild()->GetReturnType() ) ? false : true;

		std::map<int, CDS_HydroMod *> mapChildrenList;
		GetAllChildren( mapChildrenList, bAscendingOrder );
		CDS_HydroMod *pclPreviousChild = NULL;

		// Project water characteristics.
		CWaterChar clWaterCharProject = *TASApp.GetpTADS()->GetpWCForProject()->GetpWCData();
		int iCount = 0;
		CAnchorPt::PipeType ePipeType;
		CAnchorPt::PipeLocation ePipeLocation;
		CAnchorPt::CircuitSide eCircuitSide;

		// Set water characteristics in each pipe for the current circuit.
		enum
		{
			Loop_circ_prim_supply,
			Loop_circ_sec_supply,
			Loop_circ_prim_return,
			Loop_circ_sec_return,
			Loop_circ_bypass,
			Loop_dist_supply,
			Loop_dist_return,
			Loop_last
		};

		while( iCount < Loop_last )
		{
			// Check on which pipe we have a wrong additive information.
			if( Loop_circ_prim_supply == iCount )
			{
				ePipeType = CAnchorPt::PipeType_Circuit;
				ePipeLocation = CAnchorPt::PipeLocation_Supply;
				eCircuitSide = CAnchorPt::CircuitSide_Primary;
			}
			else if( Loop_circ_sec_supply == iCount )
			{
				if( true == IsInjectionCircuit() )
				{
					ePipeType = CAnchorPt::PipeType_Circuit;
					ePipeLocation = CAnchorPt::PipeLocation_Supply;
					eCircuitSide = CAnchorPt::CircuitSide_Secondary;
				}
				else
				{
					iCount++;
					continue;
				}
			}
			else if( Loop_circ_prim_return == iCount )
			{
				ePipeType = CAnchorPt::PipeType_Circuit;
				ePipeLocation = CAnchorPt::PipeLocation_Return;
				eCircuitSide = CAnchorPt::CircuitSide_Primary;
			}
			else if( Loop_circ_sec_return == iCount )
			{
				if( true == IsInjectionCircuit() )
				{
					ePipeType = CAnchorPt::PipeType_Circuit;
					ePipeLocation = CAnchorPt::PipeLocation_Return;
					eCircuitSide = CAnchorPt::CircuitSide_Secondary;
				}
				else
				{
					iCount++;
					continue;
				}
			}
			else if( Loop_circ_bypass == iCount )
			{
				if( NULL != GetpCircuitBypassPipe() )
				{
					ePipeType = CAnchorPt::PipeType_Circuit;
					ePipeLocation = CAnchorPt::PipeLocation_ByPass;
					eCircuitSide = GetBypassPipeSide();
				}
				else
				{
					iCount++;
					continue;
				}
			}
			else if( Loop_dist_supply == iCount )
			{
				ePipeType = CAnchorPt::PipeType_Distribution;
				ePipeLocation = CAnchorPt::PipeLocation_Supply;
				eCircuitSide = CAnchorPt::CircuitSide_NotApplicable;
			}
			else if( Loop_dist_return == iCount )
			{
				ePipeType = CAnchorPt::PipeType_Distribution;
				ePipeLocation = CAnchorPt::PipeLocation_Supply;
				eCircuitSide = CAnchorPt::CircuitSide_NotApplicable;
			}
			
			iCount++;

			CWaterChar clWaterChar = *( GetpWaterChar( ePipeType, ePipeLocation, eCircuitSide ) );

			// We verify that fluid additive is the same that the project one. We should have the same Additive char. in each pipe.
			// Remark: as soon as we found a bad water characteristic, we correct for all pipes and break the loop !!
			if( ( 0 != StringCompare( clWaterChar.GetAdditFamID(), clWaterCharProject.GetAdditFamID() ) )
					|| ( 0 != StringCompare( clWaterChar.GetAdditID(), clWaterCharProject.GetAdditID() ) )
					|| ( clWaterChar.GetPcWeight() != clWaterCharProject.GetPcWeight() ) )
			{
				// Circuit supply pipe at the primary side and distribution supply pipe take the same water characteristic as the global one for the project.
				// Remark: Temperatures will be set at the right value below.
				SetWaterChar( &clWaterCharProject, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
				SetWaterChar( &clWaterCharProject, CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Supply );

				// When we open a project without unfreeze we do not perform any compute so the return temp. must not have the same value than the supply.
				CWaterChar WCForReturn = clWaterCharProject;

				// Put back return temperature and set water characteristics.
				double dRetTempCirc = GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );
				WCForReturn.UpdateFluidData( dRetTempCirc );
				SetWaterChar( &WCForReturn, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );
				
				double dRetTempDist = GetTemperature( CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );
				WCForReturn.UpdateFluidData( dRetTempDist );
				SetWaterChar( &WCForReturn, CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );

				if( true == IsInjectionCircuit() )
				{
					SetWaterChar( &clWaterCharProject, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary );
					
					// Put back return temperature and set water characteristics.
					double dRetTemp = GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );
					WCForReturn.UpdateFluidData( dRetTemp );
					SetWaterChar( &WCForReturn, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );
				}

				if( NULL != GetpCircuitBypassPipe() )
				{
					// Correct the bypass pipe.
					// Remark: temperatures will be set at the right value below.
					SetWaterChar( &clWaterCharProject, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_ByPass, GetBypassPipeSide() );
				}

				break;
			}
		}

		double dSupplyTempDesign = 0.0;

		GetDesignTemperature( dSupplyTempDesign, pclDesignTemperatureInterfaceHM, CAnchorPt::PipeLocation_Supply );

		double dParentSupplyTemp = 0.0;

		if( NULL != GetParent() )
		{
			// Parent design temperature. Already up to date.
			GetDesignTemperature( dParentSupplyTemp, GetParent(), CAnchorPt::PipeLocation_Supply );
		}
		else
		{
			dParentSupplyTemp = dSupplyTempDesign;
		}

		// for 3-way mixing secondary supply temperature exists even if it is not used. It must be equal to the primary supply. 
		if( CDB_CircSchemeCateg::e3wTypeMixing == GetpSchcat()->Get3WType() )
		{
			// secondary temperature is the same than primary temperature
			SetTemperature( dParentSupplyTemp, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary );
			SetTemperature( dParentSupplyTemp, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
			SetTemperature( dParentSupplyTemp, CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
			
			( (CDS_HmInj *)this )->SetDesignCircuitSupplySecondaryTemperature( dParentSupplyTemp );
		}
		else if( true == IsInjectionCircuit() )
		{
			SetTemperature( dSupplyTempDesign, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary );
			
			( (CDS_HmInj *)pclDesignTemperatureInterfaceHM )->SetDesignCircuitSupplySecondaryTemperature( dSupplyTempDesign );
			SetTemperature( dParentSupplyTemp, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
			SetTemperature( dParentSupplyTemp, CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
		}
		else
		{
			SetTemperature( dSupplyTempDesign, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
			SetTemperature( dSupplyTempDesign, CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Supply );
		}

		// Now we can update bypass pipe if exists.
		if( NULL != GetpCircuitBypassPipe() )
		{
			if( CAnchorPt::CircuitSide::CircuitSide_Primary == GetBypassPipeSide() )
			{
				// If bypass pipe is at the primary side, we take the same water characteristic and temperature of the circuit supply at the primary side.
				double dTemp = GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
				SetTemperature( dTemp, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_ByPass, CAnchorPt::CircuitSide_Primary );
			}
			else if( CAnchorPt::CircuitSide::CircuitSide_Secondary == GetBypassPipeSide() )
			{
				// If bypass pipe is at the secondary side, we take the same water characteristic and temperature of the circuit return at the secondary side.
				double dTemp = GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );
				SetTemperature( dTemp, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_ByPass, CAnchorPt::CircuitSide_Secondary );
			}
		}

		// Scan all children, from root to last circuit.
		for( auto &iterChild : mapChildrenList )
		{
			if( true == iterChild.second->IsaModule() && false == iterChild.second->IsInjectionCircuit() )
			{
				iterChild.second->DistributeWaterCharacteristics( pclDesignTemperatureInterfaceHM );
			}

			if( false == iterChild.second->IsaModule() )
			{
				CWaterChar clWaterCharSupply;
				CWaterChar clWaterCharReturn;
				double dReturnTempCirc = 0.0;
				double dReturnTempDist = 0.0;

				GetDesignWaterChar( clWaterCharSupply, pclDesignTemperatureInterfaceHM, CAnchorPt::PipeLocation::PipeLocation_Supply );
				dReturnTempCirc = iterChild.second->GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );

				iterChild.second->SetWaterChar( &clWaterCharSupply, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
				clWaterCharReturn = clWaterCharSupply;
				clWaterCharReturn.UpdateFluidData( dReturnTempCirc );
				iterChild.second->SetWaterChar( &clWaterCharReturn, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );

				dReturnTempDist = iterChild.second->GetTemperature( CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );

				iterChild.second->SetWaterChar( &clWaterCharSupply, CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Supply );
				clWaterCharReturn.UpdateFluidData( dReturnTempDist );
				iterChild.second->SetWaterChar( &clWaterCharReturn, CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );

				if( true == iterChild.second->IsInjectionCircuit() )
				{
					iterChild.second->SetWaterChar( &clWaterCharSupply, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary );
					clWaterCharReturn.UpdateFluidData( dReturnTempCirc );
					iterChild.second->SetWaterChar( &clWaterCharReturn, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );

					if( CDB_CircSchemeCateg::e3wTypeMixing != GetpSchcat()->Get3WType() )
					{
						GetDesignTemperature( dSupplyTempDesign, iterChild.second, CAnchorPt::PipeLocation_Supply );
						iterChild.second->SetTemperature( dSupplyTempDesign, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary );
					}
					else
					{
						GetDesignTemperature( dSupplyTempDesign, pclDesignTemperatureInterfaceHM, CAnchorPt::PipeLocation_Supply );
						iterChild.second->SetTemperature( dSupplyTempDesign, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary );
						
						// Update m_dDesignCircuitSupplySecondaryTemperature it is used for injection circuit.
						( (CDS_HmInj *)iterChild.second )->SetDesignCircuitSupplySecondaryTemperature( dSupplyTempDesign );
					}
				}

				if( NULL != iterChild.second->GetpCircuitBypassPipe() )
				{
					iterChild.second->SetWaterChar( &clWaterCharSupply, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_ByPass,  iterChild.second->GetBypassPipeSide() );

					double dTempByPass = ( CAnchorPt::CircuitSide::CircuitSide_Primary == iterChild.second->GetBypassPipeSide() ) ? dSupplyTempDesign : dReturnTempDist;
					iterChild.second->SetTemperature( dTempByPass, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_ByPass, iterChild.second->GetBypassPipeSide() );
				}
			}
		}
	}
}

void CDS_HydroMod::VerifyAllWaterCharacteristics( CDS_HydroMod *pclDesignTemperatureInterfaceHM )
{
	if( NULL == pclDesignTemperatureInterfaceHM )
	{
		ASSERT_RETURN;
	}

	// If current circuit is not in HM calc mode, we don't verify.
	if( false == IsHMCalcMode() )
	{
		return;
	}

	if( true == IsaModule() )
	{
		if( NULL == GetFirstHMChild() )
		{
			return;
		}

		// To compute the return temperatures in the distributions pipes we need to do that:
		//		-> Direct return mode: going from the last circuit to the first one -> descending order (false).
		//		-> Reverse return mode: going from the first circuit to the last one -> ascendind order (true).
		bool bAscendingOrder = ( ReturnType::Direct == GetFirstHMChild()->GetReturnType() ) ? false : true;

		std::map<int, CDS_HydroMod *> mapChildrenList;
		GetAllChildren( mapChildrenList, bAscendingOrder );
		CDS_HydroMod *pclPreviousChild = NULL;

		// Scan all children, go deeper into HM struct.
		for( auto &iterChild : mapChildrenList )
		{
			if( true == iterChild.second->IsaModule() && false == iterChild.second->IsInjectionCircuit() )
			{
				iterChild.second->VerifyAllWaterCharacteristics( pclDesignTemperatureInterfaceHM );
			}

			iterChild.second->VerifyWaterCharacteristic( pclDesignTemperatureInterfaceHM, pclPreviousChild );
			pclPreviousChild = iterChild.second;
		}
	}
	
	VerifyWaterCharacteristic( pclDesignTemperatureInterfaceHM, NULL );
}

void CDS_HydroMod::VerifyWaterCharacteristic( CDS_HydroMod *pclDesignTemperatureInterfaceHM, CDS_HydroMod *pclPreviousCircuit )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	// Normally, this method must not be manually called. It must be called only by the 'VerifyAllWaterCharacteristics'
	// in charge to verify a design temperature interface and all its children.

	// The 'CDS_HmInj' is called first if the current circuit is an injection circuit.

	// If current circuit is not in HM calc mode, we don't verify.
	if( false == IsHMCalcMode() )
	{
		return;
	}

	if( NULL == GetpSchcat() )
	{
		ASSERT_RETURN;
	}

	if( NULL == pclDesignTemperatureInterfaceHM )
	{
		pclDesignTemperatureInterfaceHM = GetpDesignTemperatureInterface();
	}

	if( NULL == pclDesignTemperatureInterfaceHM )
	{
		ASSERT_RETURN;
	}

	// We can have 3 cases:
	//  (I)   -> current hydraulic circuit is the root.
	//	(II)  -> current hydraulic circuit is not the design temperature interface.
	//  (III) -> current hydraulic circuit is the design temperature interface.
	// 
	//  (1) -> module.
	//  (2) -> circuit.
	//
	//  (a) -> injection.
	//  (b) -> 3-way mixing or others.
	// Remark: secondary side for 3-way mixing is not managed in the 'CDS_HmInj' inherited class but well here.

	// Take the water characteristic from the general settings of the project.
	if( NULL == TASApp.GetpTADS()->GetpWCForProject() || NULL == TASApp.GetpTADS()->GetpWCForProject()->GetpWCData() )
	{
		ASSERT_RETURN;
	}
		
	CWaterChar clWaterChar;
	CWaterChar clWaterCharProject = *TASApp.GetpTADS()->GetpWCForProject()->GetpWCData();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Circuit supply primary pipe. 
	// (1) (a) 3-way mixing: water characteristic from the design temperature interface (We update also the circuit supply pipe at the secondary side that is the same).
	//     (b) injection: water characteristic from the design temperature interface.
	//     (c) others: water characteristic from the design temperature interface.
	// (2) (a) 3-way mixing: can't happen here.
	//     (b) injection: the primary side will be managed by its design temperature interface parent -> Do nothing here.
	//		   (b') EXCEPT: if the design temperature interface is the root -> Case (3) must thus be the first case !
	//     (c) others: can't happen here.
	// (3) (a) 3-way mixing: water characteristic from the general setting of the project (We update also the circuit supply pipe at the secondary side that is the same).
	//     (b) injection: water characteristic from the general setting of the project.
	// 	   (c) others: water characteristic from the general setting of the project.
	// Remark: For root even if we don't display pipes, distribution and circuit pipes are always created.
	// 
	// Summary: (A) On root -> (3a), (3b), (3c) & (2b'): take the water characteristic from the general settings of the project.
	//			(B) Not on root and design temperature interface is not the current hydraulic circuit.
	//			    -> (1a), (1b) & (1c): take the water characteristic from the design temperature interface.
	// 	        (C) Current hydraulic circuit is the design temperature interface -> (2b): do nothing !
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if( 0 == GetLevel() )
	{
		// Case (A).
		// Retrieve the supply temperature from the general settings of the project.
		clWaterChar = clWaterCharProject;
		double dTemperature = TASApp.GetpTADS()->GetpTechParams()->GetDefaultPrjTps();
		clWaterChar.UpdateFluidData( dTemperature );
	}
	else if( this != pclDesignTemperatureInterfaceHM )
	{
		// Case (B).

		// Retrieve the water characteristic from the output supply of the design temperature interface.
		// We prefer to take the DESIGN even for the circuit because if the design temperature interface is an injection circuit, we are sure
		// that its design water characteristic and temperature are already well set.
		if( false == pclDesignTemperatureInterfaceHM->GetDesignWaterChar( clWaterChar, pclDesignTemperatureInterfaceHM, CAnchorPt::PipeLocation_Supply ) )
		{
			ASSERT_RETURN;
		}
	}

	if( 0 == GetLevel() || this != pclDesignTemperatureInterfaceHM )
	{
		SetWaterChar( &clWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );

		if( CDB_CircSchemeCateg::e3wTypeMixing == GetpSchcat()->Get3WType() )
		{
			// In case of 3-way mixing, the secondary side is managed here and not in the 'CDS_HmInj' inherited class.
			SetWaterChar( &clWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary );
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Circuit return primary pipe. 
	//
	// 	Current hydraulic circuit IS the ROOT.
	// 	(I.1.a) Injection module    -> water characteristic from the circuit return secondary pipe.
	//  (I.1.b) Other module        -> water characteristic from first child or from the general setting of the project.
	// 	(I.2)   Circuit             -> Can't happen !!
	// 
	//  Current hydraulic circuit IS NOT the DESIGN TEMPERATURE INTERFACE.
	//  (II.1.a) Injection module   -> water characteristic from the circuit return secondary pipe.
	//  (II.1.b) Other module       -> water characteristic from first child or from the design temperature interface.
	// 	(II.2.a) Injection circuit  -> water characteristic from the circuit return secondary pipe.
	//  (II.2.b) Other circuit      -> water characteristic from the design temperature interface.
	// 	                                       OR if current hydraulic circuit is a circuit (With terminal unit) and the user defines the 
	//                                         terminal flow in Power/DT. In this case we must consider that the return temperature on the primary pipe 
	//                                         is the supply temperature minus (or plus) DT of this terminal unit !!
	// 
	// 	Current hydraulic circuit IS the DESIGN TEMPERATURE INTERFACE.
	// 	(III.1.a) Injection module  -> the primary side will be managed by its design temperature interface parent -> Do nothing here.
    //  (III.1.b) Other module      -> can't happen !!
	// 
	// Summary: (A) Injection: (1) -> (I.1.a), (II.1.a) and (II.2.a) -> take water characteristic from the circuit return secondary pipe.
	// 	                       (2) -> (III.1.a) -> do nothing !
	//          (B) Other root -> (I.1.b): take water from first child or from the general setting of the project.
	//          (C) Other module -> (II.1.b): take water characteristic from first child or from the design temperature interface.
	//          (D) Other circuit -> (II.2.b): take water characteristic from the design temperature interface or Power/DT.
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Remark: the 'IsInjectionCircuit' doesn't return 3-way mixing circuit !
	if( true == IsInjectionCircuit() )
	{
		if( NULL != pclDesignTemperatureInterfaceHM )
		{
			// Case (A).
			// Retrieve the water characteristic from the circuit return pipe at the secondary side.
			CWaterChar *pclWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );
			
			if( NULL == pclWaterChar )
			{
				ASSERT_RETURN;
			}

			SetWaterChar( pclWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );
		}
	}
	else
	{
		// HYS-1882: To update DT when return temp. change via parent.
		double dOldDT = 0.0;

		if( 0 == GetLevel() )
		{
			// Case (B).
			CDS_HydroMod *pclFirstChildHM = GetFirstHMChild();

			if( NULL != pclFirstChildHM )
			{
				// If a child exist, we take the water characteristic from its distribution return pipe.
				CWaterChar *pclWaterChar = pclFirstChildHM->GetpWaterChar( CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );
			
				if( NULL == pclWaterChar )
				{
					ASSERT_RETURN;
				}

				clWaterChar = *pclWaterChar;
			}
			else
			{
				// If no child, then we take the water characteristic from the general settings.
				clWaterChar = clWaterCharProject;
				double dTemperature = TASApp.GetpTADS()->GetpTechParams()->GetDefaultPrjTpr();
				clWaterChar.UpdateFluidData( dTemperature );
			}
		}
		else
		{
			if( true == IsaModule() )
			{
				// Case (C).
				CDS_HydroMod *pclFirstChildHM = GetFirstHMChild();

				if( NULL != pclFirstChildHM )
				{
					// If a child exist, we take the water characteristic from its distribution return pipe.
					CWaterChar *pclWaterChar = pclFirstChildHM->GetpWaterChar( CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );
			
					if( NULL == pclWaterChar )
					{
						ASSERT_RETURN;
					}

					clWaterChar = *pclWaterChar;
				}
				else
				{
					// If no child, then we take the water characteristic from the design temperature interface.
					if( false == GetDesignWaterChar( clWaterChar, pclDesignTemperatureInterfaceHM, CAnchorPt::PipeLocation_Return ) )
					{
						ASSERT_RETURN;
					}
				}
			}
			else
			{
				// Case (D).
				// HYS-1882: We can now have a DT field when we are in flow mode. 
				if( CTermUnit::_QType::PdT != GetpTermUnit()->GetQType() && false == IsDTFieldDisplayedInFlowMode() )
				{
					// The terminal unit is not designed with Power/DT or flow/DT.
					// Retrieve the water characteristic from the output supply of the design temperature interface.
					if( false == GetDesignWaterChar( clWaterChar, pclDesignTemperatureInterfaceHM, CAnchorPt::PipeLocation_Return ) )
					{
						ASSERT_RETURN;
					}
				}
				else
				{
					// It's a circuit with a terminal unit for which the flow is defined with Power/DT.
					// HYS-1882: Or a circuit with flow and DT.
					double dDT = (false == IsDTFieldDisplayedInFlowMode()) ? GetpTermUnit()->GetDT() : GetpTermUnit()->GetDTFlowMode();
					if( dDT > 0 )
					{
						CWaterChar* pclWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );

						if( NULL == pclWaterChar )
						{
							ASSERT_RETURN;
						}

						clWaterChar = *pclWaterChar;

						if( ProjectType::Heating == TASApp.GetpTADS()->GetpTechParams()->GetProjectApplicationType() )
						{
							clWaterChar.UpdateFluidData( clWaterChar.GetTemp() - dDT );
						}
						else
						{
							clWaterChar.UpdateFluidData( clWaterChar.GetTemp() + dDT );
						}
					}
					else
					{
						CWaterChar* pclWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );

						if( NULL == pclWaterChar )
						{
							ASSERT_RETURN;
						}

						clWaterChar = *pclWaterChar;
					}
				}
			}
		}

		SetWaterChar( &clWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );

		// HYS-1882: Update DT if its change because of new parent. If the parent change only return temp is updated we have to update DT
		// If user move a circuit with flow and DT under a parent with different regime, we don't see the DT field but the value does not change. We can see that by changing Qtype to PDT.
		// We got the same issue when changing DT from the ribbon ( the return temp. of the simple circuit without regime temp. change ). If we switch in PDT mode we see old DT.
		if( false == IsaModule() && false == IsInjectionCircuit() )
		{
			// verify DT
			if( GetpTermUnit()->GetQType() == CTermUnit::_QType::PdT )
			{
				dOldDT = GetpTermUnit()->GetDT();
			}
			else
			{
				dOldDT = GetpTermUnit()->GetDTFlowMode();
			}

			double dNewDT = 0.0;
			CWaterChar* pclTempWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
			if( ProjectType::Heating == TASApp.GetpTADS()->GetpTechParams()->GetProjectApplicationType() )
			{
				dNewDT = pclTempWaterChar->GetTemp() - clWaterChar.GetTemp();
			}
			else
			{
				dNewDT = clWaterChar.GetTemp() - pclTempWaterChar->GetTemp();
			}

			if( dNewDT != dOldDT && dNewDT != 0.0 )
			{
				// update DT
				GetpTermUnit()->UpdateDT( dNewDT );
			}
		}

		// HYS-1882: Update DT if its change because of new parent. If the parent change only return temp is updated we have to update DT
		// If user move a circuit with flow and DT under a parent with different regime, we don't see the DT field but the value does not change. We can see that by changing Qtype to PDT.
		// We got the same issue when changing DT from the ribbon ( the return temp. of the simple circuit without regime temp. change ). If we switch in PDT mode we see old DT.
		if( false == IsaModule() && false == IsInjectionCircuit() )
		{
			// verify DT
			if( GetpTermUnit()->GetQType() == CTermUnit::_QType::PdT )
			{
				dOldDT = GetpTermUnit()->GetDT();
			}
			else
			{
				dOldDT = GetpTermUnit()->GetDTFlowMode();
			}

			double dNewDT = 0.0;
			CWaterChar *pclTempWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
			
			if( ProjectType::Heating == TASApp.GetpTADS()->GetpTechParams()->GetProjectApplicationType() )
			{
				dNewDT = pclTempWaterChar->GetTemp() - clWaterChar.GetTemp();
			}
			else
			{
				dNewDT = clWaterChar.GetTemp() - pclTempWaterChar->GetTemp();
			}

			if( dNewDT != dOldDT && dNewDT != 0.0 )
			{
				// update DT
				GetpTermUnit()->UpdateDT( dNewDT );
			}
		}

		if( CDB_CircSchemeCateg::e3wTypeMixing == GetpSchcat()->Get3WType() )
		{
			// In case of 3-way mixing, the secondary side is managed here and not in the 'CDS_HmInj' inherited class.
			SetWaterChar( &clWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );
		}
	}

	// We don't call the 'SetWaterChar' here to exclude these cases: (I.2), (III.1.a) and (III.1.B).


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Bypass pipe.
	// We can have here two cases: either bypass pipe is belonging to the primary or to the secondary side.
	// If we are on an injection circuit, the "CDS_HmInj::VerifyWatgerCharacteristic" has been already called, so the water characteristics for the pipe on the 
	// secondary side are already verified.
	// And we have verified water characteristics for the pipe on the primary side just above.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if( NULL != GetpCircuitBypassPipe() )
	{
		if( CAnchorPt::CircuitSide::CircuitSide_Primary == GetBypassPipeSide() )
		{
			// If bypass pipe is at the primary side, we take the same water characteristic and temperature of the circuit supply at the primary side.
			CWaterChar *pclTempWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
			SetWaterChar( pclTempWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_ByPass, CAnchorPt::CircuitSide_Primary );
		}
		else if( CAnchorPt::CircuitSide::CircuitSide_Secondary == GetBypassPipeSide() )
		{
			// If bypass pipe is at the secondary side, we take the same water characteristic and temperature of the circuit return at the secondary side.
			CWaterChar *pclTempWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );
			SetWaterChar( pclTempWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_ByPass, CAnchorPt::CircuitSide_Secondary );
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Distribution supply pipe.
	// (1) (a) 3-way mixing: same as the circuit supply at the primary side.
	//     (b) injection: same as the circuit supply at the primary side.
	//     (c) others: same as the circuit supply at the primary side.
	// (2) (a) 3-way mixing: can't happen here.
	//     (b) injection: the primary side will be managed by its design temperature interface parent -> Do nothing here.
	//     (c) others: can't happen here.
	// (3) (a) 3-way mixing: same as the circuit supply at the primary side.
	//     (b) injection: same as the circuit supply at the primary side.
	//     (c) others: same as the circuit supply at the primary side.
	// Remark: For root even if we don't display pipes, distribution and circuit pipes are always created.
	// 
	// Summary: (A) All cases except (2b) are the same as the circuit supply at the primary side.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if( this != pclDesignTemperatureInterfaceHM || 0 == GetLevel() )
	{
		// Case (A).
		// Retrieve the water characteristic from the circuit supply pipe at the primary side.
		CWaterChar *pclWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
			
		if( NULL == pclWaterChar )
		{
			ASSERT_RETURN;
		}

		SetWaterChar( pclWaterChar, CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Supply );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Distribution return pipe.
	// (1) (a) 3-way mixing: next hydraulic circuit exists:
	//			-> no : same as the circuit return primary side.
	// 			-> yes: do the mix.
	//     (b) injection: next hydraulic circuit exists:
	//			-> no : same as the circuit return primary side.
	// 			-> yes: do the mix.
	//     (c) others: next hydraulic circuit exists:
	//			-> no : same as the circuit return primary side.
	// 			-> yes: do the mix.
	// (2) (a) 3-way mixing: can't happen here.
	//     (b) injection: the primary side will be managed by its design temperature interface parent -> Do nothing here
	//     (c) others: can't happen here.
	// (3) (a) 3-way mixing: same as the circuit return primary side.
	//     (b) injection: same as the circuit return primary side.
	//     (c) others: same as the circuit return primary side.
	// Remark: On root, there is no next circuit.
	// Remark: For root even if we don't display pipes, distribution and circuit pipes are always created.
	// 
	// Summary: (A) next hydraulic circuit does not exist: same as the circuit return pipe at the primary side.
	// 	        (B) next hydraulic circuit does exist    : do a mix.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if( this != pclDesignTemperatureInterfaceHM || 0 == GetLevel() )
	{
		CWaterChar *pclCircuitReturnPrimaryWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );

		if( NULL == pclCircuitReturnPrimaryWaterChar )
		{
			ASSERT_RETURN;
		}

		// Case (A).
		SetWaterChar( pclCircuitReturnPrimaryWaterChar, CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );

		if( NULL != pclPreviousCircuit )
		{
			// Case (B).

			// Retrieve information for the current distribution return pipe.
			CWaterChar *pclCurrentDistributionReturnWaterChar = GetpWaterChar( CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );

			if( NULL == pclCurrentDistributionReturnWaterChar )
			{
				ASSERT_RETURN;
			}

			double dCurrentDistributionReturnFlow = GetQ( CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );
			double dCurrentDistributionReturnRho = pclCurrentDistributionReturnWaterChar->GetDens();
			double dCurrentDistributionReturnCp = pclCurrentDistributionReturnWaterChar->GetSpecifHeat();

			// Retrieve information for the previous distribution return pipe.
			CWaterChar *pclPreviousDistributionReturnWaterChar = pclPreviousCircuit->GetpWaterChar( CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );

			if( NULL == pclPreviousDistributionReturnWaterChar )
			{
				ASSERT_RETURN;
			}

			double dPreviousDistributionReturnFlow = pclPreviousCircuit->GetQ( CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );
			double dPreviousDistributionReturnRho = pclPreviousDistributionReturnWaterChar->GetDens();
			double dPreviousDistributionReturnCp = pclPreviousDistributionReturnWaterChar->GetSpecifHeat();
			double dPreviousDistributionReturnTemperature = pclPreviousDistributionReturnWaterChar->GetTemp();

			// Retrieve information for the current circuit return at the primary side.
			double dCircuitReturnPrimaryFlow = GetQ( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );
			double dCircuitReturnPrimaryRho = pclCircuitReturnPrimaryWaterChar->GetDens();
			double dCircuitReturnPrimaryCp = pclCircuitReturnPrimaryWaterChar->GetSpecifHeat();
			double dCircuitReturnPrimaryTemperature = pclCircuitReturnPrimaryWaterChar->GetTemp();

			// HYS-1882: It's possible to get zero flow when we have pending circuit or a module without children.
			// We remove conditions 0.0 != dPreviousDistributionReturnFlow and 0.0 != dCircuitReturnPrimaryFlow
			if( -273.15 != dPreviousDistributionReturnTemperature && 
				-273.15 != dCircuitReturnPrimaryTemperature && 0.0 != dCurrentDistributionReturnFlow )
			{
				// See the "Return temperature calculation.docx" document in the "Doc" folder of the "HySelect" repository for more details.
				double dDistributionReturnTemperature = ( dCircuitReturnPrimaryFlow * dCircuitReturnPrimaryRho * dCircuitReturnPrimaryCp * dCircuitReturnPrimaryTemperature );
				dDistributionReturnTemperature += ( dPreviousDistributionReturnFlow * dPreviousDistributionReturnRho * dPreviousDistributionReturnCp * dPreviousDistributionReturnTemperature );

				double dDenominator = dCircuitReturnPrimaryFlow * dCircuitReturnPrimaryRho * dCircuitReturnPrimaryCp;
				dDenominator += dPreviousDistributionReturnFlow * dPreviousDistributionReturnRho * dPreviousDistributionReturnCp;
				
				if( 0 != dDenominator )
				{
					dDistributionReturnTemperature /= dDenominator;
					SetTemperature( dDistributionReturnTemperature, CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );
				}
			}
		}
	}
}

double CDS_HydroMod::GetTechParamCVMinDpProp()
{
	return GetpTechParam()->GetCVMinDpProp();
}

CDS_HydroMod *CDS_HydroMod::GetFirstHMChild()
{
	/// For a module return pointer on child at position 1
	if( true == IsaModule() )
	{
		for( IDPTR idptr = GetFirst(); NULL != idptr.MP; idptr = GetNext( idptr.MP ) )
		{
			CDS_HydroMod *pHMX = dynamic_cast<CDS_HydroMod *>( idptr.MP );

			if( 1 == pHMX->GetPos() )
			{
				return pHMX;
			}
		}
	}

	return NULL;
}

void CDS_HydroMod::GetAllChildren( std::map<int, CDS_HydroMod *> &mapChildrenList, bool bAscendingOrder )
{
	mapChildrenList.clear();

	if( false == IsaModule() )
	{
		return;
	}

	for( IDPTR ChildIDPtr = GetFirst(); NULL != ChildIDPtr.MP; ChildIDPtr = GetNext( ChildIDPtr.MP ) )
	{
		CDS_HydroMod *pclChildHM = dynamic_cast<CDS_HydroMod *>( ChildIDPtr.MP );

		if( NULL == pclChildHM )
		{
			ASSERT_CONTINUE;
		}

		int iKey = ( true == bAscendingOrder ) ? pclChildHM->GetPos() : 100000 - pclChildHM->GetPos();
		mapChildrenList.insert( std::pair<int, CDS_HydroMod *>( iKey, pclChildHM ) );
	}
}

/**
*   @brief  Fix circuit pipe or secondary pipe to the same size that distribution pipe of first child when it's locked
*			SCM 2013-08-07 - 16:08 - Alen
*   @param  pPipe pointer to circuit or secondary pipe
*/
void CDS_HydroMod::VerifyModuleCircuitPipe( CPipes *pPipe )
{
	if( true == IsaModule() )
	{
		if( NULL != pPipe )
		{
			pPipe->SetLength( 0 );
			pPipe->RemoveAllSingularities();
			/// AL - 2018-03-29 block re-compute HM
			pPipe->SetLock( false, true );

			CDS_HydroMod *pFirstChild = GetFirstHMChild();

			if( NULL != pFirstChild && NULL != pFirstChild->GetpDistrSupplyPipe() )
			{
				if( true == pFirstChild->GetpDistrSupplyPipe()->IsLocked() )
				{
					/// AL - 2018-03-29 block re-compute HM
					pPipe->SetLock( true, false );
					pPipe->SetIDPtr( pFirstChild->GetpDistrSupplyPipe()->GetIDPtr(), false );
				}
			}
		}
	}
}

double CDS_HydroMod::GetTotQwoDiversity()
{
	return GetpTermUnit()->GetTotQwoDiversity();
}

double CDS_HydroMod::GetPartialPendingQ()
{
	return GetpTermUnit()->GetPartialPendingQ();
}

void CDS_HydroMod::SetFlagCircuitIndex( bool fIsCircuitIndex )
{
	if( m_bIsCircuitIndex != fIsCircuitIndex )
	{
		m_bIsCircuitIndex = fIsCircuitIndex;
		Modified();
	}
}

void CDS_HydroMod::_TakeSnapShot( CDS_HydroMod *pHM, CString str, int iDeep )
{
	if( NULL == pHM )
	{
		return;
	}

	for( IDPTR IDPtr = pHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pHM->GetNext( IDPtr.MP ) )
	{
		CDS_HydroMod *pcHM = static_cast<CDS_HydroMod *>( IDPtr.MP );

		CString strOutput;
		strOutput.Format( _T("%s\tHMName: %s\tHNeeded: %f\tHAvail: %f\n"), str, pcHM->GetHMName(), pcHM->GetHminForUserDisplay(), pcHM->GetHAvail() );
		OutputDebugString( strOutput );

		if( true == pcHM->IsaModule() )
		{
			++iDeep;
			_TakeSnapShot( pcHM, str, iDeep );
		}
	}

	--iDeep;

	if( 0 == iDeep )
	{
		OutputDebugString( _T("\n") );
	}
}

CDS_HydroMod *CDS_HydroMod::_GetpPressIntHMForTemperature()
{
	CDS_HydroMod *pHM = this;

	// Stop when main table or injection circuit (That is not a 3-way mixing circuit) is reached.
	while( NULL != pHM )
	{
		// Stop when we reach secondary of an injection circuit.
		// The 'IsInjectionCircuit' will return 'false' if the circuit is a 3-way mixing circuit.
		if( pHM != this && true == pHM->IsInjectionCircuit() )
		{
			break;
		}

		// Stop when main table reached.
		if( true == pHM->GetIDPtr().PP->IsClass( CLASS( CTableHM ) ) || true == pHM->GetIDPtr().PP->IsClass( CLASS( CTable ) ) )
		{
			break;
		}

		// Take parent.
		pHM = dynamic_cast<CDS_HydroMod *>( pHM->GetIDPtr().PP );
	}

	if( NULL == pHM )
	{
		ASSERTA_RETURN( this );
	}

	return pHM;
}

void CDS_HydroMod::_ProcessCircuitPipes()
{
	if( NULL == GetpCircuitPrimaryPipe() )
	{
		return;
	}

	if( NULL != GetpSchcat() && false == GetpSchcat()->IsSecondarySideExist() )
	{
		VerifyModuleCircuitPipe( GetpCircuitPrimaryPipe() );
	}

	double dQ = GetpCircuitPrimaryPipe()->GetRealQ();

	IDPTR idptr = GetpCircuitPrimaryPipe()->SelectBestPipe( dQ );

	if( false == GetpCircuitPrimaryPipe()->IsLocked() )
	{
		GetpCircuitPrimaryPipe()->SetIDPtr( idptr, false );
	}
}

void CDS_HydroMod::_ProcessBypassPipe()
{
	if( NULL == GetpCircuitBypassPipe() )
	{
		return;
	}

	double dQ = GetpCircuitBypassPipe()->GetRealQ();

	IDPTR idptr = GetpCircuitBypassPipe()->SelectBestPipe( dQ );

	if( false == GetpCircuitBypassPipe()->IsLocked() )
	{
		GetpCircuitBypassPipe()->SetIDPtr( idptr, false );
	}
}

void CDS_HydroMod::CBase::_FreeBuffer( CMemFile *pclMemFile )
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

bool CDS_HydroMod::CBase::_ReadString( CMemFile &clMemFile, CString &strResult )
{
	strResult = _T("");

	// String size.
	int iStringSize = 0;
	UINT uiCount = clMemFile.Read( &iStringSize, sizeof( iStringSize ) );

	if( uiCount < sizeof( iStringSize ) )
	{
		return false;
	}

	// Description text.
	if( iStringSize > 0 )
	{
		char *pcTemp = new char[( iStringSize + 1 ) * sizeof( TCHAR )];

		if( NULL == pcTemp )
		{
			return false;
		}

		uiCount = clMemFile.Read( pcTemp, ( iStringSize + 1 ) * sizeof( TCHAR ) );

		if( uiCount < ( UINT )( ( iStringSize + 1 ) * sizeof( TCHAR ) ) )
		{
			delete[] pcTemp;
			return false;
		}

		USES_CONVERSION;
		strResult = ( TCHAR * )pcTemp;
		delete[] pcTemp;
	}

	return true;
}

void CDS_HydroMod::CBase::_DropDataHelp( CString &str, CDB_StringID *pCDBStringID, CString strName )
{
	if( pCDBStringID != NULL )
	{
		str.Format( _T("%s = "), strName );

		if( _T("\0" ) == pCDBStringID->GetIDstr() && _T( "\0") == pCDBStringID->GetIDstr2() )
		{
			str.Append( _T("nor IDStr neither IDStr2 are defined!\n") );
		}
		else
		{
			str.AppendFormat( _T("IDStr: %s" ), ( 0 == CString( pCDBStringID->GetIDstr() ).Compare( _T("") ) ) ? _T( "not defined") : pCDBStringID->GetIDstr() );
			str.AppendFormat( _T(" - IDStr2: %s" ), ( 0 == CString( pCDBStringID->GetIDstr2() ).Compare( _T("") ) ) ? _T( "not defined") : pCDBStringID->GetIDstr2() );
		}
	}
	else
	{
		str.Format( _T("%s is not defined"), strName );
	}

	str.Append( _T("\n") );
}

/////////////////////////////////////////////////////////////////////
CDS_HydroMod::CMeasData::CMeasData( CDS_HydroMod *pHM ) : CBase( pHM )
{
	m_MeasDateAndTime = 0;							// 01/01/1970
	m_TABalDateAndTime = 0;
	m_Instrument = eInstrumentUsed::TACBI;
	m_uiQMType = CDS_HydroModX::eQMtype::QMundef;	// Type of Quick Measurement.
	m_eLocate = CDS_HydroModX::eLocate::undef;		// Define where is located the balancing valve.
	m_eMode = CDS_HydroModX::eMode::TAmode;			// TAMode, KvMode.
	m_TAPID[0] = 0;									// ID of selected TAP in TADB.
	m_dKv = 0.0;									// Valve Kv in case of non TAMode.
	m_dMeasDp = 0.0;								// Measured Dp / could be the last Dp measure in case of balancing.
	m_dMeasDpl = 0.0;								// Measured Dpl / could be the last Dp measure in case of balancing.
	m_dCurOpening = 0.0;							// Valve Opening for current measure.

	for( unsigned dts = CDS_HydroModX::eDTS::DTSUndef + 1; dts < CDS_HydroModX::eDTS::LastDTS; dts++ )
	{
		m_dT[dts] = -273.15;
	}

	for( int i = 0; i < 2; i++ )
	{
		m_UsedDTSensor[i] = CDS_HydroModX::eDTS::DTSUndef;    // Temp sensor used for DT/Power measurement.
	}

	m_dDesignQ = 0.0;								// Design flow.
	m_dDesignOpening = 0.0;							// Design opening.
	m_dDesignDT = 0.0;								// Design DT.
	m_eBalanced = CDS_HydroModX::etabmNoBalancing;	// balancing done.
	m_bTaBalMeasCompleted = false;					// All measures done.
	m_dTaBalOpening_1 = 0.0;						// Opening for stage 1 of TA Balance.
	m_dTaBalMeasDp_1 = 0.0;							// Measured DP for stage 1 of TA Balance.
	m_dTaBalMeasDp_2 = 0.0;							// Measured Dp for stage 2 of TA Balance.
	m_dTaBalOpeningResult = 0.0;
	m_DpPVTABal = 0.0;								// TA Balance: Dp on valve closed and considered as PV of the HydroMod.
	m_dTABalDistribDp = 0.0;
	m_dTABalCircuitDp = 0.0;
	m_pDistributionPipe = new CHMXPipe( GetpWC(), CHMXPipe::epipeOnDistributionSupply );
	m_pCircuitPipe = new CHMXPipe( GetpWC(), CHMXPipe::epipeOnCircuit );
	m_pTerminalUnit = new CHMXTU( );
	m_dCBIQFinal = 0.0;								// Final flow.
	m_dCBIQTABal1 = 0.0;							// Flow for TA Balance stage 1.
}

CDS_HydroMod::CMeasData::~CMeasData()
{
	if( m_pDistributionPipe )
	{
		delete m_pDistributionPipe;
	}

	if( m_pCircuitPipe )
	{
		delete m_pCircuitPipe;
	}

	if( m_pTerminalUnit )
	{
		delete m_pTerminalUnit;
	}
}

double CDS_HydroMod::CMeasData::GetDesignPower()
{
	double dP = 0.0;
	dP = CalcP( GetDesignQ(), GetDesignDT(), GetpWC()->GetDens(), GetpWC()->GetSpecifHeat() );
	return dP;
}

double CDS_HydroMod::CMeasData::GetDesignDp()
{
	double dDp = 0.0;

	if( CDS_HydroModX::eMode::TAmode == GetMode() )
	{
		CDB_TAProduct *pTAP = GetpTAP();

		if( NULL != pTAP )
		{
			CDB_ValveCharacteristic *pValveCharacteristic = pTAP->GetValveCharacteristic();

			if( NULL != pValveCharacteristic )
			{
				if( false == pValveCharacteristic->GetValveDp( GetDesignQ(), &dDp, GetDesignOpening(), GetpWC()->GetDens(), GetpWC()->GetKinVisc() ) )
				{
					dDp = 0.0;
				}
			}
		}
	}
	else
	{
		dDp = CalcDp( GetDesignQ(), GetKv(), GetpWC()->GetDens() );
	}

	return dDp;
}

void CDS_HydroMod::CMeasData::Copy( CDS_HydroMod::CMeasData *pTo )
{
	pTo->SetMeasDateTime( m_MeasDateAndTime );
	pTo->SetTABalDateTime( m_TABalDateAndTime );
	pTo->SetInstrument( m_Instrument );
	pTo->SetQMType( m_uiQMType );
	pTo->SetLocate( m_eLocate );
	pTo->SetMode( m_eMode );
	pTo->SetTAPID( m_TAPID );
	pTo->SetKv( m_dKv );
	pTo->SetMeasDp( m_dMeasDp );
	pTo->SetMeasDpL( m_dMeasDpl );
	pTo->SetCurOpening( m_dCurOpening );

	for( unsigned dts = CDS_HydroModX::eDTS::DTSFIRST; dts < CDS_HydroModX::eDTS::LastDTS; dts++ )
	{
		pTo->SetT( ( CDS_HydroModX::eDTS ) dts, m_dT[dts] );
	}

	for( int i = 0; i < 2; i++ )
	{
		pTo->SetUsedDTSensor( i, m_UsedDTSensor[i] );
	}

	pTo->SetDesignQ( m_dDesignQ );
	pTo->SetDesignOpening( m_dDesignOpening );
	pTo->SetDesignDT( m_dDesignDT );
	pTo->SetFlagBalanced( m_eBalanced );
	pTo->SetFlagTaBalMeasCompleted( m_bTaBalMeasCompleted );
	pTo->SetTaBalOpening_1( m_dTaBalOpening_1 );
	pTo->SetTaBalMeasDp_1( m_dTaBalMeasDp_1 );
	pTo->SetTaBalMeasDp_2( m_dTaBalMeasDp_2 );
	pTo->SetTaBalOpeningResult( m_dTaBalOpeningResult );
	pTo->SetDpPVTABal( m_DpPVTABal );
	pTo->SetTABalDistribDp( m_dTABalDistribDp );
	pTo->SetTABalCircDp( m_dTABalCircuitDp );
	pTo->SetpDistributionPipe( m_pDistributionPipe );
	pTo->SetpCircuitPipe( m_pCircuitPipe );
	pTo->SetpTerminalUnit( m_pTerminalUnit );
	pTo->Setqf( m_dCBIQFinal );
	pTo->SetqTABal1( m_dCBIQTABal1 );
	pTo->CopyWCData( &m_WC );
}

#define CDS_HYDROMOD_CMEASDATA_VERSION		6
void CDS_HydroMod::CMeasData::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_CMEASDATA_VERSION;
	WriteData<>( outf, Version );

	// Info.
	WriteData<>( outf, m_MeasDateAndTime );
	WriteData<>( outf, m_Instrument );
	WriteData<>( outf, m_uiQMType );
	WriteData<>( outf, m_eLocate );
	WriteData<>( outf, m_eMode );
	WriteString( outf, m_TAPID );
	WriteData<>( outf, m_dKv );
	WriteData<>( outf, m_dMeasDp );
	WriteData<>( outf, m_dCurOpening );

	for( unsigned dts = CDS_HydroModX::eDTS::DTSFIRST; dts < CDS_HydroModX::eDTS::LastDTS; dts++ )
	{
		WriteData<>( outf, m_dT[dts] );
	}

	for( int i = 0; i < 2; i++ )
	{
		WriteData<>( outf, m_UsedDTSensor[i] );
	}

	WriteData<>( outf, m_dDesignQ );
	WriteData<>( outf, m_dDesignOpening );
	WriteData<>( outf, m_dDesignDT );
	WriteData<>( outf, m_eBalanced );
	WriteData<>( outf, m_bTaBalMeasCompleted );
	WriteData<>( outf, m_dTaBalOpening_1 );
	WriteData<>( outf, m_dTaBalMeasDp_1 );
	WriteData<>( outf, m_dTaBalMeasDp_2 );
	WriteData<>( outf, m_dTaBalOpeningResult );
	WriteData<>( outf, m_DpPVTABal );
	WriteData<>( outf, m_dCBIQFinal );
	WriteData<>( outf, m_dCBIQTABal1 );
	m_WC.Write( outf );

	// Version 2.
	WriteData<>( outf, m_dTABalDistribDp );
	WriteData<>( outf, m_dTABalCircuitDp );

	// Pipe don't exist for measurement coming from CBI!
	if( m_Instrument != eInstrumentUsed::TACBI )
	{
		m_pDistributionPipe->Write( outf );
		m_pCircuitPipe->Write( outf );
		m_pTerminalUnit->Write( outf );
	}

	WriteData<>( outf, m_dMeasDpl );
	WriteData<>( outf, m_TABalDateAndTime );
}

bool CDS_HydroMod::CMeasData::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_CMEASDATA_VERSION;
	ReadData<>( inpf, Version );

	if( Version < 1 || Version > CDS_HYDROMOD_CMEASDATA_VERSION )
	{
		return false;
	}

	// Info.
	ReadData<>( inpf, m_MeasDateAndTime );
	ReadData<>( inpf, m_Instrument );
	ReadData<>( inpf, m_uiQMType );
	ReadData<>( inpf, m_eLocate );
	ReadData<>( inpf, m_eMode );
	ReadString( inpf, m_TAPID, sizeof( m_TAPID ) );
	ReadData<>( inpf, m_dKv );
	ReadData<>( inpf, m_dMeasDp );
	ReadData<>( inpf, m_dCurOpening );

	if( Version < 5 )
	{
		for( unsigned dts = CDS_HydroModX::eDTS::DTS1onDPS; dts < CDS_HydroModX::eDTS::LastDTS; dts++ )
		{
			ReadData<>( inpf, m_dT[dts] );
		}
	}
	else
	{
		for( unsigned dts = CDS_HydroModX::eDTS::DTSFIRST; dts < CDS_HydroModX::eDTS::LastDTS; dts++ )
		{
			ReadData<>( inpf, m_dT[dts] );
		}
	}

	for( int i = 0; i < 2; i++ )
	{
		ReadData<>( inpf, m_UsedDTSensor[i] );
	}

	ReadData<>( inpf, m_dDesignQ );
	ReadData<>( inpf, m_dDesignOpening );
	ReadData<>( inpf, m_dDesignDT );

	if( Version < 4 )
	{
		double dDummy;
		ReadData<>( inpf, dDummy );
		ReadData<>( inpf, dDummy );
	}

	if( Version < 3 )
	{
		bool b;
		ReadData<>( inpf, b );

		if( true == b )
		{
			m_eBalanced = CDS_HydroModX::eTABalanceMethod::etabmTABalPlus;
		}
	}
	else
	{
		ReadData<>( inpf, m_eBalanced );
	}

	ReadData<>( inpf, m_bTaBalMeasCompleted );
	ReadData<>( inpf, m_dTaBalOpening_1 );
	ReadData<>( inpf, m_dTaBalMeasDp_1 );
	ReadData<>( inpf, m_dTaBalMeasDp_2 );
	ReadData<>( inpf, m_dTaBalOpeningResult );
	ReadData<>( inpf, m_DpPVTABal );
	ReadData<>( inpf, m_dCBIQFinal );
	ReadData<>( inpf, m_dCBIQTABal1 );

	if( false == m_WC.Read( inpf ) )
	{
		return false;
	}

	if( Version > 1 )
	{
		ReadData<>( inpf, m_dTABalDistribDp );
		ReadData<>( inpf, m_dTABalCircuitDp );

		if( NULL != m_pDistributionPipe )
		{
			delete m_pDistributionPipe;
			m_pDistributionPipe = NULL;
		}

		// Bug workaround, in version 2 pipe was conditionally saved but unconditionally read! --> crash
		// Pipe don't exist for measurement coming from CBI!
		if( m_Instrument != eInstrumentUsed::TACBI )
		{
			m_pDistributionPipe = new CHMXPipe( GetpWC(), CHMXPipe::epipeOnDistributionSupply );

			if( NULL != m_pDistributionPipe )
			{
				if( false == m_pDistributionPipe->Read( inpf ) )
				{
					return false;
				}
			}

			if( NULL != m_pCircuitPipe )
			{
				delete m_pCircuitPipe;
			}

			m_pCircuitPipe = new CHMXPipe( GetpWC(), CHMXPipe::epipeOnCircuit );

			if( NULL != m_pCircuitPipe )
			{
				if( false == m_pCircuitPipe->Read( inpf ) )
				{
					return false;
				}
			}

			if( NULL != m_pTerminalUnit )
			{
				delete m_pTerminalUnit;
			}

			m_pTerminalUnit = new CHMXTU( );

			if( NULL != m_pTerminalUnit )
			{
				if( false == m_pTerminalUnit->Read( inpf ) )
				{
					return false;
				}
			}
		}

		ReadData<>( inpf, m_dMeasDpl );
	}

	if( Version > 2 )
	{
		ReadData<>( inpf, m_TABalDateAndTime );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
CDS_HydroMod::CDpC::CDpC( CDS_HydroMod *pHM ) : CBase( pHM )
{
	InvalidateSelection();
	ClearUserChoiceTemporaryVariables();
	m_dQ = 0.0;
	m_fMvLocLocked = false;
	m_fMvLoc = m_pParent->GetpTechParam()->GetDpCMvLoc() ? eMvLoc::MvLocSecondary : eMvLoc::MvLocPrimary;
}

CDS_HydroMod::CDpC::~CDpC()
{
}

void CDS_HydroMod::CDpC::Copy( CDS_HydroMod::CDpC *pclTargetHMDpC, bool bResetDiversityFactor )
{
	pclTargetHMDpC->SetIDPtr( GetIDPtr() );
	pclTargetHMDpC->SetBestDpCIDPtr( GetBestDpCIDPtr() );
	pclTargetHMDpC->SetDiversityIDPtr( ( true == bResetDiversityFactor ) ? _NULL_IDPTR : GetDiversityIDPtr() );
	pclTargetHMDpC->SetMvLoc( m_fMvLoc );
	pclTargetHMDpC->SetMvLocLocked( m_fMvLocLocked );
	pclTargetHMDpC->SetValidtyFlags( GetValidtyFlags() );
	
	if (NULL == m_pUserChoiceDpCType && NULL == m_pUserChoiceDpCConn && NULL == m_pUserChoiceDpCPN)
	{
		pclTargetHMDpC->ClearUserChoiceTemporaryVariables();
	}
	else
	{
		pclTargetHMDpC->SetpSelDpCTypeAccordingUserChoice(m_pUserChoiceDpCType);
		pclTargetHMDpC->SetpSelDpCConnAccordingUserChoice(m_pUserChoiceDpCConn);
		pclTargetHMDpC->SetpSelDpCPNAccordingUserChoice(m_pUserChoiceDpCPN);
	}
	pclTargetHMDpC->SetQ(m_dQ);
}

#define CDS_HYDROMOD_CDPC_VERSION	6
void CDS_HydroMod::CDpC::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_CDPC_VERSION;
	WriteData<>( outf, Version );

	// Info.
	WriteString( outf, m_DiversityIDPtr.ID );
	WriteString( outf, m_IDPtrBestDpC.ID );
	WriteData<>( outf, m_dQ );
	WriteData<>( outf, m_fMvLoc );
	WriteString( outf, m_IDPtr.ID );
	WriteData<>( outf, m_fMvLocLocked );
	CString strID;
	strID = ( NULL != m_pUserChoiceDpCType ) ? m_pUserChoiceDpCType->GetIDPtr().ID : L"";
	WriteString( outf, strID );
	strID = ( NULL != m_pUserChoiceDpCConn ) ? m_pUserChoiceDpCConn->GetIDPtr().ID : L"";
	WriteString( outf, strID );
	strID = ( NULL != m_pUserChoiceDpCPN ) ? m_pUserChoiceDpCPN->GetIDPtr().ID : L"";
	WriteString( outf, strID );

}

bool CDS_HydroMod::CDpC::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_CDPC_VERSION;
	ReadData<>( inpf, Version );

	if( Version < 1 || Version > CDS_HYDROMOD_CDPC_VERSION )
	{
		return false;
	}

	// Info.
	ReadString( inpf, m_DiversityIDPtr.ID, sizeof( m_DiversityIDPtr.ID ) );

	// Before version 4, we read 'm_SelIDPtr.ID' that is no more used. Now, we write 'm_DiversityIDPtr' at this place.
	// We must reset ID to 0 if version is below 5 because diversity was not exist.
	if( Version < 4 )
	{
		m_DiversityIDPtr.ID[0] = 0;
	}
	else
	{
		m_DiversityIDPtr.DB = TASApp.GetpTADB();
	}

	ReadString( inpf, m_IDPtrBestDpC.ID, sizeof( m_IDPtrBestDpC.ID ) );

	if( Version < 2 )
	{
		return true;
	}

	ReadData<>( inpf, m_dQ );

	if( Version < 3 )
	{
		return true;
	}

	ReadData<>( inpf, m_fMvLoc );

	if( Version < 4 )
	{
		return true;
	}

	ReadString( inpf, m_IDPtr.ID, sizeof( m_IDPtr.ID ) );
	m_IDPtr.DB = TASApp.GetpTADB();

	if( Version < 6 )
	{
		// See CDataStruct version 4015  Convert DA50 en function CTADatastruct::UpgradeDA50IDPTR()
		CMapStringToString TableID;
		TableID[ _T("DA50_???J" ) ] = _T( "DA50_???I");
		TableID[ _T("DA50_???L" ) ] = _T( "DA50_???K");
		TableID[ _T("DA50_???N" ) ] = _T( "DA50_???M");
		TableID[ _T("DA50_???P" ) ] = _T( "DA50_???O");
		CString strDpCID = m_IDPtr.ID;

		if( strDpCID.Find( _T("DA50_") ) == 0 )
		{
			CString Key;
			CString NewID;

			for( POSITION pos = TableID.GetStartPosition(); pos != NULL; )
			{
				TableID.GetNextAssoc( pos, Key, NewID );
				bool fMatch = WildcardMatch( (LPCTSTR) strDpCID, Key );

				if( true == fMatch )
				{
					strDpCID.Replace( strDpCID.Right( 1 ), NewID.Right( 1 ) );
					IDPTR newidptr = TASApp.GetpTADB()->Get( strDpCID );

					if( NULL == newidptr.MP )
					{
						ASSERT( 0 );
					}

					m_IDPtr = newidptr;
				}
			}
		}
	}

	ReadData<>( inpf, m_fMvLocLocked );

	ClearUserChoiceTemporaryVariables();

	if( Version < 5 )
	{
		return true;
	}

	IDPTR idptr;
	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceDpCType = dynamic_cast<CDB_StringID *>( idptr.MP );
	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceDpCConn = dynamic_cast<CDB_StringID *>( idptr.MP );
	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceDpCPN = dynamic_cast<CDB_StringID *>( idptr.MP );

	return true;
}

bool CDS_HydroMod::CDpC::FillDpCInfoIntoHMX( CDS_HydroModX *pHMX )
{
	CDS_HydroModX::CDpC *pDpCHMX = pHMX->GetpDpC();
	ASSERT( pDpCHMX );

	if( NULL == pDpCHMX )
	{
		return false;
	}

	if( NULL == GetIDPtr().MP /*|| false == GetIDPtr().MP->IsExistInTASCOPE()*/ )
	{
		return false;
	}

	if( NULL != m_pParent->GetpSch() && eDpStab::DpStabOnBranch == m_pParent->GetpSch()->GetDpStab() )
	{
		pDpCHMX->SetStabilizedOn( DpStabOnBranch );
	}
	else // On a CV
	{
		pDpCHMX->SetStabilizedOn( DpStabOnCV );
	}

	if( eMvLoc::MvLocPrimary == GetMvLoc() )
	{
		pDpCHMX->SetMvLoc( MvLocPrimary );
	}
	else
	{
		pDpCHMX->SetMvLoc( MvLocSecondary );
	}

	pDpCHMX->SetValveID( GetIDPtr().ID );
	return true;
}

IDPTR CDS_HydroMod::CDpC::GetIDPtr()
{
	if( _T('\0') != *m_IDPtr.ID && NULL != m_IDPtr.DB )
	{
		Extend( &m_IDPtr );
		return m_IDPtr;
	}

	return _NULL_IDPTR;
}

IDPTR CDS_HydroMod::CDpC::GetDiversityIDPtr()
{
	if( _T('\0') != *m_DiversityIDPtr.ID && NULL != m_DiversityIDPtr.DB )
	{
		Extend( &m_DiversityIDPtr );
		return m_DiversityIDPtr;
	}

	return _NULL_IDPTR;
}

void CDS_HydroMod::CDpC::SetIDPtr( IDPTR idptr )
{
	m_IDPtr = idptr;

	if( _T('\0') != *m_IDPtr.ID && NULL != m_IDPtr.DB )
	{
		Extend( &m_IDPtr );

		if( true == m_pParent->IsForHub() )
		{
			if( true == m_pParent->IsClass( CLASS( CDS_HmHub ) ) )
			{
				// DpC Balancing on return side.
				( static_cast<CDS_HmHub *>( m_pParent ) )->SetReturnValveID( ( static_cast<CDS_HmHub *>( m_pParent ) )->FindHubPtnValve( CDB_HubValv::eSupplyReturnValv::Return,
						m_IDPtr ).ID );
			}
		}
	}

	m_pParent->Modified();
}

CDB_StringID *CDS_HydroMod::CDpC::GetpSelDpCType()
{
	CDB_StringID *p = NULL;

	if( _T('\0') != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( ( CDB_DpController * ) GetIDPtr().MP )->GetTypeIDPtr().MP ) ;
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CDpC::GetpSelDpCConn()
{
	CDB_StringID *p = NULL;

	if( _T('\0') != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( ( CDB_DpController * ) GetIDPtr().MP )->GetConnectIDPtr().MP );
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CDpC::GetpSelDpCPN()
{
	CDB_StringID *p = NULL;

	if( _T('\0') != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( ( CDB_DpController * ) GetIDPtr().MP )->GetPNIDPtr().MP );
	}

	return p;
}

double CDS_HydroMod::CDpC::GetQ()
{
	return m_dQ;
}

void CDS_HydroMod::CDpC::SetQ( double dQ )
{
	ASSERT( dQ >= 0 );
	m_dQ = dQ;

	// Is valve choice is locked it'll be necessary to change Dpmin when Q change.
	if( true == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eDpC ) )
	{
		// DpC exist.
		if( NULL != GetIDPtr().MP )
		{
			//TODO:CDpC::SetQ adjust Q when DpC is locked
		}
	}
}

bool CDS_HydroMod::CDpC::ForceDpCSelection( IDPTR DeviceIDptr )
{
	if( true == m_mapDpC.empty() )
	{
		// Unlock the valve to force filling of m_mapValves
		m_pParent->SetLock( CDS_HydroMod::eHMObj::eDpC, false );
	}

	// Lock valve if new selected valve doesn't correspond to the Best valve idptr
	m_pParent->SetLock( CDS_HydroMod::eHMObj::eDpC, !IsBestDpC( (CData *)DeviceIDptr.MP ) );

	// Forced DpC should be inside m_mapDpC as a valid choice;
	_SetpSelDpC( (LPARAM)DeviceIDptr.MP, true );
	ClearUserChoiceTemporaryVariables();

	return ( GetIDPtr().MP == DeviceIDptr.MP );
}

void CDS_HydroMod::CDpC::InvalidateSelection()
{
	if( true == GetpParentHM()->IsLocked( CDS_HydroMod::eHMObj::eDpC ) )
	{
		return;
	}
	
	SetIDPtr( _NULL_IDPTR );
	SetBestDpCIDPtr( _NULL_IDPTR );

	// Check now if we can clean also the "m_CBIValveIDPtr" variable. It depends if a previous one exists and is the same kind of the current
	// one. If it's the case we can clean it, otherwise it means the CBI valve is perhaps a regulating valve and we can't delete it here.
	if( _NULL_IDPTR != GetpParentHM()->GetCBIValveIDPtr() )
	{
		if( NULL != dynamic_cast<CDB_DpController *>( GetpParentHM()->GetCBIValveIDPtr().MP ) )
		{
			GetpParentHM()->SetCBIValveID( _T("") );
		}
	}
}

double CDS_HydroMod::CDpC::GetDpCSetting( CDB_DpController *pDpC )
{
	if( NULL == pDpC )
	{
		pDpC = static_cast< CDB_DpController * >( GetIDPtr().MP );
	}

	if( NULL == pDpC )
	{
		return 0.0;
	}

	double dDpl = GetDpToStab();
	double dDpCSetting = 0.0;
	double dQ = GetQ();
	double dpDpC = GetDp();
	double dKv;

	if( dQ <= 0.0 || dpDpC <= 0.0 )
	{
		return 0.0;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eDpC );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	dKv = CalcKv( dQ, dpDpC, pclWaterChar->GetDens() );

	// Get pointer on DpC characteristic.
	CDB_DpCCharacteristic *pDevChar = pDpC->GetDpCCharacteristic();

	if( NULL == pDevChar )
	{
		return 0.0;
	}

	double dPB = pDevChar->GetProportionalBand( dDpl );

	// Correct SP to get requested Dpl.
	double dSP = 0.0;
	double dKvm = pDpC->GetKvm();

	if( -1.0 == dKvm )
	{
		return 0.0;
	}

	if( false == pDevChar->IsProportionalBandAbsolute() )
	{
		// PB in % of SP.
		double dA = 1 - 0.3 * dPB;
		double dB = 1 + 0.7 * dPB;
		dSP = ( ( 1 / ( dB - dA ) ) / ( dB / ( dB - dA ) - dKv / dKvm ) ) * dDpl;
	}
	else
	{
		// PB is fixed.
		dSP = dDpl + dPB * ( dKv / dKvm - 0.5 );
	}

	// Check SP bounds & calculate setting.
	if( dSP < pDevChar->GetSPMin() )
	{
		dSP = pDevChar->GetSPMin();
	}

	if( dSP > pDevChar->GetSPMax() )
	{
		dSP = pDevChar->GetSPMax();
	}

	dDpCSetting = pDevChar->GetOpening( dSP );
	return dDpCSetting;
}

// Compute Dp that must be stabilized (linked to the circuit).
// if MvLoc == eMvLoc::MvLocLast use MvLoc from CDB_CircuitScheme.
double CDS_HydroMod::CDpC::GetDpToStab( eMvLoc MvLoc, bool bToPump )
{
	CDB_CircuitScheme *pSch = m_pParent->GetpSch();

	if( NULL == pSch || NULL == pSch->GetpSchCateg() )
	{
		return 0.0;
	}

	double dDpToStabilize = 0.0;

	if( eDpStab::DpStabOnCV == pSch->GetDpStab() )
	{
		// Dp stabilized on CV.
		double dDpCV = 0.0;

		if( NULL != m_pParent->GetpCV() && eb3True == m_pParent->GetpCV()->IsCVLocInPrimary() )
		{
			dDpCV = m_pParent->GetpCV()->GetDp();
		}

		dDpToStabilize += dDpCV;

		// Measuring valve on secondary.
		if( ( eMvLoc::MvLocLast == MvLoc && eMvLoc::MvLocSecondary == GetMvLoc() ) || eMvLoc::MvLocSecondary == MvLoc )
		{
			if( NULL != m_pParent->GetpBv() )
			{
				dDpToStabilize += m_pParent->GetpBv()->GetDp();
			}
		}
	}
	else if( eDpStab::DpStabOnBVBypass == pSch->GetDpStab() 
			&& ( ( eMvLoc::MvLocLast == MvLoc && eMvLoc::MvLocSecondary == GetMvLoc() ) || eMvLoc::MvLocSecondary == MvLoc ) )
	{
		// HYS-1930: auto-adapting variable flow decoupling circuit.
		// For this circuit, the measuring valve is on the secondary.
		if( NULL != m_pParent->GetpBypBv() )
		{
			if( true == bToPump )
			{
				dDpToStabilize += m_pParent->GetpBypBv()->GetDpMin();
			}
			else
			{
				dDpToStabilize += m_pParent->GetpBypBv()->GetDp();
			}
		}
	}
	else 
	{
		// Dp stabilized on branch.

		if( true == pSch->GetpSchCateg()->Is3W() && CDB_CircSchemeCateg::e3WType::e3wTypeMixing == pSch->GetpSchCateg()->Get3WType() )
		{
			// If we are with a 3-way mixing circuit, the pressure drop accross the control valve is taken by the pump at the secondary side.
			// And the balancing valve at the primary side take the same value (To offer the same resistance at the primary side for the distribution
			// when the control valve is completely closed).

			// Measuring valve on secondary.
			if( ( eMvLoc::MvLocLast == MvLoc && eMvLoc::MvLocSecondary == GetMvLoc() ) || eMvLoc::MvLocSecondary == MvLoc )
			{
				if( NULL != m_pParent->GetpBv() )
				{
					dDpToStabilize += m_pParent->GetpBv()->GetDp();
				}
			}
		}
		else
		{
			double dDpCV = 0.0;

			if( NULL != m_pParent->GetpCV() && eb3True == m_pParent->GetpCV()->IsCVLocInPrimary() )
			{
				dDpCV = m_pParent->GetpCV()->GetDp();
			}

			dDpToStabilize += dDpCV;
			dDpToStabilize += m_pParent->GetDpOnOutOfPrimSide();

			// Measuring valve on secondary.
			if( ( eMvLoc::MvLocLast == MvLoc && eMvLoc::MvLocSecondary == GetMvLoc() ) || eMvLoc::MvLocSecondary == MvLoc )
			{
				if( NULL != m_pParent->GetpBv() )
				{
					dDpToStabilize += m_pParent->GetpBv()->GetDp();
				}
			}
		}
	}

	return dDpToStabilize;
}

// Find out the Kvs bounds by sweeping over acceptable Dp controllers depending on the flow and by taking the
// best of their lower and higher Dpl bounds.
bool CDS_HydroMod::CDpC::GetKvsBounds( double *pdLowestKvs, double *pdHighestKvs, double *pdLowestDpl, double *pdHighestDpl )
{
	std::multimap <int, CDpCData > *pMapDpC = GetpMapDpC();
	std::multimap <int, CDpCData >::iterator ItMapDpC = pMapDpC->end();

	if( pMapDpC->size() == 0 )
	{
		return false;
	}
		
	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eDpC );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( false );
	}
	
	double dRho = pclWaterChar->GetDens();
	*pdLowestDpl = DBL_MAX;
	*pdHighestDpl = 0.0;
	double dBestBP = DBL_MAX;
	double dFlow = GetQ();
	bool bOneValidFound = false;

	for( ItMapDpC = pMapDpC->begin(); pMapDpC->end() != ItMapDpC; ++ItMapDpC )
	{
		CDB_DpController *pDpC = ItMapDpC->second.GetpDpC();

		// Get Kv characteristic of the valve.
		CDB_DpCCharacteristic *pDevChar = pDpC->GetDpCCharacteristic();

		if( NULL == pDevChar )
		{
			continue;
		}

		// Find min and max Dpl.
		double dDplMin = pDpC->GetDplmin();
		double dDplMax = pDpC->GetDplmax();

		if( -1.0 == dDplMin || -1.0 == dDplMax )
		{
			continue;
		}

		double dQMin = pDevChar->GetQmin( dRho );
		double dQMax = pDevChar->GetQmax( dRho );

		// Check the flow with respect to qmin and qmax.
		// Take the min of Dplmin and max of Dplmax .
		// (i.e. less limitative).
		if( dFlow >= dQMin && dFlow <= dQMax )
		{
			bOneValidFound = true;
			*pdHighestDpl = max( *pdHighestDpl, dDplMax );

			if( dDplMin < *pdLowestDpl )		// Reset Best BP
			{
				*pdLowestDpl = dDplMin;

				if( true == pDevChar->IsProportionalBandAbsolute() )
				{
					dBestBP = pDevChar->GetProportionalBand( dDplMin ) / dDplMin;
				}
				else
				{
					dBestBP = pDevChar->GetProportionalBand( dDplMin );
				}
			}
			else if( dDplMin == *pdLowestDpl )	// Update Best BP
			{
				if( true == pDevChar->IsProportionalBandAbsolute() )
				{
					dBestBP = min( dBestBP, pDevChar->GetProportionalBand( dDplMin ) / dDplMin );
				}
				else
				{
					dBestBP = min( dBestBP, pDevChar->GetProportionalBand( dDplMin ) );
				}
			}
		}
	}

	// Correct the Dp bounds when the Mv is in the secondary.
	if( NULL != m_pParent->GetpSch() && eMvLoc::MvLocSecondary == m_pParent->GetpSch()->GetMvLoc() )
	{
		*pdLowestDpl *= m_pParent->GetpTechParam()->GetCVMinAuthor() * ( 1.0 + dBestBP );
		*pdHighestDpl -= m_pParent->GetpTechParam()->GetValvMinDp();
		*pdHighestDpl = max( *pdHighestDpl, 0 );
	}

	// Calculate the Kvs bounds from the Dp bounds.
	*pdHighestKvs = CalcKv( dFlow, *pdLowestDpl, dRho );
	*pdLowestKvs = CalcKv( dFlow, *pdHighestDpl, dRho );
	return bOneValidFound;
}

void CDS_HydroMod::CDpC::ResizeBvForMvLoc()
{
	// HYS-1930: we can get now auto adaptive variable flow with decoupling circuit with a BV in the bypass.
	if( ( NULL == m_pParent->GetpBv() && NULL == m_pParent->GetpBypBv() )
			|| NULL == m_pParent->GetpSch() )
	{
		return;
	}

	if( eDpStab::DpStabOnBVBypass == m_pParent->GetpSch()->GetDpStab() && NULL != m_pParent->GetpDpC() )
	{
		// HYS-1930: auto-adapting variable flow decoupling circuit.
		
		// When going to the pump, BV must be sized for its minimum.
		// double dReqDpMv = m_pParent->GetpTechParam()->GetValvMinDp();

		double dReqDpMv = 0.0;

		if( m_pParent->GetHAvail( CAnchorPt::CircuitSide::CircuitSide_Primary ) <= 0.0 )
		{
			// When going to the pump, the balancing valve in the bypass must be sized to take the minmum differential pressure
			// that the Dp controller can stabilize.
			dReqDpMv = m_pParent->GetpDpC()->GetDplmin();
		}
		else
		{
			// When coming from the pump, the differential pressure accross the balancing valve in the bypass must be 
			// the difference between the H available at the entry of the hydraulic circuit and the pressure drop accross
			// the Dp controller.
			dReqDpMv = m_pParent->GetHAvail( CAnchorPt::CircuitSide::CircuitSide_Primary ) - m_pParent->GetpDpC()->GetDp();

			if( NULL != m_pParent->GetpCircuitPrimaryPipe() )
			{
				dReqDpMv -= m_pParent->GetpCircuitPrimaryPipe()->GetPipeDp();
				dReqDpMv -= m_pParent->GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true );
			}
		}

		m_pParent->EnableComputeHM( false );
		m_pParent->GetpBypBv()->ResizeBv( dReqDpMv );
		m_pParent->EnableComputeHM( true );
	}
	else
	{
		if( eMvLoc::MvLocPrimary == GetMvLoc() )
		{
			m_pParent->m_bComputeAllowed = false;

			// Resize Bv with Dp to stabilize.
			if( 0 != *m_IDPtrSelMv.ID )
			{
				m_pParent->GetpBv()->ForceBVSelection( m_IDPtrSelMv );
				m_pParent->GetpBv()->SetBestBvIDPtr( m_IDPtrSelMv );
			}
			else
			{
				m_pParent->GetpBv()->ResizeBv( m_pParent->GetpTechParam()->GetValvMinDp() );
			}

			m_pParent->m_bComputeAllowed = true;
		}
		else
		{
			CDB_DpController *pDpC = static_cast< CDB_DpController * >( GetIDPtr().MP );

			// Update Bv Dp.
			double dReqDpMv = m_pParent->GetpTechParam()->GetValvMinDp();

			// No H present we keep MV valve to 3kPa
			if( m_pParent->GetHAvail( CAnchorPt::CircuitSide_Primary ) > 0 )
			{
				dReqDpMv = max( dReqDpMv, pDpC->GetDplmin() - GetDpToStab( eMvLoc::MvLocPrimary ) );
			}

			// In case of Measuring valve in secondary STAM cannot be selected, re-select BV type connection version and PN.
			//m_pParent->GetpBv()->SelectBestBvType();

			m_pParent->EnableComputeHM( false );
			m_pParent->GetpBv()->ResizeBv( dReqDpMv );
			m_pParent->EnableComputeHM( true );
		}
	}
}

void CDS_HydroMod::CDpC::ResetMVLoc()
{
	if( NULL != m_pParent->GetpSch() && CDB_ControlProperties::PresetPT == m_pParent->GetpSch()->GetCvFunc() )
	{
		// Measuring valve is included with control valve, only one localization: secondary
		SetMvLoc( eMvLoc::MvLocSecondary );
		return;
	}

	CDB_DpController *pDpC = static_cast<CDB_DpController *>( GetIDPtr().MP );

	if( -1.0 == pDpC->GetDplmin() )
	{
		return;
	}

	// HYS-1930: auto-adapting variable flow decoupling circuit.
	if( eDpStab::DpStabOnBVBypass == m_pParent->GetpSch()->GetDpStab() )
	{
		// We don't change the MV location for this circuit.
		SetMvLoc( eMvLoc::MvLocSecondary );
		SetMvLocLocked( true );
		ResizeBvForMvLoc();
	}
	else
	{
		eMvLoc MvLoc = (eMvLoc) m_pParent->GetpTechParam()->GetDpCMvLoc();

		// MvLoc Locked ?
		if( true == HasMvLockLocked() )
		{
			// Test if MvLoc must stay locked.
			if( eMvLoc::MvLocPrimary == GetMvLoc() && ( pDpC->GetDplmin() > GetDpToStab( eMvLoc::MvLocPrimary ) ) )
			{
				// No valid solution if we stay in primary, switch to secondary.
				SetMvLocLocked( false );
			}
			else
			{
				ResizeBvForMvLoc();
			}
		}

		// MvLoc could be unlocked previously.
		if( false == HasMvLockLocked() )
		{
			// Measuring valve is in secondary.
			if( ( eMvLoc::MvLocPrimary == m_pParent->GetpTechParam()->GetDpCMvLoc() && pDpC->GetDplmin() > GetDpToStab( eMvLoc::MvLocPrimary ) )
					|| ( eMvLoc::MvLocSecondary == m_pParent->GetpTechParam()->GetDpCMvLoc() ) )
			{
				SetMvLoc( eMvLoc::MvLocSecondary );
			}
			else
			{
				// Measuring valve is in primary.
				SetMvLoc( eMvLoc::MvLocPrimary );
			}

			ResizeBvForMvLoc();
		}
	}
}

void CDS_HydroMod::CDpC::_SetpSelDpC( LPARAM pTaDbDpC, bool fForceLocker )
{
	if( false == fForceLocker )
	{
		if( true == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eDpC ) && _T('\0') != *GetIDPtr().ID )
		{
			m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceDpC, (LPARAM)this );
			return;
		}
	}

	// Try to find product inside main valve map
	std::multimap <int, CDpCData>::iterator itMapDpC;

	for( itMapDpC = m_mapDpC.begin(); m_mapDpC.end() != itMapDpC; ++itMapDpC )
	{
		if( pTaDbDpC == (LPARAM)itMapDpC->second.GetpDpC() )
		{
			break;
		}
	}

	if( m_mapDpC.end() != itMapDpC )
	{
		SetIDPtr( itMapDpC->second.GetpDpC()->GetIDPtr() );

		// We do not reset BV location and size for a 3-way mixing !
		bool bIs3wayMixing = ( NULL != m_pParent->GetpSchcat() && CDB_ControlProperties::CV2W3W::CV3W == m_pParent->GetpSchcat()->Get2W3W()
				&& CDB_CircSchemeCateg::e3WType::e3wTypeMixing == m_pParent->GetpSchcat()->Get3WType()
				&& eDpStab::DpStabOnBranch == m_pParent->GetpSch()->GetDpStab() );

		// And also for a 3-way mixing circuit with decoupling bypass and CV at the primary side.

		bool bIs3wayMixingDecByp = ( NULL != m_pParent->GetpSchcat() && CDB_ControlProperties::CV2W3W::CV3W == m_pParent->GetpSchcat()->Get2W3W()
				&& CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == m_pParent->GetpSchcat()->Get3WType()
				&& eDpStab::DpStabOnBranch == m_pParent->GetpSch()->GetDpStab() 
				&& CvLocation::CvLocPrimSide == m_pParent->GetpSch()->GetCvLoc() );

		if( false == bIs3wayMixing && false == bIs3wayMixingDecByp )
		{
			ResetMVLoc();
		}
		
		m_pParent->Modified();
	}
	else
	{
		//		ASSERT( 0 );
		InvalidateSelection();
	}
}

void CDS_HydroMod::CDpC::SetpSelDpCTypeAccordingUserChoice( CDB_StringID *pSelDpCType )
{
	if( NULL != pSelDpCType && false == pSelDpCType->IsClass( CLASS( CDB_StringID ) ) )
	{
		ASSERT( false );
		pSelDpCType = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceDpCType = pSelDpCType;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceDpCConnect, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CDpC::SetpSelDpCConnAccordingUserChoice( CDB_StringID *pSelDpCConn )
{
	if( NULL != pSelDpCConn && false == pSelDpCConn->IsClass( CLASS( CDB_Connect ) ) )
	{
		ASSERT( false );
		pSelDpCConn = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceDpCType = GetpSelDpCType();
	m_pUserChoiceDpCConn = pSelDpCConn;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceDpCPN, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CDpC::SetpSelDpCPNAccordingUserChoice( CDB_StringID *pSelDpCPN )
{
	if( NULL != pSelDpCPN && false == pSelDpCPN->IsClass( CLASS( CDB_StringID ) ) )
	{
		ASSERT( false );
		pSelDpCPN = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceDpCType = GetpSelDpCType();
	m_pUserChoiceDpCConn = GetpSelDpCConn();
	m_pUserChoiceDpCPN = pSelDpCPN;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceDpC, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CDpC::ResizeDpC( double dHAvailable )
{
	if( NULL == GetIDPtr().MP )
	{
		return;
	}

	// Reinit MV IDPTR.
	m_IDPtrSelMv = _NULL_IDPTR;

	if( true == GetIDPtr().MP->IsClass( CLASS( CDB_DpController ) ) )
	{
		if( true == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eDpC ) )
		{
			// ResetMVLoc();
		}
		else
		{
			SelectBestDpC( dHAvailable );
		}
	}
	else
	{
		ASSERT( 0 );
	}
}

void CDS_HydroMod::CDpC::SelectBestDpC( double dHAvailable, bool bLowestDplmin, bool bBypassStabilized )
{
	if( NULL == m_pParent->GetpSch() )
	{
		ASSERT_RETURN;
	}

	double dFlow = GetQ();

	if( dFlow <= 0 )
	{
		_AbortSelectBestDpC();
		return;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eDpC );
	
	if( NULL == pclWaterChar )
	{
		ASSERT_RETURN;
	}

	double dRho = pclWaterChar->GetDens();
	double dNu = pclWaterChar->GetKinVisc();

	int iHighestSize = 0;
	int iLowestSize = 0;
	bool bContinue = true;

	CAnchorPt::PipeLocation ePipeLocation = m_pParent->GetpSch()->GetAnchorPtPipeLocation( CAnchorPt::eFunc::DpC );
	CDB_Pipe *pPipe = m_pParent->GetPipeSizeShift( iHighestSize, iLowestSize, CAnchorPt::CircuitSide_Primary, ePipeLocation );

	if( NULL == pPipe )
	{
		_AbortSelectBestDpC();
		return;
	}

	int iDN50 = m_pParent->GetDN50();
	CDS_TechnicalParameter *pTechParam = m_pParent->GetpTechParam();
	CPrjParams *pPrjParams = m_pParent->GetpPrjParam()->GetpHmCalcParams();

	// Establish DpC list based on DpCType and the best technical choice
	CRankEx List;

	// HYS-1308: A Dp controller dedicated to put in downstream (supply) can't be placed in the upstream (return) pipe.
	// And the same a Dp controller for upstream can't be placed on the downstream.
	GetpTADB()->GetDpCList( &List, GetpParentHM()->GetpSch()->GetDpCLoc(), L"", L"", L"", L"", L"", L"", CTADatabase::FilterSelection::ForHMCalc );

	double dDpToStab = 0.0;
	
	// If we don't want a Dp controller that fits the flow and has the lowest Dplmin, we must call the "GetDpToStab" method to know
	// what is the differential pressure to stabilize.
	if( false == bLowestDplmin )
	{
		dDpToStab = GetDpToStab( eMvLoc::MvLocLast, ( 0.0 == dHAvailable ) ? true : false );
	}

	bool bUserChangeSelParameters = false;

	if( NULL != m_pUserChoiceDpCType || NULL != m_pUserChoiceDpCConn || NULL != m_pUserChoiceDpCPN )
	{
		bUserChangeSelParameters = true;
	}

	CData *pData = NULL;
	_string str;
	m_mapDpC.clear();
	std::multimap <int, CDpCData>::iterator itMapDpC;
	std::multimap <int, CDpCData>::reverse_iterator rItmapDpC;

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
			// Sanity
			continue;
		}

		// Verify qmin and qmax
		// Compute min and max q
		// qmin depends on DH-Dpl and the selection gives Hmin,
		// so:	qmin taken as qmin for DpToStab.
		double qmin = pDpCChar->GetQmin( dRho, dDpToStab );
		
		// Qmax depends on DH-Dpl and the selection gives Hmin,
		// so:	qmax taken as the max of qmax.
		double qmax = pDpCChar->GetQmax( dRho );

		if( dFlow < qmin || dFlow > qmax )
		{
			continue;
		}

		// Verify DpL min/max
		double dDplmin = pDpCChar->GetDplmin();
		double dDplmax = pDpCChar->GetDplmax();

		if( -1.0 == dDplmin || -1.0 == dDplmax )
		{
			continue;
		}

		double dDpMin = -1;

		if( dDpToStab <= dDplmax )
		{
			pDpCChar->DpCSolve( dFlow, &dDpMin, dRho );

			// In descending phase, verify Dpmin according measuring valve position
			/*
			if( dHAvailable > pTechParam->GetDpCMinDp() )
			{
				double dDpDpC = dHAvailable;

				if( MvLocSecondary == GetMvLoc() )
				{
					double dBvDp = ( NULL != m_pParent->GetpBv() ) ? m_pParent->GetpBv()->GetDpMin() :  pTechParam->GetValvMinDp();
					dDpDpC = dHAvailable - dBvDp - GetDpToStab( eMvLoc::MvLocPrimary );		// Use Bv DpMin in place of real Dp

					if( dDpDpC < dDpMin )
					{
						// No Solution; dH is too low
						//continue;
					}
				}
			}
			*/
		}

		// Add product.
		int iProdSizeKey = pDpC->GetSizeKey();
		itMapDpC = m_mapDpC.insert( std::make_pair( iProdSizeKey, CDpCData( pDpC ) ) );
		itMapDpC->second.SetDpMin( dDpMin );
	}

	if( m_mapDpC.size() == 0 )
	{
		_AbortSelectBestDpC();
		return;
	}

	// HYS-1308: Change the way we reject sizes.
	std::multimap <int, CDpCData>::iterator itLow = m_mapDpC.end();
	std::multimap <int, CDpCData>::iterator itUp = m_mapDpC.end();

	for( std::multimap <int, CDpCData>::iterator iter = m_mapDpC.begin(); iter != m_mapDpC.end(); iter++ )
	{
		if( iter->first >= iLowestSize )
		{
			itLow = iter;
			break;
		}
	}

	if( m_mapDpC.end() == itLow )
	{
		itLow = m_mapDpC.lower_bound( ( *m_mapDpC.rbegin() ).first );
	}

	bool bFound = false;

	for( std::multimap <int, CDpCData>::reverse_iterator riter = m_mapDpC.rbegin(); riter != m_mapDpC.rend(); riter++ )
	{
		if( riter->first <= iHighestSize )
		{
			itUp = riter.base();
			bFound = true;
			break;
		}
	}

	if( m_mapDpC.end() == itUp && false == bFound )
	{
		itUp = m_mapDpC.upper_bound( (*m_mapDpC.begin()).first );
	}

	if( m_mapDpC.end() != itLow && m_mapDpC.begin() != itLow )
	{
		m_mapDpC.erase( m_mapDpC.begin(), itLow );
	}

	if( m_mapDpC.end() != itUp )
	{
		m_mapDpC.erase( itUp, m_mapDpC.end() );
	}

	if( 0 == m_mapDpC.size() )
	{
		_AbortSelectBestDpC();
		return;
	}

	// Full DpC list, start to order it
	// Kvm pivot is 80% for all DpC except for index circuit where we keep selected DpC with a Dpmin close to 5kPa.
	double dBestDpCKvmPivot = 0.80;

	if( true == m_pParent->IsCircuitIndex() )
	{
		dBestDpCKvmPivot = 1;    //0.99;
	}

	typedef std::multimap<double, CDB_TAProduct *> _TMapDelta;
	_TMapDelta mapDeltaTAP;

	// Remove TU Dp when stabilization is done on CV.
	if( eDpStab::DpStabOnCV == m_pParent->GetpSch()->GetDpStab() )
	{
		dHAvailable -= m_pParent->GetDpOnOutOfPrimSide();
	}

	double dBvDp = ( NULL != m_pParent->GetpBv() ) ? m_pParent->GetpBv()->GetDp() : 0.0;

	// HYS-1930: used for auto-adaptive variable flow with decoupling bypass.
	// For more description see the "CDS_HydroMod::SetHavail" method.
	_TMapDelta m_mapDpCDplmaxTooLow;

	// Find DpC with BestDpCKvmPivot * Kvm close to the Kv(Havail; qd).
	for( itMapDpC = m_mapDpC.begin(); itMapDpC != m_mapDpC.end(); ++itMapDpC )
	{
		CDB_DpController *pDpC = itMapDpC->second.GetpDpC();
		CDB_DpCCharacteristic *pDpCChar = pDpC->GetDpCCharacteristic();

		if( NULL == pDpCChar )
		{
			// Sanity
			continue;
		}

		if( dHAvailable <= pTechParam->GetDpCMinDp() )
		{
			// To the pump, evaluate needed Dp for the DpC.
			// We select DpC with a Dpmin above techparam DpcMinDp.
			if( itMapDpC->second.GetDpMin() >= pTechParam->GetDpCMinDp() || ( true == bUserChangeSelParameters ) )
			{
				// Verify Dp to stabilize only if we don't need the lowest Dplmin for the current flow.
				if( false == bLowestDplmin )
				{
					double dDpStab = GetDpToStab( eMvLoc::MvLocPrimary );

					if( dDpStab > pDpC->GetDplmax() )
					{
						continue;
					}
				}

				double dKey = itMapDpC->second.GetDpMin();
				mapDeltaTAP.insert( std::make_pair( dKey, pDpC ) );
			}
		}
		else
		{
			// From the pump.
			double dDpDpC = dHAvailable;
			double dDpStab = -1;

			if( MvLocSecondary == GetMvLoc() )
			{
				if( true == bBypassStabilized ) 
				{
					// HYS-1930: auto-adaptive variable flow with decoupling bypass.
					// When distributing the H available at the entry of this circuit, we try to keep the Dp controller at its
					// Dp min and take the difference in the stabilized differential pressure.
					// For more description see the "CDS_HydroMod::SetHavail" method.
					dDpDpC = itMapDpC->second.GetDpMin();
					dDpStab = dHAvailable - dDpDpC;
				}
				else
				{
					// Balancing valve is included in the pressure drop to stabilize. Dp to stabilize is thus the sum of
					// the pressure drop in the terminal unit and the pressure drop in the balancing valve.
					dDpStab = GetDpToStab( eMvLoc::MvLocSecondary );
					dDpDpC = dHAvailable - dDpStab;
				}
			}
			else if( MvLocPrimary == GetMvLoc() )
			{
				// Balancing valve is not included in the pressure drop to stabilize. Dp to stabilize is thus
				// the pressure drop in the terminal unit only.

				dDpStab = GetDpToStab( eMvLoc::MvLocPrimary );
				dDpDpC = dHAvailable - dDpStab - dBvDp;

				if( dDpStab < pDpCChar->GetDplmin() )
				{
					// If Dp to stabilize is below the minimum that the Dp controller can work, we can add the pressure drop
					// on the balancing valve by forcing it to be in the primary side.

					dDpStab = pDpCChar->GetDplmin();
					dDpDpC = dHAvailable - dDpStab;
				}
			}

			bool bInsertInDplmaxTooLow = false;

			if( dDpStab > pDpC->GetDplmax() )
			{
				if( true == bBypassStabilized ) 
				{
					// HYS-1930: auto-adaptive variable flow with decoupling bypass.
					// Check if we can keep this Dp controller by using the differential pressure to stabilized to the maximum possbible
					// for this valve and take the difference in the Dp controller itself.
					// For more description see the "CDS_HydroMod::SetHavail" method.
					dDpDpC = dHAvailable - pDpC->GetDplmax();
					bInsertInDplmaxTooLow = true;
				}
				else
				{
					continue;
				}
			}

			// Dp available for DpC must be greater or equal to DpMin.
			if( dDpDpC >= ( 0.99 * itMapDpC->second.GetDpMin() ) && -1.0 != pDpC->GetKvm() )
			{
				double dKey = 0.0;
				double dKv = CalcKv( dFlow, dDpDpC, dRho );
				double dDelta = fabs( dKv / pDpC->GetKvm() - dBestDpCKvmPivot ) * 100.0;

				if( true == bBypassStabilized ) 
				{
					// HYS-1930: auto-adapting variable flow decoupling circuit.
					// Here we do not want that the Dp controller with the lower Dplmin be the best.
					// With this circuit we want that the differential pressure stabilized in the bypass
					// can help the pump at the secondary side. So a bigger Dpl is better.
					dKey = ( -dDpStab * 100.0 ) + dDelta;
				}
				else
				{
					// We would like to keep DpLmin minimum and a minimum value for dDelta.
					// We will create an ordering key composed of dDelta and Dplmin.
					dKey = pDpC->GetDplmin() * 100.0 + dDelta;
				}

				if( false == bInsertInDplmaxTooLow )
				{
					mapDeltaTAP.insert( std::make_pair( dKey, pDpC ) );
				}
				else
				{
					m_mapDpCDplmaxTooLow.insert( std::make_pair( dKey, pDpC ) );
				}
			}
		}
	}

	bFound = false;
	CDB_TAProduct *pCurrentTAP = GetpTAP();

	int iCurrentSize = -1;

	// When user change the current type, version, connection , PN, ... the current selected product is invalid
	// Try to find matching (connection, version, ...) valve in the full list of suggested products
	if( true == bUserChangeSelParameters )
	{
		pCurrentTAP = NULL;
	}

	if( NULL != pCurrentTAP )
	{
		iCurrentSize = pCurrentTAP->GetSizeKey();
	}

	// Build Sorting keys
	// Tech Param below DN65
	// idType is already fix by the filling of the full list
	CDB_StringID *psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBlw65TypeID ).MP );
	CDB_StringID *psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBlw65FamilyID ).MP );
	CDB_StringID *psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBlw65BdyMatID ).MP );
	CDB_StringID *psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBlw65ConnectID ).MP );
	CDB_StringID *psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBlw65VersionID ).MP );
	CDB_StringID *psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBlw65PNID ).MP );
	CTAPSortKey sKeyTechParamBlw65( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );

	// Tech Param above DN50
	// idType is already fix by the filling of the full list
	psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCAbv50TypeID ).MP );
	psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCAbv50FamilyID ).MP );
	psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCAbv50BdyMatID ).MP );
	psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCAbv50ConnectID ).MP );
	psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCAbv50VersionID ).MP );
	psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCAbv50PNID ).MP );
	CTAPSortKey sKeyTechParamAbv50( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );

	CTAPSortKey sKeyUserChangeSelParams;

	double dKeyCurrentTAP = 0;
	int iStartLoop = ( true == bUserChangeSelParameters ) ? iStartLoop = -1 : iStartLoop = 0;
	int iEndLoop = 7;

	_TMapDelta *pMap = NULL;

	if( true == bBypassStabilized && 0 == mapDeltaTAP.size() && m_mapDpCDplmaxTooLow.size() > 0 )
	{
		pMap = &m_mapDpCDplmaxTooLow;
	}
	else
	{
		pMap = &mapDeltaTAP;
	}

	_TMapDelta::iterator It = pMap->end();

	if( pMap->size() > 0 )
	{
		for( int UserPrefLoop = iStartLoop; UserPrefLoop < iEndLoop && false == bFound; UserPrefLoop++ )
		{
			// This loop give us the possibility to relax mask criteria one by one
			// we will start with the most constraining key and if we don't found a product we will relax constrains one by one
			CTAPSortKey::TAPSortKeyMask eMask = ( CTAPSortKey::TAPSortKeyMask )( CTAPSortKey::TAPSKM_All  & ~CTAPSortKey::TAPSKM_PriorL & ~CTAPSortKey::TAPSKM_Size
					& ~CTAPSortKey::TAPSKM_Fam );

			if( NULL != GetpTAP() )
			{
				eMask = ( CTAPSortKey::TAPSortKeyMask )( CTAPSortKey::TAPSKM_All  & ~CTAPSortKey::TAPSKM_PriorL & ~CTAPSortKey::TAPSKM_Size );
			}

			switch( UserPrefLoop )
			{
				case -1:
				{
					// Body material is not available as a user choice (combo doesn't exist).
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Bdy );
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Fam );
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Version );

					if( NULL == m_pUserChoiceDpCPN )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
					}

					if( NULL == m_pUserChoiceDpCConn )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Connect );
					}

					if( NULL == m_pUserChoiceDpCType )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Type );
					}

					sKeyUserChangeSelParams.Init( m_pUserChoiceDpCType, NULL, NULL, NULL, m_pUserChoiceDpCConn, NULL, m_pUserChoiceDpCPN, 0 );
				}
				break;

				case 0:
				{
					// All
					if( true == bUserChangeSelParameters )
					{
						// No solution found relax user request
						bUserChangeSelParameters = false;
						ClearUserChoiceTemporaryVariables();
					}
				}
				break;

				case 1:
				{
					// Relax PN
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
				}
				break;

				case 2:
				{
					// Relax PN and connection
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN  & ~CTAPSortKey::TAPSKM_Connect );
				}
				break;

				case 3:
				{
					// Relax Type, PN and connection
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN  & ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Type );
				}
				break;

				case 4:
				{
					// Relax Type, PN and connection, Bdy
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN  & ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Type &
							~CTAPSortKey::TAPSKM_Bdy );
				}
				break;

				case 5:
				{
					// Relax Type, PN and connection, Bdy, Version
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN  & ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Type & ~CTAPSortKey::TAPSKM_Bdy
							& ~CTAPSortKey::TAPSKM_Version );
				}
				break;

				case 6:
				{
					// Relax Type, PN and connection, Bdy, Version
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN  & ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Type & ~CTAPSortKey::TAPSKM_Bdy
							& ~CTAPSortKey::TAPSKM_Version & ~CTAPSortKey::TAPSKM_Fam );
				}
				break;
			}

			double dKeyTechParamBlw65 = sKeyTechParamBlw65.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
			double dKeyTechParamAbv50 = sKeyTechParamAbv50.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
			double dKeyUserChangeSelParams = sKeyUserChangeSelParams.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );

			// Extract key for currently selected TAP.
			if( NULL != pCurrentTAP )
			{
				dKeyCurrentTAP = pCurrentTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
			}

			if( true == bUserChangeSelParameters )
			{
				_TMapDelta::iterator ItBestPriority = pMap->end();

				for( It = pMap->begin(); It != pMap->end(); ++It )
				{
					CDB_TAProduct *pTAP = It->second;
					double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );

					if( dTAPKey == dKeyUserChangeSelParams )
					{
						if( ItBestPriority != pMap->end() )
						{
							if( ItBestPriority->second->GetPriorityLevel() > pTAP->GetPriorityLevel() )
							{
								ItBestPriority = It;
								bFound = true;
							}
						}
						else
						{
							ItBestPriority = It;
							bFound = true;
						}
					}
				}

				if( true == bFound )
				{
					It = ItBestPriority;
				}
			}
			else
			{
				// If a current selection exist
				if( NULL != pCurrentTAP )
				{
					// Try to find existing selected DpC
					for( It = pMap->begin(); It != pMap->end(); ++It )
					{
						if( pCurrentTAP == It->second )
						{
							bFound = true;
						}

						if( true == bFound )
						{
							// Abort loop by this way we preserve Iterator
							break;
						}
					}

					if( false == bFound )
					{
						for( It = pMap->begin(); It != pMap->end(); ++It )
						{
							CDB_TAProduct *pTAP = It->second;
							int iSize = pTAP->GetSizeKey();
							double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );

							if( ( iCurrentSize <= iDN50 && iSize <= iDN50 ) || ( iCurrentSize > iDN50 && iSize > iDN50 ) )
							{
								// Selected valve exist and it's located on the same side of iDN50 border.
								// Try to find a valve that match all requested criteria.
								if( dTAPKey == dKeyCurrentTAP )
								{
									bFound = true;
								}
							}
							else
							{
								// Selected valve is not located on the same side of iDN50 border.
								// Use defined technical choice
								if( iSize <= iDN50 )
								{
									if( dTAPKey == dKeyTechParamBlw65 )
									{
										bFound = true;
									}
								}
								else
								{
									// Size > DN50
									if( dTAPKey == dKeyTechParamAbv50 )
									{
										bFound = true;
									}
								}
							}

							if( true == bFound )
							{
								// Abort loop by this way we preserve Iterator
								break;
							}
						}
					}
				}

				// Not found or previous valve doesn't exist, restart loop only on tech params
				if( false == bFound )
				{
					for( It =  pMap->begin(); It !=  pMap->end(); ++It )
					{
						CDB_TAProduct *pTAP = It->second;
						int iSize = pTAP->GetSizeKey();
						double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );

						// Selected valve is not located on the same side of iDN50 border.
						// Use defined technical choice
						if( iSize <= iDN50 )
						{
							if( dTAPKey == dKeyTechParamBlw65 )
							{
								bFound = true;
							}
						}
						else
						{
							// Size > DN50
							if( dTAPKey == dKeyTechParamAbv50 )
							{
								bFound = true;
							}
						}

						if( true == bFound )
						{
							// Abort loop and by this way we preserve Iterator
							break;
						}
					}
				}
			}
		}
	}

	// If best valve found...
	if( true == bFound )
	{
		CDB_TAProduct *pTAP = It->second;

		if( NULL != pTAP )
		{
			SetBestDpCIDPtr( pTAP->GetIDPtr() );
			_SetpSelDpC( (LPARAM)pTAP );

			if( true == bUserChangeSelParameters )
			{
				// When the current product is valid we will analyze if the user introduced an exception regarding defined PrjParam,
				// in this case we will not consider the DN50 border that force an automatic jump.
				if( pTAP->GetSizeKey() <= iDN50 )
				{
					if( ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBlw65TypeID ).MP == pTAP->GetTypeIDPtr().MP ) &&
						( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBlw65ConnectID ).MP == pTAP->GetConnectIDPtr().MP ) &&
						( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBlw65PNID ).MP == pTAP->GetPNIDPtr().MP ) )
					{
						ClearUserChoiceTemporaryVariables();
					}
				}
				else
				{
					// DN65 and above
					if( ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCAbv50TypeID ).MP == pTAP->GetTypeIDPtr().MP ) &&
						( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCAbv50ConnectID ).MP == pTAP->GetConnectIDPtr().MP ) &&
						( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCAbv50PNID ).MP == pTAP->GetPNIDPtr().MP ) )
					{
						ClearUserChoiceTemporaryVariables();
					}
				}
			}
		}
		else
		{
			_AbortSelectBestDpC();
		}
	}
	else
	{
		_AbortSelectBestDpC();
	}

	return;
}

void CDS_HydroMod::CDpC::_AbortSelectBestDpC()
{
	if( false == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eDpC ) || 0 == *GetIDPtr().ID )
	{
		InvalidateSelection();
	}

	ClearUserChoiceTemporaryVariables();
}

void CDS_HydroMod::CDpC::ClearUserChoiceTemporaryVariables()
{
	m_pUserChoiceDpCType = NULL;
	m_pUserChoiceDpCConn = NULL;
	m_pUserChoiceDpCPN = NULL;
}

bool CDS_HydroMod::CDpC::IsDpCExistInPreselectedList( CDB_TAProduct *pclDpControllerValve )
{
	if( 0 == (int)m_mapDpC.size() )
	{
		return false;
	}

	bool bDpCExist = false;

	for( auto &iter : m_mapDpC )
	{
		if( pclDpControllerValve == iter.second.GetpDpC() )
		{
			bDpCExist = true;
			break;
		}
	}

	return bDpCExist;
}

bool CDS_HydroMod::CDpC::IsCompletelyDefined()
{
	if( NULL == GetIDPtr().MP )				// safe IDPtr, checked by Extend in GetIDPtr()
	{
		return false;
	}

	if( 0.0 == GetDpCSetting() )
	{
		return false;
	}

	if( 0.0 == GetDp() )
	{
		return false;
	}

	if( 0.0 == GetDpL() )
	{
		return false;
	}

	return true;
}

IDPTR CDS_HydroMod::CDpC::GetBestDpCIDPtr()
{
	if( _T('\0') != *m_IDPtrBestDpC.ID )
	{
		m_IDPtrBestDpC.DB = TASApp.GetpTADB();
		Extend( &m_IDPtrBestDpC );
		return m_IDPtrBestDpC;
	}

	return _NULL_IDPTR;
}

bool CDS_HydroMod::CDpC::IsBestDpC( CData *pData )
{
	return ( pData == GetBestDpCIDPtr().MP ) ? true : false;
}

double CDS_HydroMod::CDpC::GetDp()
{
	if( NULL ==  m_pParent->GetpSch() )
	{
		ASSERTA_RETURN( 0.0 );
	}

	double dHAvail = m_pParent->GetHAvail( CAnchorPt::CircuitSide_Primary );
	double dPipeDp = m_pParent->GetpCircuitPrimaryPipe()->GetPipeDp();
	double dSingularDp = m_pParent->GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true );

	if( true == m_pParent->IsaModule() && false == m_pParent->GetpSchcat()->IsSecondarySideExist() )	   // Circuit pipe doesn't exist for a module
	{
		dPipeDp = dSingularDp = 0.0;
	}

	double dBVDp = 0.0;

	if( eMvLoc::MvLocPrimary == GetMvLoc() )
	{
		if( true == m_pParent->IsBvExist( true ) )
		{
			// HYS-1930: we are sure here to take the BV on the circuit and not on the bypass.
			// Because for auto-adapting variable flow decoupling circuit the MV loc is on the secondary.
			dBVDp = m_pParent->GetpBv()->GetDp();
		}
	}

	double dTUDp = m_pParent->GetDpOnOutOfPrimSide();
	double dDpL = max( GetDpL(), GetDplmin() );
	double dDpCV = 0.0;

	// When DpC stabilize on branch DpL contains TU Dp.
	if( eDpStab::DpStabOnBranch == m_pParent->GetpSch()->GetDpStab() )
	{
		dTUDp = 0.0;
	}

	double Dp = dHAvail - ( dDpL + dTUDp + dBVDp + dDpCV + dPipeDp + dSingularDp );

	return max( Dp, GetDpmin() );
}

double CDS_HydroMod::CDpC::GetDpL()
{
	return GetDpToStab();
}

double CDS_HydroMod::CDpC::GetDplmin()
{
	double dDplmin = 0.0;

	if( _T('\0') != *GetIDPtr().ID )
	{
		dDplmin = ( (CDB_DpController *) GetIDPtr().MP )->GetDplmin();

		if( -1.0 == dDplmin )
		{
			dDplmin = 0.0;
		}
	}

	return dDplmin;
}

double CDS_HydroMod::CDpC:: GetDplmax()
{
	double dDplmax = 0.0;

	if( _T('\0') != *GetIDPtr().ID )
	{
		dDplmax = ( (CDB_DpController *) GetIDPtr().MP )->GetDplmax();

		if( -1.0 == dDplmax )
		{
			dDplmax = 0.0;
		}
	}

	return dDplmax;
}

// Minimum Dp for DpC
double CDS_HydroMod::CDpC::GetDpmin( bool fBounds )
{
	if( 0 == *GetIDPtr().ID )
	{
		return 0.0;
	}

	double dDp = m_pParent->GetpTechParam()->GetDpCMinDp();
	double dDpMin = 0.0;

	// Cast the pointer on device.
	CDB_DpController *pDev = static_cast< CDB_DpController * >( GetIDPtr().MP );

	// Get Kv characteristic of the valve.
	CDB_DpCCharacteristic *pDevChar = pDev->GetDpCCharacteristic();

	if( NULL == pDevChar )
	{
		return 0.0;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eDpC );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	double dRho = pclWaterChar->GetDens();

	if( GetQ() > pDevChar->GetQmin( dRho ) && GetQ() < pDevChar->GetQmax( dRho ) )
	{
		pDevChar->DpCSolve( GetQ(), &dDpMin, dRho );
	}

	if( true == fBounds )
	{
		return max( dDp, dDpMin );
	}

	return dDpMin;
}

eBool3 CDS_HydroMod::CDpC::CheckDpMinMax()
{
	if( NULL == GetIDPtr().MP )
	{
		return eb3Undef;
	}

	if( GetDp() < ( m_pParent->GetpTechParam()->GetDpCMinDp() * 0.99 ) )
	{
		return eb3False;
	}

	return eb3True;
}

eBool3 CDS_HydroMod::CDpC::CheckDpLRange( CDB_DpController *pDpC )
{
	if( NULL == pDpC )
	{
		pDpC = static_cast< CDB_DpController * >( GetIDPtr().MP );
	}

	if( NULL == pDpC || -1.0 == pDpC->GetDplmin() || -1.0 == pDpC->GetDplmax() )
	{
		return eb3Undef;
	}

	double dDpl = GetDpL();

	if( dDpl < ( pDpC->GetDplmin() * 0.99 ) || dDpl > ( pDpC->GetDplmax() * 1.01 ) )
	{
		return eb3False;
	}

	return eb3True;
}

CString CDS_HydroMod::CDpC::GetSettingStr( bool fWithUnit, bool *pfFullSetting )
{
	if( NULL != pfFullSetting )
	{
		*pfFullSetting = false;
	}

	CString str( _T("") );

	if( NULL != GetIDPtr().MP )
	{
		CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( GetIDPtr().MP );

		if( NULL != pDpC )
		{
			CDB_DpCCharacteristic *pChar = pDpC->GetDpCCharacteristic();

			if( NULL != pChar )
			{
				str = pChar->GetSettingString( GetDpCSetting(), fWithUnit, pfFullSetting );
			}
		}
	}

	return str;
}

int CDS_HydroMod::CDpC::CheckValidity()
{
	m_usValidityFlags = CDS_HydroMod::eValidityFlags::evfOK;

	if( NULL == GetIDPtr().MP )
	{
		return m_usValidityFlags;
	}

	CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( GetIDPtr().MP );

	if( NULL == pDpC || NULL == pDpC->GetDpCCharacteristic() )
	{
		return m_usValidityFlags;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eDpC );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( m_usValidityFlags );
	}

	if( pDpC->GetTmax() < pclWaterChar->GetTemp() )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfTempTooHigh;
	}

	if( GetDp() < m_pParent->GetpTechParam()->GetDpCMinDp() )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDpTooLow;
	}

	if( GetQ() > pDpC->GetDpCCharacteristic()->GetQmax( pclWaterChar->GetDens() ) )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfFlowTooHigh;
	}

	if( GetQ() < pDpC->GetDpCCharacteristic()->GetQmin( pclWaterChar->GetDens(), GetDpL() ) )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfFlowTooLow;
	}

	if( pDpC->GetDpmax() > 0 )
	{
		if( m_pParent->GetpPressIntHM()->GetHAvail( CAnchorPt::CircuitSide_Secondary ) > pDpC->GetDpmax() )
		{
			m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfMaxDpTooHigh;
		}
	}

	return m_usValidityFlags;
}

void CDS_HydroMod::CDpC::DropData( OUTSTREAM outf, int *piLevel )
{
	CString strTab = _T("");

	for( int i = *piLevel; i; i-- )
	{
		strTab += _T("\t");
	}

	CString str;
	str.Format( _T("m_IDPtr.ID = %s\n"), GetIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_DiversityIDPtr.ID = %s\n"), GetDiversityIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_IDPtrBestDpC.ID = %s\n"), GetBestDpCIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_IDPtrSelMv.ID = %s\n"), m_IDPtrSelMv.ID );
	WriteFormatedStringA2( outf, str, strTab );

	_DropDataHelp( str, GetpSelDpCType(), _T("DpCType") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, GetpSelDpCConn(), _T("DpCConn") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, GetpSelDpCPN(), _T("DpCPN") );
	WriteFormatedStringA2( outf, str, strTab );

	_DropDataHelp( str, m_pUserChoiceDpCType, _T("m_pUserChoiceDpCType") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, m_pUserChoiceDpCConn, _T("m_pUserChoiceDpCConn") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, m_pUserChoiceDpCPN, _T("m_pUserChoiceDpCPN") );
	WriteFormatedStringA2( outf, str, strTab );

	str = _T("m_bMvLoc = ");

	switch( GetMvLoc() )
	{
		case eMvLoc::MvLocPrimary:
			str.Append( _T("MvLocPrimary\n") );
			break;

		case eMvLoc::MvLocSecondary:
			str.Append( _T("MvLocSecondary\n") );
			break;

		case eMvLoc::MvLocNone:
			str.Append( _T("MvLocNone\n") );
			break;
	}

	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_bMvLocLocked = %s\n" ), ( true == HasMvLockLocked() ) ? _T("true") : _T( "false") );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Q = %s\n"), WriteCUDouble( _U_FLOW, GetQ(), true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_usValidityFlags = %u\n"), GetValidtyFlags() );
	WriteFormatedStringA2( outf, str, strTab );
	WriteFormatedStringA2( outf, _T("\n"), strTab );
}

int CDS_HydroMod::CDpC::GetAvailableType( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapDpC.size() )
	{
		return 0;
	}

	pmap->clear();
	std::multimap <int, CDpCData >::iterator It;

	for( It = m_mapDpC.begin(); It != m_mapDpC.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpDpC();
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetTypeIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

int CDS_HydroMod::CDpC::GetAvailableConnections( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapDpC.size() )
	{
		return 0;
	}

	pmap->clear();

	CDB_StringID *pStrIDType = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDType = dynamic_cast<CDB_StringID *>( GetpTAP()->GetTypeIDPtr().MP );
	}

	std::multimap <int, CDpCData >::iterator It;

	for( It = m_mapDpC.begin(); It != m_mapDpC.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpDpC();
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetTypeIDPtr().MP );

		if( pStrID != pStrIDType )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

int CDS_HydroMod::CDpC::GetAvailablePN( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapDpC.size() )
	{
		return 0;
	}

	pmap->clear();

	CDB_StringID *pStrIDType = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDType = dynamic_cast<CDB_StringID *>( GetpTAP()->GetTypeIDPtr().MP );
	}

	CDB_StringID *pStrIDConnection = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDConnection = dynamic_cast<CDB_StringID *>( GetpTAP()->GetConnectIDPtr().MP );
	}

	std::multimap <int, CDpCData >::iterator It;

	for( It = m_mapDpC.begin(); It != m_mapDpC.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpDpC();

		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetTypeIDPtr().MP );

		if( pStrID != pStrIDType )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );

		if( pStrID != pStrIDConnection )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetPNIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

void CDS_HydroMod::CDpC::SetMvLoc( eMvLoc MvLoc )
{
	// Find correct scheme ID
	// Extract list of compliant circuits
	CTable *pTab = dynamic_cast<CTable *>( TASApp.GetpTADB()->Get( _T("CIRCSCHEME_TAB") ).MP );

	if( NULL == pTab || NULL == m_pParent->GetpSch() )
	{
		ASSERT_RETURN;
	}

	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		CDB_CircuitScheme *pSch = dynamic_cast<CDB_CircuitScheme *>( idptr.MP );

		if( NULL == pSch )
		{
			continue;
		}

		if( pSch->GetpSchCateg() == m_pParent->GetpSch()->GetpSchCateg()
				&& pSch->GetBalType() == m_pParent->GetpSch()->GetBalType()
				&& pSch->GetDpCType() == m_pParent->GetpSch()->GetDpCType()
				&& pSch->GetDpStab() == m_pParent->GetpSch()->GetDpStab()
				&& pSch->GetTermUnit() == m_pParent->GetpSch()->GetTermUnit()
				&& pSch->GetCvFunc() == m_pParent->GetpSch()->GetCvFunc()
				&& pSch->GetCvLoc() == m_pParent->GetpSch()->GetCvLoc()
				&& pSch->GetDpCLoc() ==  m_pParent->GetpSch()->GetDpCLoc()
				&& pSch->GetMvLoc() == MvLoc )
		{
			m_pParent->SetSchemeID( idptr.ID );
			m_fMvLoc = MvLoc;
			m_pParent->Modified();
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////
CDS_HydroMod::CShutoffValve::CShutoffValve( CDS_HydroMod *pHM, eHMObj eLocated ) : CBase( pHM )
{
	// No Selection from TADB.
	SetIDPtr( _NULL_IDPTR );
	SetBestIDPtr( _NULL_IDPTR );
	SetDiversityIDPtr( _NULL_IDPTR );
	ClearUserChoiceTemporaryVariables();
	m_eLocated = eLocated;
	m_dQ = 0.0;
	m_dDp = 0.0;
}

CDS_HydroMod::CShutoffValve::~CShutoffValve()
{
}

void CDS_HydroMod::CShutoffValve::Copy( CDS_HydroMod::CShutoffValve *pclTargetHMShutoffValve, bool bResetDiversityFactor )
{
	pclTargetHMShutoffValve->SetLocate( GetHMObjectType() );
	pclTargetHMShutoffValve->SetQ( GetQ() );
	pclTargetHMShutoffValve->SetDp( GetDp() );
	pclTargetHMShutoffValve->SetIDPtr( GetIDPtr() );
	pclTargetHMShutoffValve->SetBestIDPtr( GetBestIDPtr() );
	pclTargetHMShutoffValve->SetDiversityIDPtr( ( true == bResetDiversityFactor ) ? _NULL_IDPTR : GetDiversityIDPtr() );
	pclTargetHMShutoffValve->SetValidtyFlags( GetValidtyFlags() );

	if (NULL == m_pUserChoiceType && NULL == m_pUserChoiceConn && NULL == m_pUserChoiceVersion && NULL == m_pUserChoicePN )
	{
		pclTargetHMShutoffValve->ClearUserChoiceTemporaryVariables();
	}
	else
	{
		pclTargetHMShutoffValve->SetpSelTypeAccordingUserChoice(m_pUserChoiceType);
		pclTargetHMShutoffValve->SetpSelConnAccordingUserChoice(m_pUserChoiceConn);
		pclTargetHMShutoffValve->SetpSelVersionAccordingUserChoice(m_pUserChoiceVersion);
		pclTargetHMShutoffValve->SetpSelPNAccordingUserChoice(m_pUserChoicePN);
	}
}

bool CDS_HydroMod::CShutoffValve::IsShutoffValveExistInPreselectedList( CDB_TAProduct *pclShutoffValve )
{
	if( 0 == (int)m_mapShutoffValve.size() )
	{
		return false;
	}

	bool bShutoffValveExist = false;

	for( auto &iter : m_mapShutoffValve )
	{
		if( pclShutoffValve == iter.second.GetpShutoffValve() )
		{
			bShutoffValveExist = true;
			break;
		}
	}

	return bShutoffValveExist;
}

#define CDS_HYDROMOD_CSHUTOFFVALVE_VERSION	2
// Version 2: 2016-07-11: use choice variables added.
void CDS_HydroMod::CShutoffValve::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_CSHUTOFFVALVE_VERSION;
	WriteData<>( outf, Version );

	// Info.
	WriteString( outf, m_IDPtr.ID );
	WriteString( outf, m_IDPtrBest.ID );
	WriteData<>( outf, m_eLocated );
	WriteData<>( outf, m_dQ );

	// Version 2.
	CString strID;
	strID = ( NULL != m_pUserChoiceType ) ? m_pUserChoiceType->GetIDPtr().ID : L"";
	WriteString( outf, strID );
	strID = ( NULL != m_pUserChoiceConn ) ? m_pUserChoiceConn->GetIDPtr().ID : L"";
	WriteString( outf, strID );
	strID = ( NULL != m_pUserChoiceVersion ) ? m_pUserChoiceVersion->GetIDPtr().ID : L"";
	WriteString( outf, strID );
	strID = ( NULL != m_pUserChoicePN ) ? m_pUserChoicePN->GetIDPtr().ID : L"";
	WriteString( outf, strID );

	WriteData<>( outf, GetDp() );
	WriteString( outf, m_DiversityIDPtr.ID );
}

bool CDS_HydroMod::CShutoffValve::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_CSHUTOFFVALVE_VERSION;
	ReadData<>( inpf, Version );

	if( Version < 1 || Version > CDS_HYDROMOD_CSHUTOFFVALVE_VERSION )
	{
		return false;
	}

	// Info.
	ReadString( inpf, m_IDPtr.ID, sizeof( m_IDPtr.ID ) );
	ReadString( inpf, m_IDPtrBest.ID, sizeof( m_IDPtrBest.ID ) );
	ReadData<>( inpf, m_eLocated );
	ReadData<>( inpf, m_dQ );
	m_IDPtr.DB = TASApp.GetpTADB();
	m_IDPtrBest.DB = TASApp.GetpTADB();

	ClearUserChoiceTemporaryVariables();

	if( Version < 2 )
	{
		return true;
	}

	// Version 2.
	IDPTR idptr;

	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceType = dynamic_cast<CDB_StringID *>( idptr.MP );

	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceConn = dynamic_cast<CDB_StringID *>( idptr.MP );

	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceVersion = dynamic_cast<CDB_StringID *>( idptr.MP );

	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoicePN = dynamic_cast<CDB_StringID *>( idptr.MP );

	ReadData<>( inpf, m_dDp );
	ReadString( inpf, m_DiversityIDPtr.ID, sizeof( m_DiversityIDPtr.ID ) );

	return true;
}

void CDS_HydroMod::CShutoffValve::InvalidateSelection()
{
	if( true == GetpParentHM()->IsLocked( GetHMObjectType() ) )
	{
		return;
	}
	
	m_IDPtr = _NULL_IDPTR;
	m_IDPtrBest = _NULL_IDPTR;
}

eBool3 CDS_HydroMod::CShutoffValve::CheckDpMinMax()
{
	if( NULL == GetIDPtr().MP )
	{
		return eb3Undef;
	}

	// Just compare the Dpmax. A shutoff valve is always full opened and due to its physical shape, the valve
	// has a very low resistance to the media. This is why pressure drop through this valve can be very low.
	if( GetDp() > m_pParent->GetpTechParam()->VerifyValvMaxDp( (CDB_TAProduct *)GetIDPtr().MP ) )
	{
		return eb3False;
	}

	return eb3True;
}

IDPTR CDS_HydroMod::CShutoffValve::GetDiversityIDPtr()
{
	if( _T('\0') != *m_DiversityIDPtr.ID && NULL != m_DiversityIDPtr.DB )
	{
		Extend( &m_DiversityIDPtr );
		return m_DiversityIDPtr;
	}

	return _NULL_IDPTR;
}

void CDS_HydroMod::CShutoffValve::SetIDPtr( IDPTR idptr )
{
	m_IDPtr = idptr;

	if( _T('\0') != *m_IDPtr.ID && NULL != m_IDPtr.DB )
	{
		Extend( &m_IDPtr );
	}

	m_pParent->Modified();
}

IDPTR CDS_HydroMod::CShutoffValve::SelectBestShutoffValve()
{
	if( GetQ() <= 0.0 )
	{
		_AbortSelectBestShutoffValve();
		return GetIDPtr();
	}

	if( _T('\0') != *GetIDPtr().ID && true == m_pParent->IsLocked( GetHMObjectType() ) )
	{
		_AbortSelectBestShutoffValve();
		return GetIDPtr();
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( GetHMObjectType() );
	
	if( NULL == pclWaterChar )
	{
		_AbortSelectBestShutoffValve();
		ASSERTA_RETURN( GetIDPtr() );
	}

	int iHighestSize = 0;
	int iLowestSize = 0;

	// Shut-Off valve selection is based on the pipe size.
	CAnchorPt::PipeLocation ePipeLocation = m_pParent->GetpSch()->GetAnchorPtPipeLocation( CAnchorPt::eFunc::ShutoffValve );
	CDB_Pipe *pPipe = m_pParent->GetPipeSizeShift( iHighestSize, iLowestSize, CAnchorPt::CircuitSide_Primary, ePipeLocation );

	if( NULL == pPipe )
	{
		_AbortSelectBestShutoffValve();
		return GetIDPtr();
	}

	bool bForHub = m_pParent->IsForHub();
	bool bForHubStation = false;

	if( true == bForHub && true == m_pParent->IsClass( CLASS( CDS_HmHubStation ) ) )
	{
		bForHubStation = true;
	}

	m_mapShutoffValve.clear();
	std::multimap <int, CShutoffValveData>::iterator itMapShutoffValve;

	CString str;
	CTable *pTab = static_cast<CTable *>( TASApp.GetpTADB()->Get( _T("SHUTOFF_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( IDPTR idptr = pTab->GetFirst(); _T('\0') != *idptr.ID; idptr = pTab->GetNext() )
	{
		CDB_ShutoffValve *pclShutoffValve = static_cast< CDB_ShutoffValve * >( idptr.MP );

		if( false == pclShutoffValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == bForHub && false == pclShutoffValve->IsAvailable( bForHub ) )
		{
			continue;
		}

		// If we work with a TA-COMPACT-DP we can have only the STS shut-off valve.
		if( NULL != m_pParent->GetpSch() && 0 == wcscmp( m_pParent->GetpSch()->GetIDPtr().ID, _T("2WD_DPCBCV") ) )
		{
			if( 0 != CString( pclShutoffValve->GetFamilyID() ).Compare( _T("FAM_STS") )
					&& 0 != CString( pclShutoffValve->GetFamilyID() ).Compare( _T("FAM_STS*") ) )
			{
				continue;
			}
		}

		int iProdSizeKey = pclShutoffValve->GetSizeKey();
		itMapShutoffValve = m_mapShutoffValve.insert( std::make_pair( iProdSizeKey, CShutoffValveData( pclShutoffValve ) ) );
	}

	if( 0 == m_mapShutoffValve.size() )
	{
		_AbortSelectBestShutoffValve();
		return GetIDPtr();
	}

	// HYS-1308: Change the way we reject sizes.
	std::multimap <int, CShutoffValveData>::iterator itLow = m_mapShutoffValve.end();
	std::multimap <int, CShutoffValveData>::iterator itUp = m_mapShutoffValve.end();

	for( std::multimap <int, CShutoffValveData>::iterator iter = m_mapShutoffValve.begin(); iter != m_mapShutoffValve.end(); iter++ )
	{
		if( iter->first >= iLowestSize )
		{
			itLow = iter;
			break;
		}
	}

	if( m_mapShutoffValve.end() == itLow )
	{
		itLow = m_mapShutoffValve.lower_bound( ( *m_mapShutoffValve.rbegin() ).first );
	}

	bool bFound = false;

	for( std::multimap <int, CShutoffValveData>::reverse_iterator riter = m_mapShutoffValve.rbegin(); riter != m_mapShutoffValve.rend(); riter++ )
	{
		if( riter->first <= iHighestSize )
		{
			itUp = riter.base();
			bFound = true;
			break;
		}
	}

	if( m_mapShutoffValve.end() == itUp && false == bFound )
	{
		itUp = m_mapShutoffValve.upper_bound( (*m_mapShutoffValve.begin()).first );
	}

	if( m_mapShutoffValve.end() != itLow && m_mapShutoffValve.begin() != itLow )
	{
		m_mapShutoffValve.erase( m_mapShutoffValve.begin(), itLow );
	}

	if( m_mapShutoffValve.end() != itUp )
	{
		m_mapShutoffValve.erase( itUp, m_mapShutoffValve.end() );
	}

	if( 0 == m_mapShutoffValve.size() )
	{
		_AbortSelectBestShutoffValve();
		return GetIDPtr();
	}

	int iDN50 = m_pParent->GetDN50();
	int iDN = pPipe->GetSizeKey( TASApp.GetpTADB() );

	CDB_StringID *psidType = NULL;
	CDB_StringID *psidFam = NULL;
	CDB_StringID *psidBdy = NULL;
	CDB_StringID *psidConn = NULL;
	CDB_StringID *psidVers = NULL;
	CDB_StringID *psidPN = NULL;

	// Apply user preferences.
	bool bEndLoop = false;

	for( int i = 0; i < 2 && false == bEndLoop; i++ )
	{
		switch( i )
		{
			// If a selected device exist try to find a device with same parameters (if we are in the same side of DN50 border).
			case 0:
				if( NULL != GetIDPtr().MP )
				{
					CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve *>( GetIDPtr().MP );

					if( NULL != pclShutoffValve )
					{
						psidType = dynamic_cast<CDB_StringID *>( pclShutoffValve->GetTypeIDPtr().MP );
						psidFam	= dynamic_cast<CDB_StringID *>( pclShutoffValve->GetFamilyIDPtr().MP );
						psidBdy = dynamic_cast<CDB_StringID *>( pclShutoffValve->GetBodyMaterialIDPtr().MP );
						psidConn = dynamic_cast<CDB_StringID *>( pclShutoffValve->GetConnectIDPtr().MP );
						psidVers = dynamic_cast<CDB_StringID *>( pclShutoffValve->GetVersionIDPtr().MP );
						psidPN = dynamic_cast<CDB_StringID *>( pclShutoffValve->GetPNIDPtr().MP );

						// If user force a new connection...
						if( NULL != m_pUserChoiceConn && m_pUserChoiceConn != psidConn )
						{
							psidType = NULL;
							psidBdy = NULL;
							psidFam = NULL;
							psidConn = m_pUserChoiceConn;
							psidVers = NULL;
							psidPN = NULL;
						}

						// If user force a new version...
						if( NULL != m_pUserChoiceVersion && m_pUserChoiceVersion != psidVers )
						{
							psidType = NULL;
							psidBdy = NULL;
							psidFam = NULL;
							psidVers = m_pUserChoiceVersion;
							psidPN = NULL;
						}

						// If user force a new PN...
						if( NULL != m_pUserChoicePN && m_pUserChoicePN != psidPN )
						{
							psidType = NULL;
							psidBdy = NULL;
							psidFam = NULL;
							psidPN = m_pUserChoicePN;
						}

						// Clear all specific user request.
						m_pUserChoiceConn = NULL;
						m_pUserChoiceVersion = NULL;
						m_pUserChoicePN = NULL;
						bEndLoop = true;
					}
				}

				break;

			// Default preferences from tech params.
			case 1:
			{
				CPrjParams *pPrjParams = m_pParent->GetpPrjParam()->GetpHmCalcParams();

				if( iDN <= iDN50 )
				{
					psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65TypeID ).MP );
					psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65FamilyID ).MP );
					psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65BdyMatID ).MP );
					psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65ConnectID ).MP );
					psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65VersionID ).MP );
					psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65PNID ).MP );
				}
				else
				{
					psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50TypeID ).MP );
					psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50FamilyID ).MP );
					psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50BdyMatID ).MP );
					psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50ConnectID ).MP );
					psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50VersionID ).MP );
					psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50PNID ).MP );
				}

				bEndLoop = true;
			}
			break;

		}
	}

	CTAPSortKey sKey( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );
	IDPTR BestSOIDPtr = _NULL_IDPTR;
	int iDelta = INT_MAX;
	double dSOSizeKey = 0;
	int iPipeSizeKey = pPipe->GetSizeKey( TASApp.GetpTADB() );

	// Don't take care of ePriority.
	UINT uiMask = CTAPSortKey::TAPSKM_All & ~CTAPSortKey::TAPSKM_PriorL;

	// Mask empty fields.
	uiMask = uiMask & ~CTAPSortKey::TAPSKM_Size;

	if( NULL == psidType )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Type;
	}

	if( NULL == psidFam )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Fam;
	}

	if( NULL == psidBdy )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Bdy;
	}

	if( NULL == psidConn )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Connect;
	}

	if( NULL == psidVers )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Version;
	}

	if( NULL == psidPN )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_PN;
	}

	for( itMapShutoffValve = m_mapShutoffValve.begin(); itMapShutoffValve != m_mapShutoffValve.end(); ++itMapShutoffValve )
	{
		CTAPSortKey mKey( ( CDB_TAProduct * )itMapShutoffValve->second.GetpShutoffValve() );
		int iRet = mKey.Compare( &sKey, &CTAPSortKey::CTAPSKMask( ( CTAPSortKey::TAPSortKeyMask )uiMask ) );

		if( 0 == iRet )
		{
			CDB_ShutoffValve *pclShutoffValve = ( CDB_ShutoffValve * )itMapShutoffValve->second.GetpShutoffValve();

			if( NULL == pclShutoffValve )
			{
				continue;
			}

			dSOSizeKey = pclShutoffValve->GetSizeKey();

			if( abs( ( ( int ) dSOSizeKey ) - iPipeSizeKey ) < iDelta )
			{
				iDelta = abs( ( ( int ) dSOSizeKey ) - iPipeSizeKey );
				BestSOIDPtr = pclShutoffValve->GetIDPtr();
			}

			if( NULL != pclShutoffValve->GetValveCharacteristic() )
			{
				double dRho = pclWaterChar->GetDens();
				double dKvs = pclShutoffValve->GetValveCharacteristic()->GetKvMax();
				double dDp = CalcDp( GetQ(), dKvs, dRho );
				itMapShutoffValve->second.SetDp( dDp );
			}
		}
	}

	if( _T( '\0' ) == *BestSOIDPtr.ID )
	{
		BestSOIDPtr = m_mapShutoffValve.begin()->second.GetpShutoffValve()->GetIDPtr();
	}

	SetIDPtr( BestSOIDPtr );
	SetBestIDPtr( BestSOIDPtr );

	return BestSOIDPtr;
}

int CDS_HydroMod::CShutoffValve::GetAvailableType( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapShutoffValve.size() )
	{
		return 0;
	}

	pmap->clear();
	std::multimap <int, CShutoffValveData >::iterator It;

	for( It = m_mapShutoffValve.begin(); It != m_mapShutoffValve.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpShutoffValve();
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetTypeIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		(*pmap)[iKey] = pStrID;
	}

	return pmap->size();
}

int CDS_HydroMod::CShutoffValve::GetAvailableConnections( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapShutoffValve.size() )
	{
		return 0;
	}

	pmap->clear();

	CDB_StringID *pStrIDType = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDType = dynamic_cast<CDB_StringID *>( GetpTAP()->GetTypeIDPtr().MP );
	}

	std::multimap <int, CShutoffValveData >::iterator It;

	for( It = m_mapShutoffValve.begin(); It != m_mapShutoffValve.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpShutoffValve();
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetTypeIDPtr().MP );

		if( pStrID != pStrIDType )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

int CDS_HydroMod::CShutoffValve::GetAvailableVersion( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapShutoffValve.size() )
	{
		return 0;
	}

	pmap->clear();

	CDB_StringID *pStrIDType = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDType = dynamic_cast<CDB_StringID *>( GetpTAP()->GetTypeIDPtr().MP );
	}

	CDB_StringID *pStrIDConnection = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDConnection = dynamic_cast<CDB_StringID *>( GetpTAP()->GetConnectIDPtr().MP );
	}

	std::multimap <int, CShutoffValveData >::iterator It;

	for( It = m_mapShutoffValve.begin(); It != m_mapShutoffValve.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpShutoffValve();

		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetTypeIDPtr().MP );

		if( pStrID != pStrIDType )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );

		if( pStrID != pStrIDConnection )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetPNIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

int CDS_HydroMod::CShutoffValve::GetAvailablePN( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapShutoffValve.size() )
	{
		return 0;
	}

	pmap->clear();

	CDB_StringID *pStrIDType = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDType = dynamic_cast<CDB_StringID *>( GetpTAP()->GetTypeIDPtr().MP );
	}

	CDB_StringID *pStrIDConnection = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDConnection = dynamic_cast<CDB_StringID *>( GetpTAP()->GetConnectIDPtr().MP );
	}

	CDB_StringID *pStrIDVersion = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDVersion = dynamic_cast<CDB_StringID *>( GetpTAP()->GetVersionIDPtr().MP );
	}

	std::multimap <int, CShutoffValveData >::iterator It;

	for( It = m_mapShutoffValve.begin(); It != m_mapShutoffValve.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpShutoffValve();

		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetTypeIDPtr().MP );

		if( pStrID != pStrIDType )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );

		if( pStrID != pStrIDConnection )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetVersionIDPtr().MP );

		if( pStrID != pStrIDVersion )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetPNIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

CDB_StringID *CDS_HydroMod::CShutoffValve::GetpSelType()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( ( CDB_ShutoffValve * ) GetIDPtr().MP )->GetTypeIDPtr().MP ) ;
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CShutoffValve::GetpSelConnection()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( ( CDB_ShutoffValve * ) GetIDPtr().MP )->GetConnectIDPtr().MP );
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CShutoffValve::GetpSelVersion()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( ( CDB_ShutoffValve * ) GetIDPtr().MP )->GetVersionIDPtr().MP );
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CShutoffValve::GetpSelPN()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( ( CDB_ShutoffValve * ) GetIDPtr().MP )->GetPNIDPtr().MP );
	}

	return p;
}

void CDS_HydroMod::CShutoffValve::SetpSelTypeAccordingUserChoice( CDB_StringID *pSelpType )
{
	if( NULL != pSelpType && false == pSelpType->IsClass( CLASS( CDB_StringID ) ) )
	{
		ASSERT( false );
		pSelpType = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceType = pSelpType;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceShutoffValveConnect, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CShutoffValve::SetpSelConnAccordingUserChoice( CDB_StringID *pSelpConn )
{
	if( NULL != pSelpConn && false == pSelpConn->IsClass( CLASS( CDB_Connect ) ) )
	{
		ASSERT( false );
		pSelpConn = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceType = GetpSelType();
	m_pUserChoiceConn = pSelpConn;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceShutoffValveVersion, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CShutoffValve::SetpSelVersionAccordingUserChoice( CDB_StringID *pSelpVersion )
{
	if( NULL != pSelpVersion && false == pSelpVersion->IsClass( CLASS( CDB_StringID ) ) )
	{
		ASSERT( false );
		pSelpVersion = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceType = GetpSelType();
	m_pUserChoiceConn = GetpSelConnection();
	m_pUserChoiceVersion = pSelpVersion;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceShutoffValvePN, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CShutoffValve::SetpSelPNAccordingUserChoice( CDB_StringID *pSelpPN )
{
	if( NULL != pSelpPN && false == pSelpPN->IsClass( CLASS( CDB_StringID ) ) )
	{
		ASSERT( false );
		pSelpPN = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceType = GetpSelType();
	m_pUserChoiceConn = GetpSelConnection();
	m_pUserChoiceVersion = GetpSelVersion();
	m_pUserChoicePN = pSelpPN;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceShutOffValves, (LPARAM)this );
	m_pParent->Modified();
}

double CDS_HydroMod::CShutoffValve::GetDp()
{
	if( NULL == GetIDPtr().MP )
	{
		return 0.0;
	}

	if( 0.0 == GetQ() )
	{
		return 0.0;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( GetHMObjectType() );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	CDB_ShutoffValve *pSO = static_cast< CDB_ShutoffValve * >( GetIDPtr().MP );
	double dQ = GetQ();
	double dKvs = pSO->GetKvs();
	double dRho = pclWaterChar->GetDens();
	double dDp = 0.0;

	if( dQ > 0.0 && dKvs > 0.0 )
	{
		dDp = CalcDp( dQ, dKvs, dRho );
	}
	// Update unused Dp variable, we keep variable for file compatibility
	m_dDp = dDp;
	return dDp;
}

void CDS_HydroMod::CShutoffValve::SetDp( double dDp )
{
	if( dDp != GetDp() )
	{
		m_dDp = dDp;
		m_pParent->Modified();
	}
}

bool CDS_HydroMod::CShutoffValve::IsCompletelyDefined()
{
	if( NULL == GetIDPtr().MP )				// safe IDPtr, checked by Extend in GetIDPtr()
	{
		return false;
	}

	if( 0.0 == GetDp() )
	{
		return false;
	}

	return true;
}

void CDS_HydroMod::CShutoffValve::ResizeShutoffValve()
{
	if( false == m_pParent->IsLocked( GetHMObjectType() ) )
	{
		SelectBestShutoffValve();
		return;
	}

	if( NULL == GetIDPtr().MP || 0.0 == GetQ() )
	{
		InvalidateSelection();
		return;
	}

	CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve *>( GetIDPtr().MP );

	if( NULL == pclShutoffValve || NULL == pclShutoffValve->GetValveCharacteristic() )
	{
		InvalidateSelection();
		return;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( GetHMObjectType() );
	
	if( NULL == pclWaterChar )
	{
		ASSERT_RETURN;
	}

	CDB_ValveCharacteristic *pValveCharacteristic = pclShutoffValve->GetValveCharacteristic();
	double dRho = pclWaterChar->GetDens();
	double Kvs = pValveCharacteristic->GetKvMax();
	double dDp = CalcDp( GetQ(), Kvs, dRho );

	SetDp( dDp );
}

bool CDS_HydroMod::CShutoffValve::IsBestShutoffValve( CData *pData )
{
	return ( pData == GetBestIDPtr().MP ) ? true : false;
}

IDPTR CDS_HydroMod::CShutoffValve::GetBestIDPtr()
{
	if( _T('\0') != *m_IDPtrBest.ID )
	{
		m_IDPtrBest.DB = TASApp.GetpTADB();
		Extend( &m_IDPtrBest );
		return m_IDPtrBest;
	}

	return _NULL_IDPTR;
}

IDPTR CDS_HydroMod::CShutoffValve::GetIDPtr()
{
	if( _T('\0') != *m_IDPtr.ID )
	{
		m_IDPtr.DB = TASApp.GetpTADB();
		Extend( &m_IDPtr );
		return m_IDPtr;
	}

	return _NULL_IDPTR;
}

CDB_ValveCharacteristic *CDS_HydroMod::CShutoffValve::GetpValveChar()
{
	CDB_ValveCharacteristic *pDevChar = NULL;

	if( NULL != GetIDPtr().MP )
	{
		pDevChar = ( ( CDB_TAProduct * ) GetIDPtr().MP )->GetValveCharacteristic();
	}

	return pDevChar;
}

bool CDS_HydroMod::CShutoffValve::FillShutoffValveInfoIntoHMX( CDS_HydroModX *pHMX, eHMObj HMObj )
{
	if( eHMObj::eShutoffValveSupply != HMObj && eHMObj::eShutoffValveReturn != HMObj )
	{
		return false;
	}

	// Test if a valve exist (TAProduct) for the localization.
	if( NULL == GetIDPtr().MP )
	{
		return false;
	}

	ShutoffValveLoc eShutoffValveLocation = ( eShutoffValveSupply == HMObj ) ? ShutoffValveLocSupply : ShutoffValveLocReturn;
	CDS_HydroModX::CShutoffValve *pSV = pHMX->GetpShutoffValve( eShutoffValveLocation );

	// Design flow.
	pSV->SetValveID( GetIDPtr().ID );
	pSV->SetDesignFlow( GetQ() );

	return true;
}

bool CDS_HydroMod::CShutoffValve::ForceShutoffValveSelection( IDPTR DeviceIDptr )
{
	if( true == m_mapShutoffValve.empty() )
	{
		// Unlock the valve to force filling of 'm_mapShutoffValve'.
		m_pParent->SetLock( GetHMObjectType(), false );
	}

	// Lock valve if new selected valve doesn't correspond to the Best valve idptr
	m_pParent->SetLock( GetHMObjectType(), !IsBestShutoffValve( (CData *)DeviceIDptr.MP ) );

	// Forced valve should be inside 'm_mapShutoffValve' as a valid choice;
	_SetpSelShutoffValve( (LPARAM)DeviceIDptr.MP, true );
	ClearUserChoiceTemporaryVariables();

	return ( GetIDPtr().MP == DeviceIDptr.MP );
}

int CDS_HydroMod::CShutoffValve::CheckValidity()
{
	m_usValidityFlags = CDS_HydroMod::eValidityFlags::evfOK;

	if( NULL == GetIDPtr().MP )
	{
		return m_usValidityFlags;
	}

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( GetIDPtr().MP );

	if( NULL == pTAP )
	{
		return m_usValidityFlags;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( GetHMObjectType() );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( m_usValidityFlags );
	}

	if( pTAP->GetTmax() < pclWaterChar->GetTemp() )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfTempTooHigh;
	}

	if( GetDp() > m_pParent->GetpTechParam()->VerifyValvMaxDp( ( CDB_TAProduct * ) GetIDPtr().MP ) )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDpTooHigh;
	}

/*
	if( GetDp() < m_pParent->GetpTechParam()->GetValvMinDp() )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDpTooLow;
	}
*/
	return m_usValidityFlags;
}

void CDS_HydroMod::CShutoffValve::DropData( OUTSTREAM outf, int *piLevel )
{
	CString strTab = _T("");

	for( int i = *piLevel; i; i-- )
	{
		strTab += _T("\t");
	}

	CString str;
	str.Format( _T("m_IDPtr.ID = %s\n"), GetIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_IDPtrBest.ID = %s\n"), GetBestIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Q = %s\n"), WriteCUDouble( _U_FLOW, GetQ(), true ) );
	WriteFormatedStringA2( outf, str, strTab );

	str = _T("m_Located = ");

	switch( GetHMObjectType() )
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

	str.Format( _T("m_usValidityFlags = %u\n"), GetValidtyFlags() );
	WriteFormatedStringA2( outf, str, strTab );
	WriteFormatedStringA2( outf, _T("\n"), strTab );
}

void CDS_HydroMod::CShutoffValve::ClearUserChoiceTemporaryVariables()
{
	m_pUserChoiceType = NULL;
	m_pUserChoiceConn = NULL;
	m_pUserChoiceVersion = NULL;
	m_pUserChoicePN = NULL;
}

void CDS_HydroMod::CShutoffValve::_AbortSelectBestShutoffValve()
{
	if( false == m_pParent->IsLocked( m_eLocated ) || 0 == *GetIDPtr().ID )
	{
		InvalidateSelection();
	}

	ClearUserChoiceTemporaryVariables();
}

void CDS_HydroMod::CShutoffValve::_SetpSelShutoffValve( LPARAM pTADBShutoffValve, bool bForceLocker )
{
	if( false == bForceLocker )
	{
		if( true == m_pParent->IsLocked( GetHMObjectType() ) && 0 != *GetIDPtr().ID )
		{
			m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceShutOffValves, (LPARAM)this );
			return;
		}
	}

	// Try to find product inside main valve map.
	std::multimap <int, CShutoffValveData>::iterator itMapValves;

	for( itMapValves = m_mapShutoffValve.begin(); m_mapShutoffValve.end() != itMapValves; ++itMapValves )
	{
		if( pTADBShutoffValve == (LPARAM)itMapValves->second.GetpShutoffValve() )
		{
			break;
		}
	}

	if( m_mapShutoffValve.end() != itMapValves )
	{
		SetIDPtr( itMapValves->second.GetpShutoffValve()->GetIDPtr() );
		SetDp( itMapValves->second.GetDp() );
	}
	else
	{
		InvalidateSelection();
	}

	m_pParent->Modified();
}

CDS_HydroMod::CBV::CBV( CDS_HydroMod *pHM, eHMObj eLocated ) : CBase( pHM )
{
	m_eLocated = eLocated;
	m_dQ = 0.0;
	m_dDp = 0.0;
	m_dDpSignal = 0.0;
	m_dSetting = 0.0;
	// No Selection from TADB.
	InvalidateSelection();

	ClearUserChoiceTemporaryVariables();
}

CDS_HydroMod::CBV::~CBV()
{
}

void CDS_HydroMod::CBV::Copy( CDS_HydroMod::CBV *pclTargetHMBv, bool bResetDiversityFactor )
{
	pclTargetHMBv->SetIDPtr( GetIDPtr() );
	pclTargetHMBv->SetDiversityIDPtr( ( true == bResetDiversityFactor ) ? _NULL_IDPTR : GetDiversityIDPtr() );
	pclTargetHMBv->SetLocate( GetHMObjectType() );
	pclTargetHMBv->SetQ(GetQ());
	pclTargetHMBv->SetSetting(GetSetting());
	pclTargetHMBv->SetDp( GetDp() );
	pclTargetHMBv->SetBestBvIDPtr( GetBestBvIDPtr() );
	pclTargetHMBv->SetValidtyFlags( GetValidtyFlags() );

	if (NULL == m_pUserChoiceBVType && NULL == m_pUserChoiceBVConn && NULL == m_pUserChoiceBVVers && NULL == m_pUserChoiceBVPN)
	{
		pclTargetHMBv->ClearUserChoiceTemporaryVariables();
	}
	else
	{
		pclTargetHMBv->SetpSelBVTypeAccordingUserChoice(m_pUserChoiceBVType);
		pclTargetHMBv->SetpSelBVConnAccordingUserChoice(m_pUserChoiceBVConn);
		pclTargetHMBv->SetpSelBVVersAccordingUserChoice(m_pUserChoiceBVVers);
		pclTargetHMBv->SetpSelBVPNAccordingUserChoice(m_pUserChoiceBVPN);
	}
	// Pay attention: 'DpSignal' is computed thanks to IDPtr and Q. This call must be placed after having set these
	// concerned variables.
	pclTargetHMBv->SetDpSignal( m_dDpSignal );
}

#define CDS_HYDROMOD_CBV_VERSION	6
// Version 6 save/write user parameters for type, version, connection, PN
void CDS_HydroMod::CBV::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_CBV_VERSION;
	WriteData<>( outf, Version );

	// Info.
	WriteString( outf, m_DiversityIDPtr.ID );
	WriteString( outf, m_IDPtrBestBV.ID );
	WriteData<>( outf, m_eLocated );
	WriteData<>( outf, m_dQ );
	WriteData<>( outf, m_dDp );
	WriteData<>( outf, m_dDpSignal );
	WriteData<>( outf, m_dSetting );
	WriteString( outf, m_IDPtr.ID );
	CString strID;
	strID = ( NULL != m_pUserChoiceBVType ) ? m_pUserChoiceBVType->GetIDPtr().ID : L"";
	WriteString( outf, strID );
	strID = ( NULL != m_pUserChoiceBVConn ) ? m_pUserChoiceBVConn->GetIDPtr().ID : L"";
	WriteString( outf, strID );
	strID = ( NULL != m_pUserChoiceBVVers ) ? m_pUserChoiceBVVers->GetIDPtr().ID : L"";
	WriteString( outf, strID );
	strID = ( NULL != m_pUserChoiceBVPN ) ? m_pUserChoiceBVPN->GetIDPtr().ID : L"";
	WriteString( outf, strID );
}

bool CDS_HydroMod::CBV::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_CBV_VERSION;
	ReadData<>( inpf, Version );

	if( Version < 1 || Version > CDS_HYDROMOD_CBV_VERSION )
	{
		return false;
	}

	// Info.
	ReadString( inpf, m_DiversityIDPtr.ID, sizeof( m_DiversityIDPtr.ID ) );

	// Before version 5, we read 'm_SelIDPtr.ID' that is no more used. Now, we write 'm_DiversityIDPtr' at this place.
	// We must reset ID to 0 if version is below 5 because diversity was not exist.
	if( Version < 5 )
	{
		m_DiversityIDPtr.ID[0] = 0;
	}
	else
	{
		m_DiversityIDPtr.DB = TASApp.GetpTADB();
	}

	ReadString( inpf, m_IDPtrBestBV.ID, sizeof( m_IDPtrBestBV.ID ) );
	ReadData<>( inpf, m_eLocated );

	if( Version < 3 )
	{
		double dpMin;	// for trash
		ReadData<>( inpf, dpMin );
	}

	if( 1 == Version )
	{
		return true;
	}

	ReadData<>( inpf, m_dQ );

	if( 3 == Version )
	{
		return true;
	}

	ReadData<>( inpf, m_dDp );
	ReadData<>( inpf, m_dDpSignal );
	ReadData<>( inpf, m_dSetting );

	if( 4 == Version )
	{
		return true;
	}

	ReadString( inpf, m_IDPtr.ID, sizeof( m_IDPtr.ID ) );
	m_IDPtr.DB = TASApp.GetpTADB();

	m_pUserChoiceBVType = m_pUserChoiceBVConn = m_pUserChoiceBVVers = m_pUserChoiceBVPN = NULL;

	if( Version < 6 )
	{
		return true;
	}

	IDPTR idptr;
	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceBVType = dynamic_cast<CDB_StringID *>( idptr.MP );
	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceBVConn = dynamic_cast<CDB_StringID *>( idptr.MP );
	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceBVVers = dynamic_cast<CDB_StringID *>( idptr.MP );
	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceBVPN = dynamic_cast<CDB_StringID *>( idptr.MP );

	return true;
}

bool CDS_HydroMod::CBV::FillBvInfoIntoHMX( CDS_HydroModX *pHMX )
{
	CDS_HydroMod::CBV *pHMBV = NULL;

	// Test if a valve exist (TAProduct) for the localization.
	if( NULL == GetIDPtr().MP /*|| false == GetIDPtr().MP->IsExistInTASCOPE()*/ )
	{
		return false;
	}

	// By default TAMode and TaValve.
	CDS_HydroModX::CBV *pHMXBV = NULL;

	switch( GetHMObjectType() )
	{
		case eBVprim:
			pHMXBV = pHMX->AddBv( CDS_HydroModX::eLocate::InPrimary, CDS_HydroModX::eMode::TAmode );
			break;

		case eBVbyp:
			pHMXBV = pHMX->AddBv( CDS_HydroModX::eLocate::InBypass, CDS_HydroModX::eMode::TAmode );
			break;

		case eBVsec:
			pHMXBV = pHMX->AddBv( CDS_HydroModX::eLocate::InSecondary, CDS_HydroModX::eMode::TAmode );
			break;

		default:
			ASSERT( 0 );
	}

	// Design flow.
	pHMXBV->SetDesignFlow( m_pParent->GetDistrTotQ() );

	// Valve has been defined as a Kv.
	if( enum_VDescriptionType::edt_KvCv == m_pParent->GetVDescrType() )
	{
		// Add a balancing valve in the primary side in KvMode.
		pHMXBV->SetMode( CDS_HydroModX::eMode::Kvmode );
		pHMXBV->SetKv( m_pParent->GetKvCv() );
		return true;
	}

	pHMXBV->SetValveID( GetIDPtr().ID );
	pHMXBV->SetDesignOpening( GetSetting() );
	pHMXBV->SetCurOpening( GetSetting() );
	pHMXBV->SetDesignFlow( GetQ() );

	return true;
}

IDPTR CDS_HydroMod::CBV::GetBestBvIDPtr()
{
	if( _T('\0') != *m_IDPtrBestBV.ID )
	{
		m_IDPtrBestBV.DB = TASApp.GetpTADB();
		Extend( &m_IDPtrBestBV );
		return m_IDPtrBestBV;
	}

	return _NULL_IDPTR;
}

IDPTR CDS_HydroMod::CBV::GetIDPtr()
{
	if( _T('\0') != *m_IDPtr.ID && NULL != m_IDPtr.DB )
	{
		Extend( &m_IDPtr );
		return m_IDPtr;
	}

	return _NULL_IDPTR;
}

IDPTR CDS_HydroMod::CBV::GetDiversityIDPtr()
{
	if( _T('\0') != *m_DiversityIDPtr.ID && NULL != m_DiversityIDPtr.DB )
	{
		Extend( &m_DiversityIDPtr );
		return m_DiversityIDPtr;
	}

	return _NULL_IDPTR;
}

eBool3 CDS_HydroMod::CBV::CheckDpMinMax( bool fSignal )
{
	if( NULL == GetIDPtr().MP )
	{
		return eb3Undef;
	}

	if( true == GetIDPtr().MP->IsClass( CLASS( CDB_VenturiValve ) ) )
	{
		// Only DpSignal must be checked.
		if( true == fSignal )
		{
			if( GetDpSignal() < m_pParent->GetpTechParam()->GetVtriMinDp() ||
				GetDpSignal() > m_pParent->GetpTechParam()->GetVtriMaxDp() )
			{
				return eb3False;
			}
		}
	}
	else if( true == GetIDPtr().MP->IsClass( CLASS( CDB_CommissioningSet ) ) )
	{
		if( GetDp() < m_pParent->GetpTechParam()->GetValvMinDp() ||
			GetDp() > m_pParent->GetpTechParam()->VerifyValvMaxDp( ( CDB_TAProduct * ) GetIDPtr().MP ) )
		{
			return eb3False;
		}
	}
	else if( true == GetIDPtr().MP->IsClass( CLASS( CDB_FixedOrifice ) ) )
	{
		if( true == fSignal )
		{
			if( GetDpSignal() < m_pParent->GetpTechParam()->GetFoMinDp() ||
				GetDpSignal() > m_pParent->GetpTechParam()->GetFoMaxDp() )
			{
				return eb3False;
			}
		}
	}
	else
	{
		if( GetDp() < m_pParent->GetpTechParam()->GetValvMinDp() ||
			GetDp() > m_pParent->GetpTechParam()->VerifyValvMaxDp( ( CDB_TAProduct * ) GetIDPtr().MP ) )
		{
			return eb3False;
		}
	}

	return eb3True;
}

double CDS_HydroMod::CBV::GetTechParamMinDp()
{
	if( NULL != GetIDPtr().MP )
	{
		if( true == GetIDPtr().MP->IsClass( CLASS( CDB_VenturiValve ) ) )
		{
			return 0.0;
		}
		else if( true == GetIDPtr().MP->IsClass( CLASS( CDB_CommissioningSet ) ) )
		{
			return m_pParent->GetpTechParam()->GetValvMinDp();
		}
		else if( true == GetIDPtr().MP->IsClass( CLASS( CDB_FixedOrifice ) ) )
		{
			return 0.0;
		}
		else
		{
			return m_pParent->GetpTechParam()->GetValvMinDp();
		}
	}

	return m_pParent->GetpTechParam()->GetValvMinDp();
}

eBool3 CDS_HydroMod::CBV::CheckMinOpen()
{
	if( NULL == GetIDPtr().MP || NULL == GetpValveChar() )
	{
		return eb3Undef;
	}

	double dValveMinOpening = GetpValveChar()->GetMinRecSetting();

	if( GetSetting() < dValveMinOpening )
	{
		return eb3False;
	}

	return eb3True;
}

CDB_ValveCharacteristic *CDS_HydroMod::CBV::GetpValveChar()
{
	CDB_ValveCharacteristic *pDevChar = NULL;

	if( NULL != GetIDPtr().MP )
	{
		pDevChar = ( ( CDB_TAProduct * ) GetIDPtr().MP )->GetValveCharacteristic();
	}

	return pDevChar;
}

double  CDS_HydroMod::CBV::GetOpeningMax()
{
	double dH = 0.0;
	CDB_ValveCharacteristic *pDevChar = GetpValveChar();

	if( NULL != pDevChar )
	{
		dH = pDevChar->GetOpeningMax();

		if( -1.0 == dH )
		{
			dH = 0.0;
		}
	}

	return dH;
}

CDB_StringID *CDS_HydroMod::CBV::GetpSelBVType()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( static_cast< CDB_TAProduct *>( GetIDPtr().MP ) )->GetTypeIDPtr().MP );
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CBV::GetpSelBVConn()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( static_cast< CDB_TAProduct *>( GetIDPtr().MP ) )->GetConnectIDPtr().MP );
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CBV::GetpSelBVVers()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( static_cast< CDB_TAProduct *>( GetIDPtr().MP ) )->GetVersionIDPtr().MP );
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CBV::GetpSelBVPN()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( static_cast< CDB_TAProduct *>( GetIDPtr().MP ) )->GetPNIDPtr().MP );
	}

	return p;
}

void CDS_HydroMod::CBV::SetIDPtr( IDPTR IDPtr )
{
	m_IDPtr = IDPtr;

	if( _T('\0') != *m_IDPtr.ID && NULL != m_IDPtr.DB )
	{
		Extend( &m_IDPtr );

		if( true == m_pParent->IsForHub() )
		{
			if( true == m_pParent->IsClass( CLASS( CDS_HmHub ) ) )
			{
				// In case of DpC Balancing valve is located on supply side.
				if( true == m_pParent->IsDpCExist() )
				{
					( static_cast<CDS_HmHub *>( m_pParent ) )->SetSupplyValveID( ( static_cast<CDS_HmHub *>( m_pParent ) )->FindHubPtnValve( CDB_HubValv::eSupplyReturnValv::Supply,
							m_IDPtr ).ID );
				}
				else
				{
					( static_cast<CDS_HmHub *>( m_pParent ) )->SetReturnValveID( ( static_cast<CDS_HmHub *>( m_pParent ) )->FindHubPtnValve( CDB_HubValv::eSupplyReturnValv::Return,
							m_IDPtr ).ID );
				}
			}
			else if( true == m_pParent->IsClass( CLASS( CDS_HmHubStation ) ) )
			{
				( static_cast<CDS_HmHubStation *>( m_pParent ) )->SetReturnValveID( ( static_cast<CDS_HmHubStation *>( m_pParent ) )->FindHubStaValve(
							CDB_HubStaValv::eSupplyReturnValv::Return,
							m_IDPtr ).ID );
			}
		}
	}

	m_pParent->Modified();
}

void CDS_HydroMod::CBV::SetpSelBVTypeAccordingUserChoice( CDB_StringID *pSelBVType )
{
	if( NULL != pSelBVType && false == pSelBVType->IsClass( CLASS( CDB_StringID ) ) )
	{
		ASSERT( false );
		pSelBVType = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceBVType = pSelBVType;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceBVConnect, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CBV::SetpSelBVConnAccordingUserChoice( CDB_StringID *pSelBVConn )
{
	if( NULL != pSelBVConn && false == pSelBVConn->IsClass( CLASS( CDB_Connect ) ) )
	{
		ASSERT( false );
		pSelBVConn = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceBVType = GetpSelBVType();
	m_pUserChoiceBVConn = pSelBVConn;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceBVVers, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CBV::SetpSelBVVersAccordingUserChoice( CDB_StringID *pSelBVVers )
{
	if( NULL != pSelBVVers && false == pSelBVVers->IsClass( CLASS( CDB_StringID ) ) )
	{
		ASSERT( false );
		pSelBVVers = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceBVType = GetpSelBVType();
	m_pUserChoiceBVConn = GetpSelBVConn();
	m_pUserChoiceBVVers = pSelBVVers;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceBVPN, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CBV::SetpSelBVPNAccordingUserChoice( CDB_StringID *pSelBVPN )
{
	if( NULL != pSelBVPN && false == pSelBVPN->IsClass( CLASS( CDB_StringID ) ) )
	{
		ASSERT( false );
		pSelBVPN = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceBVType = GetpSelBVType();
	m_pUserChoiceBVConn = GetpSelBVConn();
	m_pUserChoiceBVVers = GetpSelBVVers();
	m_pUserChoiceBVPN = pSelBVPN;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceBV, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CBV::_SetpSelBV( LPARAM pTaDbBV, bool fForceLocker )
{
	if( false == fForceLocker )
	{
		if( true == m_pParent->IsLocked( GetHMObjectType() ) && 0 != *GetIDPtr().ID )
		{
			m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceBV, (LPARAM)this );
			return;
		}
	}

	// Try to find product inside main valve map
	std::multimap <int, CValvesData>::iterator itMapValves;

	for( itMapValves = m_mapValves.begin(); m_mapValves.end() != itMapValves; ++itMapValves )
	{
		if( pTaDbBV == (LPARAM)itMapValves->second.GetpTAP() )
		{
			break;
		}
	}

	if( m_mapValves.end() != itMapValves )
	{
		SetIDPtr( itMapValves->second.GetpTAP()->GetIDPtr() );
		SetDp( itMapValves->second.GetDp() );
		SetDpSignal( itMapValves->second.GetSignal() );
		SetSetting( itMapValves->second.GetSetting() );

		if( CDS_HydroMod::eHMObj::eBVprim == GetHMObjectType() )
		{
			m_pParent->SetPresetting( GetSetting() );
			m_pParent->SetCBIValveID( GetIDPtr().ID );
		}
	}
	else
	{
		InvalidateSelection();
	}

	//m_pParent->ComputeHM ( CDS_HydroMod::eComputeHMEvent::eceBV, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CBV::SetQ( double dQ )
{
	m_dQ = dQ;
}

double CDS_HydroMod::CBV::GetQ()
{
	if( m_dQ < 0.0 )
	{
		m_dQ = 0.0;
	}

	return m_dQ;
}

void CDS_HydroMod::CBV::SetSetting( double dSetting )
{
	if( dSetting != GetSetting() )
	{
		m_pParent->Modified();
	}

	m_dSetting = dSetting;
	m_pParent->SetPresetting( m_dSetting );
}

double CDS_HydroMod::CBV::GetSetting()
{
	if( m_dSetting < 0.0 )
	{
		m_dSetting = 0.0;
	}

	return m_dSetting;
}

void CDS_HydroMod::CBV::SetDp( double dDp )
{
	if( dDp != GetDp() )
	{
		m_dDp = dDp;
		m_pParent->Modified();
	}
}

double CDS_HydroMod::CBV::GetDp()
{
	if( m_dDp < 0.0 )
	{
		m_dDp = 0.0;
	}

	return m_dDp;
}

void CDS_HydroMod::CBV::SetDpSignal( double dDpSignal )
{
	if( dDpSignal != GetDpSignal() )
	{
		m_dDpSignal = dDpSignal;
		m_pParent->Modified();
	}
}

double CDS_HydroMod::CBV::GetDpSignal()
{
	double dDpSignal = 0.0;

	if( _T('\0') != *GetIDPtr().ID )
	{
		CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( GetHMObjectType() );
	
		if( NULL == pclWaterChar )
		{
			ASSERTA_RETURN( 0.0 );
		}

		double dKvSignal = ( ( CDB_TAProduct * ) GetIDPtr().MP )->GetKvSignal();
		double dRho = pclWaterChar->GetDens();
		double dQ = GetQ();

		if( dQ > 0.0 && dKvSignal > 0.0 )
		{
			dDpSignal = CalcDp( dQ, dKvSignal, dRho );
		}
	}

	return dDpSignal;
}

// Force selected valve, IDptr is BV Idptr in TADB.
bool CDS_HydroMod::CBV::ForceBVSelection( IDPTR DeviceIDptr )
{
	if( true == m_mapValves.empty() )
	{
		// Unlock the valve to force filling of 'm_mapValves'.
		m_pParent->SetLock( GetHMObjectType(), false );
	}

	// Lock valve if new selected valve doesn't correspond to the best valve idptr.
	m_pParent->SetLock( GetHMObjectType(), !IsBestBv( (CData *)DeviceIDptr.MP ) );

	// Forced valve should be inside 'm_mapValves' as a valid choice.
	_SetpSelBV( (LPARAM)DeviceIDptr.MP, true );
	ClearUserChoiceTemporaryVariables();

	return ( GetIDPtr().MP == DeviceIDptr.MP );
}

void CDS_HydroMod::CBV::InvalidateSelection()
{
	if( true == GetpParentHM()->IsLocked( GetHMObjectType() ) )
	{
		return;
	}
	
	SetIDPtr( _NULL_IDPTR );
	SetBestBvIDPtr( _NULL_IDPTR );

	// Check now if we can clean also the "m_CBIValveIDPtr" variable. It depends if a previous one exists and is the same kind of the current
	// one. If it's the case we can clean it, otherwise it means the CBI valve is perhaps a regulating valve and we can't delete it here.
	if( _NULL_IDPTR != GetpParentHM()->GetCBIValveIDPtr() )
	{
		// If it's well a regulating valve but not a control valve, we can clean.
		if( NULL != dynamic_cast<CDB_RegulatingValve *>( GetpParentHM()->GetCBIValveIDPtr().MP ) &&
			NULL == dynamic_cast<CDB_ControlValve *>( GetpParentHM()->GetCBIValveIDPtr().MP ) )
		{
			GetpParentHM()->SetCBIValveID( _T("") );
		}
	}
}

void CDS_HydroMod::CBV::ResizeBv( double dHAvail )
{
	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( GetHMObjectType() );
	
	if( NULL == pclWaterChar )
	{
		ASSERT_RETURN;
	}

	double dQ = GetQ();
	double dDp = dHAvail;

	if( dDp < GetTechParamMinDp() )
	{
		dDp = GetTechParamMinDp();
	}

	if( false == m_pParent->IsLocked( GetHMObjectType() ) )
	{
		_SelectBestBV( dDp );
		return;
	}
	else
	{
		double dRho = pclWaterChar->GetDens();
		double dNu = pclWaterChar->GetKinVisc();
		double dH = 0.0;
		bool bValid = false;

		if( NULL != GetIDPtr().MP && true == GetIDPtr().MP->IsClass( CLASS( CDB_VenturiValve ) ) )
		{
			CDB_VenturiValve *pclVenturiValve = static_cast< CDB_VenturiValve * >( GetIDPtr().MP );

			if( NULL != pclVenturiValve->GetVvCurve() )
			{
				bValid = pclVenturiValve->GetVvCurve()->GetValveOpening( dQ, dDp, &dH, dRho, dNu, 1 );

				if( false == bValid )
				{
					// Dp at full opening if greater than requested Dp.
					// Ask Dp at full opening.
					dH = pclVenturiValve->GetVvCurve()->GetOpeningMax();

					if( -1.0 != dH )
					{
						bValid = pclVenturiValve->GetVvCurve()->GetValveDp( dQ, &dDp, dH, dRho, dNu );
					}
				}

				if( true == bValid )
				{
					SetSetting( dH );
					SetDp( dDp );
					// Ask Dp at full opening.
					dH = pclVenturiValve->GetVvCurve()->GetOpeningMax();

					if( -1.0 != dH )
					{
						bValid = pclVenturiValve->GetVvCurve()->GetValveDp( dQ, &dDp, dH, dRho, dNu );

						if( true == bValid )
						{
							SetDpSignal( dDp );
						}
					}
				}
			}
		}
		else if( NULL != GetIDPtr().MP && true == GetIDPtr().MP->IsClass( CLASS( CDB_CommissioningSet ) ) )
		{
			// Compute the Fixed orifice pressure drop.
			CDB_CommissioningSet *pCommissioningSet = static_cast< CDB_CommissioningSet * >( GetIDPtr().MP );
			double dKv = pCommissioningSet->GetKvFixO();
			double dDpFixO = 0.0;

			if( -1.0 != dKv )
			{
				dDpFixO = CalcDp( dQ, dKv, dRho );
			}

			dH -= dDpFixO;

			if( NULL != pCommissioningSet->GetDrvCurve() && dH > 0.0 )
			{
				bValid = pCommissioningSet->GetDrvCurve()->GetValveOpening( dQ, dDp, &dH, dRho, dNu, 1 );

				if( false == bValid )
				{
					// Dp at full opening if greater than requested Dp.
					// Ask Dp at full opening.
					dH = pCommissioningSet->GetDrvCurve()->GetOpeningMax();

					if( -1.0 != dH )
					{
						bValid = pCommissioningSet->GetDrvCurve()->GetValveDp( dQ, &dDp, dH, dRho, dNu );

						if( true == bValid )
						{
							dDp += dDpFixO;
						}
					}
				}
			}

			if( true == bValid )
			{
				SetSetting( dH );
				SetDp( dDp );
				// Dp at full opening.
				dH = pCommissioningSet->GetDrvCurve()->GetOpeningMax();

				if( -1.0 != dH )
				{
					bValid = pCommissioningSet->GetDrvCurve()->GetValveDp( dQ, &dDp, dH, dRho, dNu );

					if( true == bValid )
					{
						dDp += dDpFixO;
						SetDpSignal( dDp );
					}
				}
			}
		}
		else if( NULL != GetIDPtr().MP && true == GetIDPtr().MP->IsClass( CLASS( CDB_FixedOrifice ) ) )
		{
			;
		}
		else if( NULL != GetIDPtr().MP )
		{
			CDB_RegulatingValve *pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( GetIDPtr().MP );

			if( NULL != pclRegulatingValve && NULL != pclRegulatingValve->GetValveCharacteristic() )
			{
				CDB_ValveCharacteristic *pValveCharacteristic = pclRegulatingValve->GetValveCharacteristic();

				if( dQ > 0.0 )
				{
					int iRounding = ( eBool3::eb3True == pValveCharacteristic->IsDiscrete() ) ? 2 : 1;
					bValid = pValveCharacteristic->GetValveOpening( dQ, dDp, &dH, dRho, dNu, iRounding );
				}

				if( false == bValid )
				{
					// Dp at full opening if greater than requested Dp.
					// Ask Dp at full opening.
					dH = pValveCharacteristic->GetOpeningMax();

					if( -1.0 != dH )
					{
						bValid = pValveCharacteristic->GetValveDp( dQ, &dDp, dH, dRho, dNu );
					}
				}
				else
				{
					// Recalculate the Dp for the setting as determined above.
					if( eBool3::eb3True == pValveCharacteristic->IsDiscrete() )
					{
						bValid = pValveCharacteristic->GetValveDp( dQ, &dDp, dH, dRho, dNu );
					}
				}

				if( true == bValid )
				{
					SetSetting( dH );
					SetDp( dDp );

					// Dp at full opening.
					dH = pValveCharacteristic->GetOpeningMax();

					if( -1.0 != dH )
					{
						bValid = pValveCharacteristic->GetValveDp( dQ, &dDp, dH, dRho, dNu );
					}

					if( true == bValid )
					{
						SetDpSignal( dDp );
					}
				}
			}
		}

		if( false == bValid )
		{
			InvalidateSelection();
		}
	}
}

// Scan currently selected MV list return Dp 100% of the first MV just above 3kPa , return MV IDPTR.
IDPTR CDS_HydroMod::CBV::GetDp2MVabv3kPa( double &dMaxDp )
{
	if( 0 == m_mapValves.size() )
	{
		return _NULL_IDPTR;
	}

	std::multimap <int, CValvesData >::iterator ItMainValveMap;
	std::multimap <int, CValvesData >::iterator ItSelectedMV = m_mapValves.end();
	double dDp = 0.0;
	double dDelta = DBL_MAX;

	for( ItMainValveMap = m_mapValves.begin(); m_mapValves.end() != ItMainValveMap; ++ItMainValveMap )
	{
		CDB_TAProduct *pTAP = ItMainValveMap->second.GetpTAP();

		if( NULL == pTAP )
		{
			continue;
		}

		if( ItMainValveMap->second.GetDpFO() <= dMaxDp )
		{
			double dVal = ItMainValveMap->second.GetDpFO() - GetTechParamMinDp();

			if( dVal > 0.0 && dVal < dDelta )
			{
				ItSelectedMV = ItMainValveMap;
				dDp =  ItMainValveMap->second.GetDpFO();
				dDelta = dVal;
			}
		}
	}

	if( dDp != 0.0 )
	{
		dMaxDp = dDp;
		return ItSelectedMV->second.GetpTAP()->GetIDPtr();
	}

	dMaxDp = 0.0;
	return _NULL_IDPTR;
}

void CDS_HydroMod::CBV::_SelectBestBV( double dDp )
{
	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( GetHMObjectType() );
	
	if( NULL == pclWaterChar )
	{
		ASSERT_RETURN;
	}

	int iHighestSize = 0;
	int iLowestSize = 0;
	double dFlow = GetQ();
	double dRho = pclWaterChar->GetDens();
	double dNu = pclWaterChar->GetKinVisc();

	if( dFlow <= 0 )
	{
		// HYS-1753: Clean the selection when de flow is not set
		InvalidateSelection();
		return;
	}

	CAnchorPt::CircuitSide eCircuitSide;
	CAnchorPt::PipeLocation ePipeLocation;

	if( eBVprim == GetHMObjectType() )
	{
		eCircuitSide = CAnchorPt::CircuitSide_Primary;
		ePipeLocation = m_pParent->GetpSch()->GetAnchorPtPipeLocation( CAnchorPt::eFunc::BV_P );
	}
	else if( eBVbyp == GetHMObjectType() )
	{
		eCircuitSide = m_pParent->GetBypassPipeSide();
		ePipeLocation = m_pParent->GetpSch()->GetAnchorPtPipeLocation( CAnchorPt::eFunc::BV_Byp );
	}
	else if( eBVsec == GetHMObjectType() )
	{
		eCircuitSide = CAnchorPt::CircuitSide_Secondary;
		ePipeLocation = m_pParent->GetpSch()->GetAnchorPtPipeLocation( CAnchorPt::eFunc::BV_S );
	}
	else
	{
		ASSERT_RETURN;
	}

	CDB_Pipe *pPipe = m_pParent->GetPipeSizeShift( iHighestSize, iLowestSize, eCircuitSide, ePipeLocation );

	if( NULL == pPipe )
	{
		return;
	}

	int iDN50 = m_pParent->GetDN50();
	CPrjParams *pPrjParams = m_pParent->GetpPrjParam()->GetpHmCalcParams();

	// By default use the same Product Type as selected product
	CDB_StringID *pBVType = GetpSelBVType();

	if( NULL != m_pUserChoiceBVType )
	{
		// User select another type of valve
		pBVType = m_pUserChoiceBVType;
	}
	else if( NULL == pBVType )
	{
		// Nothing selected yet
		// Return to pre-selection done in Tech params.
		if( pPipe->GetSizeKey( TASApp.GetpTADB() ) <= iDN50 )
		{
			pBVType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvBlw65TypeID ).MP );
		}
		else
		{
			pBVType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvAbv50TypeID ).MP );
		}
	}

	bool bUsedAsMV = false;
	bool bRemoveSTAM = true;

	// HYS-1636: we know consider m_iDpCMvWithSameSizeOnly parameter in HM Calc.
	int iDNMin = 0;
	int iDNMax = INT_MAX;
	int iDNDpC = INT_MAX;
	CDS_TechnicalParameter* pclTechParams = GetpParentHM()->GetpTechParam();

	if( true == GetpParentHM()->IsDpCExist() )
	{
		// No need for a measuring valve if located in secondary side or in a bypass.
		// In those case the BV is never associated with a DpC.
		if( eBVprim == GetHMObjectType() )
		{
			bUsedAsMV = true;

			// HYS-1636: If the user want a regulating valve selection using same Dp controller size we keep the DpC size.
			bool bDpCBvSameSize = ( pclTechParams->GetDpCMvWithSameSizeOnly() != 0 ) ? true : false;

			if( true == bDpCBvSameSize && NULL != GetpParentHM()->GetpDpC()->GetIDPtr().MP )
			{
				iDNDpC = GetpParentHM()->GetpDpC()->GetpTAP()->GetSizeKey();
			}
		}

		// In case of Measuring valve in secondary STAM cannot be selected.
		bRemoveSTAM = ( eMvLoc::MvLocSecondary == GetpParentHM()->GetpDpC()->GetMvLoc() ) ? true : false;
	}

	// Establish valve list based on Valve Type and the best technical choice
	CRankEx List;
	GetpTADB()->GetBVList( &List, pBVType->GetIDPtr().ID, L"", L"", L"", L"", CTADatabase::FilterSelection::ForHMCalc, iDNMin, iDNMax, NULL, bUsedAsMV );

	LPARAM lparam;
	_string str;
	m_mapValves.clear();
	std::multimap <int, CValvesData>::iterator itMapValves;
	std::multimap <int, CValvesData>::reverse_iterator rITmapValves;

	// Extract valves according SizeShift, keep at least one product size
	for( BOOL bCont = List.GetFirst( str, lparam ); TRUE == bCont; bCont = List.GetNext( str, lparam ) )
	{
		CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *) lparam );

		if( NULL == pTAP )
		{
			continue;
		}

		if( true == bUsedAsMV && true == bRemoveSTAM )
		{
			CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetFamilyIDPtr().MP );

			if( NULL == pStrID )
			{
				continue;
			}

			if( 0 == _tcscmp( pStrID->GetIDstr2(), _T("RVTYPE_MV") ) )
			{
				continue;
			}
		}

		int iProdSizeKey = pTAP->GetSizeKey();
		m_mapValves.insert( std::make_pair( iProdSizeKey, CValvesData( pTAP ) ) );
	}

	if( m_mapValves.size() == 0 )
	{
		return;
	}
	
	// HYS-1308: Change the way we reject sizes.
	std::multimap <int, CValvesData>::iterator itLow = m_mapValves.end();
	std::multimap <int, CValvesData>::iterator itUp = m_mapValves.end();

	for( std::multimap <int, CValvesData>::iterator iter = m_mapValves.begin(); iter != m_mapValves.end(); iter++ )
	{
		if( iter->first >= iLowestSize )
		{
			itLow = iter;
			break;
		}
	}

	if( m_mapValves.end() == itLow )
	{
		itLow = m_mapValves.lower_bound( ( *m_mapValves.rbegin() ).first );
	}

	bool bFound = false;

	for( std::multimap <int, CValvesData>::reverse_iterator riter = m_mapValves.rbegin(); riter != m_mapValves.rend(); riter++ )
	{
		if( riter->first <= iHighestSize )
		{
			itUp = riter.base();
			bFound = true;
			break;
		}
	}

	if( m_mapValves.end() == itUp && false == bFound )
	{
		itUp = m_mapValves.upper_bound( (*m_mapValves.begin()).first );
	}

	if( m_mapValves.end() != itLow && m_mapValves.begin() != itLow )
	{
		m_mapValves.erase( m_mapValves.begin(), itLow );
	}

	if( m_mapValves.end() != itUp )
	{
		m_mapValves.erase( itUp, m_mapValves.end() );
	}

	if( 0 == m_mapValves.size() )
	{
		return;
	}

	/*  HYS-708
		<double dDeltaOpening, CDB_TAProduct *> Map for best valves,
		If we can reach Dp we store Delta setting or DeltaDp (in case of FO .
		If the valve cannot reach requested Dp we store Delta Dp * 100 (*100 to be far enough of Delta setting)
		by doing this we give a preference to the valve that satisfy requested Dp
	*/
	typedef std::multimap<double, CDB_TAProduct *> _TMapDelta;
	_TMapDelta mapDelta;

	// Full BV list, start to order it
	if( 0 == _tcscmp( pBVType->GetIDPtr().ID, _T("RVTYPE_CS") ) )
	{
		double dSignalMax = m_pParent->GetpTechParam()->GetFoMaxDp();

		for( itMapValves = m_mapValves.begin(); itMapValves != m_mapValves.end(); ++itMapValves )
		{
			int iProdSizeKey = itMapValves->first;

			if( iProdSizeKey < iLowestSize || iProdSizeKey > iHighestSize )
			{
				continue;
			}

			double dKvSignal = -1.0;
			double dDpFixO = 0.0;
			CDB_CommissioningSet *pCSValve = dynamic_cast<CDB_CommissioningSet *>( itMapValves->second.GetpTAP() );

			if( NULL == pCSValve )
			{
				continue;
			}

			double dKv = pCSValve->GetKvFixO();

			if( -1.0 != dKv )
			{
				dDpFixO = CalcDp( dFlow, dKv, dRho );
			}

			dKvSignal = pCSValve->GetKvSignal();

			if( -1.0 == dKvSignal )
			{
				// Sanity
				continue;
			}

			double dSignal;
			dSignal = CalcDp( dFlow, dKvSignal, dRho );
			dSignal += dDpFixO;
			itMapValves->second.SetSignal( dSignal );

			if( dSignal <= dSignalMax )
			{
				double dDeltaSignal = ( dSignalMax - dSignal );	// Should be > 0
				mapDelta.insert( std::make_pair( dDeltaSignal, itMapValves->second.GetpTAP() ) );
			}
			else
			{
				// Fully open valve cannot reach requested Dp
				double dDeltaSignal = ( dSignal - dSignalMax ) * 100;		// Should be > 0
				mapDelta.insert( std::make_pair( dDeltaSignal, itMapValves->second.GetpTAP() ) );
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
		double dTargetSignal = pow( ( sqrt( m_pParent->GetpTechParam()->GetVtriMinDp() ) + sqrt( m_pParent->GetpTechParam()->GetVtriMaxDp() ) ), 2 ) / 4.0;

		// Compute the signal pressure drop.
		double dTemperature = pclWaterChar->GetTemp();
		double dBestDelta = DBL_MAX;
		std::pair<double, CSelectedBase *> pairBestVV( 0.0, NULL );

		// These two variables are used when dp required is defined.
		// It can happen that we have no valve with the dp required. In this case we will take the worse one.
		_TMapDelta mapTempDelta;
		double dBestWorse = DBL_MAX;
		double dDpRequired = dDp;

		for( auto &iter : m_mapValves )
		{
			CDB_VenturiValve *pclVenturiValve = dynamic_cast<CDB_VenturiValve *>( iter.second.GetpTAP() );

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
			bool bNotFound;
			bool bIsDpDefined = ( dDpRequired > 0.0 ) ? true : false;
			double dDpComputed = 0.0;
	
			// Selection process from flow and Dp if DpOK==true or from flow only if DpOK==false.
			if( true == bIsDpDefined )
			{
				// If the selected Dp is obtainable...
				if( dDpRequired >= dDpFullyOpen )
				{
					// Try to find a solution.
					dDpComputed = dDpRequired;
					bool bValid = pDevChar->GetValveOpening( dFlow, dDpComputed, &dH, dRho, dNu, 1 );
				
					if( false == bValid )
					{
						// The min Kv value was not small enough !
						dH = pDevChar->GetOpeningMin();

						if( -1.0 != dH )
						{
							if( false == pDevChar->GetValveDp( dFlow, &dDpComputed, dH, dRho, dNu ) )
							{
								continue;
							}
						}
					}
				}
			}
			else
			{
				dH = dHMax;
				dDpComputed = dDpFullyOpen;
			}

			// If h>=0, a solution was found.
			bNotFound = ( dH < 0.0 ) ? true : false;
		
			if( true == bNotFound )
			{
				dH = dHMax;
				dDpComputed = dDpFullyOpen;
			}

			iter.second.SetDp( dDpComputed );
			iter.second.SetSignal( dDpSignal );
			iter.second.SetDpFO( dDpFullyOpen );
			iter.second.SetSetting( dH );

			double dDelta = 0.0;

			if( 0.0 == dDpComputed )
			{
				dDelta = abs( dDpSignal - dTargetSignal );
				
				if( dDelta < dBestDelta && dDpSignal > m_pParent->GetpTechParam()->GetVtriMinDp() )
				{
					dBestDelta = dDelta;
				}
				
				mapDelta.insert( std::make_pair( dDelta, pclVenturiValve ) );
			}
			else
			{
				// The best valve is the one that is the closest to 75% of opening.
				CDB_ValveCharacteristic *pValveCharacteristic = (CDB_ValveCharacteristic *)pclVenturiValve->GetValveCharDataPointer();
								
				if( NULL != pValveCharacteristic )
				{
					dDelta = fabs( dH - ( 0.75 * pValveCharacteristic->GetOpeningMax() ) );

					if( dDelta < dBestDelta )
					{
						if( dDpComputed == dDpRequired )
						{
							// If Dp required is reached, we take this valve as the best...
							dBestDelta = dDelta;
							mapDelta.insert( std::make_pair( dDelta, pclVenturiValve ) );
						}
						else
						{
							// Dp required not reached, we take this one in case of there is no other valve that can
							// reach the dp. We will take the one that has its Dp at full opening the closest to the dp required.
							if( fabs( dDpComputed - dDpRequired ) < dBestWorse )
							{
								dBestWorse = fabs( dDpComputed - dDpRequired );
								mapTempDelta.insert( std::make_pair( dBestWorse, pclVenturiValve ) );
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
		double dTargetInPercentOfOpening = 0.75;

		if( true == m_pParent->IsCircuitIndex() )
		{
			dTargetInPercentOfOpening = 1;
		}

		// For each valve found
		// Establish two ordered list
		//			Dp reached store quality of settings (setting to reach Dp minus optimum settings)
		//			Dp not reached, valve is fully open store Dp difference

		// Create a map that keep already computed characteristics
		// target of this map is to increase processing
		std::map <CDB_ValveCharacteristic *, std::pair<double, double>> mapValveChar;				// std::pair<dDpFO, dH>
		std::map <CDB_ValveCharacteristic *, std::pair<double, double>>::iterator itValveChar;
		CDB_ValveCharacteristic *pValveChar65 = NULL;

		for( itMapValves = m_mapValves.begin(); itMapValves != m_mapValves.end(); ++itMapValves )
		{
			CDB_TAProduct *pValve = dynamic_cast<CDB_TAProduct *>( itMapValves->second.GetpTAP() );

			if( NULL != pValve )
			{
				int iSizeKey = pValve->GetSizeKey();
				CDB_ValveCharacteristic *pValveChar = pValve->GetValveCharacteristic();

				if( NULL == pValveChar )
				{
					// Sanity
					continue;
				}

				double dMaxSetting = pValveChar->GetOpeningMax();

				if( -1.0 == dMaxSetting )
				{
					// Sanity
					continue;
				}

				// Specific treatment for STAF65-2 vs STAF80 and for settings included between 2.6 && 4.8.
				if( iSizeKey ==  8 )	// STAF 65-2
				{
					pValveChar65 = pValveChar;
				}

				double dTargetSetting = dMaxSetting * dTargetInPercentOfOpening;

				double dDpFO = 0;
				itValveChar = mapValveChar.find( pValveChar );

				if( itValveChar != mapValveChar.end() )
				{
					dDpFO = itValveChar->second.first;
				}
				else
				{
					dDpFO = pValveChar->GetDpFullOpening( dFlow, dRho, dNu );
				}

				double dH = -1;

				if( dDpFO <= dDp )
				{
					// Valve Dp @ full opening is below 3kPa
					// We will compute distance between dtargetsetting (0.75 * MaxSetting) and computed opening for requested Dp
					bool bflag = true;

					if( itValveChar != mapValveChar.end() )
					{
						dH = itValveChar->second.second;
					}
					else
					{
						bflag = pValveChar->GetValveOpening( dFlow, dDp, &dH, dRho, dNu, ( eBool3::eb3True == pValveChar->IsMultiTurn() ) ? 0 : 1 );
					}

					if( false == bflag )
					{
						// Sanity
						continue;
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

					mapDelta.insert( std::make_pair( dDeltaSetting, pValve ) );
					
					// Update the main map with computed data.
					itMapValves->second.SetDp( dDp );
					itMapValves->second.SetDpFO( dDpFO );
					itMapValves->second.SetSetting( dH );
				}
				else
				{
					// Fully open valve cannot reach the Dp requested.
					double dDeltaDp = ( dDpFO - dDp ) * 100;		// Should be > 0
					mapDelta.insert( std::make_pair( dDeltaDp, pValve ) );
					
					// Update the main map with computed data.
					itMapValves->second.SetDp( dDpFO );
					itMapValves->second.SetDpFO( dDpFO );
					itMapValves->second.SetSetting( dMaxSetting );
				}

				// Store new characteristic results.
				if( itValveChar == mapValveChar.end() )
				{
					mapValveChar[pValveChar] = std::pair<double, double> ( dDpFO, dH );
				}
			}
		}
	}

	bFound = false;
	_TMapDelta::iterator It = mapDelta.end();
	int iCurrentSize = -1;
	CDB_TAProduct *pCurrentTAP = GetpTAP();

	// When user change the current type, version, connection , PN, ... the current selected product is invalid
	// Try to find matching (connection, version, ...) valve in the full list of suggested products
	bool fUserChangeSelParameters = false;

	if( NULL != m_pUserChoiceBVVers || NULL != m_pUserChoiceBVConn || NULL != m_pUserChoiceBVPN )
	{
		fUserChangeSelParameters = true;
		pCurrentTAP = NULL;
	}

	if( NULL != pCurrentTAP )
	{
		iCurrentSize = pCurrentTAP->GetSizeKey();
	}

	// Build Sorting keys
	// Tech Param below DN65
	// IDType is already fixed by the filling of the full list
	CDB_StringID *psidType = NULL;
	CDB_StringID *psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvBlw65FamilyID ).MP );
	CDB_StringID *psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvBlw65BdyMatID ).MP );
	CDB_StringID *psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvBlw65ConnectID ).MP );
	CDB_StringID *psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvBlw65VersionID ).MP );
	CDB_StringID *psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvBlw65PNID ).MP );
	CTAPSortKey sKeyTechParamBlw65( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );

	CTAPSortKey sKeyUserChangeSelParamsBlw65( m_pUserChoiceBVType, NULL, psidFam, psidBdy, m_pUserChoiceBVConn, m_pUserChoiceBVVers, m_pUserChoiceBVPN, 0 );

	// Tech Param above DN50
	// IDType is already fixed by the filling of the full list
	psidType = NULL;
	psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvAbv50FamilyID ).MP );
	psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvAbv50BdyMatID ).MP );
	psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvAbv50ConnectID ).MP );
	psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvAbv50VersionID ).MP );
	psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvAbv50PNID ).MP );
	CTAPSortKey sKeyTechParamAbv50( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );

	CTAPSortKey sKeyUserChangeSelParamsAbv50( m_pUserChoiceBVType, NULL, psidFam, psidBdy, m_pUserChoiceBVConn, m_pUserChoiceBVVers, m_pUserChoiceBVPN, 0 );


	double dKeyCurrentTAP = 0;
	int iStartLoop = ( true == fUserChangeSelParameters ) ? iStartLoop = -2 : iStartLoop = 0;
	int iEndLoop = 6;

	_TMapDelta *pMap = &mapDelta;

	if( pMap->size() > 0 )
	{
		for( int UserPrefLoop = iStartLoop; UserPrefLoop < iEndLoop && false == bFound; UserPrefLoop++ )
		{
			// This loop give us the possibility to relax mask criteria one by one
			// we will start with the most constraining key and if we don't found a product we will relax constrains one by one
			CTAPSortKey::TAPSortKeyMask eMask = ( CTAPSortKey::TAPSortKeyMask )( CTAPSortKey::TAPSKM_All & ~CTAPSortKey::TAPSKM_PriorL & ~CTAPSortKey::TAPSKM_Size &
												~CTAPSortKey::TAPSKM_Type );

			switch( UserPrefLoop )
			{
				case -2:
				{
					// We will try to find a matching product keeping family and body material as defined
					if( NULL == m_pUserChoiceBVPN )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
					}

					if( NULL == m_pUserChoiceBVVers )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Version );
					}

					if( NULL == m_pUserChoiceBVConn )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Connect );
					}

				}
				break;

				case -1:
				{
					// ( true == fUserChangeSelParameters )
					// Body material is not available as a user choice (combo doesn't exist)
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Bdy );
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Fam );
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Type );

					if( NULL == m_pUserChoiceBVPN )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
					}

					if( NULL == m_pUserChoiceBVVers )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Version );
					}

					if( NULL == m_pUserChoiceBVConn )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Connect );
					}
				}
				break;

				case 0:
				{
					// All
					if( true == fUserChangeSelParameters )
					{
						// No solution found relax user request
						fUserChangeSelParameters = false;
						ClearUserChoiceTemporaryVariables();
					}
				}
				break;

				case 1:
				{
					// Relax PN
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
				}
				break;

				case 2:
				{
					// Relax PN and Version
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version );
				}
				break;

				case 3:
				{
					// Relax PN, version and connection
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version & ~CTAPSortKey::TAPSKM_Connect );
				}
				break;

				case 4:
				{
					// Relax PN, version, connection, BdyMat
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version & ~CTAPSortKey::TAPSKM_Connect &
							~CTAPSortKey::TAPSKM_Bdy );
				}
				break;

				case 5:
				{
					// Relax PN, version, connection, BdyMat, family
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version & ~CTAPSortKey::TAPSKM_Connect &
							~CTAPSortKey::TAPSKM_Bdy & ~CTAPSortKey::TAPSKM_Fam );
				}
				break;
			}

			double dKeyTechParamBlw65 = sKeyTechParamBlw65.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
			double dKeyTechParamAbv50 = sKeyTechParamAbv50.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
			double dKeyUserChangeSelParamsBlw65 = sKeyUserChangeSelParamsBlw65.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
			double dKeyUserChangeSelParamsAbv50 = sKeyUserChangeSelParamsAbv50.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );

			// Extract key for currently selected TAP
			if( NULL != pCurrentTAP )
			{
				dKeyCurrentTAP = pCurrentTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
			}

			if( true == fUserChangeSelParameters )
			{
				_TMapDelta::iterator ItBestPriority = pMap->end();

				for( It = pMap->begin(); It != pMap->end(); ++It )
				{
					CDB_TAProduct *pTAP = It->second;
					int iSize = pTAP->GetSizeKey();
					double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
					double dKey = ( iSize <= iDN50 ) ? dKeyUserChangeSelParamsBlw65 : dKeyUserChangeSelParamsAbv50;

					if( dTAPKey == dKey )
					{
						if( ItBestPriority != pMap->end() )
						{
							if( ItBestPriority->second->GetPriorityLevel() > pTAP->GetPriorityLevel() )
							{
								ItBestPriority = It;
								bFound = true;
							}
						}
						else
						{
							ItBestPriority = It;
							bFound = true;
						}
					}
				}

				if( true == bFound )
				{
					It = ItBestPriority;
				}
			}
			else
			{
				// If a current selection exist
				if( NULL != pCurrentTAP )
				{
					for( It = pMap->begin(); It != pMap->end(); ++It )
					{
						CDB_TAProduct *pTAP = It->second;
						int iSize = pTAP->GetSizeKey();

						double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );

						// HYS-1636: Check bv size if it's used with DpC. iDNDpC is INT_MAX when there is no DpC or when it's not needed to get the same size.
						bool bCheckDpC = false;

						if( (iDNDpC != INT_MAX && iSize == iDNDpC)
							|| (iDNDpC == INT_MAX) )
						{
							bCheckDpC = true;
						}

						if( ( iCurrentSize <= iDN50 && iSize <= iDN50 ) || ( iCurrentSize > iDN50 && iSize > iDN50 ) )
						{
							// Selected valve exist and it's located on the same side of iDN50 border.
							// Try to find a valve that match all requested criteria.
							if( dTAPKey == dKeyCurrentTAP && true == bCheckDpC )
							{
								bFound = true;
							}
						}
						else
						{
							// Selected valve is not located on the same side of iDN50 border.
							// Use defined technical choice
							if( iSize <= iDN50 )
							{
								if( dTAPKey == dKeyTechParamBlw65 && true == bCheckDpC )
								{
									bFound = true;
								}
							}
							else
							{
								// Size > DN50
								if( dTAPKey == dKeyTechParamAbv50 && true == bCheckDpC )
								{
									bFound = true;
								}
							}
						}

						if( true == bFound )
						{
							// Abort loop by this way we preserve Iterator
							break;
						}
					}
				}

				// Not found or previous valve doesn't exist, restart loop only on tech params
				if( false == bFound )
				{
					for( It = pMap->begin(); It != pMap->end(); ++It )
					{
						CDB_TAProduct *pTAP = It->second;
						int iSize = pTAP->GetSizeKey();
						double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );

						// HYS-1636: Check bv size if it's used with DpC. iDNDpC is INT_MAX when there is no DpC or when it's not needed to get the same size.
						bool bCheckDpC = false;

						if( (iDNDpC != INT_MAX && iSize == iDNDpC)
							|| (iDNDpC == INT_MAX) )
						{
							bCheckDpC = true;
						}

						// Selected valve is not located on the same side of iDN50 border.
						// Use defined technical choice
						if( iSize <= iDN50 )
						{
							if( dTAPKey == dKeyTechParamBlw65 && bCheckDpC == true )
							{
								bFound = true;
							}
						}
						else
						{
							// Size > DN50
							if( dTAPKey == dKeyTechParamAbv50 && bCheckDpC == true )
							{
								bFound = true;
							}
						}

						if( true == bFound )
						{
							// Abort loop and by this way we preserve Iterator
							break;
						}
					}
				}
			}
		}
	}

	// If best valve found...
	if( true == bFound )
	{
		CDB_TAProduct *pTAP = It->second;

		if( NULL != pTAP )
		{
			SetBestBvIDPtr( pTAP->GetIDPtr() );
			_SetpSelBV( (LPARAM)pTAP );

			if( true == fUserChangeSelParameters )
			{
				// When the current product is valid we will analyze if the user introduced an exception regarding defined PrjParam,
				// in this case we will not consider the DN50 border that force an automatic jump.
				if( pTAP->GetSizeKey() <= iDN50 )
				{
					if( ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvBlw65TypeID ).MP == pTAP->GetTypeIDPtr().MP )
							&& ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvBlw65ConnectID ).MP == pTAP->GetConnectIDPtr().MP )
							&& ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvBlw65VersionID ).MP == pTAP->GetVersionIDPtr().MP )
							&& ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvBlw65PNID ).MP == pTAP->GetPNIDPtr().MP ) )
					{
						ClearUserChoiceTemporaryVariables();
					}
				}
				else
				{
					// DN65 and above
					if( ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvAbv50TypeID ).MP == pTAP->GetTypeIDPtr().MP )
							&& ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvAbv50ConnectID ).MP == pTAP->GetConnectIDPtr().MP )
							&& ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvAbv50VersionID ).MP == pTAP->GetVersionIDPtr().MP )
							&& ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvAbv50PNID ).MP == pTAP->GetPNIDPtr().MP ) )
					{
						ClearUserChoiceTemporaryVariables();
					}
				}
			}
		}
		else
		{
			InvalidateSelection();
			ClearUserChoiceTemporaryVariables();
		}
	}
	else
	{
		InvalidateSelection();
		ClearUserChoiceTemporaryVariables();
	}
}

bool CDS_HydroMod::CBV::IsCompletelyDefined()
{
	if( NULL == GetIDPtr().MP )				// safe IDPtr, checked by Extend in GetIDPtr()
	{
		return false;
	}

	if( 0.0 == GetSetting() )
	{
		return false;
	}

	if( 0.0 == GetDp() )
	{
		return false;
	}

	return true;
}

bool CDS_HydroMod::CBV::IsBestBv( CData *pData )
{
	return ( pData == GetBestBvIDPtr().MP ) ? true : false;
}

bool CDS_HydroMod::CBV::IsKvSignalEquipped()
{
	// Valve exist.
	if( NULL != GetIDPtr().MP )
	{
		return ( ( CDB_TAProduct * ) GetIDPtr().MP )->IsKvSignalEquipped();
	}

	return false;
}

double CDS_HydroMod::CBV::GetDpMin( bool fBounds )
{
	double dQ = GetQ();

	if( dQ <= 0 )
	{
		return 0.0;
	}
	
	// Valve exist.
	if( NULL != GetIDPtr().MP )
	{
		CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( GetHMObjectType() );
	
		if( NULL == pclWaterChar )
		{
			ASSERTA_RETURN( 0.0 );
		}

		CDB_ValveCharacteristic *pDevChar = GetpValveChar();

		if( NULL != pDevChar )
		{
			double dRho = pclWaterChar->GetDens();
			double dNu = pclWaterChar->GetKinVisc();
			double dDpFO = pDevChar->GetDpFullOpening( dQ, dRho, dNu );

			if( -1.0 == dDpFO )
			{
				dDpFO = 0.0;
			}

			// For CDB_VenturiValve Dp fully open includes Venturi Dp.
			if( true == GetIDPtr().MP->IsClass( CLASS( CDB_CommissioningSet ) ) )
			{
				// Compute the Fixed orifice pressure drop.
				double dKv = ( ( CDB_CommissioningSet * ) GetIDPtr().MP )->GetKvFixO();
				double DpFixO = 0.0;

				if( -1.0 != dKv )
				{
					DpFixO = CalcDp( dQ, dKv, dRho );
				}

				dDpFO += DpFixO;
			}

			double dValvMinDp = GetTechParamMinDp();

			if( true == fBounds )
			{
				return max( dDpFO, dValvMinDp );
			}

			return dDpFO;
		}
	}

	return 0.0;	// No valve selected
}

CString CDS_HydroMod::CBV::GetSettingStr( bool bWithUnit, bool *pbFullSetting )
{
	if( NULL != pbFullSetting )
	{
		*pbFullSetting = false;
	}

	CString str;

	if( NULL != GetIDPtr().MP )
	{
		CDB_ValveCharacteristic *pDevChar = dynamic_cast<CDB_ValveCharacteristic *>( GetpValveChar() );

		if( NULL != pDevChar )
		{
			str = pDevChar->GetSettingString( GetSetting(), bWithUnit, pbFullSetting );
		}
	}

	return str;
}

int CDS_HydroMod::CBV::CheckValidity()
{
	m_usValidityFlags = CDS_HydroMod::eValidityFlags::evfOK;

	if( NULL == GetIDPtr().MP )
	{
		return m_usValidityFlags;
	}

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( GetIDPtr().MP );

	if( NULL == pTAP )
	{
		return m_usValidityFlags;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( GetHMObjectType() );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( m_usValidityFlags );
	}

	if( pTAP->GetTmax() < pclWaterChar->GetTemp() )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfTempTooHigh;
	}

	if( GetDp() > m_pParent->GetpTechParam()->VerifyValvMaxDp( ( CDB_TAProduct * ) GetIDPtr().MP ) )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDpTooHigh;
	}

	if( GetDp() < m_pParent->GetpTechParam()->GetValvMinDp() )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDpTooLow;
	}

	if( pTAP->GetKvSignal() > 0.0 )
	{
		if( GetDpSignal() > m_pParent->GetpTechParam()->GetFoMaxDp() )
		{
			m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDpSignalTooHigh;
		}

		if( GetDpSignal() < m_pParent->GetpTechParam()->GetFoMinDp() )
		{
			m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDpSignalTooLow;
		}
	}

	return m_usValidityFlags;
}

void CDS_HydroMod::CBV::DropData( OUTSTREAM outf, int *piLevel )
{
	CString strTab = _T("");

	for( int i = *piLevel; i; i-- )
	{
		strTab += _T("\t");
	}

	CString str;
	str.Format( _T("m_IDPtr.ID = %s\n"), GetIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_DiversityIDPtr.ID = %s\n"), GetDiversityIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_IDPtrBestBV.ID = %s\n"), GetBestBvIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Q = %s\n"), WriteCUDouble( _U_FLOW, GetQ(), true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Dp = %s\n"), WriteCUDouble( _U_DIFFPRESS, GetDp(), true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_DpSignal = %s\n"), WriteCUDouble( _U_DIFFPRESS, GetDpSignal(), true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Setting = %s\n"), WriteDouble( GetSetting(), 2, 2 ) );
	WriteFormatedStringA2( outf, str, strTab );

	str = _T("m_Located = ");

	switch( GetHMObjectType() )
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

	_DropDataHelp( str, GetpSelBVType(), _T("m_pSelBVType") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, GetpSelBVConn(), _T("m_pSelBVConn") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, GetpSelBVVers(), _T("m_pSelBVVers") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, GetpSelBVPN(), _T("m_pSelBVPN") );
	WriteFormatedStringA2( outf, str, strTab );

	_DropDataHelp( str, m_pUserChoiceBVType, _T("m_pUserChoiceBVType") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, m_pUserChoiceBVConn, _T("m_pUserChoiceBVConn") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, m_pUserChoiceBVVers, _T("m_pUserChoiceBVVers") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, m_pUserChoiceBVPN, _T("m_pUserChoiceBVPN") );
	WriteFormatedStringA2( outf, str, strTab );

	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_usValidityFlags = %u\n"), GetValidtyFlags() );
	WriteFormatedStringA2( outf, str, strTab );
	WriteFormatedStringA2( outf, _T("\n"), strTab );
}

// Based on BV localisation return associated CDB_Pipe *
CPipes *CDS_HydroMod::CBV::GetpPipe()
{
	if( CDS_HydroMod::eBVprim == GetHMObjectType() )
	{
		return m_pParent->GetpCircuitPrimaryPipe();
	}

	if( CDS_HydroMod::eBVsec == GetHMObjectType() )
	{
		return m_pParent->GetpCircuitSecondaryPipe();
	}

	if( CDS_HydroMod::eBVbyp == GetHMObjectType() )
	{
		return m_pParent->GetpCircuitPrimaryPipe();
	}

	ASSERT( 0 );
	return m_pParent->GetpCircuitPrimaryPipe();
}

bool CDS_HydroMod::CBV::IsBvExistInPreselectedList( CDB_TAProduct *pclBalancingValve )
{
	if( 0 == (int)m_mapValves.size() )
	{
		return false;
	}

	bool bBVExist = false;

	for( auto &iter : m_mapValves )
	{
		if( pclBalancingValve == iter.second.GetpTAP() )
		{
			bBVExist = true;
			break;
		}
	}

	return bBVExist;
}

void CDS_HydroMod::CBV::ClearUserChoiceTemporaryVariables()
{
	m_pUserChoiceBVType = NULL;
	m_pUserChoiceBVConn = NULL;
	m_pUserChoiceBVVers = NULL;
	m_pUserChoiceBVPN = NULL;
}

int CDS_HydroMod::CBV::GetAvailableConnections( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapValves.size() )
	{
		return 0;
	}

	pmap->clear();
	std::multimap <int, CValvesData >::iterator It;

	for( It = m_mapValves.begin(); It != m_mapValves.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpTAP();
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

int CDS_HydroMod::CBV::GetAvailableVersions( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapValves.size() )
	{
		return 0;
	}

	pmap->clear();

	CDB_StringID *pStrIDConnection = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDConnection = dynamic_cast<CDB_StringID *>( GetpTAP()->GetConnectIDPtr().MP );
	}

	std::multimap <int, CValvesData >::iterator It;

	for( It = m_mapValves.begin(); It != m_mapValves.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpTAP();
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );

		if( pStrID != pStrIDConnection )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetVersionIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}


int CDS_HydroMod::CBV::GetAvailablePN( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapValves.size() )
	{
		return 0;
	}

	pmap->clear();
	CDB_StringID *pStrIDConnection = NULL;
	CDB_StringID *pStrIDVersion = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDConnection = dynamic_cast<CDB_StringID *>( GetpTAP()->GetConnectIDPtr().MP );
		pStrIDVersion = dynamic_cast<CDB_StringID *>( GetpTAP()->GetVersionIDPtr().MP );
	}

	std::multimap <int, CValvesData >::iterator It;

	for( It = m_mapValves.begin(); It != m_mapValves.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpTAP();
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );

		if( pStrID != pStrIDConnection )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetVersionIDPtr().MP );

		if( pStrID != pStrIDVersion )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetPNIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

//////////////////////////////////////////////////////////////////////
CDS_HydroMod::CCv::CCv( CDS_HydroMod *pHM, CvLocation eCvLocated ) : CBase( pHM )
{
	m_pRHSerie = static_cast< CDB_MultiString * >( TASApp.GetpTADB()->Get( _T("REYNARD_SERIES") ).MP );
	ASSERT( NULL != m_pRHSerie );

	m_IDPtrCv = _NULL_IDPTR;
	m_DiversityIDPtr = _NULL_IDPTR;
	m_IDPtrBestCV = _NULL_IDPTR;
	m_eCvLocated = eCvLocated;
	m_bCVSelectedAsaPackage = eb3Undef;
	m_ActrIDPtr = _NULL_IDPTR;					// IDPTR for selected actuator
	m_ActrAdaptIDPtr = _NULL_IDPTR;				// IDPTR for selected actuator
	m_ActrSelectedVoltageIDPtr = _NULL_IDPTR;	// IDPTR for selected voltage
	m_ActrSelectedSignalIDPtr = _NULL_IDPTR;	// IDPTR for selected signal
	m_bActrSelectedAsaPackage = eb3Undef;		// True when the selection was done as a package
	m_bActrSelectioAllowed = eb3Undef;
	m_ArCVAccIDPtr.SetSize( 0 );				// IDPTRs for selected CV accessories
	m_ArActrAccIDPtr.SetSize( 0 );				// IDPTRs for selected ACTR accessories
	m_ArCVActrAccSetIDPtr.SetSize( 0 );			// IDPTRs for selected CV-ACTR set accessories.
	m_bCVActrAccSetRefreshed = false;			// 'False' to signal that 'm_arCVActrAccSetIDPtr' is not yet refreshed.
	m_bCVLocAuto = true;
	m_mapCV.clear();
	m_strDescription = _T("");
	m_dKvsMax = 0.0;
	m_dKvs = 0.0;
	m_dQ = 0.0;
	m_dAuthority = 0.0;
	m_dSetting = 0.0;
	m_dBestKvs = 0.0;
	m_eWay = CV2W;
	m_bTACv = false;
	m_DpMin = 0.0;
	m_eControlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
	m_pUserChoiceCVConn = NULL;
	m_pUserChoiceCVVers = NULL;
	m_pUserChoiceCVPN = NULL;
	m_usValidityFlags = 0;
}

CDS_HydroMod::CCv::~CCv()
{
}

void CDS_HydroMod::CCv::Copy( CDS_HydroMod::CCv *pclTargetHMCv, bool bResetDiversityFactor )
{
	pclTargetHMCv->SetDescription( m_strDescription );
	pclTargetHMCv->SetCvLocate( GetCvLocate() );
	pclTargetHMCv->SetnWay( m_eWay );
	pclTargetHMCv->SetQ( m_dQ );
	pclTargetHMCv->SetKvs( m_dKvs );
	pclTargetHMCv->SetCtrlType( m_eControlType );
	pclTargetHMCv->SetTACv( m_bTACv );
	pclTargetHMCv->SetKvsmax( m_dKvsMax );
	pclTargetHMCv->SetAuth( m_dAuthority );
	pclTargetHMCv->SetBestKvs( GetBestKvs(), false );
	pclTargetHMCv->SetCvIDPtr( GetCvIDPtr() );
	pclTargetHMCv->SetBestCvIDPtr( GetBestCvIDPtr() );
	pclTargetHMCv->SetDiversityIDPtr( (true == bResetDiversityFactor) ? _NULL_IDPTR : GetDiversityIDPtr() );
	pclTargetHMCv->SetValidtyFlags( GetValidtyFlags() );
	
	// HYS-731: The copy of settings was deleted and it prevents to detect the valve as locked
	pclTargetHMCv->SetSetting( m_dSetting );

	if( NULL == m_pUserChoiceCVConn && NULL == m_pUserChoiceCVVers && NULL == m_pUserChoiceCVPN )
	{
		pclTargetHMCv->ClearUserChoiceTemporaryVariables();
	}
	else
	{
		pclTargetHMCv->SetpUserChoiceCVPN( m_pUserChoiceCVPN );
		pclTargetHMCv->SetpUserChoiceCVVers( m_pUserChoiceCVVers );
		pclTargetHMCv->SetpUserChoiceCVConn( m_pUserChoiceCVConn );
	}

	// Actuator and accessories.
	pclTargetHMCv->SetActrIDPtr( m_ActrIDPtr );
	pclTargetHMCv->SetActrAdaptIDPtr( m_ActrAdaptIDPtr );
	pclTargetHMCv->SetActrSelectedVoltageIDPtr( m_ActrSelectedVoltageIDPtr );
	pclTargetHMCv->SetActrSelectedSignalIDPtr( m_ActrSelectedSignalIDPtr );
	pclTargetHMCv->SetActrSelectedAsaPackage( m_bActrSelectedAsaPackage );
	pclTargetHMCv->SetActrSelectionAllowed( m_bActrSelectioAllowed );

	pclTargetHMCv->SetCVSelectedAsaPackage( m_bCVSelectedAsaPackage );
	pclTargetHMCv->SetCVLocAuto( m_bCVLocAuto );

	for( int i = 0; i < m_ArCVAccIDPtr.GetSize(); i++ )
	{
		pclTargetHMCv->AddCVAccIDPtr( m_ArCVAccIDPtr[i] );
	}

	for( int i = 0; i < m_ArCVAccIDPtr.GetSize(); i++ )
	{
		pclTargetHMCv->AddActrAccIDPtr( m_ArActrAccIDPtr[i] );
	}

	for( int i = 0; i < m_ArCVActrAccSetIDPtr.GetSize(); i++ )
	{
		pclTargetHMCv->AddCVActrAccSetIDPtr( m_ArCVActrAccSetIDPtr[i] );
	}
}

bool CDS_HydroMod::CCv::IsCvExistInPreselectedList( CDB_ControlValve *pclControlValve )
{
	if( 0 == (int)m_mapCV.size() )
	{
		return false;
	}

	bool bCVExist = false;

	for( auto &iter : m_mapCV )
	{
		if( pclControlValve == iter.second.GetpCV() )
		{
			bCVExist = true;
			break;
		}
	}

	return bCVExist;
}

#define CDS_HYDROMOD_CCV_VERSION	10
// Version 10: HYS-1757: There was a bug with the 'm_bCVLocAuto' before the version 10.
// Version 9: HYS-1209: 'm_fCVSelectedAsaPackage' variable added.
// Version 8: 2016-10-10: 'm_eLocated' variable added.
// Version 7: add 'm_ArCVActrAccSetIDPtr'.
void CDS_HydroMod::CCv::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_CCV_VERSION;
	WriteData<>( outf, Version );

	// Info.
	WriteString( outf, (LPCTSTR) m_strDescription );
	WriteString( outf, (LPCTSTR) m_DiversityIDPtr.ID );
	WriteString( outf, (LPCTSTR) m_IDPtrBestCV.ID );
	WriteData<>( outf, m_eWay );
	WriteData<>( outf, m_dKvs );
	WriteData<>( outf, m_eControlType );
	WriteData<>( outf, m_bTACv );
	WriteData<>( outf, m_dQ );
	WriteData<>( outf, m_dKvsMax );
	WriteData<>( outf, m_dAuthority );
	WriteData<>( outf, m_dBestKvs );
	WriteString( outf, (LPCTSTR) m_IDPtrCv.ID );
	WriteData<>( outf, m_dSetting );
	WriteString( outf, (LPCTSTR) m_ActrIDPtr.ID );	// IDPTR for selected actuator

	// CV accessories	
	int iSize = m_ArCVAccIDPtr.GetSize();
	WriteData<>( outf, iSize );

	for( int i = 0; i < iSize; i++ )
	{
		WriteString( outf, (LPCTSTR) m_ArCVAccIDPtr[i].ID );
	}
	
	// Actuator Accessories
	iSize = m_ArActrAccIDPtr.GetSize();
	WriteData<>( outf, iSize );

	for( int i = 0; i < iSize; i++ )
	{
		WriteString( outf, (LPCTSTR) m_ArActrAccIDPtr[i].ID );
	}

	WriteString( outf, (LPCTSTR) m_ActrAdaptIDPtr.ID );					// IDPTR for selected Adapter actuator
	WriteString( outf, (LPCTSTR) m_ActrSelectedVoltageIDPtr.ID );			// IDPTR for selected voltage
	WriteString( outf, (LPCTSTR) m_ActrSelectedSignalIDPtr.ID );			// IDPTR for selected signal
	WriteData<>( outf, m_bActrSelectedAsaPackage );
	WriteData<>( outf, m_bActrSelectioAllowed );

	// Version 6.
	CString strID;
	strID = ( NULL != m_pUserChoiceCVConn ) ? m_pUserChoiceCVConn->GetIDPtr().ID : L"";
	WriteString( outf, strID );
	strID = ( NULL != m_pUserChoiceCVVers ) ? m_pUserChoiceCVVers->GetIDPtr().ID : L"";
	WriteString( outf, strID );
	strID = ( NULL != m_pUserChoiceCVPN ) ? m_pUserChoiceCVPN->GetIDPtr().ID : L"";
	WriteString( outf, strID );

	// Version 7.
	_VerifyCVActrAccSet();
	iSize = m_ArCVActrAccSetIDPtr.GetSize();
	WriteData<>( outf, iSize );

	for( int i = 0; i < iSize; i++ )
	{
		WriteString( outf, (LPCTSTR) m_ArCVActrAccSetIDPtr[i].ID );
	}

	WriteData<>( outf, m_bCVLocAuto );

	// Version 8.
	WriteData<>( outf, m_eCvLocated );

	// Version 9.
	WriteData<>( outf, m_bCVSelectedAsaPackage );
}

bool CDS_HydroMod::CCv::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_CCV_VERSION;
	ReadData<>( inpf, Version );

	if( Version < 1 || Version > CDS_HYDROMOD_CCV_VERSION )
	{
		return false;
	}

	// Info.
	ReadString( inpf, m_strDescription );

	ReadString( inpf, m_DiversityIDPtr.ID, sizeof( m_DiversityIDPtr.ID ) );

	// Before version 4, we read 'm_SelIDPtr.ID' that is no more used. Now, we write 'm_DiversityIDPtr' at this place.
	// We must reset ID to 0 if version is below 5 because diversity was not exist.
	if( Version < 4 )
	{
		m_DiversityIDPtr.ID[0] = 0;
	}
	else
	{
		m_DiversityIDPtr.DB = TASApp.GetpTADB();
	}

	ReadString( inpf, m_IDPtrBestCV.ID, sizeof( m_IDPtrBestCV.ID ) );
	ReadData<>( inpf, m_eWay );
	ReadData<>( inpf, m_dKvs );

	if( Version < 3 )
	{
		bool fOnOff;
		ReadData<>( inpf, fOnOff );

		if( true == fOnOff )
		{
			m_eControlType = CDB_ControlProperties::CvCtrlType::eCvOnOff;
		}
		else
		{
			m_eControlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
		}
	}
	else
	{
		ReadData<>( inpf, m_eControlType );
	}

	ReadData<>( inpf, m_bTACv );
	ReadData<>( inpf, m_dQ );
	ReadData<>( inpf, m_dKvsMax );
	ReadData<>( inpf, m_dAuthority );
	ReadData<>( inpf, m_dBestKvs );

	double dDpMin;

	if( Version < 2 )
	{
		ReadData<>( inpf, dDpMin );
	}

	if( Version < 3 )
	{
		return true;
	}

	ReadString( inpf, m_IDPtrCv.ID, sizeof( m_IDPtrCv.ID ) );
	m_IDPtrCv.DB = TASApp.GetpTADB();
	ReadData<>( inpf, m_dSetting );

	if( Version < 4 )
	{
		return true;
	}

	ReadString( inpf, m_ActrIDPtr.ID, sizeof( m_ActrIDPtr.ID ) );

	if( 0 != *m_ActrIDPtr.ID )
	{
		m_ActrIDPtr = TASApp.GetpTADB()->Get( m_ActrIDPtr.ID );
	}
	else
	{
		m_ActrIDPtr = _NULL_IDPTR;
	}

	int iSize = 0;
	ReadData<>( inpf, iSize );

	for( int i = 0; i < iSize; i++ )
	{
		CString strID;
		ReadString( inpf, strID );
		IDPTR IDPtr = TASApp.GetpTADB()->Get( strID );
		ASSERT( IDPtr.MP );

		if( NULL != IDPtr.MP )		// Object exist into the database.
		{
			m_ArCVAccIDPtr.Add( IDPtr );
		}
	}

	iSize = 0;
	ReadData<>( inpf, iSize );

	for( int i = 0; i < iSize; i++ )
	{
		CString strID;
		ReadString( inpf, strID );
		IDPTR IDPtr = TASApp.GetpTADB()->Get( strID );
		ASSERT( IDPtr.MP );

		if( NULL != IDPtr.MP )		// Object exist into the database.
		{
			m_ArActrAccIDPtr.Add( IDPtr );
		}
	}

	if( Version < 5 )
	{
		return true;
	}

	ReadString( inpf, m_ActrAdaptIDPtr.ID, sizeof( m_ActrAdaptIDPtr.ID ) );

	if( 0 != *m_ActrAdaptIDPtr.ID )
	{
		m_ActrAdaptIDPtr = TASApp.GetpTADB()->Get( m_ActrAdaptIDPtr.ID );
	}
	else
	{
		m_ActrAdaptIDPtr = _NULL_IDPTR;
	}

	ReadString( inpf, m_ActrSelectedVoltageIDPtr.ID, sizeof( m_ActrSelectedVoltageIDPtr.ID ) );

	if( 0 != *m_ActrSelectedVoltageIDPtr.ID )
	{
		m_ActrSelectedVoltageIDPtr = TASApp.GetpTADB()->Get( m_ActrSelectedVoltageIDPtr.ID );
	}
	else
	{
		m_ActrSelectedVoltageIDPtr = _NULL_IDPTR;
	}

	ReadString( inpf, m_ActrSelectedSignalIDPtr.ID, sizeof( m_ActrSelectedSignalIDPtr.ID ) );

	if( 0 != *m_ActrSelectedSignalIDPtr.ID )
	{
		m_ActrSelectedSignalIDPtr = TASApp.GetpTADB()->Get( m_ActrSelectedSignalIDPtr.ID );
	}
	else
	{
		m_ActrSelectedSignalIDPtr = _NULL_IDPTR;
	}

	ReadData<>( inpf, m_bActrSelectedAsaPackage );
	ReadData<>( inpf, m_bActrSelectioAllowed );

	ClearUserChoiceTemporaryVariables();

	if( Version < 6 )
	{
		return true;
	}

	IDPTR idptr;
	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceCVConn = dynamic_cast<CDB_StringID *>( idptr.MP );
	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceCVVers = dynamic_cast<CDB_StringID *>( idptr.MP );
	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceCVPN = dynamic_cast<CDB_StringID *>( idptr.MP );

	if( Version < 7 )
	{
		return true;
	}

	iSize = 0;
	ReadData<>( inpf, iSize );

	for( int i = 0; i < iSize; i++ )
	{
		CString strID;
		ReadString( inpf, strID );
		IDPTR IDPtr = TASApp.GetpTADB()->Get( strID );
		ASSERT( NULL != IDPtr.MP );

		if( NULL != IDPtr.MP )		// Object exist into the database.
		{
			m_ArCVActrAccSetIDPtr.Add( IDPtr );
		}
	}

	m_bCVActrAccSetRefreshed = true;
	ReadData<>( inpf, m_bCVLocAuto );

	if( Version < 8 )
	{
		return true;
	}

	ReadData<>( inpf, m_eCvLocated );

	if( Version < 9 )
	{
		return true;
	}

	ReadData<>( inpf, m_bCVSelectedAsaPackage );

	if( Version < 10 )
	{
		// There was a bug with the 'm_bCVLocAuto' variable before the version 10. Sometimes the value read from the file
		// is not 'true' or 'false' but one other value and the 'm_eCvLocated' is not correct.

		if( m_bCVLocAuto > 1 )
		{
			m_bCVLocAuto = 1;
		}

		CAnchorPt::AnchorPtFunc *pAnchorPtFunc = NULL;

		if( true == m_pParent->GetpSch()->IsAnchorPtExist( CAnchorPt::eFunc::PICV ) )
		{
			pAnchorPtFunc = m_pParent->GetpSch()->GetAnchorPtDetails( CAnchorPt::eFunc::PICV );
		}
		else if( true == m_pParent->GetpSch()->IsAnchorPtExist( CAnchorPt::eFunc::DPCBCV ) )
		{
			pAnchorPtFunc = m_pParent->GetpSch()->GetAnchorPtDetails( CAnchorPt::eFunc::DPCBCV );
		}
		else if( true == m_pParent->GetpSch()->IsAnchorPtExist( CAnchorPt::eFunc::ControlValve ) )
		{
			pAnchorPtFunc = m_pParent->GetpSch()->GetAnchorPtDetails( CAnchorPt::eFunc::ControlValve );
		}

		m_eCvLocated = CvLocation::CvLocNone;

		if( NULL != pAnchorPtFunc )
		{
			if( CAnchorPt::CircuitSide_Primary == pAnchorPtFunc->m_eCircuitSide )
			{
				m_eCvLocated = CvLocation::CvLocPrimSide;
			}
			else if( CAnchorPt::CircuitSide_Secondary == pAnchorPtFunc->m_eCircuitSide )
			{
				m_eCvLocated = CvLocation::CvLocSecSide;
			}
		}
	}

	return true;
}

void CDS_HydroMod::CCv::SetCVSelectedAsaPackage( eBool3 fFlag )
{
	m_bCVSelectedAsaPackage = fFlag;
	m_pParent->Modified();
}

void CDS_HydroMod::CCv::SetActrIDPtr( IDPTR IDPtr )
{
	m_ActrIDPtr = IDPtr;
	m_pParent->Modified();
	m_bCVActrAccSetRefreshed = false;
}

void CDS_HydroMod::CCv::SetActrAdaptIDPtr( IDPTR IDPtr )
{
	m_ActrAdaptIDPtr = IDPtr;
	m_pParent->Modified();
}

void CDS_HydroMod::CCv::SetActrSelectedVoltageIDPtr( IDPTR IDPtr )
{
	m_ActrSelectedVoltageIDPtr = IDPtr;
	m_pParent->Modified();
}

void CDS_HydroMod::CCv::SetActrSelectedSignalIDPtr( IDPTR IDPtr )
{
	m_ActrSelectedSignalIDPtr = IDPtr;
	m_pParent->Modified();
}

void CDS_HydroMod::CCv::SetActrSelectionAllowed( eBool3 val )
{
	m_bActrSelectioAllowed = val;
	m_pParent->Modified();
}

void CDS_HydroMod::CCv::SetActrSelectedAsaPackage( eBool3 fFlag )
{
	m_bActrSelectedAsaPackage = fFlag;
	m_pParent->Modified();
}

IDPTR CDS_HydroMod::CCv::GetActrAdaptIDPtr()
{
	if( _T('\0') != *m_ActrAdaptIDPtr.ID && NULL != m_ActrAdaptIDPtr.DB )
	{
		Extend( &m_ActrAdaptIDPtr );
	}

	return m_ActrAdaptIDPtr;
}

IDPTR CDS_HydroMod::CCv::GetActrSelectedVoltageIDPtr()
{
	if( _T('\0') != *m_ActrSelectedVoltageIDPtr.ID && NULL != m_ActrSelectedVoltageIDPtr.DB )
	{
		Extend( &m_ActrSelectedVoltageIDPtr );
	}

	return m_ActrSelectedVoltageIDPtr;
}

IDPTR CDS_HydroMod::CCv::GetActrSelectedSignalIDPtr()
{
	if( _T('\0') != *m_ActrSelectedSignalIDPtr.ID && NULL != m_ActrSelectedSignalIDPtr.DB )
	{
		Extend( &m_ActrSelectedSignalIDPtr );
	}

	return m_ActrSelectedSignalIDPtr;
}

void CDS_HydroMod::CCv::AddCVAccIDPtr( IDPTR IDPtr )
{
	m_ArCVAccIDPtr.Add( IDPtr );
	m_pParent->Modified();
}

void CDS_HydroMod::CCv::AddActrAccIDPtr( IDPTR IDPtr )
{
	m_ArActrAccIDPtr.Add( IDPtr );
	m_pParent->Modified();
}

void CDS_HydroMod::CCv::AddCVActrAccSetIDPtr( IDPTR IDPtr )
{
	m_ArCVActrAccSetIDPtr.Add( IDPtr );
	m_pParent->Modified();
}

IDPTR CDS_HydroMod::CCv::GetActrIDPtr()
{
	if( _T('\0') != *m_ActrIDPtr.ID && NULL != m_ActrIDPtr.DB )
	{
		Extend( &m_ActrIDPtr );
	}

	return m_ActrIDPtr;
}

int CDS_HydroMod::CCv::GetCVActrAccSetCount( void )
{
	_VerifyCVActrAccSet();
	return m_ArCVActrAccSetIDPtr.GetCount();
}

IDPTR CDS_HydroMod::CCv::GetCVAccIDPtr( int iIndex )
{
	if( iIndex < m_ArCVAccIDPtr.GetSize() )
	{
		if( _T('\0') != *m_ArCVAccIDPtr[iIndex].ID && NULL != m_ArCVAccIDPtr[iIndex].DB )
		{
			Extend( &m_ArCVAccIDPtr[iIndex] );
		}

		return m_ArCVAccIDPtr[iIndex];
	}

	return _NULL_IDPTR;
}

IDPTR CDS_HydroMod::CCv::GetActrAccIDPtr( int iIndex )
{
	if( iIndex < m_ArActrAccIDPtr.GetSize() )
	{
		if( _T('\0') != *m_ArActrAccIDPtr[iIndex].ID && NULL != m_ArActrAccIDPtr[iIndex].DB )
		{
			Extend( &m_ArActrAccIDPtr[iIndex] );
		}

		return m_ArActrAccIDPtr[iIndex];
	}

	return _NULL_IDPTR;
}

IDPTR CDS_HydroMod::CCv::GetCVActrAccSetIDPtr( int iIndex )
{
	_VerifyCVActrAccSet();

	if( iIndex < m_ArCVActrAccSetIDPtr.GetSize() )
	{
		if( _T('\0') != *m_ArCVActrAccSetIDPtr[iIndex].ID && NULL != m_ArCVActrAccSetIDPtr[iIndex].DB )
		{
			Extend( &m_ArCVActrAccSetIDPtr[iIndex] );
		}

		return m_ArCVActrAccSetIDPtr[iIndex];
	}

	return _NULL_IDPTR;
}

CArray <IDPTR> *CDS_HydroMod::CCv::GetpCVActrAccSetIDPtrArray()
{
	_VerifyCVActrAccSet();
	return &m_ArCVActrAccSetIDPtr;
}

bool CDS_HydroMod::CCv::FillCvInfoIntoHMX( CDS_HydroModX *pHMX )
{
	if( NULL == pHMX )
	{
		return false;
	}

	// Test if a valve exist (TAProduct) for the localization.
	if( NULL == GetCvIDPtr().MP /*|| false == GetCvIDPtr().MP->IsExistInTASCOPE()*/ )
	{
		return false;
	}

	// 2016-10-10: Before we saved TBV-C & TBV-CM in a 'CDS_HydroModX::CBV' object and PIBCV in a 'CDS_HydroModX::CCv'.
	//             From now we save all control valve without exceptions in a 'CDS_HydroModX::CCv' object. TA-Scope can
	//             check valve capabilities (as measuring type).

	CDS_HydroModX::CCv *pHMXCV = pHMX->AddCv( GetCvLocate() );

	// Design flow.
	pHMXCV->SetValveID( GetCvIDPtr().ID );
	pHMXCV->SetDesignOpening( GetSetting() );
	pHMXCV->SetCurOpening( GetSetting() );
	pHMXCV->SetDesignFlow( GetQ() );

	// 2017-11-17: For big project it is becoming to big for the 'tsc' file.
	// return FillHySelectExchangeDataToHMX( pHMX );

	return true;
}

void CDS_HydroMod::CCv::SetQ( double dQ, bool fForced )
{
	if( true == fForced || m_dQ != dQ )
	{
		m_dQ = dQ;
		m_pParent->Modified();
	}
}

void CDS_HydroMod::CCv::SetKvsmax( double Kvsmax )
{
	ASSERT( Kvsmax >= 0.0 );

	if( m_dKvsMax != Kvsmax )
	{
		m_dKvsMax = Kvsmax;
		m_pParent->Modified();
	}
}

void CDS_HydroMod::CCv::SetBestKvs( double dKvs, bool bWithRounding )
{
	if (true == bWithRounding)
	{
		if (dKvs < 1.0)
		{
			dKvs *= 100;
			dKvs += .5;
			dKvs = (double)(int)dKvs;
			dKvs /= 100;
		}
		else if (dKvs < 10.0)
		{
			dKvs *= 10;
			dKvs += .5;
			dKvs = (double)(int)dKvs;
			dKvs /= 10;
		}
		else
		{
			dKvs += .5;
			dKvs = (double)(int)dKvs;
		}
	}

	m_dBestKvs = dKvs;
}

void CDS_HydroMod::CCv::SetKvs( double dKvs )
{
	m_dKvs = dKvs;
	m_pParent->Modified();
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceCV, (LPARAM)this );
}

void CDS_HydroMod::CCv::SetCtrlType( CDB_ControlProperties::CvCtrlType CtrlType )
{
	if( m_eControlType != CtrlType )
	{
		m_eControlType = CtrlType;
		CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( GetCvIDPtr().MP );
		CDS_HmHubStation *pHS = dynamic_cast<CDS_HmHubStation *>( m_pParent );

		// Changing control type = reset selected actuator.
		if( NULL != pCV && NULL != pHS && NULL != pHS->GetReturnActuatorIDPtr().MP )
		{
			pHS->SetReturnActuatorID( _T("") );
		}

		m_pParent->Modified();
	}
}

void CDS_HydroMod::CCv::SetCvIDPtr( IDPTR IDPtr )
{
	m_IDPtrCv = IDPtr;

	if( _T('\0') != *m_IDPtrCv.ID && NULL != m_IDPtrCv.DB )
	{
		Extend( &m_IDPtrCv );

		if( true == m_pParent->IsForHub() )
		{
			if( true == m_pParent->IsClass( CLASS( CDS_HmHubStation ) ) )
			{
				if( _T("RVTYPE_BVC") == ( static_cast<CDS_HmHubStation *>( m_pParent ) )->GetBalTypeID() )
				{
					( static_cast<CDS_HmHubStation *>( m_pParent ) )->SetReturnValveID( ( static_cast<CDS_HmHubStation *>( m_pParent ) )->FindHubStaValve(
								CDB_HubStaValv::eSupplyReturnValv::Return,
								m_IDPtrCv ).ID );
				}
			}
		}
	}

	m_pParent->Modified();
	m_bCVActrAccSetRefreshed = false;
}

IDPTR CDS_HydroMod::CCv::GetBestCvIDPtr()
{
	if( _T('\0') != *m_IDPtrBestCV.ID )
	{
		m_IDPtrBestCV.DB = TASApp.GetpTADB();
		Extend( &m_IDPtrBestCV );
		return m_IDPtrBestCV;
	}

	return _NULL_IDPTR;
}

IDPTR CDS_HydroMod::CCv::GetCvIDPtr()
{
	if( _T('\0') != *m_IDPtrCv.ID )
	{
		m_IDPtrCv.DB = TASApp.GetpTADB();
		Extend( &m_IDPtrCv );
		return m_IDPtrCv;
	}

	return _NULL_IDPTR;
}

eBool3 CDS_HydroMod::CCv::IsCVLocInPrimary() 
{ 
	if( NULL == m_pParent->GetpSch() )
	{
		return eb3Undef;
	}

	return ( CvLocation::CvLocPrimSide == m_pParent->GetpSch()->GetCvLoc() ) ? eb3True : eb3False;
}

IDPTR CDS_HydroMod::CCv::GetDiversityIDPtr()
{
	if( _T('\0') != *m_DiversityIDPtr.ID )
	{
		m_DiversityIDPtr.DB = TASApp.GetpTADB();
		Extend( &m_DiversityIDPtr );
		return m_DiversityIDPtr;
	}

	return _NULL_IDPTR;
}

CDB_StringID *CDS_HydroMod::CCv::GetpSelCVType()
{
	CDB_StringID *p = NULL;

	if( _T('\0') != *GetCvIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( static_cast< CDB_TAProduct *>( GetCvIDPtr().MP ) )->GetTypeIDPtr().MP );
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CCv::GetpSelCVConn()
{
	CDB_StringID *p = NULL;

	if( _T('\0') != *GetCvIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( static_cast< CDB_TAProduct *>( GetCvIDPtr().MP ) )->GetConnectIDPtr().MP );
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CCv::GetpSelCVVers()
{
	CDB_StringID *p = NULL;

	if( _T('\0') != *GetCvIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( static_cast< CDB_TAProduct *>( GetCvIDPtr().MP ) )->GetVersionIDPtr().MP );
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CCv::GetpSelCVBdy()
{
	CDB_StringID *p = NULL;

	if( _T('\0') != *GetCvIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( static_cast< CDB_TAProduct *>( GetCvIDPtr().MP ) )->GetBodyMaterialIDPtr().MP );
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CCv::GetpSelCVPN()
{
	CDB_StringID *p = NULL;

	if( _T('\0') != *GetCvIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( static_cast< CDB_TAProduct *>( GetCvIDPtr().MP ) )->GetPNIDPtr().MP );
	}

	return p;
}

void CDS_HydroMod::CCv::ForceCVSelection( IDPTR DeviceIDptr )
{
	SetCvIDPtr( DeviceIDptr );
	SetpSelCV( (LPARAM)DeviceIDptr.MP, true );
	ClearUserChoiceTemporaryVariables();
	m_bCVActrAccSetRefreshed = false;
}

void CDS_HydroMod::CCv::InvalidateSelection()
{
	if (GetpParentHM()->IsLocked(CDS_HydroMod::eHMObj::eCV))
	{
		return;
	}
	SetCvIDPtr( _NULL_IDPTR );
	SetBestCvIDPtr( _NULL_IDPTR );
	m_bCVActrAccSetRefreshed = false;

	// Check now if we can clean also the "m_CBIValveIDPtr" variable. It depends if a previous one exists and is the same kind of the current
	// one. If it's the case we can clean it, otherwise it means the CBI valve is perhaps a regulating valve and we can't delete it here.
	if( _NULL_IDPTR != GetpParentHM()->GetCBIValveIDPtr() )
	{
		if( NULL != dynamic_cast<CDB_ControlValve *>( GetpParentHM()->GetCBIValveIDPtr().MP ) )
		{
			GetpParentHM()->SetCBIValveID( _T("") );
		}
	}
}

void CDS_HydroMod::CCv::SetpSelCV( LPARAM pSelCV, bool fForceLocker )
{
	if( NULL == pSelCV )
	{
		return;
	}

	if( false == fForceLocker )
	{
		if( true == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eCV ) && 0 != *GetCvIDPtr().ID )
		{
			// Valve is locked forget other changes.
			m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceCV, (LPARAM)this );
			return;
		}
	}

	SetCvIDPtr( ( (CData *) pSelCV )->GetIDPtr() );
	// HYS-1443 : Don't need to call selectActuator here because the best CV could change again and 
	// the actuator will be selected in ComputeAll function after all calculations.
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceCV, (LPARAM)this );
	m_pParent->Modified();
	m_bCVActrAccSetRefreshed = false;
}

void CDS_HydroMod::CCv::ForceActuatorSelection( CDS_HydroMod::CCv *pHMCv )
{
	if( NULL == pHMCv || NULL == dynamic_cast<CDB_ControlValve*>( pHMCv->GetCvIDPtr().MP ) )
	{
		return;
	}

	if( false == pHMCv->IsTaCV() )
	{
		return;
	}

	if( m_pParent->GetSchemeIDPtr().MP != pHMCv->GetpParentHM()->GetSchemeIDPtr().MP )
	{
		return;
	}

	// Clean.
	RemoveActuator();

	// Now copy all.
	SetActrIDPtr( pHMCv->GetActrIDPtr() );
	SetActrAdaptIDPtr( pHMCv->GetActrAdaptIDPtr() );
	GetpActrAccIDPtrArray()->Copy( *( pHMCv->GetpActrAccIDPtrArray() ) );
	GetpCVActrAccSetIDPtrArray()->Copy( *( pHMCv->GetpCVActrAccSetIDPtrArray() ) );
	SetActrSelectedSignalIDPtr( pHMCv->GetActrSelectedSignalIDPtr() );
	SetActrSelectedVoltageIDPtr( pHMCv->GetActrSelectedVoltageIDPtr () );
	SetActrSelectedAsaPackage( pHMCv->GetActrSelectedAsaPackage() );
	SetActrSelectionAllowed( pHMCv->GetActrSelectionAllowed() );
}

void CDS_HydroMod::CCv::RemoveActuator()
{
	SetActrIDPtr( _NULL_IDPTR );
	SetActrSelectedSignalIDPtr( _NULL_IDPTR );
	SetActrSelectedVoltageIDPtr( _NULL_IDPTR );
	SetActrAdaptIDPtr( _NULL_IDPTR );
	SetActrSelectedAsaPackage( eb3Undef );
	SetActrAdaptIDPtr( _NULL_IDPTR );
	ClearActrAccIDPtrArray();
	ClearCVActrAccSetIDPtrArray();
}

CDB_Actuator *CDS_HydroMod::CCv::SelectActuator()
{
	CDB_ControlValve *pDBCV = dynamic_cast<CDB_ControlValve *>( GetCvIDPtr().MP );

	if( NULL == pDBCV )
	{
		return NULL;
	}

	IDPTR IDptrPrjParamsActrPowSupply = m_pParent->GetpPrjParam()->GetpHmCalcParams()->GetPrjParamIDPtr( CPrjParams::ActuatorPowerSupplyID );
	IDPTR IDptrPrjParamsActrInpSignal = m_pParent->GetpPrjParam()->GetpHmCalcParams()->GetPrjParamIDPtr( CPrjParams::ActuatorInputSignalID );
	int PrjParamsActrFailSafeFct = m_pParent->GetpPrjParam()->GetpHmCalcParams()->GetActuatorFailSafeFct();
	CDB_ControlValve::DRPFunction PrjParamsDRP = m_pParent->GetpPrjParam()->GetpHmCalcParams()->GetActuatorDRPFct();

	// Actuator can be selected when automatic selection is specified in technical parameters OR when the user select an actuator by himself.
	if( true == m_pParent->GetpPrjParam()->GetpHmCalcParams()->GetActuatorAutomaticallySelect() && eBool3::eb3False != GetActrSelectionAllowed() )
	{
		SetActrSelectionAllowed( eBool3::eb3True );
	}
	else if( NULL != GetActrIDPtr().MP )
	{
		SetActrSelectionAllowed( eBool3::eb3True );
	}
	else if( eBool3::eb3True != GetActrSelectionAllowed() )				// Actuator is NOID but was requested before, keep it as requested
	{
		SetActrSelectionAllowed( eBool3::eb3False );
	}

	// Actuator allowed?
	if( eb3False == GetActrSelectionAllowed() )
	{
		RemoveActuator();
		return NULL;
	}

	bool bPrevChoiceExist = ( NULL != ( GetActrIDPtr().MP ) );
	
	// Actuator selection will be based
	// COC: base list
	// First: Previous selection used parameters
	// Second: Tech data
	std::multimap< double, CDB_Actuator *> ActrFullMap, ActrPackMap;
	std::multimap< double, CDB_Actuator *> *pActrMap = &ActrFullMap;
	
	if( eb3Undef == GetActrSelectedAsaPackage() )
	{
		// HYS-1118: If we don't have the 'm_bActrSelectedAsaPackage' variable set, we set by default to the same value as the one
		// in the technical parameters. BUT: We need to verify once the actuator is found that this one is belonging to a set or not.
		// This is done at the end of this method.
		SetActrSelectedAsaPackage( m_pParent->GetpPrjParam()->GetpHmCalcParams()->GetActuatorSelectedByPackage() == true ? eb3True : eb3False );
	}

	// HYS-1118: We retrieve now the actuators that are compliant with the close-off characteristic of the current control valve.
	// If 'GetActrSelectedAsaPackage()' returns 'true' but the valve is not part of a set, the returned actuators will be the ones
	// that are in the actuators group of the control valve.
	int iCount = GetCOCCompliantActuatorList( &ActrFullMap, m_pParent->GetpCV()->GetActrSelectedAsaPackage() );

	if( eb3True == GetActrSelectedAsaPackage() )
	{
		pDBCV->ExtractPackageCompliantActr( &ActrFullMap, &ActrPackMap );

		if( ActrPackMap.size() > 0 )
		{
			pActrMap = &ActrPackMap;
		}
	}

	// Transfer pActrMap
	CRank rkActrList( false );
	std::multimap< double, CDB_Actuator *>::iterator It;

	for( It = pActrMap->begin(); It != pActrMap->end(); ++It )
	{
		rkActrList.Add( ( It->second )->GetName(), It->first, (LPARAM)It->second );
	}

	CRank rkPowerList( false );
	CRank rkInSigList( false );
	CRank rkFailSafeFct( false );
	CRank rkDRPList( false );
	CRank *prkResultList = &rkActrList;

	if( true == bPrevChoiceExist )
	{
		CDB_ElectroActuator *pElectActr = dynamic_cast<CDB_ElectroActuator *>( GetActrIDPtr().MP );
		CDB_Actuator *Actr = dynamic_cast<CDB_Actuator *>( GetActrIDPtr().MP );

		if( NULL == Actr )
		{
			RemoveActuator();
			return NULL;
		}

		// Try to find power
		int iCount = GetpTADB()->GetActuator( &rkPowerList, prkResultList, GetActrSelectedVoltageIDPtr().ID, L"", -1,
				CDB_ElectroActuator::DefaultReturnPosition::Undefined, CTADatabase::FilterSelection::ForHMCalc );

		if( iCount > 0 )
		{
			prkResultList = &rkPowerList;
			SetActrSelectedVoltageIDPtr( GetActrSelectedVoltageIDPtr() );
		}
		else
		{
			SetActrSelectedVoltageIDPtr( _NULL_IDPTR );
		}

		// On the remaining list try to find Input signal.
		
		iCount = GetpTADB()->GetActuator( &rkInSigList, prkResultList, L"", GetActrSelectedSignalIDPtr().ID, -1,
				CDB_ElectroActuator::DefaultReturnPosition::Undefined, CTADatabase::FilterSelection::ForHMCalc );

		if( iCount > 0 )
		{
			prkResultList = &rkInSigList;
			SetActrSelectedSignalIDPtr( GetActrSelectedSignalIDPtr() );
		}
		else
		{
			SetActrSelectedSignalIDPtr( _NULL_IDPTR );
		}

		if( NULL != pElectActr )
		{
			// On the remaining list try to find fail safe.
			iCount = GetpTADB()->GetActuator( &rkFailSafeFct, prkResultList, L"", L"", (int)( (int)pElectActr->GetFailSafe() > 0) , 
					CDB_ElectroActuator::DefaultReturnPosition::Undefined, CTADatabase::FilterSelection::ForHMCalc );

			if( iCount > 0 )
			{
				prkResultList = &rkFailSafeFct;
			}

			CDB_ElectroActuator::DefaultReturnPosition eDefaultReturnPos = pElectActr->GetDefaultReturnPos();
			iCount = GetpTADB()->GetActuator( &rkDRPList, prkResultList, L"", L"", -1, eDefaultReturnPos, CTADatabase::FilterSelection::ForHMCalc );

			if( iCount > 0 )
			{
				prkResultList = &rkDRPList;
			}
		}
	}
	else
	{
		// Based on Project params.
		// Try to find power.
		int iCount = GetpTADB()->GetActuator( &rkPowerList, prkResultList, IDptrPrjParamsActrPowSupply.ID, L"", -1,
				CDB_ElectroActuator::DefaultReturnPosition::Undefined, CTADatabase::FilterSelection::ForHMCalc );

		if( iCount > 0 )
		{
			prkResultList = &rkPowerList;
			SetActrSelectedVoltageIDPtr( IDptrPrjParamsActrPowSupply );
		}
		else
		{
			SetActrSelectedVoltageIDPtr( _NULL_IDPTR );
		}

		// On the remaining list try to find Input signal
		// When the user select On Off control force Input signal as On/Off
		// HYS-1579: Add 3-points control type
		// HYS-1691: We have to check the actuator's input signal in technical parameter BUT that should
		// be compatible with the control type chosen by the user in the PC1
		iCount = 0;
		IDPTR IDPtrInput_Signal = _NULL_IDPTR;
		CString str2;
		CDB_StringID* pstrID = NULL;

		CRank List;
		// HYS-1691: Get the input signal list and browse it to obtain input signal to show.
		CTable* pActuatorTable = (CTable*)( TASApp.GetpTADB()->Get( _T( "ACTUATOR_TAB" ) ).MP );
		iCount = TASApp.GetpTADB()->GetActuatorInputSignalList( &List, pActuatorTable, m_eControlType,	GetActrSelectedVoltageIDPtr().ID, CTADatabase::ForHMCalc );
		for( BOOL bContinue = List.GetFirstT<CDB_StringID*>( str2, pstrID ); TRUE == bContinue; bContinue = List.GetNextT<CDB_StringID*>( str2, pstrID ) )
		{
			// Try to find the good control type
			int j = 0;
			if( true == pstrID->GetIDstrAs<int>( 0, j ) && j == m_eControlType )
			{
				// If the value in technical parameter is compatible we take it. If the value in TP is not compatible
				// we take the first value compatible with the control type.
				if( pstrID->GetIDPtr() == IDptrPrjParamsActrInpSignal )
				{
					IDPtrInput_Signal = IDptrPrjParamsActrInpSignal;
					break;
				}
				else if( NULL == IDPtrInput_Signal.MP )
				{
					IDPtrInput_Signal = pstrID->GetIDPtr();
				}
			}
		}

		// Now we can get the list of actuators with the selected input signal
		if( iCount > 0 && NULL != IDPtrInput_Signal.MP )
		{
			iCount = GetpTADB()->GetActuator( &rkInSigList, prkResultList, L"", IDPtrInput_Signal.ID, -1,
					CDB_ElectroActuator::DefaultReturnPosition::Undefined, CTADatabase::FilterSelection::ForHMCalc );
		}

		if( iCount > 0 && NULL != IDPtrInput_Signal.MP )
		{
			prkResultList = &rkInSigList;
			SetActrSelectedSignalIDPtr( IDPtrInput_Signal );
		}
		else
		{
			SetActrSelectedSignalIDPtr( _NULL_IDPTR );
		}

		// On the remaining list try to find fail safe.

		iCount = GetpTADB()->GetActuator( &rkFailSafeFct, prkResultList, L"", L"", PrjParamsActrFailSafeFct, 
				CDB_ElectroActuator::DefaultReturnPosition::Undefined, CTADatabase::FilterSelection::ForHMCalc );
		
		if( iCount > 0 )
		{
			prkResultList = &rkFailSafeFct;
		}

		CDB_ElectroActuator::DefaultReturnPosition eActuatorDRP = ( CDB_ElectroActuator::DefaultReturnPosition )pDBCV->GetCompatibleActuatorDRP( PrjParamsDRP );
		iCount = GetpTADB()->GetActuator( &rkDRPList, prkResultList, L"", L"", -1, eActuatorDRP, CTADatabase::FilterSelection::ForHMCalc );

		if( iCount > 0 )
		{
			prkResultList = &rkDRPList;
		}
	}

	if( prkResultList->GetCount() > 0 )
	{
		CString str;
		LPARAM param;
		CDB_ElectroActuator *pElecActr = NULL;
		CDB_Actuator *pActr = NULL;

		// Previous actuator exist select it...
		if( true == bPrevChoiceExist && prkResultList->IfExist( (LPARAM)GetActrIDPtr().MP ) )
		{
			pActr = dynamic_cast<CDB_Actuator *>( GetActrIDPtr().MP );
		}

		if( NULL == pActr )
		{
			CDB_Actuator *pActrSav = NULL;
			
			// Close off char exist, Identify the smallest ones.
			// Otherwise select the biggest ones (KTM).
			CDB_CloseOffChar *pCloseOffChar = dynamic_cast<CDB_CloseOffChar *>( pDBCV->GetCloseOffCharIDPtr().MP );
			double dMaxForce = DBL_MAX;

			if( NULL == pCloseOffChar )
			{
				dMaxForce = DBL_MIN;
			}

			for( BOOL cont = prkResultList->GetFirst( str, param ); TRUE == cont; cont = prkResultList->GetNext( str, param ) )
			{
				pActr = (CDB_Actuator *)param;

				if( ( NULL != pCloseOffChar && dMaxForce > pActr->GetMaxForceTorque() ) 
						|| ( NULL == pCloseOffChar && dMaxForce < pActr->GetMaxForceTorque() ) )
				{
					dMaxForce = pActr->GetMaxForceTorque();
					pActrSav = pActr;
				}
			}

			if( NULL == pActrSav )
			{
				// HYS-1193: The list contains only thermostatic actuator wa take the first
				if( TRUE == prkResultList->GetFirst( str, param ) )
				{
					pActrSav = (CDB_Actuator *)param;
				}
			}

			pActr = pActrSav;
		}

		if( NULL != pActr )
		{
			pElecActr = dynamic_cast<CDB_ElectroActuator *>( pActr );
			SetActrIDPtr( pActr->GetIDPtr() );

			if( ( NULL == GetActrSelectedVoltageIDPtr().MP ) && ( NULL != pElecActr ) )
			{
				if( true == pElecActr->IsPowerSupplyAvailable( IDptrPrjParamsActrPowSupply ) )
				{
					SetActrSelectedVoltageIDPtr( IDptrPrjParamsActrPowSupply );
				}
				else
				{
					SetActrSelectedVoltageIDPtr( pElecActr->GetPowerSupplyIDPtr( 0 ) );
				}
			}

			if( ( NULL == GetActrSelectedSignalIDPtr().MP ) && ( NULL != pElecActr ) )
			{
				if( true == pElecActr->IsInputSignalAvailable( IDptrPrjParamsActrInpSignal ) )
				{
					SetActrSelectedSignalIDPtr( IDptrPrjParamsActrInpSignal );
				}
				else
				{
					SetActrSelectedSignalIDPtr( pElecActr->GetInputSignalsIDPtr( 0 ) );
				}
			}

			// Adapter needed?
			CDB_Product *pAdapt = pDBCV->GetMatchingAdapter( pActr );

			if( NULL != pAdapt && true == pAdapt->IsAnAccessory() )
			{
				SetActrAdaptIDPtr( pAdapt->GetIDPtr() );
			}
			else
			{
				SetActrAdaptIDPtr( _NULL_IDPTR );
			}

			// HYS-1118: We do a verification here to well set the 'm_bActrSelectedAsaPackage' variable.
			if( true == IsTaCV() && NULL != GetpCV() )
			{
				SetActrSelectedAsaPackage( GetCVSelectedAsaPackage() );
			}
		}
		else
		{
			// HYS-1118: We reset the 'm_bActrSelectedAsaPackage' variable.
			SetActrSelectedAsaPackage( eb3Undef );
		}
	}
	else
	{
		RemoveActuator();

		// HYS-1118: We reset the 'm_bActrSelectedAsaPackage' variable.
		SetActrSelectedAsaPackage( eb3Undef );
		return NULL;
	}

	return ( dynamic_cast<CDB_Actuator *>( GetActrIDPtr().MP ) );
}

void CDS_HydroMod::CCv::VerifyActuatorAndAdapter( bool* bActrOK, bool* bAdaptOk )
{
	CDB_ControlValve* pDBCV = dynamic_cast<CDB_ControlValve*>(GetCvIDPtr().MP);

	if( NULL == pDBCV || _T( '\0' ) == GetActrIDPtr().ID )
	{
		return;
	}

	if( NULL != bActrOK )
	{
		// Check if the current actuator in CDS_Hydromod::CCv is always compatible the the valve.
		*bActrOK = true;
		CTable* pActuatorList = (CTable*)(pDBCV->GetActuatorGroupIDPtr().MP);
		if( NULL != pActuatorList )
		{
			IDPTR idptr = pActuatorList->Get( GetActrIDPtr().ID );

			if( _T( '\0' ) == *idptr.ID )
			{
				*bActrOK = false;
			}
		}
	}

	if( NULL != bAdaptOk )
	{
		*bAdaptOk = true;
		// Check if the current actuator adapter in CDS_Hydromod::CCv is always compatible the the valve.
		CRank AdapterList;
		CDB_RuledTable* pclRuledTable = (CDB_RuledTable*)(pDBCV->GetAdapterGroupIDPtr().MP);

		if( NULL != pclRuledTable )
		{
			if( NULL != GetActrAdaptIDPtr().MP )
			{
				GetpTADB()->GetAccessories( &AdapterList, pclRuledTable );
				CDB_Product* pclAdapter = dynamic_cast<CDB_Product*>(GetActrAdaptIDPtr().MP);
				*bAdaptOk = AdapterList.IfExist( pclAdapter->GetName() );
			}
			else
			{
				// We look if we need adapter
				*bAdaptOk = false;
			}
		}
		else if ( NULL == pclRuledTable && NULL != GetActrAdaptIDPtr().MP )
		{
			// We have to remove the adapter
			*bAdaptOk = false;
		}
	}
}

void CDS_HydroMod::CCv::SetpUserChoiceCVConn( CDB_StringID *pSelCVConn )
{
	if( NULL != pSelCVConn && false == pSelCVConn->IsClass( CLASS( CDB_Connect ) ) )
	{
		ASSERT( false );
		pSelCVConn = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceCVConn = pSelCVConn;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceCVConnect, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CCv::SetpUserChoiceCVVers( CDB_StringID *pSelCVVers )
{
	if( NULL != pSelCVVers && false == pSelCVVers->IsClass( CLASS( CDB_StringID ) ) )
	{
		ASSERT( false );
		pSelCVVers = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceCVConn = GetpSelCVConn();
	m_pUserChoiceCVVers = pSelCVVers;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceCVVers, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CCv::SetpUserChoiceCVPN( CDB_StringID *pSelCVPN )
{
	if( NULL != pSelCVPN && false == pSelCVPN->IsClass( CLASS( CDB_StringID ) ) )
	{
		ASSERT( false );
		pSelCVPN = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceCVConn = GetpSelCVConn();
	m_pUserChoiceCVVers = GetpSelCVVers();
	m_pUserChoiceCVPN = pSelCVPN;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceCVPN, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CCv::SetSetting( double dSetting )
{
	m_dSetting = dSetting;

	if( true == IsPresettable() )	// TBV_C/TBV_CM, in this case RegValve doesn't exist, update Circuit Presetting (TACBI compatibility)
	{
		m_pParent->SetPresetting( dSetting );
		m_pParent->SetCBIValveID( GetCvIDPtr().ID );
	}

	if( dSetting != GetSetting() )
	{
		m_pParent->Modified();
	}
}

eBool3 CDS_HydroMod::CCv::IsPICV()
{
	eBool3 bRet = eb3False;
	CDB_CircuitScheme *pSch = m_pParent->GetpSch();

	if( NULL == pSch )
	{
		return eb3Undef;
	}

	if( CDB_CircuitScheme::eBALTYPE::DPC == pSch->GetBalType() && CDB_CircuitScheme::eDpCTypePICV == pSch->GetDpCType() )		// DpC include DpC and PICV
	{
		if( true == IsTaCV() && pSch->GetCvFunc() != CDB_ControlProperties::eCVFUNC::ControlOnly )
		{
			bRet = eb3True;
		}
	}

// For testing
#ifdef DEBUG
	// If CV already defined
	if( NULL != GetCvIDPtr().MP )
	{
		if( NULL != dynamic_cast<CDB_PIControlValve *>( GetCvIDPtr().MP ) )
		{
			ASSERT( bRet );
		}
	}
#endif

	return bRet;
}

eBool3 CDS_HydroMod::CCv::IsDpCBCV()
{
	eBool3 bRet = eb3False;
	CDB_CircuitScheme *pSch = m_pParent->GetpSch();

	if( NULL == pSch )
	{
		return eb3Undef;
	}

	if( CDB_CircuitScheme::eBALTYPE::DPC == pSch->GetBalType() && CDB_CircuitScheme::eDpCTypeDPCBCV == pSch->GetDpCType() )
	{
		bRet = eb3True;
	}

	return bRet;
}

double CDS_HydroMod::CCv::GetSetting()
{
	if( 0 == *GetCvIDPtr().ID || m_dSetting < 0.0 )
	{
		m_dSetting = 0.0;
	}

	return m_dSetting;
}

double CDS_HydroMod::CCv::GetQ()
{
	return m_dQ;
}

double CDS_HydroMod::CCv::GetDp()
{
	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	double dDp = 0.0;

	if( false == IsTaCV() )
	{
		double dRho = pclWaterChar->GetDens();

		if( GetQ() > 0.0 && GetKvs() > 0.0 )
		{
			dDp = CalcDp( m_dQ, m_dKvs, dRho );
		}
	}
	else if( '\0' != *GetCvIDPtr().ID )
	{
		double dRho = pclWaterChar->GetDens();
		double dNu = pclWaterChar->GetKinVisc();
		double dQ = max( 0, GetQ() );

		CDB_PIControlValve *pPICV = dynamic_cast<CDB_PIControlValve *>( GetCvIDPtr().MP );
		CDB_DpCBCValve *pDpCBCValve = dynamic_cast<CDB_DpCBCValve *>( GetCvIDPtr().MP );

		if( NULL != pPICV )
		{
			//dDp = pPICV->GetDp() doesn't exist because the integrated Dp will adapt it self for

			dDp = pPICV->GetDpmin( dQ, dRho );

			if( -1.0 == dDp )
			{
				dDp = 0.0;
			}

			// TODO AL CCv::GetDpMin() PICV DpMin bound?
		}
		else if( NULL != pDpCBCValve )
		{
			CDB_DpCBCVCharacteristic *pclDpCBCValveCharacteristic = pDpCBCValve->GetDpCBCVCharacteristic();

			if( NULL == pclDpCBCValveCharacteristic )
			{
				return 0.0;
			}

			double dTUDp = m_pParent->GetDpOnOutOfPrimSide();
			double dDpk = pclDpCBCValveCharacteristic->GetDpk( dQ );

			double dHAvail = m_pParent->GetHAvail( CAnchorPt::CircuitSide_Primary );
			double dPipeDp = m_pParent->GetpCircuitPrimaryPipe()->GetPipeDp();
			double dSingularDp = m_pParent->GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true );

			if( true == m_pParent->IsaModule() && NULL != m_pParent->GetpSchcat() && false == m_pParent->GetpSchcat()->IsSecondarySideExist() )	  // Circuit pipe doesn't exist for a module
			{
				dPipeDp = dSingularDp = 0.0;
			}

			double dDpSV = 0.0;

			if( true == m_pParent->IsShutoffValveExist( eHMObj::eShutoffValveReturn, true ) )
			{
				dDpSV = m_pParent->GetpShutoffValve( eHMObj::eShutoffValveReturn )->GetDp();
			}

			if( 0.0 == dHAvail )
			{
				// If we don't know yet what is the H available for the circuit we can't determine Dp on the valve with
				// the pressure drop on the Dp controller part (Dpp in regards to Q and Kvm) and the pressure drop on the control part
				// (Dpc that is in fact Dpk - Dpl).
				double dDpp = pDpCBCValve->GetDppmin( dQ, dRho );
				double dDpc = dDpk - dTUDp;
				dDp = dDpp + dDpc;
			}
			else
			{
				dDp = max( dHAvail - ( dTUDp +  dPipeDp + dSingularDp + dDpSV ), GetDpMin() );
			}
		}
		else
		{
			CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( GetCvIDPtr().MP );

			// If curve exists, use it.
			if( NULL != pRv && NULL != pRv->GetValveCharacteristic() && true == pRv->GetValveCharacteristic()->HasKvCurve() && true == IsPresettable() )
			{
				double dH = GetSetting();

				if( false == pRv->GetValveCharacteristic()->GetValveDp( dQ, &dDp, dH, dRho, dNu ) )
				{
					dDp = 0.0;
				}
			}
			else
			{
				// No Curve.
				double dKvs = 0.0;
				dKvs = pRv->GetKvs();

				if( -1.0 != dKvs )
				{
					dDp = CalcDp( dQ, dKvs, dRho );
				}
			}
		}
	}

	return dDp;
}

double CDS_HydroMod::CCv::GetDpMin( bool fBounds )
{
	double dDp = 0.0;

	// Retrieve Dp for current selected valve.
	CDB_CircSchemeCateg *pSchcat = m_pParent->GetpSchcat();
	bool f3wMixing = ( pSchcat != NULL && true == pSchcat->Is3W() && CDB_CircSchemeCateg::e3wTypeMixing == pSchcat->Get3WType() );

	if( false == IsTaCV() || false == IsPresettable() )
	{
		// Case of valve defined by its Kvs or a IMI valve but not presettable.
		if( true == m_pParent->IsLocked( eHMObj::eCV ) )
		{
			dDp = GetDp();
		}
		else
		{
			/** In case of 3 way mixing circuit we have to consider 3kPa as minimum Dp for CV  */
			if( true == IsOn_Off() || true == f3wMixing )
			{
				dDp = m_pParent->GetpTechParam()->GetCVMinDpOnoff();
			}
			else
			{
				dDp = m_pParent->GetTechParamCVMinDpProp();
			}
		}
	}
	else
	{
		// In case of presettable CV, select it as a balancing valve.
		if( NULL != GetCvIDPtr().MP && true == IsPresettable() )
		{
			CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
	
			if( NULL == pclWaterChar )
			{
				ASSERTA_RETURN( 0.0 );
			}

			double dRho = pclWaterChar->GetDens();
			double dNu = pclWaterChar->GetKinVisc();
			double dQ = GetQ();

			CDB_PIControlValve *pPICV = dynamic_cast<CDB_PIControlValve *>( GetCvIDPtr().MP );
			CDB_DpCBCValve *pclDpCBCValve = dynamic_cast<CDB_DpCBCValve *>( GetCvIDPtr().MP );

			if( NULL != pPICV )
			{
				dDp = pPICV->GetDpmin( dQ, dRho );

				if( -1.0 == dDp )
				{
					dDp = 0.0;
				}

				// TODO AL CCv::GetDpMin() PICV DpMin bound?
			}
			else if( NULL != pclDpCBCValve )
			{
				// Max Kv of all the valve (Dp controller + control part).
				double dKvs = pclDpCBCValve->GetKvs();

				dDp = pclDpCBCValve->GetHMin( dQ, dRho );

				// GetHmin() on a DP-BCV returns a value that includes DpL.
				// We have to remove this DpL because we are looking for Dp of valve itself.
				dDp = dDp - m_pParent->GetDpOnOutOfPrimSide();
			}
			else
			{
				CDB_RegulatingValve *pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( GetCvIDPtr().MP );

				if( NULL != pclRegulatingValve && NULL != pclRegulatingValve->GetValveCharacteristic() )
				{
					CDB_ValveCharacteristic *pValveCharacteristic = pclRegulatingValve->GetValveCharacteristic();
					double dH = pValveCharacteristic->GetOpeningMax();

					if( -1.0 != dH )
					{
						if( true == pValveCharacteristic->GetValveDp( dQ, &dDp, dH, dRho, dNu ) )
						{
							double dValvMinDp = 0.0;

							/** In case of 3 way mixing circuit we have to consider 3kPa as minimum Dp for CV  */
							if( true == IsOn_Off() || true == f3wMixing )
							{
								dValvMinDp = m_pParent->GetpTechParam()->GetCVMinDpOnoff();
							}
							else
							{
								dValvMinDp = m_pParent->GetTechParamCVMinDpProp();
							}

							dDp = ( ( true == fBounds ) ? max( dDp, dValvMinDp ) : dDp );
						}
					}
				}
			}
		}
	}

	return dDp;
}

bool CDS_HydroMod::CCv::IsCompletelyDefined()
{
	if( true == m_bTACv )
	{
		if( NULL == GetCvIDPtr().MP )				// safe IDPtr, checked by Extend in GetIDPtr()
		{
			return false;
		}

		if( NULL != GetpCV() && NULL != GetpCV()->GetCtrlProp() &&
			( CDB_ControlProperties::Presettable == GetpCV()->GetCtrlProp()->GetCvFunc() ||
			  CDB_ControlProperties::PresetPT == GetpCV()->GetCtrlProp()->GetCvFunc() ) )
		{
			if( 0.0 == GetSetting() )
			{
				return false;
			}
		}

		if( 0.0 == GetDp() )
		{
			return false;
		}

		if( 0.0 == GetAuth() )
		{
			return false;
		}
	}
	else
	{
		if( GetKvs() <= 0.0 )
		{
			return false;
		}
	}

	return true;
}

// Resize is used with presettable CV
void CDS_HydroMod::CCv::ResizePresetCV( double dHAvail, bool bTryToFindSmallestValve )
{
	m_pParent->EnableComputeHM( false );

	// Doesn't change Kvs if valve is locked.
	double dKvs = 0.0;
	double dQ = GetQ();
	double dDp = dHAvail;

	CDB_CircSchemeCateg *pSchcat = m_pParent->GetpSchcat();
	bool b3wMixing = ( pSchcat != NULL && true == pSchcat->Is3W() && CDB_CircSchemeCateg::e3wTypeMixing == pSchcat->Get3WType() );

	// A valve with presetting must be resized as a balancing Valve.
	if( true == IsTaCV() )
	{
		CDB_RegulatingValve *pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( GetCvIDPtr().MP );

		if( NULL == pclRegulatingValve )
		{
			// Valve doesn't exist yet.
			m_pParent->EnableComputeHM( true );
			return;
		}

		CDB_ValveCharacteristic *pValveCharacteristic = pclRegulatingValve->GetValveCharacteristic();

		if( NULL == pValveCharacteristic )
		{
			// No characteristic (normally must never happen).
			m_pParent->EnableComputeHM( true );
			return;
		}

		if( true == IsPresettable() )
		{
			if( true == GetCvIDPtr().MP->IsClass( CLASS( CDB_ControlValve ) ) || true == GetCvIDPtr().MP->IsClass( CLASS( CDB_ThermostaticValve ) ) )
			{
				/** In case of 3 way mixing circuit we have to consider 3kPa as minimum Dp for CV  */
				if( true == IsOn_Off() || true == b3wMixing )
				{
					dDp = max( m_pParent->GetpTechParam()->GetCVMinDpOnoff(), dDp );
				}
				else
				{
					dDp = max( dDp, m_pParent->GetTechParamCVMinDpProp() );
				}

				// Take in count round problems.
				dDp *= 1.000001;

				CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
	
				if( NULL == pclWaterChar )
				{
					ASSERT_RETURN;
				}

				// Close the valve to take the overflow in pressure.
				double dRho = pclWaterChar->GetDens();
				double dNu = pclWaterChar->GetKinVisc();
				double dH = 0.0;

				if( false == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eCV ) && true == bTryToFindSmallestValve )
				{
					// Try to find a smallest valve.
					double dKvs = 0.0;
					double dQ = GetQ();												//ASSERT( dQMax );

					if( 0.0 != dQ )
					{
						// If current Kvs is larger than new computed Kvs, try to resize the valve.
						// Kvs larger means less resistance and thus bigger valve.
						dKvs = CalcKv( dQ, dDp, dRho );

						if( dKvs < GetKvs() )
						{
							_SizeKvs( dDp, true );
						}
					}
				}

				// Valve could be changed (smallest size)
				pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( GetCvIDPtr().MP );

				if( NULL == pclRegulatingValve )
				{
					// Valve doesn't exist yet.
					m_pParent->EnableComputeHM( true );
					return;
				}

				pValveCharacteristic = pclRegulatingValve->GetValveCharacteristic();

				if( NULL == pValveCharacteristic )
				{
					// No characteristic (normally must never happen).
					m_pParent->EnableComputeHM( true );
					return;
				}

				int iRounding = ( eBool3::eb3True == pValveCharacteristic->IsDiscrete() ) ? 2 : 1;
				bool bValid = pValveCharacteristic->GetValveOpening( dQ, dDp, &dH, dRho, dNu, iRounding );

				if( false == bValid )
				{
					// Dp at full opening if greater than requested Dp.
					// Ask Dp at full opening.
					dH = pValveCharacteristic->GetOpeningMax();
					double dRequestedDp = dDp;

					if( -1.0 != dH )
					{
						bValid = pValveCharacteristic->GetValveDp( dQ, &dDp, dH, dRho, dNu );
					}

					// Valve cannot reach requested DP TA-FUSION-C 32 @ q = 3000l/h RequestedDp = 370 kPa
					if( dDp < dRequestedDp )
					{
						bValid = false;
					}
				}
				else
				{
					// Recalculate the Dp for the setting as determined above.
					if( eBool3::eb3True == pValveCharacteristic->IsDiscrete() )
					{
						bValid = pValveCharacteristic->GetValveDp( dQ, &dDp, dH, dRho, dNu );
					}
				}

				if( true == bValid )
				{
					SetSetting( dH );
				}
				else
				{
					// TODO: CCv::ResizePresetCV resizing fails log message?
					InvalidateSelection();
				}
			}
		}
	}
	else	/*Not a TA CV*/
	{
		// Only used to resize CV to a Dp min
		SetQ( GetQ(), true );
	}

	m_pParent->EnableComputeHM( true );
}

double CDS_HydroMod::CCv::GetAuth( bool bDesign )
{
	CDB_CircSchemeCateg *pSchcat = m_pParent->GetpSchcat();

	if( NULL == pSchcat )
	{
		ASSERTA_RETURN( 0.0 );
	}
	
	double dCvDp = 0.0;

	// TODO: AL CCv::GetAuth() authority for PICV
	if( eb3True == IsPICV() || eb3True == IsDpCBCV() )
	{
		return 1;
	}

	// In case of pre-settable CV TBV-CM, Cv Dp that should be considered is the Dp at full opening.
	dCvDp = GetDp();

	if( true == IsPresettable() )
	{
		CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( GetCvIDPtr().MP );

		if( NULL != pRv && CDB_ControlProperties::eBCVType::ebcvtCharIndep != pRv->GetCtrlProp()->GetBCVType()
				&& CDB_ControlProperties::eBCVType::ebcvtLiftLimit != pRv->GetCtrlProp()->GetBCVType() )
		{
			CDB_ValveCharacteristic *pValveCharacteristic = pRv->GetValveCharacteristic();

			if( NULL != pValveCharacteristic )
			{
				CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
	
				if( NULL == pclWaterChar )
				{
					ASSERTA_RETURN( 0.0 );
				}

				double dRho = pclWaterChar->GetDens();
				double dNu = pclWaterChar->GetKinVisc();
				// Return -1.0 if error.
				dCvDp = pValveCharacteristic->GetDpFullOpening( GetQ(), dRho, dNu );
			}
		}
	}

	if( dCvDp <= 0.0 )
	{
		return 0.0;
	}

	if( NULL != pSchcat && true == pSchcat->Is3W() && CDB_CircSchemeCateg::e3wTypeDividing == pSchcat->Get3WType() )
	{
		/// Auth = DpCV/(DpCV + DpTU)
		double dDp = m_pParent->GetDpOnOutOfPrimSide() + dCvDp;

		if( dDp > 0.0 )
		{
			SetAuth( max( 0, dCvDp / dDp ) );
		}

		return min( m_dAuthority, 0.99 );
	}

	if( true == m_pParent->IsDpCExist( true ) && eb3True == IsCVLocInPrimary() )
	{
		CDB_DpController *pDpC = static_cast<CDB_DpController *>( m_pParent->GetpDpC()->GetIDPtr().MP );

		if( NULL == pDpC || NULL == pDpC->GetDpCCharacteristic() )
		{
			return 0.0;
		}

		if( NULL != pSchcat && true == pSchcat->Is3W() )
		{
			if( CDB_CircSchemeCateg::e3wTypeMixing != pSchcat->Get3WType() )
			{
				// In case of 3-way mixing circuit with decoupling bypass the 'GetDpOnOutOfPrimSide' method will return Dp for Bypass ~= 0.
				double dDp = m_pParent->GetDpOnOutOfPrimSide() + dCvDp;

				if( dDp > 0.0 )
				{
					SetAuth( max( 0, dCvDp / dDp ) );
				}
			}
			else
			{
				// In case of simple 3-way mixing, the authority must be near 0.5.
				double dDpStab = m_pParent->GetpDpC()->GetDpL();
				SetAuth( max( 0.0, dCvDp / ( dCvDp + dDpStab ) ) );
			}
		}
		else
		{
			double dBeta = 0;

			// Bdesign = DpCV (fo@Qd) / Dpstab
			// With bypass BV Bmin will be slightly better w/o bypass Bv
			// w/o bypass BV q will increase stabilized Dpl will decrease (PB) Bmin will increase
			double dDpStab = m_pParent->GetpDpC()->GetDpL();

			if( false == bDesign )
			{
				dDpStab = pDpC->GetDpStabCorrectedWithPBfactor( dDpStab );
			}

			dBeta = dCvDp / ( dDpStab );
			SetAuth( max( 0.0, dBeta ) );
		}
	}
	else
	{
		bool b3wInjMixing = ( pSchcat != NULL && true == pSchcat->Is3W() && CDB_CircSchemeCateg::e3wTypeMixing == pSchcat->Get3WType() );
		CDS_HydroMod *pPIHm = NULL;

		if( eb3True == IsCVLocInPrimary() || true == b3wInjMixing )
		{
			pPIHm = m_pParent->GetpPressIntHM();
		}
		else
		{
			pPIHm = m_pParent;
		}

		double dDp = 0.0;
		CTableHM *pPipingTab = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
		bool bConstantFlow = pPIHm->IsaModule() ? pPIHm->IsConstantFlow() : pPipingTab->CheckIsConstantFlow( pPIHm );

		if( true == b3wInjMixing )
		{
			dDp = m_pParent->GetHAvail() + dCvDp;

			if( dDp > 0.0 )
			{
				SetAuth( max( 0.0, dCvDp / dDp ) );
			}
		}
		else if( NULL != pSchcat && pSchcat->Get3WType() == CDB_CircSchemeCateg::e3wTypeMixDecByp )
		{
			if( eb3False == IsCVLocInPrimary() )
			{
				// As described in page 200, 226 and summarized in the array page 234 in the "Total hydronic balancing" book from
				// Robert Petitjean, the control valve for a 3-way mixing with decoupling bypass has always authority near 1.
				// Near 1 because we have a little pressure drop in the bypass pipe.
				SetAuth( 1.0 );
			}
			else
			{
				// For 3-way mixing circuit with decoupling bypass and control valve at the primary side, the authority will depend
				// of the variability of the flow distribution (See "Hydronic Control circuits_Part 3_EN_V01.pptx" page 8).

				if( 0 == m_pParent->GetLevel() )
				{
					// If the module is a root, we consider by default that the flow distribution is constant.
					// And in this case the authority is 1.
					SetAuth( 1.0 );
				}
				else
				{
					// We need first to retrieve the pressure interface to check what is the variability of the flow.

					if( true == pPIHm->IsConstantFlow() )
					{
						SetAuth( 1.0 );
					}
					else
					{
						// The formulas used come from the "Hydronic Control circuit Part 3" from Bjarne Andreasen.
						ASSERT( NULL != m_pParent->GetpBv() && m_pParent->GetpBv()->GetDp() > 0.0 );

						double dAuthority = 0.0;
						double HAvailLocaly = m_pParent->GetHAvail( CAnchorPt::CircuitSide_Primary );

						if( true == bDesign )
						{
							// Authority is = DpCV / ( Havailable - DpBVp ).
							dAuthority = dCvDp / ( HAvailLocaly - m_pParent->GetpBv()->GetDp() );
						}
						else
						{
							// Authority is = DpCV / ( H@PI * ( 1 - ( DpBVp / Havailable ) ).
							double dHPressureInterface = 0.0;

							if( pPIHm == this->GetpParentHM() )
							{
								// We are on the pressure interface, we take the H available at the primary side.
								dHPressureInterface = pPIHm->GetHAvail( CAnchorPt::CircuitSide_Primary );
							}
							else
							{
								dHPressureInterface = pPIHm->GetHAvail( CAnchorPt::CircuitSide_Secondary );
							}

							dAuthority = m_pParent->GetpBv()->GetDp() / HAvailLocaly;
							dAuthority = dHPressureInterface * ( 1.0 - dAuthority );
							dAuthority = dCvDp / dAuthority;
						}

						SetAuth( dAuthority );
					}
				}
			}
		}
		else
		{
			double HavailLocaly = m_pParent->GetHAvail( eb3True == IsCVLocInPrimary() ? CAnchorPt::CircuitSide_Primary : CAnchorPt::CircuitSide_Secondary );

			if( true == bDesign && HavailLocaly > 0.0 )
			{
				SetAuth( max( 0, dCvDp / HavailLocaly ) );
			}
			else
			{

				if( eb3True == IsCVLocInPrimary() &&
					( NULL != pPIHm->GetpSchcat() && false == pPIHm->GetpSchcat()->IsSecondarySideExist() && true == pPIHm->IsDpCExist( true )
						&& CDS_HydroMod::ePressIntType::pitDpC == pPIHm->GetPressIntType() ) )
				{
					CDB_DpController *pDpC = static_cast< CDB_DpController * >( pPIHm->GetpDpC()->GetIDPtr().MP );

					if( NULL == pDpC || NULL == pDpC->GetDpCCharacteristic() )
					{
						return 0.0;
					}

					// Bdesign = DpCV (fo@Qd) / Dpstab
					// With bypass BV Bmin will be slightly better w/o bypass Bv
					// w/o bypass BV q will increase stabilized Dpl will decrease (PB) Bmin will increase
					dDp =  pPIHm->GetpDpC()->GetDpL();

					if( false == bDesign )
					{
						dDp = pDpC->GetDpStabCorrectedWithPBfactor( dDp );
					}
				}
				else
				{
					if( pPIHm == this->GetpParentHM() )
					{
						// We are on the pressure interface, consider CV localization.
						dDp = pPIHm->GetHAvail( ( eb3True == IsCVLocInPrimary() ) ? CAnchorPt::CircuitSide_Primary : CAnchorPt::CircuitSide_Secondary );
					}
					else
					{
						dDp = pPIHm->GetHAvail( CAnchorPt::CircuitSide_Secondary );
					}
				}

				if( dDp > 0.0 )
				{
					SetAuth( max( 0, dCvDp / dDp ) );
				}
			}
		}
	}

	// ASSERT( m_dAuthority <= 1.01 );
	return min( m_dAuthority, 0.99 );
}

// Find Control valve for a given Kvs
struct sCv
{
	CDB_TAProduct *pTap;
	double dVal;
};

double CDS_HydroMod::CCv::SelectBestPICV( double dHAvail, std::multimap<double, CDB_PIControlValve *> *pMap )
{
	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	// Key is the sorting Key.
	sCv PICV;
	std::multimap<double, sCv>mapPICVEqualPercentage;
	std::multimap<double, sCv>mapPICVOther;
	std::multimap<double, sCv> &mapPICV = mapPICVOther;
	std::multimap<double, sCv>::iterator mapIt;

	// HYS-1568 : Sorted map for proportional control to manage linear characteristic
	std::multimap<double, sCv>sortedMap;

	m_mapCV.clear();

	// Main loop scan all available PICV.
	// When Havail == 0.0 keep valve with  minimum Fc, to minimize H needed.
	// When Havail > 0.0 keep valve with maximum Fc below Havail to maximalize Authority.
	CTable *pTab = dynamic_cast<CTable *>( TASApp.GetpTADB()->Get( _T("PICTRLVALV_TAB") ).MP );

	if( NULL == pTab )
	{
		return 0.0;
	}

	// HYS-1221.
	bool bOldProductsAvailableForHC = true;

	// HYS-1330 : No set for these valves.
	SetCVSelectedAsaPackage( eb3False );

	for( IDPTR IDPtr = pTab->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_PIControlValve *pPICV = dynamic_cast<CDB_PIControlValve *>( IDPtr.MP );
		ASSERT( NULL != pPICV );

		if( NULL == pPICV )
		{
			continue;
		}

		if( false == pPICV->IsSelectable( true, false, bOldProductsAvailableForHC ) )
		{
			continue;
		}

		CDB_ControlProperties *pCtrlProp =  dynamic_cast<CDB_ControlProperties *>( pPICV->GetCtrlProp() );

		if( NULL == pCtrlProp )
		{
			continue;
		}

		if( GetQ() > 0.0 )
		{
			double dPreset = pPICV->GetPresetting( GetQ(), pclWaterChar->GetDens(), pclWaterChar->GetKinVisc() );

			if( dPreset <= 0.0 )
			{
				continue;
			}
		}

		// Verify Cv Func.
		// Test Cv2W3W, CvFunc.
		if( NULL != m_pParent->GetpSchcat() && NULL != m_pParent->GetpSch()
					&& false == pPICV->IsValidCV( m_pParent->GetpSchcat()->Get2W3W(), m_pParent->GetpSch()->GetCvFunc() ) )
		{
			continue;
		}

		// Check if control mode is compatible with the valve characteristic.
		if( false == pCtrlProp->CvCtrlTypeFits( m_pParent->GetCvCtrlType() ) )
		{
			continue;
		}

		// Check max flow.
		if( -1.0 == pPICV->GetQmax() || GetQ() > pPICV->GetQmax() )
		{
			continue;
		}

		// Check DpMax.
		if( dHAvail > pPICV->GetDpmax() )
		{
			continue;
		}

		// TODO AL: CCv::SizePICV what about pTech Min.Dp for fully open control part
		// Add product into the list
		PICV.pTap = pPICV;
		double dDpMin = pPICV->GetDpmin( GetQ(), pclWaterChar->GetDens() );

		if( -1.0 == dDpMin )
		{
			dDpMin = 0.0;
		}

		PICV.dVal = dDpMin;
		double d64Key = pPICV->GetSortingKey();

		// HYS-663
		// % requested by the user and the valve is proportional
		// add it in the proportional map
		if( CDB_ControlProperties::eCvProportional == m_pParent->GetCvCtrlType()
				&& CDB_ControlProperties::EqualPc == pCtrlProp->GetCvCtrlChar() )
		{
			mapPICVEqualPercentage.insert( std::pair<double, sCv>( d64Key, PICV ) );
		}
		else if( true == pCtrlProp->CvCtrlTypeFits( m_pParent->GetCvCtrlType() ) )
		{
			mapPICVOther.insert( std::pair<double, sCv>( d64Key, PICV ) );
		}
	}

	// By default pmapPICV is referencing mapPICVOther, if we are working with EqualPc and if some valve exist use it.
	if( 0 != mapPICVEqualPercentage.size() )
	{
		// HYS-1568 : Sorted map for proportional control to manage linear characteristic.
		// Create and fill the sorted map :
		int iNbElem = mapPICVEqualPercentage.size() + mapPICVOther.size();
		std::multimap<double, sCv>::iterator ItEqualPercentage = mapPICVEqualPercentage.begin();
		std::multimap<double, sCv>::iterator ItOther = mapPICVOther.begin();

		for( int i = 0; i < iNbElem; i++ )
		{
			if( ItEqualPercentage != mapPICVEqualPercentage.end() && ItOther != mapPICVOther.end() )
			{
				CDB_PIControlValve *pclEQMValve = (CDB_PIControlValve *)ItEqualPercentage->second.pTap;
				CDB_PIControlValve *pclOtherValve = (CDB_PIControlValve *)ItOther->second.pTap;

				if( pclEQMValve->GetSizeKey() <= pclOtherValve->GetSizeKey() )
				{
					sortedMap.insert( std::pair<int, sCv>( i, ItEqualPercentage->second ) );
					ItEqualPercentage++;
				}
				else
				{
					sortedMap.insert( std::pair<int, sCv>( i, ItOther->second ) );
					ItOther++;
				}
			}
			else if( ItOther != mapPICVOther.end() )
			{
				sortedMap.insert( std::pair<int, sCv>( i, ItOther->second ) );
				ItOther++;
			}
			else if( ItEqualPercentage != mapPICVEqualPercentage.end() )
			{
				sortedMap.insert( std::pair<int, sCv>( i, ItEqualPercentage->second ) );
				ItEqualPercentage++;
			}
		}

		mapPICV = sortedMap;
	}

	//--------------------------------------------
	// Verify Pipe Size shift
	//--------------------------------------------
	int iHighestSize;
	int iLowestSize;

	CAnchorPt::PipeLocation ePipeLocation = m_pParent->GetpSch()->GetAnchorPtPipeLocation( CAnchorPt::eFunc::PICV );
	CDB_Pipe *pPipe = m_pParent->GetPipeSizeShift( iHighestSize, iLowestSize, CAnchorPt::CircuitSide_Primary, ePipeLocation );

	if( NULL == pPipe )
	{
		return 0.0;
	}

	CTable *pDNTab = m_pParent->m_pclTableDN;
	ASSERT( pDNTab );
	bool bOneValveExist = false;
	int iSmallestValve = 0;
	int iHighestValve = 99999;

	// Adapt iLowestSize and iHigestSize to include at least one valve
	for( mapIt = mapPICVOther.begin(); mapIt != mapPICVOther.end() && bOneValveExist == false; ++mapIt )
	{
		int iProdSizeKey = mapIt->second.pTap->GetSizeKey();

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

	for( mapIt = mapPICV.begin(); mapIt != mapPICV.end(); )
	{
		int iProdSizeKey = mapIt->second.pTap->GetSizeKey();

		if( !( iProdSizeKey >= iLowestSize && iProdSizeKey <= iHighestSize ) )
		{
			if( mapIt == mapPICV.begin() )
			{
				// First proposition is rejected
				// TODO: warning message in compilation Box
			}

			// Remove this selection.
			if( mapPICV.size() > 1 )
			{
				std::multimap<double, sCv>::iterator it = mapIt;
				++mapIt;
				mapPICV.erase( it );
				continue;
			}
		}
		else
		{
			bOneValveExist = true;
		}

		CDB_PIControlValve *pclPIControlValve = ( CDB_PIControlValve * )mapIt->second.pTap;

		// Take a copy of map if requested.
		if( NULL != pMap )
		{
			pMap->insert( std::pair<double, CDB_PIControlValve *> ( pclPIControlValve->GetSortingKey(), pclPIControlValve ) );
		}

		m_mapCV.insert( std::make_pair( pclPIControlValve->GetSizeKey(), CCVData( pclPIControlValve ) ) );

		++mapIt;
	}

	// When pMap exist stop selection process here.
	// Function is only used to have a list a matching PICV.
	if( NULL != pMap )
	{
		return 0.0;
	}

	int iDN15 = m_pParent->GetDN15();
	int iDN50 = m_pParent->GetDN50();
	int iDN = pPipe->GetSizeKey( TASApp.GetpTADB() );

	CDB_StringID *psidType = NULL;
	CDB_StringID *psidFam = NULL;
	CDB_StringID *psidBdy = NULL;
	CDB_StringID *psidConn = NULL;
	CDB_StringID *psidVers = NULL;
	CDB_StringID *psidPN = NULL;
	std::multimap<double, sCv>::iterator ItBest = mapPICV.end();

	// Apply User preferences.
	bool fEndLoop = false;

	for( int i = 0; i < 2 && false == fEndLoop; i++ )
	{
		switch( i )
		{
			// If a selected device exist try to find a device with same parameters (if we are in the same side of DN50 border).
			case 0:
				if( NULL != GetCvIDPtr().MP )
				{
					CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( GetCvIDPtr().MP );

					if( NULL != pTAP )
					{
						psidType = dynamic_cast<CDB_StringID *>( pTAP->GetTypeIDPtr().MP );
						psidFam	= dynamic_cast<CDB_StringID *>( pTAP->GetFamilyIDPtr().MP );
						psidBdy = dynamic_cast<CDB_StringID *>( pTAP->GetBodyMaterialIDPtr().MP );
						psidConn = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );
						psidVers = dynamic_cast<CDB_StringID *>( pTAP->GetVersionIDPtr().MP );
						psidPN = dynamic_cast<CDB_StringID *>( pTAP->GetPNIDPtr().MP );

						// If user force a new connection...
						if( NULL != m_pUserChoiceCVConn && m_pUserChoiceCVConn != psidConn )
						{
							psidType = NULL;
							psidBdy = NULL;
							psidFam = NULL;
							psidConn = m_pUserChoiceCVConn;
							psidVers = NULL;
							psidPN = NULL;
						}

						// If user force a new version...
						if( NULL != m_pUserChoiceCVVers && m_pUserChoiceCVVers != psidVers )
						{
							psidType = NULL;
							psidBdy = NULL;
							psidFam = NULL;
							psidVers = m_pUserChoiceCVVers;
							psidPN = NULL;
						}

						// If user force a new PN...
						if( NULL != m_pUserChoiceCVPN && m_pUserChoiceCVPN != psidPN )
						{
							psidType = NULL;
							psidBdy = NULL;
							psidFam = NULL;
							psidPN = m_pUserChoiceCVPN;
						}

						// Clear all specific user request.
						m_pUserChoiceCVConn = NULL;
						m_pUserChoiceCVVers = NULL;
						m_pUserChoiceCVPN = NULL;
						fEndLoop = true;
					}
				}

				break;

			case 1:
				// Default preferences from tech params.
			{
				CPrjParams *pPrjParams = m_pParent->GetpPrjParam()->GetpHmCalcParams();

				if( iDN <= iDN50 )
				{
					psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvBlw65TypeID ).MP );
					psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvBlw65FamilyID ).MP );
					psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvBlw65BdyMatID ).MP );
					psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvBlw65ConnectID ).MP );
					psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvBlw65VersionID ).MP );
					psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvBlw65PNID ).MP );
				}
				else
				{
					psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvAbv50TypeID ).MP );
					psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvAbv50FamilyID ).MP );
					psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvAbv50BdyMatID ).MP );
					psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvAbv50ConnectID ).MP );
					psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvAbv50VersionID ).MP );
					psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvAbv50PNID ).MP );
				}

				fEndLoop = true;
			}
			break;

		}
	}

	if( 0 == mapPICV.size() )
	{
		// NOT FOUND !!!!
		ASSERTA_RETURN( 0.0 );
	}

	CTAPSortKey sKey( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );
	int iRet = 0;

	// By default Best PICV is the first ones.
	ItBest = mapPICV.end();
	double dBestRelPreSet = 0.0;
	double dBestDpMin = DBL_MAX;
	double dAcceptedExtraDp = m_pParent->GetpTechParam()->GetPICVExtraThresholdDp();
	// Don't take care of ePriority.
	UINT uiMask = CTAPSortKey::TAPSKM_All & ~CTAPSortKey::TAPSKM_PriorL;

	// Mask empty fields.
	uiMask = uiMask & ~CTAPSortKey::TAPSKM_Size;

	if( NULL == psidType )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Type;
	}

	if( NULL == psidFam )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Fam;
	}

	if( NULL == psidBdy )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Bdy;
	}

	if( NULL == psidConn )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Connect;
	}

	if( NULL == psidVers )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Version;
	}

	if( NULL == psidPN )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_PN;
	}

	CDB_PIControlValve *pclBestPICV = NULL;

	for( mapIt = mapPICV.begin(); mapIt != mapPICV.end(); ++mapIt )
	{
		CTAPSortKey mKey( ( CDB_TAProduct * )mapIt->second.pTap );

		iRet = mKey.Compare( &sKey, &CTAPSortKey::CTAPSKMask( ( CTAPSortKey::TAPSortKeyMask )uiMask ) );

		if( 0 == iRet )			//match
		{
			CDB_PIControlValve *pclPIControlValve = dynamic_cast<CDB_PIControlValve *>( mapIt->second.pTap );

			if( NULL == pclPIControlValve )
			{
				continue;
			}

			// Keep smallest Dpmin (FC) when Havail <= 0 / largest Dpmin below Havail when Havail >= 0.
			bool bNewBestExist = false;
			double dPresetting = pclPIControlValve->GetPresetting( GetQ(), pclWaterChar->GetDens(), pclWaterChar->GetKinVisc() );
			double dMaxSetting = pclPIControlValve->GetMaxSetting();

			double dPresetRatio = 0.0;

			// HYS-1915: Calculate dPresetRatio each time to avoid setting dBestRelPreSet with nul value.
			dPresetRatio = dPresetting / dMaxSetting;
			if( ItBest == mapPICV.end() )
			{
				bNewBestExist = true;
			}
			else
			{
				// Specific treatment for TBV-CMP 3/4"HF and 1/2"HF same body as 25NF!
				// Assume analyzing object in decreasing size order
				bool bValveSharingSameBody = false;

				if( NULL != pclBestPICV
						&& 0 == IDcmp( pclBestPICV->GetFamilyID(), _T("FAM_TBV-CMP") )
						&& 0 == IDcmp( pclPIControlValve->GetFamilyID(), _T("FAM_TBV-CMP") ) )
				{
					bValveSharingSameBody = ( pclPIControlValve->GetPICVCharacteristic() == pclBestPICV->GetPICVCharacteristic() );
					bValveSharingSameBody |= ( dPresetting < pclPIControlValve->GetPICVCharacteristic()->GetMinRecSetting() );
				}

				int iBestSize = ItBest->second.pTap->GetSizeKey();
				int iSize = mapIt->second.pTap->GetSizeKey();

				// New valve is largest than best valve but his Dp(with extra Dp) is lower.
				// New valve is smallest than the best valve and his minDp is lower.
				if( iSize == iBestSize )
				{
					if( mapIt->second.dVal < ( dBestDpMin + dAcceptedExtraDp ) )
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
				else if( ( iSize < iBestSize && mapIt->second.dVal < ( dBestDpMin + dAcceptedExtraDp ) && false == bValveSharingSameBody )
						|| ( iSize > iBestSize && ( mapIt->second.dVal + dAcceptedExtraDp ) < dBestDpMin) )
				{
					bNewBestExist = true;
				}
			}

			if( true == bNewBestExist )
			{
				dBestDpMin = mapIt->second.dVal;
				ItBest = mapIt;
				pclBestPICV = pclPIControlValve;
				dBestRelPreSet = dPresetRatio;
			}
		}
	}

	// PICV not found take the first ones by default.
	if( ItBest == mapPICV.end() )
	{
		ItBest = mapPICV.begin();
	}

	SetBestCvIDPtr( ItBest->second.pTap->GetIDPtr() );
	return ItBest->second.dVal;
}

double CDS_HydroMod::CCv::SelectBestDpCBCV( double dHAvail, std::multimap<double, CDB_DpCBCValve *> *pMap )
{
	// Key is the sorting key.
	std::multimap<double, CCVData>mapDpCBCValve;
	std::multimap<double, CCVData>::iterator mapIt;

	m_mapCV.clear();

	// Main loop scan all available DpCBCV.
	// When Havail == 0.0 keep valve with minimum Fc, to minimize H needed.
	// When Havail > 0.0 keep valve with maximum Fc below Havail to maximalize Authority.
	CTable *pTab = dynamic_cast<CTable *>( TASApp.GetpTADB()->Get( _T("DPCBALCTRLVALV_TAB") ).MP );

	if( NULL == pTab )
	{
		ASSERTA_RETURN( -1.0 );
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( -1.0 );
	}

	double dRho = pclWaterChar->GetDens();
	
	// HYS-1330 : No set for these valves.
	SetCVSelectedAsaPackage( eb3False );

	// HYS-1221.
	bool bOldProductsAvailableForHC = true; //( NULL != m_pParent->GetpPrjParam() ) ? m_pParent->GetpPrjParam()->GetpHmCalcParams()->IsOldProductsAvailableForHC() : false;

	for( IDPTR IDPtr = pTab->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_DpCBCValve *pclDpCBCValve = dynamic_cast<CDB_DpCBCValve *>( IDPtr.MP );
		ASSERT( NULL != pclDpCBCValve );

		if( NULL == pclDpCBCValve || false == pclDpCBCValve->IsSelectable( true, false, bOldProductsAvailableForHC ) )
		{
			continue;
		}

		CDB_DpCBCVCharacteristic *pclDpCBCVCharacteristic = pclDpCBCValve->GetDpCBCVCharacteristic();
		ASSERT( NULL != pclDpCBCVCharacteristic );

		if( NULL == pclDpCBCVCharacteristic )
		{
			continue;
		}

		double dQMax = pclDpCBCVCharacteristic->GetQmax();

		if( m_dQ > dQMax )
		{
			continue;
		}

		double dDplmin = pclDpCBCValve->GetDplmin();
		double dDplmax = pclDpCBCValve->GetDplmax( m_dQ, dRho );
		double dDpStabilize = -1.0;

		// If we are going to the pump, we have not yet the H available for the circuit. We check thus if each valve can
		// at least stabilize the Dp on the branch.
		// Remark: At least if Dpl is already input!!!
		dDpStabilize = m_pParent->GetDpOnOutOfPrimSide();

		if( dDpStabilize > 0.0 && ( dDpStabilize < dDplmin || dDpStabilize > dDplmax ) )
		{
			continue;
		}

		double dShutoffValveDpFO = 0.0;

		// ShutoffValve is not.
		if( true == m_pParent->IsShutoffValveExist( eHMObj::eShutoffValveReturn, true ) )
		{
			CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pParent->GetpShutoffValve( eHMObj::eShutoffValveReturn );

			if( NULL != pclHMShutoffValve && NULL != pclHMShutoffValve->GetBestIDPtr().MP )
			{
				CDB_ShutoffValve *pclBestSV = ( CDB_ShutoffValve * )( pclHMShutoffValve->GetBestIDPtr().MP );
				dShutoffValveDpFO = pclBestSV->GetDp( m_dQ, dRho );

				if( -1.0 == dShutoffValveDpFO )
				{
					dShutoffValveDpFO = 0.0;
				}
			}
		}

		double dHMin = pclDpCBCValve->GetHMin( m_dQ, dRho );

		if( -1.0 == dHMin || ( dHAvail > 0.0 && dHAvail < dHMin ) )
		{
			// If we are going from the pump and the H available for the circuit is not enough...
			continue;
		}

		double dSetting = pclDpCBCValve->GetSetting( m_dQ, dRho, m_pParent->GetDpOnOutOfPrimSide() );

		// Verify Cv Func.
		// Test Cv2W3W, CvFunc.
		if( NULL != m_pParent->GetpSchcat() && NULL != m_pParent->GetpSch() 
				&& false == pclDpCBCValve->IsValidCV( m_pParent->GetpSchcat()->Get2W3W(), m_pParent->GetpSch()->GetCvFunc() ) )
		{
			continue;
		}

		CDB_ControlProperties *pCtrlProp =  dynamic_cast<CDB_ControlProperties *>( pclDpCBCValve->GetCtrlProp() );

		if( NULL == pCtrlProp )
		{
			continue;
		}

		// Check if control mode is compatible with the valve characteristic.
		if( false == pCtrlProp->CvCtrlTypeFits( m_pParent->GetCvCtrlType() ) )
		{
			continue;
		}

		// Check max flow.
		if( -1.0 == pclDpCBCValve->GetQmax() || GetQ() > pclDpCBCValve->GetQmax() )
		{
			continue;
		}

		// Check DpMax if we have at least H available.
		if( dHAvail > 0.0 )
		{
			double dDpTU = m_pParent->GetDpOnOutOfPrimSide();
			double dPipeDp = m_pParent->GetpCircuitPrimaryPipe()->GetPipeDp();
			double dSingularDp = m_pParent->GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true );
			double dDpOnValve = dHAvail - dDpTU - dPipeDp - dSingularDp - dShutoffValveDpFO;

			if( dDpOnValve > pclDpCBCValve->GetDpmax() )
			{
				continue;
			}
		}

		double d64Key = pclDpCBCValve->GetSortingKey();
		mapIt = mapDpCBCValve.insert( std::make_pair( d64Key, CCVData( pclDpCBCValve ) ) );

		mapIt->second.SetHMin( dHMin );
		mapIt->second.SetSetting( dSetting );
		mapIt->second.SetDplmin( dDplmin );
		mapIt->second.SetDplmax( dDplmax );
		mapIt->second.SetDpToStabilise( dDpStabilize );
	}

	if( 0 == (int)mapDpCBCValve.size() )
	{
		return -1.0;
	}

	//--------------------------------------------
	// Verify pipe size shift.
	//--------------------------------------------

	int iHighestSize;
	int iLowestSize;

	CAnchorPt::PipeLocation ePipeLocation = m_pParent->GetpSch()->GetAnchorPtPipeLocation( CAnchorPt::eFunc::DPCBCV );
	CDB_Pipe *pPipe = m_pParent->GetPipeSizeShift( iHighestSize, iLowestSize, CAnchorPt::CircuitSide_Primary, ePipeLocation );

	if( NULL == pPipe )
	{
		return -1.0;
	}

	CTable *pDNTab = m_pParent->m_pclTableDN;
	ASSERT( NULL != pDNTab );
	bool bOneValveExist = false;
	int iSmallestValve = 0;
	int iHighestValve = 99999;

	// Adapt iLowestSize and iHigestSize to include at least one valve
	for( mapIt = mapDpCBCValve.begin(); mapIt != mapDpCBCValve.end() && bOneValveExist == false; ++mapIt )
	{
		int iProdSizeKey = mapIt->second.GetpCV()->GetSizeKey();

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

	for( mapIt = mapDpCBCValve.begin(); mapIt != mapDpCBCValve.end(); )
	{
		int iProdSizeKey = mapIt->second.GetpCV()->GetSizeKey();

		if( !( iProdSizeKey >= iLowestSize && iProdSizeKey <= iHighestSize ) )
		{
			if( mapIt == mapDpCBCValve.begin() )
			{
				// First proposition is rejected
				// TODO: warning message in compilation Box
			}

			// Remove this selection.
			if( mapDpCBCValve.size() > 1 )
			{
				std::multimap<double, CCVData>::iterator it = mapIt;
				++mapIt;
				mapDpCBCValve.erase( it );
				continue;
			}
		}
		else
		{
			bOneValveExist = true;
		}

		CDB_DpCBCValve *pclDpCBCValve = (CDB_DpCBCValve *)mapIt->second.GetpCV();

		// Take a copy of map if requested.
		if( NULL != pMap )
		{
			pMap->insert( std::pair<double, CDB_DpCBCValve *> ( mapIt->first, pclDpCBCValve ) );
		}

		m_mapCV.insert( std::make_pair( pclDpCBCValve->GetSizeKey(), CCVData( mapIt->second ) ) );

		++mapIt;
	}

	// When pMap exist stop selection process here.
	// Function is only used to have a list a matching PICV.
	if( NULL != pMap )
	{
		return -1.0;
	}

	int iDN15 = m_pParent->GetDN15();
	int iDN50 = m_pParent->GetDN50();
	int iDN = pPipe->GetSizeKey( TASApp.GetpTADB() );

	CDB_StringID *psidType = NULL;
	CDB_StringID *psidFam = NULL;
	CDB_StringID *psidBdy = NULL;
	CDB_StringID *psidConn = NULL;
	CDB_StringID *psidVers = NULL;
	CDB_StringID *psidPN = NULL;
	std::multimap<double, CCVData>::iterator ItBest = mapDpCBCValve.end();

	// Apply User preferences.
	bool bEndLoop = false;

	for( int i = 0; i < 2 && false == bEndLoop; i++ )
	{
		switch( i )
		{
			// If a selected device exist try to find a device with same parameters (if we are in the same side of DN50 border).
			case 0:
				if( NULL != GetCvIDPtr().MP )
				{
					CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( GetCvIDPtr().MP );

					if( NULL != pTAP )
					{
						psidType = dynamic_cast<CDB_StringID *>( pTAP->GetTypeIDPtr().MP );
						psidFam	= dynamic_cast<CDB_StringID *>( pTAP->GetFamilyIDPtr().MP );
						psidBdy = dynamic_cast<CDB_StringID *>( pTAP->GetBodyMaterialIDPtr().MP );
						psidConn = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );
						psidVers = dynamic_cast<CDB_StringID *>( pTAP->GetVersionIDPtr().MP );
						psidPN = dynamic_cast<CDB_StringID *>( pTAP->GetPNIDPtr().MP );

						// If user force a new connection...
						if( NULL != m_pUserChoiceCVConn && m_pUserChoiceCVConn != psidConn )
						{
							psidType = NULL;
							psidBdy = NULL;
							psidFam = NULL;
							psidConn = m_pUserChoiceCVConn;
							psidVers = NULL;
							psidPN = NULL;
						}

						// If user force a new version...
						if( NULL != m_pUserChoiceCVVers && m_pUserChoiceCVVers != psidVers )
						{
							psidType = NULL;
							psidBdy = NULL;
							psidFam = NULL;
							psidVers = m_pUserChoiceCVVers;
							psidPN = NULL;
						}

						// If user force a new PN...
						if( NULL != m_pUserChoiceCVPN && m_pUserChoiceCVPN != psidPN )
						{
							psidType = NULL;
							psidBdy = NULL;
							psidFam = NULL;
							psidPN = m_pUserChoiceCVPN;
						}

						// Clear all specific user request.
						m_pUserChoiceCVConn = NULL;
						m_pUserChoiceCVVers = NULL;
						m_pUserChoiceCVPN = NULL;
						bEndLoop = true;
					}
				}

				break;

			case 1:
				// Default preferences from tech params.
			{
				CPrjParams *pPrjParams = m_pParent->GetpPrjParam()->GetpHmCalcParams();

				if( iDN <= iDN50 )
				{
					psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65TypeID ).MP );
					psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65FamilyID ).MP );
					psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65BdyMatID ).MP );
					psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65ConnectID ).MP );
					psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65VersionID ).MP );
					psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65PNID ).MP );
				}
				else
				{
					psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50TypeID ).MP );
					psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50FamilyID ).MP );
					psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50BdyMatID ).MP );
					psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50ConnectID ).MP );
					psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50VersionID ).MP );
					psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50PNID ).MP );
				}

				bEndLoop = true;
			}
			break;

		}
	}

	if( 0 == mapDpCBCValve.size() )
	{
		// NOT FOUND !!!!
		return -1.0;
	}

	CTAPSortKey sKey( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );
	int iRet = 0;

	// By default best DpCBCV is the first ones.
	ItBest = mapDpCBCValve.end();
	double dBestRelPreSet = 0.0;
	double dBestHMin = DBL_MAX;

	// TODO: Is same as PICV??
	// double dAcceptedExtraDp = m_pParent->GetpTechParam()->GetPICVExtraThresholdDp();

	// Don't take care of ePriority.
	UINT uiMask = CTAPSortKey::TAPSKM_All & ~CTAPSortKey::TAPSKM_PriorL;

	// Mask empty fields.
	uiMask = uiMask & ~CTAPSortKey::TAPSKM_Size;

	if( NULL == psidType )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Type;
	}

	if( NULL == psidFam )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Fam;
	}

	if( NULL == psidBdy )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Bdy;
	}

	if( NULL == psidConn )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Connect;
	}

	if( NULL == psidVers )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_Version;
	}

	if( NULL == psidPN )
	{
		uiMask = uiMask & ~CTAPSortKey::TAPSKM_PN;
	}

	CDB_DpCBCValve *pclBestDpCBCValve = NULL;

	for( mapIt = mapDpCBCValve.begin(); mapIt != mapDpCBCValve.end(); ++mapIt )
	{
		CTAPSortKey mKey( mapIt->second.GetpCV() );

		iRet = mKey.Compare( &sKey, &CTAPSortKey::CTAPSKMask( ( CTAPSortKey::TAPSortKeyMask )uiMask ) );

		if( 0 == iRet )			//match
		{
			CDB_DpCBCValve *pclDpCBCValve = dynamic_cast<CDB_DpCBCValve *>( mapIt->second.GetpCV() );

			if( NULL == pclDpCBCValve )
			{
				continue;
			}

			// Keep smallest Dpmin (FC) when Havail <= 0 / largest Dpmin below Havail when Havail >= 0.
			bool bNewBestExist = false;

			double dSetting = mapIt->second.GetSetting();
			double dMaxSetting = pclDpCBCValve->GetMaxSetting();

			double dPresetRatio = 0.0;

			if( ItBest == mapDpCBCValve.end() )
			{
				bNewBestExist = true;
				dPresetRatio = dSetting / dMaxSetting;
			}
			else
			{
				int iBestSize = ItBest->second.GetpCV()->GetSizeKey();
				int iSize = mapIt->second.GetpCV()->GetSizeKey();

				// New valve is largest than best valve but his Dp(with extra Dp) is lower.
				// New valve is smallest than the best valve and his minDp is lower.
				if( iSize == iBestSize )
				{
					if( mapIt->second.GetHMin() < ( dBestHMin /*+ dAcceptedExtraDp*/ ) )
					{
						if( -1.0 != dSetting && -1.0 != dMaxSetting )
						{
							dPresetRatio = dSetting / dMaxSetting;

							if( -1.0 != dPresetRatio && dPresetRatio > dBestRelPreSet )
							{
								bNewBestExist = true;
							}
						}
					}
				}
				else if( ( iSize < iBestSize && mapIt->second.GetHMin() < ( dBestHMin /*+ dAcceptedExtraDp*/ ) ) ||
						 ( iSize > iBestSize && ( mapIt->second.GetHMin() /*+ dAcceptedExtraDp*/ ) < dBestHMin ) )
				{
					bNewBestExist = true;
				}
			}

			if( true == bNewBestExist )
			{
				dBestHMin = mapIt->second.GetHMin();
				ItBest = mapIt;
				pclBestDpCBCValve = pclDpCBCValve;
				dBestRelPreSet = dPresetRatio;
			}
		}
	}

	// DpCBCV not found take the first ones by default.
	if( ItBest == mapDpCBCValve.end() )
	{
		ItBest = mapDpCBCValve.begin();
	}

	SetBestCvIDPtr( ItBest->second.GetpCV()->GetIDPtr() );
	return ItBest->second.GetHMin();
}

void CDS_HydroMod::CCv::Size3WCvCstFlow( double dDp, bool fFromPump )
{
	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
	
	if( NULL == pclWaterChar )
	{
		ASSERT_RETURN;
	}

	// HYS-1333 : Don't compute when sizing because we loose the needed kvs.
	m_pParent->EnableComputeHM( false );
	dDp = __max( dDp, GetDpMin() );
	double dKvsMax = CalcKv( GetQ(), dDp, pclWaterChar->GetDens() );
	double dKvs = _SizeKvs( dDp, fFromPump );
	SetKvsmax( dKvsMax );
	m_pParent->EnableComputeHM( true );
}

double CDS_HydroMod::CCv::SizeControlValveForBetaMin( double dHAvailAtPI )
{
	// This method is mainly called when we need to reset a control valve with 'dHAvailAtPI' to 0.

	if( GetQ() <= 0.0 )
	{
		// HYS-1753: Clean the selection when de flow is not set
		InvalidateSelection();
		return dHAvailAtPI;
	}

	bool bFromPump = ( dHAvailAtPI > 0.0 ) ? true : false;
	CDB_CircSchemeCateg *pSchcat = m_pParent->GetpSchcat();

	if( NULL == pSchcat )
	{
		ASSERTA_RETURN( dHAvailAtPI );
	}

	if( eb3True == IsPICV() || eb3True == IsDpCBCV() )
	{
		return SizePIBCVorDpCBCV( dHAvailAtPI );
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( dHAvailAtPI );
	}

	m_pParent->EnableComputeHM( false );

	// During the first computing phase relax the opening of pre-settable valve.
	if( true == IsPresettable() )
	{
		CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( GetCvIDPtr().MP );

		if( NULL != pRv && NULL != pRv->GetValveCharacteristic() && true == pRv->GetValveCharacteristic()->HasKvCurve() )
		{
			double dOpeningMax = pRv->GetValveCharacteristic()->GetOpeningMax();

			if( -1.0 != dOpeningMax )
			{
				SetSetting( dOpeningMax );
			}
		}
	}

	/////////////////////////////////////////////////////////////
	// Verify if Dp on the control valve is stabilized by a DpC.
	// DpC are allowed on primary of injection circuits
	/////////////////////////////////////////////////////////////
	if( true == m_pParent->IsDpCExist() && eb3True == IsCVLocInPrimary() && NULL != m_pParent->GetpSch() && eDpStab::DpStabOnCV == m_pParent->GetpSch()->GetDpStab() )
	{
		return _SizeControlValveForBetaMinHMwithDpConPrimary( dHAvailAtPI );
	}

	CDS_HydroMod *pPIHM = NULL;
	double dHreqAtPressInt = m_pParent->_GetHreqPressInt();

	// Determine if we are working with constant or variable flow.
	bool bConstantFlow = true;

	if( eb3True == IsCVLocInPrimary() )
	{
		// HYS-1757: pay attention for a particular case. If we are working with a 3-way injection circuit with decoupling bypass AND
		// with a Dp controller that is stabilising differential pressure on branch AND the 3-way control valve is located at the 
		// primary side AND we are on an hydraulic circuit with terminal unit! The pressure interface returned will be the circuit itself 
		// and it is not correct in that case. Because the "CDS_HydroMod::m_bConstantFlow" variable is not updated for circuit but well 
		// for module and ONLY for pressure interface (This is done in the "CDS_HydroMod::ComputeAll" method at the beginning).
		// For sizing the control valve, we can effectively consider that the circuit itself is the pressure interface. Control valve will need to 
		// be sized with the H available that is here the differential pressure stabilized in the branch.
		// But to check if the primary side for this circuit has a constant or variable flow, we need to really retrieve this information from
		// the parent. Dp controller on branch doesn't not influence the variability of the flow at the primary side.
		pPIHM = m_pParent->GetpPressIntHM();
		CDS_HydroMod *pclPIForFlowVariability = pPIHM;

		// Verify 3 conditions: 
		//	1) Is it a circuit ?
		//  2) Is it a 3-way mixing circuit with decoupling bypass ?
		//  3) Is there a Dp controller that is stabilising the differenrial pressure on the branch ?
		if( false == m_pParent->IsaModule() && NULL != m_pParent->GetpSch() && CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == m_pParent->GetpSchcat()->Get3WType() 
				&& CDS_HydroMod::ePressIntType::pitDpC == m_pParent->GetPressIntType() )
		{
			pclPIForFlowVariability = pPIHM->GetParent()->GetpPressIntHM();
		}

		bConstantFlow = pclPIForFlowVariability->IsConstantFlow();

		// Remark: if we have a 3-way mixing circuit with decoupling bypass AND a Dp controller that stabilises the differential pressure on the branch AND
		//         the control valve at the primary side, the authority of the control valve is near 1. So if for this circuit the primary flow is variable, 
		//         we don't want that the control valve be sized as a 3-way normal control valve. But we want that this one be sized in the same way as if 
		//         the primary flow is constant. This is why in this case we force 'bConstantFlow' to 'true'.
		// See comments in the "CDS_Hm3WInj::ComputeHM" with case F for more details.
		if( CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == m_pParent->GetpSchcat()->Get3WType() && CDS_HydroMod::ePressIntType::pitDpC == m_pParent->GetPressIntType() )
		{
			bConstantFlow = true;
		}
	}
	else
	{
		// We are working on secondary side, 3-ways injection circuit with shortcut bypass.
		pPIHM = m_pParent;

		// Flag constant flow is depending of children if exist...
		if( true == pPIHM->IsaModule() )
		{
			// Constant flow flag has been updated in the beginning of ComputeAll() for all the pressure interfaces.
			bConstantFlow = pPIHM->IsConstantFlow();
		}
		else
		{
			CDB_CircSchemeCateg *pSchCateg = pPIHM->GetpSchcat();

			if( pSchCateg != NULL )
			{
				bConstantFlow = ( CDB_CircSchemeCateg::eFLOWVAR::Constant == pSchCateg->GetVarSecondFlow() );
			}

			// Update constant flow flag for authority computing (CCV::GetAuth()).
			pPIHM->SetFlagConstantFlow( bConstantFlow );
		}

		// Remark: if we have a 3-way mixing circuit with decoupling bypass AND the control valve at the secondary side, the authority of the control valve is always near 1. 
		//         So we want that the control valve be sized as it was in constant flow. This is why in this case we force the variable to "true".
		if( CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == m_pParent->GetpSchcat()->Get3WType() )
		{
			bConstantFlow = true;

			// And because the authority near 1, we can size the control valve to its minimum.
			dHreqAtPressInt = 0;
		}
	}

	// Specific case for 3w mixing circuit.
	if( pSchcat->Get3WType() == CDB_CircSchemeCateg::e3wTypeMixing )
	{
		double dDp = m_pParent->GetHAvail();
		Size3WCvCstFlow( dDp, bFromPump );
		dHAvailAtPI = dHreqAtPressInt + GetDp();
	}
	else if( true == pSchcat->Is3W() && true == bConstantFlow )
	{
		///////////////////////////////////////////////
		// 3-way, constant flow.
		///////////////////////////////////////////////
		double dDp = 0;

		if( true == IsOn_Off() )
		{
			dDp = GetDpMin();
		}
		else if( pSchcat->Get3WType() == CDB_CircSchemeCateg::e3wTypeDividing )
		{
			// Auth>=0.5 -> Dp(v3v) = Dp(TermUnit).
			dDp = m_pParent->GetDpOnOutOfPrimSide();
		}
		else
		{
			if( eb3True == IsCVLocInPrimary() )
			{
				// In case of 3w circuit with CV located in primary CV must take at least 3kPa,
				// if available pressure is greater try to recuperate some on the CV to decrease the size.
				if( NULL != m_pParent->GetpDpC() )
				{
					// Dpc Exist on this circuit.
					dDp = m_pParent->GetpDpC()->GetDplmin();

					if( m_pParent->GetpDpC()->GetMvLoc() == MvLocSecondary )
					{
						dDp -= m_pParent->GetpBv()->GetDpMin();
					}
				}
				else
				{
					dDp = dHAvailAtPI - dHreqAtPressInt;

					if( NULL != m_pParent->GetpBv() )
					{
						dDp -= m_pParent->GetpBv()->GetDpMin();
					}
				}

				// If we are using Reynard serie we have to take value above to avoid a too big CVDp.
				if( dDp > 0.0 && m_pParent->GetpTechParam()->GetCVDefKvReynard() )
				{
					double dRho = pclWaterChar->GetDens();
					double dKvs = 0.0;
					double dQ = GetQ();
					dKvs = CalcKv( dQ, dDp, dRho );
					dKvs = GetReynardVal( dKvs, true );
					dDp = CalcDp( dQ, dKvs, dRho );
				}

				dDp = max( dDp, GetDpMin() );
			}
			else
			{
				/// In case of 3w circuit with decoupling bypass GetDpOnOutOfPrimSide() will return Dp for Bypass ~= 0
				dDp = m_pParent->GetDpOnOutOfPrimSide();
			}
		}

		Size3WCvCstFlow( dDp, bFromPump );
		dHAvailAtPI = dHreqAtPressInt + GetDp();
	}
	else
	{
		///////////////////////////////////////////////////
		// 2-way, or 3-way variable flow.
		///////////////////////////////////////////////////
		double dCvDp = 0.0;

		// In a variable flow system a 3w valve is resized as a 2w valve.
		if( false == IsOn_Off() )
		{
			// Min Dp according to RP, prevent laminar flow.
			bool bMinDpNotReached = false;
			double dCvDpForMinAuthor = m_pParent->GetTechParamCVMinDpProp();
			CDB_DpController *pDpC = NULL;
			bool bDpStabilizedByDpC = false;
			double dHAvailAtPICorrectedWithPB = dHAvailAtPI;

			if( dHAvailAtPI <= 0.0 )
			{
				dCvDp = 0.0;

				if( true == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
				{
					dCvDp = GetDpMin();
				}
			}
			else
			{
				// Specific treatment when Dp is stabilized by a Dp controller, we have to take into account the proportional band.
				// pPIHM is the circuit itself, CV must be located on the same side that Dp controller.
				// pPIHM is another upstream circuit, is not an injection circuit in this case Dp controller is located at the primary side.
				if( CDS_HydroMod::ePressIntType::pitDpC == pPIHM->GetPressIntType() &&
					( ( pPIHM == m_pParent && eb3True == IsCVLocInPrimary() ) || ( pPIHM != m_pParent && NULL != pPIHM->GetpSchcat() && false == pPIHM->GetpSchcat()->IsSecondarySideExist() ) ) )
				{
					if( true == pPIHM->IsDpCExist() )
					{
						pDpC = static_cast<CDB_DpController *>( pPIHM->GetpDpC()->GetIDPtr().MP );

						if( NULL != pDpC && NULL != pDpC->GetDpCCharacteristic() )
						{
							bDpStabilizedByDpC = true;
							dHAvailAtPICorrectedWithPB = pDpC->GetDpStabCorrectedWithPBfactor( dHAvailAtPI );
						}
					}
				}

				// For 3-way mixing circuit with decoupling bypass AND the control valve localized at the primary side AND NO Dp controller 
				// that stabilises the differential pressure on the branch AND the distribution flow is variable, the authority is computed with one other formule.
				if( NULL != m_pParent->GetpSchcat() && CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == m_pParent->GetpSchcat()->Get3WType() 
						&& NULL != m_pParent->GetpSch() && CvLocation::CvLocPrimSide == m_pParent->GetpSch()->GetCvLoc()
						&& CDS_HydroMod::ePressIntType::pitDpC != m_pParent->GetPressIntType() )
				{
					// Bmin = DpCV / ( H@PI * ( 1 - ( DpBV / Ha ) ) ).
					// Ha =  DpCV + DpBV -> Bmin = DpCV / ( H@PI * ( 1 - ( DpBV / DpCV + DpBV ) ) ).
					// DpCV = Bmin * H@PI * ( 1 - ( DpBV / DpCV + DpBV ) )
					// DpCV = Bmin * H@PI * ( DpCV + DpBV - DpBV / DpCV + DpBV )
					// DpCV = Bmin * H@PI * ( DpCV / DpCV + DpBV )
					// DpCV + DpBV = Bmin * H@PI
					// DpCV = Bmin * H@PI - DpBV

					// Finally it's the same formula except that we deduce the pressure drop of the balancing valve why ?
					// With 2-way valves, when the valve is closed, no more flow are circulated and then it's the H available at the entry of
					// the circuit that is applied on the control valve.
					// With a 3-way valve, it's not the same. When the valve is "closed", in fact it let all the flow pass through the first bypass.
					// There is then yet a flow and the pressure drop accross the balancing valve at the primary side. When 3-way valve is closed
					// we thus have the H available at the entry of the circuit minus the pressure drop accross the balancing valve.

					dCvDp = dHAvailAtPICorrectedWithPB * m_pParent->GetpTechParam()->GetCVMinAuthor();

					if( NULL != m_pParent->GetpBv() && m_pParent->GetpBv()->GetDp() > 0.0 )
					{
						dCvDp -= m_pParent->GetpBv()->GetDp();
					}
				}
				else
				{
					dCvDp = dHAvailAtPICorrectedWithPB * m_pParent->GetpTechParam()->GetCVMinAuthor();
				}

				if( dCvDp < GetDpMin() )
				{
					dCvDp = GetDpMin();
				}
			}

			// Never below 10kPa.
			dCvDp = max( dCvDp, dCvDpForMinAuthor );

			// Specific for BV-CM.
			CDB_RegulatingValve *pRV = static_cast<CDB_RegulatingValve *>( GetCvIDPtr().MP );
			bool bBV_CM = ( NULL != pRV && ( CDB_ControlProperties::eBCVType::ebcvtCharIndep == pRV->GetCtrlProp()->GetBCVType() 
					|| CDB_ControlProperties::eBCVType::ebcvtLiftLimit == pRV->GetCtrlProp()->GetBCVType() ) );

			double d = dCvDp;
			double dKvs = _SizeKvs( d, false );

			if( false == bBV_CM )
			{
				// Never below 10kPa.
				dCvDp = max( d, dCvDpForMinAuthor );
			}

			// H at pressure interface is not yet defined, first step of sizing.
			if( dHAvailAtPI <= 0.0 || dHreqAtPressInt < 0.0 )
			{
				m_pParent->EnableComputeHM( true );
				return max( d + dHreqAtPressInt, dHAvailAtPI );
			}

			// Keep current Dp for locked Kvs.
			if( false == IsPresettable() && true == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
			{
				dCvDp = GetDp();
			}

			double dTotalHrequired = dHreqAtPressInt + dCvDp;

			// H available at pressure interface is too low it must be reevaluated.
			if( dTotalHrequired > dHAvailAtPI )
			{
				double dDpForMinAuthor = 0.0;

				// Compute Dp for minimum authority.
				// Bmin is the ratio between the pressure drop accross the control valve at design condition and the total available pressure drop when closed.
				// Bmin ~= DpCV / H@PI (H@PI = H available at the pressure interface).
				// Above, we have called the "CDS_HydroMod::_GetHreqPressInt" method to know what is the H available at the pressure interface. 
				// AND we remove the current pressure drop of the control valve deliberately.
				// So, HreqAtPump here is in fact equal to H@PI - DpCV -> H@PI = HreqAtPump + DpCV.
				// Bmin = DpCV / H@PI = DpCV / ( HreqAtPump + DpCV )
				// DpCV = Bmin * ( HreqAtPump + DpCV ) -> DpCV = Bmin * HreqAtPump + Bmin * DpCV
				// DpCV - ( Bmin * DpCV ) = Bmin * HreqAtPump -> DpCV * ( 1 - Bmin ) = Bmin * HreqAtPump -> DpCV = Bmin / ( 1 - Bmin ) * HreqAtPump
				// Here Bmin = 0.25 -> DpCV = 0.25 / ( 1 - 0.25 ) * HreqAtPump -> DpCV = 1/3 * HreqAtPump.

				// For 3-way mixing circuit with decoupling bypass AND the control valve localized at the primary side AND NO Dp controller 
				// that stabilises the differential pressure on the branch AND the distribution flow is variable, the formula is almost similar.
				// We just start with Bmin ~= DpCV / ( H@PI - DpBV ) to obtain finally DpCV = Bmin / ( 1 - Bmin ) * ( HreAtPump - DpBV ).
				double dDpBV = 0.0;

				if( NULL != m_pParent->GetpSchcat() && CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == m_pParent->GetpSchcat()->Get3WType() 
						&& NULL != m_pParent->GetpSch() && CvLocation::CvLocPrimSide == m_pParent->GetpSch()->GetCvLoc()
						&& CDS_HydroMod::ePressIntType::pitDpC != m_pParent->GetPressIntType() )
				{
					if( NULL != m_pParent->GetpBv() && m_pParent->GetpBv()->GetDp() > 0.0 )
					{
						dDpBV = m_pParent->GetpBv()->GetDp();
					}
				}

				dDpForMinAuthor = ( dHreqAtPressInt - dDpBV ) * m_pParent->GetpTechParam()->GetCVMinAuthor() / ( 1 - m_pParent->GetpTechParam()->GetCVMinAuthor() );
				
				// Never below 10kPa.
				dDpForMinAuthor = max( dDpForMinAuthor, dCvDpForMinAuthor );

				if( dDpForMinAuthor > dCvDp )
				{
					// When computing Kvs; dDpForMinAuthor is reavualuted for Kvs found (Reynard series)
					dKvs = _SizeKvs( dDpForMinAuthor, bFromPump );
					dCvDp = dDpForMinAuthor;
				}

				dHAvailAtPI = ( dHreqAtPressInt - dDpBV ) + dCvDp;
				dHAvailAtPICorrectedWithPB = dHAvailAtPI;

				if( NULL != pDpC && true == bDpStabilizedByDpC )
				{
					dHAvailAtPICorrectedWithPB = pDpC->GetDpStabCorrectedWithPBfactor( dHAvailAtPI );
				}
			}

			// HAvailable is bigger than what is requested for the valve.
			// Update KvsMax according this new value of H available.
			double dDpForHavailable = 0.0;

			// For 3-way mixing circuit with decoupling bypass AND the control valve localized at the primary side AND no Dp controller 
			// that stabilises the differential pressure on the branch AND the distribution flow is variable, the authority is computed with one other formule.
			// See notes above.
			if( NULL != m_pParent->GetpSchcat() && CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == m_pParent->GetpSchcat()->Get3WType() 
					&& NULL != m_pParent->GetpSch() && CvLocation::CvLocPrimSide == m_pParent->GetpSch()->GetCvLoc()
					&& CDS_HydroMod::ePressIntType::pitDpC != m_pParent->GetPressIntType() )
			{
				dDpForHavailable = dHAvailAtPICorrectedWithPB * m_pParent->GetpTechParam()->GetCVMinAuthor();

				if( NULL != m_pParent->GetpBv() && m_pParent->GetpBv()->GetDp() > 0.0 )
				{
					dDpForHavailable -= m_pParent->GetpBv()->GetDp();
				}
			}
			else
			{
				dDpForHavailable = dHAvailAtPICorrectedWithPB * m_pParent->GetpTechParam()->GetCVMinAuthor();
			}

			// Never below 10kPa.
			dDpForHavailable = max( dDpForHavailable, dCvDpForMinAuthor );
			double dKvsMax = CalcKv( GetQ(), dDpForHavailable, pclWaterChar->GetDens() );
			SetKvsmax( dKvsMax );
			SetBestKvs( dKvs );

			if( false == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
			{
				m_dKvs = dKvs;
				m_pParent->Modified();
			}
		}
		else // On_Off
		{
			dCvDp = dHAvailAtPI;

			if( dHAvailAtPI <= 0 )
			{
				dCvDp = 0.0;

				if( true == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
				{
					dCvDp = GetDpMin();
				}

				dCvDp = max( dCvDp, m_pParent->GetpTechParam()->GetCVMinDpOnoff() );
			}

			double dKvs = _SizeKvs( dCvDp, bFromPump );
			SetKvsmax( dKvs );
		}

		// In case of pressettable valve compute new opening.
		// Needed to have a correct return of GetDp().
		if( true == IsPresettable() )
		{
			CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( GetCvIDPtr().MP );

			if( NULL != pRv && NULL != pRv->GetValveCharacteristic() )
			{
				CDB_ValveCharacteristic *pclValveCharacteristic = pRv->GetValveCharacteristic();
				double dRho = pclWaterChar->GetDens();
				double dNu = pclWaterChar->GetKinVisc();
				double dH = 0.0;

				if( dCvDp < GetDpMin() )
				{
					dCvDp = GetDpMin();
				}

				int iRounding = ( eBool3::eb3True == pclValveCharacteristic->IsDiscrete() ) ? 2 : 1;
				bool bValid = pclValveCharacteristic->GetValveOpening( GetQ(), dCvDp, &dH, dRho, dNu, iRounding );

				if( false == bValid )
				{
					// Dp at full opening if greater than requested Dp.
					// Ask Dp at full opening.
					dH = pclValveCharacteristic->GetOpeningMax();

					if( -1.0 != dH )
					{
						// If opening is correct, check if we can have Dp on the valve.
						bValid = pclValveCharacteristic->GetValveDp( GetQ(), &dCvDp, dH, dRho, dNu );
					}
				}

				if( true == bValid )
				{
					SetSetting( dH );
				}
			}
		}
	}

	m_pParent->EnableComputeHM( true );
	return dHAvailAtPI;
}

double CDS_HydroMod::CCv::SizePIBCVorDpCBCV( double dDp )
{
	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	// PICV are sized in the 'going to the pump' phase;
	// If H available is bigger than valve Dpmax we should try to find another valve that fit this new requirement.
	if( 0.0 != dDp )
	{
		CDB_TAProduct *pTap = dynamic_cast<CDB_TAProduct *>( GetCvIDPtr().MP );

		if( NULL == pTap || pTap->GetDpmax() < dDp )
		{
			if( eb3True == IsPICV() )
			{
				SelectBestPICV( dDp );
			}
			else
			{
				if( -1.0 == SelectBestDpCBCV( dDp ) )
				{
					InvalidateSelection();
					ClearUserChoiceTemporaryVariables();
				}
			}
		}
	}
	else
	{
		if( eb3True == IsPICV() )
		{
			SelectBestPICV( 0.0 );
		}
		else
		{
			if( -1.0 == SelectBestDpCBCV( 0.0 ) )
			{
				InvalidateSelection();
				ClearUserChoiceTemporaryVariables();
			}
		}
	}

	if( false == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
	{
		SetpSelCV( (LPARAM)GetBestCvIDPtr().MP );
	}

	CDB_PIControlValve *pPICV = dynamic_cast<CDB_PIControlValve *>( GetCvIDPtr().MP );
	CDB_DpCBCValve *pDpCBCValve = dynamic_cast<CDB_DpCBCValve *>( GetCvIDPtr().MP );

	if( NULL != pPICV )
	{
		double dPresetting = pPICV->GetPresetting( GetQ(), pclWaterChar->GetDens(), pclWaterChar->GetKinVisc() );

		if( -1.0 != dPresetting )
		{
			SetSetting( dPresetting );
		}
	}
	else if( NULL != pDpCBCValve )
	{
		double dPresetting = pDpCBCValve->GetSetting( GetQ(), pclWaterChar->GetDens(), m_pParent->GetDpOnOutOfPrimSide() );

		if( -1.0 != dPresetting )
		{
			SetSetting( dPresetting );
		}
	}
	return dDp;
}

// Return the first Reynard value Above/below dVal
double CDS_HydroMod::CCv::GetReynardVal( double dVal, bool bAbove )
{
	double dReynard = 0.0;

	if( true == bAbove )
	{
		for( int i = ( m_pRHSerie->GetNumofLines() - 1 ); i >= 0 ; i-- )
		{
			if( _tstof( m_pRHSerie->GetString( i ) ) >= dVal )
			{
				dReynard = _tstof( m_pRHSerie->GetString( i ) );
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		for( int i = 0; i < m_pRHSerie->GetNumofLines(); i++ )
		{
			if( _tstof( m_pRHSerie->GetString( i ) ) <= dVal )
			{
				dReynard = _tstof( m_pRHSerie->GetString( i ) );
			}
			else
			{
				break;
			}
		}
	}

	if( 0.0 == dReynard )
	{
		return dVal;
	}

	return dReynard;
}

bool CDS_HydroMod::CCv::IsBestCV( CData *pData )
{
	if( GetBestCvIDPtr().MP == (CData *) pData )
	{
		return true;
	}

	return false;
}

bool CDS_HydroMod::CCv::IsBestCV( double dKvs )
{
	return ( GetBestKvs() == dKvs ) ? true : false;
}

double CDS_HydroMod::CCv::GetKv()
{
	double dKv = -1.0;

	if( true == IsTaCV() && true == IsPresettable() )
	{
		if( _T('\0') != *GetCvIDPtr().ID )
		{
			// TODO AL CCv::GetKvs() PICV
			if( NULL == dynamic_cast<CDB_PIControlValve *>( GetCvIDPtr().MP )
					&& NULL == dynamic_cast<CDB_DpCBCValve *>( GetCvIDPtr().MP ) )
			{
				CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( GetCvIDPtr().MP );

				// If curve exists, use it.
				if( NULL != pRv->GetValveCharacteristic() && true == pRv->GetValveCharacteristic()->HasKvCurve() )
				{
					dKv = pRv->GetValveCharacteristic()->GetKv( GetSetting() );
					dKv = __max( 0, dKv );
				}
			}
		}
	}

	return dKv;
}

double CDS_HydroMod::CCv::GetKvs()
{
	double dKvs = 0.0;

	if( true == IsTaCV() )
	{
		if( _T('\0') != *GetCvIDPtr().ID )
		{
			// TODO AL CCv::GetKvs() PICV
			if( NULL == dynamic_cast<CDB_PIControlValve *>( GetCvIDPtr().MP ) 
					&& NULL == dynamic_cast<CDB_DpCBCValve *>( GetCvIDPtr().MP ) )
			{
				CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( GetCvIDPtr().MP );

				// If curve exists, use it.
				if( NULL != pRv->GetValveCharacteristic() && true == pRv->GetValveCharacteristic()->HasKvCurve() )
				{
					dKvs = pRv->GetKvs();

					if( -1.0 == dKvs )
					{
						dKvs = 0.0;
					}
				}
				else
				{
					// Is it a Control Valve?
					CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( pRv );

					if( NULL != pCV )
					{
						dKvs = pCV->GetKvs();
					}
					else
					{
						ASSERT( 0 );
					}
				}
			}
		}
	}
	else
	{
		dKvs = m_dKvs;
	}

	return dKvs;
}

double CDS_HydroMod::CCv::GetKvsmax()
{
	return m_dKvsMax;
}

bool CDS_HydroMod::CCv::IsPresettable()
{
	CDB_CircuitScheme *pSch = dynamic_cast<CDB_CircuitScheme *>( m_pParent->GetSchemeIDPtr().MP );

	if( NULL == pSch )
	{
		ASSERTA_RETURN( false );
	}

	return ( CDB_ControlProperties::eCVFUNC::PresetPT == pSch->GetCvFunc() 
			|| CDB_ControlProperties::eCVFUNC::Presettable == pSch->GetCvFunc() );
}

eBool3 CDS_HydroMod::CCv::CheckMinAuthor( bool fDesign )
{
	if( GetAuth( fDesign ) < ( m_pParent->GetpTechParam()->GetCVMinAuthor() - 0.005 ) )
	{
		return eb3False;
	}

	return eb3True;
}

eBool3 CDS_HydroMod::CCv::CheckDpMinMax()
{
	CDB_CircSchemeCateg *pSchcat = m_pParent->GetpSchcat();
	bool b3wMixing = ( pSchcat != NULL && true == pSchcat->Is3W() && CDB_CircSchemeCateg::e3wTypeMixing == pSchcat->Get3WType() );

	if( true == IsOn_Off() || true == b3wMixing )
	{
		if( GetDp() < ( m_pParent->GetpTechParam()->GetCVMinDpOnoff() * 0.99 )
				|| GetDp() > m_pParent->GetpTechParam()->VerifyValvMaxDp( ( CDB_TAProduct * ) GetCvIDPtr().MP ) )
		{
			return eb3False;
		}

		return eb3True;
	}
	else
	{
		if( GetDp() < ( m_pParent->GetTechParamCVMinDpProp() * 0.99 )
				|| GetDp() > m_pParent->GetpTechParam()->VerifyValvMaxDp( ( CDB_TAProduct * ) GetCvIDPtr().MP ) )
		{
			return eb3False;
		}

		return eb3True;
	}
}

eBool3 CDS_HydroMod::CCv::CheckMinOpen()
{
	if( NULL == GetCvIDPtr().MP || false == IsTaCV() || false == IsPresettable() )
	{
		return eBool3::eb3Undef;
	}

	// TODO AL CCv::CheckMinOpen() PICV.
	if( NULL != dynamic_cast<CDB_PIControlValve *>( GetCvIDPtr().MP )
		 || NULL != dynamic_cast<CDB_DpCBCValve *>( GetCvIDPtr().MP ) )
	{
		return eBool3::eb3Undef;
	}

	CDB_RegulatingValve *pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( GetCvIDPtr().MP );

	if( NULL == pclRegulatingValve )
	{
		return eBool3::eb3Undef;
	}

	CDB_ValveCharacteristic *pclValveCharacteristic = pclRegulatingValve->GetValveCharacteristic();

	if( NULL == pclRegulatingValve || false == pclValveCharacteristic->HasKvCurve() )
	{
		return eBool3::eb3Undef;
	}

	double dValveMinOpen = pclValveCharacteristic->GetMinRecSetting();

	if( GetSetting() < dValveMinOpen )
	{
		return eBool3::eb3False;
	}

	return eBool3::eb3True;
}

CString CDS_HydroMod::CCv::GetSettingStr( bool bWithUnit, bool *pbFullSetting )
{
	if( NULL != pbFullSetting )
	{
		*pbFullSetting = false;
	}

	if( NULL == GetCvIDPtr().MP || false == IsTaCV() || false == IsPresettable() )
	{
		return CString( _T("") );
	}

	CString str;

	CDB_RegulatingValve *pRV = dynamic_cast<CDB_RegulatingValve *>( GetCvIDPtr().MP );

	if( NULL != pRV && NULL != pRV->GetValveCharacteristic() )
	{
		str = pRV->GetValveCharacteristic()->GetSettingString( GetSetting(), bWithUnit, pbFullSetting );
	}

	return str;
}

int CDS_HydroMod::CCv::CheckValidity()
{
	m_usValidityFlags = CDS_HydroMod::eValidityFlags::evfOK;

	if( NULL == GetCvIDPtr().MP )
	{
		return m_usValidityFlags;
	}

	CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( GetCvIDPtr().MP );

	if( NULL == pCV )
	{
		return m_usValidityFlags;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( m_usValidityFlags );
	}

	if( pCV->GetDpmax() > 0 )
	{
		if( m_pParent->GetpPressIntHM()->GetHAvail( CAnchorPt::CircuitSide_Secondary ) > pCV->GetDpmax() )
		{
			m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfMaxDpTooHigh;
		}
	}

	// HYS-1794: The same verification is done in CheckDpMinMax used for the display in SheetHMCalc.
	if( eb3False == IsPICV() && eb3False == IsDpCBCV() )
	{
		CDB_CircSchemeCateg* pSchcat = m_pParent->GetpSchcat();
		bool b3wMixing = (pSchcat != NULL && true == pSchcat->Is3W() && CDB_CircSchemeCateg::e3wTypeMixing == pSchcat->Get3WType());

		if( true == IsOn_Off() || true == b3wMixing )
		{
			if( GetDp() < (m_pParent->GetpTechParam()->GetCVMinDpOnoff() * 0.99) )
			{
				m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDpTooLow;
			}
			else if( GetDp() > m_pParent->GetpTechParam()->VerifyValvMaxDp( (CDB_TAProduct*)GetCvIDPtr().MP ) )
			{
				m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDpTooHigh;
			}
		}
		else
		{
			if( GetDp() < (m_pParent->GetTechParamCVMinDpProp() * 0.99) )
			{
				m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDpTooLow;
			}
			else if( GetDp() > m_pParent->GetpTechParam()->VerifyValvMaxDp( (CDB_TAProduct*)GetCvIDPtr().MP ) )
			{
				m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDpTooHigh;
			}

		}
	}

	if( pCV->GetTmax() < pclWaterChar->GetTemp() )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfTempTooHigh;
	}

	if( eb3True == IsPICV() && -1.0 != ( (CDB_PIControlValve *)pCV )->GetQmax() && GetQ() > ( (CDB_PIControlValve *)pCV )->GetQmax() )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfFlowTooHigh;
	}

	if( eb3True == IsDpCBCV() && -1.0 != ( (CDB_DpCBCValve *)pCV )->GetQmax() && GetQ() > ( (CDB_DpCBCValve *)pCV )->GetQmax() )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfFlowTooHigh;
	}

	return m_usValidityFlags;
}

void CDS_HydroMod::CCv::DropData( OUTSTREAM outf, int *piLevel )
{
	CString strTab = _T("");

	for( int i = *piLevel; i; i-- )
	{
		strTab += _T("\t");
	}

	CString str;
	str.Format( _T("m_IDPtrCv.ID = %s\n"), GetCvIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_DiversityIDPtr.ID = %s\n"), GetDiversityIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_IDPtrBestCV.ID = %s\n"), GetBestCvIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_ActrIDPtr.ID = %s\n"), GetActrIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Description = %s\n"), GetDescription() );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Kvsmax = %s\n"), WriteCUDouble( _C_KVCVCOEFF, GetKvsmax() ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Kvs = %s\n"), WriteCUDouble( _C_KVCVCOEFF, GetKvs() ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Q = %s\n"), WriteCUDouble( _U_FLOW, GetQ(), true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Auth = %s\n"), WriteDouble( GetAuth(), 2, 2 ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Setting = %s\n"), WriteDouble( GetSetting(), 2, 2 ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_BestKvs = %s\n"), WriteCUDouble( _C_KVCVCOEFF, GetBestKvs() ) );
	WriteFormatedStringA2( outf, str, strTab );

	str = _T("m_CtrlType = ");

	switch( GetCtrlType() )
	{
		case CDB_ControlProperties::CvCtrlType::eCvNU:
			str.Append( _T("eCvNU\n") );
			break;

		case CDB_ControlProperties::CvCtrlType::eCvProportional:
			str.Append( _T("eCvProportional\n") );
			break;

		case CDB_ControlProperties::CvCtrlType::eCvOnOff:
			str.Append( _T("eCvOnOff\n") );
			break;

		case CDB_ControlProperties::CvCtrlType::eCv3point:
			str.Append( _T("eCv3point\n") );
			break;
	}

	WriteFormatedStringA2( outf, str, strTab );

	str = _T("m_nWay = ");

	switch( GetnWay() )
	{
		case eCVnWay::CV2W:
			str.Append( _T("CV2W\n") );
			break;

		case eCVnWay::CV3W:
			str.Append( _T("CV3W\n") );
			break;

		case eCVnWay::CV3WByp:
			str.Append( _T("CV3WByp\n") );
			break;

		case eCVnWay::CV3WRdcByp:
			str.Append( _T("CV3WRdcByp\n") );
			break;
	}

	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_bTACv = %s\n" ), ( true == IsTaCV() ) ? _T("true") : _T( "false") );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_DpMin = %s\n"), WriteCUDouble( _U_DIFFPRESS, GetDpMin(), true ) );
	WriteFormatedStringA2( outf, str, strTab );

	_DropDataHelp( str, GetpSelCVConn(), _T("m_pSelCVConn") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, GetpSelCVVers(), _T("m_pSelCVVers") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, GetpSelCVBdy(), _T("m_pSelCVBdy") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, GetpSelCVPN(), _T("m_pSelCVPN") );
	WriteFormatedStringA2( outf, str, strTab );

	_DropDataHelp( str, m_pUserChoiceCVConn, _T("m_pTmpCVConn") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, m_pUserChoiceCVVers, _T("m_pTmpCVVers") );
	WriteFormatedStringA2( outf, str, strTab );
	_DropDataHelp( str, m_pUserChoiceCVPN, _T("m_pTmpCVPN") );
	WriteFormatedStringA2( outf, str, strTab );

	str.Format( _T("m_usValidityFlags = %u\n"), GetValidtyFlags() );
	WriteFormatedStringA2( outf, str, strTab );

	if( GetpCVAccIDPtrArray()->GetCount() > 0 )
	{
		str = _T("Selected CV accessories:\n");

		for( int iLoop = 0; iLoop < GetpCVAccIDPtrArray()->GetCount(); iLoop++ )
		{
			str.AppendFormat( _T("\t%s\n"), GetpCVAccIDPtrArray()->GetAt( iLoop ) );
		}
	}
	else
	{
		str = _T("There is no selected CV accessories\n");
	}

	WriteFormatedStringA2( outf, str, strTab );

	if( GetpActrAccIDPtrArray()->GetCount() > 0 )
	{
		str = _T("Selected actuator accessories:\n");

		for( int iLoop = 0; iLoop < GetpActrAccIDPtrArray()->GetCount(); iLoop++ )
		{
			str.AppendFormat( _T("\t%s\n"), GetpActrAccIDPtrArray()->GetAt( iLoop ) );
		}
	}
	else
	{
		str = _T("There is no selected actuator accessories\n");
	}

	WriteFormatedStringA2( outf, str, strTab );

	if( m_pRHSerie != NULL )
	{
		if( 0 == m_pRHSerie->GetNumofLines() )
		{
			str = _T("m_pRHSerie is defined but there is no data\n");
		}
		else
		{
			str = _T("m_pRHSerie = ");

			for( int iLoop = 0; iLoop < m_pRHSerie->GetNumofLines(); iLoop++ )
			{
				str.AppendFormat( _T("%s%s" ), ( 0 == iLoop ) ? _T("") : _T( " ;"), m_pRHSerie->GetString( iLoop ) );
			}

			str.Append( _T("\n") );
		}
	}
	else
	{
		str = _T("m_pRHSerie is not defined\n");
	}

	WriteFormatedStringA2( outf, str, strTab );
	WriteFormatedStringA2( outf, _T("\n"), strTab );
}

bool CDS_HydroMod::CCv::IsActuatorStrongEnough()
{
	// HYS-941: CDB_ElectroActuator is too specific use CDB_Actuator
	CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( GetActrIDPtr().MP );

	if( NULL != pclActuator )
	{
		CDB_ControlValve *pCV = GetpCV();

		if( NULL == pCV )
		{
			return 0;
		}

		// CV exist try to find some compliant actuators
		CDB_CloseOffChar *pCloseOffChar = static_cast< CDB_CloseOffChar * >( pCV->GetCloseOffCharIDPtr().MP );
		CDS_HydroMod *pPIHM = m_pParent->GetpPressIntHM();
		double dMinCloseOffDp = 0;

		if( NULL != pPIHM )
		{
			dMinCloseOffDp = pPIHM->GetHAvail( ( m_pParent == pPIHM ) ? CAnchorPt::CircuitSide_Primary : CAnchorPt::CircuitSide_Secondary );
		}

		double dCloseOffDp = -1.0;

		if( NULL != pCloseOffChar && CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
		{
			dCloseOffDp = pCloseOffChar->GetCloseOffDp( pclActuator->GetMaxForceTorque() );
		}

		if( dCloseOffDp >= dMinCloseOffDp )
		{
			return true;
		}
	}

	return false;
}

int CDS_HydroMod::CCv::GetCOCCompliantActuatorList( std::multimap< double, CDB_Actuator *> *pActrList, bool bWorkingForASet )
{
	if( NULL != pActrList )
	{
		pActrList->clear();
	}

	if( false == IsTaCV() )
	{
		return 0;
	}

	CDB_ControlValve *pCV = GetpCV();

	if( NULL == pCV )
	{
		return 0;
	}

	// CV exist try to find some compliant actuators
	CDB_CloseOffChar *pCloseOffChar = static_cast< CDB_CloseOffChar * >( pCV->GetCloseOffCharIDPtr().MP );
	CDS_HydroMod *pPIHM = m_pParent->GetpPressIntHM();
	double dMinCloseOffDp = 0;

	if( NULL != pPIHM )
	{
		dMinCloseOffDp = pPIHM->GetHAvail( ( m_pParent == pPIHM ) ? CAnchorPt::CircuitSide_Primary : CAnchorPt::CircuitSide_Secondary );
	}

	std::multimap< double, CDB_Actuator *> mapActr;
	CRank rkList( false );

	if( GetpTADB()->GetActuator( &rkList, pCV, bWorkingForASet, CTADatabase::FilterSelection::ForHMCalc ) )
	{
		CString str;
		LPARAM itemdata = 0;

		for( BOOL cont = rkList.GetFirst( str, itemdata ); TRUE == cont; cont = rkList.GetNext( str, itemdata ) )
		{
			CDB_Actuator *pActr = dynamic_cast<CDB_Actuator *>( (CData *) itemdata );

			if( NULL != pActr )
			{
				double dCloseOffDp = -1.0;

				if( NULL != pCloseOffChar && CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
				{
					dCloseOffDp = pCloseOffChar->GetCloseOffDp( pActr->GetMaxForceTorque() );
				}

				if( dCloseOffDp >= dMinCloseOffDp )
				{
					double iPos = pActr->GetPriorityLevel() * 10000000 + dCloseOffDp ;
					ASSERT( dCloseOffDp <= 10000000 );
					mapActr.insert( std::pair<double, CDB_Actuator *> ( iPos, pActr ) );
				}
			}
		}

		// Not found keep strongest
		if( mapActr.size() == 0 )
		{
			double dMaxCloseOffDp = -1;

			for( BOOL cont = rkList.GetFirst( str, itemdata ); TRUE == cont; cont = rkList.GetNext( str, itemdata ) )
			{
				CDB_Actuator *pActr = dynamic_cast<CDB_Actuator *>( (CData *) itemdata );

				if( NULL != pActr )
				{
					double dCloseOffDp = -1;

					if( NULL != pCloseOffChar && CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
					{
						dCloseOffDp = pCloseOffChar->GetCloseOffDp( pActr->GetMaxForceTorque() );
					}

					if( dCloseOffDp == dMaxCloseOffDp )
					{
						mapActr.insert( std::pair<double, CDB_Actuator *> ( dCloseOffDp, pActr ) );
					}

					if( dCloseOffDp > dMaxCloseOffDp )
					{
						mapActr.clear();
						dMaxCloseOffDp = dCloseOffDp;
						mapActr.insert( std::pair<double, CDB_Actuator *> ( dCloseOffDp, pActr ) );
					}
				}
			}
		}
	}

	rkList.PurgeAll();

	if( NULL != pActrList )
	{
		*pActrList = mapActr;
	}

	return mapActr.size();
}

CDB_ControlValve *CDS_HydroMod::CCv::GetpCV()
{
	if( false == IsTaCV() )
	{
		return NULL;
	}

	CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( GetCvIDPtr().MP );
	return pCV;
}

bool CDS_HydroMod::CCv::IsOn_Off()
{
	return ( m_eControlType == CDB_ControlProperties::CvCtrlType::eCvOnOff );
}

eBool3 CDS_HydroMod::CCv::CheckDpLRange( CDB_DpCBCValve *pDpCBCValve )
{
	if( NULL == pDpCBCValve )
	{
		pDpCBCValve = static_cast< CDB_DpCBCValve * >( GetCvIDPtr().MP );
	}

	if( NULL == pDpCBCValve )
	{
		return eb3Undef;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( eb3Undef );
	}

	double dRho = pclWaterChar->GetDens();
	double dDplmin = pDpCBCValve->GetDplmin();
	double dDplmax = pDpCBCValve->GetDplmax( m_dQ, dRho );

	if( -1.0 == dDplmin || -1.0 == dDplmax )
	{
		return eb3Undef;
	}

	double dDpl = GetDpToStab();

	if( dDpl < dDplmin || dDpl > dDplmax )
	{
		return eb3False;
	}

	return eb3True;
}

double CDS_HydroMod::CCv::GetDpToStab()
{
	if( 0.0 == GetQ() || NULL == GetCvIDPtr().MP || NULL == dynamic_cast<CDB_DpCBCValve *>( GetCvIDPtr().MP ) )
	{
		return 0.0;
	}

	CDB_DpCBCValve *pclDpCBCValve = dynamic_cast<CDB_DpCBCValve *>( GetCvIDPtr().MP );
	CDB_DpCBCVCharacteristic *pclDpCBCValveCharacteristic = pclDpCBCValve->GetDpCBCVCharacteristic();

	if( NULL == pclDpCBCValveCharacteristic )
	{
		return 0.0;
	}

	double dDpStabilize = pclDpCBCValveCharacteristic->GetDpk( GetQ() );

	if( -1.0 == dDpStabilize )
	{
		dDpStabilize = 0.0;
	}

	return dDpStabilize;
}

void CDS_HydroMod::CCv::ClearUserChoiceTemporaryVariables()
{
	m_pUserChoiceCVConn = NULL;
	m_pUserChoiceCVVers = NULL;
	m_pUserChoiceCVPN = NULL;
}

int CDS_HydroMod::CCv::GetAvailableVersions( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapCV.size() )
	{
		return 0;
	}

	pmap->clear();

	CDB_StringID *pStrIDConnection = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDConnection = dynamic_cast<CDB_StringID *>( GetpTAP()->GetConnectIDPtr().MP );
	}

	std::multimap <int, CCVData >::iterator It;

	for( It = m_mapCV.begin(); It != m_mapCV.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpCV();
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );

		if( pStrID != pStrIDConnection )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetVersionIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

int CDS_HydroMod::CCv::GetAvailableConnections( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapCV.size() )
	{
		return 0;
	}

	pmap->clear();
	std::multimap <int, CCVData >::iterator It;

	for( It = m_mapCV.begin(); It != m_mapCV.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpCV();
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		(*pmap)[iKey] = pStrID;
	}

	return pmap->size();
}

int CDS_HydroMod::CCv::GetAvailablePN( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapCV.size() )
	{
		return 0;
	}

	pmap->clear();
	CDB_StringID *pStrIDConnection = NULL;
	CDB_StringID *pStrIDVersion = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDConnection = dynamic_cast<CDB_StringID *>( GetpTAP()->GetConnectIDPtr().MP );
		pStrIDVersion = dynamic_cast<CDB_StringID *>( GetpTAP()->GetVersionIDPtr().MP );
	}

	std::multimap <int, CCVData >::iterator It;

	for( It = m_mapCV.begin(); It != m_mapCV.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpCV();
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );

		if( pStrID != pStrIDConnection )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetVersionIDPtr().MP );

		if( pStrID != pStrIDVersion )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetPNIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

void CDS_HydroMod::CCv::_BuildSortingKeys( CTAPSortKey *psKeyTechParamBlw65, CTAPSortKey *psKeyTechParamAbv50 )
{
	// Remark: Normally, when it's a PICV or a DpCBCV (TA-COMPACT-DP), we don't arrive in this method.

	CDB_StringID *psidType = NULL;
	CDB_StringID *psidFam = NULL;
	CDB_StringID *psidBdy = NULL;
	CDB_StringID *psidConn = NULL;
	CDB_StringID *psidVers = NULL;
	CDB_StringID *psidPN = NULL;

	CPrjParams *pPrjParams = m_pParent->GetpPrjParam()->GetpHmCalcParams();

	// Tech Param below DN65
	if( eb3True == IsPICV() )
	{
		// Pressure independent control valve.
		ASSERT( 0 );
		psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvBlw65TypeID ).MP );
		psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvBlw65FamilyID ).MP );
		psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvBlw65BdyMatID ).MP );
		psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvBlw65ConnectID ).MP );
		psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvBlw65VersionID ).MP );
		psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvBlw65PNID ).MP );
		psKeyTechParamBlw65->Init( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );
	}
	else if( eb3True == IsDpCBCV() )
	{
		// Combined Dp controller, control and balancing valve.
		ASSERT( 0 );
		psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65TypeID ).MP );
		psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65FamilyID ).MP );
		psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65BdyMatID ).MP );
		psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65ConnectID ).MP );
		psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65VersionID ).MP );
		psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65PNID ).MP );
		psKeyTechParamBlw65->Init( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );
	}
	else if( false == IsPresettable() )
	{
		// Standard control valve.
		psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvBlw65TypeID ).MP );
		psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvBlw65FamilyID ).MP );
		psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvBlw65BdyMatID ).MP );
		psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvBlw65ConnectID ).MP );
		psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvBlw65VersionID ).MP );
		psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvBlw65PNID ).MP );
		psKeyTechParamBlw65->Init( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );
	}
	else
	{
		// Balancing and control valve.
		psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BCvBlw65TypeID ).MP );
		psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BCvBlw65FamilyID ).MP );
		psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BCvBlw65BdyMatID ).MP );
		psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BCvBlw65ConnectID ).MP );
		psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BCvBlw65VersionID ).MP );
		psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BCvBlw65PNID ).MP );
		psKeyTechParamBlw65->Init( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );
	}


	// Tech Param above DN50
	if( eb3True == IsPICV() )
	{
		// Pressure independent control valve.
		ASSERT( 0 );
		psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvAbv50TypeID ).MP );
		psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvAbv50FamilyID ).MP );
		psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvAbv50BdyMatID ).MP );
		psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvAbv50ConnectID ).MP );
		psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvAbv50VersionID ).MP );
		psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvAbv50PNID ).MP );
		psKeyTechParamAbv50->Init( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );
	}
	else if( eb3True == IsDpCBCV() )
	{
		// Combined Dp controller, control and balancing valve.
		ASSERT( 0 );
		psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50TypeID ).MP );
		psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50FamilyID ).MP );
		psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50BdyMatID ).MP );
		psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50ConnectID ).MP );
		psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50VersionID ).MP );
		psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveAbv50PNID ).MP );
		psKeyTechParamAbv50->Init( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );
	}
	else if( false == IsPresettable() )
	{
		// Standard control valve.
		psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvAbv50TypeID ).MP );
		psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvAbv50FamilyID ).MP );
		psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvAbv50BdyMatID ).MP );
		psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvAbv50ConnectID ).MP );
		psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvAbv50VersionID ).MP );
		psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvAbv50PNID ).MP );
		psKeyTechParamAbv50->Init( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );
	}
	else
	{
		// Balancing and control valve.
		psidType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BCvAbv50TypeID ).MP );
		psidFam = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BCvAbv50FamilyID ).MP );
		psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BCvAbv50BdyMatID ).MP );
		psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BCvAbv50ConnectID ).MP );
		psidVers = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BCvAbv50VersionID ).MP );
		psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BCvAbv50PNID ).MP );
		psKeyTechParamAbv50->Init( psidType, NULL, psidFam, psidBdy, psidConn, psidVers, psidPN, 0 );
	}

}

double CDS_HydroMod::CCv::_SelectBestTACV( double dTargetKvs, bool fFromPump )
{
	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( -1.0 );
	}

	int iHighestSize = 0;
	int iLowestSize = 0;
	double dFlow = GetQ();
	double dRho = pclWaterChar->GetDens();
	double dNu = pclWaterChar->GetKinVisc();

	CDB_CircuitScheme *pSch = m_pParent->GetpSch();
	CDB_CircSchemeCateg *pSchcat = m_pParent->GetpSchcat();

	if( NULL == pSch || NULL == pSchcat )
	{
		return -1.0;
	}

	if( dFlow <= 0 )
	{
		// HYS-1753: Clean the selection when de flow is not set
		InvalidateSelection();
		return -1.0;
	}

	CAnchorPt::CircuitSide eCircuitSide;

	if( eBool3::eb3True == IsCVLocInPrimary() )
	{
		eCircuitSide = CAnchorPt::CircuitSide_Primary;
	}
	else if( eBool3::eb3False == IsCVLocInPrimary() )
	{
		eCircuitSide = CAnchorPt::CircuitSide_Secondary;
	}
	else
	{
		InvalidateSelection();
		ASSERTA_RETURN( -1.0 );
	}

	CAnchorPt::PipeLocation ePipeLocation = m_pParent->GetpSch()->GetAnchorPtPipeLocation( CAnchorPt::eFunc::ControlValve );
	CDB_Pipe *pPipe = m_pParent->GetPipeSizeShift( iHighestSize, iLowestSize, eCircuitSide, ePipeLocation );

	if( NULL == pPipe )
	{
		return -1.0;
	}

	int iDN50 = m_pParent->GetDN50();
	CPrjParams *pPrjParams = m_pParent->GetpPrjParam()->GetpHmCalcParams();

	// By default use the same Product Type as selected product
	CDB_StringID *pCVType = GetpSelCVType();

	if( NULL == pCVType )
	{
		// Nothing selected yet
		// Return to pre-selection done in Tech params.
		if( pPipe->GetSizeKey( TASApp.GetpTADB() ) <= iDN50 )
		{
			if( eb3True == IsPICV() )
			{
				// Pressure independent control valve.
				pCVType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvBlw65TypeID ).MP );
			}
			else if( eb3True == IsDpCBCV() )
			{
				// Combined Dp controller, control and balancing valve.
				pCVType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65TypeID ).MP );
			}
			else if( false == IsPresettable() )
			{
				// Standard control valve.
				pCVType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvBlw65TypeID ).MP );
			}
			else
			{
				// Balancing and control valve.
				pCVType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BCvBlw65TypeID ).MP );
			}
		}
		else
		{
			if( eb3True == IsPICV() )
			{
				// Pressure independent control valve.
				pCVType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::PICvBlw65TypeID ).MP );
			}
			else if( eb3True == IsDpCBCV() )
			{
				// Combined Dp controller, control and balancing valve.
				pCVType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::DpCBCValveBlw65TypeID ).MP );
			}
			else if( false == IsPresettable() )
			{
				// Standard control valve.
				pCVType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvAbv50TypeID ).MP );
			}
			else
			{
				// Balancing and control valve.
				pCVType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BCvBlw65TypeID ).MP );
			}
		}
	}

	bool bForSet = false;

	if( eb3Undef == GetCVSelectedAsaPackage() )
	{
		// HYS-1209: If we don't have the 'm_fCVSelectedAsaPackage' variable set, we set by default to the same value as the one
		// in the technical parameters.
		// Remark: for control valve and actuator obviously we use the same variable in the technical parameters.
		bForSet = m_pParent->GetpPrjParam()->GetpHmCalcParams()->GetActuatorSelectedByPackage();
		SetCVSelectedAsaPackage( bForSet == true ? eb3True : eb3False );
	}

	// Establish CV list.
	CRankEx List;
	int iNum = GetpTADB()->GetTaCVList( &List, CTADatabase::eForHMCv, false, pSchcat->Get2W3W(), pCVType->GetIDPtr().ID, _T(""), _T(""), _T(""), _T(""), 
			_T(""), pSch->GetCvFunc(), m_pParent->GetCvCtrlType(), CTADatabase::FilterSelection::ForHMCalc, 0, INT_MAX, false, NULL, bForSet );

	if( 0 == iNum )
	{
		if( true == bForSet )
		{
			// If we have done a search for set, we cant try now without set.
			bForSet = false;
			SetCVSelectedAsaPackage( eb3False );

			iNum = GetpTADB()->GetTaCVList( &List, CTADatabase::eForHMCv, false, pSchcat->Get2W3W(), pCVType->GetIDPtr().ID, _T(""), _T(""), _T(""), 
					_T(""), _T(""), pSch->GetCvFunc(), m_pParent->GetCvCtrlType(), CTADatabase::FilterSelection::ForHMCalc, 0, INT_MAX, false, NULL, bForSet );

			if( 0 == iNum )
			{
				// We can search in this case set but without type.
				bForSet = true;
				SetCVSelectedAsaPackage( eb3True );

				iNum = GetpTADB()->GetTaCVList( &List, CTADatabase::eForHMCv, false, pSchcat->Get2W3W(), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), 
						pSch->GetCvFunc(), m_pParent->GetCvCtrlType(), CTADatabase::FilterSelection::ForHMCalc, 0, INT_MAX, false, NULL, bForSet );

				if( 0 == iNum )
				{
					bForSet = false;
					SetCVSelectedAsaPackage( eb3False );
					// We can search in this case set but without type,and without set.

					iNum = GetpTADB()->GetTaCVList( &List, CTADatabase::eForHMCv, false, pSchcat->Get2W3W(), _T(""), _T(""), _T(""), _T(""),
							_T(""), _T(""), pSch->GetCvFunc(), m_pParent->GetCvCtrlType(), CTADatabase::FilterSelection::ForHMCalc, 0, INT_MAX, false, NULL, bForSet );
				}
			}
		}
		else
		{
			// If the search was done without set, we enlarge research by cleaning type.
			iNum = GetpTADB()->GetTaCVList( &List, CTADatabase::eForHMCv, false, pSchcat->Get2W3W(), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""),
					pSch->GetCvFunc(), m_pParent->GetCvCtrlType(), CTADatabase::FilterSelection::ForHMCalc );
		}
	}

	if( iNum <= 0 )
	{
		SetCVSelectedAsaPackage( eb3Undef );
		return -1.0;
	}

	// Extract valves according size-shift, keep at least one product size, valves are ordered by size
	LPARAM lparam;
	_string str;
	m_mapCV.clear();
	std::multimap <int, CCVData >::iterator itMapCV;
	std::multimap <int, CCVData >::reverse_iterator ritMapCV;

	for( BOOL bContinue = List.GetFirst( str, lparam ); TRUE == bContinue; bContinue = List.GetNext( str, lparam ) )
	{
		CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( (CData *) lparam );

		if( NULL == pCV )
		{
			continue;
		}

		int iProdSizeKey = pCV->GetSizeKey();
		m_mapCV.insert( std::make_pair( iProdSizeKey, CCVData( pCV ) ) );
	}

	if( 0 == m_mapCV.size() )
	{
		return -1.0;
	}

	// HYS-1308: Change the way we reject sizes.
	std::multimap <int, CCVData>::iterator itLow = m_mapCV.end();
	std::multimap <int, CCVData>::iterator itUp = m_mapCV.end();

	for( std::multimap <int, CCVData>::iterator iter = m_mapCV.begin(); iter != m_mapCV.end(); iter++ )
	{
		if( iter->first >= iLowestSize )
		{
			itLow = iter;
			break;
		}
	}

	if( m_mapCV.end() == itLow )
	{
		itLow = m_mapCV.lower_bound( ( *m_mapCV.rbegin() ).first );
	}

	bool bFound = false;

	for( std::multimap <int, CCVData>::reverse_iterator riter = m_mapCV.rbegin(); riter != m_mapCV.rend(); riter++ )
	{
		if( riter->first <= iHighestSize )
		{
			itUp = riter.base();
			bFound = true;
			break;
		}
	}

	if( m_mapCV.end() == itUp && false == bFound )
	{
		itUp = m_mapCV.upper_bound( (*m_mapCV.begin()).first );
	}

	if( m_mapCV.end() != itLow && m_mapCV.begin() != itLow )
	{
		m_mapCV.erase( m_mapCV.begin(), itLow );
	}

	if( m_mapCV.end() != itUp )
	{
		m_mapCV.erase( itUp, m_mapCV.end() );
	}

	if( 0 == m_mapCV.size() )
	{
		return -1.0;
	}

	struct structCvData
	{
		CDB_TAProduct *pclProduct;
		bool bMinRecSettingOK;
		// HYS-1760: When we return at the end of this function a Kvs different than target kvs,
		// we need to save the setting for valves with viscosity correction because that is what we need
		// to define the new Dp.
		double dH; 
	};

	typedef std::multimap<double, structCvData> _TMapDelta;
	_TMapDelta mapBestDeltaKvs;
	_TMapDelta mapPoorDeltaKvs;

	bool bAtLeastOneMinRecSettingOKInPoorList = false;
	double dMaxDp = m_pParent->GetpTechParam()->GetCVMaxDpProp();
	double dValveMinDp = m_pParent->GetTechParamCVMinDpProp() ;
	bool b3wMixing = ( pSchcat != NULL && true == pSchcat->Is3W() && CDB_CircSchemeCateg::e3wTypeMixing == pSchcat->Get3WType() );

	if( true == IsOn_Off() || true == b3wMixing )
	{
		dValveMinDp = m_pParent->GetpTechParam()->GetCVMinDpOnoff();
	}

	if( eb3True == IsPICV() || eb3True == IsDpCBCV() )
	{
		// Pressure independent control valve and combined Dp controller, control and balancing valve.
		ASSERTA_RETURN( -1.0 );
	}
	else if( false == IsPresettable() )
	{
		// Standard control valve.
		// Valve has been filtered according size shift; try to find best CV.
		// Best CV is the valve with a Kvs equal and below request Kvs value.
		// We will build two maps:
		//   1/ contains smallest Kvs values.
		//   2/ contains biggest Kvs values.
		// Valves with a Dp above Dpmax are marked to decrease their priority.
		for( itMapCV = m_mapCV.begin(); itMapCV != m_mapCV.end(); ++itMapCV )
		{
			CDB_ControlValve *pCV = itMapCV->second.GetpCV();
			double dKvs = pCV->GetKvs();
			double dDp = CalcDp( dFlow, dKvs, dRho );
			itMapCV->second.SetDp( dDp );
			double dDelta = abs( dTargetKvs - dKvs );

			if( dDp > dMaxDp )
			{
				// Mark delta value with a constant value that report this selection at the end of the list
				dDelta += 10000;
			}

			structCvData rCvData;
			rCvData.pclProduct = pCV;
			rCvData.dH = -1; // HYS-1760: only for valves with viscosity correction
			rCvData.bMinRecSettingOK = true;		// No presettable thus 'true' by default.
			( dTargetKvs >= dKvs ) ? mapBestDeltaKvs.insert( std::make_pair( dDelta, rCvData ) ) : mapPoorDeltaKvs.insert( std::make_pair( dDelta, rCvData ) );
		}
	}
	else
	{
		// Balancing and control valve.
		// Best valve is the smallest valve with the lowest Dpmin.
		// When we are working with valve with independent characteristics or lift limitation, we will take a bigger valve to have the possibility to close it.
		for( itMapCV = m_mapCV.begin(); itMapCV != m_mapCV.end(); ++itMapCV )
		{
			CDB_ControlValve *pCV = itMapCV->second.GetpCV();
			double dDp = 0.0;
			double dH = -1;
			double dKvs = pCV->GetKvs();
			// HYS-1760: For characteristic with viscosity correction use the corrected values for Dp, Kv, opening.
			if( NULL != pCV && NULL != pCV->GetValveCharacteristic() && true == pCV->GetValveCharacteristic()->IsViscCorrExist() )
			{
				// Get max opening of current valve
				double dHfo = 0.0;
				double dDpfo = 0.0;
				dHfo = pCV->GetValveCharacteristic()->GetOpeningMax();
				pCV->GetValveCharacteristic()->GetValveDp( dFlow, &dDpfo, dHfo, dRho, dNu );
				dDp = dDpfo;
				dH = dHfo;

				dKvs = pCV->GetValveCharacteristic()->GetKvTst( dH );
			}
			else
			{
				dDp = CalcDp( dFlow, dKvs, dRho );
				pCV->GetValveCharacteristic()->GetOpening( dTargetKvs, &dH );
			}
			itMapCV->second.SetDp( dDp );
			// No security factor for TBV-CM
			double dTAFUSIONSecurityFactor = 1;
			bool bTAFusion = false;

			if(	CDB_ControlProperties::eBCVType::ebcvtCharIndep == pCV->GetCtrlProp()->GetBCVType() ||
				CDB_ControlProperties::eBCVType::ebcvtLiftLimit == pCV->GetCtrlProp()->GetBCVType() )
			{
				// TA-FUSION
				// When valve at full opening is below ..10kPa don't target a smaller valve
				dTAFUSIONSecurityFactor = ( dDp <= dValveMinDp ) ? 1 : 0.8;
				bTAFusion = true;
			}

			double dDelta = 0;
			double dKvsCorrected = dKvs * dTAFUSIONSecurityFactor;
			dDelta = abs( dTargetKvs - dKvsCorrected );

			if( dDp > dMaxDp )
			{
				// Mark delta value with a constant value that report this selection at the end of the list
				dDelta += 10000;
			}

			structCvData rCvData;
			rCvData.pclProduct = pCV;
			rCvData.dH = dH;
			if( true == bTAFusion )
			{
				// TA Fusion we target a CV with a Kvs above the TargetKvs
				// We can close TA-Fusion to reach target value
				if( dKvsCorrected >= dTargetKvs )
				{
					// Verify if settings is above minimum recommended setting
					CDB_ValveCharacteristic *pChar = pCV->GetValveCharacteristic();
					double dh;
					pChar->GetOpening( dTargetKvs, &dh );

					if( -1 != dh )
					{
						if( dh < pChar->GetMinRecSetting() || dDp > dMaxDp )
						{
							rCvData.bMinRecSettingOK = false;
							mapPoorDeltaKvs.insert( std::make_pair( dDelta, rCvData ) );
						}
						else
						{
							rCvData.bMinRecSettingOK = true;
							mapBestDeltaKvs.insert( std::make_pair( dDelta, rCvData ) );
						}
					}
				}
				else
				{
					// Setting is OK because here we are at full opening.
					rCvData.bMinRecSettingOK = true;
					bAtLeastOneMinRecSettingOKInPoorList = true;
					mapPoorDeltaKvs.insert( std::make_pair( dDelta, rCvData ) );
				}
			}
			else
			{
				// Setting is OK because here we are at full opening.
				rCvData.bMinRecSettingOK = true;

				if( true == fFromPump )
				{
					// We accept to close a little bit TBV-CM
					dKvsCorrected *= 0.8;
				}

				// TBV-CM, we try to find the first valve with a Kvs smaller than target Kvs
				// TBV-CM should satisfy to the CV max Dp parameters coming from technical parameters.
				// Double computation is different in release and in debug, we accept that dKvsCorrected will be a bit greater than TargetKvs
				// typically we get dKvsCorrected = 4 and dTargetKvs = 3.9999999999999996
				if( ( dKvsCorrected - dTargetKvs ) <= 0.0001 && dDp < dMaxDp )
				{
					mapBestDeltaKvs.insert( std::make_pair( dDelta, rCvData ) );
				}
				else
				{
					bAtLeastOneMinRecSettingOKInPoorList = true;
					mapPoorDeltaKvs.insert( std::make_pair( dDelta, rCvData ) );
				}
			}
		}
	}

	if( true == bAtLeastOneMinRecSettingOKInPoorList && 0 == ( int )mapBestDeltaKvs.size() && 0 != ( int )mapPoorDeltaKvs.size() )
	{
		// If we have no product in the best list but well in the poor one AND if at least one product in this poor list has
		// a good setting (> min. recommended setting) thus we exclude other products that have bad setting.
		// Why? For example user can force a TBV-CM 25 -> this valve is thus locked. When unlock, imagine this circuit must resize
		// the control valve for a target Kvs = 2.9. TBV-CM have a Kvs = 4 and thus are put in the poor list (because we take in this case
		// only valve that has a lower Kvs than the target). TA-FUSION-C have a bigger Kvs (12.9) but in this case we can close the valve
		// to reach the wanted Kvs. Thus TA-FUSION-C are set in the best list. And this finally a TA-FUSION-C that is selected.
		// In the last step of the computation (with the '_DistributeHAvailable' method), we pass on this circuit and ask to resize the
		// control valve with a Kvs = 2.35. TBV-CM are always set in the poor list. And now, TA-FUSION-C to reach the wanted Kvs must have
		// a setting that is below the recommended one. Thus these ones are also put in the poor list. Because just before the computation
		// set a TA-FUSION-C, we search in the poor list the valve that has strictly the same criterion. TBV-CM are rejected and it is the
		// TA-FUSION-C that are selected even if the setting is very low.
		// This is why here we remove valves that have a low setting if others in the list exist with a good setting.

		for( _TMapDelta::iterator iter = mapPoorDeltaKvs.begin(); iter != mapPoorDeltaKvs.end(); )
		{
			if( false == iter->second.bMinRecSettingOK )
			{
				_TMapDelta::iterator nextiter = iter;
				nextiter++;
				mapPoorDeltaKvs.erase( iter );
				iter = nextiter;
			}
			else
			{
				iter++;
			}
		}
	}

	bFound = false;
	_TMapDelta::iterator It = mapBestDeltaKvs.end();
	int iCurrentSize = -1;
	CDB_TAProduct *pCurrentTAP = GetpTAP();

	// When user change the current type, version, connection , PN, ... the current selected product is invalid
	// Try to find matching (connection, version, ...) valve in the full list of suggested products
	bool bUserChangeSelParameters = false;

	if( NULL != m_pUserChoiceCVVers || NULL != m_pUserChoiceCVConn || NULL != m_pUserChoiceCVPN )
	{
		bUserChangeSelParameters = true;
		pCurrentTAP = NULL;
	}

	if( NULL != pCurrentTAP )
	{
		iCurrentSize = pCurrentTAP->GetSizeKey();
	}

	// Build sorting keys.
	CTAPSortKey sKeyUserChangeSelParams;
	CTAPSortKey sKeyTechParamBlw65;
	CTAPSortKey sKeyTechParamAbv50;
	_BuildSortingKeys( &sKeyTechParamBlw65, &sKeyTechParamAbv50 );

	double dKeyCurrentTAP = 0;
	int strtLoop = ( true == bUserChangeSelParameters ) ? strtLoop = -1 : strtLoop = 0;
	int iEndLoop = 6;

	for( int iLoop = 0; iLoop < 2 && false == bFound; iLoop++ )
	{
		// This loop give us the possibility to analyze two maps.
		// First Opening map, valves are big enough to satisfy requested Dp.
		// second Dp map, valves are too small.
		_TMapDelta *pMap = NULL;

		switch( iLoop )
		{
			case 0:
			{
				// Process mapDeltaOpeningTap
				// analyze mapDeltaOPeningTAP, that contains all valves that can satisfy requested Dp
				pMap = 	&mapBestDeltaKvs;
			}
			break;

			case 1:
			{
				// Process mapDeltaDpFOTAP
				// Valve are too small to satisfy requested Dp
				pMap = 	&mapPoorDeltaKvs;
			}
			break;
		}

		if( pMap->size() > 0 )
		{
			for( int UserPrefLoop = strtLoop; UserPrefLoop < iEndLoop && false == bFound; UserPrefLoop++ )
			{
				// This loop give us the possibility to relax mask criteria one by one
				// we will start with the most constraining key and if we don't found a product we will relax constrains one by one
				CTAPSortKey::TAPSortKeyMask eMask = ( CTAPSortKey::TAPSortKeyMask )( CTAPSortKey::TAPSKM_All & ~CTAPSortKey::TAPSKM_PriorL & ~CTAPSortKey::TAPSKM_Type &
													~CTAPSortKey::TAPSKM_Size );

				switch( UserPrefLoop )
				{
					case -1:
					{
						// ( true == bUserChangeSelParameters )
						// Body material is not available as a user choice (combo doesn't exist)
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Fam & ~CTAPSortKey::TAPSKM_Bdy );

						if( NULL == m_pUserChoiceCVPN )
						{
							eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
						}

						if( NULL == m_pUserChoiceCVVers )
						{
							eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Version );
						}

						if( NULL == m_pUserChoiceCVConn )
						{
							eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Connect );
						}

						sKeyUserChangeSelParams.Init( pCVType, NULL, NULL, NULL, m_pUserChoiceCVConn, m_pUserChoiceCVVers, m_pUserChoiceCVPN, 0 );
					}
					break;

					case 0:
					{
						// All
						if( true == bUserChangeSelParameters )
						{
							// No solution found relax user request
							bUserChangeSelParameters = false;
							ClearUserChoiceTemporaryVariables();
						}
					}
					break;

					case 1:
					{
						// Relax PN
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
					}
					break;

					case 2:
					{
						// Relax PN and Version.
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version );
					}
					break;

					case 3:
					{
						// Relax PN, version and connection.
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version & ~CTAPSortKey::TAPSKM_Connect );
					}
					break;

					case 4:
					{
						// Relax PN, version, connection and body material.
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version & ~CTAPSortKey::TAPSKM_Connect &
								~CTAPSortKey::TAPSKM_Bdy );
					}
					break;

					case 5:
					{
						// Relax PN, version, connection, body material and family.
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Version & ~CTAPSortKey::TAPSKM_Connect &
								~CTAPSortKey::TAPSKM_Bdy & ~CTAPSortKey::TAPSKM_Fam );
					}
					break;
				}

				double dKeyTechParamBlw65 = sKeyTechParamBlw65.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
				double dKeyTechParamAbv50 = sKeyTechParamAbv50.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
				double dKeyUserChangeSelParams = sKeyUserChangeSelParams.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );

				// Extract key for currently selected TAP.
				if( NULL != pCurrentTAP )
				{
					dKeyCurrentTAP = pCurrentTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
				}

				if( true == bUserChangeSelParameters )
				{
					_TMapDelta::iterator ItBestPriority = pMap->end();

					for( It = pMap->begin(); It != pMap->end(); ++It )
					{
						CDB_TAProduct *pTAP = It->second.pclProduct;
						double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );

						if( dTAPKey == dKeyUserChangeSelParams )
						{
							if( ItBestPriority != pMap->end() )
							{
								if( ItBestPriority->second.pclProduct->GetPriorityLevel() > pTAP->GetPriorityLevel() )
								{
									ItBestPriority = It;
									bFound = true;
								}
							}
							else
							{
								ItBestPriority = It;
								bFound = true;
							}
						}
					}

					if( true == bFound )
					{
						It = ItBestPriority;
					}
				}
				else
				{
					// If a current selection exist.
					if( NULL != pCurrentTAP )
					{
						for( It = pMap->begin(); It != pMap->end(); ++It )
						{
							CDB_TAProduct *pTAP = It->second.pclProduct;
							int iSize = pTAP->GetSizeKey();
							double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );

							if( ( iCurrentSize <= iDN50 && iSize <= iDN50 ) || ( iCurrentSize > iDN50 && iSize > iDN50 ) )
							{
								// Selected valve exist and it's located on the same side of iDN50 border.
								// Try to find a valve that match all requested criteria.
								if( dTAPKey == dKeyCurrentTAP )
								{
									bFound = true;
								}
							}
							else
							{
								// Selected valve is not located on the same side of iDN50 border.
								// Use defined technical choice
								if( iSize <= iDN50 )
								{
									if( dTAPKey == dKeyTechParamBlw65 )
									{
										bFound = true;
									}
								}
								else
								{
									// Size > DN50
									if( dTAPKey == dKeyTechParamAbv50 )
									{
										bFound = true;
									}
								}
							}

							if( true == bFound )
							{
								// Abort loop by this way we preserve Iterator
								break;
							}
						}
					}

					// Not found or previous valve doesn't exist, restart loop only on tech params.
					if( false == bFound )
					{
						for( It =  pMap->begin(); It !=  pMap->end(); ++It )
						{
							CDB_TAProduct *pTAP = It->second.pclProduct;
							int iSize = pTAP->GetSizeKey();
							double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );

							// Selected valve is not located on the same side of iDN50 border.
							// Use defined technical choice.
							if( iSize <= iDN50 )
							{
								if( dTAPKey == dKeyTechParamBlw65 )
								{
									bFound = true;
								}
							}
							else
							{
								// Size > DN50
								if( dTAPKey == dKeyTechParamAbv50 )
								{
									bFound = true;
								}
							}

							if( true == bFound )
							{
								// Abort loop and by this way we preserve Iterator
								break;
							}
						}
					}
				}
			}
		}
	}

	// If best valve found...
	if( true == bFound )
	{
		CDB_TAProduct *pTAP = It->second.pclProduct;

		if( NULL != pTAP )
		{
			if( true == bUserChangeSelParameters )
			{
				// When the current product is valid we will analyze if the user introduced an exception regarding defined PrjParam,
				// in this case we will not consider the DN50 border that force an automatic jump.
				if( pTAP->GetSizeKey() <= iDN50 )
				{
					if( ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvBlw65ConnectID ).MP == pTAP->GetConnectIDPtr().MP ) &&
						( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvBlw65VersionID ).MP == pTAP->GetVersionIDPtr().MP ) &&
						( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvBlw65PNID ).MP == pTAP->GetPNIDPtr().MP ) )
					{
						ClearUserChoiceTemporaryVariables();
					}
				}
				else
				{
					// DN65 and above
					if( ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvAbv50ConnectID ).MP == pTAP->GetConnectIDPtr().MP ) &&
						( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvAbv50VersionID ).MP == pTAP->GetVersionIDPtr().MP ) &&
						( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::CvAbv50PNID ).MP == pTAP->GetPNIDPtr().MP ) )
					{
						ClearUserChoiceTemporaryVariables();
					}
				}
			}

			SetBestCvIDPtr( pTAP->GetIDPtr() );
			CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( pTAP );

			if( NULL != pCV )
			{
				// HYS-2077: Set m_bCVSelectedAsaPackage 
				SetCVSelectedAsaPackage( ( true == pCV->IsPartOfaSet() ) ? eb3True : eb3False );
				CDB_ValveCharacteristic *pChar = pCV->GetValveCharacteristic();
				if( NULL != pChar && true == pChar->IsViscCorrExist() && true == IsPresettable() )
				{
					// HYS-1760: Save the setting to retrieve the corrected Dp.
					SetSetting( It->second.dH );
				}

				if( pCV->GetKvs() > dTargetKvs )
				{
					// TA-FUSION are selected with a bigger Kvs than target value (we can't do it with other control valve).
					// In that case, to reach the 'dTargetKvs' we need to close the valve. This is why we have to save the
					// new opening. Remark: when going to the pump by default all valves are set to full opening.

					if( NULL != pChar )
					{
						double dh;
						if( true == pChar->IsViscCorrExist() && true == IsPresettable() )
						{
							dh = It->second.dH;
						}
						else
						{
							pChar->GetOpening( dTargetKvs, &dh );
						}
						SetSetting( dh );
					}

					return dTargetKvs;
				}
				else
				{
					// Here we found a valve that have a Kvs below the target. That means the Dp is bigger than the wanted
					// one. When going to the pump the opening is set to the max value. We don't need to change it.

					return pCV->GetKvs();
				}
			}
		}
		else
		{
			InvalidateSelection();
			ClearUserChoiceTemporaryVariables();
		}
	}
	else
	{
		InvalidateSelection();
		ClearUserChoiceTemporaryVariables();
	}

	return -1.0;
}

void CDS_HydroMod::CCv::_VerifyCVActrAccSet( void )
{
	if( true == m_bCVActrAccSetRefreshed )
	{
		return;
	}

	m_ArCVActrAccSetIDPtr.RemoveAll();

	CDB_ControlValve *pclControlValve = GetpCV();

	if( NULL == pclControlValve )
	{
		return;
	}

	CDB_Actuator *pclActuator = static_cast< CDB_Actuator * >( GetActrIDPtr().MP );

	if( NULL == pclActuator )
	{
		return;
	}

	CTableSet *pCVActSetTab = pclControlValve->GetTableSet();
	ASSERT( NULL != pCVActSetTab );

	if( NULL == pCVActSetTab )
	{
		return;
	}

	CDB_Set *pCVActSet = pCVActSetTab->FindCompatibleSet( pclControlValve->GetIDPtr().ID, pclActuator->GetIDPtr().ID );

	if( NULL != pCVActSet && _NULL_IDPTR != pCVActSet->GetAccGroupIDPtr() )
	{
		std::vector<CData *> vecData;
		CDB_RuledTable *pclRuledTable = static_cast< CDB_RuledTable * >( pCVActSet->GetAccGroupIDPtr().MP );

		if( NULL == pclRuledTable )
		{
			return;
		}

		int iCount = pclRuledTable->GetBuiltIn( &vecData );

		if( iCount > 0 )
		{
			for( int i = 0; i < iCount; i++ )
			{
				AddCVActrAccSetIDPtr( vecData[i]->GetIDPtr() );
			}
		}
	}

	m_bCVActrAccSetRefreshed = true;
}

double CDS_HydroMod::CCv::_SizeControlValveForBetaMinHMwithDpConPrimary( double dHAvailAtPI )
{
	// This method is called only by the 'SizeControlValveForBetaMin' method when the differential pressure 
	// on the control valve is stabilized by a Dp controller.
	//
	// The control valve will be sized with the differential pressure that it takes and this one 
	// is protected by a Dp controller. So, we will obviously take the minimum differential pressure that is possible.
	// This differential pressure will be either the minimum differential pressure (Dplmin) that the Dp controller can
	// stabilize. Or, if Dplmin is too low, we will take the minimum differential pressure that the control valve needs 
	// to work with the current flow.

	// By default we take the lowest Dplmin for all Dp controllers contained in the map of CDS_HydroMod::CDpC.
	double dLowestKvs = 0.0;
	double dHighestKvs = 0.0;
	double dLowestDpl = 0.0;
	double dHighestDpl = 0.0;
	bool bDpCExist = m_pParent->GetpDpC()->GetKvsBounds( &dLowestKvs, &dHighestKvs, &dLowestDpl, &dHighestDpl );

	// If all Dp controller is already selected, we take its Dplmin.
	CDB_DpController *pDpC = static_cast<CDB_DpController *>( m_pParent->GetpDpC()->GetIDPtr().MP );

	if( NULL != pDpC && -1.0 != pDpC->GetDplmin() && -1.0 != pDpC->GetDplmax() )
	{
		// For Dpl, if a Dp controller is already selected, we take limit from it.
		dLowestDpl = pDpC->GetDplmin();
	}

	// Dp on control valve is the maximum between the Dplmin of the Dp controller and the minimum differential pressure that 
	// the control valve needs to work with the current flow.
	double dDpCV = __max( dLowestDpl, GetDpMin() );
	bool bFromPump = ( dHAvailAtPI > 0.0 ) ? true : false;
	double dKvs = _SizeKvs( dDpCV, bFromPump );
	SetKvsmax( dKvs );

	// DpC is localized on a BVC, we have to adjust BVC to satisfy DpL and minimum authority
	if( true == IsPresettable() )
	{
		if( dDpCV < dLowestDpl )
		{
			dDpCV = dLowestDpl;
		}

		ResizePresetCV( dDpCV, false );
	}

	m_pParent->EnableComputeHM( true );
	return dHAvailAtPI;
}

double CDS_HydroMod::CCv::_SizeKvs( double &dDp, bool fFromPump )
{
	// Remark: never called for PICV and DpCBCV valves.

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	double dRho = pclWaterChar->GetDens();
	double dKvs = 0.0;
	double dQ = GetQ();

	if( 0.0 == dQ )
	{
		return 0.0;
	}

	dKvs = CalcKv( dQ, dDp, dRho );

	// Correct Kvs with Reynard series.
	if( false == IsTaCV() )
	{
		if( m_pParent->GetpTechParam()->GetCVDefKvReynard() )
		{
			dKvs = GetReynardVal( dKvs );
			dDp = CalcDp( dQ, dKvs, dRho );
			double dMaxDp = m_pParent->GetpTechParam()->GetCVMaxDpProp();

			if( dDp > dMaxDp )
			{
				dKvs = CalcKv( dQ, dMaxDp, dRho );
				dKvs = GetReynardVal( dKvs, true );
			}
		}

		SetBestKvs( dKvs );

		if( false == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eCV ) || m_dKvs <= 0 )
		{
			dDp = CalcDp( dQ, dKvs, dRho );
			m_dKvs = dKvs;
			m_pParent->Modified();
		}
	}
	else
	{
		CDB_RegulatingValve* pRv = dynamic_cast<CDB_RegulatingValve*>(GetCvIDPtr().MP);
		dKvs = _SelectBestTACV( dKvs, fFromPump );
		SetBestKvs( dKvs );

		if( false == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
		{
			double d = 0.0;
			// HYS-1760: For characteristic with viscosity correction use the corrected values for Dp, Kv, opening.
			if( NULL != pRv && NULL != pRv->GetValveCharacteristic() && true == pRv->GetValveCharacteristic()->IsViscCorrExist() && true == IsPresettable() )
			{
				double dNu = pclWaterChar->GetKinVisc();
				if( false == pRv->GetValveCharacteristic()->GetValveDp( dQ, &d, GetSetting(), dRho, dNu ) )
				{
					d = 0;
				}
			}
			else
			{
				d = CalcDp( dQ, dKvs, dRho );
			}
			// OnOff valves are not changed in descending phase if the new requested Dp is biggest than available H.
			if( true == IsOn_Off() && true == fFromPump )
			{
				if( d < dDp )
				{
					dDp = d;
					SetpSelCV( (LPARAM)GetBestCvIDPtr().MP );
				}
			}
			else
			{
				dDp = d;
				SetpSelCV( (LPARAM)GetBestCvIDPtr().MP );
			}
		}
	}

	return dKvs;
}

//////////////////////////////////////////////////////////////////////
// 6-way valve.
//////////////////////////////////////////////////////////////////////
CDS_HydroMod::C6WayValve::C6WayValve( CDS_HydroMod *pHM ) : CBase( pHM )
{
	m_IDPtr6WayValve = _NULL_IDPTR;
	m_IDPtr6WayValveActuator = _NULL_IDPTR;
	m_IDPtr6WayValveActuatorAdapter = _NULL_IDPTR;

	m_Ar6WayValveAccessoryIDPtr.SetSize( 0 );				// IDPTRs for selected 6-way valve accessories.
	m_Ar6WayValveActuatorAccessoryIDPtr.SetSize( 0 );		// IDPTRs for selected 6-way valve  actuator accessories.
}

void CDS_HydroMod::C6WayValve::Copy( CDS_HydroMod::C6WayValve *pHM6WayValve, bool bResetDiversityFactor )
{
	pHM6WayValve->Set6WayValveIDPtr( Get6WayValveIDPtr() );
	pHM6WayValve->Set6WayValveActuatorIDPtr( Get6WayValveActuatorIDPtr() );
	pHM6WayValve->Set6WayValveActuatorAdapterIDPtr( Get6WayValveActuatorAdapterIDPtr() );

	for (int i = 0; i < m_Ar6WayValveAccessoryIDPtr.GetSize(); i++)
	{
		pHM6WayValve->Add6WayValveAccessoryIDPtr( m_Ar6WayValveAccessoryIDPtr[i] );
	}

	for (int i = 0; i < m_Ar6WayValveActuatorAccessoryIDPtr.GetSize(); i++)
	{
		pHM6WayValve->Add6WayValveActuatorAccessoryIDPtr( m_Ar6WayValveActuatorAccessoryIDPtr[i] );
	}
}

void CDS_HydroMod::C6WayValve::Add6WayValveAccessoryIDPtr( IDPTR IDPtr )
{
	m_Ar6WayValveAccessoryIDPtr.Add( IDPtr );
	m_pParent->Modified();
}

void CDS_HydroMod::C6WayValve::Add6WayValveActuatorAccessoryIDPtr( IDPTR IDPtr )
{
	m_Ar6WayValveActuatorAccessoryIDPtr.Add( IDPtr );
	m_pParent->Modified();
}

IDPTR CDS_HydroMod::C6WayValve::Get6WayValveIDPtr()
{
	if( _T('\0') != *m_IDPtr6WayValve.ID )
	{
		m_IDPtr6WayValve.DB = TASApp.GetpTADB();
		Extend( &m_IDPtr6WayValve );
		return m_IDPtr6WayValve;
	}

	return _NULL_IDPTR;
}

IDPTR CDS_HydroMod::C6WayValve::Get6WayValveActuatorIDPtr()
{
	if( _T('\0') != *m_IDPtr6WayValveActuator.ID && NULL != m_IDPtr6WayValveActuator.DB )
	{
		Extend( &m_IDPtr6WayValveActuator );
	}

	return m_IDPtr6WayValveActuator;
}

IDPTR CDS_HydroMod::C6WayValve::Get6WayValveActuatorAdapterIDPtr()
{
	if( _T('\0') != *m_IDPtr6WayValveActuatorAdapter.ID && NULL != m_IDPtr6WayValveActuatorAdapter.DB )
	{
		Extend( &m_IDPtr6WayValveActuatorAdapter );
	}

	return m_IDPtr6WayValveActuatorAdapter;
}

void CDS_HydroMod::C6WayValve::Set6WayValveIDPtr( IDPTR IDPtr )
{
	m_IDPtr6WayValve = IDPtr;

	if( _T('\0') != *m_IDPtr6WayValve.ID && NULL != m_IDPtr6WayValve.DB )
	{
		Extend( &m_IDPtr6WayValve );
	}

	m_pParent->Modified();
}

void CDS_HydroMod::C6WayValve::Set6WayValveActuatorIDPtr( IDPTR IDPtr )
{
	m_IDPtr6WayValveActuator = IDPtr;
	m_pParent->Modified();
}

void CDS_HydroMod::C6WayValve::Set6WayValveActuatorAdapterIDPtr( IDPTR IDPtr )
{
	m_IDPtr6WayValveActuatorAdapter = IDPtr;
	m_pParent->Modified();
}

IDPTR CDS_HydroMod::C6WayValve::Get6WayValveAccessoryIDPtr( int iIndex )
{
	if( iIndex < m_Ar6WayValveAccessoryIDPtr.GetSize() )
	{
		if( _T('\0') != *m_Ar6WayValveAccessoryIDPtr[iIndex].ID && NULL != m_Ar6WayValveAccessoryIDPtr[iIndex].DB )
		{
			Extend( &m_Ar6WayValveAccessoryIDPtr[iIndex] );
		}

		return m_Ar6WayValveAccessoryIDPtr[iIndex];
	}

	return _NULL_IDPTR;
}

IDPTR CDS_HydroMod::C6WayValve::Get6WayValveActuatorAccessoryIDPtr( int iIndex )
{
	if( iIndex < m_Ar6WayValveActuatorAccessoryIDPtr.GetSize() )
	{
		if( _T('\0') != *m_Ar6WayValveActuatorAccessoryIDPtr[iIndex].ID && NULL != m_Ar6WayValveActuatorAccessoryIDPtr[iIndex].DB )
		{
			Extend( &m_Ar6WayValveActuatorAccessoryIDPtr[iIndex] );
		}

		return m_Ar6WayValveActuatorAccessoryIDPtr[iIndex];
	}

	return _NULL_IDPTR;
}

#define CDS_HYDROMOD_C6WAYVALVE_VERSION		1
void CDS_HydroMod::C6WayValve::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_C6WAYVALVE_VERSION;
	WriteData<>( outf, Version );

	// Info.
	WriteString( outf, (LPCTSTR) m_IDPtr6WayValve.ID );
	WriteString( outf, (LPCTSTR) m_IDPtr6WayValveActuator.ID );
	WriteString( outf, (LPCTSTR) m_IDPtr6WayValveActuatorAdapter.ID );
}

bool CDS_HydroMod::C6WayValve::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_C6WAYVALVE_VERSION;
	ReadData<>( inpf, Version );

	if( Version < 1 || Version > CDS_HYDROMOD_C6WAYVALVE_VERSION )
	{
		return false;
	}

	ReadString( inpf, m_IDPtr6WayValve.ID, sizeof( m_IDPtr6WayValve.ID ) );
	return true;
}

//////////////////////////////////////////////////////////////////////
// Smart control valve.
//////////////////////////////////////////////////////////////////////
CDS_HydroMod::CSmartControlValve::CSmartControlValve( CDS_HydroMod *pHM ) : CBase( pHM )
{
	m_eLocalization = SmartValveLocalization::SmartValveLocSupply;
	m_eControlMode = SmartValveControlMode::SCVCM_Flow;
	SetIDPtr( _NULL_IDPTR );
	SetBestIDPtr( _NULL_IDPTR );
	SetDiversityIDPtr( _NULL_IDPTR );
	ClearUserChoiceTemporaryVariables();
	m_eUserChoiceLocalization = SmartValveLocalization::SmartValveLocNone;
	m_dQ = 0.0;
	m_dPowerMax = 0.0;
	m_dDp = 0.0;						// HYS-1914: Dp is equal to Dp min except if Havail at the entry of the circuit increases, in that case it's the Dp that takes the difference.
	m_ArAccessoryIDPtr.SetSize( 0 );	// IDPTRs for selected smart control valve accessories.
	m_usValidityFlags = 0;
	m_mapSmartControlValve.clear();
	m_mapErrorMessages.clear();
}

void CDS_HydroMod::CSmartControlValve::Copy( CDS_HydroMod::CSmartControlValve *pHMTargetHMSmartControlValve, bool bResetDiversityFactor )
{
	pHMTargetHMSmartControlValve->SetLocalization( m_eLocalization );
	pHMTargetHMSmartControlValve->SetControlMode( m_eControlMode );
	pHMTargetHMSmartControlValve->SetIDPtr( GetIDPtr() );
	pHMTargetHMSmartControlValve->SetBestIDPtr( GetBestIDPtr() );
	pHMTargetHMSmartControlValve->SetDiversityIDPtr( ( true == bResetDiversityFactor ) ? _NULL_IDPTR : GetDiversityIDPtr() );
	pHMTargetHMSmartControlValve->SetQ( GetQ() );
	pHMTargetHMSmartControlValve->SetPowerMax( GetPowerMax() );
	pHMTargetHMSmartControlValve->SetDp( GetDp() );
	pHMTargetHMSmartControlValve->SetValidtyFlags( GetValidtyFlags() );

	if( NULL == m_pUserChoiceBodyMaterial && NULL == m_pUserChoiceConnection && NULL == m_pUserChoicePN )
	{
		pHMTargetHMSmartControlValve->ClearUserChoiceTemporaryVariables();
	}
	else
	{
		pHMTargetHMSmartControlValve->SetpSelBodyMaterialAccordingUserChoice( m_pUserChoiceBodyMaterial );
		pHMTargetHMSmartControlValve->SetpSelConnectionAccordingUserChoice( m_pUserChoiceConnection );
		pHMTargetHMSmartControlValve->SetpSelPNAccordingUserChoice( m_pUserChoicePN );
	}

	if( SmartValveLocalization::SmartValveLocNone != m_eUserChoiceLocalization )
	{
		pHMTargetHMSmartControlValve->SetSelLocalizationUserChoice( m_eUserChoiceLocalization );
	}

	for (int i = 0; i < m_ArAccessoryIDPtr.GetSize(); i++)
	{
		pHMTargetHMSmartControlValve->AddSmartControlValveAccessoryIDPtr( m_ArAccessoryIDPtr[i] );
	}
}

bool CDS_HydroMod::CSmartControlValve::IsSmartControlValveExistInPreselectedList( CDB_TAProduct *pclSmartControlValve )
{
	if( 0 == (int)m_mapSmartControlValve.size() )
	{
		return false;
	}

	bool bSmartControlValveExist = false;

	for( auto &iter : m_mapSmartControlValve )
	{
		if( pclSmartControlValve == iter.second.GetpSmartControlValve() )
		{
			bSmartControlValveExist = true;
			break;
		}
	}

	return bSmartControlValveExist;
}

std::vector<CString> CDS_HydroMod::CSmartControlValve::GetErrorMessageList( CDS_HydroMod::ErrorWarningMessageFlags eErrorWarningMessageFlags )
{
	std::vector<CString> vecAllMessages;

	if( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Temp == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Temp )
			|| CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All ) )
	{
		if( m_mapErrorMessages.count( CDS_HydroMod::eValidityFlags::evfTempTooHigh ) > 0 )
		{
			vecAllMessages.push_back( m_mapErrorMessages.at( CDS_HydroMod::eValidityFlags::evfTempTooHigh ) );
		}
		
		if( m_mapErrorMessages.count( CDS_HydroMod::eValidityFlags::evfTempTooLow ) > 0 )
		{
			vecAllMessages.push_back( m_mapErrorMessages.at( CDS_HydroMod::eValidityFlags::evfTempTooLow ) );
		}
	}

	if( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Flow == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Flow )
			|| CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All ) )
	{
		if( m_mapErrorMessages.count( CDS_HydroMod::eValidityFlags::evfFlowTooLow ) > 0 )
		{
			vecAllMessages.push_back( m_mapErrorMessages.at( CDS_HydroMod::eValidityFlags::evfFlowTooLow ) );
		}
		
		if( m_mapErrorMessages.count( CDS_HydroMod::eValidityFlags::evfFlowTooHigh ) > 0 )
		{
			vecAllMessages.push_back( m_mapErrorMessages.at( CDS_HydroMod::eValidityFlags::evfFlowTooHigh ) );
		}
	}

	if( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dp == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dp )
			|| CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All ) )
	{
		if( m_mapErrorMessages.count( CDS_HydroMod::eValidityFlags::evfDpTooHigh ) > 0 )
		{
			vecAllMessages.push_back( m_mapErrorMessages.at( CDS_HydroMod::eValidityFlags::evfDpTooHigh ) );
		}
		
		if( m_mapErrorMessages.count( CDS_HydroMod::eValidityFlags::evfDpTooLow ) > 0 )
		{
			vecAllMessages.push_back( m_mapErrorMessages.at( CDS_HydroMod::eValidityFlags::evfDpTooLow ) );
		}
	}

	if( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Power == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Power )
			|| CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All ) )
	{
		if( m_mapErrorMessages.count( CDS_HydroMod::eValidityFlags::evfPowerTooLow ) > 0 )
		{
			vecAllMessages.push_back( m_mapErrorMessages.at( CDS_HydroMod::eValidityFlags::evfPowerTooLow ) );
		}
		
		if( m_mapErrorMessages.count( CDS_HydroMod::eValidityFlags::evfPowerTooHigh ) > 0 )
		{
			vecAllMessages.push_back( m_mapErrorMessages.at( CDS_HydroMod::eValidityFlags::evfPowerTooHigh ) );
		}
	}

	return vecAllMessages;
}

CString CDS_HydroMod::CSmartControlValve::GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags eErrorWarningMessageFlags, CString strPrefix, bool bReturnLine )
{
	return ConvertVecStringToString( GetErrorMessageList( eErrorWarningMessageFlags ), strPrefix, bReturnLine );
}

void CDS_HydroMod::CSmartControlValve::AddSmartControlValveAccessoryIDPtr( IDPTR IDPtr )
{
	m_ArAccessoryIDPtr.Add( IDPtr );
	m_pParent->Modified();
}

IDPTR CDS_HydroMod::CSmartControlValve::GetIDPtr()
{
	if( _T('\0') != *m_IDPtr.ID )
	{
		m_IDPtr.DB = TASApp.GetpTADB();
		Extend( &m_IDPtr );
		return m_IDPtr;
	}

	return _NULL_IDPTR;
}

bool CDS_HydroMod::CSmartControlValve::IsBestSmartControlValve( CData *pData )
{
	return ( pData == GetBestIDPtr().MP ) ? true : false;
}

IDPTR CDS_HydroMod::CSmartControlValve::GetBestIDPtr()
{
	if( _T('\0') != *m_IDPtrBest.ID )
	{
		m_IDPtrBest.DB = TASApp.GetpTADB();
		Extend( &m_IDPtrBest );
		return m_IDPtrBest;
	}

	return _NULL_IDPTR;
}

void CDS_HydroMod::CSmartControlValve::SetIDPtr( IDPTR IDPtr )
{
	m_IDPtr = IDPtr;

	if( _T('\0') != *m_IDPtr.ID && NULL != m_IDPtr.DB )
	{
		Extend( &m_IDPtr );
	}

	m_pParent->Modified();
}

IDPTR CDS_HydroMod::CSmartControlValve::GetAccessoryIDPtr( int iIndex )
{
	if( iIndex < m_ArAccessoryIDPtr.GetSize() )
	{
		if( _T('\0') != *m_ArAccessoryIDPtr[iIndex].ID && NULL != m_ArAccessoryIDPtr[iIndex].DB )
		{
			Extend( &m_ArAccessoryIDPtr[iIndex] );
		}

		return m_ArAccessoryIDPtr[iIndex];
	}

	return _NULL_IDPTR;
}

bool CDS_HydroMod::CSmartControlValve::FillInfoIntoHMX( CDS_HydroModX *pHMX, eHMObj HMObj )
{
	if( eHMObj::eSmartControlValve != HMObj )
	{
		return false;
	}

	// Test if a valve exist (TAProduct) for the localization.
	if( NULL == GetIDPtr().MP )
	{
		return false;
	}

	CDS_HydroModX::CSmartControlValve *pclSmartControlValve = pHMX->GetpSmartControlValve();

	// Design flow.
	pclSmartControlValve->SetValveID( GetIDPtr().ID );
	pclSmartControlValve->SetDesignFlow( GetQ() );

	return true;
}

bool CDS_HydroMod::CSmartControlValve::ForceSelection( IDPTR DeviceIDptr )
{
	if( true == m_mapSmartControlValve.empty() )
	{
		// Unlock the valve to force filling of 'm_mapSmartControlValve'.
		m_pParent->SetLock( CDS_HydroMod::eHMObj::eSmartControlValve, false );
	}

	// Lock valve if new selected valve doesn't correspond to the Best valve idptr
	m_pParent->SetLock( CDS_HydroMod::eHMObj::eSmartControlValve, !IsBestSmartControlValve( (CData *)DeviceIDptr.MP ) );

	// Forced valve should be inside 'm_mapSmartControlValve' as a valid choice;
	_SetpSelSmartControlValve( (LPARAM)DeviceIDptr.MP, true );
	ClearUserChoiceTemporaryVariables();

	return ( GetIDPtr().MP == DeviceIDptr.MP );
}

int CDS_HydroMod::CSmartControlValve::CheckValidity()
{
	CString str;
	m_usValidityFlags = CDS_HydroMod::eValidityFlags::evfOK;
	m_mapErrorMessages.clear();

	if( NULL == GetIDPtr().MP )
	{
		return m_usValidityFlags;
	}

	CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( GetIDPtr().MP );

	if( NULL == pclSmartControlValve )
	{
		return m_usValidityFlags;
	}

	// Check the pressure drop accross the valve.
	if( GetDp() > pclSmartControlValve->GetDpmax() )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDpTooHigh;

		// The pressure drop accross the valve is higher that the maximum allowed (%1 > %2).
		CString strDp = WriteCUDouble( _U_DIFFPRESS, GetDp() );
		CString strMaxDp = WriteCUDouble( _U_DIFFPRESS, pclSmartControlValve->GetDpmax(), true );
		FormatString( str, IDS_HYDROMOD_SMARTVALVEDPTOOHIGH, strDp, strMaxDp );
		m_mapErrorMessages.insert( std::pair<CDS_HydroMod::eValidityFlags, CString>( CDS_HydroMod::eValidityFlags::evfDpTooHigh, str ) );
	}

	// Check the flow.
	if( GetFlowMax() > pclSmartControlValve->GetQNom() ) 
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfFlowTooHigh;

		// The flow is higher that the maximum allowed (%1 > %2).
		CString strFlow = WriteCUDouble( _U_FLOW, GetFlowMax() );
		CString strMaxFlow = WriteCUDouble( _U_FLOW, pclSmartControlValve->GetQNom(), true );
		FormatString( str, IDS_HYDROMOD_SMARTVALVEFLOWTOOHIGH, strFlow, strMaxFlow );
		m_mapErrorMessages.insert( std::pair<CDS_HydroMod::eValidityFlags, CString>( CDS_HydroMod::eValidityFlags::evfFlowTooHigh, str ) );
	}

	if( GetFlowMax() < pclSmartControlValve->GetMinAdjustableFlow() ) 
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfFlowTooLow;

		// The flow is lower that the minimum allowed (%1 < %2).
		CString strFlow = WriteCUDouble( _U_FLOW, GetFlowMax() );
		CString strMinFlow = WriteCUDouble( _U_FLOW, pclSmartControlValve->GetMinAdjustableFlow(), true );
		FormatString( str, IDS_HYDROMOD_SMARTVALVEFLOWTOOLOW, strFlow, strMinFlow );
		m_mapErrorMessages.insert( std::pair<CDS_HydroMod::eValidityFlags, CString>( CDS_HydroMod::eValidityFlags::evfFlowTooLow, str ) );
	}

	// Check power.
	if( SmartValveControlMode::SCVCM_Power == m_eControlMode )
	{
		if( GetPowerMax() > pclSmartControlValve->GetPNom() ) 
		{
			m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfPowerTooHigh;

			// The power is higher that the maximum allowed (%1 > %2).
			CString strPower = WriteCUDouble( _U_TH_POWER, GetPowerMax() );
			CString strMaxPower = WriteCUDouble( _U_TH_POWER, pclSmartControlValve->GetPNom(), true );
			FormatString( str, IDS_HYDROMOD_SMARTVALVEPOWERTOOHIGH, strPower, strMaxPower );
			m_mapErrorMessages.insert( std::pair<CDS_HydroMod::eValidityFlags, CString>( CDS_HydroMod::eValidityFlags::evfPowerTooHigh, str ) );
		}

		if( GetPowerMax() < pclSmartControlValve->GetMinAdjustablePower() ) 
		{
			m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfPowerTooLow;

			// The power is lower that the minimum allowed (%1 < %2).
			CString strPower = WriteCUDouble( _U_TH_POWER, GetPowerMax() );
			CString strMinPower = WriteCUDouble( _U_TH_POWER, pclSmartControlValve->GetMinAdjustablePower(), true );
			FormatString( str, IDS_HYDROMOD_SMARTVALVEPOWERTOOLOW, strPower, strMinPower );
			m_mapErrorMessages.insert( std::pair<CDS_HydroMod::eValidityFlags, CString>( CDS_HydroMod::eValidityFlags::evfPowerTooLow, str ) );
		}
	}

	// Check temperatures
	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eSmartControlValve );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( m_usValidityFlags );
	}

	if( pclWaterChar->GetTemp() > pclSmartControlValve->GetTmax() )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfTempTooHigh;

		// The fluid temperature is higher that the maximum allowed for %1 (%2 > %3).
		CString strProductName = pclSmartControlValve->GetName();
		CString strTemp = WriteCUDouble( _U_TEMPERATURE, pclWaterChar->GetTemp() );
		CString strMaxTemp = WriteCUDouble( _U_TEMPERATURE, pclSmartControlValve->GetTmax(), true );
		FormatString( str, IDS_HYDROMOD_TEMPTOOHIGHFORPRODUCT, strProductName, strTemp, strMaxTemp );
		m_mapErrorMessages.insert( std::pair<CDS_HydroMod::eValidityFlags, CString>( CDS_HydroMod::eValidityFlags::evfTempTooHigh, str ) );
	}

	if( pclWaterChar->GetTemp() < pclSmartControlValve->GetTmin() )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfTempTooLow;

		// The fluid temperature is lower that the minimum allowed for %1 (%2 < %3).
		CString strProductName = pclSmartControlValve->GetName();
		CString strTemp = WriteCUDouble( _U_TEMPERATURE, pclWaterChar->GetTemp() );
		CString strMinTemp = WriteCUDouble( _U_TEMPERATURE, pclSmartControlValve->GetTmin(), true );
		FormatString( str, IDS_HYDROMOD_TEMPTOOLOWFORPRODUCT, strProductName, strTemp, strMinTemp );
		m_mapErrorMessages.insert( std::pair<CDS_HydroMod::eValidityFlags, CString>( CDS_HydroMod::eValidityFlags::evfTempTooLow, str ) );
	}

	return m_usValidityFlags;
}

void CDS_HydroMod::CSmartControlValve::DropData( OUTSTREAM outf, int *piLevel )
{
	CString strTab = _T("");

	for( int i = *piLevel; i; i-- )
	{
		strTab += _T("\t");
	}

	CString str;
	str.Format( _T("m_IDPtr.ID = %s\n"), GetIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_IDPtrBest.ID = %s\n"), GetBestIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );

	if( SmartValveControlMode::SCVCM_Flow == m_eControlMode )
	{
		str.Format( _T("m_Q = %s\n"), WriteCUDouble( _U_FLOW, GetQ(), true ) );
	}
	else if( SmartValveControlMode::SCVCM_Power == m_eControlMode )
	{
		str.Format( _T("m_Power = %s\n"), WriteCUDouble( _U_FLOW, GetPowerMax(), true ) );
	}
	
	WriteFormatedStringA2( outf, str, strTab );

	str = _T("m_Located = eSmartControlValve");
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_usValidityFlags = %u\n"), GetValidtyFlags() );
	WriteFormatedStringA2( outf, str, strTab );
	WriteFormatedStringA2( outf, _T("\n"), strTab );
}

void CDS_HydroMod::CSmartControlValve::ClearUserChoiceTemporaryVariables()
{
	m_pUserChoiceBodyMaterial = NULL;
	m_pUserChoiceConnection = NULL;
	m_pUserChoicePN = NULL;
}

void CDS_HydroMod::CSmartControlValve::_AbortSelectBest()
{
	if( false == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eSmartControlValve ) || 0 == *GetIDPtr().ID )
	{
		InvalidateSelection();
	}

	ClearUserChoiceTemporaryVariables();
}

void CDS_HydroMod::CSmartControlValve::_SetpSelSmartControlValve( LPARAM pTADBSmartControlValve, bool fForceLocker )
{
	if( false == fForceLocker )
	{
		if( true == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eSmartControlValve ) && 0 != *GetIDPtr().ID )
		{
			m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceSmartControlValve, (LPARAM)this );
			return;
		}
	}

	// Try to find product inside main valve map.
	std::multimap <int, CSmartControlValveData>::iterator itMapValves;

	for( itMapValves = m_mapSmartControlValve.begin(); m_mapSmartControlValve.end() != itMapValves; ++itMapValves )
	{
		if( pTADBSmartControlValve == (LPARAM)itMapValves->second.GetpSmartControlValve() )
		{
			break;
		}
	}

	if( m_mapSmartControlValve.end() != itMapValves )
	{
		SetIDPtr( itMapValves->second.GetpSmartControlValve()->GetIDPtr() );
		m_pParent->SetCBIValveID( GetIDPtr().ID );
	}
	else
	{
		InvalidateSelection();
	}

	m_pParent->Modified();
}

#define CDS_HYDROMOD_CSMARTCONTROLVALVE_VERSION		1
void CDS_HydroMod::CSmartControlValve::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_CSMARTCONTROLVALVE_VERSION;
	WriteData<>( outf, Version );

	// Info.
	WriteData<>( outf, m_eLocalization );
	WriteData<>( outf, m_eControlMode );
	WriteString( outf, (LPCTSTR) m_IDPtr.ID );
	WriteString( outf, m_IDPtrBest.ID );
	WriteData<>( outf, m_dQ );
	WriteData<>( outf, m_dPowerMax );
	WriteData<>( outf, GetDp() );
	WriteString( outf, m_DiversityIDPtr.ID );

	CString strID = ( NULL != m_pUserChoiceBodyMaterial ) ? m_pUserChoiceBodyMaterial->GetIDPtr().ID : _T("");
	WriteString( outf, strID );

	strID = ( NULL != m_pUserChoiceConnection ) ? m_pUserChoiceConnection->GetIDPtr().ID : _T("");
	WriteString( outf, strID );
	
	strID = ( NULL != m_pUserChoicePN ) ? m_pUserChoicePN->GetIDPtr().ID : _T("");
	WriteString( outf, strID );

	WriteData<>( outf, m_eUserChoiceLocalization );
}

bool CDS_HydroMod::CSmartControlValve::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_CSMARTCONTROLVALVE_VERSION;
	ReadData<>( inpf, Version );

	if( Version < 1 || Version > CDS_HYDROMOD_CSMARTCONTROLVALVE_VERSION )
	{
		return false;
	}

	ReadData<>( inpf, m_eLocalization );
	ReadData<>( inpf, m_eControlMode );
	ReadString( inpf, m_IDPtr.ID, sizeof( m_IDPtr.ID ) );
	ReadString( inpf, m_IDPtrBest.ID, sizeof( m_IDPtrBest.ID ) );
	ReadData<>( inpf, m_dQ );
	ReadData<>( inpf, m_dPowerMax );
	ReadData<>( inpf, m_dDp );
	m_IDPtr.DB = TASApp.GetpTADB();
	m_IDPtrBest.DB = TASApp.GetpTADB();

	ReadString( inpf, m_DiversityIDPtr.ID, sizeof( m_DiversityIDPtr.ID ) );

	ClearUserChoiceTemporaryVariables();

	IDPTR idptr;

	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceBodyMaterial = dynamic_cast<CDB_StringID *>( idptr.MP );

	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceConnection = dynamic_cast<CDB_StringID *>( idptr.MP );

	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoicePN = dynamic_cast<CDB_StringID *>( idptr.MP );

	ReadData<>( inpf, m_eUserChoiceLocalization );

	return true;
}

void CDS_HydroMod::CSmartControlValve::InvalidateSelection()
{
	if( true == GetpParentHM()->IsLocked( CDS_HydroMod::eHMObj::eSmartControlValve ) )
	{
		return;
	}

	m_IDPtr = _NULL_IDPTR;
	m_IDPtrBest = _NULL_IDPTR;
}

IDPTR CDS_HydroMod::CSmartControlValve::GetDiversityIDPtr()
{
	if( _T('\0') != *m_DiversityIDPtr.ID && NULL != m_DiversityIDPtr.DB )
	{
		Extend( &m_DiversityIDPtr );
		return m_DiversityIDPtr;
	}

	return _NULL_IDPTR;
}

void CDS_HydroMod::CSmartControlValve::SelectBestSmartControlValve()
{
	if( GetQ() <= 0.0 )
	{
		_AbortSelectBest();
		return;
	}

	if( _T('\0') != *GetIDPtr().ID && true == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eSmartControlValve ) )
	{
		_AbortSelectBest();
		return;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eSmartControlValve );
	
	if( NULL == pclWaterChar )
	{
		_AbortSelectBest();
		ASSERT_RETURN;
	}

	int iHighestSize = 0;
	int iLowestSize = 0;

	// Shut-Off valve selection is based on the pipe size.
	CAnchorPt::PipeLocation ePipeLocation = m_pParent->GetpSch()->GetAnchorPtPipeLocation( CAnchorPt::eFunc::SmartControlValve );
	CDB_Pipe *pPipe = m_pParent->GetPipeSizeShift( iHighestSize, iLowestSize, CAnchorPt::CircuitSide_Primary, ePipeLocation );

	if( NULL == pPipe )
	{
		_AbortSelectBest();
		return;
	}

	m_mapSmartControlValve.clear();
	std::multimap <int, CSmartControlValveData > mapSmartControlValve;
	std::multimap <int, CSmartControlValveData>::iterator itMapSmartControlValve;

	CRankEx List;
	GetpTADB()->GetSmartControlValveList( &List, L"SMARTCTRLVALVETYPE", L"", L"", L"", L"", L"", CTADatabase::FilterSelection::ForHMCalc );

	LPARAM lparam;
	_string str;

	for( BOOL bContinue = List.GetFirst( str, lparam ); TRUE == bContinue; bContinue = List.GetNext( str, lparam ) )
	{
		CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( (CData *)lparam );

		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		if( false == pclSmartControlValve->IsSelectable( true ) )
		{
			continue;
		}

		int iProdSizeKey = pclSmartControlValve->GetSizeKey();
		itMapSmartControlValve = mapSmartControlValve.insert( std::make_pair( iProdSizeKey, CSmartControlValveData( pclSmartControlValve ) ) );
	}

	if( 0 == mapSmartControlValve.size() )
	{
		_AbortSelectBest();
		return;
	}

	// HYS-1308: Change the way we reject sizes.
	std::multimap <int, CSmartControlValveData>::iterator itLow = mapSmartControlValve.end();
	std::multimap <int, CSmartControlValveData>::iterator itUp = mapSmartControlValve.end();

	for( std::multimap <int, CSmartControlValveData>::iterator iter = mapSmartControlValve.begin(); iter != mapSmartControlValve.end(); iter++ )
	{
		if( iter->first >= iLowestSize )
		{
			itLow = iter;
			break;
		}
	}

	if( mapSmartControlValve.end() == itLow )
	{
		itLow = mapSmartControlValve.lower_bound( ( *mapSmartControlValve.rbegin() ).first );
	}

	bool bFound = false;

	for( std::multimap <int, CSmartControlValveData>::reverse_iterator riter = mapSmartControlValve.rbegin(); riter != mapSmartControlValve.rend(); riter++ )
	{
		if( riter->first <= iHighestSize )
		{
			itUp = riter.base();
			bFound = true;
			break;
		}
	}

	if( mapSmartControlValve.end() == itUp && false == bFound )
	{
		itUp = mapSmartControlValve.upper_bound( (*mapSmartControlValve.begin()).first );
	}

	if( mapSmartControlValve.end() != itLow && mapSmartControlValve.begin() != itLow )
	{
		mapSmartControlValve.erase( mapSmartControlValve.begin(), itLow );
	}

	if( mapSmartControlValve.end() != itUp )
	{
		mapSmartControlValve.erase( itUp, mapSmartControlValve.end() );
	}

	if( 0 == mapSmartControlValve.size() )
	{
		_AbortSelectBest();
		return;
	}

	// The 'mapDelta' map will contain valves that are eligible (If flow is good (In flow control mode) or if power is good (In power control mode)).
	// The 'mapBadDelta' map will contain valves that are not eligible.
	// When a valve IS eligible, we insert it in the 'm_mapSmartControlValve' to be able to show all possibilities when opening the name combo box in 'CSheetHMCalc' or
	// 'CSheetPanelCirc2' dialog with the 'CDlgComboBoxHM' class.
	// When a valve IS NOT eligible, we insert it in the 'mapBadSmartControlValve'
	// If we have no valve that ARE eligible AND if we have valve that ARE NOT eligible, we will show these "solutions" with error.
	// In this case, we will transfer the contain of the 'mapBadSmartControlValve' map in the 'm_mapSmartControlValve' map.
	std::multimap<double, CDB_SmartControlValve *> mapDelta;
	std::multimap<double, CDB_SmartControlValve *> mapBadDelta;
	std::multimap<int, CSmartControlValveData> mapBadSmartControlValve;

	for( auto &iter : mapSmartControlValve ) 
	{
		CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( iter.second.GetpSmartControlValve() );

		if( NULL == pclSmartControlValve )
		{
			ASSERT_CONTINUE;
		}

		int iSizeKey = pclSmartControlValve->GetSizeKey();
		CDB_SmartValveCharacteristic *pclSmartValveCharacteristic = pclSmartControlValve->GetSmartValveCharacteristic();

		if( NULL == pclSmartValveCharacteristic )
		{
			ASSERT_CONTINUE;
		}

		bool bEligible = true;

		if( SmartValveControlMode::SCVCM_Flow == m_eControlMode )
		{
			if( GetQ() < pclSmartValveCharacteristic->GetMinAdjustableFlow() || GetQ() > pclSmartValveCharacteristic->GetQnom() )
			{
				bEligible = false;
			}
		}
		else if( SmartValveControlMode::SCVCM_Power == m_eControlMode )
		{
			if( GetPowerMax() < pclSmartValveCharacteristic->GetMinAdjustablePower() 
					|| GetPowerMax() > pclSmartValveCharacteristic->GetPowerNom()
					|| GetQ() > pclSmartValveCharacteristic->GetQnom() )
			{
				bEligible = false;
			}
		}

		double dDelta = 0.0;
		
		if( SmartValveControlMode::SCVCM_Flow == m_eControlMode )
		{
			dDelta = pclSmartValveCharacteristic->GetQnom() - GetQ();
		}
		else if( SmartValveControlMode::SCVCM_Power == m_eControlMode )
		{
			dDelta = pclSmartValveCharacteristic->GetPowerNom() - GetPowerMax();
		}

		if( true == bEligible )
		{
			mapDelta.insert( std::make_pair( dDelta, pclSmartControlValve ) );
			m_mapSmartControlValve.insert( std::pair<int, CSmartControlValveData>( iter.first, iter.second ) );
		}
		else
		{
			mapBadDelta.insert( std::make_pair( dDelta, pclSmartControlValve ) );
			mapBadSmartControlValve.insert( std::pair<int, CSmartControlValveData>( iter.first, iter.second ) );
		}

		// Add data.
		iter.second.SetQMax( GetQ() );
		iter.second.SetPowerMax( GetPowerMax() );
	}

	int iCurrentSize = -1;
	CDB_TAProduct *pCurrentTAP = GetpTAP();

	// When user change the current body material, connection or PN the current selected product is invalid.
	// Try to find matching (Body material, connection & PN) valve in the full list of suggested products.
	bool bUserChangeSelParameters = false;

	if( NULL != m_pUserChoiceBodyMaterial || NULL != m_pUserChoiceConnection || NULL != m_pUserChoicePN )
	{
		bUserChangeSelParameters = true;
		pCurrentTAP = NULL;
	}

	if( NULL != pCurrentTAP )
	{
		iCurrentSize = pCurrentTAP->GetSizeKey();
	}

	// Build sorting keys.
	// From technical parameters below DN65.
	CPrjParams *pPrjParams = m_pParent->GetpPrjParam()->GetpHmCalcParams();
	CDB_StringID *psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartControlValveBlw65BdyMatID ).MP );
	CDB_StringID *psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartControlValveBlw65ConnectID ).MP );
	CDB_StringID *psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartControlValveBlw65PNID ).MP );
	CTAPSortKey sKeyTechParamBlw65( NULL, NULL, NULL, psidBdy, psidConn, NULL, psidPN, 0 );

	CTAPSortKey sKeyUserChangeSelParamsBlw65( NULL, NULL, NULL, m_pUserChoiceBodyMaterial, m_pUserChoiceConnection, NULL, m_pUserChoicePN, 0 );

	// From technical parameters above DN50.
	psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartControlValveAbv50BdyMatID ).MP );
	psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartControlValveAbv50ConnectID ).MP );
	psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartControlValveAbv50PNID ).MP );
	CTAPSortKey sKeyTechParamAbv50( NULL, NULL, NULL, psidBdy, psidConn, NULL, psidPN, 0 );

	CTAPSortKey sKeyUserChangeSelParamsAbv50( NULL, NULL, NULL, m_pUserChoiceBodyMaterial, m_pUserChoiceConnection, NULL, m_pUserChoicePN, 0 );

	double dKeyCurrentTAP = 0;
	int strtLoop = ( true == bUserChangeSelParameters ) ? strtLoop = -2 : strtLoop = 0;
	int endLoop = 4;
	int iDN50 = m_pParent->GetDN50();
	bFound = false;
	std::multimap<double, CDB_SmartControlValve *>::iterator It = mapDelta.end();
	std::multimap<double, CDB_SmartControlValve *> *pmapDelta = &mapDelta;

	if( 0 == (int)mapDelta.size() && mapBadDelta.size() > 0 )
	{
		pmapDelta = &mapBadDelta;
		m_mapSmartControlValve = mapBadSmartControlValve;
	}

	if( pmapDelta->size() > 0 )
	{
		for( int UserPrefLoop = strtLoop; UserPrefLoop < endLoop && false == bFound; UserPrefLoop++ )
		{
			// This loop give us the possibility to relax mask criteria one by one.
			// we will start with the most constraining key and if we don't found a product we will relax constrains one by one.
			CTAPSortKey::TAPSortKeyMask eMask = ( CTAPSortKey::TAPSortKeyMask )( CTAPSortKey::TAPSKM_All & ~CTAPSortKey::TAPSKM_PriorL & ~CTAPSortKey::TAPSKM_Size
					& ~CTAPSortKey::TAPSKM_Type & ~CTAPSortKey::TAPSKM_Fam & ~CTAPSortKey::TAPSKM_Version );

			switch( UserPrefLoop )
			{
				case -2:
				{
					// We will try to find a matching product keeping family and body material as defined
					if( NULL == m_pUserChoicePN )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
					}

					if( NULL == m_pUserChoiceConnection )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Connect );
					}

					if( NULL == m_pUserChoiceBodyMaterial )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Bdy );
					}

				}
				break;

				case -1:
				{
					// ( true == bUserChangeSelParameters )
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Fam );
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Type );

					if( NULL == m_pUserChoicePN )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
					}

					if( NULL == m_pUserChoiceConnection )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Connect );
					}

					if( NULL == m_pUserChoiceBodyMaterial )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Bdy );
					}
				}
				break;

				case 0:
				{
					// All
					if( true == bUserChangeSelParameters )
					{
						// No solution found relax user request.
						bUserChangeSelParameters = false;
						ClearUserChoiceTemporaryVariables();
					}
				}
				break;

				// Relax PN.
				case 1:
					
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
					break;

				// Relax PN and connection.
				case 2:
					
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Connect );
					break;

				// Relax PN, connection and body material.
				case 3:
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Bdy );
					break;
			}

			double dKeyTechParamBlw65 = sKeyTechParamBlw65.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
			double dKeyTechParamAbv50 = sKeyTechParamAbv50.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
			double dKeyUserChangeSelParamsBlw65 = sKeyUserChangeSelParamsBlw65.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
			double dKeyUserChangeSelParamsAbv50 = sKeyUserChangeSelParamsAbv50.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );

			// Extract key for currently selected TAP
			if( NULL != pCurrentTAP )
			{
				dKeyCurrentTAP = pCurrentTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
			}

			if( true == bUserChangeSelParameters )
			{
				std::multimap<double, CDB_SmartControlValve *>::iterator iterBestPriority = pmapDelta->end();

				for( It = pmapDelta->begin(); It != pmapDelta->end(); It++ )
				{
					CDB_TAProduct *pTAP = It->second;
					int iSize = pTAP->GetSizeKey();
					double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
					double dKey = ( iSize <= iDN50 ) ? dKeyUserChangeSelParamsBlw65 : dKeyUserChangeSelParamsAbv50;

					if( dTAPKey == dKey )
					{
						if( iterBestPriority != pmapDelta->end() )
						{
							if( iterBestPriority->second->GetPriorityLevel() > pTAP->GetPriorityLevel() )
							{
								iterBestPriority = It;
								bFound = true;
							}
						}
						else
						{
							iterBestPriority = It;
							bFound = true;
						}
					}
				}

				if( true == bFound )
				{
					It = iterBestPriority;
				}
			}
			else
			{
				// If a current selection exist
				if( NULL != pCurrentTAP )
				{
					for( It = pmapDelta->begin(); It != pmapDelta->end(); ++It )
					{
						CDB_TAProduct *pTAP = It->second;
						int iSize = pTAP->GetSizeKey();
						double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );

						if( ( iCurrentSize <= iDN50 && iSize <= iDN50 ) || ( iCurrentSize > iDN50 && iSize > iDN50 ) )
						{
							// Selected valve exist and it's located on the same side of iDN50 border.
							// Try to find a valve that match all requested criteria.
							if( dTAPKey == dKeyCurrentTAP )
							{
								bFound = true;
							}
						}
						else
						{
							// Selected valve is not located on the same side of iDN50 border.
							// Use defined technical choice
							if( iSize <= iDN50 )
							{
								if( dTAPKey == dKeyTechParamBlw65 )
								{
									bFound = true;
								}
							}
							else
							{
								// Size > DN50
								if( dTAPKey == dKeyTechParamAbv50 )
								{
									bFound = true;
								}
							}
						}

						if( true == bFound )
						{
							// Abort loop by this way we preserve Iterator
							break;
						}
					}
				}

				// Not found or previous valve doesn't exist, restart loop only on tech params.
				if( false == bFound )
				{
					for( It = pmapDelta->begin(); It != pmapDelta->end(); ++It )
					{
						CDB_TAProduct *pTAP = It->second;
						int iSize = pTAP->GetSizeKey();
						double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );

						// Selected valve is not located on the same side of iDN50 border.
						// Use defined technical choice
						if( iSize <= iDN50 )
						{
							if( dTAPKey == dKeyTechParamBlw65 )
							{
								bFound = true;
							}
						}
						else
						{
							// Size > DN50
							if( dTAPKey == dKeyTechParamAbv50 )
							{
								bFound = true;
							}
						}

						if( true == bFound )
						{
							// Abort loop and by this way we preserve Iterator
							break;
						}
					}
				}
			}
		}
	}

	// If best valve found...
	if( true == bFound )
	{
		CDB_TAProduct *pTAP = It->second;

		if( NULL != pTAP )
		{
			SetBestIDPtr( pTAP->GetIDPtr() );
			_SetpSelSmartControlValve( (LPARAM)pTAP );

			if( true == bUserChangeSelParameters )
			{
				// When the current product is valid we will analyze if the user introduced an exception regarding defined PrjParam,
				// in this case we will not consider the DN50 border that force an automatic jump.
				if( pTAP->GetSizeKey() <= iDN50 )
				{
					if( ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartControlValveBlw65BdyMatID ).MP == pTAP->GetBodyMaterialIDPtr().MP )
							&& ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartControlValveBlw65ConnectID ).MP == pTAP->GetConnectIDPtr().MP )
							&& ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::BvBlw65PNID ).MP == pTAP->GetPNIDPtr().MP ) )
					{
						ClearUserChoiceTemporaryVariables();
					}
				}
				else
				{
					// DN65 and above.
					if( ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartControlValveAbv50BdyMatID ).MP == pTAP->GetBodyMaterialIDPtr().MP )
							&& ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartControlValveAbv50ConnectID ).MP == pTAP->GetConnectIDPtr().MP )
							&& ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartControlValveAbv50PNID ).MP == pTAP->GetPNIDPtr().MP ) )
					{
						ClearUserChoiceTemporaryVariables();
					}
				}
			}
		}
		else
		{
			InvalidateSelection();
			ClearUserChoiceTemporaryVariables();
		}
	}
	else
	{
		InvalidateSelection();
		ClearUserChoiceTemporaryVariables();
	}
}

int CDS_HydroMod::CSmartControlValve::GetAvailableBodyMaterial( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapSmartControlValve.size() )
	{
		return 0;
	}

	pmap->clear();
	std::multimap <int, CSmartControlValveData >::iterator It;

	for( It = m_mapSmartControlValve.begin(); It != m_mapSmartControlValve.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpSmartControlValve();
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetBodyMaterialIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

int CDS_HydroMod::CSmartControlValve::GetAvailableConnection( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapSmartControlValve.size() )
	{
		return 0;
	}

	pmap->clear();

	CDB_StringID *pStrIDBodyMaterial = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDBodyMaterial = dynamic_cast<CDB_StringID *>( GetpTAP()->GetBodyMaterialIDPtr().MP );
	}

	std::multimap <int, CSmartControlValveData >::iterator It;

	for( It = m_mapSmartControlValve.begin(); It != m_mapSmartControlValve.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpSmartControlValve();
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetBodyMaterialIDPtr().MP );

		if( pStrID != pStrIDBodyMaterial )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

int CDS_HydroMod::CSmartControlValve::GetAvailablePN( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapSmartControlValve.size() )
	{
		return 0;
	}

	pmap->clear();

	CDB_StringID *pStrIDBodyMaterial = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDBodyMaterial = dynamic_cast<CDB_StringID *>( GetpTAP()->GetBodyMaterialIDPtr().MP );
	}

	CDB_StringID *pStrIDConnection = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDConnection = dynamic_cast<CDB_StringID *>( GetpTAP()->GetConnectIDPtr().MP );
	}

	std::multimap <int, CSmartControlValveData >::iterator It;

	for( It = m_mapSmartControlValve.begin(); It != m_mapSmartControlValve.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpSmartControlValve();

		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetBodyMaterialIDPtr().MP );

		if( pStrID != pStrIDBodyMaterial )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );

		if( pStrID != pStrIDConnection )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetPNIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

CDB_StringID *CDS_HydroMod::CSmartControlValve::GetpSelBodyMaterial()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( (CDB_SmartControlValve *) GetIDPtr().MP )->GetBodyMaterialIDPtr().MP ) ;
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CSmartControlValve::GetpSelConnection()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( ( CDB_ShutoffValve * ) GetIDPtr().MP )->GetConnectIDPtr().MP );
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CSmartControlValve::GetpSelPN()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( ( CDB_ShutoffValve * ) GetIDPtr().MP )->GetPNIDPtr().MP );
	}

	return p;
}

void CDS_HydroMod::CSmartControlValve::SetpSelBodyMaterialAccordingUserChoice( CDB_StringID *pSelSmartControlValvepBodyMaterial )
{
	if( NULL != pSelSmartControlValvepBodyMaterial && false == pSelSmartControlValvepBodyMaterial->IsClass( CLASS( CDB_StringID ) ) )
	{
		ASSERT( false );
		pSelSmartControlValvepBodyMaterial = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceBodyMaterial = pSelSmartControlValvepBodyMaterial;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceSmartControlValveConnect, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CSmartControlValve::SetpSelConnectionAccordingUserChoice( CDB_StringID *pSelSmartControlValvepConn )
{
	if( NULL != pSelSmartControlValvepConn && false == pSelSmartControlValvepConn->IsClass( CLASS( CDB_Connect ) ) )
	{
		ASSERT( false );
		pSelSmartControlValvepConn = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceBodyMaterial = GetpSelBodyMaterial();
	m_pUserChoiceConnection = pSelSmartControlValvepConn;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceSmartControlValvePN, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CSmartControlValve::SetpSelPNAccordingUserChoice( CDB_StringID *pSelSmartControlValvepPN )
{
	if( NULL != pSelSmartControlValvepPN && false == pSelSmartControlValvepPN->IsClass( CLASS( CDB_StringID ) ) )
	{
		ASSERT( false );
		pSelSmartControlValvepPN = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceBodyMaterial = GetpSelBodyMaterial();
	m_pUserChoiceConnection = GetpSelConnection();
	m_pUserChoicePN = pSelSmartControlValvepPN;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceSmartControlValve, (LPARAM)this );
	m_pParent->Modified();
}

CString CDS_HydroMod::CSmartControlValve::GetStrLocalization()
{
	SmartValveLocalization eLoc = SmartValveLocalization::SmartValveLocNone;
	CString strReturn = _T("");

	if( NULL == dynamic_cast<CDB_SmartControlValve *>( GetIDPtr().MP ) )
	{
		return _T("");
	}

	CDB_SmartControlValve *pclSmartControlValve = (CDB_SmartControlValve *)( GetIDPtr().MP );
	return pclSmartControlValve->GetLocalizationString( GetLocalization() );
}

double CDS_HydroMod::CSmartControlValve::GetDp()
{
	if( m_dDp < 0.0 )
	{
		m_dDp = 0.0;
	}

	return m_dDp;
}

double CDS_HydroMod::CSmartControlValve::GetDpMin()
{
	if( NULL == GetIDPtr().MP )
	{
		return 0.0;
	}

	if( 0.0 == GetQ() )
	{
		return 0.0;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eSmartControlValve );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	CDB_SmartControlValve *pclSmartControlValve = static_cast<CDB_SmartControlValve *>( GetIDPtr().MP );

	double dQ = GetQ();
	double dKvs = pclSmartControlValve->GetKvs();
	double dRho = pclWaterChar->GetDens();
	double dDpMin = 0.0;

	if( dQ > 0.0 && dKvs > 0.0 )
	{
		dDpMin = CalcDp( dQ, dKvs, dRho );
	}
	
	return dDpMin;
}

double CDS_HydroMod::CSmartControlValve::GetFlowMax()
{
	return m_pParent->GetQ( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
}

double CDS_HydroMod::CSmartControlValve::GetDT()
{
	CWaterChar *pclSupplyWaterChar = m_pParent->GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );

	if( NULL == pclSupplyWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	CWaterChar *pclReturnWaterChar = m_pParent->GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );

	if( NULL == pclReturnWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	return abs( pclSupplyWaterChar->GetTemp() - pclReturnWaterChar->GetTemp() );
}

double CDS_HydroMod::CSmartControlValve::GetPowerMax()
{
	if( GetQ() <= 0.0 )
	{
		return 0.0;
	}

	CWaterChar *pclSupplyWaterChar = m_pParent->GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );

	if( NULL == pclSupplyWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	CWaterChar *pclReturnWaterChar = m_pParent->GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );

	if( NULL == pclReturnWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	// HYS-1746: Pp = Qp * ( tps * Rho(tps) * Cp(tps) - trp * Rho (trp) * Cp (trp) ).
	double dPower = pclSupplyWaterChar->GetTemp() * pclSupplyWaterChar->GetDens() * pclSupplyWaterChar->GetSpecifHeat();
	dPower -= ( pclReturnWaterChar->GetTemp() * pclReturnWaterChar->GetDens() * pclReturnWaterChar->GetSpecifHeat() );
	dPower *= GetQ();
	dPower = abs( dPower );

	return dPower;
}

CDB_ValveCharacteristic *CDS_HydroMod::CSmartControlValve::GetpValveChar()
{
	CDB_ValveCharacteristic *pDevChar = NULL;

	if( NULL != GetIDPtr().MP )
	{
		pDevChar = ( (CDB_TAProduct *)GetIDPtr().MP )->GetValveCharacteristic();
	}

	return pDevChar;
}

// HYS-1914: Dp is equal to Dp min except if Havail at the entry of the circuit increases, in that case it's the valve that takes the difference.
void CDS_HydroMod::CSmartControlValve::SetDp( double dDp )
{
	if( dDp != GetDp() )
	{
		m_dDp = dDp;
		m_pParent->Modified();
	}
}

bool CDS_HydroMod::CSmartControlValve::IsCompletelyDefined()
{
	if( NULL == GetIDPtr().MP )				// safe IDPtr, checked by Extend in GetIDPtr()
	{
		return false;
	}

	if( 0.0 == GetDp() )
	{
		return false;
	}

	return true;
}

void CDS_HydroMod::CSmartControlValve::ResizeSmartControlValve( double dHAvailable )
{
	if( 0.0 == GetQ() )
	{
		InvalidateSelection();
		return;
	}

	if( false == m_pParent->IsLocked( eHMObj::eSmartControlValve ) )
	{
		SelectBestSmartControlValve();
	}

	CDB_SmartControlValve *pclSmartControlValve = (CDB_SmartControlValve *)( GetIDPtr().MP );

	if( NULL == pclSmartControlValve )
	{
		// Nothing found, we return here.
		return;
	}

	double dDp = dHAvailable;

	if( dDp < GetDpMin() )
	{
		dDp = GetDpMin();
	}

	SetDp( dDp );
}

//////////////////////////////////////////////////////////////////////
// Smart differential pressure controller.
//////////////////////////////////////////////////////////////////////
CDS_HydroMod::CSmartDpC::CSmartDpC( CDS_HydroMod *pHM ) : CBase( pHM )
{
	m_eLocalization = SmartValveLocalization::SmartValveLocSupply;
	SetIDPtr( _NULL_IDPTR );
	SetBestIDPtr( _NULL_IDPTR );
	SetDpSensorIDPtr( _NULL_IDPTR );
	SetProductSetIDPtr( _NULL_IDPTR );
	SetDiversityIDPtr( _NULL_IDPTR );
	ClearUserChoiceTemporaryVariables();
	m_eUserChoiceLocalization = SmartValveLocalization::SmartValveLocNone;
	m_usValidityFlags = 0;
	m_dQ = 0.0;
	m_dDp = 0.0;						// HYS-1914: Dp is equal to Dp min except if Havail at the entry of the circuit increases, in that case it's the Dp that takes the difference.
	m_ArAccessoryIDPtr.SetSize( 0 );	// IDPTRs for selected smart differential pressure controller accessories.
	m_ArDpSensorAccessoryIDPtr.SetSize( 0 );
	m_ArSetContentIDPtr.SetSize( 0 );
	m_bSelectedAsaPackage = eb3Undef;		// True when the selection was done as a package
	m_bSetContentRefreshed = false;
}

void CDS_HydroMod::CSmartDpC::Copy( CDS_HydroMod::CSmartDpC *pclHMTargetHMSmartDpC, bool bResetDiversityFactor )
{
	pclHMTargetHMSmartDpC->SetLocalization( m_eLocalization );
	pclHMTargetHMSmartDpC->SetIDPtr( GetIDPtr() );
	pclHMTargetHMSmartDpC->SetDpSensorIDPtr( GetDpSensorIDPtr() );
	pclHMTargetHMSmartDpC->SetProductSetIDPtr( GetProductSetIDPtr() );
	pclHMTargetHMSmartDpC->SetBestIDPtr( GetBestIDPtr() );
	pclHMTargetHMSmartDpC->SetDiversityIDPtr( ( true == bResetDiversityFactor ) ? _NULL_IDPTR : GetDiversityIDPtr() );
	pclHMTargetHMSmartDpC->SetQ( GetQ() );
	pclHMTargetHMSmartDpC->SetDp( GetDp() );
	pclHMTargetHMSmartDpC->SetSelectedAsaPackage( IsSelectedAsaPackage() );
	pclHMTargetHMSmartDpC->SetValidtyFlags( GetValidtyFlags() );

	if( NULL == m_pUserChoiceBodyMaterial && NULL == m_pUserChoiceConnection && NULL == m_pUserChoicePN )
	{
		pclHMTargetHMSmartDpC->ClearUserChoiceTemporaryVariables();
	}
	else
	{
		pclHMTargetHMSmartDpC->SetpSelBodyMaterialAccordingUserChoice( m_pUserChoiceBodyMaterial );
		pclHMTargetHMSmartDpC->SetpSelConnectionAccordingUserChoice( m_pUserChoiceConnection );
		pclHMTargetHMSmartDpC->SetpSelPNAccordingUserChoice( m_pUserChoicePN );
	}

	if( SmartValveLocalization::SmartValveLocNone != m_eUserChoiceLocalization )
	{
		pclHMTargetHMSmartDpC->SetSelLocalizationUserChoice( m_eUserChoiceLocalization );
	}

	for (int i = 0; i < m_ArAccessoryIDPtr.GetSize(); i++)
	{
		pclHMTargetHMSmartDpC->AddAccessoryIDPtr( m_ArAccessoryIDPtr[i] );
	}

	for (int i = 0; i < m_ArDpSensorAccessoryIDPtr.GetSize(); i++)
	{
		pclHMTargetHMSmartDpC->AddDpSensorAccessoryIDPtr( m_ArDpSensorAccessoryIDPtr[i] );
	}

	for (int i = 0; i < m_ArSetContentIDPtr.GetSize(); i++)
	{
		pclHMTargetHMSmartDpC->AddSetContentIDPtr( m_ArSetContentIDPtr[i] );
	}
}

bool CDS_HydroMod::CSmartDpC::IsSmartDpCExistInPreselectedList( CDB_TAProduct *pclSmartDpC )
{
	if( 0 == (int)m_mapSmartDpC.size() )
	{
		return false;
	}

	bool bSmartControlValveExist = false;

	for( auto &iter : m_mapSmartDpC )
	{
		if( pclSmartDpC == iter.second.GetpSmartDpC() )
		{
			bSmartControlValveExist = true;
			break;
		}
	}

	return bSmartControlValveExist;
}

void CDS_HydroMod::CSmartDpC::AddErrorMessage( ErrorMessageObject eErrorMessageObject, CDS_HydroMod::eValidityFlags eValidityFlags, CString strErrorMessage )
{
	if( 0 == m_mapErrorMessages.count( eErrorMessageObject ) )
	{
		mapValidtyFlagsCString mapTemp;
		m_mapErrorMessages.insert( std::pair<ErrorMessageObject, mapValidtyFlagsCString>( eErrorMessageObject, mapTemp ) ); 
	}

	m_mapErrorMessages.at( eErrorMessageObject ).insert( std::pair<CDS_HydroMod::eValidityFlags, CString>( eValidityFlags, strErrorMessage ) );
}

std::vector<CString> CDS_HydroMod::CSmartDpC::GetErrorMessageList( ErrorMessageObject eErrorMessageObject, CDS_HydroMod::ErrorWarningMessageFlags eErrorWarningMessageFlags )
{
	std::vector<CString> vecAllMessages;

	if( 0 == m_mapErrorMessages.count( eErrorMessageObject ) )
	{
		return vecAllMessages;
	}

	// Check error messages for the smart differential pressure controller.
	if( ErrorMessageObject::eErrorMessageObject_SmartDpC == eErrorMessageObject )
	{
		if ( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Temp == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Temp )
				|| CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All ) )
		{
			if( m_mapErrorMessages.at( eErrorMessageObject ).count( CDS_HydroMod::eValidityFlags::evfTempTooHigh ) > 0 )
			{
				vecAllMessages.push_back( m_mapErrorMessages.at( eErrorMessageObject ).at( CDS_HydroMod::eValidityFlags::evfTempTooHigh ) );
			}
		
			if( m_mapErrorMessages.at( eErrorMessageObject ).count( CDS_HydroMod::eValidityFlags::evfTempTooLow ) > 0 )
			{
				vecAllMessages.push_back( m_mapErrorMessages.at( eErrorMessageObject ).at( CDS_HydroMod::eValidityFlags::evfTempTooLow ) );
			}
		}

		if( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Flow == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Flow )
				|| CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All ) )
		{
			if( m_mapErrorMessages.at( eErrorMessageObject ).count( CDS_HydroMod::eValidityFlags::evfFlowTooLow ) > 0 )
			{
				vecAllMessages.push_back( m_mapErrorMessages.at( eErrorMessageObject ).at( CDS_HydroMod::eValidityFlags::evfFlowTooLow ) );
			}
		
			if( m_mapErrorMessages.at( eErrorMessageObject ).count( CDS_HydroMod::eValidityFlags::evfFlowTooHigh ) > 0 )
			{
				vecAllMessages.push_back( m_mapErrorMessages.at( eErrorMessageObject ).at( CDS_HydroMod::eValidityFlags::evfFlowTooHigh ) );
			}
		}

		if( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dp == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dp )
				|| CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All ) )
		{
			if( m_mapErrorMessages.at( eErrorMessageObject ).count( CDS_HydroMod::eValidityFlags::evfDpTooHigh ) > 0 )
			{
				vecAllMessages.push_back( m_mapErrorMessages.at( eErrorMessageObject ).at( CDS_HydroMod::eValidityFlags::evfDpTooHigh ) );
			}
		
			if( m_mapErrorMessages.at( eErrorMessageObject ).count( CDS_HydroMod::eValidityFlags::evfDpTooLow ) > 0 )
			{
				vecAllMessages.push_back( m_mapErrorMessages.at( eErrorMessageObject ).at( CDS_HydroMod::eValidityFlags::evfDpTooLow ) );
			}
		}
	}

	// Check error messages for Dp sensor.
	if( ErrorMessageObject::eErrorMessageObject_DpSensor == eErrorMessageObject )
	{
		if( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dpl == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dpl )
				|| CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All ) )
		{
			if( m_mapErrorMessages.at( eErrorMessageObject ).count( CDS_HydroMod::eValidityFlags::evfDplTooLow ) > 0 )
			{
				vecAllMessages.push_back( m_mapErrorMessages.at( eErrorMessageObject ).at( CDS_HydroMod::eValidityFlags::evfDplTooLow ) );
			}
		
			if( m_mapErrorMessages.at( eErrorMessageObject ).count( CDS_HydroMod::eValidityFlags::evfDplTooHigh ) > 0 )
			{
				vecAllMessages.push_back( m_mapErrorMessages.at( eErrorMessageObject ).at( CDS_HydroMod::eValidityFlags::evfDplTooHigh ) );
			}
		}

		if( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Temp == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Temp )
				|| CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All == ( eErrorWarningMessageFlags & CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All ) )
		{
			if( m_mapErrorMessages.at( eErrorMessageObject ).count( CDS_HydroMod::eValidityFlags::evfTempTooHigh ) > 0 )
			{
				vecAllMessages.push_back( m_mapErrorMessages.at( eErrorMessageObject ).at( CDS_HydroMod::eValidityFlags::evfTempTooHigh ) );
			}
		
			if( m_mapErrorMessages.at( eErrorMessageObject ).count( CDS_HydroMod::eValidityFlags::evfTempTooLow ) > 0 )
			{
				vecAllMessages.push_back( m_mapErrorMessages.at( eErrorMessageObject ).at( CDS_HydroMod::eValidityFlags::evfTempTooLow ) );
			}
		}
	}

	return vecAllMessages;
}

CString CDS_HydroMod::CSmartDpC::GetErrorMessageString( ErrorMessageObject eErrorMessageObject, CDS_HydroMod::ErrorWarningMessageFlags eErrorWarningMessageFlags, 
		CString strPrefix, bool bReturnLine )
{
	return ConvertVecStringToString( GetErrorMessageList( eErrorMessageObject, eErrorWarningMessageFlags ), strPrefix, bReturnLine );
}

void CDS_HydroMod::CSmartDpC::AddDpSensorIDPtr( IDPTR IDPtr )
{
	m_DpSensorIDPtr = IDPtr;
	m_pParent->Modified();
}

void CDS_HydroMod::CSmartDpC::AddProductSetIDPtr( IDPTR IDPtr )
{
	m_ProductSetIDPtr = IDPtr;
	m_pParent->Modified();
}

void CDS_HydroMod::CSmartDpC::AddAccessoryIDPtr( IDPTR IDPtr )
{
	m_ArAccessoryIDPtr.Add( IDPtr );
	m_pParent->Modified();
}

void CDS_HydroMod::CSmartDpC::AddDpSensorAccessoryIDPtr( IDPTR IDPtr )
{
	m_ArDpSensorAccessoryIDPtr.Add( IDPtr );
	m_pParent->Modified();
}

void CDS_HydroMod::CSmartDpC::AddSetContentIDPtr( IDPTR IDPtr )
{
	m_ArSetContentIDPtr.Add( IDPtr );
	m_pParent->Modified();
}

CDB_SmartControlValve *CDS_HydroMod::CSmartDpC::GetpSmartDpC()
{
	CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( GetIDPtr().MP );
	return pclSmartDpC;
}

IDPTR CDS_HydroMod::CSmartDpC::GetIDPtr()
{
	if( _T('\0') != *m_IDPtr.ID )
	{
		m_IDPtr.DB = TASApp.GetpTADB();
		Extend( &m_IDPtr );
		return m_IDPtr;
	}

	return _NULL_IDPTR;
}

bool CDS_HydroMod::CSmartDpC::IsBestValve( CData *pData )
{
	return ( pData == GetBestIDPtr().MP ) ? true : false;
}

IDPTR CDS_HydroMod::CSmartDpC::GetBestIDPtr()
{
	if( _T('\0') != *m_IDPtrBest.ID )
	{
		m_IDPtrBest.DB = TASApp.GetpTADB();
		Extend( &m_IDPtrBest );
		return m_IDPtrBest;
	}

	return _NULL_IDPTR;
}

void CDS_HydroMod::CSmartDpC::SetIDPtr( IDPTR IDPtr )
{
	m_IDPtr = IDPtr;

	if( _T('\0') != *m_IDPtr.ID && NULL != m_IDPtr.DB )
	{
		Extend( &m_IDPtr );
	}

	m_pParent->Modified();
	m_bSetContentRefreshed = false;
}

IDPTR CDS_HydroMod::CSmartDpC::GetAccessoryIDPtr( int iIndex )
{
	if( iIndex < m_ArAccessoryIDPtr.GetSize() )
	{
		if( _T('\0') != *m_ArAccessoryIDPtr[iIndex].ID && NULL != m_ArAccessoryIDPtr[iIndex].DB )
		{
			Extend( &m_ArAccessoryIDPtr[iIndex] );
		}

		return m_ArAccessoryIDPtr[iIndex];
	}

	return _NULL_IDPTR;
}

IDPTR CDS_HydroMod::CSmartDpC::GetDpSensorAccessoryIDPtr( int iIndex )
{
	if( iIndex < m_ArDpSensorAccessoryIDPtr.GetSize() )
	{
		if( _T('\0') != *m_ArDpSensorAccessoryIDPtr[iIndex].ID && NULL != m_ArDpSensorAccessoryIDPtr[iIndex].DB )
		{
			Extend( &m_ArDpSensorAccessoryIDPtr[iIndex] );
		}

		return m_ArDpSensorAccessoryIDPtr[iIndex];
	}

	return _NULL_IDPTR;
}

int CDS_HydroMod::CSmartDpC::GetSetContentCount()
{
	_VerifySetContent();
	return m_ArSetContentIDPtr.GetCount();
}

IDPTR CDS_HydroMod::CSmartDpC::GetSetContentIDPtr( int iIndex )
{
	_VerifySetContent();

	if( iIndex < m_ArSetContentIDPtr.GetSize() )
	{
		if( _T('\0') != *m_ArSetContentIDPtr[iIndex].ID && NULL != m_ArSetContentIDPtr[iIndex].DB )
		{
			Extend( &m_ArSetContentIDPtr[iIndex] );
		}

		return m_ArSetContentIDPtr[iIndex];
	}

	return _NULL_IDPTR;
}

CArray<IDPTR> *CDS_HydroMod::CSmartDpC::GetpSetContentIDPtrArray()
{
	_VerifySetContent();
	return &m_ArSetContentIDPtr;
}

IDPTR CDS_HydroMod::CSmartDpC::GetDpSensorIDPtr()
{
	if( _T('\0') != *m_DpSensorIDPtr.ID && NULL != m_DpSensorIDPtr.DB )
	{
		Extend( &m_DpSensorIDPtr );
		return m_DpSensorIDPtr;
	}

	return _NULL_IDPTR;
}

CDB_DpSensor *CDS_HydroMod::CSmartDpC::GetpDpSensor()
{
	return dynamic_cast<CDB_DpSensor *>( GetDpSensorIDPtr().MP );
}

IDPTR CDS_HydroMod::CSmartDpC::GetProductSetIDPtr()
{
	if( _T('\0') != *m_ProductSetIDPtr.ID && NULL != m_ProductSetIDPtr.DB )
	{
		Extend( &m_ProductSetIDPtr );
		return m_ProductSetIDPtr;
	}

	return _NULL_IDPTR;
}

CDB_Product *CDS_HydroMod::CSmartDpC::GetpProductSet()
{
	return dynamic_cast<CDB_Product *>( GetProductSetIDPtr().MP );
}

CDB_DpSensor *CDS_HydroMod::CSmartDpC::GetpDpSensorSet()
{
	return dynamic_cast<CDB_DpSensor *>( GetProductSetIDPtr().MP );
}

IDPTR CDS_HydroMod::CSmartDpC::GetDpSensorIDPtrInDpSensorSet()
{
	IDPTR DpSensorIDPtr = _NULL_IDPTR;
	CDB_DpSensor *pclDpSensor = GetDpSensorInDpSensorSet();

	if( NULL != pclDpSensor )
	{
		DpSensorIDPtr = pclDpSensor->GetIDPtr();
	}

	return DpSensorIDPtr;
}

CDB_DpSensor *CDS_HydroMod::CSmartDpC::GetDpSensorInDpSensorSet()
{
	CDB_DpSensor *pclDpSensorSet = GetpDpSensorSet();

	if( NULL == pclDpSensorSet )
	{
		// We are with a connection set that doesn't contain Dp sensor.
		return NULL;
	}

	CDB_DpSensor *pclDpSensor = NULL;

	// Only one in the group is the Dp sensor.
	if( m_ArSetContentIDPtr.GetCount() > 0 )
	{
		for( int i = 0; i < m_ArSetContentIDPtr.GetCount(); i++ )
		{
			pclDpSensor = dynamic_cast<CDB_DpSensor *>( m_ArSetContentIDPtr.GetAt( i ).MP );

			if( NULL != pclDpSensor )
			{
				break;
			}
		}
	}

	return pclDpSensor;
}

CDB_Set *CDS_HydroMod::CSmartDpC::GetpSet()
{
	CDB_Set *pclSet = NULL;

	if( false == IsSelectedAsaPackage() )
	{
		return pclSet;
	}

	CDB_SmartControlValve *pclCurrentSmartDpCSelected = (CDB_SmartControlValve *)( GetIDPtr().MP );
	CDB_Product *pclCurrentProductSetSelected = GetpProductSet();
	CTableSet *pclTableSet = pclCurrentSmartDpCSelected->GetTableSet();

	if( NULL == pclCurrentSmartDpCSelected || NULL == pclCurrentProductSetSelected || NULL == pclTableSet )
	{
		return pclSet;
	}

	return pclTableSet->FindCompatibleSet( pclCurrentSmartDpCSelected->GetIDPtr().ID, pclCurrentProductSetSelected->GetIDPtr().ID );
}

void CDS_HydroMod::CSmartDpC::SetDpSensorIDPtr( IDPTR DpSensorIDPtr )
{
	m_DpSensorIDPtr = DpSensorIDPtr;
	m_pParent->Modified();
}

void CDS_HydroMod::CSmartDpC::SetProductSetIDPtr( IDPTR ProductSetIDPtr )
{
	m_ProductSetIDPtr = ProductSetIDPtr;
	m_pParent->Modified();
	m_bSetContentRefreshed = false;
}

void CDS_HydroMod::CSmartDpC::SetSelectedAsaPackage( eBool3 eFlag )
{
	m_bSelectedAsaPackage = eFlag;
	m_pParent->Modified();
}

bool CDS_HydroMod::CSmartDpC::ForceSelection( IDPTR DeviceIDptr )
{
	if( true == m_mapSmartDpC.empty() )
	{
		// Unlock the valve to force filling of 'm_mapSmartDpC'.
		m_pParent->SetLock( CDS_HydroMod::eHMObj::eSmartDpC, false );
	}

	// Lock valve if new selected valve doesn't correspond to the Best valve idptr
	m_pParent->SetLock( CDS_HydroMod::eHMObj::eSmartDpC, !IsBestValve( (CData *)DeviceIDptr.MP ) );

	// Forced valve should be inside 'm_mapSmartDpC' as a valid choice;
	_SetpSelSmartDpC( (LPARAM)DeviceIDptr.MP, true );
	ClearUserChoiceTemporaryVariables();
	
	m_bSetContentRefreshed = false;

	return ( GetIDPtr().MP == DeviceIDptr.MP );
}

int CDS_HydroMod::CSmartDpC::CheckValidity()
{
	CString str;
	m_usValidityFlags = CDS_HydroMod::eValidityFlags::evfOK;
	m_mapErrorMessages.clear();

	if( NULL == GetIDPtr().MP )
	{
		return m_usValidityFlags;
	}

	CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( GetIDPtr().MP );

	if( NULL == pclSmartDpC )
	{
		return m_usValidityFlags;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eSmartDpC );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( m_usValidityFlags );
	}

	// Check pressure drop accross the smart differential pressure controller.
	if( GetDp() > m_pParent->GetpTechParam()->VerifyValvMaxDp( (CDB_TAProduct *)GetIDPtr().MP ) )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDpTooHigh;

		// The pressure drop accross the valve is higher that the maximum allowed (%1 > %2).
		CString strDp = WriteCUDouble( _U_DIFFPRESS, GetDp() );
		CString strMaxDp = WriteCUDouble( _U_DIFFPRESS, pclSmartDpC->GetDpmax(), true );
		FormatString( str, IDS_HYDROMOD_SMARTVALVEDPTOOHIGH, strDp, strMaxDp );
		AddErrorMessage( eErrorMessageObject_SmartDpC, CDS_HydroMod::eValidityFlags::evfDpTooHigh, str );
	}

	// Check the flow accross the smart differential pressure controller.
	if( GetQ() > pclSmartDpC->GetQNom() ) 
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfFlowTooHigh;

		// The flow is higher that the maximum allowed (%1 > %2).
		CString strFlow = WriteCUDouble( _U_FLOW, GetFlowMax() );
		CString strMaxFlow = WriteCUDouble( _U_FLOW, pclSmartDpC->GetQNom(), true );
		FormatString( str, IDS_HYDROMOD_SMARTVALVEFLOWTOOHIGH, strFlow, strMaxFlow );
		AddErrorMessage( eErrorMessageObject_SmartDpC, CDS_HydroMod::eValidityFlags::evfFlowTooHigh, str );
	}

	if( GetQ() < pclSmartDpC->GetMinAdjustableFlow() ) 
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfFlowTooLow;

		// The flow is lower that the minimum allowed (%1 < %2).
		CString strFlow = WriteCUDouble( _U_FLOW, GetFlowMax() );
		CString strMinFlow = WriteCUDouble( _U_FLOW, pclSmartDpC->GetMinAdjustableFlow(), true );
		FormatString( str, IDS_HYDROMOD_SMARTVALVEFLOWTOOLOW, strFlow, strMinFlow );
		AddErrorMessage( eErrorMessageObject_SmartDpC, CDS_HydroMod::eValidityFlags::evfFlowTooLow, str );
	}

	// Check temperatures for smart differential pressure controller.
	if( pclWaterChar->GetTemp() < pclSmartDpC->GetTmin() )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfTempTooLow;

		// The fluid temperature is lower that the minimum allowed for %1 (%2 < %3).
		CString strProductName = pclSmartDpC->GetName();
		CString strTemp = WriteCUDouble( _U_TEMPERATURE, pclWaterChar->GetTemp() );
		CString strMinTemp = WriteCUDouble( _U_TEMPERATURE, pclSmartDpC->GetTmin(), true );
		FormatString( str, IDS_HYDROMOD_TEMPTOOLOWFORPRODUCT, strProductName, strTemp, strMinTemp );
		AddErrorMessage( eErrorMessageObject_SmartDpC, CDS_HydroMod::eValidityFlags::evfTempTooLow, str );
	}

	if( pclWaterChar->GetTemp() > pclSmartDpC->GetTmax() )
	{
		m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfTempTooHigh;

		// The fluid temperature is higher that the maximum allowed for %1 (%2 > %3).
		CString strProductName = pclSmartDpC->GetName();
		CString strTemp = WriteCUDouble( _U_TEMPERATURE, pclWaterChar->GetTemp() );
		CString strMaxTemp = WriteCUDouble( _U_TEMPERATURE, pclSmartDpC->GetTmax(), true );
		FormatString( str, IDS_HYDROMOD_TEMPTOOHIGHFORPRODUCT, strProductName, strTemp, strMaxTemp );
		AddErrorMessage( eErrorMessageObject_SmartDpC, CDS_HydroMod::eValidityFlags::evfTempTooHigh, str );
	}

	// Check Dp sensor dpl range.
	CDB_DpSensor *pclDpSensor = GetpDpSensor();

	if( NULL == pclDpSensor && NULL != GetpDpSensorSet() )
	{
		// Pay attention that in 'Set' it can be a 'CDB_DpSensor' for a Dp sensor set or a 'CDB_Product' for a connection set.
		pclDpSensor = GetDpSensorInDpSensorSet();
	}

	if( NULL != pclDpSensor )
	{
		double dDpToStabilize = GetDpToStabilize();

		if( dDpToStabilize > 0.0 )
		{
			if( dDpToStabilize < pclDpSensor->GetMinMeasurableDp() )
			{
				m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDplTooLow;

				// Dp to stabilize is below the minimum measurable pressure drop of the currenct selected Dp sensor (%1 < %2).
				CString strDpl = WriteCUDouble( _U_DIFFPRESS, dDpToStabilize );
				CString strMinMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMinMeasurableDp(), true );
				FormatString( str, IDS_HYDROMOD_SMARTDPCDPLTOOLOW, strDpl, strMinMeasurableDpl );
				AddErrorMessage( eErrorMessageObject_DpSensor, CDS_HydroMod::eValidityFlags::evfDplTooLow, str );
			}
			else if( dDpToStabilize > pclDpSensor->GetMaxMeasurableDp() )
			{
				m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfDplTooHigh;

				// Dp to stabilize is above the maximum measurable pressure drop of the current selected Dp sensor (%1 > %2).
				CString strDpl = WriteCUDouble( _U_DIFFPRESS, dDpToStabilize );
				CString strMaxMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMaxMeasurableDp(), true );
				FormatString( str, IDS_HYDROMOD_SMARTDPCDPLTOOHIGH, strDpl, strMaxMeasurableDpl );
				AddErrorMessage( eErrorMessageObject_DpSensor, CDS_HydroMod::eValidityFlags::evfDplTooHigh, str );
			}
		}
	}

	// Check temperatures for Dp sensor.
	if( NULL != pclDpSensor )
	{
		if( pclWaterChar->GetTemp() < pclDpSensor->GetTmin() )
		{
			m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfTempTooLow;

			// The fluid temperature is lower that the minimum allowed for %1 (%2 < %3).
			CString strProductName = pclDpSensor->GetName();
			CString strTemp = WriteCUDouble( _U_TEMPERATURE, pclWaterChar->GetTemp() );
			CString strMinTemp = WriteCUDouble( _U_TEMPERATURE, pclDpSensor->GetTmin(), true );
			FormatString( str, IDS_HYDROMOD_TEMPTOOLOWFORPRODUCT, strProductName, strTemp, strMinTemp );
			AddErrorMessage( eErrorMessageObject_DpSensor, CDS_HydroMod::eValidityFlags::evfTempTooLow, str );
		}

		if( pclWaterChar->GetTemp() > pclDpSensor->GetTmax() )
		{
			m_usValidityFlags |= CDS_HydroMod::eValidityFlags::evfTempTooHigh;

			// The fluid temperature is higher that the maximum allowed for %1 (%2 > %3).
			CString strProductName = pclDpSensor->GetName();
			CString strTemp = WriteCUDouble( _U_TEMPERATURE, pclWaterChar->GetTemp() );
			CString strMaxTemp = WriteCUDouble( _U_TEMPERATURE, pclDpSensor->GetTmax(), true );
			FormatString( str, IDS_HYDROMOD_TEMPTOOHIGHFORPRODUCT, strProductName, strTemp, strMaxTemp );
			AddErrorMessage( eErrorMessageObject_DpSensor, CDS_HydroMod::eValidityFlags::evfTempTooHigh, str );
		}
	}

	return m_usValidityFlags;
}

void CDS_HydroMod::CSmartDpC::DropData( OUTSTREAM outf, int *piLevel )
{
	CString strTab = _T("");

	for( int i = *piLevel; i; i-- )
	{
		strTab += _T("\t");
	}

	CString str;
	str.Format( _T("m_IDPtr.ID = %s\n"), GetIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_IDPtrBest.ID = %s\n"), GetBestIDPtr().ID );
	WriteFormatedStringA2( outf, str, strTab );
	str = _T("m_Located = eSmartControlValve");
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_usValidityFlags = %u\n"), GetValidtyFlags() );
	WriteFormatedStringA2( outf, str, strTab );
	WriteFormatedStringA2( outf, _T("\n"), strTab );
}

void CDS_HydroMod::CSmartDpC::ClearUserChoiceTemporaryVariables()
{
	m_pUserChoiceBodyMaterial = NULL;
	m_pUserChoiceConnection = NULL;
	m_pUserChoicePN = NULL;
}

void CDS_HydroMod::CSmartDpC::_AbortSelectBest()
{
	if( false == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eSmartDpC ) || 0 == *GetIDPtr().ID )
	{
		InvalidateSelection();
	}

	ClearUserChoiceTemporaryVariables();
}

void CDS_HydroMod::CSmartDpC::_SetpSelSmartDpC( LPARAM pclSmartDpC, bool fForceLocker )
{
	if( false == fForceLocker )
	{
		if( true == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eSmartDpC ) && 0 != *GetIDPtr().ID )
		{
			m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceSmartDpC, (LPARAM)this );
			return;
		}
	}

	// Try to find product inside main valve map.
	std::multimap <int, CSmartDDpCData>::iterator itMapValves;

	for( itMapValves = m_mapSmartDpC.begin(); m_mapSmartDpC.end() != itMapValves; ++itMapValves )
	{
		if( pclSmartDpC == (LPARAM)itMapValves->second.GetpSmartDpC() )
		{
			break;
		}
	}

	if( m_mapSmartDpC.end() != itMapValves )
	{
		SetIDPtr( itMapValves->second.GetpSmartDpC()->GetIDPtr() );
		m_pParent->SetCBIValveID( GetIDPtr().ID );
		_VerifySetContent();
	}
	else
	{
		InvalidateSelection();
	}

	m_bSetContentRefreshed = false;
	m_pParent->Modified();
}

void CDS_HydroMod::CSmartDpC::_SelectBestDpSensor()
{
	m_ArDpSensorAccessoryIDPtr.RemoveAll();
	m_bSetContentRefreshed = false;
	m_DpSensorIDPtr = _NULL_IDPTR;

	double dDpToStabilize = GetDpToStabilize();

	if( dDpToStabilize <= 0.0 )
	{
		return;
	}

	// Retrieve first the best smart differential pressure controller selected.
	CDB_SmartControlValve *pclSmartDpC = GetpSmartDpC();

	if( NULL == pclSmartDpC )
	{
		return;
	}

	// HYS-1992: Fictif Set.
	CTable *pclTableAccessorySet = (CTable*)pclSmartDpC->GetDpSensorGroupIDPtr().MP;

	if( NULL == pclTableAccessorySet )
	{
		return;
	}

	// HYS-2067: Reset connection set
	if( NULL != dynamic_cast<CDB_DpSensor *>(m_ProductSetIDPtr.MP) )
	{
		CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)(m_ProductSetIDPtr.MP);

		if( (pclDpSensor->GetMinMeasurableDp() / 1000.0) >= 100.0 )
		{
			m_ProductSetIDPtr = _NULL_IDPTR;
			m_ArSetContentIDPtr.RemoveAll();
		}
		if( dDpToStabilize < pclDpSensor->GetMinMeasurableDp() || dDpToStabilize > pclDpSensor->GetMaxMeasurableDp() )
		{
			m_ProductSetIDPtr = _NULL_IDPTR;
			m_ArSetContentIDPtr.RemoveAll();
		}
	}

	CRank ProductSetList;

	for( IDPTR IDPtr = pclTableAccessorySet->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pclTableAccessorySet->GetNext( IDPtr.MP ) )
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
	double dBestAlpha = -DBL_MAX;

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

		if( dDpToStabilize < pclProductSet->GetMinMeasurableDp() || dDpToStabilize > pclProductSet->GetMaxMeasurableDp() )
		{
			continue;
		}

		double dAlphaMin = fabs( pclProductSet->GetMinMeasurableDp() - dDpToStabilize ) / dDpToStabilize;
		double dAlphaMax = fabs( pclProductSet->GetMaxMeasurableDp() - dDpToStabilize ) / dDpToStabilize;
		double dBest = min( dAlphaMin, dAlphaMax );

		if( dBest > dBestAlpha )
		{
			dBestAlpha = dBest;
			pclBest = pclProductSet;
		}
	}

	if( NULL != pclBest && _NULL_IDPTR == m_ProductSetIDPtr )
	{
		m_ProductSetIDPtr = pclBest->GetIDPtr();
	}
	else if( NULL != pclMinMeasurableDp && _NULL_IDPTR == m_ProductSetIDPtr )
	{
		m_ProductSetIDPtr = pclMinMeasurableDp->GetIDPtr();
	}
}

void CDS_HydroMod::CSmartDpC::_VerifySetContent()
{
	if( true == m_bSetContentRefreshed )
	{
		return;
	}

	m_ArSetContentIDPtr.RemoveAll();

	// Retrieve the current selected smart differential pressure controller.
	CDB_SmartControlValve *pclCurrentSmartDpCSelected = dynamic_cast<CDB_SmartControlValve *>( GetIDPtr().MP );
	CDB_Product *pclCurrentProductSetSelected = GetpProductSet();

	if( NULL == pclCurrentSmartDpCSelected || NULL == pclCurrentProductSetSelected )
	{
		return;
	}

	// HYS-1992: Consider fictif set. Smart DpC is not a set.

	CDB_RuledTable *pRuledTable = (CDB_RuledTable *)(pclCurrentProductSetSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pRuledTable )
	{
		return;
	}

	// Not sorted ! (To keep Dp sensor at the first position if exist).
	CRank rList;
	int iCount = TASApp.GetpTADB()->GetAccessories( &rList, pRuledTable, CTADatabase::FilterSelection::ForHMCalc );

	if( 0 == iCount )
	{
		return;
	}

	CString str;
	LPARAM lparam;

	for( BOOL bContinue = rList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rList.GetNext( str, lparam ) )
	{
		CDB_Product *pclAccessory = (CDB_Product *)lparam;
		AddSetContentIDPtr( pclAccessory->GetIDPtr() );
	}

	m_bSetContentRefreshed = true;
}

#define CDS_HYDROMOD_CSMARTDPC_VERSION		1
void CDS_HydroMod::CSmartDpC::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_CSMARTDPC_VERSION;
	WriteData<>( outf, Version );

	// Info.
	WriteData<>( outf, m_eLocalization );
	WriteString( outf, (LPCTSTR) m_IDPtr.ID );
	WriteString( outf, m_IDPtrBest.ID );
	WriteData<>( outf, m_dQ );
	WriteData<>( outf, GetDp() );
	WriteString( outf, m_DpSensorIDPtr.ID );
	WriteString( outf, m_ProductSetIDPtr.ID );
	WriteData<>( outf, m_bSelectedAsaPackage );
	WriteString( outf, m_DiversityIDPtr.ID );

	CString strID = ( NULL != m_pUserChoiceBodyMaterial ) ? m_pUserChoiceBodyMaterial->GetIDPtr().ID : _T("");
	WriteString( outf, strID );

	strID = ( NULL != m_pUserChoiceConnection ) ? m_pUserChoiceConnection->GetIDPtr().ID : _T("");
	WriteString( outf, strID );
	
	strID = ( NULL != m_pUserChoicePN ) ? m_pUserChoicePN->GetIDPtr().ID : _T("");
	WriteString( outf, strID );

	WriteData<>( outf, m_eUserChoiceLocalization );

	// Smart differential pressure controller accessories.
	int iSize = m_ArAccessoryIDPtr.GetSize();
	WriteData<>( outf, iSize );

	for( int i = 0; i < iSize; i++ )
	{
		WriteString( outf, (LPCTSTR)m_ArAccessoryIDPtr[i].ID );
	}

	// Dp sensor accessories.
	iSize = m_ArDpSensorAccessoryIDPtr.GetSize();
	WriteData<>( outf, iSize );

	for( int i = 0; i < iSize; i++ )
	{
		WriteString( outf, (LPCTSTR)m_ArDpSensorAccessoryIDPtr[i].ID );
	}

	// Smart differential pressure controller and Dp sensor set accessories.
	_VerifySetContent();
	iSize = m_ArSetContentIDPtr.GetSize();
	WriteData<>( outf, iSize );

	for( int i = 0; i < iSize; i++ )
	{
		WriteString( outf, (LPCTSTR)m_ArSetContentIDPtr[i].ID );
	}
}

bool CDS_HydroMod::CSmartDpC::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_HYDROMOD_CSMARTDPC_VERSION;
	ReadData<>( inpf, Version );

	if( Version < 1 || Version > CDS_HYDROMOD_CSMARTDPC_VERSION )
	{
		return false;
	}

	ReadData<>( inpf, m_eLocalization );
	ReadString( inpf, m_IDPtr.ID, sizeof( m_IDPtr.ID ) );
	ReadString( inpf, m_IDPtrBest.ID, sizeof( m_IDPtrBest.ID ) );
	ReadData<>( inpf, m_dQ );
	ReadData<>( inpf, m_dDp );
	ReadString( inpf, m_DpSensorIDPtr.ID, sizeof( m_DpSensorIDPtr.ID ) );
	ReadString( inpf, m_ProductSetIDPtr.ID, sizeof( m_ProductSetIDPtr.ID ) );
	ReadData<>( inpf, m_bSelectedAsaPackage );
	ReadString( inpf, m_DiversityIDPtr.ID, sizeof( m_DiversityIDPtr.ID ) );

	m_IDPtr.DB = TASApp.GetpTADB();
	m_IDPtrBest.DB = TASApp.GetpTADB();
	m_DpSensorIDPtr.DB = TASApp.GetpTADB();
	m_ProductSetIDPtr.DB = TASApp.GetpTADB();
	m_DiversityIDPtr.DB = TASApp.GetpTADB();

	ClearUserChoiceTemporaryVariables();

	IDPTR idptr;

	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceBodyMaterial = dynamic_cast<CDB_StringID *>( idptr.MP );

	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoiceConnection = dynamic_cast<CDB_StringID *>( idptr.MP );

	ReadIDPtr( inpf, &idptr, TASApp.GetpTADB() );
	m_pUserChoicePN = dynamic_cast<CDB_StringID *>( idptr.MP );

	ReadData<>( inpf, m_eUserChoiceLocalization );

	// Read smart differential pressure controller accessories if exist.
	int iSize = 0;
	ReadData<>( inpf, iSize );

	for( int i = 0; i < iSize; i++ )
	{
		CString strID;
		ReadString( inpf, strID );
		IDPTR IDPtr = TASApp.GetpTADB()->Get( strID );
		ASSERT( IDPtr.MP );

		if( NULL != IDPtr.MP )		// Object exist into the database.
		{
			m_ArAccessoryIDPtr.Add( IDPtr );
		}
	}

	// Read Dp sensor accessories if exist.
	iSize = 0;
	ReadData<>( inpf, iSize );

	for( int i = 0; i < iSize; i++ )
	{
		CString strID;
		ReadString( inpf, strID );
		IDPTR IDPtr = TASApp.GetpTADB()->Get( strID );
		ASSERT( IDPtr.MP );

		if( NULL != IDPtr.MP )		// Object exist into the database.
		{
			m_ArDpSensorAccessoryIDPtr.Add( IDPtr );
		}
	}

	// Read smart differential pressure controller and Dp sensor set accessories if exist.
	iSize = 0;
	ReadData<>( inpf, iSize );

	for( int i = 0; i < iSize; i++ )
	{
		CString strID;
		ReadString( inpf, strID );
		IDPTR IDPtr = TASApp.GetpTADB()->Get( strID );
		ASSERT( IDPtr.MP );

		if( NULL != IDPtr.MP )		// Object exist into the database.
		{
			m_ArSetContentIDPtr.Add( IDPtr );
		}
	}

	m_bSetContentRefreshed = true;

	return true;
}

void CDS_HydroMod::CSmartDpC::InvalidateSelection()
{
	if( true == GetpParentHM()->IsLocked( CDS_HydroMod::eHMObj::eSmartDpC ) )
	{
		return;
	}

	m_IDPtr = _NULL_IDPTR;
	m_IDPtrBest = _NULL_IDPTR;
	m_bSetContentRefreshed = false;
}

IDPTR CDS_HydroMod::CSmartDpC::GetDiversityIDPtr()
{
	if( _T('\0') != *m_DiversityIDPtr.ID && NULL != m_DiversityIDPtr.DB )
	{
		Extend( &m_DiversityIDPtr );
		return m_DiversityIDPtr;
	}

	return _NULL_IDPTR;
}

void CDS_HydroMod::CSmartDpC::SelectBest()
{
	if( GetQ() <= 0.0 )
	{
		_AbortSelectBest();
		return;
	}

	if( _T('\0') != *GetIDPtr().ID && true == m_pParent->IsLocked( CDS_HydroMod::eHMObj::eSmartDpC ) )
	{
		_AbortSelectBest();
		return;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eSmartDpC );
	
	if( NULL == pclWaterChar )
	{
		_AbortSelectBest();
		ASSERT_RETURN;
	}

	int iHighestSize = 0;
	int iLowestSize = 0;

	// Smart differential pressure controler selection is based on the pipe size.
	CAnchorPt::PipeLocation ePipeLocation = m_pParent->GetpSch()->GetAnchorPtPipeLocation( CAnchorPt::eFunc::SmartDpC );
	CDB_Pipe *pPipe = m_pParent->GetPipeSizeShift( iHighestSize, iLowestSize, CAnchorPt::CircuitSide_Primary, ePipeLocation );

	if( NULL == pPipe )
	{
		_AbortSelectBest();
		return;
	}

	int iDN50 = m_pParent->GetDN50();
	CDB_StringID *pSmartDpCType = GetpSelType();

	if( NULL == pSmartDpCType )
	{
		CPrjParams *pPrjParams = m_pParent->GetpPrjParam()->GetpHmCalcParams();

		// Nothing selected yet
		// Return to pre-selection done in Tech params.
		if( pPipe->GetSizeKey( TASApp.GetpTADB() ) <= iDN50 )
		{
			pSmartDpCType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartDpCBlw65TypeID ).MP );
		}
		else
		{
			pSmartDpCType = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartDpCAbv50TypeID ).MP );
		}
	}

	m_mapSmartDpC.clear();
	std::multimap <int, CSmartDDpCData > mapSmartDpC;
	std::multimap <int, CSmartDDpCData>::iterator itMapSmartDpC;

	CRankEx List;
	GetpTADB()->GetSmartDpCList( &List, pSmartDpCType->GetIDPtr().ID, L"", L"", L"", L"", L"", CTADatabase::FilterSelection::ForHMCalc, 0, INT_MAX, 0, false, false );

	LPARAM lparam;
	_string str;

	for( BOOL bContinue = List.GetFirst( str, lparam ); TRUE == bContinue; bContinue = List.GetNext( str, lparam ) )
	{
		CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( (CData *)lparam );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		if( false == pclSmartDpC->IsSelectable( true ) )
		{
			continue;
		}

		int iProdSizeKey = pclSmartDpC->GetSizeKey();
		itMapSmartDpC = mapSmartDpC.insert( std::make_pair( iProdSizeKey, CSmartDDpCData( pclSmartDpC ) ) );
	}

	if( 0 == mapSmartDpC.size() )
	{
		_AbortSelectBest();
		return;
	}

	// HYS-1308: Change the way we reject sizes.
	std::multimap <int, CSmartDDpCData>::iterator itLow = mapSmartDpC.end();
	std::multimap <int, CSmartDDpCData>::iterator itUp = mapSmartDpC.end();

	for( std::multimap <int, CSmartDDpCData>::iterator iter = mapSmartDpC.begin(); iter != mapSmartDpC.end(); iter++ )
	{
		if( iter->first >= iLowestSize )
		{
			itLow = iter;
			break;
		}
	}

	if( mapSmartDpC.end() == itLow )
	{
		itLow = mapSmartDpC.lower_bound( ( *mapSmartDpC.rbegin() ).first );
	}

	bool bFound = false;

	for( std::multimap <int, CSmartDDpCData>::reverse_iterator riter = mapSmartDpC.rbegin(); riter != mapSmartDpC.rend(); riter++ )
	{
		if( riter->first <= iHighestSize )
		{
			itUp = riter.base();
			bFound = true;
			break;
		}
	}

	if( mapSmartDpC.end() == itUp && false == bFound )
	{
		itUp = mapSmartDpC.upper_bound( (*mapSmartDpC.begin()).first );
	}

	if( mapSmartDpC.end() != itLow && mapSmartDpC.begin() != itLow )
	{
		mapSmartDpC.erase( mapSmartDpC.begin(), itLow );
	}

	if( mapSmartDpC.end() != itUp )
	{
		mapSmartDpC.erase( itUp, mapSmartDpC.end() );
	}

	if( 0 == mapSmartDpC.size() )
	{
		_AbortSelectBest();
		return;
	}

	// The 'mapDelta' map will contain valves that are eligible (If flow is good).
	// The 'mapBadDelta' map will contain valves that are not eligible.
	// When a valve IS eligible, we insert it in the 'm_mapSmartDpC' to be able to show all possibilities when opening the name combo box in 'CSheetHMCalc' or
	// 'CSheetPanelCirc2' dialog with the 'CDlgComboBoxHM' class.
	// When a valve IS NOT eligible, we insert it in the 'mapBadSmartDpC'
	// If we have no valve that ARE eligible AND if we have valve that ARE NOT eligible, we will show these "solutions" with error.
	// In this case, we will transfer the contain of the 'mapBadSmartDpC' map in the 'm_mapSmartDpC' map.
	std::multimap<double, CDB_SmartControlValve *> mapDelta;
	std::multimap<double, CDB_SmartControlValve *> mapBadDelta;
	std::multimap<int, CSmartDDpCData> mapBadSmartDpC;

	for( auto &iter : mapSmartDpC ) 
	{
		CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( iter.second.GetpSmartDpC() );

		if( NULL == pclSmartDpC )
		{
			ASSERT_CONTINUE;
		}

		int iSizeKey = pclSmartDpC->GetSizeKey();
		CDB_SmartValveCharacteristic *pclSmartValveCharacteristic = pclSmartDpC->GetSmartValveCharacteristic();

		if( NULL == pclSmartValveCharacteristic )
		{
			ASSERT_CONTINUE;
		}

		bool bEligible = true;

		// Check if the flow is correct for this valve.
		if( GetQ() < pclSmartValveCharacteristic->GetMinAdjustableFlow() || GetQ() > pclSmartValveCharacteristic->GetQnom() )
		{
			bEligible = false;
		}

		double dDelta = pclSmartValveCharacteristic->GetQnom() - GetQ();

		if( true == bEligible )
		{
			mapDelta.insert( std::make_pair( dDelta, pclSmartDpC ) );
			m_mapSmartDpC.insert( std::pair<int, CSmartDDpCData>( iter.first, iter.second ) );
		}
		else
		{
			mapBadDelta.insert( std::make_pair( dDelta, pclSmartDpC ) );
			mapBadSmartDpC.insert( std::pair<int, CSmartDDpCData>( iter.first, iter.second ) );
		}

		// Add data.
		iter.second.SetQMax( GetQ() );
	}

	int iCurrentSize = -1;
	CDB_TAProduct *pCurrentTAP = GetpTAP();

	// When user change the current body material, connection or PN the current selected product is invalid.
	// Try to find matching (Body material, connection & PN) valve in the full list of suggested products.
	bool bUserChangeSelParameters = false;

	if( NULL != m_pUserChoiceBodyMaterial || NULL != m_pUserChoiceConnection || NULL != m_pUserChoicePN )
	{
		bUserChangeSelParameters = true;
		pCurrentTAP = NULL;
	}

	if( NULL != pCurrentTAP )
	{
		iCurrentSize = pCurrentTAP->GetSizeKey();
	}

	// Build sorting keys.
	// From technical parameters below DN65.
	CPrjParams *pPrjParams = m_pParent->GetpPrjParam()->GetpHmCalcParams();
	CDB_StringID *psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartDpCBlw65BdyMatID ).MP );
	CDB_StringID *psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartDpCBlw65ConnectID ).MP );
	CDB_StringID *psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartDpCBlw65PNID ).MP );
	CTAPSortKey sKeyTechParamBlw65( NULL, NULL, NULL, psidBdy, psidConn, NULL, psidPN, 0 );

	CTAPSortKey sKeyUserChangeSelParamsBlw65( NULL, NULL, NULL, m_pUserChoiceBodyMaterial, m_pUserChoiceConnection, NULL, m_pUserChoicePN, 0 );

	// From technical parameters above DN50.
	psidBdy = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartDpCAbv50BdyMatID ).MP );
	psidConn = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartDpCAbv50ConnectID ).MP );
	psidPN = dynamic_cast<CDB_StringID *>( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartDpCAbv50PNID ).MP );
	CTAPSortKey sKeyTechParamAbv50( NULL, NULL, NULL, psidBdy, psidConn, NULL, psidPN, 0 );

	CTAPSortKey sKeyUserChangeSelParamsAbv50( NULL, NULL, NULL, m_pUserChoiceBodyMaterial, m_pUserChoiceConnection, NULL, m_pUserChoicePN, 0 );

	double dKeyCurrentTAP = 0;
	int strtLoop = ( true == bUserChangeSelParameters ) ? strtLoop = -2 : strtLoop = 0;
	int endLoop = 4;
	bFound = false;
	std::multimap<double, CDB_SmartControlValve *>::iterator It = mapDelta.end();
	std::multimap<double, CDB_SmartControlValve *> *pmapDelta = &mapDelta;

	if( 0 == (int)mapDelta.size() && mapBadDelta.size() > 0 )
	{
		pmapDelta = &mapBadDelta;
		m_mapSmartDpC = mapBadSmartDpC;
	}

	if( pmapDelta->size() > 0 )
	{
		for( int UserPrefLoop = strtLoop; UserPrefLoop < endLoop && false == bFound; UserPrefLoop++ )
		{
			// This loop give us the possibility to relax mask criteria one by one.
			// we will start with the most constraining key and if we don't found a product we will relax constrains one by one.
			CTAPSortKey::TAPSortKeyMask eMask = ( CTAPSortKey::TAPSortKeyMask )( CTAPSortKey::TAPSKM_All & ~CTAPSortKey::TAPSKM_PriorL & ~CTAPSortKey::TAPSKM_Size
					& ~CTAPSortKey::TAPSKM_Type & ~CTAPSortKey::TAPSKM_Fam & ~CTAPSortKey::TAPSKM_Version );

			switch( UserPrefLoop )
			{
				case -2:
				{
					// We will try to find a matching product keeping family and body material as defined
					if( NULL == m_pUserChoicePN )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
					}

					if( NULL == m_pUserChoiceConnection )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Connect );
					}

					if( NULL == m_pUserChoiceBodyMaterial )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Bdy );
					}

				}
				break;

				case -1:
				{
					// ( true == bUserChangeSelParameters )
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Fam );
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Type );

					if( NULL == m_pUserChoicePN )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
					}

					if( NULL == m_pUserChoiceConnection )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Connect );
					}

					if( NULL == m_pUserChoiceBodyMaterial )
					{
						eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_Bdy );
					}
				}
				break;

				case 0:
				{
					// All
					if( true == bUserChangeSelParameters )
					{
						// No solution found relax user request.
						bUserChangeSelParameters = false;
						ClearUserChoiceTemporaryVariables();
					}
				}
				break;

				// Relax PN.
				case 1:
					
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN );
					break;

				// Relax PN and connection.
				case 2:
					
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Connect );
					break;

				// Relax PN, connection and body material.
				case 3:
					eMask = ( CTAPSortKey::TAPSortKeyMask )( eMask & ~CTAPSortKey::TAPSKM_PN & ~CTAPSortKey::TAPSKM_Connect & ~CTAPSortKey::TAPSKM_Bdy );
					break;
			}

			double dKeyTechParamBlw65 = sKeyTechParamBlw65.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
			double dKeyTechParamAbv50 = sKeyTechParamAbv50.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
			double dKeyUserChangeSelParamsBlw65 = sKeyUserChangeSelParamsBlw65.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );
			double dKeyUserChangeSelParamsAbv50 = sKeyUserChangeSelParamsAbv50.GetSortingKeyAsDouble( &CTAPSortKey::CTAPSKMask( eMask ) );

			// Extract key for currently selected TAP
			if( NULL != pCurrentTAP )
			{
				dKeyCurrentTAP = pCurrentTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
			}

			if( true == bUserChangeSelParameters )
			{
				std::multimap<double, CDB_SmartControlValve *>::iterator iterBestPriority = pmapDelta->end();

				for( It = pmapDelta->begin(); It != pmapDelta->end(); It++ )
				{
					CDB_TAProduct *pTAP = It->second;
					int iSize = pTAP->GetSizeKey();
					double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );
					double dKey = ( iSize <= iDN50 ) ? dKeyUserChangeSelParamsBlw65 : dKeyUserChangeSelParamsAbv50;

					if( dTAPKey == dKey )
					{
						if( iterBestPriority != pmapDelta->end() )
						{
							if( iterBestPriority->second->GetPriorityLevel() > pTAP->GetPriorityLevel() )
							{
								iterBestPriority = It;
								bFound = true;
							}
						}
						else
						{
							iterBestPriority = It;
							bFound = true;
						}
					}
				}

				if( true == bFound )
				{
					It = iterBestPriority;
				}
			}
			else
			{
				// If a current selection exist
				if( NULL != pCurrentTAP )
				{
					for( It = pmapDelta->begin(); It != pmapDelta->end(); ++It )
					{
						CDB_TAProduct *pTAP = It->second;
						int iSize = pTAP->GetSizeKey();
						double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );

						if( ( iCurrentSize <= iDN50 && iSize <= iDN50 ) || ( iCurrentSize > iDN50 && iSize > iDN50 ) )
						{
							// Selected valve exist and it's located on the same side of iDN50 border.
							// Try to find a valve that match all requested criteria.
							if( dTAPKey == dKeyCurrentTAP )
							{
								bFound = true;
							}
						}
						else
						{
							// Selected valve is not located on the same side of iDN50 border.
							// Use defined technical choice
							if( iSize <= iDN50 )
							{
								if( dTAPKey == dKeyTechParamBlw65 )
								{
									bFound = true;
								}
							}
							else
							{
								// Size > DN50
								if( dTAPKey == dKeyTechParamAbv50 )
								{
									bFound = true;
								}
							}
						}

						if( true == bFound )
						{
							// Abort loop by this way we preserve Iterator
							break;
						}
					}
				}

				// Not found or previous valve doesn't exist, restart loop only on tech params.
				if( false == bFound )
				{
					for( It = pmapDelta->begin(); It != pmapDelta->end(); ++It )
					{
						CDB_TAProduct *pTAP = It->second;
						int iSize = pTAP->GetSizeKey();
						double dTAPKey = pTAP->GetSortingKey( &CTAPSortKey::CTAPSKMask( eMask ) );

						// Selected valve is not located on the same side of iDN50 border.
						// Use defined technical choice
						if( iSize <= iDN50 )
						{
							if( dTAPKey == dKeyTechParamBlw65 )
							{
								bFound = true;
							}
						}
						else
						{
							// Size > DN50
							if( dTAPKey == dKeyTechParamAbv50 )
							{
								bFound = true;
							}
						}

						if( true == bFound )
						{
							// Abort loop and by this way we preserve Iterator
							break;
						}
					}
				}
			}
		}
	}

	// If best valve found...
	if( true == bFound )
	{
		CDB_TAProduct *pTAP = It->second;

		if( NULL != pTAP )
		{
			SetBestIDPtr( pTAP->GetIDPtr() );
			_SetpSelSmartDpC( (LPARAM)pTAP );

			// Now we can check which Dp sensor (Or dp sensor set) to validate.
			_SelectBestDpSensor();
			_VerifySetContent();

			if( true == bUserChangeSelParameters )
			{
				// When the current product is valid we will analyze if the user introduced an exception regarding defined PrjParam,
				// in this case we will not consider the DN50 border that force an automatic jump.
				if( pTAP->GetSizeKey() <= iDN50 )
				{
					if( ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartDpCBlw65BdyMatID ).MP == pTAP->GetBodyMaterialIDPtr().MP )
							&& ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartDpCBlw65ConnectID ).MP == pTAP->GetConnectIDPtr().MP )
							&& ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartDpCBlw65PNID ).MP == pTAP->GetPNIDPtr().MP ) )
					{
						ClearUserChoiceTemporaryVariables();
					}
				}
				else
				{
					// DN65 and above.
					if( ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartDpCAbv50BdyMatID ).MP == pTAP->GetBodyMaterialIDPtr().MP )
							&& ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartDpCAbv50ConnectID ).MP == pTAP->GetConnectIDPtr().MP )
							&& ( pPrjParams->GetPrjParamIDPtr( CPrjParams::_PrjParamsID::SmartDpCAbv50PNID ).MP == pTAP->GetPNIDPtr().MP ) )
					{
						ClearUserChoiceTemporaryVariables();
					}
				}
			}
		}
		else
		{
			InvalidateSelection();
			ClearUserChoiceTemporaryVariables();
		}
	}
	else
	{
		InvalidateSelection();
		ClearUserChoiceTemporaryVariables();
	}
}

int CDS_HydroMod::CSmartDpC::GetAvailableBodyMaterial( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapSmartDpC.size() )
	{
		return 0;
	}

	pmap->clear();
	std::multimap <int, CSmartDDpCData >::iterator It;

	for( It = m_mapSmartDpC.begin(); It != m_mapSmartDpC.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpSmartDpC();
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetBodyMaterialIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

int CDS_HydroMod::CSmartDpC::GetAvailableConnection( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapSmartDpC.size() )
	{
		return 0;
	}

	pmap->clear();

	CDB_StringID *pStrIDBodyMaterial = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDBodyMaterial = dynamic_cast<CDB_StringID *>( GetpTAP()->GetBodyMaterialIDPtr().MP );
	}

	std::multimap <int, CSmartDDpCData >::iterator It;

	for( It = m_mapSmartDpC.begin(); It != m_mapSmartDpC.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpSmartDpC();
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetBodyMaterialIDPtr().MP );

		if( pStrID != pStrIDBodyMaterial )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

int CDS_HydroMod::CSmartDpC::GetAvailablePN( std::map<int, CDB_StringID *> *pmap )
{
	if( NULL == pmap )
	{
		return 0;
	}

	if( 0 == m_mapSmartDpC.size() )
	{
		return 0;
	}

	pmap->clear();

	CDB_StringID *pStrIDBodyMaterial = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDBodyMaterial = dynamic_cast<CDB_StringID *>( GetpTAP()->GetBodyMaterialIDPtr().MP );
	}

	CDB_StringID *pStrIDConnection = NULL;

	if( NULL != GetpTAP() )
	{
		pStrIDConnection = dynamic_cast<CDB_StringID *>( GetpTAP()->GetConnectIDPtr().MP );
	}

	std::multimap <int, CSmartDDpCData >::iterator It;

	for( It = m_mapSmartDpC.begin(); It != m_mapSmartDpC.end(); ++It )
	{
		CDB_TAProduct *pTAP = It->second.GetpSmartDpC();

		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetBodyMaterialIDPtr().MP );

		if( pStrID != pStrIDBodyMaterial )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetConnectIDPtr().MP );

		if( pStrID != pStrIDConnection )
		{
			continue;
		}

		pStrID = dynamic_cast<CDB_StringID *>( pTAP->GetPNIDPtr().MP );
		int iKey = 0;
		pStrID->GetIDstrAs( 0, iKey );
		( *pmap ) [iKey] = pStrID;
	}

	return pmap->size();
}

CDB_StringID *CDS_HydroMod::CSmartDpC::GetpSelType()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( (CDB_SmartControlValve *) GetIDPtr().MP )->GetTypeIDPtr().MP ) ;
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CSmartDpC::GetpSelBodyMaterial()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( (CDB_SmartControlValve *) GetIDPtr().MP )->GetBodyMaterialIDPtr().MP ) ;
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CSmartDpC::GetpSelConnection()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( ( CDB_ShutoffValve * ) GetIDPtr().MP )->GetConnectIDPtr().MP );
	}

	return p;
}

CDB_StringID *CDS_HydroMod::CSmartDpC::GetpSelPN()
{
	CDB_StringID *p = NULL;

	if( 0 != *GetIDPtr().ID )
	{
		p = static_cast< CDB_StringID * >( ( ( CDB_ShutoffValve * ) GetIDPtr().MP )->GetPNIDPtr().MP );
	}

	return p;
}

void CDS_HydroMod::CSmartDpC::SetpSelBodyMaterialAccordingUserChoice( CDB_StringID *pSelBodyMaterial )
{
	if( NULL != pSelBodyMaterial && false == pSelBodyMaterial->IsClass( CLASS( CDB_StringID ) ) )
	{
		ASSERT( false );
		pSelBodyMaterial = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceBodyMaterial = pSelBodyMaterial;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceSmartControlValveConnect, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CSmartDpC::SetpSelConnectionAccordingUserChoice( CDB_StringID *pSelConnection )
{
	if( NULL != pSelConnection && false == pSelConnection->IsClass( CLASS( CDB_Connect ) ) )
	{
		ASSERT( false );
		pSelConnection = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceBodyMaterial = GetpSelBodyMaterial();
	m_pUserChoiceConnection = pSelConnection;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceSmartControlValvePN, (LPARAM)this );
	m_pParent->Modified();
}

void CDS_HydroMod::CSmartDpC::SetpSelPNAccordingUserChoice( CDB_StringID *pSelPN )
{
	if( NULL != pSelPN && false == pSelPN->IsClass( CLASS( CDB_StringID ) ) )
	{
		ASSERT( false );
		pSelPN = NULL;
	}

	ClearUserChoiceTemporaryVariables();
	m_pUserChoiceBodyMaterial = GetpSelBodyMaterial();
	m_pUserChoiceConnection = GetpSelConnection();
	m_pUserChoicePN = pSelPN;
	m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceSmartControlValve, (LPARAM)this );
	m_pParent->Modified();
}

CString CDS_HydroMod::CSmartDpC::GetStrLocalization()
{
	SmartValveLocalization eLoc = SmartValveLocalization::SmartValveLocNone;
	CString strReturn = _T("");

	if( NULL == dynamic_cast<CDB_SmartControlValve *>( GetIDPtr().MP ) )
	{
		return _T("");
	}

	CDB_SmartControlValve *pclSmartDpC = (CDB_SmartControlValve *)( GetIDPtr().MP );
	return pclSmartDpC->GetLocalizationString( GetLocalization() );
}

double CDS_HydroMod::CSmartDpC::GetDp()
{
	if( m_dDp < 0.0 )
	{
		m_dDp = 0.0;
	}

	return m_dDp;
}

double CDS_HydroMod::CSmartDpC::GetDpMin()
{
	if( NULL == GetIDPtr().MP )
	{
		return 0.0;
	}

	if( 0.0 == GetQ() )
	{
		return 0.0;
	}

	CWaterChar *pclWaterChar = m_pParent->GetpWaterChar( CDS_HydroMod::eHMObj::eSmartDpC );
	
	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	CDB_SmartControlValve *pclSmartDpC = static_cast<CDB_SmartControlValve *>( GetIDPtr().MP );

	double dQ = GetQ();
	double dKvs = pclSmartDpC->GetKvs();
	double dRho = pclWaterChar->GetDens();
	double dDpMin = 0.0;

	if( dQ > 0.0 && dKvs > 0.0 )
	{
		dDpMin = CalcDp( dQ, dKvs, dRho );
	}
	
	return dDpMin;
}

double CDS_HydroMod::CSmartDpC::GetDpToStabilize()
{
	CDB_CircuitScheme *pSch = m_pParent->GetpSch();

	if( NULL == pSch || NULL == pSch->GetpSchCateg() )
	{
		return 0.0;
	}

	return m_pParent->GetDpOnOutOfPrimSide();
}

double CDS_HydroMod::CSmartDpC::GetDplmin()
{
	double dDplmin = -1.0;

	CDB_DpSensor *pclDpSensor = GetpDpSensor();

	if( NULL != pclDpSensor )
	{
		dDplmin = pclDpSensor->GetMinMeasurableDp();
	}

	return dDplmin;
}

double CDS_HydroMod::CSmartDpC::GetDplmax()
{
	double dDplmax = -1.0;

	CDB_DpSensor *pclDpSensor = GetpDpSensor();

	if( NULL != pclDpSensor )
	{
		dDplmax = pclDpSensor->GetMaxMeasurableDp();
	}

	return dDplmax;
}

double CDS_HydroMod::CSmartDpC::GetFlowMax()
{
	return m_pParent->GetQ( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
}

double CDS_HydroMod::CSmartDpC::GetDT()
{
	CWaterChar *pclSupplyWaterChar = m_pParent->GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );

	if( NULL == pclSupplyWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	CWaterChar *pclReturnWaterChar = m_pParent->GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );

	if( NULL == pclReturnWaterChar )
	{
		ASSERTA_RETURN( 0.0 );
	}

	return abs( pclSupplyWaterChar->GetTemp() - pclReturnWaterChar->GetTemp() );
}

CDB_ValveCharacteristic *CDS_HydroMod::CSmartDpC::GetpValveChar()
{
	CDB_ValveCharacteristic *pDevChar = NULL;

	if( NULL != GetIDPtr().MP )
	{
		pDevChar = ( (CDB_TAProduct *)GetIDPtr().MP )->GetValveCharacteristic();
	}

	return pDevChar;
}

// HYS-1914: Dp is equal to Dp min except if Havail at the entry of the circuit increases, in that case it's the valve that takes the difference.
void CDS_HydroMod::CSmartDpC::SetDp( double dDp )
{
	if( dDp != GetDp() )
	{
		m_dDp = dDp;
		m_pParent->Modified();
	}
}

bool CDS_HydroMod::CSmartDpC::IsCompletelyDefined()
{
	if( NULL == GetIDPtr().MP )				// safe IDPtr, checked by Extend in GetIDPtr()
	{
		return false;
	}

	if( 0.0 == GetDp() )
	{
		return false;
	}

	return true;
}

void CDS_HydroMod::CSmartDpC::ResizeValve( double dHAvailable )
{
	if( 0.0 == GetQ() )
	{
		InvalidateSelection();
		return;
	}

	if( false == m_pParent->IsLocked( eHMObj::eSmartDpC ) )
	{
		SelectBest();
	}

	CDB_SmartControlValve *pclSmartDpC = (CDB_SmartControlValve *)( GetIDPtr().MP );

	if( NULL == pclSmartDpC )
	{
		// Nothing found, we return here.
		return;
	}

	double dDp = dHAvailable;

	if( dDp < GetDpMin() )
	{
		dDp = GetDpMin();
	}

	SetDp( dDp );

	_SelectBestDpSensor();
	_VerifySetContent();
}

//////////////////////////////////////////////////////////////////////
//			CDS_HM2W
//////////////////////////////////////////////////////////////////////

CDS_Hm2W::CDS_Hm2W( CDataBase *pDataBase, LPCTSTR ID ) : CDS_HydroMod( pDataBase, ID )
{
	m_ComputeHM2WState = CDS_HydroMod::eceNone;
	m_ComputeHM2WParam = 0;
	m_eUseShutoffValve = eUseProduct::Always;
	m_eSVObj = eHMObj::eNone;
	m_SavedSVIDPtr = _NULL_IDPTR;
}

void CDS_Hm2W::Init( IDPTR SchIDPtr, CTable *plcParent, bool bUseTACV, bool bCreateLinkedDevice )
{
	CDS_HydroMod::Init( SchIDPtr, plcParent, true );

	CDB_CircuitScheme *pSch = static_cast<CDB_CircuitScheme *>( SchIDPtr.MP );

	if( NULL != pSch )
	{
		if( CvLocation::CvLocNone != pSch->GetCvLoc() )
		{
			CreateCv( pSch->GetCvLoc() );
			ASSERT( NULL != GetpCV() );

			GetpCV()->SetTACv( bUseTACV );
		}
		else if( true == pSch->IsAnchorPtExist( CAnchorPt::SmartControlValve ) )
		{
			CreateSmartControlValve( pSch->GetSmartControlValveLoc() );
			ASSERT( NULL != GetpSmartControlValve() );
		}

		if( true == pSch->IsAnchorPtExist( CAnchorPt::ShutoffValve ) )
		{
			// By default we create this valve even if this one is optional.
			m_eUseShutoffValve = eUseProduct::Always;
			m_eSVObj = ( ShutoffValveLocSupply == pSch->GetShutoffValveLoc() ) ? eHMObj::eShutoffValveSupply : eHMObj::eShutoffValveReturn;
			CreateSV( m_eSVObj );
		}
		else
		{
			m_eUseShutoffValve = eUseProduct::Never;
			m_eSVObj = eHMObj::eNone;
		}
	}
}

void CDS_Hm2W::Copy( CDS_Hm2W *pHM2W, bool bValveDataOnly, bool bCreateLinkedDevice, bool bToClipboard, bool bResetDiversityFactor )
{
	pHM2W->SetSchemeID( GetSchemeID() );
	CDS_HydroMod::Copy( pHM2W, bValveDataOnly, bCreateLinkedDevice, bToClipboard, bResetDiversityFactor );

	pHM2W->EnableComputeHM( false );
	pHM2W->SetLock( m_usLock );

	// Flag 'm_eUseShutoffValve' should be set after the shutoff valve creation.
	pHM2W->SetUseShutoffValve( m_eUseShutoffValve, m_eSVObj );

	pHM2W->SetLock( m_usLock );
	pHM2W->EnableComputeHM( true );

}

void CDS_Hm2W::Copy( CData *pTo )
{
	( (CDS_HydroMod *)pTo )->SetSchemeIDPtr( GetSchemeIDPtr() );
	
	CDS_Hm2W::Copy( (CDS_Hm2W *)pTo, false, true, false, false );
	
	( (CDS_HydroMod *)pTo )->EnableComputeHM( false );
	( (CDS_HydroMod *)pTo )->SetLock( m_usLock );
	
	CDS_Hm2W *pTo2W = (CDS_Hm2W *)pTo;
	pTo2W->SetUseShutoffValve( GetUseShutoffValve(), GetShutoffValveLocation() );

	( (CDS_HydroMod *)pTo )->SetLock( m_usLock );
	( (CDS_HydroMod *)pTo )->EnableComputeHM( true );
}

#define CDS_HM2W_VERSION	2
// Version 2: 2016-09-15: add 'm_UseShutoffValve' variable.
void CDS_Hm2W::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_HM2W_VERSION;
	WriteData<>( outf, Version );

	// Base class.
	CDS_HydroMod::Write( outf );

	// Version 2.
	WriteData<>( outf, m_eUseShutoffValve );
	WriteData<>( outf, m_eSVObj );
}

bool CDS_Hm2W::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_HM2W_VERSION;
	ReadData<>( inpf, Version );

	if( Version < 1 || Version > CDS_HM2W_VERSION )
	{
		return false;
	}

	// Base class.
	if( false == CDS_HydroMod::Read( inpf ) )
	{
		return false;
	}

	// Info.

	if( Version < 2 )
	{
		return true;
	}

	// Version 2.
	ReadData<>( inpf, m_eUseShutoffValve );
	ReadData<>( inpf, m_eSVObj );

	return true;
}

// Compute a new setting point for balancing device in function of Havail.
void CDS_Hm2W::ResizeBalDevForH( double dHAvail, bool bForPending, CAnchorPt::CircuitSide eCircuitSide, bool *pbHasHAvailSecChanged )
{
	if( dHAvail > 0.0 && NULL == GetpDpC() )
	{
		if( NULL != GetpCV() )
		{
			if( false == GetpCV()->IsPresettable() )
			{
				dHAvail -= GetpCV()->GetDp();
			}
		}
	}

	CDS_HydroMod::ResizeBalDevForH( dHAvail, bForPending, eCircuitSide, pbHasHAvailSecChanged );

	// Create or delete shutoff valve if needed.
	SetUseShutoffValve( m_eUseShutoffValve, m_eSVObj );
}

void CDS_Hm2W::SetUseShutoffValve( eUseProduct eUseShutoffValve, eHMObj eWhichSV )
{
	if( ( eShutoffValveSupply != eWhichSV && eShutoffValveReturn != eWhichSV ) || NULL == GetpShutoffValve( eWhichSV ) )
	{
		return;
	}

	if( m_eUseShutoffValve != eUseShutoffValve )
	{
		Modified();
	}

	m_eUseShutoffValve = eUseShutoffValve;
	m_eSVObj = eWhichSV;

	switch( m_eUseShutoffValve )
	{
		case eUseProduct::Always:

			if( NULL == GetpShutoffValve( m_eSVObj )->GetIDPtr( ).MP )
			{
				GetpShutoffValve( m_eSVObj )->SetIDPtr( m_SavedSVIDPtr );
			}

			ComputeHM( eceShutoffValveAdded );
			ResetSchemeIDPtr();
			break;

		case eUseProduct::Never:

			if( _T( '\0' ) != *GetpShutoffValve( m_eSVObj )->GetIDPtr().ID )
			{
				SetLock( m_eSVObj, false, false, true );
				m_SavedSVIDPtr = GetpShutoffValve( m_eSVObj )->GetIDPtr( );
				GetpShutoffValve( m_eSVObj )->SetIDPtr( _NULL_IDPTR );
				ResetSchemeIDPtr();
			}

			break;

		case eUseProduct::Auto:
			// No automatic mode for shutoff valve.
			break;
	}
}

void CDS_Hm2W::ComputeHM( eComputeHMEvent ComputeHMEvent, LPARAM pObj )
{
	if( false == IsHMCalcMode() )
	{
		return;
	}

	// During HydroMod copy Compute HydroMod is deactivated.
	if( false == m_bComputeAllowed )
	{
		return;
	}

	// Sanity tests.
	if( m_ComputeHM2WState > eceLAST || m_ComputeHM2WState < eceNone )
	{
		ASSERT( 0 );
		m_ComputeHM2WState = eceNone;
	}

	// If the engine is already running discard new event.
	if( m_ComputeHM2WState != eceNone && m_ComputeHM2WState != eceLAST )
	{
		return;
	}

	_SetComputeHM2WEngine( ComputeHMEvent, pObj );

	// Compute base.
	CDS_HydroMod::ComputeHM( ComputeHMEvent, pObj );

	bool bAllShouldBeSized = false;

	while( m_ComputeHM2WState != eceNone && m_ComputeHM2WState != eceLAST )
	{
		switch( m_ComputeHM2WState )
		{
			case eceTUDp:
			case eceResize:
			case eceTUQ:
			case eceShutoffValveAdded:

				if( NULL != GetpShutoffValve( CDS_HydroMod::eShutoffValveSupply ) )
				{
					// Sizing shutoff valve with same flow that flows in TU.
					GetpShutoffValve( CDS_HydroMod::eShutoffValveSupply )->SetQ( GetpTermUnit()->GetQ() );

					// Call base loop to size bypass Bv.
					CDS_HydroMod::ComputeHM( eceShutoffValveType, (LPARAM)GetpShutoffValve( CDS_HydroMod::eShutoffValveSupply ) );
				}
				else if( NULL != GetpShutoffValve( CDS_HydroMod::eShutoffValveReturn ) )
				{
					// Sizing shutoff valve with same flow that flows in TU.
					GetpShutoffValve( CDS_HydroMod::eShutoffValveReturn )->SetQ( GetpTermUnit()->GetQ() );

					// Call base loop to size bypass Bv.
					CDS_HydroMod::ComputeHM( eceShutoffValveType, (LPARAM)GetpShutoffValve( CDS_HydroMod::eShutoffValveReturn ) );
				}

				_SetComputeHM2WEngine( eceLAST );
				break;

			default:
				_SetComputeHM2WEngine( eceLAST );
				break;
		}
	}
}

bool CDS_Hm2W::IsCompletelyDefined( int iHMObjToCheck )
{
	if( NULL != GetpSch() && true == GetpSch()->IsAnchorPtOptional( CAnchorPt::ShutoffValve ) && ( eHMObj::eShutoffValveSupply == m_eSVObj || eHMObj::eShutoffValveReturn == m_eSVObj )
			&& eUseProduct::Always != m_eUseShutoffValve )
	{
		// In this case, user has intentionally chosen to not have the shutoff valve and then we don't verify it.
		iHMObjToCheck &= ~( ( int )m_eSVObj );
	}

	return CDS_HydroMod::IsCompletelyDefined( iHMObjToCheck );
}

void CDS_Hm2W::TransferValvesInfoFromHMX( CDS_HydroModX *pHMX, int iHMObjToCheck )
{
	if( NULL == pHMX )
	{
		ASSERT( 0 );
		return;
	}

	CDB_CircuitScheme *pCircuitSchemeHMX = pHMX->GetpSch();

	if( NULL == pCircuitSchemeHMX )
	{
		ASSERT( 0 );
		return;
	}

	if( true == pCircuitSchemeHMX->IsAnchorPtExist( CAnchorPt::ShutoffValve ) )
	{
		ShutoffValveLoc eShutoffValveLocation = pCircuitSchemeHMX->GetShutoffValveLoc();
		CDS_HydroModX::CShutoffValve *pHMXShutoffValve = pHMX->GetpShutoffValve( eShutoffValveLocation );

		if( NULL != pHMXShutoffValve )
		{
			eHMObj HMObj = ( ShutoffValveLocSupply == eShutoffValveLocation ) ? eHMObj::eShutoffValveSupply : eHMObj::eShutoffValveReturn;

			// Complete information for the newly created shut-off valve.
			CDS_HydroMod::CShutoffValve *pHMShutoffValve = GetpShutoffValve( HMObj );

			if( NULL != pHMShutoffValve )
			{
				pHMShutoffValve->SetIDPtr( pHMXShutoffValve->GetValveIDPtr() );
				m_eUseShutoffValve = ( CDS_HydroMod::eUseProduct )pHMXShutoffValve->GetUseProduct();
				m_eSVObj = HMObj;

				// Do not allow base class to manage BV bypass be cause it is already done here.
				iHMObjToCheck &= ~( ( int )HMObj );
			}
		}
		else
		{
			// If shutoff valve doesn't exist in the TA-Scope project to import, it means that this valve is optional and
			// user has not created one. In HySelect we keep shutoff valve and we change 'm_eUseShutoffValve' to specify
			// that user can select one (It is also to get the possibility to set a unchecked checkbox in 'SheetHMCalc').
			m_eUseShutoffValve = CDS_HydroMod::eUseProduct::Never;
			m_eSVObj = CDS_HydroMod::eNone;
		}
	}

	CDS_HydroMod::TransferValvesInfoFromHMX( pHMX, iHMObjToCheck );
}

//////////////////////////////////////////////////////////////////////
//			CDS_HmInj
//////////////////////////////////////////////////////////////////////
CDS_HmInj::CDS_HmInj( CDataBase *pDataBase, LPCTSTR ID )
	: CDS_HydroMod( pDataBase, ID )
{
	m_ComputeHMInjState = eceNone;
	m_ComputeHMInjParam = 0;
	m_bComputeHMInjAllowed = true;
	m_dDesignCircuitSupplySecondaryTemperature = -273.15;
	m_dDesignCircuitReturnSecondaryTemperature = -273.15;
}

void CDS_HmInj::SetDesignCircuitSupplySecondaryTemperature( double dDesignCircuitSupplySecondaryTemperature )
{
	m_dDesignCircuitSupplySecondaryTemperature = dDesignCircuitSupplySecondaryTemperature;

	// 
	if( NULL == m_pCircuitSecondaryPipe )
	{
		ASSERT_RETURN;
	}
	
	// Because DESIGN temperature is changed, we can directly update the temperature in the circuit supply pipe at the secondary side.
	m_pCircuitSecondaryPipe->SetTemperature( dDesignCircuitSupplySecondaryTemperature, CAnchorPt::PipeLocation_Supply );
	UpdateTUTemperaturesForInjectionCircuit();
}

void CDS_HmInj::SetDesignCircuitReturnSecondaryTemperature( double dDesignCircuitReturnSecondaryTemperature )
{
	m_dDesignCircuitReturnSecondaryTemperature = dDesignCircuitReturnSecondaryTemperature;
	
	if( NULL == m_pCircuitSecondaryPipe )
	{
		ASSERT_RETURN;
	}

	// Because DESIGN temperature is changed, we can directly update the temperature in the circuit return pipe at the secondary side.
	// The COMPUTED return temperature in the pipe will be updated with children in the next compute of the installation.
	m_pCircuitSecondaryPipe->SetTemperature( dDesignCircuitReturnSecondaryTemperature, CAnchorPt::PipeLocation_Return );
	// HYS-1882: this code is commented because m_dDesignCircuitReturnSecondaryTemperature does not exist and cannot be set by the user.
	// It saves a calculated value.
	// UpdateTUTemperaturesForInjectionCircuit();
}

CWaterChar *CDS_HmInj::GetpWaterChar( CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	CWaterChar *pclWaterChar = NULL;

	if( CAnchorPt::PipeType_Distribution == ePipeType || CAnchorPt::CircuitSide_Primary == eCircuitSide )
	{
		// Distribution or circuit at primary side are managed in the base 'CDS_HydroMod' class.
		return CDS_HydroMod::GetpWaterChar( ePipeType, ePipeLocation, eCircuitSide );
	}

	// We are with injection circuit.
	// 3-way mixing is not really an injection circuit but we have secondary pipes.

	if( CAnchorPt::PipeLocation::PipeLocation_Supply == ePipeLocation 
			|| CAnchorPt::PipeLocation::PipeLocation_Return == ePipeLocation )
	{
		CPipes *pclSecondaryPipe = GetpCircuitSecondaryPipe();

		if( NULL == pclSecondaryPipe )
		{
			ASSERTA_RETURN( NULL );
		}

		pclWaterChar = pclSecondaryPipe->GetpWaterChar( ePipeLocation );
	}
	else if( CAnchorPt::PipeLocation::PipeLocation_ByPass == ePipeLocation )
	{
		CPipes *pclByPassPipe = GetpCircuitBypassPipe();

		if( NULL == pclByPassPipe )
		{
			ASSERTA_RETURN( NULL );
		}

		pclWaterChar = pclByPassPipe->GetpWaterChar( ePipeLocation );
	}
	else
	{
		ASSERTA_RETURN( NULL );
	}

	if( NULL == pclWaterChar )
	{
		ASSERTA_RETURN( NULL );
	}

	return pclWaterChar;
}

CWaterChar *CDS_HmInj::GetpOutWaterChar( CAnchorPt::PipeLocation ePipeLocation )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	return GetpWaterChar( CAnchorPt::PipeType_Circuit, ePipeLocation, CAnchorPt::CircuitSide_Secondary );
}

double CDS_HmInj::GetTemperature( CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.

	if( CAnchorPt::PipeType_Distribution == ePipeType || CAnchorPt::CircuitSide_Primary == eCircuitSide )
	{
		// Distribution or circuit at primary side are managed in the base 'CDS_HydroMod' class.
		return CDS_HydroMod::GetTemperature( ePipeType, ePipeLocation, eCircuitSide );
	}

	double dTemperature = -217.15;

	if( CAnchorPt::PipeLocation::PipeLocation_Supply == ePipeLocation 
			|| CAnchorPt::PipeLocation::PipeLocation_Return == ePipeLocation )
	{
		CPipes *pclSecondaryPipe = GetpCircuitSecondaryPipe();

		if( NULL == pclSecondaryPipe )
		{
			ASSERTA_RETURN( -217.15 );
		}

		dTemperature = pclSecondaryPipe->GetTemperature( ePipeLocation );
	}
	else if( CAnchorPt::PipeLocation::PipeLocation_ByPass == ePipeLocation )
	{
		CPipes *pclByPassPipe = GetpCircuitBypassPipe();

		if( NULL == pclByPassPipe )
		{
			ASSERTA_RETURN( NULL );
		}

		dTemperature = pclByPassPipe->GetTemperature( ePipeLocation );
	}
	else
	{
		ASSERTA_RETURN( NULL );
	}

	if( -273.15 == dTemperature )
	{
		ASSERTA_RETURN( -217.15 );
	}

	return dTemperature;
}

double CDS_HmInj::GetOutTemperature( CAnchorPt::PipeLocation ePipeLocation )
{
	// This method is called by a children to know what is the output supply or return temperature of
	// the current hydraulic circuit.
	// If this method is called it's because we ARE with an injection circuit.
	// In this case we are thus with a module WITH secondary side.
	// So, the output supply or return temperature is considered to be the temperature of the fluid 
	// of the circuit at the secondary side.
	// ATTENTION! There is a difference betwen the 'GetOutTemperature' and 'GetDesignTemperature' methods for the return temperature.
	// The first method will return the COMPUTED return temperature. While the second method will return the DESIGN tempeature
	// (The temperature set by the user by example for a secondary side of an injection circuit.
	double dOutTemperature = -273.15;

	if( NULL == GetpSchcat() )
	{
		ASSERTA_RETURN( 0.0 );
	}

	// Except for 3-way mixing that is not really an injection circuit!
	if( CDB_CircSchemeCateg::e3wTypeMixing == GetpSchcat()->Get3WType() )
	{
		dOutTemperature = CDS_HydroMod::GetOutTemperature( ePipeLocation );
	}
	else
	{
		dOutTemperature = GetTemperature( CAnchorPt::PipeType_Circuit, ePipeLocation, CAnchorPt::CircuitSide_Secondary );
	}
	
	return dOutTemperature;
}

double CDS_HmInj::GetPower( CAnchorPt::CircuitSide eCircuitSide )
{
	if( CAnchorPt::CircuitSide_Secondary != eCircuitSide )
	{
		return CDS_HydroMod::GetPower( eCircuitSide );
	}

	double dQ = GetQ( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary );

	if( dQ <= 0.0 )
	{
		return 0.0;
	}

	CWaterChar *pclCircuitSupplySecondaryWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, eCircuitSide );

	if( NULL == pclCircuitSupplySecondaryWaterChar || 273.15 == pclCircuitSupplySecondaryWaterChar->GetTemp() )
	{
		ASSERTA_RETURN( 0.0 );
	}

	CWaterChar *pclCircuitReturnSecondaryWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, eCircuitSide );

	if( NULL == pclCircuitReturnSecondaryWaterChar || 273.15 == pclCircuitReturnSecondaryWaterChar->GetTemp() )
	{
		ASSERTA_RETURN( 0.0 );
	}

	double dPower = pclCircuitSupplySecondaryWaterChar->GetTemp() * pclCircuitSupplySecondaryWaterChar->GetDens() * pclCircuitSupplySecondaryWaterChar->GetSpecifHeat();
	dPower -= ( pclCircuitReturnSecondaryWaterChar->GetTemp() * pclCircuitReturnSecondaryWaterChar->GetDens() * pclCircuitReturnSecondaryWaterChar->GetSpecifHeat() );
	dPower *=  dQ;

	return abs( dPower );
}

void CDS_HmInj::VerifyWaterCharacteristic( CDS_HydroMod *pclPressureInterfaceHM, CDS_HydroMod *pclPreviousCircuit )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	// Normally, this method must not be manually called. It must be called only by the 'VerifyAllWaterCharacteristic'
	// in charge to verify a design temperature interface and all its children.

	// If current circuit is not in HM calc mode, we don't verify.
	if( false == IsHMCalcMode() )
	{
		return;
	}

	if( NULL == GetpTermUnit() )
	{
		ASSERT_RETURN;
	}

	if( NULL == pclPressureInterfaceHM )
	{
		pclPressureInterfaceHM = GetpDesignTemperatureInterface();
	}

	if( NULL == pclPressureInterfaceHM )
	{
		ASSERT_RETURN;
	}

	if( NULL == GetpSchcat() )
	{
		ASSERT_RETURN;
	}

	if( CDB_CircSchemeCateg::e3wTypeMixing != GetpSchcat()->Get3WType() )
	{
		// The injection circuit IS NOT a 3-way mixing circuit.

		// Here we have two cases:
		//	(1): we ARE on the design temperature interface:
		//		 (a) module: we verify the secondary side. The primary side will be managed later by the design temperature interface parent of this current module.
		//       (b) circuit: can't happen !
		//	(2): we ARE NOT on the design temperature interface:
		//       (a) module: we do nothing with the secondary side here because it has been already managed in the case (1). The base class will manage in
		//                  this case the primary side.
		//       (b) circuit: because it is not considered as a design temperature interface, the case (1) has not managed the secondary side. So we need to do it here.
		if( this == pclPressureInterfaceHM || false == IsaModule() )
		{
			// HYS-1882: Set water characteristics, it can be change by the user via the ribbon.
			CWaterChar clOutSupplyPressureInterfaceWaterChar;

			if( false == pclPressureInterfaceHM->GetDesignWaterChar( clOutSupplyPressureInterfaceWaterChar, pclPressureInterfaceHM, CAnchorPt::PipeLocation_Supply ) )
			{
				ASSERT_RETURN;
			}
			SetWaterChar( &clOutSupplyPressureInterfaceWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary );

			// In these cases (1a & 2b), we have treated all the children and are now in the design temperature interface itself or we are an injection circuit that is not a module.
			
			// Circuit supply secondary pipe: the temperature will be the same as the one that user has set.
			SetTemperature( m_dDesignCircuitSupplySecondaryTemperature, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary );
			
			CWaterChar clOutDesignReturnPressureInterfaceWaterChar;

			if( false == pclPressureInterfaceHM->GetDesignWaterChar( clOutDesignReturnPressureInterfaceWaterChar, pclPressureInterfaceHM, CAnchorPt::PipeLocation_Return ) )
			{
				ASSERT_RETURN;
			}

			SetWaterChar( &clOutDesignReturnPressureInterfaceWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );

			// Circuit return secondary pipe: if a children exist, the temperature will be the same as the one computed in the distribution return pipe.
			// Otherwise, we take the one defined by the user.
			double dCircuitReturnSecondaryTemperature = -273.15;
			CDS_HydroMod *pclFirstChildHM = GetFirstHMChild();

			if( NULL != pclFirstChildHM )
			{
				dCircuitReturnSecondaryTemperature = pclFirstChildHM->GetTemperature( CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );
				// HYS-1882: Update the m_dDesignCircuitReturnSecondaryTemperature
				m_dDesignCircuitReturnSecondaryTemperature = dCircuitReturnSecondaryTemperature;
			}
			else
			{
				dCircuitReturnSecondaryTemperature = m_dDesignCircuitReturnSecondaryTemperature;
			}

			SetTemperature( dCircuitReturnSecondaryTemperature, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );
		}
	}
	else
	{
		// Injection circuit IS a 3-way mixing circuit.

		// Circuit supply secondary pipe: we are in a particular case. Here this not really an injection circuit, the user can not set new temperatures
		// for the secondary side. So, the water characteristic and temperature are exactly the same as the circuit supply primary pipe. And, in this case
		// the circuit supply primary water characteristic is the same as the DESIGN one at the output supply of the design temperature interface.
		CWaterChar clOutSupplyPressureInterfaceWaterChar;

		if( false == pclPressureInterfaceHM->GetDesignWaterChar( clOutSupplyPressureInterfaceWaterChar, pclPressureInterfaceHM, CAnchorPt::PipeLocation_Supply ) )
		{
			ASSERT_RETURN;
		}
			
		// So, we put water characteristic and there is no need to update the temperature.
		SetWaterChar( &clOutSupplyPressureInterfaceWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary );

		// HYS-1882: When temperature changes we should update the design circuit supply secondary temperature.
		SetDesignCircuitSupplySecondaryTemperature( clOutSupplyPressureInterfaceWaterChar.GetTemp() );

		// Circuit return secondary pipe: we have the case of a circuit (1) or a module (2).
		// In both cases, the return water characteristic in the circuit return pipe at the secondary side will be exactly the same as the one in the
		// circuit return pipe at the primary side.
		// (1): with a terminal unit, the water characteristic will be the DESIGN one at the outlet return of the design temperature interface.
		//      We are well talking here the DESIGN water characteristic. If we call the 'GetpOutWateChar( CAnchorPt::PipeLocation::Return )' we will
		//      retrieve the water characteristic and the COMPUTED return temperature.
		// (2): With a module, either we have children and in this case we take the COMPUTED return water characteristic that we can found in the
		//      distribution return pipe of the first child. Or we do not have yet chidren and in this case we need to do as the case (1).
		CWaterChar clOutDesignReturnPressureInterfaceWaterChar;
			
		if( false == pclPressureInterfaceHM->GetDesignWaterChar( clOutDesignReturnPressureInterfaceWaterChar, pclPressureInterfaceHM, CAnchorPt::PipeLocation_Return ) )
		{
			ASSERT_RETURN;
		}

		// Case (1).
		SetWaterChar( &clOutDesignReturnPressureInterfaceWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );

		if( true == IsaModule() )
		{
			double dCircuitReturnSecondaryTemperature = -273.15;
			CDS_HydroMod *pclFirstChildHM = GetFirstHMChild();

			if( NULL != pclFirstChildHM )
			{
				// Case (2).
				dCircuitReturnSecondaryTemperature = pclFirstChildHM->GetTemperature( CAnchorPt::PipeType_Distribution, CAnchorPt::PipeLocation_Return );
				// HYS-1882: Update the m_dDesignCircuitReturnSecondaryTemperature
				m_dDesignCircuitReturnSecondaryTemperature = dCircuitReturnSecondaryTemperature;
				SetTemperature( dCircuitReturnSecondaryTemperature, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );
			}

			// If we do not have children, we let the DESIGN water characteristic and temperature set for the case (1). 
		}
	}

	// Call now the base class.
	CDS_HydroMod::VerifyWaterCharacteristic( pclPressureInterfaceHM, pclPreviousCircuit );
}

void CDS_HmInj::SetWaterChar( CWaterChar *pclWaterChar, CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	if( NULL == GetpSchcat() )
	{
		ASSERT_RETURN;
	}

	if( CAnchorPt::PipeType_Distribution == ePipeType || CAnchorPt::CircuitSide_Primary == eCircuitSide )
	{
		// Distribution or circuit at primary side are managed in the base 'CDS_HydroMod' class.
		CDS_HydroMod::SetWaterChar( pclWaterChar, ePipeType, ePipeLocation, eCircuitSide );
	}
	else
	{
		CPipes *pclSecondaryPipe = GetpCircuitSecondaryPipe();

		if( NULL == pclSecondaryPipe )
		{
			ASSERT_RETURN;
		}

		pclSecondaryPipe->SetWaterChar( pclWaterChar, ePipeLocation );
	}
}

void CDS_HmInj::SetTemperature( double dTemperature, CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide )
{
	// For details about this method, see the comments in the 'CDS_HydroMod.h'.
	if( NULL == GetpSchcat() )
	{
		ASSERT_RETURN;
	}
	
	if( CAnchorPt::PipeType_Distribution == ePipeType || CAnchorPt::CircuitSide_Primary == eCircuitSide 
			|| CDB_CircSchemeCateg::e3wTypeMixing == GetpSchcat()->Get3WType() )
	{
		// Distribution or circuit at primary side are managed in the base 'CDS_HydroMod' class.
		// 3-way mixing is not really an injection circuit!
		CDS_HydroMod::SetTemperature( dTemperature, ePipeType, ePipeLocation, eCircuitSide );
	}
	else
	{
		CPipes *pclSecondaryPipe = GetpCircuitSecondaryPipe();

		if( NULL == pclSecondaryPipe )
		{
			ASSERT_RETURN;
		}

		pclSecondaryPipe->SetTemperature( dTemperature, ePipeLocation );
	}
}

void CDS_HmInj::UpdateTUTemperaturesForInjectionCircuit()
{
	// HYS-1882: Return temperature is now calculated from DT. DT is from user.
	if( m_dDesignCircuitSupplySecondaryTemperature > -273.15 )
	{
		// Update return temperature when terminal unit and DT are defined.
		if( NULL != GetpTermUnit() )
		{
			m_bComputeAllowed = false;

			if( CTermUnit::_QType::PdT == GetpTermUnit()->GetQType() && GetpTermUnit()->GetDT() > 0 )
			{
				double dReturnTemp = (ProjectType::Cooling == m_pTechParam->GetProjectApplicationType()) ? 
					m_dDesignCircuitSupplySecondaryTemperature + GetpTermUnit()->GetDT() : m_dDesignCircuitSupplySecondaryTemperature - GetpTermUnit()->GetDT();

				SetDesignCircuitReturnSecondaryTemperature( dReturnTemp );
			}
			else if( CTermUnit::_QType::Q == GetpTermUnit()->GetQType() && GetpTermUnit()->GetDTFlowMode() > 0 )
			{
				// HYS-1769: If Q type is in flow mode, we keep a trace of DT changed in secondary temperatures for injection.
				// For which case? When editing an injection circuit with 'SSheetPanelCirc2' and the terminal unit is in flow mode,
				// if we change return temperature and go in Power/DT mode for the terminal unit, the DT is not updated.
				double dReturnTemp = (ProjectType::Cooling == m_pTechParam->GetProjectApplicationType()) ?
					m_dDesignCircuitSupplySecondaryTemperature + GetpTermUnit()->GetDTFlowMode() : m_dDesignCircuitSupplySecondaryTemperature - GetpTermUnit()->GetDTFlowMode();

				SetDesignCircuitReturnSecondaryTemperature( dReturnTemp );
			}

			m_bComputeAllowed = true;
		}
	}
}

void CDS_HmInj::InitAllWaterCharWithParent( CTable *pclParent, eHMObj eWhichPipe )
{
	// Call first the base class to initialiaze distribution pipes and circuit pipes at the primary side.
	CDS_HydroMod::InitAllWaterCharWithParent( pclParent, eWhichPipe );

	// HYS-1716: Set now the water characteristic in the circuit pipe on the secondary side in regards to the circuit pipe water characteristics
	// already set on the primary side.
	if( NULL == GetpSchcat() )
	{
		ASSERT_RETURN;
	}

	if( eHMObj::eCircuitSecondaryPipe == ( eHMObj::eCircuitSecondaryPipe & eWhichPipe ) )
	{
		// We consider by default that we have the same water characteristic at the seconcary side that the one at the primary side.
		// So, here it will be initialized with the same supply primary water characteristic than the primary side.
		CWaterChar *pclSupplyWaterChar = CDS_HydroMod::GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
	
		if( NULL == pclSupplyWaterChar )
		{
			ASSERT_RETURN;
		}
	
		SetWaterChar( pclSupplyWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary );
		m_dDesignCircuitSupplySecondaryTemperature = pclSupplyWaterChar->GetTemp();

		// For the same reason, here we put by default the same water characteristic as the return primary water characteristic at the primary side.
		CWaterChar *pclReturnWaterChar = CDS_HydroMod::GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );
		
		if( NULL == pclReturnWaterChar )
		{
			ASSERT_RETURN;
		}

		SetWaterChar( pclReturnWaterChar, CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );
		m_dDesignCircuitReturnSecondaryTemperature = pclReturnWaterChar->GetTemp();
	}
}

double CDS_HmInj::GetDpSecSide()
{
	// We are working on secondary side
	double dDp = GetpTermUnit()->GetDp();

	if( NULL != GetpSecBv() )
	{
		dDp += GetpSecBv()->GetDp();
	}

	if( NULL != GetpCircuitSecondaryPipe() )
	{
		dDp += GetpCircuitSecondaryPipe()->GetPipeDp();
		dDp += GetpCircuitSecondaryPipe()->GetSingularityTotalDp( false );
	}

	return dDp;
}

void CDS_HmInj::ResizeBalDevForH( double dHAvail, bool bForPending, CAnchorPt::CircuitSide eCircuitSide, bool *pbHasHAvailSecChanged )
{
	if( NULL != pbHasHAvailSecChanged )
	{
		*pbHasHAvailSecChanged = false;
	}

	if( 0.0 == dHAvail && CAnchorPt::CircuitSide::CircuitSide_Secondary == eCircuitSide )
	{
		if( NULL != GetpPump() )
		{
			if( NULL != GetpSchcat() && true == GetpSchcat()->IsSecondarySideExist() )
			{
				// For 3 way mixing circuit Pump Hmin is reseted when we are working on primary side
				if( CDB_CircSchemeCateg::e3wTypeMixing != GetpSchcat()->Get3WType() )
				{
					GetpPump()->SetHmin( 0.0 );

					if( NULL != pbHasHAvailSecChanged )
					{
						*pbHasHAvailSecChanged = true;
					}
				}
			}
		}

		if( NULL != GetpSecBv() )
		{
			GetpSecBv()->ResizeBv( 0.0 );
		}
	}
}

double CDS_HmInj::GetQ( CAnchorPt::PipeType ePipeType, CAnchorPt::PipeLocation ePipeLocation, CAnchorPt::CircuitSide eCircuitSide )
{
	if( NULL == GetpSchcat() )
	{
		ASSERTA_RETURN( 0.0 );
	}

	double dQ = 0.0;

	if( CAnchorPt::PipeType_Distribution == ePipeType || CAnchorPt::CircuitSide_Primary == eCircuitSide 
			|| CDB_CircSchemeCateg::e3wTypeMixing == GetpSchcat()->Get3WType() )
	{
		// Distribution or circuit at primary side are managed in the base 'CDS_HydroMod' class.
		// 3-way mixing is not really an injection circuit!
		dQ = CDS_HydroMod::GetQ( ePipeType, ePipeLocation, eCircuitSide );
	}
	else
	{
		CPipes *pclSecondaryPipe = GetpCircuitSecondaryPipe();

		if( NULL == pclSecondaryPipe )
		{
			ASSERTA_RETURN( 0.0 );
		}

		dQ = pclSecondaryPipe->GetRealQ();
	}

	return dQ;
}

double CDS_HmInj::ComputePrimaryFlow( double dSecondaryFlow )
{
	if( GetpTermUnit()->GetQ() <= 0.0 || NULL == GetpSch() )
	{
		return 0.0;
	}

	double dPrimaryFlow = 0.0;

	if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() )
	{
		// HYS-1930: auto-adapting variable flow decoupling circuit.

		// In this circuit, we just need to have 1% additional of the flow in the bypass.
		dPrimaryFlow = dSecondaryFlow * 1.01;
	}
	else
	{
		// HYS-1746: Energy conversation:
		// Qp.Rho(tsp).Cp(tsp).tsp + Qb.Rho(trs).Cp(trs).trs = Qs.Rho(tss).Cp(tss).tss
		// Qp = ( [Rho(tss).Cp(tss).tss - Rho(trs).Cp(trs).trs] / [Rho(tsp).Cp(tsp).tsp - Rho(trs).Cp(trs).trs] ) / Qs

		CWaterChar *pclPrimarySupplyWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
		CWaterChar *pclSecondarySupplyWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Secondary );
		CWaterChar *pclSecondaryReturnWaterChar = GetpWaterChar( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );

		if( NULL == pclPrimarySupplyWaterChar || NULL == pclSecondarySupplyWaterChar || NULL == pclSecondaryReturnWaterChar )
		{
			ASSERTA_RETURN( 0.0 );
		}
	
		double dSupplyPrimaryTemperature = pclPrimarySupplyWaterChar->GetTemp();
		double dSupplySecondaryTemperature = pclSecondarySupplyWaterChar->GetTemp();
		double dReturnSecondaryTemperature = pclSecondaryReturnWaterChar->GetTemp();

		if( -273.15 == dSupplyPrimaryTemperature || -273.15 == dSupplySecondaryTemperature || -273.15 == dReturnSecondaryTemperature )
		{
			ASSERTA_RETURN( 0.0 );
		}

		double dSupplyPrimaryRho = pclPrimarySupplyWaterChar->GetDens();
		double dSupplySecondaryRho = pclSecondarySupplyWaterChar->GetDens();
		double dReturnSecondaryRho = pclSecondaryReturnWaterChar->GetDens();

		double dSupplyPrimaryCp = pclPrimarySupplyWaterChar->GetSpecifHeat();
		double dSupplySecondaryCp = pclSecondarySupplyWaterChar->GetSpecifHeat();
		double dReturnSecondaryCp = pclSecondaryReturnWaterChar->GetSpecifHeat();

		dPrimaryFlow = dSupplySecondaryRho * dSupplySecondaryCp * dSupplySecondaryTemperature;
		dPrimaryFlow -= ( dReturnSecondaryRho * dReturnSecondaryCp * dReturnSecondaryTemperature );
	
		double dDenominator = dSupplyPrimaryRho * dSupplyPrimaryCp * dSupplyPrimaryTemperature;
		dDenominator -= ( dReturnSecondaryRho * dReturnSecondaryCp * dReturnSecondaryTemperature );

		dPrimaryFlow /= dDenominator;
		dPrimaryFlow *= dSecondaryFlow;
	}

	return dPrimaryFlow;
}

double CDS_HmInj::ComputePrimaryFlow()
{
	return ComputePrimaryFlow( GetpTermUnit()->GetQ() );
}

bool CDS_HmInj::CheckIfTempAreValid( int *piErrors, std::vector<CString> *pvecStrErrors )
{
	ProjectType eProjectApplicationType = TASApp.GetpTADS()->GetpTechParams()->GetProjectApplicationType();
	double dTss = GetDesignCircuitSupplySecondaryTemperature();
	double dTrs = GetDesignCircuitReturnSecondaryTemperature();
	double dTsp = GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );

	CString strTss = WriteCUDouble( _U_TEMPERATURE, dTss, true );
	CString strTrs = WriteCUDouble( _U_TEMPERATURE, dTrs, true );
	CString strTsp = WriteCUDouble( _U_TEMPERATURE, dTsp, true );
	int iErrors = 0;
	
	if( dTss > -273.15 && dTrs > -273.15 )
	{
		CString str = _T("");

		// HYS-1833: Temperatures can't be the same.
		if( dTss == dTrs )
		{
			iErrors |= ITE_TemperatureEquals;

			if( NULL != pvecStrErrors )
			{
				// "Secondary supply temperature 'tss' (%1) can not be equal to the secondary return temperature 'trs' (%2)."
				FormatString( str, IDS_INJCIRC_TEMPERROR_CANTBEEQUAL, strTss, strTrs );
				pvecStrErrors->push_back( str );
			}
		}

		if( eProjectApplicationType == Heating )
		{
			if( dTss > dTsp )
			{
				iErrors |= ITE_Heating_TssGreaterThanTsp;

				if( NULL != pvecStrErrors )
				{
					// Heating system: supply secondary temperature 'tss' (%1) can not be greater than the primary supply temperature 'tsp' (%2).
					FormatString( str, IDS_INJCIRC_TEMPERROR_HEATING_TSSCANTBEGREATERTHANTSP, strTss, strTsp );
					pvecStrErrors->push_back( str );
				}
			}
			
			if( dTss < dTrs )
			{
				iErrors |= ITE_Heating_TssLowerThanTrs;

				if( NULL != pvecStrErrors )
				{
					// Heating system: supply secondary temperature 'tss' (%1) can not be lower than the return secondary temperature 'trs' (%2).
					FormatString( str, IDS_INJCIRC_TEMPERROR_HEATING_TSSCANTBELOWERTHANTRS, strTss, strTrs );
					pvecStrErrors->push_back( str );
				}
			}

			if( dTrs > dTsp )
			{
				iErrors |= ITE_Heating_TrsGreaterThanTsp;

				if( NULL != pvecStrErrors )
				{
					// Heating system: return secondary temperature 'trs' (%1) can not be greater than the supply primary temperature 'tsp' (%2).
					FormatString( str, IDS_INJCIRC_TEMPERROR_HEATING_TRSCANTBEGREATERTHANTSP, strTrs, strTsp );
					pvecStrErrors->push_back( str );
				}
			}
		}
		else
		{
			if( dTss < dTsp )
			{
				iErrors |= ITE_Cooling_TssLowerThanTsp;

				if( NULL != pvecStrErrors )
				{
					// Cooling system: supply secondary temperature 'tss' (%1) can not be lower than the supply primary temperature 'tsp' (%2).
					FormatString( str, IDS_INJCIRC_TEMPERROR_COOLING_TSSCANTBELOWERTHANTSP, strTss, strTsp );
					pvecStrErrors->push_back( str );
				}
			}

			if( dTss > dTrs )
			{
				iErrors |= ITE_Cooling_TssGreaterThanTrs;

				if( NULL != pvecStrErrors )
				{
					// Cooling system: supply secondary temperature 'tss' (%1) can not be greater than the return secondary temperature 'trs' (%2).
					FormatString( str, IDS_INJCIRC_TEMPERROR_COOLING_TSSCANTBEGREATERTHANTRS, strTss, strTrs );
					pvecStrErrors->push_back( str );
				}
			}

			if( dTrs < dTsp )
			{
				iErrors |= ITE_Cooling_TrsLowerThanTsp;

				if( NULL != pvecStrErrors )
				{
					// Cooling system: return secondary temperature 'trs' (%1) can not be lower than the supply primary temperature 'tsp' (%2).
					FormatString( str, IDS_INJCIRC_TEMPERROR_COOLING_TRSCANTBELOWERTHANTSP, strTrs, strTsp );
					pvecStrErrors->push_back( str );
				}
			}
		}

		// HYS-1748: Consider freezing temperature.
		double dFreezingPoint = -273.15;
		CWaterChar *pclWaterChar = TASApp.GetpTADS()->GetpWCForProject()->GetpWCData();

		if( NULL != pclWaterChar )
		{
			dFreezingPoint = pclWaterChar->GetTfreez();
		}
		
		if( dFreezingPoint != -273.15 && ( ITE_Heating_TssLowerThanTrs != ( ITE_Heating_TssLowerThanTrs & iErrors ) )
				&& ( ITE_Cooling_TssLowerThanTsp != ( ITE_Cooling_TssLowerThanTsp & iErrors ) )
				&& ( ITE_Cooling_TrsLowerThanTsp != ( ITE_Cooling_TrsLowerThanTsp & iErrors ) ) )
		{
			CString strFreezingPoint = WriteCUDouble( _U_TEMPERATURE, dFreezingPoint, true );

			if( dTss <= dFreezingPoint )
			{
				iErrors |= ITE_SupplyTemp_FreezingPoint;

				if( NULL != pvecStrErrors )
				{
					// Supply secondary temperature 'tss' (%1) must be greater than the freezing point temperature (%2).
					FormatString( str, IDS_INJCIRC_TEMPERROR_TSSBELOWOREQUALTOFREEZINGPOINT, strTss, strFreezingPoint );
					pvecStrErrors->push_back( str );
				}
			}

			if( dTrs <= dFreezingPoint )
			{
				iErrors |= ITE_ReturnTemp_FreezingPoint;

				if( NULL != pvecStrErrors )
				{
					// Return secondary temperature 'trs' (%1) must be greater than the freezing point temperature (%2).
					FormatString( str, IDS_INJCIRC_TEMPERROR_TRSBELOWOREQUALTOFREEZINGPOINT, strTrs, strFreezingPoint );
					pvecStrErrors->push_back( str );
				}
			}
		}

		if( NULL != piErrors )
		{
			*piErrors = iErrors;
		}

		return ( ITE_OK == iErrors ) ? true : false;
	}

	return false;
}

double CDS_HmInj::AdjustPumpHmin()
{
	if( NULL == GetpPump() )
	{
		return 0.0;
	}

	double dPumpHmin = 0.0;
	double dTUDp = GetpTermUnit()->GetDp();
	double dBvsecDpmin = 0.0;

	if( NULL != GetpSecBv() )
	{
		dBvsecDpmin += GetpSecBv()->GetDpMin();
	}

	double dSecPipesDp = 0.0;
	
	if( NULL != GetpCircuitSecondaryPipe() )
	{
		dSecPipesDp = GetpCircuitSecondaryPipe()->GetPipeDp();
		dSecPipesDp += GetpCircuitSecondaryPipe()->GetSingularityTotalDp( true );
	}

	if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() && NULL != GetpBypBv() && GetpBypBv()->GetDp() > 0.0 )
	{
		double dDefaultSecondaryPumpHMin = m_pTechParam->GetDefaultSecondaryPumpHMin();
		double dHTrequired = dTUDp + dSecPipesDp + dBvsecDpmin;
		double dHbypass = GetpBypBv()->GetDp();
		double dExceedOfPressure = 0.0;

		if( dHTrequired >= dHbypass * 0.99 && dHTrequired <= dHbypass * 1.01 )
		{
			// Case 1.
			dPumpHmin = 0;
		}
		else if( dHTrequired > dHbypass )
		{
			// Case 2.
			if( dHTrequired - dHbypass < dDefaultSecondaryPumpHMin )
			{
				// Case 2.a.
				dPumpHmin = dDefaultSecondaryPumpHMin;
				dExceedOfPressure = dDefaultSecondaryPumpHMin - ( dHTrequired - dHbypass );
			}
			else
			{
				// Case 2.b.
				dPumpHmin = ( dHTrequired - dHbypass );
			}
		}
		else if( dHTrequired < dHbypass )
		{
			// Case 3.
			dPumpHmin = 0.0;
			dExceedOfPressure = ( dHbypass - dHTrequired );
		}

		if( dExceedOfPressure > 0.0 )
		{
			double dBvDpToSet = dBvsecDpmin + dExceedOfPressure;
			GetpSecBv()->ResizeBv( dBvDpToSet );

			dBvsecDpmin = GetpSecBv()->GetDpMin();

			if( dBvsecDpmin > dBvDpToSet )
			{
				// When resizing the BV at the secondary side, we can get the case where the size is changed and then
				// that the Dp min is bigger that the 'dBvDpToSet' value.
				dPumpHmin = GetpPump()->GetHmin() + ( dBvsecDpmin - dBvDpToSet );
			}
		}
	}
	else
	{
		dPumpHmin = dTUDp + dBvsecDpmin + dSecPipesDp;
	}

	GetpPump()->SetHmin( dPumpHmin );

	return dPumpHmin;
}

#define CDS_HMINJ_VERSION	1
void CDS_HmInj::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_HMINJ_VERSION;
	WriteData<>( outf, Version );

	// Base class.
	CDS_HydroMod::Write( outf );

	// Info.
	WriteData<>( outf, m_dDesignCircuitSupplySecondaryTemperature );
	WriteData<>( outf, m_dDesignCircuitReturnSecondaryTemperature );
}

bool CDS_HmInj::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_HMINJ_VERSION;
	ReadData<>( inpf, Version );

	if( Version < 1 || Version > CDS_HMINJ_VERSION )
	{
		return false;
	}

	// Base class.
	if( false == CDS_HydroMod::Read( inpf ) )
	{
		return false;
	}

	if( NULL == m_pCircuitSecondaryPipe )
	{
		ASSERTA_RETURN( false );
	}

	// Info.
	m_dDesignCircuitSupplySecondaryTemperature = -273.15;
	ReadData<>( inpf, m_dDesignCircuitSupplySecondaryTemperature );
	m_pCircuitSecondaryPipe->SetTemperature( m_dDesignCircuitSupplySecondaryTemperature, CAnchorPt::PipeLocation_Supply );

	m_dDesignCircuitReturnSecondaryTemperature = -273.15;
	ReadData<>( inpf, m_dDesignCircuitReturnSecondaryTemperature );
	m_pCircuitSecondaryPipe->SetTemperature( m_dDesignCircuitReturnSecondaryTemperature, CAnchorPt::PipeLocation_Return );

	// Update terminal unit temperature.
	// Do not call the "UpdateTUTemperaturesForInjectionCircuit" method because this one will call the "CTermUnit::SetPdT" method that will
	// recompute the primary flow. We can't do that here because we don't have yet temperatures information on the circuit pipe at the primary side.
	if( m_dDesignCircuitSupplySecondaryTemperature > -273.15 && m_dDesignCircuitReturnSecondaryTemperature > -273.15 )
	{
		// Update DT when terminal unit is defined with P/DT.
		if( NULL != GetpTermUnit() && CTermUnit::_QType::PdT == GetpTermUnit()->GetQType() )
		{
			GetpTermUnit()->SetPdTOnlyValues( DBL_MAX, abs( m_dDesignCircuitSupplySecondaryTemperature - m_dDesignCircuitReturnSecondaryTemperature ) );
		}
	}

	return true;
}

void CDS_HmInj::Copy( CDS_HydroMod *pHM2W, bool bValveDataOnly, bool bCreateLinkedDevice, bool bToClipboard, bool bResetDiversityFactor )
{
	pHM2W->EnableComputeHM( false );
	pHM2W->SetLock( m_usLock );

	pHM2W->SetSchemeID( GetSchemeID() );
	CDS_HydroMod::Copy( pHM2W, bValveDataOnly, bCreateLinkedDevice, bToClipboard, bResetDiversityFactor );
	(static_cast<CDS_HmInj *>( pHM2W ))->SetDesignCircuitSupplySecondaryTemperature( m_dDesignCircuitSupplySecondaryTemperature );
	(static_cast<CDS_HmInj *>( pHM2W ))->SetDesignCircuitReturnSecondaryTemperature( m_dDesignCircuitReturnSecondaryTemperature );		
	pHM2W->SetLock( m_usLock );
	pHM2W->EnableComputeHM( true );
}

void CDS_HmInj::Copy( CData *pTo )
{
	( (CDS_HydroMod *)pTo )->EnableComputeHM( false );
	( (CDS_HydroMod *)pTo )->SetLock( m_usLock );
	( (CDS_HydroMod *)pTo )->SetSchemeIDPtr( GetSchemeIDPtr() );
	
	CDS_HmInj::Copy( (CDS_HmInj *)pTo, false, true, false, false );
	( (CDS_HydroMod *)pTo )->SetLock( m_usLock );
	( (CDS_HydroMod *)pTo )->EnableComputeHM( true );
}

void CDS_HmInj::Init( IDPTR SchIDPtr, CTable *pclParent, bool bUseTACV, bool bCreateLinkedDevice )
{
	CDS_HydroMod::Init( SchIDPtr, pclParent, true );

	CDB_CircuitScheme *pSch = static_cast<CDB_CircuitScheme *>( SchIDPtr.MP );

	if( true == pSch->IsAnchorPtExist( CAnchorPt::eFunc::BV_S ) )
	{
		CreateBv( CDS_HydroMod::eHMObj::eBVsec );
	}

	m_ComputeHMInjState = eceNone;
	m_bComputeHMInjAllowed = true;
}

bool CDS_HmInj::IsCompletelyDefined( int iHMObjToCheck )
{
	if( true == CDS_HydroMod::IsCompletelyDefined( iHMObjToCheck ) )
	{
		return CheckIfTempAreValid();
	}

	return false;
}

double CDS_HmInj::GetTotQwoDiversity()
{
	double dTotQwoDiversity = GetpTermUnit()->GetTotQwoDiversity( );
	double dPrimQTotwoDiversity = ComputePrimaryFlow( dTotQwoDiversity );
	return dPrimQTotwoDiversity;
}

double CDS_HmInj::GetPartialPendingQ()
{
	double dPartialPendingQ = GetpTermUnit()->GetPartialPendingQ();
	double dPartialPendingPrimQ = ComputePrimaryFlow( dPartialPendingQ );
	return dPartialPendingPrimQ;
}

void CDS_HmInj::FillOtherInfoIntoHMX( CDS_HydroModX *pHMX )
{
	if( NULL != pHMX )
	{
		pHMX->SetTempSecondary( GetDesignCircuitSupplySecondaryTemperature() );
		pHMX->SetTempReturn( GetDesignCircuitReturnSecondaryTemperature() );
	}

	CDS_HydroMod::FillOtherInfoIntoHMX( pHMX );
}

void CDS_HmInj::TransferOtherInfoFromHMX( CDS_HydroModX *pHMX )
{
	if( NULL == pHMX )
	{
		return;
	}

	SetDesignCircuitSupplySecondaryTemperature( pHMX->GetTempSecondary() );
	SetDesignCircuitReturnSecondaryTemperature( pHMX->GetTempReturn() );

	CDS_HydroMod::TransferOtherInfoFromHMX( pHMX );
}

//////////////////////////////////////////////////////////////////////
//			CDS_Hm2WInj
//////////////////////////////////////////////////////////////////////

CDS_Hm2WInj::CDS_Hm2WInj( CDataBase *pDataBase, LPCTSTR ID )
	: CDS_HmInj( pDataBase, ID )
{
}

void CDS_Hm2WInj::Init( IDPTR SchIDPtr, CTable *plcParent, bool bUseTACV, bool bCreateLinkedDevice )
{
	CDS_HmInj::Init( SchIDPtr, plcParent, bUseTACV );

	CDB_CircuitScheme *pSch = static_cast<CDB_CircuitScheme *>( SchIDPtr.MP );

	if( NULL != pSch )
	{
		if( CvLocation::CvLocNone != pSch->GetCvLoc() )
		{
			CreateCv( pSch->GetCvLoc() );
			ASSERT( NULL != GetpCV() );

			GetpCV()->SetTACv( bUseTACV );
		}
		else if( true == pSch->IsAnchorPtExist( CAnchorPt::SmartControlValve ) )
		{
			CreateSmartControlValve( pSch->GetSmartControlValveLoc() );
			ASSERT( NULL != GetpSmartControlValve() );
		}
	}
}

void CDS_Hm2WInj::SetHMPrimAndSecFlow( double dQ )
{
	if( NULL != GetpSecBv() )
	{
		GetpSecBv()->SetQ( dQ );
	}

	// Size Secondary pipe.
	if( NULL != GetpCircuitSecondaryPipe() )
	{
		GetpCircuitSecondaryPipe()->SetTotalQ( dQ );
		GetpCircuitSecondaryPipe()->SetRealQ( dQ );
	}

	double dPrimQ = ComputePrimaryFlow();
	CDS_HydroMod::SetHMPrimAndSecFlow( max( 0, dPrimQ ) );
}

// Compute a new setting point for balancing device in function of Havail.
void CDS_Hm2WInj::ResizeBalDevForH( double dHAvail, bool bForPending, CAnchorPt::CircuitSide eCircuitSide, bool *pbHasHAvailSecChanged )
{
	if( NULL != pbHasHAvailSecChanged )
	{
		*pbHasHAvailSecChanged = false;
	}

	CDS_HmInj::ResizeBalDevForH( dHAvail, bForPending, eCircuitSide, pbHasHAvailSecChanged );

	// dHAvail == 0 means that we are in descending phase.
	// When 2-way injection circuit is a module we have to open secondary Bv at full opening in ascending phase.
	if( CAnchorPt::CircuitSide::CircuitSide_Secondary == eCircuitSide )
	{
		if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() )
		{
			// HYS-1930: auto-adapting variable flow decoupling circuit.

			double dHAvailPrimarySide = GetHAvail( CAnchorPt::CircuitSide::CircuitSide_Primary );

			// If H available is different from 0, we adapt primary and secondary side for several scenarii.
			if( dHAvailPrimarySide > 0.0 && dHAvail > 0.0 )
			{
				double dDpCDpmin = GetpDpC()->GetDpmin();
				double dDpCDplmax = GetpDpC()->GetDplmax();
				double dHbypass = fabs( dHAvailPrimarySide - dDpCDpmin );

				if( NULL != GetpCircuitPrimaryPipe() )
				{
					dHbypass -= GetpCircuitPrimaryPipe()->GetPipeDp();
					dHbypass -= GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true );
				}

				// HYS-1930: auto-adapting variable flow decoupling circuit.
				// Now that the Dp controller is fixed and we know the H available in the bypass, we can decide
				// how to help the pump at the secondary side.
				// We have 3 cases:
				// Remark: HT est le H requis par le tertaire. On y inclus la BV secondaire.
				//	1) HT > Hbypass: we can have two cases:
				//		a) HT - Hbypass < Pump Hmin min (Tech param = 25 kPa) -> Pump Hmin = 25 kPa; BVsec += 25 - (HT - Hbypass)
				//		b) HT - Hbypass >= Pump Hmin min (25 kPa)             -> Pump Hmin = HT - Hbypass
				//  2) HT <= Hbypass: We can delete the pump at the secondary side -> BVsec += (Hbypass - HT)

				double dPumpHmin = GetpPump()->GetHmin();
				bool bPumpHUserDefined = ( GetpPump()->GetHpump() > 0.0 ) ? true : false;
				double dPumpHUser = GetpPump()->GetHpump();
				double dPumpHAvail = GetpPump()->GetHAvail();

				double dOldBvsDpMin = 0.0;

				if( NULL != GetpSecBv() )
				{
					// Will return either 3 kPa or the minimal pressure drop at full opening.
					dOldBvsDpMin = GetpSecBv()->GetDpMin();
				}

				double dTUDp = GetpTermUnit()->GetDp();
				double dSecPipesDp = 0.0;
					
				if( NULL != GetpCircuitSecondaryPipe() )
				{
					dSecPipesDp += GetpCircuitSecondaryPipe()->GetPipeDp();
					dSecPipesDp += GetpCircuitSecondaryPipe()->GetSingularityTotalDp( true );
				}

				double dHTrequired = dTUDp + dSecPipesDp + dOldBvsDpMin;

				// Check the cases.
				double dDefaultSecondaryPumpHMin = m_pTechParam->GetDefaultSecondaryPumpHMin();
				double dExceedOfPressure = 0.0;
				double dNewPumpHmin = 0.0;

				if( dHTrequired > dHbypass )
				{
					// Case 1.

					if( dHTrequired - dHbypass < dDefaultSecondaryPumpHMin )
					{
						// Case 1.a.
						dNewPumpHmin = dDefaultSecondaryPumpHMin;
						dExceedOfPressure = dDefaultSecondaryPumpHMin - ( dHTrequired - dHbypass );
					}
					else
					{
						// Case 1.b.
						dNewPumpHmin = ( dHTrequired - dHbypass );
					}
				}
				else if( dHTrequired <= dHbypass )
				{
					// Case 2.
					dNewPumpHmin = 0.0;
					dExceedOfPressure = ( dHbypass - dHTrequired );
				}

				if( dNewPumpHmin != dPumpHmin )
				{
					GetpPump()->SetHmin( dNewPumpHmin );

					if( NULL != pbHasHAvailSecChanged )
					{
						*pbHasHAvailSecChanged = true;
					}
				}

				if( NULL != GetpSecBv() && dExceedOfPressure > 0.0)
				{
					double dBvDpToSet = dOldBvsDpMin + dExceedOfPressure;
					GetpSecBv()->ResizeBv( dBvDpToSet );

					double dNewBvsDpMin = GetpSecBv()->GetDpMin();

					if( dNewBvsDpMin > dBvDpToSet )
					{
						// When resizing the BV at the secondary side, we can get the case where the size is changed and then
						// that the Dp min is bigger that the 'dBvDpToSet' value.
						double dNewPumpHMin = GetpPump()->GetHmin() + ( dNewBvsDpMin - dBvDpToSet );
						GetpPump()->SetHmin( dNewPumpHMin );

						if( NULL != pbHasHAvailSecChanged )
						{
							*pbHasHAvailSecChanged = true;
						}
					}
				}
			}
			else if( dHAvail <= 0.0 )
			{
				// We want to reset the secondary.

				if( NULL != GetpSecBv() )
				{
					GetpSecBv()->ResizeBv( 0.0 );

					// Adjust pump.
					// For our case, we will threat in this method the case for the auto-adpative flow to take into account
					// the differential pressure stabilised in the bypass.
					CDS_HmInj::AdjustPumpHmin();
				}
			}
		}
		else
		{
			// Secondary side is depending of Pump and TUDp!
			// When Resizing is done on Havail = 0, relax SecBv we are on the ascending phase
			if( NULL != GetpSecBv() )
			{
				if( 0.0 == dHAvail )
				{
					GetpSecBv()->ResizeBv( 0.0 );

					// Adjust pump for new valve Dp.
					CDS_HmInj::AdjustPumpHmin();
				}
				else
				{
					// Bv should be adjusted to take the excess of Diff. Pressure
					double dDp = GetpTermUnit()->GetDp();

					if( NULL != GetpCircuitSecondaryPipe() )
					{
						dDp += GetpCircuitSecondaryPipe()->GetPipeDp();
						dDp += GetpCircuitSecondaryPipe()->GetSingularityTotalDp( true );
					}

					double dPumpHAvail = GetpPump()->GetHAvail();
					double dExcessOfDiffPressure = max( 0.0, dPumpHAvail - dDp );

					GetpSecBv()->ResizeBv( dExcessOfDiffPressure );
				}
			}
		}
	}

	if( CAnchorPt::CircuitSide::CircuitSide_Primary == eCircuitSide )
	{
		// Primary side !
		if( dHAvail > 0.0 && NULL == GetpDpC() )
		{
			if( NULL != GetpCV() )
			{
				if( false == GetpCV()->IsPresettable() )
				{
					dHAvail -= GetpCV()->GetDp();
				}
			}
		}

		/*
		if( eDpStab::DpStabOnBVBypass == GetpSch()->GetDpStab() && false == IsaModule() )
		{
			// HYS-1930: auto-adapting variable flow decoupling circuit.

			if( NULL != GetpSecBv() )
			{
				GetpSecBv()->ResizeBv( 0.0 );
			}
			
			AdjustPumpHmin();
		}
		*/

		// BV on primary side will be resized by following function
		CDS_HydroMod::ResizeBalDevForH( dHAvail, bForPending, eCircuitSide, pbHasHAvailSecChanged );
	}
}


void CDS_Hm2WInj::ComputeHM( eComputeHMEvent ComputeHMEvent, LPARAM pObj )
{
	if( false == IsHMCalcMode() )
	{
		return;
	}

	// During HydroMod copy Compute HydroMod is deactivated.
	//
	// Why second argument? In 'CDS_Hm3W' we call 'CDS_HydroMod::ComputeHM' just after having set the value for the 3W computing engine state.
	// In 'CDS_HydroMod::ComputeHM', some steps recall 'ComputeHM' (SelectBestPipe for example) and inherited class is called again.
	// Because the 3way state engine value is already set, we directly exit the method.
	//
	// In 2WInj it's a little bit different. We call 'CDS_HydroMod::ComputeHM' once we have finished all needed computing for 2Way
	// injection. That means the engine state value is reset to 'eceLAST'. When in 'CDS_HydroMod::ComputeHM' some parts recall 'ComputeHM'
	// and the inherited class is called again, there is no more restriction and engine state for 2Winj is restarted.
	if( true == m_bComputeAllowed && true == m_bComputeHMInjAllowed )
	{
		// Sanity tests.
		if( m_ComputeHMInjState > eceLAST || m_ComputeHMInjState < eceNone )
		{
			ASSERT( 0 );
			m_ComputeHMInjState = eceNone;
		}

		if( ComputePrimaryFlow() <= 0 )
		{
			m_ComputeHMInjState = eceNone;
		}

		// If the engine is already running discard new event.
		if( m_ComputeHMInjState != eceNone && m_ComputeHMInjState != eceLAST )
		{
			return;
		}

		m_bComputeHMInjAllowed = false;
		SetComputeHMInjEngine( ComputeHMEvent, pObj );

		bool fAllShouldBeSized = false;

		while( m_ComputeHMInjState != eceNone && m_ComputeHMInjState != eceLAST )
		{
			switch( m_ComputeHMInjState )
			{
				case eceTUDp:
				case eceResize:
				case eceTUQ:
				{
					// Update flow in devices.
					double dQTU = GetpTermUnit()->GetQ();
					SetHMPrimAndSecFlow( dQTU );

					// Size Secondary pipe.
					if( NULL != GetpCircuitSecondaryPipe() )
					{
						VerifyModuleCircuitPipe( GetpCircuitSecondaryPipe() );

						IDPTR idptr = GetpCircuitSecondaryPipe()->SelectBestPipe( dQTU );

						if( false == GetpCircuitSecondaryPipe()->IsLocked() )
						{
							GetpCircuitSecondaryPipe()->SetIDPtr( idptr, false );
						}
					}

					// Size H requested on Pump.
					if( NULL != GetpPump() )
					{
						// If secondary balancing valve exist, we size it at the minimum.
						if( NULL != GetpSecBv() )
						{
							GetpSecBv()->ResizeBv( 0 );
						}

						/*
						if( eDpStab::DpStabOnBVBypass != GetpSch()->GetDpStab() )
						{
							// The minimum H required by the pump is in fact the sum of the pressure drop in the terminal unit, 
							// in the secondary balancing valve if exists and in the secondary pipes.
							bool bIsUserPumpHDefined = ( GetpPump()->GetHpump() > 0.0 ) ? true : false;
							double dPumpHmin = CDS_HmInj::AdjustPumpHmin();

							if( GetpPump()->GetHpump() >= dPumpHmin )
							{
								// If H available at the pump is higher than the required, it's the secondary balancing valve that
								// will take this excess.
								double dDpExcess = GetpPump()->GetHpump() - GetpPump()->GetHmin();

								if( NULL != GetpSecBv() )
								{
									m_bComputeAllowed = false;

									// Remark: Because pressure drop accross the secondary balancing valve is already taken into account
									// for the pump, excess does not include the Dpmin of the valve.
									GetpSecBv()->ResizeBv( dDpExcess + GetpSecBv()->GetDpMin() );
								
									m_bComputeAllowed = true;
								}
							}
							else if( true == bIsUserPumpHDefined )
							{
								// Hmin becomes higher that the value input by the user, we reset Hpump to Hmin.
								GetpPump()->SetHpump( dPumpHmin );
							}
						}
						*/
					}

					SetComputeHMInjEngine( eceLAST );
				}
				break;

				default:
					SetComputeHMInjEngine( eceLAST );
			}
		}
	}

	// Compute base.
	CDS_HydroMod::ComputeHM( ComputeHMEvent, pObj );

	m_bComputeHMInjAllowed = true;
}

void CDS_Hm2WInj::Copy( CDS_HydroMod *pHM2W, bool fValveDataOnly, bool fCreateLinkedDevice, bool fToClipboard, bool fResetDiversityFactor )
{
	CDS_HmInj::Copy( pHM2W, fValveDataOnly, fCreateLinkedDevice, fToClipboard, fResetDiversityFactor );
}

void CDS_Hm2WInj::Copy(CData *pTo)
{
	((CDS_HydroMod *)pTo)->SetSchemeIDPtr(GetSchemeIDPtr());
	CDS_Hm2WInj::Copy((CDS_Hm2WInj *)pTo, false, true, false, false);
}

#define CDS_HM2WINJ_VERSION	2
void CDS_Hm2WInj::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_HM2WINJ_VERSION;
	WriteData<>( outf, Version );

	// Base class.
	CDS_HmInj::Write( outf );
}

bool CDS_Hm2WInj::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_HM2WINJ_VERSION;
	ReadData<>( inpf, Version );

	if( Version < 1 || Version > CDS_HM2WINJ_VERSION )
	{
		return false;
	}

	if( 1 == Version )
	{
		// Base class.
		if( false == CDS_HydroMod::Read( inpf ) )
		{
			return false;
		}

		// This is a old version when temperatures for secondary side was read in this class.
		// Now the temperatures are read in the 'CDS_HmInj' base class.
		ReadData<>( inpf, m_dDesignCircuitSupplySecondaryTemperature );
		ReadData<>( inpf, m_dDesignCircuitReturnSecondaryTemperature );

		// Update terminal unit temperature.
		UpdateTUTemperaturesForInjectionCircuit();

		return true;
	}

	if( false == CDS_HmInj::Read( inpf ) )
	{
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
//			CDS_Hm3WInj
//////////////////////////////////////////////////////////////////////

CDS_Hm3WInj::CDS_Hm3WInj( CDataBase *pDataBase, LPCTSTR ID )
	: CDS_HmInj( pDataBase, ID )
{
}


void CDS_Hm3WInj::Init( IDPTR SchIDPtr, CTable *plcParent, bool bUseTACV, bool fCreateLinkedDevice )
{
	CDS_HmInj::Init( SchIDPtr, plcParent, bUseTACV );

	CDB_CircuitScheme *pSch = static_cast<CDB_CircuitScheme *>( SchIDPtr.MP );

	if( NULL != pSch )
	{
		CreateCv( pSch->GetCvLoc() );
		ASSERT( NULL != GetpCV() );

		GetpCV()->SetTACv( bUseTACV );

		// 3W injection circuit
		GetpCV()->SetCVLocAuto( true );

		if( NULL != pSch->GetpSchCateg() && CDB_ControlProperties::CV2W3W::CV3W == pSch->GetpSchCateg()->Get2W3W()
				&& CDB_CircSchemeCateg::e3WType::e3wTypeMixing == pSch->GetpSchCateg()->Get3WType()
				&& eDpStab::DpStabOnBranch == pSch->GetDpStab() && NULL != GetpDpC() )
		{
			// Force the balancing valve to the secondary side if we are with a 3-way mixing circuit with a Dp controller that stabilizes the branch.
			GetpDpC()->SetMvLoc( eMvLoc::MvLocSecondary );
			GetpDpC()->SetMvLocLocked( true );
		}
	}
}

bool CDS_Hm3WInj::_FindAndSetMatchingScheme( CvLocation CvLoc )
{
	// Extract list of compliant circuits
	CTable *pTab = dynamic_cast<CTable *>( TASApp.GetpTADB()->Get( L"CIRCSCHEME_TAB" ).MP );

	if( NULL == pTab || NULL == GetpSch() )
	{
		ASSERT( 0 );
		return false;
	}

	eMvLoc MvLoc = GetpSch()->GetMvLoc();

	if( CvLoc == CvLocSecSide )
	{
		// 3wMix with decoupling bypass .... When CV located in secondary... Mv should be located in DpStab
		MvLoc = MvLocSecondary;
	}

	CArray<CDB_CircuitScheme *> arpSch;

	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		CDB_CircuitScheme *pSch = dynamic_cast<CDB_CircuitScheme *>( idptr.MP );

		if( NULL == pSch )
		{
			continue;
		}

		if( pSch->GetpSchCateg()->Get3WType() == CDB_CircSchemeCateg::e3wTypeMixDecByp )
		{
			if( pSch->GetBalType() == GetpSch()->GetBalType() &&
				pSch->GetDpCType() == GetpSch()->GetDpCType() &&
				pSch->GetDpStab() == GetpSch()->GetDpStab() &&
				pSch->GetTermUnit() == GetpSch()->GetTermUnit() &&
				pSch->GetCvFunc() == GetpSch()->GetCvFunc() &&
				pSch->GetDpCLoc() ==  GetpSch()->GetDpCLoc() )
			{
				arpSch.Add( pSch );
			}
		}
	}

	if( arpSch.GetSize() <= 0 )
	{
		ASSERT( 0 );
		return false;
	}

	CDB_CircuitScheme *pSch = NULL;
	CDB_CircuitScheme *pSchAltMvLoc = NULL;

	for( int i = 0; i < arpSch.GetSize(); i++ )
	{
		if( arpSch[i]->GetCvLoc() == CvLoc )
		{
			if( arpSch[i]->GetMvLoc() == MvLoc )
			{
				pSch = arpSch[i];
			}
			else
			{
				pSchAltMvLoc = arpSch[i];
			}
		}

		if( NULL != pSch )
		{
			// Break only when we find the perfect match.
			break;
		}
	}

	if( pSch == NULL )
	{
		pSch = pSchAltMvLoc;
	}

	if( NULL != pSch )
	{
		SetSchemeID( pSch->GetIDPtr().ID );
		return true;
	}

	return false;
}

void CDS_Hm3WInj::AdjustCircuitIDForValveLocalisation( CvLocation CvLoc )
{
	// TODO review this function to use a lockup in database;
	// Need to add CV localization into CDB_CircuitScheme
	double dQ = GetpTermUnit()->GetQ();
	double dPrimQ = ComputePrimaryFlow();

	// 3WINJ_MIX_DECBYP1_BV & 2; 3Ways injection circuit Mixing circuit with decoupling bypass
	if( NULL != GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixDecByp == GetpSchcat()->Get3WType() )
	{
		if( CvLocation::CvLocNone == CvLoc )	// Automatic
		{
			// Primary flow lower than secondary flow, better CV location is primary
			if( dPrimQ < dQ )
			{
				if( true == _FindAndSetMatchingScheme( CvLocation::CvLocPrimSide ) )
				{
					GetpCV()->SetQ( dPrimQ );
					GetpCV()->SetCVLocAuto( true );
				}
			}
			else
			{
				if( true == _FindAndSetMatchingScheme( CvLocation::CvLocSecSide ) )
				{
					GetpCV()->SetQ( dQ );
					GetpCV()->SetCVLocAuto( true );
				}
			}
		}
		else if( CvLocation::CvLocPrimSide == CvLoc )
		{
			GetpCV()->SetCVLocAuto( false );

			// Forced on Primary side
			if( true == _FindAndSetMatchingScheme( CvLocation::CvLocPrimSide ) )
			{
				GetpCV()->SetQ( dPrimQ );
			}
		}
		else
		{
			GetpCV()->SetCVLocAuto( false );

			// Forced on secondary side
			if( true == _FindAndSetMatchingScheme( CvLocation::CvLocSecSide ) )
			{
				GetpCV()->SetQ( dQ );
			}
		}
	}
}

void CDS_Hm3WInj::SetHMPrimAndSecFlow( double dQ )
{

	if( NULL != GetpSecBv() )
	{
		GetpSecBv()->SetQ( dQ );
	}

	// Size Secondary pipe.
	if( NULL != GetpCircuitSecondaryPipe() )
	{
		GetpCircuitSecondaryPipe()->SetTotalQ( dQ );
		GetpCircuitSecondaryPipe()->SetRealQ( dQ );
	}

	double dPrimQ = ComputePrimaryFlow();
	CDS_HydroMod::SetHMPrimAndSecFlow( max( 0, dPrimQ ) );
	CvLocation CvLoc = CvLocNone;

	if( false == GetpCV()->IsCVLocAuto() && NULL != GetpSch() )
	{
		CvLoc = GetpSch()->GetCvLoc();
	}

	AdjustCircuitIDForValveLocalisation( CvLoc );
}


// Compute a new setting point for balancing device in function of Havail.
void CDS_Hm3WInj::ResizeBalDevForH( double dHAvail, bool bForPending, CAnchorPt::CircuitSide eCircuitSide, bool *pbHasHAvailSecChanged )
{
	if( NULL != pbHasHAvailSecChanged )
	{
		*pbHasHAvailSecChanged = false;
	}

	CDS_HmInj::ResizeBalDevForH( dHAvail, bForPending, eCircuitSide, pbHasHAvailSecChanged );

	if( CAnchorPt::CircuitSide::CircuitSide_Secondary == eCircuitSide )
	{
		// Secondary side is depending of Pump and TUDp!
		// When Resizing is done on Havail = 0, relax SecBv we are on the ascending phase
		if( NULL != GetpSecBv() )
		{
			if( CDB_CircSchemeCateg::e3WType::e3wTypeMixing == GetpSchcat()->Get3WType() )
			{
				// Particular case for 3-way mixing. This method is called only when the "CDS_HydroMod::_DistributeHAvailable" method is called and
				// for a CIRCUIT. We need to manage here the BV at the secondary side because there is no other moment to do it. For a MODULE,
				// we have the call to the "CDS_HydroMod::_UpdateTUAccordingHavailable" method that manage the BV at the secondary side.
				if( -1.0 == dHAvail )
				{
					// Bv should be adjusted to take the excess of differential pressure.
					double dDpToTake = 0.0;

					// Remark: Hpump and Hmin already contain respectively the Dp BVsec and the Dpmin BVsec.
					// HPump - HminPump = Dp BVsec - Dpmin BVsec = Excess of differential pressure.
					// Dp BVsec = HPump - HminPump + Dpmin BVsec.
					dDpToTake = max( 0, GetpPump()->GetHAvail() - GetpPump()->GetHmin() + GetpSecBv()->GetDpMin() );

					// Before resizing the balancing valve, we must take its Dpmin.
					double dOldBvSecDpMin = GetpSecBv()->GetDpMin();

					GetpSecBv()->ResizeBv( dDpToTake );

					// Now we have to verify if Dpmin of the balancing valve has not increased.
					double dNewBvSecDpMin = GetpSecBv()->GetDpMin();

					if( dNewBvSecDpMin != dOldBvSecDpMin )
					{
						// Hmin must change.
						GetpPump()->SetHmin( GetpPump()->GetHmin() + abs( dNewBvSecDpMin - dOldBvSecDpMin ) );

						if( NULL != pbHasHAvailSecChanged )
						{
							*pbHasHAvailSecChanged = true;
						}

						// By chance, if user has already defined a pump head, the maximum that we can reach by increasing
						// the Hmin will never be above the H available (See doc for demonstration).
					}
				}
			}
			else
			{
				if( 0.0 == dHAvail )
				{
					GetpSecBv()->ResizeBv( 0.0 );
					
					// Adjust pump for new valve Dp.
					AdjustPumpHmin();
				}
				else
				{
					// Bv should be adjusted to take the excess of differential pressure.

					// Remark: Hpump and Hmin already contain respectively the Dp BVsec and the Dpmin BVsec.
					// HPump - HminPump = Dp BVsec - Dpmin BVsec = Excess of differential pressure
					// Dp BVsec = HPump - HminPump + Dpmin BVsec.
					double dDpToTake = max( 0, GetpPump()->GetHAvail() - GetpPump()->GetHmin() + GetpSecBv()->GetDpMin() );

					// Before resizing the balancing valve, we must take its Dpmin.
					double dOldBvSecDpMin = GetpSecBv()->GetDpMin();

					GetpSecBv()->ResizeBv( dDpToTake );

					// Now we have to verify if Dpmin of the balancing valve has not increased.
					double dNewBvSecDpMin = GetpSecBv()->GetDpMin();

					if( dNewBvSecDpMin != dOldBvSecDpMin )
					{
						// Hmin must change.
						GetpPump()->SetHmin( GetpPump()->GetHmin() + abs( dNewBvSecDpMin - dOldBvSecDpMin ) );

						if( NULL != pbHasHAvailSecChanged )
						{
							*pbHasHAvailSecChanged = true;
						}

						// By chance, if user has already defined a pump head, the maximum that we can reach by increasing
						// the Hmin will never be above the H available (See doc for demonstration).
					}
				}
			}
		}
	}
	else 
	{
		// Primary side !
		if( dHAvail > 0.0 && NULL == GetpDpC() )
		{
			if( NULL != GetpCV() )
			{
				if( false == GetpCV()->IsPresettable() )
				{
					// For 3w mixing CV is a load for secondary pump.
					if( NULL != GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixing != GetpSchcat()->Get3WType() )
					{
						if( NULL != GetpSch() && CvLocPrimSide == GetpSch()->GetCvLoc() )
						{
							dHAvail -= GetpCV()->GetDp();
						}
					}
				}
			}
		}

		// BV on primary side will be resized by the following method.
		CDS_HydroMod::ResizeBalDevForH( dHAvail, bForPending, eCircuitSide, pbHasHAvailSecChanged );
	}
}

double CDS_Hm3WInj::AdjustPumpHmin()
{
	if( NULL == GetpPump() )
	{
		return 0.0;
	}

	// Adjust first as it was a simple injection circuit.
	double dp = CDS_HmInj::AdjustPumpHmin();

	// Now we adjust for these two particular cases: 3-way mixing with decoupling bypass and 3-way mixing.

	if( NULL != GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixDecByp == GetpSchcat()->Get3WType() )
	{
		// CV is localized on secondary.
		if( eb3False == GetpCV()->IsCVLocInPrimary() )
		{
			if( true == IsConstantFlow() )
			{
				// if we look secondary side from the load, we have a 3w dividing circuit with bypass as load for CV.
				double dBypDp = 0.0;
				GetpCV()->Size3WCvCstFlow( dBypDp, false );
			}

			dp += GetpCV()->GetDp();
		}
	}
	else if( NULL != GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixing == GetpSchcat()->Get3WType() )
	{
		// Case of the 3-way mixing.
			
		// In this particular circuit, the pump takes the Dp through the control valve.
		dp += GetpCV()->GetDp();
	}

	GetpPump()->SetHmin( dp );

	return dp;
}

bool CDS_Hm3WInj::IsInjectionCircuit()
{
	bool bInjectionCircuit = true;

	// See comment in the 'HYdroMod.h'.
	if( NULL != GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixing == GetpSchcat()->Get3WType() )
	{
		bInjectionCircuit = false;
	}

	return bInjectionCircuit;
}

void CDS_Hm3WInj::ComputeHM( eComputeHMEvent ComputeHMEvent, LPARAM pObj )
{
	if( false == IsHMCalcMode() )
	{
		return;
	}

	if( false == m_bComputeAllowed || false == m_bComputeHMInjAllowed )
	{
		return;
	}

	m_bComputeHMInjAllowed = false;

	// HYS-706 issue.
	if( NULL != GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixing == GetpSchcat()->Get3WType() )
	{
		if( NULL != m_pCircuitSecondaryPipe && NULL != m_pCircuitPrimaryPipe )
		{
			// if existing move secondary pipe length to circuit pipe.
			if( 0 < m_pCircuitSecondaryPipe->GetLength() )
			{
				m_pCircuitPrimaryPipe->SetLength( m_pCircuitPrimaryPipe->GetLength() + m_pCircuitSecondaryPipe->GetLength(), false );
				m_pCircuitSecondaryPipe->SetLength( 0, false );
			}

			//Tr and Ts was corrected when reading tsp file in CDS_HMInj
		}
		else
		{
			ASSERT( 0 );
		}
	}

	double dPrimQ = ComputePrimaryFlow();

	if( dPrimQ <= 0.0 )
	{
		m_bComputeHMInjAllowed = true;
		return;
	}

	// Update flow in devices.
	double dQTU = GetpTermUnit()->GetQ();
	SetHMPrimAndSecFlow( dQTU );
	double dDpCV = 0;

	// Primary and secondary circuit pipes must be recomputed before all the valves.
	// Because for each valve, we check the size limit in regards to the current pipe size and the technical parameter "size above/below".

	// Size primary pipe.
	_ProcessCircuitPipes();

	// Size secondary pipe.
	if( NULL != GetpCircuitSecondaryPipe() )
	{
		VerifyModuleCircuitPipe( GetpCircuitSecondaryPipe() );

		IDPTR idptr = GetpCircuitSecondaryPipe()->SelectBestPipe( dQTU );

		if( false == GetpCircuitSecondaryPipe()->IsLocked() )
		{
			GetpCircuitSecondaryPipe()->SetIDPtr( idptr, false );
		}
	}

	enum
	{
		Hm3WInj_case_A,
		Hm3WInj_case_B,
		Hm3WInj_case_C,
		Hm3WInj_case_D,
		Hm3WInj_case_E,
		Hm3WInj_case_F
	};

	int iCase = -1;

	// We can have here 6 different circuit schemes:

	// A) 3-way mixing circuit with a BV in primary.
	if( CDB_CircSchemeCateg::e3WType::e3wTypeMixing == GetpSchcat()->Get3WType() && eDpStab::DpStabNone == GetpSch()->GetDpStab() )
	{
		// Pressure drop in the control valve must be at the minimum of 3kPa and must be equal to the H available at the entry
		// of the circuit (DpCv = Ha). It's the pump at the secondary side that takes the pressure drop of the control valve.
		// So, we need the same resitance at the primary side. It is the BV at the primary side that will take the same pressure drop.
		//	-> DpCv = Ha = DpBv: here we can size Cv for a minimum of 3 kPa and size BV with the Dp calculated for the CV found.
		iCase = Hm3WInj_case_A;
	}

	// B) 3-way mixing circuit with a Dp controller stabilising branch with BV linked to the secondary.
	if( -1 == iCase && CDB_CircSchemeCateg::e3WType::e3wTypeMixing == GetpSchcat()->Get3WType() && eDpStab::DpStabOnBranch == GetpSch()->GetDpStab() )
	{
		
		// It's the same principle as the A) circuit. Here, the H available is in fact the pressure drop stabilised by the Dp controller.
		// We can't size to 3kPa because we are limited by the Dplmin that the Dp controller can stabilized. So, we need first to size 
		// the Dp controller with the available flow and take the one that has the lowest Dplmin. And BV and CV can then be sized to the
		// same pressure drop.
		//	-> Size first DpC@Q -> Dplmin; With Dplmin we can size CV and BV at Dplmin kPa.
		iCase = Hm3WInj_case_B;
	}

	// C) 3-way mixing circuit with decoupling bypass and CV at the secondary side.
	if( -1 == iCase && CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == GetpSchcat()->Get3WType() && CvLocation::CvLocSecSide == GetpSch()->GetCvLoc()
			&& eDpStab::DpStabNone == GetpSch()->GetDpStab() )
	{
		// Here it's the simplest case. We can size the CV to a minimum of a 3 kPa and the same for the BV at the primary side.
		// Note: due to the schematic, pressure drop through the CV when fully opened and when fully closed are almost similar and this
		//       this is why the authority is near 1.
		// The both valves are independant and then we don't need to size BV with the pressure drop of the CV found.
		//	-> Size CV secondary and BV primary at 3 kPa minimum.
		iCase = Hm3WInj_case_C;
	}

	// D) 3-way mixing circuit with decoupling bypass and CV at the primary side.
	if( -1 == iCase && CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == GetpSchcat()->Get3WType() && CvLocation::CvLocPrimSide == GetpSch()->GetCvLoc()
			&& eDpStab::DpStabNone == GetpSch()->GetDpStab() )
	{
		// Once again, we can size the CV to the minimum of 3 kPa and because we want the minimum pressure drop for the circuit, we size 
		// also the BV at the primary side at 3 kPa minimum.
		// Note: due to the schematic, pressure drop through the CV when fully opened and when fully closed are almost similar and this
		//       this is why the authority is near 1.
		//	-> Size CV primary and BV primary at 3 kPa minimum.
		iCase = Hm3WInj_case_D;
	}

	// E) 3-way mixing circuit with decoupling bypass and CV at the secondary side and Dp controller stabilising branch with BV linked to the secondary side.
	if( -1 == iCase && CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == GetpSchcat()->Get3WType() && CvLocation::CvLocSecSide == GetpSch()->GetCvLoc()
			&& eDpStab::DpStabOnBranch == GetpSch()->GetDpStab() )
	{
		// As in the C) case, the primary and secondary side are completly independant. CV can be sized to take 3 kPa at minimum.
		// For the primary side, we are exactly in the same case of B) for the Dp controller. we need first to size the Dp controller with 
		// the available flow and take the one that has the lowest Dplmin. And BV can then be sized to the same pressure drop.
		//	-> Size forst DpC@Q -> Dplmin; With Dplmin we can size BV at Dplmin kPa.
		iCase = Hm3WInj_case_E;
	}

	// F) 3-way mixing circuit with decoupling bypass and CV at the primary side and Dp controller stabilising branch with BV linked to the secondary side.
	if( -1 == iCase && CDB_CircSchemeCateg::e3WType::e3wTypeMixDecByp == GetpSchcat()->Get3WType() && CvLocation::CvLocPrimSide == GetpSch()->GetCvLoc()
			&& eDpStab::DpStabOnBranch == GetpSch()->GetDpStab() )
	{
		// Here the stabilisation of the Dp controller includes BV at the primary side and CV that is also at the primary side.
		// Because we always try to get the lowest CV we will size here the CV to take the difference between the stabilised pressure drop
		// and the pressure drop in the BV. Then in first step we will size DpC as for the cases B) and E). After that we can size the BV
		// at 3 kPa and then CV will take the difference.
		//	-> Size forst DpC@Q -> Dplmin; Size BV at 3 kPa minimum; Size CV with a pressure drop = Dplmin - DpBV
		iCase = Hm3WInj_case_F;
	}

	if( -1 == iCase )
	{
		// New case not yet treated ???
		ASSERT_RETURN;
	}

	if( Hm3WInj_case_A == iCase )
	{
		// For this 3-way mixing circuit, we will size first the control valve. The "CDS_HydromMod::CCv::SizeControlValveForBetaMin" method will itself
		// check what is the H available at the entry of the circuit and size the control valve with the same pressure drop.
		GetpCV()->Size3WCvCstFlow( 0.0, false );

		// Now we can size the balancing valve at the primary side. Don't forget that the pressure drop on the control valve is taken by the pump at
		// the secondary side. Balancing valve must take the same value as the control valve to avoid inversion in the bypass.
		double dDpBV = GetpCV()->GetDp();

		// We must deduce the pressure drop in the circuit pipe.
		// Circuit pipe doesn't exist for a module. Or it exists if there is a secondary side.
		if( ( false == IsaModule() || ( NULL != GetpSchcat() && true == GetpSchcat()->IsSecondarySideExist() ) ) && NULL != GetpCircuitPrimaryPipe() )
		{
			dDpBV -= GetpCircuitPrimaryPipe()->GetPipeDp();
			dDpBV -= GetpCircuitPrimaryPipe()->GetSingularityTotalDp( true );
		}

		GetpBv()->ResizeBv( dDpBV );

		// Now we have to update secondary side with the pressure drop accross the control valve.
		GetpSecBv()->ResizeBv( 0.0 );

		double dHPumpMin = GetpSecBv()->GetDp();
		dHPumpMin += GetpCV()->GetDp();
		dHPumpMin += GetpTermUnit()->GetDp();

		if( false == IsaModule() && NULL != GetpCircuitSecondaryPipe() )
		{
			dHPumpMin += GetpCircuitSecondaryPipe()->GetPipeDp();
			dHPumpMin += GetpCircuitSecondaryPipe()->GetSingularityTotalDp( true );
		}

		GetpPump()->SetHmin( dHPumpMin );
	}

	if( Hm3WInj_case_B == iCase )
	{
		// For this case, we have yet to cut in several cases.
		// 1) Bv primary NOT LOCKED, CV primary NOT LOCKED.
		// 2) Bv primary LOCKED, CV primary NOT LOCKED.
		// 3) Bv primary NOT LOCKED, CV primary LOCKED.
		// 4) Bv primary LOCKED, CV primary LOCKED.

		enum
		{
			Hm3WInj_case_B_1,
			Hm3WInj_case_B_2,
			Hm3WInj_case_B_3,
			Hm3WInj_case_B_4,
		};

		int iBSubCase = -1;

		if( NULL == GetpBv() || NULL == GetpBv()->GetIDPtr().MP || false == IsLocked( eHMObj::eBVprim ) 
				&& ( NULL == GetpCV() || NULL == GetpCV()->GetCvIDPtr().MP || false == IsLocked( eHMObj::eCV ) ) )
		{
			iBSubCase = Hm3WInj_case_B_1;
		}

		if( -1 == iBSubCase && NULL != GetpBv() && NULL != GetpBv()->GetIDPtr().MP && true == IsLocked( eHMObj::eBVprim )
				&& ( NULL == GetpCV() || NULL == GetpCV()->GetCvIDPtr().MP || false == IsLocked( eHMObj::eCV ) ) )
		{
			iBSubCase = Hm3WInj_case_B_2;
		}

		if( -1 == iBSubCase && NULL != GetpCV() && NULL != GetpCV()->GetCvIDPtr().MP && true == IsLocked( eHMObj::eCV )
				&& ( NULL == GetpBv() || NULL == GetpBv()->GetIDPtr().MP || false == IsLocked( eHMObj::eBVprim ) ) )
		{
			iBSubCase = Hm3WInj_case_B_3;
		}

		if( -1 == iBSubCase && NULL != GetpBv() && NULL != GetpBv()->GetIDPtr().MP && true == IsLocked( eHMObj::eBVprim )
				&& NULL != GetpCV() && NULL != GetpCV()->GetCvIDPtr().MP && true == IsLocked( eHMObj::eCV ) )
		{
			iBSubCase = Hm3WInj_case_B_4;
		}

		if( -1 == iBSubCase )
		{
			// New sub case not yet treated ???
			ASSERT_RETURN;
		}

		if( Hm3WInj_case_B_1 == iBSubCase )
		{
			// We will first size the Dp controller with current flow and take the one with the lowest Dplmin.
			// Remark: the "CDS_HydroMod::DpC::SelectBestDpC" method calls the "CDS_HydroMod::DpC::GetDpToStab" method to know what is the 
			//         differential pressure to stabilise on the branch. Here, we are in a particular case because we want the Dp controler 
			//         with the lowest Dplmin and not a particular stabilized differential pressure. So we need that the "SelectBestDpC" method 
			//         does not call the "GetDpToStab" method but uses 0 instead. This is why we have now the second argument "bLowestDplmin" 
			//         when calling the "SelectBestDpC" method to force this method to not call the "GetDpToStab" method.

			// We invalidate selection. Otherwise the "SelectBestDpC" method will try to match a Dp controller with what was previously selected.
			GetpDpC()->InvalidateSelection();
			GetpDpC()->SelectBestDpC( 0.0, true );

			// We can now size the BV at the primary side with the same pressure drop that the one that Dp controller stabilizes.
			double dDpBV = 0.0;

			if( NULL != GetpDpC() && GetpDpC()->GetDplmin() > 0.0 )
			{
				dDpBV = GetpDpC()->GetDplmin();
			}

			GetpBv()->ResizeBv( dDpBV );

			// We can now size CV also with the same pressure drop that the one that Dp controller stabilizes.
			if( NULL != GetpDpC() && GetpDpC()->GetDplmin() > 0.0 )
			{
				double dDpStabilized = GetpDpC()->GetDplmin();
				
				// The control valve must be sized with the same pressure drop a the one stabilized by the Dp controller.
				GetpCV()->Size3WCvCstFlow( dDpStabilized, false );

				// Resize the balancing valve at the primary side to take into account same Dp as CV and HAvail.
				// It's possible that the new pressure drop across the control valve will be different than requested due to compliance with Reynard series.
				double dCVDp = GetpCV()->GetDp();
				m_dHAvail = GetpDpC()->GetDpmin() + dCVDp;

				if( NULL != GetpBv() )
				{
					GetpBv()->ResizeBv( dCVDp );
				}

				// Pressure drop accross the control valve can be different that the one desired.
				// If it's the case, we resize Dp controller with the new pressure drop to stabilize.
				// Remark: we call this method after having resized the balancing valve. Because the "ResizeDpC" method will call the "SelectBestDpC"
				//         method with the "bLowestDplmin" argument to default than "false". So, the "SelectBestDpC" method will call the "GetDpToStab" method.
				//         In this last method and for 3-way mixing, it's the pressure drop in the BV that we take for the pressure drop to stabilize.
				if( dCVDp != dDpStabilized )
				{
					GetpDpC()->ResizeDpC( m_dHAvail );
				}
			}
		}
		else if( Hm3WInj_case_B_2 == iBSubCase )
		{
			// We will first size the Dp controller with current flow and take the one that can stabilize the pressure drop accross the BV that is locked.

			// We invalidate selection. Otherwise the "SelectBestDpC" method will try to match a Dp controller with what was previously selected.
			GetpDpC()->InvalidateSelection();
			GetpDpC()->SelectBestDpC( 0.0 );

			// We can now size CV also with the same pressure drop that the one that Dp controller stabilizes.
			if( NULL != GetpDpC() && GetpDpC()->GetDplmin() > 0.0 )
			{
				double dDpBv = GetpBv()->GetDp();
				
				// The control valve must be sized with the same pressure drop a the one stabilized by the Dp controller.
				GetpCV()->Size3WCvCstFlow( dDpBv, false );

				// Resize the balancing valve at the primary side to take into account same Dp as CV and HAvail.
				// It's possible that the new pressure drop across the control valve will be different than requested due to compliance with Reynard series.
				double dDpCV = GetpCV()->GetDp();
				m_dHAvail = GetpDpC()->GetDpmin() + dDpCV;

				if( NULL != GetpBv() )
				{
					GetpBv()->ResizeBv( dDpCV );
				}

				// Pressure drop accross the control valve can be different that the one desired.
				// If it's the case, we resize Dp controller with the new pressure drop to stabilize.
				// Remark: we call this method after having resized the balancing valve. Because the "ResizeDpC" method will call the "SelectBestDpC"
				//         method with the "bLowestDplmin" argument to default than "false". So, the "SelectBestDpC" method will call the "GetDpToStab" method.
				//         In this last method and for 3-way mixing, it's the pressure drop in the BV that we take for the pressure drop to stabilize.
				if( dDpCV != dDpBv )
				{
					GetpDpC()->ResizeDpC( m_dHAvail );
				}
			}
		}
		else if( Hm3WInj_case_B_3 == iBSubCase )
		{
			// To no change the code in the "SelectBestDpC" method, we need to size BV first. Because in this method, if we don't want a Dp controller
			// with the lowest Dplmin, we call the "GetDpToStab" method that returns pressure drop accross BV in the case of a 3-way mixing circuit.
			double dDpCv = GetpCV()->GetDp();
			GetpBv()->ResizeBv( dDpCv );

			// We invalidate selection. Otherwise the "SelectBestDpC" method will try to match a Dp controller with what was previously selected.
			GetpDpC()->InvalidateSelection();

			// Now we can size the DpC that will take the pressure drop of the BV as differential pressure to stabilize.
			GetpDpC()->SelectBestDpC( 0.0 );

			m_dHAvail = GetpDpC()->GetDpmin() + dDpCv;
		}
		else if( Hm3WInj_case_B_4 == iBSubCase )
		{
			// We will first size the Dp controller with current flow and take the one that can stabilize the bigger pressure drop between CV and BV.

			// We invalidate selection. Otherwise the "SelectBestDpC" method will try to match a Dp controller with what was previously selected.
			GetpDpC()->InvalidateSelection();

			double dDpCv = GetpCV()->GetDp();
			double dDpBv = GetpBv()->GetDp();
			GetpDpC()->SelectBestDpC( 0.0 );

			// We can now compute the setting of the BV at the primary side to have the same pressure drop as the control valve.
			GetpBv()->ResizeBv( dDpCv );

			m_dHAvail = GetpDpC()->GetDpmin() + dDpCv;
		}
		
		// Now we have to update secondary side with the pressure drop accross the control valve.
		GetpSecBv()->ResizeBv( 0.0 );

		double dHPumpMin = GetpSecBv()->GetDp();
		dHPumpMin += GetpCV()->GetDp();
		dHPumpMin += GetpTermUnit()->GetDp();

		if( false == IsaModule() && NULL != GetpCircuitSecondaryPipe() )
		{
			dHPumpMin += GetpCircuitSecondaryPipe()->GetPipeDp();
			dHPumpMin += GetpCircuitSecondaryPipe()->GetSingularityTotalDp( true );
		}

		GetpPump()->SetHmin( dHPumpMin );
	}

	if( Hm3WInj_case_C == iCase )
	{
		// We can directly size CV at the secondary side without any impact on the primary side.
		GetpCV()->SizeControlValveForBetaMin( 0.0 );
		dDpCV = GetpCV()->GetDp();

		// We can now size the balancing valve at the primary side with a 3 kPa minimum.
		GetpBv()->ResizeBv( 0.0 );
	}

	if( Hm3WInj_case_D == iCase )
	{
		// We size the balancing valve at the primary side with a 3 kPa minimum.
		GetpBv()->ResizeBv( 0.0 );

		// We now have this case to size the control valve yet. In the theory, control valve as either authority of 1 if the primary
		// flow is constant or an authority = DpCV / ( Hav@PI * ( 1 - DpBVp / Hav ) ).
		// It's in the "CDS_HydromMod::CCv::SizeControlValveForBetaMin" method that we will make the difference.
		GetpCV()->SizeControlValveForBetaMin( 0.0 );
	}

	if( Hm3WInj_case_E == iCase )
	{
		// We can directly size CV at the secondary side without any impact on the primary side.
		GetpCV()->SizeControlValveForBetaMin( 0.0 );
		dDpCV = GetpCV()->GetDp();

		// We will first size the Dp controller with current flow and take the one with the lowest Dplmin.
		// Remark: the "CDS_HydroMod::DpC::SelectBestDpC" method calls the "CDS_HydroMod::DpC::GetDpToStab" method to know what is the 
		//         differential pressure to stabilise on the branch. Here, we are in a particular case because we want the Dp controler 
		//         with the lowest Dplmin and not a particular stabilized differential pressure. So we need that the "SelectBestDpC" method 
		//         does not call the "GetDpToStab" method but uses 0 instead. This is why we have now the second argument "bLowestDplmin" 
		//         when calling the "SelectBestDpC" method to force this method to not call the "GetDpToStab" method.

		// We invalidate selection. Otherwise the "SelectBestDpC" method will try to match a Dp controller with what was previously selected.
		GetpDpC()->InvalidateSelection();
		GetpDpC()->SelectBestDpC( 0.0, true );

		// We can size the BV at the primary side with the same pressure drop that the one that Dp controller stabilizes.
		double dDpBV = 0.0;

		if( NULL != GetpDpC() && GetpDpC()->GetDplmin() > 0.0 )
		{
			dDpBV = GetpDpC()->GetDplmin();
		}

		GetpBv()->ResizeBv( dDpBV );
	}

	if( Hm3WInj_case_F == iCase )
	{
		// We will first size the Dp controller with current flow and take the one with the lowest Dplmin.
		// Remark: the "CDS_HydroMod::DpC::SelectBestDpC" method calls the "CDS_HydroMod::DpC::GetDpToStab" method to know what is the 
		//         differential pressure to stabilise on the branch. Here, we are in a particular case because we want the Dp controler 
		//         with the lowest Dplmin and not a particular stabilized differential pressure. So we need that the "SelectBestDpC" method 
		//         does not call the "GetDpToStab" method but uses 0 instead. This is why we have now the second argument "bLowestDplmin" 
		//         when calling the "SelectBestDpC" method to force this method to not call the "GetDpToStab" method.

		// We invalidate selection. Otherwise the "SelectBestDpC" method will try to match a Dp controller with what was previously selected.
		GetpDpC()->InvalidateSelection();
		GetpDpC()->SelectBestDpC( 0.0, true );

		// For this case, we will first size the BV at the primary side with a 3 kPa minimum.
		GetpBv()->ResizeBv( 0.0 );

		// Now we can size the CV at the primary by taking the difference between the differential pressure on the 
		// branch and the pressure drop on the BV.

		// If primary flow is constant, the "CDS_HydromMod::CCv::SizeControlValveForBetaMin" method will well retrieve the differential pressure
		// stabilized by the Dp controller and remove the pressure drop of the balancing valve if it is connected to the secondary (Normally
		// it is always the case).

		// If primary flow is variable, the "CDS_HydromMod::CCv::SizeControlValveForBetaMin" method will compute the CV as it is done for 
		// 2-way control valve with variable flow. The problem is that in this case, if a Dp controller stabilises the differential pressure on the
		// branch, we take the Havailable (Here Dplmin) and do a correction for the proportionnal band of the Dp controller and we try to size the
		// control valve with 0.25 this value to satisfy the authority. But, for this circuit F, the authority is 1. So, we don't want to
		// size the control valve like this but in the same way as if the primary flow in constant.

		// So in the "CDS_HydromMod::CCv::SizeControlValveForBetaMin" method, if we detect this circuit, we force the 'bConstantFlow' to 'true'.
		GetpCV()->SizeControlValveForBetaMin( 0.0 );
	}

	m_bComputeHMInjAllowed = true;
}

double CDS_Hm3WInj::GetTechParamCVMinDpProp()
{
	// In case of 3WayMixing circuit with decoupling bypass, Minimum CV Dp 3kPa
	// See RP book 3.4.3.3 p226 3Ed.
	if( NULL != GetpSchcat() &&
		( GetpSchcat()->GetVarSecondFlow() == CDB_CircSchemeCateg::Constant && GetpSchcat()->GetVarPrimFlow() == CDB_CircSchemeCateg::Constant )
		|| CDB_CircSchemeCateg::e3wTypeMixing == GetpSchcat()->Get3WType() )
	{
		return GetpTechParam()->GetCVMinDpOnoff();
	}

	return GetpTechParam()->GetCVMinDpProp();
}

void CDS_Hm3WInj::Copy( CDS_HydroMod *pTo, bool fValveDataOnly, bool fCreateLinkedDevice, bool fToClipboard, bool fResetDiversityFactor )
{
	CDS_HmInj::Copy(pTo, fValveDataOnly, fCreateLinkedDevice, fToClipboard, fResetDiversityFactor );
}

void CDS_Hm3WInj::Copy(CData *pTo)
{
	((CDS_HydroMod *)pTo)->SetSchemeIDPtr(GetSchemeIDPtr());
	CDS_Hm3WInj::Copy((CDS_Hm3WInj *)pTo, false, true, false, false);
}

#define CDS_Hm3WInj_VERSION	1
void CDS_Hm3WInj::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_Hm3WInj_VERSION;
	WriteData<>( outf, Version );

	// Base class.
	CDS_HmInj::Write( outf );
}

bool CDS_Hm3WInj::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_Hm3WInj_VERSION;
	ReadData<>( inpf, Version );

	if( Version < 1 || Version > CDS_Hm3WInj_VERSION )
	{
		return false;
	}

	if( false == CDS_HmInj::Read( inpf ) )
	{
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
//			CDS_HM3W
//////////////////////////////////////////////////////////////////////
CDS_Hm3W::CDS_Hm3W( CDataBase *pDataBase, LPCTSTR ID )
	: CDS_HydroMod( pDataBase, ID )
{
	m_UseBypBv = eUseProduct::Auto;
	m_ComputeHM3WState = CDS_HydroMod::eceNone;
	m_ComputeHM3WParam = 0;
	m_SavedIDPtr = _NULL_IDPTR;
}

void CDS_Hm3W::Init( IDPTR SchIDPtr, CTable *plcParent, bool bUseTACV, bool fCreateLinkedDevice )
{
	CDS_HydroMod::Init( SchIDPtr, plcParent, true );

	CDB_CircuitScheme *pSch = static_cast<CDB_CircuitScheme *>( SchIDPtr.MP );

	if( NULL != pSch )
	{
		m_UseBypBv = eUseProduct::Auto;
		CreateCv( pSch->GetCvLoc() );

		// Create Bypass BV.
		CreateBv( CDS_HydroMod::eHMObj::eBVbyp );
		GetpCV()->SetTACv( bUseTACV );
	}
}

#define CDS_HM3W_VERSION	2
void CDS_Hm3W::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_HM3W_VERSION;
	WriteData<>( outf, Version );

	// Base class.
	CDS_HydroMod::Write( outf );

	// Info.
	WriteData<>( outf, m_UseBypBv );
}

bool CDS_Hm3W::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_HM3W_VERSION;
	ReadData<>( inpf, Version );

	if( Version < 1 || Version > CDS_HM3W_VERSION )
	{
		return false;
	}

	// Base class.
	if( false == CDS_HydroMod::Read( inpf ) )
	{
		return false;
	}

	// Info.
	double dDummy;

	if( 1 == Version )
	{
		ReadData<>( inpf, dDummy );
		return true;
	}

	ReadData<>( inpf, m_UseBypBv );
	return true;
}

void CDS_Hm3W::Copy( CDS_HydroMod *pTo, bool fValveDataOnly, bool fCreateLinkedDevice, bool fToClipboard, bool fResetDiversityFactor )
{
	pTo->SetSchemeID( GetSchemeID() );
	CDS_HydroMod::Copy(pTo, fValveDataOnly, fCreateLinkedDevice, fToClipboard, fResetDiversityFactor );
	((CDS_HydroMod *)pTo)->EnableComputeHM(false);
	((CDS_HydroMod *)pTo)->SetLock(m_usLock);
	// Flag UseBypBv() chould be set after the BypValv creation
	( static_cast<CDS_Hm3W *>(pTo) )->SetUseBypBv( GetUseBypBv() );
	((CDS_HydroMod *)pTo)->SetLock(m_usLock);
	((CDS_HydroMod *)pTo)->EnableComputeHM(true);
}

void CDS_Hm3W::Copy(CData *pTo)
{
	((CDS_HydroMod *)pTo)->SetSchemeIDPtr(GetSchemeIDPtr());
	CDS_Hm3W::Copy((CDS_Hm3W *)pTo, false, true, false, false);
}

double CDS_Hm3W::GetBypOverFlow()
{
	if( GetHAvail( CAnchorPt::CircuitSide_Primary ) <= 0.0 )
	{
		return 0.0;
	}

	if( false == IsTermUnitExist() || GetDpOnOutOfPrimSide() <= 0.0 )
	{
		return 0.0;
	}

	double dFlowIncrease = GetDpOnOutOfPrimSide() / GetHAvail( CAnchorPt::CircuitSide_Primary );
	dFlowIncrease = max( sqrt( 1 / ( 1 - dFlowIncrease ) ) * 100 - 100.0, 0 );
	return dFlowIncrease;
}

// According to RP book (p206 ed.3) flow corresponding to a Dp/H = 0.3.
#define MaxBypassOverFlow 20.0
void CDS_Hm3W::SetUseBypBv( eUseProduct UseBypBv )
{
	if( NULL == GetpBypBv() )
	{
		return;
	}

	if( m_UseBypBv != UseBypBv )
	{
		Modified();
	}

	m_UseBypBv = UseBypBv;

	switch( m_UseBypBv )
	{
		case eUseProduct::Always:
			if( NULL == GetpBypBv()->GetIDPtr( ).MP )
			{
				GetpBypBv()->SetIDPtr( m_SavedIDPtr );
			}

			ComputeHM( eceBypBvAdded );
			ResetSchemeIDPtr();
			break;

		case eUseProduct::Never:
			if( 0 != *GetpBypBv()->GetIDPtr().ID )
			{
				SetLock( eBVbyp, false, false, true );
				m_SavedIDPtr = GetpBypBv()->GetIDPtr( );
				GetpBypBv()->SetIDPtr( _NULL_IDPTR );
				ResetSchemeIDPtr();
			}

			break;

		case eUseProduct::Auto:

			// TODO max value for overflow must be stored in technical parameters?
			if( GetBypOverFlow() > MaxBypassOverFlow )
			{
				if( NULL == GetpBypBv()->GetIDPtr( ).MP )
				{
					GetpBypBv()->SetIDPtr( m_SavedIDPtr );
				}

				ComputeHM( eceBypBvAdded );
				ResetSchemeIDPtr();
			}
			else
			{
				SetLock( eBVbyp, false, false, true );
				m_SavedIDPtr = GetpBypBv()->GetIDPtr( );
				GetpBypBv()->SetIDPtr( _NULL_IDPTR );
				ResetSchemeIDPtr();
			}

			break;
	}
}

void CDS_Hm3W::ResizeBalDevForH( double dHAvail, bool bForPending, CAnchorPt::CircuitSide eCircuitSide, bool *pbHasHAvailSecChanged )
{
	if( NULL != pbHasHAvailSecChanged )
	{
		*pbHasHAvailSecChanged = false;
	}

	if( dHAvail > 0.0 && NULL == GetpDpC() )
	{
		if( NULL != GetpCV() && false == GetpCV()->IsPresettable() )
		{
			dHAvail -= GetpCV()->GetDp();
		}
	}

	CDS_HydroMod::ResizeBalDevForH( dHAvail, bForPending, eCircuitSide, pbHasHAvailSecChanged );
	
	// Create or Delete BypBv if needed.
	SetUseBypBv( GetUseBypBv() );
}

void CDS_Hm3W::ComputeHM( eComputeHMEvent ComputeHMEvent, LPARAM pObj )
{
	if( false == IsHMCalcMode() )
	{
		return;
	}

	// During HydroMod copy Compute HydroMod is deactivated.
	if( false == m_bComputeAllowed )
	{
		return;
	}

	// Sanity tests.
	if( m_ComputeHM3WState > eceLAST || m_ComputeHM3WState < eceNone )
	{
		ASSERT( 0 );
		m_ComputeHM3WState = eceNone;
	}

	// If the engine is already running discard new event.
	if( m_ComputeHM3WState != eceNone && m_ComputeHM3WState != eceLAST )
	{
		return;
	}

	SetComputeHM3WEngine( ComputeHMEvent, pObj );

	// Compute base.
	CDS_HydroMod::ComputeHM( ComputeHMEvent, pObj );

	bool fAllShouldBeSized = false;

	while( m_ComputeHM3WState != eceNone && m_ComputeHM3WState != eceLAST )
	{
		switch( m_ComputeHM3WState )
		{
			case eceTUDp:
			case eceResize:
			case eceTUQ:
				if( NULL != GetpBypBv() )
				{
					// Sizing Bv with same flow that flows in TU.
					GetpBypBv()->SetQ( GetpTermUnit()->GetQ() );

					if( GetBypOverFlow() > MaxBypassOverFlow )
					{
						// Call base loop to size bypass Bv.
						CDS_HydroMod::ComputeHM( eceBVType, (LPARAM)GetpBypBv() );
					}
				}

				SetComputeHM3WEngine( eceLAST );
				break;

			case eceBypBvAdded:
				// Sizing Bv with same flow that flows in TU.
				GetpBypBv()->SetQ( GetpTermUnit()->GetQ() );
				// Call base loop to size bypass Bv.
				CDS_HydroMod::ComputeHM( eceBVType, (LPARAM)GetpBypBv() );
				SetComputeHM3WEngine( eceLAST );
				break;

			default:
				SetComputeHM3WEngine( eceLAST );
		}
	}
}

bool CDS_Hm3W::IsCompletelyDefined( int iHMObjToCheck )
{
	if( NULL != GetpSch() && true == GetpSch()->IsAnchorPtOptional( CAnchorPt::BV_Byp ) && eUseProduct::Always != m_UseBypBv )
	{
		// In this case, user has intentionally chosen to not have the bypass valve and then we don't verify it.
		iHMObjToCheck &= ~( ( int )eHMObj::eBVbyp );
	}

	return CDS_HydroMod::IsCompletelyDefined( iHMObjToCheck );
}

void CDS_Hm3W::TransferValvesInfoFromHMX( CDS_HydroModX *pHMX, int iHMObjToCheck )
{
	if( NULL != pHMX && NULL != pHMX->GetpBV( CDS_HydroModX::InBypass ) && NULL != GetpSch() && true == GetpSch()->IsAnchorPtOptional( CAnchorPt::BV_Byp ) )
	{
		CreateBv( eBVbyp );
		CDS_HydroMod::CBV *pBvByp = GetpBypBv();

		if( NULL != pBvByp )
		{
			CDS_HydroModX::CBV *pHMXBvByp = pHMX->GetpBV( CDS_HydroModX::InBypass );
			pBvByp->SetIDPtr( pHMXBvByp->GetValveIDPtr() );
			pBvByp->SetSetting( pHMXBvByp->GetDesignOpening() );
			pBvByp->SetLocate( CDS_HydroMod::eHMObj::eBVbyp );
			m_UseBypBv = ( CDS_HydroMod::eUseProduct )pHMXBvByp->GetUseProduct();

			// Do not allow base class to manage BV bypass be cause it is already done here.
			iHMObjToCheck &= ~( ( int )eHMObj::eBVbyp );
		}
	}

	CDS_HydroMod::TransferValvesInfoFromHMX( pHMX, iHMObjToCheck );
}

void CDS_Hm3W::FillOtherInfoIntoHMX( CDS_HydroModX *pHMX )
{
	// When transferring info into HMX (to upload hydraulic network to TA-Scope), the 'CTableHM::SaveToHMXTable' method in
	// 'DataStruct.cpp' file is called. This method will check if the Bv bypass valve exists by calling the 'CDS_HydoMod::IsBvBypExist()' method.
	// In the case of the Bv bypass valve is optional we have to remember choice of the user (Always, auto or never). If user choices 'Never',
	// the 'm_IDPtr' variable in the 'CDS_HydroMod::m_pBypBv' object will be NULL. Thus 'CDS_HydroMod::IsBvBypExist' will return 'false'.
	// But we need to save 'Never' info. This is why in this case we create a Bv bypass object in HMX and set 'm_UseBypBv' variable.

	if( NULL != pHMX && NULL != GetpSch() && true == GetpSch()->IsAnchorPtExist( CAnchorPt::BV_Byp ) && true == GetpSch()->IsAnchorPtOptional( CAnchorPt::BV_Byp ) )
	{
		CDS_HydroModX::CBV *pHMXBypBv = pHMX->GetpBV( CDS_HydroModX::InBypass );
		CDS_HydroMod::CBV *pHMBypBv = GetpBypBv();

		if( NULL == pHMXBypBv )
		{
			pHMXBypBv = pHMX->AddBv( CDS_HydroModX::eLocate::InBypass, CDS_HydroModX::eMode::TAmode );
			pHMXBypBv->SetValveID( ( NULL != pHMBypBv ) ? pHMBypBv->GetIDPtr().ID : _T("") );
			pHMXBypBv->SetDesignOpening( ( NULL != pHMBypBv ) ? pHMBypBv->GetSetting() : 0.0 );
			pHMXBypBv->SetCurOpening( ( NULL != pHMBypBv ) ? pHMBypBv->GetSetting() : 0.0 );
			pHMXBypBv->SetDesignFlow( GetQ() );
		}

		// Allow to memorize choice of user (Auto, always or never).
		pHMXBypBv->SetUseProduct( ( CDS_HydroModX::eUseProduct )m_UseBypBv );
	}

	CDS_HydroMod::FillOtherInfoIntoHMX( pHMX );
}
