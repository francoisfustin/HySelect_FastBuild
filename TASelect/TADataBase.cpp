#include "stdafx.h"

#include <math.h>
#include <float.h>
#include <vector>
#include <algorithm>

#ifndef TACBX
#include "TASelect.h"
#include "ModuleVersion.h"
#else
#include "global.h"
#include "TAPersist.hpp"
#endif

#ifndef WIN32
#include "crc.h"
#endif

#include "TADataBase.h"
#include "datastruct.h"

// Load DB from independent task
#ifndef WIN32
#include "smx.h"
#include <usart.h>
extern TCB_PTR InitialyserTask;
#endif

///////////////////////////////////////////////////////////////////////////////////
//
//								CTADatabase
//
///////////////////////////////////////////////////////////////////////////////////

CTADatabase::CTADatabase()
	: CDataBase( CDataBase::TADatabase )
{
}

bool CTADatabase::IsIncludedInMVTAB( _string strFamilyID, _string strVersionID, CTable *pclTable )
{
	// Be careful if you use this function in DB loop, you must initialize pTab before calling it.
	if( NULL == pclTable )
	{
		pclTable = static_cast<CTable *>( Get( _T("MEASVALV_TAB") ).MP );
	}

	ASSERT( NULL != pclTable );

	bool bFound = false;
	IDPTR IDPtr = _NULL_IDPTR;

	for( IDPtr = pclTable->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *IDPtr.ID && false == bFound; IDPtr = pclTable->GetNext( IDPtr.MP ) )
	{
		CDB_StringID *pStrID = (CDB_StringID *)( IDPtr.MP );
		
		// Test Family if exist.
		_string str( pStrID->GetIDstr() );

		if( true == str.empty() || true == strFamilyID.empty() || str == strFamilyID )
		{
			// If family match test Version if exist.
			str = pStrID->GetString();

			if( true == str.empty() || true == strVersionID.empty() || str == strVersionID )
			{
				bFound = true;
			}
		}
	}

	return bFound;
}

#ifdef TACBX
int CTADatabase::GetBVTypeList( CRankEx *pList, FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bRemoveFO, bool bRemoveSTAM, bool bForDpC )
#else
int CTADatabase::GetBVTypeList( CRankEx *pList, FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bRemoveTVFO, bool bRemoveSTAM, bool bForDpC )
#endif
{
#ifndef TACBX
	bool bRemoveFO = bRemoveTVFO;
#endif

	CTable *pMeasTab = static_cast<CTable *>( Get( _T("MEASVALV_TAB") ).MP );
	CTable *pTab = static_cast<CTable *>( Get( _T("RVTYPE_TAB") ).MP );

	if( NULL == pTab || NULL == pMeasTab )
	{
		return 0;
	}

	std::vector <LPARAM> ar( pTab->GetItemCount() );
	int i = 0;

	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		ASSERT( i < ( int )ar.size() );

		if( true == bRemoveFO && 0 == _tcscmp( IDPtr.ID, _T("RVTYPE_FO") ) )
		{
			continue;
		}

		if( true == bRemoveSTAM && 0 == _tcscmp( IDPtr.ID, _T("RVTYPE_MV") ) )
		{
			continue;
		}

		LPARAM &lparam = ar[i];
		lparam = (LPARAM)IDPtr.MP;
		i++;
	}

	// Remove unused elements.
	ar.resize( i );

	// Try to find at least one Regulating valve available for this type.
	pTab = static_cast<CTable *>( Get( _T("REGVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;

	for( i = 0; i < ( int )ar.size(); )
	{
		CDB_StringID *pSID = dynamic_cast<CDB_StringID *>( ( CData * )ar[i] );
		ASSERT( NULL != pSID );

		if( NULL == pSID )
		{
			continue;
		}

		_string strType = pSID->GetIDPtr().ID;
		bool bSelectable = false;

		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
		{
			CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( ( CData * )IDPtr.MP );

			if( NULL == pTAp )
			{
				continue;
			}

			if( false == pTAp->IsSelectable( true, false, bOldProductsAvailableForHC ) )
			{
				continue;
			}

			if( true == MustExclude( pTAp, eFilterSelection ) )
			{
				continue;
			}

			// Scan all regulating valves with the same type.
			if( strType != pTAp->GetTypeIDPtr().ID )
			{
				continue;
			}

			#ifdef TACBX

			if( CDB_TAProduct::empNone == pTAp->GetMeasPt() )
			{
				continue;
			}

			#endif

			int iDN = pTAp->GetSizeKey();

			if( iDN < iDNMin )
			{
				continue;
			}

			if( iDN > iDNMax )
			{
				continue;
			}

			// DpC compliant?
			if( true == bForDpC && false == IsIncludedInMVTAB( pTAp->GetFamilyID(), pTAp->GetVersionID(), pMeasTab ) )
			{
				continue;
			}

			bSelectable = true;

			// Loop aborted one BV of this type is enough to display the type.
			break;
		}

		if( false == bSelectable )
		{
			ar.erase( ar.begin() + i );
		}
		else
		{
			i++;
		}
	}

	// Transfer and sort list.
	for( i = 0; i < ( int )ar.size(); i++ )
	{
		pList->Add( ( (CDB_StringID *)ar[i] )->GetString(), _ttoi( ( (CDB_StringID *)ar[i] )->GetIDstr() ) - 1, ar[i] );
	}

	i = ( int )ar.size();
	ar.clear();
	return i;
}

int CTADatabase::GetBVTypeList( CRankEx *pSrcList, CRankEx *pDestList )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclValve = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclValve );

		if( NULL == pclValve )
		{
			continue;
		}

		// Add the new type ID if doesn't already exist.
		IDPTR TypeIDPtr = pclValve->GetTypeIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( TypeIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)TypeIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)TypeIDPtr.MP )->GetString(), dKey, (LPARAM)TypeIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}

int CTADatabase::GetBVFamilyList( CRankEx *pList, _string strSelTypeID, FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bForDpC, bool bIsForTools )
{
	// Fill the list according to selected type.
	CTable *pTab = static_cast<CTable *>( Get( _T("REGVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	CTable *pMeasTab = static_cast<CTable *>( Get( _T("MEASVALV_TAB") ).MP );
	ASSERT( NULL != pMeasTab );

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Selectable ?
		if( false == pTAp->IsSelectable( true, false, bOldProductsAvailableForHC, bIsForTools ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pTAp->GetTypeIDPtr().ID )
		{
			continue;
		}

#ifdef TACBX
		if( CDB_TAProduct::empNone == pTAp->GetMeasPt() )
		{
			continue;
		}
#endif

		// DpC compliant?
		if( true == bForDpC && false == IsIncludedInMVTAB( pTAp->GetFamilyID(), pTAp->GetVersionID(), pMeasTab ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey;
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		dKey = pTAp->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pTAp->GetFamilyIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pTAp->GetFamilyIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pTAp->GetFamilyIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetBVFamilyList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclValve = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclValve );

		if( NULL == pclValve )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Add the new family ID if doesn't already exist.
		IDPTR FamilyIDPtr = pclValve->GetFamilyIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( FamilyIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)FamilyIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)FamilyIDPtr.MP )->GetString(), dKey, (LPARAM)FamilyIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetBVBdyMatList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, FilterSelection eFilterSelection, int iDNMin, int iDNMax,
		bool bForDpC )
{
	// Fill the list according to selected type.
	CTable *pTab = static_cast<CTable *>( Get( _T("REGVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	CTable *pMeasTab = static_cast<CTable *>( Get( _T("MEASVALV_TAB") ).MP );
	ASSERT( NULL != pMeasTab );

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Selectable ?
		if( false == pTAp->IsSelectable( true, false, bOldProductsAvailableForHC ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pTAp->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pTAp->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// DpC compliant?
		if( true == bForDpC && false == IsIncludedInMVTAB( pTAp->GetFamilyID(), pTAp->GetVersionID(), pMeasTab ) )
		{
			continue;
		}

		// Test DN
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Bdy | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pTAp->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pTAp->GetBodyMaterialIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pTAp->GetBodyMaterialIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pTAp->GetBodyMaterialIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetBVBdyMatList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclValve = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclValve );

		if( NULL == pclValve )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Add the new material ID if doesn't already exist.
		IDPTR MaterialIDPtr = pclValve->GetBodyMaterialIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( MaterialIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)MaterialIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)MaterialIDPtr.MP )->GetString(), dKey, (LPARAM)MaterialIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetBVConnList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID, FilterSelection eFilterSelection, int iDNMin,
		int iDNMax, bool bForDpC )
{
	// Fill the list according to selected type.
	CTable *pTab = static_cast<CTable *>( Get( _T("REGVALV_TAB") ).MP );
	ASSERT( NULL != pTab );
	CTable *pMeasTab = static_cast<CTable *>( Get( _T("MEASVALV_TAB") ).MP );
	ASSERT( NULL != pMeasTab );

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Selectable ?
		if( false == pTAp->IsSelectable( true, false, bOldProductsAvailableForHC ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pTAp->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pTAp->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pTAp->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// DpC compliant?
		if( true == bForDpC && false == IsIncludedInMVTAB( pTAp->GetFamilyID(), pTAp->GetVersionID(), pMeasTab ) )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Connect | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pTAp->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pTAp->GetConnectIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pTAp->GetConnectIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pTAp->GetConnectIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetBVConnList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, _string strMaterialID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclValve = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclValve );

		if( NULL == pclValve )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test material ID.
		if( false == strMaterialID.empty() && strMaterialID != pclValve->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Add the new connection ID if doesn't already exist.
		IDPTR ConnectIDPtr = pclValve->GetConnectIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( ConnectIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)ConnectIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)ConnectIDPtr.MP )->GetString(), dKey, (LPARAM)ConnectIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetBVVersList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID, _string strSelConnectID,
		FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bForDpC )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("REGVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	CTable *pMeasTab = static_cast<CTable *>( Get( _T("MEASVALV_TAB") ).MP );
	ASSERT( NULL != pMeasTab );

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Keep only CDB_TAProduct and children.
		if( false == pTAp->IsSelectable( true, false, bOldProductsAvailableForHC ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pTAp->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pTAp->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pTAp->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		if( false == strSelConnectID.empty() && strSelConnectID != pTAp->GetConnectIDPtr().ID )
		{
			continue;
		}

		if( 0 == _tcscmp( pTAp->GetVersionIDPtr().ID, _T("VERS_EMPTY") ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// DpC compliant?
		if( true == bForDpC && false == IsIncludedInMVTAB( pTAp->GetFamilyID(), pTAp->GetVersionID(), pMeasTab ) )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Version | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pTAp->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pTAp->GetVersionIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pTAp->GetVersionIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pTAp->GetVersionIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetBVVersList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, _string strMaterialID,
		_string strConnectID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclValve = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclValve );

		if( NULL == pclValve )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test material ID.
		if( false == strMaterialID.empty() && strMaterialID != pclValve->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Test connection ID.
		if( false == strConnectID.empty() && strConnectID != pclValve->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Add the new version ID if doesn't already exist.
		IDPTR VersionIDPtr = pclValve->GetVersionIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( VersionIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)VersionIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)VersionIDPtr.MP )->GetString(), dKey, (LPARAM)VersionIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetBVPNList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID, _string strSelConnectID,
		_string strSelVersionID, FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bForDpC )
{
	// Fill the PN combo according to selected type, connection and version.
	double dPN = 0.0;
	double dDP = 0.0;
	double dPmax = 0.0;

	CTable *pTab = static_cast<CTable *>( Get( _T("REGVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	CTable	*pPNTab = static_cast<CTable *>( Get( _T("PN_TAB") ).MP );
	ASSERT( NULL != pPNTab );

	CTable *pMeasTab = static_cast<CTable *>( Get( _T("MEASVALV_TAB") ).MP );
	ASSERT( NULL != pMeasTab );

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Loop on selectable regulating valves (Those with Pmax = Pmax of previous item are not treated).
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Keep only CDB_TAProduct and children.
		if( false == pTAp->IsSelectable( true, false, bOldProductsAvailableForHC ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pTAp->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pTAp->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pTAp->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		if( false == strSelConnectID.empty() && strSelConnectID != pTAp->GetConnectIDPtr().ID )
		{
			continue;
		}

		if( false == strSelVersionID.empty() && strSelVersionID != pTAp->GetVersionIDPtr().ID )
		{
			continue;
		}

		// DpC compliant?
		if( true == bForDpC && false == IsIncludedInMVTAB( pTAp->GetFamilyID(), pTAp->GetVersionID(), pMeasTab ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		if( dPmax != pTAp->GetPmaxmax() )
		{
			dPmax =  pTAp->GetPmaxmax();
			// Find the largest PN lower than Pmax.
			dDP = DBL_MAX;
			IDPTR PNIDPtrFnd;

			for( IDPTR PNIDPtr = pPNTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *PNIDPtr.ID; PNIDPtr = pPNTab->GetNext() )
			{
				// Attention: in the PN_TAB we can't get either 'CDB_StringID' or 'CDB_RuledTable' object.
				if( NULL != dynamic_cast<CDB_RuledTable *>( PNIDPtr.MP ) )
				{
					continue;
				}

				if( NULL == dynamic_cast<CDB_StringID *>( PNIDPtr.MP ) )
				{
					ASSERTA_RETURN( -1 );
				}

				CDB_StringID *pclStringID = dynamic_cast<CDB_StringID *>( PNIDPtr.MP );

				if( RD_OK != ReadDoubleFromStr( (TCHAR *)pclStringID->GetIDstr(), &dPN ) )
				{
					ASSERTA_RETURN( -1 );
				}

				if( ( dPmax - dPN ) >= 0.0 && fabs( dPmax - dPN ) < dDP )
				{
					dDP = fabs( dPmax - dPN );
					PNIDPtrFnd = PNIDPtr;
				}
			}

			// Found product having priority.
			double dKey = 0;
			( (CDB_StringID *)PNIDPtrFnd.MP )->GetIDstrAs( 0, dKey );

			// Already added ?
			_string InListstr;
			LPARAM InListlparam = (LPARAM)PNIDPtrFnd.MP;
			double dInListkey = 0;

			if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
			{
				// Yes take a copy, check priority key, and keep only the most important.
				if( dKey < dInListkey )
				{
					pList->Delete( dInListkey );
				}
				else
				{
					continue;
				}
			}

			pList->Add( ( ( CDB_String * )PNIDPtrFnd.MP )->GetString(), dKey, (LPARAM)PNIDPtrFnd.MP );
		}
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetBVPNList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, _string strMaterialID,
		_string strConnectID, _string strVersionID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclValve = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclValve );

		if( NULL == pclValve )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test material ID.
		if( false == strMaterialID.empty() && strMaterialID != pclValve->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Test connection ID.
		if( false == strConnectID.empty() && strConnectID != pclValve->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Test version ID.
		if( false == strVersionID.empty() && strVersionID != pclValve->GetVersionIDPtr().ID )
		{
			continue;
		}

		// Add the new PN ID if doesn't already exist.
		IDPTR PNIDPtr = pclValve->GetPNIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( PNIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)PNIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)PNIDPtr.MP )->GetString(), dKey, (LPARAM)PNIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetBVList( CRankEx *pList, _string strSelTypeID, _string strSelFamilyID, _string strSelBdyMatID, _string strSelConnectID,
		_string strSelVersionID, FilterSelection eFilterSelection, int iDNMin, int iDNMax, CData *pProd, bool bForDpC, bool bIsForTools )
{
	// By default balancing valves are defined in 'REGVALV_TAB'.
	CTable *pTab = static_cast<CTable *>( Get( _T("REGVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	CTable *pMeasTab = static_cast<CTable *>( Get( _T("MEASVALV_TAB") ).MP );
	ASSERT( NULL != pMeasTab );

	// First verify if 'SelTypeID' is defined.
	if( false == strSelTypeID.empty() )
	{
		// If defined, check if it's well a 'RVTYPE'.
		CTable *pTypeTable = static_cast<CTable *>( Get( _T("RVTYPE_TAB") ).MP );

		if( NULL != pTypeTable )
		{
			CDB_StringID *pType = (CDB_StringID *)( pTypeTable->Get( strSelTypeID.c_str() ).MP );

			// If type is well a 'RVTYPE'...
			if( NULL != pType )
			{
				// Check in which table valves of this type are defined.
				// Remark: it's a special case for example 'RVTYPE_RV_INSERT' where valves are defined in 'RVINSERT_TAB' instead of
				//         'REGVALV_TAB'.
				if( NULL != static_cast<CTable *>( Get( pType->GetIDstr2() ).MP ) )
				{
					pTab = static_cast<CTable *>( Get( pType->GetIDstr2() ).MP );
				}
			}
		}
	}

	// For increasing 'if' test below.
	CDB_StringID *pSelTypeID = NULL;
	CDB_StringID *pSelFamilyID = NULL;
	CDB_StringID *pSelBdyMatID = NULL;
	CDB_StringID *pSelConnectID = NULL;
	CDB_StringID *pSelVersionID = NULL;

	if( false == strSelTypeID.empty() )
	{
		pSelTypeID = ( (CDB_StringID *)Get( strSelTypeID.c_str() ).MP );
	}

	if( false == strSelFamilyID.empty() )
	{
		pSelFamilyID = ( (CDB_StringID *)Get( strSelFamilyID.c_str() ).MP );
	}

	if( false == strSelBdyMatID.empty() )
	{
		pSelBdyMatID = ( (CDB_StringID *)Get( strSelBdyMatID.c_str() ).MP );
	}

	if( false == strSelConnectID.empty() )
	{
		pSelConnectID = ( (CDB_StringID *)Get( strSelConnectID.c_str() ).MP );
	}

	if( false == strSelVersionID.empty() )
	{
		pSelVersionID = ( (CDB_StringID *)Get( strSelVersionID.c_str() ).MP );
	}
	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Loop on selectable regulating valves.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		// Keep only CDB_TAProduct and children.
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );

		if( NULL == pTAp )
		{
			continue;
		}

		if( false == pTAp->IsSelectable( true, false, bOldProductsAvailableForHC, bIsForTools ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( NULL != pSelTypeID && pSelTypeID != ( (CDB_StringID *)pTAp->GetTypeIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelFamilyID && pSelFamilyID != ( (CDB_StringID *)pTAp->GetFamilyIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelBdyMatID && pSelBdyMatID != ( (CDB_StringID *)pTAp->GetBodyMaterialIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelConnectID && pSelConnectID != ( (CDB_StringID *)pTAp->GetConnectIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelVersionID && pSelVersionID != ( (CDB_StringID *)pTAp->GetVersionIDPtr().MP ) )
		{
			continue;
		}

		// DpC compliant?
		if( true == bForDpC && false == IsIncludedInMVTAB( pTAp->GetFamilyID(), pTAp->GetVersionID(), pMeasTab ) )
		{
			continue;
		}

		// HYS-1828: Only for DpC set, not breakable
		if( false == bForDpC && ePartOfaSetYes == pTAp->GetPartOfaSet() )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey = (double)pTAp->GetSortingKey();

		// Store Valve characteristic ID.
		CData *pValveChar = pTAp->GetValveCharDataPointer();
		bool bCouldbeAdded = true;			// Could be added by default

		if( NULL != pValveChar )
		{
#ifdef TACBX
			bCouldbeAdded = CheckIfCharactAlreadyExist( pList, pTAp, pProd );
			
			// Attention: STAD* without drain Global and Nordic versions are exactly the same. It's just
			// the color of the handwheel that changes, the characteristics are the same. We need in this
			// case to go deeper and set the preference on the product that belongs to the current database
			// version. If we are in INT version, we will put priority on the Global version. If we are in 
			// the SE version we will put the priority on the Nordic version.
			
			if( true == bCouldbeAdded )
			{
				bCouldbeAdded = CheckLocalizedVersion( pList, pTAp );
			}
#endif

			if( true == bCouldbeAdded )
			{
				pList->Add( pValveChar->GetIDPtr().ID, dKey, (LPARAM)IDPtr.MP );
			}
		}
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetBVFor6WayValveList( CRankEx *pList, _string strSelFamilyIDList, _string strSelBdyMatID, _string strSelConnectID,
		_string strSelVersionID, FilterSelection eFilterSelection, int iDNMin, int iDNMax, CData *pProd )
{
	// By default balancing valves are defined in 'REGVALV_TAB'.
	CTable *pclRegulatingValveTable = static_cast<CTable *>( Get( _T("REGVALV_TAB") ).MP );
	ASSERT( NULL != pclRegulatingValveTable );

	// For increasing 'if' test below.
	std::vector<CDB_StringID *> vecFamilyID;
	CDB_StringID *pSelBdyMatID = NULL;
	CDB_StringID *pSelConnectID = NULL;
	CDB_StringID *pSelVersionID = NULL;
	CDB_StringID *pSelFamID = NULL;
	// HYS-1375 : The bv family filter is checked if it is not empty
	if( false == strSelFamilyIDList.empty() )
	{
		pSelFamID = dynamic_cast<CDB_StringID*>( Get( strSelFamilyIDList.c_str() ).MP );
		if( NULL == pSelFamID )
		{
			if( false == strSelFamilyIDList.empty() )
			{
				CTable *pclBVFamilyListTable = static_cast<CTable *>( Get( strSelFamilyIDList.c_str() ).MP );
				ASSERT( NULL != pclBVFamilyListTable );

				for( IDPTR IDPtr = pclBVFamilyListTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclBVFamilyListTable->GetNext( IDPtr.MP ) )
				{
					vecFamilyID.push_back( (CDB_StringID *)( IDPtr.MP ) );
				}
			}
		}
	}


	if( false == strSelBdyMatID.empty() )
	{
		pSelBdyMatID = ( (CDB_StringID *)Get( strSelBdyMatID.c_str() ).MP );
	}

	if( false == strSelConnectID.empty() )
	{
		pSelConnectID = ( (CDB_StringID *)Get( strSelConnectID.c_str() ).MP );
	}

	if( false == strSelVersionID.empty() )
	{
		pSelVersionID = ( (CDB_StringID *)Get( strSelVersionID.c_str() ).MP );
	}

	// Loop on selectable regulating valves.
	for( IDPTR IDPtr = pclRegulatingValveTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclRegulatingValveTable->GetNext( IDPtr.MP ) )
	{
		// Keep only CDB_TAProduct and children.
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );

		if( NULL == pTAp )
		{
			continue;
		}

		if( false == pTAp->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		bool bContinue = false;

		if( vecFamilyID.size() > 0 )
		{
			for( auto &iter : vecFamilyID )
			{
				if( iter == (CDB_StringID *)pTAp->GetFamilyIDPtr().MP )
				{
					bContinue = true;
					break;
				}
			}
			if( false == bContinue )
			{
				continue;
			}
		}
		else if( NULL != pSelFamID && pSelFamID != ( (CDB_StringID*)pTAp->GetFamilyIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelBdyMatID && pSelBdyMatID != ( (CDB_StringID *)pTAp->GetBodyMaterialIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelConnectID && pSelConnectID != ( (CDB_StringID *)pTAp->GetConnectIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelVersionID && pSelVersionID != ( (CDB_StringID *)pTAp->GetVersionIDPtr().MP ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey = (double)pTAp->GetSortingKey();
		std::wstring wstr = _T("");
		pList->Add( wstr, dKey, (LPARAM)IDPtr.MP );
	}

	return pList->GetCount();
}
#endif

bool CTADatabase::OneAlienValvePresent()
{
	CTable *pTab = static_cast<CTable *>( Get( _T("UNIVERSALVALV_TAB") ).MP );

	if( NULL == pTab )
	{
		return false;
	}

	for( IDPTR TableIDPtr = pTab->GetFirst(); _T('\0') != *TableIDPtr.ID; TableIDPtr = pTab->GetNext( TableIDPtr.MP ) )
	{
		CTable *pManu = dynamic_cast<CTable *>( TableIDPtr.MP );

		if( NULL == pManu )
		{
			continue;
		}

		for( IDPTR ValveIDPtr = pManu->GetFirst(); _T('\0') != *ValveIDPtr.ID; ValveIDPtr = pManu->GetNext( ValveIDPtr.MP ) )
		{
			CRegValvAlienTab *pAlienValveTb = dynamic_cast<CRegValvAlienTab *>( ValveIDPtr.MP );

			if( NULL != pAlienValveTb )
			{
				for( IDPTR VIDPtr = pAlienValveTb->GetFirst(); _T('\0') != *VIDPtr.ID; VIDPtr = pAlienValveTb->GetNext( VIDPtr.MP ) )
				{
					CDB_AlienRegValve *pALIENp = dynamic_cast<CDB_AlienRegValve *>( VIDPtr.MP );

					if( NULL != pALIENp && true == pALIENp->IsAvailable() )
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

int CTADatabase::GetAlienManuList( CRankEx *pList )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("UNIVERSALVALV_TAB") ).MP );

	if( NULL == pTab )
	{
		return 0;
	}

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CTable *pManu = dynamic_cast<CTable *>( IDPtr.MP );

		if( NULL == pManu )
		{
			continue;
		}

		// Check if manufacturer has minimum one valve available in one family
		bool bOneValveExist = false;

		for( IDPTR IDPtrRegAlienValveTabIDPTR = pManu->GetFirst(); _T('\0') != *IDPtrRegAlienValveTabIDPTR.ID
				&& false == bOneValveExist; IDPtrRegAlienValveTabIDPTR = pManu->GetNext( IDPtr.MP ) )
		{
			CRegValvAlienTab *pAlienValveTb = dynamic_cast<CRegValvAlienTab *>( IDPtrRegAlienValveTabIDPTR.MP );

			if( NULL != pAlienValveTb )
			{
				for( IDPTR VIDPtr = pAlienValveTb->GetFirst(); _T('\0') != *VIDPtr.ID && false == bOneValveExist; VIDPtr = pAlienValveTb->GetNext( VIDPtr.MP ) )
				{
					CDB_AlienRegValve *pALIENp = dynamic_cast<CDB_AlienRegValve *>( VIDPtr.MP );

					if( NULL != pALIENp && true == pALIENp->IsAvailable() )
					{
						bOneValveExist = true;
					}
				}
			}
		}

		double dKey = 0;

		if( true == bOneValveExist )
		{
			pList->Add( pManu->GetName(), dKey, (LPARAM)pManu->GetIDPtr().MP );
		}
	}

	return pList->GetCount();
}

int CTADatabase::GetAlienFamilyList( CRankEx *pList, _string SelManuID )
{
	int iPos;

	CTable *pTab = static_cast<CTable *>( Get( SelManuID.c_str() ).MP );

	if( NULL == pTab )
	{
		return 0;
	}

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CFamAlienTab *pAlienFamTb = dynamic_cast<CFamAlienTab *>( IDPtr.MP );

		if( NULL != pAlienFamTb )
		{
			for( IDPTR FIDPtr = pAlienFamTb->GetFirst(); _T('\0') != *FIDPtr.ID; FIDPtr = pAlienFamTb->GetNext( FIDPtr.MP ) )
			{

				if( true == OneAlienValveExist( SelManuID, FIDPtr.ID ) )
				{
					CDB_StringID *pStrID =  dynamic_cast<CDB_StringID *>( FIDPtr.MP );
					pStrID->GetIDstrAs<int>( 0, iPos );

					pList->Add( FIDPtr.ID, (double) iPos, (LPARAM)FIDPtr.MP );
				}
			}
		}
	}

	return pList->GetCount();
}

int CTADatabase::GetAlienValveList( CRankEx *pList, _string SelManuID, _string SelFamID )
{
	CTable *pTab = static_cast<CTable *>( Get( SelManuID.c_str() ).MP );

	if( NULL == pTab )
	{
		return 0;
	}

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CRegValvAlienTab *pAlienValveTb = dynamic_cast<CRegValvAlienTab *>( IDPtr.MP );

		if( NULL != pAlienValveTb )
		{
			for( IDPTR VIDPtr = pAlienValveTb->GetFirst(); _T('\0') != *VIDPtr.ID; VIDPtr = pAlienValveTb->GetNext( VIDPtr.MP ) )
			{
				CDB_AlienRegValve *pALIENp = dynamic_cast<CDB_AlienRegValve *>( VIDPtr.MP );

				if( NULL == pALIENp )
				{
					continue;
				}

				if( SelFamID != pALIENp->GetFamIDPtr().ID )
				{
					continue;
				}

				if( false == pALIENp->IsAvailable() )
				{
					continue;
				}

				int iDN = pALIENp->GetSizeKey();
				pList->Add( pALIENp->GetIDPtr().ID, (double) iDN, (LPARAM)VIDPtr.MP );
			}
		}
	}

	return pList->GetCount();
}

bool CTADatabase::OneAlienValveExist( _string SelManuID, _string SelFamID )
{
	CTable *pTab = static_cast<CTable *>( Get( SelManuID.c_str() ).MP );

	if( NULL == pTab )
	{
		return false;
	}

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CRegValvAlienTab *pAlienValveTb = dynamic_cast<CRegValvAlienTab *>( IDPtr.MP );

		if( NULL != pAlienValveTb )
		{
			for( IDPTR VIDPtr = pAlienValveTb->GetFirst(); _T('\0') != *VIDPtr.ID; VIDPtr = pAlienValveTb->GetNext( VIDPtr.MP ) )
			{
				CDB_AlienRegValve *pALIENp = dynamic_cast<CDB_AlienRegValve *>( VIDPtr.MP );

				if( NULL == pALIENp )
				{
					continue;
				}

				if( SelFamID != pALIENp->GetFamIDPtr().ID )
				{
					continue;
				}

				if( false == pALIENp->IsAvailable() )
				{
					continue;
				}

				return true;
			}
		}
	}

	return false;
}


#ifndef TACBX
int CTADatabase::GetDpCDpStabList( CRankEx *pList )
{
	CString str = TASApp.LoadLocalizedString( IDS_BRANCH );
	pList->Add( ( LPCTSTR )str, 0 );
	str = TASApp.LoadLocalizedString( IDS_CONTROLVALVE );
	pList->Add( ( LPCTSTR )str, 1 );
	return pList->GetCount();
}

int CTADatabase::GetDpCMvLocList( CRankEx *pList )
{
	CString str = TASApp.LoadLocalizedString( IDS_PRIMARY );
	pList->Add( ( LPCTSTR )str, 0 );
	str = TASApp.LoadLocalizedString( IDS_SECONDARY );
	pList->Add( ( LPCTSTR )str, 1 );
	return pList->GetCount();
}

int CTADatabase::GetDpCLocList( CRankEx *pList, FilterSelection eFilterSelection )
{
	CString str = TASApp.LoadLocalizedString( IDS_DPCLOCDOWNSTREAM );
	pList->Add( ( LPCTSTR )str, 0 );

	// Up stream.
	// Try to find at least one DpC available for this type.
	eDpCLoc DpCLoc = DpCLocUpStream;
	CTable *pTab = static_cast<CTable *>( Get( _T("DPCONTR_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( IDPtr.MP );

		if( NULL == pDpC )
		{
			continue;
		}

		if( false == pDpC->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pDpC, eFilterSelection ) )
		{
			continue;
		}

		if( DpCLoc != pDpC->GetDpCLoc() )
		{
			continue;
		}

		str = TASApp.LoadLocalizedString( IDS_DPCLOCUPSTREAM );
		pList->Add( ( LPCTSTR )str, 1 );
		break;	// One found is enough
	}

	return pList->GetCount();
}
#endif

int CTADatabase::GetDpCTypeList( CRankEx *pList, FilterSelection eFilterSelection, eDpCLoc DpCLoc, int iDNMin, int iDNMax )
{
	std::vector<LPARAM> array;
	CTable *pTab = static_cast<CTable *>( Get( _T("DPCTYPE_TAB") ).MP );
	ASSERT( NULL != pTab );

	array.resize( pTab->GetItemCount() );
	unsigned i = 0;

	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		ASSERT( i < array.size() );
		array.at( i ) = (LPARAM)IDPtr.MP;
		i++;
	}

	// Remove unused elements.
	array.resize( i );

	// Try to find at least one DpC available for this type.
	pTab = static_cast<CTable *>( Get( _T("DPCONTR_TAB") ).MP );
	ASSERT( NULL != pTab );

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	for( i = 0; i < array.size(); )
	{
		// Take Type ID from stored MP.
		_string strType = ( (CDB_StringID *)array.at( i ) )->GetIDPtr().ID;
		bool bSelectable = false;

		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( IDPtr.MP );

			if( NULL == pDpC )
			{
				continue;
			}

			if( false == pDpC->IsSelectable( true, false, bOldProductsAvailableForHC ) )
			{
				continue;
			}

			if( true == MustExclude( pDpC, eFilterSelection ) )
			{
				continue;
			}

			if( DpCLoc != DpCLocNone && DpCLoc != pDpC->GetDpCLoc() )
			{
				continue;
			}

			// Scan all DpC with the same type.
			if( _tcscmp( ( ( CDB_TAProduct * )IDPtr.MP )->GetTypeIDPtr().ID, strType.c_str() ) )
			{
				continue;
			}

			// Test DN.
			int iDN = pDpC->GetSizeKey();

			if( iDN < iDNMin )
			{
				continue;
			}

			if( iDN > iDNMax )
			{
				continue;
			}

			bSelectable = true;

			// Loop aborted one DpC of this type is enough to display the type.
			break;
		}

		if( false == bSelectable )
		{
			array.erase( array.begin() + i );
		}
		else
		{
			i++;
		}
	}

	// Transfer and sort list.
	for( i = 0; i < array.size(); i++ )
	{
		pList->Add( ( (CDB_StringID *)array.at( i ) )->GetString(), _ttoi( ( (CDB_StringID *)array.at( i ) )->GetIDstr() ) - 1, array.at( i ) );
	}

	i = array.size();
	array.clear();
	return i;
}

int CTADatabase::GetDpCFamilyList( CRankEx *pList, eDpCLoc DpCLoc, _string strSelTypeID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("DPCONTR_TAB") ).MP );
	ASSERT( NULL != pTab );

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Find family according selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( IDPtr.MP );

		if( NULL == pDpC )
		{
			continue;
		}

		if( false == pDpC->IsSelectable( true, false, bOldProductsAvailableForHC ) )
		{
			continue;
		}

		if( true == MustExclude( pDpC, eFilterSelection ) )
		{
			continue;
		}

		if( DpCLoc != DpCLocNone && DpCLoc != pDpC->GetDpCLoc() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pDpC->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pDpC->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pDpC->GetFamilyIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pDpC->GetFamilyIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pDpC->GetFamilyIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetDpCBdyMatList( CRankEx *pList, eDpCLoc DpCLoc, _string strSelTypeID, _string strSelFamID, FilterSelection eFilterSelection, int iDNMin,
								   int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("DPCONTR_TAB") ).MP );
	ASSERT( NULL != pTab );
	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Find family according selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( IDPtr.MP );

		if( NULL == pDpC )
		{
			continue;
		}

		if( false == pDpC->IsSelectable( true, false, bOldProductsAvailableForHC ) )
		{
			continue;
		}

		if( true == MustExclude( pDpC, eFilterSelection ) )
		{
			continue;
		}

		if( DpCLoc != DpCLocNone && DpCLoc != pDpC->GetDpCLoc() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pDpC->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pDpC->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Bdy | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pDpC->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pDpC->GetBodyMaterialIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pDpC->GetBodyMaterialIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pDpC->GetBodyMaterialIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetDpCConnList( CRankEx *pList, eDpCLoc DpCLoc, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID,
								 FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("DPCONTR_TAB") ).MP );
	ASSERT( NULL != pTab );
	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Fill the connection combo according to selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( IDPtr.MP );

		if( NULL == pDpC )
		{
			continue;
		}

		if( false == pDpC->IsSelectable( true, false, bOldProductsAvailableForHC ) )
		{
			continue;
		}

		if( true == MustExclude( pDpC, eFilterSelection ) )
		{
			continue;
		}

		if( DpCLoc != DpCLocNone && DpCLoc != pDpC->GetDpCLoc() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pDpC->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pDpC->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pDpC->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		// switch between product with a fixed connection and product with a mountable connection (as DA516)
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Connect | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pDpC->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pDpC->GetConnectIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pDpC->GetConnectIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pDpC->GetConnectIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetDpCVersList( CRankEx *pList, eDpCLoc DpCLoc, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID, _string strSelConnectID,
								 FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("DPCONTR_TAB") ).MP );
	ASSERT( NULL != pTab );
	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Loop on selectable DpC.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( IDPtr.MP );

		if( NULL == pDpC )
		{
			continue;
		}

		if( false == pDpC->IsSelectable( true, false, bOldProductsAvailableForHC ) )
		{
			continue;
		}

		if( true == MustExclude( pDpC, eFilterSelection ) )
		{
			continue;
		}

		if( DpCLoc != DpCLocNone && DpCLoc != pDpC->GetDpCLoc() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pDpC->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pDpC->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		if( false == strSelConnectID.empty() && strSelConnectID != pDpC->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pDpC->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Version | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pDpC->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pDpC->GetVersionIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pDpC->GetVersionIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pDpC->GetVersionIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetDpCPNList( CRankEx *pList, eDpCLoc DpCLoc, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID, _string strSelConnID,
		_string strSelVersionID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	// Fill the PN combo according to selected type and connection.
	double dPN, dDP, dPmax = 0.0;
	CTable *pTab = static_cast<CTable *>( Get( _T("DPCONTR_TAB") ).MP );
	ASSERT( NULL != pTab );
	CTable	*pPNTab = static_cast<CTable *>( Get( _T("PN_TAB") ).MP );
	ASSERT( NULL != pPNTab );
	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Loop on selectable DpC.
	// Those with Pmax = Pmax of previous item are not treated.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( IDPtr.MP );

		if( NULL == pDpC )
		{
			continue;
		}

		if( false == pDpC->IsSelectable( true, false, bOldProductsAvailableForHC ) )
		{
			continue;
		}

		if( true == MustExclude( pDpC, eFilterSelection ) )
		{
			continue;
		}

		if( DpCLoc != DpCLocNone && DpCLoc != pDpC->GetDpCLoc() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pDpC->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pDpC->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		if( false == strSelConnID.empty() && strSelConnID != pDpC->GetConnectIDPtr().ID )
		{
			continue;
		}

		if( false == strSelVersionID.empty() && strSelVersionID != pDpC->GetVersionIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pDpC->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		if( dPmax != ( ( CDB_TAProduct * )IDPtr.MP )->GetPmaxmax() )
		{
			dPmax = ( ( CDB_TAProduct * )IDPtr.MP )->GetPmaxmax();

			// Find the largest PN lower than Pmax.
			dDP = DBL_MAX;
			IDPTR PNIDPtrFnd;

			for( IDPTR PNIDPtr = pPNTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *PNIDPtr.ID; PNIDPtr = pPNTab->GetNext() )
			{
				if( RD_OK != ReadDoubleFromStr( ( TCHAR * )( (CDB_StringID *)PNIDPtr.MP )->GetIDstr(), &dPN ) )
				{
					ASSERTA_RETURN( -1 );
				}

				if( ( dPmax - dPN ) >= 0.0 && fabs( dPmax - dPN ) < dDP )
				{
					dDP = fabs( dPmax - dPN );
					PNIDPtrFnd = PNIDPtr;
				}
			}

			// Found product having priority.
			CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_PN | CTAPSortKey::TAPSKM_PriorL );
			double dKey = (double)pDpC->GetSortingKey( &clCTAPSKMask );

			// Already added ?
			_string InListstr = _T( "" );
			LPARAM InListlparam = (LPARAM)PNIDPtrFnd.MP;
			double dInListkey = 0;

			if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
			{
				// Yes take a copy, check priority key, and keep only the most important.
				if( dKey < dInListkey )
				{
					pList->Delete( dInListkey );
				}
				else
				{
					continue;
				}
			}

			// Insert the found PN in the list
			pList->Add( ( (CDB_StringID *)PNIDPtrFnd.MP )->GetString(), dKey, (LPARAM)PNIDPtrFnd.MP );
		}
	}

	return pList->GetCount();
}

int CTADatabase::GetDpCList( CRankEx *pList, eDpCLoc DpCLoc, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID,	_string strSelConnID,
		_string strSelVersID, _string strSelPNID, FilterSelection eFilterSelection, int iDNMin, int iDNMax, CData *pProd )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("DPCONTR_TAB") ).MP );
	ASSERT( NULL != pTab );

	// For increasing 'if' test below.
	CDB_StringID *pSelTypeID = NULL;
	CDB_StringID *pSelFamilyID = NULL;
	CDB_StringID *pSelBdyMatID = NULL;
	CDB_StringID *pSelConnectID = NULL;
	CDB_StringID *pSelVersionID = NULL;
	CDB_StringID *pSelDpCPNID = NULL;

	if( false == strSelTypeID.empty() )
	{
		pSelTypeID = ( (CDB_StringID *)Get( strSelTypeID.c_str() ).MP );
	}

	if( false == strSelFamID.empty() )
	{
		pSelFamilyID = ( (CDB_StringID *)Get( strSelFamID.c_str() ).MP );
	}

	if( false == strSelBdyMatID.empty() )
	{
		pSelBdyMatID = ( (CDB_StringID *)Get( strSelBdyMatID.c_str() ).MP );
	}

	if( false == strSelConnID.empty() )
	{
		pSelConnectID = ( (CDB_StringID *)Get( strSelConnID.c_str() ).MP );
	}

	if( false == strSelVersID.empty() )
	{
		pSelVersionID = ( (CDB_StringID *)Get( strSelVersID.c_str() ).MP );
	}

	if( false == strSelPNID.empty() )
	{
		pSelDpCPNID = ( (CDB_StringID *)Get( strSelPNID.c_str() ).MP );
	}
	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Loop on selectable DpC valves.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		// Keep only CDB_TAProduct and children.
		CDB_DpController *pDpC = dynamic_cast<CDB_DpController *>( IDPtr.MP );

		if( NULL == pDpC )
		{
			continue;
		}

		if( false == pDpC->IsSelectable( true, false, bOldProductsAvailableForHC ) )
		{
			continue;
		}

		if( true == MustExclude( pDpC, eFilterSelection ) )
		{
			continue;
		}

		if( DpCLoc != DpCLocNone && DpCLoc != pDpC->GetDpCLoc() )
		{
			continue;
		}

		if( NULL != pSelTypeID && pSelTypeID != ( (CDB_StringID *)pDpC->GetTypeIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelFamilyID && pSelFamilyID != ( (CDB_StringID *)pDpC->GetFamilyIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelBdyMatID && pSelBdyMatID != ( (CDB_StringID *)pDpC->GetBodyMaterialIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelConnectID && pSelConnectID != ( (CDB_StringID *)pDpC->GetConnectIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelVersionID && pSelVersionID != ( (CDB_StringID *)pDpC->GetVersionIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelDpCPNID )
		{
			if( false == pDpC->IsPNIncludedInPNRange( strSelPNID ) )
			{
				continue;
			}
		}

		// Test DN.
		int iDN = pDpC->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey = (double)pDpC->GetSortingKey();
		bool bCouldbeAdded = true;			// Could be added by default

		//  store Valve characteristic ID.
		CData *pDpCChar = pDpC->GetValveCharDataPointer();

		if( NULL != pDpCChar )
		{
			#ifdef TACBX
			bCouldbeAdded = CheckIfCharactAlreadyExist( pList, pDpC, pProd );
			#endif

			if( true == bCouldbeAdded )
			{
				pList->Add( pDpCChar->GetIDPtr().ID, dKey, (LPARAM)IDPtr.MP );
			}
		}
	}

	return pList->GetCount();
}

int CTADatabase::GetTrvList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelConnID, _string strSelVersID, _string strSelPNID,
							 FilterSelection eFilterSelection, int iDNMin, int iDNMax, CData *pProd )
{
	// By default thermostatic valves are defined in 'TRVALV_TAB'.
	CTable *pTab = static_cast<CTable *>( Get( _T("TRVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	// First verify if 'SelTypeID' is defined.
	if( false == strSelTypeID.empty() )
	{
		// If defined, check if it's well a 'TRVTYPE'.
		CTable *pTypeTable = static_cast<CTable *>( Get( _T("TRVTYPE_TAB") ).MP );

		if( NULL != pTypeTable )
		{
			CDB_StringID *pType = (CDB_StringID *)( pTypeTable->Get( strSelTypeID.c_str() ).MP );

			// If type is well a 'TRVTYPE'...
			if( NULL != pType )
			{
				// Check in which table valves of this type are defined.
				// Remark: it's a special case for example 'TRVTYPE_INSERT_PRESET' where valves are defined in 'TRINSERT_TAB' instead of
				//         'TRVALV_TAB'.
				if( NULL != static_cast<CTable *>( Get( pType->GetIDstr2() ).MP ) )
				{
					pTab = static_cast<CTable *>( Get( pType->GetIDstr2() ).MP );
				}
			}
		}
	}

	// For increasing 'if' test below
	CDB_StringID *pSelTypeID = NULL;
	CDB_StringID *pSelFamilyID = NULL;
	CDB_StringID *pSelConnectID = NULL;
	CDB_StringID *pSelVersionID = NULL;
	CDB_StringID *pSelPNID = NULL;

	if( false == strSelTypeID.empty() )
	{
		pSelTypeID = ( (CDB_StringID *)Get( strSelTypeID.c_str() ).MP );
	}

	if( false == strSelFamID.empty() )
	{
		pSelFamilyID = ( (CDB_StringID *)Get( strSelFamID.c_str() ).MP );
	}

	if( false == strSelConnID.empty() )
	{
		pSelConnectID = ( (CDB_StringID *)Get( strSelConnID.c_str() ).MP );
	}

	if( false == strSelVersID.empty() )
	{
		pSelVersionID = ( (CDB_StringID *)Get( strSelVersID.c_str() ).MP );
	}

	if( false == strSelPNID.empty() )
	{
		pSelPNID = ( (CDB_StringID *)Get( strSelPNID.c_str() ).MP );
	}

	// Loop on selectable Trv.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		// Keep only CDB_TAProduct and children.
		CDB_ThermostaticValve *pTrv = dynamic_cast<CDB_ThermostaticValve *>( IDPtr.MP );

		if( NULL == pTrv )
		{
			continue;
		}

		if( false == pTrv->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTrv, eFilterSelection ) )
		{
			continue;
		}

		if( NULL != pSelTypeID && pSelTypeID != ( (CDB_StringID *)pTrv->GetTypeIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelFamilyID && pSelFamilyID != ( (CDB_StringID *)pTrv->GetFamilyIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelConnectID && pSelConnectID != ( (CDB_StringID *)pTrv->GetConnectIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelVersionID && pSelVersionID != ( (CDB_StringID *)pTrv->GetVersionIDPtr().MP ) )
		{
			continue;
		}

		if( NULL !=  pSelPNID )
		{
			if( false == pTrv->IsPNIncludedInPNRange( pSelPNID->GetIDstr() ) )
			{
				continue;
			}
		}

		// Test DN
		int iDN = pTrv->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey = (double)pTrv->GetSortingKey();

		// Could be added by default.
		bool bCouldbeAdded = true;
		
		// Store Valve characteristic ID.
		CData *pTrvChar = pTrv->GetValveCharDataPointer();

		if( NULL != pTrvChar )
		{
			#ifdef TACBX
			bCouldbeAdded = CheckIfCharactAlreadyExist( pList, pTrv, pProd );
			#endif

			if( true == bCouldbeAdded )
			{
				pList->Add( pTrvChar->GetIDPtr().ID, dKey, (LPARAM)IDPtr.MP );
			}
		}
	}

	return pList->GetCount();
}

int CTADatabase::GetTrvFamilyList( CRankEx *pList, _string strIncludeOnlyTypeID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("TRVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Find family according selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_ThermostaticValve *pThermostaticValve = dynamic_cast<CDB_ThermostaticValve *>( IDPtr.MP );

		if( NULL == pThermostaticValve )
		{
			continue;
		}

		if( false == pThermostaticValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pThermostaticValve, eFilterSelection ) )
		{
			continue;
		}

		// Check type if defined.
		CDB_StringID *pFamilyID = (CDB_StringID *)( pThermostaticValve->GetFamilyIDPtr().MP );

		if( false == strIncludeOnlyTypeID.empty() )
		{
			if( 0 != IDcmp( strIncludeOnlyTypeID.c_str(), pFamilyID->GetIDstr2() ) )
			{
				continue;
			}
		}

		// Test DN.
		int iDN = pThermostaticValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pThermostaticValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)pFamilyID;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( pFamilyID->GetString(), dKey, InListlparam );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetTrvFamilyList( CRankEx *pSrcList, CRankEx *pDestList )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the separators of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_ThermostaticValve *pclThermostaticValve = dynamic_cast<CDB_ThermostaticValve*>( (CData *)lParam );
		ASSERT( NULL != pclThermostaticValve );

		if( NULL == pclThermostaticValve )
		{
			continue;
		}

		// Add the new family ID if doesn't already exist.
		IDPTR FamilyIDPtr = pclThermostaticValve->GetFamilyIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( FamilyIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)FamilyIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)FamilyIDPtr.MP )->GetString(), dKey, (LPARAM)FamilyIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetTrvTypeList( CRankEx *pList, _string strSelFamilyID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("TRVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Fill the type combo according to selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_ThermostaticValve *pThermostaticValve = dynamic_cast<CDB_ThermostaticValve *>( IDPtr.MP );

		if( NULL == pThermostaticValve )
		{
			continue;
		}

		if( false == pThermostaticValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pThermostaticValve, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelFamilyID.empty() && strSelFamilyID != pThermostaticValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pThermostaticValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Type | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pThermostaticValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pThermostaticValve->GetTypeIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pThermostaticValve->GetTypeIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pThermostaticValve->GetTypeIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetTrvConnectList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("TRVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Fill the connection combo according to selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_ThermostaticValve *pThermostaticValve = dynamic_cast<CDB_ThermostaticValve *>( IDPtr.MP );

		if( NULL == pThermostaticValve )
		{
			continue;
		}

		if( false == pThermostaticValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pThermostaticValve, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pThermostaticValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pThermostaticValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pThermostaticValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority switch between product with a fixed connection and product with a mountable connection (as DA516).
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Connect | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pThermostaticValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pThermostaticValve->GetConnectIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pThermostaticValve->GetConnectIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pThermostaticValve->GetConnectIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetTrvConnectList( CRankEx *pSrcList, CRankEx *pDestList, _string strFamilyID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the separators of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_ThermostaticValve *pclThermostaticValve = dynamic_cast<CDB_ThermostaticValve*>( (CData *)lParam );
		ASSERT( NULL != pclThermostaticValve );

		if( NULL == pclThermostaticValve )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclThermostaticValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Add the new connection ID if doesn't already exist.
		IDPTR ConnectIDPtr = pclThermostaticValve->GetConnectIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( ConnectIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)ConnectIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)ConnectIDPtr.MP )->GetString(), dKey, (LPARAM)ConnectIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetTrvVersList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelConnectID, FilterSelection eFilterSelection,
		int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("TRVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Loop on selectable thermostatic valve.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_ThermostaticValve *pThermostaticValve = dynamic_cast<CDB_ThermostaticValve *>( IDPtr.MP );

		if( NULL == pThermostaticValve )
		{
			continue;
		}

		if( false == pThermostaticValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pThermostaticValve, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pThermostaticValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pThermostaticValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelConnectID.empty() && strSelConnectID != pThermostaticValve->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pThermostaticValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Version | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pThermostaticValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pThermostaticValve->GetVersionIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pThermostaticValve->GetVersionIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pThermostaticValve->GetVersionIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetTrvVersList( CRankEx *pSrcList, CRankEx *pDestList, _string strFamilyID, _string strConnectID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the separators of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_ThermostaticValve *pclThermostaticValve = dynamic_cast<CDB_ThermostaticValve*>( (CData *)lParam );
		ASSERT( NULL != pclThermostaticValve );

		if( NULL == pclThermostaticValve )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclThermostaticValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test connection ID.
		if( false == strConnectID.empty() && strConnectID != pclThermostaticValve->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Add the new version ID if doesn't already exist.
		IDPTR VersionIDPtr = pclThermostaticValve->GetVersionIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( VersionIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)VersionIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)VersionIDPtr.MP )->GetString(), dKey, (LPARAM)VersionIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetReturnValveFamilyList( CRankEx *pList, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	return GetBVFamilyList( pList, _T("RVTYPE_RV"), eFilterSelection, iDNMin, iDNMax );
}

int CTADatabase::GetReturnValveConnectList( CRankEx *pList, _string strSelFamID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	return GetBVConnList( pList, _T("RVTYPE_RV" ), strSelFamID, _T( ""), eFilterSelection, iDNMin, iDNMax );
}

int CTADatabase::GetReturnValveVersList( CRankEx *pList, _string strSelFamID, _string strSelConnectID, FilterSelection eFilterSelection, int iDNMin,
		int iDNMax )
{
	return GetBVVersList( pList, _T("RVTYPE_RV" ), strSelFamID, _T( ""), strSelConnectID, eFilterSelection, iDNMin, iDNMax );
}

int CTADatabase::GetTrvInsertList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelConnID, _string strSelVersID, _string strSelPNID,
		FilterSelection eFilterSelection, int iDNMin, int iDNMax, CData *pProd )
{
	// By default thermostatic valves are defined in 'TRVALV_TAB'.
	CTable *pTab = static_cast<CTable *>( Get( _T("TRINSERT_TAB") ).MP );
	ASSERT( NULL != pTab );

	// For increasing 'if' test below
	CDB_StringID *pSelTypeID = NULL;
	CDB_StringID *pSelFamilyID = NULL;
	CDB_StringID *pSelConnectID = NULL;
	CDB_StringID *pSelVersionID = NULL;
	CDB_StringID *pSelPNID = NULL;

	if( false == strSelTypeID.empty() )
	{
		pSelTypeID = ( (CDB_StringID *)Get( strSelTypeID.c_str() ).MP );
	}

	if( false == strSelFamID.empty() )
	{
		pSelFamilyID = ( (CDB_StringID *)Get( strSelFamID.c_str() ).MP );
	}

	if( false == strSelConnID.empty() )
	{
		pSelConnectID = ( (CDB_StringID *)Get( strSelConnID.c_str() ).MP );
	}

	if( false == strSelVersID.empty() )
	{
		pSelVersionID = ( (CDB_StringID *)Get( strSelVersID.c_str() ).MP );
	}

	if( false == strSelPNID.empty() )
	{
		pSelPNID = ( (CDB_StringID *)Get( strSelPNID.c_str() ).MP );
	}

	// Loop on selectable Trv insert.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		// Keep only CDB_TAProduct and children.
		CDB_ThermostaticValve *pTrv = dynamic_cast<CDB_ThermostaticValve *>( IDPtr.MP );

		if( NULL == pTrv )
		{
			continue;
		}

		if( false == pTrv->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTrv, eFilterSelection ) )
		{
			continue;
		}

		if( NULL != pSelTypeID && pSelTypeID != ( (CDB_StringID *)pTrv->GetTypeIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelFamilyID && pSelFamilyID != ( (CDB_StringID *)pTrv->GetFamilyIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelConnectID && pSelConnectID != ( (CDB_StringID *)pTrv->GetConnectIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelVersionID && pSelVersionID != ( (CDB_StringID *)pTrv->GetVersionIDPtr().MP ) )
		{
			continue;
		}

		if( NULL !=  pSelPNID )
		{
			if( false == pTrv->IsPNIncludedInPNRange( pSelPNID->GetIDstr() ) )
			{
				continue;
			}
		}

		// Test DN
		int iDN = pTrv->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey = (double)pTrv->GetSortingKey();
		
		// Could be added by default.
		bool bCouldbeAdded = true;
		
		// Store Valve characteristic ID.
		CData *pTrvChar = pTrv->GetValveCharDataPointer();

		if( NULL != pTrvChar )
		{
			#ifdef TACBX
			bCouldbeAdded = CheckIfCharactAlreadyExist( pList, pTrv, pProd );
			#endif

			if( true == bCouldbeAdded )
			{
				pList->Add( pTrvChar->GetIDPtr().ID, dKey, (LPARAM)IDPtr.MP );
			}
		}
	}

	return pList->GetCount();
}

int CTADatabase::GetTrvInsertFamilyList( CRankEx *pList, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("TRINSERT_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Find family according selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_ThermostaticValve *pThermostaticValve = dynamic_cast<CDB_ThermostaticValve *>( IDPtr.MP );

		if( NULL == pThermostaticValve )
		{
			continue;
		}

		if( false == pThermostaticValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pThermostaticValve, eFilterSelection ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pThermostaticValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pThermostaticValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pThermostaticValve->GetFamilyIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pThermostaticValve->GetFamilyIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pThermostaticValve->GetFamilyIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetTrvInsertTypeList( CRankEx *pList, _string strSelFamilyID, CDB_ControlProperties::eCVFUNC eCVFunc, FilterSelection eFilterSelection, 
	int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("TRINSERT_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Fill the type combo according to selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_ThermostaticValve *pThermostaticValve = dynamic_cast<CDB_ThermostaticValve *>( IDPtr.MP );

		if( NULL == pThermostaticValve )
		{
			continue;
		}

		if( false == pThermostaticValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pThermostaticValve, eFilterSelection ) )
		{
			continue;
		}

		// HYS-1467 : Add CVFunction to differentiate presettable and control only
		if( CDB_ControlProperties::LastCVFUNC != eCVFunc && eCVFunc != pThermostaticValve->GetCtrlProp()->GetCvFunc() )
		{
			continue;
		}

		if( false == strSelFamilyID.empty() && strSelFamilyID != pThermostaticValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pThermostaticValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Type | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pThermostaticValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pThermostaticValve->GetTypeIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pThermostaticValve->GetTypeIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pThermostaticValve->GetTypeIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetFLCVInsertList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelConnID, _string strSelVersID, _string strSelPNID,
	FilterSelection eFilterSelection, int iDNMin, int iDNMax, CData *pProd )
{
	CTable *pTab = static_cast<CTable *>( Get( _T( "TRINSERT_TAB" ) ).MP );
	ASSERT( NULL != pTab );

	// For increasing 'if' test below
	CDB_StringID *pSelTypeID = NULL;
	CDB_StringID *pSelFamilyID = NULL;
	CDB_StringID *pSelConnectID = NULL;
	CDB_StringID *pSelVersionID = NULL;
	CDB_StringID *pSelPNID = NULL;

	if( false == strSelTypeID.empty() )
	{
		pSelTypeID = ( (CDB_StringID *)Get( strSelTypeID.c_str() ).MP );
	}

	if( false == strSelFamID.empty() )
	{
		pSelFamilyID = ( (CDB_StringID *)Get( strSelFamID.c_str() ).MP );
	}

	if( false == strSelConnID.empty() )
	{
		pSelConnectID = ( (CDB_StringID *)Get( strSelConnID.c_str() ).MP );
	}

	if( false == strSelVersID.empty() )
	{
		pSelVersionID = ( (CDB_StringID *)Get( strSelVersID.c_str() ).MP );
	}

	if( false == strSelPNID.empty() )
	{
		pSelPNID = ( (CDB_StringID *)Get( strSelPNID.c_str() ).MP );
	}

	// Loop on selectable Trv insert.
	for( IDPTR IDPtr = pTab->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		// Keep only CDB_TAProduct and children.
		CDB_FlowLimitedControlValve *pFLC = dynamic_cast<CDB_FlowLimitedControlValve *>( IDPtr.MP );

		if( NULL == pFLC )
		{
			continue;
		}

		if( false == pFLC->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pFLC, eFilterSelection ) )
		{
			continue;
		}

		if( NULL != pSelTypeID && pSelTypeID != ( (CDB_StringID *)pFLC->GetTypeIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelFamilyID && pSelFamilyID != ( (CDB_StringID *)pFLC->GetFamilyIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelConnectID && pSelConnectID != ( (CDB_StringID *)pFLC->GetConnectIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelVersionID && pSelVersionID != ( (CDB_StringID *)pFLC->GetVersionIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelPNID )
		{
			if( false == pFLC->IsPNIncludedInPNRange( pSelPNID->GetIDstr() ) )
			{
				continue;
			}
		}

		// Test DN
		int iDN = pFLC->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey = (double)pFLC->GetSortingKey();

		// Could be added by default.
		bool bCouldbeAdded = true;

		// Store Valve characteristic ID.
		CData *pFLCChar = pFLC->GetValveCharDataPointer();

		if( NULL != pFLCChar )
		{
#ifdef TACBX
			bCouldbeAdded = CheckIfCharactAlreadyExist( pList, pFLC, pProd );
#endif

			if( true == bCouldbeAdded )
			{
				pList->Add( pFLCChar->GetIDPtr().ID, dKey, (LPARAM)IDPtr.MP );
			}
		}
	}

	return pList->GetCount();
}

int CTADatabase::GetFLCVInsertFamilyList( CRankEx *pList, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T( "TRINSERT_TAB" ) ).MP );
	ASSERT( NULL != pTab );

	// Find family according selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_FlowLimitedControlValve *pFlowLimitedControlValve = dynamic_cast<CDB_FlowLimitedControlValve *>( IDPtr.MP );

		if( NULL == pFlowLimitedControlValve )
		{
			continue;
		}

		if( false == pFlowLimitedControlValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pFlowLimitedControlValve, eFilterSelection ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pFlowLimitedControlValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pFlowLimitedControlValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pFlowLimitedControlValve->GetFamilyIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pFlowLimitedControlValve->GetFamilyIDPtr().MP )->GetString(), dKey,
			(LPARAM)(void *)pFlowLimitedControlValve->GetFamilyIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetFLCVInsertTypeList( CRankEx *pList, _string strSelFamilyID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T( "TRINSERT_TAB" ) ).MP );
	ASSERT( NULL != pTab );

	// Fill the type combo according to selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_FlowLimitedControlValve *pFlowLimitedControlValve = dynamic_cast<CDB_FlowLimitedControlValve *>( IDPtr.MP );

		if( NULL == pFlowLimitedControlValve )
		{
			continue;
		}

		if( false == pFlowLimitedControlValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pFlowLimitedControlValve, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelFamilyID.empty() && strSelFamilyID != pFlowLimitedControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pFlowLimitedControlValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Type | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pFlowLimitedControlValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pFlowLimitedControlValve->GetTypeIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pFlowLimitedControlValve->GetTypeIDPtr().MP )->GetString(), dKey,
			(LPARAM)(void *)pFlowLimitedControlValve->GetTypeIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetRVInsertFamilyList( CRankEx *pList, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("RVINSERT_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Find family according selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_RegulatingValve *pReturnValve = dynamic_cast<CDB_RegulatingValve *>( IDPtr.MP );

		if( NULL == pReturnValve )
		{
			continue;
		}

		if( false == pReturnValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pReturnValve, eFilterSelection ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pReturnValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pReturnValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pReturnValve->GetFamilyIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pReturnValve->GetFamilyIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pReturnValve->GetFamilyIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetFLCVList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelConnID, _string strSelVersID, _string strSelPNID,
							  FilterSelection eFilterSelection, int iDNMin, int iDNMax, CData *pProd )
{
	// By default flow limited control valves are defined in 'FLCTRLVALV_TAB'.
	CTable *pTab = static_cast<CTable *>( Get( _T("FLCTRLVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	// For increasing 'if' test below.
	CDB_StringID *pSelTypeID = NULL;
	CDB_StringID *pSelFamilyID = NULL;
	CDB_StringID *pSelConnectID = NULL;
	CDB_StringID *pSelVersionID = NULL;
	CDB_StringID *pSelPNID = NULL;

	if( false == strSelTypeID.empty() )
	{
		pSelTypeID = ( (CDB_StringID *)Get( strSelTypeID.c_str() ).MP );
	}

	if( false == strSelFamID.empty() )
	{
		pSelFamilyID = ( (CDB_StringID *)Get( strSelFamID.c_str() ).MP );
	}

	if( false == strSelConnID.empty() )
	{
		pSelConnectID = ( (CDB_StringID *)Get( strSelConnID.c_str() ).MP );
	}

	if( false == strSelVersID.empty() )
	{
		pSelVersionID = ( (CDB_StringID *)Get( strSelVersID.c_str() ).MP );
	}

	if( false == strSelPNID.empty() )
	{
		pSelPNID = ( (CDB_StringID *)Get( strSelPNID.c_str() ).MP );
	}

	// Loop on selectable flow limited control valves.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		// Keep only CDB_TAProduct and children.
		CDB_FlowLimitedControlValve *pFlowLimitedControlValve = dynamic_cast<CDB_FlowLimitedControlValve *>( IDPtr.MP );

		if( NULL == pFlowLimitedControlValve )
		{
			continue;
		}

		if( false == pFlowLimitedControlValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pFlowLimitedControlValve, eFilterSelection ) )
		{
			continue;
		}

		if( NULL != pSelTypeID && pSelTypeID != ( (CDB_StringID *)pFlowLimitedControlValve->GetTypeIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelFamilyID && pSelFamilyID != ( (CDB_StringID *)pFlowLimitedControlValve->GetFamilyIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelConnectID && pSelConnectID != ( (CDB_StringID *)pFlowLimitedControlValve->GetConnectIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelVersionID && pSelVersionID != ( (CDB_StringID *)pFlowLimitedControlValve->GetVersionIDPtr().MP ) )
		{
			continue;
		}

		if( NULL !=  pSelPNID )
		{
			if( false == pFlowLimitedControlValve->IsPNIncludedInPNRange( pSelPNID->GetIDstr() ) )
			{
				continue;
			}
		}

		// Test DN.
		int iDN = pFlowLimitedControlValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey = (double)pFlowLimitedControlValve->GetSortingKey();
		
		// Could be added by default.
		bool bCouldbeAdded = true;
		
		// Store valve characteristic ID.
		CData *pFLCVChar = pFlowLimitedControlValve->GetValveCharDataPointer();

		if( NULL != pFLCVChar )
		{
			#ifdef TACBX
			bCouldbeAdded = CheckIfCharactAlreadyExist( pList, pFlowLimitedControlValve, pProd );
			#endif

			if( true == bCouldbeAdded )
			{
				pList->Add( pFLCVChar->GetIDPtr().ID, dKey, (LPARAM)IDPtr.MP );
			}
		}
	}

	return pList->GetCount();
}

int CTADatabase::GetFLCVFamilyList( CRankEx *pList, _string strIncludeOnlyTypeID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("FLCTRLVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Find family according selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_FlowLimitedControlValve *pFlowLimitedControlValve = dynamic_cast<CDB_FlowLimitedControlValve *>( IDPtr.MP );

		if( NULL == pFlowLimitedControlValve )
		{
			continue;
		}

		if( false == pFlowLimitedControlValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pFlowLimitedControlValve, eFilterSelection ) )
		{
			continue;
		}

		// Check type if defined.
		CDB_StringID *pFamilyID = (CDB_StringID *)( pFlowLimitedControlValve->GetFamilyIDPtr().MP );

		if( false == strIncludeOnlyTypeID.empty() )
		{
			if( 0 != IDcmp( strIncludeOnlyTypeID.c_str(), pFamilyID->GetIDstr2() ) )
			{
				continue;
			}
		}

		// Test DN.
		int iDN = pFlowLimitedControlValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pFlowLimitedControlValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pFlowLimitedControlValve->GetFamilyIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pFlowLimitedControlValve->GetFamilyIDPtr().MP )->GetString(), dKey,
					(LPARAM)(void *)pFlowLimitedControlValve->GetFamilyIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetFLCVTypeList( CRankEx *pList, _string strSelFamilyID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("FLCTRLVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Fill the type combo according to selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_FlowLimitedControlValve *pFlowLimitedControlValve = dynamic_cast<CDB_FlowLimitedControlValve *>( IDPtr.MP );

		if( NULL == pFlowLimitedControlValve )
		{
			continue;
		}

		if( false == pFlowLimitedControlValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pFlowLimitedControlValve, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelFamilyID.empty() && strSelFamilyID != pFlowLimitedControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pFlowLimitedControlValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Type | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pFlowLimitedControlValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pFlowLimitedControlValve->GetTypeIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pFlowLimitedControlValve->GetTypeIDPtr().MP )->GetString(), dKey,
					(LPARAM)(void *)pFlowLimitedControlValve->GetTypeIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetFLCVConnectList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("FLCTRLVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Fill the connection combo according to selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_FlowLimitedControlValve *pFlowLimitedControlValve = dynamic_cast<CDB_FlowLimitedControlValve *>( IDPtr.MP );

		if( NULL == pFlowLimitedControlValve )
		{
			continue;
		}

		if( false == pFlowLimitedControlValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pFlowLimitedControlValve, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pFlowLimitedControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pFlowLimitedControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pFlowLimitedControlValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority switch between product with a fixed connection and product with a mountable connection (as DA516).
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Connect | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pFlowLimitedControlValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pFlowLimitedControlValve->GetConnectIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pFlowLimitedControlValve->GetConnectIDPtr().MP )->GetString(), dKey,
					(LPARAM)(void *)pFlowLimitedControlValve->GetConnectIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetFLCVVersList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelConnectID, FilterSelection eFilterSelection,
		int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("FLCTRLVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Loop on selectable flow limited control valve.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_FlowLimitedControlValve *pFlowLimitedControlValve = dynamic_cast<CDB_FlowLimitedControlValve *>( IDPtr.MP );

		if( NULL == pFlowLimitedControlValve )
		{
			continue;
		}

		if( false == pFlowLimitedControlValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pFlowLimitedControlValve, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pFlowLimitedControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pFlowLimitedControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelConnectID.empty() && strSelConnectID != pFlowLimitedControlValve->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pFlowLimitedControlValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Version | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pFlowLimitedControlValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pFlowLimitedControlValve->GetVersionIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pFlowLimitedControlValve->GetVersionIDPtr().MP )->GetString(), dKey,
					(LPARAM)(void *)pFlowLimitedControlValve->GetVersionIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetSeparatorTypeList( CRankEx *pList, FilterSelection eFilterSelection )
{
	std::vector<LPARAM> array;
	CTable *pTab = static_cast<CTable *>( Get( _T("SEPTYPE_TAB") ).MP );
	ASSERT( NULL != pTab );
	array.resize( pTab->GetItemCount() );
	unsigned i = 0;

	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		ASSERT( i < array.size() );
		array.at( i ) = (LPARAM)IDPtr.MP;
		i++;
	}

	// Remove unused elements.
	array.resize( i );

	// Try to find at least one separator available for each type.
	pTab = static_cast<CTable *>( Get( _T("SEPARATOR_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( i = 0; i < array.size(); )
	{
		// Take type ID from stored MP.
		_string strType = ( (CDB_StringID *)array.at( i ) )->GetIDPtr().ID;
		bool bSelectable = false;

		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator *>( IDPtr.MP );

			if( NULL == pclSeparator )
			{
				continue;
			}

			if( false == pclSeparator->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( pclSeparator, eFilterSelection ) )
			{
				continue;
			}

			// If type of the current separator is not the current one, bypass.
			if( 0 != _tcscmp( ( ( CDB_TAProduct * )IDPtr.MP )->GetTypeIDPtr().ID, strType.c_str() ) )
			{
				continue;
			}

			bSelectable = true;

			// Loop aborted because one separator of this type is enough to display the type.
			break;
		}

		if( false == bSelectable )
		{
			array.erase( array.begin() + i );
		}
		else
		{
			i++;
		}
	}

	// Transfer and sort list.
	for( i = 0; i < array.size(); i++ )
	{
		pList->Add( ( (CDB_StringID *)array.at( i ) )->GetString(), _ttoi( ( (CDB_StringID *)array.at( i ) )->GetIDstr() ) - 1, array.at( i ) );
	}

	i = array.size();
	array.clear();
	return i;
}

int CTADatabase::GetSeparatorTypeList( CRankEx *pSrcList, CRankEx *pDestList )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the separators of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator*>( (CData *)lParam );
		ASSERT( NULL != pclSeparator );

		if( NULL == pclSeparator )
		{
			continue;
		}

		// Add the new type ID if doesn't already exist.
		IDPTR TypeIDPtr = pclSeparator->GetTypeIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( TypeIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)TypeIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)TypeIDPtr.MP )->GetString(), dKey, (LPARAM)TypeIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}

int CTADatabase::GetSeparatorFamilyList( CRankEx *pList, _string strSelTypeID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("SEPARATOR_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Find family according selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator *>( IDPtr.MP );

		if( NULL == pclSeparator )
		{
			continue;
		}

		if( false == pclSeparator->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSeparator, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pclSeparator->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pclSeparator->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey = (double)pclSeparator->GetSortingKey( &CTAPSortKey::CTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL ) );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pclSeparator->GetFamilyIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pclSeparator->GetFamilyIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclSeparator->GetFamilyIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetSeparatorFamilyList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the separators of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator*>( (CData *)lParam );
		ASSERT( NULL != pclSeparator );

		if( NULL == pclSeparator )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclSeparator->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Add the new family ID if doesn't already exist.
		IDPTR FamilyIDPtr = pclSeparator->GetFamilyIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( FamilyIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)FamilyIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)FamilyIDPtr.MP )->GetString(), dKey, (LPARAM)FamilyIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}

int CTADatabase::GetSeparatorConnList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("SEPARATOR_TAB") ).MP ) ;
	ASSERT( NULL != pTab );

	// Fill the connection according to the selected type and family.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator *>( IDPtr.MP );

		if( NULL == pclSeparator )
		{
			continue;
		}

		if( false == pclSeparator->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSeparator, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pclSeparator->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pclSeparator->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pclSeparator->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		// switch between product with a fixed connection and product with a mountable connection (as DA516)
		double dKey = (double)pclSeparator->GetSortingKey( &CTAPSortKey::CTAPSKMask( CTAPSortKey::TAPSKM_Connect | CTAPSortKey::TAPSKM_PriorL ) );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pclSeparator->GetConnectIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pclSeparator->GetConnectIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclSeparator->GetConnectIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetSeparatorConnList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the separators of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator*>( (CData *)lParam );
		ASSERT( NULL != pclSeparator );

		if( NULL == pclSeparator )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclSeparator->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclSeparator->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Add the new connection ID if doesn't already exist.
		IDPTR ConnectIDPtr = pclSeparator->GetConnectIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( ConnectIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)ConnectIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)ConnectIDPtr.MP )->GetString(), dKey, (LPARAM)ConnectIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}

int CTADatabase::GetSeparatorVersList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelConnectID, FilterSelection eFilterSelection,
		int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("SEPARATOR_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Fill the version according to the selected type, family and connection.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator *>( IDPtr.MP );

		if( NULL == pclSeparator )
		{
			continue;
		}

		if( false == pclSeparator->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSeparator, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pclSeparator->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pclSeparator->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelConnectID.empty() && strSelConnectID != pclSeparator->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pclSeparator->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey = (double)pclSeparator->GetSortingKey( &CTAPSortKey::CTAPSKMask( CTAPSortKey::TAPSKM_Version | CTAPSortKey::TAPSKM_PriorL ) );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pclSeparator->GetVersionIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pclSeparator->GetVersionIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclSeparator->GetVersionIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetSeparatorVersList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, _string strConnectID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the separators of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator*>( (CData *)lParam );
		ASSERT( NULL != pclSeparator );

		if( NULL == pclSeparator )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclSeparator->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclSeparator->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test connection ID.
		if( false == strConnectID.empty() && strConnectID != pclSeparator->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Add the new version ID if doesn't already exist.
		IDPTR VersionIDPtr = pclSeparator->GetVersionIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( VersionIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)VersionIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)VersionIDPtr.MP )->GetString(), dKey, (LPARAM)VersionIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}

int CTADatabase::GetSeparatorList( CRankEx *pList, _string strSelTypeID, _string strSelFamilyID, _string strSelConnectID, _string strSelVersionID,
		FilterSelection eFilterSelection, int iDNMin, int iDNMax, CData *pProd )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("SEPARATOR_TAB") ).MP );
	ASSERT( NULL != pTab );

	// For increasing 'if' test below.
	CDB_StringID *pSelTypeID = NULL;
	CDB_StringID *pSelFamilyID = NULL;
	CDB_StringID *pSelConnectID = NULL;
	CDB_StringID *pSelVersionID = NULL;

	if( false == strSelTypeID.empty() )
	{
		pSelTypeID = ( (CDB_StringID *)Get( strSelTypeID.c_str() ).MP );
	}

	if( false == strSelFamilyID.empty() )
	{
		pSelFamilyID = ( (CDB_StringID *)Get( strSelFamilyID.c_str() ).MP );
	}

	if( false == strSelConnectID.empty() )
	{
		pSelConnectID = ( (CDB_StringID *)Get( strSelConnectID.c_str() ).MP );
	}

	if( false == strSelVersionID.empty() )
	{
		pSelVersionID = ( (CDB_StringID *)Get( strSelVersionID.c_str() ).MP );
	}

	// Loop on selectable separators.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		// Keep only CDB_TAProduct and children.
		CDB_Separator *pclSeparator = dynamic_cast<CDB_Separator *>( IDPtr.MP );

		if( NULL == pclSeparator )
		{
			continue;
		}

		if( false == pclSeparator->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSeparator, eFilterSelection ) )
		{
			continue;
		}

		if( NULL != pSelTypeID && pSelTypeID != ( (CDB_StringID *)pclSeparator->GetTypeIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelFamilyID && pSelFamilyID != ( (CDB_StringID *)pclSeparator->GetFamilyIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelConnectID && pSelConnectID != ( (CDB_StringID *)pclSeparator->GetConnectIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelVersionID && pSelVersionID != ( (CDB_StringID *)pclSeparator->GetVersionIDPtr().MP ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pclSeparator->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey = (double)pclSeparator->GetSortingKey();
		pList->Add( IDPtr.ID, dKey, (LPARAM)IDPtr.MP );
	}

	return pList->GetCount();
}
#endif

#ifndef TACBX
int CTADatabase::GetSafetyValveTypeList( CRankEx *pList, FilterSelection eFilterSelection )
{
	std::vector<LPARAM> array;
	CTable *pTab = static_cast<CTable *>( Get( _T("SFTYVTYPE_TAB") ).MP );
	ASSERT( NULL != pTab );

	array.resize( pTab->GetItemCount() );
	unsigned i = 0;

	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) ); _T( '\0' ) != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		ASSERT( i < array.size() );
		array.at( i ) = (LPARAM)IDPtr.MP;
		i++;
	}

	// Remove unused elements.
	array.resize( i );

	// Try to find at least one safety valve available for each type.
	pTab = static_cast<CTable *>( Get( _T("SAFETYVALVE_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( i = 0; i < array.size(); )
	{
		// Take type ID from stored MP.
		_string strType = ( (CDB_StringID *)array.at( i ) )->GetIDPtr().ID;
		bool bSelectable = false;

		for( IDPTR IDPtr = pTab->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase *>( IDPtr.MP );

			if( NULL == pclSafetyValve )
			{
				continue;
			}

			if( false == pclSafetyValve->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( pclSafetyValve, eFilterSelection ) )
			{
				continue;
			}

			bSelectable = true;

			// Loop aborted because one safety valve of this type is enough to display the type.
			break;
		}

		if( false == bSelectable )
		{
			array.erase( array.begin() + i );
		}
		else
		{
			i++;
		}
	}

	// Transfer and sort list.
	for( i = 0; i < array.size(); i++ )
	{
		pList->Add( ( (CDB_StringID *)array.at( i ) )->GetString(), _ttoi( ( (CDB_StringID *)array.at( i ) )->GetIDstr() ) - 1, array.at( i ) );
	}

	i = array.size();
	array.clear();
	return i;
}

int CTADatabase::GetSafetyValveTypeList( CRankEx *pSrcList, CRankEx *pDestList )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the safety valve of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase*>( (CData *)lParam );
		ASSERT( NULL != pclSafetyValve );

		if( NULL == pclSafetyValve )
		{
			continue;
		}

		// Add the new type ID if doesn't already exist.
		IDPTR TypeIDPtr = pclSafetyValve->GetTypeIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( TypeIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)TypeIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)TypeIDPtr.MP )->GetString(), dKey, (LPARAM)TypeIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}

int CTADatabase::GetSafetyValveFamilyList( CRankEx *pList, ProjectType eSystemType, FilterSelection eFilterSelection )
{
	std::vector<LPARAM> array;
	CTable *pTab = static_cast<CTable *>( Get( _T("SFTYVFAMILY_TAB") ).MP );
	ASSERT( NULL != pTab );

	array.resize( pTab->GetItemCount() );
	unsigned i = 0;

	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		ASSERT( i < array.size() );
		array.at( i ) = (LPARAM)IDPtr.MP;
		i++;
	}

	// Remove unused elements.
	array.resize( i );

	// Try to find at least one safety valve available for each type.
	pTab = static_cast<CTable *>( Get( _T("SAFETYVALVE_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( i = 0; i < array.size(); )
	{
		// Take family ID from stored MP.
		_string strFamilyID = ( (CDB_StringID *)array.at( i ) )->GetIDPtr().ID;
		bool bSelectable = false;

		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase *>( IDPtr.MP );

			if( NULL == pclSafetyValve )
			{
				continue;
			}

			if( strFamilyID != pclSafetyValve->GetFamilyID() )
			{
				continue;
			}

			if( false == pclSafetyValve->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( pclSafetyValve, eFilterSelection ) )
			{
				continue;
			}

			// If safety valve is not compatible with the project type, we bypass it.
			if( ProjectType::InvalidProjectType != eSystemType && eSystemType != pclSafetyValve->GetApplicationCompatibility() )
			{
				continue;
			}

			bSelectable = true;

			// Loop aborted because one safety valve of this family is enough to display the type.
			break;
		}

		if( false == bSelectable )
		{
			array.erase( array.begin() + i );
		}
		else
		{
			i++;
		}
	}

	// Transfer and sort list.
	for( i = 0; i < array.size(); i++ )
	{
		pList->Add( ( (CDB_StringID *)array.at( i ) )->GetString(), _ttoi( ( (CDB_StringID *)array.at( i ) )->GetIDstr() ) - 1, array.at( i ) );
	}

	i = array.size();
	array.clear();
	return i;
}

int CTADatabase::GetSafetyValveFamilyList( CRankEx *pSrcList, CRankEx *pDestList, ProjectType eSystemType )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the safety valve of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase*>( (CData *)lParam );
		ASSERT( NULL != pclSafetyValve );

		if( NULL == pclSafetyValve )
		{
			continue;
		}

		// Test project type compatibility.
		if( ProjectType::InvalidProjectType != eSystemType && eSystemType != pclSafetyValve->GetApplicationCompatibility() )
		{
			continue;
		}

		// Add the new family ID if doesn't already exist.
		IDPTR FamilyIDPtr = pclSafetyValve->GetFamilyIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( FamilyIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)FamilyIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)FamilyIDPtr.MP )->GetString(), dKey, (LPARAM)FamilyIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}

int CTADatabase::GetSafetyValveConnList( CRankEx *pList, ProjectType eSystemType, _string strSelFamilyID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("SAFETYVALVE_TAB") ).MP ) ;
	ASSERT( NULL != pTab );

	// Fill the connection according to the selected project type and safety valve type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase *>( IDPtr.MP );

		if( NULL == pclSafetyValve )
		{
			continue;
		}

		if( false == pclSafetyValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSafetyValve, eFilterSelection ) )
		{
			continue;
		}

		// Test project type compatibility.
		if( ProjectType::InvalidProjectType != eSystemType && eSystemType != pclSafetyValve->GetApplicationCompatibility() )
		{
			continue;
		}

		if( false == strSelFamilyID.empty() && strSelFamilyID != pclSafetyValve->GetFamilyID() )
		{
			continue;
		}

		// Test DN. Here we test the inlet of the safety valve, it is this part that is connected to the pipe.
		int iDN = pclSafetyValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey = (double)pclSafetyValve->GetSortingKey( &CTAPSortKey::CTAPSKMask( CTAPSortKey::TAPSKM_Connect | CTAPSortKey::TAPSKM_PriorL ) );

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)(void *)pclSafetyValve->GetConnectIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pclSafetyValve->GetConnectIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclSafetyValve->GetConnectIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetSafetyValveConnList( CRankEx *pSrcList, CRankEx *pDestList, ProjectType eSystemType, _string strSelFamilyID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the safety valve of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase*>( (CData *)lParam );
		ASSERT( NULL != pclSafetyValve );

		if( NULL == pclSafetyValve )
		{
			continue;
		}

		// Test project type compatibility.
		if( ProjectType::InvalidProjectType != eSystemType && eSystemType != pclSafetyValve->GetApplicationCompatibility() )
		{
			continue;
		}

		// Test type ID.
		if( false == strSelFamilyID.empty() && strSelFamilyID != pclSafetyValve->GetFamilyID() )
		{
			continue;
		}

		// Add the new connection ID if doesn't already exist.
		IDPTR ConnectIDPtr = pclSafetyValve->GetConnectIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( ConnectIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)ConnectIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)ConnectIDPtr.MP )->GetString(), dKey, (LPARAM)ConnectIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}

int CTADatabase::GetSafetyValveSetPressureList( CRankEx *pList, ProjectType eSystemType, _string strSelFamilyID, _string strSelConnectID, 
		FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("SAFETYVALVE_TAB") ).MP ) ;
	ASSERT( NULL != pTab );

	// Fill the set pressure list according to the selected project type, safety valve type and connection.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase *>( IDPtr.MP );

		if( NULL == pclSafetyValve )
		{
			continue;
		}

		if( false == pclSafetyValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSafetyValve, eFilterSelection ) )
		{
			continue;
		}

		// Test project type compatibility.
		if( ProjectType::InvalidProjectType != eSystemType && eSystemType != pclSafetyValve->GetApplicationCompatibility() )
		{
			continue;
		}

		if( false == strSelFamilyID.empty() && strSelFamilyID != pclSafetyValve->GetFamilyID() )
		{
			continue;
		}

		if( false == strSelConnectID.empty() && strSelConnectID != pclSafetyValve->GetConnectID() )
		{
			continue;
		}

		// Test DN. Here we test the inlet of the safety valve, it is this part that is connected to the pipe.
		int iDN = pclSafetyValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKeySI = pclSafetyValve->GetSetPressure();

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)0;

		if( false == pList->GetaCopy( InListstr, dKeySI, InListlparam ) )
		{
			// See "SetPressure - Minimum decimal.xlsx" in the "doc" folder to know how we define the number of decimal we want to display.
			int iMinDec = 0;
			CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
			UnitDesign_struct rUnitStructure = pUnitDB->GetDefaultUnit( _U_PRESSURE );

			if( rUnitStructure.Conv >= 5.0e3 && rUnitStructure.Conv <= 1.0e5 )
			{
				iMinDec = 1;
			}
			else if( rUnitStructure.Conv > 1.0e5 )
			{
				iMinDec = 2;
			}

			double dKeyCU = CDimValue::SItoCU( _U_PRESSURE, dKeySI );
			CString strFormat;
			strFormat.Format( _T("%%.%if"), iMinDec );

			CString strSetPressure;
			strSetPressure.Format( strFormat, dKeyCU );
			strSetPressure += _T(" ") + CString( pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );

			// We can convert here in int because in SI (Pa) we have for example 200000, 250000, ....
			pList->Add( (LPCTSTR)strSetPressure, dKeySI, (LPARAM)(int)dKeySI );
		}
	}

	return pList->GetCount();
}

int CTADatabase::GetSafetyValveSetPressureList( CRankEx *pSrcList, CRankEx *pDestList, ProjectType eSystemType, _string strSelFamilyID, _string strConnectID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the safety valve of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase*>( (CData *)lParam );
		ASSERT( NULL != pclSafetyValve );

		if( NULL == pclSafetyValve )
		{
			continue;
		}

		// Test project type compatibility.
		if( ProjectType::InvalidProjectType != eSystemType && eSystemType != pclSafetyValve->GetApplicationCompatibility() )
		{
			continue;
		}

		// Test type ID.
		if( false == strSelFamilyID.empty() && strSelFamilyID != pclSafetyValve->GetFamilyID() )
		{
			continue;
		}

		// Test type ID.
		if( false == strConnectID.empty() && strConnectID != pclSafetyValve->GetConnectID() )
		{
			continue;
		}

		// Add the new connection ID if doesn't already exist.
		double dKeySI = pclSafetyValve->GetSetPressure();

		// Already added ?
		_string InListstr = _T( "" );
		LPARAM InListlparam = (LPARAM)0;

		if( false == pDestList->GetaCopy( InListstr, dKeySI, InListlparam ) )
		{
			// See "SetPressure - Minimum decimal.xlsx" in the "doc" folder to know how we define the number of decimal we want to display.
			int iMinDec = 0;
			CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
			UnitDesign_struct rUnitStructure = pUnitDB->GetDefaultUnit( _U_PRESSURE );

			if( rUnitStructure.Conv >= 5.0e3 && rUnitStructure.Conv <= 1.0e5 )
			{
				iMinDec = 1;
			}
			else if( rUnitStructure.Conv > 1.0e5 )
			{
				iMinDec = 2;
			}

			double dKeyCU = CDimValue::SItoCU( _U_PRESSURE, dKeySI );
			CString strFormat;
			strFormat.Format( _T("%%.%if"), iMinDec );

			CString strSetPressure;
			strSetPressure.Format( strFormat, dKeyCU );
			strSetPressure += _T(" ") + CString( pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );

			// We can convert here in int because in SI (Pa) we have for example 200000, 250000, ....
			pDestList->Add( (LPCTSTR)strSetPressure, dKeySI, (LPARAM)(int)dKeySI );
		}
	}

	return pDestList->GetCount();
}

int CTADatabase::GetSafetyValveList( CRankEx *pList, ProjectType eSystemType, _string strSelFamilyID, _string strSelConnectID, double dSetPressure, 
		FilterSelection eFilterSelection, int iDNMin, int iDNMax, CData *pProd )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("SAFETYVALVE_TAB") ).MP );
	ASSERT( NULL != pTab );

	// For optimizing 'if' test below.
	CDB_StringID *pSelFamilyID = NULL;
	CDB_StringID *pSelConnectID = NULL;
	CDB_StringID *pSelVersionID = NULL;

	if( false == strSelFamilyID.empty() )
	{
		pSelFamilyID = ( (CDB_StringID *)Get( strSelFamilyID.c_str() ).MP );
	}

	if( false == strSelConnectID.empty() )
	{
		pSelConnectID = ( (CDB_StringID *)Get( strSelConnectID.c_str() ).MP );
	}

	// Loop on selectable safety valves.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		// Keep only CDB_TAProduct and children.
		CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase *>( IDPtr.MP );

		if( NULL == pclSafetyValve )
		{
			continue;
		}

		if( false == pclSafetyValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSafetyValve, eFilterSelection ) )
		{
			continue;
		}

		// Test project type compatibility.
		if( ProjectType::InvalidProjectType != eSystemType && eSystemType != pclSafetyValve->GetApplicationCompatibility() )
		{
			continue;
		}

		if( NULL != pSelFamilyID && pSelFamilyID != ( (CDB_StringID *)pclSafetyValve->GetFamilyIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelConnectID && pSelConnectID != ( (CDB_StringID *)pclSafetyValve->GetConnectIDPtr().MP ) )
		{
			continue;
		}

		if( 0.0 != dSetPressure && dSetPressure != pclSafetyValve->GetSetPressure() )
		{
			continue;
		}

		// Test DN. Here we test the inlet of the safety valve, it is this part that is connected to the pipe.
		int iDN = pclSafetyValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey = (double)pclSafetyValve->GetSortingKey();
		pList->Add( pclSafetyValve->GetIDPtr().ID, dKey, (LPARAM)IDPtr.MP );
	}

	return pList->GetCount();
}
#endif

#ifndef TACBX
int CTADatabase::GetActuatorTypeList( CRankEx *pList, FilterSelection eFilterSelection )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("ACTUATOR_TAB") ).MP );
	ASSERT( NULL != pTab );

	std::map<int, int> mapActrKeyList;

	// Fill the type combo according to selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_Actuator *pActuator = dynamic_cast<CDB_Actuator *>( IDPtr.MP );

		if( NULL == pActuator )
		{
			continue;
		}

		if( false == pActuator->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pActuator, eFilterSelection ) )
		{
			continue;
		}

		CTable *pActTypeTab = static_cast<CTable *>( Get( _T("ACTTYPE_TAB") ).MP );

		if( NULL == pActTypeTab )
		{
			continue;
		}

		CDB_StringID *pActrType = dynamic_cast<CDB_StringID *>( pActTypeTab->Get( pActuator->GetTypeIDPtr().ID ).MP );

		if( NULL == pActrType )
		{
			continue;
		}

		int iKey = _ttoi( pActrType->GetIDstr() );

		if( 0 == mapActrKeyList.count( iKey ) )
		{
			pList->Add( ( (CDB_StringID *)pActuator->GetTypeIDPtr().MP )->GetString(), iKey, (LPARAM)(void *)pActuator->GetTypeIDPtr().MP );
			mapActrKeyList[iKey] = iKey;
		}
	}

	return pList->GetCount();
}

int CTADatabase::GetActuatorFamilyList( CRankEx *pList, _string strSelActuatorTypeID, FilterSelection eFilterSelection )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("ACTUATOR_TAB") ).MP );
	ASSERT( NULL != pTab );

	std::map<int, int> mapActrKeyList;

	// Fill the type combo according to selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_Actuator *pActuator = dynamic_cast<CDB_Actuator *>( IDPtr.MP );

		if( NULL == pActuator )
		{
			continue;
		}

		if( false == pActuator->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pActuator, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelActuatorTypeID.empty() && strSelActuatorTypeID != pActuator->GetTypeIDPtr().ID )
		{
			continue;
		}

		CTable *pActTypeTab = static_cast<CTable *>( Get( _T("ACTFAM_TAB") ).MP );

		if( NULL == pActTypeTab )
		{
			continue;
		}

		CDB_StringID *pActrFamily = dynamic_cast<CDB_StringID *>( pActTypeTab->Get( pActuator->GetFamilyIDPtr().ID ).MP );

		if( NULL == pActrFamily )
		{
			continue;
		}

		int iKey = _ttoi( pActrFamily->GetIDstr() );

		if( 0 == mapActrKeyList.count( iKey ) )
		{
			pList->Add( ( (CDB_StringID *)pActuator->GetFamilyIDPtr().MP )->GetString(), iKey, (LPARAM)(void *)pActuator->GetFamilyIDPtr().MP );
			mapActrKeyList[iKey] = iKey;
		}
	}

	return pList->GetCount();
}

int CTADatabase::GetActuatorVersionList( CRankEx *pList, _string strSelActuatorTypeID, _string strSelActuatorFamilyID, FilterSelection eFilterSelection )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("ACTUATOR_TAB") ).MP );
	ASSERT( NULL != pTab );

	std::map<int, int> mapActrKeyList;

	// Fill the type combo according to selected type.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_Actuator *pActuator = dynamic_cast<CDB_Actuator *>( IDPtr.MP );

		if( NULL == pActuator )
		{
			continue;
		}

		if( false == pActuator->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pActuator, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelActuatorTypeID.empty() && strSelActuatorTypeID != pActuator->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelActuatorFamilyID.empty() && strSelActuatorFamilyID != pActuator->GetFamilyIDPtr().ID )
		{
			continue;
		}

		CTable *pActTypeTab = static_cast<CTable *>( Get( _T("ACTVERSION_TAB") ).MP );

		if( NULL == pActTypeTab )
		{
			continue;
		}

		CDB_StringID *pActrVersion = dynamic_cast<CDB_StringID *>( pActTypeTab->Get( pActuator->GetVersionIDPtr().ID ).MP );

		if( NULL == pActrVersion )
		{
			continue;
		}

		int iKey = _ttoi( pActrVersion->GetIDstr() );

		if( 0 == mapActrKeyList.count( iKey ) )
		{
			pList->Add( ( (CDB_StringID *)pActuator->GetVersionIDPtr().MP )->GetString(), iKey, (LPARAM)(void *)pActuator->GetVersionIDPtr().MP );
			mapActrKeyList[iKey] = iKey;
		}
	}

	return pList->GetCount();
}
#endif

#ifdef TACBX
int CTADatabase::GetPipeSerieList( CRankEx *pList, CDataBase *pDB, FilterSelection eFilterSelection )
{
	std::map<_string, CData *> mapPipeList;
	std::map<_string, CData *>::iterator it;

	CTable *pTab = static_cast<CTable *>( ( pDB->Get( _T("PIPE_TAB") ).MP ) );
	ASSERT( NULL != pTab );

	if( NULL == pTab )
	{
		return 0;
	}

	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CTable ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		if( true == IDPtr.MP->IsSelectable( true ) )
		{
			if( mapPipeList.find( ( static_cast<CTable *>( IDPtr.MP ) )->GetName() ) == mapPipeList.end() )
			{
				mapPipeList.insert( std::pair<_string, CData *>( ( static_cast<CTable *>( IDPtr.MP ) )->GetName(), IDPtr.MP ) );
			}
		}
	}

	// Try to find at least one pipe available in this series.
	_string str;

	int i = 0;

	for( it = mapPipeList.begin(); it != mapPipeList.end(); ++it )
	{
		pTab = static_cast<CTable *>( it->second );
		bool bSelectable = false;

		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			if( false == ( ( CDB_Thing * )IDPtr.MP )->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( ( CDB_Thing * )IDPtr.MP, eFilterSelection ) )
			{
				continue;
			}

			bSelectable = true;
			break;
		}

		if( true == bSelectable )
		{
			++i;
			pList->Add( it->first, i, (LPARAM)it->second, false );
		}
	}

	return pList->GetCount();
}
#endif

bool CTADatabase::CheckIfCharactAlreadyExist( CRankEx *pList, CData *pTApData, CData *pMatchTApData, bool bTestSize )
{
	CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( pTApData );
	CDB_TAProduct *pMatchTAp = dynamic_cast<CDB_TAProduct *>( pMatchTApData );

	if( NULL == pTAp )
	{
		return true;
	}

	// Valve char.
	CData *pValveCharacteristic = pTAp->GetValveCharDataPointer();

	if( NULL == pValveCharacteristic )
	{
		return true;
	}

	// Found product having priority.
	double dKey = (double)pTAp->GetSortingKey();
	double dSavedKey = 0.0;

	_string str;
	LPARAM itemdata;
	bool bExist = false;			// Already existing into the List?
	bool bForceReplace = false;		// Skip Key Test, force replacement
	bool bCouldbeAdded = true;

	bool bTAPVersion = false;
	bool bTAPConnect = false;

	if( NULL != pMatchTAp )
	{
		bTAPVersion = ( pMatchTAp->GetVersionIDPtr().MP == pTAp->GetVersionIDPtr().MP );
		bTAPConnect = ( pMatchTAp->GetConnectIDPtr().MP == pTAp->GetConnectIDPtr().MP );
	}

	/*
	 * 	pMatchTAP exist
	 * 	Characteristics matching
	  	TapConnect;	TapVersion;	SavConnect;	SavVersion	|	bExist;	bForceReplace;	bCouldbeAdded
	  	0			0			0			0		        1		0				1
	  	0			0			0			1				0		0				0
	  	0			0			1			0				0		0				0
	  	0			0			1			1				0		0				0
	  	0			1			0			0				1		1				1
	  	0			1			0			1				1		0				1
	  	0			1			1			0				1		1				1
	  	0			1			1			1				0		0				0

	  	1			0			0			0		        1		1				1
	  	1			0			0			1				0		0				0
	  	1			0			1			0				1		0				1
	  	1			0			1			1				0		0				0
	  	1			1			0			0				1		1				1
	  	1			1			0			1				1		1				1
	  	1			1			1			0				1		1				1
	  	1			1			1			1				1		0				1

	 	Reduced by Karnaught diagram bExist and bCouldbeAdded are same
	 	bExist = /bSavVersion(bTapConnect+bTapVersion+/bSavConnect)+bTapVersion(bTapConnect+/bSavConnect)
	 	bForceReplace = bTapVersion./bSavVersion+bTapConnect./bSavConnect.(bTapVersion+/bSavVersion)

	 */

	for( bool bContinue = pList->GetFirst( str, itemdata, &dSavedKey ); true == bContinue; bContinue = pList->GetNext( str, itemdata, &dSavedKey ) )
	{
		if( 0 == itemdata )
		{
			continue;
		}

		// Valve characteristic is matching?
		CDB_TAProduct *pSavedTAProduct = dynamic_cast<CDB_TAProduct *>( ( CData * )itemdata );

		if( NULL == pSavedTAProduct )
		{
			continue;
		}

		CData *pSavedCharacteristic = pSavedTAProduct->GetValveCharDataPointer();

		if( NULL == pSavedCharacteristic )
		{
			continue;
		}

		// Same characteristic and same size!
		// Size of object will be tested when flag fTestSize is true (by default, this flag is set to false when using PICV skip 15/20, 25/32,... valve duplication in TA-SCOPE)
		bool bTest = true;

		if( true == bTestSize )
		{
			bTest = ( pTAp->GetSizeIDPtr().MP == pSavedTAProduct->GetSizeIDPtr().MP ) ? true : false;
		}

		if( true == bTest && pValveCharacteristic == pSavedCharacteristic )
		{
			bExist = true;
			bForceReplace = false;

			// Verify version && connection.
			if( NULL != pMatchTAp )
			{
				bool bSavVersion = ( pMatchTAp->GetVersionIDPtr().MP == pSavedTAProduct->GetVersionIDPtr().MP );
				bool bSavConnect = ( pMatchTAp->GetConnectIDPtr().MP == pSavedTAProduct->GetConnectIDPtr().MP );
				bExist = ( bSavVersion * ( bTAPConnect + bTAPVersion + !bSavConnect ) + bTAPVersion * ( bTAPConnect + !bSavConnect ) ) ? true : false;
				bForceReplace = ( bTAPVersion * !bSavVersion + bTAPConnect * !bSavConnect * ( bTAPVersion + !bSavVersion ) ) ? true : false;
				bCouldbeAdded = bExist;
			}

			break;
		}
	}

	if( true == bExist )
	{
		if( true == bForceReplace )
		{
			pList->Delete();    // Destroy saved object, it will be replaced by the new ones
		}
		else
		{
			if( dSavedKey < dKey )	// saved object had a highest priority, keep it
			{
				return false;
			}
			else
			{
				pList->Delete();    // Destroy saved object, it will be replaced by the new ones
			}
		}
	}

	return bCouldbeAdded;
}

#ifdef TACBX
bool CTADatabase::CheckLocalizedVersion( CRankEx *pList, CData *pTApData )
{
	CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( pTApData );

	if( NULL == pTAp )
	{
		return true;
	}
	
	_string str;
	LPARAM itemdata;
	double dSavedKey = 0.0;
	
	for( bool bContinue = pList->GetFirst( str, itemdata, &dSavedKey ); true == bContinue; bContinue = pList->GetNext( str, itemdata, &dSavedKey ) )
	{
		if( 0 == itemdata )
		{
			continue;
		}

		CDB_TAProduct *pSavedTAProduct = dynamic_cast<CDB_TAProduct *>( ( CData * )itemdata );

		if( NULL == pSavedTAProduct )
		{
			continue;
		}

		// Continue if not same size.
		if( 0 != wcscmp( pSavedTAProduct->GetSizeID(), pTAp->GetSizeID() ) )
		{
			continue;
		}
		
		if( ( true == pSavedTAProduct->IsDeleted() || true == pSavedTAProduct->IsHidden() )
				&& ( false == pTAp->IsDeleted() && false == pTAp->IsHidden() ) )
		{
			// If the current product saved in the list is deleted or hidden and it's not the case
			// for the current one, we keep the current one.
			pList->Delete();
			return true;
		}
		else if( ( true == pTAp->IsDeleted() || true == pTAp->IsHidden() )
			&& ( false == pSavedTAProduct->IsDeleted() && false == pSavedTAProduct->IsHidden() ) )
		{
			// If the current product is deleted or hidden and it's not the case for the current 
			// product saved in the list, we don't keep the current one.
			return false;
		}
	}
	
	return true;
}
#endif

#ifndef TACBX
void CTADatabase::FillTargetTable( std::vector<_string> *pvTab, CvTargetTab eCvTarget )
{
	switch( eCvTarget )
	{
		case eForSSelCv:

			if( true == TASApp.IsCvDisplayed() ) 
			{
				pvTab->push_back( _string( L"CTRLVALV_TAB" ) );
			}

			break;

		case eForHMCv:

			if( true == TASApp.IsCvDisplayed() ) 
			{
				pvTab->push_back( _string( L"CTRLVALV_TAB" ) );
			}

			if( true == TASApp.IsBCvDisplayed() ) 
			{
				pvTab->push_back( _string( L"BALCTRLVALV_TAB" ) );
			}

			if( true == TASApp.IsDpCBCVDisplayed() ) 
			{
				pvTab->push_back( _string( L"DPCBALCTRLVALV_TAB" ) );
			}

			if( true == TASApp.IsTrvDisplayed() ) 
			{
				pvTab->push_back( _string( L"TRVALV_TAB" ) );
			}

			break;

		case eForHMCvBalTypeMV:

			if( true == TASApp.IsCvDisplayed() ) 
			{
				pvTab->push_back( _string( L"CTRLVALV_TAB" ) );
			}

			if( true == TASApp.IsBCvDisplayed() ) 
			{
				pvTab->push_back( _string( L"BALCTRLVALV_TAB" ) );
			}

			if( true == TASApp.IsTrvDisplayed() ) 
			{
				pvTab->push_back( _string( L"TRVALV_TAB" ) );
			}

			break;

		case eForPiCv:

			if( true == TASApp.IsPICvDisplayed() ) 
			{
				pvTab->push_back( _string( L"PICTRLVALV_TAB" ) );
			}

			break;

		case eForBCv:

			if( true == TASApp.IsBCvDisplayed() ) 
			{
				pvTab->push_back( _string( L"BALCTRLVALV_TAB" ) );
			}

			break;

		case eForTrv:

			if( true == TASApp.IsTrvDisplayed() ) 
			{
				pvTab->push_back( _string( L"TRVALV_TAB" ) );
			}

			break;

		case eForTechHCCv:
			
			// Special case for hydronic circuit calculation parameters (see 'DlgTPPageHC').
			if( true == TASApp.IsCvDisplayed() ) 
			{
				pvTab->push_back( _string( L"CTRLVALV_TAB" ) );
			}

			if( true == TASApp.IsTrvDisplayed() ) 
			{
				pvTab->push_back( _string( L"TRVALV_TAB" ) );
			}

			break;

		case eForTechHCBCv:
			// Special case for hydronic circuit calculation parameters (see 'DlgTPPageHC').

			if( true == TASApp.IsBCvDisplayed() ) 
			{
				pvTab->push_back( _string( L"BALCTRLVALV_TAB" ) );
			}

			if( true == TASApp.IsTrvDisplayed() ) 
			{
				pvTab->push_back( _string( L"TRVALV_TAB" ) );
			}

			break;

		case eForDpCBCV:

			if( true == TASApp.IsDpCBCVDisplayed() ) 
			{
				pvTab->push_back( _string( L"DPCBALCTRLVALV_TAB" ) );
			}

			break;

		// HYS-1150	
		case eFor6WayCV:

			if( true == TASApp.Is6WayCVDisplayed() ) 
			{
				pvTab->push_back( _string( L"6WAYCTRLVALV_TAB" ) );
			}

			break;

		// HYS-1150	
		case eForSmartControlValve:

			if( true == TASApp.IsSmartControlValveDisplayed() ) 
			{
				pvTab->push_back( _string( L"SMARTCONTROLVALVE_TAB" ) );
			}

			break;

		case eForSmartDpC:

			if( true == TASApp.IsSmartDpCDisplayed() ) 
			{
				pvTab->push_back( _string( L"SMARTDIFFPRESS_TAB" ) );
			}

			break;

		default:
			break;
	}
}
#else
void CTADatabase::FillTargetTable( std::vector<_string> *pvTab, CvTargetTab eCvTarget )
{
	switch( eCvTarget )
	{
		case eForSSelCv:
			pvTab->push_back( _string( L"CTRLVALV_TAB" ) );
			break;

		case eForHMCv:
			pvTab->push_back( _string( L"CTRLVALV_TAB" ) );
			pvTab->push_back( _string( L"BALCTRLVALV_TAB" ) );
			pvTab->push_back( _string( L"DPCBALCTRLVALV_TAB" ) );
			pvTab->push_back( _string( L"TRVALV_TAB" ) );
			break;

		case eForHMCvBalTypeMV:
			pvTab->push_back( _string( L"CTRLVALV_TAB" ) );
			pvTab->push_back( _string( L"BALCTRLVALV_TAB" ) );
			pvTab->push_back( _string( L"TRVALV_TAB" ) );
			break;

		case eForPiCv:
			pvTab->push_back( _string( L"PICTRLVALV_TAB" ) );
			break;

		case eForBCv:
			pvTab->push_back( _string( L"BALCTRLVALV_TAB" ) );
			break;

		case eForTrv:
			pvTab->push_back( _string( L"TRVALV_TAB" ) );
			break;

		case eForTechHCCv:
			// Special case for hydronic circuit calculation parameters (see 'DlgTPPageHC').
			pvTab->push_back( _string( L"CTRLVALV_TAB" ) );
			pvTab->push_back( _string( L"TRVALV_TAB" ) );
			break;

		case eForTechHCBCv:
			// Special case for hydronic circuit calculation parameters (see 'DlgTPPageHC').
			pvTab->push_back( _string( L"BALCTRLVALV_TAB" ) );
			pvTab->push_back( _string( L"TRVALV_TAB" ) );
			break;

		case eForDpCBCV:
			pvTab->push_back( _string( L"DPCBALCTRLVALV_TAB" ) );
			break;

		default:
			break;
	}
}
#endif

int CTADatabase::GetTACVCtrlTypeList( CRankEx *pList, CvTargetTab eCvTarget, bool bStrictCtrlTypeFit, CDB_ControlProperties::CV2W3W Cv2W3W, _string SelTypeID,
		CDB_ControlProperties::eCVFUNC CVFunc, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	// Create a vector with all tables needed.
	std::vector<_string> vTab;
	FillTargetTable( &vTab, eCvTarget );

	// Fill an array with all eCvCtrlType possibilities.
	bool bAllChecked = true;
	bool ar[CDB_ControlProperties::eLastCvCtrlType];
	memset( ar, 0, sizeof( ar ) );

	// Do the loop on the different tables.
	for( UINT i = 0; i < vTab.size(); i++ )
	{
		CTable *pTab = static_cast<CTable *>( Get( vTab[i].c_str() ).MP );

		// Do the loop on the valves of the current table.
		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// Keep only CDB_RegulatingValve and children.
			CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( IDPtr.MP );

			if( NULL == pRv )
			{
				continue;
			}

			// Test if product is selectable.
			if( false == pRv->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( pRv, eFilterSelection ) )
			{
				continue;
			}

			// Test Cv2W3W only if we are below lastCV2W3W (CV2W, CV3W or CV4W).
			if( Cv2W3W < CDB_ControlProperties::LastCV2W3W && pRv->GetCtrlProp()->GetCv2w3w() != Cv2W3W )
			{
				continue;
			}

			// Test control valve function (NoControl, ControlOnly, Presettable or PresetPT).
			if( CVFunc < CDB_ControlProperties::LastCVFUNC && pRv->GetCtrlProp()->GetCvFunc() != CVFunc )
			{
				continue;
			}

			// Test SelTypeID.
			if( false == SelTypeID.empty() && SelTypeID != pRv->GetTypeIDPtr().ID )
			{
				continue;
			}

			// Test DN.
			int iDN = pRv->GetSizeKey();

			if( iDN < iDNMin )
			{
				continue;
			}

			if( iDN > iDNMax )
			{
				continue;
			}

			// Test CtrlType.
			if( pRv->GetCtrlProp()->CvCtrlTypeFits( CDB_ControlProperties::eCvProportional ) )
			{
				ar[CDB_ControlProperties::eCvProportional] = true;
			}

			if( pRv->GetCtrlProp()->CvCtrlTypeFits( CDB_ControlProperties::eCvOnOff ) )
			{
				ar[CDB_ControlProperties::eCvOnOff] = true;
			}

			if( pRv->GetCtrlProp()->CvCtrlTypeFits( CDB_ControlProperties::eCv3point ) )
			{
				ar[CDB_ControlProperties::eCv3point] = true;
			}

			bAllChecked = true;

			for( int i = 0; i < CDB_ControlProperties::eLastCvCtrlType && true == bAllChecked; i++ )
			{
				if( false == ar[i] )
				{
					bAllChecked = false;
				}
			}

			if( true == bAllChecked )
			{
				break;
			}
		}
	}

	// Add each parameter eCvProportional, eCv3point, eCvOnOff because it is not place in the same way
	// in the enum. Do not change the enum because of TSP files compatibility.
	std::map< CDB_ControlProperties::CvCtrlType, int > mapSorted;
	mapSorted[ CDB_ControlProperties::eCvProportional ] = 0;
	mapSorted[ CDB_ControlProperties::eCv3point ] = 1;
	mapSorted[ CDB_ControlProperties::eCvOnOff ] = 2;

	if( true == ar[CDB_ControlProperties::eCvProportional] )
	{
		_string str = CDB_ControlProperties::GetCvCtrlTypeStr( CDB_ControlProperties::eCvProportional );
		pList->Add( str.c_str(), mapSorted[CDB_ControlProperties::eCvProportional], CDB_ControlProperties::eCvProportional );
	}

	// Add into the list the eCvOnOff.
	if( true == ar[CDB_ControlProperties::eCvOnOff] )
	{
		_string str = CDB_ControlProperties::GetCvCtrlTypeStr( CDB_ControlProperties::eCvOnOff );
		pList->Add( str.c_str(), mapSorted[CDB_ControlProperties::eCvOnOff], CDB_ControlProperties::eCvOnOff );
	}

	// Add into the list the eCv3point.
	if( true == ar[CDB_ControlProperties::eCv3point] )
	{
		_string str = CDB_ControlProperties::GetCvCtrlTypeStr( CDB_ControlProperties::eCv3point );
		pList->Add( str.c_str(), mapSorted[CDB_ControlProperties::eCv3point], CDB_ControlProperties::eCv3point );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetTACVCtrlTypeList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	// Fill an array with all eCvCtrlType possibilities.
	bool bAllChecked = true;
	bool ar[CDB_ControlProperties::eLastCvCtrlType];
	memset( ar, 0, sizeof( ar ) );

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the control valve list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve*>( (CData *)lParam );
		ASSERT( NULL != pclControlValve );

		if( NULL == pclControlValve )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test CtrlType.
		if( pclControlValve->GetCtrlProp()->CvCtrlTypeFits( CDB_ControlProperties::eCvProportional ) )
		{
			ar[CDB_ControlProperties::eCvProportional] = true;
		}

		if( pclControlValve->GetCtrlProp()->CvCtrlTypeFits( CDB_ControlProperties::eCvOnOff ) )
		{
			ar[CDB_ControlProperties::eCvOnOff] = true;
		}

		if( pclControlValve->GetCtrlProp()->CvCtrlTypeFits( CDB_ControlProperties::eCv3point ) )
		{
			ar[CDB_ControlProperties::eCv3point] = true;
		}

		bAllChecked = true;

		for( int i = 0; i < CDB_ControlProperties::eLastCvCtrlType && true == bAllChecked; i++ )
		{
			if( false == ar[i] )
			{
				bAllChecked = false;
			}
		}

		if( true == bAllChecked )
		{
			break;
		}
	}

	// Add each parameter eCvProportional, eCv3point, eCvOnOff because it is not place in the same way
	// in the enum. Do not change the enum because of TSP files compatibility.
	std::map< CDB_ControlProperties::CvCtrlType, int > mapSorted;
	mapSorted[ CDB_ControlProperties::eCvProportional ] = 0;
	mapSorted[ CDB_ControlProperties::eCv3point ] = 1;
	mapSorted[ CDB_ControlProperties::eCvOnOff ] = 2;

	if( true == ar[CDB_ControlProperties::eCvProportional] )
	{
		str = CDB_ControlProperties::GetCvCtrlTypeStr( CDB_ControlProperties::eCvProportional );
		pDestList->Add( str.c_str(), mapSorted[CDB_ControlProperties::eCvProportional], CDB_ControlProperties::eCvProportional );
	}

	// Add into the list the eCvOnOff.
	if( true == ar[CDB_ControlProperties::eCvOnOff] )
	{
		str = CDB_ControlProperties::GetCvCtrlTypeStr( CDB_ControlProperties::eCvOnOff );
		pDestList->Add( str.c_str(), mapSorted[CDB_ControlProperties::eCvOnOff], CDB_ControlProperties::eCvOnOff );
	}

	// Add into the list the eCv3point.
	if( true == ar[CDB_ControlProperties::eCv3point] )
	{
		str = CDB_ControlProperties::GetCvCtrlTypeStr( CDB_ControlProperties::eCv3point );
		pDestList->Add( str.c_str(), mapSorted[CDB_ControlProperties::eCv3point], CDB_ControlProperties::eCv3point );
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetTaCVTypeList( CRankEx *pList, CvTargetTab eCvTarget, CDB_ControlProperties::CV2W3W Cv2W3W,
		CDB_ControlProperties::eCVFUNC CVFunc, CDB_ControlProperties::CvCtrlType CvCtrlType, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	// Create a vector with all tables needed.
	std::vector<_string> vTab;
	FillTargetTable( &vTab, eCvTarget );

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Do the loop on the different tables.
	for( UINT i = 0; i < vTab.size(); i++ )
	{
		CTable *pTab = static_cast<CTable *>( Get( vTab[i].c_str() ).MP );

		// Do the loop on the valves of the current table.
		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// Keep only CDB_RegulatingValve and children.
			CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( IDPtr.MP );

			if( NULL == pRv )
			{
				continue;
			}

			// Test if product is selectable.
			if( false == pRv->IsSelectable( true, false, bOldProductsAvailableForHC ) )
			{
				continue;
			}

			if( true == MustExclude( pRv, eFilterSelection ) )
			{
				continue;
			}

			// Test Cv2W3W and CVFunc
			if( false == pRv->IsValidCV( Cv2W3W, CVFunc ) )
			{
				continue;
			}

			// Test CtrlType.
			if( CvCtrlType != CDB_ControlProperties::eCvNU && !pRv->GetCtrlProp()->CvCtrlTypeFits( CvCtrlType ) )
			{
				continue;
			}

			if( eFilterSelection == ForHMCalc )
			{
				CDB_ControlProperties *pCtrlProp = dynamic_cast<CDB_ControlProperties *>( pRv->GetCtrlProp() );

				if( NULL == pCtrlProp )
				{
					continue;
				}

				if( CDB_ControlProperties::PresetPT == pCtrlProp->GetCvFunc() || CDB_ControlProperties::Presettable == pCtrlProp->GetCvFunc() )
				{
					// To exclude valves without characteristics .... KTM50
					CDB_ValveCharacteristic *pValvChar = pRv->GetValveCharacteristic();

					if( NULL == pValvChar || false == pValvChar->HasKvCurve() )
					{
						continue;
					}
				}
			}

			// Test DN.
			int iDN = pRv->GetSizeKey();

			if( iDN < iDNMin )
			{
				continue;
			}

			if( iDN > iDNMax )
			{
				continue;
			}

			// Add new TypeId if doesn't already exist.
			IDPTR TypeIDPtr = pRv->GetTypeIDPtr();
			double dKey = 0;
			_string str;
			LPARAM lparam = (LPARAM)TypeIDPtr.MP;

			if( !pList->GetaCopy( str, dKey, lparam ) )
			{
				dKey = (double)_ttoi( ( (CDB_StringID *)TypeIDPtr.MP )->GetIDstr() );
				pList->Add( ( (CDB_StringID *)TypeIDPtr.MP )->GetString(), dKey, (LPARAM)TypeIDPtr.MP );
			}
		}
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetTaCVTypeList( CRankEx *pSrcList, CRankEx *pDestList )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the control valve list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve*>( (CData *)lParam );
		ASSERT( NULL != pclControlValve );

		if( NULL == pclControlValve )
		{
			continue;
		}

		// Add the new type ID if doesn't already exist.
		IDPTR TypeIDPtr = pclControlValve->GetTypeIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( TypeIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)TypeIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)TypeIDPtr.MP )->GetString(), dKey, (LPARAM)TypeIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetTaCVFamList( CRankEx *pList, CvTargetTab eCvTarget, CDB_ControlProperties::CV2W3W Cv2W3W, _string strSelTypeID,
		CDB_ControlProperties::eCVFUNC eCVFunc, CDB_ControlProperties::CvCtrlType eCvCtrlType, FilterSelection eFilterSelection, int iDNMin, int iDNMax,
		bool bDpControllerCanBeDisabled )
{
	// Create a vector with all tables needed.
	std::vector<_string> vTab;
	FillTargetTable( &vTab, eCvTarget );

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Do the loop on the different tables.
	for( unsigned int i = 0; i < vTab.size(); i++ )
	{
		CTable *pTab = static_cast<CTable *>( Get( vTab[i].c_str() ).MP );
		ASSERT( NULL != pTab );

		// Do the loop on the valves of the current table.
		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// Keep only CDB_RegulatingValve and children.
			CDB_RegulatingValve *pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( IDPtr.MP );

			if( NULL == pclRegulatingValve )
			{
				continue;
			}

			// Test IsSelectable
			if( false == pclRegulatingValve->IsSelectable( true, false, bOldProductsAvailableForHC ) )
			{
				continue;
			}

			if( true == MustExclude( pclRegulatingValve, eFilterSelection ) )
			{
				continue;
			}

			// Test Cv2W3W and CVFunc.
			if( false == pclRegulatingValve->IsValidCV( Cv2W3W, eCVFunc ) )
			{
				continue;
			}

			// Test CtrlType.
			if( eCvCtrlType != CDB_ControlProperties::eCvNU && !pclRegulatingValve->GetCtrlProp()->CvCtrlTypeFits( eCvCtrlType ) )
			{
				continue;
			}

			if( eFilterSelection == ForHMCalc )
			{
				CDB_ControlProperties *pCtrlProp = dynamic_cast<CDB_ControlProperties *>( pclRegulatingValve->GetCtrlProp() );

				if( NULL == pCtrlProp )
				{
					continue;
				}

				if( CDB_ControlProperties::PresetPT == pCtrlProp->GetCvFunc() || CDB_ControlProperties::Presettable == pCtrlProp->GetCvFunc() )
				{
					// To exclude valves without characteristics .... KTM50
					CDB_ValveCharacteristic *pValvChar = pclRegulatingValve->GetValveCharacteristic();

					if( NULL == pValvChar || false == pValvChar->HasKvCurve() )
					{
						continue;
					}
				}
			}

			// Test SelTypeID.
			if( false == strSelTypeID.empty() && strSelTypeID != pclRegulatingValve->GetTypeIDPtr().ID )
			{
				continue;
			}

			// Test DN.
			int iDN = pclRegulatingValve->GetSizeKey();

			if( iDN < iDNMin )
			{
				continue;
			}

			if( iDN > iDNMax )
			{
				continue;
			}

			// HYS-1162: Check if this valve has a Dp controller part that can be disabled.
			if( true == bDpControllerCanBeDisabled )
			{
				if( CDB_TAProduct::emdDpCAlwaysOn == ( CDB_TAProduct::emdDpCAlwaysOn & pclRegulatingValve->GetMeasurableData() ) )
				{
					continue;
				}
			}
			
			// Found product having priority.
			CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
			double dKey = pclRegulatingValve->GetSortingKey( &clCTAPSKMask );

			// Already added ?
			_string InListstr;
			LPARAM InListlparam = (LPARAM)(void *)( ( CDB_TAProduct * )IDPtr.MP )->GetFamilyIDPtr().MP;
			double dInListkey = 0;

			if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
			{
				// Yes take a copy, check priority key, and keep only the most important.
				if( dKey < dInListkey )
				{
					pList->Delete( dInListkey );
				}
				else
				{
					continue;
				}
			}

			pList->Add( ( (CDB_StringID *)pclRegulatingValve->GetFamilyIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclRegulatingValve->GetFamilyIDPtr().MP );
		}
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetTACVFamList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, CDB_ControlProperties::CvCtrlType eCvCtrlType )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the control valve list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve*>( (CData *)lParam );
		ASSERT( NULL != pclControlValve );

		if( NULL == pclControlValve )
		{
			continue;
		}

		// Test control type if defined.
		if( eCvCtrlType != CDB_ControlProperties::eCvNU && false == pclControlValve->GetCtrlProp()->CvCtrlTypeFits( eCvCtrlType ) )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		double dKey = pclControlValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)( pclControlValve->GetFamilyIDPtr().MP );
		double dInListkey = 0;

		if( pDestList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pDestList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pDestList->Add( ( (CDB_StringID *)pclControlValve->GetFamilyIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclControlValve->GetFamilyIDPtr().MP );
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetTaCVBdyMatList( CRankEx *pList, CvTargetTab eCvTarget, CDB_ControlProperties::CV2W3W Cv2W3W, _string strSelTypeID, _string strSelFamID, 
		CDB_ControlProperties::eCVFUNC eCVFunc, CDB_ControlProperties::CvCtrlType eCvCtrlType, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	// Create a vector with all tables needed.
	std::vector<_string> vTab;
	FillTargetTable( &vTab, eCvTarget );

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Do the loop on the different tables.
	for( unsigned int i = 0; i < vTab.size(); i++ )
	{
		CTable *pTab = static_cast<CTable *>( Get( vTab[i].c_str() ).MP );
		ASSERT( NULL != pTab );

		// Do the loop on the valves of the current table.
		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// Keep only CDB_RegulatingValve and children.
			CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( IDPtr.MP );

			if( NULL == pRv )
			{
				continue;
			}

			// Test if product is selectable.
			if( false == pRv->IsSelectable( true, false, bOldProductsAvailableForHC ) )
			{
				continue;
			}

			if( true == MustExclude( pRv, eFilterSelection ) )
			{
				continue;
			}

			// Test Cv2W3W and CVFunc.
			if( false == pRv->IsValidCV( Cv2W3W, eCVFunc ) )
			{
				continue;
			}

			// Test CtrlType.
			if( eCvCtrlType != CDB_ControlProperties::eCvNU && !pRv->GetCtrlProp()->CvCtrlTypeFits( eCvCtrlType ) )
			{
				continue;
			}

			if( eFilterSelection == ForHMCalc )
			{
				CDB_ControlProperties *pCtrlProp = dynamic_cast<CDB_ControlProperties *>( pRv->GetCtrlProp() );

				if( NULL == pCtrlProp )
				{
					continue;
				}

				if( CDB_ControlProperties::PresetPT == pCtrlProp->GetCvFunc() || CDB_ControlProperties::Presettable == pCtrlProp->GetCvFunc() )
				{
					// To exclude valves without characteristics .... KTM50
					CDB_ValveCharacteristic *pValvChar = pRv->GetValveCharacteristic();

					if( NULL == pValvChar || false == pValvChar->HasKvCurve() )
					{
						continue;
					}
				}
			}

			// Test SelTypeID.
			if( false == strSelTypeID.empty() && strSelTypeID != pRv->GetTypeIDPtr().ID )
			{
				continue;
			}

			// Test SelFamID.
			if( false == strSelFamID.empty() && strSelFamID != pRv->GetFamilyIDPtr().ID )
			{
				continue;
			}

			// Test DN.
			int iDN = pRv->GetSizeKey();

			if( iDN < iDNMin )
			{
				continue;
			}

			if( iDN > iDNMax )
			{
				continue;
			}

			// Found product having priority.
			CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Bdy | CTAPSortKey::TAPSKM_PriorL );
			double dKey = pRv->GetSortingKey( &clCTAPSKMask );

			// Already added ?
			_string InListstr;
			LPARAM InListlparam = (LPARAM)(void *)pRv->GetBodyMaterialIDPtr().MP;
			double dInListkey = 0;

			if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
			{
				// Yes take a copy, check priority key, and keep only the most important.
				if( dKey < dInListkey )
				{
					pList->Delete( dInListkey );
				}
				else
				{
					continue;
				}
			}

			pList->Add( ( (CDB_StringID *)pRv->GetBodyMaterialIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pRv->GetBodyMaterialIDPtr().MP );
		}
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetTaCVBdyMatList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, CDB_ControlProperties::CvCtrlType eCvCtrlType )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the control valve list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve*>( (CData *)lParam );
		ASSERT( NULL != pclControlValve );

		if( NULL == pclControlValve )
		{
			continue;
		}

		// Test control type.
		if( eCvCtrlType != CDB_ControlProperties::eCvNU && !pclControlValve->GetCtrlProp()->CvCtrlTypeFits( eCvCtrlType ) )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Bdy | CTAPSortKey::TAPSKM_PriorL );
		double dKey = pclControlValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pclControlValve->GetBodyMaterialIDPtr().MP;
		double dInListkey = 0;

		if( pDestList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pDestList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pDestList->Add( ( (CDB_StringID *)pclControlValve->GetBodyMaterialIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclControlValve->GetBodyMaterialIDPtr().MP );
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetTaCVConnList( CRankEx *pList, CvTargetTab eCvTarget, CDB_ControlProperties::CV2W3W eCv2W3W, _string strSelTypeID, 
		_string strSelFamID, _string strSelBdyMatID, CDB_ControlProperties::eCVFUNC eCVFunc, CDB_ControlProperties::CvCtrlType eCvCtrlType, 
		FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	// Create a vector with all tables needed.
	std::vector<_string> vTab;
	FillTargetTable( &vTab, eCvTarget );

	// Fill the connection combo according to selected type.
	CTable *pConnTab = static_cast<CTable *>( Get( L"CONNECT_TAB" ).MP );
	ASSERT( NULL != pConnTab );

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Do the loop on the different tables.
	for( unsigned int i = 0; i < vTab.size(); i++ )
	{
		CTable *pTab = static_cast<CTable *>( Get( vTab[i].c_str() ).MP );
		ASSERT( NULL != pTab );

		// Do the loop on the valves of the current table.
		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// Keep only CDB_RegulatingValve and children.
			CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( IDPtr.MP );

			if( NULL == pRv )
			{
				continue;
			}

			// Test if product is selectable.
			if( false == pRv->IsSelectable( true, false, bOldProductsAvailableForHC ) )
			{
				continue;
			}

			if( true == MustExclude( pRv, eFilterSelection ) )
			{
				continue;
			}

			// Test Cv2W3W and CVFunc.
			if( false == pRv->IsValidCV( eCv2W3W, eCVFunc ) )
			{
				continue;
			}

			// Test CtrlType.
			if( eCvCtrlType != CDB_ControlProperties::eCvNU && !pRv->GetCtrlProp()->CvCtrlTypeFits( eCvCtrlType ) )
			{
				continue;
			}

			if( eFilterSelection == ForHMCalc )
			{
				CDB_ControlProperties *pCtrlProp = dynamic_cast<CDB_ControlProperties *>( pRv->GetCtrlProp() );

				if( NULL == pCtrlProp )
				{
					continue;
				}

				if( CDB_ControlProperties::PresetPT == pCtrlProp->GetCvFunc() || CDB_ControlProperties::Presettable == pCtrlProp->GetCvFunc() )
				{
					// To exclude valves without characteristics .... KTM50
					CDB_ValveCharacteristic *pValvChar = pRv->GetValveCharacteristic();

					if( NULL == pValvChar || false == pValvChar->HasKvCurve() )
					{
						continue;
					}
				}
			}

			// Test SelTypeID.
			if( false == strSelTypeID.empty() && strSelTypeID != pRv->GetTypeIDPtr().ID )
			{
				continue;
			}

			// Test SelFamID.
			if( false == strSelFamID.empty() && strSelFamID != pRv->GetFamilyIDPtr().ID )
			{
				continue;
			}

			// Test SelBdyMatID.
			if( false == strSelBdyMatID.empty() && strSelBdyMatID != pRv->GetBodyMaterialIDPtr().ID )
			{
				continue;
			}

			// Test DN.
			int iDN = pRv->GetSizeKey();

			if( iDN < iDNMin )
			{
				continue;
			}

			if( iDN > iDNMax )
			{
				continue;
			}

			// Found product having priority.
			CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Connect | CTAPSortKey::TAPSKM_PriorL );
			double dKey = pRv->GetSortingKey( &clCTAPSKMask );

			// Already added ?
			_string InListstr;
			LPARAM InListlparam = (LPARAM)(void *)( ( CDB_TAProduct * )IDPtr.MP )->GetConnectIDPtr().MP;
			double dInListkey = 0;

			if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
			{
				// Yes take a copy, check priority key, and keep only the most important.
				if( dKey < dInListkey )
				{
					pList->Delete( dInListkey );
				}
				else
				{
					continue;
				}
			}

			pList->Add( ( (CDB_StringID *)pRv->GetConnectIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pRv->GetConnectIDPtr().MP );
		}
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetTaCVConnList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, _string strBodyMaterialID, CDB_ControlProperties::CvCtrlType eCvCtrlType )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the control valve list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve*>( (CData *)lParam );
		ASSERT( NULL != pclControlValve );

		if( NULL == pclControlValve )
		{
			continue;
		}

		// Test control type.
		if( eCvCtrlType != CDB_ControlProperties::eCvNU && !pclControlValve->GetCtrlProp()->CvCtrlTypeFits( eCvCtrlType ) )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test body material ID.
		if( false == strBodyMaterialID.empty() && strBodyMaterialID != pclControlValve->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Connect | CTAPSortKey::TAPSKM_PriorL );
		double dKey = pclControlValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)( pclControlValve->GetConnectIDPtr().MP );
		double dInListkey = 0;

		if( pDestList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pDestList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pDestList->Add( ( (CDB_StringID *)pclControlValve->GetConnectIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclControlValve->GetConnectIDPtr().MP );
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetTaCVVersList( CRankEx *pList, CvTargetTab eCvTarget, CDB_ControlProperties::CV2W3W eCv2W3W, _string strSelTypeID, _string strSelFamID, 
		_string strSelBdyMatID, _string strSelConnectID, CDB_ControlProperties::eCVFUNC eCVFunc, CDB_ControlProperties::CvCtrlType eCvCtrlType,
		FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	// Create a vector with all tables needed.
	std::vector<_string> vTab;
	FillTargetTable( &vTab, eCvTarget );

	CTable *pVersTab = static_cast<CTable *>( Get( L"VERSION_TAB" ).MP );
	ASSERT( NULL != pVersTab );

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Do the loop on the different tables.
	for( unsigned int i = 0; i < vTab.size(); i++ )
	{
		CTable *pTab = static_cast<CTable *>( Get( vTab[i].c_str() ).MP );
		ASSERT( NULL != pTab );

		// Do the loop on the valves of the current table.
		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// Keep only CDB_RegulatingValve and children.
			CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( IDPtr.MP );

			if( NULL == pRv )
			{
				continue;
			}

			// Test if product is selectable.
			if( false == pRv->IsSelectable( true, false, bOldProductsAvailableForHC ) )
			{
				continue;
			}

			if( true == MustExclude( pRv, eFilterSelection ) )
			{
				continue;
			}

			// Test Cv2W3W and CVFunc.
			if( false == pRv->IsValidCV( eCv2W3W, eCVFunc ) )
			{
				continue;
			}

			// Test CtrlType.
			if( eCvCtrlType != CDB_ControlProperties::eCvNU && !pRv->GetCtrlProp()->CvCtrlTypeFits( eCvCtrlType ) )
			{
				continue;
			}

			if( eFilterSelection == ForHMCalc )
			{
				CDB_ControlProperties *pCtrlProp = dynamic_cast<CDB_ControlProperties *>( pRv->GetCtrlProp() );

				if( NULL == pCtrlProp )
				{
					continue;
				}

				if( CDB_ControlProperties::PresetPT == pCtrlProp->GetCvFunc() || CDB_ControlProperties::Presettable == pCtrlProp->GetCvFunc() )
				{
					// To exclude valves without characteristics .... KTM50
					CDB_ValveCharacteristic *pValvChar = pRv->GetValveCharacteristic();

					if( NULL == pValvChar || false == pValvChar->HasKvCurve() )
					{
						continue;
					}
				}
			}

			// Test SelTypeID.
			if( false == strSelTypeID.empty() && strSelTypeID != pRv->GetTypeIDPtr().ID )
			{
				continue;
			}

			// Test SelFamID.
			if( false == strSelFamID.empty() && strSelFamID != pRv->GetFamilyIDPtr().ID )
			{
				continue;
			}

			// Test SelBdyMatID.
			if( false == strSelBdyMatID.empty() && strSelBdyMatID != pRv->GetBodyMaterialIDPtr().ID )
			{
				continue;
			}

			// Test SelConnectID.
			if( false == strSelConnectID.empty() && strSelConnectID != pRv->GetConnectIDPtr().ID )
			{
				continue;
			}

			// Test DN.
			int iDN = pRv->GetSizeKey();

			if( iDN < iDNMin )
			{
				continue;
			}

			if( iDN > iDNMax )
			{
				continue;
			}

			// Found product having priority
			CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Version | CTAPSortKey::TAPSKM_PriorL );
			double dKey = pRv->GetSortingKey( &clCTAPSKMask );

			// Already added ?
			_string InListstr;
			LPARAM InListlparam = (LPARAM)(void *)pRv->GetVersionIDPtr().MP;
			double dInListkey = 0;

			if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
			{
				// Yes take a copy, check priority key, and keep only the most important.
				if( dKey < dInListkey )
				{
					pList->Delete( dInListkey );
				}
				else
				{
					continue;
				}
			}

			pList->Add( ( (CDB_StringID *)pRv->GetVersionIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pRv->GetVersionIDPtr().MP );
		}
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetTaCVVersList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, _string strBodyMaterialID, _string strConnectID,
		CDB_ControlProperties::CvCtrlType eCvCtrlType )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the control valve list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve*>( (CData *)lParam );
		ASSERT( NULL != pclControlValve );

		if( NULL == pclControlValve )
		{
			continue;
		}

		// Test control type.
		if( eCvCtrlType != CDB_ControlProperties::eCvNU && false == pclControlValve->GetCtrlProp()->CvCtrlTypeFits( eCvCtrlType ) )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test body material ID.
		if( false == strBodyMaterialID.empty() && strBodyMaterialID != pclControlValve->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Test connection ID.
		if( false == strConnectID.empty() && strConnectID != pclControlValve->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Found product having priority
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Version | CTAPSortKey::TAPSKM_PriorL );
		double dKey = pclControlValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)( pclControlValve->GetVersionIDPtr().MP );
		double dInListkey = 0;

		if( pDestList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pDestList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pDestList->Add( ( (CDB_StringID *)pclControlValve->GetVersionIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)( pclControlValve->GetVersionIDPtr().MP ) );
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetTaCVPNList( CRankEx *pList, CvTargetTab eCvTarget, CDB_ControlProperties::CV2W3W eCv2W3W, _string strSelTypeID, _string strSelFamID,
		_string strSelBdyMatID, _string strSelConnectID, _string strSelVersionID, CDB_ControlProperties::eCVFUNC eCVFunc, CDB_ControlProperties::CvCtrlType eCvCtrlType,
		FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	// Create a vector with all tables needed.
	std::vector<_string> vTab;
	FillTargetTable( &vTab, eCvTarget );

	// Fill the PN combo according to selected type, connection and version.

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Do the loop on the different tables.
	for( unsigned int i = 0; i < vTab.size(); i++ )
	{
		CTable *pTab = static_cast<CTable *>( Get( vTab[i].c_str() ).MP );
		ASSERT( NULL != pTab );

		// Do the loop on the valves of the current table.
		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// Keep only CDB_RegulatingValve and children.
			CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( IDPtr.MP );

			if( NULL == pRv )
			{
				continue;
			}

			// Test IsSelectable
			if( false == pRv->IsSelectable( true, false, bOldProductsAvailableForHC ) )
			{
				continue;
			}

			if( true == MustExclude( pRv, eFilterSelection ) )
			{
				continue;
			}

			// Test Cv2W3W and CVFunc.
			if( false == pRv->IsValidCV( eCv2W3W, eCVFunc ) )
			{
				continue;
			}

			// Test CtrlType.
			if( eCvCtrlType != CDB_ControlProperties::eCvNU && !pRv->GetCtrlProp()->CvCtrlTypeFits( eCvCtrlType ) )
			{
				continue;
			}

			if( eFilterSelection == ForHMCalc )
			{
				CDB_ControlProperties *pCtrlProp = dynamic_cast<CDB_ControlProperties *>( pRv->GetCtrlProp() );

				if( NULL == pCtrlProp )
				{
					continue;
				}

				if( CDB_ControlProperties::PresetPT == pCtrlProp->GetCvFunc() || CDB_ControlProperties::Presettable == pCtrlProp->GetCvFunc() )
				{
					// To exclude valves without characteristics .... KTM50
					CDB_ValveCharacteristic *pValvChar = pRv->GetValveCharacteristic();

					if( NULL == pValvChar || false == pValvChar->HasKvCurve() )
					{
						continue;
					}
				}
			}

			// Test SelTypeID.
			if( false == strSelTypeID.empty() && strSelTypeID != pRv->GetTypeIDPtr().ID )
			{
				continue;
			}

			// Test SelFamID.
			if( false == strSelFamID.empty() && strSelFamID != pRv->GetFamilyIDPtr().ID )
			{
				continue;
			}

			// Test SelBdyMatID.
			if( false == strSelBdyMatID.empty() && strSelBdyMatID != pRv->GetBodyMaterialIDPtr().ID )
			{
				continue;
			}

			// Test SelConnectID.
			if( false == strSelConnectID.empty() && strSelConnectID != pRv->GetConnectIDPtr().ID )
			{
				continue;
			}

			// Test SelVersionID.
			if( false == strSelVersionID.empty() && strSelVersionID != pRv->GetVersionIDPtr().ID )
			{
				continue;
			}

			// Test DN.
			int iDN = pRv->GetSizeKey();

			if( iDN < iDNMin )
			{
				continue;
			}

			if( iDN > iDNMax )
			{
				continue;
			}

			pRv->GetPNList( pList );
		}
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetTaCVPNList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, _string strBodyMaterialID, _string strConnectID,
		_string strVersionID, CDB_ControlProperties::CvCtrlType eCvCtrlType )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the control valve list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve*>( (CData *)lParam );
		ASSERT( NULL != pclControlValve );

		if( NULL == pclControlValve )
		{
			continue;
		}

		// Test control type.
		if( eCvCtrlType != CDB_ControlProperties::eCvNU && !pclControlValve->GetCtrlProp()->CvCtrlTypeFits( eCvCtrlType ) )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test body material ID.
		if( false == strBodyMaterialID.empty() && strBodyMaterialID != pclControlValve->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Test connection ID.
		if( false == strConnectID.empty() && strConnectID != pclControlValve->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Test version ID.
		if( false == strVersionID.empty() && strVersionID != pclControlValve->GetVersionIDPtr().ID )
		{
			continue;
		}

		pclControlValve->GetPNList( pDestList );
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetTaCVList( CRankEx *pList, CvTargetTab eCvTarget, bool bRetIbFound, CDB_ControlProperties::CV2W3W eCv2W3W, _string strSelTypeID, 
		_string strSelFamID, _string strSelBdyMatID, _string strSelConnectID, _string strSelVersionID, _string strSelPNID, CDB_ControlProperties::eCVFUNC eCVFunc,
		CDB_ControlProperties::CvCtrlType eCvCtrlType, FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bForHubStation, CData *pProd, bool bOnlySet )
{
	// Create a vector with all tables needed.
	std::vector<_string> vTab;
	FillTargetTable( &vTab, eCvTarget );

#ifndef TACBX
	// HYS-1112: bForSet: To check that the Set is complete before proposing the product
	bool bForSet = false;
#endif

	// HYS-1221
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Do the loop on the different tables.
	for( unsigned int i = 0; i < vTab.size(); i++ )
	{
		CTable *pTab = static_cast<CTable *>( Get( vTab[i].c_str() ).MP );
		ASSERT( NULL != pTab );

		// Do the loop on the valves of the current table.
		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// Keep only CDB_RegulatingValve and children.
			CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( IDPtr.MP );

			if( NULL == pRv )
			{
				continue;
			}

#ifndef TACBX
			bForSet = false;
#endif

			// Exclude if we need only set and if this valve not belonging to a set.
			if( true == bOnlySet && false == pRv->IsPartOfaSet() )
			{
				continue;
			}
#ifndef TACBX
			else if( true == bOnlySet && true == pRv->IsPartOfaSet() )
			{
				bForSet = true;
			}
#endif

			// Test if product is selectable.
			if( false == pRv->IsSelectable( true, bForHubStation, bOldProductsAvailableForHC ) )
			{
				continue;
			}

			if( true == MustExclude( pRv, eFilterSelection ) )
			{
				continue;
			}

			// Test Cv2W3W and CVFunc.
			if( false == pRv->IsValidCV( eCv2W3W, eCVFunc ) )
			{
				continue;
			}

			// Test CtrlType.
			if( eCvCtrlType != CDB_ControlProperties::eCvNU && false == pRv->GetCtrlProp()->CvCtrlTypeFits( eCvCtrlType ) )
			{
				continue;
			}

			if( eFilterSelection == ForHMCalc )
			{
				CDB_ControlProperties *pCtrlProp = dynamic_cast<CDB_ControlProperties *>( pRv->GetCtrlProp() );

				if( NULL == pCtrlProp )
				{
					continue;
				}

				if( CDB_ControlProperties::PresetPT == pCtrlProp->GetCvFunc() || CDB_ControlProperties::Presettable == pCtrlProp->GetCvFunc() )
				{
					// To exclude valves without characteristics .... KTM50
					CDB_ValveCharacteristic *pValvChar = pRv->GetValveCharacteristic();

					if( NULL == pValvChar || false == pValvChar->HasKvCurve() )
					{
						continue;
					}
				}
			}

			// Test SelTypeID.
			if( false == strSelTypeID.empty() && strSelTypeID != pRv->GetTypeIDPtr().ID )
			{
				continue;
			}

			// Test SelFamID.
			if( false == strSelFamID.empty() && strSelFamID != pRv->GetFamilyIDPtr().ID )
			{
				continue;
			}

			// Test SelBdyMatID.
			if( false == strSelBdyMatID.empty() && strSelBdyMatID != pRv->GetBodyMaterialIDPtr().ID )
			{
				continue;
			}

			// Test SelConnectID.
			if( false == strSelConnectID.empty() && strSelConnectID != pRv->GetConnectIDPtr().ID )
			{
				continue;
			}

			// Test SelVersionID.
			if( false == strSelVersionID.empty() && strSelVersionID != pRv->GetVersionIDPtr().ID )
			{
				continue;
			}

			// Test SelPNID.
			if( false == strSelPNID.empty() )
			{
				if( false == pRv->IsPNIncludedInPNRange( strSelPNID ) )
				{
					continue;
				}
			}

			// Test DN.
			int iDN = pRv->GetSizeKey();

			if( iDN < iDNMin )
			{
				continue;
			}

			if( iDN > iDNMax )
			{
				continue;
			}

#ifndef TACBX
			// HYS-1112: Check that the Set is complete before proposing the product
			if( true == bForSet )
			{
				// HYS-1877: Manage 6-way valve set
				CDB_6WayValve* pcl6WayValve = dynamic_cast<CDB_6WayValve*>(IDPtr.MP);
				// Verify that there is an actuator available for this product in a Set
				
				CTableSet *pSetTab = (CTableSet *)( pRv->GetTableSet() );
				CTable *pSignalsTab = static_cast<CTable *>( Get( _T("SIG_TAB") ).MP );
				if( NULL != pSetTab )
				{
					bool bActuator_found = false;
					bool b6WayValveSetCheck_OK = true;
					// HYS-1877: For PIBCV set linked with 6-way valve set we have to use FindCompatibleSet6WayValve.
					CDB_Set* pclPIBCVSetFor6WValve = pSetTab->FindCompatibleSet6WayValve( pRv->GetIDPtr().ID, _T( "" ) );
					std::set<CDB_Set *> BCVSet;
					std::set<CDB_Set *>::const_iterator It;
					if( pSetTab->FindCompatibleSet( &BCVSet, pRv->GetIDPtr().ID, _T( "" ) ) 
						|| NULL != pclPIBCVSetFor6WValve )
					{
						if( NULL != pclPIBCVSetFor6WValve )
						{
							// HYS-1877: There is one PIBCV set for 6-way valve
							BCVSet.clear();
							BCVSet.insert( pclPIBCVSetFor6WValve );
						}
						for( It = BCVSet.begin(); It != BCVSet.end(); ++It )
						{
							// Find matching actuator.
							CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator*>( ( *It )->GetSecondIDPtr().MP );
							if( NULL == pclElectroActuator )
							{
								continue;
							}
							for( int iLoop = 0; iLoop < (int)pclElectroActuator->GetNumOfInputSignalsIDptr(); iLoop++ )
							{
								IDPTR InputSignalIDPtr = pclElectroActuator->GetInputSignalsIDPtr( iLoop );
								CDB_StringID *pclActrInputSignal = dynamic_cast<CDB_StringID *>( pSignalsTab->Get( InputSignalIDPtr.ID ).MP );

								if( NULL == pclActrInputSignal )
								{
									continue;
								}

								CDB_ControlProperties::CvCtrlType InputSignalType = ( CDB_ControlProperties::CvCtrlType )_ttoi( pclActrInputSignal->GetIDstr() );
								
								// HYS-1381: some 3-point or/and proportional actuators can work in On/Off mode.
								// In this case the "OnOffPossible" variable in the database is set to 1.
								if( eCvCtrlType == InputSignalType || eCvCtrlType == CDB_ControlProperties::eCvNU
										|| ( eCvCtrlType != InputSignalType && CDB_ControlProperties::CvCtrlType::eCvOnOff == eCvCtrlType 
										&& true == pclElectroActuator->IsOnOffPossible() ) )
								{
									bActuator_found = true;
									break;
								}
							}
							if( true == bActuator_found )
							{
								// HYS-1877: verify if we can find a PIBCV
								if( NULL == pcl6WayValve || NULL == pcl6WayValve->GetPicvGroupTableIDPtr().MP )
								{
									break;
								}
								// Actuator found, now verify PIBCV
								bool bPibcvFound = false;
								b6WayValveSetCheck_OK = false;
								CDB_Set* pclPibcvSet = dynamic_cast<CDB_Set*>(Get( (*It)->GetNextID() ).MP);
								if( NULL != pclPibcvSet && NULL != dynamic_cast<CDB_PIControlValve*>(pclPibcvSet->GetFirstIDPtr().MP) 
									&& _NULL_IDPTR != dynamic_cast<CTable*>(pcl6WayValve->GetPicvGroupTableIDPtr().MP)->Get( pclPibcvSet->GetFirstIDPtr().ID ) )
								{
									bPibcvFound = true;
									b6WayValveSetCheck_OK = true;
								}

								if( true == bPibcvFound )
								{
									break;
								}
							}
						}
					}

					if( false == bActuator_found && true == b6WayValveSetCheck_OK )
					{
						continue;
					}
				}
			}
#endif

			// Already added ?
			double dKey = 0;
			_string str;
			LPARAM lparam = (LPARAM)IDPtr.MP;

			// Could be added by default.
			bool bCouldbeAdded = true;

			// Verify the lparam doesn't exist yet
			if( !pList->GetaCopy( str, dKey, lparam ) )
			{
				#ifdef TACBX
				//				if( CTADatabase::eForPiCv == eCvTarget )
				{
					bCouldbeAdded = CheckIfCharactAlreadyExist( pList, pRv, pProd, false );
				}
				#endif

				if( true == bCouldbeAdded )
				{
					// Found product having priority.
					double dKey = pRv->GetSortingKey();
					pList->Add( pRv->GetName(), dKey, (LPARAM)pRv );

					if( true == bRetIbFound )
					{
						return pList->GetCount();
					}
				}
			}
		}
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetTaCVActuatorPowerSupplyList( CRankEx *pPowerSupplyList, CRankEx *pCVList, CDB_ControlProperties::CvCtrlType eCvCtrlType, bool bOnlyPackage, FilterSelection eFilterSelection )
{
	if( NULL == pPowerSupplyList || NULL == pCVList || 0 == pCVList->GetCount() )
	{
		return 0;
	}

	CTable *pPowerSupplyTab = static_cast<CTable *>( Get( _T("POWSUPPLY_TAB") ).MP );

	if( NULL == pPowerSupplyTab )
	{
		return 0;
	}

	pPowerSupplyList->PurgeAll();

	// First pass: run all control valves and retrieve actuator groups.
	_string str;
	LPARAM lParam;
	std::map<int, int> mapActrKeyList;
	std::map<CTable *, int> mapActrGpList;
	std::vector<CDB_ControlValve *> vecCVPackage;

	for( BOOL bContinue = pCVList->GetFirst( str, lParam ); TRUE == bContinue; bContinue = pCVList->GetNext( str, lParam ) )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( ( CData * )lParam );

		if( NULL == pclControlValve )
		{
			continue;
		}

		// If we want only package and control valve doesn't belong to a set we bypass this one.
		if( true == bOnlyPackage )
		{
			if( false == pclControlValve->IsPartOfaSet() )
			{
				continue;
			}

			vecCVPackage.push_back( pclControlValve );
		}

		// Retrieve actuator group.
		IDPTR ActrGpIDPtr = pclControlValve->GetActuatorGroupIDPtr();

		if( NULL == ActrGpIDPtr.MP || 0 == *ActrGpIDPtr.ID )
		{
			continue;
		}

		CTable *pclActrTab = dynamic_cast<CTable *>( ActrGpIDPtr.MP );

		if( NULL == pclActrTab )
		{
			continue;
		}

		if( mapActrGpList.count( pclActrTab ) > 0 )
		{
			continue;
		}

		mapActrGpList[pclActrTab] = 0;
	}

	if( 0 == mapActrGpList.size() )
	{
		return 0;
	}
	// Run alls entries for Ctrl type .
	CTable *pSignalsTab = static_cast<CTable *>( Get( _T("SIG_TAB") ).MP );
	// Second pass: run all actuator groups.
	for( std::map<CTable *, int>::iterator iter = mapActrGpList.begin(); iter != mapActrGpList.end(); ++iter )
	{
		CTable *pclActrTab = iter->first;

		// Run all actuators in this group.
		for( IDPTR ActrIDPtr = pclActrTab->GetFirst(); _T('\0') != *ActrIDPtr.ID; ActrIDPtr = pclActrTab->GetNext() )
		{
			CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ActrIDPtr.MP );

			if( NULL == pclElectroActuator )
			{
				continue;
			}

			if( false == pclElectroActuator->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( pclElectroActuator, eFilterSelection ) )
			{
				continue;
			}

			// If we want only package...
			if( true == bOnlyPackage )
			{
				// Run all control valves that has been saved just above (the vector contains thus only the valves that belong to a set).
				bool fValidActuator = false;

				for( std::vector<CDB_ControlValve *>::iterator iter = vecCVPackage.begin(); iter != vecCVPackage.end() && false == fValidActuator; ++iter )
				{
					// Retrieve the correct set table in regards to the control valve.
					CTableSet *pCVActTableSet = NULL;
					pCVActTableSet = ( *iter )->GetTableSet();

					if( NULL == pCVActTableSet )
					{
						continue;
					}

					// If there is not set with the current control valve and actuator, we continue searching.
					if( NULL == pCVActTableSet->FindCompatibleSet( ( *iter )->GetIDPtr().ID, pclElectroActuator->GetIDPtr().ID ) )
					{
						continue;
					}

					// As soon as we find one set containing both control valve and actuator, it's ok!
					fValidActuator = true;
				}

				if( false == fValidActuator )
				{
					continue;
				}
			}

			bool bOneFound = false;
			for( int iLoop = 0; iLoop < (int)pclElectroActuator->GetNumOfInputSignalsIDptr(); iLoop++ )
			{
				IDPTR InputSignalIDPtr = pclElectroActuator->GetInputSignalsIDPtr( iLoop );

				CDB_StringID *pclActrInputSignal = dynamic_cast<CDB_StringID *>( pSignalsTab->Get( InputSignalIDPtr.ID ).MP );

				if( NULL == pclActrInputSignal )
				{
					continue;
				}

				CDB_ControlProperties::CvCtrlType InputSignalType = ( CDB_ControlProperties::CvCtrlType )_ttoi( pclActrInputSignal->GetIDstr() );

				// HYS-1381: some 3-point or/and proportional actuators can work in On/Off mode.
				// In this case the "OnOffPossible" variable in the database is set to 1.
				if( eCvCtrlType != InputSignalType 
						&& ( CDB_ControlProperties::CvCtrlType::eCvOnOff != eCvCtrlType || false == pclElectroActuator->IsOnOffPossible() ) )
				{
					continue;
				}
				else
				{
					bOneFound = true;
				}
			}
			// The eCVCtrlType is not available for this actuator
			if( false == bOneFound )
			{
				continue;
			}

			// Run alls entries for power supply.
			for( int iLoop = 0; iLoop < ( int )pclElectroActuator->GetNumOfPowerSupplyIDptr(); iLoop++ )
			{
				IDPTR PowerSupplyIDPtr = pclElectroActuator->GetPowerSupplyIDPtr( iLoop );

				//CDB_StringID *pclActrPowerSupply = dynamic_cast<CDB_StringID*>( pPowwerSupplyTab->Get( PowerSupplyIDPtr.ID ).MP );
				CDB_StringID *pclActrPowerSupply = dynamic_cast<CDB_StringID *>( PowerSupplyIDPtr.MP );

				if( NULL == pclActrPowerSupply )
				{
					continue;
				}

				int iKey = _ttoi( pclActrPowerSupply->GetIDstr() );

				if( 0 == mapActrKeyList.count( iKey ) )
				{
					//pPowerSupplyList->Add( ( (CDB_StringID *)PowerSupplyIDPtr.MP )->GetString(), iKey, (LPARAM)(void *)PowerSupplyIDPtr.MP );
					pPowerSupplyList->Add( pclActrPowerSupply->GetString(), iKey, (LPARAM)(void *)PowerSupplyIDPtr.MP );
					mapActrKeyList[iKey] = iKey;
				}
			}
		}
	}

	return pPowerSupplyList->GetCount();
}

int CTADatabase::GetTaCVActuatorInputSignalList( CRankEx *pInputSignalList, CRankEx *pCVList, CDB_ControlProperties::CvCtrlType eCvCtrlType, bool bOnlyPackage,
		_string strSelPowerSupplyID, FilterSelection eFilterSelection )
{
	if( NULL == pInputSignalList || NULL == pCVList || 0 == pCVList->GetCount() )
	{
		return 0;
	}

	CTable *pSignalsTab = static_cast<CTable *>( Get( _T("SIG_TAB") ).MP );

	if( NULL == pSignalsTab )
	{
		return 0;
	}

	pInputSignalList->PurgeAll();

	// First pass: run all control valves and retrieve actuator groups.
	_string str;
	LPARAM lParam;
	std::map<int, int> mapActrKeyList;
	std::map<CTable *, int> mapActrGpList;
	std::vector<CDB_ControlValve *> vecCVPackage;

	for( BOOL bContinue = pCVList->GetFirst( str, lParam ); TRUE == bContinue; bContinue = pCVList->GetNext( str, lParam ) )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( ( CData * )lParam );

		if( NULL == pclControlValve )
		{
			continue;
		}

		// If we want only package and control valve doesn't belong to a set we bypass this one.
		if( true == bOnlyPackage )
		{
			if( false == pclControlValve->IsPartOfaSet() )
			{
				continue;
			}

			vecCVPackage.push_back( pclControlValve );
		}

		// Retrieve actuator group.
		IDPTR ActrGpIDPtr = pclControlValve->GetActuatorGroupIDPtr();

		if( NULL == ActrGpIDPtr.MP || 0 == *ActrGpIDPtr.ID )
		{
			continue;
		}

		CTable *pclActrTab = dynamic_cast<CTable *>( Get( ActrGpIDPtr.ID ).MP );

		if( NULL == pclActrTab )
		{
			continue;
		}

		if( mapActrGpList.count( pclActrTab ) > 0 )
		{
			continue;
		}

		mapActrGpList[pclActrTab] = 0;
	}

	if( 0 == mapActrGpList.size() )
	{
		return 0;
	}

	// Second pass: run all actuator groups.
	for( std::map<CTable *, int>::iterator iter = mapActrGpList.begin(); iter != mapActrGpList.end(); ++iter )
	{
		CTable *pclActrTab = iter->first;

		// Run all actuators in this group.
		for( IDPTR ActrIDPtr = pclActrTab->GetFirst(); 0 != *ActrIDPtr.ID; ActrIDPtr = pclActrTab->GetNext() )
		{
			CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ActrIDPtr.MP );

			if( NULL == pclElectroActuator )
			{
				continue;
			}

			if( false == pclElectroActuator->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( pclElectroActuator, eFilterSelection ) )
			{
				continue;
			}

			// If we want only package...
			if( true == bOnlyPackage )
			{
				// Run all control valves that has been saved just above (the vector contains thus only the valves that belong to a set).
				bool bValidActuator = false;

				for( std::vector<CDB_ControlValve *>::iterator iter = vecCVPackage.begin(); iter != vecCVPackage.end() && false == bValidActuator; ++iter )
				{
					// Retrieve the correct set table in regards to the control valve.
					CTableSet *pCVActTableSet = NULL;
					pCVActTableSet = ( *iter )->GetTableSet();

					if( NULL == pCVActTableSet )
					{
						continue;
					}

					// If there is not set with the current control valve and actuator, we continue searching.
					if( NULL == pCVActTableSet->FindCompatibleSet( ( *iter )->GetIDPtr().ID, pclElectroActuator->GetIDPtr().ID ) )
					{
						continue;
					}

					// As soon as we find one set containing both control valve and actuator, it's ok!
					bValidActuator = true;
				}

				if( false == bValidActuator )
				{
					continue;
				}
			}

			// Test power supply ID.
			bool bFound = true;

			if( false == strSelPowerSupplyID.empty() )
			{
				bFound = false;

				// Run alls entries for power supply.
				for( int iLoop = 0; iLoop < ( int )pclElectroActuator->GetNumOfPowerSupplyIDptr(); iLoop++ )
				{
					IDPTR PowerSupplyIDPtr = pclElectroActuator->GetPowerSupplyIDPtr( iLoop );

					if( strSelPowerSupplyID == PowerSupplyIDPtr.ID )
					{
						bFound = true;
						break;
					}
				}
			}

			if( false == bFound )
			{
				continue;
			}

			// Run alls entries for input signal.
			for( int iLoop = 0; iLoop < ( int )pclElectroActuator->GetNumOfInputSignalsIDptr(); iLoop++ )
			{
				IDPTR InputSignalIDPtr = pclElectroActuator->GetInputSignalsIDPtr( iLoop );

				CDB_StringID *pclActrInputSignal = dynamic_cast<CDB_StringID *>( pSignalsTab->Get( InputSignalIDPtr.ID ).MP );

				if( NULL == pclActrInputSignal )
				{
					continue;
				}

				CDB_ControlProperties::CvCtrlType InputSignalType = ( CDB_ControlProperties::CvCtrlType )_ttoi( pclActrInputSignal->GetIDstr() );

				// HYS-1381: some 3-point or/and proportional actuators can work in On/Off mode.
				// In this case the "OnOffPossible" variable in the database is set to 1.
				if( eCvCtrlType != InputSignalType 
						&& ( CDB_ControlProperties::CvCtrlType::eCvOnOff != eCvCtrlType || false == pclElectroActuator->IsOnOffPossible() ) )
				{
					continue;
				}

				int iKey = _ttoi( pclActrInputSignal->GetIDstr2() );

				if( 0 == mapActrKeyList.count( iKey ) )
				{
					CString str = ( (CDB_StringID *)InputSignalIDPtr.MP )->GetString();
					str.Replace( _T("%1" ), _T( "") );
					pInputSignalList->Add( ( LPCTSTR )str, iKey, (LPARAM)(void *)InputSignalIDPtr.MP );
					mapActrKeyList[iKey] = iKey;
				}
			}
		}
	}

	return pInputSignalList->GetCount();
}

int CTADatabase::GetTaCVActuatorFailSafeValues( CRankEx * pCVList, CDB_ControlProperties::CvCtrlType eCvCtrlType, bool bOnlyPackage, _string strSelPowerSupplyID, _string strSelInputSignalID, FilterSelection eFilterSelection )
{
	int iReturn = -1;
	if( NULL == pCVList || 0 == pCVList->GetCount() )
	{
		return iReturn;
	}

	_string str;
	LPARAM lParam;
	bool bAtLeastOneWithFailSafe = false;
	bool bAtLeastOneWithoutFailSafe = false;
	bool bCanStop = false;

	for( BOOL bContinue = pCVList->GetFirst( str, lParam ); TRUE == bContinue && false == bCanStop; bContinue = pCVList->GetNext( str, lParam ) )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( (CData *)lParam );

		if( NULL == pclControlValve )
		{
			continue;
		}

		// If we want only package and control valve doesn't belong to a set we bypass this one.
		if( true == bOnlyPackage && false == pclControlValve->IsPartOfaSet() )
		{
			continue;
		}

		// Retrieve actuator group.
		IDPTR ActrGpIDPtr = pclControlValve->GetActuatorGroupIDPtr();

		if( NULL == ActrGpIDPtr.MP || 0 == *ActrGpIDPtr.ID )
		{
			continue;
		}

		CTable *pclActrTab = dynamic_cast<CTable *>( Get( ActrGpIDPtr.ID ).MP );

		if( NULL == pclActrTab )
		{
			continue;
		}

		// Run all actuators in this group.
		for( IDPTR ActrIDPtr = pclActrTab->GetFirst(); _T( '\0' ) != *ActrIDPtr.ID && false == bCanStop; ActrIDPtr = pclActrTab->GetNext() )
		{
			CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ActrIDPtr.MP );

			if( NULL == pclElectroActuator )
			{
				continue;
			}

			if( false == pclElectroActuator->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( pclElectroActuator, eFilterSelection ) )
			{
				continue;
			}
			// If we want only package...
			if( true == bOnlyPackage )
			{
				CTableSet *pCVActTableSet = NULL;
				pCVActTableSet = pclControlValve->GetTableSet();
				if( NULL != pCVActTableSet )
				{
					// If there is not set with the current control valve and actuator, we continue searching.
					if( NULL == pCVActTableSet->FindCompatibleSet( pclControlValve->GetIDPtr().ID, pclElectroActuator->GetIDPtr().ID ) )
					{
						continue;
					}
				}
			}
			// Test power supply ID.
			bool bFound = true;

			if( false == strSelPowerSupplyID.empty() )
			{
				// Run alls entries for power supply.
				bFound = false;

				for( int iLoop = 0; iLoop < (int)pclElectroActuator->GetNumOfPowerSupplyIDptr(); iLoop++ )
				{
					IDPTR PowerSupplyIDPtr = pclElectroActuator->GetPowerSupplyIDPtr( iLoop );

					if( strSelPowerSupplyID == PowerSupplyIDPtr.ID )
					{
						bFound = true;
						break;
					}
				}
			}

			if( false == bFound )
			{
				continue;
			}

			// Test input signal ID.
			if( false == strSelInputSignalID.empty() )
			{
				// Run alls entries for input signal.
				bFound = false;

				for( int iLoop = 0; iLoop < (int)pclElectroActuator->GetNumOfInputSignalsIDptr(); iLoop++ )
				{
					IDPTR InputSignalIDPtr = pclElectroActuator->GetInputSignalsIDPtr( iLoop );

					if( strSelInputSignalID == InputSignalIDPtr.ID )
					{
						bFound = true;
						break;
					}
				}
			}
			else
			{
				// Run alls entries for Ctrl type .
				CTable *pSignalsTab = static_cast<CTable *>( Get( _T("SIG_TAB") ).MP );
				bool bOneFound = false;
				for( int iLoop = 0; iLoop < (int)pclElectroActuator->GetNumOfInputSignalsIDptr(); iLoop++ )
				{
					IDPTR InputSignalIDPtr = pclElectroActuator->GetInputSignalsIDPtr( iLoop );

					CDB_StringID *pclActrInputSignal = dynamic_cast<CDB_StringID *>( pSignalsTab->Get( InputSignalIDPtr.ID ).MP );

					if( NULL == pclActrInputSignal )
					{
						continue;
					}

					CDB_ControlProperties::CvCtrlType InputSignalType = ( CDB_ControlProperties::CvCtrlType )_ttoi( pclActrInputSignal->GetIDstr() );

					// HYS-1381: some 3-point or/and proportional actuators can work in On/Off mode.
					// In this case the "OnOffPossible" variable in the database is set to 1.
					if( eCvCtrlType != InputSignalType 
							&& ( CDB_ControlProperties::CvCtrlType::eCvOnOff != eCvCtrlType || false == pclElectroActuator->IsOnOffPossible() ) )
					{
						continue;
					}
					else
					{
						bOneFound = true;
					}
				}
				// The eCVCtrlType is not available for this actuator
				if( false == bOneFound )
				{
					bFound = false;
				}
			}
			if( false == bFound )
			{
				continue;
			}

			if( CDB_ElectroActuator::FailSafeType::eFSTypeElectronic <= pclElectroActuator->GetFailSafe() 
				&& CDB_ElectroActuator::FailSafeType::eFSTypeLast > pclElectroActuator->GetFailSafe() )
			{
				bAtLeastOneWithFailSafe = true;
			}
			else
			{
				bAtLeastOneWithoutFailSafe = true;
			}

			if( true == bAtLeastOneWithoutFailSafe && true == bAtLeastOneWithFailSafe )
			{
				bCanStop = true;
			}
		}
	}

	if( true == bAtLeastOneWithoutFailSafe && false == bAtLeastOneWithFailSafe )
	{
		iReturn = eOnlyWithoutFailSafe;
	}
	else if( false == bAtLeastOneWithoutFailSafe && true == bAtLeastOneWithFailSafe )
	{
		iReturn = eOnlyWithFailSafe;
	}
	else if( true == bAtLeastOneWithoutFailSafe && true == bAtLeastOneWithFailSafe )
	{
		iReturn = eBothFailSafe;
	}
	else
	{
		iReturn = eFirst;
	}

	return iReturn;
}

int CTADatabase::GetTaCVActuatorList( CRankEx *pActuatorList, CDB_ControlValve *pclControlValve, bool bOnlyPackage, _string strSelPowerSupplyID,
		_string strSelInputSignalID, int iFailSafe, _string strSetDRPID, FilterSelection eFilterSelection )
{
	if( NULL == pActuatorList || NULL == pclControlValve )
	{
		return 0;
	}

	pActuatorList->PurgeAll();

	if( true == bOnlyPackage && false == pclControlValve->IsPartOfaSet() )
	{
		return 0;
	}

	// Retrieve actuator group.
	IDPTR ActuatorGroupIDPtr = pclControlValve->GetActuatorGroupIDPtr();

	if( NULL == ActuatorGroupIDPtr.MP || '\0' == *ActuatorGroupIDPtr.ID )
	{
		return 0;
	}

	CTable *pclActuatorTab = dynamic_cast<CTable *>( Get( ActuatorGroupIDPtr.ID ).MP );

	if( NULL == pclActuatorTab )
	{
		return 0;
	}

	// Run all actuators in the group.
	for( IDPTR ActuatorIDPtr = pclActuatorTab->GetFirst(); ActuatorIDPtr.MP != NULL; ActuatorIDPtr = pclActuatorTab->GetNext() )
	{
		CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ActuatorIDPtr.MP );

		if( NULL == pclElectroActuator )
		{
			continue;
		}

		if( false == pclElectroActuator->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclElectroActuator, eFilterSelection ) )
		{
			continue;
		}

		// If we want only package...
		if( true == bOnlyPackage )
		{
			// Retrieve the correct set table in regards to the control valve.
			bool bValidActuator = false;
			CTableSet *pCVActTableSet = NULL;
			pCVActTableSet = pclControlValve->GetTableSet();

			if( NULL == pCVActTableSet )
			{
				continue;
			}

			// If there is not set with the current control valve and actuator, we continue searching.
			if( NULL == pCVActTableSet->FindCompatibleSet( pclControlValve->GetIDPtr().ID, pclElectroActuator->GetIDPtr().ID ) )
			{
				continue;
			}
		}

		// Test power supply ID.
		bool bFound = true;

		if( false == strSelPowerSupplyID.empty() )
		{
			// Run alls entries for power supply.
			bFound = false;

			for( int iLoop = 0; iLoop < ( int )pclElectroActuator->GetNumOfPowerSupplyIDptr(); iLoop++ )
			{
				IDPTR PowerSupplyIDPtr = pclElectroActuator->GetPowerSupplyIDPtr( iLoop );

				if( strSelPowerSupplyID == PowerSupplyIDPtr.ID )
				{
					bFound = true;
					break;
				}
			}
		}

		if( false == bFound )
		{
			continue;
		}

		// Test input signal ID.
		if( false == strSelInputSignalID.empty() )
		{
			// Run alls entries for input signal.
			bFound = false;

			for( int iLoop = 0; iLoop < ( int )pclElectroActuator->GetNumOfInputSignalsIDptr(); iLoop++ )
			{
				IDPTR InputSignalIDPtr = pclElectroActuator->GetInputSignalsIDPtr( iLoop );

				if( strSelInputSignalID == InputSignalIDPtr.ID )
				{
					bFound = true;
					break;
				}
			}
		}

		if( false == bFound )
		{
			continue;
		}

		// HYS-1458 : If fail-safe checkbox is checked we have to get a fail-safe type. If the checkbox is not checked we have to get No fail-safe. 
		// if these conditions are not satisfied, actuator is not fit.
		if( -1 != iFailSafe && ( ( pclElectroActuator->GetFailSafe() >= CDB_ElectroActuator::FailSafeType::eFSTypeElectronic 
			&& pclElectroActuator->GetFailSafe() < CDB_ElectroActuator::FailSafeType::eFSTypeLast && iFailSafe != 1 )
			|| ( (int)pclElectroActuator->GetFailSafe() == 0 && iFailSafe != 0 ) ) )
		{
			continue;
		}

		// Test Default return position.
		if( false == strSetDRPID.empty() )
		{
			bFound = false;

			if( 0 == _tcscmp( pclElectroActuator->GetDefaultReturnPosStr( pclElectroActuator->GetDefaultReturnPos() ).c_str(), strSetDRPID.c_str() ) )
			{
				bFound = true;
			}
		}

		if( true == bFound )
		{
			double dKey = pclElectroActuator->GetMaxForceTorque();
			pActuatorList->Add( ( LPCTSTR )pclElectroActuator->GetName(), dKey, (LPARAM)pclElectroActuator, true );
		}
	}

	return pActuatorList->GetCount();
}

int CTADatabase::GetTaCVDRPFunctionList( CRankEx *pDRPFctList, CDB_ControlProperties::CvCtrlType eCvCtrlType, CRankEx *pCVList, bool bOnlyPackage, _string strSelPowerSupplyID,
		_string strSelInputSignalID, int iFailSafe, FilterSelection eFilterSelection )
{
	if( NULL == pDRPFctList || NULL == pCVList || 0 == pCVList->GetCount() )
	{
		return 0;
	}

	pDRPFctList->PurgeAll();

	_string str;
	LPARAM lParam;
	bool bAtLeastOneOpening = false;
	bool bAtLeastOneClosing = false;
	bool bAtLeastOneConfigurable = false;
	bool bAtLeastOneNone = false;
	bool bCanStop = false;
	CDB_ControlValve *pclOneControlValve = NULL;

	for( BOOL bContinue = pCVList->GetFirst( str, lParam ); TRUE == bContinue && false == bCanStop; bContinue = pCVList->GetNext( str, lParam ) )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( (CData *)lParam );

		if( NULL == pclControlValve )
		{
			continue;
		}

		if( NULL == pclOneControlValve )
		{
			pclOneControlValve = pclControlValve;
		}

		// If we want only package and control valve doesn't belong to a set we bypass this one.
		if( true == bOnlyPackage && false == pclControlValve->IsPartOfaSet() )
		{
			continue;
		}

		// Retrieve actuator group.
		IDPTR ActrGpIDPtr = pclControlValve->GetActuatorGroupIDPtr();

		if( NULL == ActrGpIDPtr.MP || 0 == *ActrGpIDPtr.ID )
		{
			continue;
		}

		CTable *pclActrTab = dynamic_cast<CTable *>( Get( ActrGpIDPtr.ID ).MP );

		if( NULL == pclActrTab )
		{
			continue;
		}

		// Run all actuators in this group.
		for( IDPTR ActrIDPtr = pclActrTab->GetFirst(); _T('\0') != *ActrIDPtr.ID && false == bCanStop; ActrIDPtr = pclActrTab->GetNext() )
		{
			CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ActrIDPtr.MP );

			if( NULL == pclElectroActuator )
			{
				continue;
			}

			if( false == pclElectroActuator->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( pclElectroActuator, eFilterSelection ) )
			{
				continue;
			}

			// If we want only package...
			if( true == bOnlyPackage )
			{
				CTableSet *pCVActTableSet = NULL;
				pCVActTableSet = pclControlValve->GetTableSet();
				if( NULL != pCVActTableSet )
				{
					// If there is not set with the current control valve and actuator, we continue searching.
					if( NULL == pCVActTableSet->FindCompatibleSet( pclControlValve->GetIDPtr().ID, pclElectroActuator->GetIDPtr().ID ) )
					{
						continue;
					}
				}
			}

			// Test power supply ID.
			bool bFound = true;

			if( false == strSelPowerSupplyID.empty() )
			{
				// Run alls entries for power supply.
				bFound = false;

				for( int iLoop = 0; iLoop < ( int )pclElectroActuator->GetNumOfPowerSupplyIDptr(); iLoop++ )
				{
					IDPTR PowerSupplyIDPtr = pclElectroActuator->GetPowerSupplyIDPtr( iLoop );

					if( strSelPowerSupplyID == PowerSupplyIDPtr.ID )
					{
						bFound = true;
						break;
					}
				}
			}

			if( false == bFound )
			{
				continue;
			}

			// Test input signal ID.
			if( false == strSelInputSignalID.empty() )
			{
				// Run alls entries for input signal.
				bFound = false;

				for( int iLoop = 0; iLoop < ( int )pclElectroActuator->GetNumOfInputSignalsIDptr(); iLoop++ )
				{
					IDPTR InputSignalIDPtr = pclElectroActuator->GetInputSignalsIDPtr( iLoop );

					if( strSelInputSignalID == InputSignalIDPtr.ID )
					{
						bFound = true;
						break;
					}
				}
			}
			else
			{
				// Run alls entries for Ctrl type .
				CTable *pSignalsTab = static_cast<CTable *>( Get( _T("SIG_TAB") ).MP );
				bool bOneFound = false;
				for( int iLoop = 0; iLoop < (int)pclElectroActuator->GetNumOfInputSignalsIDptr(); iLoop++ )
				{
					IDPTR InputSignalIDPtr = pclElectroActuator->GetInputSignalsIDPtr( iLoop );

					CDB_StringID *pclActrInputSignal = dynamic_cast<CDB_StringID *>( pSignalsTab->Get( InputSignalIDPtr.ID ).MP );

					if( NULL == pclActrInputSignal )
					{
						continue;
					}
					CDB_ControlProperties::CvCtrlType InputSignalType = ( CDB_ControlProperties::CvCtrlType )_ttoi( pclActrInputSignal->GetIDstr() );

					// HYS-1381: some 3-point or/and proportional actuators can work in On/Off mode.
					// In this case the "OnOffPossible" variable in the database is set to 1.
					if( eCvCtrlType != InputSignalType 
							&& ( CDB_ControlProperties::CvCtrlType::eCvOnOff != eCvCtrlType || false == pclElectroActuator->IsOnOffPossible() ) )
					{
						continue;
					}
					else
					{
						bOneFound = true;
					}
				}
				// The eCVCtrlType is not available for this actuator
				if( false == bOneFound )
				{
					bFound = false;
				}
			}

			if( false == bFound )
			{
				continue;
			}

			// Fail-safe function
			// HYS-1458 : If fail-safe checkbox is checked we have to get a fail-safe type. If the checkbox is not checked we have to get No fail-safe. 
			// if these conditions are not satisfied, actuator is not fit.
			if( -1 != iFailSafe && ( ( pclElectroActuator->GetFailSafe() >= CDB_ElectroActuator::FailSafeType::eFSTypeElectronic 
				&& pclElectroActuator->GetFailSafe() < CDB_ElectroActuator::FailSafeType::eFSTypeLast && iFailSafe != 1 )
				|| ( (int)pclElectroActuator->GetFailSafe() == 0 && iFailSafe != 0 ) ) )
			{
				continue;
			}

			// Check what is the compatible DRP function for the current control valve and actuator.
			switch( pclControlValve->GetCompatibleDRPFunction( ( int )pclElectroActuator->GetDefaultReturnPos() ) )
			{
				case CDB_ControlValve::DRPFunction::drpfOpening:
					bAtLeastOneOpening = true;
					break;

				case CDB_ControlValve::DRPFunction::drpfClosing:
					bAtLeastOneClosing = true;
					break;

				case CDB_ControlValve::DRPFunction::drpfOpeningOrClosing:
					bAtLeastOneConfigurable = true;
					break;
				case CDB_ControlValve::DRPFunction::drpfNone:
					bAtLeastOneNone = true;
			}

			if( true == bAtLeastOneOpening && true == bAtLeastOneClosing && true == bAtLeastOneConfigurable && true == bAtLeastOneNone )
			{
				bCanStop = true;
			}
		}
	}

	// By default we have always the 'None' possibility.
	if( true == bAtLeastOneNone )
	{
		_string str = GetDRPFunctionStr( CDB_ControlValve::DRPFunction::drpfNone );
		pDRPFctList->Add( str.c_str(), CDB_ControlValve::DRPFunction::drpfNone, CDB_ControlValve::DRPFunction::drpfNone );
	}

	if( true == bAtLeastOneOpening )
	{
		_string str = GetDRPFunctionStr( CDB_ControlValve::DRPFunction::drpfOpening );
		pDRPFctList->Add( str.c_str(), CDB_ControlValve::DRPFunction::drpfOpening, CDB_ControlValve::DRPFunction::drpfOpening );
	}

	if( true == bAtLeastOneClosing )
	{
		_string str = GetDRPFunctionStr( CDB_ControlValve::DRPFunction::drpfClosing );
		pDRPFctList->Add( str.c_str(), CDB_ControlValve::DRPFunction::drpfClosing, CDB_ControlValve::DRPFunction::drpfClosing );
	}

	if( true == bAtLeastOneConfigurable )
	{
		_string str = GetDRPFunctionStr( CDB_ControlValve::DRPFunction::drpfOpeningOrClosing );
		pDRPFctList->Add( str.c_str(), CDB_ControlValve::DRPFunction::drpfOpeningOrClosing, CDB_ControlValve::DRPFunction::drpfOpeningOrClosing );
	}

	return pDRPFctList->GetCount();
}

CString CTADatabase::GetDRPFunctionStr( CDB_ControlValve::DRPFunction eDRPFunction )
{
	CString str = TASApp.LoadLocalizedString( IDS_DRPFUNCTION_NONE );

	switch( eDRPFunction )
	{
		case CDB_ControlValve::DRPFunction::drpfClosing:
			str = TASApp.LoadLocalizedString( IDS_DRPFUNCTION_CLOSING );
			break;

		case CDB_ControlValve::DRPFunction::drpfOpening:
			str = TASApp.LoadLocalizedString( IDS_DRPFUNCTION_OPENING );
			break;

		case CDB_ControlValve::DRPFunction::drpfOpeningOrClosing:
			str = TASApp.LoadLocalizedString( IDS_DRPFUNCTION_CONFIG );
			break;

		case CDB_ControlValve::DRPFunction::drpfNone:
		default:
			break;
	}

	return str;
}

int CTADatabase::GetActuator( CRank *pList, CDB_ControlValve *pCV, bool bWorkingForASet, FilterSelection eFilterSelection )
{
	std::set<CDB_Set *> ActrSet;
	std::set<CDB_Set *>::const_iterator It;

	if( NULL == pCV )
	{
		return 0;
	}

	if( true == bWorkingForASet && true == pCV->IsPartOfaSet() )
	{
		// Establish a list of available set for this control valve.
		CTableSet *pclSetTable = pCV->GetTableSet();
		ASSERT( NULL != pclSetTable );

		// HYS-1381 : For Set in drect selection we have to show compatible actuators even if it's deleted.
		if( pclSetTable->FindCompatibleSet( &ActrSet, pCV->GetIDPtr().ID, _T( "" ), eFilterSelection ) )
		{
			for( It = ActrSet.begin(); It != ActrSet.end(); ++It )
			{
				// Store matching actuator.
				CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( ( *It )->GetSecondIDPtr().MP );

				if( NULL == pclActuator )
				{
					continue;
				}

				if( true == MustExclude( pclActuator, eFilterSelection ) )
				{
					continue;
				}

				pList->Add( pclActuator->GetIDPtr().ID, pclActuator->GetOrderKey(), (LPARAM)pclActuator, false, false );
			}

			return pList->GetCount();
		}

		ASSERT( pList->GetCount() );
	}

	CTable *pTab = dynamic_cast<CTable *>( pCV->GetActuatorGroupIDPtr().MP );

	if( NULL != pTab )
	{
		GetActuator( pList, pTab );
	}

	return pList->GetCount();
}

int CTADatabase::GetActuator( CRank *pList, CTable *pTabActrLst, _string strSelPowerSupplyID, _string strSelInputSignalID,
		int iFailSafe, CDB_ElectroActuator::DefaultReturnPosition eDRP, FilterSelection eFilterSelection )
{
	if( NULL == pList || NULL == pTabActrLst )
	{
		return 0;
	}

	CRank rkList;
	rkList.FillFromTable( pTabActrLst );
	return GetActuator( pList, &rkList, strSelPowerSupplyID, strSelInputSignalID, iFailSafe, eDRP, eFilterSelection );
}

int CTADatabase::GetActuator( CRank *pList, CRank *pRankActrLst, _string strSelPowerSupplyID,
 _string strSelInputSignalID, int iFailSafe, CDB_ElectroActuator::DefaultReturnPosition eDRP, FilterSelection eFilterSelection )
{
	VERIFY( NULL != pRankActrLst );

	CDB_StringID *pStrIDPowerSupply = NULL;

	if( false == strSelPowerSupplyID.empty() )
	{
		pStrIDPowerSupply = dynamic_cast<CDB_StringID *>( ( CData * ) Get( strSelPowerSupplyID.c_str() ).MP );
	}

	// Current selected input signal.
	CDB_StringID *pStrInputSignal = NULL;

	if( false == strSelInputSignalID.empty() )
	{
		pStrInputSignal = dynamic_cast<CDB_StringID *>( ( CData * ) Get( strSelInputSignalID.c_str() ).MP );
	}

	int iCount = 0;

	if( NULL != pRankActrLst )
	{
		CString str;
		LPARAM lParam;

		// Extract list of power supply.
		for( BOOL bContinue = pRankActrLst->GetFirst( str, lParam ); TRUE == bContinue; bContinue = pRankActrLst->GetNext( str, lParam ) )
		{
			CDB_Actuator *pActr = dynamic_cast<CDB_Actuator *>( ( CData * ) lParam );
			ASSERT( NULL != pActr );
			CDB_ElectroActuator *pElecActr = dynamic_cast<CDB_ElectroActuator *>( pActr );
			// HYS-1193 : don't do 'continue' if pElecActr is NULL because we have also thermostatic actuator 
			if( NULL == pActr )
			{
				continue;
			}

			if( false == pActr->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( pActr, eFilterSelection ) )
			{
				continue;
			}

			if( NULL != pElecActr && NULL != pStrIDPowerSupply )
			{
				// Check.
				if( false == pElecActr->IsPowerSupplyAvailable( pStrIDPowerSupply->GetIDPtr() ) )
				{
					continue;
				}
			}

			if( NULL != pElecActr && NULL != pStrInputSignal )
			{
				// Check.
				if( false == pElecActr->IsInputSignalAvailable( pStrInputSignal->GetIDPtr() ) )
				{
					continue;
				}
			}
			
			// HYS-1193 : Only for electroActuator
			// HYS-1458 : If fail-safe checkbox is checked we have to get a fail-safe type. If the checkbox is not checked we have to get No fail-safe. 
			// if these conditions are not satisfied, actuator is not fit.
			if( ( NULL != pElecActr ) && iFailSafe != -1 
				&& ( ( pElecActr->GetFailSafe() >= CDB_ElectroActuator::FailSafeType::eFSTypeElectronic 
				&& pElecActr->GetFailSafe() < CDB_ElectroActuator::FailSafeType::eFSTypeLast && iFailSafe != 1 )
				|| ( (int)pElecActr->GetFailSafe() == 0 && iFailSafe != 0 ) ) )
			{
				continue;
			}

			if( NULL != pElecActr && eDRP != CDB_ElectroActuator::DefaultReturnPosition::Undefined )
			{
				// Check.
				if( eDRP != pElecActr->GetDefaultReturnPos() )
				{
					if( CDB_ElectroActuator::DefaultReturnPosition::None == eDRP )
					{
						continue;
					}
					else if ( CDB_ElectroActuator::DefaultReturnPosition::Configurable != pElecActr->GetDefaultReturnPos() )
					{
						continue;
					}
				}
			}

			pList->Add( pActr->GetIDPtr().ID, pActr->GetOrderKey(), (LPARAM)pActr, false, false );
			iCount++;
		}
	}

	return iCount;
}

int CTADatabase::GetActuatorPowerSupplyList( CRank *pList, CTable *pTab, CDB_ControlProperties::CvCtrlType eCvCtrlType, FilterSelection eFilterSelection )
{
	if( NULL == pList || NULL == pTab )
	{
		return 0;
	}

	CRank rkList;
	rkList.FillFromTable( pTab );
	return GetActuatorPowerSupplyList( pList, &rkList, eCvCtrlType, eFilterSelection );
}

int CTADatabase::GetActuatorPowerSupplyList( CRank *pList, CRank *pTab, CDB_ControlProperties::CvCtrlType eCvCtrlType, FilterSelection eFilterSelection )
{
	if( NULL == pList || NULL == pTab )
	{
		return 0;
	}

	CString str;
	LPARAM lParam;
	int iCount = 0;

	for( BOOL bContinue = pTab->GetFirst( str, lParam ); TRUE == bContinue; bContinue = pTab->GetNext( str, lParam ) )
	{
		CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ( CData * )lParam );

		// Run alls entries for power supply.
		if( NULL == pclElectroActuator )
		{
			continue;
		}

		if( false == pclElectroActuator->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclElectroActuator, eFilterSelection ) )
		{
			continue;
		}
		
		// If control type is not defined, 'IsActuatorControlFile' always return 'false'. So we can't have the case where 'eCvCtrlType == eCvNu'
		// and where 'IsActuatorControlFit' returns 'true'.
		if( eCvCtrlType != CDB_ControlProperties::CvCtrlType::eCvNU && false == pclElectroActuator->IsActuatorControlFit( eCvCtrlType, false ) )
		{
			continue;
		}

		// Extract list of power supply.
		for( int iLoop = 0; iLoop < ( int )pclElectroActuator->GetNumOfPowerSupplyIDptr(); iLoop++ )
		{
			IDPTR PowerSupplyIDPtr = pclElectroActuator->GetPowerSupplyIDPtr( iLoop );
			CDB_StringID *pclActrPowerSupply = dynamic_cast<CDB_StringID *>( PowerSupplyIDPtr.MP );

			if( NULL == pclActrPowerSupply )
			{
				continue;
			}
			
			int iKey = _ttoi( pclActrPowerSupply->GetIDstr() );

			if( false == pList->IfExist( (double)iKey ) )
			{
				pList->Add( pclActrPowerSupply->GetString(), iKey, (LPARAM)(void *)PowerSupplyIDPtr.MP );
				iCount ++;
			}
		}
	}

	return iCount;
}

int CTADatabase::GetActuatorInputSignalList( CRank *pList, CTable *pTab, CDB_ControlProperties::CvCtrlType eCvCtrlType, _string strSelPowerSupplyID, FilterSelection eFilterSelection )
{
	if( NULL == pList || NULL == pTab )
	{
		return 0;
	}

	CRank rkList;
	rkList.FillFromTable( pTab );
	return GetActuatorInputSignalList( pList, &rkList, eCvCtrlType, strSelPowerSupplyID, eFilterSelection );
}

int CTADatabase::GetActuatorInputSignalList( CRank *pList, CRank *pTab, CDB_ControlProperties::CvCtrlType eCvCtrlType, _string strSelPowerSupplyID, FilterSelection eFilterSelection )
{
	CString str;
	LPARAM lParam;
	CDB_StringID *pStrIDPowerSupply = NULL;

	if( false == strSelPowerSupplyID.empty() )
	{
		pStrIDPowerSupply = dynamic_cast<CDB_StringID *>( GetpTADB()->Get( strSelPowerSupplyID.c_str() ).MP );
	}

	// Extract list of input signal.
	int iCount = 0;

	for( BOOL bContinue = pTab->GetFirst( str, lParam ); TRUE == bContinue; bContinue = pTab->GetNext( str, lParam ) )
	{
		CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ( CData * )lParam );

		if( NULL == pclElectroActuator )
		{
			continue;
		}

		if( false == pclElectroActuator->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclElectroActuator, eFilterSelection ) )
		{
			continue;
		}

		// Run alls entries for power supply.
		for( int iLoop = 0; iLoop < ( int )pclElectroActuator->GetNumOfInputSignalsIDptr(); iLoop++ )
		{
			if( pStrIDPowerSupply != NULL )
			{
				if( false == pclElectroActuator->IsPowerSupplyAvailable( pStrIDPowerSupply->GetIDPtr() ) )
				{
					continue;
				}
			}

			// If this input signal doesn't exist yet add it.
			IDPTR InputSignalIDPtr = pclElectroActuator->GetInputSignalsIDPtr( iLoop );
			CDB_StringID *pclActrInputSignal = dynamic_cast<CDB_StringID *>( InputSignalIDPtr.MP );

			if( NULL == pclActrInputSignal )
			{
				continue;
			}

			int iKey = 0;
			pclActrInputSignal->GetIDstrAs( 1, iKey );

			if( false == pList->IfExist( (double)iKey ) )
			{
				CString str;
				FormatString( str, pclActrInputSignal->GetString(), L"" );
				pList->Add( str, iKey, (LPARAM)(void *)InputSignalIDPtr.MP );
				iCount ++;
			}
		}
	}

	return iCount;
}

int CTADatabase::GetActuatorFailSafeValues( CTable *pTab, CDB_ControlProperties::CvCtrlType eCvCtrlType, _string strSelPowerSupplyID, _string strSelInputSignalID, FilterSelection eFilterSelection )
{
	if( NULL == pTab )
	{
		return 0;
	}

	CRank rkList;
	rkList.FillFromTable( pTab );
	return GetActuatorFailSafeValues( &rkList, eCvCtrlType, strSelPowerSupplyID, strSelInputSignalID, eFilterSelection );
}

int CTADatabase::GetActuatorFailSafeValues( CRank *pTab, CDB_ControlProperties::CvCtrlType eCvCtrlType, _string strSelPowerSupplyID, _string strSelInputSignalID, FilterSelection eFilterSelection )
{
	int iReturn = -1;
	CString str;
	LPARAM lParam;

	if( NULL == pTab )
	{
		return iReturn;
	}

	CDB_StringID *pStrIDPowerSupply = NULL;

	if( false == strSelPowerSupplyID.empty() )
	{
		pStrIDPowerSupply = dynamic_cast<CDB_StringID *>( GetpTADB()->Get( strSelPowerSupplyID.c_str() ).MP );
	}

	CDB_StringID *pStrIDInputSignal = NULL;

	if( false == strSelInputSignalID.empty() )
	{
		pStrIDInputSignal = dynamic_cast<CDB_StringID *>( GetpTADB()->Get( strSelInputSignalID.c_str() ).MP );
	}

	// Extract list of input signal.
	int iCount = 0;
	bool bAtLeastOneWithFailSafe = false;
	bool bAtLeastOneWithoutFailSafe = false;
	bool bCanStop = false;
	for( BOOL bContinue = pTab->GetFirst( str, lParam ); TRUE == bContinue; bContinue = pTab->GetNext( str, lParam ) )
	{
		CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( (CData *)lParam );

		if( NULL == pclElectroActuator )
		{
			continue;
		}

		if( false == pclElectroActuator->IsSelectable( true ) )
		{
			continue;
		}

		if( pStrIDPowerSupply != NULL )
		{
			if( false == pclElectroActuator->IsPowerSupplyAvailable( pStrIDPowerSupply->GetIDPtr() ) )
			{
				continue;
			}
		}

		if( pStrIDInputSignal != NULL )
		{
			if( false == pclElectroActuator->IsInputSignalAvailable( pStrIDInputSignal->GetIDPtr() ) )
			{
				continue;
			}
		}

		if( CDB_ElectroActuator::FailSafeType::eFSTypeElectronic <= pclElectroActuator->GetFailSafe()  
			&& CDB_ElectroActuator::FailSafeType::eFSTypeLast > pclElectroActuator->GetFailSafe() )
		{
			bAtLeastOneWithFailSafe = true;
		}
		else
		{
			bAtLeastOneWithoutFailSafe = true;
		}

		if( true == bAtLeastOneWithoutFailSafe && true == bAtLeastOneWithFailSafe )
		{
			bCanStop = true;
		}
	}

	if( true == bAtLeastOneWithoutFailSafe && false == bAtLeastOneWithFailSafe )
	{
		iReturn = eOnlyWithoutFailSafe;
	}
	else if( false == bAtLeastOneWithoutFailSafe && true == bAtLeastOneWithFailSafe )
	{
		iReturn = eOnlyWithFailSafe;
	}
	else if( true == bAtLeastOneWithoutFailSafe && true == bAtLeastOneWithFailSafe )
	{
		iReturn = eBothFailSafe;
	}
	else
	{
		iReturn = eFirst;
	}

	return iReturn;
}

int CTADatabase::GetActuatorDRPList( CRank *pList, CTable *pTab, CDB_ControlProperties::CvCtrlType eCvCtrlType, CDB_ControlValve *pCV, 
	_string strSelPowerSupplyID, _string strSelInputSignalID, int iFailsafe, FilterSelection eFilterSelection )
{
	if( NULL == pList || NULL == pTab )
	{
		return 0;
	}

	CRank rkList;
	rkList.FillFromTable( pTab );
	return GetActuatorDRPList( pList, &rkList, eCvCtrlType, pCV, strSelPowerSupplyID, strSelInputSignalID, iFailsafe, eFilterSelection );
}

int CTADatabase::GetActuatorDRPList( CRank *pList, CRank *pTab, CDB_ControlProperties::CvCtrlType eCvCtrlType, CDB_ControlValve *pCV, _string strSelPowerSupplyID, _string strSelInputSignalID,
		int iFailSafeFct, FilterSelection eFilterSelection )
{
	CString str;
	LPARAM lParam;

	CDB_StringID *pStrIDPowerSupply = NULL;

	if( false == strSelPowerSupplyID.empty() )
	{
		pStrIDPowerSupply = dynamic_cast<CDB_StringID *>( GetpTADB()->Get( strSelPowerSupplyID.c_str() ).MP );
	}

	CDB_StringID *pStrIDInputSignal = NULL;

	if( false == strSelInputSignalID.empty() )
	{
		pStrIDInputSignal = dynamic_cast<CDB_StringID *>( GetpTADB()->Get( strSelInputSignalID.c_str() ).MP );
	}

	// Extract list of input signal.
	int iCount = 0;

	for( BOOL bContinue = pTab->GetFirst( str, lParam ); TRUE == bContinue; bContinue = pTab->GetNext( str, lParam ) )
	{
		CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ( CData * )lParam );

		if( NULL == pclElectroActuator )
		{
			continue;
		}

		if( false == pclElectroActuator->IsSelectable( true ) )
		{
			continue;
		}

		if( pStrIDPowerSupply != NULL )
		{
			if( false == pclElectroActuator->IsPowerSupplyAvailable( pStrIDPowerSupply->GetIDPtr() ) )
			{
				continue;
			}
		}

		if( pStrIDInputSignal != NULL )
		{
			if( false == pclElectroActuator->IsInputSignalAvailable( pStrIDInputSignal->GetIDPtr() ) )
			{
				continue;
			}
		}
		// HYS-1458 : If fail-safe checkbox is checked we have to get a fail-safe type. If the checkbox is not checked we have to get No fail-safe. 
		// if these conditions are not satisfied, actuator is not fit.
		if( -1 != iFailSafeFct && ( ( pclElectroActuator->GetFailSafe() >= CDB_ElectroActuator::FailSafeType::eFSTypeElectronic 
			&& pclElectroActuator->GetFailSafe() < CDB_ElectroActuator::FailSafeType::eFSTypeLast && iFailSafeFct != 1 )
			|| ( (int)pclElectroActuator->GetFailSafe() == 0 && iFailSafeFct != 0 ) ) )
		{
			continue;
		}

		int iKey = ( int ) CDB_ControlValve::drpfNone;

		if( NULL != pCV )
		{
			iKey = pCV->GetCompatibleDRPFunction( ( int )pclElectroActuator->GetDefaultReturnPos() );

			if( false == pList->IfExist( (double)iKey ) )
			{
				pList->Add( GetDRPFunctionStr( ( CDB_ControlValve::DRPFunction ) iKey ), iKey, (LPARAM)(void *)iKey );
				iCount ++;
			}
		}
		else
		{
			// If no CV to check look actuator's default return position to fill the combo.
			iKey = (int)pclElectroActuator->GetDefaultReturnPos();

			if( false == pList->IfExist( (double)iKey ) )
			{
				pList->Add( GetDRPFunctionStr( ( CDB_ControlValve::DRPFunction ) iKey ), iKey, (LPARAM)(void *)iKey );
				iCount ++;
			}
		}
	}

	return iCount;
}

int CTADatabase::GetAdapterList( CDB_ControlValve *pclControlValve, CRank *pclAdapterList, CDB_RuledTableBase **ppclRuledTable,
		bool bDowngradeActuatorFunctionality, CString strPowerSupplyToMatchID, CString strInputSignalToMatchID, int iFailSafeFctToMatch,
		CDB_ControlValve::DRPFunction eDRPFctToMatch, CDB_ControlProperties::CvCtrlType eCvCtrlType,
		bool bForSet, FilterSelection eFilterSelection )
{
	try
	{
		if( NULL == pclControlValve || NULL == pclAdapterList || NULL == ppclRuledTable )
		{
			return 0;
		}

		if( CDB_ControlValve::AdapterListMode::almNotApplicable == pclControlValve->GetAdapterListMode() )
		{
			return 0;
		}

		int iReturn = 0;

		if( CDB_ControlValve::AdapterListMode::almNormal == pclControlValve->GetAdapterListMode() )
		{
			pclAdapterList->PurgeAll();
			*ppclRuledTable = ( CDB_RuledTable * )( pclControlValve->GetAdapterGroupIDPtr().MP );

			if( NULL != *ppclRuledTable )
			{
				GetAccessories( pclAdapterList, *ppclRuledTable );
				iReturn = pclAdapterList->GetCount();
			}
		}
		else
		{
			pclAdapterList->PurgeAll();
			*ppclRuledTable = NULL;
			std::set< CDB_Product * > setAdapterList;
			std::vector< CDB_Product * > m_vecAdapterBuiltIn;

			if( false == bForSet )
			{
				// For special mode, we show ONLY adapters that are common to the control valve and actuators (ex: FUSION).
				// It's not always the case. For example, for KTM valves (normal mode) we show all adapters independently of compatible actuators.
				// Because in this case we allow user to choose adapters that are not a TA product.

				// Retrieve the adapter group on the control valve.
				CDB_RuledTableBase *pclCVAdapterGroup = (CDB_RuledTableBase *)( pclControlValve->GetAdapterGroupIDPtr().MP );

				if( NULL == pclCVAdapterGroup )
				{
					return 0;
				}

				// Retrieve the actuator group on the control valve.
				CDB_RuledTableBase *pclActuatorGroup = (CDB_RuledTableBase *)( pclControlValve->GetActuatorGroupIDPtr().MP );

				if( NULL == pclActuatorGroup )
				{
					return 0;
				}

				// Run all adapters linked to the control valves.
				for( IDPTR CVAdapterIDPtr = pclCVAdapterGroup->GetFirst(); CVAdapterIDPtr.MP != NULL; CVAdapterIDPtr = pclCVAdapterGroup->GetNext( CVAdapterIDPtr.MP ) )
				{
					// Run all actuators in the group.
					bool bStop = false;

					for( IDPTR ActuatorIDPtr = pclActuatorGroup->GetFirst(); ActuatorIDPtr.MP != NULL
						 && false == bStop; ActuatorIDPtr = pclActuatorGroup->GetNext( ActuatorIDPtr.MP ) )
					{
						CDB_ElectroActuator *pclActuator = dynamic_cast<CDB_ElectroActuator *>( ActuatorIDPtr.MP );

						if( NULL == pclActuator || false == pclActuator->IsSelectable( true ) || true == MustExclude( pclActuator, eFilterSelection ) )
						{
							continue;
						}

						// Make sure that this actuator can be listed (in regards of user choice with 'Power supply', 'Input signal', 'Fail safe' and 'Default return position').
						CDB_ControlValve::DRPFunction DRPFct = pclControlValve->GetCompatibleDRPFunction( ( int )pclActuator->GetDefaultReturnPos() );

						if( false == pclActuator->IsActuatorFit( strPowerSupplyToMatchID, strInputSignalToMatchID, iFailSafeFctToMatch, eDRPFctToMatch, DRPFct, eCvCtrlType,
								bDowngradeActuatorFunctionality ) )
						{
							continue;
						}

						// Run all adapters linked to the current actuator.
						CDB_RuledTableBase *pclActuatorAdapterTable = ( CDB_RuledTableBase * )( pclActuator->GetActAdapterGroupIDPtr().MP );

						if( NULL == pclActuatorAdapterTable )
						{
							continue;
						}

						for( IDPTR ActuatorAdapterIDPtr = pclActuatorAdapterTable->GetFirst(); ActuatorAdapterIDPtr.MP != NULL
							 && false == bStop; ActuatorAdapterIDPtr = pclActuatorAdapterTable->GetNext( ActuatorAdapterIDPtr.MP ) )
						{
							if( 0 == IDcmp( CVAdapterIDPtr.ID, ActuatorAdapterIDPtr.ID ) )
							{
								CDB_Product *pclAdapter = dynamic_cast<CDB_Product *>( CVAdapterIDPtr.MP );

								if( NULL == pclAdapter || false == pclAdapter->IsAnAccessory() )
								{
									continue;
								}

								if( false == pclAdapter->IsSelectable( true ) )
								{
									continue;
								}

								if( true == MustExclude( pclAdapter, eFilterSelection ) )
								{
									continue;
								}

								if( 0 == setAdapterList.count( pclAdapter ) )
								{
									setAdapterList.insert( pclAdapter );
								}

								// Memorize adapter built-in status.
								if( true == pclActuatorAdapterTable->IsBuiltIn( pclAdapter ) || true == pclCVAdapterGroup->IsBuiltIn( pclAdapter ) )
								{
									m_vecAdapterBuiltIn.push_back( pclAdapter );
								}

								bStop = true;
							}
						}
					}
				}
			}
			else
			{
				// First step: Find set that are corresponding to the control valve.
				std::set<CDB_Set *> CVActSetArray;
				CTableSet *pCVActTableSet = pclControlValve->GetTableSet();
				
				if( NULL == pCVActTableSet )
				{
					HYSELECT_THROW( _T("Internal error: The set table is not defined for the control valve %s."), pclControlValve->GetIDPtr().ID );
				}
				
				int iCDBSetCount = pCVActTableSet->FindCompatibleSet( &CVActSetArray, pclControlValve->GetIDPtr().ID, _T( "" ) );

				if( 0 == iCDBSetCount )
				{
					return 0;
				}

				for( std::set<CDB_Set *>::iterator iterCDBSet = CVActSetArray.begin(); iterCDBSet != CVActSetArray.end(); ++iterCDBSet )
				{
					CDB_Set *pCVActSet = *iterCDBSet;
					IDPTR AdapterIDPtr = pCVActSet->GetAccGroupIDPtr();

					if( NULL == AdapterIDPtr.MP )
					{
						// If no adapter in 'CDB_Set', we must choose adapters that are common with control valves and actuators.
						CDB_ControlValve *pclCVInSet = (CDB_ControlValve *)( pCVActSet->GetFirstIDPtr().MP );

						if( NULL == pclCVInSet )
						{
							continue;
						}

						CDB_RuledTableBase *pclCVAdapterGroup = (CDB_RuledTableBase *)( pclCVInSet->GetAdapterGroupIDPtr().MP );

						if( NULL == pclCVAdapterGroup )
						{
							continue;
						}

						CDB_ElectroActuator *pclActuatorInSet = ( CDB_ElectroActuator * )( pCVActSet->GetSecondIDPtr().MP );

						if( NULL == pclActuatorInSet || false == pclActuatorInSet->IsSelectable( true ) || true == MustExclude( pclActuatorInSet, eFilterSelection ) )
						{
							continue;
						}

						// Make sure that this actuator can be listed (in regards of user choice with 'Power supply', 'Input signal', 'Fail safe' and 'Default return position').
						CDB_ControlValve::DRPFunction DRPFct = pclControlValve->GetCompatibleDRPFunction( ( int )pclActuatorInSet->GetDefaultReturnPos() );

						if( false == pclActuatorInSet->IsActuatorFit( strPowerSupplyToMatchID, strInputSignalToMatchID, iFailSafeFctToMatch, eDRPFctToMatch, DRPFct, eCvCtrlType,
								bDowngradeActuatorFunctionality ) )
						{
							continue;
						}

						CDB_RuledTableBase *pclActuatorAdapterTable = (CDB_RuledTableBase *)( pclActuatorInSet->GetActAdapterGroupIDPtr().MP );

						if( NULL == pclActuatorAdapterTable )
						{
							continue;
						}

						// Run all adapters linked to the control valve.
						for( IDPTR CVAdapterIDPtr = pclCVAdapterGroup->GetFirst(); CVAdapterIDPtr.MP != NULL; CVAdapterIDPtr = pclCVAdapterGroup->GetNext( CVAdapterIDPtr.MP ) )
						{
							// Run all adapters linked to the current actuator.
							bool bStop = false;

							for( IDPTR ActuatorAdapterIDPtr = pclActuatorAdapterTable->GetFirst(); ActuatorAdapterIDPtr.MP != NULL
								 && false == bStop; ActuatorAdapterIDPtr = pclActuatorAdapterTable->GetNext( ActuatorAdapterIDPtr.MP ) )
							{
								if( 0 == IDcmp( CVAdapterIDPtr.ID, ActuatorAdapterIDPtr.ID ) )
								{
									CDB_Product *pclAdapter = dynamic_cast<CDB_Product *>( CVAdapterIDPtr.MP );

									if( NULL == pclAdapter || false == pclAdapter->IsAnAccessory() )
									{
										continue;
									}

									if( false == pclAdapter->IsSelectable( true ) )
									{
										continue;
									}

									if( true == MustExclude( pclAdapter, eFilterSelection ) )
									{
										continue;
									}

									if( 0 == setAdapterList.count( pclAdapter ) )
									{
										setAdapterList.insert( pclAdapter );
									}

									bStop = true;
								}
							}
						}
					}
					else
					{
						// Take this one.
						CDB_Product *pclAdapter = dynamic_cast<CDB_Product *>( AdapterIDPtr.MP );

						if( NULL == pclAdapter || false == pclAdapter->IsAnAccessory() )
						{
							continue;
						}

						if( false == pclAdapter->IsSelectable( true ) )
						{
							continue;
						}

						if( true == MustExclude( pclAdapter, eFilterSelection ) )
						{
							continue;
						}

						if( 0 == setAdapterList.count( pclAdapter ) )
						{
							setAdapterList.insert( pclAdapter );
						}
					}
				}
			}

			// Second step: we can now dynamically create a CDB_RuledTable to allow 'RViewSSelSS' to correctly manage exclusion/implication.
			CDB_RuledTableBase *pclLocalRuledTable = ( CDB_RuledTableBase * )( Get( _T("CVADAPTERRULES") ).MP );

			if( NULL != pclLocalRuledTable )
			{
				IDPTR IDPtr = pclLocalRuledTable->GetIDPtr();
				Access().Remove( IDPtr );
				DeleteObject( IDPtr );
				pclLocalRuledTable = NULL;
			}

			if( setAdapterList.size() > 0 )
			{
				IDPTR IDPtr;
				TASApp.GetpTADB()->CreateObject( IDPtr, CLASS( CDB_RuledTableBase ), _T("CVADAPTERRULES") );
				TASApp.GetpTADB()->Access().Insert( IDPtr );

				pclLocalRuledTable = (CDB_RuledTableBase *)( IDPtr.MP );

				// For this table, each item excludes all others.
				for( std::set< CDB_Product * >::iterator iterAdapter = setAdapterList.begin(); iterAdapter != setAdapterList.end(); ++iterAdapter )
				{
					pclAdapterList->AddStrSort( ( *iterAdapter )->GetName(), 0, (LPARAM)( *iterAdapter ), false );

					for( std::set< CDB_Product * >::iterator iterAdapter2 = setAdapterList.begin(); iterAdapter2 != setAdapterList.end(); ++iterAdapter2 )
					{
						if( *iterAdapter2 != *iterAdapter )
						{
							pclLocalRuledTable->AddItem( ( *iterAdapter )->GetIDPtr().ID, ( *iterAdapter2 )->GetIDPtr(), CDB_RuledTableBase::MMapName::Excluded );
						}
					}

					if( m_vecAdapterBuiltIn.end() != std::find( m_vecAdapterBuiltIn.begin(), m_vecAdapterBuiltIn.end(), *iterAdapter ) )
					{
						pclLocalRuledTable->AddItem( ( *iterAdapter )->GetIDPtr().ID, ( *iterAdapter )->GetIDPtr(), CDB_RuledTableBase::MMapName::BuiltIn );
					}
				}

				*ppclRuledTable = pclLocalRuledTable;
			}
		}

		return iReturn;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CTADatabase::GetAdapterList'."), __LINE__, __FILE__ );
		throw;
	}
}

#endif

int CTADatabase::GetSvTypeList( CRankEx *pList, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("SHUTTYPE_TAB") ).MP );

	if( NULL == pTab )
	{
		return 0;
	}

	std::vector <LPARAM> ar;
	ar.resize( pTab->GetItemCount() );

	int i = 0;

	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		ASSERT( i < ( int )ar.size() );
		ar.at( i ) = (LPARAM)IDPtr.MP;
		i++;
	}

	// Remove unused elements.
	ar.resize( i );

	// Try to find at least one shut-off valve available for this type.
	pTab = static_cast<CTable *>( Get( _T("SHUTOFF_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( i = 0; i < ( int )ar.size(); )
	{
		CDB_StringID *pSID = dynamic_cast<CDB_StringID *>( ( CData * )ar[i] );
		ASSERT( NULL != pSID );

		if( NULL == pSID )
		{
			continue;
		}

		_string strType = pSID->GetIDPtr().ID;
		bool bSelectable = false;

		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
		{
			CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( ( CData * )IDPtr.MP );

			if( NULL == pTAp )
			{
				continue;
			}

			if( false == pTAp->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( pTAp, eFilterSelection ) )
			{
				continue;
			}

			// Scan all shut-off valves with the same type.
			if( strType != pTAp->GetTypeIDPtr().ID )
			{
				continue;
			}

			int iDN = pTAp->GetSizeKey();

			if( iDN < iDNMin )
			{
				continue;
			}

			if( iDN > iDNMax )
			{
				continue;
			}

			bSelectable = true;

			// Loop aborted one shut-off of this type is enough to display the type.
			break;
		}

		if( false == bSelectable )
		{
			ar.erase( ar.begin() + i );
		}
		else
		{
			i++;
		}
	}

	// Transfer and sort list.
	for( i = 0; i < ( int )ar.size(); i++ )
	{
		pList->Add( ( (CDB_StringID *)ar.at( i ) )->GetString(), _ttoi( ( (CDB_StringID *)ar.at( i ) )->GetIDstr() ) - 1, ar.at( i ) );
	}

	i = ( int )ar.size();
	ar.clear();
	return i;
}

int CTADatabase::GetSvFamilyList( CRankEx *pList, _string strSelTypeID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	// Fill the list according to selected type.
	CTable *pTab = static_cast<CTable *>( Get( _T("SHUTOFF_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Selectable ?
		if( false == pTAp->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pTAp->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey;
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		dKey = pTAp->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pTAp->GetFamilyIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pTAp->GetFamilyIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pTAp->GetFamilyIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetSvBdyMatList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	// Fill the list according to selected type.
	CTable *pTab = static_cast<CTable *>( Get( _T("SHUTOFF_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Selectable ?
		if( false == pTAp->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pTAp->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pTAp->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test DN
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Bdy | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pTAp->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pTAp->GetBodyMaterialIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pTAp->GetBodyMaterialIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pTAp->GetBodyMaterialIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetSvConnList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID, FilterSelection eFilterSelection, int iDNMin,
		int iDNMax )
{
	// Fill the list according to selected type
	CTable *pTab = static_cast<CTable *>( Get( _T("SHUTOFF_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Selectable ?
		if( false == pTAp->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pTAp->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pTAp->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pTAp->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Connect | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pTAp->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pTAp->GetConnectIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pTAp->GetConnectIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pTAp->GetConnectIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetSvVersList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID, _string strSelConnectID,
		FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("SHUTOFF_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Keep only CDB_TAProduct and children.
		if( false == pTAp->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pTAp->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pTAp->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pTAp->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		if( false == strSelConnectID.empty() && strSelConnectID != pTAp->GetConnectIDPtr().ID )
		{
			continue;
		}

		if( 0 == _tcscmp( pTAp->GetVersionIDPtr().ID, _T("VERS_EMPTY") ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Version | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pTAp->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pTAp->GetVersionIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pTAp->GetVersionIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pTAp->GetVersionIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::GetSvPNList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID, _string strSelConnectID,
		_string strSelVersionID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	// Fill the PN combo according to selected type, connection and version.
	double dPN, dDP, dPmax = 0.0;
	CTable *pTab = static_cast<CTable *>( Get( _T("SHUTOFF_TAB") ).MP );
	ASSERT( NULL != pTab );
	CTable	*pPNTab = static_cast<CTable *>( Get( _T("PN_TAB") ).MP );
	ASSERT( NULL != pPNTab );

	// Loop on selectable regulating valves (Those with Pmax = Pmax of previous item are not treated).
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Keep only CDB_TAProduct and children.
		if( false == pTAp->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pTAp->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pTAp->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pTAp->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		if( false == strSelConnectID.empty() && strSelConnectID != pTAp->GetConnectIDPtr().ID )
		{
			continue;
		}

		if( false == strSelVersionID.empty() && strSelVersionID != pTAp->GetVersionIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		if( dPmax != pTAp->GetPmaxmax() )
		{
			dPmax =  pTAp->GetPmaxmax();
			// Find the largest PN lower than Pmax.
			dDP = DBL_MAX;
			IDPTR PNIDPtrFnd;

			for( IDPTR PNIDPtr = pPNTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *PNIDPtr.ID; PNIDPtr = pPNTab->GetNext() )
			{
				if( RD_OK != ReadDoubleFromStr( ( TCHAR * )( (CDB_StringID *)PNIDPtr.MP )->GetIDstr(), &dPN ) )
				{
					ASSERTA_RETURN( -1 );
				}

				if( ( dPmax - dPN ) >= 0.0 && fabs( dPmax - dPN ) < dDP )
				{
					dDP = fabs( dPmax - dPN );
					PNIDPtrFnd = PNIDPtr;
				}
			}

			// Found product having priority.
			double dKey = 0;
			( (CDB_StringID *)PNIDPtrFnd.MP )->GetIDstrAs( 0, dKey );

			// Already added ?
			_string InListstr;
			LPARAM InListlparam = (LPARAM)PNIDPtrFnd.MP;
			double dInListkey = 0;

			if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
			{
				// Yes take a copy, check priority key, and keep only the most important.
				if( dKey < dInListkey )
				{
					pList->Delete( dInListkey );
				}
				else
				{
					continue;
				}
			}

			pList->Add( ( ( CDB_String * )PNIDPtrFnd.MP )->GetString(), dKey, (LPARAM)PNIDPtrFnd.MP );
		}
	}

	return pList->GetCount();
}

int CTADatabase::GetSvList( CRankEx *pList, _string strSelTypeID, _string strSelFamilyID, _string strSelBdyMatID, _string strSelConnectID,
		_string strSelVersionID, FilterSelection eFilterSelection, int iDNMin, int iDNMax, CData *pProd )
{
	// By default shut-off valves are defined in 'SHUTOFF_TAB'.
	CTable *pTab = static_cast<CTable *>( Get( _T("SHUTOFF_TAB") ).MP );
	ASSERT( NULL != pTab );

	// For increasing 'if' test below.
	CDB_StringID *pSelTypeID = NULL;
	CDB_StringID *pSelFamilyID = NULL;
	CDB_StringID *pSelBdyMatID = NULL;
	CDB_StringID *pSelConnectID = NULL;
	CDB_StringID *pSelVersionID = NULL;

	if( false == strSelTypeID.empty() )
	{
		pSelTypeID = ( (CDB_StringID *)Get( strSelTypeID.c_str() ).MP );
	}

	if( false == strSelFamilyID.empty() )
	{
		pSelFamilyID = ( (CDB_StringID *)Get( strSelFamilyID.c_str() ).MP );
	}

	if( false == strSelBdyMatID.empty() )
	{
		pSelBdyMatID = ( (CDB_StringID *)Get( strSelBdyMatID.c_str() ).MP );
	}

	if( false == strSelConnectID.empty() )
	{
		pSelConnectID = ( (CDB_StringID *)Get( strSelConnectID.c_str() ).MP );
	}

	if( false == strSelVersionID.empty() )
	{
		pSelVersionID = ( (CDB_StringID *)Get( strSelVersionID.c_str() ).MP );
	}

	// Loop on selectable shut-off valves.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		// Keep only CDB_TAProduct and children.
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );

		if( NULL == pTAp )
		{
			continue;
		}

		if( false == pTAp->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( NULL != pSelTypeID && pSelTypeID != ( (CDB_StringID *)pTAp->GetTypeIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelFamilyID && pSelFamilyID != ( (CDB_StringID *)pTAp->GetFamilyIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelBdyMatID && pSelBdyMatID != ( (CDB_StringID *)pTAp->GetBodyMaterialIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelConnectID && pSelConnectID != ( (CDB_StringID *)pTAp->GetConnectIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelVersionID && pSelVersionID != ( (CDB_StringID *)pTAp->GetVersionIDPtr().MP ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey = (double)pTAp->GetSortingKey();

		// Store valve characteristic ID.
		CData *pValveChar = pTAp->GetValveCharDataPointer();
		bool bCouldbeAdded = true;			// Could be added by default

		if( NULL != pValveChar )
		{
#ifdef TACBX
			bCouldbeAdded = CheckIfCharactAlreadyExist( pList, pTAp, pProd );
#endif

			if( true == bCouldbeAdded )
			{
				pList->Add( pValveChar->GetIDPtr().ID, dKey, (LPARAM)IDPtr.MP );
			}
		}
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetSvActuatorList( CRank *pActuatorList, CDB_ShutoffValve *pclShutOffValve, bool bOnlyPackage, FilterSelection eFilterSelection )
{
	if( NULL == pActuatorList || NULL == pclShutOffValve )
	{
		return 0;
	}

	pActuatorList->PurgeAll();

	if( true == bOnlyPackage && false == pclShutOffValve->IsPartOfaSet() )
	{
		return 0;
	}

	// Retrieve actuator group.
	IDPTR ActuatorGroupIDPtr = pclShutOffValve->GetActuatorGroupIDPtr();

	if( NULL == ActuatorGroupIDPtr.MP || _T('\0') == *ActuatorGroupIDPtr.ID )
	{
		return 0;
	}

	CTable *pclActuatorTab = dynamic_cast<CTable *>( Get( ActuatorGroupIDPtr.ID ).MP );

	if( NULL == pclActuatorTab )
	{
		return 0;
	}

	// Run all actuators in the group.
	for( IDPTR ActuatorIDPtr = pclActuatorTab->GetFirst(); NULL != ActuatorIDPtr.MP; ActuatorIDPtr = pclActuatorTab->GetNext() )
	{
		CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ActuatorIDPtr.MP );

		if( NULL == pclElectroActuator )
		{
			continue;
		}

		if( false == pclElectroActuator->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclElectroActuator, eFilterSelection ) )
		{
			continue;
		}

		// If we want only package...
		if( true == bOnlyPackage )
		{
			// Retrieve the correct set table in regards to the control valve.
			bool bValidActuator = false;
			CTableSet *pCVActTableSet = NULL;
			pCVActTableSet = pclShutOffValve->GetTableSet();

			if( NULL == pCVActTableSet )
			{
				continue;
			}

			// If there is not set with the current control valve and actuator, we continue searching.
			if( NULL == pCVActTableSet->FindCompatibleSet( pclShutOffValve->GetIDPtr().ID, pclElectroActuator->GetIDPtr().ID ) )
			{
				continue;
			}
		}

		double dKey = pclElectroActuator->GetMaxForceTorque();
		pActuatorList->Add( ( LPCTSTR )pclElectroActuator->GetName(), dKey, (LPARAM)pclElectroActuator, true );
	}

	return pActuatorList->GetCount();
}
#endif


#ifndef TACBX
int CTADatabase::Get6WayValveConnectionList( CRankEx *pList, FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bOnlyForSet )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("6WAYCTRLVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Selectable ?
		if( false == pTAp->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		// HYS-1877: Is a part of set
		if( true == bOnlyForSet && false == pTAp->IsPartOfaSet() )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey;
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		dKey = pTAp->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pTAp->GetConnectIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pTAp->GetConnectIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pTAp->GetConnectIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::Get6WayValveConnectionList( CRankEx *pSrcList, CRankEx *pDestList )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclValve = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclValve );

		if( NULL == pclValve )
		{
			continue;
		}

		// Add the new family ID if doesn't already exist.
		IDPTR ConnectIDPtr = pclValve->GetConnectIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( ConnectIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)ConnectIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)ConnectIDPtr.MP )->GetString(), dKey, (LPARAM)ConnectIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}

int CTADatabase::Get6WayValveVersList( CRankEx *pList, _string strSelConnectID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("6WAYCTRLVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Keep only CDB_TAProduct and children.
		if( false == pTAp->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelConnectID.empty() && strSelConnectID != pTAp->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Version | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pTAp->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pTAp->GetVersionIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pTAp->GetVersionIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pTAp->GetVersionIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::Get6WayValveVersList( CRankEx *pSrcList, CRankEx *pDestList, _string strConnectID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclValve = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclValve );

		if( NULL == pclValve )
		{
			continue;
		}

		// Test Connection ID.
		if( false == strConnectID.empty() && strConnectID != pclValve->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Add the new version ID if doesn't already exist.
		IDPTR VersionIDPtr = pclValve->GetVersionIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( VersionIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)VersionIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)VersionIDPtr.MP )->GetString(), dKey, (LPARAM)VersionIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}

int CTADatabase::Get6WayValveList( CRankEx *pList, _string strSelConnectID, _string strSelVersionID, FilterSelection eFilterSelection,
		int iDNMin, int iDNMax, CData *pProd )
{
	// By default 6-way valves are defined in '6WAYCTRLVALV_TAB'.
	CTable *pTab = static_cast<CTable *>( Get( _T("6WAYCTRLVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	// For increasing 'if' test below.
	CDB_StringID *pSelConnectID = NULL;
	CDB_StringID *pSelVersionID = NULL;

	if( false == strSelConnectID.empty() )
	{
		pSelConnectID = ( (CDB_StringID *)Get( strSelConnectID.c_str() ).MP );
	}

	if( false == strSelVersionID.empty() )
	{
		pSelVersionID = ( (CDB_StringID *)Get( strSelVersionID.c_str() ).MP );
	}

	// Loop on selectable 6-way valves.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		// Keep only CDB_TAProduct and children.
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );

		if( NULL == pTAp )
		{
			continue;
		}

		if( false == pTAp->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( NULL != pSelConnectID && pSelConnectID != ( (CDB_StringID *)pTAp->GetConnectIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelVersionID && pSelVersionID != ( (CDB_StringID *)pTAp->GetVersionIDPtr().MP ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		double dKey = 0.0;
		_string str;
		LPARAM lparam = (LPARAM)IDPtr.MP;

		// Verify the lparam doesn't exist yet
		if( false == pList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)pTAp->GetSortingKey();
			pList->Add( pTAp->GetName(), dKey, (LPARAM)pTAp );
		}
	}

	return pList->GetCount();
}

int CTADatabase::Get6WayValveActrInputSignalList( CRankEx * pInputSignalList, CRankEx * pCVList, CDB_ControlProperties::CvCtrlType eCvCtrlType, 
	_string strSelFamID, bool bFastConnection, FilterSelection eFilterSelection, bool bOnlyForSet )
{
	if( NULL == pInputSignalList || NULL == pCVList || 0 == pCVList->GetCount() )
	{
		return 0;
	}

	CTable *pSignalsTab = static_cast<CTable *>( Get( _T("SIG_TAB") ).MP );

	if( NULL == pSignalsTab )
	{
		return 0;
	}

	pInputSignalList->PurgeAll();

	// First pass: run all control valves and retrieve actuator groups.
	_string str;
	std::map<int, int> mapActrKeyList;
	std::map<CTable *, int> mapActrGpList;
	std::vector<CDB_TAProduct*> vecCVPackage;

	CTable *pTab = static_cast<CTable *>( Get( _T("6WAYV_PICV_TAB") ).MP );

	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pclControlValve = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pclControlValve );

		if( NULL == pclControlValve )
		{
			continue;
		}

		// Selectable ?
		if( false == pclControlValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclControlValve, eFilterSelection ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pclControlValve->GetSizeKey();

		//if( iDN < iDNMin )
		//{
		//	continue;
		//}

		//if( iDN > iDNMax )
		//{
		//	continue;
		//}
		
		if( false == strSelFamID.empty() && ( strSelFamID != pclControlValve->GetFamilyIDPtr().ID ) )
		{
			continue;
		}

		// Retrieve actuator group.
		IDPTR ActrGpIDPtr = pclControlValve->GetActuatorGroupIDPtr();

		if( NULL == ActrGpIDPtr.MP || 0 == *ActrGpIDPtr.ID )
		{
			continue;
		}

		CTable *pclActrTab = dynamic_cast<CTable *>( Get( ActrGpIDPtr.ID ).MP );

		if( NULL == pclActrTab )
		{
			continue;
		}

		vecCVPackage.push_back( pclControlValve );

		if( mapActrGpList.count( pclActrTab ) > 0 )
		{
			continue;
		}
		mapActrGpList[pclActrTab] = 0;
	}

	if( 0 == mapActrGpList.size() )
	{
		return 0;
	}

	// Second pass: run all actuator groups.
	for( std::map<CTable *, int>::iterator iter = mapActrGpList.begin(); iter != mapActrGpList.end(); ++iter )
	{
		CTable *pclActrTab = iter->first;

		// Run all actuators in this group.
		for( IDPTR ActrIDPtr = pclActrTab->GetFirst(); 0 != *ActrIDPtr.ID; ActrIDPtr = pclActrTab->GetNext() )
		{
			CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ActrIDPtr.MP );

			if( NULL == pclElectroActuator )
			{
				continue;
			}

			if( false == pclElectroActuator->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( pclElectroActuator, eFilterSelection ) )
			{
				continue;
			}
			// if true = bFastConnection only relay co actuators are accepted. Else only relay 24 v and 220 v are accepted
			if( true == bFastConnection && ( 0 != StringCompare( pclElectroActuator->GetRelayID(), _T("RELAY_CO") ) ) )
			{
				continue;
			}
			else if( false == bFastConnection && ( 0 == StringCompare( pclElectroActuator->GetRelayID(), _T("RELAY_CO") ) ) )
			{
				continue;
			}
			else if( ( false == bFastConnection ) && ( 0 != StringCompare( pclElectroActuator->GetRelayID(), _T("RELAY_24V") ) )
				&& ( 0 != StringCompare( pclElectroActuator->GetRelayID(), _T("RELAY_220V") ) ) )
			{
				continue;
			}
			
			// If control type is not defined, 'IsActuatorControlFile' always return 'false'. So we can't have the case where 'eCvCtrlType == eCvNu'
			// and where 'IsActuatorControlFit' returns 'true'.
			if( ( eCvCtrlType != CDB_ControlProperties::CvCtrlType::eCvNU ) && ( false == pclElectroActuator->IsActuatorControlFit( eCvCtrlType, false ) ) )
			{
				continue;
			}
			// HYS-1877: If we want only package...
			if( true == bOnlyForSet )
			{
				// Run all control valves that has been saved just above (the vector contains thus only the valves that belong to a set).
				bool bValidActuator = false;

				for( std::vector<CDB_TAProduct*>::iterator iter = vecCVPackage.begin(); iter != vecCVPackage.end() && false == bValidActuator; ++iter )
				{
					// Retrieve the correct set table in regards to the control valve.
					CTableSet* pCVActTableSet = NULL;
					pCVActTableSet = ((CDB_Product*)(*iter))->GetTableSet();

					if( NULL == pCVActTableSet )
					{
						continue;
					}

					// If there is not set with the current control valve and actuator, we continue searching.
					if( NULL == pCVActTableSet->FindCompatibleSet6WayValve( (*iter)->GetIDPtr().ID, pclElectroActuator->GetIDPtr().ID ) )
					{
						continue;
					}

					// As soon as we find one set containing both control valve and actuator, it's ok!
					bValidActuator = true;
				}

				if( false == bValidActuator )
				{
					continue;
				}
			}

			// Run alls entries for input signal.
			for( int iLoop = 0; iLoop < (int)pclElectroActuator->GetNumOfInputSignalsIDptr(); iLoop++ )
			{
				IDPTR InputSignalIDPtr = pclElectroActuator->GetInputSignalsIDPtr( iLoop );

				CDB_StringID *pclActrInputSignal = dynamic_cast<CDB_StringID *>( pSignalsTab->Get( InputSignalIDPtr.ID ).MP );

				if( NULL == pclActrInputSignal )
				{
					continue;
				}

				int iKey = _ttoi( pclActrInputSignal->GetIDstr2() );

				if( 0 == mapActrKeyList.count( iKey ) )
				{
					CString str = ( (CDB_StringID *)InputSignalIDPtr.MP )->GetString();
					str.Replace( _T("%1" ), _T( "") );
					pInputSignalList->Add( (LPCTSTR)str, iKey, (LPARAM)(void *)InputSignalIDPtr.MP );
					mapActrKeyList[iKey] = iKey;
				}
			}
		}
	}

	return pInputSignalList->GetCount();
}


int CTADatabase::Get6WayValveSecondaryTypeList( CRankEx *pList, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable* pTabBV = static_cast<CTable *>( Get( _T("REGVALV_TAB") ).MP );

	for( IDPTR IDPtr = pTabBV->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pTabBV->GetNext() )
	{
		// Keep only CDB_RegulatingValve and children.
		CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve *>( IDPtr.MP );

		if( NULL == pRv )
		{
			continue;
		}

		// Test if product is selectable.
		if( false == pRv->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pRv, eFilterSelection ) )
		{
			continue;
		}

		if( 0 != _tcscmp( pRv->GetTypeIDPtr().ID, _T("RVTYPE_BV") ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pRv->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Add new TypeId if doesn't already exist.
		IDPTR TypeIDPtr = pRv->GetTypeIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)TypeIDPtr.MP;

		if( !pList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)TypeIDPtr.MP )->GetIDstr() );
			pList->Add( ( (CDB_StringID *)TypeIDPtr.MP )->GetString(), dKey, (LPARAM)TypeIDPtr.MP );
		}
	}

	return pList->GetCount();
}

int CTADatabase::Get6WayValvePIBCvFamilyList( CRankEx * pList, e6WayValveSelectionMode Selectionmode, FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bOnlyPackage )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("6WAYV_PICV_TAB") ).MP );
	
	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Selectable ?
		if( false == pTAp->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		if( Selectionmode == e6Way_OnOffControlWithPIBCV && CDB_ControlProperties::eCTRLCHAR::EqualPc == pTAp->GetCtrlProp()->GetCvCtrlChar() )
		{
			continue;
		}

		// Found product having priority.
		double dKey;
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		dKey = pTAp->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pTAp->GetFamilyIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		// HYS-1877: If we want only package and control valve doesn't belong to a set we bypass this one.
		if( true == bOnlyPackage && false == pTAp->IsPartOfaSet() )
		{
			continue;
		}

		// HYS-2002: pSetTab is never equal to NULL so to be sure that we are in a set we test with isPartOfaSet.
		CTableSet* pSetTab = (CTableSet*)(pTAp->GetTableSet());
		if( true == pTAp->IsPartOfaSet() && NULL != pSetTab && 0 != StringCompare( _T( "6WAYCTRLVALVSET_TAB" ), pSetTab->GetIDPtr().ID ) )
		{
			continue;
		}
		
		pList->Add( ( (CDB_StringID *)pTAp->GetFamilyIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pTAp->GetFamilyIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::Get6WayValveBvFamilyList( CRankEx * pList, _string strSelTypeID, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab6wayValveBVFam = static_cast<CTable *>( Get( _T("6WAYV_BVFAM_TAB") ).MP );
	
	// Fill the list according to selected type.
	CTable *pTab = static_cast<CTable *>( Get( _T("REGVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Selectable ?
		if( false == pTAp->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pTAp->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey;
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		dKey = pTAp->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pTAp->GetFamilyIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}
		// check the family
		bool bIsIn6WayValveBv = false;
		for( IDPTR FamIDPtr = pTab6wayValveBVFam->GetFirst(); _T( '\0' ) != *FamIDPtr.ID; FamIDPtr = pTab6wayValveBVFam->GetNext( FamIDPtr.MP ) )
		{
			if( 0 == StringCompare( FamIDPtr.ID, pTAp->GetFamilyIDPtr().ID ) )
			{
				bIsIn6WayValveBv = true;
				break;
			}
		}

		if( false == bIsIn6WayValveBv )
		{
			continue;
		}
		pList->Add( ( (CDB_StringID *)pTAp->GetFamilyIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pTAp->GetFamilyIDPtr().MP );
	}

	return pList->GetCount();
}

int CTADatabase::Get6WayValvePICVTypeList( CRankEx * pList, FilterSelection eFilterSelection, int iDNMin, int iDNMax )
{
	CTable *pTab = static_cast<CTable *>( Get( _T("6WAYV_PICV_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_TAProduct *pTAp = dynamic_cast<CDB_TAProduct *>( IDPtr.MP );
		ASSERT( NULL != pTAp );

		if( NULL == pTAp )
		{
			continue;
		}

		// Selectable ?
		if( false == pTAp->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pTAp, eFilterSelection ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pTAp->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey;
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		dKey = pTAp->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pTAp->GetTypeIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pTAp->GetTypeIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pTAp->GetTypeIDPtr().MP );
	}

	return pList->GetCount();
}
#endif


int CTADatabase::GetSmartControlValveTypeList( CRankEx *pList, FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bIsForSet )
{
	CTable *pclTableSmartControlValveType = static_cast<CTable *>( Get( _T("SMARTCTRLVALVETYPE_TAB") ).MP );

	if( NULL == pclTableSmartControlValveType )
	{
		return 0;
	}

	std::vector <LPARAM> vecSmartControlValveTypeList;
	vecSmartControlValveTypeList.resize( pclTableSmartControlValveType->GetItemCount() );

	int i = 0;

	for( IDPTR IDPtr = pclTableSmartControlValveType->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *IDPtr.ID; IDPtr = pclTableSmartControlValveType->GetNext() )
	{
		ASSERT( i < ( int )vecSmartControlValveTypeList.size() );
		vecSmartControlValveTypeList.at( i ) = (LPARAM)IDPtr.MP;
		i++;
	}

	// Remove unused elements.
	vecSmartControlValveTypeList.resize( i );

	// Try to find at least one smart control valve available for this type.
	CTable *pclSmartControlValveTable = static_cast<CTable *>( Get( _T("SMARTCONTROLVALVE_TAB") ).MP );
	ASSERT( NULL != pclSmartControlValveTable );

	for( i = 0; i < ( int )vecSmartControlValveTypeList.size(); )
	{
		CDB_StringID *pSID = dynamic_cast<CDB_StringID *>( (CData *)vecSmartControlValveTypeList[i] );
		ASSERT( NULL != pSID );

		if( NULL == pSID )
		{
			continue;
		}

		_string strType = pSID->GetIDPtr().ID;
		bool bSelectable = false;

		for( IDPTR IDPtr = pclSmartControlValveTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclSmartControlValveTable->GetNext( IDPtr.MP ) )
		{
			CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( (CData *)IDPtr.MP );

			if( NULL == pclSmartControlValve )
			{
				continue;
			}

			if( SmartValveTypeControl != pclSmartControlValve->GetSmartValveType() )
			{
				// It's not a smart control valve.
				continue;
			}

			if( false == pclSmartControlValve->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( pclSmartControlValve, eFilterSelection ) )
			{
				continue;
			}

			if( true == bIsForSet && false == pclSmartControlValve->IsPartOfaSet() )
			{
				continue;
			}

			// Scan all smart control valves with the same type.
			if( strType != pclSmartControlValve->GetTypeIDPtr().ID )
			{
				continue;
			}

			int iDN = pclSmartControlValve->GetSizeKey();

			if( iDN < iDNMin )
			{
				continue;
			}

			if( iDN > iDNMax )
			{
				continue;
			}

			bSelectable = true;

			// Loop aborted one smart control valve of this type is enough to display the type.
			break;
		}

		if( false == bSelectable )
		{
			vecSmartControlValveTypeList.erase( vecSmartControlValveTypeList.begin() + i );
		}
		else
		{
			i++;
		}
	}

	// Transfer and sort list.
	for( i = 0; i < ( int )vecSmartControlValveTypeList.size(); i++ )
	{
		pList->Add( ( (CDB_StringID *)vecSmartControlValveTypeList.at( i ) )->GetString(), _ttoi( ( (CDB_StringID *)vecSmartControlValveTypeList.at( i ) )->GetIDstr() ) - 1, vecSmartControlValveTypeList.at( i ) );
	}

	i = ( int )vecSmartControlValveTypeList.size();
	vecSmartControlValveTypeList.clear();
	return i;
}

int CTADatabase::GetSmartControlValveTypeList( CRankEx *pSrcList, CRankEx *pDestList )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the smart control valves of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclSmartControlValve = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclSmartControlValve );

		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		// Add the new type ID if doesn't already exist.
		IDPTR TypeIDPtr = pclSmartControlValve->GetTypeIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( TypeIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)TypeIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)TypeIDPtr.MP )->GetString(), dKey, (LPARAM)TypeIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}

int CTADatabase::GetSmartControlValveFamilyList( CRankEx *pList, _string strSelTypeID, FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bIsForSet )
{
	// Fill the list according to selected type.
	CTable *pclSmartControlValveTable = static_cast<CTable *>( Get( _T("SMARTCONTROLVALVE_TAB") ).MP );
	ASSERT( NULL != pclSmartControlValveTable );

	for( IDPTR IDPtr = pclSmartControlValveTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclSmartControlValveTable->GetNext( IDPtr.MP ) )
	{
		CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( IDPtr.MP );
		ASSERT( NULL != pclSmartControlValve );

		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		if( SmartValveTypeControl != pclSmartControlValve->GetSmartValveType() )
		{
			// It's not a smart control valve.
			continue;
		}

		// Selectable ?
		if( false == pclSmartControlValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSmartControlValve, eFilterSelection ) )
		{
			continue;
		}

		if( true == bIsForSet && false == pclSmartControlValve->IsPartOfaSet() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pclSmartControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pclSmartControlValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey;
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		dKey = pclSmartControlValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pclSmartControlValve->GetFamilyIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pclSmartControlValve->GetFamilyIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclSmartControlValve->GetFamilyIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetSmartControlValveFamilyList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the smart control valves of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclSmartControlValve = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclSmartControlValve );

		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclSmartControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Add the new family ID if doesn't already exist.
		IDPTR FamilyIDPtr = pclSmartControlValve->GetFamilyIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( FamilyIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)FamilyIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)FamilyIDPtr.MP )->GetString(), dKey, (LPARAM)FamilyIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetSmartControlValveBdyMatList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bIsForSet )
{
	// Fill the list according to selected type.
	CTable *pclSmartControlValveTable = static_cast<CTable *>( Get( _T("SMARTCONTROLVALVE_TAB") ).MP );
	ASSERT( NULL != pclSmartControlValveTable );

	for( IDPTR IDPtr = pclSmartControlValveTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclSmartControlValveTable->GetNext( IDPtr.MP ) )
	{
		CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( IDPtr.MP );
		ASSERT( NULL != pclSmartControlValve );

		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		if( SmartValveTypeControl != pclSmartControlValve->GetSmartValveType() )
		{
			// It's not a smart control valve.
			continue;
		}

		// Selectable ?
		if( false == pclSmartControlValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSmartControlValve, eFilterSelection ) )
		{
			continue;
		}

		if( true == bIsForSet && false == pclSmartControlValve->IsPartOfaSet() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pclSmartControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pclSmartControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test DN
		int iDN = pclSmartControlValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Bdy | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pclSmartControlValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pclSmartControlValve->GetBodyMaterialIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pclSmartControlValve->GetBodyMaterialIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclSmartControlValve->GetBodyMaterialIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetSmartControlValveBdyMatList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the valves of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclSmartControlValve = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclSmartControlValve );

		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclSmartControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclSmartControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Add the new material ID if doesn't already exist.
		IDPTR MaterialIDPtr = pclSmartControlValve->GetBodyMaterialIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( MaterialIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)MaterialIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)MaterialIDPtr.MP )->GetString(), dKey, (LPARAM)MaterialIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetSmartControlValveConnList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID, FilterSelection eFilterSelection, int iDNMin,
		int iDNMax, bool bIsForSet )
{
	// Fill the list according to selected type
	CTable *pclSmartControlValveTable = static_cast<CTable *>( Get( _T("SMARTCONTROLVALVE_TAB") ).MP );
	ASSERT( NULL != pclSmartControlValveTable );

	for( IDPTR IDPtr = pclSmartControlValveTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclSmartControlValveTable->GetNext( IDPtr.MP ) )
	{
		CDB_SmartControlValve *pclSmartValve = dynamic_cast<CDB_SmartControlValve *>( IDPtr.MP );
		ASSERT( NULL != pclSmartValve );

		if( NULL == pclSmartValve )
		{
			continue;
		}

		if( SmartValveTypeControl != pclSmartValve->GetSmartValveType() )
		{
			// It's not a smart control valve.
			continue;
		}

		// Selectable ?
		if( false == pclSmartValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSmartValve, eFilterSelection ) )
		{
			continue;
		}

		if( true == bIsForSet && false == pclSmartValve->IsPartOfaSet() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pclSmartValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pclSmartValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pclSmartValve->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pclSmartValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Connect | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pclSmartValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pclSmartValve->GetConnectIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pclSmartValve->GetConnectIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclSmartValve->GetConnectIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetSmartControlValveConnList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, _string strMaterialID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the smart control valves of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclSmartControlValve = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclSmartControlValve );

		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclSmartControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclSmartControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test material ID.
		if( false == strMaterialID.empty() && strMaterialID != pclSmartControlValve->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Add the new connection ID if doesn't already exist.
		IDPTR ConnectIDPtr = pclSmartControlValve->GetConnectIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( ConnectIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)ConnectIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)ConnectIDPtr.MP )->GetString(), dKey, (LPARAM)ConnectIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetSmartControlValveVersList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID, _string strSelConnectID,
		FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bIsForSet )
{
	CTable *pclSmartControlValveTable = static_cast<CTable *>( Get( _T("SMARTCONTROLVALVE_TAB") ).MP );
	ASSERT( NULL != pclSmartControlValveTable );

	for( IDPTR IDPtr = pclSmartControlValveTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclSmartControlValveTable->GetNext( IDPtr.MP ) )
	{
		CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( IDPtr.MP );
		ASSERT( NULL != pclSmartControlValve );

		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		if( SmartValveTypeControl != pclSmartControlValve->GetSmartValveType() )
		{
			// It's not a smart control valve.
			continue;
		}

		// Keep only CDB_TAProduct and children.
		if( false == pclSmartControlValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSmartControlValve, eFilterSelection ) )
		{
			continue;
		}

		if( true == bIsForSet && false == pclSmartControlValve->IsPartOfaSet() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pclSmartControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pclSmartControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pclSmartControlValve->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		if( false == strSelConnectID.empty() && strSelConnectID != pclSmartControlValve->GetConnectIDPtr().ID )
		{
			continue;
		}

		if( 0 == _tcscmp( pclSmartControlValve->GetVersionIDPtr().ID, _T("VERS_EMPTY") ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pclSmartControlValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Version | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pclSmartControlValve->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pclSmartControlValve->GetVersionIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pclSmartControlValve->GetVersionIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclSmartControlValve->GetVersionIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetSmartControlValveVersList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, _string strMaterialID,
		_string strConnectID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the smart control valves of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclSmartControlValve = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclSmartControlValve );

		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclSmartControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclSmartControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test material ID.
		if( false == strMaterialID.empty() && strMaterialID != pclSmartControlValve->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Test connection ID.
		if( false == strConnectID.empty() && strConnectID != pclSmartControlValve->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Add the new version ID if doesn't already exist.
		IDPTR VersionIDPtr = pclSmartControlValve->GetVersionIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( VersionIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)VersionIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)VersionIDPtr.MP )->GetString(), dKey, (LPARAM)VersionIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetSmartControlValvePNList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID, _string strSelConnectID,
		_string strSelVersionID, FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bIsForSet )
{
	// Fill the PN combo according to selected type, connection and version.
	CTable *pclSmartControlValveTable = static_cast<CTable *>( Get( _T("SMARTCONTROLVALVE_TAB") ).MP );
	ASSERT( NULL != pclSmartControlValveTable );

	// Loop on selectable smart control valves (Those with Pmax = Pmax of previous item are not treated).
	for( IDPTR IDPtr = pclSmartControlValveTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclSmartControlValveTable->GetNext( IDPtr.MP ) )
	{
		CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( IDPtr.MP );
		ASSERT( NULL != pclSmartControlValve );

		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		if( SmartValveTypeControl != pclSmartControlValve->GetSmartValveType() )
		{
			// It's not a smart control valve.
			continue;
		}

		// Keep only CDB_TAProduct and children.
		if( false == pclSmartControlValve->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSmartControlValve, eFilterSelection ) )
		{
			continue;
		}

		if( true == bIsForSet && false == pclSmartControlValve->IsPartOfaSet() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pclSmartControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pclSmartControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pclSmartControlValve->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		if( false == strSelConnectID.empty() && strSelConnectID != pclSmartControlValve->GetConnectIDPtr().ID )
		{
			continue;
		}

		if( false == strSelVersionID.empty() && strSelVersionID != pclSmartControlValve->GetVersionIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pclSmartControlValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		CRankEx clPNList;
		pclSmartControlValve->GetPNList( &clPNList );

		LPARAM lpParam;
		_string str;

		for( bool bContinue = clPNList.GetFirst( str, lpParam ); true == bContinue; bContinue = clPNList.GetNext( str, lpParam ) )
		{
			CDB_StringID *pDBStringID = dynamic_cast<CDB_StringID *>( (CData *)lpParam );

			if( NULL == pDBStringID )
			{
				continue;
			}

			double dKey = 0;
			pDBStringID->GetIDstrAs( 0, dKey );

			if( false == pList->IfExist( dKey ) )
			{
				pList->Add( pDBStringID->GetString(), dKey, (LPARAM)pDBStringID );
			}
		}
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetSmartControlValvePNList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, _string strMaterialID,
		_string strConnectID, _string strVersionID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the smart control valves of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclSmartControlValve = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclSmartControlValve );

		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclSmartControlValve->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclSmartControlValve->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test material ID.
		if( false == strMaterialID.empty() && strMaterialID != pclSmartControlValve->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Test connection ID.
		if( false == strConnectID.empty() && strConnectID != pclSmartControlValve->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Test version ID.
		if( false == strVersionID.empty() && strVersionID != pclSmartControlValve->GetVersionIDPtr().ID )
		{
			continue;
		}

		// Add the new PN ID if doesn't already exist.
		IDPTR PNIDPtr = pclSmartControlValve->GetPNIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( PNIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)PNIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)PNIDPtr.MP )->GetString(), dKey, (LPARAM)PNIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}

#endif

int CTADatabase::GetSmartControlValveList( CRankEx *pList, _string strSelTypeID, _string strSelFamilyID, _string strSelBdyMatID, _string strSelConnectID,
		_string strSelVersionID, _string strSelPNID, FilterSelection eFilterSelection, int iDNMin, int iDNMax, CData *pProd, bool bIsForTools, bool bIsForSet )
{
	// By default smart control valves are defined in 'SMARTCONTROLVALVE_TAB'.
	CTable *pclSmartControlValveTable = static_cast<CTable *>( Get( _T("SMARTCONTROLVALVE_TAB") ).MP );
	ASSERT( NULL != pclSmartControlValveTable );

	// For increasing 'if' test below.
	CDB_StringID *pSelTypeID = NULL;
	CDB_StringID *pSelFamilyID = NULL;
	CDB_StringID *pSelBdyMatID = NULL;
	CDB_StringID *pSelConnectID = NULL;
	CDB_StringID *pSelVersionID = NULL;
	CDB_StringID *pSelPNID = NULL;

	if( false == strSelTypeID.empty() )
	{
		pSelTypeID = ( (CDB_StringID *)Get( strSelTypeID.c_str() ).MP );
	}

	if( false == strSelFamilyID.empty() )
	{
		pSelFamilyID = ( (CDB_StringID *)Get( strSelFamilyID.c_str() ).MP );
	}

	if( false == strSelBdyMatID.empty() )
	{
		pSelBdyMatID = ( (CDB_StringID *)Get( strSelBdyMatID.c_str() ).MP );
	}

	if( false == strSelConnectID.empty() )
	{
		pSelConnectID = ( (CDB_StringID *)Get( strSelConnectID.c_str() ).MP );
	}

	if( false == strSelVersionID.empty() )
	{
		pSelVersionID = ( (CDB_StringID *)Get( strSelVersionID.c_str() ).MP );
	}

	if( false == strSelPNID.empty() )
	{
		pSelPNID = ( (CDB_StringID *)Get( strSelPNID.c_str() ).MP );
	}

	// HYS-1221.
	bool bOldProductsAvailableForHC = ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Loop on selectable smart control valves.
	for( IDPTR IDPtr = pclSmartControlValveTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclSmartControlValveTable->GetNext( IDPtr.MP ) )
	{
		// Keep only CDB_TAProduct and children.
		CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( IDPtr.MP );

		if( NULL == pclSmartControlValve )
		{
			continue;
		}

		if( SmartValveTypeControl != pclSmartControlValve->GetSmartValveType() )
		{
			// It's not a smart control valve.
			continue;
		}

		if( false == pclSmartControlValve->IsSelectable( true, false, bOldProductsAvailableForHC, bIsForTools ) )
		{
			continue;
		}

		if( true == MustExclude( pclSmartControlValve, eFilterSelection ) )
		{
			continue;
		}

		if( true == bIsForSet && false == pclSmartControlValve->IsPartOfaSet() )
		{
			continue;
		}

		if( NULL != pSelTypeID && pSelTypeID != ( (CDB_StringID *)pclSmartControlValve->GetTypeIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelFamilyID && pSelFamilyID != ( (CDB_StringID *)pclSmartControlValve->GetFamilyIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelBdyMatID && pSelBdyMatID != ( (CDB_StringID *)pclSmartControlValve->GetBodyMaterialIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelConnectID && pSelConnectID != ( (CDB_StringID *)pclSmartControlValve->GetConnectIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelVersionID && pSelVersionID != ( (CDB_StringID *)pclSmartControlValve->GetVersionIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelPNID )
		{
			if( false == pclSmartControlValve->IsPNIncludedInPNRange( strSelPNID ) )
			{
				continue;
			}
		}

		// Test DN.
		int iDN = pclSmartControlValve->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Already added ?
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)IDPtr.MP;

		// Verify the lparam doesn't exist yet.
		if( !pList->GetaCopy( str, dKey, lparam ) )
		{
			// Found product having priority.
			double dKey = pclSmartControlValve->GetSortingKey();
			pList->Add( pclSmartControlValve->GetName(), dKey, (LPARAM)pclSmartControlValve );
		}
	}

	return pList->GetCount();
}


int CTADatabase::GetSmartDpCTypeList( CRankEx *pList, FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bIsForSet )
{
	CTable *pclTableSmartDpCType = static_cast<CTable *>( Get( _T("SMARTDPCTYPE_TAB") ).MP );

	if( NULL == pclTableSmartDpCType )
	{
		return 0;
	}

	std::vector <LPARAM> vecSmartDpCTypeList;
	vecSmartDpCTypeList.resize( pclTableSmartDpCType->GetItemCount() );

	int i = 0;

	for( IDPTR IDPtr = pclTableSmartDpCType->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *IDPtr.ID; IDPtr = pclTableSmartDpCType->GetNext() )
	{
		ASSERT( i < ( int )vecSmartDpCTypeList.size() );
		vecSmartDpCTypeList.at( i ) = (LPARAM)IDPtr.MP;
		i++;
	}

	// Remove unused elements.
	vecSmartDpCTypeList.resize( i );

	// Try to find at least one smart differential pressure controller available for this type.
	CTable *pclSmartValveTable = static_cast<CTable *>( Get( _T("SMARTDIFFPRESS_TAB") ).MP );
	ASSERT( NULL != pclSmartValveTable );

	for( i = 0; i < ( int )vecSmartDpCTypeList.size(); )
	{
		CDB_StringID *pSID = dynamic_cast<CDB_StringID *>( (CData *)vecSmartDpCTypeList[i] );
		ASSERT( NULL != pSID );

		if( NULL == pSID )
		{
			continue;
		}

		_string strType = pSID->GetIDPtr().ID;
		bool bSelectable = false;

		for( IDPTR IDPtr = pclSmartValveTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclSmartValveTable->GetNext( IDPtr.MP ) )
		{
			CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( (CData *)IDPtr.MP );

			if( NULL == pclSmartDpC )
			{
				continue;
			}

			if( SmartValveTypeDpC != pclSmartDpC->GetSmartValveType() )
			{
				// It's not a smart differential pressure controller.
				continue;
			}

			if( false == pclSmartDpC->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( pclSmartDpC, eFilterSelection ) )
			{
				continue;
			}

			if( true == bIsForSet && false == pclSmartDpC->IsPartOfaSet() )
			{
				continue;
			}

			// Scan all smart differential pressure controllers with the same type.
			if( strType != pclSmartDpC->GetTypeIDPtr().ID )
			{
				continue;
			}

			int iDN = pclSmartDpC->GetSizeKey();

			if( iDN < iDNMin )
			{
				continue;
			}

			if( iDN > iDNMax )
			{
				continue;
			}

			bSelectable = true;

			// Loop aborted one smart differential pressure controller of this type is enough to display the type.
			break;
		}

		if( false == bSelectable )
		{
			vecSmartDpCTypeList.erase( vecSmartDpCTypeList.begin() + i );
		}
		else
		{
			i++;
		}
	}

	// Transfer and sort list.
	for( i = 0; i < ( int )vecSmartDpCTypeList.size(); i++ )
	{
		pList->Add( ( (CDB_StringID *)vecSmartDpCTypeList.at( i ) )->GetString(), _ttoi( ( (CDB_StringID *)vecSmartDpCTypeList.at( i ) )->GetIDstr() ) - 1, vecSmartDpCTypeList.at( i ) );
	}

	i = ( int )vecSmartDpCTypeList.size();
	vecSmartDpCTypeList.clear();
	return i;
}

int CTADatabase::GetSmartDpCTypeList( CRankEx *pSrcList, CRankEx *pDestList )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the smart differential pressure controllers of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclSmartDpC = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclSmartDpC );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		// Add the new type ID if doesn't already exist.
		IDPTR TypeIDPtr = pclSmartDpC->GetTypeIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( TypeIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)TypeIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)TypeIDPtr.MP )->GetString(), dKey, (LPARAM)TypeIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}

int CTADatabase::GetSmartDpCFamilyList( CRankEx *pList, _string strSelTypeID, FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bIsForSet )
{
	// Fill the list according to selected type.
	CTable *pclSmartDpCTable = static_cast<CTable *>( Get( _T("SMARTDIFFPRESS_TAB") ).MP );
	ASSERT( NULL != pclSmartDpCTable );

	for( IDPTR IDPtr = pclSmartDpCTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclSmartDpCTable->GetNext( IDPtr.MP ) )
	{
		CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( IDPtr.MP );
		ASSERT( NULL != pclSmartDpC );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		if( SmartValveTypeDpC != pclSmartDpC->GetSmartValveType() )
		{
			// It's not a smart differential pressure controller.
			continue;
		}

		// Selectable ?
		if( false == pclSmartDpC->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSmartDpC, eFilterSelection ) )
		{
			continue;
		}

		if( true == bIsForSet && false == pclSmartDpC->IsPartOfaSet() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pclSmartDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pclSmartDpC->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		double dKey;
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Fam | CTAPSortKey::TAPSKM_PriorL );
		dKey = pclSmartDpC->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pclSmartDpC->GetFamilyIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pclSmartDpC->GetFamilyIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclSmartDpC->GetFamilyIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetSmartDpCFamilyList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the smart differential pressure controllers of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclSmartDpC = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclSmartDpC );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclSmartDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Add the new family ID if doesn't already exist.
		IDPTR FamilyIDPtr = pclSmartDpC->GetFamilyIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( FamilyIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)FamilyIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)FamilyIDPtr.MP )->GetString(), dKey, (LPARAM)FamilyIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetSmartDpCBdyMatList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, FilterSelection eFilterSelection, int iDNMin, int iDNMax,
		bool bIsForSet )
{
	// Fill the list according to selected type.
	CTable *pclSmartDpCTable = static_cast<CTable *>( Get( _T("SMARTDIFFPRESS_TAB") ).MP );
	ASSERT( NULL != pclSmartDpCTable );

	for( IDPTR IDPtr = pclSmartDpCTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclSmartDpCTable->GetNext( IDPtr.MP ) )
	{
		CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( IDPtr.MP );
		ASSERT( NULL != pclSmartDpC );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		if( SmartValveTypeDpC != pclSmartDpC->GetSmartValveType() )
		{
			// It's not a smart differential pressure controller.
			continue;
		}

		// Selectable ?
		if( false == pclSmartDpC->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSmartDpC, eFilterSelection ) )
		{
			continue;
		}

		if( true == bIsForSet && false == pclSmartDpC->IsPartOfaSet() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pclSmartDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pclSmartDpC->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test DN
		int iDN = pclSmartDpC->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Bdy | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pclSmartDpC->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pclSmartDpC->GetBodyMaterialIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pclSmartDpC->GetBodyMaterialIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclSmartDpC->GetBodyMaterialIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetSmartDpCBdyMatList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the smart differential pressure controllers of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclSmartDpC = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclSmartDpC );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclSmartDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclSmartDpC->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Add the new material ID if doesn't already exist.
		IDPTR MaterialIDPtr = pclSmartDpC->GetBodyMaterialIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( MaterialIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)MaterialIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)MaterialIDPtr.MP )->GetString(), dKey, (LPARAM)MaterialIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetSmartDpCConnList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID, FilterSelection eFilterSelection, int iDNMin,
		int iDNMax, bool bIsForSet )
{
	// Fill the list according to selected type.
	CTable *pclSmartDpCTable = static_cast<CTable *>( Get( _T("SMARTDIFFPRESS_TAB") ).MP );
	ASSERT( NULL != pclSmartDpCTable );

	for( IDPTR IDPtr = pclSmartDpCTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclSmartDpCTable->GetNext( IDPtr.MP ) )
	{
		CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( IDPtr.MP );
		ASSERT( NULL != pclSmartDpC );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		if( SmartValveTypeDpC != pclSmartDpC->GetSmartValveType() )
		{
			// It's not a smart differential pressure controller.
			continue;
		}

		// Selectable ?
		if( false == pclSmartDpC->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSmartDpC, eFilterSelection ) )
		{
			continue;
		}

		if( true == bIsForSet && false == pclSmartDpC->IsPartOfaSet() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pclSmartDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pclSmartDpC->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pclSmartDpC->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pclSmartDpC->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Connect | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pclSmartDpC->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pclSmartDpC->GetConnectIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pclSmartDpC->GetConnectIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclSmartDpC->GetConnectIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetSmartDpCConnList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, _string strMaterialID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the smart differential pressure controllers of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclSmartDpC = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclSmartDpC );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclSmartDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclSmartDpC->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test material ID.
		if( false == strMaterialID.empty() && strMaterialID != pclSmartDpC->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Add the new connection ID if doesn't already exist.
		IDPTR ConnectIDPtr = pclSmartDpC->GetConnectIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( ConnectIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)ConnectIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)ConnectIDPtr.MP )->GetString(), dKey, (LPARAM)ConnectIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetSmartDpCVersList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID, _string strSelConnectID,
		FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bIsForSet )
{
	CTable *pclSmartDpCTable = static_cast<CTable *>( Get( _T("SMARTDIFFPRESS_TAB") ).MP );
	ASSERT( NULL != pclSmartDpCTable );

	for( IDPTR IDPtr = pclSmartDpCTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclSmartDpCTable->GetNext( IDPtr.MP ) )
	{
		CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( IDPtr.MP );
		ASSERT( NULL != pclSmartDpC );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		if( SmartValveTypeDpC != pclSmartDpC->GetSmartValveType() )
		{
			// It's not a smart differential pressure controller.
			continue;
		}

		// Keep only CDB_TAProduct and children.
		if( false == pclSmartDpC->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSmartDpC, eFilterSelection ) )
		{
			continue;
		}

		if( true == bIsForSet && false == pclSmartDpC->IsPartOfaSet() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pclSmartDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pclSmartDpC->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pclSmartDpC->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		if( false == strSelConnectID.empty() && strSelConnectID != pclSmartDpC->GetConnectIDPtr().ID )
		{
			continue;
		}

		if( 0 == _tcscmp( pclSmartDpC->GetVersionIDPtr().ID, _T("VERS_EMPTY") ) )
		{
			continue;
		}

		// Test DN.
		int iDN = pclSmartDpC->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Found product having priority.
		CTAPSortKey::CTAPSKMask clCTAPSKMask( CTAPSortKey::TAPSKM_Version | CTAPSortKey::TAPSKM_PriorL );
		double dKey = (double)pclSmartDpC->GetSortingKey( &clCTAPSKMask );

		// Already added ?
		_string InListstr;
		LPARAM InListlparam = (LPARAM)(void *)pclSmartDpC->GetVersionIDPtr().MP;
		double dInListkey = 0;

		if( pList->GetaCopy( InListstr, dInListkey, InListlparam ) )
		{
			// Yes take a copy, check priority key, and keep only the most important.
			if( dKey < dInListkey )
			{
				pList->Delete( dInListkey );
			}
			else
			{
				continue;
			}
		}

		pList->Add( ( (CDB_StringID *)pclSmartDpC->GetVersionIDPtr().MP )->GetString(), dKey, (LPARAM)(void *)pclSmartDpC->GetVersionIDPtr().MP );
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetSmartDpCVersList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, _string strMaterialID,
		_string strConnectID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the smart differential pressure controllers of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclSmartDpC = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclSmartDpC );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclSmartDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclSmartDpC->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test material ID.
		if( false == strMaterialID.empty() && strMaterialID != pclSmartDpC->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Test connection ID.
		if( false == strConnectID.empty() && strConnectID != pclSmartDpC->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Add the new version ID if doesn't already exist.
		IDPTR VersionIDPtr = pclSmartDpC->GetVersionIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( VersionIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)VersionIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)VersionIDPtr.MP )->GetString(), dKey, (LPARAM)VersionIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}
#endif

int CTADatabase::GetSmartDpCPNList( CRankEx *pList, _string strSelTypeID, _string strSelFamID, _string strSelBdyMatID, _string strSelConnectID,
		_string strSelVersionID, FilterSelection eFilterSelection, int iDNMin, int iDNMax, bool bIsForSet )
{
	// Fill the PN combo according to selected type, connection and version.
	CTable *pclSmartDpCTable = static_cast<CTable *>( Get( _T("SMARTDIFFPRESS_TAB") ).MP );
	ASSERT( NULL != pclSmartDpCTable );

	// Loop on selectable smart differential pressure controllers (Those with Pmax = Pmax of previous item are not treated).
	for( IDPTR IDPtr = pclSmartDpCTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclSmartDpCTable->GetNext( IDPtr.MP ) )
	{
		CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( IDPtr.MP );
		ASSERT( NULL != pclSmartDpC );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		if( SmartValveTypeDpC != pclSmartDpC->GetSmartValveType() )
		{
			// It's not a smart differential pressure controller.
			continue;
		}

		// Keep only CDB_TAProduct and children.
		if( false == pclSmartDpC->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pclSmartDpC, eFilterSelection ) )
		{
			continue;
		}

		if( true == bIsForSet && false == pclSmartDpC->IsPartOfaSet() )
		{
			continue;
		}

		if( false == strSelTypeID.empty() && strSelTypeID != pclSmartDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		if( false == strSelFamID.empty() && strSelFamID != pclSmartDpC->GetFamilyIDPtr().ID )
		{
			continue;
		}

		if( false == strSelBdyMatID.empty() && strSelBdyMatID != pclSmartDpC->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		if( false == strSelConnectID.empty() && strSelConnectID != pclSmartDpC->GetConnectIDPtr().ID )
		{
			continue;
		}

		if( false == strSelVersionID.empty() && strSelVersionID != pclSmartDpC->GetVersionIDPtr().ID )
		{
			continue;
		}

		// Test DN.
		int iDN = pclSmartDpC->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		CRankEx clPNList;
		pclSmartDpC->GetPNList( &clPNList );

		LPARAM lpParam;
		_string str;

		for( bool bContinue = clPNList.GetFirst( str, lpParam ); true == bContinue; bContinue = clPNList.GetNext( str, lpParam ) )
		{
			CDB_StringID *pDBStringID = dynamic_cast<CDB_StringID *>( (CData *)lpParam );

			if( NULL == pDBStringID )
			{
				continue;
			}

			double dKey = 0;
			pDBStringID->GetIDstrAs( 0, dKey );

			if( false == pList->IfExist( dKey ) )
			{
				pList->Add( pDBStringID->GetString(), dKey, (LPARAM)pDBStringID );
			}
		}
	}

	return pList->GetCount();
}

#ifndef TACBX
int CTADatabase::GetSmartDpCPNList( CRankEx *pSrcList, CRankEx *pDestList, _string strTypeID, _string strFamilyID, _string strMaterialID,
		_string strConnectID, _string strVersionID )
{
	if( NULL == pSrcList || NULL == pDestList )
	{
		return 0;
	}

	pDestList->PurgeAll();

	_string str;
	LPARAM lParam;

	// Do the loop on the differential pressure controllers of the source list.
	for( bool bContinue = pSrcList->GetFirst( str, lParam ); true == bContinue; bContinue = pSrcList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pclSmartDpC = dynamic_cast<CDB_TAProduct*>( (CData *)lParam );
		ASSERT( NULL != pclSmartDpC );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		// Test type ID.
		if( false == strTypeID.empty() && strTypeID != pclSmartDpC->GetTypeIDPtr().ID )
		{
			continue;
		}

		// Test family ID.
		if( false == strFamilyID.empty() && strFamilyID != pclSmartDpC->GetFamilyIDPtr().ID )
		{
			continue;
		}

		// Test material ID.
		if( false == strMaterialID.empty() && strMaterialID != pclSmartDpC->GetBodyMaterialIDPtr().ID )
		{
			continue;
		}

		// Test connection ID.
		if( false == strConnectID.empty() && strConnectID != pclSmartDpC->GetConnectIDPtr().ID )
		{
			continue;
		}

		// Test version ID.
		if( false == strVersionID.empty() && strVersionID != pclSmartDpC->GetVersionIDPtr().ID )
		{
			continue;
		}

		// Add the new PN ID if doesn't already exist.
		IDPTR PNIDPtr = pclSmartDpC->GetPNIDPtr();
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)( PNIDPtr.MP );

		if( false == pDestList->GetaCopy( str, dKey, lparam ) )
		{
			dKey = (double)_ttoi( ( (CDB_StringID *)PNIDPtr.MP )->GetIDstr() );
			pDestList->Add( ( (CDB_StringID *)PNIDPtr.MP )->GetString(), dKey, (LPARAM)PNIDPtr.MP );
		}
	}

	return pDestList->GetCount();
}

#endif

int CTADatabase::GetSmartDpCList( CRankEx *pList, _string strSelTypeID, _string strSelFamilyID, _string strSelBdyMatID, _string strSelConnectID,
		_string strSelVersionID, _string strSelPNID, FilterSelection eFilterSelection, int iDNMin, int iDNMax, CData *pProd, bool bIsForTools, bool bIsForSet )
{
	// By default smart differential pressure controllers are defined in 'SMARTDPCTYPE_TAB'.
	CTable *pclSmartDpCTable = static_cast<CTable *>( Get( _T("SMARTDIFFPRESS_TAB") ).MP );
	ASSERT( NULL != pclSmartDpCTable );

	// For increasing 'if' test below.
	CDB_StringID *pSelTypeID = NULL;
	CDB_StringID *pSelFamilyID = NULL;
	CDB_StringID *pSelBdyMatID = NULL;
	CDB_StringID *pSelConnectID = NULL;
	CDB_StringID *pSelVersionID = NULL;
	CDB_StringID *pSelPNID = NULL;

	if( false == strSelTypeID.empty() )
	{
		pSelTypeID = ( (CDB_StringID *)Get( strSelTypeID.c_str() ).MP );
	}

	if( false == strSelFamilyID.empty() )
	{
		pSelFamilyID = ( (CDB_StringID *)Get( strSelFamilyID.c_str() ).MP );
	}

	if( false == strSelBdyMatID.empty() )
	{
		pSelBdyMatID = ( (CDB_StringID *)Get( strSelBdyMatID.c_str() ).MP );
	}

	if( false == strSelConnectID.empty() )
	{
		pSelConnectID = ( (CDB_StringID *)Get( strSelConnectID.c_str() ).MP );
	}

	if( false == strSelVersionID.empty() )
	{
		pSelVersionID = ( (CDB_StringID *)Get( strSelVersionID.c_str() ).MP );
	}

	if( false == strSelPNID.empty() )
	{
		pSelPNID = ( (CDB_StringID *)Get( strSelPNID.c_str() ).MP );
	}

	// HYS-1221.
	bool bOldProductsAvailableForHC =  ( ForHMCalc == eFilterSelection ) ? true : false;
	
	// Loop on selectable differential pressure controllers.
	for( IDPTR IDPtr = pclSmartDpCTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclSmartDpCTable->GetNext( IDPtr.MP ) )
	{
		// Keep only CDB_TAProduct and children.
		CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( IDPtr.MP );

		if( NULL == pclSmartDpC )
		{
			continue;
		}

		//if( SmartValveTypeDpC != pclSmartValve->GetSmartValveType() )
		//{
		//	// It's not a smart differential pressure controller.
		//	continue;
		//}

		if( false == pclSmartDpC->IsSelectable( true, false, bOldProductsAvailableForHC, bIsForTools ) )
		{
			continue;
		}

		if( true == MustExclude( pclSmartDpC, eFilterSelection ) )
		{
			continue;
		}

		if( true == bIsForSet && false == pclSmartDpC->IsPartOfaSet() )
		{
			continue;
		}
		
		// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view
		if( true == bIsForSet && false == pclSmartDpC->IsPartOfaSet() )
		{
			continue;
		}
		
		// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view
		if( false == bIsForSet && true == pclSmartDpC->IsPartOfaSet() && ePartOfaSetYes == pclSmartDpC->GetPartOfaSet() )
		{
			continue;
		}

		if( NULL != pSelTypeID && pSelTypeID != ( (CDB_StringID *)pclSmartDpC->GetTypeIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelFamilyID && pSelFamilyID != ( (CDB_StringID *)pclSmartDpC->GetFamilyIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelBdyMatID && pSelBdyMatID != ( (CDB_StringID *)pclSmartDpC->GetBodyMaterialIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelConnectID && pSelConnectID != ( (CDB_StringID *)pclSmartDpC->GetConnectIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelVersionID && pSelVersionID != ( (CDB_StringID *)pclSmartDpC->GetVersionIDPtr().MP ) )
		{
			continue;
		}

		if( NULL != pSelPNID )
		{
			if( false == pclSmartDpC->IsPNIncludedInPNRange( strSelPNID ) )
			{
				continue;
			}
		}

		// Test DN.
		int iDN = pclSmartDpC->GetSizeKey();

		if( iDN < iDNMin )
		{
			continue;
		}

		if( iDN > iDNMax )
		{
			continue;
		}

		// Already added ?
		double dKey = 0;
		_string str;
		LPARAM lparam = (LPARAM)IDPtr.MP;

		// Verify the lparam doesn't exist yet
		if( !pList->GetaCopy( str, dKey, lparam ) )
		{
			// Found product having priority.
			double dKey = pclSmartDpC->GetSortingKey();
			pList->Add( pclSmartDpC->GetName(), dKey, (LPARAM)pclSmartDpC );
		}
	}

	return pList->GetCount();
}


#ifndef TACBX
bool CTADatabase::GetTAPListFromCBIDef( CRank *pList, CString CBIType, CString CBISize )
{
	ASSERT( CBIType.GetLength() );
	ASSERT( CBISize.GetLength() );

	if( 0 == CBIType.GetLength() || 0 == CBISize.GetLength() )
	{
		return false;
	}

	bool bFlag = false;
	CTable *pTab = static_cast<CTable *>( Get( _T("REGVALV_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_TAProduct *pTAP = ( CDB_TAProduct * )IDPtr.MP;

		if( NULL == pTAP )
		{
			continue;
		}

		// Test if string is not empty.
		if( !*pTAP->GetCBIType() )
		{
			continue;
		}

		if( _tcscmp( pTAP->GetCBIType(), ( LPCTSTR )CBIType ) )
		{
			continue;
		}

		if( !*pTAP->GetCBISize() && !*pTAP->GetCBISizeInch() )
		{
			continue;
		}

		if( _tcscmp( pTAP->GetCBISize(), ( LPCTSTR )CBISize ) && _tcscmp( pTAP->GetCBISizeInch(), ( LPCTSTR )CBISize ) )
		{
			continue;
		}

		// Add Valve to the List (Compose sorting key).
		int iFkey = _ttoi( ( (CDB_StringID *)pTAP->GetFamilyIDPtr().MP )->GetIDstr() );
		int iVkey = _ttoi( ( (CDB_StringID *)pTAP->GetVersionIDPtr().MP )->GetIDstr() );
		int iKey = 10000 * pTAP->GetPriorityLevel() + 100 * iFkey + iVkey;

		pList->Add( IDPtr.ID, iKey, (LPARAM)pTAP );
		bFlag = true;
	}

	return bFlag;
}

int CTADatabase::GetAccessories( CRank *pList, CTable *pTab, FilterSelection eFilterSelection, bool bApplyCrossingTable )
{
	VERIFY( NULL != pTab );
	
	// HYS-1108: Don't displayed the crossing accessory in the 'Crossing' map.
	std::vector<CDB_Product*> VectAccList;

	if( true == bApplyCrossingTable )
	{
		CDB_RuledTableBase *pRuledTableBase = dynamic_cast<CDB_RuledTableBase *>( pTab );

		for( IDPTR IDPtr = pRuledTableBase->GetFirst(); NULL != IDPtr.MP; IDPtr = pRuledTableBase->GetNext() )
		{
			// 'IsACrossingAcc' returns 'true' if the accessory in the current CDB_RuledTable must not be shown.
			if( true == pRuledTableBase->IsACrossingAcc( IDPtr.MP ) )
			{
				// Keep the accessory in a list for the second loop.
				CDB_RuledTableBase::MMap *pMap = NULL;
				pMap = pRuledTableBase->GetMap( CDB_RuledTableBase::MMapName::Crossing );
				CDB_RuledTableBase::MMap::iterator it = pMap->find( IDPtr.ID );

				if( pMap->end() != it )
				{
					CDB_RuledTableBase::RuledTableItem rRuledTableItem = it->second;
					CDB_Product *pclAccessoryToHide = dynamic_cast<CDB_Product *>( rRuledTableItem.IDPtr.MP );
					VectAccList.push_back( pclAccessoryToHide );
					continue;
				}
			}
		}
	}

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_Product *pAcc = dynamic_cast<CDB_Product *>( IDPtr.MP );
		VERIFY( NULL != pAcc );

		if( NULL == pAcc || false == pAcc->IsAnAccessory() )
		{
			continue;
		}

		if( false == pAcc->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pAcc, eFilterSelection ) )
		{
			continue;
		}

		if( VectAccList.size() > 0 )
		{
			if( std::find( VectAccList.begin(), VectAccList.end(), pAcc ) != VectAccList.end() )
			{
				continue;
			}
		}
		
		pList->AddStrSort( CString( IDPtr.ID ), 0, (LPARAM)pAcc, true );
	}

	return pList->GetCount();
}

int CTADatabase::GetSetDpCMeasValve( CRank *pList, CDB_DpController *pDpC )
{
	VERIFY( NULL != pDpC );
	std::set<CDB_Set *> DpCSet;
	std::set<CDB_Set *>::const_iterator It;

	if( NULL == pDpC )
	{
		return 0;
	}

	if( true == pDpC->IsPartOfaSet() )
	{
		// Establish a list of available Set for this control valve.
		CTableSet *pSetTab = (CTableSet *)( pDpC->GetTableSet() );

		if( NULL == pSetTab )
		{
			ASSERT( 0 );
			return 0;
		}

		if( pSetTab->FindCompatibleSet( &DpCSet, pDpC->GetIDPtr().ID, _T( "" ) ) )
		{
			for( It = DpCSet.begin(); It != DpCSet.end(); ++It )
			{
				// Store matching measuring valve.
				CDB_TAProduct *pTap = dynamic_cast<CDB_TAProduct *>( ( *It )->GetSecondIDPtr().MP );
				double dKey = pTap->GetKeyOrder();
				pList->Add( pTap->GetName(), dKey, (LPARAM)pTap, true, false );
			}
		}
	}

	ASSERT( pList->GetCount() );
	return pList->GetCount();
}

int CTADatabase::GetPipeSerieList( CRank *pList, FilterSelection eFilterSelection )
{
	CRank PipeList( false );
	CTable *pTab = TASApp.GetpPipeDB()->GetPipeTab();
	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CTable ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		if( true == IDPtr.MP->IsSelectable( true ) )
		{
			if( false == PipeList.IfExist( ( static_cast<CTable *>( IDPtr.MP ) )->GetName() ) )
			{
				PipeList.AddStrSort( ( static_cast<CTable *>( IDPtr.MP ) )->GetName(), 0, (LPARAM)IDPtr.MP, false );
			}
		}
	}

	// Try to find at least one pipe available in this series.
	CString str;
	pTab = NULL;

	for( BOOL bContinue = PipeList.GetFirstT<CTable *>( str, pTab ); TRUE == bContinue; bContinue = PipeList.GetNextT<CTable *>( str, pTab ) )
	{
		ASSERT( NULL != pTab );
		bool bSelectable = false;

		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			if( false == ( ( CDB_Thing * )IDPtr.MP )->IsSelectable( true ) )
			{
				continue;
			}

			if( true == MustExclude( ( CDB_Thing * )IDPtr.MP, eFilterSelection ) )
			{
				continue;
			}

			bSelectable = true;
			break;
		}

		if( false == bSelectable )
		{
			PipeList.DeleteT<CTable *>( str, pTab );
		}
	}

	LPARAM lParam = 0;

	// Transfer PipeList.
	for( BOOL bContinue = PipeList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = PipeList.GetNext( str, lParam ) )
	{
		pList->AddStrSort( str, 0, lParam, false );
	}

	PipeList.PurgeAll();
	return pList->GetCount();
}

int CTADatabase::GetPipeSizeList( CRank *pList, CTable *pPipeSerieTab, FilterSelection eFilterSelection )
{
	if( NULL == pPipeSerieTab )
	{
		return 0;
	}

	// Fill the Pipe size list according to the pipes series table.
	for( IDPTR IDPtr = pPipeSerieTab->GetFirst( CLASS( CDB_Pipe ) ); _T('\0') != *IDPtr.ID; IDPtr = pPipeSerieTab->GetNext() )
	{
		CDB_Pipe *pPipe = ( CDB_Pipe * )( IDPtr.MP );

		if( NULL == pPipe || false == pPipe->IsSelectable( true ) )
		{
			continue;
		}

		if( true == MustExclude( pPipe, eFilterSelection ) )
		{
			continue;
		}

		pList->Add( pPipe->GetName(), pPipe->GetIntDiameter(), (LPARAM)pPipe, true, false );
	}

	return pList->GetCount();
}

bool CTADatabase::GetTAProdDNRange( CRankEx *pList, CDB_StringID **pStrIDDNMin, CDB_StringID **pStrIDDNMax )
{
	if( NULL == pList )
	{
		return false;
	}

	_string str;
	LPARAM lParam;

	int iDNMin = INT_MAX;
	int iDNMax = 0;

	for( bool bContinue = pList->GetFirst( str, lParam ); TRUE == bContinue; bContinue = pList->GetNext( str, lParam ) )
	{
		CDB_TAProduct *pTAProduct = dynamic_cast<CDB_TAProduct *>( ( CData * )lParam );
		ASSERT( NULL != pTAProduct );

		CDB_StringID *pclStringID = dynamic_cast<CDB_StringID *>( ( CData * )pTAProduct->GetSizeIDPtr().MP );
		ASSERT( NULL != pclStringID );

		int iDN = pTAProduct->GetSizeKey();

		if( iDN < iDNMin )
		{
			iDNMin = iDN;
			*pStrIDDNMin = pclStringID;
		}

		if( iDN > iDNMax )
		{
			iDNMax = iDN;
			*pStrIDDNMax = pclStringID;
		}
	}

	if( iDNMin < iDNMax )
	{
		return true;
	}

	return false;
}

double CTADatabase::GetPrice( CString strArticleNumber )
{
	CDB_PriceList *pclPriceList = (CDB_PriceList*)( Get( _T("PRICE_LIST") ).MP );

	if( NULL == pclPriceList )
	{
		return 0.0;
	}

	double dPrice = pclPriceList->GetPrice( strArticleNumber );

	if( dPrice <= 0.0 && true == TASApp.IsLocalArtNumberUsed() )
	{
		CString strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
		strLocalArticleNumber.Trim();
		dPrice = pclPriceList->GetPrice( strLocalArticleNumber );
	}

	return dPrice;
}

bool CTADatabase::CanEditSelection( CData *pclSelection )
{
	bool bDpController = false;
	bool bDpDpReliefValve = false;
	bool bTrv = false;
	bool bCV = false;
	bool bBCV = false;
	bool bPICV = false;
	bool bAirVentSep = false;
	bool bPressMaint = false;
	bool bSafetyValve = false;
	bool b6WayValve = false;
	bool bTapWaterControl = false;
	bool bSmartControlValve = false;
	bool bSmartDpC = false;
	bool bFloorHeatingManifold = false;
	bool bFloorHeatingValve = false;
	bool bFloorHeatingController = false;

	if( NULL != dynamic_cast<CDS_SSelDpC *>( pclSelection ) )
	{
		bDpController = true;
	}

	if( NULL != dynamic_cast<CDS_SSelDpReliefValve *>( pclSelection ) )
	{
		bDpDpReliefValve = true;
	}

	if( NULL != dynamic_cast<CDS_SSelRadSet *>( pclSelection ) )
	{
		bTrv = true;
	}

	if( NULL != dynamic_cast<CDS_SSelCv *>( pclSelection ) )
	{
		bCV = true;
	}

	if( NULL != dynamic_cast<CDS_SSelBCv *>( pclSelection ) )
	{
		bBCV = true;
	}

	if( NULL != dynamic_cast<CDS_SSelPICv *>( pclSelection ) )
	{
		bPICV = true;
	}

	if( NULL != dynamic_cast<CDS_SSelAirVentSeparator *>( pclSelection ) )
	{
		bAirVentSep = true;
	}

	if( NULL != dynamic_cast<CDS_SSelPMaint *>( pclSelection ) )
	{
		bPressMaint = true;
	}

	if( NULL != dynamic_cast<CDS_SSelSafetyValve *>( pclSelection ) )
	{
		bSafetyValve = true;
	}

	if( NULL != dynamic_cast<CDS_SSel6WayValve *>( pclSelection ) )
	{
		b6WayValve = true;
	}

	if( NULL != dynamic_cast<CDS_SSelTapWaterControl *>( pclSelection ) )
	{
		bTapWaterControl = true;
	}

	if( NULL != dynamic_cast<CDS_SSelSmartControlValve *>( pclSelection ) )
	{
		bSmartControlValve = true;
	}

	if( NULL != dynamic_cast<CDS_SSelSmartDpC *>( pclSelection ) )
	{
		bSmartControlValve = true;
	}

	if( NULL != dynamic_cast<CDS_SSelFloorHeatingManifold *>( pclSelection ) )
	{
		bFloorHeatingManifold = true;
	}

	if( NULL != dynamic_cast<CDS_SSelFloorHeatingValve *>( pclSelection ) )
	{
		bFloorHeatingValve = true;
	}

	if( NULL != dynamic_cast<CDS_SSelFloorHeatingController *>( pclSelection ) )
	{
		bFloorHeatingController = true;
	}

	bool bReturn = false;

	if( ( false == bDpController && false == bTrv && false == bCV && false == bBCV && false == bPICV && false == bAirVentSep )
			|| ( true == bDpController && true == TASApp.IsDpCDisplayed() )
			|| ( true == bDpDpReliefValve && true == TASApp.IsDpCDisplayed() )
			|| ( true == bTrv && true == TASApp.IsTrvDisplayed() )
			|| ( true == bCV && true == TASApp.IsCvDisplayed() )
			|| ( true == bBCV && true == TASApp.IsBCvDisplayed() )
			|| ( true == bPICV && true == TASApp.IsPICvDisplayed() )
			|| ( true == bAirVentSep && true == TASApp.IsAirVentSepDisplayed() )
			|| ( true == bPressMaint && true == TASApp.IsPressureMaintenanceDisplayed() ) 
			|| ( true == bSafetyValve && true == TASApp.IsSafetyValveDisplayed() ) 
			|| ( true == b6WayValve && true == TASApp.Is6WayCVDisplayed() )
			|| ( true == bTapWaterControl && true == TASApp.IsTapWaterControlDisplayed() ) 
			|| ( true == bSmartControlValve && true == TASApp.IsSmartControlValveDisplayed() ) 
			|| ( true == bSmartDpC && true == TASApp.IsSmartDpCDisplayed() ) 
			|| ( true == bFloorHeatingManifold && true == TASApp.IsFloorHeatingControlDisplayed() ) 
			|| ( true == bFloorHeatingValve && true == TASApp.IsFloorHeatingControlDisplayed() )
			|| ( true == bFloorHeatingController && true == TASApp.IsFloorHeatingControlDisplayed() ) )
	{
		bReturn = true;
	}

	return bReturn;
}
#endif //TACBX////////////////////////////////////////////////////////

bool CTADatabase::MustExclude( CDB_Thing *pclDBThing, FilterSelection eFilterSelection )
{
	if( NULL == pclDBThing )
	{
		return false;
	}

	bool bReturn = false;

	switch( eFilterSelection )
	{
		case NoFiltering:
			break;

		case ForIndAndBatchSel:
			bReturn = !pclDBThing->IsAvailableForIndividualSelection();
			break;

		case ForDirSel:
			bReturn = !pclDBThing->IsAvailableForDirectSelection();
			break;

		case ForHMCalc:
			bReturn = !pclDBThing->IsAvailableForHMCalc();
			break;
	}

	return bReturn;
}

///////////////////////////////////////////////////////////////////////////////////
//
//								CUserDatabase
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef TACBX
CUserDatabase::CUserDatabase()
	: CDataBase( CDataBase::UserDatabase )
{
}
#endif //TACBX////////////////////////////////////////////////////////
