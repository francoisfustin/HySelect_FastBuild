#include "stdafx.h"
#include "TASelect.h"
#include "Select.h"

#include "DeletedProductHelper.h"


CDeletedProductHelper::CDeletedProductHelper()
{
	m_pTADS = TASApp.GetpTADS();
	m_pTADB = TASApp.GetpTADB();
	m_mapUndef.clear();
	m_mapDeleted.clear();
	m_mapNotAvailable.clear();
	m_mapNotCompatible.clear();
	m_mapHMDeleted.clear();
}

CDeletedProductHelper::~CDeletedProductHelper()
{
	m_mapUndef.clear();
	m_mapDeleted.clear();
	m_mapNotAvailable.clear();
	m_mapNotCompatible.clear();
	m_mapHMDeleted.clear();
}

bool CDeletedProductHelper::VerifyAccessoryList( CAccessoryList *pclAccessoryList, FromWhere eWhere, int iQuantity )
{
	bool bDisplay = false;

	if( pclAccessoryList->GetCount() > 0 )
	{
		CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();

		while( NULL != rAccessoryItem.IDPtr.MP )
		{
			// If accessory is not distributed on each parent product we need only 1. Otherwise there is one accessory by each
			// parent product and thus we need the same number.
			int iTotalQuantity = ( false == rAccessoryItem.fDistributed ) ? 1 : iQuantity;

			// Some accessories are sold by pair. In this case we have to multiply by 2.
			iTotalQuantity = ( false == rAccessoryItem.fByPair ) ? iTotalQuantity : ( 2 * iTotalQuantity );

			if( true == VerifyAccessory( rAccessoryItem.IDPtr, eWhere, iTotalQuantity ) )
			{
				bDisplay = true;
			}

			rAccessoryItem = pclAccessoryList->GetNext();
		}
	}

	return bDisplay;
}

bool CDeletedProductHelper::VerifyAccessory( IDPTR AccIDPtr, FromWhere eWhere, int iQuantity )
{
	bool bDisplay = false;

	CDB_Thing *pThing = ( (CDB_Thing *)m_pTADB->Get( AccIDPtr.ID ).MP );
	CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>(AccIDPtr.MP );
	ASSERT( NULL != pclAccessory );

	if( ( NULL != pclAccessory ) && ( pclAccessory->IsAnAccessory() ) )
	{
		if( true == _PrepareListCtrl( pThing, pclAccessory->GetName(), pclAccessory->GetIDPtr().ID, eWhere, iQuantity ) )
		{
			bDisplay = true;
		}
	}

	return bDisplay;
}

void CDeletedProductHelper::VerifyIndividualSelection( bool &bDisplay )
{
	CDB_Thing *pThing = NULL;

	// Scan all regulating valves.
	CTable *pTab = (CTable *)( m_pTADS->Get(L"REGVALV_TAB").MP );
	IDPTR IDPtr = _NULL_IDPTR;

	if( NULL != pTab )
	{
		for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_TAProduct *pTAP = NULL;
			CDS_SSelBv *pSelBv = dynamic_cast<CDS_SSelBv *>( IDPtr.MP );

			if( NULL != pSelBv )
			{
				FromWhere eWhere = ( true == pSelBv->IsFromDirSel() ) ? FromDirectSelection : FromIndividualSelection;
				pTAP = pSelBv->GetProductAs<CDB_TAProduct>();

				if( NULL != pTAP )
				{
					// Take product from TADB...
					if( true == _PrepareListCtrl( pTAP, pTAP->GetName(), pTAP->GetIDPtr().ID, eWhere, pSelBv->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}
				else
				{
					ASSERT( 0 );
				}

				// Check associated accessories if exist.
				CAccessoryList *pclBvAccessoryList = pSelBv->GetAccessoryList();

				if( true == VerifyAccessoryList( pclBvAccessoryList, eWhere, pSelBv->GetpSelectedInfos()->GetQuantity() ) )
				{
					bDisplay = true;
				}
			}
			else
			{
				ASSERT( 0 );
			}
		}
	}

	// Scan all Dp controllers.
	pTab = (CTable *)( m_pTADS->Get(L"DPCONTR_TAB").MP );

	if( NULL != pTab )
	{
		for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDS_SSelDpC *pSSelDpController = dynamic_cast<CDS_SSelDpC *>( IDPtr.MP );
			CDS_SSelDpReliefValve *pSSelDpReliefValve = dynamic_cast<CDS_SSelDpReliefValve *>( IDPtr.MP );

			if( NULL != pSSelDpController )
			{
				FromWhere eWhere = ( true == pSSelDpController->IsFromDirSel() ) ? FromDirectSelection : FromIndividualSelection;

				// Take product from TADB...
				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pSSelDpController->GetDpCIDPtr().MP );

				if( NULL != pTAP )
				{
					if( true == _PrepareListCtrl( pTAP, pTAP->GetName(), pSSelDpController->GetDpCIDPtr().ID, eWhere, pSSelDpController->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}

					// Check associated accessories if exist.
					CAccessoryList *pclDpCAccessoryList = pSSelDpController->GetDpCAccessoryList();

					if( true == VerifyAccessoryList( pclDpCAccessoryList, eWhere, pSSelDpController->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}

				// Check associated valve if exist.
				pTAP = pSSelDpController->GetProductAs<CDB_TAProduct>();

				if( NULL != pTAP )
				{
					if( true == _PrepareListCtrl( pTAP, pTAP->GetName(), pSSelDpController->GetProductIDPtr().ID, eWhere, pSSelDpController->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}

					// Check associated accessories if exist.
					CAccessoryList *pclBvAccessoryList = pSSelDpController->GetAccessoryList();

					if( true == VerifyAccessoryList( pclBvAccessoryList, eWhere, pSSelDpController->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}
			}
			else if( NULL != pSSelDpReliefValve )
			{
				FromWhere eWhere = ( true == pSSelDpReliefValve->IsFromDirSel() ) ? FromDirectSelection : FromIndividualSelection;
				CDB_TAProduct *pTAP = pSSelDpReliefValve->GetProductAs<CDB_TAProduct>();

				if( NULL != pTAP )
				{
					// Take product from TADB...
					if( true == _PrepareListCtrl( pTAP, pTAP->GetName(), pTAP->GetIDPtr().ID, eWhere, pSSelDpReliefValve->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}
				else
				{
					ASSERT( 0 );
				}

				// Check associated accessories if exist.
				CAccessoryList *pclDpReliefValveAccessoryList = pSSelDpReliefValve->GetAccessoryList();

				if( true == VerifyAccessoryList( pclDpReliefValveAccessoryList, eWhere, pSSelDpReliefValve->GetpSelectedInfos()->GetQuantity() ) )
				{
					bDisplay = true;
				}
			}
		}
	}

	// Scan all thermostatic valves.
	pTab = (CTable *)( m_pTADS->Get(L"RADSET_TAB").MP );

	if( NULL != pTab )
	{
		for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDS_SSelRadSet *pSSelRadSet = dynamic_cast<CDS_SSelRadSet *>( IDPtr.MP );
			ASSERT( NULL != pSSelRadSet );

			if( NULL != pSSelRadSet )
			{
				FromWhere eWhere = ( true == pSSelRadSet->IsFromDirSel() ) ? FromDirectSelection : FromIndividualSelection;

				// Take product from TADB...
				if( NULL != pSSelRadSet->GetSupplyValveIDPtr().MP )
				{
					CDB_TAProduct *pTAProduct = dynamic_cast<CDB_TAProduct *>( (CData *)pSSelRadSet->GetSupplyValveIDPtr().MP );
					ASSERT( NULL != pTAProduct);

					if( NULL != pTAProduct 
							&& true == _PrepareListCtrl( pTAProduct, pTAProduct->GetName(), pTAProduct->GetIDPtr().ID, eWhere, pSSelRadSet->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}

					// Check associated actuator if exist.
					if( _T('\0') != *pSSelRadSet->GetSupplyValveActuatorIDPtr().ID )
					{
						pThing = ( (CDB_Thing *)( m_pTADB->Get( pSSelRadSet->GetSupplyValveActuatorIDPtr().ID ).MP ) );
						CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( (CData *)( pSSelRadSet->GetSupplyValveActuatorIDPtr().MP ) );
						ASSERT( NULL != pclActuator );

						if( NULL != pclActuator )
						{
							if( true == _PrepareListCtrl( pThing, pclActuator->GetName(), pclActuator->GetIDPtr().ID, eWhere, pSSelRadSet->GetpSelectedInfos()->GetQuantity() ) )
							{
								bDisplay = true;
							}
						}
					}

					// Check associated accessories if exist.
					CAccessoryList *pclSVAccessoryList = pSSelRadSet->GetSupplyValveAccessoryList();

					if( true == VerifyAccessoryList( pclSVAccessoryList, eWhere, pSSelRadSet->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}

					// Check associated actuator's accessories if exist.
					CAccessoryList *pclActuatorAccessoryList = pSSelRadSet->GetSVActuatorAccessoryList();

					if( true == VerifyAccessoryList( pclActuatorAccessoryList, eWhere, pSSelRadSet->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}

				if( NULL != pSSelRadSet->GetReturnValveIDPtr().MP )
				{
					CDB_TAProduct *pTAProduct = dynamic_cast<CDB_TAProduct *>( (CData *)pSSelRadSet->GetReturnValveIDPtr().MP );
					ASSERT( NULL != pTAProduct );

					if( NULL != pTAProduct 
							&& true == _PrepareListCtrl( pTAProduct, pTAProduct->GetName(), pTAProduct->GetIDPtr().ID, eWhere, pSSelRadSet->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}

					// Check associated accessories if exist.
					CAccessoryList *pclRVAccessoryList = pSSelRadSet->GetReturnValveAccessoryList();

					if( true == VerifyAccessoryList( pclRVAccessoryList, eWhere, pSSelRadSet->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}
			}
		}
	}

	// Scan all control valves.
	pTab = (CTable *)( m_pTADS->Get(L"CTRLVALVE_TAB").MP );

	if( NULL != pTab )
	{
		for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDS_SSelCtrl *pSSelCtrl = dynamic_cast<CDS_SSelCtrl *>( IDPtr.MP );

			if( NULL != pSSelCtrl )
			{
				FromWhere eWhere = ( true == pSSelCtrl->IsFromDirSel() ) ? FromDirectSelection : FromIndividualSelection;
				CDB_ControlValve *pclControlValve = pSSelCtrl->GetProductAs<CDB_ControlValve>();

				// Take product from TADB...
				if( NULL != pSSelCtrl->GetProductIDPtr().MP )
				{
					pThing = ( (CDB_Thing *)pSSelCtrl->GetProductIDPtr().MP );

					if( true == _PrepareListCtrl( pThing, pclControlValve->GetName(), pclControlValve->GetIDPtr().ID, eWhere, pSSelCtrl->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}

				// Check associated actuator if exist.
				if( _T('\0') != *pSSelCtrl->GetActrIDPtr().ID )
				{
					pThing = ( (CDB_Thing *)( m_pTADB->Get( pSSelCtrl->GetActrIDPtr().ID ).MP ) );
					CDB_Actuator *pclActuator = pSSelCtrl->GetpActuator();
					ASSERT( NULL != pclActuator );

					if( NULL != pclActuator )
					{
						if( true == _PrepareListCtrl( pThing, pclActuator->GetName(), pclActuator->GetIDPtr().ID, eWhere, pSSelCtrl->GetpSelectedInfos()->GetQuantity() ) )
						{
							bDisplay = true;
						}
					}
				}

				// Check associated accessories if exist.
				CAccessoryList *pclCvAccessoryList = pSSelCtrl->GetCvAccessoryList();

				if( true == VerifyAccessoryList( pclCvAccessoryList, eWhere, pSSelCtrl->GetpSelectedInfos()->GetQuantity() ) )
				{
					bDisplay = true;
				}

				// Check associated actuator's accessories if exist.
				CAccessoryList *pclActuatorAccessoryList = pSSelCtrl->GetActuatorAccessoryList();

				if( true == VerifyAccessoryList( pclActuatorAccessoryList, eWhere, pSSelCtrl->GetpSelectedInfos()->GetQuantity() ) )
				{
					bDisplay = true;
				}
			}
			else
			{
				// It can be an actuator alone or an accessory alone.
				CDS_Actuator *pSelAct = dynamic_cast<CDS_Actuator *>( IDPtr.MP );
				CDS_Accessory *pSelAcc = dynamic_cast<CDS_Accessory *>( IDPtr.MP );

				if( NULL != pSelAct )
				{
					pThing = ( (CDB_Thing *)( m_pTADB->Get( pSelAct->GetActuatorIDPtr().ID ).MP ) );
					CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( pSelAct->GetActuatorIDPtr().MP );
					ASSERT(pclActuator);

					if( NULL != pclActuator )
					{
						if( true == _PrepareListCtrl( pThing, pclActuator->GetName(), pclActuator->GetIDPtr().ID, FromDirectSelection, pSelAct->GetpSelectedInfos()->GetQuantity() ) )
						{
							bDisplay = true;
						}
					}
				}
				else if( NULL != pSelAcc )
				{
					pThing = ( (CDB_Thing *)( m_pTADB->Get( pSelAcc->GetAccessoryIDPtr().ID ).MP ) );
					CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( pSelAcc->GetAccessoryIDPtr().MP );
					ASSERT( NULL != pclAccessory );

					if( ( NULL != pclAccessory ) && ( pclAccessory->IsAnAccessory() ) )
					{
						if( true == _PrepareListCtrl( pThing, pclAccessory->GetName(), pclAccessory->GetIDPtr().ID, FromDirectSelection, pSelAcc->GetpSelectedInfos()->GetQuantity() ) )
						{
							bDisplay = true;
						}
					}
				}
				else
				{
					// Found a product in the CtrlValve Table that is not a ctrl valve, an actuator or an accessory.
					ASSERT( 0 );
				}
			}
		}
	}

	// Scan all combined Dp controller and balancing and control valve.
	pTab = (CTable *)( m_pTADS->Get(L"DPCBCVALVE_TAB").MP );
	IDPtr = _NULL_IDPTR;

	if( NULL != pTab )
	{
		for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_TAProduct *pTAP = NULL;
			CDS_SSelDpCBCV *pSelDpCBCV = dynamic_cast<CDS_SSelDpCBCV *>( IDPtr.MP );

			if( NULL != pSelDpCBCV )
			{
				FromWhere eWhere = ( true == pSelDpCBCV->IsFromDirSel() ) ? FromDirectSelection : FromIndividualSelection;
				pTAP = pSelDpCBCV->GetProductAs<CDB_TAProduct>();

				if( NULL != pTAP )
				{
					// Take product from TADB...
					if( true == _PrepareListCtrl( pTAP, pTAP->GetName(), pTAP->GetIDPtr().ID, eWhere, pSelDpCBCV->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}
				else
				{
					ASSERT( 0 );
				}

				// Check associated accessories if exist.
				CAccessoryList *pclDpCBCVAccessoryList = pSelDpCBCV->GetAccessoryList();

				if( true == VerifyAccessoryList( pclDpCBCVAccessoryList, eWhere, pSelDpCBCV->GetpSelectedInfos()->GetQuantity() ) )
				{
					bDisplay = true;
				}
			}
			else
			{
				ASSERT( 0 );
			}
		}
	}

	// Scan all air vents & separators.
	pTab = (CTable *)( m_pTADS->Get(L"AIRVENTSEP_TAB").MP );
	IDPtr = _NULL_IDPTR;

	if( NULL != pTab )
	{
		for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_TAProduct *pTAP = NULL;
			CDS_SSelAirVentSeparator *pSelAirVentSep = dynamic_cast<CDS_SSelAirVentSeparator *>( IDPtr.MP );

			if( NULL != pSelAirVentSep)
			{
				FromWhere eWhere = ( true == pSelAirVentSep->IsFromDirSel() ) ? FromDirectSelection : FromIndividualSelection;
				pTAP = pSelAirVentSep->GetProductAs<CDB_TAProduct>();

				if( NULL != pTAP )
				{
					// Take product from TADB...
					if( true == _PrepareListCtrl( pTAP, pTAP->GetName(), pTAP->GetIDPtr().ID, eWhere, pSelAirVentSep->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}
				else
				{
					ASSERT( 0 );
				}

				// Check associated accessories if exist.
				CAccessoryList *pclSeparatorAccessoryList = pSelAirVentSep->GetAccessoryList();

				if( true == VerifyAccessoryList( pclSeparatorAccessoryList, eWhere, pSelAirVentSep->GetpSelectedInfos()->GetQuantity() ) )
				{
					bDisplay = true;
				}
			}
			else
			{
				ASSERT( 0 );
			}
		}
	}

	// Scan all pressure maintenance.
	pTab = (CTable *)( m_pTADS->Get(L"PRESSMAINT_TAB").MP );
	IDPtr = _NULL_IDPTR;

	if( NULL != pTab )
	{
		for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_Product *pclProduct = NULL;
			CDS_SSelPMaint *pSelPressMaint = dynamic_cast<CDS_SSelPMaint *>( IDPtr.MP );

			if( NULL != pSelPressMaint)
			{
				FromWhere eWhere = ( true == pSelPressMaint->IsFromDirSel() ) ? FromDirectSelection : FromIndividualSelection;

				// VESSEL
				if(pSelPressMaint->GetVesselNumber() > 0 && NULL != pSelPressMaint->GetVesselIDPtr().MP )
				{
					pclProduct = dynamic_cast<CDB_Product *>( (CData *)pSelPressMaint->GetVesselIDPtr().MP );

					if( NULL != pclProduct)
					{
						int iTotalQuantity = pSelPressMaint->GetVesselNumber() * pSelPressMaint->GetpSelectedInfos()->GetQuantity();

						// Take product from TADB...
						if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, eWhere, iTotalQuantity))
						{
							bDisplay = true;
						}
					}
					else
					{
						ASSERT( 0 );
					}

					// Check associated accessories if exist.
					CAccessoryList *pclPMAccessoryList = pSelPressMaint->GetVesselAccessoryList();

					if( true == VerifyAccessoryList( pclPMAccessoryList, eWhere, pSelPressMaint->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}

				// TECBOX (Transfero or Compresso)
				if(pSelPressMaint->GetTecBoxCompTransfNumber() > 0)
				{
					pclProduct = dynamic_cast<CDB_Product *>( (CData *)pSelPressMaint->GetTecBoxCompTransfIDPtr().MP );

					if( NULL != pclProduct)
					{
						int iTotalQuantity = pSelPressMaint->GetTecBoxCompTransfNumber() * pSelPressMaint->GetpSelectedInfos()->GetQuantity();

						// Take product from TADB...
						if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, eWhere, iTotalQuantity))
						{
							bDisplay = true;
						}
					}
					else
					{
						ASSERT( 0 );
					}

					// Check associated accessories if exist.
					CAccessoryList *pclPMAccessoryList = pSelPressMaint->GetTecBoxCompTransfAccessoryList();

					if( true == VerifyAccessoryList( pclPMAccessoryList, eWhere, pSelPressMaint->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}

				// Secondary vessel.
				if(pSelPressMaint->GetSecondaryVesselNumber() > 0)
				{
					pclProduct = dynamic_cast<CDB_Product *>( (CData *)pSelPressMaint->GetSecondaryVesselIDPtr().MP );

					if( NULL != pclProduct)
					{
						int iTotalQuantity = pSelPressMaint->GetSecondaryVesselNumber() * pSelPressMaint->GetpSelectedInfos()->GetQuantity();

						// Take product from TADB...
						if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, eWhere, iTotalQuantity))
						{
							bDisplay = true;
						}
					}
					else
					{
						ASSERT( 0 );
					}
				}

				// Transfero buffer vessel.
				pclProduct = dynamic_cast<CDB_Product *>( (CData *)pSelPressMaint->GetTecBoxIntegratedVesselIDPtr().MP );

				if( NULL != pclProduct)
				{
					// Take product from TADB...
					if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, eWhere, pSelPressMaint->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}

					// HYS-872: Check associated accessories if exists.
					CAccessoryList *pclPMAccessoryList = pSelPressMaint->GetTecBoxIntegratedVesselAccessoryList();

					if (true == VerifyAccessoryList(pclPMAccessoryList, eWhere, pSelPressMaint->GetpSelectedInfos()->GetQuantity()))
					{
						bDisplay = true;
					}
				}

				// Intermediate vessel.
				if(pSelPressMaint->GetIntermediateVesselNumber() > 0)
				{
					pclProduct = dynamic_cast<CDB_Product *>( (CData *)pSelPressMaint->GetIntermediateVesselIDPtr().MP );

					if( NULL != pclProduct)
					{
						int iTotalQuantity = pSelPressMaint->GetIntermediateVesselNumber() * pSelPressMaint->GetpSelectedInfos()->GetQuantity();

						// Take product from TADB...
						if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, eWhere, iTotalQuantity))
						{
							bDisplay = true;
						}
					}
					else
					{
						ASSERT( 0 );
					}

					// Check associated accessories if exist.
					CAccessoryList *pclPMAccessoryList = pSelPressMaint->GetIntermediateVesselAccessoryList();

					if( true == VerifyAccessoryList( pclPMAccessoryList, eWhere, pSelPressMaint->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}

				// Pleno.
				if(pSelPressMaint->GetTecBoxPlenoNumber() > 0)
				{
					pclProduct = dynamic_cast<CDB_Product *>( (CData *)pSelPressMaint->GetTecBoxPlenoIDPtr().MP );

					if( NULL != pclProduct)
					{
						int iTotalQuantity = pSelPressMaint->GetTecBoxPlenoNumber() * pSelPressMaint->GetpSelectedInfos()->GetQuantity();

						// Take product from TADB...
						if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, eWhere, iTotalQuantity))
						{
							bDisplay = true;
						}
					}
					// HYS-1121 : Protection module with Pleno
					if( NULL != dynamic_cast<CDB_Product *>( (CData *)pSelPressMaint->GetTecBoxPlenoProtectionIDPtr().MP ) )
					{
						pclProduct = dynamic_cast<CDB_Product *>( (CData *)pSelPressMaint->GetTecBoxPlenoProtectionIDPtr().MP );

						if( NULL != pclProduct )
						{
							int iTotalQuantity = pSelPressMaint->GetTecBoxPlenoNumber() * pSelPressMaint->GetpSelectedInfos()->GetQuantity();

							// Take product from TADB...
							if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, eWhere, iTotalQuantity ) )
							{
								bDisplay = true;
							}
						}
					}
					// Additional Pleno is not mandatory.
					if(_NULL_IDPTR != pSelPressMaint->GetAdditionalTecBoxPlenoIDPtr())
					{
						CDB_Set* pclProductSet = dynamic_cast<CDB_Set *>( (CData *)pSelPressMaint->GetAdditionalTecBoxPlenoIDPtr().MP );

						if( NULL != pclProductSet )
						{
							// Take product from TADB...
							if( true == _PrepareListCtrl( pclProductSet, pclProductSet->GetName(), pclProductSet->GetIDPtr().ID, eWhere, pSelPressMaint->GetpSelectedInfos()->GetQuantity() ) )
							{
								bDisplay = true;
							}
						}
						else
						{
							ASSERT( 0 );
						}
					}

					// Check associated accessories if exist.
					CAccessoryList *pclPMAccessoryList = pSelPressMaint->GetTecBoxPlenoAccessoryList();

					if( true == VerifyAccessoryList( pclPMAccessoryList, eWhere, pSelPressMaint->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}

				// Vento.
				if(pSelPressMaint->GetTecBoxVentoNumber() > 0)
				{
					pclProduct = dynamic_cast<CDB_Product *>( (CData *)pSelPressMaint->GetTecBoxVentoIDPtr().MP );

					if( NULL != pclProduct)
					{
						int iTotalQuantity = pSelPressMaint->GetTecBoxVentoNumber() * pSelPressMaint->GetpSelectedInfos()->GetQuantity();

						// Take product from TADB...
						if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, eWhere, iTotalQuantity))
						{
							bDisplay = true;
						}
					}
					else
					{
						ASSERT( 0 );
					}

					// Check associated accessories if exist.
					CAccessoryList *pclPMAccessoryList = pSelPressMaint->GetTecBoxVentoAccessoryList();

					if( true == VerifyAccessoryList( pclPMAccessoryList, eWhere, pSelPressMaint->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}

				// Pleno Refill.
				if(pSelPressMaint->GetPlenoRefillNumber() > 0)
				{
					pclProduct = dynamic_cast<CDB_Product *>( (CData *)pSelPressMaint->GetPlenoRefillIDPtr().MP );

					if( NULL != pclProduct)
					{
						int iTotalQuantity = pSelPressMaint->GetPlenoRefillNumber() * pSelPressMaint->GetpSelectedInfos()->GetQuantity();

						// Take product from TADB...
						if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, eWhere, iTotalQuantity))
						{
							bDisplay = true;
						}
					}
					else
					{
						ASSERT( 0 );
					}

					// Check associated accessories if exist.
					CAccessoryList *pclPMAccessoryList = pSelPressMaint->GetPlenoRefillAccessoryList();

					if( true == VerifyAccessoryList( pclPMAccessoryList, eWhere, pSelPressMaint->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}
			}
			else
			{
				ASSERT( 0 );
			}
		}
	}

	// Scan all safety valves, blow tank and accessories.
	pTab = (CTable *)( m_pTADS->Get(L"SAFETYVALVE_TAB").MP );
	IDPtr = _NULL_IDPTR;

	if( NULL != pTab )
	{
		for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_TAProduct *pTAP = NULL;
			CDS_SSelSafetyValve *pclSelSafetyValve = dynamic_cast<CDS_SSelSafetyValve *>( IDPtr.MP );
			// HYS-1741: Manage deleted Pressure reducer valve
			CDS_SelPWQAccServices *pclSelPWQAccessoriesAndServices = dynamic_cast<CDS_SelPWQAccServices*>( IDPtr.MP );

			if( NULL != pclSelSafetyValve )
			{
				FromWhere eWhere = ( true == pclSelSafetyValve->IsFromDirSel() ) ? FromDirectSelection : FromIndividualSelection;
				CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase *>( pclSelSafetyValve->GetProductIDPtr().MP );
				CDB_BlowTank *pclBlowTank = dynamic_cast<CDB_BlowTank *>( pclSelSafetyValve->GetBlowTankIDPtr().MP );

				if( NULL != pclSafetyValve )
				{
					// Take product from TADB...
					if( true == _PrepareListCtrl( pclSafetyValve, pclSafetyValve->GetName(), pclSafetyValve->GetIDPtr().ID, eWhere, pclSelSafetyValve->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}
				
				if( NULL != pclBlowTank )
				{
					// Take product from TADB...
					if( true == _PrepareListCtrl( pclBlowTank, pclBlowTank->GetName(), pclBlowTank->GetIDPtr().ID, eWhere, pclSelSafetyValve->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}
				
				if( NULL == pclSelSafetyValve && NULL == pclBlowTank )
				{
					ASSERT( 0 );
				}

				// Check associated accessories if exist.
				CAccessoryList *pclSafetyValveAccessoryList = pclSelSafetyValve->GetAccessoryList();

				if( true == VerifyAccessoryList( pclSafetyValveAccessoryList, eWhere, pclSelSafetyValve->GetpSelectedInfos()->GetQuantity() ) )
				{
					bDisplay = true;
				}

				CAccessoryList *pclBlowTankAccessoryList = pclSelSafetyValve->GetBlowTankAccessoryList();

				if( true == VerifyAccessoryList( pclBlowTankAccessoryList, eWhere, pclSelSafetyValve->GetpSelectedInfos()->GetQuantity() ) )
				{
					bDisplay = true;
				}
			}

			if( NULL != pclSelPWQAccessoriesAndServices )
			{
				FromWhere eWhere = (true == pclSelPWQAccessoriesAndServices->IsFromDirSel()) ? FromDirectSelection : FromIndividualSelection;
				CDB_PWQPressureReducer* pclPWQPressureReduc = dynamic_cast<CDB_PWQPressureReducer*>(pclSelPWQAccessoriesAndServices->GetProductIDPtr().MP);

				if( NULL != pclPWQPressureReduc )
				{
					// Take product from TADB...
					if( true == _PrepareListCtrl( pclPWQPressureReduc, pclPWQPressureReduc->GetName(), pclPWQPressureReduc->GetIDPtr().ID, eWhere,
												  pclSelPWQAccessoriesAndServices->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}

			}

			ASSERT( NULL != pclSelSafetyValve || NULL != pclSelPWQAccessoriesAndServices );
		}
	}

	// Scan all 6-way valves.
	pTab = (CTable *)( m_pTADS->Get(L"6WAYCTRLVALV_TAB").MP );

	if( NULL != pTab )
	{
		for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDS_SSel6WayValve *pclSSel6WayValve = dynamic_cast<CDS_SSel6WayValve *>( IDPtr.MP );

			if( NULL == pclSSel6WayValve )
			{
				ASSERT( 0 );
				continue;
			}

			FromWhere eWhere = ( true == pclSSel6WayValve->IsFromDirSel() ) ? FromDirectSelection : FromIndividualSelection;
			int iGlobalQuantity = pclSSel6WayValve->GetpSelectedInfos()->GetQuantity();

			_VerifyIndividualSelection6WayValveHelper( pclSSel6WayValve->GetProductIDPtr(), pclSSel6WayValve->GetActrIDPtr(), pclSSel6WayValve->GetAccessoryList(), 
					pclSSel6WayValve->GetActuatorAccessoryList(), eWhere, iGlobalQuantity, bDisplay );

			if( e6Way_Alone == pclSSel6WayValve->GetSelectionMode() )
			{
				continue;
			}

			if( e6Way_EQMControl == pclSSel6WayValve->GetSelectionMode() )
			{
				CDS_SSelPICv *pclSSelPIBCValve = pclSSel6WayValve->GetCDSSSelPICv( BothSide );

				if( NULL == pclSSelPIBCValve || NULL == dynamic_cast<CDB_PIControlValve *>( pclSSelPIBCValve->GetProductIDPtr().MP ) )
				{
					ASSERT( 0 );
				}

				_VerifyIndividualSelection6WayValveHelper( pclSSelPIBCValve->GetProductIDPtr(), pclSSelPIBCValve->GetActrIDPtr(), pclSSelPIBCValve->GetAccessoryList(), 
						pclSSelPIBCValve->GetActuatorAccessoryList(), eWhere, iGlobalQuantity, bDisplay );
			}
			else if( e6Way_OnOffControlWithPIBCV == pclSSel6WayValve->GetSelectionMode() )
			{
				CDS_SSelPICv *pclSSelPIBCValve1 = pclSSel6WayValve->GetCDSSSelPICv( HeatingSide );
				CDS_SSelPICv *pclSSelPIBCValve2 = pclSSel6WayValve->GetCDSSSelPICv( CoolingSide );

				if( NULL == pclSSelPIBCValve1 || NULL == dynamic_cast<CDB_PIControlValve *>( pclSSelPIBCValve1->GetProductIDPtr().MP )
						|| NULL == pclSSelPIBCValve2 || NULL == dynamic_cast<CDB_PIControlValve *>( pclSSelPIBCValve2->GetProductIDPtr().MP ) )
				{
					ASSERT( 0 );
				}

				_VerifyIndividualSelection6WayValveHelper( pclSSelPIBCValve1->GetProductIDPtr(), pclSSelPIBCValve1->GetActrIDPtr(), pclSSelPIBCValve1->GetAccessoryList(), 
						pclSSelPIBCValve1->GetActuatorAccessoryList(), eWhere, iGlobalQuantity, bDisplay );

				_VerifyIndividualSelection6WayValveHelper( pclSSelPIBCValve2->GetProductIDPtr(), pclSSelPIBCValve2->GetActrIDPtr(), pclSSelPIBCValve2->GetAccessoryList(), 
						pclSSelPIBCValve2->GetActuatorAccessoryList(), eWhere, iGlobalQuantity, bDisplay );
			}
			else if( e6Way_OnOffControlWithSTAD == pclSSel6WayValve->GetSelectionMode() )
			{
				CDS_SSelBv *pclSSelBalancingValve1 = pclSSel6WayValve->GetCDSSSelBv( HeatingSide );
				CDS_SSelBv *pclSSelBalancingValve2 = pclSSel6WayValve->GetCDSSSelBv( CoolingSide );

				if( NULL == pclSSelBalancingValve1 || NULL == dynamic_cast<CDB_RegulatingValve *>( pclSSelBalancingValve1->GetProductIDPtr().MP )
						|| NULL == pclSSelBalancingValve2 || NULL == dynamic_cast<CDB_RegulatingValve *>( pclSSelBalancingValve2->GetProductIDPtr().MP ) )
				{
					ASSERT( 0 );
				}

				_VerifyIndividualSelectionBvHelper( pclSSelBalancingValve1->GetProductIDPtr(), pclSSelBalancingValve1->GetAccessoryList(), eWhere, iGlobalQuantity, bDisplay );
				_VerifyIndividualSelectionBvHelper( pclSSelBalancingValve2->GetProductIDPtr(), pclSSelBalancingValve2->GetAccessoryList(), eWhere, iGlobalQuantity, bDisplay );
			}
		}
	}

	
	// Scan all floor heating control products.
	pTab = (CTable *)( m_pTADS->Get(L"FLOORHCTRL_TAB").MP );
	IDPtr = _NULL_IDPTR;

	if( NULL != pTab )
	{
		for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_Product *pclProduct = NULL;
			CDS_SSel *pSelFloorHeatingControl = dynamic_cast<CDS_SSel *>( IDPtr.MP );

			if( NULL != pSelFloorHeatingControl )
			{
				FromWhere eWhere = ( true == pSelFloorHeatingControl->IsFromDirSel() ) ? FromDirectSelection : FromIndividualSelection;
				pclProduct = pSelFloorHeatingControl->GetProductAs<CDB_Product>();

				if( NULL != pclProduct )
				{
					// Take product from TADB...
					if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, eWhere, pSelFloorHeatingControl->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}
				else
				{
					ASSERT( 0 );
				}

				// Check associated accessories if exist.
				CAccessoryList *pclFloorHeatingControlAccessoryList = pSelFloorHeatingControl->GetAccessoryList();

				if( true == VerifyAccessoryList( pclFloorHeatingControlAccessoryList, eWhere, pSelFloorHeatingControl->GetpSelectedInfos()->GetQuantity() ) )
				{
					bDisplay = true;
				}
			}
			else
			{
				ASSERT( 0 );
			}
		}
	}

	// Scan all tap water control products.
	pTab = (CTable *)( m_pTADS->Get(L"TAPWATERCTRL_TAB").MP );
	IDPtr = _NULL_IDPTR;

	if( NULL != pTab )
	{
		for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_TAProduct *pTAP = NULL;
			CDS_SSelTapWaterControl *pSelTapWaterControl = dynamic_cast<CDS_SSelTapWaterControl *>( IDPtr.MP );

			if( NULL != pSelTapWaterControl )
			{
				FromWhere eWhere = ( true == pSelTapWaterControl->IsFromDirSel() ) ? FromDirectSelection : FromIndividualSelection;
				pTAP = pSelTapWaterControl->GetProductAs<CDB_TAProduct>();

				if( NULL != pTAP )
				{
					// Take product from TADB...
					if( true == _PrepareListCtrl( pTAP, pTAP->GetName(), pTAP->GetIDPtr().ID, eWhere, pSelTapWaterControl->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}
				else
				{
					ASSERT( 0 );
				}

				// Check associated accessories if exist.
				CAccessoryList *pclTapWaterControlAccessoryList = pSelTapWaterControl->GetAccessoryList();

				if( true == VerifyAccessoryList( pclTapWaterControlAccessoryList, eWhere, pSelTapWaterControl->GetpSelectedInfos()->GetQuantity() ) )
				{
					bDisplay = true;
				}
			}
			else
			{
				ASSERT( 0 );
			}
		}
	}

	// Scan all smart control valves.
	pTab = (CTable *)( m_pTADS->Get(L"SMARTCONTROLVALVE_TAB").MP );
	IDPtr = _NULL_IDPTR;

	if( NULL != pTab )
	{
		for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_TAProduct *pTAP = NULL;
			CDS_SSelSmartControlValve *pSelSmartControlValve = dynamic_cast<CDS_SSelSmartControlValve *>( IDPtr.MP );

			if( NULL != pSelSmartControlValve )
			{
				FromWhere eWhere = ( true == pSelSmartControlValve->IsFromDirSel() ) ? FromDirectSelection : FromIndividualSelection;
				pTAP = pSelSmartControlValve->GetProductAs<CDB_TAProduct>();

				if( NULL != pTAP )
				{
					// Take product from TADB...
					if( true == _PrepareListCtrl( pTAP, pTAP->GetName(), pTAP->GetIDPtr().ID, eWhere, pSelSmartControlValve->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}
				else
				{
					ASSERT( 0 );
				}
			}
			else
			{
				ASSERT( 0 );
			}
		}
	}

	// Scan all smart differential pressure controllers.
	pTab = (CTable *)( m_pTADS->Get(L"SMARTDPC_TAB").MP );
	IDPtr = _NULL_IDPTR;

	if( NULL != pTab )
	{
		for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_TAProduct *pTAP = NULL;
			CDS_SSelSmartDpC *pSelSmartDpC = dynamic_cast<CDS_SSelSmartDpC *>( IDPtr.MP );

			if( NULL != pSelSmartDpC )
			{
				FromWhere eWhere = ( true == pSelSmartDpC->IsFromDirSel() ) ? FromDirectSelection : FromIndividualSelection;
				pTAP = pSelSmartDpC->GetProductAs<CDB_TAProduct>();

				if( NULL != pTAP )
				{
					// Take product from TADB...
					if( true == _PrepareListCtrl( pTAP, pTAP->GetName(), pTAP->GetIDPtr().ID, eWhere, pSelSmartDpC->GetpSelectedInfos()->GetQuantity() ) )
					{
						bDisplay = true;
					}
				}
				else
				{
					ASSERT( 0 );
				}
			}
			else
			{
				ASSERT( 0 );
			}
		}
	}
}

void CDeletedProductHelper::VerifyHydroMod(IDPTR IDPtr, bool &bDisplay)
{
	CDB_Product *pclProduct = NULL;
	CTable *pTab = (CTable *)( IDPtr.MP );

	for( IDPTR IDPtrChild = pTab->GetFirst(); _T('\0') != *IDPtrChild.ID; IDPtrChild = pTab->GetNext() )
	{
		CDS_HydroMod *pChild = (CDS_HydroMod *)(IDPtrChild.MP );

		if( true == pChild->IsaModule() )
		{
			VerifyHydroMod( pChild->GetIDPtr(), bDisplay );
		}

		// DpC.
		if( true == pChild->IsDpCExist( true ) )
		{
			CDS_HydroMod::CDpC *pHMDpC = pChild->GetpDpC();
			pclProduct = dynamic_cast<CDB_Product *>( (CData*)pHMDpC->GetIDPtr().MP );

			if( NULL != pclProduct)
			{
				// Take product from TADB...
				if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, FromHydraulicNetwork, 1 ) )
				{
					bDisplay = true;
					// HYS-1291 : Add one deleted product  
					m_mapHMDeleted.insert( std::pair<CDS_HydroMod*, CDS_HydroMod::eHMObj>( pChild, CDS_HydroMod::eHMObj::eDpC ) );
				}
			}
			else
			{
				ASSERT( 0 );
			}
		}

		// CV.
		if( true == pChild->IsCvExist( true ) )
		{
			CDS_HydroMod::CCv *pHMCv = pChild->GetpCV();
			pclProduct = dynamic_cast<CDB_Product *>( (CData *)pHMCv->GetCvIDPtr().MP );
			
			// HYS-2077: When TBV-C/CM Set are deleted they are not considered as deleted products to display.
			bool bCheckActuator = true;
			if( NULL != pclProduct && eb3True != pHMCv->GetActrSelectedAsaPackage() )
			{
				// Take product from TADB...
				if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, FromHydraulicNetwork, 1 ) )
				{
					bDisplay = true;
					// HYS-1291 : Add one deleted product 
					m_mapHMDeleted.insert( std::pair<CDS_HydroMod*, CDS_HydroMod::eHMObj>( pChild, CDS_HydroMod::eHMObj::eCV ) );
				}
			}
			else if( NULL != pclProduct && eb3True == pHMCv->GetActrSelectedAsaPackage() )
			{
				CTableSet* pclSetTable = pclProduct->GetTableSet();
				ASSERT( NULL != pclSetTable );

				CDB_Set* pclSet = pclSetTable->FindCompatibleSet( pclProduct->GetIDPtr().ID, pHMCv->GetActrIDPtr().ID );
				if( NULL != pclSet )
				{
					bCheckActuator = false;
					// Take product from TADB...
					if (true == _PrepareListCtrl( pclSet, pclSet->GetName(), pclSet->GetIDPtr().ID, FromHydraulicNetwork, 1 ))
					{
						bDisplay = true;
						m_mapHMDeleted.insert( std::pair<CDS_HydroMod*, CDS_HydroMod::eHMObj>( pChild, CDS_HydroMod::eHMObj::eCV ) );
					}
				}
			}
			else if( true == pHMCv->IsTaCV() )
			{
				// Remark: if 'IsTaCV' return false, it means that user has chosen valve directly by inputing Kv. Thus it is normal to not
				// have in this case a 'CDB_Product' defined.
				ASSERT( 0 );
			}

			if( true == bCheckActuator )
			{
				CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator*>( (CData *)pHMCv->GetActrIDPtr().MP );

				// HYS-1868: bActCompatible = if the actuator is always compatible with the CV.
				// bAdaptCompatible = true if the adapter is always compatible with the CV.
				bool bActCompatible = true;
				bool bAdaptCompatible = true;

				if( NULL != pclActuator )
				{
					// verifiy if the actuator is compatible
					pHMCv->VerifyActuatorAndAdapter( &bActCompatible, &bAdaptCompatible );

					// Take product from TADB...
					if( true == _PrepareListCtrl( pclActuator, pclActuator->GetName(), pclActuator->GetIDPtr().ID, FromHydraulicNetwork, 1,
						bActCompatible, pclProduct->GetName() ) )
					{
						bDisplay = true;
						if( false == bActCompatible && true == pChild->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
						{
							// Found a new actuator and adapter
							pHMCv->SelectActuator();
							bAdaptCompatible = true;
						}
					}
				}

				if( false == bAdaptCompatible && NULL != pclActuator )
				{
					CDB_Product *pclAdapter = dynamic_cast<CDB_Product*>( (CData *)pHMCv->GetActrAdaptIDPtr().MP );

					if( ( NULL != pclAdapter ) && ( pclAdapter->IsAnAccessory() ) )
					{
						// verifiy if the adapter is compatible
						// Take product from TADB...
						if( true == _PrepareListCtrl( pclAdapter, pclAdapter->GetName(), pclAdapter->GetIDPtr().ID, FromHydraulicNetwork, 1,
							bAdaptCompatible, pclProduct->GetName() ) )
						{
							bDisplay = true;
							if( false == bAdaptCompatible )
							{
								// Select new adapter
								CDB_Product* pAdapt = pHMCv->GetpCV()->GetMatchingAdapter( pclActuator );

								if( NULL != pAdapt && true == pAdapt->IsAnAccessory() )
								{
									pHMCv->SetActrAdaptIDPtr( pAdapt->GetIDPtr() );
								}
								else
								{
									pHMCv->SetActrAdaptIDPtr( _NULL_IDPTR );
								}
							}
						}
					}
					else if( NULL == pclAdapter && false == bAdaptCompatible )
					{
						// Verify if we need adapter
						CDB_Product* pAdapt = pHMCv->GetpCV()->GetMatchingAdapter( pclActuator );

						if( NULL != pAdapt && true == pAdapt->IsAnAccessory() )
						{
							pHMCv->SetActrAdaptIDPtr( pAdapt->GetIDPtr() );
							// Adapter is missing
							bDisplay = _PrepareListCtrl( pclAdapter, pclActuator->GetName(), pclActuator->GetIDPtr().ID, FromHydraulicNetwork, 1,
								bAdaptCompatible, pclProduct->GetName() );
						}
						else
						{
							pHMCv->SetActrAdaptIDPtr( _NULL_IDPTR );
							bDisplay = false; // no adpater needed it's ok
						}

					}
				}
			}

			if( pHMCv->GetCVAccCount() > 0 )
			{
				for( int iLoopAcc = 0; iLoopAcc < pHMCv->GetCVAccCount(); iLoopAcc++ )
				{
					if( true == VerifyAccessory( pHMCv->GetCVAccIDPtr( iLoopAcc ), FromHydraulicNetwork, 1 ) )
					{
						bDisplay = true;
					}
				}
			}

			if( pHMCv->GetActrAccCount() > 0 )
			{
				for( int iLoopAcc = 0; iLoopAcc < pHMCv->GetActrAccCount(); iLoopAcc++ )
				{
					if( true == VerifyAccessory( pHMCv->GetActrAccIDPtr( iLoopAcc ), FromHydraulicNetwork, 1 ) )
					{
						bDisplay = true;
					}
				}
			}

			if( pHMCv->GetCVActrAccSetCount() > 0 )
			{
				for( int iLoopAcc = 0; iLoopAcc < pHMCv->GetCVActrAccSetCount(); iLoopAcc++ )
				{
					if( true == VerifyAccessory( pHMCv->GetCVActrAccSetIDPtr( iLoopAcc ), FromHydraulicNetwork, 1 ) )
					{
						bDisplay = true;
					}
				}
			}
		}

		// Bv.
		if( true == pChild->IsBvExist( true ) )
		{
			CDS_HydroMod::CBV *pHMBV = pChild->GetpBv();
			pclProduct = dynamic_cast<CDB_Product *>( (CData *)pHMBV->GetIDPtr().MP );

			if( NULL != pclProduct )
			{
				// Take product from TADB...
				if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, FromHydraulicNetwork, 1 ) )
				{
					bDisplay = true;
					// HYS-1291 : Add one deleted product
					m_mapHMDeleted.insert( std::pair<CDS_HydroMod*, CDS_HydroMod::eHMObj>( pChild, CDS_HydroMod::eHMObj::eBVprim ) );
				}
			}
			else
			{
				ASSERT( 0 );
			}
		}

		// Bv bypass.
		if( true == pChild->IsBvBypExist( true ) )
		{
			CDS_HydroMod::CBV *pHMBVBypass = pChild->GetpBypBv();
			pclProduct = dynamic_cast<CDB_Product *>( (CData *)pHMBVBypass->GetIDPtr().MP );

			if( NULL != pclProduct )
			{
				// Take product from TADB...
				if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, FromHydraulicNetwork, 1 ) )
				{
					bDisplay = true;
					// HYS-1291 : Add one deleted product
					m_mapHMDeleted.insert( std::pair<CDS_HydroMod*, CDS_HydroMod::eHMObj>( pChild, CDS_HydroMod::eHMObj::eBVbyp ) );
				}
			}
			else
			{
				ASSERT( 0 );
			}
		}

		// Bv secondary.
		if( true == pChild->IsBvSecExist( true ) )
		{
			CDS_HydroMod::CBV *pHMBVSecondary = pChild->GetpSecBv();
			pclProduct = dynamic_cast<CDB_Product *>( (CData *)pHMBVSecondary->GetIDPtr().MP );

			if( NULL != pclProduct )
			{
				// Take product from TADB...
				if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, FromHydraulicNetwork, 1 ) )
				{
					bDisplay = true;
					// HYS-1291 : Add one deleted product
					m_mapHMDeleted.insert( std::pair<CDS_HydroMod*, CDS_HydroMod::eHMObj>( pChild, CDS_HydroMod::eHMObj::eBVsec ) );
				}
			}
			else
			{
				ASSERT( 0 );
			}
		}

		// ShutoffValve on supply.
		if( true == pChild->IsShutoffValveExist( CDS_HydroMod::eShutoffValveSupply, true ) )
		{
			CDS_HydroMod::CShutoffValve *pHMShutoffValve = pChild->GetpShutoffValve( CDS_HydroMod::eShutoffValveSupply );
			pclProduct = dynamic_cast<CDB_Product *>( (CData *)pHMShutoffValve->GetIDPtr().MP );

			if( NULL != pclProduct )
			{
				// Take product from TADB...
				if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, FromHydraulicNetwork, 1 ) )
				{
					bDisplay = true;
					// HYS-1291 : Add one deleted product
					m_mapHMDeleted.insert( std::pair<CDS_HydroMod*, CDS_HydroMod::eHMObj>( pChild, CDS_HydroMod::eHMObj::eShutoffValveSupply ) );
				}
			}
			else
			{
				ASSERT( 0 );
			}
		}

		// ShutoffValve on return.
		if( true == pChild->IsShutoffValveExist( CDS_HydroMod::eShutoffValveReturn, true ) )
		{
			CDS_HydroMod::CShutoffValve *pHMShutoffValve = pChild->GetpShutoffValve( CDS_HydroMod::eShutoffValveReturn );
			pclProduct = dynamic_cast<CDB_Product *>( (CData *)pHMShutoffValve->GetIDPtr().MP );

			if( NULL != pclProduct)
			{
				// Take product from TADB...
				if( true == _PrepareListCtrl( pclProduct, pclProduct->GetName(), pclProduct->GetIDPtr().ID, FromHydraulicNetwork, 1 ) )
				{
					bDisplay = true;
					// HYS-1291 : Add one deleted product
					m_mapHMDeleted.insert( std::pair<CDS_HydroMod*, CDS_HydroMod::eHMObj>( pChild, CDS_HydroMod::eHMObj::eShutoffValveReturn ) );
				}
			}
			else
			{
				ASSERT( 0 );
			}
		}
	}

	// HYS-1221 : commented this code because the combos in tech param are filled by using isSelectable().
	// So it is not possible to have non-selectable products in the combos. it is no more necessary to call ResetPrjParams
	if( true == bDisplay )
	{
		CPrjParams *pHMGenParam = TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams();
		ASSERT(NULL != pHMGenParam);

		if( NULL != pHMGenParam)
		{
			pHMGenParam->ResetPrjParams( true );
		}
	}
}

bool CDeletedProductHelper::_PrepareListCtrl( CDB_Thing *pThing, LPCTSTR lpName, LPCTSTR lpNameID, FromWhere eFromWhere, int iQuantity, 
											  bool bIsCompatible, LPCTSTR NameToCheck )
{
	// Variable.
	bool bDisplay = false;
	CString str;

	Info rInfo;

	if( NULL == pThing )
	{
		// HYS-1868: In this case the adapter is missing.
		if( false == bIsCompatible )
		{
			if( m_mapNotCompatible.count( eFromWhere ) > 0 && m_mapNotCompatible[eFromWhere].count( CString( lpNameID ) ) )
			{
				m_mapNotCompatible[eFromWhere][lpNameID].m_iNumber += iQuantity;
			}
			else
			{
				rInfo.m_strValve = lpName;
				rInfo.m_iNumber = iQuantity;
				rInfo.m_strBottomMessage.Format( TASApp.LoadLocalizedString( IDS_PRODUCT_NOTCOMPATIBLE_WO_ADPT ), lpName, NameToCheck );
				m_mapNotCompatible[eFromWhere][lpNameID] = rInfo;
			}
		}
		else
		{
			if( m_mapUndef.count( eFromWhere ) > 0 && m_mapUndef[eFromWhere].count( CString( lpNameID ) ) )
			{
				m_mapUndef[eFromWhere][lpNameID].m_iNumber += iQuantity;
			}
			else
			{
				rInfo.m_strValve = lpName;
				rInfo.m_iNumber = iQuantity;
				rInfo.m_strBottomMessage.Format( TASApp.LoadLocalizedString( IDS_PRODUCT_NOTEXIST ), lpNameID );
				m_mapUndef[eFromWhere][lpNameID] = rInfo;
			}
		}
		bDisplay = true;
	}
	else
	{
		if( true == pThing->IsDeleted() )
		{
			if( m_mapDeleted.count( eFromWhere ) > 0 && m_mapDeleted[eFromWhere].count( CString( lpNameID ) ) )
			{
				m_mapDeleted[eFromWhere][lpNameID].m_iNumber += iQuantity;
			}
			else
			{
				rInfo.m_strValve = lpName;
				rInfo.m_iNumber = iQuantity;
				rInfo.m_strBottomMessage.Format( TASApp.LoadLocalizedString( IDS_PRODUCT_NOTEXIST ), lpNameID );
				m_mapDeleted[eFromWhere][lpNameID] = rInfo;
			}

			bDisplay = true;
		}

		if( false == pThing->IsAvailable() )
		{
			if( m_mapNotAvailable.count( eFromWhere ) > 0 && m_mapNotAvailable[eFromWhere].count( CString( lpNameID ) ) )
			{
				m_mapNotAvailable[eFromWhere][lpNameID].m_iNumber += iQuantity;
			}
			else
			{
				rInfo.m_strValve = lpName;
				rInfo.m_iNumber = iQuantity;
				rInfo.m_strBottomMessage.Format( TASApp.LoadLocalizedString( IDS_PRODUCT_NOTEXIST ), lpNameID );
				m_mapNotAvailable[eFromWhere][lpNameID] = rInfo;
			}

			bDisplay = true;
		}

		// HYS-1868: Look compatibility change between two products
		if( false == bIsCompatible )
		{
			if( m_mapNotCompatible.count( eFromWhere ) > 0 && m_mapNotCompatible[eFromWhere].count( CString( lpNameID ) ) )
			{
				m_mapNotCompatible[eFromWhere][lpNameID].m_iNumber += iQuantity;
			}
			else
			{
				rInfo.m_strValve = lpName;
				rInfo.m_iNumber = iQuantity;
				rInfo.m_strBottomMessage.Format( TASApp.LoadLocalizedString( IDS_PRODUCT_NOTCOMPATIBLE ), lpName, NameToCheck );
				m_mapNotCompatible[eFromWhere][lpNameID] = rInfo;
			}

			bDisplay = true;
		}
	}
	return bDisplay;
}

void CDeletedProductHelper::_VerifyIndividualSelection6WayValveHelper( IDPTR ProductIDPtr, IDPTR ActuatorIDPtr, CAccessoryList *pclAccessoryList, 
		CAccessoryList *pclActuatorAccessoryList, FromWhere eWhere, int iGlobalQuantity, bool &bDisplay )
{
	if( NULL == ProductIDPtr.MP || NULL == dynamic_cast<CDB_ControlValve*>( ProductIDPtr.MP ) )
	{
		return;
	}

	CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve*>( ProductIDPtr.MP );

	if( true == _PrepareListCtrl( pclControlValve, pclControlValve->GetName(), pclControlValve->GetIDPtr().ID, eWhere, iGlobalQuantity ) )
	{
		bDisplay = true;
	}

	// Check associated actuator if exist.
	if( NULL != dynamic_cast<CDB_Actuator*>( ActuatorIDPtr.MP ) )
	{
		CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator*>( ActuatorIDPtr.MP );

		if( true == _PrepareListCtrl( pclActuator, pclActuator->GetName(), pclActuator->GetIDPtr().ID, eWhere, iGlobalQuantity ) )
		{
			bDisplay = true;
		}
	}

	// Check associated accessories if exist.
	if( true == VerifyAccessoryList( pclAccessoryList, eWhere, iGlobalQuantity ) )
	{
		bDisplay = true;
	}

	// Check associated actuator's accessories if exist.
	if( true == VerifyAccessoryList( pclActuatorAccessoryList, eWhere, iGlobalQuantity ) )
	{
		bDisplay = true;
	}
}

void CDeletedProductHelper::_VerifyIndividualSelectionBvHelper( IDPTR ProductIDPtr, CAccessoryList *pclAccessoryList, FromWhere eWhere, int iGlobalQuantity, bool &bDisplay )
{
	if( NULL == ProductIDPtr.MP || NULL == dynamic_cast<CDB_RegulatingValve*>( ProductIDPtr.MP ) )
	{
		return;
	}

	CDB_RegulatingValve *pclBalancingValve = dynamic_cast<CDB_RegulatingValve*>( ProductIDPtr.MP );

	// Take product from TADB...
	if( true == _PrepareListCtrl( pclBalancingValve, pclBalancingValve->GetName(), pclBalancingValve->GetIDPtr().ID, eWhere, iGlobalQuantity ) )
	{
		bDisplay = true;
	}

	// Check associated accessories if exist.
	if( true == VerifyAccessoryList( pclAccessoryList, eWhere, iGlobalQuantity ) )
	{
		bDisplay = true;
	}
}
