//
// HubHandler.cpp: implementation of the CHubHandler class.
// Tools class for Hub managing
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TASelect.h"
#include "HMInclude.h"

#include "dlgcomboboxhm.h"
#include "HubHandler.h"

CHubHandler::CHubHandler( void )
{
	m_pTADB = TASApp.GetpTADB();						ASSERT( m_pTADB );
	m_pTADS = TASApp.GetpTADS();						ASSERT( m_pTADS );
	m_HubIDPtr = _NULL_IDPTR;
	m_EnBmp = new CEnBitmap();
}

CHubHandler::~CHubHandler( void )
{
	delete m_EnBmp;
}

// Return IDPTR of parent table
IDPTR CHubHandler::Attach( CString strHubID )
{
	IDPTR idptrReturn = _NULL_IDPTR;

	m_HubIDPtr = m_pTADS->Get( (LPCTSTR)strHubID );
	if( NULL != *m_HubIDPtr.ID )
		idptrReturn = m_HubIDPtr.PP->GetIDPtr();

	return idptrReturn;
}

// Create a Hub partner valve and all stations
IDPTR CHubHandler::CreateHub(	CString strTabID,
								CDS_HmHub::Application Application,
								UINT uiStations,
								CDS_HmHub::MainFeedPos MainFeedPos, 
								bool fBypass,
								CString strBalTypeID,
								IDPTR StationsConnectIDPtr,
								CDS_HmHub::SelMode SelMode,
								bool fNew,
								CString strID_WC )
{
	try
	{
		CTable *pTabHub = (CTable *)( m_pTADS->Get( (LPCTSTR)strTabID ).MP );
		
		if(	NULL == pTabHub	)
		{
			HYSELECT_THROW( _T("Internal error: Can't retrieve the object '%s' from the datastruct."), strTabID );
		}

		CDS_HmHub *pHmHub = (CDS_HmHub *)m_HubIDPtr.MP;
	
		// Always create a new Hub when we are not into the temporary table
		if( _T('\0') == *m_HubIDPtr.ID )
		{
			fNew = true;
		}
	
		if( false == fNew )
		{
			CDS_HmHub *pOldHmHub = (CDS_HmHub *)m_HubIDPtr.MP;
		
			// Change balancing type
			if( pOldHmHub->GetBalTypeID() != strBalTypeID )
			{		
				IDPTR idptr;
				m_pTADS->CreateObject( idptr, CLASS( CDS_HmHub ) );
				pTabHub->Insert( idptr );

				pHmHub = (CDS_HmHub *)idptr.MP;
				pHmHub->Init( strBalTypeID, pTabHub );
				IDPTR SchemeIDPtr = pHmHub->GetSchemeIDPtr();
				pOldHmHub->Copy( pHmHub, false, false );
			
				// BalType is overwritten by Copy function,  
				pHmHub->SetSchemeIDPtr( SchemeIDPtr );
				pHmHub->SetBalTypeID( strBalTypeID );
				pHmHub->SetLock( CDS_HydroMod::eHMObj::eALL, false, false );
			
				// Copy Children
				for( IDPTR ChildIDPtr = pOldHmHub->GetFirst(); NULL != *ChildIDPtr.ID; ChildIDPtr = pOldHmHub->GetNext() )
				{
					pOldHmHub->Remove( ChildIDPtr );
					pHmHub->Insert( ChildIDPtr );
				}
			
				pHmHub->SetReturnValveID( _T("") );
				pHmHub->SetSupplyValveID( _T("") );
				pHmHub->SetLock( CDS_HydroMod::eHMObj::eALL, false, false );

				// Delete old Hub
				Delete( m_HubIDPtr );
				m_HubIDPtr = idptr;
			}
		}
	
		// for a new Hub Delete current existing Hub 
		if( true == fNew ) 
		{
			Delete( m_HubIDPtr);
	
			IDPTR idptr;
			m_pTADS->CreateObject( idptr, CLASS( CDS_HmHub ) );
			pTabHub->Insert( idptr );

			pHmHub = (CDS_HmHub *)idptr.MP;
			pHmHub->Init( strBalTypeID, pTabHub );
			m_HubIDPtr = idptr;
		
			// An Hub can be added only into a Table or into something derived from CDS_HydroMod
			pHmHub->SetLevel( 0 );

			if( true == pTabHub->IsHMFamily() )
			{
				pHmHub->SetLevel( ( (CDS_HydroMod*)pHmHub )->GetLevel() + 1 );
			}

			pHmHub->SetPos( -1 );
		
			CTable *pTab = m_pTADS->GetpHUBSelectionTable();
		
			int iPos = pTab->GetItemCount();
			pHmHub->SetPos( iPos );
			CString str = pHmHub->RenameMe( false );

			if( NULL != m_pTADS->FindHydroMod( str, pTab, pHmHub ) )
			{
				m_pTADS->DifferentiateHMName( pTab, pHmHub->GetHMName() );
			}
		}
	
		// Refresh water characteristics
		pHmHub->SetpWC( _T("PARAM_WC") );
		pHmHub->SetApplication( Application );
		pHmHub->SetMainFeedPos( MainFeedPos );
		pHmHub->SetBypass( fBypass );
		pHmHub->SetFlagModule( true );
		pHmHub->SetSelMode( SelMode );
		pHmHub->SetStationsConnectIDPtr( StationsConnectIDPtr );

		// Create stations if needed
		// Verify and type of balancing valve available
		CDlgComboBoxHM dlg( NULL );
		dlg.InitList( pHmHub, CDlgComboBoxHM::eSCB::SCB_DS_StationValveType );
	
		CString str;
		LPARAM lParam = 0;
		dlg.GetFirstListItem( str, lParam );
		
		if( NULL == lParam )
		{
			HYSELECT_THROW( _T("Internal error: Can't retrieve first element in the list.") );
		}
	
		CString RvTypeID = _T("RVTYPE_BV");
	
		if( NULL != lParam )
		{
			( (CDB_StringID *)lParam )->GetIDPtr().ID;
		}
	
		for( UINT uiLoopStation = pHmHub->GetNumberOfStations(); uiLoopStation < uiStations; uiLoopStation++ )
		{
			IDPTR StationIDPtr;
			m_pTADS->CreateObject( StationIDPtr, CLASS( CDS_HmHubStation ) );
			pHmHub->Insert( StationIDPtr );
		
			CDS_HmHubStation *pHubStation = (CDS_HmHubStation *)StationIDPtr.MP;
			pHubStation->Init( RvTypeID, pHmHub );
			pHubStation->SetLevel( pHmHub->GetLevel() + 1 );
			pHubStation->SetPos( uiLoopStation + 1 );
			pHubStation->SetHMName( _T("*") );
		}
	
		// Remove stations if needed
		if( pHmHub->GetNumberOfStations() > uiStations )
		{
			for( UINT uiLoopStation = pHmHub->GetNumberOfStations(); uiLoopStation > uiStations; uiLoopStation-- )
			{
				IDPTR idptr = GetStationIDPTR( uiLoopStation );
				Delete( idptr );
			}
		}
	
		//Rename Stations
		pHmHub->RenameStations();

		// When the selection is done in Direct sel mode or in Q mode some value must be reseted
		if( CDS_HmHub::SelMode::Direct == SelMode || CDS_HmHub::SelMode::Q == SelMode )
		{
			for( IDPTR idptr = pHmHub->GetFirst(); _T('\0') != *idptr.ID; idptr = pHmHub->GetNext( idptr.MP ) )
			{
				// Disable parent's signaling
				if( CDS_HmHub::SelMode::Direct == SelMode )
				{
					( (CDS_HmHubStation *)idptr.MP)->GetpTermUnit()->SetQ( 0 );
				}

				if( SelMode != CDS_HmHub::SelMode::LoopDetails )
				{
					// Reset Pipe length when we are not in loop details
					( (CDS_HmHubStation *)idptr.MP )->GetpCircuitPrimaryPipe()->SetLength( 0 );
				
					// Reset TU Dp when we are not in loop details or in QDpLoop 
					if( SelMode != CDS_HmHub::SelMode::QDpLoop )
					{
						( (CDS_HmHubStation *)idptr.MP )->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Dp, 0 );
					}
				}
			}
		}
		
		return _NULL_IDPTR;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CHubHandler::CreateHub'."), __LINE__, __FILE__ );
		throw;
	}
}

// Return Hub Image
bool CHubHandler::BuildHubImg()
{
	return GetHubImg( m_EnBmp );
}

bool CHubHandler::GetBoxImg( CEnBitmap *pEnBmp )
{
	//Låda 4:  745x465x200
	//Låda 6:  895x465x200
	//Låda 8: 1045x465x200
	struct LBH {
		double dL;
		double dB;
		double dH;
	} size[3] = {
				0.745,0.465,0.2,
				0.895,0.465,0.2,
				1.045,0.465,0.2
				};

	HMODULE h = TASApp.GetDLLHandle( _HUB_SCHEMES_DLL );
	if( NULL == *m_HubIDPtr.ID )
		return false;
	
	if( NULL == pEnBmp )
		return false;
	
	CDS_HmHub *pHmHub = (CDS_HmHub *)m_HubIDPtr.MP;
	UINT uiBoxSize = pHmHub->GetNumberOfStations();
	CString strSize, strL, strB, strH;
	if( uiBoxSize <= 4 )
	{
		pEnBmp->LoadImage( GIF_BOX_4_ARROWS, _T("GIF"), h );
		strL = WriteCUDouble( _U_LENGTH, size[0].dL );
		strB = WriteCUDouble( _U_LENGTH, size[0].dB );
		strH = WriteCUDouble( _U_LENGTH, size[0].dH );
	}
	else if( uiBoxSize <= 6 )
	{
		pEnBmp->LoadImage( GIF_BOX_6_ARROWS, _T("GIF"), h );
		strL = WriteCUDouble( _U_LENGTH, size[1].dL );
		strB = WriteCUDouble( _U_LENGTH, size[1].dB );
		strH = WriteCUDouble( _U_LENGTH, size[1].dH );
	}
	else /*Size == 8*/
	{
		pEnBmp->LoadImage( GIF_BOX_8_ARROWS, _T("GIF"), h );
		strL = WriteCUDouble( _U_LENGTH, size[2].dL );
		strB = WriteCUDouble( _U_LENGTH, size[2].dB );
		strH = WriteCUDouble( _U_LENGTH, size[2].dH );
	}
	
	FormatString( strSize, IDS_SSELH_HUBBOXSIZE, strL, strB, strH );
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	strSize += CString( _T(" ") ) + pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str();
	
	CFont Text12;
	Text12.CreateFont( -38, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial") );
	pEnBmp->DrawText( 100, 430, strSize, 0, &Text12 );
	return true;
}

bool CHubHandler::GetHubImg( CEnBitmap *pEnBmp )
{
	HMODULE h =TASApp.GetDLLHandle( _HUB_SCHEMES_DLL );
	
	if( NULL == *m_HubIDPtr.ID )
		return false;

	// Supply and Return partner valve
	CDS_HmHub *pHmHub = (CDS_HmHub *)m_HubIDPtr.MP;
	IDPTR SupplyValveIDPtr, ReturnValveIDPtr;
	SupplyValveIDPtr = _NULL_IDPTR;
	ReturnValveIDPtr = _NULL_IDPTR;

	CEnBitmapRow *pTrgBmp= new CEnBitmapRow();
	CEnBitmap EnBmpPartner;
	CEnBitmap EnBmpEnd;
	CEnBitmap EnBmpNR[8];

	for( int uiLoopStation = 0; uiLoopStation < 8; uiLoopStation++ )
		EnBmpNR[uiLoopStation].LoadImage( GIF_DIGIT1 + uiLoopStation, _T("GIF"), h );

	//Partner
	if( NULL != *pHmHub->GetReturnValveIDPtr().ID )
	{
		CDB_HubValv *pHubValv = (CDB_HubValv *)( m_pTADB->Get( pHmHub->GetReturnValveID() ).MP );				ASSERT( pHubValv );
		int iImageID = pHubValv->GetImageID();
		if( NULL != pHmHub->GetpDpC() && eMvLoc::MvLocSecondary == pHmHub->GetpDpC()->GetMvLoc() )
			iImageID += GIF_HUB_OFFSET_MVSECONDARY;
		EnBmpPartner.LoadImage( iImageID, _T("GIF"), h );
		pTrgBmp->AddCEnBitmap( &EnBmpPartner );
	}
	EnBmpEnd.LoadImage( GIF_END, _T("GIF"), h );
	
	CString str;
	UINT uiNumberOfStation = pHmHub->GetNumberOfStations();
	
	// Stations connection INTERNAL/EXTENSION
	int iGifOffset = 0;
	CDB_MultiStringExt *pStationConn = (CDB_MultiStringExt *)( pHmHub->GetStationsConnectIDPtr().MP );			ASSERT( pStationConn );
	if( NULL != pStationConn )
	{
		CString str = pStationConn->GetString( CDS_HmHub::ExtConn::eTAISConnType );
		if( 0 == str.CompareNoCase( _T("EXTENSION") ) )
			iGifOffset += GIF_STATION_OFFSET_EXT;
	}
	
	for( UINT uiLoopStation = 0; uiLoopStation < uiNumberOfStation; uiLoopStation++ )
	{
		CEnBitmap EnBmpTemp;
		CEnBitmap EnBmpPart;

		CDS_HmHubStation *pHubStation = (CDS_HmHubStation *)( GetStationIDPTR( uiLoopStation + 1 ).MP );	
		if( true == pHubStation->IsPlugged() )
		{
			CEnBitmap EnBmpPart;
			EnBmpPart.LoadImage( GIF_STATION_PLUG, _T("GIF"), h );
			
			// Flip Image if needed
			if( CDS_HmHub::MainFeedPos::Right == pHmHub->GetMainFeedPos() )
				( (CEnBitmap*)&(EnBmpNR[uiLoopStation]) )->FlipImage( 1 );

			// Paste Image 18,420 are x,y coordinates into the slice
			EnBmpPart.PasteCEnBitmap( 18, 420, &(EnBmpNR[uiLoopStation]) );
			pTrgBmp->AddCEnBitmap( &EnBmpPart );	
			EnBmpPart.DeleteObject();
		}
		else if( NULL != *pHubStation->GetReturnValveID() )
		{
			CDB_HubStaValv *pHubStaValv = (CDB_HubStaValv *)( m_pTADB->Get( pHubStation->GetReturnValveID() ).MP );			ASSERT( pHubStaValv );
			int iImageID = pHubStaValv->GetImageID();
			iImageID += iGifOffset;
			
			// Actuator ?
			CDB_Actuator *pActr = dynamic_cast<CDB_Actuator *>( pHubStation->GetReturnActuatorIDPtr().MP );
			if( NULL != pActr )
			{
				if( 0 == IDcmp( pActr->GetFamilyIDPtr().ID, _ID( ACTFAM_TSEM ) ) )
					iImageID += GIF_STATION_OFFSET_TSEM;
				else
					iImageID += GIF_STATION_OFFSET_TSE;
			}
			
			// Load Image
			EnBmpPart.LoadImage( iImageID, _T("GIF"), h );
			EnBmpTemp.CopyImage( &EnBmpPart );
			
			// Flip Image if needed
			if( CDS_HmHub::MainFeedPos::Right == pHmHub->GetMainFeedPos() )
				( (CEnBitmap*)&(EnBmpNR[uiLoopStation]) )->FlipImage( 1 );
			
			// Paste Image 18,420 are x,y coordinates into the slice
			EnBmpTemp.PasteCEnBitmap( 18, 420, &(EnBmpNR[uiLoopStation]) );
			pTrgBmp->AddCEnBitmap( &EnBmpTemp );	
		}
		EnBmpTemp.DeleteObject();
		EnBmpPart.DeleteObject();
	}
	
	UINT uiPos = uiNumberOfStation;
	// Add Empty slots if needed
	for( UINT uiLoopStation = 0; uiLoopStation < GetNumberOfEmptySlots(); uiLoopStation++ )
	{
		CEnBitmap EnBmpPart;
		EnBmpPart.LoadImage( GIF_STATION_PLUG, _T("GIF"), h );
		
		// Flip Image if needed
		if( CDS_HmHub::MainFeedPos::Right == pHmHub->GetMainFeedPos() )
			( (CEnBitmap*)&(EnBmpNR[uiPos]) )->FlipImage( 1 );
		
		// Paste Image 18,420 are x,y coordinates into the slice
		EnBmpPart.PasteCEnBitmap( 18, 420, &(EnBmpNR[uiPos]) );
		uiPos++;
		pTrgBmp->AddCEnBitmap( &EnBmpPart );	
		EnBmpPart.DeleteObject();
	}
	
	// Add End Slot
	pTrgBmp->AddCEnBitmap( &EnBmpEnd );

	pTrgBmp->Compute_CEnBitmap();

	// Add Label
	CEnBitmap EnBmpLabel;
	EnBmpLabel.LoadImage( GIF_LABELTAHUB, _T("GIF"), h );
	pTrgBmp->PasteCEnBitmap( 400, 194, &EnBmpLabel );
	EnBmpLabel.DeleteObject();
	
	if( CDS_HmHub::MainFeedPos::Right == pHmHub->GetMainFeedPos() )
		( (CEnBitmap*)pTrgBmp)->FlipImage( 1 );

	pEnBmp->CopyImage( pTrgBmp );
	pTrgBmp->DeleteObject();
	delete pTrgBmp;
	EnBmpPartner.DeleteObject();
	EnBmpEnd.DeleteObject();

	return true;
}

IDPTR CHubHandler::GetStationIDPTR( int iStationPosition )
{
	if( NULL == *m_HubIDPtr.ID )
		return _NULL_IDPTR;
	
	CDS_HmHub *pHmHub = (CDS_HmHub *)m_HubIDPtr.MP;
	for( IDPTR idptr = pHmHub->GetFirst(); NULL != *idptr.ID; idptr = pHmHub->GetNext() )
	{
		CDS_HmHubStation *pHubStation = (CDS_HmHubStation *)idptr.MP;	
		if( pHubStation->GetPos() ==  iStationPosition )
			return idptr;
	}
	return _NULL_IDPTR;
}

IDPTR CHubHandler::Copy( CString ID )
{
	return Copy( (CTable *)( m_pTADS->Get( (LPCTSTR)ID ).MP ) );
}

IDPTR CHubHandler::Copy( CTable *pTab )
{
	try
	{
		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: 'pTab' argument can't be NULL.") );
		}

		if( NULL == m_HubIDPtr.MP )
		{
			HYSELECT_THROW( _T("Internal error: The 'm_HubIDPtr' object can't be NULL.") );
		}
	
		IDPTR idptr;
		m_pTADS->CreateObject( idptr, CLASS( CDS_HmHub ) );
		pTab->Insert( idptr );

		CDS_HmHub *pHmHub = (CDS_HmHub *)( idptr.MP );
		CDS_HmHub *pOrgHmHub = (CDS_HmHub *)( m_HubIDPtr.MP );
	
		if( NULL == pHmHub || NULL == pOrgHmHub )
		{
			return _NULL_IDPTR;
		}
	
		CString BalTypeID = pOrgHmHub->GetBalTypeID();
		pHmHub->Init( BalTypeID, pTab );
		pOrgHmHub->Copy( pHmHub, false, false );

		// BalType is overwritten by Copy function,  
		pHmHub->SetBalTypeID( BalTypeID );

		// Copy Children
		for( IDPTR ChildIDPtr = pOrgHmHub->GetFirst(); _T('\0') != *ChildIDPtr.ID; ChildIDPtr = pOrgHmHub->GetNext() )
		{
			CDS_HmHubStation *pOrgStation = (CDS_HmHubStation *)( ChildIDPtr.MP );
		
			IDPTR idptr;
			m_pTADS->CreateObject( idptr, CLASS( CDS_HmHubStation ) );
			pHmHub->Insert( idptr );
		
			CDS_HmHubStation *pNewStation = (CDS_HmHubStation *)( idptr.MP );
			pNewStation->Init( pOrgStation->GetBalTypeID(), pHmHub );
			pOrgStation->Copy( pNewStation, false, false );
		}
	
		return idptr;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CHubHandler::Copy'."), __LINE__, __FILE__ );
		throw;
	}
}

void CHubHandler::Delete( IDPTR IDPtr )
{
	if( NULL != *IDPtr.ID )
	{
		// could be delete in SelProd Sheet...
		if( *m_pTADS->Get( IDPtr.ID ).ID )
			m_pTADS->DeleteHM( (CDS_HydroMod *)IDPtr.MP );
		if( 0 == _tcscmp( IDPtr.ID, m_HubIDPtr.ID ) )
			m_HubIDPtr = _NULL_IDPTR;
	}
}

UINT CHubHandler::GetNumberOfEmptySlots()
{
	if( NULL != *m_HubIDPtr.ID )
	{
		UINT iNumberOfStation = ( (CDS_HmHub *)m_HubIDPtr.MP )->GetNumberOfStations();
		for( UINT i = 4; i <= 8; i += 2 )
		{
			if( iNumberOfStation <= i )
				return i - iNumberOfStation;
		}
	}
	return 0;
}

bool CHubHandler::MoveToSelectedTab()
{
	try
	{
		CTable *pTabHub = (CTable *)( m_pTADS->Get( _T("TMPHUB_TAB") ).MP );
		
		if( NULL == pTabHub )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'TMPHUB_TAB' table from the datastruct.") );
		}

		if( _T('\0') != *m_HubIDPtr.ID )
		{
			if( m_HubIDPtr.PP != pTabHub )
			{
				return false;
			}

			pTabHub->Remove( m_HubIDPtr );
		
			CTable *pTabSelHub = m_pTADS->GetpHUBSelectionTable();
			
			if( NULL == pTabSelHub )
			{
				HYSELECT_THROW( _T("Internal error: can't retrieve the 'SELHUB_TAB' table from the datastruct.") );
			}
		
			pTabSelHub->Insert( m_HubIDPtr );
			return true;
		}

		return false;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CHubHandler::MoveToSelectedTab'."), __LINE__, __FILE__ );
		throw;
	}
}

CDS_HmHub *CHubHandler::ModifySelectedProduct()
{
	try
	{
		if( _T('\0') == *m_HubIDPtr.ID )
		{
			return NULL;
		}
	
		CDS_HmHub *pSelHub = (CDS_HmHub *)( m_HubIDPtr.MP );
	
		// If exist delete Hub from TmpTab.
		CTable *pTabHub = (CTable *)( m_pTADS->Get( _T("TMPHUB_TAB") ).MP );

		if( NULL == pTabHub )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'TMPHUB_TAB' table from the datastruct.") );
		}
	
		// Verify that there is no modification in course.  If yes warn the user
		for( IDPTR idptr = pTabHub->GetFirst(); _T('\0') != *idptr.ID; idptr = pTabHub->GetNext() )
		{
			CDS_HmHub *pHub = (CDS_HmHub *)( idptr.MP );

			if( NULL != pHub->GetpSelectedInfos() )
			{
				if( NULL != *pHub->GetpSelectedInfos()->GetModifiedObjIDPtr().ID )
				{
					if( TASApp.AfxLocalizeMessageBox( AFXMSG_DISCARDHUBMODIFICATIONS, MB_YESNO | MB_ICONEXCLAMATION, 0 ) != IDYES )
					{
						return NULL;
					}
				}
			}
		
			Delete( idptr );
		}
	
		// Copy the selected Hub to the temporary table
		IDPTR idptr;
		m_pTADS->CreateObject( idptr, CLASS( CDS_HmHub ) );
		pTabHub->Insert( idptr );

		CDS_HmHub *pHmHub = (CDS_HmHub *)( idptr.MP );
		pHmHub->Init( pSelHub->GetBalTypeID(), pTabHub );
		pSelHub->Copy( pHmHub, false, false );
	
		// Copy children.
		for( IDPTR ChildIDPtr = pSelHub->GetFirst(); _T('\0') != *ChildIDPtr.ID; ChildIDPtr = pSelHub->GetNext() )
		{
			CDS_HmHubStation *pSelStation = (CDS_HmHubStation *)( ChildIDPtr.MP );
		
			m_pTADS->CreateObject( idptr, CLASS( CDS_HmHubStation ) );
			pHmHub->Insert( idptr );

			CDS_HmHubStation *pTmpStation = (CDS_HmHubStation*)( idptr.MP );
			pTmpStation->Init( pSelStation->GetBalTypeID(), pHmHub, pSelStation->GetCvTaKv() );
			pSelStation->Copy( (CDS_HmHubStation *)( idptr.MP ), false, false );
		}
	
		// Set a link to the modified Hub
		pHmHub->GetpSelectedInfos()->SetModifiedObjIDPtr( m_HubIDPtr );
		return pHmHub;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CHubHandler::ModifySelectedProduct'."), __LINE__, __FILE__ );
		throw;
	}
}
