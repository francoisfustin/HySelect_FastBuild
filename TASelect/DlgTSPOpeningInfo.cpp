
#include "stdafx.h"
#include "TASelect.h"
#include "Select.h"
#include "HydroMod.h"

#include "DlgTSPOpeningInfo.h"


CDlgTSPOpeningInfo::CDlgTSPOpeningInfo( CWnd *pParent )
: CDlgListBox(pParent), CDeletedProductHelper()
{
}

CDlgTSPOpeningInfo::~CDlgTSPOpeningInfo()
{
}

void CDlgTSPOpeningInfo::Display( bool bForUnitTest )
{
	// Variables.
	CRect rect;
	bool bDisplay = false;

	// Initialize the list control's columns.
	SetColumn( 5 );
	UseCustCol( true );
	SetColWidth( ecProblem, 23 );
	SetColWidth( ecWhere, 23 );
	SetColWidth( ecValve, 22 );
	SetColWidth( ecValveID, 22 );
	SetColWidth( ecNumber, 9 );
	AddHeaderStr( ecProblem, IDS_DLGRESOLCBIPLANTMODIF_HEADERPROBLEM );
	AddHeaderStr( ecWhere, IDS_DLGRESOLCBIPLANTMODIF_HEADERWHERE );
	AddHeaderStr( ecValve, IDS_DLGRESOLCBIPLANTMODIF_HEADERPROD );
	AddHeaderStr( ecValveID, IDS_DLGRESOLCBIPLANTMODIF_HEADERPRODID );
	AddHeaderStr( ecNumber, IDS_DLGRESOLCBIPLANTMODIF_HEADERNUMBER );

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SCAN FIRST ALL INDIVIDUAL SELECTION

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VerifyIndividualSelection( bDisplay);

	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SCAN HYDROCALC
	CTableHM *pTableHM = dynamic_cast<CTableHM *>( m_pTADS->GetpHydroModTable() );
	
	// HYS-1291 : Show the checkbox or not.
	bool bButtonLockDeletedHM = false;
	
	// HYS-1713: Do not show the checkbox when opening file without hydromod
	if (NULL != pTableHM && NULL != pTableHM->GetFirst().MP )
	{
		VerifyHydroMod( pTableHM->GetIDPtr(), bDisplay );
		bButtonLockDeletedHM = bDisplay;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Display the DialogListBox
	if( true == bDisplay && false == bForUnitTest )
	{
		_FillListCtrl();
		CDlgListBox::Display( IDS_DLGTSPOPENINGINFO_CAPTION, IDS_DLGTSPOPENINGINFO_STATICTITLE, IDS_DLGTSPOPENINGINFO_STATICPENDINGITEMS, IDB_CANNOTSENDSIGNS,
				false, bButtonLockDeletedHM );
	}
}

void CDlgTSPOpeningInfo::LockDeletedProductForHydromod()
{
	multimap< CDS_HydroMod *, CDS_HydroMod::eHMObj > mapHMdeleted = *GetHMMapDeleted();
	multimap< CDS_HydroMod *, CDS_HydroMod::eHMObj >::iterator it;

	bool bIsFreezed = TASApp.GetpTADS()->GetpProjectParams()->IsFreezed();
	TASApp.GetpTADS()->GetpProjectParams()->SetFreeze( false );

	for( it = mapHMdeleted.begin(); it != mapHMdeleted.end(); it++ )
	{
		CDS_HydroMod *pChild = (CDS_HydroMod *)( it->first );
		if( NULL != pChild )
		{
			pChild->SetLock( it->second, true );
		}
	}

	TASApp.GetpTADS()->GetpProjectParams()->SetFreeze( bIsFreezed );
}

void CDlgTSPOpeningInfo::_FillListCtrl()
{
	std::map<FromWhere, std::map<CString, Info> > *pMap;

	// HYS-1868: Add compatibility error.
	// Loop all kind of error: undefined, deleted, not available and not compatible.
	for( int i = 0; i < 4; i++ )
	{
		int imgIndex;
		CString strProblem;

		if( 0 == i )
		{
			pMap = &m_mapUndef;
			strProblem = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_UNDEFPROD );
			imgIndex = 0;
		}
		else if( 1 == i )
		{
			pMap = &m_mapDeleted;
			strProblem = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_DELPROD );
			imgIndex = 1;
		}
		else if( 2 == i )
		{
			pMap = &m_mapNotAvailable;
			strProblem = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_PRODNOTAVAILABLE );
			imgIndex = 1;
		}
		else
		{
			pMap = &m_mapNotCompatible;
			strProblem = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_PRODNOTCOMPATIBLE );
			imgIndex = 1;
		}

		// Loop all origins: from individual selection, direct selection and hydraulic circuit.
		for( mapWhereInfoIter iterOrigin = pMap->begin(); iterOrigin != pMap->end(); iterOrigin++ )
		{
			mapIDInfo *pmapOrigin = &iterOrigin->second;

			if( 0 == pmapOrigin->size() )
			{
				// If nothing continue.
				continue;
			}

			CString strWhere;
			
			if( FromIndividualSelection == iterOrigin->first )
			{
				strWhere = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMINDSEL );
			}
			else if( FromDirectSelection == iterOrigin->first )
			{
				strWhere = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMDIRECTSEL );
			}
			else
			{
				strWhere = TASApp.LoadLocalizedString( IDS_DLGTSPOPENINGINFO_FROMHMCALC );
			}

			for( mapIDInfoIter iterInfo = pmapOrigin->begin(); iterInfo != pmapOrigin->end(); iterInfo++ )
			{
				AddStr( ecProblem, strProblem );
				AddStr( ecWhere, strWhere );
				AddStr( ecValve, iterInfo->second.m_strValve );
				AddStr( ecValveID, iterInfo->first );
				CString strNumber;
				strNumber.Format( _T("%i"), iterInfo->second.m_iNumber );
				AddStr( ecNumber, strNumber );
				AddLine( (LPARAM)AddStrToList( imgIndex, iterInfo->second.m_strBottomMessage ) );
			}
		}
	}
}

void CDlgTSPOpeningInfo::OnOK()
{
	CDlgListBox::OnOK();
	// HYS-1291 : Lock all hydromod deleted product
	if( true == IsLockHMDeletedProductEnable() )
	{
		LockDeletedProductForHydromod();
	}
}

